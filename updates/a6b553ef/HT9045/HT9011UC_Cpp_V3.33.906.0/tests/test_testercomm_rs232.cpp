// ===========================================================================
//  tests/test_testercomm_rs232.cpp -- Tester-comm plan P4: the RS232Standard engine (namespace rs232std).
//  AI(W906-GB-P4) 20260926.
//
//  DEFAULT part (no disk side effects):
//    1. link: taking Rs232Engine::Create pulls every translated Rs232*.cpp into the link (a missing definition
//       anywhere fails this test's build).
//    2. golden crc_chk (cmydef.cpp:21) = CRC-16/MODBUS: the standard check value of "123456789" is 0x4B37
//       (CRC1 = high byte, CRC2 = low byte, as golden returns them).  The TTL board frames use it
//       (rs232-ttl-communication skill: "CRC16 Modbus, from SOF to the end of DATA").
//    3. golden MyDeCodeASCII: control codes print as [STX] / [ENQ] ..., printable ASCII as itself.
//    4. VCL TTimer emulation (Rs232Engine::Due).
//    5. engine guards.
//  FULL part (opt-in: HT9045_RS232_FULL_TEST=1): golden writes D:\RS232Log and opens the COM ports named in
//  Setup.ini; the two ini path variables go to %TEMP%, the COM names point at ports that do not exist.
//    6. start -> the program "finds" the Handler (Timer1 -> ProcessHandlerConnect) and sends
//       MSG_CMD_AskArmTestMode then MSG_CMD_Version carrying 12.13.902.0 (golden MainForm.cpp:627-633).
//    7. MSG_CMD_CloseGpib with bCloseGpib -> the program closes itself (release build, see Rs232Bridge.h).
//    8. a second life sends MSG_CMD_Version again (re-armed statics / reset globals).
// ===========================================================================
#include "TesterComm/Rs232/Rs232Engine.h"
#include "TesterComm/Rs232/Rs232Bridge.h"
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

static void TestLink()
{
    testercomm::TesterEngineFactory f = &rs232std::Rs232Engine::Create;
    CHECK(f != 0);
    testercomm::TesterEngine* e = f();
    CHECK(e != 0);
    CHECK(std::strcmp(e->Name(), "rs232std") == 0);
    CHECK(!e->IsUp());
    delete e;
}

static void TestCrc()
{
    unsigned char data[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    char c1 = 0, c2 = 0;
    const unsigned int crc = rs232std::crc_chk(data, (unsigned char)sizeof(data), c1, c2);
    CHECK(crc == 0x4B37);
    CHECK((unsigned char)c1 == 0x4B);
    CHECK((unsigned char)c2 == 0x37);
    if (crc != 0x4B37)
        std::printf("  crc_chk(\"123456789\") = 0x%04X\n", crc);

    unsigned char one[] = { 0x01 };
    c1 = c2 = 0;
    // CRC-16/MODBUS of {0x01} = 0x807E
    CHECK(rs232std::crc_chk(one, 1, c1, c2) == 0x807E);
}

static void TestDeCodeAscii()
{
    CHECK(rs232std::MyDeCodeASCII(2) == "[STX]");
    CHECK(rs232std::MyDeCodeASCII(3) == "[ETX]");
    CHECK(rs232std::MyDeCodeASCII(5) == "[ENQ]");
    CHECK(rs232std::MyDeCodeASCII(6) == "[ACK]");
    CHECK(rs232std::MyDeCodeASCII(32) == " ");
    CHECK(rs232std::MyDeCodeASCII(33) == "!");
}

static void TestTimerDue()
{
    typedef rs232std::Rs232Engine E;
    E::TimerSlot t;
    CHECK(!E::Due(t, true, 300, 1000));
    CHECK(!E::Due(t, true, 300, 1299));
    CHECK(E::Due(t, true, 300, 1300));
    CHECK(!E::Due(t, false, 300, 1700));
    CHECK(!E::Due(t, true, 300, 1800));
    CHECK(E::Due(t, true, 300, 2100));
}

static void TestEngineGuards()
{
    rs232std::Rs232Engine e;
    CHECK(!e.Start(0));
    CHECK(e.OnHandlerMessage(std::string("x")) == 0);
    CHECK(!e.IsUp());
    e.Stop();
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

static int FindCommand(Recorder& r, unsigned cmd, size_t from)
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
    CHECK(hub.SelectTestType(testercomm::kTestTypeRs232));

    CHECK(PumpUntil(hub, r, MSG_CMD_AskArmTestMode, base, 8000));
    const bool gotVersion = PumpUntil(hub, r, MSG_CMD_Version, base, 3000);
    CHECK(gotVersion);
    const int iv = FindCommand(r, MSG_CMD_Version, base);
    if (iv >= 0)
    {
        webbridge::WbGuard g(r.mu);
        const VM& v = r.packets[(size_t)iv];
        CHECK(std::strstr(v.cReturn, "12.13.902.0") != 0);
        if (std::strstr(v.cReturn, "12.13.902.0") == 0)
            std::printf("  life %d: version '%s'\n", life, v.cReturn);
    }
    CHECK(hub.IsUp());

    MV mv;
    std::memset(&mv, 0, sizeof(mv));
    mv.iSendCommand = MSG_CMD_CloseGpib;
    mv.bCloseGpib = true;
    mv.HandlerHwnd = rs232std::Rs232Engine::HandlerWndToken();
    mv.GpibHwnd = rs232std::Rs232Engine::BridgeWndToken();
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
    CHECK(rs232std::Rs232Engine::BridgeWndToken() == 0);
}

static void TestFullLifecycle()
{
    const char* on = std::getenv("HT9045_RS232_FULL_TEST");
    if (!on || std::strcmp(on, "1") != 0)
    {
        std::printf("full lifecycle: skipped (set HT9045_RS232_FULL_TEST=1; writes D:\\RS232Log like golden)\n");
        return;
    }
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    std::string dir = std::string(tmp) + "ht9045_rs232_ctest";
    ::CreateDirectoryA(dir.c_str(), 0);
    const std::string setup = dir + "\\Setup.ini";
    const std::string hgen = dir + "\\Gerneral.ini";
    // Standard RS232 mode; COM ports that do not exist on any PC
    // keys as golden LoadSetupData / LoadSetupData_TTL read them (MainForm.cpp:2541-2562)
    WriteText(setup, "[SystemSetup]\r\niTesterMode=0\r\n[COMPort]\r\nCommName=COM250\r\n"
                     "[COMPort_TTL]\r\nCommName=COM251\r\n[COMPort_TTL_2]\r\nCommName=COM252\r\n");
    WriteText(hgen, "[Version]\r\nModel=HT-9045\r\n[System]\r\nCUSTOMER_CODE=910\r\nTTL_CARD_TYPE=0\r\n");
    rs232std::Rs232Engine::OverrideIniPaths(setup, hgen);

    Recorder r;
    r.threadId = ::GetCurrentThreadId();
    testercomm::TesterCommHub hub;
    hub.RegisterFactory(testercomm::kTestTypeRs232, &rs232std::Rs232Engine::Create);
    hub.RegisterFactory(testercomm::kTestTypeTtl, &rs232std::Rs232Engine::Create);
    hub.SetHandlerSink(&RecordSink, &r);

    OneLife(hub, r, 1);
    OneLife(hub, r, 2);
    CHECK(!r.wrongThread);
    CHECK(hub.Thread().EngineErrors() == 0);
    CHECK(hub.HandlerErrors() == 0);

    rs232std::Rs232Engine::OverrideIniPaths(std::string(), std::string());
}

int main()
{
    TestLink();
    TestCrc();
    TestDeCodeAscii();
    TestTimerDue();
    TestEngineGuards();
    TestFullLifecycle();
    std::printf("test_testercomm_rs232: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
