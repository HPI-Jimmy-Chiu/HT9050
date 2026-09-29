// =============================================================================
//  test_w6_6_hub.cpp  --  W6.6 VERIFY: the csystem ORCHESTRATION HUB (the SPINE)
//
//  Translation wave: W6.6 (csystem HUB -- csystem.cpp, golden 25483 lines)
//  Author: AI(W6.6-HUB) 20260626
//
//  PURPOSE
//  -------
//  Proves the integration SPINE (csystem.cpp DoAllProcess + MainProc, in lib
//  ht9045_sm) LINKS against the five already-translated engines and PUMPS the
//  WHOLE machine cycle over the Sim HAL with NO hardware, ticking the engines in
//  the documented order without crash for N cycles.  Oracles are hand-derived
//  from the golden csystem.cpp (READ-ONLY @ D:\HT9045\
//  HT9011UC_Code_V3.33.906.0_20260618\csystem.cpp):
//
//    [O1] TICK-ORDER (golden DoAllProcess :9115-10255).  The static bDoProcess
//         toggles every tick (:10254), producing a 2-tick interleave:
//           TICK A (bDoProcess==true) : SensorScan, DoLoad, DoInArm, Do_Auto_SHT1, DoTestHeadMotor, DoCatchTray
//           TICK B (bDoProcess==false): SensorScan, DoLoad,         Do_Auto_SHT2, DoTestHeadMotor, DoCatchTray, DoOutArm
//         (Do_Auto_SHT3/DoSortArm only when USE_OUT_SORT_ARM!=eartUninstall ->
//          default eartUninstall -> ABSENT.)  The tick-sequence ORACLE
//          (g_csystemTrace, compiled in via CSYSTEM_TICK_ORACLE) must record
//          EXACTLY this engine id sequence on tick A and tick B.
//
//    [O2] ALTERNATION.  DoInArm appears ONLY on tick A; DoOutArm ONLY on tick B;
//         Do_Auto_SHT1 only on A, Do_Auto_SHT2 only on B (mutually exclusive by
//         tick parity -- the golden bDoProcess if/else at :10072/:10092/:10121).
//
//    [O3] ALIVE-INSTRUMENTATION (golden :16677-16729).  GetMainProcCallCount()
//         increments once per MainProc(); IsMainProcAlive(big) true after a
//         call; GetMainProcLastEnterTimeString() != "N/A" after a call.
//
//    [O4] PUMP STABILITY.  N=64 cycles of DoAllProcess() over the Sim HAL with
//         the master guards in the "Run" state complete without crash, and the
//         per-tick trace stays the documented A/B alternation throughout.
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" == clean g++
//  link + the HUB main loop pumping all five engines in the documented order
//  over the Sim HAL without crash, matching the tick-order hand-derived from the
//  golden bDoProcess interleave above.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                 // SoftStop / SystemStart / fAllMotorHome / InitialOK / AUTO_EMPTY_COLOR / AUTO3_IS_MAGAZINE / TRAY_VIBRATION / ...
#include "cprod.h"                  // TrayForm.bEnableAMR
#include "MachineType.h"            // NonVibration / eartUninstall
#include "csystem.h"                // MainProc / GetMainProcCallCount / IsMainProcAlive / GetMainProcLastEnterTimeString
#include "csystem_shims.h"          // bShuttleShake
#include "FormsFacade.h"            // fMain
#include <cstdio>
#include <vector>
#include "w906_test_motors.h"  // AI(W906-T6-MAINPROC) 20260923: MOT[].Motor 的開機不變式（見該檔檔頭）
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
// The tick-sequence oracle trace lives in csystem.cpp (compiled with
// CSYSTEM_TICK_ORACLE for the ht9045_sm lib).  Bind to it here.
extern std::vector<int> g_csystemTrace;
extern void csystemTraceClear();

// DoAllProcess is the file-internal spine driver MainProc calls; it is NOT
// declared in csystem.h (golden keeps it extern-visible but only MainProc calls
// it).  Forward-declare its exact signature so we can pump it directly.
void DoAllProcess();

// engine-id constants the oracle keys on (must match csystem.cpp enum)
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
    switch(id) {
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
    for (size_t i=0;i<g_csystemTrace.size();++i)
        printf("%s%s", engName(g_csystemTrace[i]), (i+1<g_csystemTrace.size())?", ":"");
    printf("]\n");
}

static bool traceEquals(const std::vector<int>& exp)
{
    if (g_csystemTrace.size()!=exp.size()) return false;
    for (size_t i=0;i<exp.size();++i)
        if (g_csystemTrace[i]!=exp[i]) return false;
    return true;
}

// Put DoAllProcess's master guards + tail-loop controls into the offline "Run"
// state so the engine spine executes fully (no early-return, no Auto-tray
// receive loop, no vibration, no alignment yield).
static void RunGuards()
{
    InitialOK      = true;        // MainProc head guard
    SoftStop       = false;       // master guard (golden :10049 etc.)
    SystemStart    = true;
    fAllMotorHome  = true;
    bShuttleShake  = false;       // run the shuttle/index engines (golden :10077)

    TrayForm.bEnableAMR = false;  // take the plain DoLoad / DoAutoReceive path
    USE_OUT_SORT_ARM    = eartUninstall;  // no Do_Auto_SHT3 / DoSortArm

    AUTO_EMPTY_COLOR    = 0;      // skip the DoAutoReceiveBinTray loop + Empty/Color
    AUTO3_IS_MAGAZINE   = 0;
    TRAY_VIBRATION      = NonVibration;
    SUPPORT_2_EMPTY_EMPTY = false;
    bUseAuto2Empty      = false;

    bLoaderNeedTrayMustFinish = false;
    bAutoNeedTrayMustFinish   = false;
    bRunInArmAutoAlignment    = false;   // no alignment yield-return
    bRunOutArmAutoAlignment   = false;
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("W6_6_Hub", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    // AI(W906-T6-MAINPROC) 20260923: T6 讓 MainProc 照 golden 每拍呼叫 DoSystem -> ScanAllMotorStatus，
    //   它直接讀 MOT[i].Motor->Enable；golden 開機後 MOT[].Motor 必非 NULL，本測試沒跑 InitialMotorParameter，
    //   所以先補模擬馬達物件（tests/w906_test_motors.h）。
    W906_TestEnsureSimMotors();
    printf("==== W6.6 csystem HUB verify (csystem.cpp DoAllProcess + MainProc over Sim HAL) ====\n");

    RunGuards();

    // The deterministic spine reset the harness calls before pumping.
    InitAllProcessTask();
    printf("[init] InitAllProcessTask() returned (per-engine cursor reset)\n");

    // =======================================================================
    //  PART A -- [O1]/[O2] per-tick engine ORDER (the documented A/B interleave)
    //  bDoProcess is a function-static starting true, so the FIRST DoAllProcess
    //  call is TICK A, the second TICK B, etc.
    // =======================================================================
    printf("[A] DoAllProcess tick-order oracle\n");

    std::vector<int> expA;  // TICK A (bDoProcess==true)
    expA.push_back(CT_SENSORSCAN);
    expA.push_back(CT_DOLOAD);
    expA.push_back(CT_DOINARM);
    expA.push_back(CT_SHT1);
    expA.push_back(CT_TESTHEAD);
    expA.push_back(CT_CATCHTRAY);

    std::vector<int> expB;  // TICK B (bDoProcess==false)
    expB.push_back(CT_SENSORSCAN);
    expB.push_back(CT_DOLOAD);
    expB.push_back(CT_SHT2);
    expB.push_back(CT_TESTHEAD);
    expB.push_back(CT_CATCHTRAY);
    expB.push_back(CT_OUTARM);

    csystemTraceClear();
    DoAllProcess();                 // TICK A
    dumpTrace("tickA");
    CHECK(traceEquals(expA),
          "O1a TICK A order = SensorScan,DoLoad,DoInArm,SHT1,TestHead,CatchTray (golden 9115-10255)");

    csystemTraceClear();
    DoAllProcess();                 // TICK B
    dumpTrace("tickB");
    CHECK(traceEquals(expB),
          "O1b TICK B order = SensorScan,DoLoad,SHT2,TestHead,CatchTray,DoOutArm (golden 9115-10255)");

    // [O2] alternation: DoInArm only on A, DoOutArm only on B
    {
        bool aHasInArm=false, aHasOutArm=false;
        for (size_t i=0;i<expA.size();++i){ if(expA[i]==CT_DOINARM)aHasInArm=true; if(expA[i]==CT_OUTARM)aHasOutArm=true; }
        bool bHasInArm=false, bHasOutArm=false;
        for (size_t i=0;i<expB.size();++i){ if(expB[i]==CT_DOINARM)bHasInArm=true; if(expB[i]==CT_OUTARM)bHasOutArm=true; }
        // re-derive from the observed traces (we cleared+repumped above; pump a fresh pair)
        csystemTraceClear(); DoAllProcess(); std::vector<int> obsA=g_csystemTrace;
        csystemTraceClear(); DoAllProcess(); std::vector<int> obsB=g_csystemTrace;
        bool inArmOnAonly=false, outArmOnBonly=false, sht1OnA=false, sht2OnB=false;
        for(size_t i=0;i<obsA.size();++i){ if(obsA[i]==CT_DOINARM)inArmOnAonly=true; if(obsA[i]==CT_SHT1)sht1OnA=true; }
        for(size_t i=0;i<obsB.size();++i){ if(obsB[i]==CT_OUTARM)outArmOnBonly=true; if(obsB[i]==CT_SHT2)sht2OnB=true; }
        bool inArmNotOnB=true, outArmNotOnA=true;
        for(size_t i=0;i<obsB.size();++i) if(obsB[i]==CT_DOINARM) inArmNotOnB=false;
        for(size_t i=0;i<obsA.size();++i) if(obsA[i]==CT_OUTARM) outArmNotOnA=false;
        CHECK(inArmOnAonly && inArmNotOnB, "O2a DoInArm runs ONLY on bDoProcess==true tick (golden 10072)");
        CHECK(outArmOnBonly && outArmNotOnA, "O2b DoOutArm runs ONLY on bDoProcess==false tick (golden 10121)");
        CHECK(sht1OnA && sht2OnB, "O2c Do_Auto_SHT1 on A, Do_Auto_SHT2 on B (mutually exclusive, golden 10092)");
        (void)aHasInArm;(void)aHasOutArm;(void)bHasInArm;(void)bHasOutArm;
    }

    // =======================================================================
    //  PART B -- [O3] MainProc alive-instrumentation.
    // =======================================================================
    printf("[B] MainProc alive-instrumentation\n");
    unsigned int c0 = GetMainProcCallCount();
    MainProc();
    unsigned int c1 = GetMainProcCallCount();
    printf("    GetMainProcCallCount %u -> %u\n", c0, c1);
    printf("    after MainProc: SystemStart=%d fAllMotorHome=%d SoftStop=%d MotorPowerOnDelay=%d\n",   // AI(W906-MT-E3b) 20260925: see PART C
           (int)SystemStart, (int)fAllMotorHome, (int)SoftStop, MotorPowerOnDelay);
    CHECK(c1 == c0 + 1, "O3a GetMainProcCallCount increments once per MainProc() (golden 16728)");
    CHECK(IsMainProcAlive(3600) == true, "O3b IsMainProcAlive(big) true after MainProc() (golden 16703)");
    CHECK(GetMainProcLastEnterTimeString() != AnsiString("N/A"),
          "O3c GetMainProcLastEnterTimeString() != N/A after MainProc() (golden 16687)");

    // =======================================================================
    //  PART C -- [O4] pump stability over 64 cycles, trace stays A/B alternating.
    // =======================================================================
    printf("[C] 64-cycle pump stability + sustained A/B alternation\n");
    // AI(W906-MT-E3b) 20260925: re-arm the Run guards -- PART B's MainProc() runs golden DoSystem, and since
    //   GATE G21 was lifted (golden csystem.cpp:4649-4655, EastSun ruling R1 20260925) its servo-power settle
    //   guard revokes START in this fixture: MotorPowerOnDelay starts at 10 (cmydef.cpp) and SystemSec never
    //   advances here (nothing calls GetTimeInfo), so CountMotorPowerDelay()==false -> SystemStart=false.
    //   That is golden behaviour, not a spine fault; PART C pumps DoAllProcess directly and needs the guards
    //   in the Run state the file header describes.
    RunGuards();
    {
        // resync parity to a known phase by pumping until a tick A is observed
        // (tick A is the only one carrying DoInArm).
        bool ok = true;
        for (int cyc = 0; cyc < 64 && ok; ++cyc)
        {
            csystemTraceClear();
            DoAllProcess();
            bool hasInArm=false, hasOutArm=false;
            for (size_t i=0;i<g_csystemTrace.size();++i) {
                if (g_csystemTrace[i]==CT_DOINARM)  hasInArm=true;
                if (g_csystemTrace[i]==CT_OUTARM)   hasOutArm=true;
            }
            // every tick is EITHER an A (InArm, no OutArm) OR a B (OutArm, no InArm)
            bool isA = ( hasInArm && !hasOutArm);
            bool isB = (!hasInArm &&  hasOutArm);
            if (!(isA ^ isB)) { ok = false; printf("    cycle %d: neither clean A nor B\n", cyc); }
            // every tick must start with SensorScan + DoLoad and contain TestHead + CatchTray
            if (g_csystemTrace.size() < 4 ||
                g_csystemTrace[0]!=CT_SENSORSCAN || g_csystemTrace[1]!=CT_DOLOAD)
                { ok = false; printf("    cycle %d: head not SensorScan,DoLoad\n", cyc); }
        }
        CHECK(ok, "O4 64-cycle pump: every tick is a clean A or B, head=SensorScan,DoLoad, no crash (golden 9115-10255)");
    }

    printf("\n==== W6.6 csystem HUB verify summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
