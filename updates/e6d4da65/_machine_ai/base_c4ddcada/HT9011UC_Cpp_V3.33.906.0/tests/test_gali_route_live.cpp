// =============================================================================
//  tests/test_gali_route_live.cpp -- AI(W906-INDEXZ-1203) 20260930 (ctest GaliRouteLive)
//
//  Review round 2 of a8c3b04e (INBOX 113, HT9050 Index Z1 Gali_* -> PCIE-1203): the LIVE half -- the real binding and
//  installer (EtherCAT/Pci1203GaliRoute.cpp) and the moved foreign-stop glue (W906_EngineRouteForeignStop, its EOF),
//  with a stand-in for the live environment (Pci1203MotorRouteEnv: the monitor's samples, CardUsable, ControlArmed,
//  Execute) and for the SDK / monitor facts (GaliRouteLiveSeams). No monitor, no control object, no vendor call; the
//  golden engine (MOT[], Gali_*) is the real god-stack.
//    C  the installer's decision path: called before the monitor exists -> "NOT routed" with the boot-order reason and
//       NOT latched; called after -> routed (the wb_serve boot order this protects is pinned by part S below and by
//       tools/pci1203_control_gate.ps1 check 6)
//    E(b) D1 is applied by the installer (M13 / M15 / M16 rows Enable 0 -> disabled) and a TMyEtherCatMotor at the
//       route's address refuses the install
//    E(a) / D2  the glue tells the route about a stop that bypassed it: an operator stop ends a halt (golden ST
//       bookkeeping), the alarm sweep's stop keeps it (MotorAccessAlarmSweep scope)
//    D7 every entry point is serialised (a second thread waits while a route call is inside Execute), an off-thread
//       Gali_Command sends nothing, an off-thread operator stop's bookkeeping is applied by the next owner-thread call
//    S  source pins: the one installer call site in tools/wb_serve.cpp comes after Pci1203MonitorEnable and before the
//       tick loop (C); GATE G22 / G7 / the WebStart and fNote setters of bStartKeyPressCheck are live, the TfMain
//       member exists (A)
//  Link shape: the god-stack of GaliRouteEngine + the five EtherCAT TUs + WebMotorAccess.cpp (the ledger exports and
//  the alarm-sweep scope). The torque hook's body (EtherCAT/Pci1203TorqueHook.cpp) needs WebMotorAccessLive.cpp and is
//  NOT linked: the two functions below stand in for it and only count the installer's call.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "database.h"                // HSys.MotTable / mapMotTable (the M13..M16 rows)
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "Motor/myGALILmotor.h"      // TMyGALILMotor / StopAllMotor
#include "Motor/myEthercatmotor.h"   // TMyEtherCatMotor (the engine-row refusal)
#include "Motor/GaliRoute.h"
#include "EtherCAT/Pci1203GaliRoute.h"
#include "EtherCAT/Pci1203GaliRouteCore.h"
#include "EtherCAT/Pci1203MotorRoute.h"
#include "WebMotorAccess.h"          // MotorAccessAlarmSweep
#include "canary_support.h"
#include "Config.h"                  // IniConfig.GaliPosRange
#include "CosFunction.h"             // CosFunction.bIndexProtect
#include "w906_test_motors.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern bool IndexZCanMove[2];

using namespace ht9045;

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_gali_route_live.cpp:%d]  %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- the torque hook stand-ins (EtherCAT/Pci1203TorqueHook.cpp is not linked, see the banner) -------------------
static int g_torqueInstalls = 0;
namespace ht9045 {
bool W906_InstallPci1203TorqueHook(unsigned long) { ++g_torqueInstalls; return true; }
bool Pci1203TorqueHookInstalled() { return g_torqueInstalls > 0; }
}

// --- the live-environment stand-in ---------------------------------------------------------------------------------
struct StandInEnv : Pci1203MotorRouteEnv {
    bool usable, armed;
    unsigned long poll;
    std::vector<Pci1203AxisSample> axes;
    Pci1203SlaveSample noSlave;
    std::vector<Pci1203Cmd> execs;
    void (*onExec)(const Pci1203Cmd&);          // D7 probe: runs inside Execute, i.e. inside the route's lock
    StandInEnv() : usable(true), armed(true), poll(10), onExec(0) {}
    bool CardUsable() override { return usable; }
    bool ControlArmed() override { return armed; }
    unsigned long PollCount() override { return poll; }
    unsigned long ExecCount() override { return (unsigned long)execs.size(); }
    int AxisCount() override { return (int)axes.size(); }
    const Pci1203AxisSample& Axis(int i) override { return axes[(std::size_t)i]; }
    int SlaveCount() override { return 0; }
    const Pci1203SlaveSample& Slave(int) override { return noSlave; }
    Pci1203CmdResult Execute(const Pci1203Cmd& c) override
    {
        execs.push_back(c);
        if (onExec) onExec(c);
        Pci1203CmdResult r;
        r.accepted = true; r.issued = true; r.ret = 0;
        return r;
    }
};
static StandInEnv g_env;
static bool DoorClosed() { return false; }

static void Fresh(unsigned state, double cmd, double act)
{
    ++g_env.poll;
    Pci1203AxisSample& a = g_env.axes[2];
    a.state = state; a.cmdPos = cmd; a.actPos = act; a.motionIO = 0x00004000ul;   // SVON
}
static int Kinds(Pci1203CmdKind k)
{
    int n = 0;
    for (std::size_t i = 0; i < g_env.execs.size(); ++i) if (g_env.execs[i].kind == k) ++n;
    return n;
}

static TMOTDATA* Row(const char* no, const char* alias, const char* card, int enable, int board, int port)
{
    TMOTDATA* r = new TMOTDATA();
    r->No = no; r->Alias = alias; r->CardModel = card;
    r->iEnable = enable; r->iBoardID = board; r->iPort = port; r->iDirection = 0; r->iIP = -1;
    r->dGearRatio = 1.0; r->dAcc = 1.0; r->dDec = 1.0;
    return r;
}
static void AddRow(int motIndex, TMOTDATA* r)
{
    char k[8];
    std::snprintf(k, sizeof(k), "M%02d", motIndex);
    char v[16];
    std::snprintf(v, sizeof(v), "%d", (int)HSys.MotTable.size());
    HSys.MotTable.push_back(r);
    HSys.mapMotTable[AnsiString(k)] = AnsiString(v);
}

static void FreshAxes()
{
    for (int k = 0; k < 4; ++k) {
        const int i = MTestY1 + k;
        MOT[i].Motor->Enable = true;                        // golden's ship arm (every index axis on)
        MOT[i].MovFlag = false;
        for (int l = 0; l < 10; ++l) MOT[i].Led[l] = false;
    }
    IndexZCanMove[0] = IndexZCanMove[1] = false;            // the port's initial {false,false}
}

// --- D7 lock probe --------------------------------------------------------------------------------------------------
static volatile LONG g_probeDone = 0;
static HANDLE g_probeThread = 0;
static DWORD WINAPI ProbeFaultFor(LPVOID)
{
    std::string t;
    Pci1203GaliRouteFaultFor(14, 0, t);                    // must wait for the route's lock
    ::InterlockedExchange(&g_probeDone, 1);
    return 0;
}
static bool g_probeSawWait = false;
static void ProbeOnExec(const Pci1203Cmd& c)
{
    if (c.kind != kCmdAxMoveAbs || g_probeThread != 0) return;
    DWORD tid = 0;
    g_probeThread = ::CreateThread(NULL, 0, &ProbeFaultFor, NULL, 0, &tid);
    ::Sleep(400);                                          // plenty for an unlocked FaultFor to finish
    g_probeSawWait = (::InterlockedCompareExchange(&g_probeDone, 0, 0) == 0);
}
static volatile LONG g_offDone = 0;
static DWORD WINAPI OffThreadCommand(LPVOID)
{
    MOT[MTestZ1].Gali_Command("SPY=900;ACY=1;DCY=1;PAY=-4000;BGY;");   // a golden move string from another thread
    const Pci1203Cmd stop = [] { Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = 2; return c; }();
    Pci1203CmdResult ok; ok.accepted = true; ok.issued = true; ok.ret = 0;
    W906_EngineRouteForeignStop(stop, ok);                  // an operator stop reported off the owner thread
    ::InterlockedExchange(&g_offDone, 1);
    return 0;
}

// --- source pins ----------------------------------------------------------------------------------------------------
static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string l;
    while (std::getline(f, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); out.push_back(l); }
    return true;
}
static std::string CodeOf(const std::string& l)      // the text before a // comment, /* */ removed (good enough for these lines)
{
    std::string s = l;
    for (;;) {
        const std::size_t a = s.find("/*");
        if (a == std::string::npos) break;
        const std::size_t b = s.find("*/", a + 2);
        s.erase(a, b == std::string::npos ? std::string::npos : b + 2 - a);
    }
    const std::size_t c = s.find("//");
    return c == std::string::npos ? s : s.substr(0, c);
}
static int FindLine(const std::vector<std::string>& v, const char* needle, bool codeOnly, int from = 0)
{
    for (std::size_t i = (std::size_t)from; i < v.size(); ++i) {
        const std::string t = codeOnly ? CodeOf(v[i]) : v[i];
        if (t.find(needle) != std::string::npos) return (int)i;
    }
    return -1;
}
static int CountCode(const std::vector<std::string>& v, const char* needle)
{
    int n = 0;
    for (std::size_t i = 0; i < v.size(); ++i) if (CodeOf(v[i]).find(needle) != std::string::npos) ++n;
    return n;
}
static bool StartsWith(const std::string& s, const char* p) { return s.compare(0, std::string(p).size(), p) == 0; }

int main()
{
    std::printf("=== test_gali_route_live (AI(W906-INDEXZ-1203) 20260930) ===\n");
    INDEX_MOTION_CARD = 0;
    USE_INDEX_ARM_AXES = IndexArm_4_Axis;
    IniConfig.GaliPosRange = 50;                            // the config.ini value GaliRouteEngine uses
    CosFunction.bIndexProtect = false;
    InitialOK = false;                                      // canary ShowMotorErrorMessage: record, early return   [AI(W906-JAM-STOP) 20260930: the golden body now -- stop half, Exception record, early return (golden note.cpp:1054-1065)]
    SystemStart = false;
    const char* names[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {
        const int i = MTestY1 + k;
        MOT[i].Motor = new TMyGALILMotor(k);
        MOT[i].SetAlias(i, names[k]);
        MOT[i].Motor->GearRatio = 1.0;
        MOT[i].Motor->PServoAlarmOn = true;
        MOT[i].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
    }
    W906_TestEnsureSimMotors();
    HTMotor* Z = MOT[MTestZ1].Motor;
    Z->PJogHighSpeed = 900000; Z->InitSpeed = 100; Z->SetAccDataBase(9000000.0); Z->SetDecDataBase(9000000.0);
    Z->PHomeHighSpeed = 6000; Z->PHomeLowSpeed = 300; Z->HomeDirection = true;
    FreshAxes();
    // machines/HT9050/Mot_Table.csv: M13 / M15 / M16 Enable 0, M14 = PCI1203 station 14 axis 0
    AddRow(MTestY1, Row("M13", "MTestY1", "PCI1203", 0, 13, 0));
    AddRow(MTestZ1, Row("M14", "MTestZ1", "PCI1203", 1, 14, 0));
    AddRow(MTestZ2, Row("M15", "MTestZ2", "PCI1203", 0, 15, 0));
    AddRow(MTestY2, Row("M16", "MTestY2", "PCI1203", 0, 16, 0));
    // the monitor stand-in: 4 slots, slot 2 = station 14 axis 0 (DS402), slot 0 another drive
    g_env.axes.resize(4);
    for (int s = 0; s < 4; ++s) { g_env.axes[s].valid = true; g_env.axes[s].state = 1; }
    g_env.axes[0].opened = true; g_env.axes[0].station = 3;  g_env.axes[0].stationAxis = 0;
    g_env.axes[2].opened = true; g_env.axes[2].station = 14; g_env.axes[2].stationAxis = 0; g_env.axes[2].driveIsSigmaX = true;
    g_env.axes[2].motionIO = 0x00004000ul;

    GaliRouteLiveSeams seams;
    seams.env = &g_env; seams.linked = 1; seams.monitorPresent = 1;
    Pci1203GaliRouteSetSeamsForTest(&seams);

    // ---- C: the installer's decision path ----------------------------------------------------------------------
    std::printf("-- C. the installer: before the monitor exists -> refused and NOT latched; after -> routed\n");
    Pci1203GaliRouteResetForTest(); W906_SetGaliRoute(0);
    Pci1203GaliRouteInstallForTest(kGaliGateOff);
    CHECK(!Pci1203GaliRouteInstalled() && !W906_GaliRouteOwns(MTestZ1));
    Pci1203GaliRouteInstallForTest(kGaliGateSim);
    CHECK(!Pci1203GaliRouteInstalled());
    seams.monitorPresent = 0; g_env.usable = false;        // wb_serve's OLD order: before Pci1203MonitorEnable
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(!Pci1203GaliRouteInstalled() && g_torqueInstalls == 0 && MOT[MTestY1].Motor->Enable && IndexZCanMove[1] == false);
    seams.monitorPresent = 1;                               // the monitor exists but the card did not open
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(!Pci1203GaliRouteInstalled());
    g_env.usable = true;                                    // after the monitor block: the card is open
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);   // the SAME installer again: a refusal did not latch
    CHECK(Pci1203GaliRouteInstalled() && W906_GaliRouteOwns(MTestZ1) && g_torqueInstalls == 1);
    CHECK(IndexZCanMove[0] && IndexZCanMove[1]);            // D3 = B
    // E(b): D1 -- the installer disabled the three absent index axes, never the routed one
    CHECK(!MOT[MTestY1].Motor->Enable && !MOT[MTestZ2].Motor->Enable && !MOT[MTestY2].Motor->Enable && MOT[MTestZ1].Motor->Enable);
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);   // installed: a second call changes nothing
    CHECK(g_torqueInstalls == 1);
    // E(b): a TMyEtherCatMotor at (14, 0) -- the engine motor route could claim the axis -> refused
    Pci1203GaliRouteResetForTest(); W906_SetGaliRoute(0); FreshAxes();
    HTMotor* saved = MOT[MTrayX].Motor;
    MOT[MTrayX].Motor = new TMyEtherCatMotor(1400);        // BoardID 14, Port 0
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(!Pci1203GaliRouteInstalled() && MOT[MTestY1].Motor->Enable);
    delete MOT[MTrayX].Motor;
    MOT[MTrayX].Motor = saved;
    // the M14 row not PCI1203 / two opened slots at the address -> refused
    HSys.MotTable[1]->CardModel = "SMC";
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(!Pci1203GaliRouteInstalled());
    HSys.MotTable[1]->CardModel = "PCI1203";
    g_env.axes[1].opened = true; g_env.axes[1].station = 14; g_env.axes[1].stationAxis = 0;
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(!Pci1203GaliRouteInstalled());
    g_env.axes[1].opened = false; g_env.axes[1].station = -1; g_env.axes[1].stationAxis = -1;
    // installed again for the rest
    Pci1203GaliRouteInstallForTest(kGaliGateArmedBuild);
    CHECK(Pci1203GaliRouteInstalled());

    // ---- the binding reaches the stand-in -------------------------------------------------------------------------
    std::printf("-- the live IO: golden Gali_MotMove -> the stand-in's Execute on slot 2\n");
    g_env.execs.clear();
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    SystemStart = true;
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);
    CHECK(Kinds(kCmdAxMoveAbs) == 1 && g_env.execs.back().axis == 2 && g_env.execs.back().value == 2000.0);
    Fresh(5, 2000.0, 700.0);
    CHECK(MOT[MTestZ1].Gali_MotMove(2000, 900) == false);

    // ---- E(a) / D2: the glue ------------------------------------------------------------------------------------------
    std::printf("-- E(a)/D2. W906_EngineRouteForeignStop: the alarm sweep keeps a halt, an operator stop ends it (golden ST)\n");
    Pci1203Cmd stop; stop.kind = kCmdAxStop; stop.axis = 2;
    Pci1203CmdResult ok; ok.accepted = true; ok.issued = true; ok.ret = 0;
    StopAllMotor(true);                                     // VS0;SP0 -> halted
    SystemStart = false;
    Fresh(1, 700.0, 700.0);
    {
        MotorAccessAlarmSweep sweep;                        // W906_MotorAccessOnAlarm's Stop1203 of every opened axis
        W906_EngineRouteForeignStop(stop, ok);
    }
    CHECK(MOT[MTestZ1].MovFlag == true);                    // halted: golden's move still in progress
    Fresh(1, 700.0, 700.0);
    MOT[MTestY1].Gali_Command("VS30000;SP10000,10000,10000,10000;");   // DoSystem G22 after START
    CHECK(Kinds(kCmdAxMoveAbs) == 2);                       // the sweep kept the halt: resumed
    Fresh(5, 2000.0, 900.0);
    StopAllMotor(true);                                     // halted again
    Fresh(1, 900.0, 900.0);
    MOT[MTestZ2].MovFlag = true;
    W906_EngineRouteForeignStop(stop, ok);                  // an operator stop (Motor Test STOP / the pci1203 page)
    CHECK(MOT[MTestZ1].MovFlag == false && MOT[MTestZ2].MovFlag == false);   // golden ST bookkeeping
    Fresh(1, 900.0, 900.0);
    MOT[MTestY1].Gali_Command("VS30000;SP10000,10000,10000,10000;");
    CHECK(Kinds(kCmdAxMoveAbs) == 2);                       // the halt ended: nothing resumes
    Pci1203Cmd other = stop; other.axis = 0;                // another drive's slot: not the route's
    MOT[MTestZ1].MovFlag = true;
    W906_EngineRouteForeignStop(other, ok);
    CHECK(MOT[MTestZ1].MovFlag == true);
    MOT[MTestZ1].MovFlag = false;

    // ---- D7: the lock, off-thread calls -----------------------------------------------------------------------------
    std::printf("-- D7. every entry point is serialised; an off-thread string sends nothing; off-thread bookkeeping deferred\n");
    Fresh(1, 900.0, 900.0);
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0;
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    g_env.onExec = &ProbeOnExec;
    SystemStart = true;
    MOT[MTestZ1].Gali_MotMove(3000, 900);                   // its MoveAbs runs ProbeOnExec INSIDE the route's lock
    g_env.onExec = 0;
    CHECK(g_probeThread != 0 && g_probeSawWait);            // the second thread was still waiting for the lock
    if (g_probeThread) { ::WaitForSingleObject(g_probeThread, 5000); ::CloseHandle(g_probeThread); }
    CHECK(::InterlockedCompareExchange(&g_probeDone, 0, 0) == 1);   // ... and got it afterwards
    const std::size_t nOff = g_env.execs.size();
    Fresh(5, 3000.0, 1000.0);
    MOT[MTestZ1].MovFlag = true;
    DWORD tid = 0;
    HANDLE h = ::CreateThread(NULL, 0, &OffThreadCommand, NULL, 0, &tid);
    ::WaitForSingleObject(h, 5000); ::CloseHandle(h);
    CHECK(::InterlockedCompareExchange(&g_offDone, 0, 0) == 1);
    CHECK(g_env.execs.size() == nOff);                      // nothing was sent from the other thread
    CHECK(MOT[MTestZ1].MovFlag == true);                    // the off-thread operator stop's bookkeeping is not done there ...
    long r = MOT[MTestZ1].Gali_Command("MG_BGY");           // ... but by the next owner-thread entry
    CHECK(MOT[MTestZ1].MovFlag == false && r == 1);         // (the off-thread move poisoned the axis: MG_BG answers 1)
    MOT[MTestZ1].Gali_Command("ST");                        // clears the poison
    SystemStart = false;

    // ---- S: source pins ---------------------------------------------------------------------------------------------
    std::printf("-- S. source pins (C: wb_serve boot order; A: G22 / G7 / the setters / the member are live)\n");
    {
        const std::string root = W906_SRC_ROOT;
        std::vector<std::string> ws, cs, st, fm;
        CHECK(ReadLines(root + "/tools/wb_serve.cpp", ws) && ReadLines(root + "/csystem.cpp", cs) &&
              ReadLines(root + "/WebStart.cpp", st) && ReadLines(root + "/forms/fMain.h", fm));
        const int call = FindLine(ws, "W906_InstallPci1203GaliRoute();", true);
        const int enable = FindLine(ws, "Pci1203MonitorEnable(ht9045::kPci1203TagAxes", true);
        const int yield = FindLine(ws, "SetYieldHook(&W906_OutputYieldHook)", true, enable < 0 ? 0 : enable);
        const int loop = FindLine(ws, "for (;;) {", true, call < 0 ? 0 : call);
        std::printf("   wb_serve.cpp: install call :%d, Pci1203MonitorEnable :%d, the monitor block's last line :%d, tick loop :%d\n",
                    call + 1, enable + 1, yield + 1, loop + 1);
        CHECK(CountCode(ws, "W906_InstallPci1203GaliRoute();") == 1);
        CHECK(call > 0 && enable > 0 && yield > enable && call > yield && loop > call && loop - call < 60);
        const int g22 = FindLine(cs, "GATE G22 -- golden csystem.cpp:4657-4672 VERBATIM below", false);
        CHECK(g22 > 0 && StartsWith(cs[(std::size_t)g22], "#if 1"));
        CHECK(g22 > 0 && CodeOf(cs[(std::size_t)g22 + 3]).find("ClearAllManualSuckTask();") != std::string::npos &&
              CodeOf(cs[(std::size_t)g22 + 4]).find("if(fMain->bStartKeyPressCheck==true)") != std::string::npos);
        const int g7 = FindLine(cs, "GATE G7 -- golden :16655", false);
        CHECK(g7 > 0 && StartsWith(cs[(std::size_t)g7], "#if 1") && CodeOf(cs[(std::size_t)g7 + 1]).find("fMain->bStartKeyPressCheck=true;") != std::string::npos);
        const int w5e = FindLine(st, "bStartKeyPressCheck = true;", true);
        CHECK(w5e > 0 && StartsWith(st[(std::size_t)w5e - 1], "#if 1"));
        CHECK(CountCode(ws, "fMain->bStartKeyPressCheck=true;") == 1);   // golden note.cpp:3670
        CHECK(CountCode(fm, "bool bStartKeyPressCheck = false;") == 1);
    }

    Pci1203GaliRouteResetForTest(); W906_SetGaliRoute(0);
    Pci1203GaliRouteSetSeamsForTest(0);
    CHECK(!W906_GaliRouteOwns(MTestZ1));
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_checks - g_fail, g_checks);
    return g_fail ? 1 : 0;
}
