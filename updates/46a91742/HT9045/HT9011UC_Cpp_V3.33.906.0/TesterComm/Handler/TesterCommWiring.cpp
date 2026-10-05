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
#include "TesterComm/Tcp/CmdServerPump.h"   // AI(W906-W10) 20260927 (St02-E): the TCP command server 7016 / 7017
#include "TesterComm/Handler/HandlerTesterConnect.h"   // AI(W906-D1D7) 20260928 (St02-E): ChangeTesterConnect D1/D2/D3/D6/D7
#include "TesterComm/TesterWndSeat.h"                  // AI(W906-GB-P8) 20260928 (St02-E helper): B1 seats (TfMain::GetTTLState)
#include "LastSet.h"                // AI(W906-ELA-W48B-A6) 20260929: the one-time first-tick reset below
#include "Config.h"                 // AI(W906-ELA-W48B) 20260928: IniConfig.iN10UploadProductMethod (the TimeData tick)
#include <cctype>
#include <cstdlib>
#include <string>
#include "Automation/HanaRms_St02.h"   // AI(W906-ST02-C10) 20261002 (St02-E helper): W906_HanaRmsSetRealSocket / W906_HanaRmsPumpTick (golden 912 HANA RMS, HANARMSClient); the old blank line, nothing below moves
// AI(W906-ELA-W48B) 20260928 (St02-E helper): declared here, not by including cMyDB.h / cpublic.h -- cMyDB.h would
// redeclare RecordProcess's default argument next to canary_support.h (the forms/fMain.cpp :690-691 note).  Same
// spelling as cMyDB.h:94 (`__fastcall` is the real i686 calling convention in this tree, vclcompat/vcl_compat.h:24-54)
// and cpublic.h:50 (its default argument is left to cpublic.h; the call below passes it).
void __fastcall RecordTimeData(int iDataType);          // cMyDB.cpp:479 (golden cMyDB.cpp:339)
AnsiString GetOnlyTimeInfoByString(AnsiString asSign);  // cpublic.cpp:844 (golden cpublic.cpp)

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

bool KnownKey(const std::string& k) { return k == "gpib" || k == "rs232" || k == "tcpip" || k == "home"; }   // AI(W906-TC-SHARED) 20261001 (St02-E): + home (TesterComm/UiHome.h)
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
//AI(W906-GB-P8) 20260928 (St02-E helper): B1 -- golden this->Handle / HVisionWnd / SendMessage(HVisionWnd, WM_COPYDATA) outside TesterComm/
//   (TfMain::GetTTLState, Command.cpp), through TesterComm/TesterWndSeat.h.  The same reads THandlerTesterSide::SendMSG_CMD makes.
HWND FwdHandlerWnd()                                { return fTesterSide ? fTesterSide->HandlerWndToken() : NULL; }
HWND FwdBridgeWnd()                                 { return fTesterSide ? fTesterSide->HVisionWnd : NULL; }
void FwdSendToBridge(COPYDATASTRUCT* pcp)           { if (fTesterSide) fTesterSide->SendToBridge(pcp); }

}  // namespace
bool W906_SimTcpRealSockets(bool bRealSockets);   //AI(W906-W58-4) 20260930 (St02-E): end of file -- W58-4 SIM default for 7016 / 7017 (file scope on purpose: :30-:92 is an anonymous namespace)
void W906_TesterCommInit()
{
    //AI(W906-D1D7) 20260928 (St02-E): golden TfMain::ChangeTesterConnect D1/D2/D3/D6/D7 (HandlerTesterConnect.cpp) -- Handler
    //   rules, not the bridge, so they are installed before the HT9045_TESTERCOMM=0 opt-out below.  Idempotent.
    W906_TesterConnectRulesInstall();
    if (g_inited)
        return;
    //AI(W906-GB-P3) 20260926: default = golden -- from here on ProcessHVisionConnect (Tick, every 1000 ms) launches the
    //   bridge like golden launches H9046_32GPIB.exe / RS232Standard.exe at boot: the engine thread starts, writes its
    //   golden logs, opens its GPIB card / COM ports.  HT9045_TESTERCOMM=0 opts a run out entirely (seat not installed,
    //   no engine, Tick/Poll/Shutdown inert, the page shows offline) -- for SIM regression runs that must not see a bridge.
    W906_CmdServersEnsure();  const char* optOut = std::getenv("HT9045_TESTERCOMM");   // AI(W906-W10fix) 20260928 (St02-E): fMain's 7016 / 7017 objects exist even with the opt-out (WebStart.cpp:3483-3486 writes their Active on every Start)
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
    fTesterSide->Attach(&hub);  ht9045::W906_ESDErrorDrainHook = &THandlerTesterSide::ESDErrorDrainHook;  ht9045::W906_ESCProcessHook = &THandlerTesterSide::ProcessForESCHook;   /* AI(W906-ST02-ESC) 20261005 (St02-E): golden Timer1 :2858 ProcessForESC seat */   //AI(W906-S13) 20261001 (St02-E): the tESDError part of golden TimerESDTimer (MainTimerESD.cpp E2), installed on every TesterComm init right after fTesterSide exists (not on :119, the if body -- -Wmisleading-indentation); the hook checks fTesterSide itself (Shutdown sets it NULL)
    W906_TesterForward.SendMSG_CMD              = &FwdSendCmd;          //AI(W906-GB-P2b) 20260926: install seat
    W906_TesterForward.SendMSG_CMD_Msg          = &FwdSendCmdMsg;
    W906_TesterForward.RunTestProgram           = &FwdRunTestProgram;
    W906_TesterForward.CloseGpibProgram         = &FwdCloseGpibProgram;
    W906_TesterForward.SendMSG_TestMode         = &FwdSendTestMode;
    W906_TesterForward.WakeupGPIB               = &FwdWakeupGPIB;
    W906_TesterForward.SendMSG_CMD_DeviceMapSRQ = &FwdDeviceMapSRQ;
    W906_TesterForward.BridgeFound              = &FwdBridgeFound;
    W906_TesterHandlerWndHook                   = &FwdHandlerWnd;       //AI(W906-GB-P8) 20260928 (St02-E helper): B1 seats (TesterComm/TesterWndSeat.h)
    W906_TesterBridgeWndHook                    = &FwdBridgeWnd;
    W906_TesterSendToBridgeHook                 = &FwdSendToBridge;
    g_lastConnect = ::GetTickCount() - kConnectPeriodMs;   // first ProcessHVisionConnect on the first tick
    W906_GpibAuxEnable(true);   //AI(W906-GB-P6) 20260926: 2A + Q2(a) -- only the composition root (wb_serve) reads / seeds the recipe
    W906_TcpPumpInit(W906_TcpTesterRealSocket());  { const char* w10Sim = std::getenv("HT9045_TCPCMD_SIM"); W906_CmdServerPumpInit(W906_SimTcpRealSockets(!(w10Sim && w10Sim[0] == '1' && w10Sim[1] == '\0'))); }  W906_HanaRmsSetRealSocket(W906_TcpTesterRealSocket());   /* AI(W906-ST02-C10) 20261002 (St02-E helper): golden 912 HANARMSClient (ctNonBlocking) follows the TCP tester socket policy -- SIM build Sim, SHIP real unless HT9045_TCPCMD_SIM=1 */   // P5: the Handler's built-in TCP/IP tester channel (golden TfTesterTCP).  AI(W906-W10) 20260927 (St02-E): golden TCPCommandServer / TeraTCPResultServer as polled real sockets (HT9045_TCPCMD_SIM=1 keeps them Sim).  AI(W906-W10fix) 20260928: that block sat behind the P5 comment (dead)   //AI(W906-W58-4) 20260930 (St02-E): W58-4 -- SIM build: real 7016 / 7017 sockets only with W906_SIM_TCP_SERVERS=1 (end of file)
}

void W906_TesterCommPoll()
{
    W906_TimeDataHourTick();   // AI(W906-ELA-W48B) 20260928: golden TimerAutoBackup keeps firing inside ShowModal (header)
    if (!g_inited)
        return;
    testercomm::TesterCommHub::Instance().PollHandler();  W906_CmdServerPumpPoll();  W906_HanaRmsPumpTick();   /* AI(W906-ST02-C10) 20261002: golden ShowModal kept HANARMSClient events flowing too */   // AI(W906-W10) 20260927 (St02-E): R3 -- the TCP commands keep being answered inside the modal waits (golden ShowModal kept the VCL loop running)
}

void W906_TesterCommTick()
{
    W906_TimeDataHourTick();   // AI(W906-ELA-W48B) 20260928: ★W48-2 = B, golden HS_Function.cpp:233-238 (header)
    if (!g_inited)
        return;
    testercomm::TesterCommHub::Instance().PollHandler();
    W906_TcpPumpTick();  W906_CmdServerPumpTick();  W906_HanaRmsPumpTick();   /* AI(W906-ST02-C10) 20261002 (St02-E helper): golden 912 HANARMSClient events (POLLED socket), on this thread */   // P5: TCP/IP socket events + TimerTCPIPConnect / TimerProcessTCPData, on this thread as golden.  AI(W906-W10) 20260927 (St02-E): 7016 / 7017 accept / read / commands, on this (tick) thread.  AI(W906-W10fix) 20260928: was behind the P5 comment (dead)
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

// ---------------------------------------------------------------------------
//  AI(W906-ELA-W48B) 20260928 (St02-E helper): ★W48-2 = B -- the hourly TimeData record (see TesterCommWiring.h)
// ---------------------------------------------------------------------------
namespace {
bool g_bPerHours = false;   // golden CheckClockTrigger's function-local `static bool bPerHours=false` (:4236)
}

bool W906_TimeDataHourEdge(int iNowTime, bool* pbPerHours)   // golden HS_Function.cpp:4312-4328 (iClock==60)
{
    bool bResult=false;
    int iTime=10000;
    if(*pbPerHours)
    {
        if(iNowTime%iTime>3)
        {
            *pbPerHours=false;
            bResult=true;
        }
    }
    else
    {
        if(iNowTime%iTime<3)
            *pbPerHours=true;
    }
    return bResult;
}

void (*W906_TimeDataRecordRefreshHook)() = 0;   // AI(W906-ELA-W48B-A3) 20260928 (St02-E): see W906_TimeDataHourTickAt

void W906_TimeDataHourTickAt(int iNowTime)
{
    static bool bTimerRunning=false;                                            // golden :86
    // golden :106-111 `InitialOK==false || bTimerRunning==true || (ATC_InterfaceForm->HasAlarmMsg()==true)` -> return.
    //   The ATC term is not here: V906's global ATC_InterfaceForm is the one-field TATC_InterfaceFormShim
    //   (acarry_shims.h:115, iATC_MODE_TYPE only) and the ported TATC_InterfaceForm::HasAlarmMsg
    //   (forms/fATCHandlerSide.cpp:376) has no object to call it on (CMakeLists.txt forms list: "NOT CONNECTED").
    //   Golden skips the whole timer (and the hour edge) while one is pending; that difference is noted, not built.
    if(InitialOK==false || bTimerRunning==true)
        return;
    bTimerRunning=true;
    // AI(W906-ELA-W48B-A6) 20260929 (St02-E; ruling A6 = A, Steven 0928-29 via St02-M): the first tick after start clears the
    //   counters RecordTimeData itself clears after each row (cMyDB.cpp: BinCT[1][0..19], SystemAccSecond[2] / [3], iJamCount[2];
    //   golden :359-363 / :479-484) and the A4 LastSet.iLoaderCount, once, without writing a row -- so the first hourly row covers
    //   start .. the first hour edge instead of everything accumulated since S113.  SystemAccSecond[0] / [1] are not touched.
    static bool s_firstTickReset=false;
    if(s_firstTickReset==false)
    {
        s_firstTickReset=true;
        for(int i=0; i<20; i++)
            LastSet.BinCT[1][i]=0;
        for(int i=0; i<8; i++)
        {
            LastSet.SystemAccSecond[2][i]=0;
            LastSet.SystemAccSecond[3][i]=0;
        }
        LastSet.iJamCount[2]=0;
        LastSet.iLoaderCount=0;
    }
    if(W906_TimeDataHourEdge(iNowTime, &g_bPerHours))                           // golden :233 CheckClockTrigger(60)
    {
        // AI(W906-ELA-W48B-A3) 20260928 (St02-E, St02-E2 review A3): golden's alarm / message dialogs call fMain->Timer1Timer
        //   (note.cpp:3355, mymessbox.cpp:542 -> UpdateRecordScreen), so the record counters keep advancing while one is open;
        //   V906's W906_ModalWaitTick does too since 0928 (tools/wb_serve.cpp:7630); kept as a harmless extra: bring them up to date
        //   here: UpdateRecordScreen adds the time since its last call, so one extra call never double-counts.  The body
        //   (FileRW/MainRecord.cpp W906_MainRecordTimer1Tick) is wb_serve-only, so it comes through a seat installed by
        //   TesterComm/Handler/TimeDataRecordRefresh.cpp; empty (every ctest) = as before.
        if(W906_TimeDataRecordRefreshHook) W906_TimeDataRecordRefreshHook();
        if(IniConfig.iN10UploadProductMethod==1)                                // golden :235
            RecordTimeData(2);
        else
            RecordTimeData(3);
    }
    bTimerRunning=false;
}

void W906_TimeDataHourTick()
{
    static DWORD s_last=0;                                                      // golden TimerAutoBackup: no Interval in
    const DWORD now=::GetTickCount();                                           //   HS_Function.dfm:139-143 = 1000 ms
    if(s_last!=0 && (DWORD)(now-s_last)<1000u)
        return;
    s_last=now;
    if(InitialOK==false)                                                        // golden :106 (before the clock is read)
        return;
    W906_TimeDataHourTickAt(std::atoi(GetOnlyTimeInfoByString("").c_str()));   // golden CheckClockTrigger :4243 StrToInt
}

void W906_TesterCommShutdown()
{
    if (!g_inited)
        return;
    W906_TesterForward = W906_TesterForwardTable();     //AI(W906-GB-P2b) 20260926: uninstall first -- fMain->X is a no-op again
    W906_TesterSendToBridgeHook = 0;                    //AI(W906-GB-P8) 20260928 (St02-E helper): B1 seats, uninstalled first as well
    W906_TesterBridgeWndHook    = 0;
    W906_TesterHandlerWndHook   = 0;
    W906_GpibAuxEnable(false);  W906_CmdServerPumpShutdown();   //AI(W906-GB-P6) 20260926.  AI(W906-W10) 20260927 (St02-E); AI(W906-W10fix) 20260928: the shutdown was behind the P6 comment (dead)
    testercomm::TesterCommHub::Instance().Shutdown();   // engine Stop -> golden FormClose (logs, last-data files)
    if (fTesterSide)
    {
        THandlerTesterSide* s = fTesterSide;
        fTesterSide = NULL;
        delete s;
    }
    g_inited = false;
}

#include "TesterComm/UiHome.h"   // AI(W906-TC-SHARED) 20261001 (St02-E): the ONE home JSON (here, not at the top, so :134 keeps its number)
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
        *body = "{\"keys\":[\"home\",\"gpib\",\"rs232\",\"tcpip\"]}";   // AI(W906-TC-SHARED) 20261001 (St02-E)
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
        // AI(W906-TC-SHARED) 20261001 (St02-E): E2 MR !46 m1 -- GPIB / RS232: the home JSON is composed here from the engine's
        //   own part, so it keeps updating while a Handler modal box holds the main loop (W906_TesterCommPoll does not run
        //   TcpPump; golden's two programs are separate processes).  TCP/IP and before the first TcpPump beat: TcpPump's copy.
        const int homeTt = key == "home" ? W906_TesterCommHomeTestType() : -1;
        const std::string homeKey = testercomm::HomeKeyOfTestType(homeTt);
        if (homeKey == "gpib" || homeKey == "rs232")
        {
            std::string frag;
            if (!testercomm::UiChannel::Instance().Read(homeKey + ".home", &frag, &seq))
                frag.clear();                                   // the engine has not published: up false, the same keys
            *status = 200;
            *body = testercomm::HomeCompose(homeTt, frag);
            return true;
        }
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
        std::string postKey = key;   // AI(W906-TC-SHARED) 20261001 (St02-E): the 首頁 grid's site commands go to the active interface's engine
        if (key == "home")
        {
            postKey = testercomm::HomeKeyOfTestType(W906_TesterCommHomeTestType());
            const bool tcpBin = postKey == "tcpip" && cmd.find(" bin ") != std::string::npos;   // TCP/IP: cbSimulateBin only (TcpPump)
            if (cmd.compare(0, 5, "site ") != 0 || (postKey != "gpib" && postKey != "rs232" && !tcpBin))
            {
                *status = 200;
                *body = "{\"queued\":false,\"reason\":\"home takes site commands for the active engine only (TCP/IP: site <i> bin <n>)\"}";
                return true;
            }
        }
        //AI(W906-GB-P7) 20260926: same-key time window, like the WebSocket 400 ms guard (which does not cover this
        //   HTTP path).  "click btnManualStart" / "click btManualTest" / "click btnDiagZip" / the toggles fire an action
        //   per call, so an exact repeat within 400 ms is dropped; set-value commands (check / combo / edit / site) are
        //   idempotent, so dropping their exact repeat changes nothing.
        if (!testercomm::UiChannel::Instance().Post(postKey, cmd, kPostRepeatWindowMs))
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
//------------------------------------------------------------------------------
// AI(W906-W58-4) 20260930 (St02-E): Steven W58-4 (decisions-decided.md:2801-2811 -- 20260929 08:1x 「關」, 11:06 「W58-4 ok」):
//   in the SIM build the TCP command servers 7016 / 7017 (fMain->TCPCommandServer / TeraTCPResultServer) stay in SIM mode --
//   no real listening port -- unless wb_serve is started with the environment variable W906_SIM_TCP_SERVERS=1.
//   golden opens them from the customer code in both builds (CosFunction.bEnableHandlerResultServer: Greatek / TeraPower /
//   TeraProbe, CosFunction.cpp:1645 / :1962 / :2010 / :4399; Command.cpp:13527 / :13533 Open()); the SHIP build is unchanged.
//   HT9045_TCPCMD_SIM=1 still forces SIM mode in both builds (the caller at :134).  config.ini gets no new key (customer
//   format unchanged).  ctest is unaffected: test_tcp_cmd_server calls W906_CmdServerPumpInit(false) itself and
//   test_testercomm_handler sets HT9045_TCPCMD_SIM=1.
#include <cstdio>
bool W906_SimTcpRealSockets(bool bRealSockets)
{
#ifdef SOFT_SIMULTE
    if (bRealSockets) {
        const char* e = std::getenv("W906_SIM_TCP_SERVERS");
        if (!(e && e[0] == '1' && e[1] == '\0')) {
            std::printf("[W58-4] SIM build: if this customer opens the TCP command servers 7016 / 7017 (CosFunction.bEnableHandlerResultServer), they stay in SIM mode -- set W906_SIM_TCP_SERVERS=1 for real sockets\n");   std::fflush(stdout);   //AI(W906-W58) 20260930 (St02-E): E2 MR !12 m1 -- printed at every SIM boot, before golden decides (Command.cpp:13520), hence "if"
            return false;
        }
    }
#endif
    return bRealSockets;
}
