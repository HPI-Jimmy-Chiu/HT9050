// ===========================================================================
//  TesterComm/Gpib/GpibEngine.cpp -- see GpibEngine.h.  AI(W906-GB-P1) 20260926.
//
//  Also defines the V906 plumbing members of TSerialPoll declared in GpibBridge.h (PostToHandler, RequestClose,
//  UiNotice, QueueRx, DrainRx): they are the engine's side of the contract, not golden text.
// ===========================================================================
#include "TesterComm/Gpib/GpibEngine.h"
#include "TesterComm/Gpib/GpibBridge.h"
#include "TesterComm/SyncMailbox.h"
#include "TesterComm/TesterCommHub.h"   // kDefaultSendTimeoutMs
#include "TesterComm/UiChannel.h"
#include "TesterComm/Gpib/GpibUiSnapshot.h"

#include <cstring>
#include <new>
#include <string>
#include <utility>
#include <vector>

namespace gpibbridge {

namespace {

// Golden's globals are singletons (one bridge per process); allow one live engine at a time.
std::atomic<int> g_live(0);
std::atomic<IGpibDriver*> g_injected(0);
std::atomic<void*> g_handlerTok(0);
std::atomic<void*> g_bridgeTok(0);
std::atomic<unsigned long> g_postFailures(0);
std::atomic<unsigned long> g_handlerMsgs(0);
std::string g_genIniOverride;    // written before the hub starts the thread, read in Start (thread start orders it)
std::string g_hgenIniOverride;

unsigned long long NowMs()
{
    LARGE_INTEGER f, c;
    ::QueryPerformanceFrequency(&f);
    ::QueryPerformanceCounter(&c);
    if (f.QuadPart <= 0)
        return (unsigned long long)::GetTickCount();
    return (unsigned long long)(c.QuadPart * 1000LL / f.QuadPart);
}

// How long RunOnce lets the thread sleep.  Golden TMyThread: SleepEx(1) then Synchronize(Process); a mailbox
// arrival or a queued serial chunk wakes the thread earlier.  (No timeBeginPeriod: golden did not raise the timer
// resolution either, so the effective cadence is the same system tick golden ran at.)
const unsigned kLoopMs = 1;
const unsigned kIdleMs = 50;   // after the bridge closed itself: nothing to do until the hub stops us
const unsigned kUiPeriodMs = 200;   // P7 snapshot cadence (ruling 1: the page may lag a little, the IO may not)
const char* const kUiKey = "gpib";

// VCL TApplication::CreateForm(InstanceClass, &Reference): NewInstance (memory zero-filled by TObject.InitInstance),
// then Reference := Instance, THEN the constructor runs -- so golden constructors (and anything they call) already
// see the global form pointer set, and members a constructor does not touch read as 0.  Reproduced exactly.
template <class T>
T* VclCreateForm(T*& reference)
{
    void* mem = ::operator new(sizeof(T));
    std::memset(mem, 0, sizeof(T));
    reference = static_cast<T*>(mem);
    try
    {
        new (mem) T();
    }
    catch (...)
    {
        reference = 0;
        ::operator delete(mem);
        throw;
    }
    return reference;
}

}  // namespace

// ---------------------------------------------------------------------------
//  GpibEngine
// ---------------------------------------------------------------------------
GpibEngine::GpibEngine()
    : mailbox_(0), ownedDriver_(0), up_(false), started_(false), closed_(false), depth_(0), lastUiMs_(0),
      padSaidCard_(-1), padSaidWanted_(-1)   //AI(W906-GB-PADSEEN) 20261007
{
}

GpibEngine::~GpibEngine()
{
    // Normal path: Stop() already ran on the TesterComm thread.  If Start() succeeded but Stop() never ran (the
    // thread died), the TesterComm thread is gone by the time the hub deletes us, so tearing down here is safe.
    if (started_)
        Teardown();
}

void GpibEngine::InjectDriver(IGpibDriver* driver) { g_injected.store(driver); }
void GpibEngine::OverrideIniPaths(const std::string& generalIni, const std::string& handlerGeneralIni)
{
    g_genIniOverride = generalIni;
    g_hgenIniOverride = handlerGeneralIni;
}
testercomm::TesterEngine* GpibEngine::Create() { return new GpibEngine; }
HWND GpibEngine::HandlerWndToken() { return static_cast<HWND>(g_handlerTok.load()); }
HWND GpibEngine::BridgeWndToken() { return static_cast<HWND>(g_bridgeTok.load()); }
unsigned long GpibEngine::PostFailures() { return g_postFailures.load(); }
unsigned long GpibEngine::HandlerMessages() { return g_handlerMsgs.load(); }

bool GpibEngine::Start(testercomm::SyncMailbox* mailbox)
{
    if (mailbox == 0 || started_)
        return false;
    int expected = 0;
    if (!g_live.compare_exchange_strong(expected, 1))
        return false;   // another GpibEngine is alive (golden: CreateMutex "MyMutexGPIB" refused a 2nd bridge)

    mailbox_ = mailbox;
    started_ = true;
    closed_ = false;
    depth_ = 0;

    IGpibDriver* d = g_injected.load();
    if (d == 0)
    {
        NiGpibDriver* ni = new NiGpibDriver;
        if (ni->Loaded())
        {
            d = ni;
            ownedDriver_ = ni;
        }
        else
        {
            delete ni;   // no NI-488.2 runtime: golden's ibfind fails and it logs "Error Open GPIB0"
        }
    }
    SetGpibDriver(d);

    // golden: every life is a fresh process -- globals at their initialisers, function-local statics re-armed
    // (GpibBridge.h "V906 bridge life").  Must precede the TSerialPoll ctor, which golden runs on fresh globals.
    ResetBridgeGlobals();
    ++g_bridgeLife;
    if (!g_genIniOverride.empty())
        asGeneralPath = g_genIniOverride.c_str();
    if (!g_hgenIniOverride.empty())
        asHGeneralPath = g_hgenIniOverride.c_str();

    // golden FindWindow("TfMain", "HT-9045") -> HMountWnd: the Handler side is "found" while the mailbox exists.
    HMountWnd = reinterpret_cast<HWND>(mailbox);
    g_handlerTok.store(static_cast<void*>(mailbox));

    try
    {
        // golden WinMain order (H9046_32GPIB.cpp): CreateForm(SerialPoll) [OnCreate], CreateForm(fDummyART),
        // CreateForm(fRS232Main), Application->Run [main form OnShow].  fMyPal is golden's design-time template
        // form for TMyDutPanel and has no runtime role here.
        VclCreateForm(SerialPoll);
        SerialPoll->mailbox = mailbox;
        g_bridgeTok.store(static_cast<void*>(SerialPoll));
        SerialPoll->FormCreate(0);
        VclCreateForm(fDummyART);
        VclCreateForm(fRS232Main);
        SerialPoll->FormShow(0);
    }
    catch (...)
    {
        Teardown();
        return false;
    }

    timer1_ = TimerSlot();
    timerTMode_ = TimerSlot();
    timerDummyArt_ = TimerSlot();
    up_.store(!SerialPoll->closeRequested);
    return true;
}

bool GpibEngine::Due(TimerSlot& slot, bool enabled, unsigned interval, unsigned long long nowMs)
{
    if (!enabled)
    {
        slot.wasEnabled = false;
        return false;
    }
    if (!slot.wasEnabled)
    {
        // VCL: setting Enabled=true (re)starts the period
        slot.wasEnabled = true;
        slot.lastMs = nowMs;
        return false;
    }
    if (interval == 0)
        return false;   // VCL: Interval 0 never fires
    if (nowMs - slot.lastMs < interval)
        return false;
    slot.lastMs = nowMs;   // WM_TIMER does not catch up missed periods
    return true;
}

unsigned GpibEngine::RunOnce()
{
    if (SerialPoll == 0 || closed_)
        return kIdleMs;
    TSerialPoll* sp = SerialPoll;

#define GPIB_ENGINE_CLOSE_CHECK()      \
    if (sp->closeRequested)            \
    {                                  \
        DoClose();                     \
        return kIdleMs;                \
    }

    GPIB_ENGINE_CLOSE_CHECK();

    // serial chunks queued by the COM reader threads (CommAMD / CommAMD2 / fRS232Main->CommTester)
    sp->DrainRx();
    GPIB_ENGINE_CLOSE_CHECK();

    // golden TMyThread::Execute / Process (Unit2.cpp:31-37, 40-66): started by FormShow (bEnableThread)
    if (bEnableThread && InitialOK)
    {
        sp->ProcessAddress();
        //AI(W906-GB-PADSEEN) 20261007 (Jerry, J-19 "C"): say it out loud when the address on the CARD is not the
        //  address every screen shows.  golden sets the status bar in the Handler message handler and, in the same
        //  block, also assigns oldGpibAddress (Main.cpp:3315 / V12.13.883 -- the //wei 20150629 line), which makes
        //  ProcessAddress()'s `oldGpibAddress!=GpibAddress` guard false, so ibpad() never runs.  golden survives it
        //  because WriteLastDataFile() writes the new address into general.ini and the NEXT launch of the bridge exe
        //  boots correct; on this laptop that ini is read-only to the user, the write fails silently, and the card
        //  stayed on 10 while everything displayed 1 (measured 1007).  Nothing is corrected here on purpose -- that
        //  would be a deviation from golden, and is Jimmy's call.  This only removes the silence.
        //  Logged once per distinct pair, so a real mismatch does not drown lstRecord.
        if (bGpibMode)
        {
            const int onCard = LastPrimaryAddress();
            if (onCard >= 0 && onCard != LastSet.GpibAddress &&
                (onCard != padSaidCard_ || LastSet.GpibAddress != padSaidWanted_))
            {
                padSaidCard_ = onCard;
                padSaidWanted_ = LastSet.GpibAddress;
                AnsiString w;
                w.sprintf("!! ADDRESS MISMATCH: card is %d, display/recipe says %d -- the Tester calls %d and nobody answers"
                          " (ibpad never ran; see general.ini [SystemSetup] GpibAddress and docs/handoff J-19)",
                          onCard, LastSet.GpibAddress, LastSet.GpibAddress);
                sp->WriteLog(w.c_str());
            }
            else if (onCard >= 0 && onCard == LastSet.GpibAddress && padSaidCard_ >= 0)
            {
                padSaidCard_ = -1;   // agreed again: re-arm, so a later mismatch is reported afresh
                padSaidWanted_ = -1;
            }
        }
        GPIB_ENGINE_CLOSE_CHECK();
        sp->ProcessMessage();
        GPIB_ENGINE_CLOSE_CHECK();
    }

    // golden TTimer components (Main.dfm:1531 Timer1 300 ms, :1599 TimerTMode 10 ms; DummyArt.dfm:583)
    if (sp->Timer1 && Due(timer1_, sp->Timer1->Enabled, sp->Timer1->Interval, NowMs()))
    {
        sp->Timer1Timer(sp->Timer1);
        GPIB_ENGINE_CLOSE_CHECK();
    }
    if (sp->TimerTMode && Due(timerTMode_, sp->TimerTMode->Enabled, sp->TimerTMode->Interval, NowMs()))
    {
        sp->TimerTModeTimer(sp->TimerTMode);
        GPIB_ENGINE_CLOSE_CHECK();
    }
    if (fDummyART && fDummyART->DummyARTTimer1 &&
        Due(timerDummyArt_, fDummyART->DummyARTTimer1->Enabled, fDummyART->DummyARTTimer1->Interval, NowMs()))
    {
        fDummyART->DummyARTTimer1Timer(fDummyART->DummyARTTimer1);
        GPIB_ENGINE_CLOSE_CHECK();
    }

    // P7: page commands (golden: user clicks on the bridge's own form, handled on the form thread)
    {
        const std::vector<std::string> cmds = testercomm::UiChannel::Instance().Take(kUiKey);
        for (size_t i = 0; i < cmds.size(); ++i)
        {
            ApplyUiCommand(cmds[i]);
            GPIB_ENGINE_CLOSE_CHECK();
        }
    }
    {
        const unsigned long long now = NowMs();
        if (now - lastUiMs_ >= kUiPeriodMs)
        {
            lastUiMs_ = now;
            const IGpibDriver* d = GetGpibDriver();
            testercomm::UiChannel::Instance().Publish(kUiKey, BuildUiSnapshot(up_.load(), d ? d->Name() : "none"));
            testercomm::UiChannel::Instance().Publish("gpib.home", BuildHomeFragment(up_.load()));   // AI(W906-TC-SHARED) 20261001 (St02-E): the 首頁 part
        }
    }

#undef GPIB_ENGINE_CLOSE_CHECK
    return kLoopMs;
}

int GpibEngine::OnHandlerMessage(const std::string& payload)
{
    // golden: once Close() ran the bridge window is going away and a late WM_COPYDATA is not processed.
    if (SerialPoll == 0 || closed_ || SerialPoll->closeRequested)
        return 0;
    ++g_handlerMsgs;

    // WM_COPYDATA hands the receiver a private copy that lives for the duration of the message.  Copy the
    // payload into a zero-padded, 8-byte aligned buffer at least sizeof(MV) long, so a short packet can never
    // make golden read past the end of the Handler's bytes.
    const size_t n = payload.size() < sizeof(MV) ? sizeof(MV) : payload.size();
    std::vector<unsigned long long> buf(n / sizeof(unsigned long long) + 1, 0ULL);
    if (!payload.empty())
        std::memcpy(&buf[0], payload.data(), payload.size());

    COPYDATASTRUCT cds;
    cds.dwData = 0;
    cds.cbData = static_cast<DWORD>(payload.size());
    cds.lpData = &buf[0];
    TMessage msg;
    msg.LParam = reinterpret_cast<LPARAM>(&cds);
    msg.WParam = 0;

    // GHandler2Gpib points into `buf`.  A nested Handler message (the bridge sends back to the Handler inside
    // OnMyCopyMsg and the mailbox pumps) repoints it; restore the outer packet afterwards.  Golden left the
    // pointer dangling in both cases; nothing outside OnMyCopyMsg reads it (checked: Main.cpp / RS232.cpp /
    // DummyArt.cpp), so NULL after the outermost call is safe.
    MV* outer = GHandler2Gpib;
    ++depth_;
    try
    {
        SerialPoll->OnMyCopyMsg(msg);
    }
    catch (...)
    {
        --depth_;
        GHandler2Gpib = depth_ > 0 ? outer : 0;
        throw;   // TesterCommHub::EngineSideHandler counts it
    }
    --depth_;
    GHandler2Gpib = depth_ > 0 ? outer : 0;
    return 0;    // golden OnMyCopyMsg does not set msg.Result
}

void GpibEngine::DoClose()
{
    if (closed_ || SerialPoll == 0)
        return;
    closed_ = true;
    up_.store(false);
    // golden Close() -> OnClose = FormClose (Main.dfm:152): log, SaveResult, WriteLastDataFile, WriteGpibString.
    SerialPoll->FormClose(0);
    testercomm::UiChannel::Instance().Publish(kUiKey, BuildUiSnapshot(false, "closed"));
    testercomm::UiChannel::Instance().Publish("gpib.home", BuildHomeFragment(false));   // AI(W906-TC-SHARED) 20261001 (St02-E)
}

void GpibEngine::Stop()
{
    if (!started_)
        return;
    if (!closed_)
        DoClose();   // golden: Handler's CloseGpibProgram -> MSG_CMD_CloseGpib -> Close() -> FormClose
    Teardown();
}

void GpibEngine::Teardown()
{
    up_.store(false);
    // fRS232Main first: its CommTester reader thread queues into SerialPoll.
    delete fRS232Main;
    fRS232Main = 0;
    delete fDummyART;
    fDummyART = 0;
    g_bridgeTok.store(0);
    delete SerialPoll;   // stops CommAMD / CommAMD2
    SerialPoll = 0;
    testercomm::UiChannel::Instance().Publish(kUiKey, "{\"up\":false}");
    g_handlerTok.store(0);
    HMountWnd = 0;
    GHandler2Gpib = 0;
    //AI(W906-GB-ONL) 20261007 (Jerry, J-19): hand the board back BEFORE the driver goes away.  golden has no
    //  equivalent because its bridge is a separate exe -- process exit releases the board.  In-process, skipping
    //  this left gpib0 owned by wb_serve, so the NEXT life's ibfind("gpib0") failed every time (measured 1007:
    //  21 x "Error Open GPIB0", then ProcessAddress() gave up at iErrorCT>10 and stopped logging), i.e. one
    //  Interface switch killed GPIB until wb_serve was restarted.  noncontroller is the global ProcessAddress()
    //  assigned; -1 means ibfind never succeeded, so there is nothing to release.
    if (noncontroller >= 0)
    {
        ibonl(noncontroller, 0);
        noncontroller = -1;
    }
    SetGpibDriver(0);
    delete ownedDriver_;
    ownedDriver_ = 0;
    mailbox_ = 0;
    started_ = false;
    g_live.store(0);
}

// ---------------------------------------------------------------------------
//  TSerialPoll V906 plumbing (declared in GpibBridge.h)
// ---------------------------------------------------------------------------
void TSerialPoll::PostToHandler(COPYDATASTRUCT* pcp)
{
    // golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp): synchronous; the Handler reads
    // pcp->lpData as its GGpib2Handler (VM) packet.  Its return value is not used by golden.
    if (pcp == 0 || pcp->lpData == 0 || mailbox == 0)
    {
        ++g_postFailures;
        return;
    }
    const std::string payload(static_cast<const char*>(pcp->lpData), static_cast<size_t>(pcp->cbData));
    int result = 0;
    const testercomm::SendStatus st =
        mailbox->Send(testercomm::kEngineSide, payload, &result, testercomm::kDefaultSendTimeoutMs);
    if (st != testercomm::kSent)
    {
        // golden: SendMessage to a missing / hung window just returns.  Log sparsely so a detached Handler
        // does not flood the GPIB log.
        const unsigned long n = ++g_postFailures;
        if (n == 1 || n % 100 == 0)
        {
            AnsiString s;
            s.sprintf("GPIB to Handler : mailbox %s (#%lu)", st == testercomm::kTimeout ? "timeout" : "no receiver", n);
            WriteLog(s);
        }
    }
}

void TSerialPoll::RequestClose(const char* reason)
{
    if (closeRequested)
        return;
    closeRequested = true;
    closeReason = reason ? reason : "";
}

void TSerialPoll::UiNotice(AnsiString text)
{
    // golden MessageDlg / ShowMessage: modal on the bridge's own screen.  Here: remembered for the web page (P7)
    // and written to the GPIB log; never blocks the TesterComm thread.
    lastNotice = text;
    WriteLog(AnsiString("[Notice] ") + text);
}

void TSerialPoll::QueueRx(int port, const void* buf, Word len)
{
    if (buf == 0 || len == 0)
        return;
    {
        webbridge::WbGuard g(rxMutex);
        rxQueue.push_back(std::make_pair(port, std::string(static_cast<const char*>(buf), static_cast<size_t>(len))));
    }
    if (mailbox)
        ::SetEvent(mailbox->WakeHandle(testercomm::kEngineSide));   // wake the TesterComm loop now
}

void TSerialPoll::DrainRx()
{
    std::vector<std::pair<int, std::string> > q;
    {
        webbridge::WbGuard g(rxMutex);
        q.swap(rxQueue);
    }
    for (size_t i = 0; i < q.size(); ++i)
    {
        std::string& d = q[i].second;
        const WORD n = static_cast<WORD>(d.size());
        d.push_back('\0');   // golden CommAMDReceiveData strlen()s the buffer and ignores BufferLength
        switch (q[i].first)
        {
        case kRxCommAMD:
            CommAMDReceiveData(CommAMD, &d[0], n);     // Main.dfm:1564 OnReceiveData = CommAMDReceiveData
            break;
        case kRxCommAMD2:
            CommAMDReceiveData(CommAMD2, &d[0], n);    // Main.dfm:1595 (same handler, Sender tells them apart)
            break;
        case kRxAuxTester:
            if (fRS232Main)
                fRS232Main->CommTesterReceiveData(fRS232Main->CommTester, &d[0], n);   // RS232.dfm:43
            break;
        default:
            break;
        }
        if (closeRequested)
            break;   // golden: the form is closing; later chunks are never delivered
    }
}

}  // namespace gpibbridge
