# CommandQueue.cpp完整原文

[上層](../index.md)。固定pin `90afa77bac6e9b4085e7d53ae1ba475e52482d68`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/CommandQueue.cpp`。
完整214行包含所有註解與metadata；12函式／112函式行由[manifest](../source-manifest.json)的function map核offset與SHA256。

<a id="constructor"></a> `CommandQueue::CommandQueue`。
<a id="destructor"></a> `CommandQueue::~CommandQueue`。
<a id="trypush"></a> `CommandQueue::tryPush`。
<a id="drain"></a> `CommandQueue::drain`。
<a id="capacity"></a> `CommandQueue::capacity`。
<a id="size"></a> `CommandQueue::size`。
<a id="empty"></a> `CommandQueue::empty`。
<a id="acceptedcount"></a> `CommandQueue::acceptedCount`。
<a id="rejectedcount"></a> `CommandQueue::rejectedCount`。
<a id="drainedcount"></a> `CommandQueue::drainedCount`。
<a id="peaksize"></a> `CommandQueue::peakSize`。
<a id="waitforpush"></a> `CommandQueue::waitForPush`。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  WebBridge/CommandQueue.cpp
//  AI(W906-WebBridge-State) 20260805: new file.
//
//  See CommandQueue.h for the threading contract. The invariants this file
//  must hold, and why each one matters:
//
//   1. tryPush() never waits on the UI thread. No condition variable, no
//      wait-for-space, no I/O under the lock. A full queue is answered `false`
//      on the spot so the socket thread can ack the failure and get back to
//      reading its socket.
//
//   2. The bound is enforced exactly: at size() == capacity() the next push is
//      rejected, not dropped silently and not appended "just this once".
//
//   3. drain() is FIFO and O(1)-locked: one container swap under the lock, then
//      the copy out of it happens unlocked. A deep queue therefore does not
//      lengthen the window in which a socket thread is blocked on the lock.
//
//  Same CRITICAL_SECTION rationale as TagSnapshot.cpp: this tree's MinGW.org
//  GCC 6.3.0 (Thread model: win32) has no usable <mutex>, and the port tree has
//  already standardised on raw CRITICAL_SECTION for exactly this reason
//  (vclcompat/SyncObjs.h, vclcompat/ClientSocket.cpp, MyPLC/ModbusTCPClient.h).
//  One source, both toolchains, no #ifdef'd halves.
// ===========================================================================
#include "WebBridge/CommandQueue.h"

#include <atomic>
#include <chrono>   // AI(W906-IOWEB-P21) 20260925: WebCommand::pushedUs
#include <deque>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN   // keep winsock.h out; the socket layer needs winsock2.h
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include "WebBridge/Sync.h"

namespace webbridge {

// ===========================================================================
//  File-local lock primitive
// ===========================================================================
namespace {

// WbLock/WbGuard used to be defined here, and identically in the sibling file.
// Both copies were consolidated into WebBridge/Sync.h on 20260805, which also
// serves WebBridgeServer.cpp. The typedef keeps every WbLock use below
// unchanged. Note Sync.h is Win32-only by #error rather than by silent no-op:
// the previous no-op branch made a non-Windows host build compile while
// providing NO mutual exclusion at all, which is a worse outcome than not
// building, and this product is Windows-only anyway.
typedef WbMutex WbLock;

} // anonymous namespace

// ===========================================================================
//  CommandQueue::Impl
// ===========================================================================
struct CommandQueue::Impl {
    WbLock                 lock;
    std::deque<WebCommand> q;        // guarded by lock; push_back / swap only
    size_t                 capacity;
    size_t                 peak;     // guarded by lock

    // 32-bit atomics only: this tree's MinGW target is 32-bit x86 (configured
    // --with-arch=i586), where a 64-bit atomic would need a libatomic helper.
    // Exposed as uint64_t through the accessors.
    std::atomic<unsigned long> accepted;
    std::atomic<unsigned long> rejected;
    std::atomic<unsigned long> drained;  void* ev;   // AI(W906-IOWEB-P25) 20260925: HANDLE of the auto-reset push event (void* so this struct needs no extra header)

    explicit Impl(size_t cap)
        : lock(), q(), capacity(cap), peak(0), accepted(0), rejected(0), drained(0), ev(::CreateEventA(NULL, FALSE, FALSE, NULL)) {}
};

const size_t CommandQueue::kDefaultCapacity;

// ===========================================================================
//  Construction
// ===========================================================================
CommandQueue::CommandQueue(size_t capacity) : impl_(new Impl(capacity))
{
}

CommandQueue::~CommandQueue()
{
    if (impl_->ev) ::CloseHandle((HANDLE)impl_->ev);  delete impl_;
    impl_ = 0;
}

// ===========================================================================
//  ANY SOCKET THREAD
// ===========================================================================
bool CommandQueue::tryPush(const WebCommand& c)
{
    bool accepted = false;
    {
        WbGuard g(impl_->lock);
        // ------------------------------------------------------------------
        //  Critical section: one size check and one push_back. No waiting on
        //  space, no condition variable, no I/O. If the UI thread is stalled
        //  this returns false and the socket thread carries on immediately --
        //  which is the whole contract.
        // ------------------------------------------------------------------
        if (impl_->q.size() < impl_->capacity) {
            impl_->q.push_back(c);  impl_->q.back().pushedUs = (std::uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();   // AI(W906-IOWEB-P21) 20260925: when it entered the queue
            if (impl_->q.size() > impl_->peak) {
                impl_->peak = impl_->q.size();
            }
            accepted = true;
        }
    }

    // Counters bumped outside the lock; they are atomic and nothing derives an
    // invariant from their relative timing.
    if (accepted) {  if (impl_->ev) ::SetEvent((HANDLE)impl_->ev);   // AI(W906-IOWEB-P25) 20260925: wake a waitForPush(); outside the lock, never waits
        impl_->accepted.fetch_add(1UL);
    } else {
        // Rejected, NOT dropped: the caller owes the browser an ok:false ack.
        impl_->rejected.fetch_add(1UL);
    }
    return accepted;
}

// ===========================================================================
//  UI THREAD ONLY
// ===========================================================================
size_t CommandQueue::drain(std::vector<WebCommand>& out)
{
    std::deque<WebCommand> taken;
    {
        WbGuard g(impl_->lock);
        // std::deque::swap is O(1). The queue is empty and pushable again the
        // instant this scope ends, so socket threads are never held off for the
        // duration of the copy-out below, let alone for command EXECUTION --
        // which the caller does after this returns, entirely outside the lock.
        impl_->q.swap(taken);
    }

    const size_t n = taken.size();
    out.reserve(out.size() + n);
    // front-to-back = oldest-to-newest: FIFO, appended to whatever `out` held.
    std::deque<WebCommand>::const_iterator it = taken.begin();
    for (; it != taken.end(); ++it) {
        out.push_back(*it);
    }

    impl_->drained.fetch_add(static_cast<unsigned long>(n));
    return n;
}

// ===========================================================================
//  ANY THREAD -- observation only
// ===========================================================================
size_t CommandQueue::capacity() const
{
    // Fixed at construction; read under the lock anyway so the member is only
    // ever touched in one regime.
    WbGuard g(impl_->lock);
    return impl_->capacity;
}

size_t CommandQueue::size() const
{
    WbGuard g(impl_->lock);
    return impl_->q.size();
}

bool CommandQueue::empty() const
{
    WbGuard g(impl_->lock);
    return impl_->q.empty();
}

std::uint64_t CommandQueue::acceptedCount() const
{
    return static_cast<std::uint64_t>(impl_->accepted.load());
}

std::uint64_t CommandQueue::rejectedCount() const
{
    return static_cast<std::uint64_t>(impl_->rejected.load());
}

std::uint64_t CommandQueue::drainedCount() const
{
    return static_cast<std::uint64_t>(impl_->drained.load());
}

size_t CommandQueue::peakSize() const
{
    WbGuard g(impl_->lock);
    return impl_->peak;
}

// ===========================================================================
//  UI THREAD ONLY -- AI(W906-IOWEB-P25) 20260925
//  Wait for a push instead of polling size() in Sleep(2) slices. A push that
//  happened before the wait leaves the event set, so it returns at once; a
//  spurious return is harmless because the caller re-checks size().
// ===========================================================================
bool CommandQueue::waitForPush(unsigned long ms)
{
    if (!impl_->ev) { ::Sleep(ms > 2ul ? 2ul : ms); return false; }   // no event: the old behaviour
    return ::WaitForSingleObject((HANDLE)impl_->ev, (DWORD)ms) == WAIT_OBJECT_0;
}

} // namespace webbridge

<!-- preserved-content:end -->
```
