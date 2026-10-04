// ===========================================================================
//  tests/test_main_scankey.cpp
//
//  //AI(W906-SCANKEY) 20261003: the physical panel keys (golden TfMain::ScanKey main.cpp:2379-2656, its timer
//  TimerScanKeyTimer :31994-32025, and TfHome::ScanKey uhome.cpp:4874-4889), ported in WebMainScanKey.cpp
//  (wb_serve only; compiled into this test exactly as wb_serve compiles it).  The key source is injected: a fake
//  scan() pops a queue and counts its calls (production passes ScanPannelKey, ckernel.cpp:3147).
//  START / PAUSE land in the W906_RemoteRun seat (forms/fMain.h:1440-1447, body forms/fMain.cpp:1374-1376); here the
//  seat is a recorder (pattern: tests/test_pause_forward.cpp:27-30).  fMain points at a FakeMain whose Home /
//  BtnResetClick (both virtual: forms/fMain.h:338 / :230) only record; BtnPauseClick is NOT overridden, so the real
//  forms/fMain.cpp:497 body runs -> Pause (forms/fMain.h:167, virtual, not overridden) -> W906_RemoteRunPause.
//
//    [1] guards (W906_MainScanKeyWith): fNote->fShow / noticeUp / fiosetview->fShow / MyMessageBox->fShow
//        -> scan() is not called (golden :2383-2386)
//    [2] guards (W906_MainScanKeyTickWith): InitialOK=false / bEnableEmployeeIDCheck -> scan() not called
//        (golden :31997-32004); positive control: all clear -> scan() called once
//    [3] START default -> the START seat once with "ScanKey_2" (golden :2516), outside FormLock
//    [4] START while fHome->fShow -> seat not called (golden :2514)
//    [5] PAUSE (bFinishSuckAfterPause=false) -> real BtnPauseClick -> PAUSE seat once with "BtnPauseClick" (golden :2519-2522)
//    [6] HOME: SystemStart=1 -> Home not called (BtnHome->Enabled, golden ProcessKeyFlush); SystemStart=0 ->
//        Home("ScanKey") once and bHomeByStart==false (golden :2545-2562)
//    [7] RESET with SystemStart -> BtnResetClick not called (golden :2569-2570)
//    [8] CLEAN OUT, enabled (SystemStart=0, fAllMotorHome, iOneCycle=0) -> golden CleanOut -> iCleanOut==1, under FormLock
//    [9] TRAY FEED with bUnloading -> nothing (golden :2621-2622)
//   [10] ALARM RESET with bAlarmBuzzer -> bAlarmBuzzer / bLampAlarmReset false afterwards (golden :2626-2631)
//   [11] ONE CYCLE with fContact->fShow -> W906_Contact_OneCycleProcess once (golden :2611-2612; stub counts)
//   [12] the Home hook (W906_HomeScanKeyWith): Timer1 enabled + fHome->fShow + key PAUSE -> sbAbortHomeClick + Close:
//        fHome->fAbort true, fShow false, W906_HomeTimer1Enabled false (uhome.cpp:661 = golden :4871); a second
//        press finds the timer disabled -> scan() not called.  LAST on purpose: sbAbortHomeClick -> GaliMotorServoOff
//        writes SystemStart=false / fAllMotorHome=false (uhome.cpp:4986 / :4995).
//  Writes: NewRecordProcess / RecordProcess rows (cMyDB log roots) -> machine_log_scratch (ENV-ALL); the guard below
//  refuses to run anywhere else.  No motor / IO command reaches hardware: the test process has no card (offline objects).
// ===========================================================================
#include <cstdio>
#include <deque>
#include <string>
#include "forms/fMain.h"              // TfMain (virtual Home :338 / BtnResetClick :230), fMain :1305, W906_RemoteRun :1445
#include "forms/fHome.h"              // fHome :278, fShow :165, fAbort :180
#include "forms/fNote.h"              // fNote :443, fShow :341
#include "atester_shims.h"            // fContact :251 (fShow :157), fiosetview :362 (fShow :359)
#include "mymessbox_shim.h"           // MyMessageBox :28 (fShow :24)
#include "cmydef.h"                   // SnFK* :672-683 (values cmydef.cpp:793-806), InitialOK :220, iCleanOut :255, iTrayFeed :257,
                                      // iHome :260, bOneCycle_BackUp :273, bLampAlarmReset :2571, iTrayFeedTask :2586, bAlarmBuzzer :2589,
                                      // iPauseBackUp :2590 (=-1 cmydef.cpp:2817), bHomeByStart :2738, bRunAutoClean :3199,
                                      // bSECSGEMAlarm :3763, bUnloading :3889, bEnableEmployeeIDCheck :3894, bSECSGEM_NoteAlarm :4327,
                                      // OFFLINE_ALARM :2850
#include "cprod.h"                    // TestIF_File: iAutoClean_Function :1697, bUseBarcodeAutoAdjustLight :2025
#include "Config.h"                   // IniConfig: bEnable_SECS_GEM :92, bEnableAutoCleanFunction :118, bSPILFunction :133,
                                      // bFinishSuckAfterPause :136, bG06HomeinitialCheckZ1 :211, bA17RESETButtonDisable :298,
                                      // bA56EnableAutoTeachFunciton :341, bE53LowYieldAutoClean :621, bI40_bStartProductOnLine :811,
                                      // bI52_bAQLSortMode :850, bP28Auto1OnlyBin1 :1422
#include "CosFunction.h"              // CosFunction: bUsePassBinOnlyCanSetOneBin :144, bOEEFunction :238, bManualSteplAutoTeach :330,
                                      // bUseBarcodeAutoAdjustLight :427
#include "common.h"                   // as9045LogPath / asSaveEventLogPath (as tests/test_main_ctlbuttons.cpp:34)
#include "w906_ctest_guard.h"         // W906TestRequireCtestRedirects :69

void W906_MainScanKeyWith(int (*scan)(), bool noticeUp);        // WebMainScanKey.cpp
void W906_HomeScanKeyWith(int (*scan)());                       // WebMainScanKey.cpp
void W906_MainScanKeyTickWith(int (*scan)(), bool noticeUp);    // WebMainScanKey.cpp
extern bool W906_HomeTimer1Enabled;                             // uhome.cpp:649
extern int  g_W906TestFormLockDepth;                            // tests/test_main_scankey_stubs.cpp
extern int  g_W906TestContactOneCycleCalls;                     // tests/test_main_scankey_stubs.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_main_scankey.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- the key source: a queue; -1 = no key (ScanPannelKey's "nothing pressed")
static std::deque<int> g_keys;
static int g_scanCalls = 0;
static int FakeScan()
{
    ++g_scanCalls;
    if (g_keys.empty()) return -1;
    const int k = g_keys.front();
    g_keys.pop_front();
    return k;
}
static void Press(int key) { g_keys.clear(); g_keys.push_back(key); g_scanCalls = 0; }

// ---- the START / PAUSE seat (forms/fMain.h:1440-1444), recorder (tests/test_pause_forward.cpp:27-30)
static int g_startCalls = 0, g_pauseCalls = 0, g_startLock = -1;
static std::string g_startFunc, g_pauseFunc;
static bool StubStart(AnsiString f) { ++g_startCalls; g_startFunc = f.c_str(); g_startLock = g_W906TestFormLockDepth; return true; }
static bool StubPause(AnsiString f) { ++g_pauseCalls; g_pauseFunc = f.c_str(); return true; }

// ---- fMain stand-in: Home / BtnResetClick record only (both virtual in forms/fMain.h:338 / :230)
struct FakeMain : public TfMain
{
    int homeCalls = 0, resetCalls = 0;
    std::string homeFunc;
    bool Home(AnsiString Func) override { ++homeCalls; homeFunc = Func.c_str(); return true; }
    void BtnResetClick(void*) override { ++resetCalls; }
};

// a homed, idle machine with every optional golden branch of ScanKey switched off
static void Idle()
{
    SystemStart = false;
    SoftStart = false;
    fAllMotorHome = true;
    InitialOK = true;
    bEnableEmployeeIDCheck = false;
    bSECSGEMAlarm = false;
    bSECSGEM_NoteAlarm = false;
    iPauseBackUp = -1;
    iOneCycle = 0;
    iCleanOut = 0;
    iTrayFeed = 0;
    iTrayFeedTask = 0;
    iHome = 0;
    bOneCycle_BackUp = false;
    bRunAutoClean = false;
    bUnloading = false;
    bAlarmBuzzer = false;
    OFFLINE_ALARM = false;
    fMain->BtnOneCycle->Down = false;
    fNote->fShow = false;
    fiosetview->fShow = false;
    MyMessageBox->fShow = false;
    fContact->fShow = false;
    fHome->fShow = false;
    IniConfig.bEnable_SECS_GEM = false;
    IniConfig.bEnableAutoCleanFunction = false;
    IniConfig.bSPILFunction = false;
    IniConfig.bFinishSuckAfterPause = false;
    IniConfig.bG06HomeinitialCheckZ1 = false;
    IniConfig.bA17RESETButtonDisable = false;
    IniConfig.bA56EnableAutoTeachFunciton = false;
    IniConfig.bI52_bAQLSortMode = false;
    IniConfig.bP28Auto1OnlyBin1 = false;
    IniConfig.bI40_bStartProductOnLine = false;
    IniConfig.bE53LowYieldAutoClean = false;
    CosFunction.bUseBarcodeAutoAdjustLight = false;
    CosFunction.bUsePassBinOnlyCanSetOneBin = false;   // ComputeCheckAutoOnlySetOneBin -> false (MainCalcCore.cpp:572)
    CosFunction.bManualSteplAutoTeach = false;         // MskAutoTeachKeyStartEnable -> true
    CosFunction.bOEEFunction = false;
    TestIF_File.iAutoClean_Function = 0;
    TestIF_File.bUseBarcodeAutoAdjustLight = false;
    g_startCalls = g_pauseCalls = 0;
    g_startLock = -1;
    g_startFunc.clear();
    g_pauseFunc.clear();
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("MainScanKey", rt))
        return 2;
    if (fMain == 0 || fHome == 0 || fNote == 0 || fiosetview == 0 || MyMessageBox == 0 || fContact == 0) {
        std::printf("FAIL: a form global is null\n");
        return 1;
    }
    FakeMain* fake = new FakeMain();     // never deleted: fMain points at it for the rest of the process
    fMain = fake;
    W906_RemoteRun.Start = &StubStart;
    W906_RemoteRun.Pause = &StubPause;

    // ---- [1] guards of the scan itself --------------------------------------------------------------------------
    std::printf("[1] a dialog is up -> the keys are not scanned (golden :2383-2386)\n");
    Idle(); fNote->fShow = true;        Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, false); CHECK(g_scanCalls == 0);
    Idle();                             Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, true);  CHECK(g_scanCalls == 0);
    Idle(); fiosetview->fShow = true;   Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, false); CHECK(g_scanCalls == 0);
    Idle(); MyMessageBox->fShow = true; Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, false); CHECK(g_scanCalls == 0);
    CHECK(g_startCalls == 0);

    // ---- [2] guards of the timer --------------------------------------------------------------------------------
    std::printf("[2] TimerScanKeyTimer guards (golden :31997-32004)\n");
    Idle(); InitialOK = false;              Press(SnFKAlarmReset); W906_MainScanKeyTickWith(&FakeScan, false); CHECK(g_scanCalls == 0);
    Idle(); bEnableEmployeeIDCheck = true;  Press(SnFKAlarmReset); W906_MainScanKeyTickWith(&FakeScan, false); CHECK(g_scanCalls == 0);
    Idle(); bSECSGEMAlarm = true;           Press(SnFKAlarmReset); W906_MainScanKeyTickWith(&FakeScan, false); CHECK(g_scanCalls == 0);   // :32006-32012
    Idle();                                 Press(SnFKAlarmReset); W906_MainScanKeyTickWith(&FakeScan, false); CHECK(g_scanCalls == 1);   // control (fHome->fShow false: the home hook does not scan)

    // ---- [3] START ----------------------------------------------------------------------------------------------
    std::printf("[3] START -> W906_RemoteRun.Start(\"ScanKey_2\") once, outside FormLock\n");
    Idle(); Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(g_scanCalls == 1);
    CHECK(g_startCalls == 1 && g_startFunc == "ScanKey_2");
    CHECK(g_startLock == 0);
    CHECK(g_pauseCalls == 0);

    // ---- [4] START while the Home dialog is up --------------------------------------------------------------------
    std::printf("[4] START while fHome->fShow -> no START (golden :2514)\n");
    Idle(); fHome->fShow = true; Press(SnFKStart); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(g_scanCalls == 1 && g_startCalls == 0);
    fHome->fShow = false;

    // ---- [5] PAUSE ----------------------------------------------------------------------------------------------
    std::printf("[5] PAUSE -> TfMain::BtnPauseClick (forms/fMain.cpp:497) -> Pause -> PAUSE seat\n");
    Idle();
    const int bpc0 = fake->W906_BtnPauseClickCallCount;
    Press(SnFKPause); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(fake->W906_BtnPauseClickCallCount == bpc0 + 1);
    CHECK(g_pauseCalls == 1 && g_pauseFunc == "BtnPauseClick");
    CHECK(g_startCalls == 0);
    CHECK(iPauseBackUp == -1);

    // ---- [6] HOME -----------------------------------------------------------------------------------------------
    std::printf("[6] HOME: disabled while SystemStart; else Home(\"ScanKey\") with bHomeByStart=false\n");
    Idle(); SystemStart = true; bHomeByStart = true;
    Press(SnFKHome); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(fake->homeCalls == 0 && bHomeByStart == true);
    Idle(); bHomeByStart = true;
    Press(SnFKHome); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(fake->homeCalls == 1 && fake->homeFunc == "ScanKey");
    CHECK(bHomeByStart == false);

    // ---- [7] RESET while running --------------------------------------------------------------------------------
    std::printf("[7] RESET while SystemStart -> nothing (golden :2569-2570)\n");
    Idle(); SystemStart = true;
    Press(SnFKReset); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(fake->resetCalls == 0);

    // ---- [8] CLEAN OUT ------------------------------------------------------------------------------------------
    std::printf("[8] CLEAN OUT enabled -> golden CleanOut (cCleanOut.cpp:69) -> iCleanOut==1\n");
    Idle();
    Press(SnFKCleanOut); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(iCleanOut == 1);                                  // InitCleanOutFunction (csystem.cpp) via cCleanOut.cpp:112-114
    CHECK(g_W906TestFormLockDepth == 0);                    // MskFormLock released
    Idle(); SystemStart = true;                             // BtnCleanOut->Enabled false (golden ProcessKeyFlush)
    Press(SnFKCleanOut); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(iCleanOut == 0);

    // ---- [9] TRAY FEED while unloading --------------------------------------------------------------------------
    std::printf("[9] TRAY FEED while bUnloading -> nothing (golden :2621-2622)\n");
    Idle(); bUnloading = true;
    Press(SnFKTrayFeed); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(iTrayFeed == 0 && iTrayFeedTask == 0);

    // ---- [10] ALARM RESET ---------------------------------------------------------------------------------------
    std::printf("[10] ALARM RESET with the buzzer on -> buzzer flags cleared\n");
    Idle(); bAlarmBuzzer = true; bLampAlarmReset = true;
    Press(SnFKAlarmReset); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(bAlarmBuzzer == false);
    CHECK(bLampAlarmReset == false);

    // ---- [11] ONE CYCLE on the Contact page ---------------------------------------------------------------------
    std::printf("[11] ONE CYCLE with fContact->fShow -> W906_Contact_OneCycleProcess once\n");
    Idle(); fContact->fShow = true;
    const int oc0 = g_W906TestContactOneCycleCalls;
    Press(SnFKOneCycle); W906_MainScanKeyWith(&FakeScan, false);
    CHECK(g_W906TestContactOneCycleCalls == oc0 + 1);
    fContact->fShow = false;

    // ---- [12] the Home dialog's own PAUSE key (LAST: GaliMotorServoOff clears SystemStart / fAllMotorHome) ------
    std::printf("[12] Home hook: PAUSE -> sbAbortHomeClick + Close, Timer1 disabled after Close (golden uhome.cpp:4871 oddity)\n");
    Idle();
    W906_HomeTimer1Enabled = true;
    fHome->fShow = true;
    fHome->fAbort = false;
    Press(SnFKPause); W906_HomeScanKeyWith(&FakeScan);
    CHECK(g_scanCalls == 1);
    CHECK(fHome->fAbort == true);                           // uhome.cpp:5013
    CHECK(fHome->fShow == false);                           // uhome.cpp:652 (Close)
    CHECK(W906_HomeTimer1Enabled == false);                 // uhome.cpp:661
    fHome->fShow = true;                                    // a later Home dialog: golden never re-enables Timer1
    Press(SnFKPause); W906_HomeScanKeyWith(&FakeScan);
    CHECK(g_scanCalls == 0);
    fHome->fShow = false;

    W906_RemoteRun.Start = 0;
    W906_RemoteRun.Pause = 0;
    std::printf("\n%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
