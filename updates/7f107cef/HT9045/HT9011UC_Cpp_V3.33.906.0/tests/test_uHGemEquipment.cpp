// =============================================================================
//  tests/test_uHGemEquipment.cpp -- verification harness for the new
//  SECSGEM/uHGemEquipment.{h,cpp} translation (THGem's CEID/Report/Alarm
//  StringGrid "database" family + GemTimer) and the new
//  vclcompat/StringGrid.{h,cpp} shim.
//
//  AI(W906-uHGemEquipment) 20260716: new file.
//
//  Golden reference:
//    HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.h:31-42,139-144
//    HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.cpp
//      (GemTimer :250-309; CEID/Report family :7361-7967; Alarm-grid
//       siblings :6211-6274; StringGrid tab-format :8034-8195,8615-8665)
//
//  BUILD: wired into tests/CMakeLists.txt as the `test_uHGemEquipment` target
//  (`ctest -R uHGemEquipment` or `ctest --test-dir build`), linking
//  ht9045_secsgem + ht9045_public + vclcompat. AI(W906-uHGemEquipment-
//  ConnLifecycle) 20260717: the connection-lifecycle slice added below pulls
//  in ClientSocket.cpp/ServerSocket.cpp/WinSocketErrorCode.cpp transitively
//  via those libraries -- if ever building this file standalone (outside
//  CMake) for quick iteration, add those three .cpp files to the command
//  alongside the ones already listed for the original StringGrid-family
//  slice (uHGemEquipment.cpp, StringGrid.cpp, AnsiString.cpp, TStringList.cpp,
//  SysUtils.cpp, TDateTime.cpp, IniFiles.cpp, SecsEventType.cpp).
//
//  *** SAFETY NOTE -- READ BEFORE EXTENDING THIS FILE ***
//  ---------------------------------------------------------------------------
//  THGem::SetCEIDContent (both overloads), THGem::SetReportIDContent, and
//  THGem::EnableDisableEventReport ALL end by calling SaveEventReportData(),
//  which -- faithfully matching golden (uHGemEquipment.cpp:8650-8665) --
//  unconditionally SAVES to the HARDCODED absolute path
//  "D:\HT9045\SECS\SECS\SYSTEM\EventReport_CEID.def" /
//  "...\EventReport_ReportID.def", with NO parameter/instance-state way to
//  redirect it. Those exact files EXIST as real data on the reference dev
//  machine this test was authored on (confirmed: they are the actual
//  production SECS/GEM event-report definition files for the D:\HT9045 tree,
//  a location strictly outside this migration's write boundary -- see
//  AGENTS.md / this task's own instructions: "絕對不要碰 ... D:/HT9045 repo
//  根目錄下其他任何檔案"). THIS TEST FILE THEREFORE NEVER CALLS
//  SetCEIDContent / SetReportIDContent / EnableDisableEventReport /
//  SaveEventReportData -- doing so would silently overwrite real files
//  outside this task's allowed scope.
//
//  Those 3 methods' grid-mutation ALGORITHMS are instead verified by:
//    (a) hand-trace, documented inline at each method's own definition in
//        SECSGEM/uHGemEquipment.cpp (see the comments there), and
//    (b) this file's [5]/[6] tests below, which exercise the SAME grid
//        layout those methods produce/consume (strGrdCEID cols
//        0=CEID,1=Mode/Enable,2=Alias,3.. = ReportIDs; stdGridReportID cols
//        0=CEID,1=Mode,2.. = SVIDs) by seeding cells DIRECTLY (bypassing the
//        two dangerous writer methods entirely) and then exercising every
//        READ/DELETE method in the family against that hand-seeded data --
//        those 15 methods (GetCEIDContent, GetReportIDContent, IsEnableEvent,
//        CheckCeidExist, CheckCEIDExist, GetReportIDType,
//        CheckCeidAlreadyDefine, CheckReportIDExist, AddCeidReportID,
//        DeleteReportID, DeleteReportIDOfCeid, DeleteAllHostDefineReportID,
//        DeleteAllHostReportIDOfCeid, DeleteHostReportIDOfAssignCeid,
//        DeleteAllHostDefineCeid, CheckReportAlreadyDefine) have NO
//        SaveEventReportData call anywhere in their own bodies (verified by
//        reading each one) and are exercised directly, for real.
//
//  ReadEventReportData is READ-ONLY (LoadFromFile, never writes) -- test [7]
//  below DOES call it for real, against the real on-disk files, as a genuine
//  oracle (guarded by FileExists so the test degrades gracefully, rather than
//  failing, on a machine without that exact path).
//
//  ReadAlamData/WriteAlamData ARE exercised directly (test [8]) because,
//  unlike the CEID/Report family, their path is DERIVED from the
//  `GemSystemPath` instance field (not hardcoded) -- this test points
//  GemSystemPath at a throwaway scratch folder under the current working
//  directory, never at the real D:\HT9045 tree.
//
//  AI(W906-uHGemEquipment-ConnLifecycle) 20260717: EXTENDED for the new
//  TCP/IP connection-lifecycle slice (clientGem/srvGem event handlers,
//  DoOpenCommuncation/DoOnLine/OnlineLocalOrRemote, Connect/DisConnect/
//  IsConnect/OnLine family, CheckSocketActiveFalse, ClearDefaultEvenReport,
//  GetTimeInfo). Golden reference for the new slice:
//    HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.cpp
//      (:315-348, 392-439, 2100-2142, 3382-3490, 3604-3645, 4988-5008,
//       5146-5159, 5548-5644, 6812-6910)
//
//  *** SAFETY NOTE for the NEW tests -- read before extending further ***
//  srvGemClientError (tested in [12] below) faithfully calls
//  SaveSECSGEMErrToLog, which -- matching golden exactly -- unconditionally
//  APPENDS to a hardcoded absolute path outside this repo/migration tree:
//  D:\SECS_GEM_LOGS\<yyyy>\<mm_dd>\SECSGEM_ErrLog_<hh>.txt (creating that
//  directory tree via ForceDirectories if absent). This is a real,
//  unavoidable side effect of exercising srvGemClientError's genuine
//  translated behavior (there is no GemSystemPath-style parameter to redirect
//  it, unlike ReadAlamData/WriteAlamData above) -- but it is fundamentally
//  LOWER RISK than SaveEventReportData's own hardcoded-path danger (which
//  OVERWRITES real production CEID/ReportID configuration): this is a small,
//  append-only diagnostic log line, never destructive to existing data, and
//  no worse than what golden's own production binary already does on every
//  real socket error.
//
//  AI(W906-uHGemEquipment-ConnLifecycle) 20260717: per explicit user
//  direction, exercising this real path during dev/test is acceptable -- the
//  actual requirement is that ctest must not leave the real archive dirtier
//  after a run than before it (a first run of this test, before this note,
//  did leave 2 stray lines behind in D:\SECS_GEM_LOGS\2026\07_17\
//  SECSGEM_ErrLog_10.txt -- left in place per user decision, now treated as
//  the accepted baseline). Test [12] below therefore captures that target
//  file's exact pre-test byte content (using the same yyyy/mm_dd/hh tokens
//  SaveSECSGEMErrToLog itself computes) and restores it verbatim immediately
//  after the assertions -- deleting the file/newly-created parent
//  directories entirely if none existed before this run. Repeat ctest runs
//  should therefore leave D:\SECS_GEM_LOGS byte-for-byte unchanged.
//  SaveSECSGEMErrToLog is otherwise NOT called directly by any other test
//  below (matching the SaveEventReportData precedent).
//
//  AI(W906-uHGemEquipment-BucketB) 20260717: EXTENDED again for the ~15
//  widget stand-ins (THGemRadioGroup/THGemEdit/THGemCheckBox/THGemComboBox/
//  THGemPanel/THGemSpeedButton/THGemMemo) and their first real consumers:
//  InitialHGem/SaveSystemDefault (scratch GemSystemIniPath, never the real
//  D:\HT9045\SECS tree -- same scratch-path discipline as test [8]'s
//  GemSystemPath), DoUpdateStatus (panel/button/EventReport refresh, INCLUDING
//  the KYEC 30-second forced-disconnect branch), ManualCreatergRoleClick,
//  ProcessShow, and StringOut(2-arg). New includes: cmydef.h (CUSTOMER_CODE/
//  CC_KYEC_LEE/CC_MAXIM_THAILAND -- this test now links ht9045_core/
//  ht9045_globals too, see tests/CMakeLists.txt's own updated comment) and
//  SECSGEM/SecsEventReport.h (g_SimLastEventReportCeid/g_SimEventReportCount/
//  ResetSimEventReport).
//
//  AI(W906-FW-GEM-W9) 20260826: test [22] 的兩條 EventReport 斷言**已改**，
//  不再走上面那組 Sim 計數器。原因不是測試寫錯，是它釘住了一個 port 缺陷：
//  DoUpdateStatus 當時把 golden 的 `EventReport(1, N)` 寫成 `EventReport(N)`，
//  於是打到 SecsEventReport.h 的自由函式（同名不同物）。golden 對應四處是
//  uHGemEquipment.cpp:4966/:4974/:4978/:4982，全部兩個參數；golden 全樹掃
//  `EventReport\s*\(\s*\d+\s*\)` 是 0 個命中。W9 翻進 THGem::EventReport
//  並回復 golden 原文後，改成觀察真本體的 LogDataString 輸出。
//
//  NOT COVERED（因上述改動而失去的覆蓋，明列以免日後誤以為有測到）：
//    * 自由函式 EventReport(unsigned Ceid) 本身（SecsEventReport.h）在本檔
//      **不再有任何呼叫點**。它的既有覆蓋在 csystem.cpp 那邊的測試，不在這裡。
//    * THGem::EventReport 的 enabled 分支（IsEnableEvent()==true -> InitLocalHead
//      + DataItemOut + SendCeid/SendAnnotatedCeid + SendLocalData，golden
//      :7712-7732）本測試沒有進去——[22] 用的是空的 strGrdCEID。
//    * THGem::EventReport 的 iCeid==SECS_EVENT.DoExit 收尾段（golden :7742-7758）
//      其中 Timer1->Enabled 與 Close() 兩行是 GATE (W9-ExitTail)，本來就沒有活碼。
//
//  AI(W906-uHGemEquipment-BucketC) 20260717: EXTENDED again -- see this
//  file's own T1-T10 block near the end (socket receive pump / real HTimer /
//  T3 timeout / HSMS control-message handshake / Timer1Timer). Test [22]'s
//  own KYEC branch (previously "structural only" against an always-fires
//  HTimer stand-in) is UPDATED here too: SECSGEM_DoSeparate is now the REAL
//  elapsed-time HTimer (D6), so that assertion now correctly expects the
//  30-second forced-disconnect to NOT fire on the same poll it was armed --
//  see that test's own updated comment.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"
#include "SECSGEM/SecsEventReport.h"   // 自由函式 EventReport(unsigned) 的宣告（Sim 計數器本檔已不使用，見檔頭 W9 註記）
#include "cmydef.h"                     // CUSTOMER_CODE / CC_KYEC_LEE / CC_MAXIM_THAILAND / CosFunction
// AI(W906-uHGemEquipment-BucketC) 20260717: D3 -- HSys.MyGem seam. Needed so
// this test binary can exercise the null-guard sites (DoConnect/
// DoProcessSFNoResponse/ProcessSocketReceiveData's catch) AND (T7) wire a
// REAL HTGem instance to HSys.MyGem to prove the SendLocalDataHook seam works
// end-to-end.
#include "database.h"            // HSys / SYSTEM_MODULAR
#include "SECSGEM/uHGemClass.h"  // HTGem (T7)
#include "Config.h"              // IniConfig.bEnable_SECS_GEM (T10)
// AI(W906-uHGemClass-Micro6) 20260721: LastDataPath/GetLastOpenFN/
// WriteLastDataFN for the S7F18 "currently open recipe" guard coverage below
// -- same repoint-before-use safety posture as test_common.cpp's own "E)"
// test (this file's own SAFETY NOTE above governs why: never touch the real
// D:\HT9045\SetUp.inf-backed default).
#include "common.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#include <windows.h>   // ::Sleep (GemTimer elapsed-time test)

// AI(W906-uHGemEquipment-BucketC) 20260717: bSECSGEM_DoSeparate has EXTERNAL
// linkage in uHGemEquipment.cpp (a plain file-scope global, golden :24) --
// not declared in uHGemEquipment.h itself (an internal implementation detail
// shared between DoUpdateStatus and Timer1Timer). Declared here, test-only,
// so T10 (below) can observe its latch behavior directly. Its siblings
// `SECSGEM_DoSeparate`/`SECSGEM_DoSeparateWait` (the two HTimer globals right
// next to it in that file) canNOT be similarly declared here -- HTimer is a
// deliberately TU-local (anonymous-namespace) type, not nameable from this
// separate translation unit; their behavior is instead observed indirectly
// (test [22]'s KYEC-branch assertions, and T10's own bWaitHTimer-path check).
extern bool bSECSGEM_DoSeparate;

// ---------------------------------------------------------------------------
//  MyDBIProcess (2-arg) -- this test binary does not link aHotPlateSubstrate.cpp
//  (the one real definition elsewhere in this tree); ReadAlamData's catch-block
//  needs SOME definition to link against (same pattern as
//  tests/test_FTPClient_EventHandlers.cpp / test_uHGemClass.cpp).
// ---------------------------------------------------------------------------
// AI(W906-GA1-B2-integrate) 20260804: local MyDBIProcess stub RETIRED (real body now in link group)

// ---------------------------------------------------------------------------
//  AI(W906-uHGemEquipment-BucketC) 20260717: ShowMyMessage -- as of this
//  wave, uHGemEquipment.cpp's new `#include "database.h"` (D3) means this
//  test binary now links ht9045_db (database.cpp, which defines the
//  referenced-by-this-TU global `HSys`). Static-archive linking pulls in
//  database.cpp's WHOLE object file once `HSys` is referenced -- which means
//  database.cpp's OWN unresolved ShowMyMessage external (database.cpp:65)
//  must ALSO resolve, even though this test never calls LoadIoData/
//  LoadMotData itself.
//
//  AI(W906-FastcallFix) 20260720: this comment used to also cover a local
//  MyDBIProcess (3-arg, __fastcall) stub that sat on the line right below,
//  needed for the SAME database.cpp-pulled-in-whole reason (its OTHER
//  unresolved external, database.cpp:64/75). That stub is REMOVED now:
//  SECSGEM/uHGemEquipment.cpp itself supplies the real, externally-linkable,
//  __fastcall-decorated definition of that overload as of this wave (moved
//  out of an anonymous namespace and given __fastcall), and this test
//  target already links ht9045_secsgem (the library that carries it, per
//  the RESCAN link group above in tests/CMakeLists.txt) -- so a local stub
//  here would now be a duplicate-definition link error, not a
//  missing-symbol fix (2026-07-20 audit, AUDIT_fastcall_tree.md finding 1).
//  The removed stub's body was a pure no-op (no capture/counter), so no test
//  coverage is lost by this removal.
// ---------------------------------------------------------------------------
// AI(W906-GA1-B2-integrate) 20260804: local ShowMyMessage stub RETIRED (real body now in link group)

// ---------------------------------------------------------------------------
//  AI(W906-uHGemClass-Unlock3) 20260723: HasICUnderMachine / HasAnyICInMachine
//  stubs -- this wave un-gated SECSGEM/uHGemClass.cpp's S2F16, which calls
//  these two real csystem.h predicates (implemented in csystem_predicates.cpp,
//  part of ht9045_sm, which this test does NOT link). uHGemClass.cpp.o is
//  unconditionally part of ht9045_secsgem (this test already links it), so
//  its now-undefined refs to these two predicates must resolve at THIS
//  test's link too, even though this test never exercises HTGem/S2F16 at
//  all. Same "avoid the god-stack ht9045_sm for a focused unit test" posture
//  already established by this file's own MyDBIProcess/ShowMyMessage stubs
//  immediately above. Conservative `false` here is NOT because the real
//  bodies still have a gated/TODO false-by-default path -- csystem_predicates.
//  cpp:56 has HT9045_KITSUCK_GRID_AVAILABLE=1 (flipped on by the W7
//  substrate wave, 20260629), so the real HasICUnderMachine() actively
//  evaluates ShuttleHasIC()||IndexHasIC()||HasICUnderHotPlate() on the real
//  (wired) TMyKitSuck grid objects. `false` is simply the correct answer for
//  THIS unlinked stub, which has no grid state to query at all -- not a
//  behavioral fork from the real body, just a link-time stand-in.
// AI(W906-GA1-B2-integrate) 20260804: local HasICUnderMachine stub RETIRED (real body now in link group)
// AI(W906-GA1-B2-integrate) 20260804: local HasAnyICInMachine stub RETIRED (real body now in link group)

// ---------------------------------------------------------------------------
//  Tiny PASS / FAIL harness (same style as tests/test_serversocket.cpp).
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ===========================================================================
//  [1] GemTimer
// ===========================================================================
static void test_gemtimer()
{
    printf("\n[1] GemTimer\n");

    GemTimer t1;
    CHECK(t1.TimerOff() == false, "never-armed timer (ulStartTicks==0) -> TimerOff()==false");

    GemTimer t2;
    t2.TimerSetMSAndOn(0);   // iTimeLen<=0 guard (golden: "jou 2012-01-04" comment)
    CHECK(t2.TimerOff() == true, "TimerSetMSAndOn(0) -> immediately elapsed (iTimeLen<=0 guard)");

    GemTimer t3;
    t3.TimerSetMSAndOn(300);
    CHECK(t3.TimerOff() == false, "TimerSetMSAndOn(300) -> not yet elapsed immediately after arming");
    ::Sleep(450);
    CHECK(t3.TimerOff() == true, "TimerSetMSAndOn(300) -> elapsed after a 450ms real sleep");

    GemTimer t4;
    t4.TimerSetSecAndOn(0.2);   // 200ms
    CHECK(t4.TimerOff() == false, "TimerSetSecAndOn(0.2) -> not yet elapsed immediately");
    ::Sleep(350);
    CHECK(t4.TimerOff() == true, "TimerSetSecAndOn(0.2) -> elapsed after a 350ms real sleep");
}

// ===========================================================================
//  [2] vclcompat::TStringGrid -- headless shim basics
// ===========================================================================
static void test_stringgrid_basic()
{
    printf("\n[2] vclcompat::TStringGrid basics\n");

    vclcompat::TStringGrid g(3, 2);   // ColCount=3, RowCount=2
    CHECK((int)g.ColCount == 3, "ctor ColCount");
    CHECK((int)g.RowCount == 2, "ctor RowCount");
    CHECK(g.Cells[0][0] == "", "new cell defaults to \"\"");

    g.Cells[1][1] = "hello";
    CHECK(g.Cells[1][1] == "hello", "Cells[][] write/read round-trips");

    g.RowCount = 5;   // grow rows
    CHECK((int)g.RowCount == 5, "RowCount grows via assignment (property side effect)");
    CHECK(g.Cells[1][1] == "hello", "growing RowCount preserves existing content");
    CHECK(g.Cells[0][4] == "", "newly-added row is blank");

    g.ColCount = 1;   // shrink cols
    CHECK((int)g.ColCount == 1, "ColCount shrinks via assignment");
    CHECK(g.Cells[0][0] == "", "surviving column content preserved after shrink");

    bool threw = false;   // AI(W906-W3-6b) 20260925: recalibrated to BCB6 grids.pas ({$R-}, sparse Cells) -- the old "throws" expectation was the port's invention
    CHECK(g.Cells[5][0] == "", "read outside ColCount returns \"\" (BCB6 GetCells :5154-5160)");  g.ColCount = 3;  CHECK(g.Cells[1][1] == "hello", "shrink kept the data; re-grow shows it (ChangeSize never touches FData)");
    g.Cells[9][40] = "hidden";  CHECK((int)g.RowCount == 5 && (int)g.ColCount == 3 && g.Cells[9][40] == "hidden", "write outside the grid is stored, counts unchanged (SetCells :5180-5186)");
    try { (void)g.Cells[0][-1]; } catch (const std::out_of_range&) { threw = true; }  CHECK(threw, "negative index throws (BCB6 EListError, TSparseList.Get :4654-4658)");
}

// ===========================================================================
//  [3] SplitStrByTabOnly -- field-splitting edge cases
// ===========================================================================
static void test_split_str_by_tab_only()
{
    printf("\n[3] SplitStrByTabOnly\n");

    // Normal two-field line: "hello\tworld" -> "hello", "world", then false.
    {
        char buf[64]; std::strcpy(buf, "hello\tworld");
        char dest[64] = {0};
        bool r1 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r1 == true && std::string(dest) == "hello", "field 1 of \"hello\\tworld\" == \"hello\"");
        bool r2 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r2 == true && std::string(dest) == "world", "field 2 of \"hello\\tworld\" == \"world\"");
        bool r3 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r3 == false, "no field 3 -> returns false (buf now empty)");
    }

    // Adjacent delimiters (empty middle field): "a\t\tb" -> "a", "", "b", then false.
    // (Hand-traced against golden's exact algorithm -- see the .cpp's own
    // header comment on SplitStrByTabOnly for the full trace.)
    {
        char buf[64]; std::strcpy(buf, "a\t\tb");
        char dest[64] = {0};
        bool r1 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r1 == true && std::string(dest) == "a", "field 1 of \"a\\t\\tb\" == \"a\"");
        bool r2 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r2 == true && std::string(dest) == "", "field 2 of \"a\\t\\tb\" == \"\" (empty middle field)");
        bool r3 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r3 == true && std::string(dest) == "b", "field 3 of \"a\\t\\tb\" == \"b\"");
        bool r4 = SplitStrByTabOnly(buf, dest, sizeof(dest));
        CHECK(r4 == false, "no field 4 -> returns false");
    }
}

// ===========================================================================
//  [4] CopyStringGridAsTabFormat / PasteStringGridAsTabFormat round-trip
//  REAL ORACLE: these 5 lines are a byte-exact excerpt (all-ASCII, verified
//  via `python3 -c "...set(data)..."` against the golden Big5 corruption
//  risk -- zero non-ASCII bytes present) copied from the actual production
//  D:\HT9045\SECS\SECS\SYSTEM\AlarmData.def on the reference dev machine
//  (header row + rows 1-4 of 2739 real data rows), NOT fabricated numbers.
//  Embedded here (rather than read from that path at test run time) so this
//  test is portable to a machine that does not have that exact tree.
// ===========================================================================
static void test_copy_paste_roundtrip_real_oracle()
{
    printf("\n[4] CopyStringGridAsTabFormat / PasteStringGridAsTabFormat (real AlarmData.def excerpt)\n");

    static const char* kRealLines[5] = {
        "No.\tID_AlarmList\tAlarmCode\tUnitNo\tUnitName\tType\tMessage\tEnable\tAlarmID\tClass\tPosition\t",
        "1\t41\t41\t0\tEvent\t0\tUnknown Alarm Code\t1\t000000000\t0\tEvent\t",
        "2\t0101\tMES0101\t1\tInput Arm\t3\tDevice pick-up error on the tray\t1\t301000101\t3\tInput Arm\t",
        "3\t0102\tMES0102\t1\tInput Arm\t3\tAbnormal auto tray end. Please check devices count.\t1\t301000102\t3\tInput Arm\t",
        "4\t0109\tJAM0109\t1\tInput Arm\t1\tDevice pick-up error on Hot Plate\t1\t101000109\t1\tInput Arm\t",
    };

    THGem gem;
    // strGrdAlarm is ctor-sized 12 cols x 5 rows already (matches golden
    // .dfm ColCount=12 default-RowCount=5) -- exactly enough for these 5 lines.
    TStringList src;
    for (int i = 0; i < 5; ++i) src.Add(AnsiString(kRealLines[i]));

    gem.PasteStringGridAsTabFormat(gem.strGrdAlarm, &src);

    CHECK(gem.strGrdAlarm->Cells[2][0] == "AlarmCode", "header row: col2 == \"AlarmCode\"");
    CHECK(gem.strGrdAlarm->Cells[2][1] == "41",        "row1 (No.=1): AlarmCode == \"41\"");
    CHECK(gem.strGrdAlarm->Cells[2][2] == "MES0101",   "row2 (No.=2): AlarmCode == \"MES0101\"");
    CHECK(gem.strGrdAlarm->Cells[4][2] == "Input Arm", "row2: UnitName == \"Input Arm\"");
    CHECK(gem.strGrdAlarm->Cells[6][2] == "Device pick-up error on the tray", "row2: Message text matches real data");
    CHECK(gem.strGrdAlarm->Cells[7][2] == "1",         "row2: Enable == \"1\"");
    CHECK(gem.strGrdAlarm->Cells[8][2] == "301000101", "row2: AlarmID == \"301000101\"");
    CHECK(gem.strGrdAlarm->Cells[9][2] == "3",         "row2: Class == \"3\"");
    CHECK(gem.strGrdAlarm->Cells[6][3] == "Abnormal auto tray end. Please check devices count.", "row3: Message text matches real data");

    // Round-trip back to a TStringList and compare byte-for-byte against the
    // original 5 real lines (this grid has no 6th row, so no all-blank-row
    // "\t"-only quirk line to account for here).
    TStringList out;
    gem.CopyStringGridAsTabFormat(gem.strGrdAlarm, &out);
    CHECK(out.Count == 5, "round-trip: 5 lines out");
    bool allMatch = true;
    for (int i = 0; i < 5 && i < out.Count; ++i)
        if (!(out.GetString(i) == AnsiString(kRealLines[i]))) allMatch = false;
    CHECK(allMatch, "round-trip: all 5 lines byte-exact vs. the real AlarmData.def excerpt");

    // Grow by one row and confirm the documented "all-blank row copies as a
    // lone tab" quirk (matches the real file's own unused-row format).
    gem.strGrdAlarm->RowCount = 6;
    TStringList out2;
    gem.CopyStringGridAsTabFormat(gem.strGrdAlarm, &out2);
    CHECK(out2.Count == 6, "after RowCount=6: 6 lines out");
    CHECK(out2.GetString(5) == "\t", "blank row copies as a lone tab char (golden quirk, matches real on-disk empty rows)");
}

// ===========================================================================
//  [5] CEID family -- hand-seeded grid (SetCEIDContent NOT called -- see
//  file-head SAFETY NOTE). Cells are written directly, replicating exactly
//  what SetCEIDContent(iCeid=11, CeidAlias="MyAlias", iReportCount=2,
//  iReportIDData={100,101}, Mode=1) would have produced at row 1 (hand-
//  traced against the translated algorithm in SECSGEM/uHGemEquipment.cpp).
// ===========================================================================
static void test_ceid_family()
{
    printf("\n[5] CEID StringGrid family (hand-seeded, no SetCEIDContent call)\n");

    THGem gem;
    // Row 1 of strGrdCEID: CEID=11, Mode/Enable=1, Alias="MyAlias", ReportIDs 100,101.
    gem.strGrdCEID->Cells[0][1] = 11;
    gem.strGrdCEID->Cells[1][1] = 1;
    gem.strGrdCEID->Cells[2][1] = "MyAlias";
    gem.strGrdCEID->Cells[3][1] = 100;
    gem.strGrdCEID->Cells[4][1] = 101;

    CHECK(gem.CheckCeidExist("11") == true,  "CheckCeidExist(\"11\") true after seeding row 1");
    CHECK(gem.CheckCeidExist("999") == false, "CheckCeidExist(\"999\") false (not seeded)");
    CHECK(gem.CheckCEIDExist("11") == true,  "CheckCEIDExist (upper-case sibling) agrees with CheckCeidExist");

    unsigned outIDs[16] = {0};
    unsigned n = gem.GetCEIDContent(11, outIDs, 2);
    CHECK(n == 2, "GetCEIDContent(11) returns 2 report IDs");
    CHECK(outIDs[0] == 100 && outIDs[1] == 101, "GetCEIDContent(11) returns {100,101} in seeded order");

    CHECK(gem.IsEnableEvent(0, 11) == true, "IsEnableEvent(_,11) true (col1 seeded \"1\")");
    CHECK(gem.IsEnableEvent(0, 999) == false, "IsEnableEvent(_,999) false (CEID not present)");

    // Now seed stdGridReportID row 1: ReportID=100, Mode=2, SVIDs 501,502,503.
    gem.stdGridReportID->Cells[0][1] = 100;
    gem.stdGridReportID->Cells[1][1] = 2;
    gem.stdGridReportID->Cells[2][1] = 501;
    gem.stdGridReportID->Cells[3][1] = 502;
    gem.stdGridReportID->Cells[4][1] = 503;

    CHECK(gem.CheckReportIDExist("100") == true,  "CheckReportIDExist(\"100\") true");
    CHECK(gem.CheckReportIDExist("777") == false, "CheckReportIDExist(\"777\") false");
    CHECK(gem.GetReportIDType("100") == 2, "GetReportIDType(\"100\") == 2 (seeded Mode)");
    CHECK(gem.GetReportIDType("777") == 3, "GetReportIDType of an unknown ReportID == 3 (golden's not-found sentinel)");

    unsigned outSVIDs[16] = {0};
    unsigned m = gem.GetReportIDContent(100, outSVIDs, 0);
    CHECK(m == 3, "GetReportIDContent(100) returns 3 SVIDs");
    CHECK(outSVIDs[0] == 501 && outSVIDs[1] == 502 && outSVIDs[2] == 503, "GetReportIDContent(100) returns {501,502,503}");

    // CheckCeidAlreadyDefine(11): true only if one of CEID 11's linked
    // report IDs has GetReportIDType(...)==0 (Host-defined). Right now
    // ReportID "100" has Mode 2, so this should be false.
    CHECK(gem.CheckCeidAlreadyDefine("11") == false, "CheckCeidAlreadyDefine(11) false (linked ReportID 100 has Mode 2, not 0)");

    // Re-tag ReportID 100 as Mode 0 (Host-defined) and re-check.
    gem.stdGridReportID->Cells[1][1] = 0;
    CHECK(gem.CheckCeidAlreadyDefine("11") == true, "CheckCeidAlreadyDefine(11) true once linked ReportID 100 is Mode 0");
    CHECK(gem.CheckReportAlreadyDefine("100") == true, "CheckReportAlreadyDefine(\"100\") true (row exists in stdGridReportID)");

    // AddCeidReportID: append a 3rd report ID (200) into CEID 11's first
    // free slot (col 5, since cols 3,4 are already 100,101).
    gem.AddCeidReportID("11", "200");
    CHECK(gem.strGrdCEID->Cells[5][1] == "200", "AddCeidReportID appends into the first empty ReportID slot (col 5)");

    // DeleteHostReportIDOfAssignCeid(11): clears ALL of CEID 11's ReportID
    // slots (cols 3..ColCount-1), unconditionally (does not check Mode).
    gem.DeleteHostReportIDOfAssignCeid("11");
    CHECK(gem.strGrdCEID->Cells[3][1] == "" && gem.strGrdCEID->Cells[4][1] == "" && gem.strGrdCEID->Cells[5][1] == "",
          "DeleteHostReportIDOfAssignCeid(11) clears all of CEID 11's ReportID slots");
    CHECK(gem.strGrdCEID->Cells[0][1] == "11", "DeleteHostReportIDOfAssignCeid does not clear the CEID column itself");

    // Re-seed for DeleteAllHostDefineCeid (clears ReportID slots for EVERY
    // non-blank CEID row, via DeleteHostReportIDOfAssignCeid).
    gem.strGrdCEID->Cells[3][1] = 999;
    gem.strGrdCEID->Cells[0][2] = 22;
    gem.strGrdCEID->Cells[3][2] = 888;
    gem.DeleteAllHostDefineCeid();
    CHECK(gem.strGrdCEID->Cells[3][1] == "" && gem.strGrdCEID->Cells[3][2] == "",
          "DeleteAllHostDefineCeid clears ReportID slots on every non-blank CEID row");
}

// ===========================================================================
//  [6] Report-ID delete family -- DeleteReportID / DeleteReportIDOfCeid /
//  DeleteAllHostDefineReportID / DeleteAllHostReportIDOfCeid (all safe: none
//  call SaveEventReportData -- see file-head SAFETY NOTE).
// ===========================================================================
static void test_report_delete_family()
{
    printf("\n[6] Report-ID delete family\n");

    THGem gem;
    // 3 rows of stdGridReportID: {ReportID=10,Mode=0}, {20,Mode=1}, {30,Mode=0}
    gem.stdGridReportID->Cells[0][1] = 10; gem.stdGridReportID->Cells[1][1] = 0;
    gem.stdGridReportID->Cells[0][2] = 20; gem.stdGridReportID->Cells[1][2] = 1;
    gem.stdGridReportID->Cells[0][3] = 30; gem.stdGridReportID->Cells[1][3] = 0;

    // DeleteReportID(20, 1): row 2 matches (ReportID=20 AND Mode==1) -> the
    // golden algorithm shifts every LATER row up by one and blanks the tail.
    bool ok = gem.DeleteReportID(20, 1);
    CHECK(ok == true, "DeleteReportID(20,1) finds and removes the matching row");
    CHECK(gem.stdGridReportID->Cells[0][2] == "30", "row 2 now holds what was row 3 (ReportID=30), rows shifted up");
    CHECK(gem.stdGridReportID->Cells[0][3] == "", "last row is now blank after the shift");

    // DeleteReportID with a Mode that does not match any row -> false, no mutation.
    bool ok2 = gem.DeleteReportID(10, 5);
    CHECK(ok2 == false, "DeleteReportID(10,5) -- Mode 5 does not match ReportID 10's stored Mode(0) -> false");
    CHECK(gem.stdGridReportID->Cells[0][1] == "10", "no mutation when DeleteReportID finds nothing");

    // DeleteReportIDOfCeid: strGrdCEID row with CEID=5, ReportIDs {10,30,40}
    // at cols 3,4,5. Removing ReportID 30 should compact the remaining IDs
    // {10,40} leftward (cols 3,4), per golden's compaction algorithm.
    gem.strGrdCEID->Cells[0][1] = 5;
    gem.strGrdCEID->Cells[3][1] = 10;
    gem.strGrdCEID->Cells[4][1] = 30;
    gem.strGrdCEID->Cells[5][1] = 40;
    gem.DeleteReportIDOfCeid(30);
    CHECK(gem.strGrdCEID->Cells[3][1] == "10", "DeleteReportIDOfCeid(30): col3 still holds surviving ReportID 10");
    CHECK(gem.strGrdCEID->Cells[4][1] == "40", "DeleteReportIDOfCeid(30): col4 compacted to the next surviving ReportID 40");
    CHECK(gem.strGrdCEID->Cells[5][1] == "",   "DeleteReportIDOfCeid(30): trailing slot blanked after compaction");

    // DeleteAllHostReportIDOfCeid(40): should find & remove ReportID 40 from
    // EVERY strGrdCEID row that references it (here, just row 1), compacting.
    gem.DeleteAllHostReportIDOfCeid(40);
    CHECK(gem.strGrdCEID->Cells[3][1] == "10", "DeleteAllHostReportIDOfCeid(40): surviving ReportID 10 remains at col3");
    CHECK(gem.strGrdCEID->Cells[4][1] == "",   "DeleteAllHostReportIDOfCeid(40): ReportID 40 removed, no gap left behind");

    // DeleteAllHostDefineReportID: repeatedly finds a stdGridReportID row
    // with Mode=="0" (Host-defined) and fully deletes it (both from
    // stdGridReportID via DeleteReportID, and from any strGrdCEID row via
    // DeleteReportIDOfCeid), until none remain. After the row-1
    // (ReportID=20) deletion above, rows are: {10,Mode0}@row1, {30,Mode0}@row2.
    bool allDeleted = gem.DeleteAllHostDefineReportID();
    CHECK(allDeleted == true, "DeleteAllHostDefineReportID always returns true (golden: unconditional)");
    CHECK(gem.stdGridReportID->Cells[0][1] == "" && gem.stdGridReportID->Cells[0][2] == "",
          "DeleteAllHostDefineReportID removes every remaining Mode==0 row (10 and 30)");
}

// ===========================================================================
//  [7] ReadEventReportData -- REAL ORACLE, read-only against the actual
//  on-disk D:\HT9045\SECS\SECS\SYSTEM\EventReport_CEID.def /
//  EventReport_ReportID.def (safe: LoadFromFile only, never writes).
//  Degrades gracefully (skips, does not fail) if that exact path is absent
//  on the machine running this test.
// ===========================================================================
static void test_read_event_report_data_real_oracle()
{
    printf("\n[7] ReadEventReportData (real on-disk oracle, read-only)\n");

    const AnsiString kPath1 = "D:\\HT9045\\SECS\\SECS\\SYSTEM\\EventReport_CEID.def";
    if (!FileExists(kPath1))
    {
        printf("  SKIP: %s not present on this machine -- test is environment-dependent, not a failure.\n", kPath1.c_str());
        return;
    }

    THGem gem;
    gem.ReadEventReportData();

    // Real file's row 0 is the literal column-header text (confirmed via
    // `python3` byte-level inspection of the actual file, 2026-07-16):
    //   "CEID\tEnable\tAlias\tReportID\t"
    CHECK(gem.strGrdCEID->Cells[0][0] == "CEID",     "strGrdCEID header row col0 == \"CEID\" (real on-disk header)");
    CHECK(gem.strGrdCEID->Cells[1][0] == "Enable",   "strGrdCEID header row col1 == \"Enable\"");
    CHECK(gem.strGrdCEID->Cells[3][0] == "ReportID", "strGrdCEID header row col3 == \"ReportID\"");

    CHECK(gem.stdGridReportID->Cells[0][0] == "ReportID", "stdGridReportID header row col0 == \"ReportID\" (real on-disk header)");
    CHECK(gem.stdGridReportID->Cells[1][0] == "Type",     "stdGridReportID header row col1 == \"Type\"");
}

// ===========================================================================
//  [8] ReadAlamData / WriteAlamData -- GemSystemPath-derived path (NOT
//  hardcoded), pointed at a throwaway scratch folder. Never touches the real
//  D:\HT9045\SECS tree.
// ===========================================================================
static void test_alarm_read_write_roundtrip()
{
    printf("\n[8] ReadAlamData / WriteAlamData (scratch folder, not the real D:\\HT9045\\SECS tree)\n");

    const AnsiString kScratchDir = "uHGemEquipment_test_scratch";
    ForceDirectories(kScratchDir);

    THGem gemWriter;
    gemWriter.GemSystemPath = kScratchDir;
    gemWriter.strGrdAlarm->Cells[0][1] = 1;   // pre-populate cols 0-6 the way
    gemWriter.strGrdAlarm->Cells[1][1] = "41";// out-of-scope registration code
    gemWriter.strGrdAlarm->Cells[2][1] = "41";// would (SetAlamData only ever
    gemWriter.strGrdAlarm->Cells[6][1] = "Unknown Alarm Code"; // touches cols 7-10).
    gemWriter.SetAlamData(1, "000000000", "0", "unused-by-SetAlamData", "Event");
    CHECK(gemWriter.strGrdAlarm->Cells[7][1] == "1",          "SetAlamData writes col7 (Enable)");
    CHECK(gemWriter.strGrdAlarm->Cells[8][1] == "000000000",  "SetAlamData writes col8 (ALID)");
    CHECK(gemWriter.strGrdAlarm->Cells[9][1] == "0",          "SetAlamData writes col9 (Class)");
    CHECK(gemWriter.strGrdAlarm->Cells[10][1] == "Event",     "SetAlamData writes col10 (Position)");

    gemWriter.WriteAlamData();   // writes <scratch>/AlarmData.def

    const AnsiString kWrittenFile = kScratchDir + "\\AlarmData.def";
    CHECK(FileExists(kWrittenFile), "WriteAlamData created AlarmData.def under the scratch folder");

    // AI(W906-uHGemEquipment) 20260716: golden QUIRK, preserved verbatim and
    // documented at ReadAlamData's own definition -- it loads into
    // strGrdAlarmOld, NOT strGrdAlarm. Read it back with a FRESH THGem to
    // confirm this round-trips correctly into the "old" grid.
    THGem gemReader;
    gemReader.GemSystemPath = kScratchDir;
    // strGrdAlarmOld's .dfm leaves ColCount at the real-VCL design-time
    // default (5) -- ReadAlamData ITSELF only ever assigns RowCount (golden
    // uHGemEquipment.cpp:6247/6256), never ColCount, so a real THGem must
    // have something else (out-of-scope FormCreate-era setup, matching
    // strGrdAlarm's own ColCount=12) widen this grid before ReadAlamData is
    // meaningfully callable in production. Confirmed empirically: WITHOUT
    // this line, PasteStringGridAsTabFormat legitimately throws
    // std::out_of_range trying to write column 8+ of an 11-column real
    // AlarmData.def row into a 5-column grid -- not a translation bug, a
    // real precondition this test must satisfy itself (this wave's scope
    // does not include whatever out-of-scope code establishes it for real).
    gemReader.strGrdAlarmOld->ColCount = 12;
    gemReader.ReadAlamData();
    CHECK(gemReader.strGrdAlarmOld->Cells[8][1] == "000000000", "ReadAlamData round-trips ALID into strGrdAlarmOld col8");
    CHECK(gemReader.strGrdAlarmOld->Cells[9][1] == "0",         "ReadAlamData round-trips Class into strGrdAlarmOld col9");
    CHECK(gemReader.strGrdAlarmOld->Cells[10][1] == "Event",    "ReadAlamData round-trips Position into strGrdAlarmOld col10");
    // strGrdAlarm (the OTHER grid) must be untouched by ReadAlamData.
    CHECK(gemReader.strGrdAlarm->Cells[8][1] == "", "ReadAlamData does NOT touch strGrdAlarm (quirk confirmed: separate grid)");

    // Cleanup: remove the scratch file/folder this test created (best-effort;
    // not asserted, since a leftover harmless scratch file is not a test failure).
    DeleteFile(kWrittenFile);
    RemoveDir(kScratchDir);
}

// ===========================================================================
//  [9] Connect / DisConnect / IsConnect state transitions
// ===========================================================================
static void test_connect_lifecycle()
{
    printf("\n[9] Connect/DisConnect/IsConnect\n");

    THGem gem;
    CHECK(gem.IsConnect() == false, "fresh THGem: IsConnect()==false");

    gem.Connect();
    CHECK(gem.bStartConnect == true && gem.bAutoConnect == true, "Connect() sets bStartConnect/bAutoConnect");
    CHECK(gem.IsConnect() == false, "Connect() alone does not flip bConnect (DoConnect's job, out of this wave's scope)");

    // Guard: golden `if(bConnect==true) return;` -- Connect() while already
    // connected is a no-op (does not even touch bStartConnect/bAutoConnect).
    gem.bConnect = true;
    gem.bStartConnect = false;
    gem.bAutoConnect = false;
    gem.Connect();
    CHECK(gem.bStartConnect == false && gem.bAutoConnect == false,
          "Connect() while bConnect==true early-returns (golden guard) -- leaves bStartConnect/bAutoConnect untouched");

    gem.DisConnect();
    CHECK(gem.IsConnect() == false && gem.bStartConnect == false && gem.bAutoConnect == false,
          "DisConnect() clears bConnect/bStartConnect/bAutoConnect");
}

// ===========================================================================
//  [10] clientGemConnect / Disconnect / Error / Connecting handler behavior
// ===========================================================================
static void test_clientgem_handlers()
{
    printf("\n[10] clientGemConnect/Disconnect/Error/Connecting\n");

    THGem gem;

    gem.clientGemConnect(gem.clientGem, gem.clientGem->Socket);
    CHECK(gem.WaitShowString->Count > 0 && gem.WaitShowString->GetString(gem.WaitShowString->Count - 1) == "Connect",
          "clientGemConnect logs \"Connect\" via StringOut (WaitShowString)");
    CHECK(gem.LogDataString->GetString(gem.LogDataString->Count - 1) == "Connect",
          "clientGemConnect's StringOut also appends to LogDataString");

    gem.bConnect = true;
    gem.clientGemDisconnect(gem.clientGem, gem.clientGem->Socket);
    CHECK(gem.bConnect == false, "clientGemDisconnect clears bConnect");
    CHECK(gem.bAutoConnect == true, "clientGemDisconnect sets bAutoConnect when bConnect was true (golden: reconnect-on-drop)");

    gem.bConnect = false;
    gem.bAutoConnect = false;
    gem.clientGemDisconnect(gem.clientGem, gem.clientGem->Socket);
    CHECK(gem.bAutoConnect == false, "clientGemDisconnect leaves bAutoConnect false when bConnect was already false");

    int errCode = 123;
    gem.clientGem->Active = true;   // Sim mode: pretend already connected
    gem.clientGemError(gem.clientGem, gem.clientGem->Socket, Scktcomp::eeGeneral, errCode);
    CHECK(gem.clientGem->Active == false, "clientGemError forces clientGem->Active=false");
    CHECK(errCode == 0, "clientGemError zeroes the ErrorCode out-param");
    CHECK(gem.bTCPIP_Error == true, "clientGemError sets bTCPIP_Error");

    gem.clientGemConnecting(gem.clientGem, gem.clientGem->Socket);
    CHECK(gem.WaitShowString->GetString(gem.WaitShowString->Count - 1) == "connecting",
          "clientGemConnecting logs \"connecting\"");
}

// ===========================================================================
//  [11] DoOpenCommuncation -- passive (srvGem) role open-guard behavior.
//  SAFETY-CRITICAL golden behavior: srvGem->Open() is called ONLY when
//  srvGem->Active==false -- never a blind Close()+Open() (see the .cpp's own
//  citation of golden's "斷線重連的秘密" warning). Verified here by accepting a
//  Sim connection and confirming it SURVIVES a re-entry into the state
//  machine while already Active (a blind Close()+Open() would have dropped it).
// ===========================================================================
static void test_doopencommuncation_open_guard()
{
    printf("\n[11] DoOpenCommuncation -- passive (srvGem) open-guard behavior\n");

    THGem gem;
    gem.bUseClientSocket = false;
    gem.srvGem->Port = 6000;
    gem.bOpenCommuncation = true;

    CHECK(gem.iOpenCommuncationTask == 1, "fresh THGem starts at iOpenCommuncationTask==1");
    gem.DoOpenCommuncation();   // case 1: bOpenCommuncation==true -> arms 0.5s delay, Task=100
    CHECK(gem.iOpenCommuncationTask == 100, "case 1: bOpenCommuncation==true advances Task to 100");

    ::Sleep(600);   // let DelayOpenCommuncation's 0.5s arm elapse
    CHECK(gem.srvGem->Active == false, "srvGem starts inactive (Sim default)");
    gem.DoOpenCommuncation();   // case 100: timer elapsed, Port!=0, inactive -> Open() -> Task=200
    CHECK(gem.srvGem->Active == true, "DoOpenCommuncation opens srvGem when it was inactive");
    CHECK(gem.iOpenCommuncationTask == 200, "Task advances to 200 after a successful Open()");

    // Accept a Sim connection so a blind Close()+Open() would be OBSERVABLE
    // (it would drop this connection -- Close() always empties Connections[]).
    gem.srvGem->SimAcceptConnection("10.1.1.5", 4000);
    CHECK(gem.srvGem->Socket->ActiveConnections == 1, "Sim-accepted one connection");

    // Re-enter case 100 with srvGem ALREADY Active.
    gem.iOpenCommuncationTask = 100;
    gem.DoOpenCommuncation();
    CHECK(gem.srvGem->Active == true, "srvGem stays Active across a re-entry with Active==true already");
    CHECK(gem.srvGem->Socket->ActiveConnections == 1,
          "open-guard preserved: the Sim-accepted connection survives (no blind Close()+Open())");
}

// ===========================================================================
//  [12] srvGemClientConnect / Disconnect / Error state updates
// ===========================================================================
static void test_srvgemclient_handlers()
{
    printf("\n[12] srvGemClientConnect/Disconnect/Error\n");

    THGem gem;

    // srvGemClientConnect needs an accepted connection so Socket->LocalPort/
    // LocalAddress (the event's own Socket param) and srvGem->Socket->
    // Connections[i] (the loop) are both meaningful.
    TCustomWinSocket *conn = gem.srvGem->SimAcceptConnection("10.1.1.9", 5555);
    gem.srvGemClientConnect(gem.srvGem, conn);
    CHECK(gem.bServoSocketConnect == true, "srvGemClientConnect sets bServoSocketConnect");
    CHECK(gem.bReceiveMultiConnect == false, "srvGemClientConnect: bReceiveMultiConnect stays false with only 1 connection");

    TCustomWinSocket *conn2 = gem.srvGem->SimAcceptConnection("10.1.1.10", 5556);
    gem.srvGemClientConnect(gem.srvGem, conn2);
    CHECK(gem.bReceiveMultiConnect == true, "srvGemClientConnect sets bReceiveMultiConnect once ActiveConnections>1");

    gem.iOpenCommuncationTask = 999;
    gem.srvGemClientDisconnect(gem.srvGem, conn);
    CHECK(gem.bServoSocketConnect == false, "srvGemClientDisconnect clears bServoSocketConnect");
    CHECK(gem.iOpenCommuncationTask == 1,
          "srvGemClientDisconnect resets iOpenCommuncationTask to 1 (Timer1Task's own reset is deferred, see .cpp comment)");

    // srvGemClientError -- see this file's own top-of-file SAFETY NOTE: this
    // real call appends one line to D:\SECS_GEM_LOGS\... via SaveSECSGEMErrToLog.
    // AI(W906-uHGemEquipment-ConnLifecycle) 20260717: per explicit user
    // direction, using the real D:\SECS_GEM_LOGS path during dev/test is fine
    // -- what's NOT fine is leaving it dirtier after the test than before.
    // Capture the exact target file's pre-test content (computed with the
    // SAME yyyy/mm_dd/hh tokens SaveSECSGEMErrToLog itself uses) and restore
    // it verbatim afterward, so repeat ctest runs don't keep appending lines
    // to a real operational log forever.
    TDateTime tdNow = Now();
    AnsiString logDirYear = "D:\\SECS_GEM_LOGS\\" + FormatDateTime("yyyy", tdNow);
    AnsiString logDirDay = logDirYear + "\\" + FormatDateTime("mm_dd", tdNow);
    AnsiString logFilePath = logDirDay + "\\SECSGEM_ErrLog_" + FormatDateTime("hh", tdNow) + ".txt";
    bool logFileExistedBefore = FileExists(logFilePath);
    bool logDayDirExistedBefore = DirectoryExists(logDirDay);
    bool logYearDirExistedBefore = DirectoryExists(logDirYear);
    std::string logFileOriginalContent;
    if (logFileExistedBefore)
    {
        FILE *rf = fopen(logFilePath.c_str(), "rb");
        if (rf)
        {
            char buf[65536];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), rf)) > 0)
                logFileOriginalContent.append(buf, n);
            fclose(rf);
        }
    }

    int errCode = 42;
    gem.bServoSocketConnect = true;
    gem.srvGemClientError(gem.srvGem, conn, Scktcomp::eeGeneral, errCode);
    CHECK(gem.bServoSocketConnect == false, "srvGemClientError clears bServoSocketConnect");
    CHECK(errCode == 0, "srvGemClientError zeroes the ErrorCode out-param");
    CHECK(gem.bTCPIP_Error == true, "srvGemClientError sets bTCPIP_Error");

    // Restore D:\SECS_GEM_LOGS to exactly its pre-test state.
    if (logFileExistedBefore)
    {
        FILE *wf = fopen(logFilePath.c_str(), "wb");
        if (wf)
        {
            fwrite(logFileOriginalContent.data(), 1, logFileOriginalContent.size(), wf);
            fclose(wf);
        }
    }
    else
    {
        remove(logFilePath.c_str());
        if (!logDayDirExistedBefore)
            RemoveDir(logDirDay);
        if (!logYearDirExistedBefore)
            RemoveDir(logDirYear);
    }
}

// ===========================================================================
//  [13] CheckSocketActiveFalse
// ===========================================================================
static void test_check_socket_active_false()
{
    printf("\n[13] CheckSocketActiveFalse\n");

    THGem gem;

    gem.bUseClientSocket = true;
    CHECK(gem.clientGem->Active == false, "fresh clientGem starts inactive (Sim default)");
    CHECK(gem.CheckSocketActiveFalse() == true, "bUseClientSocket==true + clientGem inactive -> true");
    gem.clientGem->Active = true;
    CHECK(gem.CheckSocketActiveFalse() == false, "bUseClientSocket==true + clientGem active -> false");

    gem.bUseClientSocket = false;
    CHECK(gem.srvGem->Active == false, "fresh srvGem starts inactive (Sim default)");
    CHECK(gem.CheckSocketActiveFalse() == true, "bUseClientSocket==false + srvGem inactive -> true");
    gem.srvGem->Open();
    CHECK(gem.CheckSocketActiveFalse() == false, "bUseClientSocket==false + srvGem active -> false");
}

// ===========================================================================
//  [14] ClearDefaultEvenReport -- repeatedly deletes every Mode==1
//  (Handler-defined) stdGridReportID row, and scrubs strGrdCEID's references
//  to each deleted ReportID (via the already-tested DeleteReportID /
//  DeleteReportIDOfCeid). No SaveEventReportData call in its own body
//  (verified by reading its definition) -- safe to call directly.
// ===========================================================================
static void test_clear_default_even_report()
{
    printf("\n[14] ClearDefaultEvenReport\n");

    THGem gem;
    gem.stdGridReportID->Cells[0][1] = 10; gem.stdGridReportID->Cells[1][1] = 1;
    gem.stdGridReportID->Cells[0][2] = 20; gem.stdGridReportID->Cells[1][2] = 0;
    gem.stdGridReportID->Cells[0][3] = 30; gem.stdGridReportID->Cells[1][3] = 1;

    gem.strGrdCEID->Cells[0][1] = 5;
    gem.strGrdCEID->Cells[3][1] = 10;
    gem.strGrdCEID->Cells[4][1] = 30;

    gem.ClearDefaultEvenReport();

    CHECK(gem.stdGridReportID->Cells[0][1] == "20", "ClearDefaultEvenReport removes every Mode==1 row (10,30); surviving row (20) shifts up");
    CHECK(gem.stdGridReportID->Cells[0][2] == "", "no Mode==1 rows remain after ClearDefaultEvenReport");
    CHECK(gem.strGrdCEID->Cells[3][1] == "" && gem.strGrdCEID->Cells[4][1] == "",
          "ClearDefaultEvenReport also scrubs strGrdCEID's references to the deleted ReportIDs (10,30)");
}

// ===========================================================================
//  [15] GetTimeInfo -- TimeString/GemClock population (the in-scope half;
//  the disk-free-space/memory-status tail is a documented gated stub, see
//  the .cpp's own comment at GetTimeInfo's definition).
// ===========================================================================
static void test_get_time_info()
{
    printf("\n[15] GetTimeInfo -- TimeString/GemClock population\n");

    THGem gem;
    CHECK(gem.TimeString == "", "fresh THGem: TimeString starts empty");

    gem.GetTimeInfo();
    CHECK(gem.TimeString.Length() == 23, "TimeString matches \"yyyy-mm-dd hh:nn:ss.zzz\" length (23 chars)");
    CHECK(gem.TimeString[5] == '-' && gem.TimeString[8] == '-', "TimeString has '-' separators at the expected positions");

    CHECK(gem.iTimeFormat == 0, "iTimeFormat defaults to 0 (gate-stub default -- golden's own ctor never inits it either, see header note)");
    CHECK(gem.GemClock.Length() == 12, "default iTimeFormat -> GemClock is the 12-byte 'else' branch format");

    gem.iTimeFormat = 2;   // 14-byte format
    gem.GetTimeInfo();
    CHECK(gem.GemClock.Length() == 14, "iTimeFormat==2 -> GemClock is the 14-byte format");

    gem.iTimeFormat = 3;   // ISO8601-ish, 19 bytes
    gem.GetTimeInfo();
    CHECK(gem.GemClock.Length() == 19 && gem.GemClock[5] == '-' && gem.GemClock[11] == 'T',
          "iTimeFormat==3 -> GemClock is the 19-byte ISO8601-ish format with a literal 'T'");
}

// ===========================================================================
//  [16] OnLine family / DoOnLine / small setters (OnLine/OnLineLocal/
//  OnLineRemote/OffLine/IsOnLine/GetOnLineMode/SetEstablishCommunicationsTryCount/
//  SetCanAcceptHostOnLineRequest/CloseCommuncation)
// ===========================================================================
static void test_online_family_and_misc()
{
    printf("\n[16] OnLine family / DoOnLine / misc small setters\n");

    THGem gem;

    gem.OnLine(true);
    CHECK(gem.GetOnLineMode() == true, "OnLine(true) sets OnLineLocal mode");
    CHECK(gem.IsOnLine() == false, "OnLine(Mode) alone does not yet flip bOnLine (DoOnLine's job)");
    CHECK(gem.iStartOnLineTask == 1, "OnLine(Mode) arms iStartOnLineTask=1");

    bool r1 = gem.DoOnLine();   // case 1 -> Task=200
    CHECK(r1 == false && gem.iStartOnLineTask == 200, "DoOnLine case 1 advances Task to 200, returns false");
    bool r2 = gem.DoOnLine();   // case 200 -> OnlineLocalOrRemote(); bOnLine=true; return true
    CHECK(r2 == true && gem.IsOnLine() == true && gem.bStartOnLine == false,
          "DoOnLine case 200 completes: bOnLine=true, bStartOnLine=false, returns true");

    gem.OffLine();
    CHECK(gem.IsOnLine() == false && gem.bStartOnLine == false, "OffLine clears bOnLine/bStartOnLine");

    gem.OnLineRemote();
    CHECK(gem.GetOnLineMode() == false, "OnLineRemote sets OnLineLocal mode false");
    gem.OnLineLocal();
    CHECK(gem.GetOnLineMode() == true, "OnLineLocal sets OnLineLocal mode true");

    gem.SetEstablishCommunicationsTryCount(7);
    CHECK(gem.iEstablishCommunicationsTryCount == 7, "SetEstablishCommunicationsTryCount stores the value");

    gem.SetCanAcceptHostOnLineRequest(true);   // golden: truly empty body -- must not crash
    CHECK(true, "SetCanAcceptHostOnLineRequest (golden empty body) does not crash");

    gem.clientGem->Active = true;
    gem.bCloseCommuncation = false;
    gem.CloseCommuncation();
    CHECK(gem.bCloseCommuncation == true && gem.clientGem->Active == false,
          "CloseCommuncation sets bCloseCommuncation and forces clientGem->Active=false");
}

// ===========================================================================
//  [17] StringOut(1-arg) / StringBinaryOut -- SaveSECSGEMErrToLog is
//  deliberately NOT called directly here (hardcoded absolute path outside
//  the repo -- see this file's own SAVE-related SAFETY notes above and at
//  the top of this file); it IS exercised indirectly via srvGemClientError
//  in test [12] above.
// ===========================================================================
static void test_stringout_and_binaryout()
{
    printf("\n[17] StringOut(1-arg) / StringBinaryOut\n");

    THGem gem;
    int before = gem.WaitShowString->Count;
    gem.StringOut("hello");
    CHECK(gem.WaitShowString->Count == before + 1 && gem.WaitShowString->GetString(before) == "hello",
          "StringOut appends to WaitShowString");
    CHECK(gem.LogDataString->Count == before + 1 && gem.LogDataString->GetString(before) == "hello",
          "StringOut also appends to LogDataString");

    int logBefore = gem.LogDataString->Count;
    gem.StringBinaryOut("ignored");
    CHECK(gem.LogDataString->Count == logBefore, "StringBinaryOut (golden: fully commented-out body) is a true no-op");
}

// ===========================================================================
//  [18] InitialHGem / SaveSystemDefault round-trip -- scratch GemSystemIniPath
//  (NOT the real D:\HT9045\SECS tree), same discipline as test [8]'s
//  GemSystemPath. Runs under CUSTOMER_CODE==CC_MAXIM_THAILAND so
//  ckAddDefaultReport's ReadIniData round-trip branch is the one actually
//  exercised (golden forces ckAddDefaultReport->Checked=true unconditionally
//  for every OTHER customer -- see InitialHGem's own definition).
// ===========================================================================
static void test_initialhgem_savesystemdefault_roundtrip()
{
    printf("\n[18] InitialHGem / SaveSystemDefault (scratch ini path)\n");

    const AnsiString kScratchDir = "uHGemEquipment_test_scratch_ini";
    ForceDirectories(kScratchDir);
    const AnsiString kIniPath = kScratchDir + "\\GemTest.ini";
    DeleteFile(kIniPath);   // start clean, in case a prior run left it behind

    int savedCustomerCode = CUSTOMER_CODE;
    CUSTOMER_CODE = CC_MAXIM_THAILAND;

    THGem gemWriter;
    gemWriter.GemSystemIniPath = kIniPath;
    gemWriter.OnLineOrOffLine->Items.Count = 2;   // matches golden .dfm Items.Strings count
    gemWriter.RemoteOrLocal->Items.Count = 2;
    gemWriter.rgRole->Items.Count = 2;

    gemWriter.rgRole->ItemIndex = 1;   // [2] Active role
    gemWriter.edtIP->Text = "10.20.30.40";
    gemWriter.edtPort->Text = "5100";
    gemWriter.edDeviceID->Text = "7";
    gemWriter.edtT3TimeOut->Text = "45";
    gemWriter.GemCheckBoxAcceptHostOnlineRequest->Checked = false;
    gemWriter.ckAddDefaultReport->Checked = false;
    gemWriter.SaveSystemDefault();
    CHECK(FileExists(kIniPath), "SaveSystemDefault created the scratch ini file");

    THGem gemReader;
    gemReader.GemSystemIniPath = kIniPath;
    gemReader.OnLineOrOffLine->Items.Count = 2;
    gemReader.RemoteOrLocal->Items.Count = 2;
    gemReader.rgRole->Items.Count = 2;
    gemReader.InitialHGem();

    CHECK(gemReader.rgRole->ItemIndex == 1, "InitialHGem round-trips rgRole->ItemIndex (ActiveOrPassive)");
    CHECK(gemReader.bUseClientSocket == true, "InitialHGem derives bUseClientSocket from rgRole->ItemIndex==1");
    CHECK(gemReader.edtIP->Text == "10.20.30.40", "InitialHGem round-trips edtIP->Text (Address)");
    CHECK(gemReader.clientGem->Address == "10.20.30.40", "InitialHGem also propagates edtIP->Text into clientGem->Address");
    CHECK(gemReader.edtIP->Enabled == true, "InitialHGem: bUseClientSocket==true -> edtIP->Enabled=true");
    CHECK(gemReader.edDeviceID->Text == "7", "InitialHGem round-trips edDeviceID->Text (DeviceID)");
    CHECK(gemReader.T3TimeOut == 45, "InitialHGem round-trips T3TimeOut (re-parsed via atoi after ReadIniData)");
    CHECK(gemReader.GemCheckBoxAcceptHostOnlineRequest->Checked == false, "InitialHGem round-trips AcceptHostOnlineRequest");
    CHECK(gemReader.ckAddDefaultReport->Checked == false,
          "InitialHGem round-trips AddDefaultReport under CC_MAXIM_THAILAND (the one customer where it's actually read from ini)");

    CUSTOMER_CODE = savedCustomerCode;
    DeleteFile(kIniPath);
    RemoveDir(kScratchDir);
}

// ===========================================================================
//  [19] ManualCreatergRoleClick -- bShow gate + bUseClientSocket/edtIP->Enabled toggle
// ===========================================================================
static void test_manualcreatergroleclick()
{
    printf("\n[19] ManualCreatergRoleClick\n");

    THGem gem;
    gem.bShow = false;
    gem.rgRole->ItemIndex = 1;
    gem.bUseClientSocket = false;
    gem.ManualCreatergRoleClick(NULL);
    CHECK(gem.bUseClientSocket == false, "ManualCreatergRoleClick: bShow==false -> guarded no-op (golden's own bShow gate)");

    gem.bShow = true;
    gem.ManualCreatergRoleClick(NULL);
    CHECK(gem.bUseClientSocket == true, "ManualCreatergRoleClick: bShow==true, rgRole->ItemIndex==1 -> bUseClientSocket=true");
    CHECK(gem.edtIP->Enabled == true, "ManualCreatergRoleClick: bUseClientSocket==true -> edtIP->Enabled=true");

    gem.rgRole->ItemIndex = 0;
    gem.ManualCreatergRoleClick(NULL);
    CHECK(gem.bUseClientSocket == false, "ManualCreatergRoleClick: rgRole->ItemIndex==0 -> bUseClientSocket=false");
    CHECK(gem.edtIP->Enabled == false, "ManualCreatergRoleClick: bUseClientSocket==false -> edtIP->Enabled=false");
}

// ===========================================================================
//  [20] ProcessShow -- flushes WaitShowString into DB (via the global HGem
//  pointer, matching golden's own `HGem->` dereference -- see this file's
//  header comment on THGem::ProcessShow).
// ===========================================================================
static void test_processshow()
{
    printf("\n[20] ProcessShow\n");

    THGem gem;
    gem.DB = new THGemMemo();
    THGem *savedHGem = HGem;
    HGem = &gem;

    gem.WaitShowString->Add("line1");
    gem.WaitShowString->Add("line2");
    gem.ProcessShow();
    CHECK(gem.DB->Lines->Count == 2, "ProcessShow: DB->Lines receives the 2 flushed lines");
    CHECK(gem.DB->Lines->GetString(0) == "line1" && gem.DB->Lines->GetString(1) == "line2",
          "ProcessShow: DB->Lines content matches WaitShowString, in order");
    CHECK(gem.WaitShowString->Count == 0, "ProcessShow clears WaitShowString after flushing");

    gem.DB->Clear();
    gem.ProcessShow();   // WaitShowString now empty -> golden's own `if(Count!=0)` guard -> no-op
    CHECK(gem.DB->Lines->Count == 0, "ProcessShow: empty WaitShowString -> guarded no-op");

    HGem = savedHGem;
    delete gem.DB;
    gem.DB = NULL;
}

// ===========================================================================
//  [21] StringOut(AnsiString,TColor) 2-arg overload -- DB!=NULL guard, appends
//  to both DB->Lines and LogDataString, sets DB->SelStart. The `C` (color)
//  parameter is accepted but never actually used anywhere in golden's own
//  body (verified by reading it) -- not asserted on here for that reason.
// ===========================================================================
static void test_stringout_2arg()
{
    printf("\n[21] StringOut(AnsiString,TColor) 2-arg overload\n");

    THGem gem;
    int before = gem.LogDataString->Count;
    gem.StringOut("ignored-no-db", clRed);
    CHECK(gem.LogDataString->Count == before, "StringOut(2-arg): DB==NULL -> guarded no-op");

    gem.DB = new THGemMemo();
    gem.StringOut("hello-color", clLime);
    CHECK(gem.DB->Lines->Count == 1 && gem.DB->Lines->GetString(0) == "hello-color",
          "StringOut(2-arg): DB!=NULL -> appends to DB->Lines");
    CHECK(gem.LogDataString->GetString(gem.LogDataString->Count - 1) == "hello-color",
          "StringOut(2-arg) also appends to LogDataString");
    CHECK(gem.DB->SelStart == gem.DB->Lines->Count - 1, "StringOut(2-arg) sets DB->SelStart to the last line's index");

    delete gem.DB;
    gem.DB = NULL;
}

// ===========================================================================
//  [22] DoUpdateStatus -- throttle, normal Disconnected/Offline refresh +
//  EventReport(141), the client-socket-active + OnLine-Local transitions +
//  EventReport(92), and the KYEC 30-second forced-disconnect branch (now
//  exercised against the REAL elapsed-time HTimer, D6 -- see the assertion's
//  own updated comment below for exactly what changed vs the prior
//  always-fires stand-in).
// ===========================================================================
static void test_do_update_status()
{
    printf("\n[22] DoUpdateStatus\n");

    // AI(W906-FW-GEM-W9) 20260826: 本測試原本用 SECSGEM/SecsEventReport.h 的
    // g_SimLastEventReportCeid / g_SimEventReportCount 觀察 DoUpdateStatus 的
    // EventReport 呼叫。那組計數器是**自由函式** `EventReport(unsigned Ceid)`
    // 的，而 DoUpdateStatus 當時被寫成 `EventReport(141)`（單一參數）所以剛好
    // 打到它。golden 那四處全是 `EventReport(1, N)`（THGem 自己的 2 參數成員，
    // golden uHGemEquipment.cpp:4966/4974/4978/4982；golden 全樹掃
    // `EventReport\s*\(\s*\d+\s*\)` 是 0 個命中）。FW-GEM-W9 把成員翻進來
    // 並回復 golden 原文之後，那兩條斷言自然失效——**因為它們釘的是缺陷**。
    // 改成觀察真本體：THGem::EventReport 在 IsEnableEvent()==false 時走
    // disabled 分支，用 StringOut 往 LogDataString 記兩行（golden :7735-7738）。
    // 新斷言比舊的強：它同時證明了 iDataID 與 iCeid 兩個參數都傳對了。
    bool savedEnableSecsGem22 = IniConfig.bEnable_SECS_GEM;
    IniConfig.bEnable_SECS_GEM = true;   // 否則 EventReport 在第一道守衛就 return

    THGem gem;
    gem.SECSConnectionState = new THGemPanel();
    gem.GEMCommunicatingState = new THGemPanel();
    gem.GemPanelControlState = new THGemPanel();
    gem.BtnEnableComm = new THGemSpeedButton();
    gem.GemBtnOfflineRequest = new THGemSpeedButton();
    gem.GemBtnOnlineRequest = new THGemSpeedButton();
    gem.GemBtnOnlineRemote = new THGemSpeedButton();
    gem.GemBtnOnlineLocal = new THGemSpeedButton();

    // --- throttle: only every 10th call does real work ---------------------
    for (int i = 0; i < 9; ++i)
        gem.DoUpdateStatus();
    CHECK(gem.SECSConnectionState->Caption == "", "DoUpdateStatus: throttled (9 calls < 10) -- no refresh yet");

    // --- 10th call: passive (srvGem) role, disconnected, offline -----------
    // AI(W906-FW-GEM-W9) 20260826: 本檔不再觀察那組 Sim 計數器（見檔頭），
    // 這裡不再需要歸零。留 include 只是因為它同時宣告自由函式 EventReport，
    // 而 DoUpdateStatus 所在的函式庫需要那個宣告存在才編得過。
    gem.DoUpdateStatus();
    CHECK(gem.SECSConnectionState->Caption == "SECS GEM Disconnection" && gem.SECSConnectionState->Color == clRed,
          "DoUpdateStatus: srvGem inactive -> \"SECS GEM Disconnection\" / clRed");
    CHECK(gem.GEMCommunicatingState->Caption == "1:Disable" && gem.GEMCommunicatingState->Color == clRed,
          "DoUpdateStatus: bConnect==false, non-KYEC/SIGURD -> \"1:Disable\" / clRed");
    CHECK(gem.GemPanelControlState->Caption == "Off Line" && gem.GemPanelControlState->Color == clRed,
          "DoUpdateStatus: bOnLine==false -> \"Off Line\" / clRed");
    CHECK(gem.GemBtnOfflineRequest->Enabled == false && gem.GemBtnOnlineRequest->Enabled == true,
          "DoUpdateStatus: IsOnLine()==false -> Offline button disabled, Online button enabled");
    CHECK(gem.GemBtnOnlineRemote->Enabled == false && gem.GemBtnOnlineLocal->Enabled == true,
          "DoUpdateStatus: GetOnLineMode()==false -> Remote button disabled, Local button enabled");
    CHECK(gem.BtnEnableComm->Enabled == true, "DoUpdateStatus: !IsConnect() -> BtnEnableComm enabled");
    // golden :4966 `EventReport(1, 141);` -- 新建的 THGem 其 strGrdCEID 是空的，
    // 所以 IsEnableEvent(1,141) 回 false，走 golden :7734-7739 的 disabled 分支：
    //   StringOut("[Send]    "+TimeString);
    //   StringOut("Event Report(6,11) , DataID=%d , CEID=%d be disabled , abort send !!!");
    CHECK(gem.LogDataString->Count == 2 &&
          AnsiString(gem.LogDataString->Strings[1]).AnsiPos("DataID=1") != 0 &&
          AnsiString(gem.LogDataString->Strings[1]).AnsiPos("CEID=141") != 0,
          "DoUpdateStatus: GemControlState 0->1 fires THGem::EventReport(1, 141) -- "
          "disabled-event log line carries BOTH arguments (golden :4966 + :7737)");

    // --- switch to client-socket role, becomes Active -----------------------
    gem.bUseClientSocket = true;
    gem.clientGem->Active = true;
    gem.ctUpdateStatus = 9;
    gem.DoUpdateStatus();
    CHECK(gem.SECSConnectionState->Caption == "SECS GEM Connection" && gem.SECSConnectionState->Color == clLime,
          "DoUpdateStatus: clientGem becomes Active -> \"SECS GEM Connection\" / clLime");
    CHECK(gem.bConnect == false,
          "DoUpdateStatus: golden quirk preserved verbatim -- bConnect forced false even while the client socket is Active");

    // --- go OnLine (Local): GemControlState 1->2, EventReport(141) then (92) ---
    gem.bOnLine = true;
    gem.bOnLineLocal = true;
    gem.ctUpdateStatus = 9;
    gem.DoUpdateStatus();
    CHECK(gem.GemPanelControlState->Caption == "On Line Local" && gem.GemPanelControlState->Color == clLime,
          "DoUpdateStatus: bOnLine+bOnLineLocal -> \"On Line Local\" / clLime");
    CHECK(gem.GemBtnOfflineRequest->Enabled == true && gem.GemBtnOnlineRequest->Enabled == false,
          "DoUpdateStatus: IsOnLine()==true -> Offline button enabled, Online button disabled");
    CHECK(gem.GemBtnOnlineRemote->Enabled == true && gem.GemBtnOnlineLocal->Enabled == false,
          "DoUpdateStatus: GetOnLineMode()==true -> Remote button enabled, Local button disabled");
    // 累計 7 行（實測，非估算）：
    //   [0][1] 第一次 DoUpdateStatus 的 EventReport(1, 141)
    //   [2]    中間那次 DoUpdateStatus 自己記的 "Connect"（不是 EventReport 的）
    //   [3][4] 本次 golden :4966 的 EventReport(1, 141)
    //   [5][6] 本次 golden :4978 的 EventReport(1, 92)
    // 三次 EventReport 的 DataID 都是 1，CEID 依序 141/141/92——與舊斷言用
    // Sim 計數器數到的「累計 3 次」一致，只是現在數的是真本體的輸出。
    CHECK(gem.LogDataString->Count == 7 &&
          AnsiString(gem.LogDataString->Strings[6]).AnsiPos("DataID=1") != 0 &&
          AnsiString(gem.LogDataString->Strings[6]).AnsiPos("CEID=92") != 0,
          "DoUpdateStatus: GemControlState 1->2 fires THGem::EventReport(1, 141) then "
          "(1, 92) -- cumulative 7 log lines, last one carries DataID=1 + CEID=92 (golden :4966/:4978)");

    // --- KYEC 30-second forced-disconnect branch (STRUCTURAL ONLY) ---------
    // Needs 2 consecutive primed calls to observe for real: call A settles
    // GEMCommunicatingState->Caption to "1:OffLine" (the KYEC guard inside
    // THIS SAME call still reads the STALE pre-call Caption, so it does not
    // yet fire); call B then sees the now-settled "1:OffLine" and the guard
    // fires.
    int savedCustomerCode = CUSTOMER_CODE;
    THGem *savedHGem = HGem;
    CUSTOMER_CODE = CC_KYEC_LEE;
    HGem = &gem;

    gem.flag2UpdateStatus = true;   // force the GEMCommunicatingState block to re-evaluate this call
    gem.ctUpdateStatus = 9;
    gem.DoUpdateStatus();   // call A
    CHECK(gem.GEMCommunicatingState->Caption == "1:OffLine",
          "DoUpdateStatus (KYEC): bConnect==false -> \"1:OffLine\" (not \"1:Disable\") once CUSTOMER_CODE==CC_KYEC_LEE");

    gem.ctUpdateStatus = 9;
    gem.DoUpdateStatus();   // call B: KYEC guard now sees "SECS GEM Connection" + "1:OffLine" -> ARMS SECSGEM_DoSeparate.SetSecAndOn(30)
    // AI(W906-uHGemEquipment-BucketC) 20260717: UPDATED -- SECSGEM_DoSeparate
    // is now the REAL elapsed-time HTimer (D6), so arming it with
    // SetSecAndOn(30) inside call B and polling Off() a few microseconds
    // later (same call) correctly returns false -- the forced-disconnect
    // does NOT fire immediately anymore (this is the FIX for the prior
    // Bucket-B FLAGGED LIMITATION, where an always-fires stub made this
    // branch trigger on the very same poll it was armed). clientGem->Active
    // therefore stays true. Real elapsed-time semantics for THIS SAME HTimer
    // type are separately verified for real in test [T2] below (which can
    // afford a short real Sleep(); this 30-second golden constant cannot).
    CHECK(gem.clientGem->Active == true,
          "DoUpdateStatus (KYEC): forced-disconnect branch arms a REAL 30-second HTimer (D6) -- "
          "does NOT fire on the same poll it was armed (clientGem->Active stays true)");

    CUSTOMER_CODE = savedCustomerCode;
    HGem = savedHGem;

    delete gem.SECSConnectionState;
    delete gem.GEMCommunicatingState;
    delete gem.GemPanelControlState;
    delete gem.BtnEnableComm;
    delete gem.GemBtnOfflineRequest;
    delete gem.GemBtnOnlineRequest;
    delete gem.GemBtnOnlineRemote;
    delete gem.GemBtnOnlineLocal;
    IniConfig.bEnable_SECS_GEM = savedEnableSecsGem22;
}

// =============================================================================
//  Bucket C (W906-uHGemEquipment-BucketC 20260717): socket receive pump / T3
//  timeout / HSMS control-message handshake / Timer1Timer master state
//  machine. Golden reference: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/
//  uHGemEquipment.cpp (clientGemRead :9008-9028; ProcessSocketReceiveData
//  :9030-9207; Timer1Timer :5176-5527; DoConnect :3536-3599; DoSelect
//  :3496-3509; DoSeparate :3518-3531; DoProcessSFNoResponse :4604-4694;
//  SelectRsp/DeselectRsp/LinktestRsp :8707-8765; ProcessReceiceData
//  :8772-8989; CheckSFCodeResponse :7020-7058; SendLocalData :1985-2107).
//
//  *** SIDE-EFFECT WARNING (read before extending) ***
//  Every test below that pumps ProcessSocketReceiveData (directly or via
//  Timer1Timer) reaches SaveSECSGEMTextToLog(), which -- like the
//  already-committed SaveSECSGEMErrToLog -- unconditionally APPENDS to
//  D:\SECS_GEM_LOGS\<yyyy>\<mm_dd>\SECSGEM_TextLog_<hh>.txt (creating the
//  directory tree if absent). Same acceptable-risk posture as test [12]'s own
//  SAFETY NOTE (append-only diagnostic text, not destructive to existing
//  data) -- NOT separately byte-identity-guarded per test here (that would
//  require the same capture/restore dance at every single call site below);
//  instead this translation agent's own build+verify pass performs ONE
//  before/after byte-identity check across the ENTIRE test run (see the
//  final report) covering both SECSGEM_ErrLog_<hh>.txt (test [12]) and
//  SECSGEM_TextLog_<hh>.txt (all of Bucket C).
// =============================================================================

// ---------------------------------------------------------------------------
//  BucketCPanels -- RAII helper: the 3 panels + 5 speed buttons DoUpdateStatus
//  unconditionally dereferences (golden does NOT null-guard the 5
//  TSpeedButton*s -- see uHGemEquipment.h's own THGemSpeedButton comment).
//  Needed by any test that calls Timer1Timer() (which calls DoUpdateStatus()
//  on every pump) -- test [22] above wires the identical set inline; this
//  factors it out for the several Bucket-C tests that need the same thing.
// ---------------------------------------------------------------------------
namespace {
struct BucketCPanels
{
    THGem &g;
    explicit BucketCPanels(THGem &gem) : g(gem)
    {
        g.SECSConnectionState = new THGemPanel();
        g.GEMCommunicatingState = new THGemPanel();
        g.GemPanelControlState = new THGemPanel();
        g.BtnEnableComm = new THGemSpeedButton();
        g.GemBtnOfflineRequest = new THGemSpeedButton();
        g.GemBtnOnlineRequest = new THGemSpeedButton();
        g.GemBtnOnlineRemote = new THGemSpeedButton();
        g.GemBtnOnlineLocal = new THGemSpeedButton();
    }
    ~BucketCPanels()
    {
        delete g.SECSConnectionState; g.SECSConnectionState = NULL;
        delete g.GEMCommunicatingState; g.GEMCommunicatingState = NULL;
        delete g.GemPanelControlState; g.GemPanelControlState = NULL;
        delete g.BtnEnableComm; g.BtnEnableComm = NULL;
        delete g.GemBtnOfflineRequest; g.GemBtnOfflineRequest = NULL;
        delete g.GemBtnOnlineRequest; g.GemBtnOnlineRequest = NULL;
        delete g.GemBtnOnlineRemote; g.GemBtnOnlineRemote = NULL;
        delete g.GemBtnOnlineLocal; g.GemBtnOnlineLocal = NULL;
    }
    BucketCPanels(const BucketCPanels&) = delete;
    BucketCPanels& operator=(const BucketCPanels&) = delete;
};
} // anonymous namespace

// ===========================================================================
//  [T1] vclcompat::TMemoryStream shim -- write/read/seek/clear/LoadFromStream.
// ===========================================================================
static void test_memorystream_shim()
{
    printf("\n[T1] vclcompat::TMemoryStream shim\n");

    TMemoryStream ms;
    CHECK((int)ms.Size == 0, "fresh TMemoryStream: Size==0");
    CHECK(ms.Position == 0, "fresh TMemoryStream: Position==0");

    const char data1[] = "Hello";
    ms.WriteBuffer(data1, 5);
    CHECK((int)ms.Size == 5, "WriteBuffer(5 bytes) at Position 0 -> Size==5");
    CHECK(ms.Position == 5, "WriteBuffer advances Position by Count");

    // WriteBuffer at Position 0 with a SHORTER write must KEEP the longer
    // existing tail (real VCL TStream::WriteBuffer semantics) -- only Clear()
    // resets Size, not a short overwrite.
    ms.Position = 0;
    const char data2[] = "Wr";
    ms.WriteBuffer(data2, 2);
    CHECK((int)ms.Size == 5, "WriteBuffer at Position 0 with a SHORTER write keeps the longer existing tail (Size still 5)");
    char readback[8] = {0};
    ms.Position = 0;
    ms.ReadBuffer(readback, 5);
    CHECK(std::string(readback, 5) == "Wrllo", "content is \"Wr\"+\"llo\" (tail beyond the short write survived)");

    // Seek(0, soFromEnd) append pattern.
    ms.Seek(0, soFromEnd);
    CHECK(ms.Position == 5, "Seek(0, soFromEnd) -> Position==Size");
    const char data3[] = "!!";
    ms.WriteBuffer(data3, 2);
    CHECK((int)ms.Size == 7, "append via Seek(0,soFromEnd)+WriteBuffer grows Size");

    ms.Seek(0, soFromBeginning);
    CHECK(ms.Position == 0, "Seek(0, soFromBeginning) -> Position==0");
    char full[16] = {0};
    ms.ReadBuffer(full, 7);
    CHECK(std::string(full, 7) == "Wrllo!!", "full round-trip content matches expected");

    ms.Clear();
    CHECK((int)ms.Size == 0 && ms.Position == 0, "Clear() resets Size and Position to 0");

    // LoadFromStream: replaces dst's content wholesale, rewinds Source to 0.
    TMemoryStream src;
    const char data4[] = "abcdef";
    src.WriteBuffer(data4, 6);
    TMemoryStream dst;
    const char preexisting[] = "XXXXXXXXXX";
    dst.WriteBuffer(preexisting, 10);
    dst.LoadFromStream(&src);
    CHECK((int)dst.Size == 6, "LoadFromStream replaces dst's content with src's (Size==6, not 10)");
    CHECK(dst.Position == 0, "LoadFromStream leaves dst->Position at 0");
    CHECK(src.Position == 0, "LoadFromStream rewinds Source to 0 as a side effect");
    char dstRead[8] = {0};
    dst.ReadBuffer(dstRead, 6);
    CHECK(std::string(dstRead, 6) == "abcdef", "dst content matches src's content byte-for-byte");

    // ReadBuffer shortfall -> tripwire throw.
    // NOTE: the obvious variable name `small` collides with a Windows
    // <rpcndr.h> macro (`small` -> `char`, an old MIDL basic-type alias
    // pulled in transitively via windows.h) -- named `tinyStream` instead.
    bool threw = false;
    TMemoryStream tinyStream;
    const char tiny[] = "ab";
    tinyStream.WriteBuffer(tiny, 2);
    tinyStream.Position = 0;
    char buf3[8];
    try { tinyStream.ReadBuffer(buf3, 3); }
    catch (const std::runtime_error&) { threw = true; }
    CHECK(threw, "ReadBuffer shortfall throws std::runtime_error (EReadError tripwire)");
}

// ===========================================================================
//  [T2] HTimer real elapsed-time semantics.
//
//  DEVIATION (flagged prominently, per this wave's own instructions): the
//  REAL HTimer (D6, uHGemEquipment.cpp) is DELIBERATELY TU-local (anonymous
//  namespace -- see that file's own comment on why: ODR-safety vs.
//  atester_shims.h's own separate HTimer stub). It therefore has INTERNAL
//  linkage and its type is not nameable from this SEPARATE translation unit
//  -- `HTimer t;` cannot compile here. This test instead instantiates a
//  byte-for-byte MIRROR of that exact struct (the SAME field list and
//  Off()/SetSecAndOn() bodies, ported from the SAME golden
//  D:\HT9045\elec\Component\htimer.cpp this wave's real HTimer itself was
//  ported from) to verify the ALGORITHM directly with real short sleeps.
//  This is NOT the same as testing the literal TU-local symbol -- if
//  uHGemEquipment.cpp's own HTimer is ever edited, this mirror must be
//  updated to match by hand; there is no compiler enforcement linking the
//  two. The REAL TU-local instances' actual wiring/behavior is separately
//  verified INDIRECTLY: test [22]'s KYEC branch (arms
//  SECSGEM_DoSeparate.SetSecAndOn(30), confirms it does NOT fire on the same
//  poll) and T10 below (SECSGEM_DoSeparateWait's 5-second window blocking
//  Timer1Timer's pump, sleepless variant). A future wave should consider
//  promoting a single shared vclcompat/HTimer (already flagged as a risk in
//  the design brief) to close this testability gap for good.
// ===========================================================================
namespace {
struct HTimerMirror
{
    DWORD ulStartTicks;
    int   iTimeLen;
    bool  Paused;
    bool  InUsed;

    HTimerMirror() : ulStartTicks(0), iTimeLen(0), Paused(false), InUsed(false) {}
    void SetSec(double iTime) { iTimeLen = static_cast<int>(iTime * 1000.0); }
    void On() { ulStartTicks = ::GetTickCount(); InUsed = true; }
    void SetSecAndOn(double iTime)
    {
        ulStartTicks = 0; iTimeLen = 0; InUsed = false; Paused = false;
        SetSec(iTime);
        On();
    }
    bool Off()
    {
        if (Paused) return false;
        if (ulStartTicks == 0) return false;
        if (iTimeLen <= 0) return true;
        DWORD ulLimited = ulStartTicks + static_cast<DWORD>(iTimeLen);
        DWORD ulNowTicks = ::GetTickCount();
        DWORD ulNowTicksOver = 0;
        if (ulLimited < ulStartTicks)
        {
            ulLimited = 0xFFFFFFFF - ulStartTicks + static_cast<DWORD>(iTimeLen);
            ulNowTicksOver = 0xFFFFFFFF - ulStartTicks + ulNowTicks;
            if (ulNowTicksOver < ulStartTicks && ulNowTicksOver > ulLimited) { InUsed = false; return true; }
        }
        else
        {
            if (ulNowTicks >= (ulStartTicks + static_cast<DWORD>(iTimeLen))) { InUsed = false; return true; }
        }
        return false;
    }
};
} // anonymous namespace

static void test_htimer_real_semantics()
{
    printf("\n[T2] HTimer real elapsed-time semantics (mirror -- see this test's own DEVIATION comment above)\n");

    HTimerMirror t1;
    CHECK(t1.Off() == false, "never-armed HTimer -> Off()==false");

    HTimerMirror t2;
    t2.SetSecAndOn(0.2);   // 200ms
    CHECK(t2.Off() == false, "SetSecAndOn(0.2) -> not yet elapsed immediately");
    ::Sleep(250);
    CHECK(t2.Off() == true, "SetSecAndOn(0.2) -> elapsed after a 250ms real sleep");
    CHECK(t2.Off() == true, "re-poll after expiry stays true (InUsed=false does not change Off()'s return)");
}

// ===========================================================================
//  [T3] Select handshake e2e (server role, Sim): clientGemRead (wired in the
//  ctor, D... this wave) -> ProcessSocketReceiveData -> ProcessReceiceData
//  (control head, real) -> SelectRsp -> SendLocalDataFrom -> real bytes on
//  the wire, exercised through a full Timer1Timer pump.
// ===========================================================================
static void test_select_handshake_e2e()
{
    printf("\n[T3] Select handshake e2e (server role, Sim, via Timer1Timer)\n");

    bool savedInitialOK = InitialOK;
    bool savedEnableSecsGem = IniConfig.bEnable_SECS_GEM;
    THGem *savedHGem = HGem;

    THGem g;
    BucketCPanels panels(g);
    HGem = &g;
    g.DB = new THGemMemo();
    g.OnLineOrOffLine->Items.Count = 2;
    g.RemoteOrLocal->Items.Count = 2;
    g.rgRole->Items.Count = 2;
    const AnsiString kScratchDir = "uHGemEquipment_test_scratch_t3";
    ForceDirectories(kScratchDir);
    g.GemSystemIniPath = kScratchDir + "\\Gem.ini";
    InitialOK = true;
    IniConfig.bEnable_SECS_GEM = true;

    g.Timer1Timer(NULL);   // pump #1: Timer1Task 1->10 (case 1 is unconditional); InitialHGem() runs (bFirstEntry)
    CHECK(g.Timer1Task == 10, "T3: after pump #1, Timer1Task advances 1->10");
    CHECK(g.bUseClientSocket == false, "T3: server (passive) role -- rgRole->ItemIndex defaults to 0");

    g.srvGem->Open();
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.5.5.5", 4001);
    CHECK(g.bServoSocketConnect == true, "T3: SimAcceptConnection fires srvGemClientConnect -> bServoSocketConnect=true");

    // Frame layout re-verified against SecsWireCodec::ProcessRemoteHead
    // (SecsWireCodec.cpp:1383-1398): [0..3]=length(=10, big-endian) [4..5]=
    // DeviceID(0xFFFF) [6]=MessageID_S|W_Bit(0x00) [7]=MessageID_F(0x00)
    // [8]=PType(0x00) [9]=SType(0x01=Select.req) [10..13]=SystemByte(=1).
    unsigned char frame[14] = {
        0x00,0x00,0x00,0x0A, 0xFF,0xFF, 0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x01
    };
    conn->SimPushReceive(frame, 14);   // -> clientGemRead (wired) -> RecvMemoryBuffer

    g.Timer1Timer(NULL);   // pump #2: ProcessSocketReceiveData decodes + dispatches -> SelectRsp -> SendLocalDataFrom

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() == 14, "T3: SelectRsp's reply is a 14-byte HSMS frame (control message, no data items)");
    if (tx.size() == 14)
    {
        CHECK((unsigned char)tx[9] == 2, "T3: reply SType byte (offset 9) == 2 (Select.rsp)");
        unsigned echoedSystemByte = (static_cast<unsigned>((unsigned char)tx[10]) << 24) |
                                    (static_cast<unsigned>((unsigned char)tx[11]) << 16) |
                                    (static_cast<unsigned>((unsigned char)tx[12]) << 8)  |
                                     static_cast<unsigned>((unsigned char)tx[13]);
        CHECK(echoedSystemByte == 1, "T3: reply SystemByte echoes the request's SystemByte (1)");
    }
    CHECK((int)g.RecvMemoryBuffer->Size == 0, "T3: RecvMemoryBuffer drained after processing");
    CHECK((int)g.ProcBuffer->Size == 0, "T3: ProcBuffer cleared after a fully-consumed message");
    CHECK(g.bFirstBlock == true, "T3: bFirstBlock restored to true after a fully-consumed message");

    InitialOK = savedInitialOK;
    IniConfig.bEnable_SECS_GEM = savedEnableSecsGem;
    HGem = savedHGem;
    DeleteFile(g.GemSystemIniPath);
    RemoveDir(kScratchDir);
    delete g.DB; g.DB = NULL;
}

// ===========================================================================
//  [T4] Frame reassembly -- multiple messages in one read; a message split
//  across two reads (TempProcBuffer partial-frame path); a too-short
//  garbage read. Calls ProcessSocketReceiveData DIRECTLY (not via
//  Timer1Timer) -- T3 above already proved the full pump wiring; these
//  sub-tests focus on the reassembly bookkeeping itself.
// ===========================================================================
static void test_frame_reassembly()
{
    printf("\n[T4] Frame reassembly (multi-message / split-frame / garbage)\n");

    // (a) two concatenated Select.req frames in ONE SimPushReceive -> BOTH
    // get processed within a single ProcessSocketReceiveData call (its own
    // do-while walks every complete message still in the buffer).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.1", 4100);
        g.srvGem->Open();
        unsigned char frame1[14] = { 0,0,0,0x0A, 0xFF,0xFF, 0,0,0,0x01, 0,0,0,0x01 };   // SystemByte=1
        unsigned char frame2[14] = { 0,0,0,0x0A, 0xFF,0xFF, 0,0,0,0x01, 0,0,0,0x02 };   // SystemByte=2
        unsigned char both[28];
        std::memcpy(both, frame1, 14);
        std::memcpy(both + 14, frame2, 14);
        conn->SimPushReceive(both, 28);
        g.ProcessSocketReceiveData();

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 28, "T4(a): two concatenated Select.req frames -> two 14-byte Select.rsp replies (28 bytes total)");
        if (tx.size() == 28)
        {
            CHECK((unsigned char)tx[9] == 2 && (unsigned char)tx[13] == 1, "T4(a): reply #1 SType==2 (Select.rsp), echoes SystemByte==1");
            CHECK((unsigned char)tx[23] == 2 && (unsigned char)tx[27] == 2, "T4(a): reply #2 SType==2 (Select.rsp), echoes SystemByte==2");
        }
        CHECK((int)g.ProcBuffer->Size == 0, "T4(a): ProcBuffer fully drained after both messages consumed");
    }

    // (b) one frame split across two SimPushReceive calls -> the
    // TempProcBuffer partial-frame path (golden :9123-9136) buffers the
    // first chunk and waits; only the SECOND chunk completes the message.
    //
    // JUDGMENT CALL (flagged): a REAL, unpadded control message (Select.req)
    // is ALWAYS exactly 14 bytes (Value==10 -- CreateLocalHead's own
    // invariant; no DataItemOut call ever rides along a control message), so
    // there is no way to split ONE at a byte boundary that both (i) supplies
    // >=14 bytes up front (else the "<14 byte" ERROR path fires instead,
    // clearing the buffer outright rather than waiting) and (ii) still has
    // more to come. This test therefore uses an ARTIFICIALLY padded
    // Select.req (declared Value=11 instead of 10, with one extra trailing
    // byte) purely to give the frame a length beyond the 14-byte minimum --
    // NOT a real protocol shape, just the minimum change needed to make the
    // partial-frame path reachable at all with a hand-built buffer.
    // ProcessReceiceData's control-message head only ever reads Remote.SType
    // (a fixed-offset field within the first 14 bytes), so the trailing pad
    // byte does not affect dispatch -- SelectRsp() still fires for real once
    // reassembly completes.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.2", 4101);
        g.srvGem->Open();
        unsigned char frame[15] = { 0,0,0,0x0B, 0xFF,0xFF, 0,0,0,0x01, 0,0,0,0x03, 0x99 };   // Value=11 (padded), SystemByte=3

        conn->SimPushReceive(frame, 14);   // first 14 of 15 bytes -- 1 byte still missing
        g.ProcessSocketReceiveData();
        CHECK(conn->SimTxBuffer().size() == 0, "T4(b): no reply yet after only the first 14 of 15 bytes arrived");
        CHECK(g.bFirstBlock == false, "T4(b): bFirstBlock cleared while a partial frame is buffered");

        conn->SimPushReceive(frame + 14, 1);   // the missing trailing byte
        g.ProcessSocketReceiveData();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 14, "T4(b): reply appears ONLY after the second chunk completes the frame");
        if (tx.size() == 14)
            CHECK((unsigned char)tx[9] == 2 && (unsigned char)tx[13] == 3, "T4(b): reply is Select.rsp echoing SystemByte==3");
        CHECK(g.bFirstBlock == true, "T4(b): bFirstBlock restored true once the reassembled frame is fully consumed");
    }

    // (c) garbage <14 bytes -> golden's own hardcoded error string, buffers cleared.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.3", 4102);
        unsigned char garbage[5] = { 1,2,3,4,5 };
        conn->SimPushReceive(garbage, 5);
        g.ProcessSocketReceiveData();
        bool found = false;
        for (int i = 0; i < g.WaitShowString->Count; i++)
            if (g.WaitShowString->GetString(i) == "Err : Socket Buffer Length less than 14 byte ")
                found = true;
        CHECK(found, "T4(c): <14-byte garbage read logs golden's own hardcoded error string");
        CHECK((int)g.ProcBuffer->Size == 0, "T4(c): ProcBuffer cleared after the malformed-length error path");
        CHECK(conn->SimTxBuffer().size() == 0, "T4(c): no reply sent for malformed input");
    }
}

// ===========================================================================
//  [T5] Data message with null-guarded dispatcher (W906-SysModWire): a real,
//  syntactically-valid S1F14 (W=0) frame, built via a throwaway SecsWireCodec's
//  own real encoder (InitLocalHead/DataItemOut) rather than hand-encoded bytes.
//  AI(W906-SysModWire) 20260720: the tail is REAL now (was a gated `#if 0`
//  block) -- this test still runs with HSys.MyGem==NULL (never wired), so it
//  exercises the null-guard's early-skip path, not the dispatch itself. See
//  [W10] for this same semantics re-verified post-un-gating, and W1-W9 for
//  the WIRED (HSys.MyGem != NULL) behavior.
// ===========================================================================
static void test_data_message_gated_dispatch()
{
    printf("\n[T5] Data message (S1F14, W=0) -- null-guarded dispatcher (W906-SysModWire): no crash, no reply, SReceiveData decoded\n");

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.9", 4200);
    g.srvGem->Open();

    SecsWireCodec builder;
    builder.InitLocalHead(1, 14, 0);   // S1,F14, W=0 -- shape of golden's own [S1F14] Connect Request Acknowledge
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("OK"));
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

    bool threw = false;
    try { g.ProcessSocketReceiveData(); }
    catch (...) { threw = true; }
    CHECK(threw == false, "T5: data-message path does not throw (Ifor-20260421 try/catch never needed to fire)");

    CHECK(g.WireCodec.Remote.MessageID_S == 1 && g.WireCodec.Remote.MessageID_F == 14,
          "T5: WireCodec.Remote decodes MessageID_S/F == 1/14");
    CHECK(conn->SimTxBuffer().size() == 0, "T5: null-guarded S,F dispatch tail sends no reply (HSys.MyGem stays NULL)");
    CHECK(g.WireCodec.SReceiveData->Count > 0, "T5: ShowSML's real ProcessSML decode populated SReceiveData with tokens while bReceiveData was true");
    CHECK(g.WireCodec.bReceiveData == false, "T5: bReceiveData reset to false once the message is fully consumed");
}

// ===========================================================================
//  [T6] T3 send-side record: an outbound W=1 request records "S F+1
//  SystemByte" into SFCodeResponseList/TimeLeft (T3TimeOut=30 golden ctor
//  default -> TimeLeft "300").
// ===========================================================================
static void test_t3_send_side_record()
{
    printf("\n[T6] T3 send-side record (SFCodeResponseList/TimeLeft)\n");

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.10", 4300);
    g.srvGem->Open();
    CHECK(g.bServoSocketConnect == true, "T6: SimAcceptConnection sets bServoSocketConnect (fixture precondition)");

    g.WireCodec.InitLocalHead(1, 13, 1);   // S1,F13, W=1 -- a request expecting a reply -> T3 record armed
    g.SendLocalData();

    AnsiString expected = AnsiString(1) + " " + AnsiString(14) + " " + AnsiString(g.WireCodec.Local.SystemByte);
    int idx = g.SFCodeResponseList->Items->IndexOf(expected);
    CHECK(idx != -1, "T6: SendLocalData (W=1, odd F) records \"S F+1 SystemByte\" into SFCodeResponseList");
    CHECK(idx >= 0 && g.TimeLeft->GetString(idx) == "300",
          "T6: matching TimeLeft slot == \"300\" (T3TimeOut=30 golden ctor default * 10)");
    CHECK(conn->SimTxBuffer().size() > 0, "T6: SendLocalData put real bytes on the wire (srvGem path)");
}

// ===========================================================================
//  [T7] T3 timeout -> S9F9 through the cross-class seam: a REAL HTGem
//  instance's own WireCodec, hooked to forward through THGem's real send --
//  proving the SendLocalDataHook seam (D2) works end-to-end TODAY, without
//  waiting for the future SystemModularInitial wiring wave.
// ===========================================================================
static void test_t3_timeout_s9f9_cross_class_seam()
{
    printf("\n[T7] T3 timeout -> S9F9 through the cross-class seam (HTGem.WireCodec.SendLocalDataHook)\n");

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.11", 4400);
    g.srvGem->Open();

    HTGem hgem;
    HSys.MyGem = &hgem;
    hgem.WireCodec.SendLocalDataHook = [&g](SecsWireCodec &wc) { g.SendLocalDataFrom(wc); };

    // Seed one T3 record via the SAME send path T6 exercises.
    g.WireCodec.InitLocalHead(1, 13, 1);
    g.SendLocalData();
    AnsiString expected = AnsiString(1) + " " + AnsiString(14) + " " + AnsiString(g.WireCodec.Local.SystemByte);
    int seededIndex = g.SFCodeResponseList->Items->IndexOf(expected);
    CHECK(seededIndex != -1, "T7: fixture -- T3 record seeded (same pattern as T6)");

    conn->SimClearTx();
    g.TimeLeft->Strings[seededIndex] = "1";   // one tick from expiry
    g.SystemSec = 1;                          // != iOldSecProcessSFNoResponse's ctor default (0)

    g.DoProcessSFNoResponse();

    CHECK(g.SFCodeResponseList->Items->IndexOf(expected) == -1, "T7: expired T3 record removed from SFCodeResponseList");
    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() >= 14, "T7: DoProcessSFNoResponse's S9F9 send produced real bytes on the wire via the cross-class hook");
    if (tx.size() >= 14)
        CHECK((unsigned char)tx[6] == 0x09 && (unsigned char)tx[7] == 0x09, "T7: reply header is S9F9 (Transaction Timer Timeout)");
    std::string body(tx.begin() + 14, tx.end());
    CHECK(tx.size() == 26 && (unsigned char)body[0] == 0x21 && (unsigned char)body[1] == 10 && hgem.WireCodec.LogDataString->IndexOf(expected + "    T3   time out") != -1, "T7: S9F9 body is <B[10]> (golden 913 SendS9FxEchoHeader, AI(W906-W187) 20261009; 0618 sent the text); the \"T3   time out\" text is a log line");

    HSys.MyGem = NULL;
}

// ===========================================================================
//  [T8] CheckSFCodeResponse response-matching: exact match removes the
//  record; a mismatch leaves it untouched; a TimeLeft/SFCodeResponseList
//  COUNT mismatch clears both lists outright (golden's own recovery path).
// ===========================================================================
static void test_checksfcoderesponse()
{
    printf("\n[T8] CheckSFCodeResponse response-matching\n");

    THGem g;

    g.SFCodeResponseList->Items->Add("1 14 5");
    g.TimeLeft->Add("300");
    g.WireCodec.Remote.MessageID_S = 1;
    g.WireCodec.Remote.MessageID_F = 14;
    g.WireCodec.Remote.SystemByte = 5;
    g.CheckSFCodeResponse();
    CHECK(g.SFCodeResponseList->Items->Count == 0 && g.TimeLeft->Count == 0,
          "T8: exact-match response removes the ONE seeded record from both lists");

    g.SFCodeResponseList->Items->Add("1 14 5");
    g.TimeLeft->Add("300");
    g.WireCodec.Remote.SystemByte = 999;   // no longer matches "1 14 5"
    g.CheckSFCodeResponse();
    CHECK(g.SFCodeResponseList->Items->Count == 1 && g.TimeLeft->Count == 1,
          "T8: mismatched response leaves the seeded record untouched");

    g.TimeLeft->Add("150");   // now TimeLeft->Count(2) != SFCodeResponseList->Items->Count(1)
    g.CheckSFCodeResponse();
    CHECK(g.SFCodeResponseList->Items->Count == 0 && g.TimeLeft->Count == 0,
          "T8: TimeLeft/SFCodeResponseList count mismatch -> CheckSFCodeResponse clears BOTH lists");
}

// ===========================================================================
//  [T9] DoConnect state walk.
// ===========================================================================
static void test_doconnect_state_walk()
{
    printf("\n[T9] DoConnect state walk\n");

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.20", 4500);
    g.srvGem->Open();
    conn->SimClearTx();

    CHECK(g.iStartConnectTask == 1, "T9: fresh THGem starts at iStartConnectTask==1");
    int ret1 = g.DoConnect();   // case 1: DoSelect() sends Select.req, arms ConnectDelay, Task=100
    CHECK(ret1 == 0 && g.iStartConnectTask == 100, "T9: DoConnect case 1 sends Select.req and advances to Task 100");
    CHECK(conn->SimTxBuffer().size() == 14, "T9: DoSelect's Select.req produced a 14-byte control frame on the wire");
    if (conn->SimTxBuffer().size() == 14)
        CHECK((unsigned char)conn->SimTxBuffer()[9] == 1, "T9: sent frame's SType byte == 1 (Select.req)");

    g.bWaitSelectRsp = true;   // simulate a Select.rsp having arrived (bypassing a full T3-style e2e injection)
    int ret2 = g.DoConnect();   // case 100: bWaitSelectRsp==true -> S1F13 (D3 null-guarded, HSys.MyGem==NULL) -> Task=200
    CHECK(ret2 == 0 && g.iStartConnectTask == 200,
          "T9: DoConnect case 100 (bWaitSelectRsp) advances to Task 200 -- S1F13 null-guard did not crash with HSys.MyGem==NULL");
    // AI(W906-SysModWire) 20260720: this specific instance stays unwired
    // (HSys.MyGem never set here) -- the check above still holds (true, the
    // null-guard still exists for an unwired instance). See [W3] below for
    // the WIRED (HSys.MyGem != NULL) version of this exact same state walk,
    // where S1F13 is a real send and the full handshake completes end-to-end.

    g.bWaitEstablishCommunicationsResponse = true;
    g.bWaitEstablishCommunicationsResponseError = false;
    int ret3 = g.DoConnect();   // case 200: success -> bConnect=true, Task=1, return 1
    CHECK(ret3 == 1 && g.bConnect == true && g.iStartConnectTask == 1,
          "T9: DoConnect case 200 (no error) completes -- bConnect=true, Task resets to 1, returns 1");

    // ConnectDelay timeout -> return 2 branch.
    g.bConnect = false;
    int ret4 = g.DoConnect();   // case 1 again: re-arms ConnectDelay.TimerSetSecAndOn(3), Task=100
    CHECK(ret4 == 0 && g.iStartConnectTask == 100, "T9: DoConnect re-armed (case 1) for the ConnectDelay-timeout sub-test");
    ::Sleep(3100);
    int ret5 = g.DoConnect();   // case 100: bWaitSelectRsp==false, ConnectDelay elapsed -> Task=1, return 2
    CHECK(ret5 == 2 && g.iStartConnectTask == 1, "T9: ConnectDelay's 3-second timeout -> DoConnect returns 2, resets Task to 1");
}

// ===========================================================================
//  W906-SysModWire: SystemModularInitial wiring wave (W1-W10).
//
//  All of these tests wire `HSys.MyGem` for real via `HSys.SystemModularInitial()`
//  (SECSGEM/uHGemHT9045_Shim.h's thin `HT9045Gem` shim -- zero virtual
//  overrides, so every dispatched handler runs HTGem's own base behavior;
//  see that shim's own file-head note and the design brief's §5 behavior-
//  difference table for exactly which branches differ from golden
//  production, where the REAL HT9045Gem override layer would run instead).
//
//  None of these call Timer1Timer -- ProcessSocketReceiveData()/DoConnect()
//  are invoked DIRECTLY, deliberately bypassing the master state machine (T10
//  above must stay the LAST test in this binary to ever pump Timer1Timer, so
//  its own function-static locals stay at their process-start defaults for
//  T10's own assumptions -- see that test's own comment).
//
//  Each test does its own save/restore of `HGem` (global) and always tears
//  down with `delete HSys.MyGem; HSys.MyGem=NULL;` -- the identical shape
//  golden's own main.cpp:11480-11481 uses (untranslated in this port; tests
//  must do it themselves since SystemModularInitial's own production call
//  site, the SYSTEM_MODULAR ctor, is still gated).
// ===========================================================================

// ---------------------------------------------------------------------------
//  [W1] SystemModularInitial wires HSys.MyGem for real.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w1_systemmodularinitial_wiring()
{
    printf("\n[W906-SysModWire.W1] SystemModularInitial wires HSys.MyGem\n");

    THGem *savedHGem = HGem;

    // (a) wired: HGem set to a real THGem instance BEFORE SystemModularInitial.
    {
        THGem g;
        HGem = &g;
        HSys.SystemModularInitial();
        CHECK(HSys.MyGem != NULL, "W1(a): SystemModularInitial() sets HSys.MyGem != NULL");
        CHECK(HSys.MyGem->HGemPtr == &g, "W1(a): MyGem->HGemPtr == &g (the wired THGem instance)");
        CHECK(HSys.MyGem->ActiveWire == &g.WireCodec, "W1(a): MyGem->ActiveWire re-points at the wired instance's own WireCodec (design D)");
        CHECK(HSys.MyGem->HandlerPath == "HT9045", "W1(a): MyGem->HandlerPath == \"HT9045\" (golden database.cpp:1541's literal)");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (b) golden's own static-init reality: HGem is still NULL at the call
    // site (design brief §1.1 -- HSys is a global, its ctor/SystemModularInitial
    // run during static init, BEFORE the global THGem *HGem is ever assigned
    // in golden production). NULL-tolerant: ActiveWire stays at its own
    // default (&MyGem's own by-value WireCodec) -- must not crash.
    {
        HGem = NULL;
        HSys.SystemModularInitial();
        CHECK(HSys.MyGem != NULL, "W1(b): SystemModularInitial() sets HSys.MyGem != NULL even when HGem is NULL");
        CHECK(HSys.MyGem->HGemPtr == NULL, "W1(b): MyGem->HGemPtr == NULL (golden's static-init reality)");
        CHECK(HSys.MyGem->ActiveWire == &HSys.MyGem->WireCodec, "W1(b): ActiveWire defaults to MyGem's OWN WireCodec when HGemTmp==NULL");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W2] HSys.MyGem->S1F13_EstablishCommunicationsRequest() composes a real
//  S1,F13 frame carrying GemMDLN/GemSOFTREV.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w2_s1f13_frame()
{
    printf("\n[W906-SysModWire.W2] S1F13 frame composition (real send via ActiveWire/hook)\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.1", 4701);
    g.srvGem->Open();

    HGem = &g;
    HSys.SystemModularInitial();
    g.SetMachineTypeAndSoftwarseVer("HT9045", "V906");

    conn->SimClearTx();
    HSys.MyGem->S1F13_EstablishCommunicationsRequest();

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() > 0, "W2: S1F13_EstablishCommunicationsRequest produced real bytes on the wire");
    if (!tx.empty())
    {
        CHECK(((unsigned char)tx[6] & 0x7f) == 1, "W2: sent frame's MessageID_S == 1");
        CHECK((unsigned char)tx[7] == 13, "W2: sent frame's MessageID_F == 13");
    }
    std::string body(tx.begin(), tx.end());
    CHECK(body.find("HT9045") != std::string::npos, "W2: S1F13 payload carries GemMDLN (\"HT9045\")");
    CHECK(body.find("V906") != std::string::npos, "W2: S1F13 payload carries GemSOFTREV (\"V906\")");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W3] DoConnect full end-to-end handshake, wired (task range 3 acceptance).
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w3_doconnect_e2e()
{
    printf("\n[W906-SysModWire.W3] DoConnect full e2e handshake (wired -- task range 3 acceptance)\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.2", 4702);
    g.srvGem->Open();
    conn->SimClearTx();

    HGem = &g;
    HSys.SystemModularInitial();
    g.SetMachineTypeAndSoftwarseVer("HT9045", "V906");

    int r1 = g.DoConnect();   // case 1: DoSelect() sends Select.req, Task=100
    CHECK(r1 == 0 && g.iStartConnectTask == 100, "W3: DoConnect case 1 sends Select.req, Task->100");

    conn->SimClearTx();
    // Host's Select.rsp control frame (SType=2) -- same 14-byte control-frame
    // shape as T3's own Select.req fixture, SType swapped to Select.rsp.
    unsigned char selectRsp[14] = { 0,0,0,0x0A, 0xFF,0xFF, 0,0,0,0x02, 0,0,0,0x01 };
    conn->SimPushReceive(selectRsp, 14);
    g.ProcessSocketReceiveData();
    CHECK(g.bWaitSelectRsp == true, "W3: Select.rsp control frame sets bWaitSelectRsp");

    conn->SimClearTx();
    int r2 = g.DoConnect();   // case 100: bWaitSelectRsp==true -> REAL S1F13 send (HSys.MyGem wired) -> Task=200
    CHECK(r2 == 0 && g.iStartConnectTask == 200, "W3: DoConnect case 100 sends real S1F13 (no longer null-guard short-circuit), Task->200");
    CHECK(conn->SimTxBuffer().size() > 0, "W3: S1F13 produced real bytes on the wire");

    conn->SimClearTx();
    // Host's S1F14 SUCCESS reply. Process_S1F14_ConnectRequestAcknowledge's
    // own parser (golden uHGemClass.cpp:372-388) expects EXACTLY
    // L2{ B[1]=0, L[0] } -- NOT L2{B, L2{MDLN,SOFTREV}} (that shape would take
    // the format-error branch instead).
    SecsWireCodec builder;
    unsigned char zero = 0;
    builder.InitLocalHead(1, 14, 0);
    builder.DataItemOut(2, HType.LIST_TYPE, NULL);
    builder.DataItemOut(1, HType.BINARY_TYPE, &zero);
    builder.DataItemOut(0, HType.LIST_TYPE, NULL);
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));
    g.ProcessSocketReceiveData();
    CHECK(g.bWaitEstablishCommunicationsResponse == true && g.bWaitEstablishCommunicationsResponseError == false,
          "W3: S1F14 success reply -> Process_S1F14_ConnectRequestAcknowledge sets Response=true/Error=false");

    int r3 = g.DoConnect();   // case 200: success -> bConnect=true, Task=1, return 1
    CHECK(r3 == 1 && g.bConnect == true && g.iStartConnectTask == 1,
          "W3: DoConnect case 200 completes the handshake -- bConnect=true, Task resets to 1, returns 1");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W4] Host-initiated S1F13 -> our real S1F14 reply (tail's direct-dispatch
//  quartet, third member).
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w4_host_s1f13_reply()
{
    printf("\n[W906-SysModWire.W4] Host-initiated S1F13 -> our S1F14 reply (direct-dispatch)\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.3", 4703);
    g.srvGem->Open();

    HGem = &g;
    HSys.SystemModularInitial();
    g.SetMachineTypeAndSoftwarseVer("HT9045", "V906");

    conn->SimClearTx();
    SecsWireCodec builder;
    builder.InitLocalHead(1, 13, 1);   // a real host-initiated S1,F13 request (W=1)
    builder.DataItemOut(2, HType.LIST_TYPE, NULL);
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("HOST"));
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("1.0"));
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));
    g.ProcessSocketReceiveData();

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() > 0, "W4: incoming S1F13 -> tail's direct-dispatch branch produced a reply");
    if (!tx.empty())
    {
        CHECK(((unsigned char)tx[6] & 0x7f) == 1, "W4: reply MessageID_S == 1");
        CHECK((unsigned char)tx[7] == 14, "W4: reply MessageID_F == 14 (S1F14)");
    }
    CHECK(g.bReceiveEstablishCommunicationsRequest == true, "W4: bReceiveEstablishCommunicationsRequest latched true");
    // Incidental coverage: S1F14_ConnectRequestAcknowledge's own inline
    // chkMoreMessageAbortProcess check (golden uHGemClass.cpp:350) short-
    // circuits to "ignore the incoming body" when unchecked (ctor default).
    CHECK(g.chkMoreMessageAbortProcess->Checked == false,
          "W4: fixture -- chkMoreMessageAbortProcess defaults Checked==false (S1F14's own inline short-circuit, incidentally covered)");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W5] S1F1 -> S1F2 direct-dispatch (first member of the quartet).
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w5_s1f1_s1f2()
{
    printf("\n[W906-SysModWire.W5] S1F1 -> S1F2 direct-dispatch\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.4", 4704);
    g.srvGem->Open();

    HGem = &g;
    HSys.SystemModularInitial();
    g.SetMachineTypeAndSoftwarseVer("HT9045", "V906");

    conn->SimClearTx();
    SecsWireCodec builder;
    builder.InitLocalHead(1, 1, 1);   // S1,F1 (Are You There), head-only
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));
    g.ProcessSocketReceiveData();

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() > 0, "W5: incoming S1F1 -> tail's direct-dispatch branch produced an S1F2 reply");
    if (!tx.empty())
    {
        CHECK(((unsigned char)tx[6] & 0x7f) == 1, "W5: reply MessageID_S == 1");
        CHECK((unsigned char)tx[7] == 2, "W5: reply MessageID_F == 2 (S1F2)");
    }
    std::string body(tx.begin(), tx.end());
    CHECK(body.find("HT9045") != std::string::npos, "W5: S1F2 payload carries GemMDLN (\"HT9045\")");
    CHECK(body.find("V906") != std::string::npos, "W5: S1F2 payload carries GemSOFTREV (\"V906\")");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W6] Tail negative paths: DeviceID mismatch -> S9F1; unknown odd F on a
//  known stream -> S9F5; unknown stream -> S9F3; even-F secondary reply ->
//  silent; F==0 abort -> silent; malformed SML -> bDataFormatOK=false -> S9F7.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w6_tail_negative_paths()
{
    printf("\n[W906-SysModWire.W6] Tail negative paths (S9F1/S9F5/S9F3/silent x2/S9F7)\n");

    THGem *savedHGem = HGem;

    // (a) DeviceID mismatch: Remote.DeviceID(1234) != Local.DeviceID(0, ctor
    // default) and != 0xFFFF, W_Bit==1 -> S9F1_UnrecognizedDeviceID.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.5", 4705);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[14] = {
            0x00,0x00,0x00,0x0A,   // Value=10 -> Len=14 (head only)
            0x04,0xD2,             // DeviceID=1234 (0x04D2) -- mismatches Local.DeviceID==0
            0x81,                  // MessageID_S=1 | W_Bit(0x80)
            0x01,                  // MessageID_F=1
            0x00, 0x00,            // PType=0, SType=0 (Data_Message)
            0x00,0x00,0x00,0x01    // SystemByte=1
        };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() > 0, "W6(a): DeviceID mismatch produced a reply");
        if (!tx.empty())
        {
            CHECK(((unsigned char)tx[6] & 0x7f) == 9, "W6(a): reply MessageID_S == 9");
            CHECK((unsigned char)tx[7] == 1, "W6(a): reply MessageID_F == 1 (S9F1)");
        }
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (b) known stream (S=1), unknown ODD function (F=99) -> S9F5.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.6", 4706);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[14] = {
            0x00,0x00,0x00,0x0A, 0x00,0x00, 0x01, 99, 0x00,0x00, 0x00,0x00,0x00,0x01
        };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() > 0, "W6(b): unknown odd F on a known stream produced a reply");
        if (!tx.empty())
        {
            CHECK(((unsigned char)tx[6] & 0x7f) == 9, "W6(b): reply MessageID_S == 9");
            CHECK((unsigned char)tx[7] == 5, "W6(b): reply MessageID_F == 5 (S9F5)");
        }
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (c) unknown stream (S=99) -> S9F3.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.7", 4707);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[14] = {
            0x00,0x00,0x00,0x0A, 0x00,0x00, 99, 0x01, 0x00,0x00, 0x00,0x00,0x00,0x01
        };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() > 0, "W6(c): unknown stream produced a reply");
        if (!tx.empty())
        {
            CHECK(((unsigned char)tx[6] & 0x7f) == 9, "W6(c): reply MessageID_S == 9");
            CHECK((unsigned char)tx[7] == 3, "W6(c): reply MessageID_F == 3 (S9F3)");
        }
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (d) even-F secondary reply (S6F12) -> silently accepted, no reply.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.8", 4708);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[14] = {
            0x00,0x00,0x00,0x0A, 0x00,0x00, 0x06, 0x0C, 0x00,0x00, 0x00,0x00,0x00,0x01
        };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(conn->SimTxBuffer().size() == 0, "W6(d): even-F secondary message (S6F12) -- no reply");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (e) F==0 abort transaction (S1F0) -> silently ignored, no reply.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.9", 4709);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[14] = {
            0x00,0x00,0x00,0x0A, 0x00,0x00, 0x01, 0x00, 0x00,0x00, 0x00,0x00,0x00,0x01
        };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(conn->SimTxBuffer().size() == 0, "W6(e): F==0 (abort transaction) -- no reply");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // (f) malformed SML (legal 14-byte head, ONE top-level BINARY item whose
    // declared ItemSize (5) claims more bytes than the buffer actually
    // supplies (0 -- the buffer ends immediately after the length byte)) ->
    // ProcessSMLBinary's data-item loop (SecsWireCodec.cpp) checks
    // `RunLength>=Len` BEFORE reading (unlike the sibling list-recursion
    // check a few lines above it, which reads Ptr[RunLength] first and would
    // require an out-of-bounds heap read to reach at this buffer's exact
    // boundary -- deliberately NOT used here for that reason: ProcessBuffer
    // is `new`'d to EXACTLY Value+4 bytes, uHGemEquipment.cpp's own
    // ProcessSocketReceiveData, so reading one byte past it is undefined
    // behavior this test must not rely on) -> ShowSMLBinary returns -2 ->
    // bDataFormatOK=false -> S9F7_IllegalData (the FIRST check in the tail,
    // before even CheckSFCodeResponse/DeviceID).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.10", 4710);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        conn->SimClearTx();
        unsigned char frame[16] = {
            0x00,0x00,0x00,0x0C,   // Value=12 -> Len=Value+4=16 (== this buffer's actual size)
            0xFF,0xFF,             // DeviceID=0xFFFF (sentinel; unreachable anyway -- bDataFormatOK bails first)
            0x01, 0x01,            // MessageID_S=1, MessageID_F=1 (irrelevant -- bails before dispatch)
            0x00, 0x00,            // PType=0, SType=0 (Data_Message)
            0x00,0x00,0x00,0x01,   // SystemByte=1
            0x21,                  // byte14: BINARY_TYPE(0x20)|ct=1 -> 1 length byte follows
            0x05                   // byte15: declared ItemSize=5 -- but the buffer ends HERE (16
                                   // bytes total); the data-item loop's first iteration checks
                                   // RunLength(16)>=Len(16) BEFORE any read -> returns -2
        };
        conn->SimPushReceive(frame, 16);
        g.ProcessSocketReceiveData();

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() > 0, "W6(f): malformed SML produced a reply");
        if (!tx.empty())
        {
            CHECK(((unsigned char)tx[6] & 0x7f) == 9, "W6(f): reply MessageID_S == 9");
            CHECK((unsigned char)tx[7] == 7, "W6(f): reply MessageID_F == 7 (S9F7)");
        }
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W7] Flag latches (bReceiveS7F6/S101F6/S101F8/S110F2/bS1F2_OnLineData) +
//  MoveCheckCallBack short-circuit.
// ---------------------------------------------------------------------------
static int W906_SysModWire_MoveCheckCallBack_Ret1() { return 1; }
static int W906_SysModWire_MoveCheckCallBack_Ret0() { return 0; }

static void test_w906_sysmodwire_w7_flag_latch_movecheckcallback()
{
    printf("\n[W906-SysModWire.W7] Flag latches + MoveCheckCallBack short-circuit\n");

    THGem *savedHGem = HGem;

    // S7F6 -> bReceiveS7F6 (S known via S,F direct branch, MoveCheckCallBack NULL by ctor default).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.11", 4711);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 0x07,6, 0,0, 0,0,0,1 };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(g.bReceiveS7F6 == true, "W7: S7F6 -> bReceiveS7F6 latches true");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // S101F6 -> bReceiveS101F6.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.12", 4712);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 101,6, 0,0, 0,0,0,1 };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(g.bReceiveS101F6 == true, "W7: S101F6 -> bReceiveS101F6 latches true");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // S101F8 -> bReceiveS101F8.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.13", 4713);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 101,8, 0,0, 0,0,0,1 };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(g.bReceiveS101F8 == true, "W7: S101F8 -> bReceiveS101F8 latches true");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // S110F2 -> bReceiveS110F2 (this wave's one flagged golden-ctor deviation -- see header comment).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.14", 4714);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 110,2, 0,0, 0,0,0,1 };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(g.bReceiveS110F2 == true, "W7: S110F2 -> bReceiveS110F2 latches true");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // S1F2 -> bS1F2_OnLineData (direct-dispatch quartet member, before the customer/MoveCheckCallBack gate).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.15", 4715);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 1,2, 0,0, 0,0,0,1 };
        conn->SimPushReceive(frame, 14);
        g.ProcessSocketReceiveData();
        CHECK(g.bS1F2_OnLineData == true, "W7: S1F2 -> bS1F2_OnLineData latches true");
        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // MoveCheckCallBack==1 -> the ~40-branch chain short-circuits (return
    // BEFORE even checking S,F): S1F3 produces no reply, no crash.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.16", 4716);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        g.MoveCheckCallBack = W906_SysModWire_MoveCheckCallBack_Ret1;

        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 1,3, 0,0, 0,0,0,7 };
        conn->SimPushReceive(frame, 14);
        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "W7: MoveCheckCallBack()==1 path does not throw");
        CHECK(conn->SimTxBuffer().size() == 0, "W7: MoveCheckCallBack()==1 short-circuits the S,F chain -- no reply");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // MoveCheckCallBack==0 -> lets the chain through to S1F4_SelectedStatusReply,
    // which W906-SvEcDataItem UN-GATED (was a no-op stub when this test was
    // first written -- see that wave's own uHGemClass.cpp comment). The
    // incoming S1,F3 frame below carries NO data-item body (10-byte HSMS
    // header only), so real S1F4 hits its own `GetDataItemLenAndType(...)!=1`
    // golden `else` branch and replies with a genuine S9F7 "S1,F3 data format
    // error" frame (verified byte-for-byte: 39 bytes = 4-byte length prefix +  [STALE 20261009 W-187: golden 913 S9Fx body is <B[10]> -> 26 bytes, see the CHECK below]
    // 10-byte header (S,F=9,7) + 1-byte format/length (0x41,0x17=ASCII,len 23)
    // + the 23-byte ASCII string "S1,F3 data format error") -- CheckSFCodeResponse
    // (called unconditionally earlier in the tail, BEFORE S1F4 runs) still
    // demonstrably ran against THIS message too, proving the pump reached
    // this far (same seed/consume idiom as [T8]).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.17", 4717);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        g.MoveCheckCallBack = W906_SysModWire_MoveCheckCallBack_Ret0;

        g.SFCodeResponseList->Items->Add("1 3 7");   // matches the incoming message's own S,F,SystemByte below
        g.TimeLeft->Add("300");

        unsigned char frame[14] = { 0,0,0,0x0A, 0,0, 1,3, 0,0, 0,0,0,7 };
        conn->SimPushReceive(frame, 14);
        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "W7: MoveCheckCallBack()==0 path does not throw");
        const std::vector<char> &tx1f4 = conn->SimTxBuffer();
        CHECK(tx1f4.size() == 26 && (unsigned char)tx1f4[14] == 0x21 && (unsigned char)tx1f4[15] == 10 && std::memcmp(tx1f4.data() + 16, frame + 4, 10) == 0, "W7: MoveCheckCallBack()==0 lets the chain through to now-real S1F4, which replies with a 26-byte S9F7 frame, body <B[10]> = the S1,F3 header received (golden 913 SendS9FxEchoHeader, AI(W906-W187) 20261009; 0618: 39 bytes, <A> text) (empty-body S1,F3 has no SVID list)");
        CHECK((unsigned char)tx1f4[6] == 9 && (unsigned char)tx1f4[7] == 7, "W7: reply S,F == 9,7 (S9F7, not S1F4 -- the empty-body request itself is malformed)");
        CHECK(g.SFCodeResponseList->Items->Count == 0, "W7: CheckSFCodeResponse (called before S1F4 runs) consumed the pre-seeded \"1 3 7\" record -- the pump reached this message");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // MoveCheckCallBack==0, well-formed S1,F3 "request all SVs" (L,0) -> now-real
    // S1F4_SelectedStatusReply walks SvEcReg.SV_ID (empty -- FormCreate was never
    // called on this bare THGem) and replies with a genuine, empty S1F4 list.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.18", 4718);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();
        g.MoveCheckCallBack = W906_SysModWire_MoveCheckCallBack_Ret0;

        // S1,F3 body: L,0 -- golden `HType.LIST_TYPE`=0x00, SecsWireCodec::
        // GetLengthByte(0,...) always emits exactly 1 length-byte even for
        // len==0 (its do/while runs once regardless) -- so the wire encoding
        // is exactly 2 bytes: format+lenbytecount=0x01 (LIST_TYPE(0x00)|1),
        // length-value=0x00 (same "L[0]" shape test_SecsWireCodec.cpp:740
        // already exercises). Matches golden's own "report all SVID" shape
        // (SVlen==0).
        unsigned char frame[16] = { 0,0,0,0x0C, 0,0, 1,3, 0,0, 0,0,0,8, 0x01,0x00 };
        conn->SimPushReceive(frame, 16);
        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "W7b: well-formed empty-SVID-list S1F3 does not throw");
        const std::vector<char> &tx1f4b = conn->SimTxBuffer();
        // 4-byte length prefix + 10-byte header (S,F=1,4) + 2-byte L,0 (0x01,0x00).
        CHECK(tx1f4b.size() == 16, "W7b: real S1F4 replies with a 16-byte frame (10-byte header + L,0 empty SVID list -- SvEcReg.SV_ID has zero entries, FormCreate never ran on this bare fixture)");
        CHECK((unsigned char)tx1f4b[6] == 1 && (unsigned char)tx1f4b[7] == 4, "W7b: reply S,F == 1,4 (genuine S1F4, not an error path)");
        CHECK((unsigned char)tx1f4b[14] == 0x01 && (unsigned char)tx1f4b[15] == 0x00, "W7b: reply body is L,0 (format=LIST_TYPE, count=0) -- SvEcReg.SV_ID->Count==0");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W8] CC_TFME_CHINA customer gate: direct-dispatch quartet (S1F1) is
//  unaffected; the post-gate ~40-branch chain (S2F17->S2F18) is skipped when
//  GemControlState<=1, and dispatches normally once GemControlState>1.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w8_customer_code_gate()
{
    printf("\n[W906-SysModWire.W8] CC_TFME_CHINA customer gate\n");

    THGem *savedHGem = HGem;
    int savedCustomerCode = CUSTOMER_CODE;
    CUSTOMER_CODE = CC_TFME_CHINA;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.18", 4718);
    g.srvGem->Open();
    HGem = &g;
    HSys.SystemModularInitial();
    g.SetMachineTypeAndSoftwarseVer("HT9045", "V906");
    g.GemControlState = 1;

    conn->SimClearTx();
    SecsWireCodec s1f1;
    s1f1.InitLocalHead(1, 1, 1);
    conn->SimPushReceive(s1f1.LocalBuffer.data(), static_cast<int>(s1f1.LocalLength_4));
    g.ProcessSocketReceiveData();
    CHECK(conn->SimTxBuffer().size() > 0, "W8: CC_TFME_CHINA/GemControlState<=1 -- S1F1 direct-dispatch quartet still replies (S1F2)");

    conn->SimClearTx();
    unsigned char s2f17[14] = { 0,0,0,0x0A, 0,0, 2,17, 0,0, 0,0,0,1 };
    conn->SimPushReceive(s2f17, 14);
    g.ProcessSocketReceiveData();
    CHECK(conn->SimTxBuffer().size() == 0, "W8: CC_TFME_CHINA/GemControlState<=1 -- post-gate S2F17 does NOT dispatch (no reply)");

    g.GemControlState = 2;
    conn->SimClearTx();
    unsigned char s2f17b[14] = { 0,0,0,0x0A, 0,0, 2,17, 0,0, 0,0,0,2 };
    conn->SimPushReceive(s2f17b, 14);
    g.ProcessSocketReceiveData();
    CHECK(conn->SimTxBuffer().size() > 0, "W8: CC_TFME_CHINA/GemControlState==2 (>1) -- post-gate S2F17 dispatches normally (S2F18 reply)");
    if (conn->SimTxBuffer().size() > 0)
    {
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(((unsigned char)tx[6] & 0x7f) == 2 && (unsigned char)tx[7] == 18, "W8: reply is S2F18 (Date and Time Data)");
    }

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
    CUSTOMER_CODE = savedCustomerCode;
}

// ---------------------------------------------------------------------------
//  [W9] Two-codec merge oracle: S2F26_DiagnosticLoopbackData (an ALREADY
//  ActiveWire-routed HTGem method) reads/writes THGem::WireCodec via
//  ActiveWire when wired -- proving the handler consumes the SAME decode
//  buffer ProcessSocketReceiveData just populated, not HTGem's own separate
//  by-value WireCodec instance.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w9_two_codec_merge_oracle()
{
    printf("\n[W906-SysModWire.W9] Two-codec merge oracle (S2F25 -> S2F26 loopback)\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.19", 4719);
    g.srvGem->Open();
    HGem = &g;
    HSys.SystemModularInitial();

    conn->SimClearTx();
    SecsWireCodec builder;
    unsigned char payload[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    builder.InitLocalHead(2, 25, 1);
    builder.DataItemOut(4, HType.BINARY_TYPE, payload);
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));
    g.ProcessSocketReceiveData();

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() > 0, "W9: S2F25 -> S2F26_DiagnosticLoopbackData (ACTIVE) produced a reply");
    if (!tx.empty())
    {
        CHECK(((unsigned char)tx[6] & 0x7f) == 2, "W9: reply MessageID_S == 2");
        CHECK((unsigned char)tx[7] == 26, "W9: reply MessageID_F == 26 (S2F26)");
    }
    std::string body(tx.begin(), tx.end());
    std::string expectedPayload(reinterpret_cast<char*>(payload), 4);
    CHECK(body.find(expectedPayload) != std::string::npos,
          "W9: S2F26 echoes the SAME binary payload -- handler read THGem::WireCodec.SReceiveData via ActiveWire");
    CHECK(HSys.MyGem->WireCodec.SReceiveData->Count == 0,
          "W9: MyGem's OWN by-value WireCodec (HTGem's standalone instance) was never touched -- SReceiveData stays empty");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W10] Null-guard regression: HSys.MyGem==NULL -- the tail's whole `if`
//  block is skipped (no reply, no crash), WireCodec.Remote still decodes.
//  This is [T5]'s own gated-dispatcher semantics re-verified now that the
//  tail is a real (null-guarded) block instead of a gated `#if 0`.
// ---------------------------------------------------------------------------
static void test_w906_sysmodwire_w10_null_guard_regression()
{
    printf("\n[W906-SysModWire.W10] Null-guard regression (HSys.MyGem==NULL)\n");

    CHECK(HSys.MyGem == NULL, "W10: fixture -- HSys.MyGem is NULL (no prior test in this file left it wired)");

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.2.20", 4720);
    g.srvGem->Open();

    SecsWireCodec builder;
    builder.InitLocalHead(1, 14, 0);
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("OK"));
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

    bool threw = false;
    try { g.ProcessSocketReceiveData(); }
    catch (...) { threw = true; }
    CHECK(threw == false, "W10: null-guarded tail does not throw with HSys.MyGem==NULL");

    CHECK(g.WireCodec.Remote.MessageID_S == 1 && g.WireCodec.Remote.MessageID_F == 14,
          "W10: WireCodec.Remote still decodes MessageID_S/F == 1/14");
    CHECK(conn->SimTxBuffer().size() == 0, "W10: null-guarded tail sends no reply (HSys.MyGem stays NULL)");
}

// ===========================================================================
//  W906-SvEcDataItem (design doc DESIGN_SECSGEM_closing_waves.md, Wave 1):
//  SvEcReg embed + SV/EC DataItem family + FormCreate + 13 uHGemClass.cpp
//  un-gated handlers. No file I/O anywhere in this bracket (FormCreate/
//  DataItemOutSV/DataItemOutEC/DataItemOutECNameList/Send*/IsValidSVID are
//  all pure in-memory wire-codec + SvEcReg composition) EXCEPT the dispatch-
//  pump e2e tests below, which (like the W1-W10 SysModWire block above) reach
//  the real SaveSECSGEMTextToLog() through ProcessSocketReceiveData -- wrapped
//  in the same TextLogSnapshot capture/restore idiom in main().
// ===========================================================================

// ---------------------------------------------------------------------------
//  [F1] FormCreate -- system SV/EC registration (golden uHGemEquipment.cpp:
//  6165-6207). Pure in-memory (SvEcReg.SetSVDataPointer/SetECDataPointer);
//  no file I/O, no snapshot needed.
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_formcreate()
{
    printf("\n[W906-SvEcDataItem.F1] FormCreate -- system SV/EC registration\n");

    THGem g;
    g.FormCreate(NULL);

    static const char *kExpectedSV[] = {
        "3","4","5","6","9","10","11","12","13","14","15","16","17","18","19",
        "24","25","54","57","70","71"
    };
    for (size_t i = 0; i < sizeof(kExpectedSV)/sizeof(kExpectedSV[0]); ++i)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "F1: SvEcReg.SV_ID contains SVID %s (FormCreate registered)", kExpectedSV[i]);
        CHECK(g.SvEcReg.SV_ID->IndexOf(AnsiString(kExpectedSV[i])) >= 0, msg);
    }
    CHECK(g.SvEcReg.EC_ID->IndexOf(AnsiString("68")) >= 0, "F1: SvEcReg.EC_ID contains EC68 (Time Format)");
    CHECK(g.SvEcReg.GetECDataValue("68") == "0", "F1: GetECDataValue(68) reads back iTimeFormat's default value (0)");

    // szManID/GetCPUType/lCPUFreq -- PORT-ONLY placeholders (TasmInfo.cpp D1,
    // see that file's own file-head note: inline-asm cpuid/RDTSC cannot be
    // ported to MinGW's asm dialect; conservative fixed placeholders used
    // instead since these 3 SVs are host-queried-only, zero machine-behavior
    // consumer).
    CHECK(std::string(g.szManID) == "GenuineIntel", "F1: szManID == PORT-ONLY placeholder \"GenuineIntel\" (TasmInfo D1)");
    CHECK(std::string(g.szGetCPUType) == "", "F1: szGetCPUType == PORT-ONLY placeholder \"\" (TasmInfo D1)");
    CHECK(g.lCPUFreq == 0, "F1: lCPUFreq == PORT-ONLY placeholder 0 (TasmInfo D1)");
}

// ---------------------------------------------------------------------------
//  [E1] S1F3(SVID=4) -> real S1F4 -- exercises FormCreate's raw-ptr
//  registration + DataItemOutSV's non-VCL numeric-encode path end-to-end
//  through the real dispatch pump (ProcessSocketReceiceData -> HSys.MyGem->
//  S1F4_SelectedStatusReply -> HGemPtr->DataItemOutSV). SendLocalData reaches
//  SaveSECSGEMTextToLog -- TextLogSnapshot-wrapped by the caller in main().
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_e2e_s1f4_formcreate()
{
    printf("\n[W906-SvEcDataItem.E1] FormCreate + S1F4 e2e -- specific-SVID round-trip\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.1", 4801);
    g.srvGem->Open();
    HGem = &g;
    HSys.SystemModularInitial();

    g.FormCreate(NULL);
    g.GemControlState = 2;   // SVID 4, UINT_1_TYPE -- non-default value, proves live data flows through

    SecsWireCodec builder;
    builder.InitLocalHead(1, 3, 0);
    builder.DataItemOut(1, HType.LIST_TYPE, NULL);
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("4"));
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

    bool threw = false;
    try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
    CHECK(threw == false, "E1: S1F3(SVID=4) does not throw");

    const std::vector<char> &tx = conn->SimTxBuffer();
    CHECK(tx.size() == 19, "E1: real S1F4 reply is 19 bytes (10-byte header + L,1[UINT_1(GemControlState)])");
    CHECK((unsigned char)tx[6] == 1 && (unsigned char)tx[7] == 4, "E1: reply S,F == 1,4");
    CHECK((unsigned char)tx[14] == 0x01 && (unsigned char)tx[15] == 0x01, "E1: reply body L,1 (one value)");
    CHECK((unsigned char)tx[16] == (unsigned char)(HType.UINT_1_TYPE | 1), "E1: value item format byte == UINT_1_TYPE|1");
    CHECK((unsigned char)tx[17] == 1, "E1: value item length byte == 1");
    CHECK((unsigned char)tx[18] == 2, "E1: value payload == 2 (live GemControlState, flowed through real DataItemOutSV)");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [E2] S1,F23 "report all" -> real S1F24 -- exercises strGrdCEID/
//  stdGridReportID (hand-seeded, same layout as test_ceid_family) through the
//  real dispatch pump.
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_e2e_s1f24()
{
    printf("\n[W906-SvEcDataItem.E2] S1,F23 \"report all\" e2e -- real S1F24\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.2", 4802);
    g.srvGem->Open();
    HGem = &g;
    HSys.SystemModularInitial();

    // One CEID row: CEID=11, Alias="A", zero linked ReportIDs (cols 3+ blank)
    // -- keeps the encoded reply small/predictable (SVIDList stays empty).
    g.strGrdCEID->Cells[0][1] = 11;
    g.strGrdCEID->Cells[2][1] = "A";

    SecsWireCodec builder;
    builder.InitLocalHead(1, 23, 0);
    builder.DataItemOut(0, HType.LIST_TYPE, NULL);   // L,0 -- "report all CEID"
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

    bool threw = false;
    try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
    CHECK(threw == false, "E2: S1,F23 \"report all\" does not throw");

    const std::vector<char> &tx = conn->SimTxBuffer();
    // header(14) + outer L,1(2) + inner L,3(2) + CEID UINT_4(6) + Alias ASCII"A"(3) + inner SVIDList L,0(2) = 29
    CHECK(tx.size() == 29, "E2: real S1F24 reply is 29 bytes (one CEID row, empty linked-SVID list)");
    CHECK((unsigned char)tx[6] == 1 && (unsigned char)tx[7] == 24, "E2: reply S,F == 1,24");
    CHECK((unsigned char)tx[14] == 0x01 && (unsigned char)tx[15] == 0x01, "E2: outer L,1 (one CEID entry -- only row 1 is non-blank)");
    CHECK((unsigned char)tx[16] == 0x01 && (unsigned char)tx[17] == 0x03, "E2: inner L,3 (CEID,Alias,ReportID-list)");
    CHECK((unsigned char)tx[18] == (unsigned char)(HType.UINT_4_TYPE | 1) && (unsigned char)tx[19] == 4,
          "E2: CEID field format/len == UINT_4_TYPE|1, 4 data bytes");
    CHECK((unsigned char)tx[20] == 0 && (unsigned char)tx[21] == 0 && (unsigned char)tx[22] == 0 && (unsigned char)tx[23] == 11,
          "E2: CEID value == 11 (big-endian u32)");
    CHECK((unsigned char)tx[24] == (unsigned char)(HType.ASCII_TYPE | 1) && (unsigned char)tx[25] == 1 && (unsigned char)tx[26] == 'A',
          "E2: Alias field == ASCII \"A\"");
    CHECK((unsigned char)tx[27] == 0x01 && (unsigned char)tx[28] == 0x00, "E2: linked-ReportID list == L,0 (none seeded)");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [E3] S6,F19 -> real S6F20 -- exercises stdGridReportID + SvEcReg.SV_ID
//  (IndexOf) + DataItemOutSV together through the real dispatch pump.
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_e2e_s6f20()
{
    printf("\n[W906-SvEcDataItem.E3] S6,F19 e2e -- real S6F20 (stdGridReportID + SvEcReg.SV_ID + DataItemOutSV)\n");

    THGem *savedHGem = HGem;

    THGem g;
    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.3", 4803);
    g.srvGem->Open();
    HGem = &g;
    HSys.SystemModularInitial();

    // Register one raw-ptr SV (SVID "9", UINT_1_TYPE, live value 7) and link
    // it into ReportID "100" (stdGridReportID row: ReportID=100, Mode=2,
    // SVID at col2 -- same layout test_ceid_family already established).
    static unsigned char liveVal = 7;
    g.SvEcReg.SetSVDataPointer("9", HType.UINT_1_TYPE, "TestSV9", "", (void*)&liveVal, "e2e test SV");
    g.stdGridReportID->Cells[0][1] = 100;
    g.stdGridReportID->Cells[1][1] = 2;
    g.stdGridReportID->Cells[2][1] = 9;

    SecsWireCodec builder;
    builder.InitLocalHead(6, 19, 0);
    builder.DataItemOut(HType.ASCII_TYPE, AnsiString("100"));   // requested ReportID
    conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

    bool threw = false;
    try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
    CHECK(threw == false, "E3: S6,F19(ReportID=100) does not throw");

    const std::vector<char> &tx = conn->SimTxBuffer();
    // header(14) + L,1(2) + UINT_1(liveVal)(3) = 19
    CHECK(tx.size() == 19, "E3: real S6F20 reply is 19 bytes (10-byte header + L,1[UINT_1(liveVal)])");
    CHECK((unsigned char)tx[6] == 6 && (unsigned char)tx[7] == 20, "E3: reply S,F == 6,20");
    CHECK((unsigned char)tx[14] == 0x01 && (unsigned char)tx[15] == 0x01, "E3: reply body L,1 (one linked SVID)");
    CHECK((unsigned char)tx[16] == (unsigned char)(HType.UINT_1_TYPE | 1) && (unsigned char)tx[17] == 1,
          "E3: value item format/len == UINT_1_TYPE|1");
    CHECK((unsigned char)tx[18] == 7, "E3: value payload == 7 (live SV9 value, flowed through DataItemOutSV via SvEcReg.SV_ID lookup)");

    delete HSys.MyGem; HSys.MyGem = NULL;
    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [E4] S5,F7 -> real S5F8, and direct S100F4_ReportAllAlarm -- both walk
//  strGrdAlarm (hand-seeded, same col layout uHGemClass.cpp's translated
//  bodies read: col1=ALID, col2=Class/type flag, col4=ALTX, col7=Enable).
//  S100F4 has no incoming-request trigger wired in ProcessReceiceData's own
//  dispatch guard requiring a specific format (it is called unconditionally
//  once S,F==100,3 matches), so it is exercised the SAME way as S5F8: via
//  the real dispatch pump.
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_e2e_s5f8_s100f4()
{
    printf("\n[W906-SvEcDataItem.E4] S5,F7/S100,F3 e2e -- real S5F8 / S100F4 (strGrdAlarm)\n");

    THGem *savedHGem = HGem;

    // -- S5F8 --------------------------------------------------------------
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.4", 4804);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        // One enabled alarm row: ALID=42, ALTX="Hi", Enable(col7)="1".
        g.strGrdAlarm->Cells[1][1] = 42;
        g.strGrdAlarm->Cells[4][1] = "Hi";
        g.strGrdAlarm->Cells[7][1] = "1";

        SecsWireCodec builder;
        builder.InitLocalHead(5, 7, 0);
        // golden's S5F8_ListEnableAlarmAcknowledge only PEEKS one data item
        // exists (GetDataItemLenAndType(...)!=1 -> format-error path) -- it
        // never actually consumes/decodes it, so any well-formed item (here,
        // the conventional L,0 "list everything" shape) satisfies the peek.
        builder.DataItemOut(0, HType.LIST_TYPE, NULL);
        conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "E4a: S5,F7 does not throw");

        const std::vector<char> &tx = conn->SimTxBuffer();
        // header(14) + outer L,1(2) + inner L,3(2) + BINARY(ALT=0x80)(3) + UINT_4(42)(6) + ASCII"Hi"(4) = 31
        // (golden field order inside the inner L,3 is ALT, ALID, ALTX -- NOT
        // ALID first; see THGem::S5F8_ListEnableAlarmAcknowledge/uHGemClass.cpp).
        CHECK(tx.size() == 31, "E4a: real S5F8 reply is 31 bytes (one enabled alarm row)");
        CHECK((unsigned char)tx[6] == 5 && (unsigned char)tx[7] == 8, "E4a: reply S,F == 5,8");
        CHECK((unsigned char)tx[18] == (unsigned char)(HType.BINARY_TYPE | 1) && (unsigned char)tx[19] == 1
              && (unsigned char)tx[20] == 0x80,
              "E4a: ALT field == BINARY 0x80 (fixed alarm-type byte)");
        CHECK((unsigned char)tx[21] == (unsigned char)(HType.UINT_4_TYPE | 1) && (unsigned char)tx[22] == 4,
              "E4a: ALID field == UINT_4_TYPE|1, 4 bytes");
        CHECK((unsigned char)tx[23] == 0 && (unsigned char)tx[24] == 0 && (unsigned char)tx[25] == 0 && (unsigned char)tx[26] == 42,
              "E4a: ALID value == 42");
        CHECK((unsigned char)tx[27] == (unsigned char)(HType.ASCII_TYPE | 1) && (unsigned char)tx[28] == 2
              && (unsigned char)tx[29] == 'H' && (unsigned char)tx[30] == 'i',
              "E4a: ALTX field == ASCII \"Hi\"");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    // -- S100F4 --------------------------------------------------------------
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.5", 4805);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        // One alarm row: ALID=7, Class(col2)="1"(true), ALTX(col4)="X".
        g.strGrdAlarm->Cells[1][1] = 7;
        g.strGrdAlarm->Cells[2][1] = "1";
        g.strGrdAlarm->Cells[4][1] = "X";

        SecsWireCodec builder;
        builder.InitLocalHead(100, 3, 0);
        conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "E4b: S100,F3 does not throw");

        const std::vector<char> &tx = conn->SimTxBuffer();
        // AI(W906-SvEcDataItem) 20260720: golden reports `strGrdAlarm->RowCount-1`
        // rows UNCONDITIONALLY (design doc quirk Q11), NOT just non-blank ones
        // -- strGrdAlarm's real RowCount is 5 (uHGemEquipment.cpp:376, `new
        // TStringGrid(12,5)`), so the outer list is L,4 (rows 1..4), with
        // rows 2-4 blank (ASCII "" pairs + BOOLEAN(0)). Exact byte offsets
        // for the 3 blank-row triples are not hand-verified here (low value,
        // high arithmetic-error risk); instead this test verifies the
        // header(14) + outer L,4(2 bytes) shape and searches for row 1's own
        // ALID("7")/ALTX("X")/BOOLEAN(1) triple as a byte substring, which
        // still proves live strGrdAlarm data reached the wire via the real
        // (un-gated) S100F4_ReportAllAlarm.
        CHECK(tx.size() >= 16, "E4b: real S100F4 reply carries at least a header + outer-list shape");
        CHECK((unsigned char)tx[6] == 100 && (unsigned char)tx[7] == 4, "E4b: reply S,F == 100,4");
        CHECK((unsigned char)tx[14] == 0x01 && (unsigned char)tx[15] == 0x04,
              "E4b: outer list == L,4 (golden's own RowCount-1 quirk, design doc Q11 -- unconditional, not row-count-aware)");
        // row 1's own triple: L,3 / ASCII"7" / ASCII"X" / BOOLEAN(1).
        const unsigned char row1[] = {
            (unsigned char)(HType.LIST_TYPE | 1), 3,
            (unsigned char)(HType.ASCII_TYPE | 1), 1, '7',
            (unsigned char)(HType.ASCII_TYPE | 1), 1, 'X',
            (unsigned char)(HType.BOOLEAN_TYPE | 1), 1, 1
        };
        bool found = false;
        for (size_t off = 0; off + sizeof(row1) <= tx.size() && !found; ++off)
        {
            bool match = true;
            for (size_t k = 0; k < sizeof(row1) && match; ++k)
                if ((unsigned char)tx[off + k] != row1[k]) match = false;
            if (match) found = true;
        }
        CHECK(found, "E4b: row 1's L,3[ASCII\"7\",ASCII\"X\",BOOLEAN(1)] triple is present verbatim in the reply");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [E5] S101,F5/S101,F7 smoke -- S101F6/S101F8 are real (un-gated this wave),
//  but their own S101F6_StoreHostUploadFile/S101F8_StoreHostUploadFile
//  callees STAY gated no-op stubs (Wave 3b territory) -- proves the caller
//  reaches ActiveWire->LocalAcknowledge cleanly despite the gated callee.
// ---------------------------------------------------------------------------
static void test_w906_svecdataitem_e2e_s101f6_s101f8_smoke()
{
    // AI(W906-uHGemClass-Micro7) 20260721: UPDATED comment only (no assertion
    // changes) -- S101F6_StoreHostUploadFile/S101F8_StoreHostUploadFile are
    // REAL now (un-gated this wave, uHGemClass.cpp), not the gated no-op stub
    // this test's original name/printf implied. This test still passes
    // UNCHANGED because it sends a bare S101,F5/S101,F7 header with NO data
    // items at all -- the real Sub's own outer
    // `ActiveWire->DataItemIn(5,LIST_TYPE,NULL)==1` guard is false on an empty
    // SReceiveData queue (see DataItemInSub: `SReceiveData->Count==0 ->
    // return -1`), so the Sub's whole body is skipped, same net effect as the
    // old no-op stub for this particular (empty-request) scenario. The Sub's
    // REAL success/double-ack/file-write/Checked-write paths are exercised by
    // test_w906_uhgemclass_micro7_e2e() below instead.
    printf("\n[W906-SvEcDataItem.E5] S101,F5/S101,F7 smoke -- real S101F6/S101F8 wrappers, empty-body request never enters the (now real) *_StoreHostUploadFile Sub's guarded block\n");

    THGem *savedHGem = HGem;

    // S101F6 (S101,F5 request).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.6", 4806);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        SecsWireCodec builder;
        builder.InitLocalHead(101, 5, 0);
        conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "E5a: S101,F5 does not throw (empty request never enters the now-real StoreHostUploadFile Sub's guarded block)");
        CHECK(g.bReceiveS101F5 == true, "E5a: bReceiveS101F5 latches true (S101F6's own tail)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "E5a: LocalAcknowledge(101,6,0) produces a 17-byte ack frame (14-byte header + BINARY|1 fmt + len + 1 ack byte)");
        CHECK((unsigned char)tx[6] == 101 && (unsigned char)tx[7] == 6, "E5a: reply S,F == 101,6");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }
    // S101F8 (S101,F7 request).
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.3.7", 4807);
        g.srvGem->Open();
        HGem = &g;
        HSys.SystemModularInitial();

        SecsWireCodec builder;
        builder.InitLocalHead(101, 7, 0);
        conn->SimPushReceive(builder.LocalBuffer.data(), static_cast<int>(builder.LocalLength_4));

        bool threw = false;
        try { g.ProcessSocketReceiveData(); } catch (...) { threw = true; }
        CHECK(threw == false, "E5b: S101,F7 does not throw (empty request never enters the now-real StoreHostUploadFile Sub's guarded block)");
        CHECK(g.bReceiveS101F7 == true, "E5b: bReceiveS101F7 latches true (S101F8's own tail)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "E5b: LocalAcknowledge(101,8,0) produces a 17-byte ack frame");
        CHECK((unsigned char)tx[6] == 101 && (unsigned char)tx[7] == 8, "E5b: reply S,F == 101,8");

        delete HSys.MyGem; HSys.MyGem = NULL;
    }

    HGem = savedHGem;
}

// ---------------------------------------------------------------------------
//  [W906-VCW1] AI(W906-VCW1) 20260721: DataItemOutSV/DataItemOutEC UN-GATED --
//  vclcompat/Controls.h dynamic_cast dispatch coverage (golden
//  uHGemEquipment.cpp:2513-2585/2882-2957). Direct calls, no socket/dispatch-
//  pump needed (DataItemOutSV/DataItemOutEC only ever touch `WireCodec.`,
//  never `ActiveWire->` -- see this file's own un-gating note on those
//  methods) -- pure in-memory, no file I/O, no snapshot needed, matching F1's
//  own "no snapshot needed" precedent above.
//  SCOPE REMINDER: this proves the CAST-DISPATCH CODE compiles and reads
//  correctly from a real TStringList instance constructed BY THIS TEST -- it
//  does NOT mean any real widget-backed SV/EC from uHGemHT9045_SV.cpp/_EC.cpp
//  is functional (none exist yet; see vclcompat/Controls.h's own file-head
//  note).
// ---------------------------------------------------------------------------
static void test_w906_vcw1_dataitemout_vcl_dispatch()
{
    printf("\n[W906-VCW1] DataItemOutSV/DataItemOutEC -- UN-GATED VCL-widget (TStringList-backed) read dispatch\n");

    // DataItemOutSV: SV registered VCL_NAME=="1" (TObject* tag), backed by a
    // real TStringList (dynamic_cast target #7, StringListPtr branch, first
    // in golden's if/else-if chain). ASCII_TYPE so the encoded wire item's
    // payload bytes are the CommaText string itself, unconverted -- the
    // clearest possible proof the read actually flowed through
    // StringListPtr->CommaText, not some other branch or a stale default.
    {
        THGem g;
        TStringList *slSv = new TStringList();
        slSv->CommaText = "42";
        g.SvEcReg.SV_ID->Add(AnsiString(900));
        g.SvEcReg.SV_TYPE->Add(AnsiString((int)HType.ASCII_TYPE));
        g.SvEcReg.SV_NAME->Add("TestSvStrList");
        g.SvEcReg.SV_UNIT->Add("unit");
        g.SvEcReg.SV_Ptr->Add((void*)slSv);
        g.SvEcReg.VCL_NAME->Add("1");
        g.SvEcReg.SV_LEN->Add(1);

        bool ret = g.DataItemOutSV(AnsiString(900));
        CHECK(ret == true, "DataItemOutSV: TStringList-backed SV -> returns true (found+encoded)");
        const std::vector<unsigned char> &buf = g.WireCodec.LocalBuffer;
        // LocalLength_4 starts at 4, not 0 -- SecsWireCodec's ctor primes it
        // to reserve the 4-byte message-length header golden's (out-of-
        // scope) CreateLocalHead() would otherwise write first (see
        // SecsWireCodec.cpp's own ctor comment); no InitLocalHead call here,
        // so the item bytes land right after that 4-byte reserve, at offset 4.
        CHECK(g.WireCodec.LocalLength_4 == 8, "DataItemOutSV: wire cursor advances by 4 bytes (format+len+\"42\") past the 4-byte header reserve");
        CHECK(buf[4] == (unsigned char)(HType.ASCII_TYPE | 1), "DataItemOutSV: item format byte == ASCII_TYPE|1");
        CHECK(buf[5] == 2, "DataItemOutSV: item length byte == 2 (strlen(\"42\"))");
        CHECK(buf[6] == '4' && buf[7] == '2', "DataItemOutSV: payload == \"42\" (live CommaText, flowed through StringListPtr branch)");
    }

    // DataItemOutEC: same StringListPtr branch, EC side (EC_VCL_NAME=="1").
    // Different CommaText value than the SV case above so a copy/paste bug
    // between the two (e.g. reading the wrong Ptr) would be caught.
    {
        THGem g;
        TStringList *slEc = new TStringList();
        slEc->CommaText = "77";
        g.SvEcReg.EC_ID->Add(AnsiString(901));
        g.SvEcReg.EC_TYPE->Add(AnsiString((int)HType.ASCII_TYPE));
        g.SvEcReg.EC_NAME->Add("TestEcStrList");
        g.SvEcReg.EC_UNIT->Add("unit");
        g.SvEcReg.EC_Ptr->Add((void*)slEc);
        g.SvEcReg.EC_VCL_NAME->Add("1");

        g.DataItemOutEC(AnsiString(901));
        const std::vector<unsigned char> &buf = g.WireCodec.LocalBuffer;
        CHECK(g.WireCodec.LocalLength_4 == 8, "DataItemOutEC: wire cursor advances by 4 bytes (format+len+\"77\") past the 4-byte header reserve");
        CHECK(buf[4] == (unsigned char)(HType.ASCII_TYPE | 1), "DataItemOutEC: item format byte == ASCII_TYPE|1");
        CHECK(buf[5] == 2, "DataItemOutEC: item length byte == 2 (strlen(\"77\"))");
        CHECK(buf[6] == '7' && buf[7] == '7', "DataItemOutEC: payload == \"77\" (live CommaText, flowed through StringListPtr branch)");
    }

    // Not-found path (SVID/ECID never registered) -- unaffected by this
    // wave's dynamic_cast additions, spot-checked once for regression safety.
    // golden's not-found tail unconditionally touches `DB->Lines->Add(...)`
    // (DB defaults to NULL, externally assigned via out-of-scope
    // SetDisplayPtr(TMemo*) in a live system) -- DB must be primed first,
    // matching this file's own established DB-lifecycle precedent elsewhere
    // (e.g. test_w906_alarmreportack_e2e: `g.DB = new THGemMemo(); ...
    // delete g.DB; g.DB = NULL;`), not a defect this wave introduced or
    // needs to fix.
    {
        THGem g;
        g.DB = new THGemMemo();
        bool ret = g.DataItemOutSV(AnsiString(999));
        CHECK(ret == false, "DataItemOutSV: unregistered SVID -> returns false (not-found path unaffected)");
        delete g.DB; g.DB = NULL;
    }
}

// ===========================================================================
//  [W906-AlarmReportAck] Real THGem-backed coverage for the 8 uHGemClass.cpp
//  methods un-gated this wave (see that file's own "INTEGRATE WAVE 5" note):
//  S2F34/S2F36/S2F38/S5F4/S5F6 family.
//
//  WIRING: a plain `HTGem hgem; hgem.HGemPtr=&g; hgem.ActiveWire=&g.WireCodec;`
//  (does NOT touch the HSys/HGem globals at all -- simpler and more
//  self-contained than the [W1]-family tests above, sufficient here since
//  none of these 8 methods need HSys.MyGem wired for anything else). This
//  puts BOTH the HGemPtr-side (real THGem state: slTempReportID/slTempCeID/
//  strGrdAlarm) and ActiveWire-side (real wire primitives: DataItemIn/
//  InitLocalHead/DataItemOut/SendLocalData) of each method against ONE
//  shared THGem instance -- proving the HGemPtr-vs-ActiveWire split this
//  wave's un-gating relies on actually works end-to-end, not just "compiles".
//
//  DANGER AVOIDED (flagged explicitly): THGem::SaveEventReportData() writes
//  to a HARDCODED absolute production path (see its own .cpp comment,
//  "DANGEROUS IF EXECUTED AS-IS ON THIS DEV MACHINE") -- NEVER call any code
//  path that reaches it. ProcessHostSendReportID/ProcessHostSendReportLinkID/
//  EnableDisableEventReport's SUCCESS tails all call it unconditionally, so
//  this test deliberately exercises ONLY: (a) the 2 Sub() methods' own
//  "delete all" success return directly (Sub itself never calls
//  SaveEventReportData -- only its caller's success continuation does), and
//  (b) every FORMAT-ERROR / early-return path (returns before ever reaching
//  SaveEventReportData). The full multi-method success chain
//  (Sub->Process*->Save) is NOT exercised here by design.
// ===========================================================================
static void test_w906_alarmreportack_e2e()
{
    printf("\n[W906-AlarmReportAck] S2F34/S2F36/S2F38/S5F4/S5F6 real THGem-backed coverage\n");

    // -- S2F34_DefineReportAcknowledgeSub: format-error path (empty SReceiveData) --
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        int ret = hgem.S2F34_DefineReportAcknowledgeSub();
        CHECK(ret == -1, "A1: S2F34_DefineReportAcknowledgeSub() with empty SReceiveData -> -1 (format error)");
    }
    // -- S2F34_DefineReportAcknowledgeSub: "delete all" success path (<L,2 <ASCII DATAID> <L,0>>) --
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("ABC"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(0));
        int ret = hgem.S2F34_DefineReportAcknowledgeSub();
        CHECK(ret == 1, "A2: S2F34_DefineReportAcknowledgeSub() with <DATAID><L,0> -> 1 (delete-all success, DeleteAllHostDefineReportID only)");
        CHECK(g.slTempReportID->Count == 0, "A2: slTempReportID stays empty (nothing queued by the delete-all branch)");
    }
    // -- S2F34_DefineReportAcknowledge: format-error path -> real DRACK(0x02) on the wire --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.1", 4901);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S2F34_DefineReportAcknowledge();   // empty SReceiveData -> Sub()==-1 -> DefineReportAcknowledgeFormatError()
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "A3: S2F34_DefineReportAcknowledge format-error -> 17-byte DRACK frame");
        if (tx.size() == 17)
        {
            CHECK((unsigned char)tx[6] == 2 && (unsigned char)tx[7] == 34, "A3: reply S,F == 2,34");
            CHECK((unsigned char)tx[16] == 0x02, "A3: DRACK byte == 0x02 (format error)");
        }
    }
    // -- S2F34_ProcessHostSendReportID: "invalid SVID" early-return path
    //    (returns BEFORE ever reaching SaveEventReportData) --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.2", 4902);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        TStringList *svids = new TStringList();
        svids->Add("999");   // never registered -> IsValidSVID("999")==false
        g.slTempReportID->Add("500");
        g.lTempReportIDContent->Add(svids);

        hgem.S2F34_ProcessHostSendReportID();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "A4: ProcessHostSendReportID with an unregistered SVID -> 17-byte DRACK frame");
        if (tx.size() == 17)
        {
            CHECK((unsigned char)tx[6] == 2 && (unsigned char)tx[7] == 34, "A4: reply S,F == 2,34");
            CHECK((unsigned char)tx[16] == 0x04, "A4: DRACK byte == 0x04 (invalid SVID) -- DefineReportAcknowledgeInvalidSVID fired");
        }
        // Early-return path never clears slTempReportID/lTempReportIDContent
        // (matches golden verbatim -- see ProcessHostSendReportID's own .cpp
        // citation); `svids` is deliberately left un-freed too, same as
        // golden's own leak on this exact path -- inconsequential for a
        // short-lived test process.
    }
    // -- S2F36_LinkEventReportAcknowledgeSub: format-error + "delete all" success --
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        int ret = hgem.S2F36_LinkEventReportAcknowledgeSub();
        CHECK(ret == -1, "A5: S2F36_LinkEventReportAcknowledgeSub() with empty SReceiveData -> -1 (format error)");
    }
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("ABC"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(0));
        int ret = hgem.S2F36_LinkEventReportAcknowledgeSub();
        CHECK(ret == 1, "A6: S2F36_LinkEventReportAcknowledgeSub() with <DATAID><L,0> -> 1 (delete-all success, DeleteAllHostDefineCeid only)");
        CHECK(g.slTempCeID->Count == 0, "A6: slTempCeID stays empty");
    }
    // -- S2F36_LinkEventReportAcknowledge: format-error path -> real LRACK(0x02) --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.3", 4903);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S2F36_LinkEventReportAcknowledge();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "A7: S2F36_LinkEventReportAcknowledge format-error -> 17-byte LRACK frame");
        if (tx.size() == 17)
        {
            CHECK((unsigned char)tx[6] == 2 && (unsigned char)tx[7] == 36, "A7: reply S,F == 2,36");
            CHECK((unsigned char)tx[16] == 0x02, "A7: LRACK byte == 0x02 (format error)");
        }
    }
    // -- S2F38_EnableDisableEventReportAcknowledge: format-error path (empty
    //    SReceiveData) -> real EDA(0x02) -- success path deliberately NOT
    //    exercised (reaches EnableDisableEventReport -> SaveEventReportData). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.4", 4904);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S2F38_EnableDisableEventReportAcknowledge();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "A8: S2F38 format-error (empty body) -> 17-byte EDA frame");
        if (tx.size() == 17)
        {
            CHECK((unsigned char)tx[6] == 2 && (unsigned char)tx[7] == 38, "A8: reply S,F == 2,38");
            CHECK((unsigned char)tx[16] == 0x02, "A8: EDA byte == 0x02 (format error)");
        }
    }
    // -- S5F4_EnableDisableAlarmAcknowledge: real success path (len==0 ->
    //    EnableDisableAlarmAll -> WriteAlamData under a scratch GemSystemPath,
    //    never a production path -- SAFE, unlike SaveEventReportData). --
    {
        const AnsiString kScratchDir = "uHGemEquipment_test_scratch";
        ForceDirectories(kScratchDir);

        THGem g;
        g.GemSystemPath = kScratchDir;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.5", 4905);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.strGrdAlarm->Cells[7][1] = "0";   // pre-seed one disabled alarm row

        // <L,2 <BINARY ALED=0x80 (enable)> <L,0>> -- len==0 -> "all" branch.
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0x80));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(0));

        hgem.S5F4_EnableDisableAlarmAcknowledge();
        CHECK(g.strGrdAlarm->Cells[7][1] == "1", "A9: EnableDisableAlarmAll flips strGrdAlarm col7 to \"1\" (real HGemPtr-side effect)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17, "A9: S5F4 success -> 17-byte LocalAcknowledge(5,4,0) frame");
        if (tx.size() == 17)
        {
            CHECK((unsigned char)tx[6] == 5 && (unsigned char)tx[7] == 4, "A9: reply S,F == 5,4");
            CHECK((unsigned char)tx[16] == 0x00, "A9: ack byte == 0x00 (accept)");
        }
        CHECK(FileExists(kScratchDir + "\\AlarmData.def"), "A9: EnableDisableAlarmAll's WriteAlamData wrote under the scratch folder (not production)");
    }
    // -- S5F6_ListAlarmData: "alarm not found" branch -- proves the golden
    //    zero-length-item quirk is reachable/safe (documented verbatim at
    //    the method's own definition; not re-verified byte-for-byte here). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.4.6", 4906);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        // <U4,1 999> -- one SVID (999), never registered in strGrdAlarm ->
        // GetAlarmIndex returns -1 -> the "not found" branch. (Both of
        // S5F6's own GetDataItemLenAndType peeks read the SAME [type,len]
        // token pair without consuming it -- only the final DataItemIn call
        // is destructive -- so exactly ONE [type,len,value] token group is
        // seeded, not two.)
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(999));

        bool threw = false;
        try { hgem.S5F6_ListAlarmData(); } catch (...) { threw = true; }
        CHECK(threw == false, "A10: S5F6_ListAlarmData with an unregistered SVID does not throw/crash");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14 && (unsigned char)tx[6] == 5 && (unsigned char)tx[7] == 6,
              "A10: real S5F6 reply sent (S,F == 5,6)");
    }
}

// ===========================================================================
//  [W906-uHGemClass-Micro5] Real THGem-backed coverage for the 4
//  uHGemClass.cpp methods un-gated this wave (see that file's own
//  "INTEGRATE WAVE 6" note): Process_S7F20_CurrentEPPIDData/S101F2/S101F4/
//  S125F2.
//
//  WIRING: same `THGem g; HTGem hgem; hgem.HGemPtr=&g; hgem.ActiveWire=
//  &g.WireCodec;` pattern as test_w906_alarmreportack_e2e above.
// ===========================================================================
static void test_w906_uhgemclass_micro5_e2e()
{
    printf("\n[W906-uHGemClass-Micro5] Process_S7F20/S101F2/S101F4/S125F2 real THGem-backed coverage\n");

    // -- S101F2_CurrentEPPDData: early-return path (SV_70_UNT1_ReceipeStruct
    //    defaults to 0, so !=1) -> real empty-list reply on S,F==101,2 --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.1", 6001);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S101F2_CurrentEPPDData();
        const std::vector<char> &tx1 = conn->SimTxBuffer();
        CHECK(tx1.size() >= 14, "M1: S101F2 early-return produced real bytes on the wire");
        if (tx1.size() >= 14)
            CHECK((unsigned char)tx1[6] == 101 && (unsigned char)tx1[7] == 2,
                  "M1: early-return reply S,F == 101,2");

        // -- success path: SV_70_UNT1_ReceipeStruct==1, real UploadFileString/
        //    SV_71_ASCII_FilenameExtened content echoed back --
        conn->SimClearTx();
        g.SV_70_UNT1_ReceipeStruct = 1;
        g.SV_71_ASCII_FilenameExtened = "RECIPE01.INI";
        g.UploadFileString->Add("FILE_A.INI");
        g.UploadFileString->Add("FILE_B.INI");

        hgem.S101F2_CurrentEPPDData();
        const std::vector<char> &tx2 = conn->SimTxBuffer();
        CHECK(tx2.size() >= 14, "M2: S101F2 success path produced real bytes on the wire");
        if (tx2.size() >= 14)
            CHECK((unsigned char)tx2[6] == 101 && (unsigned char)tx2[7] == 2,
                  "M2: success-path reply S,F == 101,2");
        std::string body2(tx2.begin() + 14, tx2.end());
        CHECK(body2.find("RECIPE01.INI") != std::string::npos,
              "M2: reply payload carries SV_71_ASCII_FilenameExtened (\"RECIPE01.INI\")");
        CHECK(body2.find("FILE_A.INI") != std::string::npos && body2.find("FILE_B.INI") != std::string::npos,
              "M2: reply payload carries both UploadFileString entries");
    }

    // -- S101F4_CurrentEPPDData: early-return path (SV_70_UNT1_ReceipeStruct
    //    defaults to 0, so !=2) -- GOLDEN BUG check: reply header is STILL
    //    101,2 (S101F2's header), never its own 101,4. --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.2", 6002);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S101F4_CurrentEPPDData();
        const std::vector<char> &tx1 = conn->SimTxBuffer();
        CHECK(tx1.size() >= 14, "M3: S101F4 early-return produced real bytes on the wire");
        if (tx1.size() >= 14)
            CHECK((unsigned char)tx1[6] == 101 && (unsigned char)tx1[7] == 2,
                  "M3: GOLDEN BUG preserved -- early-return reply is S,F==101,2 (S101F2's header), never 101,4");

        // -- success path: SV_70_UNT1_ReceipeStruct==2 -- header is STILL
        //    101,2 (the same golden bug), but the BODY carries this call's
        //    own real content. --
        conn->SimClearTx();
        g.SV_70_UNT1_ReceipeStruct = 2;
        g.SV_71_ASCII_FilenameExtened = "RECIPE02.INI";
        g.UploadFileString->Add("FILE_C.INI");

        hgem.S101F4_CurrentEPPDData();
        const std::vector<char> &tx2 = conn->SimTxBuffer();
        CHECK(tx2.size() >= 14, "M4: S101F4 success path produced real bytes on the wire");
        if (tx2.size() >= 14)
            CHECK((unsigned char)tx2[6] == 101 && (unsigned char)tx2[7] == 2,
                  "M4: GOLDEN BUG preserved -- success-path reply is ALSO S,F==101,2, never its own 101,4");
        std::string body2(tx2.begin() + 14, tx2.end());
        CHECK(body2.find("RECIPE02.INI") != std::string::npos,
              "M4: reply payload carries SV_71_ASCII_FilenameExtened (\"RECIPE02.INI\")");
        CHECK(body2.find("FILE_C.INI") != std::string::npos,
              "M4: reply payload carries the UploadFileString entry");
    }

    // -- Process_S7F20_CurrentEPPIDData: golden NULL-guard -- when
    //    GemRemoteReceipeList==NULL (its ctor default), the function must
    //    return BEFORE ever touching the wire (no token consumed). --
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        // g.GemRemoteReceipeList stays NULL here -- deliberately not allocated.

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        int countBefore = g.WireCodec.SReceiveData->Count;

        bool threw = false;
        try { hgem.Process_S7F20_CurrentEPPIDData(); } catch (...) { threw = true; }
        CHECK(threw == false, "M5: Process_S7F20 with GemRemoteReceipeList==NULL does not throw/crash");
        CHECK(g.WireCodec.SReceiveData->Count == countBefore,
              "M5: NULL-guard returns before touching the wire (no token consumed)");
    }
    // -- Process_S7F20_CurrentEPPIDData: non-NULL path -- <L,2 <ASCII "AAA">
    //    <ASCII "BBB">> populates GemRemoteReceipeList->Items in order. --
    {
        THGem g;
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.GemRemoteReceipeList = new THGemListBox();

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("AAA"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("BBB"));

        hgem.Process_S7F20_CurrentEPPIDData();
        CHECK(g.GemRemoteReceipeList->Items->Count == 2,
              "M6: Process_S7F20 non-NULL path populates GemRemoteReceipeList with 2 items");
        if (g.GemRemoteReceipeList->Items->Count == 2)
        {
            CHECK(g.GemRemoteReceipeList->Items->Strings[0] == "AAA", "M6: item[0] == \"AAA\"");
            CHECK(g.GemRemoteReceipeList->Items->Strings[1] == "BBB", "M6: item[1] == \"BBB\"");
        }

        delete g.GemRemoteReceipeList;
        g.GemRemoteReceipeList = NULL;
    }

    // -- S125F2_EnableDisableECDataAcknowledge: format-error path (empty
    //    SReceiveData) -> real S9F7_IllegalData reply (S,F==9,7). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.3", 6003);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        hgem.S125F2_EnableDisableECDataAcknowledge();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "M7: S125F2 format-error path produced real bytes on the wire");
        if (tx.size() >= 14)
            CHECK((unsigned char)tx[6] == 9 && (unsigned char)tx[7] == 7,
                  "M7: format-error path falls to S9F7_IllegalData (S,F==9,7)");
    }
    // -- S125F2: "all" branch (<L,2 <BINARY ALED=0x80(enable)> <L,0>>) --
    //    real EnableDisableECDataAll side effect on sgSECSECData + scratch
    //    WriteECEnableData file write + LocalAcknowledge(125,2,0). --
    {
        const AnsiString kScratchDir = "uHGemClass_micro5_test_scratch";
        ForceDirectories(kScratchDir);

        THGem g;
        g.GemSystemPath = kScratchDir;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.4", 6004);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.sgSECSECData->RowCount = 2;
        g.sgSECSECData->Cells[1][1] = "100";
        g.sgSECSECData->Cells[2][1] = "0";   // pre-seed disabled

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0x80));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(0));

        hgem.S125F2_EnableDisableECDataAcknowledge();
        CHECK(g.sgSECSECData->Cells[2][1] == "1",
              "M8: EnableDisableECDataAll flips sgSECSECData col2 to \"1\" (real HGemPtr-side effect)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "M8: S125F2 \"all\" branch produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 125 && (unsigned char)tx[7] == 2, "M8: reply S,F == 125,2");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "M8: ack byte == 0x00 (accept)");
        }
        CHECK(FileExists(kScratchDir + "\\ECEnableData.def"),
              "M8: EnableDisableECDataAll's WriteECEnableData wrote under the scratch folder (not production)");
    }
    // -- S125F2: single-ID branch, ID found -> EnableDisableECData flips the
    //    matching row + LocalAcknowledge(125,2,0). --
    {
        const AnsiString kScratchDir = "uHGemClass_micro5_test_scratch";
        ForceDirectories(kScratchDir);

        THGem g;
        g.GemSystemPath = kScratchDir;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.5", 6005);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.sgSECSECData->RowCount = 2;
        g.sgSECSECData->Cells[1][1] = "200";
        g.sgSECSECData->Cells[2][1] = "0";

        // <L,2 <BINARY ALED=0x80(enable)> <L,1 <ASCII "200">>>
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0x80));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("200"));

        hgem.S125F2_EnableDisableECDataAcknowledge();
        CHECK(g.sgSECSECData->Cells[2][1] == "1",
              "M9: EnableDisableECData flips the matching row (ID \"200\" found)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14 && (unsigned char)tx[6] == 125 && (unsigned char)tx[7] == 2,
              "M9: single-ID found -> real S125F2 reply (S,F == 125,2)");
        if (tx.size() >= 14)
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "M9: ack byte == 0x00 (accept, ID found)");
    }
    // -- S125F2: single-ID branch, ID NOT found -> EnableDisableECData
    //    returns false -> LocalAcknowledge(125,2,1) (reject). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.6", 6006);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.sgSECSECData->RowCount = 2;
        g.sgSECSECData->Cells[1][1] = "300";
        g.sgSECSECData->Cells[2][1] = "0";

        // <L,2 <BINARY ALED=0x80(enable)> <L,1 <ASCII "999">>> -- "999" is
        // never registered -> not-found branch (no file write reached).
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0x80));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(3));
        g.WireCodec.SReceiveData->Add(AnsiString("999"));

        hgem.S125F2_EnableDisableECDataAcknowledge();
        CHECK(g.sgSECSECData->Cells[2][1] == "0",
              "M10: unmatched ID leaves sgSECSECData untouched");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14 && (unsigned char)tx[6] == 125 && (unsigned char)tx[7] == 2,
              "M10: single-ID not-found -> real S125F2 reply (S,F == 125,2)");
        if (tx.size() >= 14)
            CHECK((unsigned char)tx[tx.size() - 1] == 0x01, "M10: ack byte == 0x01 (reject, ID not found)");
    }
}

// ===========================================================================
//  [W906-uHGemClass-Micro6] Real THGem-backed coverage for the 2
//  uHGemClass.cpp methods un-gated this wave (see that file's own
//  "INTEGRATE WAVE 7" note): S6F24_RequestSpooledDataAcknowledgementSend /
//  S7F18_DeleteProcessProgramAcknowledge.
//
//  WIRING: same `THGem g; HTGem hgem; hgem.HGemPtr=&g; hgem.ActiveWire=
//  &g.WireCodec;` pattern as test_w906_uhgemclass_micro5_e2e above.
//
//  SAFETY: S7F18's "currently open recipe" guard reads the real global
//  `GetLastOpenFN()`, which by default reads the production
//  D:\HT9045\SetUp.inf-backed `LastDataPath`. Following this file's own
//  SAFETY NOTE (top of file) + test_common.cpp's own established technique,
//  `LastDataPath` is repointed to a scratch file for the whole duration of
//  this test and restored at the end -- the real production file is never
//  touched.
// ===========================================================================
static void test_w906_uhgemclass_micro6_e2e()
{
    printf("\n[W906-uHGemClass-Micro6] S6F24/S7F18 real THGem-backed coverage\n");

    // -- S6F24: bSpoolActive==false (ctor default -- the safe no-op path this
    //    wave's design deliberately leaves as the only reachable behavior
    //    until some future "DoSpool subsystem" wave). Early-returns BEFORE
    //    touching bBeginTransferSpool or the input queue, but still sends a
    //    real LocalAcknowledge(6,24,0) reply. --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.10", 6010);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        CHECK(g.bSpoolActive == false, "N1: bSpoolActive defaults to false (ctor)");
        hgem.S6F24_RequestSpooledDataAcknowledgementSend();
        CHECK(g.bBeginTransferSpool == false,
              "N1: bSpoolActive==false path never touches bBeginTransferSpool");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N1: S6F24 no-op path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 6 && (unsigned char)tx[7] == 24, "N1: reply S,F == 6,24");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "N1: ack byte == 0x00");
        }
    }

    // -- S6F24: bSpoolActive==true, RSDC==0 -- the spool-wipe `system("del
    //    ...")` call is NOT reached (only RSDC==1 triggers it -- deliberately
    //    not exercised here, a real shell command has no place in this
    //    binary's own test run). Real bBeginTransferSpool latch + real
    //    DataItemIn consumption of the queued token + same unconditional
    //    ack==0 golden always sends from every branch of this method. --
    {
        THGem g;
        g.bSpoolActive = true;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.11", 6011);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.UINT_1_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0));   // RSDC = 0 -> skip system()
        int countBefore = g.WireCodec.SReceiveData->Count;

        hgem.S6F24_RequestSpooledDataAcknowledgementSend();
        CHECK(g.bBeginTransferSpool == true, "N2: bSpoolActive==true path latches bBeginTransferSpool=true");
        CHECK(g.WireCodec.SReceiveData->Count == countBefore - 3,
              "N2: DataItemIn really consumed the queued UINT_1 RSDC token (3 tokens: Type/Len/Value)");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N2: S6F24 RSDC==0 path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 6 && (unsigned char)tx[7] == 24, "N2: reply S,F == 6,24");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00,
                  "N2: ack byte == 0x00 (same as N1 -- S6F24 never sends a reject code on any branch)");
        }
    }

    // ---- S7F18 block: repoint LastDataPath for the whole block, restore at
    // the end (see this function's own SAFETY comment above). ----
    AnsiString savedLastDataPath = LastDataPath;
    const AnsiString kScratchDir = "uHGemClass_micro6_test_scratch";
    ForceDirectories(kScratchDir);
    LastDataPath = kScratchDir + "\\lastdata.inf";
    DeleteFile(LastDataPath);   // ensure GetLastOpenFN() starts from the "Fail Open" default

    // -- S7F18: happy path -- UpLoadPath/PPID directory exists (with real
    //    nested files under it) -> real DeleteDirectory() removes the whole
    //    tree -> LocalAcknowledge(7,18,0). --
    {
        const AnsiString kUpLoad = kScratchDir + "\\upload_happy";
        const AnsiString kRecipeDir = kUpLoad + "\\RECIPE_A";
        ForceDirectories(kRecipeDir + "\\sub");
        {
            FILE *f = fopen((kRecipeDir + "\\sub\\payload.txt").c_str(), "wb");
            if (f) { fputs("x", f); fclose(f); }
        }
        CHECK(DirectoryExists(kRecipeDir) == true, "N3: scratch recipe dir exists before S7F18");

        THGem g;
        g.UpLoadPath = kUpLoad;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.12", 6012);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(8));
        g.WireCodec.SReceiveData->Add(AnsiString("RECIPE_A"));

        hgem.S7F18_DeleteProcessProgramAcknowledge();
        CHECK(DirectoryExists(kRecipeDir) == false, "N3: S7F18 really deleted the scratch recipe dir");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N3: S7F18 happy path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 7 && (unsigned char)tx[7] == 18, "N3: reply S,F == 7,18");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "N3: ack byte == 0x00 (accept, deleted)");
        }
    }

    // -- S7F18: DirectoryExists==false branch -- PPID names a folder that
    //    does not exist under UpLoadPath -> LocalAcknowledge(7,18,4), and
    //    DeleteDirectory is never even called. --
    {
        const AnsiString kUpLoad = kScratchDir + "\\upload_missing";
        ForceDirectories(kUpLoad);

        THGem g;
        g.UpLoadPath = kUpLoad;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.13", 6013);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(14));
        g.WireCodec.SReceiveData->Add(AnsiString("NO_SUCH_RECIPE"));

        hgem.S7F18_DeleteProcessProgramAcknowledge();
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N4: S7F18 not-exist path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 7 && (unsigned char)tx[7] == 18, "N4: reply S,F == 7,18");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x04,
                  "N4: ack byte == 0x04 (reject, work-file directory does not exist)");
        }
    }

    // -- S7F18: golden quirk -- "currently open recipe" guard. When
    //    GetLastOpenFN()==PPID, LocalAcknowledge(7,18,1) fires BEFORE
    //    DirectoryExists/DeleteDirectory are even reached -- so a directory
    //    that genuinely exists on disk under UpLoadPath survives the call
    //    untouched, purely because it is "currently open". --
    {
        WriteLastDataFN(AnsiString("LOCKED_RECIPE"));
        CHECK(GetLastOpenFN() == AnsiString("LOCKED_RECIPE"),
              "N5 setup: GetLastOpenFN() now returns the repointed \"LOCKED_RECIPE\"");

        const AnsiString kUpLoad = kScratchDir + "\\upload_locked";
        const AnsiString kRecipeDir = kUpLoad + "\\LOCKED_RECIPE";
        ForceDirectories(kRecipeDir);
        CHECK(DirectoryExists(kRecipeDir) == true, "N5: scratch \"locked\" recipe dir exists before S7F18");

        THGem g;
        g.UpLoadPath = kUpLoad;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.14", 6014);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(13));
        g.WireCodec.SReceiveData->Add(AnsiString("LOCKED_RECIPE"));

        hgem.S7F18_DeleteProcessProgramAcknowledge();
        CHECK(DirectoryExists(kRecipeDir) == true,
              "N5: GOLDEN QUIRK -- the \"currently open\" guard fires before DeleteDirectory ever runs, "
              "so the real directory survives untouched");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N5: S7F18 \"currently open\" guard path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 7 && (unsigned char)tx[7] == 18, "N5: reply S,F == 7,18");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x01,
                  "N5: ack byte == 0x01 (reject, currently-open work file cannot be deleted)");
        }
    }

    // -- S7F18: multi-PPID L,n list -- proves the `for(i<slen)` loop really
    //    iterates and deletes EACH listed recipe directory, ending in a
    //    single LocalAcknowledge(7,18,0) for the whole batch. --
    {
        const AnsiString kUpLoad = kScratchDir + "\\upload_multi";
        const AnsiString kDirB = kUpLoad + "\\RECIPE_B";
        const AnsiString kDirC = kUpLoad + "\\RECIPE_C";
        ForceDirectories(kDirB);
        ForceDirectories(kDirC);
        CHECK(DirectoryExists(kDirB) == true && DirectoryExists(kDirC) == true,
              "N6: both scratch recipe dirs exist before S7F18");

        THGem g;
        g.UpLoadPath = kUpLoad;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.6.15", 6015);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(8));
        g.WireCodec.SReceiveData->Add(AnsiString("RECIPE_B"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(8));
        g.WireCodec.SReceiveData->Add(AnsiString("RECIPE_C"));

        hgem.S7F18_DeleteProcessProgramAcknowledge();
        CHECK(DirectoryExists(kDirB) == false, "N6: RECIPE_B deleted by the multi-item loop");
        CHECK(DirectoryExists(kDirC) == false, "N6: RECIPE_C deleted by the multi-item loop");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "N6: S7F18 multi-item path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 7 && (unsigned char)tx[7] == 18, "N6: reply S,F == 7,18");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "N6: ack byte == 0x00 (accept, both deleted)");
        }
    }

    DeleteFile(LastDataPath);
    LastDataPath = savedLastDataPath;
}

// ===========================================================================
//  [W906-uHGemClass-Micro7] Real THGem-backed coverage for the 4
//  uHGemClass.cpp methods un-gated this wave (see that file's own
//  "INTEGRATE WAVE 8" note): S10F4_TerminalDisplaySingleAcknowledge /
//  S10F6_TerminalDisplayMultiBlockAcknowledge /
//  S101F6_StoreHostUploadFile / S101F8_StoreHostUploadFile (the two Subs
//  themselves -- their wrappers S101F6()/S101F8() were already real and
//  already covered by test_w906_svecdataitem_e2e_s101f6_s101f8_smoke's
//  empty-request "Sub never entered" path; THIS block is what actually
//  drives the Subs' real bodies, including the flagged double-ack bug).
//
//  WIRING: same `THGem g; HTGem hgem; hgem.HGemPtr=&g; hgem.ActiveWire=
//  &g.WireCodec;` pattern as test_w906_uhgemclass_micro5/6_e2e above.
//
//  SAFETY: all file I/O below stays under a dedicated scratch directory
//  (kScratchDir), mirroring test_w906_uhgemclass_micro6_e2e's own
//  UpLoadPath-repointing convention just above -- never a production path.
// ===========================================================================
static void test_w906_uhgemclass_micro7_e2e()
{
    printf("\n[W906-uHGemClass-Micro7] S10F4/S10F6/S101F6_StoreHostUploadFile/S101F8_StoreHostUploadFile real THGem-backed coverage\n");

    const AnsiString kScratchDir = "uHGemClass_micro7_test_scratch";
    ForceDirectories(kScratchDir);

    // -- S10F4: guard-off path -- TerminalDisplayIndex==0 (its ctor default,
    //    the "NULL"/unset case) -> iDisplay=2, the whole
    //    Terminal*Ptr-touching body is skipped entirely (so no pointer is
    //    ever dereferenced while unset -- proves the guard, not just "it
    //    doesn't crash"). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.1", 7001);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.Remote.W_Bit = 1;

        bool threw = false;
        try { hgem.S10F4_TerminalDisplaySingleAcknowledge(); } catch (...) { threw = true; }
        CHECK(threw == false, "P1: S10F4 with TerminalDisplayIndex==0 (ctor default) does not throw/crash");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "P1: S10F4 guard-off path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 10 && (unsigned char)tx[7] == 4, "P1: reply S,F == 10,4");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x02,
                  "P1: ack byte == 0x02 (iDisplay==2, TerminalDisplayIndex==0 guard branch)");
        }
    }

    // -- S10F4: real "populated" success path -- TerminalDisplayIndex==1
    //    (memo) + TerminalDisplayIndex2==2 (listbox), BOTH really wired ->
    //    both the primary Terminal write AND the "2"-suffixed mirror write
    //    happen in one call (iDisplay==0 unlocks the second block); real
    //    SecsAlarmMessage content too. Uses CUSTOMER_CODE's real default (0,
    //    != CC_MAXIM_THAILAND==860) -- the plain (non-KYEC) branch. --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.2", 7002);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.Remote.W_Bit = 1;
        g.TerminalDisplayIndex = 1;
        g.TerminalMemoPtr = new THGemMemo();
        g.TerminalDisplayIndex2 = 2;
        g.TerminalListboxPtr2 = new THGemListBox();

        // <L,2 <BINARY uint1EC=0> <ASCII "HELLO">>
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString("HELLO"));

        hgem.S10F4_TerminalDisplaySingleAcknowledge();
        CHECK(hgem.SecsAlarmMessage->Count == 1 && hgem.SecsAlarmMessage->Strings[0] == "HELLO",
              "P2: real SecsAlarmMessage gets the plain (non-KYEC) \"HELLO\" content");
        CHECK(g.TerminalMemoPtr->Lines->Count == 1,
              "P2: primary TerminalDisplayIndex==1 path really wrote TerminalMemoPtr->Lines");
        if (g.TerminalMemoPtr->Lines->Count == 1)
            CHECK(AnsiString(g.TerminalMemoPtr->Lines->Strings[0]).Pos("HELLO") > 0,
                  "P2: TerminalMemoPtr's written line carries the \"HELLO\" payload");
        CHECK(g.TerminalListboxPtr2->Items->Count == 1,
              "P2: iDisplay==0 unlocked the \"2\"-suffixed mirror -- TerminalListboxPtr2->Items really got the SAME line");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "P2: S10F4 success path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 10 && (unsigned char)tx[7] == 4, "P2: reply S,F == 10,4");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "P2: ack byte == 0x00 (iDisplay==0, accept)");
        }
    }

    // -- S10F4: CUSTOMER_CODE==CC_MAXIM_THAILAND branch -- proves the
    //    FMessageList-numbered ("1. <text>\r\n") path really runs and lands
    //    in SecsAlarmMessage, via TerminalDisplayIndex==2 (listbox). Global
    //    CUSTOMER_CODE is saved/restored around this one block only. --
    {
        int savedCustomerCode = CUSTOMER_CODE;
        CUSTOMER_CODE = CC_MAXIM_THAILAND;

        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.3", 7003);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.Remote.W_Bit = 1;
        g.TerminalDisplayIndex = 2;
        g.TerminalListboxPtr = new THGemListBox();

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("MAXIM1"));

        hgem.S10F4_TerminalDisplaySingleAcknowledge();
        CHECK(hgem.SecsAlarmMessage->Count == 1, "P3: CC_MAXIM_THAILAND branch adds exactly 1 SecsAlarmMessage entry");
        if (hgem.SecsAlarmMessage->Count == 1)
        {
            const AnsiString &msg = hgem.SecsAlarmMessage->Strings[0];
            CHECK(msg.Pos("1. ") > 0 && msg.Pos("MAXIM1") > 0,
                  "P3: FMessageList-numbered (\"1. <text>\") content really lands in SecsAlarmMessage");
        }
        CHECK(g.TerminalListboxPtr->Items->Count == 1, "P3: TerminalDisplayIndex==2 path really wrote TerminalListboxPtr->Items");

        CUSTOMER_CODE = savedCustomerCode;
    }

    // -- S10F6: guard-off path -- TerminalDisplayIndex==0 -> iDisplay=2,
    //    same guard shape as S10F4's own P1 above (shared member). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.4", 7004);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.Remote.W_Bit = 1;

        bool threw = false;
        try { hgem.S10F6_TerminalDisplayMultiBlockAcknowledge(); } catch (...) { threw = true; }
        CHECK(threw == false, "Q1: S10F6 with TerminalDisplayIndex==0 (ctor default) does not throw/crash");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "Q1: S10F6 guard-off path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 10 && (unsigned char)tx[7] == 6, "Q1: reply S,F == 10,6");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x02, "Q1: ack byte == 0x02 (iDisplay==2, guard branch)");
        }
    }

    // -- S10F6: real multi-block success path -- <L,2 <BINARY uint1EC=0>
    //    <L,2 <ASCII "LINE1"> <ASCII "LINE2">>> -> real SecsAlarmMessage
    //    joined content "LINE1\r\nLINE2\r\n", LocalAcknowledge(10,6,0). Only
    //    needs TerminalDisplayIndex -- does NOT touch any TerminalMemoPtr/
    //    TerminalListboxPtr/TerminalEditPtr/TerminalPanelPtr(+2). --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.5", 7005);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;
        g.WireCodec.Remote.W_Bit = 1;
        g.TerminalDisplayIndex = 1;   // any non-zero value opens the guard; body below never touches TerminalMemoPtr

        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(0));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(2));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString("LINE1"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString("LINE2"));

        hgem.S10F6_TerminalDisplayMultiBlockAcknowledge();
        CHECK(hgem.SecsAlarmMessage->Count == 1, "Q2: S10F6 success path adds exactly 1 SecsAlarmMessage entry");
        if (hgem.SecsAlarmMessage->Count == 1)
            CHECK(hgem.SecsAlarmMessage->Strings[0] == AnsiString("LINE1\r\nLINE2\r\n"),
                  "Q2: real multi-block loop joins BOTH items with \\r\\n, exactly as golden");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() >= 14, "Q2: S10F6 success path produced real bytes on the wire");
        if (tx.size() >= 14)
        {
            CHECK((unsigned char)tx[6] == 10 && (unsigned char)tx[7] == 6, "Q2: reply S,F == 10,6");
            CHECK((unsigned char)tx[tx.size() - 1] == 0x00, "Q2: ack byte == 0x00 (iDisplay==0, accept)");
        }
    }

    // -- S101F6_StoreHostUploadFile (via its real wrapper HTGem::S101F6()):
    //    success path -- real file write under a scratch UpLoadPath, real
    //    GemRemoteReceipeList->Checked[i] flip true->false, real
    //    bFinishDownloadFile latch, and the GOLDEN DOUBLE-ACK bug: TWO
    //    (101,6,0) accept frames land on the wire for this ONE call, not
    //    one -- the one golden-bug-preservation claim this wave most needs a
    //    concrete test for (not just "it builds"). --
    {
        const AnsiString kUpLoadBase = kScratchDir + "\\upload_s101f6";
        // golden never creates the "HGem\\" subfolder itself in this method
        // (no ForceDirectories/MyForceDirectories call anywhere in
        // S101F6_StoreHostUploadFile) -- it is a real golden invariant that
        // UpLoadPath+"HGem\\" must already exist on disk before this runs.
        // Pre-create it here, matching that invariant (NOT "fixing" golden).
        ForceDirectories(kUpLoadBase + "\\HGem");

        THGem g;
        g.UpLoadPath = kUpLoadBase + "\\";   // golden concatenates "HGem\\" directly onto UpLoadPath with no separator -- UpLoadPath itself must already end with '\\'
        g.GemRemoteReceipeList = new THGemListBox();
        g.GemRemoteReceipeList->Items->Add("RECIPE_A");
        g.GemRemoteReceipeList->Checked[0] = true;   // pre-set true so the flip to false is observable
        static char payload[] = "DATA";
        g.WireCodec.DownLoadFilePtr = payload;

        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.6", 7006);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        // <L,5 <ASCII "RECIPE_A"> <I4 iStoreCT=1> <I4 iTotalCount=1> <I4 iFileCount=1>>
        // followed by the (out-of-band, raw-binary) <BINARY,4> header that
        // GetDataItemLenAndType peeks to learn `len` for fwrite -- see
        // S101F6_StoreHostUploadFile's own comment on DownLoadFilePtr.
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(8));
        g.WireCodec.SReceiveData->Add(AnsiString("RECIPE_A"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(4));

        bool threw = false;
        try { hgem.S101F6(); } catch (...) { threw = true; }
        CHECK(threw == false, "R1: S101F6() success path does not throw/crash");

        const AnsiString kOutFile = kUpLoadBase + "\\HGem\\RECIPE_A";
        CHECK(FileExists(kOutFile), "R1: S101F6_StoreHostUploadFile really wrote UpLoadPath+\"HGem\\\"+str under the scratch dir");
        if (FileExists(kOutFile))
        {
            FILE *rf = fopen(kOutFile.c_str(), "rb");
            char buf[16] = {0};
            size_t n = rf ? fread(buf, 1, sizeof(buf) - 1, rf) : 0;
            if (rf) fclose(rf);
            CHECK(n == 4 && memcmp(buf, "DATA", 4) == 0, "R1: written file content == the real DownLoadFilePtr payload (\"DATA\")");
        }
        CHECK(g.GemRemoteReceipeList->Checked.Values.size() >= 1 && g.GemRemoteReceipeList->Checked.Values[0] == false,
              "R1: GemRemoteReceipeList->Checked[0] really flipped true->false (SV_70_UNT1_ReceipeStruct==0 match on \"RECIPE_A\")");
        CHECK(g.bFinishDownloadFile == true, "R1: bFinishDownloadFile latches true (iFileCount<=1)");

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 34,
              "R1: GOLDEN DOUBLE-ACK BUG preserved -- exactly TWO 17-byte (101,6,0) frames on the wire for ONE successful S101F6() call, not one");
        if (tx.size() == 34)
        {
            CHECK((unsigned char)tx[6] == 101 && (unsigned char)tx[7] == 6 && (unsigned char)tx[16] == 0x00,
                  "R1: 1st ack frame == accept (101,6,0) -- sent by S101F6_StoreHostUploadFile's own internal LocalAcknowledge call");
            CHECK((unsigned char)tx[23] == 101 && (unsigned char)tx[24] == 6 && (unsigned char)tx[33] == 0x00,
                  "R1: 2nd ack frame == accept (101,6,0) -- sent unconditionally by HTGem::S101F6()'s own wrapper tail right after");
        }
    }

    // -- S101F8_StoreHostUploadFile (via its real wrapper HTGem::S101F8()):
    //    success path -- real MyForceDirectories-created subdirectory under
    //    a scratch CurrentDirectory, real file write, real
    //    GemRemoteReceipeList->Checked[i] flip true->false, and the
    //    documented ASYMMETRY: exactly ONE (101,8,0) accept frame on the
    //    wire (never two, unlike S101F6 above). --
    {
        const AnsiString kRoot = kScratchDir + "\\s101f8_root";

        THGem g;
        g.CurrentDirectory = kRoot;
        g.GemRemoteReceipeList = new THGemListBox();
        g.GemRemoteReceipeList->Items->Add("SUBDIR");
        g.GemRemoteReceipeList->Checked[0] = true;   // pre-set true so the flip to false is observable
        static char payload2[] = "DATA";
        g.WireCodec.DownLoadFilePtr = payload2;

        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.7.7", 7007);
        g.srvGem->Open();
        HTGem hgem;
        hgem.HGemPtr = &g;
        hgem.ActiveWire = &g.WireCodec;

        // <L,5 <ASCII "SUBDIR"> <ASCII "FILE.BIN"> <I4 iStoreCT=1> <I4 iTotalCount=1>>
        // followed by the same out-of-band <BINARY,4> peek header as R1 above.
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(5));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(6));
        g.WireCodec.SReceiveData->Add(AnsiString("SUBDIR"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(8));
        g.WireCodec.SReceiveData->Add(AnsiString("FILE.BIN"));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.INT_4_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString(1));
        g.WireCodec.SReceiveData->Add(AnsiString((int)HType.BINARY_TYPE));
        g.WireCodec.SReceiveData->Add(AnsiString(4));

        bool threw = false;
        try { hgem.S101F8(); } catch (...) { threw = true; }
        CHECK(threw == false, "R2: S101F8() success path does not throw/crash");

        const AnsiString kOutFile = kRoot + "\\SUBDIR\\FILE.BIN";
        CHECK(DirectoryExists(kRoot + "\\SUBDIR"), "R2: MyForceDirectories really created CurrentDirectory+PathName under the scratch dir");
        CHECK(FileExists(kOutFile), "R2: S101F8_StoreHostUploadFile really wrote CurrentDirectory+PathName+\"\\\\\"+str");
        if (FileExists(kOutFile))
        {
            FILE *rf = fopen(kOutFile.c_str(), "rb");
            char buf[16] = {0};
            size_t n = rf ? fread(buf, 1, sizeof(buf) - 1, rf) : 0;
            if (rf) fclose(rf);
            CHECK(n == 4 && memcmp(buf, "DATA", 4) == 0, "R2: written file content == the real DownLoadFilePtr payload (\"DATA\")");
        }
        CHECK(g.GemRemoteReceipeList->Checked.Values.size() >= 1 && g.GemRemoteReceipeList->Checked.Values[0] == false,
              "R2: GemRemoteReceipeList->Checked[0] really flipped true->false (PathName==\"SUBDIR\" match)");

        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() == 17,
              "R2: ASYMMETRY vs S101F6 preserved -- exactly ONE 17-byte (101,8,0) frame on the wire, never two");
        if (tx.size() == 17)
            CHECK((unsigned char)tx[6] == 101 && (unsigned char)tx[7] == 8 && (unsigned char)tx[16] == 0x00,
                  "R2: sole ack frame == accept (101,8,0)");
    }
}

// ===========================================================================
//  AI(W906-DoDownLoadRemoteFile) 20260721: THGem::DoDownLoadRemoteFile --
//  direct THGem method calls only (no ProcessSocketReceiveData/Timer1Timer
//  pump), same posture as Micro5/Micro6/Micro7 above -- no TextLogSnapshot
//  bracket or special T10 ordering needed either.
// ===========================================================================
static void test_w906_dodownloadremotefile_e2e()
{
    printf("\n[W906-DoDownLoadRemoteFile] S7F5 PP-Request retry state machine, real THGem-backed coverage\n");

    // -- D1: Task==1 (ctor default), empty queue -> stays 1 (no-op) --
    {
        THGem g;
        CHECK(g.iDownLoadRemoteFileTask == 1, "D1: ctor default Task == 1");
        CHECK(g.iRetryCTDownLoadRemoteFile == 0, "D1: ctor default retry counter == 0");
        CHECK(g.RequestRemoteDownLoad->Count == 0, "D1: ctor default queue is empty");
        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 1, "D1: empty queue -> Task stays 1 (no-op)");
    }

    // -- D2: case 1 -> 100 -> a real S7F5 request goes out on the wire --
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.5.1", 5001);
        g.srvGem->Open();
        g.RequestRemoteDownLoad->Add("RECIPE_A.PGM");

        g.DoDownLoadRemoteFile();   // case 1 -> Task=100 (iRetryCT reset)
        CHECK(g.iDownLoadRemoteFileTask == 100, "D2: non-empty queue -> Task 1->100");

        g.DoDownLoadRemoteFile();   // case 100 -> sends S7F5, Task=200
        CHECK(g.iDownLoadRemoteFileTask == 200, "D2: case 100 -> Task->200");
        CHECK(g.bReceiveS7F6 == false, "D2: case 100 clears bReceiveS7F6 before waiting for the reply");
        const std::vector<char> &tx = conn->SimTxBuffer();
        CHECK(tx.size() > 10, "D2: case 100 sends a real frame onto the wire");
        if (tx.size() > 10)
        {
            CHECK((unsigned char)(tx[6] & 0x7f) == 7, "D2: reply S == 7 (PP-Request family)");
            CHECK((unsigned char)tx[7] == 5, "D2: reply F == 5 (S7F5 PP-Request)");
            CHECK(((unsigned char)tx[6] & 0x80) != 0, "D2: W-bit set (InitLocalHead(7,5,1) -- expects an S7F6 reply)");
        }
        AnsiString frame(tx.data(), (int)tx.size());
        CHECK(frame.Pos("RECIPE_A.PGM") > 0, "D2: sent frame's data item carries the queued filename");
        CHECK(g.RequestRemoteDownLoad->Count == 1, "D2: case 100 does not dequeue yet (only case 200's success path does)");
    }

    // -- D3: case 200, bReceiveS7F6==true (host sent S7F6) -> dequeue + Task=300 --
    {
        THGem g;
        g.RequestRemoteDownLoad->Add("RECIPE_B.PGM");
        g.iDownLoadRemoteFileTask = 200;
        g.bReceiveS7F6 = true;

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 300, "D3: bReceiveS7F6==true -> Task 200->300");
        CHECK(g.bReceiveS7F6 == false, "D3: bReceiveS7F6 consumed (reset false)");
        CHECK(g.RequestRemoteDownLoad->Count == 0, "D3: success path dequeues the sent filename");
    }

    // -- D4: case 200, TimerOff()==false -- GOLDEN QUIRK: falls back to 100
    //    WITHOUT waiting for the 1-second window to elapse. The resend itself
    //    only happens on the *next* call (once Task==100 is actually
    //    dispatched) -- this method is a plain switch, not a loop.
    {
        THGem g;
        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.5.2", 5002);
        g.srvGem->Open();
        g.RequestRemoteDownLoad->Add("RECIPE_C.PGM");
        g.iDownLoadRemoteFileTask = 200;
        g.bReceiveS7F6 = false;
        g.DelayDownLoadRemoteFile.TimerSetSecAndOn(1);   // freshly armed -> TimerOff()==false

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 100,
              "D4: GOLDEN QUIRK -- TimerOff()==false falls Task back to 100 instead of continuing to wait");
        CHECK(conn->SimTxBuffer().size() == 0,
              "D4: this call's own case-200 body sends nothing (the resend happens on the NEXT pump call)");

        g.DoDownLoadRemoteFile();   // now dispatches case 100 -> immediate resend, Task=200 again
        CHECK(g.iDownLoadRemoteFileTask == 200, "D4: next pump call (Task==100) resends and returns to 200");
        CHECK(conn->SimTxBuffer().size() > 10,
              "D4: the resend fires on this 2nd call, well before the original 1s window could have elapsed");
    }

    // -- D5: case 200, TimerOff()==true and retry cap already exceeded -> gives up (Task=1) --
    {
        THGem g;
        g.RequestRemoteDownLoad->Add("RECIPE_D.PGM");
        g.iDownLoadRemoteFileTask = 200;
        g.bReceiveS7F6 = false;
        g.iRetryCTDownLoadRemoteFile = 6;   // already past the >5 cap
        g.DelayDownLoadRemoteFile.TimerSet(0);
        g.DelayDownLoadRemoteFile.TimerOn();   // iTimeLen<=0 -> TimerOff()==true immediately (no real sleep needed)

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 1, "D5: retry cap exceeded -> gives up, Task->1");
        CHECK(g.RequestRemoteDownLoad->Count == 0, "D5: give-up path also dequeues the stuck filename");
    }

    // -- D6: case 300, host sends S101F5 (or S101F7) -> stays 300, re-arms timer --
    {
        THGem g;
        g.iDownLoadRemoteFileTask = 300;
        g.DelayDownLoadRemoteFile.TimerSetSecAndOn(1);
        g.bReceiveS101F5 = true;

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 300, "D6: S101F5 receipt keeps Task at 300 (re-arms the timer, does not advance)");
        CHECK(g.bReceiveS101F5 == false, "D6: bReceiveS101F5 consumed (reset false)");
    }
    {
        THGem g;
        g.iDownLoadRemoteFileTask = 300;
        g.DelayDownLoadRemoteFile.TimerSetSecAndOn(1);
        g.bReceiveS101F7 = true;

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 300, "D6b: S101F7 receipt keeps Task at 300 (re-arms the timer, does not advance)");
        CHECK(g.bReceiveS101F7 == false, "D6b: bReceiveS101F7 consumed (reset false)");
    }

    // -- D7: case 300, TimerOff()==true and no S101F5/F7 -> whole cycle complete, Task=1 --
    {
        THGem g;
        g.iDownLoadRemoteFileTask = 300;
        g.bReceiveS101F5 = false;
        g.bReceiveS101F7 = false;
        g.DelayDownLoadRemoteFile.TimerSet(0);
        g.DelayDownLoadRemoteFile.TimerOn();   // iTimeLen<=0 -> TimerOff()==true immediately

        g.DoDownLoadRemoteFile();
        CHECK(g.iDownLoadRemoteFileTask == 1, "D7: case 300 window elapsed with no S101F5/F7 -> Task->1 (cycle complete)");
    }
}

// ===========================================================================
//  [W906-SpoolCluster] WriteToSpoolFile (golden :1887-1977) / DoSpool
//  (golden :4079-4189) / SetSpoolActive (golden :6133-6154) / GetSpoolActive
//  (golden :6158-6161) / DoSpoolSendLocalData (golden :4025-4074).
//
//  All file I/O below happens under dedicated scratch directories
//  (spoolcluster_test_scratch_p1..p4, relative to the test binary's own CWD)
//  -- never a production path, same discipline as this file's own SAFETY
//  NOTE at the top.
//
//  P4's synthetic spool "frame" is a minimal, VALID 14-byte header-only SECS
//  message (S0F0, no data items): Ptr[0..3] big-endian encode
//  (RunLength-4)==10, so DoSpoolSendLocalData computes RunLength==14 (header
//  size only) -- WireCodec::ShowSML/ProcessSML's own `if(RunLength>=Len)
//  return -1;` guard then stops immediately without reading past offset 14,
//  so this frame is safe to feed through the REAL send/show pipeline (no
//  out-of-bounds read, no crash) without needing a full SECS-II item body.
// ===========================================================================
static void test_w906_spoolcluster_e2e()
{
    printf("\n[W906-SpoolCluster] WriteToSpoolFile/DoSpool/SetSpoolActive/GetSpoolActive/DoSpoolSendLocalData, real THGem-backed coverage\n");

    // -- P1: GetSpoolActive/SetSpoolActive round-trip, empty spool dir --
    {
        THGem g;
        const AnsiString kDir = "spoolcluster_test_scratch_p1";
        ForceDirectories(kDir);
        g.GemSpoolPath = kDir;
        g.FileListBox1->Mask = kDir + AnsiString("\\*.dat");

        CHECK(g.GetSpoolActive() == false, "P1: ctor default bSpoolActive is false");
        g.SetSpoolActive(true);
        CHECK(g.GetSpoolActive() == true, "P1: SetSpoolActive(true) always takes effect (golden's confirm-dialog guard only gates Active==false)");

        // Active==false, empty spool dir -> Items->Count==0 -> confirm guard
        // skipped entirely -> falls straight through to bSpoolActive=false.
        g.SetSpoolActive(false);
        CHECK(g.GetSpoolActive() == false, "P1: SetSpoolActive(false) with an EMPTY spool dir succeeds (no confirm needed)");

        RemoveDir(kDir);
    }

    // -- P2: SetSpoolActive(false) with a NONEMPTY spool dir -- golden
    //    control-flow preserved verbatim: Gated_MessageDlgConfirmYes always
    //    returns false ("not confirmed"), so this returns WITHOUT ever
    //    assigning bSpoolActive -- it stays at whatever it was BEFORE the call.
    {
        THGem g;
        const AnsiString kDir = "spoolcluster_test_scratch_p2";
        ForceDirectories(kDir);
        g.GemSpoolPath = kDir;
        g.FileListBox1->Mask = kDir + AnsiString("\\*.dat");

        FILE *f = fopen((kDir + AnsiString("\\leftover.dat")).c_str(), "wb");
        CHECK(f != NULL, "P2: setup -- leftover spool file created");
        if (f) { fwrite("x", 1, 1, f); fclose(f); }

        g.SetSpoolActive(true);
        CHECK(g.GetSpoolActive() == true, "P2: primed bSpoolActive=true before the real assertion");

        g.SetSpoolActive(false);
        CHECK(g.GetSpoolActive() == true,
              "P2: GOLDEN QUIRK -- SetSpoolActive(false) with a nonempty spool dir and no real confirm dialog "
              "leaves bSpoolActive UNCHANGED (still true), because the (never-shown) confirm was answered No");

        DeleteFile(kDir + AnsiString("\\leftover.dat"));
        RemoveDir(kDir);
    }

    // -- P3: WriteToSpoolFile's conditional Refresh()/Update() -- only fires
    //    when FileListBox1->Items->Count==0 at entry, and seeds
    //    GemSpoolStartTime then (and only then). --
    {
        THGem g;
        const AnsiString kDir = "spoolcluster_test_scratch_p3";
        ForceDirectories(kDir);
        g.GemSpoolPath = kDir;
        g.FileListBox1->Mask = kDir + AnsiString("\\*.dat");
        g.GetTimeInfo();

        // Seed WireCodec.LocalBuffer/LocalLength_4 with a tiny real payload --
        // WriteToSpoolFile writes exactly LocalLength_4 bytes verbatim.
        g.WireCodec.LocalBuffer.assign(4, 0xAB);
        g.WireCodec.LocalLength_4 = 4;

        memset(g.GemSpoolStartTime, 0, sizeof(g.GemSpoolStartTime));
        CHECK(g.FileListBox1->Items->Count == 0, "P3: fresh scratch dir starts with Items->Count==0");

        g.WriteToSpoolFile();   // Count==0 at entry -> seeds GemSpoolStartTime + Refresh()/Update()
        CHECK(strlen(g.GemSpoolStartTime) == 14, "P3: first write (queue was empty) seeds GemSpoolStartTime (yyyyMMddHHmmss, 14 chars)");
        CHECK(g.FileListBox1->Items->Count == 1, "P3: WriteToSpoolFile's own Refresh()/Update() sees the just-written file");

        memset(g.GemSpoolStartTime, 0, sizeof(g.GemSpoolStartTime));   // clear, to prove the 2nd write does NOT reseed it
        g.WriteToSpoolFile();    // Count==1 (nonzero) at entry -> does NOT reseed GemSpoolStartTime
        CHECK(strlen(g.GemSpoolStartTime) == 0,
              "P3: second write (queue was already nonempty) does NOT touch GemSpoolStartTime -- conditional trigger confirmed");

        // cleanup: whatever *.dat files landed under kDir.
        g.FileListBox1->Refresh();
        for (int i = 0; i < g.FileListBox1->Items->Count; i++)
            DeleteFile(kDir + AnsiString("\\") + AnsiString(g.FileListBox1->Items->Strings[i]));
        RemoveDir(kDir);
    }

    // -- P4: DoSpool's FIFO consumption of Items->Strings[0] --
    {
        THGem g;
        const AnsiString kDir = "spoolcluster_test_scratch_p4";
        ForceDirectories(kDir);
        g.GemSpoolPath = kDir;
        g.FileListBox1->Mask = kDir + AnsiString("\\*.dat");

        unsigned char frame[14] = {0, 0, 0, 10,  0, 0,  0, 0,  0, 0,  0, 0, 0, 1};

        // 3 spool files, written in REVERSE chronological order on disk, to
        // prove DoSpool consumes the alphabetically/chronologically FIRST
        // one first -- not creation order.
        const char *names[3] = {
            "2026_07_21 11_00_00 000.dat",
            "2026_07_21 09_00_00 000.dat",
            "2026_07_21 10_00_00 000.dat",
        };
        for (int i = 0; i < 3; i++)
        {
            FILE *f = fopen((kDir + AnsiString("\\") + AnsiString(names[i])).c_str(), "wb");
            CHECK(f != NULL, "P4: setup -- spool test file created");
            if (f) { fwrite(frame, sizeof(frame), 1, f); fclose(f); }
        }

        TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.9.1", 5009);
        g.srvGem->Open();

        g.bSpoolActive = true;
        g.bBeginTransferSpool = true;
        g.bConnect = true;
        g.iSpoolTask = 1;
        g.GemSpoolCountActual = 3;   // matches the 3 files just seeded (real usage sets this via the bootstrap branches this test skips)

        g.DoSpool();   // case 1: picks Items->Strings[0], sends it, Task->100
        CHECK(g.iSpoolTask == 100, "P4: case 1 with a nonempty queue and bConnect==true -> Task 1->100");
        CHECK(conn->SimTxBuffer().size() == 14, "P4: case 1 sent exactly the 14-byte frame onto the wire");
        CHECK(FileExists(kDir + AnsiString("\\2026_07_21 09_00_00 000.dat")) == false,
              "P4: DoSpool consumed the CHRONOLOGICALLY-EARLIEST file first (09_00_00), not creation order");
        CHECK(FileExists(kDir + AnsiString("\\2026_07_21 10_00_00 000.dat")) == true,
              "P4: the 10_00_00 file is untouched -- still queued");
        CHECK(FileExists(kDir + AnsiString("\\2026_07_21 11_00_00 000.dat")) == true,
              "P4: the 11_00_00 file is untouched -- still queued");
        CHECK(g.GemSpoolCountActual == 2, "P4: GemSpoolCountActual decremented after consuming one file");
        conn->SimClearTx();

        g.DoSpool();   // case 100: frees SpoolPtr, Task->1 (no send, no file I/O)
        CHECK(g.iSpoolTask == 1, "P4: case 100 -> Task 100->1 (ready for the next file)");
        CHECK(conn->SimTxBuffer().size() == 0, "P4: case 100 sends nothing");

        g.DoSpool();   // case 1 again: picks the NEW Items->Strings[0] (10_00_00)
        CHECK(g.iSpoolTask == 100, "P4: case 1 (2nd file) -> Task 1->100 again");
        CHECK(conn->SimTxBuffer().size() == 14, "P4: case 1 (2nd file) sent exactly the 14-byte frame");
        CHECK(FileExists(kDir + AnsiString("\\2026_07_21 10_00_00 000.dat")) == false,
              "P4: DoSpool consumed the 2nd-earliest file (10_00_00) next -- FIFO order confirmed across 2 cycles");
        CHECK(FileExists(kDir + AnsiString("\\2026_07_21 11_00_00 000.dat")) == true,
              "P4: the 11_00_00 file is STILL untouched after 2 consume cycles");
        CHECK(g.GemSpoolCountActual == 1, "P4: GemSpoolCountActual decremented again");

        // cleanup: whatever *.dat file(s) remain under kDir.
        g.FileListBox1->Refresh();
        for (int i = 0; i < g.FileListBox1->Items->Count; i++)
            DeleteFile(kDir + AnsiString("\\") + AnsiString(g.FileListBox1->Items->Strings[i]));
        RemoveDir(kDir);
    }

    // -- P5: DoSpool's bSpoolActive==false branch -- bSpooling/
    //    bBeginTransferSpool reset, GemSpoolCountTotal/Actual re-synced from
    //    a fresh scan once SystemMin changes. Does NOT exercise the
    //    `del ...*.*/q/f` system() call's actual on-disk effect (that
    //    command string's own well-known missing-/q//f-spaces quirk, see
    //    DoSpool's own .cpp comment, makes its real deletion behavior
    //    OS-parsing-dependent) -- only the surrounding state transitions,
    //    which do not depend on whether the wipe itself actually ran.
    {
        THGem g;
        const AnsiString kDir = "spoolcluster_test_scratch_p5";
        ForceDirectories(kDir);
        g.GemSpoolPath = kDir;
        g.FileListBox1->Mask = kDir + AnsiString("\\*.dat");

        g.bSpoolActive = false;
        g.bSpooling = true;             // primed non-default, to prove DoSpool resets it
        g.bBeginTransferSpool = true;   // primed non-default, to prove DoSpool resets it
        g.OldSpoolSystemMin = static_cast<WORD>(g.SystemMin + 1);   // force != SystemMin, so the re-sync branch runs

        g.DoSpool();
        CHECK(g.bSpooling == false, "P5: bSpoolActive==false path resets bSpooling");
        CHECK(g.bBeginTransferSpool == false, "P5: bSpoolActive==false path resets bBeginTransferSpool");
        CHECK(g.OldSpoolSystemMin == g.SystemMin, "P5: OldSpoolSystemMin re-synced to the current SystemMin");
        CHECK(g.GemSpoolCountTotal == g.GemSpoolCountActual, "P5: GemSpoolCountTotal/Actual re-synced to the same (freshly re-scanned) value");

        // cleanup: whatever *.dat file(s) landed under kDir (none expected,
        // but the wipe's own on-disk effect is deliberately not asserted --
        // see this test's own file-head note).
        g.FileListBox1->Refresh();
        for (int i = 0; i < g.FileListBox1->Items->Count; i++)
            DeleteFile(kDir + AnsiString("\\") + AnsiString(g.FileListBox1->Items->Strings[i]));
        RemoveDir(kDir);
    }
}

// ===========================================================================
//  [T10] Timer1Timer's IniConfig.bEnable_SECS_GEM==false branch (forced
//  disconnect) + the SECSGEM_DoSeparateWait 5-second re-arm window.
//
//  IMPORTANT (flagged): Timer1Timer's own `bSendDoSeparate`/
//  `bSetDoSeparateWait`/`bWaitHTimer`/`bTimerRunning` are golden FUNCTION-STATIC
//  locals (persist across EVERY THGem instance AND every call, for the
//  process's entire lifetime -- a faithful translation of golden's own
//  shape, not something this wave changed). `bSECSGEM_DoSeparate`/
//  `SECSGEM_DoSeparate`/`SECSGEM_DoSeparateWait` are likewise genuine
//  file-scope globals (golden :24/:4746/:5175). This test is written to be
//  the FIRST (and only) one in this binary that ever drives
//  IniConfig.bEnable_SECS_GEM==false through Timer1Timer -- every earlier
//  Bucket-C test that pumps Timer1Timer (T3) runs with bEnable_SECS_GEM==true,
//  so the disable branch below is never entered by them and these
//  statics/globals are still at their process-start defaults when this test
//  begins. Re-ordering the test sequence in main() below could break this
//  assumption -- keep this test LAST among the Timer1Timer-pumping tests.
// ===========================================================================
static void test_timer1timer_disable_branch()
{
    printf("\n[T10] Timer1Timer disable branch (IniConfig.bEnable_SECS_GEM==false)\n");

    bool savedEnableSecsGem = IniConfig.bEnable_SECS_GEM;
    bool savedInitialOK = InitialOK;
    THGem *savedHGem = HGem;

    THGem g;
    BucketCPanels panels(g);
    HGem = &g;
    g.DB = new THGemMemo();
    InitialOK = true;

    TCustomWinSocket *conn = g.srvGem->SimAcceptConnection("10.0.0.30", 4600);
    g.srvGem->Open();
    g.clientGem->Active = true;   // so "both sockets closed" is observable on both

    IniConfig.bEnable_SECS_GEM = false;
    g.Timer1Timer(NULL);   // srvGem->Socket->ActiveConnections!=0 -> DoSeparate() + Close() both -> bSECSGEM_DoSeparate=true

    CHECK(conn->SimTxBuffer().size() == 14, "T10: disable branch sends a real Separate_req control frame");
    if (conn->SimTxBuffer().size() == 14)
        CHECK((unsigned char)conn->SimTxBuffer()[9] == 9, "T10: sent frame's SType byte == 9 (Separate_req)");
    CHECK(g.srvGem->Active == false && g.clientGem->Active == false,
          "T10: disable branch closes BOTH srvGem and clientGem");
    CHECK(bSECSGEM_DoSeparate == true, "T10: bSECSGEM_DoSeparate latches true after the forced disconnect");

    // Re-enable: Timer1Timer's own `if(bSetDoSeparateWait && bSECSGEM_DoSeparate)`
    // now arms SECSGEM_DoSeparateWait.SetSecAndOn(5) and takes the early-return
    // "bWaitHTimer" path -- sleepless variant: confirm the pump returns
    // immediately without advancing Timer1Task (the real 5-second wait, D6,
    // has not elapsed).
    IniConfig.bEnable_SECS_GEM = true;
    int taskBefore = g.Timer1Task;
    g.Timer1Timer(NULL);
    CHECK(g.Timer1Task == taskBefore,
          "T10: SECSGEM_DoSeparateWait's 5-second window blocks the pump -- Timer1Task does not advance on this call");

    HGem = savedHGem;
    InitialOK = savedInitialOK;
    IniConfig.bEnable_SECS_GEM = savedEnableSecsGem;
    delete g.DB; g.DB = NULL;
}

// ===========================================================================
//  AI(W906-uHGemEquipment-BucketC) 20260717: T3/T4/T5 (via
//  ProcessSocketReceiveData, directly or through Timer1Timer) all reach the
//  now-real SaveSECSGEMTextToLog(), which -- exactly like the
//  already-committed SaveSECSGEMErrToLog test [12] guards against -- APPENDS
//  to a real, hardcoded, outside-the-repo path:
//  D:\SECS_GEM_LOGS\<yyyy>\<mm_dd>\SECSGEM_TextLog_<hh>.txt. Per this wave's
//  own hard test-hygiene rule (any test reaching that logger must leave the
//  archive byte-for-byte unchanged afterward), this capture/restore pair
//  wraps the WHOLE Bucket-C block in main() below in ONE shot (restoring
//  after every individual SaveSECSGEMTextToLog call would be impractically
//  invasive) -- same technique as test [12]'s own inline capture/restore,
//  factored into two reusable functions here since it now brackets several
//  tests instead of just one.
// ===========================================================================
struct TextLogSnapshot
{
    AnsiString filePath;
    bool fileExistedBefore;
    bool dayDirExistedBefore;
    bool yearDirExistedBefore;
    std::string originalContent;
};

static TextLogSnapshot CaptureTextLogSnapshot()
{
    TextLogSnapshot snap;
    TDateTime tdNow = Now();
    AnsiString dirYear = "D:\\SECS_GEM_LOGS\\" + FormatDateTime("yyyy", tdNow);
    AnsiString dirDay = dirYear + "\\" + FormatDateTime("mm_dd", tdNow);
    snap.filePath = dirDay + "\\SECSGEM_TextLog_" + FormatDateTime("hh", tdNow) + ".txt";
    snap.fileExistedBefore = FileExists(snap.filePath);
    snap.dayDirExistedBefore = DirectoryExists(dirDay);
    snap.yearDirExistedBefore = DirectoryExists(dirYear);
    if (snap.fileExistedBefore)
    {
        FILE *rf = fopen(snap.filePath.c_str(), "rb");
        if (rf)
        {
            char buf[65536];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), rf)) > 0)
                snap.originalContent.append(buf, n);
            fclose(rf);
        }
    }
    return snap;
}

static void RestoreTextLogSnapshot(const TextLogSnapshot &snap)
{
    // Recompute the directory paths from the SAME captured filePath (avoids
    // re-deriving yyyy/mm_dd separately and risking an hour-boundary mismatch
    // between capture and restore -- same accepted latent limitation as
    // test [12]'s own inline version, which this mirrors).
    if (snap.fileExistedBefore)
    {
        FILE *wf = fopen(snap.filePath.c_str(), "wb");
        if (wf)
        {
            fwrite(snap.originalContent.data(), 1, snap.originalContent.size(), wf);
            fclose(wf);
        }
    }
    else
    {
        remove(snap.filePath.c_str());
        AnsiString dirDay = ExtractFilePath(snap.filePath);
        // ExtractFilePath keeps the trailing backslash; strip it before
        // ExtractFilePath'ing the parent again for the year directory.
        AnsiString dirDayNoSlash = dirDay;
        if (dirDayNoSlash.Length() > 0 && dirDayNoSlash[dirDayNoSlash.Length()] == '\\')
            dirDayNoSlash.SetLength(dirDayNoSlash.Length() - 1);
        AnsiString dirYear = ExtractFilePath(dirDayNoSlash);
        if (!snap.dayDirExistedBefore)
            RemoveDir(dirDayNoSlash);
        if (!snap.yearDirExistedBefore)
            RemoveDir(dirYear);
    }
}

//---------------------------------------------------------------------------
//  AI(W906-UploadFamily-StackFix) 20260727: DoUploadFileToHost /
//  DoUploadFileToHost_ForSingleFile -- golden dispatcher .cpp:4941-4949,
//  ForSingleFile body .cpp:4249-4599 range (this wave, 20260723). MINIMAL
//  coverage added per independent review finding (the UploadFamily wave
//  shipped with ZERO test coverage for this whole ~330-line 3-way family).
//
//  IMPORTANT, discovered while writing this test (not assumed going in): a
//  naive "single small file -> Task 1->100->200->1 happy path" test is NOT
//  what actually happens, because PRESERVED GOLDEN BUG #1 (see this
//  function's own .cpp file-head comment) bites on the VERY FIRST real
//  Task==200 call, not just on some later multi-chunk call. Case 100's own
//  `iMaxSend=EC69_UNT1_MaxTranslateLen; ... else iMaxSend=4096;` chain only
//  ever computes the LOCAL `iMaxSend` -- it is never written back to the
//  backing member `iUploadFileToHost_ForSingleFileMaxSend`. So the NEXT
//  call's `int iMaxSend=iUploadFileToHost_ForSingleFileMaxSend;` re-reads
//  that member fresh, still its ctor-default 0 (never written by ANY golden
//  code path) -- meaning `TotalFileSize>iMaxSend` (12>0) is ALWAYS true, so
//  `case 200` is permanently forced onto the "iReadSize=iMaxSend" (==0)
//  chunked branch, never the "iReadSize=TotalFileSize" whole-file-done
//  branch, no matter how small the real file is. This test exercises and
//  asserts that REAL (bug-preserving) behavior rather than the wrong
//  happy-path assumption. NOT covered here, a real gap left for a future
//  wave, not silently claimed as full coverage: ever actually completing an
//  upload (blocked by this same bug), the _ForMultiFile/_ForDirectoryFile
//  sibling bodies, or PRESERVED GOLDEN BUG #2's own leaked-FILE* angle.
//---------------------------------------------------------------------------
static void test_w906_uploadfamily_e2e()
{
    printf("\n[W906-UploadFamily] DoUploadFileToHost / DoUploadFileToHost_ForSingleFile, real THGem-backed coverage\n");

    const AnsiString kDir = "uploadfamily_test_scratch";
    ForceDirectories(kDir);
    const AnsiString kFileName = "upload_me.dat";
    const AnsiString kFullPath = kDir + AnsiString("\\") + kFileName;

    {
        FILE *f = fopen(kFullPath.c_str(), "wb");
        CHECK(f != NULL, "setup: scratch upload file created");
        if (f) { fwrite("HELLO_UPLOAD", 1, 12, f); fclose(f); }
    }

    THGem g;
    g.FileListBox2->Mask = kDir + AnsiString("\\*.dat");
    g.GemLocalFileLixtBox = new THGemListBox();
    g.GemLocalFileLixtBox->Items->Add(kFileName);
    g.GemLocalFileLixtBox->Checked[0] = true;   // pre-set true so case 100's flip to false is observable
    g.UploadFileName->Add(kFileName);

    CHECK(g.iUploadFileToHost_ForSingleFile == 1, "precondition: Task starts at 1 (golden ctor :482)");

    // Task 1 -> 100 (UploadFileName->Count==1, nonzero).
    g.DoUploadFileToHost_ForSingleFile();
    CHECK(g.iUploadFileToHost_ForSingleFile == 100, "Task 1->100: file was queued in UploadFileName");

    // Task 100 -> 200: opens the real file, computes TotalFileSize/iStoreCT/
    // iTotalCount, flips GemLocalFileLixtBox->Checked[0] false, fopen()s P
    // (LOCALLY only -- see PRESERVED GOLDEN BUG #2 in the .cpp comment; the
    // real handle this fopen() returns is never saved to any member, so it
    // is already effectively leaked/unreachable the instant this call returns).
    g.DoUploadFileToHost_ForSingleFile();
    CHECK(g.iUploadFileToHost_ForSingleFile == 200, "Task 100->200: fopen succeeded on the real scratch file");
    CHECK(g.UploadFileToHost_ForSingleFileTotalFileSize == 12, "Task 100: TotalFileSize == real file size (12 bytes)");
    CHECK(g.GemLocalFileLixtBox->Checked.Values[0] == false, "Task 100: GemLocalFileLixtBox->Checked[0] flipped true->false (golden :4615-4616)");
    CHECK(g.UploadFileName->Count == 0, "Task 100: UploadFileName->Delete(0) consumed the queued entry");

    // Task 200's FIRST real invocation -- PRESERVED GOLDEN BUG #1 in action
    // (see this test function's own file-head comment above for the full
    // derivation): iMaxSend reverts to 0, forcing the chunked branch with a
    // 0-byte read instead of a 1-shot whole-file finish.
    g.DoUploadFileToHost_ForSingleFile();
    CHECK(g.iUploadFileToHost_ForSingleFile == 300, "Task 200->300: PRESERVED GOLDEN BUG #1 -- iMaxSend reverted to 0 (member never written back by case 100), so TotalFileSize(12)>iMaxSend(0) forces the chunked branch, not a 1-shot whole-file finish");
    CHECK(g.iUploadFileToHost_ForSingleFileStoreCT == 1, "Task 200: iStoreCT incremented to 1");
    CHECK(g.UploadFileToHost_ForSingleFileTotalFileSize == 12, "Task 200: PRESERVED GOLDEN BUG #1 consequence -- TotalFileSize is UNCHANGED (iReadSize was forced to 0, so nothing was actually read/subtracted)");
    CHECK(g.bReceiveS101F6 == false, "Task 200: bReceiveS101F6 reset false at the tail of this case, gating case 300's own retry check");

    // Task 300 stays parked until a real S101F6 ack sets bReceiveS101F6 --
    // proving the state machine does NOT silently self-advance without one.
    g.DoUploadFileToHost_ForSingleFile();
    CHECK(g.iUploadFileToHost_ForSingleFile == 300, "Task 300: stays parked without a real bReceiveS101F6==true ack (golden :4668-4671)");

    if (FileExists(kFullPath)) DeleteFile(kFullPath);
    RemoveDir(kDir);

    // -- Dispatcher routing: DoUploadFileToHost() picks the right sibling via
    //    SV_70_UNT1_ReceipeStruct (golden .cpp:4941-4949). Exercised here on
    //    a FRESH THGem/scratch-dir setup (independent of the Task==300 state
    //    left over above) for just the ==0 (ForSingleFile) branch; the
    //    ==1/==2 branches (MultiFile/DirectoryFile) are NOT covered by this
    //    test.
    {
        const AnsiString kDir2 = "uploadfamily_test_scratch_dispatch";
        ForceDirectories(kDir2);
        const AnsiString kFullPath2 = kDir2 + AnsiString("\\") + kFileName;
        FILE *f = fopen(kFullPath2.c_str(), "wb");
        if (f) { fwrite("X", 1, 1, f); fclose(f); }

        THGem g2;
        g2.FileListBox2->Mask = kDir2 + AnsiString("\\*.dat");
        g2.GemLocalFileLixtBox = new THGemListBox();
        g2.GemLocalFileLixtBox->Items->Add(kFileName);
        g2.UploadFileName->Add(kFileName);
        g2.SV_70_UNT1_ReceipeStruct = 0;

        g2.DoUploadFileToHost();
        CHECK(g2.iUploadFileToHost_ForSingleFile == 100, "DoUploadFileToHost() dispatcher: SV_70_UNT1_ReceipeStruct==0 really reached DoUploadFileToHost_ForSingleFile (Task advanced 1->100)");

        if (FileExists(kFullPath2)) DeleteFile(kFullPath2);
        RemoveDir(kDir2);
    }
}

// ===========================================================================
int main()
{
    printf("=== SECSGEM/uHGemEquipment (+ vclcompat/StringGrid) translation verification ===\n");
    printf("(golden HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.{h,cpp}; original binary NOT run)\n");

    test_gemtimer();
    test_stringgrid_basic();
    test_split_str_by_tab_only();
    test_copy_paste_roundtrip_real_oracle();
    test_ceid_family();
    test_report_delete_family();
    test_read_event_report_data_real_oracle();
    test_alarm_read_write_roundtrip();
    test_connect_lifecycle();
    test_clientgem_handlers();
    test_doopencommuncation_open_guard();
    test_srvgemclient_handlers();
    test_check_socket_active_false();
    test_clear_default_even_report();
    test_get_time_info();
    test_online_family_and_misc();
    test_stringout_and_binaryout();
    test_initialhgem_savesystemdefault_roundtrip();
    test_manualcreatergroleclick();
    test_processshow();
    test_stringout_2arg();
    test_do_update_status();

    // Bucket C (W906-uHGemEquipment-BucketC 20260717)
    test_memorystream_shim();
    test_htimer_real_semantics();
    // T3/T4/T5 reach the real SaveSECSGEMTextToLog() -- see the
    // CaptureTextLogSnapshot/RestoreTextLogSnapshot comment above.
    TextLogSnapshot textLogSnap = CaptureTextLogSnapshot();
    test_select_handshake_e2e();
    test_frame_reassembly();
    test_data_message_gated_dispatch();
    RestoreTextLogSnapshot(textLogSnap);
    test_t3_send_side_record();
    test_t3_timeout_s9f9_cross_class_seam();
    test_checksfcoderesponse();
    test_doconnect_state_walk();

    // W906-SysModWire (W1-W10): W3-W10 pump ProcessSocketReceiveData, which
    // reaches the now-real SaveSECSGEMTextToLog() -- same test-hygiene rule
    // as the T3/T4/T5 bracket above (see that TextLogSnapshot comment).
    // Deliberately placed AFTER [T9] and BEFORE [T10] -- none of W1-W10 pump
    // Timer1Timer (see test_timer1timer_disable_branch's own comment on why
    // it must stay the LAST test in this binary to do so).
    TextLogSnapshot sysModWireLogSnap = CaptureTextLogSnapshot();
    test_w906_sysmodwire_w1_systemmodularinitial_wiring();
    test_w906_sysmodwire_w2_s1f13_frame();
    test_w906_sysmodwire_w3_doconnect_e2e();
    test_w906_sysmodwire_w4_host_s1f13_reply();
    test_w906_sysmodwire_w5_s1f1_s1f2();
    test_w906_sysmodwire_w6_tail_negative_paths();
    test_w906_sysmodwire_w7_flag_latch_movecheckcallback();
    test_w906_sysmodwire_w8_customer_code_gate();
    test_w906_sysmodwire_w9_two_codec_merge_oracle();
    test_w906_sysmodwire_w10_null_guard_regression();
    RestoreTextLogSnapshot(sysModWireLogSnap);

    // W906-SvEcDataItem (design doc Wave 1): FormCreate test is pure
    // in-memory (no file I/O, no snapshot needed); the e2e dispatch-pump
    // tests reach SaveSECSGEMTextToLog the SAME way the SysModWire block
    // above does -- same TextLogSnapshot bracket idiom. Placed AFTER W10 and
    // BEFORE [T10] -- none of these pump Timer1Timer (ProcessSocketReceiveData
    // only), so T10's own "must stay LAST among Timer1Timer-pumping tests"
    // invariant is unaffected.
    test_w906_svecdataitem_formcreate();
    TextLogSnapshot svEcDataItemLogSnap = CaptureTextLogSnapshot();
    test_w906_svecdataitem_e2e_s1f4_formcreate();
    test_w906_svecdataitem_e2e_s1f24();
    test_w906_svecdataitem_e2e_s6f20();
    test_w906_svecdataitem_e2e_s5f8_s100f4();
    test_w906_svecdataitem_e2e_s101f6_s101f8_smoke();
    RestoreTextLogSnapshot(svEcDataItemLogSnap);

    // W906-VCW1: direct DataItemOutSV/DataItemOutEC calls only (no socket
    // pump/Timer1Timer, no file I/O) -- same posture as W906-AlarmReportAck
    // immediately below, no TextLogSnapshot bracket needed.
    test_w906_vcw1_dataitemout_vcl_dispatch();

    // W906-AlarmReportAck: does not pump ProcessSocketReceiveData/
    // Timer1Timer at all (direct HTGem method calls only), so it needs
    // neither a TextLogSnapshot bracket nor special ordering vs [T10] below.
    test_w906_alarmreportack_e2e();

    // W906-uHGemClass-Micro5: same posture as W906-AlarmReportAck immediately
    // above -- direct HTGem method calls only, no socket pump/Timer1Timer, so
    // no TextLogSnapshot bracket or special T10 ordering needed either.
    test_w906_uhgemclass_micro5_e2e();

    // W906-uHGemClass-Micro6: same posture as Micro5 immediately above --
    // direct HTGem method calls only, no socket pump/Timer1Timer, so no
    // TextLogSnapshot bracket or special T10 ordering needed either.
    test_w906_uhgemclass_micro6_e2e();

    // W906-uHGemClass-Micro7: same posture as Micro5/Micro6 immediately
    // above -- direct HTGem method calls only, no socket pump/Timer1Timer,
    // so no TextLogSnapshot bracket or special T10 ordering needed either.
    test_w906_uhgemclass_micro7_e2e();

    // W906-DoDownLoadRemoteFile: same posture as Micro5/Micro6/Micro7 above --
    // direct THGem method calls only, no socket pump/Timer1Timer, so no
    // TextLogSnapshot bracket or special T10 ordering needed either.
    test_w906_dodownloadremotefile_e2e();

    // W906-SpoolCluster: same posture as Micro5/Micro6/Micro7/
    // DoDownLoadRemoteFile above -- direct THGem method calls only, no
    // socket pump/Timer1Timer, so no TextLogSnapshot bracket or special T10
    // ordering needed either. DoSpoolSendLocalData's own StringOut/ShowSML
    // calls DO write through THGem's normal StringOut path, same as every
    // other test in this file that calls a THGem method touching the wire --
    // not routed through SaveSECSGEMTextToLog (T3/SysModWire's own concern),
    // so no snapshot bracket is needed here either.
    test_w906_spoolcluster_e2e();

    // W906-UploadFamily-StackFix: same posture as SpoolCluster/Micro5-7/
    // DoDownLoadRemoteFile above -- direct THGem method calls only, no
    // socket pump/Timer1Timer, so no TextLogSnapshot bracket or special T10
    // ordering needed either.
    test_w906_uploadfamily_e2e();

    test_timer1timer_disable_branch();

    printf("\n=== RESULT: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
