// ===========================================================================
//  FastClockJobs.cpp  --  AI(W906-FASTCLK) 20261003: what the serve loop's fast clock runs (RULINGS_20261002 #7, s0 #35 = A).
//
//  The scheduler is FastClock.h; the wb_serve glue that registers these and hooks the loop is FastClockWbServe.cpp.  This file
//  is in ht9045_sm (no wb_serve symbol), so ctests call the very bodies wb_serve runs.
//
//    job        period  golden                                                        body here
//    heater     20 ms   THeaterThread::Execute uHeaterThread.cpp:68-76 (Synchronize +   W906_FastClockHeaterBeat
//                       MySleepEx(20)); body HeaterThreadProcess :56-66; created
//                       main.cpp:22481, started FormShow :10138, ended FormClose :11634
//    bin panel  30 ms   TfMain::Timer1Timer (main.dfm:17272-17278 Interval 30), its      MainTimersSt02.cpp W906_St02Timer1BinFast
//                       NUMBER_PANEL_TYPE==2 segment :3439-3440 (St02 E-T1-022)
//    GM-2       30 ms   TfGroundMan::Timer1Timer GroundMan.cpp:551-575 (GroundMan.dfm:    W906_FastClockGroundManBeat
//                       1876-1881 Interval 30, Enabled by default; ctor :134 sets 30
//                       again); the form is created at start-up (HT9045.cpp:261)
//  (golden = HT9011UC_Code_V3.33.906.0_20260618; RULINGS_20261002 #20a "溫控必須參考 V912": V912's uHeaterThread.cpp:1-110,
//   which holds :56-99, is byte-identical to 906's after cp950 -> UTF-8, measured 20261003 with diff.)
//
//  THE HEATER BODY, per build.
//    SIM  -- St02's W906_HeaterSimTick (HeaterSimTick.cpp, S-12): golden :58-65 without DoThermo, which there would only time
//            out against controllers that do not exist.  Unchanged; only its caller moved from PumpTick (500 ms) to here.
//    SHIP -- golden :58-65 verbatim, DoThermo included.  On a machine that is the temperature-controller traffic and the heater
//            relay / fan outputs, every 20 ms, guarded by golden's own InitialOK (true from PumpInit = golden FormShow :10464;
//            false again from the Exit stop FileRW/MainClose.cpp Q44Issue = golden 906 FormClose :11468 (V912 :11953), which
//            comes before its relay Off -- so this body cannot switch the relay back on during the close; golden's
//            EndHeaterThread :11634 then stops a thread that is already idle).  The switch to turn it off is MachineType.h
//            W906_FASTCLK_HEATER.
//            WHAT TALKS TO WHAT is written next to that switch.
//
//  OWNERSHIP FLAGS.  The two jobs that already had a slow caller keep it unless the fast clock took them: PumpTick
//  (WebBridgeTags.cpp) skips W906_HeaterSimTick while W906_FastClockOwnsHeater(), and St02's dispatcher (MainTimersSt02.cpp)
//  skips its PumpTick-path Timer1BinTick while W906_FastClockOwnsBin().  Only FastClockWbServe.cpp sets them, so every ctest
//  (no fast clock) keeps the old callers -- and so would a wb_serve whose glue registered nothing.
// ===========================================================================
#include "MachineType.h"      // SOFT_SIMULTE -- first, or the #ifdef below would see it undefined
#include "cmydef.h"           // InitialOK
#include "csystem.h"          // CheckATC6System / HeaterDoorIsOpen / DoHeaterOn
#include "bthermo.h"          // DoThermo
#include "uHeaterThread.h"    // CheckHeater
#include "forms/fGroundMan.h" // fGroundMan (TfGroundMan::Timer1Timer)

namespace ht9045 {

void W906_HeaterSimTick();    // HeaterSimTick.cpp (St02-E, S-12)

namespace {
bool g_ownHeater = false;
bool g_ownBin    = false;
}

void W906_FastClockSetOwnsHeater(bool v) { g_ownHeater = v; }
bool W906_FastClockOwnsHeater()          { return g_ownHeater; }
void W906_FastClockSetOwnsBin(bool v)    { g_ownBin = v; }
bool W906_FastClockOwnsBin()             { return g_ownBin; }

// One beat of golden THeaterThread (header).
void W906_FastClockHeaterBeat()
{
#ifdef SOFT_SIMULTE
    W906_HeaterSimTick();                                                       // St02's SIM body of golden :56-66 (HeaterSimTick.cpp head)
#else
    if(InitialOK)                                                               // golden uHeaterThread.cpp:58
    {
        CheckATC6System();                                                      // :60  //ChungHung 20141024 add
        DoThermo();                                                             // :61  //jou 981209 any time need detect heater state
        HeaterDoorIsOpen();                                                     // :62  //jou 981013 : unify check heater doop sensor
        CheckHeater();                                                          // :63  //jou 981209 any time need detect heater state
        DoHeaterOn();                                                           // :64  //jou 981209 any time,也需偵測風扇是否轉動
    }
#endif
}

// One beat of golden TfGroundMan::Timer1 (header).  Timer1Timer keeps golden's own guards: InitialOK, the bRunTimer1 re-entry
// flag, and `USE_GROUND_MAN==1 && bRs232Ok` in front of DoGroundMasterMonitor -- which stays under GATE (GM-2) `#if 0`
// (forms/fGroundMan.cpp; bRs232Ok has no writer in the port, Init_GM_RS232 is not translated).  So today a beat is: the two
// guards, then labStatus->Caption = iGroundMasterTask (golden :572).
void W906_FastClockGroundManBeat()
{
    if (fGroundMan != 0)                                                        // [W906] golden's form always exists
        fGroundMan->Timer1Timer();
}

}  // namespace ht9045
