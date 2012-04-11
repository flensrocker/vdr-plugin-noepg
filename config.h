#ifndef __NOEPG_CONFIG_H

#include <vdr/channels.h>
#include <vdr/thread.h>


enum eNoEpgMode {
  enemUnknown    = 0,
  enemWhitelist  = 1,
  enemBlacklist  = 2
  };

class cNoepgChannelID : public cListObject {
public:
  // either mode or id is set, never both
  eNoEpgMode mode;
  tChannelID id;

private:
  cString *comment;

public:
  cNoepgChannelID(void);
  cNoepgChannelID(eNoEpgMode Mode, tChannelID Id);
  virtual ~cNoepgChannelID(void);

  bool Parse(const char *s);
  bool Save(FILE *f);

  static bool                     NoEpgForceEval;
  static cMutex                   NoEpgMutex;
  static cConfig<cNoepgChannelID> NoEpgChannels;

  static bool ReadConfig(cConfig<cNoepgChannelID> &Config, cMutex *Mutex);
  static bool SaveConfig(cConfig<cNoepgChannelID> &Config, cMutex *Mutex);
  };

#endif
