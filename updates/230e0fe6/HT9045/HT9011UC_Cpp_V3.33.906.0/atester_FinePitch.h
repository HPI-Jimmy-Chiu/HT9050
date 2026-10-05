//---------------------------------------------------------------------------

#ifndef atester_FinePitchH
#define atester_FinePitchH
//---------------------------------------------------------------------------
extern int iTestYFinePitchTask;
//  ^ lines 1-6 = 910 atester_FinePitch.h:1-6 verbatim (origin/ref/frank-910-9050 2275f78c,
//    HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch/atester_FinePitch.h; its :7 is the #endif at the bottom).
//
//  AI(W906-ST01C) 20261005 (St01, SAFETY): NOT 910 below -- the declarations 910 keeps elsewhere, and the test seams.
//  The bodies are in atester_FinePitch.cpp (ht9045_sm).  ST01-C slice 1: NOTHING outside atester_FinePitch.cpp and
//  tests/ calls any of these (ctest FP9050_Index [F2] census); slice 2 switches the DoAllProcess Index slot
//  (csystem.cpp, 910 csystem.cpp:10391-10394) and declares DoTestHeadMotorFP block-scope there.
//  Steven 1005 23:1x: FinePitch-only parts are not ported -- InitTestYFPTask stays uncalled (Q1), the bEnableCalCCD CCD arm
//  is refused on HT9050 (「9050 的 CCD 不是做校正，CCD 段後續再改」).
#include "vclcompat/vcl_compat.h"                          // AnsiString (the W-44 hook below)
void DoTestHeadMotorFP();                                  // 910 csystem.h:9 (H094)
void InitTestYFPTask();                                    // 910 atester_FinePitch.cpp:1842 (910 has no caller: ledger s0 #5, plan Q1; not handled, Steven 1005 22:2x)
extern bool MoveIndexZ(int iPos);                          // 910 Motor/mymotor.h:337 (H001); body = atester_FinePitch.cpp EOF
//  the other 910 entry points (file scope in 910, declared here for the tests)
extern int iHangupCTArm;                                   // 910 :64
extern int iInitContactModeStartTask;                      // 910 :75
extern int iInitContactModeEndTask;                        // 910 :235
void InitContactModeStart();                               // 910 :76
bool DoTestZContactModeStart(int iMode);                   // 910 :81
void InitContactModeEnd();                                 // 910 :236
bool DoTestZContactModeEnd(int iMode, int iPos);           // 910 :241
bool DoFrontTestSuckICFP();                                // 910 :316
bool DoFrontTestDestroyICFP();                             // 910 :983
bool DoTestYFrontFP();                                     // 910 :1423
void DoTestYFinePitch();                                   // 910 :1847

//  NOT 910 helpers (atester_FinePitch.cpp EOF).  Live in a SHIP build, inert under SOFT_SIMULTE; never keyed on a card type
//  (Steven 1005 23:2x Q-R5): they read the Index Z1 motor object through its class interface.
//  P7 on the switch line of the 7 state machines: the motor object's alarm / servo state (+ E-042's 1203 health, reused) -> "ST" Z1 + message once,
//  true -> the caller takes its exit in the same tick.
bool W906_FP9050DriveFaultStop(const char* where);
//  W-44 (Steven 1005 23:1x): Index Z1 may press to the socket while the shuttles' X are OUTSIDE the index's safe zone. The decision
//  belongs to Frank's guard (FR-NB2 (2), RULINGS_20261005 #19), which is not in the tree yet: Frank's code registers
//      bool W906_Ht9050ShuttlesClearOfIndex(AnsiString* why);   // true = Z1 may press; why = the axis / position when not
//  here. Null (today) in a SHIP build = NOT clear (fail-safe); SOFT_SIMULTE = clear (910).
extern bool (*W906_Ht9050ShuttlesClearOfIndexHook)(AnsiString* why);
bool W906_FP9050ShuttlesClearOfIndex(AnsiString* why);     // the hook's answer with the rules above
//  W-44 entry (beside 910's DoTestYFrontFP case-130 gate): true = refused, message shown, nothing commanded.
bool W906_FP9050ShuttlesClearRefused(const char* where);
//  W-44 per tick: sm = ht9045::fp9050::kSmHeadMotor / kSmTestYFront; a press / hold task (W44PressTaskFP) while a shuttle
//  is inside the safe zone (or no guard) -> "ST" Z1 + message once, true -> the caller takes its exit.
bool W906_FP9050ShuttleClearStop(int sm, int task, const char* where);
//  F1: MOT[MTestZ1]'s position through its class API (Gali_ReadPos when the Index axes are on the Galil API, INDEX_MOTION_CARD==0;
//  910's ReadPos otherwise).
int  W906_FP9050Z1Pos();
//  Q4b: one refused MoveIndexZ descent (910's CCD-Y interlock); after 5 s of refusals one message, no motion.
void W906_FP9050CcdYHeld(int iPos);
void W906_FP9050CcdYHeldReset();                           // a MoveIndexZ that was not refused (also a test seam)
void W906_FP9050Reset();                                   // test seam: forget the W-44 / Q4b message latches
extern int W906_FP9050LeafCount;                           // test seam: leaf stops taken
#endif
