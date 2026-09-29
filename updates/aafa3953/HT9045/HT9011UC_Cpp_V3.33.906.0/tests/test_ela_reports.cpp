// =============================================================================
//  test_ela_reports.cpp -- ELA reports plan R1 / R2: ElaFileUtil, ElaCore GetEventLogTextToVec, ElaReports.
//
//  AI(W906-ELA-R1) / AI(W906-ELA-R2) / AI(W906-ELA-R3) 20260927 (St02-E).  Suite name (add_test): ELA_Reports
//  Plan: D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md §4.
//
//    1. ElaFileUtil (golden Rev891 Common.cpp / FileInfo.cpp / Analyzer.cpp helpers): PathCombin (local / FTP),
//       FixFolderPath, GetNameAndExtension, ExtractFilePath / Name (incl. a cp950 trail byte 0x5C), IsValidFileName,
//       MyForceDirectories (+ the MyDBIProcess hook ElaFtp installs), EnsureDirectoriesExist, WriteDataToFile (append /
//       overwrite, LF), SaveBytesToFile, the W906_HT9045LOG_ROOT seam;
//    2. ElaCore R1: GetStartEndGap, LogRecord, GetEventLogTextToVec (both forms) with the SAME Options as
//       GetEventLogText -- W15 split, W19 file choice and cross-file de-dup (the AllEventLog copy) -- and golden mode.
//    3. R2 SaveSummary on 2025-09-10 (W23 event log + Production_Log copies, a fake clock): the file name, CRLF, the 20
//       summary lines, "" + the by-hour table (header + 24 rows of 14 cells), the Top5 grids with "Include alarm list",
//       Total Input Qty 265 = the hours' sum, MES 123 (W15 B); no repeated detail on a second save (W18 B) vs golden's
//       append in place; IsValidFileName refusing a name; D-e: a cp950 Handler ID comes out as UTF-8;
//    4. DoVTestSaveSummary (EL_VTEST_MTBF_SUM): Now()-7 00:00:00 .. Now()-1 23:59:59, asO19_SavePath, no detail;
//    5. the R5 call point: Timer2Due (O06-4 never without [O06-8] -- ★W43 C; 10 / 30 min; N10-3 methods incl. the
//       radio-group clamp)
//       and Timer2Run (O06SaveSummaryData + N10SaveSummaryData: AutoSaveProductionPath, the EventLogSummary\yyyymm
//       folder under W906_HT9045LOG_ROOT, the net drive; folders made first even when the job is off, as golden);
//    6. the Hub job: EL_VTEST_MTBF_SUM writes the MTBF summary and leaves a job record with its range -- the page's
//       snapshot is NOT replaced (St02-E 20260927); with O10 off it is skipped (D-a); the upload jobs gated off (W22);
//    7. the oracle: <ID>-SummaryData_r2detail.txt / _r2alarm.txt from a BCB EventlogAnalyzer.exe run
//       (tests/fixtures/ela/oracle/README.txt) against the port in golden mode, byte for byte after cp950 -> UTF-8
//       (D-e).  A missing oracle is a SKIP, not a failure.
//    8. R3 N34 ChipAdvanced (golden uChipAdvancedFunc.cpp): the helpers (GetHourMinSecStr "%02.2f", ParseField golden /
//       W18 B), a lot of made-up rows with every value worked out by hand (report bytes, file name, regenerated not
//       appended), golden mode (the Lot ID's first character dropped), D-c (no row: no crash, no report), golden's
//       index 0, a W23 sample trimmed around its last Lot Start (15 codes / 39 events / 336 s), and the Hub job (O10).
//  Every write is under %TEMP%\ht9045_ela_reports_<tick>; the test refuses to run when that root (or the
//  W906_HT9045LOG_ROOT it sets) is under D:\HT9045, D:\HT9045_Log, D:\RMS, D:\MTBF_Summary or D:\EventlogAnalyzer.
//  The W23 samples are copies in tests/fixtures/ela (W906_ELA_FIXTURE_DIR, read only), copied into the sandbox first.
//  The sandbox is removed on a green run only.
// =============================================================================
#include "EventLogAnalysis/ElaCore.h"
#include "EventLogAnalysis/ElaFileUtil.h"
#include "EventLogAnalysis/ElaReports.h"   // R2
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

static int g_pass = 0, g_fail = 0, g_skip = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)
#define SKIP(msg) do { printf("  SKIP: %s\n", msg); ++g_skip; } while (0)

#ifndef W906_ELA_FIXTURE_DIR
#define W906_ELA_FIXTURE_DIR ""
#endif

static std::string g_root;          // %TEMP%\ht9045_ela_reports_<tick>

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

// true = p is one of the machine folders (or under one) this test must never write to
static bool Forbidden(const std::string& p)
{
    static const char* const kRoots[] = { "d:\\ht9045", "d:\\ht9045_log", "d:\\rms", "d:\\mtbf_summary",
                                          "d:\\eventloganalyzer" };
    const std::string l = Lower(p);
    for (size_t k = 0; k < sizeof(kRoots) / sizeof(kRoots[0]); ++k)
    {
        const std::string r = kRoots[k];
        if (l.compare(0, r.size(), r) == 0 && (l.size() == r.size() || l[r.size()] == '\\'))
            return true;
    }
    return false;
}

static void MkDirs(const std::string& path)
{
    for (size_t i = 3; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '\\')
            ::CreateDirectoryA(path.substr(0, i).c_str(), 0);
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

static std::string ReadBytes(const std::string& p, bool* ok)
{
    std::string s;
    if (ok) *ok = false;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    if (ok) *ok = true;
    return s;
}

static bool WriteBytes(const std::string& p, const std::string& bytes)
{
    if (Forbidden(p)) return false;
    const size_t slash = p.find_last_of('\\');
    if (slash != std::string::npos) MkDirs(p.substr(0, slash));
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(bytes.data(), 1, bytes.size(), f) == bytes.size();
    std::fclose(f);
    return ok;
}

static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
static bool IsDir(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// copy one fixture (tests/fixtures/ela/<rel>, read only) to <dstRoot>\<rel>
static bool CopyFixture(const std::string& rel, const std::string& dstRoot)
{
    bool ok = false;
    const std::string bytes = ReadBytes(std::string(W906_ELA_FIXTURE_DIR) + "\\" + rel, &ok);
    return ok && !bytes.empty() && WriteBytes(dstRoot + "\\" + rel, bytes);
}

static std::vector<std::string> g_hookLines;
static void TestHook(const std::string& commaText) { g_hookLines.push_back(commaText); }

static bool HasSub(const std::vector<std::string>& v, const char* t)
{
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i].find(t) != std::string::npos) return true;
    return false;
}

static int CountCodes(const std::vector<ela::LogRecord>& recs)
{
    // the rows GetEventLogText counts as stops: AlarmCode holds JAM / WAR / MES (every record has > elStopTime fields)
    int n = 0;
    for (size_t i = 0; i < recs.size(); ++i)
    {
        const std::string& c = recs[i].alarmCode;
        if (c.find("JAM") != std::string::npos || c.find("WAR") != std::string::npos || c.find("MES") != std::string::npos)
            ++n;
    }
    return n;
}

static ela::Options GoldenOptions(const std::string& jam)
{
    ela::Options o;
    o.jamIniPath = jam;
    o.bcbCommaText = true;
    o.keepGoldenBugs = true;
    o.goldenFileRule = true;
    return o;
}

// ---- R2 helpers.  AI(W906-ELA-R2) 20260927 (St02-E) ----
class FakeClock : public ela::Clock
{
public:
    explicit FakeClock(double t) : t_(t) {}
    double Now() const { return t_; }
private:
    double t_;
};

// a report's lines; *crlf = every line ends in CRLF and holds no other CR / LF
static std::vector<std::string> ReportLines(const std::string& bytes, bool* crlf)
{
    std::vector<std::string> v;
    *crlf = true;
    size_t b = 0;
    while (b < bytes.size())
    {
        const size_t e = bytes.find("\r\n", b);
        if (e == std::string::npos) { *crlf = false; v.push_back(bytes.substr(b)); break; }
        const std::string line = bytes.substr(b, e - b);
        if (line.find('\r') != std::string::npos || line.find('\n') != std::string::npos) *crlf = false;
        v.push_back(line);
        b = e + 2;
    }
    return v;
}

static std::string ReadFileBytes(const std::string& p)
{
    bool ok = false;
    return ReadBytes(p, &ok);
}

static int CountCh(const std::string& s, char c)
{
    int n = 0;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == c) ++n;
    return n;
}

// the int after "<label>" on the summary line that starts with it (-1 = no such line)
static int SummaryInt(const std::vector<std::string>& lines, const char* label)
{
    const size_t n = std::strlen(label);
    for (size_t i = 0; i < lines.size(); ++i)
        if (lines[i].compare(0, n, label) == 0) return std::atoi(lines[i].c_str() + n);
    return -1;
}

static std::string Cp950ToUtf8(const std::string& s)
{
    if (s.empty()) return s;
    const int wn = ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::vector<wchar_t> w((size_t)wn);
    ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), &w[0], wn);
    const int un = ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, NULL, 0, NULL, NULL);
    if (un <= 0) return std::string();
    std::string out((size_t)un, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, &out[0], un, NULL, NULL);
    return out;
}

// the first file in dir matching pattern ("" = none)
static std::string FindOne(const std::string& dir, const char* pattern)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\" + pattern).c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return std::string();
    const std::string p = dir + "\\" + fd.cFileName;
    ::FindClose(h);
    return p;
}

static bool OnlyFileIn(const std::string& dir, const std::string& name)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    int files = 0;
    bool found = false;
    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        ++files;
        if (name == fd.cFileName) found = true;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return found && files == 1;
}

static bool EmptyDir(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    int n = 0;
    do
    {
        if (std::strcmp(fd.cFileName, ".") != 0 && std::strcmp(fd.cFileName, "..") != 0) ++n;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return n == 0;
}

static const char* const kHourHead =
    "DateTime,Input Qty,UPH,Contact Count,Total Test Time,Avg Test Time,Total Stop Time,Alarm Count,Alarm Time,MUBA,MTBA,"
    "Faliure Count,Faliure Time,MTBF,";
static const char* const kLine = "//===================================";

// section 7: one oracle file against the port in golden mode
static void CompareOracle(const std::string& oracle, bool includeAlarmList, const std::string& ev7,
                          const std::string& prod7, const std::string& jam7)
{
    const std::string want = Cp950ToUtf8(ReadFileBytes(oracle));   // D-e: the BCB exe writes cp950
    bool crlf = false;
    const std::vector<std::string> wl = ReportLines(want, &crlf);
    const std::string idTag = "Handler ID:        ";
    std::string id;
    if (wl.size() > 1 && wl[1].compare(0, idTag.size(), idTag) == 0) id = wl[1].substr(idTag.size());
    ela::QueryRequest oq;
    oq.eventLogDir = ev7;
    oq.prodLogDir = prod7;
    oq.handlerId = id;
    oq.topFilter = 0;
    const double d = ela::EncodeDate(2025, 9, 10);
    oq.startDate = d;
    oq.startTime = d;
    oq.endDate = d;
    oq.endTime = d + ela::EncodeTime(23, 59, 59, 0);
    // golden G1: iFailCount is never reset, so the exe's Fail Count also holds its earlier queries (its start-up query
    //   of the day it ran).  First pass finds that offset from the oracle's "Fail Count:" line, second pass starts there.
    ela::Analyzer first(GoldenOptions(jam7));
    ela::BtnQuery(first, oq);
    const int offset = SummaryInt(wl, "Fail Count:        ") - first.MySummary.iFailCount;
    ela::Analyzer oa(GoldenOptions(jam7));
    oa.MySummary.iFailCount = offset > 0 ? offset : 0;
    ela::BtnQuery(oa, oq);
    const std::string got = ela::SummaryBytes(ela::BuildSummaryLines(oa, true, includeAlarmList));
    if (offset > 0)
        printf("  note: the oracle's Fail Count holds %d from the exe's earlier queries (golden G1)\n", offset);
    if (got != want)
    {
        bool c2 = false;
        const std::vector<std::string> gl = ReportLines(got, &c2);
        for (size_t i = 0; i < wl.size() || i < gl.size(); ++i)
        {
            const std::string a = i < gl.size() ? gl[i] : std::string("<none>");
            const std::string b = i < wl.size() ? wl[i] : std::string("<none>");
            if (a != b) { printf("  first difference, line %u:\n    port:   %s\n    oracle: %s\n", (unsigned)(i + 1), a.c_str(), b.c_str()); break; }
        }
    }
    const std::string msg = "7. oracle " + oracle + ": the port (golden mode) == the BCB exe byte for byte (cp950 -> UTF-8, D-e)";
    CHECK(!id.empty() && got == want, msg.c_str());
}

static std::string Itos(int v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}

int main()
{
    printf("ELA_Reports\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_ela_reports_" + tick;
    if (!g_root.empty() && g_root[g_root.size() - 1] == '\\') g_root.erase(g_root.size() - 1);
    if (Forbidden(g_root) || std::string(W906_ELA_FIXTURE_DIR).empty())
    {
        printf("  REFUSED: sandbox %s is a machine folder, or no fixture dir\n", g_root.c_str());
        return 1;
    }
    MkDirs(g_root);
    const std::string logRoot = g_root + "\\HT9045_Log";      // W906_HT9045LOG_ROOT (N10's EventLogSummary, FTP_Log)
    static std::string env;                                   // a CRT putenv: getenv() reads the CRT's copy
    env = "W906_HT9045LOG_ROOT=" + logRoot;
    HT9045_TEST_PUTENV(env.c_str());
    if (Forbidden(ela::Ht9045LogRoot()) || ela::Ht9045LogRoot() != logRoot)
    {
        printf("  REFUSED: W906_HT9045LOG_ROOT did not take (%s)\n", ela::Ht9045LogRoot().c_str());
        return 1;
    }

    // the sandbox EventLogTxt: copies of the W23 samples (tests/fixtures/ela/EventLogTxt)
    const std::string ev = g_root + "\\EventLogTxt";
    const bool fx0910 = CopyFixture("EventLogTxt\\2025\\09\\HT-9016C_PMLD1019_EventLogTxt_20250910.csv", g_root);
    const bool fx0825 = CopyFixture("EventLogTxt\\2025\\08\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", g_root);
    CHECK(fx0910 && fx0825, "0. W23 fixtures copied into the sandbox");
    const std::string noJam = g_root + "\\no_JAM0000.dat";   // absent: the MTBA / MTBF look-ups read (and write) nothing

    // ---------------------------------------------------------------------------------------------------------
    // 1. ElaFileUtil
    // ---------------------------------------------------------------------------------------------------------
    {
        CHECK(ela::PathCombin("C:\\a", "b.txt") == "C:\\a\\b.txt" && ela::PathCombin("C:\\a\\", "b.txt") == "C:\\a\\b.txt",
              "1. PathCombin: a local path gets one '\\'");
        CHECK(ela::PathCombin("/Summary/naslfs2/Handler", "h1") == "/Summary/naslfs2/Handler/h1" &&
                  ela::PathCombin("/Summary/", "Jam Rate.txt") == "/Summary/Jam Rate.txt" &&
                  ela::PathCombin("a/b\\c", "f") == "a/b\\c/f",
              "1. PathCombin: a '/' anywhere makes it an FTP path (golden)");
        CHECK(ela::PathCombin("", "f") == "f", "1. PathCombin: empty path = the file alone");
        CHECK(ela::FixFolderPath("") == "" && ela::FixFolderPath("C:\\x") == "C:\\x\\" &&
                  ela::FixFolderPath("C:\\x\\") == "C:\\x\\", "1. FixFolderPath");
        std::string n, e;
        ela::GetNameAndExtension("Jam Rate.txt", n, e);
        const bool g1 = (n == "Jam Rate" && e == "txt");
        ela::GetNameAndExtension("a.b.c", n, e);
        const bool g2 = (n == "a.b" && e == "c");
        ela::GetNameAndExtension("noext", n, e);
        const bool g3 = (n == "" && e == "");
        ela::GetNameAndExtension("dir.v\\name", n, e);
        const bool g4 = (n == "dir" && e == "v\\name");
        CHECK(g1 && g2 && g3 && g4, "1. GetNameAndExtension: the last '.' of the whole string, none = both empty");
        CHECK(ela::ExtractFilePathA("C:\\a\\b.txt") == "C:\\a\\" && ela::ExtractFileNameA("C:\\a\\b.txt") == "b.txt" &&
                  ela::ExtractFilePathA("C:b.txt") == "C:" && ela::ExtractFileNameA("b.txt") == "b.txt",
              "1. ExtractFilePath / ExtractFileName ('\\' and ':')");
        if (::IsDBCSLeadByte(0xB3))
            CHECK(ela::ExtractFileNameA("C:\\x\\\xB3\x5C.txt") == "\xB3\x5C.txt" &&
                      ela::ExtractFilePathA("C:\\x\\\xB3\x5C.txt") == "C:\\x\\",
                  "1. MBCS: the trail byte 0x5C of cp950 B3 5C is not a path delimiter (Delphi LastDelimiter)");
        else
            SKIP("1. MBCS trail-byte case: the system ANSI code page is not DBCS here");

        // IsValidFileName
        const std::string vdir = g_root + "\\valid\\sub";
        CHECK(!ela::IsValidFileName(""), "1. IsValidFileName: empty = false");
        CHECK(ela::IsValidFileName(vdir + "\\ok.txt") && IsDir(vdir), "1. IsValidFileName: true, and the folder is made");
        CHECK(!ela::IsValidFileName(g_root + "\\a?b.txt") && !ela::IsValidFileName(g_root + "\\a/b.txt") &&
                  !ela::IsValidFileName(g_root + "\\a|b.txt") && !ela::IsValidFileName(g_root + "\\a\"b.txt"),
              "1. IsValidFileName: ? / | \" in the name = false");
        CHECK(!ela::IsValidFileName("zz_ela_r1_rel\\f.txt") && !Exists("zz_ela_r1_rel"),
              "1. IsValidFileName: a relative folder with no parent = false (golden ForceDirectories raises), nothing made");

        // MyForceDirectories (+ the MyDBIProcess hook)
        std::vector<std::string> exc;
        CHECK(ela::MyForceDirectories("", "T", &exc) == -1 && exc.size() == 1 && exc[0] == "Directory value is NULL!",
              "1. MyForceDirectories(\"\") = -1, \"Directory value is NULL!\"");
        exc.clear();
        const std::string deep = g_root + "\\m1\\m2\\m3";
        CHECK(ela::MyForceDirectories(deep, "T", &exc) == 1 && IsDir(deep) && exc.empty(),
              "1. MyForceDirectories makes a nested folder = 1");
        CHECK(ela::MyForceDirectories(deep + "\\", "T", &exc) == 1, "1. MyForceDirectories: existing (trailing '\\') = 1");
        ela::SetDbiProcessHook(&TestHook);
        exc.clear();
        g_hookLines.clear();
        CHECK(ela::MyForceDirectories("zz_ela_r1_rel", "T", &exc) == -1 && !Exists("zz_ela_r1_rel") && exc.size() == 3 &&
                  exc[0] == "Unable to create directory" && exc[1] == "zz_ela_r1_rel -- T" &&
                  HasSub(exc, "Create directory fail!"),
              "1. MyForceDirectories: golden's catch path (a relative name: ForceDirectories raises) -- three memo lines");
        CHECK(g_hookLines.size() == 1 && g_hookLines[0] == exc[2] && g_hookLines[0].find("Exception,MyForceDirectories,") == 0,
              "1. MyDBIProcess: the CommaText line also goes to the FTP_Log hook (ElaFtp, R4)");
        ela::SetDbiProcessHook(0);
        CHECK(ela::GetDbiProcessHook() == 0, "1. hook cleared");

        // EnsureDirectoriesExist
        const std::string ens = g_root + "\\e1\\e2";
        ela::EnsureDirectoriesExist(ens + "\\");
        CHECK(IsDir(g_root + "\\e1") && IsDir(ens), "1. EnsureDirectoriesExist makes every level");

        // WriteDataToFile (LF, append) / SaveBytesToFile (create)
        const std::string wf = g_root + "\\w\\data.txt";
        MkDirs(g_root + "\\w");
        const bool w1 = ela::WriteDataToFile(wf, "a") && ela::WriteDataToFile(wf, "b,c");
        bool ok = false;
        CHECK(w1 && ReadBytes(wf, &ok) == "a\nb,c\n" && ok, "1. WriteDataToFile appends, each line + LF only (golden)");
        CHECK(ela::WriteDataToFile(wf, "z", true) && ReadBytes(wf, &ok) == "z\n", "1. WriteDataToFile(bOverWrite) re-creates");
        CHECK(!ela::WriteDataToFile(g_root + "\\no_such_dir\\x.txt", "q"), "1. WriteDataToFile: missing folder = false");
        const std::string sf = g_root + "\\w\\save.txt";
        std::string err;
        CHECK(ela::SaveBytesToFile(sf, "12345", &err) && ela::SaveBytesToFile(sf, "ab\r\n", &err) &&
                  ReadBytes(sf, &ok) == "ab\r\n", "1. SaveBytesToFile creates / replaces (TFileStream fmCreate)");
        CHECK(!ela::SaveBytesToFile(g_root + "\\no_such_dir\\x.txt", "q", &err) && err.find("Cannot create file") == 0,
              "1. SaveBytesToFile: EFCreateError text");
        CHECK(ela::Ht9045LogRoot() == logRoot, "1. W906_HT9045LOG_ROOT seam (unset = golden D:\\HT9045_Log)");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 2. ElaCore R1: GetStartEndGap, GetEventLogTextToVec (same Options as GetEventLogText)
    // ---------------------------------------------------------------------------------------------------------
    {
        CHECK(ela::GetStartEndGap(1.25, 3.5) == 2.25 && ela::GetStartEndGap(3.0, 1.0) == -2.0,
              "2. GetStartEndGap = dtEnd - dtStart (Common.cpp:511)");

        const std::vector<std::string> fav = ela::LoadFavorite(ev);
        const double d0910 = ela::EncodeDate(2025, 9, 10), e235959 = ela::EncodeTime(23, 59, 59, 0);

        // default Options (the rulings): W15 B split, W19 B files
        ela::Options on;
        on.jamIniPath = noJam;
        ela::Analyzer na(on);
        na.SetRange(d0910, 0.0, d0910, e235959);
        na.GetEventLogText(fav);
        CHECK(na.MySummary.iStopCount == 131 && !na.slStopList.empty(), "2. (reference) GetEventLogText 2025-09-10: 131 stop rows (ELA_Core 9)");
        std::vector<ela::LogRecord> recs;
        na.GetEventLogTextToVec(d0910, 0.0, d0910, e235959, fav, recs);
        CHECK(na.dtStart == d0910 && na.iDate == 1 && na.dtEnd > d0910 + 0.9999 && na.dtEnd < d0910 + 1.0,
              "2. first form: the pickers set dtStart / dtEnd / iDate (golden GetSysDateTimeRange)");
        CHECK(na.slStopList.empty() && na.slJamList.empty() && na.slJamWarList.empty() && na.slWarList.empty() &&
                  na.slAlarmList.empty() && na.slFailList.empty(), "2. golden clears the six query lists (:1198-1203)");
        CHECK(!recs.empty() && CountCodes(recs) == 131, "2. W15 B: every JAM / WAR / MES record = GetEventLogText's 131 stops");
        bool recipeEmpty = true, fieldsOk = true;
        for (size_t i = 0; i < recs.size(); ++i)
        {
            if (!recs[i].recipe.empty()) recipeEmpty = false;
            if (!ela::IsValidDateString(recs[i].date) || !ela::IsValidTimeString(recs[i].time) ||
                recs[i].duplicate.find('1') != std::string::npos)
                fieldsOk = false;
        }
        CHECK(recipeEmpty, "2. LogRecord.recipe is never filled (golden :1244-1254)");
        CHECK(fieldsOk, "2. every record: valid date / time, Duplicate without '1' (golden filters)");
        CHECK(recs[0].date == "2025/09/10" || recs[0].date == "2025-09-10", "2. records carry the row's date text");

        // golden mode: BCB6 CommaText (the 8 unquoted "16 System" rows are lost) and golden's file rule
        ela::Analyzer ga(GoldenOptions(noJam));
        std::vector<ela::LogRecord> grecs;
        ga.GetEventLogTextToVec(d0910, 0.0, d0910, e235959, fav, grecs);
        CHECK(CountCodes(grecs) == 123, "2. golden mode: 123 JAM / WAR / MES records (ELA_Core 9: golden 123 stops)");

        // W19 B: the AllEventLog copy of 2025-08-25 (made here without CR, as the Handler's MyStringList.cpp:599)
        bool ok = false;
        std::string day = ReadBytes(ev + "\\2025\\08\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", &ok);
        std::string noCr;
        for (size_t i = 0; i < day.size(); ++i) if (day[i] != '\r') noCr += day[i];
        CHECK(ok && WriteBytes(ev + "\\AllEventLog\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", noCr),
              "2. AllEventLog copy made");
        const std::vector<std::string> fav2 = ela::LoadFavorite(ev);
        std::vector<std::string> only;
        only.push_back(ev + "\\2025\\08\\HT9046LS_JFTH013_EventLogTxt_20250825.csv");
        const double d0825 = ela::EncodeDate(2025, 8, 25);
        const double s0825 = d0825, t0825 = d0825 + e235959;
        ela::Analyzer b1(on), b2(on);
        std::vector<ela::LogRecord> single, both;
        b1.GetEventLogTextToVec(s0825, t0825, 1, only, single);
        b2.GetEventLogTextToVec(s0825, t0825, 1, fav2, both);
        CHECK(!single.empty() && both.size() == single.size() && b2.vecDupRowsSkipped == (int)single.size(),
              "2. W19 B: the AllEventLog copy adds nothing -- every row skipped as read from another file");
        ela::Analyzer g1(GoldenOptions(noJam)), g2(GoldenOptions(noJam));
        std::vector<ela::LogRecord> gsingle, gboth;
        g1.GetEventLogTextToVec(s0825, t0825, 1, only, gsingle);
        g2.GetEventLogTextToVec(s0825, t0825, 1, fav2, gboth);
        CHECK(!gsingle.empty() && gboth.size() == 2 * gsingle.size() && g2.vecDupRowsSkipped == 0,
              "2. golden mode: the AllEventLog copy is read too -- every row twice (golden :1208-1217)");
        CHECK(!Exists(noJam), "2. no JAM0000.dat was made");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 3. R2 SaveSummary (golden Analyzer.cpp:2614-2688) on 2025-09-10
    // ---------------------------------------------------------------------------------------------------------
    const double d0910 = ela::EncodeDate(2025, 9, 10), e235959 = ela::EncodeTime(23, 59, 59, 0);
    const std::string prodDir = g_root + "\\Production_Log";
    const std::string jam = g_root + "\\JAM0000.dat";               // empty: every look-up writes its default (#17 A, sandbox)
    CHECK(CopyFixture("Production_Log\\202509\\PMLD1019_20250910.csv", g_root) && WriteBytes(jam, ""),
          "3. Production_Log fixture copied, empty JAM0000.dat in the sandbox");
    ela::Options on;
    on.jamIniPath = jam;
    ela::QueryRequest q;
    q.eventLogDir = ev;
    q.prodLogDir = prodDir;
    q.handlerId = "ELA-R2";
    q.topFilter = 0;
    q.startDate = d0910;
    q.startTime = d0910;
    q.endDate = d0910;
    q.endTime = d0910 + e235959;
    const FakeClock clk(d0910 + ela::EncodeTime(13, 0, 0, 0));
    std::string report3;                                            // section 5 compares Timer2Run's files with it
    {
        ela::Analyzer ra(on);
        CHECK(ela::BtnQuery(ra, q), "3. BtnQuery (golden btnQueryClick) 2025-09-10");
        const std::string o06 = g_root + "\\O06";
        std::string path, err;
        const bool w = ela::SaveSummary(ra, o06, "", true, false, clk, &path, &err);
        CHECK(w && path == o06 + "\\ELA-R2-SummaryData_2025-09-10.txt" && IsDir(o06) &&
                  OnlyFileIn(o06, "ELA-R2-SummaryData_2025-09-10.txt"),
              "3. SaveSummary: <Path>\\<ID>-SummaryData_<yyyy-mm-dd of Now()>.txt, the folder made by IsValidFileName");
        report3 = ReadFileBytes(path);
        bool crlf = false;
        const std::vector<std::string> L = ReportLines(report3, &crlf);
        CHECK(crlf && !report3.empty(), "3. every line ends in CRLF (golden TMemo text), nothing after the last");
        CHECK(L.size() == 46, "3. 20 summary lines + \"\" + the by-hour header + 24 hours");
        bool first20 = L.size() >= 20 && ra.mmoSummary.size() == 20;
        for (size_t i = 0; first20 && i < 20; ++i) if (L[i] != ra.mmoSummary[i]) first20 = false;
        CHECK(first20 && L[0] == kLine && L[1] == "Handler ID:        ELA-R2" && L[19] == kLine,
              "3. lines 1-20 = the query's mmoSummary (SetTotalSummary)");
        CHECK(L.size() > 21 && L[20] == "" && L[21] == kHourHead, "3. then \"\" and the by-hour header row (DateTime, ..., MTBF,)");
        bool hours = L.size() == 46;
        int sumIn = 0;
        for (int h = 0; hours && h < 24; ++h)
        {
            char want[32];
            std::snprintf(want, sizeof(want), "2025/09/10 %02d:00:00,", h);
            const std::string& row = L[22 + h];
            if (row.compare(0, std::strlen(want), want) != 0 || CountCh(row, ',') != 14) hours = false;
            else sumIn += std::atoi(row.c_str() + std::strlen(want));
        }
        CHECK(hours, "3. 24 hour rows 00:00 .. 23:00, 14 cells each + ','");
        CHECK(SummaryInt(L, "Total Input Qty:   ") == 265 && sumIn == 265,
              "3. Total Input Qty 265 (the Production_Log fixture's rows) = the hours' Input Qty");
        CHECK(SummaryInt(L, "MES Count:         ") == 123 && SummaryInt(L, "Average UPH:       ") == 11,
              "3. MES 123 (W15 B, ELA_Core 9), UPH 265 / 24 = 11");
        CHECK(ra.mmoSummary1 == L, "3. mmoSummary1 = what was saved");

        // "Include alarm list": the Top5 grids
        std::string path2;
        const bool w2 = ela::SaveSummary(ra, o06, "Named", true, true, clk, &path2, &err);
        const std::vector<std::string> L2 = ReportLines(ReadFileBytes(path2), &crlf);
        const int r5 = ra.iTop5FilterRowCount - 1;
        const int na = ra.sgTop5Alarm.empty() ? 1 : (int)ra.sgTop5Alarm.size();
        CHECK(w2 && path2 == o06 + "\\ELA-R2-SummaryData_Named.txt" && crlf && (int)L2.size() == 52 + r5 + na,
              "3. with a Name: <ID>-SummaryData_<Name>.txt; + the two Top5 grids (W18 B: the first 46 lines not repeated)");
        bool same46 = L2.size() >= 46;
        for (size_t i = 0; same46 && i < 46; ++i) if (L2[i] != L[i]) same46 = false;
        CHECK(same46, "3. W18 B: a second save starts again from the query's summary");
        if ((int)L2.size() == 52 + r5 + na)
        {
            CHECK(L2[46] == kLine && L2[47] == ",UnitName,AlarmCode,Message,Total Count,Total Stoped Time," &&
                      L2[(size_t)(48 + r5)] == "" && L2[(size_t)(49 + r5)] == kLine &&
                      L2[(size_t)(50 + r5)] == "Date,Time,UnitName,AlarmCode,Recovery,StopedTime,Duplicate,Message,ErrorPart," &&
                      L2.back() == "",
                  "3. Top5Filter header (Cells[0][0] empty) / RowCount rows / \"\" / Top5Alarm header / rows / \"\"");
            bool topRows = true;
            for (int i = 0; i < r5; ++i)
            {
                const std::string& row = L2[(size_t)(48 + i)];
                if (i < (int)ra.sgTop5Filter.size())
                {
                    std::string want;
                    for (int j = 0; j < 6; ++j) { if ((size_t)j < ra.sgTop5Filter[(size_t)i].size()) want += ra.sgTop5Filter[(size_t)i][(size_t)j]; want += ","; }
                    if (row != ela::ToUtf8(want)) topRows = false;
                }
                else if (row != ",,,,,,")
                    topRows = false;                                // W18 B: no "Top n" on an empty row
            }
            CHECK(topRows, "3. Top5Filter rows = the grid cells + ',' (6 columns)");
            std::string a0;
            if (!ra.sgTop5Alarm.empty())
                for (int j = 0; j < 9; ++j) { if ((size_t)j < ra.sgTop5Alarm[0].size()) a0 += ra.sgTop5Alarm[0][(size_t)j]; a0 += ","; }
            else
                a0 = ",No Record!!,,,,,,,,";
            CHECK(L2[(size_t)(51 + r5)] == ela::ToUtf8(a0), "3. Top5Alarm first row = its 9 cells + ','");
        }
        std::string path3;
        CHECK(ela::SaveSummary(ra, o06, "", true, false, clk, &path3, &err) && ReadFileBytes(path3) == report3,
              "3. saved again: the same bytes (regenerated, no append -- W21 / plan §3.3; W18 B no repeat)");

        // IsValidFileName refuses
        ra.sHandlerID = "bad|id";
        std::string path4, err4;
        CHECK(!ela::SaveSummary(ra, o06, "", true, false, clk, &path4, &err4) &&
                  err4.find("IsValidFileName") != std::string::npos && !Exists(path4),
              "3. a '|' in the name: not written (golden IsValidFileName)");

        // D-e: a cp950 Handler ID comes out as UTF-8 (the file name keeps the ANSI bytes)
        ra.sHandlerID = "\xA4\xA4";
        std::string path5;
        const bool w5 = ela::SaveSummary(ra, g_root + "\\O06cp", "", false, false, clk, &path5, &err);
        const std::string b5 = ReadFileBytes(path5);
        CHECK(w5 && b5.find("Handler ID:        \xE4\xB8\xAD\r\n") == std::string::npos,
              "3. (the Handler ID line comes from the query: still ELA-R2 -- the summary is not rebuilt by SaveSummary)");
        ela::BtnQuery(ra, [&]() { ela::QueryRequest x = q; x.handlerId = "\xA4\xA4"; return x; }());
        const bool w6 = ela::SaveSummary(ra, g_root + "\\O06cp", "", false, false, clk, &path5, &err);
        const std::string b6 = ReadFileBytes(path5);
        CHECK(w6 && b6.find("Handler ID:        \xE4\xB8\xAD\r\n") != std::string::npos &&
                  b6.find("\xA4\xA4") == std::string::npos,
              "3. D-e: a cp950 Handler ID (Gerneral.ini bytes) is written as UTF-8");
    }
    {
        // golden (keepGoldenBugs): SaveSummary appends to mmoSummary1 in place -- a second save repeats the detail
        ela::Analyzer gra(GoldenOptions(jam));
        ela::BtnQuery(gra, q);
        std::string g1, g2, err;
        ela::SaveSummary(gra, g_root + "\\O06g", "g1", true, false, clk, &g1, &err);
        ela::SaveSummary(gra, g_root + "\\O06g", "g2", true, false, clk, &g2, &err);
        bool c1 = false, c2 = false;
        const std::vector<std::string> G1 = ReportLines(ReadFileBytes(g1), &c1);
        const std::vector<std::string> G2 = ReportLines(ReadFileBytes(g2), &c2);
        bool prefix = !G1.empty() && G2.size() == G1.size() + 26;
        for (size_t i = 0; prefix && i < G1.size(); ++i) if (G2[i] != G1[i]) prefix = false;
        CHECK(prefix && G2[G1.size()] == "" && G2[G1.size() + 1] == kHourHead,
              "3. golden mode: the second save = the first + the detail again (golden mmoSummary1 in place)");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 4. DoVTestSaveSummary (golden :3240-3261, EL_VTEST_MTBF_SUM)
    // ---------------------------------------------------------------------------------------------------------
    {
        ela::ElaConfig c;
        c.edtO19_6 = g_root + "\\MTBF_Summary";                     // golden default D:\MTBF_Summary -- never here
        const FakeClock vclk(ela::EncodeDate(2025, 9, 17) + ela::EncodeTime(0, 0, 4, 0));
        ela::Analyzer va(on);
        const ela::ReportResult r = ela::DoVTestSaveSummary(va, c, q, vclk);
        CHECK(r.queried && r.prodOk && r.ran && r.written && r.path == c.edtO19_6 + "\\ELA-R2-SummaryData_2025-09-17.txt" &&
                  r.note.compare(0, 6, "wrote ") == 0,
              "4. VTEST: <asO19_SavePath>\\<ID>-SummaryData_<today>.txt, the folder made on the way");
        CHECK(va.dtStart == ela::EncodeDate(2025, 9, 10) && va.iDate == 7 &&
                  std::fabs(va.dtEnd - (ela::EncodeDate(2025, 9, 16) + e235959)) < 1e-9 && va.sgByDay.size() == 7,
              "4. range Now()-7 00:00:00 .. Now()-1 23:59:59 (7 days)");
        bool crlf = false;
        const std::vector<std::string> L = ReportLines(ReadFileBytes(r.path), &crlf);
        CHECK(crlf && L == va.mmoSummary && L.size() == 20, "4. bDetail=false: the 20 summary lines only");
        CHECK(SummaryInt(L, "MES Count:         ") == 123 && SummaryInt(L, "Total Input Qty:   ") == 265,
              "4. the 7 days hold the one fixture day (MES 123, input 265)");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 5. R5 call point: Timer2Due / Timer2Run (golden Timer2Timer :3010-3088, O06SaveSummaryData, N10SaveSummaryData)
    // ---------------------------------------------------------------------------------------------------------
    {
        ela::ElaConfig t;
        CHECK(!ela::Timer2Due(t, 0, 0, false) && !ela::Timer2Due(t, 0, 0, true), "5. O06-4 and N10-3 off: never");
        t.chkO06Production = true;
        // AI(W906-ELA-W43) 20260928 (St02-E helper): ★W43 = C -- without [O06-8] no minute of the day (W18 B was hourly)
        bool never = true;
        for (int hh = 0; hh < 24; ++hh)
            for (int nn = 0; nn < 60; ++nn)
                never = never && !ela::Timer2Due(t, hh, nn, false);
        CHECK(never, "5. ★W43 = C: O06-4 without [O06-8] (EnanleTimePeriodSaveLog): no save in any of the 1440 minutes");
        CHECK(ela::Timer2Due(t, 13, 1, true) && ela::Timer2Due(t, 13, 37, true), "5. golden (keepGoldenBugs): every minute");
        t.chkO06TimePeriod = true;
        t.cbO06TimePeriod = 0;
        const bool p10 = ela::Timer2Due(t, 1, 10, false) && ela::Timer2Due(t, 1, 20, false) && !ela::Timer2Due(t, 1, 5, false);
        t.cbO06TimePeriod = 1;
        const bool p30 = ela::Timer2Due(t, 1, 30, false) && !ela::Timer2Due(t, 1, 10, false);
        t.cbO06TimePeriod = 7;
        const bool pBad = ela::Timer2Due(t, 1, 30, false) && !ela::Timer2Due(t, 1, 10, false);
        CHECK(p10 && p30 && pBad, "5. TimePeriodSaveLog 0 = 10 min, 1 = 30 min, a bad index = 30 (TComboBox -1)");
        ela::ElaConfig nt;
        nt.cbN10_3 = true;
        nt.rgN10_3_1 = 0;
        const bool m0 = ela::Timer2Due(nt, 0, 0, false) && !ela::Timer2Due(nt, 1, 0, false) && !ela::Timer2Due(nt, 0, 1, false);
        nt.rgN10_3_1 = 1;
        const bool m1 = ela::Timer2Due(nt, 8, 0, false) && ela::Timer2Due(nt, 20, 0, false) &&
                        !ela::Timer2Due(nt, 9, 0, false) && !ela::Timer2Due(nt, 0, 0, false);
        nt.rgN10_3_1 = 2;
        const bool m2 = ela::Timer2Due(nt, 9, 0, false) && !ela::Timer2Due(nt, 9, 1, false);
        nt.rgN10_3_1 = 3;
        const bool m3 = ela::Timer2Due(nt, 9, 0, false);
        nt.rgN10_3_1 = -5;
        const bool mNeg = !ela::Timer2Due(nt, 0, 0, false);
        CHECK(m0 && m1 && m2, "5. N10-3: 0 = 00:00, 1 = 08:00 / 20:00, 2 = every hour");
        CHECK(m3 && mNeg, "5. N10-3 radio group clamp: 3 (the Handler's 指定時間) = Per Hour, -5 = none");

        ela::ElaConfig tc;
        tc.chkO06Production = true;
        tc.edtO06Production = g_root + "\\T2_O06";
        tc.cbN10_3 = true;
        tc.rgN10_4 = 0;                                             // "FTP": golden only saves, to EventLogSummary
        const FakeClock tclk(d0910 + ela::EncodeTime(13, 5, 0, 0));
        ela::Analyzer ta(on);
        ela::ReportResult r1, r2;
        ela::Timer2Run(ta, tc, q, tclk, &r1, &r2);
        const std::string n10dir = logRoot + "\\EventLogSummary\\202509";
        CHECK(r1.written && r1.path == tc.edtO06Production + "\\ELA-R2-SummaryData_2025-09-10.txt",
              "5. Timer2Run: O06-4 wrote today's summary to AutoSaveProductionPath");
        CHECK(r2.written && r2.path == n10dir + "\\ELA-R2-SummaryData_2025-09-10.txt",
              "5. N10-3, method 0: <HT9045_Log>\\EventLogSummary\\yyyymm\\ (W906_HT9045LOG_ROOT)");
        CHECK(ReadFileBytes(r1.path) == report3 && ReadFileBytes(r2.path) == report3,
              "5. both = section 3's report (today 00:00:00 .. 23:59:59, re-queried and regenerated)");
        tc.rgN10_4 = 1;
        tc.edtN10_8 = g_root + "\\T2_NET";
        ela::Timer2Run(ta, tc, q, tclk, &r1, &r2);
        const bool net1 = r2.written && r2.path == tc.edtN10_8 + "\\ELA-R2-SummaryData_2025-09-10.txt";
        ::DeleteFileA(r2.path.c_str());
        tc.rgN10_4 = 5;                                             // radio group clamp: Net Drive
        ela::Timer2Run(ta, tc, q, tclk, &r1, &r2);
        CHECK(net1 && r2.written && r2.path == tc.edtN10_8 + "\\ELA-R2-SummaryData_2025-09-10.txt",
              "5. N10-4 = 1 (and 5, clamped): sN10UploadDrivePath");
        ela::ElaConfig off;
        off.edtO06Production = g_root + "\\T2_O06_off";
        off.edtN10_8 = g_root + "\\T2_NET_off";
        off.rgN10_4 = 1;
        ela::Timer2Run(ta, off, q, tclk, &r1, &r2);
        CHECK(!r1.ran && !r1.written && !r2.ran && IsDir(off.edtO06Production) && EmptyDir(off.edtO06Production) &&
                  IsDir(off.edtN10_8) && EmptyDir(off.edtN10_8),
              "5. both off: nothing saved, but the folders are made first (golden MyForceDirectories before the switch)");
        ela::ElaConfig noDir;
        noDir.chkO06Production = true;
        ela::Timer2Run(ta, noDir, q, tclk, &r1, &r2);
        CHECK(!r1.ran && r1.note.find("Directory value is NULL!") != std::string::npos,
              "5. an empty AutoSaveProductionPath: MyForceDirectories -1, not run");
    }

    // ---------------------------------------------------------------------------------------------------------
    // 6. the Hub job (ElaHub RunOnce: EL_VTEST_MTBF_SUM, HOLD lifted in R2; D-a O10 gate)
    // ---------------------------------------------------------------------------------------------------------
    {
        const std::string hub = g_root + "\\hub";
        const std::string mtbf = hub + "\\MTBF_Summary";
        CHECK(WriteBytes(hub + "\\config.ini", "[Event Log]\r\nEnableAutoSaveEventLog=1\r\nasO19_SavePath=" + mtbf + "\r\n") &&
                  WriteBytes(hub + "\\Gerneral.ini", "[System]\r\nCUSTOMER_CODE=915\r\n[Version]\r\nMachine ID=H9\r\n") &&
                  WriteBytes(hub + "\\config_off.ini", "[Event Log]\r\nEnableAutoSaveEventLog=0\r\nasO19_SavePath=" + mtbf + "_off\r\n"),
              "6. sandbox config.ini (O10 on / off) and Gerneral.ini");
        ela::HubPaths hp;
        hp.configIni = hub + "\\config.ini";
        hp.generalIni = hub + "\\Gerneral.ini";
        const FakeClock hclk(ela::EncodeDate(2025, 9, 17) + ela::EncodeTime(0, 0, 4, 0));
        ela::QueryRequest hq = q;
        hq.handlerId = "H9";
        {
            ela::Hub h(on, hp);
            h.SetClock(&hclk);
            h.Post(ela::EL_UPDATE_PARAMETER);
            const bool read = h.RunOnce();
            CHECK(read && h.Config().cbO06 && h.Config().edtO19_6 == mtbf, "6. ReadConfig: O10 on, asO19_SavePath in the sandbox");
            h.PostQuery(hq);                                        // the page state (and one pending page query)
            h.Post(ela::EL_VTEST_MTBF_SUM);
            CHECK(h.RunOnce(), "6. the raised job runs before the page query (golden order)");
            std::string js = h.SnapshotJson(0);
            CHECK(Exists(mtbf + "\\H9-SummaryData_2025-09-17.txt") &&
                      js.find("EL_VTEST_MTBF_SUM: wrote ") != std::string::npos,
                  "6. EL_VTEST_MTBF_SUM: the MTBF summary written, the job history says so");
            const std::vector<ela::JobRecord> jobs = h.Jobs();
            CHECK(!jobs.empty() && jobs.back().cmd == "EL_VTEST_MTBF_SUM" && jobs.back().ran && jobs.back().written &&
                      jobs.back().path == mtbf + "\\H9-SummaryData_2025-09-17.txt" && jobs.back().hasRange &&
                      jobs.back().from == ela::EncodeDate(2025, 9, 10),
                  "6. the job record: ran, written, its file and its range (from 2025-09-10)");
            CHECK(js.find("\"cmd\":\"EL_VTEST_MTBF_SUM\",\"ran\":true,\"written\":true") != std::string::npos &&
                      js.find("\"range\":[\"2025/09/10 00:00:00\",\"2025/09/16 23:59:59\"]") != std::string::npos,
                  "6. SnapshotJson \"jobs\" carries the record and the job's 7-day range");
            CHECK(js.find("\"result\":null") != std::string::npos,
                  "6. the page's snapshot is NOT replaced by the job (St02-E 20260927; golden's window showed it)");
            CHECK(h.RunOnce(), "6. then the pending page query");
            js = h.SnapshotJson(0);
            CHECK(js.find("\"result\":{") != std::string::npos &&
                      js.find("\"range\":[\"2025/09/10 00:00:00\",\"2025/09/10 23:59:59\"]") != std::string::npos,
                  "6. the page's own query is the snapshot (2025-09-10 only)");
            // AI(W906-ELA-W22) 20260928 (St02-E): nothing is HOLD any more -- R4 took JAMWEEK (this check still expected
            // its HOLD after the R4 merge: stale), W22 SUMMARY / EVENTLOG.  On 915 the D-f gate skips all three.
            h.Post(ela::EL_UPLOAD_JAMWEEK);
            h.Post(ela::EL_UPLOAD_SUMMARY);
            h.Post(ela::EL_UPLOAD_EVENTLOG);
            h.RunOnce();
            h.RunOnce();
            h.RunOnce();
            const std::string ju = h.SnapshotJson(0);
            CHECK(ju.find("EL_UPLOAD_JAMWEEK: skipped, CUSTOMER_CODE 915") != std::string::npos &&
                      ju.find("EL_UPLOAD_SUMMARY: skipped, CUSTOMER_CODE 915") != std::string::npos &&
                      ju.find("EL_UPLOAD_EVENTLOG: skipped, CUSTOMER_CODE 915") != std::string::npos &&
                      ju.find("not ported (HOLD") == std::string::npos,
                  "6. the upload jobs run their D-f gate (915 is not 851): skipped, nothing HOLD any more (R4 / W22)");
        }
        {
            ela::HubPaths hp2 = hp;
            hp2.configIni = hub + "\\config_off.ini";
            ela::Hub h(on, hp2);
            h.SetClock(&hclk);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            h.PostQuery(hq);
            h.Post(ela::EL_VTEST_MTBF_SUM);
            h.RunOnce();
            CHECK(!h.Config().cbO06 && !Exists(mtbf + "_off") &&
                      h.SnapshotJson(0).find("EL_VTEST_MTBF_SUM: skipped") != std::string::npos &&
                      !h.Jobs().empty() && !h.Jobs().back().ran && !h.Jobs().back().written,
                  "6. O10 off: skipped, nothing written, the job record says not run (D-a: golden runs no analyzer then)");
        }
    }

    // ---------------------------------------------------------------------------------------------------------
    // 7. the oracle (BCB EventlogAnalyzer.exe output; tests/fixtures/ela/oracle/README.txt).  Missing = SKIP.
    // ---------------------------------------------------------------------------------------------------------
    {
        const std::string odir = std::string(W906_ELA_FIXTURE_DIR) + "\\oracle";
        const std::string oDetail = FindOne(odir, "*-SummaryData_r2detail.txt");
        const std::string oAlarm = FindOne(odir, "*-SummaryData_r2alarm.txt");
        if (oDetail.empty() && oAlarm.empty())
            SKIP("7. oracle missing: tests\\fixtures\\ela\\oracle\\<ID>-SummaryData_r2detail.txt / _r2alarm.txt (BCB exe run, README.txt)");
        else
        {
            const std::string ev7 = g_root + "\\oracle\\EventLogTxt";
            const std::string prod7 = g_root + "\\oracle\\Production_Log";
            const std::string jam7 = g_root + "\\oracle\\JAM0000.dat";
            bool okc = CopyFixture("EventLogTxt\\2025\\09\\HT-9016C_PMLD1019_EventLogTxt_20250910.csv", g_root + "\\oracle") &&
                       CopyFixture("Production_Log\\202509\\PMLD1019_20250910.csv", g_root + "\\oracle");
            bool jok = false;
            const std::string jbytes = ReadBytes(odir + "\\JAM0000.dat", &jok);   // the exe's JAM0000.dat after its run
            okc = okc && WriteBytes(jam7, jok ? jbytes : std::string());
            CHECK(okc, "7. oracle inputs copied into the sandbox");
            if (!oDetail.empty()) CompareOracle(oDetail, false, ev7, prod7, jam7);
            else SKIP("7. oracle r2detail missing");
            if (!oAlarm.empty()) CompareOracle(oAlarm, true, ev7, prod7, jam7);
            else SKIP("7. oracle r2alarm missing");
        }
    }

    // ---------------------------------------------------------------------------------------------------------
    // 8. R3: N34 ChipAdvanced (golden uChipAdvancedFunc.cpp).  AI(W906-ELA-R3) 20260927 (St02-E)
    // ---------------------------------------------------------------------------------------------------------
    {
        CHECK(ela::ChipAdvHourMinSecStr(0) == "00:00:0.00" && ela::ChipAdvHourMinSecStr(ela::EncodeTime(2, 0, 0, 0)) == "02:00:0.00" &&
                  ela::ChipAdvHourMinSecStr(1.5 + ela::EncodeTime(0, 0, 7, 250)) == "36:00:7.25" &&
                  ela::ChipAdvHourMinSecStr(ela::ChipAdvDaybySec(40)) == "00:00:40.00",
              "8. GetHourMinSecStr: hours run past 24, seconds are %02.2f (no zero pad below 10 s)");
        CHECK(ela::ChipAdvDateTimeStr(ela::EncodeDate(2026, 4, 10) + ela::EncodeTime(8, 0, 0, 0)) == "2026/04/10 08:00:00" &&
                  ela::ChipAdvDaybySec(86400) == 1.0, "8. GetDateTimeStr / GetDaybySec");
        const std::string lotMsg = "Lot Start, Lot ID:LOT123, OP ID:OP9, Run Mode:Normal";   // uLotInfo.cpp:1609 shape
        CHECK(ela::ChipAdvParseField(lotMsg, "Lot ID:", false) == "LOT123" && ela::ChipAdvParseField(lotMsg, "OP ID:", false) == "OP9" &&
                  ela::ChipAdvParseField(lotMsg, "Run Mode:", false) == "Normal",
              "8. ParseField, W18 B: the Handler's \"Lot ID:%s\" (no blank) keeps its first character");
        CHECK(ela::ChipAdvParseField(lotMsg, "Lot ID:", true) == "OT123" && ela::ChipAdvParseField(lotMsg, "OP ID:", true) == "P9" &&
                  ela::ChipAdvParseField(lotMsg, "Run Mode:", true) == "ormal",
              "8. ParseField, golden (keepGoldenBugs): pos+fieldLen+1 drops the first character");
        CHECK(ela::ChipAdvParseField("Lot ID: ABC, x", "Lot ID:", false) == "ABC" && ela::ChipAdvParseField("Lot ID: ABC, x", "Lot ID:", true) == "ABC" &&
                  ela::ChipAdvParseField("x", "Lot ID:", false) == "" && ela::ChipAdvParseField("Lot ID:;;X,y", "Lot ID:", false) == "X" &&
                  ela::ChipAdvParseField("Lot ID:1, OP ID:1", "Lot ID:", true) == "",
              "8. ParseField: a blank is skipped in both modes; no field = \"\"; leading ';' go; golden turns a 1-char ID into \"\"");

        // a made-up lot: every number below is worked out by hand (JAM0000.dat empty = every look-up takes its default:
        //   JAM0101 "01 Input Arm" counts for MTBA, WAR2401 "24 Motor" for MTBF, MES1640 for neither)
        const std::string cdir = g_root + "\\chipadv";
        const std::string cev = cdir + "\\EventLogTxt";
        const std::string cprod = cdir + "\\Production_Log";
        const std::string n34 = cdir + "\\OEEAlarmRpt";
        const std::string jam8 = cdir + "\\JAM0000.dat";
        std::string prod = "Schedule name, Start time, Input tray\r\n";
        const char* const inTimes[3] = { "2026/04/10 08:30:00", "2026/04/10 09:30:00", "2026/04/10 10:30:00" };
        for (int i = 0; i < 3; ++i)
        {
            std::string row;
            for (int k = 0; k < 28; ++k)
            {
                if (k) row += ",";
                if (k == 5) row += inTimes[i];                          // eLoadTime
                else if (k == 13) row += Itos(i + 1);                   // eOrderTest
                else if (k == 27) row += "00:00:01.500";                // eTestTime
                else row += "x";
            }
            prod += row + "\r\n";
        }
        const bool made = WriteBytes(jam8, "") &&
            WriteBytes(cev + "\\2026\\04\\EventLogTxt_20260410.csv",
                "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n"
                "2026/04/10,07:00:00.000,Process,MES2110,\"\t\",\"\t\",\"\t\",\"START pressed\",ScanKey_2\r\n"
                "2026/04/10,08:00:00.000,Process,\"\t\",\"\t\",\"\t\",\"\t\",\"Lot Start, Lot ID:LOT123, OP ID:OP9, Run Mode:Normal\",\r\n"
                "2026/04/10,08:10:00.000,\"01 Input Arm\",JAM0101,RETRY,30,0,\"Pick fail\",,R1\r\n"
                "2026/04/10,08:20:00.000,\"24 Motor\",WAR2401,RETRY,20,0,\"Motor alarm\",,R1\r\n"
                "2026/04/10,09:00:00.000,\"01 Input Arm\",JAM0101,RETRY,10,0,\"Pick fail\",,R1\r\n"
                "2026/04/10,09:30:00.000,\"16 System\",MES1640,,0,0,\"One cycle finish\",,R1\r\n") &&
            WriteBytes(cprod + "\\202604\\ChipAdvProd_20260410.csv", prod);
        CHECK(made, "8. made-up EventLogTxt / Production_Log / empty JAM0000.dat in the sandbox");
        ela::Options o8;
        o8.jamIniPath = jam8;
        ela::ElaConfig c8;
        c8.edN34 = n34;                                             // golden default D:\HT9045_Log\Product_Loader\OEEAlarmRpt
        ela::QueryRequest ui8;
        ui8.eventLogDir = cev;
        ui8.prodLogDir = cprod;
        ui8.handlerId = "ELA-R3";
        const FakeClock now8(ela::EncodeDate(2026, 4, 10) + ela::EncodeTime(12, 0, 0, 0));
        const std::string want8 =
            "Name,Value,Unit\n"
            "MTBA,02:00:0.00,[s]\n"                                  // 4 h / 2 alarms
            "MUBA,1,[cmp]\n"                                         // 3 / 2
            "MTBF,04:00:0.00,[s]\n"                                  // 4 h / 1 fail
            "MUBF,3,[cmp]\n"
            "UPH,0,[C/H]\n"                                          // 3 / (1 day * 24)
            "UPH Auto,0,[C/H]\n"
            "Production Quantity,3,[cmp]\n"
            "\n"
            "Name,Quantity,Time\n"
            "Start Time,,2026/04/10 08:00:00\n"
            "Stop Time,,2026/04/10 12:00:00\n"
            "Analysis Time,,04:00:0.00\n"
            "Production Time,,03:59:0.00\n"                          // 4 h - 60 s stopped
            "Operation Time,,00:00:0.00\n"
            "Alarm Time,,00:00:40.00\n"
            "Prompt Time,,00:00:0.00\n"
            "Downtime Time,,00:01:0.00\n"
            "OEE,,66.67%\n"                                          // (3 - 1) / 3
            "\n"
            "Alarm Name,Total Alarm Count,Alarm Count Ratio,Total Alarm Time,Alarm Time Ratio\n"
            "JAM0101,2,50.00%,40,66.67%\n"
            "WAR2401,1,25.00%,20,33.33%\n"
            "MES1640,1,25.00%,0,0.00%\n"
            "\n";                                                     // WriteDataToFile's own "\n"
        const std::string clip = n34 + "\\ClipAdv_LOT123_2026410.csv";
        {
            ela::Analyzer a8(o8);
            ela::ChipAdvancedFunc chip;
            const ela::ReportResult r8 = chip.AnalysisLog(a8, c8, ui8, now8);
            CHECK(r8.queried && r8.prodOk && r8.ran && r8.written && r8.path == clip && Exists(clip),
                  "8. AnalysisLog: <sN34_OEEAlarmRptPath>\\ClipAdv_<LotID>_<y><m><d>.csv (golden %d%d%d, not padded)");
            CHECK(chip.rptStruct.sLotID == "LOT123" && chip.rptStruct.sOPID == "OP9" && chip.rptStruct.sRunMode == "Normal" &&
                      chip.rptStruct.iDate == 1 && chip.rptStruct.summary.iInputCount == 3 &&
                      chip.rptStruct.summary.iStopCount == 4 && chip.rptStruct.summary.iFailCount == 1,
                  "8. the LAST Lot Start (08:00) .. Now(): lot / OP / run mode, 1 day, 3 inputs, 4 stops, 1 fail");
            CHECK(chip.anomalyVec.size() == 3 && chip.report + "\n" == want8 && ReadFileBytes(clip) == want8,
                  "8. the report bytes: metrics, OEE, alarm table (LF lines; D-e ASCII = UTF-8)");
            CHECK(r8.hasRange && r8.rangeFrom == chip.rptStruct.dtStart && r8.rangeTo == now8.Now(), "8. the job's range = the lot");
            ela::ChipAdvancedFunc again;
            again.AnalysisLog(a8, c8, ui8, now8);
            CHECK(ReadFileBytes(clip) == want8, "8. a second lot end regenerates the file (golden appends -- W21 / §3.3, D-b)");
        }
        {
            ela::Options og8 = o8;
            og8.keepGoldenBugs = true;
            ela::Analyzer ga8(og8);
            ela::ChipAdvancedFunc gchip;
            const ela::ReportResult gr = gchip.AnalysisLog(ga8, c8, ui8, now8);
            CHECK(gr.written && gr.path == n34 + "\\ClipAdv_OT123_2026410.csv" && ReadFileBytes(gr.path) == want8,
                  "8. golden mode: the Lot ID loses its first character (ClipAdv_OT123_...), the same report");
        }
        {
            ela::Analyzer a0(o8);
            ela::QueryRequest ui0 = ui8;
            ui0.eventLogDir = cdir + "\\no_rows";
            MkDirs(ui0.eventLogDir);
            ela::ChipAdvancedFunc c0;
            const ela::ReportResult r0 = c0.AnalysisLog(a0, c8, ui0, now8);
            CHECK(!r0.queried && !r0.ran && !r0.written && c0.logRecords.empty() && r0.note.find("D-c") != std::string::npos,
                  "8. D-c: no row in 8 days -- no report and no crash (golden's size_t i=size-1 wrapped)");
            ui0.eventLogDir = cdir + "\\first_row";
            WriteBytes(ui0.eventLogDir + "\\2026\\04\\EventLogTxt_20260410.csv",
                "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n"
                "2026/04/10,08:00:00.000,Process,\"\t\",\"\t\",\"\t\",\"\t\",\"Lot Start, Lot ID:LOT9, OP ID:OP9, Run Mode:Normal\",\r\n"
                "2026/04/10,08:10:00.000,\"01 Input Arm\",JAM0101,RETRY,30,0,\"Pick fail\",,R1\r\n");
            ela::ChipAdvancedFunc c1;
            const ela::ReportResult r1 = c1.AnalysisLog(a0, c8, ui0, now8);
            CHECK(!r1.queried && c1.logRecords.size() == 2 && r1.note.find("no \"Lot Start") != std::string::npos &&
                      !Exists(n34 + "\\ClipAdv_LOT9_2026410.csv"),
                  "8. golden kept: a Lot Start that is the first row of the 8 days is not found (index 0 never looked at)");
        }
        {
            // W23 sample 2025-03-30 (HT-9045HA E3200-0390), trimmed: header + lines 3900-4050 around its last Lot Start
            const std::string rdir = g_root + "\\chipadv_real";
            CHECK(CopyFixture("ChipAdv\\EventLogTxt\\2025\\03\\HT-9045HA_E3200-0390_EventLogTxt_20250330.csv", rdir),
                  "8. W23 ChipAdv fixture copied");
            ela::QueryRequest uiR = ui8;
            uiR.eventLogDir = rdir + "\\ChipAdv\\EventLogTxt";
            uiR.prodLogDir = rdir + "\\Production_Log";                 // none: 0 input
            MkDirs(uiR.prodLogDir);
            const FakeClock nowR(ela::EncodeDate(2025, 3, 31) + ela::EncodeTime(0, 30, 0, 0));
            ela::Analyzer aR(o8);
            ela::ChipAdvancedFunc rc;
            const ela::ReportResult rr = rc.AnalysisLog(aR, c8, uiR, nowR);
            CHECK(rr.written && rr.path == n34 + "\\ClipAdv_AFYD09N370-D001_2025331.csv" &&
                      rc.rptStruct.sOPID == "00072015" && rc.rptStruct.sRunMode == "EQC",
                  "8. W23 sample: the last Lot Start 23:27:28 (lot AFYD09N370-D001, OP 00072015, EQC)");
            int cnt = 0, tim = 0;
            for (size_t i = 0; i < rc.anomalyVec.size(); ++i) { cnt += rc.anomalyVec[i].count; tim += rc.anomalyVec[i].totalTime; }
            CHECK(rc.anomalyVec.size() == 15 && cnt == 39 && tim == 336 && rc.anomalyVec[0].sJamName == "MES2110" &&
                      rc.anomalyVec[0].count == 4,
                  "8. W23 sample: 15 codes, 39 events, 336 s after the Lot Start, first MES2110 x4 (Python mirror)");
            const std::string rep8 = ReadFileBytes(rr.path);
            CHECK(rep8.find("Start Time,,2025/03/30 23:27:28\n") != std::string::npos &&
                      rep8.find("Stop Time,,2025/03/31 00:30:00\n") != std::string::npos &&
                      rep8.find("Production Quantity,0,[cmp]\n") != std::string::npos && rep8.find("OEE,,0.00%\n") != std::string::npos,
                  "8. W23 sample: the lot's range, no Production_Log -> 0 input, OEE 0.00%");
        }
        {
            // the Hub job (EL_UPLOAD_CHIPADV_LOTEND, HOLD lifted in R3), O10 on / off
            const std::string hub8 = g_root + "\\hub8";
            CHECK(WriteBytes(hub8 + "\\config.ini", "[Event Log]\r\nEnableAutoSaveEventLog=1\r\n[N34 Function]\r\nsN34_OEEAlarmRptPath=" +
                                                        hub8 + "\\N34\r\n") &&
                      WriteBytes(hub8 + "\\config_off.ini", "[Event Log]\r\nEnableAutoSaveEventLog=0\r\n[N34 Function]\r\nsN34_OEEAlarmRptPath=" +
                                                                hub8 + "\\N34_off\r\n") &&
                      WriteBytes(hub8 + "\\Gerneral.ini", "[System]\r\nCUSTOMER_CODE=868\r\n[Version]\r\nMachine ID=C8\r\n"),
                  "8. sandbox config.ini (O10 on / off, sN34_OEEAlarmRptPath) and Gerneral.ini");
            ela::HubPaths hp8;
            hp8.configIni = hub8 + "\\config.ini";
            hp8.generalIni = hub8 + "\\Gerneral.ini";
            {
                ela::Hub h(o8, hp8);
                h.SetClock(&now8);
                h.Post(ela::EL_UPDATE_PARAMETER);
                h.RunOnce();
                h.PostQuery(ui8);
                h.Post(ela::EL_UPLOAD_CHIPADV_LOTEND);
                CHECK(h.RunOnce(), "8. Hub: EL_UPLOAD_CHIPADV_LOTEND runs (before the page query)");
                const std::vector<ela::JobRecord> jobs = h.Jobs();
                CHECK(!jobs.empty() && jobs.back().cmd == "EL_UPLOAD_CHIPADV_LOTEND" && jobs.back().written &&
                          jobs.back().path == hub8 + "\\N34\\ClipAdv_LOT123_2026410.csv" &&
                          ReadFileBytes(jobs.back().path) == want8 && h.SnapshotJson(0).find("\"result\":null") != std::string::npos,
                      "8. Hub: the ClipAdv file written from ReadConfig's sN34_OEEAlarmRptPath, a job record, the page snapshot untouched");
            }
            {
                ela::HubPaths hp9 = hp8;
                hp9.configIni = hub8 + "\\config_off.ini";
                ela::Hub h(o8, hp9);
                h.SetClock(&now8);
                h.Post(ela::EL_UPDATE_PARAMETER);
                h.RunOnce();
                h.PostQuery(ui8);
                h.Post(ela::EL_UPLOAD_CHIPADV_LOTEND);
                h.RunOnce();
                CHECK(!Exists(hub8 + "\\N34_off") && !h.Jobs().empty() && !h.Jobs().back().ran &&
                          h.SnapshotJson(0).find("EL_UPLOAD_CHIPADV_LOTEND: skipped") != std::string::npos,
                      "8. Hub: O10 off -- skipped, nothing written (D-a)");
            }
        }
    }

    printf("ELA_Reports: %d passed, %d failed, %d skipped\n", g_pass, g_fail, g_skip);
    if (g_fail == 0)
        RemoveTree(g_root);
    else
        printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
