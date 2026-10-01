// ===========================================================================
//  EventLogAnalysis/ElaSchedule.h -- the time-based ELA jobs (ELA plan R5, W21 / W13 / #22 D-a..D-f), no UI.
//  AI(W906-ELA-R5) 20260927 (St02-E).  Plan: ela-reports-upload-plan.md §2 / §3 / §4 R5 / §6 (skill
//  ht9045-eventlog-analyzer, references folder); ledger docs/ELA_PORT_LEDGER.md "R5 ElaSchedule".
//
//  What golden had (all fire in the same second on every machine, plan §5):
//    O06-4 / N10-3   analyzer TfrmELA::Timer2Timer (Rev891 Analyzer.cpp:3010-3088, 50 s timer + minute lock
//                    CurrentNN): O06-4 every 1 / 10 / 30 minutes (:3026-3040), N10-3 at 00:00 / 08:00+20:00 /
//                    every hour (:3042-3068); either one runs BOTH saves (:3071-3083, O06SaveSummaryData +
//                    N10SaveSummaryData, each checks its own box).
//    N10-3 method 3  the Handler's own "specified time" (CC_CYUEAN, Jimmychiu 20250912): TFormHS::TimerAutoBackupTimer,
//                    906_0625_Steven HS_Function.cpp:488-509 -- once while the clock is IN the minute of
//                    IniConfig.dN10_3_1_SpecifiedTime (bN10_3_1_Flag), no catch-up.  The analyzer clamps method 3 to
//                    every hour (rgN10_3_1 has three items).  ★W44 B (Steven 20260928; AI(W906-ELA-W44) 20260928):
//                    JOB_N10_3 runs once a day at that minute (ReadScheduleConfig / AddN10Rules / Scheduler::Tick).
//                    ★W44-2 = catch up (Steven 20260928; AI(W906-ELA-W44B) 20260928): a time missed while the
//                    Handler was off runs once after the start (D-b below; golden has no catch-up).
//    N25-3           Handler TfMain::Timer2Timer 00:00:01..04 -> EL_UPLOAD_JAMWEEK (906_0625_Steven main.cpp:21442-21447,
//                    inside #ifndef SOFT_SIMULTE :21425-21459), CC_ChipMos_ZHUBEI 851 only (cConfiguration.cpp:3729).
//    N25-4 / N25-5   the same second, after it: EL_UPLOAD_SUMMARY (bN25_4_EnableUpload, main.cpp:21449-21452) and
//                    EL_UPLOAD_EVENTLOG (bN25_5_EnableUpload, :21454-21457); the analyzer runs them in its queue order
//                    JAMWEEK > SUMMARY > EVENTLOG (Rev891 Analyzer.cpp:161-183).  W22, AI(W906-ELA-W22) 20260928.
//    O19 VTEST       Handler TFormHS::TimerAutoBackupTimer, CheckClockTrigger(60) (edge: arms HH:00:00..02, fires
//                    after HH:00:03, HS_Function.cpp:4312-4328) + weekday iO19_WeekPeriod (0 = Sunday) + 00:00..00:05
//                    (:257-272) -> EL_VTEST_MTBF_SUM once a week; CC_VTEST 915 / 919 (CosFunction.cpp:2291-2299).
//    N17 (W13)       Handler CheckClockTrigger(1) 01:00 -> TfMain::UploadProdLog (HS_Function.cpp:204-207;
//                    Command.cpp:12401-12447 = V906 Command.cpp:3608-3654): CopyFile(yesterday's Production_Log,
//                    asN17ProductionLogPath, bFailIfExists=TRUE).  A file copy, not FTP.
//
//  Here (W21: every time-based trigger lives in ElaHub, on its worker; plan §3):
//    - an injectable clock (IClock; SchedSystemClock = GetLocalTime); time = local seconds since 1899-12-30 (TDateTime*86400);
//    - a job table: id + body (JobFn) + kind (local save / network drive / FTP).  R2 (ElaReports) and R4 (ElaFtp) put
//      the real O06 / N10 / VTEST / N25-3 bodies in at merge time (Scheduler::SetJob / ScheduleSetJob); until then those
//      four are "no body" (logged once, nothing persisted).  N17 has its real body here (RunN17UploadProdLog);
//    - gates (JobEnabled): D-a O10 (EffectiveO10, ElaReports.h: the Handler's live value, else golden cprod.cpp:2379-2394
//      on config.ini) for the analyzer's jobs; the function switch per job (D-f); customer codes 851 / 915 / 919 (868 N34 is a lot-end event:
//      CustomerCodeAllows); the golden SIM rule for N25-3 (no automatic send in a SOFT_SIMULTE build);
//    - ★W43 = C (Steven 20260928; AI(W906-ELA-W43) 20260928, St02-E helper): O06-4 follows the Handler's [O06-8]
//      "Update Production Record every 10 / 30 minutes" ([Event Log] EnanleTimePeriodSaveLog / TimePeriodSaveLog):
//      unchecked = no timed SummaryData save at all (golden's analyzer saved every minute, G3; W18 B had made that once
//      an hour), checked = every 10 / 30 min.  A manual run (ScheduleRunNow) is not a timed save and still works;
//    - stagger: offset = FNV-1a-32(Machine ID) mod W, W = 900 s FTP, 300 s network drive (a UNC / DRIVE_REMOTE target,
//      or O06-4 with [O06-6] Use Net Drive; for O06-4 capped at half its period so a slot always runs before the next
//      one), local saves not staggered (plan §3 item 4; St02-E 20260927).  The DATA of a run always comes from the
//      scheduled slot (JobContext::slot), never from the moment it ran.  Each job decides alone (its own switch,
//      period and stagger): golden's one bNeedUpload flag for O06 + N10 (:3071-3083) is a deviation (W18 B);
//    - back-off: wait n = min(60*2^(n-1), 1800) s * (0.8 + 0.4*u), u = fmix32(FNV-1a-32(Machine ID + "#" + n)); at most
//      kMaxRetries = 6 retries (1 / 2 / 4 / 8 / 16 / 30 min), or until the job's next scheduled slot (plan §3 item 5);
//    - D-b boot catch-up (N10-3 at the specified time too: ★W44-2 = catch up, Steven 20260928 -- golden has none): the
//      most recent slot missed while the Handler was off runs once at boot + offset; the last
//      VERIFIED slot per job is kept in a small state file (<log root>\UploadFile\ElaScheduleState.ini) and moves
//      only on JOB_VERIFIED / JOB_NOTHING_TO_DO (plan §3 item 8);
//    - FTP_Log lines for every decision (golden TMyStringList "FTP_Log", Rev891 Analyzer.cpp:220-224, file
//      <log root>\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv, MyStringList.cpp:526/:642) through ScheduleLog; the log root
//      is W906_HT9045LOG_ROOT when set (the as9045LogPath seam, common.cpp:240), else golden "D:\HT9045_Log".
//      The transport's own FTP_Log lines are R4's (ElaFtp); both run on the hub worker, one at a time.
//  Job bodies MUST regenerate their report from the slot's data range (W21: never append) and return JOB_VERIFIED
//  only after checking the result (plan §3 item 1 for FTP; same size for a copy).
//
//  Threads: Tick() and every job body run on the caller's thread (the hub worker).  RunNow / ReloadConfig /
//  StatusJson / Log / ScheduleLog may be called from any thread.  ScheduleInstall before the worker starts,
//  ScheduleUninstall after it is joined (W906_ElaStart / W906_ElaStop sequencing, ElaService.cpp).
// ===========================================================================
#ifndef HT9045_ELA_ELASCHEDULE_H
#define HT9045_ELA_ELASCHEDULE_H

#include "WebBridge/Sync.h"
#include "EventLogAnalysis/ElaFtp.h"     // ela::Fnv1a32 (R4 defines the one copy; a second one = a duplicate symbol)

#include <functional>
#include <string>
#include <vector>

namespace ela {

// ---------------------------------------------------------------------------
//  Time: local wall-clock seconds since 1899-12-30 00:00 (= TDateTime * 86400, whole seconds)
// ---------------------------------------------------------------------------
typedef long long LocalSec;
LocalSec LocalSecFromCivil(int y, int m, int d, int hh, int nn, int ss);
void CivilFromLocalSec(LocalSec t, int* y, int* m, int* d, int* hh, int* nn, int* ss);
int DayOfWeek0(LocalSec t);                        // 0 = Sunday .. 6 = Saturday (golden Now().DayOfWeek()-1, HS :261)
std::string FormatLocalSec(LocalSec t);            // "yyyy/mm/dd hh:nn:ss"; t < 0 -> "-"
bool ParseLocalSec(const std::string& s, LocalSec* t);
inline double LocalSecToDateTime(LocalSec t) { return (double)t / 86400.0; }

class IClock
{
public:
    virtual ~IClock() {}
    virtual LocalSec Now() = 0;
};
class SchedSystemClock : public IClock             // not "SystemClock": ElaReports.h has an ela::SystemClock (R2's
{                                                  //   TDateTime clock) -- one name, two classes = one vtable symbol
public:
    LocalSec Now();                                // GetLocalTime (golden Now())
};

// ---------------------------------------------------------------------------
//  The job table
// ---------------------------------------------------------------------------
// W22 appended N25-4 / N25-5 (the state file keys by JobName, so an older file still loads).  The three FTP jobs share
// one stagger offset (the Machine ID's) and one slot, and Tick runs due jobs in id order: N25-3, N25-4, N25-5 = golden's.
enum JobId { JOB_O06_4 = 0, JOB_N10_3, JOB_N25_3, JOB_O19_VTEST, JOB_N17_PRODLOG, JOB_N25_4, JOB_N25_5, JOB_TOTAL };
enum JobKind { KIND_LOCAL = 0, KIND_NETDRIVE, KIND_FTP };
enum JobStatus
{
    JOB_VERIFIED = 0,     // done and checked -> the slot is recorded as done (state file)
    JOB_NOTHING_TO_DO,    // nothing to do for this slot (e.g. N17 source file missing, golden silent) -> recorded as done
    JOB_RETRY,            // failed, may work later -> back-off
    JOB_FAILED,           // failed, retrying cannot help (e.g. N17 target exists with another size) -> not retried
    JOB_NO_BODY           // no body registered yet (R2 / R4 not merged) -> nothing recorded
};
const char* JobName(int job);                      // "O06-4", "N10-3", "N25-3", "O19-VTEST", "N17-UploadProdLog", "N25-4", "N25-5"
const char* KindName(int kind);                    // "local", "netdrive", "ftp"
const char* StatusName(int status);

// What a job reads (ReadScheduleConfig; plain values, no passwords).  Keys and defaults: ElaSchedule.cpp.
struct ScheduleConfig
{
    bool o06_1;                 // D-a input: [Event Log] EnableAutoSaveEventLog (O06-1)
    bool o10Key;                // D-a input: [Event Log] bO10UseEventLogSaver as the O10 box saved it
                                //   effective O10 = EffectiveO10(o06_1, o10Key) at every decision (ElaReports.h)
    std::string custCode;       // Gerneral.ini [System] CUSTOMER_CODE
    bool o06Production;         // O06-4 [Event Log] EnableAutoSaveProductiont (chkO06Production)
    bool o06TimePeriod;         //       EnanleTimePeriodSaveLog ([O06-8] chkO06TimePeriod; ★W43 C: off = no timed save)
    int o06TimePeriodIdx;       //       TimePeriodSaveLog 0 = 10 min, else 30 min (cbO06TimePeriod)
    std::string o06Path;        //       AutoSaveProductionPath (edtO06Production)
    bool o06UseNetDrive;        //       bAlarmStatistAutoSaveNetDrive ([O06-6] Use Net Drive, chkO06UseNetDrive :3097)
    bool n10Daily;              // N10-3 [FTPUpLoad] bN10_DailyUploadProdData (cbN10_3; analyzer default true)
    int n10Method;              //       iN10UploadProductMethod 0 = 00:00, 1 = 08:00+20:00, 2 = every hour, 3 = none
    int n10UploadMethod;        //       iN10UploadMethod 1 (2+ clamps to 1) = network drive (sN10UploadDrivePath), else local
    std::string n10DrivePath;   //       sN10UploadDrivePath
    int n10SpecifiedMinute;     //       ★W44 B: method 3 once a day at this minute (0..1439), [FTPUpLoad]
                                //       dN10_3_1_SpecifiedTime (ReadScheduleConfig: missing = 00:00); -1 = the clamp
    bool n25_3;                 // N25-3 [ChipMos Function] bN25_3_EnableULJamLog
    std::string n25Host;        //       sN25_2_FTPHost (N25-2 host, used by N25-3 / N25-4 / N25-5)
    bool n25_4;                 // N25-4 [ChipMos Function] bN25_4_EnableUpload (W22)
    bool n25_5;                 // N25-5 [ChipMos Function] bN25_5_EnableUpload (W22)
    bool o19Week;               // O19   [Event Log] bO19_AutoRecordReportByEveryWeek
    int o19WeekDay;             //       iO19_WeekPeriod (0 = Sunday .. 6 = Saturday)
    std::string o19Path;        //       asO19_SavePath
    bool n17;                   // N17   [Production_Log] bN17UploadProdLog
    std::string n17Path;        //       asN17ProductionLogPath
    std::string prodLogRoot;    //       asTravelingLogPath (W906_PRODLOG_ROOT seam, common.cpp:253)
    std::string machineType;    //       IniConfig.sMachineType = Gerneral.ini [Version] Model (cprod.cpp:2968)
    std::string socketHandlerId;//       IniConfig.SocketHandlerID (Machine ID; " " when empty; CC_ASE_CL 933 = computer name)
    std::string pcName;         //       PC_NAME = gethostname (main.cpp:11091)
    bool spil;                  //       IniConfig.bSPILFunction (not in any ini: inferred, see ReadScheduleConfig)
    int spilForQle;             //       Gerneral.ini [System] SPIL_FOR_QLE (database.cpp:729-736)
    std::string machineId;      // stagger key source: Gerneral.ini [Version] Machine ID as stored
    std::string hostName;       // stagger fallback when Machine ID is "" or "HT-90xx" (plan §3 item 4)
    ScheduleConfig();
};
// Read-only (never writes a key: the Hub's ReadConfig and the Handler do the golden write-back).  false = config.ini
// could not be read (the caller keeps what it had).
bool ReadScheduleConfig(const std::string& configIni, const std::string& generalIni, const std::string& hostName,
                        ScheduleConfig* out);

struct JobContext
{
    int job;
    LocalSec slot;              // the scheduled slot: data ranges come from here (plan §3 item 4)
    double slotDateTime;        // the same as a TDateTime
    int attempt;                // 1 = first try
    bool catchUp;               // D-b: a slot missed while the Handler was off
    bool manual;                // ScheduleRunNow
    const ScheduleConfig* cfg;
    unsigned slotFlags;         // in / out, kept across the attempts of one slot (reset for a new slot)
    std::string detail;         // out: one short line for FTP_Log and the history (no passwords)
};
typedef std::function<JobStatus(JobContext&)> JobFn;

// ---------------------------------------------------------------------------
//  Pure parts (ctest ELA_Schedule)
// ---------------------------------------------------------------------------
// Fnv1a32: ElaFtp.h (FNV-1a 32-bit; "" -> 0x811C9DC5) -- the same function R4 uses for its FTP_Log / retry seeds
std::string StaggerKey(const std::string& machineId, const std::string& hostName);
int StaggerOffsetSec(const std::string& key, int windowSec);                 // Fnv1a32(key) % W; W <= 0 -> 0
const int kMaxRetries = 6;
int BackoffBaseSec(int n);                                                   // min(60*2^(n-1), 1800), n >= 1
int BackoffDelaySec(const std::string& key, int n);                          // base * (0.8 + 0.4*u), rounded (u: header)

typedef int (*DriveTypeFn)(const std::string& rootPath);                     // GetDriveTypeA shape ("X:\\")
bool IsRemotePath(const std::string& path, DriveTypeFn driveType);           // UNC, or a DRIVE_REMOTE drive letter
// AI(W906-W58) 20260930 (St02-E): W58 Q5 = 算 (Steven 20260929 08:1x 「算, 因為有些電腦沒有打開對應的連結, 功能會失效」): in the SIM
//   build the ELA writes nothing under a network-share path -- IsRemotePath's rule (UNC, //, a DRIVE_REMOTE letter), the
//   same one as the scheduler's KIND_NETDRIVE, so a mapped Z: counts too -- unless it was started with
//   W906_SIM_NET_PATHS=1.  A timed or a manual run alike.  The SHIP build is unchanged.
bool SimNetPathsAllowed();                                                   // W906_SIM_NET_PATHS == "1"
bool SimBlocksNetPath(const std::string& path, bool simBuild, bool allowNet, DriveTypeFn driveType);
bool SimBlocksNetPathNow(const std::string& path);                           // this build + the environment + GetDriveTypeA
int O06PeriodMin(const ScheduleConfig& c, bool keepGoldenBugs);              // [O06-8] 10 / 30; else 0 = never (★W43 C), golden 1
int N10EffectiveMethod(const ScheduleConfig& c);                             // golden rgN10_3_1 clamp: >2 -> 2, <-1 -> -1
JobKind ResolveKind(int job, const ScheduleConfig& c, DriveTypeFn driveType);
int StaggerWindowSec(int job, JobKind kind, const ScheduleConfig& c, bool keepGoldenBugs);
// automatic = a timed run (O10, the SIM rule and O06-4's [O06-8] apply); false = ScheduleRunNow (switch + customer
// code only).  keepGoldenBugs = ScheduleSetup::keepGoldenBugs: golden's every-minute O06-4 needs no [O06-8] (★W43 C).
bool JobEnabled(int job, const ScheduleConfig& c, bool simBuild, bool automatic, std::string* why, bool keepGoldenBugs = false);
// the golden customer-code gate of an EL_* command (ElaHub.h EventLog_COMMAND): JAMWEEK / SUMMARY / EVENTLOG 851,
// CHIPADV_LOTEND 868, VTEST_MTBF_SUM 915 / 919, others any.  For R3 / R4 at merge time.
bool CustomerCodeAllows(int elCommand, const std::string& custCode);

struct SlotRule
{
    enum Type { EVERY_N_MIN = 0, DAILY_AT, WEEKLY_AT } type;
    int periodMin;              // EVERY_N_MIN (divides 1440)
    int minuteOfDay;            // DAILY_AT / WEEKLY_AT
    int weekday;                // WEEKLY_AT, 0 = Sunday
};
struct Calendar
{
    std::vector<SlotRule> rules;                     // union of the rules; empty = never
    bool Latest(LocalSec now, LocalSec* slot) const; // the latest slot <= now
    bool Next(LocalSec slot, LocalSec* next) const;  // the first slot > slot
};
Calendar JobCalendar(int job, const ScheduleConfig& c, bool keepGoldenBugs);

// N17 (W13): golden Command.cpp:12401-12447 file name and a checked copy.
std::string N17FileName(const ScheduleConfig& c, int y, int m, int d);      // :12408-12422
enum CopyVerify { CV_COPIED = 0, CV_ALREADY_SAME, CV_DEST_DIR_MISSING, CV_SOURCE_MISSING, CV_EXISTS_DIFFERENT,
                  CV_COPY_FAILED, CV_SIZE_MISMATCH };
// CopyFile(src, dst, !mayOverwrite) then same size; an existing dst of the same size counts as done (a retry after
// a copy that did land).  mayOverwrite only for a dst this job wrote itself in an earlier attempt of the same slot.
CopyVerify CopyFileAndVerify(const std::string& src, const std::string& dstDir, const std::string& dst, bool mayOverwrite,
                             std::string* detail);
JobStatus RunN17UploadProdLog(JobContext& ctx);                              // the default JOB_N17_PRODLOG body

// ---------------------------------------------------------------------------
//  The scheduler
// ---------------------------------------------------------------------------
typedef void (*ScheduleLogSink)(LocalSec now, const std::string& msg);

struct ScheduleSetup
{
    IClock* clock;                                   // not owned; 0 = a SchedSystemClock
    std::function<bool(ScheduleConfig*)> readConfig; // empty = ReadScheduleConfig(configIni, generalIni, hostName)
    std::string configIni;                           // ElaConfigIniPath(): golden D:\HT9045\config\config.ini / W906_AUTH_PATH
    std::string generalIni;                          // W906_GENERAL_INI_PATH, else golden asGeneralPath
    std::string logRoot;                             // W906_HT9045LOG_ROOT, else golden D:\HT9045_Log ("" = no file)
    std::string stateFile;                           // "" = <logRoot>\UploadFile\ElaScheduleState.ini
    std::string hostName;                            // "" = this computer (GetComputerNameExA DNS host name)
    int configEverySec;                              // config re-read period (60)
    bool keepGoldenBugs;                             // true = golden G3 (O06-4 every minute) + O06/N10 coupling
    bool simBuild;                                   // default: the build (no W906_NO_SOFT_SIMULTE = SIM)
    bool simNetPaths;                                // W58 Q5: SIM runs a KIND_NETDRIVE job only when true (default SimNetPathsAllowed())
    DriveTypeFn driveType;                           // 0 = GetDriveTypeA
    ScheduleLogSink logSink;                         // 0 = the FTP_Log file under logRoot
    ScheduleSetup();
};

class Scheduler
{
public:
    explicit Scheduler(const ScheduleSetup& s);
    ~Scheduler();

    void SetJob(int job, const JobFn& fn);           // merge time (before the first Tick); empty fn = no body
    void Tick();                                     // one pass; the first one is the boot (D-b catch-up)
    bool RunNow(int job);                            // manual: queued for the next Tick (any thread)
    void ReloadConfig();                             // re-read at the next Tick (EL_UPDATE_PARAMETER)
    void Log(const std::string& msg);                // FTP_Log + history (any thread)
    std::string StatusJson() const;                  // {now, maxRetries, jobs[], history[]} (UTF-8 JSON, no passwords)
    //   jobs[]: id, enabled, why (the gate that is closed), kind, windowSec, offsetSec, done, armed, pending, slot, due,
    //   next (the next run: due when pending, else the next slot + offset; "-" when off), attempt, retries, runs, hasBody,
    //   lastAt, lastStatus, lastDetail, lastManual, last, results[] (the job's own record) -- AI(W906-ELA-R6) 20260928

    struct JobState
    {
        bool enabled, initialized, pending, pendingCatchUp;
        std::string why;
        JobKind kind;
        int windowSec, offsetSec, attempt, runs;
        LocalSec cursor, done, armed, pendingSlot, pendingDue;
        unsigned slotFlags;
        std::string lastResult;
        std::vector<std::string> results;            // this job's own record (last 20 lines), for R6 -- not the page snapshot
        LocalSec lastAt;                             // AI(W906-ELA-R6) 20260928: the last run / refusal (-1 = none)
        std::string lastStatus, lastDetail;          //   StatusName(...) or "manual refused"; the body's detail / the reason
        bool lastManual;
        bool noBodyLogged;
        JobState();
    };
    JobState State(int job) const;                   // the worker's view (tests; single thread)
    const ScheduleConfig& Config() const { return cfg_; }
    std::vector<std::string> History() const;
    std::string StateFile() const { return stateFile_; }

private:
    void ReadConfigNow(LocalSec now);
    void LoadState();
    void SaveState();
    void RunJob(int job, LocalSec now, bool manual);
    void Event(int job, LocalSec now, const std::string& event, LocalSec slot, const std::string& detail);
    void Publish(LocalSec now);

    ScheduleSetup setup_;
    IClock* clock_;
    SchedSystemClock sysClock_;
    std::string stateFile_;
    JobFn fn_[JOB_TOTAL];
    JobState st_[JOB_TOTAL];                         // worker only
    ScheduleConfig cfg_;                             // worker only
    bool haveConfig_, booted_;
    LocalSec bootAt_, configAt_;
    mutable webbridge::WbMutex mu_;                  // manual_, reload_, history_, status_
    webbridge::WbMutex logMu_;                       // the sink (file appends)
    bool manual_[JOB_TOTAL];
    bool reload_;
    std::vector<std::string> history_;
    std::string status_;
    std::string statusNow_;                          // the clock at the last Publish (R6)
};

// ---------------------------------------------------------------------------
//  The process-wide scheduler (W906_ElaStart / the hub worker; see ElaSchedule.cpp for the wiring lines)
// ---------------------------------------------------------------------------
void ScheduleInstall(const ScheduleSetup& s);        // replaces an earlier one; N17 gets its default body
void ScheduleUninstall();
void ScheduleTick();                                 // no-op when not installed
bool ScheduleSetJob(int job, const JobFn& fn);
bool ScheduleRunNow(int job);
void ScheduleReloadConfig();
void ScheduleLog(const std::string& msg);            // no-op when not installed
std::string ScheduleStatusJson();                    // "null" when not installed

// ---------------------------------------------------------------------------
//  The R2 / R4 bodies on the production Hub (merge time; no ElaHub / ElaService edit needed for the bodies).
//  They use only Hub::Config() (the EL_UPDATE_PARAMETER configuration) and Hub::GetFtpFactory(); each run has an Analyzer
//  of its own (a job never replaces the page's query: St02-E 20260927, the R2 follow-up) and a Clock fixed at the slot,
//  so the data range, the report's date and the N25-3 BackUp name come from the slot (plan §3 item 4).  opt = the
//  Options W906_ElaStart gives the Hub (ctest: a %TEMP% jamIniPath).  Results: JOB_VERIFIED only when R2's file is on
//  disk and not empty / R4's N25Result::ok() (STOR + SIZE + LIST); a switch that is on but a folder / upload that failed
//  = JOB_RETRY (back-off); HadUpload.txt already there / the log-only transport = JOB_NOTHING_TO_DO.
// ---------------------------------------------------------------------------
class Hub;
struct Options;
JobStatus RunO06OnHub(Hub* hub, const Options& opt, JobContext& ctx);     // R2 O06SaveSummaryData, the slot's day
JobStatus RunN10OnHub(Hub* hub, const Options& opt, JobContext& ctx);     // R2 N10SaveSummaryData, the slot's day
JobStatus RunVTestOnHub(Hub* hub, const Options& opt, JobContext& ctx);   // R2 DoVTestSaveSummary, slot-7 .. slot-1
JobStatus RunN25OnHub(Hub* hub, const Options& opt, JobContext& ctx);     // R4 UploadJamCode on the Hub's transport
// W22 (AI(W906-ELA-W22) 20260928): N25-4 UploadSummaryCount / N25-5 UploadEventLog on the Hub's transport, the slot's
// day (Now-31 .. Now-1 / yesterday's file); nothing to send (no record / no file) = JOB_NOTHING_TO_DO
JobStatus RunN25_4OnHub(Hub* hub, const Options& opt, JobContext& ctx);
JobStatus RunN25_5OnHub(Hub* hub, const Options& opt, JobContext& ctx);
bool ScheduleUseHubJobs(Hub* hub, const Options& opt);  // registers the six above; false = no scheduler installed

}  // namespace ela

#endif
