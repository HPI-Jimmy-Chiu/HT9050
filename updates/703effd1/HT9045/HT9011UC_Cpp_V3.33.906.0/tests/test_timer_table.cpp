// =============================================================================
//  tests/test_timer_table.cpp  --  ctest TimerTable：golden TTimer 排程表（TimerTable.h）的行為，對照 VCL TTimer。
//
//  AI(W906-TIMER-TABLE) 20261001。計畫書 docs/TIMER_TABLE_PLAN.md §6。
//  時間全用假時鐘（TimerTableTick 的 nowMs 由測試給、耗時時鐘換成 g_fakeUs），所以結果不受機器快慢影響，秒級跑完。
//  不 include cmydef、不 link god-stack、不讀寫任何檔。
//
//  argv[1]（可省）＝ docs/TIMER_CENSUS.tsv：拿來對 forms/fMain_Timers.h 那 14 支的 Enabled／Interval，
//  有人改了巨集裡的值而 golden 沒變，這裡就紅。
// =============================================================================
#include "TimerTable.h"
#include "forms/fMain_Timers.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                          \
    do {                                                                          \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                 \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static unsigned long long g_fakeUs = 0;
static unsigned long long FakeUs() { return g_fakeUs; }

static int g_countA = 0;
static int g_countB = 0;
static void OnA() { ++g_countA; }
static void OnB() { ++g_countB; }

// 從 t0 跑到 t1（含），每 step ms 呼叫一次 Tick
static void Run(unsigned long long t0, unsigned long long t1, unsigned long long step)
{
    for (unsigned long long t = t0; t <= t1; t += step) TimerTableTick(t);
}

static void Reset()
{
    g_countA = g_countB = 0;
    g_fakeUs = 0;
    TimerTableResetThreadForTest();
}

// ---------------------------------------------------------------------------
static void T1_Period()
{
    std::printf("T1 一般週期：1000 ms、每 10 ms 一圈，60 秒剛好 60 次\n");
    Reset();
    TTimerEntry t("test.T1", true, 1000, "t1", &OnA);
    Run(0, 60000, 10);
    CHECK(g_countA == 60, "60 秒 60 次（第一次在 Tick 第一次看到它之後 1000 ms，同 VCL 從 SetTimer 起算）");
    CHECK(t.Stats.fires == 60, "Stats.fires == 60");
}

static void T2_Inactive()
{
    std::printf("T2 不會跑的三種：Enabled=false、Interval=0、OnTimer=0（VCL UpdateTimer 同）\n");
    Reset();
    TTimerEntry off("test.off", false, 1000, "t2", &OnA);
    TTimerEntry zero("test.zero", true, 0, "t2", &OnA);
    TTimerEntry nobody("test.nobody", true, 1000, "t2", 0);
    Run(0, 10000, 10);
    CHECK(g_countA == 0, "三支都沒跑");
}

static void T3_EnableEdgeRestarts()
{
    std::printf("T3 Enabled 由 false 變 true：從那一刻起算；設成同一個值不重算；Interval 改了重算\n");
    Reset();
    TTimerEntry t("test.T3", false, 1000, "t3", &OnA);
    Run(0, 500, 10);
    t.Enabled = true;                                                           // golden FormShow `Timer3->Enabled=true;`
    Run(510, 1500, 10);
    CHECK(g_countA == 0, "打開後 1000 ms 之內不跑");
    Run(1510, 1510, 10);
    CHECK(g_countA == 1, "打開（t=510 那一圈看到）後 1000 ms（t=1510）跑第一次");
    t.Enabled = true;                                                           // 同一個值：VCL SetEnabled 什麼都不做
    Run(1510, 2510, 10);
    CHECK(g_countA == 2, "設成同一個值不重算：t=2510 照原本的格子跑第二次");
    t.Interval = 300;                                                           // 改 Interval：重算
    Run(2520, 2810, 10);
    CHECK(g_countA == 2, "Interval 改成 300 之後 300 ms 之內不跑");
    Run(2820, 2820, 10);
    CHECK(g_countA == 3, "改 Interval（t=2520 那一圈看到）後 300 ms 跑");
}

static void T4_CoalesceNoCatchUp()
{
    std::printf("T4 主迴圈被拖住 5.5 秒：只跑一次，不補跑（WM_TIMER 會合併）\n");
    Reset();
    TTimerEntry t("test.T4", true, 1000, "t4", &OnA);
    TimerTableTick(0);
    TimerTableTick(5500);
    CHECK(g_countA == 1, "5.5 秒後那一圈只跑一次");
    CHECK(t.Stats.maxLateMs == 4500, "記到最晚晚了 4500 ms（原本 t=1000 到期）");
    Run(5510, 6490, 10);
    CHECK(g_countA == 1, "下一次不是 t=2000（已錯過），而是從 t=5500 起算");
    Run(6500, 6500, 10);
    CHECK(g_countA == 2, "t=6500 跑第二次");
}

static void T5_PhaseKept()
{
    std::printf("T5 某一次晚了 40 ms：下一次仍在原本的格子上（同 wb_serve PumpTick 的 B13 寫法）\n");
    Reset();
    TTimerEntry t("test.T5", true, 1000, "t5", &OnA);
    TimerTableTick(0);
    TimerTableTick(1040);
    CHECK(g_countA == 1, "t=1040 跑第一次（晚 40 ms）");
    TimerTableTick(1990);
    CHECK(g_countA == 1, "t=1990 還不到");
    TimerTableTick(2000);
    CHECK(g_countA == 2, "t=2000 跑第二次（不是 2040）");
}

static TTimerEntry* g_self = 0;
static void OnSelfDisable() { ++g_countA; g_self->Enabled = false; }           // golden main.cpp:25597 `Timer3->Enabled=false;`

static void T6_SelfDisable()
{
    std::printf("T6 OnTimer 裡把自己關掉：之後不跑；再打開從那一刻起算\n");
    Reset();
    TTimerEntry t("test.T6", true, 1000, "t6", &OnSelfDisable);
    g_self = &t;
    Run(0, 5000, 10);
    CHECK(g_countA == 1, "只跑了一次");
    CHECK(!(bool)t.Enabled, "Enabled 是 false");
    t.Enabled = true;
    Run(5010, 6000, 10);
    CHECK(g_countA == 1, "重新打開後 1000 ms 之內不跑");
    Run(6010, 6010, 10);
    CHECK(g_countA == 2, "t=6010（5010＋1000）跑");
    g_self = 0;
}

static int g_throwCount = 0;
static void OnThrow() { ++g_throwCount; if (g_throwCount == 1) throw std::runtime_error("boom"); }

static void T7_ExceptionDoesNotKillTimer()
{
    std::printf("T7 OnTimer 丟例外：接住、記數，下一次照跑（VCL Application->HandleException 同）\n");
    Reset();
    g_throwCount = 0;
    TTimerEntry t("test.T7", true, 1000, "t7", &OnThrow);
    Run(0, 3000, 10);
    CHECK(g_throwCount == 3, "三次都有進 OnTimer（第一次丟例外之後照跑）");
    CHECK(t.Stats.exceptions == 1, "Stats.exceptions == 1");
    CHECK(!t.running_, "外層「正在跑」旗標放開了");
}

static TTimerEntry* g_nestOuter = 0;
static int g_nestCalls = 0;
static void OnNest()
{
    ++g_nestCalls;
    // 模擬 golden 本體裡跳 ShowModal、巢狀訊息迴圈又送 WM_TIMER：在 OnTimer 裡再呼叫一次 Tick（2 秒之後）
    if (g_nestCalls == 1) TimerTableTick(3000);
}

static void T8_Reentry()
{
    std::printf("T8 巢狀 Tick：自己還在跑就跳過（reentrySkips），別支照跑\n");
    Reset();
    g_nestCalls = 0;
    TTimerEntry outer("test.T8outer", true, 1000, "t8", &OnNest);
    TTimerEntry other("test.T8other", true, 1000, "t8", &OnB);
    g_nestOuter = &outer;
    TimerTableTick(0);
    TimerTableTick(1000);                                                       // outer 到期 → 在裡面 Tick(3000)
    CHECK(g_nestCalls == 1, "outer 沒有被自己的巢狀 Tick 再叫一次");
    CHECK(outer.Stats.reentrySkips == 1, "outer.reentrySkips == 1");
    CHECK(g_countB >= 1, "巢狀 Tick 裡 other 照跑");
    g_nestOuter = 0;
}

static void T9_PropertyCopy()
{
    std::printf("T9 屬性對拷與 `X->Enabled=!X->Enabled`（vclcompat 拷貝賦值陷阱那一族）\n");
    Reset();
    TTimerEntry a("test.T9a", false, 1000, "t9", &OnA);
    TTimerEntry b("test.T9b", true, 250, "t9", &OnB);
    TTimerEntry* pa = &a;
    TTimerEntry* pb = &b;
    const unsigned before = pa->Enabled.Changes();
    pa->Enabled = pb->Enabled;                                                  // golden `A->Enabled=B->Enabled;`
    CHECK((bool)pa->Enabled == true, "A->Enabled=B->Enabled 拷到值");
    CHECK(pa->Enabled.Changes() == before + 1, "A 的變更計數加一（不是把 B 的計數拷過來）");
    pa->Interval = pb->Interval;
    CHECK((int)pa->Interval == 250, "A->Interval=B->Interval 拷到值");
    pb->Enabled = !pb->Enabled;                                                 // golden uMotorTest.cpp:2099
    CHECK((bool)pb->Enabled == false, "X->Enabled=!X->Enabled 會翻");
    CHECK((double)(pa->Interval) == 250.0, "(double)(X->Interval) 轉得過去（golden main.cpp:25141 的寫法）");
}

static void OnSlow() { ++g_countA; g_fakeUs += 60000; }                       // 一次 60 ms

static void T10_Overrun()
{
    std::printf("T10 跑超過 50 ms：記 overrun、印一行警告\n");
    Reset();
    TimerTableSetUsClockForTest(&FakeUs);
    TTimerEntry t("test.T10", true, 1000, "t10", &OnSlow);
    Run(0, 2000, 10);
    TimerTableSetUsClockForTest(0);
    CHECK(g_countA == 2, "跑了兩次");
    CHECK(t.Stats.overruns == 2, "兩次都算 overrun");
    CHECK(t.Stats.maxUs == 60000, "maxUs == 60000");
}

static DWORD WINAPI OtherThread(LPVOID) { TimerTableTick(100000); return 0; }

static void T11_ThreadGuard()
{
    std::printf("T11 別的執行緒呼叫 Tick：不跑（印一次錯誤）\n");
    Reset();
    TTimerEntry t("test.T11", true, 1000, "t11", &OnA);
    TimerTableTick(0);                                                          // 主執行緒先登記
    HANDLE h = ::CreateThread(0, 0, &OtherThread, 0, 0, 0);
    ::WaitForSingleObject(h, 5000);
    ::CloseHandle(h);
    CHECK(g_countA == 0, "別的執行緒的 Tick(100000) 沒有讓它跑");
    TimerTableTick(1000);
    CHECK(g_countA == 1, "主執行緒照常");
}

static TTimerEntry* g_victim = 0;
static void OnKill() { ++g_countA; delete g_victim; g_victim = 0; }

static void T12_UnregisterDuringTick()
{
    std::printf("T12 OnTimer 裡拆掉另一支（表單關掉）：不當機、被拆的那支不跑\n");
    Reset();
    TTimerEntry killer("test.T12killer", true, 1000, "t12", &OnKill);
    g_victim = new TTimerEntry("test.T12victim", true, 1000, "t12", &OnB);
    Run(0, 3000, 10);
    CHECK(g_countA == 3, "killer 跑了三次");
    CHECK(g_countB == 0, "victim 在同一圈被拆，沒跑到");
}

static void T13_FindAndJson()
{
    std::printf("T13 Find／Report／JSON\n");
    Reset();
    TTimerEntry a("test.T13", true, 1000, "main.cpp:1-2", &OnA);
    TTimerEntry b("test.T13", false, 500, "main.cpp:3-4", 0);
    CHECK(TimerTableFind("test.T13") == &b, "同名回最後登記的");
    CHECK(TimerTableFind("nope") == 0, "找不到回 0");
    const std::string j = TimerTableJson();
    CHECK(j.compare(0, 14, "{\"overrunMs\":5") == 0, "JSON 開頭");
    CHECK(j.find("\"name\":\"test.T13\"") != std::string::npos, "JSON 有名字");
    CHECK(j.find("\"status\":\"no-body\"") != std::string::npos, "沒接本體的標 no-body");
    CHECK(j[j.size() - 1] == '}', "JSON 結尾");
    const std::string r = TimerTableReport();
    CHECK(r.find("test.T13") != std::string::npos, "Report 有名字");
}

static void T15_Unregister()
{
    std::printf("T15 拿下表（門面建了兩份時留下真正在用的那份）：拿下的不再跑、表上看不到、解構也安全\n");
    Reset();
    const int before = TimerTableCount();
    TTimerEntry* orphan = new TTimerEntry("test.T15", true, 1000, "t15", &OnA);
    TTimerEntry  live("test.T15", true, 1000, "t15", &OnB);
    CHECK(TimerTableCount() == before + 2, "兩支都在表上");
    const std::vector<TTimerEntry*> all = TimerTableAll();
    CHECK((int)all.size() == before + 2, "TimerTableAll 是全部的快照");
    TimerTableUnregister(orphan);
    CHECK(TimerTableCount() == before + 1, "拿下一支");
    TimerTableUnregister(orphan);
    CHECK(TimerTableCount() == before + 1, "再拿一次不會多拿");
    Run(0, 3000, 10);
    CHECK(g_countA == 0, "拿下表的那支不跑");
    CHECK(g_countB == 3, "留下的那支照跑");
    CHECK(TimerTableFind("test.T15") == &live, "Find 找到留下的那支");
    delete orphan;                                                              // 解構時它已不在表上：不可以誤刪別支
    CHECK(TimerTableCount() == before + 1, "解構拿下表的那支，不影響表上其他支");
}

// 門面的 14 支：巨集展開在一個假的 struct 裡，對 golden main.dfm（經 docs/TIMER_CENSUS.tsv）
struct FakeMain { W906_TFMAIN_TIMER_MEMBERS };

static void T14_MainMembersMatchGolden(const char* tsvPath)
{
    std::printf("T14 forms/fMain_Timers.h 的 14 支 = golden main.dfm（docs/TIMER_CENSUS.tsv）\n");
    Reset();
    const int before = TimerTableCount();
    FakeMain* m = new FakeMain();
    CHECK(TimerTableCount() == before + 14, "登記了 14 支");
    CHECK(!(bool)m->Timer3->Enabled && (int)m->Timer3->Interval == 1000, "Timer3：關、1000（FormShow :10180 才打開）");
    CHECK((bool)m->Timer8->Enabled && (int)m->Timer8->Interval == 1000, "Timer8：開、1000");
    CHECK((bool)m->TimerESD->Enabled && (int)m->TimerESD->Interval == 1000, "TimerESD：開、1000");
    CHECK(m->Timer3->OnTimer == 0, "本體還沒接（W906_TfMain_TimersBoot 才接）");
    if (!tsvPath) {
        std::printf("  (沒給 TIMER_CENSUS.tsv，略過逐支對 golden)\n");
        return;
    }
    std::ifstream f(tsvPath);
    CHECK(f.good(), "讀得到 TIMER_CENSUS.tsv");
    std::string line;
    int matched = 0;
    std::getline(f, line);                                                      // 表頭
    while (std::getline(f, line)) {
        std::vector<std::string> c;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, '\t')) c.push_back(cell);
        if (c.size() < 7 || c[0] != "TfMain") continue;                         // form_cls form_var name class Enabled Interval ...
        const std::string name = "fMain." + c[2];
        TTimerEntry* t = TimerTableFind(name.c_str());
        char msg[160];
        std::snprintf(msg, sizeof msg, "%s 在門面上", name.c_str());
        CHECK(t != 0, msg);
        if (!t) continue;
        const bool en = (c[4] == "True");
        const int iv = std::atoi(c[5].c_str());
        std::snprintf(msg, sizeof msg, "%s Enabled=%s Interval=%d 跟 golden 一樣", name.c_str(), en ? "True" : "False", iv);
        CHECK((bool)t->Enabled == en && (int)t->Interval == iv, msg);
        ++matched;
    }
    CHECK(matched == 14, "golden TfMain 正好 14 支、每支都對到");
}

int main(int argc, char** argv)
{
    TimerTableSetReportEveryMs(0);
    T1_Period();
    T2_Inactive();
    T3_EnableEdgeRestarts();
    T4_CoalesceNoCatchUp();
    T5_PhaseKept();
    T6_SelfDisable();
    T7_ExceptionDoesNotKillTimer();
    T8_Reentry();
    T9_PropertyCopy();
    T10_Overrun();
    T11_ThreadGuard();
    T12_UnregisterDuringTick();
    T13_FindAndJson();
    T15_Unregister();
    T14_MainMembersMatchGolden(argc > 1 ? argv[1] : 0);
    std::printf("\nTimerTable: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
