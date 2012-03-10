/*
 * noepg.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <vdr/libsi/section.h>
#include <vdr/epg.h>

#if VDRVERSNUM < 10726
class cEpgHandler : public cListObject {
public:
  cEpgHandler(void) {}
  virtual ~cEpgHandler() {}
  virtual bool HandleEitEvent(cSchedule *Schedule, const SI::EIT::Event *EitEvent) { return false; }
  virtual bool SetTitle(cEvent *Event, const char *Title) { return false; }
  virtual bool SetShortText(cEvent *Event, const char *ShortText) { return false; }
  virtual bool SetDescription(cEvent *Event, const char *Description) { return false; }
  virtual bool HandleEvent(cEvent *Event) { return false; }
  };
#endif


enum eNoEpgMode {
  enemUnknown    = 0,
  enemWhitelist  = 1,
  enemBlacklist  = 2
  };

class cChannelID;

cMutex              NoEpgMutex;
cConfig<cChannelID> NoEpgChannels;

class cChannelID : public cListObject {
public:
  // either mode or id is set, never both
  eNoEpgMode mode;
  tChannelID id;

  cChannelID(void)
  :mode(enemUnknown)
  {
  }

  virtual ~cChannelID(void) {}

  bool Parse(const char *s)
  {
    if (strncmp(s, "mode=w", 6) == 0) {
       mode = enemWhitelist;
       return true;
       }
    if (strncmp(s, "mode=b", 6) == 0) {
       mode = enemBlacklist;
       return true;
       }
    id = tChannelID::FromString(s);
    return id.Valid();
  }

  bool Save(FILE *f)
  {
    if (id.Valid())
       return fprintf(f, "%s\n", *id.ToString()) > 0;
    if (mode == enemWhitelist)
       return fprintf(f, "mode=whitelist\n") > 0;
    if (mode == enemBlacklist)
       return fprintf(f, "mode=blacklist\n") > 0;
    return false;
  }
  };

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

  virtual bool HandleEitEvent(cSchedule *Schedule, const SI::EIT::Event *EitEvent)
  {
    if ((_lastMode == enemUnknown) || (!(_lastChannel == Schedule->ChannelID()))) {
       cMutexLock lock(&NoEpgMutex);
       _lastChannel = Schedule->ChannelID();
       _lastChannelIsInList = false;
       for (cChannelID* c = NoEpgChannels.First(); c; c = NoEpgChannels.Next(c)) {
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

static const char *VERSION        = "0.0.1";
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

  bool ReadConfig(void);
  bool SaveConfig(void);
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
  if (ReadConfig())
     new cNoEpgHandler();
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
  SaveConfig();
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
  return NULL;
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

bool cPluginNoepg::ReadConfig(void)
{
  cMutexLock lock(&NoEpgMutex);
  cString filename = cString::sprintf("%s/settings.conf", ConfigDirectory(Name()));
  return NoEpgChannels.Load(*filename, true, true);
}

bool cPluginNoepg::SaveConfig(void)
{
  cMutexLock lock(&NoEpgMutex);
  return NoEpgChannels.Save();
}

VDRPLUGINCREATOR(cPluginNoepg); // Don't touch this!
