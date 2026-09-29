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
    delete pImpl_;
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
