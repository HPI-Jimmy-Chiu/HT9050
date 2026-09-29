// =============================================================================
//  test_jam_rules.cpp -- ★W45 (formerly W20): the shared JAM0000.dat rules (JamRules.h) and ELA's use of them.
//
//  AI(W906-ELA-W45) 20260927 (St02-E).  Suite name (add_test): Jam_Rules
//  Research: D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md §5.4 items 1-3.
//  AI(W906-ELA-W45) 20260928 (St02-E helper): Steven 0928 item 18 -- 18b W20-3 = B (the switch), 18c W20-5 = A (the
//  second box, JamRules.h jamrules::SecondBox); sections 2-4 follow the ruling, 5 and 6 are new.
//
//    1. the W906_JAM0000_PATH seam: unset = golden's literal, set = that path;
//    2. the rules as a table, both goldens as the oracle: Include MTBA (Rev891 Analyzer.cpp:2820-2832 =
//       906_0625_Steven cSecurity.cpp:1342-1354), the analyzer's MTBF list (Rev891 Analyzer.cpp:2846-2871), the two
//       customers whose Handler box lives in IncludeMTBF (933 / 967, cSecurity.cpp:1335-1338), the key names, the
//       ★W45 A / B / C defaults, and that the switch is B (Steven 0928);
//    3. ELA's JamConfig (GetJamLevel / GetJemIncludeMTBA / GetJemIncludeMTBF, through JamRules.h) against its pre-W45
//       bodies (copied below verbatim, on TIniFile) -- 33 sections x 12 codes x 4 customers, answers and file bytes:
//       golden mode (Options::goldenJamMtbfDefault) = pre-W45 exactly; the ruled default = pre-W45 but for 18b
//       (933 / 967: a missing IncludeMTBF is true);
//    4. one file, two readers, both orders (the Handler's rule built here on TIniFile + JamRules.h, ELA's real
//       JamConfig): golden mode -> the order decides the file for 933 (golden's conflict, research §4.2 example 1,
//       pinned); the ruling (B) and C -> both orders give the same bytes;
//    5. the second box (18c): its key / caption / default, the "one box per key" and "one default per key" invariants
//       over every section x code x customer, and jamrules::SecondBox on a file (load seeds, apply, save, the other
//       reader sees it, empty code, stale record in the JSON);
//    6. ELA with custCode 933 while another thread holds the JAM0000.dat lock: the ruled default is returned but not
//       written (research §5.4 item 3; waits the lock's 5 s once).
//  Every file is under %TEMP%\ht9045_jam_rules_<tick>; the test refuses to run when that folder or the seam's path
//  resolves under D:\HT9045.  The sandbox is removed on a green run only.
// =============================================================================
#include "JamRules.h"
#include "EventLogAnalysis/ElaCore.h"
#include "vclcompat/IniFiles.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// same guard as tests/test_ela_service.cpp:27-33 (MinGW.org 6.3 strict mode declares neither putenv nor _putenv)
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

using vclcompat::AnsiString;
using vclcompat::TIniFile;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static void PutEnv(const std::string& nameValue)
{
    static std::string keep[8];
    static int n = 0;
    keep[n % 8] = nameValue;
    HT9045_TEST_PUTENV(keep[n % 8].c_str());
    ++n;
}

static std::string FullLower(const std::string& p)
{
    char buf[MAX_PATH * 2];
    const DWORD n = ::GetFullPathNameA(p.c_str(), sizeof(buf), buf, 0);
    std::string s = (n > 0 && n < sizeof(buf)) ? std::string(buf, n) : p;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

static bool UnderMachineTree(const std::string& p)
{
    const std::string l = FullLower(p);
    const std::string r = "d:\\ht9045";
    return l.compare(0, r.size(), r) == 0 && (l.size() == r.size() || l[r.size()] == '\\');
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0)
                continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                RemoveTree(p);
            else
                ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static std::string ReadAll(const std::string& p)
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

static bool NewEmptyFile(const std::string& p)
{
    if (UnderMachineTree(p)) return false;
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

static bool Has(const std::string& hay, const std::string& needle) { return hay.find(needle) != std::string::npos; }

static AnsiString A(const std::string& s) { return AnsiString(s.c_str()); }

// ---- the pre-W45 bodies of ELA's JamConfig getters (EventLogAnalysis/ElaCore.cpp at v906/steven-ela-wip 6871be8e),
//      verbatim but on TIniFile without the named lock (one thread here): the "current behaviour" oracle ----
namespace pre_w45 {
struct Jam
{
    std::string ini;
    static bool FileExistsA(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
    bool CheckAndReadBool(const std::string& group, const std::string& name, bool value)
    {
        TIniFile f(A(ini));
        if (!f.ValueExists(A(group), A(name))) { f.WriteBool(A(group), A(name), value); return value; }
        return f.ReadBool(A(group), A(name), value);
    }
    int CheckAndReadInt(const std::string& group, const std::string& name, int value)
    {
        TIniFile f(A(ini));
        if (!f.ValueExists(A(group), A(name))) { f.WriteInteger(A(group), A(name), value); return value; }
        return f.ReadInteger(A(group), A(name), value);
    }
    int GetJamLevel(const std::string& sJamArea, const std::string& sJamCode)
    {
        int bLevel = 0;
        if (sJamArea != "" && sJamCode != "")
        {
            if (FileExistsA(ini))
                bLevel = CheckAndReadInt(sJamArea, sJamCode, 0);
        }
        return bLevel;
    }
    bool GetJemIncludeMTBA(const std::string& sJamArea, const std::string& sJamCode)
    {
        bool bIncludeMTBA = false;
        if (sJamArea != "" && sJamCode != "")
        {
            if (FileExistsA(ini))
            {
                if (sJamCode.find("JAM") != std::string::npos &&
                    (sJamArea == "01 Input Arm" ||
                     sJamArea == "02 Output Arm" ||
                     sJamArea == "03 Index Unit" ||
                     sJamArea == "04 Input Shuttle" ||
                     sJamArea == "05 Output Shuttle"))
                    bIncludeMTBA = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBA", true);
                else
                    bIncludeMTBA = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBA", false);
            }
        }
        return bIncludeMTBA;
    }
    bool GetJemIncludeMTBF(const std::string& sJamArea, const std::string& sJamCode)
    {
        bool bIncludeMTBF = false;
        if (FileExistsA(ini))
        {
            if (sJamArea != "" && sJamCode != "")
            {
                if (sJamArea == "24 Motor" ||
                    sJamCode == "WAR01300" || sJamCode == "WAR01301" || sJamCode == "WAR0348" ||
                    sJamCode == "JAM0407" || sJamCode == "JAM0408" ||
                    sJamCode == "WAR1635" || sJamCode == "WAR1636" || sJamCode == "WAR1638" || sJamCode == "WAR1639" ||
                    sJamCode == "WAR1690" || sJamCode == "WAR1691" || sJamCode == "WAR1692" || sJamCode == "WAR1693" ||
                    sJamCode == "WAR1694" || sJamCode == "WAR1695" || sJamCode == "WAR1696" || sJamCode == "WAR16109" ||
                    sJamCode == "WAR2201" || sJamCode == "WAR2202" || sJamCode == "WAR2203" ||
                    sJamCode == "WAR16150" || sJamCode == "WAR16151" || sJamCode == "WAR16152" ||
                    sJamCode == "MES16119")
                    bIncludeMTBF = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBF", true);
                else
                    bIncludeMTBF = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBF", false);
            }
        }
        return bIncludeMTBF;
    }
};
}  // namespace pre_w45

// ---- the ruled oracle: pre-W45 with Steven's 0928 18b (W20-3 = B) -- for 933 / 967 a missing IncludeMTBF is true
//      (the Handler's default, 906_0625_Steven cSecurity.cpp:1338); every other customer and getter unchanged ----
namespace ruled_18b {
struct Jam : pre_w45::Jam
{
    bool alias;
    Jam() : alias(false) {}
    bool GetJemIncludeMTBF(const std::string& sJamArea, const std::string& sJamCode)
    {
        if (!alias)
            return pre_w45::Jam::GetJemIncludeMTBF(sJamArea, sJamCode);
        bool bIncludeMTBF = false;
        if (FileExistsA(ini) && sJamArea != "" && sJamCode != "")
            bIncludeMTBF = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBF", true);
        return bIncludeMTBF;
    }
};
}  // namespace ruled_18b

// ---- a reader with the Handler's rule (golden TfSecurity::GetJemIncludeMTBA, 906_0625_Steven cSecurity.cpp:1329-1358,
//      through common.cpp:449-464 CheckAndReadIniData bool), built on TIniFile + JamRules.h ----
static bool HandlerReadIncludeMtba(const std::string& ini, const std::string& area, const std::string& code, bool alias,
                                   jamrules::IncludeMtbfRule rule)
{
    TIniFile f(A(ini));
    const std::string key = jamrules::HandlerIncludeMtbaKey(code, alias);
    const bool def = jamrules::DefaultIncludeMtbaHandler(area, code, alias, rule);
    if (!f.ValueExists(A(area), A(key))) { f.WriteBool(A(area), A(key), def); return def; }
    return f.ReadBool(A(area), A(key), def);
}

// ---- the analyzer's rule for a given switch value (the real ELA JamConfig always uses the configured switch) ----
static bool AnalyzerReadIncludeMtbf(const std::string& ini, const std::string& area, const std::string& code, bool alias,
                                    jamrules::IncludeMtbfRule rule)
{
    TIniFile f(A(ini));
    const std::string key = jamrules::IncludeMtbfKey(code);
    const bool def = jamrules::DefaultIncludeMtbfAnalyzer(area, code, alias, rule);
    if (!f.ValueExists(A(area), A(key))) { f.WriteBool(A(area), A(key), def); return def; }
    return f.ReadBool(A(area), A(key), def);
}

// ---- section 5: jamrules::SecondBox's Io on TIniFile = golden common.cpp CheckAndReadIniData / WriteIniData (bool),
//      what WebSecurityJamW45.h HandlerIo calls in wb_serve (without the setting-change record) ----
struct TIniIo
{
    int reads, writes;
    TIniIo() : reads(0), writes(0) {}
    bool Read(const std::string& file, const std::string& group, const std::string& name, bool def)
    {
        ++reads;
        TIniFile f(A(file));
        if (!f.ValueExists(A(group), A(name))) { f.WriteBool(A(group), A(name), def); return def; }
        return f.ReadBool(A(group), A(name), def);
    }
    void Write(const std::string& file, const std::string& group, const std::string& name, bool value)
    {
        ++writes;
        TIniFile f(A(file));
        f.WriteBool(A(group), A(name), value);
    }
};

// ---- section 5: a stand-in for webbridge::JsonWriter (the calls SecondBox::WriteJson makes) ----
struct FakeWriter
{
    std::string out;
    FakeWriter& Key(const std::string& k) { out += k + "="; return *this; }
    FakeWriter& BeginObject() { out += "{"; return *this; }
    FakeWriter& EndObject() { out += "}"; return *this; }
    FakeWriter& Bool(bool b) { out += b ? "1;" : "0;"; return *this; }
    FakeWriter& String(const std::string& s) { out += "'" + s + "';"; return *this; }
};

// ---- section 6: a second thread that holds the JAM0000.dat named lock (JamIniMerge.h:37 / ElaCore.cpp JamFileLock) ----
struct LockHolder
{
    HANDLE ready, release;
    bool got;
};
static DWORD WINAPI HoldJamLock(LPVOID p)
{
    LockHolder* h = static_cast<LockHolder*>(p);
    HANDLE m = ::CreateMutexA(NULL, FALSE, "Local\\HT9045_JAM0000_dat");
    const DWORD r = m ? ::WaitForSingleObject(m, 5000) : WAIT_FAILED;
    h->got = (r == WAIT_OBJECT_0 || r == WAIT_ABANDONED);
    ::SetEvent(h->ready);
    ::WaitForSingleObject(h->release, 30000);
    if (h->got) ::ReleaseMutex(m);
    if (m) ::CloseHandle(m);
    return 0;
}

int main()
{
    printf("Jam_Rules\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    std::string root = std::string(tmp) + "ht9045_jam_rules_" + tick;
    if (UnderMachineTree(root))
    {
        printf("  REFUSED: sandbox %s is under D:\\HT9045\n", root.c_str());
        return 1;
    }
    ::CreateDirectoryA(root.c_str(), 0);

    // ---------------------------------------------------------------------------------------------------------
    // 1. the seam
    // ---------------------------------------------------------------------------------------------------------
    PutEnv("W906_JAM0000_PATH=");
    CHECK(jamrules::Jam0000Path() == "D:\\HT9045\\Error\\English\\JAM0000.dat",
          "1. W906_JAM0000_PATH unset / empty: golden's literal (906_0625_Steven cSecurity.cpp:216, Rev891 Analyzer.cpp:296)");
    const std::string seam = root + "\\seam\\JAM0000.dat";
    PutEnv("W906_JAM0000_PATH=" + seam);
    CHECK(jamrules::Jam0000Path() == seam, "1. W906_JAM0000_PATH set: that path");
    if (UnderMachineTree(jamrules::Jam0000Path()))
    {
        printf("  REFUSED: W906_JAM0000_PATH resolves under D:\\HT9045 (%s)\n", jamrules::Jam0000Path().c_str());
        return 1;
    }

    // ---------------------------------------------------------------------------------------------------------
    // 2. the rules, both goldens as the oracle
    // ---------------------------------------------------------------------------------------------------------
    CHECK(jamrules::DefaultIncludeMtba("01 Input Arm", "JAM0109") && jamrules::DefaultIncludeMtba("05 Output Shuttle", "JAM0501") &&
              !jamrules::DefaultIncludeMtba("16 System", "JAM1601") && !jamrules::DefaultIncludeMtba("01 Input Arm", "WAR0101") &&
              !jamrules::DefaultIncludeMtba("06 Empty Tray Arm", "JAM0601") && jamrules::DefaultIncludeMtba("03 Index Unit", "XJAM9"),
          "2. Include MTBA: JAM anywhere in the code (Pos) and areas 01..05 (Analyzer.cpp:2820-2832 = cSecurity.cpp:1342-1354)");
    bool all24 = true;
    static const char* const k24[] = { "WAR01300", "WAR01301", "WAR0348", "JAM0407", "JAM0408", "WAR1635", "WAR1636",
                                       "WAR1638", "WAR1639", "WAR1690", "WAR1691", "WAR1692", "WAR1693", "WAR1694",
                                       "WAR1695", "WAR1696", "WAR16109", "WAR2201", "WAR2202", "WAR2203", "WAR16150",
                                       "WAR16151", "WAR16152", "MES16119" };
    for (size_t i = 0; i < sizeof(k24) / sizeof(k24[0]); ++i)
        if (!jamrules::AnalyzerMtbfListed("16 System", k24[i])) all24 = false;
    CHECK(all24 && jamrules::AnalyzerMtbfListed("24 Motor", "WAR2401") && jamrules::AnalyzerMtbfListed("24 Motor", "ANY") &&
              !jamrules::AnalyzerMtbfListed("07 Tester I/F", "WAR0701") && !jamrules::AnalyzerMtbfListed("16 System", "WAR163") &&
              !jamrules::AnalyzerMtbfListed("16 System", "WAR16109X"),
          "2. the analyzer's MTBF list: 24 Motor or the 24 codes, exact (Analyzer.cpp:2846-2871)");
    CHECK(jamrules::IsMtbfAlias(933) && jamrules::IsMtbfAlias(967) && !jamrules::IsMtbfAlias(868) && !jamrules::IsMtbfAlias(0) &&
              jamrules::IsMtbfAlias(std::string("933")) && jamrules::IsMtbfAlias(std::string("967")) &&
              !jamrules::IsMtbfAlias(std::string("")) && !jamrules::IsMtbfAlias(std::string("000")),
          "2. the IncludeMTBF-alias customers: CC_ASE_CL 933, CC_TERAPOWER 967 (MachineType.h:300 / :337)");
    CHECK(jamrules::HandlerIncludeMtbaKey("JAM0305", true) == "JAM0305 IncludeMTBF" &&
              jamrules::HandlerIncludeMtbaKey("JAM0305", false) == "JAM0305 IncludeMTBA" &&
              jamrules::IncludeMtbaKey("W1") == "W1 IncludeMTBA" && jamrules::IncludeMtbfKey("W1") == "W1 IncludeMTBF" &&
              jamrules::LevelKey("W1") == "W1" && jamrules::DefaultJamLevel() == 0,
          "2. key names; the Handler's Include MTBA box is IncludeMTBF for 933 / 967 (cSecurity.cpp:1335-1353, :1138-1146)");
    using jamrules::kIncludeMtbfGoldenPerSide;
    using jamrules::kIncludeMtbfHandlerWins;
    using jamrules::kIncludeMtbfAnalyzerWins;
    // JAM0305 in 03 Index Unit for 933: the research's example 1 (§4.2)
    const bool aA = jamrules::DefaultIncludeMtbfAnalyzer("03 Index Unit", "JAM0305", true, kIncludeMtbfGoldenPerSide);
    const bool hA = jamrules::DefaultIncludeMtbaHandler("03 Index Unit", "JAM0305", true, kIncludeMtbfGoldenPerSide);
    const bool aB = jamrules::DefaultIncludeMtbfAnalyzer("03 Index Unit", "JAM0305", true, kIncludeMtbfHandlerWins);
    const bool hB = jamrules::DefaultIncludeMtbaHandler("03 Index Unit", "JAM0305", true, kIncludeMtbfHandlerWins);
    const bool aC = jamrules::DefaultIncludeMtbfAnalyzer("03 Index Unit", "JAM0305", true, kIncludeMtbfAnalyzerWins);
    const bool hC = jamrules::DefaultIncludeMtbaHandler("03 Index Unit", "JAM0305", true, kIncludeMtbfAnalyzerWins);
    CHECK(!aA && hA, "2. ★W45 A (golden per side), 933 JAM0305: analyzer false (Analyzer.cpp:2877), Handler true (cSecurity.cpp:1338)");
    CHECK(aB && hB, "2. ★W45 B (the Handler's rule): both true");
    CHECK(!aC && !hC, "2. ★W45 C (the analyzer's rule): both false");
    CHECK(hB == hA, "2. ★W45 B leaves the Handler's own default golden's (Steven 0928: the Handler getter is unchanged)");
    bool others = true;
    for (int r = 0; r < 3; ++r)
    {
        const jamrules::IncludeMtbfRule rule =
            r == 0 ? kIncludeMtbfGoldenPerSide : r == 1 ? kIncludeMtbfHandlerWins : kIncludeMtbfAnalyzerWins;
        // a customer that is not 933 / 967: the analyzer's list and the Handler's Include MTBA rule, whatever the switch
        if (jamrules::DefaultIncludeMtbfAnalyzer("03 Index Unit", "JAM0305", false, rule) ||
            !jamrules::DefaultIncludeMtbfAnalyzer("24 Motor", "WAR2401", false, rule) ||
            !jamrules::DefaultIncludeMtbaHandler("03 Index Unit", "JAM0305", false, rule) ||
            jamrules::DefaultIncludeMtbaHandler("24 Motor", "WAR2401", false, rule))
            others = false;
        // listed codes agree for 933 under every switch value
        if (!jamrules::DefaultIncludeMtbfAnalyzer("24 Motor", "WAR2401", true, rule) ||
            !jamrules::DefaultIncludeMtbaHandler("24 Motor", "WAR2401", true, rule))
            others = false;
    }
    CHECK(others, "2. other customers: the switch changes nothing; a listed code (24 Motor) is true for 933 under A / B / C");
    CHECK(jamrules::kIncludeMtbfRule == kIncludeMtbfHandlerWins,
          "2. the ★W45 switch is B (Steven 0928 item 18b, W20-3 = B) -- update this line with JamRules.h's switch");

    // ---------------------------------------------------------------------------------------------------------
    // 3. ELA JamConfig (through JamRules.h) against its pre-W45 bodies: answers and file bytes
    // ---------------------------------------------------------------------------------------------------------
    std::vector<std::string> areas;
    for (int i = 0; i < 31; ++i) areas.push_back(ela::kJamAreaNames[i]);
    areas.push_back("Process");                                  // CSV UnitNames the analyzer also queries
    areas.push_back("Motion");
    static const char* const kCodes[] = { "JAM0101", "JAM0305", "JAM0407", "JAM0408", "WAR0101", "WAR2401", "WAR01300",
                                          "WAR16150", "MES16119", "MES2110", "WAR163", "XJAM9" };
    const size_t kCodeCount = sizeof(kCodes) / sizeof(kCodes[0]);
    static const char* const kCust[] = { "", "933", "967", "868" };
    const size_t kCustCount = sizeof(kCust) / sizeof(kCust[0]);
    {
        CHECK(!ela::Options().goldenJamMtbfDefault, "3. ela::Options default = the ruling (goldenJamMtbfDefault false)");
        bool answers[2] = { true, true }, bytes[2] = { true, true };
        int calls = 0;
        for (int mode = 0; mode < 2; ++mode)                     // 0 = golden (Rev891), 1 = the ruling (18b = B)
            for (size_t c = 0; c < kCustCount; ++c)
            {
                const std::string tag = std::string(mode == 0 ? "golden_" : "ruled_") + (kCust[c][0] ? kCust[c] : "none");
                const std::string fNew = root + "\\s3_new_" + tag + ".dat";
                const std::string fOld = root + "\\s3_old_" + tag + ".dat";
                if (!NewEmptyFile(fNew) || !NewEmptyFile(fOld)) { answers[mode] = false; break; }
                ela::Options o;
                o.jamIniPath = fNew;
                o.custCode = kCust[c];
                o.goldenJamMtbfDefault = (mode == 0);
                ela::JamConfig now(o);
                ruled_18b::Jam old;                              // alias false = pre_w45 exactly
                old.ini = fOld;
                old.alias = (mode == 1) && jamrules::IsMtbfAlias(std::string(kCust[c]));
                for (size_t a = 0; a < areas.size(); ++a)
                    for (size_t k = 0; k < kCodeCount; ++k)
                    {
                        if (now.GetJamLevel(areas[a], kCodes[k]) != old.GetJamLevel(areas[a], kCodes[k])) answers[mode] = false;
                        if (now.GetJemIncludeMTBA(areas[a], kCodes[k]) != old.GetJemIncludeMTBA(areas[a], kCodes[k])) answers[mode] = false;
                        if (now.GetJemIncludeMTBF(areas[a], kCodes[k]) != old.GetJemIncludeMTBF(areas[a], kCodes[k])) answers[mode] = false;
                        calls += 3;
                    }
                // a second pass reads what the first wrote
                for (size_t a = 0; a < areas.size(); ++a)
                    if (now.GetJemIncludeMTBF(areas[a], "JAM0305") != old.GetJemIncludeMTBF(areas[a], "JAM0305")) answers[mode] = false;
                const std::string bNew = ReadAll(fNew), bOld = ReadAll(fOld);
                if (bNew.empty() || bNew != bOld) bytes[mode] = false;
            }
        CHECK(calls == 2 * 4 * 33 * 12 * 3, "3. 2 modes x 4 customers (\"\", 933, 967, 868) x 33 sections x 12 codes x 3 getters");
        CHECK(answers[0], "3. golden mode (goldenJamMtbfDefault): every answer equals the pre-W45 getter's");
        CHECK(bytes[0], "3. golden mode: the written JAM0000.dat is byte for byte the pre-W45 one");
        CHECK(answers[1], "3. the ruling: every answer equals pre-W45, but 933 / 967 missing IncludeMTBF = true (18b)");
        CHECK(bytes[1], "3. the ruling: the written JAM0000.dat is byte for byte that oracle's");
        // the ruling is in effect: 933, 03 Index Unit / JAM0305 (not listed) -- pre-W45 false, now true; 868 unchanged
        const std::string f933 = root + "\\s3_effect_933.dat", fPre = root + "\\s3_effect_pre.dat", f868 = root + "\\s3_effect_868.dat";
        bool effect = NewEmptyFile(f933) && NewEmptyFile(fPre) && NewEmptyFile(f868);
        ela::Options o933;
        o933.jamIniPath = f933;
        o933.custCode = "933";
        ela::JamConfig j933(o933);
        pre_w45::Jam pre;
        pre.ini = fPre;
        ela::Options o868 = o933;
        o868.jamIniPath = f868;
        o868.custCode = "868";
        ela::JamConfig j868(o868);
        effect = effect && j933.GetJemIncludeMTBF("03 Index Unit", "JAM0305") && !pre.GetJemIncludeMTBF("03 Index Unit", "JAM0305") &&
                 !j868.GetJemIncludeMTBF("03 Index Unit", "JAM0305") && Has(ReadAll(f933), "JAM0305 IncludeMTBF=1") &&
                 Has(ReadAll(fPre), "JAM0305 IncludeMTBF=0") && Has(ReadAll(f868), "JAM0305 IncludeMTBF=0");
        CHECK(effect, "3. 18b in effect: 933 seeds JAM0305 IncludeMTBF=1 (Rev891 would seed 0); 868 still seeds 0");
        const std::string missing = root + "\\s3_missing.dat";
        ela::Options om;
        om.jamIniPath = missing;
        om.custCode = "933";
        ela::JamConfig jm(om);
        CHECK(!jm.GetJemIncludeMTBA("01 Input Arm", "JAM0101") && !jm.GetJemIncludeMTBF("24 Motor", "WAR2401") &&
                  !jm.GetJemIncludeMTBF("03 Index Unit", "JAM0305") && jm.GetJamLevel("01 Input Arm", "JAM0101") == 0 &&
                  ::GetFileAttributesA(missing.c_str()) == INVALID_FILE_ATTRIBUTES,
              "3. no JAM0000.dat: false / 0 and nothing written (Rev891 FileExists checks, unchanged by the ruling)");
        ela::Analyzer an(om);
        an.SetCustCode("933");
        CHECK(an.options().custCode == "933", "3. Analyzer::SetCustCode (also passed to JamConfig for ★W45)");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 4. one file, two readers, both orders (research §5.4 item 2)
    // ---------------------------------------------------------------------------------------------------------
    {
        const std::string area = "03 Index Unit", code = "JAM0305";
        // golden mode: the real ELA JamConfig (custCode 933, goldenJamMtbfDefault) and the Handler's rule (A)
        const std::string f1 = root + "\\s4_ela_first.dat", f2 = root + "\\s4_handler_first.dat";
        CHECK(NewEmptyFile(f1) && NewEmptyFile(f2), "4. two empty files");
        ela::Options o1;
        o1.jamIniPath = f1;
        o1.custCode = "933";
        o1.goldenJamMtbfDefault = true;
        ela::JamConfig e1(o1);
        const bool ela1 = e1.GetJemIncludeMTBF(area, code);                         // ELA first: writes 0
        const bool hnd1 = HandlerReadIncludeMtba(f1, area, code, true, kIncludeMtbfGoldenPerSide);
        const bool hnd2 = HandlerReadIncludeMtba(f2, area, code, true, kIncludeMtbfGoldenPerSide);   // Handler first: writes 1
        ela::Options o2 = o1;
        o2.jamIniPath = f2;
        ela::JamConfig e2(o2);
        const bool ela2 = e2.GetJemIncludeMTBF(area, code);
        CHECK(!ela1 && !hnd1 && hnd2 && ela2 && ReadAll(f1) != ReadAll(f2),
              "4. golden mode (A), 933: the first reader decides the file (ELA first -> both false, Handler first -> both true) -- golden's conflict");
        // the ruling: the real ELA JamConfig with the default Options and the Handler's rule with the configured switch
        const std::string r1 = root + "\\s4_ruled_ela_first.dat", r2 = root + "\\s4_ruled_handler_first.dat";
        bool ruled = NewEmptyFile(r1) && NewEmptyFile(r2);
        ela::Options q1;
        q1.jamIniPath = r1;
        q1.custCode = "933";
        ela::JamConfig g1(q1);
        const bool x1 = g1.GetJemIncludeMTBF(area, code);
        const bool y1 = HandlerReadIncludeMtba(r1, area, code, true, jamrules::kIncludeMtbfRule);
        const bool y2 = HandlerReadIncludeMtba(r2, area, code, true, jamrules::kIncludeMtbfRule);
        ela::Options q2 = q1;
        q2.jamIniPath = r2;
        ela::JamConfig g2(q2);
        const bool x2 = g2.GetJemIncludeMTBF(area, code);
        ruled = ruled && x1 && y1 && x2 && y2 && ReadAll(r1) == ReadAll(r2) && Has(ReadAll(r1), "JAM0305 IncludeMTBF=1");
        CHECK(ruled, "4. the ruling (18b = B), 933, the real ELA JamConfig: both orders -> true and the same bytes");
        // C (not ruled; kept as the switch's third value): both readers with C -> the same bytes in both orders
        bool same = true;
        const std::string g1c = root + "\\s4_ruleC_ela_first.dat", g2c = root + "\\s4_ruleC_handler_first.dat";
        if (!NewEmptyFile(g1c) || !NewEmptyFile(g2c)) same = false;
        const bool c1 = AnalyzerReadIncludeMtbf(g1c, area, code, true, kIncludeMtbfAnalyzerWins);
        const bool d1 = HandlerReadIncludeMtba(g1c, area, code, true, kIncludeMtbfAnalyzerWins);
        const bool d2 = HandlerReadIncludeMtba(g2c, area, code, true, kIncludeMtbfAnalyzerWins);
        const bool c2 = AnalyzerReadIncludeMtbf(g2c, area, code, true, kIncludeMtbfAnalyzerWins);
        if (c1 != d1 || c2 != d2 || c1 != c2 || ReadAll(g1c) != ReadAll(g2c)) same = false;
        CHECK(same, "4. ★W45 C: both orders give the same answer and the same bytes");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 5. the second box (18c, W20-5 = A)
    // ---------------------------------------------------------------------------------------------------------
    {
        CHECK(std::string(jamrules::kSecondBoxId) == "cbIncludeMTBF" &&
                  jamrules::SecondBoxKey("JAM0305", false) == "JAM0305 IncludeMTBF" &&
                  jamrules::SecondBoxKey("JAM0305", true) == "JAM0305 IncludeMTBA" &&
                  jamrules::SecondBoxCaption(false) == "Include MTBF" &&
                  jamrules::SecondBoxCaption(true) == "Include MTBA (Analyzer)",
              "5. box 2 = cbIncludeMTBF (Analyzer.dfm:2029-2042): IncludeMTBF \"Include MTBF\"; 933 / 967 IncludeMTBA \"Include MTBA (Analyzer)\"");
        // every section x code x customer: the two boxes bind the two keys (never the same one), and each box's default
        // is ELA's default for that key (IncludeMTBA -> GetJemIncludeMTBA's, IncludeMTBF -> GetJemIncludeMTBF's)
        static const int kCustInt[] = { 0, 933, 967, 868 };
        bool oneKeyEach = true, oneDefault = true;
        int goldenMismatch = 0, goldenMismatchAlias = 0, cells = 0;
        for (size_t c = 0; c < sizeof(kCustInt) / sizeof(kCustInt[0]); ++c)
        {
            const bool alias = jamrules::IsMtbfAlias(kCustInt[c]);
            for (size_t a = 0; a < areas.size(); ++a)
                for (size_t k = 0; k < kCodeCount; ++k)
                {
                    const std::string& ar = areas[a];
                    const std::string cd = kCodes[k];
                    const std::string k1 = jamrules::HandlerIncludeMtbaKey(cd, alias), k2 = jamrules::SecondBoxKey(cd, alias);
                    if (k1 == k2 || (k1 != jamrules::IncludeMtbaKey(cd) && k1 != jamrules::IncludeMtbfKey(cd)) ||
                        (k2 != jamrules::IncludeMtbaKey(cd) && k2 != jamrules::IncludeMtbfKey(cd)))
                        oneKeyEach = false;
                    // ELA's default for a key, the ruled switch / golden
                    for (int g = 0; g < 2; ++g)
                    {
                        const jamrules::IncludeMtbfRule rule = g ? kIncludeMtbfGoldenPerSide : jamrules::kIncludeMtbfRule;
                        const bool box1 = jamrules::DefaultIncludeMtbaHandler(ar, cd, alias, rule);
                        const bool box2 = jamrules::DefaultSecondBox(ar, cd, alias, rule);
                        const bool ela1 = (k1 == jamrules::IncludeMtbaKey(cd))
                                              ? jamrules::DefaultIncludeMtba(ar, cd)
                                              : jamrules::DefaultIncludeMtbfAnalyzer(ar, cd, alias, rule);
                        const bool ela2 = (k2 == jamrules::IncludeMtbaKey(cd))
                                              ? jamrules::DefaultIncludeMtba(ar, cd)
                                              : jamrules::DefaultIncludeMtbfAnalyzer(ar, cd, alias, rule);
                        if (g == 0 && (box1 != ela1 || box2 != ela2)) oneDefault = false;
                        if (g == 1 && box2 != ela2) oneDefault = false;          // box 2 agrees with ELA in golden mode too
                        if (g == 1 && box1 != ela1) { ++goldenMismatch; if (alias) ++goldenMismatchAlias; }
                    }
                    ++cells;
                }
        }
        CHECK(cells == 4 * 33 * 12 && oneKeyEach,
              "5. one box per key: box 1 and box 2 bind IncludeMTBA / IncludeMTBF, never the same key (4 x 33 x 12)");
        CHECK(oneDefault, "5. the ruling: every key has ONE missing-key default -- the page's boxes and ELA seed the same value");
        CHECK(goldenMismatch > 0 && goldenMismatch == goldenMismatchAlias,
              "5. golden mode: the only disagreement is box 1 (IncludeMTBF) on 933 / 967 -- the conflict 18b removes");

        TIniIo io;
        // other customer (868): box 2 = IncludeMTBF, default = the analyzer's list
        const std::string f868 = root + "\\s5_box2_868.dat";
        CHECK(NewEmptyFile(f868), "5. an empty JAM0000.dat for 868");
        jamrules::SecondBox b;
        CHECK(!b.Apply("cbIncludeMTBF", true) && !b.loaded(), "5. before a Load the box takes no click");
        b.Load(io, f868, "03 Index Unit", "JAM0305", 868);
        const std::string after1 = ReadAll(f868);
        CHECK(b.loaded() && !b.checked() && !b.alias() && Has(after1, "[03 Index Unit]") && Has(after1, "JAM0305 IncludeMTBF=0"),
              "5. 868 Load: missing IncludeMTBF seeded with the analyzer's default 0 (CheckAndReadIniData, Analyzer.cpp:2877)");
        b.Load(io, f868, "03 Index Unit", "JAM0305", 868);
        CHECK(ReadAll(f868) == after1, "5. a second Load of an existing key writes nothing");
        CHECK(!b.Apply("cbIncludeMTBA", true) && !b.checked(), "5. another widget's name is not box 2's (golden cbIncludeMTBA stays TfSecurity's)");
        CHECK(b.Apply("cbIncludeMTBF", true) && b.checked(), "5. Apply cbIncludeMTBF = the operator's click");
        const int w0 = io.writes;
        CHECK(b.Save(io) && io.writes == w0 + 1 && Has(ReadAll(f868), "JAM0305 IncludeMTBF=1"),
              "5. Save writes the record Load read (Analyzer.cpp:2772)");
        ela::Options oe;
        oe.jamIniPath = f868;
        oe.custCode = "868";
        ela::JamConfig e868(oe);
        CHECK(e868.GetJemIncludeMTBF("03 Index Unit", "JAM0305"),
              "5. one file, two readers: ELA's MTBF sees what box 2 saved (868)");
        FakeWriter fw;
        b.WriteJson(fw, "03 Index Unit", "JAM0305");
        CHECK(fw.out == "cbIncludeMTBF={checked=1;visible=1;enabled=1;caption='Include MTBF';key='JAM0305 IncludeMTBF';}",
              "5. boxes.cbIncludeMTBF JSON for the record shown");
        FakeWriter fs;
        b.WriteJson(fs, "03 Index Unit", "JAM0301");
        CHECK(fs.out == "cbIncludeMTBF={checked=0;visible=0;enabled=0;caption='Include MTBF';key='';}",
              "5. a record Load did not read (a stale state) hides the box");
        // switching records (select): save "from", then load "to" -- the new record's value is its own
        b.Apply("cbIncludeMTBF", false);
        b.Save(io);
        b.Load(io, f868, "24 Motor", "WAR2401", 868);
        CHECK(b.checked() && Has(ReadAll(f868), "WAR2401 IncludeMTBF=1") && Has(ReadAll(f868), "JAM0305 IncludeMTBF=0"),
              "5. SaveThenLoad: from saved (JAM0305=0), to loaded with its own default (24 Motor -> 1)");
        // empty code: Load reads nothing, Save writes nothing (Analyzer.cpp:2759-2766 / cSecurity.cpp:1065 / :1125)
        const int r0 = io.reads, w1 = io.writes;
        b.Load(io, f868, "16 System", "", 868);
        CHECK(!b.checked() && io.reads == r0 && !b.Save(io) && io.writes == w1,
              "5. an empty code: nothing read, nothing written");

        // 933: box 2 = IncludeMTBA (the analyzer's MTBA key), box 1 (the Handler's rule) = IncludeMTBF
        const std::string f933 = root + "\\s5_box2_933.dat";
        CHECK(NewEmptyFile(f933), "5. an empty JAM0000.dat for 933");
        jamrules::SecondBox b9;
        b9.Load(io, f933, "03 Index Unit", "JAM0305", 933);
        CHECK(b9.alias() && b9.checked() && Has(ReadAll(f933), "JAM0305 IncludeMTBA=1") && !Has(ReadAll(f933), "IncludeMTBF"),
              "5. 933 Load: box 2 is IncludeMTBA, seeded with the MTBA default 1 (JAM in 03); IncludeMTBF untouched");
        b9.Apply("cbIncludeMTBF", false);
        b9.Save(io);
        const bool box1 = HandlerReadIncludeMtba(f933, "03 Index Unit", "JAM0305", true, jamrules::kIncludeMtbfRule);
        ela::Options o9;
        o9.jamIniPath = f933;
        o9.custCode = "933";
        ela::JamConfig e933(o9);
        CHECK(!e933.GetJemIncludeMTBA("03 Index Unit", "JAM0305") && box1 && e933.GetJemIncludeMTBF("03 Index Unit", "JAM0305") &&
                  Has(ReadAll(f933), "JAM0305 IncludeMTBA=0") && Has(ReadAll(f933), "JAM0305 IncludeMTBF=1"),
              "5. 933: box 2 unticked -> ELA MTBA false; box 1's key IncludeMTBF (Handler true) -> ELA MTBF true");
        FakeWriter f9;
        b9.WriteJson(f9, "03 Index Unit", "JAM0305");
        CHECK(f9.out == "cbIncludeMTBF={checked=0;visible=1;enabled=1;caption='Include MTBA (Analyzer)';key='JAM0305 IncludeMTBA';}",
              "5. 933 JSON: caption Include MTBA (Analyzer), key IncludeMTBA");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 6. ELA, custCode 933, while another thread holds the JAM0000.dat lock (research §5.4 item 3)
    // ---------------------------------------------------------------------------------------------------------
    {
        const std::string f = root + "\\s6_locked.dat";
        CHECK(NewEmptyFile(f), "6. an empty JAM0000.dat");
        LockHolder h;
        h.ready = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        h.release = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        h.got = false;
        HANDLE th = ::CreateThread(NULL, 0, &HoldJamLock, &h, 0, NULL);
        const bool started = th != NULL && ::WaitForSingleObject(h.ready, 10000) == WAIT_OBJECT_0 && h.got;
        CHECK(started, "6. a second thread holds Local\\HT9045_JAM0000_dat");
        ela::Options o;
        o.jamIniPath = f;
        o.custCode = "933";
        ela::JamConfig j(o);
        const bool v = j.GetJemIncludeMTBF("03 Index Unit", "JAM0305");                // waits the 5 s, then reads only
        CHECK(v && j.writes == 0 && ReadAll(f).empty(),
              "6. lock busy: the ruled default (true) is returned, NOT written (the next query writes it)");
        ::SetEvent(h.release);
        if (th) { ::WaitForSingleObject(th, 30000); ::CloseHandle(th); }
        ::CloseHandle(h.ready);
        ::CloseHandle(h.release);
        const bool v2 = j.GetJemIncludeMTBF("03 Index Unit", "JAM0305");
        CHECK(v2 && j.writes == 1 && Has(ReadAll(f), "JAM0305 IncludeMTBF=1"), "6. lock free again: written (=1)");
    }

    printf("Jam_Rules: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
        RemoveTree(root);
    else
        printf("  sandbox kept: %s\n", root.c_str());
    return g_fail ? 1 : 0;
}
