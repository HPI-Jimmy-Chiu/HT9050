// =============================================================================
//  tests/test_indexz_autoheight_1203.cpp -- E-042 = E-038 Phase B, step B1 (SAFETY)
//  ctest IndexZAutoHeight1203.  AI(W906-E042) 20261004 (St01 / ST01-E): NEW FILE.
//
//  The golden Index Z1 Auto Height state machine (TfContact::Do_Z1_AutoGetHeight, golden 0618
//  cContact.cpp:5389-8168, now forms/fContact_AutoHeight.cpp) is called DIRECTLY here -- its only path in the tree is MainProc ->
//  DoTestContactFunction (csystem.cpp:31466, live on the HT9050 PCI1203 Index Z1 only since W-152) and [B9] pins that.  SHIP runs it against the
//  REAL TGaliRouteCore bound to a fake 1203 (the tests/test_gali_route_engine.cpp harness) with a "surface": a
//  move below it stalls there and 6077h reads pressing.  The torque value reaches edTorue0 through Phase A's
//  bridge (COM2->ReadTorque -> rs232.cpp:358 -> W906_Ht9050TorqueRead) fed by a fake read hook.
//    [B1] Panasonic / RS-232 row: golden unchanged (waits at 555 for ever, no port protection acts)
//    [B2] entry refusal (P4 + P8 at case 1): CONFIRMED missing / BASELINE=1 (Q90) / modes / options;
//         modes 1 AND 3 single arm start (Steven 1004 08:4x)
//    [B7c] (both configurations) the pure P8 whitelist / P7 health / P9 floor decisions of IndexZTorqueCore.h
//    [B3] simulated descent to the threshold: kg 15 -> limit 150, 300-count steps, trigger after 10 readings
//    [B4] P5: no torque value -> ST + golden exit after 5 s (not before)
//    [B5] P7: drive ALM / ERROR_STOP / servo off / sample invalid / monitor frozen / no hook / route fault
//         mid-descent -> ST in the SAME tick, no further move, golden exit; ALM when Do_Z1 starts = the same mid-run
//         shape (golden runs Do_Z1 case 1 after DoZ1PickFromShuttle); the run-entry predicate (B4) refuses, sends nothing
//    [B6] P9: torque never reaches kg, Z stalled -> stop at the commanded floor (golden would walk on)
//    [B8] a stop is never gated
//    [B9] census: exactly ONE live caller of the Auto Height / contact state machines outside the new TU (comments, string /
//         char literals and #if 0 blocks stripped first) = MainProc's `if(W906_IndexZLive1203()) fContactForm->DoTestContactFunction();` (W-152); the P-call sites sit
//         before the golden lines' trailing //; MODE (AI(W906-W152-MODE) 20261007 laptop): golden csystem.cpp:17690 only as the HT9050 hook call, no rbModeNormal code; ORDER: the gate line only with W-152 (C)
//    [B10] the golden span is contiguous: every golden `case N:` sits at golden line + OFFSET
//    [B12] SIM: no 1203 is driven; the helpers return false; golden's SOFT_SIMULTE edTorue0=30 path runs
//    [B13] (step B2, both configurations) TfContact::SetContactMode (golden 0618 cContact.cpp:15341-15434, now
//         forms/fContact_ContactSM.cpp, NO caller): golden-faithful mode switch (each radio -> golden's value + memo line,
//         golden's arm order, none checked -> iContactMode untouched, the KYEC / MAXIM One-Touch and KYEC_CHEN + A16 drop-
//         contact side effects, Step-only Daily Correlation), no machine action at all; the P8 chain B4 runs on its output
//         (HT9050: only Auto Height and Contact Test may start, Manual Height and Load Cell refused with their reasons,
//         nothing sent; Panasonic / RS-232: same modes, never refused); source pins: census (0 live callers outside the
//         new TU, replacing the linker interlock GATE (X-10)), every code line = golden, the body stays UI state only
//    [B14] (step B3) pick / place / Do_Z2 / calibrate-above / ATC_SwitchTjSignal (forms/fContact_IndexPickPlace.cpp and the
//         extended forms/fContact_AutoHeight.cpp, NO caller): W-44 (Steven; ST01-M 1005 03:4x: socket press ONLY) -- a
//         socket-press task of Do_Z1 / Do_Z2 with the In or Out shuttle away from home -> ST + golden exit, the socket
//         descent never starts, the pick is NOT guarded, the B4 run-entry helper refuses and sends nothing, RS-232 inert;
//         P7 on every new state machine (DoZ1/Z2PickFromShuttle, DoZPlaceToShuttle, DoArm1/2PlaceToShuttle,
//         Do_Z2_AutoGetHeight, DoCalibrateAboveHeightZ1 / Z2); Do_Z2 case 1 refused on HT9050 (one index arm); P5 on the pick's
//         torque wait; P9 on the pick's descent step; source pins (call sites before trailing //, W-44 only on the
//         socket-press SMs, the #20 D1 fix with its three-part comment, case anchors at golden + offset per span, census)
//    [B15] (step B4) DoTestContactFunction + Do_LoadCellAutoHigh (forms/fContact_ContactSM.cpp; MainProc calls DoTestContactFunction on the HT9050 PCI1203 Index Z1 only, W-152): the contact-mode
//         single entry (V912 DF_SetContactMode on the web's radios, FileRW/DeviceForm_File.cpp EOF); the case-1 run entry --
//         untranslated modes (4 / 5 / 9 / 10 / 11, 12 = TODO E-052) refused on EVERY machine, P4 / P8 / W-44 on HT9050, nothing
//         sent; the HT9050 place-back step (Steven 1005 09:2x Q101): preconditions (Z1 safe, Out shuttle X at OutSHT[0].iRight),
//         In shuttle -> InSHT[0].iRight and confirmed, failure = alarm + ST + no shuttle move; Load Cell refused on HT9050;
//         source pins (each change on its golden line before trailing //, case anchors, census)
//  No machine file is read or written: Gerneral.ini is a temp-dir copy (as test_indexz_torque_1203 [T12]).
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "atester_shims.h"                // COM2
#include "cmydef.h"
#include "common.h"                       // INIFileGeneral
#include "Config.h"                       // IniConfig
#include "CosFunction.h"
#include "cprod.h"                        // Prod / Tech / TestIF / TestIF_File
#include "FormsFacade.h"                  // fMain
#include "MachineType.h"
#include "canary_support.h"               // W906_ShowMyMessage_Count / _LastS1
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "Motor/myGALILmotor.h"           // TMyGALILMotor
#include "Motor/GaliRoute.h"
#include "EtherCAT/Pci1203GaliRouteCore.h"
#include "forms/fContact.h"               // fContactForm
#include "cContact.h"                     // CONTACT_* (CONTACT_TEST since W906-E042)
#include "IndexZTorqueCore.h"
#include "IndexZTorque1203.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include "FileRW/_FormEvent.h"             // AI(W906-E042) B4: link-only stand-ins below (the C-route is compiled in)
#include "JsonBridge/FormBridge.h"
#include <windows.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

extern bool IndexZCanMove[2];
// AI(W906-E042) 20261005 (B4): link-only stand-ins for the C-route sources this test compiles (FileRW/DeviceForm_File.cpp & co.,
// wb_serve-only symbols) -- the same set and reasons as tests/test_b8_ct3a_contactflags.cpp:91-114.
void FileRW_IniConfig_ChangeCBListProperty() {}
const char* W906_Main_sbContactClickOpen() { return nullptr; }
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
namespace ht9045 {
namespace formbridge {
const BridgeDesc* FindBridge(const std::string&) { return nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out) { out->code = "handler-failed"; out->why = "test: no A-shape page"; return false; }
}  // namespace formbridge
namespace formjson {
void FormLock() {}
void FormUnlock() {}
}  // namespace formjson
}  // namespace ht9045
extern int (*W906_Pci1203TorqueLimitHook)(int motIndex, int value01pct, AnsiString* why);   // rs232.cpp:2265

using namespace ht9045;
using namespace ht9045::idxz;

static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; std::printf("  PASS %s\n", msg); } \
                           else   { g_fail++; std::printf("  FAIL %s  [line %d]\n", msg, __LINE__); } } while (0)

// ---- reaching two PRIVATE TfContact members without touching forms/fContact.h: the standard explicit-instantiation
//      access rule ([temp.explicit]/12 -- access checking is not done on the names in an explicit instantiation).
template <typename Tag, typename Tag::type M> struct Rob { friend typename Tag::type get(Tag) { return M; } };
struct DoZ1Tag   { typedef bool (TfContact::*type)(); friend type get(DoZ1Tag); };
struct SpeedZTag { typedef int TfContact::*type;      friend type get(SpeedZTag); };
template struct Rob<DoZ1Tag, &TfContact::Do_Z1_AutoGetHeight>;
template struct Rob<SpeedZTag, &TfContact::iSpeedZ>;
static bool DoZ1() { return (fContactForm->*get(DoZ1Tag()))(); }
//  AI(W906-E042) 20261004 (B2, St01): SetContactMode is private too (forms/fContact.h:1464, between :1332 private: and :1479 public:).
struct SetModeTag { typedef void (TfContact::*type)(); friend type get(SetModeTag); };
template struct Rob<SetModeTag, &TfContact::SetContactMode>;
static void SetMode() { (fContactForm->*get(SetModeTag()))(); }
//  AI(W906-E042) 20261005 (B3, St01): the B3 state machines (private ones reached the same way).
struct PickZ1Tag  { typedef bool (TfContact::*type)(); friend type get(PickZ1Tag); };
struct PickZ2Tag  { typedef bool (TfContact::*type)(); friend type get(PickZ2Tag); };
struct PlaceTag   { typedef bool (TfContact::*type)(); friend type get(PlaceTag); };
struct DoZ2Tag    { typedef bool (TfContact::*type)(); friend type get(DoZ2Tag); };
struct CaliTag    { typedef bool (TfContact::*type)(bool); friend type get(CaliTag); };
template struct Rob<PickZ1Tag, &TfContact::DoZ1PickFromShuttle>;
template struct Rob<PickZ2Tag, &TfContact::DoZ2PickFromShuttle>;
template struct Rob<PlaceTag,  &TfContact::DoZPlaceToShuttle>;
template struct Rob<DoZ2Tag,   &TfContact::Do_Z2_AutoGetHeight>;
template struct Rob<CaliTag,   &TfContact::DoCalibrateAboveHeightZ1>;
static bool PickZ1() { return (fContactForm->*get(PickZ1Tag()))(); }
static bool PickZ2() { return (fContactForm->*get(PickZ2Tag()))(); }
static bool PlaceZ() { return (fContactForm->*get(PlaceTag()))(); }
static bool DoZ2()   { return (fContactForm->*get(DoZ2Tag()))(); }
static bool Cali()   { return (fContactForm->*get(CaliTag()))(false); }
//  W-44 test seam: the In / Out shuttle 1 positions (default home = 0).
static long g_inShPos = 0, g_outShPos = 0;
//  B4: the In shuttle move seam of the place-back step: records the target; arrives (or stops 500 counts short).
static std::vector<long> g_shMoves;
static bool g_shArrive = true;
static bool FakeShMove(int which, long target) { if (which == 0) { g_shMoves.push_back(target); g_inShPos = g_shArrive ? target : target + 500; } return true; }
void FileRW_Contact_SetContactModeSingleEntry(int rbForTest);   // FileRW/DeviceForm_File.cpp EOF (declared in forms/fContact_AutoHeight.h); compiled into this test
static bool g_shPresent = true;
static bool FakeShPos(int which, long* pos) { if (pos) *pos = (which == 0) ? g_inShPos : g_outShPos; return g_shPresent; }

// ---- the fake clock the P5 / P7 helpers read (W906_IndexZNowMsHook) ------------------------------
static unsigned long g_now = 1000000ul;
static unsigned long FakeNow() { return g_now; }

// ---- the fake 1203 + the machine model ------------------------------------------------------------
static long          g_target   = 0;          // the last card target (card = flow position for M14 Direction 0)
static long          g_surface  = -10000;     // -100.00 mm: a move below it stalls here
static unsigned      g_state    = 1;          // STA_AX_READY
static unsigned long g_bits     = 0x00004000ul;   // SVON
static long          g_rawPress = -150;       // 6077h while pressing (0.1 %: -150 = 15.0 %, pressing = negative)
static long          g_rawAir   = 80;         // 6077h holding against gravity (+8.0 %)
static bool          g_torqueOk = true;       // the 6077h SDO focus / value is there
static bool          g_healthMonitor = true;
static bool          g_sampleValid   = true;
static bool          g_routeFault    = false;
static int           g_stops = 0;
static std::vector<long> g_moves;             // every kCmdAxMoveAbs target, in order
static std::vector<int>  g_limits;            // every torque-limit write (0.1 %)

#ifndef SOFT_SIMULTE
struct FakeIo : IGaliRouteIo {
    unsigned long   poll;
    GaliRouteSample s;
    unsigned long   now;
    std::map<int, unsigned long> ledger;
    FakeIo() : poll(10), now(1000) { s.state = 1; s.motionIO = 0x00004000ul; }
    bool CardOpen() override { return true; }
    unsigned long PollCount() override { return poll; }
    int Slot(std::string&) override { return 7; }
    bool Sample(int, GaliRouteSample& o) override { o = s; return true; }
    int DriveKind(int) override { return 1; }
    Pci1203CmdResult Exec(const Pci1203Cmd& c) override
    {
        if (c.kind == kCmdAxMoveAbs) { g_target = (long)std::lround(c.value); g_moves.push_back(g_target); }
        if (c.kind == kCmdAxStop || c.kind == kCmdAxEmgStop) ++g_stops;
        Pci1203CmdResult r;
        r.accepted = true; r.issued = true; r.ret = 0;
        return r;
    }
    void Caps(GaliRouteCaps& c) override
    {
        HTMotor* M = MOT[MTestZ1].Motor;
        c.jogHigh = M->PJogHighSpeed; c.initSpeed = M->InitSpeed; c.accDb = M->GetAccDataBase(); c.decDb = M->GetDecDataBase();
    }
    unsigned long NowMs() override { return now; }
    void NoteIssued(int sl, unsigned long p) override { ledger[sl] = p; }
    bool LastIssued(int sl, unsigned long& p) override
    {
        std::map<int, unsigned long>::const_iterator it = ledger.find(sl);
        if (it == ledger.end()) return false;
        p = it->second;
        return true;
    }
    bool OnOwnerThread() override { return true; }
    void SleepMs(int) override {}
    int OrgHome(int, unsigned long) override { return -2; }
    void Log(const std::string&) override {}
    void LogForce(const std::string& l) override { std::printf("    [route!] %s\n", l.c_str()); }
};
static FakeIo         g_io;
static TGaliRouteCore g_core;
static bool T_Command(int, const char* d, long* r) { return g_core.Command(d, r); }
static int  T_HomeStart(int, bool dir, unsigned hi, unsigned lo, double a, double d) { return g_core.HomeStart(dir, hi, lo, a, d); }
static int  T_HomePoll(int) { return g_core.HomePoll(); }
static TGaliRoute g_route = { -1, T_Command, T_HomeStart, T_HomePoll };
static void Fresh()
{
    ++g_io.poll;
    g_io.s.state    = g_state;
    g_io.s.motionIO = g_bits;
    g_io.s.cmdPos   = (double)g_target;
    g_io.s.actPos   = (double)(g_target < g_surface ? g_surface : g_target);
}
static unsigned long Poll() { return g_io.poll; }
#else
static void Fresh() {}
static unsigned long Poll() { return 0; }
#endif
static bool DoorClosed() { return false; }

static void FakeRead(int, Sample* out)
{
    Sample s;
    s.haveMonitor = true; s.poll = Poll(); s.torqueValid = g_torqueOk; s.src = 2;
    s.raw = (g_target < g_surface) ? g_rawPress : g_rawAir;
    s.pctValid = true; s.num = 1; s.den = 10;
    s.servoOn = (g_bits & 0x4000ul) != 0; s.alarm = (g_bits & 0x2ul) != 0; s.focusOnAxis = g_torqueOk; s.slot = 7;
    if (!g_torqueOk) s.why = "test: focus on another axis";
    if (out) *out = s;
}
static void FakeHealth(int, Health* out)
{
    Health h;
    h.haveMonitor = g_healthMonitor; h.poll = Poll(); h.sampleValid = g_sampleValid;
    h.alarm = (g_bits & 0x2ul) != 0; h.errorStop = (g_state & 0xFFu) == 3u; h.servoOn = (g_bits & 0x4000ul) != 0;
    h.routeFault = g_routeFault; h.routeWhy = g_routeFault ? "test: move refused (latched)" : "";
    if (!g_healthMonitor) h.why = "test: monitor gone";
    if (out) *out = h;
}
static int FakeLimit(int, int v, AnsiString*) { g_limits.push_back(v); return 1; }

// ---- Gerneral.ini in %TEMP% only --------------------------------------------------------------------
static std::string g_dir, g_ini;
static TIniFile* g_oldIni = 0;
static TIniFile* g_tmpIni = 0;
static void UseIni(const char* extra1, const char* extra2 = 0)
{
    std::string s = "[Version]\r\nModel=9050GPIB\r\n[IndexDriver]\r\nINDEX_DRIVER_TYPE=0\r\n";
    if (extra1) s += std::string(extra1) + "\r\n";
    if (extra2) s += std::string(extra2) + "\r\n";
    s += "[System]\r\nX=1\r\n";
    std::ofstream f(g_ini.c_str(), std::ios::binary | std::ios::trunc);
    f.write(s.data(), (std::streamsize)s.size());
    f.close();
    INIFileGeneral = g_oldIni;
    delete g_tmpIni;
    g_tmpIni = new TIniFile(AnsiString(g_ini.c_str()));
    INIFileGeneral = g_tmpIni;
    W906_IndexZTorqueFlagsReset();
}

// ---- one MainProc-shaped tick: (the monitor polls) -> COM2->ReadTorque (csystem.cpp:30399) -> the state machine ----
static bool Tick(unsigned long dtMs = 100, bool fresh = true, bool readTorque = true)
{
    g_now += dtMs;
    if (fresh) Fresh();
    if (readTorque) COM2->ReadTorque();
    return DoZ1();
}
static int Moves() { return (int)g_moves.size(); }
static long MinMove() { long m = 0; for (std::size_t i = 0; i < g_moves.size(); ++i) if (g_moves[i] < m) m = g_moves[i]; return m; }
static bool Has(const AnsiString& s, const char* sub) { return std::string(s.c_str()).find(sub) != std::string::npos; }

//  A fresh run from case 530 (standby move + torque limit) on a healthy, confirmed HT9050.
static void StartAt(int task)
{
    W906_IndexZTorqueBridgeReset();
    W906_IndexZHealthReset();
    fMain->chkReadTorque1->Checked = false; fMain->chkReadTorque2->Checked = false;
    fMain->edTorue0->Text = "";
    g_state = 1; g_bits = 0x00004000ul; g_torqueOk = true; g_healthMonitor = true; g_sampleValid = true; g_routeFault = false;
    g_rawPress = -150;
    g_target = 0; Fresh();
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0; MOT[MTestZ1].iCheckStatusCT = 0;
    g_moves.clear(); g_limits.clear(); g_stops = 0;
    fContactForm->Z_Height_Task = task;
    fContactForm->CarlibrationTask = 400;
    fAllMotorHome = true;
    SystemStart = true;
}

// ---- source helpers ([B9] / [B10]) --------------------------------------------------------------------
static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string l;
    while (std::getline(f, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); out.push_back(l); }
    return true;
}
//  Code of one line with /* */ (same line) and the trailing // removed.  Used by the call-site PINS only, whose needles
//  deliberately include string arguments (e.g. W906_IndexZDriveFaultStop("Do_Z1_AutoGetHeight")), so string literals
//  are KEPT here.  The census uses LiveCode below, which strips them.
static std::string CodeOf(const std::string& l)
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
static std::string Norm(const std::string& l)
{
    const std::string c = CodeOf(l);
    std::string o;
    bool sp = false;
    for (std::size_t i = 0; i < c.size(); ++i) {
        const bool w = c[i] == ' ' || c[i] == '\t';
        if (w) { sp = !o.empty(); continue; }
        if (sp) { o += ' '; sp = false; }
        o += c[i];
    }
    return o;
}
static bool IdentChar(char c) { return std::isalnum((unsigned char)c) != 0 || c == '_'; }
//  Live code of a file, one entry per source line, for the CENSUS: a name counts only where the compiler would see it
//  as code.  Blanked: // and /* */ comments (across lines), the CONTENTS of string / character literals ("..." -> "",
//  escapes honoured; an unterminated one ends at the line end), raw strings R"d(...)d" (across lines), and every line
//  inside `#if 0` (nesting tracked; the `#else` / `#elif` of an `#if 0` is live; any other #if / #ifdef counts as live,
//  which can only ADD matches, never hide one).
//  AI(W906-E042) 20261004 (restart 2, St01): literals were not stripped before, so csystem.cpp:31327
//  `MyDBIProcess("Exception", "DoTestContactFunction()");` -- a log string inside `#ifdef DEBUG_TRY_CATCH`, not a call
//  (ST01-E, 1004) -- counted as a live caller: the [B9] false positive of 1004 16:27.  The real call
//  (csystem.cpp:31466) is live since W-152 (Jimmy #143 = A): guarded by W906_IndexZLive1203(), on fContactForm -- [B9] pins it as the only one.
static std::vector<std::string> LiveCode(const std::vector<std::string>& v)
{
    std::vector<std::string> out(v.size());
    std::vector<int> st;          // 1 = dead frame (#if 0), 0 = other
    enum { kCode, kBlock, kRaw } mode = kCode;
    std::string rawEnd;           // `)delim"` of the open raw string
    for (std::size_t i = 0; i < v.size(); ++i) {
        const std::string& l = v[i];
        const bool startsInCode = (mode == kCode);
        std::string code;
        std::size_t k = 0;
        while (k < l.size()) {
            if (mode == kBlock) {
                const std::size_t e = l.find("*/", k);
                if (e == std::string::npos) { k = l.size(); break; }
                k = e + 2; mode = kCode; code += ' '; continue;
            }
            if (mode == kRaw) {
                const std::size_t e = l.find(rawEnd, k);
                if (e == std::string::npos) { k = l.size(); break; }
                k = e + rawEnd.size(); mode = kCode; code += '"'; continue;
            }
            const char c = l[k];
            const char n = (k + 1 < l.size()) ? l[k + 1] : '\0';
            if (c == '/' && n == '/') break;                                      // line comment
            if (c == '/' && n == '*') { mode = kBlock; k += 2; continue; }        // block comment
            if (c == 'R' && n == '"') {                                           // raw string R"d( ... )d"
                std::size_t s = k;
                while (s > 0 && IdentChar(l[s - 1])) --s;
                const std::string pre = l.substr(s, k - s);
                const std::size_t p = l.find('(', k + 2);
                if ((pre.empty() || pre == "u8" || pre == "u" || pre == "U" || pre == "L") && p != std::string::npos) {
                    rawEnd = ")" + l.substr(k + 2, p - (k + 2)) + "\"";
                    code += "R\""; mode = kRaw; k = p + 1; continue;
                }
            }
            if (c == '\'' && k > 0 && IdentChar(l[k - 1])) {                       // 1'000: a digit separator, not a literal
                std::size_t s = k;
                while (s > 0 && IdentChar(l[s - 1])) --s;
                if (std::isdigit((unsigned char)l[s])) { ++k; continue; }
            }
            if (c == '"' || c == '\'') {                                          // literal: keep the quotes, drop the contents
                code += c; ++k;
                while (k < l.size() && l[k] != c) k += (l[k] == '\\') ? 2 : 1;
                if (k < l.size()) { code += c; ++k; }
                continue;
            }
            code += c; ++k;
        }
        bool dead = false; for (std::size_t q = 0; q < st.size(); ++q) if (st[q]) dead = true;
        std::string t = code; t.erase(0, t.find_first_not_of(" \t"));
        if (startsInCode && !t.empty() && t[0] == '#') {
            std::string d = t.substr(1); d.erase(0, d.find_first_not_of(" \t"));
            while (!d.empty() && (d[d.size() - 1] == ' ' || d[d.size() - 1] == '\t')) d.erase(d.size() - 1);
            if (d.compare(0, 2, "if") == 0) st.push_back(d.compare(0, 4, "if 0") == 0 && (d.size() == 4 || d[4] == ' ' || d[4] == '\t') ? 1 : 0);
            else if (d.compare(0, 4, "else") == 0 || d.compare(0, 4, "elif") == 0) { if (!st.empty() && st.back() == 1) st.back() = 0; }
            else if (d.compare(0, 5, "endif") == 0) { if (!st.empty()) st.pop_back(); }
            continue;
        }
        if (!dead) out[i] = code;
    }
    return out;
}
static void Walk(const std::string& dir, std::vector<std::string>& files)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (n == "tests" || n == "docs" || n == "build" || n.compare(0, 6, "build_") == 0 || n == "third_party" || n == ".git") continue;
            Walk(p, files);
        } else {
            const std::size_t dot = n.rfind('.');
            const std::string ext = dot == std::string::npos ? "" : n.substr(dot);
            if (ext == ".cpp" || ext == ".h" || ext == ".inc") files.push_back(p);
        }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// golden 0618 cContact.cpp: (line, N) of every `case N:` label in Do_Z1_AutoGetHeight (:5389-8168), extracted from
// the golden file (cp950) when the TU was generated; each must sit at golden line + OFFSET in the port.
static const int kAnchors[][2] = {
    {5425,1}, {5534,100}, {5538,110}, {5558,150}, {5572,151}, {5596,200}, {5600,520}, {5673,525}, {5679,530},
    {5700,535}, {5704,536}, {5757,550}, {5765,555}, {5888,560}, {6086,564}, {6094,565}, {6157,600}, {6202,700},
    {6245,701}, {6256,702}, {6263,710}, {6297,7110}, {6313,7120}, {6320,7125}, {6329,7150}, {6384,7160}, {6405,7170},
    {6441,720}, {6462,730}, {6495,800}, {6571,8005}, {6602,8006}, {6622,8007}, {6645,810}, {6694,900}, {6709,9000},
    {6717,9010}, {6748,901}, {6753,90100}, {6761,90110}, {6781,90120}, {6789,902}, {6810,905}, {6813,930},
    {6839,950}, {6842,960}, {6863,1000}, {6894,1100}, {6938,2000}, {6948,2050}, {6968,2060}, {6971,2070},
    {6992,2100}, {7003,2200}, {7090,2800}, {7112,2900}, {7225,2904}, {7233,2905}, {7289,2910}, {7326,2920},
    {7394,2921}, {7400,2922}, {7406,2930}, {7462,2940}, {7475,2950}, {7479,2952}, {7487,2953}, {7521,2954},
    {7532,2955}, {7551,2956}, {7556,3000}, {7628,3001}, {7634,3002}, {7661,3003}, {7667,3010}, {7729,3105},
    {7736,3100}, {7754,3110}, {7830,3150}, {7836,3160}, {7842,3170}, {7849,3180}, {7855,3182}, {7862,3184},
    {7870,3186}, {7890,3188}, {7898,3190}, {7923,3200}, {7934,3210}, {7940,3300}, {7948,4000}, {7980,4005},
    {7994,4010}, {8000,4020}, {8008,4030}, {8015,4035}, {8029,4040}, {8035,4050}, {8043,4060}, {8050,4065},
    {8064,5000}, {8084,5100}, {8090,5200}, {8104,5900}, {8129,6000}, {8141,6010}, {8148,6100}, {8160,6200}
};

// AI(W906-E042) 20261004 (B2, St01): golden 0618 cContact.cpp:15341-15434 (TfContact::SetContactMode), every CODE line
// (comments dropped, whitespace collapsed), generated from the cp950 golden by D:\AI_TempFile\st01e-e042-run\gen_contactsm.py.
// A golden line not listed here has no code; its port line must have none either.
struct ScmGoldenLine { int line; const char* code; };
static const ScmGoldenLine kScmGolden[] = {
    {15341, "void TfContact::SetContactMode()"},
    {15342, "{"},
    {15343, "if(rbModeNormal->Checked)"},
    {15344, "{"},
    {15345, "iContactMode=CONTACT_NORMAL;"},
    {15346, "if(CUSTOMER_CODE==CC_KYEC_CHEN &&"},
    {15347, "IniConfig.bA16ContactTestDropContact)"},
    {15348, "{"},
    {15349, "ChangeContactMode(false);"},
    {15350, "}"},
    {15351, "Memo1->Lines->Add(\"CONTACT_NORMAL\");"},
    {15352, "}"},
    {15353, "else if(rbDeviceMapping->Checked)"},
    {15354, "{"},
    {15355, "iContactMode=CONTACT_DEVICE_MAP_CHECK;"},
    {15356, "Memo1->Lines->Add(\"CONTACT_DEVICE_MAP_CHECK\");"},
    {15357, "}"},
    {15358, "else if(rbAutoHeight->Checked)"},
    {15359, "{"},
    {15360, "iContactMode=CONTACT_AUTO_GET_HEIGHT;"},
    {15361, "Memo1->Lines->Add(\"CONTACT_AUTO_GET_HEIGHT\");"},
    {15362, "}"},
    {15363, "else if(rbManualHeight->Checked)"},
    {15364, "{"},
    {15365, "iContactMode=CONTACT_MANUAL_GET_HEIGHT;"},
    {15366, "Memo1->Lines->Add(\"CONTACT_MANUAL_GET_HEIGHT\");"},
    {15367, "}"},
    {15368, "else if(rbContactTest->Checked)"},
    {15369, "{"},
    {15370, "iContactMode=CONTACT_TEST;"},
    {15371, "Memo1->Lines->Add(\"CONTACT_TEST\");"},
    {15372, "if(CUSTOMER_CODE==CC_KYEC_CHEN &&"},
    {15373, "IniConfig.bA16ContactTestDropContact)"},
    {15374, "{"},
    {15375, "ChangeContactMode(true);"},
    {15376, "}"},
    {15377, "}"},
    {15378, "else if(rbAutoContactTest->Checked)"},
    {15379, "{"},
    {15380, "iContactMode=AUTO_CONTACT_TEST;"},
    {15381, "Memo1->Lines->Add(\"AUTO_CONTACT_TEST\");"},
    {15382, "}"},
    {15383, "else if(rbStepContactTest->Checked)"},
    {15384, "{"},
    {15385, "iContactMode=STEP_CONTACT_TEST;"},
    {15386, "Memo1->Lines->Add(\"STEP_CONTACT_TEST\");"},
    {15387, "}"},
    {15388, "else if(rbLoadCellAutoHigh->Checked)"},
    {15389, "{"},
    {15390, "iContactMode=CONTACT_LoadCell_AUTO_GET_HEIGHT;"},
    {15391, "Memo1->Lines->Add(\"CONTACT_LoadCell_AUTO_GET_HEIGHT\");"},
    {15392, "}"},
    {15393, "else if(rbDeviceLoopTest->Checked)"},
    {15394, "{"},
    {15395, "iContactMode=CONTACT_DEVICE_LOOP_TEST;"},
    {15396, "Memo1->Lines->Add(\"CONTACT_DEVICE_LOOP_TEST\");"},
    {15397, "}"},
    {15398, "else if(rbKTempIndexMove->Checked)"},
    {15399, "{"},
    {15400, "iContactMode=K_TEMP_INDEX_MOVE;"},
    {15401, "Memo1->Lines->Add(\"K_Temp_Index_Move\");"},
    {15402, "chk_K_Temperature->Checked=true;"},
    {15403, "}"},
    {15405, "if(CUSTOMER_CODE==CC_KYEC_LEE ||"},
    {15406, "CUSTOMER_CODE==CC_KYEC_XILINX ||"},
    {15407, "CUSTOMER_CODE==CC_MAXIM_THAILAND)"},
    {15408, "{"},
    {15409, "if(iContactMode==CONTACT_AUTO_GET_HEIGHT)"},
    {15410, "{"},
    {15411, "cbOneTouchAutoContactHight->Visible=true;"},
    {15412, "}"},
    {15413, "else"},
    {15414, "{"},
    {15415, "cbOneTouchAutoContactHight->Visible=false;"},
    {15416, "cbOneTouchAutoContactHight->Checked=false;"},
    {15417, "}"},
    {15418, "}"},
    {15419, "else"},
    {15420, "{"},
    {15421, "cbOneTouchAutoContactHight->Visible=false;"},
    {15422, "cbOneTouchAutoContactHight->Checked=false;"},
    {15423, "}"},
    {15425, "if(iContactMode==STEP_CONTACT_TEST)"},
    {15426, "{"},
    {15427, "chkDailyCorrelation->Enabled=true;"},
    {15428, "}"},
    {15429, "else"},
    {15430, "{"},
    {15431, "chkDailyCorrelation->Enabled=false;"},
    {15432, "chkDailyCorrelation->Checked=false;"},
    {15433, "}"},
    {15434, "}"},
};
static std::string Norm(const std::string& l);   // CodeOf + whitespace collapsed (defined below)

// AI(W906-E042) 20261005 (B3, St01): every golden `case N:` of the B3 functions -- {golden line, N, span}; span 0 = the
// AutoHeight main span, 1 = its CalibrateAboveZ1 span, 3 / 4 / 5 = IndexPickPlace spans A / B / C.  Generated from the cp950
// golden by D:\AI_TempFile\st01e-e042-run\b3_anchors.py.
// AI(W906-E042) 20261005 (B4, St01): every golden `case N:` of DoTestContactFunction (span 6) and Do_LoadCellAutoHigh (7),
// generated by D:\AI_TempFile\st01e-e042-run\b4_anchors.py.
static const int kB4Anchors[][3] = {
    {11788,1,6}, {11941,50,6}, {11945,55,6}, {11951,70,6}, {11962,75,6}, {11982,77,6},
    {11986,78,6}, {12003,80,6}, {12007,85,6}, {12031,86,6}, {12053,90,6}, {12066,100,6},
    {12127,190,6}, {12140,200,6}, {12338,205,6}, {12354,206,6}, {12369,207,6}, {12379,208,6},
    {12394,209,6}, {12410,210,6}, {12417,211,6}, {12449,212,6}, {12466,218,6}, {12478,2180,6},
    {12485,2190,6}, {12501,2195,6}, {12574,2196,6}, {12587,2197,6}, {12619,2198,6}, {12642,219,6},
    {12667,220,6}, {12695,230,6}, {12713,290,6}, {12749,295,6}, {12755,296,6}, {12762,297,6},
    {12770,300,6}, {12777,320,6}, {12804,330,6}, {12811,335,6}, {12850,340,6}, {12996,400,6},
    {13169,500,6}, {13177,799,6}, {13183,800,6}, {13235,802,6}, {13242,805,6}, {13278,810,6},
    {13282,820,6}, {13288,1100,6}, {13348,900,6}, {13363,950,6}, {13370,1000,6}, {13464,1200,6},
    {13490,1300,6}, {13537,1500,6}, {13543,1700,6}, {13617,1800,6}, {13727,1900,6}, {13739,1910,6},
    {13747,2000,6}, {13759,2100,6}, {13783,2010,6}, {13822,2050,6}, {13836,2060,6}, {13853,2150,6},
    {13864,2151,6}, {13874,2155,6}, {13904,2160,6}, {13911,2170,6}, {13918,2200,6}, {13930,2300,6},
    {13937,2310,6}, {13947,2400,6}, {13954,2410,6}, {17212,1,7}, {17223,100,7}, {17235,110,7},
    {17250,120,7}, {17272,121,7}, {17276,122,7}, {17310,130,7}, {17327,150,7}, {17514,160,7},
    {17604,161,7}, {17621,200,7}, {17653,210,7}, {17689,300,7}, {17712,310,7}, {17750,320,7},
    {17815,330,7}, {17838,340,7}, {17869,350,7}, {17923,360,7}, {17939,490,7}, {17968,2100,7},
    {17988,2200,7},
};
static const int kB3Anchors[][3] = {
    {8204,1,0}, {8309,100,0}, {8313,110,0}, {8359,200,0}, {8363,520,0}, {8437,525,0},
    {8443,530,0}, {8461,535,0}, {8465,536,0}, {8499,550,0}, {8505,555,0}, {8627,560,0},
    {8801,564,0}, {8807,565,0}, {8870,600,0}, {8916,700,0}, {8959,701,0}, {8965,702,0},
    {8971,710,0}, {9027,7110,0}, {9043,7120,0}, {9050,7125,0}, {9060,7150,0}, {9111,7160,0},
    {9127,7170,0}, {9158,720,0}, {9178,730,0}, {9209,800,0}, {9270,810,0}, {9315,900,0},
    {9330,9000,0}, {9338,9010,0}, {9369,901,0}, {9374,90100,0}, {9382,90110,0}, {9400,90120,0},
    {9408,902,0}, {9429,905,0}, {9441,950,0}, {9444,960,0}, {9464,1000,0}, {9495,1100,0},
    {9540,2000,0}, {9550,2050,0}, {9570,2060,0}, {9573,2070,0}, {9591,2100,0}, {9636,2200,0},
    {9693,2800,0}, {9715,2900,0}, {9823,2904,0}, {9829,2905,0}, {9885,2910,0}, {9903,2920,0},
    {9939,2921,0}, {9945,2923,0}, {9951,2930,0}, {10006,2940,0}, {10019,2950,0}, {10025,2952,0},
    {10033,2953,0}, {10065,2954,0}, {10073,2955,0}, {10092,2956,0}, {10097,3000,0}, {10157,3001,0},
    {10163,3002,0}, {10178,3010,0}, {10236,3105,0}, {10243,3100,0}, {10261,3110,0}, {10323,3150,0},
    {10329,3160,0}, {10335,3170,0}, {10342,3180,0}, {10348,3182,0}, {10354,3184,0}, {10364,3186,0},
    {10384,3188,0}, {10392,3190,0}, {10417,3200,0}, {10429,3210,0}, {10435,3300,0}, {10442,5900,0},
    {10471,6000,0}, {10483,6010,0}, {10490,6100,0}, {10502,6200,0}, {22349,1,1}, {22366,100,1},
    {22372,200,1}, {22392,300,1}, {22406,400,1}, {22417,500,1}, {22485,1,1}, {22502,100,1},
    {22508,200,1}, {22528,300,1}, {22542,400,1}, {22553,500,1}, {2517,1,3}, {2536,100,3},
    {2572,110,3}, {2601,120,3}, {2621,130,3}, {2662,140,3}, {2693,141,3}, {2698,142,3},
    {2705,143,3}, {2740,150,3}, {2761,200,3}, {2814,250,3}, {2821,300,3}, {2846,400,3},
    {2933,500,3}, {2952,550,3}, {3002,555,3}, {3008,560,3}, {3045,570,3}, {3139,600,3},
    {3229,610,3}, {3248,650,3}, {3254,3000,3}, {3335,3050,3}, {3454,3100,3}, {3477,660,3},
    {3556,665,3}, {3573,670,3}, {3593,675,3}, {3608,680,3}, {3816,700,3}, {3853,800,3},
    {3910,1,3}, {3928,100,3}, {3961,110,3}, {3990,120,3}, {4008,130,3}, {4049,140,3},
    {4064,142,3}, {4070,143,3}, {4105,150,3}, {4122,200,3}, {4175,250,3}, {4182,300,3},
    {4206,400,3}, {4293,500,3}, {4312,550,3}, {4372,555,3}, {4378,560,3}, {4414,570,3},
    {4518,600,3}, {4608,610,3}, {4627,650,3}, {4633,3000,3}, {4714,3050,3}, {4833,3100,3},
    {4855,660,3}, {4932,665,3}, {4949,670,3}, {4970,675,3}, {4985,680,3}, {5188,700,3},
    {5227,800,3}, {11407,1,4}, {11497,100,4}, {11501,110,4}, {11512,1000,4}, {11516,1100,4},
    {11522,1200,4}, {11528,120,4}, {11535,150,4}, {11551,160,4}, {11572,200,4}, {11641,300,4},
    {11645,350,4}, {11657,400,4}, {11711,600,4}, {11721,700,4}, {11733,800,4}, {19303,1,5},
    {19306,100,5}, {19313,200,5}, {19330,300,5}, {19359,400,5}, {19363,500,5}, {19373,600,5},
    {19377,9999,5}, {19410,1,5}, {19413,100,5}, {19431,200,5}, {19448,300,5}, {19478,400,5},
    {19482,500,5}, {19492,600,5}, {19496,9999,5},
};

int main()
{
    if (!W906TestRequireCtestRedirects("IndexZAutoHeight1203")) return 2;
    std::printf("[indexz_autoheight_1203] %s configuration\n",
#ifdef SOFT_SIMULTE
        "SIM (SOFT_SIMULTE)"
#else
        "SHIP (no SOFT_SIMULTE)"
#endif
    );

    // ---------------- shared setup (golden boot invariants, HT9050 tables) ----------------
    INDEX_MOTION_CARD = 0;
    USE_INDEX_ARM_AXES = IndexArm_4_Axis;
    IniConfig.GaliPosRange = 50;
    CosFunction.bIndexProtect = false;
    InitialOK = false;
    SystemStart = false;
    const char* names[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {
        const int i = MTestY1 + k;
        MOT[i].Motor = new TMyGALILMotor(k);
        MOT[i].SetAlias(i, names[k]);
        MOT[i].Motor->Enable = true;
        MOT[i].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
        MOT[i].Motor->PServoAlarmOn = true;
        MOT[i].Motor->GearRatio = 1.0;
        for (int l = 0; l < 10; ++l) MOT[i].Led[l] = false;
    }
    W906_TestEnsureSimMotors();
    HTMotor* Z = MOT[MTestZ1].Motor;
    Z->PJogHighSpeed = 900000; Z->InitSpeed = 100; Z->SetAccDataBase(9000000.0); Z->SetDecDataBase(9000000.0);   // machines/HT9050/Mot_Table.csv:16
    Z->Direction = false;                                                    // M14 Direction 0
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    MOT[MTestZ1].CardType = "PCI1203";
    INDEX_DRIVER_TYPE = Panasonic_DRIVER;
    iPanasonicDriverType = Panasonic_DRIVER_A5;
    TorqueUseHPComCard = false;
    iContactMode = CONTACT_AUTO_GET_HEIGHT;
    IniConfig.bD14_AutoHeightUseSetTorque = true;                           // machines/HT9050 B_config.ini:162-163
    IniConfig.iD14_AutoHeightUseSetTorque = 15;
    IniConfig.iD17_UseHardwareHeightToContact = 0;
    IniConfig.bD10ManualHeightComptibleWithNS = false;
    IniConfig.bIndexArm2SupplyLight = false;
    TestIF.iShuttleMode = 1; TestIF.iShuttle_Sel = 0;
    Tech.iTestZDown = -4000;                                                 // standby = iTestZDown-5000 = -90.00 mm (golden :5686)
    fIndexDownPos = -148.0;                                                  // HT9046_LS decode (golden :92)
    fContactForm->*get(SpeedZTag()) = 60;                                    // golden CheckRange(iSpeedZ, 60, 10) (:12077)
    W906_Pci1203TorqueReadHook   = &FakeRead;
    W906_Pci1203IndexZHealthHook = &FakeHealth;
    W906_Pci1203TorqueLimitHook  = &FakeLimit;
    W906_IndexZNowMsHook         = &FakeNow;
    W906_IndexZShuttlePosHook    = &FakeShPos;                               // AI(W906-E042) B3: W-44 (default: both shuttles home)
    W906_IndexZShuttleMoveHook   = &FakeShMove;                              // AI(W906-E042) B4: place-back In shuttle move
    {
        char b[MAX_PATH + 1] = { 0 };
        ::GetTempPathA(MAX_PATH, b);
        g_dir = std::string(b) + "w906_e042_" + std::to_string((unsigned long)::GetCurrentProcessId());
        ::CreateDirectoryA(g_dir.c_str(), 0);
        g_ini = g_dir + "\\Gerneral.ini";
    }
    g_oldIni = INIFileGeneral;
    UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
    COM2->Comm1->SetSimMode(true);
    COM2->Comm1->StartComm();

    // ================= [B7c] the pure P8 / P7 / P9 decisions (IndexZTorqueCore.h), both configurations =================
    std::printf("-- [B7c] pure decisions: P8 whitelist = modes 1 and 3 single arm, P7 drive health, P9 floor\n");
    {
        char m[300];
        RunGateIn g;
        g.h.z1Is1203 = true; g.h.confirmed = true; g.h.driverPanasonic = true; g.h.hookInstalled = true;
        g.shuttleModeSingle = true; g.shuttleSel = 0;
        std::string why, bad;
        for (int mode = 0; mode <= 12; ++mode) {
            g.h.contactMode = mode;
            const bool ok = RunAllowed(g, why);
            if (ok != (mode == 1 || mode == 3)) bad += std::to_string(mode) + " ";
        }
        std::snprintf(m, sizeof(m), "[B7c] P8: on a PCI-1203 Index Z exactly modes 1 (Auto Height) and 3 (Contact Test) may start (Steven 1004 08:4x) %s", bad.c_str());
        CHECK(bad.empty(), m);
        g.h.contactMode = 2;  RunAllowed(g, why);
        CHECK(why.find("Manual Height stays refused") != std::string::npos, "[B7c] P8: mode 2 Manual Height refused with its reason (golden jogs with Galil JG / servo off)");
        g.h.contactMode = 8;  RunAllowed(g, why);
        CHECK(why.find("Load Cell") != std::string::npos, "[B7c] P8: mode 8 Load Cell refused with its reason (it always runs Z2 after Z1)");
        int refusedOpt[2] = { 0, 0 };
        const int modes[2] = { 1, 3 };
        for (int mi = 0; mi < 2; ++mi) {
            for (int k = 0; k < 8; ++k) {
                RunGateIn o = g;
                o.h.contactMode = modes[mi];
                switch (k) {
                    case 0: o.shuttleModeSingle = false; break;
                    case 1: o.shuttleSel = 1; break;
                    case 2: o.arm2Options = true; break;
                    case 3: o.twoArm32Site = true; break;
                    case 4: o.rtcLearning = true; break;
                    case 5: o.calibrateAbove = true; break;
                    case 6: o.latchTeach = true; break;
                    case 7: o.teachInOutArmZ = true; break;
                }
                if (!RunAllowed(o, why) && !why.empty()) ++refusedOpt[mi];
            }
        }
        CHECK(refusedOpt[0] == 8 && refusedOpt[1] == 8, "[B7c] P8: for modes 1 and 3 every arm-2 / two-shuttle / 32-site / ROI / calibrate-above / latch-teach / ASE-teach option is refused");
        int p4 = 0;
        for (int k = 0; k < 6; ++k) {
            RunGateIn o = g;
            o.h.contactMode = 3;
            switch (k) {
                case 0: o.h.confirmed = false; break;
                case 1: o.h.baselineOn = true; break;
                case 2: o.h.driverPanasonic = false; break;
                case 3: o.h.directionIsOne = true; break;
                case 4: o.h.hookInstalled = false; break;
                case 5: o.h.arm = 1; break;
            }
            if (!RunAllowed(o, why)) ++p4;
        }
        CHECK(p4 == 6, "[B7c] P4 applies to Contact Test (mode 3) too: CONFIRMED / BASELINE (Q90) / driver / Direction / hook / arm 2 each refuse");
        RunGateIn rs = g;
        rs.h.z1Is1203 = false; rs.h.confirmed = false; rs.h.hookInstalled = false; rs.shuttleModeSingle = false; rs.arm2Options = true; rs.twoArm32Site = true;
        int rsOk = 0;
        for (int mode = 0; mode <= 12; ++mode) { rs.h.contactMode = mode; if (RunAllowed(rs, why)) ++rsOk; }
        CHECK(rsOk == 13, "[B7c] P8 / P4: an RS-232 Index Z (not PCI1203) is never refused, whatever the mode or option (golden)");

        // P7 -- HealthFault
        HealthState st;
        Health h;
        h.haveMonitor = true; h.poll = 1; h.sampleValid = true; h.servoOn = true;
        unsigned long t = 5000;
        const bool healthy = !HealthFault(st, h, true, t, why);
        int immediate = 0;
        for (int k = 0; k < 6; ++k) {
            HealthState s2; Health x = h; bool hook = true;
            switch (k) {
                case 0: x.alarm = true; break;
                case 1: x.errorStop = true; break;
                case 2: x.servoOn = false; break;
                case 3: x.routeFault = true; x.routeWhy = "t"; break;
                case 4: x.haveMonitor = false; break;
                case 5: hook = false; break;
            }
            if (HealthFault(s2, x, hook, t, why) && !why.empty()) ++immediate;
        }
        CHECK(healthy && immediate == 6, "[B7c] P7: ALM / ERROR_STOP / servo off / route fault / no monitor / no hook each trip at once; a healthy sample does not");
        HealthState si; Health inv = h; inv.sampleValid = false;
        bool r1 = HealthFault(si, inv, true, t + 100, why); inv.poll = 2;
        bool r2 = HealthFault(si, inv, true, t + 200, why);
        bool r2same = HealthFault(si, inv, true, t + 300, why);   // same poll again: not a new invalid poll
        inv.poll = 3;
        bool r3 = HealthFault(si, inv, true, t + 400, why);
        CHECK(!r1 && !r2 && !r2same && r3, "[B7c] P7: an invalid sample trips on the 3rd DISTINCT invalid poll (the route's kInvalidPolls), not before");
        HealthState sf; Health fz = h; fz.poll = 77;
        bool trip1900 = false, trip2000 = false;
        for (int k = 0; k <= 20; ++k) {
            const bool f = HealthFault(sf, fz, true, t + 100ul * (unsigned long)k, why);
            if (k == 19) trip1900 = f;
            if (k == 20) trip2000 = f;
        }
        CHECK(!trip1900 && trip2000, "[B7c] P7: a monitor whose poll count stands still trips after 2 s of running ticks, not at 1.9 s");
        HealthState sg; Health gp = h; gp.poll = 9;
        for (int k = 0; k < 15; ++k) HealthFault(sg, gp, true, t + 100ul * (unsigned long)k, why);       // 1.4 s still
        const bool afterGap = HealthFault(sg, gp, true, t + 1400ul + 1500ul, why);                    // a blocked tick (golden MySleep)
        const bool soon = HealthFault(sg, gp, true, t + 1400ul + 1500ul + 100ul, why);
        CHECK(!afterGap && !soon, "[B7c] P7: a gap longer than 1 s (golden MySleep(3000) blocks the tick) restarts the frozen clock instead of tripping");

        // P9 -- the commanded floor
        CHECK(CommandAtOrBelowFloor(-148.0, -148.0) && CommandAtOrBelowFloor(-148.03, -148.0) && !CommandAtOrBelowFloor(-147.97, -148.0),
              "[B7c] P9: a step that would start at / below fIndexDownPos is refused; one above it is not");
    }

#ifndef SOFT_SIMULTE
    g_route.owner = MTestZ1;
    g_core.Bind(&g_io);
    W906_SetGaliRoute(&g_route);
    W906_GaliRouteDisableAbsentIndexAxes(false, false, false);               // HT9050: M13 / M15 / M16 Enable 0

    // ================= [B3] simulated descent to the threshold =================
    std::printf("-- [B3] descent: standby, limit kg 15, 300-count steps, trigger at the surface after 10 readings\n");
    {
        StartAt(530);
        int ticks = 0;
        long trig = 0;
        while (fContactForm->Z_Height_Task != 700 && ticks < 3000 && fAllMotorHome) { Tick(); ++ticks; if (!trig && fContactForm->Z_Height_Task == 555 && g_target < g_surface) trig = g_target; }
        CHECK(fContactForm->Z_Height_Task == 700 && fAllMotorHome, "[B3] reached golden case 700 (the contact height is recorded), no port exit");
        CHECK(!g_limits.empty() && g_limits[0] == 150, "[B3] case 536 wrote the torque limit kg 15 x 10 = 150 (0.1 %) through rs232.cpp:950");
        CHECK(!g_moves.empty() && g_moves[0] == -9000, "[B3] first move = standby Tech.iTestZDown-5000 = -9000 (golden :5686)");
        bool steps300 = g_moves.size() >= 3;
        for (std::size_t i = 2; i < g_moves.size(); ++i) if (g_moves[i] != g_moves[i - 1] - 300) steps300 = false;
        CHECK(steps300, "[B3] every descent step is golden's 300 counts (Pos-iStepSpeed, :6070)");
        CHECK(MinMove() == -10200 && g_moves.back() == -10200, "[B3] the first step below the surface triggers; no step after it (min target -10200)");
        CHECK(g_stops >= 1, "[B3] golden 'ST' reached the card at the trigger (:5815)");
        CHECK(std::fabs(std::atof(fContactForm->edContactHeight1->Text.c_str()) - (-100.0)) < 0.001,
              "[B3] edContactHeight1 = the stalled encoder, -100.00 mm (golden :5818-5819)");
    }

    // ================= [B17] (W-152 urgent, laptop) case 560 on HT9050: no Index Y -> golden's HT502 skip of the Y1 check =================
    std::printf("-- [B17] case 560: the Y1-at-the-middle check is skipped on a live 1203 Index Z (HT9050 has no Index Y)\n");
    {
        const int mid0 = Prod.TestY1_Middle;
        Prod.TestY1_Middle = 15700;                                          // the HT9050 recipe value; Y1 is an absent axis there
        MOT[MTestY1].GalilTwoY_Move(-243, 0, 1000, "B17");                   // DoZ1PickFromShuttle :457 "moves" the absent Y1 to TestY1_Front: book value only
        CHECK(MOT[MTestY1].Gali_ReadPos() == -243, "[B17] precondition: the absent Y1 reads its book value -243 (not TestY1_Middle)");
        StartAt(530);
        const int c0 = W906_ShowMyMessage_Count;
        int ticks = 0;
        while (fContactForm->Z_Height_Task != 700 && ticks < 3000 && fAllMotorHome) { Tick(); ++ticks; }
        CHECK(fContactForm->Z_Height_Task == 700 && fAllMotorHome && W906_ShowMyMessage_Count == c0,
              "[B17] Y1 book value -243 vs TestY1_Middle 15700: the descent still reaches case 700, no Y1 position error (AutoHeight :606)");
        Prod.TestY1_Middle = mid0;
    }

    // ================= [B4] P5: no torque value -> ST + exit after 5 s =================
    std::printf("-- [B4] P5: the 6077h value never arrives\n");
    {
        StartAt(530);
        g_torqueOk = false;
        unsigned long t555 = 0;
        int ticks = 0;
        bool ret = true;
        int stopsBefore = 0, movesAtExit = -1;
        while (fAllMotorHome && ticks < 400) {
            stopsBefore = g_stops;
            ret = Tick();
            ++ticks;
            if (!t555 && fContactForm->Z_Height_Task == 555) t555 = g_now;
        }
        movesAtExit = Moves();
        const unsigned long waited = g_now - t555;
        CHECK(!fAllMotorHome && !ret && fContactForm->CarlibrationTask == 1, "[B4] golden case 536's exit taken (fAllMotorHome=false, CarlibrationTask=1, return false)");
        CHECK(t555 != 0 && waited >= 5000ul && waited <= 5200ul, "[B4] fired after 5 s of waiting at case 555 -- not before");
        CHECK(g_stops == stopsBefore + 1, "[B4] 'ST' sent to the card in the exit tick");
        CHECK(Has(W906_ShowMyMessage_LastS1, "did not arrive in 5 s"), "[B4] the message says the value did not arrive");
        for (int k = 0; k < 30; ++k) Tick();
        CHECK(Moves() == movesAtExit, "[B4] no move after the exit");
    }

    // ================= [B5] P7: a device error mid-descent =================
    std::printf("-- [B5] P7: drive errors mid-descent -> ST in the same tick, no further move\n");
    {
        struct V { const char* name; int kind; };
        const V vs[] = { {"drive ALARM", 0}, {"ERROR_STOP", 1}, {"servo OFF", 2}, {"sample invalid (3 polls)", 3},
                         {"monitor frozen (2 s)", 4}, {"no health hook", 5}, {"route-latched failure", 6} };
        for (std::size_t vi = 0; vi < sizeof(vs) / sizeof(vs[0]); ++vi) {
            StartAt(530);
            g_rawPress = -149;                                   // never triggers: the descent keeps stepping
            int ticks = 0;
            while (g_target > -9600 && ticks < 500) { Tick(); ++ticks; }
            const int movesBefore = Moves();
            const int stops0 = g_stops;
            const int msg0 = W906_ShowMyMessage_Count;
            switch (vs[vi].kind) {
                case 0: g_bits |= 0x2ul; break;
                case 1: g_state = 3; break;
                case 2: g_bits &= ~0x4000ul; break;
                case 3: g_sampleValid = false; break;
                case 4: break;
                case 5: W906_Pci1203IndexZHealthHook = 0; break;
                case 6: g_routeFault = true; break;
            }
            bool ret = true;
            int n = 0;
            const int maxN = (vs[vi].kind == 3) ? 3 : (vs[vi].kind == 4 ? 40 : 1);
            while (fAllMotorHome && n < maxN) { ret = Tick(100, vs[vi].kind != 4); ++n; }
            char m[200];
            std::snprintf(m, sizeof(m), "[B5] %s: golden exit within %d tick(s), ST sent, message once", vs[vi].name, maxN);
            CHECK(!fAllMotorHome && !ret && fContactForm->CarlibrationTask == 1 && g_stops > stops0 &&
                  W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "drive error"), m);
            if (vs[vi].kind < 3 || vs[vi].kind >= 5) {
                std::snprintf(m, sizeof(m), "[B5] %s: stopped in the SAME tick as the fault, no step issued in it", vs[vi].name);
                CHECK(n == 1 && Moves() == movesBefore, m);
            }
            const int movesAtExit = Moves();
            for (int k = 0; k < 20; ++k) Tick(100, vs[vi].kind != 4);
            std::snprintf(m, sizeof(m), "[B5] %s: no move afterwards while the fault stays (Steven 1004: the machine must not move)", vs[vi].name);
            CHECK(Moves() == movesAtExit, m);
            W906_Pci1203IndexZHealthHook = &FakeHealth;
        }
        //  AI(W906-E042) 20261004 (restart 2, St01): this check used to expect the RUN-ENTRY refusal shape here (SystemStart
        //  off, nothing sent at all) and failed in SHIP at 1004 16:27.  Cause = the test expectation, not the code: golden
        //  reaches Do_Z1 case 1 only from DoTestContactFunction case 400, AFTER case 300's DoZ1PickFromShuttle has moved Z1
        //  (golden :12770, :13088), so Do_Z1 case 1 is mid-run.  P7 on the switch line ("every case", plan s3.5) rightly
        //  takes the mid-run shape: "ST" first (a stop is never gated), the message once, golden case 536's exit
        //  (fAllMotorHome=false; the contact SM's case 1 then says "Must home first" and clears SystemStart, golden
        //  :11837-11841).  The "refused, nothing sent" entry belongs to DoTestContactFunction case 1 (step B4): its
        //  predicate W906_IndexZRunRefused is pinned right below.
        StartAt(1);
        g_bits |= 0x2ul;
        int msg0 = W906_ShowMyMessage_Count;
        const bool ret = Tick();
        CHECK(!ret && !fAllMotorHome && fContactForm->CarlibrationTask == 1 && Moves() == 0 && g_limits.empty() && g_stops >= 1 &&
              W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "drive error"),
              "[B5] ALARM when Do_Z1 starts (case 1; golden runs it after DoZ1PickFromShuttle): ST + golden exit in that tick, no motion command, message once");
        for (int k = 0; k < 20; ++k) Tick();
        CHECK(Moves() == 0 && g_limits.empty(), "[B5] ALARM when Do_Z1 starts: still no motion command / limit write 20 ticks later");
        W906_IndexZHealthReset();
        const int s0 = g_stops;
        msg0 = W906_ShowMyMessage_Count;
        const bool refused = W906_IndexZRunRefused(CONTACT_AUTO_GET_HEIGHT, 0, "test run entry");
        CHECK(refused && g_stops == s0 && Moves() == 0 && W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "drive error"),
              "[B5] ALARM at the run entry (W906_IndexZRunRefused = DoTestContactFunction case 1 in B4): refused with 'drive error', nothing sent, not even a stop");
        g_bits &= ~0x2ul;
        W906_IndexZHealthReset();
        AnsiString why;
        CHECK(W906_IndexZRunAllowed(CONTACT_AUTO_GET_HEIGHT, 0, &why), "[B5] the same run entry is allowed again once the alarm is gone");
    }

    // ================= [B6] P9: the commanded floor =================
    std::printf("-- [B6] P9: 6077h saturates just under kg (14.90 -> atoi 14 < 15), Z stalled on the surface\n");
    {
        StartAt(530);
        g_rawPress = -149;
        int ticks = 0;
        bool ret = true;
        int stopsBefore = 0;
        while (fAllMotorHome && ticks < 5000) { stopsBefore = g_stops; ret = Tick(); ++ticks; }
        CHECK(!fAllMotorHome && !ret && fContactForm->CarlibrationTask == 1 && Has(W906_ShowMyMessage_LastS1, "floor"),
              "[B6] stopped by P9 with the golden exit (golden would keep stepping down)");
        CHECK(g_stops == stopsBefore + 1, "[B6] 'ST' sent in the exit tick");
        CHECK(MinMove() >= -14800 - 300 && MinMove() <= -14800, "[B6] the deepest commanded target is at most one 300-count step below the floor (-148.00 mm)");
        const int movesAtExit = Moves();
        for (int k = 0; k < 20; ++k) Tick();
        CHECK(Moves() == movesAtExit, "[B6] no move afterwards");
    }

    // ================= [B2] entry refusal (P4 + P8 + P7 at case 1) =================
    std::printf("-- [B2] case 1 entry gate\n");
    {
        struct R { const char* name; const char* ini1; const char* ini2; int mode; int shuttleMode; bool arm2; const char* expect; };
        const R rs[] = {
            { "CONFIRMED key missing",     0, 0, CONTACT_AUTO_GET_HEIGHT, 1, false, "HT9050_INDEXZ_TORQUE_CONFIRMED" },
            { "CONFIRMED = 0",             "HT9050_INDEXZ_TORQUE_CONFIRMED=0", 0, CONTACT_AUTO_GET_HEIGHT, 1, false, "E-10" },
            { "BASELINE = 1 (Q90)",        "HT9050_INDEXZ_TORQUE_CONFIRMED=1", "HT9050_INDEXZ_TORQUE_BASELINE=1", CONTACT_AUTO_GET_HEIGHT, 1, false, "Q90" },
            { "mode 2 Manual Height",      "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, CONTACT_MANUAL_GET_HEIGHT, 1, false, "contact mode 2" },
            { "mode 4 Auto Contact Test",  "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, AUTO_CONTACT_TEST, 1, false, "contact mode 4" },
            { "mode 3, two shuttles",      "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, CONTACT_TEST, 0, false, "single shuttle" },
            { "mode 3, CONFIRMED = 0",     "HT9050_INDEXZ_TORQUE_CONFIRMED=0", 0, CONTACT_TEST, 1, false, "E-10" },
            { "mode 8 Load Cell",          "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, CONTACT_LoadCell_AUTO_GET_HEIGHT, 1, false, "contact mode 8" },
            { "two shuttles",              "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, CONTACT_AUTO_GET_HEIGHT, 0, false, "single shuttle" },
            { "Index arm 2 light option",  "HT9050_INDEXZ_TORQUE_CONFIRMED=1", 0, CONTACT_AUTO_GET_HEIGHT, 1, true,  "arm 2" },
        };
        for (std::size_t ri = 0; ri < sizeof(rs) / sizeof(rs[0]); ++ri) {
            UseIni(rs[ri].ini1, rs[ri].ini2);
            StartAt(1);
            iContactMode = rs[ri].mode;
            TestIF.iShuttleMode = rs[ri].shuttleMode;
            IniConfig.bIndexArm2SupplyLight = rs[ri].arm2;
            const int msg0 = W906_ShowMyMessage_Count;
            const bool ret = Tick();
            char m[220];
            std::snprintf(m, sizeof(m), "[B2] %s: refused at case 1 (SystemStart off, CarlibrationTask 1), nothing commanded, message names '%s'",
                          rs[ri].name, rs[ri].expect);
            CHECK(!ret && !SystemStart && fContactForm->CarlibrationTask == 1 && fAllMotorHome && Moves() == 0 && g_stops == 0 &&
                  g_limits.empty() && W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, rs[ri].expect), m);
        }
        iContactMode = CONTACT_AUTO_GET_HEIGHT; TestIF.iShuttleMode = 1; IniConfig.bIndexArm2SupplyLight = false;
        UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
        StartAt(1);
        Prod.TestZ1_Safe = -2000;
        const int msg0 = W906_ShowMyMessage_Count;
        bool moved = false;
        for (int k = 0; k < 5 && !moved; ++k) { Tick(); moved = Moves() > 0; }
        CHECK(moved && g_moves[0] == -2000 && SystemStart && W906_ShowMyMessage_Count == msg0,
              "[B2] confirmed + healthy + mode 1 single arm: case 1 runs golden (Gali_Two_ZAxis_Move to TestZ1_Safe, :5524)");
        iContactMode = CONTACT_TEST;                                         // Steven 1004 08:4x: mode 3 is on the whitelist too
        StartAt(1);
        Prod.TestZ1_Safe = -2000;
        const int msg3 = W906_ShowMyMessage_Count;
        moved = false;
        for (int k = 0; k < 5 && !moved; ++k) { Tick(); moved = Moves() > 0; }
        CHECK(moved && g_moves[0] == -2000 && SystemStart && W906_ShowMyMessage_Count == msg3,
              "[B2] confirmed + healthy + mode 3 Contact Test single arm: case 1 runs golden too (Steven 1004 08:4x)");
        iContactMode = CONTACT_AUTO_GET_HEIGHT;
    }

    // ================= [B8] a stop is never gated =================
    std::printf("-- [B8] stop never gated\n");
    {
        UseIni(0);                                                           // CONFIRMED missing
        StartAt(530);
        g_bits |= 0x2ul;                                                     // and the drive in alarm
        const int s0 = g_stops;
        MOT[MTestY1].Gali_Command("ST");                                     // golden's operator / alarm stop string
        CHECK(g_stops == s0 + 1, "[B8] 'ST' reaches the card with CONFIRMED missing and the drive in alarm");
        const int s1 = g_stops;
        CHECK(W906_IndexZDriveFaultStop("test") && g_stops == s1 + 1, "[B8] P7's own trip sends 'ST' first");
        UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
    }

    // ================= [B1] RS-232 / Panasonic row: golden unchanged =================
    std::printf("-- [B1] CardType != PCI1203: golden, untouched\n");
    {
        W906_SetGaliRoute(0);
        MOT[MTestZ1].CardType = "";
        StartAt(550);
        const int msg0 = W906_ShowMyMessage_Count;
        bool stayed = true;
        for (int k = 0; k < 600; ++k) { Tick(100, true, false); if (k > 2 && fContactForm->Z_Height_Task != 555) stayed = false; }   // 60 s, no value
        CHECK(stayed && fAllMotorHome && SystemStart && W906_ShowMyMessage_Count == msg0,
              "[B1] waits at case 555 for 60 s with no value: no timeout, no exit, no message (golden waits for ever)");
        bool p5 = false;
        for (int k = 0; k < 120; ++k) { g_now += 100; if (W906_IndexZTorqueWaitTimedOut(0, true) || W906_IndexZTorqueWaitStop(0, true, "t")) p5 = true; }
        CHECK(!p5, "[B1] P5 (Phase A's W906_IndexZTorqueWaitTimedOut and the call-site W906_IndexZTorqueWaitStop) never fires on an RS-232 row over 12 s");
        AnsiString why;
        CHECK(!W906_IndexZLive1203() && W906_IndexZRunAllowed(CONTACT_MANUAL_GET_HEIGHT, 1, &why) && !W906_IndexZDriveFault(&why) &&
              !W906_IndexZTorqueWaitStop(0, true, "t") && !W906_IndexZTorqueWaitTimedOut(0, true) &&
              !W906_IndexZCommandFloorStop(-20000, -148.0, "t") && !W906_IndexZDriveFaultStop("t"),
              "[B1] every E-042 helper is inert on an RS-232 row (P4 / P5 / P7 / P8 / P9)");
        MOT[MTestZ1].CardType = "PCI1203";
    }
#else
    // ================= [B12] SIM: golden SOFT_SIMULTE =================
    std::printf("-- [B12] SIM: no 1203 is driven; golden's SOFT_SIMULTE torque (30) path\n");
    {
        AnsiString why;
        CHECK(!W906_IndexZLive1203() && W906_IndexZRunAllowed(CONTACT_AUTO_GET_HEIGHT, 0, &why) && !W906_IndexZDriveFaultStop("t") &&
              !W906_IndexZTorqueWaitStop(0, true, "t") && !W906_IndexZCommandFloorStop(-20000, -148.0, "t"),
              "[B12] CardType PCI1203 in SIM: every helper inert (the route is not installed in a SIM build)");
        //  AI(W906-E042) 20261004 (restart 2, St01): the SIM step.  This check failed at 1004 16:27 with Do_Z1 parked at
        //  case 530.  Cause = the harness, not the code: in SIM nothing owns M14 (no Galil card, no route), so golden's
        //  Gali_MotMove takes its no-card branch (Motor/myGALILmotor.cpp:2055-2094) and steps Position by the TMyMotor
        //  member `speed`.  For a SIM index axis only golden's SetMotorScaleSpeed sets it, and only when the axis is
        //  DISABLED (cinitial.cpp:13541-13553 -> SetSpeed -> mymotor.cpp:351); SetSpeed skips an ENABLED index axis
        //  (mymotor.cpp:323-329) and the constructor never sets it (global MOT[] -> 0).  This harness enables M14 (the
        //  SHIP route needs it), so `speed` stayed 0 and case 530's standby move never arrived.  Give it golden's own
        //  minimum sim step (mymotor.cpp:677-678 `if(speed<=0) speed=100;`); the golden lines under test are unchanged.
        MOT[MTestZ1].speed = 100;
        StartAt(530);
        int ticks = 0;
        while (fContactForm->Z_Height_Task != 700 && ticks < 200000 && fAllMotorHome) { Tick(1, false, true); ++ticks; }
        char m[240];
        std::snprintf(m, sizeof(m), "[B12] golden SOFT_SIMULTE: edTorue0=30 >= kg 15 -> case 700 (golden :5769-5770) -- Task %d after %d tick(s), edTorue0 '%s'",
                      fContactForm->Z_Height_Task, ticks, fMain->edTorue0->Text.c_str());
        CHECK(fContactForm->Z_Height_Task == 700 && fAllMotorHome && fMain->edTorue0->Text == AnsiString("30"), m);
        CHECK(g_limits.empty(), "[B12] SIM: no torque-limit write reaches the 1203 hook (golden SOFT_SIMULTE iWriteAndCheckMotorTorque returns 1 at once, rs232.cpp:940-946)");
    }
#endif

    // ================= [B13] (B2) SetContactMode: golden mode switch; the P8 chain B4 runs on its output =================
    //  AI(W906-E042) 20261004 (B2, St01): golden 0618 cContact.cpp:15341-15434, forms/fContact_ContactSM.cpp, NO caller.
    //  A MODE SWITCH (writes iContactMode) and nothing else: no motion, IO, ini write or message.  The HT9050 refusal of
    //  Manual Height (2) / Load Cell (8) is NOT in it (golden lets the operator pick any mode); it is P8 at
    //  DoTestContactFunction case 1 right after this call (golden :11790, B4), pinned here on this function's output.
    std::printf("-- [B13] (B2) SetContactMode: golden mode switch + the P8 chain on its output\n");
    {
        TfContact* F = fContactForm;
        TRadioButton* rbs[10] = { F->rbModeNormal, F->rbDeviceMapping, F->rbAutoHeight, F->rbManualHeight, F->rbContactTest,
                                  F->rbAutoContactTest, F->rbStepContactTest, F->rbLoadCellAutoHigh, F->rbDeviceLoopTest, F->rbKTempIndexMove };
        const char* rbName[10] = { "rbModeNormal", "rbDeviceMapping", "rbAutoHeight", "rbManualHeight", "rbContactTest",
                                   "rbAutoContactTest", "rbStepContactTest", "rbLoadCellAutoHigh", "rbDeviceLoopTest", "rbKTempIndexMove" };
        const int want[10] = { 0, 9, 1, 2, 3, 4, 5, 8, 10, 11 };             // golden cContact.cpp:74-85, as numbers on purpose
        const char* memo[10] = { "CONTACT_NORMAL", "CONTACT_DEVICE_MAP_CHECK", "CONTACT_AUTO_GET_HEIGHT", "CONTACT_MANUAL_GET_HEIGHT",
                                 "CONTACT_TEST", "AUTO_CONTACT_TEST", "STEP_CONTACT_TEST", "CONTACT_LoadCell_AUTO_GET_HEIGHT",
                                 "CONTACT_DEVICE_LOOP_TEST", "K_Temp_Index_Move" };
        const int cc0 = CUSTOMER_CODE;
        const bool a16_0 = IniConfig.bA16ContactTestDropContact;
        const int mode0 = iContactMode;
#ifndef SOFT_SIMULTE
        W906_SetGaliRoute(&g_route);                                         // so "nothing sent" is observed on the fake 1203
        MOT[MTestZ1].CardType = "PCI1203";
#endif
        UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
        W906_IndexZHealthReset();
        TestIF.iShuttleMode = 1; TestIF.iShuttle_Sel = 0; IniConfig.bIndexArm2SupplyLight = false;
        fAllMotorHome = true; SystemStart = true; F->CarlibrationTask = 1;
        g_bits = 0x00004000ul; g_state = 1; g_sampleValid = true; g_healthMonitor = true; g_routeFault = false;
        g_moves.clear(); g_limits.clear(); g_stops = 0;
        const int msgA = W906_ShowMyMessage_Count;
        //  one radio on (VCL TurnSiblingsOff), the others off, then the golden body
        int picked[10];
        auto Pick = [&](int k) { for (int j = 0; j < 10; ++j) rbs[j]->Checked = (j == k); F->Memo1->Lines->Clear(); SetMode(); };
        auto Memo = [&](int i) { return std::string(AnsiString(F->Memo1->Lines->Strings[i]).c_str()); };

        // (a) each radio -> golden's value and memo line; K Temp ticks chk_K_Temperature; Step-only Daily Correlation;
        //     One-Touch hidden and cleared off the KYEC / MAXIM customers (HT9050 = PTI 957)
        CUSTOMER_CODE = CC_PTI; IniConfig.bA16ContactTestDropContact = false;
        std::string bad;
        for (int k = 0; k < 10; ++k) {
            iContactMode = -7;
            F->chk_K_Temperature->Checked = false;
            F->chkDailyCorrelation->Checked = true; F->chkDailyCorrelation->Enabled = (k % 2) == 0;
            F->cbOneTouchAutoContactHight->Checked = true; F->cbOneTouchAutoContactHight->Visible = true;
            Pick(k);
            picked[k] = iContactMode;
            const bool step = (k == 6);
            const bool ok = iContactMode == want[k] && F->Memo1->Lines->Count == 1 && Memo(0) == memo[k] &&
                            F->chk_K_Temperature->Checked == (k == 9) &&
                            F->chkDailyCorrelation->Enabled == step && F->chkDailyCorrelation->Checked == step &&
                            !F->cbOneTouchAutoContactHight->Visible && !F->cbOneTouchAutoContactHight->Checked;
            if (!ok) bad += std::string(rbName[k]) + "(" + std::to_string(iContactMode) + ") ";
        }
        char m[400];
        std::snprintf(m, sizeof(m), "[B13] each radio -> golden's iContactMode + memo line; K Temp ticks chk_K_Temperature; Daily Correlation only for Step; One-Touch hidden + cleared (PTI) %s", bad.c_str());
        CHECK(bad.empty(), m);

        // (b) golden's arm order decides when more than one radio is on (Normal, DeviceMapping, AutoHeight, Manual, ContactTest, ...)
        auto Multi = [&](std::initializer_list<int> on) { for (int j = 0; j < 10; ++j) rbs[j]->Checked = false; for (int j : on) rbs[j]->Checked = true; F->Memo1->Lines->Clear(); iContactMode = -7; SetMode(); return iContactMode; };
        const int all = Multi({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        const int dmAh = Multi({1, 2});
        const int ahMh = Multi({2, 3});
        const int mhLc = Multi({3, 7});
        const int ctKt = Multi({4, 9});
        std::snprintf(m, sizeof(m), "[B13] golden's arm order wins: all ten -> 0, DeviceMapping+AutoHeight -> 9, AutoHeight+Manual -> 1, Manual+LoadCell -> 2, ContactTest+KTemp -> 3 (got %d %d %d %d %d)", all, dmAh, ahMh, mhLc, ctKt);
        CHECK(all == 0 && dmAh == 9 && ahMh == 1 && mhLc == 2 && ctKt == 3, m);

        // (c) no radio on (the facade radios as the port leaves them today) -> iContactMode is not touched, no memo line
        for (int j = 0; j < 10; ++j) rbs[j]->Checked = false;
        F->Memo1->Lines->Clear();
        iContactMode = CONTACT_TEST;                                         // e.g. set by the web's DF_SetContactMode
        SetMode();
        CHECK(iContactMode == 3 && F->Memo1->Lines->Count == 0,
              "[B13] no radio on (facade radios unchecked) -> iContactMode keeps the value the web set (golden: no arm fires) -- the B4 bridge note");

        // (d) One-Touch Auto Contact Height: KYEC_LEE / KYEC_XILINX / MAXIM_THAILAND show it only for Auto Height
        int oneTouch = 0;
        const int ccs[3] = { CC_KYEC_LEE, CC_KYEC_XILINX, CC_MAXIM_THAILAND };
        for (int c = 0; c < 3; ++c) {
            CUSTOMER_CODE = ccs[c];
            F->cbOneTouchAutoContactHight->Checked = true; F->cbOneTouchAutoContactHight->Visible = false;
            Pick(2);
            if (F->cbOneTouchAutoContactHight->Visible && F->cbOneTouchAutoContactHight->Checked) ++oneTouch;
            Pick(3);
            if (!F->cbOneTouchAutoContactHight->Visible && !F->cbOneTouchAutoContactHight->Checked) ++oneTouch;
        }
        CHECK(oneTouch == 6, "[B13] KYEC_LEE / KYEC_XILINX / MAXIM_THAILAND: One-Touch visible (Checked kept) for Auto Height, hidden + cleared otherwise (golden :15405-15416)");

        // (e) KYEC_CHEN + A16: Normal -> ChangeContactMode(false), Contact Test -> ChangeContactMode(true); otherwise untouched
        auto Seed = [&]() { F->cbContactMode->ItemIndex = 5; F->cbVacuumMode->ItemIndex = 7; F->edDropWaitTime->Text = "9.9"; F->edDropOffset1->Enabled = false; F->edDropOffset2->Enabled = true; };
        auto Seeded = [&]() { return F->cbContactMode->ItemIndex == 5 && F->cbVacuumMode->ItemIndex == 7 && F->edDropWaitTime->Text == AnsiString("9.9"); };
        CUSTOMER_CODE = CC_KYEC_CHEN; IniConfig.bA16ContactTestDropContact = true;
        Seed(); Pick(0);
        const bool chenNormal = F->cbContactMode->ItemIndex == DirectContactMode && F->cbVacuumMode->ItemIndex == VacuumOFFMode &&
                                F->edDropWaitTime->Text == AnsiString("0.5") && !F->edDropOffset1->Enabled && !F->edDropOffset2->Enabled;
        Seed(); Pick(4);
        const bool chenTest = F->cbContactMode->ItemIndex == DropContact && F->cbVacuumMode->ItemIndex == VacuumOFFMode &&
                              F->edDropWaitTime->Text == AnsiString("0.5") && F->edDropOffset1->Enabled && F->edDropOffset2->Enabled;
        Seed(); Pick(2);
        const bool chenOther = Seeded();
        IniConfig.bA16ContactTestDropContact = false;
        Seed(); Pick(4);
        const bool chenNoA16 = Seeded();
        CUSTOMER_CODE = CC_PTI; IniConfig.bA16ContactTestDropContact = true;
        Seed(); Pick(4);
        const bool ptiA16 = Seeded();
        CHECK(chenNormal && chenTest && chenOther && chenNoA16 && ptiA16,
              "[B13] KYEC_CHEN + A16: Normal -> direct contact, Contact Test -> drop contact (ChangeContactMode, golden :15346-15350 / :15372-15376); no change for other modes, without A16, or on other customers");
        IniConfig.bA16ContactTestDropContact = false;

        // (f) a mode switch is UI state only: nothing reached the 1203, no message, home / start / task untouched
        CHECK(Moves() == 0 && g_stops == 0 && g_limits.empty() && W906_ShowMyMessage_Count == msgA && fAllMotorHome && SystemStart && F->CarlibrationTask == 1,
              "[B13] SetContactMode (every call above) sent nothing to the card, showed no message, left fAllMotorHome / SystemStart / CarlibrationTask alone");

        // (g) the P8 chain on its output, pure decision (both configurations): HT9050 = modes 1 and 3 only
        CUSTOMER_CODE = CC_PTI;
        RunGateIn g;
        g.h.z1Is1203 = true; g.h.confirmed = true; g.h.driverPanasonic = true; g.h.hookInstalled = true;
        g.shuttleModeSingle = true; g.shuttleSel = 0;
        std::string why, badP, whyMh, whyLc;
        int rsOk = 0;
        for (int k = 0; k < 10; ++k) {
            Pick(k);
            g.h.contactMode = iContactMode;
            const bool ok = RunAllowed(g, why);
            if (ok != (k == 2 || k == 4)) badP += std::string(rbName[k]) + " ";
            if (k == 3) whyMh = why;
            if (k == 7) whyLc = why;
            RunGateIn rs = g; rs.h.z1Is1203 = false;
            if (RunAllowed(rs, why)) ++rsOk;
        }
        std::snprintf(m, sizeof(m), "[B13] chain (pure P8): of the ten radios only rbAutoHeight and rbContactTest may start on a PCI-1203 Index Z %s", badP.c_str());
        CHECK(badP.empty(), m);
        CHECK(whyMh.find("Manual Height stays refused") != std::string::npos && whyLc.find("Load Cell") != std::string::npos,
              "[B13] chain (pure P8): rbManualHeight -> mode 2 refused 'Manual Height stays refused', rbLoadCellAutoHigh -> mode 8 refused 'Load Cell' (Steven 1004 08:4x)");
        CHECK(rsOk == 10, "[B13] chain (pure P8): RS-232 / Panasonic Index Z -- every radio's mode passes (golden)");

#ifndef SOFT_SIMULTE
        // (h) the live chain B4 will run (W906_IndexZRunRefused at DoTestContactFunction case 1), SHIP, healthy + confirmed HT9050
        int refused = 0, msgs0 = W906_ShowMyMessage_Count;
        std::string badL;
        bool mhMsg = false, lcMsg = false;
        for (int k = 0; k < 10; ++k) {
            Pick(k);
            const int c0 = W906_ShowMyMessage_Count;
            const bool r = W906_IndexZRunRefused(iContactMode, 0, "test B13");
            if (r) ++refused;
            if (r != !(k == 2 || k == 4)) badL += std::string(rbName[k]) + " ";
            if (k == 3) mhMsg = r && W906_ShowMyMessage_Count == c0 + 1 && Has(W906_ShowMyMessage_LastS1, "Manual Height stays refused");
            if (k == 7) lcMsg = r && W906_ShowMyMessage_Count == c0 + 1 && Has(W906_ShowMyMessage_LastS1, "Load Cell");
        }
        std::snprintf(m, sizeof(m), "[B13] chain (live, HT9050 PCI1203 + CONFIRMED=1 + healthy): rbAutoHeight / rbContactTest start, the other 8 refused, one message each %s", badL.c_str());
        CHECK(badL.empty() && refused == 8 && W906_ShowMyMessage_Count == msgs0 + 8, m);
        CHECK(mhMsg && lcMsg, "[B13] chain (live): Manual Height and Load Cell refused with their reasons on the screen");
        CHECK(Moves() == 0 && g_stops == 0 && g_limits.empty(), "[B13] chain (live): a refusal sends nothing to the 1203 (not a move, not a limit, not even a stop)");
        // a drive alarm does not block the UI switch; the chain then refuses even mode 1 with 'drive error', nothing sent
        g_bits |= 0x2ul;
        W906_IndexZHealthReset();
        Pick(2);
        const bool uiOk = iContactMode == 1;
        const bool alm = W906_IndexZRunRefused(iContactMode, 0, "test B13 ALM") && Has(W906_ShowMyMessage_LastS1, "drive error");
        g_bits &= ~0x2ul;
        W906_IndexZHealthReset();
        CHECK(uiOk && alm && Moves() == 0 && g_stops == 0, "[B13] chain (live): with M14 in ALARM the mode switch still works (UI) and the run entry refuses Auto Height with 'drive error', nothing sent");
        // Panasonic / RS-232 Index Z: the same modes, never refused, no message
        MOT[MTestZ1].CardType = "";
        int same = 0, pass = 0;
        const int msgP = W906_ShowMyMessage_Count;
        for (int k = 0; k < 10; ++k) {
            Pick(k);
            if (iContactMode == picked[k]) ++same;
            if (!W906_IndexZRunRefused(iContactMode, 0, "test B13 RS-232")) ++pass;
        }
        CHECK(same == 10 && pass == 10 && W906_ShowMyMessage_Count == msgP && Moves() == 0 && g_stops == 0,
              "[B13] Panasonic / RS-232 Index Z (CardType ''): every radio gives the same mode as on HT9050 and none is refused (golden path unchanged)");
        MOT[MTestZ1].CardType = "PCI1203";
        W906_SetGaliRoute(0);
#else
        // (h) SIM: no 1203 is driven -- the live run entry never refuses (golden SOFT_SIMULTE)
        int pass = 0;
        const int msgP = W906_ShowMyMessage_Count;
        for (int k = 0; k < 10; ++k) { Pick(k); if (!W906_IndexZRunRefused(iContactMode, 0, "test B13 SIM")) ++pass; }
        CHECK(pass == 10 && W906_ShowMyMessage_Count == msgP, "[B13] SIM: every radio's mode passes the live run entry, no message (golden SOFT_SIMULTE)");
#endif
        for (int j = 0; j < 10; ++j) rbs[j]->Checked = false;
        F->Memo1->Lines->Clear();
        CUSTOMER_CODE = cc0; IniConfig.bA16ContactTestDropContact = a16_0; iContactMode = mode0;
    }

    // ================= [B14] (B3) W-44 / P7 / P5 / P9 on the new state machines =================
    //  AI(W906-E042) 20261005 (B3, St01).  W-44 = Steven (decisions-decided review6 f4baf1d2); scope ST01-M 1005 03:4x:
    //  socket press ONLY -- the pick / place descents need the shuttle parked under the index and stay golden.
    std::printf("-- [B14] (B3) W-44 socket press, P7 / P5 / P9 on the pick / place / Do_Z2 / calibrate state machines\n");
    {
        char m[300];
        //  pure W-44 decision (both configurations)
        //  AI(W906-E042) 20261005 (B3b): "home" = the taught clear positions InSHT[0].iLeft / OutSHT[0].iRight (ST01-M), not 0.
        ShuttleHomeIn sh; std::string why;
        sh.inPresent = sh.outPresent = true; sh.inTarget = 2000; sh.outTarget = -1500;
        sh.inPos = 2000; sh.outPos = -1500;
        const bool homeOk = ShuttlesAtHome(sh, why);
        sh.inPos = 2100; const bool tolOk = ShuttlesAtHome(sh, why);
        sh.inPos = 2101; const bool inAway = !ShuttlesAtHome(sh, why) && why.find("In shuttle 1 at 2101 / InSHT[0].iLeft 2000 NOT home") != std::string::npos;
        sh.inPos = 0; const bool zeroIsNotHome = !ShuttlesAtHome(sh, why);
        sh.inPos = 2000; sh.outPos = -5000; const bool outAway = !ShuttlesAtHome(sh, why) && why.find("Out shuttle 1 at -5000 / OutSHT[0].iRight -1500 NOT home") != std::string::npos;
        sh.outPos = -1500; sh.outPresent = false; const bool missing = !ShuttlesAtHome(sh, why) && why.find("missing") != std::string::npos;
        CHECK(homeOk && tolOk && inAway && zeroIsNotHome && outAway && missing, "[B14] W-44 pure: both shuttles within 100 counts of InSHT[0].iLeft / OutSHT[0].iRight -> press allowed; away (0 included) / a missing axis -> refused with the reason");
        int pre = 0, press = 0;
        const int preT[9] = { 1, 100, 110, 150, 151, 200, 520, 525, 2200 };
        const int pressT[8] = { 530, 535, 536, 550, 555, 560, 7170, 2900 };
        for (int k = 0; k < 9; ++k) if (!W44PressTask(preT[k])) ++pre;
        for (int k = 0; k < 8; ++k) if (W44PressTask(pressT[k])) ++press;
        CHECK(pre == 9 && press == 8, "[B14] W-44 pure: golden's pre-descent tasks (1 / 100 / 110 / 150 / 151 / 200 / 520 / 525 / 2200) are free, the socket-press tasks (530-560, 7170, 2900 ...) are guarded");
#ifndef SOFT_SIMULTE
        W906_SetGaliRoute(&g_route);
        MOT[MTestZ1].CardType = "PCI1203";
        UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
        iContactMode = CONTACT_AUTO_GET_HEIGHT; TestIF.iShuttleMode = 1; TestIF.iShuttle_Sel = 0; IniConfig.bIndexArm2SupplyLight = false;
        //  B3b: the taught clear positions are the W-44 targets; the fake positions are given RELATIVE to them below.
        const int inL0 = Prod.InSHT[0].iLeft, outR0 = Prod.OutSHT[0].iRight;
        Prod.InSHT[0].iLeft = 2000; Prod.OutSHT[0].iRight = -1500;
        auto ShAt = [&](long dIn, long dOut) { g_inShPos = 2000 + dIn; g_outShPos = -1500 + dOut; };
        ShAt(0, 0);
        MOT[MInShuttle1].Position = 2000; MOT[MOutShuttle1].Position = -1500;   // golden Do_Z1 case 150 reads MOT[].ReadPos() (:5559-5560): parked
        // (1) W-44 live on Do_Z1: In shuttle away at the socket descent -> ST + golden exit in that tick, no motion
        StartAt(530);
        ShAt(5000, 0);
        int msg0 = W906_ShowMyMessage_Count;
        bool ret = Tick();
        CHECK(!ret && !fAllMotorHome && fContactForm->CarlibrationTask == 1 && g_stops >= 1 && Moves() == 0 && g_limits.empty() &&
              W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "W-44"),
              "[B14] W-44: In shuttle away when Do_Z1 reaches the socket descent (530) -> ST + golden exit in that tick, no move, no limit write, message once");
        for (int k = 0; k < 20; ++k) Tick();
        CHECK(Moves() == 0, "[B14] W-44: no move afterwards while the shuttle stays away");
        ShAt(0, -3000);
        W906_IndexZShuttleHomeStop(1, "test: a pre-descent tick");          // a new run passes case 1 first: re-arms the once-per-trip message
        StartAt(560);
        msg0 = W906_ShowMyMessage_Count;
        ret = Tick();
        CHECK(!ret && !fAllMotorHome && g_stops >= 1 && Moves() == 0 && Has(W906_ShowMyMessage_LastS1, "Out shuttle 1 at -4500 / OutSHT[0].iRight -1500 NOT home"),
              "[B14] W-44: Out shuttle away mid-descent (560) -> ST + golden exit, no further step");
        ShAt(0, 0); g_shPresent = false;
        W906_IndexZShuttleHomeStop(1, "test: a pre-descent tick");
        StartAt(530);
        ret = Tick();
        CHECK(!ret && !fAllMotorHome && Moves() == 0, "[B14] W-44: a shuttle position that cannot be read counts as NOT home (fail safe)");
        g_shPresent = true;
        // (2) the whole run from case 1 with the In shuttle away: Z1 goes up to the safe height (golden), the socket descent never starts
        ShAt(5000, 0);
        StartAt(1);
        Prod.TestZ1_Safe = -2000;
        msg0 = W906_ShowMyMessage_Count;
        int ticks = 0;
        while (fAllMotorHome && ticks < 200) { Tick(); ++ticks; }
        CHECK(!fAllMotorHome && Has(W906_ShowMyMessage_LastS1, "W-44") && !g_moves.empty() && g_moves[0] == -2000 && MinMove() >= -2000,
              "[B14] W-44: from case 1 with the In shuttle away, Z1 only goes to TestZ1_Safe (case 1 is pre-descent); the run stops before the socket descent (no target below -2000)");
        // (2b) B3b: Do_Z1 case 1 on a live PCI-1203 Index Z goes to golden's shuttle branch 150 (not the Index-Y path 100); and
        //      with the shuttles really parked at InSHT[0].iLeft / OutSHT[0].iRight it passes 150 -> 110 -> ... -> 530 and W-44 lets it press
        ShAt(0, 0);
        StartAt(1);
        Prod.TestZ1_Safe = -2000;
        int seen150 = 0, seen100 = 0;
        ticks = 0;
        while (fAllMotorHome && ticks < 400 && Moves() < 2) { Tick(); ++ticks; if (fContactForm->Z_Height_Task == 150) ++seen150; if (fContactForm->Z_Height_Task == 100) ++seen100; }
        CHECK(seen150 > 0 && seen100 == 0 && fAllMotorHome && Moves() >= 2 && g_moves[1] == -9000,
              "[B14] B3b: on HT9050 Do_Z1 case 1 takes golden's shuttle branch (150/151, not the Index-Y 100); shuttles at their clear positions -> W-44 passes and the standby descent (-9000) starts");
        MOT[MTestZ1].CardType = "";
        StartAt(1);
        ticks = 0; seen150 = 0; seen100 = 0;
        while (ticks < 40) { Tick(100, true, false); ++ticks; if (fContactForm->Z_Height_Task == 150) ++seen150; if (fContactForm->Z_Height_Task == 100) ++seen100; }
        MOT[MTestZ1].CardType = "PCI1203";
        W906_SetGaliRoute(&g_route);
        CHECK(seen150 == 0, "[B14] B3b: an RS-232 / Panasonic Index Z with USE_OUT_SHT_MOT=0 keeps golden's case 1 -> 100 (no shuttle branch)");
        // (3) within tolerance: the descent starts as golden
        ShAt(100, 0);
        StartAt(530);
        Tick();
        CHECK(fAllMotorHome && Moves() == 1 && g_moves[0] == -9000, "[B14] W-44: shuttles at home (within 100 counts) -> golden's standby descent move (-9000) is issued");
        ShAt(0, 0);
        // (4) B4's run-entry helper: refuses, sends nothing; allows when home
        ShAt(5000, 0);
        g_moves.clear(); g_stops = 0;
        msg0 = W906_ShowMyMessage_Count;
        const bool refused = W906_IndexZShuttlesHomeRefused("test B14 entry");
        CHECK(refused && g_stops == 0 && Moves() == 0 && W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "W-44"),
              "[B14] W-44 run entry (DoTestContactFunction case 1, B4): shuttle away -> refused with the reason, nothing sent (not even a stop)");
        ShAt(0, 0);
        CHECK(!W906_IndexZShuttlesHomeRefused("test B14 entry home"), "[B14] W-44 run entry: shuttles home -> allowed");
        // (5) the pick is NOT guarded by W-44: DoZ1PickFromShuttle case 1 with the In shuttle away runs golden (Z1 to the safe height)
        ShAt(5000, 0);
        StartAt(1);
        Prod.TestZ1_Safe = -2000;
        msg0 = W906_ShowMyMessage_Count;
        g_now += 100; Fresh(); PickZ1();
        CHECK(fAllMotorHome && W906_ShowMyMessage_Count == msg0 && !g_moves.empty() && g_moves[0] == -2000,
              "[B14] W-44 is socket press only: DoZ1PickFromShuttle with the In shuttle away still runs golden (case 1 Z1 -> TestZ1_Safe), no W-44 stop");
        ShAt(0, 0);
        // (6) RS-232 / Panasonic Index Z: W-44 inert
        MOT[MTestZ1].CardType = "";
        ShAt(5000, 0);
        CHECK(!W906_IndexZShuttleHomeStop(530, "t") && !W906_IndexZShuttlesHomeRefused("t") && W906_IndexZShuttlesAtHome(0),
              "[B14] W-44 is inert on an RS-232 / Panasonic Index Z (golden)");
        ShAt(0, 0);
        MOT[MTestZ1].CardType = "PCI1203";

        // (7) P7 on every new state machine: drive ALARM -> ST + golden exit before golden's dispatch, no move
        struct S { const char* name; int kind; };
        const S ss[] = { {"DoZ1PickFromShuttle", 0}, {"DoZ2PickFromShuttle", 1}, {"DoZPlaceToShuttle", 2}, {"DoArm1PlaceToShuttle", 3},
                         {"DoArm2PlaceToShuttle", 4}, {"Do_Z2_AutoGetHeight", 5}, {"DoCalibrateAboveHeightZ1", 6}, {"DoCalibrateAboveHeightZ2", 7} };
        std::string badP7;
        for (std::size_t si = 0; si < sizeof(ss) / sizeof(ss[0]); ++si) {
            StartAt(550);
            fContactForm->iDoArm1PlaceToShuttleTask = 100; fContactForm->iDoArm2PlaceToShuttleTask = 100; fContactForm->iTaskCaliAboveHeight = 100;
            if (ss[si].kind == 4) TestIF.iShuttle_Sel = 1;                    // golden DoArm2 returns at once for shuttle 1
            g_bits |= 0x2ul;
            msg0 = W906_ShowMyMessage_Count;
            const int s0 = g_stops;
            g_now += 100; Fresh();
            bool r = true;
            switch (ss[si].kind) {
                case 0: r = PickZ1(); break;
                case 1: r = PickZ2(); break;
                case 2: r = PlaceZ(); break;
                case 3: r = fContactForm->DoArm1PlaceToShuttle(false); break;
                case 4: r = fContactForm->DoArm2PlaceToShuttle(false); break;
                case 5: r = DoZ2(); break;
                case 6: r = Cali(); break;
                case 7: r = fContactForm->DoCalibrateAboveHeightZ2(false); break;
            }
            TestIF.iShuttle_Sel = 0;
            g_bits &= ~0x2ul;
            W906_IndexZHealthReset();
            if (!(!r && !fAllMotorHome && fContactForm->CarlibrationTask == 1 && g_stops > s0 && Moves() == 0 &&
                  W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "drive error")))
                badP7 += std::string(ss[si].name) + " ";
        }
        std::snprintf(m, sizeof(m), "[B14] P7: M14 ALARM -> each of the 8 new state machines sends ST, takes golden's exit, issues no move, shows the message %s", badP7.c_str());
        CHECK(badP7.empty(), m);

        // (8) Do_Z2 case 1 on HT9050: arm 2 refused, nothing sent
        StartAt(1);
        msg0 = W906_ShowMyMessage_Count;
        g_now += 100; Fresh();
        ret = DoZ2();
        CHECK(!ret && !SystemStart && fContactForm->CarlibrationTask == 1 && Moves() == 0 && g_stops == 0 && Has(W906_ShowMyMessage_LastS1, "Index Z2"),
              "[B14] Do_Z2_AutoGetHeight case 1 on a PCI-1203 Index Z: refused (HT9050 has one index arm), nothing sent");

        // (9) P5 on the pick's torque wait (case 560): no value for 5 s -> ST + golden exit
        StartAt(560);
        g_torqueOk = false;
        unsigned long t0 = g_now;
        int stopsBefore = 0;
        ticks = 0;
        while (fAllMotorHome && ticks < 200) { stopsBefore = g_stops; g_now += 100; Fresh(); COM2->ReadTorque(); ret = PickZ1(); ++ticks; }
        const unsigned long waited = g_now - t0;
        CHECK(!fAllMotorHome && !ret && waited >= 5000ul && waited <= 5300ul && g_stops == stopsBefore + 1 && Has(W906_ShowMyMessage_LastS1, "did not arrive in 5 s"),
              "[B14] P5: DoZ1PickFromShuttle 560 without a torque value -> ST + golden exit after 5 s (golden waits for ever)");
        g_torqueOk = true;

        // (10) P9 on the pick's descent step (case 550): the torque never reaches CONTECT_SHUTTLE_KG -> stop at the floor
        StartAt(500);
        const long surf0 = g_surface; g_surface = -30000;                    // nothing to press on: 6077h reads the air value (0)
        const int pick0 = Tech.iTestZ1ShutlePick; Tech.iTestZ1ShutlePick = -40000;   // golden's encoder check (:2941) never fires
        Prod.TestY1_Front = MOT[MTestY1].Gali_ReadPos();                     // golden's Y1 check (:2984-2990)
        ticks = 0;
        while (fAllMotorHome && ticks < 3000) { g_now += 100; Fresh(); COM2->ReadTorque(); PickZ1(); ++ticks; }
        std::snprintf(m, sizeof(m), "[B14] P9: DoZ1PickFromShuttle 550 walked down without reaching kg -> stopped by the commanded floor (-148.00 mm) with golden's exit; deepest target %ld after %d ticks", MinMove(), ticks);
        CHECK(!fAllMotorHome && Has(W906_ShowMyMessage_LastS1, "floor") && !g_moves.empty() && MinMove() > -14800, m);
        g_surface = surf0; Tech.iTestZ1ShutlePick = pick0;
        Prod.InSHT[0].iLeft = inL0; Prod.OutSHT[0].iRight = outR0; g_inShPos = 0; g_outShPos = 0; MOT[MInShuttle1].Position = 0; MOT[MOutShuttle1].Position = 0;
        W906_SetGaliRoute(0);
#endif
    }

    // ================= [B15] (B4) DoTestContactFunction run entry, single entry, HT9050 place-back, Load Cell =================
    std::printf("-- [B15] (B4) contact SM run entry / single entry / place-back / Load Cell\n");
    {
        char m[300];
        std::string why;
        // pure (both configurations)
        int un = 0;
        for (int mode = 0; mode <= 12; ++mode) if (ModeUntranslated(mode) == (mode == 4 || mode == 5 || mode == 9 || mode == 10 || mode == 11 || mode == 12)) ++un;
        CHECK(un == 13, "[B15] pure: modes 4 / 5 / 9 / 10 / 11 / 12 are 'untranslated' (refused at case 1 on every machine), 0-3 and 8 are not");
        PlaceBackIn pb;
        pb.z1Safe = -2000; pb.z1Pos = -2000; pb.outPresent = true; pb.outPos = -1500; pb.outTarget = -1500;
        const bool pOk = PlaceBackPreconditions(pb, why);
        pb.z1Pos = -2101; const bool pLow = !PlaceBackPreconditions(pb, why) && why.find("safe height") != std::string::npos;
        pb.z1Pos = 0; const bool pHigh = PlaceBackPreconditions(pb, why);
        pb.outPos = -1300; const bool pOut = !PlaceBackPreconditions(pb, why) && why.find("OutSHT[0].iRight") != std::string::npos;
        pb.outPos = -1500; pb.outPresent = false; const bool pMiss = !PlaceBackPreconditions(pb, why) && why.find("cannot be read") != std::string::npos;
        CHECK(pOk && pLow && pHigh && pOut && pMiss, "[B15] pure place-back preconditions: Z1 at / above TestZ1_Safe (-100) and Out shuttle X at OutSHT[0].iRight +-100, unreadable = refused");
        // mode 12 refusal is machine-independent (also an RS-232 row / SIM)
        int msg0 = W906_ShowMyMessage_Count;
        CHECK(W906_ContactModeUntranslatedRefused(12, "t") && Has(W906_ShowMyMessage_LastS1, "TODO E-052: port V912 Task 60") && W906_ShowMyMessage_Count == msg0 + 1 &&
              !W906_ContactModeUntranslatedRefused(1, "t") && !W906_ContactModeUntranslatedRefused(3, "t") && W906_ContactModeUntranslatedRefused(4, "t"),
              "[B15] mode 12 VISUAL_DETECTION_TEST refused with 'TODO E-052: port V912 Task 60' (every machine); 1 / 3 not refused; 4 refused (leaf not translated)");
        // the single entry: the web's radio decides iContactMode (V912 DF_SetContactMode, incl. mode 12) and the facade mirrors it
        const int rbIdx[5] = { 1, 2, 3, 7, 10 };                              // kCt3aRb: AutoHeight, ManualHeight, ContactTest, LoadCell, VisualDetection
        const int rbMode[5] = { 1, 2, 3, 8, 12 };
        int se = 0;
        for (int k = 0; k < 5; ++k) {
            FileRW_Contact_SetContactModeSingleEntry(rbIdx[k]);
            if (iContactMode == rbMode[k]) ++se;
        }
        FileRW_Contact_SetContactModeSingleEntry(1);
        const bool facade = fContactForm->rbAutoHeight->Checked && !fContactForm->rbModeNormal->Checked && !fContactForm->rbContactTest->Checked;
        CHECK(W906_ContactModeSingleEntryHook == &FileRW_Contact_SetContactModeSingleEntry, "[B15] the C-route registered the single entry (W906_ContactModeSingleEntryHook)");
        CHECK(se == 5 && facade, "[B15] single entry: the web radio (g_ct3aRb) -> V912 DF_SetContactMode gives modes 1 / 2 / 3 / 8 / 12, and the facade radios mirror the pick");
        FileRW_Contact_SetContactModeSingleEntry(0);
#ifndef SOFT_SIMULTE
        W906_SetGaliRoute(&g_route);
        MOT[MTestZ1].CardType = "PCI1203";
        UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
        TestIF.iShuttleMode = 1; TestIF.iShuttle_Sel = 0; IniConfig.bIndexArm2SupplyLight = false;
        const int inL0 = Prod.InSHT[0].iLeft, inR0 = Prod.InSHT[0].iRight, outR0 = Prod.OutSHT[0].iRight, safe0 = Prod.TestZ1_Safe;
        Prod.InSHT[0].iLeft = 2000; Prod.InSHT[0].iRight = 3000; Prod.OutSHT[0].iRight = -1500; Prod.TestZ1_Safe = -2000;
        // ---- the place-back step (HT9050) ----
        auto PbStart = [&]() { W906_IndexZPlaceBackReset(); W906_IndexZHealthReset(); g_shMoves.clear(); g_stops = 0; g_moves.clear(); g_inShPos = 2000; g_outShPos = -1500; g_shPresent = true; g_shArrive = true; g_target = -2000; Fresh(); };
        PbStart(); g_target = -5000; Fresh();
        msg0 = W906_ShowMyMessage_Count;
        int r = W906_IndexZPlaceBackPrep("t");
        CHECK(r == -1 && g_shMoves.empty() && g_stops == 1 && W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "safe height") && Has(W906_ShowMyMessage_LastS1, "Q101"),
              "[B15] place-back: Z1 below its safe height -> alarm (Q101 reason) + ST, the In shuttle is NOT moved");
        PbStart(); g_outShPos = -1000;
        r = W906_IndexZPlaceBackPrep("t");
        CHECK(r == -1 && g_shMoves.empty() && g_stops == 1 && Has(W906_ShowMyMessage_LastS1, "Out shuttle X (M17) is not at OutSHT[0].iRight"),
              "[B15] place-back: Out shuttle X not at OutSHT[0].iRight -> alarm + ST, no shuttle move");
        PbStart(); g_shPresent = false;
        r = W906_IndexZPlaceBackPrep("t");
        CHECK(r == -1 && g_shMoves.empty() && g_stops == 1, "[B15] place-back: a shuttle position that cannot be read -> alarm + ST, no shuttle move");
        PbStart();
        msg0 = W906_ShowMyMessage_Count;
        r = W906_IndexZPlaceBackPrep("t");
        const int r2 = W906_IndexZPlaceBackPrep("t");
        CHECK(r == 1 && r2 == 1 && g_shMoves.size() == 1 && g_shMoves[0] == 3000 && g_stops == 0 && W906_ShowMyMessage_Count == msg0,
              "[B15] place-back: preconditions met -> the In shuttle goes to InSHT[0].iRight (3000, not iLeft), confirmed, golden's place-back may run (1), no extra move");
        g_inShPos = 2500;
        r = W906_IndexZPlaceBackPrep("t");
        CHECK(r == -1 && g_stops == 1 && Has(W906_ShowMyMessage_LastS1, "left InSHT[0].iRight"), "[B15] place-back: the In shuttle leaving iRight during the place-back -> alarm + ST");
        PbStart(); g_shArrive = false;
        r = W906_IndexZPlaceBackPrep("t");
        CHECK(r == -1 && g_stops == 1 && Has(W906_ShowMyMessage_LastS1, "did not arrive"), "[B15] place-back: the In shuttle stopping short of iRight (+500) -> alarm + ST");
        MOT[MTestZ1].CardType = "";
        PbStart();
        r = W906_IndexZPlaceBackPrep("t");
        MOT[MTestZ1].CardType = "PCI1203";
        CHECK(r == 1 && g_shMoves.empty() && g_stops == 0, "[B15] place-back: an RS-232 / Panasonic Index Z -> golden (no shuttle move, Index Y carries the IC)");
        // ---- DoTestContactFunction case 1: the run entry (nothing may be commanded when it refuses) ----
        auto Entry = [&](int rb, long inPos, long outPos) {
            FileRW_Contact_SetContactModeSingleEntry(rb);
            g_inShPos = inPos; g_outShPos = outPos; g_shPresent = true;
            W906_IndexZHealthReset();
            fContactForm->CarlibrationTask = 1; SystemStart = true; SoftStop = false; fAllMotorHome = true;
            g_moves.clear(); g_stops = 0; g_limits.clear(); g_shMoves.clear();
            const int c0 = W906_ShowMyMessage_Count;
            g_now += 100; Fresh();
            fContactForm->DoTestContactFunction();
            return W906_ShowMyMessage_Count - c0;
        };
        int n = Entry(10, 2000, -1500);
        CHECK(n == 1 && !SystemStart && iContactMode == 12 && Has(W906_ShowMyMessage_LastS1, "TODO E-052") && Moves() == 0 && g_stops == 0 && g_shMoves.empty(),
              "[B15] DoTestContactFunction case 1: mode 12 (web radio rbVisualDetectionTest) refused, nothing commanded");
        n = Entry(2, 2000, -1500);
        CHECK(n == 1 && !SystemStart && Has(W906_ShowMyMessage_LastS1, "Manual Height stays refused") && Moves() == 0 && g_stops == 0,
              "[B15] DoTestContactFunction case 1 on HT9050: Manual Height refused (P8), nothing commanded");
        n = Entry(7, 2000, -1500);
        CHECK(n == 1 && !SystemStart && Has(W906_ShowMyMessage_LastS1, "Load Cell") && Moves() == 0 && g_stops == 0,
              "[B15] DoTestContactFunction case 1 on HT9050: Load Cell refused (P8), nothing commanded");
        n = Entry(1, 5000, -1500);
        CHECK(n == 1 && !SystemStart && Has(W906_ShowMyMessage_LastS1, "W-44") && Moves() == 0 && g_stops == 0,
              "[B15] DoTestContactFunction case 1 on HT9050: In shuttle away from InSHT[0].iLeft -> W-44 run entry refuses, nothing commanded");
        g_bits |= 0x2ul;
        n = Entry(1, 2000, -1500);
        g_bits &= ~0x2ul;
        CHECK(n == 1 && !SystemStart && Has(W906_ShowMyMessage_LastS1, "drive error") && Moves() == 0 && g_stops == 0,
              "[B15] DoTestContactFunction case 1 on HT9050: M14 ALARM -> refused at the entry, nothing commanded (not even a stop)");
        MOT[MTestZ1].CardType = "";
        n = Entry(4, 0, 0);
        MOT[MTestZ1].CardType = "PCI1203";
        CHECK(n == 1 && !SystemStart && iContactMode == 4 && Has(W906_ShowMyMessage_LastS1, "not translated") && Moves() == 0,
              "[B15] DoTestContactFunction case 1 on an RS-232 row: an untranslated mode (4 Auto Contact Test) is refused instead of stopping half way");
        // ---- Do_LoadCellAutoHigh on HT9050: refused at its case 1 ----
        iContactMode = CONTACT_LoadCell_AUTO_GET_HEIGHT;
        fContactForm->Z_Height_Task = 1; SystemStart = true; fAllMotorHome = true;
        g_moves.clear(); g_stops = 0; g_limits.clear();
        msg0 = W906_ShowMyMessage_Count;
        g_now += 100; Fresh();
        const bool lc = fContactForm->Do_LoadCellAutoHigh(0);
        CHECK(!lc && !SystemStart && W906_ShowMyMessage_Count == msg0 + 1 && Has(W906_ShowMyMessage_LastS1, "Load Cell") && Moves() == 0 && g_stops == 0 && g_limits.empty(),
              "[B15] Do_LoadCellAutoHigh case 1 on HT9050: refused (Load Cell, Steven 1004 08:4x), nothing commanded");
        iContactMode = CONTACT_AUTO_GET_HEIGHT;
        FileRW_Contact_SetContactModeSingleEntry(0);
        Prod.InSHT[0].iLeft = inL0; Prod.InSHT[0].iRight = inR0; Prod.OutSHT[0].iRight = outR0; Prod.TestZ1_Safe = safe0;
        g_inShPos = 0; g_outShPos = 0;
        W906_SetGaliRoute(0);
#endif
    }

    // ================= [B9] census + call-site pins (source) =================
    std::printf("-- [B9] census and call-site pins\n");
    {
        //  AI(W906-E042) 20261004: W906_E042_CENSUS_ROOT is for red proofs ONLY (ctest never sets it).  It points [B9] /
        //  [B10] at a scratch COPY of the tree whose csystem.cpp is mutated (W-152: guard removed / fContact target / gate re-closed), so the real csystem.cpp is never written.
        const char* censusRoot = std::getenv("W906_E042_CENSUS_ROOT");
        const std::string root = (censusRoot && *censusRoot) ? std::string(censusRoot) : std::string(W906_SRC_ROOT);
        if (censusRoot && *censusRoot) std::printf("  NOTE [B9] / [B10] read the tree at W906_E042_CENSUS_ROOT=%s (red proof)\n", censusRoot);
        std::string rootW = root; for (std::size_t i = 0; i < rootW.size(); ++i) if (rootW[i] == '/') rootW[i] = '\\';
        std::vector<std::string> files;
        Walk(rootW, files);
        const char* fns[] = { "Do_Z1_AutoGetHeight(", "Do_Z2_AutoGetHeight(", "DoTestContactFunction(", "Do_LoadCellAutoHigh(",
                              "DoZ1PickFromShuttle(", "DoZ2PickFromShuttle(", "DoZPlaceToShuttle(", "DoArm1PlaceToShuttle(",
                              "DoArm2PlaceToShuttle(", "CheckIndexArmStatus(", "Do_ContactTest_32Site(",
                              "DoCalibrateAboveHeightZ1(", "DoCalibrateAboveHeightZ2(", "ATC_SwitchTjSignal(",   // AI(W906-E042) 20261005: B3 defines them too
                              "SetContactMode(" };   // the plan's s1.1 rows; SetContactMode since B2 (AI(W906-E042) 20261004) -- keep it LAST (kScm)
        const std::size_t kScm = sizeof(fns) / sizeof(fns[0]) - 1;          // index of "SetContactMode(" -- counted on its own ([B13])
        int live = 0, baselineCalls = 0, liveScm = 0;
        std::string where, baselineWhere, whereScm;
        for (std::size_t f = 0; f < files.size(); ++f) {
            std::vector<std::string> v;
            if (!ReadLines(files[f], v)) continue;
            const std::vector<std::string> lc = LiveCode(v);
            const bool isTorqueModule = files[f].find("IndexZTorque1203.cpp") != std::string::npos || files[f].find("IndexZTorque1203.h") != std::string::npos;
            for (std::size_t i = 0; i < lc.size() && !isTorqueModule; ++i)       // Q90 = A: the P3 baseline has NO call site
                if (lc[i].find("W906_IndexZTorqueBaselineReset(") != std::string::npos) { ++baselineCalls; baselineWhere += files[f] + ":" + std::to_string(i + 1) + " "; }
            if (files[f].find("fContact_AutoHeight.cpp") != std::string::npos) continue;   // the body itself
            if (files[f].find("fContact_ContactSM.cpp") != std::string::npos) continue;    // B2: SetContactMode's body (B4: DoTestContactFunction)
            if (files[f].find("fContact_IndexPickPlace.cpp") != std::string::npos) continue;   // B3: the pick / place bodies
            const bool isHeader = files[f].find("forms\\fContact.h") != std::string::npos;
            for (std::size_t i = 0; i < lc.size(); ++i)
                for (std::size_t k = 0; k < sizeof(fns) / sizeof(fns[0]); ++k) {
                    //  AI(W906-E042) 20261004 (B2): whole identifiers only, every occurrence -- the C-route's own static
                    //  DF_SetContactMode (FileRW/DeviceForm_File.gen.inc) is not TfContact::SetContactMode.
                    for (std::size_t p = lc[i].find(fns[k]); p != std::string::npos; p = lc[i].find(fns[k], p + 1)) {
                        if (p > 0 && IdentChar(lc[i][p - 1])) continue;
                        if (isHeader && lc[i].find(";") != std::string::npos && lc[i].find("->") == std::string::npos) continue;   // the declarations
                        if (k == kScm) { ++liveScm; whereScm += files[f] + ":" + std::to_string(i + 1) + " "; }
                        else           { ++live;    where    += files[f] + ":" + std::to_string(i + 1) + " "; }
                    }
                }
        }
        char m[400];
        std::snprintf(m, sizeof(m), "[B9] census: exactly 1 live caller of the Auto Height / contact state machines outside the new TU, MainProc's in csystem.cpp (W-152) (%d file(s) scanned) %s",
                      (int)files.size(), where.c_str());
        CHECK(files.size() > 300 && live == 1 && where.find(rootW + "\\csystem.cpp:") == 0, m);   // AI(W906-W152-GATE) 20261007 laptop: was live == 0 (MainProc's call #if 0) -- Jimmy #143 = A
        std::snprintf(m, sizeof(m), "[B13] census: 0 live callers of TfContact::SetContactMode outside forms/fContact_ContactSM.cpp (replaces the linker interlock GATE (X-10); DF_SetContactMode is not it) %s",
                      whereScm.c_str());
        CHECK(files.size() > 300 && liveScm == 0, m);
        std::snprintf(m, sizeof(m), "[B9] Q90 = A: W906_IndexZTorqueBaselineReset has no call site (the baseline stays default-off and unwired) %s", baselineWhere.c_str());
        CHECK(baselineCalls == 0, m);
        std::vector<std::string> cs;
        const bool r1 = ReadLines(root + "/csystem.cpp", cs);
        int callLine = -1, shimCalls = 0, arm = -1;   // AI(W906-W152-GATE) 20261007 laptop: the ONE live caller = golden :17679 in MainProc's contact-mode arm (golden :17668), guarded, on fContactForm
        for (std::size_t i = 0; i < cs.size(); ++i) { const std::string c = CodeOf(cs[i]); if (c.find("fContact->DoTestContactFunction(") != std::string::npos) ++shimCalls; if (callLine < 0 && c.find("fContactForm->DoTestContactFunction(") != std::string::npos) callLine = (int)i; if (callLine < 0 && Norm(cs[i]).compare(0, 8, "else if(") == 0) arm = (int)i; }
        const std::vector<std::string> csl = LiveCode(cs);
        CHECK(r1 && callLine > 0 && shimCalls == 0 && Norm(cs[(std::size_t)callLine]) == "if(W906_IndexZLive1203()) fContactForm->DoTestContactFunction();" && !csl[(std::size_t)callLine].empty() && where == rootW + "\\csystem.cpp:" + std::to_string(callLine + 1) + " " && arm > 0 && callLine - arm < 30 && CodeOf(cs[(std::size_t)arm]).find("W906_FormShowing(\"fContact\"") != std::string::npos && CodeOf(cs[(std::size_t)arm]).find("iContactMode!=CONTACT_NORMAL") != std::string::npos,
              "[B9] csystem.cpp: MainProc's contact arm (golden :17668) makes golden :17679's call ONLY as `if(W906_IndexZLive1203()) fContactForm->DoTestContactFunction();` (HT9050 PCI1203 Index Z1; SIM / other machines: no call); no fContact->DoTestContactFunction anywhere");
        std::vector<std::string> ah;
        const bool r2 = ReadLines(root + "/forms/fContact_AutoHeight.cpp", ah);
        int sw = -1, c1 = -1, w555 = -1, w7170 = -1, st560 = -1;
        for (std::size_t i = 0; i < ah.size(); ++i) {
            const std::string c = CodeOf(ah[i]);
            if (c.find("W906_IndexZDriveFaultStop(\"Do_Z1_AutoGetHeight\")") != std::string::npos && c.find("switch(Task)") != std::string::npos) sw = (int)i;
            if (c.find("case 1: if(W906_IndexZRunRefused(iContactMode, 0,") != std::string::npos) c1 = (int)i;
            if (c.find("W906_IndexZTorqueWaitStop(0, fMain->edTorue0->Text==\"\", \"Do_Z1_AutoGetHeight 555\")") != std::string::npos &&
                c.find("if(fMain->edTorue0->Text==\"\")") != std::string::npos) w555 = (int)i;
            if (c.find("\"Do_Z1_AutoGetHeight 7170\"") != std::string::npos && c.find("if(fMain->edTorue0->Text!=\"\")") != std::string::npos) w7170 = (int)i;
            if (c.find("W906_IndexZCommandFloorStop(Pos, fIndexDownPos,") != std::string::npos && c.find("if(INDEX_MOTION_CARD==0)") != std::string::npos) st560 = (int)i;
        }
        CHECK(r2 && sw > 0 && c1 > sw && w555 > c1 && st560 > w555 && w7170 > st560,
              "[B9] P7 (switch line) / P4+P8 (case 1) / P5 (555, 7170) / P9 (560 step) are LIVE code on golden's own lines, before any trailing //");
        int exits = 0;
        for (std::size_t i = 0; i < ah.size(); ++i) {
            const std::string c = CodeOf(ah[i]);
            if ((c.find("W906_IndexZDriveFaultStop(") != std::string::npos || c.find("W906_IndexZTorqueWaitStop(") != std::string::npos ||
                 c.find("W906_IndexZCommandFloorStop(") != std::string::npos) &&
                c.find("{ fAllMotorHome=false; CarlibrationTask=1; return false; }") != std::string::npos) ++exits;
        }
        CHECK(exits == 10, "[B9] every P5 / P7 / P9 trip takes golden case 536's exit (10 sites since B3: Do_Z1 4, Do_Z2 4, DoCalibrateAboveHeightZ1 / Z2 1 each)");
        int bad = 0;
        for (std::size_t i = 0; i < ah.size(); ++i) { const std::string c = CodeOf(ah[i]); if (c.find("CheckAndReadIniData") != std::string::npos) ++bad; }
        CHECK(bad == 0, "[B9] the new TU has no write-back ini path (CheckAndReadIniData*)");
        //  AI(W906-W152-MODE) 20261007 laptop: [B9] MODE -- golden 0618 csystem.cpp:17687-17691, the same arm right after the call pinned above:
        //  `if(fAllMotorHome==false) { iContactMode=CONTACT_NORMAL; fContact->rbModeNormal->Checked=true; }`.  The port makes :17690 ONLY as
        //  `if(W906_IndexZLive1203() && W906_ContactModeNormalHook) W906_ContactModeNormalHook();`, live, right after `iContactMode=CONTACT_NORMAL;`
        //  inside that `if(fAllMotorHome==false)`, and no rbModeNormal code (live or #if 0) is left in csystem.cpp: the radio the page shows is
        //  the C-route stand-in (FileRW/DeviceForm_File.cpp EOF W906_ContactModeNormalToPage), not fContact's (the shim has none) or fContactForm's.
        {
            int hookLine = -1, hookCalls = 0, rbCode = 0;
            for (std::size_t i = 0; i < cs.size(); ++i) {
                const std::string c = CodeOf(cs[i]);
                if (c.find("W906_ContactModeNormalHook(") != std::string::npos) { ++hookCalls; hookLine = (int)i; }
                if (c.find("rbModeNormal") != std::string::npos) ++rbCode;
            }
            std::vector<std::string> before, after;   // the nearest code lines around the call (comment-only / blank lines skipped)
            for (int i = hookLine - 1; hookLine > 0 && i >= 0 && before.size() < 3; --i) { const std::string n = Norm(cs[(std::size_t)i]); if (!n.empty()) before.push_back(n); }
            for (int i = hookLine + 1; hookLine > 0 && i < (int)cs.size() && after.empty(); ++i) { const std::string n = Norm(cs[(std::size_t)i]); if (!n.empty()) after.push_back(n); }
            const bool shape = callLine > 0 && hookLine > callLine && hookLine - callLine < 40 && before.size() == 3 && after.size() == 1 &&
                               before[0] == "iContactMode=CONTACT_NORMAL;" && before[1] == "{" && before[2] == "if(fAllMotorHome==false)" && after[0] == "}";
            std::snprintf(m, sizeof(m), "[B9] MODE: csystem.cpp makes golden :17690 ONLY as `if(W906_IndexZLive1203() && W906_ContactModeNormalHook) W906_ContactModeNormalHook();`, live, right after iContactMode=CONTACT_NORMAL in the arm's if(fAllMotorHome==false) (:%d, %d call(s)); no rbModeNormal code left (%d)",
                          hookLine + 1, hookCalls, rbCode);
            CHECK(r1 && hookCalls == 1 && hookLine > 0 && Norm(cs[(std::size_t)hookLine]) == "if(W906_IndexZLive1203() && W906_ContactModeNormalHook) W906_ContactModeNormalHook();" &&
                  !csl[(std::size_t)hookLine].empty() && shape && rbCode == 0, m);
        }
        //  AI(W906-W152-MODE) 20261007 laptop: [B9] ORDER -- b4639372 "this lands AFTER W-152 (C)": the live gate line pinned above is allowed only
        //  while the single entry FileRW_Contact_SetContactModeSingleEntry (FileRW/DeviceForm_File.cpp) calls W906_ContactRecipeToFacade and that
        //  file defines it with the recipe -> facade copy (W-152 (C)); without it the run reads empty facade edits (fDropPos 0.0,
        //  forms/fContact_AutoHeight.cpp:1290-1294).  So the gate line cannot be merged without (C).
        {
            std::vector<std::string> dfc;
            const bool rd = ReadLines(root + "/FileRW/DeviceForm_File.cpp", dfc);
            const std::vector<std::string> dl = LiveCode(dfc);
            int seBeg = -1, seEnd = -1, cDef = -1, cEnd = -1, cCalls = 0, cCopy = 0;
            for (std::size_t i = 0; i < dl.size(); ++i) {
                const std::string n = Norm(dl[i]);
                if (seBeg < 0 && n == "void FileRW_Contact_SetContactModeSingleEntry(int rbForTest)") seBeg = (int)i;
                else if (seBeg >= 0 && seEnd < 0 && dfc[i].compare(0, 1, "}") == 0) seEnd = (int)i;
                if (cDef < 0 && n == "void W906_ContactRecipeToFacade(TfContact* F)") cDef = (int)i;
                else if (cDef >= 0 && cEnd < 0 && dfc[i].compare(0, 1, "}") == 0) cEnd = (int)i;
            }
            for (int i = seBeg; seBeg >= 0 && seEnd > seBeg && i <= seEnd; ++i)
                for (std::size_t p = dl[(std::size_t)i].find("W906_ContactRecipeToFacade(F)"); p != std::string::npos; p = dl[(std::size_t)i].find("W906_ContactRecipeToFacade(F)", p + 1)) ++cCalls;
            for (int i = cDef; cDef >= 0 && cEnd > cDef && i <= cEnd; ++i)
                if (dl[(std::size_t)i].find("kW152Recipe") != std::string::npos && dl[(std::size_t)i].find("(F->*") != std::string::npos && dl[(std::size_t)i].find("EL<") != std::string::npos) ++cCopy;
            const bool gateLive = callLine > 0 && !csl[(std::size_t)callLine].empty();
            std::snprintf(m, sizeof(m), "[B9] ORDER: the live gate line (csystem.cpp:%d) only with W-152 (C): FileRW_Contact_SetContactModeSingleEntry (:%d-%d) calls W906_ContactRecipeToFacade (%d live call(s)), defined in FileRW/DeviceForm_File.cpp (:%d) with the recipe copy (%d line(s))",
                          callLine + 1, seBeg + 1, seEnd + 1, cCalls, cDef + 1, cCopy);
            CHECK(rd && (!gateLive || (seBeg > 0 && seEnd > seBeg && seEnd - seBeg < 40 && cCalls == 1 && cDef > 0 && cEnd > cDef && cCopy >= 1)), m);   // gate live => (C)
        }

        // ================= [B10] the golden span is contiguous =================
        int offset = 0; bool haveOffset = false;
        for (std::size_t i = 0; i < ah.size() && i < 80; ++i) {
            const std::size_t p = ah[i].find("port line = golden line + (");
            if (p != std::string::npos) { offset = std::atoi(ah[i].c_str() + p + 27); haveOffset = true; break; }
        }
        int miss = 0;
        const int nA = (int)(sizeof(kAnchors) / sizeof(kAnchors[0]));
        for (int a = 0; a < nA; ++a) {
            const int pl = kAnchors[a][0] + offset;                          // 1-based port line
            const std::string want = "case " + std::to_string(kAnchors[a][1]) + ":";
            if (pl < 1 || pl > (int)ah.size() || CodeOf(ah[(std::size_t)pl - 1]).find(want) == std::string::npos) ++miss;
        }
        std::snprintf(m, sizeof(m), "[B10] all %d golden `case N:` labels of Do_Z1_AutoGetHeight sit at golden line + (%d)", nA, offset);
        CHECK(haveOffset && nA == 108 && miss == 0, m);

        // ================= [B13] (B2) SetContactMode source pins =================
        std::vector<std::string> sm;
        const bool r3 = ReadLines(root + "/forms/fContact_ContactSM.cpp", sm);
        int soff = 0; bool haveSoff = false;
        for (std::size_t i = 0; i < sm.size() && i < 80; ++i) {
            const std::size_t p = sm[i].find("SetContactMode span: port line = golden line + (");
            if (p != std::string::npos) { soff = std::atoi(sm[i].c_str() + p + 48); haveSoff = true; break; }   // 48 = strlen of the needle
        }
        const int g0 = 15341, g1 = 15434;
        int diff = 0, extra = 0, nG = 0;
        std::string firstDiff;
        std::vector<char> isCode((std::size_t)(g1 - g0 + 1), 0);
        for (std::size_t a = 0; a < sizeof(kScmGolden) / sizeof(kScmGolden[0]); ++a) {
            ++nG;
            const int gl = kScmGolden[a].line;
            isCode[(std::size_t)(gl - g0)] = 1;
            const int pl = gl + soff;
            if (pl < 1 || pl > (int)sm.size() || Norm(sm[(std::size_t)pl - 1]) != kScmGolden[a].code) {
                if (firstDiff.empty()) firstDiff = "golden :" + std::to_string(gl) + " port :" + std::to_string(pl);
                ++diff;
            }
        }
        for (int gl = g0; gl <= g1; ++gl) {
            const int pl = gl + soff;
            if (!isCode[(std::size_t)(gl - g0)] && pl >= 1 && pl <= (int)sm.size() && !Norm(sm[(std::size_t)pl - 1]).empty()) {
                if (firstDiff.empty()) firstDiff = "extra code at port :" + std::to_string(pl);
                ++extra;
            }
        }
        std::snprintf(m, sizeof(m), "[B13] forms/fContact_ContactSM.cpp: SetContactMode = golden 0618 :15341-15434 code line for code line at golden line + (%d), %d code lines, no extra code %s",
                      soff, nG, firstDiff.c_str());
        CHECK(r3 && haveSoff && nG == 92 && diff == 0 && extra == 0, m);
        const std::vector<std::string> smLive = LiveCode(sm);
        const char* forbid[] = { "MOT[", "SW[", "Sen[", "Cylinder[", "Gali_", "ADAM_", "COM2", "WriteIniData", "CheckAndReadIniData", "INIFile",
                                 "ShowMyMessage", "SystemStart", "fAllMotorHome", "CarlibrationTask", "SoftStop", "W906_" };
        int hits = 0, defs = 0;
        std::string hitWhere;
        for (int gl = g0; gl <= g1; ++gl) {
            const int pl = gl + soff;
            if (pl < 1 || pl > (int)smLive.size()) continue;
            for (std::size_t q = 0; q < sizeof(forbid) / sizeof(forbid[0]); ++q)
                if (smLive[(std::size_t)pl - 1].find(forbid[q]) != std::string::npos) { ++hits; hitWhere += std::string(forbid[q]) + "@" + std::to_string(pl) + " "; }
        }
        for (std::size_t i = 0; i < smLive.size(); ++i) if (smLive[i].find("TfContact::SetContactMode(") != std::string::npos) ++defs;
        std::snprintf(m, sizeof(m), "[B13] SetContactMode stays a UI-state switch: no motion / IO / ini / message / start / home / W906_ token in its body %s", hitWhere.c_str());
        CHECK(r3 && hits == 0, m);
        CHECK(defs == 1, "[B13] TfContact::SetContactMode is defined exactly once, in forms/fContact_ContactSM.cpp");

        // ================= [B14] (B3) source pins =================
        std::vector<std::string> pp;
        const bool r4 = ReadLines(root + "/forms/fContact_IndexPickPlace.cpp", pp);
        auto OffsetOf = [&](const std::vector<std::string>& v, const std::string& needle, int& off) -> bool {
            for (std::size_t i = 0; i < v.size(); ++i) {   // B4: the ContactSM markers sit after the SetContactMode span
                const std::size_t p = v[i].find(needle);
                if (p != std::string::npos) { off = std::atoi(v[i].c_str() + p + needle.size()); return true; }
            }
            return false;
        };
        int oA = 0, oB = 0, oC = 0, oD = 0, oE = 0;
        const bool haveO = OffsetOf(pp, "IndexPickPlace span A: port line = golden line + (", oA) && OffsetOf(pp, "IndexPickPlace span B: port line = golden line + (", oB) &&
                           OffsetOf(pp, "IndexPickPlace span C: port line = golden line + (", oC);
        bool haveO2 = false;
        for (std::size_t i = 0; i < ah.size(); ++i) {
            const std::string nD = "CalibrateAboveZ1 span: port line = golden line + (", nE = "ATC_SwitchTjSignal span: port line = golden line + (";
            std::size_t p = ah[i].find(nD);
            if (p != std::string::npos) { oD = std::atoi(ah[i].c_str() + p + nD.size()); haveO2 = true; }
            p = ah[i].find(nE);
            if (p != std::string::npos) oE = std::atoi(ah[i].c_str() + p + nE.size());
        }
        int missB3 = 0; std::string firstMiss;
        const int nB3 = (int)(sizeof(kB3Anchors) / sizeof(kB3Anchors[0]));
        for (int a = 0; a < nB3; ++a) {
            const int g = kB3Anchors[a][0], key = kB3Anchors[a][2];
            const std::vector<std::string>& v = (key <= 1) ? ah : pp;
            const int off = key == 0 ? offset : key == 1 ? oD : key == 3 ? oA : key == 4 ? oB : oC;
            const int pl = g + off;
            const std::string want = "case " + std::to_string(kB3Anchors[a][1]) + ":";
            if (pl < 1 || pl > (int)v.size() || CodeOf(v[(std::size_t)pl - 1]).find(want) == std::string::npos) { if (firstMiss.empty()) firstMiss = "golden :" + std::to_string(g); ++missB3; }
        }
        std::snprintf(m, sizeof(m), "[B14] all %d golden `case N:` labels of the B3 functions sit at golden line + their span offset (A %d, B %d, C %d, calibrate %d, Do_Z2 %d) %s",
                      nB3, oA, oB, oC, oD, offset, firstMiss.c_str());
        CHECK(r4 && haveO && haveO2 && nB3 == 195 && missB3 == 0, m);
        std::snprintf(m, sizeof(m), "[B14] ATC_SwitchTjSignal is translated at golden :18565 + (%d)", oE);
        CHECK(oE != 0 && (int)ah.size() >= 18565 + oE && CodeOf(ah[(std::size_t)(18565 + oE - 1)]).find("void TfContact::ATC_SwitchTjSignal(int iIndexArm, bool bSnedArmDown)") != std::string::npos, m);
        //  P7 on each new SM's switch line, P5 / P9 on the pick lines, all live code before any trailing //
        const char* p7n[8] = { "DoZ1PickFromShuttle", "DoZ2PickFromShuttle", "DoZPlaceToShuttle", "DoArm1PlaceToShuttle", "DoArm2PlaceToShuttle", "Do_Z2_AutoGetHeight", "DoCalibrateAboveHeightZ1", "DoCalibrateAboveHeightZ2" };
        int p7ok = 0;
        for (int k = 0; k < 8; ++k) {
            const std::vector<std::string>& v = (k >= 5) ? ah : pp;
            const std::string needle = std::string("if(W906_IndexZDriveFaultStop(\"") + p7n[k] + "\")) { fAllMotorHome=false; CarlibrationTask=1; return false; }";
            for (std::size_t i = 0; i < v.size(); ++i) { const std::string c = CodeOf(v[i]); if (c.find(needle) != std::string::npos && c.find("switch(Task)") != std::string::npos) { ++p7ok; break; } }
        }
        CHECK(p7ok == 8, "[B14] P7 is live code on the switch(Task) line of all 8 new state machines (every tick, before golden's dispatch)");
        int p5 = 0, p9 = 0, w44pp = 0, w44ah = 0, ini = 0;
        for (std::size_t i = 0; i < pp.size(); ++i) {
            const std::string c = CodeOf(pp[i]);
            if (c.find("W906_IndexZTorqueWaitStop(0, fMain->edTorue0->Text==\"\", \"DoZ1PickFromShuttle 560\")") != std::string::npos && c.find("if(fMain->edTorue0->Text==\"\")") != std::string::npos) ++p5;
            if (c.find("W906_IndexZTorqueWaitStop(1, fMain->edTorue1->Text==\"\", \"DoZ2PickFromShuttle 560\")") != std::string::npos && c.find("if(fMain->edTorue1->Text==\"\")") != std::string::npos) ++p5;
            if (c.find("W906_IndexZCommandFloorStop(Pos, fIndexDownPos, \"DoZ1PickFromShuttle 550\")") != std::string::npos && c.find("if(MOT[MTestZ1].Gali_MotMoveNoWait(Pos,") != std::string::npos) ++p9;
            if (c.find("W906_IndexZCommandFloorStop(Pos, fIndexDownPos, \"DoZ2PickFromShuttle 550\")") != std::string::npos && c.find("if(MOT[MTestZ2].Gali_MotMoveNoWait(Pos,") != std::string::npos) ++p9;
            if (c.find("W906_IndexZShuttleHome") != std::string::npos || c.find("W906_IndexZShuttlesHome") != std::string::npos) ++w44pp;
            if (c.find("CheckAndReadIniData") != std::string::npos) ++ini;
        }
        for (std::size_t i = 0; i < ah.size(); ++i) {
            const std::string c = CodeOf(ah[i]);
            if (c.find("if(W906_IndexZShuttleHomeStop(Task, \"Do_Z1_AutoGetHeight\")) { fAllMotorHome=false; CarlibrationTask=1; return false; }") != std::string::npos && c.find("switch(Task)") != std::string::npos) ++w44ah;
            if (c.find("if(W906_IndexZShuttleHomeStop(Task, \"Do_Z2_AutoGetHeight\")) { fAllMotorHome=false; CarlibrationTask=1; return false; }") != std::string::npos && c.find("switch(Task)") != std::string::npos) ++w44ah;
            if (c.find("CheckAndReadIniData") != std::string::npos) ++ini;
        }
        CHECK(p5 == 2 && p9 == 2, "[B14] P5 (pick 560 waits) and P9 (pick 550 steps) are live code on golden's own lines, Z1 and Z2");
        CHECK(w44ah == 2 && w44pp == 0, "[B14] W-44 sits on the socket-press SMs only (Do_Z1 / Do_Z2 switch lines); the pick / place TU has no W-44 call (ST01-M 1005 03:4x)");
        CHECK(ini == 0, "[B14] neither TU has a write-back ini path (CheckAndReadIniData*)");
        //  #20 RULE, V912 better: DoArm1PlaceToShuttle case 300 starts hContactDeley before case 400 waits on it (D1)
        bool d1 = false;
        const int d1pl = 19356 + oC;
        if (d1pl >= 1 && d1pl <= (int)pp.size()) {
            const std::string& l = pp[(std::size_t)d1pl - 1];
            d1 = CodeOf(l).find("hContactDeley.SetSecAndOn(2); Task=400;") != std::string::npos &&
                 l.find("[906]") != std::string::npos && l.find("[V912]") != std::string::npos && l.find(":19648") != std::string::npos && l.find("[why]") != std::string::npos;
        }
        CHECK(d1, "[B14] #20 D1: DoArm1PlaceToShuttle (golden :19356) starts hContactDeley before the case-400 wait, with the three-part comment ([906] / [V912] :19648 / [why])");

        // ================= [B15] (B4) source pins =================
        std::vector<std::string> cs2;
        const bool r5 = ReadLines(root + "/forms/fContact_ContactSM.cpp", cs2);
        int oT = 0, oL = 0;
        const bool haveT = OffsetOf(cs2, "DoTestContactFunction span: port line = golden line + (", oT) && OffsetOf(cs2, "LoadCell span: port line = golden line + (", oL);
        int missB4 = 0; std::string firstMiss4;
        const int nB4 = (int)(sizeof(kB4Anchors) / sizeof(kB4Anchors[0]));
        for (int a = 0; a < nB4; ++a) {
            const int pl = kB4Anchors[a][0] + (kB4Anchors[a][2] == 6 ? oT : oL);
            const std::string want = "case " + std::to_string(kB4Anchors[a][1]) + ":";
            if (pl < 1 || pl > (int)cs2.size() || CodeOf(cs2[(std::size_t)pl - 1]).find(want) == std::string::npos) { if (firstMiss4.empty()) firstMiss4 = "golden :" + std::to_string(kB4Anchors[a][0]); ++missB4; }
        }
        std::snprintf(m, sizeof(m), "[B15] all %d golden `case N:` labels of DoTestContactFunction / Do_LoadCellAutoHigh sit at golden line + their offset (%d / %d) %s", nB4, oT, oL, firstMiss4.c_str());
        CHECK(r5 && haveT && nB4 == 97 && missB4 == 0, m);
        auto LineAt = [&](int g, int off) -> std::string { const int pl = g + off; return (pl >= 1 && pl <= (int)cs2.size()) ? CodeOf(cs2[(std::size_t)pl - 1]) : std::string(); };
        const std::string e1 = LineAt(11819, oT), se1 = LineAt(11790, oT), sw1 = LineAt(11786, oT), pb1 = LineAt(13350, oT), lsw = LineAt(17210, oL), lc1 = LineAt(17212, oL);
        CHECK(e1.find("if(W906_ContactModeUntranslatedRefused(iContactMode, \"DoTestContactFunction 1\")) { SystemStart=false; return; }") != std::string::npos &&
              e1.find("if(W906_IndexZRunRefused(iContactMode, 0, \"DoTestContactFunction 1\")) { SystemStart=false; return; }") != std::string::npos &&
              e1.find("if(W906_IndexZShuttlesHomeRefused(\"DoTestContactFunction 1\")) { SystemStart=false; return; }") != std::string::npos &&
              e1.find("W906_IndexZPlaceBackReset();") != std::string::npos && e1.find("InitialTestHeadMotorTask();") != std::string::npos &&
              e1.find("W906_ContactModeUntranslatedRefused") < e1.find("InitialTestHeadMotorTask();"),
              "[B15] the run entry (untranslated modes / P4+P8+P7 / W-44) is live code on golden :11819, before InitialTestHeadMotorTask and every output after it");
        CHECK(se1.find("if(W906_ContactModeSingleEntryHook) W906_ContactModeSingleEntryHook(-1); else SetContactMode();") != std::string::npos &&
              cs2[(std::size_t)(11790 + oT - 1)].find("[906]") != std::string::npos && cs2[(std::size_t)(11790 + oT - 1)].find("[V912]") != std::string::npos,
              "[B15] golden :11790 SetContactMode() -> the single entry, with the 906 / V912 notes on the line");
        CHECK(sw1.find("if(Task!=1 && W906_IndexZDriveFaultStop(\"DoTestContactFunction\")) { fAllMotorHome=false; CarlibrationTask=1; return; }") != std::string::npos && sw1.find("switch(Task)") != std::string::npos &&
              lsw.find("if(W906_IndexZDriveFaultStop(\"Do_LoadCellAutoHigh\"))") != std::string::npos && lsw.find("switch(Task)") != std::string::npos &&
              lc1.find("case 1: if(W906_IndexZRunRefused(iContactMode, iIndex, \"Do_LoadCellAutoHigh 1\"))") != std::string::npos,
              "[B15] P7 on the DoTestContactFunction / Do_LoadCellAutoHigh switch lines; Load Cell case-1 refusal live");
        CHECK(pb1.find("const int pb=W906_IndexZPlaceBackPrep(\"DoTestContactFunction 900\"); if(pb<0) { fAllMotorHome=false; CarlibrationTask=1; return; } if(pb==0) break;") != std::string::npos &&
              pb1.find("if(DoZPlaceToShuttle())") != std::string::npos && pb1.find("W906_IndexZPlaceBackPrep") < pb1.find("if(DoZPlaceToShuttle())") &&
              cs2[(std::size_t)(13350 + oT - 1)].find("Steven 1005 09:2x Q101: HT9050 has no Index Y; the In shuttle returns to iRight before place-back") != std::string::npos,
              "[B15] the place-back step sits on golden :13350 BEFORE DoZPlaceToShuttle(), live code, with the Q101 reason on the line");
        int seCallers = 0; std::string seWhere;
        for (std::size_t f = 0; f < files.size(); ++f) {
            if (files[f].find("fContact_ContactSM.cpp") != std::string::npos || files[f].find("DeviceForm_File.cpp") != std::string::npos || files[f].find("fContact_AutoHeight.h") != std::string::npos) continue;
            std::vector<std::string> v;
            if (!ReadLines(files[f], v)) continue;
            const std::vector<std::string> lc = LiveCode(v);
            for (std::size_t i = 0; i < lc.size(); ++i)
                if (lc[i].find("FileRW_Contact_SetContactModeSingleEntry(") != std::string::npos || lc[i].find("W906_ContactModeSingleEntryHook(") != std::string::npos) { ++seCallers; seWhere += files[f] + ":" + std::to_string(i + 1) + " "; }
        }
        std::snprintf(m, sizeof(m), "[B15] census: the single entry has no live caller outside the E-042 contact TU (and its definition / declaration) %s", seWhere.c_str());
        CHECK(seCallers == 0, m);
    }

    // ================= [B16] (W-152 urgent, laptop) DoIndecxCHECkFunction: golden reset translated, SM body still a leaf =================
    std::printf("-- [B16] DoIndecxCHECkFunction reset (case 218 / 2180) does not stop the run\n");
    {
        extern int iIndexTask;
        fContactForm->CarlibrationTask = 218; SystemStart = true; SoftStop = false;
        iIndexTask = 7;
        const int c0 = W906_ShowMyMessage_Count;
        const bool r = fContactForm->DoIndecxCHECkFunction(true);
        CHECK(!r && iIndexTask == 1 && SystemStart && fContactForm->CarlibrationTask == 218 && W906_ShowMyMessage_Count == c0,
              "[B16] reset (golden cContact.cpp:19075-19079): iIndexTask=1, returns false, no message, the run keeps going");
        const bool r2 = fContactForm->DoIndecxCHECkFunction(false);
        CHECK(!r2 && !SystemStart && fContactForm->CarlibrationTask == 1 && W906_ShowMyMessage_Count == c0 + 1 &&
              Has(W906_ShowMyMessage_LastS1, "DoIndecxCHECkFunction"),
              "[B16] the index-check SM body (golden :19081-19224, S-29) is still an E-042 leaf: message + run stopped");
        std::vector<std::string> sm;
        const bool ok = ReadLines(std::string(W906_SRC_ROOT) + "/forms/fContact_ContactSM.cpp", sm);
        int a = -1, b = -1, resets = 0, leaves = 0;
        for (std::size_t i = 0; ok && i < sm.size(); ++i) {
            const std::string c = CodeOf(sm[i]);
            if (a < 0 && c.find("case 218:") != std::string::npos) a = (int)i;
            else if (a >= 0 && b < 0 && c.find("case 2190:") != std::string::npos) b = (int)i;
        }
        for (int i = a; a >= 0 && b > a && i < b; ++i) {
            const std::string c = CodeOf(sm[(std::size_t)i]);
            if (c.find("DoIndecxCHECkFunction(true);") != std::string::npos) ++resets;
            if (c.find("E042Leaf(") != std::string::npos) ++leaves;
        }
        CHECK(ok && a >= 0 && b > a && resets == 2 && leaves == 0,
              "[B16] source: case 218 / 2180 call golden's reset DoIndecxCHECkFunction(true), no E042Leaf between case 218 and case 2190");
    }

    COM2->Comm1->StopComm();
    W906_Pci1203TorqueReadHook = 0;
    W906_Pci1203IndexZHealthHook = 0;
    W906_Pci1203TorqueLimitHook = 0;
    W906_IndexZNowMsHook = 0;
    W906_IndexZShuttlePosHook = 0;
    W906_IndexZShuttleMoveHook = 0;
    INIFileGeneral = g_oldIni;
    delete g_tmpIni;
    ::DeleteFileA(g_ini.c_str());
    ::RemoveDirectoryA(g_dir.c_str());
    std::printf("[indexz_autoheight_1203] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
