/*
 * noepg.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include "menu.h"

#include <vdr/plugin.h>
#include <vdr/epg.h>

#if VDRVERSNUM < 10726
#include <vdr/libsi/section.h>

class cEpgHandler : public cListObject {
public:
  cEpgHandler(void) {}
  virtual ~cEpgHandler() {}
  virtual bool IgnoreChannel(const cChannel *Channel) { return false; }
  };
#endif


class cNoEpgHandler : public cEpgHandler {
private:
  eNoEpgMode  _lastMode;
  tChannelID  _lastChannel;
  bool        _lastChannelIsInList;

public:
  cNoEpgHandler(void)
   :_lastMode(enemUnknown)
   ,_lastChannel()
   ,_lastChannelIsInList(false)
  {
  }

  virtual ~cNoEpgHandler()
  {
  }

  virtual bool IgnoreChannel(const cChannel *Channel)
  {
    if ((_lastMode == enemUnknown) || (!(_lastChannel == Channel->GetChannelID()))) {
       cMutexLock lock(&cNoepgChannelID::NoEpgMutex);
       _lastChannel = Channel->GetChannelID();
       _lastChannelIsInList = false;
       for (cNoepgChannelID* c = cNoepgChannelID::NoEpgChannels.First(); c; c = cNoepgChannelID::NoEpgChannels.Next(c)) {
           if (!c->id.Valid()) {
              if (c->mode != enemUnknown)
                 _lastMode = c->mode;
              continue;
              }
           if (c->id == _lastChannel) {
              _lastChannelIsInList = true;
              break;
              }
           }
       }

    if ((_lastMode == enemWhitelist) && !_lastChannelIsInList)
       return true; // block unknown channels

    if ((_lastMode == enemBlacklist) && _lastChannelIsInList)
       return true; // block known channels

    return false; // otherwise allow, esp. if mode is unknown
  }
  };

static const char *VERSION        = "0.0.2";
static const char *DESCRIPTION    = "block/allow EPG for selected channels";
static const char *MAINMENUENTRY  = NULL;

class cPluginNoepg : public cPlugin {
private:
  // Add any member variables or functions you may need here.
public:
  cPluginNoepg(void);
  virtual ~cPluginNoepg();
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

cPluginNoepg::cPluginNoepg(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginNoepg::~cPluginNoepg()
{
  // Clean up after yourself!
}

const char *cPluginNoepg::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginNoepg::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginNoepg::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
#if VDRVERSNUM >= 10726
  if (cNoepgChannelID::ReadConfig(cNoepgChannelID::NoEpgChannels, &cNoepgChannelID::NoEpgMutex))
     new cNoEpgHandler();
#endif
  return true;
}

bool cPluginNoepg::Start(void)
{
  // Start any background activities the plugin shall perform.
  return true;
}

void cPluginNoepg::Stop(void)
{
  // Stop any background activities the plugin is performing.
#if VDRVERSNUM >= 10726
  cNoepgChannelID::SaveConfig(cNoepgChannelID::NoEpgChannels, &cNoepgChannelID::NoEpgMutex);
#endif
}

void cPluginNoepg::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginNoepg::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginNoepg::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginNoepg::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginNoepg::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginNoepg::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cNoepgMainMenu();
}

bool cPluginNoepg::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginNoepg::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginNoepg::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginNoepg::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

VDRPLUGINCREATOR(cPluginNoepg); // Don't touch this!
