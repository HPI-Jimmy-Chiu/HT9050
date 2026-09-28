// =============================================================================
//  test_teachleave.cpp -- S122 VERIFY: 關掉（或打開）Teach／Motor Test ⇒ fAllMotorHome=false
//
//  //AI(W906-FRW-S122) 20260927 [W906] St01（Steven 團隊）。Suite name (add_test): WebTeachLeave
//
//  受測：WebTeachLeave.cpp（TeachLeaveStep 純邏輯、TeachLeaveTickWith 讀總表＋套用、
//        W906_TeachLeaveTick 的 static state）。總表用真的 WebWindowRegistry.cpp 餵訊框
//        （WebWindowRegistryPut），過期用 WebWindowRegistryAgeConnForTest，不真的等 15 秒。
//  案例：docs/S122_TEACH_LEAVE_PLAN_20260927.md §4.1 的表（＝§2.3 例 1～8、10），另加
//        R81 開關（clearOnOpen=false）、同一拍兩個邊緣、總表缺 fTeach 的既有政策、
//        [17] 回報全部過期時政策對 never 也回「在用」而 S122 沿用上一拍（方案 §2.1 第 5 條少寫的那一半）。
//  裁決：RULINGS_20260927 第 2 條第 18 題＝B；decisions-pending R80～R83＝A。
//  golden V912：main.cpp:28846-28847、uteach.cpp:2442-2443、uteach.cpp:1610。
//
//  純邏輯：不讀寫任何機台檔、不 link god-stack、秒級。
//
//  //AI(W906-FRW-S122) 20260927 [W906] [18]～[26]：通用的表單開／關掛勾（W906_WindowEdgeRegister／WindowEdgeTick），
//    用第三個表單 "FTestIF"（St02 的 Setup.TesterIF；background.html:447 form:'FTestIF'）：登記的拒絕／冪等、開關邊緣、
//    全部過期沿用上一拍、運轉中略過、登記時取樣、只掛一邊、callback 丟例外、表滿、與 S122 同一拍並存。
//    [0]～[17] 一行沒改（S122 行為不變）。
// =============================================================================
#include "WebTeachLeave.h"
#include "WebWindowRegistry.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                          \
    do {                                                                          \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                 \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// background.html buildRegistry 的形狀（契約 §4）：id 是瀏覽器自己的鍵（全小寫），join key 是 form。
// ⚠ 每個訊框都**明說** fTeach／fMotorTest 的狀態：總表收過訊框之後，沒出現的表單照契約 §6 算「開著」，
//   只列一個的話另一個永遠在用，測不出東西（缺表單的政策另外在 [12] 釘住）。
static std::string Frame(const char* teach, const char* mt, const char* contact = "never")
{
    std::string s = "{\"type\":\"ui.windows\",\"seq\":1,\"at\":\"2026-09-27T16:00:00+08:00\","
                    "\"topmost\":\"\",\"modalStack\":[],\"windows\":{";
    s += "\"teach\":{\"form\":\"fTeach\",\"state\":\"";          s += teach;   s += "\",\"fullscreen\":true},";
    s += "\"motortest\":{\"form\":\"fMotorTest\",\"state\":\"";  s += mt;      s += "\",\"fullscreen\":true},";
    s += "\"contact\":{\"form\":\"fContact\",\"state\":\"";      s += contact; s += "\",\"fullscreen\":false},";
    s += "\"idelog\":{\"form\":null,\"state\":\"open\",\"fullscreen\":false}";
    s += "}}";
    return s;
}

static bool Put(std::uint64_t conn, const std::string& frame)
{
    std::string why;
    const bool ok = WebWindowRegistryPut(conn, frame, why);
    if (!ok) std::printf("  (Put conn %lu rejected: %s)\n", (unsigned long)conn, why.c_str());   // 測試的 connId 都很小；MinGW msvcrt 不認 %llu
    return ok;
}

static void Fresh(TeachLeaveState& s)
{
    WebWindowRegistryResetForTest();
    WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    s = TeachLeaveState();
}

static const std::int64_t kOld = 20000;   // > 15 s 門檻 ⇒ 過期

// --- [18]～[26] 用：第三個表單 FTestIF 的訊框與 callback 計數 --------------------------------
// ⚠ 同 Frame() 的理由：每個訊框都明說三個表單的狀態（沒出現的表單照契約 §6 算開著）。
static std::string Frame3(const char* teach, const char* mt, const char* tif)
{
    std::string s = "{\"type\":\"ui.windows\",\"seq\":1,\"at\":\"2026-09-27T18:00:00+08:00\","
                    "\"topmost\":\"\",\"modalStack\":[],\"windows\":{";
    s += "\"teach\":{\"form\":\"fTeach\",\"state\":\"";          s += teach; s += "\",\"fullscreen\":true},";
    s += "\"motortest\":{\"form\":\"fMotorTest\",\"state\":\"";  s += mt;    s += "\",\"fullscreen\":true},";
    s += "\"testerif\":{\"form\":\"FTestIF\",\"state\":\"";      s += tif;   s += "\",\"fullscreen\":true}";
    s += "}}";
    return s;
}

static int g_open1 = 0, g_close1 = 0, g_open2 = 0, g_close2 = 0, g_throws = 0;
static void Open1()  { ++g_open1; }
static void Close1() { ++g_close1; }
static void Open2()  { ++g_open2; }
static void Close2() { ++g_close2; }
static void CloseThrows() { ++g_throws; throw std::runtime_error("test callback failure"); }

static void FreshEdge(TeachLeaveState& s)
{
    Fresh(s);
    WindowEdgeResetForTest();
    g_open1 = g_close1 = g_open2 = g_close2 = g_throws = 0;
}

int main()
{
    std::printf("test_teachleave (S122)\n");
    TeachLeaveState s;
    TeachLeaveVerdict v;
    bool amh = true;

    CHECK(kTeachLeaveClearOnOpen == true, "[0] R81=A: clear-on-open is on");

    // --- [1] 從沒收過總表 -----------------------------------------------------
    std::printf("[1] no frame ever\n");
    Fresh(s); amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLNone && v.edges == 0, "[1] no edge before any frame (Q8-B: policy says not in use)");
    CHECK(amh == true, "[1] flag untouched");

    // --- [2] Teach 按 EXIT ------------------------------------------------------
    std::printf("[2] Teach EXIT\n");
    Fresh(s);
    Put(1, Frame("never", "never"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0, "[2] boot frame (never/never): no edge");
    Put(1, Frame("open", "never"));
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && std::strcmp(v.edge[0].form, "fTeach") == 0 &&
          v.edge[0].closed == false, "[2] open edge clears (R81=A, golden uteach.cpp:1610)");
    CHECK(amh == false, "[2] flag cleared on open");
    amh = true;                                        // 例：Teach 開著時另外做完整機回原點
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[2] still open: no edge, flag stays");
    Put(1, Frame("closed", "never"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear, "[2] closed edge -> kTLClear");
    CHECK(v.edges == 1 && std::strcmp(v.edge[0].form, "fTeach") == 0 && v.edge[0].closed,
          "[2] edge is fTeach closed (golden main.cpp:28847)");
    CHECK(amh == false, "[2] flag cleared on EXIT");
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[2] next tick: no edge again");

    // --- [3] 從 Teach 開 Motor Test，Motor Test 按 EXIT 回到 Teach ----------------
    std::printf("[3] Motor Test EXIT back to Teach\n");
    Fresh(s);
    Put(1, Frame("open", "open"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 2, "[3] both opened in one tick: two edges");
    amh = true;
    Put(1, Frame("open", "closed"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && std::strcmp(v.edge[0].form, "fMotorTest") == 0 &&
          v.edge[0].closed, "[3] fMotorTest closed while fTeach stays open (golden uteach.cpp:2443)");
    CHECK(amh == false, "[3] flag cleared");

    // --- [4] Teach 開著按 F5 重新整理 -------------------------------------------
    std::printf("[4] F5 refresh with Teach open\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    Put(2, Frame("never", "never"));                   // 重新載入的 background.html
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[4] old conn still fresh + new conn never: still in use (fresh union)");
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && v.edge[0].closed, "[4] old conn stale -> fresh never wins -> closed");
    CHECK(amh == false, "[4] flag cleared");

    // --- [5] Teach 開著直接關掉瀏覽器，之後才有人開 HMI ---------------------------
    std::printf("[5] browser closed, nobody reopens (then someone does)\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[5] only a stale 'open' left: still in use (contract section 6), no edge");
    Put(3, Frame("never", "never"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && v.edge[0].closed, "[5] first fresh frame from a new HMI -> closed");
    CHECK(amh == false, "[5] flag cleared");

    // --- [6] Teach 開著切到別的瀏覽器分頁（心跳被節流）----------------------------
    std::printf("[6] tab switched away (heartbeat throttled)\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0, "[6] stale open: no edge");
    Put(1, Frame("open", "never"));                    // 切回來，心跳恢復
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[6] fresh open again: no edge, flag stays");

    // --- [7] WS 斷線後自動重連（頁面沒重新載入）-----------------------------------
    std::printf("[7] WS reconnect without reload\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    Put(4, Frame("open", "never"));                    // 重連後送目前的 WIN_STATE
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0, "[7] new conn says open: no edge");
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[7] old conn stale, new conn open: no edge, flag stays");

    // --- [8] 兩個 HMI 分頁：A 開著 Teach 被節流、B 一直送 never（方案 §2.3 例 8，已知誤判）--------
    std::printf("[8] two HMI tabs (documented false close)\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    Put(2, Frame("never", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && v.edge[0].closed,
          "[8] A stale, B fresh never -> counted as closed (cost: one extra home on next START)");

    // --- [9] 運轉中（SystemStart）Teach 由開變關（R82＝A）-------------------------
    std::printf("[9] closed while SystemStart=1\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    Put(1, Frame("closed", "never"));
    v = TeachLeaveTickWith(s, &amh, true);
    CHECK(v.act == kTLSkipRunning && v.edges == 1 && v.edge[0].closed, "[9] edge seen -> kTLSkipRunning");
    CHECK(amh == true, "[9] flag NOT cleared while running (DoAllProcess would stop mid-way)");
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLNone && v.edges == 0 && amh == true, "[9] after stop: no deferred clear (R82 B not chosen)");
    Put(1, Frame("open", "never"));
    v = TeachLeaveTickWith(s, &amh, true);
    CHECK(v.act == kTLSkipRunning && amh == true, "[9] opened while running: also not cleared");

    // --- [10] 別的表單開關 -------------------------------------------------------
    std::printf("[10] another form (fContact) opens and closes\n");
    Fresh(s);
    Put(1, Frame("never", "never", "open"));
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0, "[10] fContact open: no edge");
    Put(1, Frame("never", "never", "closed"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[10] fContact closed: no edge, flag stays");

    // --- [11] 開啟那一下（R81＝A 才有）、以及 R81 的 B 開關（純邏輯）------------------
    std::printf("[11] open edge, R81 switch\n");
    Fresh(s);
    Put(1, Frame("never", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    Put(1, Frame("open", "never"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 1 && !v.edge[0].closed && v.edge[0].acts,
          "[11] never -> open: kTLClear, closed=false");
    CHECK(amh == false, "[11] flag cleared on open");
    {
        TeachLeaveState p;
        TeachLeaveVerdict q = TeachLeaveStep(p, true, false, false, false);
        CHECK(q.act == kTLNone && q.edges == 1 && !q.edge[0].acts,
              "[11] clearOnOpen=false: open edge reported but no action");
        q = TeachLeaveStep(p, false, false, false, false);
        CHECK(q.act == kTLClear && q.edges == 1 && q.edge[0].closed,
              "[11] clearOnOpen=false: close edge still clears");
    }

    // --- [12] 同一拍兩個邊緣（Teach＋Motor Test 都開著時 F5）-----------------------
    std::printf("[12] both close in one tick\n");
    Fresh(s);
    Put(1, Frame("open", "open"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    Put(2, Frame("never", "never"));
    WebWindowRegistryAgeConnForTest(1, kOld);
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.act == kTLClear && v.edges == 2 &&
          std::strcmp(v.edge[0].form, "fTeach") == 0 && std::strcmp(v.edge[1].form, "fMotorTest") == 0 &&
          v.edge[0].closed && v.edge[1].closed, "[12] two close edges, fTeach first");
    CHECK(amh == false, "[12] flag cleared");

    // --- [13] 總表缺 fTeach／fMotorTest（既有政策：契約 §6 算開著；方案 §7）------------
    std::printf("[13] frame without fTeach/fMotorTest (existing policy)\n");
    Fresh(s);
    Put(1, "{\"type\":\"ui.windows\",\"seq\":1,\"windows\":{\"contact\":{\"form\":\"fContact\",\"state\":\"never\"}}}");
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 2 && !v.edge[0].closed && !v.edge[1].closed,
          "[13] unknown forms read as in use -> one opened edge each (same answer MainProc gets)");
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[13] stays in use: never a close edge (not introduced by S122)");

    // --- [14] 壞訊框不動任何東西 ---------------------------------------------------
    std::printf("[14] bad frame\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    TeachLeaveTickWith(s, &amh, false);
    amh = true;
    {
        std::string why;
        CHECK(!WebWindowRegistryPut(1, "{not json", why), "[14] bad frame rejected by the registry");
    }
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[14] bad frame: no edge, flag stays");

    // --- [15] allMotorHome == 0：只判斷不寫 -----------------------------------------
    std::printf("[15] null flag pointer\n");
    Fresh(s);
    Put(1, Frame("open", "never"));
    v = TeachLeaveTickWith(s, 0, false);
    CHECK(v.act == kTLClear && v.edges == 1, "[15] verdict still computed without a flag");

    // --- [17] 全部過期時政策對 never 也回「在用」；S122 沿用上一拍（不是邊緣）-----------------
    //   方案 §2.1 第 5 條只寫「最後說過 open ⇒ 仍算在用」；實際上 FShowConservative 對全部過期一律回 true。
    //   這一格釘住兩件事：政策本身沒被改（MainProc 過期期間照舊暫停），而 S122 不把它當成「打開」。
    std::printf("[17] all-stale: policy says in use even for 'never'; S122 holds\n");
    Fresh(s);
    Put(1, Frame("never", "never"));
    amh = true;
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0, "[17] fresh never/never: no edge");
    WebWindowRegistryAgeConnForTest(1, kOld);
    CHECK(WebWindowRegistryFShowPolicy("fTeach") == true && WebWindowRegistryFShowPolicy("fMotorTest") == true,
          "[17] registry policy unchanged: all-stale never reads as in use (contract section 6)");
    CHECK(WebWindowRegistryQuery("fMotorTest").stale == true, "[17] query marks it stale");
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[17] S122 holds the previous value: no 'opened' edge, flag stays");
    Put(1, Frame("never", "never"));
    v = TeachLeaveTickWith(s, &amh, false);
    CHECK(v.edges == 0 && amh == true, "[17] heartbeat back (fresh never): no 'closed' edge either");

    // --- [16] W906_TeachLeaveTick（wb_serve 呼叫的那一個；static state）-----------------
    std::printf("[16] W906_TeachLeaveTick (static state)\n");
    Fresh(s);
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == true, "[16] no frame: flag untouched");
    Put(1, Frame("open", "never"));
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false, "[16] open edge clears");
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == true, "[16] steady open: flag stays");
    Put(1, Frame("closed", "never"));
    W906_TeachLeaveTick(&amh, true);
    CHECK(amh == true, "[16] closed while running: not cleared");
    Put(1, Frame("open", "never"));
    W906_TeachLeaveTick(&amh, false);
    Put(1, Frame("closed", "never"));
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false, "[16] closed while stopped: cleared");

    // =========================================================================
    //  [18]～[26] 通用的表單開／關掛勾（W906_WindowEdgeRegister）—— 第三個表單 FTestIF
    //  //AI(W906-FRW-S122) 20260927 [W906] 放在 [16] 之後：[16] 以前登記表一直是空的（W906_TeachLeaveTick 的
    //    WindowEdgeTick 什麼都不做），S122 的 52 個檢查照舊。
    // =========================================================================
    WindowEdgeTickResult r;

    // --- [18] 登記：拒絕與冪等 ------------------------------------------------------
    std::printf("[18] register: refusals and idempotence\n");
    FreshEdge(s);
    CHECK(!W906_WindowEdgeRegister(0, Open1, Close1, false), "[18] null form refused");
    CHECK(!W906_WindowEdgeRegister("", Open1, Close1, false), "[18] empty form refused");
    {
        std::string longName(kWindowEdgeFormMax, 'x');
        CHECK(!W906_WindowEdgeRegister(longName.c_str(), Open1, Close1, false), "[18] form name of kWindowEdgeFormMax chars refused");
        CHECK(!W906_WindowEdgeRegister("FTestIF", 0, 0, false), "[18] neither onOpen nor onClose: refused");
        CHECK(WindowEdgeCount() == 0, "[18] refusals register nothing");
        longName.resize(kWindowEdgeFormMax - 1);
        CHECK(W906_WindowEdgeRegister(longName.c_str(), Open1, Close1, false) && WindowEdgeCount() == 1,
              "[18] kWindowEdgeFormMax-1 chars accepted");
        WindowEdgeResetForTest();
    }
    CHECK(W906_WindowEdgeRegister("FTestIF", Open1, Close1, false) && WindowEdgeCount() == 1,
          "[18] FTestIF registered before any frame");
    CHECK(W906_WindowEdgeRegister("FTestIF", Open1, Close1, false) && WindowEdgeCount() == 1,
          "[18] same form+callbacks again: idempotent, still one entry");
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_open1 == 0 && g_close1 == 0, "[18] no frame ever: no edge (Q8-B)");

    // --- [19] 開／關邊緣，走 wb_serve 呼叫的 W906_TeachLeaveTick ---------------------------
    std::printf("[19] FTestIF open / close through W906_TeachLeaveTick\n");
    amh = true;
    Put(1, Frame3("never", "never", "never"));
    W906_TeachLeaveTick(&amh, false);
    CHECK(g_open1 == 0 && g_close1 == 0, "[19] boot frame (never): no callback");
    Put(1, Frame3("never", "never", "open"));
    W906_TeachLeaveTick(&amh, false);
    CHECK(g_open1 == 1 && g_close1 == 0, "[19] FTestIF opened -> onOpen once");
    W906_TeachLeaveTick(&amh, false);
    CHECK(g_open1 == 1 && g_close1 == 0, "[19] still open: no second call");
    Put(1, Frame3("never", "never", "closed"));
    W906_TeachLeaveTick(&amh, false);
    CHECK(g_open1 == 1 && g_close1 == 1, "[19] FTestIF closed -> onClose once (golden cTesterIF.cpp:1215 FormClose)");
    CHECK(amh == true, "[19] a registered form's edges never touch fAllMotorHome");
    Put(1, Frame3("open", "never", "closed"));
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false && g_open1 == 1 && g_close1 == 1, "[19] fTeach opens: S122 clears, FTestIF callbacks not called");
    Put(1, Frame3("closed", "never", "closed"));
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false && g_open1 == 1 && g_close1 == 1, "[19] fTeach closes: S122 clears, FTestIF callbacks not called");

    // --- [20] 全部過期：沿用上一拍（同 S122 [5]／[17]）----------------------------------
    std::printf("[20] all-stale: FTestIF holds the previous value\n");
    FreshEdge(s);
    W906_WindowEdgeRegister("FTestIF", Open1, Close1, false);
    Put(1, Frame3("never", "never", "never"));
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0, "[20] fresh never: no edge");
    WebWindowRegistryAgeConnForTest(1, kOld);
    CHECK(WebWindowRegistryFShowPolicy("FTestIF") == true,
          "[20] registry policy unchanged: all-stale never reads as in use (contract section 6)");
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_open1 == 0, "[20] held: no 'opened' edge while every report is stale");
    Put(1, Frame3("never", "never", "open"));
    r = WindowEdgeTick(false);
    CHECK(r.edges == 1 && r.called == 1 && g_open1 == 1, "[20] fresh open -> opened");
    WebWindowRegistryAgeConnForTest(1, kOld);
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_close1 == 0, "[20] only a stale 'open' left (browser gone / tab throttled): no 'closed' edge");
    Put(2, Frame3("never", "never", "never"));        // 新的 HMI 連上（或 F5 重新整理的那一頁）
    r = WindowEdgeTick(false);
    CHECK(r.edges == 1 && g_close1 == 1, "[20] first fresh report from a new HMI -> closed (S122 plan 2.3 ex. 4/5)");

    // --- [21] 運轉中略過；同一個表單兩筆 --------------------------------------------------
    std::printf("[21] skipWhileRunning, two entries on one form\n");
    FreshEdge(s);
    CHECK(W906_WindowEdgeRegister("FTestIF", Open1, Close1, false), "[21] entry A (runs while running)");
    CHECK(W906_WindowEdgeRegister("FTestIF", Open2, Close2, true) && WindowEdgeCount() == 2,
          "[21] entry B (skipWhileRunning) is a second entry on the same form");
    Put(1, Frame3("never", "never", "never"));
    WindowEdgeTick(false);
    Put(1, Frame3("never", "never", "open"));
    r = WindowEdgeTick(true);
    CHECK(r.edges == 2 && r.called == 1 && r.skipped == 1, "[21] opened while SystemStart: A called, B skipped");
    CHECK(g_open1 == 1 && g_open2 == 0, "[21] only A's onOpen ran");
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_open2 == 0, "[21] after stop: no deferred call for B (same as S122 R82=A)");
    Put(1, Frame3("never", "never", "closed"));
    r = WindowEdgeTick(false);
    CHECK(r.edges == 2 && r.called == 2 && g_close1 == 1 && g_close2 == 1, "[21] closed while stopped: both called");
    CHECK(W906_WindowEdgeRegister("FTestIF", Open2, Close2, false) && WindowEdgeCount() == 2,
          "[21] re-register B with skip=false: same entry, flag updated");
    Put(1, Frame3("never", "never", "open"));
    r = WindowEdgeTick(true);
    CHECK(r.called == 2 && r.skipped == 0 && g_open2 == 1, "[21] B now runs while running too");

    // --- [22] 登記當下取樣：登記前就開著的不算「打開」 ----------------------------------------
    std::printf("[22] registration samples the current state\n");
    FreshEdge(s);
    Put(1, Frame3("never", "never", "open"));
    CHECK(W906_WindowEdgeRegister("FTestIF", Open1, Close1, false), "[22] registered while FTestIF is already open");
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_open1 == 0, "[22] no 'opened' edge for a state that predates registration");
    Put(1, Frame3("never", "never", "closed"));
    r = WindowEdgeTick(false);
    CHECK(r.edges == 1 && g_close1 == 1, "[22] its close is still seen");

    // --- [23] 只掛 onClose -------------------------------------------------------------
    std::printf("[23] only onClose registered\n");
    FreshEdge(s);
    W906_WindowEdgeRegister("FTestIF", 0, Close1, false);
    Put(1, Frame3("never", "never", "open"));
    r = WindowEdgeTick(true);
    CHECK(r.edges == 1 && r.called == 0 && r.skipped == 0, "[23] open edge seen, nothing to call (not counted as skipped)");
    Put(1, Frame3("never", "never", "closed"));
    r = WindowEdgeTick(false);
    CHECK(r.edges == 1 && r.called == 1 && g_close1 == 1, "[23] close edge calls onClose");

    // --- [24] callback 丟例外 ---------------------------------------------------------
    std::printf("[24] callback throws\n");
    FreshEdge(s);
    W906_WindowEdgeRegister("FTestIF", 0, CloseThrows, false);
    W906_WindowEdgeRegister("FTestIF", Open1, Close1, false);
    Put(1, Frame3("never", "never", "open"));
    WindowEdgeTick(false);
    Put(1, Frame3("never", "never", "closed"));
    r = WindowEdgeTick(false);
    CHECK(g_throws == 1 && r.called == 2, "[24] throwing onClose was called and the exception caught");
    CHECK(g_close1 == 1, "[24] the entry after it still ran in the same tick");
    r = WindowEdgeTick(false);
    CHECK(r.edges == 0 && g_throws == 1, "[24] edge consumed: not called again on the next tick");

    // --- [25] 表滿 ---------------------------------------------------------------------
    std::printf("[25] table full\n");
    FreshEdge(s);
    {
        bool allOk = true;
        char name[16];
        for (int i = 0; i < kWindowEdgeMax; ++i) {
            std::snprintf(name, sizeof(name), "fX%d", i);
            allOk = W906_WindowEdgeRegister(name, Open1, 0, false) && allOk;
        }
        CHECK(allOk && WindowEdgeCount() == kWindowEdgeMax, "[25] kWindowEdgeMax entries accepted");
        CHECK(!W906_WindowEdgeRegister("FTestIF", Open1, Close1, false) && WindowEdgeCount() == kWindowEdgeMax,
              "[25] one more is refused");
        CHECK(W906_WindowEdgeRegister("fX3", Open1, 0, false) && WindowEdgeCount() == kWindowEdgeMax,
              "[25] re-registering an existing entry still succeeds when full");
    }

    // --- [26] S122 與登記的表單同一拍 ------------------------------------------------------
    std::printf("[26] S122 and a registered form in the same tick\n");
    FreshEdge(s);
    W906_WindowEdgeRegister("FTestIF", Open1, Close1, true);
    Put(1, Frame3("never", "never", "never"));
    W906_TeachLeaveTick(&amh, false);                  // 對齊 W906_TeachLeaveTick 的 static state
    Put(1, Frame3("open", "never", "open"));
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false && g_open1 == 1, "[26] Teach and FTestIF open in one tick: S122 clears and onOpen runs");
    Put(1, Frame3("closed", "never", "closed"));
    amh = true;
    W906_TeachLeaveTick(&amh, true);
    CHECK(amh == true && g_close1 == 0,
          "[26] both close while running: S122 not cleared (R82=A), FTestIF onClose skipped (skipWhileRunning)");
    Put(1, Frame3("open", "never", "open"));
    W906_TeachLeaveTick(&amh, false);
    Put(1, Frame3("closed", "never", "closed"));
    amh = true;
    W906_TeachLeaveTick(&amh, false);
    CHECK(amh == false && g_open1 == 2 && g_close1 == 1, "[26] both close while stopped: S122 clears and onClose runs");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
