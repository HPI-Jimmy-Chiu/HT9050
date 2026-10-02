// =============================================================================
//  WebPageTable.cpp  --  頁面表（實作）
//
//  //AI(W906-PAGETAB-Q51) 20260928 [W906] St01（Steven 團隊）。裁決、分工與邊界寫在 WebPageTable.h 檔頭，這裡不重複。
//
//  ## 寬限 10 秒（kPageNoScreenGraceMs）怎麼來的 —— Q-P1 (2)「瀏覽器全關 ⇒ 正常 STOP」前面那一段
//
//  要擋的只有一件事：F5 重新整理不能停機。F5 的順序（D:\HT9045\web\background.html）：
//    pagehide 送最後一份「全部關」（H2）⇒ C++ 立刻看到畫面不見
//    → 新頁面載入、連上 WebSocket 就送一份全量總表（:1067-1068 connect().then(pushRegistry)，
//      :1076-1081 每秒檢查「剛連上」也送）⇒ 畫面回來
//    → 之後連著時每 5 秒心跳一次（:1088-1092）。
//  ⇒ 寬限要蓋住「一次重新載入＋最多漏掉一次心跳」：10 秒 ＝ 心跳 5 秒 × 2。
//    本機 localhost 的重新載入通常 1～3 秒；這個外框開站會同時建 60 多個同源 iframe，主執行緒可能忙好幾秒，
//    所以不取 5 秒（剛好一次心跳，太貼）。
//  ⇒ 同時刻意小於總表的過期門檻 15 秒（WebWindowRegistry.cpp kStaleAfterMsDefault）：
//    「全部關」送到的那一種，在過期規則介入之前就已經決定停或不停。
//  代價：瀏覽器真的全關時，機台最多多跑 10 秒才停；瀏覽器當掉沒送「全部關」時，要等 WebSocket 斷
//    （程序死掉＝立刻；網路斷＝WebBridgeServer 最多 45 秒）再加 10 秒。
//  ⚠ 馬達的 jog／LoopMove 不等這 10 秒：網頁馬達工作本來就有死人開關 —— 操作員連線（控制權杖）一消失，
//    下一拍 jog 立刻 StopDec、HOME／LoopMove 取消（WebMotorAccess.cpp:3759-3783 MotorAccessTick，
//    tools/wb_serve.cpp:4598 傳 server.ControlOwner()!=0）。F5 也會觸發它（既有行為，不是本檔加的）。
//    本檔在寬限到期時另外補一次 motor.stop（golden btnStopClick），給「畫面不見那一拍還有 HOME 工作」的情況。
//  停下來之後出一則 golden 告警 MES16441（R143 改號，原 MES1690；ShowErrorMessage kcode 0，Jimmy 20260928 E#36＝C），畫面回來就看得到為什麼停
//    （選碼理由在 WebPageTable.h kPageNoScreenAlarmCode 上方）。
//
//  ## 為什麼 F5 之後不用再等 15 秒
//
//  pagehide 的「全部關」是一份**新鮮**的回報 ⇒ 規則 4 立刻生效（Teach 開著按 F5 ⇒ 馬上算關，S122 馬上清）。
//  沒送到（瀏覽器當掉）⇒ 舊回報 15 秒後過期，退到規則 6／7。
// =============================================================================
#include "WebPageTable.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <set>
#include <string>
#include <vector>

namespace ht9045 {

const std::int64_t kPageNoScreenGraceMs = 10000;
const std::int64_t kPageNoScreenAlarmWaitMs = 3000;
const char* const  kPageNoScreenAlarmCode = "MES16441";   //AI(W906-PAGETAB-R143) 20260929: R143：Jimmy 20260929 改號 MES16441（原 MES1690 跟既有 WAR1690 撞號，golden 事件資料庫用數字當編號會混在一起；RULINGS_20260929 第 2 節）

namespace {

// -----------------------------------------------------------------------------
//  90 列。前 69 列 ＝ D:\HT9045\web\background.html 的 WINDOWS 表（1910e7ca，:418-520），**順序相同**
//  （ctest T2 逐列比）。後 22 列 ＝ golden 會讀、網頁沒有視窗的表單（設計 §3.1）。
//  kPgBoth 的依據（golden V912 自己開／關的畫面，設計 §5.1）：
//    fHome     uhome.cpp Show／Close（移植樹 uhome.cpp:646／:652 已接程式寫入）
//    fSpeed    csystem.cpp fSpeed->Close()、RPDefault.cpp Show／Close
//    fCleaning RPDefault.cpp Show／Close
//    fSCKART   Command.cpp fSCKART->Show()、Automation\SCK_ART.cpp Show／Close
//    fOffSet   main.cpp:26043-26044 閒置自動登出 fOffSet->Close()
//    fCCLink   note.cpp fCCLink->Show()
//  除了 fHome，其餘的程式寫入等那段 golden 翻進來時照 uhome.cpp:646 的慣例接（今天只當網頁列用）。
// -----------------------------------------------------------------------------
const PageRowDef kRows[] = {
    // ---- 網頁視窗 69 列（WINDOWS 表順序；AI(W906-PAGETAB-S10) 20260929 最後一列 trayedit）----
    { "fMain",            "main",           kPgWeb,     false },
    { "fSortCT",          "sortct",         kPgWeb,     false },
    { "fContactCT",       "contactct",      kPgWeb,     false },
    { "fLotInfo",         "lotinfo",        kPgWeb,     false },
    { "fTemperFrom",      "temperf",        kPgWeb,     false },
    { "fObserver",        "observer",       kPgWeb,     false },
    { "fShowMessage",     "showmsg",        kPgWeb,     false },
    { "fTestCategory",    "testcat",        kPgWeb,     false },
    { "fShowBinSelect",   "binselect",      kPgWeb,     false },
    { "fOffSet",          "offset",         kPgBoth,    false },
    { "fSpeed",           "speed",          kPgBoth,    false },
    { "fiosetview",       "io",             kPgWeb,     false },
    { "fConfiguration",   "config",         kPgWeb,     false },
    { "fCounterSel",      "countersel",     kPgWeb,     false },
    { "fCounterClear",    "counterclear",   kPgWeb,     false },
    { "fBuilder",         "builder",        kPgWeb,     false },
    { "fDIOFrom",         "dioform",        kPgWeb,     false },
    { "fLtcSensor",       "ltcsensor",      kPgWeb,     false },
    { "fTowerLight",      "towerlight",     kPgWeb,     false },
    { "fOmron",           "omron",          kPgWeb,     false },
    { "fQAMode",          "qamode",         kPgWeb,     false },
    { "fBarCode",         "barcode",        kPgWeb,     false },
    { "fCCLink",          "cclink",         kPgBoth,    false },
    { "fCleaning",        "cleaning",       kPgBoth,    false },
    { "fContact",         "contact",        kPgWeb,     false },
    { "FTestIF",          "testerif",       kPgWeb,     false },
    { "fGroundMan",       "groundman",      kPgWeb,     false },
    { "fLd_ULd",          "ldud",           kPgWeb,     false },
    { "fSecurity",        "security",       kPgWeb,     false },
    { "fTrayForm",        "trayform",       kPgWeb,     false },
    { "fSCKART",          "sckart",         kPgBoth,    false },
    { "fYieldMonitoring", "yieldmon",       kPgWeb,     false },
    { "fHotPlate",        "hotplate",       kPgWeb,     false },
    { "fSetup",           "setup",          kPgWeb,     false },
    { "fSmartDiagnostic", "smartdiag",      kPgWeb,     false },
    { "fStartCondition",  "startcond",      kPgWeb,     false },
    { "fTemp_Set",        "tempset",        kPgWeb,     false },
    { "fBinSel",          "binsel",         kPgWeb,     false },
    { "fTeach",           "teach",          kPgWeb,     false },
    { "fMotorTest",       "motortest",      kPgWeb,     false },
    { "fHome",            "home",           kPgBoth,    false },
    { "fShuttleMove",     "shuttlemove",    kPgWeb,     false },
    { "fTrayAssignment",  "trayassign",     kPgWeb,     false },
    { "",                 "pci1203",        kPgWeb,     false },
    { "fContactForce",    "contactforce",   kPgWeb,     false },
    { "fVacuumUnit",      "vacuumunit",     kPgWeb,     false },
    { "fAGV",             "agv",            kPgWeb,     false },
    { "HandlerSystem",    "handlersys",     kPgWeb,     true  },
    { "",                 "motorview",      kPgWeb,     false },
    { "",                 "ctrlbtn",        kPgWeb,     false },
    { "",                 "motionview",     kPgWeb,     false },
    { "",                 "logs",           kPgWeb,     false },
    { "",                 "shuttlesensor",  kPgWeb,     false },
    { "",                 "tasklist",       kPgWeb,     false },
    { "",                 "commview",       kPgWeb,     false },
    { "",                 "heaterview",     kPgWeb,     false },
    { "",                 "record",         kPgWeb,     false },
    { "",                 "unloaderinfo",   kPgWeb,     false },
    { "",                 "aoainfo",        kPgWeb,     false },
    { "",                 "eventlog",       kPgWeb,     false },
    { "",                 "testercomm",     kPgWeb,     false },
    { "",                 "styleguide",     kPgWeb,     false },
    { "",                 "i18neditor",     kPgWeb,     false },
    { "",                 "widgets",        kPgWeb,     false },
    { "",                 "compmap",        kPgWeb,     false },
    { "",                 "definejson",     kPgWeb,     false },
    { "",                 "jsoncommandlog", kPgWeb,     true  },
    { "",                 "shots",          kPgWeb,     false },
    { "TrayEditForm",     "trayedit",       kPgBoth,    false },   //AI(W906-PAGETAB-S10) 20260929: St02 S-10 Tray Edit（golden uTrayEditForm）：操作員在主畫面盤子圖右鍵開、警報回復時程式也會開（golden asendic_Auto.cpp:2833、asendic_Loader.cpp:1514）⇒ both；原本是沒有網頁的列
    // ---- C++ 自己開的對話框 10 列（opener＝程式；成員由開框的程式自己設，例 tools/wb_serve.cpp:7597）----
    { "fNote",            "cpp:fNote",            kPgProgram, false },
    { "MyMessageBox",     "cpp:MyMessageBox",     kPgProgram, false },
    { "fPassword",        "cpp:fPassword",        kPgProgram, false },
    { "fPassword2",       "cpp:fPassword2",       kPgProgram, false },
    { "fInput",           "cpp:fInput",           kPgProgram, false },
    { "fQwertyKey",       "cpp:fQwertyKey",       kPgProgram, false },
    { "fQwertyKey2",      "cpp:fQwertyKey2",      kPgProgram, false },
    { "fShowBinSet",      "cpp:fShowBinSet",      kPgProgram, false },
    { "fDefrostNote",     "cpp:fDefrostNote",     kPgProgram, false },
    { "MemoryAlarmForm",  "cpp:MemoryAlarmForm",  kPgProgram, false },
    // ---- 沒有網頁 11 列（AI(W906-PAGETAB-S10) 20260929 TrayEditForm 搬到網頁段；永遠關；Q20-甲那 5 個裡的 4 個在這裡，HandlerSystem 是上面的 debug 專用列）----
    { "Zteach",           "none:Zteach",          kPgNoWeb,   false },
    { "FrmRotate",        "none:FrmRotate",       kPgNoWeb,   false },
    { "fTrayMapping",     "none:fTrayMapping",    kPgNoWeb,   false },
    { "fFTPClient",       "none:fFTPClient",      kPgNoWeb,   false },
    { "FormBarcodeReader","none:FormBarcodeReader",kPgNoWeb,  false },
    { "fRFID",            "none:fRFID",           kPgNoWeb,   false },
    { "FormHS",           "none:FormHS",          kPgNoWeb,   false },
    { "fLiftCount",       "none:fLiftCount",      kPgNoWeb,   false },
    { "frmDTME08",        "none:frmDTME08",       kPgNoWeb,   false },
    { "fARMSLog",         "none:fARMSLog",        kPgNoWeb,   false },
    { "fLaserSensor",     "none:fLaserSensor",    kPgNoWeb,   false },
};
const std::size_t kRowCount    = sizeof(kRows) / sizeof(kRows[0]);
const std::size_t kWebRowCount = 69;   //AI(W906-PAGETAB-S10) 20260929: 68 -> 69（trayedit）

// 每列的執行期狀態（只在 wb_serve 主迴圈那條執行緒上讀寫）。
struct RowState {
    bool        webWas;      // 上一拍的網頁答案（PageWebShowing）
    bool        prog;        // 程式狀態（PageProgramSet 最後一次）
    int         want;        // 0 ＝ 沒有；1 ＝ open；2 ＝ close（只有 kPgBoth 列會設）
    long        wseq;        // want 的序號（每次 PageProgramSet 加一）
    std::string by;          // 最後是誰改的："hmi" 或程式的出處
    std::string since;       // 最後一次改的時間（HH:MM:SS）
    bool        mismatchSaid;
    RowState() : webWas(false), prog(false), want(0), wseq(0), mismatchSaid(false) {}
};

RowState& St(std::size_t i)
{
    static std::vector<RowState> s(kRowCount);
    return s[i];
}

PageTableHost g_host = { 0, 0, 0, 0, 0 };
bool          g_armed = false;  unsigned long g_startRefusals = 0;   //AI(W906-PAGETAB-R144) 20260929: PageStartAllowed 拒絕的次數（SECS S2F42 回 HCACK 用，檔尾 W906_PageStartMark）
long          g_wseqNext = 0;

std::int64_t (*g_clockForTest)() = 0;
std::int64_t   g_graceForTest    = -1;

std::int64_t g_absentSince   = 0;     // 0 ＝ 有畫面（或還沒看過）；其他 ＝ 畫面不見的那一刻（毫秒）
std::int64_t g_nextActMs     = 0;     // 同一段不見期間：下一次最早可以再做 STOP 的時刻
bool         g_jobAtAbsence  = false; // 畫面剛不見那一拍有沒有網頁馬達工作（等寬限到期時用）
std::string  g_absentSinceStr;
bool         g_noteDue      = false;  // (c2)：做過 STOP，還沒出告警
std::int64_t g_noteDueSince = 0;
std::string  g_noteWhy;
void (*g_edgeHook)(const char*, bool) = 0;   // AI(W906-PAGETAB-Q51) 20260928 [W906] 步驟 5：網頁列的邊緣給別人（PageTableSetEdgeHook）

std::string  g_json;
std::string  g_jsonBody;
long         g_jsonSeq = 0;

std::int64_t NowMs()
{
    if (g_clockForTest) return g_clockForTest();
    return (std::int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::string ClockStr()
{
    const std::time_t t = std::time(0);
    const std::tm* lt = std::localtime(&t);
    char b[16];
    if (!lt) return std::string();
    std::snprintf(b, sizeof(b), "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
    return b;
}

bool IsOpenLike(WinState s) { return s == kWinOpen || s == kWinMinimized; }

// 規則 6／7 的「還有沒有 WebSocket 連著」。沒安裝（ctest 直接呼叫）⇒ 不知道 ⇒ 當成還連著（規則 6：照最後一次）。
bool WsLive()
{
    if (!g_host.liveWs) return true;
    return g_host.liveWs() > 0;
}

// 規則 3～7（網頁那一半）。form 不可以是空字串（form:null 的列總表根本不收，永遠回 false）。
bool WebRules(const char* form)
{
    if (!form || !*form) return false;
    if (!WebWindowRegistryEverAnyFrame()) return false;                        // 3：Q8-B
    const WinQuery q = WebWindowRegistryQuery(form);
    if (!q.fromAnyConn) return false;                                           // 5★
    if (!q.stale) return IsOpenLike(q.state);                                   // 4：新鮮的之間取聯集（總表 MT-FIX1）
    if (WsLive()) return q.state == kWinOpen;                                   // 6★：總表對「過期而最後說 open」回 kWinOpen
    return false;                                                               // 7★
}

void SayUnknown(const char* form, const char* where)
{
    static std::set<std::string> said;
    const std::string f = form ? form : "";
    if (said.count(f)) return;
    said.insert(f);
    std::printf("[PAGETAB] unknown form \"%s\" (%s) -- not in the 90-row table; answered by the web rules 3-7\n",
                f.c_str(), where);
    std::fflush(stdout);
}

bool RowAnswer(std::size_t i)
{
    const PageRowDef& d = kRows[i];
    switch (d.opener) {
    case kPgProgram: return St(i).prog;                                         // 1
    case kPgBoth:    return St(i).prog || WebRules(d.form);                     // 1＋2～7
    case kPgNoWeb:   return false;                                              // 2
    case kPgWeb:
    default:         return WebRules(d.form);                                   // 3～7
    }
}

bool RowWeb(std::size_t i)
{
    const PageRowDef& d = kRows[i];
    if (d.opener == kPgProgram || d.opener == kPgNoWeb) return false;
    return WebRules(d.form);
}

const char* OpenerName(PageOpener o)
{
    switch (o) {
    case kPgProgram: return "cpp";
    case kPgBoth:    return "both";
    case kPgNoWeb:   return "noweb";
    case kPgWeb:
    default:         return "web";
    }
}

// ui.pages 的 state 字串（給人看；判斷一律用 on）。
std::string RowStateStr(std::size_t i)
{
    const PageRowDef& d = kRows[i];
    if (d.opener == kPgNoWeb)   return "noweb";
    if (d.opener == kPgProgram) return St(i).prog ? "open" : "closed";
    if (!*d.form)               return "n/a";                                   // form:null：總表不收
    if (!WebWindowRegistryEverAnyFrame()) return "boot";
    const WinQuery q = WebWindowRegistryQuery(d.form);
    if (!q.fromAnyConn) return "unreported";
    if (q.stale) return q.state == kWinOpen ? "stale-open" : "stale-closed";
    switch (q.state) {
    case kWinOpen:      return "open";
    case kWinMinimized: return "minimized";
    case kWinClosed:    return "closed";
    case kWinNever:     return "never";
    default:            return "absent";                                         // H1：這台沒建立（總表把不認得的字串當 Unknown）
    }
}

void JsonStr(std::string& o, const std::string& s)
{
    o += '"';
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') { o += '\\'; o += (char)c; }
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", (unsigned)c); o += b; }
        else o += (char)c;
    }
    o += '"';
}

} // namespace

// -----------------------------------------------------------------------------
const PageRowDef* PageTableRows(std::size_t& count) { count = kRowCount; return kRows; }
std::size_t PageTableWebRowCount() { return kWebRowCount; }

int PageTableFind(const char* form)
{
    if (!form || !*form) return -1;
    for (std::size_t i = 0; i < kRowCount; ++i)
        if (std::strcmp(kRows[i].form, form) == 0) return (int)i;
    return -1;
}

void PageTableArm(const PageTableHost& h)
{
    g_host  = h;
    g_armed = true;
    // 安裝當下取一次樣當成「上一拍」：安裝前就開著的畫面不算一次「打開」（開機時總表還沒收過 ⇒ 全部 false）。
    for (std::size_t i = 0; i < kRowCount; ++i) St(i).webWas = RowWeb(i);
    std::printf("[PAGETAB] armed: %u rows (%u web), no-screen grace %ld ms, liveWs=%s pause=%s homeClose=%s motorStop=%s alarm=%s (%s)\n",
                (unsigned)kRowCount, (unsigned)kWebRowCount, (long)PageNoScreenGraceMs(),
                h.liveWs ? "yes" : "no", h.pause ? "yes" : "no", h.homeClose ? "yes" : "no", h.motorStop ? "yes" : "no",
                h.alarm ? "yes" : "no", kPageNoScreenAlarmCode);
    std::fflush(stdout);
}

bool PageTableArmed() { return g_armed; }

std::int64_t PageNoScreenGraceMs() { return g_graceForTest >= 0 ? g_graceForTest : kPageNoScreenGraceMs; }

// -----------------------------------------------------------------------------
void PageProgramSet(const char* form, bool open, const char* where)
{
    const int i = PageTableFind(form);
    if (i < 0) { SayUnknown(form, "PageProgramSet"); return; }
    RowState& s = St((std::size_t)i);
    s.prog  = open;
    s.by    = where ? where : "program";
    s.since = ClockStr();
    if (kRows[i].opener == kPgBoth) {
        // Q-P2＝A：每個 HMI 照做一次（wseq 讓「已經照做過的」不再做，也讓重新載入的 HMI 補做目前這一次）。
        s.want = open ? 1 : 2;
        s.wseq = ++g_wseqNext;
        std::printf("[PAGETAB] program %s %s (%s) -> want=%s wseq=%ld\n", open ? "opened" : "closed",
                    kRows[i].form, s.by.c_str(), open ? "open" : "close", s.wseq);
        std::fflush(stdout);
    }
}

// -----------------------------------------------------------------------------
bool PageFormAnswer(const char* form)
{
    const int i = PageTableFind(form);
    if (i >= 0) return RowAnswer((std::size_t)i);
    SayUnknown(form, "PageFormAnswer");
    return WebRules(form);
}

bool PageFormShowing(const char* form, bool member) { return member || PageFormAnswer(form); }

bool PageWebShowing(const char* form)
{
    const int i = PageTableFind(form);
    if (i >= 0) return RowWeb((std::size_t)i);
    return WebRules(form);
}

bool PageScreenPresent()
{
    if (g_host.liveWs && g_host.liveWs() <= 0) return false;                  // 一條 WebSocket 都沒有 ⇒ 沒有畫面
    return WebRules("fMain");                                                   // 主畫面 locked：有外框就一定是 open
}

bool PageStartAllowed(const char* func, std::string* why)
{
    if (!g_armed) return true;
    if (PageScreenPresent()) return true;
    const int ws = g_host.liveWs ? g_host.liveWs() : -1;
    char b[400];
    std::snprintf(b, sizeof(b),
                  "START refused: no HMI screen connected (WebSocket=%d, main screen %s) -- "
                  "Steven Q-P1 20260928: no screen => no system start; open the HMI and press START again",
                  ws, RowStateStr(0).c_str());
    ++g_startRefusals;  if (why) *why = b;   //AI(W906-PAGETAB-R144) 20260929: 先數再填 why（同一行）
    std::printf("[PAGETAB] %s [%s]\n", b, func ? func : "");
    std::fflush(stdout);
    return false;
}

// -----------------------------------------------------------------------------
DiagVerdict PageDiagnosticsOpen(bool systemStart, int contactMode, bool homingActive)
{
    DiagVerdict v;
    v.open         = false;
    v.tier         = kDiagNone;
    v.everAnyFrame = WebWindowRegistryEverAnyFrame();

    std::size_t n = 0;
    const char* const* t = WebWindowRegistryTierForms(kDiagAlways, n);          // golden :7348
    for (std::size_t i = 0; i < n; ++i) {
        if (!homingActive && std::strcmp(t[i], "fHome") == 0) continue;         // Q9 的理由：WebWindowRegistry.h
        if (PageFormAnswer(t[i])) { v.open = true; v.tier = kDiagAlways; v.form = t[i]; return v; }
    }
    if (contactMode != 0) {                                                     // golden :7349
        t = WebWindowRegistryTierForms(kDiagContact, n);
        for (std::size_t i = 0; i < n; ++i)
            if (PageFormAnswer(t[i])) { v.open = true; v.tier = kDiagContact; v.form = t[i]; return v; }
    }
    if (!systemStart) {                                                         // golden :7350-7360
        t = WebWindowRegistryTierForms(kDiagStopped, n);
        for (std::size_t i = 0; i < n; ++i)
            if (PageFormAnswer(t[i])) { v.open = true; v.tier = kDiagStopped; v.form = t[i]; return v; }
    }
    return v;
}

// -----------------------------------------------------------------------------
PageTickResult PageTableTick(const PageTickFacts& f)
{
    PageTickResult r;
    r.edges    = 0;
    r.screen   = false;
    r.absentMs = 0;
    r.actions  = 0;
    if (!g_armed) return r;

    const std::int64_t now = NowMs();
    const bool screen = PageScreenPresent();
    r.screen = screen;

    // (a)(b) 每個網頁列的邊緣
    for (std::size_t i = 0; i < kWebRowCount; ++i) {
        const PageRowDef& d = kRows[i];
        if (!*d.form) continue;                                                 // form:null：總表不收，沒有答案可比
        RowState& s = St(i);
        const bool w = RowWeb(i);
        if (w == s.webWas) continue;
        s.webWas = w;
        s.by     = "hmi";
        s.since  = ClockStr();
        ++r.edges;
        if (g_edgeHook) g_edgeHook(d.form, w);                                  // 步驟 5：例 C 路頁關窗 ⇒ FileRW/_EditPage.cpp 清「開過了」（wb_serve 裝）
        if (!w && d.opener == kPgBoth && s.prog) {
            if (!screen) {
                std::printf("[PAGETAB] %s closed because the whole HMI screen went away -- not an operator close; "
                            "the no-screen grace (%ld ms) decides\n", d.form, (long)PageNoScreenGraceMs());
            } else if (std::strcmp(d.form, "fHome") == 0 && g_host.homeClose) {
                // golden：操作員關掉 Home Monitor ＝ TfHome::FormClose（uhome.cpp:4868-4872）⇒ fShow=false ⇒
                //   回原點狀態機 `if(fHome->fShow==false) { SoftStop=true; break; }`（Q-P2 例子「中途把它關掉 ⇒ 照 golden 停止回原點」）
                std::printf("[PAGETAB] fHome closed by the operator on the HMI -> fHome->Close() (golden TfHome::FormClose)\n");
                std::fflush(stdout);
                g_host.homeClose("operator closed Home Monitor on the HMI (golden TfHome::FormClose uhome.cpp:4868)");
                r.actions |= kPgActHomeClose;
            } else { extern void (*W906_TrayEditWindowClosedHook)(); if (std::strcmp(d.form, "TrayEditForm") == 0 && W906_TrayEditWindowClosedHook) W906_TrayEditWindowClosedHook(); else   // AI(W906-S10) 20260929 (St02-E, S-10 claim 3.7; applied by ST01-E 20260930): operator closed Tray Edit = golden Cancel (TrayEditForm.cpp WindowClosed)
                std::printf("[PAGETAB] %s closed by the operator on the HMI (program had it open; no FormClose wired yet)\n", d.form);
            }
        }
    }

    // (c) 畫面不見 ⇒ 寬限 ⇒ 正常 STOP
    if (screen) {
        if (g_absentSince != 0) {
            std::printf("[PAGETAB] HMI screen back after %ld ms\n", (long)(now - g_absentSince));
            std::fflush(stdout);
        }
        g_absentSince = 0;
        g_nextActMs   = 0;
        g_jobAtAbsence = false;
        g_absentSinceStr.clear();
    } else {
        if (g_absentSince == 0) {
            g_absentSince    = now;
            g_nextActMs      = now + PageNoScreenGraceMs();
            g_jobAtAbsence   = f.webMotorJob;
            g_absentSinceStr = ClockStr();
            std::printf("[PAGETAB] no HMI screen (WebSocket=%d) -- SystemStart=%d HOME-ALL=%d web motor job=%d; "
                        "normal STOP in %ld ms if still gone and something runs (Steven Q-P1 20260928)\n",
                        g_host.liveWs ? g_host.liveWs() : -1, f.systemStart ? 1 : 0, f.homingAll ? 1 : 0,
                        f.webMotorJob ? 1 : 0, (long)PageNoScreenGraceMs());
            std::fflush(stdout);
        }
        r.absentMs = now - g_absentSince;
        if (now >= g_nextActMs) {
            const bool job = f.webMotorJob || g_jobAtAbsence;
            const int  fh  = PageTableFind("fHome");
            const bool homeProg = fh >= 0 && St((std::size_t)fh).prog;
            char why[200];
            std::snprintf(why, sizeof(why), "W906 PAGETAB: no HMI screen for %ld ms (all browsers closed)",
                          (long)r.absentMs);
            if (f.systemStart && g_host.pause) {
                std::printf("[PAGETAB] %s while SystemStart=1 -> normal STOP (golden TfMain::Pause)\n", why);
                std::fflush(stdout);
                g_host.pause(why);
                r.actions |= kPgActPause;
            }
            if (f.homingAll && homeProg && g_host.homeClose) {
                std::printf("[PAGETAB] %s during HOME ALL -> fHome->Close() (golden: Home Monitor closed stops homing)\n", why);
                std::fflush(stdout);
                g_host.homeClose(why);
                r.actions |= kPgActHomeClose;
            }
            if (job && g_host.motorStop) {
                std::printf("[PAGETAB] %s with a web motor job -> motor.stop (golden btnStopClick)\n", why);
                std::fflush(stdout);
                g_host.motorStop(why);
                r.actions |= kPgActMotorStop;
                g_jobAtAbsence = false;
            }
            if (r.actions != 0) {
                g_nextActMs    = now + PageNoScreenGraceMs();                  // 同一段不見期間，隔一個寬限才會再做
                g_noteDue      = true;                                          // (c2)
                g_noteDueSince = now;
                g_noteWhy      = why;
            }
        }
    }

    // (c2) STOP 之後的 golden 告警（Jimmy 20260928）：等暫停檢查走完（SystemStart 變 false）或最多 kPageNoScreenAlarmWaitMs。
    //   畫面已經回來也照樣出 —— 操作員要看得到剛才為什麼停。
    if (g_noteDue && (!f.systemStart || now - g_noteDueSince >= kPageNoScreenAlarmWaitMs)) {
        g_noteDue = false;
        if (g_host.alarm) {
            std::printf("[PAGETAB] %s -> alarm %s (ShowErrorMessage, kcode 0; shown when an HMI screen is back)\n",
                        g_noteWhy.c_str(), kPageNoScreenAlarmCode);
            std::fflush(stdout);
            g_host.alarm(g_noteWhy.c_str());
            r.actions |= kPgActAlarm;
        }
    }

    // (d) 頁面數兩邊核對：表上的網頁表單，新鮮回報裡卻沒有
    const WinRegistryStats st = WebWindowRegistryStats();
    if (st.connections > st.staleConns) {
        for (std::size_t i = 0; i < kWebRowCount; ++i) {
            const PageRowDef& d = kRows[i];
            RowState& s = St(i);
            if (!*d.form || s.mismatchSaid) continue;
            if (!WebWindowRegistryQuery(d.form).fromAnyConn) {
                s.mismatchSaid = true;
                std::printf("[PAGETAB] mismatch: %s (id %s) is in the C++ page table but no HMI frame lists it "
                            "(old background.html without the 'absent' rows? D:\\HT9045\\web\\background.html buildRegistry)\n",
                            d.form, d.webId);
            }
        }
    }
    if (r.edges > 0 || r.actions != 0) std::fflush(stdout);
    return r;
}

// -----------------------------------------------------------------------------
const std::string& PageTableJson()
{
    std::string b;
    b.reserve(9000);
    b += "\"count\":";
    b += std::to_string((long long)kRowCount);
    b += ",\"web\":";
    b += std::to_string((long long)kWebRowCount);
    b += ",\"armed\":";
    b += g_armed ? "1" : "0";
    b += ",\"screen\":";
    b += (g_armed && PageScreenPresent()) ? "1" : "0";
    b += ",\"absentSince\":";
    JsonStr(b, g_absentSinceStr);
    b += ",\"rows\":[";
    for (std::size_t i = 0; i < kRowCount; ++i) {
        const PageRowDef& d = kRows[i];
        const RowState& s = St(i);
        if (i) b += ',';
        b += "{\"form\":";   JsonStr(b, d.form);
        b += ",\"id\":";     JsonStr(b, d.webId);
        b += ",\"op\":\"";   b += OpenerName(d.opener); b += '"';
        b += ",\"state\":";  JsonStr(b, RowStateStr(i));
        b += ",\"on\":";     b += RowAnswer(i) ? "1" : "0";
        b += ",\"by\":";     JsonStr(b, s.by);
        b += ",\"since\":";  JsonStr(b, s.since);
        if (d.opener == kPgProgram || d.opener == kPgBoth) {
            b += ",\"prog\":"; b += s.prog ? "1" : "0";
        }
        if (d.opener == kPgBoth) {
            b += ",\"want\":\""; b += s.want == 1 ? "open" : (s.want == 2 ? "close" : ""); b += '"';
            b += ",\"wseq\":";   b += std::to_string((long long)s.wseq);
        }
        b += '}';
    }
    b += "]";
    if (b != g_jsonBody) {
        g_jsonBody = b;
        ++g_jsonSeq;
        g_json  = "{\"type\":\"ui.pages\",\"seq\":";
        g_json += std::to_string((long long)g_jsonSeq);
        g_json += ',';
        g_json += g_jsonBody;
        g_json += '}';
    }
    return g_json;
}

// -----------------------------------------------------------------------------
void PageTableResetForTest()
{
    for (std::size_t i = 0; i < kRowCount; ++i) St(i) = RowState();
    g_host = PageTableHost();
    g_host.liveWs = 0; g_host.pause = 0; g_host.homeClose = 0; g_host.motorStop = 0; g_host.alarm = 0;
    g_noteDue = false; g_noteDueSince = 0; g_noteWhy.clear();
    g_armed = false;
    g_wseqNext = 0;
    g_absentSince = 0;
    g_nextActMs = 0;
    g_jobAtAbsence = false;
    g_absentSinceStr.clear();
    g_json.clear();
    g_jsonBody.clear();
    g_jsonSeq = 0;
    g_graceForTest = -1;
    g_edgeHook = 0;
}

void PageTableSetClockForTest(std::int64_t (*nowMs)()) { g_clockForTest = nowMs; }
void PageTableSetGraceForTest(std::int64_t ms) { g_graceForTest = ms; }
void PageTableSetEdgeHook(void (*cb)(const char* form, bool open)) { g_edgeHook = cb; }  void (*W906_TrayEditWindowClosedHook)() = 0;   // AI(W906-S10) 20260929 (St02-E, S-10 claim 3.8; applied by ST01-E 20260930): set by TrayEditForm.cpp (wb_serve only); 0 in the tests

} // namespace ht9045

// =============================================================================
//  全域入口（別人的檔用同一行的 block-scope extern 呼叫）
// =============================================================================
void W906_PageProgramSet(const char* form, bool open, const char* where) { ht9045::PageProgramSet(form, open, where); }
bool W906_PageFormAnswer(const char* form) { return ht9045::PageFormAnswer(form ? form : ""); }

bool W906_PageStartAllowed(const char* func) { return ht9045::PageStartAllowed(func, 0); }

bool W906_PageDiagnosticsOpen(bool systemStart, int contactMode, bool homingActive)
{
    return ht9045::PageDiagnosticsOpen(systemStart, contactMode, homingActive).open;
}

void W906_PageTableArm(int (*liveWs)(), void (*pause)(const char*), void (*homeClose)(const char*),
                       void (*motorStop)(const char*), void (*alarm)(const char*))
{
    ht9045::PageTableHost h;
    h.liveWs    = liveWs;
    h.pause     = pause;
    h.homeClose = homeClose;
    h.motorStop = motorStop;
    h.alarm     = alarm;
    ht9045::PageTableArm(h);
}

void W906_PageTableTick(bool systemStart, bool homingAll, bool webMotorJob)
{
    ht9045::PageTickFacts f;
    f.systemStart = systemStart;
    f.homingAll   = homingAll;
    f.webMotorJob = webMotorJob;
    ht9045::PageTableTick(f);
}

const char* W906_PageTableJson() { return ht9045::PageTableJson().c_str(); }

void W906_PageTableEdgeHookSet(void (*cb)(const char* form, bool open)) { ht9045::PageTableSetEdgeHook(cb); }

// =============================================================================
//AI(W906-PAGETAB-R144) 20260929 [W906] Steven R144 11:1x「SECS 的SF code 有ACK 可以回覆, 挑一個正確的ACK進行回覆」。
//  沒有畫面時主機遠端 START／HOME 被頁面表擋下（WebStart.cpp StartFromWeb 開頭、csystem.cpp:30425 MainProc），
//  但 SECS S2F41 的回覆（S2F42 HCACK）原本照樣回 0＝已接受。golden 同一支 S2F42_Host_Command_Acknowledge 遇到
//  「現在不能做」一律回 2（AUTO_RETEST／TRAY_FEED 機台裡有 IC、HALT 不允許，SECSGEM/uHGemHT9045.cpp:5499／:5517／:5532；HALT :5893）——
//  SEMI E5 HCACK 2＝Cannot perform now，所以這裡也回 2。
//  用法（SECSGEM/uHGemHT9045.cpp S2F42_Host_Command_Acknowledge，同一行、行數不變）：函式開頭記 mark＝W906_PageStartMark()
//  與 SoftStart；回覆前問 W906_PageStartRefusedSince(mark, 這一條指令把 SoftStart 從 false 設成 true 而且沒在運轉)：
//    * mark 之後頁面表拒絕過 START（fMain->Start → StartFromWeb 當場擋）⇒ true；
//    * 或這一條指令拉起 SoftStart（HOME、AUTO_RETEST、TRAY_FEED…；MainProc 下一拍才擋）而頁面表現在會擋 ⇒ true。
//  呼叫端經 csystem.cpp:30040 的兩個函式指標（wb_serve.cpp:4389 同一行裝上），ctest／沒有頁面表的組態指標是 0 ⇒ 照 golden。
// =============================================================================
unsigned long W906_PageStartMark() { return ht9045::g_startRefusals; }
bool W906_PageStartRefusedSince(unsigned long mark, bool softStartRaisedIdle)
{
    if (ht9045::g_startRefusals != mark) return true;
    return softStartRaisedIdle && ht9045::PageTableArmed() && !ht9045::PageScreenPresent();
}

// AI(W906-STREAM-2A) 20260930: the stream gate the tag publisher asks (ht9045::SetStreamWantedHook, installed by tools/wb_serve.cpp).
//   No WebSocket at all -> no recipient -> false (PageScreenPresent; an unarmed table answers "present" = send).
//   Otherwise the window registry's answer for that background.html id (uncertain -> send).
bool W906_PageStreamWanted(const char* webId)
{
    if (!ht9045::PageScreenPresent()) return false;
    return ht9045::WebWindowRegistryStreamWanted(webId ? std::string(webId) : std::string());
}
