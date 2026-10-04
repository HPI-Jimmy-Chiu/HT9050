// ===========================================================================
//  tests/test_testercomm_gpib.cpp -- Tester-comm plan P1: the GPIB bridge engine (namespace gpibbridge).
//  AI(W906-GB-P1) 20260926.
//
//  DEFAULT part (no disk side effects):
//    1. link: taking GpibEngine::Create pulls every translated Gpib*.cpp into the link, so a missing definition
//       anywhere in the bridge fails this test's build -- the main value of this test while STEVEN-NB3 has no
//       compiler.
//    2. GpibDriver wrappers: no driver -> ERR and ibfind < 0 (golden "Error Open GPIB0"); SimGpibDriver
//       listen / talk / SRQ semantics and ibsta / ibcnt refresh.
//    3. VCL TTimer emulation (GpibEngine::Due): Enabled edge restarts the period, Interval 0 never fires, no
//       catch-up.
//    4. golden Check2Dsum (Main.cpp:956) -- also pins vclcompat SubString(0, n) == BCB6 (index 0 acts as 1).
//    5. engine guards: Start(NULL) refuses; OnHandlerMessage before Start is a no-op.
//
//  FULL part (opt-in: set HT9045_GPIB_FULL_TEST=1).  Runs the real engine on the TesterComm thread with the
//  simulated GPIB driver.  Opt-in because golden writes to fixed paths on purpose (D:\GPIBLOG\Log, D:\gpib9045\system,
//  D:\RS232Log, D:\RS232Standard\System\Setup.ini) -- the two ini paths that are variables (asGeneralPath /
//  asHGeneralPath) are pointed at %TEMP% here, the literals cannot be.
//    6. start -> the bridge "finds" the Handler (Timer1 -> ProcessHMountConnect) and sends MSG_CMD_Version with
//       the bridge version, then MSG_CMD_TesterMode (golden Main.cpp:3121-3122), on the Handler thread.
//    7. MSG_CMD_CloseGpib with bCloseGpib -> the bridge closes itself (IsUp() false) and Shutdown joins.
//    8. a second engine life sends MSG_CMD_Version again (golden relaunches the exe; here the function-local
//       "first time" statics are re-armed per life).
// ===========================================================================
#include "TesterComm/Gpib/GpibEngine.h"
#include "TesterComm/Gpib/GpibBridge.h"
#include "TesterComm/TesterCommHub.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

// ---------------------------------------------------------------------------
static void TestLink()
{
    testercomm::TesterEngineFactory f = &gpibbridge::GpibEngine::Create;
    CHECK(f != 0);
    testercomm::TesterEngine* e = f();
    CHECK(e != 0);
    CHECK(std::strcmp(e->Name(), "gpib") == 0);
    CHECK(!e->IsUp());
    delete e;   // never started: nothing to tear down
}

static void TestDriverWrappers()
{
    using namespace gpibbridge;
    SetGpibDriver(0);
    CHECK(ibfind("GPIB0") < 0);
    CHECK((ibsta & ERR) != 0);
    char b[8];
    ibrd(0, b, sizeof(b));
    CHECK((ibsta & ERR) != 0);

    SimGpibDriver sim;
    SetGpibDriver(&sim);
    CHECK(ibfind("GPIB0") == 0);
    CHECK((ibsta & ERR) == 0);
    ibpad(0, 5);
    CHECK(sim.PrimaryAddress() == 5);

    ibwait(0, 0);
    CHECK((ibsta & LACS) == 0);
    sim.TesterWrite("FULLSITES?");
    ibwait(0, 0);
    CHECK((ibsta & LACS) != 0);
    char buf[64];
    std::memset(buf, 0, sizeof(buf));
    ibrd(0, buf, sizeof(buf) - 1);
    CHECK(ibcnt == 10);
    CHECK(std::strcmp(buf, "FULLSITES?") == 0);
    CHECK((ibsta & END) != 0);
    ibwait(0, 0);
    CHECK((ibsta & LACS) == 0);

    // not addressed to talk: the write fails like a real device write with no listener
    const char* ans = "Fullsites 00000003";
    ibwrt(0, ans, (long)std::strlen(ans));
    CHECK((ibsta & ERR) != 0);
    CHECK(sim.TakeBridgeWrites().empty());
    sim.SetTalkAddressed(true);
    ibwait(0, 0);
    CHECK((ibsta & TACS) != 0);
    ibwrt(0, ans, (long)std::strlen(ans));
    CHECK((ibsta & ERR) == 0);
    CHECK(ibcnt == (long)std::strlen(ans));
    std::vector<std::string> w = sim.TakeBridgeWrites();
    CHECK(w.size() == 1 && w[0] == ans);

    ibrsv(0, 0x41);
    std::vector<int> srq = sim.TakeSrqBytes();
    CHECK(srq.size() == 1 && srq[0] == 0x41);

    sim.FindFails = true;
    CHECK(ibfind("GPIB0") < 0);
    CHECK((ibsta & ERR) != 0);
    SetGpibDriver(0);
}

static void TestTimerDue()
{
    typedef gpibbridge::GpibEngine E;
    E::TimerSlot t;
    CHECK(!E::Due(t, false, 300, 0));
    CHECK(!E::Due(t, true, 300, 1000));     // enabled edge: period starts now
    CHECK(!E::Due(t, true, 300, 1299));
    CHECK(E::Due(t, true, 300, 1300));
    CHECK(!E::Due(t, true, 300, 1301));
    CHECK(E::Due(t, true, 300, 2500));      // late: fires once ...
    CHECK(!E::Due(t, true, 300, 2600));     // ... and does not catch up the missed periods
    CHECK(!E::Due(t, false, 300, 2900));    // disabled
    CHECK(!E::Due(t, true, 300, 3000));     // re-enabled: period restarts
    CHECK(!E::Due(t, true, 300, 3299));
    CHECK(E::Due(t, true, 300, 3300));
    E::TimerSlot z;
    CHECK(!E::Due(z, true, 0, 0));
    CHECK(!E::Due(z, true, 0, 100000));     // Interval 0 never fires (VCL)
}

static void TestCheck2Dsum()
{
    // golden: text before '#', then "%03d%s%03d" of (length, text, sum of chars % 128)
    CHECK(gpibbridge::Check2Dsum("AB#") == "002AB003");            // 65+66=131 -> 3
    CHECK(gpibbridge::Check2Dsum("A1B2#tail") == "004A1B2102");    // 65+49+66+50=230 -> 102
    CHECK(gpibbridge::Check2Dsum("NOHASH") == "000000");           // Pos("#")==0 -> SubString(0,-1) == ""
}

static void TestEngineGuards()
{
    gpibbridge::GpibEngine e;
    CHECK(!e.Start(0));
    CHECK(e.OnHandlerMessage(std::string("x")) == 0);
    CHECK(!e.IsUp());
    e.Stop();   // never started: no-op
}

// ---------------------------------------------------------------------------
//  FULL part
// ---------------------------------------------------------------------------
struct Recorder
{
    webbridge::WbMutex mu;
    std::vector<VM> packets;
    DWORD threadId;
    bool wrongThread;
    Recorder() : threadId(0), wrongThread(false) {}
};

static int RecordSink(const std::string& payload, void* ctx)
{
    Recorder* r = static_cast<Recorder*>(ctx);
    VM vm;
    std::memset(&vm, 0, sizeof(vm));
    std::memcpy(&vm, payload.data(), payload.size() < sizeof(vm) ? payload.size() : sizeof(vm));
    webbridge::WbGuard g(r->mu);
    if (::GetCurrentThreadId() != r->threadId)
        r->wrongThread = true;
    r->packets.push_back(vm);
    return 0;
}

static int FindCommand(Recorder& r, unsigned cmd, size_t from = 0)
{
    webbridge::WbGuard g(r.mu);
    for (size_t i = from; i < r.packets.size(); ++i)
        if (r.packets[i].iCommand == cmd)
            return (int)i;
    return -1;
}

static bool PumpUntil(testercomm::TesterCommHub& hub, Recorder& r, unsigned cmd, size_t from, unsigned ms)
{
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < ms)
    {
        hub.PollHandler();
        if (FindCommand(r, cmd, from) >= 0)
            return true;
        ::Sleep(5);
    }
    return false;
}

static void WriteText(const std::string& path, const char* text)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (f)
    {
        std::fputs(text, f);
        std::fclose(f);
    }
}

static void OneLife(testercomm::TesterCommHub& hub, Recorder& r, int life)
{
    const size_t base = r.packets.size();
    CHECK(hub.SelectTestType(testercomm::kTestTypeGpib));

    // Timer1 (300 ms) -> ProcessHMountConnect once a second -> SleepEx(1000) -> Version, TesterMode
    const bool gotVersion = PumpUntil(hub, r, MSG_CMD_Version, base, 8000);
    CHECK(gotVersion);
    if (!gotVersion)
        std::printf("  life %d: no MSG_CMD_Version (packets %u)\n", life, (unsigned)(r.packets.size() - base));
    const int iv = FindCommand(r, MSG_CMD_Version, base);
    if (iv >= 0)
    {
        webbridge::WbGuard g(r.mu);
        const VM& v = r.packets[(size_t)iv];
        CHECK(std::strcmp(v.cReturn, "V12.13.905.0") == 0);   // golden: GPIBVersion=VerInfo().GetFileVersion()
        if (std::strcmp(v.cReturn, "V12.13.905.0") != 0)
            std::printf("  life %d: version '%s'\n", life, v.cReturn);
    }
    CHECK(PumpUntil(hub, r, MSG_CMD_TesterMode, base, 2000));
    CHECK(hub.IsUp());
    CHECK(gpibbridge::GpibEngine::BridgeWndToken() != 0);
    CHECK(gpibbridge::GpibEngine::HandlerWndToken() != 0);

    // Handler -> bridge: CloseGpib (golden CloseGpibProgram)
    MV mv;
    std::memset(&mv, 0, sizeof(mv));
    mv.iSendCommand = MSG_CMD_CloseGpib;
    mv.bCloseGpib = true;
    mv.HandlerHwnd = gpibbridge::GpibEngine::HandlerWndToken();
    mv.GpibHwnd = gpibbridge::GpibEngine::BridgeWndToken();
    int result = -1;
    CHECK(hub.SendToEngine(std::string(reinterpret_cast<const char*>(&mv), sizeof(mv)), &result) == testercomm::kSent);
    const DWORD t0 = ::GetTickCount();
    while (hub.IsUp() && ::GetTickCount() - t0 < 2000)
    {
        hub.PollHandler();
        ::Sleep(5);
    }
    CHECK(!hub.IsUp());

    hub.Shutdown();
    CHECK(gpibbridge::GpibEngine::BridgeWndToken() == 0);
}

static void TestFullLifecycle()
{
    const char* on = std::getenv("HT9045_GPIB_FULL_TEST");
    if (!on || std::strcmp(on, "1") != 0)
    {
        std::printf("full lifecycle: skipped (set HT9045_GPIB_FULL_TEST=1; writes D:\\GPIBLOG like golden)\n");
        return;
    }
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    std::string dir = std::string(tmp) + "ht9045_gpib_ctest";
    ::CreateDirectoryA(dir.c_str(), 0);
    const std::string gen = dir + "\\general.ini";
    const std::string hgen = dir + "\\Gerneral.ini";
    WriteText(gen, "[Version]\r\nModel=9045GPIB\r\n");
    // CUSTOMER_CODE 910 (CC_SPIL_SHINCHU) is not in golden RS232.cpp:112-119, so the aux RS232 line defaults to OFF
    // and no real COM port is opened (code 0 = CC_HONPREC_QC would default it ON and open Setup.ini's COM port).
    WriteText(hgen, "[Version]\r\nModel=HT-9045\r\n[System]\r\nCUSTOMER_CODE=910\r\n");
    gpibbridge::GpibEngine::OverrideIniPaths(gen, hgen);   // applied after the per-life ResetBridgeGlobals()

    gpibbridge::SimGpibDriver sim;
    gpibbridge::GpibEngine::InjectDriver(&sim);

    Recorder r;
    r.threadId = ::GetCurrentThreadId();
    testercomm::TesterCommHub hub;
    hub.RegisterFactory(testercomm::kTestTypeGpib, &gpibbridge::GpibEngine::Create);
    hub.SetHandlerSink(&RecordSink, &r);

    OneLife(hub, r, 1);
    OneLife(hub, r, 2);
    CHECK(!r.wrongThread);   // every bridge -> Handler packet was served on this (Handler) thread
    CHECK(hub.Thread().EngineErrors() == 0);
    CHECK(hub.HandlerErrors() == 0);         // exceptions out of OnMyCopyMsg
    if (hub.Thread().EngineErrors() != 0)
        std::printf("  TesterComm thread errors: %lu\n", hub.Thread().EngineErrors());

    gpibbridge::GpibEngine::InjectDriver(0);
    gpibbridge::GpibEngine::OverrideIniPaths(std::string(), std::string());
}

int main()
{
    TestLink();
    TestDriverWrappers();
    TestTimerDue();
    TestCheck2Dsum();
    TestEngineGuards();
    TestFullLifecycle();
    std::printf("test_testercomm_gpib: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
