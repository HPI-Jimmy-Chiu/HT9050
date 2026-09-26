// =============================================================================
//  test_ela_core.cpp -- ELA plan P1a: ElaCore (Event Log Analyzer core, golden EventlogAnalyzer Rev891.0).
//
//  AI(W906-ELA-P1) 20260927.  Suite name (add_test): ELA_Core
//
//  Covers EventLogAnalysis/ElaCore.* with the golden-faithful defaults (暫照 golden（A），待使用者確認, #15-#19):
//    1. BCB6 CommaText read / write (blank splitting, quotes, "" escape, trailing comma, missing closing quote);
//    2. Common.cpp date helpers (validators, ParseDateTime incl. 2026/02/30 -> TDateTime(), formatting);
//    3. SetRange keys (by day from midnight, by hour from the start time) and iDate;
//    4. GetEventLogText over two day files: stop / alarm (MTBA) / fail (MTBF) / JAM / WAR / MES counts and times,
//       per-day and per-hour, Jam / War / Alarm summaries, By-Area and By-Function lists, duplicate / out-of-range /
//       unquoted "16 System" rows, the file-name rule;
//    5. JAM0000.dat write-back (#17 A) and jamWriteBack=false;
//    6. the golden leaks (#18 A: iFailCount and the By-Area lists grow on a second query) and keepGoldenLeaks=false;
//    7. #15 B (comma-only) counts the "16 System" row.
//    8. P1b: ConvertSecToTime / ConvertDTToTime / StrToDateTime (zh-TW) / LoadFavorite; the Top5 / Top5Filter /
//       FailAndAlarm / ByFilter grids incl. golden's quirks (stale Top5Filter rows after a filter switch, "No Record!!"
//       over a single By-Area record, prefix match in Top5); Production_Log -> sgByDay / sgByHour (incl. golden's by-hour
//       test-time bug) and the mmoSummary lines.
//  Every file is under %TEMP%\ht9045_ela_core -- never D:\HT9045_Log or D:\HT9045\Error.
// =============================================================================
#include "EventLogAnalysis/ElaCore.h"
#include <windows.h>
#include <cstdio>
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

static const char* kHeader = "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n";

int main()
{
    printf("ELA_Core\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    g_root = std::string(tmp) + "ht9045_ela_core";
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
        CHECK(ela::CommaOnlyText("a,16 System,\"x,y\"") == Row({ "a", "16 System", "x,y" }), "1. #15 B comma-only split");
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

    // 3 + 4 + 5: golden defaults
    {
        ela::Options o;
        o.jamIniPath = jamIni;
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

    // 6b. keepGoldenLeaks=false; 5b. jamWriteBack=false; 7. comma-only
    {
        WriteFile(jamIni, "[01 Input Arm]\r\n");
        ela::Options o;
        o.jamIniPath = jamIni;
        o.keepGoldenLeaks = false;
        o.jamWriteBack = false;
        ela::Analyzer a(o);
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        a.SetRange(sd, st, ed, et);
        a.GetEventLogText(files);
        CHECK(a.MySummary.iFailCount == 2 && a.ListByUnitName[0].size() == 1, "6b. keepGoldenLeaks=false resets both");
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
    }

    printf("ELA_Core: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
