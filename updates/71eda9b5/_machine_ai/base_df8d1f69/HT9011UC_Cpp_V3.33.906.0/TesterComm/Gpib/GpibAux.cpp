// ===========================================================================
//  TesterComm/Gpib/GpibAux.cpp -- golden RS232.cpp (TfRS232Main) + DummyArt.cpp (TfDummyART).
//
//  AI(W906-GB-P1) 20260926: Tester-comm plan P1.  Golden: D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525
//      RS232.h:15-18 / RS232.cpp:20-665 / RS232.dfm:16-46
//          JerryYang 20160630 "RS232加入GPIB": the RS232 line INSIDE the GPIB program.  It is NOT the separate
//          RS232Standard program (that one is its own engine); it only shares that program's Setup.ini.
//      DummyArt.cpp:17-441 / DummyArt.dfm
//          Steven 20170413 (wei) ART simulator.
//  Class shapes are fixed in GpibBridge.h; this file only defines their members and the two form globals.
//
//  THREADING.  CommTester->OnReceiveData is fired by vclcompat TComm on ITS reader thread, so the callback only
//  QueueRx()es the bytes; TSerialPoll::DrainRx() (TesterComm thread) calls CommTesterReceiveData, like golden where
//  SPComm posts the data to the form's thread.  DummyARTTimer1 is a headless TTimer: the engine owner calls
//  DummyARTTimer1Timer while DummyARTTimer1->Enabled and its Interval elapsed.  No lock is taken in this file: the
//  widget state is guarded by TSerialPoll::uiMutex, which belongs to the engine's loop (WbMutex must not be
//  re-entered, WebBridge/Sync.h:33-35).
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
#include <vector>

//------------------------------------------------------------------------------
// golden RS232.h:15-18
#ifndef _STX_
#define _STX_     0x02
#endif
#ifndef _ETX_
#define _ETX_     0x03
#endif
#ifndef _ENQ_
#define _ENQ_     0x05                                                          //Steven 20181026 : 重新按照ASCII標準定義 _ENQ_ 與 _ACK_
#endif
#ifndef _ACK_
#define _ACK_     0x06
#endif

//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden cmydef.h (GPIB copy) customer codes used by OpenTesterComm (cmydef.h:45/137-140/
//  228/275).  GpibBridge.h does not carry the CC_ table; the Handler's MachineType.h:224/319-322/410/456 defines the
//  same seven with the same values, hence the #ifndef guards (an identical later #define is also legal).
#ifndef CC_HONPREC_QC
#define CC_HONPREC_QC             0
#endif
#ifndef CC_MAXIM_THAILAND
#define CC_MAXIM_THAILAND       860 //MAXIM 泰國
#endif
#ifndef CC_Microchip_Thai
#define CC_Microchip_Thai       861 //Microchip 泰國
#endif
#ifndef CC_Microchip_Phil
#define CC_Microchip_Phil       862 //Microchip 菲律賓
#endif
#ifndef CC_Microchip_China
#define CC_Microchip_China      863 //Microchip 中國
#endif
#ifndef CC_SCC
#define CC_SCC                  943 //JSCC 江陰
#endif
#ifndef CC_MAXIM
#define CC_MAXIM                985
#endif

namespace gpibbridge {

//AI(W906-GB-P1) 20260926: golden RS232.cpp:20 `TfRS232Main *fRS232Main;` and DummyArt.cpp:17 `TfDummyART *fDummyART;`
//   are defined once, in GpibGlobals.cpp (laptop gate 3e8534c9: duplicate strong symbols; GpibBridge.h has the externs).
//AI(W906-GB-P1) 20260926: golden DummyArt.cpp:18 `extern bool bSimulate;` -- already declared by GpibBridge.h.
static bool bTimerRun=false;                                                    // golden DummyArt.cpp:19 (external there; only DummyArt.cpp uses it)

//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: RS232 widget fallback bookkeeping (see TfRS232Main ctor).  Golden never needs it: its
//  constructor aliases SerialPoll's widgets and SerialPoll is always created first (H9046_32GPIB.cpp:19-22).
static std::set<const TfRS232Main*>& RsFallbackOwners()
{
    static std::set<const TfRS232Main*> s;
    return s;
}

//AI(W906-GB-P1) 20260926: golden DoRevCommand does `strcpy(cX, sX.c_str())` into fixed char buffers (cmydef
//  cNumOfSites[10], cMachineStateDec[6], cSoakTime[10], cJamCode[10], cDoubleContactCount[5], cAllMassTemp[100],
//  cHandlerID[100], cSiteMap[256], cSiteOnOff[256], and one local char[1024]).  The source strings are copies of the
//  Handler's MV.Message (up to 2047 chars, MessageDef.h:327), so golden overruns silently on a long reply; in-process
//  that would overwrite neighbouring V906 globals.  RsStrcpy is the bounded copy (truncate + NUL).
template <std::size_t N>
static void RsStrcpy(char (&dst)[N], const char* src)
{
    std::size_t i=0;
    for(; i+1<N && src[i]!='\0'; i++)
        dst[i]=src[i];
    dst[i]='\0';
}

//AI(W906-GB-P1) 20260926: golden `cX[i]` read right after that strcpy.  Byte-for-byte what golden's buffer holds:
//  for i < strlen(src) it is src[i] (also past N, through the overrun); at i==strlen the NUL; past that golden reads
//  the buffer's stale bytes and, past N, whatever memory follows -- sent here as the in-bounds byte, else 0.
template <std::size_t N>
static char RsReplyByte(const char (&buf)[N], const AnsiString& src, int i)
{
    if(i>=0 && i<src.Length())  return src.c_str()[i];
    if(i>=0 && i<(int)N)        return buf[i];
    return '\0';
}

// ===========================================================================
//  TfRS232Main  (golden RS232.cpp)
// ===========================================================================
//---------------------------------------------------------------------------
TfRS232Main::TfRS232Main()
    : CommTester(0), MemoLog(0), cbBaudRate(0), cbByteSize(0), cbStopBit(0), cbParity(0), cbDevice(0),
      edReadIntervalTimeout(0), bCommConnect(false), iHasCE(0)                 // VCL zero-fills a new TForm
{
    //AI(W906-GB-P1) 20260926: RS232.dfm:16-46 -- CommTester is the form's only .dfm component (owned here).  Only
    //  the properties vclcompat TComm carries are applied; Outx_CtsFlow=False, Outx_DsrFlow=False,
    //  DtrControl=DtrEnable, DsrSensitivity=False, TxContinueOnXoff=True, ReplaceWhenParityError=False,
    //  IgnoreNullChar=False, RtsControl=RtsEnable, XonLimit=500, XoffLimit=500, XonChar=#17, XoffChar=#19,
    //  ReplacedChar=#0 and the Read/WriteTotalTimeout* (all 0) have no vclcompat member (Comm.h:37-41).
    //  LoadSetupData() (golden Main.cpp:615) overwrites CommName/BaudRate/ByteSize/StopBits/Parity/
    //  ReadIntervalTimeout from D:\RS232Standard\System\Setup.ini before OpenTesterComm() (Main.cpp:616).
    CommTester=new TComm(0);
    CommTester->CommName            ="COM13";
    CommTester->BaudRate            =9600;
    CommTester->ParityCheck         =false;
    CommTester->Outx_XonXoffFlow    =true;
    CommTester->Inx_XonXoffFlow     =true;
    CommTester->ByteSize            =Spcomm::_7;
    CommTester->Parity              =Spcomm::Even;
    CommTester->StopBits            =Spcomm::_1;
    CommTester->ReadIntervalTimeout =1;
    //AI(W906-GB-P1) 20260926: .dfm `OnReceiveData = CommTesterReceiveData`.  vclcompat fires it on the COM reader
    //  thread, so only queue the bytes; TSerialPoll::DrainRx() calls CommTesterReceiveData on the TesterComm thread.
    CommTester->OnReceiveData=[](TObject*, void* b, Word n){ if (SerialPoll) SerialPoll->QueueRx(TSerialPoll::kRxAuxTester, b, n); };

    if(SerialPoll                        &&
       SerialPoll->MemoLog               &&
       SerialPoll->cbBaudRate            &&
       SerialPoll->cbByteSize            &&
       SerialPoll->cbStopBit             &&
       SerialPoll->cbParity              &&
       SerialPoll->cbDevice              &&
       SerialPoll->edReadIntervalTimeout)
    {
        MemoLog     =SerialPoll->MemoLog;
        cbBaudRate  =SerialPoll->cbBaudRate;
        cbByteSize  =SerialPoll->cbByteSize;
        cbStopBit   =SerialPoll->cbStopBit ;
        cbParity    =SerialPoll->cbParity  ;
        cbDevice    =SerialPoll->cbDevice  ;
        edReadIntervalTimeout=SerialPoll->edReadIntervalTimeout;
    }
    else
    {
        //AI(W906-GB-P1) 20260926: golden dereferences SerialPoll unconditionally (it is always created first,
        //  H9046_32GPIB.cpp:19-22).  In-process a NULL there would take the Handler down, so if the owner builds
        //  this form before SerialPoll's widgets exist it gets private stand-ins (not shown on SerialPoll's RS232
        //  tab) instead of a crash.  The destructor frees only these.
        MemoLog     =new TMemo;
        cbBaudRate  =new TComboBox;
        cbByteSize  =new TComboBox;
        cbStopBit   =new TComboBox;
        cbParity    =new TComboBox;
        cbDevice    =new TComboBox;
        edReadIntervalTimeout=new TEdit;
        RsFallbackOwners().insert(this);
    }
}
//------------------------------------------------------------------------------
TfRS232Main::~TfRS232Main()
{
    //AI(W906-GB-P1) 20260926: VCL frees the owned CommTester with the form (SPComm's destructor stops its threads);
    //  vclcompat ~TComm() calls StopComm(), which joins the reader thread, so no OnReceiveData fires after this.
    delete CommTester;
    CommTester=0;

    if(RsFallbackOwners().erase(this)>0)
    {
        delete MemoLog;
        delete cbBaudRate;
        delete cbByteSize;
        delete cbStopBit;
        delete cbParity;
        delete cbDevice;
        delete edReadIntervalTimeout;
    }
    // otherwise they are SerialPoll's widgets (golden ctor aliases them) and SerialPoll frees them
    MemoLog=0;
    cbBaudRate=0;
    cbByteSize=0;
    cbStopBit=0;
    cbParity=0;
    cbDevice=0;
    edReadIntervalTimeout=0;
}
//------------------------------------------------------------------------------
bool TfRS232Main::LoadSetupData()
{
    TIniFile* IniFile=new TIniFile("D:\\RS232Standard\\System\\Setup.ini");

    CommTester->CommName = IniFile->ReadString( "COMPort", "CommName", "COM3");
    CommTester->BaudRate = IniFile->ReadInteger("COMPort", "BaudRate", 9600);
    CommTester->ByteSize = (TByteSize)IniFile->ReadInteger("COMPort" , "ByteSize", _7);
    CommTester->StopBits = (TStopBits)IniFile->ReadInteger("COMPort" , "StopBits", _1);
    CommTester->Parity   = (TParity  )IniFile->ReadInteger("COMPort" , "Parity"  , Even);
    CommTester->ReadIntervalTimeout =IniFile->ReadInteger("Detail Settng", "ReadIntervalTimeout", 70);    //wei 20150212  add
    InitDataToMainForm();                                                       //Steven 20240615 : 修正RS232畫面顯示
    delete IniFile;
    return true;
}
//------------------------------------------------------------------------------
bool TfRS232Main::SaveSetupData()                                               //JerryYang 20160630 RS232加入GPIB
{
    TIniFile* IniFile=new TIniFile("D:\\RS232Standard\\System\\Setup.ini");

    IniFile->WriteString( "COMPort", "CommName", CommTester->CommName);
    IniFile->WriteInteger("COMPort", "BaudRate", CommTester->BaudRate);
    IniFile->WriteInteger("COMPort", "ByteSize", CommTester->ByteSize);
    IniFile->WriteInteger("COMPort", "StopBits", CommTester->StopBits);
    IniFile->WriteInteger("COMPort", "Parity"  , CommTester->Parity  );
    IniFile->WriteString("Detail Settng", "ReadIntervalTimeout", edReadIntervalTimeout->Text);   //wei 20150212  add
    delete IniFile;
    return true;
}
//------------------------------------------------------------------------------
void TfRS232Main::SetFormToData()
{
    CommTester->CommName=cbDevice->Text;
    CommTester->BaudRate=atoi(cbBaudRate->Text.c_str());

    if(     cbByteSize->ItemIndex==0)   CommTester->ByteSize    =Spcomm::_5;
    else if(cbByteSize->ItemIndex==1)   CommTester->ByteSize    =Spcomm::_6;
    else if(cbByteSize->ItemIndex==2)   CommTester->ByteSize    =Spcomm::_7;
    else if(cbByteSize->ItemIndex==3)   CommTester->ByteSize    =Spcomm::_8;

    if(     cbStopBit->ItemIndex==0)    CommTester->StopBits    =Spcomm::_1;
    else if(cbStopBit->ItemIndex==1)    CommTester->StopBits    =Spcomm::_1_5;
    else if(cbStopBit->ItemIndex==2)    CommTester->StopBits    =Spcomm::_2;

    if(     cbParity->ItemIndex==0)     CommTester->Parity      =Spcomm::None;
    else if(cbParity->ItemIndex==1)     CommTester->Parity      =Spcomm::Odd;
    else if(cbParity->ItemIndex==2)     CommTester->Parity      =Spcomm::Even;
    else if(cbParity->ItemIndex==3)     CommTester->Parity      =Spcomm::Mark;
    else if(cbParity->ItemIndex==4)     CommTester->Parity      =Spcomm::Space;

    SaveSetupData();
}
//------------------------------------------------------------------------------
void TfRS232Main::InitDataToMainForm()
{
    cbDevice->Text  =CommTester->CommName;
    cbBaudRate->Text=IntToStr(CommTester->BaudRate);

    if(     CommTester->ByteSize==Spcomm::_5)   cbByteSize->ItemIndex=0;
    else if(CommTester->ByteSize==Spcomm::_6)   cbByteSize->ItemIndex=1;
    else if(CommTester->ByteSize==Spcomm::_7)   cbByteSize->ItemIndex=2;
    else if(CommTester->ByteSize==Spcomm::_8)   cbByteSize->ItemIndex=3;

    if(     CommTester->StopBits==Spcomm::_1)   cbStopBit->ItemIndex=0;
    else if(CommTester->StopBits==Spcomm::_1_5) cbStopBit->ItemIndex=1;
    else if(CommTester->StopBits==Spcomm::_2)   cbStopBit->ItemIndex=2;

    if(     CommTester->Parity==Spcomm::None)   cbParity->ItemIndex=0;
    else if(CommTester->Parity==Spcomm::Odd)    cbParity->ItemIndex=1;
    else if(CommTester->Parity==Spcomm::Even)   cbParity->ItemIndex=2;
    else if(CommTester->Parity==Spcomm::Mark)   cbParity->ItemIndex=3;
    else if(CommTester->Parity==Spcomm::Space)  cbParity->ItemIndex=4;

    edReadIntervalTimeout->Text=CommTester->ReadIntervalTimeout;                //wei 20150212  add
}
//------------------------------------------------------------------------------
bool TfRS232Main::OpenTesterComm()                                              //JerryYang 20160630 RS232加入GPIB
{
    bool bRS232=false;                                                          //Jou 20240828 : 改成可以開關RS232功能
    if(CustomerCode==CC_HONPREC_QC          ||
       CustomerCode==CC_MAXIM_THAILAND      ||
       CustomerCode==CC_MAXIM               ||
       CustomerCode==CC_MAXIM_THAILAND      ||
       CustomerCode==CC_Microchip_Thai      ||
       CustomerCode==CC_Microchip_Phil      ||
       CustomerCode==CC_Microchip_China     ||
       CustomerCode==CC_SCC)
    {
        bRS232=CheckAndReadIniData("D:\\GPIB9045\\system\\general.ini", "OpenTesterComm", "RS232", true);
    }
    else
    {
        bRS232=CheckAndReadIniData("D:\\GPIB9045\\system\\general.ini", "OpenTesterComm", "RS232", false);
    }

    if(bRS232==false)
    {
        SerialPoll->tsRS232->TabVisible=false;                                  //Steven 20240615 : 修正RS232畫面顯示
        return false;
    }

    if(bCommConnect==true)
    {
        CloseTesterComm();
    }

    try
    {
        HANDLE handle=INVALID_HANDLE_VALUE;
        AnsiString CN="\\\\.\\"+CommTester->CommName;
        //AI(W906-GB-P1) 20260926: golden `CreateFile` (BCB6 ANSI build); spelled CreateFileA so a UNICODE build
        //  cannot turn it into CreateFileW.  Same open-and-close probe; its result is ignored, as in golden.
        handle=CreateFileA(CN.c_str(),
                          GENERIC_READ|GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                          0);
        CloseHandle(handle);

        //AI(W906-GB-P1) 20260926: golden SPComm StartComm raises ECommsError("Error opening serial port") when the
        //  port cannot be opened (D:\HT9045\elec\Component\Spcomm.pas:379-388), which lands in the catch(...) below
        //  ("Connect : Fail", bCommConnect=false).  vclcompat TComm::StartComm never throws: it falls back to SIM
        //  (vclcompat/Comm.cpp:340-343).  That silent fallback is turned back into golden's exception here; a SIM
        //  the owner requested explicitly (SetSimMode(true) before this call) is left alone.
        const bool bSimRequested=CommTester->IsSimMode();
        CommTester->StartComm();
        if(CommTester->IsSimMode() && !bSimRequested)
        {
            CommTester->StopComm();
            throw std::runtime_error("Error opening serial port");
        }
        bCommConnect=true;

        ShowCommData("RS232 ==> Connect : OK", std::vector<Byte>(), true);
    }
    catch(...)
    {
        ShowCommData("RS232 ==> Connect : Fail", std::vector<Byte>(), true);
        bCommConnect = false;
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
bool TfRS232Main::CloseTesterComm()                                             //JerryYang 20160630 RS232加入GPIB
{
    try
    {
        CommTester->StopComm();
        bCommConnect=false;

        ShowCommData("RS232 ==> Disconnect : OK", std::vector<Byte>(), true);
    }
    catch(...)
    {
        ShowCommData("RS232 ==> Disconnect : Fail", std::vector<Byte>(), true);
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
void TfRS232Main::ShowCommData(AnsiString sType, std::vector<Byte> Data, bool bAddToGPIBLog)             //JerryYang 20160630 RS232加入GPIB
{
    int iSize=Data.size();

    AnsiString Str;
    AnsiString sDataHex="";
    AnsiString sDataAscii="";
    for(int i=0; i<iSize; i++)
    {
        sDataHex+=IntToHex((int)Data[i], 2)+" ";
        sDataAscii+=MyDeCodeASCII((int)Data[i]);
    }

    if(iSize==0)
    {
        SerialPoll->WriteLog(sType);
        MemoLog->Lines->Add(sType);                                             //Steven 20240615 : RS232 LOG與GPIB LOG整合
    }
    else
    {
        Str=sType+AnsiString(", CMD: ")+sDataAscii+AnsiString(", HEX: ")+sDataHex;
        MemoLog->Lines->Add(Str);
        if(bAddToGPIBLog)                                                       //Steven 20240615 : RS232 LOG與GPIB LOG整合
            SerialPoll->WriteLog(Str);
    }

    if(MemoLog->Lines->Count>10240)
    {
        SaveResult();
    }
}
//------------------------------------------------------------------------------
void TfRS232Main::SendCommandToTester(std::vector<Byte>& Data, bool bAddToGPIBLog)   //JerryYang 20160630 RS232加入GPIB
{
    if(bCommConnect==false)
    {
        ShowCommData("RS232 ==> ", Data, true);
        ShowCommData("RS232 ==> Connect Error : Send Fail", std::vector<Byte>(), true);
        return;
    }

    int iSize=Data.size();
    if(iSize<=0)
        return;

    char Buff[10240];
    ZeroMemory(Buff, sizeof(Buff));
    //AI(W906-GB-P1) 20260926: golden writes Buff[0..iSize] unchecked; clamp so an oversized vector cannot run past
    //  this stack buffer (golden's frames are STX + <=2047 reply bytes + ETX, far below the limit).
    if(iSize>(int)sizeof(Buff)-1)
        iSize=(int)sizeof(Buff)-1;
    for(int i=0; i<iSize; i++)
    {
        Buff[i]=Data[i];
    }

    Buff[iSize]='\0';

    CommTester->WriteCommData(Buff, strlen(Buff));

    ShowCommData("RS232 ==> ", Data, bAddToGPIBLog);

    Data.clear();
}
//------------------------------------------------------------------------------
void TfRS232Main::CommTesterReceiveData(TObject *Sender,
      void* Buffer, WORD BufferLength)
{

    if(BufferLength==0)
        return;

    Byte* ptr = (Byte*)Buffer;
    ReceiveData.insert(ReceiveData.end(), ptr, ptr + BufferLength);
    if(ReceiveData.size()>102400)
    {
        ReceiveData.clear();
        return;
    }

    ShowCommData("Tester ==> RS232 : ", ReceiveData);

    AnsiString sCommand = GetAnalysisString(ReceiveData);                       // Jimmychiu 20240125 : Add AnalysisString function
    int iMaxDeal = 10;
    int iNowDeal = 0;
    while(sCommand.Pos("_None_")==0 && iNowDeal<iMaxDeal)
    {
        DoRevCommand(sCommand);
        sCommand = GetAnalysisString(ReceiveData);
        iNowDeal++;
    }
    ReceiveData.clear();


/*
    for(int i=0; i<BufferLength; i++)
    {
        ReceiveData.push_back(*((Byte*)Buffer+i));
    }

    ShowCommData("Tester ==> RS232 : ", ReceiveData);
    AnsiString sCommand=GetAnalysisString(ReceiveData);                         //Jimmychiu 20240125 : Add AnalysisString function
    int iMaxDeal=10,iNowDeal=0;
    while((sCommand.Pos("_None_")>0)==false || iNowDeal>iMaxDeal)               //clear when get none or over deal
    {
        DoRevCommand(sCommand);
        sCommand=GetAnalysisString(ReceiveData);
        iNowDeal++;
        if(iNowDeal>iMaxDeal)                                                   //JimmyChiu 20250401 : Add protection
        {
            break;
        }
    }
    */
    (void)Sender;   //AI(W906-GB-P1) 20260926: unused in golden too
}
//------------------------------------------------------------------------------
AnsiString TfRS232Main::GetAnalysisString(std::vector<Byte> &ReceiveData)
{
    AnsiString sBack="_None_";
    if(ReceiveData.empty())
    {
    }
    else if(ReceiveData[0]==static_cast<Byte>(_STX_))
    {
        int stxPosition=0;
        int etxPosition=0;
        int iSize=ReceiveData.size();
        for(int i=0; i<iSize; i++)
        {
            if(ReceiveData[i]==static_cast<Byte>(_ETX_))
            {
                etxPosition=i;
                break;
            }
        }
        if(etxPosition>0)
        {
            sBack="";
            for(int i=stxPosition+1; i<etxPosition; i++)
            {
                sBack+=MyDeCodeASCII((int)ReceiveData[i]);
            }
            while(ReceiveData.size()>0)//remove data
            {
                iSize=ReceiveData.size();
                if(ReceiveData[0]==static_cast<Byte>(_ETX_))
                {
                    Del1stVec(ReceiveData);
                    break;
                }
                else
                {
                    Del1stVec(ReceiveData);
                }
            }
        }
    }
    else if(ReceiveData[0]==static_cast<Byte>(_ACK_))
    {
        sBack="_ACK_";
        Del1stVec(ReceiveData);
    }
    else if(ReceiveData[0]==static_cast<Byte>(_ENQ_))
    {
        sBack="_ENQ_";
        Del1stVec(ReceiveData);
    }
    else if(ReceiveData[0]!=_ENQ_ &&
            ReceiveData[0]!=_ACK_ &&
            ReceiveData[0]!=_STX_)                                              //Steven 20210805 : 清空雜訊
    {
        sBack="_CLEAR_";
    }
    return sBack;
}
//------------------------------------------------------------------------------
void TfRS232Main::Del1stVec(std::vector<Byte> &ReceiveData)
{
    if(!ReceiveData.empty())
    {
        ReceiveData.erase(ReceiveData.begin());
    }
}
//------------------------------------------------------------------------------
void TfRS232Main::DoRevCommand(AnsiString sCommand)
{
    static bool bSendStart = false;
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        bSendStart=false;
    }
//    static bool bRecevie2Times=false;
//    char cSit[TOTAL_SITE] = {0};
//    char cTemp[5] = {0};
//    bool bFirst = true;
    int iStrLength;
//    int SiteNo = 3;
    int iSite, iBinCode;

    if(sCommand.Pos("_ENQ_")>0)                                                 // receive "05" ,then send "06"
    {
        bSendStart=false;                                                       //Jou 2015-02-02
        SendData.clear();
        SendData.push_back(_ACK_);
        SendCommandToTester(SendData, false);
    }
    else if(sCommand.Pos("_ACK_")>0 && bSendStart==true)                        //Test send "06"
    {
        bSendStart=false;
        SendCommandToTester(SendStartData);

//        if(iHasCE==iWaitReplyCE)                                                //Steven 20231205 : 判斷RS232流程是否異常
//        {
//            iHasCE=iReplyCE;
//        }

//        for(int i=0; i<MAX_SITE_COUNT; i++)                                     //Steven 20231101 : 正確送出資料後, 才清除開site的flag
//        {
//            iStart[i]=false;                                                    //wei 20150603 送出Site就清除
//        }
    }
    else if(sCommand.Pos("ST")>0)                                               //Steven 20141014 : 神盾測試模式
    {
//        SendMSG_CMD(MSG_CMD_SwitchArm);
    }
    else if(sCommand.Pos("CF")>0)                                               // Tester send "02CF03 " 0x43,0x46
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_GetNumOfSites);
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sNumOfSites.Length();
        RsStrcpy(cNumOfSites, sNumOfSites.c_str());                             // golden strcpy(cNumOfSites, sNumOfSites.c_str()); see RsStrcpy
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cNumOfSites, sNumOfSites, i));  // golden cNumOfSites[i]; see RsReplyByte
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData, false);
    }
    else if(sCommand.Pos("CE")>0)                                               // Tester send "02CE03 " 0x43,0x45
    {

    }
    else if(sCommand.Pos("BARCODE?")>0)
    {

    }
    else if(sCommand.Pos("GET2DID?")>0)
    {

    }
    else if(sCommand.Pos("BA")>0)                                               //wei 20150604  只收到B  // Tester send result ex."02 BA 1,1,1;2,1,1 03" 0x42,0x41
    {
    }
    else if(sCommand.Pos("CZ status?")>0 || sCommand.Pos("CK")>0)               //JerryYang 20151109 for力成,測試機Send "CZ status?"
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_MachineState);                          //向handler要機台狀態

        SendStartData.clear();
        SendStartData.push_back(_STX_);

        RsStrcpy(cMachineStateDec, sMachineStateDecade.c_str());                // golden strcpy(cMachineStateDec, sMachineStateDecade.c_str());
        for(int i=0; i<iMacStateStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cMachineStateDec, sMachineStateDecade, i));   // golden cMachineStateDec[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ which?")>0)                                        //Steven 20200218 : for力成,測試機Send "CZ which?"
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        TIniFile *INIFileGeneral;
        INIFileGeneral=new TIniFile("D:\\HT9045\\system\\Gerneral.ini");
        AnsiString sID=INIFileGeneral->ReadString("Version", "Machine ID", "HT-9046");
        delete INIFileGeneral;
        const int iStrLength=sID.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sID.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ testerbin?")>0)                                    //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ testerbin?"
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_TesterBin);                             //向handler要各Bin數量

        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iTestBinLength=sTestBinCount.Length();
        char cTestBinCount[1024];
        ZeroMemory(cTestBinCount, sizeof(cTestBinCount));
        RsStrcpy(cTestBinCount, sTestBinCount.c_str());                         // golden strcpy(cTestBinCount, sTestBinCount.c_str());
        for(int i=0; i<iTestBinLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cTestBinCount, sTestBinCount, i));   // golden cTestBinCount[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ all masstemp?")>0 ||                               //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ all masstemp?"
            sCommand.Pos("CB")>0)
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_AllMassTemp);                           //向handler要溫度

        SendStartData.clear();
        SendStartData.push_back(_STX_);
        RsStrcpy(cAllMassTemp, sAllMassTemp.c_str());                           // golden strcpy(cAllMassTemp, sAllMassTemp.c_str());
        const int iStrLength=sAllMassTemp.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cAllMassTemp, sAllMassTemp, i));    // golden cAllMassTemp[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ sitemap?")>0)                                      //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ sitemap?"
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_SiteMap);                               //向handler要SiteMap
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        RsStrcpy(cSiteMap, sSiteMap.c_str());                                   // golden strcpy(cSiteMap, sSiteMap.c_str());
        const int iStrLength=sSiteMap.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cSiteMap, sSiteMap, i));        // golden cSiteMap[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ jam?")>0 ||                                        //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ jam?"
            sCommand.Pos("CZ jamnumber?")>0)                                    //Isaac 20170825(jou) for Amkor_philippine,測試機Send "CZ jamnumber?"
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_JamCode);                               //向handler要機台Jam Code
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iJamCodeLength=sJamCode.Length();
        RsStrcpy(cJamCode, sJamCode.c_str());                                   // golden strcpy(cJamCode, sJamCode.c_str());
        for(int i=0; i<iJamCodeLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cJamCode, sJamCode, i));        // golden cJamCode[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ soaktime?")>0)                                     //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ soaktime?"
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_SoakTime);                              //向handler要Soak time
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sSoakTime.Length();
        RsStrcpy(cSoakTime, sSoakTime.c_str());                                 // golden strcpy(cSoakTime, sSoakTime.c_str());
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cSoakTime, sSoakTime, i));      // golden cSoakTime[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ id?")>0)                                           //Steven 20200218 : for力成,測試機Send "CZ id?"
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        TIniFile *INIFileGeneral;
        INIFileGeneral=new TIniFile("D:\\HT9045\\system\\Gerneral.ini");
        AnsiString sID=INIFileGeneral->ReadString("Version", "Model", "HT-9046");
        delete INIFileGeneral;
        const int iStrLength=sID.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sID.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        //----------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ doublecontact?")>0)                                //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_DoubleContactCount);                    //向handler要doublecontactcount
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        RsStrcpy(cDoubleContactCount, sDoubleContactCount.c_str());             // golden strcpy(cDoubleContactCount, sDoubleContactCount.c_str());
        const int iStrLength=sDoubleContactCount.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sDoubleContactCount.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CD")>0)
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_HanderIDRS232);                         //向handler要handler ID
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sHandlerID.Length();
        RsStrcpy(cHandlerID, sHandlerID.c_str());                               // golden strcpy(cHandlerID, sHandlerID.c_str());
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cHandlerID, sHandlerID, i));    // golden cHandlerID[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CN")>0)
    {
        SerialPoll->SendMSG_CMD(MSG_CMD_GetSiteOnOff);                          //向handler要handler ID
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sSiteOnOff.Length();
        RsStrcpy(cSiteOnOff, sSiteOnOff.c_str());                               // golden strcpy(cSiteOnOff, sSiteOnOff.c_str());
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(RsReplyByte(cSiteOnOff, sSiteOnOff, i));    // golden cSiteOnOff[i]
        }
        SendStartData.push_back(_ETX_);
        bSendStart=true;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    //AI(W906-GB-P1) 20260926: golden's unused, uninitialised locals iStrLength/iSite/iBinCode (RS232.cpp:372-374) are
    //  kept as written; -Wunused-variable warns, which is expected (a (void) cast would trip MSVC C4700 under /sdl).
}
//------------------------------------------------------------------------------
void TfRS232Main::SaveResult()
{
    try
    {
        Save_Log();                                                             //kevin 20130516 add record
        MemoLog->Clear();
//        Save_Log_TTL();
    }
    catch(...)
    {
    }
}
//------------------------------------------------------------------------------
void TfRS232Main::Save_Log()
{
    AnsiString FileName;
    FileName.sprintf("D:\\RS232Log\\Log\\%04d_%02d", SystemYear, SystemMonth);

    if(!DirectoryExists(FileName))
        ForceDirectories(FileName);

    FileName.sprintf("D:\\RS232Log\\Log\\%04d_%02d\\%04d_%02d_%02d_%02d %02d %02d.txt",
                     SystemYear, SystemMonth,
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    MemoLog->Lines->SaveToFile(FileName);
}

// ===========================================================================
//  TfDummyART  (golden DummyArt.cpp)
// ===========================================================================
//------------------------------------------------------------------------------
TfDummyART::TfDummyART()
    : Label2(0), Label3(0), spbAutoRetest(0), spbStopART(0), spbPauseART(0), spbContinue(0), edLotID(0),
      edDummyCount(0), rgTestType(0), DummyARTTimer1(0), cbStepByStep(0)
{
    bTimerRun=false;   //AI(W906-GB-P1) 20260926: golden DummyArt.cpp:19 initialiser; one TfDummyART per bridge life
    //AI(W906-GB-P1) 20260926: DummyArt.dfm widgets.  Visible/Enabled are the VCL defaults (true) that the .dfm
    //  leaves out; the vclcompat stand-ins default them to false (Controls.h DEFAULT-VALUE RULE), so they are set
    //  here -- otherwise the web page would show "Initial Lot" disabled before any click, unlike golden.
    //  Geometry, fonts and glyphs are layout, not state, and are not carried.  The speed buttons' AllowAllUp=True
    //  has no vclcompat member (GroupIndex/Down are carried); rgTestType's Caption 'Tester Type' has no
    //  vclcompat TRadioGroup member.
    Label2=new TLabel;
    Label2->Caption="Lot ID";
    Label2->Visible=true;       Label2->Enabled=true;

    Label3=new TLabel;
    Label3->Caption="Lot Count";
    Label3->Visible=true;       Label3->Enabled=true;

    spbAutoRetest=new TSpeedButton;
    spbAutoRetest->Caption="Initial Lot";
    spbAutoRetest->GroupIndex=1;
    spbAutoRetest->Down=false;
    spbAutoRetest->Visible=true;    spbAutoRetest->Enabled=true;

    spbStopART=new TSpeedButton;
    spbStopART->Caption="Stop ART";
    spbStopART->GroupIndex=1;
    spbStopART->Down=false;
    spbStopART->Visible=true;       spbStopART->Enabled=true;

    spbPauseART=new TSpeedButton;
    spbPauseART->Caption="Pause";
    spbPauseART->GroupIndex=1;
    spbPauseART->Down=false;
    spbPauseART->Visible=true;      spbPauseART->Enabled=true;

    spbContinue=new TSpeedButton;
    spbContinue->Caption="Continue";
    spbContinue->GroupIndex=1;
    spbContinue->Down=false;
    spbContinue->Visible=true;      spbContinue->Enabled=true;

    edLotID=new TEdit;
    edLotID->Text="DummyLot";
    edLotID->Visible=true;          edLotID->Enabled=true;

    edDummyCount=new TEdit;
    edDummyCount->Text="50";
    edDummyCount->Visible=true;     edDummyCount->Enabled=true;

    rgTestType=new TRadioGroup;
    rgTestType->Items->Add("0: Flex");
    rgTestType->Items->Add("1: 93K");
    rgTestType->ItemIndex=1;
    rgTestType->Visible=true;       rgTestType->Enabled=true;

    cbStepByStep=new TCheckBox;
    cbStepByStep->Caption="Step by Step";
    cbStepByStep->Checked=false;
    cbStepByStep->Visible=true;     cbStepByStep->Enabled=true;

    DummyARTTimer1=new TTimer;
    DummyARTTimer1->Enabled=false;                                              // .dfm Enabled = False
    DummyARTTimer1->Interval=1000;                                              // not in .dfm: VCL TTimer default

//    LastSet.iDummyARTTask=1;                                                  //Steven 20180824 : Semi ART
}
//------------------------------------------------------------------------------
TfDummyART::~TfDummyART()
{
    delete Label2;          Label2=0;
    delete Label3;          Label3=0;
    delete spbAutoRetest;   spbAutoRetest=0;
    delete spbStopART;      spbStopART=0;
    delete spbPauseART;     spbPauseART=0;
    delete spbContinue;     spbContinue=0;
    delete edLotID;         edLotID=0;
    delete edDummyCount;    edDummyCount=0;
    delete rgTestType;      rgTestType=0;
    delete DummyARTTimer1;  DummyARTTimer1=0;
    delete cbStepByStep;    cbStepByStep=0;
}
//------------------------------------------------------------------------------
void TfDummyART::FormShow(TObject *Sender)
{
    spbPauseART->Enabled=false;
    spbContinue->Enabled=false;

    if(DummyARTTimer1->Enabled==true)
    {
        spbPauseART->Enabled=true;
    }
    else if(LastSet.iDummyARTTask!=1)                                           //Steven 20180824 : Semi ART
    {
        spbContinue->Enabled=true;
    }
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::spbAutoRetestClick(TObject *Sender)
{
    LastSet.iDummyARTTask=1;                                                    //Steven 20180824 : Semi ART
    DummyARTTimer1->Enabled=true;
    spbAutoRetest->Down=false;
    spbAutoRetest->Enabled=false;
    spbPauseART->Enabled=true;
    spbContinue->Enabled=false;
    SerialPoll->bDummyArt=true;
    bTimerRun=false;
    LastSet.bDummyART=true;                                                     //Steven 20180824 : Semi ART

    SerialPoll->WriteLastDataFile();                                            //Steven 20180824 : Semi ART
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::spbStopARTClick(TObject *Sender)
{
    LastSet.iDummyARTTask=1;                                                    //Steven 20180824 : Semi ART
    LastSet.bDummyART=false;
    DummyARTTimer1->Enabled=false;
    spbStopART->Down=false;
    spbAutoRetest->Enabled=true;
    spbPauseART->Enabled=false;
    spbContinue->Enabled=false;
    SerialPoll->bDummyArt=false;
    bTimerRun=false;
    SerialPoll->WriteLastDataFile();                                            //Steven 20180824 : Semi ART
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::spbPauseARTClick(TObject *Sender)
{
    spbPauseART->Down=false;
    spbPauseART->Enabled=false;
    spbContinue->Enabled=true;
    DummyARTTimer1->Enabled=false;
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::spbContinueClick(TObject *Sender)
{
    spbContinue->Down=false;
    spbPauseART->Enabled=true;
    spbContinue->Enabled=false;
    DummyARTTimer1->Enabled=true;
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::DummyARTTimer1Timer(TObject *Sender)
{
    if(bTimerRun)
        return;
    bTimerRun=true;
    if(rgTestType->ItemIndex==1)
    {
        HP93KART();
    }
    else
    {
        FLEXART();
    }

    bTimerRun=false;
    (void)Sender;
}
//------------------------------------------------------------------------------
void TfDummyART::HP93KART()
{
    int &Task=LastSet.iDummyARTTask;                                            //Steven 20180824 : Semi ART
    AnsiString Str1, Str2;

    switch(Task)
    {
        case 1:
            SerialPoll->WriteLog("Dummy FT <== FR?");
            SerialPoll->WriteLog("Dummy FT ==> FR0");
            SerialPoll->WriteLog("Dummy FT <== DL");
            SerialPoll->WriteLog("Dummy FT <== SRQMASK");
            SerialPoll->ProcessStatusString("SRQMASK", "", "Dummy ");
            Task=1000;
            break;
        case 1000:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy FT <== LOTCLEAR?");
            SerialPoll->ProcessStatusString("LOTCLEAR?", "", "Dummy ");
            Task=1100;
            break;
        case 1100:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                if(SerialPoll->DummyArtMSG.AnsiPos("LOTCLEARED")!=0)
                {
                    Str1.sprintf("INPUTQTY%s %s", edDummyCount->Text, edLotID->Text);
                    SerialPoll->WriteLog("Dummy <== "+AnsiString(Str1));
                    SerialPoll->ProcessStatusString(Str1, "", "Dummy ");
                    Task=1200;
                }
                else
                {
                    Task=1000;
                }

                //AI(W906-GB-P1) 20260926: golden `spbPauseART->Click();` / `spbStopART->Click();` fire the OnClick
                //  handler (VCL TControl::Click does not check Enabled).  vclcompat Click() is a no-op; call the
                //  golden OnClick handler directly.  Same at every Click() site below.
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click();
            }
            break;
        case 1200:                                                              //0xC0
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy FT <== SRQKIND?");
                SerialPoll->ProcessStatusString("SRQKIND?", "", "Dummy ");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=1300;
            }
            break;
        case 1300:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy FT <== LOTORDER0");
                SerialPoll->ProcessStatusString("LOTORDER0", "", "Dummy ");
                SerialPoll->WriteLog("Dummy FT ==> START TEST");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=2000;
            }
            break;
        case 2000:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy FT END <== SRQKIND?");
                SerialPoll->ProcessStatusString("SRQKIND?", "", "Dummy ");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=2100;
            }
            break;
        case 2100:
            if(SerialPoll->bMessageFromHandler)
            {
                if(SerialPoll->DummyArtMSG=="8")
                {
                    Task=3000;
                }
                else if(SerialPoll->DummyArtMSG=="10")
                {
                    Task=5000;
                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
            }
            break;
        case 3000:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy RT Start<== LOTRETESTCLEAR?");
            SerialPoll->ProcessStatusString("LOTRETESTCLEAR?", "", "Dummy ");
            Task=3100;
            break;
        case 3100:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                if(SerialPoll->DummyArtMSG.AnsiPos("LOTRETESTCLEARED")!=0)
                {
                    SerialPoll->WriteLog("Dummy FT <== LOTORDER2");
                    SerialPoll->ProcessStatusString("LOTORDER2", "", "Dummy ");
                    Task=3200;
                }
                else
                {
//                    Task=1000;
                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
            }
            break;
        case 3200:                                                              //0xC0
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy RT <== SRQKIND?");
                SerialPoll->ProcessStatusString("SRQKIND?", "", "Dummy ");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=3300;
            }
            break;
        case 3300:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy RT <== LOTORDER0");
                SerialPoll->ProcessStatusString("LOTORDER0", "", "Dummy ");
                SerialPoll->WriteLog("Dummy RT ==> START TEST");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=4000;
            }
            break;
        case 4000:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy RT END <== SRQKIND?");
                SerialPoll->ProcessStatusString("SRQKIND?", "", "Dummy ");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=4100;
            }
            break;
        case 4100:
            if(SerialPoll->bMessageFromHandler)
            {
                if(SerialPoll->DummyArtMSG=="8")
                {
                    Task=3000;
                }
                else if(SerialPoll->DummyArtMSG=="10")
                {
                    Task=5000;
                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
            }
            break;
        case 5000:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                SerialPoll->WriteLog("Dummy Final Lot End <== LOTORDER2");
                SerialPoll->ProcessStatusString("LOTORDER2", "", "Dummy ");
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note above
                Task=5100;
            }
            break;
        case 5100:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy Final Lot End <== LOTRETESTCLEAR?");
            SerialPoll->ProcessStatusString("LOTRETESTCLEAR?", "", "Dummy ");
            spbStopARTClick(spbStopART);                                        // golden spbStopART->Click(); see note above
            break;
    }
}
//------------------------------------------------------------------------------
void TfDummyART::FLEXART()
{
    static int iStatusR=0;
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        iStatusR=0;
    }
    int &Task=LastSet.iDummyARTTask;                                            //Steven 20180824 : Semi ART
    AnsiString Str1, Str2;

    switch(Task)
    {
        case 1:
            SerialPoll->WriteLog("Dummy <== INITIAL?");
            SerialPoll->ProcessStatusString("INITIAL?", "", "Dummy ");
            iStatusR=0;
            Task=10;
            // fall through -- golden has no break here
        case 10:
            SerialPoll->bStsMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy <== LOTSTATUS?");
            SerialPoll->ProcessStatusString("LOTSTATUS?", "", "Dummy ");
            Task=100;
            //break;
            // fall through -- golden comments the break out
        case 100:
            if(SerialPoll->bStsMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                if(SerialPoll->DummyArtStsMSG.AnsiPos("LOTSTATUS_W")!=0)
                {
                    iStatusR=0;
                    Task=10;
                    DummyARTTimer1->Interval=5000;
                }
                else
                {
                    if(SerialPoll->DummyArtStsMSG.AnsiPos("NONE")!=0)
                    {
                        Task=200;
                    }
                    else if(SerialPoll->DummyArtStsMSG.AnsiPos("LOTSTATUS_L")!=0 ||
                            SerialPoll->DummyArtStsMSG.AnsiPos("LOTSTATUS_A")!=0)
                    {
                        iStatusR=0;
                        Task=2000;
                    }
                    else if(SerialPoll->DummyArtStsMSG.AnsiPos("LOTSTATUS_R")!=0)
                    {
                        if(iStatusR==2)
                        {
                            Task=10;
                        }
                        else
                        {
                            iStatusR=1;
                            Task=200;
                        }
                    }
                    else if(SerialPoll->DummyArtStsMSG.AnsiPos("LOTSTATUS_F")!=0)
                    {
                        iStatusR=1;
                        spbStopARTClick(spbStopART);                            // golden spbStopART->Click(); see note in HP93KART
                    }
                    else
                    {
                        Task=10;
                    }
                    DummyARTTimer1->Interval=1000;
                }
            }
            break;
        case 200:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy <== QTY?");
            SerialPoll->ProcessStatusString("QTY?", "", "Dummy ");
            Task=300;
            break;
        case 300:
            if(SerialPoll->bMessageFromHandler)
            {
                if(SerialPoll->DummyArtMSG.AnsiPos("SETTINGNG")!=0)
                {
                    Task=1000;
                }
                else
                {
                    Task=3000;
                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note in HP93KART
            }
            break;
        case 1000:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy FT <== LOTCLEAR?");
            SerialPoll->ProcessStatusString("LOTCLEAR?", "", "Dummy ");
            Task=1100;
            break;
        case 1100:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                if(SerialPoll->DummyArtMSG.AnsiPos("LOTCLEARED")!=0)
                {
                    Str1.sprintf("INPUTQTY%s %s", edLotID->Text, edDummyCount->Text);
                    SerialPoll->WriteLog("Dummy FT <== "+AnsiString(Str1));
                    SerialPoll->ProcessStatusString(Str1, "", "Dummy ");
                    Task=1200;
                }
//                else
//                {
//                    Task=1000;
//                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note in HP93KART
            }
            break;
        case 1200:
            if(SerialPoll->bMessageFromHandler)
            {
                if(SerialPoll->DummyArtMSG.AnsiPos("ECHOQTY")!=0)
                {
                    SerialPoll->bMessageFromHandler=false;
                    SerialPoll->WriteLog("Dummy FT <== ECHOQTYOK");
                    SerialPoll->ProcessStatusString("ECHOQTYOK", "", "Dummy ");
                    Task=10;
                }
                if(cbStepByStep->Checked)
                    spbPauseARTClick(spbPauseART);                              // golden spbPauseART->Click(); see note in HP93KART
            }
            break;
        case 2000:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy <== ALARM?");
            SerialPoll->ProcessStatusString("ALARM?", "", "Dummy ");
            Task=10;
            break;
        case 3000:
            SerialPoll->bMessageFromHandler=false;
            SerialPoll->WriteLog("Dummy RT <== LOTRETESTCLEAR?");
            SerialPoll->ProcessStatusString("LOTRETESTCLEAR?", "", "Dummy ");
            Task=3100;
            break;
        case 3100:
            if(SerialPoll->bMessageFromHandler)
            {
                SerialPoll->bMessageFromHandler=false;
                if(SerialPoll->DummyArtMSG.AnsiPos("LOTRETESTCLEARED")!=0)
                {
                    iStatusR=2;
                    Task=10;
                }
//                if(cbStepByStep->Checked)
                SerialPoll->WriteLog("Please click continue when handler ART action is done.");
                spbPauseARTClick(spbPauseART);                                  // golden spbPauseART->Click(); see note in HP93KART
            }
            break;

    }
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
