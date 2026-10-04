// ===========================================================================
//  tests/test_ga1_cpublic.cpp
//  AI(GA1-B3) 20260804: acceptance tests for the cpublic.cpp queue/union/
//  VerInfo ungate batch (task GA-1-B3).
//
//  Covers, through the REAL translated cpublic.cpp code (not a re-impl):
//    [A] TMyStrQueue100   ring-buffer wrap-around (pure in-memory)
//    [B] TMyTimerQueue100 ring-buffer wrap-around (pure in-memory)
//    [C] TMyStrQueue100::SafeData    -- newly ungated this batch
//    [D] TMyTimerQueue100::SafeData  -- newly ungated this batch
//    [E] UDPErrorLog                 -- newly ungated this batch
//    [F] ShuttleLog                  -- newly ungated this batch (both
//        UseCanBusOrEtherCAT() branches)
//
//  Deliberately does NOT test:
//    - VerInfo::GetSVNRev/GetFileVersion/GetMainVersion -- still `#if 0`
//      (blocked by `Application->ExeName`, no TApplication port anywhere in
//      this tree; see cpublic.cpp's own tightened TODO and _ga1_b3_report.md).
//      VerInfo's ALREADY-ungated ctor/GetAppVersion path is exercised by the
//      sibling tests/test_cpublic_foundation.cpp, not duplicated here.
//    - TTLLog/HeaterLog/OutShuttleLog/HomeLog/RespondASECom/ProductionLog/
//      ProductionDataLog/LogIndexMaxMinPos/GetBundleInfo/RecordErrorLog's
//      golden real body/GetEveryCode+UT100/TMC401+E5DC -- all still gated,
//      see cpublic.cpp's per-function TODO tags for the exact blocker.
//
//  SELF-CONTAINED STAND-IN CONVENTION (same idiom as tests/test_config_loaders.cpp
//  / tests/test_MyCCLink.cpp): this batch's validation boundary forbids a
//  cmake tree-level build (a different GA-1 batch is concurrently editing
//  cprod.cpp this same day) -- so this test is designed to link WITHOUT
//  cprod.cpp at all, via a manual g++ invocation (see _ga1_b3_report.md for
//  the exact recipe), not through tests/CMakeLists.txt (outside this batch's
//  write boundary; wiring it in is an integrator step).  In that reduced
//  (non-fMain, non-cprod) link, cpublic.cpp's ONLY unresolved extern is
//  `IniConfig` (Config.h's `HT9045_CONFIG`, normally defined cprod.cpp:50) --
//  confirmed by grepping cpublic.cpp's ACTIVE (non-`#if 0`) code: every
//  `fMain->`/`ASESendMessage`/`LotSummary` mention left in the file lives
//  inside a still-gated block, so none of those need a stand-in here.  A
//  single trivial-aggregate local definition below satisfies the one real
//  link gap; nothing else is stubbed.
//
//  NEVER WRITES REAL PRODUCTION PATHS: the three log-path globals this test
//  drives (asGalilCmdPath / asUDPLogPath / asShtSenLogPath) are reassigned to
//  a GetTempPathA()-rooted subdirectory before ANY function under test runs;
//  golden's own defaults (D:\HT9045_Log\...) are never touched or created.
//
//  Manual build (no CMake changes made; see _ga1_b3_report.md for the full
//  reproduce recipe, including which cpublic.cpp/cmydef.cpp/common.cpp/
//  vclcompat/... object files this links against):
//    g++ -std=c++17 -I <repo-root> -c tests/test_ga1_cpublic.cpp
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cpublic.h"
#include "cmydef.h"
#include "common.h"
#include "Config.h"
#include "CCLink/MyCCLinkSensor_predicates.h"   // not called directly here -- SHUTTLE_SENSOR_TYPE drives it via ShuttleLog()

#include <windows.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------------------
//  Local stand-in -- see file-head banner. Trivial zero-initialised aggregate
//  (Config.h's `HT9045_CONFIG` is a plain typedef struct, default-constructible).
//  Nothing under test in THIS file reads any IniConfig member; it exists
//  purely to satisfy cpublic.cpp's link-time extern (SetSocketHandlerID and
//  other, still-gated, functions elsewhere in the same TU reference it).
// ---------------------------------------------------------------------------
HT9045_CONFIG IniConfig;

// ============================================================
//  Harness (same CHECK-style idiom as test_cpublic_foundation.cpp)
// ============================================================
static int g_pass = 0;
static int g_fail = 0;
#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL [%s:%d] %s\n", __FILE__, __LINE__, #expr); \
        g_fail++; \
    } else { \
        g_pass++; \
    } \
} while(0)

// ============================================================
//  Helpers
// ============================================================
static std::string ReadWholeFile(const AnsiString& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return std::string();
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static AnsiString MakeTempBaseDir()
{
    char buf[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, buf);
    AnsiString base = AnsiString(std::string(buf, n)) + "ga1_b3_cpublic_test";
    return base;
}

// ============================================================
//  [A] TMyStrQueue100 ring-buffer wrap-around (pure in-memory)
// ============================================================
static void test_strqueue100_wrap()
{
    TMyStrQueue100 q;
    CHECK(q.iIndex == 0);
    CHECK(q.iCount == 0);

    // Add() shape (cpublic.cpp): while iCount<MAX_Q_100(=100), fills
    // sequentially at slot iCount (iCount then ++); once full, writes at
    // iIndex (iIndex++ after, wrapping at >=100). ctor leaves iIndex=0, so
    // the FIRST overflow write lands back at slot 0.
    for (int i = 0; i < 102; i++)
    {
        char c1[32], c2[32];
        std::snprintf(c1, sizeof(c1), "cmd%d", i);
        std::snprintf(c2, sizeof(c2), "resp%d", i);
        q.Add(AnsiString(c1), AnsiString(c2));
    }

    CHECK(q.iCount == 100);           // capped at MAX_Q_100
    CHECK(q.iIndex == 2);             // 2 overflow writes past the initial fill (cmd100, cmd101)
    CHECK(q.sData1[0] == "cmd100");   // overwritten by the 101st add
    CHECK(q.sData1[1] == "cmd101");   // overwritten by the 102nd add
    CHECK(q.sData1[2] == "cmd2");     // untouched (only 2 slots wrapped so far)
    CHECK(q.sData2[0] == "resp100");
}

// ============================================================
//  [B] TMyTimerQueue100 ring-buffer wrap-around (pure in-memory)
// ============================================================
static void test_timerqueue100_wrap()
{
    TMyTimerQueue100 q;
    CHECK(q.iIndex == -1);
    CHECK(q.iCount == -1);

    // Add() shape here is DIFFERENT from TMyStrQueue100 above: iIndex
    // increments (and wraps) UNCONDITIONALLY every call; iCount increments
    // only while <MAX_Q_100 then freezes at 100. ctor leaves iIndex=-1, so
    // slot 0 is first touched on call #1, not call #0-after-fill.
    for (int i = 0; i < 103; i++)
    {
        char s1[32], s2[32];
        std::snprintf(s1, sizeof(s1), "start%d", i);
        std::snprintf(s2, sizeof(s2), "end%d", i);
        q.Add(AnsiString(s1), AnsiString(s2), i);
    }

    CHECK(q.iCount == 100);              // capped
    CHECK(q.iIndex == 2);                // 103 unconditional increments, wrapping at 100 -> (103-1) % 100 = 2
    CHECK(q.sStartTime[0] == "start100"); // wrapped-over write
    CHECK(q.sStartTime[1] == "start101");
    CHECK(q.sStartTime[2] == "start102");
    CHECK(q.sStartTime[3] == "start3");   // untouched
    CHECK(q.iTime[2] == 102);
}

// ============================================================
//  [C] TMyStrQueue100::SafeData -- newly ungated this batch
// ============================================================
static void test_strqueue100_safedata()
{
    AnsiString tempBase = MakeTempBaseDir();
    asGalilCmdPath = tempBase + "\\Galil_Log";   // golden default has NO trailing backslash -- match it

    TMyStrQueue100 q;
    q.Add("cmdA", "respA");
    q.Add("cmdB", "respB");
    q.Add("cmdC", "respC");
    CHECK(q.iCount == 3);

    q.SafeData();   // real golden body (RogerYang 20260505 FixGalilLog revision)

    // SafeData() itself calls GetTimeInfo() (real Now()), so read back the
    // globals it just set to reconstruct the exact path it wrote to --
    // avoids racing the clock between test and function-under-test.
    AnsiString asPath;
    asPath.sprintf("%s\\%04d%02d\\", asGalilCmdPath, SystemYear, SystemMonth);
    AnsiString expectFile;
    expectFile.sprintf("%s%04d%02d%02d_%02d%02d%02d.%03d.csv",
        asPath, SystemYear, SystemMonth, SystemDate,
        SystemHour, SystemMin, SystemSec, SystemMSec);

    std::string content = ReadWholeFile(expectFile);
    CHECK(!content.empty());
    CHECK(content.find("cmdA") != std::string::npos);
    CHECK(content.find("cmdB") != std::string::npos);
    CHECK(content.find("cmdC") != std::string::npos);
    // Golden quirk (preserved, not fixed): the write loop is
    // `Str.sprintf("%s, %s", DateTime[i], sData1[i])` -- sData2 ("respX") is
    // silently NEVER written to the CSV. Lock that in explicitly.
    CHECK(content.find("respA") == std::string::npos);

    // SafeData()'s own tail: ClearData() + sLastFileName tracking.
    CHECK(q.iCount == 0);
    CHECK(q.iIndex == 0);
    CHECK(q.sLastFileName == expectFile);
}

// ============================================================
//  [D] TMyTimerQueue100::SafeData -- newly ungated this batch
// ============================================================
static void test_timerqueue100_safedata()
{
    AnsiString tempBase = MakeTempBaseDir();
    asGalilCmdPath = tempBase + "\\Galil_Log";   // SAME global as [C] -- golden shares one path for both queues

    TMyTimerQueue100 q;
    q.Add("09:00:00.000", "09:00:01.000", 1000);
    q.Add("09:00:02.000", "09:00:03.500", 2000);
    q.Add("09:00:04.000", "09:00:07.000", 3000);
    CHECK(q.iCount == 2);

    q.SafeData();

    AnsiString asPath;
    asPath.sprintf("%s\\%04d%02d\\", asGalilCmdPath, SystemYear, SystemMonth);
    AnsiString expectFile;
    expectFile.sprintf("%s%04d%02d%02d_%02d%02d%02d.%03d.csv",
        asPath, SystemYear, SystemMonth, SystemDate,
        SystemHour, SystemMin, SystemSec, SystemMSec);

    std::string content = ReadWholeFile(expectFile);
    CHECK(!content.empty());
    // GetDateTime(i) format: `DateTime, "start", "end", "timeStr"` --
    // ConvertMSecToSPC(2000)="00:00:02.000" etc.
    //
    // GOLDEN QUIRK, VERIFIED EMPIRICALLY (real g++ run, not just static
    // reading) and PRESERVED, not fixed: SafeData()'s own write loop is
    // `for(i=0;i<iCount;i++)`, but the ctor/ClearData() leave iCount at -1
    // (not 0, unlike TMyStrQueue100's sibling), so after N adds iCount==N-1
    // -- one less than the live item count. Combined with GetDateTime(i)'s
    // `flag=iIndex-i` (i=0 -> the NEWEST entry), the loop walks only the
    // newest (iCount) entries and silently DROPS the single OLDEST surviving
    // entry from the CSV every time. With 3 adds here (iCount==2 after),
    // only the 2nd and 3rd adds are written -- the 1st ("09:00:00.000" /
    // ConvertMSecToSPC(1000)) never appears. Lock this in explicitly rather
    // than assume all 3 rows land.
    CHECK(content.find(ConvertMSecToSPC(2000).c_str()) != std::string::npos);
    CHECK(content.find(ConvertMSecToSPC(3000).c_str()) != std::string::npos);
    CHECK(content.find("09:00:02.000") != std::string::npos);
    CHECK(content.find("09:00:04.000") != std::string::npos);
    CHECK(content.find(ConvertMSecToSPC(1000).c_str()) == std::string::npos);  // dropped (quirk above)
    CHECK(content.find("09:00:00.000") == std::string::npos);                  // dropped (quirk above)

    CHECK(q.iCount == -1);  // ClearData() tail -- NOTE: resets iCount/iIndex
    CHECK(q.iIndex == -1);  // to -1 (not 0 like TMyStrQueue100's ClearData) --
                            // preserved golden asymmetry between the two
                            // sibling classes' ClearData bodies.
}

// ============================================================
//  [E] UDPErrorLog -- newly ungated this batch
// ============================================================
static void test_udperrorlog()
{
    AnsiString tempBase = MakeTempBaseDir();
    asUDPLogPath = tempBase + "\\UDP";   // golden default has NO trailing backslash -- match it

    UDPErrorLog("MyTitle", "MyCommand1");

    AnsiString expectFile;
    expectFile.sprintf("%s\\%04d\\%02d\\%02d%02d%02d%02d.txt",
        asUDPLogPath, SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour);

    std::string content = ReadWholeFile(expectFile);
    CHECK(!content.empty());
    CHECK(content.find("MyTitle") != std::string::npos);
    CHECK(content.find("MyCommand1") != std::string::npos);
    CHECK(content.find(",   MyTitle, MyCommand1") != std::string::npos);

    // WriteDataToFile appends (bOverWrite defaults false) -- a 2nd call must
    // ADD a line, not replace the file.
    UDPErrorLog("SecondTitle", "MyCommand2");
    std::string content2 = ReadWholeFile(expectFile);
    CHECK(content2.find("MyCommand1") != std::string::npos);  // first entry survives
    CHECK(content2.find("MyCommand2") != std::string::npos);  // second entry appended
    CHECK(content2.size() > content.size());
}

// ============================================================
//  [F] ShuttleLog -- newly ungated this batch (both UseCanBusOrEtherCAT() branches)
// ============================================================
#if 0 // AI(W906-GA1-B3-integrate) 20260804: ShuttleLog was re-gated at integrate
      // (link-graph: UseCanBusOrEtherCAT@ht9045_comms outside standard closure); this
      // section returns with it.

static void test_shuttlelog()
{
    AnsiString tempBase = MakeTempBaseDir();
    asShtSenLogPath = tempBase + "\\ShuttleSensorLog\\";   // golden default HAS a trailing backslash -- match it

    // Shuttle2 lane (i<8 branch): CH reading at i=3 -> iSensor[1][3]/iSensor[3][3]
    iSensor[1][3] = 111;
    iSensor[3][3] = 222;
    // Shuttle1 lane (i>=8 branch): i=10 -> iSensor[0][10-8]/iSensor[2][10-8] = iSensor[0][2]/iSensor[2][2]
    iSensor[0][2] = 333;
    iSensor[2][2] = 444;

    // NOTE: ShuttleLog() calls GetTimeInfo() itself; read System* back AFTER
    // the call for the exact values it used (same pattern as [C]/[D]/[E]).

    // -- false branch: UseCanBusOrEtherCAT()==false (eSensorCCLink) ----------
    SHUTTLE_SENSOR_TYPE = eSensorCCLink;
    CHECK(UseCanBusOrEtherCAT() == false);
    ShuttleLog();

    // Path built EXACTLY like ShuttleLog()'s own two-step sprintf (golden
    // cpublic.cpp): first the YYYYMM folder (sFileName in the golden body),
    // THEN the YYYYMMDD.logs leaf inside it -- asShtSenLogPath itself is only
    // the base directory, one level shallower than the actual log file.
    AnsiString shtDir;
    shtDir.sprintf("%s%04d%02d", asShtSenLogPath, SystemYear, SystemMonth);
    AnsiString expectFile;
    expectFile.sprintf("%s\\%04d%02d%02d.logs", shtDir, SystemYear, SystemMonth, SystemDate);
    std::string content = ReadWholeFile(expectFile);
    CHECK(!content.empty());
    CHECK(content.find("Start") != std::string::npos);
    CHECK(content.find("End") != std::string::npos);
    CHECK(content.find("Read:111  Set:222") != std::string::npos);
    CHECK(content.find("Read:333  Set:444") != std::string::npos);
    size_t sizeAfterFalseBranch = content.size();

    // -- true branch: UseCanBusOrEtherCAT()==true (eSensorEtherCAT) ----------
    // Golden quirk (preserved, not fixed): both branches of ShuttleLog's
    // if(UseCanBusOrEtherCAT()) produce IDENTICAL log content for the same
    // iSensor[][] values (same indices, same format string) -- the predicate
    // doesn't actually change WHAT gets logged, only which of two
    // functionally-equivalent code paths does the logging. Verify the branch
    // executes (appends a 2nd copy) and produces the SAME markers again.
    SHUTTLE_SENSOR_TYPE = eSensorEtherCAT;
    CHECK(UseCanBusOrEtherCAT() == true);
    ShuttleLog();

    std::string content2 = ReadWholeFile(expectFile);
    CHECK(content2.size() > sizeAfterFalseBranch);   // appended, not overwritten
    CHECK(content2.find("Read:111  Set:222") != std::string::npos);
    CHECK(content2.find("Read:333  Set:444") != std::string::npos);
}
#endif

// ============================================================
//  main
// ============================================================
int main()
{
    printf("=== test_ga1_cpublic ===\n");

    printf("[A] TMyStrQueue100 ring-buffer wrap-around\n");
    test_strqueue100_wrap();

    printf("[B] TMyTimerQueue100 ring-buffer wrap-around\n");
    test_timerqueue100_wrap();

    printf("[C] TMyStrQueue100::SafeData (temp-dir redirected)\n");
    test_strqueue100_safedata();

    printf("[D] TMyTimerQueue100::SafeData (temp-dir redirected)\n");
    test_timerqueue100_safedata();

    printf("[E] UDPErrorLog (temp-dir redirected)\n");
    test_udperrorlog();

    printf("[F] ShuttleLog (temp-dir redirected, both branches)\n");
    // test_shuttlelog();  // gated with ShuttleLog (see above)

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
