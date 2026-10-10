// =============================================================================
//  SecsAlarmTimer_St02.h -- POOL-15: the SECS/GEM host terminal messages (S10F3 / S10F5 / S2F41 HOST_ALARM_DESCRIPTION /
//  RunCheck failed) shown to the operator per golden 913.  AI(W906-POOL15) 20261010 (St02).  Body: SECSGEM/SecsAlarmTimer_St02.cpp
//  (its banner has the golden map, the port notes and what is left out).
//
//  The golden window is a screen: whether it is open belongs to the browser (laptop, TO_STEVEN s4 20261010 16:3x).  There is
//  no C++ form object here; the window is the web message box (tools/wb_serve.cpp, the show-my-message mailbox), installed
//  through the three hooks below.  All three 0 (the default, and every ctest that does not install fakes) = no web host =
//  the timer is off and nothing is shown.
// =============================================================================
#ifndef SECSALARMTIMER_ST02_H
#define SECSALARMTIMER_ST02_H

#include "vclcompat/vcl_compat.h"   // AnsiString
#include <string>

namespace ht9045 { class TTimerEntry; }

// Web host (tools/wb_serve.cpp W906_MsgBoxHostInstall installs them, W906_MsgBoxHostUninstall passes 0, 0, 0):
//   post   -- put one non-blocking SECS message box on the mailbox; returns its requestId, "" = not posted (retried next tick)
//   isOurs -- the mailbox still holds that requestId, pending (another box replaces it: the mailbox has one slot)
//   retire -- the mailbox back to idle (called only while it still holds ours)
typedef std::string (*W906SecsPostFn_St02)(const char* s1, const char* s2);
typedef bool        (*W906SecsIsOursFn_St02)(const char* qid);
typedef void        (*W906SecsRetireFn_St02)(const char* qid);

// golden 913 TFSECS::FormCreate (UsecegemMainFrom.cpp:150, TimerSecsAlarm->Enabled=true) / FormDestroy (:967, false):
// installs the hooks; the first call builds the timer table entry "FSECS.TimerSecsAlarm" (golden UsecegemMainFrom.dfm:10977-10982:
// Enabled = False, no Interval = 1000 ms); post != 0 turns it on, post == 0 turns it off.
void W906_SecsAlarmHostInstall_St02(W906SecsPostFn_St02 post, W906SecsIsOursFn_St02 isOurs, W906SecsRetireFn_St02 retire);

// The entry (0 before the first install).  Its OnTimer = the web upkeep (repost) + W906_TimerSecsAlarmTimer_St02.
ht9045::TTimerEntry* W906_SecsAlarmTimerEntry_St02();

// golden 913 TFSECS::TimerSecsAlarmTimer (SECSGEM/UsecegemMainFrom.cpp:1023-1110): one tick of the queue drain.
void W906_TimerSecsAlarmTimer_St02();

// golden 913 ShowSecsAlarmMessage (mymessbox.cpp:1565-1630).
void W906_ShowSecsAlarmMessage_St02(AnsiString S1, AnsiString S2 = "");

// The browser's answer to the box (tools/wb_serve.cpp W906_MsgBoxModelessAnswer asks this first):
//   0 = not this window's request (wb_serve goes on as before); 1 = taken (reply = ""); -1 = refused (reply = why).
// OK / PAUSE (the page's pnlPause) = golden 913 TSecsAlarmForm::btnOKClick (mymessbox.cpp:1455-1508) -> DoReleaseAndHide
// (:1510-1553).  Nothing else closes the window (golden 913 L12: no X, Alt+F4 refused, :1415-1416 / :1560-1563).
int W906_SecsAlarmAnswer_St02(const char* tag, const char* action, std::string* reply);

// golden 913 `fSecsAlarm && fSecsAlarm->Visible` as far as C++ knows it: a SECS message box was put on the mailbox and has not
// been answered with OK (nor hidden by the MAXIM_THAILAND branch).  Read only by the drain above (POOL-15 scope).
bool W906_SecsAlarmShown_St02();

// ctest only: the requestId the mailbox should hold now, the memo text (golden moSecs->Lines, joined with CRLF), its line count,
// and everything back to the start state (hooks and the entry are kept).
std::string W906_SecsAlarmQid_St02();
std::string W906_SecsAlarmText_St02();
int         W906_SecsAlarmLineCount_St02();
void        W906_SecsAlarmReset_St02();

#endif // SECSALARMTIMER_ST02_H
