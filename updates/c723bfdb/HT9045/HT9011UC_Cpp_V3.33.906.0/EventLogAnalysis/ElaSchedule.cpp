// ===========================================================================
//  EventLogAnalysis/ElaSchedule.cpp -- see ElaSchedule.h.  AI(W906-ELA-R5) 20260927 (St02-E).
//
//  Wiring (merge time; ElaService.cpp / ElaHub.cpp are not edited here -- four lines):
//    W906_ElaStart (ElaService.cpp), after the SetDbiProcessHook line and before `if (!g_hub->Start())`:
//        ela::ScheduleInstall(ela::ScheduleSetup());   // AI(W906-ELA-R5): time-based jobs (plan §2 / §3)
//        ela::ScheduleUseHubJobs(g_hub, o);            // AI(W906-ELA-R5): the R2 / R4 bodies on this hub
//    Hub::Entry (ElaHub.cpp), inside the do-loop right after `h->RunOnce();`:
//        ela::ScheduleTick();                          // AI(W906-ELA-R5): same worker = one job / transfer at a time
//    W906_ElaStop (ElaService.cpp), after `h->Stop();` and before `delete h;` (worker joined; the bodies hold h):
//        ela::ScheduleUninstall();
//    EL_UPDATE_PARAMETER (Hub::RunOnce) calls ela::ScheduleReloadConfig() (★W44-1, AI(W906-ELA-W44-1) 20260928: the
//    Configuration save re-bases the schedule at the next Tick; otherwise config is re-read every 60 s).
// ===========================================================================
#include "EventLogAnalysis/ElaSchedule.h"
#include "EventLogAnalysis/ElaHub.h"      // EventLog_COMMAND values (CustomerCodeAllows); Hub (the adapter at the end)
#include "EventLogAnalysis/ElaReports.h"  // R2 bodies: O06SaveSummaryData / N10SaveSummaryData / DoVTestSaveSummary
#include "EventLogAnalysis/ElaChipMos.h"  // R4 body: UploadJamCode
#include "vclcompat/IniFiles.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using vclcompat::AnsiString;
using vclcompat::TIniFile;

namespace ela {

// ===========================================================================
//  time
// ===========================================================================
static long long FloorDiv(long long a, long long b)
{
    long long q = a / b;
    if ((a % b) != 0 && ((a < 0) != (b < 0))) --q;
    return q;
}

// Howard Hinnant's days_from_civil / civil_from_days (proleptic Gregorian, day 0 = 1970-01-01)
static long long DaysFromCivil(long long y, int m, int d)
{
    y -= (m <= 2) ? 1 : 0;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const long long yoe = y - era * 400;
    const long long mp = (m > 2) ? m - 3 : m + 9;
    const long long doy = (153 * mp + 2) / 5 + d - 1;
    const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void CivilFromDays(long long z, int* y, int* m, int* d)
{
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const long long doe = z - era * 146097;
    const long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long long mp = (5 * doy + 2) / 153;
    const long long dd = doy - (153 * mp + 2) / 5 + 1;
    const long long mm = mp < 10 ? mp + 3 : mp - 9;
    *y = (int)(yoe + era * 400 + (mm <= 2 ? 1 : 0));
    *m = (int)mm;
    *d = (int)dd;
}

static long long EpochDays() { return DaysFromCivil(1899, 12, 30); }   // TDateTime 0.0

LocalSec LocalSecFromCivil(int y, int m, int d, int hh, int nn, int ss)
{
    return (DaysFromCivil(y, m, d) - EpochDays()) * 86400LL + hh * 3600LL + nn * 60LL + ss;
}

void CivilFromLocalSec(LocalSec t, int* y, int* m, int* d, int* hh, int* nn, int* ss)
{
    const long long day = FloorDiv(t, 86400);
    const long long sec = t - day * 86400;
    CivilFromDays(day + EpochDays(), y, m, d);
    *hh = (int)(sec / 3600);
    *nn = (int)((sec / 60) % 60);
    *ss = (int)(sec % 60);
}

int DayOfWeek0(LocalSec t)
{
    const long long day = FloorDiv(t, 86400);   // day 0 = Saturday 1899-12-30
    return (int)(((day + 6) % 7 + 7) % 7);
}

std::string FormatLocalSec(LocalSec t)
{
    if (t < 0) return "-";
    int y, m, d, hh, nn, ss;
    CivilFromLocalSec(t, &y, &m, &d, &hh, &nn, &ss);
    char b[32];
    std::snprintf(b, sizeof(b), "%04d/%02d/%02d %02d:%02d:%02d", y, m, d, hh, nn, ss);
    return b;
}

bool ParseLocalSec(const std::string& s, LocalSec* t)
{
    int y = 0, m = 0, d = 0, hh = 0, nn = 0, ss = 0;
    if (std::sscanf(s.c_str(), "%d/%d/%d %d:%d:%d", &y, &m, &d, &hh, &nn, &ss) != 6) return false;
    if (y < 1900 || y > 9999 || m < 1 || m > 12 || d < 1 || d > 31 || hh < 0 || hh > 23 || nn < 0 || nn > 59 || ss < 0 ||
        ss > 59)
        return false;
    *t = LocalSecFromCivil(y, m, d, hh, nn, ss);
    return true;
}

LocalSec SchedSystemClock::Now()
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    return LocalSecFromCivil(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

// ===========================================================================
//  small helpers
// ===========================================================================
static std::string EnvOr(const char* name, const char* goldenLiteral)
{
    const char* e = std::getenv(name);
    return (e != 0 && *e != 0) ? std::string(e) : std::string(goldenLiteral);
}

static std::string IntStr(long long v)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%ld", (long)v);   // every value here fits a long (sizes: see SizeStr)
    return b;
}

static std::string SizeStr(unsigned long long v)
{
    char b[32];
    int i = 31;
    b[i] = 0;
    do { b[--i] = (char)('0' + (int)(v % 10)); v /= 10; } while (v != 0 && i > 0);
    return std::string(b + i);
}

static bool FileExistsA_(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static bool DirExistsA_(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static bool FileSizeA_(const std::string& p, unsigned long long* n)
{
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &fa)) return false;
    if (fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
    *n = ((unsigned long long)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
    return true;
}

static void ForceDirs(const std::string& dir)
{
    if (dir.empty() || DirExistsA_(dir)) return;
    const size_t p = dir.find_last_of("\\/");
    if (p != std::string::npos && p > 0) ForceDirs(dir.substr(0, p));
    ::CreateDirectoryA(dir.c_str(), NULL);
}

static std::string ThisHostName()
{
    char b[256];
    DWORD n = sizeof(b);
    if (::GetComputerNameExA(ComputerNameDnsHostname, b, &n)) return std::string(b, n);   // = gethostname
    n = sizeof(b);
    if (::GetComputerNameA(b, &n)) return std::string(b, n);
    return std::string();
}

static std::string Trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) --b;
    return s.substr(a, b - a);
}

static int CustInt(const std::string& c) { return std::atoi(Trim(c).c_str()); }

// one FTP_Log column: no comma (the file is a CSV), no line break
static std::string Col(const std::string& s)
{
    std::string o = s;
    for (size_t i = 0; i < o.size(); ++i)
    {
        if (o[i] == ',') o[i] = ';';
        else if (o[i] == '\r' || o[i] == '\n') o[i] = ' ';
    }
    return o;
}

static std::string J(const std::string& s)
{
    std::string o = "\"";
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c < 0x20)
        {
            char b[8];
            std::snprintf(b, sizeof(b), "\\u%04x", c);
            o += b;
        }
        else o += (char)c;
    }
    return o + "\"";
}

// ===========================================================================
//  names
// ===========================================================================
const char* JobName(int job)
{
    static const char* const k[JOB_TOTAL] = { "O06-4", "N10-3", "N25-3", "O19-VTEST", "N17-UploadProdLog", "N25-4",
                                              "N25-5" };   // AI(W906-ELA-W22) 20260928: the last two appended
    return (job >= 0 && job < JOB_TOTAL) ? k[job] : "?";
}

const char* KindName(int kind)
{
    return kind == KIND_FTP ? "ftp" : kind == KIND_NETDRIVE ? "netdrive" : "local";
}

const char* StatusName(int status)
{
    switch (status)
    {
    case JOB_VERIFIED: return "verified";
    case JOB_NOTHING_TO_DO: return "nothing to do";
    case JOB_RETRY: return "failed (retry)";
    case JOB_FAILED: return "failed (not retried)";
    case JOB_NO_BODY: return "no job body";
    }
    return "?";
}

// ===========================================================================
//  config (read only)
// ===========================================================================
ScheduleConfig::ScheduleConfig()
    : o06_1(false), o10Key(false), custCode("000"), o06Production(false), o06TimePeriod(false), o06TimePeriodIdx(0),
      o06Path("D:\\RMS"), o06UseNetDrive(false), n10Daily(true), n10Method(1), n10UploadMethod(1), n10DrivePath("D:\\RMS"),
      n10SpecifiedMinute(-1), n25_3(false),
      n25Host("10.20.50.3"), n25_4(false), n25_5(false), o19Week(false), o19WeekDay(0), o19Path("D:\\MTBF_Summary"), n17(false), n17Path("D:\\RMS\\"),
      prodLogRoot(EnvOr("W906_PRODLOG_ROOT", "D:\\HT9045_Log\\Production_Log")), machineType("HT-9046"),
      socketHandlerId("29828"), spil(false), spilForQle(0)
{
}

namespace {
// CheckAndReadIniData without the write-back (golden Common.cpp:107-167): missing key = default; a string that is
// "" while the default is not -> the default
struct IniRead
{
    TIniFile ini;
    explicit IniRead(const std::string& f) : ini(AnsiString(f.c_str())) {}
    bool Bool(const char* g, const char* n, bool def)
    {
        return IniBoolOverride(ini.FileName.c_str(), g, n, ini.ValueExists(AnsiString(g), AnsiString(n)) ? ini.ReadBool(AnsiString(g), AnsiString(n), def) : def);   // AI(W906-SIM-W36-1) 20260928 (St02-E helper), W58 20260930: the SIM mask (ElaHub.h)
    }
    int Int(const char* g, const char* n, int def)
    {
        return ini.ValueExists(AnsiString(g), AnsiString(n)) ? ini.ReadInteger(AnsiString(g), AnsiString(n), def) : def;
    }
    double Float(const char* g, const char* n, double def)   // AI(W906-ELA-W44) 20260928: TIniFile::ReadFloat
    {
        return ini.ValueExists(AnsiString(g), AnsiString(n)) ? ini.ReadFloat(AnsiString(g), AnsiString(n), def) : def;
    }
    std::string Str(const char* g, const char* n, const std::string& def)
    {
        if (!ini.ValueExists(AnsiString(g), AnsiString(n))) return def;
        std::string s = ini.ReadString(AnsiString(g), AnsiString(n), AnsiString(def.c_str())).c_str();
        return (s.empty() && !def.empty()) ? def : s;
    }
};
// ★W44 B: golden `TDateTime dtPickerTime(...); DecodeTime(dtPickerTime, specifiedhour, specifiedminute, ...)`
//   (HS_Function.cpp:492-494): the time of day only.  NaN / inf / a huge value = golden's 0.0 (00:00).
int SpecifiedMinuteOfDay(double v)
{
    if (!(v >= -693594.0 && v <= 2958466.0)) v = 0.0;
    int y, m, d, h, n, s, ms;
    DecodeDateTime(v, &y, &m, &d, &h, &n, &s, &ms);
    return h * 60 + n;
}
}  // namespace

bool ReadScheduleConfig(const std::string& configIni, const std::string& generalIni, const std::string& hostName,
                        ScheduleConfig* out)
{
    ScheduleConfig c;
    const bool ok = FileExistsA_(configIni);
    {
        IniRead f(configIni);
        // D-a inputs; the effective O10 is decided at each decision by EffectiveO10 (ElaReports.h, golden cprod.cpp:2379-2394)
        c.o06_1            = f.Bool("Event Log", "EnableAutoSaveEventLog", false);      // analyzer ReadConfig :3093
        c.o10Key           = f.Bool("Event Log", "bO10UseEventLogSaver", false);        // cConfiguration.cpp:3980
        // analyzer-owned (Rev891 ReadConfig :3096-3120, the analyzer's defaults)
        c.o06Production    = f.Bool("Event Log", "EnableAutoSaveProductiont", false);
        c.o06TimePeriod    = f.Bool("Event Log", "EnanleTimePeriodSaveLog", false);
        c.o06TimePeriodIdx = f.Int("Event Log", "TimePeriodSaveLog", 0);
        c.o06Path          = f.Str("Event Log", "AutoSaveProductionPath", "D:\\RMS");
        c.o06UseNetDrive   = f.Bool("Event Log", "bAlarmStatistAutoSaveNetDrive", false);
        const std::string evPath = f.Str("Event Log", "AutoSaveEventLogPath", "D:\\RMS");
        c.n10Daily         = f.Bool("FTPUpLoad", "bN10_DailyUploadProdData", true);
        c.n10Method        = f.Int("FTPUpLoad", "iN10UploadProductMethod", 1);
        c.n10UploadMethod  = f.Int("FTPUpLoad", "iN10UploadMethod", 1);
        c.n10DrivePath     = f.Str("FTPUpLoad", "sN10UploadDrivePath", evPath);
        // ★W44 B (Steven 20260928; AI(W906-ELA-W44) 20260928, St02-E helper): method 3 runs once a day at the Handler's
        //   IniConfig.dN10_3_1_SpecifiedTime (906_0625_Steven Config.h:1030, read only at HS_Function.cpp:492).  Golden
        //   never loads or saves that field: the picker dtpN10_3_1_SpecifiedTime (cConfiguration.dfm:15395,
        //   cConfiguration.h:2160) is in no elConfig->Add (cConfiguration.cpp:3305-3306 / :3346 / :3369 bind only
        //   bN10_DailyUploadProdData and iN10UploadProductMethod), and IniConfig is a zero-initialised global
        //   (cprod.cpp:78), so golden's Handler uploads at 00:00.  V906 is the same (Config.h:1031; no reader in
        //   FileRW/IniConfig.gen.inc).  The key read here, [FTPUpLoad] dN10_3_1_SpecifiedTime, is the name the
        //   Handler's HTEditList would give the field (its neighbours' section, the field name as the key); missing =
        //   0.0 = 00:00, golden's value.  Read only, as every key here.
        c.n10SpecifiedMinute = SpecifiedMinuteOfDay(f.Float("FTPUpLoad", "dN10_3_1_SpecifiedTime", 0.0));
        c.o19Path          = f.Str("Event Log", "asO19_SavePath", "D:\\MTBF_Summary");
        c.n25Host          = f.Str("ChipMos Function", "sN25_2_FTPHost", "10.20.50.3");
        // Handler-owned triggers (906_0625_Steven cConfiguration.cpp defaults)
        c.n25_3            = f.Bool("ChipMos Function", "bN25_3_EnableULJamLog", false);          // :3743
        c.n25_4            = f.Bool("ChipMos Function", "bN25_4_EnableUpload", false);            // :3745 (W22)
        c.n25_5            = f.Bool("ChipMos Function", "bN25_5_EnableUpload", false);            // :3747 (W22)
        c.o19Week          = f.Bool("Event Log", "bO19_AutoRecordReportByEveryWeek", false);      // :4074
        c.o19WeekDay       = f.Int("Event Log", "iO19_WeekPeriod", 0);                            // :4075
        c.n17              = f.Bool("Production_Log", "bN17UploadProdLog", false);                // :3582
        c.n17Path          = f.Str("Production_Log", "asN17ProductionLogPath", "D:\\RMS\\");      // :3583
    }
    {
        IniRead g(generalIni);
        c.custCode    = g.Str("System", "CUSTOMER_CODE", "000");
        c.spilForQle  = g.Int("System", "SPIL_FOR_QLE", 0);
        c.machineType = g.Str("Version", "Model", "HT-9046");                                     // cprod.cpp:2968
        c.machineId   = FileExistsA_(generalIni) ? g.Str("Version", "Machine ID", "") : std::string();
        std::string id = g.Str("Version", "Machine ID", "29828");                                 // cprod.cpp:2971
        if (id.empty()) id = " ";                                                                 // SetSocketHandlerID
        c.socketHandlerId = id;
    }
    c.hostName = hostName.empty() ? ThisHostName() : hostName;
    c.pcName = c.hostName;                                                                        // main.cpp:11091
    if (CustInt(c.custCode) == 933)                                                               // CC_ASE_CL, cprod.cpp:2974-2980
    {
        char b[256];
        DWORD n = sizeof(b);
        if (::GetComputerNameA(b, &n)) c.socketHandlerId = std::string(b, n);
    }
    // bSPILFunction lives in the customer table (CosFunction), not in an ini.  N17 is only editable for SPIL or
    // CC_QUALCOMM 999 (cConfiguration.cpp:3576-3590, else the value is fixed 0), so an N17 that is on and not 999 is SPIL.
    c.spil = c.n17 && CustInt(c.custCode) != 999;
    *out = c;
    return ok;
}

// ===========================================================================
//  pure parts
// ===========================================================================
// Fnv1a32 is R4's (ElaFtp.cpp): one definition in ht9045_ela.  AI(W906-ELA-R5) 20260927: both were
// `ela::Fnv1a32(const std::string&)` -- the return type is not in the mangled name, so two copies clash at link.

std::string StaggerKey(const std::string& machineId, const std::string& hostName)
{
    const std::string id = Trim(machineId);
    return (id.empty() || id == "HT-90xx") ? hostName : id;
}

int StaggerOffsetSec(const std::string& key, int windowSec)
{
    return windowSec > 0 ? (int)((unsigned long)Fnv1a32(key) % (unsigned long)windowSec) : 0;
}

int BackoffBaseSec(int n)
{
    if (n < 1) n = 1;
    if (n > 6) return 1800;
    const int b = 60 << (n - 1);
    return b < 1800 ? b : 1800;
}

// MurmurHash3 fmix32: FNV-1a's last byte barely moves the high bits, so without it u is almost the same for every n
static unsigned long Fmix32(unsigned long h)
{
    h ^= h >> 16;
    h = (h * 0x85EBCA6BUL) & 0xFFFFFFFFUL;
    h ^= h >> 13;
    h = (h * 0xC2B2AE35UL) & 0xFFFFFFFFUL;
    h ^= h >> 16;
    return h;
}

int BackoffDelaySec(const std::string& key, int n)
{
    const double u = (double)Fmix32((unsigned long)Fnv1a32(key + "#" + IntStr(n))) / 4294967295.0;
    const double d = BackoffBaseSec(n) * (0.8 + 0.4 * u);
    return (int)(d + 0.5);
}

static int SysDriveType(const std::string& root) { return (int)::GetDriveTypeA(root.c_str()); }

bool IsRemotePath(const std::string& path, DriveTypeFn driveType)
{
    const std::string p = Trim(path);
    if (p.size() >= 2 && (p[0] == '\\' || p[0] == '/') && (p[1] == '\\' || p[1] == '/')) return true;   // UNC
    if (p.size() >= 2 && p[1] == ':' && ((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')))
        return (driveType ? driveType : &SysDriveType)(p.substr(0, 2) + "\\") == DRIVE_REMOTE;
    return false;
}

// AI(W906-W58) 20260930 (St02-E): W58 Q5 (ElaSchedule.h)
bool SimNetPathsAllowed()
{
    const char* e = std::getenv("W906_SIM_NET_PATHS");
    return e && e[0] == '1' && e[1] == '\0';
}

bool SimBlocksNetPath(const std::string& path, bool simBuild, bool allowNet, DriveTypeFn driveType)
{
    return simBuild && !allowNet && IsRemotePath(path, driveType);
}

bool SimBlocksNetPathNow(const std::string& path)
{
#ifdef W906_NO_SOFT_SIMULTE
    (void)path;
    return false;
#else
    return SimBlocksNetPath(path, true, SimNetPathsAllowed(), 0);
#endif
}

// the scheduler's side of Q5: a KIND_NETDRIVE job is off in SIM, timed and manual -- it does not run, so it is not retried
static bool SimNetKindBlocked(const ScheduleSetup& su, JobKind kind, std::string* why)
{
    if (!su.simBuild || su.simNetPaths || kind != KIND_NETDRIVE) return false;
    if (why) *why = "SIM build: the save path is a network share (W58 Q5) -- not written; W906_SIM_NET_PATHS=1 writes it";
    return true;
}

int O06PeriodMin(const ScheduleConfig& c, bool keepGoldenBugs)
{
    if (c.o06TimePeriod) return c.o06TimePeriodIdx == 0 ? 10 : 30;   // Analyzer.cpp:3028-3034
    // Analyzer.cpp:3014 `int iCheckInterval=1; //預設一小時一次` -- the comment says once an hour, the code saves every
    // minute (ledger G3); W18 B (Steven 20260927) had made that once an hour.
    // AI(W906-ELA-W43) 20260928 (St02-E helper), ★W43 = C (Steven 20260928 「W43 就按照你的建議做吧」): the analyzer
    //   follows the Handler.  [O06-8] "Update Production Record every 10 / 30 minutes" (906_0625_Steven
    //   cConfiguration.dfm:13713-13737 chkO06TimePeriod + cbO06TimePeriod, registered at cConfiguration.cpp:3950 / :3960
    //   as [Event Log] EnanleTimePeriodSaveLog / TimePeriodSaveLog, IniConfig.bO06SaveLogTimePeriod /
    //   iO06SaveLogTimePeriod, Config.h:1347-1348) is the Handler's own switch for its periodic production record:
    //   unchecked, the Handler keeps none (cpublic.cpp:598 ProductionLog returns; main.cpp:31052-31055 TimerESD counts
    //   only while it is checked).  So unchecked = no timed SummaryData save, 0 = never.  keepGoldenBugs = golden, 1.
    return keepGoldenBugs ? 1 : 0;
}

JobKind ResolveKind(int job, const ScheduleConfig& c, DriveTypeFn driveType)
{
    switch (job)
    {
    case JOB_N25_3:
    case JOB_N25_4:   // W22: the same N25-2 account and host
    case JOB_N25_5: return KIND_FTP;
    case JOB_O06_4:   // [O06-6] Use Net Drive (St02-E 20260927) or a remote AutoSaveProductionPath
        return (c.o06UseNetDrive || IsRemotePath(c.o06Path, driveType)) ? KIND_NETDRIVE : KIND_LOCAL;
    case JOB_N10_3:   // N10SaveSummaryData :3157-3179: method 1 = the drive path, else D:\HT9045_Log\EventLogSummary;
        // rgN10_4 has two items (Analyzer.dfm:2732-2734): 2 and up clamp to 1 = Net Drive, as R2's RadioIndex
        return ((c.n10UploadMethod >= 1) && IsRemotePath(c.n10DrivePath, driveType)) ? KIND_NETDRIVE : KIND_LOCAL;
    case JOB_O19_VTEST: return IsRemotePath(c.o19Path, driveType) ? KIND_NETDRIVE : KIND_LOCAL;
    case JOB_N17_PRODLOG: return IsRemotePath(c.n17Path, driveType) ? KIND_NETDRIVE : KIND_LOCAL;
    }
    return KIND_LOCAL;
}

int StaggerWindowSec(int job, JobKind kind, const ScheduleConfig& c, bool keepGoldenBugs)
{
    if (kind == KIND_FTP) return 900;
    if (kind == KIND_LOCAL) return 0;
    if (job == JOB_O06_4)
    {
        // 300 s like any network drive (St02-E 20260927), but never more than half the O06 period: a larger offset
        // would put every run after the next slot, which supersedes it (golden every-minute mode: 30 s)
        const int half = O06PeriodMin(c, keepGoldenBugs) * 60 / 2;
        return half < 300 ? half : 300;
    }
    return 300;
}

bool JobEnabled(int job, const ScheduleConfig& c, bool simBuild, bool automatic, std::string* why, bool keepGoldenBugs)
{
    std::string w;
    const int cust = CustInt(c.custCode);
    // D-a: golden's analyzer (and with it every job it runs) exists only while O10 is on (906_0625_Steven
    // main.cpp:17848 / :17974).  N17 is the Handler's own job (HS_Function.cpp:204-207): not gated by O10.
    // EffectiveO10 is evaluated here, i.e. at every Tick's decision (the getter is never cached).
    const bool needO10 = automatic && job != JOB_N17_PRODLOG;
    switch (job)
    {
    case JOB_O06_4:
        if (needO10 && !EffectiveO10(c.o06_1, c.o10Key)) w = "O10 off (" + EffectiveO10Source() + ")";
        else if (!c.o06Production) w = "O06-4 off ([Event Log] EnableAutoSaveProductiont)";
        else if (automatic && O06PeriodMin(c, keepGoldenBugs) <= 0)   // AI(W906-ELA-W43) 20260928: ★W43 = C
            w = "O06-8 off ([Event Log] EnanleTimePeriodSaveLog): no timed SummaryData save (W43 = C, as the Handler)";
        break;
    case JOB_N10_3:
        if (needO10 && !EffectiveO10(c.o06_1, c.o10Key)) w = "O10 off (" + EffectiveO10Source() + ")";
        else if (!c.n10Daily) w = "N10-3 off ([FTPUpLoad] bN10_DailyUploadProdData)";
        else if (N10EffectiveMethod(c) < 0)
            w = "N10-3 iN10UploadProductMethod=" + IntStr(c.n10Method) + " selects no item of rgN10_3_1 (Analyzer.dfm:2765-2768)";
        break;
    case JOB_N25_3:
        if (cust != 851) w = "CUSTOMER_CODE " + Trim(c.custCode) + " is not 851 (CC_ChipMos_ZHUBEI)";
        else if (needO10 && !EffectiveO10(c.o06_1, c.o10Key)) w = "O10 off (" + EffectiveO10Source() + ")";
        else if (!c.n25_3) w = "N25-3 off ([ChipMos Function] bN25_3_EnableULJamLog)";
        else if (Trim(c.n25Host).empty()) w = "N25-2 host empty ([ChipMos Function] sN25_2_FTPHost)";
        else if (automatic && simBuild)
            w = "SIM build: golden sends EL_UPLOAD_JAMWEEK only without SOFT_SIMULTE (main.cpp:21425)";
        break;
    case JOB_N25_4:   // AI(W906-ELA-W22) 20260928 (St02-E): the N25-3 gates with the job's own switch
    case JOB_N25_5:
    {
        const bool four = job == JOB_N25_4;
        if (cust != 851) w = "CUSTOMER_CODE " + Trim(c.custCode) + " is not 851 (CC_ChipMos_ZHUBEI)";
        else if (needO10 && !EffectiveO10(c.o06_1, c.o10Key)) w = "O10 off (" + EffectiveO10Source() + ")";
        else if (four && !c.n25_4) w = "N25-4 off ([ChipMos Function] bN25_4_EnableUpload)";
        else if (!four && !c.n25_5) w = "N25-5 off ([ChipMos Function] bN25_5_EnableUpload)";
        else if (Trim(c.n25Host).empty()) w = "N25-2 host empty ([ChipMos Function] sN25_2_FTPHost)";
        else if (automatic && simBuild)
            w = four ? "SIM build: golden sends EL_UPLOAD_SUMMARY only without SOFT_SIMULTE (main.cpp:21425 / :21451)"
                     : "SIM build: golden sends EL_UPLOAD_EVENTLOG only without SOFT_SIMULTE (main.cpp:21425 / :21456)";
        break;
    }
    case JOB_O19_VTEST:
        if (cust != 915 && cust != 919) w = "CUSTOMER_CODE " + Trim(c.custCode) + " is not 915 / 919 (bVTESTFunction)";
        else if (needO10 && !EffectiveO10(c.o06_1, c.o10Key)) w = "O10 off (" + EffectiveO10Source() + ")";
        else if (!c.o19Week) w = "O19 off ([Event Log] bO19_AutoRecordReportByEveryWeek)";
        else if (c.o19WeekDay < 0 || c.o19WeekDay > 6)
            w = "O19 iO19_WeekPeriod=" + IntStr(c.o19WeekDay) + " is not a weekday 0..6 (HS_Function.cpp:265)";
        break;
    case JOB_N17_PRODLOG:
        if (!c.n17) w = "N17 off ([Production_Log] bN17UploadProdLog)";
        break;
    default:
        w = "unknown job";
    }
    if (why) *why = w;
    return w.empty();
}

bool CustomerCodeAllows(int elCommand, const std::string& custCode)
{
    const int cust = CustInt(custCode);
    switch (elCommand)
    {
    case EL_UPLOAD_JAMWEEK:
    case EL_UPLOAD_SUMMARY:
    case EL_UPLOAD_EVENTLOG: return cust == 851;             // N25-3/4/5 exist only for 851 (cConfiguration.cpp:3729)
    case EL_UPLOAD_CHIPADV_LOTEND: return cust == 868;       // N34 CC_CYUEAN (plan §1.1)
    case EL_VTEST_MTBF_SUM: return cust == 915 || cust == 919;
    }
    return true;
}

static LocalSec RulePeriod(const SlotRule& r)
{
    return r.type == SlotRule::EVERY_N_MIN ? (LocalSec)r.periodMin * 60 : r.type == SlotRule::DAILY_AT ? 86400 : 604800;
}

static bool RuleLatest(const SlotRule& r, LocalSec now, LocalSec* s)
{
    const long long day = FloorDiv(now, 86400);
    if (r.type == SlotRule::EVERY_N_MIN)
    {
        const LocalSec p = (LocalSec)r.periodMin * 60;
        if (p <= 0) return false;
        *s = FloorDiv(now, p) * p;                          // 86400 % p == 0: aligned to midnight
        return true;
    }
    if (r.type == SlotRule::DAILY_AT)
    {
        LocalSec c = day * 86400 + r.minuteOfDay * 60LL;
        if (c > now) c -= 86400;
        *s = c;
        return true;
    }
    if (r.weekday < 0 || r.weekday > 6) return false;
    const int back = (DayOfWeek0(day * 86400) - r.weekday + 7) % 7;
    LocalSec c = (day - back) * 86400 + r.minuteOfDay * 60LL;
    if (c > now) c -= 604800;
    *s = c;
    return true;
}

bool Calendar::Latest(LocalSec now, LocalSec* slot) const
{
    bool any = false;
    LocalSec best = 0;
    for (size_t i = 0; i < rules.size(); ++i)
    {
        LocalSec s;
        if (RuleLatest(rules[i], now, &s) && (!any || s > best)) { best = s; any = true; }
    }
    if (any) *slot = best;
    return any;
}

bool Calendar::Next(LocalSec slot, LocalSec* next) const
{
    bool any = false;
    LocalSec best = 0;
    for (size_t i = 0; i < rules.size(); ++i)
    {
        LocalSec s;
        if (!RuleLatest(rules[i], slot, &s)) continue;
        s += RulePeriod(rules[i]);                          // the first slot of this rule after `slot`
        if (!any || s < best) { best = s; any = true; }
    }
    if (any) *next = best;
    return any;
}

static SlotRule Every(int minutes) { SlotRule r = { SlotRule::EVERY_N_MIN, minutes, 0, 0 }; return r; }
static SlotRule Daily(int minuteOfDay) { SlotRule r = { SlotRule::DAILY_AT, 0, minuteOfDay, 0 }; return r; }
static SlotRule Weekly(int weekday, int minuteOfDay) { SlotRule r = { SlotRule::WEEKLY_AT, 0, minuteOfDay, weekday }; return r; }

static void AddO06Rules(const ScheduleConfig& c, bool golden, Calendar* cal)
{
    // Timer2 :3026-3040; AI(W906-ELA-W43) 20260928, ★W43 = C: without [O06-8] there is no slot (O06PeriodMin 0)
    const int period = O06PeriodMin(c, golden);
    if (c.o06Production && period > 0) cal->rules.push_back(Every(period));
}

// N10-3 method 3 (指定時間, the Handler's CYUEAN option): the analyzer's rgN10_3_1 has three items (Analyzer.dfm:2765-2768),
// and VCL TCustomRadioGroup::SetItemIndex clamps ItemIndex = 3 to Count-1 = 2, so golden's analyzer saves "Per Hour".
// (★W44 B: the calendar uses the specified time instead -- N10SpecifiedTimeRule; this clamp still gates JobEnabled.)
int N10EffectiveMethod(const ScheduleConfig& c)
{
    if (c.n10Method < -1) return -1;
    return c.n10Method > 2 ? 2 : c.n10Method;
}

// ★W44 B (Steven 20260928; AI(W906-ELA-W44) 20260928, St02-E helper): N10-3 method 3 at the Handler's specified time
static bool N10SpecifiedTimeRule(const ScheduleConfig& c)
{
    return c.n10Daily && c.n10Method == 3 && c.n10SpecifiedMinute >= 0 && c.n10SpecifiedMinute < 1440;
}

static void AddN10Rules(const ScheduleConfig& c, Calendar* cal)
{
    if (!c.n10Daily) return;                                                    // Timer2 :3042-3068
    // ---- ★W44 B (Steven 20260928; R5's ADD-ON POINT): method 3 = the Handler's "specified time" ----------------
    // The Handler's own method 3 fires once at IniConfig.dN10_3_1_SpecifiedTime (906_0625_Steven HS_Function.cpp:488-509,
    // Config.h:1030).  ReadScheduleConfig sets n10SpecifiedMinute (00:00 when the key is missing = golden's never-loaded
    // field); a ScheduleConfig built by hand keeps -1 = the analyzer's clamp below (every hour).  The D-b boot catch-up
    // (★W44-2, AI(W906-ELA-W44B) 20260928) and a re-base when the time changes: Scheduler::Tick / ReadConfigNow.
    if (N10SpecifiedTimeRule(c))
    {
        cal->rules.push_back(Daily(c.n10SpecifiedMinute));
        return;
    }
    // ---- end of ★W44 -------------------------------------------------------------------------------------------
    const int m = N10EffectiveMethod(c);
    if (m == 0) cal->rules.push_back(Daily(0));
    else if (m == 1) { cal->rules.push_back(Daily(8 * 60)); cal->rules.push_back(Daily(20 * 60)); }
    else if (m == 2) cal->rules.push_back(Every(60));
}

Calendar JobCalendar(int job, const ScheduleConfig& c, bool keepGoldenBugs)
{
    Calendar cal;
    switch (job)
    {
    case JOB_O06_4:
        AddO06Rules(c, keepGoldenBugs, &cal);
        if (keepGoldenBugs) AddN10Rules(c, &cal);   // golden :3071-3083: an N10 slot also runs O06SaveSummaryData
        break;
    case JOB_N10_3:
        AddN10Rules(c, &cal);
        if (keepGoldenBugs) AddO06Rules(c, true, &cal);   // ... and every O06 slot runs N10SaveSummaryData
        break;
    case JOB_N25_3: cal.rules.push_back(Daily(0)); break;                        // main.cpp:21442 00:00:01..04
    case JOB_N25_4:                                                              // W22: the same trigger,
    case JOB_N25_5: cal.rules.push_back(Daily(0)); break;                        //   main.cpp:21449-21457
    case JOB_O19_VTEST: cal.rules.push_back(Weekly(c.o19WeekDay, 0)); break;     // HS_Function.cpp:257-272
    case JOB_N17_PRODLOG: cal.rules.push_back(Daily(60)); break;                 // HS_Function.cpp:204-207 01:00
    }
    return cal;
}

// ===========================================================================
//  N17 UploadProdLog (W13): golden Command.cpp:12401-12447 (V906 Command.cpp:3608-3654)
// ===========================================================================
std::string N17FileName(const ScheduleConfig& c, int y, int m, int d)
{
    char ymd[16];
    std::snprintf(ymd, sizeof(ymd), "%04d%02d%02d", y, m, d);
    if (c.spil)                                                            // :12408
    {
        if (CustInt(c.custCode) == 912 && c.spilForQle == 1)              // CC_SPIL_CHINA_SUZHOU, :12410-12413
            return c.machineType + "_" + c.socketHandlerId + "_" + ymd + "_ProductionLog.csv";
        return c.pcName + "_" + ymd + ".csv";                              // :12416
    }
    return c.machineType + "_" + c.socketHandlerId + "_" + ymd + "_ProductionLog.csv";   // :12421
}

CopyVerify CopyFileAndVerify(const std::string& src, const std::string& dstDir, const std::string& dst, bool mayOverwrite,
                             std::string* detail)
{
    std::string dummy;
    std::string& out = detail ? *detail : dummy;
    if (!DirExistsA_(dstDir))                                             // :12426 (checked before the source)
    {
        out = "N-17 path missing: " + dstDir;
        return CV_DEST_DIR_MISSING;
    }
    unsigned long long srcSize = 0, dstSize = 0;
    if (!FileExistsA_(src) || !FileSizeA_(src, &srcSize))               // :12428 (golden: silently nothing)
    {
        out = "source missing: " + src;
        return CV_SOURCE_MISSING;
    }
    bool copied = ::CopyFileA(src.c_str(), dst.c_str(), TRUE) != 0;      // :12431 bFailIfExists=TRUE
    if (!copied)
    {
        DWORD e = ::GetLastError();
        if ((e == ERROR_FILE_EXISTS || e == ERROR_ALREADY_EXISTS) && FileSizeA_(dst, &dstSize))
        {
            if (dstSize == srcSize)
            {
                out = "already there with the same size (" + SizeStr(srcSize) + " bytes): " + dst;
                return CV_ALREADY_SAME;
            }
            if (!mayOverwrite)
            {
                out = "target exists with another size (" + SizeStr(dstSize) + " vs " + SizeStr(srcSize) +
                      " bytes), not overwritten: " + dst;
                return CV_EXISTS_DIFFERENT;
            }
            copied = ::CopyFileA(src.c_str(), dst.c_str(), FALSE) != 0;   // our own partial copy from an earlier attempt
            if (!copied) e = ::GetLastError();
        }
        if (!copied)
        {
            out = "CopyFile failed (error " + IntStr((long long)e) + "): " + dst;
            return CV_COPY_FAILED;
        }
    }
    if (!FileSizeA_(dst, &dstSize) || dstSize != srcSize)
    {
        out = "copied but the size differs (" + SizeStr(dstSize) + " vs " + SizeStr(srcSize) + " bytes): " + dst;
        return CV_SIZE_MISMATCH;
    }
    out = "copied " + SizeStr(srcSize) + " bytes, same size: " + dst;
    return CV_COPIED;
}

static const unsigned kN17Wrote = 1u;   // JobContext::slotFlags: this slot's attempts wrote the target themselves

JobStatus RunN17UploadProdLog(JobContext& ctx)
{
    const ScheduleConfig& c = *ctx.cfg;
    if (!c.n17) { ctx.detail = "bN17UploadProdLog off"; return JOB_NOTHING_TO_DO; }        // :12424
    int y, m, d, hh, nn, ss;
    CivilFromLocalSec(ctx.slot - 86400, &y, &m, &d, &hh, &nn, &ss);   // GetYesterdayInfo (Now()-1) of the slot
    char ym[16];
    std::snprintf(ym, sizeof(ym), "%04d%02d", y, m);
    const std::string name = N17FileName(c, y, m, d);
    const std::string src = c.prodLogRoot + "\\" + ym + "\\" + name;  // :12407 "%s\\%04d%02d\\" + :12423
    const std::string dst = c.n17Path + "\\" + name;                  // :12430 "%s\\%s"
    std::string det;
    const CopyVerify r = CopyFileAndVerify(src, c.n17Path, dst, (ctx.slotFlags & kN17Wrote) != 0, &det);
    ctx.detail = det;
    switch (r)
    {
    case CV_COPIED: ctx.slotFlags |= kN17Wrote; return JOB_VERIFIED;
    case CV_ALREADY_SAME: return JOB_VERIFIED;
    case CV_SOURCE_MISSING: return JOB_NOTHING_TO_DO;
    case CV_EXISTS_DIFFERENT: return JOB_FAILED;           // golden: CopyFile fails -> ShowMyMessage :12434
    case CV_SIZE_MISMATCH: ctx.slotFlags |= kN17Wrote; return JOB_RETRY;
    case CV_DEST_DIR_MISSING:                              // golden: ShowMyMessage :12444 once; here retried (a share)
    case CV_COPY_FAILED: return JOB_RETRY;
    }
    return JOB_RETRY;
}

// ===========================================================================
//  FTP_Log (golden TMyStringList "FTP_Log", Rev891 Analyzer.cpp:220-224; MyStringList.cpp:158-173 / :526 / :642)
// ===========================================================================
static void AppendFtpLog(const std::string& root, LocalSec now, const std::string& msg)
{
    if (root.empty()) return;
    int y, m, d, hh, nn, ss;
    CivilFromLocalSec(now, &y, &m, &d, &hh, &nn, &ss);
    char dir[32], file[40], stamp[48];
    std::snprintf(dir, sizeof(dir), "\\UploadFile\\%04d\\%02d", y, m);
    std::snprintf(file, sizeof(file), "\\FTP_Log_%04d%02d%02d.csv", y, m, d);
    std::snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d, %02d:%02d:%02d.000, ", y, m, d, hh, nn, ss);
    const std::string folder = root + dir;
    ForceDirs(folder);
    const std::string path = folder + file;
    const bool isNew = !FileExistsA_(path);
    FILE* f = std::fopen(path.c_str(), "ab");
    if (!f) return;
    if (isNew) std::fputs("Date, Time, Action, S2, S3, S4, S5, S6\r\n", f);   // the TMyStringList FirstRow
    const std::string line = std::string(stamp) + "Schedule, " + msg + "\r\n";
    std::fwrite(line.data(), 1, line.size(), f);
    std::fclose(f);
}

static std::string Msg(int job, const std::string& event, LocalSec slot, const std::string& detail)
{
    return std::string(JobName(job)) + ", " + Col(event) + ", " + FormatLocalSec(slot) + ", " + Col(detail);
}

// ===========================================================================
//  Scheduler
// ===========================================================================
ScheduleSetup::ScheduleSetup()
    : clock(0), configIni(ElaConfigIniPath()),   // AI(W906-ELA-REV) 20260928: W906_AUTH_PATH seam (ElaHub.h)
      generalIni(EnvOr("W906_GENERAL_INI_PATH", "D:\\HT9045\\system\\Gerneral.ini")),
      logRoot(EnvOr("W906_HT9045LOG_ROOT", "D:\\HT9045_Log")), configEverySec(60), keepGoldenBugs(false),
#ifdef W906_NO_SOFT_SIMULTE
      simBuild(false),
#else
      simBuild(true),
#endif
      simNetPaths(SimNetPathsAllowed()), driveType(0), logSink(0)
{
}

Scheduler::JobState::JobState()
    : enabled(false), initialized(false), pending(false), pendingCatchUp(false), kind(KIND_LOCAL), windowSec(0), offsetSec(0),
      attempt(0), runs(0), cursor(-1), done(-1), armed(-1), pendingSlot(-1), pendingDue(-1), slotFlags(0), lastAt(-1),
      lastManual(false), noBodyLogged(false)
{
}

Scheduler::Scheduler(const ScheduleSetup& s)
    : setup_(s), clock_(s.clock ? s.clock : &sysClock_), haveConfig_(false), booted_(false), bootAt_(-1), configAt_(-1),
      reload_(false)
{
    if (setup_.hostName.empty()) setup_.hostName = ThisHostName();
    stateFile_ = !setup_.stateFile.empty() ? setup_.stateFile
                 : setup_.logRoot.empty()  ? std::string()
                                           : setup_.logRoot + "\\UploadFile\\ElaScheduleState.ini";
    for (int i = 0; i < JOB_TOTAL; ++i) manual_[i] = false;
    fn_[JOB_N17_PRODLOG] = &RunN17UploadProdLog;   // W13: the one body that lives here; R2 / R4 add the other four
}

Scheduler::~Scheduler() {}

void Scheduler::SetJob(int job, const JobFn& fn)
{
    if (job >= 0 && job < JOB_TOTAL) fn_[job] = fn;
}

bool Scheduler::RunNow(int job)
{
    if (job < 0 || job >= JOB_TOTAL) return false;
    webbridge::WbGuard g(mu_);
    manual_[job] = true;
    return true;
}

void Scheduler::ReloadConfig()
{
    webbridge::WbGuard g(mu_);
    reload_ = true;
}

void Scheduler::Log(const std::string& msg)
{
    const LocalSec now = clock_->Now();
    {
        webbridge::WbGuard g(mu_);
        history_.push_back(FormatLocalSec(now) + " " + msg);
        if (history_.size() > 200) history_.erase(history_.begin());
    }
    webbridge::WbGuard g(logMu_);
    if (setup_.logSink) setup_.logSink(now, msg);
    else AppendFtpLog(setup_.logRoot, now, msg);
}

std::vector<std::string> Scheduler::History() const
{
    webbridge::WbGuard g(mu_);
    return history_;
}

void Scheduler::Event(int job, LocalSec now, const std::string& event, LocalSec slot, const std::string& detail)
{
    // one line into FTP_Log + the global history, and into the job's own record (R6 shows it; not the page snapshot)
    JobState& s = st_[job];
    s.results.push_back(FormatLocalSec(now) + " " + event + (slot >= 0 ? " [" + FormatLocalSec(slot) + "]" : std::string()) +
                        (detail.empty() ? std::string() : " -- " + detail));
    if (s.results.size() > 20) s.results.erase(s.results.begin());
    Log(Msg(job, event, slot, detail));
}

Scheduler::JobState Scheduler::State(int job) const
{
    return (job >= 0 && job < JOB_TOTAL) ? st_[job] : JobState();
}

void Scheduler::ReadConfigNow(LocalSec now)
{
    ScheduleConfig c;
    const bool ok = setup_.readConfig ? setup_.readConfig(&c)
                                      : ReadScheduleConfig(setup_.configIni, setup_.generalIni, setup_.hostName, &c);
    configAt_ = now;
    if (!ok && haveConfig_) return;                         // keep what we had
    if (c.hostName.empty()) c.hostName = setup_.hostName;
    const bool first = !haveConfig_;
    // ★W44 B (AI(W906-ELA-W44) 20260928): the N10-3 time (or method) changed while the specified-time rule is or was in
    //   use -> N10-3 is re-based at this Tick (JobState::initialized = false).  Golden compares the clock with the
    //   CURRENT time only (HS_Function.cpp:494-497): a new time that is already past today is not run, it runs next.
    if (!first && (N10SpecifiedTimeRule(c) || N10SpecifiedTimeRule(cfg_)) &&
        (c.n10Method != cfg_.n10Method || c.n10SpecifiedMinute != cfg_.n10SpecifiedMinute))
        st_[JOB_N10_3].initialized = false;
    cfg_ = c;
    haveConfig_ = true;
    if (first && !ok) Log("config, not readable, -, " + Col(setup_.configIni) + ": every switch at its golden default");
    const std::string key = StaggerKey(cfg_.machineId, cfg_.hostName);
    for (int j = 0; j < JOB_TOTAL; ++j)
    {
        JobState& s = st_[j];
        s.kind = ResolveKind(j, cfg_, setup_.driveType);
        s.windowSec = StaggerWindowSec(j, s.kind, cfg_, setup_.keepGoldenBugs);
        s.offsetSec = StaggerOffsetSec(key, s.windowSec);
    }
}

void Scheduler::LoadState()
{
    if (stateFile_.empty()) return;
    FILE* f = std::fopen(stateFile_.c_str(), "rb");
    if (!f) return;
    int job = -1;
    char buf[512];
    while (std::fgets(buf, sizeof(buf), f))
    {
        std::string l = buf;
        while (!l.empty() && (l[l.size() - 1] == '\r' || l[l.size() - 1] == '\n')) l.erase(l.size() - 1);
        l = Trim(l);
        if (l.empty() || l[0] == ';') continue;
        if (l[0] == '[' && l[l.size() - 1] == ']')
        {
            job = -1;
            const std::string n = l.substr(1, l.size() - 2);
            for (int j = 0; j < JOB_TOTAL; ++j)
                if (n == JobName(j)) job = j;
            continue;
        }
        const size_t eq = l.find('=');
        if (job < 0 || eq == std::string::npos) continue;
        const std::string k = Trim(l.substr(0, eq));
        LocalSec t;
        if (!ParseLocalSec(Trim(l.substr(eq + 1)), &t)) continue;
        if (k == "done") st_[job].done = t;
        else if (k == "armed") st_[job].armed = t;
    }
    std::fclose(f);
}

void Scheduler::SaveState()
{
    if (stateFile_.empty()) return;
    std::string o = "; ElaSchedule state -- AI(W906-ELA-R5): done = last verified slot, armed = first V906 run with the "
                    "job on (local time)\r\n";
    for (int j = 0; j < JOB_TOTAL; ++j)
    {
        if (st_[j].done < 0 && st_[j].armed < 0) continue;
        o += std::string("[") + JobName(j) + "]\r\n";
        if (st_[j].done >= 0) o += "done=" + FormatLocalSec(st_[j].done) + "\r\n";
        if (st_[j].armed >= 0) o += "armed=" + FormatLocalSec(st_[j].armed) + "\r\n";
    }
    const size_t p = stateFile_.find_last_of("\\/");
    if (p != std::string::npos) ForceDirs(stateFile_.substr(0, p));
    const std::string tmp = stateFile_ + ".tmp";
    FILE* f = std::fopen(tmp.c_str(), "wb");
    if (!f) return;
    std::fwrite(o.data(), 1, o.size(), f);
    std::fclose(f);
    if (!::MoveFileExA(tmp.c_str(), stateFile_.c_str(), MOVEFILE_REPLACE_EXISTING)) ::DeleteFileA(tmp.c_str());
}

void Scheduler::RunJob(int job, LocalSec now, bool manual)
{
    JobState& s = st_[job];
    JobContext ctx;
    ctx.job = job;
    ctx.cfg = &cfg_;
    ctx.manual = manual;
    if (manual)
    {
        ctx.slot = now;
        ctx.attempt = 1;
        ctx.catchUp = false;
        ctx.slotFlags = 0;
    }
    else
    {
        ++s.attempt;
        ctx.slot = s.pendingSlot;
        ctx.attempt = s.attempt;
        ctx.catchUp = s.pendingCatchUp;
        ctx.slotFlags = s.slotFlags;
    }
    ctx.slotDateTime = LocalSecToDateTime(ctx.slot);
    JobStatus r = JOB_NO_BODY;
    if (fn_[job])
    {
        try { r = fn_[job](ctx); }
        catch (...) { r = JOB_RETRY; ctx.detail = "exception in the job body"; }
        ++s.runs;
    }
    const std::string tag = std::string(manual ? "manual" : "attempt " + IntStr(ctx.attempt)) +
                            (ctx.catchUp ? " (boot catch-up)" : "");
    s.lastResult = FormatLocalSec(now) + " " + tag + ": " + StatusName(r) + (ctx.detail.empty() ? "" : " -- " + ctx.detail);
    s.lastAt = now;                                  // AI(W906-ELA-R6) 20260928: the page's "last run" columns
    s.lastStatus = StatusName(r);
    s.lastDetail = ctx.detail;
    s.lastManual = manual;
    if (r == JOB_NO_BODY)
    {
        if (!s.noBodyLogged)
            Event(job, now, tag + ": " + StatusName(r), ctx.slot, "R2 / R4 not merged yet; the slot is not recorded as done");
        s.noBodyLogged = true;
        if (!manual) s.pending = false;
        return;
    }
    Event(job, now, tag + ": " + StatusName(r), ctx.slot, ctx.detail);
    if (manual) return;
    s.slotFlags = ctx.slotFlags;
    if (r == JOB_VERIFIED || r == JOB_NOTHING_TO_DO)
    {
        s.pending = false;
        if (s.pendingSlot > s.done)
        {
            s.done = s.pendingSlot;
            SaveState();
        }
        return;
    }
    if (r == JOB_FAILED)
    {
        s.pending = false;
        return;
    }
    // JOB_RETRY: back-off (plan §3 item 5)
    if (s.attempt > kMaxRetries)
    {
        s.pending = false;
        Event(job, now, "given up", s.pendingSlot, "after " + IntStr(s.attempt) + " attempts");
        return;
    }
    const int delay = BackoffDelaySec(StaggerKey(cfg_.machineId, cfg_.hostName), s.attempt);
    const LocalSec next = now + delay;
    LocalSec ns;
    if (JobCalendar(job, cfg_, setup_.keepGoldenBugs).Next(s.pendingSlot, &ns) && next >= ns)
    {
        s.pending = false;
        Event(job, now, "given up", s.pendingSlot, "the next slot " + FormatLocalSec(ns) + " comes before the retry");
        return;
    }
    s.pendingDue = next;
    Event(job, now, "retry " + IntStr(s.attempt) + " in " + IntStr(delay) + " s", s.pendingSlot, "at " + FormatLocalSec(next));
}

void Scheduler::Tick()
{
    const LocalSec now = clock_->Now();
    const bool bootTick = !booted_;
    if (bootTick)
    {
        booted_ = true;
        bootAt_ = now;
        LoadState();
    }
    bool reload = false, manual[JOB_TOTAL];
    {
        webbridge::WbGuard g(mu_);
        reload = reload_;
        reload_ = false;
        for (int j = 0; j < JOB_TOTAL; ++j) { manual[j] = manual_[j]; manual_[j] = false; }
    }
    if (bootTick || reload || !haveConfig_ || now < configAt_ || now - configAt_ >= setup_.configEverySec) ReadConfigNow(now);

    for (int j = 0; j < JOB_TOTAL; ++j)
    {
        JobState& s = st_[j];
        std::string why;
        bool en = JobEnabled(j, cfg_, setup_.simBuild, true, &why, setup_.keepGoldenBugs);
        if (en && SimNetKindBlocked(setup_, s.kind, &why)) en = false;   // AI(W906-W58) 20260930 (St02-E): W58 Q5
        if (bootTick || en != s.enabled || why != s.why)
            Event(j, now, en ? "on" : "off", -1,
                  en ? std::string(KindName(s.kind)) + "; stagger " + IntStr(s.offsetSec) + " s of " + IntStr(s.windowSec)
                     : why);
        s.enabled = en;
        s.why = why;

        if (manual[j])
        {
            std::string mwhy;
            if (JobEnabled(j, cfg_, setup_.simBuild, false, &mwhy, setup_.keepGoldenBugs) &&
                !SimNetKindBlocked(setup_, s.kind, &mwhy))                 // AI(W906-W58) 20260930 (St02-E): W58 Q5
                RunJob(j, now, true);
            else
            {
                Event(j, now, "manual refused", now, mwhy);
                s.lastAt = now;                      // AI(W906-ELA-R6): the page shows why the button did nothing
                s.lastStatus = "manual refused";
                s.lastDetail = mwhy;
                s.lastManual = true;
            }
        }
        if (!en)
        {
            if (s.pending) Event(j, now, "cancelled", s.pendingSlot, why);
            s.pending = false;
            s.initialized = false;
            continue;
        }

        const Calendar cal = JobCalendar(j, cfg_, setup_.keepGoldenBugs);
        LocalSec latest = -1;
        const bool has = cal.Latest(now, &latest);
        if (!s.initialized)
        {
            s.initialized = true;
            const LocalSec base = s.done > s.armed ? s.done : s.armed;
            if (s.done < 0 && s.armed < 0)
            {
                // no record: this is the first V906 run with the job on -- nothing is known to be missed
                s.armed = now;
                SaveState();
                Event(j, now, "armed", now, "first run with the job on: no boot catch-up");
                s.cursor = has ? latest : -1;
            }
            else if (j == JOB_N10_3 && N10SpecifiedTimeRule(cfg_))
            {
                // ★W44 B (Steven 20260928; AI(W906-ELA-W44) 20260928, St02-E helper): golden 906_0625_Steven
                //   HS_Function.cpp:488-509 runs while the clock is IN the specified minute, once (bN10_3_1_Flag).  Inside
                //   that minute golden's next timer tick fires, and so does this one (not a catch-up).
                // AI(W906-ELA-W44B) 20260928 (St02-E helper), ★W44-2 = catch up (Steven 20260928 「W44-2 要補發」; this
                //   replaces ★W44 B's "missed, not run", which was golden's -- golden has no catch-up): a specified time
                //   that passed while the Handler was off runs ONCE after the start, D-b: boot + offset, the most recent
                //   missed time only (never a backlog of days), done= keeps it to one run per time, a failure backs off.
                //   Only at boot: a time that passed while running (changed to an earlier time, or the job switched on
                //   late) is still not run, and a first run without a record only arms (both as D-b).
                s.cursor = has ? latest : -1;
                if (has && latest > base && now - latest < 60)
                {
                    if (s.pending)
                        Event(j, now, "given up", s.pendingSlot, "superseded by the specified time " + FormatLocalSec(latest));
                    s.pending = true;
                    s.pendingSlot = latest;
                    s.pendingDue = latest + s.offsetSec;
                    s.pendingCatchUp = false;
                    s.attempt = 0;
                    s.slotFlags = 0;
                }
                else if (bootTick && has && latest > base)
                {
                    s.pending = true;
                    s.pendingSlot = latest;
                    s.pendingDue = now + s.offsetSec;
                    s.pendingCatchUp = true;
                    s.attempt = 0;
                    s.slotFlags = 0;
                    Event(j, now, "boot catch-up", latest,
                          "catch-up for " + FormatLocalSec(latest).substr(0, 16) +
                              ": the specified time passed while the Handler was off (W44-2); last verified " +
                              FormatLocalSec(s.done) + "; runs at " + FormatLocalSec(s.pendingDue));
                }
            }
            else if (bootTick && has && latest > base)
            {
                // D-b: the most recent slot passed while the Handler was off (or its last run never verified)
                s.pending = true;
                s.pendingSlot = latest;
                s.pendingDue = now + s.offsetSec;
                s.pendingCatchUp = true;
                s.attempt = 0;
                s.slotFlags = 0;
                s.cursor = latest;
                Event(j, now, "boot catch-up", latest,
                      "last verified " + FormatLocalSec(s.done) + "; runs at " + FormatLocalSec(s.pendingDue));
            }
            else
                s.cursor = has ? latest : -1;   // switched on while running: slots that passed while off are not run
        }
        else if (has && latest > s.cursor)
        {
            if (s.pending)
                Event(j, now, "given up", s.pendingSlot, "superseded by the next slot " + FormatLocalSec(latest));
            s.pending = true;
            s.pendingSlot = latest;
            s.pendingDue = latest + s.offsetSec;   // stagger; the data range still comes from the slot
            s.pendingCatchUp = false;
            s.attempt = 0;
            s.slotFlags = 0;
            s.cursor = latest;
        }
        if (s.pending && now >= s.pendingDue) RunJob(j, now, false);
    }
    Publish(now);
}

void Scheduler::Publish(LocalSec now)
{
    std::string o = "[";
    for (int j = 0; j < JOB_TOTAL; ++j)
    {
        const JobState& s = st_[j];
        if (j) o += ',';
        // AI(W906-ELA-R6) 20260928: the next run -- the pending try, else the next slot after the last one taken + offset
        LocalSec next = -1;
        if (s.pending)
            next = s.pendingDue;
        else if (s.enabled)
        {
            LocalSec ns;
            if (JobCalendar(j, cfg_, setup_.keepGoldenBugs).Next(s.cursor >= 0 ? s.cursor : now, &ns)) next = ns + s.offsetSec;
        }
        o += "{\"id\":" + J(JobName(j)) + ",\"enabled\":" + (s.enabled ? "true" : "false") + ",\"why\":" + J(s.why) +
             ",\"kind\":" + J(KindName(s.kind)) + ",\"windowSec\":" + IntStr(s.windowSec) + ",\"offsetSec\":" +
             IntStr(s.offsetSec) + ",\"done\":" + J(FormatLocalSec(s.done)) + ",\"armed\":" + J(FormatLocalSec(s.armed)) +
             ",\"pending\":" + (s.pending ? "true" : "false") + ",\"slot\":" + J(FormatLocalSec(s.pending ? s.pendingSlot : -1)) +
             ",\"due\":" + J(FormatLocalSec(s.pending ? s.pendingDue : -1)) + ",\"attempt\":" + IntStr(s.attempt) +
             ",\"runs\":" + IntStr(s.runs) + ",\"hasBody\":" + (fn_[j] ? "true" : "false") + ",\"last\":" + J(s.lastResult) +
             ",\"next\":" + J(FormatLocalSec(next)) + ",\"retries\":" + IntStr(s.pending && s.attempt > 1 ? s.attempt - 1 : 0) +
             ",\"lastAt\":" + J(FormatLocalSec(s.lastAt)) + ",\"lastStatus\":" + J(s.lastStatus) +
             ",\"lastDetail\":" + J(s.lastDetail) + ",\"lastManual\":" + (s.lastManual ? "true" : "false") +
             ",\"results\":[";
        for (size_t k = 0; k < s.results.size(); ++k) o += (k ? "," : "") + J(s.results[k]);
        o += "]}";
    }
    o += "]";
    webbridge::WbGuard g(mu_);
    status_ = o;
    statusNow_ = FormatLocalSec(now);
}

std::string Scheduler::StatusJson() const
{
    webbridge::WbGuard g(mu_);
    std::string o = "{\"now\":" + J(statusNow_.empty() ? std::string("-") : statusNow_) + ",\"maxRetries\":" +
                    IntStr(kMaxRetries) + ",\"jobs\":" + (status_.empty() ? std::string("[]") : status_) + ",\"history\":[";
    for (size_t i = 0; i < history_.size(); ++i)
    {
        if (i) o += ',';
        o += J(history_[i]);
    }
    return o + "]}";
}

// ===========================================================================
//  the process-wide scheduler
// ===========================================================================
static webbridge::WbMutex g_schedMu;
static Scheduler* g_sched = 0;

static Scheduler* Current()
{
    webbridge::WbGuard g(g_schedMu);
    return g_sched;
}

void ScheduleInstall(const ScheduleSetup& s)
{
    Scheduler* n = new Scheduler(s);
    Scheduler* old = 0;
    {
        webbridge::WbGuard g(g_schedMu);
        old = g_sched;
        g_sched = n;
    }
    delete old;
}

void ScheduleUninstall()
{
    Scheduler* old = 0;
    {
        webbridge::WbGuard g(g_schedMu);
        old = g_sched;
        g_sched = 0;
    }
    delete old;
}

void ScheduleTick()
{
    if (Scheduler* s = Current()) s->Tick();
}

bool ScheduleSetJob(int job, const JobFn& fn)
{
    Scheduler* s = Current();
    if (!s || job < 0 || job >= JOB_TOTAL) return false;
    s->SetJob(job, fn);
    return true;
}

bool ScheduleRunNow(int job)
{
    Scheduler* s = Current();
    return s ? s->RunNow(job) : false;
}

void ScheduleReloadConfig()
{
    if (Scheduler* s = Current()) s->ReloadConfig();
}

void ScheduleLog(const std::string& msg)
{
    if (Scheduler* s = Current()) s->Log(msg);
}

std::string ScheduleStatusJson()
{
    Scheduler* s = Current();
    return s ? s->StatusJson() : std::string("null");
}

// ===========================================================================
//  The R2 / R4 bodies on the production Hub (see ElaSchedule.h)
// ===========================================================================
namespace {

class SlotClock : public Clock          // R2's Clock, fixed at the slot (golden Now() of the tick)
{
public:
    explicit SlotClock(double t) : t_(t) {}
    double Now() const { return t_; }
private:
    double t_;
};

std::string HandlerIdOf(const JobContext& ctx)
{
    // golden FormShow :421-424 sHandlerID: Gerneral.ini [Version] Machine ID, "HT-90xx" without it (= W906_ElaStart)
    const std::string id = ctx.cfg ? Trim(ctx.cfg->machineId) : std::string();
    return id.empty() ? std::string("HT-90xx") : ctx.cfg->machineId;
}

// golden Timer2 :3073-3079 (R2 SetPickers): the slot's day 00:00:00 .. 23:59:59; folders / filters = the page defaults
QueryRequest DayRequest(const JobContext& ctx, SysDate* sys)
{
    int y, m, d, hh, nn, ss;
    CivilFromLocalSec(ctx.slot, &y, &m, &d, &hh, &nn, &ss);
    if (sys) { sys->YY = y; sys->MM = m; sys->DD = d; }
    QueryRequest q;
    q.handlerId = HandlerIdOf(ctx);
    const double day = EncodeDate(y, m, d);
    volatile double et = day + EncodeTime(23, 59, 59, 0);   // as R2 (x87: store the sum once)
    q.startDate = day;
    q.startTime = day;
    q.endDate = day;
    q.endTime = et;
    q.top5Row = 1;
    return q;
}

JobStatus FromReport(const ReportResult& r, bool switchOn, JobContext& ctx)
{
    ctx.detail = r.note + (r.path.empty() ? std::string() : " -- " + r.path);
    unsigned long long n = 0;
    if (r.written && FileSizeA_(r.path, &n) && n > 0) return JOB_VERIFIED;   // R2 wrote it; it is there and not empty
    if (r.written) { ctx.detail += " (missing or empty after the write)"; return JOB_RETRY; }
    if (!r.prodOk) return JOB_FAILED;             // golden mode only: golden's exception ended the job for good
    return switchOn ? JOB_RETRY : JOB_NOTHING_TO_DO;   // e.g. the target folder (a network drive) is not there
}

Options JobOptions(const Options& opt, const ElaConfig& c)
{
    Options o = opt;
    o.custCode = c.sCustCode;                     // Hub::ReadConfig -> analyzer_.SetCustCode (the VTEST file rule)
    return o;
}

}  // namespace

JobStatus RunO06OnHub(Hub* hub, const Options& opt, JobContext& ctx)
{
    if (!hub) { ctx.detail = "no hub"; return JOB_NO_BODY; }
    const ElaConfig c = hub->Config();
    Analyzer a(JobOptions(opt, c));
    SysDate sys;
    const QueryRequest q = DayRequest(ctx, &sys);
    const SlotClock clk(ctx.slotDateTime);
    return FromReport(O06SaveSummaryData(a, c, q, clk, &sys), c.chkO06Production, ctx);
}

JobStatus RunN10OnHub(Hub* hub, const Options& opt, JobContext& ctx)
{
    if (!hub) { ctx.detail = "no hub"; return JOB_NO_BODY; }
    const ElaConfig c = hub->Config();
    Analyzer a(JobOptions(opt, c));
    SysDate sys;
    const QueryRequest q = DayRequest(ctx, &sys);
    const SlotClock clk(ctx.slotDateTime);
    return FromReport(N10SaveSummaryData(a, c, q, clk, &sys), c.cbN10_3, ctx);
}

JobStatus RunVTestOnHub(Hub* hub, const Options& opt, JobContext& ctx)
{
    if (!hub) { ctx.detail = "no hub"; return JOB_NO_BODY; }
    const ElaConfig c = hub->Config();
    Analyzer a(JobOptions(opt, c));
    QueryRequest ui;
    ui.handlerId = HandlerIdOf(ctx);
    const SlotClock clk(ctx.slotDateTime);        // DoVTestSaveSummary: Now()-7 .. Now()-1 of the slot
    return FromReport(DoVTestSaveSummary(a, c, ui, clk), true, ctx);
}

JobStatus RunN25OnHub(Hub* hub, const Options& opt, JobContext& ctx)
{
    if (!hub) { ctx.detail = "no hub"; return JOB_NO_BODY; }
    const ElaConfig c = hub->Config();
    // the context Hub::RunUploadJob builds (ElaChipMos.cpp), from the public side: no FtpAbortSlot (Hub::Stop then
    // waits for a running transfer's own time-outs instead of aborting it)
    ElaUploadContext x;
    x.opt = JobOptions(opt, c);
    x.logRoot = ElaLogRoot();
    x.eventLogDir = QueryRequest().eventLogDir;
    x.hostName = ElaHostName();
    x.machineId = c.edN04_ID;
    x.now = ctx.slotDateTime;                     // SaveJamCodeFor7Days range, JamWeek folder, BackUp name: the slot
    x.factory = hub->GetFtpFactory();
    x.slot = 0;
    N25Settings s;
    s.o10 = ctx.manual ? true : AutoJobsEnabled(c);   // D-a for the timed run; a manual run is not gated by O10
    s.custCode = c.sCustCode;
    s.enableJamLog = ctx.cfg ? ctx.cfg->n25_3 : false;
    s.user = c.edtN25_2_Name;
    s.password = c.edtN25_2_Password;
    s.host = c.edtN25_2_Host;
    s.jamLogPath = c.edtN25_3_LogJamPath;
    const N25Result r = UploadJamCode(x, s);
    ctx.detail = r.detail;                        // masked by R4
    if (!r.gate.empty()) { ctx.detail = "not connected: " + r.gate; return JOB_FAILED; }
    if (r.alreadyUploaded) { ctx.detail = r.saveFolder + "\\HadUpload.txt is there (golden: once a day)"; return JOB_NOTHING_TO_DO; }
    if (r.ok()) return JOB_VERIFIED;
    if (r.logOnly) return JOB_NOTHING_TO_DO;      // a log-only transport sent nothing (no build installs one since ★W36 = C)
    return r.retryable ? JOB_RETRY : JOB_FAILED;
}

// AI(W906-ELA-W22) 20260928 (St02-E): N25-4 / N25-5, the context RunN25OnHub builds plus the Production_Log root
static JobStatus RunN25UploadOnHub(Hub* hub, const Options& opt, JobContext& ctx, bool summary)
{
    if (!hub) { ctx.detail = "no hub"; return JOB_NO_BODY; }
    const ElaConfig c = hub->Config();
    const QueryRequest def;                       // W906_EVENTLOG_ROOT / W906_PRODLOG_ROOT, else golden's literals
    ElaUploadContext x;
    x.opt = JobOptions(opt, c);
    x.logRoot = ElaLogRoot();
    x.eventLogDir = def.eventLogDir;
    x.prodLogDir = def.prodLogDir;
    x.hostName = ElaHostName();
    x.machineId = c.edN04_ID;
    x.now = ctx.slotDateTime;                     // the 30 days / yesterday, the dated folder, the BackUp name: the slot
    x.factory = hub->GetFtpFactory();
    x.slot = 0;
    N25UploadSettings s;
    s.o10 = ctx.manual ? true : AutoJobsEnabled(c);   // D-a for the timed run; a manual run is not gated by O10
    s.custCode = c.sCustCode;
    s.enable = ctx.cfg ? (summary ? ctx.cfg->n25_4 : ctx.cfg->n25_5) : false;
    s.user = c.edtN25_2_Name;
    s.password = c.edtN25_2_Password;
    s.host = c.edtN25_2_Host;
    s.path = summary ? c.edtN25_4_UploadPath : c.edtN25_5_UploadPath;
    const N25Result r = summary ? UploadSummaryCount(x, s) : UploadEventLog(x, s);
    ctx.detail = r.detail;                        // masked by R4
    if (!r.gate.empty()) { ctx.detail = "not connected: " + r.gate; return JOB_FAILED; }
    if (r.alreadyUploaded) { ctx.detail = r.saveFolder + "\\HadUpload.txt is there (golden: once a day)"; return JOB_NOTHING_TO_DO; }
    if (r.sourceMissing) return JOB_NOTHING_TO_DO;   // no record in the 30 days / no event log of yesterday
    if (r.ok()) return JOB_VERIFIED;
    if (r.logOnly) return JOB_NOTHING_TO_DO;      // a log-only transport sent nothing (no build installs one since ★W36 = C)
    return r.retryable ? JOB_RETRY : JOB_FAILED;
}

JobStatus RunN25_4OnHub(Hub* hub, const Options& opt, JobContext& ctx) { return RunN25UploadOnHub(hub, opt, ctx, true); }
JobStatus RunN25_5OnHub(Hub* hub, const Options& opt, JobContext& ctx) { return RunN25UploadOnHub(hub, opt, ctx, false); }

bool ScheduleUseHubJobs(Hub* hub, const Options& opt)
{
    if (!hub || !Current()) return false;
    ScheduleSetJob(JOB_O06_4, [hub, opt](JobContext& c) { return RunO06OnHub(hub, opt, c); });
    ScheduleSetJob(JOB_N10_3, [hub, opt](JobContext& c) { return RunN10OnHub(hub, opt, c); });
    ScheduleSetJob(JOB_O19_VTEST, [hub, opt](JobContext& c) { return RunVTestOnHub(hub, opt, c); });
    ScheduleSetJob(JOB_N25_3, [hub, opt](JobContext& c) { return RunN25OnHub(hub, opt, c); });
    ScheduleSetJob(JOB_N25_4, [hub, opt](JobContext& c) { return RunN25_4OnHub(hub, opt, c); });   // W22
    ScheduleSetJob(JOB_N25_5, [hub, opt](JobContext& c) { return RunN25_5OnHub(hub, opt, c); });   // W22
    return true;
}

}  // namespace ela
