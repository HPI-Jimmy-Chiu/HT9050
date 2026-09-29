// =============================================================================
//  WebTeachLeave.cpp  --  S122：關掉（或打開）Teach／Motor Test ⇒ 標成必須重新回原點（實作）
//
//  //AI(W906-FRW-S122) 20260927 [W906] St01（Steven 團隊）。設計理由、golden 出處
//  （V912 main.cpp:28846-28847、uteach.cpp:2442-2443、uteach.cpp:1610）與裁決出處
//  （RULINGS_20260927 第 2 條第 18 題＝B；decisions-pending R80～R83＝A）寫在 WebTeachLeave.h
//  的檔頭，這裡不重複。
//
//  ⚠ 只 include 本檔標頭、WebWindowRegistry.h 與標準庫 —— **不** include cmydef.h：
//    tests/test_teachleave.cpp 只連 WebWindowRegistry.cpp＋cJSON，拉進機台全域會連不起來。
//
//  //AI(W906-FRW-S122) 20260927 [W906] 檔尾加了通用的表單開／關掛勾（W906_WindowEdgeRegister／WindowEdgeTick），
//    規則與限制寫在 WebTeachLeave.h 尾段。S122 的取樣抽成 WindowEdgeSample，行為不變（ctest [0]～[17] 照舊）。
// =============================================================================
#include "WebTeachLeave.h"

#include "WebWindowRegistry.h"

#include <cstdio>
#include <cstring>
#include <exception>

namespace ht9045 {

//AI(W906-FRW-S122) 20260927 [W906] R81＝A（decisions-pending R81，St01 建議 A）：打開那一下也清 ——
//   照 golden TfTeach::FormShow uteach.cpp:1610。改成 false 就是 R81 的 B（只在關掉時清）。
const bool kTeachLeaveClearOnOpen = true;

namespace {

// 每個邊緣對應的 golden 行（印在主控台那一行裡，上機時對得回去）。
const char* GoldenLineFor(const char* form, bool closed)
{
    const bool teach = std::strcmp(form, "fTeach") == 0;
    if (closed) return teach ? "main.cpp:28847 (after fTeach->ShowModal)"
                             : "uteach.cpp:2443 (after fMotorTest->ShowModal)";
    return teach ? "uteach.cpp:1610 (TfTeach::FormShow)"
                 : "uteach.cpp:1610 via Teach (golden opens Motor Test only from Teach, uteach.cpp:2442)";
}

} // namespace

// -----------------------------------------------------------------------------
TeachLeaveVerdict TeachLeaveStep(TeachLeaveState& s, bool teachInUse, bool mtInUse,
                                 bool systemStart, bool clearOnOpen)
{
    TeachLeaveVerdict v;
    v.act   = kTLNone;
    v.edges = 0;
    for (int i = 0; i < 2; ++i) {
        v.edge[i].form   = "";
        v.edge[i].closed = false;
        v.edge[i].acts   = false;
    }

    // ★ 兩個表單**各自**記上一拍。分開記才照得到 golden uteach.cpp:2443：
    //   從 Teach 開的 Motor Test 按 EXIT 回到 Teach（fTeach 仍在用），這一下就要清。
    struct Slot { bool* was; bool now; const char* form; };
    Slot slot[2] = { { &s.teachWas, teachInUse, "fTeach"     },
                     { &s.mtWas,    mtInUse,    "fMotorTest" } };

    bool anyActs = false;
    for (int i = 0; i < 2; ++i) {
        const bool was = *slot[i].was;
        *slot[i].was = slot[i].now;
        if (was == slot[i].now) continue;            // 沒有邊緣

        TeachLeaveEdge& e = v.edge[v.edges++];
        e.form   = slot[i].form;
        e.closed = was && !slot[i].now;              // 在用 -> 不在用
        e.acts   = e.closed || clearOnOpen;          // 打開那一下看 R81
        if (e.acts) anyActs = true;
    }

    // R82＝A：運轉中看到了也不清（golden 到不了這個狀態：main.cpp:28829-28830 運轉中進不了 Teach）。
    //   ⚠ 不記下來等停機再補清 —— 那是 R82 的 B，沒選。
    if (anyActs) v.act = systemStart ? kTLSkipRunning : kTLClear;
    return v;
}

// -----------------------------------------------------------------------------
//AI(W906-FRW-S122) 20260927 [W906] S122 與 W906_WindowEdgeRegister 的每一筆共用的取樣（理由見下面 TeachLeaveTickWith 的註解）。
//   兩個呼叫都是純讀（WebWindowRegistry.cpp WebWindowRegistryQuery／FShowPolicy 只讀 Store() 與時鐘）。
bool WindowEdgeSample(const char* form, bool prev)
{
    if (WebWindowRegistryQuery(form).stale) return prev;
    return WebWindowRegistryFShowPolicy(form);
}

// -----------------------------------------------------------------------------
TeachLeaveVerdict TeachLeaveTickWith(TeachLeaveState& s, bool* allMotorHome, bool systemStart)
{
    // ★ 跟 MainProc 暫停用的是同一個判斷（csystem.cpp MainProc → W906_FormFShow →
    //   WebMotorAccessLive.cpp W906_HookFShow → WebWindowRegistryFShowPolicy）。
    //
    //AI(W906-FRW-S122) 20260927 [W906] 只看新鮮回報：某個表單的回報**全部過期**時（WebWindowRegistryQuery().stale），
    //   這一拍沿用上一拍的值，不算邊緣。
    //   為什麼：全部過期時 FShowConservative 對**任何**表單都回 true（WebWindowRegistry.cpp `if (q.stale) return true;`，
    //   契約 §6），連最後說 never／closed 的也一樣 —— 方案 §2.1 第 5 條只寫了「最後說過 open ⇒ 仍算在用」，少了這一半
    //   （ctest [5]／[6]／[17] 量到）。不擋的話，瀏覽器關掉或分頁被節流 15 秒，fMotorTest（never）就會「打開」
    //   ⇒ R81 清旗標；心跳回來又「關掉」⇒ 再清 —— 跟 R80 選的「關瀏覽器要等下一個 HMI 連上、切到別的分頁不算」相反。
    //   沿用上一拍之後：MainProc 在過期期間照舊暫停（總表政策不變）；新鮮回報回來時，只有「過期前在用、回來不在用」
    //   才是關掉（例：F5 重新整理、關掉瀏覽器後重開 HMI），正是方案 §2.3 例 3～7 要的結果。
    //   從沒收過總表（Q8-B）與總表裡缺這個表單（契約 §6 不可知）都不算過期，照 FShowPolicy。
    //AI(W906-FRW-S122) 20260927 [W906] 原本這裡是四行（兩個 .stale、兩個「stale ? 上一拍 : FShowPolicy」），
    //   同一個式子抽成 WindowEdgeSample，讓 W906_WindowEdgeRegister 的各筆用同一套；結果相同。
    const bool teachNow   = WindowEdgeSample("fTeach",     s.teachWas);
    const bool mtNow      = WindowEdgeSample("fMotorTest", s.mtWas);
    const TeachLeaveVerdict v = TeachLeaveStep(s, teachNow, mtNow, systemStart, kTeachLeaveClearOnOpen);

    // 每個邊緣印一行（不管 fAllMotorHome 原本是什麼），上機才看得到判斷有沒有發生。
    for (int i = 0; i < v.edges; ++i) {
        const TeachLeaveEdge& e = v.edge[i];
        const char* what = e.closed ? "closed" : "opened";
        if (!e.acts) {
            std::printf("[S122] %s %s -> no action (clear-on-open off; R81)\n", e.form, what);
        } else if (v.act == kTLSkipRunning) {
            std::printf("[S122] %s %s while SystemStart=1 -> not cleared (fAllMotorHome=%d; "
                        "golden cannot reach this: main.cpp:28829; R82=A)\n",
                        e.form, what, allMotorHome ? (*allMotorHome ? 1 : 0) : -1);
        } else if (allMotorHome) {
            const bool was = *allMotorHome;
            *allMotorHome = false;
            std::printf("[S122] %s %s -> fAllMotorHome=false (was %d; golden V912 %s; "
                        "RULINGS_20260927 #18=B)\n",
                        e.form, what, was ? 1 : 0, GoldenLineFor(e.form, e.closed));
        } else {
            std::printf("[S122] %s %s -> would clear fAllMotorHome (no flag given)\n", e.form, what);
        }
    }
    if (v.edges > 0) std::fflush(stdout);
    return v;
}

} // namespace ht9045

// -----------------------------------------------------------------------------
void W906_TeachLeaveTick(bool* allMotorHome, bool systemStart)
{
    // 初值 false/false ＝ 還沒看過；wb_serve 開機時總表也還沒收過（FShowPolicy 回 false），
    // 所以第一拍不會無中生有一個邊緣。
    static ht9045::TeachLeaveState s;
    ht9045::TeachLeaveTickWith(s, allMotorHome, systemStart);
    //AI(W906-FRW-S122) 20260927 [W906] 同一拍接著跑 W906_WindowEdgeRegister 登記的各筆（S122 先跑、上面那行不變）。
    //   登記表是空的時候 WindowEdgeTick 什麼都不做 ⇒ 沒有人登記之前，這一拍跟 0b166feb 的行為完全相同。
    ht9045::WindowEdgeTick(systemStart);
}

// =============================================================================
//  通用的表單開／關掛勾（W906_WindowEdgeRegister）—— 規則、時機、限制寫在 WebTeachLeave.h 尾段。
//  //AI(W906-FRW-S122) 20260927 [W906] St01（Steven 團隊）。第一個用的是 St02 的 Setup.TesterIF（"FTestIF"）：
//    golden V912 cTesterIF.cpp:1215-1235 TFTestIF::FormClose 在網頁沒有事件可接（FROM_STEVEN §4 17:21 St02 (d)）。
// =============================================================================
namespace ht9045 {
namespace {

struct WindowEdgeEntry {
    char             form[kWindowEdgeFormMax];
    W906WindowEdgeFn onOpen;
    W906WindowEdgeFn onClose;
    bool             skipWhileRunning;
    bool             was;               // 上一拍 WindowEdgeSample(form, was)
};

// 固定大小、不配置記憶體；只在 wb_serve 主迴圈那條執行緒上讀寫（登記在開機、判斷在 500 ms 拍子）。
WindowEdgeEntry g_edge[kWindowEdgeMax];
int             g_edgeCount = 0;
bool            g_edgeInTick = false;   // callback 裡再叫 WindowEdgeTick 不重入

// callback 丟例外不可以把 wb_serve 主迴圈帶走；接住、印一行，邊緣照樣算用掉（was 在呼叫前就更新了）。
void CallEdgeFn(W906WindowEdgeFn fn, const char* form, const char* what)
{
    try {
        fn();
    } catch (const std::exception& ex) {
        std::printf("[WinEdge] %s %s -> callback threw: %s (edge consumed, loop continues)\n", form, what, ex.what());
    } catch (...) {
        std::printf("[WinEdge] %s %s -> callback threw a non-std exception (edge consumed, loop continues)\n",
                    form, what);
    }
}

} // namespace

// -----------------------------------------------------------------------------
WindowEdgeTickResult WindowEdgeTick(bool systemStart)
{
    WindowEdgeTickResult r;
    r.edges = 0;
    r.called = 0;
    r.skipped = 0;
    if (g_edgeInTick) return r;
    g_edgeInTick = true;

    // callback 裡新登記的那一筆從下一拍才開始判斷（登記時已經取過樣，不會漏也不會多一個邊緣）。
    const int n = g_edgeCount;
    for (int i = 0; i < n; ++i) {
        WindowEdgeEntry& e = g_edge[i];
        const bool now = WindowEdgeSample(e.form, e.was);   // ★ 跟 S122 同一個取樣：全部過期 ⇒ 沿用上一拍
        if (now == e.was) continue;                         // 沒有邊緣
        e.was = now;                                        // 先更新：callback 丟例外也算用掉
        ++r.edges;

        const bool closed = !now;                           // 在用 -> 不在用
        const char* what = closed ? "closed" : "opened";
        const W906WindowEdgeFn fn = closed ? e.onClose : e.onOpen;
        if (!fn) {
            std::printf("[WinEdge] %s %s -> no %s registered\n", e.form, what, closed ? "onClose" : "onOpen");
        } else if (systemStart && e.skipWhileRunning) {
            // 同 S122 的 R82＝A：運轉中不做、只印一行；之後也不補呼叫。
            ++r.skipped;
            std::printf("[WinEdge] %s %s while SystemStart=1 -> %s not called (skipWhileRunning; no deferred call)\n",
                        e.form, what, closed ? "onClose" : "onOpen");
        } else {
            ++r.called;
            std::printf("[WinEdge] %s %s -> %s (SystemStart=%d)\n", e.form, what,
                        closed ? "onClose" : "onOpen", systemStart ? 1 : 0);
            std::fflush(stdout);                            // callback 自己的輸出排在這一行後面
            CallEdgeFn(fn, e.form, what);
        }
    }
    if (r.edges > 0) std::fflush(stdout);
    g_edgeInTick = false;
    return r;
}

// -----------------------------------------------------------------------------
int WindowEdgeCount() { return g_edgeCount; }

void WindowEdgeResetForTest()
{
    g_edgeCount = 0;
    g_edgeInTick = false;
}

} // namespace ht9045

// -----------------------------------------------------------------------------
bool W906_WindowEdgeRegister(const char* form, W906WindowEdgeFn onOpen, W906WindowEdgeFn onClose,
                             bool skipWhileRunning)
{
    using namespace ht9045;
    if (!form || !*form) {
        std::printf("[WinEdge] register refused: empty form name\n");
        return false;
    }
    if (std::strlen(form) >= static_cast<std::size_t>(kWindowEdgeFormMax)) {
        std::printf("[WinEdge] register refused: form name longer than %d chars\n", kWindowEdgeFormMax - 1);
        return false;
    }
    if (!onOpen && !onClose) {
        std::printf("[WinEdge] register refused: %s has neither onOpen nor onClose\n", form);
        return false;
    }
    // 冪等：同一組 form＋onOpen＋onClose 已經在表上 ⇒ 不新增、不重取樣，只更新 skipWhileRunning。
    for (int i = 0; i < g_edgeCount; ++i) {
        WindowEdgeEntry& e = g_edge[i];
        if (std::strcmp(e.form, form) == 0 && e.onOpen == onOpen && e.onClose == onClose) {
            e.skipWhileRunning = skipWhileRunning;
            return true;
        }
    }
    if (g_edgeCount >= kWindowEdgeMax) {
        std::printf("[WinEdge] register refused: table full (%d entries) -- %s not registered\n", kWindowEdgeMax, form);
        return false;
    }
    WindowEdgeEntry& e = g_edge[g_edgeCount];
    std::memset(e.form, 0, sizeof(e.form));
    std::memcpy(e.form, form, std::strlen(form));
    e.onOpen = onOpen;
    e.onClose = onClose;
    e.skipWhileRunning = skipWhileRunning;
    // 登記當下取一次樣當「上一拍」：登記前就開著的表單不算一次「打開」；開機（還沒收過總表）⇒ false，同 S122 初值。
    e.was = WindowEdgeSample(e.form, false);
    ++g_edgeCount;
    std::printf("[WinEdge] registered %s (onOpen=%s onClose=%s skipWhileRunning=%d; now %s)\n", e.form,
                onOpen ? "yes" : "no", onClose ? "yes" : "no", skipWhileRunning ? 1 : 0,
                e.was ? "in use" : "not in use");
    std::fflush(stdout);
    return true;
}
