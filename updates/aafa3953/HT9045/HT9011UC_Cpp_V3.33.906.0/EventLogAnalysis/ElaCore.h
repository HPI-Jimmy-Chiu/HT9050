// ===========================================================================
//  EventLogAnalysis/ElaCore.h -- Event Log Analyzer core (ELA plan P1), no UI.
//  AI(W906-ELA-P1) 20260927.  Golden: EventlogAnalyzer Rev891.0 (SVN r891, D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code),
//  TfrmELA (Analyzer.cpp / Common.cpp).  Ledger: docs/ELA_PORT_LEDGER.md; skill: ht9045-eventlog-analyzer.
//
//  What is here (P1a): the multi-day statistics of TfrmELA::GetEventLogText (Analyzer.cpp:782-1181) and what it needs --
//  the query range set-up of SetStringGrid (:646-780, without the grids), the Jam ini look-ups GetJamLevel /
//  GetJemIncludeMTBA / GetJemIncludeMTBF (:2799-2882), the BCB6 TStrings CommaText read/write, TStrings::LoadFromFile
//  line splitting, and Common.cpp's date/time validators and parser (:620-800).  Not the Handler's single-file viewer
//  TfObserver::GetEventLogText (cObserver.cpp) -- same name, different function (skill §3).
//  P1b (ElaTables.cpp): the grid contents as tables of golden's cell text -- UpdateSgTop5Filter / UpdateSgTop5 /
//  UpdateSgFailAndAlarm / UpdateSgByFilter (:1949-2233), UpdateSgProduction (:1787-1947), SetTotalSummary (:2468-2566,
//  the mmoSummary lines), ListProductionLog (:2316-2466, Production_Log -> input / contact / test time), the recursive
//  *.csv scan LoadFavorite (:1644-1678), Common.cpp ConvertSecToTime / ConvertDTToTime / GetUPH / GetMUBA / GetMUBF /
//  GetMTBA / GetMTBF, and SysUtils StrToDateTime (zh-TW) for golden's implicit AnsiString -> TDateTime conversions.
//  Not ported: the ChangeLog label block (:1039-1162, UI labels only; never matches in practice, ledger §5).
//
//  Rulings (Steven 20260927; progress-st02 #15-#19, ledger ELA_PORT_LEDGER.md "W15 / W18 / W19"), each behind an Options
//  switch so that one Analyzer can still be the golden oracle for the reference exe (#23):
//    #15 W15 B    rows are split by ela::SplitEventLogCsv (EventLogCsv.h: commas only, a quoted field is one field -- the
//                 splitter the Handler's log viewer uses too, golden 912 cObserver.cpp:3812-3846, 912 only -- not in 906).  bcbCommaText=true =
//                 golden BCB6 CommaText (Analyzer.cpp:833: ',' AND unquoted blanks), which shifts an unquoted "16 System"
//                 row so that it is not counted -- what the reference exe counts.
//    #16 ELA-2 A  SPIL-format files are not analysed (every row fails the date check) -- no switch needed, golden shape.
//    #17 ELA-3 A  a missing JAM0000.dat key is WRITTEN with its default (golden CheckAndReadIniData).  jamWriteBack=false
//                 makes the look-ups read-only.  ctest always points jamIniPath into %TEMP%.
//    #18 W18 B    golden's obvious bugs are fixed and listed as deviations (ledger W18): iFailCount is reset per query
//                 (golden _MySummaryStruct::InitData :111-126 misses it) and ListByUnitName / ListByFunction are cleared
//                 per query (here); in ElaTables.cpp the by-hour test time, the single-record "No Record!!", the stale
//                 Top5Filter rows, the Top5 prefix match and the Production_Log abort.  keepGoldenBugs=true = golden.
//    #19 W19 B    files are chosen by the time their names cover (EventLogFileSpan: every save period the Handler writes
//                 -- day and _RT / _FT / _TesterOffline, hour "D HH", max-line "D HHNNSS" (between its neighbours of
//                 the same base name, the first / last open-ended), 12 h "DHH00", month, year; SelectEventLogFiles),
//                 each file is read once per query (main folders first, then AllEventLog, each by start), and a counted
//                 row already counted from ANOTHER file is skipped (dupRowsSkipped): the AllEventLog copy is no longer
//                 counted twice.  goldenFileRule=true = golden Analyzer.cpp:804-821: day by day, every file whose path
//                 holds "<yyyymmdd>.csv" and "EventLogTxt_" (VTEST 915/919: "<yyyymmdd>" and "_EventLogTxt_"), no de-dup.
//    ★W38 B      (Steven 20260928: customer-specified features keep golden's fixed formats): on a VTEST machine
//                 (custCode "915" / "919") that golden rule applies by default (Options::goldenFileRuleVtest,
//                 Analyzer::UsesGoldenFileRule) -- the MTBF report reads golden's fixed file names, no de-dup.
//    ★W45 18b     (AI(W906-ELA-W45) 20260928, St02-E helper; Steven 0928 W20-3 = B) a MISSING "<code> IncludeMTBF" key
//                 on a CC_ASE_CL 933 / CC_TERAPOWER 967 machine defaults to true -- the Handler's value (906_0625_Steven
//                 cSecurity.cpp:1338), so that both readers seed the same byte whichever comes first (JamRules.h).
//                 goldenJamMtbfDefault=true = golden Rev891 Analyzer.cpp:2846-2878 (the MTBF list for every customer).
//  R1 (AI(W906-ELA-R1) 20260927, St02-E; plan ela-reports-upload-plan.md §4): LogRecord + GetEventLogTextToVec x2
//  (Analyzer.cpp:1183-1267, the row list the reports read: N34 ChipAdvanced, N25 ChipMos) with the SAME Options as
//  GetEventLogText -- the W15 splitter, the W19 file choice and cross-file de-dup (vecDupRowsSkipped); goldenFileRule =
//  golden's day loop, which has NO VTEST branch in this function -- and GetStartEndGap (Common.cpp:511).  D-d (Steven
//  20260927: scan the folders again when a report is made): the caller passes a list scanned right before (LoadFavorite).
//  Dates are BCB TDateTime values: double days since 1899-12-30 (fraction = time of day).  Date strings use '/', the
//  DateSeparator of the machines' zh-TW locale (golden FormatString("YYYY/MM/DD")).
//  Standard C++ plus vclcompat's TIniFile (Win32 profile semantics) for JAM0000.dat.  Not thread-safe by itself: one
//  Analyzer per worker (plan: ElaThread).
// ===========================================================================
#ifndef HT9045_ELA_ELACORE_H
#define HT9045_ELA_ELACORE_H

#include <map>
#include <string>
#include <vector>

#include "EventLogAnalysis/EventLogCsv.h"   // ela::SplitEventLogCsv, the W15 B row splitter (header-only)

namespace ela {

// golden Analyzer.h:112-125
enum eEventLog
{
    elData = 0, elTime = 1, elUnit = 2, eAlarmCode = 3, elRecovery = 4, elStopTime = 5, elDuplicate = 6,
    elMessage = 7, elErrorPart = 8, elRecipe = 9, eEventLogTotal
};
// golden Analyzer.h:127-158 (e01InArm .. e30Fix6) and :160-170
enum { e07TesterIF = 6, e15Temp = 14, e20ESD = 19, eUnitNameTotal = 30 };
enum eByFunc { efRTC = 0, efTemp = 1, efESD = 2, ef2DID = 3, efOCR = 4, efAutoClean = 5, efYield = 6, eByFuncTotal };

// golden cbJamArea Items (Analyzer.dfm:1946-1977); the first eUnitNameTotal are the By-Area check boxes (ctor :265-278)
extern const char* const kJamAreaNames[31];

// golden Common.h:70-88 _MySummaryStruct
struct Summary
{
    std::string Date;
    int iInputCount, iStopCount, iAlarmCount, iFailCount, iJAMCount, iMESCount, iWARCount, iContactCount;
    double dtTestTime;
    int iTestTimeMS, iStopTime, iAlarmTime, iFailTime;
    Summary();
    void InitData(bool keepGoldenBug);    // golden Analyzer.cpp:111-126 does not clear iFailCount (#18: only when true)
};

// golden Analyzer.h:32-40 _MyJamSummary
struct JamSummary
{
    int iCount;
    std::string sUnitName, sAlarmCode, sMessage;
    int iStopTime;
    JamSummary();
    void InitData();
};

// golden Common.h:92-104 struct LogRecord (JimmyChiu 20250307, "Eventlog data to vector"): one event-log row as text.
// GetEventLogTextToVec fills date .. errorPart; recipe is never filled there (golden :1244-1254).
struct LogRecord
{
    std::string date, time, unitName, alarmCode, recovery, stoppedTime, duplicate, message, errorPart, recipe;
};

struct Options
{
    bool bcbCommaText;      // #15: false (default, W15 B) = SplitEventLogCsv; true = golden BCB6 CommaText
    bool jamWriteBack;      // #17
    bool keepGoldenBugs;    // #18: false (default, W18 B) = golden's obvious bugs fixed; true = golden's behaviour
    bool goldenFileRule;    // #19: false (default, W19 B) = file spans + row de-dup; true = golden's per-day name match
    // ★W38 B (Steven 20260928; AI(W906-ELA-W38) 20260928, St02-E helper): true (default) = on a VTEST machine (custCode
    //   "915" / "919") golden's file rule applies whatever goldenFileRule says (Analyzer::UsesGoldenFileRule); false =
    //   goldenFileRule alone (the W19 B rule on VTEST too: it also reads the plain day files, with the row de-dup)
    bool goldenFileRuleVtest;
    bool goldenJamMtbfDefault;  // ★W45 18b: false (default, W20-3 B) = 933 / 967 missing IncludeMTBF = true (the
                                //   Handler's); true = golden Rev891's list (AI(W906-ELA-W45) 20260928)
    std::string jamIniPath; // golden FileNameJam000 = D:\HT9045\Error\English\JAM0000.dat
    std::string custCode;   // golden sCustCode (Gerneral.ini [System] CUSTOMER_CODE); "915"/"919" = VTEST file names
                            //   (golden file rule only: goldenFileRule, or goldenFileRuleVtest on 915 / 919)
    Options();
};

typedef std::vector<std::string> Row;

// ---- BCB6 RTL behaviour the analyzer depends on ----
Row BcbCommaText(const std::string& line);        // TStrings::SetCommaText (SetDelimitedText ',' '"', AnsiExtractQuotedStr)
std::string BcbGetCommaText(const Row& row);      // TStrings::GetCommaText
std::vector<std::string> BcbLoadLines(const std::string& path, bool* ok);   // TStrings::LoadFromFile (CR / LF / CRLF)
bool FileExistsA(const std::string& path);

// ---- golden Common.cpp date helpers ----
double EncodeDate(int y, int m, int d);
double EncodeTime(int h, int n, int s, int ms);
// SysUtils.DateTimeToTimeStamp as ONE millisecond count: FISTP(DateTime * MSecsPerDay) -- the whole value is rounded to
// the millisecond first (x87 extended product, round half even), THEN split into date and time (div / mod).  Every
// golden FormatDateTime / DecodeDate / DecodeTime goes through it, so 46113.99999999999 is 2026/04/02 00:00:00.000.
long long DateTimeToMs(double dt);
void DecodeDateTime(double dt, int* y, int* m, int* d, int* h, int* n, int* s, int* ms);
std::string FormatYMD(double dt);                 // FormatString("YYYY/MM/DD")
std::string FormatYMDH00(double dt);              // FormatString("YYYY/MM/DD HH:00:00")
std::string FormatYYYYMMDD(double dt);            // FormatString("YYYYMMDD")
bool IsDigitString(const std::string& s);         // Common.cpp:620
bool IsValidDateString(const std::string& s);     // Common.cpp:630
bool IsValidTimeString(const std::string& s);     // Common.cpp:657
double ParseDateTime(const std::string& sDate, const std::string& sTime);   // Common.cpp:711 (0.0 = TDateTime() on failure)

// ---- #19 W19 B file selection (not golden; golden's rule = Options::goldenFileRule) ----
// true = an EventLogTxt file (the path holds golden's FN2 "EventLogTxt_" and the name ends in a save-period suffix +
// ".csv", any case) and [*from, *to) is the time its name covers (TDateTime); false = not an event-log file (ByLot,
// JamStat, RawData, a fixed name, .TXT ...).  Rows are still checked one by one against the query range.
// A max-line file ("<n>_yyyymmdd HHNNSS", HTSaveType TByMaxLineCount) holds ONE time and nothing about its end: *from =
// that time, *to = kEventLogSpanOpenEnd, *maxLineSeries = its series (the base name <n> in lower case, the AllEventLog
// copies apart; "" for every other file).  The Handler names it when it flushes (its rows are BEFORE that time,
// Public/MyStringList.cpp:431-436 -> :789), so SelectEventLogFiles gives it [previous file's time, next file's time)
// of its series -- the first open at the start, the last at the end: right whether the time is its end or its start.
const double kEventLogSpanOpenStart = -693594.0;  // before year 1 -- earlier than any time the Handler writes
const double kEventLogSpanOpenEnd = 2958466.0;    // just past Delphi MaxDateTime (9999-12-31 23:59:59.999)
bool EventLogFileSpan(const std::string& path, double* from, double* to, std::string* maxLineSeries = 0);
// W19 B: the files GetEventLogText reads for [dtStart, dtEnd], in reading order -- every event-log file whose span
// overlaps, once each; main folders first, then AllEventLog, each by span start (ties keep the list order).
std::vector<std::string> SelectEventLogFiles(const std::vector<std::string>& lstEventLog, double dtStart, double dtEnd);
bool IsAllEventLogPath(const std::string& path);  // a folder named AllEventLog (any case) -- MyStringList.cpp:586 copy

// ---- golden Common.cpp formatting and KPIs (P1b, ElaTables.cpp) ----
std::string ConvertSecToTime(long s, long iMS = -1);         // Common.cpp:379-404
std::string ConvertDTToTime(double DT, int iMS = -1);         // Common.cpp:407-432
// SysUtils.StrToDateTime with the machines' zh-TW settings (ShortDateFormat y/M/d, DateSeparator '/', TimeSeparator
// ':', DecimalSeparator '.'): "Y/M/D[ H:N[:S[.Z]]]" or a time "H:N[:S[.Z]]" alone.  false = golden EConvertError.
// Not supported (never written by the Handler): two-digit years, "M/D" without a year, AM/PM.
bool StrToDateTimeZhTw(const std::string& s, double* out);
struct Summary;
int GetUPH(const Summary& summary, int iDate);                // Common.cpp:518-528
int GetMUBA(const Summary& summary);                          // :530-540
int GetMUBF(const Summary& summary);                          // :542-552
double GetMTBA(const Summary& summary, double dtStartEndGap);   // :554-564
double GetMTBF(const Summary& summary, double dtStartEndGap);   // :566-576
double GetStartEndGap(double dtStart, double dtEnd);            // :511-516 (dtEnd - dtStart; R1)
std::vector<std::string> LoadFavorite(const std::string& dir);  // Analyzer.cpp:1644-1678 (recursive *.csv, hidden skipped)

// golden sgByDay / sgByHour columns (Analyzer.cpp:94-108) and Production_Log fields (Analyzer.h eMyProdRec)
enum { iSgDate = 0, iSgInputQty, iSgUPH, iSgContactCnt, iSgTotalTestTime, iSgAvgTestTime, iSgTotalStopTime,
       iSgAlarmCount, iSgSumAlarmTime, iSgMUBA, iSgMTBA, iSgFaliureCnt, iSgSumFailTime, iSgMTBF, iSgTotal };
enum { eLoadTime = 5, eOrderTest = 13, eTestTime = 27 };
extern const char* const kSgColName[9];     // golden sSgColName (Analyzer.cpp:69-77): the event-log grid headers

// ---- golden Analyzer.cpp:2799-2882 ----
class JamConfig
{
public:
    explicit JamConfig(const Options& o) : writes(0), opt_(o) {}
    int  GetJamLevel(const std::string& sJamArea, const std::string& sJamCode);
    bool GetJemIncludeMTBA(const std::string& sJamArea, const std::string& sJamCode);
    bool GetJemIncludeMTBF(const std::string& sJamArea, const std::string& sJamCode);
    int writes;   // keys written back (#17), for the ledger / tests
    // the machine's customer code (golden sCustCode): ★W45's IncludeMTBF default looks at 933 / 967 (JamRules.h).
    //   Analyzer::SetCustCode passes it on (Hub::ReadConfig, EL_UPDATE_PARAMETER -- which the hub runs before the boot
    //   query, ElaHub.cpp RunOnce: a raised EL_* flag wins over a pending query).  AI(W906-ELA-W45) 20260927
    void SetCustCode(const std::string& c) { opt_.custCode = c; }
private:
    bool CheckAndReadBool(const std::string& group, const std::string& name, bool value);
    int  CheckAndReadInt(const std::string& group, const std::string& name, int value);
    Options opt_;
};

// ---- golden TfrmELA state + SetStringGrid / GetEventLogText ----
class Analyzer
{
public:
    explicit Analyzer(const Options& o);

    // golden SetStringGrid :646-780 without the grids: dtStart / dtEnd / iDate (GetSysDateTimeRange :3228), the
    // by-day keys (from the start date, midnight) and by-hour keys (from the start date + time), cleared summary maps,
    // MySummary.InitData().  Arguments are the four date-time pickers (only the date / time part of each is used).
    void SetRange(double startDate, double startTime, double endDate, double endTime);

    // golden GetEventLogText :782-1181 over the file list golden keeps in lstEventLog (W19 B picks and orders the
    // files first and skips a row counted from another file -- see the file head).
    void GetEventLogText(const std::vector<std::string>& lstEventLog);

    // R1: golden GetEventLogTextToVec(logRecs) :1183-1187 -- GetSysDateTimeRange from the four pickers sets dtStart /
    // dtEnd / iDate (nothing else of SetRange), then the form below over [dtStart, dtEnd].
    void GetEventLogTextToVec(double startDate, double startTime, double endDate, double endTime,
                              const std::vector<std::string>& lstEventLog, std::vector<LogRecord>& logRecs);
    // R1: golden GetEventLogTextToVec(dtstart, dtend, idays, logRecs) :1189-1267: every row of the chosen files with a
    // valid date / time in [dtstart, dtend] and no "1" in Duplicate, in file order.  Clears the six sl*List (golden
    // :1198-1203) but no summary or map.  Same Options as GetEventLogText (file head); W19 B skips a row already taken
    // from ANOTHER file (vecDupRowsSkipped).  goldenFileRule: day by day over idays, "<yyyymmdd>.csv" + "EventLogTxt_".
    void GetEventLogTextToVec(double dtstart, double dtend, int idays, const std::vector<std::string>& lstEventLog,
                              std::vector<LogRecord>& logRecs);

    double dtStart, dtEnd;
    int iDate;
    Summary MySummary;
    std::vector<std::string> byDayKeys, byHourKeys;       // sgByDay / sgByHour row order
    std::map<std::string, Summary> mapByDayList, mapByHourList;
    std::map<std::string, JamSummary> mapAlarmSummary, mapJamSummary, mapWarSummary;
    std::vector<std::string> slStopList, slJamList, slJamWarList, slWarList, slAlarmList, slFailList;   // CommaText rows
    std::vector<std::string> ListByUnitName[eUnitNameTotal];
    std::vector<std::string> ListByFunction[eByFuncTotal];
    std::vector<std::string> filesRead;                   // golden StatusBar "Processing <file>"
    int dupRowsSkipped;                                   // W19 B: counted rows skipped as already counted from another file
    int vecDupRowsSkipped;                                // R1, W19 B: GetEventLogTextToVec rows skipped the same way
    JamConfig jam;

    // ---- P1b (ElaTables.cpp).  Grids: element 0 = golden grid row 1 (the header row is not stored); a cell golden
    //      never wrote is "".  "No Record!!" lands in column 1 of the first row, as golden's Cells[1][1]. ----
    // golden ListProductionLog :2316-2466 over lstProdFile (it ends with UpdateSgProduction + SetTotalSummary, as golden).
    // false = golden threw out of it (EFOpenError / EStringListError / EConvertError on a LoadTime): the grids and the
    // summary are then NOT refreshed, like golden; the message is in exceptions.  Only with keepGoldenBugs: W18 B skips
    // the unreadable / empty file or the bad line (one exceptions line per file), refreshes both, and returns true.
    bool ListProductionLog(const std::vector<std::string>& lstProdFile);
    void UpdateSgProduction();                            // :1787-1947 -> sgByDay / sgByHour
    void SetTotalSummary();                               // :2468-2566 -> mmoSummary
    // :2061-2204.  cbTopAlarmFilter 0 = JAM, 1 = WAR, else JAM+WAR.  -> sgTop5Alarm (the event rows), sgTop5Filter
    //   ({"Top n", UnitName, AlarmCode, Message, Total Count, Total Stoped Time}, sorted by count desc, code asc).
    //   W18 B: sgTop5Filter is rebuilt with the summary rows only; golden (keepGoldenBugs) keeps rows it does not
    //   rewrite and writes a "Top n" label per EVENT row.
    void UpdateSgTop5Filter(int cbTopAlarmFilter);
    void UpdateSgTop5(int ARow);                          // :2205-2233 -> sgTop5 ({Date, Time, UnitName, Message});
                                                          //   W18 B: AlarmCode equal (golden Pos()==1: a prefix)
    void UpdateSgFailAndAlarm();                          // :1949-1993 -> sgAlarm / sgFail
    // :1995-2059.  rgFilter 0/1 = By Area (areaChecked[i] per ListByUnitName[i]), else By Function (funcChecked[i] per
    //   ListByFunction[i]).  NOTE golden's check-box -> list mapping is crossed (ctor :257-263): cb2DID drives efTemp,
    //   cbTemperature efESD, cbOCR ef2DID, cbESD efOCR -- eventlog.html maps each box to its own list (W18 B).
    //   "No Record!!" only on an empty table (W18 B); golden (keepGoldenBugs) also writes it over a single record.
    void UpdateSgByFilter(int rgFilter, const bool* areaChecked, const bool* funcChecked);
    // golden ReadConfig sets sCustCode (Gerneral.ini [System] CUSTOMER_CODE), which GetEventLogText's file rule reads
    // (the golden rule only; ★W38 B: on 915 / 919 that rule is the default)
    void SetCustCode(const std::string& c) { opt_.custCode = c; jam.SetCustCode(c); }   // (W45: JamConfig too)
    const Options& options() const { return opt_; }       // (AI(W906-ELA-R2): ElaReports reads keepGoldenBugs)
    // ★W38 B (AI(W906-ELA-W38) 20260928): the file rule GetEventLogText / GetEventLogTextToVec use -- goldenFileRule,
    //   or goldenFileRuleVtest on a VTEST machine (custCode "915" / "919", golden's exact compare, Analyzer.cpp:807)
    bool UsesGoldenFileRule() const;

    std::vector<std::string> slAllData;                   // golden slAllData (Production_Log rows, CommaText)
    std::vector<Row> sgByDay, sgByHour, sgTop5Alarm, sgTop5Filter, sgTop5, sgAlarm, sgFail, sgByArea;
    int iTop5FilterRowCount;                              // golden sgTop5Filter->RowCount (6 = five visible rows)
    int byAreaCount;                                      // golden labByAreaAlmCnt "Count: %d"
    std::vector<std::string> mmoSummary;                  // golden mmoSummary->Lines
    std::vector<std::string> mmoSummary1;                 // golden mmoSummary1 (tab Sumary): SetTotalSummary copies
                                                          //   mmoSummary; SaveSummary adds its detail (ElaReports, R2)
    std::vector<std::string> exceptions;                  // golden mmoException->Lines
    std::string sHandlerID;                               // golden sHandlerID (summary header line)

private:
    Row Split(const std::string& line) const;
    void RangeFromPickers(double startDate, double startTime, double endDate, double endTime);   // GetSysDateTimeRange
    void InitGridsForRange();                             // the SetStringGrid part that fills the grids (P1b)
    Options opt_;
    std::map<std::string, int> mapUnitName;
};

}  // namespace ela

#endif
