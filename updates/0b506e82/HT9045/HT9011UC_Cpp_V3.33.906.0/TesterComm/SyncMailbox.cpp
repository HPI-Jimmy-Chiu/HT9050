// ===========================================================================
//  TesterComm/SyncMailbox.cpp -- see SyncMailbox.h for the semantics and the golden references.
//  AI(W906-GB-P0) 20260926.
//
//  Ownership of a Request:
//    * queued        -> owned by the queue; Reset() marks it dropped (sender deletes), a timed-out sender
//                       erases and deletes it itself.
//    * being run     -> owned by the processor; if the sender already timed out (abandoned) the processor
//                       deletes it, otherwise it marks it done and the sender deletes it.
//  Every state change happens under mu_; handlers run outside the lock so they may Send() again (nesting).
// ===========================================================================
#include "TesterComm/SyncMailbox.h"

#include <algorithm>

namespace testercomm {

namespace {
const DWORD kMaxWaitSliceMs = 50;   // re-check cadence in case a wake is ever missed
}

SyncMailbox::SyncMailbox()
{
    for (int s = 0; s < 2; ++s)
    {
        handler_[s] = 0;
        handlerCtx_[s] = 0;
        wake_[s] = ::CreateEventA(NULL, FALSE /*auto-reset*/, FALSE, NULL);
        stats_.sent[s] = 0;
        stats_.processed[s] = 0;
        stats_.timeouts[s] = 0;
    }
    stats_.abandoned = 0;
}

SyncMailbox::~SyncMailbox()
{
    Reset();
    for (int s = 0; s < 2; ++s)
    {
        if (wake_[s] != NULL)
            ::CloseHandle(wake_[s]);
    }
}

void SyncMailbox::SetHandler(MailboxSide side, MailboxHandler fn, void* ctx)
{
    webbridge::WbGuard g(mu_);
    handler_[side] = fn;
    handlerCtx_[side] = ctx;
}

SyncMailbox::Request* SyncMailbox::PopOne(MailboxSide side)
{
    webbridge::WbGuard g(mu_);
    if (queue_[side].empty())
        return 0;
    Request* r = queue_[side].front();
    queue_[side].pop_front();
    return r;
}

void SyncMailbox::Process(MailboxSide side, Request* r)
{
    MailboxHandler fn;
    void* ctx;
    {
        webbridge::WbGuard g(mu_);
        fn = handler_[side];
        ctx = handlerCtx_[side];
    }
    int res = fn ? fn(r->payload, ctx) : -1;   // outside the lock: the handler may Send() back

    MailboxSide from = r->from;
    {
        webbridge::WbGuard g(mu_);
        ++stats_.processed[side];
        if (r->abandoned)
        {
            ++stats_.abandoned;
            delete r;
        }
        else
        {
            r->result = res;
            r->done = true;
        }
    }
    ::SetEvent(wake_[from]);
}

SendStatus SyncMailbox::Send(MailboxSide from, const std::string& payload, int* result, unsigned timeoutMs)
{
    const MailboxSide to = (from == kHandlerSide) ? kEngineSide : kHandlerSide;
    Request* r = 0;
    {
        webbridge::WbGuard g(mu_);
        if (handler_[to] == 0)
            return kNoReceiver;
        r = new Request;
        r->payload = payload;
        r->result = 0;
        r->done = false;
        r->abandoned = false;
        r->dropped = false;
        r->from = from;
        queue_[to].push_back(r);
        ++stats_.sent[from];
    }
    ::SetEvent(wake_[to]);

    const DWORD start = ::GetTickCount();
    for (;;)
    {
        // Pump while waiting: requests addressed to us are served on this (our own) thread.
        Request* in;
        while ((in = PopOne(from)) != 0)
            Process(from, in);

        {
            webbridge::WbGuard g(mu_);
            if (r->done)
            {
                if (result)
                    *result = r->result;
                delete r;
                return kSent;
            }
            if (r->dropped)
            {
                delete r;
                ++stats_.timeouts[from];
                return kTimeout;
            }
            const DWORD elapsed = ::GetTickCount() - start;
            if (elapsed >= timeoutMs)
            {
                std::deque<Request*>& q = queue_[to];
                std::deque<Request*>::iterator it = std::find(q.begin(), q.end(), r);
                if (it != q.end())
                {
                    q.erase(it);        // never started: ours to free
                    delete r;
                }
                else
                {
                    r->abandoned = true;   // running right now: the processor frees it
                }
                ++stats_.timeouts[from];
                return kTimeout;
            }
        }
        const DWORD elapsed = ::GetTickCount() - start;
        DWORD slice = timeoutMs > elapsed ? (DWORD)(timeoutMs - elapsed) : 0;
        if (slice > kMaxWaitSliceMs)
            slice = kMaxWaitSliceMs;
        ::WaitForSingleObject(wake_[from], slice);
    }
}

int SyncMailbox::Poll(MailboxSide side)
{
    int n = 0;
    Request* r;
    while ((r = PopOne(side)) != 0)
    {
        Process(side, r);
        ++n;
    }
    return n;
}

void SyncMailbox::Reset()
{
    {
        webbridge::WbGuard g(mu_);
        for (int s = 0; s < 2; ++s)
        {
            for (std::size_t i = 0; i < queue_[s].size(); ++i)
                queue_[s][i]->dropped = true;   // a live sender is waiting on each queued request
            queue_[s].clear();
        }
    }
    for (int s = 0; s < 2; ++s)
    {
        if (wake_[s] != NULL)
            ::SetEvent(wake_[s]);
    }
}

MailboxStats SyncMailbox::Stats() const
{
    webbridge::WbGuard g(mu_);
    return stats_;
}

}  // namespace testercomm
