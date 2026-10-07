// =============================================================================
//  tests/test_ga2_c1_cinitial.cpp  -- GA-2-C1 real-code oracle test
//
//  Scope: cinitial.cpp block 1 (golden cinitial.cpp:77-3060) --
//  GetSHCHKPos / InitialSuckerName / InitSucker / InitialSwitchName /
//  InitialSwitch / InitialSensorName / InitialSensor / InitialSafeDoor /
//  InitialHeaterDoor.
//
//  This is a real-code ORACLE test: it calls the actual translated functions
//  (no reimplementation) and asserts against literal values copied verbatim
//  from the golden source (cited by golden cinitial.cpp line number in each
//  CHECK's message).  Minimal preconditions are seeded immediately before
//  each call (global config scalars, HSys.mapIOTable/IOTable rows, TMyKitSuck
//  iMaxRow/iMaxCol) following the seeding style of tests/test_sim_io.cpp and
//  tests/test_config_db.cpp.
//
//  Build (per GA-2-C1 verification-boundary rule -- no cmake tree build):
//    g++ -std=c++17 -I <tree root> -c tests/test_ga2_c1_cinitial.cpp
//    (link against cinitial.o + the rest of the tree's .o's for a runnable
//     binary; this file was exercised as its own TU compile plus a manual
//     link probe during GA-2-C1 -- see _ga2_c1_report.md for the exact
//     commands and their results.)
// =============================================================================
#include "cinitial.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "CosFunction.h"
#include "Config.h"
#include "database.h"
#include "mysensor.h"
#include "myswitch.h"
#include "aHotPlateSubstrate.h"
#include "acarry_shims.h"
#include "MyLaneIo.h"
#include "MyPLC/MyPLC_IO_Modbus.h"

#include <cstdio>

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  [1] GetSHCHKPos  (golden cinitial.cpp:77-255) -- pure geometry, no seeding.
// ---------------------------------------------------------------------------
static void test_GetSHCHKPos()
{
    std::printf("\n[1] GetSHCHKPos -- site-position geometry\n");

    // SingleSite: Pos=iCenterBase unconditionally (golden :83-87).
    TestIF_File.iTestMode = SingleSite;
    CHECK(GetSHCHKPos(0, 1000) == 1000, "SingleSite: GetSHCHKPos(0,1000)==1000 (golden :86)");
    CHECK(GetSHCHKPos(3, 1000) == 1000, "SingleSite: GetSHCHKPos(3,1000)==1000 (iSite ignored, golden :83-87)");

    // DualSite (1x2/2x2/2x1 BusyShuttle): site 0 = +half pitch, site 1 = -half pitch (golden :101-114).
    TestIF_File.iTestMode = DualSite;
    TestIF_File.dSiteXPitch = 40.0;   // iSiteXPitch = iUnitMultiply100(40.0) = 4000
    CHECK(GetSHCHKPos(0, 1000) == 1000 + 4000/2, "DualSite site0: iCenterBase+iSiteXPitch/2 (golden :108)");
    CHECK(GetSHCHKPos(1, 1000) == 1000 - 4000/2, "DualSite site1: iCenterBase-iSiteXPitch/2 (golden :111)");

    // QualSite1X4/_8Site2X4 (no b2x4SupportCenterPitch): 1.5/0.5/-0.5/-1.5 pitch ladder (golden :157-174).
    TestIF_File.iTestMode = QualSite1X4;
    CosFunction.b2x4SupportCenterPitch = false;
    TestIF_File.dSiteXPitch = 40.0;
    CHECK(GetSHCHKPos(0, 1000) == (int)(1000 + 4000*1.5), "QualSite1X4 site0: +1.5*pitch (golden :162)");
    CHECK(GetSHCHKPos(3, 1000) == (int)(1000 - 4000*1.5), "QualSite1X4 site3: -1.5*pitch (golden :171)");

    // _12Site2X6: 6-way ladder, site 2 = +0.5 pitch (golden :197-220).
    TestIF_File.iTestMode = _12Site2X6;
    TestIF_File.dSiteXPitch = 20.0;   // iSiteXPitch = 2000
    CHECK(GetSHCHKPos(2, 500) == (int)(500 + 2000/2), "_12Site2X6 site2: +0.5*pitch (golden :208)");
    CHECK(GetSHCHKPos(5, 500) == (int)(500 - 2000*2.5), "_12Site2X6 site5: -2.5*pitch (golden :217)");
}

// ---------------------------------------------------------------------------
//  [2] InitialSuckerName  (golden cinitial.cpp:257-354) -- .sName wiring only
//  (.SuckerName is gated -- TMySucker has no such field yet, see file banner).
// ---------------------------------------------------------------------------
static void test_InitialSuckerName()
{
    std::printf("\n[2] InitialSuckerName -- .sName wiring (portable subset)\n");

    // Seed FLCarryKit/FRCarryKit/BLCarryKit/BRCarryKit/OutSht3Kit iMaxRow/iMaxCol
    // (offline default is 0/0 -- aHotPlateSubstrate.cpp TMyKitSuck ctor; the real
    // SetMyKitSuckItemAmount() call that would do this at runtime is a later block).
    FLCarryKit.iMaxRow = 2;
    FLCarryKit.iMaxCol = 4;

    USE_PICKER_COUNT = 0;   // != ep16Picker -> exercise the !=ep16Picker naming branch downstream too

    InitialSuckerName();

    // TestSocket: str1.sprintf("%c%c",'A'+i,'a'+j) -- golden :281-282.
    CHECK(TestSocket.Suck[0][0].sName == AnsiString("Aa"), "TestSocket.Suck[0][0].sName==\"Aa\" (golden :281-282)");
    CHECK(TestSocket.Suck[3][7].sName == AnsiString("Dh"), "TestSocket.Suck[3][7].sName==\"Dh\" (i=3->'D', j=7->'h')");

    // FLCarryKit/FRCarryKit/BLCarryKit/BRCarryKit/OutSht3Kit share str1="%c%c" -- golden :301-306.
    CHECK(FLCarryKit.Suck[1][2].sName == AnsiString("Bc"), "FLCarryKit.Suck[1][2].sName==\"Bc\" (golden :302)");
    CHECK(BRCarryKit.Suck[0][3].sName == AnsiString("Ad"), "BRCarryKit.Suck[0][3].sName==\"Ad\" (golden :305)");
    CHECK(OutSht3Kit.Suck[1][1].sName == AnsiString("Bb"), "OutSht3Kit.Suck[1][1].sName==\"Bb\" (golden :306, HT-9046AU)");

    // OutArm2Suck: str2.sprintf("%c%c",'A'+i,'a'+j) -- golden :345,347.
    CHECK(OutArm2Suck.Suck[0][1].sName == AnsiString("Ab"), "OutArm2Suck.Suck[0][1].sName==\"Ab\" (golden :345,347)");
}

// ---------------------------------------------------------------------------
//  [3] InitSucker  (golden cinitial.cpp:356-1000) -- pSuck scaffolding,
//  sName/iMyRow/iMyCol/Enable wiring, and the full iMotNo Z-axis wiring.
//  The per-nozzle IO-config bodies (both branches) are gated -- not exercised.
// ---------------------------------------------------------------------------
static void test_InitSucker()
{
    std::printf("\n[3] InitSucker -- pSuck list + sName/iMyRow/iMyCol + iMotNo wiring\n");

    // -- (a) !=ep16Picker path: iMotNo wiring golden :823-839 / :896-913 (double-set, both identical) --
    USE_PICKER_COUNT = 0;                 // != ep16Picker
    InOutArmPickerUseMotor = 0;           // != eptUseMot (irrelevant on this path, set for determinism)
    MachineTypeChoice = 0;                // != Type_HT9045 / Type_HT9045_12Site (skip the Enable=false disabling)

    InitSucker();

    CHECK(InArmSuck.Suck[0][0].iMotNo == MInArmZA, "InArmSuck.Suck[0][0].iMotNo==MInArmZA (golden :823,897)");
    CHECK(InArmSuck.Suck[1][3].iMotNo == MInArmZH, "InArmSuck.Suck[1][3].iMotNo==MInArmZH (golden :830,904)");
    CHECK(OutArmSuck.Suck[0][2].iMotNo == MOutArmZE, "OutArmSuck.Suck[0][2].iMotNo==MOutArmZE (golden :836,910)");
    CHECK(OutArm2Suck.Suck[0][0].iMotNo == MOutSortAa, "OutArm2Suck.Suck[0][0].iMotNo==MOutSortAa (golden :841)");
    CHECK(OutArm2Suck.Suck[0][1].iMotNo == MOutSortAb, "OutArm2Suck.Suck[0][1].iMotNo==MOutSortAb (golden :842)");

    // FTestSuck/BTestSuck sName+iMyRow+iMyCol -- golden :780-786.
    CHECK(FTestSuck.Suck[0][0].sName == AnsiString("Aa"), "FTestSuck.Suck[0][0].sName==\"Aa\" (golden :780-781)");
    CHECK(FTestSuck.Suck[1][5].iMyRow == 1 && FTestSuck.Suck[1][5].iMyCol == 5,
          "FTestSuck.Suck[1][5].iMyRow/iMyCol==1/5 (golden :782-783)");
    CHECK(BTestSuck.Suck[0][7].sName == AnsiString("Ah"), "BTestSuck.Suck[0][7].sName==\"Ah\" (golden :784)");

    // k-index naming loop (else arm of the ep16Picker branch) -- golden :968-984.
    // k walks j-major: (i,j)=(0,0)->k0='A', (1,0)->k1='B', (0,1)->k2='C', (1,1)->k3='D', ...
    CHECK(InArmSuck.Suck[0][0].sName == AnsiString("A"), "InArmSuck.Suck[0][0].sName==\"A\" (k-index loop, golden :976)");
    CHECK(InArmSuck.Suck[1][0].sName == AnsiString("B"), "InArmSuck.Suck[1][0].sName==\"B\" (k-index loop, golden :976)");
    CHECK(InArmSuck.Suck[0][1].sName == AnsiString("C"), "InArmSuck.Suck[0][1].sName==\"C\" (k-index loop, golden :976)");
    CHECK(OutArm2Suck.Suck[0][0].sName == AnsiString("A"), "OutArm2Suck.Suck[0][0].sName==\"A\" (golden :990)");
    CHECK(OutArm2Suck.Suck[0][1].sName == AnsiString("B"), "OutArm2Suck.Suck[0][1].sName==\"B\" (golden :990)");

    // -- (b) Type_HT9045 disable branch: Enable=false for j in [4,8) -- golden :789-803 --
    MachineTypeChoice = Type_HT9045;
    InitSucker();
    CHECK(FTestSuck.Suck[0][5].Enable == false, "Type_HT9045: FTestSuck.Suck[0][5].Enable==false (golden :796)");
    CHECK(BTestSuck.Suck[1][6].Enable == false, "Type_HT9045: BTestSuck.Suck[1][6].Enable==false (golden :799)");
    MachineTypeChoice = 0;   // restore

    // -- (c) ep16Picker + eptUseMot path: HT1032 explicit motor map -- golden :915-944 --
    USE_PICKER_COUNT = ep16Picker;
    InOutArmPickerUseMotor = eptUseMot;
    InitSucker();
    CHECK(InArmSuck.Suck[0][4].iMotNo == MInArmZAe, "ep16Picker+eptUseMot: InArmSuck.Suck[0][4].iMotNo==MInArmZAe (golden :922)");
    CHECK(InArmSuck.Suck[1][7].iMotNo == MInArmZBh, "ep16Picker+eptUseMot: InArmSuck.Suck[1][7].iMotNo==MInArmZBh (golden :934)");
    CHECK(OutArmSuck.Suck[0][6].iMotNo == MOutArmZAg, "ep16Picker+eptUseMot: OutArmSuck.Suck[0][6].iMotNo==MOutArmZAg (golden :942)");
    // sName in THIS branch is "%c%c" (col-major j outer, i inner) -- golden :954-966.
    CHECK(InArmSuck.Suck[1][3].sName == AnsiString("Bd"), "ep16Picker+eptUseMot: InArmSuck.Suck[1][3].sName==\"Bd\" (golden :958-961)");

    // restore for downstream tests
    USE_PICKER_COUNT = 0;
    InOutArmPickerUseMotor = 0;
}

// ---------------------------------------------------------------------------
//  [4] InitialSwitchName  (golden cinitial.cpp:1002-1375) -- literal Name table.
// ---------------------------------------------------------------------------
static void test_InitialSwitchName()
{
    std::printf("\n[4] InitialSwitchName -- SW[] Name literals\n");

    InitialSwitchName();

    CHECK(SW[SwFKPowerOff].Name == AnsiString("SwFKPowerOff"), "SW[SwFKPowerOff].Name (golden :1004, first entry)");
    CHECK(SW[SwFKHome].Name == AnsiString("SwFKHome"), "SW[SwFKHome].Name (golden :1008)");
    CHECK(SW[SwTowerRed].Name == AnsiString("SwTowerRed"), "SW[SwTowerRed].Name (golden :1034)");
    CHECK(SW[SwLoadCarRFIDZBreaker].Name == AnsiString("SwLoadCarRFIDZBreaker"), "SW[SwLoadCarRFIDZBreaker].Name (golden :1366)");
    CHECK(SW[SwMultiEp].Name == AnsiString("SwMultiEp"), "SW[SwMultiEp].Name (golden :1374, LAST entry)");
}

// ---------------------------------------------------------------------------
//  [5] InitialSwitch  (golden cinitial.cpp:1377-1591) -- IO_Table.csv-driven
//  (card type 2/3) active path.  Seed HSys.mapIOTable/IOTable with one
//  synthetic row for SwFKPowerOff.
// ---------------------------------------------------------------------------
static void test_InitialSwitch()
{
    std::printf("\n[5] InitialSwitch -- IO_CARD_TYPE==NewIO_MN200 active path\n");

    IO_CARD_TYPE = NewIO_MN200;
    iControlPanelMode = 0;

    TIODATA* row = new TIODATA(AnsiString(""));   // safe default ctor (Count<14 branch, database.cpp:1852-1868)
    row->iEnable      = 1;
    row->iISABase     = eMotionNet;
    row->iLane        = 7;
    row->iIP          = 2;
    row->iPort        = 5;
    row->iBit         = 3;
    row->iInType      = 1;
    int idx = (int)HSys.IOTable.size();
    HSys.IOTable.push_back(row);
    HSys.mapIOTable[AnsiString("SwFKPowerOff")] = AnsiString(idx);

    InitialSwitch();

    CHECK(SW[SwFKPowerOff].Enable == true,  "SW[SwFKPowerOff].Enable==true (golden :1411-1412, iEnable==1)");
    CHECK(SW[SwFKPowerOff].ISABase == eMotionNet, "SW[SwFKPowerOff].ISABase==eMotionNet (golden :1413)");
    CHECK(SW[SwFKPowerOff].Ring == 7,  "SW[SwFKPowerOff].Ring==7 (golden :1415, from iLane)");
    CHECK(SW[SwFKPowerOff].IP == 2,    "SW[SwFKPowerOff].IP==2 (golden :1416)");
    CHECK(SW[SwFKPowerOff].Port == 5,  "SW[SwFKPowerOff].Port==5 (golden :1417)");
    CHECK(SW[SwFKPowerOff].Bit == 3,   "SW[SwFKPowerOff].Bit==3 (golden :1418)");
    CHECK(SW[SwFKPowerOff].Type == 1,  "SW[SwFKPowerOff].Type==1 (golden :1419, from iInType)");

    // A switch with no IO_Table.csv row at all must end up disabled (golden :1394-1395 / :1430).
    CHECK(SW[SwFKPowerOn].Enable == false, "SW[SwFKPowerOn].Enable==false (no mapIOTable row, golden :1430)");
}

// ---------------------------------------------------------------------------
//  [6] InitialSensorName  (golden cinitial.cpp:1593-2469) -- literal Name table.
// ---------------------------------------------------------------------------
static void test_InitialSensorName()
{
    std::printf("\n[6] InitialSensorName -- Sen[] Name literals\n");

    InitialSensorName();

    CHECK(Sen[SnFKPowerOff].Name == AnsiString("SnFKPowerOff"), "Sen[SnFKPowerOff].Name (golden :1595, first entry)");
    CHECK(Sen[SnSafeDoor1].Name == AnsiString("SnSafeDoor1"), "Sen[SnSafeDoor1].Name (present, InitialSafeDoor dependency)");
    CHECK(Sen[SnDailyCorrelation_Close].Name == AnsiString("SnDailyCorrelation_Close"),
          "Sen[SnDailyCorrelation_Close].Name (golden :2468, LAST entry)");
}

// ---------------------------------------------------------------------------
//  [7] InitialSensor  (golden cinitial.cpp:2471-2768) -- IO_Table.csv-driven
//  (card type 2/3) active path, mirroring InitialSwitch's test.
// ---------------------------------------------------------------------------
static void test_InitialSensor()
{
    std::printf("\n[7] InitialSensor -- IO_CARD_TYPE==NewIO_MN200 active path\n");

    IO_CARD_TYPE = NewIO_MN200;
    iControlPanelMode = 0;
    Enable_PLCSafety_IO = false;
    CosFunction.bTTLCanUse8Site = false;

    TIODATA* row = new TIODATA(AnsiString(""));
    row->iEnable  = 1;
    row->iISABase = eMotionNet;
    row->iLane    = 4;
    row->iIP      = 1;
    row->iPort    = 6;
    row->iBit     = 2;
    row->iInType  = 1;
    int idx = (int)HSys.IOTable.size();
    HSys.IOTable.push_back(row);
    HSys.mapIOTable[AnsiString("SnFKPowerOff")] = AnsiString(idx);

    InitialSensor();   // also runs InitialSafeDoor()+InitialHeaterDoor() as golden does (:2767)

    CHECK(Sen[SnFKPowerOff].Enable == true, "Sen[SnFKPowerOff].Enable==true (golden :2506-2507)");
    CHECK(Sen[SnFKPowerOff].ISABase == eMotionNet, "Sen[SnFKPowerOff].ISABase==eMotionNet (golden :2505)");
    CHECK(Sen[SnFKPowerOff].Ring == 4, "Sen[SnFKPowerOff].Ring==4 (golden :2519, from iLane)");
    CHECK(Sen[SnFKPowerOff].IP == 1,   "Sen[SnFKPowerOff].IP==1 (golden :2520)");
    CHECK(Sen[SnFKPowerOff].Port == 6, "Sen[SnFKPowerOff].Port==6 (golden :2523)");
    CHECK(Sen[SnFKPowerOff].Bit == 2,  "Sen[SnFKPowerOff].Bit==2 (golden :2524)");

    // SenBit0..9 / SenBit10..19 forced Enable=true unconditionally -- golden :2706-2710.
    CHECK(Sen[SenBit0+3].Enable == true,  "Sen[SenBit0+3].Enable==true (golden :2708)");
    CHECK(Sen[SenBit10+7].Enable == true, "Sen[SenBit10+7].Enable==true (golden :2709)");
}

// ---------------------------------------------------------------------------
//  [8] InitialSafeDoor  (golden cinitial.cpp:2772-2956) -- safety-critical,
//  zero gating.  Exercise the SAFE_DOOR_AMOUNT==0 (7-door) default branch.
// ---------------------------------------------------------------------------
static void test_InitialSafeDoor()
{
    std::printf("\n[8] InitialSafeDoor -- safety-critical door interlocks\n");

    MachineTypeChoice   = 0;      // != Type_HT7080, != Type_HT1032
    SAFE_DOOR_AMOUNT    = 0;      // 7-door config
    USE_MR_SYSTEM       = 0;
    USE_OHT_SYSTEM      = 0;
    AUTO3_IS_MAGAZINE   = 0;
    CUSTOMER_CODE       = 0;      // != CC_KYEC_LEE

    InitialSafeDoor();

    CHECK(Sen[SnSafeDoor1].Enable == true, "Sen[SnSafeDoor1].Enable==true (golden :2774)");
    CHECK(Sen[SnSafeDoor1].Type == 1,      "Sen[SnSafeDoor1].Type==1 (MachineTypeChoice!=Type_HT7080, golden :2784)");
    CHECK(Sen[SnSafeDoor4].Enable == false, "Sen[SnSafeDoor4].Enable==false (SAFE_DOOR_AMOUNT==0, golden :2805)");
    CHECK(Sen[SnSafeDoor10].Enable == false, "Sen[SnSafeDoor10].Enable==false (SAFE_DOOR_AMOUNT==0, golden :2807)");
    CHECK(Sen[SnMagazineSafeDoor].Enable == false,
          "Sen[SnMagazineSafeDoor].Enable==false (AUTO3_IS_MAGAZINE==0, golden :2916-2920 not taken)");
}

// ---------------------------------------------------------------------------
//  [9] InitialHeaterDoor  (golden cinitial.cpp:2962-2979) -- safety-critical,
//  zero gating.  Exercise both branches directly (SOFT_SIMULTE is OFF by
//  default in this tree -- MachineType.h:48 `//#define SOFT_SIMULTE`).
// ---------------------------------------------------------------------------
static void test_InitialHeaterDoor()
{
    std::printf("\n[9] InitialHeaterDoor -- heater-door interlock\n");

    // Branch A: ATC silicon + Single/DualSite + active cooling -> door DISABLED.
    ATC_SYSTEM = eATCSiliconType;
    TestIF_File.iTestMode = SingleSite;
    Temperature.bATCActiveCooling = true;
    InitialHeaterDoor();
    CHECK(Sen[SnHeaterDoor].Enable == false, "ATC silicon+SingleSite+cooling: SnHeaterDoor.Enable==false (golden :2970)");
    CHECK(Sen[SnHeaterDoor2].Enable == true, "SnHeaterDoor2.Enable==true unconditionally (golden :2977)");

    // Branch B: any other combo -> door ENABLED.
    Temperature.bATCActiveCooling = false;
    InitialHeaterDoor();
    CHECK(Sen[SnHeaterDoor].Enable == true, "cooling==false: SnHeaterDoor.Enable==true (golden :2974)");
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------
int main()
{
    std::printf("=================================================================\n");
    std::printf("  GA-2-C1 -- cinitial.cpp block 1 (golden :77-3060) oracle test\n");
    std::printf("=================================================================\n");

    test_GetSHCHKPos();
    test_InitialSuckerName();
    test_InitSucker();
    test_InitialSwitchName();
    test_InitialSwitch();
    test_InitialSensorName();
    test_InitialSensor();
    test_InitialSafeDoor();
    test_InitialHeaterDoor();

    std::printf("\n-----------------------------------------------------------------\n");
    std::printf("  Results: %d passed, %d failed\n", g_pass, g_fail);
    std::printf("-----------------------------------------------------------------\n");

    return (g_fail == 0) ? 0 : 1;
}
