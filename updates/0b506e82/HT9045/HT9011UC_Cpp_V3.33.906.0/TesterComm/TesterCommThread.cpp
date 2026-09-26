// ===========================================================================
//  TesterComm/TesterCommThread.cpp -- see TesterCommThread.h.  AI(W906-GB-P0) 20260926.
// ===========================================================================
#include "TesterComm/TesterCommThread.h"

namespace testercomm {

TesterCommThread::TesterCommThread()
    : engine_(0), mailbox_(0), stop_(false), running_(false), startFailed_(false), heartbeat_(0), errors_(0)
{
}

TesterCommThread::~TesterCommThread()
{
    Stop();
}

bool TesterCommThread::Start(TesterEngine* engine, SyncMailbox* mailbox)
{
    if (thread_.joinable() || engine == 0 || mailbox == 0)
        return false;
    engine_ = engine;
    mailbox_ = mailbox;
    stop_.store(false);
    startFailed_.store(false);
    running_.store(true);
    if (!thread_.start(&TesterCommThread::Entry, this))
    {
        running_.store(false);
        return false;
    }
    return true;
}

void TesterCommThread::Stop()
{
    if (!thread_.joinable())
        return;
    stop_.store(true);
    if (mailbox_ != 0)
        ::SetEvent(mailbox_->WakeHandle(kEngineSide));
    thread_.join();
    running_.store(false);
}

void TesterCommThread::Entry(void* self)
{
    static_cast<TesterCommThread*>(self)->Loop();
}

void TesterCommThread::Loop()
{
    bool started = false;
    try
    {
        started = engine_->Start(mailbox_);
    }
    catch (...)
    {
        errors_.fetch_add(1);
    }
    if (!started)
    {
        startFailed_.store(true);
        running_.store(false);
        return;
    }

    while (!stop_.load())
    {
        heartbeat_.fetch_add(1);
        unsigned waitMs = 10;
        try
        {
            mailbox_->Poll(kEngineSide);
            waitMs = engine_->RunOnce();
        }
        catch (...)
        {
            errors_.fetch_add(1);
            waitMs = 10;
        }
        if (stop_.load())
            break;
        ::WaitForSingleObject(mailbox_->WakeHandle(kEngineSide), waitMs);
    }

    try
    {
        mailbox_->Poll(kEngineSide);   // answer anything already queued so no Handler sender waits for a timeout
        engine_->Stop();
    }
    catch (...)
    {
        errors_.fetch_add(1);
    }
    running_.store(false);
}

}  // namespace testercomm
