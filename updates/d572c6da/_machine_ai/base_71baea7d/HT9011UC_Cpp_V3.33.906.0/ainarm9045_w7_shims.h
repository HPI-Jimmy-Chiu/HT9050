// =============================================================================
//  ainarm9045_w7_shims.h  --  W7-A1 in-arm SHUTTLE-MOTION seam declarations
//
//  Translation wave: W7-A1 (un-gate the in-arm shuttle floating/latch PAIR in
//                    ainarm9045.cpp -- DoInArmCheckShuttleFloating +
//                    CheckInShuttleSensor_Latch).
//  Integrate step: AI(W7-A1-INTEGRATE) 20260629
//
//  PURPOSE
//  -------
//  The two un-gated in-arm SMs reference SUBSTRATE symbols whose real home is the
//  SHUTTLE-motion engine (golden ainarm2.h), which has NOT been translated yet.
//  The translator left #ifndef-guarded forward-decls inline in ainarm9045.cpp and
//  REPORTED them in seamNeeds.  This header is the real declaration home for those
//  symbols, and the companion ainarm9045_w7_shims.cpp carries faithful OFFLINE
//  bodies (shake/knock/vibrate converge immediately on the Sim HAL -> true;
//  RecordShtSuperflous is a log no-op; IndexZCanMove[2]/bShuttleKnock are plain
//  globals).
//
//  By defining AINARM_W7A1_SHUTTLEMOTION_FWD BEFORE ainarm9045.cpp's own inline
//  forward-decl block, including this header from ainarm9045.cpp SUPPRESSES that
//  block (the engine's #ifndef guard is satisfied) and these decls win -- so there
//  is a single declaration home and zero ODR risk.
//
//  IsTestZ1NotSafeShuttle1CanNotMove / IsTestZ2NotSafeShuttle2CanNotMove:
//    DECLARED here (golden ainarm9045.cpp:3759-3760 forward-declares them itself,
//    no header) but their real DEFINITION lives in acarry.cpp (confirmed:
//    acarry.cpp:3214 / :5195).  This header declares ONLY -- it must never define
//    them (ODR).
//
//  Big5: no Chinese in this file; ZERO U+FFFD.
// =============================================================================
#ifndef ainarm9045_w7_shimsH
#define ainarm9045_w7_shimsH

#include "vclcompat/vcl_compat.h"   // (none needed directly, but keeps the AnsiString-free decls portable)

// Suppress ainarm9045.cpp's inline #ifndef AINARM_W7A1_SHUTTLEMOTION_FWD block so
// THIS header is the single declaration home for the shuttle-motion seam symbols.
#ifndef AINARM_W7A1_SHUTTLEMOTION_FWD
#define AINARM_W7A1_SHUTTLEMOTION_FWD

// ---- SHUTTLE-motion engine helpers (golden ainarm2.h) -----------------------
//  Offline (defs in ainarm9045_w7_shims.cpp): the Sim HAL has no real shuttle
//  shake/knock/vibrate motor, so each helper converges immediately -> returns
//  true ("done") so the floating SM's shake/knock/vibrate loop advances.
extern bool DoShakeShuttle(int iShuttle, bool bNeedInitial=false);              //Steven 20120801   // golden ainarm2.h:189
extern bool DoKnockShuttle(int iShuttle, bool bNeedInitial=false);             //Jou 2013-03-08     // golden ainarm2.h:190
extern bool DoKnockShuttleFirst(int iShuttle, bool bNeedInitial=false);        //jou 2015-12-09     // golden ainarm2.h:191
extern bool DoVibrateShuttle(int iShuttle, bool bNeedInitial=false);          //JerryYang 20190123  // golden ainarm2.h:214

// RecordShtSuperflous (golden ainarm2.h:218): per-shuttle "device was corrected
// by vibrate/shake" log entry.  Offline: log no-op.
extern void RecordShtSuperflous(int iShuttle);                                 //JerryYang 20181121  // golden ainarm2.h:218

// IndexZCanMove[2] (golden ainarm2.h:48): per-index Z-can-move interlock flags;
// floating case 9000 sets both true.  Plain global bool[2] (def in the .cpp).
extern bool IndexZCanMove[2];                                                  // golden ainarm2.h:48

// bShuttleKnock (golden ainarm2.h:85): shuttle-knock-in-progress flag; read at
// floating case 1411, reset false at case 9000.  Plain global (def in the .cpp).
extern bool bShuttleKnock;                                                     //Jou 2013-03-08      // golden ainarm2.h:85

// ---- Index-safe / shuttle-can-move interlocks -------------------------------
//  DEFINED in acarry.cpp (acarry.cpp:3214 / :5195).  DECLARED here only -- never
//  define (ODR).  Golden ainarm9045.cpp:3759-3760 forward-declares these itself.
extern bool IsTestZ1NotSafeShuttle1CanNotMove(int &iRetryCT);
extern bool IsTestZ2NotSafeShuttle2CanNotMove(int &iRetryCT);

#endif // AINARM_W7A1_SHUTTLEMOTION_FWD

#endif // ainarm9045_w7_shimsH
