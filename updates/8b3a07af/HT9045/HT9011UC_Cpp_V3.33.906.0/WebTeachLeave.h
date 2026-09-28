// =============================================================================
//  WebTeachLeave.h  --  S122：關掉（或打開）Teach／Motor Test ⇒ 標成必須重新回原點
//
//  //AI(W906-FRW-S122) 20260927 [W906] St01（Steven 團隊）。
//
//  ## 裁決
//
//  * RULINGS_20260927 第 2 條第 18 題＝B：只做「關掉 Teach／Motor Test ⇒ 標成必須重新
//    find home」；主畫面斷線或重新整理**不**暫停生產。
//  * decisions-pending R80～R83 全部照 St01 建議 A：
//      R80＝A  用現成的視窗總表（`ui.windows.put`，WebWindowRegistry）判斷「在用／不在用」
//      R81＝A  打開那一下也清（照 golden FormShow）
//      R82＝A  運轉中（SystemStart）不清、只印一行
//      R83＝A  不加畫面（照 golden：按 START 自己先回原點再接著跑）
//  * 方案全文：docs/S122_TEACH_LEAVE_PLAN_20260927.md §2～§4。
//
//  ## 照 golden（V912，cp950；D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\）
//
//  「必須重新回原點」在 golden 就是全域 `fAllMotorHome = false`：
//    main.cpp:28846-28847   `fTeach->ShowModal();` 回來的下一行 `fAllMotorHome=false; //Steven 20110211`
//    uteach.cpp:2442-2443   Teach 裡的 Motor Test 鈕 `fMotorTest->ShowModal(); fAllMotorHome=false;`
//    uteach.cpp:1610        `TfTeach::FormShow`（:1417 起）打開 Teach 也清
//  ⇒ 這不是「比 golden 嚴」，是把 golden 的「ShowModal 回來」翻到網頁的視窗開關。
//
//  ## [W906] 不在 golden 的部分
//
//  * golden 的 Teach／Motor Test 是 ShowModal，呼叫端在它回來的那一行清旗標；網頁沒有
//    ShowModal，C++ 也收不到 Teach 頁的 EXIT（Teach 頁不送任何東西）。
//    ⇒ 改用**視窗總表的邊緣**代替「ShowModal 回來」：每一拍（wb_serve 500 ms 拍子）看
//      `WebWindowRegistryFShowPolicy("fTeach")`／`("fMotorTest")`，各自與上一拍比：
//        在用 -> 不在用 ＝ 關掉（golden main.cpp:28847 / uteach.cpp:2443）
//        不在用 -> 在用 ＝ 打開（golden uteach.cpp:1610；R81＝A）
//    ★ 跟 MainProc 暫停用的是**同一個判斷**（csystem.cpp `MainProc` 的
//      `W906_FormFShow("fMotorTest"…)||W906_FormFShow("fTeach"…)` 經
//      WebMotorAccessLive.cpp `W906_HookFShow` → `WebWindowRegistryFShowPolicy`），
//      所以「MainProc 不再把 Teach 當開著」那一刻就是「標成要重新回原點」那一刻，兩件事不會分岔。
//      （唯一差別：回報全部過期的那段期間本檔沿用上一拍，見下面「刻意的邊界」。）
//  * 運轉中（`SystemStart==true`）的邊緣：**不清**，只印一行（R82＝A）。golden 到不了這個
//    狀態（main.cpp:28829-28830 `if(SystemStart) return;` 運轉中進不了 Teach）；網頁做得到
//    （Teach 開著時從實體面板或另一個分頁按 START）。運轉中清旗標會讓 `DoAllProcess` 每一拍
//    return（移植樹 csystem.cpp `SoftStop==true || SystemStart==false || fAllMotorHome==false`），
//    機台停在半路、沒有警報 —— 跟第 18 題 B「重新整理不停產」相反。之後也**不補清**（R3/R82 的 B 沒選）。
//  * Motor Test 打開那一下也清：golden 只能從 Teach 開 Motor Test（全樹 `fMotorTest->ShowModal`
//    只有 uteach.cpp:2442），那時 Teach 的 FormShow（:1610）已經清過 ⇒ 狀態相同；網頁另外可以從
//    主選單直接開 Motor Test（main.html `sbMotorTest`），在這裡補上同樣的效果。
//  * 每個邊緣都印一行 `[S122] ...`（不管旗標原本是什麼），上機才看得到判斷有沒有發生。
//
//  ## 刻意的邊界
//
//  * 本檔**不** include cmydef.h：`fAllMotorHome`／`SystemStart` 由呼叫端（tools/wb_serve.cpp）
//    用參數傳進來。所以 ctest（tests/test_teachleave.cpp）只要連 WebWindowRegistry.cpp＋cJSON。
//  * 「在用」的定義沿用總表現有的政策（契約 §6＋Q8-B＋Q20-甲＋MT-FIX1），本檔不改它：
//    從沒收過任何總表 -> 不在用；有新鮮（15 秒內）的回報就只看新鮮的。
//    ⚠ 只有一處不同：某個表單的回報**全部過期**時，政策對任何表單都回「在用」（連最後說 never 的也是），
//      本檔在那段期間**沿用上一拍**、不算邊緣（理由與量測在 WebTeachLeave.cpp TeachLeaveTickWith）。
//      MainProc 在過期期間照舊暫停，這一點不受影響。
//    ⇒ 重新整理約 15 秒後才算關掉；整個瀏覽器關掉要等下一個 HMI 連上才算；切到別的分頁、斷線重連不算
//      （方案 §2.3 例 3～7）。
//  * 不改網頁、不改 WebBridge、不改 WebWindowRegistry、不動 Jimmy 的 FileRW/Teach.cpp:259
//    （方案 §6 J1 已請 Jimmy 處理）。WebMotorAccess.cpp 的 `GoldenClearAllMotorHome` 照留，
//    兩邊都是寫 false，重複清無害。
//
//  ## 20260927 加：別的表單也能掛（W906_WindowEdgeRegister，本檔尾段）
//
//  //AI(W906-FRW-S122) 20260927 [W906] 同一套「視窗總表邊緣」開放給其他 golden 表單（第一個用的是 St02 的
//  Setup.TesterIF ＝ "FTestIF"）。S122 本身不改行為：取樣規則抽成 WindowEdgeSample（兩邊共用），
//  W906_TeachLeaveTick 先跑 S122、再跑登記的各筆（WindowEdgeTick），wb_serve 那一行不用改。
// =============================================================================
#ifndef WebTeachLeaveH
#define WebTeachLeaveH

namespace ht9045 {

// 兩個表單各自記上一拍「在不在用」。初值 false ＝ 還沒看過（跟「從沒收過總表 ⇒ 不在用」一致，
// 所以開機第一拍不會無中生有一個「關掉」）。
struct TeachLeaveState {
    bool teachWas = false;   // 上一拍 FShowPolicy("fTeach")
    bool mtWas    = false;   // 上一拍 FShowPolicy("fMotorTest")
};

enum TeachLeaveAct {
    kTLNone        = 0,   // 這一拍沒有會動作的邊緣
    kTLClear       = 1,   // 有邊緣、停機中 ⇒ fAllMotorHome=false
    kTLSkipRunning = 2    // 有邊緣、運轉中（SystemStart）⇒ 不清、只印一行（R82＝A）
};

struct TeachLeaveEdge {
    const char* form;     // "fTeach" / "fMotorTest"
    bool        closed;   // true ＝ 在用 -> 不在用；false ＝ 不在用 -> 在用
    bool        acts;     // 這個邊緣要不要動作（關掉一定要；打開看 clearOnOpen）
};

struct TeachLeaveVerdict {
    TeachLeaveAct  act;
    int            edges;     // 這一拍看到幾個邊緣（0..2；順序 fTeach 先、fMotorTest 後）
    TeachLeaveEdge edge[2];
};

// R81＝A：打開那一下也清。
extern const bool kTeachLeaveClearOnOpen;

// 純邏輯：兩個表單各自跟上一拍比，更新 state。不讀總表、不碰任何全域。
//   teachInUse / mtInUse -- 這一拍的 FShowPolicy("fTeach") / ("fMotorTest")
//   systemStart          -- 全域 SystemStart
//   clearOnOpen          -- 打開的邊緣要不要動作（R81 的答案；正式路徑傳 kTeachLeaveClearOnOpen）
TeachLeaveVerdict TeachLeaveStep(TeachLeaveState& s, bool teachInUse, bool mtInUse,
                                 bool systemStart, bool clearOnOpen);

// 讀總表（WebWindowRegistryFShowPolicy）＋套用：kTLClear 時把 *allMotorHome 設 false；
// 每個邊緣印一行 `[S122] ...`。allMotorHome 可以是 0（只判斷不寫）。state 由呼叫端帶（ctest 用）。
TeachLeaveVerdict TeachLeaveTickWith(TeachLeaveState& s, bool* allMotorHome, bool systemStart);

} // namespace ht9045

// tools/wb_serve.cpp 主迴圈 500 ms 拍子（pumpBeat）呼叫；state 是本檔的 static。
//   allMotorHome -- &fAllMotorHome（cmydef.h）
//   systemStart  -- SystemStart（cmydef.h）
//AI(W906-FRW-S122) 20260927 [W906] 20260927 起同一拍也跑 W906_WindowEdgeRegister 登記的各筆（S122 先、登記的後）。
void W906_TeachLeaveTick(bool* allMotorHome, bool systemStart);

// =============================================================================
//  通用的「表單關掉／打開」掛勾：W906_WindowEdgeRegister
//
//  //AI(W906-FRW-S122) 20260927 [W906] St01（Steven 團隊）。把 S122 的「視窗總表邊緣」開放給別的表單。
//
//  為什麼要有：golden 的表單關掉時跑 FormClose（呼叫端 ShowModal 回來的那幾行也是這時候），網頁沒有這個事件 ——
//    頁面按 Exit／✕ 只是 background.html 把視窗關掉，C++ 收不到任何指令。
//    例：St02 的 Setup.TesterIF 關掉不存檔 ⇒ golden V912 cTesterIF.cpp:1215-1235 `TFTestIF::FormClose`
//        （:1217 ReadTestIFFile() 把沒存的改動丟掉、:1221-1225 oldLastiTestMode、:1227-1231 TTL 收尾），
//        以及 main.cpp:28336 `FTestIF->ShowModal();` 回來之後的 :28337-28344。
//  ⇒ 開機時登記一筆（golden 表單名＋兩個 callback），之後每一拍由 W906_TeachLeaveTick
//    （tools/wb_serve.cpp:5953，500 ms 拍子；wb_serve 不用改）跟 S122 一起判斷，看到邊緣就呼叫。
//
//  規則（跟 S122 同一套；兩邊都走 ht9045::WindowEdgeSample，改規則只改一處）：
//    * 在用 ＝ WebWindowRegistryFShowPolicy(form)；這個表單的回報**全部過期**時沿用上一拍、不算邊緣
//      （理由見 WebTeachLeave.cpp TeachLeaveTickWith 的註解）。
//    * 在用 -> 不在用 ＝ 呼叫 onClose；不在用 -> 在用 ＝ 呼叫 onOpen（各自可以是 0 ＝ 那一邊不管）。
//    * skipWhileRunning ＝ true：運轉中（呼叫端傳進來的 SystemStart）看到的邊緣**不呼叫**、只印一行，
//      之後也不補呼叫（同 S122 的 R82＝A）；false ＝ 不管運轉與否都呼叫。
//    * 登記的當下先取一次樣當成「上一拍」：登記前就已經開著的表單不會被算成一次「打開」。
//      開機時（還沒收過任何總表，Q8-B）取到的是 false，跟 S122 的初值一樣。
//    * 同一個表單可以登記好幾筆（各記各的上一拍）；同一組 form＋onOpen＋onClose 重複登記 ＝ 冪等
//      （不新增、不重取樣，只更新 skipWhileRunning）。
//    * 同一拍的順序：先 S122（fTeach、fMotorTest），再照登記順序跑各筆。
//
//  時機與限制：
//    * callback 在 wb_serve 主迴圈那一條執行緒上跑（跟 WS 指令分派同一條，不用上鎖），但**不在任何 WS 指令裡**：
//      沒有「這個指令的回覆」可以附東西（C 路的 filerw::ELTodo／ELMessage 只會堆在 editlist 的 session 裡）。
//      每個邊緣本檔已經印一行 `[WinEdge] ...`；callback 自己要留痕跡請 printf。
//    * 關掉後約 0.5 秒內呼叫；F5 重新整理約 15 秒後才算關掉；整個瀏覽器關掉要等下一個 HMI 連上；
//      切到別的分頁、WS 斷線重連不算（同 S122 方案 docs/S122_TEACH_LEAVE_PLAN_20260927.md §2.3 例 3～7）。
//      兩個 HMI 分頁一開一關、開著的那頁被節流時會多算一次關掉（同方案 §2.3 例 8）。
//    * 分不出「按 Exit」還是「按 ✕」：兩種都是同一個關掉邊緣（網頁的 .exitbtn 就是叫 background.html 關視窗）。
//    * 最多 ht9045::kWindowEdgeMax 筆；form 最長 kWindowEdgeFormMax-1 字元。callback 丟例外會被接住、印一行，
//      那個邊緣照樣算用掉（不會每一拍重丟），同一拍後面的各筆照跑。
//    * form 用 golden 表單名 ＝ background.html WINDOWS 表的 `form:`（**不是** `id:`）：
//      例 Setup.TesterIF 是 "FTestIF"（大寫 F；D:\HT9045\web\background.html:447）。
//      瀏覽器不會回報的表單（WebWindowRegistry.cpp kNeverReportedForms，Q20-甲）永遠是「不在用」⇒ 永遠沒有邊緣。
//    * 本段只依賴 WebWindowRegistry（不 include cmydef.h）；SystemStart 由 W906_TeachLeaveTick 的參數傳進來。
// =============================================================================
typedef void (*W906WindowEdgeFn)();

// 回傳 true ＝ 已登記（新的一筆，或同一組 form＋onOpen＋onClose 已經在表上）。
// 回傳 false ＝ 拒絕（form 是 0／空字串／太長、onOpen 與 onClose 都是 0、表滿），會印一行 `[WinEdge] register refused`。
bool W906_WindowEdgeRegister(const char* form, W906WindowEdgeFn onOpen, W906WindowEdgeFn onClose,
                             bool skipWhileRunning);

namespace ht9045 {

const int kWindowEdgeMax     = 16;   // 登記表容量
const int kWindowEdgeFormMax = 64;   // form 名的緩衝（含結尾 0）

// 這一拍「在用」的取樣，S122 與登記的每一筆共用：
//   這個表單的回報全部過期（WebWindowRegistryQuery(form).stale）⇒ prev；否則 WebWindowRegistryFShowPolicy(form)。
bool WindowEdgeSample(const char* form, bool prev);

struct WindowEdgeTickResult {
    int edges;     // 這一拍看到的邊緣（全部登記項加總）
    int called;    // 其中真的呼叫了 callback 的（丟例外的也算）
    int skipped;   // 其中因為運轉中（skipWhileRunning）沒呼叫的
};

// 跑一拍全部登記項。W906_TeachLeaveTick 在 S122 之後呼叫；ctest 直接呼叫。
WindowEdgeTickResult WindowEdgeTick(bool systemStart);

int  WindowEdgeCount();          // 目前登記幾筆
void WindowEdgeResetForTest();   // ⛔ 只給 ctest：清空登記表（正常路徑沒有「取消登記」）

} // namespace ht9045

#endif // WebTeachLeaveH
