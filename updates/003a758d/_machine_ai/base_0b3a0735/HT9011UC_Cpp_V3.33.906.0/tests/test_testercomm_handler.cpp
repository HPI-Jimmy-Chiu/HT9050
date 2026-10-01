// ===========================================================================
//  tests/test_testercomm_handler.cpp -- Tester-comm plan P2a: the Handler side (THandlerTesterSide).
//  AI(W906-GB-P2a) 20260926.
//
//  The Handler-side golden code replies through `fMain->SendMSG_CMD(...)` (Command.cpp's Write*/Get*), which in the
//  V906 facade is still an offline no-op until the P2b hookup (forms/fMain.cpp).  This test installs a TfMain
//  subclass whose SendMSG_CMD overloads forward to fTesterSide -- exactly what the hookup will do.
//
//  DEFAULT part (no disk side effects, no engine):
//    1. link: the whole Handler side + machine archives link (RESCAN group).
//    2. identity token == the hub's mailbox (what the engines use as HMountWnd); no bridge -> FindBridgeWindow NULL.
//    3. golden guards with no bridge: RunTestProgram / CloseGpibProgram / SendMSG_CMD return without sending.
//    4. Sink(): a bridge packet (MSG_CMD_TesterMode, MSG_CMD_Version) runs OnGpibProgramMsg on this thread, no throw.
//    6. P2b install seat (forms/fMain.h W906_TesterForward): before W906_TesterCommInit the TfMain members are the old
//       offline no-ops; after it they reach fTesterSide.  St01 S86 (INSTALL_OCR != 0): the four BarCode / Pin1
//       commands sent at boot, recipe change and OCR page open return at once while no bridge is found (no mailbox
//       wait).  Shutdown uninstalls the seat first.  (Init only registers factories and binds the TCP shim's events;
//       no engine starts, nothing touches the disk.)
//    7. G3 (AI(W906-ESD-G3) 20260927): ProcessHVisionConnect's golden FindWindow("TfESDMain", "ESD_Monitor") sets
//       fMain->HESDWnd.  No such window -> NULL, and SendCommand_ESD(ESD_SYSTEM_CLOSE) (St01 S121 Exit) sends nothing.
//       A hidden window this test registers under that class / title stands in for the ESD program: it is found and
//       gets golden's WM_COPYDATA M_V (TYPE_HANDLER_ESD / CommandType_ESD / ESD_SYSTEM_CLOSE); destroyed -> NULL again.
//       Nothing is launched; WakeupGPIBdelay is armed so ProcessHVisionConnect does not wake a bridge.  Skipped when a
//       real ESD_Monitor is up on the PC (it would get the command).
//    8. G5 = B (AI(W906-ESD-G5) 20260927, RULINGS_20260927 #24): with the HT IonBar configured and no ESD window the
//       power-reset state is armed (count 450) and nothing is sent or launched -- WakeupESD() is removed from the
//       translation and V906 defines no WakeupESD, so no launch path exists at all.  With the stand-in window up, the
//       count runs one step per call; at 500 golden's ESD_HT_IONBAR_ControllerN_PowerOn M_V goes out once for each IonBar
//       whose alarm sensor is on (0 and 1 here), not for the one that is off (2) until it turns on.
//    9. U12 (AI(W906-GB-P8) 20260928, St02-E helper): a bridge packet MSG_CMD_MachineState (golden OnMyCopyMsg ->
//       TfMain::MachineStatus, golden 906_0625_Steven main.cpp:16113-16116) answers through the B1 send seat
//       (TesterComm/TesterWndSeat.h): the global HHandler2Gpib goes out whole, iSendCommand == MSG_CMD_MachineState,
//       golden's constant bits 0 / 6 / 13 are 0, and with SystemStart false Bit9 (stop) is 1 and Bit5 is 0.  Seat empty
//       -> the packet is still built and nothing is sent (the port before U12).  A capture function stands in for the
//       seat: no engine, no window, no disk.
//  0. AI(W906-TEST-SAFE) 20260928 (St02-E helper): refuses to run (exit 2, nothing called) outside ctest's redirect roots
//     (st02_test_containment.h): the 7016 pump (TCPIPCommunicationLog under as9045LogPath) and golden RecordProcess
//     would write the machine's real D:\HT9045_Log when it is run by hand.
//  FULL part (opt-in HT9045_TESTERCOMM_E2E=1; the GPIB engine writes D:\GPIBLOG like golden):
//    5. real GpibEngine on the simulated NI driver + this Handler side on one hub: the bridge finds the Handler,
//       sends Version / TesterMode, ProcessHVisionConnect finds the bridge, SendMSG_TestMode reaches the bridge and
//       passes its window-identity check (the bridge stays up), CloseGpibProgram closes it.
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/Handler/TesterCommWiring.h"
#include "forms/fMain.h"
#include "TesterComm/TesterCommHub.h"
#include "TesterComm/TesterWndSeat.h"   // AI(W906-GB-P8) 20260928 (St02-E helper): B1 seats
#include "TesterComm/Gpib/GpibEngine.h"
#include "cmydef.h"
#include "cprod.h"
#include "Interface/InterfaceSYS.h"
#include "mysensor.h"
#include "MachineType.h"
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "st02_test_containment.h"   // AI(W906-TEST-SAFE) 20260928 (St02-E helper)

// AI(W906-W10fix) 20260928 (St02-E): same guard as tests/test_ela_ftp.cpp (MinGW.org 6.3 strict mode declares no _putenv;
// getenv() reads the CRT copy, which SetEnvironmentVariableA does not update).
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#endif

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

// the P2b hookup, in test form
class TesterFMain : public TfMain
{
public:
    void SendMSG_CMD(int CMD) override
    {
        if (fTesterSide)
            fTesterSide->SendMSG_CMD(CMD);
    }
    void SendMSG_CMD(int CMD, AnsiString Message) override
    {
        if (fTesterSide)
            fTesterSide->SendMSG_CMD(CMD, Message);
    }
};

static std::string Packet(unsigned cmd, const char* ret)
{
    VM vm;
    std::memset(&vm, 0, sizeof(vm));
    vm.iCommand = cmd;
    if (ret)
        std::strncpy(vm.cReturn, ret, sizeof(vm.cReturn) - 1);
    return std::string(reinterpret_cast<const char*>(&vm), sizeof(vm));
}

static void TestOffline()
{
    testercomm::TesterCommHub hub;
    THandlerTesterSide side;
    fTesterSide = &side;
    side.Attach(&hub);

    CHECK(side.HandlerWndToken() == reinterpret_cast<HWND>(&hub.Mailbox()));
    CHECK(side.FindBridgeWindow() == NULL);
    CHECK(!side.bFind);

    // golden: RunTestProgram / CloseGpibProgram / SendMSG_CMD all start with `if(bFind==false) return`
    CHECK(side.RunTestProgram(false) == false);
    side.CloseGpibProgram("test");
    side.SendMSG_CMD(MSG_CMD_NONE);

    // bridge -> Handler packets, on this (Handler) thread
    InitialOK = true;
    bSystemClose = false;
    const unsigned long before = side.sinkMessages;
    CHECK(THandlerTesterSide::Sink(Packet(MSG_CMD_TesterMode, ""), &side) == 0);
    CHECK(THandlerTesterSide::Sink(Packet(MSG_CMD_Version, "V12.13.905.0"), &side) == 0);
    CHECK(side.sinkMessages == before + 2);
    CHECK(side.depth == 0);

    fTesterSide = NULL;
}

static void TestForwardSeat()
{
    // AI(W906-GB-P2b) 20260926: the install seat as wb_serve uses it (H3 Init / H6 Shutdown).
    CHECK(fMain != NULL);
    CHECK(W906_TesterForward.SendMSG_CMD == 0);                 // not installed: the old offline no-op
    fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);
    CHECK(fMain->RunTestProgram(true) == false);

    _putenv("HT9045_TCPCMD_SIM=1");   // AI(W906-W10fix) 20260928 (St02-E): Init now reaches the 7016 / 7017 pump -- keep both Sim
    W906_TesterCommInit();
    CHECK(fTesterSide != NULL);
    CHECK(fMain->TCPCommandServer != NULL && fMain->TeraTCPResultServer != NULL && fMain->TCPCommandServer->IsSimMode());
    CHECK(W906_TesterForward.SendMSG_CMD != 0);
    CHECK(W906_TesterForward.RunTestProgram != 0);
    // AI(W906-GB-P8) 20260928 (St02-E helper): B1 -- TfMain::GetTTLState's tokens and send are installed with the table
    CHECK(W906_TesterHandlerWndHook != 0 && W906_TesterBridgeWndHook != 0 && W906_TesterSendToBridgeHook != 0);
    if (fTesterSide)
        CHECK(W906_TesterHandlerWnd(NULL) == fTesterSide->HandlerWndToken() && W906_TesterBridgeWnd(NULL) == fTesterSide->HVisionWnd);
    if (fTesterSide)
        CHECK(!fTesterSide->bFind);
    // AI(W906-GB-P8-A1) 20260928 (St02-E): the seats are installed but HVisionWnd is NULL (no bridge) -> nothing is sent, as
    //   golden SendMessage(NULL); with a window token set, exactly one send.  A capture replaces the send seat only here.
    {
        static int s_a1Sent = 0;
        struct A1Cap { static void Send(COPYDATASTRUCT*) { ++s_a1Sent; } };
        void (*const realSend)(COPYDATASTRUCT*) = W906_TesterSendToBridgeHook;
        W906_TesterSendToBridgeHook = &A1Cap::Send;
        COPYDATASTRUCT a1cp;
        a1cp.dwData = 0;  a1cp.cbData = 0;  a1cp.lpData = 0;
        const HWND a1Saved = fTesterSide ? fTesterSide->HVisionWnd : NULL;
        if (fTesterSide) fTesterSide->HVisionWnd = NULL;
        W906_TesterSendToBridge(&a1cp);
        CHECK(s_a1Sent == 0);
        if (fTesterSide)
        {
            fTesterSide->HVisionWnd = reinterpret_cast<HWND>(&s_a1Sent);
            W906_TesterSendToBridge(&a1cp);
            CHECK(s_a1Sent == 1);
            fTesterSide->HVisionWnd = a1Saved;
        }
        W906_TesterSendToBridgeHook = realSend;
    }

    // St01 S86 (golden DoReadLastData / recipe change / OCR page): INSTALL_OCR != 0 sends these four.  No bridge yet
    // -> golden `if(bFind==false) return` (the one-argument SendMSG_CMD), so there is no 5 s mailbox wait.
    const DWORD t0 = ::GetTickCount();
    fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);
    fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);
    fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);
    fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);
    fMain->SendMSG_CMD(MSG_CMD_NONE, "no bFind check in golden: the hub has no engine -> kNoReceiver at once");
    fMain->SendMSG_CMD_DeviceMapSRQ(0);                         // no bFind check in golden either; same kNoReceiver
    CHECK(fMain->RunTestProgram(true) == false);                // golden bFind==false -> false
    fMain->CloseGpibProgram("test");
    fMain->SendMSG_TestMode();
    CHECK(::GetTickCount() - t0 < 1000);

    W906_TesterCommShutdown();
    CHECK(fTesterSide == NULL);
    CHECK(W906_TesterForward.SendMSG_CMD == 0);
    CHECK(W906_TesterForward.RunTestProgram == 0);
    // AI(W906-GB-P8) 20260928 (St02-E helper): B1 seats cleared; not installed = the token field keeps its value, nothing is sent
    CHECK(W906_TesterHandlerWndHook == 0 && W906_TesterBridgeWndHook == 0 && W906_TesterSendToBridgeHook == 0);
    CHECK(W906_TesterHandlerWnd(reinterpret_cast<HWND>(&g_total)) == reinterpret_cast<HWND>(&g_total));
    W906_TesterSendToBridge(NULL);
    fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);                  // uninstalled again: no-op
    CHECK(fMain->RunTestProgram(true) == false);
}

static void TestEndToEnd()
{
    const char* on = std::getenv("HT9045_TESTERCOMM_E2E");
    if (!on || std::strcmp(on, "1") != 0)
    {
        std::printf("end-to-end: skipped (set HT9045_TESTERCOMM_E2E=1; the GPIB engine writes D:\\GPIBLOG like golden)\n");
        return;
    }
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    const std::string dir = std::string(tmp) + "ht9045_handler_ctest";
    ::CreateDirectoryA(dir.c_str(), 0);
    const std::string gen = dir + "\\general.ini", hgen = dir + "\\Gerneral.ini";
    FILE* f = std::fopen(gen.c_str(), "wb");
    if (f) { std::fputs("[Version]\r\nModel=9045GPIB\r\n", f); std::fclose(f); }
    f = std::fopen(hgen.c_str(), "wb");
    if (f) { std::fputs("[Version]\r\nModel=HT-9045\r\n[System]\r\nCUSTOMER_CODE=910\r\n", f); std::fclose(f); }
    gpibbridge::GpibEngine::OverrideIniPaths(gen, hgen);
    gpibbridge::SimGpibDriver sim;
    gpibbridge::GpibEngine::InjectDriver(&sim);

    TfMain* oldMain = fMain;
    TesterFMain* testerMain = new TesterFMain();
    fMain = testerMain;

    testercomm::TesterCommHub hub;
    hub.RegisterFactory(testercomm::kTestTypeGpib, &gpibbridge::GpibEngine::Create);
    THandlerTesterSide side;
    fTesterSide = &side;
    side.Attach(&hub);
    InitialOK = true;
    bSystemClose = false;
    TestIF.iTestType = GPIB_MODE;
    TestIF_File.iTestType = GPIB_MODE;
    MachineTypeChoice = Type_HT9045;

    CHECK(side.StartBridgeProgram());
    // Timer1 -> ProcessHMountConnect (once a second, SleepEx(1000)) -> Version + TesterMode to this side
    const DWORD t0 = ::GetTickCount();
    unsigned long lastSink = side.sinkMessages;
    while (::GetTickCount() - t0 < 10000 && side.sinkMessages < lastSink + 2)
    {
        hub.PollHandler();
        ::Sleep(5);
    }
    CHECK(side.sinkMessages >= lastSink + 2);
    CHECK(hub.IsUp());

    // golden timer path: ProcessHVisionConnect finds the bridge window
    side.WakeupGPIBdelay.SetSecAndOn(0);
    side.ProcessHVisionConnect();
    CHECK(side.bFind);
    CHECK(side.HVisionWnd == gpibbridge::GpibEngine::BridgeWndToken());

    // Handler -> bridge with the identity the bridge checks (golden OnMyCopyMsg HandlerHwnd / GpibHwnd)
    const unsigned long bridgeMsgs = gpibbridge::GpibEngine::HandlerMessages();
    side.SendMSG_TestMode();
    CHECK(gpibbridge::GpibEngine::HandlerMessages() > bridgeMsgs);
    const DWORD t1 = ::GetTickCount();
    while (::GetTickCount() - t1 < 1500)
    {
        hub.PollHandler();
        ::Sleep(5);
    }
    CHECK(hub.IsUp());   // a wrong HandlerHwnd / GpibHwnd would have made the bridge close itself

    side.CloseGpibProgram("ctest");
    CHECK(!side.bFind);
    const DWORD t2 = ::GetTickCount();
    while (hub.IsUp() && ::GetTickCount() - t2 < 2000)
    {
        hub.PollHandler();
        ::Sleep(5);
    }
    CHECK(!hub.IsUp());
    CHECK(hub.HandlerErrors() == 0);
    CHECK(hub.Thread().EngineErrors() == 0);

    hub.Shutdown();
    fTesterSide = NULL;
    fMain = oldMain;
    delete testerMain;   // through its own type: TfMain's destructor need not be virtual
    gpibbridge::GpibEngine::InjectDriver(0);
    gpibbridge::GpibEngine::OverrideIniPaths(std::string(), std::string());
}

// 7 -- the stand-in ESD_Monitor window procedure (same thread: SendMessage calls it directly)
static int g_esdCopies = 0;
static int g_esdMode = -1, g_esdType = -1, g_esdCmd = -1;
static std::vector<int> g_esdCmds;   // every ESD command received, in order (part 8)
static LRESULT CALLBACK EsdWndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_COPYDATA)
    {
        const COPYDATASTRUCT* cp = reinterpret_cast<const COPYDATASTRUCT*>(l);
        if (cp && cp->dwData == WM_ESD_Program && cp->cbData == sizeof(M_V) && cp->lpData)
        {
            const M_V* mv = static_cast<const M_V*>(cp->lpData);
            g_esdMode = mv->bModeType;
            g_esdType = mv->bCommandType;
            g_esdCmd = mv->bCommand;
            g_esdCmds.push_back(mv->bCommand);
            ++g_esdCopies;
        }
        return TRUE;
    }
    return ::DefWindowProcA(h, m, w, l);
}

static void TestEsdWindow()
{
    if (::FindWindowA("TfESDMain", "ESD_Monitor") != NULL)
    {
        std::printf("  SKIP 7: a real ESD_Monitor window is up on this PC\n");
        return;
    }
    testercomm::TesterCommHub hub;
    THandlerTesterSide side;
    fTesterSide = &side;
    side.Attach(&hub);
    side.WakeupGPIBdelay.SetSecAndOn(60);        // no bridge here: keep WakeupGPIB out of this part
    const int oldGpibMode = TestIF.iGpibMode;
    TestIF.iGpibMode = 0;                        // not InterfaceType_SPEA_Type: that arm wakes the bridge unconditionally
    const int oldKasuga = USE_KASUGA;
    USE_KASUGA = 1;                              // SendCommand_ESD's golden gate (InterfaceSYS.cpp)

    side.ProcessHVisionConnect();
    CHECK(fMain->HESDWnd == NULL);
    SendCommand_ESD(ESD_SYSTEM_CLOSE);
    CHECK(g_esdCopies == 0);

    WNDCLASSA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = EsdWndProc;
    wc.hInstance = ::GetModuleHandleA(0);
    wc.lpszClassName = "TfESDMain";
    CHECK(::RegisterClassA(&wc) != 0);
    HWND w = ::CreateWindowA("TfESDMain", "ESD_Monitor", WS_OVERLAPPED, 0, 0, 10, 10, 0, 0, wc.hInstance, 0);   // never shown
    CHECK(w != NULL);
    side.ProcessHVisionConnect();
    CHECK(fMain->HESDWnd == w);
    SendCommand_ESD(ESD_SYSTEM_CLOSE);
    CHECK(g_esdCopies == 1);
    CHECK(g_esdMode == TYPE_HANDLER_ESD && g_esdType == CommandType_ESD && g_esdCmd == (int)ESD_SYSTEM_CLOSE);
    USE_KASUGA = 0;
    if (USE_NOVX3360 == 0 && !USE_KASUGA_Fan && iUseHTIonBarFunction == 0)
    {
        SendCommand_ESD(ESD_SYSTEM_CLOSE);       // golden gate closed (no NOVX3360 / KASUGA / KASUGA_Fan / HT IonBar)
        CHECK(g_esdCopies == 1);
    }

    if (w) ::DestroyWindow(w);
    ::UnregisterClassA("TfESDMain", wc.hInstance);
    side.ProcessHVisionConnect();
    CHECK(fMain->HESDWnd == NULL);

    USE_KASUGA = oldKasuga;
    TestIF.iGpibMode = oldGpibMode;
    fMain->HESDWnd = NULL;
    fTesterSide = NULL;
}

// 8 -- G5 = B: the HT IonBar power-on sequence with an ESD program up, never a launch
static void TestEsdIonBarPowerOn()
{
    if (::FindWindowA("TfESDMain", "ESD_Monitor") != NULL)
    {
        std::printf("  SKIP 8: a real ESD_Monitor window is up on this PC\n");
        return;
    }
    testercomm::TesterCommHub hub;
    THandlerTesterSide side;
    fTesterSide = &side;
    side.Attach(&hub);
    side.WakeupGPIBdelay.SetSecAndOn(60);        // no bridge here: keep WakeupGPIB out of this part
    const int oldGpibMode = TestIF.iGpibMode;
    TestIF.iGpibMode = 0;
    const int oldMon = ESD_Monitor, oldNovx = USE_NOVX3360, oldKas = USE_KASUGA, oldHT = iUseHTIonBarFunction;
    const bool oldKasFan = USE_KASUGA_Fan, oldSysType = bESDSystemtype;
    const bool oldReset = bUseHTIonBar_PowerReset;
    const int oldCount = iUseHTIonBar_PowerResetCount;
    int oldStatus[3], oldEnable[3], oldType[3], oldBase[3];
    for (int i = 0; i < 3; ++i)
    {
        oldStatus[i] = iUseHTIonBar_SendPowerStatus[i];
        oldEnable[i] = Sen[iHTIonBar[i]].Enable ? 1 : 0;
        oldType[i] = Sen[iHTIonBar[i]].Type;
        oldBase[i] = Sen[iHTIonBar[i]].ISABase;
    }
    ESD_Monitor = 0; USE_NOVX3360 = 0; USE_KASUGA = 0; USE_KASUGA_Fan = false; bESDSystemtype = false;
    iUseHTIonBarFunction = 1;                    // HT IonBar configured (also SendCommand_ESD's gate)
    bUseHTIonBar_PowerReset = false; iUseHTIonBar_PowerResetCount = 0;
    for (int i = 0; i < 3; ++i) iUseHTIonBar_SendPowerStatus[i] = 0;
    for (int i = 0; i < 3; ++i)                  // alarm sensors 0 and 1 on, 2 off (test_w7_l1_auto_rt.cpp:511-512 idiom)
    {
        Sen[iHTIonBar[i]].Enable = true;
        Sen[iHTIonBar[i]].ISABase = eISABase;
        Sen[iHTIonBar[i]].Type = (i < 2) ? TYPE_B : TYPE_A;
    }
    RunInfo.ESDSoftwareVersion = "V1";

    // no ESD window: arm the power-reset state, send nothing, launch nothing
    const int before = g_esdCopies;
    side.WakeupESDdelay.SetSecAndOn(0);
    side.ProcessHVisionConnect();
    CHECK(fMain->HESDWnd == NULL);
    CHECK(bUseHTIonBar_PowerReset == true && iUseHTIonBar_PowerResetCount == 450);
    CHECK(iUseHTIonBar_SendPowerStatus[0] == 0 && iUseHTIonBar_SendPowerStatus[1] == 0 && iUseHTIonBar_SendPowerStatus[2] == 0);
    CHECK(RunInfo.ESDSoftwareVersion == "");
    CHECK(g_esdCopies == before);
    CHECK(side.WakeupESDdelay.Off() == false);   // re-armed for 10 s, golden
    CHECK(::FindWindowA("TfESDMain", "ESD_Monitor") == NULL);   // nothing came up (no launch path exists)

    // the stand-in ESD program comes up
    WNDCLASSA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = EsdWndProc;
    wc.hInstance = ::GetModuleHandleA(0);
    wc.lpszClassName = "TfESDMain";
    CHECK(::RegisterClassA(&wc) != 0);
    HWND w = ::CreateWindowA("TfESDMain", "ESD_Monitor", WS_OVERLAPPED, 0, 0, 10, 10, 0, 0, wc.hInstance, 0);   // never shown
    CHECK(w != NULL);
    g_esdCmds.clear();
    for (int k = 0; k < 49; ++k) side.ProcessHVisionConnect();
    CHECK(fMain->HESDWnd == w);
    CHECK(bUseHTIonBar_PowerReset == true && iUseHTIonBar_PowerResetCount == 499 && g_esdCopies == before);
    side.ProcessHVisionConnect();                // the 50th call reaches 500
    CHECK(bUseHTIonBar_PowerReset == false && iUseHTIonBar_PowerResetCount == 0);
    CHECK(g_esdCopies == before + 2 && g_esdCmds.size() == 2);
    CHECK(g_esdCmds.size() == 2 && g_esdCmds[0] == (int)ESD_HT_IONBAR_Controller1_PowerOn &&
          g_esdCmds[1] == (int)ESD_HT_IONBAR_Controller1_PowerOn + 1);
    CHECK(g_esdMode == TYPE_HANDLER_ESD && g_esdType == CommandType_ESD);
    CHECK(iUseHTIonBar_SendPowerStatus[0] == 0 && iUseHTIonBar_SendPowerStatus[1] == 0 && iUseHTIonBar_SendPowerStatus[2] == 1);
    side.ProcessHVisionConnect();
    CHECK(g_esdCopies == before + 2);            // sensor 2 still off: nothing more
    Sen[iHTIonBar[2]].Type = TYPE_B;             // its alarm sensor comes on later
    side.ProcessHVisionConnect();
    CHECK(g_esdCopies == before + 3 && g_esdCmd == (int)ESD_HT_IONBAR_Controller1_PowerOn + 2);
    side.ProcessHVisionConnect();
    CHECK(g_esdCopies == before + 3);            // once per power-reset cycle

    if (w) ::DestroyWindow(w);
    ::UnregisterClassA("TfESDMain", wc.hInstance);
    for (int i = 0; i < 3; ++i)
    {
        iUseHTIonBar_SendPowerStatus[i] = oldStatus[i];
        Sen[iHTIonBar[i]].Enable = oldEnable[i] != 0;
        Sen[iHTIonBar[i]].Type = oldType[i];
        Sen[iHTIonBar[i]].ISABase = oldBase[i];
    }
    bUseHTIonBar_PowerReset = oldReset; iUseHTIonBar_PowerResetCount = oldCount;
    ESD_Monitor = oldMon; USE_NOVX3360 = oldNovx; USE_KASUGA = oldKas; USE_KASUGA_Fan = oldKasFan;
    bESDSystemtype = oldSysType; iUseHTIonBarFunction = oldHT;
    TestIF.iGpibMode = oldGpibMode;
    fMain->HESDWnd = NULL;
    fTesterSide = NULL;
}

// 9 -- U12: TfMain::MachineStatus's golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, ...) goes through the B1 seat
static int g_msSends = 0;
static unsigned g_msCmd = 0;
static DWORD g_msBytes = 0;
static const void* g_msData = NULL;
static int g_msStatus[17];
static void CaptureSendToBridge(COPYDATASTRUCT* pcp)
{
    if (pcp == NULL || pcp->lpData == NULL)
        return;
    ++g_msSends;
    g_msBytes = pcp->cbData;
    g_msData = pcp->lpData;
    const MV* mv = static_cast<const MV*>(pcp->lpData);
    g_msCmd = mv->iSendCommand;
    for (int i = 0; i < 17; ++i)
        g_msStatus[i] = mv->iStatus[i];
}

static void TestMachineStatusSend()
{
    testercomm::TesterCommHub hub;
    THandlerTesterSide side;
    fTesterSide = &side;
    side.Attach(&hub);
    InitialOK = true;
    bSystemClose = false;
    const int oldFileGpibMode = TestIF_File.iGpibMode;
    TestIF_File.iGpibMode = 0;                   // not InterfaceType_SPEA_Type: that arm hands the packet to _OnMyCopyMsg_Interface
    const bool oldStart = SystemStart;
    SystemStart = false;
    CHECK(W906_TesterSendToBridgeHook == 0);     // TestForwardSeat's Shutdown cleared it

    // seat empty: golden builds the packet, nothing goes out (the port before U12)
    HHandler2Gpib.iSendCommand = MSG_CMD_NONE;
    CHECK(THandlerTesterSide::Sink(Packet(MSG_CMD_MachineState, ""), &side) == 0);
    CHECK(HHandler2Gpib.iSendCommand == MSG_CMD_MachineState);
    CHECK(g_msSends == 0);

    // AI(W906-GB-P8-A1) 20260928 (St02-E): send seat installed but no bridge window (HVisionWnd NULL) -> nothing goes out,
    //   as golden SendMessage(NULL) (TesterComm/TesterWndSeat.h W906_TesterSendToBridge)
    HHandler2Gpib.iSendCommand = MSG_CMD_NONE;
    W906_TesterSendToBridgeHook = &CaptureSendToBridge;
    CHECK(W906_TesterBridgeWndHook == 0);
    CHECK(THandlerTesterSide::Sink(Packet(MSG_CMD_MachineState, ""), &side) == 0);
    CHECK(g_msSends == 0);

    // seat installed (a capture function stands in for FwdSendToBridge) and a bridge window token: the reply goes out once,
    //   the whole global
    struct U12Wnd { static HWND Get() { return reinterpret_cast<HWND>(&g_msSends); } };   // AI(W906-GB-P8-A1): any non-NULL token
    HHandler2Gpib.iSendCommand = MSG_CMD_NONE;
    W906_TesterBridgeWndHook = &U12Wnd::Get;
    CHECK(THandlerTesterSide::Sink(Packet(MSG_CMD_MachineState, ""), &side) == 0);
    W906_TesterSendToBridgeHook = 0;
    W906_TesterBridgeWndHook = 0;
    CHECK(g_msSends == 1);
    CHECK(g_msCmd == MSG_CMD_MachineState);
    CHECK(g_msBytes == sizeof(HHandler2Gpib) && g_msData == static_cast<const void*>(&HHandler2Gpib));
    CHECK(g_msStatus[Bit0_HandlerReBoot] == 0 && g_msStatus[Bit6_Reversed] == 0 && g_msStatus[Bit13_HandlerOk] == 0);
    CHECK(g_msStatus[Bit9_HandlerStop] == 1 && g_msStatus[Bit5_IndexCheck] == 0);   // golden: SystemStart==false -> bEMG, no Index Check
    CHECK(side.depth == 0);

    SystemStart = oldStart;
    TestIF_File.iGpibMode = oldFileGpibMode;
    fTesterSide = NULL;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);   // AI(W906-TEST-SAFE) 20260928 (St02-E helper): unbuffered -- a crash report shows the exact last line
    std::printf("TesterComm_Handler\n");
    // AI(W906-TEST-SAFE) 20260928 (St02-E helper): containment first -- run by hand (no ctest environment),
    //   the 7016 pump (TCPIPCommunicationLog) and golden RecordProcess would write the machine's real D:\HT9045_Log / D:\RMS trees.
    //   Refuse, exit 2, before any Handler code (st02_test_containment.h).
    if (!W906TestInsideCtestRoots("TesterComm_Handler"))
        return 2;

    TestOffline();
    TestForwardSeat();
    TestMachineStatusSend();   // AI(W906-GB-P8) 20260928 (St02-E helper): U12
    TestEsdWindow();
    TestEsdIonBarPowerOn();
    TestEndToEnd();
    std::printf("test_testercomm_handler: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
