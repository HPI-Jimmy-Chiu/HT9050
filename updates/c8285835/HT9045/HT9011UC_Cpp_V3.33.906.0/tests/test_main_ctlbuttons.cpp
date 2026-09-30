// ===========================================================================
//  tests/test_main_ctlbuttons.cpp
//
//  AI(W906-FLOW-4) 20260930: INBOX 109 -- the main screen's control buttons, end to end on the C++ side:
//    WS entry W906_Main_EvB6Op("act.main.ctlButton", ...)  (FileRW/MainClick.cpp, St01 -- the same function
//      tools/wb_serve.cpp's dispatch calls; compiled into this test as it is)
//    -> St01's per-button queue (3 s TTL, CtlBtnEnabled = golden ProcessKeyFlush)
//    -> W906_MainCtlButtonTick()  (WebMainCtlButtons.cpp, what wb_serve's main loop calls every pass)
//    -> golden TfMain::BtnOneCycleClick / BtnTrayEndClick / BtnAlarmResetClick  (cCleanOut.cpp, 906 main.cpp:4332 / :13944 /
//       :22159; linked from ht9045_sm like wb_serve does)
//
//    [1] ONE CYCLE click -> one tick runs golden BtnOneCycleClick ONCE: BtnOneCycle->Down, bManualOneCycle, SECS DoOneCycle;
//        the queue is drained (Take returns false, op:get pending 0) and a second tick runs nothing
//    [2] TRAY FEED click -> golden BtnTrayEndClick -> InitialTrayFeedTask: iTrayFeed=1 iTrayFeedTask=1, counter +1
//    [3] ALARM RESET: two clicks -> two body runs (two SECS DoAlarmReset events), queue drained
//    [4] DISABLED: SystemStart -> St01 refuses the TRAY FEED / ALARM RESET click ("disabled"), nothing queued, tick runs 0
//    [5] ONE CYCLE's own golden guard: fAllMotorHome=false -> the body runs (ONE CYCLE is always Enabled in golden
//        ProcessKeyFlush) but returns at its first line: nothing changes
//    [6] EXPIRED: a click left > 3 s is dropped by St01's queue: the tick runs nothing
//    [7] RESET and the site cell are NOT taken by the consumer (decision for the user): their events stay queued
//  Writes: the bodies' NewRecordProcess / RecordProcess rows (cMyDB log roots) -- ctest points them into machine_log_scratch;
//  the guard below refuses to run anywhere else.  No motor / IO / cylinder command is issued by any of the three bodies.
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "forms/fMain.h"
#include "forms/fSecurity.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "common.h"                   // as9045LogPath / asSaveEventLogPath
#include "SECSGEM/SecsEventReport.h"  // g_SimLastEventReportCeid / g_SimEventReportCount
#include "SECSGEM/SecsEventType.h"    // SECS_EVENT
#include "FileRW/MainClickTail.h"     // W906_Main_TakeCtlButtonEvent / W906_Main_TakeSiteClickEvent
#include "w906_ctest_guard.h"

std::string W906_Main_EvB6Op(const std::string& cmd, const std::string& payloadJson, bool* ok);   // FileRW/MainClick.cpp
extern int iMaxLevelItem;                                                                         // cSecurity.cpp:47
int W906_MainCtlButtonTick();                                                                     // WebMainCtlButtons.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_main_ctlbuttons.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string Click(const char* button, bool* ok)
{
    std::string payload = std::string("{\"op\":\"click\",\"button\":\"") + button + "\"}";
    bool k = false;
    const std::string r = W906_Main_EvB6Op("act.main.ctlButton", payload, &k);
    if (ok) *ok = k;
    std::printf("  click %-10s -> ok=%d %s\n", button, (int)k, r.substr(0, 200).c_str());
    return r;
}
static bool Has(const std::string& s, const char* needle) { return s.find(needle) != std::string::npos; }
static std::string Get()
{
    bool k = false;
    return W906_Main_EvB6Op("act.main.ctlButton", "{\"op\":\"get\"}", &k);
}

// the state a homed, idle, empty machine is in (what the golden button guards look at)
static void IdleHomed()
{
    SystemStart = false;
    SoftStart = false;
    fAllMotorHome = true;
    iOneCycle = 0;
    iCleanOut = 0;
    iTrayFeed = 0;
    iTrayFeedTask = 0;
    iHome = 0;
    iReset = 0;
    bSECSGEMAlarm = false;
    bEnableEmployeeIDCheck = false;
    bManualOneCycle = false;
    bDoEmptySocketOneCycle = false;
    bIsAutoOneCycle = false;
    fMain->BtnOneCycle->Down = false;
    IniConfig.bEnable_SECS_GEM = true;       // so the golden EventReport lines are observable (a counter, SecsEventReport.cpp)
    IniConfig.bO01_ResetNeedClearAndCheckHP = false;
    IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed = false;
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("MainCtlButtons", rt))
        return 2;
    CHECK(fMain != 0);
    CHECK(fMain != 0 && fMain->BtnOneCycle != 0);
    if (fMain == 0 || fMain->BtnOneCycle == 0) { std::printf("FAIL: no fMain\n"); return 1; }
    InitialOK = true;

    // ---- [1] ONE CYCLE ---------------------------------------------------------------------------------------------
    std::printf("[1] ONE CYCLE click -> golden BtnOneCycleClick once\n");
    IdleHomed();
    CHECK(W906_MainCtlButtonTick() == 0);                                       // nothing pending yet
    ResetSimEventReport();
    bool ok = false;
    std::string r = Click("oneCycle", &ok);
    CHECK(ok);
    CHECK(Has(r, "\"eventOnly\":true"));
    CHECK(fMain->BtnOneCycle->Down == false);                                   // St01's side does NOT run the body
    CHECK(W906_MainCtlButtonTick() == 1);
    CHECK(fMain->BtnOneCycle->Down == true);                                    // golden :4374
    CHECK(bManualOneCycle == true);                                             // golden :4376
    CHECK(iOneCycle == 0);                                                      // InitOneCycle (csystem.cpp:391) zeroes it; MainProc latches it later
    CHECK(g_SimEventReportCount == 1 && g_SimLastEventReportCeid == SECS_EVENT.DoOneCycle);   // golden :4377-4378
    CHECK(W906_Main_TakeCtlButtonEvent("oneCycle") == false);                   // drained
    CHECK(Has(Get(), "\"oneCycle\":0"));
    CHECK(W906_MainCtlButtonTick() == 0);                                       // once, not every tick
    CHECK(g_SimEventReportCount == 1);

    // ---- [2] TRAY FEED ---------------------------------------------------------------------------------------------
    std::printf("[2] TRAY FEED click -> golden BtnTrayEndClick -> InitialTrayFeedTask\n");
    IdleHomed();
    const int cnt0 = fMain->W906_BtnTrayEndClickCallCount;
    r = Click("trayFeed", &ok);
    CHECK(ok);
    CHECK(iTrayFeed == 0 && iTrayFeedTask == 0);
    CHECK(W906_MainCtlButtonTick() == 1);
    CHECK(fMain->W906_BtnTrayEndClickCallCount == cnt0 + 1);
    CHECK(iTrayFeed == 1);                                                      // golden main.cpp:2372-2376 (or :2344-2348 for AutoSiteMap)
    CHECK(iTrayFeedTask == 1);
    CHECK(iCleanOut == 0 && iHome == 0 && iReset == 0);
    CHECK(W906_Main_TakeCtlButtonEvent("trayFeed") == false);
    CHECK(W906_MainCtlButtonTick() == 0);

    // ---- [3] ALARM RESET x2 ----------------------------------------------------------------------------------------
    std::printf("[3] ALARM RESET clicked twice -> golden BtnAlarmResetClick twice\n");
    IdleHomed();
    ResetSimEventReport();
    Click("alarmReset", &ok);
    CHECK(ok);
    Click("alarmReset", &ok);
    CHECK(ok);
    CHECK(Has(Get(), "\"alarmReset\":2"));
    CHECK(W906_MainCtlButtonTick() == 2);
    CHECK(g_SimEventReportCount == 2 && g_SimLastEventReportCeid == SECS_EVENT.DoAlarmReset);   // golden :22162-22163
    CHECK(Has(Get(), "\"alarmReset\":0"));
    CHECK(W906_MainCtlButtonTick() == 0);

    // ---- [4] DISABLED ----------------------------------------------------------------------------------------------
    std::printf("[4] SystemStart -> TRAY FEED / ALARM RESET are Enabled=false (golden ProcessKeyFlush): refused, nothing runs\n");
    IdleHomed();
    SystemStart = true;
    ResetSimEventReport();
    const int cnt1 = fMain->W906_BtnTrayEndClickCallCount;
    r = Click("trayFeed", &ok);
    CHECK(!ok);
    CHECK(Has(r, "\"guard\":\"disabled\""));
    r = Click("alarmReset", &ok);
    CHECK(!ok);
    CHECK(W906_MainCtlButtonTick() == 0);
    CHECK(fMain->W906_BtnTrayEndClickCallCount == cnt1);
    CHECK(iTrayFeed == 0 && iTrayFeedTask == 0);
    CHECK(g_SimEventReportCount == 0);
    SystemStart = false;

    // ---- [5] ONE CYCLE's own golden guard -----------------------------------------------------------------------
    std::printf("[5] ONE CYCLE while not homed -> the body runs and returns at golden :4334\n");
    IdleHomed();
    fAllMotorHome = false;
    ResetSimEventReport();
    r = Click("oneCycle", &ok);
    CHECK(ok);                                                                  // BtnOneCycle is not in ProcessKeyFlush's list
    CHECK(W906_MainCtlButtonTick() == 1);
    CHECK(fMain->BtnOneCycle->Down == false);
    CHECK(bManualOneCycle == false);
    CHECK(g_SimEventReportCount == 0);

    // ---- [6] EXPIRED -----------------------------------------------------------------------------------------------
    std::printf("[6] a click left in the queue longer than 3 s is dropped (St01 kEvTtlMs)\n");
    IdleHomed();
    ResetSimEventReport();
    Click("alarmReset", &ok);
    CHECK(ok);
    ::Sleep(3300);
    CHECK(W906_MainCtlButtonTick() == 0);
    CHECK(g_SimEventReportCount == 0);
    CHECK(Has(Get(), "\"alarmReset\":0"));

    // ---- [7] RESET / site cell are left to St01's queue -------------------------------------------------------------
    std::printf("[7] RESET and the site cell are not taken by the consumer (not wired: decision)\n");
    IdleHomed();
    r = Click("reset", &ok);
    CHECK(ok);
    CHECK(W906_MainCtlButtonTick() == 0);
    CHECK(Has(Get(), "\"reset\":1"));                                           // still pending -> expires as before FLOW-4
    CHECK(W906_Main_TakeCtlButtonEvent("reset") == true);                        // (the test takes it itself)
    // St01's site-click guard asks golden fSecurity->Insufficient(10) (cSecurity.cpp:650): the permission table is what wb_serve's
    // boot W906_SecurityBoot loads (iMaxLevelItem=180) -- give this process the same table size and an operator level that passes item 10
    iMaxLevelItem = 180;
    LevelSet.AccessLevel[10] = 0;
    AccessLevel = 0;
    bool sok = false;
    const std::string sr = W906_Main_EvB6Op("act.main.siteClick", "{\"op\":\"click\",\"x\":0,\"y\":0}", &sok);
    std::printf("  siteClick -> ok=%d %s\n", (int)sok, sr.substr(0, 200).c_str());
    CHECK(sok);
    if (sok) {
        CHECK(W906_MainCtlButtonTick() == 0);
        int x = -1, y = -1;
        CHECK(W906_Main_TakeSiteClickEvent(&x, &y) == true && x == 0 && y == 0);   // still queued: the consumer did not touch it
    } else {
        std::printf("  (site click refused by St01's golden guards in this state -- %s; the consumer path is not reached)\n",
                    sr.substr(0, 120).c_str());
    }

    std::printf("\n%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
