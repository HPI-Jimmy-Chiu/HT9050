// =============================================================================
//  test_st02_w213_sckart_gpib.cpp -- W-213 step 2: the tester sees the one lot state (W-214) through HandlerGpibMsg G8-G11 and
//  HandlerBridgeCtl G13, as golden 913 main.cpp:16266-16359 / :19209-19214.
//
//  AI(W906-W213) 20261010 (St02-E).  Suite name (add_test): St02_W213SckartGpib.  argv[1] = port root (reads csystem.cpp,
//  TesterComm/Handler/HandlerBridgeCtl.cpp, TesterComm/Handler/HandlerGpibMsg.cpp).  Stacked on W-214 (!417): TfSCKART::GetLotStatus,
//  iLOTSTATUS_T / _L / _F and csystem.cpp's lot end writing fSCKART.  Packets go through THandlerTesterSide::ProcessARTMessage as the
//  GPIB thread hands them over (the tests/test_st02_w132_sckart.cpp way).  bUseSCKART stays false, so AccessFile is a no-op (golden
//  :195) and nothing is forwarded to the ART bridge; the machine's lastdata*.dat / config.ini are compared byte for byte.
//    [1] lot end -> LOTSTATUS reply: csystem.cpp's lot end is `W7C1_SCKART->SetLotStatus(fSCKART->iLOTSTATUS_L);` (:3200 / :3251,
//        the seam forwards to fSCKART -- pinned in source); running that statement's effect on fSCKART, then MSG_CMD_SCKART_LOTSTATUS
//        with no lot replies "0,0,LOTSTATUS_L" (G8, golden :16278; before: an empty reply).
//    [2] with a lot: sLotID "LOT1", iFTRTCount 2 -> "2,LOT1,LOTSTATUS_L" (G9, golden :16282); status L -> nothing else changes (G10 :16294).
//    [3] G10, status F: no IC under the machine -> iWaitGPIBLotR 4, status stays F (golden :16297-16306).
//    [4] MSG_CMD_SCKART_INITIAL (G11, golden :16354-16359): A -> F, W -> F, R -> unchanged.
//    [5] HandlerBridgeCtl.cpp RunTestProgram: G13 live -- `fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_T);` under `if(bNeedTest)` outside any
//        `#if 0` (golden :19211-19212); source pin (RunTestProgram starts the tester program, not run here).
//    [6] G7 (MSG_CMD_SCKART_Alarm) still closed: TfSCKART::CheckNeedRT is an offline no-op (source pin).
// =============================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "forms/fSCKART.h"
#include "forms/fMain.h"
#include "MessageDef.h"
#include "CosFunction.h"
#include "Config.h"
#include "cmydef.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
std::string Slurp(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return "<missing>";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
// one ART packet through the handler; returns the reply text (CR / LF shown as ~)
std::string Packet(int cmd)
{
    VM vm;
    std::memset(&vm, 0, sizeof(vm));
    vm.iCommand = cmd;
    HGpib2Handler = &vm;
    std::memset(HHandler2Gpib.Message, 0, sizeof(HHandler2Gpib.Message));
    THandlerTesterSide side;
    side.ProcessARTMessage();
    HGpib2Handler = 0;
    std::string r = HHandler2Gpib.Message;
    for (size_t i = 0; i < r.size(); ++i) if (r[i] == '\r' || r[i] == '\n') r[i] = '~';
    return r;
}
// the line holding `needle` is live: not between a `#if 0` and its `#endif` (simple scan, enough for these flat gates)
bool LiveLine(const std::string& src, const std::string& needle, const std::string& after)
{
    const size_t a = after.empty() ? 0 : src.find(after);
    if (a == std::string::npos) return false;
    const size_t at = src.find(needle, a);
    if (at == std::string::npos) return false;
    const size_t if0 = src.rfind("\n#if 0", at), endif = src.rfind("\n#endif", at);
    return if0 == std::string::npos || (endif != std::string::npos && endif > if0);
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W213SckartGpib -- the tester sees the one lot state (W-213 step 2)\n");
    const std::string root = argc > 1 ? argv[1] : std::string();
    static const char* const kReal[4] = {"D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\lastdata_backup.dat",
                                         "D:\\HT9045\\system\\lastdata_backup2.dat", "D:\\HT9045\\config\\config.ini"};
    std::string before[4];
    for (int i = 0; i < 4; ++i) before[i] = Slurp(kReal[i]);

    Check(fSCKART != 0 && fMain != 0 && fMain->palMainStatus != 0, "setup: the fSCKART / fMain facades exist");
    CosFunction.bUseSCKART    = false;   // AccessFile no-op (golden :195); LOTSTATUS is not forwarded to the bridge
    IniConfig.bA10_AutoReTest = false;
    fMain->palMainStatus->Caption = "RUN";   // not "HALT": golden :16270 would reset a lot-less status to NONE before the reply
    fSCKART->sLotID = "";
    fSCKART->iFTRTCount = 0;
    fSCKART->iWaitGPIBLotR = 0;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] lot end -> LOTSTATUS reply (golden 913 main.cpp:16278)\n");
    {
        const std::string cs = Slurp(root + "/csystem.cpp");
        size_t n = 0;
        for (size_t at = cs.find("W7C1_SCKART->SetLotStatus(fSCKART->iLOTSTATUS_L);"); at != std::string::npos;
             at = cs.find("W7C1_SCKART->SetLotStatus(fSCKART->iLOTSTATUS_L);", at + 1)) ++n;
        Check(n == 2 && cs.find("void SetLotStatus(int iStatus){ fSCKART->SetLotStatus(iStatus);") != std::string::npos,
              "[1] csystem.cpp: both lot-end sites pass fSCKART's L and the seam forwards to fSCKART (W-214)");
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_L);   // what those two sites now do to the one lot state
        const std::string r = Packet(MSG_CMD_SCKART_LOTSTATUS);
        Check(r == "0,0,LOTSTATUS_L", "[1] no lot: reply \"0,0,LOTSTATUS_L\" (got \"" + r + "\")");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] with a lot (golden :16282) -- G10 leaves L alone\n");
    {
        fSCKART->sLotID = "LOT1";
        fSCKART->iFTRTCount = 2;
        fSCKART->iWaitGPIBLotR = 0;
        const std::string r = Packet(MSG_CMD_SCKART_LOTSTATUS);
        Check(r == "2,LOT1,LOTSTATUS_L", "[2] reply \"2,LOT1,LOTSTATUS_L\" (got \"" + r + "\")");
        Check(fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_L && fSCKART->iWaitGPIBLotR == 0,
              "[2] status L: still L, iWaitGPIBLotR untouched (got " + std::to_string(fSCKART->iCurrentStatus) + " / " +
                  std::to_string(fSCKART->iWaitGPIBLotR) + ")");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] G10, status F, no IC under the machine (golden :16297-16306)\n");
    {
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_F);
        fSCKART->iWaitGPIBLotR = 0;
        const std::string r = Packet(MSG_CMD_SCKART_LOTSTATUS);
        Check(r == "2,LOT1,LOTSTATUS_F" && fSCKART->iCurrentStatus == fSCKART->iLOTSTATUS_F && fSCKART->iWaitGPIBLotR == 4,
              "[3] reply \"2,LOT1,LOTSTATUS_F\", status stays F, iWaitGPIBLotR 4 (got \"" + r + "\", " +
                  std::to_string(fSCKART->iCurrentStatus) + ", " + std::to_string(fSCKART->iWaitGPIBLotR) + ")");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] MSG_CMD_SCKART_INITIAL (G11, golden :16354-16359)\n");
    {
        const int from[3] = {fSCKART->iLOTSTATUS_A, fSCKART->iLOTSTATUS_W, fSCKART->iLOTSTATUS_R};
        const int want[3] = {fSCKART->iLOTSTATUS_F, fSCKART->iLOTSTATUS_F, fSCKART->iLOTSTATUS_R};
        const char* name[3] = {"A -> F", "W -> F", "R -> unchanged"};
        const char* wantS[3] = {"LOTSTATUS_F", "LOTSTATUS_F", "LOTSTATUS_R"};
        for (int i = 0; i < 3; ++i) {
            fSCKART->SetLotStatus(from[i]);
            Packet(MSG_CMD_SCKART_INITIAL);
            Check(fSCKART->iCurrentStatus == want[i] && std::string(fSCKART->GetLotStatus().c_str()) == wantS[i],
                  std::string("[4] ") + name[i] + " (got " + std::to_string(fSCKART->iCurrentStatus) + ", \"" + fSCKART->GetLotStatus().c_str() + "\")");
        }
    }

    // ---------------------------------------------------------------- [5] / [6]
    std::printf("[5] HandlerBridgeCtl G13 / [6] HandlerGpibMsg G7\n");
    {
        const std::string bc = Slurp(root + "/TesterComm/Handler/HandlerBridgeCtl.cpp");
        Check(LiveLine(bc, "fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_T);", "bool THandlerTesterSide::RunTestProgram("),
              "[5] RunTestProgram: SetLotStatus(iLOTSTATUS_T) is live under if(bNeedTest) (golden 913 main.cpp:19211-19212)");
        const std::string gm = Slurp(root + "/TesterComm/Handler/HandlerGpibMsg.cpp");
        Check(!LiveLine(gm, "fSCKART->CheckNeedRT();", "MSG_CMD_SCKART_Alarm"),
              "[6] MSG_CMD_SCKART_Alarm: G7 (CheckNeedRT -> R / F) still inside its #if 0 (CheckNeedRT is an offline no-op)");
    }

    bool same = true;
    for (int i = 0; i < 4; ++i) same = same && Slurp(kReal[i]) == before[i];
    Check(same, "real files: D:\\HT9045\\system\\lastdata*.dat and D:\\HT9045\\config\\config.ini are byte-identical");
    std::printf("St02_W213SckartGpib: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
