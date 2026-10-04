// =============================================================================
//  test_ela_hub.cpp -- ELA plan P2 / P3: ElaHub (EL_* commands, the worker, the query snapshot).
//
//  AI(W906-ELA-P2) 20260927.  Suite name (add_test): ELA_Hub
//
//    1. idle hub: RunOnce() has nothing to do;
//    2. golden AnalysisThreadProcess order and coalescing: UPDATE_PARAMETER before UPLOAD_JAMWEEK, a repeated post runs
//       once, an out-of-range command is ignored; ReadConfig reads config.ini / Gerneral.ini and writes the 38 missing
//       config.ini keys back (#17 A); UPLOAD_JAMWEEK is skipped by its configuration gate (R4, #22 D-f: O10 off here);
//    3. a query through RunOnce(): the JSON snapshot carries the summary, the grids, the config and the history
//       (AI(W906-ELA-W48) 20260928: and ★W48's "oee" days);
//    4. the same query through the worker thread (Start / Stop);
//    5. ToUtf8: cp950 -> UTF-8, UTF-8 unchanged;
//    6. jamWriteBack=false: ReadConfig writes nothing.
//  Every file is under %TEMP%\ht9045_ela_hub -- never D:\HT9045\config, D:\HT9045\system or D:\HT9045_Log.
// =============================================================================
#include "EventLogAnalysis/ElaHub.h"
#include "EventLogAnalysis/ElaFtp.h"   // ElaLogRoot (the refuse-first check)
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cctype>                  // AI(W906-ELA-REV): MachineDataPath
#include <cstring>
#include <string>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static void WriteFile(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

// same guard as tests/test_ela_ftp.cpp:54-58 (MinGW.org 6.3 strict mode declares neither putenv nor _putenv).
// MUST stay a CRT call: getenv() reads the CRT's copy, which SetEnvironmentVariableA does not update.
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#endif

// AI(W906-ELA-REV) 20260928 (St02-E): refuse-first -- no root or seam of this test may resolve into the machine's data
static bool MachineDataPath(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) s[i] = (s[i] == '/') ? '\\' : (char)std::tolower((unsigned char)s[i]);
    static const char* const k[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary" };   // d:\ht9045 also covers d:\ht9045_log
    for (size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
        if (s.compare(0, std::strlen(k[i]), k[i]) == 0) return true;
    return false;
}

int main()
{
    printf("ELA_Hub\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    const std::string root = std::string(tmp) + W906_TestTmpName("ht9045_ela_hub");   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h) (W906_HT9045LOG_ROOT below follows it)
    if (MachineDataPath(root))                                          // before anything is written
    {
        printf("  REFUSED: sandbox %s is under the machine's data (D:\\HT9045*, D:\\RMS, D:\\MTBF_Summary)\n", root.c_str());
        return 1;
    }
    _putenv(("W906_HT9045LOG_ROOT=" + root).c_str());                  // the log root the Hub's upload jobs would use
    if (MachineDataPath(ela::ElaLogRoot()))
    {
        printf("  REFUSED: W906_HT9045LOG_ROOT did not take (%s)\n", ela::ElaLogRoot().c_str());
        return 1;
    }
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\EventLogTxt").c_str(), 0);
    ::CreateDirectoryA((root + "\\EventLogTxt\\2026").c_str(), 0);
    ::CreateDirectoryA((root + "\\EventLogTxt\\2026\\04").c_str(), 0);
    ::CreateDirectoryA((root + "\\Production_Log").c_str(), 0);
    WriteFile(root + "\\EventLogTxt\\2026\\04\\EventLogTxt_20260401.csv",
              "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n"
              "2026/04/01,08:10:00,\"01 Input Arm\",JAM0101,1,30,0,\"Pick \"\"A\"\" fail\",,R1\r\n"
              "2026/04/01,08:20:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n");
    const std::string jamIni = root + "\\JAM0000.dat";
    WriteFile(jamIni, "[01 Input Arm]\r\n");
    const std::string cfg = root + "\\config.ini";
    ::DeleteFileA(cfg.c_str());
    WriteFile(root + "\\Gerneral.ini", "[System]\r\nCUSTOMER_CODE=910\r\n[Version]\r\nMachine ID=M1\r\n");

    ela::Options o;
    o.jamIniPath = jamIni;
    ela::HubPaths hp;
    hp.configIni = cfg;
    hp.generalIni = root + "\\Gerneral.ini";
    hp.summaryDir = root + "\\RMS\\";   // AI(W906-ELA-REV) 20260928: not used here; pinned so no path of hp is the machine's
    hp.timeDataDir = root + "\\TimeData";   // AI(W906-ELA-W48B) 20260928: ★W48-2's TimeData folder, pinned too (absent: no off)

    {
        ela::Hub h(o, hp);
        // 1
        CHECK(!h.RunOnce() && h.Seq() == 0, "1. idle hub does nothing");

        // 2
        h.Post(ela::EL_UPLOAD_JAMWEEK);
        h.Post(ela::EL_UPDATE_PARAMETER);
        h.Post(ela::EL_UPDATE_PARAMETER);
        h.Post(99);
        CHECK(h.RunOnce(), "2. first pass runs a job");
        const ela::ElaConfig c = h.Config();
        CHECK(c.sCustCode == "910" && c.edN04_ID == "M1" && c.edO06_FilePath == "D:\\RMS" && c.cbN10_3 == true &&
                  c.edtN10_8 == "D:\\RMS", "2. UPDATE_PARAMETER first: ReadConfig values (golden defaults, Gerneral.ini)");
        CHECK(h.ConfigWrites() == 38, "2. the 38 missing config.ini keys written back (#17 A)");
        CHECK(h.RunOnce(), "2. second pass runs the next job");
        unsigned long seq = 0;
        std::string js = h.SnapshotJson(&seq);
        CHECK(Has(js, "EL_UPLOAD_JAMWEEK: skipped, O10"), "2. JAMWEEK accepted, skipped by the D-f gate (R4)");   // AI(W906-ELA-R4) 20260927
        CHECK(Has(js, "\"jobs\":[{\"at\":") && Has(js, "\"cmd\":\"EL_UPDATE_PARAMETER\",\"ran\":true") &&
                  Has(js, "\"cmd\":\"EL_UPLOAD_JAMWEEK\",\"ran\":false") && h.Jobs().size() == 2,
              "2. each EL_* job leaves a job record (AI(W906-ELA-R2): kept apart from the page's snapshot)");
        CHECK(!h.RunOnce(), "2. the repeated UPDATE_PARAMETER was coalesced; 99 ignored");
        CHECK(Has(js, "\"custCode\":\"910\"") && !Has(js, "Password") && Has(js, "\"result\":null"),
              "2. snapshot: config without passwords, no query yet");

        // 3
        ela::QueryRequest q;
        q.startDate = ela::EncodeDate(2026, 4, 1);
        q.startTime = 0.0;
        q.endDate = ela::EncodeDate(2026, 4, 1);
        q.endTime = ela::EncodeTime(23, 59, 59, 0);
        q.eventLogDir = root + "\\EventLogTxt";
        q.prodLogDir = root + "\\Production_Log";
        q.handlerId = "H1";
        h.PostQuery(q);
        CHECK(h.RunOnce(), "3. the query runs when no EL job is raised");
        js = h.SnapshotJson(&seq);
        CHECK(Has(js, "\"result\":{") && Has(js, "Handler ID:        H1") && Has(js, "JAM0101") &&
                  Has(js, "\"filesRead\":[") && Has(js, "Pick \\\"A\\\" fail"),
              "3. snapshot carries the summary, the grids and escaped text");
        CHECK(Has(js, "\"byDay\":[[\"2026/04/01\"") && Has(js, "\"byHour\":[[\"2026/04/01 00:00:00\""),
              "3. by-day / by-hour rows");
        // AI(W906-ELA-W48) 20260928 (St02-E helper): ★W48 the user's query snapshot carries "oee" (ElaOee.h): the one day,
        //   (no Production_Log here: no touchdowns).  The day is long past the real clock, so its span is whole.
        //   AI(W906-ELA-W48B) 20260928: W48-3 = B (Steven 0928) -- down = the MTBA alarms only: JAM0101 in 01 Input Arm
        //   (golden default: counted) 30 s; WAR2401 24 Motor (golden default: not counted) is idle.  Was 30 + 20 s.
        CHECK(Has(js, "\"oee\":{\"rule\":{\"test\":\"touchdownTestTime\"") &&   // AI(W906-ELA-W48B): W48-1
                  Has(js, "\"days\":[{\"date\":\"2026/04/01\",\"spanSec\":86400,\"testSec\":0,\"downSec\":30,"),
              "3. snapshot carries \"oee\" (W48): 2026/04/01, down 30 s (W48-3: the MTBA alarm only)");

        // 4
        const unsigned long before = h.Seq();
        CHECK(h.Start(), "4. worker started");
        h.PostQuery(q);
        for (int i = 0; i < 100 && h.Seq() == before; ++i) ::Sleep(100);
        h.Stop();
        CHECK(h.Seq() > before, "4. the worker ran the query (golden 500 ms loop)");
    }

    // 5
    CHECK(ela::ToUtf8("\xA4\xA4") == "\xE4\xB8\xAD" && ela::ToUtf8("abc") == "abc" &&
              ela::ToUtf8("\xE4\xB8\xAD") == "\xE4\xB8\xAD", "5. cp950 -> UTF-8, UTF-8 unchanged");

    // 6
    {
        const std::string cfg2 = root + "\\config2.ini";
        ::DeleteFileA(cfg2.c_str());
        ela::Options o2;
        o2.jamIniPath = jamIni;
        o2.jamWriteBack = false;
        ela::HubPaths hp2 = hp;
        hp2.configIni = cfg2;
        ela::Hub h2(o2, hp2);
        h2.Post(ela::EL_UPDATE_PARAMETER);
        h2.RunOnce();
        CHECK(h2.ConfigWrites() == 0 && ::GetFileAttributesA(cfg2.c_str()) == INVALID_FILE_ATTRIBUTES,
              "6. jamWriteBack=false: config.ini not created");
    }

    if (g_fail == 0) W906_TestTmpRemoveTree(root);   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h): a green run leaves nothing; a red run keeps the sandbox to look at (like ELA_Oee)
    printf("ELA_Hub: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
