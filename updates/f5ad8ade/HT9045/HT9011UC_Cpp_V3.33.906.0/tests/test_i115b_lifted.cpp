// =============================================================================
//  tests/test_i115b_lifted.cpp -- AI(W906-I115B) 20260930
//
//  INBOX 115 class B (RULINGS_20260930 #4, user 0930 13:4x "全部開"): nine of
//  the ten HIGH `#if 0` blocks of docs/INBOX115_TRIAGE_20260930.md section 3
//  are open now, translated as golden.  Row 18 (VacuumUnit) has its own test
//  (tests/test_i115b_vacuum.cpp) because VacuumUnit.h pulls Public/HTEditList.h,
//  which cannot share a TU with aHotPlateSubstrate.h (TList clash).
//
//  Each part proves that the GOLDEN statement now runs, in a direction the old
//  gate could not produce:
//    A  row 14  PowerSavingMode.cpp TMtrModule::Doing -> fHome->GaliMotorServoOff
//               (golden :369), reached through TPowerSaving::OnScanTmr Task 4 with
//               a TPowerSaving this test creates.  In the product tPowerSaving is
//               NEVER assigned (PowerSavingMode.cpp:516, no `new TPowerSaving`) --
//               asserted below -- so this path is dead at runtime today.
//    C  row 22  ainarm9045.cpp ProcessSCKARTLoadingCount case 1 calls the golden
//               TMySucker::Destroy (golden :7319): with the DUMMY vacuum timer the
//               cursor now WAITS at 1 (the old gate finished at once).
//    D  row 24  ainarm9045.cpp TrayYDirForArmYPitch second route (golden
//               :4856-4859): LoadTrayCanUse8Suck()==1 && bSearchLastMode -> true.
//               The YPitch==0 case is NOT run -- golden divides by zero there
//               (SIGFPE), which is kept and written on the source line.
//    E  row 33  cinitial.cpp GetIndexParm (golden :13713-13814): with
//               bGali_CardInstall the Galil geometry is computed; without it
//               nothing changes (the port state today: nothing sets the flag).
//    F  row 45  csystem.cpp ProcessCCDLight CC_SIGURD_HUKOU arm (golden
//               :19206-19210): a safe door open -> SwCCDLight ON.
//    G  row 46  csystem.cpp DoSystem hot-gun ladder (golden :4566-4605): gun-1
//               flow sensor off -> WAR15195 + SystemStart=false.
//    H  row 53  csystem.cpp DoInitialCylinderCheck finish block (golden
//               :25408-25477): 5-run averages land in fSmartDiagnostic, and a limit
//               breach calls fMain->Pause("DoInitialCylinderCheck") -- which is
//               still the TfMain::Pause shell (forms/fMain.cpp:246), so no real
//               pause happens yet.
//    I  rows 91/92  uHeaterThread.cpp CheckHeater GATE 7 (golden :1136 / :1259):
//               the DUT cooling fan is written with HasAreaOverAmbientTemp()
//               (was: forced 0).  In Tempture_Hot golden writes it OFF again at the
//               end of the SAME pass (golden :1342-1346), so the test records the
//               1203 output writes to see the lifted one.  SHIP BUILD ONLY -- under
//               SOFT_SIMULTE CheckHeater returns at its first line, exactly as
//               golden does; that part prints SKIP there.
//
//  Writes nothing on disk: ProcessSCKARTLoadingCount / DoSystem / CheckHeater
//  only reach the ShowErrorMessage capture and RecordProcess; the run refuses to
//  start outside ctest's redirect environment (w906_ctest_guard.h).
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "ainarm9045.h"
#include "aHotPlateSubstrate.h"     // InArmSuck / InArmSuckUse / LoadTrayCanUse8Suck
#include "csystem.h"                // ProcessCCDLight / CheckSafeDoor_1 / DoSystem hooks
#include "cinitial.h"
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/myGALILmotor.h"     // bGali_CardInstall
#include "cprod.h"                  // Prod / TestIF / TestIF_File / UserDefForm / TrayForm / Temperature
#include "cpublic.h"
#include "cmydef.h"
#include "Config.h"                 // IniConfig
#include "CosFunction.h"
#include "canary_support.h"         // ShowErrorMessage capture, LastSet
#include "atester_shims.h"          // fContact
#include "forms/fHome.h"
#include "forms/fMain.h"
#include "forms/fSmartDiagnostic.h"
#include "PowerSavingMode.h"
#include "uHeaterThread.h"
#include "bthermo.h"                // bGetHeaterUsed
#include "myswitch.h"
#include "mysensor.h"
#include "mycylin.h"
#include "IOBackend.h"              // TIOBackend (part I records the fan output writes)
#include "MyLaneIo.h"               // MyLaneIO.SetBackends
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"

#include <cstdio>
#include <string>
#include <vector>

// --- names no includable header declares ------------------------------------
extern bool ProcessSCKARTLoadingCount(bool bReset);                           // ainarm9045.cpp:1417 (golden :7305)
extern bool TrayYDirForArmYPitch();                                           // ainarm9045.cpp:7497 (golden :4831)
extern void GetIndexParm();                                                   // cinitial.cpp:16185 (golden :13713)
extern void DoSystem();                                                       // csystem.cpp:16308
extern void DoInitialCylinderCheck();                                         // csystem.cpp:25483

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_i115b_lifted.cpp:%d]  %s\n", line, what); }
    else     {           std::printf("  ok    %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- hooks --------------------------------------------------------------------
static int         g_stopCalls = 0;
static std::string g_stopWhy;
static void FakeStop1203(const char* why) { ++g_stopCalls; g_stopWhy = why ? why : ""; }

static std::vector<std::string> g_codes;
static int RecordCode(const char* code, int, int) { g_codes.push_back(code ? code : ""); return 0; }
static bool Saw(const char* code)
{
    for (size_t i = 0; i < g_codes.size(); ++i) if (g_codes[i] == code) return true;
    return false;
}

// A sensor whose read is decided by Type alone: Enable=true and an ISABase no
// branch of TMySensor::IsOn / IsOff matches, so `ret` stays false
// (mysensor.cpp:117-205).  IsOn() == (Type==0), IsOff() == (Type!=0).
static void FixSensor(int idx, bool on)
{
    Sen[idx].Enable  = true;
    Sen[idx].ISABase = 99;
    Sen[idx].Type    = on ? 0 : 1;
}
static void ReleaseSensor(int idx) { Sen[idx].Enable = false; }

// Part I: every 1203 output write, so the fan command INSIDE one CheckHeater pass is visible.  Golden turns the
// DUT fan OFF again at the end of the same pass in Tempture_Hot mode (golden uHeaterThread.cpp:1342-1346 /
// port :1742-1746), so the switch's final state cannot show what the lifted line wrote.
struct TRecBackend : public TIOBackend
{
    std::vector<int> fan;                                                     // values written to the fan point, in order
    int r, ip, port, bit;
    TRecBackend() : r(0), ip(77), port(5), bit(3) {}
    int WriteBit(int Ring, int IP, int Port, int Bit, int Value) override
    {
        if (Ring == r && IP == ip && Port == port && Bit == bit) fan.push_back(Value);
        return 0;
    }
};
static TRecBackend g_rec;

int main()
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("I115B_Lifted", rt)) return 2;
    W906_ShowErrorMessage_Hook = &RecordCode;

    // -------------------------------------------------------------------------
    //  PART E -- row 33 GetIndexParm (runs first: MOT[].Motor must be NULL so
    //  Gali_ReadPos takes its offline value 0, Motor/myGALILmotor.cpp:3632)
    // -------------------------------------------------------------------------
    std::printf("\n-- PART E: row 33 GetIndexParm (golden cinitial.cpp:13713-13814) --\n");
    CHECK(MOT[MTestZ1].Motor == 0 && MOT[MTestZ2].Motor == 0);
    CHECK(bGali_CardInstall == false);                                        // nothing in the port calls Open_GaliCard
    Prod.All_TestZ_Test_Safe = 1000; Prod.TestY1_Front = 500; Prod.TestY1_Middle = 200;
    Prod.TestZ1_Safe = 1500; Prod.TestZ2_Test = 4000; Prod.TestZ1_Test = 3500;
    Prod.TestY2_Middle = 900; Prod.TestY2_Rear = 100; Prod.TestZ1_Place = 1800; Prod.TestZ2_Place = 1900;
    Prod.TestY_Pre_MovePos = 77;
    TestIF_File.iShuttleMode = 0;
    Z1Safe = -1; XShiftF = -1; Z1Up = -1; Z2Down = -1; AxisY_Pre_MovePos = -1; Z1DownToShuttle = -1;
    GetIndexParm();                                                           // flag false -> golden body skipped
    CHECK(Z1Safe == -1 && XShiftF == -1 && Z1Up == -1 && Z2Down == -1);
    bGali_CardInstall = true;
    GetIndexParm();
    CHECK(Z1Safe == 1000);                                                    // All_TestZ_Test_Safe - Gali_ReadPos(0)
    CHECK(XShiftF == 300);                                                    // TestY1_Front - TestY1_Middle
    CHECK(Z1Up == 500 && Z2DownSafe == -500);
    CHECK(Z2Down == 3000 && Z1Down == 2500);                                  // non-DEBUG_GALIL arm
    CHECK(XShiftR == 800 && Z2Up == 500 && Z1DownSafe == -500);
    CHECK(Z2Down2Speed == 5000 && Z1Down2Speed == 5000);
    CHECK(AxisY_Pre_MovePos == 77);
    CHECK(Z1DownToShuttle == 300 && Z2DownToShuttle == 400);
    bGali_CardInstall = false;

    W906_TestEnsureSimMotors();                                               // golden boot invariant for the DoSystem / GaliMotorServoOff parts

    // -------------------------------------------------------------------------
    //  PART A -- row 14 power saving -> TfHome::GaliMotorServoOff
    // -------------------------------------------------------------------------
    std::printf("\n-- PART A: row 14 TMtrModule::Doing -> GaliMotorServoOff (golden PowerSavingMode.cpp:369) --\n");
    CHECK(tPowerSaving == 0);                                                 // product: never assigned -> path dead today
    CHECK(fHome != 0 && fMain != 0 && fContact != 0);
    InitialOK = true; SystemStart = false; iHome = 0;
    if (fContact) fContact->fShow = false;
    IniConfig.bPowerSaveFunction   = true;
    IniConfig.bC05_PowerSaveMotor  = true;
    IniConfig.bC05_PowerSaveTemp   = false;
    IniConfig.bC05_PowerSaveVacuum = false;
    IniConfig.iPowersaveMode       = 0;
    IniConfig.iHaltTime_Motor      = 1;
    {
        TPowerSaving* ps = new TPowerSaving();
        ps->tModule->AlarmTmr   = EncodeTime(0, 0, 0, 0);                    // the halt times are minutes; the test does not wait
        ps->MtrModule->AlarmTmr = EncodeTime(0, 0, 0, 0);
        W906_Stop1203AllHook = &FakeStop1203; g_stopCalls = 0; g_stopWhy = "";
        bPowersaving = false; bMotorPowerState = true; fAllMotorHome = true;
        for (int tick = 0; tick < 40 && g_stopCalls == 0; ++tick)
        {
            ps->OnScanTmr(0);
            Sleep(20);
        }
        CHECK(g_stopCalls == 1);
        CHECK(g_stopWhy == "GaliMotorServoOff - Motor enter power saving mode.");
        CHECK(bPowersaving == true);                                         // golden :371
        CHECK(bMotorPowerState == false && fAllMotorHome == false);          // GaliMotorServoOff body ran
        CHECK(ps->MtrModule->Enabled == false);                              // Task 4 bookkeeping
        W906_Stop1203AllHook = 0;
        delete ps;
    }

    // -------------------------------------------------------------------------
    //  PART C -- row 22 ProcessSCKARTLoadingCount -> TMySucker::Destroy
    // -------------------------------------------------------------------------
    std::printf("\n-- PART C: row 22 ProcessSCKARTLoadingCount case 1 Destroy (golden ainarm9045.cpp:7319) --\n");
    iLoadPickX = 0; iLoadPickY = 0;
    InArmSuckUse[0][0] = false;
    LastSet.iRealDummy = DUMMY;                                               // DUMMY arm: VacuumOffTime timer (mykitsuck.cpp:2425-2475)
    iHome = 0;
    InArmSuck.Suck[0][0].ReStart();                                           // OffTask=1 (golden MyKitSuck.cpp:1849-1853)
    InArmSuck.Suck[0][0].VacuumOffTime = 120;
    InArmSuck.Suck[0][0].iNozzleEvent = 7;                                    // sentinel: Destroy() writes 2 on entry, 0 when done
    ProcessSCKARTLoadingCount(true);
    CHECK(iProcessSCKARTLoadingCountTask == 1);
    ProcessSCKARTLoadingCount(false);
    CHECK(InArmSuck.Suck[0][0].iNozzleEvent == 2);                            // Destroy() entered (mykitsuck.cpp:2415)
    CHECK(iProcessSCKARTLoadingCountTask == 1 && InArmSuckUse[0][0] == false);// the old gate: done at once, Task=100
    {
        int ticks = 0;
        while (iProcessSCKARTLoadingCountTask == 1 && ticks < 200) { Sleep(5); ProcessSCKARTLoadingCount(false); ++ticks; }
        CHECK(iProcessSCKARTLoadingCountTask == 100);
        CHECK(InArmSuckUse[0][0] == true);                                    // golden :7321
        CHECK(InArmSuck.Suck[0][0].iNozzleEvent == 0);                        // Destroy() reported done (iHome!=1)
        CHECK(ticks > 1);                                                     // it took more than one call
    }

    // -------------------------------------------------------------------------
    //  PART D -- row 24 TrayYDirForArmYPitch second route
    // -------------------------------------------------------------------------
    std::printf("\n-- PART D: row 24 TrayYDirForArmYPitch LoadTrayCanUse8Suck route (golden ainarm9045.cpp:4856-4859) --\n");
    {
        const bool savedAuto = USE_IN_Y_IS_AUTO_PITCH;
        USE_IN_Y_IS_AUTO_PITCH = false;
        bInArmPickErrFromLoader = false;
        TrayForm.Loader.iTrayType = 0;
        UserDefForm[0].YPitch = 4000.0;
        TestIF.iARM_Y_PITCH = 8000;                                           // 8000 % 4000 == 0 -> LoadTrayCanUse8Suck()==1
        Prod.LoadForm.iYPitch = 4000;                                         // != iARM_Y_PITCH -> not the :4852 route
        CHECK(LoadTrayCanUse8Suck() == 1);
        TestIF.bSearchLastMode = true;
        CHECK(TrayYDirForArmYPitch() == true);                                // was false while k7-G2 was gated
        TestIF.bSearchLastMode = false;
        CHECK(TrayYDirForArmYPitch() == false);
        UserDefForm[0].YPitch = 3000.0;                                       // 8000 % 3000 != 0
        TestIF.bSearchLastMode = true;
        CHECK(TrayYDirForArmYPitch() == false);
        //AI(W906-YPITCH0) 20260930: a zero / sub-1 loader tray YPitch must not divide by zero (SIGFPE would kill the process)
        //  and must answer "cannot pick 8 at once" -- not 1, which a clamp-to-1 would give (8000 % 1 == 0).
        UserDefForm[0].YPitch = 0.0;
        CHECK(LoadTrayCanUse8Suck() == 0);
        CHECK(TrayYDirForArmYPitch() == false);
        UserDefForm[0].YPitch = 0.4;                                          // golden's `int TrayYPitch=` truncates this to 0
        CHECK(LoadTrayCanUse8Suck() == 0);
        UserDefForm[0].YPitch = -4000.0;
        CHECK(LoadTrayCanUse8Suck() == 0);
        UserDefForm[0].YPitch = 4000.0;                                       // control: a valid pitch still answers 1
        CHECK(LoadTrayCanUse8Suck() == 1);
        USE_IN_Y_IS_AUTO_PITCH = savedAuto;
    }

    // -------------------------------------------------------------------------
    //  PART F -- row 45 ProcessCCDLight CC_SIGURD_HUKOU chamber lamp
    // -------------------------------------------------------------------------
    std::printf("\n-- PART F: row 45 ProcessCCDLight CC_SIGURD_HUKOU (golden csystem.cpp:19206-19210) --\n");
    {
        const int savedCC = CUSTOMER_CODE;
        REAL_TIME_CCD = false;
        IniConfig.bEnableCCDUSETCPIP = false;
        bHeaterDoorIsOpen[0] = false; bHeaterDoorIsOpen[1] = false;
        CUSTOMER_CODE = CC_SIGURD_HUKOU;
        CHECK(CheckSafeDoor_1() == true);                                     // no door sensor installed -> all closed
        SW[SwCCDLight].OutValue = true;
        ProcessCCDLight();
        CHECK(SW[SwCCDLight].OutValue == false);                              // closed -> golden final else
        FixSensor(iSafeDoor[0], false);                                       // door 1 reads OFF == open
        CHECK(CheckSafeDoor_1() == false);
        SW[SwCCDLight].OutValue = false;
        ProcessCCDLight();
        CHECK(SW[SwCCDLight].OutValue == true);                               // golden :19208 -- the lifted arm
        CUSTOMER_CODE = savedCC == CC_SIGURD_HUKOU ? 0 : savedCC;
        SW[SwCCDLight].OutValue = true;
        ProcessCCDLight();
        CHECK(SW[SwCCDLight].OutValue == false);                              // other customers: door open does not light it
        ReleaseSensor(iSafeDoor[0]);
        CUSTOMER_CODE = savedCC;
    }

    // -------------------------------------------------------------------------
    //  PART G -- row 46 DoSystem hot-gun flow ladder
    // -------------------------------------------------------------------------
    std::printf("\n-- PART G: row 46 DoSystem hot-gun flow alarms (golden csystem.cpp:4566-4605) --\n");
    {
        Temperature.bActiveHeatGun = true;
        bUseHotGunCheck = true; bUseHotGunFlowCheck = false; HotGunFlowEnable = false;
        FixSensor(SnHotGun1, false);                                          // gun 1 flow sensor OFF
        CHECK(CheckHotGun() == 1);
        g_codes.clear();
        InitialOK = true; SystemStart = true;
        DoSystem();
        CHECK(Saw("WAR15195"));                                               // golden :4570
        CHECK(!Saw("WAR15196"));
        CHECK(SystemStart == false);                                          // golden :4571
        bUseHotGunCheck = false;                                              // control: the same machine without the check
        g_codes.clear();
        SystemStart = true;
        DoSystem();
        CHECK(!Saw("WAR15195"));
        ReleaseSensor(SnHotGun1);
        Temperature.bActiveHeatGun = false;
        SystemStart = false;
    }

    // -------------------------------------------------------------------------
    //  PART H -- row 53 DoInitialCylinderCheck finish block
    // -------------------------------------------------------------------------
    std::printf("\n-- PART H: row 53 DoInitialCylinderCheck averages + limit -> fMain->Pause (golden csystem.cpp:25408-25477) --\n");
    {
        const int pushSen[10] = { SnLoaderEdgePush, SnLoaderFixCyPush, SenEmptyFixCyPush, SenColorFixCyPush,
                                  SnAuto1EdgePush, SnAuto1FixCyPush, SnAuto2EdgePush, SnAuto2FixCyPush,
                                  SnAuto3EdgePush, SnAuto3FixCyPush };
        const int cyl[10] = { C_LoaderEdgePush, C_TrayY_Fixer, C_Empty_Fix, C_Color_Fix, C_Auto1EdgePush,
                              C_Auto1Side_Fixer, C_Auto2EdgePush, C_Auto2Side_Fixer, C_Auto3EdgePush, C_Auto3Side_Fixer };
        for (int i = 0; i < 10; ++i) { FixSensor(pushSen[i], true); Cylinder[cyl[i]].Enable = false; }   // pushed at once; disabled cylinders read "back" (mycylin.cpp:212)
        CHECK(fSmartDiagnostic != 0);
        for (int pass = 0; pass < 2; ++pass)
        {
            const bool wantErr = (pass == 1);
            iInitialCylinderCheckTask = 1; bInitialCylinderCheck = true;
            DoInitialCylinderCheck();                                         // case 1: ReadCylinderData + grid reset
            for (int i = 0; i < 10; ++i)
            {
                fSmartDiagnostic->iPushAvgTime[i] = -1; fSmartDiagnostic->iPopAvgTime[i] = -1;
                fSmartDiagnostic->iPush_UpLimitTime[i] = 100000; fSmartDiagnostic->iPush_LowLimitTime[i] = 0;
                fSmartDiagnostic->iPop_UpLimitTime[i]  = 100000; fSmartDiagnostic->iPop_LowLimitTime[i]  = 0;
                fSmartDiagnostic->iWarPercent[i] = 100;
            }
            if (wantErr) fSmartDiagnostic->iPush_UpLimitTime[1] = 1;          // any real push time breaks it
            const int pause0 = fMain->W906_PauseCallCount;
            int ticks = 0;
            while (bInitialCylinderCheck && ticks < 6000) { Sleep(5); DoInitialCylinderCheck(); ++ticks; }
            CHECK(bInitialCylinderCheck == false);                            // 5 runs finished (golden :25406)
            CHECK(fSmartDiagnostic->iPushAvgTime[1] > 0);                     // golden :25419 -- the lifted averaging
            CHECK(fSmartDiagnostic->iPopAvgTime[1] > 0);
            CHECK(fSmartDiagnostic->iPopAvgTime[0] == 0);                     // golden records no pop time for cylinder 0
            if (wantErr)
            {
                CHECK(fMain->W906_PauseCallCount == pause0 + 1);              // golden :25475
                CHECK(fMain->W906_PauseLastFunc == "DoInitialCylinderCheck");
            }
            else
            {
                CHECK(fMain->W906_PauseCallCount == pause0);                  // inside every limit: no pause
            }
        }
        for (int i = 0; i < 10; ++i) ReleaseSensor(pushSen[i]);
    }

    // -------------------------------------------------------------------------
    //  PART I -- rows 91/92 CheckHeater GATE 7 (ship build only)
    // -------------------------------------------------------------------------
    std::printf("\n-- PART I: rows 91/92 CheckHeater DUT fan follows HasAreaOverAmbientTemp (golden uHeaterThread.cpp:1136 / :1259) --\n");
#ifdef SOFT_SIMULTE
    std::printf("  SKIP  SOFT_SIMULTE build: CheckHeater returns at uHeaterThread.cpp:505-511 (golden :138-144), GATE 7 is unreachable here\n");
#else
    {
        LastSet.iTemperature = Tempture_Hot;
        TestIF.iTestMode = SingleSite; IniConfig.bD30EnableSiteModeSelect = false;
        IniConfig.bL07UseSingleTenmpertureLimit = true;
        IniConfig.bL12TempErrNoCloseHeater = false;
        IniConfig.bL28TempOfsUseReadyTempRange = false;
        CosFunction.bUseIndividulTempSet = false;
        Temperature.fWorkTemperBase = 100.0;
        Temperature.bBoostFuncttion = false; Temperature.bLBTempFunction = false;
        for (int i = 0; i < tcTotalCount; ++i) { bUT150Install[i] = false; UN150Read[i] = 25.0; }
        bUT150Install[tcHead1] = true;
        CHECK(bGetHeaterUsed(tcHead1) == true);
        SW[SwHeaterRelay].Enable = true; SW[SwHeaterRelay].ISABase = 99; SW[SwHeaterRelay].Type = 1; SW[SwHeaterRelay].OutValue = true;
        iHome = 0; SystemStart = true;
        MyLaneIO.SetBackends(&g_rec, 0, 0);                                   // 1203 writes -> the recorder (part I is the last part)
        SW[SwDutHeaterCoolFan].Enable = true; SW[SwDutHeaterCoolFan].ISABase = ePCI1203; SW[SwDutHeaterCoolFan].Type = 1;
        SW[SwDutHeaterCoolFan].Ring = g_rec.r; SW[SwDutHeaterCoolFan].IP = g_rec.ip;
        SW[SwDutHeaterCoolFan].Port = g_rec.port; SW[SwDutHeaterCoolFan].Bit = g_rec.bit;
        struct Case { double limit; double read; bool fan; const char* what; };
        const Case cases[2] = {
            { 5.0, 110.0, true,  "head 110 > 100+5 and >= 100+2 -> HasAreaOverAmbientTemp true -> fan ON" },
            { -5.0, 97.0, false, "head 97 > 100-5 but < 100+2 -> HasAreaOverAmbientTemp false -> fan OFF" },
        };
        AnsiString code; code.sprintf("WAR15%02d", tcHead1 + 100);           // both sites raise it next to the fan line (golden :1141 / :1258)
        int sec = 10;
        for (int c = 0; c < 2; ++c)
        {
            std::printf("  case: %s\n", cases[c].what);
            IniConfig.dSingleTempLimit[tcHead1] = cases[c].limit;
            UN150Read[tcHead1] = cases[c].read;
            CHECK(HasAreaOverAmbientTemp() == cases[c].fan);
            // site 1 (golden :1136): fHeaterOK true, one channel over, >30 s -- the code is raised right AFTER the fan line.
            // Writes of the pass that ran it, expected: [ HasAreaOverAmbientTemp() (the lifted line), 0 (golden :1345) ].
            // With the old gate the first entry was always 0.
            fHeaterOK = true;
            g_codes.clear();
            int k1 = 0;
            for (; k1 < 45 && !Saw(code.c_str()); ++k1)
            {
                SystemSec = (Word)(sec++ % 60);
                g_rec.fan.clear();
                CheckHeater();
            }
            CHECK(Saw(code.c_str()) && fHeaterOK == false);                  // site 1 ran
            CHECK(k1 > 30);                                                   // only after the 30 s window (golden :1127)
            CHECK(g_rec.fan.size() == 2);
            CHECK(g_rec.fan.size() == 2 && g_rec.fan[0] == (cases[c].fan ? 1 : 0));   // golden :1136
            CHECK(g_rec.fan.size() == 2 && g_rec.fan[1] == 0);                // golden :1345 -- same pass, fan OFF again (Hot mode)
            CHECK(SW[SwDutHeaterCoolFan].OutValue == false);
            // site 2 (golden :1259): fHeaterOK false, over temp, iAlarmSecond > 30 -- the code is raised just BEFORE the fan line
            g_codes.clear();
            int k2 = 0;
            for (; k2 < 45 && !Saw(code.c_str()); ++k2)
            {
                SystemSec = (Word)(sec++ % 60);
                g_rec.fan.clear();
                CheckHeater();
            }
            CHECK(Saw(code.c_str()) && k2 > 30);
            CHECK(g_rec.fan.size() == 2 && g_rec.fan[0] == (cases[c].fan ? 1 : 0));   // golden :1259
            CHECK(g_rec.fan.size() == 2 && g_rec.fan[1] == 0);                // golden :1345
        }
        SW[SwDutHeaterCoolFan].Enable = false;
        bUT150Install[tcHead1] = false;
        SW[SwHeaterRelay].Enable = false;
        SystemStart = false;
    }
#endif

    W906_ShowErrorMessage_Hook = 0;
    std::printf("\n%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
