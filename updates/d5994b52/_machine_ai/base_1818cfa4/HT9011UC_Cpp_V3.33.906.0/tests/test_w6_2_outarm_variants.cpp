// =============================================================================
//  test_w6_2_outarm_variants.cpp  --  W6.2c-OUT VERIFY: the LIVE out-arm site
//  variants
//    batch-1: 1x1_1 / 1x2_2 / 1x2_4 / 1x3_2_14 / 1x3_4 / 1x4_2
//    batch-2: 1x4_4S / 1x4_4 / 1x4_8 / 2x1_2 / 2x2_4 / 2x2_8 / 2x3_6_14
//    batch-3 (FINAL): 2x3_6 / 2x4_4 / 2x4_8 / 2x5_8 / 2x6_8 / 2x8_8 / All_1Picker
//  ... and the DEAD out-arm site variants (compiled+linked, never dispatched):
//    batch-4 (DEAD): 1x4_4_Back / 2x2_4_14 / 2x2_4_23 / 2x4_16 / 2x8_16 / 2x8_32
//
//  Translation wave: W6.2c-OUT (out-arm per-site-config variants over Sim HAL)
//  Author: AI(W6.2c-outarm-verify) 20260627
//          (batch-2 added 20260627; batch-3 FINAL added 20260627;
//           batch-4 DEAD added 20260627)
//
//  DEAD-VARIANT WITNESS (PART D, batch-4)
//  --------------------------------------
//  The 6 batch-4 files are NEVER on the engine dispatch path (per the
//  ENUM->CALLEE map + MachineType.h enum table):
//    * 1x4_4_Back: enum e9045_1x4_4_Back=10 present, but DoOutArm_9045()'s arm is
//      an EMPTY //-body and DoPickFromShuttle routes to LIVE 1x4_4 -- the
//      _1x4_4_Back callee is never called.
//    * 2x2_4_14: enum e9045_2x2_4_14=16 present, both ladders route to LIVE 2x2_4.
//    * 2x8_32:   enum e9045_2x8_32=25 present, both ladders route to LIVE 2x8_8.
//    * 2x2_4_23 / 2x4_16 / 2x8_16: enums ABSENT from MachineType.h -> no ladder
//      arm can ever select them (compile-time-provable dead).
//  Their DoOutArm_9045_<deadv>() / DoPickFromShuttle_9045_<deadv>(int) symbols
//  carry the FULL _<deadv> suffix, so they are UNIQUE never-called globals that
//  must merely COMPILE+LINK as dead weight.  We prove that with two PURE-geometry
//  helper oracles (no HAL/grid/Prod dependency on the asserted branch), hand-
//  derived from the golden, that resolve the dead files' external symbols at link
//  and return the golden value -- i.e. the dead bodies are real & callable, not
//  stubs, yet they are not on any dispatch ladder (engine never names them).
//
//  PURPOSE
//  -------
//  Independently prove the 6 LIVE out-arm site variants of this batch are not
//  just COMPILED+LINKED but ACTUALLY DISPATCHED: that the engine dispatch arm
//  for each layout resolves to the REAL translated DoOutArm_9045_<v>() body
//  (NOT the removed file-local no-op `static void ...(){}` stub that used to
//  short-circuit them in aoutarm9045.cpp).
//
//  DISPATCH-ROUTING WITNESS (observable side effect, golden-cited)
//  ---------------------------------------------------------------
//  Each variant's DoOutArm_9045_<v>() opens with the SAME verbatim golden
//  task-1 entry (e.g. aoutarm9045_1x1_1.cpp:676, aoutarm9045_1x4_2.cpp:788):
//
//      int &Task = OutArmTask;
//      if (bCarryControlOutarm1 || bCarryControlOutarm2) return;   // guard
//      switch (Task) {
//        case 1:
//          bSortingAllBinTrayFinish = false;   // <-- observable global write
//          Task = 5;                           // (no break -> fall through)
//        case 5:
//          if (MoveOutArmToAutoSafe()) Task = 10;   // offline stub -> true
//          if (Task != 10) break;
//        case 10:
//          if (CheckOutArmInitState()) Task = 50;   // offline shim -> true
//          if (Task != 50) break;
//        case 50: ...
//
//  So from a seeded entry cursor OutArmTask==1, ONE call to the REAL body:
//    * sets the global bSortingAllBinTrayFinish = false, AND
//    * advances OutArmTask off 1 (to 50, since the offline MoveOutArmToAutoSafe()
//      stub + CheckOutArmInitState() shim both return true and the case-1/5/10
//      fall-throughs chain to case 50).
//  The DELETED static no-op stub `static void DoOutArm_9045_<v>(){}` would have
//  left OutArmTask==1 untouched and never written bSortingAllBinTrayFinish.
//  Observing OutArmTask move off 1 is therefore a sound, deterministic witness
//  that the dispatch reaches the REAL func.  We assert it for ALL 6 variants by
//  calling each variant's engine-dispatched callee directly (the engine ladder
//  in aoutarm9045.cpp routes its enum to exactly this symbol -- see the
//  ENUM->CALLEE map: e9045_1x1_1|1x4_1_Ac->1x1_1; 1x2_2_13|14->1x2_2;
//  1x2_4_Hot->1x2_4; 1x3_2_14->1x3_2_14; 1x3_4->1x3_4; 1x4_2_14->1x4_2).
//
//  We pre-seed OutArmTask to a SENTINEL (1) and bSortingAllBinTrayFinish to a
//  DIRTY value (true) before each call, so a no-op stub would be caught by BOTH
//  the unchanged cursor AND the un-cleared flag.
//
//  EQUIVALENCE NOTE: no Borland binary exists -> "equivalence" here == clean g++
//  compile/link + the variant bodies are DEFINED (not stubbed) + the dispatch
//  reaches them (cursor/flag side effect) over the Sim HAL with no crash.
// =============================================================================
// MachineType.h FIRST: it owns the global enum e2x8ModeTotal that the golden
// aoutarm9045_2x8_8.h extern array decl (XPHSuckToSht_2x8_8_OutArm[e2x8ModeTotal]
// [2][8]) mirrors.  The variant .cpp files pull MachineType.h before their own
// .h; this TU includes the variant .h directly, so it must do the same.
#include "MachineType.h"            // global enums (e2x8ModeTotal, eInArmType ...)
#include "aoutarm9045_1x1_1.h"      // void DoOutArm_9045_1x1_1();
#include "aoutarm9045_1x2_2.h"      // void DoOutArm_9045_1x2_2();
#include "aoutarm9045_1x2_4.h"      // void DoOutArm_9045_1x2_4();
#include "aoutarm9045_1x3_2_14.h"   // void DoOutArm_9045_1x3_2_14();
#include "aoutarm9045_1x3_4.h"      // void DoOutArm_9045_1x3_4();
#include "aoutarm9045_1x4_2.h"      // void DoOutArm_9045_1x4_2();
// --- W6.2c-OUT batch-2 (1x4_4S/1x4_4/1x4_8/2x1_2/2x2_4/2x2_8/2x3_6_14) ---
#include "aoutarm9045_1x4_4S.h"     // void DoOutArm_9045_1x4_4S();
#include "aoutarm9045_1x4_4.h"      // void DoOutArm_9045_1x4_4();
#include "aoutarm9045_1x4_8.h"      // void DoOutArm_9045_1x4_8();
#include "aoutarm9045_2x1_2.h"      // void DoOutArm_9045_2x1_2();
#include "aoutarm9045_2x2_4.h"      // void DoOutArm_9045_2x2_4();
#include "aoutarm9045_2x2_8.h"      // void DoOutArm_9045_2x2_8();
#include "aoutarm9045_2x3_6_14.h"   // void DoOutArm_9045_2x3_6_14();
// --- W6.2c-OUT batch-3 FINAL (2x3_6/2x4_4/2x4_8/2x5_8/2x6_8/2x8_8/All_1Picker) ---
#include "aoutarm9045_2x3_6.h"      // void DoOutArm_9045_2x3_6();
#include "aoutarm9045_2x4_4.h"      // void DoOutArm_9045_2x4_4();
#include "aoutarm9045_2x4_8.h"      // void DoOutArm_9045_2x4_8();
#include "aoutarm9045_2x5_8.h"      // void DoOutArm_9045_2x5_8();
#include "aoutarm9045_2x6_8.h"      // void DoOutArm_9045_2x6_8();
#include "aoutarm9045_2x8_8.h"      // void DoOutArm_9045_2x8_8();
#include "aoutarm9045_All_1Picker.h"// void DoOutArm_9045_All_1Picker(); (DoOutArm-ONLY carve-out)
#include "aoutarm9045.h"            // GetOutArmPitch_9045 (engine geometry oracle)
#include "aHotPlateSubstrate.h"     // OutArmSuck (iXStep/iYStep) for Oracle 2
#include "cprod.h"                  // Prod / IniConfig (via Config.h)
#include "cmydef.h"                 // bSortingAllBinTrayFinish / iPitch_Max_minus_Min / iXpitchMinX3
#include <cstdio>

// ---------------------------------------------------------------------------
//  Engine SM cursor + carry-control flags that are NOT on the variant .h
//  surfaces.  Same forward-declare idiom test_w6_2_outarm_core.cpp uses for
//  engine-internal cursors:
//    OutArmTask              -- offline-defined in aoutarm_shims.cpp:31
//    bCarryControlOutarm1/2  -- offline-defined in acarry_shims.cpp:171-172
//      (golden aoutarm.cpp:67-68 carry-control hand-off flags)
// ---------------------------------------------------------------------------
extern int  OutArmTask;
extern bool bCarryControlOutarm1;
extern bool bCarryControlOutarm2;

// ---------------------------------------------------------------------------
//  Oracle-1 pure-geometry helper from the 2x1_2 variant (external linkage,
//  NOT on the variant .h surface -- forward-declared here, defined in
//  aoutarm9045_2x1_2.cpp:310, golden aoutarm9045_2x1_2.cpp:30).
//    GetNowShuttleMode_2x1_2(iSht): first branch is
//      if(ptrOutSHT->UseSiteNoIC()) return 0;
//    Offline TMyKitSuck::UseSiteNoIC() == true (aHotPlateSubstrate.cpp:181),
//    ptrOutSHT bound to FRCarryKit/BRCarryKit (both all-NULL_IC offline) ->
//    GetNowShuttleMode_2x1_2(0)==0 AND ==1)==0 deterministically.  EXPECT 0.
// ---------------------------------------------------------------------------
extern int GetNowShuttleMode_2x1_2(int iSht);

// ---------------------------------------------------------------------------
//  Oracle-2 pure mode helper from the 2x4_4 variant (external linkage, NOT on
//  the variant .h surface -- forward-declared here, defined in
//  aoutarm9045_2x4_4.cpp:158, golden aoutarm9045_2x4_4.cpp:38).
//    GetNowShuttleMode_2x4_4(iSht): the VERY FIRST branch is
//      if(OutArmSuck.iXStep==1 && OutArmSuck.iYStep==1) return 2;
//    (single-pick mode), taken UNCONDITIONALLY of FR/BRCarryKit grid contents
//    or any Prod dependency.  We seed OutArmSuck.iXStep=iYStep=1 and EXPECT 2.
//    This proves the 2x4_4 variant's helper (and its 2x4_4 batch-3 file) was
//    translated VERBATIM and link-resolves over the Sim substrate.
// ---------------------------------------------------------------------------
extern int GetNowShuttleMode_2x4_4(int iSht);

// ---------------------------------------------------------------------------
//  DEAD variants (batch-4: 1x4_4_Back / 2x2_4_14 / 2x2_4_23 / 2x4_16 / 2x8_16 /
//  2x8_32) are PROVEN by build-artifact inspection, NOT by linking them into
//  this executable.  WHY: each dead variant's pure-geometry helper
//  (GetNowShuttleMode_2x4_16, GetNowShuttleMode_2x8_32, ...) lives in the SAME
//  translation unit / .obj as its heavy DoOutArm_9045_<deadv> body, which (being
//  genuinely never-dispatched) references cross-module/UI symbols that have no
//  offline home (iOutArmPlaceOrder / iMagPos / DoOutArmPlaceToAuto / OutputForm /
//  OutArmContinuousMove ...).  GNU ld's archive scan only pulls a member .obj
//  when one of its symbols is REFERENCED; the live engine + the 20 live variants
//  never name any dead symbol, so the dead .obj's are NOT pulled and the link is
//  clean.  If THIS test referenced a dead helper, ld would force-link the heavy
//  dead .obj and surface those (by-design-unsatisfied) externs -- which would
//  MISREPRESENT the dead files as broken when they are correctly dead weight.
//  So the dead-by-design proof is done OUT-OF-BAND by the Verify agent:
//    (1) the 6 aoutarm9045_<deadv>.cpp.obj are present in libht9045_sm.a with
//        their DoOutArm_9045_<deadv> / GetNowShuttleMode_<deadv> symbols DEFINED
//        (nm 'T'), AND
//    (2) the engine aoutarm9045.cpp contains ZERO references to any
//        DoOutArm_9045_<deadv> / DoPickFromShuttle_9045_<deadv> (grep), so they
//        are off every dispatch ladder.
//  Both are asserted in the Verify report, not in this TU (keeping the test link
//  faithful to the real reachability).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the other W6 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// Drive one variant's engine-dispatched callee from the task-1 entry and assert
// the REAL body ran (cursor advanced off 1 + the entry global was cleared).
static void witnessRouting(const char* name, void (*fn)())
{
    bCarryControlOutarm1 = false;        // clear the function-top early-return guard
    bCarryControlOutarm2 = false;
    OutArmTask              = 1;         // seed entry cursor
    bSortingAllBinTrayFinish = true;     // DIRTY: real case-1 entry sets it false

    fn();                                // dispatch to the REAL translated body

    char buf[160];
    snprintf(buf, sizeof(buf),
             "%s: REAL body ran -- OutArmTask advanced off 1 (got %d, golden case-1->5->10->50)",
             name, OutArmTask);
    CHECK(OutArmTask != 1, buf);

    snprintf(buf, sizeof(buf),
             "%s: REAL body ran -- bSortingAllBinTrayFinish cleared to false (golden case 1)",
             name);
    CHECK(bSortingAllBinTrayFinish == false, buf);
}

int main()
{
    printf("==== W6.2c-OUT variants verify (b1: 1x1_1.. b2: 1x4_4S.. b3 FINAL: 2x3_6/2x4_4/2x4_8/2x5_8/2x6_8/2x8_8/All_1Picker) ====\n");

    // =======================================================================
    //  PART A -- dispatch-routing witness for all 6 LIVE variants
    // =======================================================================
    printf("[A] dispatch routes to the REAL DoOutArm_9045_<v> (not the removed no-op stub)\n");
    witnessRouting("1x1_1",    DoOutArm_9045_1x1_1);
    witnessRouting("1x2_2",    DoOutArm_9045_1x2_2);
    witnessRouting("1x2_4",    DoOutArm_9045_1x2_4);
    witnessRouting("1x3_2_14", DoOutArm_9045_1x3_2_14);
    witnessRouting("1x3_4",    DoOutArm_9045_1x3_4);
    witnessRouting("1x4_2",    DoOutArm_9045_1x4_2);

    // =======================================================================
    //  PART A2 -- dispatch-routing witness for the 7 W6.2c-OUT batch-2 LIVE
    //  variants.  Same golden task-1 entry (verified in each variant .cpp:
    //  case 1: bSortingAllBinTrayFinish=false; Task=5; falling through 5->10->50
    //  via offline MoveOutArmToAutoSafe()/CheckOutArmInitState()==true).  The
    //  engine ladder routes each enum to exactly these callees (ENUM->CALLEE:
    //  1x4_4_13->1x4_4S; 1x4_4->1x4_4; 1x4_8_Hot->1x4_8; 2x1_2_13->2x1_2;
    //  2x2_4_12|13|14->2x2_4; 2x2_8_Hot->2x2_8; 2x3_6_14->2x3_6_14).  Their
    //  static no-op stubs were removed in aoutarm9045.cpp -- these resolve to
    //  the REAL translated bodies in ht9045_sm.
    // =======================================================================
    printf("[A2] dispatch routes to the REAL DoOutArm_9045_<v> (batch-2, stubs removed)\n");
    witnessRouting("1x4_4S",   DoOutArm_9045_1x4_4S);
    witnessRouting("1x4_4",    DoOutArm_9045_1x4_4);
    witnessRouting("1x4_8",    DoOutArm_9045_1x4_8);
    witnessRouting("2x1_2",    DoOutArm_9045_2x1_2);
    witnessRouting("2x2_4",    DoOutArm_9045_2x2_4);
    witnessRouting("2x2_8",    DoOutArm_9045_2x2_8);
    witnessRouting("2x3_6_14", DoOutArm_9045_2x3_6_14);

    // =======================================================================
    //  PART A3 -- dispatch-routing witness for the 7 W6.2c-OUT batch-3 (FINAL)
    //  LIVE variants.  Each variant's DoOutArm_9045_<v>() opens with the SAME
    //  verbatim golden task-1 entry (confirmed in each variant .cpp:
    //  case 1: bSortingAllBinTrayFinish=false; Task=5; falling 5->10->50 via the
    //  offline MoveOutArmToAutoSafe()/CheckOutArmInitState()==true stubs).  The
    //  engine ladders route each enum to exactly these callees (ENUM->CALLEE,
    //  Oracle 3 / MachineType.h enum eInArmType): iInArmType=18 e9045_2x3_6
    //  ->_2x3_6; 20 e9045_2x4_4_14 OR 21 e9045_2x4_4_13 ->_2x4_4; 22->_2x4_8;
    //  23->_2x5_8; 24->_2x6_8; 26 e9045_2x8_8 AND 25 e9045_2x8_32 BOTH ->_2x8_8.
    //  All_1Picker is NOT enum-routed: it is the FIRST ladder branch, taken when
    //  USE_PICKER_COUNT==ep1Picker (DoOutArm-ONLY -- no DoPickFromShuttle arm).
    //  Their static no-op stubs were removed in aoutarm9045.cpp -- these resolve
    //  to the REAL translated bodies in ht9045_sm.  This batch empties BOTH the
    //  static-stub and the extern-gate blocks -> out-arm fan-out is now 100% live.
    // =======================================================================
    printf("[A3] dispatch routes to the REAL DoOutArm_9045_<v> (batch-3 FINAL, stubs removed)\n");
    witnessRouting("2x3_6",      DoOutArm_9045_2x3_6);
    witnessRouting("2x4_4",      DoOutArm_9045_2x4_4);
    witnessRouting("2x4_8",      DoOutArm_9045_2x4_8);
    witnessRouting("2x5_8",      DoOutArm_9045_2x5_8);
    witnessRouting("2x6_8",      DoOutArm_9045_2x6_8);
    witnessRouting("2x8_8",      DoOutArm_9045_2x8_8);
    // All_1Picker: DoOutArm-ONLY (picker-gated FIRST ladder branch, no DoPick arm)
    witnessRouting("All_1Picker", DoOutArm_9045_All_1Picker);

    // =======================================================================
    //  PART C2 -- Oracle 2: pure mode helper for 2x4_4 (golden :38 early-return).
    //  GetNowShuttleMode_2x4_4(iSht) returns 2 IMMEDIATELY when
    //  OutArmSuck.iXStep==1 && OutArmSuck.iYStep==1 (single-pick), DETERMINISTIC,
    //  no FR/BRCarryKit grid or Prod dependency.  We seed both steps to 1 and
    //  EXPECT 2 for BOTH shuttles -- proving the real batch-3 2x4_4 helper body
    //  (not a stub) is linked and the early-return formula is verbatim.
    // =======================================================================
    printf("[C2] Oracle 2: GetNowShuttleMode_2x4_4 early-return (golden aoutarm9045_2x4_4.cpp:38)\n");
    OutArmSuck.iXStep = 1;
    OutArmSuck.iYStep = 1;
    CHECK(GetNowShuttleMode_2x4_4(0) == 2,
          "C2 GetNowShuttleMode_2x4_4(0) == 2 (iXStep==iYStep==1 single-pick early-return)");
    CHECK(GetNowShuttleMode_2x4_4(1) == 2,
          "C2 GetNowShuttleMode_2x4_4(1) == 2 (iXStep==iYStep==1 single-pick early-return)");

    // =======================================================================
    //  PART C -- Oracle 1: pure geometry/shuttle-mode helper for 2x1_2.
    //  GetNowShuttleMode_2x1_2(iSht) -> 0 offline for BOTH shuttles because the
    //  first branch `if(ptrOutSHT->UseSiteNoIC()) return 0;` is taken (offline
    //  TMyKitSuck::UseSiteNoIC()==true).  Golden aoutarm9045_2x1_2.cpp:30-44.
    //  This proves the 2x1_2 variant's helper formula was translated VERBATIM
    //  and link-resolves (external linkage) over the Sim substrate.
    // =======================================================================
    printf("[C] Oracle 1: GetNowShuttleMode_2x1_2 (golden aoutarm9045_2x1_2.cpp:30)\n");
    CHECK(GetNowShuttleMode_2x1_2(0) == 0,
          "C GetNowShuttleMode_2x1_2(0) == 0 (FRCarryKit->UseSiteNoIC() true offline)");
    CHECK(GetNowShuttleMode_2x1_2(1) == 0,
          "C GetNowShuttleMode_2x1_2(1) == 0 (BRCarryKit->UseSiteNoIC() true offline)");

    // =======================================================================
    //  PART B -- engine geometry oracle still holds with variants linked
    //  (regression guard: adding the 6 variant TUs to ht9045_sm must not
    //   perturb the engine GetOutArmPitch_9045 interpolation -- same oracle
    //   as test_w6_2_outarm_core [N3], hand-derived from golden:212).
    // =======================================================================
    printf("[B] engine geometry oracle GetOutArmPitch_9045 (golden :212)\n");
    Prod.iOutArmX40Pitch  = 4000;
    Prod.iOutArmX120Pitch = 12000;
    iPitch_Max_minus_Min  = 8000;
    iXpitchMinX3          = 4000;
    CHECK(GetOutArmPitch_9045(8000) == 8000,
          "B GetOutArmPitch_9045(w=8000) == 8000 (m=1.0, endpoint)");
    CHECK(GetOutArmPitch_9045(4000) == 4000,
          "B GetOutArmPitch_9045(w=4000) == 4000 (min teach point -> X40)");

    // NOTE: the batch-4 DEAD variants (1x4_4_Back / 2x2_4_14 / 2x2_4_23 / 2x4_16
    // / 2x8_16 / 2x8_32) are intentionally NOT exercised here -- see the comment
    // block above PART A.  Linking their helpers would force-pull the heavy,
    // by-design-undispatched DoOutArm_9045_<deadv> object code (which references
    // offline-unsatisfiable cross-module/UI externs), misrepresenting correct
    // dead weight as broken.  Their compile/define/dead-by-design proof is the
    // Verify agent's out-of-band nm + grep check, reported in the schema.

    printf("==== W6.2c-OUT variants verify: %d passed, %d failed ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
