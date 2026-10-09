// =============================================================================
//  test_w190_nnofs.cpp -- AI(W906-W190-NNOFS) 20261009 (Ifor01)
//
//  W-190 ② (TO_IFOR 1009 14:3x; Steven 1009 13:5x "913 新增功能照 913 補進 cpp"): TfTemp_Set::SaveRemoteTempOffset is now
//  golden 913 uTemp_Set.cpp:6119-6277 -- RogerYang 0824 (the [ATC] ATCTempOffset[] index comes from iSiteToOfs, the table
//  SetATCOffset uses), 0918 (the three NN modes keep iIndexTag) and 0922 (a site that is on the other arm is a skip, not a
//  failure; same return value, different log line).  The port had the 0618 body (always iIndexTag).
//  Under ctest only (st02_test_containment.h): every file is under %TEMP%\ht9045_w190_nnofs_<tick> (DataPath / LastDataPath
//  pointed there, abort before any call otherwise).  Harness as tests/test_w9_remote_temp_offset.cpp (St02 W9).
//    a. ordinary mode, arm 1 site 3 (r0 c2): written to ATCTempOffset[iSiteToOfs[0][0][2]], not to [iIndexTag]
//    b. arm 2 uses iSiteToOfs[1][..]
//    c. the real table: after fTemp_Set->InitialAddrToATC() the index is what it filled in
//    d. QualSite2X2N / _6Site2X3N / _8Site2X4N keep iIndexTag (0918)
//    e. Tri_Temp_Machine==1 keeps iIndexTag; f. a table value outside 0..31 keeps iIndexTag; g. a site not in the site map
//       keeps iIndexTag
//    h. RefreshTempData -1 (site 9): -1 and no write -- also when site 9 is in the site map (0922's skip path)
//  Every case first resets ATCTempOffset[0..31] to 0 and then checks that exactly one key changed.
// =============================================================================
#include "forms/fTemp_Set.h"
#include "forms/fMain.h"
#include "MyTempPanel.h"
#include "common.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "TempCtrl/TriTemp.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "st02_test_containment.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string g_root, g_file;

// arm 1 site k -> tcAa1+k-1, arm 2 site k -> tcAa2+k-1 (8 sites per arm, the Index heaters); site 9 -> -1
class NnMain : public TfMain
{
public:
    int RefreshTempData(bool /*bTransfer*/, int iArm, int iSite) override
    {
        if (iSite < 1 || iSite > 8) return -1;
        return (iArm == 2 ? tcAa2 : tcAa1) + iSite - 1;
    }
};

static std::string Key(const char* sec, const char* key)
{
    char buf[64];
    ::GetPrivateProfileStringA(sec, key, "<none>", buf, sizeof(buf), g_file.c_str());
    return buf;
}
static void Seed(const char* sec, const char* key, const char* v) { ::WritePrivateProfileStringA(sec, key, v, g_file.c_str()); }
static void ResetAtc() { char k[32]; for (int i = 0; i < 32; ++i) { std::snprintf(k, sizeof(k), "ATCTempOffset[%d]", i); Seed("ATC", k, "0"); } }
// index whose key is not "0" any more (-1 = none, -2 = more than one)
static int ChangedAtc()
{
    int hit = -1; char k[32];
    for (int i = 0; i < 32; ++i) {
        std::snprintf(k, sizeof(k), "ATCTempOffset[%d]", i);
        if (Key("ATC", k) != "0") { if (hit != -1) return -2; hit = i; }
    }
    return hit;
}
static std::string AtcKey(int i) { char k[32]; std::snprintf(k, sizeof(k), "ATCTempOffset[%d]", i); return Key("ATC", k); }
static void ClearSiteMap() { for (int r = 0; r < MAX_SOCKET_ROW; ++r) for (int c = 0; c < MAX_SOCKET_COL; ++c) TestIF_File.iSiteMap[r][c] = 0; }
static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s.compare(0, 9, "d:\\ht9045") == 0;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    printf("W190_NnRemoteOffset\n");
    if (!W906TestInsideCtestRoots("W190_NnRemoteOffset")) return 2;   // RecordProcess writes the cMyDB log roots: only under ctest
    char tmp[MAX_PATH]; ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32]; std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w190_nnofs_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    { FILE* f = std::fopen((g_root + "\\SetUp.inf").c_str(), "wb"); if (f) { std::fputs("NNOFS\r\n", f); std::fclose(f); } }
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    g_file = g_root + "\\Data\\NNOFS\\Temperature.Data";
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(std::string(LastDataPath.c_str()))) {
        printf("  ABORT: DataPath / LastDataPath not in the sandbox -- nothing was called\n");
        return 2;
    }

    NnMain nm;
    TfMain* const oldMain = fMain; fMain = &nm;
    TfTemp_Set* const oldTempSet = fTemp_Set;
    fTemp_Set = new TfTemp_Set();
    fTemp_Set->Init();
    ::CreateDirectoryA((g_root + "\\Data\\NNOFS").c_str(), 0);

    InputLimit.iTempLow = -60; InputLimit.iTempHigh = 60;
    CosFunction.bUseOldATCTempOffset = false;
    IniConfig.iN31_UseAutoTempOfsByFTP = 0;
    IniConfig.bSPILFunction = false;
    CUSTOMER_CODE = 0;
    ATC_SYSTEM = eNewATCSystem;
    Tri_Temp_Machine = 0;
    TestIF_File.iTestMode = 0;
    const int tag1 = fTemp_Set->myTempPal[tcAa1 + 2] ? fTemp_Set->myTempPal[tcAa1 + 2]->iIndexTag : -1;   // arm 1 site 3
    const int tag2 = fTemp_Set->myTempPal[tcAa2 + 2] ? fTemp_Set->myTempPal[tcAa2 + 2]->iIndexTag : -1;   // arm 2 site 3
    CHECK(tag1 >= 0 && tag1 < 32 && tag2 >= 0 && tag2 < 32, "0. myTempPal[Ac1 / Ac2]->iIndexTag are valid ATC slots");
    int pick1 = 21, pick2 = 29;
    if (pick1 == tag1) pick1 = 22;
    if (pick2 == tag2) pick2 = 30;
    char msg[160];

    // a. ordinary mode, arm 1: the table decides
    ClearSiteMap(); TestIF_File.iSiteMap[0][2] = 3;
    fTemp_Set->iSiteToOfs[0][0][2] = pick1; fTemp_Set->iSiteToOfs[1][0][2] = pick2;
    ResetAtc();
    std::snprintf(msg, sizeof(msg), "a. arm 1 site 3 -> ATCTempOffset[%d] (iSiteToOfs) = 1.5000, nothing else (iIndexTag %d)", pick1, tag1);
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == pick1 && AtcKey(pick1) == "1.5000", msg);

    // b. arm 2
    ResetAtc();
    std::snprintf(msg, sizeof(msg), "b. arm 2 site 3 -> ATCTempOffset[%d] (iSiteToOfs[1])", pick2);
    CHECK(fTemp_Set->SaveRemoteTempOffset(2, 3, 1.0) == 0 && ChangedAtc() == pick2 && AtcKey(pick2) == "1.0000", msg);

    // c. the real table (InitialAddrToATC, run by ChangeSite at boot since N1-G5)
    fTemp_Set->InitialAddrToATC();
    const int real1 = fTemp_Set->iSiteToOfs[0][0][2];
    ResetAtc();
    const int rc = fTemp_Set->SaveRemoteTempOffset(1, 3, 0.5);
    const int expect = (real1 >= 0 && real1 < 32) ? real1 : tag1;
    std::snprintf(msg, sizeof(msg), "c. after InitialAddrToATC: iSiteToOfs[0][0][2]=%d -> ATCTempOffset[%d]", real1, expect);
    CHECK(rc == 0 && ChangedAtc() == expect && AtcKey(expect) == "0.5000", msg);
    fTemp_Set->iSiteToOfs[0][0][2] = pick1;

    // d. the three NN modes keep iIndexTag (0918)
    const int nn[3] = { QualSite2X2N, _6Site2X3N, _8Site2X4N };
    const char* const nnName[3] = { "QualSite2X2N", "_6Site2X3N", "_8Site2X4N" };
    for (int i = 0; i < 3; ++i) {
        TestIF_File.iTestMode = nn[i];
        ResetAtc();
        std::snprintf(msg, sizeof(msg), "d. %s keeps ATCTempOffset[iIndexTag %d]", nnName[i], tag1);
        CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == tag1, msg);
    }
    TestIF_File.iTestMode = 0;

    // e. tri-temp machine keeps iIndexTag
    Tri_Temp_Machine = 1; ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == tag1, "e. Tri_Temp_Machine==1 keeps iIndexTag");
    Tri_Temp_Machine = 0;

    // f. a table value outside 0..31 keeps iIndexTag
    fTemp_Set->iSiteToOfs[0][0][2] = 32; ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == tag1, "f. iSiteToOfs 32 (invalid) keeps iIndexTag");
    fTemp_Set->iSiteToOfs[0][0][2] = -1; ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == tag1, "f. iSiteToOfs -1 (invalid) keeps iIndexTag");
    fTemp_Set->iSiteToOfs[0][0][2] = pick1;

    // g. a site that is not in the site map keeps iIndexTag
    ClearSiteMap(); ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 3, 1.5) == 0 && ChangedAtc() == tag1, "g. site not in the site map keeps iIndexTag");

    // h. no channel for the site: -1, nothing written (with and without the site in the site map -- 0922 only changes the log)
    ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 9, 1.5) == -1 && ChangedAtc() == -1, "h. site 9 (no channel), not in the map: -1, no write");
    TestIF_File.iSiteMap[1][0] = 9; ResetAtc();
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 9, 1.5) == -1 && ChangedAtc() == -1, "h. site 9 in the site map (other arm, 0922 skip): -1, no write");
    ClearSiteMap();

    delete fTemp_Set;
    fTemp_Set = oldTempSet;
    fMain = oldMain;
    printf("W190_NnRemoteOffset: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root); else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
