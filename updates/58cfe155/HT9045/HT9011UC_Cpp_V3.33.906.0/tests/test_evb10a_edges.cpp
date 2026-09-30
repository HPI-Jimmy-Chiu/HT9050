// =============================================================================
//  test_evb10a_edges.cpp -- 事件批次 B10 part a：設定視窗「真的開／關」那一下的分派表（FileRW/WindowEdgeTails.h）
//
//  //AI(W906-EVB10A) 20260929 [W906] St01。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10
//    （YM-3、SU-9、OS-6、CT-3b、CC-E10；Steven 20260928「任何畫面的事件, 都是我們做」、20260929「照 BCB 的邏輯」）。
//  受測：FileRW/WindowEdgeTails.h 的表 kRows、Find、Decide、OpenLatch（純邏輯），以及它跟頁面表（WebPageTable.cpp）的接法：
//    [1] 表上每一個 golden 表單名都是頁面表的網頁列（opener web／both、有 webId），而且 D:\HT9045\web\background.html 的 WINDOWS
//        有同一個 form:（改名會靜默對不上 ⇒ 紅）；表的列數＝FileRW/MainClick.cpp W906_EvB10A_WindowEdge 的函式表（static_assert 在那一邊）
//    [2] Decide：開的那一下全部 kNotListed（這一批沒有開窗動作）；關：不在表上 ⇒ kNotListed；運轉中 ⇒ skipWhileRunning 的三個
//        （Yield／Setup／Configuration，golden 模態）kSkipRunning、Offset（非模態）與 Contact（FormClose 自己就是停機）照樣 kRun
//    [3] OpenLatch：Shown → TakeForClose 一次 true、再問 false；Closed 之後 false；沒開過 false
//    [4] 跟頁面表一起跑（真的 WebWindowRegistry 餵 ui.windows.put 訊框，PageTableSetEdgeHook 收邊緣，照 wb_serve.cpp:4389 的 lambda
//        先 Decide）：Setup 開→關 ⇒ 一個 fSetup 關的邊緣、Decide＝kRun；運轉中關 ⇒ kSkipRunning；Contact 運轉中關 ⇒ kRun；
//        Speed（不在表上）開→關 ⇒ 有邊緣但 kNotListed；F5 的「全部關」bye 訊框 ⇒ 開著的那幾個各一個關的邊緣
//        （//AI(W906-EVB10C) 20260929 [W906]：Speed 已上表 ⇒ 這一段改用 Motor Test）
//    [5] //AI(W906-EVB10C) 20260929 [W906] B10c 的 14 列：列序、每列都叫 golden FormClose、開的那一下不動、停機時關照跑、運轉中
//        照 golden 開法（ShowModal 不跑、Show 照跑；逐列手抄 golden 出處）；跟頁面表一起跑（Temp_Set／BinSel／CounterSel 運轉中關、F5 全部關）。
//        「這一次開窗 golden FormClose 已經跑過就不跑第二次」的記號在 FileRW/_EditPage.cpp（ctest OpenEnterLog [10]）。
//    [6] //AI(W906-EVB10C) 20260929 [W906] B2 守衛：evb10a::RanDuringOpen 的記／清／擋（Start Condition 開著時跑過生產）與兩句訊息。
//    [7] //AI(W906-EVB10C-BC) 20260930 [W906] B10c 後續：BarCode 的 Exit 鈕＝golden TfBarCode::sbtExitClick（V912 BarCode\BarCode.cpp:2408-2417）
//        「接上了沒」的原始碼棘輪（argv[2]＝移植樹根目錄）：產生檔處理器照 golden 順序、事件表一列、替身與 DFM 父層、兩個 golden 全域、頁面送出點。
//        ack.closed 與關窗邊緣「不跑第二次」的交接在 ctest FormEvent_Position [18]。
//  NOT COVERED：各表單的 golden 本體（FileRW/<結構>.cpp 檔尾，要 god-stack＋配方檔，只編進 wb_serve）—— 語法檢查＋wb_serve 連結為準，
//    上機驗列在交件。
//  不 include cmydef、不 link god-stack、不寫任何檔（background.html 唯讀），秒級。
// =============================================================================
#include "FileRW/WindowEdgeTails.h"
#include "WebPageTable.h"
#include "WebTeachLeave.h"
#include "WebWindowRegistry.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
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

// ---- 頁面表的 host（同 tests/test_pagetable.cpp 的做法）----------------------------------------------------------
static int          g_ws = 1;
static std::int64_t g_now = 1000000;
static int  FakeWs() { return g_ws; }
static std::int64_t FakeNow() { return g_now; }
static void FakeNop(const char*) {}

// wb_serve.cpp:4389 的 lambda：邊緣先交給 W906_EvB10A_WindowEdge（這裡只記 Decide 的結果）
static bool g_running = false;
struct Seen { std::string form; bool open; evb10a::Verdict v; };
static std::vector<Seen> g_seen;
static void Edge(const char* form, bool open)
{
    Seen s;
    s.form = form ? form : "";
    s.open = open;
    s.v = evb10a::Decide(form, open, g_running);
    g_seen.push_back(s);
}

static void Fresh()
{
    WebWindowRegistryResetForTest();
    WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    PageTableResetForTest();
    PageTableSetClockForTest(&FakeNow);
    WindowEdgeResetForTest();
    g_ws = 1; g_now = 1000000; g_running = false;
    g_seen.clear();
    PageTableHost h;
    h.liveWs = &FakeWs; h.pause = &FakeNop; h.homeClose = &FakeNop; h.motorStop = &FakeNop; h.alarm = &FakeNop;
    PageTableArm(h);
    PageTableSetEdgeHook(&Edge);
}

// background.html buildRegistry 的形狀（同 tests/test_pagetable.cpp Frame）：47 個有 golden 名的網頁列（預設 never、fMain open），overrides 蓋掉
static std::string Frame(const std::string& overrides, bool bye = false)
{
    std::size_t n = 0;
    const PageRowDef* rows = PageTableRows(n);
    std::string s = "{\"type\":\"ui.windows\",\"seq\":1,\"at\":\"2026-09-29T12:00:00+08:00\",\"topmost\":null,\"modalStack\":[],";
    if (bye) s += "\"bye\":true,";
    s += "\"windows\":{";
    bool first = true;
    for (std::size_t i = 0; i < PageTableWebRowCount(); ++i) {
        const std::string form = rows[i].form;
        if (form.empty()) continue;
        std::string st = (form == "fMain") ? "open" : "never";
        if (bye) st = "closed";
        const std::string key = "," + form + "=";
        const std::string ov = "," + overrides;
        const std::size_t p = ov.find(key);
        if (p != std::string::npos) {
            const std::size_t b = p + key.size();
            const std::size_t e = ov.find(',', b);
            st = ov.substr(b, e == std::string::npos ? std::string::npos : e - b);
        }
        if (!first) s += ",";
        first = false;
        s += "\""; s += rows[i].webId; s += "\":{\"form\":\""; s += form; s += "\",\"state\":\""; s += st;
        s += "\",\"fullscreen\":false}";
    }
    s += "}}";
    return s;
}
static bool Put(const std::string& frame)
{
    std::string why;
    const bool ok = WebWindowRegistryPut(1, frame, why);
    if (!ok) std::printf("  (Put rejected: %s)\n", why.c_str());
    return ok;
}
static void Tick()
{
    PageTickFacts f;
    f.systemStart = g_running; f.homingAll = false; f.webMotorJob = false;
    PageTableTick(f);
    g_now += 500;
}
static int Count(const char* form, bool open, evb10a::Verdict v)
{
    int n = 0;
    for (std::size_t i = 0; i < g_seen.size(); ++i)
        if (g_seen[i].form == form && g_seen[i].open == open && g_seen[i].v == v) ++n;
    return n;
}

int main(int argc, char** argv)
{
    std::printf("[1] every kRows form is a page-table web row and a background.html WINDOWS form\n");
    {
        CHECK(evb10a::kRowCount == 19, "19 rows (B10a: YM-3, SU-9, OS-6, CT-3b, CC-E10; B10c: 14 more, see [5])");   //AI(W906-EVB10C) 20260929 [W906]: 5 → 19（B10c 加 14 列）
        std::string bg;
        if (argc > 1) {
            std::ifstream f(argv[1], std::ios::binary);
            std::stringstream ss;
            ss << f.rdbuf();
            bg = ss.str();
        }
        CHECK(argc < 2 || !bg.empty(), "background.html readable (when given)");
        std::size_t n = 0;
        const PageRowDef* rows = PageTableRows(n);
        for (std::size_t i = 0; i < evb10a::kRowCount; ++i) {
            const char* form = evb10a::kRows[i].form;
            const int r = PageTableFind(form);
            const bool web = r >= 0 && (rows[r].opener == kPgWeb || rows[r].opener == kPgBoth) && rows[r].webId && *rows[r].webId &&
                             std::strncmp(rows[r].webId, "cpp:", 4) != 0 && std::strncmp(rows[r].webId, "none:", 5) != 0;
            std::string m = std::string(form) + " is a web row of the page table";
            CHECK(web, m.c_str());
            if (!bg.empty()) {
                const bool inBg = bg.find(std::string("form:'") + form + "'") != std::string::npos ||
                                  bg.find(std::string("form:\"") + form + "\"") != std::string::npos;
                m = std::string(form) + " is a form: of background.html WINDOWS";
                CHECK(inBg, m.c_str());
            }
            CHECK(evb10a::Find(form) == (int)i, "Find returns the row index");
        }
        CHECK(evb10a::Find("fMotorTest") == -1 && evb10a::Find("") == -1 && evb10a::Find(0) == -1, "unlisted / empty / null -> -1");   //AI(W906-EVB10C) 20260929 [W906]: fSpeed 已上表（B10c）→ 改用 fMotorTest（不是設定頁）
        CHECK(evb10a::Find("fSetu") == -1 && evb10a::Find("fSetupX") == -1, "prefix / longer names do not match");
    }

    std::printf("[2] Decide\n");
    {
        for (std::size_t i = 0; i < evb10a::kRowCount; ++i) {
            const char* form = evb10a::kRows[i].form;
            CHECK(evb10a::Decide(form, true, false) == evb10a::kNotListed && evb10a::Decide(form, true, true) == evb10a::kNotListed,
                  "open edges: no action in this batch");
            CHECK(evb10a::Decide(form, false, false) == evb10a::kRun, "close edge, stopped -> run");
        }
        CHECK(evb10a::Decide("fYieldMonitoring", false, true) == evb10a::kSkipRunning, "Yield close while running -> skip (modal)");
        CHECK(evb10a::Decide("fSetup", false, true) == evb10a::kSkipRunning, "Setup close while running -> skip (modal)");
        CHECK(evb10a::Decide("fConfiguration", false, true) == evb10a::kSkipRunning, "Configuration close while running -> skip (modal)");
        CHECK(evb10a::Decide("fOffSet", false, true) == evb10a::kRun, "Offset close while running -> run (non-modal, golden has no guard)");
        CHECK(evb10a::Decide("fContact", false, true) == evb10a::kRun, "Contact close while running -> run (golden FormClose stops the machine)");
        CHECK(evb10a::Decide("fMotorTest", false, false) == evb10a::kNotListed && evb10a::Decide("fTeach", false, false) == evb10a::kNotListed,
              "not listed (Motor Test; Teach = Jimmy's close-edge cancel, B10c not now) -> nothing");   //AI(W906-EVB10C) 20260929 [W906]: fSpeed／fTemp_Set 已上表（B10c）→ 改用 fMotorTest／fTeach
    }

    std::printf("[3] OpenLatch\n");
    {
        evb10a::OpenLatch l;
        CHECK(!l.TakeForClose(), "never shown -> no FormClose");
        l.Shown();
        CHECK(l.TakeForClose(), "shown -> FormClose once");
        CHECK(!l.TakeForClose(), "second close edge without a new open -> no FormClose");
        l.Shown();
        l.Closed();
        CHECK(!l.TakeForClose(), "golden Close() already ran -> no second FormClose");
        l.Shown();
        l.Shown();
        CHECK(l.TakeForClose() && !l.TakeForClose(), "reload inside one open (Shown twice) -> still one FormClose");
    }

    std::printf("[4] with the page table: edges reach the hook, Decide gets the right answer\n");
    {
        Fresh();
        Put(Frame(""));
        Tick();
        g_seen.clear();
        Put(Frame("fSetup=open"));
        Tick();
        CHECK(Count("fSetup", true, evb10a::kNotListed) == 1, "Setup opened -> one open edge, no action");
        Put(Frame("fSetup=closed"));
        Tick();
        CHECK(Count("fSetup", false, evb10a::kRun) == 1 && g_seen.size() == 2, "Setup closed -> one close edge, run");

        g_seen.clear();
        Put(Frame("fSetup=open"));
        Tick();
        g_running = true;
        Put(Frame("fSetup=closed"));
        Tick();
        CHECK(Count("fSetup", false, evb10a::kSkipRunning) == 1, "Setup closed while running -> skip");

        g_seen.clear();
        g_running = false;
        Put(Frame("fContact=open"));
        Tick();
        g_running = true;
        Put(Frame("fContact=closed"));
        Tick();
        CHECK(Count("fContact", false, evb10a::kRun) == 1, "Contact closed while running -> run (FormClose stops)");

        g_seen.clear();   //AI(W906-EVB10C) 20260929 [W906]: 這一段原本用 fSpeed 當「不在表上」的視窗；fSpeed 已上表（B10c）→ 改用 fMotorTest
        g_running = false;
        Put(Frame("fMotorTest=open"));
        Tick();
        CHECK(Count("fMotorTest", true, evb10a::kNotListed) == 1, "Motor Test (not listed) opened -> edge but no action");
        g_seen.clear();
        Put(Frame("fMotorTest=minimized"));
        Tick();
        CHECK(g_seen.empty(), "open -> minimized is not a close (minimized counts as open)");
        Put(Frame("fMotorTest=closed"));
        Tick();
        CHECK(Count("fMotorTest", false, evb10a::kNotListed) == 1, "Motor Test (not listed) closed -> edge but no action");

        g_seen.clear();
        Put(Frame("fOffSet=open,fYieldMonitoring=open,fConfiguration=open"));
        Tick();
        Put(Frame("", true));   // F5／關分頁：外框 pagehide 送「全部關」
        Tick();
        CHECK(Count("fOffSet", false, evb10a::kRun) == 1 && Count("fYieldMonitoring", false, evb10a::kRun) == 1 &&
              Count("fConfiguration", false, evb10a::kRun) == 1, "F5 bye frame -> one close edge per open window, each runs");
    }

    // AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c —— 另外 14 個 C 路設定頁的 golden FormClose（表 FileRW/WindowEdgeTails.h 第 6～19 列）。
    //   期望值逐列手抄：golden V912 的開法（ShowModal ⇒ 運轉中不跑；Show ⇒ 照跑）。golden 出處寫在每一列。
    std::printf("[5] B10c rows: golden ShowModal -> skip while running; golden Show (non-modal) -> run\n");
    {
        struct Exp { const char* form; bool skip; const char* why; };
        const Exp k[] = {
            {"fSpeed",          true,  "fSpeed->ShowModal() main.cpp:28695"},
            {"fLd_ULd",         true,  "fLd_ULd->ShowModal() main.cpp:28453"},
            {"fTrayForm",       true,  "fTrayForm->ShowModal() main.cpp:28409"},
            {"fTrayAssignment", true,  "fTrayAssignment->ShowModal() main.cpp:28433"},
            {"fTemp_Set",       false, "ShowModal main.cpp:28352 but fTemp_Set->Show() from Contact cContact.cpp:15256 / :17477"},
            {"fDIOFrom",        true,  "fDIOFrom->ShowModal() main.cpp:28669"},
            {"fQAMode",         false, "fQAMode->Show() main.cpp:29925"},
            {"fBarCode",        false, "fBarCode->Show() main.cpp:29916"},
            {"fVacuumUnit",     false, "fVacuumUnit->Show() main.cpp:35560"},
            {"fBinSel",         true,  "fBinSel->ShowModal() main.cpp:28324"},
            {"HandlerSystem",   true,  "HandlerSystem->ShowModal() cTemperFrom.cpp:1778"},
            {"fCounterSel",     true,  "fCounterSel->ShowModal() main.cpp:28565"},
            {"fStartCondition", true,  "fStartCondition->ShowModal() main.cpp:28553"},
            {"fCleaning",       true,  "fCleaning->ShowModal() main.cpp:29674"},
        };
        const std::size_t n = sizeof(k) / sizeof(k[0]);
        CHECK(n == 14 && evb10a::kRowCount == 5 + n, "B10c adds 14 rows after the 5 B10a rows");
        for (std::size_t i = 0; i < n; ++i) {
            const int r = evb10a::Find(k[i].form);
            char rowNo[16];
            std::snprintf(rowNo, sizeof(rowNo), "%d", (int)(5 + i));   // MinGW 6.3：不用 std::to_string（同 FileRW/_EditPage.cpp EvNum）
            std::string m = std::string(k[i].form) + " is row " + rowNo + " (after B10a)";
            CHECK(r == (int)(5 + i), m.c_str());
            m = std::string(k[i].form) + ": no open-edge action, close while stopped -> run";
            CHECK(evb10a::Decide(k[i].form, true, false) == evb10a::kNotListed && evb10a::Decide(k[i].form, true, true) == evb10a::kNotListed &&
                  evb10a::Decide(k[i].form, false, false) == evb10a::kRun, m.c_str());
            m = std::string(k[i].form) + (k[i].skip ? ": close while running -> skip (" : ": close while running -> run (") + k[i].why + ")";
            CHECK(evb10a::Decide(k[i].form, false, true) == (k[i].skip ? evb10a::kSkipRunning : evb10a::kRun), m.c_str());
            m = std::string(k[i].form) + ": row names a golden FormClose";
            CHECK(r >= 0 && std::strstr(evb10a::kRows[r].golden, "FormClose") != nullptr, m.c_str());
        }
        // 跟頁面表一起跑：非模態（Temp_Set）運轉中關照跑、模態（BinSel、CounterSel）運轉中關不跑；F5 全部關 ⇒ 各一個關的邊緣
        Fresh();
        Put(Frame(""));
        Tick();
        g_seen.clear();
        Put(Frame("fTemp_Set=open,fBinSel=open,fCounterSel=open"));
        Tick();
        g_running = true;
        Put(Frame("fTemp_Set=closed,fBinSel=closed,fCounterSel=closed"));
        Tick();
        CHECK(Count("fTemp_Set", false, evb10a::kRun) == 1 && Count("fBinSel", false, evb10a::kSkipRunning) == 1 &&
              Count("fCounterSel", false, evb10a::kSkipRunning) == 1, "running: Temp_Set close runs, BinSel / CounterSel close skipped");
        g_seen.clear();
        g_running = false;
        Put(Frame("fStartCondition=open,fCleaning=open,fVacuumUnit=open"));
        Tick();
        Put(Frame("", true));
        Tick();
        CHECK(Count("fStartCondition", false, evb10a::kRun) == 1 && Count("fCleaning", false, evb10a::kRun) == 1 &&
              Count("fVacuumUnit", false, evb10a::kRun) == 1, "F5 bye frame -> StartCondition / Cleaning / VacuumUnit each one close edge, run");
    }

    // AI(W906-EVB10C) 20260929 [W906]：B2 守衛（ST01-E 20260929）—— Start Condition 開著時跑過生產 ⇒ 關窗不跑 golden FormClose、存檔拒存。
    //   FileRW/WindowEdgeTails.h evb10a::RanDuringOpen／StartConditionProductionGuard（本體接在 FileRW/StartCondition.cpp 檔尾，要 god-stack，只在 wb_serve）。
    std::printf("[6] B2 guard: production during a Start Condition open -> close skips golden FormClose, saves refused, re-read clears\n");
    {
        evb10a::RanDuringOpen g;
        CHECK(!g.Ran() && evb10a::StartConditionProductionGuard(g, false) == nullptr && evb10a::StartConditionProductionGuard(g, true) == nullptr,
              "fresh: not set, close runs, save allowed");
        CHECK(!g.Sample(true, false) && !g.Ran(), "running but the page is not open -> not set");
        CHECK(!g.Sample(false, true) && !g.Ran(), "page open, machine stopped -> not set");
        CHECK(g.Sample(true, true) && g.Ran(), "page open and SystemStart||SoftStart -> set (first time returns true)");
        CHECK(!g.Sample(true, true) && g.Ran(), "set stays set; later samples do not report again");
        CHECK(!g.Sample(false, true) && g.Ran(), "machine stopped again -> still set (the grid is still the open-time grid)");
        const char* c = evb10a::StartConditionProductionGuard(g, false);
        CHECK(c && std::strcmp(c, "[EVB10C] fStartCondition closed after production ran during this open -- golden FormClose (save) not run, "
                                  "to avoid rolling back LastSet.iContactCT") == 0, "close edge: skip, with the agreed console line");
        const char* s = evb10a::StartConditionProductionGuard(g, true);
        CHECK(s && std::strstr(s, "Close and reopen this page") && std::strstr(s, "nothing was saved"), "save: refused with a reason for the page");
        CHECK(std::strcmp(evb10a::kStartConditionProductionGuardZh, "生產中開著這一頁，計數已經變了；請關掉重開這一頁再存") == 0,
              "save: agreed Chinese reason");
        g.FormShown();
        CHECK(!g.Ran() && evb10a::StartConditionProductionGuard(g, true) == nullptr, "golden FormShow (open / re-read) clears it");
        CHECK(g.Sample(true, true) && evb10a::StartConditionProductionGuard(g, false) != nullptr, "set again in the next open when production runs");
        CHECK(evb10a::Decide("fStartCondition", false, true) == evb10a::kSkipRunning,
              "closing while still running is skipped by the table anyway (modal) -- the guard covers 'ran, then stopped, then closed'");
    }

    // AI(W906-EVB10C-BC) 20260930 [W906]：B10c 後續 —— BarCode 的 Exit 鈕＝golden TfBarCode::sbtExitClick（V912 BarCode\BarCode.cpp:2408-2417）。
    //   本體只編進 wb_serve（要 god-stack），這裡做「接上了沒」的原始碼棘輪（build 綠證明不了接上：事件表少一列、替身沒建、頁面沒送都一樣綠）：
    //   產生檔的處理器照 golden 順序、事件表有這一列、sbtExit 有替身與 DFM 父層、兩個 golden 全域有定義、頁面攔 Exit 送 form.event。
    //   argv[2]＝移植樹根目錄（tests/CMakeLists.txt 的 add_test 帶 ${CMAKE_SOURCE_DIR}）；沒帶就不查（同 [1] 的 background.html）。
    std::printf("[7] BarCode Exit button wired to golden sbtExitClick (source ratchet; the body itself is wb_serve-only)\n");
    if (argc > 2) {
        auto slurp = [](const std::string& p) {
            std::ifstream f(p, std::ios::binary);
            std::stringstream ss;
            ss << f.rdbuf();
            std::string s = ss.str(), o;
            o.reserve(s.size());
            for (char c : s) if (c != '\r') o += c;
            return o;
        };
        const std::string root = argv[2];
        const std::string gen = slurp(root + "/FileRW/TestIF_File_BarCode.gen.inc");
        const std::string cpp = slurp(root + "/FileRW/TestIF_File_BarCode.cpp");
        const std::string bcc = slurp(root + "/BarCode/BarCode.cpp");
        const std::string js  = slurp(root + "/../web/page/ht9045_barcode_ev.js");
        const std::string htm = slurp(root + "/../web/page/Setup.BarCode.html");
        CHECK(!gen.empty() && !cpp.empty() && !bcc.empty() && !js.empty() && !htm.empty(), "all five sources readable");
        // (a) 產生檔的處理器：golden :2411-2416 的六句照順序（:2410 Down 已轉成 ";"；#if 0 裡的原文不算）
        std::string body;
        {
            const std::size_t a = gen.find("static void BC_sbtExitClick()\n{");
            const std::size_t b = a == std::string::npos ? a : gen.find("\n}\n", a);
            if (b != std::string::npos) body = gen.substr(a, b - a);
            std::string live;
            bool dead = false;
            std::istringstream in(body);
            for (std::string ln; std::getline(in, ln);) {
                if (ln.compare(0, 5, "#if 0") == 0) { dead = true; continue; }
                if (ln.compare(0, 6, "#endif") == 0) { dead = false; continue; }
                if (!dead) live += ln.substr(0, ln.find("//")) + "\n";
            }
            body = live;
        }
        const char* const kSeq[] = {"filerw::ELMark(\"closed\"); BC_FormClose();", "bShow=false;", "i2DIDCheckSH1Task=1;", "i2DIDCheckSH2Task=1;",
                                    "EL<TSpeedButton>(\"TfBarCode\", \"btStart2DIDCheckSh1\")->Enabled=true;",
                                    "EL<TSpeedButton>(\"TfBarCode\", \"btStart2DIDCheckSh2\")->Enabled=true;"};
        std::size_t at = 0;
        bool inOrder = !body.empty();
        for (const char* s : kSeq) {
            const std::size_t p = body.find(s, at);
            if (p == std::string::npos) { inOrder = false; std::printf("       missing / out of order: %s\n", s); break; }
            at = p + std::strlen(s);
        }
        CHECK(inOrder, "BC_sbtExitClick = golden :2411-2416 in order (Close() -> \"closed\" + BC_FormClose, bShow, both tasks = 1, both check buttons enabled)");
        auto count = [](const std::string& s, const char* t) { int n = 0; for (std::size_t p = s.find(t); p != std::string::npos; p = s.find(t, p + 1)) ++n; return n; };
        CHECK(count(body, "Close();") == 1 && count(body, "BC_FormClose();") == 1, "exactly one FormClose, no bare VCL Close() left live");
        // (b) 事件表、DFM 父層、宣告
        CHECK(gen.find("{\"sbtExit\", \"click\", \"BarCode/BarCode.cpp:2408 TfBarCode::sbtExitClick\", &BC_Ev_sbtExitClick}") != std::string::npos,
              "kBC_Events has the sbtExit click row (golden BarCode.dfm:7246 OnClick = sbtExitClick)");
        CHECK(gen.find("{\"sbtExit\", \"Panel1\"}") != std::string::npos, "kBC_ParentOf: sbtExit is under Panel1 (golden BarCode.dfm:7182)");
        CHECK(gen.find("\nextern int i2DIDCheckSH1Task;\nextern int i2DIDCheckSH2Task;\n") != std::string::npos, "gen.inc forward-declares the two golden globals");
        // (c) 手寫入口：替身開機建、事件表有註冊
        {
            const std::size_t a = cpp.find("static void BC_EvBootProxies()\n{");
            const std::size_t b = a == std::string::npos ? a : cpp.find("\n}\n", a);
            const std::string boot = b == std::string::npos ? std::string() : cpp.substr(a, b - a);
            const std::size_t p = boot.find("EL<TSpeedButton>(\"TfBarCode\", \"sbtExit\");");
            const std::size_t line = p == std::string::npos ? p : boot.rfind('\n', p);
            const bool live = p != std::string::npos && boot.substr(line + 1, p - line - 1).find("//") == std::string::npos;
            CHECK(live, "BC_EvBootProxies builds the sbtExit proxy (not commented out) -- otherwise form.event answers handler-failed");
        }
        CHECK(cpp.find("filerw::PageEventsRegistrar g_evreg(\"TestIF_File_BarCode\", kBC_Events,") != std::string::npos, "kBC_Events is registered");
        // (d) golden BarCode.cpp:46-47 的全域：移植樹有定義（行首，不在註解裡）。golden BarCode.h:1008-1009 的 extern 沒補進移植樹 BarCode/BarCode.h
        //     （經 aHotPlateSubstrate.h 進 221 支 TU；今天唯一的讀者是產生檔，上面 (b) 查它自己的前置宣告）
        CHECK(bcc.find("\nint i2DIDCheckSH1Task;\nint i2DIDCheckSH2Task;\n") != std::string::npos, "BarCode/BarCode.cpp defines i2DIDCheckSH1Task / SH2Task");
        // (e) 頁面：攔 Exit（捕獲階段）、送 form.event、ack.closed 才關；頁面有載入這支
        const std::size_t e0 = js.find("closest('#sbtExit')");
        CHECK(e0 != std::string::npos && js.find("}, true);", e0) != std::string::npos, "ht9045_barcode_ev.js catches #sbtExit in the capture phase");
        CHECK(js.find("control: 'sbtExit', event: 'click'") != std::string::npos && js.find("R.rawCmd('form.event'") != std::string::npos,
              "ht9045_barcode_ev.js sends form.event {control:'sbtExit', event:'click'}");
        CHECK(htm.find("<script src=\"ht9045_barcode_ev.js\"></script>") != std::string::npos && htm.find("id=\"sbtExit\"") != std::string::npos,
              "Setup.BarCode.html loads ht9045_barcode_ev.js and has #sbtExit");
    } else {
        std::printf("  (skipped: no source-root argument)\n");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
