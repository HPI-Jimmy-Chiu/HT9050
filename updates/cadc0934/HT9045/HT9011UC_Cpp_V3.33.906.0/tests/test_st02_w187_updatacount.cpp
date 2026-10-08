// =============================================================================
//  test_st02_w187_updatacount.cpp -- W-187 (POOL-9 GOLDEN913-UPDATE) #1: TEST_CATEGORY::UpdataCount reads each arm's counts by
//  its arm-local row (golden 913 cSocket.cpp:1057-1286, Ifor 20260807 "Fix TesterCate Arm1 show 0") -- the way the writer
//  ProcessArmCount stores them (atester_ProcessCount.cpp: SetArmSKTData(iRow32, ...), iRow32 = i-2 / 0 / i).
//
//  AI(W906-W187) 20261009 (St02-E).  Suite name (add_test): St02_W187UpdataCount.  argv[1] = port root (source pin [3]).
//  In memory only: fresh TArm objects stand in for ArmData[0..1] / ArmDataLot[0..1] (restored at the end), counts seeded through the
//  real writer TArm::SetArmSKTData with arm-local rows, the real IsNNMode (cinitial.cpp) picks the branch from TestIF_File.iTestMode.
//    [1] NN_1Row (QualSite2X2N, 2 x 2): row 0 = arm 1 local row 0, row 1 = arm 0 local row 0.  ArmData (UpdataCount(true)) and
//        ArmDataLot (UpdataCount(false)): every site's total / bin / by-site counts come from its own arm's local row.
//    [2] two rows per arm (_16Site4X4, 4 x 2): rows 0-1 = arm 1 local rows 0-1, rows 2-3 = arm 0 local rows 0-1 (iRow-2).
//    [3] source: cSocket.cpp carries golden 913's four iSrcRow lines and 52 ArmSKET[iSrcRow][iCol].
// =============================================================================
#include "MachineType.h"
#include "cprod.h"               // TestIF_File, TestIF
#include "cmydef.h"              // iTestBinCount
#include "cSocket.h"             // TEST_CATEGORY, TArm, ArmData, ArmDataLot
#include "aHotPlateSubstrate.h"  // TestSocket
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
static TEST_CATEGORY tc;   // big arrays: not on the stack
static std::string N(int v) { return std::to_string(v); }

// n test results of `bin` into arm `arm` at its local row/col, in ArmData and ArmDataLot (as ProcessArmCount :2110-2111)
static void Seed(int arm, int localRow, int col, int bin, int n)
{
    for (int i = 0; i < n; ++i) {
        ArmData[arm]->SetArmSKTData(localRow, col, bin);
        ArmDataLot[arm]->SetArmSKTData(localRow, col, bin);
    }
}
static void Fresh(const char* tag)
{
    ArmData[0] = new TArm((std::string("W187-") + tag + "-A0").c_str());
    ArmData[1] = new TArm((std::string("W187-") + tag + "-A1").c_str());
    ArmDataLot[0] = new TArm((std::string("W187-") + tag + "-L0").c_str());
    ArmDataLot[1] = new TArm((std::string("W187-") + tag + "-L1").c_str());
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W187UpdataCount -- TEST_CATEGORY::UpdataCount reads by the arm-local row (golden 913 cSocket.cpp:1057-1286)\n");
    TArm* const sv[4] = { ArmData[0], ArmData[1], ArmDataLot[0], ArmDataLot[1] };
    const int svMode = TestIF_File.iTestMode, svRow = TestSocket.iShtRow, svCol = TestSocket.iShtCol, svBins = iTestBinCount;
    int svMap[4][2];
    for (int r = 0; r < 4; ++r) for (int c = 0; c < 2; ++c) svMap[r][c] = TestIF.iSiteMap[r][c];
    iTestBinCount = 8;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] NN_1Row (QualSite2X2N, 2 x 2)\n");
    Fresh("1");
    TestIF_File.iTestMode = QualSite2X2N;
    TestSocket.iShtRow = 2; TestSocket.iShtCol = 2;
    TestIF.iSiteMap[0][0] = 1; TestIF.iSiteMap[0][1] = 2; TestIF.iSiteMap[1][0] = 3; TestIF.iSiteMap[1][1] = 4;
    Seed(1, 0, 0, 1, 3);   // row 0 col 0 (DUT 1): arm 1 local row 0
    Seed(1, 0, 1, 2, 4);   // row 0 col 1 (DUT 2)
    Seed(0, 0, 0, 3, 5);   // row 1 col 0 (DUT 3): arm 0 local row 0
    Seed(0, 0, 1, 1, 6);   // row 1 col 1 (DUT 4)
    for (int pass = 0; pass < 2; ++pass) {
        const bool yield = pass == 0;
        tc.UpdataCount(yield);
        const std::string w = yield ? "[1] ArmData (UpdataCount(true)): " : "[1] ArmDataLot (UpdataCount(false)): ";
        Check(tc.iCountSocketTotal[0][0] == 3 && tc.iCountSocketTotal[0][1] == 4 && tc.iCountSocketTotal[1][0] == 5 && tc.iCountSocketTotal[1][1] == 6,
              w + "socket totals 3 / 4 / 5 / 6 (got " + N(tc.iCountSocketTotal[0][0]) + " / " + N(tc.iCountSocketTotal[0][1]) + " / " +
              N(tc.iCountSocketTotal[1][0]) + " / " + N(tc.iCountSocketTotal[1][1]) + ")");
        Check(tc.iCountCategory[0][1][0][3] == 5 && tc.iCountCategory[0][1][1][1] == 6 && tc.iCountHeadTotal[0][1][0] == 5,
              w + "arm 0's sites (row 1) read from arm 0's local row 0: bin 3 = 5, bin 1 = 6");
        Check(tc.iBySiteTotal[2] == 5 && tc.iBySiteTotal[3] == 6 && tc.iBySiteCate[2][3] == 5 && tc.iTotalSocket == 18 &&
                  tc.iTotalCategory[1] == 9 && tc.iTotalCategory[2] == 4 && tc.iTotalCategory[3] == 5,
              w + "by-site DUT 3 / 4 = 5 / 6, machine total 18, bin totals 9 / 4 / 5 (total " + N(tc.iTotalSocket) + ")");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] two rows per arm (_16Site4X4, 4 x 2)\n");
    Fresh("2");
    TestIF_File.iTestMode = _16Site4X4;
    TestSocket.iShtRow = 4; TestSocket.iShtCol = 2;
    for (int r = 0; r < 4; ++r) for (int c = 0; c < 2; ++c) TestIF.iSiteMap[r][c] = r * 2 + c + 1;
    Seed(1, 1, 0, 3, 1);   // row 1 col 0 (DUT 3): arm 1 local row 1
    Seed(0, 0, 0, 1, 7);   // row 2 col 0 (DUT 5): arm 0 local row 0 (= iRow-2)
    Seed(0, 1, 1, 2, 2);   // row 3 col 1 (DUT 8): arm 0 local row 1
    for (int pass = 0; pass < 2; ++pass) {
        const bool yield = pass == 0;
        tc.UpdataCount(yield);
        const std::string w = yield ? "[2] ArmData: " : "[2] ArmDataLot: ";
        Check(tc.iCountSocketTotal[1][0] == 1 && tc.iCountSocketTotal[2][0] == 7 && tc.iCountSocketTotal[3][1] == 2 && tc.iTotalSocket == 10,
              w + "socket totals row1/col0 = 1, row2/col0 = 7, row3/col1 = 2, total 10 (got " + N(tc.iCountSocketTotal[1][0]) + " / " +
              N(tc.iCountSocketTotal[2][0]) + " / " + N(tc.iCountSocketTotal[3][1]) + ", " + N(tc.iTotalSocket) + ")");
        Check(tc.iBySiteTotal[4] == 7 && tc.iBySiteTotal[7] == 2 && tc.iCountCategory[0][2][0][1] == 7 && tc.iCountCategory[0][3][1][2] == 2,
              w + "arm 0's rows 2-3 read from its local rows 0-1 (iRow-2): DUT 5 = 7, DUT 8 = 2");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] source pin\n");
    {
        std::ifstream f((std::string(argc > 1 ? argv[1] : "") + "/cSocket.cpp").c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string s = ss.str();
        auto count = [&](const std::string& k) { int n = 0; for (size_t p = s.find(k); p != std::string::npos; p = s.find(k, p + 1)) ++n; return n; };
        Check(count("iSrcRow=(iArm==0)?0:iRow;") == 2 && count("iSrcRow=(iArm==0)?(iRow-2):iRow;") == 2 && count("ArmSKET[iSrcRow][iCol]") == 52,
              "[3] cSocket.cpp: golden 913's iSrcRow lines (2 + 2) and 52 ArmSKET[iSrcRow][iCol]");
    }

    ArmData[0] = sv[0]; ArmData[1] = sv[1]; ArmDataLot[0] = sv[2]; ArmDataLot[1] = sv[3];
    TestIF_File.iTestMode = svMode; TestSocket.iShtRow = svRow; TestSocket.iShtCol = svCol; iTestBinCount = svBins;
    for (int r = 0; r < 4; ++r) for (int c = 0; c < 2; ++c) TestIF.iSiteMap[r][c] = svMap[r][c];
    std::printf("St02_W187UpdataCount: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
