// =============================================================================
//  WebWindowRegistry.h  --  視窗狀態總表（取代 golden 的 `fShow`）
//
//  AI(W906-P6-WINREG) 20260920
//
//  契約來源：Steven `WINDOW_REGISTRY_CONTRACT.md`（20260918），
//  交付包在 `U:\共用區\HT-9050\HT9045_V906_changes_20260919_fShow\`。
//  使用者 20260919 裁決（計畫 §0.7）：「他是負責網頁和網頁通訊 JSON 部分，
//  以後不要我來點頭，都執行」⇒ 直接做。
//
//  ## 為什麼需要這個東西
//
//  golden 在 `Command.cpp:7348-7360` 用 `fXxx->fShow` 判斷「有沒有人正在用
//  某個診斷畫面」，用來決定要不要放行 START。移植樹的 UI 是網頁，**沒有
//  `fShow` 這個東西**，所以那些閘今天全部是樁（ST 計畫 §5.2 的 10 個）。
//
//  這份總表就是 `fShow` 的替代品：瀏覽器**如實回報**哪些視窗開著，
//  C++ 拿它去套 golden 的分層規則做判斷。
//
//  ## ⚠ 分層是 C++ 的事，不是瀏覽器的事（契約 §9）
//
//  瀏覽器**只報事實**，不算「現在可不可以 START」。理由寫在契約裡而且是對的：
//  把安全判斷放在瀏覽器 = 放在最容易被繞過的一層（WS 直連完全繞得過它）。
//  ⇒ 本檔只提供「某個表單現在是什麼狀態」，**不做放行判斷**。
//     判斷留給 P6-b（把它接上 `fShow` 那些閘）。
//
//  ## ⚠⚠ 不可知／stale 的方向與 `guard.systemStart` **相反**
//
//      guard.systemStart   不可知 -> 當成 true（正在運轉）
//      視窗狀態總表        不可知 -> 當成「**那頁還開著**」
//
//  兩邊都往「比較擋得住」的那一側倒，但**值相反**。契約 §6 自己說這是
//  整份契約最容易寫反的地方。⇒ `FShowConservative()` 對 Unknown / stale
//  一律回 `true`。
//
//  ⛔ 斷線時**不可以歸零**。歸零 = C++ 以為沒人在教導 = 放行本該擋住的 START。
//
//  ## stale 怎麼認定（一個誠實的實作限制）
//
//  契約希望「瀏覽器斷線時標記 stale」。但 `WebBridgeServer` 今天**沒有**
//  對外暴露連線關閉事件或存活連線清單（量過：`WebBridgeServer.h` 只有
//  `ControlOwner()`）。
//  ⇒ 本實作用**年齡**認定：超過 `WebWindowRegistryStaleMs()` 沒有新訊框就算 stale。
//     這在行為上與真正的斷線偵測**等價**，因為 stale 與斷線的處理是同一個
//     （照舊回答「開著」）；差別只在精確度，不在安全方向。
//     契約 §6 也明文允許逾時，只要求「逾時的處理不可以是當成全部關閉」。
//  ⚠ 要更精確就得在 `WebBridgeServer` 開一個連線生命週期的介面 ——
//     那是共用檔，留給需要它的人做，不在這一波順手改。
// =============================================================================
#ifndef WebWindowRegistryH
#define WebWindowRegistryH

#include <string>
#include <vector>
#include <cstdint>

namespace ht9045 {

// 契約 §3 的四態。`never` 與 `closed` 對 fShow 都是 false，
// 但**刻意分開記** —— 查問題時「從來沒開過」與「開過又關掉」是不同的線索。
enum WinState {
    kWinNever = 0,   // 開站建好但從未開過
    kWinOpen,        // 顯示中                      -> fShow true
    kWinMinimized,   // 縮到工作列，人還在用        -> fShow true（使用者裁示）
    kWinClosed,      // 按了 ✕ 或頁面自己的 Exit
    kWinUnknown      // 這份總表裡沒有這個表單（不是「關著」）
};

struct WinQuery {
    WinState state;
    bool     stale;      // 來源訊框已經過期（或來自已斷線的連線）
    bool     fromAnyConn;// 有沒有任何一條連線報過這個表單
};

// 瀏覽器推上來的一個訊框（契約 §4）。value 是 JSON 字串。
// 回傳 false 代表這個訊框不合契約（解析失敗／缺必要欄位），
// 此時**不會**動既有的快取 —— 壞訊框不可以把好資料洗掉。
bool WebWindowRegistryPut(std::uint64_t connId, const std::string& frameJson,
                          std::string& whyNot);

// 查一個 golden 表單名（join key 用 `form`，**不是** background.html 的 id；
// 契約 §4 說得很清楚：id 全小寫而且是瀏覽器自己的鍵）。
WinQuery WebWindowRegistryQuery(const std::string& goldenForm);

// 契約 §6 的保守答案：Unknown 或 stale 一律 true（＝「還開著」）。
// ⚠ 這就是取代 `fShow` 時該用的那一個。
bool WebWindowRegistryFShowConservative(const std::string& goldenForm);

// 觀測用。
struct WinRegistryStats {
    std::size_t  connections;    // 有幾條連線報過總表（含已 stale 的）
    std::size_t  windowsTotal;   // 去重後的表單數
    std::size_t  openLike;       // state == open || minimized
    std::size_t  staleConns;
    std::int64_t lastSeq;        // 最後一個訊框的 seq；-1 = 還沒收過
    std::string  topmost;        // 最後一個非 stale 訊框的 topmost（可能是空字串）
};
WinRegistryStats WebWindowRegistryStats();

// 測試用：把整個快取清掉。⛔ 正常執行路徑**不要**呼叫它 ——
// 「清空總表」正是契約禁止的那個動作。
void WebWindowRegistryResetForTest();

// stale 門檻（毫秒）。契約把逾時留給 Jimmy 決定；預設取 15 秒，
// 理由：瀏覽器是「狀態有變才送」，所以沒有心跳；15 秒足夠涵蓋一次
// 正常的頁面切換，又不會讓一個真的斷線拖太久才被標記。
// ⚠ 改這個值**不會**改變安全方向（stale 仍然回答「開著」），只改精確度。
extern const std::int64_t kStaleAfterMsDefault;
std::int64_t WebWindowRegistryStaleMs();

// =============================================================================
//  P6-b 區塊 A：政策層 —— 把總表變成 golden `Command.cpp:7348-7360` 的那個判斷
//
//  AI(W906-P6b-A) 20260921
//
//  ## 這一層與上面那一層的分工
//
//  上面（P6-a）是**契約層**：瀏覽器說什麼就存什麼，查詢照契約 §6 保守回答。
//  這一層是**政策層**：套上使用者的兩個裁決，再照 golden 的三層規則算出
//  「現在算不算有診斷畫面開著」。
//
//  ⚠ 兩層刻意分開，因為它們的權威來源不同：
//     契約層改了要跟 Steven 對；政策層改了要跟使用者對。
//     混在一起會變成「改一個就得問兩個人」。
//
//  ## 兩個使用者裁決（不是契約，是覆蓋契約的）
//
//  **Q8 → B（20260920 13:05）**：分界是「**有沒有過**連線」，不是「現在有沒有」。
//    * 從來沒收過任何總表（開機、沒有瀏覽器）-> 視同沒有診斷畫面開著，**不擋**
//    * 收過但該表單沒出現在總表 -> 保守，當成開著
//    * 收過但 stale（斷線）      -> 保守，當成開著，**不歸零**
//    ⛔ 斷線之後**不可以**退回「從未收過」—— 退回去 = 斷線就解除所有閘。
//       所以 `WebWindowRegistryEverAnyFrame()` 是**黏的**（只有測試能清）。
//
//  **Q20 → 甲（20260921 08:0x）**：瀏覽器根本不會回報的表單，C++ 一律當成關著。
//    原話：「Q20 用甲，P10 也用甲，這些決定也需要讓 steven 可以知道」。
//    ⚠ 這**偏離契約 §6 的字面**（「沒說關著就當開著」）。偏離是刻意的：
//       那幾個表單不在瀏覽器的回報範圍內，保守擋住的話**沒有任何辦法解除** ——
//       使用者沒有那個視窗可以關。
//    ⚠⚠ 代價：Steven 之後把它們補進 `background.html` 的 WINDOWS 表時，
//        下面 `kNeverReportedForms` 必須**同步拿掉**，否則會變成
//        「瀏覽器說開著、C++ 說關著」的靜默分岔。已列入 Q5 要告知他的事。
// =============================================================================

// Q8-B 的那條分界：有沒有**任何**連線曾經送過一個合法的總表訊框。
// ⚠ 黏的：一旦為 true，除了 `WebWindowRegistryResetForTest()` 之外不會變回 false。
bool WebWindowRegistryEverAnyFrame();

// Q20-甲 的白名單（其實是黑名單）：這個 golden 表單名，瀏覽器會不會回報？
// false = 不會 -> 政策層一律回「關著」。
bool WebWindowRegistryIsReportable(const std::string& goldenForm);

// 套上 Q8-B 與 Q20-甲 之後的 `fShow`。**取代 `fShow` 時該用的是這一個**，
// 不是 `WebWindowRegistryFShowConservative()`（那是純契約，沒有裁決）。
bool WebWindowRegistryFShowPolicy(const std::string& goldenForm);

// golden `Command.cpp:7348-7360` 的三層。數值刻意從 1 起跳，0 留給「沒命中」。
enum DiagTier {
    kDiagNone    = 0,   // 沒有任何一層命中
    kDiagAlways  = 1,   // 無條件診斷：fTeach / fMotorTest / fShuttleMove / fHome
    kDiagContact = 2,   // 條件式    ：fContact 且 contact mode 非 NORMAL
    kDiagStopped = 3    // 停機才算  ：其餘 28 個，只有 SystemStart==false 時才算
};

struct DiagVerdict {
    bool        open;          // 等同 golden 的 `Bit4_HandlerDiagnostics`
    DiagTier    tier;          // 命中的那一層
    std::string form;          // 第一個命中的表單名（診斷用；順序照 golden）
    bool        everAnyFrame;  // Q8-B 的分界值，回報出來免得要另外查
};

// 算出 golden 那個 `if` 的值。
//   systemStart  -- 全域 `SystemStart`
//   contactMode  -- 全域 `iContactMode`（0 == CONTACT_NORMAL）。
//                   golden 寫的是 `fContact->rbModeNormal->Checked==false`，
//                   本樹沒有那顆 radio，改讀它背後的那個全域
//                   （對照 `WebBridgeTags.cpp:1153` 的 `guard.contactMode`）。
//   homingActive -- 機台**現在真的在回原點**嗎（建議傳 `fHome->iHomeStep != 1`）。
//                   ⚠ 這個參數是**刻意偏離 golden 的**，見下面那一段。
//
//  ## ⚠⚠ `fHome` 為什麼多一個條件（使用者 Q9 裁決的下游後果）
//
//  golden 把 `fHome` 放在**無條件**診斷層，而那個分類建立在一個前提上：
//  golden 全樹只有一個 `fHome->Show()` 呼叫點（`uhome.cpp:2497`，回原點狀態機
//  `case 20`）⇒ **Home Monitor 在畫面上就等於機台正在回原點**。
//
//  移植樹把那個前提打破了：網頁在 `main.html:86` 自己加了一個 Config 選單入口，
//  而使用者 20260920（INBOX Q9）裁決**保留它** ——
//  原話「照最小修改為主，不動它」＋「回 HOME 顯示畫面是為了讓使用者知道現在軸回
//  home 進度」。⇒ 操作員**被預期**會自己打開它看進度。
//
//  ⇒ 照搬 golden 的無條件分類，會把「操作員開著看進度」誤判成「機台在回原點」。
//    Steven 自己在契約 §8 就指出了這一格，並說「這會改變 C++ 解讀總表的方式」。
//
//  ★ 這仍然是**忠於 golden 的意圖**，不是「怕它擋住」：golden 要的是
//    「機台正在回原點時算診斷中」，它只是用一個在 golden 裡成立、在這裡不成立的
//    代理變數去表達。我們把代理變數換成它真正想問的那件事。
//
//  ⚠ 佐證訊號由**呼叫端**提供而不是本檔自己去讀 `fHome`：這個檔必須能只跟
//    cJSON 一起連結（`tests/CMakeLists.txt:2970` 的測試目標就是這樣連的）。
//    在這裡 `#include "forms/fHome.h"` 會把整個 VCL 相容層拖進來。
DiagVerdict WebWindowRegistryDiagnosticsOpen(bool systemStart, int contactMode,
                                             bool homingActive);

// 觀測與測試用：某一層的表單清單，順序**完全照 golden 的 `||` 鏈**。
// 回傳陣列長度寫進 count；tier 不合法時回 0 並把 count 設 0。
const char* const* WebWindowRegistryTierForms(DiagTier tier, std::size_t& count);

// 觀測與測試用：Q20-甲 的「瀏覽器不會回報」清單。
const char* const* WebWindowRegistryNeverReportedForms(std::size_t& count);

// -----------------------------------------------------------------------------
// 測試用：覆寫門檻。傳負值代表「一切立刻算 stale」。
// ⚠ 存在的理由很實際：不給覆寫的話，「stale 仍然回答開著」那一條要等 15 秒
//   才驗得到，而一支要跑 15 秒的測試最後一定會被改成不測那一格 ——
//   偏偏那一格是整份契約最容易寫反的地方（契約 §6 自己這樣說）。
void WebWindowRegistrySetStaleMsForTest(std::int64_t ms);

} // namespace ht9045

#endif // WebWindowRegistryH
