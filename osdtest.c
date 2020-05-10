/*
 * osdtest.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include "osdimage.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Enter description for 'osdtest' plugin";
static const char *MAINMENUENTRY  = "Osdtest";

class cPluginOsdtest : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  int lastDisplayWidth = 0;
  int lastDisplayHeight = 0;

public:
  cPluginOsdtest(void);
  virtual ~cPluginOsdtest();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual time_t WakeupTime(void);
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginOsdtest::cPluginOsdtest(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginOsdtest::~cPluginOsdtest()
{
  // Clean up after yourself!
}

const char *cPluginOsdtest::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginOsdtest::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginOsdtest::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return true;
}

bool cPluginOsdtest::Start(void)
{
  // Start any background activities the plugin shall perform.
  return true;
}

void cPluginOsdtest::Stop(void)
{
  // Stop any background activities the plugin is performing.
}

void cPluginOsdtest::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginOsdtest::MainThreadHook(void)
{
    if (osdImage != NULL) {
        static int osdState = 0;
        if (cOsdProvider::OsdSizeChanged(osdState)) {
            int newWidth;
            int newHeight;
            double ph;
            cDevice::PrimaryDevice()->GetOsdSize(newWidth, newHeight, ph);

            if (newWidth != lastDisplayWidth || newHeight != lastDisplayHeight) {
                fprintf(stderr, "Old Size: %dx%d, New Size. %dx%d\n", lastDisplayWidth, lastDisplayHeight, newWidth, newHeight);
                lastDisplayWidth = newWidth;
                lastDisplayHeight = newHeight;

                fprintf(stderr, "MainThreadHook => Display()");
                osdImage->Display();
            }

            // osdImage->TriggerOsdResize();
        }
    }
}

cString cPluginOsdtest::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginOsdtest::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginOsdtest::MainMenuAction(void)
{
    osdImage = new OsdImage();
    osdImage->readOsdUpdate();
    return osdImage;
}

cMenuSetupPage *cPluginOsdtest::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginOsdtest::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginOsdtest::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginOsdtest::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginOsdtest::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

VDRPLUGINCREATOR(cPluginOsdtest); // Don't touch this!
