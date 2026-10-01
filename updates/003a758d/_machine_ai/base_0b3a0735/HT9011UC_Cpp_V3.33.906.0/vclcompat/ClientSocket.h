// ===========================================================================
//  vclcompat/ClientSocket.h
//  Portable shim for the VCL ScktComp TClientSocket / TCustomWinSocket
//  components (BCB6 #include <ScktComp.hpp>).
//
//  AI(W5-MyPLC-Translate) 20260710: new file. The ONLY 906 consumer of
//  TClientSocket/TCustomWinSocket is MyPLC/ModbusTCPClient.cpp (recon
//  confirmed via full-tree grep). This shim is genuinely new shared
//  vclcompat/ infrastructure -- authorized for THIS unit only (per the batch
//  task spec) because it is this unit's own prerequisite and no other
//  parallel unit in this batch needs a TCP client socket. A later unit that
//  also needs one should REUSE this file rather than adding a second shim.
//
//  PROVENANCE: golden ScktComp.hpp is Borland RTL and is NOT present anywhere
//  in this repo snapshot (unlike SPComm.hpp, which vclcompat/Comm.h could
//  cite from D:/HT9045/elec/Component/spcomm.hpp). The API surface below was
//  reverse-engineered EXHAUSTIVELY from the actual call sites in golden
//  MyPLC/ModbusTCPClient.cpp (grepped, not guessed from memory of the full
//  VCL API -- see SCOPED API SURFACE below).
//
//  SCOPED API SURFACE (every TClientSocket/TCustomWinSocket touch point in
//  the golden 906 tree, MyPLC/ModbusTCPClient.cpp:1-221):
//      new TClientSocket(NULL)                     ctor, owner=NULL   (:12)
//      ->OnConnect    = SocketConnect               event assign      (:14)
//      ->OnDisconnect = SocketDisConnect             event assign      (:15)
//      ->OnError      = SocketError                  event assign      (:16)
//      ->OnRead       = SocketRead                    event assign      (:17)
//      ->Address      = AnsiString                    property          (:40)
//      ->Port         = int                            property          (:41)
//      ->Active       = bool     SIDE EFFECT: true opens/connects,
//                                 false closes                          (:48,:83)
//      ->Close()                                       method            (:86)
//      ->Socket->SendBuf(void*, int)                   method            (:141)
//      ->Socket->ReceiveLength()                        method -> int     (:180)
//      ->Socket->ReceiveBuf(void*, int)                  method -> int    (:186)
//      ->Socket->RemotePort                              property -> int  (:170)
//      ->Socket->Disconnect(int Port)                    method            (:169)
//        (golden SocketError handler calls Disconnect(Socket->RemotePort);
//        kept faithfully even though vanilla documented VCL TCustomWinSocket
//        does not advertise a Disconnect(int) overload -- this project's 906
//        source calls it this way, so the shim provides exactly that call
//        shape rather than "fixing" it to Close().)
//
//  Event signatures (standard VCL ScktComp __closure -> std::function, same
//  translation choice as vclcompat/Comm.h's TReceiveDataEvent):
//      TSocketNotifyEvent = void(TObject* Sender, TCustomWinSocket* Socket)
//      TSocketErrorEvent  = void(TObject* Sender, TCustomWinSocket* Socket,
//                                TErrorEvent ErrorEvent, int& ErrorCode)
//
//  NOT implemented (never called by the only 906 consumer; do NOT add):
//      ClientType, LookupAddress/LookupPort/LocalPort, OnLookup, Handle,
//      Owner/Session, the whole TServerSocket/TServerWinSocket family.
//
//  AI(W5-Final-ClientSocketExt) 20260711: EXTENSION for two Phase-2 consumers
//  in the same migration batch -- BarCode's 8-CCD glue unit (BarCode.h's 12
//  shared TClientSocket instances: ClientSocket_Shuttle1_A/_B,
//  ClientSocket_Shuttle2_A/_B, ClientSocket_Bottom_1..8,
//  ClientSocket_BarcodeChangeFile) and Automation's TesterTCP unit
//  (Interface/TesterTCP.cpp). Added, with exact golden call-shape citations:
//      TClientSocket::Tag                 int, public, default 0.
//          Golden: BarCode.cpp:305 `Ptr->Tag=i;` (assign, .dfm-equivalent
//          per-instance init) and read back at BarCode.cpp:3162
//          `if(Ptr->Tag==iBarCode1_1)`, :3215 `int Tag=Ptr->Tag;`, etc. --
//          Ptr is always `(TClientSocket*)Sender` (a cast of the OnConnect/
//          OnDisconnect/OnError/OnRead event's TObject* Sender parameter),
//          used to discriminate WHICH of the 12 shared sockets fired the
//          event. Real VCL's TComponent natively carries a Tag:NativeInt
//          property (Classes.pas) that every descendant inherits -- so this
//          is the single most faithful placement in real-VCL terms. It is
//          placed directly on TClientSocket here (NOT hoisted onto the
//          shared vclcompat::TComponent base in Comm.h) because this unit's
//          write scope is ClientSocket.h/.cpp only; Comm.h/TComm has zero
//          golden call sites that read/write ->Tag (grepped), so TComm gains
//          nothing from the hoist and this keeps the change minimal/scoped.
//      TClientSocket::Open()               thin alias for Active=true.
//          Golden: Interface/TesterTCP.cpp:228 `ClientSocket_TCPIP->Open();`
//          (wrapped in try/catch by the caller) and BarCode-family callers
//          via ->Open() at other shared-handler call sites (ASE_K Socket /
//          AutoAlignment use the same Open() spelling on their own
//          ClientSocket instances). Forwards straight to the SAME private
//          DoConnect_() the ActiveProxy::operator=(true) path already calls
//          -- so Open() reaches exactly the same connected state, fires the
//          same OnConnect, and is just as idempotent (no-op while already
//          connected) as `Active=true` would be. No new connect logic.
//      TCustomWinSocket::SendText(const AnsiString&) -> int
//          Golden: Interface/TesterTCP.cpp:271
//          `ClientSocket_TCPIP->Socket->SendText(Msg2+"\r\n");` and 20+
//          BarCode.cpp call sites (e.g. :5338 `...Socket->SendText(Msg2+
//          "\r\n");`). Callers already append their own line terminator
//          ("\r\n"/"\r") before calling -- SendText must NOT append a
//          second one; it forwards the AnsiString's raw bytes verbatim to
//          the existing SendBuf primitive. Return type is `int` (bytes
//          sent, i.e. whatever SendBuf returns), matching real Delphi
//          ScktComp (`function SendText(const S: string): Integer;` ==
//          `SendBuf(Pointer(S)^, Length(S))`) -- confirmed independently by
//          an OUT-OF-SCOPE golden call site, ATC\TCPData.cpp:187
//          `int iSend = _client_socket->Socket->SendText(Data.c_str());`,
//          which assigns the return value (would not compile against a
//          void SendText). All 3 in-scope call sites for THIS batch discard
//          the return value as a bare statement, which is source-compatible
//          with either signature -- `int` was chosen for closer real-VCL
//          fidelity and to not foreclose that other call site's needs.
//      TCustomWinSocket::ReceiveText() -> AnsiString
//          Golden: Interface/TesterTCP.cpp:336
//          `EthernetBuffer=Socket->ReceiveText();` (called right after a
//          `Socket->ReceiveLength()>0` guard) and BarCode.cpp:3295/:3465
//          same pattern. Drains WHATEVER is currently queued (via the
//          existing ReceiveLength()/ReceiveBuf() primitives) into a new
//          AnsiString and returns it -- does not block, does not require a
//          terminator (real VCL ReceiveText has no line-framing, it is a
//          raw "give me what's arrived so far" call; framing, if any, is
//          the caller's job, e.g. golden's own `StringReplace(...,"\r\n",
//          "")` right after the call).
//
//  AI(W906-ServerSocket) 20260713: EXTENSION for the new sibling shim
//  vclcompat/ServerSocket.h (TServerSocket/TServerWinSocket -- the
//  multi-connection server side of ScktComp). Real Delphi ScktComp.pas has
//  TServerWinSocket.Connections[i] declared as plain TCustomWinSocket, and
//  golden CONFIRMS this by wiring the identical event handler function to
//  both a TClientSocket's OnRead and a TServerSocket's OnClientRead
//  (SECSGEM/uHGemEquipment.dfm:550 `OnRead = clientGemRead` and :569
//  `OnClientRead = clientGemRead` -- one function assigned to two
//  std::function-typed slots only type-checks if both carry the same
//  Socket parameter type) -- so ServerSocket.h REUSES this class for its
//  per-connection objects rather than defining a parallel/mirrored type.
//  Added, all cited directly by SECSGEM/uHGemEquipment.cpp and
//  Automation/automation.cpp (see ServerSocket.h's own file-header for the
//  full citation list):
//      bool       Connected     -- golden Connections[0]->Connected (uHGemEquipment.cpp:2006)
//      AnsiString LocalAddress  -- golden Connections[i]->LocalAddress (:6818) and the
//                                  event's own Socket->LocalAddress (:6824)
//      int        LocalPort     -- golden event Socket->LocalPort (:6822)
//      int        Handle        -- golden Connections[0]->Handle (automation.cpp:636)
//      int        SocketHandle  -- golden Connections[i]->SocketHandle (automation.cpp:824/
//                                  829/847); golden's own :817-820 comment expects
//                                  iHandle==iSocketHandle (used only for a mismatch-debug
//                                  log), i.e. golden itself treats these as the same
//                                  underlying connection identity spelled two ways -- this
//                                  shim keeps them numerically identical, not two identities.
//  Plus generic (non-golden, additive plumbing) hooks so ONE TCustomWinSocket
//  class can be owned by either a TClientSocket OR a TServerSocket connection
//  slot without hard-coding a TClientSocket* back-pointer:
//      SetReadNotifyHook(fn) / SetCloseNotifyHook(fn) -- fire in ADDITION to
//          (never instead of) the pre-existing TClientSocket owner->OnRead
//          path, so existing TClientSocket behavior is bit-for-bit unchanged
//          when neither hook is set (the default).
//      StartReaderThread() -- Real-mode-only background reader; factored out
//          of TClientSocket::DoConnect_()'s inline CreateThread call (now a
//          thin wrapper around this) so ServerSocket.cpp's accept path can
//          spawn the identical per-connection reader for each accepted peer
//          without needing friend access to the private Impl.
//      AttachRealSocket_(intptr_t osSocketHandle, int remotePort) -- lets
//          ServerSocket.cpp's Real-mode accept() path wrap an already-live OS
//          socket handle into a fresh TCustomWinSocket from outside the
//          class, again without friend access.
//
//  BACKING -- Sim/Real, SAFETY-MOTIVATED POLICY (deliberately DIFFERENT from
//  the Comm.h auto-fallback precedent -- see rationale below):
//    * SIM mode (the DEFAULT, unconditionally, until a caller opts in to
//      Real): no OS socket. Active=true synchronously marks connected and
//      fires OnConnect (deterministic for offline tests -- real VCL's async
//      connect timing is not reproduced). SendBuf captures into a tx sink;
//      the test hook Socket->SimPushReceive() enqueues bytes and fires OnRead
//      so the handler pulls them via ReceiveLength()/ReceiveBuf() exactly as
//      the real one does.
//    * REAL mode -- entered ONLY via an explicit SetSimMode(false) call (a
//      composition-root / factory decision, matching the project's Sim<->Real
//      selection architecture, KNOWLEDGE.md Sec.1) -- WinSock2 connect() to
//      Address:Port; a background reader thread (Win32 CreateThread, same
//      idiom as vclcompat/Comm.cpp's ReaderProc_) blocks in recv() and, on
//      each arrival, enqueues bytes + fires OnRead (a "data available" notice
//      -- the handler itself pulls via Socket->ReceiveBuf, exactly as golden
//      ModbusTCPClient::SocketRead does).
//    * DELIBERATE DEVIATION from Comm.h: Comm.h's TComm auto-falls-back to
//      Sim whenever the real port cannot be opened. This shim does NOT: this
//      client feeds Enable_PLCSafety_IO (an E-STOP/safe-door interlock read
//      over Modbus, cmydef.h/cinitial.cpp) -- silently reporting a fake
//      "connected" success after a genuine Real-mode connect failure would
//      mask a real safety-IO outage in production. In Real mode, a failed
//      connect() leaves bConnected==false, fires OnError with the real
//      WSAGetLastError() code, and otherwise changes NOTHING -- so golden's
//      OWN faithfully-translated retry loop (PLCStatusCheck's tCycleConnect,
//      5000ms) keeps retrying exactly as the 906 source intends. Flagged
//      explicitly for the integrate agent / reviewer.
//
//  Include directly: #include "vclcompat/ClientSocket.h" (NOT yet added to
//  the vcl_compat.h umbrella -- left for the integrate agent to decide/wire).
// ===========================================================================
#ifndef VCLCOMPAT_CLIENTSOCKET_H
#define VCLCOMPAT_CLIENTSOCKET_H

#include "vclcompat/AnsiString.h"
#include "vclcompat/TStringList.h"   // vclcompat::TObject
#include "vclcompat/Comm.h"          // vclcompat::TComponent (reuse, per Comm.h)
#include <vector>
#include <functional>
#include <cstdint>   // intptr_t -- AttachRealSocket_ (AI(W906-ServerSocket))

namespace Scktcomp {

using vclcompat::AnsiString;
using vclcompat::TObject;
using vclcompat::TComponent;

// Real VCL ScktComp TErrorEvent enum (documented Borland RTL ordinals). The
// golden SocketError handler never inspects this parameter (only clears
// ErrorCode), so exact ordinal fidelity is not load-bearing here -- kept for
// signature completeness.
enum TErrorEvent { eeGeneral, eeSend, eeReceive, eeConnect, eeDisconnect, eeAccept };

class TClientSocket;   // fwd (back-pointer target)

// ---------------------------------------------------------------------------
//  TCustomWinSocket -- the live "Socket" sub-object (golden: ->Socket->...).
//  Always non-null on a TClientSocket (mirrors VCL's auto-created
//  TClientWinSocket instance) -- golden always dereferences ->Socket
//  unconditionally, never null-checks it.
// ---------------------------------------------------------------------------
class TCustomWinSocket : public TObject {
public:
    TCustomWinSocket();
    virtual ~TCustomWinSocket();

    // ---- properties ---------------------------------------------------
    int RemotePort;   // golden Socket->RemotePort; 0 until connected

    // AI(W906-ServerSocket) 20260713: added for the TServerSocket-family shim
    // -- see this header's own file-header EXTENSION note above for the
    // exact golden call-shape citations (SECSGEM/uHGemEquipment.cpp,
    // Automation/automation.cpp). Plain fields (never assigned by golden,
    // read-only from a consumer's point of view), matching this class's
    // existing plain-field idiom for RemotePort.
    bool       Connected;
    AnsiString LocalAddress;
    int        LocalPort;
    int        Handle;
    int        SocketHandle;

    // ---- methods (faithful signatures per the SCOPED API SURFACE above) ---
    int  ReceiveLength() const;               // bytes currently queued
    int  ReceiveBuf(void* Buf, int BufSize);   // dequeue up to BufSize bytes
    int  SendBuf(void* Buf, int BufSize);      // send/capture bytes; -1 on fail

    // AI(W5-Final-ClientSocketExt) 20260711: added for BarCode/TesterTCP (see
    // the file-header EXTENSION note for golden call-shape citations).
    // SendText: forwards the AnsiString's raw bytes to SendBuf verbatim (the
    // caller supplies its own terminator, e.g. Msg2+"\r\n" -- do NOT append
    // one here). Returns the byte count SendBuf reports (real-VCL-faithful).
    int SendText(const AnsiString& s);
    // ReceiveText: drains whatever is currently queued (ReceiveLength() +
    // ReceiveBuf()) into a new AnsiString. Non-blocking; callers already
    // guard with `if (Socket->ReceiveLength() > 0)` before calling, matching
    // golden (Interface/TesterTCP.cpp:333-336).
    AnsiString ReceiveText();

    // Faithful to golden `Socket->Disconnect(Socket->RemotePort)`
    // (SocketError handler). Closes the underlying connection; Port is
    // accepted for call-shape fidelity and not otherwise used.
    void Disconnect(int Port);

    // ---- SIM / test surface (additive, NOT part of the BCB6 API) ----------
    void SimPushReceive(const void* pData, int len);  // enqueue as if-arrived,
                                                       // fires owner->OnRead
    const std::vector<char>& SimTxBuffer() const;     // bytes SendBuf captured (sim)
    void SimClearTx();

    // AI(W906-ServerSocket) 20260713: generic notify hooks + Real-mode
    // plumbing -- see file-header EXTENSION note above. Fire IN ADDITION to
    // the pre-existing TClientSocket owner->OnRead path (never instead of),
    // so existing TClientSocket callers are unaffected when unset (default).
    void SetReadNotifyHook(const std::function<void(TCustomWinSocket*)>& fn);
    void SetCloseNotifyHook(const std::function<void(TCustomWinSocket*)>& fn);

    // Real-mode-only: spawns the background reader thread (no-op / idempotent
    // in Sim mode or if already running). Factored out of
    // TClientSocket::DoConnect_()'s inline CreateThread call so
    // ServerSocket.cpp's accept path can reuse the identical reader for each
    // accepted connection without needing friend access to Impl.
    void StartReaderThread();

    // AI(W906-ServerSocket-fix) 20260713: Real-mode-only: signals + joins this
    // socket's OWN reader thread (mirrors TClientSocket::DoClose_'s inline
    // shutdown/WaitForSingleObject/CloseHandle sequence, factored out here so
    // an external owner -- ServerSocket.cpp, which only sees this public
    // interface, not TClientSocket's private Impl -- can guarantee a
    // connection's reader thread has fully exited before deleting the
    // TCustomWinSocket object it runs against. No-op/idempotent in Sim mode
    // or once already joined (hReader==0). MUST be called (per-connection,
    // for every entry ever created, not just currently-"live" ones) before
    // any external deleter destroys a TCustomWinSocket that was ever handed
    // to StartReaderThread() -- see ServerSocket.cpp's ~TServerSocket() for
    // the motivating use-after-free this closes (an independent fidelity
    // review of the triple-front wave found ~TServerSocket() deleting live
    // connections without first joining their reader threads).
    void StopReaderThread();

    // Real-mode-only: wraps an already-connected OS socket handle (from
    // ServerSocket.cpp's accept() loop) into this (freshly-constructed,
    // otherwise-Sim-default) instance. Sets bSim=false, Connected=true,
    // RemotePort=remotePort. Never called by Sim-mode code paths.
    void AttachRealSocket_(intptr_t osSocketHandle, int remotePort);

private:
    TCustomWinSocket(const TCustomWinSocket&);
    TCustomWinSocket& operator=(const TCustomWinSocket&);

    friend class TClientSocket;
    static unsigned long __stdcall ReaderProc_(void* param);  // real-mode reader thread

    struct Impl;
    Impl* pImpl_;
};

// Event signatures (VCL __closure -> std::function; matches the Comm.h
// TReceiveDataEvent translation choice).
typedef std::function<void(TObject* Sender, TCustomWinSocket* Socket)>
        TSocketNotifyEvent;
typedef std::function<void(TObject* Sender, TCustomWinSocket* Socket,
                            TErrorEvent ErrorEvent, int& ErrorCode)>
        TSocketErrorEvent;

// ---------------------------------------------------------------------------
//  TClientSocket
// ---------------------------------------------------------------------------
class TClientSocket : public TComponent {
public:
    explicit TClientSocket(TComponent* AOwner);
    virtual ~TClientSocket();

    // ---- properties --------------------------------------------------------
    AnsiString Address;
    int        Port;

    // AI(W5-Final-ClientSocketExt) 20260711: added for BarCode's 12 shared
    // TClientSocket instances (see file-header EXTENSION note). Real VCL's
    // TComponent carries Tag:NativeInt natively; golden BarCode.dfm sets one
    // per instance (0..7 range for the 8 "Bottom" sockets, per BarCode.cpp:305
    // `Ptr->Tag=i;`) and event handlers read it back off the cast Sender to
    // discriminate which physical socket fired. Plain public int, default 0
    // (matches real VCL's Tag default and this shim's existing plain-field
    // idiom for other formerly-__property members).
    int Tag;

    // Active: BCB6 property with a connect/disconnect SIDE EFFECT on plain
    // assignment (`pClinetSocket->Active=true;`). Proxy-with-operator=
    // idiom, same pattern already established by vclcompat/TStringList.h's
    // StringsProxy/TextProxy family.
    class ActiveProxy {
    public:
        explicit ActiveProxy(TClientSocket* o) : owner_(o) {}
        operator bool() const;
        ActiveProxy& operator=(bool v);
    private:
        TClientSocket* owner_;
    };
    ActiveProxy Active;

    void Close();   // golden also calls this explicitly right after Active=false

    // AI(W5-Final-ClientSocketExt) 20260711: thin alias for the same connect
    // path `Active=true` already drives (see file-header EXTENSION note).
    // Golden: Interface/TesterTCP.cpp:228 `ClientSocket_TCPIP->Open();`
    // (caller wraps it in try/catch defensively; DoConnect_() itself never
    // throws, matching this shim's existing no-throw Real-mode-failure
    // policy -- see the Sim/Real BACKING note above -- so the try/catch is
    // harmless dead code here, not something this shim needs to satisfy).
    void Open();

    // ---- events --------------------------------------------------------------
    TSocketNotifyEvent OnConnect;
    TSocketNotifyEvent OnDisconnect;
    TSocketErrorEvent  OnError;
    TSocketNotifyEvent OnRead;

    // ---- Socket sub-object -----------------------------------------------
    TCustomWinSocket* Socket;   // owned; always valid for the object's lifetime

    // ---- SIM / test surface (additive) ------------------------------------
    // Composition-root switch: default is Sim (safe, offline, deterministic).
    // Call SetSimMode(false) to opt in to a REAL WinSock2 connection.
    void SetSimMode(bool bSim);
    bool IsSimMode() const;
    bool IsActiveNow() const;   // true between a successful connect and close

private:
    TClientSocket(const TClientSocket&);
    TClientSocket& operator=(const TClientSocket&);

    void DoConnect_();
    void DoClose_();
    friend class ActiveProxy;

    struct Impl;
    Impl* pImpl_;
};

} // namespace Scktcomp

// Bring the ScktComp names into global scope, matching the original header's
// implicit unqualified TClientSocket/TCustomWinSocket usage in golden
// (Comm.h precedent: same umbrella-suppressible `using namespace`).
#ifndef VCLCOMPAT_NO_GLOBAL_USING
using namespace Scktcomp;
#endif

#endif // VCLCOMPAT_CLIENTSOCKET_H
