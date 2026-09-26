// ===========================================================================
//  TesterComm/SyncMailbox.h -- in-process stand-in for SendMessage(WM_COPYDATA) between the machine (Handler)
//  thread and the TesterComm thread.
//
//  AI(W906-GB-P0) 20260926: Tester-comm plan P0 (skill ht9045-gpib-bridge §8, docs/GPIB_20260926_INTEGRATION_PROPOSAL.md
//  §2.2).  User rulings 20260926: "GPIB 的 IO 通訊必須是即時的" (keep golden's synchronous semantics),
//  "GPIB／RS232／TCPIP 可以共享執行緒", "執行緒必須跟機台的執行緒分開，不能互相干擾".
//
//  Semantics copied from golden (H9046_32GPIB Main.cpp:4908 / RS232Standard MainForm.cpp:655 ⇄ Handler main.cpp):
//    * Send() blocks until the OTHER side's handler has processed the payload (SendMessage is synchronous).
//    * While blocked, the sender keeps servicing requests addressed to ITSELF ("pump while waiting").  Win32 does
//      the same for sent messages, and golden depends on it: the bridge's OnMyCopyMsg sends back to the Handler,
//      and RS232Standard's "CZ status?" path sends MSG_CMD_MachineState and reads the answer right after.
//      Nested sends in both directions therefore never deadlock.
//    * A handler always runs on the thread that owns its side (Poll() / a pumping Send() on that thread), never on
//      the caller's thread.  This is the isolation contract: the TesterComm thread never executes Handler code and
//      the machine thread never executes engine code.
//  Deliberate additions (golden has none):
//    * timeout: a Send() that is not processed within timeoutMs returns kTimeout; the request is abandoned safely
//      (removed if still queued; freed by the processor if already running).  Protects the machine thread from a
//      hung engine.
//    * Reset(): drops every queued request (used when TesterCommHub switches engines).
//
//  Payloads are opaque bytes; P0 does not bind MessageDef.h (the GGpib2Handler / GHandler2Gpib structs are copied
//  in as bytes by the engines in later phases, keeping this unit free of vclcompat).
//  Win32 only (CRITICAL_SECTION + auto-reset events), same constraint as WebBridge/Sync.h.
// ===========================================================================
#ifndef TESTERCOMM_SYNCMAILBOX_H
#define TESTERCOMM_SYNCMAILBOX_H

#include "WebBridge/Sync.h"

#include <atomic>
#include <deque>
#include <string>

namespace testercomm {

enum MailboxSide { kHandlerSide = 0, kEngineSide = 1 };

// Handler for requests arriving at one side.  Runs on that side's owning thread.  Its return value is the
// result the sender gets back (the analogue of the LRESULT of WM_COPYDATA).
typedef int (*MailboxHandler)(const std::string& payload, void* ctx);

enum SendStatus { kSent = 0, kTimeout = 1, kNoReceiver = 2 };

struct MailboxStats
{
    unsigned long sent[2];       // per sending side
    unsigned long processed[2];  // per receiving side
    unsigned long timeouts[2];   // per sending side
    unsigned long abandoned;     // timed-out requests the processor finished later
};

class SyncMailbox
{
public:
    SyncMailbox();
    ~SyncMailbox();

    // Install the handler for requests addressed to `side`.  A side with no handler rejects sends (kNoReceiver).
    void SetHandler(MailboxSide side, MailboxHandler fn, void* ctx);

    // Blocking send from `from` to the other side.  Returns kSent and fills *result, or kTimeout / kNoReceiver.
    // Must be called on the thread that owns `from`.
    SendStatus Send(MailboxSide from, const std::string& payload, int* result, unsigned timeoutMs);

    // Process every request currently queued for `side`.  Call from that side's own loop.  Returns the count.
    int Poll(MailboxSide side);

    // Waitable handle signalled whenever something arrives for `side` (lets a loop sleep until work arrives).
    HANDLE WakeHandle(MailboxSide side) const { return wake_[side]; }

    // Drop all queued requests on both sides.  Senders blocked on them return kTimeout at their next wake.
    void Reset();

    MailboxStats Stats() const;

private:
    SyncMailbox(const SyncMailbox&);
    SyncMailbox& operator=(const SyncMailbox&);

    struct Request
    {
        std::string payload;
        int result;
        bool done;
        bool abandoned;   // sender gave up; processor frees it
        bool dropped;     // removed by Reset() before processing; sender frees it
        MailboxSide from;
    };

    // Runs one request on the current thread (the receiving side's owner).
    void Process(MailboxSide side, Request* r);
    // Pops one request for `side`, or NULL.
    Request* PopOne(MailboxSide side);

    mutable webbridge::WbMutex mu_;
    std::deque<Request*> queue_[2];   // queue_[s] = requests addressed TO side s
    MailboxHandler handler_[2];
    void* handlerCtx_[2];
    HANDLE wake_[2];
    MailboxStats stats_;
};

}  // namespace testercomm

#endif
