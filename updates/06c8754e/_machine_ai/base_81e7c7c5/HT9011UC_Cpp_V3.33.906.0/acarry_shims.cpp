// =============================================================================
//  acarry_shims.cpp  --  definitions for the W6.5 SHUTTLE/CARRY engine offline shims
//
//  Translation wave: W6.5 (SHUTTLE/CARRY ENGINE -- acarry.cpp)
//  Translator: AI(W6.5-CARRY) 20260626
//
//  All bodies are offline-safe (false / 0 / no-op / empty grid) so the shuttle
//  state machines pump over the Sim HAL and take the deterministic canary
//  branches (no floating, no HTTP fail, no 9046AU sort-arm, no ATC flood-gate,
//  no latch events).  See acarry_shims.h for the golden-home documentation.
//
//  Big5: Chinese comments decoded via cp950, preserved UTF-8.  ZERO U+FFFD.
// =============================================================================
#include "acarry_shims.h"
#include "csystem.h"      // InSHT1InLF / InSHT2InLF / InSHT1InRT / InSHT2InRT (real predicates)
#include "cmydef.h"       // K_RESET (not used here, but keeps constant set consistent)
#include <cstring>

// ---------------------------------------------------------------------------
//  TECH Tech -- AI(W906-GA1-B1) 20260804: shim ctor+definition RETIRED; the
//  real definition lives in LastSet.cpp (golden LastSet.cpp).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  fLtcSensor (TfLtcSensor)  -- golden LtcSensor.h.  All latch counts = 0 offline.
// ---------------------------------------------------------------------------
TfLtcSensor::TfLtcSensor()
{
    LatchDataCnt0 = LatchDataCnt1 = LatchDataCnt2 = LatchDataCnt3 = LatchDataCnt4 = 0;
    LatchDataCnt5 = LatchDataCnt6 = LatchDataCnt7 = LatchDataCnt8 = LatchDataCnt9 = 0;
    std::memset(LatchDataTable0, 0, sizeof(LatchDataTable0));
    std::memset(LatchDataTable1, 0, sizeof(LatchDataTable1));
    std::memset(LatchDataTable2, 0, sizeof(LatchDataTable2));
    std::memset(LatchDataTable3, 0, sizeof(LatchDataTable3));
    std::memset(LatchDataTable4, 0, sizeof(LatchDataTable4));
    std::memset(LatchDataTable5, 0, sizeof(LatchDataTable5));
    std::memset(LatchDataTable6, 0, sizeof(LatchDataTable6));
    std::memset(LatchDataTable7, 0, sizeof(LatchDataTable7));
    std::memset(LatchDataTable8, 0, sizeof(LatchDataTable8));
    std::memset(LatchDataTable9, 0, sizeof(LatchDataTable9));
}
int  TfLtcSensor::GetLtcSensor(int /*iWhich*/)   { return 0; }   // offline: no latch read
void TfLtcSensor::SetLtcSensor(int /*iWhich*/)   {}              // offline: no-op
void TfLtcSensor::ClearLtcSensor(int iWhich)
{
    // offline: clear the matching latch count (defensive; counts are already 0)
    switch(iWhich)
    {
        case 0: LatchDataCnt0=0; break;
        case 1: LatchDataCnt1=0; break;
        case 2: LatchDataCnt2=0; break;
        case 3: LatchDataCnt3=0; break;
        case 4: LatchDataCnt4=0; break;
        case 5: LatchDataCnt5=0; break;
        case 6: LatchDataCnt6=0; break;
        case 7: LatchDataCnt7=0; break;
        case 8: LatchDataCnt8=0; break;
        case 9: LatchDataCnt9=0; break;
        default: break;
    }
}
TfLtcSensor *fLtcSensor = new TfLtcSensor();            // golden LtcSensor.h

// ---------------------------------------------------------------------------
//  OutSht3Kit (TMyKitSuck)  -- golden MyKitSuck.h (9046AU sort kit).  Sole def.
// ---------------------------------------------------------------------------
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 有 golden 定義，本顆起註冊；兩份會重複定義）：TMyKitSuck OutSht3Kit;                                  // golden MyKitSuck.h

// ---------------------------------------------------------------------------
//  ATC_InterfaceForm / ATC_Data  -- golden ATC_Handler_Side.h.  No ATC offline.
// ---------------------------------------------------------------------------
TATC_InterfaceFormShim::TATC_InterfaceFormShim() : iATC_MODE_TYPE(0) {}
TATC_InterfaceFormShim *ATC_InterfaceForm = new TATC_InterfaceFormShim();
TATC_DataShim::TATC_DataShim() : dChillerSetTemp(0.0) {}
TATC_DataShim ATC_Data;

// ---------------------------------------------------------------------------
//  Shuttle-floating detectors (golden LaserSensorShuttle.h:16-18).
//  AI(W906-PT-W3-integrate) 20260808: the three offline stubs that lived here are
//  RETIRED -- the real bodies landed with OmronLaser/LaserSensorShuttle.cpp in
//  PT-W3 (:656 / :1070 / :1368).  See acarry_shims.h for the full account,
//  including why only ONE of the three announced itself as a link error and how
//  the other two had been quietly winning over the real engine.
//  BEHAVIOUR CHANGE, and it is a real one -- stated as what it IS, not as a
//  reassurance: acarry.cpp's floating checks used to return `false` immediately,
//  unconditionally.  They now enter golden's real multi-step state machine
//  (LaserSensorShuttle.cpp:656 -- `iShtLaserCheckTask` cases that command
//  MOT[MInShuttle1+iSht].MotorMove(...) and read the laser between steps), so a
//  caller no longer gets an answer on the first tick.  Its offline outcome is
//  whatever the Sim HAL's motor + laser stand-ins produce and this wave does NOT
//  pin that down; the ctest set is the measurement.  Note also that the second
//  argument means RESET in golden (`bReset`), not "alarm" as the old shim's
//  parameter name claimed -- acarry.cpp:4335/:6161 pass true, which golden treats
//  as "repaint every cell NeedCheck, zero the retry count, restart at Task 1".
//  The shim ignored it entirely.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  In-shuttle LF/RT (int) overloads (golden csystem.cpp) -- route to the real
//  (void) predicates already implemented in csystem_predicates.cpp.
// ---------------------------------------------------------------------------
#if 0   // PT-W5c RETIRED (InShtInLF)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InShtInLF(int iSht) { return (iSht==0) ? InSHT1InLF() : InSHT2InLF(); }     // golden csystem.cpp
#endif
#if 0   // PT-W5c RETIRED (InShtInRT)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InShtInRT(int iSht) { return (iSht==0) ? InSHT1InRT() : InSHT2InRT(); }     // golden csystem.cpp
#endif

// ---------------------------------------------------------------------------
//  9046AU sort-shuttle position predicates (golden csystem.cpp) -- offline false.
// ---------------------------------------------------------------------------
#if 0   // PT-W5c RETIRED (SortSHTInLF)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool SortSHTInLF() { return false; }                   // golden csystem.cpp (9046AU)
#endif
#if 0   // PT-W5c RETIRED (SortSHTInRT)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool SortSHTInRT() { return false; }                   // golden csystem.cpp (9046AU)
#endif

// ---------------------------------------------------------------------------
//  In-shuttle step / residual / prominent checks (golden csystem.cpp).
//   - DoStepShuttleCheck: offline no step pending -> true (proceed).
//   - EmptyIC / MustHasIC / Prominent: offline find no anomaly -> true (OK).
// ---------------------------------------------------------------------------
#if 0   // PT-W5c RETIRED (DoStepShuttleCheck)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool DoStepShuttleCheck(int /*iShuttle*/) { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle1EmptyIC)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "no anomaly found, proceed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle1EmptyIC()             { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle2EmptyIC)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "no anomaly found, proceed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle2EmptyIC()             { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle1MustHasIC)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "check passed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle1MustHasIC()           { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle2MustHasIC)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "check passed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle2MustHasIC()           { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle1Prominent)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "no anomaly found, proceed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle1Prominent()           { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (DoCheckShuttle2Prominent)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "no anomaly found, proceed" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoCheckShuttle2Prominent()           { return true; }     // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle1MustHasICTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle1MustHasICTask()           {}          // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle2MustHasICTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle2MustHasICTask()           {}          // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle1ProminentNoHasICTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle1ProminentNoHasICTask()    {}          // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle2ProminentNoHasICTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle2ProminentNoHasICTask()    {}          // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle1ProminentTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle1ProminentTask()           {}          // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (InitialCheckShuttle2ProminentTask)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void InitialCheckShuttle2ProminentTask()           {}          // golden csystem.cpp
#endif

// ---------------------------------------------------------------------------
//  32-site enable predicates (golden csystem.cpp) -- offline false.
// ---------------------------------------------------------------------------
#if 0   // PT-W5f RETIRED (Check32siteOnlyEnabled24Site)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
bool Check32siteOnlyEnabled24Site() { return false; }  // golden csystem.cpp
#endif
#if 0   // PT-W5f RETIRED (Check32siteOnlyEnabled4x4Site)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
bool Check32siteOnlyEnabled4x4Site(){ return false; }  // golden csystem.cpp
#endif

// ---------------------------------------------------------------------------
//  out-arm let-shuttle-pass / bottom-2DID HTTP / sort-arm safe-move.
//   - MoveOutArmXY_ToShuttleAlarmArea: offline out-arm already away -> true.
//   - SendHTTPRequest: offline server OK -> 0 (NOT -1, so no retry loop).
//   - MoveSortArmToAutoSafe: 9046AU sort-arm -> no-op offline.
// ---------------------------------------------------------------------------
// AI(W906-PT-W4-integrate) 20260809: 5 STAND-IN DEFINITION(S) RETIRED FROM HERE.
//   aoutarm.cpp (golden's own home for all of them) landed in wave PT-W4 and is
//   registered in ht9045_sm, so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of ...`. The linker named
//   each one, which is also the proof the signatures match exactly -- a decorated-name
//   collision cannot happen otherwise.
//   Retired here: MoveOutArmXY_ToShuttleAlarmArea, MoveOutArmToAutoSafe, bCarryControlOutarm1, bCarryControlOutarm2, bPickShuttleError
//   BEHAVIOUR: these were offline defaults (return true/false/0/no-op); the real bodies
//   run golden's actual logic, so out-arm paths that used to short-circuit now execute.
//   That is the point of the wave, and it is why this wave was measured on its own.
int  SendHTTPRequest(int /*iSht*/)     { return 0; }       // golden main -- 0 == OK
#if 0   // PT-W5c RETIRED (MoveSortArmToAutoSafe)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
void MoveSortArmToAutoSafe()           {}                  // golden 9046AU
#endif

// ---------------------------------------------------------------------------
//  Do_Auto_SHT1AsOutArmIsRotater -- golden acarry.h decl-only (no .cpp body).
//  Inert active stub so the declaration resolves.
// ---------------------------------------------------------------------------
void Do_Auto_SHT1AsOutArmIsRotater() {}                    // golden acarry.h decl-only

// ---------------------------------------------------------------------------
//  out-arm cross-module safe-move + offline timing + system-NG / 2x8 step.
//   - MoveOutArmToAutoSafe: offline the out-arm Z reaches the safe position
//     immediately -> true (same posture as aoutarm9045.cpp's file-local variant).
//   - MySleepEx / MySleep: offline no real OS wait; MySleepEx returns 0 (the
//     WAIT_OBJECT_0 "completed" code the kept-verbatim thread/step loops ignore).
//   - SystemNG: false offline (system not in NG -> the SM guards proceed).
//   - iCloseSiteStep_2x8: 0 offline (2x8 close-site geometry baseline).
// ---------------------------------------------------------------------------
// AI(W906-CommonCompletion) 20260721: MySleepEx/MySleep stand-in DEFINITIONS
// REMOVED -- common.cpp now provides the real bodies (golden common.h:260-261
// un-gated this wave), and this file's own header (acarry_shims.h:230-232)
// already declares these with the exact same signature, citing "golden
// common.h:260/261" -- so removing just the definitions here needs zero
// caller changes; the real common.cpp definitions now satisfy every caller
// that previously linked against these offline stand-ins.
extern bool SystemNG;                             // golden main-side global  //AI(W906-HALARM) 20260926: 定義搬到 HAlarm.cpp（golden HAlarm.cpp:14 `bool SystemNG;`，halarm.h:31 extern）；這裡改成宣告，否則兩個定義
//AI(W6.2c-INARM-batch4) 20260626: the placeholder DEFINITION of iCloseSiteStep_2x8
// (was =0) was REMOVED -- ainarm9045_2x8_8.cpp now owns the REAL def (=0, golden
// ainarm9045_2x8_8.cpp:110).  The extern decl in acarry_shims.h:234 still satisfies
// acarry.cpp.  Value identical (0) so numeric behavior is unchanged.

// ---------------------------------------------------------------------------
//  Cross-module GLOBALS + LOG functions owned by aoutarm.cpp / cpublic.cpp /
//  main.cpp (no translated home this wave).  Offline-safe defaults matching the
//  golden initializers (aoutarm.cpp:67-70 all false) + no-op logging.
//    * bCarryControlOutarm1/2 -- golden aoutarm.cpp:67-68 (carry-control hand-off
//      flags; the carry engine WRITES them, the out-arm READS them).  false.
//    * bPickShuttleError       -- golden aoutarm.cpp:70 (shuttle-pick error latch).
//    * OutShuttleLog           -- golden cpublic.cpp:679 (fMain-UI shuttle-sensor
//      log; gated #if 0 in cpublic.cpp -> own an offline no-op here).
//    * LogSoftwareOffTime      -- golden main.cpp (software-off-time logger).
// ---------------------------------------------------------------------------
void OutShuttleLog(bool /*bFlag*/)            {}    // golden cpublic.cpp:679 -- offline no-op
void LogSoftwareOffTime(AnsiString /*Flag*/)  {}    // golden main.cpp -- offline no-op
