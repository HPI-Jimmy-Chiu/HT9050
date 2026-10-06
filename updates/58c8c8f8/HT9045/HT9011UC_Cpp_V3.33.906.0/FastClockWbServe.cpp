// ===========================================================================
//  FastClockWbServe.cpp  --  AI(W906-FASTCLK) 20261003: the serve loop's side of the fast clock.  wb_serve only.
//
//  RULINGS_20261002 #7 (NIGHT_REPORT s0 #35 = A, user 1002 09:1x): one shared fast-clock hook in tools/wb_serve.cpp's serve
//  loop -- heater 20 ms (golden THeaterThread), St02's bin-number panel 30 ms (E-T1-022), GM-2 ground monitor 30 ms -- each
//  job's longest / average time recorded, and one pass of the loop measured in the SIM build first.
//
//  TWO HOOKS in tools/wb_serve.cpp, both same-line (no line of that file moves):
//    W906_FastClockClampNext(next, n0)  in the sleep block, right after `const DWORD n0 = ::GetTickCount();` -- the loop sleeps
//                                       to the earlier of its own deadline (PumpTick 500 ms / 1203 IO 200 ms, 50 ms cap) and the
//                                       next fast-clock job, so a 20 ms job is not stuck behind the 50 ms cap.  It also ENDS
//                                       the pass meter (the work of the previous pass is done when the loop computes its sleep).
//    W906_FastClockPassBegin()          first statement after the sleep (the line that starts with W906_NativeFormsPump):
//                                       BEGINS the pass meter, then Service() runs every due job on this thread.  On a pump
//                                       beat that is before PumpTick, so the heater body still runs ahead of MainProc as the
//                                       S-12 HeaterSimTick did inside PumpTick.
//  The clock and the jobs are created on the first PassBegin, so the first heater run is 20 ms into the loop (golden: the
//  thread is resumed at FormShow :10138 and does nothing until InitialOK, :10464 -- here InitialOK is already true).
//
//  IN THE BLOCKING BOXES TOO.  AI(W906-FASTCLK-MODAL) 20261003 (NIGHT_REPORT E-FT1-001, "框開著時照樣...查加熱"): while an
//  alarm / yes-no / ShowMyMessage box waits, the main loop sits in that box's wait loop (tools/wb_serve.cpp :537 / :806 /
//  :6760, each pass `wait <= 100 ms` then W906_ModalWaitTick) and never reaches the two hooks above.  golden keeps all three
//  jobs running then: the heater thread's Synchronize and both TTimers (TfMain::Timer1, TfGroundMan::Timer1) are served by
//  ShowModal's message loop.  So the box's loop gets two more same-line hooks (no line of wb_serve.cpp moves):
//    W906_FastClockModalWait(*g_pumpQueue, 100)  replaces the three `g_pumpQueue->waitForPush(100)`: the same wait (a command
//                                       still wakes it at once, 100 ms at most), sliced at the clock's due times, each due job
//                                       run in between (FastClock.h WaitServicing) -- so the heater keeps 20 ms and the panels
//                                       30 ms while everything else in the box's loop keeps its <= 100 ms pass.
//    W906_FastClockService()            in W906_ModalWaitTick's first line: one more Service() per pass (a pass that skipped
//                                       the wait for a carried command still serves a due job) + the 10 s report.
//  Re-entry: a job whose own body opened the box (SHIP heater: CheckHeater -> ShowErrorMessage) is on the stack and is
//  skipped in the box, its due time untouched (FastClock.h POLICY = golden: the heater thread is blocked inside Synchronize
//  until the body returns; Timer1Timer has bRunTimer1); the other jobs run in the box as VCL timers fire inside ShowModal.
//  ONE bin-panel runner: once this clock owns the bin panel, St02's dispatcher runs Timer1BinTick on neither of its paths --
//  the PumpTick pass nor the boxes' pass (MainTimersSt02.cpp, AI(W906-FASTCLK-MODAL) at its call); before the clock starts
//  (a box during boot) the dispatcher keeps it, as before.
//
//  WHY kRate FOR THE HEATER.  golden's loop is "body, then MySleepEx(20)" (fixed delay), on a process that called
//  timeBeginPeriod(1) (golden uruncontrol.cpp:45; gated in the port, uruncontrol.cpp GATE (1)).  wb_serve keeps the default
//  timer resolution (measured 20261003 07:06 on this laptop, a scratch probe: GetTickCount steps 16 ms; WaitForSingleObject
//  (20 ms) returns after 31.0-35.0 ms, median 31.6, (4 ms) after 15.8; with timeBeginPeriod(1): 20.1 / 4.5), so a fixed
//  delay of 20 ms would run the heater at ~32 Hz and stretch every COUNT in its body --
//  DoHeaterOn's iHeaterFanSameCount>=1000 (WAR1637 "Heater fan can not run", golden csystem.cpp:1155-1159) is 20 s at
//  golden's 50 Hz.  kRate keeps golden's 50 Hz on average: single gaps vary with the wake granularity, a beat more than a
//  whole period late is dropped and counted, never burst.  (FastClock.h keeps kDelay; it is the literal shape if the 1 ms
//  timer resolution is ever turned on.)
//  AI(W906-TIMERRES) 20261003: it is on now -- wb_serve calls W906_RunControlExecuteBegin (uruncontrol.cpp, golden :45
//  timeBeginPeriod(1)) right after PumpInit, before the first pass of this clock, so the waits above wake within ~1 ms of
//  the due time.  The heater stays kRate (exactly golden's 50 per s on average); switching it to golden's literal kDelay
//  ("body, then MySleepEx(20)", a little under 50 per s) is left to the user (the [FASTCLK] lines show both rates' inputs).
//
//  REPORT.  One "[FASTCLK] ..." line per 10 s through ht9045::StreamEmit (the [STREAM] line's channel: console unless
//  W906_STREAM_STATS=0, plus the op log when W906_OPLOG_DIR is set): the pass meter (passes, busy avg / max, how many
//  >= 20 / 30 / 100 ms, average sleep) and per job runs, avg / max body ms, overruns (body > period), late runs (>= one period
//  late), missed beats, worst lateness, re-entry skips, exceptions; plus the cooling fan's FormLock-busy skips
//  (FileRW/_ProxyTry.cpp).  At start one line says which jobs are registered and which heater body this build runs.
// ===========================================================================
#include "FastClock.h"
#include "MachineType.h"            // W906_FASTCLK_HEATER, SOFT_SIMULTE
#include "WebStreamStats.h"         // StreamEmit
#include "forms/fGroundMan.h"       // fGroundMan->Timer1->Interval (golden :134 / GroundMan.dfm:1877)
#include "WebBridge/CommandQueue.h" // AI(W906-FASTCLK-MODAL) 20261003: the boxes' wait (W906_FastClockModalWait)

#include <cstdio>
#include <string>

namespace ht9045 {
void W906_FastClockHeaterBeat();           // FastClockJobs.cpp
void W906_FastClockGroundManBeat();        // FastClockJobs.cpp
void W906_FastClockSetOwnsHeater(bool);    // FastClockJobs.cpp
void W906_FastClockSetOwnsBin(bool);       // FastClockJobs.cpp
void W906_St02Timer1BinFast();             // MainTimersSt02.cpp
}
unsigned long FileRW_ProxyTryBusyCount();  // FileRW/_ProxyTry.cpp
void W906_PadFastClockAdd(ht9045::fastclock::FastClock* clock, std::string& jobs);  void W906_SafePlcFastClockAdd(ht9045::fastclock::FastClock* clock, std::string& jobs);   //AI(W906-SAFEPLC) 20261006: MyPLC/SafePlcClock_St02.cpp (golden TPLCIOThread), same line   // AI(W906-ST02-P1) 20261005 (St02-E): PadInterface_St02.cpp (golden TPadRS232Thread); global, outside the anonymous namespace. On the old blank line
namespace {

using ht9045::fastclock::FastClock;
using ht9045::fastclock::PassMeter;

FastClock*    g_clock = 0;
PassMeter     g_pass;
std::uint64_t g_windowT0 = 0;
unsigned long g_busyT0   = 0;
const std::uint64_t kReportUs = 10000000ull;          // the [STREAM] window (WebStreamStats.h)
unsigned long long g_boxPasses = 0;                   // AI(W906-FASTCLK-MODAL) 20261003: W906_FastClockService calls (= box-loop passes) in the window

std::string Ms(unsigned long long us)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%.3f", us / 1000.0);
    return b;
}

void Start()
{
    g_clock = new FastClock();
    std::string jobs;
#ifdef W906_FASTCLK_HEATER
    const int h = g_clock->Add("heater", 20, ht9045::fastclock::kRate, &ht9045::W906_FastClockHeaterBeat);   // golden uHeaterThread.cpp:73 MySleepEx(20)
    ht9045::W906_FastClockSetOwnsHeater(h >= 0);
#ifdef SOFT_SIMULTE
    jobs += "heater 20 ms (SIM body: HeaterSimTick, no DoThermo; PumpTick no longer calls it)";
#else
    jobs += "heater 20 ms (SHIP body: golden HeaterThreadProcess incl. DoThermo -- temperature controllers + heater relay)";
#endif
#else
    jobs += "heater OFF (MachineType.h W906_FASTCLK_HEATER not defined: SIM keeps HeaterSimTick in PumpTick, SHIP has none)";
#endif
    const int b = g_clock->Add("binpanel", 30, ht9045::fastclock::kRate, &ht9045::W906_St02Timer1BinFast);       // golden main.dfm:17274 Timer1 Interval 30
    ht9045::W906_FastClockSetOwnsBin(b >= 0);
    jobs += ", binpanel 30 ms (TfMain::Timer1 NUMBER_PANEL_TYPE==2)";
    const unsigned gmMs = (fGroundMan != 0 && fGroundMan->Timer1->Interval > 0) ? (unsigned)fGroundMan->Timer1->Interval : 30u;
    g_clock->Add("groundman", gmMs, ht9045::fastclock::kRate, &ht9045::W906_FastClockGroundManBeat);              // golden GroundMan.dfm:1877 / ctor :134
    jobs += ", groundman " + std::to_string(gmMs) + " ms (TfGroundMan::Timer1Timer; DoGroundMasterMonitor stays GATE GM-2)";  ::W906_PadFastClockAdd(g_clock, jobs);  ::W906_SafePlcFastClockAdd(g_clock, jobs);   //AI(W906-SAFEPLC) 20261006: + "safeplc" 1 ms kDelay = golden TPLCIOThread (Synchronize(PLCIOProcess) + MySleepEx(1)), only when SafePlcIO=1; also runs inside the boxes (golden mymessbox / note bPLCStatusCheck)   // AI(W906-ST02-P1) 20261005 (St02-E): + "pad" 1 ms kDelay = golden TPadRS232Thread (Synchronize(Main232) + SleepEx(1)), only when ControlPanelMode==1 (golden main.cpp:22479 / :10142); also runs inside the boxes like golden mymessbox / note's Main232 pump
    g_windowT0 = g_clock->NowUs();
    g_busyT0   = FileRW_ProxyTryBusyCount();
    ht9045::StreamEmit("[FASTCLK] serve-loop fast clock started: " + jobs);
}

std::string U(unsigned long long v) { return std::to_string(v); }   // no %llu: msvcrt dialect (WebStreamStats.cpp)

void Report(std::uint64_t now)
{
    const PassMeter::Window p = g_pass.Take();
    const double secs = (now - g_windowT0) / 1e6;
    char rate[64];
    std::snprintf(rate, sizeof(rate), "%.1fs", secs);
    std::string line = std::string("[FASTCLK] ") + rate + " passes=" + U(p.passes);
    std::snprintf(rate, sizeof(rate), " (%.1f/s)", secs > 0 ? p.passes / secs : 0.0);
    line += rate;
    line += " busy avg=" + Ms(p.passes ? p.busyUs / p.passes : 0) + " max=" + Ms(p.maxBusyUs) + " ms >=20ms=" + U(p.over20ms)
          + " >=30ms=" + U(p.over30ms) + " >=100ms=" + U(p.over100ms) + " sleep avg=" + Ms(p.passes ? p.sleepUs / p.passes : 0) + " ms";
    for (std::size_t i = 0; i < g_clock->Size(); ++i) {
        const ht9045::fastclock::Stats s = g_clock->TakeWindow((int)i);
        const ht9045::fastclock::JobInfo& ji = g_clock->Info((int)i);
        line += " | " + ji.name + " " + std::to_string(ji.periodMs) + "ms n=" + U(s.runs) + " avg=" + Ms(s.runs ? s.busyUs / s.runs : 0)
              + " max=" + Ms(s.maxUs) + " ms over=" + U(s.overruns) + " late=" + U(s.lateRuns) + " missed=" + U(s.missedBeats)
              + " maxLate=" + Ms(s.maxLateUs) + " ms reent=" + U(s.reentrySkips) + " exc=" + U(s.exceptions);
    }
    const unsigned long busy = FileRW_ProxyTryBusyCount();
    line += " | formlock-busy skips=" + std::to_string(busy - g_busyT0);
    line += " | box passes=" + U(g_boxPasses);                     // AI(W906-FASTCLK-MODAL) 20261003: > 0 = a blocking box was up in this window (the jobs above ran in it too)
    g_boxPasses = 0;
    g_busyT0   = busy;
    g_windowT0 = now;
    ht9045::StreamEmit(line);
}

// AI(W906-FASTCLK-MODAL) 20261003: the boxes' wait (W906_FastClockModalWait) -- CommandQueue::waitForPush as a WaitFn.
bool QueueWait(void* q, unsigned long ms)
{
    return static_cast<webbridge::CommandQueue*>(q)->waitForPush(ms);
}

}  // namespace

// tools/wb_serve.cpp serve loop: the first statement after the sleep (header).
void W906_FastClockPassBegin()
{
    if (g_clock == 0)
        Start();
    g_pass.Begin();
    g_clock->Service();
    const std::uint64_t now = g_clock->NowUs();
    if (now - g_windowT0 >= kReportUs)
        Report(now);
}

// tools/wb_serve.cpp serve loop sleep block (header): the earlier of the loop's deadline and the next job; ends the pass meter.
unsigned long W906_FastClockClampNext(unsigned long nextTick, unsigned long nowTick)
{
    g_pass.End();
    if (g_clock == 0)
        return nextTick;
    return (unsigned long)ht9045::fastclock::ClampTickDeadline((std::uint32_t)nextTick, (std::uint32_t)nowTick,
                                                               g_clock->NowUs(), g_clock->NextDueUs());
}

// For the blocking boxes' wait loop (header, E-FT1-001): run the due jobs, no pass metering; the 10 s report goes on while a box
// is up.  AI(W906-FASTCLK-MODAL) 20261003: hooked -- tools/wb_serve.cpp W906_ModalWaitTick, its first line (once per box pass).
void W906_FastClockService()
{
    if (g_clock == 0)
        return;                                                    // a box before the first serve-loop pass: nothing registered yet
    g_clock->Service();
    ++g_boxPasses;
    const std::uint64_t now = g_clock->NowUs();
    if (now - g_windowT0 >= kReportUs)
        Report(now);
}

// AI(W906-FASTCLK-MODAL) 20261003: the blocking boxes' wait (header, E-FT1-001) -- tools/wb_serve.cpp :537 / :806 / :6760, in
// place of `g_pumpQueue->waitForPush(100)`: the same wait (a command wakes it at once; capMs at most), sliced at the jobs' due
// times with each due job run in between (FastClock.h WaitServicing).  Before the clock starts: q.waitForPush(capMs), as before.
bool W906_FastClockModalWait(webbridge::CommandQueue& q, unsigned long capMs)
{
    return ht9045::fastclock::WaitServicing(g_clock, capMs, &QueueWait, &q);
}

// One job's totals since the start, by name (the ctest FastClk_Jobs; a later status tag can use it too).  false = no such job
// (or the clock has not started yet: no pass has run).
bool W906_FastClockJob(const char* name, unsigned* periodMs, unsigned long long* runs, unsigned long long* busyUs, unsigned long long* maxUs)
{
    if (g_clock == 0 || name == 0)
        return false;
    for (std::size_t i = 0; i < g_clock->Size(); ++i) {
        if (g_clock->Info((int)i).name != name)
            continue;
        const ht9045::fastclock::Stats& s = g_clock->Total((int)i);
        if (periodMs) *periodMs = g_clock->Info((int)i).periodMs;
        if (runs)     *runs     = s.runs;
        if (busyUs)   *busyUs   = s.busyUs;
        if (maxUs)    *maxUs    = s.maxUs;
        return true;
    }
    return false;
}
