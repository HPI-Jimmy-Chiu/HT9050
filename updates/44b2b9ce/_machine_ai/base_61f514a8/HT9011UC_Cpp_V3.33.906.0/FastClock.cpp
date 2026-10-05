// ===========================================================================
//  FastClock.cpp  --  AI(W906-FASTCLK) 20261003.  See FastClock.h (policy, ruling).  Pure: no machine code.
// ===========================================================================
#include "FastClock.h"

#include <windows.h>

namespace ht9045 {
namespace fastclock {

std::uint64_t DefaultNowUs()
{
    static std::uint64_t s_freq = 0;
    if (s_freq == 0) {
        LARGE_INTEGER fr;
        ::QueryPerformanceFrequency(&fr);
        s_freq = (std::uint64_t)fr.QuadPart;
    }
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    const std::uint64_t q = (std::uint64_t)c.QuadPart, f = s_freq;
    return (q / f) * 1000000ull + ((q % f) * 1000000ull) / f;      // no overflow for any realistic uptime
}

FastClock::FastClock(NowUsFn now) : now_(now ? now : &DefaultNowUs) {}

int FastClock::Add(const char* name, unsigned periodMs, Mode mode, Body body)
{
    if (periodMs == 0 || body == 0)
        return -1;
    Job j;
    j.info.name     = name ? name : "?";
    j.info.periodMs = periodMs;
    j.info.mode     = mode;
    j.body          = body;
    j.dueUs         = now_() + (std::uint64_t)periodMs * 1000ull;   // VCL: the first fire one Interval after Enabled
    jobs_.push_back(j);
    return (int)jobs_.size() - 1;
}

namespace {
void Note(Stats& s, std::uint64_t lateUs, std::uint64_t periodUs, unsigned long long missed, std::uint64_t bodyUs, bool threw)
{
    ++s.runs;
    s.busyUs += bodyUs;
    if (bodyUs > s.maxUs) s.maxUs = bodyUs;
    if (bodyUs > periodUs) ++s.overruns;
    if (lateUs >= periodUs) ++s.lateRuns;
    if (lateUs > s.maxLateUs) s.maxLateUs = lateUs;
    s.missedBeats += missed;
    if (threw) ++s.exceptions;
}
}  // namespace

int FastClock::Service()
{
    int ran = 0;
    // Index loop, not iterators: a body may (through a waiting box) call Service() again -- never Add() -- so the vector does
    // not reallocate under us, but a nested Service() may run later jobs first; they are then simply not due here any more.
    for (std::size_t i = 0; i < jobs_.size(); ++i) {
        const std::uint64_t now = now_();
        if (now < jobs_[i].dueUs)
            continue;
        if (jobs_[i].running) {                                    // its own body is on the stack (POLICY): skip, due untouched
            ++jobs_[i].total.reentrySkips;
            ++jobs_[i].window.reentrySkips;
            continue;
        }
        const std::uint64_t periodUs = (std::uint64_t)jobs_[i].info.periodMs * 1000ull;
        const std::uint64_t lateUs   = now - jobs_[i].dueUs;
        unsigned long long missed = 0;
        if (jobs_[i].info.mode == kRate) {                         // the next grid point now, before the body (a nested
            if (lateUs >= periodUs) {                              // Service() must see this beat as taken)
                missed = (unsigned long long)(lateUs / periodUs);
                jobs_[i].dueUs = now + periodUs;
            } else {
                jobs_[i].dueUs += periodUs;
            }
        } else {
            jobs_[i].dueUs = kNever;   // kDelay: set from the END below
        }
        jobs_[i].running = true;
        bool threw = false;
        const std::uint64_t t0 = now_();
        try {
            jobs_[i].body();
        } catch (...) {
            threw = true;
        }
        const std::uint64_t t1 = now_();
        jobs_[i].running = false;
        if (jobs_[i].info.mode == kDelay)
            jobs_[i].dueUs = t1 + periodUs;                        // golden: MySleepEx(period) after Synchronize returned
        const std::uint64_t bodyUs = t1 >= t0 ? t1 - t0 : 0;
        Note(jobs_[i].total,  lateUs, periodUs, missed, bodyUs, threw);
        Note(jobs_[i].window, lateUs, periodUs, missed, bodyUs, threw);
        ++ran;
    }
    return ran;
}

std::uint64_t FastClock::NextDueUs() const
{
    std::uint64_t best = kNever;
    for (std::size_t i = 0; i < jobs_.size(); ++i)
        if (!jobs_[i].running && jobs_[i].dueUs < best)
            best = jobs_[i].dueUs;
    return best;
}

Stats FastClock::TakeWindow(int id)
{
    Job& j = jobs_[(std::size_t)id];
    const Stats s = j.window;
    j.window = Stats();
    return s;
}

std::uint32_t ClampTickDeadline(std::uint32_t nextTick, std::uint32_t nowTick, std::uint64_t nowUs, std::uint64_t dueUs)
{
    if (dueUs == kNever)
        return nextTick;                                           // no job
    std::uint64_t inMs = 0;
    if (dueUs > nowUs)
        inMs = (dueUs - nowUs + 999ull) / 1000ull;                 // round UP: waking early would only spin once more
    if (inMs > 0x7FFFFFFFull)
        return nextTick;
    const std::uint32_t mine = nowTick + (std::uint32_t)inMs;
    return ((std::int32_t)(mine - nextTick) < 0) ? mine : nextTick;   // the earlier of the two, wrap-safe like the loop's (long)(a-b)
}

PassMeter::PassMeter(NowUsFn now) : now_(now ? now : &DefaultNowUs) {}

void PassMeter::Begin()
{
    const std::uint64_t t = now_();
    if (end_ != 0 && t >= end_)
        w_.sleepUs += t - end_;
    begin_ = t;
    open_  = true;
}

void PassMeter::End()
{
    if (!open_)
        return;                                                    // the first sleep comes before the first pass
    const std::uint64_t t = now_();
    const std::uint64_t busy = t >= begin_ ? t - begin_ : 0;
    ++w_.passes;
    w_.busyUs += busy;
    if (busy > w_.maxBusyUs) w_.maxBusyUs = busy;
    if (busy >=  20000ull) ++w_.over20ms;
    if (busy >=  30000ull) ++w_.over30ms;
    if (busy >= 100000ull) ++w_.over100ms;
    end_  = t;
    open_ = false;
}

PassMeter::Window PassMeter::Take()
{
    const Window w = w_;
    w_ = Window();
    return w;
}

}  // namespace fastclock
}  // namespace ht9045
