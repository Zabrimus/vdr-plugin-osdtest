#ifndef OSDIMAGE_H
#define OSDIMAGE_H

#include <string>
#include <thread>
#include <mutex>
#include <vdr/tools.h>
#include <vdr/osd.h>
#include <vdr/osdbase.h>

class OsdImage : public cOsdObject {

private:
    cPixmap *pixmap;
    cOsd* osd;

    int disp_width = 100;
    int disp_height = 100;
    bool resizeOsd;

public:
    OsdImage();
    ~OsdImage() override;
    void Show() override;

    void Display();

    void SetOsdSize();
    void TriggerOsdResize();

    void readOsdUpdate();
    eOSState ProcessKey(eKeys Key) override;
};

extern OsdImage *osdImage;

#endif // OSDIMAGE_H
