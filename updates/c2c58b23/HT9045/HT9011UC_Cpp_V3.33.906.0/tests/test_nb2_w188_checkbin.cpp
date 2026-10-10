// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #8 -- CheckBin = golden 913 (GitLab honprec/rd/rd5/ht9045_913 main e9908638)
//  aoutarm.h:59 / aoutarm.cpp:654 (+ bool bPredictOnly, default false; RogerYang 20260823i), :706-726 (iNoBinToErrCT:
//  one "Message" line per 8 consecutive pieces whose bin has no valid target and silently go to the Error tray;
//  RogerYang 20260804), :787 (no WAR07356 in predict mode).
//  [1] trace: 7 no-bin pieces -> no line; the 8th -> one line "No valid bin to Error tray : bin=16, Sht=1, consecutive=8 pcs";
//      a valid bin resets the run; predict-only calls neither count nor reset.
//  [2] WAR07356: raised by a normal call, not by a predict-only call; both return the same target tray.
//  [3] source ratchets (argv[1] = source root).
//  Use: only through ctest (NB2_W188CheckBin).
// =============================================================================
#include "aoutarm.h"
#include "cmydef.h"
#include "cprod.h"
#include "cpublic.h"
#include "canary_support.h"
#include "st02_test_containment.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern int        W906_MyDBIProcess_Count;
extern AnsiString W906_MyDBIProcess_LastS1;
extern AnsiString W906_MyDBIProcess_LastS2;
void W906_MyDBIProcess_Reset();

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> v;
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        v.push_back(s);
    }
    return v;
}
static int Count(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}

static int Bad(int iSht, bool bPredict = false) { int ct = iTestBinCount; return CheckBin(ct, iSht, bPredict); }
static int Good(bool bPredict = false)          { int ct = 0;             return CheckBin(ct, 1, bPredict); }

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestInsideCtestRoots("NB2_W188CheckBin"))
        return 2;
    std::printf("==== W-188 #8 CheckBin -> golden 913 ====\n");

    char msg[200];
    Prod.iIfErrorT6   = eAuto6;
    Prod.iT6CatData[0] = eAuto1;
    iHWFix_BinBox     = 0;
    bCancelErrorBin   = false;

    // ---------------------------------------------------------------- [1] trace every 8 consecutive no-bin pieces
    std::printf("-- [1] no-bin trace --\n");
    LastSet.iRealDummy = DUMMY;                                  // no WAR07356 path in this block
    Good();
    W906_MyDBIProcess_Reset();
    for (int i = 0; i < 7; i++) Bad(1);
    std::snprintf(msg, sizeof msg, "7 consecutive no-bin pieces: no line (%d)", W906_MyDBIProcess_Count);
    CHECK(W906_MyDBIProcess_Count == 0, msg);
    const int r8 = Bad(1);
    CHECK(W906_MyDBIProcess_Count == 1 && W906_MyDBIProcess_LastS1 == AnsiString("Message") &&
          W906_MyDBIProcess_LastS2 == AnsiString("No valid bin to Error tray : bin=16, Sht=1, consecutive=8 pcs"),
          "the 8th: one Message line with bin / shuttle / consecutive count");
    CHECK(r8 == eAuto6, "no-bin piece still goes to the Error tray (Prod.iIfErrorT6)");
    for (int i = 0; i < 8; i++) Bad(0);
    CHECK(W906_MyDBIProcess_Count == 2 && W906_MyDBIProcess_LastS2 == AnsiString("No valid bin to Error tray : bin=16, Sht=0, consecutive=16 pcs"),
          "the 16th: second line, consecutive=16");

    Good();                                                      // valid bin resets the run
    for (int i = 0; i < 7; i++) Bad(1);
    CHECK(W906_MyDBIProcess_Count == 2, "after a valid bin the run restarts: 7 more -> no line");
    Bad(1);
    CHECK(W906_MyDBIProcess_Count == 3 && W906_MyDBIProcess_LastS2 == AnsiString("No valid bin to Error tray : bin=16, Sht=1, consecutive=8 pcs"),
          "8 after the reset -> consecutive=8 again");

    Good();
    for (int i = 0; i < 4; i++) Bad(1);
    for (int i = 0; i < 8; i++) Bad(1, true);                    // predict-only: not counted
    Good(true);                                                  // predict-only valid bin: does not reset
    CHECK(W906_MyDBIProcess_Count == 3, "predict-only calls neither count nor log");
    for (int i = 0; i < 4; i++) Bad(1);
    CHECK(W906_MyDBIProcess_Count == 4 && W906_MyDBIProcess_LastS2 == AnsiString("No valid bin to Error tray : bin=16, Sht=1, consecutive=8 pcs"),
          "predict-only calls do not reset: 4 + 4 normal pieces -> consecutive=8");

    // ---------------------------------------------------------------- [2] WAR07356 only outside predict mode
    std::printf("-- [2] WAR07356 --\n");
    LastSet.iRealDummy = REALLY;
    LastSet.iTester    = ON_LINE;
    iHWFix_BinBox      = 1;                                      // Prod.iIfErrorT6 != eBulkBox -> bShowMess
    const int a0 = W906_ShowErrorMessage_Count;
    const int rp = Bad(1, true);
    std::snprintf(msg, sizeof msg, "predict-only: no WAR07356 (%d calls)", W906_ShowErrorMessage_Count - a0);
    CHECK(W906_ShowErrorMessage_Count == a0, msg);
    const int rn = Bad(1);
    CHECK(W906_ShowErrorMessage_Count == a0 + 1 && W906_ShowErrorMessage_LastCode == AnsiString("WAR07356"),
          "normal call: WAR07356 raised");
    CHECK(rp == rn && rn == eAuto6, "predict and normal return the same target tray");
    LastSet.iRealDummy = DUMMY; iHWFix_BinBox = 0;

    // ---------------------------------------------------------------- [3] source ratchets
    std::printf("-- [3] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> H = ReadLines(root + "/aoutarm.h");
    const std::vector<std::string> C = ReadLines(root + "/aoutarm.cpp");
    CHECK(Count(H, "int  CheckBin(int &ct, int iShuttle, bool bPredictOnly=false);") == 1, "header: bPredictOnly defaults to false");
    CHECK(Count(C, "int CheckBin(int &ct, int iShuttle, bool bPredictOnly)") == 1, "definition takes bPredictOnly");
    CHECK(Count(C, "    static int iNoBinToErrCT=0; if(bPredictOnly==false) { if(ct>iTestBinCount-1) { iNoBinToErrCT++; if(iNoBinToErrCT%8==0)") == 1,
          "no-bin counter in code (not after a comment)");
    CHECK(Count(C, "    if(bShowMess && bAlarm && bPredictOnly==false)") == 1, "WAR07356 gated by bPredictOnly");

    std::printf("==== W-188 #8: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
