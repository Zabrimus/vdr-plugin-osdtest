#ifdef __cplusplus
extern "C" {
#endif
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <fstream>
#include <chrono>
#include <thread>
#include <vdr/plugin.h>
#include <unistd.h>
#include "osdimage.h"

#define SET_AREA_VERY_EARLY
#define SCALEOSD

OsdImage *osdImage;

uint8_t testImage[1280 * 720 * 4];
struct SwsContext *swsCtx = nullptr;

OsdImage::OsdImage() {
    fprintf(stderr, "==> Construct OsdImage...\n");

    osd = nullptr;
    pixmap = nullptr;
    resizeOsd = false;

    // load test image
    FILE *f = fopen("/tmp/test_image.rgba", "rb");
    fread(&testImage[0], 1280 * 720 * 4, 1, f);
    fclose(f);

    osdImage = this;
}

OsdImage::~OsdImage() {
    fprintf(stderr, "Destroy OsdImage...\n");

    if (osd != nullptr) {
        delete osd;
        osd = nullptr;
    }

    pixmap = nullptr;
    osdImage = nullptr;
}

void OsdImage::Show() {
    fprintf(stderr, "==> OsdImage Show()\n");
    Display();
}

void OsdImage::Display() {
    fprintf(stderr, "==> Display\n");
    if (osd) {
        delete osd;
    }

    osd = cOsdProvider::NewOsd(0, 0);

#ifdef SET_AREA_VERY_EARLY
    tArea areas[] = {
            {0, 0, 3840 - 1, 2160 - 1, 32}, // 4K2K
            {0, 0, 2560 - 1, 1440 - 1, 32}, // 2K
            {0, 0, 1920 - 1, 1080 - 1, 32}, // Full HD
            {0, 0, 1280 - 1,  720 - 1, 32}, // 720p
    };

    bool areaFound = false;
    for (int i = 0; i < 4; ++i) {
        auto areaResult = osd->SetAreas(&areas[i], 1);

        if (areaResult == oeOk) {
            fprintf(stderr, "Area size set to %d:%d - %d:%d\n", areas[i].x1, areas[i].y1, areas[i].x2, areas[i].y2);
            areaFound = true;
            break;
        }
    }

    if (!areaFound) {
        fprintf(stderr, "Unable set any OSD area. OSD will not be created\n");
    }
#endif
    SetOsdSize();
}

void OsdImage::TriggerOsdResize() {
    fprintf(stderr, "OsdImage TriggerOsdResize()\n");
    resizeOsd = true;
    readOsdUpdate();
}

void OsdImage::SetOsdSize() {
    fprintf(stderr, "OsdImage SetOsdSize()\n");

    double ph;
    cDevice::PrimaryDevice()->GetOsdSize(disp_width, disp_height, ph);

    if (disp_width <= 0 || disp_height <= 0 || disp_width > 4096 || disp_height > 2160) {
        fprintf(stderr, "hbbtv: Got illegal OSD size %dx%d", disp_width, disp_height);
        pixmap = nullptr;
        return;
    }

    cRect rect(0, 0, disp_width, disp_height);

#ifndef SET_AREA_VERY_EARLY
    tArea area  = {0, 0, disp_width - 1, disp_height - 1, 32};
    auto areaResult = osd->SetAreas(&area, 1);

    if (areaResult == oeOk) {
        fprintf(stderr, "Area size set to %d:%d - %d:%d\n", area.x1, area.y1, area.x2, area.y2);
        return;
    }
#endif

    // try to get a pixmap
    fprintf(stderr, "OsdImage SetOsdSize, Create pixmap %dx%d\n", disp_width, disp_height);
    pixmap = osd->CreatePixmap(0, rect, rect);

    if (pixmap == nullptr) {
        fprintf(stderr, "== pixmap is null ==\n");
        return;
    }

    fprintf(stderr, "OsdImage SetOsdSize, Clear Pixmap\n");
    pixmap->Lock();
    pixmap->Clear();
    pixmap->Unlock();

    resizeOsd = false;

    readOsdUpdate();
}

eOSState OsdImage::ProcessKey(eKeys Key) {
    eOSState state = cOsdObject::ProcessKey(Key);

    if (state == osUnknown) {
        if (Key == kBack) {
            return osEnd;
        }
    }

    return state;
}

void OsdImage::readOsdUpdate() {
    fprintf(stderr, "start readOsdUpdate\n");

    if (resizeOsd) {
        fprintf(stderr, "OsdImage readOsdUpdate, setOsdSize\n");
        SetOsdSize();
        resizeOsd = false;
    }

    if (disp_width <= 0 || disp_height <= 0 || disp_width > 4096 || disp_height > 2160) {
        fprintf(stderr, "hbbtv: Got illegal OSD size %dx%d", disp_width, disp_height);
        return;
    }

#ifdef SCALEOSD
    fprintf(stderr, "Image size %dx%d\n", disp_width, disp_height);

    // create image buffer for scaled image
    cSize recImageSize(disp_width, disp_height);
    cPoint recPoint(0, 0);
    const cImage recImage(recImageSize);
    auto *scaled  = (uint8_t*)(recImage.Data());

    if (scaled == nullptr) {
        fprintf(stderr, "Out of memory reading OSD image\n");
        return;
    }

    // scale image
    fprintf(stderr, "OsdImage readOsdUpdate, get scale context\n");
    swsCtx = sws_getCachedContext(swsCtx,
                                  1280, 720, AV_PIX_FMT_BGRA,
                                  disp_width, disp_height, AV_PIX_FMT_BGRA,
                                  SWS_BILINEAR, NULL, NULL, NULL);

    uint8_t *inData[1] = { &testImage[0] };
    int inLinesize[1] = { 4 * 1280 };
    int outLinesize[1] = { 4 * disp_width };

    fprintf(stderr, "OsdImage readOsdUpdate, scale image\n");
    sws_scale(swsCtx, inData, inLinesize, 0, 720, &scaled, outLinesize);

    if (pixmap != nullptr) {
        fprintf(stderr, "OsdImage readOsdUpdate, draw image lock\n");
        pixmap->Lock();
        pixmap->DrawImage(recPoint, recImage);
        pixmap->Unlock();
        fprintf(stderr, "OsdImage readOsdUpdate, draw image unlock\n");
    }
#else
    fprintf(stderr, "Image size %dx%d\n", disp_width, disp_height);

    // create image buffer for scaled image
    cSize recImageSize(1280, 720);
    cPoint recPoint(0, 0);
    const cImage recImage(recImageSize);
    auto *scaled  = (uint8_t*)(recImage.Data());

    if (scaled == nullptr) {
        fprintf(stderr, "Out of memory reading OSD image\n");
        return;
    }

    fprintf(stderr, "OsdImage readOsdUpdate, image\n");

    if (pixmap != nullptr) {
        fprintf(stderr, "OsdImage readOsdUpdate, draw image lock\n");
        memcpy(scaled, &testImage[0], 1280 * 720 * 4);
        pixmap->Lock();
        pixmap->DrawImage(recPoint, recImage);
        pixmap->Unlock();
        fprintf(stderr, "OsdImage readOsdUpdate, draw image unlock\n");
    }
#endif

    if (osd != nullptr) {
        fprintf(stderr, "OsdImage readOsdUpdate, flush osd\n");
        osd->Flush();
    }
}
