// =============================================================================
//  test_l08_nnhead.cpp -- AI(W906-L08) 20261010 (Ifor01)
//
//  L08 (TO_IFOR 1010 13:5x; St02-M scout ST02_SCOUT_20261010.md §3): golden 913 GetNNHeadBySite (Command.cpp:86-153, RogerYang
//  20260909) -- TfMain::RefreshTempData(true, arm, site) in the NN modes returns the site's heater head (it returned iTempKit0, always 0,
//  so SaveRemoteTempOffset -- W190_NnRemoteOffset -- never found the head in 2x2N / 2x3N / 2x4N).  Through the real fMain:
//    [1] 2x2N, 16 heaters: arm 1 = row 1 -> tcAa1 / tcAb1, arm 2 = row 0 -> tcAa2 / tcAb2; a site of the other arm / site 0 -> -1
//    [2] 2x3N, 4-heater machine: arm 1 cols 0,1 -> tcHead1 (split 2), col 2 -> tcHead2; arm 2 col 2 -> tcHead4
//    [3] 2x4N, 16 heaters: col 3 -> tcAd1 / tcAd2; 4-heater: arm 1 col 1 -> tcHead1, col 2 -> tcHead2
//    [4] source pins (argv[1] = port root, read only): the six assignments call GetNNHeadBySite with golden's (cols, split)
// =============================================================================
#include "forms/fMain.h"
#include "cprod.h"
#include "cmydef.h"
#include "MachineType.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
static int Count(const std::string& s, const std::string& k) { int n = 0; for (size_t p = s.find(k); p != std::string::npos; p = s.find(k, p + 1)) n++; return n; }
static void Map(const int* row1, const int* row0, int cols)
{
    std::memset(TestIF_File.iSiteMap, 0, sizeof(TestIF_File.iSiteMap));
    for (int c = 0; c < cols; c++) { TestIF_File.iSiteMap[1][c] = row1[c]; TestIF_File.iSiteMap[0][c] = row0[c]; }
}
static int H(int arm, int site) { return fMain->RefreshTempData(true, arm, site); }

int main(int argc, char** argv)
{
    std::printf("L08_NnHeadBySite\n");
    Check(fMain != 0, "0. fMain exists");
    if (fMain == 0) return 1;
    const int savedMode = TestIF_File.iTestMode, savedHeater = USE_16_HEATER;
    int savedMap[sizeof(TestIF_File.iSiteMap) / sizeof(int)];
    std::memcpy(savedMap, TestIF_File.iSiteMap, sizeof(TestIF_File.iSiteMap));

    {   // [1] 2x2N, 16 heaters
        const int r1[] = {1, 2}, r0[] = {3, 4};
        Map(r1, r0, 2); TestIF_File.iTestMode = QualSite2X2N; USE_16_HEATER = eht16Heater;
        Check(H(1, 1) == tcAa1 && H(1, 2) == tcAb1, "1a. 2x2N 16 heaters: arm 1 (row 1) sites 1 / 2 -> tcAa1 / tcAb1 (golden 913 :122-131)");
        Check(H(2, 3) == tcAa2 && H(2, 4) == tcAb2, "1b. arm 2 (row 0) sites 3 / 4 -> tcAa2 / tcAb2 (:133-142)");
        Check(H(2, 1) == -1 && H(1, 0) == -1 && H(1, 9) == -1, "1c. a site of the other arm, site 0, an unmapped site -> -1 (:98-113)");
    }
    {   // [2] 2x3N, 4-heater machine
        const int r1[] = {1, 2, 3}, r0[] = {4, 5, 6};
        Map(r1, r0, 3); TestIF_File.iTestMode = _6Site2X3N; USE_16_HEATER = eht4Heater;
        Check(H(1, 1) == tcHead1 && H(1, 2) == tcHead1 && H(1, 3) == tcHead2, "2a. 2x3N heads: arm 1 cols 0,1 -> tcHead1, col 2 -> tcHead2 (split 2, :147-148)");
        Check(H(2, 4) == tcHead3 && H(2, 6) == tcHead4, "2b. arm 2 col 0 -> tcHead3, col 2 -> tcHead4 (:149-150)");
    }
    {   // [3] 2x4N
        const int r1[] = {1, 2, 3, 4}, r0[] = {5, 6, 7, 8};
        Map(r1, r0, 4); TestIF_File.iTestMode = _8Site2X4N; USE_16_HEATER = eht16Heater;
        Check(H(1, 4) == tcAd1 && H(2, 8) == tcAd2 && H(2, 7) == tcAc2, "3a. 2x4N 16 heaters: col 3 -> tcAd1 / tcAd2, col 2 -> tcAc2");
        USE_16_HEATER = eht4Heater;
        Check(H(1, 2) == tcHead1 && H(1, 3) == tcHead2 && H(2, 5) == tcHead3, "3b. 2x4N heads: split 2 (arm 1 col 1 -> tcHead1, col 2 -> tcHead2)");
    }
    TestIF_File.iTestMode = savedMode; USE_16_HEATER = savedHeater;
    std::memcpy(TestIF_File.iSiteMap, savedMap, sizeof(TestIF_File.iSiteMap));

    if (argc > 1) {
        const std::string cm = Slurp(std::string(argv[1]) + "/Command.cpp");
        Check(Count(cm, "iHead=GetNNHeadBySite(iArm, iSite, 2, 1);") == 2 && Count(cm, "iHead=GetNNHeadBySite(iArm, iSite, 3, 2);") == 2
              && Count(cm, "iHead=GetNNHeadBySite(iArm, iSite, 4, 2);") == 2,
              "4a. the three NN branches call GetNNHeadBySite with golden's (cols, split) = (2,1) / (3,2) / (4,2) (golden 913 :240 / :283 / :334)");
        Check(Count(cm, "static int GetNNHeadBySite(int iArm, int iSite, int iColCount, int iHeadSplitCol)") == 2,
              "4b. one forward declaration + one body (golden 913 :96)");
    } else std::printf("  (no argv[1]: source pins skipped)\n");

    std::printf("L08_NnHeadBySite: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
