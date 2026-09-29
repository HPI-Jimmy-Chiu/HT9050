// ===========================================================================
//  EventLogAnalysis/ElaChipMos.h -- ChipMos ZHUBEI (CC 851) N25-3 Jam log upload and the N10 BYFILE upload (plan R4).
//  AI(W906-ELA-R4) 20260927 (St02-E).  Golden: EventlogAnalyzer Rev891 uChipMosZHUBEI_Func.cpp -- SaveJamCodeFor7Days
//  :25-56, N10_UploadDataToFTP :145-182, N25_UploadJamDataToFTP :184-255, UploadJamCode :328-350, UploadByFile_N10
//  :392-418 -- dispatched by TAnalysisProcessThread (Analyzer.cpp:161-166 EL_UPLOAD_JAMWEEK, :185-190
//  EL_UPLOAD_BYFILE_N10).  Transport: ElaFtp.h (IElaFtp).  Ledger docs/ELA_PORT_LEDGER.md "R4".
//
//  Deviations (plan §3, #22 D-a / D-b / D-e / D-f, W18 B; each in the ledger):
//    * an upload counts only when STOR + SIZE + a fresh LIST agree (UploadAndVerify); golden: "no exception" = success;
//    * HadUpload.txt only when BOTH the main file and the BackUp copy are verified (golden :227 writes "Upload path:" into
//      it after the main STOR, so a failed BackUp still blocks every retry that day, and :235-243 lets the BackUp result
//      alone decide); its "Upload backup path:" line names the BackUp folder (golden :237 prints the main folder);
//    * Jam Rate.txt is rebuilt (deleted, then written) instead of appended to on every run; UTF-8 (D-e), LF like golden;
//    * the records are re-read from the EventLogTxt folder at report time (D-d) through R1's
//      Analyzer::GetEventLogTextToVec (ElaCore, the Hub's W15 / W19 options) on an Analyzer of the job's own;
//    * the job connects only when the configuration says so (D-f): O10 (= O06-1, D-a), CUSTOMER_CODE 851 (golden only
//      loads bN25_3_EnableULJamLog for CC_ChipMos_ZHUBEI, 906_0625_Steven cConfiguration.cpp:3729 / :3743),
//      bN25_3_EnableULJamLog, a non-empty N25-2 host and N25-3 path (golden :188 checks the path);
//    * golden's SOFT_SIMULTE overrides (:206-210 account / 127.0.0.1, :342-344 path) are NOT carried over; the simulation
//      build uploads like the shipping one (★W36 = C, Steven 20260928, ElaFtp.h; AI(W906-ELA-W36) St02-E helper);
//    * the local roots use the Handler's W906_HT9045LOG_ROOT seam (golden literal "D:\\HT9045_Log", :332 / :396).
//  N10 BYFILE: golden has no sender (906_0625_Steven HS_Function.cpp:432-436 is commented out) and V906 has none either.
//  It runs only when O10 is on, iN10UploadMethod is 0 (FTP) and the N10 host is set (D-f; golden's analyzer never gates it).
//
//  W22 (AI(W906-ELA-W22) 20260928, St02-E; W22 = 「功能開關有開就做」): N25-4 Jam_Summary.csv and N25-5 EventLog.txt -- see
//  the "N25-4 / N25-5" block below.  Same transport, verify, HadUpload rule and D-f gate as N25-3.
// ===========================================================================
#ifndef HT9045_ELA_ELACHIPMOS_H
#define HT9045_ELA_ELACHIPMOS_H

#include "EventLogAnalysis/ElaCore.h"
#include "EventLogAnalysis/ElaFtp.h"

#include <string>
#include <vector>

namespace ela {

// what an upload job needs besides the configuration (the Hub fills it; the ctest injects every field)
struct ElaUploadContext
{
    Options opt;              // the Hub's options (W15 splitter / W19 file rule for the records)
    std::string logRoot;      // ElaLogRoot(): JamWeek\..., UploadFile\UploadFile.csv, UploadFile\...\FTP_Log_*.csv
    std::string eventLogDir;  // golden labPath (QueryRequest default, W906_EVENTLOG_ROOT)
    std::string prodLogDir;   // W22: golden literal D:\HT9045_Log\Production_Log (QueryRequest default, W906_PRODLOG_ROOT)
    std::string hostName;     // golden edtN04_Host = gethostname (Analyzer.cpp:429-432)
    std::string machineId;    // golden edN04_ID (Gerneral.ini [Version] Machine ID) -- the back-off seed
    double now;               // TDateTime of the run (golden Now())
    FtpFactory factory;       // the transport; 0 = the null transport
    FtpAbortSlot* slot;       // Hub::Stop cancels through it; may be 0
    ElaUploadContext();
};

// ---- N25-3 ----
struct N25Settings
{
    bool o10;                 // D-a: AutoJobsEnabled = EffectiveO10 (ElaReports.h; golden cprod.cpp:2379-2394) -- AI(W906-ELA-R5)
    bool enableJamLog;        // IniConfig.bN25_3_EnableULJamLog (config.ini [ChipMos Function], golden default 0)
    std::string custCode;     // Gerneral.ini [System] CUSTOMER_CODE
    std::string user, password, host;   // N25-2 account (golden edtN25_2_Name / _Password / _Host, Analyzer.cpp:3122-3124)
    std::string jamLogPath;   // golden edtN25_3_LogJamPath (sN25_3_JamLogFTPPath)
    N25Settings() : o10(false), enableJamLog(false) {}
};
std::string N25_3GateReason(const N25Settings& s);   // "" = the configuration allows the connection (D-f)

struct N25Result
{
    std::string gate;         // "" = open; otherwise why nothing was done (no transport was created)
    bool alreadyUploaded;     // HadUpload.txt was there (golden :336-339)
    bool connected, mainOk, backupOk, hadUploadWritten, logOnly, retryable;
    bool sourceMissing;       // W22: nothing to send (N25-4 no event-log record in the 30 days, N25-5 no file of yesterday)
    int records;              // JAM rows in Jam Rate.txt (N25-3) / JamCount summed over the intervals (N25-4)
    int factoryCalls;
    int intervals;            // W22 N25-4: rows of Jam_Summary.csv (CLEAN_OUT splits + 1)
    long long devices;        // W22 N25-4: Summary summed over the intervals
    std::string saveFolder, jamFile, backupName, detail;   // detail is masked
    std::string remoteMain, remoteBackup;                  // the two remote paths (HadUpload.txt lines)
    N25Result()
        : alreadyUploaded(false), connected(false), mainOk(false), backupOk(false), hadUploadWritten(false),
          logOnly(false), retryable(false), sourceMissing(false), records(0), factoryCalls(0), intervals(0), devices(0) {}
    bool ok() const { return gate.empty() && !alreadyUploaded && mainOk && backupOk && !logOnly; }
};

// golden SaveJamCodeFor7Days (:25-56): "Jam Rate.txt" in saveFolder, header + every record whose AlarmCode holds "JAM",
// range Now-8 00:00:00.000 .. Now-1 23:59:59.059 (golden EncodeTime(23,59,59,59)).  Rebuilt, UTF-8, LF.
bool SaveJamCodeFor7Days(const ElaUploadContext& c, const std::string& saveFolder, std::string* jamFileName, int* rows);
// golden N25_UploadJamDataToFTP (:184-255) with the verified uploads; fills r->connected / mainOk / backupOk / detail
bool N25_UploadJamDataToFTP(const ElaUploadContext& c, const N25Settings& s, const std::string& sourcesPath,
                            const std::string& targetPath, const std::string& jamFileName, N25Result* r);
// golden UploadJamCode (:328-350) behind the D-f gate
N25Result UploadJamCode(const ElaUploadContext& c, const N25Settings& s);
double NextN25Slot(double now);   // the next golden trigger time, 00:00:01 tomorrow (906_0625_Steven main.cpp:21442)

// ---- N25-4 / N25-5 (W22; AI(W906-ELA-W22) 20260928, St02-E) ----
//  Golden Rev891 uChipMosZHUBEI_Func.cpp: SaveJamSummaryFor30Days :58-132, SaveEventLog :134-143,
//  N25_UploadSummaryCountToFTP :257-326, UploadSummaryCount :352-370, UploadEventLog :372-390 (it uploads with N25-3's
//  N25_UploadJamDataToFTP :184-255); TAnalysisProcessThread :167-171 EL_UPLOAD_SUMMARY, :179-183 EL_UPLOAD_EVENTLOG.
//  The Handler raises both at 00:00:01..04 when bN25_4_EnableUpload / bN25_5_EnableUpload is on (906_0625_Steven
//  main.cpp:21449-21457, inside #ifndef SOFT_SIMULTE :21425), keys only for CC_ChipMos_ZHUBEI 851 (cConfiguration.cpp
//  :3729, :3745-3748).  Account = the N25-2 one (golden :275-277); remote = <path>/<hostname>/ + BackUp/<name>_yyyymmdd_hhnn.
//  Deviations (ledger "W22"):
//    * the D-f gate as N25-3 (O10, 851, the switch, the N25-2 host, the path) -- golden's analyzer reads no switch;
//    * verified uploads, HadUpload.txt only when BOTH copies are verified (golden: the BackUp result alone decides,
//      :307-314 / :235-243); N25-5's "Upload backup path:" names the BackUp folder (golden :237 prints the main one);
//      N25-4 writes only "HadUpload" (golden :367: N25_UploadSummaryCountToFTP has no log path);
//    * an FTP_Log line per run and per missing file ("N25-4 Jam_Summary upload", "N25-5 EventLog upload", "File not
//      exist N25-4/5"); golden writes none for N25-4, and N25-5's source is copied silently;
//    * no connection when there is nothing to send (golden connects first and checks the file after, :296 / :223);
//    * Jam_Summary.csv is rebuilt (golden appends: a second run the same day repeats the header and every row);
//      lines through ReportEncode (D-e; ASCII anyway), LF like golden;
//    * no event-log record in the 30 days: golden reads tIntervalRecs[size-1] of an empty vector (:95, ledger G9);
//      here no file and no upload (D-c, as R3's N34 does), "nothing to do";
//    * the device count (golden AnalysisProductionLog::GetDeviceCount, uAnalysisProductionLog.cpp:62-79 / :187-191)
//      steps one day from the interval's START time, so an interval whose end is earlier in its day than its start
//      misses the last day's file; W18 B reads every calendar day from the start's day to the end's
//      (Options::keepGoldenBugs = golden);
//    * N25-5 checks its copy (the same size as the source).  The source is golden's plain name of yesterday,
//      <EventLogTxt>\yyyy\mm\EventLogTxt_yyyymmdd.csv (uAnalysisEventLogText.cpp:27-41) -- a machine that names its
//      event log with O15 / N10 (EventLogTxt_<ID>_..., <Model>_<ID>_EventLogTxt_...) has no such file: nothing to do,
//      as in golden (asked, ledger W22);
//    * golden's SOFT_SIMULTE account override (:279-283) is not carried over (★W36 = C: the SIM build uploads like ship).
struct N25UploadSettings
{
    bool o10;                 // D-a: AutoJobsEnabled = EffectiveO10 (the timed run); a manual run passes true
    bool enable;              // N25-4 bN25_4_EnableUpload / N25-5 bN25_5_EnableUpload (config.ini [ChipMos Function])
    std::string custCode;     // Gerneral.ini [System] CUSTOMER_CODE
    std::string user, password, host;   // the N25-2 account (golden edtN25_2_Name / _Password / _Host)
    std::string path;         // golden edtN25_4_UploadPath / edtN25_5_UploadPath (sN25_4_UploadPath / sN25_5_UploadPath)
    N25UploadSettings() : o10(false), enable(false) {}
};
std::string N25_4GateReason(const N25UploadSettings& s);   // "" = the configuration allows the connection (D-f)
std::string N25_5GateReason(const N25UploadSettings& s);

// golden getColumn (Common.cpp:442-467): the colIndex-th comma field, '"' toggles quoting and is dropped; "" past the end
std::string GetColumn(const std::string& line, int colIndex);
// golden ParseDateTime(AnsiString) (Common.cpp:692-713): Trim; < 19 chars -> 0; "<date> <first 8 chars of the time>"
double ParseDateTimeText(const std::string& s);
// golden AnalysisProductionLog::GetDeviceCount (uAnalysisProductionLog.cpp:187-191 -> GetProInfoDevice :62-79,
// readCSVAndGetDeviceCount :101-121, AnalysisRowDataOnlyTime :172-185): rows (header skipped) of
// <prodRoot>\yyyymm\<fileTitle>_yyyymmdd.csv whose In Time (field 5, eLoadTime) is in [dtstart, dtend]
int ProductionDeviceCount(const std::string& prodRoot, const std::string& fileTitle, double dtstart, double dtend,
                          bool keepGoldenBugs);

struct JamInterval            // golden TimeIntervalRec (uChipMosZHUBEI_Func.h)
{
    double dtTimeS, dtTimeE;
    int iJamCount, iDeviceCount;
};
// golden SaveJamSummaryFor30Days (:122-132 -> :58-120): Now-31 00:00:00.000 .. Now-1 23:59:59.059, intervals split
// at every record whose Recovery holds "CLEAN_OUT", "Time,Summary,JamCount" rows (end time, devices, JAM records).
// false = not written (*events = 0: no record, D-c; else the file could not be written).
bool SaveJamSummaryFor30Days(const ElaUploadContext& c, const std::string& saveFolder, std::string* fileName,
                             std::vector<JamInterval>* intervals, int* events);
// golden SaveEventLog (:134-143): CopyFile(yesterday's EventLogTxt, <saveFolder>\EventLog.txt, overwrite), checked
bool SaveEventLog(const ElaUploadContext& c, const std::string& saveFolder, std::string* fileName, std::string* source,
                  bool* sourceMissing, std::string* detail);
// golden UploadSummaryCount (:352-370) / UploadEventLog (:372-390) behind the D-f gate
N25Result UploadSummaryCount(const ElaUploadContext& c, const N25UploadSettings& s);
N25Result UploadEventLog(const ElaUploadContext& c, const N25UploadSettings& s);

// ---- N10 BYFILE ----
struct N10Settings
{
    bool o10;
    int uploadMethod;         // golden rgN10_4 = iN10UploadMethod (0 FTP, 1 network drive)
    std::string user, password, host;   // golden edN10UserName / edN10Password / edN10Host (:148-150)
    N10Settings() : o10(false), uploadMethod(1) {}
};
std::string N10ByFileGateReason(const N10Settings& s);

struct N10Result
{
    std::string gate;
    bool listFound;           // UploadFile.csv exists
    int rows, uploaded, failed, missing, factoryCalls;
    bool logOnly;
    std::string detail;
    N10Result() : listFound(false), rows(0), uploaded(0), failed(0), missing(0), factoryCalls(0), logOnly(false) {}
};
// golden N10_UploadDataToFTP (:145-182): one row = one session; multi-level folder walk (EnsureDirPath); verified upload
bool N10_UploadDataToFTP(const ElaUploadContext& c, const N10Settings& s, const std::string& sourcesPath,
                         const std::string& targetPath, const std::string& sourcesFile, const std::string& targetFile,
                         N10Result* r);
// golden UploadByFile_N10 (:392-418): every row of <logRoot>\UploadFile\UploadFile.csv with 4 fields (header skipped;
// the file is not deleted -- golden :401 is commented out, so every run uploads every row again)
N10Result UploadByFile_N10(const ElaUploadContext& c, const N10Settings& s);

}  // namespace ela

#endif
