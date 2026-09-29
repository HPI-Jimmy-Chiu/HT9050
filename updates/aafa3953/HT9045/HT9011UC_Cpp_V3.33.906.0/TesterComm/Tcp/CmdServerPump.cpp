// ===========================================================================
//  TesterComm/Tcp/CmdServerPump.cpp -- see CmdServerPump.h.  AI(W906-W10) 20260927 (St02-E).
// ===========================================================================
#include "TesterComm/Tcp/CmdServerPump.h"
#include "TesterComm/Tcp/TcpCmdFramer.h"
#include "forms/fMain.h"      // fMain, TCPCommandServer / TeraTCPResultServer, HanderTcpIp, TCPIPCommunicationLog,
                              // W906_TcpCmdRunFrame
#include "CosFunction.h"      // CosFunction.bEnableHandlerResultServer
#include "WebBridge/Sync.h"   // WbMutex / WbGuard (the stats)

#include <windows.h>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

namespace {

bool g_inited = false;
bool g_formShown = false;
DWORD g_tickThread = 0;
TServerSocket* g_cmd = 0;                  // fMain->TCPCommandServer at Init (7016)
TServerSocket* g_res = 0;                  // fMain->TeraTCPResultServer at Init (7017)
TSocketNotifyEvent g_goldenRead;           // the ctor's golden dfm bindings, put back by Shutdown
TSocketNotifyEvent g_goldenDisconnect;
tcpcmd::Framer g_framer;
W906CmdServerPumpStats g_stats = { 0, 0, 0, 0, 0, 0 };

// AI(W906-ELA-REV) 20260928 (St02-E): the tick thread counts, W906_CmdServerPumpGetStats may be called from any thread --
//   every write, the reset and the copy take this lock, so a copy is one consistent set of six.  Built on first use and never
//   freed, so no static-init or exit-order question arises.
webbridge::WbMutex& StatsMu()
{
    static webbridge::WbMutex* m = new webbridge::WbMutex;
    return *m;
}
void Bump(unsigned long W906CmdServerPumpStats::*f)
{
    webbridge::WbGuard g(StatsMu());
    ++(g_stats.*f);
}

// one line in golden's TCP/IP log, same column layout as golden's "[%4d][%4d][ #Receive#  ] %s"
void Log(TCustomWinSocket* k, const char* tag, const AnsiString& text)
{
    if (fMain == 0)
        return;
    AnsiString s;
    s.sprintf("[%4d][%4d][ %-11s] %s", k ? k->SocketHandle : 0, k ? k->LocalPort : 0, tag, text);
    fMain->TCPIPCommunicationLog(s);
}

void LogEvents(TCustomWinSocket* k, const std::vector<tcpcmd::Event>& ev)
{
    for (size_t i = 0; i < ev.size(); ++i)
    {
        AnsiString t;
        switch (ev[i].kind)
        {
        case tcpcmd::kDiscard:
            Bump(&W906CmdServerPumpStats::discards);
            t.sprintf("%u bytes dropped (not a HTGR, / HTSET, command)", (unsigned)ev[i].bytes);
            Log(k, "#Discard#", t);
            break;
        case tcpcmd::kIncomplete:
            Bump(&W906CmdServerPumpStats::incompletes);
            t.sprintf("%u bytes dropped (a command cut short by the next HTGR, / HTSET,)", (unsigned)ev[i].bytes);
            Log(k, "#Incomplete#", t);
            break;
        case tcpcmd::kOverflow:
            Bump(&W906CmdServerPumpStats::overflows);
            t.sprintf("%u bytes dropped (limit %u)", (unsigned)ev[i].bytes, (unsigned)tcpcmd::kCap);
            Log(k, "#Overflow#", t);
            break;
        }
    }
}

// OnClientRead of 7016 (R2): take every waiting byte, like golden's ReceiveBuf(..., ReceiveLength()), but into this
// connection's buffer instead of char[100].  Commands are handed over by Pass(), not here.
void OnCmdRead(TObject* /*Sender*/, TCustomWinSocket* k)
{
    if (k == 0)
        return;
    const AnsiString a = k->ReceiveText();
    if (a.Length() > 0)
        g_framer.Append(k, a.c_str(), static_cast<size_t>(a.Length()));
}

void RunOne(TCustomWinSocket* k, const std::string& frame)
{
    Bump(&W906CmdServerPumpStats::frames);
    try
    {
        W906_TcpCmdRunFrame(g_cmd, k, AnsiString(frame.data(), static_cast<int>(frame.size())));
    }
    // RULED W41 = A (Steven 0928, AI(W906-W10)): log it and send no reply, as golden.  Known source: HTSET,702 with a
    //   non-numeric quantity -> vclcompat StrToInt throws std::runtime_error (golden 906_0625_Steven Command.cpp
    //   :13821-13822; VCL's Application->HandleException showed an error box and the reply was never sent).  Here the
    //   tick thread keeps running, the reply stays unsent (as golden), and the log gets one line.
    catch (const std::exception& e)
    {
        Bump(&W906CmdServerPumpStats::caught);
        AnsiString t;
        t.sprintf("%s -- %s", e.what(), AnsiString(frame.c_str()));
        Log(k, "#Exception#", t);
    }
    catch (...)
    {
        Bump(&W906CmdServerPumpStats::caught);
        Log(k, "#Exception#", AnsiString("(unknown) -- ") + AnsiString(frame.c_str()));
    }
}

void DispatchPending()
{
    if (g_cmd == 0)
        return;
    const std::vector<TCustomWinSocket*> conns = g_cmd->Socket->Connections;   // a copy: a command may Close() / Open()
    for (size_t i = 0; i < conns.size(); ++i)
    {
        TCustomWinSocket* k = conns[i];
        for (int n = 0; n < tcpcmd::kMaxFramesPerPass; ++n)
        {
            std::string frame;
            std::vector<tcpcmd::Event> ev;
            const bool got = g_framer.Next(k, &frame, &ev);
            LogEvents(k, ev);
            if (!got)
                break;
            RunOne(k, frame);
        }
    }
}

void Pass()
{
    Bump(&W906CmdServerPumpStats::passes);
    if (!g_formShown)
    {
        g_formShown = true;
        // golden 906_0625_Steven main.cpp:10974-10977 (TfMain::FormShow): the servers open at boot, not only at START
        try
        {
            if (CosFunction.bEnableHandlerResultServer == true)
                fMain->HanderTcpIp();
        }
        catch (...)
        {
            Bump(&W906CmdServerPumpStats::caught);
            Log(0, "#Exception#", "HanderTcpIp (FormShow)");
        }
    }
    try
    {
        if (g_cmd)
            g_cmd->Poll();                 // accept / read / disconnect -> the golden events, on this thread
        if (g_res)
            g_res->Poll();
    }
    catch (...)
    {
        Bump(&W906CmdServerPumpStats::caught);
        Log(0, "#Exception#", "Poll");
    }
    DispatchPending();
}

bool OnTickThread() { return g_tickThread != 0 && ::GetCurrentThreadId() == g_tickThread; }

}  // namespace

// AI(W906-W10fix) 20260928 (St02-E): see CmdServerPump.h.  St01 20:40: TesterComm_TcpCmdServer SIGSEGV in
//   TServerSocket::IsSimMode (both members null), and WebStart.cpp:3485 would do the same on Start.
void W906_CmdServersEnsure()
{
    if (fMain != 0 && fMain->TCPCommandServer == 0 && fMain->TeraTCPResultServer == 0)
        W906_TcpServersCreate(fMain);
}

void W906_CmdServerPumpInit(bool bRealSockets)
{
    W906_CmdServersEnsure();
    if (g_inited || fMain == 0 || fMain->TCPCommandServer == 0 || fMain->TeraTCPResultServer == 0)
        return;
    g_inited = true;
    g_formShown = false;
    g_tickThread = 0;
    g_cmd = fMain->TCPCommandServer;
    g_res = fMain->TeraTCPResultServer;
    TServerSocket* const both[2] = { g_cmd, g_res };
    for (int i = 0; i < 2; ++i)
    {
        both[i]->SetPolled(true);
        if (bRealSockets)
        {
            both[i]->SetExclusiveAddr(true);   // a second listener on 7016 / 7017 fails instead of sharing the port
            both[i]->SetSimMode(false);        // takes effect at the next Open() (golden HanderTcpIp: Close / Open)
        }
    }
    // R2: the framer sits between the socket and the golden body.  The golden dfm binding (OnClientRead =
    //   TCPCommandServerClientRead, forms/fMain.cpp W906_TcpServersCreate) becomes "append here, hand over complete
    //   commands in Pass()"; a disconnect also forgets that connection's half command.
    g_goldenRead = g_cmd->OnClientRead;
    g_goldenDisconnect = g_cmd->OnClientDisconnect;
    g_cmd->OnClientRead = &OnCmdRead;
    const TSocketNotifyEvent golden = g_goldenDisconnect;
    g_cmd->OnClientDisconnect = [golden](TObject* s, TCustomWinSocket* k) {
        g_framer.Drop(k);
        if (golden)
            golden(s, k);
    };
    {
        webbridge::WbGuard g(StatsMu());
        g_stats = W906CmdServerPumpStats();
    }
}

void W906_CmdServerPumpTick()
{
    if (!g_inited)
        return;
    if (g_tickThread == 0)
        g_tickThread = ::GetCurrentThreadId();   // the first tick names the tick thread
    if (!OnTickThread())
        return;
    Pass();
}

void W906_CmdServerPumpPoll()
{
    if (!g_inited || !OnTickThread())
        return;
    Pass();
}

void W906_CmdServerPumpShutdown()
{
    if (!g_inited)
        return;
    if (g_cmd)
    {
        g_cmd->Close();                        // golden: the sockets close with the form at program exit
        g_cmd->OnClientRead = g_goldenRead;
        g_cmd->OnClientDisconnect = g_goldenDisconnect;
    }
    if (g_res)
        g_res->Close();
    g_framer.Clear();
    g_inited = false;
    g_formShown = false;
    g_tickThread = 0;
    g_cmd = 0;
    g_res = 0;
}

W906CmdServerPumpStats W906_CmdServerPumpGetStats()
{
    webbridge::WbGuard g(StatsMu());   // AI(W906-ELA-REV) 20260928: a consistent copy (see StatsMu)
    return g_stats;
}
