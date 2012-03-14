#include "menu.h"

#include <vdr/channels.h>


cNoepgMainMenu::cNoepgMainMenu(void)
{
  bool modeSet = false;
  int inList;
  tChannelID cid;
  cNoepgChannelID *item;
  cNoepgChannelID::ReadConfig(_filelist, NULL);
  for (cChannel *chan = Channels.First(); chan; chan = Channels.Next(chan)) {
       if (chan->GroupSep())
          continue;
       cid = chan->GetChannelID();
       inList = 0;
       for (cNoepgChannelID* c = _filelist.First(); c; c = _filelist.Next(c)) {
           if (!c->id.Valid()) {
              if (!modeSet) {
                 modeSet = true;
                 if (c->mode == enemBlacklist)
                    _mode = 0;
                 else if (c->mode == enemWhitelist)
                    _mode = 1;
                 Add(new cMenuEditBoolItem(tr("mode"), &_mode, tr("blacklist"), tr("whitelist")));
                 }
              continue;
              }

           if (c->id == cid) {
              inList = 1;
              break;
              }
           }

      item = new cNoepgChannelID();
      item->mode = (eNoEpgMode)inList;
      item->id = cid;
      _editlist.Add(item);
      Add(new cMenuEditBoolItem(chan->Name(), (int*)&item->mode));
      }
}

cNoepgMainMenu::~cNoepgMainMenu(void)
{
}

void cNoepgMainMenu::Store(void)
{
  _filelist.cList<cNoepgChannelID>::Clear();
  _filelist.Add(new cNoepgChannelID((_mode == 1) ? enemWhitelist : enemBlacklist, tChannelID::InvalidID));
  for (cNoepgChannelID *c = _editlist.First(); c; c = _editlist.Next(c)) {
      bool inList = ((int)c->mode == 1);
      if (c->id.Valid() && inList) {
         _filelist.Add(new cNoepgChannelID(enemUnknown, c->id));
         }
      }
  cNoepgChannelID::SaveConfig(_filelist, NULL);
  cNoepgChannelID::ReadConfig(cNoepgChannelID::NoEpgChannels, &cNoepgChannelID::NoEpgMutex);
}
