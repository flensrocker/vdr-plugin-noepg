#ifndef __NOEPG_MENU_H
#define __NOEPG_MENU_H

#include "config.h"

#include <vdr/menuitems.h>


class cNoepgMainMenu : public cMenuSetupPage
{
private:
 int  _mode;
 int  _dummy;
 cConfig<cNoepgChannelID> _filelist;
 cConfig<cNoepgChannelID> _editlist;

protected:
  virtual void Store(void);

public:
  cNoepgMainMenu(void);
  virtual ~cNoepgMainMenu(void);
};

#endif
