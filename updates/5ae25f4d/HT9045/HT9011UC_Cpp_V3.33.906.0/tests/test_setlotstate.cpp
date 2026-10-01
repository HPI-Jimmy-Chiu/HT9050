// AI(W906-SETLOTSTATE) 20261001: TfMain::SetLotState (forms/fMain_SetLotState.cpp) = golden main.cpp:15160-15407.
//   Drives the GPIB arm through test hooks in the port's own seats (forms/fMain.h W906_TesterForward.BridgeFound,
//   TesterComm/TesterWndSeat.h W906_TesterBridgeWndHook / W906_TesterSendToBridgeHook):
//     bridge not found -> return before anything (golden :15200-15201);
//     FT start (2) -> SCK ART step 2, lot-start time stamped, the 120 s lot-start timeout armed, HHandler2Gpib =
//       MSG_CMD_LotStatus + iLotStatus 2, the AMR "DoLotEnd sent" flag cleared;
//     RT start (4) -> step 8; lot end (8) -> step 4 (one FT/RT pass) or 10 (more), the lot-end timeout armed;
//     the packet reaches the bridge for UTAC, for SCK / SCS at FT start and for [GPIBLotEnd]; nobody else;
//     a TCP/IP tester of SJ Semiconductor OS / XINITECH -> return at once; TestIF not GPIB -> return.
//   Part 2 reads the source: the three TU-local no-op macros now call fMain->SetLotState, the forms/ empty body is
//   gone, and the mapped calls are in place. NOT COVERED: the TCP/IP arm's socket send (TesterTCPSocket needs a live
//   socket), the EventReport arms, fAutomation (the no-op shim). Memory only; no file is written.
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "forms/fMain.h"
#include "forms/fSCKART.h"
#include "forms/fAGV.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "MessageDef.h"
#include "csystem.h"
#include "TesterComm/TesterWndSeat.h"

static int g_fail = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static bool g_bridge = false;
static bool BridgeFound() { return g_bridge; }
static HWND BridgeWnd() { return reinterpret_cast<HWND>(1); }
static int g_sends = 0, g_lastLotStatus = -1;
static unsigned g_lastCmd = 0;
static void SendToBridge(COPYDATASTRUCT* pcp)
{
    ++g_sends;
    const MV* mv = reinterpret_cast<const MV*>(pcp->lpData);
    g_lastCmd = (unsigned)mv->iSendCommand;
    g_lastLotStatus = mv->iLotStatus;
}

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main(int argc, char** argv)
{
    W906_TesterForward.BridgeFound = &BridgeFound;
    W906_TesterBridgeWndHook = &BridgeWnd;
    W906_TesterSendToBridgeHook = &SendToBridge;
    TestIF_File.iTestType = 0;                 // not TCP/IP
    TestIF.iTestType = GPIB_MODE;
    CUSTOMER_CODE = CC_KYEC_LEE;               // none of the customer arms
    CosFunction.bAutoRetestGPIBmode = true;    // keep the OLP PRODUCTION_REQUEST (no-op shim) out of the picture
    CosFunction.bUseSCKART = false; CosFunction.bGPIBLotEnd = false; CosFunction.iAutoRetestTCPmode = 0;
    CosFunction.bART_SECSGEM_93K = false; IniConfig.bA37LotStartLotEnd = false; IniConfig.bA10_AutoReTest = false;
    TestIF_File.bRENESAS_EnableFTCT = false;

    // bridge not found -> nothing
    g_bridge = false; fSCKART->iCurrent93KARTStep = -1; HHandler2Gpib.iLotStatus = -7;
    fMain->SetLotState(2);
    check(fSCKART->iCurrent93KARTStep == -1 && HHandler2Gpib.iLotStatus == -7 && g_sends == 0,
          "GPIB tester, bridge not found -> returns before touching anything (golden :15200-15201 fMain->bFind==false)");

    g_bridge = true;
    LastSet.bWaitStartLotAutoRetestGPIB = true; fAGV->bATK_AMR_DoLotEndSent = true; fSCKART->sLotStartTime = "";
    fMain->SetLotState(2);
    check(fSCKART->iCurrent93KARTStep == 2 && fSCKART->sLotStartTime.Length() == 14 && fAGV->bATK_AMR_DoLotEndSent == false &&
          LastSet.bWaitStartLotAutoRetestGPIB == false,
          "FT start (2): SCK ART step 2, lot-start time yyyymmddhhnnss, AMR DoLotEnd flag cleared, wait-start cleared (golden :15205-15220)");
    check(hLotStartTimeOut.Off() == false, "FT start: the 120 s lot-start timeout is armed (golden :15219)");
    check(HHandler2Gpib.iSendCommand == MSG_CMD_LotStatus && HHandler2Gpib.iLotStatus == 2,
          "HHandler2Gpib = MSG_CMD_LotStatus + iLotStatus 2 (golden :15257-15258)");
    check(g_sends == 0, "another customer, no ART / [GPIBLotEnd] -> the packet is not sent (golden :15396-15403 condition false)");

    fMain->SetLotState(4);
    check(fSCKART->iCurrent93KARTStep == 8 && HHandler2Gpib.iLotStatus == 4, "RT start (4): step 8 (golden :15221-15227)");
    fSCKART->iFTRTCount = 1;
    fMain->SetLotState(8);
    check(fSCKART->iCurrent93KARTStep == 4 && hLotEndTimeOut.Off() == false, "lot end (8), one FT/RT pass: step 4, lot-end timeout armed (golden :15228-15235)");
    fSCKART->iFTRTCount = 2;
    fMain->SetLotState(8);
    check(fSCKART->iCurrent93KARTStep == 10, "lot end (8) after more passes: step 10");
    fMain->SetLotState(10);
    check(fSCKART->iCurrent93KARTStep == 11, "final lot end (10): step 11 (golden :15236-15246)");

    CUSTOMER_CODE = CC_UTAC; g_sends = 0;
    fMain->SetLotState(8);
    check(g_sends == 1 && g_lastCmd == MSG_CMD_LotStatus && g_lastLotStatus == 8,
          "UTAC -> the MSG_CMD_LotStatus packet (iLotStatus 8) goes to the bridge (golden :15335-15338, SendMessage -> W906_TesterSendToBridge)");
    CUSTOMER_CODE = CC_SCK; g_sends = 0;
    fMain->SetLotState(2);
    check(g_sends == 1 && g_lastLotStatus == 2, "SCK at FT start -> sent (golden :15399-15400)");
    fMain->SetLotState(8);
    check(g_sends == 1, "SCK at lot end without ART / [GPIBLotEnd] -> not sent");
    CUSTOMER_CODE = CC_KYEC_LEE; CosFunction.bGPIBLotEnd = true;
    fMain->SetLotState(8);
    check(g_sends == 2 && g_lastLotStatus == 8, "[GPIBLotEnd] -> sent at lot end (golden :15401)");
    CosFunction.bGPIBLotEnd = false;

    TestIF.iTestType = 0; g_sends = 0; fSCKART->iCurrent93KARTStep = -1;
    fMain->SetLotState(2);
    check(fSCKART->iCurrent93KARTStep == -1 && g_sends == 0, "tester not GPIB (and not TCP/IP) -> returns (golden :15195-15198)");
    TestIF.iTestType = GPIB_MODE;
    TestIF_File.iTestType = TCP_IP_MODE; CUSTOMER_CODE = CC_SJ_Semiconductor_OS;
    fMain->SetLotState(2);
    check(fSCKART->iCurrent93KARTStep == -1 && g_sends == 0, "TCP/IP tester of SJ Semiconductor OS -> returns at once (golden :15166-15170)");
    TestIF_File.iTestType = 0; CUSTOMER_CODE = CC_KYEC_LEE;

    // Part 2: the source
    const std::string src = argc > 1 ? argv[1] : ".";
    const std::string fm = Read(src + "/forms/fMain.cpp"), sl = Read(src + "/forms/fMain_SetLotState.cpp");
    const std::string cs = Read(src + "/csystem.cpp"), ar = Read(src + "/AutoRetest.cpp"), sr = Read(src + "/Automation/SCK_ART_Remainder.cpp");
    check(fm.find("\nvoid TfMain::SetLotState(int /*iState*/) {}") == std::string::npos && fm.find("//void TfMain::SetLotState(int /*iState*/) {}") != std::string::npos,
          "forms/fMain.cpp: the empty body is retired (commented, same line)");
    check(cs.find("#define W7C2_FMAIN_SETLOTSTATE(n)          fMain->SetLotState(n)") != std::string::npos &&
          ar.find("#define W906ART_FMAIN_SETLOTSTATE(n)   fMain->SetLotState(n)") != std::string::npos &&
          sr.find("#define W5SCKARTREM_FMAIN_SETLOTSTATE(n)            fMain->SetLotState(n)") != std::string::npos,
          "the three TU-local no-op macros now call fMain->SetLotState (csystem.cpp / AutoRetest.cpp / SCK_ART_Remainder.cpp)");
    std::string code;                                   // the file without its // comments (they quote the golden text on purpose)
    {
        std::istringstream in(sl);
        std::string line;
        while (std::getline(in, line)) {
            const std::size_t c = line.find("//");
            code += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
        }
    }
    check(!code.empty() && code.find("fTesterTCP->") == std::string::npos && code.find("fMain->bFind") == std::string::npos &&
          code.find("SendMessage(") == std::string::npos && code.find("btClearBarcodeList->Click") == std::string::npos &&
          code.find("W906_TesterSendToBridge(pcp);") != std::string::npos && code.find("W906_TesterBridgeFound()") != std::string::npos,
          "fMain_SetLotState.cpp code (comments stripped): every golden VCL-only call is mapped (no fTesterTCP-> / bFind / SendMessage / ->Click left)");

    W906_TesterForward.BridgeFound = 0; W906_TesterBridgeWndHook = 0; W906_TesterSendToBridgeHook = 0;
    std::printf("%s SetLotState (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
