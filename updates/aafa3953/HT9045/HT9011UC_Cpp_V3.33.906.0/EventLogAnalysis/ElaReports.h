// ===========================================================================
//  EventLogAnalysis/ElaReports.h -- the analyzer's reports (ELA reports plan R2 summaries, R3 N34 ChipAdvanced), no UI.
//  AI(W906-ELA-R2) / AI(W906-ELA-R3) 20260927 (St02-E).  Golden: EventlogAnalyzer Rev891.0 (SVN r891,
//  D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code) Analyzer.cpp: SaveSummary (:2614-2688), O06SaveSummaryData
//  (:3142-3152), N10SaveSummaryData (:3154-3180), DoVTestSaveSummary (:3240-3261), the save part of Timer2Timer
//  (:3010-3088).  Plan: D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md §4 R2.
//  Ledger: docs/ELA_PORT_LEDGER.md "R1 / R2", "R3".  R3 (uChipAdvancedFunc.cpp): see the R3 block at the end.
//
//  One file for three jobs (plan §1.1): [O06] O06-4 AutoSaveProductionPath, [N10] N10-3 (network drive or
//  D:\HT9045_Log\EventLogSummary\yyyymm), [O19] VTEST 915 / 919 EL_VTEST_MTBF_SUM (asO19_SavePath, no detail).
//  Each runs golden's btnQuery (ElaHub.h BtnQuery: the folders are scanned again -- golden ImportProductionLod(2), D-d)
//  on the ONE Analyzer the page's queries use (golden has one TfrmELA), then SaveSummary.  The job's query does NOT become
//  the page's snapshot (St02-E 20260927, a deviation: golden's window showed the job's range afterwards): the Hub keeps
//  each job's result as a job record (ElaHub.h JobRecord, SnapshotJson "jobs"; R6 shows them).
//
//  Rulings in here (Steven 20260927):
//    D-e  the report file is UTF-8 (golden: the ANSI memo text, cp950): every report line goes through ReportEncode() --
//         the ONE function that decides report bytes (valid UTF-8 kept, cp950 converted; no BOM), so a later cp950 /
//         switch ruling changes one line.  CRLF line ends as golden's TMemo text.  File NAMES stay the ANSI bytes.
//    W21 / §3.3  regenerated, never appended: SaveSummary creates the file (golden TFileStream fmCreate) -- same as golden.
//    D-a  O10 gates the automatic jobs: golden's analyzer only runs when O10 is on.  AutoJobsEnabled() is that gate (the
//         Hub's EL_VTEST_MTBF_SUM / CHIPADV_LOTEND / upload jobs, and R5's schedule); EffectiveO10 below is its one source.
//    W18 B  G3: O06-4 without the time-period option saved every minute (Timer2 `SystemNN % 1`) although golden's own
//         comment says "預設一小時一次" (W18 B: once an hour -- replaced by ★W43 below); and a second SaveSummary without
//         a new query no longer repeats the detail (golden appends to mmoSummary1 in place).  keepGoldenBugs = golden, both.
//    ★W43 C (Steven 20260928; AI(W906-ELA-W43) 20260928, St02-E helper): O06-4 follows the Handler's [O06-8] "Update
//         Production Record every 10 / 30 minutes" -- unchecked = no timed save at all, checked = every 10 / 30 min.
//  Not here: when to run (R5 ElaSchedule owns the 50 s timer, the minute lock, the stagger, boot catch-up) -- see
//  Timer2Due / Timer2Run, the R5 CALL POINT.  The manual "Save Summary" button (W22 later) is SaveSummary with a Name.
// ===========================================================================
#ifndef HT9045_ELA_ELAREPORTS_H
#define HT9045_ELA_ELAREPORTS_H

#include "EventLogAnalysis/ElaCore.h"
#include "EventLogAnalysis/ElaHub.h"   // ElaConfig, QueryRequest, BtnQuery, ToUtf8

#include <string>
#include <vector>

namespace ela {

// golden Now() (SysUtils: the local date + time, milliseconds included).  ctests pass a fake.
class Clock
{
public:
    virtual ~Clock() {}
    virtual double Now() const = 0;                 // TDateTime
};
class SystemClock : public Clock
{
public:
    double Now() const;                              // GetLocalTime -> EncodeDate + EncodeTime
};
const Clock& DefaultClock();

// golden's globals SystemYY / SystemMM / SystemDD: Timer2Timer and SaveSummary (a report with no Name) DecodeDate(Now())
// into them, and N10SaveSummaryData names its folder from whatever they hold then.
struct SysDate
{
    int YY, MM, DD;
    SysDate() : YY(0), MM(0), DD(0) {}
};

// D-a: golden's analyzer -- and so every automatic report / upload -- only exists when O10 is on (906_0625_Steven
// main.cpp:17848 / :17974 IniConfig.bO10UseEventLogSaver).  The page's own queries are not gated.
// AI(W906-ELA-R5) 20260927 (St02-E): the ONE O10 source for every automatic job (AutoJobsEnabled, ElaSchedule).
//   golden 906_0625_Steven cprod.cpp:2379-2394 ProcessLastSetIni_EventLog, after the Configuration page read the stored
//   key (cConfiguration.cpp:3978-3980; ReadLastSetIni :2966 before :3019):
//       if(IniConfig.bEventLogAutoSaveFunction) { ... IniConfig.bO10UseEventLogSaver = IniConfig.bO06_EventLogAutoSave; }
//   V906 applies the same override to IniConfig at cprod.cpp:2519.  So
//       effective O10 = bEventLogAutoSaveFunction ? [Event Log] EnableAutoSaveEventLog : the stored bO10UseEventLogSaver.
//   bEventLogAutoSaveFunction is a customer-table flag (CosFunction), in no ini.  InitialCosFunction sets it true for every
//   customer first (golden CosFunction.cpp:3894, V906 CosFunction.cpp:4111; CustomerFunctionSelect golden cprod.cpp:3654,
//   V906 :3901) and nothing sets it false in either tree today -- so O10 follows EnableAutoSaveEventLog for now.
// The Handler's live value wins: wb_serve installs a getter of IniConfig.bO10UseEventLogSaver (FileRW/TestIF_File_TesterIF.cpp,
// at static init); it is called at every decision (the Configuration page can change O10 while running), never cached.
// Without a getter (ctests, other hosts) the formula runs on the config.ini values with an injected function flag
// (default true = InitialCosFunction's value).  No customer table is copied here.
typedef bool (*EffectiveO10Getter)();
void SetEffectiveO10Getter(EffectiveO10Getter g);            // 0 = none
EffectiveO10Getter GetEffectiveO10Getter();
void SetO10FunctionFlagFallback(bool functionFlag);          // the formula's bEventLogAutoSaveFunction without a getter
bool O10FunctionFlagFallback();
bool EffectiveO10FromIni(bool functionFlag, bool enableAutoSaveEventLog, bool storedO10);   // the formula
bool EffectiveO10FromIni(bool functionFlag, const std::string& configIni);                 // both keys, read only
bool EffectiveO10(bool enableAutoSaveEventLog, bool storedO10);   // the getter (called now) if installed, else the formula
std::string EffectiveO10Source();                            // one phrase for why-texts / logs
bool AutoJobsEnabled(const ElaConfig& c);                    // = EffectiveO10(c.cbO06, c.o10Stored)

// golden SaveSummary(Path, Name="", bDetail=true) :2614-2688 on the Analyzer's last query.  The text = mmoSummary1
// (SetTotalSummary's copy of mmoSummary), then with bDetail "" and sgByHour (header row + every row, each cell + ","),
// then with includeAlarmList (golden chkIncludeAlarmList, unchecked in the dfm and never read from an ini) && bDetail
// "//===...", sgTop5Filter (RowCount rows), "", "//===...", sgTop5Alarm (RowCount rows), "".
// File: FixFolderPath(Path) + sHandlerID + "-SummaryData_" + (Name, or yyyy-mm-dd of clock.Now()) + ".txt".
// false = not written: IsValidFileName said no, or the file could not be created (golden EFCreateError); *error says
// which.  *fullName (may be null) = the name golden builds, written or not.  sys (may be null): the SystemYY.. globals.
bool SaveSummary(Analyzer& a, const std::string& Path, const std::string& Name, bool bDetail, bool includeAlarmList,
                 const Clock& clock, std::string* fullName, std::string* error, SysDate* sys = 0);
// The lines SaveSummary writes (before ReportEncode), and golden's mmoSummary1 update (keepGoldenBugs: appended in place).
std::vector<std::string> BuildSummaryLines(Analyzer& a, bool bDetail, bool includeAlarmList);
// D-e (Steven 20260927): the bytes of one report line -- the only place that decides the report encoding (UTF-8 now:
// ToUtf8, valid UTF-8 kept, cp950 converted).  Every report file (SaveSummary, R3's ClipAdv, later ones) uses it.
std::string ReportEncode(const std::string& line);
// SaveSummary's file bytes: each line ReportEncode()d + CRLF (golden TMemo text)
std::string SummaryBytes(const std::vector<std::string>& lines);

// what one job did, for the Hub's job history (and R5)
struct ReportResult
{
    bool ran;             // golden's conditions held: a query ran and a save was attempted
    bool queried;         // btnQuery ran: the Analyzer now holds that query (the Hub keeps its range in the job record;
                          //   the page's snapshot is left alone)
    bool prodOk;          // BtnQuery's ListProductionLog result; false = golden threw out of the query (keepGoldenBugs
                          //   only): no file -- golden's exception also left Timer2 / the analysis thread for good
    bool written;         // the report file was written
    std::string path;     // the file golden names ("" when no save was attempted)
    std::string note;     // one line: what happened
    bool hasRange;        // queried: [rangeFrom, rangeTo] = the job's query range (TDateTime)
    double rangeFrom, rangeTo;
    ReportResult() : ran(false), queried(false), prodOk(true), written(false), hasRange(false), rangeFrom(0), rangeTo(0) {}
};

// golden DoVTestSaveSummary :3240-3261 (EL_VTEST_MTBF_SUM, RogerYang 20251104 偉測MTBF文件生成): pickers = Now()-7
// 00:00:00 .. Now()-1 23:59:59, btnQuery, SaveSummary(asO19_SavePath, "", bDetail=false).  ui = the page state golden's
// btnQuery reads (folders, cbTopAlarmFilter, filters, sHandlerID, chkIncludeAlarmList); its dates are replaced.
// No MyForceDirectories here (golden): IsValidFileName makes the folder.
ReportResult DoVTestSaveSummary(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock);

// golden O06SaveSummaryData :3142-3152: MyForceDirectories(AutoSaveProductionPath) FIRST (even when O06-4 is off, as
// golden), then with chkO06Production: btnQuery on q (Timer2 set today's range) and SaveSummary(AutoSaveProductionPath).
ReportResult O06SaveSummaryData(Analyzer& a, const ElaConfig& c, const QueryRequest& q, const Clock& clock,
                                SysDate* sys);
// golden N10SaveSummaryData :3154-3180: iN10UploadMethod 1 (network drive) -> sN10UploadDrivePath; otherwise ("FTP",
// but golden only saves here) <HT9045_Log>\EventLogSummary\<SystemYY><SystemMM>\ (Ht9045LogRoot(), W906_HT9045LOG_ROOT);
// MyForceDirectories first, then with N10-3 (bN10_DailyUploadProdData): btnQuery on q and SaveSummary(that folder).
ReportResult N10SaveSummaryData(Analyzer& a, const ElaConfig& c, const QueryRequest& q, const Clock& clock,
                                SysDate* sys);

// ---- R5 CALL POINT (ElaSchedule; not wired in R2) ----
// golden Timer2Timer :3010-3088, split: R5 owns the 50 s tick, bFirstIn, the CurrentNN "once per minute" lock,
// ProcessHMountConnect (Handler window search -- nothing to port), the D-a gate and the stagger / catch-up.
// Timer2Due(c, SystemHH, SystemNN) = golden's bNeedUpload for a NEW minute: O06-4 (chkO06Production) every
// iCheckInterval minutes (EnanleTimePeriodSaveLog = [O06-8]: TimePeriodSaveLog 0 -> 10, else 30; otherwise never --
// ★W43 C; golden 1, G3, only with keepGoldenBugs), or N10-3 (cbN10_3) by iN10UploadProductMethod: 0 = 00:00, 1 = 08:00
// and 20:00, 2 = every hour, 3 (指定時間) = every hour too (rgN10_3_1 has three items: RadioIndex clamps 3 to 2).
bool Timer2Due(const ElaConfig& c, int hh, int nn, bool keepGoldenBugs);
// Timer2Run = golden :3071-3083: SystemYY/MM/DD = the tick's date, pickers = that day 00:00:00 .. 23:59:59, then
// O06SaveSummaryData and N10SaveSummaryData (both results out).
void Timer2Run(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock, ReportResult* o06,
               ReportResult* n10);

// ===========================================================================
//  R3: [N34] OEE And Failure Report (CC_CYUEAN 868) -- golden uChipAdvancedFunc.cpp / .h (Jimmychiu 20250119 / 20250324).
//  AI(W906-ELA-R3) 20260927 (St02-E).
//  EL_UPLOAD_CHIPADV_LOTEND: the Handler sends it at lot end with bN34_GenerateOEEAlarmRpt (golden 906_0625_Steven
//  uLotInfo.cpp:2327-2334; V906 forms/fLotInfo.cpp:7082-7095, St01's file, still #if 0).  The analyzer checks no switch
//  (golden AnalysisThreadProcess :173-178) -- here only D-a's O10 (the Hub job).
//  AnalysisLog: the rows of the last 8 days (GetEventLogTextToVec; D-d: the folder scanned first) -> the LAST
//  "Lot Start, Lot ID:" row -> btnQuery from that row to Now() (SetStringGrid + GetEventLogText + ListProductionLog) ->
//  the rows again -> per-code counts -> <sN34_OEEAlarmRptPath>\ClipAdv_<LotID>_<y><m><d>.csv (%d%d%d, not padded).
//  Rulings / deviations:
//    D-c  golden FindStartLotFromRecVec `for(size_t i=iTsize-1; i>0; i--)` wraps with no row and reads past the
//         vector (a crash): with no row there is no report (Steven 20260927).
//    W21 / §3.3 + D-b  the file is regenerated (WriteDataToFile bOverWrite); golden appends to it (OPEN_ALWAYS).
//    D-e  the report bytes go through ReportEncode (LF lines as golden).
//    W18 B  ParseField skips one character after "Lot ID:" (golden `pos+fieldLen+1`, written for "Lot ID: X"), but the
//         Handler writes "Lot ID:%s" (906_0625_Steven uLotInfo.cpp:1609): golden drops the first character of the Lot
//         ID / OP ID / Run Mode (a one-character Lot ID becomes "" -> file "ClipAdv_(null)_...").  Here the blank is
//         skipped only when there is one; keepGoldenBugs = golden.
//  Kept (golden): index 0 of the 8-day rows is never looked at for the Lot Start, the counts start at index 1 (that is
//  the Lot Start row itself in the second list); an empty Lot ID prints "(null)" in the file name (Borland RTL %s of a
//  NULL AnsiString -- the Handler's own logs show it, e.g. "(null)==>EQC"); "OEE" = (input - iFailCount) / input.
//  NOT verified: golden's "%0.2f%" / "%.2f%" end in a lone '%' (undefined in C); here the '%' is written, as intended.
// ===========================================================================
struct ChipAdvReport                  // golden ChipAdvancedFunc::reportStruct
{
    int iDate;
    double dtStart, dtEnd;
    Summary summary;
    std::string sLotID, sOPID, sRunMode;
    ChipAdvReport();
    void InitData();
};

struct AnomalyStats                   // golden ChipAdvancedFunc::AnomalyStats
{
    std::string sJamName;
    int count;                        // 異常次數
    int totalTime;                    // 異常總時間（秒）
};

// golden ChipAdvancedFunc members that use no state
std::string ChipAdvParseField(const std::string& source, const std::string& fieldName, bool keepGoldenBugs);   // :243-257
std::string ChipAdvHourMinSecStr(double dtTime);        // GetHourMinSecStr :193-201 ("%02d:%02d:%02.2f", hours over 24)
std::string ChipAdvDateTimeStr(double dtTime);          // GetDateTimeStr :203-206 ("yyyy/mm/dd hh:nn:ss")
double ChipAdvDaybySec(int iSec);                       // GetDaybySec :208-211
std::string ChipAdvPerformanceData(const std::string& sName, const std::string& sData1, const std::string& sData2);
std::string ChipAdvAnomalyStatsData(const std::string& sAlarmName, const std::string& sTotalAlarmCount,
                                    const std::string& sAlarmCountRatio, const std::string& sTotalAlarmTime,
                                    const std::string& sAlarmTimeRatio);

class ChipAdvancedFunc                // golden ChipAdvancedFunc (one per job, as golden's AnalysisThreadProcess)
{
public:
    ChipAdvReport rptStruct;
    std::vector<LogRecord> logRecords;
    std::vector<AnomalyStats> anomalyVec;
    std::string report;               // golden sRpt (before ReportEncode)

    // golden AnalysisLog :22-72 on the page's Analyzer (ui = the page state: folders, filters, sHandlerID).  The report
    // folder is c.edN34 (sN34_OEEAlarmRptPath); "" = not written (golden would make a "(null)" folder -- ReadConfig
    // never leaves it "").
    ReportResult AnalysisLog(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock);
    bool HadAnomaly(const std::string& sJamCode, int iStopTime);                          // :74-87
    bool FindStartLotFromRecVec(std::string& sDate, std::string& sTime, ChipAdvReport& rptstruct, bool keepGoldenBugs);
    std::string GetEventLogReport(const ChipAdvReport& rptstruct) const;                  // :89-98
    std::string GetPerformanceMetrics(const Summary& summary, double dtStartEndGap, int iDate) const;   // :100-124
    std::string GenerateOEEReport(const ChipAdvReport& rptstruct) const;                  // :126-152
    std::string GenerateAlarmReport(const ChipAdvReport& rptstruct) const;                // :154-175
    // SaveReport :232-241: EnsureDirectoriesExist, ClipAdv_<LotID>_<y><m><d>.csv, WriteDataToFile (regenerated here)
    bool SaveReport(const std::string& sRpt, ChipAdvReport& rptstruct, const std::string& sSaveFolder,
                    const Clock& clock, std::string* path);
};

}  // namespace ela

#endif
