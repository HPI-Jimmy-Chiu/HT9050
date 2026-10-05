// =============================================================================
//  Ht9050DryRun.cpp -- AI(W906-HT9050-DRYRUN) 20261005: HT9050 motor-only dry-run cycle (see Ht9050DryRun.h).
//
//  EastSun 1005 steps (one IC at a time, strictly in this order, then again from Step 1 until PAUSE):
//    Step1  (10-13)  In Arm picks at the Loader         : ZA up -> X/Y to Tech.iInArmLoadStageX/Y -> ZA down to Tech.iInArmLoadStagePickZ2 -> ZA up
//                    (Loader Y "always has an IC", no vacuum, Loader Z never moved)
//    Step2  (20-23)  In Arm places on In Shuttle 1      : In Shuttle 1 to Left -> X/Y to Tech.iInArmShuttle1X/Y -> ZA down to Tech.iInArmShuttlePlaceZ -> ZA up
//    Step3  (30)     Index Z1 HOME lamp on and Out Shuttle 1 at Right (if not: cleared to Right -- Out Arm ZA at home, Out Shuttle 2 to Left first)
//    Step4  (40)     In Shuttle 1 to Right (Index)      : the IC is handed to the Index as data only (Index Z bypassed)
//    Step5  (50)     In Shuttle 1 to Left
//    Step6  (60)     Out Shuttle 1 to Left (Index)      : the IC comes onto Out Shuttle 1 as data
//    Step7  (70-71)  Out Shuttle 1 to Right (out of the Index), THEN Out Shuttle 2 to Right   (EastSun 1005「Step7 ... 這個是Out Shuttle2才對」/「先退出來，再動 Out Shuttle 2」)
//    Step8  (80-82)  Out Arm to the Out Shuttle 1 pick  : ZA up -> C_OutArmSmallY Off -> X to Tech.iOutArmShuttle1X FIRST -> then Y to Tech.iOutArmShuttle1Y
//    Step9  (90-91)  Out Arm picks                      : Out Shuttle 1 / 2 at Right + C_OutArmSmallY Off for 3 s -> ZA down to Tech.iOutArmShuttlePickZ2 -> ZA up
//    Step10 (100-103) Out Arm places on Auto1           : Y to Tech.iOutArmAuto1Y with C_OutArmSmallY On -> (Y there + On for 3 s) -> X to Tech.iOutArmAuto1X
//                    -> ZA down to Tech.iOutArmPlaceZ2 -> ZA up
//    (110)           Out Shuttle 2 back to Left (standby, EastSun 1005「Step 10完成後將Out Shuttle2移動至左邊待命」); one cycle done
//  C_OutArmSmallY: no sensor is asked in the dry run -- 3 s after the On / Off command count as done (EastSun 1005「汽缸問題先假裝沒感測器，先等待三秒來代替」).
//
//  Rules checked on EVERY tick of the step that needs them (a broken rule stops the moving axis and the step waits):
//    * an arm's X / Y move only with its Z HOME lamp on (HT9050 ORG rule, Motor/mymotor.cpp ScanMotorStatus);
//    * In Shuttle 1 moves only with the In Arm ZA HOME lamp on; to the Index only with Out Shuttle 1 at Right and Index Z1 at home;
//    * Out Shuttle 1 moves (in or out of the Index) only with In Shuttle 1 at Left (into the Index), Out Shuttle 2 at
//      Tech.iOutShuttle2Left, the Out Arm ZA and the Index Z1 at home (EastSun rules 9 / 10, 1004);
//    * a Z goes down only with its X / Y at the target (and, for the Out Arm pick, Out Shuttle 1 still at Right).
//  An axis with no motor object or Mot_Table Enable=0 counts as in place / at home (as the full HOME's W906_AxisOff).
//  A wait longer than ~1000 ticks shows one message with the reason (then keeps waiting).
// =============================================================================
#include "Ht9050DryRun.h"
#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[] / W906_Ht9050OrgHome
#include "cprod.h"                  // Prod
#include "cmydef.h"                 // axis indices / iHomeLed / C_OutArmSmallY
#include "LastSet.h"                // Tech
#include "mycylin.h"                // Cylinder[]
#include "canary_support.h"         // ShowMyMessage
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <map>

bool W906_Ht9050OutSht2ToLeft(std::string *why);    // acarry.cpp AI(W906-HT9050-SHTSAFE)
bool W906_Ht9050OutSht1ToRight(std::string *why);   // acarry.cpp AI(W906-HT9050-SHTSAFE)
bool W906_Ht9050IndexZ1AtHome();                    // acarry.cpp AI(W906-HT9050-Z1HOMEPOS) 20261005: lamp, or homed + still + at 0
extern void (*g_W906HomeNote)(const char*);         // uhome.cpp: the op log's HOME line (wb_serve registers it); 0 = console only

#ifndef W906_HT9050_DRYRUN_POS_TOL
#define W906_HT9050_DRYRUN_POS_TOL 100              // encoder counts, as acarry.cpp W906_HT9050_SHT_POS_TOL
#endif
//AI(W906-HT9050-DRYRUN-2) 20261005: EastSun 1005 on the machine「目前速度太快，幫我降速，各軸在10%的速度」-- the dry run issues its own
//  MotorMove (golden's flow sets each move's speed with SetSpeed(Prod / ARM speed); the dry run bypasses it, so the axes ran at whatever the
//  full HOME left). Every driven axis gets TMyMotor::SetSpeed(this %) on each START / resume (Motor/mymotor.cpp:320: PJogHighSpeed * % / 100).
#ifndef W906_HT9050_DRYRUN_SPEED
#define W906_HT9050_DRYRUN_SPEED 50                  // AI(W906-HT9050-DRYRUN-4) 20261005: EastSun「整機的速度設定為50%」(10 -> 20 -> 50)
#endif
//  A step waiting this long (with a reason) shows one message (was ~1000 ticks; a tick is not a fixed time, 1005 the operator gave up first).
#ifndef W906_HT9050_DRYRUN_WAIT_MSG_MS
#define W906_HT9050_DRYRUN_WAIT_MSG_MS 15000
#endif
//AI(W906-HT9050-DRYRUN-4) 20261005: C_OutArmSmallY On / Off counts as done this long after the command (no sensor asked in the dry run).
#ifndef W906_HT9050_DRYRUN_CYL_WAIT_MS
#define W906_HT9050_DRYRUN_CYL_WAIT_MS 3000
#endif
// Test seam: the dry run's clock (ms). 0 = std::chrono::steady_clock (the machine); tests/test_ht9050_dryrun.cpp sets a fake one.
unsigned long (*g_W906DryRunClockMs)() = 0;

namespace {
int         s_step   = 10;
int         s_cycles = 0;
bool        s_paused = false;
unsigned long s_waitSince = 0;                       // steady-clock ms when this step started waiting (0 = not waiting)
bool        s_waitShown = false;
int         s_lastStep = 0;
bool        s_xyDone[2] = { false, false };
bool        s_speedSet = false;                     // AI(W906-HT9050-DRYRUN-2): W906_HT9050_DRYRUN_SPEED applied since the last START / resume
std::string s_why, s_lastWhy;

// printf + the op log (runcfg\logs\oplog_*.txt "HOME  dryrun: ..."), so a stop can be read afterwards.  AI(W906-HT9050-DRYRUN-2) 20261005
void Note(const std::string &s)
{
    std::printf("%s\n", s.c_str());
    if (g_W906HomeNote) g_W906HomeNote(s.c_str());
}

const int kAxes[] = { MInArmX, MInArmY, MInArmZA, MInShuttle1, MOutShuttle1, MOutShuttle2, MOutArmX, MOutArmY, MOutArmZA };

bool AxisOff(int mi) { return MOT[mi].Motor == NULL || MOT[mi].Motor->Enable == false; }
bool Moving(int mi)  { return !AxisOff(mi) && MOT[mi].Motor->MotionDone() == false; }
void StopIfMoving(int mi) { if (Moving(mi)) MOT[mi].PCIL132_StopMotor(); }
bool At(int mi, int target)
{
    if (AxisOff(mi)) return true;
    if (MOT[mi].Motor->MotionDone() == false) return false;
    return std::abs(MOT[mi].ReadEncoderPos() - target) <= W906_HT9050_DRYRUN_POS_TOL;
}
// AI(W906-HT9050-DRYRUN-3) 20261005: the speed again right before each NEW target of an axis (golden's flow also calls SetSpeed before its
//   moves). The START-time SetSpeed alone was not enough: 1005 03:38:09 oplog -- In Arm X / Y ran at 10% (about 1000 / 2000 counts/s) but
//   In Shuttle 1 at about 8000 counts/s (its Mot_Table Rate 80 of JogHighSpeed 10000). Cleared on START / PAUSE (s_speedSet).
std::map<int, int> s_speedTarget;                   // axis -> the target its speed was last set for
void SpeedFor(int mi, int target)
{
    if (AxisOff(mi)) return;
    std::map<int, int>::iterator it = s_speedTarget.find(mi);
    if (it != s_speedTarget.end() && it->second == target) return;
    MOT[mi].SetSpeed(W906_HT9050_DRYRUN_SPEED);
    s_speedTarget[mi] = target;
}
bool Move(int mi, int target) { SpeedFor(mi, target); return AxisOff(mi) || MOT[mi].MotorMove(target) == 1; }
bool HomeLamp(int mi)
{
    if (AxisOff(mi)) return true;
    MOT[mi].ScanMotorStatus();
    return MOT[mi].Led[iHomeLed];
}
// Z up to the arm's safe height, then its HOME lamp must be on.
bool ZUp(int z, int safe)
{
    if (!Move(z, safe)) return false;
    if (HomeLamp(z)) return true;
    s_why = std::string(MOT[z].Alias.c_str()) + " at the safe height but its HOME lamp is off / 在安全高度但原點燈沒亮";
    return false;
}
// true = keep waiting (why set, the moving axes stopped)
bool Blocked(bool ok, const char *why, int a1, int a2 = -1)
{
    if (ok) return false;
    s_why = why;
    StopIfMoving(a1);
    if (a2 >= 0) StopIfMoving(a2);
    return true;
}
void Go(int next) { s_step = next; s_xyDone[0] = s_xyDone[1] = false; s_why.clear(); }
int InArmSafeZ()  { return Prod.ZInArmSafe[0][0]; }
int OutArmSafeZ() { return Prod.ZOutArmSafe[0][0]; }
bool Z1Home()     { return W906_Ht9050IndexZ1AtHome(); }   // AI(W906-HT9050-Z1HOMEPOS) 20261005: was HomeLamp(MTestZ1); EastSun 1005: homed + encoder within +-20 (acarry.cpp)
unsigned long NowMs()
{
    if (g_W906DryRunClockMs) return g_W906DryRunClockMs();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
// AI(W906-HT9050-DRYRUN-4) 20261005: C_OutArmSmallY On / Off, done W906_HT9050_DRYRUN_CYL_WAIT_MS after the command changed (no sensor).
int           s_cylCmd = -1;                        // -1 not commanded yet, 0 Off, 1 On
unsigned long s_cylSince = 0;
bool CylFor(bool on)
{
    if (on) Cylinder[C_OutArmSmallY].On(); else Cylinder[C_OutArmSmallY].Off();
    const unsigned long now = NowMs();
    if (s_cylCmd != (on ? 1 : 0)) { s_cylCmd = on ? 1 : 0; s_cylSince = now; }
    if (now - s_cylSince >= (unsigned long)W906_HT9050_DRYRUN_CYL_WAIT_MS) return true;
    s_why = std::string("C_OutArmSmallY ") + (on ? "On" : "Off") + ": waiting 3 s (no sensor in the dry run) / C_OutArmSmallY 等待 3 秒（空跑不看感測器）";
    return false;
}

void StepTick()
{
    std::string why;
    switch (s_step)
    {
        // ---- Step1: In Arm picks at the Loader ----
        //  AI(W906-HT9050-DRYRUN-2) 20261005: EastSun「Out Arm ZA與In Arm ZA做同樣的Home Sensor處理」-- the Out Arm ZA also goes up to its safe
        //  height (where its HOME lamp is on) at the cycle start, as the In Arm ZA; after the full HOME it sat at 0 with the lamp off and
        //  Step3 waited for ever on "Out Arm ZA not at home" (1005 01:41:02 oplog).
        case 10:
        {
            const bool inUp  = ZUp(MInArmZA, InArmSafeZ());
            const bool outUp = ZUp(MOutArmZA, OutArmSafeZ());
            if (inUp && outUp) Go(11);
            break;
        }
        case 11:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInArmX, MInArmY)) break;
            if (!s_xyDone[0]) s_xyDone[0] = Move(MInArmX, Tech.iInArmLoadStageX);
            if (!s_xyDone[1]) s_xyDone[1] = Move(MInArmY, Tech.iInArmLoadStageY);
            if (s_xyDone[0] && s_xyDone[1]) Go(12);
            break;
        case 12: if (Move(MInArmZA, Tech.iInArmLoadStagePickZ2)) Go(13); break;
        case 13: if (ZUp(MInArmZA, InArmSafeZ())) Go(20); break;
        // ---- Step2: In Arm places on In Shuttle 1 ----
        case 20:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInShuttle1)) break;
            if (Move(MInShuttle1, Prod.InSHT[0].iLeft)) Go(21);
            break;
        case 21:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInArmX, MInArmY)) break;
            if (!s_xyDone[0]) s_xyDone[0] = Move(MInArmX, Tech.iInArmShuttle1X);
            if (!s_xyDone[1]) s_xyDone[1] = Move(MInArmY, Tech.iInArmShuttle1Y);
            if (s_xyDone[0] && s_xyDone[1]) Go(22);
            break;
        case 22:
            if (Blocked(At(MInShuttle1, Prod.InSHT[0].iLeft), "In Shuttle 1 not at Left / In Shuttle 1 不在左邊", MInArmZA)) break;
            if (Move(MInArmZA, Tech.iInArmShuttlePlaceZ)) Go(23);
            break;
        case 23: if (ZUp(MInArmZA, InArmSafeZ())) Go(30); break;
        // ---- Step3: Index Z1 at home and Out Shuttle 1 at Right ----
        case 30:
            if (!ZUp(MOutArmZA, OutArmSafeZ())) break;     // AI(W906-HT9050-DRYRUN-2) 20261005: as the In Arm ZA (see case 10)
            if (!Z1Home()) { s_why = "Index Z1 not at home (not homed, or encoder outside +-20) (Z1 is not moved) / Index Z1 不在原點（沒回完原點，或 Encoder 不在正負 20 內；Z1 不動作）"; break; }
            SpeedFor(MOutShuttle2, Prod.OutSHT[1].iLeft); SpeedFor(MOutShuttle1, Prod.OutSHT[0].iRight);   // AI(W906-HT9050-DRYRUN-3): acarry.cpp moves them
            if (!W906_Ht9050OutSht1ToRight(&why)) { s_why = why; break; }
            Go(40);
            break;
        // ---- Step4: In Shuttle 1 to the Index ----
        case 40:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInShuttle1)) break;
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MInShuttle1)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MInShuttle1)) break;
            if (Move(MInShuttle1, Prod.InSHT[0].iRight)) Go(50);   // the IC goes to the Index as data (Index Z bypassed)
            break;
        // ---- Step5: In Shuttle 1 to Left ----
        case 50:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInShuttle1)) break;
            if (Move(MInShuttle1, Prod.InSHT[0].iLeft)) Go(60);
            break;
        // ---- Step6 / Step7: Out Shuttle 1 into the Index and back out ----
        case 60:
        case 70:
        {
            const int target = (s_step == 60) ? Prod.OutSHT[0].iLeft : Prod.OutSHT[0].iRight;
            if (s_step == 60 && Blocked(At(MInShuttle1, Prod.InSHT[0].iLeft), "In Shuttle 1 not at Left / In Shuttle 1 不在左邊", MOutShuttle1)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle1)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MOutShuttle1)) break;
            if (!At(MOutShuttle2, Prod.OutSHT[1].iLeft)) { StopIfMoving(MOutShuttle1); SpeedFor(MOutShuttle2, Prod.OutSHT[1].iLeft); W906_Ht9050OutSht2ToLeft(&why); s_why = why; break; }
            if (Move(MOutShuttle1, target)) Go(s_step == 60 ? 70 : 71);
            break;
        }
        // ---- Step7 (2nd half): Out Shuttle 2 to Right, only with Out Shuttle 1 out of the Index (at Right) and the Out Arm ZA up ----
        //  AI(W906-HT9050-DRYRUN-4) 20261005: EastSun「Step7:將Out Shuttle1移動至右邊->這個是Out Shuttle2才對」+「先退出來，再動 Out Shuttle 2」
        case 71:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutShuttle2)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle2)) break;
            if (Move(MOutShuttle2, Prod.OutSHT[1].iRight)) Go(80);
            break;
        // ---- Step8: Out Arm to the Out Shuttle 1 pick, X first, then Y ----
        case 80: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(81); break;
        case 81:
            CylFor(false);                                 // AI(W906-HT9050-DRYRUN-4): the 3 s of the Off start here
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmX)) break;
            if (Move(MOutArmX, Tech.iOutArmShuttle1X)) Go(82);
            break;
        case 82:
            CylFor(false);
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmY)) break;
            if (Move(MOutArmY, Tech.iOutArmShuttle1Y)) Go(90);
            break;
        // ---- Step9: Out Arm picks ----
        case 90:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutArmZA)) break;
            if (Blocked(At(MOutShuttle2, Prod.OutSHT[1].iRight), "Out Shuttle 2 not at Right / Out Shuttle 2 不在右邊", MOutArmZA)) break;   // AI(W906-HT9050-DRYRUN-4)
            if (!CylFor(false)) { StopIfMoving(MOutArmZA); break; }   // AI(W906-HT9050-DRYRUN-4): 3 s after Off, no sensor (was OffSensor)
            if (Move(MOutArmZA, Tech.iOutArmShuttlePickZ2)) Go(91);
            break;
        case 91: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(100); break;
        // ---- Step10: Out Arm places on Auto1, Y (with C_OutArmSmallY On) first, then X ----
        case 100:
        {
            const bool cylDone = CylFor(true);             // AI(W906-HT9050-DRYRUN-4): On with Y; 3 s after On, no sensor (was OnSensor)
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmY)) break;
            if (!s_xyDone[1]) s_xyDone[1] = Move(MOutArmY, Tech.iOutArmAuto1Y);
            if (!s_xyDone[1] || !cylDone) break;
            Go(101);
            break;
        }
        case 101:
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmX)) break;
            if (Move(MOutArmX, Tech.iOutArmAuto1X)) Go(102);
            break;
        case 102: if (Move(MOutArmZA, Tech.iOutArmPlaceZ2)) Go(103); break;
        case 103: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(110); break;
        // ---- after Step10: Out Shuttle 2 back to Left (standby); Out Shuttle 1 out of the Index and the Out Arm ZA up ----
        //  AI(W906-HT9050-DRYRUN-4) 20261005: EastSun「Step 10完成後將Out Shuttle2移動至左邊待命」
        case 110:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutShuttle2)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle2)) break;
            if (Move(MOutShuttle2, Prod.OutSHT[1].iLeft)) { ++s_cycles; Go(10); }
            break;
        default: Go(10); break;
    }
}
} // namespace

bool W906_Ht9050DryRunOn()
{
#if defined(W906_HT9050_DRYRUN) && !defined(SOFT_SIMULTE)   //AI(W906-HT9050-PRODFLOW) 20261005: NB2-1 R233, Jimmy 1005 16:2x / 16:4x (to NB2, URGENT U23)「把空跑關掉，改跑 Frank 的正式流程…除非機台端要求修改空跑功能，不然就是修正式流程」＋「機台端不要影響，讓Eastsun決定要不要開起空跑」: the SIMULATION build (SOFT_SIMULTE: dev PCs, NB2, ctest SIM) never lets the dry run own DoAllProcess -> an HT9050 START runs the Type_HT9050 production flow; the machine build (no SOFT_SIMULTE) is unchanged -- EastSun keeps MachineType.h W906_HT9050_DRYRUN on or comments it out. ctest HT9050_ProdFlow
    return W906_Ht9050OrgHome(MTrayX) != -2;
#else
    return false;
#endif
}

void W906_Ht9050DryRunTick(bool paused, bool firstRunTick)
{
    if (firstRunTick && !paused)
    {
        for (int mi : kAxes) MOT[mi].fCMD = false;   // see Ht9050DryRun.h: a stopped axis must not count as arrived
        s_xyDone[0] = s_xyDone[1] = false;
        s_speedSet = false; s_speedTarget.clear();
        Note("dryrun: START at step " + std::to_string(s_step) + " (cycle " + std::to_string(s_cycles) + ") -- every move re-issued");
    }
    if (paused)
    {
        if (!s_paused)
        {
            s_paused = true;
            s_speedSet = false; s_speedTarget.clear();
            for (int mi : kAxes) if (!AxisOff(mi)) MOT[mi].PCIL132_StopMotor();
            Note("dryrun: PAUSE at step " + std::to_string(s_step) + " (cycle " + std::to_string(s_cycles) + ") -- axes stopped");
        }
        return;
    }
    if (s_paused)
    {
        s_paused = false;
        s_xyDone[0] = s_xyDone[1] = false;   // the stop reset each axis's command; the step re-issues its moves
        Note("dryrun: resume at step " + std::to_string(s_step));
    }
    if (!s_speedSet)                         // AI(W906-HT9050-DRYRUN-2) 20261005: every driven axis at W906_HT9050_DRYRUN_SPEED %
    {
        s_speedSet = true;
        for (int mi : kAxes) if (!AxisOff(mi)) MOT[mi].SetSpeed(W906_HT9050_DRYRUN_SPEED);
        Note("dryrun: speed " + std::to_string(W906_HT9050_DRYRUN_SPEED) + "% on every driven axis (W906_HT9050_DRYRUN_SPEED)");
    }
    s_why.clear();
    StepTick();
    if (s_step != s_lastStep)
    {
        Note("dryrun: step " + std::to_string(s_step) + " (cycle " + std::to_string(s_cycles) + ")");
        s_lastStep = s_step;
        s_waitSince = 0; s_waitShown = false; s_lastWhy.clear();
        return;
    }
    if (s_why.empty()) { s_waitSince = 0; return; }
    if (s_why != s_lastWhy) { s_lastWhy = s_why; Note("dryrun: step " + std::to_string(s_step) + " waiting: " + s_why); }
    const unsigned long now = NowMs();
    if (s_waitSince == 0) s_waitSince = now ? now : 1;
    if (!s_waitShown && now - s_waitSince >= (unsigned long)W906_HT9050_DRYRUN_WAIT_MSG_MS)
    {
        s_waitShown = true;
        ShowMyMessage(AnsiString(("Dry run step " + std::to_string(s_step) + " waiting: " + s_why).c_str()),
                      AnsiString(("空跑第 " + std::to_string(s_step) + " 步等待中：" + s_why).c_str()),
                      "W906_Ht9050DryRun");
    }
}

void W906_Ht9050DryRunReset() { Go(10); s_paused = false; s_waitSince = 0; s_waitShown = false; s_lastStep = 0; s_speedSet = false; s_speedTarget.clear(); s_lastWhy.clear(); s_cylCmd = -1; }
int  W906_Ht9050DryRunStep()   { return s_step; }
int  W906_Ht9050DryRunCycles() { return s_cycles; }
std::string W906_Ht9050DryRunWhy() { return s_why; }
