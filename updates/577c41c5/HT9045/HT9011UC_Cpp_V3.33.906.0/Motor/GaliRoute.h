// =============================================================================
//  Motor/GaliRoute.h -- an installable route for ONE index axis of the Galil
//                       layer (TMyMotor::Gali_*), used for HT9050's Index Z1.
//
//  AI(W906-INDEXZ-1203) 20260929: new file.
//    RULINGS_20260928 section 6 (user: "HT9050 的 Index Z ... 接下來修改成 1203
//    控制，盡可能思考不動到動作流程 task"), RULINGS_20260926 section 27 R66-GALI = A
//    (route in the Gali_* layer by the axis's CardModel), RULINGS_20260929
//    section 5 (D1 = A, D3 = B, D4 = install the torque hook).
//    Design: D:\HT9045\backup\night_tools_20260928\INDEXZ_1203_DESIGN_20260929.md
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo (cc426093 reverted by 94cd23d7 after its review):
//    * D2 is now OFF (MachineType.h EOF `//#define WB_ENGINE_INDEXZ_1203`), installed only under the engine motor   [AI(W906-INDEXZ2) 20261002: D2 ON again as ruled; still installed only with WB_PUMP_1203_START_RING too]
//      route's arming conditions (EtherCAT/Pci1203GaliRoute.cpp);
//    * D1 (M13 / M15 / M16 disabled from the table) moved from InitialMotorParameter (cinitial.cpp) to the
//      INSTALLER (W906_GaliRouteDisableAbsentIndexAxes below) -- review #7: with it at construction an HT9050
//      ship build WITHOUT the route was not golden, so the "byte for byte" sentence below was false. It is true now;
//    * a stop that bypassed the route: W906_GaliRouteForeignStopBookkeeping (golden "ST"'s MovFlag clearing);
//    * Gali_Two_ZAxis_Move no longer flips a no-card Z2's Position every call (review #4, myGALILmotor.cpp).
//
//  WHY A ROUTE AND NOT A NEW MOTOR OBJECT
//    Every golden Index state machine calls MOT[MTestZ1].Gali_* directly
//    (docs/FLOW_COVERAGE_FT005054.md: AutoClean 42, atester 77, Front 66,
//    Rear 76, 32Site 40, uhome 11, csystem 11 call sites).  HT9050's Z1 (M14)
//    sits on the PCIE-1203 and no Galil card is ever opened (bGali_CardInstall
//    has no writer outside Open_GaliCard, which nobody calls), so today each of
//    those calls takes golden's "no card" branch: the steps run and Z1 never
//    presses down.  With a route installed for the axis:
//      * the eight per-axis card tests (Gali_ScanMotStatus / ScanAlarmStatus /
//        MotMove2 / MotMove / MotMoveNoWait / MotMoveSkipEncoder / ReadPos /
//        Two_ZAxis_Move) read "card installed OR this axis is routed", so the
//        golden CARD branch runs unchanged with all its own side effects
//        (MovFlag, settle count, encoder range check -> JAM, TIMO alarm latch);
//      * Gali_Command's no-card branch hands the Galil string to the route
//        AFTER golden's own "ST" MovFlag clearing and log; the route answers
//        exactly what atol() of the DMC reply would have been;
//      * Gali_SingalHome delegates at function level (the Galil HM/FI sensor
//        dance becomes the drive's own home; golden task numbers and flags kept);
//      * Gali_FindZPhase (D63, Galil-only) refuses -- never a fake success.
//    With NO route installed (every ctest but the two route tests, every
//    SOFT_SIMULTE build, every build without WB_ENGINE_INDEXZ_1203, a machine
//    whose M14 row is not PCI1203, a binary without the SDK, no open card) every
//    predicate reduces to golden's bGali_CardInstall, the hook returns false, and
//    nothing else in the engine consults the route: the behaviour is byte for byte
//    what it was (tests/test_gali_route_engine.cpp part 0 and
//    tests/test_machine_motors.cpp part (5) pin it).
//
//  WHO INSTALLS IT
//    wb_serve only: EtherCAT/Pci1203GaliRoute.cpp W906_InstallPci1203GaliRoute(),
//    after the 1203 monitor is enabled (AI(W906-INDEXZ-1203) 20260930: it used to run
//    right after W906_InstallPci1203MotorRoute, before the monitor existed, and so never
//    installed -- review round 2 C).  The engine library (ht9045_motor)
//    never gets HAVE_PCI1203 and includes no EtherCAT header -- this POD is the seam.
// =============================================================================
#ifndef MOTOR_GALIROUTE_H
#define MOTOR_GALIROUTE_H

class TMyMotor;

struct TGaliRoute {
    int  owner;   // TMyMotor::Mot_Name of the routed axis (MTestZ1); -1 = none
    // One Galil command string. true = claimed, *reply = what atol() of the DMC
    // reply would have been; false = not this axis's business (golden's no-card
    // branch then returns 0 as before). receiver = the Mot_Name the string was
    // sent through -- information only: a Galil string addresses axes by letter.
    bool (*command)(int receiver, const char* data, long* reply);
    // Start the drive's home. 0 = issued; anything else = refused (the route has
    // already marked the axis failed, so its "TI" answer reports the alarm).
    int  (*homeStart)(int motName, bool homeDirection, unsigned homeHigh, unsigned homeLow,
                      double acc, double dec);
    // 0 = still homing (also for ever after a stop cancelled the job -- golden never
    // finishes a stopped home), 1 = done (origin set, a sample newer than the home is
    // in), -1 = failed (ERROR_STOP, never entered HOMING within 5 s, 180 s, card lost).
    int  (*homePoll)(int motName);
};

void              W906_SetGaliRoute(const TGaliRoute* r);   // 0 = uninstall (tests)
const TGaliRoute* W906_GaliRoute();
bool              W906_GaliRouteOwns(int motName);
bool              W906_GaliRouteCommand(int receiver, const char* data, long* reply);

// AI(W906-INDEXZ) 20260930: D1 = A, applied by the INSTALLER (review #7). HT9050 has one index axis (Z1); golden's
// INDEX_MOTION_CARD==0 arm builds all four as enabled Galil axes (cinitial.cpp InitialMotorParameter), and an
// enabled axis with no card never finishes HOME step 1250 (uhome.cpp) and holds the Z1 interlock. Each of Y1 / Z2 /
// Y2 whose Mot_Table row is missing or has Enable 0 (the flag below is false) gets Motor->Enable=false. The routed
// axis itself is never touched. Returns how many were disabled. The only production caller is the installer.
int               W906_GaliRouteDisableAbsentIndexAxes(bool y1InTable, bool z2InTable, bool y2InTable);

// AI(W906-INDEXZ) 20260930: golden "ST"'s bookkeeping (golden Motor/myGALILmotor.cpp:577-592, the no-card branch)
// for a stop of the ROUTED axis that did not go through Gali_Command (Motor Test / Teach STOP, the alarm path's
// Stop1203 on every opened axis, jog release, dead-man, the pci1203 page): MOT[motName].MovFlag=false, so the
// golden move function re-issues its move on its next call instead of reading the short stop as arrival (the
// INBOX 112 review HIGH-1 rule; the engine route does golden's fCMD=false there). Only for the routed axis; no
// route = does nothing. Returns true when it cleared the flag.
// AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- all of golden ST's clearing (MovFlag of Y1 / Y2 (4-axis) / Z1 / Z2,
// bZ1Z2Exute of Y1 / Y2 (4-axis)), as Gali_Command's no-card "ST" branch does; the core decides when (operator stops always,
// the alarm sweep only when it cut a running move -- TGaliRouteCore::NoteForeignStop).
bool              W906_GaliRouteForeignStopBookkeeping(int motName);

// Engine-internal (Motor/myGALILmotor.cpp EOF): the delegated bodies.
bool W906_GaliRoutedSingalHome(TMyMotor& m);
bool W906_GaliRoutedFindZPhase(TMyMotor& m);

#endif  // MOTOR_GALIROUTE_H
