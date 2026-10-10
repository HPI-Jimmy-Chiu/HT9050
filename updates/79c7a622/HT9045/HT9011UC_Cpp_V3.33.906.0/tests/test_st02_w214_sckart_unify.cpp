// =============================================================================
//  test_st02_w214_sckart_unify.cpp -- W-214 (POOL-12 SCKART-UNIFY): TfSCKART is golden's ONE fSCKART and the one lot state.
//
//  AI(W906-W214) 20261010 (St02-E).  Suite name (add_test): St02_W214SckartUnify.  argv[1] = port root (reads csystem.cpp).
//  Design docs/handoff/ST02_W214_SCKART_UNIFY_DESIGN_20261010.md option A (laptop 06:2x): csystem.cpp's W7C1 / W7C2 seams forward
//  SetLotStatus to fSCKART and read its status / constants at every call site; tests/test_w7_f2_sckart_state.cpp PART C C1-C4 / C9 pin
//  the seam side.  In memory only.  Design §3 item 2 (lot end -> GPIB LOTSTATUS_L end to end) comes with W-213 step 2 (G8 / G9).
//    [1] golden constants on TfSCKART: NONE 0, W 1, T 2, L 3, R 4, F 5, A 6 (golden Automation/SCK_ART.cpp:43-49); sLOTSTATUS "NONE"
//        and GetLotStatus() "NONE" at construction (:51).
//    [2] one lot-status meaning: for every iStatus -1..7, TfSCKART::SetLotStatus -> GetLotStatus() / iCurrentStatus equal
//        SckArt_SetLotStatus -> SckArt_GetLotStatus / iCurrentStatus (the forms-layer switch and Automation/SCK_ART.cpp's, both golden
//        SCK_ART.cpp:641-665, pinned equal).
//    [3] GPIB side -> csystem side: a status set on the global fSCKART (what HandlerGpibMsg's G2 / G4 / G6, and later G7-G11 / G13,
//        write) is what DoART_AfterCleanOut compares: csystem.cpp's code (comments dropped) compares fSCKART->iCurrentStatus at its
//        four ART sites and no longer reads W7C2_SCKART->iCurrentStatus.
//    [4] iWaitGPIBLotR is one field too: csystem.cpp reads / writes fSCKART->iWaitGPIBLotR (:3202 / :3253), no W7C1 shadow left.
// =============================================================================
#include "forms/fSCKART.h"              // TfSCKART, fSCKART
#include "Automation/SCK_ART.h"         // SckArtState, SckArt_SetLotStatus, SckArt_GetLotStatus

#include <cstdio>
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
std::string Txt(const AnsiString& a) { return std::string(a.c_str()); }
// the file with each line's `//` tail dropped (enough for csystem.cpp's code lines; no `//` inside a string on the lines counted)
std::string CodeOnly(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string out, line;
    while (std::getline(f, line)) {
        const size_t c = line.find("//");
        out += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
    }
    return out;
}
int Count(const std::string& s, const std::string& needle)
{
    int n = 0;
    for (size_t at = s.find(needle); at != std::string::npos; at = s.find(needle, at + 1)) ++n;
    return n;
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W214SckartUnify -- TfSCKART is the one lot state (W-214)\n");
    const std::string root = argc > 1 ? argv[1] : std::string();

    // ---------------------------------------------------------------- [1]
    std::printf("[1] golden constants on TfSCKART (golden Automation/SCK_ART.cpp:43-49, :51)\n");
    {
        TfSCKART t;
        Check(t.iLOTSTATUS_NONE == 0 && t.iLOTSTATUS_W == 1 && t.iLOTSTATUS_T == 2 && t.iLOTSTATUS_L == 3 && t.iLOTSTATUS_R == 4 &&
                  t.iLOTSTATUS_F == 5 && t.iLOTSTATUS_A == 6,
              "[1] NONE..A = 0..6 (got " + std::to_string(t.iLOTSTATUS_NONE) + std::to_string(t.iLOTSTATUS_W) + std::to_string(t.iLOTSTATUS_T) +
                  std::to_string(t.iLOTSTATUS_L) + std::to_string(t.iLOTSTATUS_R) + std::to_string(t.iLOTSTATUS_F) + std::to_string(t.iLOTSTATUS_A) + ")");
        Check(Txt(t.sLOTSTATUS) == "NONE" && Txt(t.GetLotStatus()) == "NONE", "[1] sLOTSTATUS / GetLotStatus() \"NONE\" at construction");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] TfSCKART::SetLotStatus == SckArt_SetLotStatus for -1..7 (golden SCK_ART.cpp:641-665)\n");
    {
        int good = 0;
        for (int i = -1; i <= 7; ++i) {
            TfSCKART t;
            SckArtState st;
            t.SetLotStatus(i);
            SckArt_SetLotStatus(st, i);
            const bool ok = Txt(t.GetLotStatus()) == Txt(SckArt_GetLotStatus(st)) && t.iCurrentStatus == i && st.iCurrentStatus == i;
            if (ok) ++good;
            else std::printf("    iStatus %d: facade \"%s\" / %d, SckArt \"%s\" / %d\n", i, t.GetLotStatus().c_str(), t.iCurrentStatus,
                             SckArt_GetLotStatus(st).c_str(), st.iCurrentStatus);
        }
        Check(good == 9, "[2] all 9 statuses give the same string and iCurrentStatus (" + std::to_string(good) + "/9)");
        TfSCKART t;
        t.SetLotStatus(t.iLOTSTATUS_L);
        Check(Txt(t.GetLotStatus()) == "LOTSTATUS_L" && t.iCurrentStatus == 3, "[2] SetLotStatus(iLOTSTATUS_L) -> \"LOTSTATUS_L\", 3 (lot end, golden)");
    }

    const std::string cs = CodeOnly(root + "/csystem.cpp");
    Check(cs.size() > 1000000, "setup: csystem.cpp read (" + std::to_string(cs.size()) + " bytes of code)");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] GPIB side -> csystem side: one iCurrentStatus\n");
    {
        const int saved = fSCKART->iCurrentStatus;
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_F);
        Check(Txt(fSCKART->GetLotStatus()) == "LOTSTATUS_F" && fSCKART->iCurrentStatus == 5,
              "[3] the global fSCKART holds F after SetLotStatus(iLOTSTATUS_F) (what the GPIB handlers write)");
        fSCKART->SetLotStatus(saved);
        const int nLive = Count(cs, "fSCKART->iCurrentStatus!=fSCKART->iLOTSTATUS_A") + Count(cs, "fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_R");
        const int nShadow = Count(cs, "W7C2_SCKART->iCurrentStatus") + Count(cs, "W7C2_SCKART->iLOTSTATUS_") + Count(cs, "W7C1_SCKART->iLOTSTATUS_");
        Check(nLive == 4 && nShadow == 0, "[3] csystem.cpp code: DoART_AfterCleanOut's 4 ART comparisons read fSCKART (" + std::to_string(nLive) +
                                              "), no seam status / constant read left (" + std::to_string(nShadow) + ")");
        Check(Count(cs, "SetLotStatus(int iStatus){ fSCKART->SetLotStatus(iStatus);") == 2,
              "[3] both seam SetLotStatus forward to fSCKART->SetLotStatus first");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] iWaitGPIBLotR: one field\n");
    {
        Check(Count(cs, "fSCKART->iWaitGPIBLotR==0") >= 1 && Count(cs, "fSCKART->iWaitGPIBLotR=1;") >= 1 && Count(cs, "W7C1_SCKART->iWaitGPIBLotR") == 0,
              "[4] csystem.cpp reads / writes fSCKART->iWaitGPIBLotR (:3202 / :3253); no W7C1 shadow read or write left");
    }

    std::printf("St02_W214SckartUnify: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
