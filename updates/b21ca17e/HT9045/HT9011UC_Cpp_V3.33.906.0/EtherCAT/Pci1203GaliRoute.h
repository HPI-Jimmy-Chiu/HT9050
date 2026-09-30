// =============================================================================
//  EtherCAT/Pci1203GaliRoute.h -- HT9050's Index Z1 (M14 MTestZ1), routed from
//  golden's Galil layer to the PCIE-1203.  wb_serve only.
//
//  AI(W906-INDEXZ-1203) 20260929: new file. RULINGS_20260929 section 5
//  (D1 = A, D3 = B, D4 = install the torque hook), RULINGS_20260928
//  section 6, RULINGS_20260926 sections 2 / 6 / 27. Design:
//  D:\HT9045\backup\night_tools_20260928\INDEXZ_1203_DESIGN_20260929.md
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo (cc426093's review, revert 94cd23d7): OFF by default, the engine
//  motor route's arming conditions, refuses without an open card (#5), D1 at install (#7), foreign stops, the
//  failed-stop latch on the web diag (#6).
//  AI(W906-INDEXZ-1203) 20260930: review round 2 of a8c3b04e --
//    C  the installer runs AFTER the 1203 monitor is enabled (tools/wb_serve.cpp, the "entering tick loop" line; it
//       used to run before Pci1203MonitorEnable, so the card was never open and the route never installed), refuses
//       with a boot-order reason when the monitor does not exist, and only a SUCCESSFUL install latches;
//    D2 the foreign-stop glue W906_EngineRouteForeignStop moved here from WebMotorAccessLive.cpp EOF (so a ctest links
//       it) and tells the route whether the stop is the ALARM sweep's (MotorAccessInAlarmSweep);
//    D7 every entry point is serialised by one lock; an operator stop seen off the owner thread has its golden ST
//       bookkeeping applied by the next owner-thread entry;
//    the torque hook (D4) moved to EtherCAT/Pci1203TorqueHook.cpp (it needs MotorAccessLiveBackend, which is
//    WebMotorAccessLive.cpp's; the installer calls W906_InstallPci1203TorqueHook).
//
//  WHAT IT CONNECTS
//      MOT[MTestZ1].Gali_*            (golden Index state machines, unchanged)
//        -> Motor/GaliRoute.h         (engine seam: 8 card tests + the Gali_Command
//                                      no-card hook + the SingalHome delegation)
//        -> TGaliRouteCore            (EtherCAT/Pci1203GaliRouteCore.h: Galil
//                                      strings -> Pci1203Cmd, pure)
//        -> this file's live IO:
//             reads : the 1203 monitor's axis sample for Mot_Table (BoardID, Port) --
//                     the ONE opened, unambiguous slot (Pci1203MotorRouteFindSlot, the
//                     engine motor route's rule, stricter than Motor Test's Resolve)
//             writes: TPci1203Control::Execute (EastSun's allowlist / DRY-LIVE /
//                     audit). No vendor call of its own (pci1203_control_gate check 5d).
//             ledger: Motor Test's per-axis "last command" poll mark is shared, so
//                     neither side acts on a sample older than the other's command;
//                     a STOP that bypassed the route is reported by the wb_serve glue
//                     (W906_EngineRouteForeignStop, this file's EOF).
//  and, in the same installer (D4), rs232.cpp's W906_Pci1203TorqueLimitHook
//  (kCmdAxTorqueLimitSet: 60E0h + 60E1h, both read back), which DoTestHeadMotor
//  case 12000 needs before the first press (atester.cpp:6810 -> rs232.cpp:950).
//
//  INSTALLED ONLY WHEN (else it prints why and nothing changes -- Motor/GaliRoute.h "byte for byte"):
//    the build: not SOFT_SIMULTE; WB_ENGINE_INDEXZ_1203 (MachineType.h EOF, OFF in the committed tree) +
//      INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL + WB_PUMP_1203_START_RING -- the engine motor route's
//      conditions (EtherCAT/Pci1203MotorRoute.cpp W906_ENG1203_GATE; Q9: without ring 0 a motion answers
//      SUCCESS and the axis does not move);
//    at start: GaliRouteInstallOk (EtherCAT/Pci1203GaliRouteCore.h): SDK linked, control armed, the monitor
//      EXISTS, card OPEN, INDEX_MOTION_CARD == 0, the M14 row PCI1203 / Enable 1 / BoardID+Port / Direction 0,
//      exactly one unambiguous monitor slot, MOT[MTestZ1] enabled, no other Gali route, no TMyEtherCatMotor row at
//      the same (BoardID, Port) (the engine route could claim it).
//
//  ⚠ NOT SOLVED HERE -- review #3 (documented, not invented; an EastSun question):
//    D1 disables M13 MTestY1 (HT9050 has no Index Y). Every golden guard that reads Y1 before Z1 goes down then
//    answers "safe" -- golden's own answer for a disabled axis: CheckYPosWhenZDown (Motor/mymotor.cpp:1736, only
//    with bIndexProtect), Gali_Two_ZAxis_Move's CheckYPos (Motor/myGALILmotor.cpp:3816-3825, only with bIndexProtect
//    and Pos<0), the Y1 term of CheckTestZ, and every MOT[MTestY1].Gali_ReadEncoderInRandge*/MaxRandge check (their
//    Enable==false arm returns true). The route adds NO substitute guard. What is physically under Z1 on HT9050 and
//    what keeps that space clear without a Y1 is EastSun's to answer. Until then the design's safe defaults stand:
//    D1 as ruled, bIndexProtect = 0, D13 / D63 = 0, WB_ENGINE_INDEXZ_1203 OFF, and the first automatic Index run at
//    a low ArmSpeed[IndexArm] with EastSun at the machine (design section 8 Q1 / Q7).
//
//  BEFORE ARMING (review round 2, F) -- CLOSED 20260930 by INBOX 118 (AI(W906-JAM-STOP)); the old text, for the history:
//    Z1 JAMs are SILENT on wb_serve. Every JAM path of the routed axis ends in ShowMotorErrorMessage (the encoder
//    range check in Gali_MotMove*, ScanIndexMotorCanMove, the TIMO latch -> ALM_MOTOR_MOVE ...). In wb_serve that is
//    the RECORDING sim canary_support.cpp:486-525, which records the JAM, clears SoftStop / SoftStart / fAllMotorHome
//    (golden note.cpp:1054-1055 / :1059) and returns (or raises WAR16101 for the bare "WAR"); it omits golden
//    note.cpp:1056-1058 -- StopAllMotor(); MOT[MTestY1].Gali_Command("ST"); IndexMotorBreakerOFF(); -- so on a Z1 JAM the machine is
//    neither stopped nor is Z1's move aborted nor its brake held. The INBOX 112 engine motor route has the same hole
//    (every TMyEtherCatMotor JAM goes through the same function).  NOW: that stand-in is #if 0; the golden body (stop half +
//    the note, forms/fNote_ShowError.cpp EOF) runs, wb_serve shows the note without waiting (ctest NoteMotorError).
//    Also open (an EastSun question, not decided here): the route's DP (header B of the Core) writes
//    Acm_AxSetCmdPosition on the drive's axis. golden sends the current encoder (TestZ1SetPos), so the command
//    position becomes the actual one; whether a CSP DS402 drive takes that without a jump is to be confirmed at the
//    machine.
// =============================================================================
#ifndef ETHERCAT_PCI1203GALIROUTE_H
#define ETHERCAT_PCI1203GALIROUTE_H

#include <string>

#include "EtherCAT/Pci1203Control.h"   // Pci1203Cmd / Pci1203CmdResult

namespace ht9045 {

class Pci1203MotorRouteEnv;              // EtherCAT/Pci1203MotorRoute.h

// Motor Test's per-axis ledger (WebMotorAccess.cpp g_issuedPoll, W4B-5), exported
// at WebMotorAccess.cpp EOF: the monitor pollCount at the last motion command on
// that 1203 monitor axis slot. AI(W906-INDEXZ-1203) 20260930 (D7): tick thread only -- the route calls them only on
// its owner thread (TGaliRouteCore's confinement rule; the live binding refuses them on any other thread).
void MotorAccessNoteIssuedAt(int axis1203, unsigned long pollCount);
bool MotorAccessLastIssued(int axis1203, unsigned long& pollCount);

bool Pci1203GaliRouteInstalled();
bool Pci1203TorqueHookInstalled();                               // EtherCAT/Pci1203TorqueHook.cpp
bool W906_InstallPci1203TorqueHook(unsigned long ownerThreadId); // EtherCAT/Pci1203TorqueHook.cpp (D4), called by the installer only

// AI(W906-INDEXZ) 20260930: a stop that reached TPci1203Control::Execute WITHOUT this route (the wb_serve glue
// W906_EngineRouteForeignStop, this file's EOF, calls it after EVERY such Execute). Acts only when the route is
// installed, the kind is kCmdAxStop / kCmdAxEmgStop, it went out (issued OK, or DRY) and c.axis is the routed slot:
// TGaliRouteCore::NoteForeignStop, then golden "ST"'s bookkeeping (W906_GaliRouteForeignStopBookkeeping) when the
// core asks for it. AI(W906-INDEXZ-1203) 20260930 (D2): alarmSweep = the stop is W906_MotorAccessOnAlarm's sweep of
// every opened axis (a halted Z1 job then stays halted); false = an operator stop (golden "ST": the halt ends).
// true = it was ours. No route installed = returns false at once.
bool Pci1203GaliRouteNoteForeignStop(const Pci1203Cmd& c, const Pci1203CmdResult& r, bool alarmSweep);

// AI(W906-INDEXZ) 20260930: review #6 -- the latched failed stops / motions for the web diag of (station,
// stationAxis) (WebMotorAccessLive.cpp OverlaySnapshot). false = not the routed axis, or nothing latched.
bool Pci1203GaliRouteFaultFor(int station, int stationAxis, std::string& text);

// ---------------------------------------------------------------------------
//  AI(W906-INDEXZ-1203) 20260930: TESTS ONLY (tests/test_gali_route_live.cpp) -- stand-ins for the live facts the
//  binding reads, and the installer body with a forced gate. Production never calls these (the control gate's
//  check 6 counts the installer's call sites outside tests\).
// ---------------------------------------------------------------------------
struct GaliRouteLiveSeams {
    Pci1203MotorRouteEnv* env;          // monitor samples / CardUsable / ControlArmed / Execute (0 = Pci1203MotorRouteLiveEnv())
    int                   linked;       // -1 = Pci1203ControlLinked()
    int                   monitorPresent;   // -1 = Pci1203Monitor() != 0
    GaliRouteLiveSeams() : env(0), linked(-1), monitorPresent(-1) {}
};
void Pci1203GaliRouteSetSeamsForTest(const GaliRouteLiveSeams* s);   // 0 = live
void Pci1203GaliRouteResetForTest();                                // forget every state (the caller also runs W906_SetGaliRoute(0): control gate check 6 allows one slot writer outside tests\)
void Pci1203GaliRouteInstallForTest(int gate);                      // the installer body with this gate (kGaliGate*)

}  // namespace ht9045

// wb_serve calls this once, after the 1203 monitor block (tools/wb_serve.cpp, the "entering tick loop" line).
void W906_InstallPci1203GaliRoute();

#endif  // ETHERCAT_PCI1203GALIROUTE_H
