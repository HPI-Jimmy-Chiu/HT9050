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
//  FULL part (opt-in HT9045_TESTERCOMM_E2E=1; the GPIB engine writes D:\GPIBLOG like golden):
//    5. real GpibEngine on the simulated NI driver + this Handler side on one hub: the bridge finds the Handler,
//       sends Version / TesterMode, ProcessHVisionConnect finds the bridge, SendMSG_TestMode reaches the bridge and
//       passes its window-identity check (the bridge stays up), CloseGpibProgram closes it.
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/Handler/TesterCommWiring.h"
#include "forms/fMain.h"
#include "TesterComm/TesterCommHub.h"
#include "TesterComm/Gpib/GpibEngine.h"
#include "cmydef.h"
#include "cprod.h"
#include "Interface/InterfaceSYS.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

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

    W906_TesterCommInit();
    CHECK(fTesterSide != NULL);
    CHECK(W906_TesterForward.SendMSG_CMD != 0);
    CHECK(W906_TesterForward.RunTestProgram != 0);
    if (fTesterSide)
        CHECK(!fTesterSide->bFind);

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

int main()
{
    TestOffline();
    TestForwardSeat();
    TestEsdWindow();
    TestEndToEnd();
    std::printf("test_testercomm_handler: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
