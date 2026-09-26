// ===========================================================================
//  TesterComm/Rs232/Rs232Support.cpp -- golden MyStringList.cpp (TMyStringList) + uSocketServerClient.cpp
//  (uSocketBase / uSocketServer / uSocketClient) of the RS232Standard program.
//
//  AI(W906-GB-P4) 20260926: Tester-comm plan P4.  Golden: D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410
//      MyStringList.h:19-58 / MyStringList.cpp:14-350
//          RS232Standard's OWN TMyStringList (the RS232 log: MainForm.cpp:361-364 slRS232Log, :2238, :2248).
//          Not the Handler's Public/MyStringList.
//      uSocketServerClient.h:10-118 / uSocketServerClient.cpp:11-415
//          The SOFT_SIMULTE tester-simulator socket (MainForm.cpp:379-383 uServer, :1845 DoOpenCommuncation,
//          :2169 SendCommand).  uSocketClient is translated too; golden MainForm never uses it.
//  Class shapes are fixed in Rs232Bridge.h; this file only defines their members, plus two file-static helpers for
//  vclcompat socket surface the golden bodies need and the shim lacks (each cited at its definition).
//
//  PROPERTIES.  Golden TMyStringList's __property Path/FileName/FirstRow/MaxLineCount/SaveType/AutoSave have plain
//  assignment setters (MyStringList.cpp:54-82).  Rs232Bridge.h models each as a public field plus a private
//  reference with the golden HT* name; both constructors bind the references in their mem-initializer lists, so the
//  golden bodies that write the property (`Path=...`) and the ones that read the backing field (`HTPath`) touch the
//  same object.  The setters are kept (golden text) and, as in golden, nothing calls them directly.
//
//  THREADING.  vclcompat TServerSocket / TClientSocket fire OnClientRead / OnRead (and connect / disconnect) on
//  THEIR threads in real mode, synchronously on the caller's thread in SIM.  The read handlers below fill
//  strReceiveUse and call tpvReceive exactly as golden; the callback TfRS232Main installs with SetReceiveFunc only
//  QueueRx()es a copy (TRANSLATION_RULES.md rule 14), so calling it from the shim's thread is correct.  This file
//  takes no lock.  TMyStringList is used only from the TesterComm thread (ShowCommData).
//
//  SOCKET MODE.  Both shims stay in their default SIM mode: this file never calls SetSimMode, and ServerSocket /
//  ClientSocket are private members, so nothing outside can flip them.  Golden MainForm never calls ReloadData, so
//  the server runs on uSocketBase::InitialData's "127.0.0.1" / "59999" and InitializationSocketServer's
//  Port=59999; no ini is read or written for it.
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

#include <winsock2.h>   // Ping(): WSAStartup / inet_addr / gethostbyname / gethostbyaddr / LPHOSTENT / INADDR_NONE.
                        // After the umbrella's <windows.h>, same order as Public/WinSocketErrorCode.cpp
                        // (vcl_compat.h:137-180 explains why it must not come first under MinGW).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace rs232std {

//==============================================================================
//  golden MyStringList.cpp
//==============================================================================

//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden `__fastcall TMyStringList::TMyStringList()` (MyStringList.cpp:14-24).  The
//   mem-initializer list binds the HT* references to the property fields (see the file banner); the body is golden.
TMyStringList::TMyStringList()
    : TStringList(),
      HTPath(Path),
      HTFileName(FileName),
      HTFirstRow(FirstRow),
      HTMaxLineCount(MaxLineCount),
      HTSaveType(SaveType),
      HTAutoSave(AutoSave)
{
    MaxLineCount        =1000;
    Path                ="D:\\RS232Log";
    FirstRow            ="";
    SaveType            =TByDay;
    AutoSave            =true;
    bUseFTRT            =false;
    bFilePathWithDate   =true;                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    MyList              =new TStringList();
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden MyStringList.cpp:26-38; references bound as in the default constructor.
TMyStringList::TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow)
    : TStringList(),
      HTPath(Path),
      HTFileName(FileName),
      HTFirstRow(FirstRow),
      HTMaxLineCount(MaxLineCount),
      HTSaveType(SaveType),
      HTAutoSave(AutoSave)
{
    MaxLineCount        =1;
    Path                =sPath;
    FileName            =sFileName;
    FirstRow            =sFirstRow;
    SaveType            =TByDay;

    AutoSave            =true;
    bUseFTRT            =false;
    bFilePathWithDate   =true;                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    MyList              =new TStringList();
}
//------------------------------------------------------------------------------
TMyStringList::~TMyStringList()
{
    try
    {
        MySaveToFile();
        MyList->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete MyList;
    }
    catch(...)
    {

    }
}
//------------------------------------------------------------------------------
void TMyStringList::SetPath(AnsiString P)
{
    HTPath=P;
}
//------------------------------------------------------------------------------
void TMyStringList::SetFileName(AnsiString P)
{
    HTFileName=P;
}
//------------------------------------------------------------------------------
void TMyStringList::SetFirstRow(AnsiString P)
{
    HTFirstRow=P;
}
//------------------------------------------------------------------------------
void TMyStringList::SetMaxLineCount(int Cnt)
{
    HTMaxLineCount=Cnt;
}
//------------------------------------------------------------------------------
void TMyStringList::SetSaveType(TSaveType Type)
{
    HTSaveType=Type;
}
//------------------------------------------------------------------------------
void TMyStringList::SetAutoSave(bool P)
{
    HTAutoSave=P;
}
//------------------------------------------------------------------------------
AnsiString TMyStringList::AddText(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2)
{
    AnsiString Str;
    GetTimeInfo();
    if(sMsg2=="" && sMsg1=="")
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName);
    }
    else if(sMsg2=="")
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sMsg1);
    }
    else
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\",\"%s\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sMsg1, sMsg2);
    }

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
    return Str;
}
//------------------------------------------------------------------------------
AnsiString TMyStringList::AddTextWithHex(AnsiString sUnitName, std::vector<Byte>bHex)
{
    AnsiString Str;
    GetTimeInfo();

    int iSize=bHex.size();
    AnsiString sDataHex="";
    AnsiString sDataAscii="";
    for(int i=0; i<iSize; i++)
    {
        sDataHex+=IntToHex((int)bHex[i], 2)+" ";
        sDataAscii+=MyDeCodeASCII((int)bHex[i]);
    }

    if(iSize==0)
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName);
    }
    else
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\", \"%s\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sDataAscii, sDataHex);
    }

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
    return Str;
}
//------------------------------------------------------------------------------
void TMyStringList::GetYesterdayInfo()
{
    static TDateTime dtPresent;
    //AI(W906-GB-P4) 20260926: dtPresent is pure scratch (assigned on the next line before any read), so it needs no
    //   g_rs232Life re-arm (TRANSLATION_RULES.md rule 13).
    //AI(W906-GB-P4) 20260926: golden `dtPresent=Now()-1;` (BCB6 TDateTime::operator-(int): minus one day).  Under
    //   vclcompat it is ambiguous (TDateTime::operator-(const TDateTime&) via TDateTime(double) vs the built-in
    //   double-int through operator double); same value spelled explicitly, as cpublic.cpp:461 does.
    dtPresent=TDateTime(Now().Val()-1.0);
    DecodeDate(dtPresent, SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday);
}
//------------------------------------------------------------------------------
void TMyStringList::MySaveToFile()
{
    AnsiString sPathName;
    AnsiString sFileName;
    AnsiString Str;
    FILE *pFile;

    if(HTAutoSave==false || MyList==NULL || MyList->Count==0)                   //沒資料就不用存檔
        return;

    sFileName=GetFileName();

    //AI(W906-GB-P4) 20260926: `MyList->Text` is a vclcompat TextProxy; wrapped in AnsiString() (no value change:
    //   TStringList::GetText joins every line with "\r\n", like BCB6 TStrings::GetTextStr).
    if(FileExists(sFileName)==false && HTFirstRow!="")
    {
        Str=HTFirstRow+"\r\n"+AnsiString(MyList->Text);
    }
    else
    {
        Str=AnsiString(MyList->Text);
    }

    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
    pFile=fopen(sFileName.c_str(), "a");
    if(pFile!=NULL)
    {
        fputs(Str.c_str(), pFile);
        fclose(pFile);
    }

    pFile=NULL;
    MyList->Clear();
}
//------------------------------------------------------------------------------
AnsiString TMyStringList::GetFileName()
{
    AnsiString sPathName;
    AnsiString sFileName="";
    AnsiString Str;
    int iHour;
    GetTimeInfo();

    if(HTPath=="")
        Path="D:\\HandlerLog";

    if(FileName=="")
        FileName="";

    if(bFilePathWithDate)                                                       //Steven 20210623 : 預設存檔要有日期當資料夾
    {
        if(HTSaveType>=TByMonth)                                                //使用年月存檔的話,就By年分類
        {
            sPathName.sprintf("%s\\%04d", HTPath, SystemYear);
        }
        else //if(HTSaveType>=TByDay)                                           //使用每日存檔的話,就By月分類
        {
            sPathName.sprintf("%s\\%04d\\%02d", HTPath, SystemYear, SystemMonth);
        }
    }
    else
    {
        sPathName=HTPath;
    }

    MyForceDirectories(sPathName);
    if(HTSaveType==TByMaxLineCount)
    {
        sFileName.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    }
    else
    {
        if(HTSaveType==TByHour)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
        }
        else if(HTSaveType==TBy2Hour)
        {
            iHour=SystemHour-SystemHour%2;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy4Hour)
        {
            iHour=SystemHour-SystemHour%4;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy6Hour)
        {
            iHour=SystemHour-SystemHour%6;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy8Hour)
        {
            iHour=SystemHour-SystemHour%8;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy12Hour)
        {
            if(SystemHour<8)
            {
                GetYesterdayInfo();
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday, 20);
            }
            else if(SystemHour>=20)
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 20);
            }
            else
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 8);
            }
        }
        else if(HTSaveType==TByDay)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
        }
        else if(HTSaveType==TByMonth)
        {
            sFileName.sprintf("%s\\%s_%04d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth);
        }
        else if(HTSaveType==TByMin)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        }
        else //if(HTSaveType==TByYear)
        {
            sFileName.sprintf("%s\\%s_%04d.log", sPathName, HTFileName, SystemYear);
        }
    }
    return sFileName;
}
//------------------------------------------------------------------------------
int TMyStringList::GetLastLine()                                                //Steven 20191016 : 取得目前檔案的行數
{
    AnsiString sFileName=GetFileName();
    TStringList *File;
    int iCount=0;

    if(FileExists(sFileName))                                                   //Steven 20200221 : 修正沒有檔案就不要讀檔
    {
        File=new TStringList();
        File->LoadFromFile(sFileName);
        iCount=File->Count;
        File->Clear();
        delete File;
    }
    sLastFileName=sFileName;                                                    //Steven 20191107 : 紀錄現在的檔名

    return iCount;
}
//------------------------------------------------------------------------------
void TMyStringList::MyInsertToFile(int iCount)
{
    if(HTAutoSave==false || MyList->Count==0)                                   //沒資料就不用存檔
        return;

    AnsiString Str;
    TStringList *File;
    File=new TStringList();

    if(FileExists(sLastFileName)==false)
    {
        sLastFileName=GetFileName();
    }
    else                                                                        //Steven 20200221 : 修正沒有檔案就不要讀檔
    {
        File->LoadFromFile(sLastFileName);
    }

    //AI(W906-GB-P4) 20260926: golden `Str=MyList->Text.SubString(0, MyList->Text.Length()-2);`.  Member access on the
    //   vclcompat TextProxy does not convert, so the property is wrapped in AnsiString() (rule 11); SubString(0, ...)
    //   keeps the BCB6 index-0-behaves-like-1 quirk (vclcompat AnsiString.cpp SubString), i.e. the text minus its
    //   trailing "\r\n".
    Str=AnsiString(MyList->Text).SubString(0, AnsiString(MyList->Text).Length()-2);
    Str.Insert(' ', 12);
    Str.Insert(' ', 26);

    if(FileExists(sLastFileName) && File->Count>iCount && iCount>0)             //Steven 20200215 : 加上保護機制
        File->Insert(iCount, Str);
    else
        File->Add(Str);

    File->SaveToFile(sLastFileName);                                            //Steven 20191107 : 紀錄現在的檔名
    File->Clear();
    MyList->Clear();
    delete File;
}

//==============================================================================
//  golden uSocketServerClient.cpp
//==============================================================================

//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden uSocketServerClient.h:80 `bool IsConnected(){return ServerSocket->Socket->Connected;}`
//   reads Connected on the LISTENING socket.  In VCL ScktComp that flag is set by TCustomWinSocket.DoListen once
//   listen() succeeds and cleared by Disconnect/Close (TCustomServerSocket.DoActivate compares Active against it),
//   i.e. it means "the server is listening".  vclcompat TServerSocket never writes its own Socket->Connected
//   (ServerSocket.cpp DoOpen_ / DoClose_ only move Impl::bActive; only accepted connections get Connected=true,
//   ServerSocket.cpp:233), so the literal read is false for ever: DoOpenCommuncation would cycle 50->100->200->50
//   and SendCommand would never send.  This helper returns the VCL value: the golden field, or the shim's own
//   "listening" state (IsActiveNow(): true between a successful Open() and Close(), in SIM and in real mode).
//   EJ1N/uSocketServerClient.h:191 answers a different question (ActiveConnections>0 = "a client is attached").
static bool ServerWinSocketConnected(TServerSocket* Server)
{
    return Server->Socket->Connected || Server->IsActiveNow();
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden `ServerSocket->Socket->Connections[0]->RemoteAddress` (uSocketServerClient.cpp:263).
//   vclcompat TCustomWinSocket has no RemoteAddress.  For a connection accepted by TServerSocket the shim stores the
//   peer's dotted quad in LocalAddress on both paths (ServerSocket.cpp:236 SIM, SimAcceptConnection's peerAddress;
//   :292-293 real, inet_ntoa of accept()'s peer sockaddr), which is the string VCL RemoteAddress (getpeername)
//   returns.  Same substitution as EJ1N/uSocketServerClient.cpp:470-474.  Valid for server-side connections only.
static AnsiString AcceptedPeerAddress(TCustomWinSocket* Conn)
{
    return Conn->LocalAddress;
}
//------------------------------------------------------------------------------
uSocketBase::uSocketBase()
{
    InitialData();
}
//------------------------------------------------------------------------------
uSocketBase::~uSocketBase()
{
//
}
//------------------------------------------------------------------------------
void uSocketBase::ReloadData(AnsiString asSettingFileNameWithPath)
{
    InitialData();
    ReadSettingFile(asSettingFileNameWithPath);
    WriteSettingFile(asSettingFileNameWithPath);
    iBufferLenght=0;
}
//------------------------------------------------------------------------------
void uSocketBase::InitialData()
{
    asAddress="127.0.0.1";
    asPort="59999";
}
//------------------------------------------------------------------------------
void uSocketBase::ReadSettingFile(AnsiString asSettingFileNameWithPath)
{
    TIniFile* IniFile=new TIniFile(asSettingFileNameWithPath);
    asAddress=IniFile->ReadString(GetSettingSection(), "asAddress", asAddress);
    asPort=IniFile->ReadString(GetSettingSection(), "asPort", asPort);
    delete IniFile;
}
//------------------------------------------------------------------------------
void uSocketBase::WriteSettingFile(AnsiString asSettingFileNameWithPath)
{
    TIniFile* IniFile=new TIniFile(asSettingFileNameWithPath);
    IniFile->WriteString(GetSettingSection(), "asAddress", asAddress);
    IniFile->WriteString(GetSettingSection(), "asPort", asPort);
    delete IniFile;
}
//------------------------------------------------------------------------------
void uSocketBase::SetReceiveFunc(TPointVoidReceive _func)
{
    tpvReceive=_func;
}
//------------------------------------------------------------------------------
bool uSocketBase::Ping(AnsiString asIP)
{
    HANDLE hIcmp = LoadLibrary("ICMP.DLL");         // ICMP.DLL load
    if(hIcmp == NULL)
        return false;
    //Get pointers to the functions
    //AI(W906-GB-P4) 20260926: golden passes the HANDLE straight to GetProcAddress / FreeLibrary (BCB6 accepted it);
    //   MinGW types both as HMODULE, so each call adds `(HMODULE)` -- the same pointer, as in
    //   EJ1N/uSocketServerClient.cpp:233-248.
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
        //AI(W906-GB-P4) 20260926: golden `(BYTE *)&iaDest`; MinGW declares the parameter `const char*` and
        //   unsigned char* does not convert implicitly.  Same bytes (EJ1N/uSocketServerClient.cpp:273-279).
        pHost=gethostbyaddr((const char *)(BYTE *)&iaDest, sizeof(struct in_addr), AF_INET);
    if(pHost==NULL)
    {
        WSACleanup();
        FreeLibrary((HMODULE)hIcmp);
        return false;
    }

    DWORD* pAddress=(DWORD*)(*pHost->h_addr_list);  // IP-Adresse copy
    HANDLE hIcmpFile=pfIcmpCreateFile();            //ICMP Echo Request Handle obtain
    ICMPECHO icmpEcho;                              // ICMP-Echo response buffer
    //AI(W906-GB-P4) 20260926: golden leaves icmpEcho uninitialised.  When IcmpSendEcho fails without writing a reply
    //   (a timeout, or the 28-byte reply buffer golden passes being judged too small), the dwSource / dwStatus reads
    //   below are indeterminate, which is undefined behaviour in C++.  Golden intent: only a written reply with
    //   status 0 is a success, so start from "no reply".  Identical whenever a reply is written.
    ::ZeroMemory(&icmpEcho, sizeof(icmpEcho));
    icmpEcho.dwStatus=(DWORD)-1;
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
//------------------------------------------------------------------------------
uSocketServer::uSocketServer()
{
    Initialization();
}
//------------------------------------------------------------------------------
uSocketServer::~uSocketServer()
{
    if(ServerSocket!=NULL)
    {
        delete ServerSocket;
    }
}
//------------------------------------------------------------------------------
void uSocketServer::Initialization()
{
    InitializationSocketServer();
    DoCommuncationTask=0;
}
//------------------------------------------------------------------------------
void uSocketServer::InitializationSocketServer()
{
    ServerSocket=new TServerSocket(NULL);
    ServerSocket->Port=59999;
    //AI(W906-GB-P4) 20260926: golden `ServerSocket->OnClientConnect=ServerSocketConnect;` etc. bind __closure method
    //   pointers; the vclcompat events are std::function, so each is a lambda capturing this and calling the golden
    //   member (they run on the shim's thread in real mode; see the file banner).
    ServerSocket->OnClientConnect=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketConnect(Sender, Socket); };
    ServerSocket->OnClientDisconnect=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketDisconnect(Sender, Socket); };
    ServerSocket->OnClientError=[this](TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
        { ServerSocketError(Sender, Socket, ErrorEvent, ErrorCode); };
    ServerSocket->OnClientRead=[this](TObject *Sender, TCustomWinSocket *Socket)
        { ServerSocketRead(Sender, Socket); };
}
//------------------------------------------------------------------------------
void uSocketServer::ServerSocketConnect(TObject *Sender,
          TCustomWinSocket *Socket)
{
    //
}
//------------------------------------------------------------------------------
void uSocketServer::ServerSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    //
}
//------------------------------------------------------------------------------
void uSocketServer::ServerSocketError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
      int &ErrorCode)
{
    //
}
//------------------------------------------------------------------------------
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
            //AI(W906-GB-P4) 20260926: golden hands tpvReceive a new[] copy (Buff), not strReceiveUse, and never frees
            //   it: ReceiveData_TCPIP (MainForm.cpp:3727-3744) copies cGet into ReceiveData and does not delete it.
            //   Kept as golden (a leak of iBufferLenght bytes per read, not UB).  In-process tpvReceive is the
            //   TfRS232Main QueueRx lambda, which copies the bytes before returning, so the leak is the only effect.
            char* Buff=new char[iBufferLenght];
            memcpy(Buff,strReceiveUse, iBufferLenght);
            tpvReceive(Buff, iBufferLenght);
        }
        catch(...)
        {}
    }
}
//------------------------------------------------------------------------------
int uSocketServer::GetActiveConnections()
{
    if(ServerSocket->Socket!=NULL)
        return ServerSocket->Socket->ActiveConnections;
    else
        return 0;
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
bool uSocketServer::MatchServerPort()
{
    if(GetSocketPort()=="" || GetSocketPort()=="0")
        return false;
    //AI(W906-GB-P4) 20260926: `int != AnsiString` compiles unchanged: vclcompat has a free
    //   operator!=(const AnsiString&, const AnsiString&) and an implicit AnsiString(int), so it is the decimal-string
    //   compare ("59999" vs asPort).  Either way Port ends up atoi(asPort).
    if(ServerSocket->Port!=GetSocketPort())
        ServerSocket->Port=atoi(GetSocketPort().c_str());
    return true;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden inline at uSocketServerClient.h:80 (moved out of line by Rs232Bridge.h); the
//   Connected read goes through ServerWinSocketConnected() above, which supplies the VCL meaning the shim lacks.
bool uSocketServer::IsConnected()
{
    return ServerWinSocketConnected(ServerSocket);
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
AnsiString uSocketServer::GetConnectClientAddress()
{
    if(IsConnected() && GetActiveConnections()==1)
        return AcceptedPeerAddress(ServerSocket->Socket->Connections[0]);   // golden ->RemoteAddress, see the helper
    return "None";
}
//------------------------------------------------------------------------------
bool uSocketServer::SendCommand(char* cSet, int iLen)
{
    if(IsConnected()==false || GetActiveConnections()!=1)
        return false;
    ServerSocket->Socket->Connections[0]->SendBuf(cSet, iLen);
    Sleep(50);
    return true;
}
//------------------------------------------------------------------------------
uSocketClient::uSocketClient()
{
    Initialization();
}
//------------------------------------------------------------------------------
uSocketClient::~uSocketClient()
{
    Close();
    if(ClientSocket!=NULL)
        delete ClientSocket;
}
//------------------------------------------------------------------------------
void uSocketClient::Initialization()
{
    InitializationSocketClient();
    bConnected=false;
}
//------------------------------------------------------------------------------
void uSocketClient::InitializationSocketClient()
{
    ClientSocket                =new TClientSocket(NULL);
    ClientSocket->Address       =GetSocketAddress();
    ClientSocket->Port          =atoi(GetSocketPort().c_str());
    //AI(W906-GB-P4) 20260926: golden __closure event assigns -> lambdas, as in InitializationSocketServer().
    ClientSocket->OnError       =[this](TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
        { ClientSocketError(Sender, Socket, ErrorEvent, ErrorCode); };
    ClientSocket->OnRead        =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketRead(Sender, Socket); };
    ClientSocket->OnConnect     =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketConnect(Sender, Socket); };
    ClientSocket->OnDisconnect  =[this](TObject *Sender, TCustomWinSocket *Socket)
        { ClientSocketDisconnect(Sender, Socket); };
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void uSocketClient::ClientSocketError(TObject *Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    bConnected=false;
    ErrorCode=0;
}
//------------------------------------------------------------------------------
void uSocketClient::ClientSocketConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    bConnected=true;
}
//------------------------------------------------------------------------------
void uSocketClient::ClientSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    bConnected=false;
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
bool uSocketClient::MatchClientSetting()
{
    if(bConnected!=ClientSocket->Active)
    {
        Close();
        return false;
    }
    if(GetSocketPort()=="" || GetSocketPort()=="0")
        return false;
    //AI(W906-GB-P4) 20260926: `ClientSocket->Port!=GetSocketPort()` is the decimal-string compare, as in
    //   uSocketServer::MatchServerPort.
    if(ClientSocket->Address!=GetSocketAddress() || ClientSocket->Port!=GetSocketPort())
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
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden inline at uSocketServerClient.h:113 (moved out of line by Rs232Bridge.h).
bool uSocketClient::IsConnected()
{
    return bConnected || ClientSocket->Active==true;
}
//------------------------------------------------------------------------------
void uSocketClient::Close()
{
    //AI(W906-GB-P4) 20260926: golden quirk kept -- ClientSocket is dereferenced before its NULL check (never NULL:
    //   InitializationSocketClient runs in the constructor).
    ClientSocket->Active=false;
    if(ClientSocket!=NULL)
        ClientSocket->Close();
}
//------------------------------------------------------------------------------
bool uSocketClient::SendCommand(char* cSet, int iLen)
{
    if(IsConnected()==false)
        return false;
    ClientSocket->Socket->SendBuf(cSet,iLen);
    Sleep(50);
    return true;
}
//------------------------------------------------------------------------------
bool uSocketClient::SetCommParameter(AnsiString asAddress, AnsiString asPort)
{
    SetSocketAddress(asAddress);
    SetSocketPort(asPort);
    return MatchClientSetting();
}
//------------------------------------------------------------------------------

}  // namespace rs232std
