// =============================================================================
//  test_w6_2_inarm_canary.cpp  --  W6.2 VERIFY: shared InArm HP geometry leaves
//
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP pick/place geometry)
//  Author: AI(W6.2-INARM) 20260626
//
//  PURPOSE
//  -------
//  Companion verify TU for the W6.2 wave (analogous to test_w6_1_empty_canary /
//  test_w6_canary).  Proves the two shared HP-geometry leaf modules
//  (ainarm_SearchPickPlate.cpp + ainarm_SearchPlacePlate.cpp) + the W6.2
//  substrate (aHotPlateSubstrate.cpp) LINK and RUN with NO hardware, and pins a
//  set of FAITHFUL geometry-leaf values over a KNOWN Sim substrate state.
//
//    PART A -- GEOMETRY LEAVES (faithful branch values, golden-derived):
//      * GetHotPlateColStep(iPickHP): iInArmType==e9045_1x1_1 -> 1 (golden :751);
//        iPickCol==2 (not 1x1) -> 2 (golden :756);
//        default (iPickCol!=2, XDivision!=6/10) -> 4 (golden :850-853).
//      * GetInArmToHPYPitch_9045(): USE_IN_Y_IS_AUTO_PITCH=false, neither
//        iXPitchManual635/360 guard -> 6000 (golden :424); AUTO_PITCH + iPickRow==1
//        (not HT9046_LS/HT9016C) -> 6000 (golden :406).
//      * GetHeaterWaitTime(): pick cell NULL_IC -> 0; with IC + HotTime>=iHotTime
//        clamp -> 0 (golden :32-47, the ret<0 -> 0 clamp).
//      * Row2CanPutHP / HotPlateYPitchCanPutAll: iInArmType==e9045_2x3_6_14 ->
//        false (golden :200/:318 early-out).
//
//    PART B -- SM + DISPATCHER (link + converge, no crash):
//      * DoInArmPickFromHotPlate_9045(): set its cursor to entry (1) and pump.
//        Offline PickFromHPList is empty so SearchPlateToPick()==false ->
//        the SM returns true and resets cursor to 1 (golden case 1 early-out).
//      * DoPlaceToHotPlate_9045(): the W7-gated dispatcher's active stub -> false.
//      * DoPlaceToHPSwapData / DoPlaceToHPBackupData: callable, no crash
//        (offline data-swap over the Sim MOT[] tray + InArmSuck grid).
//
//  OFFLINE: aHotPlateSubstrate.cpp supplies the conservative substrate (empty
//  team list, no-IC kit grid).  We set the few globals each asserted branch
//  reads so the asserted value is the one under test.  iInArmPickFromHotPlateTask
//  is owned by ainarm_SearchPickPlate.cpp; we drive it via its extern.
// =============================================================================
#include "ainarm_SearchPickPlate.h"
#include "ainarm_SearchPlacePlate.h"
#include "aHotPlateSubstrate.h"
#include "csystem.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage, etc.
#include "FormsFacade.h"        // fMain/fSortCT/fLotInfo/fOffSet/fSCKART
#include <cstdio>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
// The SM owns this cursor in ainarm_SearchPickPlate.cpp; drive it via extern.
extern int iInArmPickFromHotPlateTask;

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the other W6 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// DoInArmPickFromHotPlate_9045 documented cursor set (golden switch(Task)).
static bool pickCursorSane(int t)
{
    switch (t)
    {
        case 1: case 50: case 100: case 105: case 110: case 150: case 190:
        case 200: case 210: case 220: case 300: case 320: case 330: case 335:
        case 336: case 340: case 350: case 400:
            return true;
        default:
            return false;
    }
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("W6_2_InArmCanary", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    printf("==== W6.2 InArm HotPlate geometry verify ====\n");
    // AI(W906-A4-6) 20260924: 吸嘴類別改用 golden 完整佈局後，TMyKitSuck::CopyToTray 是 golden 版（精簡鏡像原本是只寫 Item 的樁），
    //   會解參考 Mot.Tray.PordRec[c][r]->asBuffer。golden 在 InitialMotorParameter 配置這些紀錄（golden cinitial.cpp:4068，
    //   移植 cinitial.cpp:4585 起）；本測試不跑 InitialMotorParameter，照同一段替 DoPlaceToHPSwapData 會用到的兩個加熱盤配置。
    for (int x = 0; x < _MAX_COL_ITEM; x++)
        for (int y = 0; y < _MAX_ROW_ITEM; y++) {
            MOT[MMPlate1].Tray.PordRec[x][y] = new TMyProductionRecord();
            MOT[MMPlate2].Tray.PordRec[x][y] = new TMyProductionRecord();
        }

    // -----------------------------------------------------------------------
    //  PART A -- geometry leaves
    // -----------------------------------------------------------------------
    printf("[A] geometry leaves\n");

    // GetHotPlateColStep faithful branches (pick action) --------------------
    iPickPlateX[0] = 0;
    iPickPlateY[0] = 0;
    HotPlateForm.XDivision = 4;           // not 6/10 -> default branch
    HotPlateForm.YDivision = 8;
    i8PickerHPMode = 0;                   // not iHPWideHP

    iInArmType = e9045_1x1_1;             // golden :751 -> iStepHP=1
    CHECK(GetHotPlateColStep(iPickHP) == 1, "GetHotPlateColStep: e9045_1x1_1 -> 1");

    iInArmType = e9045_2x4_8;             // generic multi-picker
    InArmSuck.iPickCol = 2;               // golden :756 -> iStepHP=2
    CHECK(GetHotPlateColStep(iPickHP) == 2, "GetHotPlateColStep: iPickCol==2 -> 2");

    InArmSuck.iPickCol = 4;               // not 1/2 -> default branch
    CHECK(GetHotPlateColStep(iPickHP) == 4, "GetHotPlateColStep: default XDiv4 -> 4");

    // GetInArmToHPYPitch_9045 faithful -> 6000 ------------------------------
    USE_IN_Y_IS_AUTO_PITCH = false;
    USE_IN_OUT_ARM_Y_PITCH = 0;           // neither iXPitchManual635/360 guard
    CHECK(GetInArmToHPYPitch_9045() == 6000, "GetInArmToHPYPitch_9045: manual default -> 6000");

    USE_IN_Y_IS_AUTO_PITCH = true;        // AUTO_PITCH branch
    InArmSuck.iPickRow = 1;               // iPickRow==1, not HT9046_LS/HT9016C
    MachineTypeChoice = 0;                // != Type_HT9046_LS
    CHECK(GetInArmToHPYPitch_9045() == 6000, "GetInArmToHPYPitch_9045: auto-pitch row1 -> 6000");
    USE_IN_Y_IS_AUTO_PITCH = false;       // restore

    // GetHeaterWaitTime: pick cell NULL_IC -> 0 -----------------------------
    iPickPlate[0]  = 0;
    iPickPlateX[0] = 0;
    iPickPlateY[0] = 0;
    MOT[MMPlate1 + 0].SetTraySingleData(0, 0, NULL_IC);
    CHECK(GetHeaterWaitTime() == 0, "GetHeaterWaitTime: NULL_IC cell -> 0");

    // Row2CanPutHP / HotPlateYPitchCanPutAll: e9045_2x3_6_14 early-out false -
    iInArmType = e9045_2x3_6_14;
    CHECK(HotPlateYPitchCanPutAll() == false, "HotPlateYPitchCanPutAll: e9045_2x3_6_14 -> false");
    CHECK(Row2CanPutHP(true) == false,        "Row2CanPutHP: e9045_2x3_6_14 -> false");
    iInArmType = e9045_2x4_8;             // restore

    // ArmRow1AllHasIC is the faithful dead-body -> true (golden :50-78) ------
    CHECK(ArmRow1AllHasIC() == true, "ArmRow1AllHasIC: faithful dead-body -> true");

    // -----------------------------------------------------------------------
    //  PART B -- SM + dispatcher (link + converge, no crash)
    // -----------------------------------------------------------------------
    printf("[B] SM + dispatcher\n");

    // DoPlaceToHotPlate_9045 -- W7-gated dispatcher active stub -> false -----
    CHECK(DoPlaceToHotPlate_9045() == false, "DoPlaceToHotPlate_9045: W7 stub -> false");

    // DoInArmPickFromHotPlate_9045 -- offline empty team list: SearchPlateToPick
    // ()==false on case 1 -> the SM returns true & resets cursor to 1.  Pump a
    // bounded number of ticks; assert cursor stays sane and converges to 1.
    LastSet.iRunStartMode = 0;            // not rsmAutoSiteMap
    iInArmPickFromHotPlateTask = 1;
    bool bConverged = false;
    bool bCursorSane = true;
    for (int tick = 0; tick < 64; ++tick)
    {
        DoInArmPickFromHotPlate_9045();
        if (!pickCursorSane(iInArmPickFromHotPlateTask)) { bCursorSane = false; break; }
        if (iInArmPickFromHotPlateTask == 1) { bConverged = true; }
    }
    CHECK(bCursorSane,  "DoInArmPickFromHotPlate_9045: cursor stays in documented set");
    CHECK(bConverged,   "DoInArmPickFromHotPlate_9045: converges to entry/terminal (Task==1)");

    // DoPlaceToHPSwapData / DoPlaceToHPBackupData -- callable, no crash ------
    //   In-range coords over the Sim MOT[] tray + InArmSuck grid.
    InArmSuck.Item[0][0] = HAS_NULL_IC;   // no real IC -> no swap-error message path
    MOT[MMPlate1 + 0].SetTraySingleData(0, 0, NULL_IC);
    DoPlaceToHPSwapData(0, 0, 0, 0, 0);
    DoPlaceToHPBackupData(0, 0, 0, 0, 0);
    CHECK(true, "DoPlaceToHPSwapData / DoPlaceToHPBackupData: callable (no crash)");

    // =======================================================================
    //  PART C -- W6.2b IN-ARM ENGINE helpers (faithful golden formulas).
    //  Forward-declared here (not via ainarm9045.h) to avoid the default-arg
    //  double-declaration with aHotPlateSubstrate.h already included above.
    // =======================================================================
    extern double FindCentorPointIndex(int iItemCount);                 // golden :9071
    extern int    GetShtStartPos(int iCentorABSPostion, double dCentorIndex, int iGap); // golden :9078
    extern int    GetJStep();                                           // golden :8341
    extern int    GetShuttleCol(int iSuckRow, int iSuckCol, bool bOutArm); // golden :8360
    extern int    GetInArmPitch_9045(int w);                            // golden :129

    // FindCentorPointIndex: iPitchCount=(n-1); ret=(double)iPitchCount/2 -------
    //   n=4 -> 3/2 = 1.5 ; n=1 -> 0/2 = 0.0  (golden :9073-9075, verbatim).
    CHECK(FindCentorPointIndex(4) == 1.5, "FindCentorPointIndex(4) == 1.5 (golden 9071)");
    CHECK(FindCentorPointIndex(1) == 0.0, "FindCentorPointIndex(1) == 0.0 (golden 9071)");

    // GetShtStartPos(center,index,gap)=center-(int)(index*gap)  (golden :9080) --
    //   center=1000, index=1.5, gap=600 -> 1000-(int)(900)=100.
    CHECK(GetShtStartPos(1000, 1.5, 600) == 100, "GetShtStartPos(1000,1.5,600) == 100 (golden 9078)");

    // GetJStep: default picker mode (no Ax* / no 1x4-close) -> 1 (golden :8353) -
    iInArmType            = e9045_1x1_1;   // not in any Ax*/ACEG ladder
    iCloseSiteModeFor2x8  = 0;
    iCloseSiteModeFor1x4  = 0;
    bRunAutoClean         = false;
    CHECK(GetJStep() == 1, "GetJStep() default mode == 1 (golden 8341)");

    // GetShuttleCol ep1Picker early-return == iSuckCol  (golden :8364) ---------
    {
        int savedPicker = USE_PICKER_COUNT;
        USE_PICKER_COUNT = ep1Picker;
        CHECK(GetShuttleCol(0, 3, false) == 3, "GetShuttleCol ep1Picker -> iSuckCol (golden 8364)");
        USE_PICKER_COUNT = savedPicker;
    }

    // GetInArmPitch_9045 interpolation (golden :131-134):
    //   m = (X120-X40)/iPitch_Max_minus_Min ; r = X40 + m*(w - iXpitchMinX3).
    //   Set X40=1000, X120=1000 (m=0) -> r = 1000 + 0 = 1000 for any w.
    Prod.iInArmX40Pitch  = 1000;
    Prod.iInArmX120Pitch = 1000;
    CHECK(GetInArmPitch_9045(iXpitchMinX3) == 1000, "GetInArmPitch_9045 m==0 -> X40 (golden 129)");

    // -----------------------------------------------------------------------
    printf("==== W6.2 verify: %d passed, %d failed ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
