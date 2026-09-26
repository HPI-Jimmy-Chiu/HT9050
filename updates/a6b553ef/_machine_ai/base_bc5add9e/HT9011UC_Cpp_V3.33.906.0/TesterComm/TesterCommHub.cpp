// ===========================================================================
//  TesterComm/TesterCommHub.cpp -- see TesterCommHub.h.  AI(W906-GB-P0) 20260926.
// ===========================================================================
#include "TesterComm/TesterCommHub.h"

namespace testercomm {

TesterCommHub::TesterCommHub() : engine_(0), type_(-1), handlerErrors_(0)
{
    for (int i = 0; i < kTestTypeCount; ++i)
        factories_[i] = 0;
}

TesterCommHub::~TesterCommHub()
{
    Shutdown();
}

TesterCommHub& TesterCommHub::Instance()
{
    static TesterCommHub hub;
    return hub;
}

void TesterCommHub::RegisterFactory(int testType, TesterEngineFactory factory)
{
    if (testType >= 0 && testType < kTestTypeCount)
        factories_[testType] = factory;
}

// Runs on the TesterComm thread (mailbox handlers always run on their owning side's thread).
int TesterCommHub::EngineSideHandler(const std::string& payload, void* ctx)
{
    TesterCommHub* hub = static_cast<TesterCommHub*>(ctx);
    TesterEngine* e = hub->engine_;
    if (e == 0)
        return -1;
    try
    {
        return e->OnHandlerMessage(payload);
    }
    catch (...)
    {
        hub->handlerErrors_.fetch_add(1);   // the sender still gets an answer instead of waiting for the timeout
        return -1;
    }
}

void TesterCommHub::StopEngine()
{
    mailbox_.SetHandler(kEngineSide, 0, 0);   // new Handler sends now get kNoReceiver
    thread_.Stop();                            // loop answers what was already queued, then engine->Stop()
    mailbox_.Reset();
    delete engine_;
    engine_ = 0;
}

bool TesterCommHub::SelectTestType(int testType)
{
    if (testType == type_)
        return engine_ != 0 && thread_.Running();   // unchanged: golden does not relaunch the bridge

    StopEngine();
    type_ = testType;
    if (testType < 0 || testType >= kTestTypeCount || factories_[testType] == 0)
        return false;

    engine_ = factories_[testType]();
    if (engine_ == 0)
        return false;
    mailbox_.SetHandler(kEngineSide, &TesterCommHub::EngineSideHandler, this);
    if (!thread_.Start(engine_, &mailbox_))
    {
        StopEngine();
        return false;
    }
    return true;
}

const char* TesterCommHub::EngineName() const
{
    return engine_ ? engine_->Name() : "";
}

bool TesterCommHub::IsUp() const
{
    return engine_ != 0 && thread_.Running() && engine_->IsUp();
}

SendStatus TesterCommHub::SendToEngine(const std::string& payload, int* result, unsigned timeoutMs)
{
    if (engine_ == 0 || !thread_.Running())
        return kNoReceiver;
    return mailbox_.Send(kHandlerSide, payload, result, timeoutMs);
}

void TesterCommHub::SetHandlerSink(MailboxHandler fn, void* ctx)
{
    mailbox_.SetHandler(kHandlerSide, fn, ctx);
}

int TesterCommHub::PollHandler()
{
    return mailbox_.Poll(kHandlerSide);
}

void TesterCommHub::Shutdown()
{
    StopEngine();
    type_ = -1;
}

}  // namespace testercomm
