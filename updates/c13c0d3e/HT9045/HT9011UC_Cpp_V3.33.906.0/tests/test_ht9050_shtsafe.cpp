// =============================================================================
//  test_ht9050_shtsafe.cpp  --  AI(W906-HT9050-SHTSAFE) 20261004: In Shuttle 1 may go to the Index only with
//  Out Shuttle 1 at Right; otherwise Out Shuttle 1 is cleared to Right (Out Arm ZA at home, Index Z1 safe,
//  Out Shuttle 2 at Left FIRST).  ctest HT9050_ShtSafe.  Memory only (sim motors, no file, no card).
//
//  EastSun 1004:
//    「自動流程中，In Shuttle 1 往 Index 走之前，先確認 Out Shuttle 1 在 Right、而且沒在動 -> 不符合請判斷Out Arm Z軸是否在Home點，
//     若可以請將Out Shuttle1移至右邊，移動前請確認Out Shuttle2是否在Tech.iOutShuttle2Left上 ... 進出接需判斷」
//    rule 9 = (A) Out Shuttle 2 moved to Left automatically, (B) only after the Out Arm ZA is really at home.
//  Under test (acarry.cpp): W906_Ht9050OutSht1NotClearShuttle1CanNotMove, W906_Ht9050OutSht1ToRight, W906_Ht9050OutSht2ToLeft.
//  ORACLES (hand-derived from the ruling, not from the code):
//    [G] not HT9050 (no origin hook = -2): the guard answers false and moves nothing (golden).
//    [R] HT9050, Out Shuttle 1 at Right and still: false (In Shuttle 1 may go).
//    [Z] HT9050, Out Shuttle 1 at Left, Out Arm ZA NOT at home: true; In Shuttle 1 stopped; Out Shuttle 1 / 2 do not move.
//    [S] HT9050, ZA at home, Out Shuttle 2 at Right: Out Shuttle 2 reaches Left BEFORE Out Shuttle 1 leaves Left;
//        then Out Shuttle 1 reaches Right; then the guard answers false.
//    [E] Out Shuttle 2 / Out Arm ZA with Enable=0 count as at Left / at home (as the full HOME's W906_AxisOff).
//    [P] source pins: the guard sits after IsTestZ1NotSafeShuttle1CanNotMove at the five In Shuttle 1 moves toward the
//        Index (acarry.cpp), the HOME's last step moves Out Shuttle 2 to Prod.OutSHT[1].iLeft (uhome.cpp), and the
//        disabled-fixer skip (ainarm9045.cpp) -- argv[1] = source root, read only.
//  Index Z1 is Enable=false here (its encoder read goes through the Galil route); the Z1 arm is the same test as
//  IsTestZ1NotSafeShuttle1CanNotMove and is not exercised.
//
//  AI(W906-FRNB2-2) 20261006: [W] FR-NB2 (2), the shuttle side of W-44 (RULINGS_20261006 #11; spec motion-priority-interlock §4.1 /
//  §5.2; Frank 1006 15:3x "1A2A3A"), on the machine's directions (In Shuttle 1 Left 0 / Right -69123, Out Shuttle 1 Right 0 /
//  Left 91850). ORACLES (from the rulings): a shuttle is clear of the socket while it is on its home side of home + 1000 toward
//  the socket (2A; beyond home always clear; Left == Right not clear); moving counts only if the commanded target is clear too;
//  St01's hook is installed at start-up; In Shuttle 1 into the Index needs Out Shuttle 1 in ITS SAFE ZONE (1A, not only at Right)
//  and the Index Z1 at its safe height (+-10); Out Shuttle 1 into the Index needs In Shuttle 1 in its safe zone and Z1 safe;
//  every Out Shuttle 1 / Out Shuttle Y move needs M108 at home = HOME lamp AND encoder within +-100 (3A; Enable 0 = home);
//  Out Shuttle Y to Left needs Out Shuttle 1 at Right (spec M18 (1)) on Type_HT9050 only.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cprod.h"                  // Prod
#include "cmydef.h"                 // MInShuttle1 / MOutShuttle1 / MOutShuttle2 / MOutArmZA / MTestZ1 / MTrayX / iHomeLed
#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[] / W906_Ht9050OrgHomeHook
#include "Motor/mySimMotor.h"
#include "w906_test_motors.h"       // W906_TestEnsureSimMotors
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

bool W906_Ht9050OutSht1NotClearShuttle1CanNotMove();   // acarry.cpp
bool W906_Ht9050OutSht1ToRight(std::string *why);      // acarry.cpp
bool W906_Ht9050OutSht2ToLeft(std::string *why);       // acarry.cpp
bool W906_Ht9050InSht1ToIndexBlocked(int &iRetryCT);   // acarry.cpp (AI(W906-HT9050-SHTSAFE-2): Do_Auto_InSH case 200)
bool W906_Ht9050OutSht1MoveBlocked(int target);        // acarry.cpp (AI(W906-HT9050-SHTSAFE-2): Do_Auto_OutSH cases 100 / 200 / 201)
bool W906_Ht9050OutShtYToRight(std::string *why);      // acarry.cpp (AI(W906-F9050-FIX2))
bool W906_ShtPosInSafeZone(int pos, int home, int socket);              // acarry.cpp (AI(W906-FRNB2-2))
bool W906_Ht9050ShuttlesClearOfIndex(AnsiString *why);                  // acarry.cpp (AI(W906-FRNB2-2))
extern bool (*W906_Ht9050ShuttlesClearOfIndexHook)(AnsiString* why);    // atester_FinePitch.cpp (St01)
struct TMovingSimMotor : public TMySimMotor                             // [W4]: a sim axis that reports "still moving"
{
    bool moving = false;
    bool MotionDone() override { return !moving; }
};
#include "mycylin.h"                                    // Cylinder[] (C_OutArmSmallY, rule 10)
extern int  g_W906OutArmRule10;                         // Motor/mymotor.cpp (AI(W906-HT9050-RULE10)): 1 = to Auto1, 2 = to the Out Shuttle pick
extern int  iPCIL112_OutArmXYMoveTask;
int PCIL112_OutArmXYMove(int iXComPos, int iYComPos);
#ifndef PNP_DONE
#define PNP_DONE  0                                     // Motor/mymotor.cpp:78-79 (file-local there)
#define PNP_DOING 1
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool DoorClosed() { return false; }   // PF_CHECK (Motor/HTMotor.h), as tests/test_flow9050_shuttle.cpp

static const int OUT1_L = 90000, OUT1_R = 5000;
static const int OUT2_L = 20000, OUT2_R = 40000;
static const int IN_R   = 60000;

static int g_zaHome = 1;
static int g_ccdHome = 1;                                                  // [W]: M108 (MCCDY)'s HOME lamp
static int HookHT9050(int mi) { return mi == MOutArmZA ? g_zaHome : (mi == MCCDY ? g_ccdHome : 1); }   // != -2 = HT9050; ZA's / M108's HOME lamp from the test

static void SetMotPos(int m, int p)
{
    MOT[m].Position = p;
    if (MOT[m].Motor != NULL) MOT[m].Motor->SetPosition(p);
    MOT[m].fCMD = false;
}
static int Pos(int m) { return MOT[m].Motor->ReadPos(); }
static void World()
{
    Prod.OutSHT[0].iLeft = OUT1_L; Prod.OutSHT[0].iRight = OUT1_R;
    Prod.OutSHT[1].iLeft = OUT2_L; Prod.OutSHT[1].iRight = OUT2_R;
    for (int m : { (int)MInShuttle1, (int)MOutShuttle1, (int)MOutShuttle2, (int)MOutArmZA }) {
        MOT[m].Motor->Enable = true;
        MOT[m].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
        MOT[m].fCanMove = MOT[m].fCanMoveR = MOT[m].fCanMoveM = MOT[m].fCanMoveL = true;
    }
    MOT[MTestZ1].Motor->Enable = false;
    MOT[MTestY1].Motor->Enable = false;
    SetMotPos(MInShuttle1, 0); SetMotPos(MOutShuttle1, OUT1_L); SetMotPos(MOutShuttle2, OUT2_R);
    g_zaHome = 1; g_ccdHome = 1;
}
static std::string ReadFile(const std::string &p)
{
    std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str();
}
static int Count(const std::string &hay, const std::string &needle)
{
    int n = 0; for (size_t i = hay.find(needle); i != std::string::npos; i = hay.find(needle, i + 1)) ++n; return n;
}

static bool db_center_pin(const std::string &root)   // AI(W906-HT9050-SHTCENTER): the four centre offsets are 0 inside the 9050GPIB test
{
    const std::string db = ReadFile(root + "/database.cpp");
    const size_t k = db.find("if(W906_GpibModel==\"9050GPIB\")\r\n        {\r\n            iInArmShtXCenterPos=0;");
    const size_t k2 = db.find("if(W906_GpibModel==\"9050GPIB\")\n        {\n            iInArmShtXCenterPos=0;");
    return k != std::string::npos || k2 != std::string::npos;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("==== HT9050 shuttle safety: Out Shuttle 1 clear before In Shuttle 1 goes to the Index ====\n");
    W906_TestEnsureSimMotors();
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Mot_Name = i;   // boot invariant (InitialMotorParameter): TMyMotor::ScanMotorStatus asks the origin hook by Mot_Name

    printf("[G] not HT9050 (no hook): golden, nothing moves\n");
    {
        World(); W906_Ht9050OrgHomeHook = 0;
        CHECK(W906_Ht9050OutSht1NotClearShuttle1CanNotMove() == false, "guard false without the HT9050 flag");
        CHECK(Pos(MOutShuttle1) == OUT1_L && Pos(MOutShuttle2) == OUT2_R, "Out Shuttle 1 / 2 untouched");
    }

    W906_Ht9050OrgHomeHook = &HookHT9050;

    printf("[R] HT9050, Out Shuttle 1 at Right: In Shuttle 1 may go\n");
    {
        World(); SetMotPos(MOutShuttle1, OUT1_R);
        CHECK(W906_Ht9050OutSht1NotClearShuttle1CanNotMove() == false, "guard false with Out Shuttle 1 at Right");
        CHECK(Pos(MOutShuttle2) == OUT2_R, "Out Shuttle 2 not touched when nothing has to move");
    }

    printf("[Z] HT9050, Out Shuttle 1 at Left, Out Arm ZA not at home: block, nothing moves\n");
    {
        World(); g_zaHome = 0;
        MOT[MInShuttle1].fCMD = true;   // a move in progress
        bool all = true;
        for (int i = 0; i < 20; ++i) all = all && W906_Ht9050OutSht1NotClearShuttle1CanNotMove();
        CHECK(all, "guard true on every tick");
        CHECK(MOT[MInShuttle1].fCMD == false, "In Shuttle 1 stopped (PCIL132_StopMotor resets fCMD)");
        CHECK(Pos(MOutShuttle1) == OUT1_L && Pos(MOutShuttle2) == OUT2_R, "Out Shuttle 1 / 2 do not move without the Out Arm ZA at home");
        std::string why; W906_Ht9050OutSht1ToRight(&why);
        CHECK(why.find("Out Arm ZA") != std::string::npos, "the reason names the Out Arm ZA");
    }

    printf("[S] HT9050, ZA at home, Out Shuttle 2 at Right: Out Shuttle 2 to Left first, then Out Shuttle 1 to Right\n");
    {
        World();
        bool out1MovedEarly = false; int ticks = 0; bool blocked = true;
        for (; ticks < 50 && blocked; ++ticks) {
            blocked = W906_Ht9050OutSht1NotClearShuttle1CanNotMove();
            if (Pos(MOutShuttle1) != OUT1_L && Pos(MOutShuttle2) != OUT2_L) out1MovedEarly = true;
        }
        CHECK(!out1MovedEarly, "Out Shuttle 1 never left Left before Out Shuttle 2 was at Left");
        CHECK(Pos(MOutShuttle2) == OUT2_L, "Out Shuttle 2 at Left (Prod.OutSHT[1].iLeft)");
        CHECK(Pos(MOutShuttle1) == OUT1_R, "Out Shuttle 1 at Right (Prod.OutSHT[0].iRight)");
        CHECK(!blocked, "then the guard lets In Shuttle 1 go");
        CHECK(Pos(MInShuttle1) == 0, "the guard itself never moves In Shuttle 1");
    }

    printf("[S2] ZA leaves home mid-way: Out Shuttle 1 is not commanded on\n");
    {
        World();
        std::string why;
        W906_Ht9050OutSht1ToRight(&why);   // Out Shuttle 2 commanded to Left (the sim motor is instant: a second call would already, rightly, move Out Shuttle 1)
        g_zaHome = 0;
        for (int i = 0; i < 10; ++i) W906_Ht9050OutSht1ToRight(&why);
        CHECK(Pos(MOutShuttle1) == OUT1_L, "Out Shuttle 1 still at Left while the Out Arm ZA is not at home");
    }

    printf("[E] Enable=0 axes count as in place / at home\n");
    {
        World(); MOT[MOutShuttle2].Motor->Enable = false;
        std::string why;
        CHECK(W906_Ht9050OutSht2ToLeft(&why) == true, "Out Shuttle 2 Enable=0 = at Left");
        World(); MOT[MOutArmZA].Motor->Enable = false; g_zaHome = 0;
        int ticks = 0; while (ticks < 50 && W906_Ht9050OutSht1NotClearShuttle1CanNotMove()) ++ticks;
        CHECK(Pos(MOutShuttle1) == OUT1_R, "Out Arm ZA Enable=0 = at home: Out Shuttle 1 cleared");
        World(); MOT[MOutShuttle1].Motor->Enable = false;
        CHECK(W906_Ht9050OutSht1NotClearShuttle1CanNotMove() == false, "Out Shuttle 1 Enable=0 = at Right");
    }
    printf("[I2] Type_HT9050 flow, Do_Auto_InSH case 200: In Shuttle 1 waits (never pulls Out Shuttle 1)\n");
    {
        World(); int rc = 0;
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == true, "Out Shuttle 1 at Left: In Shuttle 1 waits");
        CHECK(Pos(MOutShuttle1) == OUT1_L && Pos(MOutShuttle2) == OUT2_R, "...and nothing else is moved (Do_Auto_OutSH clears Out Shuttle 1 in this flow)");
        SetMotPos(MOutShuttle1, OUT1_R);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == false, "Out Shuttle 1 at Right (Index Z1 off = safe): In Shuttle 1 may go");
        W906_Ht9050OrgHomeHook = 0; SetMotPos(MOutShuttle1, OUT1_L);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == false, "not HT9050: golden (910's own OutSHT1InLF check only)");
        W906_Ht9050OrgHomeHook = &HookHT9050;
    }

    printf("[O2] Type_HT9050 flow, Do_Auto_OutSH: Out Shuttle 1 in / out of the Index needs Out Shuttle 2 at Left + Out Arm ZA at home\n");
    {
        World(); g_zaHome = 0;
        CHECK(W906_Ht9050OutSht1MoveBlocked(OUT1_R) == true, "ZA not at home: Out Shuttle 1 may not leave Left");
        CHECK(Pos(MOutShuttle2) == OUT2_R, "...and Out Shuttle 2 is not moved either");
        g_zaHome = 1;
        int n = 0; while (n < 50 && W906_Ht9050OutSht1MoveBlocked(OUT1_R)) ++n;
        CHECK(Pos(MOutShuttle2) == OUT2_L, "ZA at home: Out Shuttle 2 brought to Left first");
        CHECK(Pos(MOutShuttle1) == OUT1_L, "the gate itself never moves Out Shuttle 1 (the flow's MotorMove does)");
        CHECK(W906_Ht9050OutSht1MoveBlocked(OUT1_R) == false, "then Out Shuttle 1 may move");
        SetMotPos(MOutShuttle1, OUT1_R); g_zaHome = 0;
        CHECK(W906_Ht9050OutSht1MoveBlocked(OUT1_R) == false, "already at the target: not blocked (the Out Arm ZA may be down picking at Right)");
        World(); SetMotPos(MOutShuttle1, OUT1_R); SetMotPos(MOutShuttle2, OUT2_L);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OUT1_L) == false, "into the Index with Out Shuttle 2 at Left and ZA at home: allowed");
        g_zaHome = 0;
        CHECK(W906_Ht9050OutSht1MoveBlocked(OUT1_L) == true, "into the Index with the ZA not at home: blocked");
    }
    printf("[R10] rule 10: the Out Arm XY move to Auto1 goes Y first (C_OutArmSmallY On), then X\n");
    {
        World();
        for (int m : { (int)MOutArmX, (int)MOutArmY }) {
            MOT[m].Motor->Enable = true; MOT[m].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
            MOT[m].fCanMove = MOT[m].fCanMoveR = MOT[m].fCanMoveM = MOT[m].fCanMoveL = true;
        }
        Cylinder[C_OutArmSmallY].Enable = false;   // disabled cylinder: OnSensor() / OffSensor() answer true (mycylin.cpp)
        SetMotPos(MOutArmX, 1000); SetMotPos(MOutArmY, 2000);
        g_W906OutArmRule10 = 1; iPCIL112_OutArmXYMoveTask = 1;
        int r = PCIL112_OutArmXYMove(30000, 40000);
        CHECK(r == PNP_DOING && iPCIL112_OutArmXYMoveTask == 500, "case 1 -> 500 when the flag is up");
        bool xMovedBeforeY = false; int n = 0;
        while (n < 20 && r != PNP_DONE) {
            r = PCIL112_OutArmXYMove(30000, 40000); ++n;
            if (Pos(MOutArmX) != 1000 && Pos(MOutArmY) != 40000) xMovedBeforeY = true;
        }
        CHECK(!xMovedBeforeY, "X never moved before Y was at the Auto1 Y");
        CHECK(r == PNP_DONE && Pos(MOutArmX) == 30000 && Pos(MOutArmY) == 40000, "then X reaches Auto1, PNP_DONE");
        CHECK(Cylinder[C_OutArmSmallY].CylinderName == "C_OutArmSmallY" || Cylinder[C_OutArmSmallY].CylinderName == "",
              "C_OutArmSmallY slot (named by InitialCylinderName at boot)");
        g_W906OutArmRule10 = 2;   // to the Out Shuttle pick: XY together, C_OutArmSmallY Off (disabled = off)
        SetMotPos(MOutArmX, 1000); SetMotPos(MOutArmY, 2000); iPCIL112_OutArmXYMoveTask = 1;
        int r2 = PNP_DOING; for (int k = 0; k < 10 && r2 != PNP_DONE; ++k) r2 = PCIL112_OutArmXYMove(30000, 40000);
        CHECK(r2 == PNP_DONE && Pos(MOutArmX) == 30000 && Pos(MOutArmY) == 40000, "flag 2: XY together, done with the cylinder off");
        g_W906OutArmRule10 = 0;
        SetMotPos(MOutArmX, 1000); SetMotPos(MOutArmY, 2000); iPCIL112_OutArmXYMoveTask = 1;
        PCIL112_OutArmXYMove(30000, 40000); PCIL112_OutArmXYMove(30000, 40000);
        CHECK(Pos(MOutArmX) == 30000 && Pos(MOutArmY) == 40000, "flag 0: golden, X and Y together");
    }
    printf("[W] FR-NB2 (2): W-44 shuttle side -- safe zone, Index Z1, M108, St01's hook (Frank 1006 1A2A3A)\n");
    {
        const int keepType = MachineTypeChoice;
        const int IL = 0, IR = -69123, OR_ = 0, OL = 91850;                    // the machine's teach.ini (runcfg, 1006)
        CHECK(W906_ShtPosInSafeZone(-1000, IL, IR) && !W906_ShtPosInSafeZone(-1001, IL, IR), "W1 In Shuttle 1: up to 1000 toward the socket (Right, negative here) clear, 1001 not");
        CHECK(W906_ShtPosInSafeZone(500, IL, IR) && W906_ShtPosInSafeZone(50000, IL, IR), "W1 In Shuttle 1: beyond home (the shake side) clear");
        CHECK(W906_ShtPosInSafeZone(1000, OR_, OL) && !W906_ShtPosInSafeZone(1001, OR_, OL) && W906_ShtPosInSafeZone(-500, OR_, OL), "W1 Out Shuttle 1: 1000 toward Left clear, 1001 not, beyond home clear");
        CHECK(!W906_ShtPosInSafeZone(0, 5, 5), "W1 Left == Right (not taught): never clear");
        CHECK(W906_Ht9050ShuttlesClearOfIndexHook == &W906_Ht9050ShuttlesClearOfIndex, "W2 St01's W906_Ht9050ShuttlesClearOfIndexHook is installed at start-up");

        MachineTypeChoice = Type_HT9050;
        auto MWorld = [&]() {
            World();
            Prod.InSHT[0].iLeft = IL;  Prod.InSHT[0].iRight = IR;
            Prod.OutSHT[0].iLeft = OL; Prod.OutSHT[0].iRight = OR_;
            SetMotPos(MInShuttle1, IL); SetMotPos(MOutShuttle1, OR_); SetMotPos(MOutShuttle2, OUT2_L);
            MOT[MInShuttle1].iOldPos = IL; MOT[MOutShuttle1].iOldPos = OR_;
            MOT[MCCDY].Motor->Enable = true; MOT[MCCDY].Motor->MotorIdleSafeDoorCheck = &DoorClosed; SetMotPos(MCCDY, 0);  MOT[MCCDY].HomeFlag = 1;   /* AI(W906-CCDYHOME9050) 20261008: HT9050 M108 "at home" = HomeFlag 1 + |enc| <= 100 (the lamp is not asked); g_ccdHome below drives HomeFlag too */
            MOT[MTestZ1].Motor->Enable = true; MOT[MTestZ1].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
            Prod.TestZ1_Safe = 200; SetMotPos(MTestZ1, 200);
        };
        AnsiString why;
        MWorld();
        CHECK(W906_Ht9050ShuttlesClearOfIndex(&why) && why == AnsiString(""), "W3 both shuttles at home: clear (the Index may press)");
        SetMotPos(MOutShuttle1, 300);
        CHECK(W906_Ht9050ShuttlesClearOfIndex(&why), "W3 Out Shuttle 1 300 off home toward the socket (5S / shake): still clear -- not only home (Frank W-97)");
        SetMotPos(MOutShuttle1, 1001);
        CHECK(!W906_Ht9050ShuttlesClearOfIndex(&why) && std::string(why.c_str()).find("Out Shuttle 1") != std::string::npos, "W3 Out Shuttle 1 1001 toward the socket: not clear, the reason names it");
        MWorld(); SetMotPos(MInShuttle1, -1001);
        CHECK(!W906_Ht9050ShuttlesClearOfIndex(&why) && std::string(why.c_str()).find("In Shuttle 1") != std::string::npos, "W3 In Shuttle 1 1001 toward the socket: not clear, the reason names it");
        MWorld(); MOT[MInShuttle1].Motor->Enable = false; SetMotPos(MInShuttle1, IR);
        CHECK(W906_Ht9050ShuttlesClearOfIndex(&why), "W3 an Enable=0 shuttle counts as clear (as W906_ShtAt)");
        {
            MWorld();
            HTMotor *keep = MOT[MInShuttle1].Motor;
            TMovingSimMotor mv; mv.Enable = true; mv.MotorIdleSafeDoorCheck = &DoorClosed; mv.SetPosition(IL);
            MOT[MInShuttle1].Motor = &mv; mv.moving = true;
            MOT[MInShuttle1].iOldPos = IR;
            CHECK(!W906_Ht9050ShuttlesClearOfIndex(&why), "W4 In Shuttle 1 still at home but moving toward the socket (target Right): not clear");
            MOT[MInShuttle1].iOldPos = IL + 500;
            CHECK(W906_Ht9050ShuttlesClearOfIndex(&why), "W4 moving to the shake point beyond home: clear -- the shake is not blocked (Steven's reason for the safe zone)");
            mv.moving = false; MOT[MInShuttle1].iOldPos = IR;
            CHECK(W906_Ht9050ShuttlesClearOfIndex(&why), "W4 stopped at home with an old target: clear (the target only counts while moving)");
            MOT[MInShuttle1].Motor = keep;
        }
        int rc = 0;
        MWorld(); SetMotPos(MOutShuttle1, 300);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == false, "W5 In Shuttle 1 to the Index, Out Shuttle 1 300 off home (its safe zone): may go (Frank 1A; was: Right +-100)");
        SetMotPos(MOutShuttle1, 1001);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == true, "W5 Out Shuttle 1 1001 toward the socket: In Shuttle 1 waits");
        MWorld(); SetMotPos(MTestZ1, 211);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == true, "W6 Index Z1 11 off its safe height: In Shuttle 1 waits (spec §5.2 In Shuttle 1 (3))");
        SetMotPos(MTestZ1, 190);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == false, "W6 Index Z1 10 off its safe height (Frank's +-10): may go");
        SetMotPos(MTestZ1, -4357);
        CHECK(W906_Ht9050InSht1ToIndexBlocked(rc) == true, "W6 Index Z1 at the shuttle pick height: In Shuttle 1 waits");
        MWorld();
        CHECK(W906_Ht9050OutSht1MoveBlocked(OL) == false, "W7 Out Shuttle 1 into the Index, all clear: may go");
        SetMotPos(MInShuttle1, -1001);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OL) == true, "W7 In Shuttle 1 1001 toward the socket: Out Shuttle 1 waits");
        SetMotPos(MInShuttle1, -300);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OL) == false, "W7 In Shuttle 1 300 off home (its safe zone): may go (1A)");
        SetMotPos(MTestZ1, 500);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OL) == true, "W7 Index Z1 off its safe height: Out Shuttle 1 waits (spec §5.2 M17 (4))");
        CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == false && Pos(MOutShuttle1) == OR_, "W7 already at Right: not blocked (no Z1 check out of the socket)");
        MWorld(); SetMotPos(MOutShuttle1, OL);
        g_ccdHome = 0; MOT[MCCDY].HomeFlag = 0;
        CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == true, "W8 M108 not homed (HT9050: HomeFlag 0, 1008): Out Shuttle 1 may not move (spec §5.2 M17 (5))");
        g_ccdHome = 1; MOT[MCCDY].HomeFlag = 1; SetMotPos(MCCDY, 101);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == true, "W8 M108 lamp on, encoder 101 off 0: blocked (Frank 3A: lamp AND encoder)");
        SetMotPos(MCCDY, -100);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == false, "W8 M108 lamp on, encoder -100: at home -> may move");  g_ccdHome = 0; CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == false, "W8 1008: M108 homed at -100 with its lamp OFF (switch edge after a DS402 home): at home -> may move"); g_ccdHome = 1;
        MOT[MCCDY].Motor->Enable = false; g_ccdHome = 0; SetMotPos(MCCDY, 5000);
        CHECK(W906_Ht9050OutSht1MoveBlocked(OR_) == false, "W8 M108 Enable=0 counts as at home");
        std::string w2;
        MWorld(); SetMotPos(MOutShuttle2, OUT2_R); SetMotPos(MOutShuttle1, OL);
        CHECK(!W906_Ht9050OutSht2ToLeft(&w2) && Pos(MOutShuttle2) == OUT2_R && w2.find("Out Shuttle 1 not at Right") != std::string::npos,
              "W9 Out Shuttle Y to Left with Out Shuttle 1 in the Index: refused, not moved (spec §5.2 M18 (1))");
        SetMotPos(MOutShuttle1, OR_); g_ccdHome = 0; MOT[MCCDY].HomeFlag = 0;
        CHECK(!W906_Ht9050OutSht2ToLeft(&w2) && Pos(MOutShuttle2) == OUT2_R && w2.find("M108") != std::string::npos, "W9 M108 not at home: refused (spec §5.2 M18 (4))");
        g_ccdHome = 1; MOT[MCCDY].HomeFlag = 1; W906_Ht9050OutSht2ToLeft(&w2);
        CHECK(Pos(MOutShuttle2) == OUT2_L, "W9 Out Shuttle 1 at Right + M108 + ZA at home: Out Shuttle Y moved to Left");
        MachineTypeChoice = Type_HT9046_LS; SetMotPos(MOutShuttle2, OUT2_R); SetMotPos(MOutShuttle1, OL);
        W906_Ht9050OutSht2ToLeft(&w2);
        CHECK(Pos(MOutShuttle2) == OUT2_L, "W9 not Type_HT9050 (the 9046_LS flow clears Out Shuttle Y first): the old rule, moved");
        MachineTypeChoice = Type_HT9050;
        MWorld(); g_ccdHome = 0; MOT[MCCDY].HomeFlag = 0;
        CHECK(!W906_Ht9050OutShtYToRight(&w2) && Pos(MOutShuttle2) == OUT2_L && w2.find("M108") != std::string::npos, "W10 Out Shuttle Y to Right with M108 not at home: refused");
        g_ccdHome = 1; MOT[MCCDY].HomeFlag = 1; W906_Ht9050OutShtYToRight(&w2);
        CHECK(Pos(MOutShuttle2) == OUT2_R, "W10 M108 at home: Out Shuttle Y moved to Right");
        MOT[MCCDY].Motor->Enable = false; MOT[MTestZ1].Motor->Enable = false;
        MachineTypeChoice = keepType;
    }
    W906_Ht9050OrgHomeHook = 0;

    printf("[P] source pins (argv[1] = source root)\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string ac = ReadFile(root + "/acarry.cpp");
        const std::string uh = ReadFile(root + "/uhome.cpp");
        const std::string ai = ReadFile(root + "/ainarm9045.cpp");
        CHECK(Count(ac, "if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))\r\n                break;\r\n            if(W906_Ht9050OutSht1NotClearShuttle1CanNotMove())") +
              Count(ac, "if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))\n                break;\n            if(W906_Ht9050OutSht1NotClearShuttle1CanNotMove())") == 5,
              "acarry.cpp: the guard right after the Z1 guard at 5 In Shuttle 1 moves toward the Index");
        CHECK(Count(ac, "W906_Ht9050OutSht1NotClearShuttle1CanNotMove())") == 5, "acarry.cpp: exactly 5 call sites");
        CHECK(uh.find("flag6=(MOT[MOutShuttle2].MotorMove(Prod.OutSHT[1].iLeft)==1);") != std::string::npos, "uhome.cpp: HOME's last step = Out Shuttle 2 to Prod.OutSHT[1].iLeft");
        CHECK(Count(ac, "if(W906_Ht9050InSht1ToIndexBlocked(iRetryCT))") == 1, "acarry.cpp: Do_Auto_InSH case 200 calls the In Shuttle 1 gate");
        CHECK(Count(ac, "if(W906_Ht9050OutSht1MoveBlocked(Prod.OutSHT[0].iLeft))") == 1 && Count(ac, "if(W906_Ht9050OutSht1MoveBlocked(Prod.OutSHT[0].iRight))") == 2,
              "acarry.cpp: Do_Auto_OutSH cases 100 (Left) / 200 + 201 (Right) call the Out Shuttle 1 gate");
        const std::string ao = ReadFile(root + "/aoutarm9045.cpp");
        CHECK(ao.find("g_W906OutArmRule10=(W906_Ht9050OrgHome(MTrayX)!=-2 && iOutPutTray==eAuto1) ? 1 : 0;") != std::string::npos, "aoutarm9045.cpp: rule 10 only for HT9050 + Auto1");
        const std::string o11 = ReadFile(root + "/aoutarm9045_1x1_1.cpp");
        CHECK(o11.find("g_W906OutArmRule10=(W906_Ht9050OrgHome(MTrayX)!=-2) ? 2 : 0;") != std::string::npos, "aoutarm9045_1x1_1.cpp: C_OutArmSmallY Off on the Out Shuttle pick (HT9050)");
        CHECK(db_center_pin(root), "database.cpp: shuttle centre offset 0 on 9050GPIB");
        const std::string ci = ReadFile(root + "/cinitial.cpp");
        CHECK(ci.find("Cylinder[C_OutArmSmallY         ].CylinderName=\"C_OutArmSmallY\";") != std::string::npos, "cinitial.cpp: C_OutArmSmallY named (IO_Table mapping)");
        const std::string db = ReadFile(root + "/database.cpp");
        CHECK(db.find("#ifndef W906_HT9050_AS_LS") != std::string::npos && db.find("MachineTypeChoice=Type_HT9050;") != std::string::npos, "database.cpp: 9050GPIB -> Type_HT9050 unless W906_HT9050_AS_LS");
        CHECK(ai.find("!(W906_Ht9050OrgHome(MTrayX)!=-2 && Cylinder[C_TrayY_Fixer].Enable==false)") != std::string::npos, "ainarm9045.cpp: disabled fixer skipped on HT9050 only");
        CHECK(Count(ac, "static const bool s_W906ShuttlesClearHookSet=(W906_Ht9050ShuttlesClearOfIndexHook=&W906_Ht9050ShuttlesClearOfIndex, true);") == 1,
              "acarry.cpp: FR-NB2 (2) installs St01's W-44 hook once");
        CHECK(Count(ac, "#define W906_HT9050_SHT_SAFE_BAND 1000") == 1, "acarry.cpp: the safe-zone band defaults to 1000 (Frank 2A)");
        CHECK(Count(ac, "#include \"atester_FinePitch.h\"") == 0, "acarry.cpp: St01's header is not included (ctest FP9050_Index [F2] census)");
    } else {
        printf("  FAIL: argv[1] (source root) missing\n"); ++g_fail;
    }

    printf("RESULT: %s (%d pass, %d fail)\n", g_fail ? "FAIL" : "ALL PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
