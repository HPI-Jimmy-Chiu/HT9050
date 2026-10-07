// =============================================================================
//  test_w7_l1_color.cpp  --  W7-L1 Wave 1: asendic_Color (Color-tray feed) SM test
//
//  Translation wave: W906-W7-L1 Wave 1
//  Author: AI(W906-W7-L1-Color) 20260802
//
//  PROOF OBJECTIVE
//  ---------------
//  Drive the Color-tray stack-feed SM family over the Sim HAL
//  (Cylinder[]/MOT[]/Sen[]/SW[]/MyLaneIO) with no hardware, and assert that each
//  SM walks the SPECIFIC cursor trajectory golden prescribes AND leaves the
//  actuators + bookkeeping in the state golden prescribes AT THE POINT golden
//  prescribes it.  Every assertion below has been driven RED by a targeted
//  mutation of a scratch copy of asendic_Color.cpp and back GREEN by reverting
//  it; the mutation table is in the translate report, and each assertion group
//  names its own probe in the comment above it.
//
//  Scope of the trajectory claim: a trajectory assertion is load-bearing for the
//  `Task=N` transitions ON THE DRIVEN PATHS listed under WHAT IT DRIVES.  It says
//  nothing about the arms in the NOT-COVERED register at the bottom of this file,
//  and is not claimed to.
//
//  THREE FIXTURE SEAMS (all fixture-only; zero production change)
//  --------------------------------------------------------------
//  1. VIRTUAL TIME.  TQPF_Timer is a real QueryPerformanceCounter wall clock with
//     no injection point, so a tight loop can never outlast a SetSecAndOn(>0)
//     wait.  Instead the file-scope timers of asendic_Color.cpp are extern'd here
//     and RE-ARMED to 0 ms (SetMSAndOn(0) -> rEnd = rStart - 2*overhead, so the
//     next Off() is true).  CRITICALLY, each timer is only re-armed WHILE THE
//     CURSOR IS PARKED ON THE STATE WHERE ITS PROGRESS ROLE IS LIVE -- see
//     advanceLoadCursor / advanceRearCursor etc.  THREE OF THEM ARE DUAL-ROLE and
//     are handled by cursor, never blanket-advanced:
//       * ColorDelay          progress at DoLoadNewColorTrayToCar case 400;
//                             JAM1413 WATCHDOG at case 420.  Advanced only at 400.
//       * hColorTrayToRear    arrival settle at DoColorTrayToRear 450/460/480;
//                             JAM1401 WATCHDOG at case 200.  Advanced only at 460.
//       * ColorReadTrayID     the tray-ID read deadline at case 53 -- it is the
//                             arm UNDER TEST in [3f], advanced there deliberately
//                             and nowhere else.
//     hColorTrayToFront (20 s JAM1412) is NEVER advanced.
//  2. SIM SENSOR LEVELS -- Sen[] is TRI-STATE and this file uses all three:
//       simSensorUnknown : Enable=false          -> IsOn()==false AND IsOff()==false
//       simSensorOff     : Enable + TYPE_A(1)    -> IsOff()==true , IsOn()==false
//       simSensorOn      : Enable + TYPE_B(0)    -> IsOn()==true  , IsOff()==false
//     (ISABase=eISABase routes to mysensor.cpp's raw-port stub, which is always
//     false, so Type alone picks the polarity.)  Sub-test [2] proves all three.
//  3. SIM WIRE-LEVEL SWITCH READBACK.  DoColorTrayToFront case 100 READS
//     SW[SwACColorCCW].Status() / SW[SwACColorCW].Status() (golden :304-305) and
//     re-arms BOTH of its deadlines when both read stopped.  TMySwitch::Status()
//     returns false whenever Enable==false, so with the default fixture that
//     re-arm fires on EVERY tick and the DUMMY-timeout arm at golden :323-331 is
//     structurally unreachable.  simSwitchWire() gives the two switches a real
//     Sim IO address (Enable + TYPE_A + eMotionNet + Ring1/IP1/Port2/Bit n), after
//     which On() really sets the wire bit and Status() really reads it back --
//     which is what makes golden :304-309 observable at all (mutation P1).
//
//  WHAT IT DRIVES
//    * All 7 Init* (4 exported + 3 .cpp-internal) reset their own cursor to 1;
//      InitColorTrayToRearTask additionally releases the iReadCIDAction port lock
//      and zeroes iColorIdTrayCt.
//    * DoLoadNewColorTrayToCar (golden :42-274) FIVE ways: the DUMMY lifter walk
//      50,60,100,200,300,400,410,420; the case-1 short-circuit; the case-1 ->400
//      fast entry; the case-1 ELSE arm -> case 20 -> JAM1406; and the non-DUMMY
//      ASE tray-ID read 50,52,53,60,... including the WAR16335 timeout arm.
//    * DoColorTrayToFront (golden :285-371) BOTH ways: the car-has-tray arm
//      (1->200->300) and the DUMMY-timeout arm (park at 100 -> 300).
//    * DoColorTrayToRear (golden :387-811) THREE ways: the plain non-AMR path
//      200,450,480,500,510; the Keyence tray-ID hand-off 510->520 and its
//      bEnableTrayID2==false exit; and the ATK-AMR RFID path 450,455,460 with
//      DoColorTrayReadCID answering +1 (-> 480,500,600) and -1 (-> 465 / 470).
//    * DoColorTrayReadCID (golden :1487-1552) as a unit, both fall-throughs.
//    * DoUnLoadNewColorToStack (golden :1170-1291) path 50,100,150,200,300,400,500
//      plus the case-1 ->10 recirculation and the case-10 ->50 double-tray arm.
//    * DoAutoColorReceive (golden :1300-1453) full chain, the bP04 case-50 arm,
//      the P24 bNeedAlarm park/release pair, and the case-300 pre-alarm branch
//      (which then makes the case-200 MES1420 pre-alarm arm reachable).
//    * DoAutoColor (golden :821-1161) over five scenarios + all four early-return
//      gates + the fMain->StringGrid2 supply-flag log line.
//    * ForTERAPOWERCheckColorSensor (golden :1455-1478) -- all four combinations
//      of (Tray-From-Empty / Tray-From-Color) x (sensor on / sensor off), which is
//      what pins the IsOn()/IsOff() spelling AND the RT/FT row selection.
//
//  Offline: no vendor SDK, no hardware, no window.
// =============================================================================
#include "asendic_Color.h"
#include "csystem.h"
#include "asendic.h"
#include "acatchtray.h"
#include "acatchtray_shims.h"   // fTrayMapping / MyMessageBox / iReadCIDAction
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "MyLaneIo.h"
#include "myTimer.h"            // TQPF_Timer (the virtual-time seam below)
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage/ShowMyMessage seams
#include "FormsFacade.h"        // fMain / fSortCT / fAGV
// AI(W906-W7-L1-Wave3) 20260802: CylinderUp/Middle/Lower are golden's real
// closed-loop lifter SMs now (asendic.cpp), not Sim bodies that returned true on
// their first call.  This header is the physical Color tray-group stack that
// answers their position sensors; w3::Tick() at the top of every drive loop below
// moves it and expires LifterTime[][].
#include "w3_cylinder_plant.h"
#include <cstdio>
#include <set>

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as test_w7_l1_auto2.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  VIRTUAL-TIME SEAM -- the file-scope TQPF_Timer objects of asendic_Color.cpp,
//  and the two file-scope ints that are not in asendic_Color.h (golden does not
//  declare them either; they have external linkage in golden too).
// ---------------------------------------------------------------------------
extern TQPF_Timer ColorDelay;                    // golden asendic_Color.cpp:33  (DUAL ROLE)
extern TQPF_Timer ColorMidCylinTime;             // golden asendic_Color.cpp:34
extern TQPF_Timer ColorReadTrayID;               // golden asendic_Color.cpp:35
extern TQPF_Timer hColorTrayToFront;             // golden asendic_Color.cpp:277 (20 s WATCHDOG -- never advanced)
extern TQPF_Timer hColorTrayToFrontForDummy;     // golden asendic_Color.cpp:278
extern TQPF_Timer UnloadDelay;                   // golden asendic_Color.cpp:279
extern TQPF_Timer hColorTrayToRear;              // golden asendic_Color.cpp:375 (DUAL ROLE)
extern TQPF_Timer hColorTrayToRearForDummy;      // golden asendic_Color.cpp:376
extern TQPF_Timer hColorUnderCoveyorDelay;       // golden asendic_Color.cpp:377
extern TQPF_Timer DoAutoColorDelay;              // golden asendic_Color.cpp:814
extern TQPF_Timer CheckColorTrayDelay;           // golden asendic_Color.cpp:815
extern TQPF_Timer DoUnLoadNewColorToStackDelay;  // golden asendic_Color.cpp:1164
extern TQPF_Timer tNFCColorDelay;                // golden asendic_Color.cpp:1481
extern int iColorIdTrayCt;                       // golden asendic_Color.cpp:374

// .cpp-internal entry points golden does not declare in its header either.
extern void InitLoadNewColorTrayToCarTask();     // golden asendic_Color.cpp:37
extern void InitColorTrayToFrontTask();          // golden asendic_Color.cpp:280
extern bool DoColorTrayToFront();                // golden asendic_Color.cpp:285
extern void InitUnLoadNewColorTrayTask();        // golden asendic_Color.cpp:1165
extern bool DoUnLoadNewColorToStack();           // golden asendic_Color.cpp:1170

// ---------------------------------------------------------------------------
//  SIM SENSOR SEAM -- see banner note 2.  All THREE levels, because this SM
//  family reads IsOn() and IsOff() as INDEPENDENT predicates, not complements.
// ---------------------------------------------------------------------------
static void simSensorOff(int idx)       // IsOff()==true , IsOn()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_A;          // cmydef.cpp:269 TYPE_A==1
    Sen[idx].ISABase = eISABase;        // -> mysensor.cpp raw-port stub (false)
}
static void simSensorOn(int idx)        // IsOn()==true  , IsOff()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_B;          // cmydef.cpp:270 TYPE_B==0
    Sen[idx].ISABase = eISABase;
}
static void simSensorUnknown(int idx)   // IsOn()==false AND IsOff()==false
{
    Sen[idx].Enable = false;
}

// ---------------------------------------------------------------------------
//  SIM WIRE SEAM -- see banner note 3.
// ---------------------------------------------------------------------------
static void simSwitchWire(int idx, int bit)
{
    SW[idx].Enable  = true;
    SW[idx].Type    = TYPE_A;
    SW[idx].ISABase = eMotionNet;       // -> MyLaneIO Sim backend (real bit store)
    SW[idx].Ring    = 1;
    SW[idx].IP      = 1;
    SW[idx].Port    = 2;
    SW[idx].Bit     = bit;
}

// ---------------------------------------------------------------------------
//  Trajectory recorder + reporter
// ---------------------------------------------------------------------------
typedef std::set<int> StateSet;

static bool covers(const StateSet &seen, const int *want, int nWant)
{
    for (int i = 0; i < nWant; ++i)
        if (seen.find(want[i]) == seen.end())
            return false;
    return true;
}
static void printSeen(const char *what, const StateSet &seen)
{
    printf("    (%s visited:", what);
    for (StateSet::const_iterator it = seen.begin(); it != seen.end(); ++it)
        printf(" %d", *it);
    printf(")\n");
}
static void printMissing(const char *what, const StateSet &seen,
                         const int *want, int nWant)
{
    printf("    (%s MISSING:", what);
    for (int i = 0; i < nWant; ++i)
        if (seen.find(want[i]) == seen.end())
            printf(" %d", want[i]);
    printf(")\n");
}

// ---------------------------------------------------------------------------
//  The shared run-mode fixture.  Everything the Color SM family reads that is
//  NOT a per-sub-test variable is pinned here so no sub-test can inherit another
//  one's state.  The three settle delays are pinned to 0 s because they are
//  operator-tunable machine settings (Ld_ULd form), not SM logic.
// ---------------------------------------------------------------------------
static void setupColorFixture()
{
    LastSet.iRealDummy    = DUMMY;      // dummy feed
    LastSet.iRunStartMode = 0;

    iReceiveColorTray            = 0;
    fColorCanSupplyNewTray       = true;
    bColorPause                  = false;
    bHandlerPause                = false;
    bReadColorTrayID             = false;
    bIsCatchingFromBuffer        = false;
    bIsPlacingToBuffer           = false;
    bTrayArmIsPickColor          = false;
    bEmptyReadIDOK               = false;
    bLoadNewColorTrayToCarStart  = false;
    bUnLoadNewColorToStackStart  = false;
    iUnLoaderCount               = 0;
    iColorIdTrayCt               = 0;

    CUSTOMER_CODE         = 0;
    AUTO_EMPTY_COLOR      = 0;
    TRAY_ARM_MODE         = 0;          // NOT eUnderCoveyor
    DOUBLE_BELT_MODE      = 0;
    USE_COLOR_TRAY_SENSOR = 0;
    USE_TRAY_MAPPING      = 0;
    USE_KEYENCE_EMPTY     = 0;
    USE_COVER_TRAYID      = tCIDNotUse; // -> fAGV->IsATK_AMR()==false

    IniConfig.bA65_BundleIDList                       = false;
    IniConfig.bEnable_SECS_GEM                        = false;
    IniConfig.bG11ASEReport                           = false;
    IniConfig.bP04ColorIsEmptyUnloader                = false;
    IniConfig.bP32EmptyColorTrayPreAlarm              = false;
    IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray= false;
    IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty   = false;
    CosFunction.bUseEmptyColorTrayPreAlarm            = false;
    CosFunction.bSpecialP24                           = false;
    TrayForm.bEnableAMR   = false;
    TrayForm.bColorTray   = false;

    // AI(W906-W7-L1-Wave3) 20260802: wire the Color tray-group lifter the way a
    // real machine wires it (Enable + distinct Sim IO addresses for the output
    // bit and the two position sensors + non-zero alarm windows), and reset the
    // lifter cursors the way golden main.cpp:9159-9160 does at FormShow.  Golden's
    // real CylinderUp/Middle/Lower are closed loops on those sensors; with the
    // default all-false wiring they can never report arrival and every SM that
    // calls them parks.
    w3::Forget();
    w3::WireLifter(C_Color_Up, C_Color_Middle, 13);
    w3::ResetCursors();
    Ld_UldDelayTime.LD_BeforeDownDelay    = 0;
    Ld_UldDelayTime.LD_StackMiddLockDelay = 0;
    Ld_UldDelayTime.LD_LiftDownDelay      = 0;
    TestIF_File.bEnableTrayID2 = false;

    Ld_UldDelayTime.ULD_LiftDownDelay  = 0;
    Ld_UldDelayTime.LD_TrayArrivalDely = 0;
    Ld_UldDelayTime.ULD_TrayBackDelay  = 0;

    iReadCIDAction = ePortTotal;        // port lock released
    asTrayIDData2  = "";

    // Color track/stack sensors: "unknown" by default (an unconfigured offline IO
    // table); each sub-test raises exactly the ones it needs.
    simSensorUnknown(SenColorSelectHasTray);
    simSensorUnknown(SenColorCCWDete);
    simSensorUnknown(SenColorCarHasTray);
    simSensorUnknown(SenColorHasTray);
    simSensorUnknown(SenColorCWDete);
    simSensorUnknown(SenColorFixCyPush);
    simSensorUnknown(SnColorIsFull);
    simSensorUnknown(SnColorIsPreAlarm);
    simSensorUnknown(SnColorSeparateHasTray);
    simSensorUnknown(SnColorColorTrayDetect);

    // Lifter pair MUST be Enable'd: asendic.cpp's CylinderUp/Middle/Lower only
    // touch Cylinder[] `if(...Enable)`, so with Enable==false the whole lifter
    // surface is invisible and every lifter assertion below would be vacuous.
    Cylinder[C_Color_Up].Enable            = true;
    Cylinder[C_Color_Middle].Enable        = true;
    Cylinder[C_Color_Fix].Enable           = true;
    Cylinder[C_ColorLoaderZ_Select].Enable = true;
    Cylinder[C_ColorTrackFloodgate].Enable = true;   // golden guards on .Enable

    // Switches unwired by default (Status()==false) -- [4b] wires them.
    SW[SwACColorCCW].Enable = false;
    SW[SwACColorCW].Enable  = false;

    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].ClearTray("test_w7_l1_color");
    MOT[MMColorZ].ClearTray("test_w7_l1_color");
    MOT[MMTrayY].ClearTray("test_w7_l1_color");
    MOT[MTrayX].ClearTray("test_w7_l1_color");
    MOT[MMColor].Tray.iNeedManualRemoved     = -1;
    MOT[MMColor_Car].Tray.iNeedManualRemoved = -1;

    fTrayMapping->ldRFID->bCommConnect = false;
    fTrayMapping->ldRFID->bClearCmd2   = false;
    fTrayMapping->ldRFID->bReadUID2    = false;
    fTrayMapping->ldRFID->sBlockID2    = "";
    fTrayMapping->ldRFID->sUID2        = "";
    fMain->chkE84IDTray->Checked       = false;

    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();
}

// ---------------------------------------------------------------------------
//  [1] Init* reset their own cursors (and the port lock)
// ---------------------------------------------------------------------------
static void test_inits()
{
    printf("\n[1] Init* reset task cursors to 1\n");
    setupColorFixture();

    iLoadNewColorTrayToCarTask = 999;  InitLoadNewColorTrayToCarTask();
    CHECK(iLoadNewColorTrayToCarTask == 1, "InitLoadNewColorTrayToCarTask() -> cursor==1");

    iColorTrayToFrontTask = 999;       InitColorTrayToFrontTask();
    CHECK(iColorTrayToFrontTask == 1, "InitColorTrayToFrontTask() -> cursor==1");

    iAutoColorTask = 999;              InitAutoColorTask();
    CHECK(iAutoColorTask == 1, "InitAutoColorTask() -> cursor==1  (RETIRES the csystem_shims no-op)");

    iAutoColorReceiveTask = 999;       InitAutoColorReceiveTask();
    CHECK(iAutoColorReceiveTask == 1, "InitAutoColorReceiveTask() -> cursor==1  (RETIRES the acatchtray_shims no-op)");

    iUnLoadNewColorTrayTask = 999;     InitUnLoadNewColorTrayTask();
    CHECK(iUnLoadNewColorTrayTask == 1, "InitUnLoadNewColorTrayTask() -> cursor==1");

    iColorTrayReadCIDTask = 999;       InitColorTrayReadCIDTask();
    CHECK(iColorTrayReadCIDTask == 1, "InitColorTrayReadCIDTask() -> cursor==1");

    // InitColorTrayToRearTask does THREE things, not one (golden :378-384).
    iColorTrayToRearTask = 999;
    iColorIdTrayCt       = 7;
    iReadCIDAction       = ePortColor;          // port locked by this port
    InitColorTrayToRearTask();
    CHECK(iColorTrayToRearTask == 1, "InitColorTrayToRearTask() -> cursor==1");
    CHECK(iReadCIDAction == ePortTotal,
          "InitColorTrayToRearTask() RELEASED the iReadCIDAction port lock (golden :380-381)");
    CHECK(iColorIdTrayCt == 0, "InitColorTrayToRearTask() zeroed iColorIdTrayCt (golden :383)");

    // ...and only when the lock is OURS: a Loader-held lock must survive.
    iReadCIDAction = ePortLoader;
    InitColorTrayToRearTask();
    CHECK(iReadCIDAction == ePortLoader,
          "InitColorTrayToRearTask() leaves a LOADER-held port lock alone (golden :380 guard)");
    iReadCIDAction = ePortTotal;
}

// ---------------------------------------------------------------------------
//  [2] The three fixture seams must work, or every assertion below is vacuous.
// ---------------------------------------------------------------------------
static void test_seams()
{
    printf("\n[2] Fixture seams (virtual time / tri-state sensor / Sim wire)\n");
    setupColorFixture();

    DoAutoColorDelay.SetMSAndOn(1000);
    CHECK(DoAutoColorDelay.Off() == false, "a freshly armed 1000 ms TQPF_Timer is NOT elapsed");
    DoAutoColorDelay.SetMSAndOn(0);
    CHECK(DoAutoColorDelay.Off() == true,  "the same timer re-armed to 0 ms IS elapsed");

    simSensorUnknown(SenColorHasTray);
    CHECK(Sen[SenColorHasTray].IsOn() == false && Sen[SenColorHasTray].IsOff() == false,
          "TRI-STATE: Enable==false reads neither On nor Off ('unknown')");
    simSensorOff(SenColorHasTray);
    CHECK(Sen[SenColorHasTray].IsOff() == true && Sen[SenColorHasTray].IsOn() == false,
          "TRI-STATE: Enable+TYPE_A reads Off");
    simSensorOn(SenColorHasTray);
    CHECK(Sen[SenColorHasTray].IsOn() == true && Sen[SenColorHasTray].IsOff() == false,
          "TRI-STATE: Enable+TYPE_B reads On");
    simSensorUnknown(SenColorHasTray);

    simSwitchWire(SwACColorCCW, 4);
    SW[SwACColorCCW].Off();
    CHECK(SW[SwACColorCCW].Status() == false, "Sim wire: Off() -> Status()==false");
    SW[SwACColorCCW].On();
    CHECK(SW[SwACColorCCW].Status() == true,  "Sim wire: On() -> Status()==true (real bit readback)");
    SW[SwACColorCCW].Off();
    SW[SwACColorCCW].Enable = false;
}

// ---------------------------------------------------------------------------
//  [3a] DoLoadNewColorTrayToCar -- DUMMY lifter walk.
//
//  MUTATION PROBES for this group (each applied ALONE to a scratch copy of
//  asendic_Color.cpp; see the FAILABILITY PROOF block at the foot of this file):
//    M01 golden :186 CylinderUp -> CylinderLower       -> lifter-up red
//    M02 golden :188 MOT[MMColorZ].ClearTray deleted   -> zAt100 red
//    M03 golden :249 MOT[MMColorZ].ClearTray deleted   -> re-armed clear red
//    M04 golden :252 iTrayTotal[2]++ deleted           -> tray-total red
//    M05 golden :253 TrayID rotation broken            -> rotation red
//    M06 golden :230 CylinderLower -> CylinderUp       -> lifter-down red
// ---------------------------------------------------------------------------
static void test_load_new_color_dummy_walk()
{
    printf("\n[3a] DoLoadNewColorTrayToCar() DUMMY lifter walk 50,60,100,200,300,400,410,420\n");

    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    simSensorOff(SenColorCCWDete);      // must NOT read On, else case 1 short-circuits
    simSensorOff(SenColorCarHasTray);

    // Pre-drive the lifter pair to the OPPOSITE of what case 60 must write, so a
    // DELETED CylinderUp fails the same assertion an INVERTED one does.
    Cylinder[C_Color_Up].Off();
    Cylinder[C_Color_Middle].Off();

    TrayID[2][0] = "COLORCID1";
    TrayID[2][1] = "";
    const int trayTotalBefore = iTrayTotal[2];

    StateSet seen;
    bool done = false;
    int  steps = 0;
    int  zAt100 = -1;       // MMColorZ.fHasTray sampled the first time cursor rests on 100
    int  upAt100 = -1, midAt100 = -1;
    bool armedForCase60 = false, rearmedForCase420 = false;
    for (steps = 0; steps < 200 && !done; ++steps)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        // ColorDelay is DUAL-ROLE: advance it ONLY at cursor 400, where its
        // progress role is live (golden :224).  At 420 it is the JAM1413 watchdog.
        if (iLoadNewColorTrayToCarTask == 400) ColorDelay.SetMSAndOn(0);
        // Arm the Z stage before case 60 runs, so golden :188's clear is visible.
        if (iLoadNewColorTrayToCarTask == 50 && !armedForCase60)
        {
            MOT[MMColorZ].SetTray(NULL_IC, "test_w7_l1_color");
            armedForCase60 = true;
        }
        // Re-arm it again before case 420 runs, so golden :249's clear is
        // INDEPENDENTLY load-bearing (without this, deleting :249 alone would find
        // fHasTray already false, left over from :188).
        if (iLoadNewColorTrayToCarTask == 410 && !rearmedForCase420)
        {
            MOT[MMColorZ].SetTray(NULL_IC, "test_w7_l1_color");
            rearmedForCase420 = true;
        }
        done = DoLoadNewColorTrayToCar();
        seen.insert(iLoadNewColorTrayToCarTask);
        if (iLoadNewColorTrayToCarTask == 100 && zAt100 < 0)
        {
            zAt100   = MOT[MMColorZ].fHasTray ? 1 : 0;
            upAt100  = Cylinder[C_Color_Up].bCylinderOn ? 1 : 0;
            midAt100 = Cylinder[C_Color_Middle].bCylinderOn ? 1 : 0;
        }
    }

    static const int want[] = {50, 60, 100, 200, 300, 400, 410, 420};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(done, "DoLoadNewColorTrayToCar() returns true (converged, not bound-exhausted)");
    CHECK(covers(seen, want, nWant),
          "trajectory covers 50,60,100,200,300,400,410,420");
    CHECK(upAt100 == 1 && midAt100 == 1,
          "case 60 RAISED the lifter pair before handing to 100 (golden :186 CylinderUp, not Lower)");
    CHECK(zAt100 == 0,
          "case 60 cleared the MMColorZ tray flag before handing to 100 (golden :188)");
    CHECK(rearmedForCase420 && MOT[MMColorZ].fHasTray == false,
          "case 420 cleared the RE-ARMED MMColorZ tray flag on success (golden :249)");
    CHECK(Cylinder[C_Color_Up].bCylinderOn == false &&
          Cylinder[C_Color_Middle].bCylinderOn == false,
          "case 410 LOWERED the lifter pair (golden :230 CylinderLower, not Up)");
    CHECK(MOT[MMColor_Car].fHasTray == true,
          "case 420 latched the tray onto MMColor_Car (golden :248)");
    CHECK(iTrayTotal[2] == trayTotalBefore + 1,
          "case 420 incremented iTrayTotal[2] exactly once (golden :252 -- index 2 == Color)");
    CHECK(TrayID[2][1] == "COLORCID1" && TrayID[2][0] == "",
          "case 420 rotated the Color tray ID read->pick and cleared the read slot (golden :253-254)");
    CHECK(iLoadNewColorTrayToCarTask == 1, "cursor reset to 1 on convergence");
    CHECK(W906_ShowErrorMessage_Count == 0, "no alarm was raised anywhere on the happy path");
    printSeen("DoLoadNewColorTrayToCar", seen);
    if (!covers(seen, want, nWant)) printMissing("DoLoadNewColorTrayToCar", seen, want, nWant);
}

// ---------------------------------------------------------------------------
//  [3b] DoLoadNewColorTrayToCar case-1 short-circuit (golden :65-72) and
//  [3c] the case-1 -> 400 fast entry (golden :55-60).
//  PROBE: golden :68 SetTray deleted -> [3b] car-latch red;
//         golden :58 `Task=400;` -> `Task=1;` -> [3c] red.
// ---------------------------------------------------------------------------
static void test_load_new_color_shortcircuit_and_fast_entry()
{
    printf("\n[3c] DoLoadNewColorTrayToCar() case-1 -> 400 fast entry (Z stage occupied)\n");
    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    simSensorOn(SenColorCCWDete);                   // would short-circuit, if we ever got that far
    MOT[MMColorZ].SetTray(NULL_IC, "test_w7_l1_color");

    bool first = DoLoadNewColorTrayToCar();
    CHECK(first == false,
          "case 1 with MMColorZ occupied does NOT short-circuit -- it takes the ->400 fast entry");
    CHECK(iLoadNewColorTrayToCarTask == 400,
          "case-1 fast entry: MMColorZ occupied -> cursor 400 (golden :55-59)");
    CHECK(ColorDelay.Off() == false,
          "the fast entry armed the 1 s ColorDelay before handing to 400 (golden :57)");

    printf("\n[3b] DoLoadNewColorTrayToCar() case-1 short-circuit (CCW sensor ON)\n");
    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    simSensorOn(SenColorCCWDete);
    MOT[MMColor_Car].ClearTray("test_w7_l1_color");
    // The Z stage MUST read empty or case 1 takes the fast entry above -- so
    // golden :69's ClearTray cannot be observed through fHasTray here.  It IS
    // observable through the OTHER field ClearTray always resets: stamp the Z
    // stage's tray ID and require the short-circuit to wipe it.
    MOT[MMColorZ].fHasTray      = false;
    MOT[MMColorZ].Tray.cTrayID  = "ZSTAGE-ID";

    bool sc = DoLoadNewColorTrayToCar();
    CHECK(sc == true, "returns true on the FIRST step (case-1 short-circuit, golden :65-71)");
    CHECK(iLoadNewColorTrayToCarTask == 1, "cursor never left 1 -- case 50 NOT reached");
    CHECK(MOT[MMColor_Car].fHasTray == true,
          "short-circuit latched MMColor_Car (golden :68 SetTray)");
    CHECK(MOT[MMColorZ].Tray.cTrayID == "",
          "short-circuit also cleared the MMColorZ stage record (golden :69 ClearTray)");
}

// ---------------------------------------------------------------------------
//  [3d] DoLoadNewColorTrayToCar case-1 ELSE arm -> case 20 -> JAM1406.
//  This is the only path that drives the two lifter cylinders DOWN directly
//  (golden :101-102 / :114-115) rather than through CylinderLower.
//  PROBES: golden :101 `.Off()` -> `.On()`  -> cylinder-down red;
//          golden :110 "MES1421" -> "MES1420" -> MES1421 code red;
//          golden :123 "JAM1406" -> "JAM1403" -> JAM1406 code red.
// ---------------------------------------------------------------------------
static void test_load_new_color_else_arm_and_jam1406()
{
    printf("\n[3d] DoLoadNewColorTrayToCar() case-1 ELSE arm -> case 20 -> JAM1406\n");

    // --- first: AUTO_EMPTY_COLOR==0 leaves only the MES1421 alarm -------------
    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    LastSet.iRealDummy = REALLY;                 // so the case-1 DUMMY disjunct is false
    simSensorOff(SenColorSelectHasTray);         // IsOff()==true -> `IsOff()==false` is FALSE -> ELSE arm
    AUTO_EMPTY_COLOR = 0;
    DoLoadNewColorTrayToCar();
    CHECK(W906_ShowErrorMessage_LastCode == "MES1421" &&
          W906_ShowErrorMessage_LastKCode == K_RETRY,
          "ELSE arm with AUTO_EMPTY_COLOR==0 raises MES1421/K_RETRY (golden :110)");
    CHECK(iLoadNewColorTrayToCarTask == 1, "MES1421 arm leaves the cursor on 1");

    // --- then: AUTO_EMPTY_COLOR!=0 runs the cylinder-down protection ----------
    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    LastSet.iRealDummy = REALLY;
    simSensorOff(SenColorSelectHasTray);
    simSensorOff(SenColorCarHasTray);            // golden :99 needs IsOff() on BOTH
    simSensorOff(SenColorCCWDete);
    AUTO_EMPTY_COLOR = 1;
    MOT[MMColor_Car].ClearTray("test_w7_l1_color");
    // Pre-drive BOTH lifter cylinders ON so a DELETED .Off() fails the same
    // assertion an INVERTED one does.
    Cylinder[C_Color_Up].On();
    Cylinder[C_Color_Middle].On();
    // AI(W906-W7-L1-Wave3) 20260802: settle the plant on that pre-drive.  Golden
    // :96-97 gates this whole arm on `Cylinder[C_Color_Middle].OffStatus()==false
    // && Enable==true` -- "the separation cylinder is still up".  Before Wave 3
    // the fixture left OffSenEnable false, so OffStatus() answered false for the
    // WRONG reason (no sensor at all) and the gate opened by accident.  Now the
    // cylinder really has an OFF sensor, so the fixture has to actually put the
    // stack UP for the gate to open -- one plant tick on the pre-drive does that
    // (both commanded ON -> position `top` -> Middle's OFF sensor not made).
    // Deliberately the ONLY tick in this block: the two DoLoadNewColorTrayToCar()
    // calls below must see the stack still UP, which is the condition case 20 is
    // protecting against.
    w3::Tick();

    DoLoadNewColorTrayToCar();
    CHECK(iLoadNewColorTrayToCarTask == 20,
          "ELSE arm handed to case 20 (the Middle-cylinder-stuck protection, golden :104)");
    CHECK(Cylinder[C_Color_Up].bCylinderOn == false &&
          Cylinder[C_Color_Middle].bCylinderOn == false,
          "case 1 ELSE arm LOWERED both lifter cylinders (golden :101-102 Off(), not On())");
    CHECK(ColorMidCylinTime.Off() == false,
          "case 1 ELSE arm armed the 5 s ColorMidCylinTime before handing to 20 (golden :103)");

    // case 20 keeps re-commanding down until the 5 s guard expires, then alarms.
    W906_ShowErrorMessage_Reset();
    DoLoadNewColorTrayToCar();                   // guard not yet expired
    CHECK(iLoadNewColorTrayToCarTask == 20 && W906_ShowErrorMessage_Count == 0,
          "case 20 waits (no alarm) while ColorMidCylinTime is still running");
    ColorMidCylinTime.SetMSAndOn(0);             // the arm UNDER TEST -- see banner note 1
    DoLoadNewColorTrayToCar();
    CHECK(W906_ShowErrorMessage_LastCode == "JAM1406" &&
          W906_ShowErrorMessage_LastKCode == K_RETRY,
          "case 20 raised JAM1406/K_RETRY once ColorMidCylinTime expired (golden :123)");
    CHECK(iLoadNewColorTrayToCarTask == 1, "JAM1406 arm reset the cursor to 1 (golden :124)");
}

// ---------------------------------------------------------------------------
//  [3e] DoLoadNewColorTrayToCar non-DUMMY ASE tray-ID read 50,52,53,60,... and
//       the Select-cylinder Push/Pop pair that the DUMMY walk short-circuits.
//  PROBES: golden :132 `&& LastSet.iRealDummy!=DUMMY` deleted -> still green
//            (documented in the report as a fixture-invariant limitation, NOT a
//            covered site) ; golden :195 `.Push()` -> `.Pop()` -> select-push red;
//          golden :216 `.Pop()` -> `.Push()` -> select-pop red;
//          golden :253 `TrayID[2][1]=TrayID[2][0]` -> `TrayID[2][1]=""` -> red.
// ---------------------------------------------------------------------------
static void test_load_new_color_ase_read()
{
    printf("\n[3e] DoLoadNewColorTrayToCar() non-DUMMY ASE tray-ID read + Select cylinder\n");

    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    LastSet.iRealDummy      = REALLY;
    IniConfig.bG11ASEReport = true;
    simSensorOn(SenColorSelectHasTray);   // IsOff()==false -> take the FEED branch
    simSensorOff(SenColorCCWDete);
    simSensorOff(SenColorCarHasTray);
    Cylinder[C_ColorLoaderZ_Select].Off();   // pre-drive OPPOSITE of case 100's Push()
    TrayID[2][0] = "";
    TrayID[2][1] = "";

    StateSet seen;
    bool done = false;
    int  selectAt200 = -1, selectAt400 = -1;
    bool fedTrayID = false, flippedCar = false;
    for (int i = 0; i < 200 && !done; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iLoadNewColorTrayToCarTask == 400) ColorDelay.SetMSAndOn(0);
        // The RFID/ASE reader answers once the SM is parked on 53.
        if (iLoadNewColorTrayToCarTask == 53 && !fedTrayID)
        {
            TrayID[2][0]     = "ASECID001";
            bReadColorTrayID = true;
            fedTrayID        = true;
        }
        // case 420 on the NON-dummy path needs a real track sensor to confirm the
        // tray landed on the car (golden :243-245).
        if (iLoadNewColorTrayToCarTask == 420 && !flippedCar)
        {
            simSensorOn(SenColorCarHasTray);
            flippedCar = true;
        }
        done = DoLoadNewColorTrayToCar();
        seen.insert(iLoadNewColorTrayToCarTask);
        if (iLoadNewColorTrayToCarTask == 200 && selectAt200 < 0)
            selectAt200 = Cylinder[C_ColorLoaderZ_Select].bCylinderOn ? 1 : 0;
        if (iLoadNewColorTrayToCarTask == 400 && selectAt400 < 0)
            selectAt400 = Cylinder[C_ColorLoaderZ_Select].bCylinderOn ? 1 : 0;
    }

    static const int want[] = {50, 52, 53, 60, 100, 200, 300, 400, 410, 420};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(done, "DoLoadNewColorTrayToCar() converged on the non-DUMMY ASE path");
    CHECK(covers(seen, want, nWant), "trajectory covers 50,52,53,60,100,200,300,400,410,420");
    CHECK(selectAt200 == 1,
          "case 100 EXTENDED the Z-select cylinder before handing to 200 (golden :195 Push(), not Pop())");
    CHECK(selectAt400 == 0,
          "case 300 RETRACTED the Z-select cylinder before handing to 400 (golden :216 Pop(), not Push())");
    CHECK(TrayID[2][1] == "ASECID001" && TrayID[2][0] == "",
          "the ASE-read Color tray ID reached the pick slot (golden :253-254)");
    CHECK(W906_ShowErrorMessage_Count == 0, "no alarm on the successful ASE read path");
    printSeen("DoLoadNewColorTrayToCar/ASE", seen);
    if (!covers(seen, want, nWant)) printMissing("DoLoadNewColorTrayToCar/ASE", seen, want, nWant);
}

// ---------------------------------------------------------------------------
//  [3f] The WAR16335 tray-ID-read timeout arm -- and the GOLDEN BUG it contains.
//
//  golden :149 reads TrayID[2][0] (Color) but golden :172's NoTrayID fallback
//  WRITES TrayID[1][0] (Empty).  That is a cross-index defect in GOLDEN; it is
//  translated verbatim and PINNED here so the port cannot silently "fix" it.
//  PROBES: golden :172 `TrayID[1][0]` -> `TrayID[2][0]` -> the bug pin goes red;
//          golden :164 "WAR16335" -> "WAR16115" -> the code assertion goes red.
// ---------------------------------------------------------------------------
static void test_load_new_color_trayid_timeout_goldenbug()
{
    printf("\n[3f] DoLoadNewColorTrayToCar() WAR16335 timeout + golden TrayID[1] cross-index bug\n");

    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    LastSet.iRealDummy      = REALLY;
    IniConfig.bG11ASEReport = true;
    simSensorOn(SenColorSelectHasTray);
    simSensorOff(SenColorCCWDete);
    simSensorOff(SenColorCarHasTray);
    TrayID[1][0] = "EMPTYCID9";
    TrayID[2][0] = "";

    DoLoadNewColorTrayToCar();                     // case 1  -> 50
    DoLoadNewColorTrayToCar();                     // case 50 -> 52
    DoLoadNewColorTrayToCar();                     // case 52 -> 53 (arms the 7 s read deadline)
    CHECK(iColorTrayReadCIDTask >= 1 && iLoadNewColorTrayToCarTask == 53,
          "case 52 armed the ASE query and handed to 53");

    ColorReadTrayID.SetMSAndOn(0);                 // first timeout: iRead2D 0 -> 1, re-arm, NO alarm
    DoLoadNewColorTrayToCar();
    CHECK(W906_ShowErrorMessage_Count == 0 && iLoadNewColorTrayToCarTask == 53,
          "first read timeout only retries the query -- no alarm yet (golden :178-182)");

    W906_ShowErrorMessage_SimReturn = K_SKIP;      // force the NON-retry arm
    ColorReadTrayID.SetMSAndOn(0);                 // second timeout: iRead2D>=1 -> WAR16335
    DoLoadNewColorTrayToCar();
    CHECK(W906_ShowErrorMessage_LastCode == "WAR16335" &&
          W906_ShowErrorMessage_LastKCode == K_RETRY,
          "second read timeout raised WAR16335 offering only Retry (golden :164)");
    CHECK(iLoadNewColorTrayToCarTask == 60,
          "the non-Retry answer gave up on the tray ID and handed to 60 (golden :175)");
    CHECK(TrayID[1][0] == "NoTrayID",
          "GOLDEN BUG PINNED: the Color NoTrayID fallback writes TrayID[1][0] (EMPTY slot), golden :172");
    CHECK(TrayID[2][0] == "",
          "GOLDEN BUG PINNED: the Color slot TrayID[2][0] is left untouched by its own fallback");

    // ...and the Retry answer loops back to re-query.
    setupColorFixture();
    InitLoadNewColorTrayToCarTask();
    LastSet.iRealDummy      = REALLY;
    IniConfig.bG11ASEReport = true;
    simSensorOn(SenColorSelectHasTray);
    simSensorOff(SenColorCCWDete);
    simSensorOff(SenColorCarHasTray);
    DoLoadNewColorTrayToCar();  DoLoadNewColorTrayToCar();  DoLoadNewColorTrayToCar();
    ColorReadTrayID.SetMSAndOn(0);  DoLoadNewColorTrayToCar();
    ColorReadTrayID.SetMSAndOn(0);  DoLoadNewColorTrayToCar();     // SimReturn back to K_RETRY
    CHECK(iLoadNewColorTrayToCarTask == 52,
          "the Retry answer re-arms the ASE query (cursor back to 52, golden :168)");
}

// ---------------------------------------------------------------------------
//  [4] DoColorTrayToFront -- car-has-tray arm, with the case-1 fall-through pin.
//  PROBES: insert `break;` after golden :296 `Task=100;` -> fall-through pin red;
//          golden :311 `.Off()` deleted -> Fix-release red;
//          golden :363 pnlCoverTrayD Caption clear deleted -> caption red;
//          golden :359-360 iNeedManualRemoved transfer deleted -> transfer red.
// ---------------------------------------------------------------------------
static void test_tray_to_front_car_arm()
{
    printf("\n[4] DoColorTrayToFront() car-has-tray arm (1 -> 200 -> 300)\n");

    setupColorFixture();
    InitColorTrayToFrontTask();
    simSensorOn(SenColorCarHasTray);            // IsOff()==false -> golden :316 arm
    MOT[MMColor].SetTray(NULL_IC, "test_w7_l1_color");
    MOT[MMColor].Tray.iNeedManualRemoved     = 1;   // P24: tray needs manual removal
    MOT[MMColor_Car].Tray.iNeedManualRemoved = -1;
    Cylinder[C_Color_Fix].On();                 // pre-drive OPPOSITE of golden :311
    fSortCT->pnlCoverTrayD->Caption = "STALE-CID";

    bool done = false;
    int  afterFirstTick = -1, fixAfterFirstTick = -1;
    int  steps = 0;
    for (steps = 0; steps < 50 && !done; ++steps)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        done = DoColorTrayToFront();
        if (steps == 0)
        {
            afterFirstTick    = iColorTrayToFrontTask;
            fixAfterFirstTick = Cylinder[C_Color_Fix].bCylinderOn ? 1 : 0;
        }
    }

    CHECK(afterFirstTick == 200,
          "GOLDEN QUIRK pin: case 1 fell through into case 100 in ONE tick (cursor 1 -> 200)");
    CHECK(fixAfterFirstTick == 0,
          "case 100 RELEASED the Color fix cylinder (golden :311 Off(), not On())");
    CHECK(done, "DoColorTrayToFront() returns true (converged)");
    CHECK(MOT[MMColor_Car].fHasTray == true, "case 300 latched the tray onto MMColor_Car (golden :356)");
    CHECK(MOT[MMColor].fHasTray == false,    "case 300 cleared MMColor (golden :364)");
    CHECK(MOT[MMColor_Car].Tray.iNeedManualRemoved == 1 &&
          MOT[MMColor].Tray.iNeedManualRemoved == -1,
          "case 300 MOVED the P24 manual-removal flag Color -> Color_Car (golden :359-360)");
    CHECK(fSortCT->pnlCoverTrayD->Caption == "",
          "case 300 cleared the cover-tray-ID panel caption (golden :363)");
    CHECK(iColorTrayToFrontTask == 1, "cursor reset to 1 on convergence");
}

// ---------------------------------------------------------------------------
//  [4b] DoColorTrayToFront -- the DUMMY-timeout arm (golden :323-331), which is
//  only reachable once the belt-stopped re-arm at golden :304-309 is satisfied.
//  PROBES: M20 golden :304-305, BOTH `Status()==false` operands -> `==true`
//              -> the ->300 assertion red (the re-arm then fires every tick and
//              the 5 s deadline never expires).  NOTE: inverting only the FIRST
//              operand SURVIVES -- with both belts running the conjunction is
//              false either way.  Measured; the probe had to be widened.
//          M21 golden :329 `Task=300;` -> `Task=200;` -> the ->300 assertion red.
// ---------------------------------------------------------------------------
static void test_tray_to_front_dummy_timeout()
{
    printf("\n[4b] DoColorTrayToFront() DUMMY-timeout arm (park at 100, then -> 300)\n");

    setupColorFixture();
    InitColorTrayToFrontTask();
    LastSet.iRealDummy = DUMMY;
    simSensorOff(SenColorCarHasTray);           // IsOff()==true -> golden :316 arm NOT taken
    simSwitchWire(SwACColorCCW, 4);
    simSwitchWire(SwACColorCW,  5);
    SW[SwACColorCCW].On();                      // belts RUNNING -> golden :304 re-arm skipped
    SW[SwACColorCW].On();
    MOT[MMColor].SetTray(NULL_IC, "test_w7_l1_color");

    bool t1 = DoColorTrayToFront();
    CHECK(t1 == false && iColorTrayToFrontTask == 100,
          "cursor PARKED at 100 (car sensor off, belts running, no deadline expired)");

    hColorTrayToFrontForDummy.SetMSAndOn(0);    // progress timer, live only at cursor 100
    bool t2 = DoColorTrayToFront();
    CHECK(t2 == false && iColorTrayToFrontTask == 300,
          "the 5 s DUMMY deadline handed straight to 300, bypassing 200 (golden :329)");

    bool t3 = DoColorTrayToFront();
    CHECK(t3 == true && iColorTrayToFrontTask == 1,
          "case 300 then converged and reset the cursor");
    CHECK(W906_ShowErrorMessage_Count == 0, "the DUMMY-timeout path raises no JAM1412");

    SW[SwACColorCCW].Off();  SW[SwACColorCCW].Enable = false;
    SW[SwACColorCW].Off();   SW[SwACColorCW].Enable  = false;
}

// ---------------------------------------------------------------------------
//  [5] DoColorTrayToRear -- plain non-AMR path, with the case-1 fall-through pin.
//  PROBES: insert `break;` after golden :408 -> fall-through pin red;
//          golden :413 `.Pop()` -> `.Push()` -> fix-retract red;
//          golden :712 `.Push()` -> `.Pop()` -> fix-clamp red;
//          golden :660 floodgate `.Off()` deleted -> floodgate red;
//          golden :730/:731 SetTray/ClearTray swapped -> tray-handover red.
// ---------------------------------------------------------------------------
static void test_tray_to_rear_plain()
{
    printf("\n[5] DoColorTrayToRear() plain non-AMR path 200,450,480,500,510\n");

    setupColorFixture();
    InitColorTrayToRearTask();
    simSensorOn(SenColorHasTray);       // IsOff()==false  -> golden :477-478 arm -> 450
    simSensorOn(SenColorCWDete);
    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    Cylinder[C_Color_Fix].On();                 // pre-drive OPPOSITE of golden :413 Pop()
    Cylinder[C_ColorTrackFloodgate].On();       // pre-drive OPPOSITE of golden :660 Off()

    StateSet seen;
    bool done = false;
    int  afterFirstTick = -1, fixAfterFirstTick = -1, gateAt500 = -1;
    for (int i = 0; i < 100 && !done; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        done = DoColorTrayToRear();
        seen.insert(iColorTrayToRearTask);
        if (i == 0)
        {
            afterFirstTick    = iColorTrayToRearTask;
            fixAfterFirstTick = Cylinder[C_Color_Fix].bCylinderOn ? 1 : 0;
        }
        if (iColorTrayToRearTask == 500 && gateAt500 < 0)
            gateAt500 = Cylinder[C_ColorTrackFloodgate].bCylinderOn ? 1 : 0;
    }

    static const int want[] = {200, 450, 480, 500, 510};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(afterFirstTick == 200,
          "GOLDEN QUIRK pin: case 1 fell through into case 100 in ONE tick (cursor 1 -> 200)");
    CHECK(fixAfterFirstTick == 0,
          "case 100 RETRACTED the Color fix cylinder (golden :413 Pop(), not Push())");
    CHECK(done, "DoColorTrayToRear() returns true (converged)");
    CHECK(covers(seen, want, nWant), "trajectory covers 200,450,480,500,510");
    CHECK(gateAt500 == 0,
          "case 480 closed the Color track floodgate before handing on (golden :659-660)");
    CHECK(Cylinder[C_Color_Fix].bCylinderOn == true,
          "case 510 CLAMPED the tray with the fix cylinder (golden :712 Push(), not Pop())");
    CHECK(MOT[MMColor].fHasTray == true && MOT[MMColor_Car].fHasTray == false,
          "case 510 handed the tray Color_Car -> Color (golden :730-731)");
    CHECK(iReadCIDAction == ePortTotal, "the non-AMR path never takes the CID port lock");
    CHECK(iColorTrayToRearTask == 1, "cursor reset to 1 on convergence");
    printSeen("DoColorTrayToRear/plain", seen);
    if (!covers(seen, want, nWant)) printMissing("DoColorTrayToRear/plain", seen, want, nWant);
}

// ---------------------------------------------------------------------------
//  [5b] DoColorTrayToRear Keyence tray-ID hand-off 510 -> 520, and the
//  bEnableTrayID2==false exit at 520 (golden :759-766).
//
//  fTrayMapping->DoTrayIDKeyence2() is offline FALSE and there is no seam for it,
//  so cases 530/540 (and the ERROR / duplicate arms) are structurally unreachable
//  -- see NOT COVERED.  What IS observable is the four-way guard at golden
//  :716-719 and the else-if exit, and both are pinned.
//  PROBES: golden :719 `USE_KEYENCE_EMPTY==3` -> `==0` -> hand-off red;
//          golden :762-763 SetTray/ClearTray deleted -> exit-handover red.
// ---------------------------------------------------------------------------
static void test_tray_to_rear_keyence_handoff()
{
    printf("\n[5b] DoColorTrayToRear() Keyence tray-ID hand-off 510 -> 520 and its exit\n");

    setupColorFixture();
    InitColorTrayToRearTask();
    LastSet.iRealDummy         = REALLY;        // golden :722 -- no tray-ID read while dummy-running
    USE_TRAY_MAPPING           = 1;
    USE_KEYENCE_EMPTY          = 3;
    TestIF_File.bEnableTrayID2 = true;
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");

    bool done = false;
    int  steps = 0;
    for (steps = 0; steps < 60 && !done && iColorTrayToRearTask != 520; ++steps)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        done = DoColorTrayToRear();
    }

    CHECK(iColorTrayToRearTask == 520 && done == false,
          "case 510 took the Keyence branch and handed to 520 (golden :716-726)");
    CHECK(MOT[MMColor].fHasTray == false && MOT[MMColor_Car].fHasTray == true,
          "the Keyence branch has NOT yet handed the tray over (golden :730 is after the break)");

    // Offline DoTrayIDKeyence2 never answers, so 520 parks...
    for (int i = 0; i < 20; ++i) { w3::Tick(); /* AI(W906-W7-L1-Wave3) 20260802: lifter plant tick */ DoColorTrayToRear(); }
    CHECK(iColorTrayToRearTask == 520,
          "520 PARKS while DoTrayIDKeyence2() is offline-false (no fabricated progress)");

    // ...until the tray-ID-2 feature is switched off, which is golden's own exit.
    TestIF_File.bEnableTrayID2 = false;
    bool exited = DoColorTrayToRear();
    CHECK(exited == true && iColorTrayToRearTask == 1,
          "520's bEnableTrayID2==false exit converged (golden :759-765)");
    CHECK(MOT[MMColor].fHasTray == true && MOT[MMColor_Car].fHasTray == false,
          "the 520 exit handed the tray Color_Car -> Color (golden :762-763)");
    CHECK(bEmptyReadIDOK == true, "the 520 exit latched bEmptyReadIDOK (golden :761)");
}

// ---------------------------------------------------------------------------
//  [6] DoColorTrayToRear ATK-AMR RFID path + DoColorTrayReadCID, BOTH answers.
//
//  fAGV->IsATK_AMR() is REAL translated code (Automation/AGV_predicates.cpp):
//  USE_COVER_TRAYID==tCID_NFC && IniConfig.bA65_BundleIDList && CUSTOMER_CODE==
//  CC_AMKOR_Korea.  Setting those three globals turns the whole AMR branch on --
//  it is not stubbed.
//  PROBES: M29 golden :580 `iReadCIDAction=ePortColor;` deleted -> the
//              lock-at-455 assertion red (the END-of-run form of that assertion
//              SURVIVED, because DoColorTrayReadCID :1499 writes the same value;
//              that is why g_lockAt455 exists);
//          M30 golden :631 `iColorIdTrayCt++;` deleted -> counter + escalation red;
//          M31 golden :649 `Checked=false` deleted -> case-470 assertion red;
//          M32 golden :628-629 port release deleted -> lock-release red;
//          M33 golden :630 fix-cylinder release deleted -> release red (needed the
//              reArmFixCylinder pre-drive: without it this probe SURVIVED);
//          M34 golden :623-624 port publish deleted -> publish red;
//          M36 golden :1536 length gate 9 -> 8 -> the whole verdict chain red.
// ---------------------------------------------------------------------------
static void enableAtkAmr()
{
    USE_COVER_TRAYID            = tCID_NFC;
    IniConfig.bA65_BundleIDList = true;
    CUSTOMER_CODE               = CC_AMKOR_Korea;
}

//  Step the AMR rear-flow until it parks somewhere interesting, feeding the RFID
//  reader the answers a real reader would give.  sBlockID picks the verdict:
//  a 9-char block ID is golden's "good read" (golden :1536), anything else is a
//  bad read.
//  g_lockAt455 -- iReadCIDAction sampled the FIRST time the cursor rests on 455,
//  i.e. after case 450 has run but BEFORE DoColorTrayReadCID has ever executed.
//  This distinction is load-bearing: DoColorTrayReadCID case 1 ALSO writes
//  iReadCIDAction=ePortColor (golden :1499), so reading the flag at the END of the
//  run cannot tell whether golden :580 ran at all (measured -- deleting :580 left
//  the end-of-run assertion green).
static int g_lockAt455 = -99;
//  reArmFixCylinder -- re-drive Cylinder[C_Color_Fix] ON each time the cursor
//  rests on 460, so the read-fail arm's release at golden :630 is INDEPENDENTLY
//  observable.  Without it, case 100's Pop() has already left the cylinder
//  retracted and deleting :630 changes nothing (measured).
//  stopOnOperatorMessage -- leave as soon as the SM has escalated to the case-470
//  operator dialog, otherwise the read-fail loop keeps re-feeding the tray and
//  iColorIdTrayCt runs away past the value under test.
static void driveAmrRear(StateSet &seen, const char *sBlockID, int maxSteps,
                         bool stopOnOperatorMessage = false,
                         bool reArmFixCylinder = false)
{
    g_lockAt455 = -99;
    for (int i = 0; i < maxSteps; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iColorTrayToRearTask == 455)
            fTrayMapping->ldRFID->bClearCmd2 = true;       // reader acked the clear
        if (iColorTrayToRearTask == 460)
        {
            hColorTrayToRear.SetMSAndOn(0);                // arrival settle, live at 460
            tNFCColorDelay.SetMSAndOn(0);                  // DoColorTrayReadCID progress
            fTrayMapping->ldRFID->bReadUID2 = true;        // reader answered the UID
            if (reArmFixCylinder) Cylinder[C_Color_Fix].On();
            if (iColorTrayReadCIDTask == 2000)
                fTrayMapping->ldRFID->sBlockID2 = sBlockID;
        }
        if (DoColorTrayToRear()) break;
        seen.insert(iColorTrayToRearTask);
        if (iColorTrayToRearTask == 455 && g_lockAt455 == -99)
            g_lockAt455 = iReadCIDAction;
        if (stopOnOperatorMessage && W906_ShowMyMessage_Count > 0) break;
    }
}

static void test_tray_to_rear_amr_rfid()
{
    printf("\n[6] DoColorTrayToRear() ATK-AMR RFID path + DoColorTrayReadCID\n");

    // ---- 6a: a GOOD 9-character block ID -> ret==1 -> 480,500,600 ------------
    setupColorFixture();
    enableAtkAmr();
    InitColorTrayToRearTask();
    LastSet.iRealDummy = REALLY;                // golden :569 -- the non-dummy RFID arm
    IniConfig.bEnable_SECS_GEM = true;
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    CHECK(fAGV->IsATK_AMR() == true,
          "fixture precondition: fAGV->IsATK_AMR() is TRUE (real predicate, not a stub)");

    StateSet seenA;
    driveAmrRear(seenA, "123456789", 200);

    static const int wantA[] = {200, 450, 455, 460, 480, 500, 600};
    const int nWantA = (int)(sizeof(wantA)/sizeof(wantA[0]));
    CHECK(covers(seenA, wantA, nWantA), "trajectory covers 200,450,455,460,480,500,600");
    CHECK(iColorTrayToRearTask == 600,
          "600 PARKS while fTrayMapping->DoCoverTrayID() is offline-false (no fabricated progress)");
    CHECK(g_lockAt455 == ePortColor,
          "case 450 TOOK the CID port lock for Color before starting the read (golden :580)");
    CHECK(iReadCIDAction == ePortColor,
          "the AMR path still holds the CID port lock while parked at 600");
    CHECK(W906_ShowErrorMessage_Count == 0, "no alarm on the successful RFID read");
    printSeen("DoColorTrayToRear/AMR-ok", seenA);
    if (!covers(seenA, wantA, nWantA)) printMissing("DoColorTrayToRear/AMR-ok", seenA, wantA, nWantA);

    // ---- 6b: a SHORT block ID -> ret==-1 -> the read-fail arm -> 465 ---------
    setupColorFixture();
    enableAtkAmr();
    InitColorTrayToRearTask();
    LastSet.iRealDummy = REALLY;
    IniConfig.bEnable_SECS_GEM = true;
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    iThisPortNo = -1;

    StateSet seenB;
    // reArmFixCylinder=true: case 100's Pop() has already retracted the fix
    // cylinder long before the read fails, so re-drive it EXTENDED at every 460
    // tick -- otherwise golden :630's release is invisible.
    driveAmrRear(seenB, "1234", 200, false, true);

    CHECK(iColorTrayToRearTask == 465,
          "a short block ID -> DoColorTrayReadCID()==-1 -> the read-fail arm handed to 465 (golden :635)");
    CHECK(iColorIdTrayCt == 1, "the read-fail arm bumped iColorIdTrayCt to 1 (golden :631)");
    CHECK(iThisPortNo == ePortColor && iThisPortStatus == iPortStatus[ePortColor],
          "the read-fail arm published the Color port + its status (golden :623-624)");
    CHECK(iReadCIDAction == ePortTotal, "the read-fail arm RELEASED the CID port lock (golden :628-629)");
    CHECK(Cylinder[C_Color_Fix].bCylinderOn == false,
          "the read-fail arm released the fix cylinder so the tray can be re-fed (golden :630)");

    // 465 pushes the tray back out and only returns to 1 once BOTH car sensors see it.
    DoColorTrayToRear();
    CHECK(iColorTrayToRearTask == 465, "465 waits while the car sensors have not seen the tray");
    simSensorOn(SenColorCCWDete);
    simSensorOn(SenColorCarHasTray);
    DoColorTrayToRear();
    CHECK(iColorTrayToRearTask == 1, "465 returned to 1 once both car sensors read On (golden :641-646)");

    // ---- 6c: the THIRD consecutive failure escalates to case 470 -------------
    setupColorFixture();
    enableAtkAmr();
    InitColorTrayToRearTask();
    LastSet.iRealDummy = REALLY;
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    MOT[MMColor].ClearTray("test_w7_l1_color");
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    fMain->chkE84IDTray->Checked = true;
    iColorIdTrayCt = 2;                         // fixture pre-set: two failures already logged
    W906_ShowMyMessage_Reset();

    StateSet seenC;
    driveAmrRear(seenC, "1234", 200, true);     // stop the moment case 470 speaks

    CHECK(seenC.count(470) == 1,
          "the third read failure routed to case 470 rather than 465 (golden :632-633)");
    CHECK(iColorIdTrayCt == 3 && iColorTrayToRearTask == 1,
          "case 470 ran on exactly the third failure and reset the cursor (golden :631, :651)");
    CHECK(fMain->chkE84IDTray->Checked == false,
          "case 470 cleared fMain->chkE84IDTray (golden :649)");
    CHECK(W906_ShowMyMessage_LastS1 == "Read ID tray fail!!",
          "case 470 told the operator the tray ID could not be read (golden :650)");
}

// ---------------------------------------------------------------------------
//  [6d] DoColorTrayReadCID as a UNIT -- the LIVE fall-through and both verdicts.
//
//  THIS FUNCTION HAS TWO FALL-THROUGHS AND ONLY ONE OF THEM IS BEHAVIOURALLY
//  LIVE.  Measured, not assumed:
//    * case 1 -> case 10 (golden :1501, no break): LIVE.  Case 10's body arms the
//      1 s deadline and issues the read-UID click in the SAME tick, so inserting a
//      `break;` after golden :1501 leaves the cursor on 10 instead of 100 and the
//      assertion below goes red (mutation M67).
//    * case 10 -> case 100 (golden :1509, where the `break;` is COMMENTED OUT):
//      INERT.  Case 10 always leaves bReadUID2 false (it clears it at golden
//      :1505) and always arms tNFCColorDelay to a fresh 1 s (golden :1507), so
//      case 100's two guards are BOTH false in that same tick no matter what.
//      RESTORING golden :1509's break left this suite at 163 passed, 0 failed
//      (mutation M35) -- so this file does NOT pin it and does not claim to.  It
//      is preserved verbatim in the translation regardless.
//  PROBES: M67 break after golden :1501 -> the one-tick pin red;
//          M36 golden :1536 `Length()!=9` -> `!=8` -> both verdict assertions red.
// ---------------------------------------------------------------------------
static void test_read_cid_unit()
{
    printf("\n[6d] DoColorTrayReadCID() unit -- fall-throughs and both verdicts\n");

    setupColorFixture();
    InitColorTrayReadCIDTask();
    iReadCIDAction = ePortTotal;

    int r1 = DoColorTrayReadCID();
    CHECK(r1 == 0 && iColorTrayReadCIDTask == 100,
          "GOLDEN QUIRK pin: case 1 fell through into case 10 in ONE tick (golden :1501)");
    CHECK(iReadCIDAction == ePortColor, "case 1 took the CID port lock for Color (golden :1499)");
    CHECK(tNFCColorDelay.Off() == false, "case 10 armed the 1 s UID-read deadline (golden :1507)");

    fTrayMapping->ldRFID->bReadUID2 = true;
    int r2 = DoColorTrayReadCID();
    CHECK(r2 == 0 && iColorTrayReadCIDTask == 1000,
          "case 100 saw the UID answer and handed to 1000 (golden :1511-1514)");

    tNFCColorDelay.SetMSAndOn(0);
    int r3 = DoColorTrayReadCID();
    CHECK(r3 == 0 && iColorTrayReadCIDTask == 2000,
          "case 1000 issued the read-block command and handed to 2000 (golden :1526-1530)");

    fTrayMapping->ldRFID->sBlockID2 = "123456789";      // exactly 9 -> good
    CHECK(DoColorTrayReadCID() == 1,
          "case 2000 with a 9-character block ID answers +1 (golden :1536-1539)");

    fTrayMapping->ldRFID->sBlockID2 = "12345678";       // 8 -> bad
    CHECK(DoColorTrayReadCID() == -1,
          "case 2000 with an 8-character block ID answers -1 (golden :1537)");

    // The Loader owning the CID port blocks case 1 outright (golden :1496-1497).
    InitColorTrayReadCIDTask();
    iReadCIDAction = ePortLoader;
    CHECK(DoColorTrayReadCID() == 0 && iColorTrayReadCIDTask == 1,
          "case 1 refuses to start while the LOADER holds the CID port (golden :1496)");
    CHECK(iReadCIDAction == ePortLoader, "...and does not steal the lock");
    iReadCIDAction = ePortTotal;
}

// ---------------------------------------------------------------------------
//  [7] DoUnLoadNewColorToStack -- the DUMMY unload walk, plus the case-1
//  recirculation and the case-10 double-tray arm.
//  PROBES: M68 golden :1185 SetTray deleted -> the case-1 Z-stage-set red;
//          M38 golden :1226 SetTray deleted -> the case-50 Z-stage-set red (this
//              probe SURVIVED until the mid-trajectory wipe below was added --
//              golden writes the same flag twice and the second write was masked);
//          M39 golden :1230-1231 floodgate close deleted -> floodgate red;
//          M40 golden :1250 CylinderUp -> CylinderLower -> lifter-up red;
//          M41 golden :1266 ClearTray deleted -> Z-stage-clear red;
//          M42 golden :1282 CylinderLower -> CylinderUp -> lifter-down red;
//          M43 golden :1195-1196 double-tray guard IsOn -> IsOff -> case-10 red.
// ---------------------------------------------------------------------------
static void test_unload_to_stack()
{
    printf("\n[7] DoUnLoadNewColorToStack() DUMMY unload walk 50,100,150,200,300,400,500\n");

    setupColorFixture();
    InitUnLoadNewColorTrayTask();
    LastSet.iRealDummy = DUMMY;                 // golden :1178-1179 guard falls through
    MOT[MMColorZ].ClearTray("test_w7_l1_color");
    Cylinder[C_Color_Up].Off();                 // pre-drive OPPOSITE of golden :1250 CylinderUp
    Cylinder[C_Color_Middle].Off();
    Cylinder[C_ColorTrackFloodgate].On();       // pre-drive OPPOSITE of golden :1231 Off()

    StateSet seen;
    bool done = false;
    int  zAt50 = -1, zAt100 = -1, upAt300 = -1, gateAt100 = -1;
    bool wipedForCase50 = false;
    for (int i = 0; i < 200 && !done; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iUnLoadNewColorTrayTask == 150 || iUnLoadNewColorTrayTask == 500 ||
            iUnLoadNewColorTrayTask == 20)
            DoUnLoadNewColorToStackDelay.SetMSAndOn(0);
        // Case 1 and case 50 BOTH mark the Z stage (golden :1185 and :1226).  Sample
        // after case 1, then WIPE the flag before case 50 runs, so each SetTray is
        // INDEPENDENTLY load-bearing -- without the wipe, deleting golden :1226
        // alone leaves the flag already set by :1185 and the suite stays green
        // (measured: mutation M38 survived before this was added).
        if (iUnLoadNewColorTrayTask == 50 && !wipedForCase50)
        {
            zAt50 = MOT[MMColorZ].fHasTray ? 1 : 0;
            MOT[MMColorZ].ClearTray("test_w7_l1_color");
            wipedForCase50 = true;
        }
        done = DoUnLoadNewColorToStack();
        seen.insert(iUnLoadNewColorTrayTask);
        if (iUnLoadNewColorTrayTask == 100 && zAt100 < 0)
        {
            zAt100    = MOT[MMColorZ].fHasTray ? 1 : 0;
            gateAt100 = Cylinder[C_ColorTrackFloodgate].bCylinderOn ? 1 : 0;
        }
        if (iUnLoadNewColorTrayTask == 300 && upAt300 < 0)
            upAt300 = Cylinder[C_Color_Up].bCylinderOn ? 1 : 0;
    }

    static const int want[] = {50, 100, 150, 200, 300, 400, 500};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(done, "DoUnLoadNewColorToStack() returns true (converged)");
    CHECK(covers(seen, want, nWant), "trajectory covers 50,100,150,200,300,400,500");
    CHECK(zAt50 == 1, "case 1 marked the Z stage as carrying the tray (golden :1185 SetTray)");
    CHECK(zAt100 == 1, "case 50 RE-marked the wiped Z stage flag (golden :1226 SetTray, independent of :1185)");
    CHECK(gateAt100 == 0, "case 50 closed the Color track floodgate (golden :1230-1231)");
    CHECK(upAt300 == 1, "case 200 RAISED the lifter (golden :1250 CylinderUp, not Lower)");
    CHECK(MOT[MMColorZ].fHasTray == false, "case 400 cleared the Z stage tray flag (golden :1266)");
    CHECK(Cylinder[C_Color_Up].bCylinderOn == false &&
          Cylinder[C_Color_Middle].bCylinderOn == false,
          "case 500 LOWERED the lifter pair before converging (golden :1282)");
    CHECK(iUnLoadNewColorTrayTask == 1, "cursor reset to 1 on convergence");
    printSeen("DoUnLoadNewColorToStack", seen);
    if (!covers(seen, want, nWant)) printMissing("DoUnLoadNewColorToStack", seen, want, nWant);

    // --- case-1 -> 10 recirculation (non-DUMMY, TrayMoveStatus()==0) ----------
    setupColorFixture();
    InitUnLoadNewColorTrayTask();
    LastSet.iRealDummy = REALLY;
    StateSet seenR;
    for (int i = 0; i < 12; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iUnLoadNewColorTrayTask == 20) DoUnLoadNewColorToStackDelay.SetMSAndOn(0);
        DoUnLoadNewColorToStack();
        seenR.insert(iUnLoadNewColorTrayTask);
    }
    CHECK(seenR.count(10) && seenR.count(20) && seenR.count(1) && seenR.count(50) == 0,
          "non-DUMMY case 1 recirculates 1->10->20->1 and NEVER reaches 50 (golden :1177-1183)");
    printSeen("DoUnLoadNewColorToStack/recirc", seenR);

    // --- case-10 double-tray arm: both track sensors On -> straight to 50 -----
    setupColorFixture();
    InitUnLoadNewColorTrayTask();
    LastSet.iRealDummy = REALLY;
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    DoUnLoadNewColorToStack();                  // 1 -> 10
    DoUnLoadNewColorToStack();                  // 10 -> 50 (double-tray protection)
    CHECK(iUnLoadNewColorTrayTask == 50,
          "case 10 with BOTH track sensors On jumps to 50 (double-tray protection, golden :1195-1199)");
}

// ---------------------------------------------------------------------------
//  [8] DoAutoColorReceive -- the full receive chain, the bP04 case-50 arm, the
//  P24 manual-removal park/release pair, and the pre-alarm branch.
//  PROBES: golden :1332 `iReceiveColorTray=2;` deleted -> chain assertion red;
//          golden :1397 `iUnLoaderCount=8;` deleted -> pre-alarm red;
//          golden :1400 `iPreAlarmPosition=9` -> `=8` -> position red
//              (9 == Color; 8 is the EMPTY track -- a one-character slip);
//          golden :1431 `Task=300;` -> `Task=1;` -> the P24 park assertion red.
// ---------------------------------------------------------------------------
static void test_receive_chain()
{
    printf("\n[8] DoAutoColorReceive() chain / bP04 arm / P24 park / pre-alarm\n");

    // ---- 8a: the full chain converges ---------------------------------------
    setupColorFixture();
    InitAutoColorReceiveTask();
    simSensorOn(SenColorCarHasTray);            // DoColorTrayToFront's car arm
    MOT[MMColor].SetTray(NULL_IC, "test_w7_l1_color");

    // NOTE the loop shape: the cursor STARTS at 1, so a `while (cursor != 1)` loop
    // would never execute and every assertion after it would pass vacuously.  Run
    // until the cursor has LEFT 1 and come back.
    bool sawUnloadStart = false, sawReceive200 = false;
    bool leftOne = false, backToOne = false;
    for (int i = 0; i < 600 && !backToOne; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iUnLoadNewColorTrayTask == 150 || iUnLoadNewColorTrayTask == 500 ||
            iUnLoadNewColorTrayTask == 20)
            DoUnLoadNewColorToStackDelay.SetMSAndOn(0);
        DoAutoColorReceive();
        if (bUnLoadNewColorToStackStart)    sawUnloadStart = true;
        if (iAutoColorReceiveTask == 200)   sawReceive200  = true;
        if (iAutoColorReceiveTask != 1)     leftOne = true;
        else if (leftOne)                   backToOne = true;
    }
    CHECK(backToOne, "the receive chain LEFT cursor 1 and converged back to it");
    CHECK(sawReceive200,
          "the chain went through case 200 -- i.e. DoColorTrayToFront left a tray on the car (golden :1333-1336)");
    CHECK(sawUnloadStart, "case 200 raised bUnLoadNewColorToStackStart while unloading (golden :1366)");
    CHECK(bUnLoadNewColorToStackStart == false, "and cleared it on completion (golden :1369)");
    CHECK(iReceiveColorTray == 0, "case 300 released iReceiveColorTray (golden :1446)");
    CHECK(MOT[MMColor_Car].fHasTray == false, "case 200 cleared MMColor_Car (golden :1379)");

    // ---- 8b: the bP04ColorIsEmptyUnloader case-50 arm ------------------------
    setupColorFixture();
    InitAutoColorReceiveTask();
    IniConfig.bP04ColorIsEmptyUnloader = true;
    MOT[MOutArmX].fCanMove = false;
    MOT[MOutArmY].fCanMove = false;
    DoAutoColorReceive();
    CHECK(iAutoColorReceiveTask == 50, "bP04 routes case 1 to the case-50 wait (golden :1310-1313)");
    DoAutoColorReceive();
    CHECK(MOT[MOutArmX].fCanMove == true && MOT[MOutArmY].fCanMove == true,
          "case 50 released BOTH out-arm axes once TrayX cleared the Empty column (golden :1323-1324)");
    CHECK(iAutoColorReceiveTask == 100, "case 50 then handed to 100 (golden :1326)");

    // ---- 8c: the case-300 pre-alarm branch, then the case-200 MES1420 arm ----
    setupColorFixture();
    CosFunction.bUseEmptyColorTrayPreAlarm = true;
    IniConfig.bP32EmptyColorTrayPreAlarm   = true;
    simSensorOn(SnColorIsFull);
    RunInfo.iPreAlarmPosition = 0;
    iUnLoaderCount            = 0;
    iAutoColorReceiveTask     = 300;            // park the cursor on the branch under test
    iReceiveColorTray         = 2;
    DoAutoColorReceive();
    CHECK(iUnLoaderCount == 8,
          "case-300 pre-alarm kept the Handler running (iUnLoaderCount=8, golden :1397)");
    CHECK(RunInfo.iPreAlarmPosition == 9,
          "case-300 pre-alarm reported position 9 == COLOR, not 8 == Empty (golden :1400)");
    CHECK(iReceiveColorTray == 0 && iAutoColorReceiveTask == 1,
          "case-300 pre-alarm still released the receive flag and reset the cursor (golden :1408-1409)");

    // The pre-alarm latch is what makes the case-200 MES1420 arm reachable.
    W906_ShowErrorMessage_Reset();
    iAutoColorReceiveTask = 200;
    DoAutoColorReceive();
    CHECK(W906_ShowErrorMessage_LastCode == "MES1420" &&
          W906_ShowErrorMessage_LastKCode == K_RETRY,
          "case 200 refuses to lift into a FULL stack once pre-alarm latched (MES1420, golden :1357)");
    CHECK(iAutoColorReceiveTask == 200, "...and holds the cursor on 200 (golden :1358 break)");

    // ---- 8d: the P24 manual-removal park, then its release -------------------
    setupColorFixture();
    W906_ShowMyMessage_Reset();
    simSensorOn(SenColorSelectHasTray);
    MOT[MMColor_Car].Tray.iNeedManualRemoved = 1;
    iAutoColorReceiveTask = 200;
    iReceiveColorTray     = 2;
    for (int i = 0; i < 200 && iAutoColorReceiveTask != 300; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iUnLoadNewColorTrayTask == 150 || iUnLoadNewColorTrayTask == 500 ||
            iUnLoadNewColorTrayTask == 20)
            DoUnLoadNewColorToStackDelay.SetMSAndOn(0);
        DoAutoColorReceive();
    }
    CHECK(iAutoColorReceiveTask == 300, "the unload completed and handed to 300");
    CHECK(MOT[MMColor_Car].Tray.iNeedManualRemoved == -1,
          "case 200 consumed the P24 manual-removal flag (golden :1372)");
    DoAutoColorReceive();
    CHECK(iAutoColorReceiveTask == 300 &&
          W906_ShowMyMessage_LastS1 == "Please remove color tray manually. And check if has IC inside the tray.",
          "case 300 PARKS on 300 asking the operator to remove the tray (golden :1428-1432)");
    CHECK(iReceiveColorTray == 2, "...and does NOT release the receive flag while parked");
    simSensorOff(SenColorSelectHasTray);        // operator took the tray off
    DoAutoColorReceive();
    CHECK(iAutoColorReceiveTask == 1 && iReceiveColorTray == 0,
          "case 300 released once the select sensor read Off (golden :1434-1439)");
}

// ---------------------------------------------------------------------------
//  [9] DoAutoColor -- the top dispatcher, five scenarios.
//  PROBES: golden :891 `Task=20;` -> `Task=999;` -> scenario B trajectory red;
//          golden :1000-1001 receive hand-off deleted -> scenario D red;
//          golden :1122-1123 `InitColorTrayToRearTask(); Task=300;` -> `Task=1;`
//              -> scenario E's 300 -> 1 one-tick edge red;
//          golden :1113-1115 buffer arm deleted -> scenario C red.
// ---------------------------------------------------------------------------
static void test_drive_auto_color()
{
    printf("\n[9] DoAutoColor() dispatcher scenarios\n");

    // ---- A: empty track -> case 100 -> the feed hand-off at 200 -------------
    setupColorFixture();
    InitAutoColorTask();
    InitLoadNewColorTrayToCarTask();
    simSensorOff(SenColorHasTray);
    DoAutoColor();
    CHECK(iAutoColorTask == 100, "A: an empty Color track hands case 1 straight to 100 (golden :899-900)");
    DoAutoColor();
    CHECK(iAutoColorTask == 200 && bLoadNewColorTrayToCarStart == false,
          "A: case 100 with an empty car selects the FEED arm 200 (golden :1128-1129)");

    // ---- B: an occupied track walks the fix-cylinder release chain ----------
    setupColorFixture();
    InitAutoColorTask();
    LastSet.iRealDummy = HAS_TRAY;              // so case 65's re-detect blocks are LIVE
    MOT[MMColor].SetTray(NULL_IC, "test_w7_l1_color");
    simSensorOff(SenColorHasTray);
    simSensorOff(SenColorCWDete);
    Cylinder[C_Color_Fix].On();                 // pre-drive OPPOSITE of golden :915 Off()
    StateSet seenB;
    int fixAt30 = -1;
    bool sawStaleClear = false;
    for (int i = 0; i < 60; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iAutoColorTask == 65) CheckColorTrayDelay.SetMSAndOn(0);
        if (iAutoColorTask == 70 || iAutoColorTask == 150) DoAutoColorDelay.SetMSAndOn(0);
        DoAutoColor();
        seenB.insert(iAutoColorTask);
        if (iAutoColorTask == 30 && fixAt30 < 0)
            fixAt30 = Cylinder[C_Color_Fix].bCylinderOn ? 1 : 0;
        if (iAutoColorTask == 60 && MOT[MMColor].fHasTray == false) sawStaleClear = true;
    }
    static const int wantB[] = {20, 30, 40, 50, 60, 65};
    const int nWantB = (int)(sizeof(wantB)/sizeof(wantB[0]));
    CHECK(covers(seenB, wantB, nWantB), "B: trajectory covers 20,30,40,50,60,65");
    CHECK(fixAt30 == 0, "B: case 20 RELEASED the fix cylinder before handing to 30 (golden :915)");
    CHECK(sawStaleClear,
          "B: case 65 cleared the STALE MMColor tray flag when both track sensors read Off (golden :969-976)");
    printSeen("DoAutoColor B", seenB);

    // ---- C: case 65's re-detect + receive hand-off --------------------------
    setupColorFixture();
    InitAutoColorTask();
    InitAutoColorReceiveTask();
    LastSet.iRealDummy = HAS_TRAY;
    MOT[MMColor].SetTray(NULL_IC, "test_w7_l1_color");
    MOT[MMColor_Car].ClearTray("test_w7_l1_color");
    simSensorOn(SenColorHasTray);               // IsOff()==false -> re-detect arms fire
    simSensorOn(SenColorCWDete);
    simSensorOn(SenColorCarHasTray);
    simSensorOn(SenColorCCWDete);
    for (int i = 0; i < 40 && iAutoColorReceiveTask != 200; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        if (iAutoColorTask == 65) CheckColorTrayDelay.SetMSAndOn(0);
        DoAutoColor();
    }
    CHECK(MOT[MMColor_Car].fHasTray == true,
          "C: case 65 RE-DETECTED a tray on the Color car from its two sensors (golden :987-993)");
    CHECK(iReceiveColorTray == 2 && iAutoColorReceiveTask == 200,
          "C: the double-tray condition fired the receive hand-off (golden :1004-1009)");

    // ---- D: the buffer-place arm of case 100 --------------------------------
    setupColorFixture();
    InitAutoColorTask();
    InitAutoColorReceiveTask();
    iAutoColorTask     = 100;
    bIsPlacingToBuffer = true;
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    MOT[MTrayX].SetTray(NULL_IC, "test_w7_l1_color");
    Prod.iXTrayColor = MOT[MTrayX].ReadPos();   // TrayArm parked over the Color column
    iAutoColorReceiveTask = 999;
    DoAutoColor();
    CHECK(fColorCanSupplyNewTray == false && iReceiveColorTray == 1,
          "D: case 100 with the TrayArm at the Color column closed the supply gate (golden :1113-1114)");
    CHECK(iAutoColorReceiveTask == 1, "D: ...and re-initialised the receive SM (golden :1115)");
    CHECK(iAutoColorTask == 150, "D: ...and parked on the 1 s settle at 150 (golden :1117-1118)");

    // ---- E: the recycle arm of case 100, and its one-tick 300 -> 1 edge ------
    setupColorFixture();
    InitAutoColorTask();
    iAutoColorTask = 100;
    MOT[MMColor_Car].SetTray(NULL_IC, "test_w7_l1_color");
    MOT[MTrayX].ClearTray("test_w7_l1_color");
    simSensorOn(SenColorHasTray);
    simSensorOn(SenColorCWDete);
    DoAutoColor();
    CHECK(iAutoColorTask == 300, "E: case 100 with no TrayArm demand selects the RECYCLE arm 300 (golden :1122-1123)");
    bool saw300to1 = false;
    for (int i = 0; i < 100; ++i)
    {
        w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: lifter plant tick
        int prev = iAutoColorTask;
        DoAutoColor();
        if (prev == 300 && iAutoColorTask == 1) saw300to1 = true;
        if (saw300to1) break;
    }
    CHECK(saw300to1,
          "E: case 300 consumed DoColorTrayToRear()==true and stepped 300 -> 1 in ONE tick (golden :1155-1157)");
}

// ---------------------------------------------------------------------------
//  [10] The four early-return gates + the StringGrid2 supply-flag log line.
//  PROBES: golden :845-846 gate deleted -> gate assertion red;
//          golden :831 StringGrid2 write deleted -> log assertion red;
//          golden :836 `CC_SCC` -> `0` -> the SCC guard assertion red.
// ---------------------------------------------------------------------------
static void test_gates_and_log()
{
    printf("\n[10] DoAutoColor() early-return gates + StringGrid2 supply log\n");

    // supply gate closed
    setupColorFixture();
    InitAutoColorTask();
    fColorCanSupplyNewTray = false;
    int before = iAutoColorTask;
    for (int i = 0; i < 20; ++i) { w3::Tick(); /* AI(W906-W7-L1-Wave3) 20260802: lifter plant tick */ DoAutoColor(); }
    CHECK(iAutoColorTask == before,
          "DoAutoColor no-ops while fColorCanSupplyNewTray==false (golden :845-846)");

    // the supply-flag transition is logged into fMain->StringGrid2 -- the ONLY
    // StringGrid2 use in the whole asendic_* family (golden :831).
    fMain->StringGrid2->Cells[3][38] = "sentinel";
    fColorCanSupplyNewTray = true;
    DoAutoColor();
    CHECK(fMain->StringGrid2->Cells[3][38] != "sentinel",
          "the fColorCanSupplyNewTray transition was logged to StringGrid2 Cells[3][38] (golden :831)");
    CHECK(fMain->StringGrid2->Cells[3][38] == BoolToStr(true),
          "...with BoolToStr of the NEW value (golden :830-831)");

    // bP04ColorIsEmptyUnloader gate
    setupColorFixture();
    InitAutoColorTask();
    IniConfig.bP04ColorIsEmptyUnloader = true;
    before = iAutoColorTask;
    for (int i = 0; i < 10; ++i) { w3::Tick(); /* AI(W906-W7-L1-Wave3) 20260802: lifter plant tick */ DoAutoColor(); }
    CHECK(iAutoColorTask == before, "DoAutoColor no-ops while bP04ColorIsEmptyUnloader (golden :848-849)");

    // the P24/P25 no-supply gate
    setupColorFixture();
    InitAutoColorTask();
    CosFunction.bSpecialP24 = true;
    IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray = true;
    IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty    = true;
    before = iAutoColorTask;
    for (int i = 0; i < 10; ++i) { w3::Tick(); /* AI(W906-W7-L1-Wave3) 20260802: lifter plant tick */ DoAutoColor(); }
    CHECK(iAutoColorTask == before, "DoAutoColor no-ops under P24+P25 no-supply (golden :853-857)");

    // the JSCC buffer-catch guard: receive mode + CC_SCC + catching -> re-init only
    setupColorFixture();
    InitAutoColorTask();
    iReceiveColorTray     = 2;
    CUSTOMER_CODE         = CC_SCC;
    bIsCatchingFromBuffer = true;
    iAutoColorReceiveTask = 777;
    DoAutoColor();
    CHECK(iAutoColorReceiveTask == 1,
          "JSCC guard re-initialises the receive SM instead of running it (golden :836-840)");
}

// ---------------------------------------------------------------------------
//  [11] ForTERAPOWERCheckColorSensor -- Wave 2's cross-file dependency.
//  asendic_Auto.cpp calls this at golden :363 / :2029 / :2074, so its polarity is
//  about to become load-bearing for a file that does not exist yet.  All FOUR
//  combinations are pinned, which is what makes the IsOn()/IsOff() spelling and
//  the RT/FT row selection non-normalisable.
//  PROBES: golden :1471 `IsOn()` -> `IsOff()` -> two assertions red;
//          golden :1462 `irsm=RT;` -> `irsm=FT;` -> the row-selection pair red.
// ---------------------------------------------------------------------------
static void test_terapower_color_sensor()
{
    printf("\n[11] ForTERAPOWERCheckColorSensor() -- all four arms + RT/FT row select\n");
    setupColorFixture();

    const int iPos = 0;
    const int sen  = SnAutoColorTrayDete[iPos];

    LastSet.iRunStartMode = 0;                          // not a retest -> irsm == FT
    TrayForm.AutoFromEmptyColor[FT][iPos] = 0;          // Tray From Empty  -> read IsOn()
    simSensorOn(sen);
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == true,
          "From-Empty + sensor On  -> true  (golden :1471 IsOn(), not IsOff())");
    simSensorOff(sen);
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == false,
          "From-Empty + sensor Off -> false (golden :1471)");

    TrayForm.AutoFromEmptyColor[FT][iPos] = 1;          // Tray From Color -> read IsOff()
    simSensorOff(sen);
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == true,
          "From-Color + sensor Off -> true  (golden :1475 IsOff(), not IsOn())");
    simSensorOn(sen);
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == false,
          "From-Color + sensor On  -> false (golden :1475)");

    // TRI-STATE: an unconfigured sensor answers false to BOTH spellings, so the
    // two arms are NOT complements.  This is the property the brief forbids
    // normalising away, and it is directly observable here.
    simSensorUnknown(sen);
    TrayForm.AutoFromEmptyColor[FT][iPos] = 0;
    bool fromEmptyUnknown = ForTERAPOWERCheckColorSensor(iPos);
    TrayForm.AutoFromEmptyColor[FT][iPos] = 1;
    bool fromColorUnknown = ForTERAPOWERCheckColorSensor(iPos);
    CHECK(fromEmptyUnknown == false && fromColorUnknown == false,
          "TRI-STATE: a DISABLED sensor answers false to BOTH arms -- IsOn/IsOff are not complements");

    // RT/FT row selection: make the two rows disagree and flip the run mode.
    simSensorOn(sen);
    TrayForm.AutoFromEmptyColor[RT][iPos] = 0;          // RT row: From Empty  -> true
    TrayForm.AutoFromEmptyColor[FT][iPos] = 1;          // FT row: From Color  -> false
    LastSet.iRunStartMode = rsmContinuRetest;
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == true,
          "rsmContinuRetest selects the RT row (golden :1459-1462)");
    LastSet.iRunStartMode = rsmCInitialRetest;
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == true,
          "rsmCInitialRetest also selects the RT row (golden :1460)");
    LastSet.iRunStartMode = 0;
    CHECK(ForTERAPOWERCheckColorSensor(iPos) == false,
          "any other run mode selects the FT row (golden :1466)");

    simSensorUnknown(sen);
    TrayForm.AutoFromEmptyColor[RT][iPos] = 0;
    TrayForm.AutoFromEmptyColor[FT][iPos] = 0;
}

// =============================================================================
//  FAILABILITY PROOF -- REPRODUCIBLE RECIPE, NOT A POINTER
//
//  RECIPE.  Everything below was run in a private scratch dir; the shared build/
//  tree was only READ.  Each mutation was applied to a COPY of asendic_Color.cpp,
//  compiled off-tree with the flags CMake records for target ht9045_sm
//  (-std=c++1z -DCSYSTEM_TICK_ORACLE -Wall -Wextra -Wno-conversion-null
//   -Wno-write-strings -Wno-unused-parameter -I<tree> -I<tree>/SECSGEM, plus an
//  -I<scratch>/include_override carrying the shim headers with the three
//  retirement edits pre-applied), `ar r`'d into a COPY of libht9045_sm.a and
//  linked into a private exe.  The mutation was then reverted and the suite
//  re-run green before the next one.  The tables quote the ASSERTION MESSAGE and
//  never a `(line N)` suffix: the message is stable, the line number is pushed by
//  every edit to this banner.
//
//  BASELINE (pristine asendic_Color.cpp):   RESULT: 165 passed, 0 failed (exit 0)
//
//  68 mutations were run, ONE PER BUILD.  67 went red.  The full per-mutation
//  transcript is in the translate report; the shape of the result is:
//    * every actuator-DIRECTION probe went red (M01/M06 lifter up-vs-down in
//      DoLoadNewColorTrayToCar; M09 the case-1 ELSE arm's direct Off(); M12/M13
//      the Z-select Push-vs-Pop pair; M17/M23/M24/M54 the four Color-fix
//      Off/On/Pop/Push sites; M25/M39 the two floodgate closes; M40/M42 the
//      unload lifter pair).  Push() and Pop() BOTH return true offline, so none
//      of these could have been caught by a cursor assertion -- only by
//      Cylinder[].bCylinderOn;
//    * every tray-bookkeeping probe went red (M02/M03/M08/M38/M41/M68 the six
//      MMColorZ writes; M26/M28 the two Color_Car->Color handovers; M04
//      iTrayTotal[2]; M05 the TrayID rotation);
//    * every alarm-CODE probe went red (M10 MES1421, M11 JAM1406, M15 WAR16335,
//      M49 MES1420) -- so a copy-paste of the Empty file's code numbers into the
//      Color file cannot ship;
//    * all three LIVE fall-throughs went red when a `break;` was inserted
//      (M16 DoColorTrayToFront :296, M22 DoColorTrayToRear :408,
//       M67 DoColorTrayReadCID :1501).
//
//  FOUR PROBES SURVIVED ON FIRST RUN AND THREE OF THEM WERE TEST DEFECTS.  They
//  are recorded because each one is exactly the class of hole this project keeps
//  paying for, and because the fix is visible in the fixture:
//    (a) M20, golden :304-305.  Inverting only the FIRST `Status()==false`
//        survived: with both belts driven ON the conjunction is false either way.
//        Widened to invert BOTH operands -> red.  The PROBE was too narrow, not
//        the assertion.
//    (b) M29, golden :580.  Reading iReadCIDAction at the END of the AMR run
//        survived, because DoColorTrayReadCID :1499 writes the same value later.
//        Fixed by sampling the lock the first time the cursor rests on 455
//        (g_lockAt455) -> red.
//    (c) M33, golden :630.  The read-fail arm's fix-cylinder release survived,
//        because case 100's Pop() had already retracted it.  Fixed by re-driving
//        the cylinder EXTENDED at every 460 tick (reArmFixCylinder) -> red.
//    (d) M38, golden :1226.  The case-50 MMColorZ SetTray survived, because case 1
//        (golden :1185) had already set the same flag.  Fixed by WIPING the flag
//        mid-trajectory before case 50 runs -> red, and golden :1185 then got its
//        own independent assertion (M68) too.
//  THE FIFTH, M35, IS NOT A DEFECT AND WAS NOT "FIXED": restoring golden :1509's
//  commented-out `break;` leaves the suite green because that fall-through is
//  behaviourally inert.  See the [6d] banner for the argument and the
//  NOT-COVERED register for the entry.
// =============================================================================

// =============================================================================
//  NOT COVERED -- stated explicitly, because silence reads as coverage.
//
//  Fixture invariants that do most of the pruning:
//    (i)   TrayMoveIn/TrayMoveOut/TrayCylinMoveIn/TrayCylinMoveOut are NO-OPS in
//          this tree (asendic.cpp) and TrayMoveStatus is a hardwired `return 0`
//          with NO seam (asendic.h says so, and Wave 3 owns it).  So this file's
//          belt commands are completely unobservable, and every "did the belt
//          run the right way" question is out of reach until Wave 3.
//    (ii)  fTrayMapping's Keyence/cover-tray methods are offline-FALSE stubs with
//          no seam: DoTrayIDKeyence2() and DoCoverTrayID() never answer.
//    (iii) DoTrayIDCheck() returns TRUE offline by explicit Wave-0 decision, so
//          the "tray ID duplicate" arms cannot be reached from a successful read
//          either -- but since (ii) means no read ever succeeds, that decision is
//          not what is blocking them here.
//    (iv)  Cylinder Push()/Pop() BOTH return true offline, so no cursor assertion
//          can tell them apart -- which is why every direction claim above reads
//          Cylinder[].bCylinderOn instead.
//
//  ARMS THIS TEST DOES NOT REACH (and does not claim to):
//    * ALL FOUR belt-direction questions -- by (i).  Note the ASYMMETRY with
//      tests/test_w7_l1_auto2.cpp, which DOES assert belt polarity: asendic_Auto2
//      .cpp drives SW[] DIRECTLY, whereas asendic_Color.cpp routes every belt
//      command through TrayMoveIn/TrayMoveOut.  The two Color SW[] references
//      (golden :304-305) are READS, and they ARE exercised, in [4b].
//    * DoColorTrayToRear cases 300 and 400 -- MUTUALLY UNREACHABLE IN GOLDEN.
//      The only `Task=300` is inside case 400 (golden :542) and the only
//      `Task=400` is inside case 300 (golden :527), so neither can ever be
//      entered.  Reported as a golden observation, not a translation gap.
//    * DoColorTrayToRear case 120 and case 420 -- DEAD IN GOLDEN for the same
//      reason: nothing anywhere assigns Task=120 or Task=420.  (Note case 420's
//      body is a duplicate of the golden :477-480 guard inside case 200, and
//      case 120's body is a duplicate of case 455's -- they read like leftovers
//      from an earlier refactor.)  Both are translated verbatim regardless.
//    * DoColorTrayToRear cases 530 and 540, and the two "TrayID Read Error" /
//      "Tray ID Duplicate" arms of case 520 -- unreachable by (ii).
//    * DoAutoColor cases 80 and 85 -- same, reached only from case 70's
//      InitialTrayID2Task branch, which then waits on DoTrayIDKeyence2().
//    * DoColorTrayToRear case 600's success body (golden :799-807) -- the
//      EventReport(UnloaderTrayIDReadOK) + asBundleTrayID publish -- unreachable
//      by (ii).  [6] asserts only that the SM PARKS there.
//    * The eUnderCoveyor arms (TRAY_ARM_MODE==eUnderCoveyor) at golden :313, :410,
//      :466-473, :662-666, :490 and the DOUBLE_BELT_MODE==1 arm at :456-464, plus
//      case 430.  They are reachable in principle -- this fixture simply pins both
//      mode globals to 0 and does not drive them.  A follow-up wave should.
//    * The JAM1412 (golden :336) and JAM1401 (golden :510) watchdog arms: both are
//      guarded by 20 s real-QPC deadlines that a bounded virtual-tick loop cannot
//      burn, and both timers are deliberately never advanced (banner note 1).
//    * JAM1403 at golden :540 (case 400, dead per above) and at golden :933
//      (DoAutoColor case 40): the latter needs Sen[SenColorFixCyPush].IsOn() AND a
//      non-DUMMY run, and every DoAutoColor scenario here leaves that sensor
//      "unknown", so the else arm at golden :938-942 always wins.
//    * DoLoadNewColorTrayToCar case 200's bColorPause split-pause arm
//      (golden :201-207) -- the fixture pins bColorPause false throughout.
//    * Every bHandlerPause arm (golden :237-240, :298-302, :435-442) -- the
//      fixture pins bHandlerPause false throughout.
//    * The `#ifdef SOFT_SIMULTE` arms -- not defined in this build (by design;
//      the REAL machine logic is what is under test).
//    * The MES1420 arm of DoUnLoadNewColorToStack case 400 (golden :1275): its
//      `|| DUMMY` disjunct (golden :1261) wins on the DUMMY path, and the
//      non-DUMMY path never reaches 400 (it recirculates at 1->10->20, see [7]).
//    * DoColorTrayReadCID's TIMEOUT/RETRY arms: case 100's `else if
//      (tNFCColorDelay.Off())` iCount ladder (golden :1516-1523) and case 2000's
//      (golden :1541-1548).  Every sub-test here answers the reader (bReadUID2
//      true, then a non-empty sBlockID2), so the then-arms always win first.
//    * DoColorTrayReadCID's case 10 -> case 100 fall-through (golden :1509).  It
//      is BEHAVIOURALLY INERT -- see the [6d] banner for the measurement.  It is
//      translated verbatim but no assertion here can see it, and none claims to.
//    * golden :132's `&& LastSet.iRealDummy!=DUMMY` conjunct on the bG11ASEReport
//      gate: [3e]/[3f] run non-DUMMY and [3a] runs DUMMY-with-bG11ASEReport-false,
//      so no sub-test isolates that conjunct.  Deleting it alone leaves the suite
//      green.  Recorded rather than papered over.
//    * REAL lifter behaviour.  asendic.cpp's CylinderUp/Middle/Lower are SIM
//      bodies that command Cylinder[] and return "done" immediately; golden's are
//      full lifter state machines.  What is asserted here is exactly which
//      cylinders each call site drives and in which direction -- nothing about
//      lifter timing, alarms, or the iLifterTask/LifterTime god-stack.
// =============================================================================

int main()
{
    printf("==============================================================\n");
    printf(" W7-L1 Wave 1 -- asendic_Color (Color-tray stack feed) SM test\n");
    printf("==============================================================\n");

    test_inits();
    test_seams();
    test_load_new_color_dummy_walk();
    test_load_new_color_shortcircuit_and_fast_entry();
    test_load_new_color_else_arm_and_jam1406();
    test_load_new_color_ase_read();
    test_load_new_color_trayid_timeout_goldenbug();
    test_tray_to_front_car_arm();
    test_tray_to_front_dummy_timeout();
    test_tray_to_rear_plain();
    test_tray_to_rear_keyence_handoff();
    test_tray_to_rear_amr_rfid();
    test_read_cid_unit();
    test_unload_to_stack();
    test_receive_chain();
    test_drive_auto_color();
    test_gates_and_log();
    test_terapower_color_sensor();

    printf("\n--------------------------------------------------------------\n");
    printf(" RESULT: %d passed, %d failed\n", g_pass, g_fail);
    printf("--------------------------------------------------------------\n");
    return (g_fail == 0) ? 0 : 1;
}
