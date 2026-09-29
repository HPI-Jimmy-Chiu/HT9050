// =============================================================================
//  test_ela_core.cpp -- ELA plan P1a: ElaCore (Event Log Analyzer core, golden EventlogAnalyzer Rev891.0).
//
//  AI(W906-ELA-P1) 20260927.  Suite name (add_test): ELA_Core
//
//  Covers EventLogAnalysis/ElaCore.*.  Sections 3-8 pin the golden switches (bcbCommaText=true ...): they are the golden
//  oracle regression (#23).  Later sections run Steven's 20260927 rulings (the defaults) -- AI(W906-ELA-W15/W18/W19):
//    1. BCB6 CommaText read / write (blank splitting, quotes, "" escape, trailing comma, missing closing quote);
//    2. Common.cpp date helpers (validators, ParseDateTime incl. 2026/02/30 -> TDateTime(), formatting);
//    3. SetRange keys (by day from midnight, by hour from the start time) and iDate;
//    4. GetEventLogText over two day files: stop / alarm (MTBA) / fail (MTBF) / JAM / WAR / MES counts and times,
//       per-day and per-hour, Jam / War / Alarm summaries, By-Area and By-Function lists, duplicate / out-of-range /
//       unquoted "16 System" rows, the file-name rule;
//    5. JAM0000.dat write-back (#17 A) and jamWriteBack=false;
//    6. the golden leaks (golden mode: iFailCount and the By-Area lists grow on a second query) and keepGoldenBugs=false;
//    7. #15 B (comma-only) counts the "16 System" row.
//    8. P1b: ConvertSecToTime / ConvertDTToTime / StrToDateTime (zh-TW) / LoadFavorite; the Top5 / Top5Filter /
//       FailAndAlarm / ByFilter grids incl. golden's quirks (stale Top5Filter rows after a filter switch, "No Record!!"
//       over a single By-Area record, prefix match in Top5); Production_Log -> sgByDay / sgByHour (incl. golden's by-hour
//       test-time bug) and the mmoSummary lines.
//    9. W15 B: ela::SplitEventLogCsv on real row shapes, against the V906 viewer's two helpers (cObserver.cpp:1351-1385,
//       as they were before W15, copied below verbatim on vclcompat) and the bodies cObserver.cpp has now (compiled
//       here verbatim; the copy is checked against the file's text), the round trip through the lists, and sample (a)
//       counted both ways.
//   10. the files of sections 3-8 with the default Options (the rulings): W15 B counts, W18 B fixes (iFailCount and the
//       lists reset, Top5Filter rebuilt, Top5 exact match, one By-Area record kept, by-hour test time x OneDayMS,
//       a half-written Production_Log line skipped).
//   11. W19 B: EventLogFileSpan over every save-period name (and what is not an event-log file), the row de-dup across
//       files (repeats inside one file still count), and the W23 samples: AllEventLog counted once, 12 h files incl.
//       the one that runs past midnight, hour-named files, a month file read once for a two-day query; ByLot /
//       JamStat / RawData never read; each query also in golden mode.  Max-line files run between their neighbours of
//       the same base name (first / last open-ended); VTEST 915 / 919 in both modes (golden skips plain day files).
//       ★W38 B (AI(W906-ELA-W38) 20260928, St02-E helper): on 915 / 919 the golden file rule is the DEFAULT
//       (Options::goldenFileRuleVtest); goldenFileRuleVtest=false gives back the W19 B rule there.
//  Every file is under %TEMP%\ht9045_ela_core or %TEMP%\ht9045_ela_core_fx -- never D:\HT9045_Log or D:\HT9045\Error.
//  The W23 samples are copies in tests/fixtures/ela (W906_ELA_FIXTURE_DIR, read only), copied to %TEMP% first.
// =============================================================================
#include "EventLogAnalysis/ElaCore.h"
#include "vclcompat/AnsiString.h"
#include "vclcompat/SysUtils.h"
#include "vclcompat/TStringList.h"
#include <windows.h>
#include <cstdio>
#include <cctype>                  // AI(W906-ELA-REV): MachineDataPath
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string g_root;

static void MkDir(const std::string& p) { ::CreateDirectoryA(p.c_str(), 0); }

static void WriteFile(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static std::string ReadAll(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

// ---- AI(W906-ELA-W15) 20260927 (St02-E): the reference for W15 B -- the V906 viewer's two helpers as they were
//      before W15 (cObserver.cpp:1351-1385 up to a9b93d61 = golden 912 cObserver.cpp:3812-3846), VERBATIM on vclcompat.
//      ela::SplitEventLogCsv and the viewer's bodies now (viewer_now below) must give the same fields on every line,
//      so moving the viewer onto the shared splitter changed nothing on screen. ----
namespace viewer {
using vclcompat::AnsiString;
using vclcompat::TStringList;
using vclcompat::StringReplace;
using vclcompat::TReplaceFlags;
using vclcompat::rfReplaceAll;
static void AddEventLogCsvField(TStringList *tsRow, AnsiString asField)
{
    asField=asField.Trim();
    if(asField.Length()>=1 && asField[1]=='"')
    {
        asField=asField.SubString(2, asField.Length()-1);
        if(asField.Length()>=1 && asField[asField.Length()]=='"')
            asField=asField.SubString(1, asField.Length()-1);
        asField=StringReplace(asField, "\"\"", "\"", TReplaceFlags()<<rfReplaceAll);
    }
    tsRow->Add(asField);
}
static void SplitEventLogCsvLine(const AnsiString &asLine, TStringList *tsRow)
{
    tsRow->Clear();

    int  iLen    =asLine.Length();
    int  iStart  =1;
    bool bInQuote=false;

    for(int i=1; i<=iLen; i++)
    {
        if(asLine[i]=='"')
        {
            bInQuote=!bInQuote;
        }
        else if(asLine[i]==',' && bInQuote==false)
        {
            AddEventLogCsvField(tsRow, asLine.SubString(iStart, i-iStart));
            iStart=i+1;
        }
    }
    AddEventLogCsvField(tsRow, asLine.SubString(iStart, iLen-iStart+1));
}
static ela::Row Split(const std::string& line)
{
    TStringList ts;
    SplitEventLogCsvLine(AnsiString(line), &ts);
    ela::Row r;
    for (int i = 0; i < ts.Count; ++i)
        r.push_back(std::string(AnsiString(ts.Strings[i]).c_str()));
    return r;
}
}  // namespace viewer

// ---- the viewer's two bodies NOW (cObserver.cpp:1369-1385 since a9b93d61, W15 = B), VERBATIM: this build compiles
//      them (cObserver.cpp itself is in ht9045_sm, which this test does not link), section 9 checks that the text
//      below is still the text of cObserver.cpp (kViewerNowSource) and that it gives what the pre-W15 bodies gave ----
namespace viewer_now {
using vclcompat::AnsiString;
using vclcompat::TStringList;
static void AddEventLogCsvField(TStringList *tsRow, const std::string &sField)
{
    tsRow->Add(AnsiString(sField));                                             // one field, as split
}
//---------------------------------------------------------------------------
static void SplitEventLogCsvLine(const AnsiString &asLine, TStringList *tsRow)
{
    tsRow->Clear();

    const std::vector<std::string> vField=
        ela::SplitEventLogCsv(std::string(asLine.c_str(), (size_t)asLine.Length()));

    for(size_t i=0; i<vField.size(); i++)
    {
        AddEventLogCsvField(tsRow, vField[i]);
    }
}
static ela::Row Split(const std::string& line)
{
    TStringList ts;
    SplitEventLogCsvLine(AnsiString(line), &ts);
    ela::Row r;
    for (int i = 0; i < ts.Count; ++i)
        r.push_back(std::string(AnsiString(ts.Strings[i]).c_str()));
    return r;
}
}  // namespace viewer_now

// the same two functions as text, to find in cObserver.cpp (once) and in this file (twice: here and viewer_now)
static const char kViewerNowSource[] = R"CODE(static void AddEventLogCsvField(TStringList *tsRow, const std::string &sField)
{
    tsRow->Add(AnsiString(sField));                                             // one field, as split
}
//---------------------------------------------------------------------------
static void SplitEventLogCsvLine(const AnsiString &asLine, TStringList *tsRow)
{
    tsRow->Clear();

    const std::vector<std::string> vField=
        ela::SplitEventLogCsv(std::string(asLine.c_str(), (size_t)asLine.Length()));

    for(size_t i=0; i<vField.size(); i++)
    {
        AddEventLogCsvField(tsRow, vField[i]);
    }
}
)CODE";
#ifndef W906_ELA_SRC_DIR
#define W906_ELA_SRC_DIR ""
#endif
static size_t CountOf(const std::string& hay, const std::string& needle)
{
    size_t n = 0;
    for (size_t k = hay.find(needle); k != std::string::npos; k = hay.find(needle, k + 1)) ++n;
    return n;
}

// ---- the W23 fixture tree: copies of D:\HT9045_Log\EventLogTxt samples (tests/fixtures/ela), copied again into
//      %TEMP%\ht9045_ela_core_fx\EventLogTxt\<the real yyyy\mm[\dd] folders> before any read ----
#ifndef W906_ELA_FIXTURE_DIR
#define W906_ELA_FIXTURE_DIR ""
#endif
static const char* const kFixtures[] = {
    "2025\\09\\HT-9016C_PMLD1019_EventLogTxt_20250910.csv",         // (a) 490 lines, 8 unquoted "16 System,MES1640" rows
    "2025\\08\\HT9046LS_JFTH013_EventLogTxt_20250825.csv",          // (b) day file; section 11 makes its AllEventLog copy
    "2024\\07\\01\\HT-9045W_HHT-55_EventLogTxt_202407010800.csv",   // (c) 12 h, 08:00-20:00
    "2024\\07\\01\\HT-9045W_HHT-55_EventLogTxt_202407012000.csv",   // (c) 12 h, 20:00-08:00 (lines 0-201 + 2480-end kept)
    "2023\\09\\01\\EventLogTxt_20230901 00.csv",                    // hour-named files
    "2023\\09\\01\\EventLogTxt_20230901 12.csv",
    "2025\\11\\EventLogTxt_202511.csv",                             // month file
};
static std::string g_fxRoot, g_fx;   // %TEMP%\ht9045_ela_core_fx and its EventLogTxt

static void MkDirs(const std::string& path)
{
    for (size_t i = 3; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '\\')
            ::CreateDirectoryA(path.substr(0, i).c_str(), 0);
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0)
                continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                RemoveTree(p);
            else
                ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static std::string ReadBytes(const std::string& p, bool* ok)
{
    std::string s;
    *ok = false;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    *ok = true;
    return s;
}

static bool WriteBytes(const std::string& p, const std::string& bytes)
{
    const size_t slash = p.find_last_of('\\');
    if (slash != std::string::npos) MkDirs(p.substr(0, slash));
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(bytes.data(), 1, bytes.size(), f) == bytes.size();
    std::fclose(f);
    return ok;
}

// returns how many fixtures were copied (all of kFixtures when the tree is complete)
static int SetUpFixtureTree()
{
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    g_fxRoot = std::string(tmp) + "ht9045_ela_core_fx";
    g_fx = g_fxRoot + "\\EventLogTxt";
    RemoveTree(g_fxRoot);                                  // only ever this %TEMP% sandbox
    MkDirs(g_fx);
    int n = 0;
    for (size_t i = 0; i < sizeof(kFixtures) / sizeof(kFixtures[0]); ++i)
    {
        bool ok = false;
        const std::string b = ReadBytes(std::string(W906_ELA_FIXTURE_DIR) + "/EventLogTxt/" + kFixtures[i], &ok);
        if (ok && !b.empty() && WriteBytes(g_fx + "\\" + kFixtures[i], b)) ++n;
    }
    return n;
}

static bool Contains(const std::vector<std::string>& v, const char* t)
{
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i].find(t) != std::string::npos) return true;
    return false;
}

static const char* kHeader = "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n";

// AI(W906-ELA-W18) 20260927: one Production_Log row of 28 fields (LoadTime / OrderTest / TestTime set, the rest "x")
static std::string ProdRow(const char* loadTime, const char* orderTest, const char* testTime)
{
    std::string r;
    for (int c = 0; c < 28; ++c)
    {
        if (c) r += ",";
        r += (c == ela::eLoadTime) ? loadTime : (c == ela::eOrderTest) ? orderTest : (c == ela::eTestTime) ? testTime : "x";
    }
    return r + "\r\n";
}

// AI(W906-ELA-REV) 20260928 (St02-E): refuse-first -- no root or seam of this test may resolve into the machine's data
static bool MachineDataPath(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) s[i] = (s[i] == '/') ? '\\' : (char)std::tolower((unsigned char)s[i]);
    static const char* const k[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary" };   // d:\ht9045 also covers d:\ht9045_log
    for (size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
        if (s.compare(0, std::strlen(k[i]), k[i]) == 0) return true;
    return false;
}

int main()
{
    printf("ELA_Core\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    g_root = std::string(tmp) + "ht9045_ela_core";
    if (MachineDataPath(g_root) || MachineDataPath(g_root + "_fx"))   // before anything is written
    {
        printf("  REFUSED: sandbox %s is under the machine's data (D:\\HT9045*, D:\\RMS, D:\\MTBF_Summary)\n", g_root.c_str());
        return 1;
    }
    MkDir(g_root);
    MkDir(g_root + "\\EventLogTxt");
    MkDir(g_root + "\\EventLogTxt\\2026");
    MkDir(g_root + "\\EventLogTxt\\2026\\04");

    // 1. CommaText
    {
        using ela::Row;
        CHECK(ela::BcbCommaText("16 System,x") == Row({ "16", "System", "x" }), "1. unquoted blank splits the field (BCB6)");
        CHECK(ela::BcbCommaText("\"16 System\",x") == Row({ "16 System", "x" }), "1. quoted field keeps its blank");
        CHECK(ela::BcbCommaText("\"a\"\"b\",c") == Row({ "a\"b", "c" }), "1. doubled quote");
        CHECK(ela::BcbCommaText("  a , b ") == Row({ "a", "b" }), "1. blanks around items skipped");
        CHECK(ela::BcbCommaText("a,") == Row({ "a", "" }), "1. trailing comma adds an empty item");
        CHECK(ela::BcbCommaText(",a") == Row({ "", "a" }), "1. leading comma gives an empty first item");
        CHECK(ela::BcbCommaText("\"abc") == Row({ "ab" }), "1. missing closing quote drops the last char (RTL quirk)");
        CHECK(ela::BcbCommaText("\"a\"\"b\"\"c\",d") == Row({ "a\"b\"c", "d" }), "1. two doubled quotes");
        CHECK(ela::BcbGetCommaText(Row({ "16 System", "x" })) == "\"16 System\",x", "1. GetCommaText quotes a blank");
        CHECK(ela::BcbGetCommaText(Row({ "a\"b" })) == "\"a\"\"b\"", "1. GetCommaText doubles quotes");
        CHECK(ela::BcbGetCommaText(Row({ "" })) == "\"\"", "1. GetCommaText of one empty item");
        CHECK(ela::SplitEventLogCsv("a,16 System,\"x,y\"") == Row({ "a", "16 System", "x,y" }),
              "1. #15 W15 B SplitEventLogCsv: comma-only split");
    }

    // 2. dates
    {
        CHECK(ela::EncodeDate(1899, 12, 30) == 0.0 && ela::EncodeDate(2026, 4, 1) == 46113.0, "2. EncodeDate epoch");
        CHECK(ela::IsValidDateString("2026/04/01") && ela::IsValidDateString("2026-04-01") &&
                  !ela::IsValidDateString("2026/04-01") && !ela::IsValidDateString("26/04/01"), "2. IsValidDateString");
        CHECK(ela::IsValidTimeString("08:00:00") && ela::IsValidTimeString("08:00:00.123") &&
                  !ela::IsValidTimeString("8:00:00") && !ela::IsValidTimeString("24:00:00"), "2. IsValidTimeString");
        CHECK(ela::ParseDateTime("2026/02/30", "00:00:00") == 0.0, "2. 2026/02/30 -> TDateTime() (EncodeDate raised)");
        const double t = ela::ParseDateTime("2026/04/01", "08:30:15");
        CHECK(ela::FormatYMD(t) == "2026/04/01" && ela::FormatYMDH00(t) == "2026/04/01 08:00:00" &&
                  ela::FormatYYYYMMDD(t) == "20260401", "2. formatting");
    }

    // fixtures
    const std::string day1 = g_root + "\\EventLogTxt\\2026\\04\\EventLogTxt_20260401.csv";
    const std::string day2 = g_root + "\\EventLogTxt\\2026\\04\\EventLogTxt_20260402.csv";
    const std::string decoy = g_root + "\\EventLogTxt\\2026\\04\\Other_20260401.csv";
    const std::string jamIni = g_root + "\\JAM0000.dat";
    std::string d1 = kHeader;
    d1 += "2026/04/01,08:10:00,\"01 Input Arm\",JAM0101,1,30,0,\"Pick fail\",,R1\r\n";          // JAM, MTBA (default true)
    d1 += "2026/04/01,08:20:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n";            // WAR, MTBF (24 Motor)
    d1 += "2026/04/01,09:05:00,\"15 Temp. Controller\",MES1501,1,5,0,\"Temperature high\",,R1\r\n";   // MES, By-Func Temp
    d1 += "2026/04/01,09:30:00,\"01 Input Arm\",JAM0101,1,10,1,\"Pick fail\",,R1\r\n";          // Duplicate -> skipped
    d1 += "2026/04/01,10:00:00,16 System,WAR1601,1,7,0,\"System\",,R1\r\n";                     // BCB split -> no code
    d1 += "2026/04/01,11:00:00,\"22 Motion Log\",WAR2201,1,3,0,\"Motion\",,R1\r\n";              // WAR in Motion: not WAR count; MTBF list
    d1 += "2026/03/31,23:00:00,\"01 Input Arm\",JAM0101,1,99,0,\"Pick fail\",,R1\r\n";          // out of range
    d1 += "2026/02/30,08:00:00,\"01 Input Arm\",JAM0101,1,99,0,\"Pick fail\",,R1\r\n";          // invalid date
    WriteFile(day1, d1.c_str());
    std::string d2 = kHeader;
    d2 += "2026/04/02,00:30:00,\"02 Output Arm\",JAM0201,1,40,0,\"RTC error\",,R1\r\n";          // JAM, MTBA, By-Func RTC
    WriteFile(day2, d2.c_str());
    WriteFile(decoy, d1.c_str());                                                                  // wrong file name
    WriteFile(jamIni, "[01 Input Arm]\r\n");
    std::vector<std::string> files;
    files.push_back(decoy);
    files.push_back(day1);
    files.push_back(day2);

    const double sd = ela::EncodeDate(2026, 4, 1), st = ela::EncodeTime(8, 0, 0, 0);
    const double ed = ela::EncodeDate(2026, 4, 2), et = ela::EncodeTime(23, 59, 59, 0);

    // 3 + 4 + 5: golden mode -- AI(W906-ELA-W15) 20260927: the golden switches are set explicitly (the defaults are the
    //   rulings now); this block is the oracle regression for the reference exe (#23)
    {
        ela::Options o;
        o.jamIniPath = jamIni;
        o.bcbCommaText = true;             // #15 golden BCB6 CommaText
        o.keepGoldenBugs = true;           // #18 golden's bugs
        o.goldenFileRule = true;           // #19 golden per-day file-name match
        ela::Analyzer a(o);
        a.SetRange(sd, st, ed, et);
        CHECK(a.iDate == 2, "3. iDate = 2");
        CHECK(a.byDayKeys.size() == 2 && a.byDayKeys[0] == "2026/04/01" && a.byDayKeys[1] == "2026/04/02", "3. by-day keys");
        CHECK(a.byHourKeys.size() == 40 && a.byHourKeys[0] == "2026/04/01 08:00:00" &&
                  a.byHourKeys.back() == "2026/04/02 23:00:00", "3. by-hour keys 08:00 .. next day 23:00");
        CHECK(a.byHourKeys[16] == "2026/04/02 00:00:00",
              "3. 08:00 + 16 x 1/24 is 04/02 00:00, not 04/01 24:00 (DateTimeToTimeStamp rounds the whole value)");
        CHECK(ela::FormatYMDH00(ela::EncodeDate(2026, 4, 1) + 0.99999999999) == "2026/04/02 00:00:00",
              "3. a value 1 us short of midnight carries into the date");

        a.GetEventLogText(files);
        const ela::Summary& s = a.MySummary;
        CHECK(a.filesRead.size() == 2, "4. only the two EventLogTxt_<date>.csv files are read");
        CHECK(s.iStopCount == 5 && s.iStopTime == 98, "4. stop 5 rows / 98 s (duplicate, out-of-range, 16 System excluded)");
        CHECK(s.iAlarmCount == 2 && s.iAlarmTime == 70, "4. MTBA: the two JAMs in units 01/02");
        CHECK(s.iFailCount == 2 && s.iFailTime == 23, "4. MTBF: 24 Motor + WAR2201");
        CHECK(s.iJAMCount == 2 && s.iWARCount == 1 && s.iMESCount == 1, "4. JAM 2 / WAR 1 (Motion excluded) / MES 1");
        CHECK(a.mapByDayList["2026/04/01"].iStopTime == 58 && a.mapByDayList["2026/04/01"].iAlarmCount == 1 &&
                  a.mapByDayList["2026/04/01"].iFailCount == 2 && a.mapByDayList["2026/04/01"].iFailTime == 23,
              "4. by day 04/01");
        CHECK(a.mapByDayList["2026/04/02"].iStopTime == 40 && a.mapByDayList["2026/04/02"].iAlarmTime == 40, "4. by day 04/02");
        CHECK(a.mapByHourList["2026/04/01 08:00:00"].iStopTime == 50 &&
                  a.mapByHourList["2026/04/01 11:00:00"].iFailTime == 3 &&
                  a.mapByHourList["2026/04/02 00:00:00"].iAlarmCount == 1, "4. by hour");
        CHECK(a.mapJamSummary.size() == 2 && a.mapJamSummary["JAM0101"].iStopTime == 30 &&
                  a.mapWarSummary.size() == 1 && a.mapAlarmSummary.size() == 3, "4. Jam / War / Alarm summaries");
        CHECK(a.ListByUnitName[0].size() == 1 && a.ListByUnitName[23].size() == 1 && a.ListByUnitName[21].size() == 1,
              "4. By-Area: 01 Input Arm, 24 Motor, 22 Motion Log");
        CHECK(a.ListByFunction[ela::efTemp].size() == 1 && a.ListByFunction[ela::efRTC].size() == 1, "4. By-Function Temp / RTC");
        CHECK(a.slStopList.size() == 5 && a.slStopList[0].find("\"01 Input Arm\"") != std::string::npos,
              "4. lists hold golden CommaText rows");

        CHECK(a.jam.writes == 10, "5. JAM0000.dat: 5 stop rows x (IncludeMTBA + IncludeMTBF) written back (#17 A)");
        const std::string ini = ReadAll(jamIni);
        CHECK(ini.find("JAM0101 IncludeMTBA=1") != std::string::npos && ini.find("WAR2401 IncludeMTBF=1") != std::string::npos,
              "5. defaults written as golden (1 = true)");

        // 6. second query on the same Analyzer: golden leaks
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        CHECK(a.jam.writes == 10, "6. second run writes nothing more");
        CHECK(a.MySummary.iFailCount == 4 && a.MySummary.iStopCount == 5, "6. iFailCount leaks to 4 (#18 A), others reset");
        CHECK(a.ListByUnitName[0].size() == 2, "6. By-Area list keeps growing (#18 A)");
    }

    // 6b. keepGoldenBugs=false; 5b. jamWriteBack=false; 7. comma-only
    {
        WriteFile(jamIni, "[01 Input Arm]\r\n");
        ela::Options o;
        o.jamIniPath = jamIni;
        o.bcbCommaText = true;             // golden except #18
        o.goldenFileRule = true;
        o.keepGoldenBugs = false;
        o.jamWriteBack = false;
        ela::Analyzer a(o);
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        CHECK(a.MySummary.iFailCount == 2 && a.ListByUnitName[0].size() == 1, "6b. keepGoldenBugs=false resets both");
        CHECK(a.jam.writes == 0 && ReadAll(jamIni) == "[01 Input Arm]\r\n", "5b. jamWriteBack=false leaves JAM0000.dat alone");

        ela::Options b;
        b.jamIniPath = jamIni;
        b.jamWriteBack = false;
        b.bcbCommaText = false;
        ela::Analyzer c(b);
        c.SetRange(sd, st, ed, et);
        c.GetEventLogText(files);
        CHECK(c.MySummary.iStopCount == 6 && c.MySummary.iWARCount == 2, "7. #15 B counts the unquoted 16 System row");
    }

    // 8. P1b
    {
        CHECK(ela::ConvertSecToTime(3661) == "01:01:01" && ela::ConvertSecToTime(90061) == "1 days 01:01:01" &&
                  ela::ConvertSecToTime(5, 0) == "00:00:05.000", "8. ConvertSecToTime");
        CHECK(ela::ConvertDTToTime(1.5) == "1 days 12:00:00" && ela::ConvertDTToTime(0.25) == "06:00:00", "8. ConvertDTToTime");
        double v = 0.0;
        // compared the way golden uses a TDateTime: through DateTimeToTimeStamp (whole milliseconds), not double ==
        CHECK(ela::StrToDateTimeZhTw("2026/04/01 08:10:00", &v) &&
                  ela::DateTimeToMs(v) == 46113LL * 86400000LL + 8LL * 3600000LL + 10LL * 60000LL, "8. StrToDateTime date + time");
        CHECK(ela::StrToDateTimeZhTw("00:00:03", &v) && ela::DateTimeToMs(v) == 3000LL, "8. StrToDateTime time only");
        CHECK(ela::StrToDateTimeZhTw("12:34:56.7", &v) && ela::DateTimeToMs(v) == 45296007LL, "8. '.7' is 7 ms (ScanNumber)");
        CHECK(!ela::StrToDateTimeZhTw("", &v) && !ela::StrToDateTimeZhTw("2026/13/01", &v), "8. StrToDateTime errors");
        const std::vector<std::string> fav = ela::LoadFavorite(g_root + "\\EventLogTxt");
        CHECK(fav.size() == 3, "8. LoadFavorite finds the three .csv files recursively");

        WriteFile(jamIni, "[01 Input Arm]\r\n");
        ela::Options o;
        o.jamIniPath = jamIni;
        o.bcbCommaText = true;             // golden mode (see block 3)
        o.keepGoldenBugs = true;
        o.goldenFileRule = true;
        ela::Analyzer a(o);
        a.sHandlerID = "H1";
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);

        a.UpdateSgTop5Filter(0);
        CHECK(a.sgTop5Filter.size() >= 2 && a.sgTop5Filter[0][0] == "Top 1" && a.sgTop5Filter[0][2] == "JAM0101" &&
                  a.sgTop5Filter[0][4] == "1" && a.sgTop5Filter[0][5] == "30" && a.sgTop5Filter[1][2] == "JAM0201",
              "8. Top5Filter JAM: count desc, then code asc");
        CHECK(a.sgTop5Alarm.size() == 2 && a.iTop5FilterRowCount == 6, "8. Top5Alarm rows = JAM list; five visible filter rows");
        a.UpdateSgTop5(1);
        CHECK(a.sgTop5.size() == 1 && a.sgTop5[0][0] == "2026/04/01" && a.sgTop5[0][3] == "Pick fail", "8. Top5 rows of JAM0101");
        a.UpdateSgTop5Filter(1);
        CHECK(a.sgTop5Filter[0][2] == "WAR2401" && a.sgTop5Filter[1][2] == "JAM0201",
              "8. golden quirk: a filter switch leaves the old second row");
        a.UpdateSgFailAndAlarm();
        CHECK(a.sgAlarm.size() == 2 && a.sgFail.size() == 2 && a.sgAlarm[0][3] == "JAM0101", "8. Alarm / Fail grids");
        bool area[ela::eUnitNameTotal], func[ela::eByFuncTotal];
        for (int i = 0; i < ela::eUnitNameTotal; ++i) area[i] = true;
        for (int i = 0; i < ela::eByFuncTotal; ++i) func[i] = true;
        a.UpdateSgByFilter(0, area, func);
        CHECK(a.sgByArea.size() == 5 && a.byAreaCount == 5, "8. By Area, all units: 5 rows");
        for (int i = 1; i < ela::eUnitNameTotal; ++i) area[i] = false;
        a.UpdateSgByFilter(0, area, func);
        CHECK(a.byAreaCount == 1 && a.sgByArea[0][0] == "2026/04/01" && a.sgByArea[0][1] == "No Record!!",
              "8. golden quirk: one record gets \"No Record!!\" over its time");
        a.UpdateSgByFilter(2, area, func);
        CHECK(a.byAreaCount == 2, "8. By Function: Temp + RTC");

        // Production_Log
        MkDir(g_root + "\\Production_Log");
        const std::string prod = g_root + "\\Production_Log\\Production_20260401.csv";
        std::string pl = "h0,h1,h2,h3,h4,Load Time,h6,h7,h8,h9,h10,h11,h12,OrderTest,h14,h15,h16,h17,h18,h19,h20,h21,h22,h23,"
                         "h24,h25,h26,TestTime\r\n";
        const char* rows[4][3] = { { "2026/04/01 08:15:00", "1", "00:00:02.500" },
                                   { "2026/04/01 08:16:00", "1", "00:00:02.500" },
                                   { "2026/04/01 09:00:00", "2", "00:00:03.250" },
                                   { "2026/03/31 08:00:00", "3", "00:00:09.000" } };
        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 28; ++c)
            {
                if (c) pl += ",";
                pl += (c == ela::eLoadTime) ? rows[r][0] : (c == ela::eOrderTest) ? rows[r][1] : (c == ela::eTestTime) ? rows[r][2] : "x";
            }
            pl += "\r\n";
        }
        WriteFile(prod, pl.c_str());
        std::vector<std::string> pf;
        pf.push_back(prod);
        CHECK(a.ListProductionLog(pf), "8. ListProductionLog ran");
        CHECK(a.MySummary.iInputCount == 3 && a.MySummary.iContactCount == 2 && a.MySummary.iTestTimeMS == 750,
              "8. input 3 (header trick undone), 2 touch-downs, 750 ms");
        // expected values from the same stored inputs and the same expression golden uses (UpdateSgProduction)
        const ela::Summary& sd1 = a.mapByDayList["2026/04/01"];
        const long dayMs = (long)(double(sd1.dtTestTime) * double(86400000L) + double(sd1.iTestTimeMS));
        const ela::Row& d0 = a.sgByDay[0];
        CHECK(d0[ela::iSgInputQty] == "3" && d0[ela::iSgUPH] == "0" && d0[ela::iSgContactCnt] == "2" && d0[ela::iSgMUBA] == "3" &&
                  d0[ela::iSgMTBA] == "1 days 00:00:00" && d0[ela::iSgFaliureCnt] == "2" && d0[ela::iSgMTBF] == "12:00:00" &&
                  d0[ela::iSgTotalStopTime] == "00:00:58" && d0[ela::iSgSumAlarmTime] == "00:00:30",
              "8. sgByDay 04/01");
        CHECK(d0[ela::iSgTotalTestTime] == ela::ConvertSecToTime(dayMs / 1000, dayMs % 1000) &&
                  d0[ela::iSgAvgTestTime] == ela::ConvertSecToTime((dayMs / 2) / 1000, (dayMs / 2) % 1000),
              "8. sgByDay test time / average");
        const ela::Row& h0 = a.sgByHour[0];
        const ela::Summary& sh8 = a.mapByHourList["2026/04/01 08:00:00"];
        const long hourMs = (long)(double(sh8.dtTestTime) * double(3600000L) + double(sh8.iTestTimeMS));
        CHECK(sh8.iTestTimeMS == 500 && ela::DateTimeToMs(sh8.dtTestTime) == 2000LL, "8. 08:00 holds A's 2 s + 500 ms");
        CHECK(h0[ela::iSgDate] == "2026/04/01 08:00:00" && h0[ela::iSgInputQty] == "2" && h0[ela::iSgUPH] == "2" &&
                  h0[ela::iSgMTBA] == "01:00:00" && h0[ela::iSgTotalTestTime] == ela::ConvertSecToTime(hourMs / 1000, hourMs % 1000),
              "8. sgByHour 08:00 (golden: dtTestTime x OneHourMS)");
        CHECK(a.mmoSummary.size() == 20 && a.mmoSummary[1] == "Handler ID:        H1" &&
                  a.mmoSummary[2] == "Total Input Qty:   3" && a.mmoSummary[3] == "Time Period:       1 days 15:59:59" &&
                  a.mmoSummary[13] == "Alarm Count:       2" && a.mmoSummary[15] == "Fail Count:        2" &&
                  a.mmoSummary[17] == "Total Stop Time:   00:01:38.000", "8. mmoSummary lines");
        const long gap = (long)(double(a.dtEnd - a.dtStart) * 86400.0);
        CHECK(a.mmoSummary[5] == "MTBA:              " + ela::ConvertSecToTime((long)(double(gap) / 2)), "8. MTBA = period / alarms");

        // AI(W906-ELA-W18) 20260927 (St02-E): two more golden quirks this block pins (W18 B fixes them, section 10)
        a.UpdateSgTop5Filter(0);
        a.sgTop5Filter[0][2] = "JAM010";   // a prefix of JAM0101 -- the real pairs: WAR1520 / WAR15206, WAR1519 / WAR15194
        a.UpdateSgTop5(1);
        CHECK(a.sgTop5.size() == 1 && a.sgTop5[0][3] == "Pick fail", "8. golden quirk: Top5 matches the AlarmCode by prefix");
        const std::string prod2 = g_root + "\\Production_Log\\Production_20260402.csv";
        WriteFile(prod2, (pl.substr(0, pl.find("\r\n") + 2) + ProdRow("2026/04/02 08:00:00", "4", "00:00:01.000") +
                          "x,x,x\r\n").c_str());      // a half-written last line
        std::vector<std::string> pf2;
        pf2.push_back(prod2);
        const size_t exBefore = a.exceptions.size();
        CHECK(!a.ListProductionLog(pf2) && a.exceptions.size() == exBefore + 1 &&
                  a.exceptions.back() == "List index out of bounds (5)",
              "8. golden quirk: a half-written last Production_Log line throws out of ListProductionLog");
    }

    // 9. W15 B (Steven 20260927): ela::SplitEventLogCsv -- real row shapes (W23 samples), the V906 viewer's helpers as
    //    the reference, the round trip through the analyzer's lists, and sample (a) counted with both splitters
    {
        using ela::Row;
        const std::string tab = "\t";
        const char* const L1 = "2024-07-01, 20:00:00.093, Process,\"\t\",\"\t\",\"\t\",\"\t\",\"By time OEE record : 2024-07-01 20:00 3552198\",";
        const char* const L2 = "2024-07-01, 08:00:04.078, Process,\"\t\",\"\t\",\"\t\",\"\t\","
                               "\"XCOPY /y/a/e/c/i/h/f/r \"\"D:\\HT9045_Log\\a.csv\"\" \"\"D:\\HT9045_Log\\Jamstat\"\"\",";
        const char* const L3 = "2025-09-10, 13:30:01.001,16 System,MES1640,,0,0,\"One cycle finish\", ,R1";
        const char* const L4 = "2025-09-10,\" 08:45:33.712\",\"16 System\",WAR16100,1,5,0,\"msg\",,R1";
        CHECK(ela::SplitEventLogCsv(L1) == Row({ "2024-07-01", "20:00:00.093", "Process", tab, tab, tab, tab,
                                                 "By time OEE record : 2024-07-01 20:00 3552198", "" }),
              "9. blanks around a field are trimmed (Process), the quoted TAB placeholder stays a TAB, trailing comma = one more field");
        const Row r2 = ela::SplitEventLogCsv(L2);
        CHECK(r2.size() == 9 && r2[7] == "XCOPY /y/a/e/c/i/h/f/r \"D:\\HT9045_Log\\a.csv\" \"D:\\HT9045_Log\\Jamstat\"",
              "9. XCOPY row: a doubled quote inside a quoted field becomes one quote");
        CHECK(ela::SplitEventLogCsv(L3) == Row({ "2025-09-10", "13:30:01.001", "16 System", "MES1640", "", "0", "0",
                                                 "One cycle finish", "", "R1" }),
              "9. an unquoted 16 System stays one field (BCB6 CommaText splits it); a blank field is empty");
        const Row r4 = ela::SplitEventLogCsv(L4);
        CHECK(r4.size() == 10 && r4[1] == " 08:45:33.712" && r4[2] == "16 System" && r4[3] == "WAR16100" &&
                  ela::IsValidTimeString(r4[1]), "9. a quoted field keeps its leading blank; the time validator trims it");
        CHECK(ela::SplitEventLogCsv("a,\"b,c") == Row({ "a", "b,c" }), "9. an unclosed quote runs to the end of the line");
        CHECK(ela::SplitEventLogCsv("") == Row({ "" }) && ela::SplitEventLogCsv("a,") == Row({ "a", "" }),
              "9. an empty line is one empty field; a trailing comma adds one");
        CHECK(ela::SplitEventLogCsv("\"a\"\"b\",c") == Row({ "a\"b", "c" }) &&
                  ela::SplitEventLogCsv("a\"\"b,c") == Row({ "a\"\"b", "c" }),
              "9. a doubled quote becomes one only in a field that starts with a quote (the viewer's rule)");
        CHECK(ela::SplitEventLogCsv("\xA4\xA4\xA4\xE5,\"\xB4\xFA \xB8\xD5\",x") ==
                  Row({ "\xA4\xA4\xA4\xE5", "\xB4\xFA \xB8\xD5", "x" }), "9. cp950 bytes are split as bytes");
        const char* const shapes[] = { L1, L2, L3, L4, "a,\"b,c", "", "a,", "\"a\"\"b\",c", "a\"\"b,c", "  a , b ", "\"\"",
                                       "\"", ",", "a,,b", "\"x\"y\",z", "\"\t\"", " \" q \" ,x" };
        bool same = true;
        for (size_t i = 0; i < sizeof(shapes) / sizeof(shapes[0]); ++i)
            same = same && ela::SplitEventLogCsv(shapes[i]) == viewer::Split(shapes[i]) &&
                   viewer_now::Split(shapes[i]) == viewer::Split(shapes[i]);
        CHECK(same, "9. every shape above: SplitEventLogCsv and cObserver's bodies now == the pre-W15 bodies");
        {
            // the compiled viewer_now copy IS cObserver.cpp's text (read-only reads of two repo sources)
            bool okC = false, okT = false;
            std::string cobs = ReadBytes(std::string(W906_ELA_SRC_DIR) + "/cObserver.cpp", &okC);
            std::string self = ReadBytes(std::string(W906_ELA_SRC_DIR) + "/tests/test_ela_core.cpp", &okT);
            for (size_t k = cobs.find('\r'); k != std::string::npos; k = cobs.find('\r', k)) cobs.erase(k, 1);
            for (size_t k = self.find('\r'); k != std::string::npos; k = self.find('\r', k)) self.erase(k, 1);
            CHECK(okC && okT && CountOf(cobs, kViewerNowSource) == 1 && CountOf(self, kViewerNowSource) == 2,
                  "9. viewer_now is the text of cObserver.cpp:1369-1385 (W906_ELA_SRC_DIR)");
        }

        // the W23 copies: every line, both ways
        CHECK(std::strlen(W906_ELA_FIXTURE_DIR) > 0, "9. W906_ELA_FIXTURE_DIR is set (tests/CMakeLists.txt)");
        const int copied = SetUpFixtureTree();
        CHECK(copied == (int)(sizeof(kFixtures) / sizeof(kFixtures[0])), "9. the W23 fixture copies are in %TEMP%\\ht9045_ela_core_fx");
        size_t lines = 0, sameN = 0, roundTrip = 0;
        for (size_t f = 0; f < sizeof(kFixtures) / sizeof(kFixtures[0]); ++f)
        {
            bool ok = false;
            const std::vector<std::string> ls = ela::BcbLoadLines(g_fx + "\\" + kFixtures[f], &ok);
            for (size_t i = 0; i < ls.size(); ++i)
            {
                const Row r = ela::SplitEventLogCsv(ls[i]);
                ++lines;
                const Row cur = viewer::Split(ls[i]);
                if (r == cur && viewer_now::Split(ls[i]) == cur) ++sameN;
                if (ela::BcbCommaText(ela::BcbGetCommaText(r)) == r) ++roundTrip;
            }
        }
        CHECK(lines >= 490 && sameN == lines,
              "9. every fixture line: SplitEventLogCsv and cObserver's bodies now == the pre-W15 bodies");
        CHECK(roundTrip == lines, "9. every fixture row survives BcbGetCommaText -> BcbCommaText (how the lists store rows)");

        // sample (a) 2025-09-10, one whole day, with each splitter
        const std::string noJam = g_root + "\\no_JAM0000.dat";      // absent: MTBA / MTBF look-ups read nothing
        ::DeleteFileA(noJam.c_str());
        const std::vector<std::string> fav = ela::LoadFavorite(g_fx);
        const double d0910 = ela::EncodeDate(2025, 9, 10), e0910 = ela::EncodeTime(23, 59, 59, 0);
        ela::Options og;
        og.jamIniPath = noJam;
        og.bcbCommaText = true;
        og.keepGoldenBugs = true;
        og.goldenFileRule = true;          // golden mode
        ela::Analyzer ga(og);
        ga.SetRange(d0910, 0.0, d0910, e0910);
        ga.GetEventLogText(fav);
        CHECK(ga.filesRead.size() == 1 && ga.MySummary.iStopCount == 123 && ga.MySummary.iStopTime == 0 &&
                  ga.MySummary.iMESCount == 115, "9. sample (a), golden mode (CommaText): 123 stop rows / 0 s / MES 115");
        ela::Options on;
        on.jamIniPath = noJam;
        ela::Analyzer na(on);
        na.SetRange(d0910, 0.0, d0910, e0910);
        na.GetEventLogText(fav);
        CHECK(Contains(na.filesRead, "EventLogTxt_20250910.csv") && na.MySummary.iStopCount == 131 &&
                  na.MySummary.iStopTime == 309 && na.MySummary.iMESCount == 123,
              "9. sample (a), W15 B: 131 stop rows / 309 s / MES 123 (the 8 unquoted 16 System rows count)");
        CHECK(ga.jam.writes == 0 && na.jam.writes == 0 && ::GetFileAttributesA(noJam.c_str()) == INVALID_FILE_ATTRIBUTES,
              "9. no JAM0000.dat: nothing written");
    }

    // 10. the files of sections 3-8 with the DEFAULT Options = Steven's rulings (W15 B + W18 B + W19 B; W17 A) -- the flipped
    //     expectations of research-ela-w15-w18-w19.md §4.  AI(W906-ELA-W18) 20260927 (St02-E)
    {
        WriteFile(jamIni, "[01 Input Arm]\r\n");
        ela::Options o;
        o.jamIniPath = jamIni;
        CHECK(!o.bcbCommaText && !o.keepGoldenBugs && !o.goldenFileRule && o.jamWriteBack && o.goldenFileRuleVtest,
              "10. defaults = the rulings (W15 B, W18 B, W19 B, W17 A; W38 B: golden file rule on VTEST 915 / 919)");
        ela::Analyzer a(o);
        a.sHandlerID = "H1";
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        const ela::Summary& s = a.MySummary;
        CHECK(a.filesRead.size() == 2 && a.dupRowsSkipped == 0,
              "10. W19: the two EventLogTxt_<date>.csv files (the decoy name has no EventLogTxt_), no duplicate rows");
        CHECK(s.iStopCount == 6 && s.iStopTime == 105, "10. W15: stop 6 rows / 105 s (the unquoted 16 System row counts)");
        CHECK(s.iAlarmCount == 2 && s.iAlarmTime == 70 && s.iFailCount == 2 && s.iFailTime == 23, "10. MTBA / MTBF unchanged");
        CHECK(s.iJAMCount == 2 && s.iWARCount == 2 && s.iMESCount == 1, "10. JAM 2 / WAR 2 (WAR1601 of 16 System) / MES 1");
        CHECK(a.mapByDayList["2026/04/01"].iStopTime == 65 && a.mapByDayList["2026/04/02"].iStopTime == 40, "10. by day 65 s / 40 s");
        CHECK(a.mapJamSummary.size() == 2 && a.mapWarSummary.size() == 2 && a.mapAlarmSummary.size() == 4,
              "10. Jam / War / Alarm summaries 2 / 2 / 4");
        CHECK(a.ListByUnitName[15].size() == 1 && a.ListByUnitName[15][0].find("\"16 System\"") != std::string::npos,
              "10. By-Area: the 16 System row, stored quoted (GetCommaText)");
        CHECK(a.slStopList.size() == 6, "10. slStopList 6");
        CHECK(a.jam.writes == 12, "10. JAM0000.dat: 6 stop rows x (IncludeMTBA + IncludeMTBF) written back (#17 A)");

        // second query on the same Analyzer: W18 B -- nothing carries over
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        CHECK(a.jam.writes == 12, "10. second run writes nothing more");
        CHECK(a.MySummary.iFailCount == 2 && a.MySummary.iStopCount == 6, "10. W18: iFailCount reset (2, golden 4)");
        CHECK(a.ListByUnitName[0].size() == 1, "10. W18: the By-Area list is cleared per query (golden 2)");

        // the grids
        a.UpdateSgTop5Filter(0);
        CHECK(a.sgTop5Filter.size() == 2 && a.sgTop5Filter[0][0] == "Top 1" && a.sgTop5Filter[0][2] == "JAM0101" &&
                  a.sgTop5Filter[1][0] == "Top 2" && a.sgTop5Filter[1][2] == "JAM0201",
              "10. W18: Top5Filter holds its two summary rows only (golden labels ten rows)");
        CHECK(a.sgTop5Alarm.size() == 2 && a.iTop5FilterRowCount == 6, "10. Top5Alarm = the JAM rows; six visible filter rows");
        a.UpdateSgTop5(1);
        CHECK(a.sgTop5.size() == 1 && a.sgTop5[0][0] == "2026/04/01" && a.sgTop5[0][3] == "Pick fail", "10. Top5 rows of JAM0101");
        a.sgTop5Filter[0][2] = "JAM010";   // a prefix of JAM0101 -- the real pairs: WAR1520 / WAR15206, WAR1519 / WAR15194
        a.UpdateSgTop5(1);
        CHECK(a.sgTop5.empty(), "10. W18: Top5 takes the code itself (golden's prefix match took JAM0101)");
        a.UpdateSgTop5Filter(1);
        CHECK(a.sgTop5Filter.size() == 2 && a.sgTop5Filter[0][2] == "WAR1601" && a.sgTop5Filter[1][2] == "WAR2401",
              "10. W18: a filter switch leaves no row of the earlier filter (WAR1601, WAR2401)");
        a.UpdateSgFailAndAlarm();
        CHECK(a.sgAlarm.size() == 2 && a.sgFail.size() == 2 && a.sgAlarm[0][3] == "JAM0101", "10. Alarm / Fail grids");
        bool area[ela::eUnitNameTotal], func[ela::eByFuncTotal];
        for (int i = 0; i < ela::eUnitNameTotal; ++i) area[i] = true;
        for (int i = 0; i < ela::eByFuncTotal; ++i) func[i] = true;
        a.UpdateSgByFilter(0, area, func);
        CHECK(a.sgByArea.size() == 6 && a.byAreaCount == 6, "10. By Area, all units: 6 rows (16 System counts)");
        for (int i = 1; i < ela::eUnitNameTotal; ++i) area[i] = false;
        a.UpdateSgByFilter(0, area, func);
        CHECK(a.byAreaCount == 1 && a.sgByArea.size() == 1 && a.sgByArea[0][1] == "08:10:00",
              "10. W18: one record keeps its Time (golden wrote No Record!! over it)");
        area[0] = false;
        a.UpdateSgByFilter(0, area, func);
        CHECK(a.byAreaCount == 0 && a.sgByArea.size() == 1 && a.sgByArea[0][1] == "No Record!!", "10. an empty table: No Record!!");
        a.UpdateSgByFilter(2, area, func);
        CHECK(a.byAreaCount == 2, "10. By Function: Temp + RTC");

        // Production_Log (the file of section 8)
        const std::string prod = g_root + "\\Production_Log\\Production_20260401.csv";
        std::vector<std::string> pf;
        pf.push_back(prod);
        CHECK(a.ListProductionLog(pf) && a.MySummary.iInputCount == 3 && a.MySummary.iContactCount == 2,
              "10. ListProductionLog: input 3, 2 touch-downs");
        const ela::Row& d0 = a.sgByDay[0];
        CHECK(d0[ela::iSgTotalStopTime] == "00:01:05" && d0[ela::iSgSumAlarmTime] == "00:00:30" && d0[ela::iSgFaliureCnt] == "2",
              "10. sgByDay 04/01: stop 00:01:05 (golden 00:00:58)");
        const ela::Summary& sh8 = a.mapByHourList["2026/04/01 08:00:00"];
        const long hourMs = (long)(double(sh8.dtTestTime) * double(86400000L) + double(sh8.iTestTimeMS));
        CHECK(a.sgByHour[0][ela::iSgTotalTestTime] == ela::ConvertSecToTime(hourMs / 1000, hourMs % 1000) &&
                  hourMs / 1000 == 2 && a.sgByHour[0][ela::iSgMTBA] == "01:00:00",
              "10. W18: sgByHour 08:00 test time = dtTestTime x OneDayMS (A's 2 s + 500 ms; golden x OneHourMS gave 0.58 s); MTBA period 1 h");
        CHECK(a.mmoSummary.size() == 20 && a.mmoSummary[17] == "Total Stop Time:   00:01:45.000" &&
                  a.mmoSummary[15] == "Fail Count:        2", "10. mmoSummary: stop 00:01:45.000 (golden 00:01:38.000)");

        // W18 B: a half-written last Production_Log line is skipped and logged; the grids and the summary still refresh
        const std::string prod2 = g_root + "\\Production_Log\\Production_20260402.csv";
        std::vector<std::string> pf2;
        pf2.push_back(prod2);
        const size_t exBefore = a.exceptions.size();
        a.mmoSummary.clear();
        CHECK(a.ListProductionLog(pf2) && a.exceptions.size() == exBefore + 1 &&
                  a.exceptions.back().find("1 bad line(s) skipped (first: List index out of bounds (5))") != std::string::npos &&
                  a.mmoSummary.size() == 20,
              "10. W18: the bad line is skipped and logged; the summary is refreshed (golden threw out)");
    }

    // 11. W19 B (Steven 20260927): the time each file name covers, the row de-duplication across files, and the W23
    //     samples of every other save period.  AI(W906-ELA-W19) 20260927 (St02-E)
    {
        double f = 0.0, t = 0.0;
        const char* const E = "X:\\HT9045_Log\\EventLogTxt\\";
        struct SpanCase { const char* name; int y, m, d, h; long long hours; };
        const SpanCase ok[] = {
            { "2024\\07\\HT-9045W_HHT-55_EventLogTxt_20240701.csv", 2024, 7, 1, 0, 24 },      // day
            { "2025\\08\\EventLogTxt_20250825_RT.csv", 2025, 8, 25, 0, 24 },                    // day, FT / RT
            { "2025\\08\\EventLogTxt_20250825_FT.CSV", 2025, 8, 25, 0, 24 },                    // any case
            { "2023\\07\\EventLogTxt_M1_20230717_TesterOffline.csv", 2023, 7, 17, 0, 24 },      // Cypress off-line
            { "2023\\09\\01\\EventLogTxt_20230901 12.csv", 2023, 9, 1, 12, 12 },                 // hour / 2..12 h
            { "2023\\09\\01\\EventLogTxt_20230901 07_FT.csv", 2023, 9, 1, 7, 17 },               // hour, FT
            { "2024\\07\\01\\HT-9045W_HHT-55_EventLogTxt_202407010800.csv", 2024, 7, 1, 8, 12 }, // 12 h, 08:00
            { "2024\\07\\01\\HT-9045W_HHT-55_EventLogTxt_202407012000.csv", 2024, 7, 1, 20, 12 },// 12 h, 20:00 -> 08:00
            { "AllEventLog\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", 2025, 8, 25, 0, 24 },    // the AllEventLog copy
        };
        bool allOk = true;
        for (size_t i = 0; i < sizeof(ok) / sizeof(ok[0]); ++i)
        {
            const long long f0 = ela::DateTimeToMs(ela::EncodeDate(ok[i].y, ok[i].m, ok[i].d)) + ok[i].h * 3600000LL;
            const bool r = ela::EventLogFileSpan(std::string(E) + ok[i].name, &f, &t);
            if (!r || ela::DateTimeToMs(f) != f0 || ela::DateTimeToMs(t) != f0 + ok[i].hours * 3600000LL)
            {
                printf("    span case %u: %s\n", (unsigned)i, ok[i].name);
                allOk = false;
            }
        }
        CHECK(allOk, "11. EventLogFileSpan: day (+_RT/_FT/_TesterOffline), hour, 12 h 0800/2000, AllEventLog");
        CHECK(ela::EventLogFileSpan(std::string(E) + "2025\\11\\EventLogTxt_202511.csv", &f, &t) &&
                  ela::DateTimeToMs(f) == ela::DateTimeToMs(ela::EncodeDate(2025, 11, 1)) &&
                  ela::DateTimeToMs(t) == ela::DateTimeToMs(ela::EncodeDate(2025, 12, 1)), "11. month file: the whole month");
        CHECK(ela::EventLogFileSpan(std::string(E) + "2025\\12\\EventLogTxt_202512.csv", &f, &t) &&
                  ela::DateTimeToMs(t) == ela::DateTimeToMs(ela::EncodeDate(2026, 1, 1)), "11. December ends at the new year");
        CHECK(ela::EventLogFileSpan(std::string(E) + "2025\\EventLogTxt_2025.csv", &f, &t) &&
                  ela::DateTimeToMs(f) == ela::DateTimeToMs(ela::EncodeDate(2025, 1, 1)) &&
                  ela::DateTimeToMs(t) == ela::DateTimeToMs(ela::EncodeDate(2026, 1, 1)), "11. year file: the whole year");
        const char* const bad[] = {
            "ByLotID\\2025\\08\\25\\HT9046LS_JFTH013_600000_20250825130115_ByLotEventLog.csv",   // no EventLogTxt_
            "2024\\07\\01\\HT-9045W_HHT-55_JamStat_202407012000.csv",
            "SGJamCount\\2024\\11\\HHT187_20241124_RawData.csv",
            "2025\\02\\EventLogTxt_20250230.csv",          // no such day
            "2025\\EventLogTxt_202513.csv",                // no such month
            "2023\\09\\01\\EventLogTxt_20230901 24.csv",   // no such hour
            "2024\\EventLogTxt_20240701_0830.TXT",         // minute files are .TXT
            "EventLogTxt_29828.csv",                       // a fixed-name O15 file (EventLogTxt_<ID>)
            "EventLogTxt.csv",
        };
        bool noneOk = true;
        for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
            if (ela::EventLogFileSpan(std::string(E) + bad[i], &f, &t))
            {
                printf("    accepted: %s\n", bad[i]);
                noneOk = false;
            }
        CHECK(noneOk, "11. not event-log files: ByLot / JamStat / RawData, bad dates, .TXT, fixed names");
        CHECK(ela::IsAllEventLogPath("X:\\EventLogTxt\\AllEventLog\\a_EventLogTxt_20250825.csv") &&
                  ela::IsAllEventLogPath("X:/eventlogtxt/alleventlog/a.csv") &&
                  !ela::IsAllEventLogPath("X:\\EventLogTxt\\2025\\08\\AllEventLogX_EventLogTxt_20250825.csv"),
              "11. IsAllEventLogPath: a folder named AllEventLog, any case");

        // row de-duplication, synthetic: repeats inside one file count (golden), the same row in another file does not
        const std::string syn = g_fxRoot + "\\syn\\EventLogTxt";
        const std::string rA = "2026/04/10,08:00:00,\"01 Input Arm\",JAM0101,1,10,0,\"Pick fail\",,R1\r\n";
        const std::string rB = "2026/04/10,09:00:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n";
        const std::string rC = "2026/04/10,10:00:00,\"02 Output Arm\",JAM0201,1,40,0,\"Place fail\",,R1\r\n";
        WriteBytes(syn + "\\2026\\04\\EventLogTxt_20260410.csv", std::string(kHeader) + rA + rA + rB);
        std::string allCopy = std::string(kHeader) + rA + rB + rC;
        for (size_t k = allCopy.find('\r'); k != std::string::npos; k = allCopy.find('\r')) allCopy.erase(k, 1);
        WriteBytes(syn + "\\AllEventLog\\EventLogTxt_20260410.csv", allCopy);   // LF only, as MyStringList.cpp:599 writes it
        const std::vector<std::string> synFiles = ela::LoadFavorite(syn);
        const std::string noJam = g_root + "\\no_JAM0000.dat";
        ::DeleteFileA(noJam.c_str());
        const double d0410 = ela::EncodeDate(2026, 4, 10), eod = ela::EncodeTime(23, 59, 59, 0);
        {
            ela::Options o;
            o.jamIniPath = noJam;
            ela::Analyzer a(o);
            a.SetRange(d0410, 0.0, d0410, eod);
            a.GetEventLogText(synFiles);
            CHECK(synFiles.size() == 2 && a.filesRead.size() == 2 && a.filesRead[1].find("AllEventLog") != std::string::npos,
                  "11. both files read, the main folder first");
            CHECK(a.MySummary.iStopCount == 4 && a.dupRowsSkipped == 2 && a.MySummary.iStopTime == 80 &&
                      a.mapJamSummary["JAM0101"].iCount == 2,
                  "11. W19: A twice (same file) + B + C (only in AllEventLog) = 4 rows / 80 s; A and B of the copy skipped");
            ela::Options g = o;
            g.bcbCommaText = true;
            g.keepGoldenBugs = true;
            g.goldenFileRule = true;
            ela::Analyzer ga(g);
            ga.SetRange(d0410, 0.0, d0410, eod);
            ga.GetEventLogText(synFiles);
            CHECK(ga.MySummary.iStopCount == 6 && ga.dupRowsSkipped == 0, "11. golden file rule: both files counted, 6 rows");
        }

        // max-line files (HTSaveType TByMaxLineCount, "<n>_yyyymmdd HHNNSS.csv", MyStringList.cpp:789): the name holds ONE
        // time and nothing about the end.  The Handler names the file when it flushes its buffer (AddTextWithDateTime
        // :431-436 -> MySaveToFile -> GetFileName), so its rows are from BEFORE that time; St02-M read it as the start.
        // SelectEventLogFiles covers both: a file runs from the previous file's time to the next file's time of the same
        // base name; the first is open at the start, the last open at the end (W19 B -- not a fixed two days).
        {
            std::string ser1, ser2, ser3, ser4, ser5;
            const bool m1 = ela::EventLogFileSpan(std::string(E) + "2026\\04\\20\\EventLogTxt_20260420 083015.csv", &f, &t, &ser1);
            CHECK(m1 && ela::DateTimeToMs(f) == ela::DateTimeToMs(ela::EncodeDate(2026, 4, 20)) + (8 * 3600 + 30 * 60 + 15) * 1000LL &&
                      t >= ela::kEventLogSpanOpenEnd && !ser1.empty(),
                  "11. a max-line file on its own: the time in its name, open-ended");
            ela::EventLogFileSpan(std::string(E) + "2026\\04\\21\\EventLogTxt_20260421 120000.csv", &f, &t, &ser2);
            ela::EventLogFileSpan(std::string(E) + "2026\\04\\21\\M2_EventLogTxt_20260421 120000.csv", &f, &t, &ser3);
            ela::EventLogFileSpan(std::string(E) + "AllEventLog\\EventLogTxt_20260421 120000.csv", &f, &t, &ser4);
            ela::EventLogFileSpan(std::string(E) + "2026\\04\\EventLogTxt_20260421.csv", &f, &t, &ser5);
            CHECK(ser1 == ser2 && ser1 != ser3 && ser1 != ser4 && ser5.empty(),
                  "11. max-line series = the base name (the AllEventLog copies apart); a day file has none");

            // three files of one base name, named 04-20 08:00 (A), 04-21 12:00 (B), 04-24 07:00 (C).  Rows as the Handler
            // writes them (before the name) plus one row after C's name (the reading "the name is the start")
            const std::string m = g_fxRoot + "\\syn3\\EventLogTxt";
            const std::string mA = m + "\\2026\\04\\20\\EventLogTxt_20260420 080000.csv";
            const std::string mB = m + "\\2026\\04\\21\\EventLogTxt_20260421 120000.csv";
            const std::string mC = m + "\\2026\\04\\24\\EventLogTxt_20260424 070000.csv";
            WriteBytes(mA, std::string(kHeader) + "2026/04/20,07:00:00,\"01 Input Arm\",JAM0101,1,10,0,\"Pick fail\",,R1\r\n");
            WriteBytes(mB, std::string(kHeader) + "2026/04/20,09:00:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n"
                                                  "2026/04/21,10:00:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n");
            WriteBytes(mC, std::string(kHeader) + "2026/04/23,08:00:00,\"15 Temp. Controller\",MES1501,1,5,0,\"Temp\",,R1\r\n"
                                                  "2026/04/27,09:00:00,\"02 Output Arm\",JAM0201,1,40,0,\"RTC error\",,R1\r\n");
            const std::vector<std::string> ml = ela::LoadFavorite(m);
            struct MQ { int d0, h0, d1, h1, n1; const char* expect; int stop, stopSec; };
            const MQ mq[] = {
                { 19, 0, 19, 23, 59, "A", 0, 0 },      // A [open, B's time)
                { 21, 0, 21, 11, 0, "AB", 1, 20 },     // B [A's time, C's time): its 04-21 10:00 row (before its name)
                { 22, 0, 22, 23, 59, "BC", 0, 0 },     // C [B's time, open)
                { 27, 0, 27, 23, 59, "C", 1, 40 },     // the last is open-ended: its 04-27 row (a fixed two days missed it)
                { 19, 0, 27, 23, 59, "ABC", 5, 95 },
            };
            bool chainOk = ml.size() == 3;
            for (size_t i = 0; i < sizeof(mq) / sizeof(mq[0]); ++i)
            {
                const double q0 = ela::EncodeDate(2026, 4, mq[i].d0) + ela::EncodeTime(mq[i].h0, 0, 0, 0);
                const double q1 = ela::EncodeDate(2026, 4, mq[i].d1) + ela::EncodeTime(mq[i].h1, mq[i].n1, 59, 0);
                const std::vector<std::string> sel = ela::SelectEventLogFiles(ml, q0, q1);
                std::string got;
                for (size_t k = 0; k < sel.size(); ++k)
                    got += (sel[k] == mA) ? "A" : (sel[k] == mB) ? "B" : (sel[k] == mC) ? "C" : "?";
                ela::Options o;
                o.jamIniPath = noJam;
                ela::Analyzer a(o);
                a.SetRange(q0, q0, q1, q1);
                a.GetEventLogText(ml);
                if (got != mq[i].expect || a.MySummary.iStopCount != mq[i].stop || a.MySummary.iStopTime != mq[i].stopSec ||
                    a.filesRead.size() != sel.size())
                {
                    printf("    max-line query %u: files %s (want %s), %d row(s) / %d s (want %d / %d)\n", (unsigned)i,
                           got.c_str(), mq[i].expect, a.MySummary.iStopCount, a.MySummary.iStopTime, mq[i].stop, mq[i].stopSec);
                    chainOk = false;
                }
            }
            CHECK(chainOk, "11. max-line files: each runs between its neighbours of the same base name; first / last open");
            ela::Options g;
            g.jamIniPath = noJam;
            g.bcbCommaText = true;
            g.keepGoldenBugs = true;
            g.goldenFileRule = true;
            ela::Analyzer ga(g);
            ga.SetRange(ela::EncodeDate(2026, 4, 19), 0.0, ela::EncodeDate(2026, 4, 27), eod);
            ga.GetEventLogText(ml);
            CHECK(ga.filesRead.empty() && ga.MySummary.iStopCount == 0, "11. golden: max-line names never match \"<yyyymmdd>.csv\"");
        }

        // VTEST (Gerneral.ini CUSTOMER_CODE 915 / 919): golden's own VTEST rule (Analyzer.cpp:807-811, "<yyyymmdd>" and
        // "_EventLogTxt_") skips a plain EventLogTxt_yyyymmdd.csv.  ★W38 B (Steven 20260928; AI(W906-ELA-W38) 20260928,
        // St02-E helper): that rule is now the DEFAULT on 915 / 919 (Options::goldenFileRuleVtest); the W19 B rule (both
        // files) only with goldenFileRuleVtest=false.  Other customer codes keep W19 B by default.
        {
            const std::string v = g_fxRoot + "\\syn4\\EventLogTxt";
            const std::string vDay = v + "\\2026\\04\\EventLogTxt_20260410.csv";
            const std::string v12 = v + "\\2026\\04\\10\\HT-9045W_HHT-55_EventLogTxt_202604100800.csv";
            WriteBytes(vDay, std::string(kHeader) + rA);            // 08:00 JAM0101 10 s
            WriteBytes(v12, std::string(kHeader) + rB);             // 09:00 WAR2401 20 s
            const std::vector<std::string> vl = ela::LoadFavorite(v);
            enum { kGolden = 0, kDefault, kVtestOff };
            struct VQ { const char* cust; int mode; size_t files; int stop, stopSec; const char* first; bool usesGolden; };
            const VQ vq[] = {
                { "919", kGolden, 1, 1, 20, "202604100800", true },   // golden VTEST: only "_EventLogTxt_" + "20260410"
                { "915", kGolden, 1, 1, 20, "202604100800", true },
                { "000", kGolden, 1, 1, 10, "EventLogTxt_20260410.csv", true },   // golden non-VTEST: only "20260410.csv"
                { "919", kDefault, 1, 1, 20, "202604100800", true },  // ★W38 B: the default on VTEST = golden's names
                { "915", kDefault, 1, 1, 20, "202604100800", true },
                { "000", kDefault, 2, 2, 30, "EventLogTxt_20260410.csv", false },  // W19 B: both (day file first)
                { "919", kVtestOff, 2, 2, 30, "EventLogTxt_20260410.csv", false }, // goldenFileRuleVtest=false: W19 B
            };
            const char* const modeName[3] = { "golden", "default", "goldenFileRuleVtest=false" };
            for (size_t i = 0; i < sizeof(vq) / sizeof(vq[0]); ++i)
            {
                ela::Options o;
                o.jamIniPath = noJam;
                o.custCode = vq[i].cust;
                if (vq[i].mode == kGolden)
                {
                    o.bcbCommaText = true;
                    o.keepGoldenBugs = true;
                    o.goldenFileRule = true;
                }
                if (vq[i].mode == kVtestOff)
                    o.goldenFileRuleVtest = false;
                ela::Analyzer a(o);
                a.SetRange(d0410, 0.0, d0410, eod);
                a.GetEventLogText(vl);
                char msg[200];
                std::snprintf(msg, sizeof(msg), "11. VTEST, CUSTOMER_CODE \"%s\", %s: %u file(s), %d row(s) / %d s", vq[i].cust,
                              modeName[vq[i].mode], (unsigned)vq[i].files, vq[i].stop, vq[i].stopSec);
                CHECK(vl.size() == 2 && a.filesRead.size() == vq[i].files && a.MySummary.iStopCount == vq[i].stop &&
                          a.MySummary.iStopTime == vq[i].stopSec && a.filesRead[0].find(vq[i].first) != std::string::npos &&
                          a.UsesGoldenFileRule() == vq[i].usesGolden && a.dupRowsSkipped == 0, msg);
            }
            // SetCustCode (Hub::ReadConfig's path) switches the rule on the same Analyzer
            ela::Options o;
            o.jamIniPath = noJam;
            ela::Analyzer a(o);
            const bool before = a.UsesGoldenFileRule();
            a.SetCustCode("919");
            a.SetRange(d0410, 0.0, d0410, eod);
            a.GetEventLogText(vl);
            CHECK(!before && a.UsesGoldenFileRule() && a.filesRead.size() == 1 && a.MySummary.iStopTime == 20,
                  "11. ★W38 B: SetCustCode(\"919\") after construction = golden's VTEST names (1 file, 20 s)");
        }

        // the W23 samples (fixture copies in %TEMP%\ht9045_ela_core_fx, set up in section 9) plus the copies golden's
        // folder also holds: the AllEventLog day copy (CR stripped here, as MyStringList.cpp:599 writes it), a ByLot
        // file, a JamStat file and an SGJamCount RawData file with the same rows -- none of those three may be read
        bool okB = false, okC = false;
        const std::string dayB = ReadBytes(g_fx + "\\2025\\08\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", &okB);
        const std::string day2000 = ReadBytes(g_fx + "\\2024\\07\\01\\HT-9045W_HHT-55_EventLogTxt_202407012000.csv", &okC);
        std::string dayBLF = dayB;
        for (size_t k = dayBLF.find('\r'); k != std::string::npos; k = dayBLF.find('\r')) dayBLF.erase(k, 1);
        CHECK(okB && okC && dayBLF.size() + 284 == dayB.size(), "11. sample (b) has 284 CRLF lines; the AllEventLog copy is LF");
        WriteBytes(g_fx + "\\AllEventLog\\HT9046LS_JFTH013_EventLogTxt_20250825.csv", dayBLF);
        WriteBytes(g_fx + "\\ByLotID\\2025\\08\\25\\HT9046LS_JFTH013_600000_20250825130115_ByLotEventLog.csv", dayB);
        WriteBytes(g_fx + "\\SGJamCount\\2025\\08\\HT9046LS_JFTH013_20250825_RawData.csv", dayB);
        WriteBytes(g_fx + "\\2024\\07\\01\\HT-9045W_HHT-55_JamStat_202407012000.csv", day2000);
        const std::vector<std::string> fav = ela::LoadFavorite(g_fx);
        CHECK(fav.size() == 11, "11. the sample tree: 7 fixtures + AllEventLog + ByLot + RawData + JamStat");

        struct Q
        {
            const char* what;
            int y0, m0, d0, y1, m1, d1, h1, n1, s1;
            int stop, stopSec, jam, war, mes, dup, files;     // W19 B (the defaults)
            int gStop, gFiles;                                // golden mode (all three switches)
        };
        const Q qs[] = {
            { "(b) 2025-08-25 + AllEventLog", 2025, 8, 25, 2025, 8, 25, 23, 59, 59, 40, 0, 0, 0, 40, 40, 2, 80, 2 },
            { "(c) 12 h 2024-07-01..02", 2024, 7, 1, 2024, 7, 2, 23, 59, 59, 278, 510, 37, 2, 221, 0, 2, 0, 0 },
            { "(c) 12 h 2024-07-01 only", 2024, 7, 1, 2024, 7, 1, 23, 59, 59, 162, 69, 31, 0, 127, 0, 2, 0, 0 },
            { "(c) 12 h 2024-07-02 00:00..07:59:59", 2024, 7, 2, 2024, 7, 2, 7, 59, 59, 116, 441, 6, 2, 94, 0, 1, 0, 0 },
            { "hour files 2023-09-01", 2023, 9, 1, 2023, 9, 1, 23, 59, 59, 66, 0, 0, 0, 66, 0, 2, 0, 0 },
            { "month file 2025-11-06..07", 2025, 11, 6, 2025, 11, 7, 23, 59, 59, 11, 2, 0, 1, 10, 0, 1, 0, 0 },
        };
        for (size_t i = 0; i < sizeof(qs) / sizeof(qs[0]); ++i)
        {
            const Q& q = qs[i];
            const double sd0 = ela::EncodeDate(q.y0, q.m0, q.d0), ed1 = ela::EncodeDate(q.y1, q.m1, q.d1);
            const double et1 = ela::EncodeTime(q.h1, q.n1, q.s1, 0);
            ela::Options o;
            o.jamIniPath = noJam;
            ela::Analyzer a(o);
            a.SetRange(sd0, 0.0, ed1, et1);
            a.GetEventLogText(fav);
            const ela::Summary& s = a.MySummary;
            char msg[200];
            std::snprintf(msg, sizeof(msg), "11. W19 %s: %d stop rows / %d s, JAM %d WAR %d MES %d, %d duplicate(s), %d file(s)",
                          q.what, q.stop, q.stopSec, q.jam, q.war, q.mes, q.dup, q.files);
            CHECK(s.iStopCount == q.stop && s.iStopTime == q.stopSec && s.iJAMCount == q.jam && s.iWARCount == q.war &&
                      s.iMESCount == q.mes && a.dupRowsSkipped == q.dup && (int)a.filesRead.size() == q.files &&
                      !Contains(a.filesRead, "ByLotEventLog") && !Contains(a.filesRead, "RawData") &&
                      !Contains(a.filesRead, "JamStat"), msg);
            if (i == 0)
                CHECK(a.filesRead.size() == 2 && a.filesRead[0].find("AllEventLog") == std::string::npos &&
                          a.filesRead[1].find("AllEventLog") != std::string::npos,
                      "11. (b): the 2025\\08 file first, then its AllEventLog copy (whose 40 rows are all skipped)");
            if (i == 1)
                CHECK(a.mapByDayList["2024/07/01"].iStopTime == 69 && a.mapByDayList["2024/07/02"].iStopTime == 441 &&
                          a.filesRead.size() == 2 && a.filesRead[0].find("202407010800") != std::string::npos,
                      "11. (c): the 20:00 file of 07/01 holds 07/02's rows (441 s after midnight); 0800 read first");
            ela::Options g = o;
            g.bcbCommaText = true;
            g.keepGoldenBugs = true;
            g.goldenFileRule = true;
            ela::Analyzer ga(g);
            ga.SetRange(sd0, 0.0, ed1, et1);
            ga.GetEventLogText(fav);
            std::snprintf(msg, sizeof(msg), "11. golden %s: %d stop rows, %d file(s)", q.what, q.gStop, q.gFiles);
            CHECK(ga.MySummary.iStopCount == q.gStop && (int)ga.filesRead.size() == q.gFiles && ga.dupRowsSkipped == 0, msg);
        }
        CHECK(::GetFileAttributesA(noJam.c_str()) == INVALID_FILE_ATTRIBUTES, "11. no JAM0000.dat was created");
    }

    printf("ELA_Core: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
