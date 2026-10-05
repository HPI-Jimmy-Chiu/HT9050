// =============================================================================
//  tests/test_fastclock.cpp -- AI(W906-FASTCLK) 20261003.  Suite: FastClock.  Pure (FastClock.cpp only, a fake clock), -Wall -Wextra.
//
//  The serve loop's shared fast clock (FastClock.h; RULINGS_20261002 #7, s0 #35 = A), its POLICY one rule at a time:
//    1. kRate on a perfect clock: first run one period after Add, then exactly one per period (20 ms -> 50 per s,
//       30 ms -> 33 per s), in Add order when due together.
//    2. kRate, late: woken 7 ms late every time -> still 50 per s (the grid holds, single gaps vary); 45 ms late once on a
//       20 ms job -> ONE run (no burst), 2 missed beats counted, a late run counted, re-gridded from now; maxLate = 45 ms.
//    3. kDelay: the next run is due one period after the body ENDED (a 5 ms body + 20 ms -> every 25 ms); no missed beats.
//    4. stats: runs, total / max body, overruns (body > period), per-window TakeWindow resets only the window.
//    5. re-entry: a body that calls Service() again (a waiting box inside it) -- itself is skipped (due untouched,
//       reentrySkips +1), the other due job runs inside; after the return the outer loop does not run that one twice.
//    6. an exception out of a body is caught and counted; the next job and the next beat still run.
//    7. NextDueUs: the earliest due of the jobs not running; UINT64_MAX with none.  ClampTickDeadline: the earlier of the
//       loop's tick deadline and the job's (rounded up to whole ms), wrap-safe across the 49.7-day GetTickCount wrap.
//    8. PassMeter: busy = End - Begin, sleep = next Begin - End, the >= 20 / 30 / 100 ms buckets, End before any Begin ignored.
//    9. Add refuses period 0 / no body.
//   10. AI(W906-FASTCLK-MODAL) 20261003 (E-FT1-001): WaitServicing, a blocking box's wait -- no clock = one plain wait; 100 ms
//       with no command = 100 ms in all, sliced at each due job, every job on its period (20 ms -> 5, 30 ms -> 3); a command
//       returns at once (true); early wakes never run a job early; a body that opens its own box is not re-entered (skipped
//       on each beat inside, reentrySkips 3) while the other job runs inside on its period and not again after the box.
//  CONTROL (run 20261003): FastClock.cpp with the kRate re-grid replaced by `due += period` (catch-up) turns 2 red.
// =============================================================================
#include "FastClock.h"

#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace ht9045::fastclock;

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- fake clock --------------------------------------------------------------------------------------------------------------
static std::uint64_t g_now = 1000000;       // us
static std::uint64_t Now() { return g_now; }

static std::vector<int> g_log;              // which job ran, in order
static std::uint64_t g_bodyUs = 0;          // how long a body "takes" (advances the fake clock)
static void BodyA() { g_log.push_back(0); g_now += g_bodyUs; }
static void BodyB() { g_log.push_back(1); g_now += g_bodyUs; }

static FastClock* g_re = 0;                 // for the re-entry case
static bool g_reBox = false;
static void ReBody()                        // job 0 of g_re: the first time, opens "a box" that services the clock once
{
    g_log.push_back(10);
    if (g_reBox) {
        g_reBox = false;
        g_now += 40000;                     // the box stays up 40 ms: job 0 (20 ms) and job 1 (30 ms) both fall due
        g_re->Service();
    }
}
static void ReOther() { g_log.push_back(11); }

static void Throws() { g_log.push_back(20); throw std::runtime_error("boom"); }

// ---- section 10: a blocking box's wait (WaitServicing; AI(W906-FASTCLK-MODAL) 20261003) ------------------------------------
static std::vector<unsigned long> g_waits;  // the ms each wait was asked for
static std::uint64_t g_pushAt = 0;          // fake-clock time a command arrives (0 = none)
static std::uint64_t g_shortUs = 0;         // each wait returns this much early, no command (an early wake)
static bool FakeWait(void*, unsigned long ms)   // CommandQueue::waitForPush(ms) on the fake clock
{
    g_waits.push_back(ms);
    const std::uint64_t until = g_now + (std::uint64_t)ms * 1000ull;
    if (g_pushAt != 0 && g_pushAt <= until) {
        if (g_pushAt > g_now) g_now = g_pushAt;
        g_pushAt = 0;
        return true;
    }
    g_now = until - (g_shortUs < (std::uint64_t)ms * 1000ull ? g_shortUs : 0);
    return false;
}
static FastClock* g_box = 0;                // the clock a body's box waits on
static bool g_openBox = false;              // the body opens its box the first time only
static int g_depth = 0, g_maxDepth = 0;     // the body's nesting
static void BoxBody()                       // golden SHIP heater: CheckHeater -> ShowErrorMessage -> a box that waits 100 ms
{
    ++g_depth;
    if (g_depth > g_maxDepth) g_maxDepth = g_depth;
    g_log.push_back(30);
    if (g_openBox) {
        g_openBox = false;
        WaitServicing(g_box, 100, &FakeWait, 0);
    }
    --g_depth;
}
static void BoxOther() { g_log.push_back(31); }

static int Count(int id) { int n = 0; for (std::size_t i = 0; i < g_log.size(); ++i) if (g_log[i] == id) ++n; return n; }

int main()
{
    std::printf("FastClock\n");

    std::printf(" 1. kRate, perfect clock\n");
    {
        g_now = 1000000; g_log.clear(); g_bodyUs = 0;
        FastClock c(&Now);
        const int a = c.Add("a", 20, kRate, &BodyA);
        const int b = c.Add("b", 30, kRate, &BodyB);
        CHECK(a == 0 && b == 1 && c.Size() == 2);
        CHECK(c.DueUs(a) == 1020000 && c.DueUs(b) == 1030000);        // one period after Add
        c.Service();
        CHECK(g_log.empty());                                          // nothing due at the start
        for (int ms = 1; ms <= 1000; ++ms) { g_now += 1000; c.Service(); }
        CHECK(Count(0) == 50 && Count(1) == 33);                       // 1000 / 20, floor(1000 / 30)
        CHECK(c.Total(a).runs == 50 && c.Total(a).lateRuns == 0 && c.Total(a).missedBeats == 0 && c.Total(a).maxLateUs == 0);
        g_log.clear();
        FastClock d(&Now);
        d.Add("a", 30, kRate, &BodyA);
        d.Add("b", 30, kRate, &BodyB);
        g_now += 30000; d.Service();
        CHECK(g_log.size() == 2 && g_log[0] == 0 && g_log[1] == 1);    // due together -> Add order
    }

    std::printf(" 2. kRate, late wakes: the grid holds, no burst\n");
    {
        g_now = 5000000; g_log.clear(); g_bodyUs = 0;
        FastClock c(&Now);
        const int a = c.Add("a", 20, kRate, &BodyA);
        // the loop wakes 7 ms after every due time (a coarse timer)
        for (int k = 0; k < 50; ++k) { g_now = c.DueUs(a) + 7000; c.Service(); }
        CHECK(Count(0) == 50);
        CHECK(c.DueUs(a) == 5000000 + 51ull * 20000);                  // still on the start + k*20 ms grid
        CHECK(c.Total(a).lateRuns == 0 && c.Total(a).missedBeats == 0 && c.Total(a).maxLateUs == 7000);
        // one 45 ms stall
        g_log.clear();
        const std::uint64_t due = c.DueUs(a);
        g_now = due + 45000;
        c.Service();
        CHECK(Count(0) == 1);                                          // one run, not three
        CHECK(c.Total(a).missedBeats == 2 && c.Total(a).lateRuns == 1 && c.Total(a).maxLateUs == 45000);
        CHECK(c.DueUs(a) == g_now + 20000);                            // re-gridded from now
        c.Service();
        CHECK(Count(0) == 1);                                          // and nothing more right now
    }

    std::printf(" 3. kDelay: due one period after the body ended\n");
    {
        g_now = 9000000; g_log.clear(); g_bodyUs = 5000;
        FastClock c(&Now);
        const int a = c.Add("a", 20, kDelay, &BodyA);
        std::vector<std::uint64_t> starts;
        for (int ms = 1; ms <= 200; ++ms) {
            g_now = 9000000 + (std::uint64_t)ms * 1000;
            const std::size_t before = g_log.size();
            const std::uint64_t t = g_now;
            c.Service();
            if (g_log.size() != before) starts.push_back(t);
        }
        bool every25 = starts.size() >= 2;
        for (std::size_t i = 1; i < starts.size(); ++i) every25 = every25 && starts[i] - starts[i - 1] == 25000;
        CHECK(!starts.empty() && starts[0] == 9020000);                 // first: one period after Add
        CHECK(every25);                                                // 5 ms body + 20 ms
        CHECK(c.Total(a).missedBeats == 0);
        g_bodyUs = 0;
    }

    std::printf(" 4. stats and the report window\n");
    {
        g_now = 20000000; g_log.clear();
        FastClock c(&Now);
        const int a = c.Add("a", 20, kRate, &BodyA);
        g_bodyUs = 3000; g_now = c.DueUs(a); c.Service();
        g_bodyUs = 25000; g_now = c.DueUs(a); c.Service();             // 25 ms body on a 20 ms job: an overrun
        const Stats w = c.TakeWindow(a);
        CHECK(w.runs == 2 && w.busyUs == 28000 && w.maxUs == 25000 && w.overruns == 1);
        CHECK(c.TakeWindow(a).runs == 0);                              // the window resets
        CHECK(c.Total(a).runs == 2 && c.Total(a).overruns == 1);       // the total does not
        g_bodyUs = 0;
    }

    std::printf(" 5. re-entry (a waiting box inside a body)\n");
    {
        g_now = 30000000; g_log.clear(); g_reBox = true;
        FastClock c(&Now);
        g_re = &c;
        const int a = c.Add("heater", 20, kRate, &ReBody);
        const int b = c.Add("timer", 30, kRate, &ReOther);
        g_now = c.DueUs(a);                                            // 20 ms: only the first is due
        c.Service();
        // inside: +40 ms -> 'heater' due again but on the stack (skipped), 'timer' due (ran once inside)
        CHECK(Count(10) == 1 && Count(11) == 1);
        CHECK(c.Total(a).reentrySkips == 1 && c.Total(b).runs == 1);
        CHECK(!c.Running(a) && !c.Running(b));
        CHECK(c.DueUs(a) == 30000000 + 40000);                         // its grid point taken BEFORE the body (POLICY)
        const std::size_t n = g_log.size();
        c.Service();                                                   // 'timer' (due +30 ms from inside) does not run twice
        CHECK(g_log.size() == n + 1 && g_log.back() == 10);            // only 'heater', due again since the box (no box this time)
        g_re = 0;
    }

    std::printf(" 6. an exception is caught and counted\n");
    {
        g_now = 40000000; g_log.clear(); g_bodyUs = 0;
        FastClock c(&Now);
        const int t = c.Add("throws", 20, kRate, &Throws);
        const int a = c.Add("a", 20, kRate, &BodyA);
        g_now += 20000; c.Service();
        g_now += 20000; c.Service();
        CHECK(Count(20) == 2 && Count(0) == 2);
        CHECK(c.Total(t).exceptions == 2 && c.Total(t).runs == 2 && c.Total(a).exceptions == 0);
        CHECK(!c.Running(t));
    }

    std::printf(" 7. NextDueUs / ClampTickDeadline\n");
    {
        g_now = 50000000;
        FastClock e(&Now);
        CHECK(e.NextDueUs() == kNever);
        FastClock c(&Now);
        c.Add("a", 30, kRate, &BodyA);
        c.Add("b", 20, kRate, &BodyB);
        CHECK(c.NextDueUs() == 50020000);
        // the loop: tick 1000, its own deadline 1050 (50 ms cap / PumpTick); a job due in 20.4 ms -> 1021 (rounded up)
        CHECK(ClampTickDeadline(1050u, 1000u, 50000000ull, 50020400ull) == 1021u);
        CHECK(ClampTickDeadline(1010u, 1000u, 50000000ull, 50020000ull) == 1010u);      // the loop's own is earlier
        CHECK(ClampTickDeadline(1050u, 1000u, 50000000ull, 49990000ull) == 1000u);      // overdue -> now
        CHECK(ClampTickDeadline(1050u, 1000u, 50000000ull, kNever) == 1050u);   // no job
        CHECK(ClampTickDeadline(0x00000010u, 0xFFFFFFF0u, 50000000ull, 50020000ull) == 0x00000004u);   // across the wrap: 0xFFFFFFF0 + 20
        CHECK(ClampTickDeadline(0x00000002u, 0xFFFFFFF0u, 50000000ull, 50020000ull) == 0x00000002u);   // the loop's is earlier, across the wrap
    }

    std::printf(" 8. PassMeter\n");
    {
        g_now = 60000000;
        PassMeter p(&Now);
        p.End();                                                       // before any Begin: ignored
        p.Begin(); g_now += 2000;  p.End();                            // 2 ms
        g_now += 18000;                                                // sleep 18 ms
        p.Begin(); g_now += 25000; p.End();                            // 25 ms
        g_now += 5000;
        p.Begin(); g_now += 120000; p.End();                           // 120 ms
        const PassMeter::Window w = p.Take();
        CHECK(w.passes == 3 && w.busyUs == 147000 && w.maxBusyUs == 120000);
        CHECK(w.over20ms == 2 && w.over30ms == 1 && w.over100ms == 1);
        CHECK(w.sleepUs == 23000);
        CHECK(p.Take().passes == 0);
    }

    std::printf(" 9. Add refuses period 0 / no body\n");
    {
        FastClock c(&Now);
        CHECK(c.Add("zero", 0, kRate, &BodyA) == -1 && c.Add("none", 20, kRate, 0) == -1 && c.Size() == 0);
    }

    std::printf("10. WaitServicing: a blocking box's wait serves the clock (E-FT1-001)\n");
    {
        // (a) no clock yet (a box during boot): one plain wait of capMs, its answer passed back
        g_now = 70000000; g_log.clear(); g_waits.clear(); g_pushAt = 0; g_shortUs = 0; g_bodyUs = 0;
        CHECK(WaitServicing(0, 100, &FakeWait, 0) == false && g_waits.size() == 1 && g_waits[0] == 100 && g_now == 70100000);
        // (b) 100 ms, no command: the wait stops at each due job, the jobs keep their period, 100 ms in all
        g_waits.clear();
        FastClock c(&Now);
        const int a = c.Add("heater", 20, kRate, &BodyA);
        const int b = c.Add("panel", 30, kRate, &BodyB);
        const std::uint64_t t0 = g_now;
        CHECK(WaitServicing(&c, 100, &FakeWait, 0) == false);
        CHECK(g_now == t0 + 100000);                                   // waitForPush(100)'s 100 ms
        CHECK(Count(0) == 5 && Count(1) == 3);                         // +20/40/60/80/100 and +30/60/90
        CHECK(c.Total(a).missedBeats == 0 && c.Total(a).lateRuns == 0 && c.Total(b).missedBeats == 0);
        unsigned long sum = 0; bool positive = true, sliced = true;
        for (std::size_t i = 0; i < g_waits.size(); ++i) { sum += g_waits[i]; positive = positive && g_waits[i] > 0; sliced = sliced && g_waits[i] <= 20; }
        CHECK(sum == 100 && positive && sliced);                       // sliced at the due times, never a 0 ms spin
        // (c) a command 45 ms in: back at once (true), the jobs due before it ran (+120: both; +140: heater)
        g_log.clear(); g_waits.clear();
        const std::uint64_t t1 = g_now;
        g_pushAt = t1 + 45000;
        CHECK(WaitServicing(&c, 100, &FakeWait, 0) == true && g_now == t1 + 45000);
        CHECK(Count(0) == 2 && Count(1) == 1);
        // (d) every wait comes back 1 ms early: a job never runs before its due time, one more 1 ms wait instead
        g_log.clear(); g_waits.clear(); g_shortUs = 1000;
        CHECK(WaitServicing(&c, 100, &FakeWait, 0) == false);
        CHECK(Count(0) == 5 && Count(1) == 4);                         // heater +160..+240, panel +150..+240 (box +145..+245)
        CHECK(c.Total(a).maxLateUs == 0 && c.Total(b).maxLateUs == 0); // on time, never early (Service runs only what is due)
        CHECK(g_waits.size() <= 2 * 7 + 2);                            // 7 due points x (wait, 1 ms more) + the end
        g_shortUs = 0;
        // (e) re-entry: the heater's own body opens the box -- the heater is not re-entered, the panel runs inside on its period
        g_now = 80000000; g_log.clear(); g_waits.clear(); g_depth = 0; g_maxDepth = 0;
        FastClock d(&Now);
        g_box = &d; g_openBox = true;
        const int h = d.Add("heater", 20, kRate, &BoxBody);
        const int p = d.Add("panel", 30, kRate, &BoxOther);
        g_now = d.DueUs(h);                                            // +20: the heater runs and opens its box until +120
        d.Service();
        CHECK(Count(30) == 1 && g_maxDepth == 1 && !d.Running(h));     // once, never nested
        CHECK(d.Total(h).reentrySkips == 3);                           // due inside its own box at +60 / +90 / +120: skipped
        CHECK(Count(31) == 4 && d.Total(p).runs == 4);                 // +30 / +60 / +90 / +120 inside the box; not again after it
        CHECK(g_waits.size() == 4);                                    // the box woke for the panel only (the heater is on the stack)
        g_box = 0;
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
