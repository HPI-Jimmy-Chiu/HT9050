// ===========================================================================
//  TesterComm/Handler/HandlerTesterSide.cpp -- construction and the V906 plumbing of THandlerTesterSide (see the
//  header).  AI(W906-GB-P2a) 20260926.  The golden bodies are in HandlerGpibMsg.cpp and HandlerBridgeCtl.cpp, except the
//  tESDError part of golden TimerESDTimer at the end of this file (AI(W906-S13) 20261001, St02-E).
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/TesterCommHub.h"
#include "TesterComm/Gpib/GpibEngine.h"
#include "TesterComm/Rs232/Rs232Engine.h"
#include "cprod.h"     // TestIF (SYSTEM_TEST_IF)
#include "cmydef.h"    // TTL_MODE / GPIB_MODE / RS232_MODE / TCP_IP_MODE, ON_LINE
#include "LastSet.h"   // LastSet.iTester (P2d Off-Line rule)
#include "Config.h"    // IniConfig.iI25UseGPIBFormat (P6)
#include "forms/fSCKART.h"   // fSCKART->iTesterType (P6)
#include "TesterComm/HandlerSettings.h"   // P6 snapshot
#include "TesterComm/Handler/HandlerGpibAux.h"   // P6 2A + Q2(a): the GPIB extra RS232 port follows the recipe
#include "MachineType.h"          // AI(W906-S13) 20261001: MAX_Index_Row (TimerESDDrainESDError, file end)
#include "aHotPlateSubstrate.h"   // AI(W906-S13): FTestSuck / BTestSuck / TestSocket (not mykitsuck.h, KNOWLEDGE.md two-TMyKitSuck)
#include "canary_support.h"       // AI(W906-S13): ShowErrorMessage / RecordProcess
#include "csystem.h"              // AI(W906-S13): InitOneCycle, W906_FormShowing
#include "atester.h"              // AI(W906-S13): TestProcessSetToErr
#include "forms/fNote.h"          // AI(W906-S13): fNote->fShow

#include <cstring>
#include <vector>

THandlerTesterSide *fTesterSide = NULL;

namespace {
VM g_lastPacket;   // zero-initialised; Handler thread only

VM* KeepLastPacket(const void* packet)
{
    std::memcpy(&g_lastPacket, packet, sizeof(g_lastPacket));
    return &g_lastPacket;
}
}  // namespace

THandlerTesterSide::THandlerTesterSide()
    : bFind(false),                    // golden TfMain::TfMain main.cpp:2201
      HVisionWnd(NULL),                // :2214
      lblGPIBWND(new TLabel),
      mmo1(new TMemo),
      tESDError(new TStringList()),    // :2280
      bAMDRs232ConnectError(false),    // BCB zero-fills a new TForm; golden FormShow :11673 sets it for AMD (P2b)
      bHasPin1Error(false),
      bReceivePPSELECT(false),         // :1735
      bTriggerESC(false),              // :1385
      oldLastSetiTester(-1),           // :2206   AI(W906-GB-P2f) 20260926
      oldLastiTestBinCount(0),         // :2209   golden `=false`
      oldbA10_6(true),                 // :2210   golden `=-1` on a bool
      oldLastiTestMode(-1),            // :2211
      hub(&testercomm::TesterCommHub::Instance()),
      sinkMessages(0),
      depth(0)
{
    WakeupGPIBdelay.SetSecAndOn(1);    // :2247
    WakeupESDdelay.SetSecAndOn(1);     // :2249 (AI(W906-ESD-G5) 20260927)
}

THandlerTesterSide::~THandlerTesterSide()
{
    if (hub)
        hub->SetHandlerSink(0, 0);
    delete lblGPIBWND;
    delete mmo1;
    if (tESDError)
        tESDError->Clear();
    delete tESDError;
}

void THandlerTesterSide::Attach(testercomm::TesterCommHub* h)
{
    if (hub && hub != h)
        hub->SetHandlerSink(0, 0);
    hub = h;
    if (hub)
        hub->SetHandlerSink(&THandlerTesterSide::Sink, this);
}

HWND THandlerTesterSide::HandlerWndToken()
{
    // The engines set their HMountWnd to the mailbox they were started with (GpibEngine / Rs232Engine::Start).
    return hub ? reinterpret_cast<HWND>(&hub->Mailbox()) : NULL;
}

HWND THandlerTesterSide::FindBridgeWindow()
{
    // golden FindWindow("TSerialPoll", "<model>GPIB") / FindWindow("TfRS232Main", "RS232Standard"): the running
    // bridge's identity token, or NULL when no bridge program is up.
    if (hub == 0 || !hub->IsUp())
        return NULL;
    HWND h = gpibbridge::GpibEngine::BridgeWndToken();
    if (h == NULL)
        h = rs232std::Rs232Engine::BridgeWndToken();
    return h;
}

bool THandlerTesterSide::OffLineGpibWay()
{
    //AI(W906-GB-P2d) 20260926: USER RULINGS 20260926 (github-59 relayed, FROM_STEVEN §1 P2d row):
    //   "on line 時, 各走各的；off line 時, 統一走 gpib 的方式就好", then "流程上就是跑 GPIB 程式在 off line 模式時
    //   會做的事情" and "使用 GPIB 的相關設定".  So while not ON_LINE (the same test as golden's bSimulate,
    //   main.cpp:19083), an RS232 / TTL recipe gets the GPIB engine in simulate -- no Tester / TTL COM port is
    //   opened -- and every packet is filled the GPIB way.  DEVIATION from golden for RS232 / TTL only: golden
    //   runs RS232Standard.exe in its own simulate mode with its COM ports open (Rs232Ui.cpp:2334,
    //   Rs232HandlerMsg.cpp:717-744).  GPIB and TCP/IP recipes already go the GPIB way in golden.
    return LastSet.iTester!=ON_LINE &&
           (TestIF.iTestType==RS232_MODE || TestIF_File.iTestType==TTL_MODE);
}

void THandlerTesterSide::PublishSettings()
{
    //AI(W906-GB-P6) 20260926: rulings C-2 (暫照建議，待使用者確認 decision #5): the engine reads these instead of its own ini.
    //   3A / Q1(a): the Auto Retest brand the Handler holds now (golden learning kept on both sides).
    //   4A / Q4 A : the Handler-ID reply format from the Handler's config.
    //   Called before every bridge start, after the Handler's own brand learning (HandlerGpibMsg.cpp) and every tick.
    testercomm::HsArtTesterType().store(fSCKART ? fSCKART->iTesterType : -1);
    testercomm::HsUseGPIBFormat().store(IniConfig.iI25UseGPIBFormat);
}

int THandlerTesterSide::EffectiveBridgeTestType()
{
    return OffLineGpibWay() ? GPIB_MODE : TestIF.iTestType;
}

bool THandlerTesterSide::StartBridgeProgram()
{
    // golden CreateProcess(d:\gpib9045\H9046_32GPIB.exe / d:\RS232Standard\RS232Standard.exe), and
    // TerminateProcess of the old one when TestIF.iTestType changed (WakeupGPIB).  Which engine serves which type
    // is registered by the composition root (P3): GPIB_MODE -> GpibEngine, RS232_MODE and TTL_MODE -> Rs232Engine.
    if (hub == 0)
        return false;
    const int t = EffectiveBridgeTestType();   //AI(W906-GB-P2d) 20260926: the Off-Line rule (see OffLineGpibWay)
    PublishSettings();                          //AI(W906-GB-P6) 20260926: before the engine starts / restarts (its FormShow reads them)
    if (hub->CurrentTestType() != t || !hub->IsUp())
        W906_GpibAuxBeforeBridgeStart(t);       //AI(W906-GB-P6) 20260926: 2A + Q2(a) -- seeds the recipe once, publishes the framing LoadSetupData reads
    if (hub->CurrentTestType() != t)
        return hub->SelectTestType(t);
    if (!hub->IsUp())
        return hub->Restart();   // it closed itself (golden: the exe exited and is launched again)
    return true;
}

void THandlerTesterSide::SendToBridge(COPYDATASTRUCT* pcp)
{
    // golden SendMessage(HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp): synchronous; SendMessage to a NULL
    // or dead window just returns 0.
    if (hub == 0 || pcp == 0 || pcp->lpData == 0)
        return;
    const std::string payload(static_cast<const char*>(pcp->lpData), static_cast<size_t>(pcp->cbData));
    int result = 0;
    hub->SendToEngine(payload, &result);
}

int THandlerTesterSide::Sink(const std::string& payload, void* ctx)
{
    // A bridge -> Handler packet (golden WM_COPYDATA with WParam WM_GPIB_Program = 0) on the Handler thread.
    THandlerTesterSide* self = static_cast<THandlerTesterSide*>(ctx);
    if (self == 0)
        return 0;
    ++self->sinkMessages;

    // WM_COPYDATA gives the receiver a private copy for the duration of the message: zero-padded, 8-byte aligned,
    // at least sizeof(VM) so a short packet can never make golden read past the bridge's bytes.
    const size_t n = payload.size() < sizeof(VM) ? sizeof(VM) : payload.size();
    std::vector<unsigned long long> buf(n / sizeof(unsigned long long) + 1, 0ULL);
    if (!payload.empty())
        std::memcpy(&buf[0], payload.data(), payload.size());
    COPYDATASTRUCT cds;
    cds.dwData = 0;
    cds.cbData = static_cast<DWORD>(payload.size());
    cds.lpData = &buf[0];

    // HGpib2Handler points into `buf` during the call.  A nested bridge message (the Handler answers with
    // SendMSG_CMD while handling, and the mailbox pumps) repoints it; restore the outer packet afterwards.  After the
    // outermost call golden's pointer still aimed at the (freed) WM_COPYDATA copy, whose bytes were the last packet;
    // Command.cpp reads HGpib2Handler->cReturn, so instead of NULL it is left on a persistent copy of that packet.
    VM* outer = HGpib2Handler;
    ++self->depth;
    try
    {
        self->OnGpibProgramMsg(reinterpret_cast<LPARAM>(&cds));
    }
    catch (...)
    {
        --self->depth;
        HGpib2Handler = self->depth > 0 ? outer : KeepLastPacket(&buf[0]);
        throw;
    }
    --self->depth;
    HGpib2Handler = self->depth > 0 ? outer : KeepLastPacket(&buf[0]);
    return 0;
}

// ===========================================================================
//  AI(W906-S13) 20261001 (St02-E): golden TfMain::TimerESDTimer, the tESDError part -- 906_0625_Steven main.cpp:30937-31026
//  ("Ifor 20180821 : Add 避免ESD Alarm 短時間內重複發生").  Called once per TimerESD tick from MainTimerESD.cpp
//  W906_TimerESDTimer through ht9045::W906_ESDErrorDrainHook, after golden's guards there (InitialOK, bRun, and the
//  SOFT_SIMULTE return -- the SIM build never gets here from the timer).  The queue is this class's tESDError (golden
//  main.h:1394; HandlerGpibMsg.cpp adds the codes).  With it here: tESDAlarmTimer (golden TfMain member main.h:1547,
//  latched in FormShow :11152) and asOldESDError (golden function static, :30915).
//  [W906] golden `fNote->fShow==false` is read through the page table too (W906_FormShowing, as forms/fNote_ShowError.cpp:372).
//  [W906] golden BtnOneCycle (TfMain member) is fMain->BtnOneCycle (the header's rule for TfMain members).
//  ShowErrorMessage waits like golden's ShowModal in wb_serve (ForwardShowErrorMessage); the code is deleted only after
//  the answer, as golden.  KNOWN GAP (not this file): TestProcessSetToErr (atester.cpp:12502, laptop) is still an empty
//  stand-in -- its golden body (atester.cpp:11772-11818) is gated there -- so WAR0732x / WAR07317 + Skip does not set the
//  socket devices to the error bin yet.
// ===========================================================================
namespace {
TQPF_Timer tESDAlarmTimer;                                                      // golden TfMain member, main.h:1547
struct LatchESDAlarmTimerAtBoot { LatchESDAlarmTimerAtBoot() { tESDAlarmTimer.LatchCycleTime(true); } } g_latchESDAlarmTimer;   // golden FormShow :11152
int (*g_esdAlarmAgeForTest)() = 0;                                              // ctest seam (TimerESDSetAlarmAgeForTest)
int ESDAlarmAge() { return g_esdAlarmAgeForTest ? g_esdAlarmAgeForTest() : tESDAlarmTimer.LatchCycleTime(); }
}  // namespace

void THandlerTesterSide::TimerESDSetAlarmAgeForTest(int (*fn)()) { g_esdAlarmAgeForTest = fn; }

void THandlerTesterSide::ESDErrorDrainHook()
{
    if (fTesterSide != NULL)
        fTesterSide->TimerESDDrainESDError();
}

void THandlerTesterSide::TimerESDDrainESDError()
{
    static AnsiString asOldESDError="";                                         //Ifor 20180821 :Add 判斷ESD Alarm Code 是否相同
    int iRet;

    //Ifor 20180821 : Add 避免ESD Alarm 短時間內重複發生
    //==>
    if(tESDError->Count!=0 && W906_FormShowing("fNote", fNote->fShow)==false)  //Steven 20220610 : Note畫面沒顯示才可以Alarm   [W906] golden fNote->fShow==false, through the page table
    {
        if(asOldESDError==tESDError->Strings[0] && ESDAlarmAge()<1500)          //相同Alarm需間隔1.5秒才警報(設定秒數須大於Scan Time 1秒)   [W906] golden tESDAlarmTimer.LatchCycleTime()
        {
            RecordProcess("The same Alarm occurs : "+asOldESDError);            //紀錄發生相同Alarm
            asOldESDError="";
        }
        else
        {
            asOldESDError=tESDError->Strings[0];
            if(asOldESDError=="MES0731")                                        //Steven 20220517 : Add for GIGA
            {
                iRet=ShowErrorMessage(tESDError->Strings[0], K_SKIP|K_ONECYCLE, MMSystem, false, "TimerESD");
                if(iRet==K_SKIP)                                                //做Reset
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC && FTestSuck.Item[i][j]!=HAS_NULL_IC && FTestSuck.Item[i][j]<TEST_PASS)
                            {
                                FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                FTestSuck.PordRec[i][j].AddTestResultRecord(iTestBinCount, FTestSuck.cSBin[i][j], "RESET");                                     //Frank 20160505 add
                            }

                            if(BTestSuck.Item[i][j]!=NULL_IC && BTestSuck.Item[i][j]!=HAS_NULL_IC && BTestSuck.Item[i][j]<TEST_PASS)
                            {
                                BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                BTestSuck.PordRec[i][j].AddTestResultRecord(iTestBinCount, BTestSuck.cSBin[i][j], "RESET");                                     //Frank 20160505 add
                            }

                            if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC && TestSocket.Item[i][j]<TEST_PASS)
                            {
                                TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "RESET");                                   //Frank 20160505 add
                            }
                        }
                    }

                    InitOneCycle("After Tester Pause handler, user skip IC on index arm.");
                }
                else                                                            //if(iRet==K_ONECYCLE)
                {
                    InitOneCycle("After Tester Pause handler, user perform one cycle.");
                }
                fMain->BtnOneCycle->Down=true;                                  // [W906] golden BtnOneCycle->Down=true;
            }
            else if(asOldESDError=="WAR07326" ||                                //Steven 20231017 : GPIB flow error need alarm
                    asOldESDError=="WAR07327" ||
                    asOldESDError=="WAR07328" ||
                    asOldESDError=="WAR07317")                                  //Steven 20231205 : 判斷RS232流程是否異常
            {
                if(IniConfig.iI46_ActionWhenGpibFlowErr==1)
                    iRet=ShowErrorMessage(tESDError->Strings[0], K_SKIP, MMSystem, false, "TimerESD");
                else if(IniConfig.iI46_ActionWhenGpibFlowErr==2)
                    iRet=ShowErrorMessage(tESDError->Strings[0], K_RETRY, MMSystem, false, "TimerESD");
                else
                    iRet=ShowErrorMessage(tESDError->Strings[0], K_SKIP|K_RETRY, MMSystem, false, "TimerESD");

                if(iRet==K_SKIP)
                {
                    TestProcessSetToErr("Test flow error set socket device to Error bin");
                }
                else
                {
                }
            }
            else if(asOldESDError=="MES1713" ||                                 //RogerYang 20250626 偉測不可複測bin功能
                    asOldESDError=="MES1813" ||
                    asOldESDError=="MES1913")
            {
                ShowErrorMessage(tESDError->Strings[0], K_RETRY|K_SKIP, MMSystem, false, "TimerESD");
            }
            else
            {
                ShowErrorMessage(tESDError->Strings[0], K_RETRY, MMSystem, false, "TimerESD");
            }
//            dESDAlarmTime = MyTickCount();
            tESDAlarmTimer.LatchCycleTime(true);
        }

        tESDError->Delete(0);
        if(tESDError->Count==0)                                                 //釋放記憶體
        {
            tESDError->Clear();
        }
    }
    //<==
    //Ifor 20180821 : Add 避免ESD Alarm 短時間內重複發生
}
