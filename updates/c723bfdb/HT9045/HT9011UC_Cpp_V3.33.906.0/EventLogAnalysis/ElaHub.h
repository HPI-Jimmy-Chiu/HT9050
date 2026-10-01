// ===========================================================================
//  EventLogAnalysis/ElaHub.h -- Event Log Analyzer commands + worker (ELA plan P2 / P3), no UI.
//  AI(W906-ELA-P2) 20260927.  Golden: EventlogAnalyzer Rev891.0 TAnalysisProcessThread (Analyzer.cpp:154-215),
//  OnMyCopyMsg (:487-558), ReadConfig (:3090-3141), btnQueryClick (:2884-2895).  Ledger docs/ELA_PORT_LEDGER.md.
//
//  Golden shape: the Handler's SendCommand_EventLog (Interface/InterfaceSYS.cpp) sends WM_COPYDATA to the analyzer
//  window; OnMyCopyMsg only raises bEL_<cmd> (repeats coalesce); TAnalysisProcessThread runs one raised job per
//  >= 500 ms in the priority order UPDATE_PARAMETER > UPLOAD_JAMWEEK > UPLOAD_SUMMARY > UPLOAD_CHIPADV_LOTEND >
//  UPLOAD_EVENTLOG > UPLOAD_BYFILE_N10 > VTEST_MTBF_SUM.  In golden that "thread" Synchronize()s everything onto the UI
//  thread; here the jobs really run on the worker (plan: the analysis never runs on the machine tick).
//
//  Here: Hub::Post(cmd) = OnMyCopyMsg; Hub::RunOnce() = one AnalysisThreadProcess pass; Hub::Start()/Stop() = the
//  worker (WbThread, SleepEx(500) between passes).  Hub::PostQuery() = the page's Query button (golden btnQueryClick);
//  a query runs when no EL job is raised; one pending query, a newer one replaces it.  The result is a JSON snapshot
//  (Hub::SnapshotJson) for the /api/ela route -- that route, eventlog.html and the InterfaceSYS wiring are shared files
//  and are HELD (github-59), so nothing calls this outside the ctest yet.
//
//  Ported:  EL_UPDATE_PARAMETER -> ReadConfig into ElaConfig (golden widget names).  CheckAndReadIniData's write-back of
//           missing keys into config.ini is #17 ELA-3 (暫照 golden（A）, Options::jamWriteBack also covers it).
//  R2:      EL_VTEST_MTBF_SUM -> ElaReports DoVTestSaveSummary (AI(W906-ELA-R2) 20260927, St02-E; #22 ruled): only when
//           O10 is on (D-a: golden's analyzer does not run otherwise).  Hub::SetClock is the fake clock for ctests.
//  Jobs vs the page (St02-E 20260927, a deviation): every EL_* job leaves a JobRecord (SnapshotJson "jobs", for R6) and
//           NEVER replaces the page's snapshot -- that changes only on the user's own queries.  Golden's window showed
//           a job's range afterwards (its date pickers are the parameter bus, ledger §1.2).
//  R3:      EL_UPLOAD_CHIPADV_LOTEND -> ElaReports ChipAdvancedFunc::AnalysisLog (N34, AI(W906-ELA-R3) 20260927), same
//           O10 gate and job record.  The Handler's sender (V906 forms/fLotInfo.cpp:7082-7095, St01's) is still #if 0.
//  R4:      EL_UPLOAD_JAMWEEK (N25-3) and EL_UPLOAD_BYFILE_N10 run (ElaChipMos.cpp) on the Hub's IElaFtp behind the
//           configuration gate (#22 D-f); nothing in V906 posts them yet (the schedule is R5).  AI(W906-ELA-R4) 20260927
//           (St02-E); the upload jobs read the same Hub::SetClock clock.
//  W22:     EL_UPLOAD_SUMMARY (N25-4 Jam_Summary.csv) and EL_UPLOAD_EVENTLOG (N25-5 EventLog.txt) run the same way
//           (ElaChipMos.cpp; AI(W906-ELA-W22) 20260928, St02-E) -- no EL_* job is held any more.
// ===========================================================================
#ifndef HT9045_ELA_ELAHUB_H
#define HT9045_ELA_ELAHUB_H

#include "EventLogAnalysis/ElaCore.h"
#include "WebBridge/Sync.h"
#include "EventLogAnalysis/ElaFtp.h"   // AI(W906-ELA-R4) 20260927 (St02-E): the upload transport

#include <atomic>
#include <string>
#include <vector>

namespace ela {

// golden Common.h:31-41 enum EventLog_COMMAND (the Handler sends these values)
enum EventLog_COMMAND
{
    EL_UPDATE_PARAMETER = 0, EL_UPLOAD_JAMWEEK = 1, EL_UPLOAD_CHIPADV_LOTEND = 2, EL_UPLOAD_SUMMARY = 3,
    EL_UPLOAD_EVENTLOG = 4, EL_UPLOAD_BYFILE_N10 = 5, EL_VTEST_MTBF_SUM = 6, EL_COMMAND_TOTAL
};
const char* CommandName(int cmd);

// golden ReadConfig (:3090-3141): the values it puts into the widgets, under the golden widget names
struct ElaConfig
{
    bool cbO06, chkO06AlarmHistroy, chkO06AlarmStatist, chkO06Production, chkO06UseNetDrive, chkO06TimePeriod;
    int cbO06TimePeriod;
    std::string edO06_FilePath, edtO06AlarmHistroy, edtO06AlarmStatist, edtO06Production, edtO06_Remote, edtO06_Local;
    bool cbN10_1, cbN10_2, cbN10_3;
    int rgN10_3_1, rgN10_4;
    std::string edN10UserName, edN10Password, edN10Host, edN10UploadPath, edtN10_6, edtN10_8;
    std::string edtN25_2_Name, edtN25_2_Password, edtN25_2_Host, edtN25_3_LogJamPath, edtN25_4_UploadPath,
        edtN25_5_UploadPath, edN04_ID;
    bool cbN34;
    std::string edN34;
    bool cbO19_1, cbO19_2, cbO19_4;
    int coO19_3, coO19_5;
    std::string edtO19_6;
    std::string sCustCode;
    bool o10Stored;            // AI(W906-ELA-R5) 20260927: [Event Log] bO10UseEventLogSaver, read only (not golden's ReadConfig)
    ElaConfig();
};

// The page's query (golden: the date-time pickers, labPath / lblProdPath, cbTopAlarmFilter, rgFilter, the check boxes)
struct QueryRequest
{
    double startDate, startTime, endDate, endTime;
    std::string eventLogDir;   // golden labPath, default D:\HT9045_Log\EventLogTxt (W906_EVENTLOG_ROOT when set)
    std::string prodLogDir;    // golden lblProdPath (dirlstProdLog), default D:\HT9045_Log\Production_Log (W906_PRODLOG_ROOT)
    int topFilter;             // cbTopAlarmFilter 0 JAM / 1 WAR / 2 all
    int rgFilter;              // 0/1 By Area, 2 By Function
    bool area[eUnitNameTotal];
    bool func[eByFuncTotal];   // indexed by list (efRTC..efYield) -- see ElaCore.h about golden's crossed boxes
    int top5Row;               // UpdateSgTop5(ARow); golden's query uses 1
    std::string handlerId;
    bool includeAlarmList;     // golden chkIncludeAlarmList (unchecked in the dfm, never read from an ini): SaveSummary
                               //   adds the Top5 grids (ElaReports, R2)
    QueryRequest();
};

// golden btnQueryClick (:2884-2895) on q: SetStringGrid -- whose ImportProductionLod(2) scans both folders again every
// time (D-d for the reports) -- GetEventLogText with its four UpdateSg* (q's filters), ListProductionLog.  Returns
// ListProductionLog's result (false = golden threw out of the query; only with keepGoldenBugs).  AI(W906-ELA-R2) 20260927
bool BtnQuery(Analyzer& a, const QueryRequest& q);

class Clock;                   // ElaReports.h (golden Now())
struct ReportResult;           // ElaReports.h (what one report job did)

// one EL_* job run, kept apart from the page's query result (R6 shows these).  AI(W906-ELA-R2) 20260927
struct JobRecord
{
    std::string at;            // local time the job finished (yyyy/mm/dd hh:nn:ss)
    std::string cmd;           // CommandName
    bool ran, written;         // golden's conditions held / a report file was written
    std::string path, note;    // the file ("" = none); one line: what happened
    bool hasRange;             // the job queried: [from, to] (TDateTime)
    double from, to;
    JobRecord() : ran(false), written(false), hasRange(false), from(0), to(0) {}
};

// CheckAndReadIniData (golden Common.cpp:107-167): a missing key is written with the default when writeBack; the
// AnsiString form also rewrites a key whose value is "" while the default is not.  *writes counts the writes.
bool IniCheckAndReadBool(const std::string& file, const std::string& group, const std::string& name, bool value,
                         bool writeBack, int* writes);
int IniCheckAndReadInt(const std::string& file, const std::string& group, const std::string& name, int value,
                       bool writeBack, int* writes);
std::string IniCheckAndReadString(const std::string& file, const std::string& group, const std::string& name,
                                  const std::string& value, bool writeBack, int* writes);
// AI(W906-SIM-W36-1) 20260928 (St02-E helper), AI(W906-W58) 20260930: ★W36-1 / W58, the SIM build starts every
//   launch with the FTP / network options off.  Every boolean the ELA reads from config.ini (IniCheckAndReadBool above,
//   IniRead::Bool in ElaSchedule.cpp) passes IniBoolOverride(file, group, name, the file value): no override installed
//   (the SHIP build; every ctest that installs none) = the file value.  The SIM wb_serve installs SimNet/SimNetMask.cpp's
//   through W906_ElaSetBoolOverride (ElaIniOverride.cpp; AI(W906-B20-ELAWININET) 20261001: was ElaService.cpp): a key masked this run reads false, a key released this run reads
//   true (W58 Q1: the golden save has written its tick).  The function is called on the ELA worker thread.
typedef bool (*IniBoolOverrideFn)(const char* file, const char* group, const char* name, bool fileValue);
bool IniBoolOverride(const std::string& file, const std::string& group, const std::string& name, bool fileValue);

// Text from BCB-written logs is cp950 (Big5); V906-written text is UTF-8.  Valid UTF-8 passes; otherwise cp950 -> UTF-8.
std::string ToUtf8(const std::string& s);

// AI(W906-ELA-REV) 20260928 (St02-E): the config.ini ELA reads -- the Handler's AuthPath seam: W906_AUTH_PATH set ->
//   W906_AUTH_PATH + "config.ini" (common.cpp:139-168 / :406 take the variable verbatim, and the Handler composes
//   AuthPath+"config.ini", cprod.cpp:2193 / :2373 -- so it ends with a separator, as golden's "D:\HT9045\config\");
//   unset -> golden D:\HT9045\config\config.ini.  HubPaths::configIni and ScheduleSetup::configIni both use it.
std::string ElaConfigIniPath();

struct HubPaths
{
    std::string configIni;     // golden D:\HT9045\config\config.ini (W906_AUTH_PATH + "config.ini" when set)
    std::string generalIni;    // golden asGeneralPath D:\HT9045\system\Gerneral.ini (W906_GENERAL_INI_PATH when set)
    std::string summaryDir;    // AI(W906-ELA-P7a) 20260928: Save Summary's folder -- golden btnSaveSummaryClick's SaveDialog default
                               //   "D:\RMS\" (Rev891 Analyzer.cpp:2563); W906_RMS_ROOT when set (the Handler's D:\RMS seam,
                               //   common.cpp:263 asProductionLogPath)
    std::string timeDataDir;   // AI(W906-ELA-W48B) 20260928 (St02-E helper): ★W48-2 = B -- the Handler's hourly TimeData folder
                               //   (golden main.cpp:1539 D:\HT9045_Log\TimeData; ElaOee.h DefaultTimeDataDir: the
                               //   W906_HT9045LOG_ROOT seam when set).  Read only, by the OEE tab's query snapshot.
    HubPaths();
};

class Hub
{
public:
    Hub(const Options& o, const HubPaths& p);
    ~Hub();

    void Post(int cmd);                      // golden OnMyCopyMsg: raise bEL_<cmd> (repeats coalesce); bad values ignored
    void PostQuery(const QueryRequest& q);   // the Query button; replaces a query that has not started yet
    // AI(W906-ELA-P7a) 20260928 (St02-E): the Save Summary button (golden btnSaveSummaryClick :2557-2570 -> SaveSummary
    //   :2614-2688, R2 ElaReports::SaveSummary) on the page's own last query, run by the worker after a pending query.
    //   name = golden SaveDialog's file name ("" = its default, today's yyyy-mm-dd); one pending, a newer one replaces it.
    //   The result is a JobRecord (cmd "SaveSummary"); the page's query snapshot is not replaced.
    void PostSaveSummary(const std::string& name);
    std::string SummaryDir() const { return paths_.summaryDir; }
    bool RunOnce();                          // one golden AnalysisThreadProcess pass, else the pending query; false = idle

    bool Start();                            // the worker thread (RunOnce + SleepEx(500)), golden TAnalysisProcessThread
    void SetClock(const Clock* c);           // the reports' Now() (0 = the system clock); ctests set a fake
    void Stop();                             // stop and join

    std::string SnapshotJson(unsigned long* seq) const;   // last query result + state + job history (UTF-8 JSON)
    unsigned long Seq() const { return seq_.load(); }
    bool Busy() const { return busy_.load(); }
    ElaConfig Config() const;
    int ConfigWrites() const;
    std::vector<JobRecord> Jobs() const;     // the last 50 EL_* jobs, oldest first
    // AI(W906-ELA-R4) 20260927 (St02-E): the upload transport (ElaFtp.h).  Default = the null transport, so a Hub built
    //   anywhere but W906_ElaStart (ElaService.cpp) never reaches the network.  Set before Start().
    void SetFtpFactory(FtpFactory f) { ftpFactory_ = f ? f : &NewNullFtp; }
    FtpFactory GetFtpFactory() const { return ftpFactory_; }
    UploadRetry JamWeekRetry() const { return n25Retry_; }             // worker-owned: read it while idle

private:
    static void Entry(void* self);
    void ReadConfig();
    void RunQuery(const QueryRequest& q);
    void PublishSnapshot(bool prodOk);       // the Analyzer's last query -> snapshot_ (the user's queries only)
    void RunVTest();                         // EL_VTEST_MTBF_SUM (R2)
    void RunChipAdv();                       // EL_UPLOAD_CHIPADV_LOTEND (R3)
    void RecordReportJob(int cmd, const ReportResult& r);   // one job record + one history line
    void AddHistory(const std::string& line);
    void AddJob(const JobRecord& j);
    void RunUploadJob(int job);              // AI(W906-ELA-R4): EL_UPLOAD_JAMWEEK / _BYFILE_N10 (W22: _SUMMARY / _EVENTLOG), ElaChipMos.cpp
    void RecordUploadJob(int job);           // (R2 follow-up) the upload job's record = the history line it left
    void RunSaveSummary(const std::string& name);   // AI(W906-ELA-P7a)

    Options opt_;
    HubPaths paths_;
    Analyzer analyzer_;                      // one per Hub, alive across queries like golden's single TfrmELA (W18 B:
                                             //   nothing leaks from one query into the next unless keepGoldenBugs)
    webbridge::WbMutex runMu_;               // RunOnce is not re-entrant: the worker and a direct caller never overlap
    mutable webbridge::WbMutex mu_;          // guards flags_, pending query, snapshot, history, config
    bool flags_[EL_COMMAND_TOTAL];
    bool hasQuery_;
    QueryRequest query_;
    std::string snapshot_;
    std::vector<std::string> history_;
    std::vector<JobRecord> jobs_;            // guarded by mu_
    ElaConfig config_;
    int configWrites_;
    std::atomic<unsigned long> seq_;
    std::atomic<bool> busy_;
    std::atomic<bool> stop_;
    webbridge::WbThread thread_;
    QueryRequest base_;                      // the page state golden's EL_* jobs query with (last PostQuery; guarded by mu_)
    const Clock* clock_;                     // RunOnce only (R2; the R4 upload jobs read it too)
    FtpFactory ftpFactory_ = &NewNullFtp;    // AI(W906-ELA-R4) 20260927 (St02-E)
    FtpAbortSlot ftpSlot_;                   // Stop() cancels a transfer in flight
    UploadRetry n25Retry_;                   // plan §3 item 5 back-off (R5 acts on it)
    // AI(W906-ELA-P7a) 20260928 (St02-E): Save Summary.  hasSave_ / saveName_ guarded by mu_; the rest worker-only (RunOnce).
    bool hasSave_ = false;
    std::string saveName_;
    bool hasUserQuery_ = false;              // the page ran a query (RunQuery)
    QueryRequest lastUserQuery_;
    bool analyzerIsUser_ = false;            // analyzer_ still holds that query (VTEST / ChipAdv jobs query on analyzer_ too)
};

}  // namespace ela

#endif
