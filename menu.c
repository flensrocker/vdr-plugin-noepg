#include "menu.h"

#include <vdr/channels.h>


cNoepgMainMenu::cNoepgMainMenu(void)
{
  bool modeSet = false;
  int inList;
  tChannelID cid;
  cString *comment;
  cNoepgChannelID *item;

  if (!cNoepgChannelID::ReadConfig(_filelist, NULL)) {
     modeSet = true;
     _mode = 0;
     Add(new cMenuEditBoolItem(tr("mode"), &_mode, tr("blacklist"), tr("whitelist")));
     }

#if VDRVERSNUM > 20300
  LOCK_CHANNELS_READ;
  const cChannels *channels = Channels;
#else
  const cChannels *channels = &Channels;
#endif
  for (const cChannel *chan = channels->First(); chan; chan = channels->Next(chan)) {
      if (chan->GroupSep())
         continue;
      cid = chan->GetChannelID();
      inList = 0;
      comment = NULL;
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
             if (c->comment != NULL)
                comment = new cString(*c->comment);
             break;
             }
          }

      item = new cNoepgChannelID();
      item->mode = (eNoEpgMode)inList;
      item->id = cid;
      item->comment = comment;
      _editlist.Add(item);
      Add(new cMenuEditBoolItem(chan->Name(), (int*)&item->mode));
      }

  SetHelp(NULL, tr("page up"), tr("page down"), NULL);
}

cNoepgMainMenu::~cNoepgMainMenu(void)
{
}

void cNoepgMainMenu::Store(void)
{
  _filelist.cList<cNoepgChannelID>::Clear();
  _filelist.Add(new cNoepgChannelID((_mode == 1) ? enemWhitelist : enemBlacklist, tChannelID::InvalidID, NULL));
  for (cNoepgChannelID *c = _editlist.First(); c; c = _editlist.Next(c)) {
      bool inList = ((int)c->mode == 1);
      if (c->id.Valid() && inList) {
         cString *comment = NULL;
         if (c->comment != NULL)
            comment = new cString(*c->comment);
         _filelist.Add(new cNoepgChannelID(enemUnknown, c->id, comment));
         }
      }
  cNoepgChannelID::SaveConfig(_filelist, NULL);

  cMutexLock lock(&cNoepgChannelID::NoEpgMutex);
  cNoepgChannelID::ReadConfig(cNoepgChannelID::NoEpgChannels, &cNoepgChannelID::NoEpgMutex);
  cNoepgChannelID::NoEpgForceEval = true;
}

eOSState cNoepgMainMenu::ProcessKey(eKeys Key)
{
  if (Key == kGreen) {
     PageUp();
     return osUnknown;
     }
  if (Key == kYellow) {
     PageDown();
     return osUnknown;
     }
  return cMenuSetupPage::ProcessKey(Key);
}
