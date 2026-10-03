// =============================================================================
//  TimerTable.h  --  golden TTimer 的排程表（Timer 排程表核心）
//
//  AI(W906-TIMER-TABLE) 20261001。計畫書 docs/TIMER_TABLE_PLAN.md；golden 全表 docs/TIMER_CENSUS.md（tools/timer_census.py）。
//
//  ## 為什麼要有它
//
//  golden 的 124 支 TTimer（60 張表單）靠 Windows 的 WM_TIMER 觸發；移植樹沒有 VCL 訊息迴圈，
//  之前接上的片段是各自掛在 tools/wb_serve.cpp 主迴圈那幾行的尾巴、各自用 GetTickCount 限成一秒一次。
//  使用者 20261001 裁決（RULINGS_20261001 第 39 條）：改成一張排程表 ——
//    * golden 的 `Timer3->Enabled=true;` / `Timer3->Interval=500;` 原樣照翻（這裡的 Enabled／Interval 是屬性）；
//    * 主迴圈每一圈呼叫一次 TimerTableTick()，到期的才跑；
//    * 跟 golden 一樣跑在主執行緒（golden 的生產主流程 MainProc 也是經 Synchronize 跑在主執行緒，uruncontrol.cpp）；
//    * 告警框開著時不跑（第 39 條第 3 題＝甲：三個阻塞等待迴圈不呼叫 TimerTableTick）；
//    * 每支記錄跑了幾次、每次多久；超過門檻（預設 50 ms）印警告 —— 慢的 I/O 先量再搬（第 39 條第 7 題）。
//
//  ## 跟 VCL TTimer（BCB6 ExtCtrls）對照
//
//  | VCL                                         | 這裡                                                        |
//  |---------------------------------------------|-------------------------------------------------------------|
//  | Enabled／Interval 改了值 ⇒ KillTimer＋SetTimer，從現在重新數 | 屬性記一個「改過幾次」的計數；Tick 看到計數變了就從當下重新數       |
//  | 設成同一個值 ⇒ 什麼都不做                    | 同（值沒變計數不加）                                         |
//  | Interval=0 或 OnTimer=NULL ⇒ 不觸發          | 同                                                          |
//  | WM_TIMER 會合併：程式忙了 30 秒也只來一次     | 同：到期只跑一次，下一次排在下一個格子上，不補跑                |
//  | 同一支在自己的 OnTimer 裡又被觸發（ShowModal 等巢狀訊息迴圈）⇒ 再進來一次 | Tick 若被巢狀呼叫，正在跑的那一支跳過（計數 reentrySkips）；golden 本體自己的 static bRun 旗標照抄 |
//  | OnTimer 丟例外 ⇒ Application->HandleException，Timer 照樣在 | 同：接住、記數、印一行，下一次照跑；外層「正在跑」旗標一定放開   |
//  | 最小解析度約 10～16 ms                        | 主迴圈一圈（最長 50 ms，有網頁命令時提早醒）—— Interval 小於一圈的 Timer 一圈最多跑一次 |
//
//  ## 執行緒
//
//  只准在主迴圈那條執行緒呼叫 TimerTableTick()（第一次呼叫時記下執行緒，之後別的執行緒呼叫會印錯誤並直接 return）。
//  翻譯過來的 golden 本體本來就假設單一執行緒（沒有任何鎖），Enabled／Interval 也只該在主執行緒改。
//
//  ## 這個檔刻意不做的事
//
//  * 不 include windows.h、cmydef.h —— 放在 ht9045_globals（每個程式都 link 的最底層），誰都能用。
//  * 不知道「畫面開著沒」。那是 golden 本體自己判斷的（`if(!fShow) return;` 照抄，fShow 由頁面表給）；
//    排程表只照 Enabled 跑 —— 第 39 條第 6 題：該不該跑照 golden 每支的寫法，不統一成「網頁開了才跑」。
// =============================================================================
#ifndef W906_TIMER_TABLE_H
#define W906_TIMER_TABLE_H

#include <string>
#include <vector>

namespace ht9045 {

// golden TTimer::Enabled。`X->Enabled=true;`、`if(X->Enabled)`、`X->Enabled=!X->Enabled;`、
// `A->Enabled=B->Enabled;` 都照原樣寫得出來。值真的變了才把 Changes() 加一（VCL SetEnabled 同）。
class TTimerBoolProp {
public:
    explicit TTimerBoolProp(bool v) : v_(v), changes_(0) {}
    TTimerBoolProp& operator=(bool v) { if (v != v_) { v_ = v; ++changes_; } return *this; }
    // ⚠ 一定要有這個：沒有它 `A->Enabled=B->Enabled;` 會綁到隱含的拷貝賦值、把計數一起拷過去
    //   （記憶 ht9045-v906-vclcompat-proxy-copy-assign-trap 那一族）。
    TTimerBoolProp& operator=(const TTimerBoolProp& o) { return *this = o.v_; }
    operator bool() const { return v_; }
    unsigned Changes() const { return changes_; }
private:
    TTimerBoolProp(const TTimerBoolProp&);
    bool     v_;
    unsigned changes_;
};

// golden TTimer::Interval（ms，golden 型別 Cardinal）。傳給 printf 類可變參數時要寫 (int)X->Interval。
class TTimerIntProp {
public:
    explicit TTimerIntProp(int v) : v_(v), changes_(0) {}
    TTimerIntProp& operator=(int v) { if (v != v_) { v_ = v; ++changes_; } return *this; }
    TTimerIntProp& operator=(const TTimerIntProp& o) { return *this = o.v_; }
    operator int() const { return v_; }
    unsigned Changes() const { return changes_; }
private:
    TTimerIntProp(const TTimerIntProp&);
    int      v_;
    unsigned changes_;
};

struct TTimerStats {
    unsigned long long fires;          // OnTimer 真的被呼叫了幾次
    unsigned long long totalUs;        // 累計耗時（微秒）
    unsigned long long maxUs;          // 最長一次
    unsigned long long lastUs;         // 最近一次
    unsigned long long lastStartMs;    // 最近一次開始的時間（TimerTableTick 的 nowMs）
    unsigned long long maxLateMs;      // 到期之後最晚多久才跑到（主迴圈被拖住的程度）
    unsigned           overruns;       // 超過門檻（TimerTableSetOverrunMs，預設 50 ms）的次數
    unsigned           exceptions;     // OnTimer 丟出例外的次數
    unsigned           reentrySkips;   // 到期時自己還在跑（巢狀 Tick）而跳過的次數
};

// golden 表單上的一支 TTimer。golden `TTimer *Timer3;` ⇒ 門面 `ht9045::TTimerEntry *Timer3;`。
//   建構時登記進排程表、解構時登出；Enabled／Interval 照 golden .dfm（沒寫＝True／1000）。
//   OnTimer 是翻譯好的本體（自由函式，golden 的 Sender 不用）；還沒翻的留 0 ⇒ 不會跑（VCL 同）。
class TTimerEntry {
public:
    TTimerEntry(const char* name, bool enabled, int interval, const char* goldenCite, void (*onTimer)() = 0);
    ~TTimerEntry();

    TTimerBoolProp Enabled;            // golden TTimer::Enabled
    TTimerIntProp  Interval;           // golden TTimer::Interval（ms）
    void         (*OnTimer)();         // golden TTimer::OnTimer（翻譯好的本體）

    const char*    Name;               // "fMain.Timer3"（表單變數.元件名）
    const char*    GoldenCite;         // golden 本體位置，例 "main.cpp:25106-25742"
    const char*    Status;             // 翻譯進度：0＝沒接本體；"stub"／"partial"／"full"（工作卡接上時填）

    TTimerStats    Stats;              // 只讀（排程表自己寫）

    // ---- 排程表內部用，翻譯的程式碼不要碰 ----
    unsigned long long nextDueMs_;
    unsigned           seenEnabled_;
    unsigned           seenInterval_;
    void             (*seenOnTimer_)();
    bool               armed_;
    bool               running_;
private:
    TTimerEntry(const TTimerEntry&);
    TTimerEntry& operator=(const TTimerEntry&);
};

// 主迴圈每一圈呼叫一次（tools/wb_serve.cpp）。nowMs＝單調遞增的毫秒（W906_TickMs64）。
void TimerTableTick(unsigned long long nowMs);

// 依名字找（"fMain.Timer3"）；同名有多個時回最後登記的那一個。找不到回 0。
TTimerEntry* TimerTableFind(const char* name);
int          TimerTableCount();

// 目前表上的全部（快照）／把一支拿下表（物件不刪，之後怎麼寫它的 Enabled 都不會跑；解構時再登出也安全）。
//   用途：同一張表單的門面被建了兩份時，留下真正在用的那一份（W906_TfMain_TimersBoot：fMain 換成 TfMainWeb 之後）。
std::vector<TTimerEntry*> TimerTableAll();
void         TimerTableUnregister(TTimerEntry* t);

// 一支一行的文字報告／給網頁或除錯用的 JSON（{"overrunMs":50,"timers":[...]}）。
std::string  TimerTableReport();
std::string  TimerTableJson();

// 超過幾 ms 算「太慢」要印警告（預設 50；第 39 條第 7 題）。
void         TimerTableSetOverrunMs(unsigned ms);
unsigned     TimerTableOverrunMs();

// 每隔多久自動把 TimerTableReport() 印到主控台（預設 600000＝10 分鐘；0＝不印）。第一次在開機後一個週期。
void         TimerTableSetReportEveryMs(unsigned long long ms);

// 每跑一支 Timer 之前呼叫一次（傳 Timer 名字）。wb_serve 接到停擺看門狗：卡在哪一支 Timer，看門狗的報告就印得出名字。
void         TimerTableSetMarkHook(void (*mark)(const char* timerName));

// ---- 測試用 ----
// 換掉量耗時用的時鐘（微秒）；傳 0 換回 QueryPerformanceCounter。
void         TimerTableSetUsClockForTest(unsigned long long (*clockUs)());
// 忘掉「第一次呼叫的執行緒」（每個測試案例開頭用）。
void         TimerTableResetThreadForTest();

}  // namespace ht9045

// tools/wb_serve.cpp 的寫法是在區塊裡 `{ extern void X(); X(); }`，宣告不了命名空間裡的函式 —— 給它兩個全域的入口。
void W906_TimerTableTick(unsigned long long nowMs);                    // = ht9045::TimerTableTick
void W906_TimerTableTickNow();                                         // = TimerTableTick(GetTickCount 的 64 位元延伸)；主迴圈用這支
void W906_TimerTableSetMarkHook(void (*mark)(const char* timerName));  // = ht9045::TimerTableSetMarkHook
void W906_TimerTableReportNow();                                       // 把 TimerTableReport() 印到主控台（wb_serve 離開主迴圈時）

#endif  // W906_TIMER_TABLE_H
