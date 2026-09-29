// =============================================================================
//  WebPageTable.h  --  頁面表：C++ 的「每個畫面開著沒」（取代 BCB 的 fShow）
//
//  //AI(W906-PAGETAB-Q51) 20260928 [W906] St01（Steven 團隊）。
//
//  ## 裁決
//
//  * Steven 20260928 Q51：「在c++端弄一個陣列, 紀錄目前總共有多少頁面, 每個頁面的狀態是開還是關, 然後跟html端互通」
//  * 第一優先：「bcb的 fShow的flag, 使用剛剛說的那個網頁顯示的陣列來替代, html跟c++要同時修改」
//  * Q-P1（20260928 10:3x，AskUserQuestion 給 ST01-E）：「system start時, 或是馬達有在操作的時候, 要確保是開著.
//    其他時間比較不重要, 但是關閉沒畫面的時候, c++不可以system start」
//      (1) 完全沒有 HMI 畫面時，所有 START 都拒絕，直到有畫面連上（PageStartAllowed）。
//      (2) 運轉中（SystemStart）或馬達操作中，瀏覽器全部關掉 ⇒ 照 golden 正常 STOP；中間留一段寬限，
//          讓 F5 重新整理不會停機（kPageNoScreenGraceMs，理由在 WebPageTable.cpp 檔頭）。
//      其餘照設計的回答規則（新鮮 ⇒ 聯集；過期但還有 WebSocket ⇒ 最後一次；過期且沒有 WebSocket ⇒ 關；
//      沒回報／absent ⇒ 關）。
//  * Q-P2＝A：程式自己開／關的畫面（例：HOME 時的 Home Monitor），每個 HMI 自動開／關（ui.pages 的 want）。
//  * Q-P3＝A：直接生效（不先只記錄）。本檔只提供函式與 4 個呼叫端改接；讀取點分批（批 1～6）不在這一刀。
//  * 設計全文：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md（9df70eeb）。
//
//  ## 分工（為什麼不改 WebWindowRegistry）
//
//  視窗總表（WebWindowRegistry，Jimmy／EastSun）＝「網頁說了什麼」（契約層），本檔**一行都不改它**；
//  頁面表＝「C++ 怎麼用它」（政策層，跟 Steven 對）。網頁的回報照舊由總表解析，本檔只用它現成的查詢
//  （WebWindowRegistryQuery／EverAnyFrame／TierForms）。
//
//  ## 刻意的邊界
//
//  * 只依賴 WebWindowRegistry 與標準庫（不 include cmydef.h／vclcompat）⇒ tests/test_pagetable.cpp 只要連
//    本檔＋WebWindowRegistry.cpp＋cJSON，秒級。機台全域（SystemStart、fHome、g_webMain…）由 wb_serve 用參數
//    或 callback 傳進來（PageTableArm／PageTableTick）。
//  * 執行緒：跟總表一樣，只在 wb_serve 主迴圈那一條執行緒上讀寫（ui.windows.put 分派、500 ms 拍子、
//    PublishHandlerTags 都在那一條）。沒有鎖。
//  * 沒有安裝（PageTableArm 沒呼叫過 ＝ ctest／沒有伺服器）：本表不主動做任何事 —— 不拒絕 START、
//    拍子什麼都不做；WebTeachLeave 的取樣退回原本的總表政策（行為與 1910e7ca 相同）。
// =============================================================================
#ifndef WebPageTableH
#define WebPageTableH

#include <cstddef>
#include <cstdint>
#include <string>

#include "WebWindowRegistry.h"   // DiagVerdict

namespace ht9045 {

// 誰會開這個畫面。
enum PageOpener {
    kPgWeb     = 0,   // 操作員從網頁開（視窗總表回報）
    kPgProgram = 1,   // C++ 自己開的對話框（網頁沒有這個視窗；成員＋程式寫入）
    kPgBoth    = 2,   // 兩者都會，而且網頁有視窗（fHome：回原點程式 Show／Close＋操作員選單）
    kPgNoWeb   = 3    // golden 會讀、網頁沒有視窗 ⇒ 永遠關（成員照 OR）
};

// 列定義（寫死在 C++；ctest T2 對 D:\HT9045\web\background.html 的 WINDOWS 表核對）。
struct PageRowDef {
    const char* form;      // golden 表單名（fShow 的查詢鍵）；網頁自己的頁（form:null）是 ""
    const char* webId;     // background.html WINDOWS 的 id；C++ 對話框 "cpp:<form>"；沒有網頁 "none:<form>"
    PageOpener  opener;
    bool        debugOnly; // background.html 的 debugOnly:true（release 不建立 ⇒ 網頁回報 absent）
};

const PageRowDef* PageTableRows(std::size_t& count);   // 90 列：網頁 68（順序＝WINDOWS 表）＋網頁沒有的 22
std::size_t       PageTableWebRowCount();               // 68
int               PageTableFind(const char* form);      // golden 表單名 → 列號；-1 ＝ 不在表上（"" 也回 -1）

// ---- 安裝（wb_serve 開機時一次，tools/wb_serve.cpp:4389）-----------------------------------------
//  liveWs     -- 現在連著的 WebSocket 數（WebBridgeServer::LiveWebSocketCount，WebBridgeServer.h:247）
//  pause      -- golden STOP：TfMain::Pause（移植 TfMainWeb::PauseFromWeb，面板 STOP 鍵 golden main.cpp ScanKey
//                SnFKPause → BtnPauseClick → Pause("BtnPauseClick") 走的同一條）
//  homeClose  -- golden fHome->Close()（TfHome::FormClose：fShow=false ⇒ 回原點狀態機照 golden SoftStop）
//  motorStop  -- 網頁馬達工作的 STOP（motor.stop ＝ golden btnStopClick：StopAllMotor＋AllBtnUp）
//  alarm      -- 停下來之後的 golden 告警（Jimmy 20260928 TO_STEVEN §4 11:2x、NIGHT_REPORT E#36＝C）：
//                ShowErrorMessage(kPageNoScreenAlarmCode, 0, MMSystem, …)，kcode 0 ＝ 通知（不阻塞；照 20260924 裁決照樣停機），
//                寫進告警信箱，畫面回來時 background.html 會把它彈出來 ⇒ 操作員知道為什麼停。
//  任何一個可以是 0（那一個動作不做）。
struct PageTableHost {
    int  (*liveWs)();
    void (*pause)(const char* why);
    void (*homeClose)(const char* why);
    void (*motorStop)(const char* why);
    void (*alarm)(const char* why);
};
// 告警碼：MES16441（單元 16＝"System"，cMyDB.cpp:145 AlarmUnit[16]）。//AI(W906-PAGETAB-R143) 20260929: R143：Jimmy 20260929 改號 MES16441（原 MES1690 跟既有 WAR1690 撞號，golden 事件資料庫用數字當編號會混在一起；RULINGS_20260929 第 2 節）；
//   MES16441 查過沒人用（AlarmCodeList、D:\HT9045\Error\ 說明檔、golden、移植樹、網頁 JSON）。以下是 20260928 原本選 MES1690 時的查證：golden V912 與移植樹的 .cpp／.h、
//   D:\HT9045\Error\AlarmCodeList.txt 都沒有 MES1690～MES1698（MES1699 是既有碼）。MES（訊息類）⇒ AlarmType 3（W906_AlarmTypeOfCode），
//   不是 JAM／WAR：機台沒有壞，是沒有畫面。說明檔（Error\English\MES16441.dat、Error\Chinese\MES16441.dat、AlarmCodeList.txt 一行；文字 Jimmy 20260929 照原稿）
//   還沒加 ⇒ 在那之前畫面照 golden 顯示碼＋"Unknown Alarm Code"；理由另寫進事件紀錄（RecordProcess，wb_serve 那一行）。
extern const char* const kPageNoScreenAlarmCode;
void PageTableArm(const PageTableHost& h);
bool PageTableArmed();

// ---- 程式寫入：golden fXxx->Show()／Close() 的移植位置旁呼叫 ------------------------------------
//  open＝true 是 Show（FormShow），false 是 Close（FormClose）。where＝出處（檔:行＋golden 行），印在 ui.pages 的 by。
//  kPgBoth 列（網頁有視窗）另外設 want（"open"／"close"）＋遞增 wseq：每個 HMI 對每個 wseq 照做一次（H3）。
void PageProgramSet(const char* form, bool open, const char* where);

// ---- 回答（依序，第一條成立就回答；★＝與 1910e7ca 不同）---------------------------------------
//   1  程式列（kPgProgram）             ⇒ 程式狀態（成員另外由 PageFormShowing OR 進來）
//      兩者列（kPgBoth）                ⇒ 程式狀態 OR 網頁答案（下面 2～7）
//   2  沒有網頁的列（kPgNoWeb）          ⇒ 關
//   3  從來沒收過任何網頁的總表（Q8-B）  ⇒ 關
//   4  有新鮮（15 秒內）的回報           ⇒ 任何一個 HMI 說 open／minimized ⇒ 開；都說關／never／absent ⇒ 關
//   5★ 連著的 HMI 都沒提到這個表單       ⇒ 關（1910e7ca：不知道＝開）
//   6★ 只剩過期回報、還有 WebSocket 連著 ⇒ 照最後一次回報（1910e7ca：一律開）
//   7★ 只剩過期回報、沒有 WebSocket      ⇒ 關（1910e7ca：一律開）
//  不在表上的表單名：照網頁列的規則 3～7 回答，第一次印一行 [PAGETAB] unknown form。
bool PageFormAnswer(const char* form);                  // 表的答案（不含成員）
bool PageFormShowing(const char* form, bool member);    // member || PageFormAnswer(form)
bool PageWebShowing(const char* form);                  // 只看網頁（規則 3～7）；程式列／沒有網頁的列 ⇒ false

// 有沒有 HMI 畫面：連著的 WebSocket > 0 而且主畫面（fMain，WINDOWS 表 main 列，locked＝永遠開著）的網頁答案是開。
//   瀏覽器全關（pagehide 最後一份「全部關」）⇒ 立刻 false；瀏覽器當掉沒送 ⇒ WebSocket 斷掉就 false
//   （網路斷線沒收到關閉的，WebBridgeServer 最多 45 秒才算斷）。
bool PageScreenPresent();

// Q-P1 (1)：START 前問。沒安裝 ⇒ true（不擋）；有畫面 ⇒ true；沒有畫面 ⇒ false＋why＋印一行 [PAGETAB]。
bool PageStartAllowed(const char* func, std::string* why);

// golden Command.cpp:7348-7360 的三層（Bit4_HandlerDiagnostics），跟 WebWindowRegistryDiagnosticsOpen 同一套清單
//   與順序（WebWindowRegistryTierForms），每個表單改問 PageFormAnswer。fHome 仍要 homingActive（Q9 的理由，
//   WebWindowRegistry.h 參數說明）。
DiagVerdict PageDiagnosticsOpen(bool systemStart, int contactMode, bool homingActive);

// ---- 500 ms 拍子（tools/wb_serve.cpp:5953，pumpBeat）-----------------------------------------------
//  做四件事：
//   (a) 每個網頁列跟上一拍比，更新 by／since（ui.pages 用）；
//   (b) kPgBoth 列被**操作員**關掉（網頁答案 開→關、畫面還在、程式狀態仍是開）⇒ 照 golden FormClose：
//       fHome ⇒ host.homeClose（回原點照 golden 停下）。畫面整個不見時不算操作員關（交給 (c) 的寬限）。
//   (c) Q-P1 (2)：畫面不見滿 kPageNoScreenGraceMs 而且（SystemStart 或 HOME ALL 或網頁馬達工作）⇒ 正常 STOP：
//       SystemStart ⇒ host.pause；HOME ALL（fHome 程式開著）⇒ host.homeClose；網頁馬達工作
//       （這一拍或畫面剛不見那一拍）⇒ host.motorStop。做完之後同一段不見期間隔一個寬限才會再做。
//   (c2) 做了 (c) 的任何一個 ⇒ 等停下來（SystemStart 變 false，最多 kPageNoScreenAlarmWaitMs）再 host.alarm 一次：
//       先讓 golden 的暫停檢查（ckernel.cpp:1046 SoftStop 那一支）走完，再出告警 —— 告警本身會把 SoftStop 清掉（note.cpp:799）。
//   (d) 表上有、新鮮回報裡卻沒有的網頁表單 ⇒ 印一次 [PAGETAB] mismatch（頁面數兩邊核對）。
struct PageTickFacts {
    bool systemStart;    // 全域 SystemStart
    bool homingAll;      // golden HOME ALL 正在跑：fHome->iHomeStep != 1（forms/fHome.h:131）
    bool webMotorJob;    // 網頁馬達工作（W906_MotorHomingHook：Motor Test／Teach 的 HOME 工作）
};
enum PageTickAction {
    kPgActPause     = 1,
    kPgActHomeClose = 2,
    kPgActMotorStop = 4,
    kPgActAlarm     = 8
};
struct PageTickResult {
    int          edges;      // 這一拍網頁答案有變的列數
    bool         screen;     // PageScreenPresent()
    std::int64_t absentMs;   // 畫面不見多久了（有畫面 ＝ 0）
    int          actions;    // PageTickAction 的 OR
};
PageTickResult PageTableTick(const PageTickFacts& f);

extern const std::int64_t kPageNoScreenGraceMs;   // 10000（理由見 WebPageTable.cpp 檔頭）
extern const std::int64_t kPageNoScreenAlarmWaitMs;   // 3000：STOP 之後等 SystemStart 變 false 的上限，到了照樣出告警
std::int64_t PageNoScreenGraceMs();

// ---- ui.pages（WebBridgeTags.cpp:1263 同一行經 W906_UiPagesJsonHook 送出）---------------------------
//  {"type":"ui.pages","seq":N,"count":90,"web":68,"armed":1,"screen":1,"absentSince":"",
//   "rows":[{"form":"fMain","id":"main","op":"web","state":"open","on":1,"by":"hmi","since":"11:22:33",
//            "prog":0,"want":"","wseq":0}, ...]}
//  seq 只在內容變的時候加一（tag 通道只送變化）。
const std::string& PageTableJson();

// ---- 網頁列的開／關邊緣給別人（步驟 5；AI(W906-PAGETAB-Q51) 20260928 [W906]）--------------------------------
//  PageTableTick (a) 每看到一個網頁列（有 golden 表單名的那 47 列）的網頁答案變了，就呼叫一次 cb(golden 表單名, 現在開著嗎)。
//  用途：C 路頁關窗 ⇒ FileRW/_EditPage.cpp 把那一頁的「開過了」清掉，下次開窗＝重新開頁（golden FormShow、開窗閘、記 Enter；
//  R108／R110）。在 wb_serve 主迴圈那一條執行緒、不在任何 WS 指令裡（同 WebTeachLeave 的邊緣，不會在告警框的等待迴圈裡重入）。
//  只有一個座位（後裝的蓋掉先裝的）；0 ＝ 不呼叫。本檔不 include FileRW —— wb_serve 開機時接（tools/wb_serve.cpp:4389）。
void PageTableSetEdgeHook(void (*cb)(const char* form, bool open));

// ---- 測試用 ---------------------------------------------------------------------------------------
void PageTableResetForTest();                             // ⛔ 正常路徑不要呼叫：清掉安裝、程式狀態、寬限計時
void PageTableSetClockForTest(std::int64_t (*nowMs)());   // 0 ＝ 回到真的時鐘（steady_clock）
void PageTableSetGraceForTest(std::int64_t ms);           // <0 ＝ 回到 kPageNoScreenGraceMs

} // namespace ht9045

// ---- 全域入口（別人的檔用同一行的 block-scope extern 呼叫，不必 include 本檔）------------------------
void        W906_PageProgramSet(const char* form, bool open, const char* where);   // W906_FormProgramShowHook 的本體
bool        W906_PageFormAnswer(const char* form);                                  // W906_FormFShowHook 的本體（WebMotorAccessLive.cpp:1142）
bool        W906_PageStartAllowed(const char* func);                                // WebStart.cpp StartFromWeb 開頭
bool        W906_PageDiagnosticsOpen(bool systemStart, int contactMode, bool homingActive);   // wb_serve.cpp:888-890
void        W906_PageTableArm(int (*liveWs)(), void (*pause)(const char*), void (*homeClose)(const char*),
                              void (*motorStop)(const char*), void (*alarm)(const char*));   // wb_serve.cpp:4389
void        W906_PageTableTick(bool systemStart, bool homingAll, bool webMotorJob); // wb_serve.cpp:5953
const char* W906_PageTableJson();                                                   // W906_UiPagesJsonHook 的本體
void        W906_PageTableEdgeHookSet(void (*cb)(const char* form, bool open));       // tools/wb_serve.cpp:4389（步驟 5：C 路頁關窗）

//AI(W906-PAGETAB-R144) 20260929: SECS S2F42 的 HCACK（本體 WebPageTable.cpp 檔尾；說明在那裡）
unsigned long W906_PageStartMark();
bool          W906_PageStartRefusedSince(unsigned long mark, bool softStartRaisedIdle);

#endif // WebPageTableH
