// =============================================================================
//  EtherCAT/Pci1203GaliRoute.cpp -- see the header for what this connects and why.
//
//  AI(W906-INDEXZ-1203) 20260929: new file, wb_serve only (CMakeLists.txt, next to
//  Pci1203IoRoute.cpp). ⚠ This TU makes NO vendor call of its own: reads are the
//  1203 monitor's samples, writes go through TPci1203Control::Execute. The only
//  vendor names in it are in comments (tools/pci1203_control_gate.ps1 check 5d).
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo -- the install gate GaliRouteCompiledGate() (the engine motor route's
//  conditions, OFF in the committed tree), GaliRouteInstallOk (card open, review #5), D1 at install
//  (review #7), the engine motor route's slot rule, foreign stops, the fault text for the web diag (#6).
//  AI(W906-INDEXZ-1203) 20260930: review round 2 (header C, D2, D7): the monitor-present fact and a latch on success
//  only, the foreign-stop glue (from WebMotorAccessLive.cpp EOF) with the alarm-sweep flag, one lock over every entry
//  point, the torque hook moved to EtherCAT/Pci1203TorqueHook.cpp, test seams. Also linked by
//  tests/test_gali_route_live.cpp (GaliRouteLive).
//  AI(W906-INDEXZ2) 20261002: LiveIo::OrgHome -- Z1's HOME lamp from the engine's ORG rule (HT9050: per axis by SensorType).
// =============================================================================
#include "MachineType.h"   // first, as in every route TU (the install decision itself is GaliRouteCompiledGate(), EtherCAT/Pci1203GaliRouteCore.cpp)
#include "EtherCAT/Pci1203GaliRoute.h"
#include "EtherCAT/Pci1203GaliRouteCore.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

#include "vclcompat/vcl_compat.h"      // AnsiString; <windows.h> (CRITICAL_SECTION, GetCurrentThreadId)
#include "database.h"                  // HSys.mapMotTable / HSys.MotTable (the M14 row checks, D1)
#include "cmydef.h"                    // MTestZ1 / INDEX_MOTION_CARD
#include "Motor/mymotor.h"             // MOT[]
#include "Motor/HTMotor.h"             // HTMotor (PJogHighSpeed / InitSpeed / Acc/Dec DB)
#include "Motor/GaliRoute.h"           // TGaliRoute / W906_SetGaliRoute / D1 / the foreign-stop bookkeeping
#include "Motor/EcatMotorRoute.h"      // W906_EcEngineMotorsAt (one owner per axis) / W906_EcForeignStopResetFcmd (the glue)
#include "EtherCAT/Pci1203Monitor.h"   // Pci1203Monitor()
#include "EtherCAT/Pci1203Control.h"   // Pci1203Control() / Pci1203ControlLinked()
#include "EtherCAT/Pci1203MotorRoute.h" // the engine motor route's slot / drive-kind / card rules (reused, not copied) + its foreign-stop half
#include "WebMotorAccess.h"            // MotorAccessInAlarmSweep (the glue, D2)

extern bool IndexZCanMove[2];          // ainarm9045_w7_shims.cpp:57 (port {false,false}); golden Motor/myGALILmotor.cpp:50 {true,true}

namespace ht9045 {

namespace {

bool           s_installed = false;
DWORD          s_tid = 0;              // the thread that installed the route = the wb_serve tick loop
int            s_station = -1;         // Mot_Table (BoardID, Port) of the M14 row = the monitor's (station, stationAxis)
int            s_stationAxis = -1;
unsigned long  s_logN = 0;
TGaliRouteCore s_core;
bool           s_stDue = false;        // D7: an operator stop seen off the owner thread -- golden ST bookkeeping still to do
const GaliRouteLiveSeams* s_seams = 0; // tests only

// ---------------------------------------------------------------------------
//  AI(W906-INDEXZ-1203) 20260930 (D7): ONE lock over every entry point (the engine's Gali_Command no-card hook, the
//  delegated home, the foreign-stop glue, the web diag, the installer). CRITICAL_SECTION is re-entrant for the same
//  thread; the route never calls back into itself on another thread. Initialised at static-init time (every entry
//  point runs after main starts).
// ---------------------------------------------------------------------------
CRITICAL_SECTION s_cs;
struct CsInit { CsInit() { ::InitializeCriticalSection(&s_cs); } } s_csInit;
struct Locked {
    Locked()  { ::EnterCriticalSection(&s_cs); }
    ~Locked() { ::LeaveCriticalSection(&s_cs); }
};

Pci1203MotorRouteEnv& Env() { return (s_seams && s_seams->env) ? *s_seams->env : Pci1203MotorRouteLiveEnv(); }
bool LinkedNow()            { return (s_seams && s_seams->linked >= 0) ? s_seams->linked != 0 : Pci1203ControlLinked(); }
bool MonitorPresentNow()    { return (s_seams && s_seams->monitorPresent >= 0) ? s_seams->monitorPresent != 0 : Pci1203Monitor() != 0; }
bool OnOwner()              { return ::GetCurrentThreadId() == s_tid; }

class LiveIo : public IGaliRouteIo {
public:
    bool CardOpen() override { return Env().CardUsable(); }   // a detach also bumps pollCount (Pci1203Monitor.cpp:2099)
    unsigned long PollCount() override { return Env().PollCount(); }
    int Slot(std::string& why) override
    {
        int hits = 0, amb = 0;
        const int s = Pci1203MotorRouteFindSlot(Env(), s_station, s_stationAxis, &hits, &amb);
        if (s < 0) {
            char b[160];
            std::snprintf(b, sizeof(b), "the monitor has %d opened slot(s) at station %d axis %d (%d stationAmbiguous) -- exactly 1 is required",
                          hits, s_station, s_stationAxis, amb);
            why = b;
        }
        return s;
    }
    bool Sample(int slot, GaliRouteSample& s) override
    {
        Pci1203MotorRouteEnv& e = Env();
        if (slot < 0 || slot >= e.AxisCount()) return false;
        const Pci1203AxisSample& a = e.Axis(slot);
        if (!a.valid || !a.opened) return false;
        s.state = a.state; s.motionIO = a.motionIO; s.cmdPos = a.cmdPos; s.actPos = a.actPos;
        return true;
    }
    int DriveKind(int slot) override { return Pci1203MotorRouteDriveKind(Env(), slot); }
    Pci1203CmdResult Exec(const Pci1203Cmd& c) override
    {
        //  Straight to Execute, not WebMotorAccess's Pci1203Execute: that one prints every call and runs the
        //  foreign-stop glue (a route stop is not foreign to the route), and G04 sends ST + VS0;SP0 every tick
        //  while the motor power is off. Every call is still in Pci1203Control's own audit log.
        return Env().Execute(c);
    }
    void Caps(GaliRouteCaps& c) override
    {
        c = GaliRouteCaps();
        HTMotor* M = MOT[MTestZ1].Motor;
        if (M == 0) return;
        c.jogHigh   = M->PJogHighSpeed;
        c.initSpeed = M->InitSpeed;
        c.accDb     = M->GetAccDataBase();
        c.decDb     = M->GetDecDataBase();
    }
    unsigned long NowMs() override { return (unsigned long)::GetTickCount(); }
    void NoteIssued(int slot, unsigned long poll) override { if (OnOwner()) MotorAccessNoteIssuedAt(slot, poll); }   // D7: the ledger is tick-thread-only
    bool LastIssued(int slot, unsigned long& poll) override { return OnOwner() && MotorAccessLastIssued(slot, poll); }
    bool OnOwnerThread() override { return OnOwner(); }
    void SleepMs(int ms) override { if (ms > 0) ::Sleep((DWORD)ms); }
    //AI(W906-INDEXZ2) 20261002: Z1's HOME from the ENGINE's rule, not a copy of it: W906_Ht9050OrgHome (Motor/mymotor.h; the
    //  hook WebMotorAccessLive.cpp W906_HookHt9050OrgHome = WebMotorAccess.cpp MotorAccessTeachHomeLed: on HT9050
    //  (W906_GpibModel "9050GPIB") a 1203 axis's ORG bit is read by its Mot_Table SensorType -- 1: LOW = at home, 0: HIGH,
    //  AI(W906-HT9050-ORG-ST); -1 no sample / none since the last command / no golden motor object; -2 not HT9050 or no
    //  hook). It reads the same monitor sample through MotorAccessLiveBackend(), so the TS byte golden's
    //  Gali_ScanMotStatus decodes and TMyMotor::ScanMotorStatus's override give Z1 the same lamp.
    int OrgHome(int, unsigned long) override { return W906_Ht9050OrgHome(MTestZ1); }
    void Log(const std::string& line) override
    {
        ++s_logN;
        if (!(s_logN <= 50 || (s_logN % 200) == 0)) return;
        std::printf("index Z -> 1203 #%lu: %s\n", s_logN, line.c_str());
        if (s_logN == 50) std::printf("index Z -> 1203: further lines only every 200th (failed stops / motions always)\n");
    }
    void LogForce(const std::string& line) override
    {
        ++s_logN;
        std::printf("index Z -> 1203 #%lu: %s\n", s_logN, line.c_str());   // review #6: never dropped (TGaliRouteCore::Force holds back repeats of a KEY for 10 s, counted)
    }
};
LiveIo s_io;

// D7: an operator stop's golden ST bookkeeping that arrived off the owner thread is applied here, on it.
void ApplyDueBookkeeping()
{
    if (!s_stDue || !OnOwner()) return;
    s_stDue = false;
    const bool cleared = W906_GaliRouteForeignStopBookkeeping(MTestZ1);
    std::printf("index Z -> 1203: the golden ST bookkeeping of an off-thread operator stop applied now%s\n", cleared ? "" : " (route gone: nothing)");
}

bool RouteCommand(int receiver, const char* data, long* reply)
{
    (void)receiver;                    // a Galil string addresses axes by letter, not by receiver
    Locked l;
    ApplyDueBookkeeping();
    return s_core.Command(data, reply);
}
int RouteHomeStart(int motName, bool homeDirection, unsigned homeHigh, unsigned homeLow, double acc, double dec)
{
    (void)motName;
    Locked l;
    ApplyDueBookkeeping();
    return s_core.HomeStart(homeDirection, homeHigh, homeLow, acc, dec);
}
int RouteHomePoll(int motName)
{
    (void)motName;
    Locked l;
    ApplyDueBookkeeping();
    return s_core.HomePoll();
}

TGaliRoute s_route = { -1, RouteCommand, RouteHomeStart, RouteHomePoll };   // owner set at install (MTestZ1)

// The Mot_Table row of MOT[i] ("M%02d", cinitial.cpp:3879-3883's lookup), or 0.
const TMOTDATA* RowOf(int i)
{
    AnsiString k;
    k.sprintf("M%02d", i);
    std::map<AnsiString, AnsiString>::iterator it = HSys.mapMotTable.find(k);
    const int r = (it == HSys.mapMotTable.end()) ? -1 : std::atoi(it->second.c_str());
    return (r >= 0 && r < (int)HSys.MotTable.size()) ? HSys.MotTable[r] : 0;
}
bool InTable(int i)
{
    const TMOTDATA* row = RowOf(i);
    return row != 0 && row->iEnable != 0;   // cc426093's D1 expression `bHasMotor && iEnable!=0` (was cinitial.cpp:3956)
}

// ---------------------------------------------------------------------------
//  The installer body. AI(W906-INDEXZ-1203) 20260930 (C): only a SUCCESSFUL install latches (it used to latch on the
//  first call, so a call before the monitor existed refused for good); the monitor-present fact names a boot-order
//  bug instead of "card not open".
// ---------------------------------------------------------------------------
void InstallWithGate(int gate)
{
    Locked l;
    //  AI(W906-INDEXZ) 20260930: the branches are chosen by the build's gate (GaliRouteCompiledGate, Core TU) at run
    //  time, so the armed branch below is compiled -- type-checked -- in every build, armed or not.
    if (gate == kGaliGateSim) {
        std::printf("index Z -> 1203: SIM build -- route NOT installed, MTestZ1 keeps golden's SOFT_SIMULTE branch (Enable=false); torque hook NOT installed\n");
        return;
    }
    if (gate == kGaliGateNoStartRing) {
        std::printf("index Z -> 1203: NOT routed (WB_PUMP_1203_START_RING is off -- ring 0 not started, a Z1 motion would report SUCCESS without moving; the engine route's Q9) -- MTestZ1 stays on golden's no-card Galil branch\n");
        return;
    }
    if (gate != kGaliGateArmedBuild) {
        std::printf("index Z -> 1203: NOT routed (needs WB_ENGINE_INDEXZ_1203 + INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL + WB_PUMP_1203_START_RING; one of the first three is not defined in this build) -- MTestZ1 stays on golden's no-card Galil branch\n");   //AI(W906-INDEXZ2) 20261002: was "WB_ENGINE_INDEXZ_1203 is off by default" -- D2 turned it on
        return;
    }
    if (s_installed) return;
    GaliRouteInstallFacts f;
    f.linked          = LinkedNow();
    f.controlArmed    = Env().ControlArmed();                 // Pci1203Control() != 0 (the live env)
    f.monitorPresent  = MonitorPresentNow();
    f.cardUsable      = Env().CardUsable();
    f.indexMotionCard = INDEX_MOTION_CARD;
    const TMOTDATA* row = RowOf(MTestZ1);
    f.haveRow = (row != 0);
    if (row) {
        f.cardModel = row->CardModel.c_str();
        f.enable = row->iEnable; f.boardId = row->iBoardID; f.port = row->iPort; f.direction = row->iDirection;
    }
    if (f.cardUsable && row)
        Pci1203MotorRouteFindSlot(Env(), f.boardId, f.port, &f.slotHits, &f.slotAmbiguous);
    f.z1Present        = MOT[MTestZ1].Motor != 0 && MOT[MTestZ1].Motor->Enable;
    f.routeTaken       = W906_GaliRoute() != 0;
    f.engineRowsAtAddr = row ? W906_EcEngineMotorsAt(f.boardId, f.port) : 0;
    std::string why;
    if (!GaliRouteInstallOk(f, why)) {
        std::printf("index Z -> 1203: NOT routed -- %s. MTestZ1 stays on golden's no-card Galil branch (M13/M15/M16 stay as golden built them); torque hook NOT installed\n", why.c_str());
        return;
    }
    s_station = f.boardId;
    s_stationAxis = f.port;
    s_tid = ::GetCurrentThreadId();
    s_stDue = false;
    s_route.owner = MTestZ1;
    s_core.Bind(&s_io);
    IndexZCanMove[0] = true;           // D3 = B: golden's initial {true,true} (golden Motor/myGALILmotor.cpp:50), only with the route
    IndexZCanMove[1] = true;
    W906_SetGaliRoute(&s_route);
    s_installed = true;
    const int off = W906_GaliRouteDisableAbsentIndexAxes(InTable(MTestY1), InTable(MTestZ2), InTable(MTestY2));   // D1 = A, review #7
    const bool tq = W906_InstallPci1203TorqueHook((unsigned long)s_tid);   // D4 (rs232.cpp:2275: without it the first press is "torque set error")
    TPci1203Control* ctl = Pci1203Control();
    std::printf("index Z -> 1203: ROUTED -- MOT[%d] %s (station %d axis %d) Gali_* -> 1203 monitor samples / Pci1203Control;"
                " IndexZCanMove={1,1}; %d absent index axis(es) disabled from the table (D1); torque hook %s (%s)\n",
                MTestZ1, row->Alias.c_str(), row->iBoardID, row->iPort, off, tq ? "installed" : "NOT installed",
                ctl == 0 ? "no control object in this process (a test stand-in)"
                : ctl->IsDryRun() ? "DRY RUN: every Z1 move is refused and Z1 reports its alarm"
                                  : "*** LIVE: the Index flow moves M14 -- first automatic run with EastSun present ***");
}

}  // namespace

bool Pci1203GaliRouteInstalled() { return s_installed && W906_GaliRoute() == &s_route; }

bool Pci1203GaliRouteNoteForeignStop(const Pci1203Cmd& c, const Pci1203CmdResult& r, bool alarmSweep)
{
    Locked l;
    if (!Pci1203GaliRouteInstalled()) return false;                   // no route: touch nothing
    if (c.kind != kCmdAxStop && c.kind != kCmdAxEmgStop) return false;
    if (!r.accepted || (r.issued && r.ret != 0)) return false;        // it did not go out: the route's own rule
    const int ans = s_core.NoteForeignStop(c.axis, alarmSweep);
    if (ans == kGaliForeignNotOurs) return false;
    bool cleared = false, deferred = false;
    if (ans == kGaliForeignMoveAborted || ans == kGaliForeignStBookkeeping) {
        if (OnOwner()) cleared = W906_GaliRouteForeignStopBookkeeping(MTestZ1);
        else { s_stDue = true; deferred = true; }                      // D7: MOT[] is the tick thread's
    }
    std::printf("index Z -> 1203: stop outside the route on slot %d (%s, %s)%s%s\n", c.axis, Pci1203CmdName(c.kind),
                alarmSweep ? "the alarm sweep" : "an operator stop = golden ST",
                cleared ? " -> golden ST bookkeeping: MovFlag=false on the index axes (golden re-issues its move)" : "",
                deferred ? " -> golden ST bookkeeping DEFERRED to the owner thread (off-thread caller)" : "");
    return true;
}

bool Pci1203GaliRouteFaultFor(int station, int stationAxis, std::string& text)
{
    Locked l;
    text.clear();
    if (!Pci1203GaliRouteInstalled() || station != s_station || stationAxis != s_stationAxis) return false;
    text = GaliRouteFaultText(s_core.Fault());
    return !text.empty();
}

void Pci1203GaliRouteSetSeamsForTest(const GaliRouteLiveSeams* s) { Locked l; s_seams = s; }
void Pci1203GaliRouteResetForTest()
{
    Locked l;
    //  The engine slot itself is NOT cleared here: tools/pci1203_control_gate.ps1 check 6 allows exactly one slot writer
    //  outside tests\ (the installer). The test uninstalls it (W906_SetGaliRoute(0)) itself.
    s_installed = false;
    s_tid = 0;
    s_station = s_stationAxis = -1;
    s_stDue = false;
    s_route.owner = -1;
    s_core.Bind(0);
}
void Pci1203GaliRouteInstallForTest(int gate) { InstallWithGate(gate); }

}  // namespace ht9045

void W906_InstallPci1203GaliRoute()
{
    ht9045::InstallWithGate(ht9045::GaliRouteCompiledGate());
}

//AI(W906-ENG1203) 20260929: review HIGH-1 (INBOX 112) -- the wb_serve glue for a stop that reached
//  TPci1203Control::Execute WITHOUT the engine motor route (declared in EtherCAT/Pci1203MotorRoute.h).
//  Called right after LiveBackend::Pci1203Execute's Execute (every WebMotorAccess 1203 command) and after the pci1203
//  page dispatcher's (tools/wb_serve.cpp W906_Dispatch1203Ex). Two halves, both no-ops unless their route is installed:
//    1. the route's ledger (Pci1203MotorRouteNoteForeignStop: home job cancelled, axis pending past the next full Poll);
//    2. golden PCIL132_StopMotor's bookkeeping on the engine rows of that (station, axis): fCMD=false
//       (W906_EcForeignStopResetFcmd, Motor/EcatMotorRoute.cpp) -- so a move the alarm path stopped short is
//       re-issued after RETRY instead of reading as arrived (golden MotorMovePosition, mymotor.cpp:5743-5745).
//  Same thread as every other Execute (the tick thread); prints, never raises (Q10).
//AI(W906-INDEXZ-1203) 20260930: MOVED here from WebMotorAccessLive.cpp EOF (review round 2, E(a): a ctest --
//  tests/test_gali_route_live.cpp -- links this TU, not that one), unchanged but for the Index Z1 half's alarm-sweep
//  flag (D2): W906_MotorAccessOnAlarm's Stop1203 of every opened axis is the port's 1203 half of golden StopAllMotor,
//  whose per-axis PCIL132_StopMotor skips the index axes -- a halted Z1 job stays halted; any other stop here is an
//  operator's (golden "ST").
void W906_EngineRouteForeignStop(const ht9045::Pci1203Cmd& c, const ht9045::Pci1203CmdResult& r)
{
    int station = -1, stationAxis = -1;  if (::W906_Pci1203CmdNote) ::W906_Pci1203CmdNote(c, r);   //AI(W906-BRAKE-SERVOFIRST) 20261004: the foreign commands (WebMotorAccess, the pci1203 page) reach the brake ledger too
    ht9045::Pci1203GaliRouteNoteForeignStop(c, r, ht9045::MotorAccessInAlarmSweep());   //AI(W906-INDEXZ) 20260930: INBOX 113 -- first the Index Z1 Gali_* route (M14, never an engine-route axis: the installer refuses when a TMyEtherCatMotor sits at its address). No Gali route installed = nothing
    if (!ht9045::Pci1203MotorRouteNoteForeignStop(c, r, &station, &stationAxis)) return;
    const int rows = W906_EcForeignStopResetFcmd(station, stationAxis);
    std::printf("engine motor -> 1203: foreign stop on st %d ax %d -> golden fCMD=false on %d engine row(s) (PCIL132_StopMotor's bookkeeping)\n",
                station, stationAxis, rows);
}
