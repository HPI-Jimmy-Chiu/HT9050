// ===========================================================================
//  FastClock.h  --  AI(W906-FASTCLK) 20261003: the serve loop's shared fast clock.  NOT in golden (infrastructure).
//
//  RULINGS_20261002 #7 (NIGHT_REPORT s0 #35 = A, user 1002 09:1x):
//    「tools/wb_serve.cpp 的 serve loop 加一個共用快時鐘掛點：加熱 20 ms（golden THeaterThread）、St02 的 Bin 號碼面板 30 ms
//     （E-T1-022）、GM-2 接地監測 30 ms 都掛上去；每個掛上去的工作記最長／平均耗時；先在模擬組態量一拍的時間」
//
//  WHY A CLOCK OF ITS OWN.  golden runs these on two things this program does not have: a worker thread that Synchronize()s
//  its body onto the VCL main thread and then sleeps (THeaterThread, uHeaterThread.cpp:68-76), and VCL TTimers fired from the
//  main thread's message loop (TfMain::Timer1 30 ms main.dfm:17272-17278; TfGroundMan::Timer1 30 ms GroundMan.dfm:1876-1881).
//  Both run ON THE MAIN THREAD between other work.  wb_serve's main thread is its serve loop (tools/wb_serve.cpp), whose
//  engine beat is 500 ms (kServeTickMs, ruling B13) -- 25x too slow for the heater.  So the loop gets one scheduler: it asks
//  NextDueUs() when it computes how long to sleep, and calls Service() at the top of every pass; Service() runs each job that
//  is due, on that same thread, in registration order, and times it.
//
//  This header is PURE: no machine code, no Windows call except the default clock in FastClock.cpp.  The jobs and the
//  wb_serve glue are FastClockJobs.cpp (ht9045_sm) and FastClockWbServe.cpp (wb_serve only).
//
//  POLICY (pinned by tests/test_fastclock.cpp):
//    * kRate  = a VCL TTimer: the first run one period after Add(); then a fixed grid (due += period).  A run that starts a
//      whole period or more after its due time re-grids from now (due = now + period) and counts the beats in between as
//      MISSED -- no catch-up burst (Windows coalesces WM_TIMER the same way; St02's MainTimersSt02.cpp Due() and the serve
//      loop's nextPump use the same rule).
//    * kDelay = golden's worker loop `do { Synchronize(body); MySleepEx(period); } while(...)`: the next run is due one period
//      after this one ENDED.  Nothing is "missed"; lateness is still measured.
//    * a job whose body is still on the stack when it falls due again (its body opened a waiting box and that box's loop
//      called Service()) is skipped with its due time untouched: golden's heater thread is blocked inside Synchronize until
//      the body returns, and Timer1Timer / TfGroundMan::Timer1Timer have their own running flag (bRunTimer1).  Other jobs
//      run normally inside that nested Service(), as VCL timers fire inside a ShowModal.
//    * an exception out of a body is caught and counted; the loop goes on (as WebBridgeTags.cpp PumpTick does for MainProc).
//    * per job: runs, total / max body time, OVERRUNS (body longer than its period), LATE runs (started >= one period after
//      due), missed beats, the worst start delay, re-entry skips, exceptions -- cumulative and per report window.
// ===========================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ht9045 {
namespace fastclock {

enum Mode { kRate = 0, kDelay = 1 };

const std::uint64_t kNever = ~std::uint64_t(0);     // "no due time" (NextDueUs with no job; a kDelay job while its body runs)

typedef void (*Body)();
typedef std::uint64_t (*NowUsFn)();                // monotonic microseconds

std::uint64_t DefaultNowUs();                      // QueryPerformanceCounter (FastClock.cpp)

struct Stats {
    unsigned long long runs         = 0;           // bodies that ran
    unsigned long long busyUs       = 0;           // their total time
    unsigned long long maxUs        = 0;           // the longest one
    unsigned long long overruns     = 0;           // body time > period
    unsigned long long lateRuns     = 0;           // started >= one period after due
    unsigned long long missedBeats  = 0;           // kRate beats dropped by the no-catch-up rule
    unsigned long long maxLateUs    = 0;           // worst (start - due)
    unsigned long long reentrySkips = 0;           // due while its own body was still on the stack
    unsigned long long exceptions   = 0;           // bodies that threw
};

struct JobInfo {
    std::string name;
    unsigned    periodMs = 0;
    Mode        mode     = kRate;
};

class FastClock {
public:
    explicit FastClock(NowUsFn now = 0);           // 0 = DefaultNowUs

    // Registers a job; its first run is due one period after now.  periodMs 0 is refused (-1).  Returns the job id.
    int  Add(const char* name, unsigned periodMs, Mode mode, Body body);
    std::size_t Size() const { return jobs_.size(); }

    // Runs every job that is due now, once each, in Add order.  Returns how many bodies ran.  Re-entrant (see POLICY).
    int  Service();

    // The earliest due time of a job that is not running; kNever when there is none.
    std::uint64_t NextDueUs() const;

    const JobInfo& Info(int id) const { return jobs_[(std::size_t)id].info; }
    const Stats&   Total(int id) const { return jobs_[(std::size_t)id].total; }
    Stats          TakeWindow(int id);             // the stats since the last TakeWindow, then resets them
    std::uint64_t  DueUs(int id) const { return jobs_[(std::size_t)id].dueUs; }
    bool           Running(int id) const { return jobs_[(std::size_t)id].running; }
    std::uint64_t  NowUs() const { return now_(); }

private:
    struct Job {
        JobInfo       info;
        Body          body    = 0;
        std::uint64_t dueUs   = 0;
        bool          running = false;
        Stats         total, window;
    };
    NowUsFn          now_;
    std::vector<Job> jobs_;
};

// The serve loop's sleep: given the loop's own GetTickCount() deadline `nextTick` and the clock's next due time, the earlier
// of the two, in the tick domain (wrap-safe, DWORD arithmetic).  dueUs <= nowUs -> nowTick (sleep the loop's 1 ms floor).
std::uint32_t ClampTickDeadline(std::uint32_t nextTick, std::uint32_t nowTick, std::uint64_t nowUs, std::uint64_t dueUs);

// One serve-loop pass, for the measurement the ruling asks for ("先在模擬組態量一拍的時間"): Begin() right after the sleep,
// End() when the loop starts computing its next sleep.  busy = End - Begin; the gap from End to the next Begin is the sleep.
class PassMeter {
public:
    explicit PassMeter(NowUsFn now = 0);
    void Begin();
    void End();
    struct Window {
        unsigned long long passes = 0, busyUs = 0, maxBusyUs = 0, sleepUs = 0;
        unsigned long long over20ms = 0, over30ms = 0, over100ms = 0;   // passes whose busy part was >= 20 / 30 / 100 ms
    };
    Window Take();                                 // since the last Take, then resets
private:
    NowUsFn       now_;
    bool          open_  = false;
    std::uint64_t begin_ = 0, end_ = 0;
    Window        w_;
};

}  // namespace fastclock
}  // namespace ht9045
