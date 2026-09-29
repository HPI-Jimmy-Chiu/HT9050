// =============================================================================
//  test_w6_6_csystem_cycle.cpp  --  W6.6 VERIFY: the INTEGRATED MACHINE CYCLE
//  pumps offline over the Sim HAL (csystem.cpp DoAllProcess + MainProc).
//
//  Translation wave: W6.6 (csystem orchestration HUB -- csystem.cpp, golden
//  25483 lines @ READ-ONLY D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\csystem.cpp)
//  Author: AI(W6.6-HUB) 20260626
//
//  PURPOSE (task-mandated cycle harness)
//  -------------------------------------
//  Plain-assert harness (no framework).  Sets a KNOWN Sim HAL state, drives the
//  main process loop fn (DoAllProcess / MainProc) for N cycles, and asserts:
//    (a) it ticks the FIVE engines in the documented order (observed via the
//        g_csystemTrace tick-sequence oracle compiled into csystem.cpp under
//        CSYSTEM_TICK_ORACLE) without crash;
//    (b) the entry/guard behaviour matches the golden (cited line);
//    (c) the numeric/cursor oracles (MainProc instrumentation) match values
//        hand-derived from the golden.
//
//  This is the *cycle* harness companion to test_w6_6_hub.cpp.  Where the hub TU
//  proves the A/B order shape, this TU is organised strictly around the four
//  HAND-DERIVED golden oracles and adds the GUARD oracle (O3 here) that the hub
//  TU does not exercise: the master-guard early-return at golden :10049 that
//  must emit ONLY [SensorScan] and ZERO engine tokens.
//
//  ORACLES (hand-derived from the golden csystem.cpp -- asserted below):
//    O1  TICK-SEQUENCE (golden DoAllProcess :9115; toggle :10254).  bDoProcess is
//        a function-static initialised TRUE (golden :9120), giving a 2-tick
//        interleave.  Over 4 ticks (T,F,T,F):
//          TICK1(T): SensorScan, DoLoad, DoInArm, Do_Auto_SHT1, DoTestHeadMotor, DoCatchTray
//          TICK2(F): SensorScan, DoLoad, Do_Auto_SHT2, DoTestHeadMotor, DoCatchTray, DoOutArm
//          TICK3(T): == TICK1
//          TICK4(F): == TICK2
//        Invariants: DoInArm/Do_Auto_SHT1 ONLY on odd (bDoProcess==true, golden
//        :10072/:10092); Do_Auto_SHT2/DoOutArm ONLY on even (golden :10098/:10121);
//        DoLoad/DoTestHeadMotor/DoCatchTray EVERY tick; Do_Auto_SHT3/DoSortArm
//        ABSENT (USE_OUT_SORT_ARM==eartUninstall, golden :10112/:10125).
//    O2  INSTRUMENTATION/NUMERIC (golden :16687-16728).  GetMainProcCallCount()==0
//        before first MainProc; +1 each MainProc (golden :16728); ==5 after 5
//        calls.  GetMainProcLastEnterTimeString()=="N/A" before any call (golden
//        :16689-16690), non-"N/A" after.  IsMainProcAlive(60) false before first
//        call (golden :16705-16706), true immediately after.
//    O3  GUARD (golden :10049).  With fAllMotorHome=false (or SystemStart=false
//        or SoftStop=true), DoAllProcess() runs ProcessSensorScan() then returns
//        BEFORE any engine -- the trace contains ONLY [SensorScan], ZERO engine
//        tokens.
//    O4  TOGGLE (golden :10254).  After one DoAllProcess() reaching the tail, the
//        file-static bDoProcess flips, so the engine pattern alternates across
//        consecutive ticks -- verified by pumping N cycles and confirming each is
//        a clean A or B and they strictly alternate.
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" == clean g++
//  compile/link + the integrated cycle PUMPS all five engines over the Sim HAL
//  in the golden-derived order without crash, matching the hand-derived oracles.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                 // SoftStop / SystemStart / fAllMotorHome / InitialOK / AUTO_EMPTY_COLOR / AUTO3_IS_MAGAZINE / TRAY_VIBRATION ...
#include "cprod.h"                  // TrayForm.bEnableAMR
#include "MachineType.h"            // NonVibration / eartUninstall
#include "csystem.h"                // MainProc / InitAllProcessTask / GetMainProcCallCount / IsMainProcAlive / GetMainProcLastEnterTimeString
#include "csystem_shims.h"          // bShuttleShake
#include "FormsFacade.h"            // fMain
//AI(W906-W7-L1-W3fixA) 20260802: for the O5 WIRING PIN below.
#include "asendic.h"                // DoLoaderTrayFeed / InitDoLoaderTrayFeedTask (golden asendic.h:34-35)
#include "Motor/mymotor.h"          // MOT[] / TTrayMotor::fHasTray (the tray the pin seeds)
#include <cstdio>
#include <vector>
#include "w906_test_motors.h"  // AI(W906-T6-MAINPROC) 20260923: MOT[].Motor 的開機不變式（見該檔檔頭）

// The tick-sequence oracle trace lives in csystem.cpp (compiled with
// CSYSTEM_TICK_ORACLE for the ht9045_sm lib).  Bind to it here.
extern std::vector<int> g_csystemTrace;
extern void csystemTraceClear();

// DoAllProcess is the file-internal per-tick spine driver MainProc calls (golden
// keeps it extern-visible but only MainProc calls it); not declared in
// csystem.h.  Forward-declare its exact signature so we can pump it directly.
void DoAllProcess();

// engine-id constants the oracle keys on (must match the csystem.cpp enum).
enum {
    CT_SENSORSCAN = 1, CT_DOLOAD = 2, CT_DOINARM = 3, CT_SHT1 = 4, CT_SHT2 = 5,
    CT_SHT3 = 6, CT_TESTHEAD = 7, CT_CATCHTRAY = 8, CT_OUTARM = 9, CT_SORTARM = 10
};

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static const char* engName(int id)
{
    switch (id) {
        case CT_SENSORSCAN: return "SensorScan";
        case CT_DOLOAD:     return "DoLoad";
        case CT_DOINARM:    return "DoInArm";
        case CT_SHT1:       return "Do_Auto_SHT1";
        case CT_SHT2:       return "Do_Auto_SHT2";
        case CT_SHT3:       return "Do_Auto_SHT3";
        case CT_TESTHEAD:   return "DoTestHeadMotor";
        case CT_CATCHTRAY:  return "DoCatchTray";
        case CT_OUTARM:     return "DoOutArm";
        case CT_SORTARM:    return "DoSortArm";
        default:            return "?";
    }
}

static void dumpTrace(const char* tag)
{
    printf("    %s trace = [", tag);
    for (size_t i = 0; i < g_csystemTrace.size(); ++i)
        printf("%s%s", engName(g_csystemTrace[i]),
               (i + 1 < g_csystemTrace.size()) ? ", " : "");
    printf("]\n");
}

static bool traceEquals(const std::vector<int>& exp)
{
    if (g_csystemTrace.size() != exp.size()) return false;
    for (size_t i = 0; i < exp.size(); ++i)
        if (g_csystemTrace[i] != exp[i]) return false;
    return true;
}

static bool traceContains(const std::vector<int>& v, int id)
{
    for (size_t i = 0; i < v.size(); ++i) if (v[i] == id) return true;
    return false;
}

// Put DoAllProcess's master guards + tail-loop controls into the offline "Run"
// state so the engine spine executes fully (no early-return, no Auto-tray receive
// loop, no vibration, no alignment yield).  These are the KNOWN Sim HAL inputs.
static void RunGuards()
{
    InitialOK             = true;          // MainProc head guard (golden :16724)
    SoftStop              = false;         // master guard (golden :10049)
    SystemStart           = true;
    fAllMotorHome         = true;
    bShuttleShake         = false;         // run shuttle/index engines (golden :10077)

    TrayForm.bEnableAMR   = false;         // plain DoLoad / DoAutoReceive path
    USE_OUT_SORT_ARM      = eartUninstall; // no Do_Auto_SHT3 / DoSortArm (golden :10112/:10125)

    AUTO_EMPTY_COLOR      = 0;             // skip DoAutoReceiveBinTray loop + Empty/Color
    AUTO3_IS_MAGAZINE     = 0;
    TRAY_VIBRATION        = NonVibration;
    SUPPORT_2_EMPTY_EMPTY  = false;
    bUseAuto2Empty        = false;

    bLoaderNeedTrayMustFinish = false;
    bAutoNeedTrayMustFinish   = false;
    bRunInArmAutoAlignment    = false;     // no alignment yield-return (golden :10231)
    bRunOutArmAutoAlignment   = false;
}

// Classify one observed tick trace as a clean A (odd) or B (even) tick.
//   A: has DoInArm + SHT1, no OutArm, no SHT2
//   B: has DoOutArm + SHT2, no DoInArm, no SHT1
static int classifyTick(const std::vector<int>& v)  // returns 1=A, 2=B, 0=neither
{
    bool inarm = traceContains(v, CT_DOINARM);
    bool sht1  = traceContains(v, CT_SHT1);
    bool outarm= traceContains(v, CT_OUTARM);
    bool sht2  = traceContains(v, CT_SHT2);
    if ( inarm &&  sht1 && !outarm && !sht2) return 1;
    if (!inarm && !sht1 &&  outarm &&  sht2) return 2;
    return 0;
}

int main()
{
    // AI(W906-T6-MAINPROC) 20260923: T6 讓 MainProc 照 golden 每拍呼叫 DoSystem -> ScanAllMotorStatus，
    //   它直接讀 MOT[i].Motor->Enable；golden 開機後 MOT[].Motor 必非 NULL，本測試沒跑 InitialMotorParameter，
    //   所以先補模擬馬達物件（tests/w906_test_motors.h）。
    W906_TestEnsureSimMotors();
    printf("==== W6.6 csystem INTEGRATED MACHINE CYCLE verify (DoAllProcess/MainProc over Sim HAL) ====\n");

    // -------------------------------------------------------------------------
    //  O2 pre-state: instrumentation BEFORE any MainProc() call.  Asserted FIRST,
    //  before we touch MainProc, so the gbMainProcEntered==false branch (golden
    //  :16689/:16705) is genuinely observed.
    // -------------------------------------------------------------------------
    printf("[O2-pre] MainProc instrumentation BEFORE first call (golden :16689/:16705/:16728)\n");
    CHECK(GetMainProcCallCount() == 0u,
          "O2-pre GetMainProcCallCount()==0 before any MainProc() (golden :16728 counter starts 0)");
    CHECK(GetMainProcLastEnterTimeString() == AnsiString("N/A"),
          "O2-pre GetMainProcLastEnterTimeString()==\"N/A\" before any call (golden :16689-16690)");
    CHECK(IsMainProcAlive(60) == false,
          "O2-pre IsMainProcAlive(60)==false before any call (golden :16705-16706 gbMainProcEntered==false)");

    // KNOWN Sim HAL state + deterministic per-engine cursor reset.
    RunGuards();
    InitAllProcessTask();
    printf("[init] RunGuards() + InitAllProcessTask() (per-engine cursor reset; golden :5698)\n");

    // =========================================================================
    //  O1 -- TICK-SEQUENCE over 4 ticks (T,F,T,F).  bDoProcess is a function-
    //  static starting TRUE (golden :9120), so the 1st DoAllProcess() is TICK1(T),
    //  the 2nd TICK2(F), etc.  Expected token streams hand-derived from golden
    //  :9115-10255 with USE_OUT_SORT_ARM==eartUninstall.
    // =========================================================================
    printf("[O1] 4-tick documented engine order (golden :9115-10255, toggle :10254)\n");

    std::vector<int> expT;  // odd tick (bDoProcess==true)
    expT.push_back(CT_SENSORSCAN);
    expT.push_back(CT_DOLOAD);
    expT.push_back(CT_DOINARM);
    expT.push_back(CT_SHT1);
    expT.push_back(CT_TESTHEAD);
    expT.push_back(CT_CATCHTRAY);

    std::vector<int> expF;  // even tick (bDoProcess==false)
    expF.push_back(CT_SENSORSCAN);
    expF.push_back(CT_DOLOAD);
    expF.push_back(CT_SHT2);
    expF.push_back(CT_TESTHEAD);
    expF.push_back(CT_CATCHTRAY);
    expF.push_back(CT_OUTARM);

    const std::vector<int>* expSeq[4] = { &expT, &expF, &expT, &expF };
    const char* tickName[4] = { "TICK1(T)", "TICK2(F)", "TICK3(T)", "TICK4(F)" };

    for (int t = 0; t < 4; ++t)
    {
        csystemTraceClear();
        DoAllProcess();                         // pump one tick of the integrated cycle
        dumpTrace(tickName[t]);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "O1 %s engine order matches golden-derived stream (golden :9115-10255)",
                 tickName[t]);
        CHECK(traceEquals(*expSeq[t]), msg);
    }

    // O1 invariants re-checked over a fresh T/F pair (re-derived from observed
    // traces, not the expectation vectors).
    {
        csystemTraceClear(); DoAllProcess(); std::vector<int> obsA = g_csystemTrace; // T
        csystemTraceClear(); DoAllProcess(); std::vector<int> obsB = g_csystemTrace; // F

        CHECK( traceContains(obsA, CT_DOINARM) && !traceContains(obsB, CT_DOINARM),
               "O1-inv DoInArm ONLY on odd tick (bDoProcess==true, golden :10072)");
        CHECK( traceContains(obsB, CT_OUTARM) && !traceContains(obsA, CT_OUTARM),
               "O1-inv DoOutArm ONLY on even tick (bDoProcess==false, golden :10121)");
        CHECK( traceContains(obsA, CT_SHT1) && !traceContains(obsA, CT_SHT2) &&
               traceContains(obsB, CT_SHT2) && !traceContains(obsB, CT_SHT1),
               "O1-inv Do_Auto_SHT1 on odd, Do_Auto_SHT2 on even, mutually exclusive (golden :10092)");
        CHECK( traceContains(obsA, CT_DOLOAD)   && traceContains(obsB, CT_DOLOAD) &&
               traceContains(obsA, CT_TESTHEAD) && traceContains(obsB, CT_TESTHEAD) &&
               traceContains(obsA, CT_CATCHTRAY)&& traceContains(obsB, CT_CATCHTRAY),
               "O1-inv DoLoad/DoTestHeadMotor/DoCatchTray on EVERY tick (golden :10054/:10115/:10135)");
        CHECK( !traceContains(obsA, CT_SHT3) && !traceContains(obsB, CT_SHT3) &&
               !traceContains(obsA, CT_SORTARM) && !traceContains(obsB, CT_SORTARM),
               "O1-inv Do_Auto_SHT3/DoSortArm ABSENT (USE_OUT_SORT_ARM==eartUninstall, golden :10112/:10125)");
    }

    // =========================================================================
    //  O3 -- GUARD: with fAllMotorHome=false the master guard at golden :10049
    //  returns BEFORE any engine.  Trace must contain ONLY [SensorScan].
    //  (We restore Run state afterwards.)  Tested with all three guard variables.
    // =========================================================================
    printf("[O3] master-guard early-return emits ONLY SensorScan (golden :10049)\n");
    {
        std::vector<int> expScanOnly; expScanOnly.push_back(CT_SENSORSCAN);

        fAllMotorHome = false;
        csystemTraceClear(); DoAllProcess(); dumpTrace("guard fAllMotorHome=false");
        CHECK(traceEquals(expScanOnly),
              "O3a fAllMotorHome==false -> ONLY [SensorScan], zero engines (golden :10049)");
        fAllMotorHome = true;

        SystemStart = false;
        csystemTraceClear(); DoAllProcess(); dumpTrace("guard SystemStart=false");
        CHECK(traceEquals(expScanOnly),
              "O3b SystemStart==false -> ONLY [SensorScan], zero engines (golden :10049)");
        SystemStart = true;

        SoftStop = true;
        csystemTraceClear(); DoAllProcess(); dumpTrace("guard SoftStop=true");
        CHECK(traceEquals(expScanOnly),
              "O3c SoftStop==true -> ONLY [SensorScan], zero engines (golden :10049)");
        SoftStop = false;
    }

    // =========================================================================
    //  O4 -- TOGGLE / PUMP STABILITY: pump N cycles, confirm each tick is a clean
    //  A or B and they STRICTLY ALTERNATE (the file-static bDoProcess flip at
    //  golden :10254), with no crash.
    //  NOTE: O3 above pumped an ODD number of guard ticks (3) -- but every guarded
    //  tick returns at :10049 BEFORE the :10254 toggle, so bDoProcess parity is
    //  UNCHANGED by guarded ticks.  We therefore do not assume a specific start
    //  parity here; we only assert clean-A-or-B + strict alternation.
    // =========================================================================
    printf("[O4] N=64-cycle pump: clean A/B each tick + strict alternation + no crash (golden :10254)\n");
    {
        const int N = 64;
        bool ok = true;
        int prev = 0;                       // 0=unknown, 1=A, 2=B
        int countA = 0, countB = 0;
        for (int cyc = 0; cyc < N && ok; ++cyc)
        {
            csystemTraceClear();
            DoAllProcess();                 // pump one integrated machine cycle tick
            int cls = classifyTick(g_csystemTrace);
            if (cls == 0) { ok = false; printf("    cycle %d: neither clean A nor B\n", cyc); break; }
            if (cls == 1) ++countA; else ++countB;
            // head must always be SensorScan, DoLoad (golden :9130/:10054)
            if (g_csystemTrace.size() < 2 ||
                g_csystemTrace[0] != CT_SENSORSCAN || g_csystemTrace[1] != CT_DOLOAD)
                { ok = false; printf("    cycle %d: head not [SensorScan,DoLoad]\n", cyc); break; }
            // strict alternation
            if (prev != 0 && cls == prev)
                { ok = false; printf("    cycle %d: parity did not toggle (golden :10254)\n", cyc); break; }
            prev = cls;
        }
        printf("    pumped %d cycles: %d A-ticks, %d B-ticks\n", N, countA, countB);
        CHECK(ok, "O4 64-cycle integrated pump: every tick clean A/B, head=[SensorScan,DoLoad], strict toggle, no crash (golden :9115-10255)");
        CHECK(countA + countB == N, "O4 all 64 cycles classified (no dropped/extra ticks)");
    }

    // =========================================================================
    //  O2 -- INSTRUMENTATION/NUMERIC via MainProc() (golden :16711).  MainProc
    //  dispatches the integrated spine (DoAllProcess + finish-checks).  Pump 5
    //  MainProc() calls; CallCount must land on exactly 5, alive flags flip true.
    // =========================================================================
    printf("[O2] MainProc instrumentation + integrated dispatch over 5 calls (golden :16711-16728)\n");
    {
        unsigned int before = GetMainProcCallCount();
        for (int i = 0; i < 5; ++i)
        {
            unsigned int pre = GetMainProcCallCount();
            MainProc();                     // outer per-frame entry -> DoAllProcess + finish checks
            unsigned int post = GetMainProcCallCount();
            if (post != pre + 1)
                printf("    MainProc call %d: count %u -> %u (expected +1)\n", i, pre, post);
        }
        unsigned int after = GetMainProcCallCount();
        printf("    GetMainProcCallCount %u -> %u over 5 MainProc()\n", before, after);
        CHECK(after == before + 5u,
              "O2a GetMainProcCallCount increments by exactly 5 over 5 MainProc() (golden :16728)");
        CHECK(IsMainProcAlive(60) == true,
              "O2b IsMainProcAlive(60)==true immediately after MainProc() (golden :16703-16708)");
        CHECK(GetMainProcLastEnterTimeString() != AnsiString("N/A"),
              "O2c GetMainProcLastEnterTimeString()!=\"N/A\" after MainProc() (golden :16687-16692)");
    }

    // =========================================================================
    //  O5 -- WIRING PIN (AI(W906-W7-L1-W3fixA) 20260802).
    //
    //  WHY THIS EXISTS.  Wave 3 landed golden's real DoLoaderTrayFeed
    //  (golden asendic.cpp:1520-1537) but csystem.cpp kept
    //  `#define DoLoaderTrayFeed W7C2_DoLoaderTrayFeed`, a `{ return true; }`
    //  stand-in.  csystem.cpp is golden's ONLY caller, so the landed body was
    //  unreachable -- and NOTHING in the suite could tell, because every test
    //  ran with bDoLoaderCleanOut==false, where the two bodies agree.  This
    //  block closes that window: it drives DoOneCycleFinishCheck's
    //  `else if(iCleanOut==1)` branch (golden csystem.cpp:14016-14031) into the
    //  ONE state where stub and real body DISAGREE, so re-introducing the
    //  redirect turns this red instead of staying silently green.
    //
    //  THE DISCRIMINATOR.  In that branch golden reads:
    //      if(DoLoaderTrayFeed()==false) return;
    //      else { bDoLoaderCleanOut=false; bLoadBFBackTray=false; }
    //  Golden's case 1 returns TRUE only when the loader is empty, and returns
    //  FALSE (after arming a feed sub-task) when a tray is present.  So:
    //      tray present  ->  real body false -> flag SURVIVES ; stub -> cleared
    //      loader empty  ->  real body true  -> flag CLEARED   ; stub -> cleared
    //  O5a and O5b assert both halves.  O5b is the negative control that keeps
    //  O5a honest -- a body that simply always returned false would pass O5a and
    //  fail O5b, so the pair cannot be satisfied by any constant function.
    // =========================================================================
    printf("[O5] WIRING PIN: DoOneCycleFinishCheck reaches the REAL DoLoaderTrayFeed (golden :14016-14031)\n");
    {
        // --- save every global this pin touches ------------------------------
        const int  saveOneCycle   = iOneCycle;
        const int  saveCleanOut   = iCleanOut;
        const bool saveDoLdClean  = bDoLoaderCleanOut;
        const bool saveBFBack     = bLoadBFBackTray;
        const bool saveHasTray    = MOT[MMTrayY].fHasTray;

        // iOneCycle==0 skips the big if(iOneCycle) block (golden :12820) so the
        // else-if branch below is the ONLY code that runs.
        iOneCycle        = 0;
        iCleanOut        = 1;
        bLoadBFBackTray  = false;

        // --- O5a: tray present -> real body returns false -> flag survives ----
        InitDoLoaderTrayFeedTask();          // cursor -> 1 (golden asendic.cpp:1515)
        bDoLoaderCleanOut     = true;
        MOT[MMTrayY].fHasTray = true;        // "Has Tray" (golden asendic.cpp:1542)
        DoOneCycleFinishCheck();
        CHECK(bDoLoaderCleanOut == true,
              "O5a tray present -> DoLoaderTrayFeed()==false -> bDoLoaderCleanOut SURVIVES "
              "(the `return true` stand-in would have cleared it; golden :14021-14029)");

        // --- O5b: loader empty -> real body returns true -> flag cleared ------
        InitDoLoaderTrayFeedTask();          // re-arm; O5a left the cursor at 2000
        bDoLoaderCleanOut     = true;
        MOT[MMTrayY].fHasTray = false;       // loader empty -> golden's case 1 `return true`
        DoOneCycleFinishCheck();
        CHECK(bDoLoaderCleanOut == false,
              "O5b loader empty -> DoLoaderTrayFeed()==true -> bDoLoaderCleanOut CLEARED "
              "(negative control: the body is not a constant false)");

        // --- restore ----------------------------------------------------------
        InitDoLoaderTrayFeedTask();
        MOT[MMTrayY].fHasTray = saveHasTray;
        bLoadBFBackTray       = saveBFBack;
        bDoLoaderCleanOut     = saveDoLdClean;
        iCleanOut             = saveCleanOut;
        iOneCycle             = saveOneCycle;
    }

    printf("\n==== W6.6 csystem CYCLE verify summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
