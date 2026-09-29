// =============================================================================
//  test_w9_remote_temp_offset.cpp -- Steven W9 = A: a remote temperature offset (GPIB SETTESTOFFSET_ / DEVICETEMP, SECS
//  TEMP_OFFSET) is written into the recipe's Temperature.Data as golden does (SAFETY GATE S3 opened), plus the 912
//  supplement: the GPIB path reloads the offsets after a successful write, as SECS and FTP already do.
//
//  AI(W906-W9) 20260927 (St02).  Suite name (add_test): TesterComm_W9RemoteTempOffset
//
//  Covers uTemp_Set.cpp TfTemp_Set::SaveRemoteTempOffset (golden 906_0625_Steven uTemp_Set.cpp:5856-5973),
//  SaveRemoteTempOffsetFromGPIB (:5975-6059 + 912 uTemp_Set.cpp:6142-6163) and ReadRemoteTempOffset (:6061-6115):
//    a. non-ATC: [User OffSet] CH<head+1> written as "%0.4f", and it ACCUMULATES (now + offset);
//    b. the bounds are strict (59 + 1 with a 60 limit is refused, the file is unchanged); head -1 -> -1, file unchanged;
//    c. eNewATCSystem: [ATC] ATCTempOffset[<myTempPal[head]->iIndexTag>]; the N31 limits (given swapped) are used;
//       bUseOldATCTempOffset -> -1; eATC30 -> -1 and no write;
//    d. the 912 reload: FromGPIB("SETTESTOFFSET_1.5") updates the file AND Temperature.fTempOffSet (a sentinel 99 becomes
//       1.5; another channel's sentinel is reloaded to its file value 0).  golden 906 would leave 99;
//    e. SETTESTOFFSET_0 and SETTESTOFFSET_70 write nothing and do NOT reload (the sentinel stays);
//    f. iArm 0 (IndexStatus neither Z1Down_Z2Up nor Z1Up_Z2Down) writes both arms.
//  Every file is under %TEMP%\ht9045_w9_<tick>: the test points DataPath / LastDataPath there itself and aborts before any
//  call if DataPath is still under D:\HT9045.  It also checks that the machine's own Temperature.Data (the recipe named in
//  D:\HT9045\SetUp.inf, read-only) keeps its size and write time.  CloseIniFile() is never called (common.cpp keeps a
//  freed INIFile pointer, golden's faithful bug).  The sandbox is removed on a green run.
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
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string g_root, g_file;

// arm 1 site k -> tcAa1+k-1, arm 2 site k -> tcAa2+k-1 (8 sites per arm, the Index heaters); site 9 -> -1
class W9Main : public TfMain
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

static std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

static bool Stat(const std::string& p, WIN32_FILE_ATTRIBUTE_DATA* d)
{
    return ::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, d) != 0;
}

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

// the machine's own recipe file (read-only): D:\HT9045\IniData\Data\<SetUp.inf line 1>\Temperature.Data
static std::string MachineTemperatureData()
{
    std::string inf = Bytes("D:\\HT9045\\SetUp.inf");
    size_t e = inf.find_first_of("\r\n");
    if (e != std::string::npos) inf.erase(e);
    if (inf.empty()) return "";
    return "D:\\HT9045\\IniData\\Data\\" + inf + "\\Temperature.Data";
}

int main()
{
    printf("TesterComm_W9RemoteTempOffset\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w9_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    {
        FILE* f = std::fopen((g_root + "\\SetUp.inf").c_str(), "wb");
        if (f) { std::fputs("W9R\r\n", f); std::fclose(f); }
    }
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    g_file = g_root + "\\Data\\W9R\\Temperature.Data";               // golden: DataPath + GetLastOpenFN() + "\\Temperature.Data"
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(std::string(LastDataPath.c_str())))
    {
        printf("  ABORT: DataPath / LastDataPath not in the sandbox (%s, %s) -- nothing was called\n", DataPath.c_str(), LastDataPath.c_str());
        return 2;
    }
    const std::string machineFile = MachineTemperatureData();
    WIN32_FILE_ATTRIBUTE_DATA before; std::memset(&before, 0, sizeof(before));
    const bool machineFileThere = !machineFile.empty() && Stat(machineFile, &before);

    W9Main w9;
    TfMain* const oldMain = fMain;
    fMain = &w9;
    TfTemp_Set* const oldTempSet = fTemp_Set;
    fTemp_Set = new TfTemp_Set();
    fTemp_Set->Init();                                              // the wb_serve boot does the same (tools/wb_serve.cpp:3111/:3120)

    InputLimit.iTempLow = -60;
    InputLimit.iTempHigh = 60;
    CosFunction.bUseOldATCTempOffset = false;
    IniConfig.iN31_UseAutoTempOfsByFTP = 0;
    IniConfig.bSPILFunction = false;
    CUSTOMER_CODE = 0;
    iContactMode = 0;
    const int chA1 = tcAa1 + 1, chA2 = tcAa2 + 1;                  // [User OffSet] CH<head+1>
    char kA1[16], kA2[16], kB1[16];
    std::snprintf(kA1, sizeof(kA1), "CH%d", chA1);
    std::snprintf(kA2, sizeof(kA2), "CH%d", chA2);
    std::snprintf(kB1, sizeof(kB1), "CH%d", tcAb1 + 1);

    // a. non-ATC, written as %0.4f, accumulating
    ATC_SYSTEM = eATCUninstall;
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 1.5) == 0, "a. arm 1 site 1 +1.5 accepted");
    CHECK(Key("User OffSet", kA1) == "1.5000", "a. [User OffSet] CH<Aa1+1> = 1.5000 (golden WriteIniData %0.4f)");
    Seed("User OffSet", kA1, "2");
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 1.5) == 0 && Key("User OffSet", kA1) == "3.5000", "a. it accumulates: 2 + 1.5 = 3.5000");

    // b. strict bounds, head -1
    Seed("User OffSet", kA1, "59");
    std::string was = Bytes(g_file);
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 1.0) == -1 && Bytes(g_file) == was, "b. 59 + 1 with the 60 limit is refused, file unchanged (strict <)");
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 9, 1.0) == -1 && Bytes(g_file) == was, "b. RefreshTempData -1 -> -1, file unchanged");

    // c. ATC
    const int tag = fTemp_Set->myTempPal[tcAa1] ? fTemp_Set->myTempPal[tcAa1]->iIndexTag : -1;
    CHECK(tag >= 0 && tag < 32, "c. myTempPal[tcAa1]->iIndexTag is a valid ATC slot");
    char kAtc[32];
    std::snprintf(kAtc, sizeof(kAtc), "ATCTempOffset[%d]", tag);
    ATC_SYSTEM = eNewATCSystem;
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 2.0) == 0 && Key("ATC", kAtc) == "2.0000", "c. eNewATCSystem: [ATC] ATCTempOffset[tag] = 2.0000");
    Seed("ATC", kAtc, "0");
    IniConfig.iN31_UseAutoTempOfsByFTP = 1;
    IniConfig.dN31_MinOffset = 5.0;                                // given swapped; golden swaps them (:5898-5902)
    IniConfig.dN31_MaxOffset = -5.0;
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 4.0) == 0 && Key("ATC", kAtc) == "4.0000", "c. N31 limits (-5, 5): 4 accepted");
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 2.0) == -1 && Key("ATC", kAtc) == "4.0000", "c. N31 limits: 4 + 2 = 6 refused");
    IniConfig.iN31_UseAutoTempOfsByFTP = 0;
    CosFunction.bUseOldATCTempOffset = true;
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 1.0) == -1 && Key("ATC", kAtc) == "4.0000", "c. bUseOldATCTempOffset -> -1, no write");
    CosFunction.bUseOldATCTempOffset = false;
    ATC_SYSTEM = eATC30;
    CHECK(fTemp_Set->SaveRemoteTempOffset(1, 1, 1.0) == -1 && Key("ATC", kAtc) == "4.0000", "c. eATC30 -> -1, no write");

    // d. the 912 reload after a successful GPIB write
    ATC_SYSTEM = eATCUninstall;
    Seed("User OffSet", kA1, "0");
    IndexStatus = Z1Down_Z2Up;                                     // -> iArm 1
    Temperature.fTempOffSet[UserOffSet][tcAa1] = 99.0;
    Temperature.fTempOffSet[UserOffSet][tcAb1] = 77.0;             // CH<Ab1+1> is not in the file -> reloads to 0
    fTemp_Set->SaveRemoteTempOffsetFromGPIB("SETTESTOFFSET_1.5");
    CHECK(Key("User OffSet", kA1) == "1.5000", "d. FromGPIB SETTESTOFFSET_1.5 -> file 1.5000");
    CHECK(Temperature.fTempOffSet[UserOffSet][tcAa1] == 1.5, "d. 912 :6142-6163: memory reloaded (sentinel 99 -> 1.5); golden 906 leaves 99");
    CHECK(Temperature.fTempOffSet[UserOffSet][tcAb1] == 0.0, "d. the reload reads every channel (77 -> its file value 0)");

    // e. nothing written -> no reload
    Temperature.fTempOffSet[UserOffSet][tcAa1] = 99.0;
    fTemp_Set->SaveRemoteTempOffsetFromGPIB("SETTESTOFFSET_0");
    CHECK(Key("User OffSet", kA1) == "1.5000" && Temperature.fTempOffSet[UserOffSet][tcAa1] == 99.0, "e. SETTESTOFFSET_0: no write, no reload");
    fTemp_Set->SaveRemoteTempOffsetFromGPIB("SETTESTOFFSET_70");
    CHECK(Key("User OffSet", kA1) == "1.5000" && Temperature.fTempOffSet[UserOffSet][tcAa1] == 99.0, "e. SETTESTOFFSET_70 (out of range): no write, no reload");

    // f. both arms
    Seed("User OffSet", kA1, "0");
    Seed("User OffSet", kA2, "0");
    IndexStatus = 0;
    fTemp_Set->SaveRemoteTempOffsetFromGPIB("SETTESTOFFSET_1");
    CHECK(Key("User OffSet", kA1) == "1.0000" && Key("User OffSet", kA2) == "1.0000", "f. iArm 0 -> both arms written");

    // the machine's own recipe file is untouched
    if (machineFileThere)
    {
        WIN32_FILE_ATTRIBUTE_DATA after; std::memset(&after, 0, sizeof(after));
        const bool still = Stat(machineFile, &after);
        CHECK(still && after.nFileSizeLow == before.nFileSizeLow &&
              CompareFileTime(&after.ftLastWriteTime, &before.ftLastWriteTime) == 0,
              "0. the machine's Temperature.Data (SetUp.inf's recipe) keeps its size and write time");
    }
    else
        printf("  (no machine SetUp.inf / Temperature.Data on this PC -- the real-file check is skipped)\n");

    delete fTemp_Set;
    fTemp_Set = oldTempSet;
    fMain = oldMain;
    printf("TesterComm_W9RemoteTempOffset: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
