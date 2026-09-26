// ===========================================================================
//  tests/test_testercomm_ipc.cpp -- Tester-comm plan P0: SyncMailbox / TesterCommThread / TesterCommHub.
//  AI(W906-GB-P0) 20260926.
//
//  What is proven (the P0 gate in docs/GPIB_20260926_INTEGRATION_PROPOSAL.md §4):
//    1. start: selecting a registered test type starts ONE engine on its own thread
//    2. synchronous send: Handler -> engine returns the handler's result (SendMessage semantics)
//    3. nesting: engine handler sends back to the Handler, whose handler sends to the engine again -- no deadlock,
//       golden SendMessage order preserved
//    4. timeout: a hung engine handler costs the Handler thread at most the timeout; the late completion is freed
//    5. engine -> Handler: requests are served on the Handler thread by PollHandler()
//    6. an exception in the engine's message handler still answers the sender (-1) and is counted
//    7. an exception in RunOnce() is counted and the loop keeps running
//    8. isolation: every engine call runs on the TesterComm thread, every Handler sink call on the main thread
//    9. switching test type stops and deletes the old engine; an unregistered type leaves none (kNoReceiver)
//   10. shutdown joins the thread
//  No machine code is linked: only TesterComm/*.cpp.
// ===========================================================================
#include "TesterComm/TesterCommHub.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace testercomm;

static int g_fail = 0;
static int g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c)
    {
        ++g_fail;
        std::printf("FAIL line %d: %s\n", line, e);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- shared event log (written from both threads) -------------------------
static webbridge::WbMutex g_logMu;
static std::vector<std::string> g_log;
static void Log(const std::string& s)
{
    webbridge::WbGuard g(g_logMu);
    g_log.push_back(s);
}
static std::vector<std::string> TakeLog()
{
    webbridge::WbGuard g(g_logMu);
    std::vector<std::string> v;
    v.swap(g_log);
    return v;
}

// MinGW.org GCC 6.3 has no std::to_string.
static std::string Num(int v)
{
    char b[16];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}

static DWORD g_mainThread = 0;
static std::atomic<unsigned long> g_lastEngineThread(0);   // thread of the most recently started engine
static std::atomic<int> g_threadViolations(0);
static std::atomic<int> g_engineStops(0);
static std::atomic<int> g_engineDeletes(0);
static std::atomic<bool> g_sendTick(false);
static std::atomic<bool> g_throwInRun(false);
static std::atomic<int> g_ticksSeen(0);


class TestEngine : public TesterEngine
{
public:
    explicit TestEngine(const char* name) : name_(name), mb_(0), up_(false), runs_(0), thread_(0) {}
    ~TestEngine() { ++g_engineDeletes; }
    const char* Name() const { return name_; }
    bool Start(SyncMailbox* mb)
    {
        thread_ = ::GetCurrentThreadId();
        g_lastEngineThread = thread_;
        NoteEngineThread();
        mb_ = mb;
        up_ = true;
        return true;
    }
    unsigned RunOnce()
    {
        NoteEngineThread();
        ++runs_;
        if (g_throwInRun.exchange(false))
            throw 2;
        if (g_sendTick.load())
        {
            int r = 0;
            mb_->Send(kEngineSide, "tick", &r, 1000);
        }
        return 5;
    }
    int OnHandlerMessage(const std::string& p)
    {
        NoteEngineThread();
        Log("E:" + p);
        if (p.compare(0, 5, "echo:") == 0)
            return (int)p.size() - 5;
        if (p == "nest")
        {
            int r = 0;
            SendStatus st = mb_->Send(kEngineSide, "back", &r, 2000);
            Log(st == kSent ? "E:back=" + Num(r) : std::string("E:back-fail"));
            return 7;
        }
        if (p == "inner")
            return 42;
        if (p == "slow")
        {
            ::Sleep(300);
            return 1;
        }
        if (p == "throw")
            throw 1;
        return 0;
    }
    void Stop() { NoteEngineThread(); up_ = false; ++g_engineStops; }
    bool IsUp() const { return up_.load(); }
    int Runs() const { return runs_.load(); }

private:
    // Every call on this engine must come from the one thread that started it, and never from the main thread.
    void NoteEngineThread()
    {
        unsigned long me = ::GetCurrentThreadId();
        if (me != thread_ || me == g_mainThread)
            ++g_threadViolations;
    }

    const char* name_;
    SyncMailbox* mb_;
    std::atomic<bool> up_;
    std::atomic<int> runs_;
    unsigned long thread_;
};

static TesterEngine* MakeGpib() { return new TestEngine("test-gpib"); }
static TesterEngine* MakeRs232() { return new TestEngine("test-rs232"); }

static TesterCommHub* g_hub = 0;

// Handler sink: runs on the main thread only.
static int HandlerSink(const std::string& p, void*)
{
    if (::GetCurrentThreadId() != g_mainThread)
        ++g_threadViolations;
    if (p == "back")
    {
        Log("H:back");
        int r = 0;
        SendStatus st = g_hub->SendToEngine("inner", &r, 2000);   // nested send while the engine is waiting on us
        Log(st == kSent ? "H:inner=" + Num(r) : std::string("H:inner-fail"));
        return 99;
    }
    if (p == "tick")
    {
        ++g_ticksSeen;
        return 0;
    }
    return -5;
}

template <class Pred>
static bool WaitFor(Pred pred, unsigned ms, bool pollHandler)
{
    DWORD start = ::GetTickCount();
    while (::GetTickCount() - start < ms)
    {
        if (pollHandler)
            g_hub->PollHandler();
        if (pred())
            return true;
        ::Sleep(5);
    }
    return pred();
}

int main()
{
    g_mainThread = ::GetCurrentThreadId();
    TesterCommHub hub;
    g_hub = &hub;
    hub.RegisterFactory(kTestTypeGpib, &MakeGpib);
    hub.RegisterFactory(kTestTypeRs232, &MakeRs232);
    hub.SetHandlerSink(&HandlerSink, 0);

    // 1) start
    int r = 0;
    CHECK(hub.SendToEngine("echo:x", &r) == kNoReceiver);   // nothing selected yet
    CHECK(hub.SelectTestType(kTestTypeGpib));
    CHECK(WaitFor([&] { return hub.IsUp(); }, 2000, false));
    CHECK(std::string(hub.EngineName()) == "test-gpib");

    // 2) synchronous send
    CHECK(hub.SendToEngine("echo:abc", &r) == kSent && r == 3);

    // 3) nesting, golden SendMessage order
    TakeLog();
    CHECK(hub.SendToEngine("nest", &r, 3000) == kSent && r == 7);
    {
        std::vector<std::string> v = TakeLog();
        const char* want[] = { "E:nest", "H:back", "E:inner", "H:inner=42", "E:back=99" };
        bool ok = v.size() == 5;
        for (std::size_t i = 0; ok && i < 5; ++i)
            ok = (v[i] == want[i]);
        CHECK(ok);
        if (!ok)
            for (std::size_t i = 0; i < v.size(); ++i)
                std::printf("  log[%u] = %s\n", (unsigned)i, v[i].c_str());
    }

    // 4) timeout + late completion freed
    DWORD t0 = ::GetTickCount();
    CHECK(hub.SendToEngine("slow", &r, 50) == kTimeout);
    CHECK(::GetTickCount() - t0 < 250);   // the Handler thread was released well before the 300 ms handler ended
    CHECK(WaitFor([&] { return hub.Mailbox().Stats().abandoned == 1; }, 2000, false));
    CHECK(hub.SendToEngine("echo:xy", &r) == kSent && r == 2);

    // 5) engine -> Handler, served by PollHandler on this thread
    g_sendTick = true;
    CHECK(WaitFor([&] { return g_ticksSeen.load() > 0; }, 2000, true));
    g_sendTick = false;
    hub.PollHandler();

    // 6) exception in the engine's message handler
    CHECK(hub.SendToEngine("throw", &r) == kSent && r == -1);
    CHECK(hub.HandlerErrors() == 1);

    // 7) exception in RunOnce: counted, loop keeps running
    unsigned long hb = hub.Thread().Heartbeat();
    g_throwInRun = true;
    CHECK(WaitFor([&] { return hub.Thread().EngineErrors() >= 1; }, 2000, false));
    CHECK(WaitFor([&] { return hub.Thread().Heartbeat() > hb + 3; }, 2000, false));
    CHECK(hub.IsUp());

    // 8) isolation
    CHECK(g_threadViolations.load() == 0);

    // 9) switching
    unsigned long firstEngineThread = g_lastEngineThread.load();
    CHECK(hub.SelectTestType(kTestTypeRs232));
    CHECK(g_engineStops.load() == 1 && g_engineDeletes.load() == 1);
    CHECK(WaitFor([&] { return hub.IsUp(); }, 2000, false));
    CHECK(std::string(hub.EngineName()) == "test-rs232");
    CHECK(hub.SendToEngine("echo:abcd", &r) == kSent && r == 4);
    CHECK(g_lastEngineThread.load() != 0 && g_lastEngineThread.load() != g_mainThread);
    (void)firstEngineThread;   // not compared: Windows may reuse the old thread's id once it has exited
    CHECK(!hub.SelectTestType(kTestTypeTcpIp));   // no TCP engine registered in P0
    CHECK(g_engineStops.load() == 2 && g_engineDeletes.load() == 2);
    CHECK(hub.SendToEngine("echo:x", &r) == kNoReceiver);
    CHECK(!hub.IsUp());

    // 10) shutdown
    CHECK(hub.SelectTestType(kTestTypeGpib));
    CHECK(WaitFor([&] { return hub.IsUp(); }, 2000, false));
    hub.Shutdown();
    CHECK(!hub.Thread().Running());
    CHECK(g_engineDeletes.load() == 3);
    CHECK(g_threadViolations.load() == 0);

    std::printf("test_testercomm_ipc: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
