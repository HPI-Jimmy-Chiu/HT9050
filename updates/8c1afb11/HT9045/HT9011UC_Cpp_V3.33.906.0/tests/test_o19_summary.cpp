// =============================================================================
//  test_o19_summary.cpp -- ctest MyDB_O19_Summary.  AI(W906-O19-C4) 20261002 (St02-E helper): card ST02-C4 (todo D-004 (1)).
//
//  O19SummaryReport.cpp = golden 906_0625_Steven TfObserver::DoProduction_Summary_Report (cObserver.cpp:4889-5058) on
//  the REAL fObserver (cObserver.cpp:3289, static init) and its strngrdMDBQuery grid, plus the three O19 helpers.
//    0. containment first (the b12ab375 rule): refuse, exit 2, before ANY Handler code unless the cMyDB log roots
//       (st02_test_containment.h) and W906_PRODLOADER_ROOT / W906_O19_ROOT are ctest's machine_log_scratch; then point
//       both variables, asProduct_LoaderPath and IniConfig.asO19_SavePath at %TEMP%\ht9045_o19_<tick> (refused under
//       D:\HT9045*).  Every file this test makes is there; the sandbox is removed on a green run.
//    1. Production_Loader stamps written with WriteIniData (the writer RecordTimeData uses, cMyDB.cpp:572-573) where the
//       function reads them: <root>\Production_Loader\<yyyy>\<yyyy-mm-dd>-<0800-2000|2000-0800>.txt [Product].
//    2. day report on a pre-dirtied grid: 14 rows (golden bUseMDB==false: "No Record!!" jam sections, real section 3);
//       the cells were worked out from the golden rule by hand, not by calling the port.
//    3. week (8 days, the end date excluded), 4. month across a year, 5. start == end (iDay 1),
//       5b. W906_PRODLOADER_ROOT unset -> the golden global asProduct_LoaderPath.
//    6. SGDToCSV bytes: CRLF (text-mode "\n"), a second call appends, only the FIRST "," in a cell becomes ";".
//    7. W906O19_DayOfWeek / W906O19_HostName (= Winsock gethostname) / W906O19_SaveDir; CONTROL: the old no-op stub must
//       fail the step-2 check.
//  Not covered: the 08:00 path inside RecordTimeData (no clock seam) -- human review.
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h> (gethostname, the Winsock reference)
#include "O19SummaryReport.h"
#include "common.h"                       // WriteIniData, SGDToCSV, asProduct_LoaderPath
#include "Config.h"                       // IniConfig.asO19_SavePath
#include "st02_test_containment.h"        // W906TestInsideCtestRoots
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// _putenv: the tests/test_agv_e84.cpp:158-163 guard (MinGW.org 6.3.0 declares neither _putenv nor putenv under -std=c++17)
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
static void Check(bool c, const char* msg, int line)
{
    if (c) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else   { std::printf("  FAIL: %s  (line %d)\n", msg, line); ++g_fail; }
}
#define CHECK(c, msg) Check((c), (msg), __LINE__)

static std::string Lower(const char* p)
{
    std::string s(p ? p : "");
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

static bool UnderMachineTree(const std::string& p)
{
    return Lower(p.c_str()).compare(0, 9, "d:\\ht9045") == 0;   // D:\HT9045\... and D:\HT9045_Log\...
}

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

static void ListFiles(const std::string& dir, std::vector<std::string>* files, std::vector<std::string>* dirs)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do
    {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ListFiles(p, files, dirs);
            dirs->push_back(p);
        }
        else
            files->push_back(p);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// one grid row as SGDToCSV would write it (every cell followed by ",")
static std::string RowText(vclcompat::TStringGrid* g, int r)
{
    std::string s;
    for (int c = 0; c < (int)g->ColCount; ++c)
    {
        s += g->Cells[c][r].c_str();
        s += ",";
    }
    return s;
}

// the grid holds exactly `want` (RowCount n, ColCount 7); on a mismatch print what it holds (unless quiet)
static bool GridIs(vclcompat::TStringGrid* g, const std::vector<std::string>& want, bool quiet)
{
    bool ok = (int)g->RowCount == (int)want.size() && (int)g->ColCount == 7;
    for (int r = 0; ok && r < (int)want.size(); ++r) ok = RowText(g, r) == want[r];
    if (!ok && !quiet)
    {
        std::printf("    RowCount %d ColCount %d (want %d / 7)\n", (int)g->RowCount, (int)g->ColCount, (int)want.size());
        for (int r = 0; r < (int)g->RowCount; ++r)
            std::printf("    row %2d got  [%s]\n             want [%s]\n", r, RowText(g, r).c_str(),
                        r < (int)want.size() ? want[r].c_str() : "(none)");
    }
    return ok;
}

// the golden no-DB result: sections 1 and 2 = "No Record!!", section 3 from the INI files
static std::vector<std::string> Report(const char* start, const char* end, const char* runTimeH, const char* loaders)
{
    std::vector<std::string> v;
    v.push_back(",,,,,,,");                                      //  0 jam list header (MyDBVProcess writes none without a DB)
    v.push_back(",No Record!!,,,,,,");                           //  1
    v.push_back(",,,,,,,");                                      //  2 iRowNullSpace
    v.push_back(",,,,,,,");                                      //  3
    v.push_back(",,,,,,,");                                      //  4 jam statistics header
    v.push_back(",No Record!!,,,,00:00:00,00:00:00,");           //  5 cols 5 / 6 = ConvertSecondToSPC(atoi(""))
    v.push_back(",,,,,,,");                                      //  6 iRowNullSpace
    v.push_back(",,,,,,,");                                      //  7
    v.push_back(std::string("Data Period,") + start + " 08:00:00," + end + " 08:00:00,,,,,");   //  8
    v.push_back(std::string("Run Time/H,") + runTimeH + ",,,,,,");                             //  9
    v.push_back(std::string("Total Loader,") + loaders + ",,,,,,");                            // 10
    v.push_back("Total Count,0,,,,,,");                          // 11 the jam-row count
    v.push_back("Down Time,00:00:00,,,,,,");                     // 12
    v.push_back("MTTR,,,,,,,");                                  // 13 empty: no jam
    return v;
}

static void Dirty(vclcompat::TStringGrid* g)                       // RowCount 20, ColCount 9, garbage everywhere
{
    g->ColCount = 9;
    g->RowCount = 20;
    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 9; ++c)
            g->Cells[c][r] = AnsiString("old");
}

// CONTROL: the stub that stood at cObserver.cpp:2040-2044 before this card
static void OldStub(TfObserver*, AnsiString, AnsiString, AnsiString, AnsiString) {}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("MyDB_O19_Summary\n");

    // ---- 0. containment first (the b12ab375 rule): refuse, exit 2, before ANY Handler code ----
    if (!W906TestInsideCtestRoots("MyDB_O19_Summary"))
        return 2;
    {
        const char* const envs[] = { "W906_PRODLOADER_ROOT", "W906_O19_ROOT" };
        bool contained = true;
        for (size_t i = 0; i < sizeof(envs) / sizeof(envs[0]); ++i)
        {
            const char* v = std::getenv(envs[i]);
            if (Lower(v).find("machine_log_scratch") == std::string::npos)
            {
                std::printf("  %s = %s\n", envs[i], v ? v : "(unset)");
                contained = false;
            }
        }
        if (!contained)
        {
            std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R MyDB_O19_Summary) -- nothing was called\n");
            return 2;
        }
    }

    // ---- the sandbox ----
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_o19_" + stamp;
    const std::string plDir = root + "\\Production_Loader";
    const std::string o19Dir = root + "\\O19";
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA(plDir.c_str(), 0);
    ::CreateDirectoryA(o19Dir.c_str(), 0);
    static std::string s_plEnv, s_o19Env;
    s_plEnv = "W906_PRODLOADER_ROOT=" + plDir;
    s_o19Env = "W906_O19_ROOT=" + o19Dir;
    HT9045_TEST_PUTENV(s_plEnv.c_str());
    HT9045_TEST_PUTENV(s_o19Env.c_str());
    asProduct_LoaderPath = AnsiString(plDir.c_str());
    IniConfig.asO19_SavePath = AnsiString(o19Dir.c_str());
    {
        const char* pe = std::getenv("W906_PRODLOADER_ROOT");
        const char* oe = std::getenv("W906_O19_ROOT");
        if (UnderMachineTree(root) || pe == 0 || std::string(pe) != plDir || oe == 0 || std::string(oe) != o19Dir ||
            UnderMachineTree(asProduct_LoaderPath.c_str()) || UnderMachineTree(IniConfig.asO19_SavePath.c_str()))
        {
            std::printf("  ABORT: %s is not the sandbox (W906_PRODLOADER_ROOT = %s, W906_O19_ROOT = %s) -- nothing was called\n",
                        root.c_str(), pe ? pe : "(unset)", oe ? oe : "(unset)");
            return 2;
        }
    }
    if (fObserver == NULL || fObserver->strngrdMDBQuery == NULL || fObserver->Chart2 == NULL)
    {
        std::printf("  FAIL: fObserver (cObserver.cpp:3289) or its grid / chart is NULL\n");
        return 1;
    }
    TfObserverGrid* grid = fObserver->strngrdMDBQuery;

    // ---- 1. the Production_Loader stamps (golden cMyDB.cpp:423-430 layout, the same writer) ----
    struct Stamp { const char* year; const char* name; int loader; unsigned long seconds; };
    const Stamp stamps[] = {
        { "2026", "2026-10-01-0800-2000.txt", 120, 7200 },
        { "2026", "2026-10-01-2000-0800.txt", 30, 1800 },
        { "2026", "2026-09-24-0800-2000.txt", 5, 360 },
        { "2026", "2026-10-02-0800-2000.txt", 999, 99999 },   // the end date of the day / week reports: never counted
        { "2025", "2025-12-31-2000-0800.txt", 7, 700 },       // the month report across the year
    };
    {
        bool ok = true;
        for (size_t i = 0; i < sizeof(stamps) / sizeof(stamps[0]); ++i)
        {
            const std::string yd = plDir + "\\" + stamps[i].year;
            ::CreateDirectoryA(yd.c_str(), 0);
            const std::string p = yd + "\\" + stamps[i].name;
            WriteIniData(AnsiString(p.c_str()), "Product", "LoaderCount", stamps[i].loader);
            WriteIniData(AnsiString(p.c_str()), "Product", "ProductTime", stamps[i].seconds);
            char want[64];
            std::snprintf(want, sizeof(want), "LoaderCount=%d", stamps[i].loader);
            const std::string b = Bytes(p);
            if (b.find("[Product]") == std::string::npos || b.find(want) == std::string::npos)
            {
                std::printf("    %s holds [%s]\n", p.c_str(), b.c_str());
                ok = false;
            }
        }
        CHECK(ok, "1. five Production_Loader stamps written with WriteIniData (the RecordTimeData writer)");
    }

    // ---- 2. day report (golden cMyDB.cpp:450: asDate 08:00:00 .. as1DayDate 08:00:00) on a pre-dirtied grid ----
    const std::vector<std::string> day = Report("2026-10-01", "2026-10-02", "2.5", "150");
    Dirty(grid);
    grid->Visible = false;
    fObserver->Chart2->Visible = true;
    W906O19_DoProduction_Summary_Report(fObserver, "2026-10-01", "08:00:00", "2026-10-02", "08:00:00");
    CHECK(GridIs(grid, day, false),
          "2. day: 14 rows -- \"No Record!!\" jam sections (golden bUseMDB==false), Run Time/H 2.5, Total Loader 150 (10-02 not counted)");
    CHECK(grid->Visible && !fObserver->Chart2->Visible, "2. golden :4906-4907: the grid shown, Chart2 hidden");
    CHECK(grid->DefaultColWidth == 70 && grid->ColWidths[0] == 70 && grid->ColWidths[3] == 550 && grid->ColWidths[4] == 55 &&
              grid->ColWidths[6] == 60,
          "2. golden :4914-4922: DefaultColWidth 70, ColWidths 70 / 550 / 55 / 60");

    // ---- 3. week report (golden :460: as1WeekDate = Now-8 .. today, 8 days, the end date excluded) ----
    W906O19_DoProduction_Summary_Report(fObserver, "2026-09-24", "08:00:00", "2026-10-02", "08:00:00");
    CHECK(GridIs(grid, Report("2026-09-24", "2026-10-02", "2.6", "155"), false),
          "3. week: 8 days -- Run Time/H 2.6 (9360 s), Total Loader 155, the 10-02 file excluded");

    // ---- 4. month report across a year (golden :471: Now-31 .. today, 31 days, reads the 2025 folder) ----
    W906O19_DoProduction_Summary_Report(fObserver, "2025-12-02", "08:00:00", "2026-01-02", "08:00:00");
    CHECK(GridIs(grid, Report("2025-12-02", "2026-01-02", "0.2", "7"), false),
          "4. month across the year: 31 days, <yyyy> = 2025 for 2025-12-31 -- Run Time/H 0.2 (700 s), Total Loader 7");

    // ---- 5. start == end: golden :5011-5014 iDay<=0 -> 1 ----
    W906O19_DoProduction_Summary_Report(fObserver, "2026-10-01", "08:00:00", "2026-10-01", "08:00:00");
    CHECK(GridIs(grid, Report("2026-10-01", "2026-10-01", "2.5", "150"), false), "5. start == end: iDay 1, Total Loader 150");

    // ---- 5b. the A4 seam unset: golden asProduct_LoaderPath itself (still the sandbox) ----
    HT9045_TEST_PUTENV("W906_PRODLOADER_ROOT=");
    W906O19_DoProduction_Summary_Report(fObserver, "2026-10-01", "08:00:00", "2026-10-02", "08:00:00");
    CHECK(std::getenv("W906_PRODLOADER_ROOT") == 0 && GridIs(grid, day, false),
          "5b. W906_PRODLOADER_ROOT unset: the golden global asProduct_LoaderPath (sandbox) -- the same day report");
    HT9045_TEST_PUTENV(s_plEnv.c_str());

    // ---- 6. SGDToCSV (golden common.cpp:2050-2064 through WriteDataToFile "a" + "\n", text mode) ----
    {
        const std::string yd = o19Dir + "\\2026";
        ::CreateDirectoryA(yd.c_str(), 0);
        const std::string csv = yd + "\\HOST_1002.csv";
        W906O19_DoProduction_Summary_Report(fObserver, "2026-10-01", "08:00:00", "2026-10-02", "08:00:00");
        SGDToCSV(fObserver->strngrdMDBQuery, ",", ";", AnsiString(csv.c_str()));
        std::string want;
        for (size_t i = 0; i < day.size(); ++i) want += day[i] + "\r\n";
        const std::string got1 = Bytes(csv);
        if (got1 != want) std::printf("    got  [%s]\n    want [%s]\n", got1.c_str(), want.c_str());
        CHECK(got1 == want, "6. the CSV: the 14 rows, 7 fields each followed by \",\", CRLF");
        SGDToCSV(fObserver->strngrdMDBQuery, ",", ";", AnsiString(csv.c_str()));
        const std::string got2 = Bytes(csv);
        CHECK(got2 == want + want, "6. a second call APPENDS (28 lines, no header) -- golden fopen \"a\"");
        vclcompat::TStringGrid one(1, 1);
        one.Cells[0][0] = AnsiString("a,b,c");
        const std::string csv2 = yd + "\\comma.csv";
        SGDToCSV(&one, ",", ";", AnsiString(csv2.c_str()));
        const std::string got3 = Bytes(csv2);
        if (got3 != "a;b,c,\r\n") std::printf("    got  [%s]\n", got3.c_str());
        CHECK(got3 == "a;b,c,\r\n", "6. only the FIRST \",\" in a cell becomes \";\" (TReplaceFlags() without rfReplaceAll)");
    }

    // ---- 7. the helpers ----
    {
        const int a = W906O19_DayOfWeek(EncodeDate(1899, 12, 30));
        const int b = W906O19_DayOfWeek(EncodeDate(2026, 9, 27));
        const int c = W906O19_DayOfWeek(EncodeDate(2026, 9, 28));
        const int d = W906O19_DayOfWeek(EncodeDate(2026, 10, 2));
        const int e = W906O19_DayOfWeek(EncodeDate(2027, 1, 1));
        const int f = W906O19_DayOfWeek(EncodeDate(2026, 10, 2) + EncodeTime(8, 0, 5, 0));
        if (!(a == 7 && b == 1 && c == 2 && d == 6 && e == 6 && f == 6))
            std::printf("    got %d %d %d %d %d %d (want 7 1 2 6 6 6)\n", a, b, c, d, e, f);
        CHECK(a == 7 && b == 1 && c == 2 && d == 6 && e == 6 && f == 6,
              "7. W906O19_DayOfWeek = BCB DayOfWeek: 1899-12-30 Sat 7, 2026-09-27 Sun 1, 09-28 Mon 2, 10-02 Fri 6, 2027-01-01 Fri 6, 10-02 08:00:05 6");
    }
    {
        WSADATA wsa;
        ::WSAStartup(MAKEWORD(2, 0), &wsa);
        char hn[256];
        std::memset(hn, 0, sizeof(hn));
        ::gethostname(hn, (int)sizeof(hn) - 1);
        ::WSACleanup();
        const std::string mine = W906O19_HostName().c_str();
        if (mine != hn) std::printf("    W906O19_HostName [%s], gethostname [%s]\n", mine.c_str(), hn);
        CHECK(!mine.empty() && mine == hn, "7. W906O19_HostName() = Winsock gethostname (golden cConfiguration.cpp:121-127), not empty");
    }
    {
        const bool envSide = std::string(W906O19_SaveDir().c_str()) == o19Dir;
        HT9045_TEST_PUTENV("W906_O19_ROOT=");
        const std::string cfg = o19Dir + "\\from_ini";
        IniConfig.asO19_SavePath = AnsiString(cfg.c_str());
        const bool iniSide = std::getenv("W906_O19_ROOT") == 0 && std::string(W906O19_SaveDir().c_str()) == cfg;
        HT9045_TEST_PUTENV(s_o19Env.c_str());
        IniConfig.asO19_SavePath = AnsiString(o19Dir.c_str());
        CHECK(envSide && iniSide, "7. W906O19_SaveDir: W906_O19_ROOT when set, IniConfig.asO19_SavePath once it is cleared");
    }

    // ---- CONTROL: the old no-op stub must fail the step-2 check ----
    {
        Dirty(grid);
        OldStub(fObserver, "2026-10-01", "08:00:00", "2026-10-02", "08:00:00");
        const bool red = !GridIs(grid, day, true) && (int)grid->RowCount == 20;
        CHECK(red, "CONTROL: the old stub (cObserver.cpp:2040-2044, no-op) leaves RowCount 20 -- the step-2 check is red against it");
    }

    std::printf("MyDB_O19_Summary: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
    {
        std::vector<std::string> f, d;
        ListFiles(root, &f, &d);
        for (size_t i = 0; i < f.size(); ++i) ::DeleteFileA(f[i].c_str());
        for (size_t i = 0; i < d.size(); ++i) ::RemoveDirectoryA(d[i].c_str());
        ::RemoveDirectoryA(root.c_str());
    }
    return g_fail ? 1 : 0;
}
