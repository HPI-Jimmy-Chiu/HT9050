// =============================================================================
//  test_observer_core.cpp -- FW-3 cObserver Wave 1 test coverage
//
//  AI(W906-FW3-Observer-W1) 20260818: new file.
//
//  Covers (per this wave's task brief):
//   (a) CalculateStopTime/UnCalculateStopTime -- round-trip symmetry AND
//       known literal values, for both the <=60-minute and >60-minute
//       branches.
//   (b) GetEventLogText's 3 filter kinds (JAM/WAR/MES prefix filter,
//       UnitName substring filter, "show all") + the file-not-found ->
//       "No Record!!" branch. Drives it through cbbMonthChange (the real
//       call path, not a hand-rolled shortcut) so the W906_EVENTLOG_ROOT
//       redirect (cObserver.cpp, this wave) is exercised end to end, against
//       a scratch CSV this test writes itself (quoted fields, an embedded
//       comma AND embedded double-space, verifying vclcompat::TStringList's
//       CommaText parser does not shift columns on either).
//   (c) One BtnQueryClick eQueryType branch's SQL text (Alarm_History),
//       via the PORT-ONLY BuildWhereQuery_OccurDateTimeRange/
//       BuildQuery_AlarmHistory extraction (forms/fObserver.h) -- NOT by
//       calling BtnQueryClick itself (its real MyDBVProcess/MyDBVEventFreq
//       calls sqlite3_get_table(dbReadOnly,...) unconditionally with no
//       NULL-handle guard -- cMyDB.cpp:1584 -- so invoking it here would
//       need a real, opened SQLite handle just to avoid a crash; not this
//       wave's job, and NOT cMyDB mock/behaviour-fake either way).
//
//  SAFETY -- constructing a real TfObserver touches TWO global config paths
//  --------------------------------------------------------------------------
//  TfObserver's ctor (golden :137-346) calls GetObserAuth() (reads/seeds
//  D:\HT9045\config\Security_new.def via the global `AuthPath`) and
//  CheckAndReadIniDataGeneral() (reads/seeds D:\HT9045\system\Gerneral.ini
//  via the global `INIFileGeneral`, opened from `asGeneralPath`) -- BOTH
//  hardcoded production paths (common.cpp:89/:102). `INIFileGeneral` starts
//  NULL and CheckAndReadIniDataGeneral operates on it with NO NULL check
//  (common.cpp:1478), so constructing a TfObserver WITHOUT first opening it
//  would crash, not silently write. This test redirects BOTH globals to a
//  scratch directory before constructing anything, restores them in main()'s
//  teardown -- the SAME idiom tests/test_ga1_cprod.cpp already established
//  for `AuthPath` (its own ScratchDir()/MyForceDirectories precedent,
//  :322-325), extended here to `asGeneralPath`/OpenGeneralIniFile() (both
//  `extern`-declared in common.h, common.h:60/:71/:213) for the second path.
// =============================================================================
#include "forms/fObserver.h"
#include "common.h"        // AuthPath, asGeneralPath, OpenGeneralIniFile/CloseGeneralIniFile, MyForceDirectories
#include "cmydef.h"        // CUSTOMER_CODE
#include "Config.h"        // IniConfig (bSPILFunction)
#include "Public/MyStringList.h"   // AI(W906-FW-Q5) 20260818: TMyStringList full type -- slEventLog fixture in Test_StatisticalJamCount_Family (cmydef.h only fwd-declares)

#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__MINGW32__)
// AI(W906-FW0) 20260817 precedent (tests/test_bootstrap.cpp:65-67): -std=c++17
// defines __STRICT_ANSI__, which hides MinGW's _putenv declaration.
extern "C" int _putenv(const char *);
#endif

// ---------------------------------------------------------------------------
//  Minimal CHECK harness (matches tests/test_ga1_cprod.cpp precedent)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
        if (cond) { g_pass++; } \
        else { g_fail++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
    } while (0)

static AnsiString ScratchDir()
{
    return AnsiString("test_observer_core_scratch\\");
}

// =============================================================================
//  (a) CalculateStopTime / UnCalculateStopTime
// =============================================================================
static void Test_CalculateStopTime_KnownValuesAndRoundTrip()
{
    // <=60-minute branch (golden's `else` arm: "0:MM:S", MM zero-padded, S NOT).
    CHECK(TfObserver::CalculateStopTime(0) == "0:00:0", "CalculateStopTime(0) == \"0:00:0\"");
    CHECK(TfObserver::CalculateStopTime(125) == "0:02:5", "CalculateStopTime(125) == \"0:02:5\" (2 min 5 sec)");
    CHECK(TfObserver::CalculateStopTime(59) == "0:00:59", "CalculateStopTime(59) == \"0:00:59\" (0 min 59 sec)");

    // >60-minute branch (golden's `if(i_mm>60)` arm: "H:MM:S", note i_mm==60
    // exactly (60 minutes on the nose) does NOT take this branch -- golden's
    // own `>` not `>=`, translated verbatim, not "fixed").
    CHECK(TfObserver::CalculateStopTime(3725) == "1:02:5", "CalculateStopTime(3725) == \"1:02:5\" (1hr 2min 5sec)");
    // AI(W906-FW3-Observer-W1) 20260818: CalculateStopTime(3600): i_mm=60,
    // golden's condition is `i_mm>60` (strictly greater), so exactly 60
    // minutes does NOT take the hh:mm:ss arm -- it falls into the `else`
    // arm, which formats i_mm (60) as mm directly ("60", already 2 chars, no
    // padding needed) with a literal "0:" hour prefix: "0:60:0". This is
    // golden's own apparent-but-harmless quirk (60 minutes displays as
    // "0:60:0" instead of "1:00:0") -- translated verbatim, documented in
    // cObserver.cpp is not needed since it falls straight out of the
    // >-vs->= comparison already called out there for the CalculateStopTime
    // translation. Asserted here as a KNOWN VALUE, not "fixed".
    CHECK(TfObserver::CalculateStopTime(3600) == "0:60:0", "CalculateStopTime(3600) == \"0:60:0\" (i_mm==60 is NOT >60 -- golden's own quirk, verbatim)");

    // Round-trip: UnCalculateStopTime(CalculateStopTime(Sec)) == Sec, both branches.
    int samples[] = { 0, 5, 59, 60, 61, 125, 3599, 3600, 3601, 3725, 7325 };
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i)
    {
        int sec = samples[i];
        AnsiString formatted = TfObserver::CalculateStopTime(sec);
        int roundTripped = TfObserver::UnCalculateStopTime(formatted);
        CHECK(roundTripped == sec, "round-trip UnCalculateStopTime(CalculateStopTime(Sec)) == Sec");
    }

    // golden :2241 `if(Time=="") return 0;`
    CHECK(TfObserver::UnCalculateStopTime("") == 0, "UnCalculateStopTime(\"\") == 0 (empty-string guard)");
}

// =============================================================================
//  (c) BtnQueryClick's Alarm_History branch SQL text, via the PORT-ONLY
//      extraction (see forms/fObserver.h banner on these two methods).
// =============================================================================
static void Test_BuildQuery_AlarmHistory()
{
    TDateTime d1 = EncodeDate(2026, 1, 1);
    TDateTime t1 = EncodeTime(0, 0, 0, 0);
    TDateTime d2 = EncodeDate(2026, 12, 31);
    TDateTime t2 = EncodeTime(23, 59, 59, 0);

    AnsiString whereQuery = TfObserver::BuildWhereQuery_OccurDateTimeRange(d1, t1, d2, t2);
    CHECK(whereQuery == " WHERE (OccurDateTime >= '2026-01-01 00:00:00' AND OccurDateTime <= '2026-12-31 23:59:59')",
          "BuildWhereQuery_OccurDateTimeRange builds golden's exact WHERE clause text");

    AnsiString asQuery = TfObserver::BuildQuery_AlarmHistory(whereQuery);
    AnsiString expected = AnsiString("SELECT AlarmCode, UnitName, Message, ErrPart, Recovery, Duplicate, "
                                      "DATE(OccurDateTime) Date, TIME(OccurDateTime) Time, StopedTime, Temperature "
                                      "FROM AlarmHistoryView") + " " + whereQuery + " ORDER BY OccurDateTime DESC";
    CHECK(asQuery == expected, "BuildQuery_AlarmHistory (golden's Alarm_History eQueryType branch) builds the exact SELECT text");
}

// =============================================================================
//  (b) GetEventLogText -- 3 filter kinds + file-not-found, via cbbMonthChange
//      (real call path) against a scratch CSV + W906_EVENTLOG_ROOT redirect.
// =============================================================================
static void WriteScratchCsv(const AnsiString &path)
{
    std::FILE *fp = std::fopen(path.c_str(), "wb");
    CHECK(fp != NULL, "(setup) scratch EventLogTxt CSV created");
    if (!fp) return;
    // 9-column non-SPIL layout (golden ctor's strngrdEventLog->ColWidths[0..8]
    // comments: Date,Time,UnitName,AlarmCode,Recovery,StopedTime,Duplicate,
    // Message,ErrPart). Row 0 is a header row (every filter branch starts its
    // scan at i=1, skipping it). Two Message fields are deliberately
    // CSV-quoted: one with an embedded COMMA (the classic column-shift
    // hazard), one with an embedded DOUBLE SPACE (quoted even though not
    // strictly required, to confirm vclcompat::TStringList's CommaText
    // parser accepts unnecessary quoting too).
    const char *rows =
        "Date,Time,UnitName,AlarmCode,Recovery,StopedTime,Duplicate,Message,ErrPart\r\n"
        "2026-08-18,09:00:00,InArm,JAM0301,1,12,0,\"Loader, jam near site A\",InArm1\r\n"
        "2026-08-18,09:05:00,OutArm,WAR16102,1,3,0,\"Clean pad worn out\",OutArm2\r\n"
        "2026-08-18,09:10:00,Shuttle,MES0101,1,5,0,\"Barcode read fail  extra space\",Shuttle1\r\n"
        "2026-08-18,09:15:00,Index,ALM0001,1,8,0,\"Generic alarm ignored by JAM WAR MES\",Index1\r\n";
    std::fputs(rows, fp);
    std::fclose(fp);
}

static void Test_GetEventLogText_FiltersAndNotFound()
{
    AnsiString root = ScratchDir() + "eventlog_root";
    AnsiString yearDir = root + "\\2026";
    AnsiString monthDir = yearDir + "\\08";
    MyForceDirectories(monthDir);
    AnsiString csvPath = monthDir + "\\sample.csv";
    WriteScratchCsv(csvPath);

    int savedCustomerCode = CUSTOMER_CODE;
    bool savedSpil = IniConfig.bSPILFunction;
    CUSTOMER_CODE = 0;                    // not CC_SINOICTECH -- plain full-path branch in GetEventLogText
    IniConfig.bSPILFunction = false;      // non-SPIL 9-column layout (matches the CSV header above)

    AnsiString savedEventLogRootEnv = getenv("W906_EVENTLOG_ROOT") ? AnsiString(getenv("W906_EVENTLOG_ROOT")) : AnsiString("");
    bool hadEventLogRootEnv = getenv("W906_EVENTLOG_ROOT") != NULL;
    AnsiString envAssign = AnsiString("W906_EVENTLOG_ROOT=") + root;
    _putenv(envAssign.c_str());

    {
        TfObserver observer;
        observer.cbbEventLogYear->Text = "2026";
        observer.cbbMonth->Text = "08";

        // --- baseline: cbbMonthChange search-populates lstEventLog, sets
        //     cbbFilter->ItemIndex=0 ("show all"), and calls GetEventLogText
        //     itself (golden :3796) -- exercises the W906_EVENTLOG_ROOT
        //     redirect end to end, not a hand-set lstEventLog shortcut.
        observer.cbbMonthChange(NULL);
        CHECK(observer.lstEventLog->Items->Count == 1, "(setup) SearchFileAll found exactly the 1 scratch CSV under W906_EVENTLOG_ROOT");
        CHECK(observer.strngrdEventLog->RowCount == 5, "GetEventLogText (show all): RowCount == 5 (1 header + 4 data rows)");
        CHECK(observer.strngrdEventLog->Cells[3][1] == "JAM0301", "show-all: row 1 AlarmCode column not shifted by the quoted-comma Message field");
        CHECK(observer.strngrdEventLog->Cells[7][1] == "Loader, jam near site A", "show-all: row 1 Message field (embedded comma) parsed verbatim, not truncated at the comma");
        CHECK(observer.strngrdEventLog->Cells[8][1] == "InArm1", "show-all: row 1 ErrPart column (the field AFTER the quoted Message) not shifted");
        CHECK(observer.strngrdEventLog->Cells[7][3] == "Barcode read fail  extra space", "show-all: row 3 Message field (embedded double-space, unnecessarily quoted) parsed verbatim");

        // --- JAM only filter (iJamCol=3, AlarmCode, prefix match) ----------
        observer.cbbFilter->ItemIndex = 1;
        observer.cbbFilter->Text = "JAM only";
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 3, "JAM only filter: RowCount == 3 (2 fixed + 1 match: JAM0301)");
        CHECK(observer.strngrdEventLog->Cells[3][1] == "JAM0301", "JAM only filter: the one matching row's AlarmCode is JAM0301");
        CHECK(observer.strngrdEventLog->Cells[7][1] == "Loader, jam near site A", "JAM only filter: matching row's Message column not shifted by its embedded comma");

        // --- WAR only filter -------------------------------------------------
        observer.cbbFilter->ItemIndex = 2;
        observer.cbbFilter->Text = "WAR only";
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 3, "WAR only filter: RowCount == 3 (2 fixed + 1 match: WAR16102)");
        CHECK(observer.strngrdEventLog->Cells[3][1] == "WAR16102", "WAR only filter: the one matching row's AlarmCode is WAR16102");

        // --- MES only filter -------------------------------------------------
        observer.cbbFilter->ItemIndex = 3;
        observer.cbbFilter->Text = "MES only";
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 3, "MES only filter: RowCount == 3 (2 fixed + 1 match: MES0101)");
        CHECK(observer.strngrdEventLog->Cells[3][1] == "MES0101", "MES only filter: the one matching row's AlarmCode is MES0101");

        // --- UnitName filter (the "else" branch, iJamCol=2, substring match) -
        observer.cbbFilter->ItemIndex = 4;
        observer.cbbFilter->Text = "Shuttle";
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 3, "UnitName filter \"Shuttle\": RowCount == 3 (2 fixed + 1 match)");
        CHECK(observer.strngrdEventLog->Cells[2][1] == "Shuttle", "UnitName filter: the one matching row's UnitName is Shuttle");
        CHECK(observer.strngrdEventLog->Cells[3][1] == "MES0101", "UnitName filter: matched row is the Shuttle/MES0101 one, not shifted");

        // --- file-not-found -> "No Record!!" ----------------------------------
        observer.lstEventLog->Clear();
        observer.lstEventLog->Items->Add(root + "\\2026\\08\\does_not_exist.csv");
        observer.lstEventLog->ItemIndex = 0;
        observer.cbbFilter->ItemIndex = 0;
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 2, "file-not-found: RowCount == 2 (the fixed \"No Record!!\" shape)");
        CHECK(observer.strngrdEventLog->Cells[1][1] == "No Record!!", "file-not-found: Cells[1][1] == \"No Record!!\"");

        // --- empty lstEventLog -> "No Record!!" (golden's OTHER no-record path) -
        observer.lstEventLog->Clear();
        observer.lstEventLog->ItemIndex = -1;
        observer.GetEventLogText();
        CHECK(observer.strngrdEventLog->RowCount == 2, "empty lstEventLog: RowCount == 2");
        CHECK(observer.strngrdEventLog->Cells[1][1] == "No Record!!", "empty lstEventLog: Cells[1][1] == \"No Record!!\"");
    }

    if (hadEventLogRootEnv)
    {
        AnsiString restore = AnsiString("W906_EVENTLOG_ROOT=") + savedEventLogRootEnv;
        _putenv(restore.c_str());
    }
    else
    {
        _putenv("W906_EVENTLOG_ROOT=");   // MinGW/MSVC _putenv: "NAME=" removes the variable
    }
    CUSTOMER_CODE = savedCustomerCode;
    IniConfig.bSPILFunction = savedSpil;
}

// =============================================================================
//  -- FW3-Obs2 ORACLE APPEND --  FW-3 cObserver Wave 2 test coverage
// =============================================================================
//  AI(W906-FW3-Observer-W2) 20260818: new oracle block, appended after Wave 1's
//  own 3 test functions (Test_CalculateStopTime_KnownValuesAndRoundTrip /
//  Test_BuildQuery_AlarmHistory / Test_GetEventLogText_FiltersAndNotFound,
//  their 40 checks all UNCHANGED above). Covers (per this wave's task brief,
//  "至少蓋 CalculateStopTime 之外的 3 個新方法（含一個時間記錄類的直寫 oracle）"):
//   (d) RecordIndexTime      -- direct-write oracle, the time-record class.
//   (e) AddTimeData          -- direct-write oracle, grid-delta arithmetic.
//   (f) GetTimeDataText      -- file-based (mirrors Wave 1's GetEventLogText
//                               test shape: real CSV / not-found / no-record),
//                               driven directly (no env-redirectable search
//                               step exists for this grid -- that's
//                               pgcMessageChange, NOT one of this wave's
//                               targets, see cObserver.cpp's own note).
//   (g) WriteCategoryData    -- branching/layout oracle, deliberately at an
//                               ALL-ZERO TastCategory baseline (see its own
//                               design note below) rather than fabricating
//                               TMySocket/ArmData fixture counts.
//   (h) CountMTBF            -- Wave 1's no-op stub retired this wave; verified
//                               with hand-computed arithmetic.
//   (i) ProcessRunInfo / GetMachineData -- LastSet-driven Caption assembly.
// =============================================================================
#include "LastSet.h"     // LastSet global -- ProcessRunInfo/GetMachineData/CountMTBF
#include "cpublic.h"     // ConvertMSecToTime/ConvertSecondToSPC -- comparison oracles below
#include "CosFunction.h" // CosFunction.bOEEFunction -- RecordIndexTime's OEE branch guard
#include "aHotPlateSubstrate.h" // TestSocket (NOT mykitsuck.h -- see KNOWLEDGE.md two-TMyKitSuck ODR gotcha,
                                //   same header cObserver.cpp itself uses) -- WriteCategoryData's
                                //   TastCategory.UpdataCount(true) input

// =============================================================================
//  (d) RecordIndexTime -- direct-write oracle (the time-record class)
// =============================================================================
static void Test_RecordIndexTime_ShiftAndAverage()
{
    TfObserver observer;
    bool savedOEE = CosFunction.bOEEFunction;
    CosFunction.bOEEFunction = false;   // skip the OEE sub-branch -- exercised structurally, not this test's focus

    for (int i = 0; i < 20; ++i)
        observer.fRecordIndexTime[i] = 0;

    observer.RecordIndexTime(5.0);
    CHECK(observer.fRecordIndexTime[0] == 5.0, "RecordIndexTime(5.0): fRecordIndexTime[0] == 5.0");
    CHECK(observer.fRecordIndexTime[11] == 5.0, "RecordIndexTime(5.0): average over 1 nonzero sample == 5.0");

    observer.RecordIndexTime(10.0);
    CHECK(observer.fRecordIndexTime[0] == 10.0, "RecordIndexTime(10.0): fRecordIndexTime[0] shifted to the new value");
    CHECK(observer.fRecordIndexTime[1] == 5.0, "RecordIndexTime(10.0): fRecordIndexTime[1] holds the PREVIOUS [0] (5.0)");
    CHECK(observer.fRecordIndexTime[11] == 7.5, "RecordIndexTime(10.0): average of {10.0,5.0} == 7.5");

    observer.RecordIndexTime(0.0);   // golden `if(fRecordIndexTime[i]!=0)` excludes zero samples from the average
    CHECK(observer.fRecordIndexTime[0] == 0.0, "RecordIndexTime(0.0): fRecordIndexTime[0] == 0.0");
    CHECK(observer.fRecordIndexTime[1] == 10.0, "RecordIndexTime(0.0): fRecordIndexTime[1] holds the PREVIOUS [0] (10.0)");
    CHECK(observer.fRecordIndexTime[2] == 5.0, "RecordIndexTime(0.0): fRecordIndexTime[2] holds the PREVIOUS [1] (5.0)");
    CHECK(observer.fRecordIndexTime[11] == 7.5, "RecordIndexTime(0.0): the new 0.0 sample is EXCLUDED -- average stays {10.0,5.0}/2 == 7.5");

    CosFunction.bOEEFunction = savedOEE;
}

// =============================================================================
//  (e) AddTimeData -- direct-write oracle, grid-delta arithmetic (iRow==7 branch)
// =============================================================================
static void Test_AddTimeData_ComputesDeltas()
{
    TfObserver observer;

    observer.AddTimeData(6, 100.0);    // sgTimeData->Cells[1][6] = "100"
    CHECK(observer.sgTimeData->Cells[1][6] == "100", "AddTimeData(6,100.0): Cells[1][6] == \"100\"");

    observer.AddTimeData(7, 130.5);    // iRow==7 branch: Cells[1][16] = Cells[1][7]-Cells[1][6] = 130.5-100 = 30.5
    CHECK(observer.sgTimeData->Cells[1][7] == "130.5", "AddTimeData(7,130.5): Cells[1][7] == \"130.5\"");
    CHECK(observer.sgTimeData->Cells[1][16] == "30.5", "AddTimeData(7,130.5): Cells[1][16] == Cells[1][7]-Cells[1][6] == \"30.5\"");
    CHECK(observer.sgTimeData->Cells[2][16] == "", "AddTimeData(7,130.5): Cells[2][16] left \"\" -- column-2 guard (Cells[2][7]/[2][6] both still \"\") correctly skipped");
}

// =============================================================================
//  (f) GetTimeDataText -- file-based (real CSV / not-found / no-record)
// =============================================================================
static void WriteScratchTimeDataCsv(const AnsiString &path)
{
    std::FILE *fp = std::fopen(path.c_str(), "wb");
    CHECK(fp != NULL, "(setup) scratch TimeData CSV created");
    if (!fp) return;
    const char *rows =
        "Motion Part,Current,Last 1\r\n"
        "DoArmPickFromLoadStage,12.3,45.6\r\n"
        "DoPlaceToHotPlate,7.8,9.0\r\n";
    std::fputs(rows, fp);
    std::fclose(fp);
}

// AI(W906-COMMATEXT-BCB6) 20261003: the same header quoted the way TStrings.CommaText (getter) writes items with blanks.
static void WriteScratchTimeDataCsvQuoted(const AnsiString &path)
{
    std::FILE *fp = std::fopen(path.c_str(), "wb");
    CHECK(fp != NULL, "(setup) scratch quoted TimeData CSV created");
    if (!fp) return;
    std::fputs("\"Motion Part\",Current,\"Last 1\"\r\n"
               "DoArmPickFromLoadStage,12.3,45.6\r\n", fp);
    std::fclose(fp);
}

static void Test_GetTimeDataText_FileAndNoRecord()
{
    TfObserver observer;
    AnsiString csvPath = ScratchDir() + "timedata_sample.csv";
    WriteScratchTimeDataCsv(csvPath);

    // --- real file ------------------------------------------------------
    observer.lstTimeData->Clear();
    observer.lstTimeData->Items->Add(csvPath);
    observer.lstTimeData->ItemIndex = 0;
    observer.GetTimeDataText();
    CHECK(observer.strngrdTimeData->RowCount == 3, "GetTimeDataText (real file): RowCount == 3 (1 header + 2 data rows)");
    // AI(W906-COMMATEXT-BCB6) 20261003: golden reads each line with TStrings.CommaText = SetDelimitedText (Delphi 6 classes.pas):
    //   an UNQUOTED item ends at ANY char <= ' ', so BCB6 shows this unquoted header as Motion | Part | Current | Last | 1.
    //   vclcompat = BCB6 since INBOX 152 (71be4468); this pin used to expect the old comma-only split ("Motion Part"), which BCB6
    //   never produced. The Handler's own TimeData lines come from the CommaText GETTER (cMyDB.cpp:652), which quotes items
    //   with blanks -- the quoted case below must round-trip intact.
    CHECK(observer.strngrdTimeData->Cells[0][0] == "Motion", "GetTimeDataText: unquoted header splits at the blank as BCB6 -- Cells[0][0] == \"Motion\"");
    CHECK(observer.strngrdTimeData->Cells[1][0] == "Part", "GetTimeDataText: unquoted header -- Cells[1][0] == \"Part\"");
    CHECK(observer.strngrdTimeData->Cells[4][0] == "1", "GetTimeDataText: unquoted \"Last 1\" -> Cells[3][0] \"Last\", Cells[4][0] == \"1\"");
    CHECK(observer.strngrdTimeData->Cells[1][1] == "12.3", "GetTimeDataText: row 1 Cells[1][1] == \"12.3\"");
    CHECK(observer.strngrdTimeData->Cells[2][2] == "9.0", "GetTimeDataText: row 2 Cells[2][2] == \"9.0\"");

    // --- quoted header (the form the CommaText getter writes) -------------
    // AI(W906-COMMATEXT-BCB6) 20261003: a quoted item keeps its blank (SetDelimitedText -> AnsiExtractQuotedStr).
    AnsiString csvQuoted = ScratchDir() + "timedata_quoted.csv";
    WriteScratchTimeDataCsvQuoted(csvQuoted);
    observer.lstTimeData->Clear();
    observer.lstTimeData->Items->Add(csvQuoted);
    observer.lstTimeData->ItemIndex = 0;
    observer.GetTimeDataText();
    CHECK(observer.strngrdTimeData->RowCount == 2, "GetTimeDataText (quoted header): RowCount == 2");
    CHECK(observer.strngrdTimeData->Cells[0][0] == "Motion Part", "GetTimeDataText (quoted header): Cells[0][0] == \"Motion Part\"");
    CHECK(observer.strngrdTimeData->Cells[2][0] == "Last 1", "GetTimeDataText (quoted header): Cells[2][0] == \"Last 1\"");
    CHECK(observer.strngrdTimeData->Cells[1][1] == "12.3", "GetTimeDataText (quoted header): Cells[1][1] == \"12.3\"");

    // --- file not found ---------------------------------------------------
    observer.lstTimeData->Clear();
    observer.lstTimeData->Items->Add(ScratchDir() + "does_not_exist_timedata.csv");
    observer.lstTimeData->ItemIndex = 0;
    observer.GetTimeDataText();
    CHECK(observer.strngrdTimeData->RowCount == 2, "GetTimeDataText (file not found): RowCount == 2");
    CHECK(observer.strngrdTimeData->Cells[1][1] == "No Record!!", "GetTimeDataText (file not found): Cells[1][1] == \"No Record!!\"");

    // --- empty lstTimeData (golden's OTHER no-record path) ----------------
    observer.lstTimeData->Clear();
    observer.lstTimeData->ItemIndex = -1;
    observer.GetTimeDataText();
    CHECK(observer.strngrdTimeData->RowCount == 2, "GetTimeDataText (empty lstTimeData): RowCount == 2");
    CHECK(observer.strngrdTimeData->Cells[1][1] == "No Record!!", "GetTimeDataText (empty lstTimeData): Cells[1][1] == \"No Record!!\"");
}

// =============================================================================
//  (g) WriteCategoryData -- branching/layout oracle at an ALL-ZERO baseline.
//
//  DESIGN NOTE: WriteCategoryData calls `TastCategory.UpdataCount(true)` TWICE
//  (golden's own idiom, kept verbatim in the port) -- a REAL recount that
//  iterates `TestSocket.iShtRow x TestSocket.iShtCol` reading `ArmData[arm]->
//  ArmSKET[row][col]` (real TMySocket counters). Rather than fabricate a
//  TMySocket/ArmData fixture (out of proportion for THIS method's own unique
//  contribution -- the grid layout/branching logic, already covered by
//  Wave5's atester_ProcessCount.cpp tests on the TastCategory/UpdataCount
//  side), this test forces TestSocket.iShtRow=iShtCol=0 so the recount touches
//  ZERO cells and TastCategory settles at a deterministic ALL-ZERO baseline
//  (ClearCount(), no accumulation). WriteCategoryData's OWN per-column loop
//  (driven by mtChName->Core.FXItem, fully test-controlled) still runs and is
//  what this test actually verifies.
// =============================================================================
static void Test_WriteCategoryData_ZeroBaseline()
{
    TfObserver observer;

    int savedShtRow = TestSocket.iShtRow;
    int savedShtCol = TestSocket.iShtCol;
    bool savedTwoArm = bUseTwoArm32Site;
    TestSocket.iShtRow = 0;
    TestSocket.iShtCol = 0;
    bUseTwoArm32Site = false;

    // AI(W906-FW3-Obs2fix) 20260818: the Wave-2 FINDING that used to live here
    // (ctor left every non-mtRow tray at TrayCore's default 2x2, so
    // WriteCategoryData writes past (1,1) were SILENTLY DROPPED -- this test
    // caught mtTotal cell(0,3) reading back "" instead of "0") is now FIXED at
    // the source: the ctor carries a .dfm-hydration block sizing all 16 trays
    // from tools/dfm2rc/ir_out/cObserver.dfm.ir.json. The manual
    // SetXItem/SetYItem workaround this test used is deliberately REMOVED so
    // the assertions below now verify the ctor's own hydration end to end --
    // reintroducing the 2x2 default makes them fail again.
    observer.rbHeadNumber->Checked = false;
    observer.rbSocketNumber->Checked = true;    // forces the NUMBER branch AND mtCategoryNo->XItem==8,
    observer.rbSocketPercent->Checked = false;  // independent of IsNNMode()'s (uncontrolled-by-this-test) value
    observer.rbHeadPercent->Checked = false;

    observer.rgRowNo->ItemIndex = 0;
    observer.WriteCategoryData();
    CHECK(observer.mtRowName->Core.GetCellText(0, 0) == "RowA", "WriteCategoryData (RowNo=0, bUseTwoArm32Site=false): mtRowName == \"RowA\"");
    CHECK(observer.mtCategoryNo->Core.FXItem == 8, "WriteCategoryData (rbSocketNumber checked): mtCategoryNo->XItem == 8");
    CHECK(observer.mtTotal->Core.GetCellText(0, 1) == "0", "WriteCategoryData (zero baseline): mtTotal cell(0,1) [iTotalSocket] == \"0\"");
    CHECK(observer.mtTotal->Core.GetCellText(0, 3) == "0", "WriteCategoryData (zero baseline): mtTotal cell(0,3) [iPassSocket] == \"0\"");

    observer.rgRowNo->ItemIndex = 1;
    observer.WriteCategoryData();
    CHECK(observer.mtRowName->Core.GetCellText(0, 0) == "RowB", "WriteCategoryData (RowNo=1, bUseTwoArm32Site=false): mtRowName == \"RowB\"");

    TestSocket.iShtRow = savedShtRow;
    TestSocket.iShtCol = savedShtCol;
    bUseTwoArm32Site = savedTwoArm;
}

// =============================================================================
//  (h) CountMTBF -- hand-computed arithmetic (Wave 1's no-op stub retired this wave)
// =============================================================================
static void Test_CountMTBF_KnownValues()
{
    TfObserver observer;

    observer.pnlTotalCount->Caption = "2";       // dJamCount
    observer.lbltTotalLoader->Caption = "10";    // dLoadingCount
    observer.iPauseTime = 60;
    observer.iProductTime = 60;
    observer.iJamTime = 0;                        // iMTBFTime = 120 -> iMins = 120/60=2 -> 2/dJamCount(2) = 1 -> "1 / 1"

    observer.DateTimePicker1->Date = EncodeDate(2026, 8, 18);
    observer.DateTimePicker3->Date = EncodeDate(2026, 8, 18);   // same day -> 0 date delta
    observer.DateTimePicker2->Time = EncodeTime(0, 0, 0, 0);
    // AI(W906-FW3-Obs2-cal) 20260818: was EncodeTime(5,0,0,0), which puts
    // iUPH = 10/dTime EXACTLY on the int-truncation boundary: dTime is
    // (5/24)*24, and 5/24 is not binary-representable, so the product is
    // 5.0 +/- 1ulp -- x87 keeps it in an 80-bit register for a different
    // lifetime under -O3, and the Release build truncated 10/(5+ulp) to 1
    // while Debug got 2 (caught by the double-build gate, invisible in
    // Debug alone). +4h puts the quotient at 2.5, safely interior: 10/(4
    // +/- ulp) truncates to 2 under every rounding. The production code is
    // untouched -- golden's own math sits on this boundary by design.
    observer.DateTimePicker4->Time = EncodeTime(4, 0, 0, 0);    // +4h -> iUPH = int(10/4.0) = 2 (interior)

    int iSGCountBefore = observer.strngrdMDBQuery->RowCount;
    observer.CountMTBF();

    CHECK(observer.strngrdMDBQuery->RowCount == iSGCountBefore + 7, "CountMTBF: RowCount grew by exactly 7");
    CHECK(observer.strngrdMDBQuery->Cells[1][iSGCountBefore + 2] == "MTBA", "CountMTBF: Cells[1][iSGCount+2] == \"MTBA\"");
    CHECK(observer.strngrdMDBQuery->Cells[2][iSGCountBefore + 2] == "1 / 1", "CountMTBF: asMTBA == \"1 / 1\" (iMTBFTime=120 -> iMins=2 -> 2/dJamCount(2)=1)");
    CHECK(observer.strngrdMDBQuery->Cells[3][iSGCountBefore + 2] == "[mins]", "CountMTBF: Cells[3][iSGCount+2] == \"[mins]\"");
    CHECK(observer.strngrdMDBQuery->Cells[1][iSGCountBefore + 3] == "MUBA", "CountMTBF: Cells[1][iSGCount+3] == \"MUBA\"");
    CHECK(observer.strngrdMDBQuery->Cells[2][iSGCountBefore + 3] == "1 / 5", "CountMTBF: asMUBA == \"1 / 5\" (dLoadingCount(10)/dJamCount(2)=5)");
    CHECK(observer.strngrdMDBQuery->Cells[1][iSGCountBefore + 6] == "UPH", "CountMTBF: Cells[1][iSGCount+6] == \"UPH\"");
    CHECK(observer.strngrdMDBQuery->Cells[2][iSGCountBefore + 6] == "2", "CountMTBF: iUPH == 2 (int(10/4.0)==2, interior point)");
}

// =============================================================================
//  (i) ProcessRunInfo / GetMachineData -- LastSet-driven Caption assembly
// =============================================================================
// =============================================================================
//  (Q5) StatisticalJamCount family -- golden :5060-5329 (AI(W906-FW-Q5) 20260818)
// =============================================================================
static void Test_StatisticalJamCount_Family()
{
    AnsiString root = ScratchDir() + "jamcount_root";

    // The function locates its input CSV at slEventLog->Path\YYYY\MM\
    // <slEventLog->FileName>_YYYYMMDD.csv using GetTimeInfo()'s TODAY --
    // sample the same clock source to build the fixture.
    GetTimeInfo();
    AnsiString monthDir; monthDir.sprintf("%s\\%04d\\%02d", root, (int)SystemYear, (int)SystemMonth);
    MyForceDirectories(monthDir);
    AnsiString csvPath; csvPath.sprintf("%s\\HTLog_%04d%02d%02d.csv", monthDir, (int)SystemYear, (int)SystemMonth, (int)SystemDate);
    {
        std::FILE *fp = std::fopen(csvPath.c_str(), "wb");
        CHECK(fp != NULL, "(setup) scratch JamCount EventLog CSV created");
        if (!fp) return;
        // 2x JAM0301 + 1x JAM0302 + 1 non-JAM: exercises the new-code path,
        // the duplicate-count path, and the JAM-prefix filter in one file.
        // Row 1's Message keeps the embedded-comma quoting (load-bearing:
        // CommaText would column-shift without it -- the same hazard the
        // EventLog memory documents).
        const char *rows =
            "Date,Time,UnitName,AlarmCode,Recovery,StopedTime,Duplicate,Message,ErrPart\r\n"
            "2026-08-18,09:00:00,InArm,JAM0301,1,12,0,\"Loader, jam near site A\",InArm1\r\n"
            "2026-08-18,09:05:00,OutArm,WAR16102,1,3,0,\"Clean pad worn out\",OutArm2\r\n"
            "2026-08-18,09:10:00,InArm,JAM0301,1,7,0,\"Loader, jam near site A\",InArm1\r\n"
            "2026-08-18,09:15:00,Shuttle,JAM0302,1,5,0,\"Shuttle jam\",Shuttle1\r\n";
        std::fputs(rows, fp);
        std::fclose(fp);
    }

    // --- state setup with full save/restore (cross-test hygiene) ------------
    bool savedInitialOK = InitialOK;
    TMyStringList *savedSlEventLog = slEventLog;
    int savedLoaderCount = iOneDayLoaderCount;
    AnsiString savedHandlerID = IniConfig.asA32_1_HandlerID;
    bool savedFtpFlag = IniConfig.bN26_UseJamRawDataUpdataToFTP;
    AnsiString savedEnv = getenv("W906_EVENTLOG_ROOT") ? AnsiString(getenv("W906_EVENTLOG_ROOT")) : AnsiString("");
    bool hadEnv = getenv("W906_EVENTLOG_ROOT") != NULL;
    AnsiString envAssign = AnsiString("W906_EVENTLOG_ROOT=") + root;
    _putenv(envAssign.c_str());

    InitialOK = true;
    slEventLog = new TMyStringList(root, "HTLog", "hdr");
    iOneDayLoaderCount = 100;
    IniConfig.asA32_1_HandlerID = "";           // -> "HandlerID" default branch
    IniConfig.bN26_UseJamRawDataUpdataToFTP = false;   // FTP tail is (Q5a)-gated anyway

    {
        TfObserver observer;
        observer.StatisticalJamCount(false);

        CHECK(observer.strngrdJamLog->Cells[2][1] == "JAM0301", "StatisticalJamCount: row 1 AlarmCode == JAM0301 (JAM-prefix filter passed, WAR row skipped)");
        CHECK(observer.strngrdJamLog->Cells[1][1] == "InArm", "StatisticalJamCount: row 1 UnitName from CSV field 2");
        CHECK(observer.strngrdJamLog->Cells[3][1] == "Loader, jam near site A", "StatisticalJamCount: quoted embedded-comma Message survives CommaText parse (quoting is load-bearing)");
        CHECK(observer.strngrdJamLog->Cells[4][1] == "2", "StatisticalJamCount: JAM0301 counted twice (duplicate-count path, golden :5164-5166)");
        CHECK(observer.strngrdJamLog->Cells[2][2] == "JAM0302", "StatisticalJamCount: second distinct code lands on row 2 (new-code path)");
        CHECK(observer.strngrdJamLog->Cells[4][2] == "1", "StatisticalJamCount: JAM0302 counted once");
        CHECK(observer.strngrdJamLog->Cells[5][1] == "2.00", "StatisticalJamCount: Rate(%) = 2/100 -> \"2.00\" (ChangeToFloat is the PERCENTAGE helper, MachineType.h:1593 \"取出 float %\" -- first draft of this check wrongly expected the raw quotient 0.02)");
        CHECK(observer.labLoaderCount->Caption == "100", "StatisticalJamCount: labLoaderCount shows iOneDayLoaderCount (bIsNextDay=false keeps it)");
        CHECK(DirectoryExists(root + "\\SGJamCount"), "StatisticalJamCountEnable: SGJamCount dir created under the W906_EVENTLOG_ROOT redirect (NOT D:\\HT9045_Log)");

        // --- LoaderCount write/read round-trip ------------------------------
        observer.StatisticalLoaderCount();
        CHECK(iOneDayLoaderCount == 101, "StatisticalLoaderCount: increments the counter");
        CHECK(FileExists(root + "\\SGJamCount\\LoaderCount.txt"), "StatisticalLoaderCount: LoaderCount.txt written under the redirect root");
        iOneDayLoaderCount = 0;
        observer.ReadLoaderCount();
        CHECK(iOneDayLoaderCount == 101, "ReadLoaderCount: reads back the persisted count (write/read round-trip)");

        // --- bIsNextDay=true resets the day counter (uses YESTERDAY's date;
        //     that CSV doesn't exist -> early return BEFORE the reset, so
        //     exercise only the documented early-return here) ----------------
        //
        // AI(W906-P0-5) 20260920: **這裡原本是一顆定時炸彈，20260920 才炸。**
        //   上面那句「that CSV doesn't exist」是**假設**，不是這個測試建立的
        //   狀態 —— 而 `ScratchDir()`（:68-71）回的是**相對路徑**
        //   `test_observer_core_scratch\`，解析到 `build/tests/` 之下，
        //   **跨次執行保留**。於是：
        //
        //     昨天跑過一次 -> 留下 jamcount_root\YYYY\MM\HTLog_<昨天>.csv
        //     今天再跑     -> 「昨天的 CSV」存在 -> 不會 early-return
        //                     -> 計數器被動到 -> 這一條 FAIL
        //
        //   實測證據（20260920 22:33 那一輪）：
        //     jamcount_root/2026/09/HTLog_20260919.csv           mtime 09-19 09:13（昨天那輪寫的）
        //     .../SGJamCount/2026/09/HandlerID_20260919_RawData.csv mtime 09-20 22:33（今天寫進昨天的檔）
        //   第二行就是「沒有 early-return、走完整路徑」的直接證據。
        //
        //   ⇒ **這支測試只在「前一天沒跑過測試」的日子會綠**，與被測程式無關。
        //     一個會依執行歷史變色的測試，等於訓練人忽略它的紅燈。
        //   ⇒ 修法：把前提**建立出來**，而不是假設它成立 ——
        //     用被測程式自己的 `GetYesterdayInfo()` 算出同一條路徑再刪掉。
        //     刻意不用「整個 scratch dir 開跑前清空」那種寫法：那會連
        //     上面 LoaderCount 的 write/read round-trip 一起洗掉。
        GetYesterdayInfo();
        {
            AnsiString yMonthDir; yMonthDir.sprintf("%s\\%04d\\%02d", root,
                                                    (int)SystemYearYesterday, (int)SystemMonthYesterday);
            AnsiString yCsv; yCsv.sprintf("%s\\HTLog_%04d%02d%02d.csv", yMonthDir,
                                          (int)SystemYearYesterday, (int)SystemMonthYesterday,
                                          (int)SystemDateYesterday);
            std::remove(yCsv.c_str());          // 不存在就是 no-op，這正是要的
            CHECK(FileExists(yCsv) == false,
                  "(setup) yesterday's CSV really is absent -- 這一條是上面那顆定時炸彈的哨兵，"
                  "它紅了就表示刪不掉，而不是被測程式有問題");
        }
        int before = iOneDayLoaderCount;
        observer.StatisticalJamCount(true);
        CHECK(iOneDayLoaderCount == before, "StatisticalJamCount(true): yesterday's CSV absent -> early return, counter untouched (golden :5106-5111)");
    }

    delete slEventLog;
    slEventLog = savedSlEventLog;
    InitialOK = savedInitialOK;
    iOneDayLoaderCount = savedLoaderCount;
    IniConfig.asA32_1_HandlerID = savedHandlerID;
    IniConfig.bN26_UseJamRawDataUpdataToFTP = savedFtpFlag;
    AnsiString envRestore = AnsiString("W906_EVENTLOG_ROOT=") + (hadEnv ? savedEnv : AnsiString(""));
    _putenv(envRestore.c_str());
}

static void Test_ProcessRunInfo_And_GetMachineData()
{
    TfObserver observer;
    int savedCustomerCode = CUSTOMER_CODE;
    bool savedVTEST = IniConfig.bVTESTFunction;
    IniConfig.bVTESTFunction = false;   // skip the pnlDayJamRate branch, not this test's focus

    LastSet.SystemAccSecond[0][stPowerOn]    = 3600000;   // 3600 sec
    LastSet.SystemAccSecond[0][stStartTime]  = 1800000;
    LastSet.SystemAccSecond[0][stProductTime]= 600000;    // 600 sec
    LastSet.SystemAccSecond[0][stPauseTime]  = 60000;     // 60 sec
    LastSet.SystemAccSecond[0][stJamTime]    = 0;
    LastSet.iJamCount[1] = 0;   // both MTBA/MUBA take golden's "0 / ..." branch
    LastSet.SendCT[0] = 111;
    LastSet.SendCT[1] = 222;

    CUSTOMER_CODE = 0;   // not CC_AMKOR_Korea -> labLoadingCount reads SendCT[1]
    observer.GetMachineData();   // calls ProcessRunInfo() internally (golden :766)

    CHECK(observer.labPowerOnTime->Caption == ConvertMSecToTime(3600000), "GetMachineData: labPowerOnTime == ConvertMSecToTime(stPowerOn)");
    CHECK(observer.labLoadingCount->Caption == "222", "GetMachineData (not CC_AMKOR_Korea): labLoadingCount == SendCT[1] (\"222\")");
    CHECK(observer.labMTBA->Caption == AnsiString("0 / ") + ConvertSecondToSPC(660), "ProcessRunInfo (iJamCount[1]==0): labMTBA == \"0 / \"+ConvertSecondToSPC(660) (dSec=(60000+600000+0)/1000=660)");
    CHECK(observer.labMUBA->Caption == "0 / 222 unit", "ProcessRunInfo (iJamCount[1]==0): labMUBA == \"0 / 222 unit\" (SendCT[1])");
    CHECK(observer.labMTBF->Caption == ConvertSecondToSPC(3600), "ProcessRunInfo: labMTBF == ConvertSecondToSPC(stPowerOn/1000)");

    // --- CC_AMKOR_Korea branch: labLoadingCount switches to SendCT[0], and a
    //     nonzero iJamCount[1] takes the "1 / ..." ChangeToFloatNonPcnt branch --
    CUSTOMER_CODE = CC_AMKOR_Korea;
    LastSet.iJamCount[1] = 2;
    observer.GetMachineData();
    CHECK(observer.labLoadingCount->Caption == "111", "GetMachineData (CC_AMKOR_Korea): labLoadingCount == SendCT[0] (\"111\")");
    CHECK(AnsiString(observer.labMTBA->Caption).SubString(1, 4) == "1 / ", "ProcessRunInfo (iJamCount[1]=2!=0): labMTBA takes the \"1 / ...\" branch, not \"0 / ...\"");

    CUSTOMER_CODE = savedCustomerCode;
    IniConfig.bVTESTFunction = savedVTEST;
}

int main()
{
    // --- one-time safety redirect: BOTH production config paths TfObserver's
    //     ctor touches, BEFORE constructing anything -- see file-head SAFETY
    //     note. `AuthPath` follows tests/test_ga1_cprod.cpp's own established
    //     precedent (:322-325); `asGeneralPath`/OpenGeneralIniFile is the
    //     analogous extension for INIFileGeneral, using the same extern
    //     globals + public open function common.h already provides.
    AnsiString savedAuthPath = AuthPath;
    AnsiString savedGeneralPath = asGeneralPath;
    AuthPath = ScratchDir();
    MyForceDirectories(AuthPath);
    asGeneralPath = ScratchDir() + "general_test.ini";
    OpenGeneralIniFile();

    Test_CalculateStopTime_KnownValuesAndRoundTrip();
    Test_BuildQuery_AlarmHistory();
    Test_GetEventLogText_FiltersAndNotFound();

    // -- FW3-Obs2 ORACLE APPEND -- Wave 2's new coverage (see banner above) --
    Test_RecordIndexTime_ShiftAndAverage();
    Test_AddTimeData_ComputesDeltas();
    Test_GetTimeDataText_FileAndNoRecord();
    Test_WriteCategoryData_ZeroBaseline();
    Test_CountMTBF_KnownValues();
    Test_StatisticalJamCount_Family();
    Test_ProcessRunInfo_And_GetMachineData();

    CloseGeneralIniFile();
    AuthPath = savedAuthPath;
    asGeneralPath = savedGeneralPath;

    std::printf("%d/%d checks passed (test_observer_core)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
