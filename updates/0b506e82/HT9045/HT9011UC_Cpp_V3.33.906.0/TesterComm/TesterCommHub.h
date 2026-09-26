// ===========================================================================
//  TesterComm/TesterCommHub.h -- owns the one TesterComm thread, the SyncMailbox and the active engine.
//  AI(W906-GB-P0) 20260926: Tester-comm plan P0 (skeleton; no engine is registered yet, and nothing calls this
//  from wb_serve yet -- wiring is P3/P7).
//
//  Handler-side API (call ONLY on the machine/tick thread):
//    RegisterFactory / SelectTestType / SendToEngine / PollHandler / SetHandlerSink / Shutdown
//  SelectTestType() follows golden's CloseGpibProgram() -> RunTestProgram() order: stop the running engine
//  (its thread is joined), drop queued mail, create the engine registered for the new TestIF_File.iTestType,
//  start the thread.  An unregistered type leaves no engine running (golden: no bridge program launched).
//
//  Replaces, in later phases: golden fMain->bFind (FindWindow on the bridge window) -> IsUp();
//  golden SendMessage(HVisionWnd, WM_COPYDATA) from the Handler -> SendToEngine().
// ===========================================================================
#ifndef TESTERCOMM_TESTERCOMMHUB_H
#define TESTERCOMM_TESTERCOMMHUB_H

#include "TesterComm/TesterCommThread.h"

#include <atomic>

namespace testercomm {

const unsigned kDefaultSendTimeoutMs = 5000;
const int kTestTypeCount = 4;

class TesterCommHub
{
public:
    TesterCommHub();
    ~TesterCommHub();

    static TesterCommHub& Instance();

    void RegisterFactory(int testType, TesterEngineFactory factory);

    // Returns true when the TesterComm thread was started for `testType`.  Whether the engine's own Start()
    // succeeded is asynchronous: check IsUp() / Thread().StartFailed().  Selecting the current type again is a
    // no-op (golden does not restart the bridge when nothing changed).
    bool SelectTestType(int testType);
    int CurrentTestType() const { return type_; }
    const char* EngineName() const;
    bool IsUp() const;

    // Handler -> engine, synchronous (SendMessage semantics).  kNoReceiver when no engine is running.
    SendStatus SendToEngine(const std::string& payload, int* result, unsigned timeoutMs = kDefaultSendTimeoutMs);

    // Where engine -> Handler requests go.  The sink runs on the Handler thread inside PollHandler() or while a
    // SendToEngine() is pumping.
    void SetHandlerSink(MailboxHandler fn, void* ctx);
    int PollHandler();

    void Shutdown();

    SyncMailbox& Mailbox() { return mailbox_; }
    const TesterCommThread& Thread() const { return thread_; }
    unsigned long HandlerErrors() const { return handlerErrors_.load(); }   // exceptions from OnHandlerMessage

private:
    TesterCommHub(const TesterCommHub&);
    TesterCommHub& operator=(const TesterCommHub&);

    static int EngineSideHandler(const std::string& payload, void* ctx);
    void StopEngine();

    TesterEngineFactory factories_[kTestTypeCount];
    TesterEngine* engine_;
    TesterCommThread thread_;
    SyncMailbox mailbox_;
    int type_;                       // -1 = none selected
    std::atomic<unsigned long> handlerErrors_;
};

}  // namespace testercomm

#endif
