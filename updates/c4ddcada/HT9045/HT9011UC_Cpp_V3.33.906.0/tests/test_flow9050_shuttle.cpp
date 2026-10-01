// =============================================================================
//  test_flow9050_shuttle.cpp  --  F-01b batch A VERIFY: the HT9050 single-pair
//  In / Out Shuttle state machines and the four small Type_HT9050 branches.
//
//  AI(W906-FLOW9050-A) 20261001: new file (ctest Flow9050_Shuttle).
//
//  WHAT IS UNDER TEST (all translated line by line from Frank's 910 tree,
//  ref/frank-910-9050 beba23ee, HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch):
//    H063  acarry.cpp      CheckIndexStatusERRSH1 / Do_Auto_InSH / Do_Auto_OutSH   (910 acarry.cpp:8522-8804)
//    H089  csystem_predicates.cpp  OutSHT1InLF / OutSHT1InRT two-arm bodies     (910 csystem.cpp:701-768)
//    H093  csystem.cpp     DoAllProcess shuttle dispatch, Type_HT9050 arm       (910 csystem.cpp:10360-10386)
//    H074  aoutarm9045.cpp InitialOutArmNeedSuck, Type_HT9050 arm               (910 aoutarm9045.cpp:3581-3591)
//    H004  ainarm9045.cpp  MoveInArmXYToWaitTrayArm, Type_HT9050 arm            (910 ainarm9045.cpp:834-858)
//    H003  ainarm2.cpp     AdjustShuttlePlaceOrder, Type_HT9050 arm             (910 ainarm2.cpp:714-718)
//  Every new behaviour is inside MachineTypeChoice==Type_HT9050; today 9050GPIB
//  decodes to Type_HT9046_LS (database.cpp:517), so the [L*] parts pin that the
//  Type_HT9046_LS answers are the old ones.
//
//  ORACLES are hand-derived from the 910 text (NOT from the translation):
//    [D]  DoAllProcess (H093): Type_HT9050 runs Do_Auto_InSH on the bDoProcess
//         tick and Do_Auto_OutSH on the other, never Do_Auto_SHT1/SHT2.  Told
//         apart by case 1: Do_Auto_InSH/OutSH wait for MOT[MTestZ1].ReadPos()==
//         Prod.TestZ1_Safe (910 :8559/:8645), Do_Auto_SHT1/SHT2 do not
//         (acarry.cpp case 1 -> Task=10).  CSYS_TICK keeps CT_SHT1/CT_SHT2 (the
//         shuttle slot), so the trace says WHICH SLOT ran and the cursor says WHO.
//    [A]  CheckIndexStatusERRSH1 == false offline (Y1/Z1 axes disabled; and
//         Gali_ReadEncoderBelowCheckHeight is false under SOFT_SIMULTE anyway).
//    [I]  Do_Auto_InSH: case 1 gates (IsCanMove = 4 flags, bContraPoisitionFlag,
//         Z1 safe, exact ==); case 10 routing (910 :8566-8590); case 100 move
//         left -> MOutShuttle1.fCanMove=true, b1ShuttleMoveToLeft=true, Task=1
//         (910 :8600-8605); case 200 blocked by OutSHT1InLF (910 :8610), move
//         right and only when OutSHT1InRT -> MOutShuttle1.fCanMove=false,
//         b1ShuttleMoveToLeft=false, Task=1 (910 :8613-8620); the testing-stop
//         and SystemNG early returns (910 :8532 / :8549).
//    [O]  Do_Auto_OutSH: case 1/10 (910 :8640-8655); case 100 needs InSHT1InLF,
//         move left -> MInShuttle1.fCanMove=false, Task=1 (910 :8657-8698);
//         case 200 move right -> MInShuttle1.fCanMove=true, waits on
//         Led[8] while bCheckShuttleFlag (910 :8724-8792); case 201 -> 100 with
//         the never-reset static iRetryCount (910 :8710-8723); SystemNG early
//         return ACTIVE (910 :8635, Frank 20261001).
//    [R]  The residual-IC check of case 100 (910 :8681-8690) is unreachable:
//         case 100 breaks unless InSHT1InLF(), and InSHT1InLF() requires
//         b1ShuttleMoveToLeft==true (910 csystem.cpp:416-421) -- pinned with
//         CheckShuttleOutputHasICError forced true.  (Golden oddity, kept.)
//    [P]  OutSHT1InLF/InRT: Type_HT9050 reads MOutShuttle1 with EXACT equality
//         (910 :721/:753); Type_HT9046_LS returns InSHT1InLF()/InSHT1InRT().
//    [S]  AdjustShuttlePlaceOrder / InitialOutArmNeedSuck / MoveInArmXYToWaitTrayArm.
//
//  FIXTURE NOTES (known traps):
//    * TMySimMotor::ScanMotorStatus sets Led[iInposLed]=true, which golden reads
//      as "NOT yet in position" (csystem.cpp InSHT1InLF).  The two shuttle axes
//      get TFlowSimMotor below, whose in-position LED the test controls.
//    * TMyMotor::ReadPos returns Motor->ReadPos() when Motor && Enable, so
//      positions are set through HTMotor::SetPosition as well as Position.
//    * Do_Auto_OutSH case 200 calls MOT[MOutShuttle1].Motor->Stop() without a
//      NULL check (golden); W906_TestEnsureSimMotors fills every axis.
//    * MTestY1/MTestZ1 are Enable=false for the unit parts so that
//      CheckIndexStatusERRSH1 never reaches Gali_Command.
//    * HTMotor::CheckIsSafeDoorOpen() is TRUE (door open) with no callback and
//      Enable==true, so the two shuttle axes get a door-closed callback.
//  The unit parts are memory only.  [D] pumps DoAllProcess (every engine), so the
//  test refuses to run outside ctest's redirect roots, like W6_6_CSystemCycle.
//  AI(W906-FLOW9050-A) 20261001 (review round 1): [D3] pins Do_Auto_SHT3 with
//  USE_OUT_SORT_ARM=eartInstall, one bDoProcess==true tick per type, and [S3b/c]
//  drive the HT9046_LS arm of MoveInArmXYToWaitTrayArm (TMySimMotor axes only).
// =============================================================================
#include "vclcompat/vcl_compat.h"   // AnsiString (acarry.h decls reference it)
#include "aHotPlateSubstrate.h"     // FTestSuck / FLCarryKit / FRCarryKit / BRCarryKit / InArmSuck / OutArmSuck / ptrOutSHT / AdjustShuttlePlaceOrder
#include "cprod.h"                  // Prod / IniConfig / TestIF / TestIF_File / InputLimit / TrayForm
#include "cpublic.h"
#include "cmydef.h"                 // MachineTypeChoice / MInShuttle1 / MOutShuttle1 / bCheckShuttle1Flag / bShuttle1Pause / bContraPoisitionFlag ...
#include "MachineType.h"            // Type_HT9050 / Type_HT9046_LS / eInSH8Sen / eUnderCoveyor / eartUninstall / NonVibration
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/mySimMotor.h"       // TMySimMotor
#include "mysensor.h"               // Sen[]
#include "acarry.h"                 // Do_Auto_InSH / Do_Auto_OutSH / AutoSHT1Task / AutoSHT2Task / b1ShuttleMoveToLeft / bShuttleHasIC
#include "acarry_shims.h"           // Tech / LastSet / SystemNG
#include "csystem.h"                // OutSHT1InLF / OutSHT1InRT / InSHT1InLF / InSHT1InRT / InitAllProcessTask
#include "csystem_shims.h"          // bShuttleShake
#include "aoutarm9045.h"            // InitialOutArmNeedSuck
#include "FormsFacade.h"            // fMain
#include <cstdio>
#include <vector>
#include "w906_test_motors.h"       // W906_TestEnsureSimMotors (MOT[].Motor boot invariant)
#include "w906_ctest_guard.h"       // W906TestRequireCtestRedirects

// No header home (golden acarry.cpp file-scope / ainarm9045.h clashes with aHotPlateSubstrate.h, see acarry.cpp:82-86).
bool CheckIndexStatusERRSH1();                                              // acarry.cpp (910 acarry.cpp:8522)
bool CheckShuttleOutputHasICError(int iSelSHT, int &X, int &Y);             // acarry.cpp (golden acarry.cpp:2076)
int  MoveInArmXYToWaitTrayArm(int iSht, int iKit, bool IncludeZ, bool bPlace);  // ainarm9045.cpp (910 ainarm9045.cpp:828)
extern int iMoveToShuttle;                                                  // ainarm9045.h:35
void EnsureArmOffsetObjects();                                              // cOffSet.cpp:166 (forms/fOffSet.h:450), golden main.cpp:2123-2139; as tests/test_AutoClean.cpp:70
void DoAllProcess();                                                        // csystem.cpp (only MainProc calls it; no header)
extern std::vector<int> g_csystemTrace;                                     // csystem.cpp tick-sequence oracle (CSYSTEM_TICK_ORACLE on ht9045_sm)
extern void csystemTraceClear();
enum { CT_SENSORSCAN = 1, CT_DOLOAD = 2, CT_DOINARM = 3, CT_SHT1 = 4, CT_SHT2 = 5,
       CT_SHT3 = 6, CT_TESTHEAD = 7, CT_CATCHTRAY = 8, CT_OUTARM = 9, CT_SORTARM = 10 };   // must match csystem.cpp

// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// A sim motor whose in-position LED the test sets.  Golden reads
// Led[iInposLed]==true as "still moving / not in position".
class TFlowSimMotor : public TMySimMotor
{
public:
    bool bBusy;
    TFlowSimMotor() : bBusy(false) {}
    virtual void ScanMotorStatus(bool *Led)
    {
        TMySimMotor::ScanMotorStatus(Led);
        if (Led != NULL)
            Led[iInposLed] = bBusy;
    }
};
static TFlowSimMotor *g_inSht  = 0;   // MInShuttle1
static TFlowSimMotor *g_outSht = 0;   // MOutShuttle1
static bool DoorClosed() { return false; }   // PF_CHECK (Motor/HTMotor.h); same fixture as tests/test_gali_route_engine.cpp

// Test geometry (positive, away from the -500 home-sensor check in MotorMovePosition).
static const int IN_L  = 10000, IN_R  = 60000;
static const int OUT_L = 12000, OUT_R = 62000;
static const int Z1_SAFE = 3000;

static void SetMotPos(int m, int p)
{
    MOT[m].Position = p;
    if (MOT[m].Motor != NULL)
        MOT[m].Motor->SetPosition(p);
    MOT[m].fCMD = false;
}
static void FreeMot(int m, bool b)
{
    MOT[m].fCanMove = b; MOT[m].fCanMoveR = b; MOT[m].fCanMoveM = b; MOT[m].fCanMoveL = b;
}
static bool HasTick(int id)
{
    for (size_t i = 0; i < g_csystemTrace.size(); ++i) if (g_csystemTrace[i] == id) return true;
    return false;
}
// The four flags TMyMotor::IsCanMove() ANDs (Motor/mymotor.cpp IsCanMove), by index.
static const char *const kCanMoveName[4] = { "fCanMove", "fCanMoveR", "fCanMoveM", "fCanMoveL" };
static bool &CanMoveFlag(int m, int k)
{
    switch (k)
    {
        case 0:  return MOT[m].fCanMove;
        case 1:  return MOT[m].fCanMoveR;
        case 2:  return MOT[m].fCanMoveM;
        default: return MOT[m].fCanMoveL;
    }
}
// Pump fn up to n times until pred() holds; returns the number of calls made (n+1 = never).
template <class F, class P> static int PumpUntil(F fn, P pred, int n)
{
    for (int i = 1; i <= n; ++i) { fn(); if (pred()) return i; }
    return n + 1;
}

// The guards Do_Auto_SHT1/SHT2 and DoAllProcess need in their "proceed" state
// (tests/test_w6_5_shuttle.cpp ArmSHT1Guards + tests/test_w6_6_csystem_cycle.cpp RunGuards).
static void ProceedGuards()
{
    IniConfig.bEnableTestingNeedStopAllMotor = false;
    IniConfig.bI24TestingNeedStopAllMotor    = false;
    bTestingStopAllMotor                     = false;
    IniConfig.bF16CheckShuttleSensorBroken   = false;
    IniConfig.bD43IndexDropErrorCanRetryandSkip = false;
    bRunInArmAutoAlignment    = false;
    lOutArmAutoAlignmentFlag  = 0;
    bDoingF16                 = false;
    SystemNG                  = false;
    In_Shuttle_Auto_Latch     = eInSH8Sen;
    bShuttle1MoveToLeft = false; bShuttle1MoveToRight = false;
    bShuttle2MoveToLeft = false; bShuttle2MoveToRight = false;
    bShuttleMoveToLeftforFix3 = false;
    bContraPoisitionFlag      = false;
    iOneCycle = 0; iCleanOut = 0;
}
static void RunGuards()
{
    InitialOK             = true;
    SoftStop              = false;
    SystemStart           = true;
    fAllMotorHome         = true;
    bShuttleShake         = false;
    TrayForm.bEnableAMR   = false;
    USE_OUT_SORT_ARM      = eartUninstall;
    AUTO_EMPTY_COLOR      = 0;
    AUTO3_IS_MAGAZINE     = 0;
    TRAY_VIBRATION        = NonVibration;
    SUPPORT_2_EMPTY_EMPTY = false;
    bUseAuto2Empty        = false;
    bLoaderNeedTrayMustFinish = false;
    bAutoNeedTrayMustFinish   = false;
    bRunInArmAutoAlignment    = false;
    bRunOutArmAutoAlignment   = false;
}
static void ClearKit(TMyKitSuck &k)
{
    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; ++i)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; ++j)
            k.Item[i][j] = NULL_IC;
}
// The 9050 unit-test world: both shuttles free, Z1 safe, kits empty, flags clear.
static void ResetWorld()
{
    MachineTypeChoice = Type_HT9050;
    ProceedGuards();
    bShuttle1Pause = false;
    Prod.InSHT[0].iLeft  = IN_L;  Prod.InSHT[0].iRight  = IN_R;
    Prod.OutSHT[0].iLeft = OUT_L; Prod.OutSHT[0].iRight = OUT_R;
    Prod.TestZ1_Safe = Z1_SAFE;
    InputLimit.iOffsetXYLow = 0;
    bSHTOfsChangeLeft[0] = false; bSHTOfsChangeRight[0] = false;
    FreeMot(MInShuttle1, true); FreeMot(MOutShuttle1, true);
    g_inSht->bBusy = false; g_outSht->bBusy = false;
    SetMotPos(MInShuttle1, IN_L); SetMotPos(MOutShuttle1, OUT_R);
    SetMotPos(MTestZ1, Z1_SAFE);
    ClearKit(FTestSuck); ClearKit(FLCarryKit); ClearKit(FRCarryKit);
    LastSet.iRealDummy = DUMMY;   // ShowOutputShuttleICStatus / CheckShuttleOutputHasICError take their offline early return
    b1ShuttleMoveToLeft = false; b1ShuttleMoveToRight = false;
    bCheckShuttle1Flag = false;
    AutoSHT1Task = 1; AutoSHT2Task = 1;
}

int main()
{   extern AnsiString DataPath; const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("Flow9050_Shuttle", rt)) return 2;   // DoAllProcess below runs every engine (same guard as W6_6_CSystemCycle)

    setvbuf(stdout, NULL, _IONBF, 0);   // unbuffered (as tests/test_AutoClean.cpp:181): a crash then shows the last PASS line, not a 4 KB-old one
    printf("==== F-01b batch A: HT9050 In/Out Shuttle (acarry.cpp) + Type_HT9050 branches ====\n");

    // Fixture: the two shuttle axes get the LED-controllable sim motor; every other axis a plain TMySimMotor.
    g_inSht  = new TFlowSimMotor();
    g_outSht = new TFlowSimMotor();
    MOT[MInShuttle1].Motor  = g_inSht;
    MOT[MOutShuttle1].Motor = g_outSht;
    W906_TestEnsureSimMotors();

    // =======================================================================
    //  [D] DoAllProcess shuttle dispatch (H093).  Run first, on the fresh
    //      process state (same set-up as test_w6_6_csystem_cycle.cpp), with
    //      Y1/Z1 left Enable=true exactly as there.  Each tick: both cursors at 1,
    //      every shuttle axis free; the trace tells which slot ran.
    // =======================================================================
    printf("[D] DoAllProcess: Type_HT9050 -> Do_Auto_InSH / Do_Auto_OutSH, never Do_Auto_SHT1/SHT2 (910 csystem.cpp:10360-10386)\n");
    {
        RunGuards();
        InitAllProcessTask();
        Prod.TestZ1_Safe = Z1_SAFE;
        struct Want { int machine; bool z1Safe; int expT; int expF; const char* what; };
        const Want wants[3] = {
            { Type_HT9050,    false, 1,  1,  "HT9050, Z1 NOT safe: InSH/OutSH case 1 wait (SHT1/SHT2 would go to 10)" },
            { Type_HT9050,    true,  10, 10, "HT9050, Z1 safe: InSH/OutSH case 1 -> 10 (the slot really runs them)" },
            { Type_HT9046_LS, false, 10, 10, "HT9046_LS, Z1 NOT safe: Do_Auto_SHT1/SHT2 case 1 -> 10 (old dispatch)" },
        };
        for (int w = 0; w < 3; ++w)
        {
            bool sawT = false, sawF = false;
            for (int tick = 0; tick < 4 && !(sawT && sawF); ++tick)
            {
                MachineTypeChoice = wants[w].machine;
                RunGuards();
                ProceedGuards();
                AutoSHT1Task = 1; AutoSHT2Task = 1;
                FreeMot(MInShuttle1, true); FreeMot(MInShuttle2, true); FreeMot(MOutShuttle1, true);
                SetMotPos(MTestZ1, wants[w].z1Safe ? Z1_SAFE : Z1_SAFE + 5000);
                csystemTraceClear();
                DoAllProcess();
                const bool t = HasTick(CT_SHT1), f = HasTick(CT_SHT2);
                char msg[300];
                if (t && !f)
                {
                    sawT = true;
                    snprintf(msg, sizeof(msg), "D%d-T %s: AutoSHT1Task==%d (observed %d)", w, wants[w].what, wants[w].expT, AutoSHT1Task);
                    CHECK(AutoSHT1Task == wants[w].expT, msg);
                    snprintf(msg, sizeof(msg), "D%d-T the bDoProcess tick leaves AutoSHT2Task alone (observed %d)", w, AutoSHT2Task);
                    CHECK(AutoSHT2Task == 1, msg);
                }
                else if (f && !t)
                {
                    sawF = true;
                    snprintf(msg, sizeof(msg), "D%d-F %s: AutoSHT2Task==%d (observed %d)", w, wants[w].what, wants[w].expF, AutoSHT2Task);
                    CHECK(AutoSHT2Task == wants[w].expF, msg);
                    snprintf(msg, sizeof(msg), "D%d-F the other tick leaves AutoSHT1Task alone (observed %d)", w, AutoSHT1Task);
                    CHECK(AutoSHT1Task == 1, msg);
                }
                else
                {
                    snprintf(msg, sizeof(msg), "D%d tick %d reached exactly one shuttle slot (trace size %d)", w, tick, (int)g_csystemTrace.size());
                    CHECK(false, msg);
                    break;
                }
            }
            CHECK(sawT && sawF, "D both ticks (bDoProcess true and false) observed");
        }
    }

    // =======================================================================
    //  [D3] Do_Auto_SHT3 (9046AU sort shuttle) with USE_OUT_SORT_ARM=eartInstall
    //       (MachineType.h:1029): the Type_HT9050 arm never calls it (910
    //       csystem.cpp:10360-10369), the else arm still does (910 :10371-10386).
    //       One bDoProcess==true tick per type.  Ticks are first spent with
    //       eartUninstall until the bDoProcess==false slot (CT_SHT2) has run, so
    //       the eartInstall tick is the bDoProcess==true one and DoSortArm (only
    //       on bDoProcess==false) is never reached.  eartUninstall is restored.
    // =======================================================================
    printf("[D3] DoAllProcess with USE_OUT_SORT_ARM=eartInstall: Type_HT9050 never Do_Auto_SHT3, Type_HT9046_LS still does\n");
    {
        const int machines[2] = { Type_HT9050, Type_HT9046_LS };
        for (int w = 0; w < 2; ++w)
        {
            bool aligned = false;
            for (int tick = 0; tick < 4 && !aligned; ++tick)
            {
                MachineTypeChoice = machines[w];
                RunGuards();                    // USE_OUT_SORT_ARM=eartUninstall
                ProceedGuards();
                AutoSHT1Task = 1; AutoSHT2Task = 1; AutoSHT3Task = 1;
                FreeMot(MInShuttle1, true); FreeMot(MInShuttle2, true); FreeMot(MOutShuttle1, true);
                SetMotPos(MTestZ1, Z1_SAFE + 5000);
                csystemTraceClear();
                DoAllProcess();
                aligned = HasTick(CT_SHT2) && !HasTick(CT_SHT1);
            }
            char msg[300];
            snprintf(msg, sizeof(msg), "D3.%d fixture: a bDoProcess==false tick ran, so the next tick is bDoProcess==true", w);
            CHECK(aligned, msg);

            MachineTypeChoice = machines[w];
            RunGuards();
            ProceedGuards();
            USE_OUT_SORT_ARM = eartInstall;
            AutoSHT1Task = 1; AutoSHT2Task = 1; AutoSHT3Task = 1;
            FreeMot(MInShuttle1, true); FreeMot(MInShuttle2, true); FreeMot(MOutShuttle1, true); FreeMot(MOutSortSht, true);
            SetMotPos(MTestZ1, Z1_SAFE + 5000);
            csystemTraceClear();
            DoAllProcess();
            USE_OUT_SORT_ARM = eartUninstall;
            AutoSHT3Task = 1;

            snprintf(msg, sizeof(msg), "D3.%d the eartInstall tick is the bDoProcess==true one (CT_SHT1 slot, no CT_SHT2)", w);
            CHECK(HasTick(CT_SHT1) && !HasTick(CT_SHT2), msg);
            if (machines[w] == Type_HT9050)
                CHECK(!HasTick(CT_SHT3), "D3.0 HT9050 + eartInstall: Do_Auto_SHT3 NOT called (no CT_SHT3; 910 csystem.cpp:10360-10369)");
            else
                CHECK(HasTick(CT_SHT3),  "D3.1 HT9046_LS + eartInstall: Do_Auto_SHT3 still called (CT_SHT3; old dispatch, 910 :10382-10385)");
        }
        CHECK(USE_OUT_SORT_ARM == eartUninstall, "D3 USE_OUT_SORT_ARM restored to eartUninstall");
    }

    // From here on: unit calls.  Y1/Z1 disabled so CheckIndexStatusERRSH1 never reaches Gali_Command; the two shuttle
    // axes get a closed safe door (HTMotor::CheckIsSafeDoorOpen with no callback and Enable==true reports OPEN, and
    // MotorMove then returns -1 forever -- golden boot installs IdleCheckSafeDoor, cinitial.cpp:4557).
    MOT[MTestY1].Motor->Enable = false;
    MOT[MTestZ1].Motor->Enable = false;
    g_inSht->MotorIdleSafeDoorCheck  = &DoorClosed;
    g_outSht->MotorIdleSafeDoorCheck = &DoorClosed;

    // =======================================================================
    //  [A] CheckIndexStatusERRSH1
    // =======================================================================
    printf("[A] CheckIndexStatusERRSH1 (910 acarry.cpp:8522-8528)\n");
    ResetWorld();
    CHECK(CheckIndexStatusERRSH1() == false, "A1 Y1/Z1 disabled -> false (Z1 below-height check false) -- shuttles not blocked");
    CHECK(FTestSuck.iMaxRow >= 1 && FTestSuck.iMaxCol >= 1 && FLCarryKit.iMaxRow >= 1 && FLCarryKit.iMaxCol >= 1 &&
          FRCarryKit.iMaxRow >= 1 && FRCarryKit.iMaxCol >= 1,
          "A2 fixture: the three kits scan at least Item[0][0] (HasIC/NoIC/AlreadyTest loop iMaxRow x iMaxCol)");

    // =======================================================================
    //  [I] Do_Auto_InSH
    // =======================================================================
    printf("[I] Do_Auto_InSH (910 acarry.cpp:8530-8624)\n");
    ResetWorld();
    SetMotPos(MTestZ1, Z1_SAFE + 1);   Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "I1a case 1: Z1 one count off safe -> stays 1 (exact ==, 910 :8559)");
    SetMotPos(MTestZ1, Z1_SAFE);
    bContraPoisitionFlag = true;       Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "I1b case 1: bContraPoisitionFlag -> stays 1 (910 :8557)");
    bContraPoisitionFlag = false;
    for (int k = 0; k < 4; ++k)
    {
        CanMoveFlag(MInShuttle1, k) = false; Do_Auto_InSH();
        char msg[200];
        snprintf(msg, sizeof(msg), "I1c.%d case 1: MInShuttle1.%s false -> IsCanMove false -> stays 1 (910 :8555)", k, kCanMoveName[k]);
        CHECK(AutoSHT1Task == 1, msg);
        CanMoveFlag(MInShuttle1, k) = true;
    }
    IniConfig.bEnableTestingNeedStopAllMotor = true; IniConfig.bI24TestingNeedStopAllMotor = true; bTestingStopAllMotor = true;
    Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "I1d testing-stop-all-motor early return (910 :8532)");
    IniConfig.bEnableTestingNeedStopAllMotor = false; IniConfig.bI24TestingNeedStopAllMotor = false; bTestingStopAllMotor = false;
    SystemNG = true;  Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "I1e SystemNG early return (910 :8549)");
    SystemNG = false; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 10, "I1f case 1: all gates open -> 10 (910 :8561)");

    // case 10 routing (iOneCycle==0 && iCleanOut==0 -> the else arm, 910 :8576-8589)
    FTestSuck.Item[0][0] = NULL_IC; FLCarryKit.Item[0][0] = HAS_IC;  AutoSHT1Task = 10; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 200, "I2a case 10: index empty + in-shuttle has IC -> 200 (910 :8580-8581)");
    FTestSuck.Item[0][0] = NULL_IC; FLCarryKit.Item[0][0] = NULL_IC; AutoSHT1Task = 10; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 100, "I2b case 10: index empty + in-shuttle empty -> 100 (910 :8583)");
    FTestSuck.Item[0][0] = HAS_IC;  FLCarryKit.Item[0][0] = HAS_IC;  AutoSHT1Task = 10; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 100, "I2c case 10: index has IC -> 100 (910 :8587)");
    // the one-cycle arm of case 10 (910 :8566-8574) routes these mixes exactly like the else arm, so the answer does
    // not depend on which arm IsInArmOneCycleFinish() selects -- this pins only that iOneCycle does not change routing
    iOneCycle = 1;
    CHECK(IsInArmOneCycleFinish() == true, "I2d precondition: with iOneCycle=1 the one-cycle arm of case 10 is really selected (csystem.h:104)");
    FTestSuck.Item[0][0] = NULL_IC; FLCarryKit.Item[0][0] = HAS_IC;  AutoSHT1Task = 10; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 200, "I2d case 10, iOneCycle=1: index empty + in-shuttle has IC -> 200 (910 :8566-8590, both arms)");
    FTestSuck.Item[0][0] = HAS_IC;  FLCarryKit.Item[0][0] = HAS_IC;  AutoSHT1Task = 10; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 100, "I2e case 10, iOneCycle=1: index has IC -> 100 (910 :8566-8590, both arms)");
    iOneCycle = 0;

    // case 100: in-shuttle from the index side back to the left; frees the out-shuttle
    ResetWorld();
    SetMotPos(MInShuttle1, IN_R); MOT[MOutShuttle1].fCanMove = false;
    AutoSHT1Task = 100;
    {
        int n = PumpUntil(Do_Auto_InSH, [](){ return AutoSHT1Task != 100; }, 5);
        CHECK(n <= 5 && AutoSHT1Task == 1, "I3a case 100: MotorMove(InSHT[0].iLeft) completes -> Task=1 (910 :8600-8604)");
    }
    CHECK(MOT[MInShuttle1].ReadPos() == IN_L, "I3b case 100: in-shuttle is at Prod.InSHT[0].iLeft");
    CHECK(MOT[MOutShuttle1].fCanMove == true, "I3c case 100: MOT[MOutShuttle1].fCanMove=true (yield to the out-shuttle, 910 :8602)");
    CHECK(b1ShuttleMoveToLeft == true, "I3d case 100: b1ShuttleMoveToLeft=true (910 :8603) -- not bShuttle1MoveToLeft");
    CHECK(bShuttle1MoveToLeft == false, "I3e case 100 does not touch Do_Auto_SHT1's bShuttle1MoveToLeft");

    // =======================================================================
    //  [O] Do_Auto_OutSH (continues the cycle: in-shuttle left, out-shuttle right)
    // =======================================================================
    printf("[O] Do_Auto_OutSH (910 acarry.cpp:8626-8803)\n");
    FTestSuck.Item[0][0] = TEST_PASS;   // tested IC on the index arm, out-shuttle pocket empty
    FRCarryKit.Item[0][0] = NULL_IC;
    AutoSHT2Task = 1;
    SystemNG = true;  Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 1, "O1a SystemNG early return is ACTIVE (910 :8635-8636, Frank 20261001)");
    SystemNG = false;
    for (int k = 0; k < 4; ++k)
    {
        CanMoveFlag(MOutShuttle1, k) = false; Do_Auto_OutSH();
        char msg[200];
        snprintf(msg, sizeof(msg), "O1c.%d case 1: MOutShuttle1.%s false -> IsCanMove false -> stays 1 (910 :8641)", k, kCanMoveName[k]);
        CHECK(AutoSHT2Task == 1, msg);
        CanMoveFlag(MOutShuttle1, k) = true;
    }
    IniConfig.bEnableTestingNeedStopAllMotor = true; IniConfig.bI24TestingNeedStopAllMotor = true; bTestingStopAllMotor = true;
    Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 10, "O1b no testing-stop guard in Do_Auto_OutSH (910 has none, unlike Do_Auto_InSH) -> case 1 -> 10");
    IniConfig.bEnableTestingNeedStopAllMotor = false; IniConfig.bI24TestingNeedStopAllMotor = false; bTestingStopAllMotor = false;
    // case 10 is 100 only for "tested IC on the index arm AND out pocket empty"; every other mix -> 200 (910 :8651-8654)
    FTestSuck.Item[0][0] = HAS_IC;                                   AutoSHT2Task = 10; Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 200, "O2a case 10: index IC not yet tested (HAS_IC, AlreadyTest false) -> 200 (910 :8651/:8654)");
    FTestSuck.Item[0][0] = TEST_PASS; FRCarryKit.Item[0][0] = HAS_IC; AutoSHT2Task = 10; Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 200, "O2b case 10: tested IC but out-shuttle pocket occupied (FRCarryKit HAS_IC) -> 200 (910 :8651/:8654)");
    FTestSuck.Item[0][0] = TEST_PASS; FRCarryKit.Item[0][0] = NULL_IC; AutoSHT2Task = 10;
    Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 100, "O2 case 10: index has a tested IC + out pocket empty -> 100 (910 :8651-8652)");
    bShuttle1Pause = true;
    {
        int n = PumpUntil(Do_Auto_OutSH, [](){ return AutoSHT2Task != 100; }, 5);
        CHECK(n <= 5 && AutoSHT2Task == 1, "O3a case 100: InSHT1InLF true -> MotorMove(OutSHT[0].iLeft) completes -> Task=1 (910 :8677-8698)");
    }
    CHECK(MOT[MOutShuttle1].ReadPos() == OUT_L, "O3b case 100: out-shuttle at Prod.OutSHT[0].iLeft");
    CHECK(MOT[MInShuttle1].fCanMove == false, "O3c case 100: MOT[MInShuttle1].fCanMove=false (910 :8679)");
    CHECK(bShuttle1Pause == false, "O3d case 100: bShuttle1Pause cleared (910 :8691-8693)");
    CHECK(b1ShuttleMoveToLeft == true, "O3e case 100: b1ShuttleMoveToLeft stays true");
    AutoSHT1Task = 1; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "O3f mutual yield: in-shuttle case 1 waits while MInShuttle1.fCanMove==false");
    // the in-shuttle may not come to the index side while the out-shuttle is there
    AutoSHT1Task = 200; Do_Auto_InSH();
    CHECK(AutoSHT1Task == 200 && MOT[MInShuttle1].ReadPos() == IN_L,
          "O3g Do_Auto_InSH case 200 breaks on OutSHT1InLF() (out-shuttle settled at its left, 910 :8610) -- no move");
    AutoSHT1Task = 1;

    // case 200: out-shuttle back to the right
    FTestSuck.Item[0][0] = NULL_IC;
    AutoSHT2Task = 10; Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 200, "O4a case 10: nothing to unload -> 200 (910 :8654)");
    g_outSht->bBusy = true;     // Led[iInposLed] true = not settled
    Do_Auto_OutSH();            // pos != iRight -> bCheckShuttleFlag=true; move commanded
    CHECK(bCheckShuttle1Flag == true, "O4b case 200: pos!=OutSHT[0].iRight -> bCheckShuttleFlag(=bCheckShuttle1Flag)=true (910 :8734-8737)");
    CHECK(b1ShuttleMoveToLeft == false, "O4c case 200 while moving: b1ShuttleMoveToLeft=false (910 :8795-8798)");
    for (int i = 0; i < 3; ++i) Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 200, "O4d case 200: arrived but Led[8] true and bCheckShuttleFlag -> waits (910 :8762-8763)");
    CHECK(MOT[MInShuttle1].fCanMove == true, "O4e case 200: arrival frees the in-shuttle (MOT[MInShuttle1].fCanMove=true, 910 :8752)");
    CHECK(b1ShuttleMoveToRight == true, "O4f case 200: b1ShuttleMoveToRight=true (910 :8756)");
    g_outSht->bBusy = false;
    Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 1 && bCheckShuttle1Flag == false, "O4g case 200: settled -> bCheckShuttleFlag=false, Task=1 (910 :8790-8791)");

    // in-shuttle to the index side now that the out-shuttle has left (Do_Auto_InSH case 200)
    FLCarryKit.Item[0][0] = HAS_IC;
    AutoSHT1Task = 1; Do_Auto_InSH(); Do_Auto_InSH();
    CHECK(AutoSHT1Task == 200, "I4a cycle: in-shuttle 1 -> 10 -> 200 once the out-shuttle is back");
    g_outSht->bBusy = true;
    for (int i = 0; i < 3; ++i) Do_Auto_InSH();
    CHECK(AutoSHT1Task == 200 && MOT[MInShuttle1].ReadPos() == IN_R,
          "I4b case 200: moved to InSHT[0].iRight but OutSHT1InRT false (out-shuttle not settled) -> stays 200 (910 :8615)");
    g_outSht->bBusy = false;
    Do_Auto_InSH();
    CHECK(AutoSHT1Task == 1, "I4c case 200: OutSHT1InRT -> Task=1 (910 :8619)");
    CHECK(MOT[MOutShuttle1].fCanMove == false, "I4d case 200: MOT[MOutShuttle1].fCanMove=false (910 :8617)");
    CHECK(b1ShuttleMoveToLeft == false, "I4e case 200: b1ShuttleMoveToLeft=false (910 :8618)");
    AutoSHT2Task = 1; Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 1, "I4f mutual yield: out-shuttle case 1 waits while MOutShuttle1.fCanMove==false");

    // Do_Auto_OutSH case 100 without b1ShuttleMoveToLeft: InSHT1InLF false -> break, no move
    ResetWorld();
    AutoSHT2Task = 100;
    for (int i = 0; i < 3; ++i) Do_Auto_OutSH();
    CHECK(AutoSHT2Task == 100 && MOT[MOutShuttle1].ReadPos() == OUT_R,
          "O5 case 100: b1ShuttleMoveToLeft==false -> InSHT1InLF false -> break, out-shuttle not moved (910 :8657-8658)");

    // case 201 (only reachable by setting the cursor): 201 -> 100; the static iRetryCount is never reset
    {
        bool ok3[3] = { false, false, false };
        for (int k = 0; k < 3; ++k)
        {
            ResetWorld();
            SetMotPos(MOutShuttle1, OUT_L);
            bCheckShuttle1Flag = true;
            AutoSHT2Task = 201;
            int n = PumpUntil(Do_Auto_OutSH, [](){ return AutoSHT2Task != 201; }, 5);
            ok3[k] = (n <= 5 && AutoSHT2Task == 100 && b1ShuttleMoveToRight == true && MOT[MOutShuttle1].ReadPos() == OUT_R);
            char msg[200];
            snprintf(msg, sizeof(msg), "O6.%d case 201: MotorMove(OutSHT[0].iRight) -> b1ShuttleMoveToRight=true, Task=100 (910 :8711-8721)", k + 1);
            CHECK(ok3[k], msg);
            snprintf(msg, sizeof(msg), "O6.%d iRetryCount=%d -> bCheckShuttleFlag %s (910 :8714-8717; >2 clears)", k + 1, k + 1, k < 2 ? "kept" : "cleared");
            CHECK(bCheckShuttle1Flag == (k < 2), msg);
        }
    }

    // =======================================================================
    //  [R] the residual-IC check of case 100 cannot fire (golden oddity, kept)
    // =======================================================================
    printf("[R] Do_Auto_OutSH case 100 residual check is unreachable (910 :8657 vs :8681)\n");
    {
        ResetWorld();
        const int  saveReal = LastSet.iRealDummy;
        const bool saveSen  = Sen[SnOutPutSHT1S1].Enable;
        const int  saveT1 = Tech.OutSH1ZOneRowDetectPos, saveT2 = Tech.OutSH2ZOneRowDetectPos;
        const int  saveMode = TestIF.iTestMode;
        const bool saveNS = TestIF.bNS7000kit;
        const bool saveEn = ENABLE_OUT_SHUTTLE_SENEOR;
        const int  saveCC = CUSTOMER_CODE;
        LastSet.iRealDummy = REALLY;
        Sen[SnOutPutSHT1S1].Enable = true;
        Tech.OutSH1ZOneRowDetectPos = 1; Tech.OutSH2ZOneRowDetectPos = 1;
        TestIF.iTestMode = SingleSite; TestIF.bNS7000kit = false;
        ENABLE_OUT_SHUTTLE_SENEOR = false;
        if (CUSTOMER_CODE == CC_SIGURD_HUKOU) CUSTOMER_CODE = 0;
        bShuttleHasIC[0][0][0] = true;
        FRCarryKit.Item[0][0] = NULL_IC;
        int X = -1, Y = -1;
        const bool residual = CheckShuttleOutputHasICError(0, X, Y);
        CHECK(residual == true, "R0 precondition: CheckShuttleOutputHasICError(0) forced TRUE (residual IC on the out-shuttle)");

        b1ShuttleMoveToLeft = false;          // the only state in which case 100 would run the check ...
        AutoSHT2Task = 100; Do_Auto_OutSH();
        CHECK(AutoSHT2Task == 100, "R1 ... but then InSHT1InLF() is false and case 100 breaks first");
        b1ShuttleMoveToLeft = true;           // the only state in which case 100 gets past its guard ...
        int n = PumpUntil(Do_Auto_OutSH, [](){ return AutoSHT2Task != 100; }, 5);
        CHECK(n <= 5 && AutoSHT2Task == 1, "R2 ... and then the `if(b1ShuttleMoveToLeft==false)` block is skipped: Task=1, never 201 (910 :8681-8690)");

        bShuttleHasIC[0][0][0] = false;
        CUSTOMER_CODE = saveCC; ENABLE_OUT_SHUTTLE_SENEOR = saveEn;
        TestIF.iTestMode = saveMode; TestIF.bNS7000kit = saveNS;
        Tech.OutSH1ZOneRowDetectPos = saveT1; Tech.OutSH2ZOneRowDetectPos = saveT2;
        Sen[SnOutPutSHT1S1].Enable = saveSen;
        LastSet.iRealDummy = saveReal;
    }

    // =======================================================================
    //  [P] OutSHT1InLF / OutSHT1InRT (H089) and the Type_HT9046_LS answers
    // =======================================================================
    printf("[P] OutSHT1InLF / OutSHT1InRT (910 csystem.cpp:701-768)\n");
    ResetWorld();
    // HT9050 arm: reads MOutShuttle1 with exact equality
    b1ShuttleMoveToLeft = true; SetMotPos(MOutShuttle1, OUT_L);
    CHECK(OutSHT1InLF() == true,  "P1a 9050 OutSHT1InLF: b1ShuttleMoveToLeft + settled + pos==OutSHT[0].iLeft -> true");
    SetMotPos(MOutShuttle1, OUT_L + 1);
    CHECK(OutSHT1InLF() == false, "P1b 9050 OutSHT1InLF: pos one count off -> false (exact ==, CompareCommandPos tolerance stays a comment)");
    SetMotPos(MOutShuttle1, OUT_L); g_outSht->bBusy = true;
    CHECK(OutSHT1InLF() == false, "P1c 9050 OutSHT1InLF: Led[iInposLed] true -> false");
    g_outSht->bBusy = false; b1ShuttleMoveToLeft = false;
    CHECK(OutSHT1InLF() == false, "P1d 9050 OutSHT1InLF: b1ShuttleMoveToLeft false -> false");
    SetMotPos(MOutShuttle1, OUT_R);
    CHECK(OutSHT1InRT() == true,  "P1e 9050 OutSHT1InRT: settled + pos==OutSHT[0].iRight -> true (no b1ShuttleMoveToRight test, as 910)");
    SetMotPos(MOutShuttle1, OUT_R - 1);
    CHECK(OutSHT1InRT() == false, "P1f 9050 OutSHT1InRT: pos one count off -> false (exact ==)");
    // a state where the arms disagree: in-shuttle settled at its right, out-shuttle at its left
    SetMotPos(MInShuttle1, IN_R); SetMotPos(MOutShuttle1, OUT_L); b1ShuttleMoveToLeft = false;
    CHECK(OutSHT1InRT() == false, "P2a 9050: out-shuttle not at its right -> OutSHT1InRT false (in-shuttle ignored)");
    MachineTypeChoice = Type_HT9046_LS;
    CHECK(OutSHT1InRT() == InSHT1InRT() && OutSHT1InRT() == true, "P2b HT9046_LS: OutSHT1InRT() == InSHT1InRT() == true (old one-line delegate)");
    SetMotPos(MInShuttle1, IN_L); SetMotPos(MOutShuttle1, OUT_R); b1ShuttleMoveToLeft = true;
    CHECK(OutSHT1InLF() == InSHT1InLF() && OutSHT1InLF() == true, "P2c HT9046_LS: OutSHT1InLF() == InSHT1InLF() == true (old one-line delegate)");
    MachineTypeChoice = Type_HT9050;
    CHECK(OutSHT1InLF() == false, "P2d 9050: same state, out-shuttle not at its left -> OutSHT1InLF false");
    MachineTypeChoice = Type_HT9046_LS; b1ShuttleMoveToLeft = false;
    CHECK(OutSHT1InLF() == InSHT1InLF() && OutSHT1InLF() == false, "P2e HT9046_LS: b1ShuttleMoveToLeft false -> both false");

    // =======================================================================
    //  [S] the three small branches
    // =======================================================================
    printf("[S] AdjustShuttlePlaceOrder (H003) / InitialOutArmNeedSuck (H074) / MoveInArmXYToWaitTrayArm (H004)\n");
    {
        const int saveMode = TestIF_File.iShuttleMode, saveSel = TestIF_File.iShuttle_Sel;
        const bool saveD58 = IniConfig.bD58UseArm1PickPlaceArm2Test;
        TestIF_File.iShuttleMode = 1; TestIF_File.iShuttle_Sel = 1;
        MachineTypeChoice = Type_HT9050;    InArmSuck.iWhichSht = -7; AdjustShuttlePlaceOrder(0);
        CHECK(InArmSuck.iWhichSht == 0, "S1a 9050 AdjustShuttlePlaceOrder: single-shuttle mode, Sel=1 -> still Shuttle 1 (iWhichSht=0, 910 ainarm2.cpp:714-717)");
        MachineTypeChoice = Type_HT9046_LS; InArmSuck.iWhichSht = -7; AdjustShuttlePlaceOrder(0);
        CHECK(InArmSuck.iWhichSht == 1, "S1b HT9046_LS AdjustShuttlePlaceOrder: Sel=1 -> iWhichSht=1 (old)");
        TestIF_File.iShuttleMode = 0; IniConfig.bD58UseArm1PickPlaceArm2Test = false;
        MachineTypeChoice = Type_HT9050;    InArmSuck.iWhichSht = -7; AdjustShuttlePlaceOrder(1);
        CHECK(InArmSuck.iWhichSht == 0, "S1c 9050 AdjustShuttlePlaceOrder(1) -> 0");
        MachineTypeChoice = Type_HT9046_LS; InArmSuck.iWhichSht = -7; AdjustShuttlePlaceOrder(1);
        CHECK(InArmSuck.iWhichSht == 1, "S1d HT9046_LS AdjustShuttlePlaceOrder(1) -> 1 (old)");
        TestIF_File.iShuttleMode = saveMode; TestIF_File.iShuttle_Sel = saveSel;
        IniConfig.bD58UseArm1PickPlaceArm2Test = saveD58;
    }
    {
        ResetWorld();
        SetMotPos(MOutShuttle1, OUT_R); SetMotPos(MInShuttle1, 0);   // out-shuttle at its right; in-shuttle NOT at its right
        ptrOutSHT = 0;
        MachineTypeChoice = Type_HT9050;
        CHECK(InitialOutArmNeedSuck(0) == true && ptrOutSHT == &FRCarryKit,
              "S2a 9050 InitialOutArmNeedSuck(0): MOutShuttle1 settled at OutSHT[0].iRight -> true, ptrOutSHT=&FRCarryKit (910 aoutarm9045.cpp:3581-3590)");
        MachineTypeChoice = Type_HT9046_LS;
        CHECK(InitialOutArmNeedSuck(0) == false, "S2b HT9046_LS InitialOutArmNeedSuck(0): looks at MInShuttle1 (not at its right) -> false (old)");
        MachineTypeChoice = Type_HT9050;
        g_outSht->bBusy = true;
        CHECK(InitialOutArmNeedSuck(0) == false, "S2c 9050: MOutShuttle1 Led[iInposLed] true -> false");
        g_outSht->bBusy = false; SetMotPos(MOutShuttle1, OUT_R - 1);
        CHECK(InitialOutArmNeedSuck(0) == false, "S2d 9050: MOutShuttle1 short of iRight+iOffsetXYLow*100 -> false");
        SetMotPos(MOutShuttle1, OUT_L); SetMotPos(MInShuttle1, IN_R);
        MachineTypeChoice = Type_HT9046_LS;
        CHECK(InitialOutArmNeedSuck(0) == true, "S2e HT9046_LS: MInShuttle1 settled at InSHT[0].iRight -> true (old)");
        MachineTypeChoice = Type_HT9050;
        CHECK(InitialOutArmNeedSuck(0) == false, "S2f 9050: same state, MOutShuttle1 at its left -> false");
    }
    {
        // H004: TRAY_ARM_MODE==eUnderCoveyor makes MoveInArm2XYToWait return before any motion (ainarm2.cpp
        // MoveInArm2XYToWait), so the 9050 arm's iMoveToShuttle==1 shows up as result 1000.  The HT9046_LS arm,
        // same state, chooses 2 and runs MoveInArmXYToShuttle_9045 (S3b/c).  Every axis is a TMySimMotor
        // (W906_TestEnsureSimMotors), so nothing physical moves; TestIF.iTestMode=SingleSite keeps its switch off
        // the default arm, whose bResult is indeterminate (golden bug kept, ainarm9045.cpp MoveInArmXYToShuttle_9045).
        const int saveArm = TRAY_ARM_MODE;
        const bool saveHas = MOT[MMTrayY].fHasTray;
        const bool saveOk = bMoveInArm2XYToWaitOk, saveDev = bDoTrayDeviceCheck, saveOne = bOneCycleInArmToLoader;
        const bool saveE61 = IniConfig.bE61InArmStandbyPosOnLoader, saveSrv = IniConfig.bAlarmNeedServoOff;
        IniConfig.bAlarmNeedServoOff = false;   // MoveInArm2XYToWait would read fNote->bMyServoOffInArm
        TRAY_ARM_MODE = eUnderCoveyor;
        MOT[MMTrayY].fHasTray = true;
        bDoTrayDeviceCheck = false; bOneCycleInArmToLoader = false; IniConfig.bE61InArmStandbyPosOnLoader = false;
        MachineTypeChoice = Type_HT9050;
        iMoveToShuttle = 0;
        CHECK(MOT[MMTrayY].Tray.HasIC() == false, "S3 precondition: MMTrayY tray has no IC");
        const int r = MoveInArmXYToWaitTrayArm(0, 0, false, true);
        printf("    MoveInArmXYToWaitTrayArm(0,...) = %d, iMoveToShuttle = %d\n", r, iMoveToShuttle);
        CHECK(r == 1000 && iMoveToShuttle == 0,
              "S3a 9050 MoveInArmXYToWaitTrayArm: tray empty + has tray -> iMoveToShuttle=1 even under eUnderCoveyor -> MoveInArm2XYToWait -> 1000 (910 ainarm9045.cpp:834-845)");
        {
            // MoveInArmXYToShuttle_9045's per-layout movers read InArmOffSet[] (GetInArmPitchX_9045 /
            // GetInArmPitchY_9045, ainarm9045.cpp:146/:182); golden TfMain allocates them at boot (main.cpp:2123-2139).  Without this the
            // call is an access violation (measured 20261001: SEGFAULT right after S3a).
            EnsureArmOffsetObjects();
            const int saveTM = TestIF.iTestMode;
            TestIF.iTestMode = SingleSite;
            MachineTypeChoice = Type_HT9046_LS;
            iMoveToShuttle = 0;
            const int r2 = MoveInArmXYToWaitTrayArm(0, 0, false, true);
            const int ims2 = iMoveToShuttle;
            printf("    HT9046_LS: MoveInArmXYToWaitTrayArm(0,...) = %d, iMoveToShuttle = %d\n", r2, ims2);
            CHECK(r2 != 1000, "S3b HT9046_LS, same state: NOT 1000 -- TRAY_ARM_MODE==eUnderCoveyor keeps the old arm off MoveInArm2XYToWait");
            CHECK((r2 == 0 && ims2 == 2) || (r2 == 1100 && ims2 == 0),
                  "S3c HT9046_LS: iMoveToShuttle=2 -> MoveInArmXYToShuttle_9045 (0 with the cursor left at 2, or 1100 once arrived)");
            TestIF.iTestMode = saveTM;
            iMoveToShuttle = 0;
            MachineTypeChoice = Type_HT9050;
        }
        TRAY_ARM_MODE = saveArm; MOT[MMTrayY].fHasTray = saveHas;
        bMoveInArm2XYToWaitOk = saveOk; bDoTrayDeviceCheck = saveDev; bOneCycleInArmToLoader = saveOne;
        IniConfig.bE61InArmStandbyPosOnLoader = saveE61; IniConfig.bAlarmNeedServoOff = saveSrv;
        iMoveToShuttle = 0;
    }

    MachineTypeChoice = Type_HT9045;   // leave the process the way the other tests expect it
    printf("\n==== Flow9050_Shuttle summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
