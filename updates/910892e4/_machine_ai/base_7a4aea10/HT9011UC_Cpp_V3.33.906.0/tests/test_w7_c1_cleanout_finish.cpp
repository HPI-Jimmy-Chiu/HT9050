// =============================================================================
//  test_w7_c1_cleanout_finish.cpp -- W7-C1 VERIFY: end-of-lot CLEAN OUT drain
//                                     finish-check does REAL work over the grid.
//
//  Translation wave: W7-C1 (translate DoCleanOutFinishCheck, golden
//                    csystem.cpp:14713-15748 ~1035 lines, as the REAL body in the
//                    translated csystem.cpp -- replacing the no-op stub).
//  Author: AI(W7C1-Integrate) 20260629
//  Suite name (add_test): W7_C1_CleanOutFinish
//
//  PURPOSE
//  -------
//  Before W7-C1, DoCleanOutFinishCheck() was a NO-OP stub (csystem_shims.cpp:146)
//  -> clean-out "finished" instantly and falsely.  After W7-C1 the REAL golden
//  body is live and, because the cross-arm occupancy grid is now seedable (W7
//  grid-occupancy wave), it does REAL work:
//    * residual IC anywhere  -> set a drain/non-finish cursor + early-return
//                               (do NOT finish);
//    * empty machine         -> drain proceeds toward finish (the cursor is NOT
//                               parked on the residual-IC cursor).
//
//  ENTRY GUARD (golden csystem.cpp:14720-14721, reproduced verbatim): the body
//  does REAL work ONLY when (iCleanOut==1 && iOneCycle==0).  The W6.6 hub never
//  sets iCleanOut, so it sees NO behaviour change (the no-regression key).
//
//  ORACLES (the point of the wave)
//  -------------------------------
//    (b) FALSE-FINISH GUARD (the load-bearing NEW capability): seed residual
//        occupancy (InArmSuck use-site = HAS_IC).  DoCleanOutFinishCheck() must
//        NOT finish: it sets iCleanOutCycleTask=1001 (the residual-IC drain
//        cursor, golden csystem.cpp:14739) and early-returns, with iCleanOut
//        still 1.  With the OLD no-op stub the cursor never moved and iCleanOut
//        was never inspected -- seeding could not have produced 1001.
//    (a) EMPTY machine (no occupancy seeded): the body does NOT park on the
//        residual-IC cursor 1001 -- clean-out can DRAIN toward finish (it passes
//        the residual-IC gate; the downstream motion/tray ladder is exercised by
//        the live drain path).
//    (c) DRAIN: clear the seed -> the next call no longer parks on 1001 (the
//        residual gate is satisfied again).
//
//  NO-REGRESSION witness: with iOneCycle=1 (the iOneCycle branch of the entry
//  guard) the body takes the empty `if(iOneCycle){}` arm and leaves the cursor
//  untouched -- proving the guard gates the real work.
//
//  All globals the test seeds are SAVED and RESTORED so sibling suites see the
//  pristine state.  No Borland binary exists; "equivalence" == clean g++
//  compile/link + the cursor reflects the seeded grid exactly as the golden
//  csystem.cpp:14739 residual-IC gate dictates.
// =============================================================================
#include "csystem.h"               // DoCleanOutFinishCheck / InitCleanOutFunction
#include "aHotPlateSubstrate.h"    // TMyKitSuck + InArmSuck / OutArmSuck / TestSocket globals
#include "cmydef.h"                // iCleanOut / iOneCycle / iCleanOutCycleTask / IndexStatus / Z1_Z2_Normal / NULL_IC / HAS_IC
#include <cstdio>

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
{
    printf("==== W7-C1 CleanOutFinish verify (DoCleanOutFinishCheck REAL body) ====\n");

    // --- save every global the body / oracle touches ------------------------
    const int saveCleanOut       = iCleanOut;
    const int saveOneCycle       = iOneCycle;
    const int saveCleanOutCursor = iCleanOutCycleTask;
    const int saveIndexStatus    = IndexStatus;

    KitSnap inSave, outSave, tsSave;
    SaveKit(InArmSuck,  inSave);
    SaveKit(OutArmSuck, outSave);
    SaveKit(TestSocket, tsSave);

    // Common offline-empty baseline: every cross-arm kit empty (== all NULL_IC),
    // IndexStatus normal, clean-out armed, not in one-cycle, cursor at entry.
    InArmSuck.ClearAll();
    OutArmSuck.ClearAll();
    TestSocket.ClearAll();
    IndexStatus = Z1_Z2_Normal;
    iOneCycle   = 0;
    iCleanOut   = 1;          // == InitCleanOutFunction() effect: arm the drain
    iCleanOutCycleTask = 0;

    // =======================================================================
    //  (a) EMPTY machine -> does NOT park on the residual-IC cursor 1001
    // =======================================================================
    printf("[a] EMPTY machine -> clean-out can DRAIN (not parked on residual cursor)\n");
    DoCleanOutFinishCheck();
    CHECK(iCleanOutCycleTask != 1001,
          "a1 empty machine -> cursor NOT 1001 (residual-IC gate passed; drain proceeds)");
    // The empty drain may set a downstream motion/tray cursor (e.g. 15/16/17 for
    // out-arm-not-parked over the Sim HAL) or reach finish; either way it is NOT
    // the residual-IC park, and iCleanOut is only cleared on the finish path.
    printf("     (empty-drain cursor = %d, iCleanOut = %d)\n", iCleanOutCycleTask, iCleanOut);

    // =======================================================================
    //  (b) FALSE-FINISH GUARD -- residual IC seeded -> cursor 1001, NO finish
    // =======================================================================
    printf("[b] residual IC seeded (InArmSuck) -> must NOT finish (cursor 1001, iCleanOut stays 1)\n");
    iCleanOut          = 1;
    iOneCycle          = 0;
    iCleanOutCycleTask = 0;
    InArmSuck.iShtRow = 2; InArmSuck.iShtCol = 1;
    InArmSuck.Item[0][0] = HAS_IC;        // residual IC at an in-arm nozzle
    CHECK(InArmSuck.HasIC() == true,
          "b0 seed witness: InArmSuck.HasIC()==true");
    DoCleanOutFinishCheck();
    CHECK(iCleanOutCycleTask == 1001,
          "b1 residual InArmSuck -> iCleanOutCycleTask==1001 (golden csystem.cpp:14739 residual gate)");
    CHECK(iCleanOut == 1,
          "b2 residual InArmSuck -> iCleanOut stays 1 (NO false finish)");

    // =======================================================================
    //  (c) DRAIN -- clear the seed -> next call no longer parks on 1001
    // =======================================================================
    printf("[c] clear residual -> drain proceeds (cursor no longer 1001)\n");
    InArmSuck.ClearAll();
    iCleanOut          = 1;
    iOneCycle          = 0;
    iCleanOutCycleTask = 0;
    DoCleanOutFinishCheck();
    CHECK(iCleanOutCycleTask != 1001,
          "c1 cleared residual -> cursor NOT 1001 (residual gate satisfied again)");
    printf("     (post-clear cursor = %d, iCleanOut = %d)\n", iCleanOutCycleTask, iCleanOut);

    // =======================================================================
    //  NO-REGRESSION: iOneCycle==1 -> body takes the empty if(iOneCycle){} arm
    // =======================================================================
    printf("[d] NO-REGRESSION: iOneCycle=1 -> entry guard gates the real work\n");
    InArmSuck.Item[0][0] = HAS_IC;        // even WITH residual IC present...
    iCleanOut          = 1;
    iOneCycle          = 1;               // ...the iOneCycle arm is taken first
    iCleanOutCycleTask = 12345;           // sentinel the body must NOT touch
    DoCleanOutFinishCheck();
    CHECK(iCleanOutCycleTask == 12345,
          "d1 iOneCycle=1 -> cursor untouched (real clean-out work gated off)");

    // =======================================================================
    //  NO-REGRESSION: iCleanOut!=1 && iOneCycle==0 -> only the else-branch one-
    //  cycle trigger; BtnOneCycle->Down is false offline + bOneCycle_BackUp guard
    //  -> the body must not park a clean-out drain cursor (it is inert for the hub).
    // =======================================================================
    printf("[e] NO-REGRESSION: iCleanOut=0 (hub posture) -> no clean-out drain cursor set\n");
    InArmSuck.ClearAll();
    iCleanOut          = 0;               // the W6.6 hub never sets iCleanOut
    iOneCycle          = 0;
    iCleanOutCycleTask = 54321;           // sentinel
    DoCleanOutFinishCheck();
    CHECK(iCleanOutCycleTask == 54321,
          "e1 iCleanOut=0 -> clean-out cursor untouched (hub sees no behaviour change)");

    // --- restore every seeded global ----------------------------------------
    RestoreKit(InArmSuck,  inSave);
    RestoreKit(OutArmSuck, outSave);
    RestoreKit(TestSocket, tsSave);
    iCleanOut          = saveCleanOut;
    iOneCycle          = saveOneCycle;
    iCleanOutCycleTask = saveCleanOutCursor;
    IndexStatus        = saveIndexStatus;

    printf("==== W7-C1 CleanOutFinish verify: %d passed, %d failed ====\n",
           g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
