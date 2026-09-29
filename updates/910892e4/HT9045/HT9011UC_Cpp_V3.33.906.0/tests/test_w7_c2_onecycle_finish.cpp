// =============================================================================
//  test_w7_c2_onecycle_finish.cpp -- W7-C2 VERIFY: end-of-lot ONE CYCLE
//                                     finish-check does REAL work over the grid.
//
//  Translation wave: W7-C2 (translate DoOneCycleFinishCheck, golden
//                    csystem.cpp:12813-14047 ~1235 lines, as the REAL body in the
//                    translated csystem.cpp -- replacing the no-op stub at
//                    csystem_shims.cpp:145).
//  Author: AI(W7C2-Integrate) 20260701
//  Suite name (add_test): W7_C2_OneCycleFinish
//
//  PURPOSE
//  -------
//  Before W7-C2, DoOneCycleFinishCheck() was a NO-OP stub (csystem_shims.cpp:145)
//  -> one-cycle "finished" instantly and falsely.  After W7-C2 the REAL golden
//  body is live and, because the cross-arm occupancy grid is now seedable (W7
//  grid-occupancy wave), it does REAL work:
//    * residual IC anywhere  -> set a wait cursor + early-return (do NOT finish);
//    * empty machine         -> the residual-IC wait gate is satisfied so the
//                               cursor is NOT parked on the residual cursor.
//
//  ENTRY GUARD (golden csystem.cpp:12820, reproduced verbatim as if(iOneCycle)):
//  the body does REAL work ONLY when iOneCycle!=0.  The W6.6 hub never sets
//  iOneCycle (the only flipping seam, fMain->BtnOneCycle->Down, is offline-false),
//  so it sees NO behaviour change (the no-regression key).
//
//  ORACLES (the point of the wave)
//  -------------------------------
//    (b) FALSE-FINISH GUARD (the load-bearing NEW capability): seed residual
//        occupancy (InArmSuck use-site = HAS_IC).  DoOneCycleFinishCheck() must
//        NOT finish: it sets iOneCycleTask=5 (the residual-IC wait cursor, golden
//        csystem.cpp:12933 -> reproduced at translated csystem.cpp:2487) and
//        early-returns, with iOneCycle still 1 (the finish path that clears
//        iOneCycle=0 at golden :13931 / translated csystem.cpp:3452 is NOT
//        reached).  With the OLD no-op stub the cursor never moved and iOneCycle
//        was never inspected -- seeding could not have produced 5.
//    (a) EMPTY machine (no occupancy seeded): the body does NOT park on the
//        residual-IC wait cursor 5 -- the residual gate (golden :12928-12934) is
//        satisfied, so one-cycle can proceed toward finish (the downstream
//        motion/tray-drain ladder is exercised by the live path over the Sim HAL).
//    (c) DRAIN: clear the seed -> the next call no longer parks on 5 (the residual
//        gate is satisfied again).
//
//  NO-REGRESSION witness: with iOneCycle=0 the body takes the else-branch
//  (golden :14025; fMain->BtnOneCycle->Down is offline-false + bOneCycle_BackUp
//  guard) and leaves the cursor untouched -- proving the guard gates the real work.
//
//  All globals the test seeds are SAVED and RESTORED so sibling suites see the
//  pristine state (as test_w7_grid_occupancy / test_w7_c1_cleanout_finish do).
//  No Borland binary exists; "equivalence" == clean g++ compile/link + the cursor
//  reflects the seeded grid exactly as the golden residual-IC gate dictates.
// =============================================================================
#include "csystem.h"               // DoOneCycleFinishCheck / InitOneCycle
#include "aHotPlateSubstrate.h"    // TMyKitSuck + InArmSuck / OutArmSuck / TestSocket globals
#include "cmydef.h"                // iOneCycle / iOneCycleTask / IndexStatus / Z1_Z2_Normal / NULL_IC / HAS_IC / CUSTOMER_CODE
#include <cstdio>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the W6/W7 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// Snapshot/restore a TMyKitSuck Item grid so seeding a global kit cannot leak.
struct KitSnap {
    int item[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int iShtRow, iShtCol, iMaxRow, iMaxCol;
};
static void SaveKit(TMyKitSuck &k, KitSnap &s) {
    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; ++i)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; ++j)
            s.item[i][j] = k.Item[i][j];
    s.iShtRow = k.iShtRow; s.iShtCol = k.iShtCol;
    s.iMaxRow = k.iMaxRow; s.iMaxCol = k.iMaxCol;
}
static void RestoreKit(TMyKitSuck &k, const KitSnap &s) {
    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; ++i)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; ++j)
            k.Item[i][j] = s.item[i][j];
    k.iShtRow = s.iShtRow; k.iShtCol = s.iShtCol;
    k.iMaxRow = s.iMaxRow; k.iMaxCol = s.iMaxCol;
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("W7_C2_OneCycleFinish", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    printf("==== W7-C2 OneCycleFinish verify (DoOneCycleFinishCheck REAL body) ====\n");

    // --- save every global the body / oracle touches ------------------------
    const int saveOneCycle      = iOneCycle;
    const int saveOneCycleCursor= iOneCycleTask;
    const int saveIndexStatus   = IndexStatus;
    const int saveCustomerCode  = CUSTOMER_CODE;

    KitSnap inSave, outSave, tsSave;
    SaveKit(InArmSuck,  inSave);
    SaveKit(OutArmSuck, outSave);
    SaveKit(TestSocket, tsSave);

    // Common offline-empty baseline: every cross-arm kit empty (== all NULL_IC),
    // IndexStatus normal, in one-cycle, cursor at entry.  CUSTOMER_CODE=0 keeps
    // the KYEC-only Fix3-full pre-gate (golden csystem.cpp:12822 / translated
    // csystem.cpp:2378, CC_KYEC_LEE==921) inactive so the residual gate is reached.
    InArmSuck.ClearAll();
    OutArmSuck.ClearAll();
    TestSocket.ClearAll();
    IndexStatus   = Z1_Z2_Normal;
    CUSTOMER_CODE = 0;
    iOneCycle     = 1;          // ENTER the active if(iOneCycle) path (golden :12820)
    iOneCycleTask = 0;

    // =======================================================================
    //  (a) EMPTY machine -> does NOT park on the residual-IC wait cursor 5
    // =======================================================================
    printf("[a] EMPTY machine -> one-cycle proceeds (not parked on residual cursor 5)\n");
    DoOneCycleFinishCheck();
    CHECK(iOneCycleTask != 5,
          "a1 empty machine -> cursor NOT 5 (residual-IC wait gate passed; one-cycle proceeds)");
    // The empty path may set a downstream motion/tray wait cursor (e.g. 9/12/13
    // over the Sim HAL) or reach finish; either way it is NOT the residual-IC
    // wait cursor 5, and iOneCycle is only cleared on the finish path.
    printf("     (empty cursor = %d, iOneCycle = %d)\n", iOneCycleTask, iOneCycle);

    // =======================================================================
    //  (b) FALSE-FINISH GUARD -- residual IC seeded -> cursor 5, NO finish
    // =======================================================================
    printf("[b] residual IC seeded (InArmSuck) -> must NOT finish (cursor 5, iOneCycle stays 1)\n");
    iOneCycle     = 1;
    iOneCycleTask = 0;
    IndexStatus   = Z1_Z2_Normal;
    InArmSuck.iShtRow = 2; InArmSuck.iShtCol = 1;
    InArmSuck.Item[0][0] = HAS_IC;        // residual IC at an in-arm nozzle
    CHECK(InArmSuck.HasIC() == true,
          "b0 seed witness: InArmSuck.HasIC()==true");
    DoOneCycleFinishCheck();
    CHECK(iOneCycleTask == 5,
          "b1 residual InArmSuck -> iOneCycleTask==5 (golden csystem.cpp:12933 residual wait gate)");
    CHECK(iOneCycle == 1,
          "b2 residual InArmSuck -> iOneCycle stays 1 (NO false finish; golden :13931 unreached)");

    // =======================================================================
    //  (c) DRAIN -- clear the seed -> next call no longer parks on 5
    // =======================================================================
    printf("[c] clear residual -> one-cycle proceeds (cursor no longer 5)\n");
    InArmSuck.ClearAll();
    iOneCycle     = 1;
    iOneCycleTask = 0;
    IndexStatus   = Z1_Z2_Normal;
    DoOneCycleFinishCheck();
    CHECK(iOneCycleTask != 5,
          "c1 cleared residual -> cursor NOT 5 (residual wait gate satisfied again)");
    printf("     (post-clear cursor = %d, iOneCycle = %d)\n", iOneCycleTask, iOneCycle);

    // =======================================================================
    //  NO-REGRESSION: iOneCycle==0 -> body takes the else-branch (golden :14025),
    //  where fMain->BtnOneCycle->Down is offline-false so iOneCycle stays 0 and
    //  the wait cursor is never touched.  This is the hub posture (iOneCycle
    //  never set to 1 by the W6.6 hub or any existing suite) -> no behaviour change.
    // =======================================================================
    printf("[d] NO-REGRESSION: iOneCycle=0 (hub posture) -> entry guard gates the real work\n");
    InArmSuck.Item[0][0] = HAS_IC;        // even WITH residual IC present...
    iOneCycle     = 0;                    // ...the else-branch is taken (no real work)
    iOneCycleTask = 12345;                // sentinel the body must NOT touch
    DoOneCycleFinishCheck();
    CHECK(iOneCycleTask == 12345,
          "d1 iOneCycle=0 -> cursor untouched (real one-cycle work gated off; golden :14025 else)");
    CHECK(iOneCycle == 0,
          "d2 iOneCycle=0 -> stays 0 (BtnOneCycle->Down offline-false; hub sees no change)");

    // --- restore every seeded global ----------------------------------------
    RestoreKit(InArmSuck,  inSave);
    RestoreKit(OutArmSuck, outSave);
    RestoreKit(TestSocket, tsSave);
    iOneCycle     = saveOneCycle;
    iOneCycleTask = saveOneCycleCursor;
    IndexStatus   = saveIndexStatus;
    CUSTOMER_CODE = saveCustomerCode;

    printf("==== W7-C2 OneCycleFinish verify: %d passed, %d failed ====\n",
           g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
