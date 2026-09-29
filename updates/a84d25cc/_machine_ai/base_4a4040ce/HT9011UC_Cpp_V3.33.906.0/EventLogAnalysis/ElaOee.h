// ===========================================================================
//  EventLogAnalysis/ElaOee.h -- ★W48 OEE chart: test / down / idle / off per day, for eventlog.html's OEE tab.
//  AI(W906-ELA-W48) 20260928 (St02-E helper): first version (Steven 0928 = A).
//  AI(W906-ELA-W48B) 20260928 (St02-E helper): Steven 0928 09:3x ruled all three rules -- W48-1 (test = per touchdown
//    from 0x41 to BIN ON + ECHO OK), W48-2 = B (idle vs off by the software-on time), W48-3 = B (down = the "include in
//    MTBA" alarms only) -- replacing the St02 defaults Q1 = B / Q2 = A / Q3 = A of the first version.
//  AI(W906-ELA-A8) 20260928 (St02-E helper): review finding A8 (St02's call) -- the Production_Log columns below are the
//    golden DEFAULT layout; a CC_Greatek (956) file has its own order, so ComputeOee now picks them by the file's
//    header names, else by CUSTOMER_CODE (ProdLogColumns, below).
//  Ledger docs/ELA_PORT_LEDGER.md "★W48 OEE"; skill ht9045-eventlog-analyzer.
//
//  A NEW feature, not a port.  Golden (analyzer Rev891, D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code) has no real OEE:
//  FormShow draws its three OEE canvases once with random() data and then hides the three tabs (Analyzer.cpp:385-416,
//  srand :359); chart 1 "YYYY-MM 每日稼動率" = SetOeeAnaDay (:1310-1385, legend Running / Alarm / Pause :1305-1307,
//  colours :1286-1288).  Ruling A = that chart 1 with real per-day numbers; charts 2 / 3 are out of scope.
//
//  Input = what ONE query already left in the Analyzer (no second pass over those files; ElaCore stays golden-only),
//  plus the TimeData rows the caller read (ReadTimeData, below):
//    slStopList  the JAM / WAR / MES rows GetEventLogText kept (range, Duplicate, W19 de-dup), i.e. the rows golden's
//                Total Stop Time sums (Analyzer.cpp:852-881; ElaCore.cpp GetEventLogText);
//    slAlarmList the part of slStopList that JamConfig::GetJemIncludeMTBA counts -- golden's alarm rows (Analyzer.cpp
//                :883-899, MySummary.iAlarmTime / By Day "Alarm Time"); the "<code> IncludeMTBA" keys of JAM0000.dat,
//                set by the customer on the Security page's Jam tab (★W45);
//    slAllData   the Production_Log rows ListProductionLog kept (In Time in (dtStart, dtEnd)); element 0 = the header
//                slot (golden slAllData->Add(slFile->Strings[0]));
//    byDayKeys   the By Day row keys (SetRange);  dtStart / dtEnd  the query range;
//    td          the Handler's hourly TimeData rows (906_0625_Steven cMyDB.cpp:339-498 RecordTimeData, hourly from
//                HS_Function.cpp:233-238; V906 cMyDB.cpp:479, hourly from TesterComm/Handler/TesterCommWiring.cpp).
//
//  Per day d (times as millisecond counts, ElaCore DateTimeToMs):
//    span_d   = the day  intersect  [dtStart, the end of the second dtEnd names)  intersect  [.., now)
//               (the To second is included: golden's row check is dtCurrent <= dtEnd, so "23:59:59" = the whole day)
//    down_d   = |DownSet intersect span_d|, DownSet = the union of [t, t + StopedTime) over slAlarmList
//               W48-3 = B: only the alarms flagged "include in MTBA" -- the rows golden's Alarm Time adds (Analyzer.cpp
//               :883-899, GetJemIncludeMTBA per row).  A code the file has no key for takes JamConfig's default, which is
//               golden's (Analyzer.cpp:2812-2835: "JAM" in the code and area 01..05 = counted, anything else = not;
//               ★W45 18b changes only the IncludeMTBF default), and without a JAM0000.dat nothing counts (:2818
//               FileExists).  On CC_ASE_CL 933 / CC_TERAPOWER 967 this key is the merged page's second box ("Include
//               MTBA (Analyzer)", ★W45 18c); their Handler box writes IncludeMTBF.  Every other kept row (a WAR, MES1640
//               One cycle finish, an unchecked JAM) is idle time.  A union (overlapping dialogs are not counted twice)
//               cut at midnight -- unlike By Day's Alarm Time, which adds the whole StopedTime to the row's own day.
//    test_d   = |(TestSet minus DownSet) intersect span_d|
//               W48-1: a touchdown's test time runs from the GPIB 0x41 (start test) until BIN ON and ECHO OK are in
//               (RS232 / TTL: their equivalents); a day = the sum.  Golden's Production_Log Test Time (col 27,
//               "hh:nn:ss.zzz") is measured that way: RunInfo.TestTime (906_0625_Steven cObserver.cpp:2024 ->
//               AddTestTime :2033) = the latest TimeInfoGrid row (RecordTimeInfo :1894-1931) = RecordEndTestTime
//               (:2150-2161) - RecordStartTestTime (:2136-2147).  The start is stamped right before fMain->RunTestProgram
//               (atester.cpp:1481-1482 / :1530), which makes the bridge raise SRQ 0x41 (V906
//               TesterComm/Gpib/GpibTestGpib.cpp:400-402); the end after the tester task saw bEcho (atester.cpp:1645),
//               which the Handler sets on the bridge's result (main.cpp:17122), which the GPIB bridge sends only after
//               ECHOOK (GpibTestGpib.cpp:1422-1449 return 1 -> GpibCore.cpp:1079-1116), or right after BINON with BINON
//               echo off.  So col 27 = Steven's time plus the Handler <-> bridge message latency at both ends.  One
//               touchdown per change of Order of testing (col 13, golden's contact-count rule); it starts at t(k) = SOT
//               (col 14 "yyyymmdd_hhmmss" = sBufferSOT, stamped by the same RecordStartTestTime :2146), else EOT (col 29,
//               :2161) - Test Time, else Arm Time (col 12), else In Time (col 5).  TestSet = the union of
//               [t(k), t(k) + Test Time(k)) -- the sum while touchdowns do not overlap (one test head).  A touchdown
//               whose Test Time is unreadable or 0 adds nothing (OeeResult::noTestTime).  Down wins where both hold (an
//               alarm dialog open while a test runs counts as down).
//               Golden caveats (not changed): a double contact (SRQ 0x42 / 0x43 / 0xC1 retest) re-stamps the start at
//               every contact and the end once, so col 27 = the last contact only; the arithmetic keeps minutes /
//               seconds / ms only (a test of 60 min or more is wrong); with iShuttleMode 1 RecordTimeInfo runs for the
//               selected arm only (cObserver.cpp:2163-2169); no real IC = no stamps (atester.cpp:1481).
//    off_d    = |(Covered minus (OnSet union TestSet union StopSet)) intersect span_d|
//               W48-2 = B: software running with no test and no down = idle; software not running = off.  OnSet comes
//               from TimeData: row k (time at(k), PowerOnTime P(k) = the software-on seconds since the previous record,
//               golden UpdateRecordScreen main.cpp:8166-8177 = V906 FileRW/MainRecord.cpp:141-152) is on during
//               [at(k-1), at(k)) when at(k) - at(k-1) - P(k) <= kTimeDataSlackSec, else during
//               [max(at(k) - P(k), at(k-1)), at(k)) -- the amount is exact, the place inside a gap is not recorded, so it
//               is put at the end (a record after a restart also carries the time before the shutdown).  Past the last
//               row, [at(last), now) is on while now - at(last) <= 3600 s + the slack (this analyzer runs inside the
//               Handler, and the hourly record is not due yet).  Covered = [the first row's on start, at(last)) plus that
//               tail.  A test or an alarm row (any kept stop row) proves the software ran, so it is never off.  Time
//               that no TimeData covers (no file, before the first row, an overdue tail) is not called off: it stays in
//               idle and is reported as uncoveredSec -- so without TimeData the result is the first version's.
//    idle_d   = span_d - test_d - down_d - off_d
//    test% / down% / off% = rounded to 0.01 (half up); idle% = 100 - the three (so the four add up to 100.00; a
//             rounding overshoot is taken from off, then down).
//    noData   = no touchdown, no stop row, no test and no down time, and no TimeData cover in span_d; then every time
//               and % is 0 (nothing is claimed; the page draws light grey).  A day after `now` has span 0 and is noData.
//    Also per day (informational, from TimeData): onSec = |OnSet intersect span_d|, startSec = the SystemStart seconds
//    (StartTime) of the rows, each spread evenly over its on span -- test is part of it; SystemStart without a test
//    (handling, soak, homing) is idle in the four segments.
//  Not seen (待上機確認): an alarm raised before dtStart; the soak before the first touchdown counts as idle.
//
//  A8, the column layout (906_0625_Steven Public/MyProductionRecord.cpp; 912_0908_Jimmy the same): the col numbers above
//  are the default layout, SaveRecord :710-772 (asBuffer->CommaText in eMyProdRec order, MyProductionRecord.h:83-157;
//  header asDataTitle :92-162 -- In Time :97, Arm Time :104, Order of testing :105, SOT time stamp :106, Test Time :119,
//  EOT time stamp :121).  CUSTOMER_CODE == CC_Greatek (956, MachineType.h:325) returns at :703-708 after
//  SaveDataForGreatek (:851-887): iSortData :858 reorders the fields and the header is asDataTitleGreatek (:46-88), so
//  In Time = col 6 (:52), Arm Time 18 (:64), Order of testing 19 (:65), SOT 20 (:66), Test Time 37 (:83), EOT 39 (:85);
//  col 27 there is Out Arm Shuttle Pick (:73).  No other CUSTOMER_CODE branch moves a column of this file (KYEC_LEE /
//  Greatek / HANA_MICRON only add 1 to the X / Y values :255 / :291 / :394; LEADYO also writes the default layout to
//  Production_ByFile :1038; SJ / O24 change the file name only :1120-1142).  ProdLogColumns: each column by its golden
//  header name in slAllData[0] (both headers name all six), else the customer's golden index.
//  ⚠ Upstream (not changed here, needs a decision): golden ListProductionLog (Rev891 Analyzer.cpp:2377-2385;
//  ElaTables.cpp:531-543) keeps a row by col 5 = In Time for every customer; on a Greatek file col 5 is In Y (a small
//  integer, read as a time of day 1899-12-30), so no Greatek row reaches slAllData and this OEE finds no touchdown there.
//
//  V906 does not write alarm rows with StopedTime (SaveErrEventLog gated, forms/fNote.h:95) yet; its SaveRecord port
//  (Public/MyProductionRecord.cpp:1185, Greatek arm :1232, called from aoutarm9045.cpp:5058) writes both layouts.  It
//  writes TimeData hourly (from this change).
//  ComputeOee / OeeJson: standard C++, no file access, no clock of its own (the caller passes now: Hub = its report
//  clock, Hub::SetClock).  ReadTimeData reads files (read only).
// ===========================================================================
#ifndef HT9045_ELA_ELAOEE_H
#define HT9045_ELA_ELAOEE_H

#include "EventLogAnalysis/ElaCore.h"

#include <string>
#include <vector>

namespace ela {

// W48-2 = B: an hourly TimeData row whose PowerOnTime falls short of the time since the previous row by at most this
// many seconds covers the whole hour (the Handler writes whole seconds, "%i" of ms / 1000, and the trigger jitters)
const int kTimeDataSlackSec = 60;

struct OeeOptions
{
    int timeDataSlackSec;        // kTimeDataSlackSec
    OeeOptions() : timeDataSlackSec(kTimeDataSlackSec) {}
};

// one TimeData row (golden slTimeData->AddTextWithDateTime(CommaText of RecordTimeData's str), 906_0625_Steven
// cMyDB.cpp:380-392 / :491; header main.cpp:1539-1543 "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime,
// ProductionTime, JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA")
struct TimeDataRow
{
    double at;                   // Date + Time: when RecordTimeData wrote it
    long long onMs;              // PowerOnTime (s) * 1000: software-on time since the previous record
    long long startMs;           // StartTime (s) * 1000: SystemStart time since the previous record
    TimeDataRow() : at(0.0), onMs(0), startMs(0) {}
};

struct OeeDay
{
    std::string date;            // the By Day key "YYYY/MM/DD"
    long long spanMs, testMs, downMs, idleMs, offMs;
    long long onMs;              // |OnSet intersect span| (TimeData)
    long long startMs;           // the TimeData SystemStart time spread onto this day
    long long uncoveredMs;       // the part of the span no TimeData covers (counted in idle)
    int testBp, downBp, idleBp, offBp;   // hundredths of a percent (7500 = 75.00 %)
    int touchdowns;              // touchdowns whose t(k) lies in the span
    int stopRows;                // slStopList rows whose own time lies in the span (a zero-second MES row included)
    int alarmRows;               // the slAlarmList (MTBA) rows among them -- the ones that give down
    bool noData;
    OeeDay();
};

// A8: the six Production_Log columns ComputeOee reads (the file head).  The default = golden's default layout.
struct ProdLogCols
{
    int inTime, armTime, order, sot, testTime, eot;
    ProdLogCols();               // 5, 12, 13, 14, 27, 29 (eMyProdRec, MyProductionRecord.h:90 / :97-99 / :112 / :114)
};
extern const char* const kCustCodeGreatek;   // "956" (CC_Greatek, 906_0625_Steven MachineType.h:325)
ProdLogCols GreatekProdLogCols();             // 6, 18, 19, 20, 37, 39 (SaveDataForGreatek iSortData, :858)
// each column = the first field of `header` (slAllData[0], blanks turned to '_' by ListProductionLog) whose name is
// golden's ("In Time", "Arm Time", "Order of testing", "SOT time stamp", "Test Time", "EOT time stamp"; blanks and
// '_' trimmed), else GreatekProdLogCols() when custCode is exactly "956" (golden's sCustCode compare, Analyzer.cpp:807),
// else ProdLogCols()
ProdLogCols ProdLogColumns(const std::string& header, const std::string& custCode);

struct OeeResult
{
    double now;                  // the TDateTime the spans were cut at
    ProdLogCols cols;            // A8: the columns used (ProdLogColumns of slAllData[0] and the Analyzer's custCode)
    int touchdowns;              // every touchdown found in slAllData
    int noTestTime;              // touchdowns whose Test Time (cols.testTime: 27, Greatek 37) is unreadable or 0
    int stopRows;                // every usable row of slStopList
    int alarmRows;               // every usable row of slAlarmList (W48-3 = B: these give down)
    int timeDataRows;            // TimeData rows used
    int timeDataSlackSec;
    std::vector<OeeDay> days;    // byDayKeys order
    OeeResult();
};

// the numbers above for the query the Analyzer holds (SetRange + GetEventLogText + ListProductionLog, i.e. BtnQuery)
// and the TimeData rows td (any order; ReadTimeData's; empty = no TimeData: no off segment)
OeeResult ComputeOee(const Analyzer& a, const std::vector<TimeDataRow>& td, double now, const OeeOptions& opt);

// the snapshot's "oee" value: {"rule":{...},"days":[{"date",spanSec,testSec,downSec,idleSec,offSec,testPct,downPct,
// idlePct,offPct,onSec,startSec,uncoveredSec,touchdowns,stopRows,alarmRows,noData}, ...]} (seconds with up to 3
// decimals, % with 2).  timeDataDir is shown in the rule (JSON-escaped); rule.w481 = "col" + r.cols.testTime (A8).
std::string OeeJson(const OeeResult& r, const std::string& timeDataDir);

// Production_Log SOT / EOT time stamp "yyyymmdd_hhmmss" (golden Analyzer.h eSOTTime kevin 20140918, eEOTTime wei
// 20181211) -> TDateTime; false = not that form or not a valid date / time.  (For the ctest.)
bool ParseSotStamp(const std::string& s, double* out);

// one TimeData line -> row; false = the header, an empty or a malformed line.  "yyyy-mm-dd, hh:nn:ss.zzz, <11 numbers>"
// (golden TMyStringList::AddTextWithDateTime; the SIGURD form has no blank after the commas)
bool ParseTimeDataLine(const std::string& line, TimeDataRow* out);

// W906_HT9045LOG_ROOT (the Handler's as9045LogPath seam, common.cpp:240) or golden "D:\HT9045_Log", + "\TimeData" --
// golden main.cpp:1539 slTimeData's folder (V906 LogObjects.cpp:136)
std::string DefaultTimeDataDir();

// the TimeData rows of the years year(dtFrom) - 1 .. year(dtTo) + 1: <dir>\<yyyy>\TimeData_<yyyy>.csv (golden
// TMyStringList, SaveType TByYear with bFilePathWithDate, Steven 20210623) and <dir>\TimeData_<yyyy>.csv (the flat name
// before that); sorted by time, exact duplicates dropped.  Read only; files = the files that existed.
std::vector<TimeDataRow> ReadTimeData(const std::string& dir, double dtFrom, double dtTo, std::vector<std::string>* files);

}  // namespace ela

#endif
