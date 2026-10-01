// ===========================================================================
//  EventLogAnalysis/ElaReports.cpp -- see ElaReports.h.  AI(W906-ELA-R2) / AI(W906-ELA-R3) 20260927 (St02-E).
// ===========================================================================
#include "EventLogAnalysis/ElaReports.h"
#include "EventLogAnalysis/ElaFileUtil.h"

#include <windows.h>   // GetLocalTime (SystemClock)

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace ela {

// ---------------------------------------------------------------------------
//  Clock (golden Now())
// ---------------------------------------------------------------------------
double SystemClock::Now() const
{
    // SysUtils.Now: EncodeDate(wYear, wMonth, wDay) + EncodeTime(wHour, wMinute, wSecond, wMilliseconds)
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    volatile double v = EncodeDate(t.wYear, t.wMonth, t.wDay) + EncodeTime(t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    return v;
}

const Clock& DefaultClock()
{
    static SystemClock c;
    return c;
}

// ---------------------------------------------------------------------------
//  O10, the one source (ElaReports.h).  AI(W906-ELA-R5) 20260927 (St02-E).  Plain pointers / bools, constant-initialized,
//  so FileRW/TestIF_File_TesterIF.cpp may install the getter from its own static initializer.
// ---------------------------------------------------------------------------
static EffectiveO10Getter g_o10Getter = 0;
static bool g_o10FunctionFlag = true;            // InitialCosFunction (golden CosFunction.cpp:3894, V906 :4111)

void SetEffectiveO10Getter(EffectiveO10Getter g) { g_o10Getter = g; }
EffectiveO10Getter GetEffectiveO10Getter() { return g_o10Getter; }
void SetO10FunctionFlagFallback(bool functionFlag) { g_o10FunctionFlag = functionFlag; }
bool O10FunctionFlagFallback() { return g_o10FunctionFlag; }

bool EffectiveO10FromIni(bool functionFlag, bool enableAutoSaveEventLog, bool storedO10)
{
    return functionFlag ? enableAutoSaveEventLog : storedO10;   // golden cprod.cpp:2379-2394
}

bool EffectiveO10FromIni(bool functionFlag, const std::string& configIni)
{
    // read only: no write-back (golden's analyzer never reads bO10UseEventLogSaver; the Handler owns both keys)
    const bool o06 = IniCheckAndReadBool(configIni, "Event Log", "EnableAutoSaveEventLog", false, false, 0);
    const bool o10 = IniCheckAndReadBool(configIni, "Event Log", "bO10UseEventLogSaver", false, false, 0);
    return EffectiveO10FromIni(functionFlag, o06, o10);
}

bool EffectiveO10(bool enableAutoSaveEventLog, bool storedO10)
{
    const EffectiveO10Getter g = g_o10Getter;   // AI(W906-ELA-REV) 20260928: unlocked on purpose -- this may run on an HTTP thread (GET /api/ela/schedule) while the Handler's config load / save writes the one bool g() reads; a bool cannot tear, and a stale value only delays one status line or one job gate by a pass
    return g ? g() : EffectiveO10FromIni(g_o10FunctionFlag, enableAutoSaveEventLog, storedO10);
}

std::string EffectiveO10Source()
{
    if (g_o10Getter) return "the Handler's IniConfig.bO10UseEventLogSaver";
    return g_o10FunctionFlag ? "config.ini [Event Log] EnableAutoSaveEventLog (bEventLogAutoSaveFunction on)"
                             : "config.ini [Event Log] bO10UseEventLogSaver (bEventLogAutoSaveFunction off)";
}

bool AutoJobsEnabled(const ElaConfig& c)
{
    return EffectiveO10(c.cbO06, c.o10Stored);   // D-a
}

// golden TCustomRadioGroup.SetItemIndex clamps: < -1 -> -1, >= Items.Count -> Items.Count - 1 (ReadConfig assigns the
// ini value to the radio group, Timer2 / N10 read ItemIndex back).  rgN10_3_1 has 3 items (Analyzer.dfm:2765-2768:
// 00:00~24:00 / 08:00~20:00 / Per Hour) -- so the Handler's iN10UploadProductMethod 3 (指定時間) is Per Hour here;
// rgN10_4 has 2 (FTP / Net Drive, :2732-2734).
static int RadioIndex(int v, int count)
{
    if (v < -1) v = -1;
    if (v >= count) v = count - 1;
    return v;
}

// ---------------------------------------------------------------------------
//  SaveSummary (golden Analyzer.cpp:2614-2688)
// ---------------------------------------------------------------------------
static const char* const kByHourHead[iSgTotal] = {                 // golden InitStringGrid :623-643 (sgByHour row 0)
    "DateTime", "Input Qty", "UPH", "Contact Count", "Total Test Time", "Avg Test Time", "Total Stop Time",
    "Alarm Count", "Alarm Time", "MUBA", "MTBA", "Faliure Count", "Faliure Time", "MTBF" };
static const char* const kTop5FilterHead[6] = {                    // :586-590 (Cells[0][0] is never written)
    "", "UnitName", "AlarmCode", "Message", "Total Count", "Total Stoped Time" };
static const int kTop5AlarmCols = 9;                               // Analyzer.dfm:754 ColCount = 9; row 0 = sSgColName

static std::string CellsLine(const Row* r, int colCount)
{
    // golden `for(j<ColCount) Str=Str+Cells[j][i]+AnsiString(",")` -- a cell golden never wrote is ""
    std::string s;
    for (int j = 0; j < colCount; j++)
    {
        if (r && (size_t)j < r->size()) s += (*r)[(size_t)j];
        s += ",";
    }
    return s;
}

static std::string HeadLine(const char* const* head, int colCount)
{
    std::string s;
    for (int j = 0; j < colCount; j++)
    {
        s += head[j];
        s += ",";
    }
    return s;
}

std::vector<std::string> BuildSummaryLines(Analyzer& a, bool bDetail, bool includeAlarmList)
{
    // golden appends to mmoSummary1 in place, so a second save without a new query repeats the detail (keepGoldenBugs);
    // W18 B: every save starts from the query's summary (SetTotalSummary's mmoSummary = mmoSummary1 right after it)
    std::vector<std::string> L = a.options().keepGoldenBugs ? a.mmoSummary1 : a.mmoSummary;

    if (bDetail == true)                                            //RogerYang 20251104 : 偉測MTBF文件生成，這裡的資料不需要
    {
        L.push_back("");
        // sgByHour: RowCount = the by-hour keys + the header row, ColCount = iSgTotal (golden ctor :288)
        L.push_back(HeadLine(kByHourHead, iSgTotal));
        for (size_t i = 0; i < a.sgByHour.size(); i++)
            L.push_back(CellsLine(&a.sgByHour[i], iSgTotal));
    }

    if (includeAlarmList && bDetail == true)                        // golden chkIncludeAlarmList->Checked
    {
        L.push_back("//===================================");
        // sgTop5Filter: RowCount (6, or more rows when more summaries -- UpdateSgTop5Filter :2190-2193), ColCount 6
        L.push_back(HeadLine(kTop5FilterHead, 6));
        for (int i = 1; i < a.iTop5FilterRowCount; i++)
            L.push_back(CellsLine((size_t)(i - 1) < a.sgTop5Filter.size() ? &a.sgTop5Filter[(size_t)(i - 1)] : 0, 6));
        L.push_back("");

        L.push_back("//===================================");
        // sgTop5Alarm: RowCount = the list rows + 1, or 2 with "No Record!!" (:2092-2099), ColCount 9
        L.push_back(HeadLine(kSgColName, kTop5AlarmCols));
        const size_t rows = a.sgTop5Alarm.empty() ? 1 : a.sgTop5Alarm.size();
        for (size_t i = 0; i < rows; i++)
            L.push_back(CellsLine(i < a.sgTop5Alarm.size() ? &a.sgTop5Alarm[i] : 0, kTop5AlarmCols));
        L.push_back("");
    }
    a.mmoSummary1 = L;
    return L;
}

std::string ReportEncode(const std::string& line)
{
    // D-e (Steven 20260927): UTF-8, not golden's ANSI (cp950) text.  Line by line: each line comes from one source (the
    //   summary, one grid row, one report line), so a cp950 row and a UTF-8 row in one report both come out right.
    //   A later cp950 / switch ruling changes only this line.
    return ToUtf8(line);
}

std::string SummaryBytes(const std::vector<std::string>& lines)
{
    // TMemo text = every line + CRLF (TMemoStrings.Insert adds S + #13#10), which SaveToFile writes as it is
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        out += ReportEncode(lines[i]);
        out += "\r\n";
    }
    return out;
}

bool SaveSummary(Analyzer& a, const std::string& PathIn, const std::string& Name, bool bDetail, bool includeAlarmList,
                 const Clock& clock, std::string* fullName, std::string* error, SysDate* sys)
{
    const std::string Path = FixFolderPath(PathIn);

    std::string sFullName;
    if (Name == "")
    {
        int y, m, d, h, n, s, ms;
        DecodeDateTime(clock.Now(), &y, &m, &d, &h, &n, &s, &ms);   // DecodeDate(Now(), SystemYY, SystemMM, SystemDD)
        if (sys) { sys->YY = y; sys->MM = m; sys->DD = d; }
        char b[32];
        std::snprintf(b, sizeof(b), "%04d-%02d-%02d", y, m, d);
        sFullName = Path + a.sHandlerID + "-SummaryData_" + b + ".txt";
    }
    else
    {
        sFullName = Path + a.sHandlerID + "-SummaryData_" + Name + ".txt";
    }
    if (fullName) *fullName = sFullName;

    const std::vector<std::string> lines = BuildSummaryLines(a, bDetail, includeAlarmList);   // (golden: before the check)

    if (!IsValidFileName(sFullName))                                //RogerYang 20251104 : 偉測MTBF文件生成
    {
        if (error) *error = "not a valid file name (golden IsValidFileName)";
        return false;
    }
    return SaveBytesToFile(sFullName, SummaryBytes(lines), error);   // mmoSummary1->Lines->SaveToFile(sFullName)
}

// ---------------------------------------------------------------------------
//  The three jobs
// ---------------------------------------------------------------------------
static void DayStart(double day, double* date)
{
    // Str.sprintf("%04d/%02d/%02d 00:00:00", y, m, d) -> a picker (zh-TW StrToDateTime): the date at midnight
    int y, m, d, h, n, s, ms;
    DecodeDateTime(day, &y, &m, &d, &h, &n, &s, &ms);
    *date = EncodeDate(y, m, d);
}

static void SetPickers(QueryRequest* q, double startDay, double endDay)
{
    // dtpStartDate / dtpStartTime = "<start> 00:00:00", dtpEndDate / dtpEndTime = "<end> 23:59:59"; golden's btnQuery
    // -> GetEventLogText's tail runs UpdateSgTop5(1)
    double sd = 0.0, ed = 0.0;
    DayStart(startDay, &sd);
    DayStart(endDay, &ed);
    volatile double et = ed + EncodeTime(23, 59, 59, 0);
    q->startDate = sd;
    q->startTime = sd;
    q->endDate = ed;
    q->endTime = et;
    q->top5Row = 1;
}

static std::string Joined(const std::vector<std::string>& v)
{
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) { if (i) s += " / "; s += v[i]; }
    return s;
}

// btnQuery, then SaveSummary(folder, "", bDetail)
static void QueryAndSave(Analyzer& a, const QueryRequest& q, const std::string& folder, bool bDetail, const Clock& clock,
                         SysDate* sys, ReportResult* r)
{
    r->queried = true;
    r->prodOk = BtnQuery(a, q);                                     // btnQuery->Click()
    r->hasRange = true;
    r->rangeFrom = a.dtStart;
    r->rangeTo = a.dtEnd;
    if (!r->prodOk)
    {
        r->note = "the query stopped in Production_Log (" + Joined(a.exceptions) +
                  ") -- golden: that exception leaves the job, nothing is saved";
        return;
    }
    r->ran = true;
    std::string err;
    r->written = SaveSummary(a, folder, "", bDetail, q.includeAlarmList, clock, &r->path, &err, sys);
    r->note = r->written ? "wrote " + r->path : "not written: " + err + " (" + r->path + ")";
}

ReportResult DoVTestSaveSummary(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock)
{
    ReportResult r;
    const double now = clock.Now();

    //取得前七天的資料
    const double startDate = now - 7;
    const double endDate = now - 1;
    QueryRequest q = ui;
    SetPickers(&q, startDate, endDate);

    QueryAndSave(a, q, c.edtO19_6, false, clock, 0, &r);           // SaveSummary(edtO19_6->Text, "", false)
    return r;
}

ReportResult O06SaveSummaryData(Analyzer& a, const ElaConfig& c, const QueryRequest& q, const Clock& clock, SysDate* sys)
{
    ReportResult r;
    std::vector<std::string> exc;
    if (MyForceDirectories(c.edtO06Production, "O06SaveSummaryData", &exc) == 1)
    {
        if (c.chkO06Production)
            QueryAndSave(a, q, c.edtO06Production, true, clock, sys, &r);
        else
            r.note = "O06-4 (EnableAutoSaveProductiont) is off";
    }
    else
        r.note = "no folder " + c.edtO06Production + (exc.empty() ? std::string() : " (" + Joined(exc) + ")");
    a.exceptions.insert(a.exceptions.end(), exc.begin(), exc.end());   // golden mmoException
    return r;
}

ReportResult N10SaveSummaryData(Analyzer& a, const ElaConfig& c, const QueryRequest& q, const Clock& clock, SysDate* sys)
{
    ReportResult r;
    std::vector<std::string> exc;
    std::string Str;
    if (RadioIndex(c.rgN10_4, 2) == 1)                              //網路硬碟
        Str = c.edtN10_8;
    else                                                            //FTP (golden only saves here)
    {
        char b[16];
        std::snprintf(b, sizeof(b), "%04d%02d", sys ? sys->YY : 0, sys ? sys->MM : 0);
        Str = Ht9045LogRoot() + "\\EventLogSummary\\" + b + "\\";   // golden "D:\\HT9045_Log\\EventLogSummary\\%04d%02d\\"
    }
    if (MyForceDirectories(Str, "N10SaveSummaryData", &exc) == 1)
    {
        if (c.cbN10_3)
            QueryAndSave(a, q, Str, true, clock, sys, &r);
        else
            r.note = "N10-3 (bN10_DailyUploadProdData) is off";
    }
    else
        r.note = "no folder " + Str + (exc.empty() ? std::string() : " (" + Joined(exc) + ")");
    a.exceptions.insert(a.exceptions.end(), exc.begin(), exc.end());
    return r;
}

// ---------------------------------------------------------------------------
//  R5 CALL POINT: golden Timer2Timer (:3010-3088)
// ---------------------------------------------------------------------------
bool Timer2Due(const ElaConfig& c, int SystemHH, int SystemNN, bool keepGoldenBugs)
{
    // AI(W906-ELA-R2) 20260927 (St02-E), W18 B G3: golden `int iCheckInterval=1; //預設一小時一次` saves the whole-day
    //   summary every minute although the comment says once an hour (W18 B had made it 60).
    // AI(W906-ELA-W43) 20260928 (St02-E helper), ★W43 = C (Steven 20260928): the analyzer follows the Handler's [O06-8]
    //   (chkO06TimePeriod = [Event Log] EnanleTimePeriodSaveLog) -- unchecked = no O06-4 save at all (0), checked = every
    //   10 / 30 min as golden; keepGoldenBugs = golden's 1.  The Handler cites: ElaSchedule.cpp O06PeriodMin.
    int iCheckInterval = keepGoldenBugs ? 1 : 0;
    bool bNeedUpload = false;
    if (c.chkO06Production)
    {
        if (c.chkO06TimePeriod)
        {
            if (c.cbO06TimePeriod == 0)                             // TComboBox: a bad index reads back -1 -> 30
                iCheckInterval = 10;
            else
                iCheckInterval = 30;
        }
        if (iCheckInterval > 0 && SystemNN % iCheckInterval == 0)
            bNeedUpload = true;
    }
    if (c.cbN10_3)
    {
        const int method = RadioIndex(c.rgN10_3_1, 3);
        if (method == 0)
        {
            if (SystemHH == 0 && SystemNN == 0)
                bNeedUpload = true;
        }
        else if (method == 1)
        {
            if ((SystemHH == 8 || SystemHH == 20) && SystemNN == 0)
                bNeedUpload = true;
        }
        else if (method == 2)
        {
            if (SystemNN == 0)
                bNeedUpload = true;
        }
    }
    return bNeedUpload;
}

void Timer2Run(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock, ReportResult* o06,
               ReportResult* n10)
{
    SysDate sys;
    int y, m, d, h, n, s, ms;
    DecodeDateTime(clock.Now(), &y, &m, &d, &h, &n, &s, &ms);      // Timer2: DecodeDate(Now(), SystemYY, ...)
    sys.YY = y;
    sys.MM = m;
    sys.DD = d;
    QueryRequest q = ui;
    const double day = EncodeDate(sys.YY, sys.MM, sys.DD);
    SetPickers(&q, day, day);                                       // "%04d/%02d/%02d 00:00:00" .. " 23:59:59"
    const ReportResult r1 = O06SaveSummaryData(a, c, q, clock, &sys);
    const ReportResult r2 = N10SaveSummaryData(a, c, q, clock, &sys);   // SystemYY/MM as O06's SaveSummary left them
    if (o06) *o06 = r1;
    if (n10) *n10 = r2;
}

// ===========================================================================
//  R3: ChipAdvancedFunc (golden uChipAdvancedFunc.cpp).  AI(W906-ELA-R3) 20260927 (St02-E)
// ===========================================================================
ChipAdvReport::ChipAdvReport() : iDate(0), dtStart(0.0), dtEnd(0.0) {}

void ChipAdvReport::InitData()
{
    iDate = 0;
    dtStart = 0.0;
    dtEnd = 0.0;
    summary = Summary();
    sLotID = "";
    sOPID = "";
    sRunMode = "";
}

static std::string IntToStr(int v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}

// golden AnsiString().sprintf("%0.2f%", d) / ("%.2f%", d): the lone trailing '%' is written here (see ElaReports.h)
static std::string Percent2(double d)
{
    char b[64];
    std::snprintf(b, sizeof(b), "%.2f", d);
    return std::string(b) + "%";
}

std::string ChipAdvParseField(const std::string& source, const std::string& fieldName, bool keepGoldenBugs)
{
    const size_t p = source.find(fieldName);                       // int pos=source.Pos(fieldName)
    if (p == std::string::npos || fieldName.empty())
        return "";
    // golden SubString(pos + fieldLen + 1, ...): one character after the field name is skipped (keepGoldenBugs);
    //   W18 B: only a blank is skipped -- the Handler writes "Lot ID:%s" with none (uLotInfo.cpp:1609)
    size_t b = p + fieldName.size();
    if (keepGoldenBugs || (b < source.size() && source[b] == ' '))
        ++b;
    const std::string afterField = b < source.size() ? source.substr(b) : std::string();
    const size_t commaPos = afterField.find(',');
    std::string value = (commaPos != std::string::npos) ? afterField.substr(0, commaPos) : afterField;
    while (value.size() > 0 && value[0] == ';')
        value = value.substr(1);
    return value;
}

std::string ChipAdvHourMinSecStr(double dtTime)
{
    const int days = (int)std::floor(dtTime);
    int y, m, d, hour, minute, second, millisecond;
    DecodeDateTime(dtTime, &y, &m, &d, &hour, &minute, &second, &millisecond);   // DecodeTime
    const int totalHours = days * 24 + hour;
    const double seconds = second + millisecond / 1000.0;
    char b[64];
    std::snprintf(b, sizeof(b), "%02d:%02d:%02.2f", totalHours, minute, seconds);
    return b;
}

std::string ChipAdvDateTimeStr(double dtTime)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(dtTime, &y, &m, &d, &h, &n, &s, &ms);          // FormatString("yyyy/mm/dd hh:nn:ss"), zh-TW '/'
    char b[40];
    std::snprintf(b, sizeof(b), "%04d/%02d/%02d %02d:%02d:%02d", y, m, d, h, n, s);
    return b;
}

double ChipAdvDaybySec(int iSec)
{
    return (((double(iSec) / 24.0) / 60.0) / 60.0);
}

std::string ChipAdvPerformanceData(const std::string& sName, const std::string& sData1, const std::string& sData2)
{
    return sName + "," + sData1 + "," + sData2 + "\n";              // "%s,%s,%s%s" + FileInfo().GetNewLine()
}

std::string ChipAdvAnomalyStatsData(const std::string& sAlarmName, const std::string& sTotalAlarmCount,
                                    const std::string& sAlarmCountRatio, const std::string& sTotalAlarmTime,
                                    const std::string& sAlarmTimeRatio)
{
    return sAlarmName + "," + sTotalAlarmCount + "," + sAlarmCountRatio + "," + sTotalAlarmTime + "," + sAlarmTimeRatio + "\n";
}

bool ChipAdvancedFunc::HadAnomaly(const std::string& sJamCode, int iStopTime)
{
    for (size_t i = 0; i < anomalyVec.size(); i++)
    {
        AnomalyStats& aStatus = anomalyVec[i];
        if (aStatus.sJamName == sJamCode)
        {
            aStatus.count++;
            aStatus.totalTime += iStopTime;
            return true;
        }
    }
    return false;
}

bool ChipAdvancedFunc::FindStartLotFromRecVec(std::string& sDate, std::string& sTime, ChipAdvReport& rptstruct,
                                              bool keepGoldenBugs)
{
    const size_t iTsize = logRecords.size();
    if (iTsize == 0)
        return false;       // D-c (Steven 20260927): golden's `size_t i=iTsize-1` wraps and reads past the vector
    for (size_t i = iTsize - 1; i > 0; i--)                        // (golden: index 0 is never looked at)
    {
        const LogRecord& record = logRecords[i];
        if (record.message.find("Lot Start, Lot ID:") != std::string::npos)   // AnsiPos(...)>0
        {
            rptstruct.sLotID = ChipAdvParseField(record.message, "Lot ID:", keepGoldenBugs);
            rptstruct.sOPID = ChipAdvParseField(record.message, "OP ID:", keepGoldenBugs);
            rptstruct.sRunMode = ChipAdvParseField(record.message, "Run Mode:", keepGoldenBugs);
            sDate = record.date;
            sTime = record.time;
            return true;
        }
    }
    return false;
}

std::string ChipAdvancedFunc::GetPerformanceMetrics(const Summary& summary, double dtStartEndGap, int iDate) const
{
    std::string sret = "";
    sret += ChipAdvPerformanceData("Name", "Value", "Unit");
    const std::string sMTBA = ChipAdvHourMinSecStr(GetMTBA(summary, dtStartEndGap));
    sret += ChipAdvPerformanceData("MTBA", sMTBA, "[s]");
    const std::string sMUBA = IntToStr(GetMUBA(summary));
    sret += ChipAdvPerformanceData("MUBA", sMUBA, "[cmp]");
    const std::string sMTBF = ChipAdvHourMinSecStr(GetMTBF(summary, dtStartEndGap));
    sret += ChipAdvPerformanceData("MTBF", sMTBF, "[s]");
    const std::string sMUBF = IntToStr(GetMUBF(summary));
    sret += ChipAdvPerformanceData("MUBF", sMUBF, "[cmp]");
    const std::string sUPH = IntToStr(GetUPH(summary, iDate));
    sret += ChipAdvPerformanceData("UPH", sUPH, "[C/H]");
    sret += ChipAdvPerformanceData("UPH Auto", sUPH, "[C/H]");
    const std::string sProductionQuantity = IntToStr(summary.iInputCount);
    sret += ChipAdvPerformanceData("Production Quantity", sProductionQuantity, "[cmp]");
    return sret;
}

std::string ChipAdvancedFunc::GenerateOEEReport(const ChipAdvReport& rptstruct) const
{
    std::string sret = "";
    sret += ChipAdvPerformanceData("Name", "Quantity", "Time");
    sret += ChipAdvPerformanceData("Start Time", "", ChipAdvDateTimeStr(rptstruct.dtStart));
    sret += ChipAdvPerformanceData("Stop Time", "", ChipAdvDateTimeStr(rptstruct.dtEnd));
    sret += ChipAdvPerformanceData("Analysis Time", "", ChipAdvHourMinSecStr(rptstruct.dtEnd - rptstruct.dtStart));
    volatile double dtCurrent = (rptstruct.dtEnd - rptstruct.dtStart) - ChipAdvDaybySec(rptstruct.summary.iStopTime);   // TDateTime
    sret += ChipAdvPerformanceData("Production Time", "", ChipAdvHourMinSecStr(dtCurrent));
    sret += ChipAdvPerformanceData("Operation Time", "", ChipAdvHourMinSecStr(0));
    sret += ChipAdvPerformanceData("Alarm Time", "", ChipAdvHourMinSecStr(ChipAdvDaybySec(rptstruct.summary.iAlarmTime)));
    sret += ChipAdvPerformanceData("Prompt Time", "", ChipAdvHourMinSecStr(0));
    sret += ChipAdvPerformanceData("Downtime Time", "", ChipAdvHourMinSecStr(ChipAdvDaybySec(rptstruct.summary.iStopTime)));
    double dOEE = 0.0;
    if (rptstruct.summary.iInputCount == 0)
        dOEE = 0.0;
    else
        dOEE = ((double)rptstruct.summary.iInputCount - rptstruct.summary.iFailCount) / rptstruct.summary.iInputCount;
    dOEE *= 100;
    sret += ChipAdvPerformanceData("OEE", "", Percent2(dOEE));
    return sret;
}

std::string ChipAdvancedFunc::GenerateAlarmReport(const ChipAdvReport&) const
{
    std::string sret = "";
    int iSumCount = 0;
    int iSumTime = 0;
    for (size_t i = 0; i < anomalyVec.size(); i++)
    {
        iSumCount += anomalyVec[i].count;
        iSumTime += anomalyVec[i].totalTime;
    }
    sret += ChipAdvAnomalyStatsData("Alarm Name", "Total Alarm Count", "Alarm Count Ratio", "Total Alarm Time", "Alarm Time Ratio");
    double dAlarmCountRatio = 0.0, dAlarmTimeRatio = 0.0;
    for (size_t i = 0; i < anomalyVec.size(); i++)
    {
        const AnomalyStats& aStatus = anomalyVec[i];
        dAlarmCountRatio = (iSumCount == 0) ? 0.0 : ((double)aStatus.count / iSumCount) * 100;
        dAlarmTimeRatio = (iSumTime == 0) ? 0.0 : ((double)aStatus.totalTime / iSumTime) * 100;
        sret += ChipAdvAnomalyStatsData(aStatus.sJamName, IntToStr(aStatus.count), Percent2(dAlarmCountRatio),
                                        IntToStr(aStatus.totalTime), Percent2(dAlarmTimeRatio));
    }
    return sret;
}

std::string ChipAdvancedFunc::GetEventLogReport(const ChipAdvReport& rptstruct) const
{
    std::string srpt;
    srpt += GetPerformanceMetrics(rptstruct.summary, GetStartEndGap(rptstruct.dtStart, rptstruct.dtEnd), rptstruct.iDate);
    srpt += "\n";                                                   // FileInfo().GetNewLine()
    srpt += GenerateOEEReport(rptstruct);
    srpt += "\n";
    srpt += GenerateAlarmReport(rptstruct);
    return srpt;
}

// D-e for an LF-lined report: each line through ReportEncode
static std::string EncodeLfText(const std::string& text)
{
    std::string out;
    size_t b = 0;
    while (b <= text.size())
    {
        const size_t e = text.find('\n', b);
        if (e == std::string::npos) { out += ReportEncode(text.substr(b)); break; }
        out += ReportEncode(text.substr(b, e - b));
        out += "\n";
        b = e + 1;
    }
    return out;
}

bool ChipAdvancedFunc::SaveReport(const std::string& sRpt, ChipAdvReport& rptstruct, const std::string& sSaveFolder,
                                  const Clock& clock, std::string* path)
{
    EnsureDirectoriesExist(sSaveFolder);
    int year, month, day, h, n, s, ms;
    DecodeDateTime(clock.Now(), &year, &month, &day, &h, &n, &s, &ms);   // DecodeDate(Now(), year, month, day)
    char ymd[40];
    std::snprintf(ymd, sizeof(ymd), "%d%d%d", year, month, day);         // golden "%d%d%d": not zero-padded
    // golden sprintf("ClipAdv_%s_...", rptstruct.sLotID): an empty AnsiString is a NULL char* -> Borland "(null)"
    const std::string sFileName = "ClipAdv_" + (rptstruct.sLotID.empty() ? std::string("(null)") : rptstruct.sLotID) +
                                  "_" + ymd + ".csv";
    const std::string sFilePath = PathCombin(sSaveFolder, sFileName);
    if (path) *path = sFilePath;
    // W21 / plan §3.3 + D-b (Steven 20260927): regenerated -- golden WriteDataToFile(sFilePath, sRpt) appends
    return WriteDataToFile(sFilePath, EncodeLfText(sRpt), true);
}

ReportResult ChipAdvancedFunc::AnalysisLog(Analyzer& a, const ElaConfig& c, const QueryRequest& ui, const Clock& clock)
{
    ReportResult r;
    const bool golden = a.options().keepGoldenBugs;

    //Find a starting lot in 8 days
    rptStruct.InitData();
    rptStruct.dtStart = clock.Now() - 8;
    rptStruct.dtEnd = clock.Now();
    // pickers = DateOf / TimeOf of each; GetEventLogTextToVec(logRecords) reads them back (GetSysDateTimeRange).
    //   D-d (Steven 20260927): the folder is scanned again first (golden: the list of the last query)
    a.GetEventLogTextToVec(rptStruct.dtStart, rptStruct.dtStart, rptStruct.dtEnd, rptStruct.dtEnd,
                           LoadFavorite(ui.eventLogDir), logRecords);
    std::string sDate = "", sTime = "";
    if (FindStartLotFromRecVec(sDate, sTime, rptStruct, golden) == false)
    {
        r.note = logRecords.empty() ? "no event-log row in the last 8 days -- no report (D-c: golden crashed here)"
                                    : "no \"Lot Start, Lot ID:\" row in the last 8 days -- no report";
        return r;
    }
    rptStruct.dtStart = ParseDateTime(sDate, sTime);

    // frmELA->SetStringGrid(); GetEventLogText(); ListProductionLog(); -- btnQueryClick's three, on the lot's range
    QueryRequest q = ui;
    q.startDate = rptStruct.dtStart;
    q.startTime = rptStruct.dtStart;
    q.endDate = rptStruct.dtEnd;
    q.endTime = rptStruct.dtEnd;
    q.top5Row = 1;
    r.queried = true;
    r.prodOk = BtnQuery(a, q);
    r.hasRange = true;
    r.rangeFrom = a.dtStart;
    r.rangeTo = a.dtEnd;
    if (!r.prodOk)
    {
        r.note = "the query stopped in Production_Log -- golden: that exception leaves the job, no report";
        return r;
    }
    rptStruct.summary = a.MySummary;
    rptStruct.dtStart = a.dtStart;                                   // GetDateTime(pickers)
    rptStruct.dtEnd = a.dtEnd;
    rptStruct.iDate = (int)(rptStruct.dtEnd - rptStruct.dtStart + 1);   // golden: a TDateTime assigned to int (truncated)
    a.GetEventLogTextToVec(rptStruct.dtStart, rptStruct.dtStart, rptStruct.dtEnd, rptStruct.dtEnd,
                           LoadFavorite(ui.eventLogDir), logRecords);   // the list SetStringGrid scanned
    anomalyVec.clear();
    const size_t iTsize = logRecords.size();
    for (size_t i = 1; i < iTsize; i++)                              // (golden: from index 1)
    {
        const LogRecord& record = logRecords[i];
        const std::string& code = record.alarmCode;
        const int iStop = std::atoi(record.stoppedTime.c_str());
        if ((code.find("JAM") != std::string::npos || code.find("WAR") != std::string::npos ||
             code.find("MES") != std::string::npos) &&
            HadAnomaly(code, iStop) == false)
        {
            AnomalyStats aStatus;
            aStatus.sJamName = code;
            aStatus.count = 1;
            aStatus.totalTime = iStop;
            anomalyVec.push_back(aStatus);
        }
    }
    report = GetEventLogReport(rptStruct);
    r.ran = true;
    if (c.edN34.empty())
    {
        r.note = "no sN34_OEEAlarmRptPath -- not written (golden would write under a \"(null)\" folder)";
        return r;
    }
    r.written = SaveReport(report, rptStruct, c.edN34, clock, &r.path);
    r.note = (r.written ? "wrote " : "could not write ") + r.path + " (lot " + rptStruct.sLotID + " from " +
             ChipAdvDateTimeStr(rptStruct.dtStart) + ")";
    return r;
}

}  // namespace ela
