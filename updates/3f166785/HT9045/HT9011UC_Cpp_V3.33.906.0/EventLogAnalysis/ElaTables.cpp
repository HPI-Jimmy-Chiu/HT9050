// ===========================================================================
//  EventLogAnalysis/ElaTables.cpp -- ELA plan P1b: the analyzer's grids, summary, Production_Log and file scan as data.
//  AI(W906-ELA-P1) 20260927.  Golden EventlogAnalyzer Rev891.0 (Analyzer.cpp / Common.cpp); see ElaCore.h.
//  Golden quirks kept on purpose (ledger P1b): the by-hour test time multiplies dtTestTime by OneHourMS; ByFilter's
//  "No Record!!" overwrites column 1 of a single record; UpdateSgTop5Filter leaves the summary rows it does not rewrite
//  (a filter switch without a new query keeps old rows); UpdateSgTop5 matches the AlarmCode by prefix.
// ===========================================================================
#include "EventLogAnalysis/ElaCore.h"

#include <windows.h>   // FindFirstFileA (LoadFavorite)

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ela {

const char* const kSgColName[9] = { "Date", "Time", "UnitName", "AlarmCode", "Recovery", "StopedTime", "Duplicate",
                                    "Message", "ErrorPart" };

// golden Common.cpp:17-19 (long is 32 bits on the target, as in BCB)
static const long OneDaySec = 1 * 24 * 60 * 60;
static const long OneDayMS = 1 * 24 * 60 * 60 * 1000;
static const long OneHourMS = 1 * 60 * 60 * 1000;

static std::string IntStr(long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);
    return b;
}

static void SetCell(std::vector<Row>& grid, size_t row, size_t col, const std::string& v)
{
    if (grid.size() <= row) grid.resize(row + 1);
    if (grid[row].size() <= col) grid[row].resize(col + 1);
    grid[row][col] = v;
}

static std::string CellOf(const std::vector<Row>& grid, size_t row, size_t col)
{
    return (row < grid.size() && col < grid[row].size()) ? grid[row][col] : std::string();
}

// ===========================================================================
//  Common.cpp helpers
// ===========================================================================
std::string ConvertSecToTime(long s, long iMS)
{
    long secs, mins, hours, days;
    secs = (s) % 60L;
    mins = (s) / 60L;
    hours = mins / 60L;
    mins = mins % 60L;
    days = hours / 24L;
    hours = hours % 24L;
    char b[64];
    if (iMS == -1)
    {
        if (days <= 0)
            std::snprintf(b, sizeof(b), "%02d:%02d:%02d", int(hours), int(mins), int(secs));
        else
            std::snprintf(b, sizeof(b), "%d days %02d:%02d:%02d", (int)days, int(hours), int(mins), int(secs));
    }
    else
    {
        if (days <= 0)
            std::snprintf(b, sizeof(b), "%02d:%02d:%02d.%03d", int(hours), int(mins), int(secs), int(iMS));
        else
            std::snprintf(b, sizeof(b), "%d days %02d:%02d:%02d.%03d", (int)days, int(hours), int(mins), int(secs),
                          int(iMS));
    }
    return b;
}

std::string ConvertDTToTime(double DT, int iMS)
{
    const int iDay = int(DT);
    int y, m, d, h, n, s, ms;
    DecodeDateTime(DT, &y, &m, &d, &h, &n, &s, &ms);   // DT.FormatString("HH:NN:SS")
    char hms[16];
    std::snprintf(hms, sizeof(hms), "%02d:%02d:%02d", h, n, s);
    char b[64];
    if (iMS == -1)
    {
        if (iDay >= 1) std::snprintf(b, sizeof(b), "%d days %s", iDay, hms);
        else           std::snprintf(b, sizeof(b), "%s", hms);
    }
    else
    {
        if (iDay >= 1) std::snprintf(b, sizeof(b), "%d days %s.%03d", iDay, hms, iMS);
        else           std::snprintf(b, sizeof(b), "%s.%03d", hms, iMS);
    }
    return b;
}

int GetUPH(const Summary& summary, int iDate)
{
    int UPH = 0;
    if (summary.iInputCount == 0) UPH = 0;
    else if (iDate < 0)           UPH = 0;
    else                          UPH = summary.iInputCount / (iDate * 24);
    return UPH;
}

int GetMUBA(const Summary& summary)
{
    int iMS = 0;
    if (summary.iInputCount == 0)      iMS = 0;
    else if (summary.iAlarmCount == 0) iMS = summary.iInputCount;
    else                               iMS = summary.iInputCount / summary.iAlarmCount;
    return iMS;
}

int GetMUBF(const Summary& summary)
{
    int iMS = 0;
    if (summary.iInputCount == 0)     iMS = 0;
    else if (summary.iFailCount == 0) iMS = summary.iInputCount;
    else                              iMS = summary.iInputCount / summary.iFailCount;
    return iMS;
}

double GetMTBA(const Summary& summary, double dtStartEndGap)
{
    double dret = 0;
    if (summary.iInputCount == 0)      dret = 0.0;
    else if (summary.iAlarmCount == 0) dret = dtStartEndGap;
    else                               dret = dtStartEndGap / summary.iAlarmCount;
    return dret;
}

double GetMTBF(const Summary& summary, double dtStartEndGap)
{
    double dret = 0;
    if (summary.iInputCount == 0)     dret = 0.0;                              //Steven 20240827 : Fixed for display error
    else if (summary.iFailCount == 0) dret = dtStartEndGap;
    else                              dret = dtStartEndGap / summary.iFailCount;
    return dret;
}

// ---------------------------------------------------------------------------
//  SysUtils.StrToDateTime (Delphi 6 ScanDate / ScanTime), zh-TW: DateOrder YMD, '/', ':', '.'
// ---------------------------------------------------------------------------
static void ScanBlanks(const std::string& s, size_t* pos)
{
    while (*pos < s.size() && s[*pos] == ' ') ++*pos;
}

static bool ScanNumber(const std::string& s, size_t* pos, int* number, int* charCount)
{
    ScanBlanks(s, pos);
    size_t i = *pos;
    int n = 0;
    while (i < s.size() && s[i] >= '0' && s[i] <= '9' && n < 1000)   // golden `(N < 1000)`
    {
        n = n * 10 + (s[i] - '0');
        ++i;
    }
    if (i > *pos)
    {
        *charCount = (int)(i - *pos);
        *pos = i;
        *number = n;
        return true;
    }
    return false;
}

static bool ScanChar(const std::string& s, size_t* pos, char ch)
{
    ScanBlanks(s, pos);
    if (*pos < s.size() && s[*pos] == ch)
    {
        ++*pos;
        return true;
    }
    return false;
}

static int DaysIn(int y, int m)
{
    static const int k[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    return (m == 2 && leap) ? 29 : k[m - 1];
}

static bool ScanDate(const std::string& s, size_t* pos, double* date)
{
    int n1, n2, n3, l1, l2, l3;
    if (!(ScanNumber(s, pos, &n1, &l1) && ScanChar(s, pos, '/') && ScanNumber(s, pos, &n2, &l2)))
        return false;
    if (!ScanChar(s, pos, '/'))
        return false;                               // "M/D" (current year) is not supported -- see ElaCore.h
    if (!ScanNumber(s, pos, &n3, &l3))
        return false;
    if (l1 <= 2)
        return false;                               // two-digit year: not supported
    ScanChar(s, pos, '/');
    ScanBlanks(s, pos);
    const int y = n1, m = n2, d = n3;
    if (y < 1 || y > 9999 || m < 1 || m > 12 || d < 1 || d > DaysIn(y, m))
        return false;                               // TryEncodeDate
    *date = EncodeDate(y, m, d);
    return true;
}

static bool ScanTime(const std::string& s, size_t* pos, double* t)
{
    int hour, mn = 0, sec = 0, msec = 0, l;
    if (!ScanNumber(s, pos, &hour, &l))
        return false;
    if (ScanChar(s, pos, ':'))
        if (!ScanNumber(s, pos, &mn, &l)) return false;
    if (ScanChar(s, pos, ':'))
        if (!ScanNumber(s, pos, &sec, &l)) return false;
    if (ScanChar(s, pos, '.'))
        if (!ScanNumber(s, pos, &msec, &l)) return false;   // the digits as an integer: ".5" is 5 ms
    ScanBlanks(s, pos);
    if (hour >= 24 || mn >= 60 || sec >= 60 || msec >= 1000)
        return false;                               // TryEncodeTime
    *t = EncodeTime(hour, mn, sec, msec);
    return true;
}

bool StrToDateTimeZhTw(const std::string& s, double* out)
{
    size_t pos = 0;
    double date = 0.0, t = 0.0;
    if (ScanDate(s, &pos, &date) && (pos >= s.size() || ScanTime(s, &pos, &t)))
    {
        *out = (date >= 0) ? date + t : date - t;
        return true;
    }
    pos = 0;                                        // try a time alone
    if (!ScanTime(s, &pos, &t) || pos < s.size())
        return false;
    *out = t;
    return true;
}
// (the results above are stored through the out-pointer, i.e. to memory: a double, as golden's TDateTime Result)

// ---------------------------------------------------------------------------
//  LoadFavorite (Analyzer.cpp:1644-1678)
// ---------------------------------------------------------------------------
static std::string LowerAscii(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

static std::string ExtractFileExt(const std::string& name)
{
    const size_t i = name.find_last_of(".\\:");
    return (i != std::string::npos && name[i] == '.') ? name.substr(i) : std::string();
}

static void LoadFavoriteInto(const std::string& dir, std::vector<std::string>* list)
{
    std::string szDir = dir;
    if (szDir.empty() || szDir[szDir.size() - 1] != '\\') szDir += '\\';   // IncludeTrailingPathDelimiter
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((szDir + "*.*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return;
    do
    {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0 || std::strcmp(fd.cFileName, ".") == 0 ||
            std::strcmp(fd.cFileName, "..") == 0)
            continue;
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            LoadFavoriteInto(szDir + fd.cFileName, list);
        else if (LowerAscii(ExtractFileExt(fd.cFileName)) == ".csv")
            list->push_back(szDir + fd.cFileName);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

std::vector<std::string> LoadFavorite(const std::string& dir)
{
    std::vector<std::string> list;
    LoadFavoriteInto(dir, &list);
    return list;
}

// ===========================================================================
//  Analyzer grids
// ===========================================================================
static Row ZeroProductionRow(const std::string& key)
{
    // golden SetStringGrid :712-727 / :754-769
    Row r(iSgTotal);
    r[iSgDate] = key;
    r[iSgInputQty] = "0";
    r[iSgUPH] = "0";
    r[iSgContactCnt] = "0";
    r[iSgTotalTestTime] = "00:00:00";
    r[iSgAvgTestTime] = "00:00:00";
    r[iSgTotalStopTime] = "00:00:00";
    r[iSgAlarmCount] = "0";
    r[iSgSumAlarmTime] = "00:00:00";
    r[iSgMUBA] = "0";
    r[iSgMTBA] = "00:00:00";
    r[iSgFaliureCnt] = "0";
    r[iSgSumFailTime] = "00:00:00";
    r[iSgMTBF] = "00:00:00";
    return r;
}

void Analyzer::InitGridsForRange()
{
    // golden SetStringGrid :660-684 and the by-day / by-hour zero fill
    sgTop5Alarm.clear();
    sgTop5Filter.clear();
    iTop5FilterRowCount = 6;
    sgAlarm.clear();
    sgFail.clear();
    sgByDay.clear();
    for (size_t i = 0; i < byDayKeys.size(); ++i) sgByDay.push_back(ZeroProductionRow(byDayKeys[i]));
    sgByHour.clear();
    for (size_t i = 0; i < byHourKeys.size(); ++i) sgByHour.push_back(ZeroProductionRow(byHourKeys[i]));
}

void Analyzer::UpdateSgFailAndAlarm()
{
    sgAlarm.clear();
    if (slAlarmList.empty())
        SetCell(sgAlarm, 0, 1, "No Record!!");
    else
        for (size_t i = 0; i < slAlarmList.size(); ++i) sgAlarm.push_back(BcbCommaText(slAlarmList[i]));
    sgFail.clear();
    if (slFailList.empty())
        SetCell(sgFail, 0, 1, "No Record!!");
    else
        for (size_t i = 0; i < slFailList.size(); ++i) sgFail.push_back(BcbCommaText(slFailList[i]));
}

void Analyzer::UpdateSgByFilter(int rgFilter, const bool* areaChecked, const bool* funcChecked)
{
    sgByArea.clear();
    int iRow = 1;
    if (rgFilter == 0 || rgFilter == 1)
    {
        for (int i = 0; i < eUnitNameTotal; i++)
            if (areaChecked && areaChecked[i] && !ListByUnitName[i].empty())
                for (size_t j = 0; j < ListByUnitName[i].size(); ++j, ++iRow)
                    sgByArea.push_back(BcbCommaText(ListByUnitName[i][j]));
    }
    else
    {
        for (int i = 0; i < eByFuncTotal; i++)
            if (funcChecked && funcChecked[i] && !ListByFunction[i].empty())
                for (size_t j = 0; j < ListByFunction[i].size(); ++j, ++iRow)
                    sgByArea.push_back(BcbCommaText(ListByFunction[i][j]));
    }
    if (sgByArea.size() <= 1)                       // golden `if(sgByArea->RowCount==2)` -- also true for ONE record
        SetCell(sgByArea, 0, 1, "No Record!!");
    byAreaCount = iRow - 1;
}

struct CmpJam
{
    // golden `struct cmp` (Analyzer.cpp:31-39): count desc, then AlarmCode asc
    bool operator()(const JamSummary& a, const JamSummary& b) const
    {
        if (a.iCount != b.iCount) return a.iCount > b.iCount;
        return a.sAlarmCode < b.sAlarmCode;
    }
};

void Analyzer::UpdateSgTop5Filter(int cbTopAlarmFilter)
{
    for (int i = 1; i <= 10; ++i)
        SetCell(sgTop5Filter, i - 1, 0, "Top " + IntStr(i));
    sgTop5Alarm.clear();

    const std::map<std::string, JamSummary>& src =
        (cbTopAlarmFilter == 0) ? mapJamSummary : (cbTopAlarmFilter == 1) ? mapWarSummary : mapAlarmSummary;
    const std::vector<std::string>& list =
        (cbTopAlarmFilter == 0) ? slJamList : (cbTopAlarmFilter == 1) ? slWarList : slJamWarList;

    std::vector<JamSummary> vJam;
    for (std::map<std::string, JamSummary>::const_iterator it = src.begin(); it != src.end(); ++it)
        vJam.push_back(it->second);
    std::sort(vJam.begin(), vJam.end(), CmpJam());

    if (list.empty())
        SetCell(sgTop5Alarm, 0, 1, "No Record!!");
    else
        for (size_t i = 0; i < list.size(); ++i)
        {
            SetCell(sgTop5Filter, i, 0, "Top " + IntStr((long)i + 1));
            sgTop5Alarm.push_back(BcbCommaText(list[i]));
        }

    if (vJam.empty())
        SetCell(sgTop5Alarm, 0, 1, "No Record!!");
    else
        for (size_t i = 0; i < vJam.size(); ++i)
        {
            const int iStep = (int)i + 1;
            if (iStep > 5)
                iTop5FilterRowCount = iStep + 1;
            SetCell(sgTop5Filter, i, 1, vJam[i].sUnitName);
            SetCell(sgTop5Filter, i, 2, vJam[i].sAlarmCode);
            SetCell(sgTop5Filter, i, 3, vJam[i].sMessage);
            SetCell(sgTop5Filter, i, 4, IntStr(vJam[i].iCount));
            SetCell(sgTop5Filter, i, 5, IntStr(vJam[i].iStopTime));
        }
}

void Analyzer::UpdateSgTop5(int ARow)
{
    sgTop5.clear();
    const std::string strJam = CellOf(sgTop5Filter, (size_t)(ARow - 1), 2);
    if (strJam.compare(0, 3, "JAM") == 0 || strJam.compare(0, 3, "WAR") == 0)
    {
        for (size_t i = 0; i < sgTop5Alarm.size(); ++i)
        {
            if (CellOf(sgTop5Alarm, i, 3).compare(0, strJam.size(), strJam) == 0)   // Pos(strJam)==1: a prefix match
            {
                Row r(4);
                r[0] = CellOf(sgTop5Alarm, i, 0);
                r[1] = CellOf(sgTop5Alarm, i, 1);
                r[2] = CellOf(sgTop5Alarm, i, 2);
                r[3] = CellOf(sgTop5Alarm, i, 7);
                sgTop5.push_back(r);
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  Production_Log (Analyzer.cpp:2316-2466) and the production grids (:1787-1947)
// ---------------------------------------------------------------------------
static std::string ReplaceAll(std::string s, char from, char to)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] == from) s[i] = to;
    return s;
}

static std::string SubStr(const std::string& s, int index, int count)
{
    // AnsiString::SubString (System::Copy): 1-based, index < 1 -> 1, count < 0 -> 0
    if (index < 1) index = 1;
    if (count <= 0 || (size_t)index > s.size()) return std::string();
    return s.substr((size_t)index - 1, (size_t)count);
}

static int AnsiPos(const std::string& s, const std::string& t)
{
    const size_t p = s.find(t);
    return (t.empty() || p == std::string::npos) ? 0 : (int)p + 1;
}

bool Analyzer::ListProductionLog(const std::vector<std::string>& lstProdFile)
{
    int iOrderTest = -1, iCurrOrderTest = -1;
    slAllData.clear();                              // cbbProdCate / cbbStartTime: UI only
    for (int d = 0; d < iDate; d++)
    {
        const std::string FileName = FormatYYYYMMDD(dtStart + d) + ".csv";
        for (size_t i = 0; i < lstProdFile.size(); i++)
        {
            const std::string& FN = lstProdFile[i];
            if (FN.find(FileName) == std::string::npos)
                continue;
            bool ok = false;
            const std::vector<std::string> slFile = BcbLoadLines(FN, &ok);
            if (!ok)
            {
                exceptions.push_back("Cannot open file \"" + FN + "\"");   // golden EFOpenError leaves the function
                return false;
            }
            if (MySummary.iInputCount == 0)
            {
                if (slFile.empty())
                {
                    exceptions.push_back("List index out of bounds (0)");   // golden slFile->Strings[0]
                    return false;
                }
                const std::string Str = ReplaceAll(slFile[0], ' ', '_');
                slAllData.push_back(Str);
                MySummary.iInputCount++;
            }
            for (size_t j = 1; j < slFile.size(); j++)
            {
                std::string Str = slFile[j];
                if (Str.find('"') == std::string::npos)
                    Str = ReplaceAll(Str, ' ', '_');
                const Row slLine = BcbCommaText(Str);
                if ((int)slLine.size() <= eLoadTime)
                {
                    exceptions.push_back("List index out of bounds (" + IntStr(eLoadTime) + ")");
                    return false;
                }
                double dtCurrent = 0.0;
                if (!StrToDateTimeZhTw(ReplaceAll(slLine[eLoadTime], '_', ' '), &dtCurrent))
                {
                    exceptions.push_back("'" + ReplaceAll(slLine[eLoadTime], '_', ' ') + "' is not a valid date and time");
                    return false;                   // golden `dtCurrent=Str;` is outside the try
                }
                if ((int)slLine.size() > eOrderTest)
                    iCurrOrderTest = std::atoi(slLine[eOrderTest].c_str());
                else
                    iCurrOrderTest = iOrderTest;

                if (dtCurrent > dtStart && dtCurrent < dtEnd)
                {
                    std::map<std::string, Summary>::iterator IterByDay = mapByDayList.find(FormatYMD(dtCurrent));
                    if (IterByDay != mapByDayList.end())
                        IterByDay->second.iInputCount = IterByDay->second.iInputCount + 1;
                    std::map<std::string, Summary>::iterator IterByHour = mapByHourList.find(FormatYMDH00(dtCurrent));
                    if (IterByHour != mapByHourList.end())
                        IterByHour->second.iInputCount = IterByHour->second.iInputCount + 1;

                    if (iOrderTest != iCurrOrderTest)
                    {
                        iOrderTest = iCurrOrderTest;
                        if ((int)slLine.size() > eTestTime)
                        {
                            const std::string& tt = slLine[eTestTime];
                            const std::string sMS = SubStr(tt, AnsiPos(tt, ".") + 1, 3);
                            const std::string s2 = (tt != "") ? SubStr(tt, 1, AnsiPos(tt, ".") - 1) : std::string("00:00");
                            double dtCurrTestTime = 0.0;
                            if (StrToDateTimeZhTw(s2, &dtCurrTestTime))
                            {
                                const int iMS = std::atoi(sMS.c_str());
                                MySummary.iTestTimeMS = MySummary.iTestTimeMS + iMS;
                                MySummary.dtTestTime = MySummary.dtTestTime + dtCurrTestTime;
                                MySummary.iContactCount++;
                                if (IterByDay != mapByDayList.end())
                                {
                                    IterByDay->second.iTestTimeMS = IterByDay->second.iTestTimeMS + iMS;
                                    IterByDay->second.dtTestTime = IterByDay->second.dtTestTime + dtCurrTestTime;
                                    IterByDay->second.iContactCount = IterByDay->second.iContactCount + 1;
                                }
                                if (IterByHour != mapByHourList.end())
                                {
                                    IterByHour->second.iTestTimeMS = IterByHour->second.iTestTimeMS + iMS;
                                    IterByHour->second.dtTestTime = IterByHour->second.dtTestTime + dtCurrTestTime;
                                    IterByHour->second.iContactCount = IterByHour->second.iContactCount + 1;
                                }
                            }
                            else
                            {
                                // golden catch: `e.HelpContext + ":" + e.Message` is int + char* (pointer arithmetic); with
                                // HelpContext 0 it yields ":" + Message.  The MyDBIProcess("Exception", ...) call is not ported.
                                exceptions.push_back(":'" + s2 + "' is not a valid date and time");
                            }
                        }
                    }
                    slAllData.push_back(BcbGetCommaText(slLine));
                    MySummary.iInputCount++;
                }
            }
        }
    }
    if (MySummary.iInputCount != 0)
        MySummary.iInputCount--;
    UpdateSgProduction();
    SetTotalSummary();
    return true;
}

static void FillProductionRow(Row& r, const Summary& s, bool byHour)
{
    // golden UpdateSgProduction :1799-1868 (by day) / :1873-1944 (by hour)
    const long unitMS = byHour ? OneHourMS : OneDayMS;
    const int iInput = s.iInputCount, iContact = s.iContactCount;
    const int iUPH = byHour ? iInput : iInput / 24;
    long lTemp;
    if (s.iInputCount == 0)     lTemp = 0;
    else if (s.iFailCount <= 1) lTemp = unitMS;
    else                        lTemp = unitMS / s.iFailCount;
    const long lMTBFSec = lTemp / 1000;
    // golden bug kept: by hour multiplies the day fraction dtTestTime by OneHourMS
    const long lTestTimeMS = (long)(double(s.dtTestTime) * double(unitMS) + double(s.iTestTimeMS));
    const long lTestSec = lTestTimeMS / 1000, lTestMSec = lTestTimeMS % 1000;
    lTemp = (iContact != 0) ? lTestTimeMS / iContact : lTestTimeMS;
    const long lAvgTSec = lTemp / 1000, lAvgTMSec = lTemp % 1000;
    int iMUBA;
    if (s.iInputCount == 0)      { iMUBA = 0; lTemp = 0; }
    else if (s.iAlarmCount <= 1) { iMUBA = s.iInputCount; lTemp = unitMS; }
    else                         { iMUBA = iInput / s.iAlarmCount; lTemp = unitMS / s.iAlarmCount; }
    const long lMTBASec = lTemp / 1000;

    r.resize(iSgTotal);
    r[iSgInputQty] = IntStr(iInput);
    r[iSgUPH] = IntStr(iUPH);
    r[iSgContactCnt] = IntStr(iContact);
    r[iSgMUBA] = IntStr(iMUBA);
    r[iSgFaliureCnt] = IntStr(s.iFailCount);
    r[iSgAlarmCount] = IntStr(s.iAlarmCount);
    r[iSgTotalStopTime] = ConvertSecToTime(s.iStopTime);
    r[iSgSumAlarmTime] = ConvertSecToTime(s.iAlarmTime);
    r[iSgSumFailTime] = ConvertSecToTime(s.iFailTime);
    r[iSgTotalTestTime] = ConvertSecToTime(lTestSec, lTestMSec);
    r[iSgAvgTestTime] = ConvertSecToTime(lAvgTSec, lAvgTMSec);
    r[iSgMTBF] = ConvertSecToTime(lMTBFSec);
    r[iSgMTBA] = ConvertSecToTime(lMTBASec);
}

void Analyzer::UpdateSgProduction()
{
    for (size_t i = 0; i < sgByDay.size(); ++i)
    {
        std::map<std::string, Summary>::const_iterator it = mapByDayList.find(sgByDay[i][iSgDate]);
        if (it != mapByDayList.end()) FillProductionRow(sgByDay[i], it->second, false);
    }
    for (size_t i = 0; i < sgByHour.size(); ++i)
    {
        std::map<std::string, Summary>::const_iterator it = mapByHourList.find(sgByHour[i][iSgDate]);
        if (it != mapByHourList.end()) FillProductionRow(sgByHour[i], it->second, true);
    }
}

static std::string Line(const char* label, const std::string& v) { return std::string(label) + v; }

void Analyzer::SetTotalSummary()
{
    // golden :2468-2566 (the lines of mmoSummary, in golden order)
    double dtCurrent = dtEnd - dtStart;
    long lSec, lMS, lTestTime;
    const std::string sInput = Line("Total Input Qty:   ", IntStr(MySummary.iInputCount));
    const std::string sTotalTime = Line("Time Period:       ", ConvertDTToTime(dtCurrent));
    const std::string sJam = Line("JAM Count:         ", IntStr(MySummary.iJAMCount));
    const std::string sWar = Line("WAR Count:         ", IntStr(MySummary.iWARCount));
    const std::string sMes = Line("MES Count:         ", IntStr(MySummary.iMESCount));
    const std::string sAlarmCount = Line("Alarm Count:       ", IntStr(MySummary.iAlarmCount));
    const std::string sFailCount = Line("Fail Count:        ", IntStr(MySummary.iFailCount));
    const std::string sStopTime = Line("Total Stop Time:   ", ConvertSecToTime(MySummary.iStopTime, 0));
    const std::string sAlarmTime = Line("Alarm Time:        ", ConvertSecToTime(MySummary.iAlarmTime, 0));
    const std::string sFailTime = Line("Fail Time:         ", ConvertSecToTime(MySummary.iFailTime, 0));
    const std::string sUPH = Line("Average UPH:       ", IntStr(GetUPH(MySummary, iDate)));
    const std::string sMUBA = Line("MUBA:              ", IntStr(GetMUBA(MySummary)));

    lSec = (long)(double(dtCurrent) * double(OneDaySec));
    lSec = (long)GetMTBA(MySummary, (double)lSec);
    const std::string sMTBA = Line("MTBA:              ", ConvertSecToTime(lSec));

    lSec = (long)(double(dtCurrent) * double(OneDaySec));
    std::string sMTBF;
    if (MySummary.iInputCount == 0)     sMTBF = Line("MTBF:              ", ConvertSecToTime(0));
    else if (MySummary.iFailCount == 0) sMTBF = Line("MTBF:              ", ConvertSecToTime(lSec));
    else                                sMTBF = Line("MTBF:              ", ConvertSecToTime(lSec / MySummary.iFailCount));

    lTestTime = (long)(double(MySummary.dtTestTime) * double(OneDayMS) + MySummary.iTestTimeMS);
    lSec = lTestTime / 1000;
    lMS = lTestTime % 1000;
    const std::string sTestTime = Line("Total Test Time:   ", ConvertSecToTime(lSec, lMS));
    if (MySummary.iContactCount > 1)
        lTestTime = lTestTime / MySummary.iContactCount;
    lSec = lTestTime / 1000;
    lMS = lTestTime % 1000;
    const std::string sAvgTTime = Line("Average Test Time: ", ConvertSecToTime(lSec, lMS));

    dtCurrent = (dtEnd - dtStart) - (((double(MySummary.iStopTime) / 24.0) / 60.0) / 60.0);
    const std::string sRunTime = Line("Running Time:      ", ConvertDTToTime(dtCurrent));

    mmoSummary.clear();
    mmoSummary.push_back("//===================================");
    mmoSummary.push_back("Handler ID:        " + sHandlerID);
    mmoSummary.push_back(sInput);
    mmoSummary.push_back(sTotalTime);
    mmoSummary.push_back(sMUBA);
    mmoSummary.push_back(sMTBA);
    mmoSummary.push_back(sMTBF);
    mmoSummary.push_back(sTestTime);
    mmoSummary.push_back(sAvgTTime);
    mmoSummary.push_back(sUPH);
    mmoSummary.push_back(sMes);
    mmoSummary.push_back(sWar);
    mmoSummary.push_back(sJam);
    mmoSummary.push_back(sAlarmCount);
    mmoSummary.push_back(sAlarmTime);
    mmoSummary.push_back(sFailCount);
    mmoSummary.push_back(sFailTime);
    mmoSummary.push_back(sStopTime);
    mmoSummary.push_back(sRunTime);
    mmoSummary.push_back("//===================================");
}

}  // namespace ela
