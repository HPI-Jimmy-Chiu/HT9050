// =============================================================================
//  tests/test_tcp_tester_real.cpp -- H-008: the TCP tester socket in REAL, POLLED mode.  AI(W906-H008) 20261001 (St02-E).
//  Suite: TesterComm_TcpTesterReal (both configs).
//
//  CONTAINMENT: every socket this test opens is its own, on 127.0.0.1 with an ephemeral port (bind port 0).  It never
//  uses 6000 / 7016 / 7017 or the TesterIF address, never reads a TesterIF file, never calls W906_TesterCommInit, and
//  points asTestTCPIPLogPath at a scratch folder next to the exe first (test_testertcp_socket.cpp:21-29).  Before every
//  TesterTCPSocket_TimerTCPIPConnectTimer() call it CHECKs that the address is still 127.0.0.1 (E2 review m3).
//    1. vclcompat TClientSocket POLLED (vclcompat/ClientSocket.cpp end of file): Open() returns with Active false and
//       no event; Poll() -> OnConnect; send / receive; the peer closes -> OnDisconnect, Active false, a second Close()
//       fires nothing; a refused connect (target = a bound, NOT listening 127.0.0.1 socket, held open: an RST, and no
//       one else can take the port) -> OnError(eeConnect) from Poll(), never from Open(), within 10 s (Windows retries
//       the SYN after an RST); Close() while connecting fires nothing; a plain Sim socket behaves as before.
//    2. TcpPump end to end (W906_TcpPumpInit(true): REAL polled; golden TfTesterTCP through Interface/TesterTCP_Socket):
//       TCP_IP_MODE + ON_LINE + InitialOK -> the golden timer opens (Count1 starts at 20, golden TesterTCP.cpp:194) ->
//       ON-LINE, bConnectOK; SOT "TEST 1;" reaches the server with "\r\n"; a reply reaches sTCPIPRecevieData; the server
//       closes -> OFF-LINE; a refused target -> ERROR, bTCPError, and the next timer shows "TCP/IP error for tester!"
//       once (no host hook in a ctest: ShowMyMessage only records, canary_support.cpp).  Outcomes, not call counts
//       (W906_TcpPumpTick fires the connect timer itself every 1000 ms too).
//    3. The policy W906_TcpTesterRealSocket(): SIM build false; SHIP true, false with HT9045_TCPCMD_SIM=1.
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h>
#include "MachineType.h"                  // SOFT_SIMULTE
#include "Interface/TesterTCP_Socket.h"   // TesterTCPSocket_*, TesterTCPSocket, TestIF_File, LastSet, InitialOK, fLotInfo
#include "TesterComm/Tcp/TcpPump.h"       // W906_TcpPumpInit / Tick, W906_TcpTesterRealSocket
#include "vclcompat/ClientSocket.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)   // MinGW.org strict mode: no _putenv prototype (test_agv_e84.cpp:143-162)
extern "C" int _putenv(const char*);
#  define HT9045_TEST_PUTENV _putenv
#else
#  define HT9045_TEST_PUTENV putenv
#endif

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- test-owned loopback sockets -------------------------------------------------------------------------
static SOCKET Bound(bool listening, int* port)   // 127.0.0.1:0; listening or only bound
{
    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in a;
    std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = ::inet_addr("127.0.0.1");
    a.sin_port = 0;
    if (s == INVALID_SOCKET || ::bind(s, reinterpret_cast<sockaddr*>(&a), sizeof(a)) != 0)
        return INVALID_SOCKET;
    if (listening && ::listen(s, 4) != 0)
        return INVALID_SOCKET;
    int len = sizeof(a);
    ::getsockname(s, reinterpret_cast<sockaddr*>(&a), &len);
    *port = static_cast<int>(::ntohs(a.sin_port));
    return s;
}
static SOCKET AcceptNow(SOCKET l)                // non-blocking accept (INVALID_SOCKET when nothing waits)
{
    fd_set r;
    FD_ZERO(&r);
    FD_SET(l, &r);
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (::select(0, &r, 0, 0, &tv) <= 0)
        return INVALID_SOCKET;
    return ::accept(l, 0, 0);
}
static std::string RecvWait(SOCKET s, DWORD ms)  // what arrives on a test-owned socket within ms
{
    std::string got;
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < ms)
    {
        fd_set r;
        FD_ZERO(&r);
        FD_SET(s, &r);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 20000;
        if (::select(0, &r, 0, 0, &tv) > 0)
        {
            char b[256];
            const int n = ::recv(s, b, sizeof(b), 0);
            if (n <= 0)
                break;
            got.append(b, static_cast<size_t>(n));
            if (got.find('\n') != std::string::npos)
                break;
        }
    }
    return got;
}

// ---- section 1 event counters ----------------------------------------------------------------------------
struct Ev { int connect, disconnect, error, read, lastErr; TErrorEvent lastKind; };
static void Hook(TClientSocket& c, Ev& e)
{
    e.connect = e.disconnect = e.error = e.read = e.lastErr = 0;
    e.lastKind = eeGeneral;
    c.OnConnect    = [&e](TObject*, TCustomWinSocket*) { ++e.connect; };
    c.OnDisconnect = [&e](TObject*, TCustomWinSocket*) { ++e.disconnect; };
    c.OnRead       = [&e](TObject*, TCustomWinSocket*) { ++e.read; };
    c.OnError      = [&e](TObject*, TCustomWinSocket*, TErrorEvent k, int& code) { ++e.error; e.lastErr = code; e.lastKind = k; code = 0; };
}
template <class Pred> static bool PollUntil(TClientSocket& c, DWORD ms, Pred done)
{
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < ms)
    {
        c.Poll();
        if (done())
            return true;
        ::Sleep(10);
    }
    return done();
}

static void Section1()
{
    std::printf(" 1. vclcompat TClientSocket POLLED\n");
    int port = 0;
    SOCKET l = Bound(true, &port);
    CHECK(l != INVALID_SOCKET && port > 0);
    if (l == INVALID_SOCKET) return;

    Ev e;                                                          // before the socket: its dtor may still fire
    TClientSocket c(0);
    Hook(c, e);
    c.SetPolled(true);
    c.SetSimMode(false);
    c.Address = "127.0.0.1";
    c.Port = port;
    CHECK(c.IsPolled() && !c.IsSimMode());
    c.Open();
    CHECK(!c.Active && e.connect == 0 && e.error == 0);            // Open() returned at once, no event yet
    SOCKET peer = INVALID_SOCKET;
    const bool up = PollUntil(c, 2000, [&]() { if (peer == INVALID_SOCKET) peer = AcceptNow(l); return e.connect == 1; });
    CHECK(up && c.Active && e.error == 0);
    if (peer == INVALID_SOCKET) peer = AcceptNow(l);
    CHECK(peer != INVALID_SOCKET);
    if (up && peer != INVALID_SOCKET)
    {
        CHECK(c.Socket->SendText("PING\r\n") == 6);
        CHECK(RecvWait(peer, 2000) == "PING\r\n");
        ::send(peer, "PONG\r\n", 6, 0);
        CHECK(PollUntil(c, 2000, [&]() { return e.read >= 1; }));
        CHECK(std::string(c.Socket->ReceiveText().c_str()) == "PONG\r\n");
        ::closesocket(peer);                                       // the peer closes
        peer = INVALID_SOCKET;
        CHECK(PollUntil(c, 2000, [&]() { return e.disconnect == 1; }));
        CHECK(!c.Active);
        c.Close();
        CHECK(e.disconnect == 1);                                  // nothing more: the handle is already gone
    }
    if (peer != INVALID_SOCKET) ::closesocket(peer);
    ::closesocket(l);

    std::printf("  -- refused (a bound, not listening target, held open)\n");
    int rport = 0;
    SOCKET r = Bound(false, &rport);
    CHECK(r != INVALID_SOCKET && rport > 0);
    Ev e2;
    TClientSocket c2(0);
    Hook(c2, e2);
    c2.SetPolled(true);
    c2.SetSimMode(false);
    c2.Address = "127.0.0.1";
    c2.Port = rport;
    c2.Open();
    CHECK(e2.error == 0 && e2.connect == 0 && !c2.Active);         // never from inside Open()
    CHECK(PollUntil(c2, 10000, [&]() { return e2.error == 1; }));
    std::printf("     (OnError code %d, kind %d)\n", e2.lastErr, static_cast<int>(e2.lastKind));
    CHECK(e2.lastKind == eeConnect && e2.lastErr != 0 && e2.connect == 0 && !c2.Active);

    std::printf("  -- Close() while connecting\n");
    Ev e3;
    TClientSocket c3(0);
    Hook(c3, e3);
    c3.SetPolled(true);
    c3.SetSimMode(false);
    c3.Address = "127.0.0.1";
    c3.Port = rport;
    c3.Open();
    c3.Close();
    for (int i = 0; i < 20; ++i) { c3.Poll(); ::Sleep(10); }
    CHECK(e3.connect == 0 && e3.error == 0 && e3.disconnect == 0 && !c3.Active);
    if (r != INVALID_SOCKET) ::closesocket(r);

    std::printf("  -- a plain Sim socket is unchanged\n");
    Ev es;
    TClientSocket s(0);
    Hook(s, es);
    s.Address = "127.0.0.1";
    s.Port = 1;
    s.Open();                                                      // Sim: connects at once, fires OnConnect inside Open()
    CHECK(s.IsSimMode() && !s.IsPolled() && s.Active && es.connect == 1);
    CHECK(s.Poll() == 0);
    s.Close();
    CHECK(es.disconnect == 1);
}

// ---- section 2: TcpPump + golden TfTesterTCP ----------------------------------------------------------------
static bool AddrIsLoopback() { return TestIF_File.asTester_Address == AnsiString("127.0.0.1"); }

static void Section2()
{
    std::printf(" 2. TcpPump end to end (REAL, polled)\n");
    int port = 0;
    SOCKET l = Bound(true, &port);
    CHECK(l != INVALID_SOCKET && port > 0);
    if (l == INVALID_SOCKET) return;
    CHECK(fLotInfo != 0 && fLotInfo->labTCPIPStatus != 0);
    if (fLotInfo == 0 || fLotInfo->labTCPIPStatus == 0) return;

    InitialOK = true;
    TestIF_File.iTestType = TCP_IP_MODE;
    LastSet.iTester = ON_LINE;
    TestIF_File.asTester_Address = "127.0.0.1";
    TestIF_File.iTester_Port = port;
    W906_TcpPumpInit(true);
    CHECK(TesterTCPSocket_ClientSocket->IsPolled() && !TesterTCPSocket_ClientSocket->IsSimMode());

    SOCKET peer = INVALID_SOCKET;
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < 8000 && !TesterTCPSocket.bConnectOK)
    {
        CHECK(AddrIsLoopback());
        if (!AddrIsLoopback()) break;
        TesterTCPSocket_TimerTCPIPConnectTimer();                 // golden: Count1 > 30 -> Open()
        W906_TcpPumpTick();                                         // Poll + replay the golden handlers
        if (peer == INVALID_SOCKET) peer = AcceptNow(l);
        ::Sleep(5);
    }
    CHECK(TesterTCPSocket.bConnectOK);
    CHECK(std::string(fLotInfo->labTCPIPStatus->Caption.c_str()) == "ON-LINE");
    if (peer == INVALID_SOCKET) peer = AcceptNow(l);
    CHECK(peer != INVALID_SOCKET);
    if (TesterTCPSocket.bConnectOK && peer != INVALID_SOCKET)
    {
        TesterTCPSocket_SendTCPIPCommand(0, "SOT", "TEST 1;");      // HandlerBridgeCtl.cpp:910's call shape
        CHECK(RecvWait(peer, 2000) == "TEST 1;\r\n");
        ::send(peer, "HELLO\r\n", 7, 0);
        const DWORD t1 = ::GetTickCount();
        while (::GetTickCount() - t1 < 2000 && std::string(TesterTCPSocket.sTCPIPRecevieData.c_str()).find("HELLO") == std::string::npos)
        {
            W906_TcpPumpTick();
            ::Sleep(5);
        }
        CHECK(std::string(TesterTCPSocket.sTCPIPRecevieData.c_str()).find("HELLO") != std::string::npos);
        ::closesocket(peer);                                        // the tester goes away
        peer = INVALID_SOCKET;
        const DWORD t2 = ::GetTickCount();
        while (::GetTickCount() - t2 < 2000 && TesterTCPSocket.bConnectOK)
        {
            W906_TcpPumpTick();
            ::Sleep(5);
        }
        CHECK(!TesterTCPSocket.bConnectOK);
        CHECK(std::string(fLotInfo->labTCPIPStatus->Caption.c_str()) == "OFF-LINE");
    }
    if (peer != INVALID_SOCKET) ::closesocket(peer);
    ::closesocket(l);

    std::printf("  -- a refused tester -> ERROR, one box at the next timer\n");
    int rport = 0;
    SOCKET r = Bound(false, &rport);
    CHECK(r != INVALID_SOCKET && rport > 0);
    TestIF_File.iTester_Port = rport;
    const int boxes0 = W906_ShowMyMessage_Count;
    const DWORD t3 = ::GetTickCount();
    // the pump's own 1000 ms timer may show the box inside a tick (and clear the flag): stop on either
    while (::GetTickCount() - t3 < 45000 && !TesterTCPSocket.bTCPError && W906_ShowMyMessage_Count == boxes0)
    {
        CHECK(AddrIsLoopback());
        if (!AddrIsLoopback()) break;
        TesterTCPSocket_TimerTCPIPConnectTimer();
        W906_TcpPumpTick();
        ::Sleep(5);
    }
    CHECK(TesterTCPSocket.bTCPError || W906_ShowMyMessage_Count == boxes0 + 1);
    CHECK(std::string(fLotInfo->labTCPIPStatus->Caption.c_str()) == "ERROR");
    CHECK(AddrIsLoopback());
    if (AddrIsLoopback() && TesterTCPSocket.bTCPError)
        TesterTCPSocket_TimerTCPIPConnectTimer();                 // golden :196-200: the box, then the flag clears
    CHECK(W906_ShowMyMessage_Count == boxes0 + 1);
    CHECK(std::string(W906_ShowMyMessage_LastS1.c_str()) == "TCP/IP error for tester!");
    CHECK(!TesterTCPSocket.bTCPError);
    LastSet.iTester = OFF_LINE;                                     // stop the timers: TcpPump closes on the edge
    W906_TcpPumpTick();
    if (r != INVALID_SOCKET) ::closesocket(r);
}

static void Section3()
{
    std::printf(" 3. W906_TcpTesterRealSocket\n");
#ifdef SOFT_SIMULTE
    CHECK(W906_TcpTesterRealSocket() == false);
    HT9045_TEST_PUTENV(const_cast<char*>("HT9045_TCPCMD_SIM=0"));
    CHECK(W906_TcpTesterRealSocket() == false);                     // SIM stays SIM whatever the variable says
#else
    HT9045_TEST_PUTENV(const_cast<char*>("HT9045_TCPCMD_SIM=0"));
    CHECK(W906_TcpTesterRealSocket() == true);
    HT9045_TEST_PUTENV(const_cast<char*>("HT9045_TCPCMD_SIM=1"));
    CHECK(W906_TcpTesterRealSocket() == false);
    HT9045_TEST_PUTENV(const_cast<char*>("HT9045_TCPCMD_SIM=11"));
    CHECK(W906_TcpTesterRealSocket() == true);                      // exactly "1", as TesterCommWiring.cpp:134
#endif
    HT9045_TEST_PUTENV(const_cast<char*>("HT9045_TCPCMD_SIM="));
}

int main()
{
    std::printf("TesterComm_TcpTesterReal\n");
    asTestTCPIPLogPath = ".\\_test_scratch_tcptesterreal";          // never the production D:\HT9045_Log\Test_TCPIP
    WSADATA wsa;
    CHECK(::WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
    Section1();
    Section2();
    Section3();
    ::WSACleanup();
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
