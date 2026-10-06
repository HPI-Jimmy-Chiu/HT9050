// =============================================================================
//  tests/test_idx1203_gali_branch.cpp -- AI(W906-IDX1203) 20261006
//
//  The Index axis on the PCIE-1203 CLASS (Motor/GaliRoute.h end, Motor/myGALILmotor.cpp end; plan
//  D:\HT9045\_idxmove_1006\PLAN_IDX1203_CLASS.md).  EastSun 1006: Q130 "INDEX_MOTION_CARD 改成非 0（用 MyMotor）",
//  「gali 是不是有單獨class ? 可以用1203 class 分支?」, 「你可以統一 index 寫成一個移動函式 裡面再分支呼叫?」.
//
//  MOT[MTestZ1] gets a fake 1203-class motor (a TMySimMotor whose motion, alarm, servo and encoder the test drives)
//  with CardType "PCI1203"; MTestY1 / MTestZ2 / MTestY2 are disabled (HT9050's Enable 0 rows).  Checked:
//    0. INDEX_MOTION_CARD = 0 -> nothing branches (golden Galil Index, every machine today);
//    1. INDEX_MOTION_CARD = 1 -> Z1 is a 1203-class axis, Y1 / Z2 / Y2 are "absent = done at once";
//    2. Gali_MotMove = W906_IndexMove: speed to the class, one MoveToPos, not arrived while moving, arrived on done,
//       MovFlag / IndexZCanMove as golden; a new target re-issues;
//    3. the encoder range check on arrival -> golden JAM (iHome = 1), never an arrival;
//    4. NoWait / SkipEncoder / Two_ZAxis_Move (Z1 receiver) take the same function;
//    5. Gali_ReadPos / Gali_ReadEncoderPos read the class;
//    6. Gali_Command strings: ST / AB / VS0;SP0 stop, SH / MO servo, DP = command := encoder, TP / TD / MG_BG / TI answers;
//       a raw Galil motion string moves nothing;
//    7. the 1203 class's own Index special cases: PCIL132_StopMotor stops Z1, GetMotorAlarm reads the drive, SetSpeed reaches it;
//    8. Gali_ScanMotStatus / ScanAlarmStatus: lamps from the class, golden's alarm latch only while moving;
//    9. Gali_SingalHome: the class's MotorHome inside golden's task numbers, HomeFlag, the safe move, Task 900;
//   10. Gali_FindZPhase refuses.
//  ShowMotorErrorMessage is the golden body (InitialOK=false: stop half + Exception record) -> ctest redirects only.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                 // INDEX_MOTION_CARD / USE_INDEX_ARM_AXES / iHome / InitialOK / MTest* / SystemStart
#include "Config.h"                 // IniConfig.GaliPosRange
#include "CosFunction.h"            // CosFunction.bIndexProtect
#include "cprod.h"                  // Prod.TestZ1_Safe
#include "canary_support.h"         // W906_ShowMotorErrorMessage_Count
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/HTMotor.h"          // iServoOn / iInposLed / iAlarmLed ...
#include "Motor/mySimMotor.h"
#include "Motor/myGALILmotor.h"     // StopAllMotor / ScanIndexMotorCanMove
#include "Motor/GaliRoute.h"        // the IDX1203 API
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>

extern bool IndexZCanMove[2];       // ainarm9045_w7_shims.cpp

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_idx1203_gali_branch.cpp:%d]  %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- a 1203-class motor the test drives ---------------------------------------
struct FakeIdx : TMySimMotor {
    int  cmd, act;                  // command / encoder (pulses; GearRatio 1)
    bool done, alarm, servo;
    int  moves, lastTar, stops, resets, lastReset, servoOn, servoOff;
    unsigned speed;
    FakeIdx() : cmd(0), act(0), done(true), alarm(false), servo(true), moves(0), lastTar(0), stops(0), resets(0), lastReset(-1),
                servoOn(0), servoOff(0), speed(0) {}
    int  ReadPos() override            { return cmd; }
    int  ReadRealPos() override        { return cmd; }
    int  ReadEnCoderRealPos() override { return act; }
    bool MoveToPos(int Tar) override   { ++moves; lastTar = Tar; done = false; return false; }
    bool MotionDone() override         { return done; }
    bool GetAlarm(void) override       { return alarm; }
    void Stop() override               { ++stops; done = true; }
    void DecStop(void) override        { ++stops; done = true; }
    void SetSpeed(unsigned int x, bool) override { speed = x; iSpeed = x; }
    void SetServoOn(bool on) override  { servo = on; if (on) ++servoOn; else ++servoOff; }
    bool ResetPos(int p) override      { ++resets; lastReset = p; cmd = p; return true; }
    bool HomeObject() override         { cmd = 0; act = 0; done = true; return true; }
    bool HomeFlag(void) override       { return true; }
    void ScanMotorStatus(bool* Led) override
    {
        if (!Led) return;
        Led[iCwLed] = false; Led[iCcwLed] = false; Led[iEmgLed] = false; Led[iSoftcwLed] = false; Led[iSoftccwLed] = false;
        Led[iHomeLed] = (cmd == 0);
        Led[iInposLed] = !done;                                     // golden's "in position" lamp reads TRUE while the axis is still busy (Galil TS bit 7; MotorMovePosition waits on it)
        Led[iAlarmLed] = alarm; Led[iServoalarmLed] = alarm;
        Led[iServoOn] = servo;
    }
    void Arrive() { cmd = lastTar; act = lastTar; done = true; }
};

static bool DoorClosed() { return false; }

int main()
{
    if (!W906TestRequireCtestRedirects("Idx1203GaliBranch")) return 2;
    std::printf("=== test_idx1203_gali_branch (AI(W906-IDX1203)) ===\n");
    USE_INDEX_ARM_AXES = IndexArm_4_Axis;                   // HT9050 Gerneral.ini
    MOTION_CARD_TYPE = 0;                                   // HT9050 Gerneral.ini (PCIL132_ResetPos takes the encoder arm)
    IniConfig.GaliPosRange = 50;                            // D:\HT9045\config\config.ini value
    CosFunction.bIndexProtect = false;
    InitialOK = false;                                      // golden ShowMotorErrorMessage: stop half, Exception record, early return
    SystemStart = false;
    W906_TestEnsureSimMotors();                             // every MOT[].Motor non-NULL
    FakeIdx* Z = new FakeIdx();
    const int idx[4] = { MTestY1, MTestZ1, MTestZ2, MTestY2 };
    const char* names[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {
        const int i = idx[k];
        if (i == MTestZ1) MOT[i].Motor = Z;
        MOT[i].SetAlias(i, names[k]);
        MOT[i].Motor->Enable = (i == MTestZ1);              // HT9050: M13 / M15 / M16 Enable 0 in the Mot_Table
        MOT[i].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
        MOT[i].Motor->PServoAlarmOn = true;
        MOT[i].Motor->GearRatio = 1.0;
        MOT[i].Motor->PSoftLimitP = 999999; MOT[i].Motor->PSoftLimitN = -999999;
        for (int l = 0; l < 10; ++l) MOT[i].Led[l] = false;
        MOT[i].MovFlag = false; MOT[i].fCMD = false;
    }
    MOT[MTestZ1].CardType = "PCI1203";                      // cinitial.cpp: the M14 row's CardModel
    Z->PJogHighSpeed = 90000; Z->InitSpeed = 10000; Z->PHomeHighSpeed = 20000; Z->PHomeLowSpeed = 1000;
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    W906_SetGaliRoute(0);

    // ---- 0: INDEX_MOTION_CARD = 0 ------------------------------------------------
    std::printf("-- 0. INDEX_MOTION_CARD=0 = golden Galil Index: nothing branches\n");
    INDEX_MOTION_CARD = 0;
    CHECK(!W906_IdxIs1203(MOT[MTestZ1]) && !W906_Idx1203Machine());
    { long r = 7; CHECK(!W906_Idx1203Command(MTestY1, "ST", &r) && r == 7); }
    CHECK(!W906_GaliRouteAbsentAxis(MTestZ2));               // no route, no 1203 class: golden
    MOT[MTestZ1].Gali_Command("ST");
    CHECK(Z->stops == 0);                                    // golden no-card "ST": nothing reaches the drive

    // ---- 1: INDEX_MOTION_CARD = 1 ------------------------------------------------
    std::printf("-- 1. INDEX_MOTION_CARD=1: Z1 on the 1203 class, Y1 / Z2 / Y2 absent\n");
    INDEX_MOTION_CARD = 1;
    CHECK(W906_IdxIs1203(MOT[MTestZ1]) && W906_Idx1203Machine());
    CHECK(!W906_IdxIs1203(MOT[MTestZ2]) && !W906_IdxIs1203(MOT[MInArmX]));
    CHECK(W906_GaliRouteAbsentAxis(MTestY1) && W906_GaliRouteAbsentAxis(MTestZ2) && W906_GaliRouteAbsentAxis(MTestY2) && !W906_GaliRouteAbsentAxis(MTestZ1));
    CHECK(MOT[MTestZ2].Gali_MotMove(500, 900) == true && IndexZCanMove[0] == true && Z->moves == 0);   // absent: done at once, Z1 not frozen
    CHECK(MOT[MTestY1].GalilTwoY_Move(2000, 3000, 900) == true && Z->moves == 0);
    MOT[MTestZ1].CardType = "SMC";
    CHECK(!W906_IdxIs1203(MOT[MTestZ1]));                    // a non-1203 row stays golden
    MOT[MTestZ1].CardType = "PCI1203";

    // ---- 2: Gali_MotMove = W906_IndexMove ----------------------------------------
    std::printf("-- 2. Gali_MotMove -> the 1203 class\n");
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 500) == false);   // sent, moving
    CHECK(Z->moves == 1 && Z->lastTar == 1000 && Z->speed == 500u);
    CHECK(MOT[MTestZ1].MovFlag == true && MOT[MTestZ1].TargetPosition == 1000 && IndexZCanMove[1] == false);
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 500) == false && Z->moves == 1);   // still moving: no second command
    Z->Arrive();
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 500) == true);    // arrived
    CHECK(MOT[MTestZ1].MovFlag == false && IndexZCanMove[1] == true && Z->moves == 1);
    CHECK(MOT[MTestZ1].Gali_MotMove(1000, 500) == true && Z->moves == 1);    // already there: true, nothing sent
    // a new target while the old move runs: golden MotorMove waits for the running move to finish, then issues the new one
    CHECK(MOT[MTestZ1].Gali_MotMove(3000, 500) == false && Z->moves == 2 && Z->lastTar == 3000);
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 400) == false && Z->moves == 2 && Z->speed == 400u);   // the old move still runs: nothing sent yet
    Z->Arrive();                                                                                // the 3000 move ends
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 400) == false && Z->moves == 3 && Z->lastTar == 2000);
    Z->Arrive();
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 400) == true);
    // the Z1 / Z2 interlock
    IndexZCanMove[0] = false;
    CHECK(MOT[MTestZ1].Gali_MotMove(0, 500) == false && Z->moves == 3);
    IndexZCanMove[0] = true;

    // ---- 3: encoder range on arrival -> JAM ----------------------------------------
    std::printf("-- 3. encoder short on arrival -> golden JAM, never an arrival\n");
    {
        const int jam0 = W906_ShowMotorErrorMessage_Count;
        iHome = 0;
        CHECK(MOT[MTestZ1].Gali_MotMove(20000, 500) == false);
        Z->cmd = 20000; Z->act = 5000; Z->done = true;       // command there, encoder 15000 short
        CHECK(MOT[MTestZ1].Gali_MotMove(20000, 500) == false);
        CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1 && iHome == 1 && MOT[MTestZ1].MovFlag == false);
        iHome = 0; Z->act = Z->cmd; IndexZCanMove[0] = IndexZCanMove[1] = true;
    }

    // ---- 4: the other move modes -----------------------------------------------
    std::printf("-- 4. NoWait / SkipEncoder / Two_ZAxis_Move(Z1) take the same function\n");
    {
        const int m0 = Z->moves;
        CHECK(MOT[MTestZ1].Gali_MotMoveNoWait(5000, 800, 0) == false && Z->moves == m0 + 1 && Z->lastTar == 5000);
        Z->Arrive();
        CHECK(MOT[MTestZ1].Gali_MotMoveNoWait(5000, 800, 0) == true);
        CHECK(MOT[MTestZ1].Gali_MotMoveSkipEncoder(6000, 800) == false && Z->lastTar == 6000);
        Z->cmd = 6000; Z->act = 0; Z->done = true;           // encoder far off: SkipEncoder does not check it
        CHECK(MOT[MTestZ1].Gali_MotMoveSkipEncoder(6000, 800) == true);
        Z->act = Z->cmd;
        CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(700, 30000) == false && Z->lastTar == 700 && Z->speed == 30000u);
        Z->Arrive();
        CHECK(MOT[MTestZ1].Gali_Two_ZAxis_Move(700, 30000) == true);
        const int m1 = Z->moves;
        CHECK(MOT[MTestZ2].Gali_Two_ZAxis_Move(900, 100) == true && Z->moves == m1);   // receiver Z2 absent: golden instant true, Z1 not moved
        CHECK(W906_IndexMove(MOT[MTestZ1], 800, 500, kIdxMoveWait, "test") == false && Z->lastTar == 800);   // the unified function itself
        Z->Arrive();
        CHECK(W906_IndexMove(MOT[MTestZ1], 800, 500, kIdxMoveWait, "test") == true);
        MOT[MTestZ1].MovFlag = false;
    }

    // ---- 5: reads ------------------------------------------------------------------
    std::printf("-- 5. Gali_ReadPos / Gali_ReadEncoderPos read the class\n");
    Z->cmd = 1234; Z->act = 1230;
    CHECK(MOT[MTestZ1].Gali_ReadPos() == 1234 && MOT[MTestZ1].Gali_ReadEncoderPos() == 1230);
    CHECK(MOT[MTestZ1].Gali_ReadEncoderInRandge(1234) == true && MOT[MTestZ1].Gali_ReadEncoderInRandge(5000) == false);

    // ---- 6: Galil strings ------------------------------------------------------------
    std::printf("-- 6. Gali_Command strings on the 1203 class\n");
    {
        int s0 = Z->stops;
        MOT[MTestZ1].MovFlag = true; MOT[MTestZ1].fCMD = true;
        MOT[MTestY1].Gali_Command("ST");                     // golden: "ST" through Y1 stops every Index axis
        CHECK(Z->stops == s0 + 1 && MOT[MTestZ1].MovFlag == false && MOT[MTestZ1].fCMD == false);
        MOT[MTestY1].Gali_Command("VS0;SP0,0,0,0;");         // golden halt
        CHECK(Z->stops == s0 + 2);
        MOT[MTestY1].Gali_Command("AB1");
        CHECK(Z->stops == s0 + 3);
        MOT[MTestZ2].Gali_Command("STZ");                    // another axis's stop: Z1 untouched
        CHECK(Z->stops == s0 + 3);
        MOT[MTestY1].Gali_Command("MOY");  CHECK(Z->servo == false && Z->servoOff == 1);
        MOT[MTestY1].Gali_Command("SHY");  CHECK(Z->servo == true && Z->servoOn == 1);
        Z->cmd = 400; Z->act = 410;
        MOT[MTestY1].Gali_Command("DPY=410");                // golden sets the position: PCIL132_ResetPos (command := encoder; this fake is no TMyEtherCatMotor,
        CHECK(Z->resets == 1);                               //   so it takes the MotionCard_SYN arm, ResetPos() -- on the machine the 1203 arm writes the encoder)
        CHECK(MOT[MTestY1].Gali_Command("TPY") == 410);      // TP = + actual (as the Galil route)
        CHECK(MOT[MTestY1].Gali_Command("TDY") == -Z->cmd);  // TD = - command
        Z->done = false; CHECK(MOT[MTestY1].Gali_Command("MG_BGy") == 1);
        Z->done = true;  CHECK(MOT[MTestY1].Gali_Command("MG_BGy") == 0);
        Z->alarm = true;  CHECK(MOT[MTestY1].Gali_Command("TI") == 0x08);
        Z->alarm = false; CHECK(MOT[MTestY1].Gali_Command("TI") == 0);
        const int m0 = Z->moves;
        MOT[MTestY1].Gali_Command("SPY=100;PAY=-5000;BGY;");  // a raw Galil motion string moves nothing
        CHECK(Z->moves == m0);
    }

    // ---- 7: the 1203 class's own Index special cases ----------------------------
    std::printf("-- 7. PCIL132_StopMotor / GetMotorAlarm / SetSpeed reach the 1203 Z1\n");
    {
        const int s0 = Z->stops;
        MOT[MTestZ1].fCMD = true;
        MOT[MTestZ1].PCIL132_StopMotor();
        CHECK(Z->stops == s0 + 1 && MOT[MTestZ1].fCMD == false);
        Z->alarm = true;  CHECK(MOT[MTestZ1].GetMotorAlarm() == true);
        Z->alarm = false; CHECK(MOT[MTestZ1].GetMotorAlarm() == false);
        Z->speed = 0; MOT[MTestZ1].SetSpeed(50);
        CHECK(Z->speed != 0u);                               // the Index name no longer swallows the speed
    }

    // ---- 8: status ----------------------------------------------------------------
    std::printf("-- 8. Gali_ScanMotStatus / ScanAlarmStatus: the class's lamps, golden's latch only while moving\n");
    Z->alarm = true; MOT[MTestZ1].MovFlag = false;
    MOT[MTestZ1].Gali_ScanMotStatus();
    CHECK(MOT[MTestZ1].Led[iAlarmLed] == true && MOT[MTestZ1].Gali_MotorAlarm == false);
    MOT[MTestZ1].MovFlag = true;
    MOT[MTestZ1].Gali_ScanAlarmStatus();
    CHECK(MOT[MTestZ1].Gali_MotorAlarm == true);
    Z->alarm = false; MOT[MTestZ1].MovFlag = false;
    MOT[MTestZ1].Gali_ScanMotStatusTIMO();
    CHECK(MOT[MTestZ1].Led[iAlarmLed] == false && MOT[MTestZ1].Gali_MotorAlarm == false && MOT[MTestZ1].Led[iServoOn] == true);

    // ---- 9: Gali_SingalHome ------------------------------------------------------
    std::printf("-- 9. Gali_SingalHome: MotorHome inside golden's task numbers\n");
    {
        extern int (*g_W906PreHomeHook)(int, char*, int);   // Motor/mymotor.cpp: WebMotorAccessLive registers the stepper "leave the origin" step; no live backend here
        g_W906PreHomeHook = 0;
        Z->cmd = 3000; Z->act = 3000; Z->done = true;
        MOT[MTestZ1].HomeFlag = 0;
        MOT[MTestZ1].iGali_SingalHomeTask = 1;
        IndexZCanMove[0] = IndexZCanMove[1] = true;
        bool ok = false;
        for (int k = 0; k < 200 && !ok; ++k) {
            ok = MOT[MTestZ1].Gali_SingalHome();
            if (k < 12 || ok) std::printf("    home call %d: Task %d, ok %d, HomeFlag %d, cmd %d, act %d, done %d, moves %d\n", k, MOT[MTestZ1].iGali_SingalHomeTask, (int)ok, MOT[MTestZ1].HomeFlag, Z->cmd, Z->act, (int)Z->done, Z->moves);
            if (!Z->done) Z->Arrive();                       // the safe move (Task 450) arrives on the next call
        }
        CHECK(ok && MOT[MTestZ1].HomeFlag == 1 && MOT[MTestZ1].iGali_SingalHomeTask == 900 && Z->cmd == 0);
        CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 900);   // done: nothing until re-armed
        Z->servo = false; MOT[MTestZ1].iGali_SingalHomeTask = 1;
        CHECK(MOT[MTestZ1].Gali_SingalHome() == false && MOT[MTestZ1].iGali_SingalHomeTask == 1);    // golden case 8: servo off -> back to 1
        Z->servo = true;
    }

    // ---- 10: Gali_FindZPhase ----------------------------------------------------
    std::printf("-- 10. Gali_FindZPhase refuses (D63 is Galil-only)\n");
    CHECK(MOT[MTestZ1].Gali_FindZPhase() == false);

    INDEX_MOTION_CARD = 0;
    std::printf("=== %d checks, %d failed ===\n", g_checks, g_fail);
    if (g_fail == 0) std::printf("ALL PASS\n");
    return g_fail == 0 ? 0 : 1;
}
