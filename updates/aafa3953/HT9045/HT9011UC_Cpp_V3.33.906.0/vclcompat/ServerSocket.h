// ===========================================================================
//  vclcompat/ServerSocket.h
//  Portable shim for the VCL ScktComp TServerSocket / TServerWinSocket
//  components (BCB6 #include <ScktComp.hpp>) -- the multi-connection server
//  sibling of vclcompat/ClientSocket.h's TClientSocket/TCustomWinSocket.
//
//  AI(W906-ServerSocket) 20260713: new file. FRONT C prerequisite
//  infrastructure -- no consumer wired yet in this wave (same shape as the
//  earlier SecsWireCodec/SecsSvEcRegistration prerequisite slices that landed
//  ahead of their consumer). Two future consumers, both grounded below by
//  concrete golden call-shape citations (grepped, not guessed from memory of
//  the full VCL API -- same discipline as ClientSocket.h's own SCOPED API
//  SURFACE section).
//
//  PROVENANCE: golden ScktComp.hpp is Borland RTL and is NOT present anywhere
//  in this repo snapshot (same situation ClientSocket.h documents).
//
//  ---------------------------------------------------------------------------
//  [Consumer 1] SECSGEM/uHGemEquipment.cpp -- THGem's passive-HSMS role.
//  ---------------------------------------------------------------------------
//  THGem::SendLocalData() (golden ~1985-2091), the bUseClientSocket==false
//  (server/passive) branch:
//      if(srvGem->Active==true && bServoSocketConnect==true)
//      {
//          if(srvGem->Socket->ActiveConnections>0)
//          {
//              if(srvGem->Socket->Connections[0]->Connected)
//              {
//                  if(srvGem->Socket->ActiveConnections==1)
//                      srvGem->Socket->Connections[0]->SendBuf(LocalBuffer, LocalLength_4);
//              }
//          }
//          else { bServoSocketConnect=false; srvGem->Close(); srvGem->Open(); }
//      }
//  Plus the rest of the golden file (grepped for every srvGem/TServerSocket
//  touch point, not just the SendLocalData range):
//      srvGem->Port                   (uHGemEquipment.cpp:3455,5058,5067)
//      srvGem->Active                 (:3458,3461,5155 -- read) / :699,7754 (write false)
//      srvGem->Open() / srvGem->Close() (:2018,2038,2039,3459,4820,5202,5275,... )
//      srvGem->Socket->ActiveConnections (:3481,4787,5194,6816,6829,9050)
//      srvGem member declared `TServerSocket *srvGem;` (uHGemEquipment.h:94)
//      Event wiring (uHGemEquipment.dfm:562-573):
//          Active = False, Port = 6000, ServerType = stNonBlocking,
//          ThreadCacheSize = 1000,
//          OnClientConnect = srvGemClientConnect
//          OnClientDisconnect = srvGemClientDisconnect
//          OnClientRead = clientGemRead        <-- SAME handler function also
//                                                   wired to clientGem's (a
//                                                   TClientSocket) OnRead
//                                                   (uHGemEquipment.dfm:550)
//          OnClientError = srvGemClientError
//      Event bodies (uHGemEquipment.cpp:6812-6910, :9008):
//          srvGemClientConnect(TObject*, TCustomWinSocket* Socket):
//              loops `for(i=0;i<srvGem->Socket->ActiveConnections;i++)` reading
//              `srvGem->Socket->Connections[i]->LocalAddress`, and separately
//              reads `Socket->LocalPort` / `Socket->LocalAddress` off the
//              event's OWN Socket parameter (the just-accepted connection).
//          srvGemClientError(TObject*, TCustomWinSocket*, TErrorEvent,
//              int& ErrorCode): clears ErrorCode, calls srvGem->Close().
//          srvGemClientDisconnect(TObject*, TCustomWinSocket*): resets a
//              retry-task cursor; does not touch the Socket parameter.
//          clientGemRead(TObject*, TCustomWinSocket* Socket) (:9008): the
//              shared handler -- reads via the Socket parameter, generic to
//              either the client or server connection that fired it.
//      The shared-handler wiring (OnRead == OnClientRead pointing at the same
//      C++ function) is why this shim's per-connection socket type REUSES
//      Scktcomp::TCustomWinSocket from ClientSocket.h rather than defining a
//      parallel/mirrored type: a single std::function-typed field can only be
//      assigned the same callable if both event slots share an identical
//      Socket parameter type. See ClientSocket.h's own AI(W906-ServerSocket)
//      EXTENSION note for the fields that had to be added there
//      (Connected/LocalAddress/LocalPort/Handle/SocketHandle) to satisfy the
//      citations above and the second consumer below.
//
//  ---------------------------------------------------------------------------
//  [Consumer 2] Automation/automation.cpp -- OLPServer (2nd independent
//  golden call-shape citation, per this project's established convention of
//  always grounding new vclcompat surface in >=1 concrete golden call site;
//  ClientSocket.h's own EXTENSION note demonstrates the same citation style).
//  ---------------------------------------------------------------------------
//      Declared `TServerSocket *OLPServer;` (Automation/automation.h:37).
//      .dfm wiring (automation.dfm:456-466):
//          Active = False, Port = 5001, ServerType = stNonBlocking,
//          OnClientConnect    = OLPServerClientConnect
//          OnClientDisconnect = OLPServerClientDisconnect
//          OnClientRead       = OLPServerClientRead
//          OnClientError      = OLPServerClientError
//      Call-shape citations (automation.cpp):
//          :293,629,809  int n = (int)(OLPServer->Socket->ActiveConnections);
//          :636          OLPServer->Socket->Connections[0]->Handle
//          :638,845      OLPServer->Socket->Connections[i]->SendBuf(S.c_str(), S.Length());
//          :660          OLPServer->Open();
//          :824,829,847  OLPServer->Socket->Connections[i]->SocketHandle
//              (golden's own :817-820 comment expects iHandle==iSocketHandle,
//              used only for a mismatch-debug log -- i.e. golden itself
//              treats Handle and SocketHandle as the SAME underlying
//              connection identity spelled two ways, not two identities; this
//              shim keeps them numerically identical rather than inventing a
//              second one -- see ClientSocket.h's EXTENSION note.)
//
//  ---------------------------------------------------------------------------
//  API SURFACE BUILT -- nothing beyond what the two citations above need
//  (same discipline as ClientSocket.h's own "NOT implemented" section):
//  ---------------------------------------------------------------------------
//      TServerSocket (: TComponent):
//          int Port;
//          ActiveProxy Active;        -- bool w/ connect(listen)/close side
//                                        effect, same idiom as
//                                        TClientSocket::Active
//          void Open(); void Close();
//          TSocketNotifyEvent OnClientConnect, OnClientDisconnect, OnClientRead;
//          TSocketErrorEvent  OnClientError;
//              (reuses the EXACT typedefs from ClientSocket.h -- both were
//              already Sender/Socket-generic, not TClientSocket-specific, so
//              no new event-type surface was needed)
//          TServerWinSocket* Socket;  -- owned, always valid
//          SetSimMode(bool) / IsSimMode() / IsActiveNow()  -- same
//              composition-root idiom as TClientSocket
//      TServerWinSocket (: TCustomWinSocket) -- matches real Delphi
//          ScktComp.pas, where TServerWinSocket really does extend
//          TCustomWinSocket:
//          int ActiveConnections;
//          std::vector<TCustomWinSocket*> Connections;  -- plain vector, not
//              a proxy: every cited golden read is a bare Connections[i]
//              subscript, never an assignment to the array itself.
//      TCustomWinSocket (ClientSocket.h) EXTENDED -- see that header's own
//      change note for the exact fields/hooks and their citations.
//
//  NOT implemented (never called by either cited consumer; do NOT add):
//      ServerType / ThreadCacheSize -- pure .dfm design-time property
//          assignments; grepped both golden .cpp bodies end-to-end, neither
//          is ever READ by a single line of C++ (design-time-only in this
//          codebase's actual usage).
//      TServerSocket::Address -- real VCL TServerSocket has no such property
//          either (a server binds all interfaces); neither citation uses one.
//      Owner/Session, LookupAddress/LookupPort, ThreadCacheSize tuning knobs.
//      A per-index Disconnect-by-connection method beyond what already
//          exists on TCustomWinSocket (Disconnect(int), reused as-is by
//          SimDropConnection() below -- no new method name was invented).
//
//  ---------------------------------------------------------------------------
//  BACKING -- Sim (default) / Real, same policy SHAPE as ClientSocket.h:
//  ---------------------------------------------------------------------------
//    * SIM mode (the DEFAULT): no OS socket. SimAcceptConnection() synthesizes
//      a new TCustomWinSocket "as if" a peer just connected -- deterministic,
//      offline -- and fires OnClientConnect exactly as a Real accept() would.
//      SimDropConnection() simulates the peer/local side closing it (calls
//      the existing TCustomWinSocket::Disconnect(), which now also fires the
//      generic close-notify hook this file wires up -- see ClientSocket.h's
//      EXTENSION note). Connections[]/ActiveConnections bookkeeping updates
//      identically to the Real path.
//    * REAL mode (entered ONLY via an explicit SetSimMode(false) call, same
//      composition-root opt-in as ClientSocket.h): WinSock2
//      socket()+bind()+listen(), a background acceptor thread (Win32
//      CreateThread) blocking in accept(); each accepted peer is wrapped via
//      TCustomWinSocket::AttachRealSocket_() and given its own reader thread
//      via TCustomWinSocket::StartReaderThread() (the SAME reader idiom
//      ClientSocket.cpp already established, factored out for reuse -- see
//      ClientSocket.h's EXTENSION note).
//    * DELIBERATE LIFETIME SIMPLIFICATION (documented, not a silent gap):
//      when a connection closes (Sim drop, Real peer EOF, or Close()
//      teardown), it is removed from the live Socket->Connections view and
//      OnClientDisconnect fires, but the TCustomWinSocket object itself is
//      NOT deleted at that moment -- it is retained in a private "ever
//      accepted" list and only actually deleted in ~TServerSocket(). This
//      deliberately avoids a same-thread self-delete hazard: in Real mode,
//      the close-notify hook that removes a connection can run ON that
//      connection's OWN background reader thread (the thread that just hit
//      recv()==0); deleting the TCustomWinSocket -- and thus its own Impl,
//      which that very thread's stack-adjacent bookkeeping still
//      references -- from inside that thread is a use-after-free waiting to
//      happen. Deferring the actual `delete` to teardown sidesteps THAT
//      specific same-thread self-delete hazard (a hook synchronously
//      deleting the very object whose thread is running it) at the cost of
//      connection objects living a bit longer than "logically active" --
//      acceptable since nothing in either cited golden call site depends on
//      prompt deallocation (only on ActiveConnections/Connections[]
//      correctly reflecting "currently connected").
//      AI(W906-ServerSocket-fix) 20260713: deferring the delete does NOT, by
//      itself, prove the reader thread has actually finished running by the
//      time ~TServerSocket() reaches it (an independent fidelity review
//      caught this gap: closesocket()-via-Disconnect() unblocks a pending
//      recv() but does not GUARANTEE the thread has returned from it and
//      touched its last few lines of pImpl_ before the destructor's `delete`
//      runs) -- so ~TServerSocket() now calls
//      TCustomWinSocket::StopReaderThread() (shutdown + WaitForSingleObject +
//      CloseHandle, the same sequence TClientSocket::DoClose_ always used for
//      its own single connection) on EVERY entry in allConnections, not just
//      the ones DoClose_() found still "live", before deleting any of them.
//      That closes the actual UAF window; the paragraph above only explains
//      why delete isn't done eagerly at disconnect-time.
//    * NOT reproduced (same posture as ClientSocket.h's own documented
//      limitation): real BCB6 ScktComp with ServerType=stNonBlocking marshals
//      all these events onto the single VCL main-thread message loop. This
//      portable rewrite has no such message loop, so Real-mode
//      OnClientConnect/OnClientRead/OnClientDisconnect can fire from a
//      background accept/reader thread with no marshaling back to a "main"
//      thread -- exactly the same simplification ClientSocket.cpp's Real
//      mode already makes for OnRead. A internal critical section guards
//      this file's OWN Connections/ActiveConnections mutations (Attach/
//      Detach), but a consumer reading srvGem->Socket->ActiveConnections
//      from outside this file concurrently with a live accept/reader thread
//      is exactly as unsynchronized as golden's own single-VCL-thread
//      assumption would make unnecessary in the original -- flagged for
//      whichever future wave wires a real consumer.  AI(W906-W10) 20260927 (St02-E): POLLED mode (SetPolled / Poll, end of ServerSocket.cpp) fires every event on the Poll() caller's thread -- the TCP command server 7016 / 7017 uses it.
// ===========================================================================
#ifndef VCLCOMPAT_SERVERSOCKET_H
#define VCLCOMPAT_SERVERSOCKET_H

#include "vclcompat/ClientSocket.h"   // TCustomWinSocket, TErrorEvent, TSocketNotifyEvent/
                                      // TSocketErrorEvent, TComponent, AnsiString (all reused)
#include <vector>

namespace Scktcomp {

// ---------------------------------------------------------------------------
//  TServerWinSocket -- the live "Socket" sub-object of a TServerSocket
//  (golden: srvGem->Socket->... / OLPServer->Socket->...). Real Delphi
//  ScktComp.pas: TServerWinSocket = class(TCustomWinSocket) -- it inherits
//  SendBuf/ReceiveBuf/etc (unexercised by either golden citation on the
//  LISTENING socket itself; only ever called via Connections[i]) and adds
//  exactly the two members below.
// ---------------------------------------------------------------------------
class TServerWinSocket : public TCustomWinSocket {
public:
    TServerWinSocket();
    virtual ~TServerWinSocket();

    // golden: srvGem->Socket->ActiveConnections / OLPServer->Socket->ActiveConnections
    int ActiveConnections;

    // golden: srvGem->Socket->Connections[i] / OLPServer->Socket->Connections[i].
    // Plain vector (not a proxy) -- every cited golden read is a bare
    // Connections[i] subscript, never an assignment to the array itself.
    // NOT owning: lifetime is managed by TServerSocket (see BACKING note
    // above about the deliberate deferred-delete lifetime simplification).
    std::vector<TCustomWinSocket*> Connections;

private:
    TServerWinSocket(const TServerWinSocket&);
    TServerWinSocket& operator=(const TServerWinSocket&);
};

// ---------------------------------------------------------------------------
//  TServerSocket
// ---------------------------------------------------------------------------
class TServerSocket : public TComponent {
public:
    explicit TServerSocket(TComponent* AOwner);
    virtual ~TServerSocket();

    // ---- properties --------------------------------------------------------
    int Port;   // golden: srvGem->Port / OLPServer .dfm Port=5001

    // Active: BCB6 property with a listen/stop-listening SIDE EFFECT on plain
    // assignment, same ActiveProxy idiom already established by
    // TClientSocket::Active (ClientSocket.h).
    class ActiveProxy {
    public:
        explicit ActiveProxy(TServerSocket* o) : owner_(o) {}
        operator bool() const;
        ActiveProxy& operator=(bool v);
    private:
        TServerSocket* owner_;
    };
    ActiveProxy Active;

    void Open();    // golden: srvGem->Open() / OLPServer->Open()
    void Close();   // golden: srvGem->Close()

    // ---- events --------------------------------------------------------------
    // Reuses ClientSocket.h's TSocketNotifyEvent/TSocketErrorEvent typedefs
    // verbatim (both were already Sender/Socket-generic; see the shared-
    // handler citation above for why this matters -- clientGemRead must be
    // assignable to both an OnRead and an OnClientRead slot).
    TSocketNotifyEvent OnClientConnect;
    TSocketNotifyEvent OnClientDisconnect;
    TSocketErrorEvent  OnClientError;
    TSocketNotifyEvent OnClientRead;

    // ---- Socket sub-object -----------------------------------------------
    TServerWinSocket* Socket;   // owned; always valid for the object's lifetime

    // ---- SIM / test surface (additive) ------------------------------------
    // Composition-root switch: default is Sim (safe, offline, deterministic),
    // same idiom as TClientSocket::SetSimMode.
    void SetSimMode(bool bSim);
    bool IsSimMode() const;
    bool IsActiveNow() const;   // true between a successful Open() and Close()
    void SetPolled(bool bPolled); bool IsPolled() const; int Poll(); void SetBindAddress(const AnsiString& addr); void SetExclusiveAddr(bool bExclusive); int BoundPort() const; void SimFailNextOpen(); static const int kPolledQueueCap = 65536;   // AI(W906-W10) 20260927 (St02-E): POLLED mode, off by default -- contract + bodies at the end of ServerSocket.cpp
    // SimAcceptConnection: synthesizes an inbound connection "as if" a peer
    // just connected -- appends to Socket->Connections, bumps
    // Socket->ActiveConnections, fires OnClientConnect(this, conn) exactly as
    // a Real accept() would. peerAddress/peerPort seed the new connection's
    // RemotePort/LocalAddress (LocalPort is always this server's own Port,
    // matching golden's Socket->LocalPort read in srvGemClientConnect).
    // Returns the new (non-owning-to-the-caller) TCustomWinSocket*.
    TCustomWinSocket* SimAcceptConnection(const AnsiString& peerAddress = AnsiString(),
                                          int peerPort = 0);

    // SimDropConnection: simulates the peer (or local side) closing an
    // already-accepted connection -- thin wrapper over the existing
    // TCustomWinSocket::Disconnect(), whose close-notify hook (wired by
    // AttachConnection_ below) removes it from Socket->Connections, updates
    // ActiveConnections, and fires OnClientDisconnect(this, conn).
    void SimDropConnection(TCustomWinSocket* conn);

private:
    TServerSocket(const TServerSocket&);
    TServerSocket& operator=(const TServerSocket&);

    void DoOpen_();
    void DoClose_();
    void AttachConnection_(TCustomWinSocket* conn);    // shared by Sim accept + Real accept
    void DetachConnection_(TCustomWinSocket* conn);    // shared by Sim drop + Real EOF + Close()
    friend class ActiveProxy;
    bool PolledOpen_();   // AI(W906-W10) 20260927 (St02-E): DoOpen_'s first step (SimFailNextOpen / polled Open), end of ServerSocket.cpp
#if defined(_WIN32)
    static unsigned long __stdcall AcceptorProc_(void* param);  // real-mode accept() loop
#endif

    struct Impl;
    Impl* pImpl_;
};

} // namespace Scktcomp

// Bring TServerSocket/TServerWinSocket into global scope alongside
// TClientSocket/TCustomWinSocket (already brought in by ClientSocket.h,
// #include'd above), matching golden's unqualified spelling.
#ifndef VCLCOMPAT_NO_GLOBAL_USING
using namespace Scktcomp;
#endif

#endif // VCLCOMPAT_SERVERSOCKET_H
