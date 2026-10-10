// =============================================================================
//  test_l07_persist.cpp -- AI(W906-L07) 20261010 (Ifor01)
//
//  L07 step 2 (W-190 5; TO_IFOR 1010 11:2x B: data model + persistence + counters): golden 913 JerryYang 20260917/18 persistence of the
//  ATC pre-compensation rows in the hand port uTemp_Set.cpp, on a %TEMP% sandbox (the shape of tests/test_settemp_save.cpp):
//    [1] ReadTempFile(true) reads the six sections "Pre OffSet" .. "2nd After Offset" (CH<n>) into fTempOffSet[17..22] (golden 913
//        :2251-2262) and [ATC] bATCPreOffset only with ATC active cooling (:2681-2686)
//    [2] DoIniDataToForm fills the six boxes of every channel (:3520-3530, JerryYang 20260918) and the two check boxes (:3368-3371)
//    [3] spbSaveClick (DefineTemp file): an ATC channel with pre-offset on writes the six Pre keys and NOT its Low / Mid / High / SHigh /
//        AmbientHotLow / Mid keys -- golden 913 :4634-4658 kept as is (GOLDEN BUG: ReadTempFile never reads the Pre keys back from
//        DefineTemp); a non-ATC channel writes the plain keys; SaveSetupFile (recipe) writes [ATC] bATCPreOffset and the six keys in
//        addition to the plain ones (:4985, :5130-5145)
//    [4] the machine's files are untouched
// =============================================================================
#include "forms/fTemp_Set.h"
#include "forms/fMain.h"
#include "MyTempPanel.h"
#include "common.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "MachineType.h"
#include "st02_test_containment.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static void Spit(const std::string& p, const std::string& s) { FILE* f = std::fopen(p.c_str(), "wb"); if (f) { std::fwrite(s.data(), 1, s.size(), f); std::fclose(f); } }
static bool UnderMachineTree(const std::string& p) { std::string l = W906TestSafeLower(p.c_str()); return l.find("d:\\ht9045\\") == 0 || l.find("d:/ht9045/") == 0; }
// [section] key=value, case-insensitive; "<none>" when missing
static std::string IniValue(const std::string& text, const char* section, const char* key)
{
    std::string sec;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t e = text.find('\n', pos);
        if (e == std::string::npos) e = text.size();
        std::string line = text.substr(pos, e - pos);
        pos = e + 1;
        while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == ' ')) line.erase(line.size() - 1);
        if (line.size() >= 2 && line[0] == '[' && line[line.size() - 1] == ']') { sec = W906TestSafeLower(line.substr(1, line.size() - 2).c_str()); continue; }
        const size_t eq = line.find('=');
        if (eq == std::string::npos || sec != W906TestSafeLower(section)) continue;
        std::string k = line.substr(0, eq);
        while (!k.empty() && k[k.size() - 1] == ' ') k.erase(k.size() - 1);
        if (W906TestSafeLower(k.c_str()) == W906TestSafeLower(key)) return line.substr(eq + 1);
    }
    return std::string("<none>");
}
struct Stamp { bool there; DWORD size; FILETIME t; };
static Stamp StampOf(const std::string& p)
{
    Stamp s; std::memset(&s, 0, sizeof(s));
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &a)) { s.there = true; s.size = a.nFileSizeLow; s.t = a.ftLastWriteTime; }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b)
{ return a.there == b.there && a.size == b.size && a.t.dwLowDateTime == b.t.dwLowDateTime && a.t.dwHighDateTime == b.t.dwHighDateTime; }

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("L07_AtcPreOffsetPersist\n");
    if (!W906TestInsideCtestRoots("L07_AtcPreOffsetPersist"))
        return 2;
    char tmp[MAX_PATH]; ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32]; std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_l07p_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((root + "\\Data\\STR").c_str(), 0);
    ::CreateDirectoryA((root + "\\DefineTemp").c_str(), 0);
    ::CreateDirectoryA((root + "\\Offset").c_str(), 0);
    Spit(root + "\\SetUp.inf", "STR\r\n");
    const int ch = tcAa1;                                    // the first ATC channel (MachineType.h): CH12
    const int plain = 0;                                      // a non-ATC channel: CH1
    char key[16]; std::snprintf(key, sizeof(key), "CH%d", ch + 1);
    char keyPlain[16]; std::snprintf(keyPlain, sizeof(keyPlain), "CH%d", plain + 1);
    const std::string recipe = root + "\\Data\\STR\\Temperature.Data";
    std::string r = "[Mode]\r\nMode=0\r\nTemperature=50.0\r\n[Time]\r\nSoak=10.0\r\n[ATC]\r\nActive Cooling=1\r\nbATCPreOffset=1\r\n";
    r += std::string("[Pre OffSet]\r\n") + key + "=1.5\r\n[Pre OffSet Time]\r\n" + key + "=30\r\n[After Offset]\r\n" + key + "=0.7\r\n";
    r += std::string("[2nd Pre OffSet]\r\n") + key + "=2.5\r\n[2nd Pre OffSet Time]\r\n" + key + "=40\r\n[2nd After Offset]\r\n" + key + "=0.9\r\n";
    Spit(recipe, r);
    DataPath     = AnsiString((root + "\\Data\\").c_str());
    DefaultPath  = AnsiString((root + "\\").c_str());
    OffsetPath   = AnsiString((root + "\\Offset\\").c_str());
    LastDataPath = AnsiString((root + "\\SetUp.inf").c_str());
    const bool authScratch = W906TestSafeLower(AuthPath.c_str()).find("scratch") != std::string::npos;
    const bool genScratch = W906TestSafeLower(asGeneralPath.c_str()).find("scratch") != std::string::npos;
    if (UnderMachineTree(DataPath.c_str()) || UnderMachineTree(DefaultPath.c_str()) || UnderMachineTree(LastDataPath.c_str()) || !authScratch || !genScratch) {
        std::printf("  ABORT: a path is not in the sandbox -- nothing was called\n");
        return 2;
    }
    const char* const kMachine[] = { "D:\\HT9045\\config\\ATC.ini", "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\system\\Gerneral.ini",
                                     "D:\\HT9045\\IniData\\DefineTemp\\Temperature.Data" };
    const int kN = (int)(sizeof(kMachine) / sizeof(kMachine[0]));
    Stamp before[kN];
    for (int i = 0; i < kN; i++) before[i] = StampOf(kMachine[i]);

    OpenGeneralIniFile();
    TfTemp_Set* const oldTempSet = fTemp_Set;
    fTemp_Set = new TfTemp_Set();
    fTemp_Set->Init();
    SystemStart = false;
    IniConfig.bEnable_SECS_GEM = false;
    IniConfig.bA02DisableSaveParsWhenSwitchToOp = false;
    CosFunction.bSaveTemperatureByMachine = false;
    CosFunction.bNotClearAllHotBuffer = false;
    CosFunction.bUseSecondATCTempOffset = true;
    Temperature.bATCActiveCooling = true;
    const int savedAtc = ATC_SYSTEM; ATC_SYSTEM = eNewATCSystem; CosFunction.bUseOldATCTempOffset = false;   // ReadTempFile's ATC block (:2737 ATC_SYSTEM>eATC30) and the hardware-position map (:3496) need the new ATC system

    // [1] read
    fTemp_Set->ReadTempFile(true);
    Check(Temperature.fTempOffSet[PreOffset][ch] == 1.5 && Temperature.fTempOffSet[PreOffsetTime][ch] == 30.0 && Temperature.fTempOffSet[AfterOffset][ch] == 0.7,
          "1a. first group read into rows 17..19 (golden 913 ReadTempFile :2255-2257)");
    Check(Temperature.fTempOffSet[PreOffset2nd][ch] == 2.5 && Temperature.fTempOffSet[PreOffsetTime2nd][ch] == 40.0 && Temperature.fTempOffSet[AfterOffset2nd][ch] == 0.9,
          "1b. second group read into rows 20..22 (:2259-2261)");
    Check(Temperature.fTempOffSet[PreOffset][plain] == 0.0 && Temperature.fTempOffSet[PreOffset2nd][plain] == 0.0, "1c. a channel with no keys reads 0");
    Check(Temperature.bATCPreOffset == true, "1d. [ATC] bATCPreOffset=1 read with ATC active cooling (:2681-2683)");
    { std::string rr = Slurp(recipe); rr.replace(rr.find("Active Cooling=1"), 16, "Active Cooling=0"); Spit(recipe, rr); }   // ReadTempFile reads [ATC] Active Cooling itself (:2805)
    fTemp_Set->ReadTempFile(true);
    Check(Temperature.bATCPreOffset == false, "1e. no ATC active cooling -> bATCPreOffset false (:2684-2685)");
    { std::string rr = Slurp(recipe); rr.replace(rr.find("Active Cooling=0"), 16, "Active Cooling=1"); Spit(recipe, rr); }
    fTemp_Set->ReadTempFile(true);

    // [2] form
    fTemp_Set->DoIniDataToForm(true);
    TMyTempPanel* p = fTemp_Set->myTempPal[ch];
    Check(p != 0, "2. the ATC channel's panel exists after Init");
    if (p != 0) {
        Check(p->edPreOffset->Text == "1.5" && p->edPreOfsTime->Text == "30" && p->edAfterOfs->Text == "0.7",
              "2a. first group boxes filled (\"0.0\" / \"0\" formats, golden 913 :3522-3524)");
        Check(p->ed2ndPreOffset->Text == "2.5" && p->ed2ndPreOfsTime->Text == "40" && p->ed2ndAfterOfs->Text == "0.9",
              "2b. second group boxes filled (:3526-3528)");
        Check(p->ed2ndPreOffset->Visible == false && p->ed2ndAfterOfs->Visible == false, "2c. the second group starts hidden (golden 913 MyTempPanel :322-419)");
    }
    Check(fTemp_Set->chkATCPreOffset->Visible == true && fTemp_Set->chkATCPreOffset->Checked == true && fTemp_Set->cbPreOffset->Visible == true,
          "2d. chkATCPreOffset visible + checked, cbPreOffset visible (:3368-3370)");
    {
        const int tag = p ? p->iIndexTag : -1;
        char m[200];
        std::snprintf(m, sizeof m, "2e. rows -> both groups at the channel's ATC position %d (golden 913 :3340-3349)", tag);
        Check(tag >= 0 && tag < 32 && Temperature.dATCPreOffset[tag] == 1.5 && Temperature.iATCPreOfsTime[tag] == 30 && Temperature.dATCAfterOfs[tag] == 0.7
              && Temperature.d2ndATCPreOffset[tag] == 2.5 && Temperature.i2ndATCPreOfsTime[tag] == 40 && Temperature.d2ndATCAfterOfs[tag] == 0.9, m);
    }

    // [3] save -- the recipe's flag is set to 0 first, so 3d proves SaveSetupFile writes the check box
    { std::string rr = Slurp(recipe); const size_t q = rr.find("bATCPreOffset=1"); if (q != std::string::npos) rr.replace(q, 15, "bATCPreOffset=0");
      const std::string a = std::string(key) + "=1.5", b = std::string(key) + "=0.9";       // and the two values 3e reads, so 3e proves SaveSetupFile wrote them
      const size_t qa = rr.find(a); if (qa != std::string::npos) rr.replace(qa, a.size(), std::string(key) + "=9.9");
      const size_t qb = rr.find(b); if (qb != std::string::npos) rr.replace(qb, b.size(), std::string(key) + "=9.9");
      Spit(recipe, rr); }
    fTemp_Set->spbSaveClick(NULL);
    const std::string def = Slurp(root + "\\DefineTemp\\Temperature.Data");
    Check(def != "<missing>", "3. spbSaveClick wrote the sandbox DefineTemp\\Temperature.Data");
    Check(IniValue(def, "Pre OffSet", key) == "1.5" && IniValue(def, "2nd Pre OffSet Time", key) == "40",
          "3a. DefineTemp: the ATC channel writes the six Pre keys (golden 913 spbSaveClick :4640-4648)");
    Check(IniValue(def, "Low OffSet", key) == "<none>" && IniValue(def, "SHigh OffSet", key) == "<none>" && IniValue(def, "AmbientHotMidOffSet", key) == "<none>",
          "3b. ...and NOT its Low / SHigh / AmbientHotMid keys (golden 913 :4650-4656 else-branch -- GOLDEN BUG kept as is)");
    Check(IniValue(def, "Low OffSet", keyPlain) != "<none>" && IniValue(def, "Pre OffSet", keyPlain) == "<none>",
          "3c. a non-ATC channel writes its plain keys and no Pre keys");
    const std::string rec = Slurp(recipe);
    Check(IniValue(rec, "ATC", "bATCPreOffset") == "1", "3d. recipe: [ATC] bATCPreOffset written by SaveSetupFile (golden 913 :4985)");
    Check(IniValue(rec, "Pre OffSet", key) == "1.5" && IniValue(rec, "2nd After Offset", key) == "0.9",
          "3e. recipe: the six keys written by SaveSetupFile (:5130-5145) -- the place ReadTempFile reads them back from");
    Check(IniValue(rec, "Pre OffSet", keyPlain) == "<none>", "3f. recipe: a non-ATC channel gets no Pre keys (SaveSetupFile :5130-5134 ATC channels only)");
    fTemp_Set->ReadTempFile(true);
    Check(Temperature.fTempOffSet[PreOffset2nd][ch] == 2.5 && Temperature.bATCPreOffset == true, "3g. read back after the save: unchanged");

    for (int i = 0; i < kN; i++)
        Check(SameStamp(before[i], StampOf(kMachine[i])), kMachine[i]);
    ATC_SYSTEM = savedAtc;
    fTemp_Set = oldTempSet;
    std::printf("L07_AtcPreOffsetPersist: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
