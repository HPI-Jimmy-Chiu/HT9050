// ===========================================================================
//  vclcompat/ServerSocket.cpp
//  Implementation of the Scktcomp::TServerSocket / TServerWinSocket shim
//  (see ServerSocket.h for the full provenance / Sim-Real policy / deferred-
//  delete lifetime rationale).
//
//  AI(W906-ServerSocket) 20260713: new file.
//
//  Include order note: same rationale as ClientSocket.cpp -- <winsock2.h>
//  before <windows.h> (both guarded #if _WIN32) so a legacy <winsock.h>
//  pulled in transitively by windows.h can never precede winsock2.h.
// ===========================================================================
#include "vclcompat/ServerSocket.h"

#include <cstring>
#include <algorithm>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <windows.h>
#endif

namespace Scktcomp {

// ---------------------------------------------------------------------------
//  WinSock2 process-wide init (real mode only), lazy + once.
//  Duplicated from ClientSocket.cpp's own EnsureWinsockInit_ (internal
//  linkage in both TUs -- WSAStartup/WSACleanup are refcounted by Windows
//  itself per-process, so two independent lazy-init call sites are safe;
//  this keeps the two shim files independently self-contained rather than
//  exposing a shared non-static helper for ~10 lines of code).
// ---------------------------------------------------------------------------
#if defined(_WIN32)
static bool EnsureWinsockInit_()
{
    static bool s_inited = false;
    static bool s_ok = false;
    if (!s_inited)
    {
        s_inited = true;
        WSADATA wsa;
        s_ok = (::WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
    }
    return s_ok;
}
#endif
struct W906PollState_;  static void W906PollStateDelete_(W906PollState_* p);   // AI(W906-W10) 20260927 (St02-E): POLLED mode, end of this file
// ===========================================================================
//  TServerWinSocket
// ===========================================================================
TServerWinSocket::TServerWinSocket()
    : ActiveConnections(0)
{
}

TServerWinSocket::~TServerWinSocket()
{
    // Non-owning: the TCustomWinSocket* elements are deleted by
    // TServerSocket::~TServerSocket() (see the deferred-delete lifetime note
    // in ServerSocket.h). Just drop our references.
    Connections.clear();
}

// ===========================================================================
//  TServerSocket
// ===========================================================================
struct TServerSocket::Impl {
    bool bSimRequested;   // composition-root choice via SetSimMode()
    bool bSim;
    bool bActive;

    // Every connection object ever created by this server (Sim or Real),
    // owned here for final deletion in ~TServerSocket() regardless of
    // whether it is still "live" in Socket->Connections (see ServerSocket.h
    // BACKING note: deliberate deferred-delete lifetime simplification).
    std::vector<TCustomWinSocket*> allConnections;
    W906PollState_* pPoll = nullptr;   // AI(W906-W10) 20260927 (St02-E): POLLED-mode state (end of this file); 0 = never configured
#if defined(_WIN32)
    SOCKET            listenSock;
    HANDLE            hAcceptor;
    volatile LONG     bStopAcceptor;
    CRITICAL_SECTION  csConn;   // guards Attach/DetachConnection_'s mutation
                                // of Socket->Connections/ActiveConnections
                                // against a concurrent Real-mode
                                // accept/reader thread (see ServerSocket.h's
                                // "NOT reproduced" note about the rest of a
                                // consumer's own reads being unsynchronized,
                                // same as golden's single-VCL-thread
                                // assumption not being reproduced here).
#endif

    Impl()
        : bSimRequested(true)
        , bSim(true)
        , bActive(false)
#if defined(_WIN32)
        , listenSock(INVALID_SOCKET)
        , hAcceptor(0)
        , bStopAcceptor(0)
#endif
    {
#if defined(_WIN32)
        ::InitializeCriticalSection(&csConn);
#endif
    }

    ~Impl()
    {
#if defined(_WIN32)
        ::DeleteCriticalSection(&csConn);
#endif
    }
};

TServerSocket::TServerSocket(TComponent* AOwner)
    : TComponent(AOwner)
    , Port(0)
    , Active(this)
    , OnClientConnect()
    , OnClientDisconnect()
    , OnClientError()
    , OnClientRead()
    , Socket(new TServerWinSocket())
    , pImpl_(new Impl())
{
}

TServerSocket::~TServerSocket()
{
    DoClose_();

    // AI(W906-ServerSocket-fix) 20260713: join EVERY connection's reader
    // thread (not just the ones DoClose_() found still "live") before
    // deleting it -- closes a genuine use-after-free race an independent
    // fidelity review found: DoClose_() only disconnected live connections
    // (closesocket + mark disconnected), it never waited for that
    // connection's own background reader thread (ReaderProc_, blocked in
    // recv()) to actually finish. If that thread was still unwinding when
    // the loop below used to run straight into `delete`, it could dereference
    // an already-freed TCustomWinSocket (and, via its closeNotify hook, this
    // already-being-destroyed TServerSocket too). StopReaderThread() is
    // idempotent/no-op for a connection whose reader already exited on its
    // own (natural EOF) or that never had one (Sim mode).
    for (size_t i = 0; i < pImpl_->allConnections.size(); ++i)
        pImpl_->allConnections[i]->StopReaderThread();

    for (size_t i = 0; i < pImpl_->allConnections.size(); ++i)
        delete pImpl_->allConnections[i];
    delete Socket;
    Socket = 0;
    W906PollStateDelete_(pImpl_->pPoll); delete pImpl_;   // AI(W906-W10) 20260927 (St02-E)
    pImpl_ = 0;
}

void TServerSocket::SetSimMode(bool bSim)
{
    pImpl_->bSimRequested = bSim;
    if (!pImpl_->bActive)
        pImpl_->bSim = bSim;
}

bool TServerSocket::IsSimMode() const   { return pImpl_->bSim; }
bool TServerSocket::IsActiveNow() const { return pImpl_->bActive; }

// ---------------------------------------------------------------------------
//  AttachConnection_ / DetachConnection_ -- shared bookkeeping for BOTH the
//  Sim path (SimAcceptConnection/SimDropConnection) and the Real path
//  (AcceptorProc_ / a per-connection reader thread's close-notify hook).
// ---------------------------------------------------------------------------
void TServerSocket::AttachConnection_(TCustomWinSocket* conn)
{
#if defined(_WIN32)
    ::EnterCriticalSection(&pImpl_->csConn);
#endif
    pImpl_->allConnections.push_back(conn);
    Socket->Connections.push_back(conn);
    Socket->ActiveConnections = static_cast<int>(Socket->Connections.size());
#if defined(_WIN32)
    ::LeaveCriticalSection(&pImpl_->csConn);
#endif

    // These two hooks are what let ONE TCustomWinSocket instance behave as a
    // TServerSocket connection slot (see ClientSocket.h's EXTENSION note):
    // read arrivals -> OnClientRead; the connection closing (Sim drop, Real
    // peer EOF, or wholesale Close()) -> DetachConnection_.
    conn->SetReadNotifyHook([this, conn](TCustomWinSocket*) {
        if (OnClientRead) OnClientRead(this, conn);
    });
    conn->SetCloseNotifyHook([this, conn](TCustomWinSocket*) {
        DetachConnection_(conn);
    });

    if (OnClientConnect) OnClientConnect(this, conn);
}

void TServerSocket::DetachConnection_(TCustomWinSocket* conn)
{
    bool wasLive = false;
#if defined(_WIN32)
    ::EnterCriticalSection(&pImpl_->csConn);
#endif
    std::vector<TCustomWinSocket*>& v = Socket->Connections;
    std::vector<TCustomWinSocket*>::iterator it = std::find(v.begin(), v.end(), conn);
    if (it != v.end())
    {
        v.erase(it);
        Socket->ActiveConnections = static_cast<int>(v.size());
        wasLive = true;
    }
#if defined(_WIN32)
    ::LeaveCriticalSection(&pImpl_->csConn);
#endif

    // Idempotent: a connection already detached (e.g. Close() dropped it,
    // then its reader thread's own EOF fires the same hook a second time)
    // does not re-fire OnClientDisconnect.
    if (wasLive && OnClientDisconnect)
        OnClientDisconnect(this, conn);

    // Deliberately NOT deleted here -- see ServerSocket.h's BACKING note
    // (deferred-delete lifetime simplification, avoids a same-thread
    // self-delete hazard when this runs on the connection's own Real-mode
    // reader thread). Actual deletion happens in ~TServerSocket() via
    // pImpl_->allConnections.
}

// ---------------------------------------------------------------------------
//  SimAcceptConnection / SimDropConnection
// ---------------------------------------------------------------------------
TCustomWinSocket* TServerSocket::SimAcceptConnection(const AnsiString& peerAddress, int peerPort)
{
    TCustomWinSocket* conn = new TCustomWinSocket();
    conn->Connected    = true;
    conn->RemotePort   = peerPort;
    conn->LocalPort    = Port;             // golden: event Socket->LocalPort (uHGemEquipment.cpp:6822)
    conn->LocalAddress = (peerAddress.Length() > 0) ? peerAddress : AnsiString("127.0.0.1");

    // Sim connection identity: a monotonically increasing synthetic id (Sim
    // mode has no real OS socket fd to reuse). Handle/SocketHandle are kept
    // numerically identical -- see ServerSocket.h's citation on why golden
    // treats them as one identity spelled two ways.
    static int s_nextSimHandle = 1000;
    conn->Handle = conn->SocketHandle = s_nextSimHandle++;

    AttachConnection_(conn);
    return conn;
}

void TServerSocket::SimDropConnection(TCustomWinSocket* conn)
{
    if (conn == 0)
        return;
    // Disconnect() (ClientSocket.cpp) marks Connected=false and fires the
    // close-notify hook wired in AttachConnection_ above -> DetachConnection_
    // runs synchronously on the caller's thread here (Sim mode, no
    // background thread involved).
    conn->Disconnect(conn->RemotePort);
}

// ---------------------------------------------------------------------------
//  Real-mode accept() loop (private static member -> touches pImpl_ freely).
// ---------------------------------------------------------------------------
#if defined(_WIN32)
unsigned long __stdcall TServerSocket::AcceptorProc_(void* param)
{
    TServerSocket* srv = static_cast<TServerSocket*>(param);
    for (;;)
    {
        if (::InterlockedCompareExchange(&srv->pImpl_->bStopAcceptor, 1, 1) != 0)
            break;

        sockaddr_in peerAddr;
        int peerLen = sizeof(peerAddr);
        std::memset(&peerAddr, 0, sizeof(peerAddr));
        SOCKET c = ::accept(srv->pImpl_->listenSock,
                             reinterpret_cast<sockaddr*>(&peerAddr), &peerLen);

        if (::InterlockedCompareExchange(&srv->pImpl_->bStopAcceptor, 1, 1) != 0)
        {
            if (c != INVALID_SOCKET)
                ::closesocket(c);
            break;
        }
        if (c == INVALID_SOCKET)
            break;   // listen socket closed or a real accept() error: stop

        TCustomWinSocket* conn = new TCustomWinSocket();
        conn->AttachRealSocket_(static_cast<intptr_t>(c), static_cast<int>(::ntohs(peerAddr.sin_port)));
        conn->LocalPort    = srv->Port;
        conn->Handle       = static_cast<int>(c);
        conn->SocketHandle = static_cast<int>(c);   // same identity, see citation note
        char* pAddrText = ::inet_ntoa(peerAddr.sin_addr);
        conn->LocalAddress = (pAddrText != 0) ? AnsiString(pAddrText) : AnsiString("0.0.0.0");

        srv->AttachConnection_(conn);
        conn->StartReaderThread();
    }
    return 0;
}
#endif

// ---------------------------------------------------------------------------
//  DoOpen_ / DoClose_ -- Active=true / Active=false.
// ---------------------------------------------------------------------------
void TServerSocket::DoOpen_()
{
    if (pImpl_->bActive)
        return;   // idempotent, mirrors TClientSocket::DoConnect_'s guard
    if (PolledOpen_()) return;   // AI(W906-W10) 20260927 (St02-E): SimFailNextOpen / POLLED real Open (end of this file); false = the two paths below, unchanged
#if defined(_WIN32)
    if (!pImpl_->bSimRequested)
    {
        if (EnsureWinsockInit_())
        {
            SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (s != INVALID_SOCKET)
            {
                int reuse = 1;
                ::setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                             reinterpret_cast<const char*>(&reuse), sizeof(reuse));

                sockaddr_in addr;
                std::memset(&addr, 0, sizeof(addr));
                addr.sin_family      = AF_INET;
                addr.sin_addr.s_addr = INADDR_ANY;
                addr.sin_port        = ::htons(static_cast<u_short>(Port));

                if (::bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0
                    && ::listen(s, SOMAXCONN) == 0)
                {
                    pImpl_->listenSock = s;
                    pImpl_->bSim       = false;
                    pImpl_->bActive    = true;

                    ::InterlockedExchange(&pImpl_->bStopAcceptor, 0);
                    DWORD tid = 0;
                    pImpl_->hAcceptor = ::CreateThread(
                        0, 0,
                        reinterpret_cast<LPTHREAD_START_ROUTINE>(&TServerSocket::AcceptorProc_),
                        this, 0, &tid);
                    return;
                }
                ::closesocket(s);
            }
        }
        // bind()/listen()/socket()/WSAStartup failed: stay inactive, no Sim
        // fallback -- same safety-IO stance as ClientSocket.cpp's DoConnect_
        // (a silently-fake "listening" success would mask a real bind/port
        // conflict in production).
        return;
    }
#endif

    // SIM mode (the default): succeed synchronously and deterministically.
    pImpl_->bSim    = true;
    pImpl_->bActive = true;
}

void TServerSocket::DoClose_()
{
    if (!pImpl_->bActive)
        return;   // idempotent

#if defined(_WIN32)
    if (!pImpl_->bSim)
    {
        ::InterlockedExchange(&pImpl_->bStopAcceptor, 1);
        if (pImpl_->listenSock != INVALID_SOCKET)
        {
            ::closesocket(pImpl_->listenSock);   // unblocks a pending accept()
            pImpl_->listenSock = INVALID_SOCKET;
        }
        if (pImpl_->hAcceptor != 0)
        {
            ::WaitForSingleObject(pImpl_->hAcceptor, 2000);
            ::CloseHandle(pImpl_->hAcceptor);
            pImpl_->hAcceptor = 0;
        }
    }
#endif

    // Drop every currently-live connection (Sim or Real) -- copy the vector
    // first since Disconnect()'s close-notify hook mutates the original.
    // AI(W906-ServerSocket-fix) 20260713: join each connection's reader
    // thread BEFORE Disconnect() so a subsequent DoOpen_() reopen cycle never
    // leaves a stray reader thread racing a fresh accept cycle (belt-and-
    // suspenders alongside ~TServerSocket()'s own unconditional join over
    // ALL allConnections, which is what actually closes the reviewed UAF).
    std::vector<TCustomWinSocket*> live = Socket->Connections;
    for (size_t i = 0; i < live.size(); ++i)
    {
        live[i]->StopReaderThread();
        live[i]->Disconnect(live[i]->RemotePort);
    }

    pImpl_->bActive = false;
    // Restore the sim flag to the last explicitly-requested policy for the
    // next DoOpen_() (mirrors TClientSocket::DoClose_'s same restoration).
    pImpl_->bSim = pImpl_->bSimRequested;
}

void TServerSocket::Open()  { DoOpen_(); }
void TServerSocket::Close() { DoClose_(); }

// ---------------------------------------------------------------------------
//  ActiveProxy
// ---------------------------------------------------------------------------
TServerSocket::ActiveProxy::operator bool() const { return owner_->IsActiveNow(); }

TServerSocket::ActiveProxy& TServerSocket::ActiveProxy::operator=(bool v)
{
    if (v) owner_->DoOpen_();
    else   owner_->DoClose_();
    return *this;
}

} // namespace Scktcomp

// ===========================================================================
//  AI(W906-W10) 20260927 (St02-E): POLLED mode -- W10 = B, the Handler's TCP command server 7016 / 7017
//  (TesterComm/Tcp/CmdServerPump.cpp).  Additive and OFF by default: the Sim users and the thread-per-connection
//  Real mode above behave exactly as before unless a caller calls SetPolled(true) or SimFailNextOpen().
//
//  Why: golden's servers are ServerType = stNonBlocking (golden 906_0625_Steven main.dfm:17358-17377) -- VCL runs
//  accept / read / disconnect off the ONE main-thread message loop, so golden's handler (Command.cpp
//  TCPCommandServerClientRead) and golden's send loop (HandlerTCPIPResultSendProcess walks Socket->Connections[])
//  never race.  The Real mode above fires the events on its own accept / reader threads while the golden send loop
//  walks Connections[] on the tick thread -- a real race (W10 plan finding (e)).  Polled mode creates no thread:
//  Poll(), called by the owner on ITS thread (every tick), does a non-blocking accept() and a non-blocking recv() on
//  every connection it accepted, and fires OnClientConnect / OnClientRead / OnClientError / OnClientDisconnect right
//  there.
//
//  Contract
//    SetPolled(true) + SetSimMode(false)  the next Open() binds + listens non-blocking (no thread).  A bind failure
//                                         (port in use, bad address) leaves Active == false: VCL's Open() raised
//                                         ESocketError there, so the golden caller checks Active after Open().
//    SetPolled(true), Sim                 Open() is the Sim Open above; Poll() does nothing (tests keep driving a Sim
//                                         server with SimAcceptConnection / SimPushReceive).
//    SetBindAddress("127.0.0.1")          bind that address (default "": INADDR_ANY -- golden VCL binds every interface).
//    SetExclusiveAddr(true)               SO_EXCLUSIVEADDRUSE instead of SO_REUSEADDR (W10 plan finding (f): with
//                                         REUSEADDR a second process -- the BCB6 exe, another wb_serve -- could share
//                                         7016 silently).
//    BoundPort()                          the port actually bound (a test binds Port 0); 0 when not listening for real.
//    SimFailNextOpen()                    the next Open() (Sim or Real) fails once and leaves Active == false (the ctest
//                                         of golden's "Socket Server Open Error!!" branch).
//    Receive queue                        at most kPolledQueueCap (64 KiB) bytes wait per connection: Poll() stops
//                                         reading a connection whose queue is full; TCP flow control holds the rest
//                                         (nothing is dropped here).  The owner drains it in OnClientRead with
//                                         ReceiveLength() / ReceiveBuf() / ReceiveText(), as golden does.
//    SendBuf                              accepted sockets are non-blocking: SendBuf() never blocks the caller (a full
//                                         send buffer returns -1, like a VCL stNonBlocking SendBuf).
//    Threads                              Open / Close / Poll / SendBuf / Connections[] on ONE thread (the owner's).
// ===========================================================================
#if defined(_WIN32)
#  ifndef SO_EXCLUSIVEADDRUSE
#    define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR))   // winsock2.h of MinGW.org 6.3 does not define it
#  endif
#endif

namespace Scktcomp {

struct W906PollState_ {
    bool       bPolled;
    bool       bExclusive;
    bool       bFailNextOpen;
    AnsiString bindAddress;
    // connections Poll() accepted, with their OS socket (non-owning: allConnections owns the objects)
    std::vector<std::pair<TCustomWinSocket*, uintptr_t> > conns;
    W906PollState_() : bPolled(false), bExclusive(false), bFailNextOpen(false) {}
};

static void W906PollStateDelete_(W906PollState_* p) { delete p; }

static W906PollState_& W906PollOf_(W906PollState_*& p)
{
    if (p == 0)
        p = new W906PollState_();
    return *p;
}

void TServerSocket::SetPolled(bool bPolled)                { W906PollOf_(pImpl_->pPoll).bPolled = bPolled; }
bool TServerSocket::IsPolled() const                       { return pImpl_->pPoll != 0 && pImpl_->pPoll->bPolled; }
void TServerSocket::SetBindAddress(const AnsiString& addr) { W906PollOf_(pImpl_->pPoll).bindAddress = addr; }
void TServerSocket::SetExclusiveAddr(bool bExclusive)      { W906PollOf_(pImpl_->pPoll).bExclusive = bExclusive; }
void TServerSocket::SimFailNextOpen()                      { W906PollOf_(pImpl_->pPoll).bFailNextOpen = true; }

int TServerSocket::BoundPort() const
{
#if defined(_WIN32)
    if (!pImpl_->bActive || pImpl_->bSim || pImpl_->listenSock == INVALID_SOCKET)
        return 0;
    sockaddr_in a;
    int len = sizeof(a);
    std::memset(&a, 0, sizeof(a));
    if (::getsockname(pImpl_->listenSock, reinterpret_cast<sockaddr*>(&a), &len) != 0)
        return 0;
    return static_cast<int>(::ntohs(a.sin_port));
#else
    return 0;
#endif
}

// DoOpen_'s first step (ServerSocket.cpp DoOpen_).  true = handled here (Active may still be false: a failure);
// false = not configured for polling -> DoOpen_'s original Sim / thread-mode paths run, unchanged.
bool TServerSocket::PolledOpen_()
{
    W906PollState_* ps = pImpl_->pPoll;
    if (ps == 0)
        return false;
    if (ps->bFailNextOpen)
    {
        ps->bFailNextOpen = false;
        return true;                               // stays inactive, once
    }
    if (!ps->bPolled || pImpl_->bSimRequested)
        return false;
#if defined(_WIN32)
    if (!EnsureWinsockInit_())
        return true;
    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
        return true;
    int one = 1;
    ::setsockopt(s, SOL_SOCKET, ps->bExclusive ? SO_EXCLUSIVEADDRUSE : SO_REUSEADDR,
                 reinterpret_cast<const char*>(&one), sizeof(one));
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = ::htons(static_cast<u_short>(Port));
    if (ps->bindAddress.Length() > 0)
    {
        const unsigned long ip = ::inet_addr(ps->bindAddress.c_str());
        if (ip == INADDR_NONE)
        {
            ::closesocket(s);
            return true;
        }
        addr.sin_addr.s_addr = ip;
    }
    u_long nb = 1;
    if (::ioctlsocket(s, FIONBIO, &nb) != 0
        || ::bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0
        || ::listen(s, SOMAXCONN) != 0)
    {
        ::closesocket(s);
        return true;                               // bind / listen failed: Active stays false
    }
    pImpl_->listenSock = s;
    pImpl_->hAcceptor  = 0;                        // no acceptor thread in polled mode (DoClose_ skips the join)
    pImpl_->bSim       = false;
    pImpl_->bActive    = true;
    ps->conns.clear();
    return true;
#else
    return true;
#endif
}

int TServerSocket::Poll()
{
    W906PollState_* ps = pImpl_->pPoll;
    if (ps == 0 || !ps->bPolled || !pImpl_->bActive || pImpl_->bSim)
        return 0;
    int events = 0;
#if defined(_WIN32)
    // 1. accept what is waiting (bounded per call; the rest waits for the next Poll)
    for (int k = 0; k < 16 && pImpl_->bActive && pImpl_->listenSock != INVALID_SOCKET; ++k)
    {
        sockaddr_in peer;
        int plen = sizeof(peer);
        std::memset(&peer, 0, sizeof(peer));
        SOCKET c = ::accept(pImpl_->listenSock, reinterpret_cast<sockaddr*>(&peer), &plen);
        if (c == INVALID_SOCKET)
            break;                                 // WSAEWOULDBLOCK (nothing waiting) or an error
        u_long nb = 1;
        ::ioctlsocket(c, FIONBIO, &nb);
        TCustomWinSocket* conn = new TCustomWinSocket();
        conn->AttachRealSocket_(static_cast<intptr_t>(c), static_cast<int>(::ntohs(peer.sin_port)));
        conn->LocalPort    = Port;
        conn->Handle       = static_cast<int>(c);
        conn->SocketHandle = static_cast<int>(c);
        char* pAddrText = ::inet_ntoa(peer.sin_addr);
        conn->LocalAddress = (pAddrText != 0) ? AnsiString(pAddrText) : AnsiString("0.0.0.0");   // same field use as AcceptorProc_
        ps->conns.push_back(std::make_pair(conn, static_cast<uintptr_t>(c)));
        AttachConnection_(conn);                   // -> OnClientConnect(this, conn), on this thread
        ++events;
    }

    // 2. read every live connection Poll() accepted (a copy: a handler may Close() / Open() the server)
    std::vector<std::pair<TCustomWinSocket*, uintptr_t> > conns = ps->conns;
    for (size_t i = 0; i < conns.size(); ++i)
    {
        if (!pImpl_->bActive)
            break;
        TCustomWinSocket* conn = conns[i].first;
        const std::vector<TCustomWinSocket*>& live = Socket->Connections;
        if (!conn->Connected || std::find(live.begin(), live.end(), conn) == live.end())
            continue;                              // already closed (Close(), Disconnect(), peer EOF)
        const SOCKET sock = static_cast<SOCKET>(conns[i].second);
        std::vector<char> got;
        bool closed = false;
        int err = 0;
        char buf[4096];
        for (;;)
        {
            const int room = kPolledQueueCap - conn->ReceiveLength() - static_cast<int>(got.size());
            if (room <= 0)
                break;                             // queue full: leave the rest in the OS buffer
            const int want = room < static_cast<int>(sizeof(buf)) ? room : static_cast<int>(sizeof(buf));
            const int n = ::recv(sock, buf, want, 0);
            if (n > 0)
            {
                got.insert(got.end(), buf, buf + n);
                continue;
            }
            if (n == 0)
            {
                closed = true;                     // peer closed
                break;
            }
            const int e = ::WSAGetLastError();
            if (e != WSAEWOULDBLOCK)
            {
                err = e;
                closed = true;
            }
            break;
        }
        if (!got.empty())
        {
            conn->SimPushReceive(&got[0], static_cast<int>(got.size()));   // -> OnClientRead(this, conn)
            ++events;
        }
        if (closed && conn->Connected)
        {
            if (err != 0 && OnClientError)
            {
                int code = err;
                OnClientError(this, conn, eeReceive, code);
            }
            if (conn->Connected)
                conn->Disconnect(conn->RemotePort);   // closesocket + OnClientDisconnect(this, conn)
            ++events;
        }
    }

    // 3. forget the connections that are gone
    std::vector<std::pair<TCustomWinSocket*, uintptr_t> > still;
    const std::vector<TCustomWinSocket*>& live = Socket->Connections;
    for (size_t i = 0; i < ps->conns.size(); ++i)
    {
        TCustomWinSocket* conn = ps->conns[i].first;
        if (conn->Connected && std::find(live.begin(), live.end(), conn) != live.end())
            still.push_back(ps->conns[i]);
    }
    ps->conns.swap(still);
#endif
    return events;
}

} // namespace Scktcomp
