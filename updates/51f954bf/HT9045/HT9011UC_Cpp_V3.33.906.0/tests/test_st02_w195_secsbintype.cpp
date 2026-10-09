// =============================================================================
//  test_st02_w195_secsbintype.cpp -- W-195 MR-B: TfBinSel::ReadFile builds the SECS "Bin 0-255 Type" lists (sBinType[tag],
//                                    EC 3656 / 3719 / 3724 / 3803 / 3903 / 4003 / 4103) from each tag's recipe (golden 913).
//
//  AI(W906-W195) 20261009 (St02-E).  Suite name (add_test): St02_W195SecsBinType.  argv[1] = the tree root (read only).
//    golden 913 cBinSel.cpp:1585-1596 (AI(ht9045-secs-sem) 20260720 RogerYang; V912 identical): bin i is "1" (Fail) when the
//    T3 tray it goes to (BinSelect[tag].iCatDataT3Pos[i]-1) is a fail category (iStackDefFailCate>0), else "0" -- per tag,
//    instead of the run mode's Prod.bIsPassBin[i] (one list for every tag; all "1" before InitialOK).
//    The recipe is seeded where ReadFile reads it (seed at the real source): a fresh <recipe>\Binasgn.Data (FT) and
//    BinasgnOff.Data (RT) in the old format ReadFunctionData takes when no "Bin Func" section exists -- [Category0] Bin /
//    [Category1] Bin = the T3 positions of Auto1 / Auto2 (cBinSel.cpp:2455-2459), and [Auto1] / [Auto2] Pass/Fail
//    (cBinSel.cpp:1679-1683) -- pass / fail in the FT file, the other way round in the RT file.  Prod.bIsPassBin is set the
//    opposite way to the FT expectation, so the old lines give a different answer.
//    [1] the seeds landed (BinSelect[eBinFT] / [eBinRT] tables printed);
//    [2] sBinType[eBinFT] = 0,1 and sBinType[eBinRT] = 1,0 (each tag from its own recipe);
//    [3] a bin with no tray (Bin = NoUse) is "0" whatever Prod.bIsPassBin says;
//    [4] source: EC 3656 / 3719 still register fBinSel->sBinType[eBinFT] / [eBinRT] (the lists checked above).
//  Reverse check (recorded in the MR): put the two old Prod.bIsPassBin lines back -> [2] and [3] red.
//  Every path is under %TEMP%\ht9045_w195b_<tick>; the test aborts before ReadFile if one is under D:\HT9045.  The sandbox is
//  removed on a green run.
// =============================================================================
#include "database.h"
#include "csystem.h"
#include "cprod.h"
#include "common.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "forms/fBinSel.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261009 (laptop, batch 136 integration): fTemp_Set before ReadFile reaches ChangeSite (see main)

static int g_pass = 0, g_fail = 0;
static char g_got[700];
#define GOT(...) std::snprintf(g_got, sizeof(g_got), __VA_ARGS__)
#define CHECK(cond, msg)                                                                                   \
    do {                                                                                                   \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got); ++g_pass; }                                 \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got, __LINE__); ++g_fail; }        \
        g_got[0] = 0;                                                                                      \
    } while (0)

static std::string g_root;

static std::string Slurp(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string();
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static void WriteText(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;   // a build dir under D:\HT9045 is a sandbox (as St02_SecsEcFileWrites)
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

static void Ini(const std::string& file, const char* sec, const char* key, int v)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", v);
    ::WritePrivateProfileStringA(sec, key, buf, file.c_str());
}

static std::string Item(TStringList* l, int i)
{
    return (l != 0 && i < l->Count) ? std::string(l->Strings[i].c_str()) : std::string("<none>");
}

static std::string LineWith(const std::string& text, const std::string& anchor)
{
    const size_t at = text.find(anchor);
    if (at == std::string::npos) return std::string();
    const size_t a = text.rfind('\n', at), b = text.find('\n', at);
    return text.substr(a == std::string::npos ? 0 : a + 1, (b == std::string::npos ? text.size() : b) - (a == std::string::npos ? 0 : a + 1));
}

int main(int argc, char** argv)
{
    printf("St02_W195SecsBinType\n");
    if (argc < 2) { printf("  ABORT: argv[1] (the tree root) missing\n"); return 2; }
    const std::string src = std::string(argv[1]) + "/";

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w195b_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data\\W195B").c_str(), 0);
    WriteText(g_root + "\\SetUp.inf", "W195B\r\n");
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    asTeachPath  = AnsiString((g_root + "\\teach.ini").c_str());   // ReadFile -> SetWorkParameter -> ReadTechData (as St02_SecsEcFileWrites)
    const std::string recipe = std::string(GetRecipePath().c_str());
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(recipe) || UnderMachineTree(std::string(asTeachPath.c_str())) ||
        UnderMachineTree(std::string(asGeneralPath.c_str())) || recipe.compare(0, g_root.size(), g_root) != 0)
    {
        printf("  ABORT: a path is not in the sandbox (DataPath=%s recipe=%s teach=%s general=%s) -- ReadFile was not called\n",
               DataPath.c_str(), recipe.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
        return 2;
    }
    GOT("recipe=%s teach=%s general=%s", recipe.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
    CHECK(true, "0. recipe dir, teach.ini and the general ini are all outside the machine tree");
    OpenGeneralIniFile();
    GOT("fBinSel=%p iTestBinCount=%d", (void*)fBinSel, iTestBinCount);
    CHECK(fBinSel != 0 && iTestBinCount >= 3, "0. the fBinSel object exists and has at least three bins");
    if (fBinSel == 0 || iTestBinCount < 3) { printf("St02_W195SecsBinType: %d passed, %d failed\n", g_pass, g_fail); return 1; }

    // ReadFile's file per tag (cBinSel.cpp:1424-1439): with these three flags off, RT (tag 0) reads BinasgnOff.Data and FT
    // (tag 1) reads Binasgn.Data -- the leading "\" some of those names carry lands on the same file (W-195 MR-A).
    IniConfig.bA02BinModelPrime = false;
    IniConfig.bFTBin2RTBin = false;
    CosFunction.bDisableRTBinSet = false;
    const std::string ft = recipe + "Binasgn.Data", rt = recipe + "BinasgnOff.Data";
    const int t1 = iTo3Unload[eAuto1], t2 = iTo3Unload[eAuto2];
    const char* n1 = s6TrayName[eAuto1].c_str();
    const char* n2 = s6TrayName[eAuto2].c_str();
    Ini(ft, "Category0", "Bin", t1 + 1); Ini(ft, "Category1", "Bin", t2 + 1); Ini(ft, "Category2", "Bin", e3PosNoUse);
    Ini(ft, n1, "Pass/Fail", 0);         Ini(ft, n2, "Pass/Fail", 1);
    Ini(rt, "Category0", "Bin", t1 + 1); Ini(rt, "Category1", "Bin", t2 + 1); Ini(rt, "Category2", "Bin", e3PosNoUse);
    Ini(rt, n1, "Pass/Fail", 1);         Ini(rt, n2, "Pass/Fail", 0);
    // the run mode's pass flags, opposite to the FT recipe: the old lines (Prod.bIsPassBin) would give 1,0,1
    Prod.bIsPassBin[0] = false; Prod.bIsPassBin[1] = true; Prod.bIsPassBin[2] = false;

    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261009 (laptop, batch 136 integration): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite; since !369 ChangeSite calls fTemp_Set->InitialAddrToATC() and this ReadFile goes SetWorkParameter -> ChangeSite -- the same line !369 added to St02_SecsEcFileWrites and the other ChangeSite tests
    fBinSel->ReadFile(false, false, "");
    CloseIniFile();

    // ---- [1] the seeds landed ---------------------------------------------------------------------------------------------
    {
        const SYSTEM_BIN_SELECT& F = BinSelect[eBinFT];
        const SYSTEM_BIN_SELECT& R = BinSelect[eBinRT];
        GOT("Auto1/Auto2 T3 %d/%d (%s/%s); FT pos %d,%d,%d fail(t1,t2)=%d,%d; RT pos %d,%d,%d fail(t1,t2)=%d,%d; IfErrorT3 FT %d RT %d",
            t1, t2, n1, n2, F.iCatDataT3Pos[0], F.iCatDataT3Pos[1], F.iCatDataT3Pos[2], F.iStackDefFailCate[t1], F.iStackDefFailCate[t2],
            R.iCatDataT3Pos[0], R.iCatDataT3Pos[1], R.iCatDataT3Pos[2], R.iStackDefFailCate[t1], R.iStackDefFailCate[t2], F.IfErrorT3, R.IfErrorT3);
        const bool ok = t1 != t2 && F.iCatDataT3Pos[0] == t1 + 1 && F.iCatDataT3Pos[1] == t2 + 1 && F.iCatDataT3Pos[2] == e3PosNoUse &&
                        F.iStackDefFailCate[t1] == 0 && F.iStackDefFailCate[t2] > 0 &&
                        R.iCatDataT3Pos[0] == t1 + 1 && R.iCatDataT3Pos[1] == t2 + 1 &&
                        R.iStackDefFailCate[t1] > 0 && R.iStackDefFailCate[t2] == 0;
        CHECK(ok, "1. ReadFile read the seeded recipes: bins 0/1 -> Auto1/Auto2; FT Auto1 pass, Auto2 fail; RT the other way round");
    }

    // ---- [2] / [3] sBinType per tag ------------------------------------------------------------------------------------
    {
        const std::string f0 = Item(fBinSel->sBinType[eBinFT], 0), f1 = Item(fBinSel->sBinType[eBinFT], 1), f2 = Item(fBinSel->sBinType[eBinFT], 2);
        const std::string r0 = Item(fBinSel->sBinType[eBinRT], 0), r1 = Item(fBinSel->sBinType[eBinRT], 1), r2 = Item(fBinSel->sBinType[eBinRT], 2);
        GOT("FT %s,%s,%s  RT %s,%s,%s  (Prod.bIsPassBin 0,1,0 would give 1,0,1 for both)", f0.c_str(), f1.c_str(), f2.c_str(), r0.c_str(), r1.c_str(), r2.c_str());
        CHECK(f0 == "0" && f1 == "1" && r0 == "1" && r1 == "0",
              "2. sBinType[eBinFT] = 0,1 and sBinType[eBinRT] = 1,0: each tag from its own recipe's pass / fail trays (golden 913 :1585-1596)");
        GOT("bin 2 (no tray): FT %s RT %s", f2.c_str(), r2.c_str());
        CHECK(f2 == "0" && r2 == "0", "3. a bin with no tray (Bin = NoUse) is 0 (Pass) whatever Prod.bIsPassBin says (golden :1589 iT3ForSecsType>=0)");
    }

    // ---- [4] source: the ECs read these lists ----------------------------------------------------------------------------
    {
        const std::string ec = Slurp(src + "SECSGEM/uHGemHT9045_EC.cpp");
        const std::string l3656 = LineWith(ec, "SetECDataPointer(3656 ,"), l3719 = LineWith(ec, "SetECDataPointer(3719 ,");
        GOT("3656 %s; 3719 %s", l3656.empty() ? "<not found>" : "found", l3719.empty() ? "<not found>" : "found");
        CHECK(l3656.find("fBinSel->sBinType[eBinFT]") != std::string::npos && l3719.find("fBinSel->sBinType[eBinRT]") != std::string::npos,
              "4. SECSGEM/uHGemHT9045_EC.cpp: EC 3656 / 3719 point at fBinSel->sBinType[eBinFT] / [eBinRT] (the lists checked in [2])");
    }

    printf("St02_W195SecsBinType: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
