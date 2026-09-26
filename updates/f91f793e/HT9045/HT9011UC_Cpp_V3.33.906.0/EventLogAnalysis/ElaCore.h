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
//  PROVISIONAL (暫照 golden（A），待使用者確認 -- progress-st02 #15-#19, ledger §8), each behind an Options switch:
//    #15 ELA-1 A  rows are split with BCB6 CommaText (',' AND unquoted blanks), so an unquoted "16 System" field shifts
//                 the row -- exactly what the reference exe counts.  B (comma-only, quote-aware) = bcbCommaText=false.
//    #16 ELA-2 A  SPIL-format files are not analysed (every row fails the date check) -- no switch needed, golden shape.
//    #17 ELA-3 A  a missing JAM0000.dat key is WRITTEN with its default (golden CheckAndReadIniData).  jamWriteBack=false
//                 makes the look-ups read-only.  ctest always points jamIniPath into %TEMP%.
//    #18 ELA-4 A  golden leaks kept: _MySummaryStruct::InitData never clears iFailCount; ListByUnitName / ListByFunction
//                 are never cleared between queries.  keepGoldenLeaks=false fixes both.
//    #19 ELA-5 A  file selection as golden: every listed file whose path contains "<yyyymmdd>.csv" and "EventLogTxt_"
//                 (VTEST 915/919: "<yyyymmdd>" and "_EventLogTxt_") is read, no de-duplication.
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
    void InitData(bool keepGoldenLeak);   // golden Analyzer.cpp:111-126 does not clear iFailCount (#18)
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

struct Options
{
    bool bcbCommaText;      // #15, see the file head
    bool jamWriteBack;      // #17
    bool keepGoldenLeaks;   // #18
    std::string jamIniPath; // golden FileNameJam000 = D:\HT9045\Error\English\JAM0000.dat
    std::string custCode;   // golden sCustCode (Gerneral.ini [System] CUSTOMER_CODE); "915"/"919" = VTEST file names
    Options();
};

typedef std::vector<std::string> Row;

// ---- BCB6 RTL behaviour the analyzer depends on ----
Row BcbCommaText(const std::string& line);        // TStrings::SetCommaText (SetDelimitedText ',' '"', AnsiExtractQuotedStr)
Row CommaOnlyText(const std::string& line);       // #15 B: quote-aware, comma-only (no blank splitting)
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

    // golden GetEventLogText :782-1181 over the file list golden keeps in lstEventLog.
    void GetEventLogText(const std::vector<std::string>& lstEventLog);

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
    JamConfig jam;

    // ---- P1b (ElaTables.cpp).  Grids: element 0 = golden grid row 1 (the header row is not stored); a cell golden
    //      never wrote is "".  "No Record!!" lands in column 1 of the first row, as golden's Cells[1][1]. ----
    // golden ListProductionLog :2316-2466 over lstProdFile (it ends with UpdateSgProduction + SetTotalSummary, as golden).
    // false = golden threw out of it (EFOpenError / EStringListError / EConvertError on a LoadTime): the grids and the
    // summary are then NOT refreshed, like golden; the message is in exceptions.
    bool ListProductionLog(const std::vector<std::string>& lstProdFile);
    void UpdateSgProduction();                            // :1787-1947 -> sgByDay / sgByHour
    void SetTotalSummary();                               // :2468-2566 -> mmoSummary
    // :2061-2204.  cbTopAlarmFilter 0 = JAM, 1 = WAR, else JAM+WAR.  -> sgTop5Alarm (the event rows), sgTop5Filter
    //   ({"Top n", UnitName, AlarmCode, Message, Total Count, Total Stoped Time}, sorted by count desc, code asc).
    void UpdateSgTop5Filter(int cbTopAlarmFilter);
    void UpdateSgTop5(int ARow);                          // :2205-2233 -> sgTop5 ({Date, Time, UnitName, Message})
    void UpdateSgFailAndAlarm();                          // :1949-1993 -> sgAlarm / sgFail
    // :1995-2059.  rgFilter 0/1 = By Area (areaChecked[i] per ListByUnitName[i]), else By Function (funcChecked[i] per
    //   ListByFunction[i]).  NOTE golden's check-box -> list mapping is crossed (ctor :257-263): cb2DID drives efTemp,
    //   cbTemperature efESD, cbOCR ef2DID, cbESD efOCR -- a page must reproduce or fix that (#18).
    void UpdateSgByFilter(int rgFilter, const bool* areaChecked, const bool* funcChecked);
    // golden ReadConfig sets sCustCode (Gerneral.ini [System] CUSTOMER_CODE), which GetEventLogText's file rule reads
    void SetCustCode(const std::string& c) { opt_.custCode = c; }

    std::vector<std::string> slAllData;                   // golden slAllData (Production_Log rows, CommaText)
    std::vector<Row> sgByDay, sgByHour, sgTop5Alarm, sgTop5Filter, sgTop5, sgAlarm, sgFail, sgByArea;
    int iTop5FilterRowCount;                              // golden sgTop5Filter->RowCount (6 = five visible rows)
    int byAreaCount;                                      // golden labByAreaAlmCnt "Count: %d"
    std::vector<std::string> mmoSummary;                  // golden mmoSummary->Lines
    std::vector<std::string> exceptions;                  // golden mmoException->Lines
    std::string sHandlerID;                               // golden sHandlerID (summary header line)

private:
    Row Split(const std::string& line) const;
    void InitGridsForRange();                             // the SetStringGrid part that fills the grids (P1b)
    Options opt_;
    std::map<std::string, int> mapUnitName;
};

}  // namespace ela

#endif
