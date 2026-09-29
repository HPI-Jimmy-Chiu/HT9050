// =============================================================================
//  WebWindowRegistry.cpp  --  視窗狀態總表（實作）
//
//  AI(W906-P6-WINREG) 20260920
//  設計理由與安全方向寫在 WebWindowRegistry.h 的檔頭，這裡不重複。
// =============================================================================
#include "WebWindowRegistry.h"

#include "Public/cJSON.h"

#include <map>
#include <ctime>

namespace ht9045 {

const std::int64_t kStaleAfterMsDefault = 15000;

namespace {
std::int64_t g_staleMs = kStaleAfterMsDefault;

//AI(W906-P6b-A) 20260921: Q8-B 的黏性旗標。宣告放在這裡（而不是政策層那一段）
// 只有一個理由：`WebWindowRegistryPut()` 在檔案前半，要看得到它。
// ⛔ 只有 `WebWindowRegistryResetForTest()` 能把它清掉。用 `Store().empty()`
//    代替它會讓「斷線後不可退回未知」依賴「Store 永遠不刪 entry」這個實作
//    細節；哪天加了連線清理就會靜默失效，而失效的方向正好是危險的那一側。
bool g_everAnyFrame = false;
}

std::int64_t WebWindowRegistryStaleMs() { return g_staleMs; }

void WebWindowRegistrySetStaleMsForTest(std::int64_t ms) { g_staleMs = ms; }

namespace {

struct WinEntry {
    WinState    state;
    std::string id;          // background.html 的視窗 id（診斷用，不是 join key）
    bool        fullscreen;
};

struct ConnFrame {
    std::int64_t                      seq;
    std::string                       at;
    std::string                       topmost;
    std::vector<std::string>          modalStack;
    std::map<std::string, WinEntry>   byForm;   // join key = golden 表單名
    std::int64_t                      recvMs;
};

// connId -> 最後一個好訊框。
// ⛔ 這個 map **只增不減**。斷線不刪 —— 契約 §6：保留最後已知並標記 stale，
//    不可歸零。刪掉它等於「當成全部關閉」，那正是被禁止的那個行為。
std::map<std::uint64_t, ConnFrame>& Store()
{
    static std::map<std::uint64_t, ConnFrame> s;
    return s;
}

std::int64_t NowMs()
{
    return (std::int64_t)std::time(0) * 1000;
}

WinState ParseState(const char* s)
{
    if (s == 0)                     return kWinUnknown;
    if (std::string(s) == "open")      return kWinOpen;
    if (std::string(s) == "minimized") return kWinMinimized;
    if (std::string(s) == "closed")    return kWinClosed;
    if (std::string(s) == "never")     return kWinNever;
    return kWinUnknown;
}

bool IsOpenLike(WinState s)
{
    // ★ minimized 算「開著」—— 使用者裁示（契約 §3）。
    //   「有人正在用這個畫面」≠「像素有沒有畫出來」。
    //   算成關閉會讓 START 在不該放行的時候放行。
    return s == kWinOpen || s == kWinMinimized;
}

} // namespace

// -----------------------------------------------------------------------------
bool WebWindowRegistryPut(std::uint64_t connId, const std::string& frameJson,
                          std::string& whyNot)
{
    cJSON* root = cJSON_Parse(frameJson.c_str());
    if (root == 0) {
        whyNot = "frame is not valid JSON";
        return false;
    }

    ConnFrame f;
    f.seq    = -1;
    f.recvMs = NowMs();

    const cJSON* jtype = cJSON_GetObjectItemCaseSensitive(root, "type");
    if (!cJSON_IsString(jtype) || std::string(jtype->valuestring) != "ui.windows") {
        // 契約 §4 的訊框第一個欄位就是 type。不對就不是這個東西。
        whyNot = "frame.type is not \"ui.windows\"";
        cJSON_Delete(root);
        return false;
    }

    const cJSON* jwins = cJSON_GetObjectItemCaseSensitive(root, "windows");
    if (!cJSON_IsObject(jwins)) {
        whyNot = "frame.windows missing or not an object";
        cJSON_Delete(root);
        return false;
    }

    const cJSON* jseq = cJSON_GetObjectItemCaseSensitive(root, "seq");
    if (cJSON_IsNumber(jseq)) f.seq = (std::int64_t)jseq->valuedouble;

    const cJSON* jat = cJSON_GetObjectItemCaseSensitive(root, "at");
    if (cJSON_IsString(jat)) f.at = jat->valuestring;

    // topmost 可以是 null（沒有任何視窗開著），那是合法的，不是缺漏。
    const cJSON* jtop = cJSON_GetObjectItemCaseSensitive(root, "topmost");
    if (cJSON_IsString(jtop)) f.topmost = jtop->valuestring;

    const cJSON* jstack = cJSON_GetObjectItemCaseSensitive(root, "modalStack");
    if (cJSON_IsArray(jstack)) {
        const cJSON* it = 0;
        cJSON_ArrayForEach(it, jstack)
            if (cJSON_IsString(it)) f.modalStack.push_back(it->valuestring);
    }

    // windows 的鍵是 background.html 的 id，但 **join key 是裡面的 form**
    // （契約 §4：「C++ 端請用 form 當 join key，不要用 id」——
    //  id 全小寫而且是瀏覽器自己的鍵，改它會打斷既有入口）。
    const cJSON* w = 0;
    cJSON_ArrayForEach(w, jwins) {
        if (!cJSON_IsObject(w) || w->string == 0) continue;

        const cJSON* jform = cJSON_GetObjectItemCaseSensitive(w, "form");
        // form == null 是**明確的事實**，不是「不知道」：代表這個視窗在
        // golden 沒有對應的 TForm（main.dfm 的頁籤、或網頁自己的工具頁）。
        // ⇒ 跳過它，而且**不要**把它當成缺漏去補（契約 §4 事實 5）。
        if (!cJSON_IsString(jform)) continue;

        WinEntry e;
        e.id         = w->string;
        e.fullscreen = false;
        const cJSON* jfs = cJSON_GetObjectItemCaseSensitive(w, "fullscreen");
        if (cJSON_IsBool(jfs)) e.fullscreen = cJSON_IsTrue(jfs) ? true : false;

        const cJSON* jst = cJSON_GetObjectItemCaseSensitive(w, "state");
        e.state = ParseState(cJSON_IsString(jst) ? jst->valuestring : 0);

        f.byForm[jform->valuestring] = e;
    }

    cJSON_Delete(root);

    // ★ 以**連線為單位**保存，不是覆蓋同一份。
    //   兩個分頁會各送各的總表；覆蓋的話後開的會洗掉先開的，
    //   於是先開的那頁「消失」了 —— C++ 以為沒人在教導，放行 START。
    //   （契約 §9-2 / INBOX Q6 事實 3。）
    Store()[connId] = f;
    //AI(W906-P6b-A) 20260921: Q8-B 的分界在這一行變過去，而且**只在這裡**。
    // 放在 `Store()[connId] = f;` 之後而不是函式開頭：壞訊框（上面每一個
    // `return false`）不可以把「曾經收過」點亮 —— 那會讓一個打錯的 JSON
    // 就把所有閘從「不擋」切到「保守擋」，而且沒有辦法再退回去。
    g_everAnyFrame = true;
    return true;
}

// -----------------------------------------------------------------------------
WinQuery WebWindowRegistryQuery(const std::string& goldenForm)
{
    WinQuery q;
    q.state       = kWinUnknown;
    q.stale       = false;
    q.fromAnyConn = false;

    const std::int64_t now = NowMs();
    //AI(W906-MT-FIX1) 20260926: 新鮮的回報蓋過過期的回報（審查 medium：「總表永遠忘不掉死掉那條連線的 open」）。
    //   原本任何一條連線說 open 就是 open —— 連過期的也算，而且新鮮連線明確說 closed 也蓋不掉。HMI 重新整理或 WS 重連時
    //   Motor Test／Teach 開著，舊連線最後一個訊框（open）永遠留在 Store()（只增不減，契約 §6），新連線每 5 秒送 closed，
    //   結果 fMotorTest 永遠讀成 open；MT-E3b 起 golden MainProc 在 fMotorTest／fTeach 開著時暫停 ⇒ 生產永遠停住。
    //   現在：這個表單只要有任何一條**新鮮**的回報，就只看新鮮的回報（新鮮的之間照舊取聯集 —— 兩個分頁各開各的，
    //   任何一頁說 open 就是 open）；**全部都過期**時才照契約 §6 往「開著」那側倒（下面 stale 分支，行為不變）。
    //   為什麼這樣仍然安全：過期的那條連線 15 秒沒送任何訊框（background.html 每 5 秒一次心跳）——那一頁已經不在了，
    //   新鮮的那條才是現在畫面上的事實。
    bool freshSeen = false, freshOpen = false, staleSeen = false, staleOpen = false;
    WinState freshOpenState = kWinMinimized, freshOther = kWinUnknown;

    for (std::map<std::uint64_t, ConnFrame>::const_iterator it = Store().begin();
         it != Store().end(); ++it)
    {
        std::map<std::string, WinEntry>::const_iterator w =
            it->second.byForm.find(goldenForm);
        if (w == it->second.byForm.end()) continue;

        q.fromAnyConn = true;
        const bool entryStale = (now - it->second.recvMs) > g_staleMs;

        if (entryStale) {
            staleSeen = true;
            if (IsOpenLike(w->second.state)) staleOpen = true;
        } else {
            freshSeen = true;
            if (IsOpenLike(w->second.state)) {
                // ★ 任何一條新鮮連線說它開著，就是開著（聯集）。open 蓋過 minimized。
                freshOpen = true;
                if (w->second.state == kWinOpen) freshOpenState = kWinOpen;
            } else if (freshOther == kWinUnknown) {
                freshOther = w->second.state;
            }
        }
    }

    if (freshSeen) {
        q.state = freshOpen ? freshOpenState : freshOther;
    } else if (staleSeen) {
        // 只有過期的回報 —— 契約 §6：保留最後已知並標記 stale；說過 open 的照舊算開著。
        if (staleOpen) q.state = kWinOpen;
        q.stale = true;
    }
    return q;
}

// -----------------------------------------------------------------------------
bool WebWindowRegistryFShowConservative(const std::string& goldenForm)
{
    const WinQuery q = WebWindowRegistryQuery(goldenForm);

    // ⚠⚠ 這三行就是契約 §6 那一格，也是整份契約最容易寫反的地方。
    //    方向與 guard.systemStart **相反**：
    //      不可知 / stale  ->  當成「那頁還開著」-> true
    //    寫反的後果是 C++ 以為沒人在教導，於是放行本該擋住的 START。
    if (!q.fromAnyConn) return true;   // 從來沒人報過這個表單 = 不可知
    if (q.stale)        return true;   // 報過但過期了
    return IsOpenLike(q.state);
}

// -----------------------------------------------------------------------------
WinRegistryStats WebWindowRegistryStats()
{
    WinRegistryStats s;
    s.connections  = Store().size();
    s.windowsTotal = 0;
    s.openLike     = 0;
    s.staleConns   = 0;
    s.lastSeq      = -1;

    const std::int64_t now = NowMs();
    std::map<std::string, WinState> merged;
    std::int64_t freshestMs = -1;

    for (std::map<std::uint64_t, ConnFrame>::const_iterator it = Store().begin();
         it != Store().end(); ++it)
    {
        const bool entryStale = (now - it->second.recvMs) > g_staleMs;
        if (entryStale) ++s.staleConns;
        if (it->second.seq > s.lastSeq) s.lastSeq = it->second.seq;
        if (!entryStale && it->second.recvMs > freshestMs) {
            freshestMs = it->second.recvMs;
            s.topmost  = it->second.topmost;
        }
        for (std::map<std::string, WinEntry>::const_iterator w = it->second.byForm.begin();
             w != it->second.byForm.end(); ++w)
        {
            std::map<std::string, WinState>::iterator m = merged.find(w->first);
            if (m == merged.end() || (!IsOpenLike(m->second) && IsOpenLike(w->second.state)))
                merged[w->first] = w->second.state;
        }
    }

    s.windowsTotal = merged.size();
    for (std::map<std::string, WinState>::const_iterator m = merged.begin();
         m != merged.end(); ++m)
        if (IsOpenLike(m->second)) ++s.openLike;

    return s;
}

// =============================================================================
//  P6-b 區塊 A：政策層
//  AI(W906-P6b-A) 20260921
//  設計理由與兩個使用者裁決寫在 WebWindowRegistry.h，這裡不重複。
// =============================================================================
namespace {

// golden `Command.cpp:7348` 第一層：無條件診斷，一開就算。
// 順序完全照 golden 的 `||` 鏈 —— `DiagVerdict::form` 要與 golden 短路到的
// 那一個相同，否則查問題時兩邊會指到不同的表單。
const char* const kTierAlways[] = {
    "fTeach", "fMotorTest", "fShuttleMove", "fHome"
};

// golden `Command.cpp:7349` 第二層：`fContact->fShow && rbModeNormal->Checked==false`。
const char* const kTierContact[] = {
    "fContact"
};

// golden `Command.cpp:7351-7360` 第三層：只有 `SystemStart==false` 時才算。
// ⚠ 其中 5 個 golden 讀的是 `bShow` 不是 `fShow`
//    （fCCLink / fBarCode / fBinSel / fLtcSensor / fOmron）——
//    那是那幾支表單自己的成員名，語意同樣是「這張表單正顯示著」，
//    在總表這一側沒有區別（瀏覽器只回報狀態，不回報成員叫什麼）。
// ⚠ `fContact` 在第二層與第三層**都出現**，golden 亦然，不是抄重複。
const char* const kTierStopped[] = {
    "fSetup",      "fOffSet",       "fConfiguration",
    "fSpeed",      "fDIOFrom",      "fYieldMonitoring",
    "fTrayForm",   "fHotPlate",     "fTrayAssignment",
    "fTemp_Set",   "FTestIF",       "fCounterClear",
    "fLd_ULd",     "fCCLink",       "fTowerLight",
    "fCleaning",   "fQAMode",       "fCounterSel",
    "FrmRotate",   "fBuilder",      "fStartCondition",
    "fBarCode",    "fSecurity",     "fBinSel",
    "fLtcSensor",  "fOmron",        "fContact",
    "fiosetview"
};

// Q20-甲：瀏覽器**不會**回報的表單。20260921 對 Steven 20260919 交付包裡的
// `code/background.html` 逐項量出來的，不是抄 Q20 條目的敘述：
//
//   量法：`grep -o "form:'[A-Za-z_0-9]*'"` 得到它宣告的 44 個 golden 表單名，
//        再與上面三層的 33 個（去重後 32 個）取差集。
//
//   FrmRotate      -- 差集只有這一個。它在 golden 第三層裡，但 WINDOWS 表
//                     根本沒有對應的視窗。
//   HandlerSystem  -- 在 WINDOWS 表裡**有** form 名，但帶 `debugOnly:true`，
//                     而 `background.html:817` 在 release 模式直接 return，
//                     視窗沒建立就不會進 `WIN_STATE`，於是也不會進訊框
//                     （`buildRegistry()`: `if(!(cfg.id in WIN_STATE)) return;`）。
//                     ⇒ 對 C++ 而言與「沒宣告」等價。不在 Bit4 規則裡，
//                        但 B 區塊接 ST §5.2 的閘時會查到它。
//   Zteach / fTrayMapping / TrayEditForm
//                  -- Q20 原始條目列的另外三個。同樣不在 WINDOWS 表裡。
//                     它們不在 Bit4 規則裡，列在這裡是為了 B 區塊。
//
// ⚠⚠ Steven 把任何一個補進 WINDOWS 表時，這裡要同步刪掉那一行。
const char* const kNeverReportedForms[] = {
    "FrmRotate", "HandlerSystem", "Zteach", "fTrayMapping"   //AI(W906-PAGETAB-S10) 20260929: TrayEditForm 已補進 WINDOWS 表（St02 S-10 trayedit），照上面 ⚠⚠ 同步刪掉
};

// ⓘ Q8-B 的黏性旗標 `g_everAnyFrame` 宣告在檔頭那個匿名 namespace 裡
//   （`WebWindowRegistryPut()` 要看得到它），不在這裡。

bool InList(const char* const* list, std::size_t n, const std::string& form)
{
    for (std::size_t i = 0; i < n; ++i)
        if (form == list[i]) return true;
    return false;
}

} // namespace

// -----------------------------------------------------------------------------
bool WebWindowRegistryEverAnyFrame() { return g_everAnyFrame; }

// -----------------------------------------------------------------------------
bool WebWindowRegistryIsReportable(const std::string& goldenForm)
{
    return !InList(kNeverReportedForms,
                   sizeof(kNeverReportedForms) / sizeof(kNeverReportedForms[0]),
                   goldenForm);
}

// -----------------------------------------------------------------------------
bool WebWindowRegistryFShowPolicy(const std::string& goldenForm)
{
    // Q8-B：從來沒有任何瀏覽器報過總表 -> 視同沒有診斷畫面開著。
    // ★ 這一關要放在最前面：它講的是「我們對整個世界一無所知」，
    //   而不是「我們對這個表單一無所知」。後者才是契約 §6 那一格。
    if (!g_everAnyFrame) return false;

    // Q20-甲：瀏覽器不會回報的表單 -> 一律關著。
    if (!WebWindowRegistryIsReportable(goldenForm)) return false;

    // 其餘照契約 §6 保守回答（不可知 / stale -> 開著）。
    return WebWindowRegistryFShowConservative(goldenForm);
}

// -----------------------------------------------------------------------------
DiagVerdict WebWindowRegistryDiagnosticsOpen(bool systemStart, int contactMode,
                                             bool homingActive)
{
    DiagVerdict v;
    v.open         = false;
    v.tier         = kDiagNone;
    v.everAnyFrame = g_everAnyFrame;

    // --- 第一層：無條件（golden :7348）---------------------------------------
    for (std::size_t i = 0; i < sizeof(kTierAlways) / sizeof(kTierAlways[0]); ++i) {
        // `fHome` 是這一層唯一帶條件的：golden 的無條件分類建立在
        // 「只有回原點狀態機會叫出它」這個前提上，而移植樹保留了操作員入口
        //（使用者 Q9 裁決乙）⇒ 前提不成立。理由與 Steven 契約 §8 的對照
        // 寫在 WebWindowRegistry.h 的參數說明，這裡不重複。
        if (!homingActive && std::string(kTierAlways[i]) == "fHome") continue;
        if (WebWindowRegistryFShowPolicy(kTierAlways[i])) {
            v.open = true;
            v.tier = kDiagAlways;
            v.form = kTierAlways[i];
            return v;
        }
    }

    // --- 第二層：fContact 且 contact mode 非 NORMAL（golden :7349）------------
    // golden: `fContact->fShow && fContact->rbModeNormal->Checked==false`
    // 本樹沒有那顆 radio，讀它背後的全域 `iContactMode`（0 == CONTACT_NORMAL）。
    // ⓘ `iContactMode` 在這支 binary 裡今天恆為 0（唯一的賦值點
    //   `csystem.cpp:1583` 落在 `#if 0` 的 golden 逐字副本裡，量法見
    //   `WebBridgeTags.cpp:1153` 的註解）⇒ 這一層今天不會命中。
    //   那是**真的**不是「還沒接上」：能把機台切進診斷 contact 模式的碼
    //   沒有被編進來。Contact 那一波落地後這一層會自動活過來，不必回來改。
    if (contactMode != 0) {
        for (std::size_t i = 0; i < sizeof(kTierContact) / sizeof(kTierContact[0]); ++i) {
            if (WebWindowRegistryFShowPolicy(kTierContact[i])) {
                v.open = true;
                v.tier = kDiagContact;
                v.form = kTierContact[i];
                return v;
            }
        }
    }

    // --- 第三層：只有停機時才算（golden :7350-7360）---------------------------
    if (!systemStart) {
        for (std::size_t i = 0; i < sizeof(kTierStopped) / sizeof(kTierStopped[0]); ++i) {
            if (WebWindowRegistryFShowPolicy(kTierStopped[i])) {
                v.open = true;
                v.tier = kDiagStopped;
                v.form = kTierStopped[i];
                return v;
            }
        }
    }

    return v;
}

// -----------------------------------------------------------------------------
const char* const* WebWindowRegistryTierForms(DiagTier tier, std::size_t& count)
{
    switch (tier) {
    case kDiagAlways:
        count = sizeof(kTierAlways) / sizeof(kTierAlways[0]);
        return kTierAlways;
    case kDiagContact:
        count = sizeof(kTierContact) / sizeof(kTierContact[0]);
        return kTierContact;
    case kDiagStopped:
        count = sizeof(kTierStopped) / sizeof(kTierStopped[0]);
        return kTierStopped;
    default:
        count = 0;
        return 0;
    }
}

// -----------------------------------------------------------------------------
const char* const* WebWindowRegistryNeverReportedForms(std::size_t& count)
{
    count = sizeof(kNeverReportedForms) / sizeof(kNeverReportedForms[0]);
    return kNeverReportedForms;
}

// -----------------------------------------------------------------------------
void WebWindowRegistryResetForTest()
{
    Store().clear();
    // ⛔ 只有這裡能把 Q8-B 的黏性旗標清掉。正常執行路徑清它 =
    //    「斷線就退回從未收過」= 契約 §6 明文禁止的那件事。
    g_everAnyFrame = false;
}

//AI(W906-MT-FIX1) 20260926
void WebWindowRegistryAgeConnForTest(std::uint64_t connId, std::int64_t ageMs)
{
    std::map<std::uint64_t, ConnFrame>::iterator it = Store().find(connId);
    if (it != Store().end()) it->second.recvMs -= ageMs;
}

} // namespace ht9045
