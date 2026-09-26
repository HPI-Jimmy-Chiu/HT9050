// =============================================================================
//  WebBridge/Sync.h -- the minimum threading primitives this layer needs.
//
//  WHY THIS FILE EXISTS (and why it is not <mutex>/<thread>)
//  --------------------------------------------------------
//  This tree's MinGW oracle is MinGW.org GCC 6.3.0, target mingw32, thread
//  model **win32**. Under that thread model libstdc++ DECLARES std::mutex,
//  std::lock_guard and std::thread but does not define them, so every use is a
//  link-time or compile-time failure. It is not a missing include -- there is
//  no working <mutex>/<thread> to include.
//
//  vclcompat/SyncObjs.h already reached this conclusion for the VCL side and
//  standardised on a raw Win32 CRITICAL_SECTION. WebBridge must stay free of
//  vclcompat (this layer is deliberately independent of VCL and of every
//  machine header), so the same primitive is provided here instead of taking a
//  dependency in the wrong direction.
//
//  Two WebBridge sources -- TagSnapshot.cpp and CommandQueue.cpp -- each grew
//  their own private copy of this lock while being written in parallel. This
//  header is that duplication consolidated; both now include it.
//
//  WHAT IS *NOT* HERE, DELIBERATELY
//  --------------------------------
//  std::atomic IS available on this toolchain (verified by compiling and
//  running a probe with atomic<int>/<bool>/<unsigned long long>), because it
//  lowers to intrinsics rather than to the gthreads layer. So atomics are used
//  directly and are not wrapped here.
//
//  There is no condition variable, because nothing in this layer waits on one:
//  the socket thread blocks in select() and is woken through a loopback
//  self-connect, and the UI thread never blocks on the bridge at all.
//
//  CRITICAL_SECTION is RECURSIVE on Win32. Do not rely on that -- no code here
//  re-enters a lock, and the moment something does, the deadlock it would have
//  caused on a non-recursive mutex was a real bug worth finding.
// =============================================================================
#ifndef WEBBRIDGE_SYNC_H
#define WEBBRIDGE_SYNC_H

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  error "WebBridge/Sync.h currently supports Win32 only (see the file head)."
#endif

namespace webbridge {

// ---------------------------------------------------------------------------
//  WbMutex -- a mutex. Non-copyable, non-movable: it owns an OS object whose
//  address callers hold, so duplicating it is never meaningful.
// ---------------------------------------------------------------------------
class WbMutex {
public:
    WbMutex()  { ::InitializeCriticalSection(&cs_); }
    ~WbMutex() { ::DeleteCriticalSection(&cs_); }

    void lock()   { ::EnterCriticalSection(&cs_); }
    void unlock() { ::LeaveCriticalSection(&cs_); }

private:
    WbMutex(const WbMutex&);
    WbMutex& operator=(const WbMutex&);

    CRITICAL_SECTION cs_;
};

// ---------------------------------------------------------------------------
//  WbGuard -- scoped lock. The drop-in for std::lock_guard<std::mutex>.
// ---------------------------------------------------------------------------
class WbGuard {
public:
    explicit WbGuard(WbMutex& m) : m_(m) { m_.lock(); }
    ~WbGuard()                           { m_.unlock(); }

private:
    WbGuard(const WbGuard&);
    WbGuard& operator=(const WbGuard&);

    WbMutex& m_;
};

// ---------------------------------------------------------------------------
//  WbThread -- one joinable thread, started explicitly.
//
//  Unlike std::thread this does NOT terminate the process if it is destroyed
//  while still joinable; it detaches instead. That is the safer default inside
//  a machine-control application: leaking one handle at shutdown is strictly
//  better than std::terminate() taking the handler down with it.
// ---------------------------------------------------------------------------
class WbThread {
public:
    typedef void (*EntryFn)(void* arg);

    WbThread() : h_(NULL), id_(0) {}

    ~WbThread()
    {
        if (h_ != NULL) { ::CloseHandle(h_); h_ = NULL; }
    }

    // Returns false if a thread is already running or the OS refused.
    bool start(EntryFn fn, void* arg)
    {
        if (h_ != NULL) return false;
        Payload* p = new Payload;
        p->fn = fn;
        p->arg = arg;
        h_ = ::CreateThread(NULL, 0, &WbThread::Trampoline, p, 0, &id_);
        if (h_ == NULL) { delete p; return false; }
        return true;
    }

    bool joinable() const { return h_ != NULL; }

    void join()
    {
        if (h_ == NULL) return;
        ::WaitForSingleObject(h_, INFINITE);
        ::CloseHandle(h_);
        h_ = NULL;
        id_ = 0;
    }

private:
    WbThread(const WbThread&);
    WbThread& operator=(const WbThread&);

    struct Payload {
        EntryFn fn;
        void*   arg;
    };

    static DWORD WINAPI Trampoline(LPVOID raw)
    {
        Payload* p = static_cast<Payload*>(raw);
        EntryFn fn = p->fn;
        void*   arg = p->arg;
        delete p;
        if (fn) fn(arg);
        return 0;
    }

    HANDLE h_;
    DWORD  id_;
};

// ---------------------------------------------------------------------------
inline void WbSleepMs(unsigned ms) { ::Sleep(static_cast<DWORD>(ms)); }   // AI(W906-SYNC-SLEEP) 20260926: DWORD 是 ::Sleep 的精確型別。不轉的話，先 include vclcompat（using namespace vclcompat 帶進 vclcompat::Sleep(int)，SysUtils.h:183）再 include 這檔的 TU 會報 ambiguous（Steven GB P1 的 8 個檔全中）

}  // namespace webbridge

#endif  // WEBBRIDGE_SYNC_H
