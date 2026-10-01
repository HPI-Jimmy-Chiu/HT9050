// ===========================================================================
//  TesterComm/Rs232/Rs232Engine.cpp -- see Rs232Engine.h.  AI(W906-GB-P4) 20260926.
//
//  Also defines the V906 plumbing members of TfRS232Main declared in Rs232Bridge.h (PostToHandler, RequestClose,
//  UiNotice, QueueRx, DrainRx): the engine's side of the contract, not golden text.
// ===========================================================================
#include "TesterComm/Rs232/Rs232Engine.h"
#include "TesterComm/Rs232/Rs232Bridge.h"
#include "TesterComm/SyncMailbox.h"
#include "TesterComm/TesterCommHub.h"   // kDefaultSendTimeoutMs
#include "TesterComm/UiChannel.h"
#include "TesterComm/Rs232/Rs232UiSnapshot.h"

#include <cstring>
#include <new>
#include <string>
#include <utility>
#include <vector>

namespace rs232std {

namespace {

// Golden's globals are singletons (one program per process: CreateMutex "MyRS232Standard"); one live engine.
std::atomic<int> g_live(0);
std::atomic<void*> g_handlerTok(0);
std::atomic<void*> g_bridgeTok(0);
std::atomic<unsigned long> g_postFailures(0);
std::atomic<unsigned long> g_handlerMsgs(0);
std::string g_setupIniOverride;   // written before the hub starts the thread, read in Start (thread start orders it)
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

// Golden RS232Standard has no worker thread: everything runs from the VCL message loop (Timer1 300 ms, TComm and
// socket events).  10 ms keeps Timer1 within 10 ms of its period; a queued receive wakes the loop at once.
const unsigned kLoopMs = 10;
const unsigned kIdleMs = 50;
const unsigned kUiPeriodMs = 200;
const char* const kUiKey = "rs232";

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
//  Rs232Engine
// ---------------------------------------------------------------------------
Rs232Engine::Rs232Engine() : mailbox_(0), up_(false), started_(false), closed_(false), depth_(0), lastUiMs_(0) {}

Rs232Engine::~Rs232Engine()
{
    if (started_)
        Teardown();   // Stop() never ran (thread died); the TesterComm thread is gone, so this is safe
}

testercomm::TesterEngine* Rs232Engine::Create() { return new Rs232Engine; }
void Rs232Engine::OverrideIniPaths(const std::string& setupIni, const std::string& handlerGeneralIni)
{
    g_setupIniOverride = setupIni;
    g_hgenIniOverride = handlerGeneralIni;
}
HWND Rs232Engine::HandlerWndToken() { return static_cast<HWND>(g_handlerTok.load()); }
HWND Rs232Engine::BridgeWndToken() { return static_cast<HWND>(g_bridgeTok.load()); }
unsigned long Rs232Engine::PostFailures() { return g_postFailures.load(); }
unsigned long Rs232Engine::HandlerMessages() { return g_handlerMsgs.load(); }

bool Rs232Engine::Due(TimerSlot& slot, bool enabled, unsigned interval, unsigned long long nowMs)
{
    if (!enabled)
    {
        slot.wasEnabled = false;
        return false;
    }
    if (!slot.wasEnabled)
    {
        slot.wasEnabled = true;   // VCL: Enabled=true (re)starts the period
        slot.lastMs = nowMs;
        return false;
    }
    if (interval == 0)
        return false;             // VCL: Interval 0 never fires
    if (nowMs - slot.lastMs < interval)
        return false;
    slot.lastMs = nowMs;          // WM_TIMER does not catch up missed periods
    return true;
}

bool Rs232Engine::Start(testercomm::SyncMailbox* mailbox)
{
    if (mailbox == 0 || started_)
        return false;
    int expected = 0;
    if (!g_live.compare_exchange_strong(expected, 1))
        return false;   // golden: CreateMutex "MyRS232Standard" refused a second instance

    mailbox_ = mailbox;
    started_ = true;
    closed_ = false;
    depth_ = 0;

    // golden: every launch is a fresh process (Rs232Bridge.h "V906 program life")
    ResetRs232Globals();
    ++g_rs232Life;
    if (!g_setupIniOverride.empty())
        IniFileName = g_setupIniOverride.c_str();
    if (!g_hgenIniOverride.empty())
        asHGeneralPath = g_hgenIniOverride.c_str();

    // golden FindWindow("TfMain", ...) -> HMountWnd: the Handler side is "found" while the mailbox exists.
    HMountWnd = reinterpret_cast<HWND>(mailbox);
    g_handlerTok.store(static_cast<void*>(mailbox));

    try
    {
        // golden WinMain (RS232Standard.cpp): CreateForm(fRS232Main) [OnCreate], CreateForm(fMyPal), Run [OnShow].
        VclCreateForm(fRS232Main);
        fRS232Main->mailbox = mailbox;
        g_bridgeTok.store(static_cast<void*>(fRS232Main));
        fRS232Main->FormCreate(0);
        fRS232Main->FormShow(0);
    }
    catch (...)
    {
        Teardown();
        return false;
    }

    timer1_ = TimerSlot();
    up_.store(!fRS232Main->closeRequested);
    return true;
}

unsigned Rs232Engine::RunOnce()
{
    if (fRS232Main == 0 || closed_)
        return kIdleMs;
    TfRS232Main* f = fRS232Main;

#define RS232_ENGINE_CLOSE_CHECK()     \
    if (f->closeRequested)             \
    {                                  \
        DoClose();                     \
        return kIdleMs;                \
    }

    RS232_ENGINE_CLOSE_CHECK();

    f->DrainRx();   // CommTester / CommTester_TTL / CommTester_TTL_2 / uServer chunks, in arrival order
    RS232_ENGINE_CLOSE_CHECK();

    if (f->Timer1 && Due(timer1_, f->Timer1->Enabled, f->Timer1->Interval, NowMs()))   // MainForm.dfm Timer1 300 ms
    {
        f->Timer1Timer(f->Timer1);
        RS232_ENGINE_CLOSE_CHECK();
    }

    // P7: page commands, then a snapshot every kUiPeriodMs
    {
        const std::vector<std::string> cmds = testercomm::UiChannel::Instance().Take(kUiKey);
        for (size_t i = 0; i < cmds.size(); ++i)
        {
            ApplyUiCommand(cmds[i]);
            RS232_ENGINE_CLOSE_CHECK();
        }
        const unsigned long long now = NowMs();
        if (now - lastUiMs_ >= kUiPeriodMs)
        {
            lastUiMs_ = now;
            testercomm::UiChannel::Instance().Publish(kUiKey, BuildUiSnapshot(up_.load()));
        }
    }

#undef RS232_ENGINE_CLOSE_CHECK
    return kLoopMs;
}

int Rs232Engine::OnHandlerMessage(const std::string& payload)
{
    if (fRS232Main == 0 || closed_ || fRS232Main->closeRequested)
        return 0;   // golden: the window is going away
    ++g_handlerMsgs;

    // WM_COPYDATA hands the receiver a private copy: zero-padded, 8-byte aligned, at least sizeof(MV).
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

    // GHandler2Gpib points into `buf`; restore the outer packet after a nested message, NULL after the outermost
    // (golden left it dangling; nothing outside OnMyCopyMsg reads it: checked golden MainForm.cpp 20260926).
    MV* outer = GHandler2Gpib;
    ++depth_;
    try
    {
        fRS232Main->OnMyCopyMsg(msg);
    }
    catch (...)
    {
        --depth_;
        GHandler2Gpib = depth_ > 0 ? outer : 0;
        throw;   // TesterCommHub::EngineSideHandler counts it
    }
    --depth_;
    GHandler2Gpib = depth_ > 0 ? outer : 0;
    return 0;
}

void Rs232Engine::DoClose()
{
    if (closed_ || fRS232Main == 0)
        return;
    closed_ = true;
    up_.store(false);
    fRS232Main->FormClose(0);   // golden Close() -> OnClose
    testercomm::UiChannel::Instance().Publish(kUiKey, BuildUiSnapshot(false));
}

void Rs232Engine::Stop()
{
    if (!started_)
        return;
    if (!closed_)
        DoClose();   // golden: Handler's CloseGpibProgram -> MSG_CMD_CloseGpib -> Close()
    Teardown();
}

void Rs232Engine::Teardown()
{
    up_.store(false);
    if (fRS232Main)
    {
        try
        {
            fRS232Main->FormDestroy(0);   // golden OnDestroy after OnClose (MainForm.dfm)
        }
        catch (...)
        {
        }
    }
    g_bridgeTok.store(0);
    delete fRS232Main;   // stops the three TComm reader threads and uServer first (Rs232Ui.cpp dtor)
    fRS232Main = 0;
    testercomm::UiChannel::Instance().Publish(kUiKey, "{\"up\":false}");
    g_handlerTok.store(0);
    HMountWnd = 0;
    GHandler2Gpib = 0;
    mailbox_ = 0;
    started_ = false;
    g_live.store(0);
}

// ---------------------------------------------------------------------------
//  TfRS232Main V906 plumbing (declared in Rs232Bridge.h)
// ---------------------------------------------------------------------------
void TfRS232Main::PostToHandler(COPYDATASTRUCT* pcp)
{
    // golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp): synchronous.
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
        const unsigned long n = ++g_postFailures;
        if (n == 1 || n % 100 == 0)
            ShowCommData("[RS232 ==> Handler]", "mailbox",
                         AnsiString(st == testercomm::kTimeout ? "timeout #" : "no receiver #") + AnsiString((int)n));
    }
}

void TfRS232Main::RequestClose(const char* reason)
{
    if (closeRequested)
        return;
    closeRequested = true;
    closeReason = reason ? reason : "";
}

void TfRS232Main::UiNotice(AnsiString text)
{
    lastNotice = text;
    ShowCommData("[Notice]", text);
}

void TfRS232Main::QueueRx(int port, const void* buf, Word len)
{
    if (buf == 0 || len == 0)
        return;
    {
        webbridge::WbGuard g(rxMutex);
        rxQueue.push_back(std::make_pair(port, std::string(static_cast<const char*>(buf), static_cast<size_t>(len))));
    }
    if (mailbox)
        ::SetEvent(mailbox->WakeHandle(testercomm::kEngineSide));
}

void TfRS232Main::DrainRx()
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
        d.push_back('\0');   // golden handlers may strlen() the buffer
        switch (q[i].first)
        {
        case kRxTester:
            CommTesterReceiveData(CommTester, &d[0], n);                 // MainForm.dfm CommTester OnReceiveData
            break;
        case kRxTtl1:
            CommTester_TTLReceiveData(CommTester_TTL, &d[0], n);         // CommTester_TTL
            break;
        case kRxTtl2:
            CommTester_TTL_2ReceiveData(CommTester_TTL_2, &d[0], n);     // CommTester_TTL_2
            break;
        case kRxTcp:
            ReceiveData_TCPIP(&d[0], static_cast<int>(n));               // golden uServer->SetReceiveFunc(ReceiveData_TCPIP)
            break;
        default:
            break;
        }
        if (closeRequested)
            break;
    }
}

}  // namespace rs232std
