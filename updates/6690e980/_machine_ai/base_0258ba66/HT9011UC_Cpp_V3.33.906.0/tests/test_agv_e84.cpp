// =============================================================================
//  tests/test_agv_e84.cpp  --  W5-Final VERIFY: E84 LOADER/UNLOADER HANDSHAKE
//
//  Translation wave: W5-Final (AGV_E84 unit -- Automation/AGV_E84.{h,cpp})
//  Author: AI(W5-Final-AGV_E84) 20260711
//
//  PURPOSE
//  -------
//  Proves Automation/AGV_E84.cpp (in lib ht9045_sm) LINKS and its translated
//  DoE84Loader()/DoE84Unloader() case-ladder SMs PUMP over the Sim HAL with NO
//  real hardware and NO Borland binary (same "no live diff possible" caveat as
//  every other W6/W5 verify TU in this tree -- see test_ContactForce.cpp's own
//  LIMITATION note).  "Equivalence" here == clean g++ link + the case-ladder
//  advancing/staying exactly where the golden switch(Task) logic says it must,
//  given a KNOWN, deterministic Sim-HAL sensor/switch configuration.
//
//  SIM-HAL FORCING TECHNIQUE (backend-independent, no Ring/IP/Port/Bit
//  coordinate collisions with any other unconfigured Sen[]/SW[] element):
//    TMySensor::IsOn()  (mysensor.cpp) : if Enable==false -> false (guard).
//      Else routes through MyLaneIO ONLY when ISABase matches a known routing
//      enum (eMotionNet=0/ePCI1203/ePLCbase/eISABase/ePCI1735U).  Setting
//      ISABase to a SENTINEL value (-1, matches none) makes `ret` stay at its
//      local-declared default `false` regardless of backend state; the
//      function then returns `Type ? ret : !ret`.  So:
//        Enable=true, ISABase=-1, Type=0 (ctor default) -> IsOn()==true
//        Enable=true, ISABase=-1, Type=1                -> IsOn()==false
//    TMySwitch::Status() (myswitch.cpp) mirrors this: Enable=true, ISABase=-1,
//      Type=1 -> Status() reports whatever `On()`/`Off()` last set OutValue to
//      (On()/Off() themselves set OutValue unconditionally BEFORE the
//      Enable-guarded hardware-routing branch, so OutValue is reliable even
//      offline).  golden AGV.cpp calls `SW[SwE84_1_ES].On(); SW[SwE84_1_HOAVBL]
//      .On();` unconditionally at the top of EVERY DoE84Loader()/DoE84Unloader()
//      call (`#ifndef SOFT_SIMULTE`, ACTIVE in this build -- SOFT_SIMULTE is
//      `//#define`d out), so HOAVBL's OutValue is already true by the time
//      case 1 checks Status() within that SAME call.
//    TMyCylinder is NOT touched by these oracles (case 100's OffStatus() gate
//      is one step further than we pump -- see ORACLE O3 scope note below).
//
//  ORACLES (hand-derived from the golden switch(Task) ladder, NOT from this
//  translation):
//
//    [O1] CURSOR RESET.  InitialE84LoadTask()/InitialE84UnLoaderTask() ->
//         iE84LoadTask==1 / iE84UnloadTask==1 (golden :176-184).
//
//    [O2] CONSERVATIVE GATE (offline, zero Sim-HAL configuration).  With every
//         Sen[]/SW[] at its ctor default (Enable=false), case 1's guard
//         `SW[...HOAVBL].Status()==true && Sen[...GO].IsOn()` can never be
//         true (Status()/IsOn() both hard-return false when Enable==false,
//         golden mysensor.cpp/myswitch.cpp -- verified in the read, not
//         asserted blind).  Pumping DoE84Loader()/DoE84Unloader() 10 ticks
//         each must leave both cursors sitting at 1 (no spurious advance).
//
//    [O3] SINGLE-TICK ADVANCE (case 1 -> 50, golden :242-264 / :606-625).  With
//         HOAVBL forced Status()==true, GO and CS0 forced IsOn()==true (see
//         forcing technique above), ONE call must set iPlaceWhichBuffer[x]=10
//         (assigned unconditionally inside the outer if, golden :248/:609) and
//         advance the cursor to 50 (the CS0-true branch, golden :249-258/
//         :610-619).  Scope note: we do NOT pump past 50 into the
//         Cylinder-gated case 100 (that would additionally require forcing
//         TMyCylinder::OffStatus(), a 3rd HAL object type -- out of proportion
//         for this verify TU; the case-1/50 transition already exercises the
//         translated Sen[]/SW[] wiring + iPlaceWhichBuffer bookkeeping).
//
//    [O4] TIMEOUT-RECOVERY ROUND TRIP (case 50 -> 5000 -> back to 1, golden
//         :265-277 timeout arm / :581-583 Auto-Recover /:1055-1065 the
//         btInitalLoad/btInitalUnLoadClick handler bodies).  This is the
//         DIRECT verification of this unit's one flagged JUDGMENT CALL: the
//         golden `btInitalLoad->Click()` / `btInitalUnLoad->Click()` widget
//         call was collapsed to a direct call of the click handler's own body
//         (InitialE84LoadTask()+InitialE84LoadSensor() / InitialE84UnLoaderTask()
//         +InitialE84UnloadSensor()).  With TestIF_File.iE84TimeOut_K12[0][9]
//         forced to 0 seconds (so E84LoadDelay/E84UnLoadDelay -- both armed
//         from THIS SAME [0][9] cell per the golden index-bug this unit
//         preserved verbatim, see AGV_E84.cpp's DoE84Unloader case-1 comment --
//         report themselves already-expired on the very next .Off() check),
//         Sen[...VALID] left at its Enable=false default (never satisfied):
//           tick after O3: case 50 -> Sensor_VALID not on, timer expired -> 5000
//           tick after that: case 5000 -> collapsed handler runs -> cursor==1
//         If the collapse were NOT behaviourally equivalent to the real
//         Click(), the cursor would NOT return to 1 here.
//
//    [O5] LOGGING SMOKE + GetTimeInfo() WIRING.  ShowE84Log()/E84StatusLog()
//         must not crash for representative (str, iFunction, iLoader) /
//         (str, bflag) argument combinations (including the iLoader==3/4
//         ShowMyMessage(str) branch, which calls the REAL canary_support.cpp
//         Sim stand-in -- ACTIVE, not gated).  SystemYear (cmydef.cpp ctor
//         sentinel 9999, golden cmydef.cpp:292) must become a real calendar
//         year (>=2020, !=9999) after GetTimeInfo() runs inside ShowE84Log(),
//         proving the cpublic.cpp GetTimeInfo()->cmydef.h SystemYear/Month/
//         Date/Hour/Min/Sec/MSec wiring is intact end-to-end.
//         SIDE EFFECT (documented, not a defect): ShowE84Log()/E84StatusLog()
//         call the REAL vclcompat DirectoryExists/ForceDirectories against the
//         verbatim golden path "D:\HT9045_Log\E84DataTxt\<yyyymm>\<mmdd>\" --
//         this creates that (empty; WriteDataToFile itself stays gated, see
//         AGV_E84.cpp file head) directory on disk if it doesn't already
//         exist, exactly mirroring what a real production run would do.
//         Matches the already-accepted test_vclcompat.cpp precedent of
//         exercising ForceDirectories for real (tests/test_vclcompat.cpp:374).
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" here == clean
//  g++ compile/link + the SM cursor landing exactly where the golden case
//  ladder's own logic says it must, under a known/deterministic Sim-HAL input.
// =============================================================================
#include "vclcompat/vcl_compat.h"   // AnsiString
#include "cmydef.h"                  // Sn*/Sw*/C_* index constants, SystemYear/Month/Date, bE84LoaderActionflag/bE84UnloaderActionflag, iMMAuto[]
#include "cprod.h"                   // TestIF_File (.iE84TimeOut_K12[2][11])
#include "cpublic.h"                 // GetTimeInfo() (indirectly exercised via ShowE84Log)
#include "mysensor.h"                 // Sen[] (TMySensor Sim HAL)
#include "myswitch.h"                 // SW[] (TMySwitch Sim HAL)
#include "mycylin.h"                  // Cylinder[] (not directly forced by this TU; header needed for MyMotor/cmydef transitively)
#include "Motor/mymotor.h"            // MOT[] (TTrayMotor) -- DoE84Unloader case 1000 dependency
#include "Automation/AGV_E84.h"       // the unit under test
#include <cstdio>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
#include <cstdlib>                    // AI(W906-BA-GL5o) 20260911: getenv / _putenv
#include <cstring>                    // AI(W906-BA-GL5o) 20260911: strstr (redirect assertion)

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the other W6/W5 verify TUs)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  AI(W906-BA-GL5o) 20260911: CONTAINMENT FOR A HAND-RUN OF THIS EXECUTABLE.
//  Migrated from the D:\HT9050 tree (its AI(W906-GL-5o)/GL-5p 20260831).
//
//  tests/CMakeLists.txt redirects five production roots for the whole suite,
//  which covers ctest. It does NOT cover running this .exe directly -- exactly
//  what one does while debugging. This TU drives ShowE84Log/E84StatusLog for
//  real, and on 20260911 a ctest run of it appended to the machine's live
//  D:\HT9045_Log\E84DataTxt tree.
//
//  ⚠ THE DECLARATION MUST BE AT FILE SCOPE: `extern "C"` is a linkage
//  specification and is ILL-FORMED inside a function body. That error appears
//  only on the arm that takes it, so a one-arm syntax error stays invisible
//  until you build that arm.
//
//  ⚠ _putenv vs putenv is NOT a version gap. MinGW.org declares the POSIX
//  names only in GNU mode, and this tree compiles STRICT (CXX_EXTENSIONS OFF):
//      MinGW.org 6.3.0 (ORACLE)   -std=c++14 / c++17   NEITHER declared
//      WinLibs 16.2.0 (non-orac)  -std=c++14 / c++17   both declared
//      MinGW.org 6.3.0            -std=gnu++17         putenv IS declared
//
//  The guard is narrowed on __MINGW64_VERSION_MAJOR rather than declaring
//  _putenv unconditionally: unconditional builds on WinLibs but warns
//  "'int _putenv(const char*)' redeclared without dllimport attribute", and
//  adding a warning to a clean lane to fix another lane is not a fix. That lane
//  matters in this tree now -- build_x64.bat and build_nonoracle.bat arrived
//  here today.
//  (tests/test_bootstrap.cpp:66 already declares _putenv, but unconditionally,
//  and a .cpp's declarations are TU-local, so this file cannot borrow it.)
// ---------------------------------------------------------------------------
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static void RedirectE84LogRoot()
{
    static char buf[512];
    const char* tmp = getenv("TEMP");
    if (tmp == 0) tmp = getenv("TMP");
    if (tmp == 0) tmp = ".";
    snprintf(buf, sizeof(buf), "W906_E84DATA_ROOT=%s\\ht9045_e84_scratch%s", tmp, W906_TestTmpTag().c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
    // ⚠ MUST STAY A CRT CALL. SetEnvironmentVariableA would also compile, and
    // would SILENTLY STOP CONTAINING: it updates the process environment block,
    // not the CRT's own copy that getenv() reads -- and getenv() is what the
    // production code under test consults. Compiling everywhere while quietly
    // not containing is the worst outcome available here.
    HT9045_TEST_PUTENV(buf);
}

// ---------------------------------------------------------------------------
//  Sim-HAL forcing helpers (see file-head "SIM-HAL FORCING TECHNIQUE").
//  Sentinel ISABase == -1 (matches none of the eMotionNet/ePCI1203/ePLCbase/
//  eISABase/ePCI1735U routing enum values, all 0..4 -- see MachineType.h:717).
// ---------------------------------------------------------------------------
static const int kSentinelISABase = -1;

static void ForceSensorOn(TMySensor &s)
{
    s.Enable  = true;
    s.ISABase = kSentinelISABase;
    s.Type    = 0;      // ctor default; IsOn() returns !ret with ret staying false -> true
}

static void ForceSwitchStatusReadable(TMySwitch &sw)
{
    // Makes Status() report exactly whatever the most recent On()/Off() set
    // OutValue to (bypassing the backend entirely).
    sw.Enable  = true;
    sw.ISABase = kSentinelISABase;
    sw.Type    = 1;
}

int main()
{
    // AI(W906-BA-GL5o) 20260911: contain BEFORE anything can write. See the
    // block above for why this is here and not only in tests/CMakeLists.txt.
    RedirectE84LogRoot();
    {
        const char* got = getenv("W906_E84DATA_ROOT");
        printf("E84 log root override: %s\n",
               got ? got : "(NOT SET -- would write production!)");
        // A redirect that failed to apply looks exactly like a pass, so assert it.
        CHECK(got != 0 && strstr(got, "HT9045_Log") == 0,
              "E84 data-log root redirected away from the live machine tree "
              "(production would be D:\\HT9045_Log\\E84DataTxt)");
    }

    printf("=== AGV_E84 (E84 Loader/Unloader handshake) translation verification ===\n");
    printf("(no Borland binary available; verification = compiles/links clean +\n");
    printf(" cursor/oracle values hand-derived from the golden switch(Task) ladder)\n\n");

    // AI(W5-Final-Integrate) 20260711: FIXED -- this "before GetTimeInfo()"
    // check originally lived down in [O5], AFTER O3/O4 had already run
    // DoE84Loader()/DoE84Unloader() through case 1 with sensors forced ON,
    // which unconditionally calls ShowE84Log() (see e.g. AGV_E84.cpp case 1's
    // `ShowE84Log(str,1,1);`), which unconditionally calls GetTimeInfo() at
    // its own top -- so by the time [O5] ran, SystemYear was ALREADY a real
    // calendar year, not the cmydef.cpp ctor sentinel (9999), and this CHECK
    // failed deterministically on every run (first caught when this test was
    // actually linked+run for the first time, this integrate pass). O1/O2
    // never reach a sensors-forced branch (sensors start Enable=false, and
    // case 1's `else` -- unknown position -- is empty, no ShowE84Log call),
    // so SystemYear is still guaranteed to be the untouched ctor sentinel
    // right here, before anything else in this TU has run.
    CHECK(SystemYear == 9999, "SystemYear starts at the cmydef.cpp ctor sentinel (9999) before GetTimeInfo()");

    // -----------------------------------------------------------------------
    // [O1] Cursor reset
    // -----------------------------------------------------------------------
    printf("[O1] Task-init helpers reset both cursors to 1\n");
    iE84LoadTask = 12345;
    iE84UnloadTask = 6789;
    InitialE84LoadTask();
    InitialE84UnLoaderTask();
    CHECK(iE84LoadTask == 1,   "InitialE84LoadTask() -> iE84LoadTask==1");
    CHECK(iE84UnloadTask == 1, "InitialE84UnLoaderTask() -> iE84UnloadTask==1");

    // -----------------------------------------------------------------------
    // [O2] Conservative gate: zero Sim-HAL configuration -> cursor never moves
    // -----------------------------------------------------------------------
    printf("\n[O2] Conservative gate: offline (Enable=false everywhere) never advances past 1\n");
    for (int i = 0; i < 10; ++i)
        DoE84Loader();
    CHECK(iE84LoadTask == 1, "10x DoE84Loader() with disabled Sim HAL -> still Task==1");

    for (int i = 0; i < 10; ++i)
        DoE84Unloader();
    CHECK(iE84UnloadTask == 1, "10x DoE84Unloader() with disabled Sim HAL -> still Task==1");

    // -----------------------------------------------------------------------
    // [O3] Single-tick advance 1 -> 50 (Loader)
    // -----------------------------------------------------------------------
    printf("\n[O3] Forced HOAVBL/GO/CS0 -> single-tick case 1 -> 50 (Loader)\n");
    ForceSwitchStatusReadable(SW[SwE84_1_HOAVBL]);
    ForceSensorOn(Sen[SnE84_1_GO]);
    ForceSensorOn(Sen[SnE84_1_CS0]);
    // TD0 timeout forced to 0s up front so the O4 timeout-recovery round trip
    // below fires on the very next tick (shared [0][9] cell -- see the golden
    // index-bug preserved verbatim in DoE84Unloader's case 1, AGV_E84.cpp).
    TestIF_File.iE84TimeOut_K12[0][9] = 0;

    DoE84Loader();   // tick: case 1 -> 50
    CHECK(iE84LoadTask == 50,        "DoE84Loader() tick1 -> Task==50");
    CHECK(iPlaceWhichBuffer[0] == 10, "DoE84Loader() tick1 -> iPlaceWhichBuffer[0]==10 (golden :248)");

    // -----------------------------------------------------------------------
    // [O4] Timeout-recovery round trip 50 -> 5000 -> 1 (Loader) -- the direct
    // verification of the btInitalLoad->Click() collapse judgment call.
    // -----------------------------------------------------------------------
    printf("\n[O4] Timeout-recovery round trip (verifies the Click()->direct-call collapse)\n");
    DoE84Loader();   // tick: case 50 -> Sensor_VALID not on, TD0 expired -> 5000
    CHECK(iE84LoadTask == 5000, "DoE84Loader() tick2 -> Task==5000 (Auto Recover, TD0 timed out)");

    DoE84Loader();   // tick: case 5000 -> collapsed InitialE84LoadTask()+InitialE84LoadSensor()
    CHECK(iE84LoadTask == 1, "DoE84Loader() tick3 -> Task==1 (collapsed Click() handler fired)");

    // -----------------------------------------------------------------------
    // [O3]/[O4] mirrored for Unloader
    // -----------------------------------------------------------------------
    printf("\n[O3/O4 mirror] Same 2 oracles for DoE84Unloader()\n");
    ForceSwitchStatusReadable(SW[SwE84_2_HOAVBL]);
    ForceSensorOn(Sen[SnE84_2_GO]);
    ForceSensorOn(Sen[SnE84_2_CS0]);
    // TestIF_File.iE84TimeOut_K12[0][9] already forced to 0 above; DoE84Unloader's
    // case 1 arms E84UnLoadDelay from that SAME [0][9] cell (golden index bug).

    DoE84Unloader();  // tick: case 1 -> 50
    CHECK(iE84UnloadTask == 50,        "DoE84Unloader() tick1 -> Task==50");
    CHECK(iPlaceWhichBuffer[1] == 10,  "DoE84Unloader() tick1 -> iPlaceWhichBuffer[1]==10 (golden :609)");

    DoE84Unloader();  // tick: case 50 -> timeout -> 5000
    CHECK(iE84UnloadTask == 5000, "DoE84Unloader() tick2 -> Task==5000 (Auto Recover, TD0 timed out)");

    DoE84Unloader();  // tick: case 5000 -> collapsed InitialE84UnLoaderTask()+InitialE84UnloadSensor()
    CHECK(iE84UnloadTask == 1, "DoE84Unloader() tick3 -> Task==1 (collapsed Click() handler fired)");

    // -----------------------------------------------------------------------
    // [O5] Logging smoke test + GetTimeInfo() wiring
    // -----------------------------------------------------------------------
    printf("\n[O5] ShowE84Log()/E84StatusLog() smoke test + GetTimeInfo() wiring\n");
    // (the "before GetTimeInfo()" half of this oracle now runs at the very top
    // of main(), before O1 -- see the FIXED note there for why)

    ShowE84Log("test message, normal path", 1, 1);       // iLoader==1 -> Loader tag, no ShowMyMessage
    ShowE84Log("test message, error path",  50, 3);       // iLoader==3 -> Loader tag + ShowMyMessage(str) branch
    ShowE84Log("", 999, 2);                               // str=="" -> early-out body, must not crash
    E84StatusLog("SwE84_1_LREQ", true);                   // Off->On branch
    E84StatusLog("SwE84_1_LREQ", false);                  // On->Off branch
    printf("  (ran without crashing)\n");
    ++g_pass;

    CHECK(SystemYear != 9999 && SystemYear >= 2020,
          "SystemYear updated to a real calendar year by GetTimeInfo() (called inside ShowE84Log)");

    // -----------------------------------------------------------------------
    { const char* r = getenv("W906_E84DATA_ROOT"); if (r != 0 && strstr(r, "ht9045_e84_scratch") != 0) W906_TestTmpRemoveTree(r); }   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h): remove this run's E84 sandbox (only ever the one RedirectE84LogRoot set)
    printf("\n=== SUMMARY: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
