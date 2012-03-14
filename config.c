#include "config.h"

#include <vdr/plugin.h>


bool                     cNoepgChannelID::NoEpgForceEval = false;
cMutex                   cNoepgChannelID::NoEpgMutex;
cConfig<cNoepgChannelID> cNoepgChannelID::NoEpgChannels;


cNoepgChannelID::cNoepgChannelID(void)
:mode(enemUnknown)
{
}

cNoepgChannelID::cNoepgChannelID(eNoEpgMode Mode, tChannelID Id)
:mode(Mode)
,id(Id)
{
}

bool cNoepgChannelID::Parse(const char *s)
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

bool cNoepgChannelID::Save(FILE *f)
{
  if (id.Valid())
     return fprintf(f, "%s\n", *id.ToString()) > 0;
  if (mode == enemWhitelist)
     return fprintf(f, "mode=whitelist\n") > 0;
  if (mode == enemBlacklist)
     return fprintf(f, "mode=blacklist\n") > 0;
  return false;
}

static cMutex sFileMutex;

bool cNoepgChannelID::ReadConfig(cConfig<cNoepgChannelID> &Config, cMutex *Mutex)
{
  cMutexLock filelock(&sFileMutex);
  cMutexLock lock(Mutex);
  cString filename = cString::sprintf("%s/settings.conf", cPlugin::ConfigDirectory("noepg"));
  return Config.Load(*filename, true, true);
}

bool cNoepgChannelID::SaveConfig(cConfig<cNoepgChannelID> &Config, cMutex *Mutex)
{
  cMutexLock filelock(&sFileMutex);
  cMutexLock lock(Mutex);
  return Config.Save();
}
