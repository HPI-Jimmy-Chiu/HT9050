// ===========================================================================
//  TesterComm/Handler/HandlerTesterSide.cpp -- construction and the V906 plumbing of THandlerTesterSide (see the
//  header).  AI(W906-GB-P2a) 20260926.  The golden bodies are in HandlerGpibMsg.cpp and HandlerBridgeCtl.cpp.
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/TesterCommHub.h"
#include "TesterComm/Gpib/GpibEngine.h"
#include "TesterComm/Rs232/Rs232Engine.h"
#include "cprod.h"     // TestIF (SYSTEM_TEST_IF)
#include "cmydef.h"    // TTL_MODE / GPIB_MODE / RS232_MODE / TCP_IP_MODE, ON_LINE
#include "LastSet.h"   // LastSet.iTester (P2d Off-Line rule)

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
