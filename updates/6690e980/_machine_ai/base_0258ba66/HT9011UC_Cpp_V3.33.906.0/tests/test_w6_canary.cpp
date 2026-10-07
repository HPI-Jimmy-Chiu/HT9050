// =============================================================================
//  test_w6_canary.cpp  --  W6 VERIFY: predicate API + Empty-tray canary stepper
//
//  Translation wave: W6.0 SCAFFOLD + W6.1 CANARY (verify pass)
//  Author: AI(W6.0-SCAFFOLD) 20260626
//  AI(W906-W6-CANARYfix) 20260801: PART B rewritten -- see "TAUTOLOGY REPAIR".
//
//  PURPOSE (task-mandated dedicated verify TU)
//  -------------------------------------------
//  Companion to test_w6_1_empty_canary.cpp.  Where that smoke test asserts the
//  stack LINKS + the SM does not crash, THIS test pins two concrete contracts:
//
//    PART A -- PREDICATE TEST
//      Set a KNOWN Sim HAL state (Sen[] sensors via TSimIOBackend, MOT[] tray
//      flags) and assert the frozen csystem.h HasIC-family predicates return the
//      EXPECTED bool.  We exercise:
//        * the conservative leaf baseline (TMyKitSuck grid gated #if 0 -> false),
//        * the LIVE InputShuttleRearHasIC IniConfig/TestIF_File early-return guard
//          (real logic in csystem_predicates.cpp, NOT gated) -- toggling the guard
//          flag must not change the answer because the leaf is false either way,
//          but the guard BRANCH itself is covered both ways,
//        * the composite OR-aggregation shape (ShuttleHasIC == Input||Output,
//          IndexHasIC == TestHead||TestSocket, HasAnyICInMachine aggregate).
//      Because the KitSuck grid is deferred (W6.x), every leaf is conservatively
//      false, so every aggregate is false -- we assert exactly that, AND that the
//      predicates are PURE (calling them does not perturb the Sim HAL state we set).
//
//    PART B -- CANARY STEPPER TEST
//      Step each of the three Empty-tray stack SMs one tick at a time over the
//      Sim HAL, record the EXACT cursor trajectory, and assert both the
//      trajectory AND the actuator/tray-flag state changes each visited case is
//      supposed to produce.
//
//  TAUTOLOGY REPAIR -- AI(W906-W6-CANARYfix) 20260801
//  --------------------------------------------------
//  Part B previously carried the project's third copy of an unfalsifiable
//  assertion template.  All three copies are replaced here:
//
//   (T1) CHECK(done || iLoadNewEmptyTrayToCarTask != 1 || steps == 400, ...)
//        Leaving `for(steps=0; steps<400; ++steps){ if(Do()) {done=true;break;} }`
//        has exactly two exits -- break (=> done) or exhaustion (=> steps==400) --
//        so `done || steps==400` is UNCONDITIONALLY TRUE.  A loop's own
//        termination condition must never appear in the assertion's disjunction.
//
//   (T2) CHECK(iLoadNewEmptyTrayToCarTask >= 1, "...no corruption")
//        The DoLoadNewEmptyTrayToCar cursor set is {1,20,50,52,53,60,100,200,300,
//        400,410,420}; nothing in the SM can write a value < 1.  Falsifiable only
//        by memory corruption, i.e. by a state the code cannot reach.
//
//   (T3) CHECK(aux_sane, "DoAutoEmpty1() ... stayed on a sane cursor")
//        aux_sane is cleared only by iAutoEmpty1ReceiveTask < 0 || > 100000.  The
//        DoAutoEmpty1 cursor set is {1,100,200} -- same unreachable-state defect.
//        WORSE: with no tray on MOT[MMEmpty1] and both Empty1 sensors disabled,
//        case 1's guard never fires, so all 300 ticks re-ran case 1 and the SM
//        never moved at all.
//
//   (T4) FALSE LABEL on the DoLoadNewEmptyTrayToCar block.  The old fixture set
//        Sen[SenEmptyCCWDete].Enable = true; a TMySensor with Enable==true and
//        Type==TYPE_B(0) reads IsOn()==true, so case 1's
//        `if(Sen[SenEmptyCCWDete].IsOn() || ...) { ...; return true; }` fired on
//        the FIRST tick.  Measured before the repair:
//            (DoLoadNewEmptyTrayToCar returned-true=1 after 0 steps, final cursor=1)
//        The lifter chain (case 50/60/100/200/300/400/410/420) was walked ZERO
//        times, yet the label claimed "(lifter+Cylinder HAL)".
//
//   (T5) FALSE LABEL on the DoAutoEmpty block.  The label claimed convergence
//        "through the time-gated 1->...->100/200->1 path".  Part A leaves
//        Sen[SenEmptyHasTray] and Sen[SenEmptyCWDete] Enabled (=> IsOn()==true),
//        so DoAutoEmpty case 1 took its tray-PRESENT branch every run.  The
//        measured trajectory was 1->20->30->40->50->60->70->100->150->1 -- case
//        200 (the DoLoadNewEmptyTrayToCar sub-call the label named) was never
//        entered, which the block's own "max-cursor=150" print already showed.
//
//  HOW THE REPLACEMENTS ARE MADE FALSIFIABLE
//  -----------------------------------------
//    * Each SM is stepped ONE tick at a time and its cursor trajectory is
//      recorded and compared against the EXACT expected sequence.  Any added,
//      dropped or re-ordered transition fails.
//    * Every actuator the visited cases command is observed at the WIRE, not at
//      the call site: the Cylinder[]/SW[] under test are given a real Sim IO
//      address (OutISABase==eMotionNet, OutType==TYPE_A) so On()/Push() drive
//      TLaneIO::IOBitOn and Off()/Pop() drive IOBitOff, and the resulting bit is
//      read back through Cylinder[].GetOutBit() / SW[].Status().  A call that is
//      DELETED and a call that is INVERTED therefore fail the same assertion.
//    * Every actuator is pre-driven to the OPPOSITE of the expected value, and
//      re-armed mid-trajectory wherever two same-direction commands would mask
//      each other (e.g. case 60 CylinderUp then case 200 CylinderMiddle both
//      raise C_Empty_Middle).
//    * MOT[] tray flags are likewise pre-driven opposite / re-armed mid-run, so
//      SetTray() and ClearTray() are both observable.
//
//  TIMER DISCIPLINE
//  ----------------
//  TQPF_Timer is a QueryPerformanceCounter WALL CLOCK, so a tight test loop can
//  never outlast a SetMSAndOn(>0) wait.  We extern the SM's file-scope timers and
//  re-arm ONLY the pure PROGRESS timers, and only while the cursor is parked on
//  the state that waits on them:
//      EmptyDelay          re-armed only at cursor 400 (armed by case 300)
//      DoAutoEmptyDelay    re-armed only at cursor 70 / 150
//      CheckEmptyTrayDelay re-armed only at cursor 65
//      DoAutoEmpty1Delay   re-armed only at cursor 200
//  The JAM watchdogs are NEVER advanced: WaitTrayDelay (JAM1001, DoAutoEmpty1
//  case 100), EmptyMidCylinTime (JAM1006, case 20), and EmptyDelay in its
//  SECOND role as the case-420 JAM1013 watchdog (armed by case 410, so cursor
//  is 420 -- outside the cursor==400 pin) all keep their real wall-clock arming.
//
//  OFFLINE: TSimIOBackend is the default IO backend (no vendor SDK, no hardware);
//  sensors default Enable==false.  We explicitly Enable + drive the few sensors a
//  predicate/canary path reads, so the asserted state is the one under test.
//
//  Faithful state facts (golden asendic_Empty.cpp, cp950-decoded this run):
//    DoLoadNewEmptyTrayToCar (golden :43-263) cursor set
//                                = {1,20,50,52,53,60,100,200,300,400,410,420}
//    DoAutoEmpty  cursor set     = {1,20,30,40,50,60,65,70,100,150,200,300}
//    DoAutoEmpty1 cursor set     = {1,100,200}
//    DoAutoEmpty entry / terminal = case 1 (recurring "idle/dispatch" state)
//    DoLoadNewEmptyTrayToCar converges -> returns true & resets cursor to 1.
// =============================================================================
#include "asendic_Empty.h"
#include "csystem.h"
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage, etc.
#include "FormsFacade.h"        // fAGV
#include <cstdio>
#include <windows.h>            // Sleep / GetTickCount -- pump past the SM's QPF wait timers
// AI(W906-W7-L1-Wave3) 20260802: CylinderUp/Middle/Lower are golden's real
// closed-loop lifter SMs now (asendic.cpp), not Sim bodies that return true.
// This header is the physical stack that answers their position sensors.
#include "w3_cylinder_plant.h"

// File-scope (external-linkage) init not exported by the mirrored header; the
// golden calls it cross-module via an implicit decl, so we forward-declare it
// here exactly as the golden does at its call site.
extern void InitLoadNewEmptyTrayToCarTask();

//AI(W906-W6-CANARYfix) 20260801: the SM's file-scope TQPF_Timers (asendic_Empty.cpp,
//  all external linkage, none exported by asendic_Empty.h).  Externed so the
//  PROGRESS waits can be collapsed per the TIMER DISCIPLINE note above; the JAM
//  watchdogs are deliberately NOT externed here so they cannot be advanced.
extern TQPF_Timer EmptyDelay;           // case 300->400 progress AND case 420 JAM1013 watchdog
extern TQPF_Timer DoAutoEmptyDelay;     // case 60->70 and case 100->150 progress
extern TQPF_Timer CheckEmptyTrayDelay;  // case 60->65 progress
extern TQPF_Timer DoAutoEmpty1Delay;    // case 100->200 progress

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as the substrate tests)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  AI(W906-W6-CANARYfix) 20260801: cursor-trajectory recorder.
//  Records the sequence of DISTINCT cursor values a stepper visits, so the test
//  asserts the path taken instead of asserting that the loop terminated.
// ---------------------------------------------------------------------------
#define TRAJ_MAX 64
struct Traj
{
    int  seq[TRAJ_MAX];
    int  n;
    bool overflow;

    void Start(int first) { n = 0; overflow = false; Note(first); }
    void Note(int cur)
    {
        if (n > 0 && seq[n-1] == cur) return;    // only transitions
        if (n >= TRAJ_MAX) { overflow = true; return; }
        seq[n++] = cur;
    }
    bool Equals(const int *expect, int en) const
    {
        if (overflow || n != en) return false;
        for (int i = 0; i < en; ++i) if (seq[i] != expect[i]) return false;
        return true;
    }
    void Print(const char *label) const
    {
        printf("    %s: %d", label, n > 0 ? seq[0] : -1);
        for (int i = 1; i < n; ++i) printf(" -> %d", seq[i]);
        if (overflow) printf(" ...(overflow)");
        printf("\n");
    }
};

// ---------------------------------------------------------------------------
//  AI(W906-W6-CANARYfix) 20260801: give one Cylinder[]/SW[] a real Sim IO
//  address so its command reaches the wire (TLaneIO::IOBitOn/IOBitOff ->
//  OutPortData) and can be read back.  OutType==TYPE_A is the non-inverting
//  wiring: On/Push -> bit 1, Off/Pop -> bit 0 (mycylin.cpp OnSwitch/OffSwitch).
//  No sensors installed (OnSenEnable/OffSenEnable false) and zero delay times,
//  so Push()/Pop() complete in one tick -- offline they BOTH return true, which
//  is exactly why the wire bit, not the return value, is what we assert on.
// ---------------------------------------------------------------------------
static void WireCylinder(int c, int ring, int ip, int port, int bit)
{
    Cylinder[c].Enable       = true;
    Cylinder[c].OutISABase   = eMotionNet;
    Cylinder[c].OutType      = TYPE_A;
    Cylinder[c].OutRing      = ring;
    Cylinder[c].OutIP        = ip;
    Cylinder[c].OutPort      = port;
    Cylinder[c].OutBit       = bit;
    Cylinder[c].OnSenEnable  = false;
    Cylinder[c].OffSenEnable = false;
    Cylinder[c].OnDelayTime  = 0;
    Cylinder[c].OffDelayTime = 0;
    Cylinder[c].OnAlarmTime  = 0;
    Cylinder[c].OffAlarmTime = 0;
}

static void WireSwitch(int s, int ring, int ip, int port, int bit)
{
    SW[s].Enable  = true;
    SW[s].ISABase = eMotionNet;
    SW[s].Type    = TYPE_A;
    SW[s].Ring    = ring;
    SW[s].IP      = ip;
    SW[s].Port    = port;
    SW[s].Bit     = bit;
}

// AI(W906-W6-CANARYfix) 20260801: TMySensor is THREE-state; never normalise.
//   Enable==false        -> "unknown": IsOn() and IsOff() are BOTH false
//   Enable + Type==TYPE_A -> reads OFF (IsOn()==false, IsOff()==true)
//   Enable + Type==TYPE_B -> reads ON  (IsOn()==true,  IsOff()==false)
// (mysensor.cpp IsOn/IsOff; the Sim backend leaves the raw input bit 0.)
static void SensorOff(int s) { Sen[s].Enable = true;  Sen[s].Type = TYPE_A; }
static void SensorOn (int s) { Sen[s].Enable = true;  Sen[s].Type = TYPE_B; }

// DoAutoEmpty cursor's documented state set (golden dispatch).  A value outside
// this set means corruption.
static bool autoEmptyCursorSane(int t)
{
    switch (t)
    {
        case 1: case 20: case 30: case 40: case 50: case 60: case 65:
        case 70: case 100: case 150: case 200: case 300:
            return true;
        default:
            return false;
    }
}

static void setupDummyFeed()
{
    LastSet.iRealDummy             = DUMMY;
    fEmptyCanSupplyNewTray         = true;
    iReceiveEmptyTray              = 0;
    bIsCatchingFromBuffer          = false;
    bIsPlacingToBuffer             = false;
    bEmptyPause                    = false;
    bHandlerPause                  = false;
    bOCRRunTime                    = false;

    CosFunction.bSpecialP24                  = false;
    CosFunction.bUseEmptyColorTrayPreAlarm   = false;
    IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty = false;
    IniConfig.bA65_BundleIDList              = false;
    IniConfig.bP32EmptyColorTrayPreAlarm     = false;
    IniConfig.bG11ASEReport                  = false;
    IniConfig.bEnable_SECS_GEM               = false;
    IniConfig.bP04ColorIsEmptyUnloader       = false;
    TrayForm.bEnableAMR                      = false;
    TrayForm.bColorTray                      = false;
}

// ===========================================================================
//  PART A -- PREDICATE TEST: known Sim HAL state -> expected predicate bools
// ===========================================================================
static void test_predicates_known_state()
{
    printf("\n[A] Predicate API over a KNOWN Sim HAL state\n");

    // --- Establish a known, deterministic HAL state -------------------------
    // Put trays on a couple of MOT stations and drive a couple of sensors ON so
    // we are NOT relying on whatever the substrate left behind.  None of these
    // feed the (gated) KitSuck leaves, so every predicate must still read false;
    // this proves the conservative gate holds regardless of surrounding state.
    MOT[MMEmptyZ].SetTray(NULL_IC, "test_w6_canary");
    MOT[MMEmpty_Car].SetTray(NULL_IC, "test_w6_canary");

    //AI(W906-W6-CANARYfix) 20260801: Type is now set EXPLICITLY.  The old code set
    //  .Enable only and relied on the default Type==0, which is TYPE_B, i.e. these
    //  two sensors read ON -- the hidden cause of the false DoAutoEmpty path label
    //  (T5).  Part B no longer inherits this state; it establishes its own.
    SensorOn(SenEmptyHasTray);
    SensorOn(SenEmptyCWDete);

    // --- Leaf baseline: KitSuck grid deferred (#if 0) -> every leaf is false --
    CHECK(InputShuttleFrontHasIC() == false, "InputShuttleFrontHasIC()==false (KitSuck leaf gated)");
    CHECK(OutputShuttleFrontHasIC()== false, "OutputShuttleFrontHasIC()==false (KitSuck leaf gated)");
    CHECK(FrontTestHeadHasIC()     == false, "FrontTestHeadHasIC()==false (KitSuck leaf gated)");
    CHECK(RearTestHeadHasIC()      == false, "RearTestHeadHasIC()==false (KitSuck leaf gated)");
    CHECK(TestSocketHasIC()        == false, "TestSocketHasIC()==false (KitSuck leaf gated)");
    CHECK(SortShuttleHasIC()       == false, "SortShuttleHasIC()==false (KitSuck leaf gated)");
    CHECK(HasICUnderHotPlate()     == false, "HasICUnderHotPlate()==false (hotplate map deferred)");

    // --- LIVE guard branch: InputShuttleRearHasIC IniConfig/TestIF_File guard --
    // This is REAL logic in csystem_predicates.cpp (not gated).  Cover BOTH the
    // guard-taken and guard-not-taken branches.  The answer is false either way
    // (leaf gated), but exercising the guard both ways covers the live branch.
    IniConfig.bIndexArm2SupplyLight = true;             // guard condition TRUE
    CHECK(InputShuttleRearHasIC() == false, "InputShuttleRearHasIC()==false (Arm2-supply-light guard taken)");
    IniConfig.bIndexArm2SupplyLight        = false;     // guard condition FALSE
    TestIF_File.bForEgisTecTest            = false;
    IniConfig.bD58UseArm1PickPlaceArm2Test = false;
    TestIF_File.bArm1PickPlaceArm2Test     = false;
    CHECK(InputShuttleRearHasIC() == false, "InputShuttleRearHasIC()==false (guard not taken, leaf gated)");

    // Same live guard exists on OutputShuttleRearHasIC -- cover it too.
    IniConfig.bIndexArm2SupplyLight = true;
    CHECK(OutputShuttleRearHasIC() == false, "OutputShuttleRearHasIC()==false (Arm2-supply-light guard taken)");
    IniConfig.bIndexArm2SupplyLight = false;
    CHECK(OutputShuttleRearHasIC() == false, "OutputShuttleRearHasIC()==false (guard not taken, leaf gated)");

    // --- Composite OR-aggregation shape (delegates to the leaves faithfully) --
    CHECK(InputShuttleHasIC()  == false, "InputShuttleHasIC() == Front||Rear (==false)");
    CHECK(OutputShuttleHasIC() == false, "OutputShuttleHasIC()==false (front/rear leaves gated)");
    CHECK(ShuttleHasIC()       == false, "ShuttleHasIC() == Input||Output (==false)");
    CHECK(TestHeadHasIC()      == false, "TestHeadHasIC() == Front||Rear (==false)");
    CHECK(IndexHasIC()         == false, "IndexHasIC() == TestHead||TestSocket (==false)");
    CHECK(IndexHasRealIC()     == false, "IndexHasRealIC()==false (HasRealIC leaves gated)");
    CHECK(HasICUnderMachine()  == false, "HasICUnderMachine() == Shuttle||Index||HotPlate (==false)");
    CHECK(HasAutoICInMachine() == false, "HasAutoICInMachine()==false (auto-tray walk deferred)");
    CHECK(HasAnyICInMachine()  == false, "HasAnyICInMachine() == UnderMachine||AutoIC (==false)");

    // --- Purity: predicates must not perturb the Sim HAL state we set up ------
    CHECK(MOT[MMEmptyZ].fHasTray == true,
          "predicates are pure: MOT[MMEmptyZ].fHasTray still set after predicate calls");

    // Clean up the trays so they do not leak into Part B.
    MOT[MMEmptyZ].ClearTray("test_w6_canary");
    MOT[MMEmpty_Car].ClearTray("test_w6_canary");
}

// ===========================================================================
//  PART B.0 -- FIXTURE CONTRACT
//  Every Part B label below depends on the TMySensor three-state semantics and
//  on the wired-cylinder wire read.  Assert them, do not assume them.
// ===========================================================================
static void test_fixture_contract()
{
    printf("\n[B.0] Fixture contract: Sen[] three-state + wired-actuator readback\n");

    const int s = SenEmptyCCWDete;
    Sen[s].Enable = false; Sen[s].Type = TYPE_A;
    CHECK(Sen[s].IsOn() == false && Sen[s].IsOff() == false,
          "Sen[] Enable==false is UNKNOWN: IsOn() and IsOff() are both false");
    SensorOff(s);
    CHECK(Sen[s].IsOn() == false && Sen[s].IsOff() == true,
          "Sen[] Enable+TYPE_A reads OFF (IsOn()==false, IsOff()==true)");
    SensorOn(s);
    CHECK(Sen[s].IsOn() == true && Sen[s].IsOff() == false,
          "Sen[] Enable+TYPE_B reads ON (IsOn()==true, IsOff()==false)");

    // Wired-cylinder readback: the command must reach the Sim IO wire, else no
    // Part B actuator assertion below can distinguish a call from a no-op.
    WireCylinder(C_Empty_Up, 1, 1, 2, 1);
    Cylinder[C_Empty_Up].Off();
    const bool w0 = Cylinder[C_Empty_Up].GetOutBit();
    Cylinder[C_Empty_Up].On();
    const bool w1 = Cylinder[C_Empty_Up].GetOutBit();
    Cylinder[C_Empty_Up].Off();
    const bool w2 = Cylinder[C_Empty_Up].GetOutBit();
    CHECK(w0 == false && w1 == true && w2 == false,
          "wired Cylinder OutType==TYPE_A: On()->wire 1, Off()->wire 0 (GetOutBit readback)");
}

// ===========================================================================
//  PART B.1 -- DoAutoEmpty: the Empty-tray-PRESENT dispatch branch
//
//  FAITHFUL PATH (golden asendic_Empty.cpp DoAutoEmpty case 1 tray-present arm):
//    case 1   Sen[SenEmptyHasTray].IsOn() && Sen[SenEmptyCWDete].IsOn() -> 20
//    case 20  Cylinder[C_Empty_Fix].Off()                               -> 30
//    case 30  Cylinder[C_Empty_Fix].Pop()                               -> 40
//    case 40  Sen[SenEmptyFixCyPush] OFF -> no JAM1003                  -> 50
//    case 50  Cylinder[C_Empty_Fix].Pop()                               -> 60
//    case 60  MOT[MMEmpty].fHasTray==false -> arm DoAutoEmptyDelay(1s)  -> 70
//    case 70  DUMMY && !fAGV->IsATK_AMR() && !MOT[MMEmpty].fHasTray:
//             MOT[MMEmpty_Car].ClearTray() + MOT[MMEmpty].SetTray()     -> 100
//    case 100 MMEmpty_Car empty -> Task=200, but MOT[MMEmpty].fHasTray
//             is now TRUE, so the golden re-arms the 2s delay            -> 150
//    case 150 DoAutoEmptyDelay.Off()                                    -> 1
//
//  NOTE (this is the label the repair fixed): case 200 -- the branch that would
//  call DoLoadNewEmptyTrayToCar -- is NOT on this path.  It is entered only when
//  the Empty stack reads EMPTY at case 1; that arm is exercised directly by B.2.
// ===========================================================================
static void test_autoempty_tray_present_branch()
{
    printf("\n[B.1] DoAutoEmpty: Empty-tray-PRESENT branch 1->20->..->70->100->150->1\n");

    setupDummyFeed();

    // --- Set the task int to its ENTRY state explicitly ---------------------
    iAutoEmptyTask = 999;            // wild value
    InitAutoEmptyTask();             // entry
    InitAutoEmptyReceiveTask();
    CHECK(iAutoEmptyTask == 1, "InitAutoEmptyTask() sets the cursor to entry state 1");

    // Sensor fixture (explicit three-state, not inherited from Part A).
    SensorOn (SenEmptyHasTray);      // stack has a tray  -> case 1 takes the 20 arm
    SensorOn (SenEmptyCWDete);
    SensorOff(SenEmptyFixCyPush);    // fix cylinder NOT pushed -> case 40 skips JAM1003

    // Tray fixture: pre-drive BOTH flags to the OPPOSITE of what case 70 writes,
    // so the ClearTray AND the SetTray are each observable.
    MOT[MMEmpty].ClearTray("test_w6_canary");                    // case 70 SETS this
    MOT[MMEmpty_Car].SetTray(NULL_IC, "test_w6_canary");         // case 70 CLEARS this
    MOT[MMTrayY].ClearTray("test_w6_canary");
    MOT[MTrayX].ClearTray("test_w6_canary");

    // Actuator fixture: C_Empty_Fix on the wire, pre-driven ON (opposite).
    WireCylinder(C_Empty_Fix, 1, 1, 2, 5);
    Cylinder[C_Empty_Fix].On();

    Traj tr; tr.Start(iAutoEmptyTask);
    bool cursorSane   = true;
    int  fixWireAt40  = -1;      // proves case 30's Pop() drove the wire low
    int  fixWireAt60  = -1;      // proves case 50's Pop() drove the wire low
    int  prev         = iAutoEmptyTask;
    bool leftEntry    = false;
    bool returned     = false;
    int  ticks        = 0;

    const DWORD deadline = GetTickCount() + 6000;   // wall-clock backstop
    while (ticks < 5000 && GetTickCount() < deadline)
    {
        ++ticks;
        DoAutoEmpty();
        if (!autoEmptyCursorSane(iAutoEmptyTask)) cursorSane = false;
        if (iAutoEmptyTask != prev)
        {
            prev = iAutoEmptyTask;
            tr.Note(prev);
            // Re-arm mid-trajectory: case 20 already drove C_Empty_Fix low, so
            // without this the later Pop()s would be invisible.
            if (prev == 30) Cylinder[C_Empty_Fix].On();
            if (prev == 40) fixWireAt40 = Cylinder[C_Empty_Fix].GetOutBit();
            if (prev == 50) Cylinder[C_Empty_Fix].On();
            if (prev == 60) fixWireAt60 = Cylinder[C_Empty_Fix].GetOutBit();
        }
        // PROGRESS timers only -- pinned to the state that waits on them.
        if (iAutoEmptyTask == 70 || iAutoEmptyTask == 150) DoAutoEmptyDelay.SetMSAndOn(0);
        if (iAutoEmptyTask == 65)                          CheckEmptyTrayDelay.SetMSAndOn(0);

        if (iAutoEmptyTask != 1)   leftEntry = true;
        else if (leftEntry)      { returned  = true; break; }
    }

    static const int kExpect[] = { 1, 20, 30, 40, 50, 60, 70, 100, 150, 1 };
    tr.Print("DoAutoEmpty trajectory");
    printf("    (ticks=%d, fix-cylinder wire at cursor 40=%d, at cursor 60=%d)\n",
           ticks, fixWireAt40, fixWireAt60);

    CHECK(cursorSane, "cursor stayed within the documented state set the whole run");
    CHECK(tr.Equals(kExpect, (int)(sizeof(kExpect)/sizeof(kExpect[0]))),
          "DoAutoEmpty walked EXACTLY 1->20->30->40->50->60->70->100->150->1");
    CHECK(returned && iAutoEmptyTask == 1,
          "DoAutoEmpty converged back to the terminal/recurring state (Task==1)");
    CHECK(fixWireAt40 == 0,
          "case 30 Cylinder[C_Empty_Fix].Pop() drove the fix-cylinder wire LOW (re-armed high at cursor 30)");
    CHECK(fixWireAt60 == 0,
          "case 50 Cylinder[C_Empty_Fix].Pop() drove the fix-cylinder wire LOW (re-armed high at cursor 50)");
    CHECK(MOT[MMEmpty].fHasTray == true,
          "case 70 MOT[MMEmpty].SetTray() ran (flag started CLEARED)");
    CHECK(MOT[MMEmpty_Car].fHasTray == false,
          "case 70 MOT[MMEmpty_Car].ClearTray() ran (flag started SET)");
}

// ===========================================================================
//  PART B.2 -- DoLoadNewEmptyTrayToCar: the LIFTER / Cylinder[] path
//
//  This is the block the false label (T4) belonged to.  The fixture now drives
//  the Empty stack to read EMPTY at case 1 so the lifter chain really runs:
//    case 1   MMEmptyZ empty, SenEmptySelectHasTray OFF but DUMMY, CCW/Car OFF,
//             SnEmptyIsFull OFF                                        -> 50
//    case 50  IniConfig.bG11ASEReport==false                           -> 60
//    case 60  CylinderUp(C_Empty_Up) + MOT[MMEmptyZ].ClearTray()       -> 100
//    case 100 DUMMY                                                    -> 200
//    case 200 CylinderMiddle(C_Empty_Up), bEmptyPause false            -> 300
//             (golden CylinderMiddle drives the UP cylinder ON and the MIDDLE
//              cylinder OFF -- asendic.cpp case 1 `Cylinder[CylinderName].On();
//              Cylinder[CylinderName+1].Off();` and case 100 the same pair.  The
//              retired Sim body did the OPPOSITE, and B.2's midWireAt300
//              assertion below was written against the Sim body.)
//    case 300 DUMMY, arm EmptyDelay(1s)                                -> 400
//    case 400 EmptyDelay.Off()                                         -> 410
//    case 410 CylinderLower(C_Empty_Up), re-arm EmptyDelay(1s)         -> 420
//    case 420 DUMMY -> MOT[MMEmpty_Car].SetTray(), MOT[MMEmptyZ].ClearTray(),
//             iTrayTotal[1]++, TrayID[1][1]=TrayID[1][0], TrayID[1][0]="",
//             return true                                              -> 1
//
//  asendic.cpp's CylinderUp/Middle/Lower gate every command on Cylinder[].Enable
//  and BOTH lifter cylinders must therefore be Enabled AND wired, else the whole
//  lifter chain is silently a no-op.
//
//  AI(W906-W7-L1-Wave3) 20260802: "...that still returns true" is no longer true
//  either.  The real bodies WAIT for Cylinder[].OnStatus()/OffStatus(), so the
//  fixture must also give both cylinders position sensors and a physical law
//  relating them to the commanded outputs -- w3::WireLifter + w3::Tick, see
//  tests/w3_cylinder_plant.h.  Enable alone now buys nothing.
// ===========================================================================
static void test_loadnewemptytray_lifter_path()
{
    printf("\n[B.2] DoLoadNewEmptyTrayToCar: lifter/Cylinder[] path to convergence\n");

    setupDummyFeed();
    InitLoadNewEmptyTrayToCarTask();

    MOT[MMEmptyZ].ClearTray("test_w6_canary");                 // case 1 must NOT jump to 400
    MOT[MMEmpty_Car].ClearTray("test_w6_canary");              // case 420 SETS this

    // Every sensor case 1 reads is driven to an EXPLICIT OFF (Enable + TYPE_A).
    // Leaving them Enable==false would be "unknown", which reads the same on
    // IsOn() but NOT on IsOff() -- the two must not be conflated.
    SensorOff(SenEmptyCCWDete);        // else case 1 returns true on tick 0 (the old bug)
    SensorOff(SenEmptyCarHasTray);
    SensorOff(SnEmptyIsFull);          // else case 1 raises MES1020 and returns false
    SensorOff(SenEmptySelectHasTray);  // IsOff()==true, so LastSet.iRealDummy==DUMMY is
                                       // the ONLY term keeping case 1 out of the MES1021 arm

    // Lifter cylinders on the wire, pre-driven to the OPPOSITE (LOW) of what
    // case 60 commands.  AI(W906-W7-L1-Wave3) 20260802: WireCylinder() left both
    // sensor-enables FALSE, which was fine for the Sim bodies and is fatal for
    // the real ones (OnStatus()/OffStatus() would both answer false forever).
    // w3::WireLifter does the full realistic wiring -- output bit, ON sensor,
    // OFF sensor, non-zero alarm windows -- and leaves both retracted.
    w3::Forget();
    w3::WireLifter(C_Empty_Up, C_Empty_Middle, 12);
    w3::ResetCursors();
    Ld_UldDelayTime.LD_BeforeDownDelay    = 0;
    Ld_UldDelayTime.LD_StackMiddLockDelay = 0;
    Ld_UldDelayTime.LD_LiftDownDelay      = 0;

    TrayID[1][0] = "CANARY-EMPTY-ID";      // case 420 rotates [0] -> [1] and blanks [0]
    TrayID[1][1] = "";
    const int trayTotalBefore = iTrayTotal[1];

    Traj tr; tr.Start(iLoadNewEmptyTrayToCarTask);
    bool done       = false;
    int  prev       = iLoadNewEmptyTrayToCarTask;
    int  steps      = 0;
    int  upWireAt100  = -1, midWireAt100 = -1, zHasTrayAt100 = -1;
    int  midWireAt300 = -1, upWireAt300 = -1;
    int  upWireAt420  = -1, midWireAt420 = -1;

    for (steps = 0; steps < 400; ++steps)
    {
        w3::Tick();                 // move the lifter metal + expire LifterTime[][]
        const bool ret = DoLoadNewEmptyTrayToCar();
        if (iLoadNewEmptyTrayToCarTask != prev)
        {
            prev = iLoadNewEmptyTrayToCarTask;
            tr.Note(prev);
            // Re-arm the Z-stage tray flag before case 60 runs, so its ClearTray
            // is observable (case 1 required the flag CLEAR to get here at all).
            if (prev == 50)  MOT[MMEmptyZ].SetTray(NULL_IC, "test_w6_canary");
            if (prev == 100) { upWireAt100  = Cylinder[C_Empty_Up].GetOutBit();
                               midWireAt100 = Cylinder[C_Empty_Middle].GetOutBit();
                               zHasTrayAt100= MOT[MMEmptyZ].fHasTray; }
            // Re-arm: case 60's CylinderUp leaves C_Empty_Middle RAISED, and
            // golden's CylinderMiddle is precisely the transition that drops it,
            // so pre-clearing it here would MASK the very thing case 200 does.
            // AI(W906-W7-L1-Wave3) 20260802: the pre-clear is REMOVED -- with the
            // real body the observable is "case 200 lowered Middle while holding
            // Up", which needs Middle to still be HIGH when case 200 starts.
            if (prev == 300) { midWireAt300 = Cylinder[C_Empty_Middle].GetOutBit();
                               upWireAt300  = Cylinder[C_Empty_Up].GetOutBit(); }
            if (prev == 420) { upWireAt420  = Cylinder[C_Empty_Up].GetOutBit();
                               midWireAt420 = Cylinder[C_Empty_Middle].GetOutBit(); }
        }
        // PROGRESS timer only: EmptyDelay's case-300->400 role.  Its SECOND role
        // (the case-420 JAM1013 watchdog, armed by case 410) is left alone
        // because the cursor is 420, not 400, while that role is live.
        if (iLoadNewEmptyTrayToCarTask == 400) EmptyDelay.SetMSAndOn(0);
        if (ret) { done = true; break; }
    }

    static const int kExpect[] = { 1, 50, 60, 100, 200, 300, 400, 410, 420, 1 };
    tr.Print("DoLoadNewEmptyTrayToCar trajectory");
    printf("    (returned-true=%d after %d steps, final cursor=%d; wire at100 Up=%d Mid=%d,"
           " at300 Mid=%d, at420 Up=%d Mid=%d)\n",
           (int)done, steps, iLoadNewEmptyTrayToCarTask,
           upWireAt100, midWireAt100, midWireAt300, upWireAt420, midWireAt420);

    CHECK(done, "DoLoadNewEmptyTrayToCar RETURNED TRUE (the tray-loaded terminal)");
    CHECK(iLoadNewEmptyTrayToCarTask == 1,
          "DoLoadNewEmptyTrayToCar reset its cursor to the entry state 1 on convergence");
    CHECK(tr.Equals(kExpect, (int)(sizeof(kExpect)/sizeof(kExpect[0]))),
          "walked EXACTLY 1->50->60->100->200->300->400->410->420->1 (the lifter chain)");
    CHECK(upWireAt100 == 1 && midWireAt100 == 1,
          "case 60 CylinderUp(C_Empty_Up) drove BOTH lifter cylinder wires HIGH (both started LOW)");
    CHECK(zHasTrayAt100 == 0,
          "case 60 MOT[MMEmptyZ].ClearTray() ran (flag re-armed SET at cursor 50)");
    // AI(W906-W7-L1-Wave3) 20260802: ASSERTION CHANGED, and deliberately -- the
    // OLD claim ("CylinderMiddle drove C_Empty_Middle HIGH") described the
    // RETIRED Sim body, which raised the middle cylinder.  Golden's real
    // CylinderMiddle does the opposite: `Cylinder[CylinderName].On();
    // Cylinder[CylinderName+1].Off();` at case 1 and `Cylinder[CylinderName]
    // .On(); Cylinder[CylinderMidd].Off();` at case 100 -- the UP cylinder is
    // held and the MIDDLE lock is DROPPED, which is what puts the stack at the
    // middle height.  The replacement pins BOTH wires instead of one, so it is
    // strictly stronger than what it replaces: an inverted CylinderMiddle now
    // fails on either half.
    CHECK(upWireAt300 == 1 && midWireAt300 == 0,
          "case 200 CylinderMiddle(C_Empty_Up) HELD C_Empty_Up HIGH and drove C_Empty_Middle LOW (golden asendic.cpp CylinderMiddle case 1/100)");
    CHECK(upWireAt420 == 0 && midWireAt420 == 0,
          "case 410 CylinderLower(C_Empty_Up) drove BOTH lifter cylinder wires LOW (both were HIGH)");
    CHECK(MOT[MMEmpty_Car].fHasTray == true,
          "case 420 MOT[MMEmpty_Car].SetTray() ran (flag started CLEARED)");
    CHECK(MOT[MMEmptyZ].fHasTray == false,
          "case 420 MOT[MMEmptyZ].ClearTray() ran");
    CHECK(iTrayTotal[1] == trayTotalBefore + 1,
          "case 420 incremented iTrayTotal[1] EXACTLY once (Empty lane tray-in count)");
    CHECK(TrayID[1][1] == AnsiString("CANARY-EMPTY-ID") && TrayID[1][0] == AnsiString(""),
          "case 420 rotated TrayID[1][0] -> TrayID[1][1] and blanked TrayID[1][0]");
}

// ===========================================================================
//  PART B.3 -- DoAutoEmpty1: the independent Empty1 receive stepper
//
//  FAITHFUL PATH (golden DoAutoEmpty1):
//    case 1   MOT[MMEmpty1].fHasTray -> SW[SwACEmpty1CCW].On(),
//             WaitTrayDelay(15s) JAM1001 watchdog armed                 -> 100
//    case 100 DUMMY -> arm DoAutoEmpty1Delay(5s)                        -> 200
//    case 200 DoAutoEmpty1Delay.Off() && DUMMY -> SW[SwACEmpty1CCW].Off(),
//             MOT[MMEmpty1].ClearTray()                                 -> 1
//
//  The old fixture left MOT[MMEmpty1] clear and both Empty1 sensors disabled, so
//  case 1's guard never fired and the SM never left state 1 (see T3).
// ===========================================================================
static void test_autoempty1_receive()
{
    printf("\n[B.3] DoAutoEmpty1: independent Empty1 receive stepper 1->100->200->1\n");

    setupDummyFeed();
    iAutoEmpty1ReceiveTask = 1;

    MOT[MMEmpty1].SetTray(NULL_IC, "test_w6_canary");   // case 200 CLEARS this
    WireSwitch(SwACEmpty1CCW, 1, 1, 3, 0);
    SW[SwACEmpty1CCW].Off();                            // pre-drive OPPOSITE of case 1

    Traj tr; tr.Start(iAutoEmpty1ReceiveTask);
    int  prev        = iAutoEmpty1ReceiveTask;
    int  swOutAt100  = -1, swWireAt100 = -1;
    int  swOutAtEnd  = -1, swWireAtEnd = -1;
    bool converged   = false;
    int  ticks       = 0;

    const DWORD deadline = GetTickCount() + 6000;
    while (ticks < 5000 && GetTickCount() < deadline)
    {
        ++ticks;
        DoAutoEmpty1();
        if (iAutoEmpty1ReceiveTask != prev)
        {
            const int from = prev;
            prev = iAutoEmpty1ReceiveTask;
            tr.Note(prev);
            // Read OutValue (NOT Status(), which collapses to false when the
            // point is disabled); read the wire separately for polarity.
            if (prev == 100) { swOutAt100 = SW[SwACEmpty1CCW].OutValue;
                               swWireAt100 = SW[SwACEmpty1CCW].Status(); }
            if (from == 200 && prev == 1)
            {
                swOutAtEnd  = SW[SwACEmpty1CCW].OutValue;
                swWireAtEnd = SW[SwACEmpty1CCW].Status();
                converged   = true;
                break;
            }
        }
        // PROGRESS timer only.  WaitTrayDelay -- the case-100 JAM1001 watchdog --
        // is deliberately never touched.
        if (iAutoEmpty1ReceiveTask == 200) DoAutoEmpty1Delay.SetMSAndOn(0);
    }

    static const int kExpect[] = { 1, 100, 200, 1 };
    tr.Print("DoAutoEmpty1 trajectory");
    printf("    (ticks=%d, SW OutValue/wire at cursor 100 = %d/%d, at return-to-1 = %d/%d)\n",
           ticks, swOutAt100, swWireAt100, swOutAtEnd, swWireAtEnd);

    CHECK(tr.Equals(kExpect, (int)(sizeof(kExpect)/sizeof(kExpect[0]))),
          "DoAutoEmpty1 walked EXACTLY 1->100->200->1");
    CHECK(converged && iAutoEmpty1ReceiveTask == 1,
          "DoAutoEmpty1 converged back to its entry state 1");
    CHECK(swOutAt100 == 1 && swWireAt100 == 1,
          "case 1 SW[SwACEmpty1CCW].On() drove the Empty1-CCW belt output HIGH (started LOW)");
    CHECK(swOutAtEnd == 0 && swWireAtEnd == 0,
          "case 200 SW[SwACEmpty1CCW].Off() drove the Empty1-CCW belt output LOW again");
    CHECK(MOT[MMEmpty1].fHasTray == false,
          "case 200 MOT[MMEmpty1].ClearTray() ran (flag started SET)");
}

// ===========================================================================
//  PART B.4 -- fAGV satellite: the canary's only form-pointer deref
//  AGV_IsATK_AMR() is REAL translated logic (Automation/AGV_predicates.cpp), not
//  a hardcoded false, so drive it BOTH ways instead of asserting the default.
// ===========================================================================
static void test_fagv_satellite()
{
    printf("\n[B.4] fAGV satellite: IsATK_AMR() driven both ways\n");

    CHECK(fAGV != 0, "fAGV satellite constructed (non-null) -- guards the derefs below");
    if (fAGV == 0) return;

    const int  saveCoverTrayID = USE_COVER_TRAYID;
    const int  saveCustomer    = CUSTOMER_CODE;
    const bool saveBundleID    = IniConfig.bA65_BundleIDList;

    USE_COVER_TRAYID           = tCIDNotUse;
    IniConfig.bA65_BundleIDList= false;
    CHECK(fAGV->IsATK_AMR() == false, "fAGV->IsATK_AMR()==false on the offline non-AMR default");

    USE_COVER_TRAYID           = tCID_NFC;
    IniConfig.bA65_BundleIDList= true;
    CUSTOMER_CODE              = CC_AMKOR_Korea;
    CHECK(fAGV->IsATK_AMR() == true,
          "fAGV->IsATK_AMR()==true for the ATK combination (NFC cover-tray + A65 + CC_AMKOR_Korea)");

    // One term at a time back off -- proves the predicate is an AND, not a
    // latch on any single flag.
    CUSTOMER_CODE              = saveCustomer;
    CHECK(fAGV->IsATK_AMR() == false, "fAGV->IsATK_AMR()==false once CUSTOMER_CODE leaves CC_AMKOR_Korea");

    USE_COVER_TRAYID           = saveCoverTrayID;
    IniConfig.bA65_BundleIDList= saveBundleID;
    CHECK(fAGV->IsATK_AMR() == false, "fAGV->IsATK_AMR()==false after the globals are restored");
}

// ===========================================================================
//  NOT-COVERED REGISTER -- AI(W906-W6-CANARYfix) 20260801
//  State this test deliberately does NOT constrain.  Do not read a green run as
//  coverage of anything below.
//
//   asendic_Empty.cpp DoLoadNewEmptyTrayToCar
//     * case 1 alarm arms: MES1020 (SnEmptyIsFull + the SECS
//       EventReport(SECS_EVENT.NoCoverTray_Normal) call) and MES1021.  Both end
//       in ShowErrorMessage, whose sim body lives in another wave's file
//       (canary_support.cpp); the fixture keeps them out of the path.
//     * case 20 -- the EmptyMidCylinTime / JAM1006 "separation cylinder (lower)
//       error" guard.  Requires C_Empty_Middle.OffStatus()==false WITH
//       .Enable==true plus a 5s wall-clock watchdog; not driven.
//     * case 52 / 53 -- the ASE (IniConfig.bG11ASEReport) tray-ID query,
//       RespondASECom, EmptyReadTrayID timeout and WAR16335.  Not driven.
//     * case 100 / 300 real-machine arms -- Cylinder[C_EmptyLoaderZ_Select]
//       .Push() / .Pop().  Offline BOTH return true, so only bCylinderOn (or a
//       wire read) can tell them apart; the DUMMY fixture short-circuits both.
//     * case 420 -- the JAM1013 watchdog + K_SKIP recovery arm, and the
//       bEmptyHasDuplicateError latch.  Unreachable while iRealDummy==DUMMY.
//     * case 200 -- the bEmptyPause "Empty split wait" ShowMyMessage re-entry.
//
//   asendic.cpp lifter helpers
//     * These are SIM bodies, not the golden lifter state machines (iLifterTask/
//       LifterTime/LOAD_Z_USE_MOTOR are not translated).  B.2 pins WHICH cylinder
//       each helper commands and in WHICH direction; it does NOT pin lifter
//       timing, motor-driven lifters, or the golden's per-Part branch table.
//
//   Actuator polarity
//     * B.1/B.2/B.3 wire every actuator they assert on with OutType==TYPE_A.
//       The INVERTING wiring (OutType==TYPE_B, where OnSwitch drives IOBitOff)
//       is not exercised for these points.
//
//   Part A predicates
//     * Every HasIC-family leaf is still conservatively false because the
//       TMyKitSuck grid is gated #if 0.  Part A therefore pins the GATE, not the
//       predicate logic; it cannot detect a wrong aggregation once the grid lands.
// ===========================================================================

int main()
{
    printf("==============================================================\n");
    printf(" W6 VERIFY -- predicate API + Empty-tray canary stepper\n");
    printf("==============================================================\n");

    test_predicates_known_state();
    test_fixture_contract();
    test_autoempty_tray_present_branch();
    test_loadnewemptytray_lifter_path();
    test_autoempty1_receive();
    test_fagv_satellite();

    printf("\n--------------------------------------------------------------\n");
    printf(" RESULT: %d passed, %d failed\n", g_pass, g_fail);
    printf("--------------------------------------------------------------\n");
    return (g_fail == 0) ? 0 : 1;
}
