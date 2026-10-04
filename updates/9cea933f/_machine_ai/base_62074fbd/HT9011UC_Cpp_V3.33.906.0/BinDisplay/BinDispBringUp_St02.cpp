// =============================================================================
//  BinDisplay/BinDispBringUp_St02.cpp -- ST02-C14 Bin Display: the C++ bring-up (golden 906 base).
//  AI(W906-ST02-C14) 20261002 (St02-E helper).  Declarations: BinDisplay/BinDispBringUp_St02.h.
//
//  Golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven (cp950, decoded with Python; 0 U+FFFD on the spans used).
//  What this file is, in golden order:
//    TDataModule3 *DataModule3        BinDisplay\MyBinDisp.cpp:21
//    TDataModule3::TDataModule3       BinDisplay\MyBinDisp.cpp:32-35 + the .dfm stream MyBinDisp.dfm:1-68
//    TDataModule3::DataModuleDestroy  BinDisplay\MyBinDisp.cpp:3070-3073
//    TMyBinDispCtrl::Timer1Timer      BinDisplay\MyBinDisp.cpp:284-604  (GATE (1) of MyBinDisp.h lifted: the stub at
//                                     MyBinDisp.cpp:332-334 is retired by a same-line claim, this is the body)
//    W906_BinDispSystemModularBoot_St02  database.cpp:1543-1545 (the bin-display half of SystemModularInitial) +
//                                     HT9045.cpp:212 Application->CreateForm(__classid(TDataModule3), &DataModule3)
//    W906_BinDispFormShowAt_St02      main.cpp:10483-10485   HSys.BinDisCtrl->InitialOK=InitialOK  (TfMain::FormShow)
//    W906_BinDispFormClose_St02       main.cpp:11567-11569   the same copy                       (TfMain::FormClose)
//    W906_BinDispBinSelFormShow_St02  cBinSel.cpp:1719-1721  HSys.BinDisCtrl->ProcessStopStart(false) (TfBinSel::FormShow)
//  The tick: MainTimersSt02.cpp runs golden Timer1 (MyBinDisp.cpp:92-96, Interval 200 ms, owner NULL) as one more
//  deadline slot of the St02 dispatcher -- see that file's header (PumpTick ~500 ms, <= 100 ms inside the modal waits).
//
//  [W906] DEVIATIONS (each also marked in place):
//   (1) RECEIVE DELIVERY.  golden SPComm posts OnReceiveData to the main thread, and the .dfm ReadIntervalTimeout (100 ms;
//       50 ms that Timer1Timer case 1 sets for TFT) makes one reply arrive as one call.  vclcompat TComm calls OnReceiveData
//       on its reader thread with whatever ReadFile returned (vclcompat/Comm.cpp ReaderProc_), so golden's handlers would race
//       the tick thread and see split replies.  The rs232.cpp TCOM2Shim pattern (W906_Comm1QueueRx / W906_PumpComm1) is
//       copied: the reader thread only queues (RxSink), and every dispatcher pass hands each queue that has been quiet for
//       the port's ReadIntervalTimeout to golden CommBinReceiveData / CommBinReceiveData2 (W906_BinDispRxPumpAt_St02), before
//       Timer1 fires.  A queue that reaches 1023 bytes is handed over at once in <= 1023-byte pieces (golden's handlers drop
//       anything >= 1024, MyBinDisp.cpp:200 / :255).
//   (2) NO COM PORT IN SIM OR UNDER CTEST.  Both TComm get SetSimMode(true) in the SIM build (SOFT_SIMULTE) and whenever
//       ctest's redirect environment is present (W906_BinDispUnderCtest_St02: W906_GENERAL_INI_PATH in general_ini_scratch or
//       W906_HT9045LOG_ROOT in machine_log_scratch, tests/CMakeLists.txt _w906_env_all_tests).  Under ctest the SHIP-only
//       GetCOMPortStatus probe (an exclusive CreateFile, EJ1N/TextProcess.cpp:770) is replaced by the same answer from our
//       own ports ("free" = a COM name that neither of our TComm holds open), so not even a probe touches a device.
//   (3) StartComm.  golden SPComm StartComm throws when the port is already open or CreateFile fails (Spcomm.pas:371-388);
//       vclcompat returns silently / falls back to SIM.  BinDispGoldenStartComm puts the two exceptions back (the
//       TesterComm/Rs232/Rs232Comm.cpp W906_GoldenStartComm scheme), so golden's catch(...) -> "Error open com port" runs where
//       golden's did; a SIM the owner asked for (2) is left alone.
//   (4) The .dfm values vclcompat TComm cannot carry: XonLimit / XoffLimit 500, XonChar #17 / XoffChar #19, ReplacedChar,
//       DsrSensitivity, TxContinueOnXoff = False (vclcompat forces True), Read/WriteTotalTimeout* (vclcompat: read 50 ms when
//       an interval is set, write 2000 ms).  DtrControl = DtrEnable / RtsControl = RtsEnable / Outx_CtsFlow / Outx_DsrFlow =
//       False are what vclcompat forces anyway (vclcompat/Comm.cpp AI(W906-TORQUE-COMM)).  Type 3 keeps Inx_XonXoffFlow = True
//       with the driver's XON / XOFF characters and limits -- HUMAN REVIEW B (TFT, type 4, turns XON/XOFF off in case 1).
//   (5) CreateForm(TDataModule3) only for NUMBER_PANEL_TYPE 3 / 4 (golden HT9045.cpp:212 is unconditional; the module is used
//       only by Timer1Timer case 1, which returns for every other type): types 0 / 1 / 2 keep exactly what main does today.
//   (6) The BinDisplayLog folder follows the W906_HT9045LOG_ROOT ctest seam: slBinDispLog->Path = as9045LogPath +
//       "\\BinDisplayLog" -- the same value as the golden literal (MyBinDisp.cpp:70 / port :183) unless the seam is set
//       (the common.cpp :244 asShtLogPath pattern).
//   (7) NULL guards golden does not have: Timer1Timer case 1 without DataModule3; the InitialOK copies and the BinSel pause
//       with no instance (golden derefs HSys.BinDisCtrl unguarded at main.cpp:10485 / :11569 / cBinSel.cpp:1721, safe there
//       because the guarding NUMBER_PANEL_TYPE==3||4 implies an instance).
//   (8) The BinSel pause does not run while SystemStart: golden 0618 hides palSetup / palConfig then (DoMainPadProcess
//       main.cpp:3842-3846), so TfBinSel cannot be opened.  ST02-C14b (NB2 R168 low): this was SystemStart || SoftStart;
//       golden checks SystemStart only.  A web page closed mid-run without its FormClose is resumed by (10).
//   (9) PROTECTED ACCESS.  Three golden members this file must reach are protected and their headers are not St02's:
//       TMyBinDispCtrl::Timer1 / CommBinReceiveData / CommBinReceiveData2 (MyBinDisp.h:274 / :280 / :281) and
//       SYSTEM_MODULAR::InstallColorBinDisplay (database.h:232).  Instead of claims that change those headers, each is named
//       once through a pointer to member formed inside a never-instantiated derived struct (standard C++ [class.protected]:
//       a derived class may form &Derived::member).  A later wave that owns those headers may make them public and delete
//       the two Access structs below.
//   (10) ST02-C14b (NB2 R168 M1): the BinSel pause is resumed once the page is no longer showing even when its FormClose
//       was skipped (closed while running) -- W906_BinDispBinSelResumeTick_St02, the display half of golden FormClose only.
//  Golden behaviour kept as it is (it looks wrong; HUMAN REVIEW): case 1 retries every tick while neither port can be opened
//  and logs two "Stop Comm" lines per tick; the TFT Interval 30 (case 1) is undone at the top of the next tick (golden
//  oddity (p), MyBinDisp.cpp:313-320 vs :346); the 2 s / dDelaySec waits are TQPF_Timer wall-clock timers.
// =============================================================================
#include "BinDisplay/BinDispBringUp_St02.h"
#include "BinDisplay/MyBinDisp.h"

#include "canary_support.h"         // ShowMyMessage (golden mymessbox.h substitute, as MyBinDisp.cpp)
#include "Public/MyStringList.h"    // complete TMyStringList (slBinDispLog)
#include "cmydef.h"                 // NUMBER_PANEL_TYPE, MAGAZINE_BIN_DISP_TYPE, InitialOK, SystemStart, SoftStart
#include "cprod.h"
#include "database.h"               // HSys, SYSTEM_MODULAR
#include "common.h"                 // as9045LogPath
#include "EJ1N/TextProcess.h"       // GetCOMPortStatus (golden TextProcess.h)
#include "forms/fShowBinSelect.h"   // fShowBinSelect->PageControl1 / tsUnloadMap (W906_BinDispShownScope_St02)

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

// The 2-arg overload, declared locally as MyBinDisp.cpp:112 does (the cMyDB.h 3-arg one would make the call ambiguous).
extern void MyDBIProcess(AnsiString S1, AnsiString S2);

TDataModule3 *DataModule3 = NULL;                                               // golden MyBinDisp.cpp:21

namespace {

// ---- (9) protected access, see the banner -----------------------------------
struct BinDispAccess_St02 : TMyBinDispCtrl                                      // never instantiated
{
    typedef void (TMyBinDispCtrl::*RxFn)(TObject*, void*, Spcomm::Word);
    typedef ht9045_bindisp::TTimer* TMyBinDispCtrl::*TimerPtr;
    static TimerPtr TimerMember() { return &BinDispAccess_St02::Timer1; }
    static RxFn RxMember(int port) { return port == 0 ? &BinDispAccess_St02::CommBinReceiveData : &BinDispAccess_St02::CommBinReceiveData2; }
};
struct SysModAccess_St02 : SYSTEM_MODULAR                                       // never instantiated
{
    typedef void (SYSTEM_MODULAR::*InstallFn)(int);
    static InstallFn InstallMember() { return &SysModAccess_St02::InstallColorBinDisplay; }
};

ht9045_bindisp::TTimer* TimerOf(TMyBinDispCtrl* bd)
{
    return bd ? bd->*BinDispAccess_St02::TimerMember() : NULL;
}

// ---- (1) receive queues -------------------------------------------------------
unsigned long RealClock() { return ::GetTickCount(); }
unsigned long (*g_rxClock)() = &RealClock;

struct BinDispRxQueue
{
    CRITICAL_SECTION cs;
    std::vector<unsigned char> pending;
    unsigned long lastTick;
    BinDispRxQueue() : lastTick(0) { ::InitializeCriticalSection(&cs); }
    ~BinDispRxQueue() { ::DeleteCriticalSection(&cs); }
};
BinDispRxQueue g_rx[2];                                                         // [0] CommBin (BinDisp), [1] CommBin2 (BinDisp2)
const size_t kRxMaxPiece = 1023;                                                // golden handlers return when BufferLength>=1024

void RxQueue(int port, void* Buffer, unsigned short len)                        // the TComm reader thread (or SimInjectReceive)
{
    if (port < 0 || port > 1 || Buffer == NULL || len == 0)
        return;
    const unsigned char* p = (const unsigned char*)Buffer;
    BinDispRxQueue& q = g_rx[port];
    ::EnterCriticalSection(&q.cs);
    q.pending.insert(q.pending.end(), p, p + len);
    q.lastTick = g_rxClock();
    ::LeaveCriticalSection(&q.cs);
}

Spcomm::TReceiveDataEvent RxSink(int port)                                      // what case 1 binds instead of the golden handler
{
    return [port](TObject* /*Sender*/, void* Buffer, Spcomm::Word BufferLength) { RxQueue(port, Buffer, BufferLength); };
}

// ---- (2) / (3) ports ----------------------------------------------------------
unsigned long g_realProbes = 0;
bool g_formShown = false;
bool g_formClosed = false;
bool g_binSelPaused = false;                                                    // ST02-C14b [W906] (10): a BinSel FormShow pause waits for its FormClose

#ifndef SOFT_SIMULTE
bool PortHeldBySelf(const AnsiString& port)
{
    if (DataModule3 == NULL)
        return false;
    const AnsiString cn = AnsiString("\\\\.\\") + port;
    Spcomm::TComm* const c[2] = { DataModule3->BinDisp, DataModule3->BinDisp2 };
    for (int i = 0; i < 2; ++i)
        if (c[i] != NULL && c[i]->IsOpen() && c[i]->CommName == cn)
            return true;
    return false;
}

bool BinDispPortFree(const AnsiString& port)                                    // golden GetCOMPortStatus(port)
{
    if (W906_BinDispUnderCtest_St02())                                          // [W906] (2): ctest never probes a device
        return port.Pos("COM") > 0 && PortHeldBySelf(port) == false;
    ++g_realProbes;
    return GetCOMPortStatus(port);
}

void BinDispGoldenStartComm(Spcomm::TComm* Comm)                                // golden CommBin->StartComm(), [W906] (3)
{
    if (Comm->IsOpen())
        throw std::runtime_error("This serial port already opened");
    const bool bSimRequested = Comm->IsSimMode();
    Comm->StartComm();
    if (Comm->IsSimMode() && !bSimRequested)
    {
        Comm->StopComm();
        throw std::runtime_error("Error opening serial port");
    }
}
#endif

std::string LowerSlash(const char* s)
{
    std::string o(s ? s : "");
    for (size_t i = 0; i < o.size(); ++i)
    {
        if (o[i] >= 'A' && o[i] <= 'Z') o[i] = (char)(o[i] - 'A' + 'a');
        if (o[i] == '/') o[i] = '\\';
    }
    return o;
}

}  // namespace

bool W906_BinDispUnderCtest_St02()
{
    if (LowerSlash(std::getenv("W906_GENERAL_INI_PATH")).find("general_ini_scratch") != std::string::npos)
        return true;
    if (LowerSlash(std::getenv("W906_HT9045LOG_ROOT")).find("machine_log_scratch") != std::string::npos)
        return true;
    return false;
}

//------------------------------------------------------------------------------
// golden MyBinDisp.cpp:32-35 (empty ctor body) + MyBinDisp.dfm:1-68 (what the VCL streams in before it).
//------------------------------------------------------------------------------
TDataModule3::TDataModule3(vclcompat::TComponent* AOwner)
    : vclcompat::TComponent(AOwner),
      BinDisp(new Spcomm::TComm(this)),                                         // MyBinDisp.dfm:8  object BinDisp: TComm
      BinDisp2(new Spcomm::TComm(this))                                         // MyBinDisp.dfm:38 object BinDisp2: TComm
{
    Spcomm::TComm* const c[2] = { BinDisp, BinDisp2 };
    for (int i = 0; i < 2; ++i)                                                 // the two .dfm objects carry the same values
    {
        c[i]->CommName = "COM3";                                                // dfm :9 / :39
        c[i]->BaudRate = 9600;                                                  // :10 / :40
        c[i]->ParityCheck = false;                                              // :11 / :41
        c[i]->Outx_XonXoffFlow = false;                                         // :17 / :47
        c[i]->Inx_XonXoffFlow = true;                                           // :18 / :48
        c[i]->ByteSize = Spcomm::_8;                                            // :24 / :54
        c[i]->Parity = Spcomm::None;                                            // :25 / :55
        c[i]->StopBits = Spcomm::_1;                                            // :26 / :56
        c[i]->ReadIntervalTimeout = 100;                                        // :30 / :60 (safe since AI(W906-TORQUE-COMM): read total 50 ms)
        // [W906] (4): Outx_CtsFlow / Outx_DsrFlow False, DtrEnable, RtsEnable = what vclcompat forces; XonLimit / XoffLimit /
        //   XonChar / XoffChar / ReplacedChar / DsrSensitivity / TxContinueOnXoff / *TotalTimeout* have no vclcompat member.
    }
    bool bSim = W906_BinDispUnderCtest_St02();                                  // [W906] (2)
#ifdef SOFT_SIMULTE
    bSim = true;                                                                // [W906] (2): golden SIM never starts these ports either (Timer1Timer :355 / :435)
#endif
    if (bSim)
    {
        BinDisp->SetSimMode(true);
        BinDisp2->SetSimMode(true);
    }
}

TDataModule3::~TDataModule3()
{
    DataModuleDestroy(this);                                                    // VCL OnDestroy, then the owned components go
    delete BinDisp2;
    delete BinDisp;
}

//------------------------------------------------------------------------------
// golden MyBinDisp.cpp:3070-3073
//------------------------------------------------------------------------------
void TDataModule3::DataModuleDestroy(TObject * /*Sender*/)
{
    BinDisp->StopComm();
}

//------------------------------------------------------------------------------
// 顯示器控制中心   golden MyBinDisp.cpp:282-604 (GATE (1) of BinDisplay/MyBinDisp.h, lifted here)
//------------------------------------------------------------------------------
void TMyBinDispCtrl::Timer1Timer(TObject * /*Sender*/)
{
    static bool bMagTFT=false;
    static bool bRun=false;
    {
        AnsiString Str="";

        if(InitialOK==false)                                                    //jou 2010-05-19 start : 未initital完成,不能執行
            return;

        if(NUMBER_PANEL_TYPE==3 ||
           NUMBER_PANEL_TYPE==4)                                                //Sam 20240604 : 新增 BinDisplay TFT
        {
        }
        else
        {
            return;
        }

        if(bStopProcess==false)
            return;

        if(bRun)
            return;

        bRun=true;
        int &Task=iBinDispCtrlTask;
        AnsiString CN;

        if(IsAnyFlashing() && IniConfig.bP66AutoChangingFlashWarn)              //Eastsun 20260514
        {
            Timer1->Interval=50;
        }
        else
        {
            Timer1->Interval=iOldTimerInterval;
        }

        switch(Task)
        {
            case 1:
                iRusStatus=eBDP_Initial;                                        //Sam 20240604 : 新增 BinDisplay TFT
                bMagTFT=false;
                (void)bMagTFT;                                                  // golden writes it and never reads it (906)
                if(ComPort.Pos("COM")==0)
                {
                    bRun=false;
                    return;
                }
                if(DataModule3==NULL)                                           // [W906] (7): golden's CreateForm always ran
                {
                    bRun=false;
                    return;
                }
                CommBin=DataModule3->BinDisp;
                CommBin->OnReceiveData=RxSink(0);                               // golden =CommBinReceiveData; [W906] (1) queued
                CommBin->Parity=ComParity;

                CommBin2=DataModule3->BinDisp2;
                CommBin2->OnReceiveData=RxSink(1);                              // golden =CommBinReceiveData2; [W906] (1) queued
                CommBin2->Parity=ComParity;

                bStartSetBin=true;                                              //重設Bin
                bStartSetColor=true;                                            //重設顏色
                bStartOnce=true;                                                //開始進行設定TFT顯示器設定    //Sam 20240604 : 新增 BinDisplay TFT
                bStartCycle=true;                                               //開始進行 TFT 顯示器
                if(NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
                {
                    Timer1->Interval=30;
                    CommBin->ReadIntervalTimeout=50;
                    CommBin->Inx_XonXoffFlow=false;
                    CommBin->Outx_XonXoffFlow=false;
                }
                for(int i=0; i<MAX_BIN_UNIT; i++)
                {
                    bHasError[i]=false;
                }
                #ifndef SOFT_SIMULTE

                if(BinDispPortFree(ComPort))                                    // golden GetCOMPortStatus(ComPort); [W906] (2)
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort;
                        CommBin->CommName=CN;
                        BinDispGoldenStartComm(CommBin);                        // golden CommBin->StartComm(); [W906] (3)
                        Str.sprintf("BinDisp, Start Comm OK., %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error open com port");
                        Str.sprintf("BinDisp, Start Comm NG!, %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    Task=50;
                    iStartGetStatusTask=1;
                }
                else
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort;
                        CommBin->CommName=CN;
                        CommBin->StopComm();
                        Str.sprintf("BinDisp, Stop Comm OK., %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error close com port");
                        Str.sprintf("BinDisp, Stop Comm NG!, %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                }

                if(BinDispPortFree(ComPort2))                                   // golden GetCOMPortStatus(ComPort2); [W906] (2)
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort2;
                        CommBin2->CommName=CN;
                        BinDispGoldenStartComm(CommBin2);                       // golden CommBin2->StartComm(); [W906] (3)
                        Str.sprintf("BinDisp, Start Comm OK., %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error open com port");
                        Str.sprintf("BinDisp, Start Comm NG!, %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    Task=50;
                    iStartGetStatusTask=1;
                }
                else
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort2;
                        CommBin2->CommName=CN;
                        CommBin2->StopComm();
                        Str.sprintf("BinDisp, Stop Comm OK., %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error close com port");
                        Str.sprintf("BinDisp, Stop Comm NG!, %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                }

                #else
                    bStartSetColor=false;
                    Task=50;
                    iStartGetStatusTask=1;
                #endif
                break;
            case 50:
                if(DoStartGetStatus())
                {
                    Task=100;
                }
                break;
            case 100:
                if(bHasUnit==false)
                    break;
                #ifndef SOFT_SIMULTE
                if(BinDispPortFree(ComPort))                                    //假設RS232斷線   // golden GetCOMPortStatus(ComPort); [W906] (2)
                {
                    iRusStatus=eBDP_Initial;                                    //Sam 20240604 : 新增 BinDisplay TFT
                    CommBin->StopComm();
                    CommBin2->StopComm();
                    for(int i=0; i<MAX_BIN_UNIT; i++)
                    {
                        iBinNow[i]=0;                                           // 顯示器目前的Bin
                        iColorNow[i]=1;                                         // 顯示器目前的的Color
                    }

                    InitialTask();                                              //Sam 20240604 : 新增 BinDisplay TFT
                    break;
                }

                if(BinDispPortFree(ComPort2))                                   //假設RS232斷線   // golden GetCOMPortStatus(ComPort2); [W906] (2)
                {
                    iRusStatus=0;
                    CommBin2->StopComm();

                    for(int i=0; i<MAX_BIN_UNIT; i++)
                    {
                        iBinNow[i]=0;                                           // 顯示器目前的Bin
                        iColorNow[i]=1;                                         // 顯示器目前的的Color
                    }

                    bStartSetColor=true;                                        //重設顏色
                    bStartSetBin=true;                                          //重設Bin
                    Task=1;
                    break;
                }

                if(IsAnyFlashing())                                             //Eastsun 20260513 : 閃爍功能
                {
                    bStartSetColor = true;
                }
                #endif

                if(NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
                {
                    if(bStartOnce)
                    {
                        iOnceTask=1;
                        Task=400;
                    }
                    else if(bStartCycle)
                    {
                        iCycleTask=1;
                        Task=500;
                    }
                }
                else
                {
                     if(bStartSetColor)
                     {
                         iStartSetColorTask=1;
                         Task=200;
                     }
                     else if(bStartSetBin)
                     {
                         iStartSetBinTask=1;
                         Task=300;
                     }
                     else if(bStartSetBin==false)
                     {
                         iStartSetBinTask=100;
                         Task=300;
                     }
                }
                break;
            case 200:
                if(DoStartSetColor())
                {
                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                    {
                        iStartSetBinTask=1;
                        Task=300;
                    }
                    else
                    {
                        bStartSetColor=false;
                        Task=100;
                    }
                }
                break;
            case 300:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                    break;
                }

                if(DoStartSetBin())
                {
                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                        Task=600;
                    else
                        Task=100;
                }
                break;
            case 400:                                                           //NUMBER_PANEL_TYPE==4
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                    break;
                }

                if(DoOnce())
                {
                    bStartOnce=false;

                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                    {
                        Task=200;
                        iStartSetColorTask=1;
                    }
                    else
                    {
                        Task=100;
                    }
                }
                break;
            case 500:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                    break;
                }

                if(DoCycle())
                {
                    bStartCycle=false;
                    Task=100;
                }
                break;
            case 600:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                    break;
                }

                if(DoCycle())
                {
                    bStartCycle=false;
                    Addr=0;
                    iStartSetBinTask=100;
                    Task=300;
                }
        }
    }
    bRun=false;
//    #endif
}

//------------------------------------------------------------------------------
// golden database.cpp:1543-1545 (inside SYSTEM_MODULAR::SystemModularInitial, :1539-1546) + HT9045.cpp:212.
// [W906] only the bin-display half: the :1541 MyGem=new HT9045Gem half is NOT run here (wb_serve never calls
// SystemModularInitial, so it has no HT9045Gem today; changing that is a SECS decision, not this card's).
//------------------------------------------------------------------------------
void W906_BinDispSystemModularBoot_St02()
{
    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {
        if(DataModule3==NULL)                                                   // [W906] (5): CreateForm only for 3 / 4
            DataModule3=new TDataModule3(NULL);                                 // golden HT9045.cpp:212
        if(HSys.BinDisCtrl==NULL)                                               // once (golden: one HSys ctor)
            (HSys.*SysModAccess_St02::InstallMember())(NUMBER_PANEL_TYPE);      // golden :1545 InstallColorBinDisplay(NUMBER_PANEL_TYPE); [W906] (9)
        if(HSys.BinDisCtrl!=NULL && HSys.BinDisCtrl->slBinDispLog!=NULL)
            HSys.BinDisCtrl->slBinDispLog->Path=as9045LogPath+"\\BinDisplayLog";   // [W906] (6): golden literal unless the ctest seam is set
        std::printf("[ST02-C14] bin display: NUMBER_PANEL_TYPE=%d, BinDisCtrl=%s, ports %s / %s (%s)\n",
                    NUMBER_PANEL_TYPE, HSys.BinDisCtrl ? "TMyBinDispHT9046" : "(none)",
                    HSys.sNumberPanelComPort.c_str(), HSys.sNumberPanelComPort2.c_str(),
                    (DataModule3 && DataModule3->BinDisp->IsSimMode()) ? "sim, no COM port is opened" : "real ports");
        std::fflush(stdout);
    }
}

//------------------------------------------------------------------------------
// golden TfMain::FormShow main.cpp:10483-10485 (after InitialOK=true at :10464)
//------------------------------------------------------------------------------
void W906_BinDispFormShowAt_St02()
{
    if(g_formShown || InitialOK==false)
        return;
    g_formShown=true;
    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {
        if(HSys.BinDisCtrl!=NULL)                                               // [W906] (7)
            HSys.BinDisCtrl->InitialOK=InitialOK;
    }
}

//------------------------------------------------------------------------------
// golden TfMain::FormClose main.cpp:11565-11569 (bSystemClose=true, then the copy; golden 906 does not stop the ports here)
//------------------------------------------------------------------------------
void W906_BinDispFormClose_St02()
{
    if(g_formClosed)
        return;
    g_formClosed=true;
    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {
        if(HSys.BinDisCtrl!=NULL)                                               // [W906] (7)
            HSys.BinDisCtrl->InitialOK=InitialOK;
    }
}

//------------------------------------------------------------------------------
// golden TfBinSel::FormShow cBinSel.cpp:1719-1721
//------------------------------------------------------------------------------
const char* W906_BinDispBinSelFormShow_St02()
{
    if(!(NUMBER_PANEL_TYPE==3 ||
         NUMBER_PANEL_TYPE==4))                                                 //Sam 20240604 : 新增 BinDisplay TFT
        return "bin display: not type 3 / 4";
    if(HSys.BinDisCtrl==NULL)                                                   // [W906] (7)
        return "bin display: no instance";
    if(SystemStart)                                                             // [W906] (8): golden 0618 main.cpp:3842-3846 (ST02-C14b: was || SoftStart)
        return "bin display: running -- golden cannot open TfBinSel now, not paused";
    HSys.BinDisCtrl->ProcessStopStart(false);  g_binSelPaused=true;            // [W906] (10)
    return "bin display: ProcessStopStart(false) (golden cBinSel.cpp:1721)";
}

//------------------------------------------------------------------------------
// ST02-C14b (NB2 R168 M1) [W906] (10): golden TfBinSel is ShowModal, so its FormClose (golden 0618 cBinSel.cpp:2137-2145; port
// FileRW/BinSelect.cpp FileRW_BinSelect_WindowEdge) always follows the FormShow pause above.  The web page can be closed while
// SystemStart is on; the close tail is then skipped (FileRW/MainClick.cpp W906_EvB10A_WindowEdge kSkipRunning) and never run
// later, so the panels would stay paused for the whole lot.  Once the BinSel window is no longer showing (the page table), this
// runs the display half of that FormClose -- fShowBinSelect->InitShowBinDigital() (golden 0618 cBinSel.cpp:2141) -- and the next
// DoShowBinDigital re-sends the target bins and ProcessStopStart(true)s.  MainTimer3.cpp calls it once a second right before
// golden Timer3's DoShowBinDigital.  After an idle close the close tail has already done it; this then re-arms it once more
// (harmless: InitShowBinDigital only asks DoShowBinDigital to re-send).  Bin data (ReadFile) is NOT reloaded here: that half of
// FormClose stays with the close tail, which skips it mid-run on purpose.
//------------------------------------------------------------------------------
bool W906_FormShowing(const char* goldenForm, bool member);                     // csystem.cpp (csystem.h:440 / W906FormShowing.h; not included here)
const char* W906_BinDispBinSelResumeTick_St02()
{
    if(!g_binSelPaused)
        return "bin display: not paused by the BinSel page";
    if(W906_FormShowing("fBinSel", false))                                      // the page table only (fBinSel->bShow stays true when the close tail is skipped)
        return "bin display: BinSel page still open";
    g_binSelPaused=false;
    fShowBinSelect->InitShowBinDigital();                                       // golden 0618 cBinSel.cpp:2141 (TfBinSel::FormClose)
    return "bin display: BinSel page closed -> InitShowBinDigital (golden 0618 cBinSel.cpp:2141)";
}

//------------------------------------------------------------------------------
// ST02-C14b (NB2-2 A5; RULINGS_20261003 #19: HT9050 is NUMBER_PANEL_TYPE 4 = TFT, COM14): golden 0618 cSortCT.cpp:407-408 in
// TfSortCT::ShowSortIC -- `if(NUMBER_PANEL_TYPE==4) HSys.BinDisCtrl->WriteTargetCount(iTo3Unload[i]+3, LastSet.BinCT[0][...]);`,
// the count each TFT unit shows (TMyBinDispCtrl::iSetCount, sent by WriteCount_TFT).  cSortCT.cpp's GATE G2 (now lifted) calls
// this: that TU has neither database.h (HSys) nor BinDisplay/MyBinDisp.h, and adding includes would move St01's lines.
// [W906] (7) NULL guard.  (The H1 boot call once here is gone: the port's boot already runs golden's first InitShowBinDigital,
// wb_serve.cpp:4159 W906_DoReadLastData(true) -> :3594 fMain->SetStartModeData() -> RunStartMode.cpp SetRunStartMode :783 --
// golden main.cpp:9577; :10203 is golden's second call.  NB2-2 A5, re-verified by St02-E 1003.)
//------------------------------------------------------------------------------
void W906_BinDispWriteTargetCount_St02(int Index, int iCount)
{
    if(HSys.BinDisCtrl==NULL)                                                   // [W906] (7)
        return;
    HSys.BinDisCtrl->WriteTargetCount(Index, iCount);                          // golden 0618 cSortCT.cpp:408
}

//------------------------------------------------------------------------------
// ST02-C14b (NB2 R168 M2): golden 0618 csystem.cpp:4354-4355 (DoSystem, the SnSystemPower-off branch, after SystemStart=false):
// re-initialise the Bin display after a system power loss.  csystem.cpp's GATE G09 (now lifted, it calls this) kept it off
// because BinDisCtrl was NULL before ST02-C14.  [W906] (7) NULL guard: no instance on a machine without a type 3 / 4 display.
//------------------------------------------------------------------------------
void W906_BinDispPowerLossReinit_St02()
{
    if(HSys.BinDisCtrl==NULL)                                                   // [W906] (7)
        return;
    HSys.BinDisCtrl-> bFirstInit=true;                                          // golden 0618 csystem.cpp:4354
    HSys.BinDisCtrl->ProcessStopStart(true);                                    // golden 0618 csystem.cpp:4355
}

//------------------------------------------------------------------------------
// [W906] (1): SPComm's main-thread delivery, on the tick thread
//------------------------------------------------------------------------------
void W906_BinDispRxPumpAt_St02(unsigned long now)
{
    TMyBinDispCtrl* const bd = HSys.BinDisCtrl;
    if (bd == NULL)
        return;
    for (int port = 0; port < 2; ++port)
    {
        Spcomm::TComm* const c = (port == 0) ? bd->CommBin : bd->CommBin2;
        const unsigned long gap = (c != NULL && c->ReadIntervalTimeout != 0) ? c->ReadIntervalTimeout : 100;
        std::vector<unsigned char> frame;
        BinDispRxQueue& q = g_rx[port];
        ::EnterCriticalSection(&q.cs);
        if (q.pending.empty() == false &&
            ((long)(now - q.lastTick) >= (long)gap || q.pending.size() >= kRxMaxPiece))   // ST02-C14b (NB2 R168 low): signed -- the reader thread can stamp lastTick after this loop's `now`
        {
            frame.swap(q.pending);
        }
        ::LeaveCriticalSection(&q.cs);
        for (size_t off = 0; off < frame.size(); off += kRxMaxPiece)
        {
            const size_t n = (frame.size() - off < kRxMaxPiece) ? frame.size() - off : kRxMaxPiece;
            (bd->*BinDispAccess_St02::RxMember(port))((TObject*)c, &frame[off], (Spcomm::Word)n);   // golden Sender = the TComm
        }
    }
}

unsigned long W906_BinDispTimer1Interval_St02()
{
    ht9045_bindisp::TTimer* const t = TimerOf(HSys.BinDisCtrl);
    if (t == NULL || t->Enabled == false || t->Interval <= 0 || !t->OnTimer)
        return 0;
    return (unsigned long)t->Interval;
}

void W906_BinDispTimer1Fire_St02()
{
    ht9045_bindisp::TTimer* const t = TimerOf(HSys.BinDisCtrl);
    if (t != NULL && t->Enabled && t->OnTimer)
        t->OnTimer(NULL);                                                       // golden Sender = Timer1 (not a TObject in this port; Timer1Timer ignores it)
}

// ---- ST02-C14 part 3: the web "Bin Display Status" pane (banner of BinDispBringUp_St02.h, W906_BinDispShownScope_St02) ----
namespace {
unsigned long g_jumpSeq = 0;                                                    // golden 906 cShowBinSelect.cpp:272 jumps seen
}

W906_BinDispShownScope_St02::W906_BinDispShownScope_St02()
    : keepPage_(NULL), keepIndex_(0), armed_(false)
{
    TfShowBinSelect* const f = fShowBinSelect;
    if (f == NULL || f->PageControl1 == NULL)
        return;
    keepPage_  = f->PageControl1->ActivePage;
    keepIndex_ = f->PageControl1->ActivePageIndex;
    f->PageControl1->ActivePage      = f->tsUnloadMap;                          // [W906] tsUnloadMap counts as shown (golden :283 reads it)
    f->PageControl1->ActivePageIndex = -1;                                      // [W906] only golden's own jump (:272) can make it 3
    armed_ = true;
}

W906_BinDispShownScope_St02::~W906_BinDispShownScope_St02()
{
    TfShowBinSelect* const f = fShowBinSelect;
    if (!armed_ || f == NULL || f->PageControl1 == NULL)
        return;
    if (f->PageControl1->ActivePageIndex == 3)                                  // golden :269-273 ran in this call
    {
        ++g_jumpSeq;
        f->PageControl1->ActivePage = f->tsUnloadMap;                           // VCL: setting ActivePageIndex also sets ActivePage
    }
    else
    {
        f->PageControl1->ActivePage      = static_cast<TTabSheet*>(keepPage_);
        f->PageControl1->ActivePageIndex = keepIndex_;
    }
}

unsigned long W906_BinDispJumpSeq_St02()
{
    return g_jumpSeq;
}

// ---- ctest only ---------------------------------------------------------------
void W906_BinDispTestReset_St02()
{
    g_jumpSeq = 0;                                                              // ST02-C14 part 3
    g_formShown = false;
    g_formClosed = false;
    g_binSelPaused = false;                                                     // ST02-C14b
    g_realProbes = 0;
    for (int i = 0; i < 2; ++i)
    {
        ::EnterCriticalSection(&g_rx[i].cs);
        g_rx[i].pending.clear();
        g_rx[i].lastTick = 0;
        ::LeaveCriticalSection(&g_rx[i].cs);
    }
}

void W906_BinDispSetRxClock_St02(unsigned long (*clock)())
{
    g_rxClock = clock ? clock : &RealClock;
}

unsigned long W906_BinDispRealPortProbes_St02()
{
    return g_realProbes;
}
