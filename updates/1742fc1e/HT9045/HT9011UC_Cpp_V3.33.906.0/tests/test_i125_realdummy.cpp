// tests/test_i125_realdummy.cpp
//
// AI(W906-I125) 20260930: new test.  Pins the REAL TfMain::CheckCanChangeRealDummy
// (cMainStatus.cpp:323, = golden main.cpp:12374-12380 line by line) at the three
// consumer families that used to see a stand-in instead:
//
//   (1) Automation/auto9045.cpp -- the W5FA_TfMainExt stand-in hard-coded
//       `return true` ("golden main.cpp body unavailable").  Retired in I125; the
//       free function CheckCanChangeRealDummy() (golden auto9045.cpp:1388-1391),
//       CheckNeedCleanOut() (golden :1135-1141), SetStartMode() (golden :1107-1127)
//       and SetLotInfo() (golden :1020-1040) now reach the real member.
//   (2) Command.cpp (S4) TfMain::GetHandlerStatusByDll -- golden 906_20260618
//       Command.cpp:9620 `fMain->CheckCanChangeRealDummy()==false` -> ret=3.
//   (3) Command.cpp (S10) TfMain::SetProdModeByDll(0) with a lot started --
//       golden :14335 `fMain->CheckCanChangeRealDummy()==false || HasICUnderMachine()`
//       -> return -1.
//   Both Command.cpp sites were a ComputeCanChangeRealDummy(...) substitution
//   (same six HasIC() reads) until I125 lifted them back to golden's text.
//
// WHICH ASSERTIONS CAN TELL THE OLD STAND-IN FROM THE REAL BODY (measured by
// reading the bodies, not assumed): HasICUnderMachine() (csystem.cpp:13304) ORs
// in every one of the six places CheckCanChangeRealDummy looks at, so
// CheckNeedCleanOut() and SetProdModeByDll() reject an IC even with the old
// `return true` stand-in.  The discriminating checks are the ones that consult
// CheckCanChangeRealDummy ALONE: the free function itself, SetStartMode,
// SetLotInfo, and GetHandlerStatusByDll's `ret=3`.  Those are the ones a revert
// of the I125 call sites turns red.
//
// NO FILE I/O: every call below returns before any write (checked per call,
// cited inline).  The empty-machine half deliberately does NOT call SetStartMode
// / SetLotInfo / SetProdModeByDll(0)+bLotStart -- on an empty machine they go on
// to SetRunStartMode / SetLotStart / SetLotEnd, which do real work.
//
// Each IC placement is followed by a PRECONDITION check that the placement is
// visible (e.g. InArmSuck.HasIC()==true); without it a grid whose iMaxRow is 0
// would make every "rejects" assertion vacuous (tests/test_auto9045.cpp PART 3
// records that exact trap for TestSocket).

#include "Automation/auto9045.h"   // CheckCanChangeRealDummy / CheckNeedCleanOut / SetStartMode / SetLotInfo

#include "MachineDefine.h"
#include "MachineType.h"
#include "cmydef.h"                // InitialOK / SystemStart / HAS_IC / NULL_IC / MMPlate1 / MMPlate2 / SnMotorPower
#include "cprod.h"                 // RunInfo.bLotStart
#include "FormsFacade.h"           // fMain (forms/fMain.h TfMain), palMainStatus
#include "aHotPlateSubstrate.h"    // InArmSuck / OutArmSuck / FRCarryKit -- the SAME TMyKitSuck header cMainStatus.cpp:29 uses
#include "Motor/mymotor.h"         // MOT[] / TTrayMotor::HasIC
#include "mysensor.h"              // Sen[SnMotorPower]
#include "csystem.h"               // ShuttleHasIC / IndexHasIC / HasICUnderMachine

#include <cstdio>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                 \
        else      { std::printf("FAIL  %s\n", msg); ++g_fail; }                 \
    } while (0)

// ---------------------------------------------------------------------------
static void ClearKit(TMyKitSuck &k)
{
    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; i++)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; j++)
            k.Item[i][j] = NULL_IC;
}

static void ClearMachine()
{
    ClearKit(InArmSuck);
    ClearKit(OutArmSuck);
    ClearKit(FRCarryKit);
    MOT[MMPlate1].fHasTray = false;
    MOT[MMPlate1].Tray.Data[0][0] = 0;
}

// The one place this test puts a hot-plate IC: TTrayMotor::HasIC (Motor/mymotor.cpp:1960)
// needs fHasTray AND TMyTray::HasIC (mytray.cpp:231), which scans XItem x YItem.
static int g_savedPlateX = 0, g_savedPlateY = 0;
static void PutPlateIC()
{
    g_savedPlateX = MOT[MMPlate1].Tray.XItem;
    g_savedPlateY = MOT[MMPlate1].Tray.YItem;
    if (MOT[MMPlate1].Tray.XItem < 1) MOT[MMPlate1].Tray.XItem = 1;
    if (MOT[MMPlate1].Tray.YItem < 1) MOT[MMPlate1].Tray.YItem = 1;
    MOT[MMPlate1].fHasTray = true;
    MOT[MMPlate1].Tray.Data[0][0] = HAS_IC;
}
static void RestorePlateGeometry()
{
    MOT[MMPlate1].Tray.XItem = g_savedPlateX;
    MOT[MMPlate1].Tray.YItem = g_savedPlateY;
}

// GetHandlerStatusByDll (Command.cpp:9848) reaches the (S4) check only when:
// InitialOK, !SystemStart, no fNote/MyMessageBox showing, no settings window open,
// and Sen[SnMotorPower].IsOff()==false.  TMySensor::IsOff returns false when
// Enable==false (mysensor.cpp:177-181), which is the ctor default (mysensor.cpp:49);
// pinned explicitly here so a loaded IO table cannot change the path.
static bool g_savedSenEnable = false;
static void ArmStatusPath()
{
    InitialOK   = true;
    SystemStart = false;
    g_savedSenEnable = Sen[SnMotorPower].Enable;
    Sen[SnMotorPower].Enable = false;
}
static void DisarmStatusPath()
{
    Sen[SnMotorPower].Enable = g_savedSenEnable;
    InitialOK = false;
}

// ---------------------------------------------------------------------------
static void Part1_EmptyMachine()
{
    std::printf("\n[1] empty machine -> CheckCanChangeRealDummy()==true (golden main.cpp:12379 `return true`)\n");
    ClearMachine();
    CHECK(!InArmSuck.HasIC() && !OutArmSuck.HasIC() && !ShuttleHasIC() && !IndexHasIC() &&
          !MOT[MMPlate1].HasIC() && !MOT[MMPlate2].HasIC(),
          "precondition: none of the six places CheckCanChangeRealDummy reads has an IC");
    CHECK(HasICUnderMachine() == false, "precondition: HasICUnderMachine()==false (csystem.cpp:13304)");

    CHECK(fMain->CheckCanChangeRealDummy() == true,  "TfMain::CheckCanChangeRealDummy() == true on an empty machine");
    CHECK(CheckCanChangeRealDummy() == true,         "auto9045 CheckCanChangeRealDummy() (golden auto9045.cpp:1388-1391) == true");
    CHECK(CheckNeedCleanOut() == false,              "auto9045 CheckNeedCleanOut() (golden :1135-1141) == false -> host Set* commands proceed");

    ArmStatusPath();
    AnsiString savedCaption = fMain->palMainStatus->Caption;
    fMain->palMainStatus->Caption = "HALT";
    CHECK(fMain->GetHandlerStatusByDll() == 1, "(S4) GetHandlerStatusByDll: empty + palMainStatus HALT -> 1 (golden Command.cpp:9624-9627), not 3");
    fMain->palMainStatus->Caption = "";
    CHECK(fMain->GetHandlerStatusByDll() == 6, "(S4) GetHandlerStatusByDll: empty + other caption -> 6 (golden :9628-9631), not 3");
    fMain->palMainStatus->Caption = savedCaption;
    DisarmStatusPath();
}

// ---------------------------------------------------------------------------
//  One IC in one place -> every consumer rejects.
// ---------------------------------------------------------------------------
static void ExpectRejects(const char *where)
{
    char msg[256];

    std::snprintf(msg, sizeof msg, "[%s] TfMain::CheckCanChangeRealDummy() == false", where);
    CHECK(fMain->CheckCanChangeRealDummy() == false, msg);

    std::snprintf(msg, sizeof msg, "[%s] auto9045 CheckCanChangeRealDummy() == false (DISCRIMINATING: the retired stand-in said true)", where);
    CHECK(CheckCanChangeRealDummy() == false, msg);

    std::snprintf(msg, sizeof msg, "[%s] auto9045 CheckNeedCleanOut() == true -> host Set* commands answer 1", where);
    CHECK(CheckNeedCleanOut() == true, msg);

    AnsiString d[40];
    d[0] = "1";                                                  // a valid StartMode, so only the guard can refuse
    std::snprintf(msg, sizeof msg, "[%s] SetStartMode(\"1\") == 2 (golden auto9045.cpp:1111/:1124-1127; DISCRIMINATING)", where);
    CHECK(SetStartMode(d) == 2, msg);                            // else arm returns before SetRunStartMode

    AnsiString li[40];
    li[0] = "I125LOT"; li[1] = "I125OP"; li[2] = "0";
    std::snprintf(msg, sizeof msg, "[%s] SetLotInfo(...) == 2 (golden auto9045.cpp:1022/:1037-1040; DISCRIMINATING)", where);
    CHECK(SetLotInfo(li) == 2, msg);                             // else arm returns before SetLotID/SetLotStart

    ArmStatusPath();
    std::snprintf(msg, sizeof msg, "[%s] (S4) GetHandlerStatusByDll() == 3 (golden Command.cpp:9620-9623; DISCRIMINATING)", where);
    CHECK(fMain->GetHandlerStatusByDll() == 3, msg);

    bool savedLot = RunInfo.bLotStart;
    RunInfo.bLotStart = true;
    std::snprintf(msg, sizeof msg, "[%s] (S10) SetProdModeByDll(0) with a lot started == -1 (golden Command.cpp:14335-14338)", where);
    CHECK(fMain->SetProdModeByDll(0) == -1, msg);                // returns before SetLotEnd
    RunInfo.bLotStart = savedLot;
    DisarmStatusPath();
}

static void Part2_InArmSuck()
{
    std::printf("\n[2] IC in InArmSuck\n");
    ClearMachine();
    InArmSuck.Item[0][0] = HAS_IC;
    CHECK(InArmSuck.HasIC() == true, "precondition: InArmSuck.HasIC()==true (the placement is visible)");
    ExpectRejects("InArmSuck");
    ClearMachine();
}

static void Part3_OutArmSuck()
{
    std::printf("\n[3] IC in OutArmSuck\n");
    ClearMachine();
    OutArmSuck.Item[0][0] = HAS_IC;
    CHECK(OutArmSuck.HasIC() == true, "precondition: OutArmSuck.HasIC()==true");
    ExpectRejects("OutArmSuck");
    ClearMachine();
}

static void Part4_Shuttle()
{
    std::printf("\n[4] IC on the output shuttle (FRCarryKit)\n");
    ClearMachine();
    FRCarryKit.Item[0][0] = HAS_IC;
    CHECK(ShuttleHasIC() == true, "precondition: ShuttleHasIC()==true via FRCarryKit.UseSiteHasIC() (csystem.cpp:19077/:19094)");
    ExpectRejects("shuttle FRCarryKit");
    ClearMachine();
}

static void Part5_HotPlate()
{
    std::printf("\n[5] IC on hot plate 1 (MOT[MMPlate1])\n");
    ClearMachine();
    PutPlateIC();
    CHECK(MOT[MMPlate1].HasIC() == true, "precondition: MOT[MMPlate1].HasIC()==true");
    ExpectRejects("MMPlate1");
    ClearMachine();
    RestorePlateGeometry();
}

static void Part6_BackToEmpty()
{
    std::printf("\n[6] cleared again -> true again (no latch)\n");
    ClearMachine();
    CHECK(fMain->CheckCanChangeRealDummy() == true, "TfMain::CheckCanChangeRealDummy() == true after the IC is removed");
    CHECK(CheckNeedCleanOut() == false,             "CheckNeedCleanOut() == false after the IC is removed");
}

int main()
{
    Part1_EmptyMachine();
    Part2_InArmSuck();
    Part3_OutArmSuck();
    Part4_Shuttle();
    Part5_HotPlate();
    Part6_BackToEmpty();

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
