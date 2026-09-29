// ===========================================================================
//  EventLogAnalysis/ElaOee.cpp -- see ElaOee.h.  AI(W906-ELA-W48) 20260928 (St02-E helper): ★W48 OEE (Steven 0928 = A).
//  AI(W906-ELA-W48B) 20260928 (St02-E helper): the three rules are Steven's now (0928 09:3x: W48-1, W48-2 = B,
//  W48-3 = B) -- each one is marked where it is applied.
//  AI(W906-ELA-A8) 20260928 (St02-E helper): review finding A8 -- the Production_Log columns by the file's layout
//  (ProdLogColumns: golden header names, else CC_Greatek 956 / default by CUSTOMER_CODE); the default is unchanged.
// ===========================================================================
#include "EventLogAnalysis/ElaOee.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace ela {

OeeDay::OeeDay()
    : spanMs(0), testMs(0), downMs(0), idleMs(0), offMs(0), onMs(0), startMs(0), uncoveredMs(0), testBp(0), downBp(0),
      idleBp(0), offBp(0), touchdowns(0), stopRows(0), alarmRows(0), noData(true)
{
}

OeeResult::OeeResult()
    : now(0.0), touchdowns(0), noTestTime(0), stopRows(0), alarmRows(0), timeDataRows(0), timeDataSlackSec(kTimeDataSlackSec)
{
}

namespace {

typedef std::pair<long long, long long> Span;   // [first, second), milliseconds (DateTimeToMs)
typedef std::vector<Span> SpanSet;              // after Merge: sorted, disjoint, non-empty spans

const long long kDayMs = 86400000LL;
const long long kHourMs = 3600000LL;
// the Production_Log columns: ProdLogCols (A8; golden Analyzer.h eMyProdRec = the default layout)
// golden TimeData columns (main.cpp:1541): 0 Date, 1 Time, 2 StartTime, ... 8 PowerOnTime, ... 12 MTBA
const int kTdStartCol = 2;
const int kTdPowerOnCol = 8;
const int kTdCols = 13;

SpanSet Merge(SpanSet v)
{
    std::sort(v.begin(), v.end());
    SpanSet out;
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (v[i].second <= v[i].first)
            continue;
        if (!out.empty() && v[i].first <= out.back().second)
        {
            if (v[i].second > out.back().second)
                out.back().second = v[i].second;
        }
        else
            out.push_back(v[i]);
    }
    return out;
}

// a minus b (both merged)
SpanSet Subtract(const SpanSet& a, const SpanSet& b)
{
    SpanSet out;
    size_t j = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        long long lo = a[i].first;
        const long long hi = a[i].second;
        while (j < b.size() && b[j].second <= lo)
            ++j;
        for (size_t k = j; lo < hi && k < b.size() && b[k].first < hi; ++k)
        {
            if (b[k].first > lo)
                out.push_back(Span(lo, b[k].first));
            if (b[k].second > lo)
                lo = b[k].second;
        }
        if (lo < hi)
            out.push_back(Span(lo, hi));
    }
    return out;
}

SpanSet Union(const SpanSet& a, const SpanSet& b)
{
    SpanSet v(a);
    v.insert(v.end(), b.begin(), b.end());
    return Merge(v);
}

struct EndAfter                                 // upper_bound: the first span whose end is past v
{
    bool operator()(long long v, const Span& s) const { return v < s.second; }
};

// |s intersect [lo, hi)|
long long Measure(const SpanSet& s, long long lo, long long hi)
{
    if (hi <= lo)
        return 0;
    long long sum = 0;
    for (SpanSet::const_iterator it = std::upper_bound(s.begin(), s.end(), lo, EndAfter());
         it != s.end() && it->first < hi; ++it)
        sum += std::min(hi, it->second) - std::max(lo, it->first);
    return sum;
}

// how many of the sorted times lie in [lo, hi)
int CountIn(const std::vector<long long>& sorted, long long lo, long long hi)
{
    if (hi <= lo)
        return 0;
    return (int)(std::lower_bound(sorted.begin(), sorted.end(), hi) - std::lower_bound(sorted.begin(), sorted.end(), lo));
}

std::string BlankFromUnderscore(std::string s)
{
    // ListProductionLog turned the blanks of an unquoted line into '_' (golden :2335) and turns them back before
    // StrToDateTime (golden :2352); the same here
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] == '_')
            s[i] = ' ';
    return s;
}

std::string TrimBlanks(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && (unsigned char)s[b] <= ' ')
        ++b;
    while (e > b && (unsigned char)s[e - 1] <= ' ')
        --e;
    return s.substr(b, e - b);
}

// a Production_Log date-time field ("2026/04/09 14:56:42", blanks maybe '_') -> TDateTime
bool ParseStamp(const std::string& field, double* out)
{
    const std::string s = TrimBlanks(BlankFromUnderscore(field));
    return !s.empty() && StrToDateTimeZhTw(s, out) && *out >= 1.0;
}

// Test Time ("00:00:00.029", golden :2401-2437 reads it as a time of day + milliseconds) -> ms; unreadable = 0
long long TestTimeMs(const std::string& field)
{
    const std::string s = TrimBlanks(BlankFromUnderscore(field));
    double v = 0.0;
    if (s.empty() || !StrToDateTimeZhTw(s, &v) || v < 0.0 || v >= 1.0)
        return 0;
    return DateTimeToMs(v);
}

// a kept stop row (CommaText) -> its time and StopedTime in ms; false = unreadable (cannot happen for a kept row:
// GetEventLogText checked the date, the time and the field count)
bool StopRowSpan(const std::string& commaText, long long* t, long long* len)
{
    const Row f = BcbCommaText(commaText);
    if ((int)f.size() <= elStopTime)
        return false;
    const double dt = ParseDateTime(f[elData], f[elTime]);
    if (dt == 0.0)
        return false;
    *t = DateTimeToMs(dt);
    *len = (long long)std::atoi(f[elStopTime].c_str()) * 1000LL;   // golden atoi (ElaCore.cpp GetEventLogText)
    if (*len < 0)
        *len = 0;                                       // (golden adds a negative StopedTime to Total Stop Time; no span here)
    return true;
}

// an integer field ("3600", " -12") -> *out; false = not an integer
bool IntField(const std::string& field, long long* out)
{
    const std::string s = TrimBlanks(field);
    const size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
    if (i >= s.size() || s.size() - i > 15)
        return false;
    long long v = 0;
    for (size_t k = i; k < s.size(); ++k)
    {
        if (!std::isdigit((unsigned char)s[k]))
            return false;
        v = v * 10 + (s[k] - '0');
    }
    *out = i ? -v : v;
    return true;
}

bool ByAt(const TimeDataRow& x, const TimeDataRow& y)
{
    if (x.at != y.at)
        return x.at < y.at;
    if (x.onMs != y.onMs)
        return x.onMs < y.onMs;
    return x.startMs < y.startMs;
}

bool SameRow(const TimeDataRow& x, const TimeDataRow& y)
{
    return DateTimeToMs(x.at) == DateTimeToMs(y.at) && x.onMs == y.onMs && x.startMs == y.startMs;
}

// MinGW.org msvcrt printf has no %lld (ElaFtp.cpp I64): long long by hand
std::string Int(long long v)
{
    if (v == 0)
        return "0";
    const bool neg = v < 0;
    unsigned long long u = neg ? 0ull - (unsigned long long)v : (unsigned long long)v;
    char b[24];
    int i = (int)sizeof(b);
    b[--i] = 0;
    while (u)
    {
        b[--i] = (char)('0' + (int)(u % 10));
        u /= 10;
    }
    if (neg)
        b[--i] = '-';
    return std::string(&b[i]);
}

std::string SecText(long long ms)
{
    // seconds with up to three decimals, no trailing zeros ("64800", "1.8"); ms >= 0 here
    const long long frac = ms % 1000;
    if (frac == 0)
        return Int(ms / 1000);
    char b[8];
    std::snprintf(b, sizeof(b), "%03d", (int)frac);
    size_t n = std::strlen(b);
    while (n > 0 && b[n - 1] == '0')
        b[--n] = '\0';
    return Int(ms / 1000) + "." + b;
}

std::string PctText(int bp)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%d.%02d", bp / 100, bp % 100);
    return b;
}

// JSON string body: '\\', '"' and control bytes escaped; a non-ASCII byte becomes '?' (a cp950 path is not UTF-8)
std::string JsonText(const std::string& s)
{
    std::string o;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = (unsigned char)s[i];
        if (c == '\\' || c == '"')
        {
            o += '\\';
            o += (char)c;
        }
        else if (c < 0x20)
        {
            char b[8];
            std::snprintf(b, sizeof(b), "\\u%04x", (unsigned)c);
            o += b;
        }
        else if (c >= 0x80)
            o += '?';
        else
            o += (char)c;
    }
    return o;
}

// half-up rounding of part / whole to hundredths of a percent (0 <= part <= whole, whole > 0)
int Bp(long long part, long long whole)
{
    return (int)((part * 20000LL + whole) / (whole * 2LL));
}

// the part of `startMs`, spread evenly over [lo0, hi0), that falls in [lo, hi)
long long Spread(long long startMs, long long lo0, long long hi0, long long lo, long long hi)
{
    const long long a = std::max(lo, lo0), b = std::min(hi, hi0);
    if (b <= a || hi0 <= lo0 || startMs <= 0)
        return 0;
    return (long long)std::floor((double)startMs * (double)(b - a) / (double)(hi0 - lo0) + 0.5);
}

}  // namespace

// ---- A8: the Production_Log layout (906_0625_Steven Public/MyProductionRecord.cpp; see ElaOee.h) ----
ProdLogCols::ProdLogCols()
    : inTime(eLoadTime), armTime(12), order(eOrderTest), sot(14), testTime(eTestTime), eot(29)
{
}

const char* const kCustCodeGreatek = "956";

ProdLogCols GreatekProdLogCols()
{
    // SaveDataForGreatek (:851-887) writes asBuffer->Strings[iSortData[i]] (:858) under asDataTitleGreatek (:46-88)
    ProdLogCols c;
    c.inTime = 6;                                   // eLoadTime  (:52 "In Time")
    c.armTime = 18;                                 // eArmTime   (:64 "Arm Time")
    c.order = 19;                                   // eOrderTest (:65 "Order of testing")
    c.sot = 20;                                     // eSOTTime   (:66 "SOT time stamp")
    c.testTime = 37;                                // eTestTime  (:83 "Test Time"; col 27 = Out Arm Shuttle Pick :73)
    c.eot = 39;                                     // eEOTTime   (:85 "EOT time stamp")
    return c;
}

ProdLogCols ProdLogColumns(const std::string& header, const std::string& custCode)
{
    ProdLogCols c = (custCode == kCustCodeGreatek) ? GreatekProdLogCols() : ProdLogCols();
    // golden's names, in both headers (default asDataTitle :97 / :104-106 / :119 / :121, Greatek the lines above)
    static const char* const kName[6] = { "In Time", "Arm Time", "Order of testing", "SOT time stamp", "Test Time",
                                          "EOT time stamp" };
    int* const col[6] = { &c.inTime, &c.armTime, &c.order, &c.sot, &c.testTime, &c.eot };
    const Row f = BcbCommaText(header);
    for (int k = 0; k < 6; ++k)
        for (size_t i = 0; i < f.size(); ++i)
            if (TrimBlanks(BlankFromUnderscore(f[i])) == kName[k])
            {
                *col[k] = (int)i;
                break;
            }
    return c;
}

bool ParseSotStamp(const std::string& in, double* out)
{
    const std::string s = TrimBlanks(in);
    if (s.size() != 15 || (s[8] != '_' && s[8] != ' '))
        return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (i != 8 && !std::isdigit((unsigned char)s[i]))
            return false;
    // "yyyymmdd_hhmmss" -> "yyyy/mm/dd hh:mm:ss" and the zh-TW StrToDateTime (it checks the day of the month and the time)
    const std::string t = s.substr(0, 4) + "/" + s.substr(4, 2) + "/" + s.substr(6, 2) + " " + s.substr(9, 2) + ":" +
                          s.substr(11, 2) + ":" + s.substr(13, 2);
    double v = 0.0;
    if (!StrToDateTimeZhTw(t, &v) || v < 1.0)
        return false;
    *out = v;
    return true;
}

bool ParseTimeDataLine(const std::string& line, TimeDataRow* out)
{
    // golden AddTextWithDateTime: "%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s" (SIGURD: no blank after the commas) + the
    // CommaText of RecordTimeData's "%i,%i,%i,%i,%i,%i,%i,%i,%i,%d, %d" (906_0625_Steven cMyDB.cpp:380-391)
    std::vector<std::string> f;
    size_t p = 0;
    for (;;)
    {
        const size_t c = line.find(',', p);
        f.push_back(TrimBlanks(line.substr(p, c == std::string::npos ? std::string::npos : c - p)));
        if (c == std::string::npos)
            break;
        p = c + 1;
    }
    if ((int)f.size() < kTdCols)
        return false;
    const std::string& d = f[0];
    if (d.size() != 10 || d[4] != '-' || d[7] != '-')
        return false;                                   // the header row ("Date") and anything else
    for (size_t i = 0; i < d.size(); ++i)
        if (i != 4 && i != 7 && !std::isdigit((unsigned char)d[i]))
            return false;
    double at = 0.0;
    if (!StrToDateTimeZhTw(d.substr(0, 4) + "/" + d.substr(5, 2) + "/" + d.substr(8, 2) + " " + f[1], &at) || at < 1.0)
        return false;
    long long start = 0, on = 0;
    if (!IntField(f[kTdStartCol], &start) || !IntField(f[kTdPowerOnCol], &on))
        return false;
    out->at = at;
    out->onMs = on > 0 ? on * 1000LL : 0;               // a negative counter (golden's 32-bit ms overflow) = nothing
    out->startMs = start > 0 ? start * 1000LL : 0;
    return true;
}

std::string DefaultTimeDataDir()
{
    const char* e = std::getenv("W906_HT9045LOG_ROOT");
    return ((e != 0 && *e != 0) ? std::string(e) : std::string("D:\\HT9045_Log")) + "\\TimeData";
}

std::vector<TimeDataRow> ReadTimeData(const std::string& dir, double dtFrom, double dtTo, std::vector<std::string>* files)
{
    std::vector<TimeDataRow> rows;
    if (files)
        files->clear();
    if (dir.empty())
        return rows;
    int y0, y1, m, d, h, n, s, ms;
    DecodeDateTime(dtFrom, &y0, &m, &d, &h, &n, &s, &ms);
    DecodeDateTime(dtTo, &y1, &m, &d, &h, &n, &s, &ms);
    if (y1 < y0)
        std::swap(y0, y1);
    if (y1 - y0 > 20)
        y0 = y1 - 20;                                   // (a query range is days to months; a guard, not a rule)
    for (int y = y0 - 1; y <= y1 + 1; ++y)
    {
        char b[64];
        std::snprintf(b, sizeof(b), "%04d", y);
        const std::string name = std::string("TimeData_") + b + ".csv";
        const std::string paths[2] = { dir + "\\" + b + "\\" + name, dir + "\\" + name };
        for (int i = 0; i < 2; ++i)
        {
            if (!FileExistsA(paths[i]))
                continue;
            bool ok = false;
            const std::vector<std::string> lines = BcbLoadLines(paths[i], &ok);
            if (!ok)
                continue;
            if (files)
                files->push_back(paths[i]);
            for (size_t k = 0; k < lines.size(); ++k)
            {
                TimeDataRow r;
                if (ParseTimeDataLine(lines[k], &r))
                    rows.push_back(r);
            }
        }
    }
    std::sort(rows.begin(), rows.end(), ByAt);
    rows.erase(std::unique(rows.begin(), rows.end(), SameRow), rows.end());
    return rows;
}

OeeResult ComputeOee(const Analyzer& a, const std::vector<TimeDataRow>& td, double now, const OeeOptions& opt)
{
    OeeResult r;
    r.now = now;
    r.timeDataSlackSec = opt.timeDataSlackSec > 0 ? opt.timeDataSlackSec : 0;
    const long long slackMs = (long long)r.timeDataSlackSec * 1000LL;
    const long long nowMs = DateTimeToMs(now);

    // ---- touchdowns (W48-1, Steven 0928 09:3x): [t(k), t(k) + Test Time(k)), col 27 (A8: Greatek col 37) ----
    // A8: the columns by the header slot's golden names, else by the customer (default 5 / 12 / 13 / 14 / 27 / 29)
    const ProdLogCols c = ProdLogColumns(a.slAllData.empty() ? std::string() : a.slAllData[0], a.options().custCode);
    r.cols = c;
    std::vector<std::pair<long long, long long> > touch;   // (t(k), Test Time(k)) in ms
    int prevOrder = -1;                                     // golden ListProductionLog iOrderTest starts at -1
    for (size_t i = 1; i < a.slAllData.size(); ++i)         // element 0 = the header slot
    {
        const Row f = BcbCommaText(a.slAllData[i]);
        const int n = (int)f.size();
        if (n <= c.inTime)
            continue;
        const int order = (n > c.order) ? std::atoi(f[c.order].c_str()) : prevOrder;
        if (order == prevOrder)
            continue;                                       // another site / IC of the same touchdown
        prevOrder = order;
        const long long tt = (n > c.testTime) ? TestTimeMs(f[c.testTime]) : 0;
        // the start: SOT (RecordStartTestTime's own stamp), else EOT - Test Time, else Arm Time, else In Time
        double t = 0.0, eot = 0.0;
        long long start = 0;
        if (n > c.sot && ParseSotStamp(f[c.sot], &t))
            start = DateTimeToMs(t);
        else if (n > c.eot && ParseSotStamp(f[c.eot], &eot))
            start = DateTimeToMs(eot) - tt;
        else if ((n > c.armTime && ParseStamp(f[c.armTime], &t)) || ParseStamp(f[c.inTime], &t))
            start = DateTimeToMs(t);
        else
            continue;
        touch.push_back(std::make_pair(start, tt));
    }
    std::sort(touch.begin(), touch.end());
    SpanSet run;                                            // the TestSet
    std::vector<long long> touchTimes;
    for (size_t k = 0; k < touch.size(); ++k)
    {
        touchTimes.push_back(touch[k].first);
        if (touch[k].second > 0)
            run.push_back(Span(touch[k].first, touch[k].first + touch[k].second));
        else
            ++r.noTestTime;
    }
    r.touchdowns = (int)touch.size();

    // ---- stop rows: every kept row proves the software ran and is counted per day; only the MTBA rows are down ----
    // W48-3 = B (Steven 0928 09:3x): down = [t, t + StopedTime) over slAlarmList, the rows JamConfig::GetJemIncludeMTBA
    //   counts (golden Alarm Time, Analyzer.cpp:883-899)
    SpanSet down, allStop;
    std::vector<long long> stopTimes, alarmTimes;
    for (size_t i = 0; i < a.slStopList.size(); ++i)
    {
        long long t = 0, len = 0;
        if (!StopRowSpan(a.slStopList[i], &t, &len))
            continue;
        stopTimes.push_back(t);
        allStop.push_back(Span(t, t + len));
    }
    for (size_t i = 0; i < a.slAlarmList.size(); ++i)
    {
        long long t = 0, len = 0;
        if (!StopRowSpan(a.slAlarmList[i], &t, &len))
            continue;
        alarmTimes.push_back(t);
        down.push_back(Span(t, t + len));
    }
    std::sort(stopTimes.begin(), stopTimes.end());
    std::sort(alarmTimes.begin(), alarmTimes.end());
    r.stopRows = (int)stopTimes.size();
    r.alarmRows = (int)alarmTimes.size();

    // ---- TimeData (W48-2 = B, Steven 0928 09:3x): the software-on time ----
    std::vector<TimeDataRow> rows(td);
    std::sort(rows.begin(), rows.end(), ByAt);
    rows.erase(std::unique(rows.begin(), rows.end(), SameRow), rows.end());
    SpanSet on, covered;
    std::vector<std::pair<Span, long long> > starts;        // (row k's on span, its StartTime), in time order
    long long prevAt = 0, firstLo = 0;
    for (size_t k = 0; k < rows.size(); ++k)
    {
        const long long at = DateTimeToMs(rows[k].at);
        const long long p = rows[k].onMs;
        long long lo = 0;
        if (k == 0)
            lo = at - std::min(p, kHourMs + slackMs);       // nothing before it: at most the hour it closes
        else if (at - prevAt - p <= slackMs)
            lo = prevAt;                                    // on for the whole time since the previous record
        else
            lo = at - p;                                    // a gap: the recorded on time, put at its end
        if (k == 0)
            firstLo = lo;
        if (at > lo)
        {
            on.push_back(Span(lo, at));
            starts.push_back(std::make_pair(Span(lo, at), rows[k].startMs));
        }
        prevAt = at;
    }
    if (!rows.empty())
    {
        covered.push_back(Span(firstLo, prevAt));
        if (nowMs > prevAt && nowMs - prevAt <= kHourMs + slackMs)
        {
            on.push_back(Span(prevAt, nowMs));              // the hour being recorded now (the Handler runs this)
            covered.push_back(Span(prevAt, nowMs));
        }
    }
    r.timeDataRows = (int)rows.size();

    down = Merge(down);
    const SpanSet runOnly = Subtract(Merge(run), down);    // down wins over test
    on = Merge(on);
    covered = Merge(covered);
    // off = covered, minus the on time, minus every test and every kept stop row (they prove the software ran)
    const SpanSet off = Subtract(covered, Union(Union(on, Merge(run)), Merge(allStop)));

    // ---- per day ----
    const long long fromMs = DateTimeToMs(a.dtStart);
    const long long toMs = DateTimeToMs(a.dtEnd);
    const long long toEnd = toMs - (((toMs % 1000) + 1000) % 1000) + 1000;   // the end of the To second (it is included)
    const long long endMs = std::min(toEnd, nowMs);
    for (size_t i = 0; i < a.byDayKeys.size(); ++i)
    {
        OeeDay d;
        d.date = a.byDayKeys[i];
        long long coveredMs = 0;
        double day = 0.0;
        if (StrToDateTimeZhTw(d.date, &day))
        {
            const long long dayMs = DateTimeToMs(std::floor(day));
            const long long lo = std::max(dayMs, fromMs);
            const long long hi = std::min(dayMs + kDayMs, endMs);
            if (hi > lo)
            {
                d.spanMs = hi - lo;
                d.testMs = Measure(runOnly, lo, hi);
                d.downMs = Measure(down, lo, hi);
                d.offMs = Measure(off, lo, hi);
                d.onMs = Measure(on, lo, hi);
                coveredMs = Measure(covered, lo, hi);
                d.uncoveredMs = d.spanMs - coveredMs;
                for (size_t k = 0; k < starts.size(); ++k)
                    d.startMs += Spread(starts[k].second, starts[k].first.first, starts[k].first.second, lo, hi);
                d.touchdowns = CountIn(touchTimes, lo, hi);
                d.stopRows = CountIn(stopTimes, lo, hi);
                d.alarmRows = CountIn(alarmTimes, lo, hi);
            }
        }
        d.noData = d.touchdowns == 0 && d.stopRows == 0 && d.testMs == 0 && d.downMs == 0 && coveredMs == 0;
        if (d.noData)
        {
            d.offMs = d.onMs = d.startMs = d.uncoveredMs = 0;   // nothing is claimed
        }
        else
        {
            // W48-2 = B: idle = the rest (software on, no test, no counted alarm; plus the time no TimeData covers)
            d.idleMs = d.spanMs - d.testMs - d.downMs - d.offMs;
            d.testBp = Bp(d.testMs, d.spanMs);
            d.downBp = Bp(d.downMs, d.spanMs);
            d.offBp = Bp(d.offMs, d.spanMs);
            d.idleBp = 10000 - d.testBp - d.downBp - d.offBp;
            int* const takeFrom[3] = { &d.offBp, &d.downBp, &d.testBp };
            for (int k = 0; k < 3 && d.idleBp < 0; ++k)     // rounded up past 100.00 (idle ~ 0): take it from off, down
            {
                const int t = std::min(*takeFrom[k], -d.idleBp);
                *takeFrom[k] -= t;
                d.idleBp += t;
            }
        }
        r.days.push_back(d);
    }
    return r;
}

std::string OeeJson(const OeeResult& r, const std::string& timeDataDir)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(r.now, &y, &m, &d, &h, &n, &s, &ms);
    char nowText[40];
    std::snprintf(nowText, sizeof(nowText), "%04d/%02d/%02d %02d:%02d:%02d", y, m, d, h, n, s);
    // rule: which rules produced the numbers (the page prints them) -- all three Steven's (0928 09:3x)
    // (A8: w481 names the Test Time column used -- "col27" by default, "col37" on a Greatek file)
    std::string o = "{\"rule\":{\"test\":\"touchdownTestTime\",\"w481\":\"col" + Int(r.cols.testTime) +
                    "\",\"down\":\"mtbaAlarmsUnion\","
                    "\"w483\":\"B\",\"off\":\"timeDataPowerOn\",\"w482\":\"B\",\"idle\":\"spanMinusTestDownOff\","
                    "\"ruled\":\"Steven 0928 09:3x\",\"toSecondIncluded\":true,\"now\":\"" + std::string(nowText) +
                    "\",\"touchdowns\":" + Int(r.touchdowns) + ",\"noTestTime\":" + Int(r.noTestTime) +
                    ",\"stopRows\":" + Int(r.stopRows) + ",\"alarmRows\":" + Int(r.alarmRows) +
                    ",\"timeDataRows\":" + Int(r.timeDataRows) + ",\"timeDataSlackSec\":" + Int(r.timeDataSlackSec) +
                    ",\"timeDataDir\":\"" + JsonText(timeDataDir) + "\"},\"days\":[";
    for (size_t i = 0; i < r.days.size(); ++i)
    {
        const OeeDay& x = r.days[i];
        if (i)
            o += ',';
        // x.date is a By Day key (digits and '/'), no escaping needed
        o += "{\"date\":\"" + x.date + "\",\"spanSec\":" + SecText(x.spanMs) + ",\"testSec\":" + SecText(x.testMs) +
             ",\"downSec\":" + SecText(x.downMs) + ",\"idleSec\":" + SecText(x.idleMs) + ",\"offSec\":" + SecText(x.offMs) +
             ",\"testPct\":" + PctText(x.testBp) + ",\"downPct\":" + PctText(x.downBp) + ",\"idlePct\":" +
             PctText(x.idleBp) + ",\"offPct\":" + PctText(x.offBp) + ",\"onSec\":" + SecText(x.onMs) + ",\"startSec\":" +
             SecText(x.startMs) + ",\"uncoveredSec\":" + SecText(x.uncoveredMs) + ",\"touchdowns\":" + Int(x.touchdowns) +
             ",\"stopRows\":" + Int(x.stopRows) + ",\"alarmRows\":" + Int(x.alarmRows) + ",\"noData\":" +
             (x.noData ? "true" : "false") + "}";
    }
    return o + "]}";
}

}  // namespace ela
