// =============================================================================
//  test_ela_schedule.cpp -- ELA plan R5: ElaSchedule (time-based ELA jobs, stagger, back-off, gates, boot catch-up).
//
//  AI(W906-ELA-R5) 20260927 (St02-E).  Suite name (add_test): ELA_Schedule.  A fake clock, no sleeping.
//
//    0. containment: the sandbox %TEMP%\ht9045_ela_sched_<tick> (refused under D:\HT9045*), the seams
//       W906_HT9045LOG_ROOT / W906_GENERAL_INI_PATH / W906_PRODLOG_ROOT pointed into it, ScheduleSetup follows them;
//    1. time helpers (TDateTime epoch, weekday, format / parse);
//    2. FNV-1a-32 vectors, stagger offsets per Machine ID, back-off bounds / values / determinism;
//    3. gates: O10 (D-a), function switches (D-f), customer codes 851 / 868 / 915 / 919, the SIM rule, manual;
//    4. job kinds and stagger windows (UNC / network drive / [O06-6] Use Net Drive / local);
//    5. calendars (every N min, 08:00+20:00, weekly, golden O06 / N10 coupling);
//    6. O06-4 / N10-3 once per slot (golden every minute + coupling vs W18 B);
//    7. N25-3 at midnight + FTP stagger; SIM build none;
//    8. VTEST once a week (edge), 915 / 919 only;
//    9. W13 N17 UploadProdLog at 01:00 (real copy in the sandbox), CopyFileAndVerify cases, failed-then-verified;
//   10. O10 off: nothing automatic (N17 still runs: a Handler job in golden);
//   11. D-b boot catch-up after a simulated downtime (state file), none without a record, only the latest slot;
//   12. back-off: failed-then-verified timing, give up after 1 + 6 attempts, cut by the next slot;
//   13. manual RunNow; no job body; FTP_Log file; the process-wide entry points;
//   14. ReadScheduleConfig from sandbox ini files (read only);
//   15. the real D:\HT9045\config\config.ini / D:\HT9045\system\Gerneral.ini / state file are untouched;
//   16. the hub adapter (merge-time bodies): R2's O06 on a %TEMP% Hub writes a verified summary for the slot's day;
//       ScheduleUseHubJobs registers the six bodies (N17 keeps its own);
//   17. O10 has one source (ElaReports.h EffectiveO10, golden cprod.cpp:2379-2394): the formula with the function flag on
//       (O06-1 decides) and off (the stored O10 decides), this machine's EnableAutoSaveEventLog=0 / bO10UseEventLogSaver=1,
//       R2 / R3 / R4 AutoJobsEnabled and the schedule gate agree; an installed getter wins and is read at every decision;
//   18. R6: the per-job status eventlog.html shows (next run, retries, last run / result / detail, the closed gate) and the
//       two routes GET /api/ela/schedule and POST /api/ela/job (403 / 400 / 202 / 503; the queued run is manual).
//   19. ★W44 B (AI(W906-ELA-W44) 20260928, St02-E helper): N10-3 method 3 once a day at the Handler's specified time
//       (golden 906_0625_Steven HS_Function.cpp:488-509) -- a fake clock over days: not before, once, the next day again;
//       a restart the same day; a missed time caught up ONCE at boot (★W44-2, AI(W906-ELA-W44B) 20260928; golden has no
//       catch-up), only the latest of several, none without a record; a boot inside the minute; the time changed while
//       running (re-based: a time already past is not run); ReadScheduleConfig's [FTPUpLoad] dN10_3_1_SpecifiedTime.
//   20. ★W43 = C (AI(W906-ELA-W43) 20260928, St02-E helper): O06-4 follows the Handler's [O06-8] -- a fake clock over a
//       day: unchecked = no timed save, 10 min = 144, 30 min = 48; checked / unchecked while running; a manual run.
//  0 refuses (exit 2) before any scheduler when a seam did not take (containment first, as test_tcp_cmd_server.cpp).
//  W22 (AI(W906-ELA-W22) 20260928, St02-E): N25-4 / N25-5 in 3 (gates, SIM rule), 4 (ftp, 900 s), 5 (daily 00:00),
//       7 (N25-3 / N25-4 / N25-5 in one slot, golden order), 14 (the two switches), 16 (seven bodies).
//  The sandbox is removed on a green run (kept for a look when something fails).
// =============================================================================
#include "EventLogAnalysis/ElaSchedule.h"
#include "EventLogAnalysis/ElaHub.h"
#include "EventLogAnalysis/ElaReports.h"   // EffectiveO10 / AutoJobsEnabled (section 17), RunO06OnHub's R2 body
#include "EventLogAnalysis/ElaService.h"   // ela::ServeHttp -- the R6 routes (section 18)
#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

using ela::LocalSec;

static std::string g_root;

static void PutEnv(const std::string& nameValue)
{
    static std::string keep[16];
    static int n = 0;
    keep[n % 16] = nameValue;
    HT9045_TEST_PUTENV(keep[n % 16].c_str());
    ++n;
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

static void WriteText(const std::string& p, const std::string& text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fwrite(text.data(), 1, text.size(), f); std::fclose(f); }
}

static std::string ReadText(const std::string& p)
{
    std::string o;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return o;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) o.append(b, n);
    std::fclose(f);
    return o;
}

static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

static bool StartsWithNoCase(const std::string& s, const char* prefix)
{
    for (size_t i = 0; prefix[i]; ++i)
        if (i >= s.size() || std::tolower((unsigned char)s[i]) != std::tolower((unsigned char)prefix[i])) return false;
    return true;
}

struct Stamp
{
    bool exists;
    unsigned long long size, mtime;
};
static Stamp StampOf(const std::string& p)
{
    Stamp s = { false, 0, 0 };
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &fa))
    {
        s.exists = true;
        s.size = ((unsigned long long)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
        s.mtime = ((unsigned long long)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b)
{
    return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime;
}

// ---------------------------------------------------------------------------
struct FakeClock : public ela::IClock
{
    LocalSec t;
    FakeClock() : t(0) {}
    LocalSec Now() { return t; }
};

static std::vector<std::string> g_logs;
static void CaptureSink(LocalSec, const std::string& m) { g_logs.push_back(m); }
static int CountLogs(const char* a, const char* b = 0)
{
    int n = 0;
    for (size_t i = 0; i < g_logs.size(); ++i)
        if (Has(g_logs[i], a) && (!b || Has(g_logs[i], b))) ++n;
    return n;
}

static int FakeDriveType(const std::string& root) { return (root == "Z:\\" || root == "z:\\") ? DRIVE_REMOTE : DRIVE_FIXED; }

struct Rec
{
    int job;
    LocalSec slot, at;
    int attempt;
    bool catchUp, manual;
};
struct Recorder
{
    FakeClock* clock;
    std::vector<Rec> recs;
    std::vector<ela::JobStatus> script;   // per call; past the end: dflt
    ela::JobStatus dflt;
    explicit Recorder(FakeClock* c) : clock(c), dflt(ela::JOB_VERIFIED) {}
};
static ela::JobFn Body(Recorder* r)
{
    return [r](ela::JobContext& c) {
        Rec x = { c.job, c.slot, r->clock->t, c.attempt, c.catchUp, c.manual };
        r->recs.push_back(x);
        const size_t k = r->recs.size() - 1;
        c.detail = "test body";
        return k < r->script.size() ? r->script[k] : r->dflt;
    };
}

static LocalSec T(int y, int m, int d, int hh, int nn, int ss) { return ela::LocalSecFromCivil(y, m, d, hh, nn, ss); }

static ela::ScheduleConfig Cfg()
{
    ela::ScheduleConfig c;
    c.o06_1 = true;
    c.custCode = "000";
    c.n10Daily = false;                 // the analyzer's default is true; each test turns on what it needs
    c.machineId = "PJLD1014";
    c.hostName = "HOSTX";
    c.o06Path = g_root + "\\RMS";
    c.n10DrivePath = g_root + "\\RMS";
    c.o19Path = g_root + "\\MTBF_Summary";
    c.n17Path = g_root + "\\RMS";
    c.prodLogRoot = g_root + "\\Production_Log";
    c.machineType = "HT-9045";
    c.socketHandlerId = "PJLD1014";
    c.pcName = "HOSTX";
    return c;
}

static ela::ScheduleSetup Setup(FakeClock* clk, ela::ScheduleConfig* cfg, const std::string& stateFile)
{
    ela::ScheduleSetup s;
    s.clock = clk;
    s.readConfig = [cfg](ela::ScheduleConfig* o) { *o = *cfg; return true; };
    s.configIni = g_root + "\\no_such_config.ini";
    s.generalIni = g_root + "\\no_such_general.ini";
    s.logRoot = g_root + "\\log";
    s.stateFile = stateFile;
    s.hostName = "HOSTX";
    s.configEverySec = 60;
    s.keepGoldenBugs = false;
    s.simBuild = false;
    s.driveType = &FakeDriveType;
    s.logSink = &CaptureSink;
    return s;
}

// AI(W906-ELA-SPEED) 20261001 (St02-E) (ported from the St02-E helper's 3f93430f, 0929; St02-E2 ST02_ELA_SCHEDULE_TIMEOUT_20261001.md): St01's SHIP
//   gate timed this test out at 600 s.  Not a hang (the loops only advance the fake clock; nothing waits): ~615,000
//   Scheduler::Tick calls, each rebuilding the seven-job status JSON (Publish), ~0.45 ms each (276 s alone, 0929).  The
//   long spans now step as coarsely as each check allows (~80,000 ticks; 1 s kept through every exact-second
//   assertion), and every fake-clock loop checks a wall-clock budget, so the test stops itself with a FAIL line
//   (stdout unbuffered) well before ctest's timeout instead of being killed silently.  No assertion changed.
static DWORD g_t0 = 0;
static const DWORD kBudgetMs = 300000;
static void CheckBudget()
{
    const DWORD ms = ::GetTickCount() - g_t0;
    if (ms <= kBudgetMs) return;
    printf("  FAIL: wall-clock budget exceeded (%lu ms > %lu ms) -- a fake-clock loop ran too long; stopped, sandbox kept: %s\n",
           (unsigned long)ms, (unsigned long)kBudgetMs, g_root.c_str());
    std::fflush(stdout);
    std::exit(3);
}

static void Run(ela::Scheduler& s, FakeClock& c, LocalSec until, int step)
{
    if (step < 1) step = 1;   // a non-positive step would never reach `until`
    while (c.t <= until)
    {
        s.Tick();
        c.t += step;
        CheckBudget();
    }
}

static int CountJob(const Recorder& r, int job)
{
    int n = 0;
    for (size_t i = 0; i < r.recs.size(); ++i)
        if (r.recs[i].job == job) ++n;
    return n;
}

static std::vector<Rec> OfJob(const Recorder& r, int job)
{
    std::vector<Rec> o;
    for (size_t i = 0; i < r.recs.size(); ++i)
        if (r.recs[i].job == job) o.push_back(r.recs[i]);
    return o;
}

static std::string Sb(const char* name) { return g_root + "\\" + name; }

static bool g_liveO10 = true;                        // section 17: stands in for IniConfig.bO10UseEventLogSaver
static bool LiveO10() { return g_liveO10; }

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);   // AI(W906-ELA-SPEED) 20261001 (St02-E): every line reaches ctest's log as it is printed
    g_t0 = ::GetTickCount();                 //   and the wall-clock budget (CheckBudget) starts here
    printf("=== ELA_Schedule (EventLogAnalysis/ElaSchedule, ELA plan R5) ===\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_ela_sched_" + tick;
    if (!g_root.empty() && g_root[g_root.size() - 1] == '\\') g_root.erase(g_root.size() - 1);
    // 0. containment first: never under D:\HT9045 (the machine data) nor D:\HT9045_Log (the machine logs)
    if (StartsWithNoCase(g_root, "D:\\HT9045"))
    {
        printf("  FAIL: sandbox %s is under D:\\HT9045* -- refused\n", g_root.c_str());
        return 1;
    }
    ::CreateDirectoryA(g_root.c_str(), NULL);
    ::CreateDirectoryA(Sb("RMS").c_str(), NULL);
    ::CreateDirectoryA(Sb("Production_Log").c_str(), NULL);
    ::CreateDirectoryA(Sb("state").c_str(), NULL);

    const std::string realCfg = "D:\\HT9045\\config\\config.ini", realGen = "D:\\HT9045\\system\\Gerneral.ini",
                      realState = "D:\\HT9045_Log\\UploadFile\\ElaScheduleState.ini";
    const Stamp s0Cfg = StampOf(realCfg), s0Gen = StampOf(realGen), s0State = StampOf(realState);

    PutEnv("W906_HT9045LOG_ROOT=");
    PutEnv("W906_GENERAL_INI_PATH=");
    PutEnv("W906_AUTH_PATH=");
    {
        ela::ScheduleSetup g;
        CHECK(g.logRoot == "D:\\HT9045_Log" && g.generalIni == "D:\\HT9045\\system\\Gerneral.ini" &&
                  g.configIni == "D:\\HT9045\\config\\config.ini" && g.stateFile.empty() && g.configEverySec == 60 &&
                  !g.keepGoldenBugs && g.clock == 0 && g.logSink == 0,
              "0. seams unset: the golden literals (and W18 B, 60 s config period)");
#ifdef W906_NO_SOFT_SIMULTE
        CHECK(!g.simBuild, "0. ship build: simBuild false");
#else
        CHECK(g.simBuild, "0. SIM build: simBuild true (golden main.cpp:21425 rule for N25-3)");
#endif
    }
    PutEnv("W906_HT9045LOG_ROOT=" + Sb("log"));
    PutEnv("W906_GENERAL_INI_PATH=" + Sb("Gerneral.ini"));
    PutEnv("W906_PRODLOG_ROOT=" + Sb("Production_Log"));
    PutEnv("W906_AUTH_PATH=" + Sb("auth") + "\\");      // AI(W906-ELA-REV) 20260928: the Handler's AuthPath seam
    {
        ela::ScheduleSetup g;
        ela::ScheduleConfig c;
        CHECK(g.logRoot == Sb("log") && g.generalIni == Sb("Gerneral.ini") && c.prodLogRoot == Sb("Production_Log"),
              "0. ScheduleSetup / ScheduleConfig follow W906_HT9045LOG_ROOT / W906_GENERAL_INI_PATH / W906_PRODLOG_ROOT");
        CHECK(g.configIni == Sb("auth") + "\\config.ini", "0. ScheduleSetup::configIni follows W906_AUTH_PATH (<path>config.ini)");
        CHECK(!StartsWithNoCase(g.logRoot, "D:\\HT9045") && !StartsWithNoCase(c.prodLogRoot, "D:\\HT9045"),
              "0. every root the test uses is outside D:\\HT9045*");
        // AI(W906-ELA-W43) 20260928 (St02-E helper): containment first, as tests/test_tcp_cmd_server.cpp (b12ab375) -- a
        //   seam that did not take stops the run here, before any scheduler, so a run by hand never reaches the machine
        if (StartsWithNoCase(g.logRoot, "D:\\HT9045") || StartsWithNoCase(g.generalIni, "D:\\HT9045") ||
            StartsWithNoCase(g.configIni, "D:\\HT9045") || StartsWithNoCase(c.prodLogRoot, "D:\\HT9045"))
        {
            printf("  ABORT: a seam did not take (log %s, general %s, config %s, prodlog %s) -- nothing was run\n",
                   g.logRoot.c_str(), g.generalIni.c_str(), g.configIni.c_str(), c.prodLogRoot.c_str());
            return 2;
        }
    }

    // 1. time
    {
        CHECK(ela::LocalSecFromCivil(1899, 12, 30, 0, 0, 0) == 0, "1. 1899-12-30 00:00 = TDateTime 0");
        CHECK(T(2026, 9, 27, 0, 0, 0) == 46292LL * 86400, "1. 2026-09-27 = day 46292");
        CHECK(ela::DayOfWeek0(T(2026, 9, 27, 12, 0, 0)) == 0 && ela::DayOfWeek0(T(2026, 9, 28, 0, 0, 0)) == 1 &&
                  ela::DayOfWeek0(T(1899, 12, 30, 0, 0, 0)) == 6,
              "1. weekday: 2026-09-27 Sunday, 2026-09-28 Monday, 1899-12-30 Saturday");
        LocalSec p = 0;
        CHECK(ela::FormatLocalSec(T(2028, 2, 29, 23, 59, 58)) == "2028/02/29 23:59:58" &&
                  ela::ParseLocalSec("2028/02/29 23:59:58", &p) && p == T(2028, 2, 29, 23, 59, 58) &&
                  !ela::ParseLocalSec("2028/13/01 00:00:00", &p) && ela::FormatLocalSec(-1) == "-",
              "1. format / parse round trip (leap day), bad month refused");
        int y, m, d, hh, nn, ss;
        ela::CivilFromLocalSec(T(2026, 12, 31, 23, 59, 59) + 1, &y, &m, &d, &hh, &nn, &ss);
        CHECK(y == 2027 && m == 1 && d == 1 && hh == 0 && nn == 0 && ss == 0, "1. year roll-over");
        CHECK(ela::LocalSecToDateTime(T(2026, 9, 27, 12, 0, 0)) == 46292.5, "1. as a TDateTime");
    }

    // 2. FNV-1a / stagger / back-off
    {
        CHECK(ela::Fnv1a32("") == 0x811C9DC5UL && ela::Fnv1a32("a") == 0xE40C292CUL &&
                  ela::Fnv1a32("foobar") == 0xBF9CF968UL && ela::Fnv1a32("PJLD1014") == 0x1D030E33UL,
              "2. FNV-1a-32 vectors (\"\", a, foobar, PJLD1014)");
        CHECK(ela::StaggerOffsetSec("PJLD1014", 900) == 607 && ela::StaggerOffsetSec("PJLD1014", 300) == 7 &&
                  ela::StaggerOffsetSec("M-0927", 900) == 571 && ela::StaggerOffsetSec("HOSTX", 900) == 873 &&
                  ela::StaggerOffsetSec("PJLD1014", 0) == 0,
              "2. offset = FNV-1a-32(Machine ID) mod W (900 / 300 / local 0), different per machine");
        CHECK(ela::StaggerKey("PJLD1014", "HOSTX") == "PJLD1014" && ela::StaggerKey("HT-90xx", "HOSTX") == "HOSTX" &&
                  ela::StaggerKey("", "HOSTX") == "HOSTX" && ela::StaggerKey("  ", "HOSTX") == "HOSTX",
              "2. Machine ID \"HT-90xx\" / empty -> host name (plan §3 item 4)");
        const int base[8] = { 60, 120, 240, 480, 960, 1800, 1800, 1800 };
        bool okBase = true, okBound = true, okDet = true, anyDiff = false;
        for (int n = 1; n <= 8; ++n)
        {
            okBase = okBase && ela::BackoffBaseSec(n) == base[n - 1];
            const int dly = ela::BackoffDelaySec("PJLD1014", n);
            okBound = okBound && dly >= base[n - 1] * 8 / 10 && dly <= base[n - 1] * 12 / 10;
            okDet = okDet && dly == ela::BackoffDelaySec("PJLD1014", n);
            anyDiff = anyDiff || dly != ela::BackoffDelaySec("M-0927", n);
        }
        CHECK(okBase, "2. back-off base min(60*2^(n-1), 1800): 60 120 240 480 960 1800 1800");
        CHECK(okBound, "2. back-off within +-20% of the base");
        CHECK(okDet && anyDiff, "2. back-off deterministic per Machine ID + n, different between machines");
        const int want[6] = { 54, 127, 198, 520, 997, 2120 };   // mirrored in Python (fmix32(FNV-1a-32("PJLD1014#n")))
        bool okVal = true;
        for (int n = 1; n <= 6; ++n) okVal = okVal && ela::BackoffDelaySec("PJLD1014", n) == want[n - 1];
        CHECK(okVal, "2. back-off values for PJLD1014: 54 127 198 520 997 2120 s");
        CHECK(ela::kMaxRetries == 6, "2. six retries (1 / 2 / 4 / 8 / 16 / 30 min)");
    }

    // 3. gates
    {
        std::string why;
        ela::ScheduleConfig c = Cfg();
        c.o06Production = true;
        c.o06TimePeriod = true;                                       // [O06-8]: a timed O06-4 needs it (★W43 C)
        c.n10Daily = true;
        c.n10Method = 2;
        c.n25_3 = true;
        c.o19Week = true;
        c.n17 = true;
        c.custCode = "851";
        CHECK(ela::JobEnabled(ela::JOB_O06_4, c, false, true, &why) && ela::JobEnabled(ela::JOB_N10_3, c, false, true, &why) &&
                  ela::JobEnabled(ela::JOB_N25_3, c, false, true, &why) && ela::JobEnabled(ela::JOB_N17_PRODLOG, c, false, true, &why),
              "3. 851 with every switch on: O06-4 / N10-3 / N25-3 / N17 on");
        CHECK(!ela::JobEnabled(ela::JOB_O19_VTEST, c, false, true, &why) && Has(why, "915"), "3. VTEST needs 915 / 919");
        CHECK(!ela::JobEnabled(ela::JOB_N25_3, c, true, true, &why) && Has(why, "SIM") &&
                  ela::JobEnabled(ela::JOB_N25_3, c, true, false, &why),
              "3. SIM build: no automatic N25-3 (golden main.cpp:21425), manual still allowed");
        c.o06_1 = false;
        CHECK(!ela::JobEnabled(ela::JOB_O06_4, c, false, true, &why) && Has(why, "O10") &&
                  !ela::JobEnabled(ela::JOB_N10_3, c, false, true, &why) && !ela::JobEnabled(ela::JOB_N25_3, c, false, true, &why),
              "3. D-a: O10 off -> O06-4 / N10-3 / N25-3 off");
        CHECK(ela::JobEnabled(ela::JOB_N17_PRODLOG, c, false, true, &why), "3. O10 off: N17 (a Handler job in golden) still on");
        CHECK(ela::JobEnabled(ela::JOB_O06_4, c, false, false, &why) && ela::JobEnabled(ela::JOB_N25_3, c, false, false, &why),
              "3. manual ignores O10 (the page's analysis is not gated, D-a)");
        {
            // AI(W906-ELA-W43) 20260928 (St02-E helper): ★W43 = C -- a timed O06-4 needs [O06-8]
            ela::ScheduleConfig w = Cfg();
            w.o06Production = true;
            CHECK(!ela::JobEnabled(ela::JOB_O06_4, w, false, true, &why) && Has(why, "O06-8") &&
                      Has(why, "EnanleTimePeriodSaveLog") && ela::JobEnabled(ela::JOB_O06_4, w, false, false, &why) &&
                      ela::JobEnabled(ela::JOB_O06_4, w, false, true, &why, true),
                  "3. ★W43 = C: O06-4 without [O06-8] is off for timed runs; a manual run works; golden mode (every minute) on");
            w.o06TimePeriod = true;
            CHECK(ela::JobEnabled(ela::JOB_O06_4, w, false, true, &why) && why.empty(), "3. ★W43 = C: with [O06-8], on");
            w.o06Production = false;
            CHECK(!ela::JobEnabled(ela::JOB_O06_4, w, false, false, &why) && Has(why, "EnableAutoSaveProductiont"),
                  "3. [O06-8] alone does nothing: O06-4 (EnableAutoSaveProductiont) off, manual too");
        }
        c.o06_1 = true;
        c.custCode = "000";
        CHECK(!ela::JobEnabled(ela::JOB_N25_3, c, false, true, &why) && Has(why, "851") &&
                  !ela::JobEnabled(ela::JOB_N25_3, c, false, false, &why),
              "3. N25-3 only for 851, manual too");
        c.custCode = "915";
        CHECK(ela::JobEnabled(ela::JOB_O19_VTEST, c, false, true, &why), "3. VTEST 915 on");
        c.custCode = " 919 ";
        CHECK(ela::JobEnabled(ela::JOB_O19_VTEST, c, false, true, &why), "3. VTEST 919 on (spaces trimmed)");
        c.o19WeekDay = 7;
        CHECK(!ela::JobEnabled(ela::JOB_O19_VTEST, c, false, true, &why) && Has(why, "0..6"), "3. iO19_WeekPeriod 7: never");
        c.n10Method = 3;
        CHECK(ela::JobEnabled(ela::JOB_N10_3, c, false, true, &why) && ela::N10EffectiveMethod(c) == 2,
              "3. N10-3 method 3 (specified time): golden rgN10_3_1 has 3 items -> clamped to Per Hour, on");
        c.n10Method = 7;
        CHECK(ela::N10EffectiveMethod(c) == 2, "3. N10-3 method 7: clamped to Per Hour");
        c.n10Method = -2;
        CHECK(!ela::JobEnabled(ela::JOB_N10_3, c, false, true, &why) && Has(why, "no item") && ela::N10EffectiveMethod(c) == -1,
              "3. N10-3 method -2: no radio item selected, off");
        c.n25_3 = false;
        c.custCode = "851";
        CHECK(!ela::JobEnabled(ela::JOB_N25_3, c, false, false, &why) && Has(why, "bN25_3_EnableULJamLog"),
              "3. D-f: N25-3 follows bN25_3_EnableULJamLog (manual too)");
        c.n25_3 = true;
        c.n25Host = " ";
        CHECK(!ela::JobEnabled(ela::JOB_N25_3, c, false, true, &why) && Has(why, "host"), "3. N25-2 host empty: off");
        {
            // AI(W906-ELA-W22) 20260928: N25-4 / N25-5 -- N25-3's gates, each with its own switch
            ela::ScheduleConfig w = Cfg();
            w.custCode = "851";
            w.n25_4 = true;
            w.n25_5 = true;
            CHECK(ela::JobEnabled(ela::JOB_N25_4, w, false, true, &why) && ela::JobEnabled(ela::JOB_N25_5, w, false, true, &why) &&
                      !ela::JobEnabled(ela::JOB_N25_3, w, false, true, &why),
                  "3. W22: 851 with bN25_4 / bN25_5 on: N25-4 / N25-5 on, each on its own switch (N25-3 stays off)");
            CHECK(!ela::JobEnabled(ela::JOB_N25_4, w, true, true, &why) && Has(why, "EL_UPLOAD_SUMMARY") &&
                      !ela::JobEnabled(ela::JOB_N25_5, w, true, true, &why) && Has(why, "EL_UPLOAD_EVENTLOG") &&
                      ela::JobEnabled(ela::JOB_N25_4, w, true, false, &why) && ela::JobEnabled(ela::JOB_N25_5, w, true, false, &why),
                  "3. W22 SIM build: no automatic N25-4 / N25-5 (golden main.cpp:21425), manual allowed");
            w.o06_1 = false;
            CHECK(!ela::JobEnabled(ela::JOB_N25_4, w, false, true, &why) && Has(why, "O10") &&
                      !ela::JobEnabled(ela::JOB_N25_5, w, false, true, &why) && ela::JobEnabled(ela::JOB_N25_4, w, false, false, &why),
                  "3. W22 D-a: O10 off -> N25-4 / N25-5 off; a manual run ignores O10");
            w.o06_1 = true;
            w.n25_4 = false;
            CHECK(!ela::JobEnabled(ela::JOB_N25_4, w, false, false, &why) && Has(why, "bN25_4_EnableUpload") &&
                      ela::JobEnabled(ela::JOB_N25_5, w, false, true, &why),
                  "3. W22 D-f: bN25_4_EnableUpload off -> N25-4 off (manual too), N25-5 unchanged");
            w.n25_5 = false;
            CHECK(!ela::JobEnabled(ela::JOB_N25_5, w, false, false, &why) && Has(why, "bN25_5_EnableUpload"),
                  "3. W22 D-f: bN25_5_EnableUpload off -> N25-5 off");
            w.n25_4 = true;
            w.n25_5 = true;
            w.custCode = "000";
            CHECK(!ela::JobEnabled(ela::JOB_N25_4, w, false, false, &why) && Has(why, "851") &&
                      !ela::JobEnabled(ela::JOB_N25_5, w, false, true, &why),
                  "3. W22: N25-4 / N25-5 only for 851, manual too");
            w.custCode = "851";
            w.n25Host = "";
            CHECK(!ela::JobEnabled(ela::JOB_N25_4, w, false, true, &why) && Has(why, "host") &&
                      !ela::JobEnabled(ela::JOB_N25_5, w, false, true, &why),
                  "3. W22: N25-2 host empty -> N25-4 / N25-5 off");
            CHECK(ela::CustomerCodeAllows(ela::EL_UPLOAD_SUMMARY, "851") && !ela::CustomerCodeAllows(ela::EL_UPLOAD_EVENTLOG, "868"),
                  "3. W22: CustomerCodeAllows SUMMARY / EVENTLOG = 851");
        }
        CHECK(ela::CustomerCodeAllows(ela::EL_UPLOAD_JAMWEEK, "851") && !ela::CustomerCodeAllows(ela::EL_UPLOAD_JAMWEEK, "000") &&
                  ela::CustomerCodeAllows(ela::EL_UPLOAD_CHIPADV_LOTEND, "868") &&
                  !ela::CustomerCodeAllows(ela::EL_UPLOAD_CHIPADV_LOTEND, "851") &&
                  ela::CustomerCodeAllows(ela::EL_VTEST_MTBF_SUM, "919") && !ela::CustomerCodeAllows(ela::EL_VTEST_MTBF_SUM, "868") &&
                  ela::CustomerCodeAllows(ela::EL_UPDATE_PARAMETER, "000"),
              "3. CustomerCodeAllows: JAMWEEK 851, CHIPADV_LOTEND 868, VTEST 915 / 919, others any");
    }

    // 4. kinds / windows
    {
        CHECK(ela::IsRemotePath("\\\\srv\\share\\x", &FakeDriveType) && ela::IsRemotePath("//srv/x", &FakeDriveType) &&
                  ela::IsRemotePath("Z:\\Summary", &FakeDriveType) && !ela::IsRemotePath("D:\\RMS", &FakeDriveType) &&
                  !ela::IsRemotePath("RMS", &FakeDriveType),
              "4. remote = UNC or a DRIVE_REMOTE letter");
        CHECK(ela::SimBlocksNetPath("\\\\srv\\share\\x", true, false, &FakeDriveType) &&
                  ela::SimBlocksNetPath("Z:\\Summary", true, false, &FakeDriveType) &&
                  !ela::SimBlocksNetPath("D:\\RMS", true, false, &FakeDriveType) &&
                  !ela::SimBlocksNetPath("\\\\srv\\share\\x", true, true, &FakeDriveType) &&
                  !ela::SimBlocksNetPath("\\\\srv\\share\\x", false, false, &FakeDriveType),
              "4. W58 Q5: SIM blocks a UNC / DRIVE_REMOTE path, not a local one, not with W906_SIM_NET_PATHS=1, never in SHIP");
        ela::ScheduleConfig c = Cfg();
        CHECK(ela::ResolveKind(ela::JOB_N25_3, c, &FakeDriveType) == ela::KIND_FTP &&
                  ela::ResolveKind(ela::JOB_O06_4, c, &FakeDriveType) == ela::KIND_LOCAL,
              "4. N25-3 ftp, O06-4 local");
        CHECK(ela::ResolveKind(ela::JOB_N25_4, c, &FakeDriveType) == ela::KIND_FTP &&
                  ela::ResolveKind(ela::JOB_N25_5, c, &FakeDriveType) == ela::KIND_FTP &&
                  ela::StaggerWindowSec(ela::JOB_N25_4, ela::KIND_FTP, c, false) == 900 &&
                  ela::StaggerWindowSec(ela::JOB_N25_5, ela::KIND_FTP, c, true) == 900,
              "4. W22: N25-4 / N25-5 ftp, window 900 s");
        c.o06Path = "\\\\srv\\rms";
        c.n10DrivePath = "Z:\\Summary";
        c.n10UploadMethod = 0;
        CHECK(ela::ResolveKind(ela::JOB_O06_4, c, &FakeDriveType) == ela::KIND_NETDRIVE &&
                  ela::ResolveKind(ela::JOB_N10_3, c, &FakeDriveType) == ela::KIND_LOCAL,
              "4. O06-4 to UNC = netdrive; N10-3 method 0 saves locally (EventLogSummary) whatever the drive path");
        c.n10UploadMethod = 1;
        CHECK(ela::ResolveKind(ela::JOB_N10_3, c, &FakeDriveType) == ela::KIND_NETDRIVE, "4. N10-3 method 1 to Z: = netdrive");
        c.n10UploadMethod = 5;
        CHECK(ela::ResolveKind(ela::JOB_N10_3, c, &FakeDriveType) == ela::KIND_NETDRIVE,
              "4. N10-4 value 5: rgN10_4 has two items -> clamped to Net Drive");
        c.n10UploadMethod = -1;
        CHECK(ela::ResolveKind(ela::JOB_N10_3, c, &FakeDriveType) == ela::KIND_LOCAL, "4. N10-4 value -1: the local folder");
        c.n10UploadMethod = 1;
        CHECK(ela::StaggerWindowSec(ela::JOB_N25_3, ela::KIND_FTP, c, false) == 900 &&
                  ela::StaggerWindowSec(ela::JOB_N10_3, ela::KIND_NETDRIVE, c, false) == 300 &&
                  ela::StaggerWindowSec(ela::JOB_O06_4, ela::KIND_NETDRIVE, c, false) == 0 &&   //AI(W906-MERGE-0929) 20260929: c.o06TimePeriod is off here, so since W43 O06PeriodMin returns 0 (ElaSchedule.cpp:435) and the window is min(0*60/2, 300) = 0; b11a7a20 updated :532-534 but not this line
                  ela::StaggerWindowSec(ela::JOB_O06_4, ela::KIND_NETDRIVE, c, true) == 30 &&
                  ela::StaggerWindowSec(ela::JOB_O19_VTEST, ela::KIND_LOCAL, c, false) == 0,
              "4. windows: ftp 900, netdrive 300 (O06: 0 with the period off; capped at half its period: golden 1 min -> 30), local 0");
        ela::ScheduleConfig nd = Cfg();
        nd.o06UseNetDrive = true;
        CHECK(ela::ResolveKind(ela::JOB_O06_4, nd, &FakeDriveType) == ela::KIND_NETDRIVE &&
                  ela::ResolveKind(ela::JOB_O19_VTEST, nd, &FakeDriveType) == ela::KIND_LOCAL,
              "4. [O06-6] Use Net Drive: O06-4 is a network-drive job (300 s) even on a local path; others unchanged");
        c.o06TimePeriod = true;
        c.o06TimePeriodIdx = 0;
        CHECK(ela::O06PeriodMin(c, false) == 10 && ela::StaggerWindowSec(ela::JOB_O06_4, ela::KIND_NETDRIVE, c, false) == 300,
              "4. O06 10-min period: window 300 (= half the period)");
        c.o06TimePeriodIdx = 1;
        CHECK(ela::O06PeriodMin(c, true) == 30, "4. O06 30-min period");
        c.o06TimePeriod = false;
        CHECK(ela::O06PeriodMin(c, true) == 1 && ela::O06PeriodMin(c, false) == 0 &&
                  ela::StaggerWindowSec(ela::JOB_O06_4, ela::KIND_NETDRIVE, c, false) == 0,
              "4. O06 without [O06-8]: golden every minute (G3) vs ★W43 = C never (0, no window)");
    }

    // 5. calendars
    {
        ela::ScheduleConfig c = Cfg();
        c.o06Production = true;
        LocalSec s = 0, n = 0;
        ela::Calendar k = ela::JobCalendar(ela::JOB_O06_4, c, false);
        CHECK(k.rules.empty() && !k.Latest(T(2026, 9, 27, 10, 37, 12), &s),
              "5. ★W43 = C: O06 without [O06-8] has no slot at all (W18 B was hourly)");
        k = ela::JobCalendar(ela::JOB_O06_4, c, true);
        CHECK(k.Latest(T(2026, 9, 27, 10, 37, 12), &s) && s == T(2026, 9, 27, 10, 37, 0), "5. O06 golden: every minute");
        c.o06TimePeriod = true;                                       // [O06-8] 10 min from here on
        k = ela::JobCalendar(ela::JOB_O06_4, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 10, 37, 12), &s) && s == T(2026, 9, 27, 10, 30, 0) && k.Next(s, &n) &&
                  n == T(2026, 9, 27, 10, 40, 0),
              "5. ★W43 = C: O06 with [O06-8] 10 min: latest 10:30, next 10:40");
        c.n10Daily = true;
        c.n10Method = 1;
        k = ela::JobCalendar(ela::JOB_N10_3, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 7, 59, 59), &s) && s == T(2026, 9, 26, 20, 0, 0) &&
                  k.Latest(T(2026, 9, 27, 8, 0, 0), &s) && s == T(2026, 9, 27, 8, 0, 0) && k.Next(s, &n) &&
                  n == T(2026, 9, 27, 20, 0, 0) && k.Next(n, &s) && s == T(2026, 9, 28, 8, 0, 0),
              "5. N10-3 method 1: 08:00 and 20:00");
        CHECK(ela::JobCalendar(ela::JOB_O06_4, c, false).rules.size() == 1 &&
                  ela::JobCalendar(ela::JOB_O06_4, c, true).rules.size() == 3 &&
                  ela::JobCalendar(ela::JOB_N10_3, c, true).rules.size() == 3,
              "5. golden coupling (Timer2 :3071-3083): O06 and N10 share both calendars only in golden mode");
        c.n10Method = 0;
        k = ela::JobCalendar(ela::JOB_N10_3, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 23, 59, 59), &s) && s == T(2026, 9, 27, 0, 0, 0), "5. N10-3 method 0: 00:00");
        k = ela::JobCalendar(ela::JOB_N25_4, c, false);
        CHECK(k.rules.size() == 1 && k.Latest(T(2026, 9, 27, 23, 59, 59), &s) && s == T(2026, 9, 27, 0, 0, 0) && k.Next(s, &n) &&
                  n == T(2026, 9, 28, 0, 0, 0) && ela::JobCalendar(ela::JOB_N25_5, c, true).rules.size() == 1,
              "5. W22: N25-4 / N25-5 daily 00:00 (golden main.cpp:21449-21457)");
        c.n10Method = 2;
        k = ela::JobCalendar(ela::JOB_N10_3, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 23, 59, 59), &s) && s == T(2026, 9, 27, 23, 0, 0), "5. N10-3 method 2: every hour");
        c.n10Method = 3;
        k = ela::JobCalendar(ela::JOB_N10_3, c, false);
        CHECK(k.rules.size() == 1 && k.Latest(T(2026, 9, 27, 23, 59, 59), &s) && s == T(2026, 9, 27, 23, 0, 0),
              "5. N10-3 method 3 without a specified minute (a hand-built config): the analyzer's clamp = every hour");
        c.n10SpecifiedMinute = 7 * 60 + 30;
        k = ela::JobCalendar(ela::JOB_N10_3, c, false);
        CHECK(k.rules.size() == 1 && k.Latest(T(2026, 9, 27, 23, 59, 59), &s) && s == T(2026, 9, 27, 7, 30, 0) && k.Next(s, &n) &&
                  n == T(2026, 9, 28, 7, 30, 0),
              "5. ★W44 B: N10-3 method 3 at n10SpecifiedMinute = 07:30, once a day");
        c.n10SpecifiedMinute = -1;
        c.o19WeekDay = 1;
        k = ela::JobCalendar(ela::JOB_O19_VTEST, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 12, 0, 0), &s) && s == T(2026, 9, 21, 0, 0, 0) && k.Latest(T(2026, 9, 28, 0, 0, 0), &s) &&
                  s == T(2026, 9, 28, 0, 0, 0) && k.Next(s, &n) && n == T(2026, 10, 5, 0, 0, 0),
              "5. VTEST weekday 1: Mondays 00:00");
        k = ela::JobCalendar(ela::JOB_N17_PRODLOG, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 0, 59, 59), &s) && s == T(2026, 9, 26, 1, 0, 0), "5. N17 01:00");
        k = ela::JobCalendar(ela::JOB_N25_3, c, false);
        CHECK(k.Latest(T(2026, 9, 27, 0, 0, 0), &s) && s == T(2026, 9, 27, 0, 0, 0), "5. N25-3 00:00");
    }

    // 6. O06-4 / N10-3 once per slot
    {
        FakeClock clk;
        Recorder rec(&clk);
        ela::ScheduleConfig cfg = Cfg();
        cfg.o06Production = true;
        cfg.n10Daily = true;
        cfg.n10Method = 2;
        ela::ScheduleSetup su = Setup(&clk, &cfg, Sb("state\\s6a.ini"));
        su.keepGoldenBugs = true;
        ela::Scheduler s(su);
        s.SetJob(ela::JOB_O06_4, Body(&rec));
        s.SetJob(ela::JOB_N10_3, Body(&rec));
        clk.t = T(2026, 9, 27, 10, 0, 30);
        Run(s, clk, T(2026, 9, 27, 10, 5, 30), 1);
        const std::vector<Rec> o = OfJob(rec, ela::JOB_O06_4), n = OfJob(rec, ela::JOB_N10_3);
        bool okMin = o.size() == 5 && n.size() == 5;
        for (size_t i = 0; okMin && i < o.size(); ++i)
            okMin = o[i].slot == T(2026, 9, 27, 10, 1 + (int)i, 0) && o[i].at == o[i].slot && o[i].attempt == 1 &&
                    n[i].slot == o[i].slot;
        CHECK(okMin, "6. golden: O06-4 every minute (G3), and each O06 slot also runs N10-3 (coupling) -- once per minute");
        CHECK(s.State(ela::JOB_O06_4).armed == T(2026, 9, 27, 10, 0, 30) && s.State(ela::JOB_O06_4).done == T(2026, 9, 27, 10, 5, 0),
              "6. first run: armed at boot (no catch-up), done = last verified slot");

        ela::ScheduleConfig c2 = Cfg();
        c2.o06Production = true;
        c2.o06TimePeriod = true;
        c2.o06TimePeriodIdx = 0;
        c2.n10Daily = true;
        c2.n10Method = 1;
        for (int golden = 0; golden < 2; ++golden)
        {
            Recorder rr(&clk);
            ela::ScheduleSetup s2u = Setup(&clk, &c2, Sb(golden ? "state\\s6c.ini" : "state\\s6b.ini"));
            s2u.keepGoldenBugs = golden != 0;
            ela::Scheduler s2(s2u);
            s2.SetJob(ela::JOB_O06_4, Body(&rr));
            s2.SetJob(ela::JOB_N10_3, Body(&rr));
            clk.t = T(2026, 9, 27, 7, 55, 30);
            Run(s2, clk, T(2026, 9, 27, 8, 25, 30), 1);
            const std::vector<Rec> oo = OfJob(rr, ela::JOB_O06_4), nn = OfJob(rr, ela::JOB_N10_3);
            const bool okO = oo.size() == 3 && oo[0].slot == T(2026, 9, 27, 8, 0, 0) && oo[1].slot == T(2026, 9, 27, 8, 10, 0) &&
                             oo[2].slot == T(2026, 9, 27, 8, 20, 0);
            if (!golden)
                CHECK(okO && nn.size() == 1 && nn[0].slot == T(2026, 9, 27, 8, 0, 0),
                      "6. W18 B: O06-4 every 10 min, N10-3 only at 08:00 (decoupled)");
            else
                CHECK(okO && nn.size() == 3 && nn[2].slot == T(2026, 9, 27, 8, 20, 0),
                      "6. golden: N10-3 also on every O06 slot");
        }
    }

    // 7. N25-3 at midnight, FTP stagger
    {
        for (int sim = 0; sim < 2; ++sim)
        {
            FakeClock clk;
            Recorder rec(&clk);
            ela::ScheduleConfig cfg = Cfg();
            cfg.custCode = "851";
            cfg.n25_3 = true;
            ela::ScheduleSetup su = Setup(&clk, &cfg, Sb(sim ? "state\\s7b.ini" : "state\\s7a.ini"));
            su.simBuild = sim != 0;
            ela::Scheduler s(su);
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 27, 23, 50, 0);
            Run(s, clk, T(2026, 9, 28, 0, 20, 0), 1);
            if (!sim)
                CHECK(rec.recs.size() == 1 && rec.recs[0].slot == T(2026, 9, 28, 0, 0, 0) &&
                          rec.recs[0].at == T(2026, 9, 28, 0, 10, 7) && s.State(ela::JOB_N25_3).offsetSec == 607 &&
                          s.State(ela::JOB_N25_3).kind == ela::KIND_FTP,
                      "7. N25-3 once at 00:00 + FNV-1a(PJLD1014) mod 900 = 607 s (00:10:07), slot 00:00");
            else
                CHECK(rec.recs.empty() && Has(s.State(ela::JOB_N25_3).why, "SIM"), "7. SIM build: no automatic N25-3");
        }
        {
            // AI(W906-ELA-W22) 20260928: the three FTP jobs share the slot and the Machine ID's offset; Tick runs them in id
            // order = golden's queue order JAMWEEK > SUMMARY > EVENTLOG (Rev891 Analyzer.cpp:161-183)
            FakeClock clk;
            Recorder rec(&clk);
            ela::ScheduleConfig cfg = Cfg();
            cfg.custCode = "851";
            cfg.n25_3 = true;
            cfg.n25_4 = true;
            cfg.n25_5 = true;
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s7c.ini")));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            s.SetJob(ela::JOB_N25_4, Body(&rec));
            s.SetJob(ela::JOB_N25_5, Body(&rec));
            clk.t = T(2026, 9, 27, 23, 50, 0);
            Run(s, clk, T(2026, 9, 28, 0, 20, 0), 1);
            bool ok = rec.recs.size() == 3 && rec.recs[0].job == ela::JOB_N25_3 && rec.recs[1].job == ela::JOB_N25_4 &&
                      rec.recs[2].job == ela::JOB_N25_5 && s.State(ela::JOB_N25_4).offsetSec == 607 &&
                      s.State(ela::JOB_N25_5).kind == ela::KIND_FTP;
            for (size_t i = 0; ok && i < rec.recs.size(); ++i)
                ok = rec.recs[i].slot == T(2026, 9, 28, 0, 0, 0) && rec.recs[i].at == T(2026, 9, 28, 0, 10, 7);
            CHECK(ok, "7. W22: N25-3 / N25-4 / N25-5 in the 00:00 slot at + 607 s, one after another in golden order");
        }
        for (int allow = 0; allow < 2; ++allow)
        {
            // AI(W906-W58) 20260930 (St02-E): W58 Q5 -- in SIM a job whose target is a network share is off, timed and manual
            FakeClock clk;
            Recorder rec(&clk);
            ela::ScheduleConfig cfg = Cfg();
            cfg.n17 = true;
            cfg.n17Path = "\\\\srv\\RMS";
            ela::ScheduleSetup su = Setup(&clk, &cfg, Sb(allow ? "state\\s7e.ini" : "state\\s7d.ini"));
            su.simBuild = true;
            su.simNetPaths = allow != 0;
            ela::Scheduler s(su);
            s.SetJob(ela::JOB_N17_PRODLOG, Body(&rec));
            clk.t = T(2026, 9, 27, 23, 50, 0);
            s.Tick();
            const ela::Scheduler::JobState& st = s.State(ela::JOB_N17_PRODLOG);
            if (!allow)
            {
                CHECK(!st.enabled && Has(st.why, "W58 Q5") && st.kind == ela::KIND_NETDRIVE,
                      "7. W58 Q5: SIM, N17 to \\\\srv\\RMS -- off (a network share)");
                s.RunNow(ela::JOB_N17_PRODLOG);
                clk.t = T(2026, 9, 27, 23, 50, 1);
                s.Tick();
                CHECK(rec.recs.empty() && s.State(ela::JOB_N17_PRODLOG).lastStatus == "manual refused" &&
                          Has(s.State(ela::JOB_N17_PRODLOG).lastDetail, "W58 Q5"),
                      "7. W58 Q5: a manual run is refused too");
            }
            else
                CHECK(st.enabled, "7. W58 Q5: with W906_SIM_NET_PATHS=1 (simNetPaths) the same job is on");
        }
    }

    // 8. VTEST once a week (edge)
    {
        const char* custs[3] = { "915", "919", "851" };
        for (int k = 0; k < 3; ++k)
        {
            FakeClock clk;
            Recorder rec(&clk);
            ela::ScheduleConfig cfg = Cfg();
            cfg.custCode = custs[k];
            cfg.o19Week = true;
            cfg.o19WeekDay = 1;
            ela::Scheduler s(Setup(&clk, &cfg, Sb(k == 0 ? "state\\s8a.ini" : k == 1 ? "state\\s8b.ini" : "state\\s8c.ini")));
            s.SetJob(ela::JOB_O19_VTEST, Body(&rec));
            clk.t = T(2026, 9, 27, 12, 0, 0);
            Run(s, clk, T(2026, 10, 12, 12, 0, 0), 600);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 600 s grid (hits every 00:00; was 10 s = 129,600 ticks)
            bool ok = rec.recs.size() == 3;
            for (size_t i = 0; ok && i < rec.recs.size(); ++i)
                ok = ela::DayOfWeek0(rec.recs[i].slot) == 1 && rec.recs[i].at == rec.recs[i].slot;
            ok = ok && rec.recs[0].slot == T(2026, 9, 28, 0, 0, 0) && rec.recs[2].slot == T(2026, 10, 12, 0, 0, 0);
            if (k < 2) CHECK(ok, k == 0 ? "8. VTEST 915: once a week, Mondays 00:00 (3 in 15 days)" : "8. VTEST 919: the same");
            else CHECK(rec.recs.empty(), "8. VTEST with 851: never");
        }
    }

    // 9. W13 N17 UploadProdLog (real copy inside the sandbox)
    {
        const std::string prod = Sb("Production_Log"), rms = Sb("RMS");
        ::CreateDirectoryA((prod + "\\202609").c_str(), NULL);
        ela::ScheduleConfig cfg = Cfg();
        cfg.n17 = true;
        CHECK(ela::N17FileName(cfg, 2026, 9, 27) == "HT-9045_PJLD1014_20260927_ProductionLog.csv",
              "9. N17 file name (non-SPIL, Command.cpp:12421)");
        ela::ScheduleConfig sp = cfg;
        sp.spil = true;
        CHECK(ela::N17FileName(sp, 2026, 9, 27) == "HOSTX_20260927.csv", "9. SPIL: <PC_NAME>_yyyymmdd.csv (:12416)");
        sp.custCode = "912";
        sp.spilForQle = 1;
        CHECK(ela::N17FileName(sp, 2026, 9, 27) == "HT-9045_PJLD1014_20260927_ProductionLog.csv", "9. SPIL 912 + QLE (:12412)");

        const std::string src1 = prod + "\\202609\\HT-9045_PJLD1014_20260927_ProductionLog.csv";
        WriteText(src1, "Date,Time,Lot\r\n2026/09/27,10:00:00,L1\r\n");
        FakeClock clk;
        ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s9.ini")));
        clk.t = T(2026, 9, 28, 0, 59, 0);
        Run(s, clk, T(2026, 9, 28, 1, 5, 0), 1);
        const std::string dst1 = rms + "\\HT-9045_PJLD1014_20260927_ProductionLog.csv";
        CHECK(s.State(ela::JOB_N17_PRODLOG).runs == 1 && ReadText(dst1) == ReadText(src1) &&
                  s.State(ela::JOB_N17_PRODLOG).done == T(2026, 9, 28, 1, 0, 0) && CountLogs("N17-UploadProdLog", "verified") == 1,
              "9. N17 at 01:00: yesterday's Production_Log copied to asN17ProductionLogPath, same size, verified");

        // failed-then-verified: the N-17 folder is missing at 01:00, appears after the first try
        const std::string src2 = prod + "\\202609\\HT-9045_PJLD1014_20260928_ProductionLog.csv";
        WriteText(src2, "Date,Time,Lot\r\n2026/09/28,11:00:00,L2\r\n2026/09/28,12:00:00,L3\r\n");
        cfg.n17Path = Sb("RMS2");
        s.ReloadConfig();
        const int runs0 = s.State(ela::JOB_N17_PRODLOG).runs;
        const int d1 = ela::BackoffDelaySec("PJLD1014", 1);
        clk.t = T(2026, 9, 29, 0, 59, 0);
        while (clk.t <= T(2026, 9, 29, 1, 10, 0))
        {
            s.Tick();
            if (s.State(ela::JOB_N17_PRODLOG).runs == runs0 + 1 && !Exists(Sb("RMS2"))) ::CreateDirectoryA(Sb("RMS2").c_str(), NULL);
            clk.t += 1;
        }
        const ela::Scheduler::JobState st = s.State(ela::JOB_N17_PRODLOG);
        CHECK(st.runs == runs0 + 2 && st.done == T(2026, 9, 29, 1, 0, 0) &&
                  ReadText(Sb("RMS2") + "\\HT-9045_PJLD1014_20260928_ProductionLog.csv") == ReadText(src2) &&
                  CountLogs("N17-UploadProdLog", "failed (retry)") == 1 && CountLogs("N17-UploadProdLog", "retry 1 in") == 1,
              "9. N-17 folder missing -> retry after the back-off, then copied and verified");
        CHECK(Has(st.lastResult, "2026/09/29 01:00:54 attempt 2: verified") && d1 == 54,
              "9. the retry ran at 01:00:00 + back-off(1) = 01:00:54");

        // CopyFileAndVerify cases
        std::string det;
        const std::string dd = Sb("RMS3");
        CHECK(ela::CopyFileAndVerify(src1, dd, dd + "\\a.csv", false, &det) == ela::CV_DEST_DIR_MISSING, "9. target folder missing");
        ::CreateDirectoryA(dd.c_str(), NULL);
        CHECK(ela::CopyFileAndVerify(Sb("nope.csv"), dd, dd + "\\a.csv", false, &det) == ela::CV_SOURCE_MISSING,
              "9. source missing (golden: silently nothing)");
        CHECK(ela::CopyFileAndVerify(src1, dd, dd + "\\a.csv", false, &det) == ela::CV_COPIED, "9. copied + same size");
        CHECK(ela::CopyFileAndVerify(src1, dd, dd + "\\a.csv", false, &det) == ela::CV_ALREADY_SAME,
              "9. again: already there with the same size = done (bFailIfExists=TRUE retry)");
        WriteText(dd + "\\b.csv", "other");
        CHECK(ela::CopyFileAndVerify(src1, dd, dd + "\\b.csv", false, &det) == ela::CV_EXISTS_DIFFERENT &&
                  ReadText(dd + "\\b.csv") == "other",
              "9. a foreign target of another size is not overwritten");
        CHECK(ela::CopyFileAndVerify(src1, dd, dd + "\\b.csv", true, &det) == ela::CV_COPIED && ReadText(dd + "\\b.csv") == ReadText(src1),
              "9. our own earlier partial copy may be overwritten");
        ela::ScheduleConfig c3 = cfg;
        c3.n17Path = dd;
        WriteText(dd + "\\HT-9045_PJLD1014_20260927_ProductionLog.csv", "short");
        ela::JobContext ctx;
        ctx.job = ela::JOB_N17_PRODLOG;
        ctx.slot = T(2026, 9, 28, 1, 0, 0);
        ctx.slotDateTime = ela::LocalSecToDateTime(ctx.slot);
        ctx.attempt = 1;
        ctx.catchUp = false;
        ctx.manual = false;
        ctx.cfg = &c3;
        ctx.slotFlags = 0;
        CHECK(ela::RunN17UploadProdLog(ctx) == ela::JOB_FAILED && Has(ctx.detail, "not overwritten"),
              "9. RunN17UploadProdLog: foreign target of another size -> failed, not retried");
    }

    // 10. O10 off: nothing automatic
    {
        FakeClock clk;
        Recorder rec(&clk);
        ela::ScheduleConfig cfg = Cfg();
        cfg.o06_1 = false;
        cfg.custCode = "851";
        cfg.o06Production = true;
        cfg.o06TimePeriod = true;                                     // [O06-8] (★W43 C): only O10 is closed
        cfg.n10Daily = true;
        cfg.n10Method = 2;
        cfg.n25_3 = true;
        cfg.n17 = true;
        ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s10a.ini")));
        for (int j = 0; j < ela::JOB_TOTAL; ++j) s.SetJob(j, Body(&rec));
        clk.t = T(2026, 9, 27, 0, 30, 0);
        Run(s, clk, T(2026, 9, 29, 0, 30, 0), 60);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 60 s grid (hits 01:00:00; was 10 s)
        CHECK(CountJob(rec, ela::JOB_O06_4) == 0 && CountJob(rec, ela::JOB_N10_3) == 0 && CountJob(rec, ela::JOB_N25_3) == 0 &&
                  Has(s.State(ela::JOB_N25_3).why, "O10"),
              "10. O10 off: no O06-4 / N10-3 / N25-3 in two days");
        CHECK(CountJob(rec, ela::JOB_N17_PRODLOG) == 2, "10. O10 off: N17 still runs at 01:00 (golden Handler job)");
        Recorder r2(&clk);
        ela::ScheduleConfig c2 = cfg;
        c2.custCode = "915";
        c2.o19Week = true;
        c2.o19WeekDay = 1;
        ela::Scheduler s2(Setup(&clk, &c2, Sb("state\\s10b.ini")));
        s2.SetJob(ela::JOB_O19_VTEST, Body(&r2));
        clk.t = T(2026, 9, 27, 12, 0, 0);
        Run(s2, clk, T(2026, 10, 6, 0, 0, 0), 600);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 600 s grid (was 10 s = 73,440 ticks)
        CHECK(r2.recs.empty(), "10. O10 off: no VTEST");
    }

    // 11. D-b boot catch-up
    {
        const std::string sf = Sb("state\\s11.ini");
        FakeClock clk;
        ela::ScheduleConfig cfg = Cfg();
        cfg.custCode = "851";
        cfg.n25_3 = true;
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 26, 23, 55, 0);
            Run(s, clk, T(2026, 9, 27, 0, 30, 0), 1);
            CHECK(rec.recs.size() == 1 && s.State(ela::JOB_N25_3).done == T(2026, 9, 27, 0, 0, 0), "11. day 1: verified 09-27 00:00");
        }   // the Handler goes off
        const std::string text = ReadText(sf);
        CHECK(Has(text, "[N25-3]") && Has(text, "done=2026/09/27 00:00:00") && Has(text, "armed=2026/09/26 23:55:00"),
              "11. state file: [N25-3] done / armed");
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 29, 8, 0, 0);   // 09-28 and 09-29 00:00 were missed
            Run(s, clk, T(2026, 9, 29, 8, 15, 0), 1);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 1 s through the catch-up at 08:10:07,
            Run(s, clk, T(2026, 9, 29, 12, 0, 0), 60);   //   then a 60 s grid to 12:00 (nothing else is due)
            CHECK(rec.recs.size() == 1 && rec.recs[0].catchUp && rec.recs[0].slot == T(2026, 9, 29, 0, 0, 0) &&
                      rec.recs[0].at == T(2026, 9, 29, 8, 10, 7) && s.State(ela::JOB_N25_3).done == T(2026, 9, 29, 0, 0, 0),
                  "11. boot 09-29 08:00: only the latest missed slot (09-29 00:00) runs once, at boot + 607 s");
        }
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 29, 13, 0, 0);
            Run(s, clk, T(2026, 9, 29, 23, 0, 0), 60);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 60 s grid (was 5 s; nothing may run)
            CHECK(rec.recs.empty(), "11. boot again the same day: the slot is done, no second run");
        }
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 10, 20, 9, 0, 0);   // off for three weeks
            Run(s, clk, T(2026, 10, 20, 10, 0, 0), 1);
            CHECK(rec.recs.size() == 1 && rec.recs[0].slot == T(2026, 10, 20, 0, 0, 0) && rec.recs[0].catchUp,
                  "11. off for three weeks: one catch-up, the latest slot only");
        }
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s11_new.ini")));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 29, 8, 0, 0);
            Run(s, clk, T(2026, 9, 29, 12, 0, 0), 60);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 60 s grid (was 1 s; nothing may run)
            CHECK(rec.recs.empty() && Has(ReadText(Sb("state\\s11_new.ini")), "armed=2026/09/29 08:00:00"),
                  "11. no record: no catch-up (nothing known to be missed), armed written");
        }
        {
            // failed at the slot and the Handler went off before a retry: redone at the next boot
            Recorder rec(&clk);
            rec.dflt = ela::JOB_RETRY;
            const std::string sf2 = Sb("state\\s11_fail.ini");
            {
                ela::Scheduler s(Setup(&clk, &cfg, sf2));
                s.SetJob(ela::JOB_N25_3, Body(&rec));
                clk.t = T(2026, 9, 27, 23, 55, 0);
                Run(s, clk, T(2026, 9, 28, 0, 10, 30), 1);
            }
            Recorder r2(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf2));
            s.SetJob(ela::JOB_N25_3, Body(&r2));
            clk.t = T(2026, 9, 28, 9, 0, 0);
            Run(s, clk, T(2026, 9, 28, 9, 20, 0), 1);
            CHECK(rec.recs.size() == 1 && r2.recs.size() == 1 && r2.recs[0].catchUp && r2.recs[0].slot == T(2026, 9, 28, 0, 0, 0) &&
                      s.State(ela::JOB_N25_3).done == T(2026, 9, 28, 0, 0, 0),
                  "11. a failed slot (not verified) is redone at the next boot and then recorded");
        }
    }

    // 12. back-off
    {
        const int d[7] = { 0, 54, 127, 198, 520, 997, 2120 };
        {
            FakeClock clk;
            Recorder rec(&clk);
            rec.script.push_back(ela::JOB_RETRY);
            rec.script.push_back(ela::JOB_RETRY);
            ela::ScheduleConfig cfg = Cfg();
            cfg.custCode = "851";
            cfg.n25_3 = true;
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s12a.ini")));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 27, 23, 59, 0);
            Run(s, clk, T(2026, 9, 28, 0, 20, 0), 1);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 1 s through the tries (00:10:07 .. 00:13:08),
            Run(s, clk, T(2026, 9, 28, 2, 0, 0), 60);   //   then a 60 s grid to 02:00 (nothing else is due)
            const LocalSec a0 = T(2026, 9, 28, 0, 10, 7);
            CHECK(rec.recs.size() == 3 && rec.recs[0].at == a0 && rec.recs[1].at == a0 + d[1] && rec.recs[2].at == a0 + d[1] + d[2] &&
                      rec.recs[2].attempt == 3 && s.State(ela::JOB_N25_3).done == T(2026, 9, 28, 0, 0, 0),
                  "12. failed twice, then verified: retries at +54 s and +127 s, done only after the verified one");
        }
        {
            FakeClock clk;
            Recorder rec(&clk);
            rec.dflt = ela::JOB_RETRY;
            ela::ScheduleConfig cfg = Cfg();
            cfg.custCode = "851";
            cfg.n25_3 = true;
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s12b.ini")));
            s.SetJob(ela::JOB_N25_3, Body(&rec));
            g_logs.clear();
            clk.t = T(2026, 9, 27, 23, 59, 0);
            Run(s, clk, T(2026, 9, 28, 1, 30, 0), 1);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 1 s through the 7th try (01:17:03) and the give-up,
            Run(s, clk, T(2026, 9, 28, 6, 0, 0), 60);   //   then a 60 s grid to 06:00 (nothing else is due)
            bool ok = rec.recs.size() == 7;
            for (size_t i = 1; ok && i < rec.recs.size(); ++i) ok = rec.recs[i].at - rec.recs[i - 1].at == d[i];
            CHECK(ok && CountLogs("N25-3", "given up") == 1 && s.State(ela::JOB_N25_3).done < 0 && !s.State(ela::JOB_N25_3).pending,
                  "12. always failing: 1 + 6 attempts (waits 54 127 198 520 997 2120 s), then given up, not recorded");
            const ela::Scheduler::JobState st12 = s.State(ela::JOB_N25_3);
            bool has7 = false;
            for (size_t i = 0; i < st12.results.size(); ++i) has7 = has7 || Has(st12.results[i], "attempt 7: failed (retry)");
            CHECK(st12.results.size() == 16 && Has(st12.results.back(), "given up") && has7 &&
                      Has(st12.results[0], " on") && Has(s.StatusJson(), "\"results\":[\""),
                  "12. the job's own record: on, armed, 7 attempts, 6 retries, given up (16 lines, in StatusJson)");
        }
        {
            FakeClock clk;
            Recorder rec(&clk);
            rec.dflt = ela::JOB_RETRY;
            ela::ScheduleConfig cfg = Cfg();
            cfg.o06Production = true;
            cfg.o06TimePeriod = true;
            cfg.o06TimePeriodIdx = 0;   // every 10 min, local
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s12c.ini")));
            s.SetJob(ela::JOB_O06_4, Body(&rec));
            g_logs.clear();
            clk.t = T(2026, 9, 27, 9, 59, 30);
            Run(s, clk, T(2026, 9, 27, 10, 10, 30), 1);
            const LocalSec a0 = T(2026, 9, 27, 10, 0, 0);
            CHECK(rec.recs.size() == 5 && rec.recs[3].at == a0 + d[1] + d[2] + d[3] && rec.recs[3].slot == a0 &&
                      rec.recs[4].slot == T(2026, 9, 27, 10, 10, 0) && rec.recs[4].attempt == 1 &&
                      CountLogs("O06-4", "comes before the retry") == 1,
                  "12. retries stop at the next slot (10:10): 4 tries for 10:00, then 10:10 starts fresh");
        }
    }

    // 13. manual, no body, FTP_Log, the process-wide entry points
    {
        FakeClock clk;
        Recorder rec(&clk);
        ela::ScheduleConfig cfg = Cfg();
        cfg.o06_1 = false;
        cfg.custCode = "851";
        cfg.n25_3 = true;
        ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s13a.ini")));
        s.SetJob(ela::JOB_N25_3, Body(&rec));
        clk.t = T(2026, 9, 27, 10, 0, 0);
        s.Tick();
        CHECK(s.RunNow(ela::JOB_N25_3) && !s.RunNow(ela::JOB_TOTAL), "13. RunNow queues; a bad id is refused");
        clk.t += 1;
        s.Tick();
        CHECK(rec.recs.size() == 1 && rec.recs[0].manual && rec.recs[0].slot == clk.t && s.State(ela::JOB_N25_3).done < 0,
              "13. manual run with O10 off: once, slot = now, nothing recorded");
        cfg.custCode = "000";
        s.ReloadConfig();
        s.RunNow(ela::JOB_N25_3);
        g_logs.clear();
        clk.t += 1;
        s.Tick();
        CHECK(rec.recs.size() == 1 && CountLogs("N25-3", "manual refused") == 1, "13. manual on a non-851 machine: refused");

        Recorder r2(&clk);
        ela::ScheduleConfig c2 = Cfg();
        c2.o06Production = true;
        c2.o06TimePeriod = true;                                      // [O06-8] 30 min (★W43 C)
        c2.o06TimePeriodIdx = 1;
        ela::Scheduler s2(Setup(&clk, &c2, Sb("state\\s13b.ini")));
        s2.SetJob(ela::JOB_O06_4, ela::JobFn());
        g_logs.clear();
        clk.t = T(2026, 9, 27, 9, 59, 0);
        Run(s2, clk, T(2026, 9, 27, 12, 30, 0), 5);
        CHECK(CountLogs("O06-4", "no job body") == 1 && s2.State(ela::JOB_O06_4).done < 0 && !s2.State(ela::JOB_O06_4).pending,
              "13. no body (R2 not merged): logged once over six slots, nothing recorded");
        CHECK(Has(s2.StatusJson(), "\"id\":\"O06-4\"") && Has(s2.StatusJson(), "\"hasBody\":false") &&
                  Has(s2.StatusJson(), "\"history\":["),
              "13. StatusJson: jobs + history");

        // FTP_Log (default sink) under the sandbox log root
        ela::ScheduleSetup fu = Setup(&clk, &cfg, Sb("state\\s13c.ini"));
        fu.logSink = 0;
        cfg.custCode = "851";
        cfg.o06_1 = true;
        {
            ela::Scheduler s3(fu);
            s3.SetJob(ela::JOB_N25_3, Body(&rec));
            clk.t = T(2026, 9, 27, 23, 59, 0);
            Run(s3, clk, T(2026, 9, 28, 0, 11, 0), 1);
        }
        const std::string f1 = Sb("log\\UploadFile\\2026\\09\\FTP_Log_20260927.csv");
        const std::string f2 = Sb("log\\UploadFile\\2026\\09\\FTP_Log_20260928.csv");
        const std::string t1 = ReadText(f1), t2 = ReadText(f2);
        const char* hdr = "Date, Time, Action, S2, S3, S4, S5, S6\r\n";
        CHECK(t1.compare(0, std::strlen(hdr), hdr) == 0 && Has(t1, ", Schedule, N25-3, on, -, ftp; stagger 607 s of 900") &&
                  Has(t2, "2026-09-28, 00:10:07.000, Schedule, N25-3, attempt 1: verified, 2026/09/28 00:00:00, test body\r\n"),
              "13. FTP_Log: golden header + one line per decision (<root>\\UploadFile\\yyyy\\mm\\FTP_Log_yyyymmdd.csv)");
        CHECK(!Has(t1 + t2, "assword") && !Has(t1 + t2, "handler"), "13. FTP_Log carries no password");

        ela::ScheduleLog("not installed");   // no-op
        CHECK(ela::ScheduleStatusJson() == "null" && !ela::ScheduleRunNow(ela::JOB_N25_3) &&
                  !ela::ScheduleSetJob(ela::JOB_N25_3, ela::JobFn()),
              "13. process-wide: not installed = no-op");
        ela::ScheduleTick();
        Recorder r4(&clk);
        ela::ScheduleConfig c4 = Cfg();
        c4.custCode = "851";
        c4.n25_3 = true;
        ela::ScheduleInstall(Setup(&clk, &c4, Sb("state\\s13d.ini")));
        CHECK(ela::ScheduleSetJob(ela::JOB_N25_3, Body(&r4)), "13. ScheduleSetJob after ScheduleInstall");
        clk.t = T(2026, 9, 30, 12, 0, 0);
        ela::ScheduleTick();
        ela::ScheduleRunNow(ela::JOB_N25_3);
        clk.t += 1;
        ela::ScheduleTick();
        g_logs.clear();
        ela::ScheduleLog("hello, R4");
        CHECK(r4.recs.size() == 1 && r4.recs[0].manual && Has(ela::ScheduleStatusJson(), "N25-3") && CountLogs("hello, R4") == 1,
              "13. ScheduleInstall / ScheduleTick / ScheduleRunNow / ScheduleLog / ScheduleStatusJson");
        ela::ScheduleUninstall();
        ela::ScheduleTick();
        CHECK(ela::ScheduleStatusJson() == "null", "13. ScheduleUninstall");
    }

    // 14. ReadScheduleConfig (read only)
    {
        const std::string ci = Sb("config.ini"), gi = Sb("Gerneral.ini");
        WriteText(ci,
                  "[Event Log]\r\nEnableAutoSaveEventLog=1\r\nbO10UseEventLogSaver=0\r\nEnableAutoSaveProductiont=1\r\n"
                  "bAlarmStatistAutoSaveNetDrive=1\r\n"
                  "EnanleTimePeriodSaveLog=1\r\nTimePeriodSaveLog=1\r\nAutoSaveProductionPath=\r\nasO19_SavePath=E:\\MTBF\r\n"
                  "bO19_AutoRecordReportByEveryWeek=1\r\niO19_WeekPeriod=3\r\n"
                  "[FTPUpLoad]\r\niN10UploadProductMethod=2\r\niN10UploadMethod=0\r\n"
                  "[ChipMos Function]\r\nbN25_3_EnableULJamLog=1\r\nsN25_2_FTPHost=10.1.2.3\r\nbN25_4_EnableUpload=1\r\n"
                  "[Production_Log]\r\nbN17UploadProdLog=1\r\nasN17ProductionLogPath=\\\\srv\\rms\\\r\n");
        WriteText(gi, "[System]\r\nCUSTOMER_CODE=851\r\nSPIL_FOR_QLE=1\r\n[Version]\r\nModel=HT-9045\r\nMachine ID=HT-90xx\r\n");
        const Stamp a = StampOf(ci), b = StampOf(gi);
        ela::ScheduleConfig c;
        CHECK(ela::ReadScheduleConfig(ci, gi, "HOSTX", &c), "14. read");
        CHECK(c.o06_1 && !c.o10Key && c.o06Production && c.o06TimePeriod && c.o06TimePeriodIdx == 1 && c.o06Path == "D:\\RMS" &&
                  c.o06UseNetDrive &&
                  c.o19Path == "E:\\MTBF" && c.o19Week && c.o19WeekDay == 3,
              "14. [Event Log]: O10 = EnableAutoSaveEventLog, O06 keys, \"\" path -> default D:\\RMS, O19 keys");
        CHECK(c.n10Daily && c.n10Method == 2 && c.n10UploadMethod == 0 && c.n10DrivePath == "D:\\RMS" && c.n10SpecifiedMinute == 0,
              "14. [FTPUpLoad]: missing bN10_DailyUploadProdData -> analyzer default true; drive path -> AutoSaveEventLogPath; "
              "missing dN10_3_1_SpecifiedTime -> 00:00 (W44 B: golden's never-loaded IniConfig field)");
        CHECK(c.n25_3 && c.n25Host == "10.1.2.3" && c.n17 && c.n17Path == "\\\\srv\\rms\\" && c.spil && c.n25_4 && !c.n25_5,
              "14. N25-3 / N25-4 (W22; bN25_5_EnableUpload missing -> off) / N17 keys; N17 on and not 999 -> SPIL naming");
        CHECK(c.custCode == "851" && c.spilForQle == 1 && c.machineType == "HT-9045" && c.machineId == "HT-90xx" &&
                  c.socketHandlerId == "HT-90xx" && c.hostName == "HOSTX" && c.pcName == "HOSTX" &&
                  ela::StaggerKey(c.machineId, c.hostName) == "HOSTX",
              "14. Gerneral.ini: customer code, SPIL_FOR_QLE, Model, Machine ID (HT-90xx -> host name for the stagger)");
        CHECK(SameStamp(a, StampOf(ci)) && SameStamp(b, StampOf(gi)), "14. read only: both files unchanged");
        ela::ScheduleConfig d;
        CHECK(!ela::ReadScheduleConfig(Sb("none.ini"), Sb("none2.ini"), "HOSTX", &d) && !d.o06_1 && d.n10Daily && !d.o06UseNetDrive &&
                  !d.n25_4 && !d.n25_5 &&
                  d.custCode == "000" && d.machineId.empty() && d.socketHandlerId == "29828",
              "14. no files: false, golden defaults");
    }

    // 16. the hub adapter: R2's O06SaveSummaryData through RunO06OnHub on a sandbox Hub (own config.ini / Gerneral.ini /
    //     JAM0000.dat / EventLogTxt / Production_Log), slot 2026-09-28 10:00 -> the file
    //     PJLD1014-SummaryData_2026-09-28.txt in AutoSaveProductionPath
    {
        const std::string hr = Sb("hub");
        ::CreateDirectoryA(hr.c_str(), NULL);
        ::CreateDirectoryA((hr + "\\EventLogTxt").c_str(), NULL);
        ::CreateDirectoryA((hr + "\\Production_Log").c_str(), NULL);
        PutEnv("W906_EVENTLOG_ROOT=" + hr + "\\EventLogTxt");
        PutEnv("W906_PRODLOG_ROOT=" + hr + "\\Production_Log");
        const std::string ci = hr + "\\config.ini", gi = hr + "\\Gerneral.ini";
        WriteText(ci, "[Event Log]\r\nEnableAutoSaveEventLog=1\r\nEnableAutoSaveProductiont=1\r\nAutoSaveProductionPath=" + hr +
                          "\\RMS_O06\r\n");
        WriteText(gi, "[System]\r\nCUSTOMER_CODE=000\r\n[Version]\r\nMachine ID=PJLD1014\r\n");
        ela::Options o;
        o.jamIniPath = hr + "\\JAM0000.dat";
        WriteText(o.jamIniPath, "[01 Input Arm]\r\n");
        ela::HubPaths hp;
        hp.configIni = ci;
        hp.generalIni = gi;
        CHECK(!StartsWithNoCase(o.jamIniPath, "D:\\HT9045") && !StartsWithNoCase(hp.configIni, "D:\\HT9045"),
              "16. the hub's files are in the sandbox");
        ela::Hub hub(o, hp);
        hub.Post(ela::EL_UPDATE_PARAMETER);
        hub.RunOnce();                                    // ReadConfig (its write-back goes to the sandbox files)
        ela::ScheduleConfig sc = Cfg();
        sc.machineId = "PJLD1014";
        ela::JobContext ctx;
        ctx.job = ela::JOB_O06_4;
        ctx.slot = T(2026, 9, 28, 10, 0, 0);
        ctx.slotDateTime = ela::LocalSecToDateTime(ctx.slot);
        ctx.attempt = 1;
        ctx.catchUp = false;
        ctx.manual = false;
        ctx.cfg = &sc;
        ctx.slotFlags = 0;
        const ela::JobStatus st = ela::RunO06OnHub(&hub, o, ctx);
        const std::string f = hr + "\\RMS_O06\\PJLD1014-SummaryData_2026-09-28.txt";
        CHECK(hub.Config().chkO06Production && st == ela::JOB_VERIFIED && Exists(f) && !ReadText(f).empty(),
              "16. RunO06OnHub: R2's summary for the slot's day, on disk and not empty = verified");
        CHECK(ela::RunO06OnHub(0, o, ctx) == ela::JOB_NO_BODY, "16. no hub: no body");

        FakeClock clk;
        ela::ScheduleConfig c4 = Cfg();
        CHECK(!ela::ScheduleUseHubJobs(&hub, o), "16. ScheduleUseHubJobs before ScheduleInstall: false");
        ela::ScheduleInstall(Setup(&clk, &c4, Sb("state\\s16.ini")));
        CHECK(ela::ScheduleUseHubJobs(&hub, o), "16. ScheduleUseHubJobs after ScheduleInstall");
        clk.t = T(2026, 9, 30, 12, 0, 0);
        ela::ScheduleTick();
        const std::string js = ela::ScheduleStatusJson();
        size_t bodies = 0, pos = 0;
        while ((pos = js.find("\"hasBody\":true", pos)) != std::string::npos) { ++bodies; ++pos; }
        CHECK(bodies == 7 && js.find("\"hasBody\":false") == std::string::npos,
              "16. all seven jobs have a body (six on the hub + N17; W22 added N25-4 / N25-5)");
        ela::ScheduleUninstall();
        PutEnv("W906_EVENTLOG_ROOT=");
        PutEnv("W906_PRODLOG_ROOT=" + Sb("Production_Log"));
    }

    // 17. O10: one source for every automatic job (ElaReports.h EffectiveO10; golden 906_0625_Steven cprod.cpp:2379-2394,
    //     V906 cprod.cpp:2519).  Nothing installed in a ctest: the config.ini formula with the injected function flag.
    {
        CHECK(ela::GetEffectiveO10Getter() == 0 && ela::O10FunctionFlagFallback(),
              "17. defaults: no getter, function flag on (InitialCosFunction sets bEventLogAutoSaveFunction for every customer)");
        CHECK(!ela::EffectiveO10FromIni(true, false, true) && ela::EffectiveO10FromIni(true, true, false) &&
                  ela::EffectiveO10FromIni(false, false, true) && !ela::EffectiveO10FromIni(false, true, false),
              "17. formula: function flag on -> EnableAutoSaveEventLog decides; off -> the stored bO10UseEventLogSaver decides");
        const std::string ci = Sb("o10_config.ini");
        WriteText(ci, "[Event Log]\r\nEnableAutoSaveEventLog=0\r\nbO10UseEventLogSaver=1\r\n");   // this machine's pattern
        const Stamp st0 = StampOf(ci);
        CHECK(!ela::EffectiveO10FromIni(true, ci) && ela::EffectiveO10FromIni(false, ci) && SameStamp(st0, StampOf(ci)),
              "17. from config.ini, EnableAutoSaveEventLog=0 / bO10UseEventLogSaver=1: flag on -> off, flag off -> on; read only");
        ela::ElaConfig ec;
        ec.cbO06 = false;
        ec.o10Stored = true;
        ela::ScheduleConfig sc = Cfg();
        sc.o06_1 = false;
        sc.o10Key = true;
        sc.o06Production = true;
        sc.o06TimePeriod = true;                                      // [O06-8] (★W43 C)
        std::string why1, why2;
        const bool a1 = ela::AutoJobsEnabled(ec), j1 = ela::JobEnabled(ela::JOB_O06_4, sc, false, true, &why1);
        ela::SetO10FunctionFlagFallback(false);
        const bool a2 = ela::AutoJobsEnabled(ec), j2 = ela::JobEnabled(ela::JOB_O06_4, sc, false, true, &why2);
        ela::SetO10FunctionFlagFallback(true);
        CHECK(!a1 && !j1 && Has(why1, "EnableAutoSaveEventLog") && a2 && j2,
              "17. no getter: R2 / R3 / R4 AutoJobsEnabled and the schedule gate give the same answer on both branches");

        g_liveO10 = true;
        ela::SetEffectiveO10Getter(&LiveO10);
        ela::ElaConfig off;                                           // config.ini says off on both keys
        const bool g1 = ela::AutoJobsEnabled(off);
        g_liveO10 = false;
        ela::ElaConfig on;
        on.cbO06 = true;
        on.o10Stored = true;
        const bool g2 = ela::AutoJobsEnabled(on);
        CHECK(g1 && !g2 && Has(ela::EffectiveO10Source(), "IniConfig"), "17. getter installed: it wins over config.ini");

        FakeClock clk;
        Recorder rec(&clk);
        ela::ScheduleConfig cfg = Cfg();
        cfg.o06_1 = false;                                            // config.ini off: only the getter can open the gate
        cfg.o10Key = false;
        cfg.o06Production = true;                                     // O06-4 every 30 min ([O06-8], ★W43 C), local
        cfg.o06TimePeriod = true;
        cfg.o06TimePeriodIdx = 1;
        g_liveO10 = true;
        ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s17.ini")));
        s.SetJob(ela::JOB_O06_4, Body(&rec));
        clk.t = T(2026, 9, 27, 9, 59, 0);
        Run(s, clk, T(2026, 9, 27, 10, 20, 0), 5);
        const size_t n1 = rec.recs.size();
        g_liveO10 = false;                                            // the Configuration page turned O10 off at 10:20
        Run(s, clk, T(2026, 9, 27, 11, 20, 0), 5);
        const size_t n2 = rec.recs.size();
        const std::string whyOff = s.State(ela::JOB_O06_4).why;
        g_liveO10 = true;                                             // and on again at 11:20
        Run(s, clk, T(2026, 9, 27, 11, 50, 0), 5);
        CHECK(n1 == 1 && rec.recs[0].slot == T(2026, 9, 27, 10, 0, 0) && n2 == 1 && Has(whyOff, "IniConfig") &&
                  rec.recs.size() == 2 && rec.recs[1].slot == T(2026, 9, 27, 11, 30, 0),
              "17. getter read at every decision: 10:00 runs, off at 10:20 -> 10:30 / 11:00 skipped, on at 11:20 -> 11:30 runs");
        ela::SetEffectiveO10Getter(0);
        CHECK(ela::GetEffectiveO10Getter() == 0 && Has(ela::EffectiveO10Source(), "config.ini"), "17. getter removed");
    }

    // 18. R6: what the automatic-jobs panel reads (ElaSchedule StatusJson through GET /api/ela/schedule) and the manual run
    {
        FakeClock clk;
        Recorder recN(&clk), recO(&clk);
        recN.script.push_back(ela::JOB_RETRY);                        // N25-3: the first try fails, the retry works
        ela::ScheduleConfig c = Cfg();
        c.custCode = "851";
        c.n25_3 = true;                                               // FTP, offset 607
        c.o06Production = true;                                       // O06-4 every 30 min ([O06-8], ★W43 C), local
        c.o06TimePeriod = true;
        c.o06TimePeriodIdx = 1;
        ela::ScheduleInstall(Setup(&clk, &c, Sb("state\\s18.ini")));
        ela::ScheduleSetJob(ela::JOB_N25_3, Body(&recN));
        ela::ScheduleSetJob(ela::JOB_O06_4, Body(&recO));
        clk.t = T(2026, 9, 27, 23, 58, 0);
        while (clk.t <= T(2026, 9, 28, 0, 10, 7)) { ela::ScheduleTick(); clk.t += 1; CheckBudget(); }   // AI(W906-ELA-SPEED) 20261001 (St02-E): the wall-clock budget here too
        std::string js = ela::ScheduleStatusJson();
        std::string n25 = js.substr(js.find("{\"id\":\"N25-3\""), js.find("{\"id\":\"O19-VTEST\"") - js.find("{\"id\":\"N25-3\""));
        const std::string o06 = js.substr(js.find("{\"id\":\"O06-4\""), js.find("{\"id\":\"N10-3\"") - js.find("{\"id\":\"O06-4\""));
        const std::string vt = js.substr(js.find("{\"id\":\"O19-VTEST\""), js.find("{\"id\":\"N17-UploadProdLog\"") - js.find("{\"id\":\"O19-VTEST\""));
        CHECK(Has(js, "\"now\":\"2026/09/28 00:10:07\"") && Has(js, "\"maxRetries\":6"), "18. StatusJson: now, maxRetries");
        CHECK(Has(n25, "\"pending\":true") && Has(n25, "\"attempt\":1") && Has(n25, "\"retries\":0") &&
                  Has(n25, "\"next\":\"2026/09/28 00:11:01\"") && Has(n25, "\"lastAt\":\"2026/09/28 00:10:07\"") &&
                  Has(n25, "\"lastStatus\":\"failed (retry)\"") && Has(n25, "\"lastDetail\":\"test body\"") &&
                  Has(n25, "\"lastManual\":false"),
              "18. N25-3 after a failed try: pending, next = the retry (00:11:01), last run / result / detail");
        CHECK(Has(o06, "\"lastStatus\":\"verified\"") && Has(o06, "\"lastAt\":\"2026/09/28 00:00:00\"") &&
                  Has(o06, "\"next\":\"2026/09/28 00:30:00\"") && Has(o06, "\"pending\":false"),
              "18. O06-4 verified at 00:00, next = the next slot 00:30");
        CHECK(Has(vt, "\"enabled\":false") && Has(vt, "915") && Has(vt, "\"next\":\"-\"") && Has(vt, "\"lastAt\":\"-\""),
              "18. VTEST off on 851: the closed gate is the reason, no next run");
        while (clk.t <= T(2026, 9, 28, 0, 12, 0)) { ela::ScheduleTick(); clk.t += 1; CheckBudget(); }   // AI(W906-ELA-SPEED) 20261001 (St02-E): the wall-clock budget here too
        js = ela::ScheduleStatusJson();
        n25 = js.substr(js.find("{\"id\":\"N25-3\""), js.find("{\"id\":\"O19-VTEST\"") - js.find("{\"id\":\"N25-3\""));
        CHECK(Has(n25, "\"lastStatus\":\"verified\"") && Has(n25, "\"lastAt\":\"2026/09/28 00:11:01\"") &&
                  Has(n25, "\"next\":\"2026/09/29 00:10:07\"") && Has(n25, "\"done\":\"2026/09/28 00:00:00\""),
              "18. N25-3 after the verified retry: next = tomorrow 00:00 + 607 s");

        const std::string hr = Sb("hub18");
        ::CreateDirectoryA(hr.c_str(), NULL);
        ela::Options o;
        o.jamIniPath = hr + "\\JAM0000.dat";
        ela::HubPaths hp;
        hp.configIni = hr + "\\config.ini";
        hp.generalIni = hr + "\\Gerneral.ini";
        ela::Hub hub(o, hp);                                          // not started, never ReadConfig: O10 off
        ela::QueryRequest base;
        int st = 0;
        std::string ct, bd;
        CHECK(ela::ServeHttp(&hub, base, "GET", "/api/ela/schedule", "", true, &st, &ct, &bd) && st == 200 &&
                  Has(bd, "\"installed\":true") && Has(bd, "\"o10\":false") && Has(bd, "\"schedule\":{\"now\"") &&
                  Has(bd, "\"id\":\"N17-UploadProdLog\"") && !Has(bd, "assword"),
              "18. GET /api/ela/schedule: installed, the effective O10, the schedule status; no password");
#ifdef W906_NO_SOFT_SIMULTE
        CHECK(Has(bd, "\"build\":\"ship\"") && Has(bd, "\"ftp\":\"WinINet\""), "18. ship build: WinINet (the page asks first)");
#else
        // AI(W906-ELA-W36) 20260928 (St02-E helper): ★W36 = C -- the sim build uploads like ship (was "log-only")
        CHECK(Has(bd, "\"build\":\"sim\"") && Has(bd, "\"ftp\":\"WinINet\""), "18. SIM build: WinINet too (★W36 = C, the page asks first)");
#endif
        CHECK(ela::ServeHttp(&hub, base, "POST", "/api/ela/job", "id=N25-3", false, &st, &ct, &bd) && st == 403,
              "18. POST /api/ela/job on a read-only server: 403");
        CHECK(ela::ServeHttp(&hub, base, "POST", "/api/ela/job", "id=N99", true, &st, &ct, &bd) && st == 400 &&
                  Has(bd, "O06-4"),
              "18. POST /api/ela/job with an unknown id: 400 (lists the ids)");
        CHECK(ela::ServeHttp(&hub, base, "POST", "/api/ela/job", "id=N25-3", true, &st, &ct, &bd) && st == 202 &&
                  Has(bd, "\"job\":\"N25-3\""),
              "18. POST /api/ela/job?id=N25-3: 202 queued");
        const size_t before = recN.recs.size();
        ela::ScheduleTick();
        CHECK(recN.recs.size() == before + 1 && recN.recs.back().manual, "18. the queued run happens at the next tick, as a manual run");
        CHECK(ela::ServeHttp(&hub, base, "GET", "/api/ela/nope", "", true, &st, &ct, &bd) && st == 404, "18. other sub-paths: 404");
        ela::ScheduleUninstall();
        CHECK(ela::ServeHttp(&hub, base, "GET", "/api/ela/schedule", "", true, &st, &ct, &bd) && st == 200 &&
                  Has(bd, "\"installed\":false") && Has(bd, "\"schedule\":null"),
              "18. no scheduler: installed false, schedule null");
        CHECK(ela::ServeHttp(&hub, base, "POST", "/api/ela/job", "id=N25-3", true, &st, &ct, &bd) && st == 503,
              "18. no scheduler: POST /api/ela/job 503");
    }

    // 19. ★W44 B (Steven 20260928; AI(W906-ELA-W44) 20260928, St02-E helper): N10-3 method 3 (specified time) once a
    //     day at the Handler's specified time.  Golden 906_0625_Steven HS_Function.cpp:488-509: once while the clock is
    //     in that minute (bN10_3_1_Flag), no catch-up.  ★W44-2 = catch up (Steven 20260928; AI(W906-ELA-W44B) 20260928):
    //     a time missed while the Handler was off runs once at boot (D-b).  Local target (offset 0); a fake clock
    //     stepping 10 s (every time below is on that grid).
    {
        ela::ScheduleConfig cfg = Cfg();
        cfg.n10Daily = true;
        cfg.n10Method = 3;
        cfg.n10SpecifiedMinute = 7 * 60 + 30;                        // 07:30
        const std::string sf = Sb("state\\s19.ini");
        FakeClock clk;
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 9, 28, 6, 0, 0);
            Run(s, clk, T(2026, 9, 28, 7, 29, 59), 10);
            const size_t before = rec.recs.size();
            Run(s, clk, T(2026, 9, 29, 7, 29, 59), 10);
            const size_t day1 = rec.recs.size();
            Run(s, clk, T(2026, 9, 29, 9, 0, 0), 10);
            CHECK(before == 0 && s.State(ela::JOB_N10_3).kind == ela::KIND_LOCAL && s.State(ela::JOB_N10_3).offsetSec == 0,
                  "19. not before the time: 06:00 .. 07:29:59 no run (local target, no stagger)");
            CHECK(day1 == 1 && rec.recs[0].slot == T(2026, 9, 28, 7, 30, 0) && rec.recs[0].at == T(2026, 9, 28, 7, 30, 0) &&
                      !rec.recs[0].catchUp && !rec.recs[0].manual,
                  "19. at 07:30:00 once, and not again over the rest of the day");
            CHECK(rec.recs.size() == 2 && rec.recs[1].slot == T(2026, 9, 29, 7, 30, 0) && rec.recs[1].at == T(2026, 9, 29, 7, 30, 0) &&
                      s.State(ela::JOB_N10_3).done == T(2026, 9, 29, 7, 30, 0),
                  "19. the next day at 07:30 again (done = the last verified time, in the state file)");
        }
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 9, 29, 7, 30, 30);
            Run(s, clk, T(2026, 9, 29, 23, 59, 59), 60);   // AI(W906-ELA-SPEED) 20261001 (St02-E): 60 s grid (was 5 s; nothing may run)
            CHECK(rec.recs.empty(), "19. restart the same day inside the minute (07:30:30) and after: the time is done, no second run");
        }
        {
            Recorder rec(&clk);
            g_logs.clear();
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 9, 30, 8, 0, 0);                         // off over 09-30 07:30
            Run(s, clk, T(2026, 10, 1, 7, 29, 59), 10);
            // AI(W906-ELA-W44B) 20260928 (St02-E helper): ★W44-2 = catch up -- was "not run, logged missed" (★W44 B, golden)
            const bool one = rec.recs.size() == 1 && rec.recs[0].catchUp && !rec.recs[0].manual &&
                             rec.recs[0].slot == T(2026, 9, 30, 7, 30, 0) && rec.recs[0].at == T(2026, 9, 30, 8, 0, 0) &&
                             s.State(ela::JOB_N10_3).done == T(2026, 9, 30, 7, 30, 0);
            Run(s, clk, T(2026, 10, 1, 8, 0, 0), 10);
            CHECK(one && CountLogs("N10-3, boot catch-up, 2026/09/30 07:30:00, catch-up for 2026/09/30 07:30") == 1 &&
                      CountLogs("N10-3, attempt 1 (boot catch-up): verified, 2026/09/30 07:30:00") == 1 &&
                      CountLogs("N10-3", "missed") == 0,
                  "19. ★W44-2: the time passed while the Handler was off (on at 08:00): caught up ONCE at boot, in FTP_Log");
            CHECK(rec.recs.size() == 2 && rec.recs[1].slot == T(2026, 10, 1, 7, 30, 0) && !rec.recs[1].catchUp,
                  "19. ... and the next day at 07:30 as usual");
        }
        {
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, sf));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 10, 2, 7, 30, 20);                       // on inside the minute, last verified 10-01
            Run(s, clk, T(2026, 10, 2, 12, 0, 0), 10);
            CHECK(rec.recs.size() == 1 && rec.recs[0].slot == T(2026, 10, 2, 7, 30, 0) && rec.recs[0].at == T(2026, 10, 2, 7, 30, 20) &&
                      !rec.recs[0].catchUp,
                  "19. on inside the minute (07:30:20): runs once at once, as golden's next timer tick (not a catch-up)");
        }
        {
            // ★W44-2 (AI(W906-ELA-W44B) 20260928): off over two times (10-03 and 10-04 07:30), on before today's (06:00):
            //   D-b catches up the most recent missed time only, then today's time runs as usual; a restart after both = none
            Recorder rec(&clk);
            g_logs.clear();
            {
                ela::Scheduler s(Setup(&clk, &cfg, sf));
                s.SetJob(ela::JOB_N10_3, Body(&rec));
                clk.t = T(2026, 10, 5, 6, 0, 0);
                Run(s, clk, T(2026, 10, 5, 8, 0, 0), 10);
            }
            const bool two = rec.recs.size() == 2 && rec.recs[0].catchUp && rec.recs[0].slot == T(2026, 10, 4, 7, 30, 0) &&
                             rec.recs[0].at == T(2026, 10, 5, 6, 0, 0) && !rec.recs[1].catchUp &&
                             rec.recs[1].slot == T(2026, 10, 5, 7, 30, 0) && rec.recs[1].at == T(2026, 10, 5, 7, 30, 0);
            {
                ela::Scheduler s(Setup(&clk, &cfg, sf));
                s.SetJob(ela::JOB_N10_3, Body(&rec));
                clk.t = T(2026, 10, 5, 9, 0, 0);
                Run(s, clk, T(2026, 10, 5, 23, 0, 0), 30);
            }
            CHECK(two && CountLogs("catch-up for 2026/10/04 07:30") == 1 && CountLogs("catch-up for 2026/10/03") == 0,
                  "19. ★W44-2: off over 10-03 and 10-04: one catch-up at boot (06:00) for the latest (10-04 07:30), no backlog");
            CHECK(rec.recs.size() == 2, "19. ★W44-2: restart the same day after both runs: nothing more (done=)");
        }
        {
            // ★W44-2 keeps D-b's first-run rule: no record = nothing known to be missed
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s19c.ini")));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 10, 6, 8, 0, 0);
            Run(s, clk, T(2026, 10, 7, 7, 29, 30), 30);
            CHECK(rec.recs.empty() && Has(ReadText(Sb("state\\s19c.ini")), "armed=2026/10/06 08:00:00"),
                  "19. ★W44-2: the first run (no record) after the time: no catch-up, armed written");
        }
        {
            // the time changed while running (ReloadConfig); a fresh state file (first run = armed)
            ela::ScheduleConfig c2 = cfg;
            Recorder rec(&clk);
            ela::Scheduler s(Setup(&clk, &c2, Sb("state\\s19b.ini")));
            s.SetJob(ela::JOB_N10_3, Body(&rec));
            clk.t = T(2026, 10, 5, 7, 0, 0);
            Run(s, clk, T(2026, 10, 5, 7, 9, 59), 10);
            c2.n10SpecifiedMinute = 6 * 60;                          // 06:00: already past today
            s.ReloadConfig();
            Run(s, clk, T(2026, 10, 5, 7, 39, 59), 10);
            const bool none0600 = rec.recs.empty();
            c2.n10SpecifiedMinute = 9 * 60 + 15;                     // 09:15: still ahead today
            s.ReloadConfig();
            Run(s, clk, T(2026, 10, 5, 9, 59, 59), 10);
            const bool at0915 = rec.recs.size() == 1 && rec.recs[0].slot == T(2026, 10, 5, 9, 15, 0) &&
                                rec.recs[0].at == T(2026, 10, 5, 9, 15, 0);
            c2.n10SpecifiedMinute = 9 * 60 + 40;                     // 09:40 at 10:00: after the last run, before now
            s.ReloadConfig();
            Run(s, clk, T(2026, 10, 6, 9, 39, 59), 10);
            const bool none0940 = rec.recs.size() == 1;
            Run(s, clk, T(2026, 10, 6, 11, 0, 9), 10);
            const bool next0940 = rec.recs.size() == 2 && rec.recs[1].slot == T(2026, 10, 6, 9, 40, 0);
            c2.n10SpecifiedMinute = 11 * 60;                         // 11:00 at 11:00:10: inside the minute
            s.ReloadConfig();
            Run(s, clk, T(2026, 10, 6, 12, 0, 0), 10);
            CHECK(none0600, "19. time changed to 06:00 at 07:10: not run (already past today) -- and 07:30 no longer either");
            CHECK(at0915, "19. then changed to 09:15 at 07:40: runs at 09:15 the same day");
            CHECK(none0940 && next0940,
                  "19. changed to 09:40 at 10:00 (after the 09:15 run): not run now, the next day at 09:40 (re-based)");
            CHECK(rec.recs.size() == 3 && rec.recs[2].slot == T(2026, 10, 6, 11, 0, 0) && rec.recs[2].at == T(2026, 10, 6, 11, 0, 10),
                  "19. changed to 11:00 at 11:00:10: inside the minute, runs at once (golden's next tick matches)");
        }
        {
            // ReadScheduleConfig: [FTPUpLoad] dN10_3_1_SpecifiedTime (the IniConfig field's name; golden has no reader).
            // AI(W906-ELA-W44-1) 20260928 (St02-E helper): ★W44-1 = A -- the picker is saved by HTEditList as
            //   FloatToStr(Ttp->DateTime) (golden 906_0625_Steven Public/HTEditList.cpp:925, V906 Public/HTEditList.cpp:979-988:
            //   15 significant digits), not "%0.4f".  00:11 locks that: FloatToStr 0.00763888888888889 = 00:11, while a
            //   %0.4f 0.0076 would read back as 00:10 (656.64 s).
            struct KV { const char* line; int minute; };
            const KV kv[] = {
                { "dN10_3_1_SpecifiedTime=0.3125\r\n", 7 * 60 + 30 },            // 07:30 as FloatToStr text
                { "dN10_3_1_SpecifiedTime=0.00763888888888889\r\n", 11 },        // 00:11 as FloatToStr text (W44-1)
                { "dN10_3_1_SpecifiedTime=40162.5441927662\r\n", 13 * 60 + 3 },  // the dfm's design value (13:03:38)
                { "dN10_3_1_SpecifiedTime=abc\r\n", 0 },                         // unparseable -> 0.0 (ReadFloat default)
                { "", 0 },                                                        // missing -> 0.0 = 00:00, golden's field
            };
            bool ok = true;
            ela::ScheduleConfig c;
            for (size_t i = 0; i < sizeof(kv) / sizeof(kv[0]); ++i)
            {
                char name[32];
                std::snprintf(name, sizeof(name), "s19_config_%u.ini", (unsigned)i);
                const std::string ci = Sb(name);
                WriteText(ci, std::string("[FTPUpLoad]\r\nbN10_DailyUploadProdData=1\r\niN10UploadProductMethod=3\r\n") + kv[i].line);
                c = ela::ScheduleConfig();
                const bool r = ela::ReadScheduleConfig(ci, Sb("no_general_19.ini"), "HOSTX", &c);
                if (!(r && c.n10Daily && c.n10Method == 3 && c.n10SpecifiedMinute == kv[i].minute))
                {
                    printf("    kv %u: read %d, minute %d (want %d)\n", (unsigned)i, (int)r, c.n10SpecifiedMinute, kv[i].minute);
                    ok = false;
                }
            }
            CHECK(ok, "19. ReadScheduleConfig: dN10_3_1_SpecifiedTime -> the minute of the day (07:30, 00:11 as FloatToStr, 13:03; "
                      "bad / missing = 00:00)");
            LocalSec s = 0, n = 0;
            const ela::Calendar k = ela::JobCalendar(ela::JOB_N10_3, c, false);
            CHECK(k.rules.size() == 1 && k.Latest(T(2026, 10, 7, 12, 0, 0), &s) && s == T(2026, 10, 7, 0, 0, 0) && k.Next(s, &n) &&
                      n == T(2026, 10, 8, 0, 0, 0),
                  "19. no key: N10-3 once a day at 00:00 (golden's Handler uploads at 00:00 too: the field is never loaded)");
        }
    }

    // 20. ★W43 = C (Steven 20260928; AI(W906-ELA-W43) 20260928, St02-E helper): O06-4 follows the Handler's [O06-8]
    //     "Update Production Record every 10 / 30 minutes" ([Event Log] EnanleTimePeriodSaveLog / TimePeriodSaveLog;
    //     golden 906_0625_Steven cConfiguration.cpp:3950 / :3960).  Local target (offset 0), a fake clock.
    {
        const char* names[3] = { "s20_off.ini", "s20_10.ini", "s20_30.ini" };
        int count[3] = { -1, -1, -1 };
        bool aligned[3] = { false, false, false };
        std::string why0;
        for (int mode = 0; mode < 3; ++mode)                         // 0 = [O06-8] unchecked, 1 = 10 min, 2 = 30 min
        {
            FakeClock clk;
            Recorder rec(&clk);
            ela::ScheduleConfig cfg = Cfg();
            cfg.o06Production = true;
            cfg.o06TimePeriod = mode != 0;
            cfg.o06TimePeriodIdx = mode == 2 ? 1 : 0;
            ela::Scheduler s(Setup(&clk, &cfg, Sb((std::string("state\\") + names[mode]).c_str())));
            s.SetJob(ela::JOB_O06_4, Body(&rec));
            clk.t = T(2026, 10, 7, 0, 0, 30);                        // one day: 00:00:30 .. the next 00:00:00
            Run(s, clk, T(2026, 10, 8, 0, 0, 29), 30);
            count[mode] = (int)rec.recs.size();
            const LocalSec p = mode == 2 ? 1800 : 600;
            bool ok = true;
            for (size_t i = 0; ok && i < rec.recs.size(); ++i)
                ok = rec.recs[i].slot == T(2026, 10, 7, 0, 0, 0) + p * (LocalSec)(i + 1) && rec.recs[i].at == rec.recs[i].slot &&
                     !rec.recs[i].manual && !rec.recs[i].catchUp;
            aligned[mode] = ok;
            if (mode == 0) why0 = s.State(ela::JOB_O06_4).why;
        }
        CHECK(count[0] == 0 && Has(why0, "O06-8"),
              "20. ★W43 = C: [O06-8] unchecked -- no O06-4 save over a whole day (the closed gate says O06-8)");
        CHECK(count[1] == 144 && aligned[1], "20. [O06-8] 10 min: 144 saves, 00:10 .. the next 00:00, on the slot");
        CHECK(count[2] == 48 && aligned[2], "20. [O06-8] 30 min: 48 saves, 00:30 .. the next 00:00, on the slot");

        FakeClock clk;
        Recorder rec(&clk);
        ela::ScheduleConfig cfg = Cfg();
        cfg.o06Production = true;                                     // [O06-8] unchecked at the start
        ela::Scheduler s(Setup(&clk, &cfg, Sb("state\\s20_t.ini")));
        s.SetJob(ela::JOB_O06_4, Body(&rec));
        clk.t = T(2026, 10, 7, 9, 0, 0);
        Run(s, clk, T(2026, 10, 7, 9, 25, 0), 5);
        const bool none = rec.recs.empty() && !s.State(ela::JOB_O06_4).enabled;
        cfg.o06TimePeriod = true;                                     // checked at 09:25 (10 min)
        s.ReloadConfig();
        Run(s, clk, T(2026, 10, 7, 10, 5, 0), 5);
        bool four = rec.recs.size() == 4;
        for (size_t i = 0; four && i < rec.recs.size(); ++i) four = rec.recs[i].slot == T(2026, 10, 7, 9, 30, 0) + 600 * (LocalSec)i;
        cfg.o06TimePeriod = false;                                    // unchecked again at 10:05
        s.ReloadConfig();
        Run(s, clk, T(2026, 10, 7, 11, 0, 0), 5);
        const bool stopped = rec.recs.size() == 4 && !s.State(ela::JOB_O06_4).enabled && Has(s.State(ela::JOB_O06_4).why, "O06-8");
        s.RunNow(ela::JOB_O06_4);
        clk.t += 5;
        s.Tick();
        CHECK(none && four, "20. checked while running (09:25): 09:30 / 09:40 / 09:50 / 10:00 run; 09:20 (passed while off) is not run");
        CHECK(stopped, "20. unchecked again (10:05): no more timed saves");
        CHECK(rec.recs.size() == 5 && rec.recs[4].manual, "20. a manual run (Auto Jobs page) still works with [O06-8] off: not a timed save");
    }

    // 15. the machine files
    CHECK(SameStamp(s0Cfg, StampOf(realCfg)) && SameStamp(s0Gen, StampOf(realGen)) && SameStamp(s0State, StampOf(realState)),
          "15. D:\\HT9045\\config\\config.ini, D:\\HT9045\\system\\Gerneral.ini, D:\\HT9045_Log\\UploadFile\\ElaScheduleState.ini untouched");

    printf("=== ELA_Schedule: %d passed, %d failed ===\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail == 0 ? 0 : 1;
}
