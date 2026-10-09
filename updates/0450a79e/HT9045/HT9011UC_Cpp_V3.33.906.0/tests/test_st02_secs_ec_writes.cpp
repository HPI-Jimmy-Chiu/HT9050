// =============================================================================
//  test_st02_secs_ec_writes.cpp -- POOL-2: the three SECS host-triggered calls in SECSGEM/uHGemHT9045.cpp that were gated
//                                  because their callee had no body ([E1] / [E5] / [L1]) run as golden 0618.
//
//  AI(W906-POOL2-SECS) 20261007 (St02-E).  Suite name (add_test): St02_SecsEcFileWrites
//
//  Real frames: built by SecsWireCodec's own encoder, pushed through THGem's simulated socket and decoded by its real
//  ProcessSocketReceiveData (HSys.MyGem stays NULL, so nothing is dispatched), then the real HT9045Gem handler is called --
//  its ActiveWire is that THGem's WireCodec (uHGemHT9045.cpp:371).  The ECs the handlers check are registered here with the
//  same SetECDataPointer lines SECSGEM/uHGemHT9045_EC.cpp uses (:808 3616, :2190 37800), into HT9045Gem's own SvEcReg
//  (HTGem::CheckECValue / SetECValue read that one).
//    [1] [E1] S2F15 check, ECID 37800 -> ReadESDDataFile() reloads ESD_GENERAL from the ESD ini (golden 0618 :685-688);
//        control: ECID 3616 (outside 37800-37887) -> no reload;
//    [2] [E5] S2F15 update, EC 3616 = 5 -> fBinSel->Save(3616, eBinFT) writes [I/F Error] Bin=5 into the recipe's Binasgn
//        file; EC 3617 / 3677 -> their "Bin Func FT" / Pass/Fail keys are written (golden :1013-1021);
//    [3] [L1] S125F3 with one level -> HCACK 0 -> fSecurity->SetLevelSet() writes levelset.dat; with an empty list ->
//        HCACK 1 -> S9F7 and no write (golden :6210-6213);
//    [4] real files: D:\HT9045\system\levelset.dat and D:\ESD_Program\system\General.ini byte-identical.
//  Every path is under %TEMP%\ht9045_secsec_<tick>: DataPath / LastDataPath, aESDSetDataFileName, asTeachPath and
//  W906_LEVELSET_PATH are set here (asGeneralPath is the per-test scratch copy tests/test_bootstrap.cpp makes), and the test
//  aborts before any handler runs if one of them is under D:\HT9045 or D:\ESD_Program.  Every CHECK prints the values it read.
//  The sandbox is removed on a green run.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"
#include "SECSGEM/uHGemHT9045.h"
#include "SECSGEM/SecsWireCodec.h"
#include "database.h"
#include "csystem.h"
#include "cprod.h"
#include "common.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "forms/fBinSel.h"
#include "forms/fSecurity.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261008 (Ifor01): fTemp_Set (see main)

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

const char* W906_LevelSetPath();   // cSecurity.cpp file end

static int g_pass = 0, g_fail = 0;
static char g_got[600];
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
    if (!f) return std::string("<missing>");
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
    //AI(W906-W132-SANDBOX) 20261007 laptop: a build dir under D:\HT9045 (<tree>\Obj\V906\<build>\tests\..., where tests/test_bootstrap.cpp
    //  keeps this test's scratch general ini) is a sandbox, not the machine's data -- same ABORT as St02_W132SckArtLotRt in gate b83a on
    //  every box whose build dir is under D:\HT9045. The machine's levelset.dat / ESD General.ini are still byte-checked (check 4).
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 14, "d:\\esd_program") == 0;
}

static std::string IniText(const std::string& file, const char* sec, const char* key)
{
    char buf[512];
    ::GetPrivateProfileStringA(sec, key, "<none>", buf, sizeof(buf), file.c_str());
    return buf;
}

// the two files TfBinSel::Save can write (SavePath[0]: Binasgn.Data or BinasgnOff.Data by IniConfig / CosFunction flags,
// cBinSel.cpp:893-910); its ReadFile (EC 3616 only) creates the other one as well, so the checks look at both.
static std::string BinFile(int i)
{
    return std::string(GetRecipePath().c_str()) + (i == 0 ? "\\Binasgn.Data" : "\\BinasgnOff.Data");
}

// AI(W906-W191) 20261009 (St02-E): W-191 MR-5 -- the file name WriteIniData hands the change-log hook (common.cpp:1236) for the
//   Binasgn*.Data writes of fBinSel->Save (golden 913 cBinSel.cpp:6355-6381: no doubled backslash after GetRecipePath()).
static std::vector<std::string> g_w191Files;
static W906_ChangeLogFn_Str g_w191Prev = 0;
static void W191Capture(AnsiString F, AnsiString G, AnsiString N, AnsiString ret, AnsiString V, AnsiString& s1, AnsiString& s2, bool& b)
{
    if (G == "I/F Error")                                   // only fBinSel->Save(3616)'s own keys (cBinSel.cpp:921-922); ReadFile /
        g_w191Files.push_back(F.c_str());                   //   SaveFunctionData write other Binasgn files with their own paths
    if (g_w191Prev) g_w191Prev(F, G, N, ret, V, s1, s2, b);
}
static void DeleteBinFiles()
{
    ::DeleteFileA(BinFile(0).c_str());
    ::DeleteFileA(BinFile(1).c_str());
}

// true when either file has <sec> <key> == want (want "*" = the key exists); fills `seen` with both values
static bool EitherHas(const char* sec, const char* key, const std::string& want, std::string* seen)
{
    bool hit = false;
    *seen = "";
    for (int i = 0; i < 2; ++i)
    {
        const std::string v = IniText(BinFile(i), sec, key);
        *seen += std::string(i == 0 ? "Binasgn.Data=" : " BinasgnOff.Data=") + v;
        if (v != "<none>" && (want == "*" || v == want)) hit = true;
    }
    return hit;
}

// one host message: encode with the codec's own encoder, push it through THGem's simulated socket, decode it there
struct Host
{
    THGem* gem;
    TCustomWinSocket* conn;
    void Push(SecsWireCodec& b)
    {
        conn->SimClearTx();
        conn->SimPushReceive(b.LocalBuffer.data(), static_cast<int>(b.LocalLength_4));
        gem->ProcessSocketReceiveData();
    }
    // stream / function of every frame the equipment sent since the last Push
    std::string Sent() const
    {
        const std::vector<char>& tx = conn->SimTxBuffer();
        std::string out;
        size_t i = 0;
        while (i + 14 <= tx.size())
        {
            const unsigned len = ((unsigned char)tx[i] << 24) | ((unsigned char)tx[i + 1] << 16) | ((unsigned char)tx[i + 2] << 8) | (unsigned char)tx[i + 3];
            char sf[24];
            std::snprintf(sf, sizeof(sf), "S%dF%d ", (unsigned char)tx[i + 6] & 0x7f, (unsigned char)tx[i + 7]);
            out += sf;
            i += 4 + len;
        }
        return out.empty() ? std::string("(none)") : out;
    }
};

static void S2F15(Host& host, unsigned ecid, int value)
{
    SecsWireCodec b;
    b.InitLocalHead(2, 15, 1);
    b.DataItemOut(1, HType.LIST_TYPE, NULL);
    b.DataItemOut(2, HType.LIST_TYPE, NULL);
    b.DataItemOut(1, HType.UINT_4_TYPE, &ecid);
    b.DataItemOut(1, HType.INT_4_TYPE, &value);
    host.Push(b);
}

static void S2F15Ascii(Host& host, unsigned ecid, const char* value)
{
    SecsWireCodec b;
    b.InitLocalHead(2, 15, 1);
    b.DataItemOut(1, HType.LIST_TYPE, NULL);
    b.DataItemOut(2, HType.LIST_TYPE, NULL);
    b.DataItemOut(1, HType.UINT_4_TYPE, &ecid);
    b.DataItemOut(HType.ASCII_TYPE, AnsiString(value));
    host.Push(b);
}

int main()
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261008 (Ifor01): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite, which calls fTemp_Set->InitialAddrToATC() since N1-G5
    printf("St02_SecsEcFileWrites\n");
    const std::string realLevel = "D:\\HT9045\\system\\levelset.dat", realEsd = "D:\\ESD_Program\\system\\General.ini";
    const std::string levelBefore = Slurp(realLevel), esdBefore = Slurp(realEsd);

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_secsec_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data\\SECSR").c_str(), 0);
    WriteText(g_root + "\\SetUp.inf", "SECSR\r\n");
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    const std::string esdIni = g_root + "\\ESD_General.ini";
    WriteText(esdIni, "[Setting]\r\nScanInterval=300\r\n");
    aESDSetDataFileName = AnsiString(esdIni.c_str());
    static char levelEnv[MAX_PATH + 32];
    const std::string levelFile = g_root + "\\levelset.dat";
    std::snprintf(levelEnv, sizeof(levelEnv), "W906_LEVELSET_PATH=%s", levelFile.c_str());
    HT9045_TEST_PUTENV(levelEnv);
    // [E5]'s TfBinSel::Save -> ReadFile -> SetWorkParameter -> ReadTechData reads teach.ini (fTeach->ReadFile) and the general
    // ini (CheckAndReadIniDataGeneral needs INIFileGeneral open, as wb_serve's LoadMachineConfig keeps it, database.cpp:3161):
    // teach.ini goes to the sandbox here; asGeneralPath is this test's own scratch copy (tests/test_bootstrap.cpp).
    asTeachPath = AnsiString((g_root + "\\teach.ini").c_str());

    // containment first: nothing below may reach the machine's files
    const std::string recipe = std::string(GetRecipePath().c_str()), level = W906_LevelSetPath();
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(recipe) || UnderMachineTree(level) ||
        UnderMachineTree(std::string(aESDSetDataFileName.c_str())) || UnderMachineTree(std::string(asTeachPath.c_str())) ||
        UnderMachineTree(std::string(asGeneralPath.c_str())) || recipe.compare(0, g_root.size(), g_root) != 0 ||
        level != levelFile)
    {
        printf("  ABORT: a path is not in the sandbox (DataPath=%s recipe=%s levelset=%s esd=%s teach=%s general=%s) -- nothing was called\n",
               DataPath.c_str(), recipe.c_str(), level.c_str(), aESDSetDataFileName.c_str(), asTeachPath.c_str(), asGeneralPath.c_str());
        return 2;
    }
    GOT("recipe=%s levelset=%s esd=%s teach=%s general=%s", recipe.c_str(), level.c_str(), aESDSetDataFileName.c_str(),
        asTeachPath.c_str(), asGeneralPath.c_str());
    CHECK(true, "0. recipe dir, levelset.dat, the ESD ini, teach.ini and the general ini are all outside the machine tree");
    OpenGeneralIniFile();
    GOT("fBinSel=%p fSecurity=%p", (void*)fBinSel, (void*)fSecurity);
    CHECK(fBinSel != 0 && fSecurity != 0, "0. the fBinSel / fSecurity objects exist (cBinSel.cpp:159, cSecurity.cpp:46)");

    THGem gem;
    TCustomWinSocket* conn = gem.srvGem->SimAcceptConnection("10.0.9.1", 5000);
    gem.srvGem->Open();
    THGem* savedHGem = HGem;
    HGem = &gem;
    HT9045Gem h(AnsiString(""), &gem);
    Host host = { &gem, conn };
    // the EC lines SECSGEM/uHGemHT9045_EC.cpp registers (:2190 / :808), into the registry HTGem::CheckECValue reads
    h.SvEcReg.SetECDataPointer(37800, HType.INT_4_TYPE, "ScanInterval", "", &ESD_GENERAL.ScanInterval, "2000", "100", "1000", "");
    h.SvEcReg.SetECDataPointer(3616, HType.INT_4_TYPE, "Error Bin Tray Select", "", &BinSelect[eBinFT].IfErrorT3, "33", "0", "0", "");

    // [1] E1
    ESD_GENERAL.ScanInterval = 555;
    S2F15(host, 37800, 250);
    int r = h.S2F15_CheckNewEquipmentConstant();
    GOT("check=%d ScanInterval=%d (ini 300) tokens-left=%d", r, ESD_GENERAL.ScanInterval, gem.WireCodec.SReceiveData ? gem.WireCodec.SReceiveData->Count : -1);
    CHECK(r == 0 && ESD_GENERAL.ScanInterval == 300, "1. [E1] S2F15 check, ECID 37800 -> ReadESDDataFile reloads ESD_GENERAL from the ini (golden :685-688)");
    ESD_GENERAL.ScanInterval = 555;
    S2F15(host, 3616, 5);
    r = h.S2F15_CheckNewEquipmentConstant();
    GOT("check=%d ScanInterval=%d", r, ESD_GENERAL.ScanInterval);
    CHECK(r == 0 && ESD_GENERAL.ScanInterval == 555, "1. control: ECID 3616 is outside 37800-37887 -> no reload");

    // [2] E5 -- both Binasgn files deleted before each EC, so a key found afterwards was written by that EC's Save
    std::string seenBin, seenTray, seen;
    BinSelect[eBinFT].IfErrorT3 = 1;
    DeleteBinFiles();
    g_w191Files.clear(); g_w191Prev = W906_ChangeLogHook_Str; W906_ChangeLogHook_Str = W191Capture;   // AI(W906-W191) MR-5
    S2F15(host, 3616, 5);
    r = h.S2F15_UpdateNewEquipmentConstant();
    const bool bin5 = EitherHas("I/F Error", "Bin", "5", &seenBin);
    const bool tray5 = EitherHas("I/F Error", "Tray", std::string(s3TrayName[5].c_str()), &seenTray);
    GOT("update=%d IfErrorT3 after ReadFile=%d; Bin: %s; Tray: %s", r, BinSelect[eBinFT].IfErrorT3, seenBin.c_str(), seenTray.c_str());
    CHECK(bin5 && tray5, "2. [E5] EC 3616 = 5 -> fBinSel->Save(3616, eBinFT) writes [I/F Error] Bin=5 / Tray=s3TrayName[5] (golden :1013-1015)");
    W906_ChangeLogHook_Str = g_w191Prev;
    {
        int binasgn = 0, doubled = 0;
        std::string last;
        for (size_t i = 0; i < g_w191Files.size(); ++i)
        {
            const std::string& f = g_w191Files[i];
            const size_t at = f.find("Binasgn");
            if (at == std::string::npos) continue;
            ++binasgn; last = f;
            if (f.find("\\\\") != std::string::npos || at == 0 || f[at - 1] != '\\' || (at >= 2 && f[at - 2] == '\\')) ++doubled;
        }
        GOT("W-191 MR-5: %d Binasgn writes seen by the change-log hook, %d with a doubled backslash; last %s", binasgn, doubled, last.c_str());
        CHECK(binasgn > 0 && doubled == 0, "2. W-191 MR-5 (golden 913 cBinSel.cpp:6355-6381): Save writes <recipe>\\Binasgn*.Data -- one backslash, not GetRecipePath()'s plus another");
    }
    DeleteBinFiles();
    S2F15Ascii(host, 3617, "0,1");
    r = h.S2F15_UpdateNewEquipmentConstant();
    const bool k3617 = EitherHas("Bin Func FT", "3617 BinTraySetting", "*", &seen);
    GOT("update=%d 3617 BinTraySetting: %s", r, seen.c_str());
    CHECK(k3617, "2. [E5] EC 3617 -> Save(3617): [Bin Func FT] 3617 BinTraySetting written (golden :1016-1018)");
    DeleteBinFiles();
    S2F15Ascii(host, 3677, "0");
    r = h.S2F15_UpdateNewEquipmentConstant();
    const bool k3677 = EitherHas(s3TrayName[0].c_str(), "Pass/Fail", "*", &seen);
    GOT("update=%d [%s] Pass/Fail: %s", r, s3TrayName[0].c_str(), seen.c_str());
    CHECK(k3677, "2. [E5] EC 3677 -> Save(3677): per-tray Pass/Fail written (golden :1019-1021)");

    // [3] L1
    ::DeleteFileA(levelFile.c_str());
    {
        SecsWireCodec b;
        b.InitLocalHead(125, 3, 1);
        unsigned lsid = 1, lev = 2;
        b.DataItemOut(1, HType.LIST_TYPE, NULL);
        b.DataItemOut(2, HType.LIST_TYPE, NULL);
        b.DataItemOut(1, HType.UINT_4_TYPE, &lsid);
        b.DataItemOut(1, HType.UINT_4_TYPE, &lev);
        host.Push(b);
    }
    h.S125F4_LevelSettingChangeAcknowledge();
    std::string lv = Slurp(levelFile);
    GOT("sent=%s levelset.dat %s (%u bytes, LevelSet %u)", host.Sent().c_str(), lv == "<missing>" ? "missing" : "written",
        (unsigned)(lv == "<missing>" ? 0 : lv.size()), (unsigned)sizeof(LevelSet));
    CHECK(lv != "<missing>" && lv.size() == sizeof(LevelSet), "3. [L1] S125F3 with one level -> HCACK 0 -> fSecurity->SetLevelSet() writes levelset.dat (golden :6213)");
    ::DeleteFileA(levelFile.c_str());
    {
        SecsWireCodec b;
        b.InitLocalHead(125, 3, 1);
        b.DataItemOut(0, HType.LIST_TYPE, NULL);
        host.Push(b);
    }
    h.S125F4_LevelSettingChangeAcknowledge();
    const std::string sent = host.Sent();
    lv = Slurp(levelFile);
    GOT("sent=%s levelset.dat %s", sent.c_str(), lv == "<missing>" ? "missing" : "written");
    CHECK(sent.find("S9F7") != std::string::npos && lv == "<missing>", "3. [L1] S125F3 with an empty list -> HCACK 1 -> S9F7, no levelset.dat (golden :6210-6211)");

    HGem = savedHGem;

    // [4] real files
    const bool sameLevel = Slurp(realLevel) == levelBefore, sameEsd = Slurp(realEsd) == esdBefore;
    GOT("levelset.dat %s, ESD General.ini %s", sameLevel ? "same" : "CHANGED", sameEsd ? "same" : "CHANGED");
    CHECK(sameLevel && sameEsd, "4. the machine's levelset.dat and D:\\ESD_Program\\system\\General.ini are unchanged");

    printf("St02_SecsEcFileWrites: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
