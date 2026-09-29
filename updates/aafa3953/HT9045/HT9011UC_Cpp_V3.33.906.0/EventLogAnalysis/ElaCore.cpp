// ===========================================================================
//  EventLogAnalysis/ElaCore.cpp -- see ElaCore.h.  AI(W906-ELA-P1) 20260927.
// ===========================================================================
#include "EventLogAnalysis/ElaCore.h"
#include "vclcompat/IniFiles.h"   // TIniFile: Win32 profile semantics (golden Common.cpp CheckAndReadIniData)
#include "JamRules.h"              // AI(W906-ELA-W45) 20260927: the JAM0000.dat keys / missing-key defaults (shared rules)
#include <windows.h>               // CreateMutexA / WaitForSingleObject: the JAM0000.dat lock (JamFileLock below)

using vclcompat::AnsiString;
using vclcompat::TIniFile;

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>

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

void Summary::InitData(bool keepGoldenBug)
{
    // golden Analyzer.cpp:111-126, in golden order.  iFailCount is missing from golden's list (#18: W18 B resets it).
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
    if (!keepGoldenBug)
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
    : bcbCommaText(false), jamWriteBack(true), keepGoldenBugs(false), goldenFileRule(false),   // #15 / #18 / #19 = B
      goldenFileRuleVtest(true),                                                              // ★W38 = B (Steven 20260928)
      goldenJamMtbfDefault(false),                                                             // ★W45 18b = B
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

double GetStartEndGap(double dtStart, double dtEnd)
{
    // golden Common.cpp:511-516 (AI(W906-ELA-R1) 20260927): `double dret=dtEnd-dtStart; return dret;`
    return RoundToDouble(dtEnd - dtStart);
}

// ===========================================================================
//  W19 B file selection (Steven 20260927; AI(W906-ELA-W19) 20260927, St02-E) -- not golden: golden's rule is kept
//  behind Options::goldenFileRule in Analyzer::GetEventLogText
// ===========================================================================
static std::string LowerAsciiCopy(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

static bool DigitsAt(const std::string& s, size_t b, size_t n)
{
    if (b + n > s.size()) return false;
    for (size_t i = b; i < b + n; ++i)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

static int IntAt(const std::string& s, size_t b, size_t n) { return std::atoi(s.substr(b, n).c_str()); }

bool IsAllEventLogPath(const std::string& path)
{
    // golden MyStringList MySaveToFile_1 writes the copy into "<HTPath>\AllEventLog" (V906 Public/MyStringList.cpp:586)
    size_t b = 0;
    while (b <= path.size())
    {
        size_t e = path.find_first_of("\\/", b);
        if (e == std::string::npos) e = path.size();
        if (LowerAsciiCopy(path.substr(b, e - b)) == "alleventlog") return true;
        b = e + 1;
    }
    return false;
}

bool EventLogFileSpan(const std::string& path, double* from, double* to, std::string* maxLineSeries)
{
    // The names the Handler writes (golden TMyStringList::GetFileName, V906 Public/MyStringList.cpp:731-903; the
    // AllEventLog copy :586-592 is always a day name), base <n> = EventLogTxt, EventLogTxt_<ID> (O15) or
    // <MT>_<ID>_EventLogTxt (N10):
    //   <n>_yyyymmdd[_RT|_FT|_TesterOffline].csv   day                          [D, D+1)
    //   <n>_yyyymmdd HH[_RT|_FT].csv               1 / 2 / 4 / 6 / 8 / 12 h      [D+HH, D+1)   (a period never passes 24:00)
    //   <n>_yyyymmdd HHNNSS.csv                    max line count               [D+HHNNSS, open) here; SelectEventLogFiles
    //                                                                           widens it to [previous, next) of its
    //                                                                           base name (see ElaCore.h)
    //   <n>_yyyymmddHH00.csv                       12 h at 08:00 / 20:00        [D+HH, D+HH+12 h) -- 2000 runs to 08:00
    //   <n>_yyyymm.csv / <n>_yyyy.csv              month / year                 the whole month / year
    // golden's FN2 test "the path contains EventLogTxt_" stays the test for "an event-log file", so ByLot, JamStat and
    // SGJamCount RawData files never match; minute files are .TXT and never listed (LoadFavorite takes *.csv).
    if (maxLineSeries) maxLineSeries->clear();
    if (path.find("EventLogTxt_") == std::string::npos)
        return false;
    const size_t slash = path.find_last_of("\\/");
    const std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    if (name.size() < 4 || LowerAsciiCopy(name.substr(name.size() - 4)) != ".csv")
        return false;
    std::string stem = name.substr(0, name.size() - 4);
    static const char* const kTag[3] = { "_rt", "_ft", "_testeroffline" };
    const std::string lower = LowerAsciiCopy(stem);
    for (int k = 0; k < 3; ++k)
    {
        const size_t n = std::strlen(kTag[k]);
        if (lower.size() > n && lower.compare(lower.size() - n, n, kTag[k]) == 0)
        {
            stem.erase(stem.size() - n);
            break;
        }
    }
    const size_t u = stem.rfind('_');
    if (u == std::string::npos)
        return false;
    const std::string t = stem.substr(u + 1);
    double f = 0.0, e = 0.0;
    if (t.size() == 4 && DigitsAt(t, 0, 4))                                    // year
    {
        const int y = IntAt(t, 0, 4);
        if (y < 1900 || y > 9998) return false;
        f = EncodeDate(y, 1, 1);
        e = EncodeDate(y + 1, 1, 1);
    }
    else if (t.size() == 6 && DigitsAt(t, 0, 6))                               // month
    {
        const int y = IntAt(t, 0, 4), m = IntAt(t, 4, 2);
        if (y < 1900 || y > 9998 || m < 1 || m > 12) return false;
        f = EncodeDate(y, m, 1);
        e = (m == 12) ? EncodeDate(y + 1, 1, 1) : EncodeDate(y, m + 1, 1);
    }
    else if (t.size() >= 8 && DigitsAt(t, 0, 8))
    {
        const int y = IntAt(t, 0, 4), m = IntAt(t, 4, 2), d = IntAt(t, 6, 2);
        if (y < 1900 || y > 9998 || m < 1 || m > 12 || d < 1 || d > DaysInMonth(y, m)) return false;
        const double D = EncodeDate(y, m, d);
        const std::string r = t.substr(8);
        if (r.empty())                                                         // day
        {
            f = D;
            e = D + 1.0;
        }
        else if (r.size() == 3 && r[0] == ' ' && DigitsAt(r, 1, 2))            // "D HH"
        {
            const int h = IntAt(r, 1, 2);
            if (h > 23) return false;
            f = D + EncodeTime(h, 0, 0, 0);
            e = D + 1.0;
        }
        else if (r.size() == 7 && r[0] == ' ' && DigitsAt(r, 1, 6))            // "D HHNNSS", max line count
        {
            const int h = IntAt(r, 1, 2), n = IntAt(r, 3, 2), sec = IntAt(r, 5, 2);
            if (h > 23 || n > 59 || sec > 59) return false;
            f = D + EncodeTime(h, n, sec, 0);
            e = kEventLogSpanOpenEnd;                                          // SelectEventLogFiles sets the real span
            if (maxLineSeries)
                *maxLineSeries = (IsAllEventLogPath(path) ? "alleventlog|" : "|") + LowerAsciiCopy(stem.substr(0, u));
        }
        else if (r.size() == 4 && DigitsAt(r, 0, 4) && r[2] == '0' && r[3] == '0')   // "DHH00", 12 h at 08 / 20
        {
            const int h = IntAt(r, 0, 2);
            if (h > 23) return false;
            f = D + EncodeTime(h, 0, 0, 0);
            e = f + 0.5;
        }
        else
            return false;
    }
    else
        return false;
    *from = RoundToDouble(f);
    *to = RoundToDouble(e);
    return true;
}

// ===========================================================================
//  JAM0000.dat (golden Analyzer.cpp:2799-2882 + Common.cpp CheckAndReadIniData)
// ===========================================================================
// AI(W906-SEC-S128) 20260927 (St02): the one JAM0000.dat lock, shared with the security.jam page in the same wb_serve
//   process (WebSecurityJam.cpp / JamIniMerge.h on v906/steven-st02-on-cbridge: open / select / save / import and the
//   S128 merge).  A Win32 named mutex, so neither side links against the other; recursive per thread, WAIT_ABANDONED
//   counts as acquired.  Golden had the same two writers in two programs with no lock at all.
//   Held for one key's check-then-write.  Not acquired within 5 s (the page holds it for one op or one 200-key merge
//   chunk) -> the key is read but its default is NOT written back this time; the next query writes it.
namespace {
class JamFileLock {
public:
    JamFileLock() : h_(::CreateMutexA(NULL, FALSE, "Local\\HT9045_JAM0000_dat")), held_(false)
    {
        if (h_) { const DWORD r = ::WaitForSingleObject(h_, 5000); held_ = (r == WAIT_OBJECT_0 || r == WAIT_ABANDONED); }
    }
    ~JamFileLock() { if (held_) ::ReleaseMutex(h_); if (h_) ::CloseHandle(h_); }
    bool held() const { return held_; }
private:
    HANDLE h_;
    bool held_;
    JamFileLock(const JamFileLock&);
    JamFileLock& operator=(const JamFileLock&);
};
}  // namespace

bool JamConfig::CheckAndReadBool(const std::string& group, const std::string& name, bool value)
{
    JamFileLock lock;                                             // S128: same lock as the security.jam page
    TIniFile ini(AnsiString(opt_.jamIniPath.c_str()));
    if (!ini.ValueExists(AnsiString(group.c_str()), AnsiString(name.c_str())))
    {
        if (opt_.jamWriteBack && lock.held())                     // #17 ELA-3 A (golden): write the default (S128: only under the lock)
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
    JamFileLock lock;                                             // S128: same lock as the security.jam page
    TIniFile ini(AnsiString(opt_.jamIniPath.c_str()));
    if (!ini.ValueExists(AnsiString(group.c_str()), AnsiString(name.c_str())))
    {
        if (opt_.jamWriteBack && lock.held())
        {
            ini.WriteInteger(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
            ++writes;
        }
        return value;
    }
    return ini.ReadInteger(AnsiString(group.c_str()), AnsiString(name.c_str()), value);
}

// AI(W906-ELA-W45) 20260927 (St02-E): the keys and the missing-key defaults come from JamRules.h (shared with the Handler's
//   Jam Code page, WebSecurityJamW45.h).  AI(W906-ELA-W45) 20260928 (St02-E helper): Steven 0928 18b = B -- the only
//   change: GetJemIncludeMTBF's default for 933 / 967 is true (the Handler's); Options::goldenJamMtbfDefault = golden.
//   ctest Jam_Rules section 3 compares these three getters with their pre-W45 bodies on every area x a code table x
//   four customers: equal in golden mode, and equal but for that one default under the ruling.
int JamConfig::GetJamLevel(const std::string& sJamArea, const std::string& sJamCode)
{
    int bLevel = 0;
    if (sJamArea != "" && sJamCode != "")
    {
        if (FileExistsA(opt_.jamIniPath))                         //Steven 20240820 : Fixed for off-line usage
            bLevel = CheckAndReadInt(sJamArea, jamrules::LevelKey(sJamCode), jamrules::DefaultJamLevel());   // golden 0
    }
    return bLevel;
}

bool JamConfig::GetJemIncludeMTBA(const std::string& sJamArea, const std::string& sJamCode)
{
    bool bIncludeMTBA = false;
    if (sJamArea != "" && sJamCode != "")
    {
        if (FileExistsA(opt_.jamIniPath))
            // golden Rev891 Analyzer.cpp:2820-2832: JAM + areas 01..05 -> true, else false (jamrules::DefaultIncludeMtba)
            bIncludeMTBA = CheckAndReadBool(sJamArea, jamrules::IncludeMtbaKey(sJamCode),
                                            jamrules::DefaultIncludeMtba(sJamArea, sJamCode));
    }
    return bIncludeMTBA;
}

bool JamConfig::GetJemIncludeMTBF(const std::string& sJamArea, const std::string& sJamCode)
{
    bool bIncludeMTBF = false;
    if (FileExistsA(opt_.jamIniPath))
    {
        if (sJamArea != "" && sJamCode != "")
            // golden Rev891 Analyzer.cpp:2846-2878: "24 Motor" or the 24 codes -> true, else false.  ★W45 18b (Steven
            //   0928, W20-3 = B): for CC_ASE_CL 933 / CC_TERAPOWER 967 a missing key is true (jamrules::kIncludeMtbfRule =
            //   the Handler's, 906_0625_Steven cSecurity.cpp:1338) -- a deviation from Rev891; goldenJamMtbfDefault = golden
            bIncludeMTBF = CheckAndReadBool(sJamArea, jamrules::IncludeMtbfKey(sJamCode),
                                            jamrules::DefaultIncludeMtbfAnalyzer(sJamArea, sJamCode,
                                                                                 jamrules::IsMtbfAlias(opt_.custCode),
                                                                                 opt_.goldenJamMtbfDefault
                                                                                     ? jamrules::kIncludeMtbfGoldenPerSide
                                                                                     : jamrules::kIncludeMtbfRule));
    }
    return bIncludeMTBF;
}

// ===========================================================================
//  Analyzer
// ===========================================================================
Analyzer::Analyzer(const Options& o)
    : dtStart(0.0), dtEnd(0.0), iDate(0), dupRowsSkipped(0), vecDupRowsSkipped(0), jam(o), iTop5FilterRowCount(6),
      byAreaCount(0), opt_(o)
{
    // golden ctor Analyzer.cpp:265-278: mapUnitName over the By-Area check boxes (the first eUnitNameTotal names)
    for (int i = 0; i < eUnitNameTotal; i++)
        mapUnitName[kJamAreaNames[i]] = i;
}

Row Analyzer::Split(const std::string& line) const
{
    // #15: W15 B = ela::SplitEventLogCsv (EventLogCsv.h, the viewer's splitter; it replaces the earlier CommaOnlyText,
    //   which kept the blank of ", Process" and so broke the "Process" test and the mapUnitName look-up below)
    return opt_.bcbCommaText ? BcbCommaText(line) : SplitEventLogCsv(line);
}

void Analyzer::RangeFromPickers(double startDate, double startTime, double endDate, double endTime)
{
    // GetSysDateTimeRange :3228-3233 / GetDateTime :3222-3226 (DateOf + TimeOf) / GetTotalDays :3235-3238
    const double sd = std::floor(startDate), ed = std::floor(endDate);
    dtStart = RoundToDouble(sd + (startTime - std::floor(startTime)));
    dtEnd = RoundToDouble(ed + (endTime - std::floor(endTime)));
    iDate = (int)dtEnd - (int)dtStart + 1;
}

void Analyzer::SetRange(double startDate, double startTime, double endDate, double endTime)
{
    RangeFromPickers(startDate, startTime, endDate, endTime);   // (AI(W906-ELA-R1): shared with GetEventLogTextToVec)
    const double sd = std::floor(startDate);

    // By Day: dtByDay = sDateS ("YYYY/MM/DD " -> midnight of the start date); do { key; +1 } while(cur < dtEnd)
    byDayKeys.clear();
    mapByDayList.clear();
    double cur = sd;
    do
    {
        Summary s;
        s.InitData(opt_.keepGoldenBugs);
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
        hs.InitData(opt_.keepGoldenBugs);
        hs.Date = FormatYMDH00(cur);
        byHourKeys.push_back(hs.Date);
        mapByHourList[hs.Date] = hs;
        cur = RoundToDouble(cur + (1.0 / 24.0));               // golden TDateTime variable dtCurrByHour
    } while (cur < dtEnd);

    mapAlarmSummary.clear();
    mapJamSummary.clear();
    mapWarSummary.clear();
    MySummary.InitData(opt_.keepGoldenBugs);                   // #18: golden leaves iFailCount (W18 B resets it)
    if (!opt_.keepGoldenBugs)                                  // #18 W18 B: golden never clears these two (G2)
    {
        for (int i = 0; i < eUnitNameTotal; i++) ListByUnitName[i].clear();
        for (int i = 0; i < eByFuncTotal; i++) ListByFunction[i].clear();
    }
    InitGridsForRange();                                       // P1b: SetStringGrid's grid part (ElaTables.cpp)
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }       // AnsiPos()!=0
static bool StartsWith(const std::string& s, const char* t) { return s.compare(0, std::strlen(t), t) == 0; }   // Pos()==1

namespace {
// W19 B: one file to read, and the reading order -- main folders first, then the AllEventLog copies, each by the start
// of what the name covers; ties keep the lstEventLog order.  AI(W906-ELA-W19) 20260927 (St02-E)
struct FilePick
{
    int allEventLog;
    double from, to;
    size_t index;
    std::string path;
    std::string series;                            // max-line files: the series (EventLogFileSpan), else ""
};
struct FilePickLess
{
    bool operator()(const FilePick& a, const FilePick& b) const
    {
        if (a.allEventLog != b.allEventLog) return a.allEventLog < b.allEventLog;
        if (a.from != b.from) return a.from < b.from;
        return a.index < b.index;
    }
};
struct SeriesLess                                  // files of one max-line series, by the time in the name
{
    const std::vector<FilePick>* v;
    bool operator()(size_t a, size_t b) const
    {
        const FilePick& x = (*v)[a];
        const FilePick& y = (*v)[b];
        if (x.from != y.from) return x.from < y.from;
        return x.index < y.index;
    }
};
}  // namespace

std::vector<std::string> SelectEventLogFiles(const std::vector<std::string>& lstEventLog, double dtStart, double dtEnd)
{
    // W19 B (Steven 20260927; AI(W906-ELA-W19) 20260927, St02-E): see ElaCore.h.  The VTEST (915 / 919) names are among
    //   the patterns, so there is no custCode branch here (golden's is behind Analyzer::UsesGoldenFileRule: goldenFileRule,
    //   and by default on 915 / 919 -- ★W38 B, AI(W906-ELA-W38) 20260928).
    std::vector<FilePick> all;
    std::set<std::string> listed;
    for (size_t iFile = 0; iFile < lstEventLog.size(); iFile++)
    {
        FilePick pick;
        if (!EventLogFileSpan(lstEventLog[iFile], &pick.from, &pick.to, &pick.series))
            continue;
        if (!listed.insert(lstEventLog[iFile]).second)
            continue;
        pick.allEventLog = IsAllEventLogPath(lstEventLog[iFile]) ? 1 : 0;
        pick.index = iFile;
        pick.path = lstEventLog[iFile];
        all.push_back(pick);
    }
    // a max-line file: from the previous file's time to the next file's time of its series.  Public/MyStringList.cpp
    //   :431-436 names it at the flush, so its rows are before its time; read as the start time, the same span holds
    //   too.  The first of a series is open at the start, the last at the end.  (St02-M 20260927: not two days.)
    std::map<std::string, std::vector<size_t> > series;
    for (size_t k = 0; k < all.size(); ++k)
        if (!all[k].series.empty())
            series[all[k].series].push_back(k);
    for (std::map<std::string, std::vector<size_t> >::iterator it = series.begin(); it != series.end(); ++it)
    {
        std::vector<size_t>& idx = it->second;
        SeriesLess less;
        less.v = &all;
        std::sort(idx.begin(), idx.end(), less);
        std::vector<double> named(idx.size());
        for (size_t j = 0; j < idx.size(); ++j)
            named[j] = all[idx[j]].from;
        for (size_t j = 0; j < idx.size(); ++j)
        {
            double lo = kEventLogSpanOpenStart, hi = kEventLogSpanOpenEnd;
            for (size_t k = j; k-- > 0;)
                if (named[k] < named[j]) { lo = named[k]; break; }
            for (size_t k = j + 1; k < idx.size(); ++k)
                if (named[k] > named[j]) { hi = named[k]; break; }
            all[idx[j]].from = lo;
            all[idx[j]].to = hi;
        }
    }
    std::vector<FilePick> picks;
    for (size_t k = 0; k < all.size(); ++k)
        if (all[k].from <= dtEnd && all[k].to > dtStart)
            picks.push_back(all[k]);
    std::sort(picks.begin(), picks.end(), FilePickLess());
    std::vector<std::string> out;
    for (size_t k = 0; k < picks.size(); ++k)
        out.push_back(picks[k].path);
    return out;
}

bool Analyzer::UsesGoldenFileRule() const
{
    // ★W38 B (Steven 20260928: customer-specified features keep golden's fixed formats; AI(W906-ELA-W38) 20260928,
    //   St02-E helper): the VTEST MTBF report (915 / 919) reads golden's fixed names -- Analyzer.cpp:807-811
    //   `if(sCustCode=="915" || sCustCode=="919")` compares the CUSTOMER_CODE string as read, no trim, as here
    return opt_.goldenFileRule || (opt_.goldenFileRuleVtest && (opt_.custCode == "915" || opt_.custCode == "919"));
}

void Analyzer::GetEventLogText(const std::vector<std::string>& lstEventLog)
{
    slStopList.clear();
    slJamList.clear();
    slJamWarList.clear();
    slWarList.clear();
    slAlarmList.clear();
    slFailList.clear();
    filesRead.clear();
    dupRowsSkipped = 0;

    // the files to read, in reading order
    std::vector<std::string> toRead;
    const bool golden = UsesGoldenFileRule();            // ★W38 B: goldenFileRule, or a VTEST machine by default
    if (golden)
    {
        // golden :804-821: day by day, every listed file whose path holds FN1 and FN2 -- files of another save period
        //   are never read, and the AllEventLog copy of a day is read too
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
                if (Has(lstEventLog[iFile], FN1.c_str()) && Has(lstEventLog[iFile], FN2.c_str()))
                    toRead.push_back(lstEventLog[iFile]);
        }
    }
    else
    {
        // W19 B (Steven 20260927): every event-log file whose name covers part of [dtStart, dtEnd], each once, main
        //   folders first, then AllEventLog (SelectEventLogFiles)
        toRead = SelectEventLogFiles(lstEventLog, dtStart, dtEnd);
    }

    std::map<std::string, size_t> countedIn;             // W19 B: a counted row (CommaText) -> the read it was counted in
    for (size_t iRead = 0; iRead < toRead.size(); iRead++)
    {
        const std::string& strEvent = toRead[iRead];
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
                if (!golden)
                {
                    // W19 B: a row already counted from ANOTHER file (the AllEventLog copy; the same rows in an hour
                    //   and a day file) is skipped; a repeat inside one file still counts, as golden (the Duplicate
                    //   field is the filter for an operator's repeats)
                    std::map<std::string, size_t>::iterator seen = countedIn.find(ct);
                    if (seen == countedIn.end())
                        countedIn[ct] = iRead;
                    else if (seen->second != iRead)
                    {
                        ++dupRowsSkipped;
                        continue;
                    }
                }
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

// ===========================================================================
//  R1: GetEventLogTextToVec (golden Analyzer.cpp:1183-1267, JimmyChiu 20250401 "Add for upload log")
//  AI(W906-ELA-R1) 20260927 (St02-E)
// ===========================================================================
void Analyzer::GetEventLogTextToVec(double startDate, double startTime, double endDate, double endTime,
                                    const std::vector<std::string>& lstEventLog, std::vector<LogRecord>& logRecs)
{
    RangeFromPickers(startDate, startTime, endDate, endTime);  // GetSysDateTimeRange(dtStart, dtEnd, iDate)
    GetEventLogTextToVec(dtStart, dtEnd, iDate, lstEventLog, logRecs);
}

void Analyzer::GetEventLogTextToVec(double dtstart, double dtend, int idays, const std::vector<std::string>& lstEventLog,
                                    std::vector<LogRecord>& logRecs)
{
    logRecs.clear();
    slStopList.clear();                                        // golden :1198-1203
    slJamList.clear();
    slJamWarList.clear();
    slWarList.clear();
    slAlarmList.clear();
    slFailList.clear();
    vecDupRowsSkipped = 0;

    std::vector<std::string> toRead;
    const bool golden = UsesGoldenFileRule();            // ★W38 B: the same rule as GetEventLogText (no VTEST caller)
    if (golden)
    {
        // golden :1208-1217: day by day, every listed file whose path holds "<yyyymmdd>.csv" and "EventLogTxt_" -- no
        //   VTEST (915 / 919) branch in this function, unlike GetEventLogText :807-811
        for (int d = 0; d < idays; d++)
        {
            const std::string FN1 = FormatYYYYMMDD(dtstart + d) + ".csv";
            const std::string FN2 = "EventLogTxt_";
            for (size_t iFile = 0; iFile < lstEventLog.size(); iFile++)
                if (Has(lstEventLog[iFile], FN1.c_str()) && Has(lstEventLog[iFile], FN2.c_str()))
                    toRead.push_back(lstEventLog[iFile]);
        }
    }
    else
        toRead = SelectEventLogFiles(lstEventLog, dtstart, dtend);   // W19 B, as GetEventLogText

    std::map<std::string, size_t> takenIn;                    // W19 B: a row (CommaText) -> the read it was taken in
    for (size_t iRead = 0; iRead < toRead.size(); iRead++)
    {
        const std::string& strEvent = toRead[iRead];
        if (!FileExistsA(strEvent))
            continue;
        bool ok = false;
        const std::vector<std::string> tsLogFile = BcbLoadLines(strEvent, &ok);
        for (size_t iLog = 1; iLog < tsLogFile.size(); iLog++)
        {
            const Row tsRow = Split(tsLogFile[iLog]);           // #15: the same splitter as GetEventLogText
            if ((int)tsRow.size() < elRecipe)
                continue;
            if (!IsValidDateString(tsRow[elData]))
                continue;
            if (!IsValidTimeString(tsRow[elTime]))
                continue;
            const double dtCurrent = ParseDateTime(tsRow[elData], tsRow[elTime]);
            if (dtCurrent >= dtstart && dtCurrent <= dtend)
            {
                if (Has(tsRow[elDuplicate], "1"))                //Steven 20240820 : 過濾重複操作沒解除的alarm
                    continue;
                if (!golden)
                {
                    // W19 B: a row already taken from ANOTHER file (the AllEventLog copy, an hour and a day file) is
                    //   skipped; a repeat inside one file stays, as golden
                    const std::string key = BcbGetCommaText(tsRow);
                    std::map<std::string, size_t>::iterator seen = takenIn.find(key);
                    if (seen == takenIn.end())
                        takenIn[key] = iRead;
                    else if (seen->second != iRead)
                    {
                        ++vecDupRowsSkipped;
                        continue;
                    }
                }
                LogRecord record;
                record.date = tsRow[elData];
                record.time = tsRow[elTime];
                record.unitName = tsRow[elUnit];
                record.alarmCode = tsRow[eAlarmCode];
                record.recovery = tsRow[elRecovery];
                record.stoppedTime = tsRow[elStopTime];
                record.duplicate = tsRow[elDuplicate];
                record.message = tsRow[elMessage];
                record.errorPart = tsRow[elErrorPart];
                logRecs.push_back(record);
            }
        }
    }
}

}  // namespace ela
