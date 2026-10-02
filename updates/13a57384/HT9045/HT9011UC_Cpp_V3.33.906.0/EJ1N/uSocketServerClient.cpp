// =============================================================================
//  uSocketServerClient.cpp  --  generic raw-TCP client/server wrapper (Omron
//                                EJ1N family transport option).
//
//  Faithful translation of golden EJ1N/uSocketServerClient.cpp (425 lines,
//  BCB6, Big5/cp950 -- carries no Chinese comments).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 group "ej1n".  Second EJ1N/ file to land
//  (after EJ1N/TextProcess.cpp/.h, W906-GA-EJ1N).
//
//  ROLE: see this unit's own uSocketServerClient.h banner.  This .cpp holds
//  every method body: uSocketBase's settings-persistence + ICMP Ping, and the
//  two symmetrical TServerSocket/TClientSocket wrappers' tiny state machines.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim bodies -- all 36 golden function definitions):
//     uSocketBase::uSocketBase              golden :8-11
//     uSocketBase::~uSocketBase             golden :13-16
//     uSocketBase::ReloadData               golden :18-24
//     uSocketBase::InitialData              golden :26-30
//     uSocketBase::ReadSettingFile          golden :32-38
//     uSocketBase::WriteSettingFile         golden :40-46
//     uSocketBase::SetReceiveFunc           golden :48-51
//     uSocketBase::Ping                     golden :53-133
//     uSocketServer::uSocketServer          golden :136-139
//     uSocketServer::~uSocketServer         golden :141-147
//     uSocketServer::Initialization         golden :149-153
//     uSocketServer::InitializationSocketServer golden :155-163
//     uSocketServer::ServerSocketConnect    golden :165-169
//     uSocketServer::ServerSocketDisconnect golden :171-175
//     uSocketServer::ServerSocketError      golden :177-182
//     uSocketServer::ServerSocketRead       golden :184-201
//     uSocketServer::GetActiveConnections   golden :203-209
//     uSocketServer::DoOpenCommuncation     golden :211-235
//     uSocketServer::MatchServerPort        golden :237-244
//     uSocketServer::Open                   golden :246-257
//     uSocketServer::GetConnectClientAddress golden :259-264 (ADAPTATION 2)
//     uSocketServer::SendCommand            golden :266-273
//     uSocketClient::uSocketClient          golden :276-279
//     uSocketClient::~uSocketClient         golden :281-286
//     uSocketClient::Initialization         golden :288-292
//     uSocketClient::InitializationSocketClient golden :294-303
//     uSocketClient::ClientSocketRead       golden :305-320
//     uSocketClient::ClientSocketError      golden :322-335 (ADAPTATION 3)
//     uSocketClient::ClientSocketConnect    golden :337-341
//     uSocketClient::ClientSocketDisconnect golden :343-347
//     uSocketClient::DoOpenCommuncation     golden :349-361
//     uSocketClient::MatchClientSetting     golden :363-384
//     uSocketClient::Open                   golden :386-401
//     uSocketClient::Close                  golden :403-408
//     uSocketClient::SendCommand            golden :410-417
//     uSocketClient::SetCommParameter       golden :419-424
//   Plus the header's 7 inline one-liners (GetSettingSection/SetSocketAddress/
//   GetSocketAddress/SetSocketPort/GetSocketPort/uSocketServer::IsConnected/
//   uSocketClient::IsConnected) -- see uSocketServerClient.h, ADAPTATION 1
//   re-cited there for uSocketServer::IsConnected.
//   SATISFIED-BY-SUBSTRATE: TServerSocket/TServerWinSocket/TClientSocket/
//     TCustomWinSocket (vclcompat/ServerSocket.h + ClientSocket.h) supply
//     every ->Port/->Active/->Open()/->Close()/->Socket->.../->SendBuf/
//     ->ReceiveLength/->ReceiveBuf call golden makes.  TIniFile
//     (vclcompat/IniFiles.h) supplies ReadString/WriteString.  WinSock (raw
//     gethostbyname/WSAStartup/inet_addr/ZeroMemory/LoadLibrary/
//     GetProcAddress/FreeLibrary) comes from MachineDefine.h's own explicit
//     `#include <winsock.h>` (MachineDefine.h:11) -- see the INCLUDE ORDER
//     note below for why this file adds NO extra winsock guard of its own.
//
//  GATE REGISTER -- exactly ONE #if 0/#else macro-pair gate (golden property
//  genuinely absent from this port's substrate, not merely renamed):
//   (1) `ServerSocket->Socket->Connections[0]->RemoteAddress` golden :262 --
//       vclcompat/ClientSocket.h's TCustomWinSocket has NO `RemoteAddress`
//       member (grepped the whole port tree: zero matches for
//       `RemoteAddress` outside this comment).  It DOES carry a
//       `LocalAddress` field, and vclcompat/ServerSocket.cpp populates that
//       field, for every accepted connection, with the PEER's dotted-quad
//       (Sim: SimAcceptConnection's `peerAddress` argument,
//       ServerSocket.cpp:236; Real: `::inet_ntoa(peerAddr.sin_addr)` off the
//       accept()'d sockaddr_in, ServerSocket.cpp:292-293) -- i.e. this shim's
//       `LocalAddress` field IS, in every code path, the accepted
//       connection's remote/peer address (a naming choice this port made to
//       match golden's OWN SECSGEM citation, `Socket->LocalAddress` read on
//       the just-accepted connection in srvGemClientConnect -- see
//       ClientSocket.h's AI(W906-ServerSocket) EXTENSION note).  Using it
//       here delivers the IDENTICAL observable string golden's
//       `->RemoteAddress` would have returned; the golden call is preserved
//       verbatim in the #if 0 arm purely so a future `RemoteAddress` alias
//       lands with the exact citation, and there is NO behaviour delta on
//       either the Sim or Real path.
//   (2) `Socket->Close();` golden :329 (uSocketClient::ClientSocketError) --
//       this port's TCustomWinSocket has NO Close() method (grepped: zero
//       matches for a Close() declaration on that class; only TClientSocket/
//       TServerSocket have one).  It DOES carry Disconnect(int Port), added
//       specifically for this exact "close the connection from inside an
//       OnError handler" golden idiom (see ClientSocket.h's own SCOPED API
//       SURFACE citation, grounded in MyPLC/ModbusTCPClient.cpp's own OnError
//       handler doing the identical thing with the OTHER BCB6 spelling,
//       `Socket->Disconnect(Socket->RemotePort)`).  ACTIVE arm uses that
//       call; NO behaviour delta -- this handler already forces
//       bConnected=false itself one line above, independent of Socket's own
//       torn-down state, and Disconnect() tears the connection down exactly
//       as Close() would have.  Re-cited at its own call site.
//   Also documented (NOT a #if 0 gate -- the symbol has a real compiled body,
//   just one this shim never populates for the reason cited): the
//   `uSocketServer::IsConnected()` ADAPTATION is declared and cited entirely
//   in uSocketServerClient.h (golden :85) -- not repeated here since it is a
//   header inline, not a .cpp definition.
//
//  MINIMAL NECESSARY ADAPTATIONS (3, all flagged inline with //AI at the
//  line, none changes observable behaviour):
//   (3) golden `GetProcAddress(hIcmp, ...)` / `FreeLibrary(hIcmp)` (golden
//       :59-67,:127) pass a plain HANDLE straight through; MinGW's
//       <winbase.h> types both parameters as HMODULE/HINSTANCE (distinct
//       pointer types from HANDLE under -fpermissive's stricter default --
//       BCB6 accepted the implicit conversion). Every such call site adds an
//       explicit `(HMODULE)` cast -- HANDLE and HMODULE are both opaque
//       `void*` on Win32, so this is purely a portability cast.
//   (4) golden `gethostbyaddr((BYTE *)&iaDest, ...)` (golden :92) -- MinGW's
//       <winsock.h> declares the first parameter `const char*`; BYTE*
//       (unsigned char*) does not implicitly convert to that under
//       -fpermissive's stricter default. Recast to `(const char *)&iaDest`
//       -- same bytes, same call.
//   (5) `LogClientSocketExceptionError(Sender, "ClientSocket Error");`
//       (golden :333) -- golden `Public/WinSocketErrorCode.h`'s
//       LogClientSocketExceptionError is itself DEFERRED in this port
//       (vclcompat-adjacent Public/WinSocketErrorCode.h/.cpp's own banner:
//       "couples to VCL TClientSocket->Name/Address/Port and MyDBIProcess...
//       Omitted from this translation unit").  Every other unit that reaches
//       this same golden call already resolves it with its OWN TU-local
//       no-op wrapper rather than waiting on that deferred symbol --
//       Interface/TesterTCP_Socket.cpp's Gated_LogClientSocketExceptionError,
//       Automation/automation.cpp's W906Auto_LogClientSocketExceptionError,
//       SECSGEM/uHGemEquipment.cpp's LogClientSocketExceptionError_ -- this
//       is the SAME, already-established substitution, named
//       EJ1N_LogClientSocketExceptionError for this TU.  Conservative no-op:
//       golden's own role here is best-effort DIAGNOSTIC logging of an
//       exception the caller already caught and already acted on
//       (Disconnect just above, inside the very same catch -- see GATE (2)),
//       so dropping the extra log line is behaviour-neutral for control flow.
//
//  INCLUDE ORDER NOTE (why no extra winsock guard is added here): golden's
//  own uSocketBase::Ping() (:53-133) calls raw WinSock1.1 API directly
//  (WSAStartup(0x0101,...), gethostbyname, gethostbyaddr, inet_addr,
//  LPHOSTENT, struct in_addr) with NO extra include beyond golden's own
//  `#include "MachineDefine.h"`.  vclcompat/vcl_compat.h's own banner records
//  a verified MinGW-specific hazard: pre-including <winsock2.h> ahead of the
//  umbrella's <windows.h> "actively BREAKS MinGW" (defeats winerror.h's
//  general Win32 error codes for the rest of the TU -- vcl_compat.h:137-165)
//  and is therefore gated `#if defined(_MSC_VER)` there, NOT applied
//  unconditionally.  This file does not reference any of the affected error
//  codes (ERROR_SHARING_VIOLATION / ERROR_LOCK_VIOLATION), and
//  MachineDefine.h's own explicit `#include <winsock.h>` (MachineDefine.h:11,
//  reached via this file's own `#include "MachineDefine.h"` below, AFTER
//  vcl_compat.h has already run its MSVC-only winsock2 pre-include and its
//  own plain <windows.h> at vcl_compat.h:191) is exactly what golden itself
//  relies on for every WinSock symbol Ping() touches -- so this file adds
//  NOTHING beyond golden's own include line.  Verified by the clean
//  -fsyntax-only run recorded at the end of this banner's wave.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :6).  `__fastcall` dropped from all four event
//  handler definitions (neutralized to nothing by vcl_compat.h; dropped here
//  for readability, matching every other ported unit -- the .h already drops
//  it from the declarations).  `TObject *Sender, TCustomWinSocket *Socket`
//  parameter shapes kept EXACT.  AnsiString throughout (no std::string).
//  Numeric semantics, magic numbers (59999, 0x0101, 500ms, Sleep(50), the
//  DoCommuncationTask cursor values 0/50/100/200) kept EXACT -- no rewrites.
//
//  Big5: this golden .cpp has ZERO Chinese comments (confirmed by decoding
//  the raw bytes as cp950 and inspecting every line) -- nothing to preserve,
//  nothing to gate on U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "uSocketServerClient.h" ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella +
                                    //   <windows.h>/<winsock.h> + portable STL);
                                    //   see the INCLUDE ORDER NOTE above for why
                                    //   this is sufficient for Ping()'s raw WinSock
                                    //   calls with no extra guard in this TU.
#include "EJ1N/uSocketServerClient.h"       // this unit's own contract
//---------------------------------------------------------------------------
uSocketBase::uSocketBase()
{
    InitialData();
}
//---------------------------------------------------------------------------
uSocketBase::~uSocketBase()
{
//
}
//---------------------------------------------------------------------------
void uSocketBase::ReloadData(AnsiString asSettingFileNameWithPath)
{
    InitialData();
    ReadSettingFile(asSettingFileNameWithPath);
    WriteSettingFile(asSettingFileNameWithPath);
    iBufferLenght=0;
}
//---------------------------------------------------------------------------
void uSocketBase::InitialData()
{
    asAddress="127.0.0.1";
    asPort="59999";
}
//---------------------------------------------------------------------------
void uSocketBase::ReadSettingFile(AnsiString asSettingFileNameWithPath)
{
    TIniFile* IniFile=new TIniFile(asSettingFileNameWithPath);
    asAddress=IniFile->ReadString(GetSettingSection(), "asAddress", asAddress);
    asPort=IniFile->ReadString(GetSettingSection(), "asPort", asPort);
    delete IniFile;
}
//---------------------------------------------------------------------------
void uSocketBase::WriteSettingFile(AnsiString asSettingFileNameWithPath)
{
    TIniFile* IniFile=new TIniFile(asSettingFileNameWithPath);
    IniFile->WriteString(GetSettingSection(), "asAddress", asAddress);
    IniFile->WriteString(GetSettingSection(), "asPort", asPort);
    delete IniFile;
}
//---------------------------------------------------------------------------
void uSocketBase::SetReceiveFunc(TPointVoidReceive _func)
{
    tpvReceive=_func;
}
//---------------------------------------------------------------------------
bool uSocketBase::Ping(AnsiString asIP)
{
    HANDLE hIcmp = LoadLibrary("ICMP.DLL");         // ICMP.DLL load
    if(hIcmp == NULL)
        return false;
    //Get pointers to the functions
    //AI(W906-PT-W2) 20260807: golden passes `hIcmp` (a plain HANDLE, the
    //  LoadLibrary result) straight to GetProcAddress/FreeLibrary, which BCB6
    //  accepted without complaint; MinGW's <winbase.h> types both as taking
    //  HMODULE/HINSTANCE (distinct pointer types from HANDLE under
    //  -fpermissive's stricter default), so every such call below adds an
    //  explicit `(HMODULE)` cast.  HANDLE and HMODULE are both opaque `void*`
    //  on Win32 -- purely a portability cast, no behaviour change.
    PF_CMPCREATEFILE pfIcmpCreateFile = (PF_CMPCREATEFILE)
    GetProcAddress((HMODULE)hIcmp, "IcmpCreateFile");
    PF_ICMPCLOSEHANDLE pfIcmpCloseHandle = (PF_ICMPCLOSEHANDLE)
    GetProcAddress((HMODULE)hIcmp, "IcmpCloseHandle");
    PF_ICMPSENDECHO pfIcmpSendEcho = (PF_ICMPSENDECHO)
    GetProcAddress((HMODULE)hIcmp, "IcmpSendEcho");
    if(pfIcmpCreateFile==NULL || pfIcmpCloseHandle==NULL || pfIcmpSendEcho==NULL)
    {
        FreeLibrary((HMODULE)hIcmp);
        return false;
    }
    WSADATA wsaData; //WinSock initialize
    int ilRetVal=WSAStartup(0x0101, &wsaData);      //initial WINSOCK
    if(ilRetVal)
    {
        WSACleanup();
        FreeLibrary((HMODULE)hIcmp);
        return false;
    }

    if(0x0101!=wsaData.wVersion)                    //Check WinSock version
    {
        WSACleanup();
        FreeLibrary((HMODULE)hIcmp);
        return false;
    }
    //check host name
    struct in_addr iaDest;                          // Structure for the Internet address
    iaDest.s_addr=inet_addr(asIP.c_str());
    LPHOSTENT pHost;                                // Pointer to the Host Entry structure
    if(iaDest.s_addr==INADDR_NONE)
        pHost=gethostbyname(asIP.c_str());
    else
        //AI(W906-PT-W2) 20260807: golden casts to `(BYTE *)`; MinGW's
        //  <winsock.h> declares gethostbyaddr's first parameter as
        //  `const char*`, and BYTE* (unsigned char*) does not implicitly
        //  convert to that under -fpermissive's stricter default (BCB6 was
        //  looser). Recast to `(const char *)` -- same bytes, same call,
        //  purely a portability cast.
        pHost=gethostbyaddr((const char *)&iaDest, sizeof(struct in_addr), AF_INET);
    if(pHost==NULL)
    {
        WSACleanup();
        FreeLibrary((HMODULE)hIcmp);
        return false;
    }

    DWORD* pAddress=(DWORD*)(*pHost->h_addr_list);  // IP-Adresse copy
    HANDLE hIcmpFile=pfIcmpCreateFile();            //ICMP Echo Request Handle obtain
    ICMPECHO icmpEcho;                              // ICMP-Echo response buffer
    IPINFO ipInfo;                                  // IP options structure
    int ilCount=0;                                  // Number of Round Trip Time data
    for(int ilPingNo=0; ilPingNo<1; ilPingNo++)
    {
        ::ZeroMemory(&ipInfo, sizeof(ipInfo));
        ipInfo.bTimeToLive=255;

        // Request ICMP Echo
        pfIcmpSendEcho(hIcmpFile,                   // Handle of IcmpCreateFile
                       *pAddress,                   // destination IP address
                       NULL,                        // Pointer to the buffer with the
                       0,                           // Buffergrosse in Bytes
                       &ipInfo,                     // Request Options
                       &icmpEcho,                   // response buffer
                       sizeof(struct tagICMPECHO),  // Buffergrosse
                       500);                        // Max. wait time in milliseconds

        iaDest.s_addr=icmpEcho.dwSource;
        if(icmpEcho.dwStatus)
            break;
        ilCount++;
    }

    pfIcmpCloseHandle(hIcmpFile);                   // Close echo request file handle
    FreeLibrary((HMODULE)hIcmp);
    WSACleanup();                                   //release WINSOCK
    if(ilRetVal!=ilCount)
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
uSocketServer::uSocketServer()
{
    Initialization();
}
//---------------------------------------------------------------------------
uSocketServer::~uSocketServer()
{
    if(ServerSocket!=NULL)
    {
        delete ServerSocket;
    }
}
//---------------------------------------------------------------------------
void uSocketServer::Initialization()
{
    InitializationSocketServer();
    DoCommuncationTask=0;
}
//---------------------------------------------------------------------------
void uSocketServer::InitializationSocketServer()
{
    ServerSocket=new TServerSocket(NULL);
    ServerSocket->Port=59999;
    //AI(W906-PT-W2) 20260807: golden `__closure` bound-method event assigns
    //  (`ServerSocket->OnClientConnect=ServerSocketConnect;` etc, golden
    //  :159-162) implicitly bind `this`; the port's TSocketNotifyEvent/
    //  TSocketErrorEvent are std::function, so each is wrapped in a lambda
    //  capturing `this` -- same translation choice as Automation/
    //  automation.cpp's OLPServer wiring (automation.cpp:191-199).
    ServerSocket->OnClientConnect=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketConnect(Sender, Socket); };
    ServerSocket->OnClientDisconnect=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketDisconnect(Sender, Socket); };
    ServerSocket->OnClientError=[this](TObject *Sender, TCustomWinSocket *Socket,
        TErrorEvent ErrorEvent, int &ErrorCode)
        { ServerSocketError(Sender, Socket, ErrorEvent, ErrorCode); };
    ServerSocket->OnClientRead=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketRead(Sender, Socket); };
}
//---------------------------------------------------------------------------
void uSocketServer::ServerSocketConnect(TObject *Sender,
          TCustomWinSocket *Socket)
{
    //
}
//---------------------------------------------------------------------------
void uSocketServer::ServerSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    //
}
//---------------------------------------------------------------------------
void uSocketServer::ServerSocketError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
      int &ErrorCode)
{
    //
}
//---------------------------------------------------------------------------
void uSocketServer::ServerSocketRead(TObject *Sender,
      TCustomWinSocket *Socket)
{
    iBufferLenght=Socket->ReceiveLength();
    if(iBufferLenght>0)
    {
        iBufferLenght=iBufferLenght>ReceiveLen?ReceiveLen:iBufferLenght;
        Socket->ReceiveBuf(strReceiveUse, iBufferLenght);
        try
        {
            char* Buff=new char[iBufferLenght];
            memcpy(Buff,strReceiveUse, iBufferLenght);
            tpvReceive(Buff, iBufferLenght);
        }
        catch(...)
        {}
    }
}
//---------------------------------------------------------------------------
int uSocketServer::GetActiveConnections()
{
    if(ServerSocket->Socket!=NULL)
        return ServerSocket->Socket->ActiveConnections;
    else
        return 0;
}
//---------------------------------------------------------------------------
bool uSocketServer::DoOpenCommuncation()
{
    switch(DoCommuncationTask)
    {
        case 0:
            DoCommuncationTask=50;
            break;
        case 50:
            if(MatchServerPort())
                DoCommuncationTask=100;
            break;
        case 100:
            if(Open())
                DoCommuncationTask=200;
            else
                DoCommuncationTask=50;
            break;
        case 200:
            if(IsConnected())
                return true;
            else
                DoCommuncationTask=50;
    }
    return false;
}
//---------------------------------------------------------------------------
bool uSocketServer::MatchServerPort()
{
    if(GetSocketPort()=="" || GetSocketPort()=="0")
        return false;
    if(ServerSocket->Port!=GetSocketPort())
        ServerSocket->Port=atoi(GetSocketPort().c_str());
    return true;
}
//---------------------------------------------------------------------------
bool uSocketServer::Open()
{
    try
    {
        ServerSocket->Open();
    }
    catch(...)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
AnsiString uSocketServer::GetConnectClientAddress()
{
    if(IsConnected() && GetActiveConnections()==1)
        //AI(W906-PT-W2) 20260807: GATE (1) -- golden
        //  `ServerSocket->Socket->Connections[0]->RemoteAddress`.  This port's
        //  TCustomWinSocket has no RemoteAddress member; `LocalAddress` is
        //  the field vclcompat/ServerSocket.cpp populates with the accepted
        //  connection's PEER dotted-quad on every path (Sim :236, Real
        //  :292-293) -- an intentional naming choice this port made to match
        //  golden's OWN SECSGEM citation for the identical field (see
        //  ClientSocket.h's AI(W906-ServerSocket) EXTENSION note).  Same
        //  observable string either way -- NO behaviour delta.  See this
        //  file's own GATE REGISTER above.
#if 0
        return ServerSocket->Socket->Connections[0]->RemoteAddress;
#else
        return ServerSocket->Socket->Connections[0]->LocalAddress;
#endif
    return "None";
}
//---------------------------------------------------------------------------
bool uSocketServer::SendCommand(char* cSet, int iLen)
{
    if(IsConnected()==false || GetActiveConnections()!=1)
        return false;
    ServerSocket->Socket->Connections[0]->SendBuf(cSet, iLen);
    Sleep(50);
    return true;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
uSocketClient::uSocketClient()
{
    Initialization();
}
//---------------------------------------------------------------------------
uSocketClient::~uSocketClient()
{
    Close();
    if(ClientSocket!=NULL)
        delete ClientSocket;
}
//---------------------------------------------------------------------------
void uSocketClient::Initialization()
{
    InitializationSocketClient();
    bConnected=false;
}
//---------------------------------------------------------------------------
void uSocketClient::InitializationSocketClient()
{
    ClientSocket                =new TClientSocket(NULL);
    ClientSocket->Address       =GetSocketAddress();
    ClientSocket->Port          =atoi(GetSocketPort().c_str());
    //AI(W906-PT-W2) 20260807: golden `__closure` bound-method event assigns
    //  (golden :299-302) -- same lambda-wrap translation as
    //  InitializationSocketServer() above.
    ClientSocket->OnError       =[this](TObject *Sender, TCustomWinSocket *Socket,
        TErrorEvent ErrorEvent, int &ErrorCode)
        { ClientSocketError(Sender, Socket, ErrorEvent, ErrorCode); };
    ClientSocket->OnRead        =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketRead(Sender, Socket); };
    ClientSocket->OnConnect     =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketConnect(Sender, Socket); };
    ClientSocket->OnDisconnect  =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketDisconnect(Sender, Socket); };
}
//---------------------------------------------------------------------------
void uSocketClient::ClientSocketRead(TObject *Sender,
        TCustomWinSocket *Socket)
{
    iBufferLenght=Socket->ReceiveLength();
    if(iBufferLenght>0)
    {
        iBufferLenght=iBufferLenght>ReceiveLen?ReceiveLen:iBufferLenght;
        Socket->ReceiveBuf(strReceiveUse, iBufferLenght);
        try
        {
            tpvReceive(strReceiveUse, iBufferLenght);
        }
        catch(...)
        {}
    }
}
//---------------------------------------------------------------------------
//AI(W906-PT-W2) 20260807: EJ1N_LogClientSocketExceptionError -- golden
//  Public/WinSocketErrorCode.h`s LogClientSocketExceptionError (golden call
//  site :333) is DEFERRED in this port (Public/WinSocketErrorCode.h`s own
//  banner: "couples to VCL TClientSocket->Name/Address/Port and
//  MyDBIProcess... Omitted from this translation unit"). Every other unit
//  reaching this same golden call already resolves it with its own TU-local
//  no-op wrapper rather than waiting on the deferred symbol --
//  Interface/TesterTCP_Socket.cpp's Gated_LogClientSocketExceptionError,
//  Automation/automation.cpp's W906Auto_LogClientSocketExceptionError,
//  SECSGEM/uHGemEquipment.cpp's LogClientSocketExceptionError_ -- this is the
//  SAME, already-established substitution. See ADAPTATION (3) in the file
//  banner: conservative no-op, behaviour-neutral (best-effort diagnostic log
//  of an exception the caller already caught and already acted on).
//---------------------------------------------------------------------------
static void EJ1N_LogClientSocketExceptionError(TObject * /*Sender*/, const AnsiString & /*Context*/)
{
}
//---------------------------------------------------------------------------
void uSocketClient::ClientSocketError(TObject *Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    bConnected=false;
    ErrorCode=0;
    try
    {
        //AI(W906-PT-W2) 20260807: GATE (2) -- golden `Socket->Close();`
        //  (golden :329). Real BCB6 TCustomWinSocket carries a Close()
        //  method; this port's vclcompat/ClientSocket.h TCustomWinSocket does
        //  not (grepped the whole port tree: zero matches for
        //  `TCustomWinSocket::Close` or a `Close()` declaration on that
        //  class -- only TClientSocket/TServerSocket have one). It DOES
        //  carry Disconnect(int Port), added specifically to satisfy this
        //  exact "close the connection from inside an OnError handler"
        //  golden idiom -- see ClientSocket.h's own SCOPED API SURFACE
        //  citation: "golden SocketError handler calls
        //  Disconnect(Socket->RemotePort); kept faithfully even though
        //  vanilla documented VCL TCustomWinSocket does not advertise a
        //  Disconnect(int) overload". That citation is MyPLC/
        //  ModbusTCPClient.cpp's OWN OnError handler doing the identical
        //  thing this golden file's OnError handler does with a differently-
        //  spelled BCB6 call. Disconnect(RemotePort) marks Connected=false
        //  and tears down the connection -- the same net effect Close()
        //  would have here (this handler already forces bConnected=false
        //  itself the line above, independent of Socket's own state).
#if 0
        Socket->Close();
#else
        Socket->Disconnect(Socket->RemotePort);
#endif
    }
    catch(...)
    {
        EJ1N_LogClientSocketExceptionError(Sender, "ClientSocket Error");
    }
}
//---------------------------------------------------------------------------
void uSocketClient::ClientSocketConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    bConnected=true;
}
//---------------------------------------------------------------------------
void uSocketClient::ClientSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    bConnected=false;
}
//---------------------------------------------------------------------------
bool uSocketClient::DoOpenCommuncation()
{
    if(MatchClientSetting() &&
       Open()               &&
       bConnected)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
bool uSocketClient::MatchClientSetting()
{
    if(bConnected!=ClientSocket->Active)
    {
        Close();
        return false;
    }

    if(GetSocketPort()=="" || GetSocketPort()=="0")
        return false;
    if(ClientSocket->Address!=GetSocketAddress() || ClientSocket->Port!=atof(GetSocketPort().c_str()))  //AI(W906-NUMCMP) 20260927: golden `Port!=GetSocketPort()` (number on the left) is Variant(Port)!=Variant(str) in BCB6 = NUMERIC (NB2 R89), so "05000"/"5000 " match 5000; vclcompat compared strings and reconnected every tick.  atof, not ToDouble(): a non-numeric port throws in both BCB6 (EVariantError, caught by VCL) and vclcompat (std::runtime_error, uncaught here) -- atof reads it as 0 instead
    {
        if(IsConnected())
        {
            Close();
            return false;
        }
        ClientSocket->Address=GetSocketAddress();
        ClientSocket->Port=atoi(GetSocketPort().c_str());
    }
    return true;
}
//---------------------------------------------------------------------------
bool uSocketClient::Open()
{
    if(IsConnected())
        return true;
    try
    {
        Close();
        ClientSocket->Open();
    }
    catch(...)
    {
        bConnected=false;
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void uSocketClient::Close()
{
    ClientSocket->Active=false;
    if(ClientSocket!=NULL)
        ClientSocket->Close();
}
//---------------------------------------------------------------------------
bool uSocketClient::SendCommand(char* cSet, int iLen)
{
    if(IsConnected()==false)
        return false;
    ClientSocket->Socket->SendBuf(cSet,iLen);
    Sleep(50);
    return true;
}
//---------------------------------------------------------------------------
bool uSocketClient::SetCommParameter(AnsiString asAddress, AnsiString asPort)
{
    SetSocketAddress(asAddress);
    SetSocketPort(asPort);
    return MatchClientSetting();
}
//---------------------------------------------------------------------------
//AI(W906-I03) 20261002 (Ifor01): port-only, I-03 phase 2 (the DTME08 Modbus/TCP link).
//  golden's `new TClientSocket(NULL)` (InitializationSocketClient, golden :296) is ctNonBlocking -- the VCL default:
//  Open() returns at once, and the connect / read / disconnect come back as window messages that the main thread's
//  message loop delivers between timer events.  vclcompat models exactly that with POLLED mode (vclcompat/ClientSocket.cpp,
//  end of file, AI(W906-H008)): a non-blocking connect, and Poll() -- called by the owner on ITS thread -- finishes the
//  connect, reads, and fires OnConnect / OnRead / OnDisconnect / OnError right there.  So an unplugged or unreachable
//  controller never stalls the caller (the blocking Real mode would wait the OS connect timeout, ~21 s, on the serve loop).
//  Opt-in per instance: every other uSocketClient keeps vclcompat's default (SIM, no network).
void uSocketClient::W906_UsePolledRealSocket()
{
    if(ClientSocket==NULL)
        return;
    ClientSocket->SetPolled(true);
    ClientSocket->SetSimMode(false);
}
//---------------------------------------------------------------------------
void uSocketClient::W906_Poll()
{
    if(ClientSocket!=NULL && ClientSocket->IsPolled())
        ClientSocket->Poll();
}
//---------------------------------------------------------------------------
