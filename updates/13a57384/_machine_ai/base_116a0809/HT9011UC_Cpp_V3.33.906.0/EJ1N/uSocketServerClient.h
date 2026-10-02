// =============================================================================
//  uSocketServerClient.h  --  generic raw-TCP client/server wrapper (Omron
//                              EJ1N family transport option; also used
//                              standalone as a bare socket "device").
//
//  Faithful translation of golden EJ1N/uSocketServerClient.h (126 lines,
//  BCB6, Big5/cp950 -- this header carries no Chinese comments).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 group "ej1n".  Second EJ1N/ file to land in
//  the port (after EJ1N/TextProcess.cpp/.h, W906-GA-EJ1N).
//
//  ROLE: two thin, symmetrical wrappers around VCL ScktComp --
//    uSocketBase   -- shared address/port settings persistence (an ini
//                     section "SocketSetting"), a receive-callback slot, and
//                     a synchronous ICMP echo (Ping) probe.
//    uSocketServer : uSocketBase -- a single-listener TServerSocket wrapper
//                     with a tiny state-machine `DoOpenCommuncation()`
//                     cursor (0 -> 50 -- 100 -- 200) that opens the listener
//                     and reports "connected" once exactly one client is on.
//    uSocketClient : uSocketBase -- a TClientSocket wrapper with the same
//                     state-machine SHAPE folded into one call
//                     (`MatchClientSetting() && Open() && bConnected`).
//  NO class in this header owns a golden switch(Task) cursor beyond the two
//  named above; both are reproduced verbatim in the .cpp.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim declarations): every member of uSocketBase /
//     uSocketServer / uSocketClient, all 3 golden typedefs/structs used by
//     Ping() (IPINFO/ICMPECHO/PF_CMPCREATEFILE/PF_ICMPCLOSEHANDLE/
//     PF_ICMPSENDECHO), and ReceiveLen.
//   SATISFIED-BY-SUBSTRATE: TServerSocket/TServerWinSocket/TClientSocket/
//     TCustomWinSocket/TErrorEvent all come from vclcompat/ServerSocket.h +
//     vclcompat/ClientSocket.h (pulled in transitively by the vcl_compat.h
//     umbrella -- see the .cpp's own banner for the two ADAPTATIONS this
//     substrate forces, both re-cited at their call sites).
//   GATE (#if 0): none in THIS header (both of this unit's gates/adaptations
//     are call-site-local inside the .cpp; see that file's own GATE/
//     ADAPTATION register).
//
//  VCL/Borland conversions:
//   * `#include <ScktComp.hpp>` / `<map>` / `<vcl.h>` (golden :6-8) -> this
//     port includes "vclcompat/vcl_compat.h" directly (self-contained header,
//     same convention EJ1N/TextProcess.h already established), which pulls
//     the umbrella's AnsiString/TIniFile plus vclcompat/ServerSocket.h (which
//     itself pulls vclcompat/ClientSocket.h) -- see vcl_compat.h:223-240.
//     `<map>` is unused by this header's own declarations (golden never
//     references std::map here either -- grepped, dead include); dropped.
//   * `typedef void (__closure *TPointVoidReceive)(char*,int);` (golden :38)
//     -- BCB6 bound-method closure has no standard-C++ spelling.  Per
//     vclcompat/vcl_compat.h's documented convention ("Where a __closure
//     typedef is needed, translated code should use a std::function-style
//     typedef instead"), the SAME choice EJ1N's sibling BarCode_Helpers.h
//     already made for its own TPointVoidShowMsg -- translated here as
//     `std::function<void(char*,int)>`.  A default-constructed std::function
//     is "empty" (throws std::bad_function_call if invoked); both golden call
//     sites (.cpp ServerSocketRead/ClientSocketRead) invoke it unconditionally
//     inside a `try{...}catch(...){}` block that was ALREADY there in golden
//     for other reasons, so an unset receive callback now throws-and-is-
//     caught instead of golden's undefined-behaviour null-closure call --
//     STRICTLY safer, same silently-swallowed outcome either way.
//   * `void __fastcall ServerSocketConnect(...)` etc. (golden :75-83,:103-110)
//     -- `__fastcall` dropped (neutralized to nothing by vcl_compat.h anyway,
//     kept dropped here for readability, matching every other ported unit).
//   * `TObject *Sender, TCustomWinSocket *Socket` parameters are UNCHANGED --
//     both types are real classes in this port (vclcompat::TObject via
//     Scktcomp; Scktcomp::TCustomWinSocket), matching the golden call shape
//     exactly, including the `TErrorEvent ErrorEvent, int &ErrorCode`
//     signature tail on the two *Error handlers.
//   * BYTE/DWORD/WORD/HANDLE/VOID/LPVOID/WINAPI/FAR (golden :15-36, the ICMP
//     structs) come from <windows.h>, pulled in by MachineDefine.h's own
//     `#include <windows.h>` (MachineDefine.h:9) -- reproduced VERBATIM,
//     same field order, same names.
//
//  Big5: this golden header has ZERO Chinese comments (confirmed by decoding
//  the raw bytes as cp950 and inspecting every line) -- nothing to preserve,
//  nothing to gate on U+FFFD.
// =============================================================================

// BCB6 ORIGINAL header (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #ifndef uSocketServerClientH / #define uSocketServerClientH
//   #include <ScktComp.hpp>
//   #include <map>
//   #include <vcl.h>
// =============================================================================

//---------------------------------------------------------------------------

#ifndef uSocketServerClientH
#define uSocketServerClientH

#include "vclcompat/vcl_compat.h"   // AnsiString, TIniFile; umbrella pulls
                                    //   vclcompat/ServerSocket.h -> ClientSocket.h
                                    //   (TServerSocket/TClientSocket/TCustomWinSocket/
                                    //   TErrorEvent), and <windows.h> (BYTE/DWORD/WORD/
                                    //   HANDLE/VOID/LPVOID/WINAPI/FAR for the ICMP block)
#include <functional>               // TPointVoidReceive -- see banner ADAPTATION note

//---------------------------------------------------------------------------
#define ReceiveLen 1023
//---------------------------------------------------------------------------
// Definition of the IP options structure
typedef struct tagIPINFO
{
    BYTE bTimeToLive; // Time To Live
    BYTE bTypeOfService; // Type Of Service
    BYTE bIpFlags; // IP-Flags
    BYTE OptSize; // Size of the options data buffers
    BYTE FAR *Options; // Pointer to options data buffer
} IPINFO, *PIPINFO;

// Definition the ICMP-Echo response structure
typedef struct tagICMPECHO
{
    DWORD dwSource; // destination address
    DWORD dwStatus; // IP-Status
    DWORD dwRTTime; // Round trip time in milliseconds
    WORD wDataSize; // Grosse des reply-Buffers
    WORD wReserved;
    void FAR *pData; // pointer to the reply-Data
    IPINFO ipInfo; // reply-options
} ICMPECHO, *PICMPECHO;
typedef HANDLE (WINAPI *PF_CMPCREATEFILE)(VOID);
typedef BOOL (WINAPI *PF_ICMPCLOSEHANDLE)(HANDLE);
typedef DWORD (WINAPI *PF_ICMPSENDECHO)(HANDLE,DWORD,LPVOID,WORD,
PIPINFO,LPVOID,DWORD,DWORD);
//---------------------------------------------------------------------------
//AI(W906-PT-W2) 20260807: golden `typedef void (__closure *TPointVoidReceive)(char*,int);`
//  (golden :38) -- __closure has no standard-C++ spelling; translated per
//  vcl_compat.h's documented convention to a std::function typedef (same
//  choice EJ1N sibling BarCode_Helpers.h made for TPointVoidShowMsg).
typedef std::function<void(char*,int)> TPointVoidReceive;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class uSocketBase
{
private:    // User declarations
    AnsiString asAddress;
    AnsiString asPort;
public:     // User declarations
    TPointVoidReceive tpvReceive;
    char strReceiveUse[ReceiveLen];
    int iBufferLenght;
    //for setting
    AnsiString GetSettingSection(){return "SocketSetting";}
    void SetSocketAddress(AnsiString asValue){asAddress=asValue;}
    AnsiString GetSocketAddress(){return asAddress;}
    void SetSocketPort(AnsiString asValue){asPort=asValue;}
    AnsiString GetSocketPort(){return asPort;}
    //
    uSocketBase();
    ~uSocketBase();
    void InitialData();
    void ReloadData(AnsiString asSettingFileNameWithPath);
    void ReadSettingFile(AnsiString asSettingFileNameWithPath);
    void WriteSettingFile(AnsiString asSettingFileNameWithPath);
    void SetReceiveFunc(TPointVoidReceive _func);
    bool Ping(AnsiString asIP);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class uSocketServer : public uSocketBase
{
private:    // User declarations
    int DoCommuncationTask;
    TServerSocket *ServerSocket;
    //function
    void InitializationSocketServer();
    void ServerSocketConnect(TObject *Sender,
          TCustomWinSocket *Socket);
    void ServerSocketDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
    void ServerSocketError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
    void ServerSocketRead(TObject *Sender,
          TCustomWinSocket *Socket);
    bool MatchServerPort();
    //AI(W906-PT-W2) 20260807: golden `return ServerSocket->Socket->Connected;`
    //  (golden :85) -- see the .cpp banner's ADAPTATION (1): this port's
    //  TServerWinSocket never sets its OWN `Connected` field (only its
    //  per-connection children do -- vclcompat/ServerSocket.cpp never writes
    //  it), so a literal translation is permanently false and jams
    //  DoOpenCommuncation() in state 200 forever.  ActiveConnections>0 is the
    //  substrate field this shim DOES maintain for "does the server currently
    //  have an accepted client", and is the same field golden itself reads
    //  for the identical purpose elsewhere (e.g. SECSGEM/uHGemEquipment.cpp
    //  :3481 `srvGem->Socket->ActiveConnections`).  Golden's literal call is
    //  kept verbatim in the .cpp's GATE REGISTER, not here (this is just the
    //  declaration).
    bool IsConnected(){return ServerSocket->Socket->ActiveConnections>0;}
    bool Open();
public:     // User declarations
    uSocketServer();
    ~uSocketServer();
    void Initialization();
    int GetActiveConnections();
    AnsiString GetConnectClientAddress();
    bool DoOpenCommuncation();
    bool SendCommand(char* cSet,int iLen);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class uSocketClient : public uSocketBase
{
private:    // User declarations
    TClientSocket* ClientSocket;
    //function
    void ClientSocketRead(TObject *Sender,
        TCustomWinSocket *Socket);
    void ClientSocketError(TObject *Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void ClientSocketConnect(TObject *Sender,
      TCustomWinSocket *Socket);
    void ClientSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket);
    bool MatchClientSetting();
    bool bConnected;
public:     // User declarations
    uSocketClient();
    ~uSocketClient();
    void Initialization();
    void InitializationSocketClient();
    bool Open();
    bool IsConnected(){return bConnected  || ClientSocket->Active==true;}
    bool DoOpenCommuncation();
    void Close();
    bool SendCommand(char* cSet,int iLen);
    bool SetCommParameter(AnsiString asAddress,AnsiString asPort);
};
//---------------------------------------------------------------------------
#endif
