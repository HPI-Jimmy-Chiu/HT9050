// =============================================================================
//  Ht9050DryRun.cpp -- AI(W906-HT9050-DRYRUN) 20261005: HT9050 motor-only dry-run cycle (see Ht9050DryRun.h).
//
//  AI(W906-HT9050-DRYRUN-14) 20261005 (Frank01, card FR-DR14 / W-71): EastSun 1005's 14 steps replace the 10 steps (GitHub
//  HPI-Jimmy-Chiu/HT9050 machine/integ-ioweb dispatch/20261005_dryrun14_indexz/REQUEST.md §0, verbatim there). New against the 10
//  steps: the Index Z1 really goes down and up (Step5/6 at In Shuttle 1, Step9/10 at Out Shuttle 1) with the Index nozzle's vacuum;
//  the In / Out Arms still carry the IC as data only (REQUEST §3「Step1～2 沿用現有空跑」), no vacuum on them.
//  One IC at a time, strictly in this order, then again from Step 1 until PAUSE:
//    Step1  (10-13)   In Arm picks at the Loader         : ZA up -> X/Y to Tech.iInArmLoadStageX/Y -> ZA down to Tech.iInArmLoadStagePickZ2 -> ZA up
//                     (Loader Y "always has an IC", no vacuum, Loader Z never moved)
//    Step2  (20-23)   In Arm places on In Shuttle 1      : In Shuttle 1 to Left -> X/Y to Tech.iInArmShuttle1X/Y -> ZA down to Tech.iInArmShuttlePlaceZ -> ZA up
//    Step3  (30)      Index Z1 at home and Out Shuttle 1 at Right (if not: cleared to Right -- Out Arm ZA at home, Out Shuttle 2 to Left first)
//    Step4  (40)      In Shuttle 1 to Right (the Index)  : Index Z1 not moved (EastSun「Index Z動作By Pass」)
//    Step5  (50)      Index Z1 down to Prod.TestZ1_Pick (the In Shuttle Z teach point), Index vacuum on
//    Step6  (60-61)   up only with the IC on the nozzle (waits for the Index vacuum; a long wait = the usual wait message, no dialog of
//                     its own), then Index Z1 up to its home (0)
//    Step7  (70)      In Shuttle 1 to Left
//    Step8  (80)      Out Shuttle 1 to Left (the Index)
//    Step9  (90-91)   Index Z1 down to Prod.TestZ1_Place (the Out Shuttle Z teach point on HT9050, cinitial.cpp:5805-5807), release:
//                     vacuum off + break on, W906_HT9050_DRYRUN_BREAK_MS (0.3 s), break off
//    Step10 (100)     Index Z1 up to its home (0)
//    Step11 (110-111) Out Shuttle 1 to Right (out of the Index), THEN Out Shuttle 2 to Right -- kept from the 10 steps' Step7 (EastSun 1005
//                     「Step7:將Out Shuttle1移動至右邊->這個是Out Shuttle2才對」/「先退出來，再動 Out Shuttle 2」): the Out Arm picks with
//                     both at Right (Step13). To be confirmed by EastSun for the 14 steps (Step11 names Out Shuttle 1 only).
//    Step12 (120-122) Out Arm to the Out Shuttle 1 pick  : ZA up -> C_OutArmSmallY Off -> X to Tech.iOutArmShuttle1X FIRST -> then Y to Tech.iOutArmShuttle1Y
//    Step13 (130-131) Out Arm picks                      : Out Shuttle 1 / 2 at Right + C_OutArmSmallY Off for 3 s -> ZA down to Tech.iOutArmShuttlePickZ2 -> ZA up
//    Step14 (140-143) Out Arm places on Auto1            : Y to Tech.iOutArmAuto1Y with C_OutArmSmallY On -> (Y there + On for 3 s) -> X to Tech.iOutArmAuto1X
//                     -> ZA down to Tech.iOutArmPlaceZ2 -> ZA up   (Y first, X second: the interference zone)
//    (150)            Out Shuttle 2 back to Left (standby, EastSun 1005「Step 10完成後將Out Shuttle2移動至左邊待命」); one cycle done
//  C_OutArmSmallY: no sensor is asked in the dry run -- 3 s after the On / Off command count as done (EastSun 1005「汽缸問題先假裝沒感測器，先等待三秒來代替」).
//
//  Rules checked on EVERY tick of the step that needs them (a broken rule stops the moving axis and the step waits):
//    * an arm's X / Y move only with its Z HOME lamp on (HT9050 ORG rule, Motor/mymotor.cpp ScanMotorStatus);
//    * no shuttle moves unless the Index Z1 is at home (homed, still, encoder within +-20 of 0: acarry.cpp W906_Ht9050IndexZ1AtHome)
//      -- EastSun REQUEST §3「Index Z1 不在原點不准動飛梭」;
//    * In Shuttle 1 moves only with the In Arm ZA HOME lamp on; to the Index only with Out Shuttle 1 at Right;
//    * Out Shuttle 1 moves (in or out of the Index) only with In Shuttle 1 at Left (into the Index), Out Shuttle 2 at
//      Tech.iOutShuttle2Left and the Out Arm ZA at home (EastSun rules 9 / 10, 1004);
//    * the Index Z1 goes down only with both shuttles still at its hand-over: at the In Shuttle (Step5) In Shuttle 1 at Right and
//      Out Shuttle 1 at Right; at the Out Shuttle (Step9) Out Shuttle 1 at Left and In Shuttle 1 at Left;
//    * a Z goes down only with its X / Y at the target (and, for the Out Arm pick, Out Shuttle 1 still at Right).
//  An axis with no motor object or Mot_Table Enable=0 counts as in place / at home (as the full HOME's W906_AxisOff); an Index
//  nozzle with Enable=0 counts as "vacuum made" (golden TMySucker::Sensor()), so nothing waits on a switched-off IO.
//  Index Z1 move = MOT[MTestZ1].Gali_MotMove (the HT9050 Galil-routed single-axis move, as the full HOME's W906_HomeTwoZ, uhome.cpp);
//  Index nozzle = FTestSuck.Suck[0][0] (the Test Arm 1 nozzle golden aTester_Front.cpp DoFrontTestSuckIC / DoFrontTestDestroyIC drive;
//  with the HT9050 VC4 group switched on it is the group master, AI(W906-F03-QUADVAC)).
//  A wait longer than W906_HT9050_DRYRUN_WAIT_MSG_MS shows one message with the reason (then keeps waiting).
// =============================================================================
#include "Ht9050DryRun.h"
#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[] / W906_Ht9050OrgHome
#include "cprod.h"                  // Prod
#include "cmydef.h"                 // axis indices / iHomeLed / C_OutArmSmallY / DUMMY / HAS_TRAY
#include "LastSet.h"                // Tech / LastSet
#include "mycylin.h"                // Cylinder[]
#include "mykitsuck.h"              // TMyKitSuck / TMySucker  AI(W906-HT9050-DRYRUN-14)
#include "canary_support.h"         // ShowMyMessage
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <map>

bool W906_Ht9050OutSht2ToLeft(std::string *why);    // acarry.cpp AI(W906-HT9050-SHTSAFE)
bool W906_Ht9050OutSht1ToRight(std::string *why);   // acarry.cpp AI(W906-HT9050-SHTSAFE)
bool W906_Ht9050IndexZ1AtHome();                    // acarry.cpp AI(W906-HT9050-Z1HOMEPOS) 20261005: lamp, or homed + still + at 0
extern void (*g_W906HomeNote)(const char*);         // uhome.cpp: the op log's HOME line (wb_serve registers it); 0 = console only
extern TMyKitSuck FTestSuck;                        // AI(W906-HT9050-DRYRUN-14): golden MyKitSuck.h:363 (front test head); aHotPlateSubstrate.h:660
extern bool IndexZCanMove[2];                       // AI(W906-HT9050-DRYRUN-14): golden ainarm2.h:48; ainarm9045_w7_shims.cpp:57

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
//AI(W906-HT9050-DRYRUN-14) 20261005: EastSun Step9「釋放IC 並且破真空0.3秒」-- the Index nozzle's vacuum break pulse at the release.
#ifndef W906_HT9050_DRYRUN_BREAK_MS
#define W906_HT9050_DRYRUN_BREAK_MS 300
#endif
// Test seam: the dry run's clock (ms). 0 = std::chrono::steady_clock (the machine); tests/test_ht9050_dryrun.cpp sets a fake one.
unsigned long (*g_W906DryRunClockMs)() = 0;
// AI(W906-HT9050-DRYRUN-14) 20261005: test seams for the Index Z1 and the Index nozzle (Ht9050DryRun.h); 0 = the machine.
bool (*g_W906DryRunZ1Move)(int pos) = 0;
void (*g_W906DryRunZ1Stop)()        = 0;
void (*g_W906DryRunIdxVac)(int op)  = 0;
bool (*g_W906DryRunIdxVacMade)()    = 0;

namespace {
int         s_step   = 10;
int         s_cycles = 0;
bool        s_paused = false;
unsigned long s_waitSince = 0;                       // steady-clock ms when this step started waiting (0 = not waiting)
bool        s_waitShown = false;
int         s_lastStep = 0;
bool        s_xyDone[2] = { false, false };
bool        s_speedSet = false;                     // AI(W906-HT9050-DRYRUN-2): W906_HT9050_DRYRUN_SPEED applied since the last START / resume
bool        s_once = false;                         // AI(W906-HT9050-DRYRUN-14): the step's one-shot (Index vacuum on / release) is done
unsigned long s_since = 0;                          // AI(W906-HT9050-DRYRUN-14): when the step's one-shot was done (ms)
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
void Go(int next) { s_step = next; s_xyDone[0] = s_xyDone[1] = false; s_why.clear(); s_once = false; s_since = 0; }
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

// ---- AI(W906-HT9050-DRYRUN-14) 20261005: the Index Z1 (EastSun REQUEST §3「Index Z1 = MTestZ1，PCI-1203 軸走 Galil 路線」) ----
const int kZ1Home = 0;                              // the HT9050 Index Z1 home position (myGALILmotor.cpp AI(W906-HT9050-HOMEPOS0))
bool Z1Off() { return AxisOff(MTestZ1); }
// The dry-run % of the axis's JogHighSpeed (the route runs min(SP, PJogHighSpeed)); 30000 = the full HOME's Z1 speed (uhome.cpp W906_HomeTwoZ callers).
int Z1Speed()
{
    const int base = (MOT[MTestZ1].Motor != NULL && MOT[MTestZ1].Motor->PJogHighSpeed > 0) ? (int)MOT[MTestZ1].Motor->PJogHighSpeed : 30000;
    const int sp = (int)((long long)base * W906_HT9050_DRYRUN_SPEED / 100);
    return sp > 0 ? sp : 30000;
}
// true = arrived (Gali_MotMove's own answer: issued once, then true when the encoder is in range).
bool Z1Move(int pos)
{
    if (Z1Off()) return true;
    if (g_W906DryRunZ1Move) return g_W906DryRunZ1Move(pos);
    return MOT[MTestZ1].Gali_MotMove(pos, Z1Speed(), "W906_Ht9050DryRun");
}
// Gali_MotMove's "move pending" flags back to idle, as TfHome::InitGali_HomeTask (uhome.cpp) does for Z1: after a stop the dry run did not
// see (PAUSE: StopAllMotor) or did issue, the next Gali_MotMove re-issues the move instead of waiting for -- or alarming on -- the stopped one.
void Z1FlagsIdle()
{
    if (Z1Off()) return;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    IndexZCanMove[0] = IndexZCanMove[1] = true;
}
// Stop the Index Z1 through the Galil route: "ST", as the full HOME does (uhome.cpp ProcessMotorHome case 20). PCIL132_StopMotor does NOT
// stop it -- golden's guard returns for the four Index names (Motor/mymotor.cpp) -- and golden's PAUSE stops the Index with StopAllMotor's
// "VS0;SP0,0,0,0;" (Motor/myGALILmotor.cpp; the route halts the move, EtherCAT/Pci1203GaliRouteCore.h HALT / RESUME). "ST" cancels the move,
// so after it the flags go idle and the step issues the move again on START.
void Z1Stop()
{
    if (Z1Off()) return;
    if (g_W906DryRunZ1Stop) g_W906DryRunZ1Stop();
    else MOT[MTestZ1].Gali_Command("ST", "W906_Ht9050DryRun");
    Z1FlagsIdle();
}
// true = keep waiting (why set, the Index Z1 stopped if a move of it was under way)
bool Z1Blocked(bool ok, const char *why)
{
    if (ok) return false;
    s_why = why;
    if (Moving(MTestZ1) || MOT[MTestZ1].MovFlag) Z1Stop();
    return true;
}
// Z1 to its home, then "at home" (homed + still + +-20) must hold.
bool Z1ToHome()
{
    if (!Z1Move(kZ1Home)) return false;
    if (Z1Home()) return true;
    s_why = "Index Z1 up but not at home (not homed, or encoder outside +-20) / Index Z1 已上升但不在原點（沒回完原點，或 Encoder 不在正負 20 內）";
    return false;
}

// ---- AI(W906-HT9050-DRYRUN-14) 20261005: the Index nozzle (REQUEST §3: the golden Index pick / release IO) ----
enum { kVacOn = 1, kVacOff = 2, kBreakOff = 3 };
void IdxVac(int op)
{
    if (g_W906DryRunIdxVac) { g_W906DryRunIdxVac(op); return; }
    TMySucker &s = FTestSuck.Suck[0][0];
    if (op == kVacOn) s.On();                       // break off + vacuum on
    else if (op == kVacOff) s.Off();                // vacuum off + break on
    else s.OffDestroy();                            // break off
}
bool IdxVacMade()
{
    if (g_W906DryRunIdxVacMade) return g_W906DryRunIdxVacMade();
    TMySucker &s = FTestSuck.Suck[0][0];
    return s.Enable == false || s.W906_GetStatusAllOn();   // Enable=0: golden TMySucker::Sensor() answers true; a VC4 group: all 4 on
}
std::string VacWhy()
{
    std::string w = "Index vacuum not made yet (Index Z1 stays down until the IC is on the nozzle) / Index 真空尚未建立（沒吸到 IC，Index Z1 不上升）";
    if (!g_W906DryRunIdxVacMade && (LastSet.iRealDummy == DUMMY || LastSet.iRealDummy == HAS_TRAY))
        w += " [run mode DUMMY / HAS_TRAY: golden TMySucker::On() opens no vacuum / 執行模式 DUMMY／HAS_TRAY：原版 On() 不開真空]";
    return w;
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
            //  AI(W906-HT9050-DRYRUN-14): no shuttle move with the Index Z1 away -- asked only when In Shuttle 1 really has to move
            //  (it is normally already at Left here, and Step3 is where EastSun asks for the Z1 at home)
            if (!At(MInShuttle1, Prod.InSHT[0].iLeft) && Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MInShuttle1)) break;
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
            if (!Z1Home()) { s_why = "Index Z1 not at home (not homed, or encoder outside +-20) / Index Z1 不在原點（沒回完原點，或 Encoder 不在正負 20 內）"; break; }
            SpeedFor(MOutShuttle2, Prod.OutSHT[1].iLeft); SpeedFor(MOutShuttle1, Prod.OutSHT[0].iRight);   // AI(W906-HT9050-DRYRUN-3): acarry.cpp moves them
            if (!W906_Ht9050OutSht1ToRight(&why)) { s_why = why; break; }
            Go(40);
            break;
        // ---- Step4: In Shuttle 1 to the Index (Index Z1 not moved) ----
        case 40:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInShuttle1)) break;
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MInShuttle1)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MInShuttle1)) break;
            if (Move(MInShuttle1, Prod.InSHT[0].iRight)) Go(50);
            break;
        // ---- Step5: Index Z1 down to the In Shuttle pick height, Index vacuum on ----  AI(W906-HT9050-DRYRUN-14)
        case 50:
            if (Z1Blocked(At(MInShuttle1, Prod.InSHT[0].iRight), "In Shuttle 1 not at Right (the Index) / In Shuttle 1 不在右邊（Index 區）")) break;
            if (Z1Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊")) break;
            if (!s_once) { IdxVac(kVacOn); s_once = true; }
            if (Z1Move(Prod.TestZ1_Pick)) Go(60);
            break;
        // ---- Step6: up only with the IC on the nozzle, then Index Z1 to its home ----  AI(W906-HT9050-DRYRUN-14)
        case 60:
            if (!IdxVacMade()) { s_why = VacWhy(); break; }   // the Z1 stays down; a long wait shows the usual wait message
            Go(61);
            break;
        case 61: if (Z1ToHome()) Go(70); break;
        // ---- Step7: In Shuttle 1 back to Left ----  AI(W906-HT9050-DRYRUN-14)
        case 70:
            if (Blocked(HomeLamp(MInArmZA), "In Arm ZA HOME lamp off / In Arm ZA 原點燈沒亮", MInShuttle1)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MInShuttle1)) break;
            if (Move(MInShuttle1, Prod.InSHT[0].iLeft)) Go(80);
            break;
        // ---- Step8: Out Shuttle 1 into the Index / Step11 (1st half): back out ----
        case 80:
        case 110:
        {
            const int target = (s_step == 80) ? Prod.OutSHT[0].iLeft : Prod.OutSHT[0].iRight;
            if (s_step == 80 && Blocked(At(MInShuttle1, Prod.InSHT[0].iLeft), "In Shuttle 1 not at Left / In Shuttle 1 不在左邊", MOutShuttle1)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle1)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MOutShuttle1)) break;
            if (!At(MOutShuttle2, Prod.OutSHT[1].iLeft)) { StopIfMoving(MOutShuttle1); SpeedFor(MOutShuttle2, Prod.OutSHT[1].iLeft); W906_Ht9050OutSht2ToLeft(&why); s_why = why; break; }
            if (Move(MOutShuttle1, target)) Go(s_step == 80 ? 90 : 111);
            break;
        }
        // ---- Step9: Index Z1 down to the Out Shuttle place height, release (vacuum off + 0.3 s break) ----  AI(W906-HT9050-DRYRUN-14)
        case 90:
            if (Z1Blocked(At(MOutShuttle1, Prod.OutSHT[0].iLeft), "Out Shuttle 1 not at Left (the Index) / Out Shuttle 1 不在左邊（Index 區）")) break;
            if (Z1Blocked(At(MInShuttle1, Prod.InSHT[0].iLeft), "In Shuttle 1 not at Left / In Shuttle 1 不在左邊")) break;
            if (Z1Move(Prod.TestZ1_Place)) Go(91);
            break;
        case 91:
        {
            const unsigned long now = NowMs();
            if (!s_once) { IdxVac(kVacOff); s_once = true; s_since = now; }   // vacuum off + break on
            if (now - s_since < (unsigned long)W906_HT9050_DRYRUN_BREAK_MS) break;   // the break pulse, not a wait (no why)
            IdxVac(kBreakOff);
            Go(100);
            break;
        }
        // ---- Step10: Index Z1 up to its home ----  AI(W906-HT9050-DRYRUN-14)
        case 100: if (Z1ToHome()) Go(110); break;
        // ---- Step11 (2nd half): Out Shuttle 2 to Right, only with Out Shuttle 1 out of the Index (at Right) and the Out Arm ZA up ----
        //  AI(W906-HT9050-DRYRUN-4) 20261005: EastSun「Step7:將Out Shuttle1移動至右邊->這個是Out Shuttle2才對」+「先退出來，再動 Out Shuttle 2」
        case 111:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutShuttle2)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle2)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MOutShuttle2)) break;   // AI(W906-HT9050-DRYRUN-14)
            if (Move(MOutShuttle2, Prod.OutSHT[1].iRight)) Go(120);
            break;
        // ---- Step12: Out Arm to the Out Shuttle 1 pick, X first, then Y ----
        case 120: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(121); break;
        case 121:
            CylFor(false);                                 // AI(W906-HT9050-DRYRUN-4): the 3 s of the Off start here
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmX)) break;
            if (Move(MOutArmX, Tech.iOutArmShuttle1X)) Go(122);
            break;
        case 122:
            CylFor(false);
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmY)) break;
            if (Move(MOutArmY, Tech.iOutArmShuttle1Y)) Go(130);
            break;
        // ---- Step13: Out Arm picks ----
        case 130:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutArmZA)) break;
            if (Blocked(At(MOutShuttle2, Prod.OutSHT[1].iRight), "Out Shuttle 2 not at Right / Out Shuttle 2 不在右邊", MOutArmZA)) break;   // AI(W906-HT9050-DRYRUN-4)
            if (!CylFor(false)) { StopIfMoving(MOutArmZA); break; }   // AI(W906-HT9050-DRYRUN-4): 3 s after Off, no sensor (was OffSensor)
            if (Move(MOutArmZA, Tech.iOutArmShuttlePickZ2)) Go(131);
            break;
        case 131: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(140); break;
        // ---- Step14: Out Arm places on Auto1, Y (with C_OutArmSmallY On) first, then X ----
        case 140:
        {
            const bool cylDone = CylFor(true);             // AI(W906-HT9050-DRYRUN-4): On with Y; 3 s after On, no sensor (was OnSensor)
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmY)) break;
            if (!s_xyDone[1]) s_xyDone[1] = Move(MOutArmY, Tech.iOutArmAuto1Y);
            if (!s_xyDone[1] || !cylDone) break;
            Go(141);
            break;
        }
        case 141:
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutArmX)) break;
            if (Move(MOutArmX, Tech.iOutArmAuto1X)) Go(142);
            break;
        case 142: if (Move(MOutArmZA, Tech.iOutArmPlaceZ2)) Go(143); break;
        case 143: if (ZUp(MOutArmZA, OutArmSafeZ())) Go(150); break;
        // ---- after Step14: Out Shuttle 2 back to Left (standby); Out Shuttle 1 out of the Index and the Out Arm ZA up ----
        //  AI(W906-HT9050-DRYRUN-4) 20261005: EastSun「Step 10完成後將Out Shuttle2移動至左邊待命」
        case 150:
            if (Blocked(At(MOutShuttle1, Prod.OutSHT[0].iRight), "Out Shuttle 1 not at Right / Out Shuttle 1 不在右邊", MOutShuttle2)) break;
            if (Blocked(HomeLamp(MOutArmZA), "Out Arm ZA HOME lamp off / Out Arm ZA 原點燈沒亮", MOutShuttle2)) break;
            if (Blocked(Z1Home(), "Index Z1 not at home / Index Z1 不在原點", MOutShuttle2)) break;   // AI(W906-HT9050-DRYRUN-14)
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
        Z1FlagsIdle();                               // AI(W906-HT9050-DRYRUN-14): the same for the Index Z1's Galil-route move
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
            Z1Stop();                                // AI(W906-HT9050-DRYRUN-14)
            if (s_step == 91 && s_once) { IdxVac(kBreakOff); s_once = false; }   // AI(W906-HT9050-DRYRUN-14): no break left on; a fresh 0.3 s pulse after START
            Note("dryrun: PAUSE at step " + std::to_string(s_step) + " (cycle " + std::to_string(s_cycles) + ") -- axes stopped");
        }
        return;
    }
    if (s_paused)
    {
        s_paused = false;
        s_xyDone[0] = s_xyDone[1] = false;   // the stop reset each axis's command; the step re-issues its moves
        Z1FlagsIdle();                       // AI(W906-HT9050-DRYRUN-14)
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
