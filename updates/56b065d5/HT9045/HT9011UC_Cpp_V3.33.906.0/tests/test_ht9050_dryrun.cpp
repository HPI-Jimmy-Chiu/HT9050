// =============================================================================
//  test_ht9050_dryrun.cpp -- AI(W906-HT9050-DRYRUN) 20261005: the program simulation EastSun asked for before the
//  real machine runs the dry-run cycle (「將以上的步驟使用程式模擬是否能正確執行，若能模擬正確執行再進行真正的機台動作」).
//  ctest HT9050_DryRun. Memory only: every axis the cycle drives is a SLOW sim motor (moves a fixed distance per tick,
//  so moves overlap ticks like the real ones); the HOME lamp of each arm Z follows its position (lit only at / above the
//  safe height). No file, no card.
//
//  AI(W906-HT9050-DRYRUN-14) 20261005 (Frank01, card FR-DR14 / W-71): EastSun 1005's 14 steps (REQUEST.md §0 / §3). The Index Z1 is a
//  slow sim motor too, driven through the dry run's g_W906DryRunZ1Move seam (Gali_MotMove = the Galil route on the machine); the
//  Index nozzle through g_W906DryRunIdxVac / g_W906DryRunIdxVacMade (its vacuum sensor comes on 2 ticks after the valve opens with
//  the Z1 at the In Shuttle pick height -- the IC is there -- and goes off when the valve closes).
//
//  ORACLES (from EastSun's Step1..Step14 and rules 9 / 10 / REQUEST §3, not from the code):
//    [O] the step order of every cycle is exactly Step1..Step14
//        (10 11 12 13 | 20-23 | 30 | 40 | 50 | 60 61 | 70 | 80 | 90 91 | 100 | 110 111 | 120-122 | 130 131 | 140-143 | 150);
//    [C] at least 2 (here 5) full cycles complete (EastSun: 「跑過至少兩輪完整 14 步」);
//    [I] on EVERY tick:
//        I1  never In Shuttle 1 away from Left while Out Shuttle 1 is away from Right (the shared Index point);
//        I2  Out Shuttle 1 moves only with Out Shuttle 2 at Left and the Out Arm ZA at home;
//        I3  an arm's X / Y moves only with its Z at home; In Shuttle 1 moves only with the In Arm ZA at home;
//        I4  Out Arm X and Y never move in the same tick (Step12 X then Y, Step14 Y then X);
//        I5  the Loader Z never moves;
//        I6  the Out Arm ZA is down at the Out Shuttle pick only with Out Shuttle 1 at Right;
//        I7  Step12: X reaches the pick X before Y leaves the Auto1 Y; Step14: Y reaches Auto1 Y before X leaves the pick X;
//        I8  Out Shuttle 2 goes toward Right only with Out Shuttle 1 at Right, and never moves with the Out Arm ZA down;
//        I9  no shuttle moves while the Index Z1 is away from home (REQUEST §3「Index Z1 不在原點不准動飛梭」);
//        I10 the Index Z1 moves only with every shuttle still, and is below home only at a hand-over: In Shuttle 1 at Right and
//            Out Shuttle 1 at Right (the pick), or Out Shuttle 1 at Left and In Shuttle 1 at Left (the place);
//        I11 the Index Z1 leaves the In Shuttle pick height only with the vacuum made (Step6「有吸到 IC 上升」);
//    [V] the Index vacuum: on before the Z1 reaches the pick height; Step6 waits (Z1 down, the reason says vacuum) while the
//        sensor stays off; the release at Step9 is at the Out Shuttle height, vacuum off with a 0.3 s break (300..400 ms on the
//        100 ms simulation clock) every cycle; the Z1 reaches exactly Prod.TestZ1_Pick / Prod.TestZ1_Place and home (0) after each;
//    [P] PAUSE mid-move (a shuttle, the Index Z1, the Out Arm X) stops it (no position change while paused), START resumes
//        the same step and finishes;
//    [S] start with Out Shuttle 1 parked in the Index and Out Shuttle 2 at Right: Step3 clears them (Out Shuttle 2 to Left
//        first) without breaking any rule;
//    [Z] the Out Arm ZA HOME lamp off: the cycle waits at the step, nothing moves into the Index;
//    [H] after the full HOME (Out Arm ZA at 0, Z1 homed at 0 with the lamp off): runs; Z1 "at home" = homed + encoder +-20;
//    [R] HOME's W906_Ht9050DryRunReset() in the middle of a cycle: the next cycle starts at Step1;
//    [G] each guard pulled on its own (the cycle order alone keeps most of them true): the Z1 not at home when a shuttle step starts,
//        Out Shuttle 1 away when In Shuttle 1 / the Z1 would go into the Index -- nothing moves, the step waits, then goes on;
//    [E] the Index Z1 and the Index nozzle with Enable=0 (no seams: the real FTestSuck): skipped, nothing waits.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "cmydef.h"
#include "MachineType.h"
#include "LastSet.h"
#include "Motor/mymotor.h"
#include "Motor/mySimMotor.h"
#include "mycylin.h"
#include "mykitsuck.h"
#include "Ht9050DryRun.h"
#include "w906_test_motors.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

extern TMyKitSuck FTestSuck;    // golden MyKitSuck.h:363 (aHotPlateSubstrate.h:660)

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
static const int Z1TOL = 20;    // acarry.cpp W906_HT9050_DRYRUN_Z1_HOME_TOL
static std::map<int, TSlowMotor*> g_m;
static const int kAxes[] = { MInArmX, MInArmY, MInArmZA, MInShuttle1, MOutShuttle1, MOutShuttle2, MOutArmX, MOutArmY, MOutArmZA, MTestZ1, MLoaderZ };
static const int kShuttles[] = { MInShuttle1, MOutShuttle1, MOutShuttle2 };

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

// ---- AI(W906-HT9050-DRYRUN-14) 20261005: the Index Z1 and the Index nozzle behind the dry run's seams ----
static int  g_z1Calls = 0;                       // how often the dry run asked for a Z1 move
static bool SimZ1Move(int pos)                   // as Gali_MotMove: command the target, true once arrived and still
{
    ++g_z1Calls;
    g_m[MTestZ1]->MoveToPos(pos);
    return g_m[MTestZ1]->MotionDone() && P(MTestZ1) == pos;
}
static int  g_z1Stops = 0;
static void SimZ1Stop() { ++g_z1Stops; g_m[MTestZ1]->Stop(); }   // as the route's "ST": the move is cancelled where it is
static bool g_valve = false;                     // vacuum valve open
static bool g_break = false;                     // break valve open
static bool g_ic = false;                        // the IC is on the Index nozzle
static int  g_icTicks = 0;                       // ticks with the valve open at the pick height
static bool g_sensorStuckOff = false;            // [V] the vacuum never comes (no IC in the shuttle pocket)
static unsigned long g_breakSince = 0;
static std::vector<unsigned long> g_breakPulse;  // length of every break pulse (ms)
static std::vector<int> g_releaseZ1;             // the Z1 position at every release (vacuum off)
static std::vector<int> g_vacOnZ1;               // the Z1 position at every vacuum on
static void SimVac(int op)
{
    if (op == 1) { g_valve = true; g_break = false; g_vacOnZ1.push_back(P(MTestZ1)); }
    else if (op == 2) { if (g_valve) g_releaseZ1.push_back(P(MTestZ1)); g_valve = false; g_ic = false; g_icTicks = 0; if (!g_break) { g_break = true; g_breakSince = g_ms; } }
    else { if (g_break) g_breakPulse.push_back(g_ms - g_breakSince); g_break = false; }
}
static bool SimVacMade() { return !g_sensorStuckOff && g_valve && g_ic; }
static void SimVacAdvance()   // the IC comes onto the nozzle 2 ticks after the valve opens at the pick height
{
    if (g_valve && !g_ic && P(MTestZ1) == Prod.TestZ1_Pick) { if (++g_icTicks >= 2) g_ic = true; }
}

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
    g_m[MTestZ1]->step = 500;                   // AI(W906-HT9050-DRYRUN-14): the Index Z1 moves (Step5/6/9/10) -- slow, so moves overlap ticks
    MOT[MTestZ1].HomeFlag = 1;                  // homed by the full HOME
    // teach.ini values of 20261004 (runcfg\system\teach.ini)
    Tech.iInArmLoadStageX = -8307;  Tech.iInArmLoadStageY = 13531;  Tech.iInArmLoadStagePickZ2 = -18289;
    Tech.iInArmShuttle1X  = -9719;  Tech.iInArmShuttle1Y  = 113317; Tech.iInArmShuttlePlaceZ   = -5000;
    Tech.iOutArmShuttle1X = 26793;  Tech.iOutArmShuttle1Y = 53278;  Tech.iOutArmShuttlePickZ2  = -4939;
    Tech.iOutArmAuto1X    = 214512; Tech.iOutArmAuto1Y    = 13283;  Tech.iOutArmPlaceZ2        = -18191;
    Prod.InSHT[0].iLeft  = 0;     Prod.InSHT[0].iRight  = 69123;
    Prod.OutSHT[0].iLeft = 91850; Prod.OutSHT[0].iRight = 0;
    Prod.OutSHT[1].iLeft = -11609; Prod.OutSHT[1].iRight = -45670;
    Prod.ZInArmSafe[0][0] = ZSAFE; Prod.ZOutArmSafe[0][0] = ZSAFE;
    // AI(W906-HT9050-DRYRUN-14): the two Index Z1 heights -- In Shuttle Z = teach.ini [MTestZ1] setEditIndex1ToSht1Z (machine -4357);
    //   Out Shuttle Z (setEditIndex1ToOutSht1Z, machine default 0 = EastSun 1005) set to a real depth here so the place really goes down
    Prod.TestZ1_Pick  = -4357;
    Prod.TestZ1_Place = -3500;
    Cylinder[C_OutArmSmallY].Enable = false;   // disabled: On / Off sensors answer true (mycylin.cpp)
    W906_Ht9050OrgHomeHook = &HookZ;
    g_W906DryRunClockMs = &FakeMs;
    g_W906DryRunZ1Move = &SimZ1Move;
    g_W906DryRunZ1Stop = &SimZ1Stop;
    g_W906DryRunIdxVac = &SimVac;
    g_W906DryRunIdxVacMade = &SimVacMade;
}
static void Place(int mi, int p) { g_m[mi]->SetPosition(p); MOT[mi].Position = p; MOT[mi].fCMD = false; }
static void HomeWorld()   // where the full HOME leaves the machine (Z at the safe height, shuttles at their home side, the Index Z1 at 0)
{
    for (int mi : kAxes) Place(mi, 0);
    Place(MInArmZA, ZSAFE); Place(MOutArmZA, ZSAFE);
    Place(MOutShuttle2, Prod.OutSHT[1].iLeft);
    Place(MOutArmY, Tech.iOutArmAuto1Y); Place(MOutArmX, Tech.iOutArmAuto1X);
    g_valve = g_break = g_ic = false; g_icTicks = 0; g_sensorStuckOff = false;
    g_breakPulse.clear(); g_releaseZ1.clear(); g_vacOnZ1.clear();
    W906_Ht9050DryRunReset();
}

struct Viol { int n = 0; std::vector<std::string> what; void Add(const std::string &w) { if (++n <= 10) what.push_back(w); } };
static std::map<int, int> g_prev;
static bool g_prevSensor = false;
static void Snap() { for (int mi : kAxes) g_prev[mi] = P(mi); g_prevSensor = SimVacMade(); }
static bool Moved(int mi) { return g_prev[mi] != P(mi); }
static bool AnyShuttleMoved() { for (int mi : kShuttles) if (Moved(mi)) return true; return false; }

static void CheckTick(Viol &v, int tick)
{
    const std::string t = " (tick " + std::to_string(tick) + ", step " + std::to_string(W906_Ht9050DryRunStep()) + ")";
    if (P(MInShuttle1) != Prod.InSHT[0].iLeft && P(MOutShuttle1) != Prod.OutSHT[0].iRight) v.Add("I1 In Shuttle 1 off Left while Out Shuttle 1 off Right" + t);
    if (Moved(MOutShuttle1) && (g_prev[MOutShuttle2] != Prod.OutSHT[1].iLeft || HookZ(MOutArmZA) != 1)) v.Add("I2 Out Shuttle 1 moved without Out Shuttle 2 Left / Out Arm ZA home" + t);
    if ((Moved(MInArmX) || Moved(MInArmY) || Moved(MInShuttle1)) && g_prev[MInArmZA] < ZSAFE - 5) v.Add("I3 In Arm X/Y or In Shuttle moved with the In Arm ZA down" + t);
    if ((Moved(MOutArmX) || Moved(MOutArmY)) && g_prev[MOutArmZA] < ZSAFE - 5) v.Add("I3 Out Arm X/Y moved with the Out Arm ZA down" + t);
    if (Moved(MOutArmX) && Moved(MOutArmY)) v.Add("I4 Out Arm X and Y moved in the same tick" + t);
    if (Moved(MLoaderZ)) v.Add("I5 Loader Z moved" + t);
    if (P(MOutArmZA) < ZSAFE - 5 && P(MOutArmX) == Tech.iOutArmShuttle1X && P(MOutArmY) == Tech.iOutArmShuttle1Y && P(MOutShuttle1) != Prod.OutSHT[0].iRight)
        v.Add("I6 Out Arm ZA down at the Out Shuttle pick with Out Shuttle 1 away" + t);
    const int s = W906_Ht9050DryRunStep();
    if (s == 121 && Moved(MOutArmY)) v.Add("I7 Step12: Y moved before X reached the pick X" + t);
    if (s == 140 && Moved(MOutArmX)) v.Add("I7 Step14: X moved before Y reached Auto1 Y" + t);
    // AI(W906-HT9050-DRYRUN-4) 20261005: Out Shuttle 2 goes toward Right only with Out Shuttle 1 out of the Index (at Right); never with the Out Arm ZA down
    if (Moved(MOutShuttle2) && P(MOutShuttle2) < g_prev[MOutShuttle2] && g_prev[MOutShuttle1] != Prod.OutSHT[0].iRight) v.Add("I8 Out Shuttle 2 moved toward Right with Out Shuttle 1 in the Index" + t);
    if (Moved(MOutShuttle2) && g_prev[MOutArmZA] < ZSAFE - 5) v.Add("I8 Out Shuttle 2 moved with the Out Arm ZA down" + t);
    // AI(W906-HT9050-DRYRUN-14) 20261005: the Index Z1 rules (REQUEST §3)
    if (AnyShuttleMoved() && (std::abs(g_prev[MTestZ1]) > Z1TOL || std::abs(P(MTestZ1)) > Z1TOL)) v.Add("I9 a shuttle moved with the Index Z1 away from home" + t);
    if (Moved(MTestZ1) && AnyShuttleMoved()) v.Add("I10 the Index Z1 moved while a shuttle moved" + t);
    if (P(MTestZ1) < -Z1TOL) {
        const bool atPick  = P(MInShuttle1) == Prod.InSHT[0].iRight && P(MOutShuttle1) == Prod.OutSHT[0].iRight;
        const bool atPlace = P(MOutShuttle1) == Prod.OutSHT[0].iLeft && P(MInShuttle1) == Prod.InSHT[0].iLeft;
        if (!atPick && !atPlace) v.Add("I10 the Index Z1 below home away from a hand-over" + t);
    }
    if (g_prev[MTestZ1] == Prod.TestZ1_Pick && P(MTestZ1) > Prod.TestZ1_Pick && !g_prevSensor) v.Add("I11 the Index Z1 left the pick height without the vacuum made" + t);
}
// one tick: the dry run commands, then every motor advances once (the motion between two ticks)
static void Tick(bool paused = false) { Snap(); W906_Ht9050DryRunTick(paused); for (int mi : kAxes) g_m[mi]->Advance(); SimVacAdvance(); g_ms += 100; }

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("==== HT9050 dry-run cycle simulation (Step1..Step14, slow sim motors, Index Z1 + vacuum) ====\n");
    Setup();

    printf("[O][C][I][V] 5 cycles from the HOME position\n");
    {
        HomeWorld();
        Viol v; std::vector<int> order; int lastStep = -1, tick = 0;
        unsigned long t121 = 0, t131 = 0, t140 = 0, t141 = 0;   // AI(W906-HT9050-DRYRUN-4): first time each step is reached (fake ms)
        std::vector<int> z1AtStepEnd;                           // the Z1 position when the step left 50 / 61 / 90 / 100
        for (; tick < 400000 && W906_Ht9050DryRunCycles() < 5; ++tick) {
            Tick(); CheckTick(v, tick);
            const int s = W906_Ht9050DryRunStep();
            if (s != lastStep) {
                if (lastStep == 50 || lastStep == 61 || lastStep == 90 || lastStep == 100) z1AtStepEnd.push_back(g_prev[MTestZ1]);
                order.push_back(s); lastStep = s;
                if (s == 121 && !t121) t121 = g_ms; if (s == 131 && !t131) t131 = g_ms;
                if (s == 140 && !t140) t140 = g_ms; if (s == 141 && !t141) t141 = g_ms;
            }
        }
        CHECK(t131 && t131 - t121 >= 3000, "Step13: the Out Arm ZA goes down only 3 s after C_OutArmSmallY Off (no sensor)");
        CHECK(t141 && t141 - t140 >= 3000, "Step14: X moves only 3 s after C_OutArmSmallY On (no sensor)");
        CHECK(W906_Ht9050DryRunCycles() == 5, "5 cycles completed (EastSun: at least 2 full rounds of the 14 steps)");
        printf("  (%d ticks, %d cycles, last wait '%s')\n", tick, W906_Ht9050DryRunCycles(), W906_Ht9050DryRunWhy().c_str());
        const int one[] = { 11, 12, 13, 20, 21, 22, 23, 30, 40, 50, 60, 61, 70, 80, 90, 91, 100, 110, 111, 120, 121, 122, 130, 131, 140, 141, 142, 143, 150, 10 };
        std::vector<int> want; for (int c = 0; c < 5; ++c) for (int s : one) want.push_back(s);
        bool same = order.size() >= want.size();
        for (size_t i = 0; same && i < want.size(); ++i) same = (order[i] == want[i]);
        CHECK(same, "step order = Step1..Step14 every cycle");
        if (!same) { printf("  order:"); for (size_t i = 0; i < order.size() && i < 80; ++i) printf(" %d", order[i]); printf("\n"); }
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken on any tick (I1..I11)");
        CHECK(P(MOutArmX) == Tech.iOutArmAuto1X && P(MOutArmY) == Tech.iOutArmAuto1Y && P(MOutArmZA) == ZSAFE, "cycle end: Out Arm at Auto1, ZA up");
        // [V] the Index Z1 heights and the nozzle
        bool heights = z1AtStepEnd.size() >= 20;
        for (size_t i = 0; heights && i + 3 < z1AtStepEnd.size(); i += 4)
            heights = z1AtStepEnd[i] == Prod.TestZ1_Pick && z1AtStepEnd[i + 1] == 0 && z1AtStepEnd[i + 2] == Prod.TestZ1_Place && z1AtStepEnd[i + 3] == 0;
        CHECK(heights, "[V] each cycle: Z1 at Prod.TestZ1_Pick (Step5), home (Step6), Prod.TestZ1_Place (Step9), home (Step10)");
        bool vacOnAbove = g_vacOnZ1.size() == 5;
        for (int z : g_vacOnZ1) if (z == Prod.TestZ1_Pick) vacOnAbove = false;
        CHECK(vacOnAbove, "[V] the Index vacuum is switched on (once per cycle) before the Z1 reaches the pick height");
        bool relAtPlace = g_releaseZ1.size() == 5;
        for (int z : g_releaseZ1) if (z != Prod.TestZ1_Place) relAtPlace = false;
        CHECK(relAtPlace, "[V] the IC is released (vacuum off) at the Out Shuttle height, once per cycle");
        bool pulseOk = g_breakPulse.size() == 5;
        for (unsigned long p : g_breakPulse) if (p < 300 || p > 400) pulseOk = false;
        CHECK(pulseOk, "[V] vacuum break 0.3 s at every release (300..400 ms on the 100 ms clock)");
        printf("  (break pulses:"); for (unsigned long p : g_breakPulse) printf(" %lu", p); printf(" ms)\n");
        CHECK(!g_valve && !g_break, "[V] cycle end: vacuum and break valves closed");
    }

    printf("[V] no IC on the nozzle: Step6 waits with the Z1 down and says why\n");
    {
        HomeWorld(); g_sensorStuckOff = true;
        Viol v; int tick = 0; const int c0 = W906_Ht9050DryRunCycles();
        while (tick < 100000 && W906_Ht9050DryRunStep() != 60) { Tick(); CheckTick(v, tick++); }
        for (int k = 0; k < 300; ++k) { Tick(); CheckTick(v, tick++); }   // 30 s of the simulation clock (past the 15 s message)
        CHECK(W906_Ht9050DryRunStep() == 60, "still Step6 after 30 s without the vacuum");
        CHECK(P(MTestZ1) == Prod.TestZ1_Pick, "the Index Z1 stays down at the pick height");
        CHECK(W906_Ht9050DryRunWhy().find("vacuum") != std::string::npos, "the wait reason names the vacuum");
        printf("  (waiting: '%s')\n", W906_Ht9050DryRunWhy().c_str());
        g_sensorStuckOff = false;
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 1) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 1, "the IC comes (sensor on): the cycle goes on and finishes");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken while waiting for the vacuum");
    }

    printf("[P] PAUSE in the middle of Step4 (In Shuttle 1 to the Index), of Step5 (Z1 down) and of Step14 (Out Arm X), then START\n");
    {
        HomeWorld();
        Viol v; int tick = 0; const int c0 = W906_Ht9050DryRunCycles();
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
        // AI(W906-HT9050-DRYRUN-14): PAUSE (the dry run's own paused tick) while the Index Z1 goes down at Step5
        for (int k = 0; k < 3 && W906_Ht9050DryRunStep() == 50; ++k) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunStep() == 50 && P(MTestZ1) < 0 && P(MTestZ1) > Prod.TestZ1_Pick, "the Index Z1 is between home and the pick height when PAUSE comes");
        const int z0 = P(MTestZ1); bool stillZ = true; const int stops0 = g_z1Stops;
        for (int k = 0; k < 20; ++k) { Tick(true); if (Moved(MTestZ1)) stillZ = false; }
        CHECK(g_z1Stops == stops0 + 1, "paused=true: one Index Z1 stop is sent (the Galil route's ST, not PCIL132_StopMotor)");
        CHECK(stillZ && P(MTestZ1) == z0, "paused=true: the Index Z1 stops and holds");
        const int calls0 = g_z1Calls;
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); SimVacAdvance(); g_ms += 100; CheckTick(v, tick++);
        CHECK(g_z1Calls > calls0 && W906_Ht9050DryRunStep() == 50, "START: the Z1 move is issued again, still Step5");
        while (tick < 200000 && W906_Ht9050DryRunStep() == 50) { Tick(); CheckTick(v, tick++); }
        CHECK(P(MTestZ1) == Prod.TestZ1_Pick, "the Index Z1 really reached the pick height before Step6");
        // a paused tick of the dry run itself (paused=true) also stops and holds the Out Arm X (Step14)
        while (tick < 200000 && W906_Ht9050DryRunStep() != 141) { Tick(); CheckTick(v, tick++); }
        for (int k = 0; k < 3; ++k) { Tick(); CheckTick(v, tick++); }
        const int x0 = P(MOutArmX); bool still2 = true;
        for (int k = 0; k < 20; ++k) { Tick(true); if (Moved(MOutArmX)) still2 = false; }
        CHECK(still2 && P(MOutArmX) == x0, "paused=true: Out Arm X stops and holds");
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); CheckTick(v, tick++);
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 2) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 2, "after START the cycle finishes and goes on");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken around the PAUSEs");
    }

    printf("[S] Out Shuttle 1 parked in the Index, Out Shuttle 2 at Right at START\n");
    {
        HomeWorld();
        Place(MOutShuttle1, Prod.OutSHT[0].iLeft); Place(MOutShuttle2, Prod.OutSHT[1].iRight);
        Viol v; int tick = 0; bool out2LeftFirst = true;
        const int c0 = W906_Ht9050DryRunCycles();
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 2) {
            Tick();
            if (Moved(MOutShuttle1) && P(MOutShuttle2) != Prod.OutSHT[1].iLeft) out2LeftFirst = false;
            CheckTick(v, tick++);
        }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 2, "cycles run after Step3 cleared Out Shuttle 1");
        CHECK(out2LeftFirst, "Out Shuttle 2 reached Left before Out Shuttle 1 moved");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken while clearing");
    }

    printf("[Z] Out Arm ZA HOME lamp off: nothing goes into the Index\n");
    {
        HomeWorld(); g_zaLampForcedOff = true;
        int tick = 0; bool out1Moved = false, z1Moved = false;
        for (; tick < 20000; ++tick) { Tick(); if (Moved(MOutShuttle1)) out1Moved = true; if (Moved(MTestZ1)) z1Moved = true; }
        CHECK(!out1Moved, "Out Shuttle 1 never moved");
        CHECK(!z1Moved, "the Index Z1 never moved");
        CHECK(W906_Ht9050DryRunStep() <= 80, "the cycle waits (at or before Step8)");
        printf("  (waiting at step %d: '%s')\n", W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
        g_zaLampForcedOff = false;
    }

    printf("[R] HOME resets the dry run in the middle of a cycle (uhome.cpp step 300 W906_Ht9050DryRunReset)\n");
    {
        HomeWorld();
        Viol v; int tick = 0;
        while (tick < 100000 && W906_Ht9050DryRunStep() != 120) { Tick(); CheckTick(v, tick++); }
        HomeWorld();                                   // the full HOME puts every axis back and calls W906_Ht9050DryRunReset()
        CHECK(W906_Ht9050DryRunStep() == 10, "after the reset the dry run is at Step1");
        std::vector<int> order; int lastStep = -1; const int c0 = W906_Ht9050DryRunCycles();
        while (tick < 300000 && W906_Ht9050DryRunCycles() < c0 + 1) { Tick(); CheckTick(v, tick++); const int s = W906_Ht9050DryRunStep(); if (s != lastStep) { order.push_back(s); lastStep = s; } }
        CHECK(!order.empty() && order[0] == 11 && W906_Ht9050DryRunCycles() == c0 + 1, "a whole new cycle from Step1 follows");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken after the reset");
    }

    // AI(W906-HT9050-DRYRUN-14) 20261005: the guards themselves. In the normal cycle the order alone keeps most of them true (Step7 starts only
    //   after Step6 put the Z1 at home), so they are pulled here: the condition is broken the moment the step is entered, nothing may move,
    //   the step waits; once the condition holds again the cycle goes on.
    printf("[G] the guards: Z1 not at home at a shuttle step / a shuttle away at a Z1 or Index step -> nothing moves, the step waits\n");
    {
        struct G { int step, kind; const char *what; } gs[] = {
            { 40,  0, "Step4: Index Z1 not at home -> In Shuttle 1 stays out of the Index" },
            { 70,  0, "Step7: Index Z1 not at home -> In Shuttle 1 stays in the Index" },
            { 80,  0, "Step8: Index Z1 not at home -> Out Shuttle 1 stays out of the Index" },
            { 110, 0, "Step11: Index Z1 not at home -> Out Shuttle 1 stays in the Index" },
            { 111, 0, "Step11: Index Z1 not at home -> Out Shuttle 2 stays at Left" },
            { 150, 0, "after Step14: Index Z1 not at home -> Out Shuttle 2 stays at Right" },
            { 40,  1, "Step4: Out Shuttle 1 not at Right -> In Shuttle 1 stays out of the Index" },
            { 50,  2, "Step5: Out Shuttle 1 not at Right -> the Index Z1 does not go down" },
            { 90,  3, "Step9: Out Shuttle 1 not at Left -> the Index Z1 does not go down" } };
        for (auto &g : gs) {
            HomeWorld();
            int tick = 0;
            while (tick < 200000 && W906_Ht9050DryRunStep() != g.step) { Tick(); ++tick; }
            const int saved = P(MOutShuttle1);
            if (g.kind == 0) MOT[MTestZ1].HomeFlag = 0;                                    // Z1 "not at home" (W906_Ht9050IndexZ1AtHome: not homed)
            else if (g.kind == 1 || g.kind == 2) Place(MOutShuttle1, Prod.OutSHT[0].iLeft); // Out Shuttle 1 not cleared (in the Index)
            else Place(MOutShuttle1, Prod.OutSHT[0].iRight);                               // Out Shuttle 1 not under the Index
            std::map<int, int> p0; for (int mi : kAxes) p0[mi] = P(mi);
            for (int k = 0; k < 50; ++k) Tick();
            bool still = W906_Ht9050DryRunStep() == g.step;
            if (g.kind <= 1) { for (int mi : kShuttles) if (P(mi) != p0[mi]) still = false; }
            else if (P(MTestZ1) != p0[MTestZ1]) still = false;
            CHECK(still, g.what);
            if (!still) printf("  (step %d, waiting '%s')\n", W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
            if (g.kind == 0) MOT[MTestZ1].HomeFlag = 1; else Place(MOutShuttle1, saved);
            const int c0 = W906_Ht9050DryRunCycles();
            while (tick < 400000 && W906_Ht9050DryRunCycles() < c0 + 1) { Tick(); ++tick; }
            CHECK(W906_Ht9050DryRunCycles() == c0 + 1, (std::string(g.what) + " ... and the cycle goes on once it holds again").c_str());
        }
    }

    // AI(W906-HT9050-DRYRUN-2) 20261005: the machine's state at 1005 01:41 (oplog), where the dry run waited at Step3 for ever:
    //   after the full HOME the Out Arm ZA sat at 0 (lamp lit only at the safe height) and the Index Z1 ended its 2nd home READY at enc 0
    //   with the lamp off. EastSun「Out Arm ZA與In Arm ZA做同樣的Home Sensor處理」: the Out Arm ZA goes up at the cycle start as the In Arm ZA;
    //   Z1 counts as at home when homed + still + at 0. Also: every driven axis at W906_HT9050_DRYRUN_SPEED % after START.
    printf("[H] after the full HOME: Out Arm ZA at 0 (lamp off), Index Z1 homed at 0 with its lamp off; speed 50%% (Ht9050DryRun.cpp W906_HT9050_DRYRUN_SPEED)\n");
    {
        HomeWorld();
        Place(MOutArmZA, 0); Place(MInArmZA, 0);
        Place(MTestZ1, 0); MOT[MTestZ1].HomeFlag = 1; g_z1LampOff = true;
        for (int mi : kAxes) g_m[mi]->PJogHighSpeed = 80000;
        Viol v; int tick = 0; const int c0 = W906_Ht9050DryRunCycles();
        Snap(); W906_Ht9050DryRunTick(false, true); for (int mi : kAxes) g_m[mi]->Advance(); CheckTick(v, tick++);   // START
        CHECK(MOT[MInArmX].speed == 40000 && MOT[MOutArmZA].speed == 40000 && MOT[MOutShuttle1].speed == 40000,
              "START: every driven axis at 50% (speed = PJogHighSpeed * 50 / 100)");
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 2) { Tick(); CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 2, "2 more cycles run (Step3 no longer waits on the Out Arm ZA / Index Z1)");
        printf("  (%d ticks, step %d, last wait '%s')\n", tick, W906_Ht9050DryRunStep(), W906_Ht9050DryRunWhy().c_str());
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken (I1..I11)");

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
        g_z1LampOff = false; MOT[MTestZ1].HomeFlag = 1;
    }

    printf("[E] the Index Z1 and the Index nozzle with Enable=0, no seams (the real FTestSuck): skipped, nothing waits\n");
    {
        HomeWorld();
        g_W906DryRunZ1Move = 0; g_W906DryRunZ1Stop = 0; g_W906DryRunIdxVac = 0; g_W906DryRunIdxVacMade = 0;
        g_m[MTestZ1]->Enable = false;
        FTestSuck.Suck[0][0].Enable = false;
        Viol v; int tick = 0; bool z1Moved = false; const int c0 = W906_Ht9050DryRunCycles();
        while (tick < 200000 && W906_Ht9050DryRunCycles() < c0 + 2) { Tick(); if (Moved(MTestZ1)) z1Moved = true; CheckTick(v, tick++); }
        CHECK(W906_Ht9050DryRunCycles() >= c0 + 2, "2 cycles complete with the Index Z1 and nozzle switched off");
        CHECK(!z1Moved, "the switched-off Index Z1 is never moved");
        for (auto &w : v.what) printf("  VIOLATION: %s\n", w.c_str());
        CHECK(v.n == 0, "no rule broken with the Index Z1 / nozzle off");
        g_m[MTestZ1]->Enable = true;
        g_W906DryRunZ1Move = &SimZ1Move; g_W906DryRunZ1Stop = &SimZ1Stop; g_W906DryRunIdxVac = &SimVac; g_W906DryRunIdxVacMade = &SimVacMade;
    }
    printf("[Z] AI(W906-Z1GUARD) 20261007: a pick / place target at or above home (Out Sht Z not taught: 0 + Release height 1499) is refused, the Z1 is not driven\n");
    {
        const int savedPlace = Prod.TestZ1_Place;
        HomeWorld(); Prod.TestZ1_Place = 1499;
        int tick = 0; bool z1Above = false;
        while (tick < 100000 && W906_Ht9050DryRunStep() != 90) { Tick(); ++tick; }
        for (int k = 0; k < 50; ++k) { Tick(); if (P(MTestZ1) > 0) z1Above = true; }
        CHECK(W906_Ht9050DryRunStep() == 90, "Step9 waits when Prod.TestZ1_Place = +1499");
        CHECK(!z1Above && P(MTestZ1) == 0, "the Index Z1 stays at home, never above it");
        CHECK(W906_Ht9050DryRunWhy().find("Out Sht Z") != std::string::npos, "the wait reason says to teach Out Sht Z");
        printf("  (waiting: '%s')\n", W906_Ht9050DryRunWhy().c_str());
        Prod.TestZ1_Place = savedPlace;
        const int savedPick = Prod.TestZ1_Pick;
        HomeWorld(); Prod.TestZ1_Pick = 0;
        tick = 0;
        while (tick < 100000 && W906_Ht9050DryRunStep() != 50) { Tick(); ++tick; }
        for (int k = 0; k < 50; ++k) Tick();
        CHECK(W906_Ht9050DryRunStep() == 50, "Step5 waits when Prod.TestZ1_Pick = 0 (at home)");
        Prod.TestZ1_Pick = savedPick;
    }
    W906_Ht9050OrgHomeHook = 0;

    printf("RESULT: %s (%d pass, %d fail)\n", g_fail ? "FAIL" : "ALL PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
