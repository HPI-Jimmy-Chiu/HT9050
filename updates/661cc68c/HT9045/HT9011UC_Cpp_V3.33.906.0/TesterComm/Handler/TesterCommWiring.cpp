// ===========================================================================
//  TesterComm/Handler/TesterCommWiring.cpp -- see TesterCommWiring.h.  AI(W906-GB-P3) 20260926.
// ===========================================================================
#include "TesterComm/Handler/TesterCommWiring.h"
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/Handler/HandlerGpibAux.h"   // AI(W906-GB-P6) 20260926: 2A + Q2(a)
#include "TesterComm/TesterCommHub.h"
#include "TesterComm/UiChannel.h"
#include "TesterComm/Gpib/GpibEngine.h"
#include "TesterComm/Rs232/Rs232Engine.h"
#include "TesterComm/Tcp/TcpPump.h"
#include "forms/fMain.h"            // W906_TesterForward (P2b install seat)
#include "cmydef.h"                 // InitialOK / SystemStart (P2f tick guard)

#include <cctype>
#include <cstdlib>
#include <string>

namespace {

bool g_inited = false;
DWORD g_lastConnect = 0;
const DWORD kConnectPeriodMs = 1000;   // golden main.dfm Timer2 (Interval default 1000) -> ProcessHVisionConnect

std::string UrlDecode(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i];
        if (c == '+')
            o += ' ';
        else if (c == '%' && i + 2 < s.size() && std::isxdigit((unsigned char)s[i + 1]) &&
                 std::isxdigit((unsigned char)s[i + 2]))
        {
            o += static_cast<char>(std::strtol(s.substr(i + 1, 2).c_str(), 0, 16));
            i += 2;
        }
        else
            o += c;
    }
    return o;
}

std::string QueryParam(const std::string& query, const char* name)
{
    const std::string key = std::string(name) + "=";
    size_t p = 0;
    while (p < query.size())
    {
        size_t e = query.find('&', p);
        if (e == std::string::npos)
            e = query.size();
        if (query.compare(p, key.size(), key) == 0)
            return UrlDecode(query.substr(p + key.size(), e - p - key.size()));
        p = e + 1;
    }
    return std::string();
}

bool KnownKey(const std::string& k) { return k == "gpib" || k == "rs232" || k == "tcpip"; }
const unsigned kPostRepeatWindowMs = 400;   // AI(W906-GB-P7) 20260926: same value as the WebSocket same-key guard

//AI(W906-GB-P2b) 20260926: TfMain's golden tester members forward here (forms/fMain.h W906_TesterForward).
//   fTesterSide is read at call time, so a call after Shutdown (table cleared first) or before Init is a no-op.
void FwdSendCmd(int CMD)                            { if (fTesterSide) fTesterSide->SendMSG_CMD(CMD); }
void FwdSendCmdMsg(int CMD, AnsiString Message)     { if (fTesterSide) fTesterSide->SendMSG_CMD(CMD, Message); }
bool FwdRunTestProgram(bool bNeedTest, bool* bSite) { return fTesterSide ? fTesterSide->RunTestProgram(bNeedTest, bSite) : false; }
void FwdCloseGpibProgram(AnsiString Src)            { if (fTesterSide) fTesterSide->CloseGpibProgram(Src); }
void FwdSendTestMode()                              { if (fTesterSide) fTesterSide->SendMSG_TestMode(); }
void FwdWakeupGPIB(AnsiString FuncName)             { if (fTesterSide) fTesterSide->WakeupGPIB(FuncName); }
void FwdDeviceMapSRQ(int iStatus)                   { if (fTesterSide) fTesterSide->SendMSG_CMD_DeviceMapSRQ(iStatus); }
bool FwdBridgeFound()                               { return fTesterSide ? fTesterSide->bFind : false; }   // golden fMain->bFind (P2b)

}  // namespace

void W906_TesterCommInit()
{
    if (g_inited)
        return;
    //AI(W906-GB-P3) 20260926: default = golden -- from here on ProcessHVisionConnect (Tick, every 1000 ms) launches the
    //   bridge like golden launches H9046_32GPIB.exe / RS232Standard.exe at boot: the engine thread starts, writes its
    //   golden logs, opens its GPIB card / COM ports.  HT9045_TESTERCOMM=0 opts a run out entirely (seat not installed,
    //   no engine, Tick/Poll/Shutdown inert, the page shows offline) -- for SIM regression runs that must not see a bridge.
    const char* optOut = std::getenv("HT9045_TESTERCOMM");
    if (optOut && optOut[0] == '0' && optOut[1] == '\0')
        return;
    g_inited = true;
    testercomm::TesterCommHub& hub = testercomm::TesterCommHub::Instance();
    // golden WakeupGPIB (main.cpp:18545-18553): RS232_MODE, or TTL_MODE with TTL_CARD_TYPE 2/3, launches
    // RS232Standard.exe; every other type launches H9046_32GPIB.exe (TCP_IP_MODE included -- golden runs the GPIB
    // bridge alongside the Handler's built-in TCP channel).  TTL_MODE with TTL_CARD_TYPE<2 (the direct DIO-card
    // branch) is not used on the new machines (user ruling 20260926), so TTL_MODE always gets RS232Standard here.
    hub.RegisterFactory(testercomm::kTestTypeGpib,  &gpibbridge::GpibEngine::Create);
    hub.RegisterFactory(testercomm::kTestTypeTcpIp, &gpibbridge::GpibEngine::Create);
    hub.RegisterFactory(testercomm::kTestTypeRs232, &rs232std::Rs232Engine::Create);
    hub.RegisterFactory(testercomm::kTestTypeTtl,   &rs232std::Rs232Engine::Create);
    if (fTesterSide == NULL)
        fTesterSide = new THandlerTesterSide();
    fTesterSide->Attach(&hub);
    W906_TesterForward.SendMSG_CMD              = &FwdSendCmd;          //AI(W906-GB-P2b) 20260926: install seat
    W906_TesterForward.SendMSG_CMD_Msg          = &FwdSendCmdMsg;
    W906_TesterForward.RunTestProgram           = &FwdRunTestProgram;
    W906_TesterForward.CloseGpibProgram         = &FwdCloseGpibProgram;
    W906_TesterForward.SendMSG_TestMode         = &FwdSendTestMode;
    W906_TesterForward.WakeupGPIB               = &FwdWakeupGPIB;
    W906_TesterForward.SendMSG_CMD_DeviceMapSRQ = &FwdDeviceMapSRQ;
    W906_TesterForward.BridgeFound              = &FwdBridgeFound;
    g_lastConnect = ::GetTickCount() - kConnectPeriodMs;   // first ProcessHVisionConnect on the first tick
    W906_GpibAuxEnable(true);   //AI(W906-GB-P6) 20260926: 2A + Q2(a) -- only the composition root (wb_serve) reads / seeds the recipe
    W906_TcpPumpInit();   // P5: the Handler's built-in TCP/IP tester channel (golden TfTesterTCP)
}

void W906_TesterCommPoll()
{
    if (!g_inited)
        return;
    testercomm::TesterCommHub::Instance().PollHandler();
}

void W906_TesterCommTick()
{
    if (!g_inited)
        return;
    testercomm::TesterCommHub::Instance().PollHandler();
    W906_TcpPumpTick();   // P5: TCP/IP socket events + TimerTCPIPConnect / TimerProcessTCPData, on this thread as golden
    const DWORD now = ::GetTickCount();
    if (fTesterSide && now - g_lastConnect >= kConnectPeriodMs)
    {
        g_lastConnect = now;
        fTesterSide->ProcessHVisionConnect();   // golden Timer2Timer (main.cpp:21853): finds / wakes the bridge
        testercomm::TesterCommHub::Instance().PollHandler();
    }
    //AI(W906-GB-P2f) 20260926: golden Timer2Timer :22089-22234 (Handler -> bridge settings sync) runs on every Timer2
    //   tick after its InitialOK / SystemStart==false guards (:21495, :22080); change-driven, so cheap per tick.
    if (fTesterSide && InitialOK && !SystemStart)
        fTesterSide->SyncBridgeSettings();
    THandlerTesterSide::PublishSettings();   //AI(W906-GB-P6) 20260926: recipe / config changes reach the engine (HandlerSettings.h)
    //AI(W906-GB-P6) 20260926: 2A + Q2(a) -- the recipe's RS232 values changed (machine stopped): close the bridge; WakeupGPIB
    //   relaunches it and its LoadSetupData takes the new framing (golden CheckRs232StandardIni -> CloseGpibProgram).
    if (fTesterSide && W906_GpibAuxNeedsRestart(fTesterSide->bFind))
        fTesterSide->CloseGpibProgram("P6 2A: GPIB extra RS232 port follows the recipe");
}

void W906_TesterCommShutdown()
{
    if (!g_inited)
        return;
    W906_TesterForward = W906_TesterForwardTable();     //AI(W906-GB-P2b) 20260926: uninstall first -- fMain->X is a no-op again
    W906_GpibAuxEnable(false);                          //AI(W906-GB-P6) 20260926
    testercomm::TesterCommHub::Instance().Shutdown();   // engine Stop -> golden FormClose (logs, last-data files)
    if (fTesterSide)
    {
        THandlerTesterSide* s = fTesterSide;
        fTesterSide = NULL;
        delete s;
    }
    g_inited = false;
}

bool W906_TesterCommHttp(const std::string& method, const std::string& path, const std::string& query,
                         bool allowCmd, int* status, std::string* contentType, std::string* body)
{
    const std::string base = "/api/testercomm";
    if (path.compare(0, base.size(), base) != 0 || (path.size() > base.size() && path[base.size()] != '/'))
        return false;
    *contentType = "application/json; charset=utf-8";
    std::string key = path.size() > base.size() + 1 ? path.substr(base.size() + 1) : std::string();
    if (!key.empty() && key[key.size() - 1] == '/')
        key.erase(key.size() - 1);

    if (key.empty())
    {
        if (method != "GET" && method != "HEAD")
        {
            *status = 405;
            *body = "{\"error\":\"GET only\"}";
            return true;
        }
        *status = 200;
        *body = "{\"keys\":[\"gpib\",\"rs232\",\"tcpip\"]}";
        return true;
    }
    if (!KnownKey(key))
    {
        *status = 404;
        *body = "{\"error\":\"unknown key\"}";
        return true;
    }
    if (method == "GET" || method == "HEAD")
    {
        std::string json;
        unsigned long seq = 0;
        if (!testercomm::UiChannel::Instance().Read(key, &json, &seq))
        {
            *status = 404;
            *body = "{\"error\":\"no snapshot yet (engine not running)\"}";
            return true;
        }
        *status = 200;
        *body = json;
        return true;
    }
    if (method == "POST")
    {
        if (!allowCmd)
        {
            *status = 403;
            *body = "{\"error\":\"read-only server\"}";
            return true;
        }
        const std::string cmd = QueryParam(query, "cmd");
        if (cmd.empty() || cmd.size() > 512)
        {
            *status = 400;
            *body = "{\"error\":\"cmd missing or too long\"}";
            return true;
        }
        //AI(W906-GB-P7) 20260926: same-key time window, like the WebSocket 400 ms guard (which does not cover this
        //   HTTP path).  "click btnManualStart" / "click btManualTest" / "click btnDiagZip" / the toggles fire an action
        //   per call, so an exact repeat within 400 ms is dropped; set-value commands (check / combo / edit / site) are
        //   idempotent, so dropping their exact repeat changes nothing.
        if (!testercomm::UiChannel::Instance().Post(key, cmd, kPostRepeatWindowMs))
        {
            *status = 200;
            *body = "{\"queued\":false,\"reason\":\"repeat within 400 ms\"}";
            return true;
        }
        *status = 202;
        *body = "{\"queued\":true}";
        return true;
    }
    *status = 405;
    *body = "{\"error\":\"GET or POST\"}";
    return true;
}
