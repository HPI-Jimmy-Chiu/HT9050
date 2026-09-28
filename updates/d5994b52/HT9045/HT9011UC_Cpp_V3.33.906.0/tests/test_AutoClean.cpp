// =============================================================================
//  test_AutoClean.cpp  --  W906-AutoCleanFoundation VERIFY
//
//  Translation wave: W906-AutoCleanFoundation
//  Author: AI(W906-AutoCleanFoundation) 20260721
//
//  PURPOSE
//  -------
//  Companion verify TU for the AutoClean foundation wave (same minimal
//  PASS/FAIL harness idiom as test_w6_2_inarm_canary.cpp / test_w6_canary.cpp).
//  Exercises every Part A pure-calc function with at least one representative
//  case, and pumps the HAL-only state-machine helpers against the Sim HAL
//  (MOT[MMAutoCleanKit] / InArmSuck / FLCarryKit / BLCarryKit) to prove they
//  link + run with no hardware and behave sanely. Also exercises the
//  FormsFacade InitialUnLoaderTask behaviour-change fix (Part B).
//
//  W906-AutoCleanCluster (20260722) ADD: extends this same TU with the 9
//  small helpers + 4 core pick/place engines + 3 shuttle-clean state machines
//  + the DoAutoCleanKit master orchestrator (the first of two remaining
//  dependency clusters; DoIndexAutoClean(+variant) stay out of scope -- a
//  TEMPORARY placeholder stub only, see AutoClean.cpp).
//
//  NOT COVERED
//  -----------
//  AI(pt-wave) 20260811 PT-W7e-part2: coverage LOST when the three case-30
//  expectations near line 646 were recalibrated. Recorded here rather than left
//  implicit, because a recalibrated expectation that is not written down reads
//  afterwards as "this path is tested" when it is not.
//
//    * DoAutoCleanPickfromCleanKit case 30 -> case 10 bounce-back, and the
//      WAR1922 alarm it raises (iAutoCleanAlarm==1, iResult==2).
//      WHY IT IS NO LONGER REACHED: that path used to be entered because
//      MoveInArm2XYToShuttle2Wait() was an offline-true stub. The stub is
//      retired and golden's real body is now live (ainarm2.cpp, golden
//      ainarm2.cpp:1440-1480); it reads MOT[MInArmPitch].ReadPos() and peers to
//      decide whether the arm has ACTUALLY reached the shuttle-2 wait position.
//      The simulated arm never arrives, so case 30 now parks on motion instead.
//      On a real machine the arm does arrive and the golden path is taken --
//      i.e. this is an OFFLINE reachability loss, not a behaviour change.
//      TO RE-COVER IT: drive MOT[MInArmPitch] to the shuttle-2 wait position in
//      the Sim HAL before the case-30 pump, then reinstate the original three
//      assertions (task==10, iAutoCleanAlarm==1, rPick==2).
// =============================================================================
#include "AutoClean/AutoClean.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "FormsFacade.h"
// AI(W906-AutoCleanCluster) 20260722: ADD -- fContact (TfContactShim) / bShuttleShake,
// acarry.h family symbols this extended test exercises directly.
#include "atester_shims.h"          // fContact
#include "csystem_shims.h"          // bShuttleShake
#include "acarry.h"                 // b1ShuttleMoveToLeft family (not directly asserted, but keeps parity with AutoClean.cpp's own include set)
#include "canary_support.h"
#include <cstdio>
#include <cstring>
#include "Motor/mySimMotor.h"        //AI(W906-FLOW-2) 20260928: TMySimMotor for W906_WithSimMotors (end of file)
// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the other W6/W906 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)
static void W906_WithSimMotors(void (*fn)());   void EnsureArmOffsetObjects();   //AI(W906-FLOW-2) 20260928: helper at EOF; EnsureArmOffsetObjects = cOffSet.cpp:166 (forms/fOffSet.h:450)
// ---------------------------------------------------------------------------
//  Reset the handful of globals every function under test reads, to a sane,
//  known baseline before each case (mirrors the "set the few globals each
//  asserted branch reads" idiom test_w6_2_inarm_canary.cpp uses).
// ---------------------------------------------------------------------------
// AI(W906-FLOW-2) 20260928: SUPERSEDED -- ARM_OFFSET has a ctor (cprod.cpp:267) and main() now allocates InArmOffSet[] (EnsureArmOffsetObjects), as golden TfMain does (main.cpp:2123-2139).  History: AI(W906-AutoCleanFoundation) 20260721: DISCOVERED PRE-EXISTING GAP (not
// introduced by this wave, and NOT fixed here -- see this wave's own report):
// InArmOffSet[]/OutArmOffSet[] (cprod.h) are declared as raw pointer arrays,
// but `ARM_OFFSET::ARM_OFFSET()`'s entire body sits inside cprod.cpp's own
// `#if 0 // TODO(W6): function bodies depend on untranslated state machines +
// globals` gate (cprod.cpp:184-1842, a pre-existing 1658-line deferred region
// unrelated to AutoClean) -- so ARM_OFFSET currently has NO linkable
// constructor anywhere in this tree, InArmOffSet[]/OutArmOffSet[] are
// permanently null pointers, and any code path that dereferences
// InArmOffSet[idx]->Get*() (e.g. MoveInArmXYPickCleanKit's unconditional
// GetInArmPitchY_9045/GetInArmPitchX_9045 calls) segfaults. This is a
// pre-existing cprod.cpp infrastructure gap (a future W6/W7-style wave's
// job to un-gate), not something an AutoClean-foundation-scoped fix should
// reach into. MoveInArmXYPickCleanKit is therefore deliberately NOT
// exercised below (translated + declared, just not smoke-run) -- every
// OTHER function this wave translates that reads InArmOffSet only does so
// behind a config-flag branch this test's defaults route around (verified:
// MoveInArmZToShuttlePlace/MoveInArmZ_Shuttle_Pick both PASS below).
static void ResetAutoCleanTestState()
{
    TestIF.iTestMode = QualSite1X4;
    TestIF_File.iTestMode = QualSite1X4;
    TestIF.iAutoClean_XDivision = 4;
    TestIF_File.iAutoClean_XDivision = 4;
    TestIF.iAutoClean_YDivision = 1;
    TestIF_File.iAutoClean_YDivision = 1;
    TestIF.iAutoClean_DeveicePices = 4;
    TestIF_File.iAutoClean_DeveicePices = 4;
    TestIF.dAutoClean_XPitch = 2000;
    TestIF_File.dAutoClean_XPitch = 2000;
    TestIF.dAutoClean_YPitch = 2000;
    TestIF_File.dAutoClean_YPitch = 2000;
    TestIF.iAutoClean_AlarmCount = 999999;   // effectively "never alarm" for most cases
    TestIF_File.iAutoClean_AlarmCount = 999999;
    TestIF.iAutoClean_Function = 1;
    TestIF_File.iAutoClean_Function = 1;
    TestIF.iAutoClean_Tray = eCKPos_CleanKit;
    TestIF_File.iAutoClean_Tray = eCKPos_CleanKit;
    TestIF.iAutoClean_SelectArm = 0;
    TestIF.iShuttleMode = 0;
    TestIF.iShuttle_Sel = 0;
    TestIF.bCleanIndexOtherArm = false;
    TestIF_File.iShuttleMode = 0;
    TestIF_File.iShuttle_Sel = 0;
    TestIF.bEnableAutoAlignment = false;
    TestIF_File.bEnableAutoAlignment = false;
    TestIF.bAutoClean_UseTray = false;
    TestIF_File.bAutoClean_UseTray = false;
    MACHINE_HAS_AUTO_ALIGNMENT_CCD = false;
    iInArmType = e9045_1x4_4;
    bUse8Picker = false;
    bUseTwoArm32Site = false;
    bRunAutoClean = false;
    iCloseSiteModeFor1x4 = 0;
    i1x2_4UseACEGPicker = 0;
    i1x2_4UseACEGPicker = 0;
    bCleanKitPitchLess4000 = false;
    bCleanKitPitchOver12000 = false;
    IniConfig.bE43AutoCleanUseHotplate = false;
    IniConfig.bEnableAutoCleanFunction = false;
    IniConfig.bE48_ShuttleUse4Offset_Autoclean = false;
    IniConfig.bAlarmNeedServoOff = false;
    CosFunction.bAutoCleanOffsetUseSingleSetting = false;
    CosFunction.bAutoCleanAutoSelIndexArm = false;
    CosFunction.bUseAutoCleanCloseSiteAlsoDo = false;
    CosFunction.bCleanCountAlarmByMin = false;
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen = false;
    CosFunction.bDeviceMapTest = false;
    bPlaceToShuttleByAutoClean = false;
    bPickFromShuttleByAutoClean = false;
    bPlaceToCleanKit = false;
    bPickFromKitByAutoClean = false;

    // Give MOT[MMAutoCleanKit].Tray a real XY size + all-NULL_IC content so the
    // grid-scan HAL-only functions have somewhere sane to read/write.
    MOT[MMAutoCleanKit].Tray.SetXYItem(TestIF.iAutoClean_XDivision, TestIF.iAutoClean_YDivision);
    MOT[MMAutoCleanKit].Tray.ClearData();
    fMain->AutoCleanStringGrid->ColCount = 8;
    fMain->AutoCleanStringGrid->RowCount = 8;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            fMain->AutoCleanStringGrid->Cells[x][y] = AnsiString("0");

    InArmSuck.ClearAll();
    FLCarryKit.ClearAll();
    BLCarryKit.ClearAll();
    FTestSuck.ClearAll();
    BTestSuck.ClearAll();
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j)
        {
            InArmSuck.iAutoCleanRecX[i][j] = -1;
            InArmSuck.iAutoCleanRecY[i][j] = -1;
            FLCarryKit.iAutoCleanRecX[i][j] = -1;
            FLCarryKit.iAutoCleanRecY[i][j] = -1;
            BLCarryKit.iAutoCleanRecX[i][j] = -1;
            BLCarryKit.iAutoCleanRecY[i][j] = -1;
        }

    LastSet.iRealDummy = DUMMY;
    fCleaning->b1x2SiteAbClosePutDummy = false;
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("==== W906-AutoCleanFoundation verify ====\n");
    ResetAutoCleanTestState();    EnsureArmOffsetObjects();   //AI(W906-FLOW-2) 20260928: golden boot invariant (wb_serve.cpp:4010-4013): the real MoveInArmXYToShuttle_9045 (GATE W7d-I1 retired) reads InArmOffSet[]

    // -----------------------------------------------------------------------
    //  Part A -- pure calc / config
    // -----------------------------------------------------------------------
    printf("[A] pure calc / config\n");

    // GetAutoCleanPickCount: default (bUseAxExPicker/AxxG false, not SingleSite,
    // not small pitch) -> 4 (golden else branch).
    ResetAutoCleanTestState();
    CHECK(GetAutoCleanPickCount() == 4, "GetAutoCleanPickCount default -> 4");
    TestIF.iTestMode = SingleSite;
    CHECK(GetAutoCleanPickCount() == 1, "GetAutoCleanPickCount SingleSite -> 1");

    // GetAutoCleanPickStep: SingleSite, col 0, no other-suck flags -> 0.
    ResetAutoCleanTestState();
    TestIF.iTestMode = SingleSite;
    CHECK(GetAutoCleanPickStep(0) == 0, "GetAutoCleanPickStep SingleSite col0 -> 0");
    // Non-picker default path: iSuckCol==iCol.
    ResetAutoCleanTestState();
    CHECK(GetAutoCleanPickStep(2) == 2, "GetAutoCleanPickStep default passthrough -> col");

    // CalculateAutoCleanXPitch: SingleSite -> 1.
    ResetAutoCleanTestState();
    TestIF.iTestMode = SingleSite;
    CHECK(CalculateAutoCleanXPitch() == 1, "CalculateAutoCleanXPitch SingleSite -> 1");
    // default path: ceil(XDivision/4.0)
    ResetAutoCleanTestState();
    TestIF.iTestMode = QualSite1X4;
    TestIF_File.iAutoClean_XDivision = 8;
    CHECK(CalculateAutoCleanXPitch() == 2, "CalculateAutoCleanXPitch default ceil(8/4)");

    // GetXPitchOfCleanKit_Kit / GetXPitchOfCleanKit dispatcher: SingleSite -> iXpitchMaxX3.
    ResetAutoCleanTestState();
    TestIF.iTestMode = SingleSite;
    CHECK(GetXPitchOfCleanKit_Kit() == iXpitchMaxX3, "GetXPitchOfCleanKit_Kit SingleSite -> iXpitchMaxX3");
    CHECK(GetXPitchOfCleanKit() == GetXPitchOfCleanKit_Kit(), "GetXPitchOfCleanKit dispatches to _Kit when HP disabled");
    IniConfig.bE43AutoCleanUseHotplate = true;
    CHECK(GetXPitchOfCleanKit() == GetXPitchOfCleanKit_HP(), "GetXPitchOfCleanKit dispatches to _HP when enabled");

    // GetYPitchOfCleanKit: bE43AutoCleanUseHotplate false -> TestIF.iARM_Y_PITCH passthrough.
    ResetAutoCleanTestState();
    TestIF.iARM_Y_PITCH = 12345;
    CHECK(GetYPitchOfCleanKit() == 12345, "GetYPitchOfCleanKit passthrough when HP-pitch disabled");

    // RunAutoCleanByArmPickArm2Test: all 3 gates default false -> false.
    ResetAutoCleanTestState();
    CHECK(RunAutoCleanByArmPickArm2Test() == false, "RunAutoCleanByArmPickArm2Test default false");
    IniConfig.bD58UseArm1PickPlaceArm2Test = true;
    TestIF_File.bArm1PickPlaceArm2Test = true;
    TestIF_File.bArm1PickPlaceArm2Test_RunAutoClean = true;
    CHECK(RunAutoCleanByArmPickArm2Test() == true, "RunAutoCleanByArmPickArm2Test all-3-gates true");

    // Special_2X6_Tray_XItem7: default state -> false (not e9045_2x6_8 etc.)
    ResetAutoCleanTestState();
    CHECK(Special_2X6_Tray_XItem7() == false, "Special_2X6_Tray_XItem7 default -> false");

    // GetInarmSuckRow: row 1 -> kitStep 0, suckRow 0; row 3 -> kitStep 4, suckRow 0.
    {
        int suckRow=-1, kitStep=-1;
        GetInarmSuckRow(1, suckRow, kitStep);
        CHECK(suckRow==0 && kitStep==0, "GetInarmSuckRow(1) -> suckRow=0,kitStep=0");
        GetInarmSuckRow(3, suckRow, kitStep);
        CHECK(suckRow==0 && kitStep==4, "GetInarmSuckRow(3) -> suckRow=0,kitStep=4");
        GetInarmSuckRow(2, suckRow, kitStep);
        CHECK(suckRow==1 && kitStep==0, "GetInarmSuckRow(2) -> suckRow=1,kitStep=0");
    }

    // -----------------------------------------------------------------------
    //  Part A -- HAL-only state-machine helpers
    // -----------------------------------------------------------------------
    printf("[A] HAL-only helpers\n");

    // Init task family -- each resets its own cursor(s) to 1.
    ResetAutoCleanTestState();
    iDoAutoCleanTask = 0; iDoShuttle1AutoCleanTask=0; iDoShuttle2AutoCleanTask=0;
    iDoShuttleAutoCleanTask=0; iDoIndexAutoCleanTask=0; iAutoCleanPickFromShuttleTask=0;
    iAutoCleanPlaceToShuttleTask=0;
    InitialAutoCleanTask();
    CHECK(iDoAutoCleanTask==1, "InitialAutoCleanTask resets iDoAutoCleanTask=1");
    InitialShuttleAutoCleanTask();
    CHECK(iDoShuttle1AutoCleanTask==1 && iDoShuttle2AutoCleanTask==1 && iDoShuttleAutoCleanTask==1,
          "InitialShuttleAutoCleanTask resets all 3 shuttle cursors");
    InitialIndexAutoCleanTask();
    CHECK(iDoIndexAutoCleanTask==1, "InitialIndexAutoCleanTask resets iDoIndexAutoCleanTask=1");
    InitPickFromShuttleTask();
    CHECK(iAutoCleanPickFromShuttleTask==1, "InitPickFromShuttleTask resets cursor=1");
    InitPlaceToShuttleTask();
    CHECK(iAutoCleanPlaceToShuttleTask==1, "InitPlaceToShuttleTask resets cursor=1");

    // CleanSetSpeed: both directions should just run without crashing (offline
    // motor-speed setters are no-ops/Sim HAL).  //AI(W906-FLOW-2) 20260928: not any more -- GATE (W7a-I4) is retired, so the real SetMotorAccelSpeed/SetMotorScaleSpeed run and need drivers (golden boot invariant); W906_WithSimMotors lends sim drivers for just these calls.
    W906_WithSimMotors([]{ CleanSetSpeed(true); });
    W906_WithSimMotors([]{ CleanSetSpeed(false); });
    CHECK(true, "CleanSetSpeed(true/false) runs without crash");

    // InOutArmSuckActiveSet: zeroes the whole grid.
    ResetAutoCleanTestState();
    bInArmSuckActive[0][0] = true; bInArmSuckActive[1][3] = true;
    InOutArmSuckActiveSet();
    CHECK(bInArmSuckActive[0][0]==false && bInArmSuckActive[1][3]==false, "InOutArmSuckActiveSet zeroes grid");

    // TrayHasCleanIC / TrayHasCleanICCount: empty grid -> false / 0.
    ResetAutoCleanTestState();
    CHECK(TrayHasCleanIC()==false, "TrayHasCleanIC empty grid -> false");
    CHECK(TrayHasCleanICCount()==0, "TrayHasCleanICCount empty grid -> 0");
    MOT[MMAutoCleanKit].SetTraySingleData(0, 0, HAS_CLEAN_IC);
    CHECK(TrayHasCleanIC()==true, "TrayHasCleanIC with one HAS_CLEAN_IC cell -> true");
    CHECK(TrayHasCleanICCount()==1, "TrayHasCleanICCount counts the one cell");

    // SetAutoCleanStringGrid + ReadWriteAutoCleanCount round-trip.
    ResetAutoCleanTestState();
    SetAutoCleanStringGrid(2, 1, AnsiString(7));
    CHECK(fMain->AutoCleanStringGrid->Cells[2][1] == AnsiString("7"), "SetAutoCleanStringGrid writes the cell");
    bRunAutoClean = false;   // ReadWriteAutoCleanCount's outer guard is bRunAutoClean==false -> proceeds
    ReadWriteAutoCleanCount(false, false);   // write path: should not crash
    CHECK(true, "ReadWriteAutoCleanCount(write) runs without crash");
    ReadWriteAutoCleanCount(true, false);    // read path: should not crash
    CHECK(true, "ReadWriteAutoCleanCount(read) runs without crash");

    // CheckAutoCleanCloseSite: bUseTwoArm32Site false -> false regardless of iSht.
    ResetAutoCleanTestState();
    CHECK(CheckAutoCleanCloseSite(0)==false, "CheckAutoCleanCloseSite false when bUseTwoArm32Site false");

    // DoInArmMoveToWaitPosByAutoClean: bIndexJamInArmMoveSafePostionByAutoClaen
    // false -> unconditionally true (and unlocks both locks).
    ResetAutoCleanTestState();
    bLockPlaceToShuttleByAutoClean = true; bLockPickFromShuttleByAutoClean = true;
    CHECK(DoInArmMoveToWaitPosByAutoClean()==true, "DoInArmMoveToWaitPosByAutoClean true when feature disabled");
    CHECK(bLockPlaceToShuttleByAutoClean==false && bLockPickFromShuttleByAutoClean==false,
          "DoInArmMoveToWaitPosByAutoClean clears both locks on the disabled path");

    // DoSocketSensorAlarm: should run without crash (iShowSocketSensor==0 default).
    ResetAutoCleanTestState();
    iShowSocketSensor = 0;
    CHECK(DoSocketSensorAlarm("test", 1) == true, "DoSocketSensorAlarm always returns true");

    // DoInArmPineRelease: bAlarmNeedServoOff false -> unconditionally true.
    ResetAutoCleanTestState();
    CHECK(DoInArmPineRelease()==true, "DoInArmPineRelease true when bAlarmNeedServoOff disabled");

    // CheckInSuckICFallDown / CheckInArmSuckFromCleanKitICFallDown: LastSet.iRealDummy==DUMMY -> false fast-path.
    ResetAutoCleanTestState();
    CHECK(CheckInSuckICFallDown(K_RETRY)==false, "CheckInSuckICFallDown false on DUMMY LastSet");
    CHECK(CheckInArmSuckFromCleanKitICFallDown(true)==false, "CheckInArmSuckFromCleanKitICFallDown false on DUMMY LastSet");

    // RestoreCleanKitData: empty InArmSuck/shuttles -> no crash, returns true.
    ResetAutoCleanTestState();
    CHECK(RestoreCleanKitData()==true, "RestoreCleanKitData returns true on empty state");

    // SearchCleanKitRowCol(int&,int&): empty tray -> false (no HAS_CLEAN_IC cell).
    ResetAutoCleanTestState();
    {
        int kr=-1, kc=-1;
        CHECK(SearchCleanKitRowCol(kr, kc)==false, "SearchCleanKitRowCol false over an all-NULL_IC tray");
        MOT[MMAutoCleanKit].SetTraySingleData(1, 0, HAS_CLEAN_IC);
        CHECK(SearchCleanKitRowCol(kr, kc)==true && kc==1 && kr==0,
              "SearchCleanKitRowCol finds the HAS_CLEAN_IC cell");
    }

    // DoPlaceToKitSwapData + PickFromCleanKit + PlaceToCleanKit: basic no-crash
    // smoke run over the Sim substrate (PlaceToCleanList is the offline-empty
    // uPlateInfo, so PlaceToCleanKit's GetHPFirstTeam takes the "no team" path).
    ResetAutoCleanTestState();
    DoPlaceToKitSwapData(bAutoPick, HAS_CLEAN_IC, 1, 0, 0, 0);
    CHECK(InArmSuck.Item[1][0]==HAS_CLEAN_IC, "DoPlaceToKitSwapData(bAutoPick) sets InArmSuck.Item");
    CHECK(PlaceToCleanKit()==true, "PlaceToCleanKit no-crash smoke run (empty PlaceToCleanList -> true)");

    // MoveInArmZToShuttlePlace / MoveInArmZ_Shuttle_Pick / MoveInOutArmZToKitPickPlace /
    // MoveInArmXYPickCleanKit: pure link+no-crash smoke runs over the Sim HAL
    // (InArmZMoveDown / InArmContinuousMove_9045 are the already-real Sim-HAL
    // motor movers other translated engines already exercise).
    ResetAutoCleanTestState();
    MoveInArmZToShuttlePlace(euShuttle1, 1);
    CHECK(true, "MoveInArmZToShuttlePlace runs without crash");
    MoveInArmZ_Shuttle_Pick(euShuttle1, 1);
    CHECK(true, "MoveInArmZ_Shuttle_Pick runs without crash");
    MoveInOutArmZToKitPickPlace(bAutoPick, true, euShuttle1, 1);
    CHECK(true, "MoveInOutArmZToKitPickPlace(pick) runs without crash");
    MoveInOutArmZToKitPickPlace(bAutoPlace, false, euShuttle1, 1);
    CHECK(true, "MoveInOutArmZToKitPickPlace(place, empty team list) runs without crash");
    // MoveInArmXYPickCleanKit deliberately NOT exercised here -- see the
    // ResetAutoCleanTestState() banner comment (pre-existing ARM_OFFSET ctor
    // gap in cprod.cpp; InArmOffSet[] is always null, and this function
    // unconditionally dereferences it via GetInArmPitchY_9045/X_9045).

    // GetShuttleState dispatcher: QualSite1X4 -> routes to GetShuttleState_1x4_4.
    ResetAutoCleanTestState();
    int stateViaDispatch = GetShuttleState(euShuttle1, true);
    (void)stateViaDispatch;
    CHECK(true, "GetShuttleState dispatcher runs without crash for QualSite1X4");

    // CheckShuttleSensor_Clean dispatcher: smoke run for a couple of modes.
    ResetAutoCleanTestState();
    CheckShuttleSensor_Clean(euShuttle1, false);
    TestIF.iTestMode = _16Site2X8;
    CheckShuttleSensor_Clean(euShuttle1, false);
    CHECK(true, "CheckShuttleSensor_Clean dispatcher runs without crash for 1x4/2x8");

    // SearchCleanKitRowCol(eWhichShuttle) / SearchCleanKitUpDown: smoke run.
    ResetAutoCleanTestState();
    MOT[MMAutoCleanKit].SetTraySingleData(0, 0, HAS_CLEAN_IC);
    SearchCleanKitRowCol(euShuttle1);
    CHECK(true, "SearchCleanKitRowCol(eWhichShuttle) runs without crash");
    SearchCleanKitUpDown(1, euShuttle1);
    CHECK(true, "SearchCleanKitUpDown runs without crash");

    // SetShuttleIcForSpecialMode: default close-site modes (all "OneByOne" /
    // none of the special enums) -> falls into the tail cleanup branch.
    ResetAutoCleanTestState();
    SetShuttleIcForSpecialMode(euShuttle1, NULL_IC);
    CHECK(true, "SetShuttleIcForSpecialMode runs without crash");

    // CleanPad_PlaceToShuttle / CleanPad_PickFromShuttle: smoke run, no crash.
    ResetAutoCleanTestState();
    CleanPad_PlaceToShuttle(euShuttle1);
    CHECK(true, "CleanPad_PlaceToShuttle runs without crash");
    CleanPad_PickFromShuttle(euShuttle1, 1);
    CHECK(true, "CleanPad_PickFromShuttle runs without crash");

    // AI(W906-AutoCleanFoundation-Review) 20260722: MoveSuckDataDiff is the MOVE
    // primitive CleanPad_PlaceToShuttle/CleanPad_PickFromShuttle rely on -- review
    // found a prior revision stopped after the target-side copy and silently
    // dropped golden's source-slot clear (MyKitSuck.cpp:1531-1560), leaving stale
    // data behind in the source grid.  Directly exercise the move semantics here
    // (both directions the two callers above use) rather than only smoke-testing
    // through the higher-level functions.
    {
        ResetAutoCleanTestState();
        InArmSuck.ClearAll();
        FLCarryKit.ClearAll();

        InArmSuck.Item[0][0] = HAS_IC;
        InArmSuck.iWhichSite[0][0] = 3;
        InArmSuck.bPass[0][0] = true;
        InArmSuck.cDeviceInf[0][0] = "ABC123";
        InArmSuck.PordRec[0][0].bUse = true;
        // AI(W906-A4-6) 20260924: 吸嘴類別改用 golden 完整佈局後，ClearAll 會對每格呼叫 PordRec.InitialRecord()，
        //   而 golden InitialRecord 遇到行數 != eDataTotal 的紀錄會 Delete 後再解參考（golden 自己的 bug，照翻保留，
        //   Public/MyProductionRecord.cpp:326-345）。原本的種子 CommaText="1,2,3" 只有 3 行，精簡鏡像的 ClearAll 不碰 PordRec
        //   所以沒事；換成 golden 後下一次 ClearAll 就 SEGFAULT。改用完整長度的紀錄（前三欄 1/2/3，其餘空），比對意思不變。
        InArmSuck.PordRec[0][0].asBuffer->Strings[0] = "1";
        InArmSuck.PordRec[0][0].asBuffer->Strings[1] = "2";
        InArmSuck.PordRec[0][0].asBuffer->Strings[2] = "3";
        const AnsiString kSeedComma = InArmSuck.PordRec[0][0].asBuffer->CommaText;

        FLCarryKit.MoveSuckDataDiff(InArmSuck, 0, 0, 0, 0);

        CHECK(FLCarryKit.Item[0][0] == HAS_IC, "MoveSuckDataDiff target gets Item");
        CHECK(FLCarryKit.iWhichSite[0][0] == 3, "MoveSuckDataDiff target gets iWhichSite");
        CHECK(FLCarryKit.bPass[0][0] == true, "MoveSuckDataDiff target gets bPass");
        CHECK(FLCarryKit.cDeviceInf[0][0] == "ABC123", "MoveSuckDataDiff target gets cDeviceInf");
        CHECK(FLCarryKit.PordRec[0][0].bUse == true, "MoveSuckDataDiff target gets PordRec.bUse");
        CHECK(FLCarryKit.PordRec[0][0].asBuffer->CommaText == kSeedComma, "MoveSuckDataDiff target gets PordRec.asBuffer->CommaText");
        CHECK(FLCarryKit.PordRec[0][0].asBuffer->Count == eDataTotal && FLCarryKit.PordRec[0][0].asBuffer->Strings[2] == "3", "MoveSuckDataDiff target PordRec keeps full length (eDataTotal) and field 2");

        CHECK(InArmSuck.Item[0][0] == NULL_IC, "MoveSuckDataDiff source Item cleared to NULL_IC (golden :1531)");
        CHECK(InArmSuck.iWhichSite[0][0] == -1, "MoveSuckDataDiff source iWhichSite reset to -1 (golden :1540)");
        CHECK(InArmSuck.bPass[0][0] == false, "MoveSuckDataDiff source bPass reset to false (golden :1543)");
        CHECK(InArmSuck.cDeviceInf[0][0] == "", "MoveSuckDataDiff source cDeviceInf reset to empty (golden :1553)");

        InArmSuck.ClearAll();
        FLCarryKit.ClearAll();
    }

    // -----------------------------------------------------------------------
    //  Part D -- TfCleaning free-function translations
    // -----------------------------------------------------------------------
    printf("[D] TfCleaning free functions\n");

    // CleanPadCountCanSupport2Arm: QualSite1X4, DeveicePices=4 -> <=4 -> false.
    ResetAutoCleanTestState();
    CHECK(CleanPadCountCanSupport2Arm()==false, "CleanPadCountCanSupport2Arm false at the <=4 threshold");
    TestIF_File.iAutoClean_DeveicePices = 8;
    CHECK(CleanPadCountCanSupport2Arm()==true, "CleanPadCountCanSupport2Arm true above the threshold");

    // SetDeviceInTray: DualSite USE_PICKER_COUNT!=0 path with 2 devices on an
    // 8-col tray -- routes through SetCleanCellValue -> MOT[MMAutoCleanKit] HAL
    // (iMode==eAutoCleanUsed), no crash + writes at least one cell.
    ResetAutoCleanTestState();
    MOT[MMAutoCleanKit].Tray.SetXYItem(8, 1);
    MOT[MMAutoCleanKit].Tray.ClearData();
    SetDeviceInTray(8, 1, 2, eAutoCleanUsed);
    CHECK(TrayHasCleanICCount() >= 1, "SetDeviceInTray(eAutoCleanUsed) writes at least one HAS_CLEAN_IC cell");

    // -----------------------------------------------------------------------
    //  Part B -- FormsFacade InitialUnLoaderTask behaviour-change fix
    // -----------------------------------------------------------------------
    printf("[B] FormsFacade::InitialUnLoaderTask\n");
    fLotInfo->iUnloaderTask[1] = 0;
    fLotInfo->InitialUnLoaderTask(1);
    CHECK(fLotInfo->iUnloaderTask[1]==1, "InitialUnLoaderTask now REALLY sets iUnloaderTask[pos]=1 (was a no-op)");
    CHECK(fLotInfo->iUnloaderTask[0]==0 && fLotInfo->iUnloaderTask[2]==0,
          "InitialUnLoaderTask only touches the targeted index");

    // =========================================================================
    //  W906-AutoCleanCluster (20260722)
    // =========================================================================

    // -----------------------------------------------------------------------
    //  New substrate: TMyKitSuck.ArmAll_HasICType/ShtAll_HasICType/FindNoIC,
    //  uPlateInfo::ClearGroupList, TfContactShim::DoFullViewCheck/InitDoFullViewCheck
    // -----------------------------------------------------------------------
    printf("[substrate] AutoCleanCluster new TMyKitSuck/uPlateInfo/fContact members\n");

    // ArmAll_HasICType/FindNoIC scan the FULL pick grid (iMaxRow/iMaxCol) --
    // this test file never calls SetPickerCount, so those default to 0
    // (zero-initialized, ctor does not set them) unless set explicitly here;
    // pin them so the loop is non-vacuous.
    ResetAutoCleanTestState();
    InArmSuck.iMaxRow = 2; InArmSuck.iMaxCol = 4;
    InArmSuck.ClearAll();
    CHECK(InArmSuck.ArmAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC)==true,
          "ArmAll_HasICType: an all-NULL_IC pick grid matches (NULL_IC, x)");
    CHECK(InArmSuck.FindNoIC()==true, "FindNoIC: true when at least one NULL_IC cell remains");
    InArmSuck.SetItemData(0, 0, HAS_CLEAN_IC);
    CHECK(InArmSuck.ArmAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC)==false,
          "ArmAll_HasICType: one HAS_CLEAN_IC cell breaks the match");
    InArmSuck.ClearAll();

    // ShtAll_HasICType scans the SHUTTLE-side grid (iShtRow/iShtCol) -- ctor
    // homes these to 2/1 and nothing in this test file changes them.
    FLCarryKit.ClearAll();
    CHECK(FLCarryKit.ShtAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC)==true,
          "ShtAll_HasICType: an all-NULL_IC shuttle grid matches (NULL_IC, x)");
    FLCarryKit.SetItemData(0, 0, HAS_CLEAN_IC);
    CHECK(FLCarryKit.ShtAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC)==false,
          "ShtAll_HasICType: one HAS_CLEAN_IC cell breaks the match");
    FLCarryKit.ClearAll();

    PlaceToCleanList->ClearGroupList();
    CHECK(true, "uPlateInfo::ClearGroupList runs without crash (offline list no-op)");

    fContact->InitDoFullViewCheck();
    CHECK(fContact->DoFullViewCheck()==true, "TfContactShim::DoFullViewCheck offline-completes (true) -- see .h banner for why");

    // -----------------------------------------------------------------------
    //  Part E -- 9 small helpers
    // -----------------------------------------------------------------------
    printf("[E] AutoCleanCluster small helpers\n");

    // GetMotFunc: pure "%s %d" formatter.
    CHECK(GetMotFunc("Test", 5) == AnsiString("Test 5"), "GetMotFunc formats \"%s %d\"");

    // CleanOnlyHasNullInShuttle: loop bound is InArmSuck.iShtRow/iShtCol (2x1
    // from the ctor); writes land on BLCarryKit.
    ResetAutoCleanTestState();
    BLCarryKit.ClearAll();
    BLCarryKit.SetItemData(0, 0, HAS_CLEAN_IC);   // a real IC breaks the "only null" condition
    CleanOnlyHasNullInShuttle();
    CHECK(BLCarryKit.Item[0][0]==HAS_CLEAN_IC, "CleanOnlyHasNullInShuttle leaves a real-IC shuttle untouched");
    BLCarryKit.ClearAll();
    BLCarryKit.SetItemData(0, 0, HAS_NULL_CLEAN_IC);  // still "only null-ish" (HAS_NULL_CLEAN_IC counts)
    CleanOnlyHasNullInShuttle();
    CHECK(BLCarryKit.Item[0][0]==NULL_IC, "CleanOnlyHasNullInShuttle clears an all-null-ish shuttle to NULL_IC");
    BLCarryKit.ClearAll();

    // ResetAutoClean: no clean pad stranded anywhere -> only bRunAutoClean clears.
    ResetAutoCleanTestState();
    bRunAutoClean = true;
    fAllMotorHome = true;
    ResetAutoClean();
    CHECK(bRunAutoClean==false, "ResetAutoClean always clears bRunAutoClean");
    CHECK(fAllMotorHome==true, "ResetAutoClean leaves fAllMotorHome alone when no clean pad is stranded anywhere");
    // a stranded clean pad on InArmSuck -> triggers the full-clear branch.
    ResetAutoCleanTestState();
    InArmSuck.SetItemData(0, 0, HAS_CLEAN_IC);
    fAllMotorHome = true;
    ResetAutoClean();
    CHECK(InArmSuck.Item[0][0]==NULL_IC, "ResetAutoClean clears InArmSuck when a stranded clean pad is found");
    CHECK(fAllMotorHome==false, "ResetAutoClean clears fAllMotorHome when a stranded clean pad is found (forces a real re-home)");
    CHECK(bRunAutoClean==false, "ResetAutoClean always clears bRunAutoClean (stranded-pad branch too)");

    // SetAutoCleanTrayPosition: HP-position disabled -> the static Top/Width/Height defaults.
    ResetAutoCleanTestState();
    fMain->tmyAutoClean->Top=-1; fMain->tmyAutoClean->Width=-1; fMain->tmyAutoClean->Height=-1;
    SetAutoCleanTrayPosition();
    CHECK(fMain->tmyAutoClean->Top==351 && fMain->tmyAutoClean->Width==89 && fMain->tmyAutoClean->Height==25,
          "SetAutoCleanTrayPosition uses the static Top/Width/Height defaults when HP-position disabled");
    // HP-position enabled -> mirrors mtPlate2 (the ONLY FormsFacade.h addition this wave).
    IniConfig.bE43AutoCleanUseHotplate = true;
    fMain->mtPlate2->Top=100; fMain->mtPlate2->Width=200; fMain->mtPlate2->Height=300;
    SetAutoCleanTrayPosition();
    CHECK(fMain->tmyAutoClean->Top==100 && fMain->tmyAutoClean->Width==200 && fMain->tmyAutoClean->Height==300,
          "SetAutoCleanTrayPosition mirrors fMain->mtPlate2 when HP-position enabled");

    // SearchiAutoCleanNum: two HAS_CLEAN_IC cells with distinct counts -> the
    // smaller-count cell's position label wins.
    ResetAutoCleanTestState();
    MOT[MMAutoCleanKit].SetTraySingleData(0, 0, HAS_CLEAN_IC);
    MOT[MMAutoCleanKit].SetTraySingleData(1, 0, HAS_CLEAN_IC);
    fMain->AutoCleanStringGrid->Cells[0][1] = AnsiString("5");   // count at (X=0,Y=0)
    fMain->AutoCleanStringGrid->Cells[1][1] = AnsiString("2");   // count at (X=1,Y=0) -- smaller
    fMain->AutoCleanStringGrid->Cells[0][3] = AnsiString("10");  // position label, Y+iAutoClean_YDivision(1)+2=3
    fMain->AutoCleanStringGrid->Cells[1][3] = AnsiString("20");
    iAutoCleanNum = -1;
    SearchiAutoCleanNum();
    CHECK(iAutoCleanNum==20, "SearchiAutoCleanNum picks the position label of the smaller-count cell (col 1, count=2 < count=5)");

    // EnableAutoclean(Manual=true): function enabled, not mid one-cycle -> triggers InitialAutoCleanAllTask.
    ResetAutoCleanTestState();
    TestIF.iAutoClean_Function = 1;
    bIsAutoOneCycle = false;
    iOneCycle = 0;
    bIsAutoOneCycleAutoclean = false;
    iDoAutoCleanTask = 0; iDoShuttle1AutoCleanTask=0; iDoShuttle2AutoCleanTask=0;
    iDoShuttleAutoCleanTask=0; iDoIndexAutoCleanTask=0;
    EnableAutoclean(true);
    CHECK(iDoAutoCleanTask==1, "EnableAutoclean(Manual) triggers InitialAutoCleanAllTask (resets iDoAutoCleanTask)");
    CHECK(bIsAutoOneCycleAutoclean==true, "EnableAutoclean(Manual) sets the bIsAutoOneCycleAutoclean latch");

    // EnableAutoclean(Manual=false): interval reached -> same trigger, plus the
    // contact-count label mirror (unconditional in the Manual==false branch).
    ResetAutoCleanTestState();
    TestIF.iAutoClean_Function = 1;
    TestIF.iAutoClean_IntervalContact = 5;
    iOneCycle = 0;
    iAutoClean_IndexContactCount = 10;   // >= IntervalContact
    iDoAutoCleanTask = 0;
    bIsAutoOneCycleAutoclean = false;
    EnableAutoclean(false);
    CHECK(iDoAutoCleanTask==1, "EnableAutoclean(auto, interval reached) triggers InitialAutoCleanAllTask");
    CHECK(fMain->AutoCleanContactCountLabel->Caption == AnsiString(10),
          "EnableAutoclean(auto) always mirrors iAutoClean_IndexContactCount onto the label");

    // AutoCleanWriteData: real WriteIniData file I/O (GetLastOpenFN/DataPath are
    // un-gated, see common.h) -- smoke-run only, matching this suite's existing
    // treatment of other real-file-writing calls.
    ResetAutoCleanTestState();
    AutoCleanWriteData("iAutoClean_IndexTime", 42);
    CHECK(true, "AutoCleanWriteData runs without crash (real WriteIniData file I/O)");

    // SetAutoCleanICCount: InitialOK==false -> guarded no-op.
    ResetAutoCleanTestState();
    InitialOK = false;
    fMain->AutoCleanStringGrid->ColCount = 99;
    SetAutoCleanICCount(false);
    CHECK(fMain->AutoCleanStringGrid->ColCount==99, "SetAutoCleanICCount(InitialOK=false) is a guarded no-op");

    // SetAutoCleanICCount: iAutoClean_Function disabled -> clears the whole tray, returns.
    ResetAutoCleanTestState();
    InitialOK = true;
    TestIF_File.iAutoClean_Function = 0;
    MOT[MMAutoCleanKit].SetTraySingleData(0, 0, HAS_CLEAN_IC);
    SetAutoCleanICCount(false);
    CHECK(MOT[MMAutoCleanKit].Tray.Data[0][0]==NULL_IC,
          "SetAutoCleanICCount clears the whole tray when iAutoClean_Function is disabled");

    // SetAutoCleanICCount: normal path (Work=false) -- resizes the grid + repopulates via SetDeviceInTray.
    ResetAutoCleanTestState();
    InitialOK = true;
    TestIF_File.iAutoClean_Function = 1;
    bRunAutoClean = false;
    iAutoCleanAlarm = 0;
    SetAutoCleanICCount(false);
    CHECK(fMain->AutoCleanStringGrid->ColCount==TestIF.iAutoClean_XDivision,
          "SetAutoCleanICCount(Work=false) resizes AutoCleanStringGrid->ColCount to iAutoClean_XDivision");
    CHECK(fMain->tmyAutoClean->XItem==TestIF.iAutoClean_XDivision && fMain->tmyAutoClean->YItem==TestIF.iAutoClean_YDivision,
          "SetAutoCleanICCount(Work=false) sets tmyAutoClean XItem/YItem");
    CHECK(TrayHasCleanICCount()>=1, "SetAutoCleanICCount repopulates the clean-kit tray via SetDeviceInTray");

    // -----------------------------------------------------------------------
    //  Part F -- 4 core pick/place engines
    // -----------------------------------------------------------------------
    printf("[F] AutoCleanCluster core pick/place engines\n");

    // DoAutoCleanPickfromCleanKit(Restart=true): resets both cursors, returns 0.
    ResetAutoCleanTestState();
    iAutoCleanPickFromCleanKitStageTask = 999;
    iAutoCleanNum = 0;
    int rPick = DoAutoCleanPickfromCleanKit(euShuttle1, true);
    CHECK(rPick==0, "DoAutoCleanPickfromCleanKit(Restart=true) returns 0");
    CHECK(iAutoCleanPickFromCleanKitStageTask==1, "DoAutoCleanPickfromCleanKit(Restart=true) resets the stage-task cursor to 1");
    CHECK(iAutoCleanNum==1, "DoAutoCleanPickfromCleanKit(Restart=true) resets iAutoCleanNum to 1");

    // Tick through case 1 -> case 10 -> case 30 -> case 10 (oscillates): an
    // empty clean-kit tray means CheckCleaningCount() returns FALSE (its scan
    // only sets bFlag=true for a NON-NULL_IC cell below the alarm threshold;
    // with every cell NULL_IC that never fires) -- so case 10 routes to case
    // 30 ("out of clean pads"), and case 30's MoveInArm2XYToShuttle2Wait() is
    // an offline-true stub (acatchtray_shims.h) so it fires the WAR1922 alarm
    // and (bUse_NewAutoCleanForm default false) bounces straight back to case
    // 10 every other tick. Deliberately never reaches case 20/21 -- MoveInArmXYPickCleanKit
    // is a KNOWN pre-existing gap this suite's own banner documents (InArmOffSet[]
    // has no linkable ctor, so that call would segfault) -- an empty tray's
    // case-10 path never goes there regardless.
    rPick = DoAutoCleanPickfromCleanKit(euShuttle1, false);   // case 1 -> case 10
    CHECK(iAutoCleanPickFromCleanKitStageTask==10, "DoAutoCleanPickfromCleanKit ticks case 1 -> case 10 (Z reaches safe immediately)");
    rPick = DoAutoCleanPickfromCleanKit(euShuttle1, false);   // case 10 -> case 30 (empty tray, CheckCleaningCount()==false)
    CHECK(iAutoCleanPickFromCleanKitStageTask==30, "DoAutoCleanPickfromCleanKit routes an empty clean-kit tray from case 10 to case 30 (out of clean pads)");
    CHECK(rPick==0, "... and keeps returning 0 (not finished, not erroring)");
    iAutoCleanAlarm = 0;
    rPick = DoAutoCleanPickfromCleanKit(euShuttle1, false);   // case 30 -> case 10 (WAR1922 alarm, bUse_NewAutoCleanForm==false)
    // AI(pt-wave) 20260811 PT-W7e-part2: these three expectations were built on
    // MoveInArm2XYToShuttle2Wait() being an offline-true stub -- this test said so itself in the
    // comment above. That stub is now RETIRED and golden's real body is live
    // (ainarm2.cpp, golden ainarm2.cpp:1440-1480): it reads MOT[MInArmPitch].ReadPos() and peers
    // and reports whether the arm has actually REACHED the shuttle-2 wait position. Offline the
    // simulated arm never gets there, so case 30 now PARKS waiting for motion instead of firing
    // WAR1922 and bouncing to case 10. That is golden behaviour on a real machine (the arm does
    // arrive); it is only unreachable offline. The old expectations are kept in this comment so
    // the change is visible rather than silently rewritten:
    //     was: task==10, iAutoCleanAlarm==1, rPick==2   (stub returned true immediately)
    CHECK(iAutoCleanPickFromCleanKitStageTask==30, "DoAutoCleanPickfromCleanKit case 30 now PARKS: the real MoveInArm2XYToShuttle2Wait needs actual arm motion");
    CHECK(iAutoCleanAlarm==0, "... so WAR1922 is NOT raised offline (it fires only after the arm reaches the wait position)");
    CHECK(rPick==0, "... and it keeps returning 0 (still working, not finished, not erroring)");

    // case 3300 "finish" path directly (seeded -- avoids the segfault-prone
    // case 20/21 path): LastSet.iRealDummy==DUMMY -> CheckInArmSuckFromCleanKitICFallDown
    // returns false -> Task resets to 1, AddHPSuckGroup() called, iResult=1.
    iAutoCleanPickFromCleanKitStageTask = 3300;
    rPick = DoAutoCleanPickfromCleanKit(euShuttle1, false);
    CHECK(rPick==1, "DoAutoCleanPickfromCleanKit case 3300 finish path (DUMMY LastSet) returns 1");
    CHECK(iAutoCleanPickFromCleanKitStageTask==1, "... and resets the stage-task cursor back to 1");

    // DoPlaceToShuttle: case 1 -> case 100 (fallthrough) -> ArmAll_HasICType
    // true (InArm fully clear) -> case 2000; InSHT1InLF() false by default (no
    // shuttle-in-place sensor asserted offline) -> parks at case 2000.
    ResetAutoCleanTestState();
    iAutoCleanPlaceToShuttleTask = 1;
    InArmSuck.ClearAll();
    bool rPlace = DoPlaceToShuttle(euShuttle1);
    CHECK(rPlace==false, "DoPlaceToShuttle first tick does not finish");
    CHECK(iAutoCleanPlaceToShuttleTask==2000, "DoPlaceToShuttle reaches case 2000 (case1->100->2000) when InArm is fully clear");

    // DoPickFromShuttle: case 1 -> case 10; MoveInArmXYToShuttle_9045 is the REAL body now (AI(W906-FLOW-2) 20260928: GATE W7d-I1 retired; was: "is an
    // offline Sim stub that always returns false (ainarm9045.cpp), so case 10
    // parks -- a deterministic, documented Sim-HAL limitation, not a bug here.")  It still parks offline: InArmContinuousMove_9045 returns false while MOT[MInArmX].Motor is NULL (Motor/mymotor.cpp:4962 guard).
    ResetAutoCleanTestState();
    iAutoCleanPickFromShuttleTask = 1;
    bool rPickSht = DoPickFromShuttle(euShuttle1, 1);
    CHECK(rPickSht==false, "DoPickFromShuttle first tick does not finish");
    CHECK(iAutoCleanPickFromShuttleTask==10, "DoPickFromShuttle reaches case 10 (SetShuttleIcForSpecialMode done)");
    rPickSht = DoPickFromShuttle(euShuttle1, 1);
    CHECK(iAutoCleanPickFromShuttleTask==10, "DoPickFromShuttle parks at case 10 (real MoveInArmXYToShuttle_9045; InArmContinuousMove_9045 is false with no X/Y driver)");

    // DoAutoCleanPlaceToCleanKit(Reset=true): resets the task cursor, returns false.
    ResetAutoCleanTestState();
    iAutoCleanPlaceToCleanKitTask = 555;
    bool rPlaceCK = DoAutoCleanPlaceToCleanKit(true);
    CHECK(rPlaceCK==false && iAutoCleanPlaceToCleanKitTask==1,
          "DoAutoCleanPlaceToCleanKit(Reset=true) resets the task cursor to 1 and returns false");
    // case 1 -> case 10 (Z reaches safe immediately); case 10 with InArm empty -> returns true (nothing to place).
    rPlaceCK = DoAutoCleanPlaceToCleanKit(false);
    CHECK(iAutoCleanPlaceToCleanKitTask==10, "DoAutoCleanPlaceToCleanKit ticks case 1 -> case 10");
    CHECK(rPlaceCK==false, "... does not finish on the tick that only just reached case 10");
    rPlaceCK = DoAutoCleanPlaceToCleanKit(false);
    CHECK(rPlaceCK==true, "DoAutoCleanPlaceToCleanKit returns true when InArm holds no IC (nothing to place)");

    // -----------------------------------------------------------------------
    //  Part G -- 3 shuttle-clean state machines
    // -----------------------------------------------------------------------
    printf("[G] AutoCleanCluster shuttle-clean state machines\n");

    ResetAutoCleanTestState();
    MOT[MInShuttle1].fCanMove=MOT[MInShuttle1].fCanMoveR=MOT[MInShuttle1].fCanMoveM=MOT[MInShuttle1].fCanMoveL=true;
    TestIF_File.iAutoClean_Tray = eCKPos_CleanKit;
    iDoShuttle1AutoCleanTask = 1;
    IniConfig.bD43IndexDropErrorCanRetryandSkip = false;
    DoShuttle1AutoClean_Arm1PickArm2Test();
    CHECK(iDoShuttle1AutoCleanTask==200,
          "DoShuttle1AutoClean_Arm1PickArm2Test: case 1 -> case 200 (shuttle1 IsCanMove + Clean-Kit tray mode)");
    // case 200: FTestSuck/FLCarryKit both clear (ResetAutoCleanTestState) ->
    // bIndexHasCleanIC=false, FLCarryKit.UseSiteNoIC()=true -> case 1000 (SHT_LEFT).
    DoShuttle1AutoClean_Arm1PickArm2Test();
    CHECK(iDoShuttle1AutoCleanTask==1000,
          "... case 200 with an empty shuttle+index routes to case 1000 (SHT_LEFT)");

    // DoShuttle1AutoClean: same case1->200 entry, but through the "real" (non-
    // Arm1PickArm2Test) wrapper -- exercises its own extra early-return guards.
    ResetAutoCleanTestState();
    MOT[MInShuttle1].fCanMove=MOT[MInShuttle1].fCanMoveR=MOT[MInShuttle1].fCanMoveM=MOT[MInShuttle1].fCanMoveL=true;
    TestIF_File.iAutoClean_Tray = eCKPos_CleanKit;
    iDoShuttle1AutoCleanTask = 1;
    bShuttleShake = false;
    IniConfig.bF16CheckShuttleSensorBroken = false;
    IniConfig.bD58UseArm1PickPlaceArm2Test = false;    // RunAutoCleanByArmPickArm2Test() -> false -> does NOT delegate
    bInSh1DoLtc = false;
    DoShuttle1AutoClean();
    CHECK(iDoShuttle1AutoCleanTask==200, "DoShuttle1AutoClean: case 1 -> case 200 (its own early guards all pass through)");

    // DoShuttle2AutoClean: mirrors DoShuttle1AutoClean's entry-guard shape plus
    // its OWN extra iAutoClean_SelectArm gate.
    ResetAutoCleanTestState();
    TestIF.iAutoClean_SelectArm = 2;             // != 0 -> does not early-return on the SelectArm gate
    CosFunction.bAutoCleanAutoSelIndexArm = false;
    MOT[MInShuttle2].fCanMove=MOT[MInShuttle2].fCanMoveR=MOT[MInShuttle2].fCanMoveM=MOT[MInShuttle2].fCanMoveL=true;
    TestIF_File.iAutoClean_Tray = eCKPos_CleanKit;
    iDoShuttle2AutoCleanTask = 1;
    bShuttleShake = false;
    IniConfig.bF16CheckShuttleSensorBroken = false;
    IniConfig.bD58UseArm1PickPlaceArm2Test = false;
    bInSh2DoLtc = false;
    DoShuttle2AutoClean();
    CHECK(iDoShuttle2AutoCleanTask==200, "DoShuttle2AutoClean: case 1 -> case 200 (its own early guards + SelectArm gate all pass through)");

    // -----------------------------------------------------------------------
    //  Part H -- master orchestrator (DoAutoCleanKit)
    // -----------------------------------------------------------------------
    printf("[H] AutoCleanCluster DoAutoCleanKit master orchestrator\n");

    // Early-exit guards: none of these should touch iDoAutoCleanTask at all.
    ResetAutoCleanTestState();
    iDoAutoCleanTask = 777;
    bLockPlaceToShuttleByAutoClean = true;
    bLockPickFromShuttleByAutoClean = false;
    bPlaceToCleanKit = false; bPickFromKitByAutoClean = false;
    DoAutoCleanKit();
    CHECK(iDoAutoCleanTask==777,
          "DoAutoCleanKit early-returns when locked-for-safe-move and neither place/pick-from-kit flag is set");

    ResetAutoCleanTestState();
    iDoAutoCleanTask = 777;
    fContact->fShow = true;
    iContactMode = 9;   // mirrors AutoClean.cpp's file-local CONTACT_DEVICE_MAP_CHECK constant (=9)
    DoAutoCleanKit();
    CHECK(iDoAutoCleanTask==777, "DoAutoCleanKit early-returns on fContact->fShow + Device-Map-Check contact mode");
    fContact->fShow = false;   // restore -- fContact is a shared global shim instance

    ResetAutoCleanTestState();
    iDoAutoCleanTask = 777;
    IniConfig.bF16CheckShuttleSensorBroken = true;
    bDoingF16 = true;
    DoAutoCleanKit();
    CHECK(iDoAutoCleanTask==777, "DoAutoCleanKit early-returns while an F16 shuttle-sensor-broken check is in progress");

    // Main flow: an empty clean-kit tray (0 clean pads < iAutoClean_DeveicePices)
    // routes case 1 -> case 5 -> case 2000 (the error-finish path), then case
    // 2000's own body runs (falls through into case 2001, a no-op here since
    // IniConfig.bA81WaitSECS defaults false) and parks at 2000 -- golden itself
    // re-runs case 2000's whole body every tick in this state (no case 2000
    // Task= reassignment on the bA81WaitSECS==false path); preserved verbatim.
    ResetAutoCleanTestState();
    iDoAutoCleanTask = 1;
    bLockPlaceToShuttleByAutoClean = false; bLockPickFromShuttleByAutoClean = false;
    fContact->fShow = false;
    IniConfig.bF16CheckShuttleSensorBroken = false;
    IniConfig.bA81WaitSECS = false;
    IniConfig.bEnable_SECS_GEM = false;
    USE_IN_Y_IS_AUTO_PITCH = false;              // skip the ASE-KaohSiung AOA branch entirely
    CosFunction.bFullTestBeforeAutoClean = false; // skip the RTC full-view-check branch (case 1 -> case 5 directly)
    bAutoCleaning = true;
    bIndexCheckState = false;
    bErrorAutoClean = false;
    DoAutoCleanKit();   // tick 1: case 1 -> case 5
    CHECK(iDoAutoCleanTask==5, "DoAutoCleanKit tick1: case 1 -> case 5 (Fix3 cylinder + full-view-check both disabled)");
    DoAutoCleanKit();   // tick 2: case 5 -> case 2000 (0 clean pads < DeveicePices)
    CHECK(iDoAutoCleanTask==2000, "DoAutoCleanKit tick2: case 5 routes an empty clean-kit tray to case 2000 (error-finish)");
    CHECK(bAutoCleaning==true, "... bAutoCleaning is still true (case 2000 hasn't run its body yet)");
    W906_WithSimMotors([]{ DoAutoCleanKit(); });   // tick 3: case 2000 body runs, falls through to case 2001 (no-op), parks  //AI(W906-FLOW-2) 20260928: case 2000 CleanSetSpeed(false) pushes speeds now (GATE W7a-I4 retired)
    CHECK(iDoAutoCleanTask==2000, "DoAutoCleanKit tick3: case 2000 parks after running its finish bookkeeping (bA81WaitSECS disabled)");
    CHECK(bAutoCleaning==false, "... case 2000 clears bAutoCleaning");
    CHECK(bIndexCheckState==true, "... case 2000 sets bIndexCheckState");
    CHECK(bErrorAutoClean==false, "... case 2000 clears bErrorAutoClean (set true by case 5's error path)");

    // -----------------------------------------------------------------------
    printf("==== %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

// AI(W906-FLOW-2) 20260928: golden's boot invariant -- InitialMotorParameter leaves no MOT[i].Motor NULL -- for exactly
// the calls that now push motor speeds: the retired GATE (W7a-I4) tail of CleanSetSpeed (golden AutoClean.cpp:770-815)
// dereferences MOT[i].Motor->Enable in SetMotorAccelSpeed / SetMotorScaleSpeed (cinitial.cpp:16780 / :13515), as golden
// does.  The sim drivers are detached again afterwards, so every other check in this suite keeps running on the
// NULL-driver HAL it was written against.  Same idea as tests/w906_test_motors.h, which attaches permanently.
static void W906_WithSimMotors(void (*fn)())
{
    static TMySimMotor* made[MAX_TRAY_MOTOR];
    for (int i = 0; i < MAX_TRAY_MOTOR; ++i)
    {
        made[i] = 0;
        if (MOT[i].Motor == 0) { made[i] = new TMySimMotor(); MOT[i].Motor = made[i]; }
    }
    fn();
    for (int i = 0; i < MAX_TRAY_MOTOR; ++i)
        if (made[i]) { MOT[i].Motor = 0; delete made[i]; made[i] = 0; }
}
