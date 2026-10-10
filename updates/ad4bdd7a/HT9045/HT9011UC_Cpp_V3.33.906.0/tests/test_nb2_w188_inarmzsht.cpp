// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #10 -- GetInArmZShtDownPos_9045 = golden 913 (GitLab honprec/rd/rd5/ht9045_913
//  main e9908638) ainarm9045.cpp:532-592 (Ifor 20260806): the in-arm Z height to a shuttle is taken from the nozzle's
//  PHYSICAL slot (InArmSuck.Suck[i][j].iMyRow / iMyCol) -- DualSite one-suck remaps logical slot [0][1] to another
//  physical nozzle, whose teach value must be used.  0618 used the logical [i][j].
//  [1] identity mapping: unchanged.  [2] slot [0][1] remapped to physical [1][3]: place / shuttle 1 uses
//  ZInArm_Shuttle1_Place[1][3]; pick / shuttle 2 with Auto Clean uses ZInArm_Shuttle2_Place[1][3] + pad + offset.
//  [3] source ratchets (argv[1] = source root).
//  Use: only through ctest (NB2_W188InArmZSht).
// =============================================================================
#include "aHotPlateSubstrate.h"
#include "cmydef.h"
#include "cprod.h"
#include "canary_support.h"
#include "mykitsuck.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

void SetMyKitSuckItemAmount();
void GetInArmZShtDownPos_9045(int iSht, bool bPlace, bool IncludeZ);   // ainarm9045.cpp (golden 913 :532)

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

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("NB2_W188InArmZSht"))
        return 2;
    std::printf("==== W-188 #10 GetInArmZShtDownPos_9045 -> golden 913 ====\n");

    char msg[200];
    SetMyKitSuckItemAmount();
    const int saveRow = InArmSuck.iMotRow, saveCol = InArmSuck.iMotCol;
    InArmSuck.iMotRow = 2; InArmSuck.iMotCol = 2;
    for (int r = 0; r < MAX_ARM_Row; ++r)
        for (int c = 0; c < MAX_ARM_Col; ++c) {
            Prod.ZInArm_Shuttle1_Place[r][c] = 1000 + r * 10 + c;
            Prod.ZInArm_Shuttle2_Place[r][c] = 2000 + r * 10 + c;
        }
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j) { InArmSuck.Suck[i][j].iMyRow = i; InArmSuck.Suck[i][j].iMyCol = j; }
    bRunAutoClean = false;

    // ---------------------------------------------------------------- [1] identity mapping
    std::printf("-- [1] identity mapping --\n");
    GetInArmZShtDownPos_9045(0, true, false);
    std::snprintf(msg, sizeof msg, "logical == physical: shuttle 1 place heights 1000/1001/1010/1011 (got %d/%d/%d/%d)",
                  iZPosToSht[0][0], iZPosToSht[0][1], iZPosToSht[1][0], iZPosToSht[1][1]);
    CHECK(iZPosToSht[0][0] == 1000 && iZPosToSht[0][1] == 1001 && iZPosToSht[1][0] == 1010 && iZPosToSht[1][1] == 1011, msg);

    // ---------------------------------------------------------------- [2] slot [0][1] remapped to physical [1][3]
    std::printf("-- [2] remapped nozzle --\n");
    InArmSuck.Suck[0][1].iMyRow = 1; InArmSuck.Suck[0][1].iMyCol = 3;
    GetInArmZShtDownPos_9045(0, true, false);
    std::snprintf(msg, sizeof msg, "place / shuttle 1: slot [0][1] uses ZInArm_Shuttle1_Place[1][3] = 1013 (got %d; 0618 gave 1001)", iZPosToSht[0][1]);
    CHECK(iZPosToSht[0][1] == 1013 && iZPosToSht[0][0] == 1000, msg);
    bRunAutoClean = true;
    TestIF_File.iPadThickness = 7;
    TestIF_File.iAutoClean_Shuttle2PickOffset = 100;
    GetInArmZShtDownPos_9045(1, false, false);
    std::snprintf(msg, sizeof msg, "pick / shuttle 2 / Auto Clean: ZInArm_Shuttle2_Place[1][3]+7+100 = 2120 (got %d)", iZPosToSht[0][1]);
    CHECK(iZPosToSht[0][1] == 2120 && iZPosToSht[1][1] == 2011 + 107, msg);
    bRunAutoClean = false;
    InArmSuck.iMotRow = saveRow; InArmSuck.iMotCol = saveCol;

    // ---------------------------------------------------------------- [3] source ratchets
    std::printf("-- [3] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> C = ReadLines(root + "/ainarm9045.cpp");
    CHECK(Count(C, "    bool bNeedDown=false;   int iR, iC;") == 1, "iR / iC declared in code (not after a comment)");
    CHECK(Count(C, "        {   iR=InArmSuck.Suck[i][j].iMyRow; iC=InArmSuck.Suck[i][j].iMyCol;") == 1, "physical row / col read per nozzle");
    CHECK(Count(C, "iZPosToSht[i][j]=Prod.ZInArm_Shuttle1_Place[iR][iC]") == 4 && Count(C, "iZPosToSht[i][j]=Prod.ZInArm_Shuttle2_Place[iR][iC]") == 4,
          "all 8 shuttle heights indexed by the physical slot");

    std::printf("==== W-188 #10: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
