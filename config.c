#include "config.h"

#include <vdr/plugin.h>


bool                     cNoepgChannelID::NoEpgForceEval = false;
cMutex                   cNoepgChannelID::NoEpgMutex;
cConfig<cNoepgChannelID> cNoepgChannelID::NoEpgChannels;


cNoepgChannelID::cNoepgChannelID(void)
:mode(enemUnknown)
,id(tChannelID::InvalidID)
,comment(NULL)
{
}

cNoepgChannelID::cNoepgChannelID(eNoEpgMode Mode, tChannelID Id, cString *Comment)
:mode(Mode)
,id(Id)
,comment(Comment)
{
}

cNoepgChannelID::~cNoepgChannelID(void)
{
  if (comment != NULL) {
     delete comment;
     comment = NULL;
     }
}

bool cNoepgChannelID::Parse(const char *s)
{
  comment = NULL;
  if (strncmp(s, "mode=w", 6) == 0) {
     mode = enemWhitelist;
     return true;
     }
  if (strncmp(s, "mode=b", 6) == 0) {
     mode = enemBlacklist;
     return true;
     }
  // allow comments after first space
  int spos = 0;
  int slen = strlen(s);
  while ((spos < slen) && (s[spos] != ' '))
        spos++;
  if (spos < slen) {
     char *tmp = new char[spos + 1];
     memcpy(tmp, s, spos);
     tmp[spos] = 0;
     id = tChannelID::FromString(tmp);
     delete tmp;
     tmp = skipspace(s + spos);
     if ((tmp != NULL) && (strlen(tmp) > 0)) {
        if ((strlen(tmp) <= 3) || !startswith(tmp, "//="))
           comment = new cString(tmp);
        }
     }
  else
     id = tChannelID::FromString(s);
  return id.Valid();
}

bool cNoepgChannelID::Save(FILE *f)
{
  if (id.Valid()) {
     if (comment == NULL) {
#if VDRVERSNUM > 20300
        LOCK_CHANNELS_READ;
        const cChannels *channels = Channels;
#else
        cChannels *channels = &Channels;
#endif
        const cChannel *c = channels->GetByChannelID(id);
        if (c != NULL)
           return fprintf(f, "%s //= %s\n", *id.ToString(), c->Name()) > 0;
        return fprintf(f, "%s\n", *id.ToString()) > 0;
        }
     return fprintf(f, "%s %s\n", *id.ToString(), **comment) > 0;
     }
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
