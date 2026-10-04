// =============================================================================
//  test_ela_oee.cpp -- ★W48 OEE chart: ElaOee ComputeOee / OeeJson / ReadTimeData (EventLogAnalysis/ElaOee.h).
//
//  AI(W906-ELA-W48) 20260928 (St02-E helper).  Suite name (add_test): ELA_Oee
//  AI(W906-ELA-W48B) 20260928 (St02-E helper): Steven 0928 09:3x ruled the three rules (they replaced the St02
//  defaults Q1 = B / Q2 = A / Q3 = A of the first version):
//    W48-1      test = per touchdown [start, start + Test Time) -- golden col 27 is Steven's 0x41 .. BIN ON + ECHO OK;
//               start = SOT, else EOT - Test Time, else Arm Time, else In Time; a union, minus down (no gap joining);
//    W48-2 = B  off = the time TimeData covers but the software was not on (PowerOnTime per hourly row), minus any test
//               or alarm row; idle = span - test - down - off (time without TimeData stays idle);
//    W48-3 = B  down = the stop rows JamConfig::GetJemIncludeMTBA counts (slAlarmList, golden Alarm Time),
//               [t, t + StopedTime), a union cut at midnight.  The sandbox JAM0000.dat unchecks JAM0302 and checks
//               MES1641; every other code takes golden's default ("JAM" and area 01..05).
//  0. containment first: refuses (exit 2) unless ctest's redirect root W906_HT9045LOG_ROOT is the machine_log_scratch
//  sandbox (tests/CMakeLists.txt _ht9045_env_extra) -- this test opens nothing outside its own %TEMP% folder, but it
//  is not meant to be run by hand.  The logs are GENERATED here (never copied from D:\HT9045_Log), under
//  %TEMP%\ht9045_ela_oee (refused under D:\HT9045*, D:\RMS, D:\MTBF_Summary), removed on a green run.
//  Query through BtnQuery (the Hub's path): 2026/05/01 00:00:00 .. 2026/05/05 23:59:59, now = 2026/05/05 12:00:00.
//  Two Production_Log rows (sites 1 / 2, one Order of testing) per touchdown.  TimeData (71 hourly rows at hh:00:04,
//  PowerOnTime 3600 s): on from 04/30 22:00:04 to 05/02 12:00:04; off, then the 05/02 21:00:04 row carries 5400 s (30
//  min before the shutdown + 1 h after the restart, placed at its end: on from 19:30:04); on to 05/04 00:00:04; off;
//  on again from 05/05 06:00:04 (row 07:00:04) to the last row 11:00:04 and on to now (the hour being recorded).
//
//    day    fixture                                                     span   test  down   idle    off  test% down% idle%  off%
//    05/01  touchdowns every 30 s 04:00:00-09:59:30 and 11:00:00-
//           22:59:30, Test Time 30 s; JAM 10:00 3600 s; MES2110 04:00
//           0 s; WAR Duplicate=1 15:00 999 s (ignored) -- Steven's
//           18 / 1 / 5 h; on all day                                   86400  64800  3600  18000      0  75.00  4.17 20.83  0.00
//    05/02  touchdowns 08:00:00, 08:05:00, 08:10:01, Test Time 2 s
//           each (W48-1: 6 s, the gaps are idle); JAM0105 13:00 120 s
//           (inside the off time: it proves the software ran); JAM
//           23:50 1200 s split 600 + 600; off 12:00:04-19:30:04        86400      6   720  58794  26880   0.01  0.83 68.05 31.11
//    05/03  the 600 s carried over; JAM 10:00 600 s + JAM 10:05 600 s
//           (union 900, not 1200); 31 touchdowns every 60 s 09:50-
//           10:20, Test Time 2 s, 15 of them inside the stop (down
//           wins); W48-3: WAR0301 300 s, MES1640 One cycle finish 600
//           s, JAM0601 (area 06) 100 s and the unchecked JAM0302 300 s
//           are idle, the checked MES1641 200 s is down; on all day    86400     32  1700  84668      0   0.04  1.97 97.99  0.00
//    05/04  no log rows; on 00:00:00-00:00:04 only -> off (was noData
//           before W48-2)                                              86400      0     0      4  86396   0.00  0.00  0.00 100.00
//    05/05  touchdowns every 30 s 08:00:00-09:59:30, Test Time 30 s;
//           JAM 11:50 1200 s cut at now 12:00; off until 06:00:04;
//           W61: the 07:00:04 row HomeTime 600 s, the 08:00:04 row
//           ContactTestTime 900 s (both inside StartTime) -> production
//           8700 = test 7200 + contact 900 + home 600, 20.14 %          43200   7200   600  12296  21604  16.67  1.39 28.46 50.01
//  W61 (AI(W906-ELA-W61) 20260929, St02-E helper; ★W61 = B, Steven: every SystemStart second is production, split into
//  test / contact test / home / the rest; no test and no SystemStart = idle): production = test + min(max(StartTime -
//  motor-alarm down - test, 0), span - test - down - off); on 05/01 (StartTime 3604 s < test) production = test, so
//  05/01-05/04 keep their W48 numbers (prod% = test%); only 05/05 changes (idle 13796 -> 12296 s, 31.93 -> 28.46 %).
//  Section 9 = the focused W61 cases (05/09).  The new numbers come from an independent Python model of the arithmetic.
//  Also: ParseSotStamp; ParseTimeDataLine / ReadTimeData (both file names, the SIGURD form, a duplicate, the header, a
//  bad line); onSec / startSec / uncoveredSec; without TimeData = the W48-1 / W48-3 numbers with no off (05/04 noData);
//  an overdue tail (now 12:01:10) is uncovered, not off; a first row carrying days of PowerOn covers one hour at most;
//  golden By Day Total Stop / Alarm Time stay the unclipped own-day sums; now before the range = span 0 / noData; a
//  one-day query with To 23:59:59 has a whole-day span; the JSON text; 7. W48-1's start without SOT.
//  8. A8 (review finding, St02's call; AI(W906-ELA-A8) 20260928, St02-E helper): the Production_Log layout
//  (906_0625_Steven Public/MyProductionRecord.cpp).  CC_Greatek (956) writes SaveDataForGreatek's order (:858, header
//  :46-88): Test Time col 37 (col 27 = Out Arm Shuttle Pick), Order 19, SOT 20, EOT 39, Arm 18, In 6.  The same five
//  touchdowns in both layouts give the same numbers (05/06 15 s / 2, 05/07 22 s / 3); ProdLogColumns (header names win,
//  else CUSTOMER_CODE "956", else default); the old misread; the default untouched; and the KNOWN GAP upstream --
//  ListProductionLog (golden col 5 In Time) keeps no Greatek row.  Numbers from an independent Python model.
// =============================================================================
#include "EventLogAnalysis/ElaOee.h"
#include "EventLogAnalysis/ElaHub.h"   // QueryRequest + BtnQuery: the query path the Hub runs before PublishSnapshot
#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> g_files, g_dirs;   // made by this test; removed on a green run

static void MakeDir(const std::string& p)
{
    ::CreateDirectoryA(p.c_str(), 0);
    g_dirs.push_back(p);
}

static void WriteText(const std::string& p, const std::string& text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f)
    {
        std::fwrite(text.data(), 1, text.size(), f);
        std::fclose(f);
    }
    g_files.push_back(p);
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

static std::string Lower(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) s[i] = (s[i] == '/') ? '\\' : (char)std::tolower((unsigned char)s[i]);
    return s;
}

// refuse-first (same list as tests/test_ela_hub.cpp): the sandbox may not resolve into the machine's data
static bool MachineDataPath(const std::string& p)
{
    const std::string s = Lower(p);
    static const char* const k[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary" };   // d:\ht9045 also covers d:\ht9045_log
    for (size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
        if (s.compare(0, std::strlen(k[i]), k[i]) == 0) return true;
    return false;
}

// ---- generated logs (May 2026) ----
static std::string Stamp(int day, long sec)          // "2026/05/dd hh:mm:ss"
{
    char b[32];
    std::snprintf(b, sizeof(b), "2026/05/%02d %02ld:%02ld:%02ld", day, sec / 3600, sec / 60 % 60, sec % 60);
    return b;
}

static std::string Sot(int day, long sec)            // Production_Log SOT "yyyymmdd_hhmmss"
{
    char b[32];
    std::snprintf(b, sizeof(b), "202605%02d_%02ld%02ld%02ld", day, sec / 3600, sec / 60 % 60, sec % 60);
    return b;
}

static const char kProdHeader[] =
    "Schedule name, Start time, Input tray, In X, In Y, In Time, Hot X, Hot Y, Hot Time, 2DCode, Site No, Arm No, "
    "Arm Time, Order of testing, SOT time stamp, Index cycle time, Test category, iWhichAuto, Output tray, Out X, Out Y, "
    "Out Time, Out Arm X Pos, Out Arm Y Pos, Out Arm Time, Error log, OCR Code , Test Time, TSD Time,   EOT time stamp, "
    "In Arm Loader Pick\r\n";
static const char kEvHeader[] = "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n";
// golden main.cpp:1541 (V906 LogObjects.cpp:138); the Handler's MySaveToFile writes "\n" line ends
static const char kTdHeader[] = "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, "
                                "PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA\n";

static int g_order = 0;                              // Order of testing, one per touchdown

// one touchdown at SOT `sot` (seconds of the day) = two rows in the shape of D:\HT9045_Log\Production_Log (field 5 In
// Time, 12 Arm Time, 13 Order of testing, 14 SOT, 27 Test Time)
static void AddTouchdown(std::string* csv, int day, long sot, const char* testTime)
{
    ++g_order;
    for (int site = 1; site <= 2; ++site)
    {
        char b[640];
        std::snprintf(b, sizeof(b),
                      ",202605%02d,I1-1,%d,0,\"%s\",0,0,\"%s\",,%d,1,\"%s\",%d,%s,\" 5.630\",1,1,O1-13,0,0,\"%s\",-46288,"
                      "-72427,\"%s\",na,na,%s,,%s,A\r\n",
                      day, site - 1, Stamp(day, sot - 40).c_str(), Stamp(day, sot - 39).c_str(), site,
                      Stamp(day, sot - 5).c_str(), g_order, Sot(day, sot).c_str(), Stamp(day, sot + 20).c_str(),
                      Stamp(day, sot + 20).c_str(), testTime, Sot(day, sot).c_str());
        *csv += b;
    }
}

// one Production_Log row as ListProductionLog keeps it (CommaText; 31 fields), for section 7
static std::string ProdRow(int order, const char* sot, const char* eot, const char* armTime, const char* testTime)
{
    std::vector<std::string> f(31, "0");
    char b[16];
    std::snprintf(b, sizeof(b), "%d", order);
    f[5] = "\"2026/05/06 09:00:00\"";                 // In Time
    f[12] = armTime;                                    // Arm Time
    f[13] = b;                                          // Order of testing
    f[14] = sot;                                        // SOT time stamp
    f[27] = testTime;                                   // Test Time
    f[29] = eot;                                        // EOT time stamp
    std::string s;
    for (size_t i = 0; i < f.size(); ++i)
        s += (i ? "," : "") + f[i];
    return s;
}

// ---- 8. A8 (AI(W906-ELA-A8) 20260928): the Production_Log layout, 906_0625_Steven Public/MyProductionRecord.cpp ----
// golden asDataTitleGreatek (:46-88), byte for byte (checked against the source by the independent Python model)
static const char kGreatekHeader[] =
    "Schedule name, Start time, Input tray, Tray Form, In X, In Y, In Time, In Arm Loader Pick, Hotplate No,Hotplate "
    "Form, Hot X, Hot Y, Hot Time, In Arm Hotplate Pick, 2DCode, In Arm Place Shuttle Site No, Arm No, Index Pick "
    "Shuttle Site No, Arm Time, Order of testing, SOT time stamp, Index cycle time, Test category, Test Mode, Index "
    "Place Shuttle Site No,iWhichAuto, Out Shuttle Detect Site No,Out Arm Shuttle Pick, Output tray, Out X, Out Y, Out "
    "Time, Out Arm X Pos, Out Arm Y Pos, Out Arm Time, Error log, OCR Code , Test Time, TSD Time,   EOT time stamp,Out "
    "Arm X pitch Pos, Out Arm X pitch2 Pos, Out Arm Y pitch Pos, ";
// SaveDataForGreatek iSortData (:858): 43 values in an array of eDataTotal = 71 (MyProductionRecord.h:156) -- [43..70]
// are 0, i.e. the schedule name again
static const int kGreatekSort[43] = { 0, 1, 2, 41, 3, 4, 5, 30, 35, 42, 6, 7, 8, 36, 9, 10, 11, 31, 12, 13, 14, 15,
                                      16, 37, 32, 17, 33, 34, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 38, 39, 40 };

static std::string Underscore(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] == ' ') s[i] = '_';
    return s;
}

// what ListProductionLog keeps of one file line: blanks -> '_' when unquoted, CommaText in, CommaText out
// (ElaTables.cpp:519-522 / :588 = golden Rev891 Analyzer.cpp:2371-2375)
static std::string Kept(const std::string& line)
{
    return ela::BcbGetCommaText(ela::BcbCommaText(line.find('"') == std::string::npos ? Underscore(line) : line));
}

struct A8Td { int order; long sot, eot, arm, in; const char* testTime; };   // seconds from 05/06 00:00:00; -1 = none

static std::string A8At(long s)                      // "2026/05/dd hh:mm:ss", s seconds after 2026/05/06 00:00:00
{
    char b[32];
    std::snprintf(b, sizeof(b), "2026/05/%02ld %02ld:%02ld:%02ld", 6 + s / 86400, s % 86400 / 3600, s % 3600 / 60, s % 60);
    return b;
}

static std::string A8Sot(long s)                     // SOT / EOT "yyyymmdd_hhmmss"
{
    char b[32];
    std::snprintf(b, sizeof(b), "202605%02ld_%02ld%02ld%02ld", 6 + s / 86400, s % 86400 / 3600, s % 3600 / 60, s % 60);
    return b;
}

// one IC's asBuffer (eMyProdRec order, MyProductionRecord.h:85-155); greatek = golden's +1 on the X / Y values
// (:255-259 / :291-295 / :394-400).  In Arm Hotplate Pick "A", Out Arm Shuttle Pick "Aa", 2DCode "na", Test Mode "2x4".
static std::vector<std::string> A8Buffer(const A8Td& t, int site, bool greatek)
{
    std::vector<std::string> b(71);
    char n[16];
    const char* one = greatek ? "1" : "0";
    b[0] = "PGM1";
    b[1] = "20260506";
    b[2] = "I1-1";
    std::snprintf(n, sizeof(n), "%d", site - 1 + (greatek ? 1 : 0));
    b[3] = n;
    b[4] = one;
    b[5] = A8At(t.in);                               // eLoadTime "In Time"
    b[6] = one;
    b[7] = one;
    b[8] = A8At(t.in + 1);                           // eHotTime
    b[9] = "na";
    std::snprintf(n, sizeof(n), "%d", site);
    b[10] = n;
    b[11] = "1";
    b[12] = A8At(t.arm);                             // eArmTime
    std::snprintf(n, sizeof(n), "%d", t.order);
    b[13] = n;                                       // eOrderTest
    b[14] = t.sot >= 0 ? A8Sot(t.sot) : "";          // eSOTTime
    b[15] = " 5.630";
    b[16] = "1";
    b[17] = "1";
    b[18] = "O1-13";
    b[19] = one;
    b[20] = one;
    b[21] = A8At(t.arm + 25);
    b[22] = "-46288";
    b[23] = "-72427";
    b[24] = A8At(t.arm + 25);
    b[25] = "na";
    b[26] = "na";
    b[27] = t.testTime;                              // eTestTime
    b[29] = t.eot >= 0 ? A8Sot(t.eot) : "";          // eEOTTime
    b[30] = "A";
    b[31] = b[32] = b[33] = "A-1";
    b[34] = "Aa";                                    // eOutArmShuttlePick (Greatek col 27)
    b[35] = "1";
    b[36] = "A";                                     // eInArmHotplatePick (Greatek col 13)
    b[37] = "2x4";                                   // eTestMode (Greatek col 23; default col 37)
    b[38] = b[39] = b[40] = "0";
    b[41] = b[42] = "10x20";
    return b;
}

// the Handler's file line: SaveRecord fprintf("%s \n", asBuffer->CommaText) (:721-726) / SaveDataForGreatek (:870-881)
static std::string A8Line(const A8Td& t, int site, bool greatek)
{
    const std::vector<std::string> b = A8Buffer(t, site, greatek);
    std::string s;
    if (greatek)
        for (int i = 0; i < 71; ++i) s += b[i < 43 ? kGreatekSort[i] : 0] + ",";
    else
        s = ela::BcbGetCommaText(b);
    return s + " ";
}

static bool ColsAre(const ela::ProdLogCols& c, int in, int arm, int order, int sot, int tt, int eot)
{
    const bool ok = c.inTime == in && c.armTime == arm && c.order == order && c.sot == sot && c.testTime == tt &&
                    c.eot == eot;
    if (!ok)
        printf("    got cols %d / %d / %d / %d / %d / %d\n", c.inTime, c.armTime, c.order, c.sot, c.testTime, c.eot);
    return ok;
}

static std::string EvRow(int day, const char* hms, const char* unit, const char* code, int stopSec, int dup, const char* msg)
{
    char b[256];
    std::snprintf(b, sizeof(b), "2026/05/%02d,%s,\"%s\",%s,1,%d,%d,\"%s\",,R1\r\n", day, hms, unit, code, stopSec, dup, msg);
    return b;
}

// one TimeData row at 2026-mm-dd hh:00:04.000 in the Handler's own form (golden AddTextWithDateTime + the CommaText of
// RecordTimeData's "%i,%i,%i,%i,%i,%i,%i,%i,%i,%d, %d": StartTime, HomeTime, ContactTestTime, .. PowerOnTime .. MTBA)
static std::string TdLine(int month, int day, int hour, long onSec, long startSec, long homeSec = 0, long contactSec = 0)
{
    char b[160];
    std::snprintf(b, sizeof(b), "2026-%02d-%02d, %02d:00:04.000, %ld,%ld,%ld,0,0,0,%ld,0,0,0,0\n", month, day, hour,
                  startSec, homeSec, contactSec, onSec);
    return b;
}

// ---- 9. W61 (AI(W906-ELA-W61) 20260929): 24 hourly TimeData rows closing 05/09 01:00:00 .. 05/10 00:00:00 ----
struct Td9Hour { int hour; long start, home, contact; };   // the row closing at `hour` (1..24) of 05/09: [hour - 1, hour)

static std::vector<ela::TimeDataRow> Td9(const Td9Hour* x, int n, long everyHourStart)
{
    std::vector<ela::TimeDataRow> v;
    for (int h = 1; h <= 24; ++h)
    {
        ela::TimeDataRow t;
        t.at = (h < 24) ? ela::EncodeDate(2026, 5, 9) + ela::EncodeTime(h, 0, 0, 0) : ela::EncodeDate(2026, 5, 10);
        t.onMs = 3600000LL;                                   // PowerOnTime 3600 s: on all day
        t.startMs = everyHourStart * 1000LL;
        for (int i = 0; i < n; ++i)
            if (x[i].hour == h)
            {
                t.startMs = x[i].start * 1000LL;
                t.homeMs = x[i].home * 1000LL;
                t.contactMs = x[i].contact * 1000LL;
            }
        v.push_back(t);
    }
    return v;
}

// ---- checks ----
static const ela::OeeDay* Day(const ela::OeeResult& r, const char* date)
{
    for (size_t i = 0; i < r.days.size(); ++i)
        if (r.days[i].date == date) return &r.days[i];
    return 0;
}

struct Want
{
    long span, test, down, idle, off;
    int testBp, downBp, idleBp, offBp, touchdowns, stopRows, alarmRows;
    bool noData;
};

static bool Is(const ela::OeeDay* d, const Want& w)
{
    if (!d)
    {
        printf("    (day missing)\n");
        return false;
    }
    const bool ok = d->spanMs == w.span * 1000LL && d->testMs == w.test * 1000LL && d->downMs == w.down * 1000LL &&
                    d->idleMs == w.idle * 1000LL && d->offMs == w.off * 1000LL && d->testBp == w.testBp &&
                    d->downBp == w.downBp && d->idleBp == w.idleBp && d->offBp == w.offBp &&
                    d->touchdowns == w.touchdowns && d->stopRows == w.stopRows && d->alarmRows == w.alarmRows &&
                    d->noData == w.noData;
    if (!ok)
        printf("    got %s: span %.3f test %.3f down %.3f idle %.3f off %.3f s, bp %d / %d / %d / %d, touchdowns %d, stop rows "
               "%d, alarm rows %d, noData %d\n",
               d->date.c_str(), d->spanMs / 1000.0, d->testMs / 1000.0, d->downMs / 1000.0, d->idleMs / 1000.0,
               d->offMs / 1000.0, d->testBp, d->downBp, d->idleBp, d->offBp, d->touchdowns, d->stopRows, d->alarmRows,
               d->noData ? 1 : 0);
    return ok;
}

// the TimeData-only fields (seconds)
static bool TdIs(const ela::OeeDay* d, long on, long start, long uncovered)
{
    if (!d)
        return false;
    const bool ok = d->onMs == on * 1000LL && d->startMs == start * 1000LL && d->uncoveredMs == uncovered * 1000LL;
    if (!ok)
        printf("    got %s: on %.3f start %.3f uncovered %.3f s\n", d->date.c_str(), d->onMs / 1000.0, d->startMs / 1000.0,
               d->uncoveredMs / 1000.0);
    return ok;
}

// W61: production and its parts besides test (seconds, hundredths of a percent); also the two sums (prod + down + idle
// + off = 100.00 %, test + contact + home + rest = prod) -- for a day that is not noData
static bool ProdIs(const ela::OeeDay* d, long prod, long contact, long home, long rest, int prodBp, int contactBp,
                   int homeBp, int restBp)
{
    if (!d)
        return false;
    const bool ok = d->prodMs == prod * 1000LL && d->contactMs == contact * 1000LL && d->homeMs == home * 1000LL &&
                    d->prodRestMs == rest * 1000LL && d->prodBp == prodBp && d->contactBp == contactBp &&
                    d->homeBp == homeBp && d->prodRestBp == restBp &&
                    d->prodBp + d->downBp + d->idleBp + d->offBp == 10000 &&
                    d->testBp + d->contactBp + d->homeBp + d->prodRestBp == d->prodBp &&
                    d->prodMs == d->testMs + d->contactMs + d->homeMs + d->prodRestMs &&
                    d->prodMs + d->downMs + d->idleMs + d->offMs == d->spanMs;
    if (!ok)
        printf("    got %s: prod %.3f contact %.3f home %.3f rest %.3f s, bp %d / %d / %d / %d (test %d)\n", d->date.c_str(),
               d->prodMs / 1000.0, d->contactMs / 1000.0, d->homeMs / 1000.0, d->prodRestMs / 1000.0, d->prodBp,
               d->contactBp, d->homeBp, d->prodRestBp, d->testBp);
    return ok;
}

// a kept EventLog row (what GetEventLogText leaves in slStopList / slAlarmList): EvRow without the line end
static std::string EvKept(int day, const char* hms, const char* unit, const char* code, int stopSec, const char* msg)
{
    const std::string s = EvRow(day, hms, unit, code, stopSec, 0, msg);
    return s.substr(0, s.size() - 2);
}

int main()
{
    printf("ELA_Oee\n");

    // ---- 0. containment first (AI(W906-ELA-W48B) 20260928, the tests/test_tcp_cmd_server.cpp rule) ----
    {
        const char* v = std::getenv("W906_HT9045LOG_ROOT");
        printf("  W906_HT9045LOG_ROOT = %s\n", v ? v : "(unset)");
        if (Lower(v ? v : "").find("machine_log_scratch") == std::string::npos)
        {
            printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R ELA_Oee) -- nothing was written\n");
            return 2;
        }
    }

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    const std::string root = std::string(tmp) + "ht9045_ela_oee";
    if (MachineDataPath(root))                                           // before anything is written
    {
        printf("  REFUSED: sandbox %s is under the machine's data (D:\\HT9045*, D:\\RMS, D:\\MTBF_Summary)\n", root.c_str());
        return 1;
    }
    MakeDir(root);
    MakeDir(root + "\\EventLogTxt");
    MakeDir(root + "\\EventLogTxt\\2026");
    MakeDir(root + "\\EventLogTxt\\2026\\05");
    MakeDir(root + "\\Production_Log");
    MakeDir(root + "\\Production_Log\\202605");
    MakeDir(root + "\\TimeData");
    MakeDir(root + "\\TimeData\\2026");
    const std::string ev = root + "\\EventLogTxt\\2026\\05\\EventLogTxt_202605";   // + dd.csv
    const std::string pl = root + "\\Production_Log\\202605\\T_202605";           // + dd.csv
    const std::string tdDir = root + "\\TimeData";
    // W48-3 reads the MTBA flags from it (GetJemIncludeMTBA; without the file nothing counts, golden :2818) -- read only
    // here (jamWriteBack = false): JAM0302 unchecked, MES1641 checked, the rest missing
    WriteText(root + "\\JAM0000.dat", "[03 Index Unit]\r\nJAM0302 IncludeMTBA=0\r\n[16 System]\r\nMES1641 IncludeMTBA=1\r\n");

    {   // 05/01: Steven's example
        std::string p = kProdHeader;
        for (long k = 0; k < 720; ++k) AddTouchdown(&p, 1, 4 * 3600 + 30 * k, "00:00:30.000");    // 04:00:00 .. 09:59:30
        for (long k = 0; k < 1440; ++k) AddTouchdown(&p, 1, 11 * 3600 + 30 * k, "00:00:30.000");  // 11:00:00 .. 22:59:30
        WriteText(pl + "01.csv", p);
        WriteText(ev + "01.csv", std::string(kEvHeader) + EvRow(1, "04:00:00", "16 System", "MES2110", 0, 0, "START") +
                                     EvRow(1, "10:00:00", "01 Input Arm", "JAM0101", 3600, 0, "Pick fail") +
                                     EvRow(1, "15:00:00", "24 Motor", "WAR2401", 999, 1, "Motor alarm"));
    }
    {   // 05/02: W48-1 (no gap joining); a stop inside the off time; a stop across midnight
        std::string p = kProdHeader;
        AddTouchdown(&p, 2, 8 * 3600, "00:00:02.000");                                            // 08:00:00
        AddTouchdown(&p, 2, 8 * 3600 + 300, "00:00:02.000");                                      // 08:05:00
        AddTouchdown(&p, 2, 8 * 3600 + 601, "00:00:02.000");                                      // 08:10:01
        WriteText(pl + "02.csv", p);
        WriteText(ev + "02.csv", std::string(kEvHeader) + EvRow(2, "13:00:00", "01 Input Arm", "JAM0105", 120, 0, "Pick fail 5") +
                                     EvRow(2, "23:50:00", "03 Index Unit", "JAM0301", 1200, 0, "Index jam"));
    }
    {   // 05/03: overlapping stops; touchdowns inside a stop; the W48-3 rows (only the checked MES1641 is MTBA)
        std::string p = kProdHeader;
        for (long k = 0; k <= 30; ++k) AddTouchdown(&p, 3, 9 * 3600 + 50 * 60 + 60 * k, "00:00:02.000");  // 09:50 .. 10:20
        WriteText(pl + "03.csv", p);
        WriteText(ev + "03.csv", std::string(kEvHeader) + EvRow(3, "10:00:00", "01 Input Arm", "JAM0101", 600, 0, "Pick fail") +
                                     EvRow(3, "10:05:00", "02 Output Arm", "JAM0201", 600, 0, "Place fail") +
                                     EvRow(3, "12:00:00", "03 Index Unit", "WAR0301", 300, 0, "Index warning") +
                                     EvRow(3, "14:00:00", "16 System", "MES1640", 600, 0, "One cycle finish") +
                                     EvRow(3, "15:00:00", "03 Index Unit", "JAM0302", 300, 0, "Index jam 2") +
                                     EvRow(3, "16:00:00", "16 System", "MES1641", 200, 0, "Operator call") +
                                     EvRow(3, "17:00:00", "06 Empty Tray Arm", "JAM0601", 100, 0, "Empty tray jam"));
    }
    // 05/04: no log file at all
    {   // 05/05: cut at now 12:00
        std::string p = kProdHeader;
        for (long k = 0; k < 240; ++k) AddTouchdown(&p, 5, 8 * 3600 + 30 * k, "00:00:30.000");    // 08:00:00 .. 09:59:30
        WriteText(pl + "05.csv", p);
        WriteText(ev + "05.csv", std::string(kEvHeader) + EvRow(5, "11:50:00", "01 Input Arm", "JAM0101", 1200, 0, "Pick fail"));
    }
    {   // TimeData (W48-2 = B): the year file <dir>\2026\TimeData_2026.csv, 71 rows
        std::string t = kTdHeader;
        t += TdLine(4, 30, 23, 3600, 0);
        t += TdLine(5, 1, 0, 3600, 3600);
        t += TdLine(5, 1, 1, 3600, 3600);
        for (int h = 2; h <= 23; ++h) t += TdLine(5, 1, h, 3600, 0);
        for (int h = 0; h <= 12; ++h) t += TdLine(5, 2, h, 3600, 0);                              // off after 12:00:04
        t += TdLine(5, 2, 21, 5400, 0);                                                           // 30 min before + 1 h after
        for (int h = 22; h <= 23; ++h) t += TdLine(5, 2, h, 3600, 0);
        for (int h = 0; h <= 23; ++h) t += TdLine(5, 3, h, 3600, 0);
        t += TdLine(5, 4, 0, 3600, 0);                                                            // off after 05/04 00:00:04
        t += TdLine(5, 5, 7, 3600, 600, 600, 0);                                                  // W61: homing 600 s
        t += TdLine(5, 5, 8, 3600, 900, 0, 900);                                                  // W61: contact test 900 s
        t += TdLine(5, 5, 9, 3600, 3600);
        t += TdLine(5, 5, 10, 3600, 3600);
        t += TdLine(5, 5, 11, 3600, 0);
        WriteText(tdDir + "\\2026\\TimeData_2026.csv", t);
        // the flat name (before Steven 20210623): the header, 05/01 01:00:04 again in the SIGURD form (a duplicate), a bad line
        WriteText(tdDir + "\\TimeData_2026.csv",
                  std::string(kTdHeader) + "2026-05-01,01:00:04.000,3600,0,0,0,0,0,3600,0,0,0,0\n" + "garbage,1,2\n");
    }

    ela::Options o;
    o.jamIniPath = root + "\\JAM0000.dat";
    o.jamWriteBack = false;
    ela::QueryRequest q;
    q.startDate = ela::EncodeDate(2026, 5, 1);
    q.startTime = 0.0;
    q.endDate = ela::EncodeDate(2026, 5, 5);
    q.endTime = ela::EncodeTime(23, 59, 59, 0);
    q.eventLogDir = root + "\\EventLogTxt";
    q.prodLogDir = root + "\\Production_Log";
    if (MachineDataPath(o.jamIniPath) || MachineDataPath(q.eventLogDir) || MachineDataPath(q.prodLogDir) ||
        MachineDataPath(tdDir))
    {
        printf("  REFUSED: a path of this test left the sandbox\n");
        return 1;
    }
    ela::Analyzer a(o);
    const bool prodOk = ela::BtnQuery(a, q);
    CHECK(prodOk && a.byDayKeys.size() == 5 && a.slStopList.size() == 12 && a.slAlarmList.size() == 7 &&   //AI(W906-MERGE-0929) 20260929: totals = the sum of this test's own per-day rows (stops 2+2+7+0+1, MTBA 1+2+3+0+1); 9d7872d3 added the 05/02 JAM0105 and 05/03 W48-3 rows but not to the totals
              a.MySummary.iContactCount == 2434,
          "0. query: 5 By Day keys, 12 stop rows kept (the Duplicate=1 WAR dropped), 7 of them MTBA, Contact Count 2434");

    // 0. the SOT stamp
    double v = 0.0;
    CHECK(ela::ParseSotStamp("20260409_145647", &v) &&
              ela::DateTimeToMs(v) == ela::DateTimeToMs(ela::EncodeDate(2026, 4, 9) + ela::EncodeTime(14, 56, 47, 0)) &&
              !ela::ParseSotStamp("20260230_000000", &v) && !ela::ParseSotStamp("2026049_145647", &v) &&
              !ela::ParseSotStamp("20260409-145647", &v) && !ela::ParseSotStamp("20260409_246000", &v) &&
              !ela::ParseSotStamp("", &v),
          "0. ParseSotStamp: yyyymmdd_hhmmss; bad day / length / separator / time refused");

    // 0. TimeData lines and files
    {
        ela::TimeDataRow t;
        const bool plain = ela::ParseTimeDataLine("2026-05-01, 01:00:04.123, 3000,1,2,3,4,5,3600,6,7,8, 9", &t) &&
                           ela::DateTimeToMs(t.at) == ela::DateTimeToMs(ela::EncodeDate(2026, 5, 1) +
                                                                        ela::EncodeTime(1, 0, 4, 123)) &&
                           t.onMs == 3600000 && t.startMs == 3000000 && t.homeMs == 1000 && t.contactMs == 2000;
        ela::TimeDataRow w;                              // W61: HomeTime / ContactTestTime not numbers -> 0, row kept
        const bool lenient = ela::ParseTimeDataLine("2026-05-01, 02:00:04.000, 5,x,,0,0,0,3600,0,0,0, 0", &w) &&
                             w.startMs == 5000 && w.homeMs == 0 && w.contactMs == 0 && w.onMs == 3600000;
        ela::TimeDataRow s;
        const bool sigurd = ela::ParseTimeDataLine("2026-05-01,01:00:04.000,3000,1,2,3,4,5,3600,6,7,8,9", &s) &&
                            s.onMs == 3600000;
        ela::TimeDataRow n;
        const bool negative = ela::ParseTimeDataLine("2026-05-01, 01:00:04.000, -5,0,0,0,0,0,-1,0,0,0, 0", &n) &&
                              n.onMs == 0 && n.startMs == 0;
        ela::TimeDataRow x;
        const bool refused = !ela::ParseTimeDataLine(kTdHeader, &x) && !ela::ParseTimeDataLine("", &x) &&
                             !ela::ParseTimeDataLine("2026-02-30, 01:00:04.000, 0,0,0,0,0,0,3600,0,0,0, 0", &x) &&
                             !ela::ParseTimeDataLine("2026-05-01, 25:00:04.000, 0,0,0,0,0,0,3600,0,0,0, 0", &x) &&
                             !ela::ParseTimeDataLine("2026-05-01, 01:00:04.000, 0,0,0,0,0,0,x,0,0,0, 0", &x) &&
                             !ela::ParseTimeDataLine("2026-05-01, 01:00:04.000, 0,0,0", &x);
        CHECK(plain && sigurd && negative && refused && lenient,
              "0. ParseTimeDataLine: the Handler's form (W61: HomeTime col 3, ContactTestTime col 4) and the SIGURD form; "
              "a negative counter = 0; header / bad date / bad time / not a number / too short refused; W61 columns "
              "that are not numbers read 0");
    }
    std::vector<std::string> tdFiles;
    const std::vector<ela::TimeDataRow> td = ela::ReadTimeData(tdDir, a.dtStart, a.dtEnd, &tdFiles);
    {
        bool sorted = true;
        for (size_t i = 1; i < td.size(); ++i) sorted = sorted && td[i - 1].at < td[i].at;
        const std::string def = ela::DefaultTimeDataDir();
        CHECK(td.size() == 71 && tdFiles.size() == 2 && sorted &&
                  def.size() > 9 && def.compare(def.size() - 9, 9, "\\TimeData") == 0,
              "0. ReadTimeData: both file names read, 71 rows (the SIGURD duplicate and the bad line dropped), sorted; "
              "DefaultTimeDataDir ends in \\TimeData");
    }

    const double now = ela::EncodeDate(2026, 5, 5) + ela::EncodeTime(12, 0, 0, 0);
    const ela::OeeResult r = ela::ComputeOee(a, td, now, ela::OeeOptions());
    CHECK(r.days.size() == 5 && r.touchdowns == 2434 && r.noTestTime == 0 && r.stopRows == 12 && r.alarmRows == 7 &&   //AI(W906-MERGE-0929) 20260929: same totals as :469
              r.timeDataRows == 71 && r.timeDataSlackSec == 60 && ela::kTimeDataSlackSec == 60,
          "0. 5 days, 2434 touchdowns (one per Order of testing), all with a Test Time, 12 stop rows / 7 MTBA alarms, "
          "71 TimeData rows, slack 60 s");

    // 1-5: the fixture table (file head)
    {
        const Want d1 = { 86400, 64800, 3600, 18000, 0, 7500, 417, 2083, 0, 2160, 2, 1, false };
        const Want d2 = { 86400, 6, 720, 58794, 26880, 1, 83, 6805, 3111, 3, 2, 2, false };
        const Want d3 = { 86400, 32, 1700, 84668, 0, 4, 197, 9799, 0, 31, 7, 3, false };
        const Want d4 = { 86400, 0, 0, 4, 86396, 0, 0, 0, 10000, 0, 0, 0, false };
        const Want d5 = { 43200, 7200, 600, 12296, 21604, 1667, 139, 2846, 5001, 240, 1, 1, false };   // W61: idle
        CHECK(Is(Day(r, "2026/05/01"), d1) && TdIs(Day(r, "2026/05/01"), 86400, 3604, 0) &&
                  ProdIs(Day(r, "2026/05/01"), 64800, 0, 0, 0, 7500, 0, 0, 0),
              "1. Steven's 18 / 1 / 5 h = 75.00 / 4.17 / 20.83 %; on all day; SystemStart 4 s of the 00:00:04 row + 3600 s "
              "(W61: less than the test, so production = test)");
        CHECK(Is(Day(r, "2026/05/02"), d2) && TdIs(Day(r, "2026/05/02"), 59400, 0, 0) &&
                  ProdIs(Day(r, "2026/05/02"), 6, 0, 0, 0, 1, 0, 0, 0),
              "2. W48-1: 3 x 2 s; W48-2: off 12:00:04-19:30:04 minus the 120 s JAM0105 inside it (the 5400 s row put at its end)");
        CHECK(Is(Day(r, "2026/05/03"), d3) && TdIs(Day(r, "2026/05/03"), 86400, 0, 0) &&
                  ProdIs(Day(r, "2026/05/03"), 32, 0, 0, 0, 4, 0, 0, 0),
              "3. the other 600 s after midnight; union 900 s; down wins over the 15 touchdowns inside it (16 x 2 s test); "
              "W48-3: WAR / MES1640 / area 06 / unchecked JAM0302 idle, checked MES1641 200 s down");
        CHECK(Is(Day(r, "2026/05/04"), d4) && TdIs(Day(r, "2026/05/04"), 4, 0, 0) &&
                  ProdIs(Day(r, "2026/05/04"), 0, 0, 0, 0, 0, 0, 0, 0),
              "4. W48-2: a day with the software off is off, not noData (4 s on after midnight = idle)");
        CHECK(Is(Day(r, "2026/05/05"), d5) && TdIs(Day(r, "2026/05/05"), 21596, 8700, 0) &&
                  ProdIs(Day(r, "2026/05/05"), 8700, 900, 600, 0, 2014, 208, 139, 0),
              "5. cut at now 12:00; off until 06:00:04; the hour being recorded (11:00:04-12:00) is on; W61: SystemStart "
              "8700 s (read from the file's HomeTime / ContactTestTime columns) = production 20.14 %, idle 28.46 %");
    }

    // without TimeData: no off segment (the uncovered time stays idle) -- 05/04 is noData again
    {
        const std::vector<ela::TimeDataRow> none;
        const ela::OeeResult r0 = ela::ComputeOee(a, none, now, ela::OeeOptions());
        const Want d2 = { 86400, 6, 720, 85674, 0, 1, 83, 9916, 0, 3, 2, 2, false };
        const Want d4 = { 86400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true };
        const Want d5 = { 43200, 7200, 600, 35400, 0, 1667, 139, 8194, 0, 240, 1, 1, false };
        CHECK(r0.timeDataRows == 0 && Is(Day(r0, "2026/05/02"), d2) && TdIs(Day(r0, "2026/05/02"), 0, 0, 86400) &&
                  Is(Day(r0, "2026/05/04"), d4) && Is(Day(r0, "2026/05/05"), d5) &&
                  ProdIs(Day(r0, "2026/05/05"), 7200, 0, 0, 0, 1667, 0, 0, 0),
              "W48-2 without TimeData: no off, the uncovered time is idle; 05/04 noData; W61: production = test");
    }

    // an overdue tail: now 12:01:10 is 3666 s after the last row (> 3600 + 60): uncovered (idle), not off
    {
        const ela::OeeResult rt = ela::ComputeOee(a, td, ela::EncodeDate(2026, 5, 5) + ela::EncodeTime(12, 1, 10, 0),
                                                  ela::OeeOptions());
        const Want d5 = { 43270, 7200, 670, 12296, 21604, 1664, 155, 2841, 4993, 240, 1, 1, false };   // W61: idle
        CHECK(Is(Day(rt, "2026/05/05"), d5) && TdIs(Day(rt, "2026/05/05"), 18000, 8700, 3666) &&
                  ProdIs(Day(rt, "2026/05/05"), 8700, 900, 600, 0, 2011, 208, 139, 0),
              "W48-2: past 3600 + 60 s after the last row the tail is uncovered (idle), not on and not off");
    }

    // a first row that carries days of PowerOn (the counters were never reset hourly) covers one hour at most
    {
        std::vector<ela::TimeDataRow> one(1);
        one[0].at = ela::EncodeDate(2026, 5, 5) + ela::EncodeTime(11, 0, 4, 0);
        one[0].onMs = 432000000LL;                                                                // 5 days
        const ela::OeeResult r1 = ela::ComputeOee(a, one, now, ela::OeeOptions());
        const ela::OeeDay* d4 = Day(r1, "2026/05/04");
        CHECK(TdIs(Day(r1, "2026/05/05"), 7256, 0, 35944) && Day(r1, "2026/05/05")->offMs == 0 && d4 && d4->noData,
              "W48-2: the first row covers [at - 3660 s, at) (09:59:04) + the tail; 05/04 stays noData");
    }

    // golden By Day keeps its own (unclipped, own-day) Total Stop Time / Alarm Time -- the OEE split is a deliberate
    // difference (Alarm Time is golden's own MTBA sum, the rows W48-3 = B counts as down)
    {
        std::map<std::string, ela::Summary>::const_iterator d2 = a.mapByDayList.find("2026/05/02");
        std::map<std::string, ela::Summary>::const_iterator d3 = a.mapByDayList.find("2026/05/03");
        CHECK(d2 != a.mapByDayList.end() && d2->second.iStopTime == 1320 && d2->second.iAlarmTime == 1320 &&
                  d3 != a.mapByDayList.end() && d3->second.iStopTime == 2700 && d3->second.iAlarmTime == 1400,
              "2/3. By Day Total Stop / Alarm Time stay golden's 1320 / 1320 s and 2700 / 1400 s (OEE down: 720 s / 1700 s)");
    }

    // now before the range: every span 0, noData
    {
        const ela::OeeResult early = ela::ComputeOee(a, td, ela::EncodeDate(2026, 4, 30), ela::OeeOptions());
        bool allEmpty = early.days.size() == 5;
        for (size_t i = 0; i < early.days.size(); ++i)
            allEmpty = allEmpty && early.days[i].spanMs == 0 && early.days[i].noData && early.days[i].testBp == 0 &&
                       early.days[i].offMs == 0;
        CHECK(allEmpty, "5. a day after now: span 0, noData");
    }

    // the JSON the snapshot carries
    {
        const std::string js = ela::OeeJson(r, tdDir);
        // (W61: the W48 keys first, in their W48 order -- tests/test_ela_hub.cpp and the page read them; W61's after
        // offPct)
        CHECK(Has(js, "{\"date\":\"2026/05/01\",\"spanSec\":86400,\"testSec\":64800,\"downSec\":3600,\"idleSec\":18000,"
                      "\"offSec\":0,\"testPct\":75.00,\"downPct\":4.17,\"idlePct\":20.83,\"offPct\":0.00,\"prodSec\":64800,"
                      "\"contactSec\":0,\"homeSec\":0,\"prodRestSec\":0,\"prodPct\":75.00,\"contactPct\":0.00,"
                      "\"homePct\":0.00,\"prodRestPct\":0.00,\"onSec\":86400,\"startSec\":3604,\"uncoveredSec\":0,"
                      "\"touchdowns\":2160,\"stopRows\":2,\"alarmRows\":1,\"noData\":false}"),
              "JSON: day 1");
        CHECK(Has(js, "{\"date\":\"2026/05/02\",\"spanSec\":86400,\"testSec\":6,\"downSec\":720,\"idleSec\":58794,"
                      "\"offSec\":26880,\"testPct\":0.01,\"downPct\":0.83,\"idlePct\":68.05,\"offPct\":31.11,\"prodSec\":6,"
                      "\"contactSec\":0,\"homeSec\":0,\"prodRestSec\":0,\"prodPct\":0.01,\"contactPct\":0.00,"
                      "\"homePct\":0.00,\"prodRestPct\":0.00,\"onSec\":59400,\"startSec\":0,\"uncoveredSec\":0,"
                      "\"touchdowns\":3,\"stopRows\":2,\"alarmRows\":2,\"noData\":false}"),
              "JSON: day 2 (off)");
        CHECK(Has(js, "{\"date\":\"2026/05/04\",\"spanSec\":86400,\"testSec\":0,\"downSec\":0,\"idleSec\":4,"
                      "\"offSec\":86396,\"testPct\":0.00,\"downPct\":0.00,\"idlePct\":0.00,\"offPct\":100.00,\"prodSec\":0,"
                      "\"contactSec\":0,\"homeSec\":0,\"prodRestSec\":0,\"prodPct\":0.00,\"contactPct\":0.00,"
                      "\"homePct\":0.00,\"prodRestPct\":0.00,\"onSec\":4,\"startSec\":0,\"uncoveredSec\":0,"
                      "\"touchdowns\":0,\"stopRows\":0,\"alarmRows\":0,\"noData\":false}"),
              "JSON: day 4 (off all day)");
        CHECK(Has(js, "{\"date\":\"2026/05/05\",\"spanSec\":43200,\"testSec\":7200,\"downSec\":600,\"idleSec\":12296,"
                      "\"offSec\":21604,\"testPct\":16.67,\"downPct\":1.39,\"idlePct\":28.46,\"offPct\":50.01,"
                      "\"prodSec\":8700,\"contactSec\":900,\"homeSec\":600,\"prodRestSec\":0,\"prodPct\":20.14,"
                      "\"contactPct\":2.08,\"homePct\":1.39,\"prodRestPct\":0.00,\"onSec\":21596,\"startSec\":8700,"
                      "\"uncoveredSec\":0,\"touchdowns\":240,\"stopRows\":1,\"alarmRows\":1,\"noData\":false}"),
              "JSON: day 5 (W61: production 8700 s = test + contact test + home)");
        CHECK(js.compare(0, 9, "{\"rule\":{") == 0 &&
                  Has(js, "\"test\":\"touchdownTestTime\",\"w481\":\"col27\",\"down\":\"mtbaAlarmsUnion\",\"w483\":\"B\","
                          "\"off\":\"timeDataPowerOn\",\"w482\":\"B\",\"idle\":\"spanMinusProdDownOff\","
                          "\"ruled\":\"Steven 0928 09:3x\",\"prod\":\"timeDataStartTime\",\"w61\":\"B\","
                          "\"contact\":\"timeDataContactTestTime\",\"home\":\"timeDataHomeTime\","
                          "\"offline\":\"noGoldenSignal\",\"motorDownOutOfStart\":\"WAR24\"") &&
                  !Has(js, "gapSec") && !Has(js, "pending") && Has(js, "\"now\":\"2026/05/05 12:00:00\"") &&
                  Has(js, "\"touchdowns\":2434,\"noTestTime\":0,\"stopRows\":12,\"alarmRows\":7,\"timeDataRows\":71,"   //AI(W906-MERGE-0929) 20260929: same totals as :469
                          "\"timeDataSlackSec\":60,\"timeDataDir\":\"") &&
                  Has(js, "ht9045_ela_oee\\\\TimeData\"},\"days\":["),
              "JSON: rule (W48-1 / W48-2 = B / W48-3 = B, Steven; W61 = B), now, totals, the TimeData folder (escaped)");
    }

    // 6. one day 00:00:00 .. 23:59:59: the To second is included -> a whole-day span
    {
        ela::Analyzer a1(o);
        ela::QueryRequest q1 = q;
        q1.endDate = ela::EncodeDate(2026, 5, 1);
        ela::BtnQuery(a1, q1);
        const ela::OeeResult r1 = ela::ComputeOee(a1, td, ela::EncodeDate(2026, 5, 6), ela::OeeOptions());
        const Want d1 = { 86400, 64800, 3600, 18000, 0, 7500, 417, 2083, 0, 2160, 2, 1, false };
        CHECK(r1.days.size() == 1 && Is(Day(r1, "2026/05/01"), d1), "6. one-day query To 23:59:59: span 86400 s, idle 18000 s");
    }

    // 7. W48-1's start when SOT is missing -- EOT - Test Time, then Arm Time; a touchdown whose Test Time is unreadable
    //    is counted but adds no test time
    {
        ela::Analyzer b(o);
        b.SetRange(ela::EncodeDate(2026, 5, 6), 0.0, ela::EncodeDate(2026, 5, 6), ela::EncodeTime(23, 59, 59, 0));
        b.slAllData.push_back("header");
        b.slAllData.push_back(ProdRow(1, "", "20260506_100010", "\"2026/05/06 09:59:00\"", "00:00:10.000"));   // [10:00:00, 10:00:10)
        b.slAllData.push_back(ProdRow(2, "", "", "\"2026/05/06 11:00:00\"", "00:00:05.000"));                   // [11:00:00, 11:00:05)
        b.slAllData.push_back(ProdRow(3, "20260506_120000", "", "\"2026/05/06 11:59:00\"", "na"));             // no Test Time
        const std::vector<ela::TimeDataRow> none;
        const ela::OeeResult rb = ela::ComputeOee(b, none, ela::EncodeDate(2026, 5, 7), ela::OeeOptions());
        const ela::OeeDay* d6 = Day(rb, "2026/05/06");
        CHECK(rb.touchdowns == 3 && rb.noTestTime == 1 && d6 && d6->testMs == 15000 && d6->touchdowns == 3 &&
                  !d6->noData,
              "7. start = EOT - Test Time (10 s) / Arm Time (5 s); the 'na' Test Time counts a touchdown, adds 0 s");
    }

    // 8. A8 (review finding, St02's call; AI(W906-ELA-A8) 20260928): the Production_Log layout.  The same five
    //    touchdowns (two sites each) written the Greatek way and the default way, kept as ListProductionLog keeps them:
    //    05/06 08:00:00 7 s, 08:01:00 8 s; SOT 05/07 00:00:10 7 s (its In Time 05/06 23:59:30); no SOT: EOT 05/07
    //    09:00:10 - 10 s; no SOT / EOT: Arm Time 05/07 10:00:00 5 s.  -> 05/06 15 s / 2 touchdowns, 05/07 22 s / 3.
    {
        static const A8Td kTd[5] = {
            { 1, 28800, 28807, 28795, 28760, "00:00:07.000" },
            { 2, 28860, 28868, 28855, 28820, "00:00:08.000" },
            { 3, 86410, 86417, 86405, 86370, "00:00:07.000" },
            { 4, -1, 118810, 118795, 118760, "00:00:10.000" },
            { 5, -1, -1, 122400, 122365, "00:00:05.000" },
        };
        std::vector<std::string> gRows, dRows;       // as ListProductionLog keeps them
        std::string gFile = std::string(kGreatekHeader) + "\r\n";   // TStringList::SaveToFile, then the "a+" lines
        for (int k = 0; k < 5; ++k)
            for (int site = 1; site <= 2; ++site)
            {
                const std::string gl = A8Line(kTd[k], site, true);
                gRows.push_back(Kept(gl));
                gFile += gl + "\r\n";
                dRows.push_back(Kept(A8Line(kTd[k], site, false)));
            }
        const std::string gHead = Underscore(kGreatekHeader);   // ListProductionLog's slAllData[0]
        const std::string dHead = a.slAllData.empty() ? std::string("header") : a.slAllData[0];   // the kProdHeader slot
        const std::vector<ela::TimeDataRow> none;
        const double now8 = ela::EncodeDate(2026, 5, 8);
        const Want w6 = { 86400, 15, 0, 86385, 0, 2, 0, 9998, 0, 2, 0, 0, false };
        const Want w7 = { 86400, 22, 0, 86378, 0, 3, 0, 9997, 0, 3, 0, 0, false };

        // 8a. ProdLogColumns: a golden header name wins; else CUSTOMER_CODE exactly "956"; else the default
        CHECK(ColsAre(ela::ProdLogColumns(dHead, ""), 5, 12, 13, 14, 27, 29) &&
                  ColsAre(ela::ProdLogColumns(dHead, "956"), 5, 12, 13, 14, 27, 29) &&
                  ColsAre(ela::ProdLogColumns(gHead, "956"), 6, 18, 19, 20, 37, 39) &&
                  ColsAre(ela::ProdLogColumns(gHead, "000"), 6, 18, 19, 20, 37, 39) &&
                  ColsAre(ela::ProdLogColumns("header", "956"), 6, 18, 19, 20, 37, 39) &&
                  ColsAre(ela::ProdLogColumns("header", "000"), 5, 12, 13, 14, 27, 29) &&
                  ColsAre(ela::ProdLogColumns("", " 956"), 5, 12, 13, 14, 27, 29) &&
                  ColsAre(r.cols, 5, 12, 13, 14, 27, 29) && std::strcmp(ela::kCustCodeGreatek, "956") == 0,
              "8a. columns: the golden header names (default 5/12/13/14/27/29, Greatek 6/18/19/20/37/39) win over "
              "CUSTOMER_CODE; without names 956 = Greatek, anything else (' 956' too) = default; the main query = default");

        ela::Analyzer g(o);
        g.SetCustCode("956");
        g.SetRange(ela::EncodeDate(2026, 5, 6), 0.0, ela::EncodeDate(2026, 5, 7), ela::EncodeTime(23, 59, 59, 0));
        g.slAllData.push_back(gHead);
        g.slAllData.insert(g.slAllData.end(), gRows.begin(), gRows.end());
        const ela::OeeResult rg = ela::ComputeOee(g, none, now8, ela::OeeOptions());
        CHECK(rg.touchdowns == 5 && rg.noTestTime == 0 && ColsAre(rg.cols, 6, 18, 19, 20, 37, 39) &&
                  Is(Day(rg, "2026/05/06"), w6) && Is(Day(rg, "2026/05/07"), w7) &&
                  Has(ela::OeeJson(rg, "x"), "\"w481\":\"col37\",\"down\""),
              "8b. Greatek file (CUSTOMER_CODE 956, golden header): Test Time col 37, Order 19, SOT 20, EOT 39, Arm 18 "
              "-> 5 touchdowns, 15 s / 22 s (the 00:00:10 SOT lands on 05/07); JSON w481 col37");

        g.slAllData[0] = "header";                   // no names: CUSTOMER_CODE alone
        const ela::OeeResult rc = ela::ComputeOee(g, none, now8, ela::OeeOptions());
        CHECK(rc.touchdowns == 5 && rc.noTestTime == 0 && ColsAre(rc.cols, 6, 18, 19, 20, 37, 39) &&
                  Is(Day(rc, "2026/05/06"), w6) && Is(Day(rc, "2026/05/07"), w7),
              "8c. Greatek rows, header slot without names: CUSTOMER_CODE 956 -> golden's Greatek index, same numbers");

        g.SetCustCode("000");                        // what A8 found: the default columns on a Greatek file
        const ela::OeeResult rd = ela::ComputeOee(g, none, now8, ela::OeeOptions());
        const ela::OeeDay* rd6 = Day(rd, "2026/05/06");
        const ela::OeeDay* rd7 = Day(rd, "2026/05/07");
        CHECK(rd.touchdowns == 1 && rd.noTestTime == 1 && ColsAre(rd.cols, 5, 12, 13, 14, 27, 29) && rd6 &&
                  rd6->touchdowns == 1 && rd6->testMs == 0 && !rd6->noData && rd7 && rd7->noData,
              "8d. the A8 misread (no names, not 956): col 13 'A' = one Order, col 27 'Aa' = no Test Time -> 1 touchdown, "
              "0 s");

        ela::Analyzer d(o);                          // 8e. non-Greatek: the same touchdowns in the default layout
        d.SetRange(ela::EncodeDate(2026, 5, 6), 0.0, ela::EncodeDate(2026, 5, 7), ela::EncodeTime(23, 59, 59, 0));
        d.slAllData.push_back(dHead);
        d.slAllData.insert(d.slAllData.end(), dRows.begin(), dRows.end());
        const ela::OeeResult rn = ela::ComputeOee(d, none, now8, ela::OeeOptions());
        d.slAllData[0] = "header";
        const ela::OeeResult rh = ela::ComputeOee(d, none, now8, ela::OeeOptions());
        CHECK(rn.touchdowns == 5 && rn.noTestTime == 0 && ColsAre(rn.cols, 5, 12, 13, 14, 27, 29) &&
                  Is(Day(rn, "2026/05/06"), w6) && Is(Day(rn, "2026/05/07"), w7) &&
                  Has(ela::OeeJson(rn, "x"), "\"w481\":\"col27\",\"down\"") && rh.touchdowns == 5 && rh.noTestTime == 0 &&
                  ColsAre(rh.cols, 5, 12, 13, 14, 27, 29) && Is(Day(rh, "2026/05/06"), w6) && Is(Day(rh, "2026/05/07"), w7),
              "8e. non-Greatek (CUSTOMER_CODE unset, header or none): col 27 / 13 / 14 / 29 / 12 as before -> the same "
              "numbers as the Greatek file; JSON w481 col27 (sections 1-7 unchanged)");

        // 8f. the gap upstream (not changed here): golden ListProductionLog keeps a row by col 5 for every customer
        MakeDir(root + "\\Production_Log_A8");
        MakeDir(root + "\\Production_Log_A8\\202605");
        const std::string gPath = root + "\\Production_Log_A8\\202605\\T_20260506.csv";
        WriteText(gPath, gFile);
        ela::Analyzer lp(o);
        lp.SetCustCode("956");
        lp.SetRange(ela::EncodeDate(2026, 5, 6), 0.0, ela::EncodeDate(2026, 5, 7), ela::EncodeTime(23, 59, 59, 0));
        const bool lpOk = lp.ListProductionLog(std::vector<std::string>(1, gPath));
        const ela::OeeResult rl = ela::ComputeOee(lp, none, now8, ela::OeeOptions());
        CHECK(lpOk && lp.slAllData.size() == 1 && lp.slAllData[0] == gHead && lp.MySummary.iInputCount == 0 &&
                  lp.MySummary.iContactCount == 0 && lp.exceptions.empty() && ColsAre(rl.cols, 6, 18, 19, 20, 37, 39) &&
                  rl.touchdowns == 0,
              "8f. KNOWN GAP (golden Rev891 too): ListProductionLog reads a Greatek file's col 5 = In Y ('1' = 01:00 of "
              "1899) as In Time and keeps no row -> no Greatek touchdown reaches the OEE; its header slot names col 37");
    }

    // 9. ★W61 = B (AI(W906-ELA-W61) 20260929, St02-E helper; Steven: every SystemStart second is production, split into
    //    test / contact test / home / the rest; no test and no SystemStart = idle).  05/09, on all day (Td9: 24 hourly
    //    rows, PowerOnTime 3600 s), four touchdowns of 500 s at 10:00 / 10:10 / 10:20 / 10:30 = test 2000 s.  Numbers
    //    from an independent Python model of the arithmetic, not from the built code.
    {
        ela::Analyzer b(o);
        b.SetRange(ela::EncodeDate(2026, 5, 9), 0.0, ela::EncodeDate(2026, 5, 9), ela::EncodeTime(23, 59, 59, 0));
        b.slAllData.push_back("header");
        for (int k = 0; k < 4; ++k)
        {
            char sot[32];
            std::snprintf(sot, sizeof(sot), "20260509_10%02d00", 10 * k);
            b.slAllData.push_back(ProdRow(k + 1, sot, "", "\"2026/05/09 09:59:00\"", "00:08:20.000"));
        }
        const double now9 = ela::EncodeDate(2026, 5, 10);
        const ela::OeeOptions opt;

        // 9a. SystemStart 5000 s (StartTime of the 11:00 and 12:00 rows), test 2000 s, no down: production 5000 s =
        //     test 2000 + rest 3000; idle drops by 3000 s against the same day without SystemStart
        const Td9Hour a9[2] = { { 11, 3600, 0, 0 }, { 12, 1400, 0, 0 } };
        const ela::OeeResult ra = ela::ComputeOee(b, Td9(a9, 2, 0), now9, opt);
        const ela::OeeResult rz = ela::ComputeOee(b, Td9(0, 0, 0), now9, opt);
        const ela::OeeDay* da = Day(ra, "2026/05/09");
        const ela::OeeDay* dz = Day(rz, "2026/05/09");
        const Want wa = { 86400, 2000, 0, 81400, 0, 231, 0, 9421, 0, 4, 0, 0, false };
        const Want wz = { 86400, 2000, 0, 84400, 0, 231, 0, 9769, 0, 4, 0, 0, false };
        CHECK(ra.touchdowns == 4 && Is(da, wa) && TdIs(da, 86400, 5000, 0) && ProdIs(da, 5000, 0, 0, 3000, 579, 0, 0, 348) &&
                  Is(dz, wz) && ProdIs(dz, 2000, 0, 0, 0, 231, 0, 0, 0) && da && dz && da->idleMs == dz->idleMs - 3000000,
              "9a. W61: SystemStart 5000 s, test 2000 s, no down -> production 5000 s (test 2000 + rest 3000, 5.79 %), "
              "idle 81400 s = 3000 s less than without SystemStart (84400 s)");

        // 9b. the split: 08:00-09:00 homing 600 s (HomeTime), 09:00-10:00 contact test 900 s (ContactTestTime),
        //     10:00-11:00 3500 s (ProductionTime, the tests inside it) -> StartTime 5000 s = test 2000 + contact 900 +
        //     home 600 + rest 1500
        const Td9Hour b9[3] = { { 9, 600, 600, 0 }, { 10, 900, 0, 900 }, { 11, 3500, 0, 0 } };
        const ela::OeeResult rb9 = ela::ComputeOee(b, Td9(b9, 3, 0), now9, opt);
        const ela::OeeDay* db = Day(rb9, "2026/05/09");
        CHECK(Is(db, wa) && TdIs(db, 86400, 5000, 0) && ProdIs(db, 5000, 900, 600, 1500, 579, 104, 69, 175),
              "9b. W61 split: production 5000 s = test 2000 + contact test 900 + home 600 + rest 1500 (5.79 = 2.31 + "
              "1.04 + 0.69 + 1.75 %), idle 81400 s");
        CHECK(Has(ela::OeeJson(rb9, "x"),
                  "{\"date\":\"2026/05/09\",\"spanSec\":86400,\"testSec\":2000,\"downSec\":0,\"idleSec\":81400,"
                  "\"offSec\":0,\"testPct\":2.31,\"downPct\":0.00,\"idlePct\":94.21,\"offPct\":0.00,\"prodSec\":5000,"
                  "\"contactSec\":900,\"homeSec\":600,\"prodRestSec\":1500,\"prodPct\":5.79,\"contactPct\":1.04,"
                  "\"homePct\":0.69,\"prodRestPct\":1.75,\"onSec\":86400,\"startSec\":5000,\"uncoveredSec\":0,"
                  "\"touchdowns\":4,\"stopRows\":0,\"alarmRows\":0,\"noData\":false}"),
              "9b. JSON: prodSec / contactSec / homeSec / prodRestSec and their % (the W48 keys kept)");

        // 9c. down vs SystemStart: a counted motor alarm WAR240011 14:00 400 s -- golden keeps SystemStart on and
        //     fAllMotorHome off under that Note, so the 14:00-15:00 row carries StartTime / HomeTime 400 s too; it is
        //     taken out of both (same production as 9b).  The same 400 s as a JAM (ShowErrorMessage stops SystemStart
        //     first) is not taken out: that row's 400 s homing is real production.
        const Td9Hour c9[4] = { { 9, 600, 600, 0 }, { 10, 900, 0, 900 }, { 11, 3500, 0, 0 }, { 15, 400, 400, 0 } };
        b.slStopList.assign(1, EvKept(9, "14:00:00", "24 Motor", "WAR240011", 400, "M 01 error"));
        b.slAlarmList = b.slStopList;
        const ela::OeeResult rc9 = ela::ComputeOee(b, Td9(c9, 4, 0), now9, opt);
        const ela::OeeDay* dc = Day(rc9, "2026/05/09");
        const Want wc = { 86400, 2000, 400, 81000, 0, 231, 46, 9375, 0, 4, 1, 1, false };
        b.slStopList.assign(1, EvKept(9, "14:00:00", "01 Input Arm", "JAM0101", 400, "Pick fail"));
        b.slAlarmList = b.slStopList;
        const ela::OeeResult rj9 = ela::ComputeOee(b, Td9(c9, 4, 0), now9, opt);
        const ela::OeeDay* dj = Day(rj9, "2026/05/09");
        const Want wj = { 86400, 2000, 400, 80600, 0, 231, 46, 9329, 0, 4, 1, 1, false };
        CHECK(Is(dc, wc) && TdIs(dc, 86400, 5400, 0) && ProdIs(dc, 5000, 900, 600, 1500, 579, 104, 69, 175) &&
                  Is(dj, wj) && TdIs(dj, 86400, 5400, 0) && ProdIs(dj, 5400, 900, 1000, 1500, 625, 104, 116, 174),
              "9c. W61: a counted WAR24 motor alarm (400 s down) is taken out of StartTime / HomeTime (production 5000 s, "
              "idle 81000 s); a JAM of 400 s is not (production 5400 s, home 1000 s, idle 80600 s)");

        // 9d. the cap: StartTime 3600 s in every row (86400 s) with a JAM of 3600 s -- production takes what test, down
        //     and off leave: 2000 + 80800 = 82800 s, idle 0
        b.slStopList.assign(1, EvKept(9, "14:00:00", "01 Input Arm", "JAM0101", 3600, "Pick fail"));
        b.slAlarmList = b.slStopList;
        const ela::OeeResult rd9 = ela::ComputeOee(b, Td9(0, 0, 3600), now9, opt);
        const ela::OeeDay* dd = Day(rd9, "2026/05/09");
        const Want wd = { 86400, 2000, 3600, 0, 0, 231, 417, 0, 0, 4, 1, 1, false };
        CHECK(Is(dd, wd) && TdIs(dd, 86400, 86400, 0) && ProdIs(dd, 82800, 0, 0, 80800, 9583, 0, 0, 9352),
              "9d. W61 cap: SystemStart all day with a 3600 s JAM -> production 82800 s (95.83 %), down 3600 s (4.17 %), "
              "idle 0");

        // 9e. rounding: test / contact / home 13 s each (1.50 hundredths of a % -> 2 each) in a production of 39 s
        //     (4.51 -> 5): the rest would be -1, so it is taken from home (the parts add up to production's 0.05 %)
        ela::Analyzer e(o);
        e.SetRange(ela::EncodeDate(2026, 5, 9), 0.0, ela::EncodeDate(2026, 5, 9), ela::EncodeTime(23, 59, 59, 0));
        e.slAllData.push_back("header");
        e.slAllData.push_back(ProdRow(1, "20260509_100000", "", "\"2026/05/09 09:59:00\"", "00:00:13.000"));
        const Td9Hour e9[1] = { { 11, 39, 13, 13 } };
        const ela::OeeResult re9 = ela::ComputeOee(e, Td9(e9, 1, 0), now9, opt);
        const ela::OeeDay* de = Day(re9, "2026/05/09");
        const Want we = { 86400, 13, 0, 86361, 0, 2, 0, 9995, 0, 1, 0, 0, false };
        CHECK(Is(de, we) && ProdIs(de, 39, 13, 13, 0, 5, 2, 1, 0),
              "9e. W61 rounding: 2 + 2 + 2 > 5 -> home gives 1 back (test 2, contact 2, home 1, rest 0 = production 5)");
    }

    printf("ELA_Oee: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
    {
        for (size_t i = g_files.size(); i-- > 0;) ::DeleteFileA(g_files[i].c_str());
        for (size_t i = g_dirs.size(); i-- > 0;) ::RemoveDirectoryA(g_dirs[i].c_str());
    }
    return g_fail ? 1 : 0;
}
