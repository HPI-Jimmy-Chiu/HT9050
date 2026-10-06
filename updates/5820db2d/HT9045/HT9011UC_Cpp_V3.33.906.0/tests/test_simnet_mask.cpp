// =============================================================================
//  test_simnet_mask.cpp -- W36-1: the SIM build starts every launch with the FTP / network options off, "this run only".
//
//  AI(W906-SIM-W36-1) 20260928 (St02-E helper) prototype; AI(W906-W58) 20260930 (St02-E): Steven's W58 answers
//  (decisions-decided.md:2779-2812): Q1 = 要, Q2 = 要, Q3 = 關 + enable, Q5 = 算.  Suite name (add_test): SimNet_Mask
//
//  SimNet/SimNetMask.cpp is compiled here with W906_SIMNET_CORE_ONLY: the real key table + the Mask + the SIM gate, no
//  Handler globals.  A fake Handler stands in for ReadLastSetIni / SaveLastSetIni (fields, page proxies, the golden
//  writers: WriteIniData = TIniFile in place, HTEditList = TMemIniFile rewrite).  The ELA part runs the REAL readers
//  (ela::IniCheckAndReadBool, ela::ReadScheduleConfig) through the override (W906_ElaSetBoolOverride).
//    0. containment first: the sandbox is %TEMP%\ht9045_simnet_<tick>, refused (exit 2, nothing written) when it is under
//       D:\HT9045* / D:\RMS / D:\MTBF_Summary / D:\EventlogAnalyzer; the machine's config.ini / Gerneral.ini are stat'ed
//       read-only before and after (size + write time); no override is installed when the test starts;
//    1. the table (W58 Q2 = 要, Jimmy RULINGS_20261001 #3): 58 rows -- A / B and table C's network keys in, N07-1 / N07-2, C's local keys and D
//       not in, no duplicate;
//    2. the raw ini helpers (verbatim value, first section, ';' comments, in-place write, remove key / empty section);
//    3. launch (phase 0): armed keys read false, proxies unchecked, the N31 radio 0, a customer-forced key too (Q3 = 關);
//       every key outside the table keeps the file value; the file is not written;
//    4. the ELA while armed: armed -> false, not masked / not in the table / not config.ini -> the file value,
//       a missing key with default true -> false;
//    5. a save with nothing released: phase 1 unmasks (the save writes the file's own values), phase 2 writes the
//       snapshot back even over a stray writer; the file is byte-identical (a missing key / section added by the save
//       is removed again);
//    6. released (checked + saved) -> on (field true, proxy checked, ELA true) and written to config.ini (Q1 = 要); a
//       customer-forced key is on for this run only (HTEditList never saves it); a later page open keeps it on;
//       unchecked + saved -> off again, and 0 in the file (the operator's choice);
//    7. a new launch (Arm + phase 0) -> off again although the file now says on; a save then keeps the file's on;
//    8. not ready (the edit lists not registered): rows stay pending, fields untouched, the ELA reads off; resolved later;
//    9. the SIM gate: SimBuild() == !W906_NO_SOFT_SIMULTE; InstallHooks sets the slot + the ELA setter in SIM, touches
//       nothing in SHIP (the build_ship configuration compiles this test with the SHIP expectations);
//   10. the override removed -> the ELA reads the file again; the machine files unchanged; the sandbox removed if green.
//  Safe to run by hand: it writes nothing outside its own %TEMP% sandbox and calls no Handler code.
// =============================================================================
#include "SimNet/SimNetMask.h"
#include "EventLogAnalysis/ElaHub.h"        // ela::IniCheckAndReadBool / IniBoolOverride
#include "EventLogAnalysis/ElaSchedule.h"   // ela::ReadScheduleConfig
#include "EventLogAnalysis/ElaService.h"    // W906_ElaSetBoolOverride
#include "vclcompat/IniFiles.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using vclcompat::AnsiString;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                                     \
    do {                                                                                     \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                            \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }      \
        std::fflush(stdout);                                                                 \
    } while (0)

namespace {

std::string g_root, g_cfg, g_gen;

// ---- files ----------------------------------------------------------------------------------------------------------
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    const char* const bad[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary", "d:\\eventloganalyzer", "d:\\handlersummary" };
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
        if (s.compare(0, std::strlen(bad[i]), bad[i]) == 0) return true;
    return false;
}

std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    std::size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

void Put(const std::string& p, const std::string& data)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return;
    std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

struct Stamp
{
    bool exists;
    unsigned long long size, write;
};

Stamp StampOf(const char* p)
{
    Stamp s;
    s.exists = false;
    s.size = s.write = 0;
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (::GetFileAttributesExA(p, GetFileExInfoStandard, &a))
    {
        s.exists = true;
        s.size = ((unsigned long long)a.nFileSizeHigh << 32) | a.nFileSizeLow;
        s.write = ((unsigned long long)a.ftLastWriteTime.dwHighDateTime << 32) | a.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}

bool SameStamp(const Stamp& a, const Stamp& b) { return a.exists == b.exists && a.size == b.size && a.write == b.write; }

void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

// BCB6 TMemIniFile layout (vclcompat TIniStore::SaveToFile(.., true)): a blank line after every section, CRLF.  The
// fixture is written in it, so the HTEditList-style rewrite in HandlerSave round-trips byte for byte.
const char* const kFixture =
    "[FTPUpLoad]\r\n"
    "bEnable_FTPUpLoadLog=1\r\n"
    "bN10_UploadSummaryToFTP=0\r\n"
    "iN10UploadMethod=1\r\n"
    "\r\n"
    "[ChipMos Function]\r\n"
    "bN25_1_EnableStartControl=1\r\n"
    "bN25_3_EnableULJamLog=1\r\n"
    "bN25_4_EnableUpload=1\r\n"
    "bN25_5_EnableUpload=0\r\n"
    "sN25_2_FTPHost=10.20.50.3\r\n"
    "\r\n"
    "[Event Log]\r\n"
    "EnableAutoSaveEventLog=1\r\n"
    "bAlarmStatistAutoSaveNetDrive=1\r\n"
    "\r\n"
    "[SECS GEM]\r\n"
    "Enable SECS GEM=1\r\n"
    "Wait SECS=1\r\n"
    "Enable RCMD START=1\r\n"
    "\r\n"
    "[FTP]\r\n"
    "Enable FTP=1\r\n"
    "Enable FTP Password Download=0\r\n"
    "\r\n"
    "[RMS]\r\n"
    "RMS Enable=1\r\n"
    "\r\n"
    "[bAutoTmpeOfsByFTP]\r\n"
    "bN31_UseAutoTempOfsByFTP=2\r\n"
    "\r\n"
    "[Production_Log]\r\n"
    "bN17UploadProdLog=1\r\n"
    "\r\n";
// not in the fixture on purpose: [FTPUpLoad] bN10_DailyUploadProdData (ELA default true), [RMS] ERMS Enable, and the
// whole [Murata Function] section (N23-1 / N23-3 are registered for this fake customer)

// ---- the fake Handler -----------------------------------------------------------------------------------------------
std::size_t g_n = 0;
const simnet::Row* g_rows = 0;

struct Fake
{
    int resolveAs;          // what Resolve answers: 1 mask / 0 not masked / -1 not ready
    bool isInt, saveCopy;
    bool forced;            // customer-forced (bReadFromFile == false): read = DefaultValue 1, never saved (W58 Q3)
    bool bField;
    int iField;
    int proxy;              // the page proxy: Checked (0 / 1) or ItemIndex
    std::string section, key;
    Fake() : resolveAs(0), isInt(false), saveCopy(false), forced(false), bField(false), iField(0), proxy(0) {}
};
std::vector<Fake> g_fake;

int IndexOf(const simnet::Row& r)
{
    for (std::size_t i = 0; i < g_n; ++i)
        if (std::strcmp(g_rows[i].code, r.code) == 0 && std::strcmp(g_rows[i].section, r.section) == 0 &&
            std::strcmp(g_rows[i].key, r.key) == 0)
            return (int)i;
    return -1;
}

int ByCode(const char* code)
{
    for (std::size_t i = 0; i < g_n; ++i)
        if (std::strcmp(g_rows[i].code, code) == 0) return (int)i;
    std::printf("  (no row %s)\n", code);
    return 0;
}

class FakeEnv : public simnet::Env
{
public:
    bool ready;
    int logs;
    FakeEnv() : ready(true), logs(0) {}
    std::string ConfigIniPath() { return g_cfg; }
    int Resolve(const simnet::Row& row, simnet::Binding* b)
    {
        if (!ready) return -1;
        const int i = IndexOf(row);
        if (i < 0) return 0;
        Fake& f = g_fake[(std::size_t)i];
        if (f.resolveAs != 1) return f.resolveAs;
        b->section = f.section;
        b->key = f.key;
        if (f.isInt) b->iField = &f.iField;
        else b->bField = &f.bField;
        b->onlyOneIsOn = !f.saveCopy;
        b->forced = f.forced;
        b->control = &f;
        return 1;
    }
    void SetProxy(const simnet::Row& row, const simnet::Binding&, int value)
    {
        const int i = IndexOf(row);
        if (i >= 0) g_fake[(std::size_t)i].proxy = value;
    }
    int Choice(const simnet::Row& row, const simnet::Binding&)
    {
        const int i = IndexOf(row);
        if (i < 0) return 0;
        const Fake& f = g_fake[(std::size_t)i];
        return f.saveCopy ? (f.bField ? 1 : 0) : f.proxy;
    }
    void Log(const std::string& line)
    {
        ++logs;
        std::printf("    [log] %s\n", line.c_str());
    }
};

// golden ReadLastSetIni: ProcessLastSetIni_* (CheckAndReadIniData -> ReadBool) + elConfig ReadEditTextFromFile
// (ReadInteger == 1) + InitialDataToEdit (proxy = field), then phase 0 (cprod.cpp:3217)
void HandlerRead(simnet::Mask& m)
{
    vclcompat::TIniFile ini(AnsiString(g_cfg.c_str()));
    for (std::size_t i = 0; i < g_fake.size(); ++i)
    {
        Fake& f = g_fake[i];
        if (f.section.empty()) continue;
        if (f.forced) { f.bField = true; f.proxy = 1; continue; }   // HTEditList.cpp:1140: DefaultValue, not the file
        const int v = ini.ReadInteger(AnsiString(f.section.c_str()), AnsiString(f.key.c_str()), 0);
        if (f.isInt) f.iField = v;
        else f.bField = f.saveCopy ? v != 0 : v == 1;
        f.proxy = f.isInt ? f.iField : (f.bField ? 1 : 0);
    }
    m.Phase(0);
}

// the page: ELApplyProxies; for a CheckConfigurationBeforeSave key also the proxy -> field copy
void Page(const char* code, int value)
{
    Fake& f = g_fake[(std::size_t)ByCode(code)];
    f.proxy = value;
    if (f.saveCopy) f.bField = value != 0;
}

// golden SaveLastSetIni between phase 1 (:3263) and phase 2 (:3323): ProcessLastSetIni_FTP / _RMS write the FIELD with
// WriteIniData (TIniFile, in place); HTEditList::SaveEditTextToFile writes the PROXY with a TMemIniFile rewrite and copies
// it into the field; a customer-forced (bReadFromFile == false) entry is not written.  strayKey: one more writer that
// puts "0" on a masked key (proves phase 2 writes the snapshot back whatever the save did).  Then the golden
// LoadConfiguration -> ReadLastSetIni (FileRW/IniConfig.gen.inc:8286).
struct AfterPhase1
{
    bool n25_3, a81, n10_3;
    int n31;
};
AfterPhase1 HandlerSave(simnet::Mask& m, const char* straySection, const char* strayKey)
{
    m.Phase(1);
    AfterPhase1 a;
    a.n25_3 = g_fake[(std::size_t)ByCode("N25-3")].proxy != 0;
    a.a81 = g_fake[(std::size_t)ByCode("A81")].proxy != 0;
    a.n10_3 = g_fake[(std::size_t)ByCode("N10-3")].proxy != 0;
    a.n31 = g_fake[(std::size_t)ByCode("N31-1")].proxy;
    {
        vclcompat::TIniFile ini(AnsiString(g_cfg.c_str()));
        for (std::size_t i = 0; i < g_fake.size(); ++i)
        {
            const Fake& f = g_fake[i];
            if (f.saveCopy && f.resolveAs == 1)
                ini.WriteBool(AnsiString(f.section.c_str()), AnsiString(f.key.c_str()), f.bField);
        }
    }
    {
        vclcompat::TMemIniFile mem(AnsiString(g_cfg.c_str()));
        for (std::size_t i = 0; i < g_fake.size(); ++i)
        {
            Fake& f = g_fake[i];
            if (f.saveCopy || f.resolveAs != 1 || f.forced) continue;   // HTEditList.cpp:718: a forced entry is not saved
            mem.WriteInteger(AnsiString(f.section.c_str()), AnsiString(f.key.c_str()), f.proxy);
            if (f.isInt) f.iField = f.proxy;
            else f.bField = f.proxy != 0;
        }
        mem.UpdateFile();
    }
    if (strayKey)
    {
        vclcompat::TIniFile ini(AnsiString(g_cfg.c_str()));
        ini.WriteString(AnsiString(straySection), AnsiString(strayKey), AnsiString("0"));
    }
    m.Phase(2);
    HandlerRead(m);
    return a;
}

simnet::Mask* g_m = 0;
bool TestEla(const char* file, const char* section, const char* key, bool fileValue)
{
    return g_m ? g_m->ElaValue(file, section, key, fileValue) : fileValue;
}

bool Ela(const char* section, const char* key, bool def)
{
    return ela::IniCheckAndReadBool(g_cfg, section, key, def, false, 0);
}

// ---- the SIM gate probes --------------------------------------------------------------------------------------------
int g_setterCalls = 0;
simnet::ElaBoolFn g_setterGot = 0;
void ProbePhase(int) {}
bool ProbeEla(const char*, const char*, const char*, bool v) { return v; }
void ProbeSetter(simnet::ElaBoolFn fn)
{
    ++g_setterCalls;
    g_setterGot = fn;
}

// one exact replacement in a copy of the fixture (false when `from` is not there exactly once)
bool Sub(std::string* s, const char* from, const char* to)
{
    const std::size_t p = s->find(from);
    if (p == std::string::npos || s->find(from, p + 1) != std::string::npos) return false;
    s->replace(p, std::strlen(from), to);
    return true;
}

bool HasRow(const char* section, const char* key)
{
    for (std::size_t i = 0; i < g_n; ++i)
        if (std::strcmp(g_rows[i].section, section) == 0 && std::strcmp(g_rows[i].key, key) == 0) return true;
    return false;
}

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef W906_NO_SOFT_SIMULTE
    const char* const build = "SHIP";
#else
    const char* const build = "SIM";
#endif
    std::printf("SimNet_Mask (W36-1 / W58, %s build)\n", build);

    // ---- 0. containment first -----------------------------------------------------------------------------------------
    {
        char tmp[MAX_PATH];
        const DWORD n = ::GetTempPathA(sizeof(tmp), tmp);
        char stamp[32];
        std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
        g_root = (n > 0 && n < sizeof(tmp)) ? std::string(tmp) + "ht9045_simnet_" + stamp : std::string();
        g_cfg = g_root + "\\config.ini";
        g_gen = g_root + "\\Gerneral.ini";
        std::printf("  sandbox = %s\n", g_root.c_str());
        if (g_root.empty() || UnderMachineTree(g_root) || UnderMachineTree(g_cfg) || UnderMachineTree(g_gen))
        {
            std::printf("  ABORT: the sandbox is not under %%TEMP%% outside the machine trees -- nothing was written\n");
            return 2;
        }
        if (!ela::IniBoolOverride("x\\config.ini", "ChipMos Function", "bN25_3_EnableULJamLog", true))
        {
            std::printf("  ABORT: an ELA override is already installed -- nothing was written\n");
            return 2;
        }
    }
    const char* const machine[] = { "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\system\\Gerneral.ini" };
    Stamp before[2];
    for (int i = 0; i < 2; ++i) before[i] = StampOf(machine[i]);
    ::CreateDirectoryA(g_root.c_str(), 0);
    Put(g_cfg, kFixture);
    Put(g_gen, "[System]\r\nCUSTOMER_CODE=943\r\n[ChipMos Function]\r\nbN25_3_EnableULJamLog=1\r\n");
    const std::string bytes0 = Bytes(g_cfg);
    CHECK(bytes0 == kFixture, "0. the sandbox config.ini is the fixture");

    // ---- 1. the table (W58 Q2) -----------------------------------------------------------------------------------------
    std::printf("1. the key table\n");
    g_rows = simnet::DefaultRows(&g_n);
    {   // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #14 -- no pinned row count (it moved twice): every row's item is in
        //   the design note's tables A / B / C (docs/SIMNET_W58.md "The key table"), every design item has a row, no row twice,
        //   and none of the items the note keeps out.  A row count change that follows the note no longer needs this test edited twice.
        static const char* const kDesign[] = {
            // A. FTP / net-drive (the prototype's rows + W58's N14-8 / N14-9)
            "N06", "N06-1", "N10-1", "N10-2", "N10-3", "N10-9", "N10-11", "N10-12", "N40-1", "N12", "N14-3", "N14-12", "N14-13",
            "N14-19", "N17-1", "N21-1", "N22", "N23", "N35", "N22-1", "N23-1", "N23-3", "N25-2", "N25-3", "N25-4", "N25-5", "N26-1",
            "N27-1", "N30-1", "N31-1", "N32-1", "N33", "N35-1", "O06", "A32", "A55", "N14-8", "N14-9",
            // B. network links (N07-1 out, Jimmy RULINGS_20261001 #3)
            "A81", "N05", "N05-1", "N25-1", "N24",
            // C. "in doubt", W58 Q2 (N07-2 out, same ruling)
            "N07-3", "N07-3-2", "N07-4", "N07-5", "N07-6", "N07-6-1", "N07-7", "N13", "N09-1", "N14-4", "N14-10", "N14-22", "N15-1", "N41-1" };
        static const char* const kOut[] = { "N07-1", "N07-2",                                   // RULINGS_20261001 #3 / W63 = A
            "A75", "N28", "N08-1", "N09-2", "N09-4", "N14-11", "N14-21", "N14-23", "N15-2", "N16", "B11", "N17-3", "N14-18",   // "C left out"
            "N26-2", "N32-2", "N32-3", "N32-4", "N33-1" };                                       // sub-options of a masked parent
        const std::size_t nDesign = sizeof(kDesign) / sizeof(kDesign[0]), nOut = sizeof(kOut) / sizeof(kOut[0]);
        bool inDesign = g_n > 0, noOut = true, unique = true, covered = true;
        for (std::size_t i = 0; i < g_n; ++i) {
            bool found = false;
            for (std::size_t k = 0; k < nDesign; ++k) found = found || std::strcmp(g_rows[i].code, kDesign[k]) == 0;
            if (!found) { inDesign = false; std::printf("  row %s [%s] %s is not in the design tables\n", g_rows[i].code, g_rows[i].section, g_rows[i].key); }
            for (std::size_t k = 0; k < nOut; ++k)
                if (std::strcmp(g_rows[i].code, kOut[k]) == 0) { noOut = false; std::printf("  row %s is one the note keeps out\n", g_rows[i].code); }
            for (std::size_t j = i + 1; j < g_n; ++j)
                if (std::strcmp(g_rows[i].code, g_rows[j].code) == 0 && std::strcmp(g_rows[i].section, g_rows[j].section) == 0 &&
                    std::strcmp(g_rows[i].key, g_rows[j].key) == 0) { unique = false; std::printf("  row %s [%s] %s twice\n", g_rows[i].code, g_rows[i].section, g_rows[i].key); }
        }
        for (std::size_t k = 0; k < nDesign; ++k) {
            bool has = false;
            for (std::size_t i = 0; i < g_n; ++i) has = has || std::strcmp(g_rows[i].code, kDesign[k]) == 0;
            if (!has) { covered = false; std::printf("  design item %s has no row\n", kDesign[k]); }
        }
        std::printf("  (%u rows, %u design items)\n", (unsigned)g_n, (unsigned)nDesign);
        CHECK(inDesign && noOut && unique && covered,
              "1. every row is a design-table A / B / C item (docs/SIMNET_W58.md), every item has a row, no row twice, none kept out");
    }
    CHECK(!HasRow("SECS GEM", "Enable SECS GEM") && !HasRow("SECS GEM", "Enable RCMD START"),
          "1. Jimmy RULINGS_20261001 #3: N07-1 Enable SECS GEM and N07-2 Enable RCMD START are not rows");
    CHECK(HasRow("FTPUpLoad", "bEnable_FTPUpLoadLog") && HasRow("FTPUpLoad", "bN10_UploadSummaryToFTP") &&
              HasRow("FTPUpLoad", "bN10_DailyUploadProdData") && HasRow("FTP", "Enable FTP") &&
              HasRow("FTP", "Enable FTP Password Download") && HasRow("FTP", "Second Enable FTP"),
          "1. A: N06 / N06-1 / N10-1..3 / N12 in");
    CHECK(HasRow("ChipMos Function", "bN25_2_EnableUploadLog") && HasRow("ChipMos Function", "bN25_3_EnableULJamLog") &&
              HasRow("ChipMos Function", "bN25_4_EnableUpload") && HasRow("ChipMos Function", "bN25_5_EnableUpload") &&
              HasRow("Event Log", "bAlarmStatistAutoSaveNetDrive") && HasRow("Function", "bA32EnableFTPAutomation") &&
              HasRow("Function", "bA55EnablePMAlarmUpdateFromServerbyFTP") && HasRow("bAutoTmpeOfsByFTP", "bN31_UseAutoTempOfsByFTP"),
          "1. A: N25-2..5 / O06 net drive / A32 / A55 / N31 in");
    CHECK(HasRow("SECS GEM", "Wait SECS") &&
              HasRow("ChipMos Function", "bN25_1_EnableStartControl") && HasRow("RTM Function", "bN24_EnableRTM") &&
              HasRow("", "") && HasRow("", "ERMS Enable"),
          "1. B: A81 / N25-1 / N24 / N05 RMS / N05-1 ERMS in");
    CHECK(HasRow("ARMS", "bN13_EnableARMSFunction") && HasRow("Handler_OEE", "N14_HandlerOEEAutoLoadMOFile") &&
              HasRow("FTPUpLoad", "bN41_1_HandlerDataBackUpToDiskUseFunction") && HasRow("SECS GEM", "SECS GEM OneCycle") &&
              HasRow("Specific", "N07_SecsLotCheck") && HasRow("Automation", "bN09_LotCountAutoFunc") &&
              HasRow("ESD_Control", "N15_ESDControlUserLevelByTxt") && HasRow("Handler_OEE", "bN14_8_ULSetup"),
          "1. C (W58 Q2 = 要) in: N13 / N14-4 / N41-1 / N07-3 / N07-4 / N09-1 / N15-1, and N14-8 (an FTP upload)");
    CHECK(!HasRow("Production_Log", "bN17UploadProdLog") && !HasRow("JSCK Function", "bN26_SCK_OEE") &&
              !HasRow("Server", "Enable Check File") && !HasRow("RMS", "Enable Check File") &&
              !HasRow("PrecautionRecord", "bB11UsePATServerFile") && !HasRow("Function", "bA75DownloadItemByAccessLevel") &&
              !HasRow("Automation", "iN09_4_UploadMethod") && !HasRow("Handler_OEE", "bN14_21_SetUpConfiguration") &&
              !HasRow("N16", "bEnableOffsetFTP"),
          "1. C without a network effect not in: N17-3 / N28 / Enable Check File / B11 / A75 / N09-4 / N14-21 / N16");
    CHECK(!HasRow("Socket", "bEnableSocketCommunication") && !HasRow("NETWORK", "bN20_CheckMD5") &&
              !HasRow("JamRawDataUpdataToFTP", "bN26_UseJamRawDataRecord") && !HasRow("Visible", "bG24DisableSECSGEMStatus") &&
              !HasRow("FTPUpLoad", "bN10FtpPassive"),
          "1. D (out) and sub-options not in: Socket / N20 MD5 / N26-2 / G24 / N10 passive");
    {
        bool dup = false;
        for (std::size_t i = 0; i < g_n; ++i)
            for (std::size_t j = i + 1; j < g_n; ++j)
                if (g_rows[i].section[0] && std::strcmp(g_rows[i].section, g_rows[j].section) == 0 &&
                    std::strcmp(g_rows[i].key, g_rows[j].key) == 0)
                    dup = true;
        bool kinds = true;
        for (std::size_t i = 0; i < g_n; ++i)
            if ((g_rows[i].kind == simnet::kSaveCopy) != (g_rows[i].proxy != 0)) kinds = false;
        CHECK(!dup, "1. no (section, key) twice");
        CHECK(kinds, "1. every kSaveCopy row names its proxy, no kEditList row does");
    }

    // ---- 2. the raw ini helpers ---------------------------------------------------------------------------------------
    std::printf("2. raw ini helpers\n");
    {
        const std::string p = g_root + "\\raw.ini";
        Put(p, "[A]\nk1 = v one \n;k2=comment\nK2=two\n[a]\nk1=second section\n[B]\r\nx=1\r\n");
        std::string v;
        bool sec = false;
        CHECK(simnet::ReadRawValue(p, " a ", "K1", &v, &sec) && v == " v one " && sec, "2. verbatim value, case-insensitive, trimmed names");
        CHECK(simnet::ReadRawValue(p, "A", "k2", &v) && v == "two", "2. a ';' line is a comment");
        CHECK(!simnet::ReadRawValue(p, "A", "nope", &v, &sec) && sec, "2. missing key: false, the section found");
        CHECK(!simnet::ReadRawValue(p, "C", "x", &v, &sec) && !sec, "2. missing section");
        simnet::WriteRawValue(p, "A", "k1", "1");
        CHECK(Bytes(p) == "[A]\nk1 =1\n;k2=comment\nK2=two\n[a]\nk1=second section\n[B]\r\nx=1\r\n",
              "2. WriteRawValue: in place, up to '=' kept, first section only, LF lines untouched");
        CHECK(simnet::RemoveKey(p, "A", "K2", false) &&
                  Bytes(p) == "[A]\nk1 =1\n;k2=comment\n[a]\nk1=second section\n[B]\r\nx=1\r\n",
              "2. RemoveKey: that one line");
        CHECK(simnet::RemoveKey(p, "B", "x", true) && Bytes(p) == "[A]\nk1 =1\n;k2=comment\n[a]\nk1=second section\n",
              "2. RemoveKey(dropEmptySection): the header goes too when nothing is left");
        CHECK(!simnet::RemoveKey(p, "B", "x", true), "2. RemoveKey of a missing key: false");
        ::DeleteFileA(p.c_str());
    }

    // ---- 2b. W58 Q3 decisions on the real registrations (E2 W58 m5) ------------------------------------------------
    //   AI(W906-W58) 20260930 (St02-E): simnet::Q3Decide(readFromFile, enabled, visible, valueOn) -- what HandlerEnv::Resolve does to
    //   the entry; the real THTEdit / page part (enable, show, the save's drop rule) stays a Human review item.
    //   The flags below are copied by hand from FileRW/IniConfig.gen.inc :6357 / :6220 / :6324 (E2 delta n1): a
    //   regenerated gen.inc has to be re-checked by hand.
    std::printf("2b. Q3 decisions\n");
    {
        const simnet::Q3Plan kyec = simnet::Q3Decide(false, false, true, true);    // KYEC_LEE N10-3: bShow, bDisable, bFixedValue, 1 (gen.inc:6357)
        CHECK(kyec.mask && kyec.forced && !kyec.show, "2b. KYEC_LEE N10-3 (shown, greyed, forced on): masked, forced, not re-shown");
        // KYEC_LEE N10-1 (gen.inc:6347) has the same flags as N10-3 above -- covered by that case (E2 delta n3)
        const simnet::Q3Plan vtest = simnet::Q3Decide(false, false, false, true);  // VTEST N10-3: bNoShow, bDisable, bFixedValue, 1 (gen.inc:6324)
        CHECK(vtest.mask && vtest.forced && vtest.show, "2b. VTEST N10-3 (hidden, forced on): masked AND shown in SIM (E2 m1)");
        const simnet::Q3Plan none = simnet::Q3Decide(false, false, false, false);  // the "no such function" fallback: bNoShow, bDisable, bFixedValue, 0
        CHECK(!none.mask && !none.forced && !none.show, "2b. hidden and off (no such function): not masked, control untouched");
        const simnet::Q3Plan plain = simnet::Q3Decide(true, true, true, false);     // an ordinary key, file 0
        const simnet::Q3Plan plainOn = simnet::Q3Decide(true, true, true, true);    // an ordinary key, file 1
        CHECK(plain.mask && !plain.forced && !plain.show && plainOn.mask && !plainOn.forced && !plainOn.show,
              "2b. an ordinary shown key: masked (off or on), not forced, not re-shown");
        const simnet::Q3Plan hiddenFile = simnet::Q3Decide(true, true, false, true);
        CHECK(hiddenFile.mask && !hiddenFile.forced && hiddenFile.show, "2b. hidden, read from the file, on: masked and shown, not forced");
    }

    // ---- the fake Handler for this "customer" -------------------------------------------------------------------------
    g_fake.assign(g_n, Fake());
    for (std::size_t i = 0; i < g_n; ++i)
    {
        Fake& f = g_fake[i];
        f.section = g_rows[i].section;
        f.key = g_rows[i].key;
        f.saveCopy = g_rows[i].kind == simnet::kSaveCopy;
        f.isInt = std::strcmp(g_rows[i].code, "N31-1") == 0;
    }
    g_fake[(std::size_t)ByCode("N05")].section = "RMS";           // CUSTOMER_CODE 943 (CC_SCC): cprod.cpp:2196
    g_fake[(std::size_t)ByCode("N05")].key = "RMS Enable";
    g_fake[(std::size_t)ByCode("N05-1")].section = "RMS";
    const char* const masked[] = { "N10-1", "N10-2", "N10-3", "N25-1", "N25-3", "N25-4", "N25-5", "O06", "A81",
                                   "N06", "N06-1", "N05", "N05-1", "N31-1", "N23-1", "N23-3" };
    const int kMasked = (int)(sizeof(masked) / sizeof(masked[0]));
    for (int i = 0; i < kMasked; ++i) g_fake[(std::size_t)ByCode(masked[i])].resolveAs = 1;
    g_fake[(std::size_t)ByCode("N25-4")].forced = true;   // golden registers it bFixedValue for this customer; W58 Q3 = 關:
    // masked too.  Every other row: not an elConfig key for this customer

    FakeEnv env;
    simnet::Mask mask;
    mask.Attach(g_rows, g_n, &env);
    g_m = &mask;
    W906_ElaSetBoolOverride(&TestEla);

    // ---- 3. launch -----------------------------------------------------------------------------------------------------
    std::printf("3. launch (phase 0)\n");
    HandlerRead(mask);
    {
        bool allOff = true;
        for (int i = 0; i < kMasked; ++i)
        {
            const Fake& f = g_fake[(std::size_t)ByCode(masked[i])];
            if ((f.isInt ? f.iField : (int)f.bField) != 0 || f.proxy != 0) allOff = false;
        }
        CHECK(allOff, "3. every armed key: field false / 0 and proxy unchecked (file had N10-1 / N25-1 / N25-3 / N25-4 / O06 / A81 / N06 / N05 = 1, N31 = 2)");
        CHECK(mask.CountIn(simnet::kArmed) == kMasked && mask.CountIn(simnet::kSkipped) == (int)g_n - kMasked &&
                  mask.CountIn(simnet::kPending) == 0 && mask.CountIn(simnet::kReleased) == 0,
              "3. the 16 masked keys armed, every other row not masked, none pending / released");
        const std::size_t i254 = (std::size_t)ByCode("N25-4");
        CHECK(!g_fake[i254].bField && g_fake[i254].proxy == 0 && mask.State(i254) == simnet::kArmed &&
                  mask.BindingAt(i254).forced,
              "3. customer-forced N25-4 (Q3 = 關): masked too, the binding says forced");
        CHECK(Bytes(g_cfg) == bytes0, "3. phase 0 writes nothing (config.ini byte-identical)");
        const simnet::Binding b = mask.BindingAt((std::size_t)ByCode("N05"));
        CHECK(b.section == "RMS" && b.key == "RMS Enable" && b.bField != 0 && !b.onlyOneIsOn,
              "3. the RMS row resolved to [RMS] RMS Enable (the Env picks the section), ReadBool rule");
    }

    // ---- 4. the ELA while armed ---------------------------------------------------------------------------------------
    std::printf("4. the ELA while armed\n");
    {
        CHECK(!Ela("ChipMos Function", "bN25_3_EnableULJamLog", false), "4. IniCheckAndReadBool N25-3: armed -> false (file 1)");
        CHECK(!Ela("Event Log", "bAlarmStatistAutoSaveNetDrive", false), "4. O06 net drive: armed -> false (file 1)");
        CHECK(!Ela("FTPUpLoad", "bN10_DailyUploadProdData", true), "4. N10-3 missing, default true: armed -> false (ElaHub.cpp:77 path)");
        CHECK(!Ela("ChipMos Function", "bN25_4_EnableUpload", true), "4. forced N25-4: armed -> false (file 1)");
        CHECK(Ela("Event Log", "EnableAutoSaveEventLog", false), "4. a key outside the table -> the file value 1");
        CHECK(Ela("SECS GEM", "Enable SECS GEM", false) && Ela("SECS GEM", "Enable RCMD START", false),
              "4. Jimmy RULINGS_20261001 #3: N07-1 / N07-2 are not masked -- in SIM they read their config.ini value (1)");
        CHECK(Ela("chipmos function", "BN25_3_ENABLEULJAMLOG", true) == false, "4. names match case-insensitively (Win32 ini)");
        CHECK(ela::IniCheckAndReadBool(g_gen, "ChipMos Function", "bN25_3_EnableULJamLog", false, false, 0),
              "4. the same key in another file (Gerneral.ini) -> the file value");
        ela::ScheduleConfig c;
        ela::ReadScheduleConfig(g_cfg, g_gen, "SIMNETTEST", &c);
        CHECK(!c.n25_3 && !c.o06UseNetDrive && !c.n10Daily, "4. ReadScheduleConfig (ElaSchedule.cpp:273): n25_3 / o06UseNetDrive / n10Daily false");
        CHECK(!c.n25_4 && c.o06_1 && c.n17, "4. ReadScheduleConfig: n25_4 (forced, masked) false; o06_1 / n17 (not in the table) = file 1");
        CHECK(Bytes(g_cfg) == bytes0, "4. the ELA reads wrote nothing");
    }

    // ---- 5. a save with nothing released ------------------------------------------------------------------------------
    std::printf("5. save, nothing released\n");
    {
        const AfterPhase1 a = HandlerSave(mask, "SECS GEM", "Wait SECS");
        CHECK(a.n25_3 && a.a81 && !a.n10_3 && a.n31 == 2, "5. phase 1 unmasks: the save sees the file's values (N25-3 1, A81 1, N10-3 missing = 0, N31 2)");
        CHECK(Bytes(g_cfg) == bytes0, "5. config.ini byte-identical after the save (stray write undone, missing key / [Murata Function] removed)");
        CHECK(mask.CountIn(simnet::kArmed) == kMasked && mask.CountIn(simnet::kReleased) == 0, "5. still all armed");
        CHECK(!g_fake[(std::size_t)ByCode("N25-3")].bField && g_fake[(std::size_t)ByCode("N25-3")].proxy == 0 &&
                  g_fake[(std::size_t)ByCode("N31-1")].iField == 0,
              "5. after the save: fields / proxies off again");
        CHECK(!Ela("SECS GEM", "Wait SECS", false) && Ela("SECS GEM", "Enable SECS GEM", false),
              "5. the ELA still reads A81 off, and N07-1 (not masked) still reads its file value");
    }

    // ---- 6. released = on, and written to config.ini (Q1 = 要) -----------------------------------------------------------
    std::printf("6. release (checked + saved)\n");
    std::string bytes6 = bytes0;
    {
        Page("N25-5", 1);     // file 0
        Page("N06-1", 1);     // file 0 (a CheckConfigurationBeforeSave key)
        Page("N31-1", 1);     // file 2
        Page("N25-4", 1);     // customer-forced: HTEditList never saves it
        HandlerSave(mask, 0, 0);
        const std::size_t i255 = (std::size_t)ByCode("N25-5"), i061 = (std::size_t)ByCode("N06-1"),
                          i31 = (std::size_t)ByCode("N31-1"), i254 = (std::size_t)ByCode("N25-4");
        CHECK(mask.State(i255) == simnet::kReleased && mask.State(i061) == simnet::kReleased &&
                  mask.State(i31) == simnet::kReleased && mask.State(i254) == simnet::kReleased &&
                  mask.CountIn(simnet::kReleased) == 4,
              "6. N25-5 / N06-1 / N31-1 / N25-4 released, nothing else");
        CHECK(g_fake[i255].bField && g_fake[i255].proxy == 1 && g_fake[i061].bField && g_fake[i061].proxy == 1 &&
                  g_fake[i31].iField == 1 && g_fake[i31].proxy == 1 && mask.RunValue(i31) == 1 && g_fake[i254].bField,
              "6. field true / proxy checked (N31 = 1) for this run");
        std::string want = bytes0;
        const bool subs = Sub(&want, "bN25_5_EnableUpload=0", "bN25_5_EnableUpload=1") &&
                          Sub(&want, "Enable FTP Password Download=0", "Enable FTP Password Download=1") &&
                          Sub(&want, "bN31_UseAutoTempOfsByFTP=2", "bN31_UseAutoTempOfsByFTP=1");
        CHECK(subs && Bytes(g_cfg) == want,
              "6. config.ini now has N25-5=1, Enable FTP Password Download=1, N31=1 and nothing else changed (Q1 = 要)");
        CHECK(Ela("ChipMos Function", "bN25_5_EnableUpload", false) && Ela("ChipMos Function", "bN25_4_EnableUpload", false),
              "6. the ELA reads N25-5 and forced N25-4 on");
        ela::ScheduleConfig c;
        ela::ReadScheduleConfig(g_cfg, g_gen, "SIMNETTEST", &c);
        CHECK(c.n25_5 && c.n25_4 && !c.n25_3, "6. ReadScheduleConfig: n25_5 / n25_4 on, n25_3 still off");
        HandlerRead(mask);    // another page open (FormShow -> ReadLastSetIni)
        CHECK(g_fake[i255].bField && g_fake[i255].proxy == 1 && g_fake[i254].bField && g_fake[i254].proxy == 1 &&
                  !g_fake[(std::size_t)ByCode("N25-3")].bField,
              "6. a later page open keeps N25-5 / N25-4 on and N25-3 off");
        Page("N25-5", 0);
        Page("N25-4", 0);
        HandlerSave(mask, 0, 0);
        CHECK(mask.State(i255) == simnet::kArmed && !g_fake[i255].bField && !Ela("ChipMos Function", "bN25_5_EnableUpload", true) &&
                  mask.State(i254) == simnet::kArmed && !g_fake[i254].bField,
              "6. unchecked + saved -> armed (off) again, forced N25-4 too");
        CHECK(Sub(&want, "bN25_5_EnableUpload=1", "bN25_5_EnableUpload=0") && Bytes(g_cfg) == want,
              "6. and the file has N25-5=0 again (the operator's choice); N06-1 / N31 stay 1; forced N25-4 never written (1)");
        bytes6 = want;
    }

    // ---- 7. a new launch -----------------------------------------------------------------------------------------------
    std::printf("7. a new launch\n");
    {
        mask.Arm();
        HandlerRead(mask);
        const std::size_t i061 = (std::size_t)ByCode("N06-1"), i31 = (std::size_t)ByCode("N31-1");
        CHECK(mask.CountIn(simnet::kArmed) == kMasked && mask.CountIn(simnet::kReleased) == 0, "7. re-armed: all armed, none released");
        CHECK(!g_fake[i061].bField && g_fake[i061].proxy == 0 && g_fake[i31].iField == 0 && g_fake[i31].proxy == 0,
              "7. N06-1 / N31 off again although the file says 1 / 1 (W36-1: every SIM launch starts off)");
        CHECK(Bytes(g_cfg) == bytes6, "7. phase 0 wrote nothing");
        HandlerSave(mask, 0, 0);
        CHECK(Bytes(g_cfg) == bytes6, "7. a save with nothing ticked keeps the file's 1 / 1 (a later SHIP launch follows the tick)");
    }

    // ---- 8. not ready --------------------------------------------------------------------------------------------------
    std::printf("8. not ready (edit lists not registered)\n");
    {
        FakeEnv env2;
        env2.ready = false;
        simnet::Mask m2;
        m2.Attach(g_rows, g_n, &env2);
        const std::size_t i253 = (std::size_t)ByCode("N25-3");
        g_fake[i253].bField = true;
        g_fake[i253].proxy = 1;
        m2.Phase(0);
        CHECK(m2.CountIn(simnet::kPending) == (int)g_n && g_fake[i253].bField && g_fake[i253].proxy == 1,
              "8. every row pending, no field touched");
        CHECK(!m2.ElaValue(g_cfg.c_str(), "ChipMos Function", "bN25_3_EnableULJamLog", true),
              "8. the ELA reads a pending key off (fail safe)");
        m2.Phase(1);
        m2.Phase(2);
        CHECK(Bytes(g_cfg) == bytes6 && g_fake[i253].bField, "8. phases 1 / 2 on pending rows: no write, no field change");
        env2.ready = true;
        m2.Phase(0);
        CHECK(m2.CountIn(simnet::kArmed) == kMasked && !g_fake[i253].bField, "8. resolved at the next phase 0: armed, off");
        mask.Phase(0);   // the main mask again (fields shared with the fake)
    }

    // ---- 9. the SIM gate -----------------------------------------------------------------------------------------------
    std::printf("9. the SIM gate\n");
    {
#ifdef W906_NO_SOFT_SIMULTE
        const bool expectSim = false;
#else
        const bool expectSim = true;
#endif
        CHECK(simnet::SimBuild() == expectSim, "9. SimBuild() follows MachineType.h SOFT_SIMULTE (= !W906_NO_SOFT_SIMULTE)");
        void (*slot)(int) = 0;
        const bool r = simnet::InstallHooks(&slot, &ProbePhase, &ProbeSetter, &ProbeEla);
        if (expectSim)
            CHECK(r && slot == &ProbePhase && g_setterCalls == 1 && g_setterGot == &ProbeEla,
                  "9. SIM: the phase slot and the ELA override are installed");
        else
            CHECK(!r && slot == 0 && g_setterCalls == 0, "9. SHIP: nothing installed, the slot stays NULL");
    }

    // ---- 10. the end ---------------------------------------------------------------------------------------------------
    std::printf("10. the end\n");
    W906_ElaSetBoolOverride(0);
    g_m = 0;
    CHECK(Ela("ChipMos Function", "bN25_3_EnableULJamLog", false), "10. override removed: the ELA reads the file (1) again");
    CHECK(env.logs > 0, "10. the mask logged its phases");
    for (int i = 0; i < 2; ++i)
    {
        char m[160];
        std::snprintf(m, sizeof(m), "10. the machine's %s is unchanged (read-only stat)", machine[i]);
        CHECK(SameStamp(before[i], StampOf(machine[i])), m);
    }

    std::printf("SimNet_Mask: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else std::printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
