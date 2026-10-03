// =============================================================================
//  TimerTable.cpp  --  golden TTimer 的排程表（實作）。說明、對照表、執行緒規則在 TimerTable.h 檔頭。
//
//  AI(W906-TIMER-TABLE) 20261001。測試 tests/test_timer_table.cpp（ctest TimerTable）。
// =============================================================================
#include "TimerTable.h"

#include <windows.h>

#include <cstdio>
#include <vector>

namespace ht9045 {

namespace {

// 用 new 出來、永遠不刪的登記表：TTimerEntry 可能是靜態物件（建構順序不定，記憶 ht9045-v906-homecoming-siof-lottery），
// 第一個登記的人建出它；程式結束時也不拆，免得晚解構的 TTimerEntry 登出到一張已經拆掉的表。
std::vector<TTimerEntry*>& Registry()
{
    static std::vector<TTimerEntry*>* r = new std::vector<TTimerEntry*>();
    return *r;
}

unsigned            g_overrunMs = 50;
unsigned long long  g_reportEveryMs = 600000ULL;
unsigned long long  g_nextReportMs = 0;
unsigned long long (*g_clockUs)() = 0;
void               (*g_markHook)(const char*) = 0;
DWORD               g_tickThread = 0;
bool                g_threadErrPrinted = false;
int                 g_tickDepth = 0;

unsigned long long QpcUs()
{
    static LARGE_INTEGER freq = { };
    if (freq.QuadPart == 0) ::QueryPerformanceFrequency(&freq);
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return (unsigned long long)(c.QuadPart / freq.QuadPart) * 1000000ULL
         + (unsigned long long)((c.QuadPart % freq.QuadPart) * 1000000LL / freq.QuadPart);
}

unsigned long long NowUs() { return g_clockUs ? g_clockUs() : QpcUs(); }

bool Registered(const TTimerEntry* t)
{
    const std::vector<TTimerEntry*>& r = Registry();
    for (size_t i = 0; i < r.size(); ++i)
        if (r[i] == t) return true;
    return false;
}

// VCL UpdateTimer 的條件：Enabled、Interval≠0、OnTimer 有指定，三者都成立才有在數。
bool Active(const TTimerEntry* t) { return (bool)t->Enabled && (int)t->Interval > 0 && t->OnTimer != 0; }

// 看「Enabled／Interval／OnTimer 是不是改過」：改過（VCL 的 KillTimer＋SetTimer）就從 nowMs 重新數。
void SyncArming(TTimerEntry* t, unsigned long long nowMs)
{
    const bool changed = t->Enabled.Changes() != t->seenEnabled_ || t->Interval.Changes() != t->seenInterval_
                      || t->OnTimer != t->seenOnTimer_;
    const bool active = Active(t);
    if (changed || active != t->armed_) {
        t->seenEnabled_  = t->Enabled.Changes();
        t->seenInterval_ = t->Interval.Changes();
        t->seenOnTimer_  = t->OnTimer;
        t->armed_ = active;
        if (active) t->nextDueMs_ = nowMs + (unsigned long long)(int)t->Interval;
    }
}

// 外層「正在跑」旗標：不管 OnTimer 怎麼離開（return／例外）都放開。golden 本體自己的 static bRun 旗標不歸這裡管（照抄）。
struct RunningScope {
    explicit RunningScope(TTimerEntry* t) : t_(t) { t_->running_ = true; }
    ~RunningScope() { t_->running_ = false; }
    TTimerEntry* t_;
};

struct DepthScope {
    DepthScope()  { ++g_tickDepth; }
    ~DepthScope() { --g_tickDepth; }
};

void Fire(TTimerEntry* t, unsigned long long nowMs)
{
    TTimerStats& s = t->Stats;
    if (t->running_) {                                                          // 巢狀 Tick：自己還沒跑完（VCL 會再進來一次；golden 靠本體的 bRun 擋）
        ++s.reentrySkips;
        return;
    }
    if (g_markHook) g_markHook(t->Name ? t->Name : "?");
    const unsigned long long t0 = NowUs();
    {
        RunningScope run(t);
        try {
            t->OnTimer();
        } catch (...) {
            ++s.exceptions;
            if (s.exceptions <= 3 || s.exceptions % 100 == 0)
                std::printf("[TIMER] %s 的 OnTimer 丟出例外（第 %u 次）—— golden 由 Application->HandleException 接住、Timer 照樣在；這裡同。golden %s\n",
                            t->Name, s.exceptions, t->GoldenCite ? t->GoldenCite : "?");
        }
    }
    const unsigned long long dur = NowUs() - t0;
    ++s.fires;
    s.totalUs += dur;
    s.lastUs = dur;
    if (dur > s.maxUs) s.maxUs = dur;
    s.lastStartMs = nowMs;
    if (dur > (unsigned long long)g_overrunMs * 1000ULL) {
        ++s.overruns;
        if (s.overruns <= 3 || s.overruns % 100 == 0)
            std::printf("[TIMER] %s 跑了 %llu ms（門檻 %u ms，第 %u 次）—— 主迴圈只有一條執行緒，這段時間 PumpTick 與網頁命令都在等。golden %s\n",
                        t->Name, dur / 1000ULL, g_overrunMs, s.overruns, t->GoldenCite ? t->GoldenCite : "?");
    }
    // OnTimer 裡改了自己的 Enabled／Interval（例 golden main.cpp:25597 `Timer3->Enabled=false;`）：
    //   VCL 在那一刻就重新數；這裡用「開始時間＋耗時」當那一刻。
    if (Registered(t)) SyncArming(t, nowMs + dur / 1000ULL);
}

void AppendJsonString(std::string& out, const char* s)
{
    out += '"';
    for (const char* p = s ? s : ""; *p; ++p) {
        const unsigned char c = (unsigned char)*p;
        if (c == '"' || c == '\\') { out += '\\'; out += (char)c; }
        else if (c < 0x20) { char buf[8]; std::snprintf(buf, sizeof buf, "\\u%04x", c); out += buf; }
        else out += (char)c;
    }
    out += '"';
}

}  // namespace

// -----------------------------------------------------------------------------
TTimerEntry::TTimerEntry(const char* name, bool enabled, int interval, const char* goldenCite, void (*onTimer)())
    : Enabled(enabled), Interval(interval), OnTimer(onTimer), Name(name), GoldenCite(goldenCite), Status(0),
      Stats(), nextDueMs_(0), seenEnabled_(0), seenInterval_(0), seenOnTimer_(onTimer), armed_(false), running_(false)
{
    Registry().push_back(this);
}

TTimerEntry::~TTimerEntry()
{
    std::vector<TTimerEntry*>& r = Registry();
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == this) { r.erase(r.begin() + (long)i); break; }
    }
}

// -----------------------------------------------------------------------------
void TimerTableTick(unsigned long long nowMs)
{
    const DWORD self = ::GetCurrentThreadId();
    if (g_tickThread == 0) g_tickThread = self;
    if (self != g_tickThread) {
        if (!g_threadErrPrinted) {
            g_threadErrPrinted = true;
            std::printf("[TIMER] 錯誤：TimerTableTick 從執行緒 %lu 被呼叫，排程表屬於執行緒 %lu（主迴圈）—— 不跑。"
                        "golden 的 Timer 全在主執行緒，翻過來的本體沒有任何鎖\n",
                        (unsigned long)self, (unsigned long)g_tickThread);
        }
        return;
    }
    DepthScope depth;
    // 拍一張快照再走：OnTimer 可能建立／拆掉表單（登記或登出 Timer）。走到時已經登出的跳過。
    const std::vector<TTimerEntry*> snap = Registry();
    for (size_t i = 0; i < snap.size(); ++i) {
        TTimerEntry* t = snap[i];
        if (!Registered(t)) continue;
        SyncArming(t, nowMs);
        if (!t->armed_) continue;
        if ((long long)(nowMs - t->nextDueMs_) < 0) continue;
        const unsigned long long late = nowMs - t->nextDueMs_;
        if (late > t->Stats.maxLateMs) t->Stats.maxLateMs = late;
        // 下一次排在原本的格子上；已經錯過整格就從現在起算 —— 不補跑（WM_TIMER 會合併；同 wb_serve PumpTick 的 B13 寫法）
        const unsigned long long iv = (unsigned long long)(int)t->Interval;
        t->nextDueMs_ += iv;
        if ((long long)(nowMs - t->nextDueMs_) >= 0) t->nextDueMs_ = nowMs + iv;
        Fire(t, nowMs);
    }
    if (g_reportEveryMs != 0 && g_tickDepth == 1) {
        if (g_nextReportMs == 0) {
            g_nextReportMs = nowMs + g_reportEveryMs;
        } else if ((long long)(nowMs - g_nextReportMs) >= 0) {
            g_nextReportMs = nowMs + g_reportEveryMs;
            std::printf("%s", TimerTableReport().c_str());
            std::fflush(stdout);
        }
    }
}

TTimerEntry* TimerTableFind(const char* name)
{
    if (!name) return 0;
    const std::vector<TTimerEntry*>& r = Registry();
    for (size_t i = r.size(); i-- > 0; )
        if (r[i]->Name && std::string(r[i]->Name) == name) return r[i];
    return 0;
}

int TimerTableCount() { return (int)Registry().size(); }

std::vector<TTimerEntry*> TimerTableAll() { return Registry(); }

void TimerTableUnregister(TTimerEntry* t)
{
    std::vector<TTimerEntry*>& r = Registry();
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == t) { r.erase(r.begin() + (long)i); return; }
    }
}

std::string TimerTableReport()
{
    std::string out;
    const std::vector<TTimerEntry*>& r = Registry();
    char line[512];
    std::snprintf(line, sizeof line, "[TIMER] 排程表 %d 支（超過 %u ms 算太慢）\n", (int)r.size(), g_overrunMs);
    out += line;
    for (size_t i = 0; i < r.size(); ++i) {
        const TTimerEntry* t = r[i];
        const TTimerStats& s = t->Stats;
        std::snprintf(line, sizeof line,
                      "  %-36s %-3s %5d ms  %-7s fires=%llu avg=%lluus max=%lluus late<=%llums over=%u exc=%u skip=%u  golden %s\n",
                      t->Name ? t->Name : "?", (bool)t->Enabled ? "on" : "off", (int)t->Interval,
                      t->OnTimer ? (t->Status ? t->Status : "bound") : "no-body",
                      s.fires, s.fires ? s.totalUs / s.fires : 0ULL, s.maxUs, s.maxLateMs,
                      s.overruns, s.exceptions, s.reentrySkips, t->GoldenCite ? t->GoldenCite : "?");
        out += line;
    }
    return out;
}

std::string TimerTableJson()
{
    std::string out;
    char num[160];
    std::snprintf(num, sizeof num, "{\"overrunMs\":%u,\"timers\":[", g_overrunMs);
    out += num;
    const std::vector<TTimerEntry*>& r = Registry();
    for (size_t i = 0; i < r.size(); ++i) {
        const TTimerEntry* t = r[i];
        const TTimerStats& s = t->Stats;
        if (i) out += ',';
        out += "{\"name\":";
        AppendJsonString(out, t->Name);
        out += ",\"cite\":";
        AppendJsonString(out, t->GoldenCite);
        out += ",\"status\":";
        AppendJsonString(out, t->OnTimer ? (t->Status ? t->Status : "bound") : "no-body");
        std::snprintf(num, sizeof num, ",\"enabled\":%s,\"interval\":%d,\"fires\":%llu,\"avgUs\":%llu,\"maxUs\":%llu,\"lastUs\":%llu,",
                      (bool)t->Enabled ? "true" : "false", (int)t->Interval, s.fires,
                      s.fires ? s.totalUs / s.fires : 0ULL, s.maxUs, s.lastUs);
        out += num;
        std::snprintf(num, sizeof num, "\"lastStartMs\":%llu,\"maxLateMs\":%llu,\"overruns\":%u,\"exceptions\":%u,\"reentrySkips\":%u}",
                      s.lastStartMs, s.maxLateMs, s.overruns, s.exceptions, s.reentrySkips);
        out += num;
    }
    out += "]}";
    return out;
}

void     TimerTableSetReportEveryMs(unsigned long long ms) { g_reportEveryMs = ms; g_nextReportMs = 0; }
void     TimerTableSetOverrunMs(unsigned ms) { g_overrunMs = ms; }
unsigned TimerTableOverrunMs() { return g_overrunMs; }

void TimerTableSetMarkHook(void (*mark)(const char*)) { g_markHook = mark; }

void TimerTableSetUsClockForTest(unsigned long long (*clockUs)()) { g_clockUs = clockUs; }
void TimerTableResetThreadForTest() { g_tickThread = 0; g_threadErrPrinted = false; }

}  // namespace ht9045

void W906_TimerTableTick(unsigned long long nowMs)                   { ht9045::TimerTableTick(nowMs); }

// GetTickCount 的 64 位元延伸（MinGW 6.3 的 w32api 沒有 GetTickCount64；同 wb_serve.cpp 的 W906_TickMs64，那一支在匿名命名空間裡叫不到）。
//   49.7 天回捲時進位；只在主迴圈執行緒呼叫（TimerTableTick 本來就只准那條執行緒）。
void W906_TimerTableTickNow()
{
    static DWORD last = 0;
    static unsigned long long high = 0;
    const DWORD t = ::GetTickCount();
    if (t < last) high += 0x100000000ULL;
    last = t;
    ht9045::TimerTableTick(high + t);
}
void W906_TimerTableSetMarkHook(void (*mark)(const char* timerName)) { ht9045::TimerTableSetMarkHook(mark); }
void W906_TimerTableReportNow() { std::printf("%s", ht9045::TimerTableReport().c_str()); std::fflush(stdout); }
