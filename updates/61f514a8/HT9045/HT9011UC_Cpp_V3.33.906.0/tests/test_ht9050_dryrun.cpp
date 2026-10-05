// =============================================================================
//  test_ht9050_dryrun.cpp -- AI(W906-HT9050-DRYRUN) 20261005: the program simulation EastSun asked for before the
//  real machine runs the dry-run cycle (「將以上的步驟使用程式模擬是否能正確執行，若能模擬正確執行再進行真正的機台動作」).
//  ctest HT9050_DryRun. Memory only: every axis the cycle drives is a SLOW sim motor (moves a fixed distance per tick,
//  so moves overlap ticks like the real ones); the HOME lamp of each arm Z follows its position (lit only at / above the
//  safe height), Index Z1's is lit. No file, no card.
//
//  ORACLES (from EastSun's Step1..Step10 and rules 9 / 10, not from the code):
//    [O] the step order of every cycle is exactly Step1..Step10 (10 11 12 13 20 21 22 23 30 40 50 60 70 80 81 82 90 91 100 101 102 103);
//    [C] N cycles complete;
//    [I] on EVERY tick:
//        I1  never In Shuttle 1 away from Left while Out Shuttle 1 is away from Right (the shared Index point);
//        I2  Out Shuttle 1 moves only with Out Shuttle 2 at Left and the Out Arm ZA at home;
//        I3  an arm's X / Y moves only with its Z at home; In Shuttle 1 moves only with the In Arm ZA at home;
//        I4  Out Arm X and Y never move in the same tick (Step8 X then Y, Step10 Y then X);
//        I5  Index Z1 and Loader Z never move;
//        I6  the Out Arm ZA is down at the Out Shuttle pick only with Out Shuttle 1 at Right;
//        I7  Step8: X reaches the pick X before Y leaves the Auto1 Y; Step10: Y reaches Auto1 Y before X leaves the pick X;
//    [P] PAUSE mid-move stops every axis (no position changes while paused), START resumes the same step and finishes;
//    [S] start with Out Shuttle 1 parked in the Index and Out Shuttle 2 at Right: Step3 clears them (Out Shuttle 2 to Left
//        first) without breaking any rule;
//    [Z] the Out Arm ZA HOME lamp off: the cycle waits at the step, nothing moves into the Index.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "cmydef.h"
#include "MachineType.h"
#include "LastSet.h"
#include "Motor/mymotor.h"
#include "Motor/mySimMotor.h"
#include "mycylin.h"
#include "Ht9050DryRun.h"
#include "w906_test_motors.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// A sim motor that moves STEP counts per tick (Advance(), called once per tick by the test).
class TSlowMotor : public TMySimMotor
{
public:
    int pos, target, step;
    TSlowMotor() : pos(0), target(0), step(2000) {}
    virtual int  ReadPos()            { return pos; }
    virtual int  ReadRealPos()        { return pos; }
    virtual int  ReadEnCoderRealPos() { return pos; }
    virtual bool MoveToPos(int t)     { target = t; return true; }
    virtual bool MotionDone()         { return pos == target; }
    virtual int  SetPosition(int p)   { pos = target = p; return 0; }
    virtual int  SetCommand(int p)    { pos = target = p; return 0; }
    virtual void Stop()               { target = pos; }
    virtual void DecStop(void)        { target = pos; }
    virtual void ScanMotorStatus(bool *Led) { TMySimMotor::ScanMotorStatus(Led); if (Led) Led[iInposLed] = (pos != target); }
    void Advance() { if (pos < target) pos = (target - pos > step) ? pos + step : target; else if (pos > target) pos = (pos - target > step) ? pos - step : target; }
};

static bool DoorClosed() { return false; }
static const int ZSAFE = 50;
static std::map<int, TSlowMotor*> g_m;
static const int kAxes[] = { MInArmX, MInArmY, MInArmZA, MInShuttle1, MOutShuttle1, MOutShuttle2, MOutArmX, MOutArmY, MOutArmZA, MTestZ1, MLoaderZ };

static int P(int mi) { return g_m[mi]->pos; }
static int Hook(int mi)
{
    if (mi == MInArmZA || mi == MOutArmZA) return P(mi) >= ZSAFE - 5 ? 1 : 0;   // lamp lit only at / above the safe height
    if (g_m.count(mi) && mi != MTestZ1) return std::abs(P(mi)) < 500 ? 1 : 0;  // X / Y / shuttles: lit only near their origin (golden's "arrived > 2000 away
    return 1;                                                                   //  with the lamp lit = Home sensor error" check, Motor/mymotor.cpp MotorMovePosition)
}
// AI(W906-HT9050-DRYRUN-4) 20261005: the dry run's clock (Ht9050DryRun.cpp g_W906DryRunClockMs) -- 100 ms per tick, so the 3 s C_OutArmSmallY wait is 30 ticks
extern unsigned long (*g_W906DryRunClockMs)();
static unsigned long g_ms = 1;
static unsigned long FakeMs() { return g_ms; }
static bool g_zaLampForcedOff = false;
static bool g_z1LampOff = false;   // AI(W906-HT9050-DRYRUN-2) 20261005: [H] Index Z1 homed at 0 but its lamp off (the DS402 home ends just off the switch)
static int HookZ(int mi) { if (g_zaLampForcedOff && mi == MOutArmZA) return 0; if (g_z1LampOff && mi == MTestZ1) return 0; return Hook(mi); }

static void Setup()
{
    W906_TestEnsureSimMotors();
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Mot_Name = i;   // boot invariant (InitialMotorParameter): ScanMotorStatus asks the origin hook by Mot_Name
    for (int mi : kAxes) {
        TSlowMotor *m = new TSlowMotor();
        m->Enable = true; m->MotorIdleSafeDoorCheck = &DoorClosed;
        m->PSoftLimitP = 9999999; m->PSoftLimitN = -9999999;
        MOT[mi].Motor = m; g_m[mi] = m;
        MOT[mi].fCanMove = MOT[mi].fCanMoveR = MOT[mi].fCanMoveM = MOT[mi].fCanMoveL = true;
    }
    g_m[MInArmZA]->step = g_m[MOutArmZA]->step = 1000;
    g_m[MTestZ1]->Enable = false;   // Index Z1's encoder read goes through the Galil route (not in a unit test): Enable=0 = "at home"; I5 still checks it never moves
    // teach.ini values of 20261004 (runcfg\system\teach.ini)
    Tech.iInArmLoadStageX = -8307;  Tech.iInArmLoadStageY = 13531;  Tech.iInArmLoadStagePickZ2 = -18289;
    Tech.iInArmShuttle1X  = -9719;  Tech.iInArmShuttle1Y  = 113317; Tech.iInArmShuttlePlaceZ   = -5000;
    Tech.iOutArmShuttle1X = 26793;  Tech.iOutArmShuttle1Y = 53278;  Tech.iOutArmShuttlePickZ2  = -4939;
    Tech.iOutArmAuto1X    = 214512; Tech.iOutArmAuto1Y    = 13283;  Tech.iOutArmPlaceZ2        = -18191;
    Prod.InSHT[0].iLeft  = 0;     Prod.InSHT[0].iRight  = 69123;
    Prod.OutSHT[0].iLeft = 91850; Prod.OutSHT[0].iRight = 0;
    Prod.OutSHT[1].iLeft = -11609; Prod.OutSHT[1].iRight = -45670;
    Prod.ZInArmSafe[0][0] = ZSAFE; Prod.ZOutArmSafe[0][0] = ZSAFE;
    Cylinder[C_OutArmSmallY].Enable = false;   // disabled: On / Off sensors answer true (mycylin.cpp)
    W906_Ht9050OrgHomeHook = &HookZ;
    g_W906DryRunClockMs = &FakeMs;
}
static void Place(int mi, int p) { g_m[mi]->SetPosition(p); MOT[mi].Position = p; MOT[mi].fCMD = false; }
static void HomeWorld()   // where the full HOME leaves the machine (Z at the safe height, shuttles at their home side)
{
    for (int mi : kAxes) Place(mi, 0);
    Place(MInArmZA, ZSAFE); Place(MOutArmZA, ZSAFE);
    Place(MOutShuttle2, Prod.OutSHT[1].iLeft);
    Place(MOutArmY, Tech.iOutArmAuto1Y); Place(MOutArmX, Tech.iOutArmAuto1X);
    W906_Ht9050DryRunReset();
}

struct Viol { int n = 0; std::vector<std::string> what; void Add(const std::string &w) { if (++n <= 10) what.push_back(w); } };
static std::map<int, int> g_prev;
static void Snap() { for (int mi : kAxes) g_prev[mi] = P(mi); }
static bool Moved(int mi) { return g_prev[mi] != P(mi); }

static void CheckTick(Viol &v, int tick)
{
    const std::string t = " (tick " + std::to_string(tick) + ", step " + std::to_string(W906_Ht9050DryRunStep()) + ")";
    if (P(MInShuttle1) != Prod.InSHT[0].iLeft && P(MOutShuttle1) != Prod.OutSHT[0].iRight) v.Add("I1 In Shuttle 1 off Left while Out Shuttle 1 off Right" + t);
    if (Moved(MOutShuttle1) && (g_prev[MOutShuttle2] != Prod.OutSHT[1].iLeft || HookZ(MOutArmZA) != 1)) v.Add("I2 Out Shuttle 1 moved without Out Shuttle 2 Left / Out Arm ZA home" + t);
    if ((Moved(MInArmX) || Moved(MInArmY) || Moved(MInShuttle1)) && g_prev[MInArmZA] < ZSAFE - 5) v.Add("I3 In Arm X/Y or In Shuttle moved with the In Arm ZA down" + t);
    if ((Moved(MOutArmX) || Moved(MOutArmY)) && g_prev[MOutArmZA] < ZSAFE - 5) v.Add("I3 Out Arm X/Y moved with the Out Arm ZA down" + t);
    if (Moved(MOutArmX) && Moved(MOutArmY)) v.Add("I4 Out Arm X and Y moved in the same tick" + t);
    if (Moved(MTestZ1) || Moved(MLoaderZ)) v.Add("I5 Index Z1 / Loader Z moved" + t);
    if (P(MOutArmZA) < ZSAFE - 5 && P(MOutArmX) == Tech.iOutArmShuttle1X && P(MOutArmY) == Tech.iOutArmShuttle1Y && P(MOutShuttle1) != Prod.OutSHT[0].iRight)
        v.Add("I6 Out Arm ZA down at the Out Shuttle pick with Out Shuttle 1 away" + t);
    const int s = W906_Ht9050DryRunStep();
    if (s == 81 && Moved(MOutArmY)) v.Add("I7 Step8: Y moved before X reached the pick X" + t);
    if (s == 100 && Moved(MOutArmX)) v.Add("I7 Step10: X moved before Y reached Auto1 Y" + t);
    // AI(W906-HT9050-DRYRUN-4) 20261005: Out Shuttle 2 goes toward Right only with Out Shuttle 1 out of the Index (at Right); never with the Out Arm ZA down
    if (Moved(MOutShuttle2) && P(MOutShuttle2) < g_prev[MOutShuttle2] && g_prev[MOutShuttle1] != Prod.OutSHT[0].iRight) v.Add("I8 Out Shuttle 2 moved toward Right with Out Shuttle 1 in the Index" + t);
    if (Moved(MOutShuttle2) && g_prev[MOutArmZA] < ZSAFE - 5) v.Add("I8 Out Shuttle 2 moved with the Out Arm ZA down" + t);
}
// one tick: the dry run commands, then every motor advances once (the motion between two ticks)
static void Tick(bool paused = false) { Snap(); W906_Ht9050DryRunTick(paused); for (int mi : kAxes) g_m[mi]->Advance(); g_ms += 100; }

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("==== HT9050 dry-run cycle simulation (Step1..Step10, slow sim motors) ====\n");
    Setup();

    printf("[O][C][I] 5 cycles from the HOME position\n");
    {
        HomeWorld();
        Viol v; std::vector<int> order; int lastStep = -1, tick = 0;
        unsigned long t81 = 0, t91 = 0, t100 = 0, t101 = 0;   // AI(W906-HT9050-DRYRUN-4): first time each step is reached (fake ms)
        for (; tick < 200000 && W906_Ht9050DryRunCycles() < 5; ++tick) {
            Tick(); CheckTick(v, tick);
            const int s = W906_Ht9050DryRunStep();
            if (s != lastStep) {
                order.push_back(s); lastStep = s;
                if (s == 81 && !t81) t81 = g_ms; if (s == 91 && !t91) t91 = g_ms;
                if (s == 100 && !t100) t100 = g_ms; if (s == 101 && !t101) t101 = g_ms;
            }
        }
        CHECK(t91 && t91 - t81 >= 3000, "Step9: the Out Arm ZA goes down only 3 s after C_OutArmSmallY Off (no sensor)");
        CHECK(t101 && t101 - t100 >= 3000, "Step10: X moves only 3 s after C_OutArmSmallY On (no sensor)");
        printf("  (Off -> pick ZA down %lu ms, On -> X %lu ms)\n", t91 - t81, t101 - t100);
        CHECK(W906_Ht9050DryRunCycles() == 5, "5 cycles completed");
        printf("  (%d ticks, %d cycles, last wait '%s')\n", tick, W906_Ht9050DryRunCycles(), W906_Ht9050DryRunWhy().c_str());
        const int one[] = { 11, 12, 13, 20, 21, 22, 23, 30, 40, 50, 60, 70, 71, 80, 81, 82, 90, 91, 100, 101, 102, 103, 110, 10 };   // AI(W906-HT9050-DRYRUN-4): + 71 / 110
        std::vector<int> want; for (int c = 0; c < 5; ++c) for (int s : one) want.push_back(s);
        bool same = order.size() >= want.size();
        for (size_t i = 0; same && i < want.size(); ++i) same = (order[i] == want[i]);
        CHECK(same, "step order = Step1..Step10 every cycle");
        if (!same) { printf("  order:"); for (size_t i = 0; i < order.size() && i < 60; ++i) printf(" %d", order[i]); printf("\n"); }
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken on any tick (I1..I8)");
        CHECK(P(MOutArmX) == Tech.iOutArmAuto1X && P(MOutArmY) == Tech.iOutArmAuto1Y && P(MOutArmZA) == ZSAFE, "cycle end: Out Arm at Auto1, ZA up");
    }

    printf("[P] PAUSE in the middle of Step4 (In Shuttle 1 to the Index), then START\n");
    {
        HomeWorld();
        Viol v; int tick = 0;
        while (tick < 100000 && W906_Ht9050DryRunStep() != 40) { Tick(); CheckTick(v, tick++); }
        for (int k = 0; k < 5; ++k) { Tick(); CheckTick(v, tick++); }   // In Shuttle 1 on its way
        const int mid = P(MInShuttle1);
        CHECK(mid > Prod.InSHT[0].iLeft && mid < Prod.InSHT[0].iRight, "In Shuttle 1 is between Left and Right when PAUSE comes");
        // the real PAUSE (WebStart.cpp PauseFromWeb): StopAllMotor stops the axes and DoAllProcess is no longer called at all;
        // MOT[].fCMD of the moving axis stays true (the dry run does not see the stop)
        for (int mi : kAxes) g_m[mi]->Stop();
        bool still = true;
        for (int k = 0; k < 50; ++k) { Snap(); for (int mi : kAxes) g_m[mi]->Advance(); for (int mi : kAxes) if (Moved(mi)) still = false; }
        CHECK(still, "while paused no axis moves");
        CHECK(W906_Ht9050DryRunStep() == 40, "paused at the same step");
        CHECK(P(MInShuttle1) == mid, "In Shuttle 1 stopped where the PAUSE caught it");
        // START: the first running tick has iHandlerStartCount==0
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); CheckTick(v, tick++);
        CHECK(W906_Ht9050DryRunStep() == 40, "after START still Step4 (the stopped In Shuttle 1 is NOT taken as arrived)");
        while (tick < 200000 && W906_Ht9050DryRunStep() == 40) { Tick(); CheckTick(v, tick++); }
        CHECK(P(MInShuttle1) == Prod.InSHT[0].iRight, "In Shuttle 1 really reached Right before Step5");
        // a paused tick of the dry run itself (paused=true) also stops and holds
        while (tick < 200000 && W906_Ht9050DryRunStep() != 101) { Tick(); CheckTick(v, tick++); }
        for (int k = 0; k < 3; ++k) { Tick(); CheckTick(v, tick++); }
        const int x0 = P(MOutArmX); bool still2 = true;
        for (int k = 0; k < 20; ++k) { Tick(true); if (Moved(MOutArmX)) still2 = false; }
        CHECK(still2 && P(MOutArmX) == x0, "paused=true: Out Arm X stops and holds");
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); CheckTick(v, tick++);
        while (tick < 200000 && W906_Ht9050DryRunCycles() < 6) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= 6, "after START the cycle finishes and goes on");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken around the PAUSE");
    }

    printf("[S] Out Shuttle 1 parked in the Index, Out Shuttle 2 at Right at START\n");
    {
        HomeWorld();
        Place(MOutShuttle1, Prod.OutSHT[0].iLeft); Place(MOutShuttle2, Prod.OutSHT[1].iRight);
        Viol v; int tick = 0; bool out2LeftFirst = true;
        while (tick < 200000 && W906_Ht9050DryRunCycles() < 8) {
            Tick();
            if (Moved(MOutShuttle1) && P(MOutShuttle2) != Prod.OutSHT[1].iLeft) out2LeftFirst = false;
            // I1 is expected to be "broken" before Step3 clears the start state (In Shuttle 1 at Left = 0 is fine); check the rest
            CheckTick(v, tick++);
        }
        CHECK(W906_Ht9050DryRunCycles() >= 8, "cycles run after Step3 cleared Out Shuttle 1");
        CHECK(out2LeftFirst, "Out Shuttle 2 reached Left before Out Shuttle 1 moved");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken while clearing");
    }

    printf("[Z] Out Arm ZA HOME lamp off: nothing goes into the Index\n");
    {
        HomeWorld(); g_zaLampForcedOff = true;
        int tick = 0; bool out1Moved = false;
        for (; tick < 20000; ++tick) { Tick(); if (Moved(MOutShuttle1)) out1Moved = true; }
        CHECK(!out1Moved, "Out Shuttle 1 never moved");
        CHECK(W906_Ht9050DryRunStep() <= 60, "the cycle waits (at or before Step6)");
        printf("  (waiting at step %d: '%s')\n", W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
        g_zaLampForcedOff = false;
    }

    // AI(W906-HT9050-DRYRUN-2) 20261005: the machine's state at 1005 01:41 (oplog), where the dry run waited at Step3 for ever:
    //   after the full HOME the Out Arm ZA sat at 0 (lamp lit only at the safe height) and the Index Z1 ended its 2nd home READY at enc 0
    //   with the lamp off. EastSun「Out Arm ZA與In Arm ZA做同樣的Home Sensor處理」: the Out Arm ZA goes up at the cycle start as the In Arm ZA;
    //   Z1 counts as at home when homed + still + at 0. Also: every driven axis at W906_HT9050_DRYRUN_SPEED % after START.
    printf("[H] after the full HOME: Out Arm ZA at 0 (lamp off), Index Z1 homed at 0 with its lamp off; speed 50%% (Ht9050DryRun.cpp W906_HT9050_DRYRUN_SPEED)\n");
    {
        HomeWorld();
        Place(MOutArmZA, 0); Place(MInArmZA, 0);
        g_m[MTestZ1]->Enable = true; Place(MTestZ1, 0); MOT[MTestZ1].HomeFlag = 1; g_z1LampOff = true;
        for (int mi : kAxes) g_m[mi]->PJogHighSpeed = 80000;
        Viol v; int tick = 0; const int c0 = W906_Ht9050DryRunCycles();
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); CheckTick(v, tick++);   // START
        CHECK(MOT[MInArmX].speed == 40000 && MOT[MOutArmZA].speed == 40000 && MOT[MOutShuttle1].speed == 40000,
              "START: every driven axis at 50% (speed = PJogHighSpeed * 50 / 100)");
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 2) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 2, "2 more cycles run (Step3 no longer waits on the Out Arm ZA / Index Z1)");
        printf("  (%d ticks, step %d, last wait '%s')\n", tick, W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken (I1..I8)");

        // EastSun 1005: Index Z homed and its encoder within +-20 = at home (dry run only); outside, or not homed = Step3 waits
        struct { int enc, homed; bool home; const char *what; } z[] = {
            { 20, 1, true,  "Index Z1 homed, encoder +20: at home, Step3 goes on" },
            { -20, 1, true, "Index Z1 homed, encoder -20: at home, Step3 goes on" },
            { 21, 1, false, "Index Z1 homed, encoder +21: NOT at home, waits at Step3, In Shuttle 1 stays out of the Index" },
            { -21, 1, false, "Index Z1 homed, encoder -21: NOT at home, waits at Step3" },
            { 0, 0, false,  "Index Z1 not homed (HomeFlag 0), encoder 0: NOT at home, waits at Step3" } };
        for (auto &k : z) {
            HomeWorld(); Place(MTestZ1, k.enc); MOT[MTestZ1].HomeFlag = k.homed;
            int t2 = 0; bool inSht1In = false;
            for (; t2 < 20000 && W906_Ht9050DryRunStep() <= 40; ++t2) { Tick(); if (P(MInShuttle1) > Prod.InSHT[0].iLeft) inSht1In = true; }
            const bool passed3 = W906_Ht9050DryRunStep() > 30;
            CHECK(k.home ? passed3 : (!passed3 && !inSht1In && W906_Ht9050DryRunStep() == 30), k.what);
            if (!k.home) printf("  (waiting at step %d: '%s')\n", W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
        }
        g_z1LampOff = false; g_m[MTestZ1]->Enable = false;
    }
    W906_Ht9050OrgHomeHook = 0;

    printf("RESULT: %s (%d pass, %d fail)\n", g_fail ? "FAIL" : "ALL PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
