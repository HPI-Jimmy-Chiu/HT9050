//------------------------------------------------------------------------------
// AI(W906-H013) 20261001 (St02-E): golden 912 mymessbox.cpp:1405 -- the ONE symbol of this file, fSecsAlarm
//   (class: SecsAlarmForm.h; ruling 11 = B, todo H-013 item 2).  Read by Command.cpp (ht9045_sm) at golden 912's three
//   terms; it lives alone in ht9045_globals, the archive every machine archive links, so a later reader anywhere links
//   too (St02 rule: a new global symbol other libraries call gets a file of its own).  Constant initialisation (no
//   static-init order risk).  Nothing in the port assigns it: golden 912's only writer is ShowSecsAlarmMessage
//   (mymessbox.cpp:1579), not translated -- so it stays NULL.
//------------------------------------------------------------------------------
#include <cstddef>
#include "SecsAlarmForm.h"

TSecsAlarmForm *fSecsAlarm = NULL;                                              //RogerYang 20260724 : Add for new S10F3 SECS alarm
