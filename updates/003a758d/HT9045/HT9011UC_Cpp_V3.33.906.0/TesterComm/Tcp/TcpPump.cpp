// ===========================================================================
//  TesterComm/Tcp/TcpPump.cpp -- see TcpPump.h.  AI(W906-GB-P5) 20260926.
// ===========================================================================
#include "MachineType.h"                     // SOFT_SIMULTE (W906_TcpTesterRealSocket) -- before any #ifdef
#include "TesterComm/Tcp/TcpPump.h"
#include "Interface/TesterTCP_Socket.h"   // TesterTCPSocket_*, TesterTCPSocket, TestIF_File, LastSet, OFF_LINE/ON_LINE
#include "TesterComm/UiChannel.h"
#include "WebBridge/Sync.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

enum EvType { kEvConnect, kEvDisconnect, kEvError, kEvRead };

struct Ev
{
    int type;
    TObject* sender;
    TCustomWinSocket* socket;
    TErrorEvent error;
    int code;
};

webbridge::WbMutex g_mu;
std::vector<Ev> g_q;
unsigned long g_replayed = 0;
bool g_inited = false;
bool g_wasOn = false;

struct TimerSlot
{
    bool wasEnabled;
    DWORD last;
    TimerSlot() : wasEnabled(false), last(0) {}
};
TimerSlot g_tConnect, g_tProcess;
const DWORD kConnectIntervalMs = 1000;   // TesterTCP.dfm TimerTCPIPConnect: Interval not stored -> VCL default 1000
const DWORD kProcessIntervalMs = 1;      // TesterTCP.dfm TimerProcessTCPData: Interval = 1
DWORD g_lastUi = 0;

void Push(const Ev& e)
{
    webbridge::WbGuard g(g_mu);
    g_q.push_back(e);
}

// VCL TTimer: Enabled false->true restarts the period; Interval elapsed -> fire once (no catch-up).
bool Due(TimerSlot& s, bool enabled, DWORD interval, DWORD now)
{
    if (!enabled)
    {
        s.wasEnabled = false;
        return false;
    }
    if (!s.wasEnabled)
    {
        s.wasEnabled = true;
        s.last = now;
        return false;
    }
    if (now - s.last < interval)
        return false;
    s.last = now;
    return true;
}

std::string Q(const AnsiString& a) { return testercomm::JsonEscape(std::string(a.c_str())); }

std::string Snapshot(bool on)
{
    std::string o = "{\"up\":";
    o += on ? "true" : "false";
    o += ",\"connected\":";
    o += TesterTCPSocket.bConnectOK ? "true" : "false";
    o += ",\"connecting\":";
    o += TesterTCPSocket.bConnect ? "true" : "false";
    o += ",\"error\":";
    o += TesterTCPSocket.bTCPError ? "true" : "false";
    o += ",\"errorMessage\":" + Q(TesterTCPSocket.ErrorMessage);
    o += ",\"address\":" + Q(TestIF_File.asTester_Address);
    char b[32];
    std::snprintf(b, sizeof(b), "%d", TestIF_File.iTester_Port);
    o += ",\"port\":";
    o += b;
    o += ",\"lastReceive\":" + Q(TesterTCPSocket.sTCPIPRecevieData);
    o += ",\"sites\":[";
    for (int i = 0; i < 32; ++i)
    {
        if (i)
            o += ',';
        std::snprintf(b, sizeof(b), "%d", TesterTCPSocket.cbSimulateBin[i].ItemIndex);
        o += "{\"site\":" + Q(TesterTCPSocket.plSite[i].Caption) + ",\"ocr\":" + Q(TesterTCPSocket.labOcr[i].Caption) +
             ",\"bin\":" + b + "}";
    }
    o += "],\"receive\":[";
    TStringList* l = TesterTCPSocket.SocketTCPIPReceiveList;
    if (l)
    {
        const int n = l->GetCount();
        const int from = n > 100 ? n - 100 : 0;
        for (int i = from; i < n; ++i)
        {
            if (i > from)
                o += ',';
            o += Q(l->GetString(i));
        }
    }
    o += "]}";
    return o;
}

}  // namespace

void W906_TcpPumpInit(bool bRealSocket)
{
    if (g_inited)
        return;
    g_inited = true;
    TesterTCPSocket_Init();   // golden .dfm event bindings (TesterTCP_Socket.cpp:185)
    // ...then route them through the queue: the shim fires on its own thread, golden handled them on the main one.
    TesterTCPSocket_ClientSocket->OnConnect = [](TObject* s, TCustomWinSocket* k) {
        Ev e = { kEvConnect, s, k, eeGeneral, 0 };
        Push(e);
    };
    TesterTCPSocket_ClientSocket->OnDisconnect = [](TObject* s, TCustomWinSocket* k) {
        Ev e = { kEvDisconnect, s, k, eeGeneral, 0 };
        Push(e);
    };
    TesterTCPSocket_ClientSocket->OnError = [](TObject* s, TCustomWinSocket* k, TErrorEvent ev, int& code) {
        Ev e = { kEvError, s, k, ev, code };
        Push(e);
        code = 0;   // golden ClientSocket_TCPIPError always ends with ErrorCode=0 (no VCL exception); the shim reads it now
    };
    TesterTCPSocket_ClientSocket->OnRead = [](TObject* s, TCustomWinSocket* k) {
        Ev e = { kEvRead, s, k, eeGeneral, 0 };
        Push(e);   // the bytes stay in the socket's buffer; golden's OnRead pulls them with ReceiveText()
    };
    // AI(W906-H008) 20261001 (St02-E): H-008 -- POLLED: Open() never blocks, Poll() (Tick step 0) fires the events on this thread.
    //   SetSimMode(false) takes effect at the next Open() (the socket is not connected yet here).
    TesterTCPSocket_ClientSocket->SetPolled(true);
    TesterTCPSocket_ClientSocket->SetSimMode(!bRealSocket);
    std::printf("[H-008] TCP tester socket: %s\n", bRealSocket ? "REAL (polled)" : "SIM");
}

void W906_TcpPumpTick()
{
    if (!g_inited)
        return;

    // 0) AI(W906-H008) 20261001 (St02-E): the POLLED socket -- finish a pending connect, read, notice a close (H-008).
    //    Its events land in the queue below and are replayed in this same tick.
    TesterTCPSocket_ClientSocket->Poll();

    // 1) socket events, in arrival order, on this (Handler) thread
    std::vector<Ev> q;
    {
        webbridge::WbGuard g(g_mu);
        q.swap(g_q);
    }
    for (size_t i = 0; i < q.size(); ++i)
    {
        Ev& e = q[i];
        switch (e.type)
        {
        case kEvConnect:    TesterTCPSocket_OnConnect(e.sender, e.socket); break;
        case kEvDisconnect: TesterTCPSocket_OnDisconnect(e.sender, e.socket); break;
        case kEvError:      TesterTCPSocket_OnError(e.sender, e.socket, e.error, e.code); break;
        case kEvRead:       TesterTCPSocket_OnRead(e.sender, e.socket); break;
        default: break;
        }
        ++g_replayed;
    }

    // 2) golden's enable rule for the two timers
    const bool on = (TestIF_File.iTestType == TCP_IP_MODE && LastSet.iTester == ON_LINE);
    if (g_wasOn && !on)
    {
        // golden cTesterIF.cpp:614-617 / main.cpp:29774-29777: timers off and ClientSocket_TCPIP->Close()
        TesterTCPSocket.bTimerTCPIPConnectEnabled = false;
        TesterTCPSocket.bTimerProcessTCPDataEnabled = false;
        TesterTCPSocket_ClientSocket->Close();
#if 0 // TODO(W906-GB-P5): fLotInfo->labTCPIPSimulate is not a V906 TfLotInfo member (golden main.cpp:29777 / cTesterIF.cpp:587)
        fLotInfo->labTCPIPSimulate->Visible = (TestIF_File.iTestType == TCP_IP_MODE);
#endif
    }
    else if (on)
    {
        TesterTCPSocket.bTimerTCPIPConnectEnabled = true;     // golden main.cpp:11323 / cTesterIF.cpp:591
        TesterTCPSocket.bTimerProcessTCPDataEnabled = true;   // :11324 / :592
    }
    g_wasOn = on;

    // 3) the timers
    const DWORD now = ::GetTickCount();
    if (Due(g_tConnect, TesterTCPSocket.bTimerTCPIPConnectEnabled, kConnectIntervalMs, now))
        TesterTCPSocket_TimerTCPIPConnectTimer();
    if (Due(g_tProcess, TesterTCPSocket.bTimerProcessTCPDataEnabled, kProcessIntervalMs, now))
        TesterTCPSocket_TimerProcessTCPDataTimer();

    // 4) the page's TCP/IP tab (P7)
    if (now - g_lastUi >= 200)
    {
        g_lastUi = now;
        testercomm::UiChannel::Instance().Publish("tcpip", Snapshot(on));
    }
}

unsigned long W906_TcpPumpEvents() { return g_replayed; }

// AI(W906-H008) 20261001 (St02-E): H-008 policy, here (not TesterCommWiring) so a narrow ctest can link it (E2 review n6).
//   SIM build: the tester client stays SIM (St02-M 20261001: W906_SIM_TCP_SERVERS covers 7016 / 7017 only, W58-4).
//   SHIP: real, unless HT9045_TCPCMD_SIM is exactly "1" (the same test TesterCommWiring.cpp applies to 7016 / 7017).
bool W906_TcpTesterRealSocket()
{
#ifdef SOFT_SIMULTE
    return false;
#else
    const char* e = std::getenv("HT9045_TCPCMD_SIM");
    return !(e && e[0] == '1' && e[1] == '\0');
#endif
}
