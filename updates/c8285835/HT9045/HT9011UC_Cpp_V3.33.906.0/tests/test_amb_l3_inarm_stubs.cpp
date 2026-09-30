// =============================================================================
//  test_amb_l3_inarm_stubs.cpp -- AI(W906-AMB-L3) 20260930
//  ctest: AmbL3InArmStubs
//
//  INBOX 111 layer 2: eight golden ainarm9045.cpp bodies that were empty `{}` stubs in the port stub block
//  (ainarm9045.cpp:3441-3444, :3458-3460, :3462) are now translated at the END of ainarm9045.cpp.  This test
//  proves each body RUNS (a `{}` stub fails every check below) and matches golden's arithmetic / strings:
//
//    A  SetAutoSkipCount (golden :2408-2476)      counter ++ / clear, iInArmWaitPosition=0 on clear, the two
//                                                  start/end tick pairs and the "Total time" branch (value 2 is
//                                                  what acatchtray.cpp:6345 writes, golden acatchtray.cpp:6211)
//    B  DoTraySkipProcess_9045 (already live)     THE WIRING: the Auto Skip limit ArmSpeed[InArm].iAutoSkipCT now
//                                                  trips -- bAutoSkipCntOver, counter back to 0 and the Loader tray
//                                                  cleared by DoRecordSkipPosition_9045.  With the stub the counter
//                                                  never moved and the limit could never be reached.
//    C  PickErrorData / TrayPickupErrorData / AutoSkipHasIClog / sLoadPickupClean (golden :2502 / :2516 / :2388 /
//       :4182)                                    exact strings incl. golden's "(X:%d,Y%d)" End format
//    D  ShowAutoSkipError (golden :2478-2500)      non-ASE: nothing; CC_ASE_KaohSiung: MES0102 with kcode 0 and
//                                                  errPart = sAskStartDetect, then SetAutoSkipCount(0)
//    E  DoInArmSuckPreOn (golden :2231-2252)       REALLY + pre-suck: bPickFromLoader, vacuum On() only for used
//                                                  NULL_IC nozzles and only within +-50 pulses; nothing otherwise
//    F  DoInArmLoadPickUP_9045 (golden :4175-4180) MyDBIProcess("Message", "LoadTrayPickupError<part>:<tray>")
//
//  Files touched: none of the machine's.  No IO: every InArmSuck nozzle gets OnEnable/OffEnable=false before
//  DoInArmSuckPreOn, so golden TMySucker::On() (mykitsuck.cpp) only sets Status.  RecordProcess / MyDBIProcess
//  write the event-log / production-log text files, which the directory-wide redirect roots in tests/CMakeLists.txt
//  (_w906_env_all_tests) send to the build dir.
// =============================================================================
#include "MachineDefine.h"
#include "ainarm9045.h"
#include "cprod.h"
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "cpublic.h"
#include "CosFunction.h"
#include "Config.h"
#include "aHotPlateSubstrate.h"
#include "FormsFacade.h"
#include "canary_support.h"
#include "mysensor.h"
#include <cstdio>

extern int iInArmWaitPosition;              // golden ainarm2.h:219 -- DEF acatchtray.cpp:136 (no header declares it)
extern int iXPosition[8], iYPosition;       // golden ainarm2.h:82 -- DEF ainarm9045_2x4_16_shims.cpp

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
static void PartA_SetAutoSkipCount()
{
    std::printf("PART A -- SetAutoSkipCount (golden :2408-2476)\n");
    const int savedCC = CUSTOMER_CODE;
    if (CUSTOMER_CODE == CC_ASE_KaohSiung) CUSTOMER_CODE = CC_ASE_KaohSiung + 1;   // keep RecordProcess out of part A
    iAutoSkipCT = 5; iAse_LoadSkipTime = 0; iAse_LoadTrayEndTotalTime = 0;
    iAutoTrayEndTotal = 0; iAutoTrayendhasIC = 0; iInArmWaitPosition = 3;

    SetAutoSkipCount(0);
    CHECK(iAutoSkipCT == 0, "A1 clear: iAutoSkipCT 5 -> 0 (golden :2436)");
    CHECK(iInArmWaitPosition == 0, "A2 clear: iInArmWaitPosition 3 -> 0 (golden :2437, Ifor 20210209)");

    SetAutoSkipCount(1);
    CHECK(iAutoSkipCT == 1 && iAutoTrayEndTotal == 1, "A3 +1: iAutoSkipCT 1, iAutoTrayEndTotal 1 (golden :2461-2462)");
    CHECK(iAse_LoadSkipTime == 1 && iAse_LoadTrayEndTotalTime == 1, "A4 first +1 arms both timers (golden :2447 / :2459)");

    const DWORD t0 = AutoTrayendStartTime[0];
    SetAutoSkipCount(1);
    CHECK(iAutoSkipCT == 2 && iAutoTrayEndTotal == 2, "A5 second +1: 2 / 2");
    CHECK(AutoTrayendStartTime[0] == t0, "A6 second +1 does not restart the timer (golden :2441, both flags already 1)");

    AutoTrayendStartTime[0] = MyTickCount() - 3500;                             // pretend 3.5 s of auto skip
    SetAutoSkipCount(0);
    CHECK(AutoTrayendPassTime[0] == (AutoTrayendEndTime[0] - AutoTrayendStartTime[0]) / 1000,
          "A7 clear: PassTime[0] = (End[0]-Start[0])/1000 (golden :2416)");
    CHECK(AutoTrayendPassTime[0] >= 3, "A8 clear: the 3.5 s window gives PassTime[0] >= 3");
    CHECK(iAse_LoadSkipTime == 0, "A9 clear with a non-zero count re-arms iAse_LoadSkipTime=0 (golden :2421)");
    CHECK(iAse_LoadTrayEndTotalTime == 1 && iAutoTrayEndTotal == 2,
          "A10 clear keeps the total (only the ==2 branch resets it, golden :2424-2435)");

    iAutoTrayendhasIC = 7;
    iAse_LoadTrayEndTotalTime = 2;                                              // what acatchtray.cpp:6345 writes
    AutoTrayendStartTime[1] = MyTickCount() - 2500;
    SetAutoSkipCount(0);
    CHECK(iAse_LoadTrayEndTotalTime == 0, "A11 ==2 branch: iAse_LoadTrayEndTotalTime -> 0 (golden :2426)");
    CHECK(iAutoTrayEndTotal == 0 && iAutoTrayendhasIC == 0, "A12 ==2 branch: totals -> 0 (golden :2433-2434)");
    CHECK(AutoTrayendPassTime[1] == (AutoTrayendEndTime[1] - AutoTrayendStartTime[1]) / 1000 && AutoTrayendPassTime[1] >= 2,
          "A13 ==2 branch: PassTime[1] = (End[1]-Start[1])/1000 (golden :2428)");
    CUSTOMER_CODE = savedCC;
}

// ---------------------------------------------------------------------------
static void SeedLoaderTray4x4()
{
    MOT[MMTrayY].Tray.SetXYItem(4, 4);
    for (int x = 0; x < _MAX_COL_ITEM; ++x)
        for (int y = 0; y < _MAX_ROW_ITEM; ++y) {
            MOT[MMTrayY].Tray.Data[x][y]       = (x < 4 && y < 4) ? HAS_IC : NULL_IC;
            MOT[MMTrayY].Tray.BufferData[x][y] = NULL_IC;
        }
}
static void OneEmptyPocketAt(int x)
{
    for (int i = 0; i < MAX_ARM_Row; ++i)
        for (int j = 0; j < MAX_ARM_Col; ++j)
            InArmSuck.Suck[i][j].Error = false;
    InArmSuck.Suck[0][0].Error = true;                                          // nozzle [0][0] picked nothing
    InArmSuckUse[0][0] = true;
    iXPosition[0] = x; iYPosition = 1; iLoadPitchStepY = 0;
}
static void PartB_AutoSkipLimitTrips()
{
    std::printf("PART B -- DoTraySkipProcess_9045: the Auto Skip limit now trips (ainarm9045.cpp:6844 / :6868)\n");
    const int savedCC = CUSTOMER_CODE;
    if (CUSTOMER_CODE == CC_ASE_KaohSiung) CUSTOMER_CODE = CC_ASE_KaohSiung + 1;
    const bool sRec = IniConfig.bRecordSkipPosition, sFifo = IniConfig.bI37_EnableFIFOMode, sLast = IniConfig.bE62TryPickLastRow;
    const bool sFtct = TestIF_File.bRENESAS_EnableFTCT, sHp = CosFunction.bShowHPICCount;
    const bool sAuto = ArmSpeed[InArm].bAutoSKIP; const int sLimit = ArmSpeed[InArm].iAutoSkipCT;
    IniConfig.bRecordSkipPosition = true;  IniConfig.bI37_EnableFIFOMode = false;  IniConfig.bE62TryPickLastRow = false;
    TestIF_File.bRENESAS_EnableFTCT = false;  CosFunction.bShowHPICCount = false;
    ArmSpeed[InArm].bAutoSKIP = true;  ArmSpeed[InArm].iAutoSkipCT = 2;          // give up the tray after 2 empty pockets
    bAutoSkipCntOver = false;

    SeedLoaderTray4x4();
    SetAutoSkipCount(0);
    OneEmptyPocketAt(1);
    const bool r1 = DoTraySkipProcess_9045();
    CHECK(r1, "B1 first skip returns true (golden :2995)");
    CHECK(iAutoSkipCT == 1, "B2 first skip counts: iAutoSkipCT 0 -> 1 (was stuck at 0 with the stub)");
    CHECK(bAutoSkipCntOver == false, "B3 first skip: 1 < limit 2, not over");
    CHECK(MOT[MMTrayY].Tray.Data[1][1] == NULL_IC && MOT[MMTrayY].Tray.BufferData[1][1] == HAS_SKIP_IC,
          "B4 the empty pocket (1,1) is marked NULL_IC / HAS_SKIP_IC");
    CHECK(MOT[MMTrayY].Tray.Data[0][0] == HAS_IC && MOT[MMTrayY].Tray.Data[3][3] == HAS_IC, "B5 the rest of the tray untouched");
    CHECK(InArmSuck.Suck[0][0].Error == false && InArmSuckUse[0][0] == false, "B6 nozzle error consumed");

    OneEmptyPocketAt(2);
    iInArmWaitPosition = 3;
    const bool r2 = DoTraySkipProcess_9045();
    CHECK(r2, "B7 second skip returns true");
    CHECK(bAutoSkipCntOver == true, "B8 second skip reaches the limit: bAutoSkipCntOver (golden :2982-2984)");
    CHECK(iAutoSkipCT == 0, "B9 limit reached -> SetAutoSkipCount(0) cleared the counter (golden :2985)");
    CHECK(iInArmWaitPosition == 0, "B10 ... and iInArmWaitPosition (golden :2437)");
    bool allNull = true;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) if (MOT[MMTrayY].Tray.Data[x][y] != NULL_IC) allNull = false;
    CHECK(allNull, "B11 bClearLoaderTray -> DoRecordSkipPosition_9045 emptied the whole Loader tray (golden :3065-3067)");

    IniConfig.bRecordSkipPosition = sRec;  IniConfig.bI37_EnableFIFOMode = sFifo;  IniConfig.bE62TryPickLastRow = sLast;
    TestIF_File.bRENESAS_EnableFTCT = sFtct;  CosFunction.bShowHPICCount = sHp;
    ArmSpeed[InArm].bAutoSKIP = sAuto;  ArmSpeed[InArm].iAutoSkipCT = sLimit;
    bAutoSkipCntOver = false;  SetAutoSkipCount(0);
    CUSTOMER_CODE = savedCC;
}

// ---------------------------------------------------------------------------
static void PartC_Strings()
{
    std::printf("PART C -- PickErrorData / TrayPickupErrorData / AutoSkipHasIClog / sLoadPickupClean\n");
    sAutoTrayendStartPos = "";  sAutoTrayendEndPos = "";  bAutoSkipStartXYlog = true;
    PickErrorData(2, 4);
    CHECK(sAutoTrayendStartPos == "(X:3,Y:5)", "C1 PickErrorData start string (golden :2506)");
    CHECK(sAutoTrayendEndPos == "(X:3,Y5)", "C2 PickErrorData end string keeps golden's missing colon (golden :2510)");
    CHECK(bAutoSkipStartXYlog == false, "C3 PickErrorData clears bAutoSkipStartXYlog (golden :2508)");
    PickErrorData(0, 0);
    CHECK(sAutoTrayendStartPos == "(X:3,Y:5)(X:1,Y:1)" && sAutoTrayendEndPos == "(X:3,Y5)(X:1,Y1)", "C4 PickErrorData appends");

    TrayPickupErrorData();
    CHECK(sAutoTrayendEndPos == "" && sAutoTrayendEndPosBuffer == "", "C5 TrayPickupErrorData sends and clears (golden :2518-2524)");
    CHECK(sAutoTrayendStartPos == "(X:3,Y:5)(X:1,Y:1)", "C6 TrayPickupErrorData leaves the start string");

    bASkStart = false;  bAutoSkiplog = false;  bAutoSkipHasIC = false;  bAutoTrayEndHasIC = false;
    sAskStartDetect = "";  sAutoTrayendabnormalPos = "";  iAutoTrayendhasIC = 0;
    AutoSkipHasIClog("A1", 2, 4);
    CHECK(iAutoTrayendhasIC == 0 && sAutoTrayendabnormalPos == "" && !bAutoTrayEndHasIC, "C7 AutoSkipHasIClog idle when both flags off (golden :2391)");
    bASkStart = true;
    AutoSkipHasIClog("A1", 2, 4);
    CHECK(bAutoSkipHasIC && sAskStartDetect == "(X:3,Y:5),", "C8 bASkStart: bAutoSkipHasIC + sAskStartDetect (golden :2396-2398)");
    CHECK(sAutoTrayendabnormalPos == "(X:3,Y:5)," && sAutoTrayendabnormalBuffer == "", "C9 abnormal position appended, buffer cleared");
    CHECK(bAutoTrayEndHasIC && iAutoTrayendhasIC == 1, "C10 bAutoTrayEndHasIC / iAutoTrayendhasIC++ (golden :2403-2404)");
    bASkStart = false;  bAutoSkiplog = true;
    AutoSkipHasIClog("A1", 0, 0);
    CHECK(sAskStartDetect == "(X:3,Y:5)," && sAutoTrayendabnormalPos == "(X:3,Y:5),(X:1,Y:1)," && iAutoTrayendhasIC == 2,
          "C11 bAutoSkiplog only: abnormal list grows, start-detect list does not");
    bAutoSkiplog = false;  bAutoSkipHasIC = false;  bAutoTrayEndHasIC = false;
    sAskStartDetect = "";  sAutoTrayendabnormalPos = "";  iAutoTrayendhasIC = 0;
    sAutoTrayendStartPos = "";  sAutoTrayendEndPos = "";

    const int sR = InArmSuck.iMaxRow, sC = InArmSuck.iMaxCol;
    InArmSuck.iMaxRow = 1;  InArmSuck.iMaxCol = 2;
    for (int i = 0; i < MAX_ARM_Row; ++i) for (int j = 0; j < MAX_ARM_Col; ++j) sLoadPickupErrorTrayPos[i][j] = "(9,9);";
    sLoadPickupClean();
    CHECK(sLoadPickupErrorTrayPos[0][0] == "" && sLoadPickupErrorTrayPos[0][1] == "", "C12 sLoadPickupClean clears inside iMaxRow x iMaxCol");
    CHECK(sLoadPickupErrorTrayPos[0][2] == "(9,9);" && sLoadPickupErrorTrayPos[1][0] == "(9,9);", "C13 ... and only there (golden's loop bounds)");
    InArmSuck.iMaxRow = sR;  InArmSuck.iMaxCol = sC;
    for (int i = 0; i < MAX_ARM_Row; ++i) for (int j = 0; j < MAX_ARM_Col; ++j) sLoadPickupErrorTrayPos[i][j] = "";
}

// ---------------------------------------------------------------------------
static void PartD_ShowAutoSkipError()
{
    std::printf("PART D -- ShowAutoSkipError (golden :2478-2500)\n");
    const int savedCC = CUSTOMER_CODE;
    const bool sAuto = ArmSpeed[InArm].bAutoSKIP;
    ArmSpeed[InArm].bAutoSKIP = false;                                          // keep SetAutoSkipCount's RecordProcess out of it
    if (CUSTOMER_CODE == CC_ASE_KaohSiung) CUSTOMER_CODE = CC_ASE_KaohSiung + 1;
    bAutoSkiplog = true;  sAutoTrayendabnormalPos = "(X:1,Y:1),";  bAutoSkipHasIC = true;  bASkStart = true;
    sAskStartDetect = "(X:3,Y:5),";  iAutoSkipCT = 4;
    W906_ShowErrorMessage_Reset();
    ShowAutoSkipError();
    CHECK(W906_ShowErrorMessage_Count == 0 && bAutoSkipHasIC && iAutoSkipCT == 4 && bAutoSkiplog,
          "D1 not CC_ASE_KaohSiung: returns at once (golden :2480-2481)");

    CUSTOMER_CODE = CC_ASE_KaohSiung;
    ShowAutoSkipError();
    CHECK(bAutoSkiplog == false && sAutoTrayendabnormalPos == "" && sAutoTrayendabnormalBuffer == "",
          "D2 ASE: abnormal list sent and cleared (golden :2483-2490)");
    CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "MES0102" && W906_ShowErrorMessage_LastKCode == 0,
          "D3 ASE: ShowErrorMessage(\"MES0102\", 0, ...) once (golden :2496)");
    CHECK(W906_ShowErrorMessage_LastErrPart == "(X:3,Y:5),", "D4 ASE: errPart is sAskStartDetect");
    CHECK(!bASkStart && !bAutoSkipHasIC && sAskStartDetect == "" && iAutoSkipCT == 0,
          "D5 ASE: flags cleared and SetAutoSkipCount(0) ran (golden :2494-2498)");
    W906_ShowErrorMessage_Reset();
    CUSTOMER_CODE = savedCC;
    ArmSpeed[InArm].bAutoSKIP = sAuto;
}

// ---------------------------------------------------------------------------
static void ResetNozzles()
{
    for (int i = 0; i < MAX_ARM_Row; ++i)
        for (int j = 0; j < MAX_ARM_Col; ++j) {
            InArmSuck.Suck[i][j].OnEnable  = false;                             // no IO line: On() only sets Status
            InArmSuck.Suck[i][j].OffEnable = false;
            InArmSuck.Suck[i][j].Status    = false;
            InArmSuckUse[i][j]             = false;
            InArmSuck.Item[i][j]           = NULL_IC;
        }
}
static void PartE_DoInArmSuckPreOn()
{
    std::printf("PART E -- DoInArmSuckPreOn (golden :2231-2252)\n");
    const int sRD = LastSet.iRealDummy;  const bool sPre = ArmSpeed[InArm].bSuckOnDown;
    CHECK(Sen[SnRKManualTStart].IsOn() == false, "E0 precondition: manual T-start key not pressed");
    const int x = MOT[MInArmX].ReadPos(), y = MOT[MInArmY].ReadPos();

    ResetNozzles();
    InArmSuckUse[0][0] = true;  InArmSuckUse[0][1] = true;  InArmSuck.Item[0][1] = HAS_IC;
    LastSet.iRealDummy = REALLY;  ArmSpeed[InArm].bSuckOnDown = true;  bPickFromLoader = false;
    DoInArmSuckPreOn(x + 50, y - 50);                                           // edge of the +-50 window
    CHECK(bPickFromLoader == true, "E1 REALLY + pre-suck: bPickFromLoader=true (golden :2240)");
    CHECK(InArmSuck.Suck[0][0].Status == true, "E2 used NULL_IC nozzle [0][0] opened (golden :2248-2249)");
    CHECK(InArmSuck.Suck[0][1].Status == false, "E3 nozzle already holding an IC stays closed");
    CHECK(InArmSuck.Suck[1][0].Status == false, "E4 unused nozzle stays closed");

    ResetNozzles();
    InArmSuckUse[0][0] = true;  bPickFromLoader = false;
    DoInArmSuckPreOn(x + 51, y);
    CHECK(bPickFromLoader == true && InArmSuck.Suck[0][0].Status == false,
          "E5 51 pulses away: flag set (before the range test, as golden) but no vacuum");

    ResetNozzles();
    InArmSuckUse[0][0] = true;  bPickFromLoader = false;  ArmSpeed[InArm].bSuckOnDown = false;
    DoInArmSuckPreOn(x, y);
    CHECK(bPickFromLoader == false && InArmSuck.Suck[0][0].Status == false, "E6 pre-suck off: nothing (golden :2237)");

    ArmSpeed[InArm].bSuckOnDown = true;  LastSet.iRealDummy = DUMMY;
    DoInArmSuckPreOn(x, y);
    CHECK(bPickFromLoader == false && InArmSuck.Suck[0][0].Status == false, "E7 DUMMY: nothing (golden :2236)");

    ResetNozzles();
    LastSet.iRealDummy = sRD;  ArmSpeed[InArm].bSuckOnDown = sPre;  bPickFromLoader = false;
}

// ---------------------------------------------------------------------------
static void PartF_DoInArmLoadPickUP()
{
    std::printf("PART F -- DoInArmLoadPickUP_9045 (golden :4175-4180)\n");
    W906_MyDBIProcess_Reset();
    DoInArmLoadPickUP_9045("A1B2", "(1,2);(3,4);");
    CHECK(W906_MyDBIProcess_Count == 1, "F1 one MyDBIProcess call");
    CHECK(W906_MyDBIProcess_LastS1 == "Message", "F2 table \"Message\" (golden :4179)");
    CHECK(W906_MyDBIProcess_LastS2 == "LoadTrayPickupErrorA1B2:(1,2);(3,4);", "F3 text \"LoadTrayPickupError%s:%s\" (golden :4178)");
    W906_MyDBIProcess_Reset();
}

int main()
{
    PartA_SetAutoSkipCount();
    PartB_AutoSkipLimitTrips();
    PartC_Strings();
    PartD_ShowAutoSkipError();
    PartE_DoInArmSuckPreOn();
    PartF_DoInArmLoadPickUP();
    std::printf("\nAmbL3InArmStubs: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
