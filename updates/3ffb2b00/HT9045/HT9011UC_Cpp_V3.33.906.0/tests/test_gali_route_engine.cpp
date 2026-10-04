// =============================================================================
//  tests/test_gali_route_engine.cpp -- AI(W906-INDEXZ-1203) 20260929
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo -- part 0 now builds the four index axes as golden's ship arm does
//  (all enabled) and pins that nothing disables them without the route (review #7); D1 is applied the way the
//  installer applies it; parts 9-12 are new: Z2's Position is not flipped (review #4), a stopped home is never done
//  (review #1), VS0;SP0 + START resumes the move (review #2), a stop outside the route re-issues the move.
//
//  The ENGINE half of HT9050's Index Z1 Gali_* -> PCIE-1203 route: golden's own
//  MOT[MTestZ1].Gali_* state machines (Motor/myGALILmotor.cpp, Motor/mymotor.cpp)
//  run unchanged against the REAL TGaliRouteCore bound to a FAKE 1203 (no monitor,
//  no control object, no vendor call). Design section 6 "GaliRouteEngine" 1-8:
//    0  no route installed: today's no-card answers, the route sees nothing, the
//       four index axes stay as golden built them; then install + D1
//    1  Gali_MotMove: start -> stale -> moving -> READY x DelayCount -> true once;
//       encoder off by > 6000 -> golden's JAM (iHome, GaliAxisAlarm[1])
//    2  Gali_MotMoveNoWait(iNeedDelayTime=0): done on the first fresh idle sample
//    3  Gali_Two_ZAxis_Move: receiver Z1 moves Z1 only, completes with Z2 disabled;
//       receiver Z2 sends nothing (golden disabled-axis behaviour)
//    4  StopAllMotor(true) = one Stop + ExtDrive(0); MOT[MTestY1].Gali_Command("ST")
//       stops Z1 and clears its MovFlag (golden's own clearing)
//    5  LEDs from the monitor bits; TIMO alarm / servo; ScanIndexMotorCanMove on ALM
//    6  ServoOnOff(true/false) -> SH / AB1 + MO -> SvOn 1 / EmgStop + SvOn 0
//    7  Gali_SingalHome routed: 1->5->8->200->250->400->450->500, true once,
//       HomeFlag=1, the move to TestZ1_Safe; servo off at 8 -> back to 1
//    8  Gali_FindZPhase refuses
//    9  Gali_Two_ZAxis_Move leaves a no-card Z2's Position alone (review #4)
//   10  a home stopped at 250 (ST / StopAllMotor) stays at 250, HomeFlag 0 (review #1)
//   11  PAUSE-like halt during Gali_MotMove: StopAllMotor(true), SystemStart false,
//       then START -> the move is re-issued to its target and completes (review #2)
//       AI(W906-INDEXZ-1203) 20260930 (round 2 A): START with the door OPEN (ckernel's KS string, the door check,
//       StopAllMotor) resumes nothing; START with the door closed resumes only on DoSystem G22's VS/SP string
//   12  a stop outside the route mid-move: golden MovFlag=false -> the move is re-issued
//       (round 2 D2: an operator stop = golden ST's whole bookkeeping, all four MovFlag + bZ1Z2Exute)
//   13  uninstall restores "not routed"
//   AI(W906-INDEXZ-1203) 20260930: part 10 how==2 -- StopAllMotor(true) during the home now HALTS it (D1) and
//       DoSystem's VS/SP string restarts it to completion; part 14 -- golden TestZ1SetPos's "DP" (B) defines the
//       command position (aTester_Front.cpp:314-320, the real function).
//   AI(W906-TEACH-ZDISABLED) 20261001: part 15 -- the Teach interlock (forms/fTeach.cpp IsCanQuickJogMove, the real body) for the
//       shuttle and Index Y movers: MTestZ2 with Mot_Table Enable=0 is not checked; Enable=1 + LED false keeps golden's refusal.
//   AI(W906-INDEXZ2) 20261002: part 17 -- Z1's HOME lamp from the route follows the tree's ORG rule (HT9050: per axis by SensorType).
//  Reads and writes no file (ShowMotorErrorMessage / ShowMyMessage are the   [AI(W906-JAM-STOP) 20260930: no longer so -- ShowMotorErrorMessage is the golden body now (forms/fNote_ShowError.cpp EOF):
//  canary recorders; InitialOK=false takes their early return).              its InitialOK==false arm still returns before the note, after the stop half (StopAllMotor / "ST" / brake), and writes MyDBIProcess("Exception") into the redirect roots -- main() refuses to run outside ctest (w906_ctest_guard.h)]
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                 // INDEX_MOTION_CARD / USE_INDEX_ARM_AXES / iHome / InitialOK / MTest* / SystemStart
#include "Config.h"                 // IniConfig.GaliPosRange
#include "CosFunction.h"            // CosFunction.bIndexProtect
#include "cprod.h"                  // Prod.TestZ1_Safe
#include "canary_support.h"         // W906_ShowMotorErrorMessage_Count
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/HTMotor.h"          // iServoOn / iInposLed / iHomeLed / iCwLed / iCcwLed / iAlarmLed
#include "Motor/myGALILmotor.h"     // TMyGALILMotor / StopAllMotor / ScanIndexMotorCanMove
#include "Motor/GaliRoute.h"
#include "EtherCAT/Pci1203GaliRouteCore.h"
#include "aTester_Front.h"          // AI(W906-INDEXZ-1203) 20260930: TestZ1SetPos (part 14)
#include "w906_test_motors.h"
#include "forms/fTeach.h"           //AI(W906-TEACH-ZDISABLED) 20261001: TfTeach::IsCanQuickJogMove + W906_TeachHomeLedHook / W906_TeachLive1203Hook (part 15)
#include "w906_ctest_guard.h"   //AI(W906-JAM-STOP) 20260930: W906TestRequireCtestRedirects (the JAM's Exception record writes; on the old blank line, no line moves)
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

extern bool IndexZCanMove[2];       // ainarm9045_w7_shims.cpp:57
extern bool bGalilTwoYMoveFlag;      // Motor/myGALILmotor.cpp (GalilTwoY_Move's file-level flag) -- AI(W906-INDEX-ABSENT) 20261004
extern bool GaliAxisAlarm[4];       // Motor/myGALILmotor.cpp:737
extern int  GailAcSpeed;            // Motor/myGALILmotor.cpp:744 (golden initial 20000000; SetGaliRate is not called here)
extern int  GailAcSpeed2;           // Motor/myGALILmotor.cpp:746

using namespace ht9045;

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_gali_route_engine.cpp:%d]  %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- the fake 1203 -------------------------------------------------------------
struct FakeIo : IGaliRouteIo {
    unsigned long   poll;
    GaliRouteSample s;
    int             kind;
    unsigned long   now;
    std::map<int, unsigned long> ledger;
    std::vector<Pci1203Cmd> cmds;
    FakeIo() : poll(10), kind(1), now(1000) { s.state = 1; s.motionIO = 0x00004000ul; }
    bool CardOpen() override { return true; }
    unsigned long PollCount() override { return poll; }
    int Slot(std::string&) override { return 7; }
    bool Sample(int, GaliRouteSample& o) override { o = s; return true; }
    int DriveKind(int) override { return kind; }
    Pci1203CmdResult Exec(const Pci1203Cmd& c) override
    {
        cmds.push_back(c);
        Pci1203CmdResult r;
        r.accepted = true; r.issued = true; r.ret = 0;
        return r;
    }
    void Caps(GaliRouteCaps& c) override
    {
        HTMotor* M = MOT[MTestZ1].Motor;
        c.jogHigh = M->PJogHighSpeed; c.initSpeed = M->InitSpeed; c.accDb = M->GetAccDataBase(); c.decDb = M->GetDecDataBase();
    }
    unsigned long NowMs() override { return now; }
    void NoteIssued(int sl, unsigned long p) override { ledger[sl] = p; }
    bool LastIssued(int sl, unsigned long& p) override
    {
        std::map<int, unsigned long>::const_iterator it = ledger.find(sl);
        if (it == ledger.end()) return false;
        p = it->second;
        return true;
    }
    bool OnOwnerThread() override { return true; }
    void SleepMs(int) override {}  int OrgHome(int, unsigned long) override { return W906_Ht9050OrgHome(MTestZ1); }   //AI(W906-INDEXZ2) 20261002: as EtherCAT/Pci1203GaliRoute.cpp LiveIo::OrgHome (part 17); no hook = -2 = golden's decode (parts 0-16)
    void Log(const std::string& l) override { std::printf("    [route] %s\n", l.c_str()); }
    void LogForce(const std::string& l) override { std::printf("    [route!] %s\n", l.c_str()); }
};

static FakeIo         g_io;
static TGaliRouteCore g_core;
static bool T_Command(int, const char* d, long* r) { return g_core.Command(d, r); }
static int  T_HomeStart(int, bool dir, unsigned hi, unsigned lo, double a, double d) { return g_core.HomeStart(dir, hi, lo, a, d); }
static int  T_HomePoll(int) { return g_core.HomePoll(); }
static TGaliRoute g_route = { -1, T_Command, T_HomeStart, T_HomePoll };

static bool DoorClosed() { return false; }

static void Fresh(unsigned state, double cmd, double act, unsigned long io = 0x00004000ul)
{
    ++g_io.poll;
    g_io.s.state = state; g_io.s.cmdPos = cmd; g_io.s.actPos = act; g_io.s.motionIO = io;
}
static int Kinds(Pci1203CmdKind k)
{
    int n = 0;
    for (std::size_t i = 0; i < g_io.cmds.size(); ++i) if (g_io.cmds[i].kind == k) ++n;
    return n;
}
static double LastMoveAbs()
{
    for (std::size_t i = g_io.cmds.size(); i > 0; --i) if (g_io.cmds[i - 1].kind == kCmdAxMoveAbs) return g_io.cmds[i - 1].value;
    return -1.0;
}

int main()
{
    if (!W906TestRequireCtestRedirects("GaliRouteEngine")) return 2;  std::printf("=== test_gali_route_engine (AI(W906-INDEXZ)) ===\n");   //AI(W906-JAM-STOP) 20260930: guard first (see the banner)
    INDEX_MOTION_CARD = 0;                                  // HT9050 Gerneral.ini (stays 0, RULINGS_20260926 #6 / D1=A)
    USE_INDEX_ARM_AXES = IndexArm_4_Axis;                   // HT9050 Gerneral.ini
    IniConfig.GaliPosRange = 50;                            // D:\HT9045\config\config.ini value
    CosFunction.bIndexProtect = false;
    InitialOK = false;                                      // canary ShowMotorErrorMessage: record, early return   [AI(W906-JAM-STOP) 20260930: the golden body now -- stop half, Exception record, early return (golden note.cpp:1054-1065)]
    SystemStart = false;
    const char* names[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {
        const int i = MTestY1 + k;
        MOT[i].Motor = new TMyGALILMotor(k);                // cinitial.cpp Galil arm (ports 0..3 = x/y/z/w)
        MOT[i].SetAlias(i, names[k]);
        MOT[i].Motor->Enable = true;                        // golden's ship arm: every index axis on (4-axis), card or not
        MOT[i].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
        MOT[i].Motor->PServoAlarmOn = true;
        MOT[i].Motor->GearRatio = 1.0;
        for (int l = 0; l < 10; ++l) MOT[i].Led[l] = false;
    }
    W906_TestEnsureSimMotors();                             // every other MOT[].Motor non-NULL (golden boot invariant)
    HTMotor* Z = MOT[MTestZ1].Motor;
    Z->PJogHighSpeed = 900000; Z->InitSpeed = 100; Z->SetAccDataBase(9000000.0); Z->SetDecDataBase(9000000.0);   // machines/HT9050/Mot_Table.csv:16
    Z->PHomeHighSpeed = 6000; Z->PHomeLowSpeed = 300; Z->HomeDirection = true;
    IndexZCanMove[0] = IndexZCanMove[1] = true;             // D3=B: what the installer sets
    const double acc2 = GailAcSpeed2 > 9000000 ? 9000000.0 : (double)GailAcSpeed2;
    const double acc1 = GailAcSpeed  > 9000000 ? 9000000.0 : (double)GailAcSpeed;

    // ---- 0: no route --------------------------------------------------------------
    std::printf("-- 0. no route installed = golden no-card; the four index axes stay as golden built them (review #7)\n");
    W906_SetGaliRoute(0);
    CHECK(!W906_GaliRouteOwns(MTestZ1));
    CHECK(MOT[MTestZ1].Gali_ReadEncoderPos() == 0);         // TPY, no card -> 0
    MOT[MTestZ1].Position = -777;
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 777);              // no-card: -Position
    MOT[MTestZ1].Position = -300;
    CHECK(MOT[MTestZ1].Gali_MotMove(300, 50) == true);      // no-card sim branch: already there
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(500, 100) == true);
    MOT[MTestZ1].MovFlag = true;
    MOT[MTestY1].Gali_Command("ST");
    CHECK(MOT[MTestZ1].MovFlag == false);                   // golden's no-card "ST" clearing
    MOT[MTestZ1].MovFlag = true;
    CHECK(W906_GaliRouteForeignStopBookkeeping(MTestZ1) == false && MOT[MTestZ1].MovFlag == true);   // no route: touches nothing
    MOT[MTestZ1].MovFlag = false;
    StopAllMotor(true);
    CHECK(g_io.cmds.empty());
    CHECK(MOT[MTestY1].Motor->Enable && MOT[MTestZ1].Motor->Enable && MOT[MTestZ2].Motor->Enable && MOT[MTestY2].Motor->Enable);
    //AI(W906-INDEX-ABSENT) 20261004 (dispatch 7 C1): with NO route nothing is "absent" -- a disabled Z2 still takes golden's path,
    //  including the IndexZCanMove[0] lock its Gali_MotMove takes before the Enable test (so C1 changes no other machine)
    CHECK(!W906_GaliRouteAbsentAxis(MTestY1) && !W906_GaliRouteAbsentAxis(MTestZ2) && !W906_GaliRouteAbsentAxis(MTestY2));
    MOT[MTestZ2].Motor->Enable = false; IndexZCanMove[0] = IndexZCanMove[1] = true; MOT[MTestZ2].Position = 0;
    CHECK(!W906_GaliRouteAbsentAxis(MTestZ2));
    CHECK(MOT[MTestZ2].Gali_MotMove(500, 900) == false && IndexZCanMove[0] == false && MOT[MTestZ2].Position == 0);   // golden: the disabled Z2 takes Z1's
                                                            //  lock, then simulates at member speed 0 -- never arrives (dispatch 7 A5-6, the Z1 freeze); kept where no route
    MOT[MTestZ2].Motor->Enable = true; MOT[MTestZ2].MovFlag = false;
    MOT[MTestZ1].Position = 0; MOT[MTestZ2].Position = 0;
    IndexZCanMove[0] = IndexZCanMove[1] = true;

    // install, as W906_InstallPci1203GaliRoute does (owner, Bind, {1,1}, the route, then D1)
    g_route.owner = MTestZ1;
    g_core.Bind(&g_io);
    W906_SetGaliRoute(&g_route);
    CHECK(W906_GaliRouteOwns(MTestZ1) && !W906_GaliRouteOwns(MTestZ2) && !W906_GaliRouteOwns(MTestY1));
    CHECK(W906_GaliRouteDisableAbsentIndexAxes(true, false, false) == 2);   // Y1 "in the table" is kept
    CHECK(MOT[MTestY1].Motor->Enable && !MOT[MTestZ2].Motor->Enable && !MOT[MTestY2].Motor->Enable && MOT[MTestZ1].Motor->Enable);
    CHECK(W906_GaliRouteDisableAbsentIndexAxes(false, false, false) == 1);  // HT9050: M13 / M15 / M16 have Enable 0
    CHECK(!MOT[MTestY1].Motor->Enable && MOT[MTestZ1].Motor->Enable);      // the routed axis is never touched
    CHECK(W906_GaliRouteDisableAbsentIndexAxes(false, false, false) == 0);

    // ---- 0b: AI(W906-INDEX-ABSENT) 20261004 (dispatch 7 C1): the axes the route left absent are done at once ----
    std::printf("-- 0b. absent Index axes (Y1 / Z2 / Y2 on HT9050) are done at once, before any lock, nothing sent\n");
    CHECK(W906_GaliRouteAbsentAxis(MTestY1) && W906_GaliRouteAbsentAxis(MTestZ2) && W906_GaliRouteAbsentAxis(MTestY2));
    CHECK(!W906_GaliRouteAbsentAxis(MTestZ1) && !W906_GaliRouteAbsentAxis(MInArmX));
    {
        const std::size_t n0 = g_io.cmds.size();
        IndexZCanMove[0] = IndexZCanMove[1] = true; MOT[MTestZ2].Position = 0;
        CHECK(MOT[MTestZ2].Gali_MotMove(500, 900) == true && IndexZCanMove[0] == true && IndexZCanMove[1] == true);   // A5-6: Z1 not frozen
        CHECK(MOT[MTestZ2].Position == 0);                  // done at once: golden's disabled branch (and its lock) never ran
        CHECK(MOT[MTestZ2].Gali_MotMoveNoWait(500, 900, 0) == true && IndexZCanMove[0] == true);
        CHECK(MOT[MTestZ2].Gali_MotMove2(500, 900, 900) == true && IndexZCanMove[0] == true);
        CHECK(MOT[MTestZ2].Gali_MotMoveSkipEncoder(500, 900) == true);
        MOT[MTestY1].Position = 123; MOT[MTestY2].Position = 456; bGalilTwoYMoveFlag = true;
        CHECK(MOT[MTestY1].GalilTwoY_Move(2000, 3000, 900) == true && MOT[MTestY1].Position == 2000 && MOT[MTestY2].Position == -3000 &&
              MOT[MTestY1].TargetPosition == 2000 && MOT[MTestY2].TargetPosition == -3000 && bGalilTwoYMoveFlag == false);   // the disabled branch's final state
        CHECK(g_io.cmds.size() == n0);                      // none of them reached the 1203
        MOT[MTestY1].Position = 0; MOT[MTestY2].Position = 0; MOT[MTestZ2].Position = 0;
        IndexZCanMove[0] = IndexZCanMove[1] = true;
    }

    // ---- 0c: AI(W906-INDEX-PRESSBLOCK) 20261004 (dispatch 7 A5-7): the paired presses are refused, never reported done ----
    std::printf("-- 0c. Z1UpZ2Down / Z1DownZ2Up / nnMode Down / Up refused where the route left Y1 absent (Z1 was never pressed)\n");
    {
        const std::size_t n0 = g_io.cmds.size();
        MOT[MTestZ1].Position = 0; MOT[MTestY1].Position = 0;
        W906_ShowMyMessage_Reset();
        CHECK(MOT[MTestY1].Z1UpZ2Down(100, false, false) == false && W906_ShowMyMessage_Count == 1 &&
              std::string(W906_ShowMyMessage_LastS1.c_str()).find("Z1UpZ2Down Error") == 0);
        CHECK(MOT[MTestY1].Z1DownZ2Up(100, false, false) == false && MOT[MTestY1].Gali_nnMode_Z1Z2_Down(100, false) == false &&
              MOT[MTestY1].Gali_nnMode_Z1Z2_Up(100, false) == false && W906_ShowMyMessage_Count == 4);
        CHECK(MOT[MTestZ1].Position == 0 && MOT[MTestY1].Position == 0 && g_io.cmds.size() == n0);   // nothing rewritten, nothing sent
        W906_ShowMyMessage_Reset();
    }

    // ---- 1: Gali_MotMove ----------------------------------------------------
    std::printf("-- 1. Gali_MotMove\n");
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0; MOT[MTestZ1].iCheckStatusCT = 0;
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 900) == false);   // start
    CHECK(g_io.cmds.size() == 5);
    if (g_io.cmds.size() == 5) {
        CHECK(g_io.cmds[0].kind == kCmdAxSetSpeed && g_io.cmds[0].speed == kSpeedInit && g_io.cmds[0].value == 100.0 && g_io.cmds[0].axis == 7);
        CHECK(g_io.cmds[1].speed == kSpeedRun && g_io.cmds[1].value == 900.0);
        CHECK(g_io.cmds[2].speed == kSpeedAcc && g_io.cmds[2].value == acc2);   // GailAcSpeed2 capped by the M14 Acc
        CHECK(g_io.cmds[4].kind == kCmdAxMoveAbs && g_io.cmds[4].value == 1000.0);   // PAY=-1000 -> card +1000
    }
    CHECK(MOT[MTestZ1].MovFlag == true && IndexZCanMove[1] == false);
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 900) == false);   // same poll: stale sample
    Fresh(5, 1000.0, 400.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 900) == false);   // PTP
    Fresh(1, 1000.0, 1000.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 900) == false);   // READY: settle 1 of DelayCount 2
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 900) == true);    // settle 2 -> range check OK -> true once
    CHECK(MOT[MTestZ1].MovFlag == false && IndexZCanMove[1] == true && g_io.cmds.size() == 5);
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 1000 && MOT[MTestZ1].Gali_ReadEncoderPos() == 1000);   // TDY=-1000 -> +1000; TPY=+1000
    // encoder short by 15000 -> golden JAM
    {
        const int jam0 = W906_ShowMotorErrorMessage_Count;
        iHome = 0;
        CHECK(MOT[MTestZ1].Gali_MotMove(20000, 900) == false);
        CHECK(g_io.cmds.size() == 10 && g_io.cmds[9].value == 20000.0);
        Fresh(1, 20000.0, 5000.0);
        CHECK(MOT[MTestZ1].Gali_MotMove(20000, 900) == false);
        CHECK(MOT[MTestZ1].Gali_MotMove(20000, 900) == false);   // range check fails
        CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1 && iHome == 1 && GaliAxisAlarm[1] == true);
        CHECK(MOT[MTestZ1].MovFlag == false);
        iHome = 0;
    }

    // ---- 2: Gali_MotMoveNoWait ------------------------------------------------
    std::printf("-- 2. Gali_MotMoveNoWait(iNeedDelayTime=0)\n");
    Fresh(1, 0.0, 0.0);
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    g_io.cmds.clear();
    CHECK(MOT[MTestZ1].Gali_MotMoveNoWait(5000, 800, 0) == false);
    CHECK(g_io.cmds.size() == 5 && g_io.cmds[4].value == 5000.0);
    Fresh(1, 5000.0, 5000.0);
    CHECK(MOT[MTestZ1].Gali_MotMoveNoWait(5000, 800, 0) == true);   // first fresh idle sample; golden MySleep(100) runs
    CHECK(MOT[MTestZ1].MovFlag == false && IndexZCanMove[1] == true);

    // ---- 3: Gali_Two_ZAxis_Move --------------------------------------------------
    std::printf("-- 3. Gali_Two_ZAxis_Move\n");
    for (int l = 0; l < 10; ++l) MOT[MTestZ2].Led[l] = false;
    g_io.cmds.clear();
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(500, 30000) == false);
    CHECK(g_io.cmds.size() == 5);
    if (g_io.cmds.size() == 5) {
        CHECK(g_io.cmds[1].speed == kSpeedRun && g_io.cmds[1].value == 30000.0);
        CHECK(g_io.cmds[2].value == acc1);                   // the Two-Z string carries GailAcSpeed
        CHECK(g_io.cmds[4].kind == kCmdAxMoveAbs && g_io.cmds[4].value == 500.0);
    }
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(500, 30000) == false);   // stale
    Fresh(1, 500.0, 500.0);
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(500, 30000) == false);   // settle 1 (MG_BGz: not claimed -> 0, Z2 lamp off)
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(500, 30000) == true);    // settle 2
    CHECK(IndexZCanMove[0] == true && IndexZCanMove[1] == true && g_io.cmds.size() == 5);
    CHECK(MOT[MTestZ2].Gali_Two_ZAxis_Move(700, 100) == true);      // receiver Z2 disabled: golden instant true
    CHECK(g_io.cmds.size() == 5);                                   // ... and Z1 is NOT moved
    MOT[MTestZ2].Position = 0;

    // ---- 4: stops ----------------------------------------------------------------
    std::printf("-- 4. stops\n");
    g_io.cmds.clear();
    StopAllMotor(true);                                     // MOT[MTestY1].Gali_Command("VS0;SP0,0,0,0;")
    CHECK(Kinds(kCmdAxStop) == 1 && Kinds(kCmdAxSetExtDrive) == 1 && g_io.cmds.size() == 2);
    CHECK(!g_core.Halted());                                // Z1 was idle: nothing to keep
    MOT[MTestZ1].MovFlag = true;
    g_io.cmds.clear();
    MOT[MTestY1].Gali_Command("ST");
    CHECK(Kinds(kCmdAxStop) == 1 && Kinds(kCmdAxSetExtDrive) == 1 && MOT[MTestZ1].MovFlag == false);

    // ---- 5: LEDs -----------------------------------------------------------------
    std::printf("-- 5. LEDs / TIMO / alarm gate\n");
    Fresh(1, 500.0, 500.0, 0x00004000ul | 0x10ul | 0x04ul);   // SVON + ORG + LMT+
    MOT[MTestZ1].iCheckStatusCT = 0;
    MOT[MTestZ1].ScanMotorStatus();                         // golden :1789 restored for the routed axis
    CHECK(MOT[MTestZ1].Led[iHomeLed] == true && MOT[MTestZ1].Led[iCcwLed] == true);
    CHECK(MOT[MTestZ1].Led[iCwLed] == false && MOT[MTestZ1].Led[iInposLed] == false);
    Fresh(1, 500.0, 500.0, 0x00004000ul | 0x02ul);          // SVON + ALM
    MOT[MTestZ1].MovFlag = false;
    MOT[MTestZ1].Gali_ScanMotStatusTIMO();
    CHECK(MOT[MTestZ1].Led[iAlarmLed] == true && MOT[MTestZ1].Led[iServoalarmLed] == true && MOT[MTestZ1].Led[iServoOn] == true);
    {
        const int jam0 = W906_ShowMotorErrorMessage_Count;
        CHECK(ScanIndexMotorCanMove() == false);            // ALM on an enabled PServoAlarmOn axis -> golden JAM, no move
        CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1);
    }
    MOT[MTestZ1].MovFlag = true;
    CHECK(MOT[MTestZ1].GetMotorAlarm() == true);            // golden :1771 restored: scan, then the latch
    MOT[MTestZ1].MovFlag = false;
    Fresh(1, 500.0, 500.0);                                 // alarm gone
    CHECK(ScanIndexMotorCanMove() == true);

    // ---- 6: ServoOnOff -------------------------------------------------------------
    std::printf("-- 6. ServoOnOff\n");
    g_io.cmds.clear();
    MOT[MTestZ1].iCheckStatusCT = 0; MOT[MTestZ1].Led[iServoOn] = false;
    MOT[MTestZ1].ServoOnOff(true);
    CHECK(g_io.cmds.size() == 1 && g_io.cmds[0].kind == kCmdAxSvOn && g_io.cmds[0].value == 1.0);
    g_io.cmds.clear();
    MOT[MTestZ1].iCheckStatusCT = 0; MOT[MTestZ1].Led[iServoOn] = true;
    MOT[MTestZ1].ServoOnOff(false);
    CHECK(g_io.cmds.size() == 3 && g_io.cmds[0].kind == kCmdAxEmgStop && g_io.cmds[1].kind == kCmdAxSetExtDrive &&
          g_io.cmds[2].kind == kCmdAxSvOn && g_io.cmds[2].value == 0.0);

    // ---- 7: Gali_SingalHome ------------------------------------------------------------
    std::printf("-- 7. Gali_SingalHome (routed)\n");
    Fresh(1, 0.0, 0.0);
    Prod.TestZ1_Safe = 3000;
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    MOT[MTestZ1].iGali_SingalHomeTask = 1; MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].HomeFlag = 0;
    MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    g_io.cmds.clear();
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 5);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 8);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 200);   // servo on
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);
    CHECK(g_io.cmds.size() == 5 && g_io.cmds[4].kind == kCmdAxHome && g_io.cmds[4].homeMode == 124 && g_io.cmds[4].dir == 1);
    CHECK(g_io.cmds.size() == 5 && g_io.cmds[0].value == 300.0 && g_io.cmds[1].value == 6000.0);   // PTP = home speeds (Motor Test StartHome1203)
    CHECK(MOT[MTestZ1].MovFlag == true);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);   // stale
    Fresh(4, 0.0, 0.0);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);   // HOMING
    Fresh(1, 0.0, 0.0);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 400);   // done
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 450);
    CHECK(MOT[MTestZ1].HomeFlag == 1 && MOT[MTestZ1].MovFlag == false && MOT[MTestZ1].Motor->LastHomePos == 0);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 450);   // Gali_MotMove(TestZ1_Safe) starts
    CHECK(g_io.cmds.size() == 10 && g_io.cmds[6].value == 6000.0 && g_io.cmds[9].value == 3000.0);   // SP = PHomeHighSpeed, PAY=-3000 -> +3000
    Fresh(1, 3000.0, 3000.0);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false);         // settle 1
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 500);   // settle 2 -> 500
    CHECK(MOT[MTestZ1].Gali_SingalHome() == true && MOT[MTestZ1].iGali_SingalHomeTask == 900);    // true once
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false);         // golden: Task 900 has no case
    CHECK(MOT[MTestZ1].HomeFlag == 1);
    // servo off at 8 -> Task 1, false
    Fresh(1, 3000.0, 3000.0, 0ul);
    MOT[MTestZ1].iGali_SingalHomeTask = 1;
    MOT[MTestZ1].Gali_SingalHome(); MOT[MTestZ1].Gali_SingalHome();
    CHECK(MOT[MTestZ1].iGali_SingalHomeTask == 8);
    CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 1 && MOT[MTestZ1].Led[iServoOn] == false);

    // ---- 8: FindZPhase -----------------------------------------------------------------
    std::printf("-- 8. Gali_FindZPhase refuses\n");
    {
        const std::size_t n = g_io.cmds.size();
        CHECK(MOT[MTestZ1].Gali_FindZPhase() == false && g_io.cmds.size() == n);
    }

    // ---- 9: review #4 -- Z2's Position is not flipped --------------------------------------
    std::printf("-- 9. Gali_Two_ZAxis_Move keeps a no-card Z2's Position (review #4)\n");
    Fresh(1, 3000.0, 3000.0);
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    MOT[MTestZ2].Position = 1234;
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(600, 30000) == false);   // start
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(600, 30000) == false);   // stale
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(600, 30000) == false);   // stale
    CHECK(MOT[MTestZ2].Position == 1234);                           // was -1234 after the 3rd call (Gali_ReadPos no-card `-Position`)
    Fresh(1, 600.0, 600.0);
    MOT[MTestZ1].Gali_Two_ZAxis_Move(600, 30000);
    CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(600, 30000) == true && MOT[MTestZ2].Position == 1234);
    MOT[MTestZ2].Position = 0;

    // ---- 10: review #1 -- a stopped home stays at 250; round 2 D1 -- VS0;SP0 halts it and G22 restarts it --------
    std::printf("-- 10. a home stopped at 250 by ST is never done (review #1); halted by VS0;SP0 it restarts on DoSystem's VS/SP (D1)\n");
    for (int how = 0; how < 3; ++how) {                     // Z1's own "ST", Y1's "ST" (uhome / G04), StopAllMotor(true)
        Fresh(1, 600.0, 600.0);
        MOT[MTestZ1].iGali_SingalHomeTask = 1; MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].HomeFlag = 0;
        for (int k = 0; k < 4; ++k) MOT[MTestZ1].Gali_SingalHome();   // 1 -> 5 -> 8 -> 200 -> 250 (home started)
        CHECK(MOT[MTestZ1].iGali_SingalHomeTask == 250 && Kinds(kCmdAxHome) >= 1);
        Fresh(4, 600.0, 600.0);
        CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);   // HOMING seen
        if (how == 0)      MOT[MTestZ1].Gali_Command("ST");
        else if (how == 1) MOT[MTestY1].Gali_Command("ST");
        else               StopAllMotor(true);
        if (how < 2) CHECK(g_core.HomeCancelled() && !g_core.HomeHalted());
        else         CHECK(g_core.HomeHalted() && !g_core.HomeCancelled());
        Fresh(1, 250.0, 250.0);                             // the drive stopped short: READY after HOMING
        for (int k = 0; k < 5; ++k) CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);
        CHECK(MOT[MTestZ1].HomeFlag == 0);                  // golden: case 300 waits for MG_SC==10 -- never (ST) / not yet (SP0)
        if (how == 2) {                                     // golden HM at speed 0 goes on after START: DoSystem's one-shot VS/SP
            const int homes0 = Kinds(kCmdAxHome);
            MOT[MTestY1].Gali_Command("KS4,4,4,4;VT0.1,0.1,0.1,0.1;");   // ckernel's START string: nothing restarts
            CHECK(Kinds(kCmdAxHome) == homes0 && g_core.HomeHalted());
            MOT[MTestY1].Gali_Command("VS30000;SP10000,10000,10000,10000;");   // csystem.cpp GATE G22 (golden :4669-4670)
            CHECK(Kinds(kCmdAxHome) == homes0 + 1 && !g_core.Halted());
            Fresh(4, 250.0, 250.0);
            CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 250);   // HOMING again
            Fresh(1, 0.0, 0.0);
            CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 400);   // done
            MOT[MTestZ1].Gali_SingalHome();
            CHECK(MOT[MTestZ1].HomeFlag == 1);
        }
    }

    // ---- 11: review #2 -- halt and resume; round 2 A -- only DoSystem's VS/SP resumes, never the door-open START -----
    std::printf("-- 11. StopAllMotor(true) during Gali_MotMove; START with the door open resumes nothing; G22's VS/SP resumes\n");
    Fresh(1, 250.0, 250.0);
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    SystemStart = true;                                     // running
    g_io.cmds.clear();
    const unsigned long resumes10 = g_core.Resumes();
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);   // start
    CHECK(g_io.cmds.size() == 5 && LastMoveAbs() == 2000.0);
    Fresh(5, 2000.0, 800.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);   // PTP
    StopAllMotor(true);                                     // W906_AlarmStopLikeGolden / Pause / Yes-No: golden note.cpp:796
    SystemStart = false;                                    // ... then SystemStart=false (golden note.cpp:801)
    CHECK(g_core.Halted() && g_io.cmds.size() == 7);
    Fresh(1, 800.0, 800.0);                                 // stopped at 800 on the card
    for (int k = 0; k < 3; ++k) CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);   // MG_BG=1: in progress at speed 0 (no JAM, no arrival)
    CHECK(g_io.cmds.size() == 7 && MOT[MTestZ1].MovFlag == true);
    // START with the DOOR OPEN: ckernel.cpp:1015-1041 -- SystemStart=true, "KS4,4,4,4;VT..." to MTestY1, the safe-door check
    // fails -> SystemStart=false + StopAllMotor(); DoSystem's G22 never runs with SystemStart. (The old SystemStart resume
    // rule resumed Z1 on the KS string -- before the door check, golden ckernel.cpp:526-531 jou 20171030.)
    SystemStart = true;
    MOT[MTestY1].Gali_Command("KS4,4,4,4;VT0.1,0.1,0.1,0.1;");
    CHECK(Kinds(kCmdAxMoveAbs) == 1 && g_core.Halted());
    SystemStart = false;
    StopAllMotor();                                         // ckernel.cpp:1041 (bIndexCanStop=true: VS0;SP0 again)
    for (int k = 0; k < 3; ++k) { Fresh(1, 800.0, 800.0); CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false); }
    CHECK(Kinds(kCmdAxMoveAbs) == 1 && g_core.Halted() && g_core.Resumes() == resumes10);   // part 10's home restart is the only resume so far
    // START with the DOOR CLOSED: the KS string, the arm state machines tick (Gali_MotMove queries), then DoSystem's G22
    SystemStart = true;
    MOT[MTestY1].Gali_Command("KS4,4,4,4;VT0.1,0.1,0.1,0.1;");
    for (int k = 0; k < 2; ++k) CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);
    CHECK(Kinds(kCmdAxMoveAbs) == 1 && g_core.Halted());
    MOT[MTestY1].Gali_Command("VS30000;SP10000,10000,10000,10000;");   // csystem.cpp GATE G22 (golden :4669-4670)
    CHECK(Kinds(kCmdAxMoveAbs) == 2 && LastMoveAbs() == 2000.0 && !g_core.Halted() && g_core.Resumes() >= 1);
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);   // pending
    Fresh(5, 2000.0, 1500.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);
    Fresh(1, 2000.0, 2000.0);
    {
        const int jam0 = W906_ShowMotorErrorMessage_Count;
        const std::size_t n = g_io.cmds.size();
        CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);   // settle 1
        CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == true);    // settle 2 -> at the target
        CHECK(W906_ShowMotorErrorMessage_Count == jam0 && g_io.cmds.size() == n);
    }
    SystemStart = false;

    // ---- 12: a stop outside the route mid-move -------------------------------------------
    std::printf("-- 12. an operator stop outside the route (Motor Test STOP / pci1203 page): golden ST bookkeeping, the move is re-issued\n");
    Fresh(1, 2000.0, 2000.0);
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    g_io.cmds.clear();
    CHECK(MOT[MTestZ1].Gali_MotMove(3000, 900) == false);   // start
    Fresh(5, 3000.0, 2500.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(3000, 900) == false);   // PTP
    MOT[MTestZ2].MovFlag = true; MOT[MTestY2].MovFlag = true; MOT[MTestY1].MovFlag = true;
    MOT[MTestY1].bZ1Z2Exute = true; MOT[MTestY2].bZ1Z2Exute = true;
    CHECK(g_core.NoteForeignStop(7, false) == kGaliForeignStBookkeeping);
    CHECK(W906_GaliRouteForeignStopBookkeeping(MTestZ1) == true && MOT[MTestZ1].MovFlag == false);
    CHECK(!MOT[MTestZ2].MovFlag && !MOT[MTestY2].MovFlag && !MOT[MTestY1].MovFlag);   // golden :579-590, 4-axis
    CHECK(!MOT[MTestY1].bZ1Z2Exute && !MOT[MTestY2].bZ1Z2Exute);
    Fresh(1, 2500.0, 2500.0);                               // stopped at 2500
    CHECK(MOT[MTestZ1].Gali_MotMove(3000, 900) == false);   // MovFlag false -> golden's first call again: re-issued
    CHECK(g_io.cmds.size() == 10 && LastMoveAbs() == 3000.0);
    Fresh(1, 3000.0, 3000.0);
    MOT[MTestZ1].Gali_MotMove(3000, 900);
    CHECK(MOT[MTestZ1].Gali_MotMove(3000, 900) == true);

    // ---- 14: golden TestZ1SetPos's DP (round 2 B) ---------------------------------------------
    std::printf("-- 14. TestZ1SetPos (aTester_Front.cpp:314-320 = golden :291-297): \"DP<y1>,<-z1 encoder>\" defines Z1's command position\n");
    Fresh(1, 1234.0, 1224.0);                               // command 1234, encoder 1224 (TestZ1OutRandge)
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 1234 && MOT[MTestZ1].Gali_ReadEncoderPos() == 1224);
    g_io.cmds.clear();
    TestZ1SetPos();
    CHECK(g_io.cmds.size() == 1 && g_io.cmds[0].kind == kCmdAxSetCmdPos && g_io.cmds[0].axis == 7 && g_io.cmds[0].value == 1224.0);
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 1224);             // command = encoder now (was: the DP dropped, TD stayed 1234)
    Fresh(1, 1224.0, 1224.0);
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 1224 && MOT[MTestZ1].Gali_ReadEncoderPos() == 1224 && !g_core.Poisoned());

    // ---- 13: uninstall ------------------------------------------------------------------
    std::printf("-- 13. uninstall\n");
    W906_SetGaliRoute(0);
    CHECK(!W906_GaliRouteOwns(MTestZ1));
    {
        const std::size_t n = g_io.cmds.size();
        MOT[MTestY1].Gali_Command("ST");
        CHECK(g_io.cmds.size() == n);
    }

    // ---- 15: Teach interlock (forms/fTeach.cpp IsCanQuickJogMove) with Z2 not fitted ---------------------------------
    //AI(W906-TEACH-ZDISABLED) 20261001: EastSun 1001「如果 testz2 enable 是0 那就不要擋testz2」. The REAL TfTeach body, no route (HT9050
    //  today): MOT[MTestZ2] is the Galil arm's ENABLED TMyGALILMotor and nothing writes its HOME LED (no card, not routed). The hook
    //  stands in for WebMotorAccess.cpp MotorAccessTeachInterlockHome (its Mot_Table half is pinned in tests/test_web_motor_access.cpp):
    //  Z2 -3 = Mot_Table Enable 0 (not checked) / -2 = Enable 1 (golden LED); Z1 = a 1203 axis (1 home, -1 unknown); others 1.
    std::printf("-- 15. Teach IsCanQuickJogMove: MTestZ2 Enable=0 -> not checked; Enable=1 + LED false -> golden refusal; Z1 unknown still refuses\n");
    {
        static int z1Code = 1, z2Code = -2;
        const int sf0 = SHUTTLE_FLOODGATE;
        SHUTTLE_FLOODGATE = 0;                              // CheckShuttleCanMove (golden :5428) answers true: only the Z checks are under test
        MOT[MTestZ2].Motor->Enable = true;                  // golden's Galil arm (part 0); the route's D1 is not there on HT9050 today
        MOT[MTestZ2].MovFlag = false;
        MOT[MTestZ2].Led[iAlarmLed] = false;
        MOT[MTestZ2].Led[iHomeLed] = false;
        W906_TeachLive1203Hook = [](int) { return true; };  // a SOFT_SIMULTE build runs the body too (W906_TeachBodyInSim)
        W906_TeachHomeLedHook  = [](int mi) { return mi == MTestZ2 ? z2Code : (mi == MTestZ1 ? z1Code : 1); };
        TfTeach* t = new TfTeach();
        const int se0 = W906_ShowErrorMessage_Count;
        const int movers[2] = { MInShuttle1, MTestY1 };     // golden :1077 (shuttle) and :1112 (INDEX_MOTION_CARD==0 Index Y) both check Z2
        for (int k = 0; k < 2; ++k) {
            t->ActiveMotorIndex = movers[k];
            z1Code = 1;
            z2Code = -2;                                    // (b) Enable=1, LED false -> refused, golden's own message
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false);
            CHECK(MOT[MTestZ2].Led[iHomeLed] == false);     // Gali_ScanMotStatus ran (golden) and wrote nothing (no card, not routed)
            CHECK(W906_ShowMyMessage_Count == 1 && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ2 at home position first!!");
            z2Code = -3;                                    // (a) Mot_Table Enable=0 -> that check passes, the move is allowed
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == true);
            CHECK(W906_ShowMyMessage_Count == 0);
            z1Code = -1;                                    // (c) an enabled 1203 axis whose home is unknown still refuses (fail-closed)
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false);
            CHECK(W906_ShowMyMessage_Count == 1 && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ1 at home position first!!");
        }
        CHECK(W906_ShowErrorMessage_Count == se0);          // no floodgate prompt took part
        delete t;
        W906_TeachHomeLedHook = 0;
        W906_TeachLive1203Hook = 0;
        MOT[MTestZ2].Motor->Enable = false;                 // as D1 left it above
        SHUTTLE_FLOODGATE = sf0;
        W906_ShowMyMessage_Reset();
    }

    // ---- 16: Teach interlock for the three axes that got teach points (W906 extension, not golden) ----------------------
    //AI(W906-TEACH-3AXES) 20261001: EastSun 1001「三軸都幫我加 在合適的地方」. The REAL TfTeach body: golden has no branch for
    //  MOutShuttle1/2 or MCCDY (falls through to `return true`); the port's last check W906_Teach3AxesHomeOk (forms/fTeach.cpp EOF)
    //  makes MOutShuttle1/2 wait for MTestZ1 + MTestZ2 + every In/Out Arm Z, and MCCDY for MTestZ1 + MTestZ2, with the same home
    //  predicate (hook codes as part 15: 1 home, 0 not home, -1 unknown, -3 Enable=0 non-1203 = not checked). Other axes unchanged.
    std::printf("-- 16. Teach IsCanQuickJogMove: MOutShuttle1/2 need Index Z + In/Out Arm Z at home; MCCDY needs Index Z; others unchanged\n");
    {
        static std::map<int, int> code;                     // MOT index -> hook code (absent = 1, at home)
        code.clear();
        const int sf0 = SHUTTLE_FLOODGATE;
        SHUTTLE_FLOODGATE = 0;                              // CheckShuttleCanMove answers true: only the Z checks are under test
        W906_TeachLive1203Hook = [](int) { return true; };
        W906_TeachHomeLedHook  = [](int mi) { std::map<int, int>::const_iterator it = code.find(mi); return it == code.end() ? 1 : it->second; };
        TfTeach* t = new TfTeach();
        const int se0 = W906_ShowErrorMessage_Count;
        const int inZ = InArmZIndex[0][0], outZ = OutArmZIndex[MAX_ARM_Row - 1][MAX_ARM_Col - 1];
        const int shuttles[2] = { MOutShuttle1, MOutShuttle2 };
        for (int k = 0; k < 2; ++k) {
            t->ActiveMotorIndex = shuttles[k];
            code.clear(); code[MTestZ2] = -3;               // HT9050: Z2 not fitted (Enable=0) -> not checked
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == true && W906_ShowMyMessage_Count == 0);           // everything at home
            code[MTestZ1] = -1;                             // Index Z1 home unknown -> refused (fail-closed)
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false);
            CHECK(W906_ShowMyMessage_Count == 1 && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ1 at home position first!!");
            code[MTestZ1] = 1; code[MTestZ2] = 0;           // a fitted Z2 (1203) not at home -> refused
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ2 at home position first!!");
            code[MTestZ2] = -3; code[inZ] = 0;              // an In Arm Z not at home -> refused
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let InArm Z at home position first!!");
            code[inZ] = 1; code[outZ] = -1;                 // an Out Arm Z unknown -> refused
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == false && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let OutArm Z at home position first!!");
            code[outZ] = 1;                                 // back at home -> allowed (a 1203 code: no golden ScanMotorStatus on the test's motor objects)
            W906_ShowMyMessage_Reset();
            CHECK(t->IsCanQuickJogMove() == true && W906_ShowMyMessage_Count == 0);
        }
        t->ActiveMotorIndex = MCCDY;
        code.clear(); code[MTestZ2] = -3; code[inZ] = 0; code[outZ] = 0;   // the arm Z are not MCCDY's business
        W906_ShowMyMessage_Reset();
        CHECK(t->IsCanQuickJogMove() == true && W906_ShowMyMessage_Count == 0);
        code[MTestZ1] = 0;                                  // Index Z1 not at home -> refused
        W906_ShowMyMessage_Reset();
        CHECK(t->IsCanQuickJogMove() == false && W906_ShowMyMessage_Count == 1 &&
              std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ1 at home position first!!");
        code[MTestZ1] = 1; code[MTestZ2] = -1;              // a fitted Z2 unknown -> refused
        W906_ShowMyMessage_Reset();
        CHECK(t->IsCanQuickJogMove() == false && std::string(W906_ShowMyMessage_LastS1.c_str()) == "Please let MTestZ2 at home position first!!");
        t->ActiveMotorIndex = MTrayZ;                       // an axis with no golden branch: still golden's `return true`
        code.clear(); code[MTestZ1] = -1; code[inZ] = 0;
        W906_ShowMyMessage_Reset();
        CHECK(t->IsCanQuickJogMove() == true && W906_ShowMyMessage_Count == 0);
        CHECK(W906_Teach3AxesHomeOk(MOutArmX) == true && W906_Teach3AxesHomeOk(-1) == true);
        CHECK(W906_ShowErrorMessage_Count == se0);          // no floodgate prompt took part
        delete t;
        W906_TeachHomeLedHook = 0;
        W906_TeachLive1203Hook = 0;
        SHUTTLE_FLOODGATE = sf0;
        W906_ShowMyMessage_Reset();
    }

    // ---- 17: ORG polarity (AI(W906-INDEXZ2) 20261002) -----------------------------------------------------------------
    //  The tree's rule for "is this 1203 axis at home" lives in the engine hook W906_Ht9050OrgHome (Motor/mymotor.h; on the
    //  machine WebMotorAccessLive.cpp W906_HookHt9050OrgHome = WebMotorAccess.cpp MotorAccessTeachHomeLed): on HT9050 the ORG
    //  bit is read by the axis's Mot_Table SensorType (AI(W906-HT9050-ORG-ST): 1 = LOW at home, 0 = HIGH at home; golden
    //  InitMotor's uOrgLogic; M14 has SensorType 1). The fake binding asks the hook exactly as EtherCAT/Pci1203GaliRoute.cpp
    //  LiveIo::OrgHome does; the stand-in hook applies that rule to the fake sample and MOT[MTestZ1].Motor->bSensorType.
    //  golden's own Gali_ScanMotStatus (every routed Gali_* move and the delegated home call it; readers such as
    //  aTester_Front.cpp:1153 take whatever it left) and TMyMotor::ScanMotorStatus (+ the hook's override) must give Z1 the
    //  SAME lamp, for either SensorType. Before 20261002 the route's TS byte said "ORG bit set = at home" for every axis.
    std::printf("-- 17. ORG polarity: the route's HOME lamp = the tree's rule (per axis by SensorType); Gali_ScanMotStatus and ScanMotorStatus agree\n");
    {
        g_route.owner = MTestZ1;
        g_core.Bind(&g_io);
        W906_SetGaliRoute(&g_route);                        // part 13 uninstalled it
        MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].iCheckStatusCT = 0;
        const bool st0 = MOT[MTestZ1].Motor->bSensorType;
        W906_Ht9050OrgHomeHook = [](int mi) {               // stand-in for MotorAccessTeachHomeLed's HT9050 arm
            if (mi != MTestZ1) return -2;
            const bool orgHigh = (g_io.s.motionIO & 0x10ul) != 0;
            return (MOT[MTestZ1].Motor->bSensorType ? !orgHigh : orgHigh) ? 1 : 0;
        };
        const unsigned long lo = 0x00004000ul, hi = 0x00004000ul | 0x10ul;   // SVON, ORG bit low / high
        for (int stype = 1; stype >= 0; --stype) {
            MOT[MTestZ1].Motor->bSensorType = (stype == 1);   // 1 = machines/HT9050/Mot_Table.csv M14; 0 = the other polarity
            const unsigned long home = stype ? lo : hi, away = stype ? hi : lo;
            Fresh(1, 0.0, 0.0, home);
            MOT[MTestZ1].Led[iHomeLed] = false;
            MOT[MTestZ1].Gali_ScanMotStatus();
            CHECK(MOT[MTestZ1].Led[iHomeLed] == true);      // golden's decode of the route's TS byte
            MOT[MTestZ1].ScanMotorStatus();
            CHECK(MOT[MTestZ1].Led[iHomeLed] == true);      // + the hook's override: the same lamp
            CHECK((MOT[MTestZ1].Gali_Command("TSY") & 0x02) == 0);
            Fresh(1, 0.0, 0.0, away);
            MOT[MTestZ1].Gali_ScanMotStatus();
            CHECK(MOT[MTestZ1].Led[iHomeLed] == false);
            MOT[MTestZ1].ScanMotorStatus();
            CHECK(MOT[MTestZ1].Led[iHomeLed] == false);
            CHECK((MOT[MTestZ1].Gali_Command("TSY") & 0x02) != 0);
        }
        MOT[MTestZ1].Motor->bSensorType = st0;
        W906_Ht9050OrgHomeHook = [](int) { return -1; };    // the tree cannot tell -> not at home (fail-closed), whatever the bit
        Fresh(1, 0.0, 0.0, lo);
        MOT[MTestZ1].Gali_ScanMotStatus();
        CHECK(MOT[MTestZ1].Led[iHomeLed] == false);
        Fresh(1, 0.0, 0.0, hi);
        MOT[MTestZ1].Gali_ScanMotStatus();
        CHECK(MOT[MTestZ1].Led[iHomeLed] == false);
        W906_Ht9050OrgHomeHook = 0;                         // no hook (not HT9050; parts 0-16): golden EtherCAT's decode, bit set = at home
        Fresh(1, 0.0, 0.0, hi);
        MOT[MTestZ1].Gali_ScanMotStatus();
        CHECK(MOT[MTestZ1].Led[iHomeLed] == true);          // = part 5
        Fresh(1, 0.0, 0.0, lo);
        MOT[MTestZ1].Gali_ScanMotStatus();
        CHECK(MOT[MTestZ1].Led[iHomeLed] == false);
        W906_SetGaliRoute(0);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_checks - g_fail, g_checks);
    return g_fail ? 1 : 0;
}
