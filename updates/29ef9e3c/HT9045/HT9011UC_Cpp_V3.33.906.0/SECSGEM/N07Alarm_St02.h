// =============================================================================
//  N07Alarm_St02.h -- the N07 SECS/GEM disconnect alarm (JSCC NetworkMonitor, Steven 20260603).
//  AI(W906-C15-N07) 20261003 (St02-E helper), card ST02-C15.  Body: SECSGEM/N07Alarm_St02.cpp.
//
//  The two golden globals bN07AlarmActive / bN07BuzzerSilenced are declared where golden declares them, cmydef.h
//  (golden 906_0625_Steven cmydef.h:4326-4327); their definitions are in N07Alarm_St02.cpp.
// =============================================================================
#ifndef N07ALARM_ST02_H
#define N07ALARM_ST02_H

namespace ht9045 {

// One golden TfMain::Timer2Timer tick of the N07 block (golden 906_0625_Steven main.cpp:20937-20999).
// Called by the Timer2 slot of MainTimersSt02.cpp (golden's 1000 ms, its fShow / InitialOK / bTimer2Run guards there).
void W906_N07Timer2Tick_St02();

// ctest only: golden's two block statics (iN07SecCounter / bN07LastAlarm, :20939-20940) and both globals back to their
// start values (false / 0), as at program start.
void W906_N07Timer2Reset_St02();

// ctest only: golden's 10-tick counter (:20939), to show where the recheck grid is.
int W906_N07SecCounter_St02();

}  // namespace ht9045

#endif // N07ALARM_ST02_H
