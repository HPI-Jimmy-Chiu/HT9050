// =============================================================================
//  test_pagetable.cpp -- 頁面表（WebPageTable.cpp）的 T1＋T2。Suite name (add_test): WebPageTable
//
//  //AI(W906-PAGETAB-Q51) 20260928 [W906] St01（Steven 團隊）。
//
//  受測：WebPageTable.cpp（90 列、回答規則 1～7、程式寫入、START 前的畫面檢查、拍子的邊緣與寬限 STOP、ui.pages JSON），
//        以及 WebTeachLeave.cpp 在頁面表安裝之後的取樣（S122／W906_WindowEdgeRegister 改問頁面表）。
//  總表用真的 WebWindowRegistry.cpp 餵訊框（WebWindowRegistryPut），過期用 WebWindowRegistryAgeConnForTest，
//  WebSocket 數與時鐘用假函式 —— 不真的等 10／15 秒。
//  T2：解析 argv[1]（D:\HT9045\web\background.html）的 `var WINDOWS=[ ... ];`，70 列逐列跟 C++ 表比（id、form、debugOnly）。
//  設計：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §6 T1／T2；
//  裁決：Steven 20260928 Q51、Q-P1（畫面不見 ⇒ 不准 START、運轉中全關 ⇒ 寬限後正常 STOP）、Q-P2＝A、Q-P3＝A。
//
//  純邏輯：只讀 background.html 一個檔、不寫任何檔、不 link god-stack、秒級。
// =============================================================================
#include "WebPageTable.h"
#include "WebTeachLeave.h"
#include "WebWindowRegistry.h"
#include "Public/cJSON.h"

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

// ---- 假的 WebSocket 數、時鐘、host callback ------------------------------------
static int          g_ws = 1;
static std::int64_t g_now = 1000000;
static int g_pauses = 0, g_homeCloses = 0, g_motorStops = 0, g_alarms = 0;
static std::string g_lastWhy;

static int  FakeWs() { return g_ws; }
static std::int64_t FakeNow() { return g_now; }
static void FakePause(const char* why)     { ++g_pauses;     g_lastWhy = why ? why : ""; }
static void FakeHomeClose(const char* why) { ++g_homeCloses; g_lastWhy = why ? why : ""; }
static void FakeMotorStop(const char* why) { ++g_motorStops; g_lastWhy = why ? why : ""; }
static void FakeAlarm(const char* why)     { ++g_alarms;     g_lastWhy = why ? why : ""; }
static std::string g_edges;   // AI(W906-PAGETAB-Q51) 20260928 [W906] [15]：PageTableSetEdgeHook 收到的邊緣，"fContact+" ＝ 開、"fContact-" ＝ 關
static void FakeEdge(const char* form, bool open) { g_edges += form ? form : "?"; g_edges += open ? "+" : "-"; }

static void Arm()
{
    PageTableHost h;
    h.liveWs = &FakeWs; h.pause = &FakePause; h.homeClose = &FakeHomeClose; h.motorStop = &FakeMotorStop; h.alarm = &FakeAlarm;
    PageTableArm(h);
}

// 每一格從乾淨的狀態開始：總表清空、頁面表清空、1 條 WebSocket、時鐘歸位、計數歸零。
static void Fresh(bool arm = true)
{
    WebWindowRegistryResetForTest();
    WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    PageTableResetForTest();
    PageTableSetClockForTest(&FakeNow);
    WindowEdgeResetForTest();
    g_ws = 1; g_now = 1000000;
    g_pauses = g_homeCloses = g_motorStops = g_alarms = 0;
    g_lastWhy.clear();
    if (arm) Arm();
}

// background.html buildRegistry 的形狀：表上 47 個有 golden 名的網頁列全部列出（預設 never、fMain open），
// 再用 overrides 蓋掉（"form=state,form=state"）。skipForms 裡的表單不列（模擬舊外框「沒建立的不進總表」）。
static std::string Frame(const std::string& overrides, const std::string& skipForms = "", bool bye = false)
{
    std::size_t n = 0;
    const PageRowDef* rows = PageTableRows(n);
    std::string s = "{\"type\":\"ui.windows\",\"seq\":1,\"at\":\"2026-09-28T12:00:00+08:00\",\"topmost\":null,"
                    "\"modalStack\":[],";
    if (bye) s += "\"bye\":true,";
    s += "\"windows\":{";
    bool first = true;
    for (std::size_t i = 0; i < PageTableWebRowCount(); ++i) {
        const std::string form = rows[i].form;
        if (form.empty()) continue;
        if ((std::string(",") + skipForms + ",").find("," + form + ",") != std::string::npos) continue;
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
    s += ",\"idelog\":{\"form\":null,\"state\":\"open\",\"fullscreen\":false}";
    s += "}}";
    return s;
}

static bool Put(std::uint64_t conn, const std::string& frame)
{
    std::string why;
    const bool ok = WebWindowRegistryPut(conn, frame, why);
    if (!ok) std::printf("  (Put conn %lu rejected: %s)\n", (unsigned long)conn, why.c_str());
    return ok;
}

static PageTickResult Tick(bool systemStart = false, bool homingAll = false, bool job = false)
{
    PageTickFacts f;
    f.systemStart = systemStart; f.homingAll = homingAll; f.webMotorJob = job;
    return PageTableTick(f);
}

static const std::int64_t kOld = 20000;   // > 15 s ⇒ 過期

// ---- T2：background.html 的 WINDOWS 表 ------------------------------------------------------
struct WebWin { std::string id, form; bool debugOnly; };

static bool ParseWindows(const char* path, std::vector<WebWin>& out, std::string& why)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) { why = std::string("cannot open ") + path; return false; }
    std::stringstream ss; ss << in.rdbuf();
    const std::string t = ss.str();
    const std::size_t s = t.find("var WINDOWS=[");
    if (s == std::string::npos) { why = "no `var WINDOWS=[` in the file"; return false; }
    const std::size_t e = t.find("\n];", s);
    if (e == std::string::npos) { why = "no closing `];` after WINDOWS"; return false; }
    std::size_t p = s;
    while (true) {
        p = t.find("{id:'", p);
        if (p == std::string::npos || p > e) break;
        const std::size_t le = t.find('\n', p);
        const std::string line = t.substr(p, (le == std::string::npos ? e : le) - p);
        WebWin w;
        const std::size_t a = 5, b = line.find('\'', a);
        w.id = line.substr(a, b - a);
        const std::size_t f = line.find("form:");
        std::size_t q = f + 5;
        while (q < line.size() && line[q] == ' ') ++q;
        if (line.compare(q, 4, "null") == 0) w.form = "";
        else if (line[q] == '\'') { const std::size_t c = line.find('\'', q + 1); w.form = line.substr(q + 1, c - q - 1); }
        else { why = "row " + w.id + ": form is neither null nor a quoted name"; return false; }
        w.debugOnly = line.find("debugOnly:true") != std::string::npos;
        out.push_back(w);
        p = p + 1;
    }
    return true;
}

// =============================================================================
int main(int argc, char** argv)
{
    std::printf("test_pagetable (W906-PAGETAB-Q51)\n");

    // --- ① 列數與欄位 -----------------------------------------------------------
    std::printf("[1] rows\n");
    Fresh(false);
    {
        std::size_t n = 0;
        const PageRowDef* rows = PageTableRows(n);
        CHECK(n == 90, "[1] 90 rows (70 web + 10 C++ dialogs + 10 no-web; S-10 trayedit and LI-9 ftpclient moved to the web part)");
        CHECK(PageTableWebRowCount() == 70, "[1] 70 web rows");
        int named = 0, nulls = 0, prog = 0, noweb = 0, both = 0, bad = 0, dup = 0;
        for (std::size_t i = 0; i < n; ++i) {
            const std::string id = rows[i].webId, form = rows[i].form;
            if (i < 70) {
                if (form.empty()) ++nulls; else ++named;
                if (id.empty() || id.find(':') != std::string::npos) ++bad;
                if (rows[i].opener == kPgBoth) ++both;
                if (rows[i].opener != kPgWeb && rows[i].opener != kPgBoth) ++bad;
            } else {
                if (form.empty()) ++bad;
                if (rows[i].opener == kPgProgram) { ++prog; if (id != "cpp:" + form) ++bad; }
                else if (rows[i].opener == kPgNoWeb) { ++noweb; if (id != "none:" + form) ++bad; }
                else ++bad;
            }
            for (std::size_t j = i + 1; j < n; ++j) {
                if (id == rows[j].webId) ++dup;
                if (!form.empty() && form == rows[j].form) ++dup;
            }
        }
        CHECK(named == 49 && nulls == 21, "[1] web rows: 49 golden names + 21 form:null");
        CHECK(prog == 10 && noweb == 10, "[1] 10 C++ dialogs (opener=program) + 10 no-web");
        CHECK(both == 8, "[1] 8 both-rows (fHome fSpeed fCleaning fSCKART fOffSet fCCLink TrayEditForm fFTPClient)");
        CHECK(bad == 0, "[1] ids and openers well-formed");
        CHECK(dup == 0, "[1] no duplicate id or golden name");
        CHECK(PageTableFind("fTeach") >= 0 && PageTableFind("") < 0 && PageTableFind("fNoSuchForm") < 0,
              "[1] PageTableFind");
        const char* q20[] = { "FrmRotate", "HandlerSystem", "Zteach", "fTrayMapping" };   // AI(W906-PAGETAB-S10) 20260929: TrayEditForm now a web row
        int q20ok = 0;
        for (int k = 0; k < 4; ++k) {
            const int i = PageTableFind(q20[k]);
            if (i >= 0 && (rows[i].opener == kPgNoWeb || rows[i].debugOnly)) ++q20ok;
        }
        CHECK(q20ok == 4, "[1] Q20-A four: no-web row or debug-only web row");
        std::size_t nr = 0;
        const char* const* never = WebWindowRegistryNeverReportedForms(nr);
        int inTable = 0;
        for (std::size_t k = 0; k < nr; ++k) if (PageTableFind(never[k]) >= 0) ++inTable;
        CHECK((std::size_t)inTable == nr, "[1] every registry never-reported form is in the page table");
    }

    // --- ② 規則 1～7 ------------------------------------------------------------
    std::printf("[2] answer rules 1-7\n");
    Fresh();
    PageProgramSet("fNote", true, "test");
    CHECK(PageFormAnswer("fNote") == true, "[2] rule 1: program row = program state (open)");
    PageProgramSet("fNote", false, "test");
    CHECK(PageFormAnswer("fNote") == false, "[2] rule 1: program row = program state (closed)");
    CHECK(PageFormAnswer("Zteach") == false, "[2] rule 2: no-web row is closed");
    CHECK(PageFormAnswer("fTeach") == false && PageFormAnswer("fContact") == false,
          "[2] rule 3: no frame ever (Q8-B) -> closed");
    Put(1, Frame("fTeach=open"));
    CHECK(PageFormAnswer("fTeach") == true, "[2] rule 4: fresh open -> open");
    CHECK(PageFormAnswer("fContact") == false, "[2] rule 4: fresh never -> closed");
    Put(1, Frame("fTeach=minimized"));
    CHECK(PageFormAnswer("fTeach") == true, "[2] rule 4: minimized counts as open (contract section 3)");
    Put(1, Frame("", "fBarCode"));
    CHECK(PageFormAnswer("fBarCode") == false, "[2] rule 5: not mentioned by any HMI -> closed (was: open)");
    CHECK(WebWindowRegistryFShowPolicy("fBarCode") == true, "[2] rule 5: registry policy itself unchanged (says open)");
    Put(1, Frame("fTeach=open"));
    WebWindowRegistryAgeConnForTest(1, kOld);
    g_ws = 1;
    CHECK(PageFormAnswer("fTeach") == true, "[2] rule 6: stale + WebSocket live -> last known (open)");
    CHECK(PageFormAnswer("fMotorTest") == false, "[2] rule 6: stale + WebSocket live -> last known (never = closed)");
    CHECK(WebWindowRegistryFShowPolicy("fMotorTest") == true, "[2] rule 6: registry policy itself unchanged (all-stale = open)");
    g_ws = 0;
    CHECK(PageFormAnswer("fTeach") == false, "[2] rule 7: stale + no WebSocket -> closed (was: open)");
    g_ws = 1;
    CHECK(PageFormAnswer("fNoSuchForm") == false, "[2] unknown form: web rules (not reported -> closed)");
    CHECK(PageFormShowing("fTeach", true) == true, "[2] member is OR'ed in (PageFormShowing)");

    // --- ③ absent ＝ 關 --------------------------------------------------------
    std::printf("[3] absent\n");
    Fresh();
    Put(1, Frame("fBarCode=absent,HandlerSystem=absent"));
    CHECK(PageFormAnswer("fBarCode") == false && PageFormAnswer("HandlerSystem") == false,
          "[3] 'absent' (this machine does not build that window) -> closed");
    CHECK(WebWindowRegistryQuery("fBarCode").fromAnyConn == true, "[3] registry keeps it as reported (unknown state)");

    // --- ④ 兩個 HMI 取聯集 -------------------------------------------------------
    std::printf("[4] two HMIs\n");
    Fresh();
    Put(1, Frame("fTeach=open"));
    Put(2, Frame(""));
    CHECK(PageFormAnswer("fTeach") == true, "[4] A open, B never -> open (union)");
    Put(1, Frame(""));
    CHECK(PageFormAnswer("fTeach") == false, "[4] both closed -> closed");

    // --- ⑤ F5：離開訊框（全部關）→ 新頁總表 ----------------------------------------
    std::printf("[5] F5 order\n");
    Fresh();
    Put(1, Frame("fTeach=open"));
    Tick();
    CHECK(PageScreenPresent() == true, "[5] screen present before F5");
    Put(1, Frame("", "", true));                                               // pagehide：全部關（bye）
    CHECK(PageFormAnswer("fTeach") == false, "[5] bye frame: Teach closed at once (was: 15 s)");
    CHECK(PageScreenPresent() == false, "[5] bye frame: no screen");
    Put(2, Frame(""));                                                          // 新頁（新連線）開站送全量
    CHECK(PageScreenPresent() == true, "[5] new page frame: screen back");
    CHECK(PageFormAnswer("fTeach") == false, "[5] new page never -> Teach stays closed");

    // --- ⑥ 程式開 → want → 程式關 ------------------------------------------------
    std::printf("[6] program writer and want\n");
    Fresh();
    Put(1, Frame(""));
    PageProgramSet("fHome", true, "uhome.cpp:646");
    CHECK(PageFormAnswer("fHome") == true, "[6] fHome program open -> open");
    {
        cJSON* j = cJSON_Parse(PageTableJson().c_str());
        const cJSON* rows = j ? cJSON_GetObjectItemCaseSensitive(j, "rows") : 0;
        const cJSON* r = rows ? cJSON_GetArrayItem(rows, PageTableFind("fHome")) : 0;
        const cJSON* want = r ? cJSON_GetObjectItemCaseSensitive(r, "want") : 0;
        const cJSON* wseq = r ? cJSON_GetObjectItemCaseSensitive(r, "wseq") : 0;
        const cJSON* by   = r ? cJSON_GetObjectItemCaseSensitive(r, "by") : 0;
        CHECK(cJSON_IsString(want) && std::string(want->valuestring) == "open" && cJSON_IsNumber(wseq) &&
              wseq->valuedouble == 1 && cJSON_IsString(by) && std::string(by->valuestring) == "uhome.cpp:646",
              "[6] ui.pages: fHome want=open wseq=1 by=uhome.cpp:646");
        cJSON_Delete(j);
    }
    Put(1, Frame("fHome=open"));                                                // HMI 照做
    Tick();
    PageProgramSet("fHome", false, "uhome.cpp:652");
    {
        cJSON* j = cJSON_Parse(PageTableJson().c_str());
        const cJSON* r = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(j, "rows"), PageTableFind("fHome"));
        const cJSON* want = cJSON_GetObjectItemCaseSensitive(r, "want");
        const cJSON* wseq = cJSON_GetObjectItemCaseSensitive(r, "wseq");
        CHECK(std::string(want->valuestring) == "close" && wseq->valuedouble == 2, "[6] program close -> want=close wseq=2");
        cJSON_Delete(j);
    }
    CHECK(PageFormAnswer("fHome") == true, "[6] still open while an HMI shows it (web half)");
    Put(1, Frame("fHome=closed"));
    CHECK(PageFormAnswer("fHome") == false, "[6] HMI closed it -> closed");
    Tick();
    CHECK(g_homeCloses == 0, "[6] program's own close is not an operator close (no extra FormClose)");
    PageProgramSet("fNote", true, "wb_serve.cpp:7597");
    {
        cJSON* j = cJSON_Parse(PageTableJson().c_str());
        const cJSON* r = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(j, "rows"), PageTableFind("fNote"));
        CHECK(cJSON_GetObjectItemCaseSensitive(r, "want") == 0, "[6] C++ dialog rows carry no want (the modal channel shows them)");
        cJSON_Delete(j);
    }

    // --- ⑦ 程式列 ＝ 成員 OR 表 --------------------------------------------------
    std::printf("[7] program row = member OR table\n");
    Fresh();
    CHECK(PageFormShowing("MyMessageBox", false) == false, "[7] member false, table closed -> closed");
    CHECK(PageFormShowing("MyMessageBox", true) == true, "[7] member true -> open (C++ fact)");
    PageProgramSet("MyMessageBox", true, "wb_serve.cpp:7600");
    CHECK(PageFormShowing("MyMessageBox", false) == true, "[7] table open -> open");
    CHECK(PageWebShowing("MyMessageBox") == false, "[7] program row has no web half");

    // --- ⑧ 邊緣（S122／W906_WindowEdgeRegister 改問頁面表）-----------------------------
    std::printf("[8] edges through WebTeachLeave (armed)\n");
    Fresh();
    {
        TeachLeaveState s;
        bool amh = true;
        TeachLeaveVerdict v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 0 && amh, "[8] no frame: no edge");
        Put(1, Frame("", "fTeach,fMotorTest"));                                 // 舊外框：沒列 fTeach／fMotorTest
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 0 && amh, "[8] not reported -> closed -> no 'opened' edge (test_teachleave [13] was: opened)");
        Put(1, Frame("fTeach=open"));
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 1 && !v.edge[0].closed && !amh, "[8] Teach opened -> edge, flag cleared (R81=A)");
        amh = true;
        WebWindowRegistryAgeConnForTest(1, kOld);
        g_ws = 1;
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 0 && amh, "[8] stale but WebSocket live -> last known -> no edge");
        g_ws = 0;
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 1 && v.edge[0].closed && !amh, "[8] all web gone (stale, no WebSocket) -> one closed edge");
        amh = true;
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 0 && amh, "[8] ... only once");
        g_ws = 1;
        Put(2, Frame("fTeach=open"));
        TeachLeaveTickWith(s, &amh, false);
        amh = true;
        Put(2, Frame("", "", true));                                            // F5：bye
        v = TeachLeaveTickWith(s, &amh, false);
        CHECK(v.edges == 1 && v.edge[0].closed && !amh, "[8] F5 bye -> Teach closed edge at once (was: ~15 s)");
    }
    Fresh();
    {
        static int opens = 0, closes = 0;
        struct F { static void O() { ++opens; } static void C() { ++closes; } };
        opens = closes = 0;
        CHECK(W906_WindowEdgeRegister("FTestIF", &F::O, &F::C, false), "[8] register FTestIF");
        Put(1, Frame("FTestIF=open"));
        WindowEdgeTick(false);
        Put(1, Frame("FTestIF=closed"));
        WindowEdgeTick(false);
        CHECK(opens == 1 && closes == 1, "[8] registered form: one open + one close through the page table");
    }

    // --- ⑨ 設定中位元（Bit4）---------------------------------------------------------
    std::printf("[9] diagnostics bit\n");
    Fresh();
    {
        Put(1, Frame("fSetup=open"));
        const DiagVerdict a = WebWindowRegistryDiagnosticsOpen(false, 0, false);
        const DiagVerdict b = PageDiagnosticsOpen(false, 0, false);
        CHECK(a.open == b.open && a.tier == b.tier && a.form == b.form && b.open && b.form == "fSetup",
              "[9] fresh: same verdict as WebWindowRegistryDiagnosticsOpen (fSetup, stopped tier)");
        CHECK(PageDiagnosticsOpen(true, 0, false).open == false, "[9] SystemStart: stopped tier not counted");
        Put(1, Frame("fTeach=open"));
        const DiagVerdict c = PageDiagnosticsOpen(true, 0, false);
        CHECK(c.open && c.tier == kDiagAlways && c.form == "fTeach", "[9] Teach open: always tier");
        Put(1, Frame(""));
        CHECK(PageDiagnosticsOpen(false, 0, false).open == false, "[9] all closed -> not in diagnostics");
        PageProgramSet("fHome", true, "uhome.cpp:646");
        CHECK(PageDiagnosticsOpen(true, 0, true).form == "fHome", "[9] homing + Home Monitor (program) -> always tier fHome");
        CHECK(PageDiagnosticsOpen(true, 0, false).open == false, "[9] fHome needs homingActive (Q9)");
        PageProgramSet("fHome", false, "uhome.cpp:652");
        Put(1, Frame("fSetup=open"));
        WebWindowRegistryAgeConnForTest(1, kOld);
        g_ws = 0;
        CHECK(WebWindowRegistryDiagnosticsOpen(false, 0, false).open == true && PageDiagnosticsOpen(false, 0, false).open == false,
              "[9] stale + no WebSocket: registry says open, page table says closed (rule 7)");
    }

    // --- ⑩ ui.pages JSON 形狀 ------------------------------------------------------
    std::printf("[10] ui.pages JSON\n");
    Fresh();
    {
        Put(1, Frame(""));
        const std::string j1 = PageTableJson();
        cJSON* j = cJSON_Parse(j1.c_str());
        CHECK(j != 0, "[10] valid JSON");
        const cJSON* type = cJSON_GetObjectItemCaseSensitive(j, "type");
        const cJSON* seq  = cJSON_GetObjectItemCaseSensitive(j, "seq");
        const cJSON* cnt  = cJSON_GetObjectItemCaseSensitive(j, "count");
        const cJSON* rows = cJSON_GetObjectItemCaseSensitive(j, "rows");
        CHECK(cJSON_IsString(type) && std::string(type->valuestring) == "ui.pages", "[10] type=ui.pages");
        CHECK(cJSON_IsNumber(cnt) && cnt->valuedouble == 90 && cJSON_GetArraySize(rows) == 90, "[10] count=90, 90 rows");
        const cJSON* r0 = cJSON_GetArrayItem(rows, 0);
        CHECK(std::string(cJSON_GetObjectItemCaseSensitive(r0, "id")->valuestring) == "main" &&
              std::string(cJSON_GetObjectItemCaseSensitive(r0, "state")->valuestring) == "open" &&
              cJSON_GetObjectItemCaseSensitive(r0, "on")->valuedouble == 1, "[10] row 0 = main, open, on=1");
        const double s1 = seq->valuedouble;
        cJSON_Delete(j);
        CHECK(PageTableJson() == j1, "[10] nothing changed -> same text, same seq");
        Put(1, Frame("fTeach=open"));
        cJSON* k = cJSON_Parse(PageTableJson().c_str());
        CHECK(cJSON_GetObjectItemCaseSensitive(k, "seq")->valuedouble == s1 + 1, "[10] a change -> seq+1");
        cJSON_Delete(k);
        CHECK(j1.size() < 16000, "[10] size stays small (< 16 KB)");
    }

    // --- ⑪ START 前的畫面檢查（Q-P1 (1)）----------------------------------------------
    std::printf("[11] START gate\n");
    Fresh(false);
    CHECK(PageStartAllowed("t", 0) == true, "[11] not armed (ctest / no server): never refuses");
    Arm();
    std::string why;
    CHECK(PageStartAllowed("t", &why) == false && !why.empty(), "[11] armed, no frame ever: refused (no screen)");
    Put(1, Frame(""));
    CHECK(PageStartAllowed("t", 0) == true, "[11] HMI frame (main open) + WebSocket: allowed");
    g_ws = 0;
    CHECK(PageStartAllowed("t", 0) == false, "[11] fresh frame but no WebSocket at all: refused");
    g_ws = 1;
    Put(1, Frame("", "", true));
    CHECK(PageStartAllowed("t", 0) == false, "[11] after the bye frame: refused");
    Put(2, Frame(""));
    WebWindowRegistryAgeConnForTest(1, kOld);
    WebWindowRegistryAgeConnForTest(2, kOld);
    CHECK(PageStartAllowed("t", 0) == true, "[11] stale main + WebSocket live (throttled tab): allowed (last known)");
    CHECK(W906_PageStartAllowed("t") == true, "[11] global entry");

    // --- ⑫ 拍子：畫面不見 ⇒ 寬限 ⇒ 正常 STOP（Q-P1 (2)）-----------------------------------
    std::printf("[12] no-screen grace -> normal STOP\n");
    Fresh();
    Put(1, Frame(""));
    Tick(true);
    CHECK(g_pauses == 0, "[12] screen present while running: nothing");
    Put(1, Frame("", "", true));                                                // 全部關
    PageTickResult r = Tick(true);
    CHECK(!r.screen && g_pauses == 0, "[12] screen gone: not at once");
    g_now += kPageNoScreenGraceMs - 500;
    Tick(true);
    CHECK(g_pauses == 0, "[12] 9.5 s: still waiting");
    g_now += 500;
    r = Tick(true);
    CHECK(g_pauses == 1 && (r.actions & kPgActPause), "[12] 10 s gone while SystemStart -> pause (golden TfMain::Pause)");
    CHECK(g_alarms == 0, "[12] alarm waits until the pause is processed (SystemStart still 1)");
    g_now += 500;
    r = Tick(false);                                                            // 暫停檢查走完（ckernel.cpp:1046）
    CHECK(g_alarms == 1 && (r.actions & kPgActAlarm) && g_lastWhy.find("no HMI screen") != std::string::npos,
          "[12] SystemStart dropped -> one golden alarm (MES16441, Jimmy E#36=C, R143 renumber)");
    CHECK(std::string(kPageNoScreenAlarmCode) == "MES16441", "[12] alarm code MES16441 (R143, was MES1690)");
    g_now += 500;
    Tick(true);
    CHECK(g_pauses == 1 && g_alarms == 1, "[12] not again right away");
    g_now += kPageNoScreenGraceMs;
    Tick(true);
    CHECK(g_pauses == 2, "[12] still running (e.g. a remote start) one grace later -> pause again");
    g_now += kPageNoScreenAlarmWaitMs;
    Tick(true);
    CHECK(g_alarms == 2, "[12] SystemStart did not drop within 3 s -> alarm anyway");
    Put(2, Frame(""));
    r = Tick(true);
    CHECK(r.screen && r.absentMs == 0, "[12] screen back -> timer reset");

    Fresh();                                                                    // F5 在寬限內回來
    Put(1, Frame(""));
    Tick(true);
    Put(1, Frame("", "", true));
    Tick(true);
    g_now += 3000;
    Put(2, Frame(""));
    Tick(true);
    g_now += kPageNoScreenGraceMs * 2;
    Tick(true);
    CHECK(g_pauses == 0 && g_alarms == 0, "[12] F5 back within the grace: no stop, no alarm");

    Fresh();                                                                    // 停機中全關：不動
    Put(1, Frame(""));
    Tick(false);
    Put(1, Frame("", "", true));
    Tick(false);
    g_now += kPageNoScreenGraceMs * 3;
    Tick(false);
    CHECK(g_pauses == 0 && g_homeCloses == 0 && g_motorStops == 0 && g_alarms == 0,
          "[12] idle machine: browsers closed -> nothing to stop, no alarm");

    Fresh();                                                                    // HOME ALL 中全關
    Put(1, Frame(""));
    PageProgramSet("fHome", true, "uhome.cpp:646");
    Put(1, Frame("fHome=open"));
    Tick(true, true);
    Put(1, Frame("", "", true));
    Tick(true, true);
    CHECK(g_homeCloses == 0, "[12] HOME ALL, screen gone: fHome not closed at once (grace)");
    g_now += kPageNoScreenGraceMs;
    r = Tick(true, true);
    CHECK(g_pauses == 1 && g_homeCloses == 1, "[12] HOME ALL 10 s without screen -> pause + fHome->Close()");

    Fresh();                                                                    // 網頁馬達工作（畫面剛不見那一拍）
    Put(1, Frame("fMotorTest=open"));
    Tick(false, false, true);
    Put(1, Frame("", "", true));
    Tick(false, false, true);                                                   // 這一拍還有工作（死人開關下一拍才取消）
    g_now += kPageNoScreenGraceMs;
    Tick(false, false, false);
    CHECK(g_motorStops == 1 && g_pauses == 0 && g_alarms == 1,
          "[12] web motor job when the screen went away -> motor.stop at the grace + alarm at once (nothing running)");

    // --- ⑬ 操作員關掉 Home Monitor（畫面還在）⇒ 照 golden FormClose ----------------------------
    std::printf("[13] operator closes Home Monitor\n");
    Fresh();
    Put(1, Frame(""));
    PageProgramSet("fHome", true, "uhome.cpp:646");
    Put(1, Frame("fHome=open"));
    Tick(true, true);
    Put(1, Frame("fHome=closed"));
    r = Tick(true, true);
    CHECK(g_homeCloses == 1 && (r.actions & kPgActHomeClose), "[13] closed on the HMI while homing -> fHome->Close() at once");
    CHECK(g_alarms == 0 && g_pauses == 0, "[13] operator close: no no-screen alarm, no pause");
    Fresh();
    Put(1, Frame("fHome=open"));                                                // 操作員自己從選單開（程式沒開）
    Tick();
    Put(1, Frame("fHome=closed"));
    Tick();
    CHECK(g_homeCloses == 0, "[13] operator-opened Home Monitor closed while not homing -> no FormClose");

    // --- ⑭ T2：background.html 的 WINDOWS 表 ---------------------------------------------
    std::printf("[14] T2 background.html WINDOWS vs C++ table\n");
    {
        std::vector<WebWin> web;
        std::string pwhy;
        const char* path = argc > 1 ? argv[1] : "../web/background.html";
        const bool ok = ParseWindows(path, web, pwhy);
        if (!ok) std::printf("  (%s)\n", pwhy.c_str());
        CHECK(ok, "[14] parsed WINDOWS");
        std::size_t n = 0;
        const PageRowDef* rows = PageTableRows(n);
        CHECK(web.size() == PageTableWebRowCount(), "[14] 70 rows on both sides");
        int diff = 0;
        for (std::size_t i = 0; i < web.size() && i < PageTableWebRowCount(); ++i) {
            if (web[i].id != rows[i].webId || web[i].form != rows[i].form || web[i].debugOnly != rows[i].debugOnly) {
                ++diff;
                std::printf("  row %u: web {%s,%s,%d} vs C++ {%s,%s,%d}\n", (unsigned)i, web[i].id.c_str(),
                            web[i].form.c_str(), web[i].debugOnly ? 1 : 0, rows[i].webId, rows[i].form,
                            rows[i].debugOnly ? 1 : 0);
            }
        }
        CHECK(diff == 0, "[14] every row: same id, same golden form (or null), same debugOnly, same order");
        int webHasNoWeb = 0;
        for (std::size_t i = PageTableWebRowCount(); i < n; ++i)
            for (std::size_t k = 0; k < web.size(); ++k)
                if (web[k].form == rows[i].form) ++webHasNoWeb;
        CHECK(webHasNoWeb == 0, "[14] none of the 20 C++-only rows has a web window");
    }

    // AI(W906-PAGETAB-Q51) 20260928 [W906] 步驟 5：網頁列的開／關邊緣交給 hook（wb_serve 接到 FileRW/_EditPage.cpp：C 路頁關窗 ⇒ 下次開窗＝重新開頁）
    std::printf("[15] web-row edges handed to the edge hook (step 5)\n");
    Fresh();
    g_edges.clear();
    PageTableSetEdgeHook(&FakeEdge);
    Put(1, Frame(""));
    Tick();
    CHECK(g_edges == "fMain+", "[15] first frame: only the main screen (locked, always open) -> hook(fMain, open)");  g_edges.clear();
    Put(1, Frame("fContact=open"));
    Tick();
    CHECK(g_edges == "fContact+", "[15] Contact opened -> hook(fContact, open)");
    Put(1, Frame("fContact=minimized"));
    Tick();
    CHECK(g_edges == "fContact+", "[15] minimized is still open -> no edge");
    Put(1, Frame("fContact=closed"));
    Tick();
    CHECK(g_edges == "fContact+fContact-", "[15] Contact closed -> hook(fContact, closed)");
    Put(1, Frame("fSetup=open"));
    Tick();
    g_ws = 0;
    WebWindowRegistryAgeConnForTest(1, kOld);
    Tick();
    CHECK(g_edges == "fContact+fContact-fSetup+fMain-fSetup-", "[15] every browser gone (stale, no WebSocket) -> one close edge per open page (row order)");
    Tick();
    CHECK(g_edges == "fContact+fContact-fSetup+fMain-fSetup-", "[15] no repeat on the next tick");
    Fresh();
    g_edges.clear();
    Put(1, Frame("fSetup=open"));
    Tick();
    CHECK(g_edges.empty(), "[15] PageTableResetForTest clears the hook");
    PageTableSetEdgeHook(0);

    // --- ⑯ R144：SECS S2F42 的 HCACK（AI(W906-PAGETAB-R144) 20260929）---------------------------------
    //  Steven「SECS 的SF code 有ACK 可以回覆, 挑一個正確的ACK進行回覆」⇒ uHGemHT9045.cpp S2F42 回覆前問
    //  W906_PageStartRefusedSince(mark, 這條指令拉起 SoftStart 且沒在運轉)，true 就回 HCACK=2。
    std::printf("[16] R144 SECS HCACK mark / refused-since\n");
    Fresh(false);
    unsigned long m0 = W906_PageStartMark();
    CHECK(W906_PageStartRefusedSince(m0, true) == false, "[16] not armed (ctest / no server): never refused, even with SoftStart raised");
    Arm();
    m0 = W906_PageStartMark();
    CHECK(W906_PageStartRefusedSince(m0, false) == false, "[16] armed, no START tried in this command: not refused");
    CHECK(W906_PageStartRefusedSince(m0, true) == true, "[16] armed, no screen, command raised SoftStart (HOME / AUTO_RETEST): refused");
    CHECK(PageStartAllowed("t", 0) == false, "[16] fMain->Start path: StartFromWeb refuses");
    CHECK(W906_PageStartRefusedSince(m0, false) == true, "[16] a refusal after the mark => refused (HCACK 2)");
    CHECK(W906_PageStartMark() == m0 + 1, "[16] one refusal counted");
    Put(1, Frame(""));
    const unsigned long m1 = W906_PageStartMark();
    CHECK(PageStartAllowed("t", 0) == true, "[16] HMI present: START allowed");
    CHECK(W906_PageStartRefusedSince(m1, true) == false, "[16] HMI present + SoftStart raised: not refused (HCACK stays 0)");
    CHECK(W906_PageStartMark() == m1, "[16] allowed START is not counted");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
