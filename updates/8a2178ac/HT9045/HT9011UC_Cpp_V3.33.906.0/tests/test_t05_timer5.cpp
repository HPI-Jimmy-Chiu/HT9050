// =============================================================================
//  tests/test_t05_timer5.cpp  --  AI(W906-T05) 20261008 (Ifor01)
//
//  T-05（docs/TIMER_TABLE_PLAN.md §5.2；census129_e E-TM-010）：golden TfMain::Timer5Timer（main.cpp:31210-31297，ATC 連線看門狗）
//  要讀真的 ATC 連線狀態，所以先把 ATC 介面表單的三支計時器接上排程表（本體 ATC/ATCInterface.cpp 早已翻好、沒人驅動）：
//    [R1] ATCInterfaceForm.ATCWatchTimer／TimerChillerStop／TimerATC 在排程表上，週期照 golden dfm（200／5000／5000 ms）、
//         設計期都關著、OnTimer 有接、Status full
//    [R2] 開機 W906_BootInitialATC：沒裝 ATC ⇒ 三支都不開（HT9050 的情形）；HonPrec ⇒ InitialATC 開 ATCWatchTimer＋TimerChillerStop，
//         另照 golden FormShow main.cpp:10701-10704 開 TimerATC
//    [R3] W906_TfMain_TimersBoot 把 fMain->Timer5 的 OnTimer 接到 W906_TfMain_Timer5Timer（Status partial）
//    [R4] Timer5 本體：InitialOK 守衛；HonPrec＋運轉中＋主動冷卻：燈熄＋tester 線上 ⇒ iATCOnLine=false、WAR15309 一次；
//         tester 離線 ⇒ 不報；燈亮 ⇒ iATCOnLine=true；停機 ⇒ 不動；沒裝 ATC ⇒ 不動；ATC 7.0（G2 閘住）⇒ 不動；
//         D1：畫面上 WAR15309 還開著 ⇒ 這一拍直接返回
//    [R5] 燈號的來源是 ATC socket：RefreshChannelState 依 HT_ATC::IsConnect 改燈（測試沒有連線 ⇒ 熄）
//  不跑 ATCWatchTimerTimer 本體（它會 Connect／OnLine 連網路）；只驗它接上了、什麼時候會被打開。
// =============================================================================
#include <cstdio>
#include <string>
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "LastSet.h"
#include "canary_support.h"
#include "TimerTable.h"
#include "forms/fMain.h"
#include "forms/fMain_Timers.h"
#include "forms/fNote.h"
#include "ATC/ATCInterface.h"
#include "w906_ctest_guard.h"

void W906_BootInitialATC();      // forms/fMain_ATCSiteUse.cpp
void W906_TfMain_TimersBoot();   // forms/fMain_Timers.cpp
void W906_TfMain_Timer5Timer();  // MainTimer5.cpp

static int g_fail = 0;
static void check(bool ok, const char* label)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok) ++g_fail;
}

static void entry(const char* name, int interval)
{
    ht9045::TTimerEntry* e = ht9045::TimerTableFind(name);
    char m[220];
    std::snprintf(m, sizeof(m), "R1 %s on the table, %d ms, designed off, OnTimer bound, Status full", name, interval);
    check(e != 0 && (int)e->Interval == interval && !(bool)e->Enabled && e->OnTimer != 0 &&
          e->Status != 0 && std::string(e->Status) == "full", m);
}

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("T05_Timer5"))
        return 2;
    if (ATCInterfaceForm == 0 || fMain == 0 || fNote == 0) { std::printf("FAIL: no ATCInterfaceForm / fMain / fNote\n"); return 1; }
    char m[220];

    std::puts("-- [R1] the three TATCInterfaceForm timers on the timer table (golden ATCInterface.dfm :1064 / :1070 / :1085)");
    entry("ATCInterfaceForm.ATCWatchTimer", 200);
    entry("ATCInterfaceForm.TimerChillerStop", 5000);
    entry("ATCInterfaceForm.TimerATC", 5000);

    std::puts("-- [R2] boot: only a HonPrec ATC machine switches them on");
    const int saveSys = ATC_SYSTEM;
    ATC_SYSTEM = eATCUninstall;                       // HT9050: USE_ATC_MODE=0
    W906_BootInitialATC();
    check(!(bool)ATCInterfaceForm->ATCWatchTimer->Enabled && !(bool)ATCInterfaceForm->TimerChillerStop->Enabled &&
          !(bool)ATCInterfaceForm->TimerATC->Enabled, "R2 no ATC installed: all three stay off (golden FormShow :9357 / :10701 only for HonPrec)");
    ATC_SYSTEM = eATCHonPrecType;
    W906_BootInitialATC();
    check(ATCInterfaceForm->ATC_SYS_PAL.size() == 4, "R2 HonPrec: InitialATC built 4 channels (golden :9359)");
    check((bool)ATCInterfaceForm->ATCWatchTimer->Enabled && (bool)ATCInterfaceForm->TimerChillerStop->Enabled,
          "R2 HonPrec: InitialATC switched ATCWatchTimer and TimerChillerStop on (golden ATCInterface.cpp InitialATC tail)");
    check((bool)ATCInterfaceForm->TimerATC->Enabled, "R2 HonPrec: TimerATC on (golden TfMain::FormShow main.cpp:10701-10704)");

    std::puts("-- [R3] fMain->Timer5 bound to the Timer5 body");
    W906_TfMain_TimersBoot();
    check(fMain->Timer5->OnTimer == &W906_TfMain_Timer5Timer && fMain->Timer5->Status != 0 &&
          std::string(fMain->Timer5->Status) == "partial", "R3 fMain->Timer5->OnTimer == W906_TfMain_Timer5Timer, Status partial");

    std::puts("-- [R4] Timer5 body (golden main.cpp:31210-31297)");
    TMyLed* led = ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect;
    Temperature.bATCActiveCooling = true;
    Temperature.bATC70Active = false;
    LastSet.iTester = ON_LINE;
    led->Value = false;
    SystemStart = true;
    fNote->fShow = false;
    InitialOK = false;
    iATCOnLine = true;
    W906_ShowErrorMessage_Reset();
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true && W906_ShowErrorMessage_Count == 0, "R4 InitialOK=false: golden returns first (nothing changes)");
    InitialOK = true;
    W906_TfMain_Timer5Timer();
    std::snprintf(m, sizeof(m), "R4 HonPrec, running, active cooling, LED off, tester on line: iATCOnLine=false and one WAR15309 (count %d, code %s)",
                  W906_ShowErrorMessage_Count, W906_ShowErrorMessage_LastCode.c_str());
    check(iATCOnLine == false && W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == AnsiString("WAR15309"), m);
    LastSet.iTester = OFF_LINE;
    iATCOnLine = true;
    W906_ShowErrorMessage_Reset();
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == false && W906_ShowErrorMessage_Count == 0, "R4 tester off line: iATCOnLine=false but no alarm (golden :31240)");
    led->Value = true;
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true && W906_ShowErrorMessage_Count == 0, "R4 LED on: iATCOnLine=true");
    SystemStart = false;
    led->Value = false;
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true, "R4 not running: golden updates nothing (:31233)");
    SystemStart = true;
    Temperature.bATCActiveCooling = false;
    Temperature.bATC70Active = true;
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true && W906_ShowErrorMessage_Count == 0, "R4 ATC 7.0 branch is gated (G2: no live aldATCPower) -- nothing changes");
    Temperature.bATC70Active = false;
    Temperature.bATCActiveCooling = true;
    ATC_SYSTEM = eATCUninstall;
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true && W906_ShowErrorMessage_Count == 0, "R4 no ATC installed: nothing changes");
    ATC_SYSTEM = eATCHonPrecType;
    LastSet.iTester = ON_LINE;
    fNote->fShow = true;                              // D1: a WAR15309 notice is still up
    fNote->edErrorCode->Text = "WAR15309";
    W906_ShowErrorMessage_Reset();
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == true && W906_ShowErrorMessage_Count == 0, "R4 D1: WAR15309 still showing -> this pass returns (golden's modal holds the timer)");
    fNote->fShow = false;
    fNote->edErrorCode->Text = "";
    W906_TfMain_Timer5Timer();
    check(iATCOnLine == false && W906_ShowErrorMessage_Count == 1, "R4 D1: notice closed -> the next pass alarms again");

    std::puts("-- [R5] the LED follows the ATC socket (RefreshChannelState <- HT_ATC::IsConnect)");
    led->Value = true;
    ATCInterfaceForm->RefreshChannelState();
    check(led->Value == false, "R5 no ATC connection in the test -> RefreshChannelState turns LedATCConnect off");

    SystemStart = false;
    InitialOK = false;
    ATC_SYSTEM = saveSys;
    std::printf("test_t05_timer5: %d failure(s)\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
