// =============================================================================
//  test_tcp_cmd_server.cpp -- W10 = B: the Handler's TCP command server 7016 / result server 7017 end to end.
//
//  AI(W906-W10) 20260927 (St02-E).  Suite name (add_test): TesterComm_TcpCmdServer
//
//  The two fMain servers stay in SIM for the whole run (W906_CmdServerPumpInit(false)); the test FAILS if either one
//  ever has a real OS socket (IsSimMode / BoundPort).  The only real socket is a test-owned polled server on
//  127.0.0.1:<ephemeral> (never 7016 / 7017) in section 9.
//
//    1. golden FormShow HanderTcpIp (main.cpp:10974) on the first pump pass: both Sim servers Active on 7016 / 7017,
//       "[7016] Server Listen"; SimFailNextOpen -> golden's catch: "Socket Server Open Error!!" (log + ShowMyMessage);
//    2. a table of command replies through the pump + framer + the unchanged golden body (quirks included:
//       HTSET,317 answers with an HTSET prefix, HTSET,701 -> "HTSR,701,OK," (W-213 step 4; was "HTSR,701,," while gated), 519 / 520 NG, 720 / 721 no trailing comma,
//       gated 208 -> 0 bytes, unknown -> 0 bytes), and the S-a writes land in the sandbox's config.ini / *.Data;
//       W40 option A: HTSET,354 writes the sandbox Contact.Data [Torque Control] N / G (both key-name sets, golden
//       Command.cpp:13433-13447 with fContact closed), reads them back into DeviceForm_File, Fail and no write when running.
//    3. every reply goes to every 7016 client (golden broadcast);
//    4. R2 through the pump: split commands, several per read, 12 in one read (10 per pass), noise, a >2048-byte blob
//       (#Overflow# in the log), 4096 bytes of garbage (#Discard#), the next command still answered;
//    5. HTSET,333 / 334: not installed -> NG; a stub W906_RemoteRun gets exactly "TCP Command Start!!" /
//       "TCP Command Pause"; refused (manual teach) -> NG; the base TfMain::Pause counter never moves;
//    6. HTSET,702,abc (vclcompat StrToInt throws): caught at the pump, no reply, #Exception# logged, the next command
//       answered; HTSET,702,12,LOTX -> golden's two "HTSR,702,OK," (iTesterType 1);
//    7. file writes only in the sandbox: the machine's config.ini / SetUp.inf / lastdata.dat and today's real
//       TCPIP_Log file keep their size and write time (read-only stat);
//    8. R1: HTGR,255 with 255 bins (~3 KB) and HTSET,403 with 1500 characters are sent in full;
//    9. vclcompat POLLED real mode on loopback: callbacks on the test thread, an exclusive-bind conflict, a 3000-byte
//       send, the 64 KiB queue cap with no byte lost, peer close -> OnClientDisconnect;
//   10. HTSET,322 / 323 (R2): the index split across reads is joined; out of range -> golden's reply + #Ignore#.
//  Paths: the test points AuthPath / DataPath / LastDataPath / asTCPIPPath / as9045LogPath at %TEMP%\ht9045_w10_<tick>
//  and aborts before any call if one is still under D:\HT9045 (or D:\HT9045_Log).  lastdata*.dat is redirected by
//  tests/test_bootstrap.cpp.  CloseIniFile() is never called.  The sandbox is removed on a green run.
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h>

#include "forms/fMain.h"
#include "forms/fLotInfo.h"
#include "forms/fSCKART.h"
#include "TesterComm/Tcp/CmdServerPump.h"
#include "TesterComm/Tcp/TcpCmdFramer.h"
#include "canary_support.h"
#include "common.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261008 (Ifor01): fTemp_Set (see main)

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
        std::fflush(stdout);                                                    \
    } while (0)

static std::string g_root;

// ---- files ------------------------------------------------------------------------------------------------------
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    return s.compare(0, 9, "d:\\ht9045") == 0;   // D:\HT9045\... and D:\HT9045_Log\...
}
static std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}
static void AllFiles(const std::string& dir, std::string* out)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do
    {
        if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
        const std::string p = dir + "\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) AllFiles(p, out);
        else *out += Bytes(p);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}
static std::string TcpLog() { std::string s; AllFiles(g_root + "\\TCPIP_Log", &s); return s; }
static size_t Occurrences(const std::string& hay, const std::string& needle)
{
    size_t n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + 1)) ++n;
    return n;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}
static std::string Key(const std::string& file, const char* sec, const char* key)
{
    char buf[256];
    ::GetPrivateProfileStringA(sec, key, "<none>", buf, sizeof(buf), file.c_str());
    return buf;
}

// read-only stamp of a machine file (size + write time); "" when absent
struct Stamp { bool there; DWORD size; FILETIME t; };
static Stamp StampOf(const std::string& p)
{
    Stamp s; std::memset(&s, 0, sizeof(s));
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &d)) { s.there = true; s.size = d.nFileSizeLow; s.t = d.ftLastWriteTime; }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b)
{
    return a.there == b.there && (!a.there || (a.size == b.size && ::CompareFileTime(&a.t, &b.t) == 0));
}

// ---- the Sim 7016 clients -----------------------------------------------------------------------------------------
static std::string Tx(TCustomWinSocket* k)
{
    const std::vector<char>& v = k->SimTxBuffer();
    std::string s(v.begin(), v.end());
    k->SimClearTx();
    return s;
}
static void Send(TCustomWinSocket* k, const std::string& s) { k->SimPushReceive(s.data(), (int)s.size()); }
static std::string Ask(TCustomWinSocket* k, const std::string& cmd)
{
    Send(k, cmd);
    W906_CmdServerPumpTick();
    // AI(W906-W10-SAFE) 20260928 (St02-E): the test used to null INIFileGeneral / INIFile / INIFileMem here; since 4d3468a3
    //   common.cpp nulls them after each delete itself, and nulling them here would leak live objects (St02-E2 review A10).   //AI(W906-MERGE-0929) 20260929: the 322/323 segfault was INIFileGeneral never opened in this test (only LoadMachineConfig opens it, database.cpp:3142), not a dangling pointer -- see :385
    return Tx(k);
}
static bool ServersStaySim()
{
    if (fMain == 0 || fMain->TCPCommandServer == 0 || fMain->TeraTCPResultServer == 0)
        return false;   // AI(W906-W10fix) 20260928 (St02-E): no pair = no golden servers (St01 20:40 SIGSEGV here)
    return fMain->TCPCommandServer->IsSimMode() && fMain->TeraTCPResultServer->IsSimMode()
        && fMain->TCPCommandServer->BoundPort() == 0 && fMain->TeraTCPResultServer->BoundPort() == 0;
}

// ---- the W906_RemoteRun stub (section 5) -------------------------------------------------------------------------
static int g_startCalls = 0, g_pauseCalls = 0;
static bool g_startAllowed = true;
static std::string g_startFunc, g_pauseFunc;
static bool StubStart(AnsiString f) { if (!g_startAllowed) return false; ++g_startCalls; g_startFunc = f.c_str(); return true; }
static bool StubPause(AnsiString f) { ++g_pauseCalls; g_pauseFunc = f.c_str(); return true; }

// ---- section 9: a test-owned polled real server on loopback -------------------------------------------------------
static bool PollUntil(TServerSocket& s, bool (*done)(), DWORD ms)
{
    const DWORD t0 = ::GetTickCount();
    for (;;)
    {
        s.Poll();
        if (done()) return true;
        if (::GetTickCount() - t0 > ms) return false;
        ::Sleep(5);
    }
}
static int g_lbConnects = 0, g_lbDisconnects = 0, g_lbReads = 0;
static bool g_lbWrongThread = false, g_lbDrain = true;
static DWORD g_lbThread = 0;
static std::string g_lbGot;
static TServerSocket* g_lb = 0;
static bool LbConnected() { return g_lbConnects == 1; }
static bool LbGot9() { return g_lbGot.size() >= 9; }
static bool LbGone() { return g_lbDisconnects == 1; }
static bool LbQueueFull() { return g_lb->Socket->ActiveConnections == 1 && g_lb->Socket->Connections[0]->ReceiveLength() == TServerSocket::kPolledQueueCap; }

static void Section9Loopback()
{
    std::printf("\n-- 9. polled real mode on 127.0.0.1 (a test-owned server, never 7016 / 7017)\n");
    WSADATA wsa;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { CHECK(false, "9. WSAStartup"); return; }
    TServerSocket srv(NULL);
    g_lb = &srv;
    g_lbThread = ::GetCurrentThreadId();
    srv.SetPolled(true);
    srv.SetSimMode(false);
    srv.SetExclusiveAddr(true);
    srv.SetBindAddress("127.0.0.1");
    srv.Port = 0;
    srv.OnClientConnect = [](TObject*, TCustomWinSocket*) { ++g_lbConnects; if (::GetCurrentThreadId() != g_lbThread) g_lbWrongThread = true; };
    srv.OnClientDisconnect = [](TObject*, TCustomWinSocket*) { ++g_lbDisconnects; if (::GetCurrentThreadId() != g_lbThread) g_lbWrongThread = true; };
    srv.OnClientRead = [](TObject*, TCustomWinSocket* k) {
        ++g_lbReads;
        if (::GetCurrentThreadId() != g_lbThread) g_lbWrongThread = true;
        if (g_lbDrain) { AnsiString a = k->ReceiveText(); g_lbGot.append(a.c_str(), (size_t)a.Length()); }
    };
    srv.Open();
    const int port = srv.BoundPort();
    CHECK(srv.Active && !srv.IsSimMode() && srv.IsPolled() && port > 0 && port != 7016 && port != 7017,
          "9. polled Open(): listening for real on an ephemeral loopback port");

    TServerSocket srv2(NULL);
    srv2.SetPolled(true);
    srv2.SetSimMode(false);
    srv2.SetExclusiveAddr(true);
    srv2.SetBindAddress("127.0.0.1");
    srv2.Port = port;
    srv2.Open();
    CHECK(!srv2.Active, "9. a second exclusive listener on the same port fails (Active false = golden's ESocketError)");

    SOCKET c = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in a;
    std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = ::inet_addr("127.0.0.1");
    a.sin_port = ::htons((u_short)port);
    DWORD tmo = 3000;
    ::setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));
    const bool conn = (c != INVALID_SOCKET) && ::connect(c, (sockaddr*)&a, sizeof(a)) == 0;
    CHECK(conn, "9. a Winsock client connects");
    if (!conn) { if (c != INVALID_SOCKET) ::closesocket(c); srv.Close(); ::WSACleanup(); return; }
    CHECK(PollUntil(srv, &LbConnected, 3000) && srv.Socket->ActiveConnections == 1, "9. Poll() accepts it: OnClientConnect, ActiveConnections 1");
    ::send(c, "HTGR,101,", 9, 0);
    CHECK(PollUntil(srv, &LbGot9, 3000) && g_lbGot == "HTGR,101,", "9. Poll() reads it: OnClientRead, ReceiveText = the 9 bytes");
    CHECK(!g_lbWrongThread, "9. every callback ran on the Poll() caller's (test) thread");

    TCustomWinSocket* k = srv.Socket->Connections[0];
    std::string big(3000, 'r');
    const int sent = k->SendBuf(&big[0], (int)big.size());
    std::string back;
    char buf[4096];
    while (back.size() < big.size())
    {
        const int n = ::recv(c, buf, sizeof(buf), 0);
        if (n <= 0) break;
        back.append(buf, (size_t)n);
    }
    CHECK(sent == 3000 && back == big, "9. SendBuf of 3000 bytes (non-blocking) reaches the client whole");

    // the queue cap: the handler stops draining, the client pushes more than 64 KiB (non-blocking)
    g_lbDrain = false;
    u_long nb = 1;
    ::ioctlsocket(c, FIONBIO, &nb);
    std::string blob(200000, 'q');
    size_t pushed = 0;
    const DWORD t0 = ::GetTickCount();
    while (pushed < blob.size() && ::GetTickCount() - t0 < 3000)
    {
        const int n = ::send(c, blob.data() + pushed, (int)(blob.size() - pushed), 0);
        if (n > 0) { pushed += (size_t)n; continue; }
        if (::WSAGetLastError() != WSAEWOULDBLOCK) break;
        srv.Poll();
        if (LbQueueFull()) break;
        ::Sleep(2);
    }
    const bool full = pushed > (size_t)TServerSocket::kPolledQueueCap && PollUntil(srv, &LbQueueFull, 3000);
    CHECK(full && k->ReceiveLength() == TServerSocket::kPolledQueueCap, "9. the receive queue stops at kPolledQueueCap (64 KiB)");
    // drain everything: nothing was dropped (TCP flow control held the rest)
    size_t drained = 0;
    std::vector<char> tmp(70000);
    const DWORD t1 = ::GetTickCount();
    while (drained < pushed && ::GetTickCount() - t1 < 5000)
    {
        const int n = k->ReceiveBuf(&tmp[0], (int)tmp.size());
        if (n > 0) drained += (size_t)n;
        // flush what the client could not hand over yet
        while (pushed < blob.size())
        {
            const int m = ::send(c, blob.data() + pushed, (int)(blob.size() - pushed), 0);
            if (m <= 0) break;
            pushed += (size_t)m;
        }
        srv.Poll();
        if (n <= 0) ::Sleep(2);
    }
    CHECK(drained == pushed && drained > (size_t)TServerSocket::kPolledQueueCap, "9. every byte the client sent arrived (no loss at the cap)");

    ::closesocket(c);
    CHECK(PollUntil(srv, &LbGone, 3000) && srv.Socket->ActiveConnections == 0, "9. peer close -> OnClientDisconnect, ActiveConnections 0");
    CHECK(!g_lbWrongThread, "9. still every callback on the test thread");
    srv.Close();
    CHECK(!srv.Active && srv.BoundPort() == 0, "9. Close()");
    g_lb = 0;
    ::WSACleanup();
}

int main()
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261008 (Ifor01): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite, which calls fTemp_Set->InitialAddrToATC() since N1-G5
    std::setvbuf(stdout, NULL, _IONBF, 0);   // AI(W906-W10-SAFE) 20260928: unbuffered -- a crash report shows the exact last line
    std::printf("TesterComm_TcpCmdServer\n");

    // ---- 0. containment first (AI(W906-W10-SAFE) 20260928, St02-E) ----------------------------------------------------
    // St01 09:0x: run BY HAND (no ctest environment) this exe wrote the real D:\HT9045_Log\SaveEventLog\HANDLER LOG_*.csv --
    // since P4 the golden RecordProcess / MyDBIProcess bodies write through these globals, which only ctest's redirect roots
    // (tests/CMakeLists.txt: the global ENVIRONMENT block + _ht9045_env_extra, "machine_log_scratch") point away from the
    // machine.  Same rule as MyDB_P4_Containment: refuse, exit 2, before ANY Handler code, unless every root is the scratch.
    {
        const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
        const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
        const char* const envs[] = { "W906_HT9045LOG_ROOT", "W906_SAVEEVENTLOG_ROOT", "W906_RMS_ROOT", "W906_PRODINFO_ROOT",
                                     "W906_EVENTLOG_ROOT", "W906_AUTH_PATH" };
        bool contained = true;
        for (int i = 0; i < 4; ++i)
        {
            const std::string r = Lower(roots[i]->c_str());
            std::printf("  %s = %s\n", names[i], roots[i]->c_str());
            if (r.find("machine_log_scratch") == std::string::npos)   // ctest's scratch may itself sit under D:\HT9045\...\build
                contained = false;
        }
        for (size_t i = 0; i < sizeof(envs) / sizeof(envs[0]); ++i)
        {
            const char* v = std::getenv(envs[i]);
            const std::string lv = Lower(v ? v : "");
            if (lv.find("machine_log_scratch") == std::string::npos && lv.find("machine_config_scratch") == std::string::npos)
            {
                std::printf("  %s = %s\n", envs[i], v ? v : "(unset)");
                contained = false;
            }
        }
        if (!contained)
        {
            std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R TesterComm_TcpCmdServer) -- nothing was called\n");
            return 2;
        }
    }

    // ---- the sandbox ------------------------------------------------------------------------------------------------
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w10_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\config").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data\\W10R").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\TCPIP_Log").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\HT9045_Log").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\system").c_str(), 0);   // AI(W906-W10-SAFE) 20260928: the sandbox Gerneral.ini (below)
    {
        FILE* f = std::fopen((g_root + "\\SetUp.inf").c_str(), "wb");
        if (f) { std::fputs("W10R\r\n", f); std::fclose(f); }
    }
    AuthPath      = AnsiString((g_root + "\\config\\").c_str());
    DataPath      = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath  = AnsiString((g_root + "\\SetUp.inf").c_str());
    asTCPIPPath   = AnsiString((g_root + "\\TCPIP_Log").c_str());
    as9045LogPath = AnsiString((g_root + "\\HT9045_Log").c_str());
    // AI(W906-W10-SAFE) 20260928 (St02-E): a save command reaches golden SetWorkParameter -> ReadTechData, which opens   //AI(W906-MERGE-0929) 20260929: it does not open it: ReadTechData reads through INIFileGeneral, opened by LoadMachineConfig in wb_serve (golden SYSTEM_MODULAR ctor database.cpp:50); this test opens it at :385
    //   asGeneralPath -- the REAL D:\HT9045\system\Gerneral.ini unless W906_GENERAL_INI_PATH is set.  AI(W906-TESTGUARD-ST02) 20260928:
    //   since TESTGUARD (main 3fb74540) ctest sets it (general_ini_scratch/<test>); this test still uses its own %TEMP% copy below.
    asGeneralPath = AnsiString((g_root + "\\system\\Gerneral.ini").c_str());
    const char* paths[] = { AuthPath.c_str(), DataPath.c_str(), LastDataPath.c_str(), asTCPIPPath.c_str(), as9045LogPath.c_str(),
                            asGeneralPath.c_str() };
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
        if (UnderMachineTree(paths[i]))
        {
            std::printf("  ABORT: %s is not in the sandbox -- nothing was called\n", paths[i]);
            return 2;
        }
    OpenGeneralIniFile(); W906_CmdServersEnsure();   // AI(W906-W10fix) 20260928 (St02-E): this test never runs the TfMain ctor (forms/fMain.cpp:235, live since b2a7349c), so it creates the servers itself   //AI(W906-MERGE-0929) 20260929: open INIFileGeneral on the sandbox asGeneralPath set at :376 (wb_serve does it in LoadMachineConfig) -- HTSET,322 fBinSel->spbSaveClick -> SetWorkParameter -> ReadTechData -> CheckAndReadIniDataGeneral derefs it (cinitial.cpp:16137 / common.cpp:1645)
    if (!ServersStaySim())
    {
        std::printf("  ABORT: an fMain server is missing or not in Sim before the test\n");
        return 2;
    }

    // 7. the machine's own files (read-only stamps)
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char realLog[MAX_PATH];
    std::snprintf(realLog, sizeof(realLog), "D:\\HT9045_Log\\TCPIP_Log\\%04d_%02d_%02d\\%04d_%02d_%02d_%02d.txt",
                  st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour);
    const char* machine[] = { "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\SetUp.inf", "D:\\HT9045\\system\\lastdata.dat", realLog,
                              "D:\\HT9045\\system\\Gerneral.ini" };   // AI(W906-W10-SAFE) 20260928: + the real General ini
    const int kMachine = (int)(sizeof(machine) / sizeof(machine[0]));
    Stamp before[5];
    for (int i = 0; i < kMachine; ++i) before[i] = StampOf(machine[i]);

    // ---- the state the replies read ---------------------------------------------------------------------------------
    const int oldCC = CUSTOMER_CODE;
    CUSTOMER_CODE = 868;                                        // not CC_TERAPOWER (section 5 sets it)
    CosFunction.bEnableHandlerResultServer = true;
    CosFunction.bRemoteLotStart = false;
    LastSet.SendCT[0] = 13546;
    iSECSGEMPass = 10849;
    iSECSGEMFail = 7;
    LastSet.iTester = OFF_LINE;
    fLotInfo->edtSysLotID->Text = "LOT-W10";
    fLotInfo->edtSysOperatorID->Text = "OP-W10";
    IniConfig.sGPIBMachineID = "PJLY1027";
    fMain->palMainStatus->Caption = "HALT";
    SystemStart = false;
    bIsAutoOneCycle = false;
    DeviceForm_File.ForcePerPinG = 40.0;
    LastSet.iTemperature = Tempture_Ambient;
    LastSet.iRunStartMode = 0;
    TestIF_File.bSCKART_EnableART = false;
    Prod.bART6Tray[0] = Prod.bART6Tray[1] = Prod.bART6Tray[2] = false;
    LastSet.iTCPModeLotState = 2;
    bRunAutoClean = false;
    bLoadingNewICTray = false;
    LastSet.bAMRLoaderLast = false;
    bQAModeFlag = false;                                        // HTSET,702 reaches StrToInt (golden :13821)
    iTestRunMode = 0;
    const int pause0 = fMain->W906_PauseCallCount;

    // ---- 1. FormShow HanderTcpIp on the first pass ----------------------------------------------------------------
    std::printf("\n-- 1. HanderTcpIp (Sim)\n");
    W906_CmdServerPumpInit(false);
    W906_CmdServerPumpTick();
    CHECK(fMain->TCPCommandServer->Active && fMain->TCPCommandServer->Port == 7016 && fMain->TeraTCPResultServer->Active
          && fMain->TeraTCPResultServer->Port == 7017, "1. the first pump pass opens 7016 / 7017 (golden FormShow main.cpp:10974)");
    CHECK(ServersStaySim(), "1. ... in Sim: no OS socket, BoundPort 0");
    std::string log = TcpLog();
    CHECK(log.find("[7016] Server Listen") != std::string::npos && log.find("[7017] Server Listen") != std::string::npos,
          "1. \"[7016] Server Listen\" / \"[7017] Server Listen\" in the sandbox TCPIP_Log");

    TCustomWinSocket* a = fMain->TCPCommandServer->SimAcceptConnection("10.0.0.1", 5001);
    TCustomWinSocket* b = fMain->TCPCommandServer->SimAcceptConnection("10.0.0.2", 5002);
    CHECK(fMain->TCPCommandServer->Socket->ActiveConnections == 2 && TcpLog().find("Client Connect") != std::string::npos,
          "1. two clients: golden TCPCommandServerClientConnect logged");

    // ---- 3. broadcast ------------------------------------------------------------------------------------------------
    std::printf("\n-- 3. broadcast\n");
    CHECK(Ask(a, "HTGR,109,") == "HTSR,109,Off Line," && Tx(b) == "HTSR,109,Off Line,", "3. the reply goes to BOTH clients (golden)");
    fMain->TCPCommandServer->SimDropConnection(b);
    CHECK(fMain->TCPCommandServer->Socket->ActiveConnections == 1, "3. client B gone");

    // ---- 2. the reply table ---------------------------------------------------------------------------------------
    std::printf("\n-- 2. replies\n");
    struct Row { const char* cmd; const char* reply; };
    const Row rows[] = {
        { "HTGR,101,", "HTSR,101,13546," },           { "HTGR,102,", "HTSR,102,10849," },
        { "HTGR,103,", "HTSR,103,7," },               { "HTGR,109,", "HTSR,109,Off Line," },
        { "HTGR,201,", "HTSR,201,LOT-W10," },         { "HTGR,202,", "HTSR,202,OP-W10," },
        { "HTGR,203,", "HTSR,203,PJLY1027," },        { "HTSET,205,", "HTSR,205,HALT," },
        { "HTGR,215,", "HTSR,215,HALT," },            { "HTGR,252,", "HTSR,252,W10R," },
        { "HTGR,254,", "HTSR,254,40.0000," },         { "HTSET,306,", "HTSR,306,ContinuefailON," },
        { "HTSET,307,", "HTSR,307,ContinuefailOFF," }, { "HTSET,308,", "HTSR,308,Empty," },
        { "HTSET,309,", "HTSR,309,Color," },          { "HTSET,310,", "HTSR,310,Supervisor," },
        { "HTSET,311,", "HTSR,311,Operator," },       { "HTSET,312,", "HTSR,312,LowYieldOpen," },
        { "HTSET,313,", "HTSR,313,LowYieldClose," },  { "HTSET,317,", "HTSET,317,RT," },        // golden quirk: HTSET prefix
        { "HTGR,401,", "HTSR,401,OFF," },             { "HTSET,403,hello,", "HTSR,403,hello,OK," },
        { "HTSET,519,", "HTSR,519,NG," },             { "HTSET,520,", "HTSR,520,NG," },
        { "HTSET,701,", "HTSR,701,OK," },           { "HTSET,703,", "HTSR,703,OK," },   // 701: AI(W906-W213) 20261010 (St02-E), golden 913 Command.cpp:13780 (iTesterType 1 -> the OK arm)
        { "HTSET,704,", "HTSR,704,OK," },             { "HTSET,706,", "HTSR,706,2," },
        { "HTSET,720,LOTX,", "HTSR,720,NG,NOT_SUPPORTED" }, { "HTSET,721,", "HTSR,721,NG,NOT_SUPPORTED" },
        { "HTGR,732,", "HTSR,732,Close," },           { "HTGR,733,", "HTSR,733,NG," },
        { "HTGR,801,", "HTSR,801,Normal," },          { "HTSET,803,", "HTSR,803,OK," },
        { "HTSET,841,", "HTSR,841,OK," },             { "HTSET,851,", "HTSR,851,OK," },
        { "HTSET,861,", "HTSR,861,OK," },             { "HTGR,208,", "HTSR,208,TestTime,0.0," },   // AI(W906-S09-ST) 20260930 (St02-E): golden Command.cpp:12978 now live (fObserver is the real TfObserver; TimeInfoGrid Cells[3][14] is "" -> atof 0.0); was 0 bytes while A9 was gated
        { "HTGR,999,", "" },                          // unknown: golden sends 0 bytes
    };
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i)
    {
        const std::string got = Ask(a, rows[i].cmd);
        char msg[256];
        std::snprintf(msg, sizeof(msg), "2. %s -> \"%s\"%s%s", rows[i].cmd, rows[i].reply, got == rows[i].reply ? "" : "  got: ", got == rows[i].reply ? "" : got.c_str());
        CHECK(got == rows[i].reply, msg);
    }
    CHECK(W906_ShowMyMessage_LastS1 == "hello", "2. HTSET,403 -> ShowMyMessage(\"hello\")");
    // S-a: the config / recipe writes land in the sandbox
    const std::string cfg = g_root + "\\config\\config.ini";
    CHECK(Ask(a, "HTSET,314,") == "HTSR,314,DoubleContactD22Ture," && Key(cfg, "Index", "bD22SupportMultiDoubleContact") != "<none>",
          "2. S-a HTSET,314 writes [Index] bD22SupportMultiDoubleContact into the sandbox config.ini");
    CHECK(Ask(a, "HTSET,316,4,") == "HTSR,316,SetOK," && IniConfig.iD22DoubleContactCount == 2 && Key(cfg, "Index", "iD22DoubleContactCount") != "<none>",
          "2. S-a HTSET,316,4 -> iD22DoubleContactCount 2, written");
    CHECK(Ask(a, "HTSET,315,") == "HTSR,315,DoubleContactD22False,", "2. HTSET,315");
    CHECK(Ask(a, "HTSET,331,") == "HTSR,331,GpibLotEndI31_True," && Key(cfg, "Specific", "I31_GPIBLotEnd") != "<none>", "2. S-a HTSET,331 writes [Specific] I31_GPIBLotEnd");
    CHECK(Ask(a, "HTSET,332,") == "HTSR,332,GpibLotEndI31_Off,", "2. HTSET,332");
    CHECK(Ask(a, "HTSET,804,1,") == "HTSR,804,OK," && IniConfig.bA60EnableAMR && Key(cfg, "Function", "bA60EnableAMR") != "<none>", "2. S-a HTSET,804,1 writes [Function] bA60EnableAMR");
    CHECK(Ask(a, "HTGR,805,") == "HTSR,805,1,", "2. HTGR,805 -> 1");
    CHECK(Ask(a, "HTSET,804,5,") == "HTSR,804,NG,", "2. HTSET,804,5 -> NG");
    const std::string hc = g_root + "\\Data\\W10R\\HandlerCondition.Data";
    CHECK(Ask(a, "HTSET,461,On,") == "HTSR,461,OK," && Key(hc, "Configuration", "iAutoClean_Function") != "<none>", "2. S-a HTSET,461,On -> HandlerCondition.Data");
    CHECK(Ask(a, "HTSET,463,1.5,") == "HTSR,463,OK," && Key(hc, "Configuration", "iAutoClean_ContactTime") != "<none>", "2. S-a HTSET,463");
    CHECK(Ask(a, "HTSET,464,3,") == "HTSR,464,OK," && Key(hc, "Configuration", "iAutoClean_ContactCount") != "<none>", "2. S-a HTSET,464");
    CHECK(Ask(a, "HTSET,466,4,") == "HTSR,466,OK," && Key(hc, "Configuration", "iAutoClean_IntervalContact") != "<none>", "2. S-a HTSET,466");
    CHECK(Ask(a, "HTSET,468,5,") == "HTSR,468,OK," && Key(hc, "Configuration", "iAutoClean_AlarmCount") != "<none>", "2. S-a HTSET,468");
    CHECK(Ask(a, "HTSET,470,2.5,") == "HTSR,470,OK," && Key(hc, "Configuration", "fAutoClean_DevicePinForceGf") != "<none>", "2. S-a HTSET,470");
    const std::string td = g_root + "\\Data\\W10R\\Tester.Data";
    CHECK(Ask(a, "HTSET,710,5,") == "HTSR,710,OK," && TestIF_File.iSCKART_TryCnt == 5 && Key(td, "AutoRetest", "Try Count") != "<none>", "2. S-a HTSET,710,5 -> Tester.Data [AutoRetest] Try Count");
    CHECK(Ask(a, "HTSET,713,0,") == "HTSR,713,OK," && Key(td, "AutoRetest", "Auto Socket Off") != "<none>", "2. S-a HTSET,713");
    const std::string ct = g_root + "\\Data\\W10R\\Contact.Data";
    CHECK(Ask(a, "HTSET,350,-1.5,-2.5,") == "HTSR,350,OK," && Key(ct, "Test Arm1", "Contact") == "-1.5" && Key(ct, "Test Arm2", "Contact") == "-2.5",
          "2. S-a HTSET,350 (4 fields) writes Contact.Data [Test Arm1] / [Test Arm2]");
    CHECK(DeviceForm_File.IndexContact[0] == -1.5 && DeviceForm_File.IndexContact[1] == -2.5, "2. HTSET,350 reads them back (golden ReadWriteIni)");
    // AI(W906-W40-354) 20260928 (St02-E helper): W40 option A -- HTSET,354 writes Contact.Data as golden 906_0625_Steven
    // Command.cpp:13433-13447 does with the Contact form closed: N is the text golden's DoIniDataToForm left in
    // edForcePerPinN (cContact.cpp:937, FormatFloat "0.0000" of DeviceForm_File.ForcePerPinN), G is the command's field
    // verbatim (golden sets edForcePerPinG->Text to sData[2], its OnChange returns while fShow is false, :1943), then both
    // are read back.
    {
        const bool oldFix = CosFunction.bFixNameOfForcePerPinG;
        const double oldN = DeviceForm_File.ForcePerPinN, oldG = DeviceForm_File.ForcePerPinG;
        CosFunction.bFixNameOfForcePerPinG = true;
        DeviceForm_File.ForcePerPinN = 0.29403;                 // golden's edit box shows it as "0.2940"
        CHECK(Ask(a, "HTSET,354,33.3,") == "HTSR,354,OK,", "2. W40 HTSET,354,33.3 -> golden reply \"HTSR,354,OK,\"");
        CHECK(Key(ct, "Torque Control", "Force Per Pin N") == "0.2940" && Key(ct, "Torque Control", "Force Per Pin G") == "33.3",
              "2. W40 bFixNameOfForcePerPinG: [Torque Control] Force Per Pin N=0.2940 (edit text), Force Per Pin G=33.3 (verbatim)");
        CHECK(Key(ct, "Torque Control", "Force Per Pin") == "<none>" && Key(ct, "Torque Control", "Force Per Pin Kg") == "<none>",
              "2. W40 ... the legacy key names are not written in this branch");
        CHECK(std::fabs(DeviceForm_File.ForcePerPinN - 0.294) < 1e-12 && std::fabs(DeviceForm_File.ForcePerPinG - 33.3) < 1e-12,
              "2. W40 ... read back into DeviceForm_File: N 0.294, G 33.3 (golden :13438-13439)");
        CHECK(Ask(a, "HTGR,254,") == "HTSR,254,33.3000,", "2. W40 HTGR,254 now answers the new G");
        CHECK(Key(ct, "Test Arm1", "Contact") == "-1.5" && Key(ct, "Test Arm2", "Contact") == "-2.5",
              "2. W40 ... the rest of Contact.Data is unchanged");
        CosFunction.bFixNameOfForcePerPinG = false;
        DeviceForm_File.ForcePerPinN = 0.5;
        CHECK(Ask(a, "HTSET,354,51.0204,") == "HTSR,354,OK," && Key(ct, "Torque Control", "Force Per Pin") == "0.5000"
              && Key(ct, "Torque Control", "Force Per Pin Kg") == "51.0204",
              "2. W40 legacy names (golden :13443-13444): Force Per Pin=0.5000, Force Per Pin Kg=51.0204");
        CHECK(Key(ct, "Torque Control", "Force Per Pin N") == "0.2940" && Key(ct, "Torque Control", "Force Per Pin G") == "33.3",
              "2. W40 ... the new-name keys are left as they were");
        CHECK(std::fabs(DeviceForm_File.ForcePerPinN - 0.5) < 1e-12 && std::fabs(DeviceForm_File.ForcePerPinG - 51.0204) < 1e-12,
              "2. W40 ... read back (golden :13445-13446)");
        const std::string ct0 = Bytes(ct);
        SystemStart = true;
        CHECK(Ask(a, "HTSET,354,77,") == "HTSR,354,Fail," && Bytes(ct) == ct0, "2. W40 running -> \"HTSR,354,Fail,\", Contact.Data not touched (golden)");
        SystemStart = false;
        CosFunction.bFixNameOfForcePerPinG = oldFix;
        DeviceForm_File.ForcePerPinN = oldN;
        DeviceForm_File.ForcePerPinG = oldG;
    }
    CHECK(Ask(a, "HTSET,811,Last,") == "HTSR,811,OK," && LastSet.bAMRLoaderLast, "2. HTSET,811,Last");
    CHECK(Ask(a, "HTSET,812,") == "HTSR,812,NO,", "2. HTSET,812 after Last -> NO");
    LastSet.bAMRLoaderLast = false;
    CHECK(Ask(a, "HTSET,812,") == "HTSR,812,OK," && LastSet.bAMRRequestSupplyTray, "2. HTSET,812 -> OK, bAMRRequestSupplyTray (golden)");
    SystemStart = true;
    CHECK(Ask(a, "HTSET,404,99,") == "HTSR,404,NG,Running,", "2. HTSET,404 while running -> NG,Running,");
    SystemStart = false;

    // ---- 4. R2 through the pump -------------------------------------------------------------------------------------
    std::printf("\n-- 4. R2 through the pump\n");
    Send(a, "HTGR,1");
    W906_CmdServerPumpTick();
    CHECK(Tx(a).empty(), "4. \"HTGR,1\": no reply yet");
    CHECK(Ask(a, "01,") == "HTSR,101,13546,", "4. + \"01,\" -> one reply");
    CHECK(Ask(a, "HTGR,101,HTGR,102,") == "HTSR,101,13546,HTSR,102,10849,", "4. two in one read -> two replies, in order");
    std::string twelve;
    for (int i = 0; i < 12; ++i) twelve += "HTGR,103,";
    const std::string first = Ask(a, twelve);
    CHECK(Occurrences(first, "HTSR,103,7,") == 10, "4. 12 in one read: 10 answered in this pass");
    W906_CmdServerPumpTick();
    CHECK(Occurrences(Tx(a), "HTSR,103,7,") == 2, "4. ... the other 2 in the next pass, with no new bytes");
    CHECK(Ask(a, "xyz\r\nHTGR,109,\r\n") == "HTSR,109,Off Line,", "4. noise before a head + CRLF: the command is answered");
    const W906CmdServerPumpStats s0 = W906_CmdServerPumpGetStats();
    CHECK(Ask(a, "HTSET,403," + std::string(3000, 'x')).empty(), "4. a 3010-byte half command: no reply");
    const W906CmdServerPumpStats s1 = W906_CmdServerPumpGetStats();
    CHECK(s1.overflows == s0.overflows + 1 && TcpLog().find("#Overflow#") != std::string::npos
          && TcpLog().find("3010 bytes dropped (limit 2048)") != std::string::npos, "4. ... #Overflow# 3010 bytes dropped (limit 2048)");
    CHECK(Ask(a, "HTGR,109,") == "HTSR,109,Off Line,", "4. the next command is answered (the connection stays)");
    CHECK(Ask(a, std::string(4096, '#') + "HTGR,109,") == "HTSR,109,Off Line," && TcpLog().find("4096 bytes dropped") != std::string::npos,
          "4. 4096 bytes of garbage: #Discard#, the command after it answered");
    CHECK(fMain->TCPCommandServer->Socket->ActiveConnections == 1, "4. still connected");

    // ---- 5. HTSET,333 / 334 -----------------------------------------------------------------------------------------
    std::printf("\n-- 5. HTSET,333 / 334\n");
    CHECK(Ask(a, "HTSET,333,") == "HTSR,333,NG,", "5. 333 on a non-TeraPower machine without bRemoteLotStart -> NG (golden)");
    CUSTOMER_CODE = CC_TERAPOWER;
    CHECK(Ask(a, "HTSET,333,") == "HTSR,333,NG,", "5. 333, TeraPower, HALT, seam NOT installed -> NG (never the base TfMain::Start)");
    W906_RemoteRun.Start = &StubStart;
    W906_RemoteRun.Pause = &StubPause;
    CHECK(Ask(a, "HTSET,333,") == "HTSR,333,OK," && g_startCalls == 1 && g_startFunc == "TCP Command Start!!", "5. installed -> Start(\"TCP Command Start!!\"), OK");
    g_startAllowed = false;
    CHECK(Ask(a, "HTSET,333,") == "HTSR,333,NG," && g_startCalls == 1, "5. refused (manual teach) -> NG, Start not called");
    g_startAllowed = true;
    fMain->palMainStatus->Caption = "RUN";
    CHECK(Ask(a, "HTSET,333,") == "HTSR,333,NG," && g_startCalls == 1, "5. not HALT -> NG (golden), Start not called");
    fMain->palMainStatus->Caption = "HALT";
    CHECK(Ask(a, "HTSET,334,") == "HTSR,334,NG," && g_pauseCalls == 0, "5. 334 while not running -> NG (golden)");
    SystemStart = true;
    CHECK(Ask(a, "HTSET,334,") == "HTSR,334,OK," && g_pauseCalls == 1 && g_pauseFunc == "TCP Command Pause", "5. 334 running -> Pause(\"TCP Command Pause\"), OK");
    W906_RemoteRun.Pause = 0;
    CHECK(Ask(a, "HTSET,334,") == "HTSR,334,NG,", "5. 334 with no Pause installed -> NG");
    SystemStart = false;
    W906_RemoteRun.Start = 0;
    CUSTOMER_CODE = 868;
    CHECK(fMain->W906_PauseCallCount == pause0, "5. the base TfMain::Pause was never called");

    // ---- 6. HTSET,702 -----------------------------------------------------------------------------------------------
    std::printf("\n-- 6. HTSET,702\n");
    const W906CmdServerPumpStats s2 = W906_CmdServerPumpGetStats();
    CHECK(Ask(a, "HTSET,702,abc,LOTE,").empty(), "6. HTSET,702,abc: StrToInt throws -> no reply (as golden)");
    const W906CmdServerPumpStats s3 = W906_CmdServerPumpGetStats();
    CHECK(s3.caught == s2.caught + 1 && TcpLog().find("#Exception#") != std::string::npos, "6. ... caught at the pump boundary, #Exception# logged");
    CHECK(Ask(a, "HTGR,109,") == "HTSR,109,Off Line,", "6. the tick thread survived: the next command is answered");
    const bool tt1 = (fSCKART->iTesterType == 1);
    const std::string r702 = Ask(a, "HTSET,702,12,LOTX,");
    CHECK(r702 == (tt1 ? "HTSR,702,OK,HTSR,702,OK," : "HTSR,702,OK,"), "6. HTSET,702,12,LOTX -> OK (twice when iTesterType==1, golden)");
    CHECK(fSCKART->iLotCount == 12 && fSCKART->sLotID == "LOTX", "6. ... lot 12 / LOTX");

    // ---- 6b. HTSET,701 both arms (AI(W906-W213) 20261010 (St02-E), W-213 step 4: golden 913 Command.cpp:13780-13838) -------------------------
    std::printf("\n-- 6b. HTSET,701\n");
    {
        const int it0 = fSCKART->iTesterType, n0 = W906_ClearBarcodeListCallCount;
        fSCKART->iTesterType = 0;
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_R);
        LastSet.iSCKART_RTUnitCount = 9;  fSCKART->iInputJamCnt = 7;  fSCKART->iOutputJamCnt = 8;  LastSet.lShuttleCount = 77;  fSCKART->iWaitGPIBLotR = 0;
        const std::string r1 = Ask(a, "HTSET,701,");
        CHECK(r1 == "HTSR,701,OK," && fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_W && fSCKART->iWaitGPIBLotR == 3 && fSCKART->iInputCount == 9 &&
              LastSet.iSCKART_RTUnitCount == 0 && fSCKART->iInputJamCnt == 0 && fSCKART->iOutputJamCnt == 0 && LastSet.lShuttleCount == 0 &&
              W906_ClearBarcodeListCallCount == n0 + 1,
              "6b. HTSET,701 (iTesterType 0, status R): HTSR,701,OK, -- RT counts cleared, R -> W, iWaitGPIBLotR 3, the 2DID list cleared through btClearBarcodeListClick");
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_A);
        fSCKART->iCurrentFlexARTStep = 0;
        const std::string r2 = Ask(a, "HTSET,701,");
        CHECK(r2 == "HTSR,701,NG," && fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_W && fSCKART->iCurrentFlexARTStep == 10,
              "6b. HTSET,701 (iTesterType 0, status A): HTSR,701,NG, -- status W, iCurrentFlexARTStep 10 (golden :13833-13838)");
        fSCKART->iTesterType = it0;
    }

    // ---- 8. R1 --------------------------------------------------------------------------------------------------------
    std::printf("\n-- 8. R1: long replies in full\n");
    const int oldBins = iTestBinCount;
    iTestBinCount = 255;
    std::string want = "HTSR,255,";
    for (int i = 0; i < 255; ++i)
    {
        BinSelect[0].iCatDataT3Pos[i] = 1;
        BinSelect[0].bConsFail[i] = false;
        LastSet.iBinData32[2][i] = i;
        char t[32];
        std::snprintf(t, sizeof(t), "P-Bin%d=%d,", i, i);
        want += t;
    }
    const std::string r255 = Ask(a, "HTGR,255,");
    CHECK(want.size() > 500 && r255 == want, "8. HTGR,255 with 255 bins (> 500 bytes) is sent whole (golden cBuffer[500] overflowed)");
    iTestBinCount = oldBins;
    // AI(W906-W10-SAFE) 20260928 (St02-E): St01 09:0x -- this used 1500 characters and expected OK, but golden's own check is
    //   Length() < 1023 (Command.cpp:17220 = golden 906_0625_Steven HTSET,403), so 1500 answers Fail.  R1 is shown with 1000
    //   characters: a 1013-byte OK reply (> golden's cBuffer[500]) sent whole.
    const std::string text(1000, 'z');
    CHECK(Ask(a, "HTSET,403," + text + ",") == "HTSR,403," + text + ",OK," && W906_ShowMyMessage_LastS1 == AnsiString(text.c_str()),
          "8. HTSET,403 with 1000 characters reaches ShowMyMessage and the 1013-byte reply is sent whole");
    const std::string text1500(1500, 'y');
    CHECK(Ask(a, "HTSET,403," + text1500 + ",") == "HTSR,403,Fail," && W906_ShowMyMessage_LastS1 == AnsiString(text1500.c_str()),
          "8. 1500 characters -> Fail (golden < 1023), and golden still shows the message");
    CHECK(Ask(a, "HTSET,403," + std::string(1100, 'w') + ",") == "HTSR,403,Fail,", "8. 1100 characters -> golden's own <1023 check: Fail");

    // ---- 9. the real-socket loopback --------------------------------------------------------------------------------
    Section9Loopback();

    // ---- 10. HTSET,322 / 323 (R2) ---------------------------------------------------------------------------------
    std::printf("\n-- 10. HTSET,322 / 323\n");
    // AI(W906-I126) 20261001 (St02-E): INBOX 126 / NB2 R123 s3.  HTSET,322 / 323 run golden fBinSel->spbSaveClick, whose
    //   SaveOther writes about 20 keys per Category for iTestBinCount bins, one INI write each (cBinSel.cpp:1292-1351):
    //   HTSET,322,5 alone took 58.8 s under gate load.  Two bins here (section 8 sets 255 the same way), restored at the
    //   end of the section.  The save itself is now checked in the files: iTestRunMode 0 = eBinRT, so 322's
    //   ChangeActivePageIndex picks page 1 = BinasgnOff.Data (cBinSel.cpp:507-512, :1026-1043); 323 with
    //   LastSet.iRunStartMode 0 picks page 0 = Binasgn.Data (Command.cpp:17062-17063, cBinSel.cpp:1069-1073).  Both files
    //   are removed first, so only this section's saves can satisfy the checks: without 322's save (Command.cpp:17055)
    //   the first file check fails.
    const int oldBins10 = iTestBinCount;
    iTestBinCount = 2;
    const std::string binOff = g_root + "\\Data\\W10R\\BinasgnOff.Data";
    const std::string binFt  = g_root + "\\Data\\W10R\\Binasgn.Data";
    ::DeleteFileA(binOff.c_str());
    ::DeleteFileA(binFt.c_str());
    CHECK(Ask(a, "HTSET,322,5,") == "HTSR,322,DoubleContact_On,5,", "10. HTSET,322,5 -> golden reply");
    CHECK(Key(binOff, "Category0", "Scan") != "<none>" && Key(binOff, "Category1", "Scan") != "<none>"
          && Key(binOff, "Category2", "Scan") == "<none>",
          "10. HTSET,322,5 saved BinasgnOff.Data: [Category0] / [Category1] Scan written, [Category2] not (two bins)");
    Send(a, "HTSET,322,");
    W906_CmdServerPumpTick();
    CHECK(Tx(a).empty(), "10. \"HTSET,322,\" alone: waits (golden would have run it on bin 0)");
    CHECK(Ask(a, "6,") == "HTSR,322,DoubleContact_On,6,", "10. + \"6,\" -> index 6");
    CHECK(Ask(a, "HTSET,322,300,") == "HTSR,322,DoubleContact_On,300," && TcpLog().find("HTSET,322 index 300 out of range 0..255") != std::string::npos,
          "10. 322 index 300: golden's reply, #Ignore# logged, no write past iDBContact[256]");
    CHECK(Ask(a, "HTSET,322,-1,") == "HTSR,322,DoubleContact_On,-1," && TcpLog().find("HTSET,322 index -1 out of range") != std::string::npos, "10. 322 index -1 -> #Ignore#");
    CHECK(Ask(a, "HTSET,323,5,") == "HTSR,323,DoubleContact_Off,5,", "10. HTSET,323,5 -> golden reply");
    CHECK(Key(binFt, "Category1", "Scan") != "<none>", "10. HTSET,323,5 saved Binasgn.Data: [Category1] Scan written");
    CHECK(Ask(a, "HTSET,323,256,") == "HTSR,323,DoubleContact_Off,256," && TcpLog().find("HTSET,323 index 256 out of range 0..255") != std::string::npos,
          "10. 323 index 256 -> #Ignore#");
    CHECK(Occurrences(TcpLog(), "#Ignore#") == 3, "10. three #Ignore# lines");
    iTestBinCount = oldBins10;

    // ---- 1 (end). the open-error branch -------------------------------------------------------------------------------
    std::printf("\n-- 1b. HanderTcpIp open error\n");
    const int smm0 = W906_ShowMyMessage_Count;
    fMain->TCPCommandServer->SimFailNextOpen();
    fMain->HanderTcpIp();
    CHECK(!fMain->TCPCommandServer->Active && W906_ShowMyMessage_Count == smm0 + 1 && W906_ShowMyMessage_LastS1 == "Socket Server Open Error!!"
          && TcpLog().find("Socket Server Open Error!!") != std::string::npos, "1. SimFailNextOpen -> golden's catch: \"Socket Server Open Error!!\"");
    CHECK(fMain->TCPCommandServer->Socket->ActiveConnections == 0, "1. Close() dropped the clients (golden: every 7016 connection goes)");
    fMain->HanderTcpIp();
    CHECK(fMain->TCPCommandServer->Active && fMain->TeraTCPResultServer->Active, "1. the next HanderTcpIp opens both again");

    // ---- 7. nothing outside the sandbox -----------------------------------------------------------------------------
    std::printf("\n-- 7. the machine's files\n");
    for (int i = 0; i < kMachine; ++i)
    {
        char msg[400];
        std::snprintf(msg, sizeof(msg), "7. %s: same size and write time as before", machine[i]);
        CHECK(SameStamp(before[i], StampOf(machine[i])), msg);
    }
    CHECK(ServersStaySim(), "7. the fMain servers never had a real socket (never 7016 / 7017 on this machine)");

    W906_CmdServerPumpShutdown(); CloseGeneralIniFile();   //AI(W906-MERGE-0929) 20260929: pair of :385
    CUSTOMER_CODE = oldCC;
    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
        RemoveTree(g_root);
    else
        std::printf("sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
