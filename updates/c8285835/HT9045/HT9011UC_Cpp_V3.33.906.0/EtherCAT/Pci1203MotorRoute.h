// =============================================================================
//  EtherCAT/Pci1203MotorRoute.h -- the engine's TMyEtherCatMotor calls, routed
//  onto the PCIE-1203 through EastSun's control layer.
//
//  AI(W906-ECAT-ROUTE) 20260929: new file. INBOX 112 / RULINGS_20260929 #11
//  decision 1 = A; design docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md, Q1-Q11 at the
//  document's recommended defaults (user-approved). The interface the engine
//  sees is Motor/EcatMotorRoute.h; this is the body, linked into wb_serve only
//  (CMakeLists.txt, next to Pci1203IoRoute.cpp), installed by
//  W906_InstallPci1203MotorRoute() (tools/wb_serve.cpp, one call site).
//
//  WHAT IT CONNECTS
//      TMyMotor (golden, unchanged: door / lock / soft-limit / MotionDone gates)
//        -> HTMotor* = TMyEtherCatMotor, its `#else` arms (ht9045_motor has no HAVE_PCI1203)
//        -> TEcatMotorRoute (Motor/EcatMotorRoute.h)       <- installed here
//             reads : the 1203 monitor's Pci1203AxisSample   (no vendor call)
//             writes: TPci1203Control::Execute              (no vendor call HERE --
//                     tools/pci1203_control_gate.ps1 check 5d greps this file)
//      Axis identity: golden's (iBoardID, iPortID) = the monitor's (station,
//      stationAxis); the ONE opened slot that matches, 0 or 2+ refused -- the
//      same rule as WebMotorAccessLive.cpp LiveBackend::Resolve.
//
//  THE RULES THIS FILE EXISTS TO KEEP (design sections 3, 4, 8)
//    * pending (4.2, the most important one): after a state-changing command is
//      passed to Execute, MotionDone/Busy/homeDone must not answer from a
//      sample taken before it -- the axis stays "moving" until the monitor's
//      pollCount advances. Keyed by (station, axis), never by slot.
//    * Q11: a MOTION that was refused, failed, or only validated (DRY RUN) makes
//      the axis "never done" until the next stop reaches Execute. golden takes
//      the next READY as arrival (Motor/mymotor.cpp MotorMovePosition, fCMD) --
//      that golden defect is not copied.
//    * Q10 / 4.4: never ShowErrorMessage. A failure = a non-zero return, the
//      control layer's audit line, one rate-limited console line.
//      AI(W906-ENG1203) 20260929: + a per-axis LATCH for failed stops / motions, read by the
//      web side (Pci1203MotorRouteAxisFault below); a failed-stop line is never rate-limited.
//    * Q1: SetCmdPos / SetActPos refused on a DS402 drive -- and on a drive the
//      monitor cannot identify (the safe side of "never guess") -- the drive
//      defines the origin (EastSun). A card-side drive follows golden.
//    * Q5: a Direction=1 row is never claimed (ruling 6B), with the reason printed.
//    * Q7 (the half this file can do): effect-identical engine writes are
//      skipped so the per-tick StopAllMotor / SetSpeed flood does not wash
//      EastSun's 512-line audit: a StopDec (and the ExtDrive(0) of the same
//      DecStop) when a sample newer than this route's last command AND than any
//      other Execute caller's last command reads READY; a speed equal to the
//      last SUCCESSFUL write that the card also reads back. The OTHER half -- one
//      per-axis "last issued pollCount" ledger inside Execute shared by Motor
//      Test, the pci1203 page and this route -- needs EtherCAT/Pci1203Control.cpp
//      (EastSun's file) and is NOT done here; see the .cpp for how the missing
//      ledger is made safe (foreign-Execute detection).
// =============================================================================
#ifndef ETHERCAT_PCI1203MOTORROUTE_H
#define ETHERCAT_PCI1203MOTORROUTE_H

#include "Motor/EcatMotorRoute.h"
#include "EtherCAT/Pci1203Control.h"
#include "EtherCAT/Pci1203Monitor.h"

namespace ht9045 {

// Everything the route body touches, as a seam. The live one reads Pci1203Monitor()
// and writes Pci1203Control(); tests/test_pci1203_motor_route.cpp supplies a fake.
class Pci1203MotorRouteEnv {
public:
    virtual ~Pci1203MotorRouteEnv() {}
    virtual bool CardUsable() = 0;                       // monitor present, Open_(), !Disabled(), card().open
    virtual bool ControlArmed() = 0;                     // Pci1203Control() != 0
    virtual unsigned long PollCount() = 0;               // card().pollCount
    virtual unsigned long ExecCount() = 0;               // control acceptedCount()+refusedCount() -- any caller
    virtual int AxisCount() = 0;
    virtual const Pci1203AxisSample& Axis(int i) = 0;
    virtual int SlaveCount() = 0;
    virtual const Pci1203SlaveSample& Slave(int i) = 0;
    virtual Pci1203CmdResult Execute(const Pci1203Cmd& c) = 0;
};
Pci1203MotorRouteEnv& Pci1203MotorRouteLiveEnv();

// The same identity rules WebMotorAccessLive.cpp uses (LiveBackend::Resolve / Pci1203DriveKind),
// written once more here because that file is outside this change -- exported so the next edit of
// WebMotorAccessLive.cpp can call these instead of keeping two copies.
//   FindSlot : the ONE opened slot with station==station && stationAxis==stationAxis; -1 otherwise, *hits = matches
//   DriveKind: 1 = DS402 (driveIsSigmaX, a ring-0 slave at that station with CiA 402 or /SERVOPACK/),
//              0 = another drive (a profile was read and it was not 402), -1 = cannot tell
//   AI(W906-ENG1203) 20260929: review LOW-2 -- FindSlot also REFUSES (-1) when any matching opened slot is
//     stationAmbiguous (its station number is shared by another drive, Pci1203Monitor.h 1203PHYS-1), the rule
//     JsonBridge/ChanMotorPoints.cpp FindCardAxis applies. Stricter than WebMotorAccessLive.cpp's Resolve on
//     purpose: engine motion is automatic, no operator is watching which drive answers. *ambiguous = such slots.
int Pci1203MotorRouteFindSlot(Pci1203MotorRouteEnv& e, int station, int stationAxis, int* hits, int* ambiguous = 0);
int Pci1203MotorRouteDriveKind(Pci1203MotorRouteEnv& e, int slot);

// AI(W906-ENG1203) 20260929: review R10 / R15 -- the live environment's two conditions, as pure functions so a
// ctest can pin them (no monitor object can be opened without the SDK). LiveEnv only fetches the four facts /
// the control object and hands them here.
//   CardUsable: the monitor exists, holds a device handle (Open_), is NOT Disabled (a Disabled monitor keeps
//               valid=true samples nobody refreshes -- review P17-CPP-4) and its card sample says open.
//   ExecCount : every Execute any caller made (accepted + refused); 0 without a control object.
bool Pci1203MotorRouteCardUsableOf(bool monitorPresent, bool open_, bool disabled, bool cardOpen);
unsigned long Pci1203MotorRouteExecCountOf(const TPci1203Control* c);

// The pure op -> Pci1203Cmd mapping (no ledger, no dedup, no Q1). False for an unknown op / which.
//   motion        = Execute's IsMotion kinds (jog, moveAbs, moveRel) -- Q11 applies
//   stateChanging = can change the READY state or the position (stops, ExtDrive, SvOn, ResetError,
//                   SetCmdPos/SetActPos, motion) -- the pending rule applies; speed / limit writes do not
bool Pci1203MotorRouteMakeCmd(int op, int which, double v, int slot, Pci1203Cmd& out,
                              bool* motion, bool* stateChanging);

struct Pci1203MotorRouteStats {
    unsigned long calls;            // write-side entries (call / initCfg / homeStart)
    unsigned long executed;         // Pci1203Cmd handed to Execute
    unsigned long issued;           // ... and issued with SUCCESS
    unsigned long dry;              // ... accepted, not issued (DRY RUN)
    unsigned long refused;          // ... refused by Execute, or failed before it
    unsigned long failed;           // ... issued, vendor error
    unsigned long skippedStop;      // Q7: StopDec skipped (effect-identical)
    unsigned long skippedExtDrive;  // Q7: the ExtDrive(0) of a skipped StopDec
    unsigned long skippedSpeed;     // Q7: speed equal to the last successful write and the read-back
    unsigned long refusedCoord;     // Q1: SetCmdPos / SetActPos on a DS402 or unidentified drive
    unsigned long bindOk, bindRefused;
    //AI(W906-ENG1203) 20260929: review HIGH-1 / MEDIUM-1
    unsigned long foreignStops;     // stops of an engine-claimed axis that reached Execute WITHOUT this route (Pci1203MotorRouteNoteForeignStop)
    unsigned long stopFailed;       // engine stops (StopDec / StopEmg / the ExtDrive(0) of a DecStop) refused or failed -- golden WAR16122
    unsigned long motionFailed;     // engine motions (jog / MoveAbs / MoveRel / home) refused or failed (not DRY) -- golden WAR16122
    unsigned long linesShown, linesDropped;   // console lines printed (or, when quiet, that WOULD print) / dropped by the per-axis budget
    Pci1203MotorRouteStats()
        : calls(0), executed(0), issued(0), dry(0), refused(0), failed(0), skippedStop(0)
        , skippedExtDrive(0), skippedSpeed(0), refusedCoord(0), bindOk(0), bindRefused(0)
        , foreignStops(0), stopFailed(0), motionFailed(0), linesShown(0), linesDropped(0) {}
};
const Pci1203MotorRouteStats& Pci1203MotorRouteStatsNow();

// ---------------------------------------------------------------------------
//  AI(W906-ENG1203) 20260929: review MEDIUM-1 -- a failed stop is not silent.
//  golden raises WAR16122 when StopDec / ExtDrive(0) fails (golden myEthercatmotor.cpp:423-435), which stops the
//  machine and needs the operator. The route may not raise anything (Q10 / design 4.4: ShowErrorMessage re-enters
//  StopAllMotor -> DecStop -> the route), so every such failure is LATCHED per (station, axis) -- never cleared while
//  wb_serve runs -- and printed (a failed-stop console line is never dropped by the budget). The web side reads it
//  through /api/struct/motor/... diag.why (WebMotorAccessLive.cpp OverlaySnapshot). Refused / failed motion (golden
//  WAR16122 on MoveAbs, :1535-1538) is latched the same way; DRY is not a failure.
// ---------------------------------------------------------------------------
struct Pci1203MotorRouteFault {
    unsigned long stopFailed;       // this axis's engine stops that did not reach the card or came back with an error
    unsigned long motionFailed;     // this axis's engine motions refused / failed (Q11 keeps the axis "not done" anyway)
    unsigned long lastRet;          // the last one's code: the vendor ret, or a kEcRc* route code
    unsigned long lastPoll;         // pollCount when it happened
    std::string   lastWhat;         // e.g. "StopDec"
    Pci1203MotorRouteFault() : stopFailed(0), motionFailed(0), lastRet(0), lastPoll(0) {}
};
// false = the route has never seen this (station, axis).
bool Pci1203MotorRouteAxisFault(int station, int stationAxis, Pci1203MotorRouteFault& out);
// One line for the operator ("" when nothing is latched).
std::string Pci1203MotorRouteFaultText(const Pci1203MotorRouteFault& f);

// ---------------------------------------------------------------------------
//  AI(W906-ENG1203) 20260929: review HIGH-1 -- stops that do NOT go through this route.
//  WebMotorAccess's Stop1203 / Stop1203All (the alarm path W906_MotorAccessOnAlarm, Motor Test / Teach STOP, jog
//  release, the dead-man, home / loop cancels) and the pci1203 page's ax.stop / ax.emgStop call Execute directly.
//  Without this the route would not know: a home job stays active (a later HOMING -> READY reads as "homed") and
//  golden's fCMD stays set (the next READY reads as "arrived" -- MotorMovePosition, mymotor.cpp:5743-5745).
//  Call it right after that Execute with the command and its result. It acts only when
//    the engine route is installed (EcatMotorRoute() == this route's table) -- otherwise it returns at once and
//      touches nothing, so no build without WB_ENGINE_MOTOR_1203 changes by a bit;
//    the kind is kCmdAxStop / kCmdAxEmgStop and it went out (issued OK, or DRY: this route's own stop rule);
//    the monitor slot c.axis maps to a (station, axis) this route has CLAIMED.
//  Then: the home job is cancelled, and the axis is pending until a Poll that STARTED after the stop has ended
//  (pollCount > now + 1: the stop may have run inside a Poll, from the output-first yield hook). Q11's "stuck" is NOT
//  released -- only this route's own stop does that. Returns true and the (station, axis); the caller then does
//  golden PCIL132_StopMotor's bookkeeping on the engine rows (W906_EcForeignStopResetFcmd, Motor/EcatMotorRoute.h).
// ---------------------------------------------------------------------------
bool Pci1203MotorRouteNoteForeignStop(const Pci1203Cmd& c, const Pci1203CmdResult& r, int* station, int* stationAxis);

// ---------------------------------------------------------------------------
//  AI(W906-ENG1203) 20260929: review R9 -- the install decision, made ONCE by the preprocessor in the route TU
//  (W906_ENG1203_GATE) and used by the installer, so a ctest can see which branch this build compiled.
// ---------------------------------------------------------------------------
enum {
    kEcGateSim         = 1,   // SOFT_SIMULTE: never installed
    kEcGateOff         = 2,   // WB_ENGINE_MOTOR_1203 (or INSTALL_1203_MONITOR / WB_PUMP_1203_CONTROL) not defined -- the committed tree
    kEcGateNoStartRing = 3,   // all three, but WB_PUMP_1203_START_RING off: NOT installed (Q9)
    kEcGateArmedBuild  = 4    // installed at start, if Pci1203Control() is armed
};
int Pci1203MotorRouteInstallGate();

// The route table the installer hands to SetEcatMotorRoute. Tests install it themselves
// (with a fake env) -- the installer is the ONE production path (pci1203_control_gate.ps1 check 6).
// AI(W906-ENG1203) 20260929: review build #1 -- check 6 now proves it: outside tests/ the tree has exactly ONE
// SetEcatMotorRoute( / Pci1203MotorRouteTable( / W906_EcRouteSlot_( call site, the installer's.
const TEcatMotorRoute* Pci1203MotorRouteTable();
bool Pci1203MotorRouteInstalled();

// Tests only: 0 = back to the live env; Reset clears every per-axis ledger and the stats.
void Pci1203MotorRouteSetEnvForTest(Pci1203MotorRouteEnv* env);
void Pci1203MotorRouteResetForTest();
// Tests only: 0 = quiet (the default prints like the IO route: the first 30 lines, then every 500th).
void Pci1203MotorRouteSetVerboseForTest(bool on);

}  // namespace ht9045

// Called once from wb_serve after the 1203 control block (global name, declared inline at the
// call site like W906_InstallPci1203IoRoute).
void W906_InstallPci1203MotorRoute();

// AI(W906-ENG1203) 20260929: review HIGH-1 -- the wb_serve glue (defined in EtherCAT/Pci1203GaliRoute.cpp EOF since AI(W906-INDEXZ-1203) 20260930, was WebMotorAccessLive.cpp; first the Index Z1 Gali route's half):
// Pci1203MotorRouteNoteForeignStop, then W906_EcForeignStopResetFcmd for the (station, axis) it names. Called after
// every Execute that bypasses this route: WebMotorAccessLive.cpp LiveBackend::Pci1203Execute and tools/wb_serve.cpp's
// pci1203 page dispatcher. No engine route installed = nothing happens.
void W906_EngineRouteForeignStop(const ht9045::Pci1203Cmd& c, const ht9045::Pci1203CmdResult& r);

#endif  // ETHERCAT_PCI1203MOTORROUTE_H
