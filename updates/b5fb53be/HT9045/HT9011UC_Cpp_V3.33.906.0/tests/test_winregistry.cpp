// =============================================================================
//  test_winregistry.cpp -- P6-a VERIFY: 視窗狀態總表（取代 golden 的 fShow）
//
//  Wave: AI(W906-P6-WINREG) 20260920.  Suite name (add_test): WebWindowRegistry
//
//  契約：Steven `WINDOW_REGISTRY_CONTRACT.md`（交付包 20260919_fShow）。
//
//  ## 這支測試的重點在哪
//
//  契約 §6 自己寫著：不可知／stale 的方向**與 `guard.systemStart` 相反**，
//  而且「這是整份契約最容易寫反的地方」。所以本檔最重的斷言不是「存得進去」，
//  而是這三條：
//
//    * 不可知（沒人報過）   -> fShow **true**
//    * stale（報過但過期）  -> fShow **true**，而且總表**沒有被清空**
//    * minimized            -> fShow **true**（使用者裁示，不是像素有沒有畫出來）
//
//  三條寫反的後果都是同一個：C++ 以為沒人在教導，**放行本該擋住的 START**。
//
//  ## 另外兩條「會安靜出錯」的
//
//    * 以**連線為單位**保存：第二個分頁不可以洗掉第一個（契約 §9-2）
//    * join key 是 `form` 不是 id：id 是瀏覽器自己的鍵而且全小寫（§4）
// =============================================================================
#include "WebWindowRegistry.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// 契約 §4 的訊框。id 故意用全小寫（`motortest`），form 用駝峰（`fMotorTest`）——
// 那正是契約特別警告的地方。
static std::string Frame(const char* seq, const char* teachState,
                         const char* motorState)
{
    std::string s = "{\"type\":\"ui.windows\",\"seq\":";
    s += seq;
    s += ",\"at\":\"2026-09-20T01:00:00+08:00\"";
    s += ",\"topmost\":\"motortest\"";
    s += ",\"modalStack\":[\"teach\",\"motortest\"]";
    s += ",\"windows\":{";
    s += "\"teach\":{\"form\":\"fTeach\",\"state\":\"";
    s += teachState;
    s += "\",\"fullscreen\":true},";
    s += "\"motortest\":{\"form\":\"fMotorTest\",\"state\":\"";
    s += motorState;
    s += "\",\"fullscreen\":true},";
    // form:null = golden 沒有對應 TForm（頁籤／網頁自己的工具頁）。
    // 契約 §4 事實 5：那是**明確的事實**，不是「不知道」，不要當缺漏去補。
    s += "\"idelog\":{\"form\":null,\"state\":\"open\",\"fullscreen\":false}";
    s += "}}";
    return s;
}

//AI(W906-P6b-A) 20260921: 三層規則的訊框 —— 三層的**每一個**表單都明說一個狀態。
//
// 為什麼要全部列出來，不能只列要測的那一個：政策層對「沒出現在總表裡」的回答是
// **開著**（契約 §6）。只列一個表單的話，另外 31 個都會是不可知 ⇒ 永遠命中第一層，
// 於是每一格都變成「擋住」，測不出任何東西。
//
// 表單清單直接向受測單元要（`WebWindowRegistryTierForms`），不在測試裡另抄一份 ——
// 抄一份就變成「拿自己的清單驗自己的清單」，golden 漏抄一個兩邊會一起漏。
// [13] 另外對清單本身的長度與頭尾逐項對帳，那一格才是與 golden 的對照。
//
// ⚠ 刻意**跳過**不可回報的表單（Q20-甲）：真實的瀏覽器就是不會送它們。
//   送了反而測不到「不送的時候政策層怎麼答」。
static std::string FrameAll(const char* seq, const char* baseState,
                            const char* overrideForm, const char* overrideState)
{
    std::string s = "{\"type\":\"ui.windows\",\"seq\":";
    s += seq;
    s += ",\"at\":\"2026-09-21T12:00:00+08:00\",\"topmost\":\"\",\"modalStack\":[]";
    s += ",\"windows\":{";

    const DiagTier tiers[3] = { kDiagAlways, kDiagContact, kDiagStopped };
    std::vector<std::string> emitted;   // fContact 在第二、三層都有，只能送一次
    bool first = true;
    for (int t = 0; t < 3; ++t) {
        std::size_t n = 0;
        const char* const* forms = WebWindowRegistryTierForms(tiers[t], n);
        for (std::size_t i = 0; i < n; ++i) {
            const std::string form = forms[i];
            if (!WebWindowRegistryIsReportable(form)) continue;   // Q20-甲
            bool dup = false;
            for (std::size_t k = 0; k < emitted.size(); ++k)
                if (emitted[k] == form) dup = true;
            if (dup) continue;
            emitted.push_back(form);
            if (!first) s += ",";
            first = false;
            // id 用「表單名轉小寫」只是為了產生一個唯一的鍵；真實的 id 是
            // 瀏覽器自己的（契約 §4），join key 從頭到尾是 form。
            std::string id = form;
            for (std::size_t k = 0; k < id.size(); ++k)
                if (id[k] >= 'A' && id[k] <= 'Z') id[k] = static_cast<char>(id[k] - 'A' + 'a');
            s += "\"";
            s += id;
            s += "\":{\"form\":\"";
            s += form;
            s += "\",\"state\":\"";
            s += (overrideForm && form == overrideForm && overrideState)
                     ? overrideState : baseState;
            s += "\",\"fullscreen\":false}";
        }
    }
    s += "}}";
    return s;
}

int main()
{
    printf("==== P6-a WebWindowRegistry: 視窗狀態總表 ====\n");
    std::string why;

    // -----------------------------------------------------------------------
    printf("\n[1] 不可知 -> fShow true（契約 §6，方向與 guard.systemStart 相反）\n");
    WebWindowRegistryResetForTest();
    WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    {
        const WinQuery q = WebWindowRegistryQuery("fTeach");
        CHECK(q.state == kWinUnknown, "沒人報過 -> state 是 Unknown（不是 Closed）");
        CHECK(q.fromAnyConn == false, "沒人報過 -> fromAnyConn false");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "★ 不可知時 fShow 保守回 true（寫反就會放行本該擋住的 START）");
    }

    // -----------------------------------------------------------------------
    printf("\n[2] 基本存取 + join key 是 form 不是 id\n");
    {
        CHECK(WebWindowRegistryPut(1001, Frame("1", "open", "closed"), why),
              "合法訊框收得下");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "fTeach state=open -> fShow true");
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == false,
              "fMotorTest state=closed -> fShow false");

        // ★ 用 id 查必須查不到 —— 契約 §4 明講 join key 是 form。
        const WinQuery byId = WebWindowRegistryQuery("motortest");
        CHECK(byId.fromAnyConn == false,
              "★ 用 background.html 的 id 查不到（join key 是 form，不是 id）");

        // form:null 的那一筆不該進表
        const WinRegistryStats st = WebWindowRegistryStats();
        CHECK(st.windowsTotal == 2,
              "form:null 的視窗不進表（它是明確事實，不是缺漏）");
        CHECK(st.lastSeq == 1, "seq 記下來了");
    }

    // -----------------------------------------------------------------------
    printf("\n[3] minimized 算「開著」（使用者裁示，契約 §3）\n");
    {
        CHECK(WebWindowRegistryPut(1001, Frame("2", "minimized", "closed"), why),
              "minimized 訊框收得下");
        const WinQuery q = WebWindowRegistryQuery("fTeach");
        CHECK(q.state == kWinMinimized, "state 確實記成 minimized，沒有塌成 open");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "★ minimized -> fShow true（『有人正在用』≠『像素有沒有畫出來』）");
    }

    // -----------------------------------------------------------------------
    printf("\n[4] never 與 closed 分開記，但 fShow 都是 false\n");
    {
        WebWindowRegistryPut(1001, Frame("3", "never", "closed"), why);
        const WinQuery n = WebWindowRegistryQuery("fTeach");
        const WinQuery c = WebWindowRegistryQuery("fMotorTest");
        CHECK(n.state == kWinNever,  "never 記成 never");
        CHECK(c.state == kWinClosed, "closed 記成 closed（兩者沒有被混成同一個）");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == false,
              "never -> fShow false");
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == false,
              "closed -> fShow false");
    }

    // -----------------------------------------------------------------------
    printf("\n[5] ★ 以連線為單位：第二個分頁不可以洗掉第一個（契約 §9-2）\n");
    {
        WebWindowRegistryResetForTest();
        // 分頁 A：fTeach 開著
        WebWindowRegistryPut(1001, Frame("10", "open", "closed"), why);
        // 分頁 B：fTeach 關著（它自己那一頁沒開 teach）
        WebWindowRegistryPut(2002, Frame("11", "closed", "open"), why);

        CHECK(WebWindowRegistryStats().connections == 2, "兩條連線各自保存");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "★ 分頁 A 說 fTeach 開著，分頁 B 說關著 -> 聯集＝開著，"
              "B 沒有洗掉 A");
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == true,
              "★ 反過來也成立：B 說 fMotorTest 開著就算開著");
    }

    // -----------------------------------------------------------------------
    //AI(W906-MT-FIX1) 20260926: HMI 重新整理／WS 重連時 Motor Test 開著 —— 舊連線最後說 open（永遠留在總表），
    //   新連線每 5 秒說 closed。原本 stale 的 open 永遠蓋過新鮮的 closed ⇒ fMotorTest 永遠開著 ⇒ MainProc 永遠暫停。
    printf("\n[5b] ★ 新鮮的回報蓋過過期的回報（重新整理後不會永遠卡在 open）\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistryPut(2001, Frame("30", "open", "open"), why);     // 舊頁面：兩頁都開著
        WebWindowRegistryAgeConnForTest(2001, kStaleAfterMsDefault + 1000);   // 舊連線斷了，超過門檻
        WebWindowRegistryPut(2002, Frame("1", "closed", "closed"), why);  // 重新整理後的新頁面：都關著
        const WinQuery q = WebWindowRegistryQuery("fMotorTest");
        CHECK(q.state == kWinClosed && q.stale == false, "stale open + fresh closed -> closed（不是 stale）");
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == false, "★ fShow(fMotorTest) = false：MainProc 不再被卡住");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == false, "fShow(fTeach) = false");

        WebWindowRegistryPut(2002, Frame("2", "closed", "open"), why);    // 新頁面又把 Motor Test 打開
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == true, "fresh open -> true");

        WebWindowRegistryResetForTest();                                   // 兩條都新鮮：聯集照舊
        WebWindowRegistryPut(2003, Frame("1", "open", "closed"), why);
        WebWindowRegistryPut(2004, Frame("1", "closed", "closed"), why);
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true, "兩條新鮮連線：任何一條說 open 就是 open（聯集不變）");

        WebWindowRegistryResetForTest();                                   // 只有過期的：照舊往「開著」倒
        WebWindowRegistryPut(2005, Frame("1", "closed", "open"), why);
        WebWindowRegistryAgeConnForTest(2005, kStaleAfterMsDefault + 1000);
        const WinQuery q2 = WebWindowRegistryQuery("fMotorTest");
        CHECK(q2.state == kWinOpen && q2.stale == true, "只有過期的 open -> open + stale（契約 §6 不變）");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true, "只有過期的 closed -> 照舊算開著（不可知往擋得住那側）");
    }

    // -----------------------------------------------------------------------
    printf("\n[6] ★★ stale：保留最後已知、標記 stale、照舊回答開著（契約 §6）\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistryPut(1001, Frame("20", "open", "closed"), why);
        CHECK(WebWindowRegistryStats().windowsTotal == 2, "前置：表裡有兩個表單");

        // 讓一切立刻算 stale（等同「瀏覽器斷線了」）
        WebWindowRegistrySetStaleMsForTest(-1);

        const WinRegistryStats st = WebWindowRegistryStats();
        CHECK(st.connections == 1,
              "⛔ 斷線後總表**沒有被清空**（清空＝當成全部關閉＝契約禁止）");
        CHECK(st.staleConns == 1, "該連線被標記 stale");
        CHECK(st.windowsTotal == 2, "最後已知的兩個表單還在");

        const WinQuery q = WebWindowRegistryQuery("fTeach");
        CHECK(q.stale == true, "查詢結果標著 stale");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "★★ stale 時 fTeach 照舊回 true（它本來就開著）");
        CHECK(WebWindowRegistryFShowConservative("fMotorTest") == true,
              "★★ stale 時連本來 closed 的也回 true —— "
              "因為 stale 代表『不可知』，不可知一律往擋得住的那側倒");

        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    }

    // -----------------------------------------------------------------------
    printf("\n[7] 壞訊框不可以洗掉好資料\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistryPut(1001, Frame("30", "open", "closed"), why);

        CHECK(!WebWindowRegistryPut(1001, "not json at all", why),
              "不是 JSON -> 拒收");
        CHECK(!WebWindowRegistryPut(1001, "{\"type\":\"something.else\"}", why),
              "type 不對 -> 拒收");
        CHECK(!WebWindowRegistryPut(1001, "{\"type\":\"ui.windows\"}", why),
              "缺 windows -> 拒收");

        CHECK(WebWindowRegistryStats().windowsTotal == 2,
              "★ 三個壞訊框之後，原本的好資料**原封不動**");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "★ 好資料還答得出原本的答案");
    }

    // =======================================================================
    //  P6-b 區塊 A：政策層
    //  AI(W906-P6b-A) 20260921
    //
    //  上面 [1]-[7] 驗的是**契約層**（Steven 說什麼就存什麼）。
    //  下面驗的是**政策層**：兩個使用者裁決 ＋ golden 的三層規則。
    //  兩層的答案在同一個輸入下會**不一樣**，而那正是重點 ——
    //  所以每一組都同時斷言 Conservative 與 Policy，讓分岔是可見的。
    // =======================================================================

    // -----------------------------------------------------------------------
    printf("\n[8] Q8-B：從來沒收過總表 -> 不擋（與契約層相反，而且是刻意的）\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);

        CHECK(WebWindowRegistryEverAnyFrame() == false,
              "前置：reset 之後「曾經收過」是 false");
        CHECK(WebWindowRegistryFShowConservative("fTeach") == true,
              "契約層：不可知 -> 開著（§6 沒變）");
        CHECK(WebWindowRegistryFShowPolicy("fTeach") == false,
              "★ 政策層：從沒收過 -> 關著（Q8-B）");

        const DiagVerdict v = WebWindowRegistryDiagnosticsOpen(false, 0, false);
        CHECK(v.open == false,
              "★ 沒有瀏覽器時 START **不會**被視窗閘擋住（Q8-B 的全部重點）");
        CHECK(v.tier == kDiagNone, "沒有任何一層命中");
        CHECK(v.everAnyFrame == false, "裁決值有回報出來");
    }

    // -----------------------------------------------------------------------
    printf("\n[9] Q8-B 的黏性：收過一次之後就回不去了\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
        CHECK(WebWindowRegistryPut(1001, Frame("40", "closed", "closed"), why),
              "前置：收一個合法訊框");
        CHECK(WebWindowRegistryEverAnyFrame() == true, "「曾經收過」點亮了");

        // 這一格是 Q8 條目裡標 ⛔ 的那一格：斷線之後**不可以**退回「從未收過」。
        WebWindowRegistrySetStaleMsForTest(-1);   // 一切立刻算 stale ＝ 模擬斷線
        CHECK(WebWindowRegistryEverAnyFrame() == true,
              "★ 全部 stale（＝瀏覽器斷線）之後仍然是「曾經收過」");
        CHECK(WebWindowRegistryFShowPolicy("fTeach") == true,
              "★ 斷線後回到保守側：當成還開著（不是回到 Q8-B 的不擋）");
        const DiagVerdict v = WebWindowRegistryDiagnosticsOpen(false, 0, false);
        CHECK(v.open == true && v.tier == kDiagAlways,
              "★ 斷線後 START 被擋住 —— 這是 Q8-B 與「斷線解除全部閘」的分界");
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
    }
    {
        // 壞訊框不可以點亮旗標：一個打錯的 JSON 不該把所有閘從「不擋」切到
        // 「保守擋」，而且切過去就回不來。
        WebWindowRegistryResetForTest();
        std::string why2;
        CHECK(!WebWindowRegistryPut(2001, "not json at all", why2), "前置：拒收");
        CHECK(WebWindowRegistryEverAnyFrame() == false,
              "★ 壞訊框**沒有**點亮「曾經收過」");
    }

    // -----------------------------------------------------------------------
    printf("\n[10] Q20-甲：瀏覽器不會回報的表單一律當成關著\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
        CHECK(WebWindowRegistryPut(1001, Frame("50", "closed", "closed"), why),
              "前置：收過總表（所以 Q8-B 那一關已經過了）");

        std::size_t n = 0;
        const char* const* never = WebWindowRegistryNeverReportedForms(n);
        CHECK(n == 5, "Q20 的清單是 5 個（量自 Steven 20260919 background.html）");
        bool allClosed = true;
        for (std::size_t i = 0; i < n; ++i) {
            if (WebWindowRegistryIsReportable(never[i])) allClosed = false;
            if (WebWindowRegistryFShowPolicy(never[i]))  allClosed = false;
        }
        CHECK(allClosed, "★ 清單上每一個：不可回報，且政策層回關著");
        CHECK(WebWindowRegistryFShowConservative("FrmRotate") == true,
              "★ 同一個輸入，契約層仍然回「開著」—— 兩層確實分岔了");

        CHECK(WebWindowRegistryIsReportable("fTeach") == true,
              "白名單沒有誤傷：fTeach 是會回報的");
        CHECK(WebWindowRegistryFShowPolicy("fSetup") == true,
              "★ 會回報但沒出現在總表裡的 -> 照契約保守，當成開著");
    }

    // -----------------------------------------------------------------------
    printf("\n[11] golden Command.cpp:7348-7360 的三層\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
        CHECK(WebWindowRegistryPut(1001, FrameAll("60", "closed", 0, 0), why),
              "前置：三層每一個表單都明說 closed");

        CHECK(WebWindowRegistryDiagnosticsOpen(false, 0, false).open == false,
              "前置：全關 ＋ 停機 -> 不擋");
        CHECK(WebWindowRegistryDiagnosticsOpen(true, 0, false).open == false,
              "前置：全關 ＋ 運轉 -> 不擋");
    }
    {
        // 第一層：無條件，與 SystemStart 無關。
        WebWindowRegistryResetForTest();
        CHECK(WebWindowRegistryPut(1001, FrameAll("61", "closed", "fShuttleMove", "open"), why),
              "前置：只有 fShuttleMove 開著");
        const DiagVerdict stopped = WebWindowRegistryDiagnosticsOpen(false, 0, false);
        const DiagVerdict running = WebWindowRegistryDiagnosticsOpen(true, 0, false);
        CHECK(stopped.open && stopped.tier == kDiagAlways && stopped.form == "fShuttleMove",
              "★ 第一層命中，且指名是哪一個表單");
        CHECK(running.open && running.tier == kDiagAlways,
              "★ 第一層在**運轉中**照樣命中（golden 的 || 鏈在 SystemStart 判斷之外）");
    }
    {
        // 第二層：fContact 且 contact mode 非 NORMAL。
        // ⚠ 必須用 SystemStart==true 才隔離得出來 —— fContact 在第三層也有，
        //   停機時會被第三層吃掉，看起來像第二層過了其實沒有。
        WebWindowRegistryResetForTest();
        CHECK(WebWindowRegistryPut(1001, FrameAll("62", "closed", "fContact", "open"), why),
              "前置：只有 fContact 開著");
        CHECK(WebWindowRegistryDiagnosticsOpen(true, 0, false).open == false,
              "★ 運轉中 ＋ contact mode == NORMAL -> 不算（golden 的 rbModeNormal 那一格）");
        const DiagVerdict diag = WebWindowRegistryDiagnosticsOpen(true, 1, false);
        CHECK(diag.open && diag.tier == kDiagContact && diag.form == "fContact",
              "★ 運轉中 ＋ contact mode 非 NORMAL -> 第二層命中");
        CHECK(WebWindowRegistryDiagnosticsOpen(false, 0, false).tier == kDiagStopped,
              "★ 停機時同一個 fContact 改由第三層命中（golden 兩層都列了它）");
    }
    {
        // 第三層：只有停機時才算。
        WebWindowRegistryResetForTest();
        CHECK(WebWindowRegistryPut(1001, FrameAll("63", "closed", "fSpeed", "open"), why),
              "前置：只有 fSpeed 開著");
        CHECK(WebWindowRegistryDiagnosticsOpen(true, 0, false).open == false,
              "★ 運轉中 -> 第三層整層不算");
        const DiagVerdict v = WebWindowRegistryDiagnosticsOpen(false, 0, false);
        CHECK(v.open && v.tier == kDiagStopped && v.form == "fSpeed",
              "★ 停機 -> 第三層命中");
    }
    {
        // minimized 也算開著（使用者裁示，契約 §3）—— 政策層不可以把它吃掉。
        // ⚠ 這裡用 fMotorTest 不用 fHome：fHome 在 [13] 有自己的條件，
        //   拿它來測 minimized 會把兩件事混在一起，紅的時候分不出是哪一件壞了。
        WebWindowRegistryResetForTest();
        CHECK(WebWindowRegistryPut(1001, FrameAll("64", "closed", "fMotorTest", "minimized"), why),
              "前置：fMotorTest 縮到工作列");
        CHECK(WebWindowRegistryDiagnosticsOpen(true, 0, false).tier == kDiagAlways,
              "★ minimized 照樣算「人還在用」");
    }

    // -----------------------------------------------------------------------
    printf("\n[12] fHome 降層（使用者 Q9 裁決乙的下游後果）\n");
    {
        WebWindowRegistryResetForTest();
        WebWindowRegistrySetStaleMsForTest(kStaleAfterMsDefault);
        CHECK(WebWindowRegistryPut(1001, FrameAll("70", "closed", "fHome", "open"), why),
              "前置：只有 fHome 開著（操作員從 Config 選單自己打開看進度）");

        // golden 這裡會回 true（無條件層）。我們刻意回 false ——
        // 理由是 golden 的無條件分類建立在「只有狀態機會叫出 fHome」，
        // 而移植樹保留了操作員入口（Q9 裁決乙）。
        const DiagVerdict watching = WebWindowRegistryDiagnosticsOpen(true, 0, false);
        CHECK(watching.open == false,
              "★ 沒在回原點、只是開著看 -> **不**算診斷中（刻意偏離 golden）");

        const DiagVerdict homing = WebWindowRegistryDiagnosticsOpen(true, 0, true);
        CHECK(homing.open && homing.tier == kDiagAlways && homing.form == "fHome",
              "★ 真的在回原點 -> 算診斷中（golden 原本要問的就是這件事）");

        // 降層只降 fHome 一個，不可以順手把整層弄軟。
        WebWindowRegistryResetForTest();
        CHECK(WebWindowRegistryPut(1001, FrameAll("71", "closed", "fTeach", "open"), why),
              "前置：改成 fTeach 開著");
        CHECK(WebWindowRegistryDiagnosticsOpen(true, 0, false).tier == kDiagAlways,
              "★ 同一層的 fTeach **沒有**被連帶降層");
    }

    // -----------------------------------------------------------------------
    printf("\n[13] 三層的形狀 —— 與 golden 逐項對帳\n");
    {
        std::size_t n1 = 0, n2 = 0, n3 = 0, nn = 0;
        const char* const* t1 = WebWindowRegistryTierForms(kDiagAlways,  n1);
        const char* const* t2 = WebWindowRegistryTierForms(kDiagContact, n2);
        const char* const* t3 = WebWindowRegistryTierForms(kDiagStopped, n3);
        WebWindowRegistryNeverReportedForms(nn);

        CHECK(n1 == 4,  "第一層 4 個（golden :7348）");
        CHECK(n2 == 1,  "第二層 1 個（golden :7349）");
        CHECK(n3 == 28, "第三層 28 個（golden :7351-7360）");
        CHECK(std::string(t1[0]) == "fTeach" && std::string(t1[3]) == "fHome",
              "第一層的頭尾與 golden 的 || 鏈同序（form 欄位才指得對）");
        CHECK(std::string(t2[0]) == "fContact", "第二層就是 fContact");
        CHECK(std::string(t3[0]) == "fSetup" && std::string(t3[27]) == "fiosetview",
              "第三層的頭尾與 golden 同序");

        // ★ Q20 的接縫寫成斷言：FrmRotate **在第三層裡**，而且**在不回報清單裡**。
        //   Steven 哪天把它補進 WINDOWS 表、我們忘了同步刪掉這一行時，
        //   這條斷言不會紅 —— 它擋不住那件事。但它讓「這兩件事是綁在一起的」
        //   在程式碼裡看得見，而 §0.2 的告知清單才是真正的守門員。
        bool inTier3 = false;
        for (std::size_t i = 0; i < n3; ++i)
            if (std::string(t3[i]) == "FrmRotate") inTier3 = true;
        CHECK(inTier3, "FrmRotate 確實在 golden 第三層裡");
        CHECK(WebWindowRegistryIsReportable("FrmRotate") == false,
              "★ 而它正是 Q20 要豁免的那一個（兩邊必須同時成立才有意義）");

        CHECK(WebWindowRegistryTierForms(kDiagNone, n1) == 0 && n1 == 0,
              "kDiagNone 問清單 -> 空的，不是越界");
    }

    printf("\n==== 結果: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
