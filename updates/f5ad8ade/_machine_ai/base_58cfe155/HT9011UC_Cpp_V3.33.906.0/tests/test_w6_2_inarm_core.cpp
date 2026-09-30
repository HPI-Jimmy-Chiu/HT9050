// =============================================================================
//  test_w6_2_inarm_core.cpp  --  W6.2b VERIFY: the IN-ARM ENGINE core SMs
//
//  Translation wave: W6.2b (IN-ARM ENGINE -- ainarm9045.cpp)
//  Author: AI(W6.2b-INARM) 20260626
//
//  PURPOSE
//  -------
//  The task-mandated *core engine* verify TU (companion to the W6.2 canary/
//  search TUs).  Where test_w6_2_inarm_canary pins the shared HP-geometry
//  leaves, THIS TU proves the ainarm9045.cpp ENGINE's central task state
//  machines + numeric helpers link into ht9045_sm and PUMP over the Sim HAL
//  with NO hardware, asserting:
//
//    (a) a KNOWN Sim HAL state is set (InArmSuck items / FLCarryKit-BLCarryKit
//        NoIC / iLoadPickX,Y / the per-SM entry cursors);
//    (b) the 3 ACTIVE core in-arm task SMs are pumped for N ticks FROM THEIR
//        ENTRY CURSOR;
//    (c) each cursor stays inside the documented switch(Task) state set and the
//        entry / early-out behaviour matches the GOLDEN (golden line cited);
//    (d) the geometry / numeric ORACLE values (hand-derived from the golden
//        formula) match the translation -- incl. the float->int TRUNCATION
//        contract.
//
//  Prints PASS/FAIL per case + a final summary; returns non-zero on any FAIL.
//
//  CORE SMs UNDER TEST (all ACTIVE in ainarm9045.cpp -- cursor flow verbatim):
//    [SM1] DoInArmAdditionalFunction()  cursor iInArmAdditionalFunctionTask
//          golden ainarm9045.cpp:3209.  Documented cursor universe:
//          {1,100,10000,20000,30000,40000,41000}.  case 1 FALLS THROUGH to
//          case 100 (no break @golden:3220).  Offline: every additional-fn
//          selector flag (bDieClean/bPrecisor/bBottom2DID/bInRotator) is gated
//          out, so case 100 takes the live `else { bResult=true; }` branch
//          (golden:3241-3243) -> Task settles at 100, returns true, no further
//          state.  ORACLE: from entry Task=1 the FIRST tick advances to 100 and
//          returns true.
//    [SM2] DoInArmIonFanGiveWay()  cursor iInArmIonFanGiveWayTask
//          golden ainarm9045.cpp:9166.  Documented cursor universe: {1,2,3,5}.
//          Offline: IniConfig.bA15_1ESDGiveWayFunction defaults false so case 1
//          takes the final `else { Task=5; }` (golden:9193-9196).  case 5
//          checks the carry-kit/InSHT*InLF guard; InSHT1InLF()/InSHT2InLF()
//          return false offline (csystem_predicates.cpp:219-220) so bResult
//          stays false and the cursor SETTLES at 5 (no crash, no escape from
//          the set).  ORACLE: from entry Task=1 the FIRST tick advances to 5
//          and the cursor never leaves {1,2,3,5}.
//    [SM3] ProcessSCKARTLoadingCount()  cursor iProcessSCKARTLoadingCountTask
//          golden ainarm9045.cpp:7305.  Documented cursor universe: {1,100,200}.
//          Offline: the Suck[].Destroy() body is gated to "done"=true and
//          MoveInArmZToPlateSafe() returns true (aHotPlateSubstrate.cpp:265),
//          so 1 -> 100 -> 200; case 200 sets bResult=true & bPickFromLoader
//          =false (golden:7331-7335) -> settles at 200.  iLoadPickX/Y default 0
//          (cmydef.cpp:4399-4400) so InArmSuckUse[0][0] write is in-bounds.
//          ORACLE: from entry Task=1 the SM converges to 200 and returns true.
//
//  NUMERIC / GEOMETRY ORACLES (hand-derived from the golden formula):
//    [N3] GetInArmPitch_9045 (golden:107/129): m=ChangeToFloatNonPcnt(X120-X40,
//         iPitch_Max_minus_Min); r=X40+m*(w-iXpitchMinX3); return (int)r.
//         X40=4000,X120=12000,Max_minus_Min=8000,iXpitchMinX3=4000:
//           w=8000 -> m=1.0 -> r=4000+1.0*4000=8000 -> 8000.
//           w=4000 -> r=4000+1.0*0   =4000          -> 4000 (min teach point).
//    [N4] GetInArmPitch_9045 TRUNCATION (golden:112 `return (int)r`):
//         X40=4000,X120=12005,Max_minus_Min=8000,iXpitchMinX3=4000,w=5000:
//           m=8005/8000=1.000625 -> r=4000+1.000625*1000=5000.625 ->
//           (int)5000.625 == 5000 (truncation toward zero, NOT round-to-5001).
//    [N5] GetJStep (golden:468/8341): both pickers false & 1x4-close!=AbAc -> 1;
//         AxEx -> 2; AxxG (and not AxEx) -> 3.  Offline default == 1.
//    [N6] FindCentorPointIndex (golden:577/9071): (double)(n-1)/2.
//         n=4 -> 1.5 ; n=1 -> 0.0.
//    [N7] GetShtStartPos(center,index,gap)=center-(int)(index*gap) (golden:584/
//         9078): center=1000,index=1.5,gap=600 -> 1000-(int)900 = 100.
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" here ==
//  clean g++ compile/link + the core SMs pump over the Sim HAL without crash +
//  outputs match values hand-derived from the golden formula/cursor logic.
// =============================================================================
#include "ainarm9045.h"            // engine surface (GetInArmPitch_9045, GetJStep, ...)
#include "aHotPlateSubstrate.h"    // InArmSuck / FLCarryKit / BLCarryKit / fBarCode
#include "csystem.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"        // LastSet, ShowErrorMessage, ...
#include "FormsFacade.h"
#include <cstdio>

// The 3 core-SM cursors are engine-owned (ainarm9045.cpp); drive them via the
// externs declared in ainarm9045.h (iInArmAdditionalFunctionTask,
// iInArmIonFanGiveWayTask, iProcessSCKARTLoadingCountTask) -- already in scope.
// ProcessSCKARTLoadingCount has a default-arg `bool bReset=false` and is NOT in
// the header surface; declare it locally (matching golden:7305 signature).
extern bool ProcessSCKARTLoadingCount(bool bReset);

// FindCentorPointIndex / GetShtStartPos are engine-internal shuttle-cell
// geometry leaves -- ACTIVE in ainarm9045.cpp but NOT on the ainarm9045.h
// surface.  Forward-declare them here (same idiom as test_w6_2_inarm_canary).
extern double FindCentorPointIndex(int iItemCount);                            // golden ainarm9045.cpp:577
extern int    GetShtStartPos(int iCentorABSPostion, double dCentorIndex, int iGap); // golden ainarm9045.cpp:584

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the other W6 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---- documented switch(Task) state sets (golden) --------------------------
static bool addlCursorSane(int t)
{   // golden ainarm9045.cpp:3216-3245 (+ gated action cases 10000..41000)
    switch (t) {
        case 1: case 100: case 10000: case 20000: case 30000:
        case 40000: case 41000: return true;
        default: return false;
    }
}
static bool ionFanCursorSane(int t)
{   // golden ainarm9045.cpp:9171-9217
    switch (t) {
        case 1: case 2: case 3: case 5: return true;
        default: return false;
    }
}
static bool sckartCursorSane(int t)
{   // golden ainarm9045.cpp:7316-7336
    switch (t) {
        case 1: case 100: case 200: return true;
        default: return false;
    }
}

int main()
{
    printf("==== W6.2b InArm CORE engine verify ====\n");

    // -----------------------------------------------------------------------
    //  KNOWN Sim HAL state (shared baseline for the SM pumps)
    // -----------------------------------------------------------------------
    //  Carry kits empty + no real IC in the in-arm grid; the per-SM entry
    //  preconditions are set just before each pump below.
    FLCarryKit.ClearAll();
    BLCarryKit.ClearAll();
    InArmSuck.SetAllToNullIC();          // HasRealIC() -> false
    LastSet.iRunStartMode = 0;           // not rsmAutoSiteMap

    // =======================================================================
    //  PART A -- core in-arm task SMs pumped over the Sim HAL
    // =======================================================================
    printf("[A] core task state machines\n");

    // ---- SM1: DoInArmAdditionalFunction -----------------------------------
    //  ENTRY behaviour (golden:3218-3220): case 1 runs the (offline-noop)
    //  predicate then falls through to case 100, which -- with all selector
    //  flags gated out -- takes `else { bResult=true; }` (golden:3241).
    InitDoInArmAdditionalFunction();     // golden:978 -> iInArmAdditionalFunctionTask=1
    CHECK(iInArmAdditionalFunctionTask == 1,
          "SM1 AdditionalFunction: entry cursor == 1 (golden 980)");
    bool addlRet = false;
    {
        bool sane = true, settled100 = false;
        for (int tick = 0; tick < 64; ++tick) {
            bool r = DoInArmAdditionalFunction();
            if (tick == 0) addlRet = r;          // first-tick return (entry->100)
            if (!addlCursorSane(iInArmAdditionalFunctionTask)) { sane = false; break; }
            if (iInArmAdditionalFunctionTask == 100) settled100 = true;
        }
        CHECK(sane,        "SM1 AdditionalFunction: cursor stays in {1,100,10000..41000}");
        CHECK(addlRet,     "SM1 AdditionalFunction: first tick returns true (golden 3243 else-path)");
        CHECK(settled100,  "SM1 AdditionalFunction: fall-through 1->100 (golden 3220 no-break)");
        CHECK(iInArmAdditionalFunctionTask == 100,
              "SM1 AdditionalFunction: settles at Task==100 (no further state)");
    }

    // ---- SM2: DoInArmIonFanGiveWay ----------------------------------------
    //  ENTRY (golden:9173): bA15_1ESDGiveWayFunction false -> else Task=5
    //  (golden:9195).  case 5 guard (InSHT*InLF false) -> bResult stays false,
    //  cursor SETTLES at 5 without escaping {1,2,3,5}.
    IniConfig.bA15_1ESDGiveWayFunction = false;     // make the entry else-path live
    InitDoInArmIonFanGiveWayTask();                 // golden:1123 -> iInArmIonFanGiveWayTask=1
    CHECK(iInArmIonFanGiveWayTask == 1,
          "SM2 IonFanGiveWay: entry cursor == 1 (golden 1125)");
    {
        bool sane = true, reached5 = false;
        for (int tick = 0; tick < 64; ++tick) {
            DoInArmIonFanGiveWay();
            if (!ionFanCursorSane(iInArmIonFanGiveWayTask)) { sane = false; break; }
            if (iInArmIonFanGiveWayTask == 5) reached5 = true;
        }
        CHECK(sane,     "SM2 IonFanGiveWay: cursor stays in {1,2,3,5}");
        CHECK(reached5, "SM2 IonFanGiveWay: entry else-path Task->5 (golden 9195)");
        CHECK(iInArmIonFanGiveWayTask == 5,
              "SM2 IonFanGiveWay: settles at Task==5 (InSHT*InLF false guard, golden 9211)");
    }

    // ---- SM3: ProcessSCKARTLoadingCount -----------------------------------
    //  reset to entry via the bReset=true path (golden:7309-7313), then pump.
    //  1 -> 100 -> 200 (destroy stub + MoveInArmZToPlateSafe true) -> bResult.
    iLoadPickX = 0;  iLoadPickY = 0;                // in-bounds InArmSuckUse[0][0]
    ProcessSCKARTLoadingCount(true);                // reset
    CHECK(iProcessSCKARTLoadingCountTask == 1,
          "SM3 SCKART: reset sets cursor == 1 (golden 7311)");
    {
        bool sane = true, converged200 = false, finalRet = false;
        for (int tick = 0; tick < 64; ++tick) {
            bool r = ProcessSCKARTLoadingCount(false);
            if (!sckartCursorSane(iProcessSCKARTLoadingCountTask)) { sane = false; break; }
            if (iProcessSCKARTLoadingCountTask == 200) { converged200 = true; finalRet = r; }
        }
        CHECK(sane,         "SM3 SCKART: cursor stays in {1,100,200}");
        CHECK(converged200, "SM3 SCKART: converges 1->100->200 (golden 7318-7331)");
        CHECK(finalRet,     "SM3 SCKART: case 200 returns true (golden 7334)");
        CHECK(bPickFromLoader == false,
              "SM3 SCKART: case 200 clears bPickFromLoader (golden 7335)");
    }

    // =======================================================================
    //  PART B -- numeric / geometry oracles (golden-formula hand-derived)
    // =======================================================================
    printf("[B] numeric / geometry oracles\n");

    // ---- N3: GetInArmPitch_9045 interpolation endpoints -------------------
    //  Use the documented oracle vector: X40=4000, X120=12000,
    //  iPitch_Max_minus_Min=8000, iXpitchMinX3=4000.
    Prod.iInArmX40Pitch  = 4000;
    Prod.iInArmX120Pitch = 12000;
    iPitch_Max_minus_Min = 8000;
    iXpitchMinX3         = 4000;
    CHECK(GetInArmPitch_9045(8000) == 8000,
          "N3 GetInArmPitch_9045(w=8000) == 8000 (golden 129: m=1.0)");
    CHECK(GetInArmPitch_9045(4000) == 4000,
          "N3 GetInArmPitch_9045(w=4000) == 4000 (min teach point -> X40)");

    // ---- N4: GetInArmPitch_9045 float->int TRUNCATION ---------------------
    //  X120=12005 -> m=1.000625 -> r=5000.625 -> (int)==5000 (NOT 5001).
    Prod.iInArmX120Pitch = 12005;
    CHECK(GetInArmPitch_9045(5000) == 5000,
          "N4 GetInArmPitch_9045 truncation: 5000.625 -> 5000 (golden 112 (int)r)");
    // restore the interpolation vector
    Prod.iInArmX120Pitch = 12000;

    // ---- N5: GetJStep default picker mode == 1 ----------------------------
    iInArmType           = e9045_1x1_1;   // not in any AxEx/AxxG ladder
    iCloseSiteModeFor2x8 = 0;
    iCloseSiteModeFor1x4 = 0;             // != e1x4CloseAbAc(3)
    bRunAutoClean        = false;
    CHECK(bUseAxExPicker() == false, "N5 bUseAxExPicker() false @e9045_1x1_1 (golden 408)");
    CHECK(bUseAxxGPicker() == false, "N5 bUseAxxGPicker() false @e9045_1x1_1 (golden 431)");
    CHECK(GetJStep() == 1,           "N5 GetJStep() default mode == 1 (golden 8341)");

    // ---- N6: FindCentorPointIndex -----------------------------------------
    CHECK(FindCentorPointIndex(4) == 1.5, "N6 FindCentorPointIndex(4) == 1.5 (golden 9071)");
    CHECK(FindCentorPointIndex(1) == 0.0, "N6 FindCentorPointIndex(1) == 0.0 (golden 9071)");

    // ---- N7: GetShtStartPos(center,index,gap) -----------------------------
    CHECK(GetShtStartPos(1000, 1.5, 600) == 100,
          "N7 GetShtStartPos(1000,1.5,600) == 100 (golden 9078)");

    // -----------------------------------------------------------------------
    printf("==== W6.2b InArm core verify: %d passed, %d failed ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
