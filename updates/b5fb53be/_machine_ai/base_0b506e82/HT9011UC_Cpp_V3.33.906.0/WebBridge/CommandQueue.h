// ===========================================================================
//  WebBridge/CommandQueue.h
//  The command half of the browser<->handler seam: a BOUNDED, thread-safe
//  queue of operator commands that socket threads push and the UI thread
//  drains on its existing timer tick.
//
//  AI(W906-WebBridge-State) 20260805: new file. NEW-BUILD infrastructure, not
//  a translation of any BCB6 golden source.
//
//  ===========================================================================
//  THREADING CONTRACT
//  ===========================================================================
//  ARCHITECTURE.md section 5 (D:\HT9045\web\docs\ARCHITECTURE.md):
//
//    "Commands go the other way through a queue the UI thread drains on its
//     existing timer tick. The server enqueues and returns immediately; the
//     `ack` is sent when the UI thread has actually processed it."
//    "Nothing on the socket thread may block on machine I/O. Ever."
//
//    ANY SOCKET THREAD:  tryPush()
//    UI THREAD ONLY:     drain()
//    ANY THREAD:         size() / capacity() / empty() / the counters
//
//  "tryPush never blocks" -- precisely what that means
//  ---------------------------------------------------
//  tryPush() never waits for the UI thread to do anything: there is no
//  condition variable, no wait-for-space, no timeout, no machine I/O. When the
//  queue is full it returns false immediately and the caller sends
//  {"type":"ack","ok":false,...}. The only wait that exists is the queue's own
//  critical section, which is held for a single push_back or a single O(1)
//  container swap and never across any I/O -- so its worst case is bounded by
//  another thread's push, not by the machine.
//
//  WHY BOUNDED
//  -----------
//  An unbounded queue is a memory leak with extra steps: a UI thread stalled
//  for 30 s (which is a real, documented failure mode in this product) with a
//  browser retrying commands would grow it without limit, and the commands that
//  eventually ran would be minutes-stale operator intent. A rejected push
//  produces a visible failure ack, so the operator learns the machine did not
//  take the command. That is strictly better than silently dropping it, and
//  strictly better than queueing motion requests nobody remembers making.
//
//  windows.h is deliberately NOT included by this header (the lock lives in a
//  pimpl) so the socket component can include <winsock2.h> without fighting
//  the legacy winsock.h that windows.h would drag in first.
// ===========================================================================
#ifndef WEBBRIDGE_COMMANDQUEUE_H
#define WEBBRIDGE_COMMANDQUEUE_H

#include "WebBridge/TagValue.h"

#include <cstdint>
#include <string>
#include <vector>

namespace webbridge {

// ---------------------------------------------------------------------------
//  WebCommand -- one operator action, exactly as section 4 defines it:
//      { "type":"cmd", "id":n, "cmd":"...", "tag":"...", "value":... }
//  e.g. { cmd:"temp.setSV", tag:"temp.sv", value:"135" }
//       { cmd:"run.setOffLine" }                       (no tag, no value)
//
//  `tag` and `value` are OPTIONAL on the wire, and "absent" is not the same
//  fact as "present and empty"/"present and null" -- the same distinction
//  section 4 rule 3 makes for tag values. Hence the explicit hasTag/hasValue
//  flags rather than overloading "" and Null to mean absent.
// ---------------------------------------------------------------------------
struct WebCommand {
    // Browser-supplied correlation id, echoed back verbatim in the ack. Not
    // generated or validated here; the browser owns this number space.
    std::int64_t id;

    std::string  cmd;        // command name, e.g. "temp.setSV"

    std::string  tag;        // valid only when hasTag
    bool         hasTag;

    TagValue     value;      // valid only when hasValue (may itself be Null)
    bool         hasValue;

    // Opaque routing token for the ack, owned by the socket layer. This layer
    // never interprets or dereferences it; it only carries it across the seam
    // so the UI thread can tell the socket layer WHICH connection to ack.
    //
    // An integer handle and NOT a pointer, on purpose: a connection can close
    // while its command is still queued, and a pointer would dangle by the
    // time the UI thread got to it. An integer that no longer maps to a live
    // connection just fails the ack lookup harmlessly. 0 = unspecified.
    std::uint64_t connId;

    WebCommand()
        : id(0), cmd(), tag(), hasTag(false), value(), hasValue(false), connId(0) {}
};

// ---------------------------------------------------------------------------
//  CommandQueue
// ---------------------------------------------------------------------------
class CommandQueue {
public:
    // Default bound. ~234 tags on this screen and a handful of buttons: if more
    // than 64 operator commands are outstanding, the UI thread is stalled and
    // the honest answer to command 65 is "no".
    static const size_t kDefaultCapacity = 64;

    // capacity == 0 is legal and means "reject everything". That is the
    // READ-ONLY bridge mode ARCHITECTURE.md section 6 question 3 recommends
    // shipping first: every command gets an immediate, explicit ok:false ack
    // instead of the endpoint pretending commands do not exist.
    explicit CommandQueue(size_t capacity = kDefaultCapacity);
    ~CommandQueue();

    // =====================================================================
    //  ANY SOCKET THREAD
    // =====================================================================

    // Enqueue and return. true = accepted (an ack follows once the UI thread
    // processes it); false = REJECTED because the queue is full, and the caller
    // must send a failure ack right away. Never waits on the UI thread.
    bool tryPush(const WebCommand& c);

    // =====================================================================
    //  UI THREAD ONLY
    // =====================================================================

    // Move every queued command into `out` in FIFO order (oldest first),
    // APPENDING to whatever `out` already holds, and leave the queue empty.
    // Returns how many were appended.
    //
    // Bulk, not one-at-a-time: the lock is taken once for an O(1) internal
    // swap, so a tick's drain cost does not scale with queue depth while a
    // socket thread is waiting to push.
    size_t drain(std::vector<WebCommand>& out);

    // =====================================================================
    //  ANY THREAD -- observation only
    // =====================================================================
    size_t capacity() const;
    size_t size() const;
    bool   empty() const;

    // Cumulative counters, for logging and for the "is the UI thread stalled?"
    // question. Monotonic; never reset.
    std::uint64_t acceptedCount() const;
    std::uint64_t rejectedCount() const;   // == number of ok:false acks owed
    std::uint64_t drainedCount() const;

    // High-water mark of size(). A value at capacity() means commands were
    // being rejected; a value near it means they nearly were.
    size_t peakSize() const;

    // Non-copyable: this object IS the shared seam.
    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

private:
    struct Impl;   // holds the lock + the deque; keeps windows.h out of here
    Impl* impl_;
};

} // namespace webbridge

#endif // WEBBRIDGE_COMMANDQUEUE_H
