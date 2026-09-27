// ===========================================================================
//  EventLogAnalysis/ElaCore.cpp -- see ElaCore.h.  AI(W906-ELA-P1) 20260927.
// ===========================================================================
#include "EventLogAnalysis/ElaCore.h"
#include "vclcompat/IniFiles.h"   // TIniFile: Win32 profile semantics (golden Common.cpp CheckAndReadIniData)

using vclcompat::AnsiString;
using vclcompat::TIniFile;

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ela {

const char* const kJamAreaNames[31] = {
    "01 Input Arm", "02 Output Arm", "03 Index Unit", "04 Input Shuttle", "05 Output Shuttle", "06 Empty Tray Arm",
    "07 Tester I/F", "08 Scanner", "09 Tray Loader", "10 Empty Tray", "11 Tray Unloader 1", "12 Tray Unloader 2",
    "13 Tray Unloader 3", "14 Color Tray", "15 Temp. Controller", "16 System", "17 Fix Tray 1", "18 Fix Tray 2",
    "19 Fix Tray 3", "20 ESD System", "21 Process Log", "22 Motion Log", "23 Message", "24 Motor",
    "25 Tray Unloader 4", "26 Tray Unloader 5", "27 Tray Unloader 6", "28 Fix Tray 4", "29 Fix Tray 5",
    "30 Fix Tray 6", "31 Cylinder" };

// ---------------------------------------------------------------------------
Summary::Summary()
    : iInputCount(0), iStopCount(0), iAlarmCount(0), iFailCount(0), iJAMCount(0), iMESCount(0), iWARCount(0),
      iContactCount(0), dtTestTime(0.0), iTestTimeMS(0), iStopTime(0), iAlarmTime(0), iFailTime(0)
{
}

void Summary::InitData(bool keepGoldenLeak)
{
    // golden Analyzer.cpp:111-126, in golden order.  iFailCount is missing from golden's list (#18, ELA-4).
    Date            ="";
    iInputCount     =0;
    iStopCount      =0;
    iAlarmCount     =0;
    iJAMCount       =0;
    iMESCount       =0;
    iWARCount       =0;
    iContactCount   =0;
    dtTestTime      =0.0;
    iTestTimeMS     =0;
    iStopTime       =0;
    iAlarmTime      =0;
    iFailTime       =0;
    if (!keepGoldenLeak)
        iFailCount  =0;
}

JamSummary::JamSummary() : iCount(0), iStopTime(0) {}

void JamSummary::InitData()
{
    // golden Analyzer.cpp:128-135
    sAlarmCode      ="";
    sUnitName       ="";
    sMessage        ="";
    iCount          =0;
    iStopTime       =0;
}

Options::Options()
    : bcbCommaText(true), jamWriteBack(true), keepGoldenLeaks(true),
      jamIniPath("D:\\HT9045\\Error\\English\\JAM0000.dat")
{
}

// ===========================================================================
//  BCB6 RTL behaviour
// ===========================================================================
static bool IsBlank(unsigned char c) { return c >= 1 && c <= ' '; }   // Delphi `P^ in [#1..' ']`

// SysUtils.AnsiExtractQuotedStr(Src, '"'), index for index: *io is on the opening quote and ends one past the closing
// quote (or at the end).  Byte-wise is exact for cp950 and UTF-8 here: neither uses 0x22 as a trail byte.  RTL quirks
// kept: with no closing quote the last character is dropped (Src := StrEnd(P); ... Src - P - 1), and `""` gives "".
static std::string ExtractQuoted(const std::string& s, size_t* io)
{
    const size_t n = s.size();
    const size_t npos = std::string::npos;
    size_t src = *io + 1;                                   // Inc(Src)
    int dropCount = 1;
    const size_t p0 = src;                                  // P := Src
    size_t q = s.find('"', src);                            // Src := AnsiStrScan(Src, Quote)
    size_t end = npos;
    while (q != npos)
    {
        src = q + 1;                                        // Inc(Src)
        if (src >= n || s[src] != '"') { end = src; break; }   // if Src^ <> Quote then Break
        ++src;                                              // Inc(Src)
        ++dropCount;
        q = s.find('"', src);
    }
    if (end == npos) end = n;                               // if Src = nil then Src := StrEnd(P)
    *io = end;
    if (end - p0 <= 1)                                      // if ((Src - P) <= 1) then Exit
        return std::string();
    if (dropCount == 1)
        return s.substr(p0, end - p0 - 1);                  // SetString(Result, P, Src - P - 1)
    std::string out;
    size_t P = p0;
    size_t S = s.find('"', P);                              // Src := AnsiStrScan(P, Quote)
    while (S != npos)
    {
        ++S;                                                // Inc(Src)
        if (S >= n || s[S] != '"') break;                   // if Src^ <> Quote then Break
        out.append(s, P, S - P);                            // Move(P^, Dest^, Src - P)
        ++S;                                                // Inc(Src)
        P = S;
        S = s.find('"', S);
    }
    const size_t E = (S == npos) ? n : S;                   // if Src = nil then Src := StrEnd(P)
    if (E > P + 1)
        out.append(s, P, E - P - 1);                        // Move(P^, Dest^, Src - P - 1)  (count <= 0 moves nothing)
    return out;
}

Row BcbCommaText(const std::string& v)
{
    // Classes.TStrings.SetDelimitedText with Delimiter=',' QuoteChar='"' (BCB6 / Delphi 6-7 RTL).
    Row r;
    const size_t n = v.size();
    size_t i = 0;
    while (i < n && v[i] != '\0' && IsBlank((unsigned char)v[i])) ++i;
    while (i < n && v[i] != '\0')
    {
        std::string s;
        if (v[i] == '"')
            s = ExtractQuoted(v, &i);
        else
        {
            const size_t p1 = i;
            while (i < n && (unsigned char)v[i] > ' ' && v[i] != ',') ++i;
            s = v.substr(p1, i - p1);
        }
        r.push_back(s);
        while (i < n && v[i] != '\0' && IsBlank((unsigned char)v[i])) ++i;
        if (i < n && v[i] == ',')
        {
            if (i + 1 >= n || v[i + 1] == '\0')
                r.push_back(std::string());                 // a trailing delimiter adds an empty item (Delphi 7 RTL;
                                                            //  verify against the reference exe -- ledger P1)
            do { ++i; } while (i < n && v[i] != '\0' && IsBlank((unsigned char)v[i]));
        }
    }
    return r;
}

Row CommaOnlyText(const std::string& v)
{
    // #15 B (not the default): quote-aware split on commas only; blanks are kept as field content.
    Row r;
    std::string cur;
    bool inQ = false, any = false;
    for (size_t i = 0; i < v.size(); ++i)
    {
        const char c = v[i];
        any = true;
        if (inQ)
        {
            if (c == '"')
            {
                if (i + 1 < v.size() && v[i + 1] == '"') { cur += '"'; ++i; }
                else inQ = false;
            }
            else cur += c;
        }
        else if (c == '"') inQ = true;
        else if (c == ',') { r.push_back(cur); cur.clear(); }
        else if (c != '\r' && c != '\n') cur += c;
    }
    if (any) r.push_back(cur);
    return r;
}

std::string BcbGetCommaText(const Row& row)
{
    // Classes.TStrings.GetDelimitedText (',' '"'): quote an item that contains a control char, a blank, a quote or a
    // comma (AnsiQuotedStr doubles the quotes); a single empty item gives "".
    if (row.size() == 1 && row[0].empty())
        return "\"\"";
    std::string out;
    for (size_t k = 0; k < row.size(); ++k)
    {
        const std::string& s = row[k];
        bool quote = false;
        for (size_t i = 0; i < s.size(); ++i)
        {
            const unsigned char c = (unsigned char)s[i];
            if (c == 0) break;
            if (c <= ' ' || c == '"' || c == ',') { quote = true; break; }
        }
        if (quote)
        {
            out += '"';
            for (size_t i = 0; i < s.size(); ++i) { if (s[i] == '"') out += '"'; out += s[i]; }
            out += '"';
        }
        else
            out += s;
        if (k + 1 < row.size()) out += ',';
    }
    return out;
}

std::vector<std::string> BcbLoadLines(const std::string& path, bool* ok)
{
    // TStrings.LoadFromFile -> SetTextStr: CR, LF or CRLF end a line; a final terminator adds no empty line; NUL stops.
    std::vector<std::string> lines;
    if (ok) *ok = false;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return lines;
    std::string data;
    char buf[65536];
    size_t got;
    while ((got = std::fread(buf, 1, sizeof(buf), f)) > 0) data.append(buf, got);
    std::fclose(f);
    if (ok) *ok = true;
    size_t i = 0;
    const size_t n = data.size();
    while (i < n && data[i] != '\0')
    {
        const size_t start = i;
        while (i < n && data[i] != '\0' && data[i] != '\r' && data[i] != '\n') ++i;
        lines.push_back(data.substr(start, i - start));
        if (i < n && data[i] == '\r') ++i;
        if (i < n && data[i] == '\n') ++i;
    }
    return lines;
}

bool FileExistsA(const std::string& path)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

// ===========================================================================
//  Dates (BCB TDateTime: double days since 1899-12-30)
// ===========================================================================
static long long DaysFromCivil(long long y, int m, int d)
{
    y -= (m <= 2) ? 1 : 0;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const long long yoe = y - era * 400;
    const long long mp = (m > 2) ? (m - 3) : (m + 9);
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
    const long long mm = (mp < 10) ? (mp + 3) : (mp - 9);
    *y = (int)(yoe + era * 400 + ((mm <= 2) ? 1 : 0));
    *m = (int)mm;
    *d = (int)dd;
}

static bool IsLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

static int DaysInMonth(int y, int m)
{
    static const int k[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return (m == 2 && IsLeap(y)) ? 29 : k[m - 1];
}

// golden TDateTime values are doubles in memory (Delphi stores every Result / variable): on a 32-bit x87 build GCC may
// keep a C++ double in an 80-bit register, so the places where golden stores a TDateTime are forced through memory.
static double RoundToDouble(double x)
{
    volatile double v = x;
    return v;
}

double EncodeDate(int y, int m, int d)
{
    return RoundToDouble((double)(DaysFromCivil(y, m, d) - DaysFromCivil(1899, 12, 30)));
}

double EncodeTime(int h, int n, int s, int ms)
{
    // SysUtils.EncodeTime: (Hour*3600000 + Min*60000 + Sec*1000 + MSec) / MSecsPerDay, stored as TDateTime
    return RoundToDouble((h * 3600000.0 + n * 60000.0 + s * 1000.0 + ms) / 86400000.0);
}

long long DateTimeToMs(double dt)
{
    // FLD DateTime; FMUL FMSecsPerDay; FISTP qword -- long double is the x87 80-bit type on MinGW (== double on MSVC)
    return std::llrint((long double)dt * 86400000.0L);   // C++11 long double overload (std::llrintl is missing from old libstdc++)
}

void DecodeDateTime(double dt, int* y, int* m, int* d, int* h, int* n, int* s, int* ms)
{
    // SysUtils.DateTimeToTimeStamp (Delphi 6 asm): ms := Round(DateTime * MSecsPerDay); a negative count is negated,
    // divided and the quotient negated again (the remainder stays positive); Date := quotient + DateDelta, Time := rest.
    const long long all = DateTimeToMs(dt);
    long long days, t;
    if (all >= 0)
    {
        days = all / 86400000LL;
        t = all % 86400000LL;
    }
    else
    {
        days = -((-all) / 86400000LL);
        t = (-all) % 86400000LL;
    }
    CivilFromDays(days + DaysFromCivil(1899, 12, 30), y, m, d);
    *h = (int)(t / 3600000); t %= 3600000;
    *n = (int)(t / 60000);   t %= 60000;
    *s = (int)(t / 1000);
    *ms = (int)(t % 1000);
}

static std::string Fmt(const char* f, int a, int b, int c, int d = 0)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), f, a, b, c, d);
    return buf;
}

std::string FormatYMD(double dt)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(dt, &y, &m, &d, &h, &n, &s, &ms);
    return Fmt("%04d/%02d/%02d", y, m, d);
}

std::string FormatYMDH00(double dt)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(dt, &y, &m, &d, &h, &n, &s, &ms);
    return Fmt("%04d/%02d/%02d %02d:00:00", y, m, d, h);
}

std::string FormatYYYYMMDD(double dt)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(dt, &y, &m, &d, &h, &n, &s, &ms);
    return Fmt("%04d%02d%02d", y, m, d);
}

static std::string Trim(const std::string& s)
{
    // AnsiString::Trim: drop leading / trailing chars <= ' '
    size_t b = 0, e = s.size();
    while (b < e && (unsigned char)s[b] <= ' ') ++b;
    while (e > b && (unsigned char)s[e - 1] <= ' ') --e;
    return s.substr(b, e - b);
}

bool IsDigitString(const std::string& s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit((unsigned char)s[i]))
            return false;
    return true;
}

bool IsValidDateString(const std::string& in)
{
    // golden Common.cpp:630-655 (1-based AnsiString indexes shifted to 0-based)
    const std::string d = Trim(in);
    if (d.size() != 10)
        return false;
    const char delim1 = d[4], delim2 = d[7];
    if ((delim1 != '/' && delim1 != '-') || delim1 != delim2)
        return false;
    const std::string ys = d.substr(0, 4), ms = d.substr(5, 2), ds = d.substr(8, 2);
    if (!IsDigitString(ys) || !IsDigitString(ms) || !IsDigitString(ds))
        return false;
    const int month = std::atoi(ms.c_str()), day = std::atoi(ds.c_str());
    if (month < 1 || month > 12 || day < 1 || day > 31)
        return false;
    return true;
}

bool IsValidTimeString(const std::string& in)
{
    // golden Common.cpp:657-690
    const std::string t = Trim(in);
    const size_t len = t.size();
    if (len != 8 && len != 12)
        return false;
    if (t[2] != ':' || t[5] != ':')
        return false;
    const bool hasMs = (len == 12);
    if (hasMs && t[8] != '.')
        return false;
    const std::string hs = t.substr(0, 2), ns = t.substr(3, 2), ss = t.substr(6, 2);
    const std::string mss = hasMs ? t.substr(9, 3) : std::string();
    if (!IsDigitString(hs) || !IsDigitString(ns) || !IsDigitString(ss) || (hasMs && !IsDigitString(mss)))
        return false;
    const int hour = std::atoi(hs.c_str()), mn = std::atoi(ns.c_str()), sec = std::atoi(ss.c_str());
    const int msv = hasMs ? std::atoi(mss.c_str()) : 0;
    if (hour < 0 || hour > 23 || mn < 0 || mn > 59 || sec < 0 || sec > 59 || (hasMs && (msv < 0 || msv > 999)))
        return false;
    return true;
}

double ParseDateTime(const std::string& sDateIn, const std::string& sTimeIn)
{
    // golden Common.cpp:711-731 + ParseDate :733-770 + ParseTime :772-790.  golden's EncodeDate raises EConvertError for
    // a day past the month's end (the validator allows 31 in every month); the catch(...) returns TDateTime() = 0.0.
    const std::string sDate = Trim(sDateIn), sTime = Trim(sTimeIn);
    if (sDate.empty() || sTime.empty())
        return 0.0;
    if (!IsValidDateString(sDate) || !IsValidTimeString(sTime))
        return 0.0;
    const int year = std::atoi(sDate.substr(0, 4).c_str());
    const int month = std::atoi(sDate.substr(5, 2).c_str());
    const int day = std::atoi(sDate.substr(8, 2).c_str());
    if (day > DaysInMonth(year, month))
        return 0.0;
    const int hour = std::atoi(sTime.substr(0, 2).c_str());
    const int mn = std::atoi(sTime.substr(3, 2).c_str());
    const int sec = std::atoi(sTime.substr(6, 2).c_str());
    const int ms = (sTime.find('.') != std::string::npos) ? std::atoi(sTime.substr(9, 3).c_str()) : 0;
    return RoundToDouble(EncodeDate(year, month, day) + EncodeTime(hour, mn, sec, ms));   // golden `return dDate+dTime;`
}

// ===========================================================================
//  JAM0000.dat (golden Analyzer.cpp:2799-2882 + Common.cpp CheckAndReadIniData)
// ===========================================================================
bool JamConfig::CheckAndReadBool(const std::string& group, const std::string& name, bool value)
{
    TIniFile ini(AnsiString(opt_.jamIniPath.c_str()));
    if (!ini.ValueExists(AnsiString(group.c_str()), AnsiString(name.c_str())))
    {
        if (opt_.jamWriteBack)                                    // #17 ELA-3 A (golden): write the default
        {
            ini.WriteBool(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
            ++writes;
        }
        return value;
    }
    return ini.ReadBool(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
}

int JamConfig::CheckAndReadInt(const std::string& group, const std::string& name, int value)
{
    TIniFile ini(AnsiString(opt_.jamIniPath.c_str()));
    if (!ini.ValueExists(AnsiString(group.c_str()), AnsiString(name.c_str())))
    {
        if (opt_.jamWriteBack)
        {
            ini.WriteInteger(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
            ++writes;
        }
        return value;
    }
    return ini.ReadInteger(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
}

int JamConfig::GetJamLevel(const std::string& sJamArea, const std::string& sJamCode)
{
    int bLevel = 0;
    if (sJamArea != "" && sJamCode != "")
    {
        if (FileExistsA(opt_.jamIniPath))                         //Steven 20240820 : Fixed for off-line usage
            bLevel = CheckAndReadInt(sJamArea, sJamCode, 0);
    }
    return bLevel;
}

bool JamConfig::GetJemIncludeMTBA(const std::string& sJamArea, const std::string& sJamCode)
{
    bool bIncludeMTBA = false;
    if (sJamArea != "" && sJamCode != "")
    {
        if (FileExistsA(opt_.jamIniPath))
        {
            if (sJamCode.find("JAM") != std::string::npos &&
                (sJamArea == "01 Input Arm" ||
                 sJamArea == "02 Output Arm" ||
                 sJamArea == "03 Index Unit" ||
                 sJamArea == "04 Input Shuttle" ||
                 sJamArea == "05 Output Shuttle"))
                bIncludeMTBA = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBA", true);
            else
                bIncludeMTBA = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBA", false);
        }
    }
    return bIncludeMTBA;
}

bool JamConfig::GetJemIncludeMTBF(const std::string& sJamArea, const std::string& sJamCode)
{
    bool bIncludeMTBF = false;
    if (FileExistsA(opt_.jamIniPath))
    {
        if (sJamArea != "" && sJamCode != "")
        {
            if (sJamArea == "24 Motor" ||
                sJamCode == "WAR01300" || sJamCode == "WAR01301" || sJamCode == "WAR0348" ||
                sJamCode == "JAM0407" || sJamCode == "JAM0408" ||
                sJamCode == "WAR1635" || sJamCode == "WAR1636" || sJamCode == "WAR1638" || sJamCode == "WAR1639" ||
                sJamCode == "WAR1690" || sJamCode == "WAR1691" || sJamCode == "WAR1692" || sJamCode == "WAR1693" ||
                sJamCode == "WAR1694" || sJamCode == "WAR1695" || sJamCode == "WAR1696" || sJamCode == "WAR16109" ||
                sJamCode == "WAR2201" || sJamCode == "WAR2202" || sJamCode == "WAR2203" ||
                sJamCode == "WAR16150" || sJamCode == "WAR16151" || sJamCode == "WAR16152" ||
                sJamCode == "MES16119")
                bIncludeMTBF = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBF", true);
            else
                bIncludeMTBF = CheckAndReadBool(sJamArea, sJamCode + " IncludeMTBF", false);
        }
    }
    return bIncludeMTBF;
}

// ===========================================================================
//  Analyzer
// ===========================================================================
Analyzer::Analyzer(const Options& o)
    : dtStart(0.0), dtEnd(0.0), iDate(0), jam(o), iTop5FilterRowCount(6), byAreaCount(0), opt_(o)
{
    // golden ctor Analyzer.cpp:265-278: mapUnitName over the By-Area check boxes (the first eUnitNameTotal names)
    for (int i = 0; i < eUnitNameTotal; i++)
        mapUnitName[kJamAreaNames[i]] = i;
}

Row Analyzer::Split(const std::string& line) const
{
    return opt_.bcbCommaText ? BcbCommaText(line) : CommaOnlyText(line);   // #15
}

void Analyzer::SetRange(double startDate, double startTime, double endDate, double endTime)
{
    // GetSysDateTimeRange :3228-3233 / GetDateTime :3222-3226 (DateOf + TimeOf) / GetTotalDays :3235-3238
    const double sd = std::floor(startDate), ed = std::floor(endDate);
    dtStart = RoundToDouble(sd + (startTime - std::floor(startTime)));
    dtEnd = RoundToDouble(ed + (endTime - std::floor(endTime)));
    iDate = (int)dtEnd - (int)dtStart + 1;

    // By Day: dtByDay = sDateS ("YYYY/MM/DD " -> midnight of the start date); do { key; +1 } while(cur < dtEnd)
    byDayKeys.clear();
    mapByDayList.clear();
    double cur = sd;
    do
    {
        Summary s;
        s.InitData(opt_.keepGoldenLeaks);
        s.Date = FormatYMD(cur);
        byDayKeys.push_back(s.Date);
        mapByDayList[s.Date] = s;
        cur = RoundToDouble(cur + 1.0);                        // golden TDateTime variable dtCurrByDay
    } while (cur < dtEnd);

    // By Hour: dtByHour = sDateS+sTimeS ("YYYY/MM/DD HH:NN:SS", milliseconds dropped); +1/24 while(cur < dtEnd)
    int y, m, d, h, n, s, ms;
    DecodeDateTime(startTime, &y, &m, &d, &h, &n, &s, &ms);
    byHourKeys.clear();
    mapByHourList.clear();
    cur = RoundToDouble(sd + EncodeTime(h, n, s, 0));
    do
    {
        Summary hs;
        hs.InitData(opt_.keepGoldenLeaks);
        hs.Date = FormatYMDH00(cur);
        byHourKeys.push_back(hs.Date);
        mapByHourList[hs.Date] = hs;
        cur = RoundToDouble(cur + (1.0 / 24.0));               // golden TDateTime variable dtCurrByHour
    } while (cur < dtEnd);

    mapAlarmSummary.clear();
    mapJamSummary.clear();
    mapWarSummary.clear();
    MySummary.InitData(opt_.keepGoldenLeaks);                  // #18: golden leaves iFailCount
    if (!opt_.keepGoldenLeaks)
    {
        for (int i = 0; i < eUnitNameTotal; i++) ListByUnitName[i].clear();
        for (int i = 0; i < eByFuncTotal; i++) ListByFunction[i].clear();
    }
    InitGridsForRange();                                       // P1b: SetStringGrid's grid part (ElaTables.cpp)
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }       // AnsiPos()!=0
static bool StartsWith(const std::string& s, const char* t) { return s.compare(0, std::strlen(t), t) == 0; }   // Pos()==1

void Analyzer::GetEventLogText(const std::vector<std::string>& lstEventLog)
{
    slStopList.clear();
    slJamList.clear();
    slJamWarList.clear();
    slWarList.clear();
    slAlarmList.clear();
    slFailList.clear();
    filesRead.clear();

    for (int d = 0; d < iDate; d++)
    {
        std::string FN1, FN2;
        if (opt_.custCode == "915" || opt_.custCode == "919")                //RogerYang 20251119 : 偉測MTBF文件生成
        {
            FN1 = FormatYYYYMMDD(dtStart + d);
            FN2 = "_EventLogTxt_";
        }
        else
        {
            FN1 = FormatYYYYMMDD(dtStart + d) + ".csv";
            FN2 = "EventLogTxt_";
        }
        for (size_t iFile = 0; iFile < lstEventLog.size(); iFile++)
        {
            const std::string& strEvent = lstEventLog[iFile];
            if (!Has(strEvent, FN1.c_str()) || !Has(strEvent, FN2.c_str()))
                continue;
            if (!FileExistsA(strEvent))
                continue;
            filesRead.push_back(strEvent);
            bool ok = false;
            const std::vector<std::string> tsLogFile = BcbLoadLines(strEvent, &ok);
            for (size_t iLog = 1; iLog < tsLogFile.size(); iLog++)
            {
                const Row tsRow = Split(tsLogFile[iLog]);
                if ((int)tsRow.size() < elRecipe)
                    continue;
                if (!IsValidDateString(tsRow[elData]))                        //JimmyChiu 20250401 : Add protection
                    continue;
                if (!IsValidTimeString(tsRow[elTime]))
                    continue;

                const double dtCurrent = ParseDateTime(tsRow[elData], tsRow[elTime]);
                if (!(dtCurrent >= dtStart && dtCurrent <= dtEnd))
                    continue;
                if (Has(tsRow[elDuplicate], "1"))                              //Steven 20240820 : 過濾重複操作沒解除的alarm
                    continue;

                const std::string& sCode = tsRow[eAlarmCode];
                const std::string& sUnit = tsRow[elUnit];
                const std::string& sMsg = tsRow[elMessage];
                if ((Has(sCode, "JAM") || Has(sCode, "WAR") || Has(sCode, "MES")) &&
                    (int)tsRow.size() > elStopTime)                                //Alarm Sec
                {
                    const std::string ct = BcbGetCommaText(tsRow);
                    slStopList.push_back(ct);
                    const int iMS = std::atoi(tsRow[elStopTime].c_str());
                    const std::string sByDay = FormatYMD(dtCurrent);
                    const std::string sByHour = FormatYMDH00(dtCurrent);
                    std::map<std::string, Summary>::iterator IterByDay = mapByDayList.find(sByDay);
                    if (IterByDay != mapByDayList.end())
                        IterByDay->second.iStopTime = IterByDay->second.iStopTime + iMS;
                    std::map<std::string, Summary>::iterator IterByHour = mapByHourList.find(sByHour);
                    if (IterByHour != mapByHourList.end())
                        IterByHour->second.iStopTime = IterByHour->second.iStopTime + iMS;

                    MySummary.iStopCount++;
                    MySummary.iStopTime = MySummary.iStopTime + iMS;
                    if (jam.GetJemIncludeMTBA(sUnit, sCode))
                    {
                        slAlarmList.push_back(ct);
                        MySummary.iAlarmCount++;
                        MySummary.iAlarmTime = MySummary.iAlarmTime + iMS;
                        if (IterByDay != mapByDayList.end())
                        {
                            IterByDay->second.iAlarmCount = IterByDay->second.iAlarmCount + 1;
                            IterByDay->second.iAlarmTime = IterByDay->second.iAlarmTime + iMS;
                        }
                        if (IterByHour != mapByHourList.end())
                        {
                            IterByHour->second.iAlarmCount = IterByHour->second.iAlarmCount + 1;
                            IterByHour->second.iAlarmTime = IterByHour->second.iAlarmTime + iMS;
                        }
                    }
                    if (jam.GetJemIncludeMTBF(sUnit, sCode))
                    {
                        slFailList.push_back(ct);
                        MySummary.iFailCount++;
                        MySummary.iFailTime = MySummary.iFailTime + iMS;
                        if (IterByDay != mapByDayList.end())
                        {
                            IterByDay->second.iFailCount = IterByDay->second.iFailCount + 1;
                            IterByDay->second.iFailTime = IterByDay->second.iFailTime + iMS;
                        }
                        if (IterByHour != mapByHourList.end())
                        {
                            IterByHour->second.iFailCount = IterByHour->second.iFailCount + 1;
                            IterByHour->second.iFailTime = IterByHour->second.iFailTime + iMS;
                        }
                    }

                    JamSummary AlarmSummary;
                    AlarmSummary.InitData();
                    AlarmSummary.sAlarmCode = sCode;
                    AlarmSummary.sUnitName = sUnit;
                    AlarmSummary.sMessage = sMsg;
                    AlarmSummary.iStopTime = iMS;
                    AlarmSummary.iCount = 1;

                    if (StartsWith(sCode, "JAM"))
                    {
                        slJamList.push_back(ct);
                        slJamWarList.push_back(ct);
                        MySummary.iJAMCount++;
                        std::map<std::string, JamSummary>::iterator it = mapJamSummary.find(sCode);
                        if (it == mapJamSummary.end()) mapJamSummary[sCode] = AlarmSummary;
                        else { it->second.iStopTime = it->second.iStopTime + iMS; it->second.iCount++; }
                        it = mapAlarmSummary.find(sCode);
                        if (it == mapAlarmSummary.end()) mapAlarmSummary[sCode] = AlarmSummary;
                        else { it->second.iStopTime = it->second.iStopTime + iMS; it->second.iCount++; }
                    }
                    else if (StartsWith(sCode, "WAR") && !Has(sUnit, "Motion"))
                    {
                        slWarList.push_back(ct);
                        slJamWarList.push_back(ct);
                        MySummary.iWARCount++;
                        std::map<std::string, JamSummary>::iterator it = mapWarSummary.find(sCode);
                        if (it == mapWarSummary.end()) mapWarSummary[sCode] = AlarmSummary;
                        else { it->second.iStopTime = it->second.iStopTime + iMS; it->second.iCount++; }
                        it = mapAlarmSummary.find(sCode);
                        if (it == mapAlarmSummary.end()) mapAlarmSummary[sCode] = AlarmSummary;
                        else { it->second.iStopTime = it->second.iStopTime + iMS; it->second.iCount++; }
                    }
                    else if (StartsWith(sCode, "MES"))
                    {
                        MySummary.iMESCount++;
                    }

                    // By Area放資料
                    std::map<std::string, int>::iterator IterUnitName = mapUnitName.find(sUnit);
                    if (IterUnitName != mapUnitName.end())
                        ListByUnitName[IterUnitName->second].push_back(ct);

                    // By Function放資料
                    if (sUnit != "Process" && (Has(sCode, "JAM") || Has(sCode, "WAR") || Has(sCode, "MES")))
                    {
                        if (Has(sMsg, "RTC"))
                            ListByFunction[efRTC].push_back(ct);
                        else if (sUnit == kJamAreaNames[e20ESD] || Has(sMsg, "ESD") || Has(sMsg, "Ion fan"))
                            ListByFunction[efESD].push_back(ct);
                        else if (sUnit == kJamAreaNames[e15Temp] || Has(sMsg, "Temperature"))
                            ListByFunction[efTemp].push_back(ct);
                        else if (Has(sMsg, "2D"))
                            ListByFunction[ef2DID].push_back(ct);
                        else if (Has(sMsg, "OCR"))
                            ListByFunction[efOCR].push_back(ct);
                        else if (Has(sMsg, "clean"))
                            ListByFunction[efAutoClean].push_back(ct);
                        else if (sUnit == kJamAreaNames[e07TesterIF])
                            ListByFunction[efYield].push_back(ct);
                    }
                }
                // golden :1039-1162 ChangeLog / ParameterLog block: UI labels only, not ported (see the header)
            }
        }
    }
}

}  // namespace ela
