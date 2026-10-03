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
//  NOT IN THE BLOCKING BOXES.  While an alarm / yes-no / ShowMyMessage box waits, the main loop sits in its wait loop
//  (tools/wb_serve.cpp W906_ModalWaitTick) and this clock is not serviced -- the same as PumpTick today.  golden keeps all
//  three running then (the heater's Synchronize and both TTimers are served by ShowModal's message loop).  That belongs to
//  the open-box work (NIGHT_REPORT E-FT1-001, "框開著時照樣...查加熱"): ONE call of W906_FastClockService() in
//  W906_ModalWaitTick covers all three -- re-entry is safe (a job whose own body opened the box is skipped, FastClock.h),
//  and the bin panel cannot fire twice (St02's modal path and this job share golden's own 20 ms dwOldTime guard,
//  MainTimersSt02.cpp).  Until then St02's dispatcher still runs the bin panel inside the boxes (its <= 100 ms modal pass).
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

namespace {

using ht9045::fastclock::FastClock;
using ht9045::fastclock::PassMeter;

FastClock*    g_clock = 0;
PassMeter     g_pass;
std::uint64_t g_windowT0 = 0;
unsigned long g_busyT0   = 0;
const std::uint64_t kReportUs = 10000000ull;          // the [STREAM] window (WebStreamStats.h)

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
    jobs += ", groundman " + std::to_string(gmMs) + " ms (TfGroundMan::Timer1Timer; DoGroundMasterMonitor stays GATE GM-2)";
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
    g_busyT0   = busy;
    g_windowT0 = now;
    ht9045::StreamEmit(line);
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

// For the blocking boxes' wait loop (header, E-FT1-001): run the due jobs, no pass metering.  Not hooked yet.
void W906_FastClockService()
{
    if (g_clock != 0)
        g_clock->Service();
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
