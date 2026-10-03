// =============================================================================
//  EtherCAT/Pci1203MotorRoute.cpp -- see the header for what this connects and why.
//
//  AI(W906-ECAT-ROUTE) 20260929: new file, wb_serve only (CMakeLists.txt, next to
//  Pci1203IoRoute.cpp). ⚠ This TU makes NO vendor call of its own: reads come
//  from TPci1203Monitor's samples, writes go through TPci1203Control::Execute.
//  tools/pci1203_control_gate.ps1 check 5d proves it on every run: no
//  `Acm_<name>(` call in CODE (comments and string literals stripped) in this
//  file, Motor/EcatMotorRoute.h or EtherCAT/Pci1203IoRoute.cpp. The vendor names
//  below appear in comments only.
//  Design: docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md (section numbers cited below).
// =============================================================================
#include "MachineType.h"   // SOFT_SIMULTE / INSTALL_1203_MONITOR / WB_PUMP_1203_CONTROL / WB_PUMP_1203_START_RING / WB_ENGINE_MOTOR_1203 decided BEFORE the #if below (tools/macro_order_gate.ps1)
#include "EtherCAT/Pci1203MotorRoute.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <string>

// ---------------------------------------------------------------------------
//  AI(W906-ENG1203) 20260929: review R9 -- THE INSTALL DECISION, made once, here, by the preprocessor.
//  W906_InstallPci1203MotorRoute() below branches on this value only, and Pci1203MotorRouteInstallGate()
//  returns it, so tests/test_pci1203_motor_route.cpp can check that a build WITHOUT WB_ENGINE_MOTOR_1203
//  (the committed tree) compiled the "not routed" branch -- dropping that macro from the test below used
//  to leave every test green. The numbers are the header's kEcGate* values.
//  The compile-only probe ht9045_engine_route_armed_probe (CMakeLists.txt EOF) builds this TU with the
//  arming macros defined and W906_ENG1203_PROBE_EXPECT_ARMED, so the armed branch is type-checked on
//  every build and the static_assert proves it is the branch that probe really compiled.
// ---------------------------------------------------------------------------
#if defined(SOFT_SIMULTE)
#define W906_ENG1203_GATE 1        /* kEcGateSim */
#elif defined(WB_ENGINE_MOTOR_1203) && defined(INSTALL_1203_MONITOR) && defined(WB_PUMP_1203_CONTROL)
#if defined(WB_PUMP_1203_START_RING)
#define W906_ENG1203_GATE 4        /* kEcGateArmedBuild */
#else
#define W906_ENG1203_GATE 3        /* kEcGateNoStartRing (Q9) */
#endif
#else
#define W906_ENG1203_GATE 2        /* kEcGateOff */
#endif
#if defined(W906_ENG1203_PROBE_EXPECT_ARMED)
static_assert(W906_ENG1203_GATE == 4, "ht9045_engine_route_armed_probe must compile the ARMED installer branch");
#endif

void (*g_W906RouteNote)(const char*) = 0;    //AI(W906-ECAT-RECLAIM) 20261003: the op log's ROUTE line (tools/wb_serve.cpp W906_OpLogInit registers it); 0 = console only

namespace ht9045 {

namespace {

// ---------------------------------------------------------------------------
//  The live environment: the monitor that owns the card and EastSun's control
//  layer. Same preconditions the IO route checks (Pci1203IoRoute.cpp:127-136,
//  review P17-CPP-4: a Disabled monitor keeps valid=true samples nobody refreshes).
//  AI(W906-ENG1203) 20260929: the two conditions live in Pci1203MotorRouteCardUsableOf /
//  Pci1203MotorRouteExecCountOf (pinned by the route test); these methods only fetch the objects.
// ---------------------------------------------------------------------------
class LiveEnv : public Pci1203MotorRouteEnv {
public:
    bool CardUsable() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return Pci1203MotorRouteCardUsableOf(m != 0, m != 0 && m->Open_(), m != 0 && m->Disabled(), m != 0 && m->card().open);
    }
    bool ControlArmed() override { return Pci1203Control() != 0; }
    unsigned long PollCount() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m ? m->card().pollCount : 0ul;
    }
    unsigned long ExecCount() override { return Pci1203MotorRouteExecCountOf(Pci1203Control()); }
    int AxisCount() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m ? m->axisCount() : 0;
    }
    const Pci1203AxisSample& Axis(int i) override { return Pci1203Monitor()->axis(i); }    int SlaveCount() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m ? m->slaveCount() : 0;
    }
    const Pci1203SlaveSample& Slave(int i) override { return Pci1203Monitor()->slave(i); }
    Pci1203CmdResult Execute(const Pci1203Cmd& c) override
    {
        TPci1203Control* ctl = Pci1203Control();
        if (ctl == 0) {
            Pci1203CmdResult r;
            r.accepted = false; r.issued = false; r.ret = 0;
            r.why = "1203 control is not armed";
            return r;
        }
        return ctl->Execute(c);
    }
};
LiveEnv               s_live;
Pci1203MotorRouteEnv* s_env = 0;   // 0 = live
Pci1203MotorRouteEnv& Env() { return s_env ? *s_env : s_live; }

// ---------------------------------------------------------------------------
//  Per (station, axis) ledger. Keyed like WebMotorAccess keys nothing -- by the
//  golden identity, because the monitor's slot numbers move on Rescan.
// ---------------------------------------------------------------------------
const int kSpd = Pci1203AxisSample::kSpeedCount;   // speed[] / speedValid[] are indexed by Pci1203SpeedParam

struct AxisRec {
    bool          claimed;
    int           motorId;
    bool          haveCmd;  unsigned long lastCmdPoll;   // pollCount at this route's last Execute on the axis (Q7 stop test)
    bool          havePend; unsigned long pendPoll;      // ... at its last ACCEPTED state-changing command (4.2)
    bool          stuck;                                 // Q11: refused / failed / DRY motion, until the next stop
    bool          stopSkipped;                           // the StopDec of this DecStop was skipped (Q7) -> skip its ExtDrive(0)
    bool          homeActive, homeSent, homeSawHoming, homeCardSide, homeNoHomingNoted;   int homeReadyNoHoming;   //AI(W906-HOME-FAILVISIBLE) 20261003: RouteHomeDone calls that read READY with no HOMING seen
    unsigned long homeCmdPoll = 0;   //AI(W906-HOME-SEEN) 20261003: card pollCount when the home command was issued
    double        spd[kSpd];    // last SUCCESSFUL write per Pci1203SpeedParam (Q7)
    bool          spdOk[kSpd];
    unsigned long bindTries;
    Pci1203MotorRouteFault fault;                        //AI(W906-ENG1203) 20260929: MEDIUM-1 latch (never cleared while wb_serve runs)
    AxisRec()
        : claimed(false), motorId(-1), haveCmd(false), lastCmdPoll(0), havePend(false), pendPoll(0)
        , stuck(false), stopSkipped(false), homeActive(false), homeSent(false), homeSawHoming(false)
        , homeCardSide(false), homeNoHomingNoted(false), homeReadyNoHoming(0), bindTries(0)
    {
        for (int q = 0; q < kSpd; ++q) { spd[q] = 0.0; spdOk[q] = false; }
    }
};
std::map<long, AxisRec> s_axes;
long Key(int board, int port) { return (long)board * 1000L + (long)port; }
AxisRec* Find(int board, int port)
{
    std::map<long, AxisRec>::iterator it = s_axes.find(Key(board, port));
    return it == s_axes.end() ? 0 : &it->second;
}
//AI(W906-ENG1203) 20260929: pending until pollCount > `until`; never SHORTENS a longer window already set
//  (a foreign stop's "now + 1", Pci1203MotorRouteNoteForeignStop). pollCount only grows, so for this route's own
//  commands this is the same as the plain assignment it replaces.
void MarkPending(AxisRec& r, unsigned long until)
{
    if (!r.havePend || until > r.pendPoll) r.pendPoll = until;
    r.havePend = true;
}

Pci1203MotorRouteStats s_stats;
bool                   s_installed = false;
bool                   s_verbose = true;
std::map<long, unsigned long> s_sayPerAxis;              //AI(W906-ENG1203) 20260929: MEDIUM-1 console budget per (station, axis)
unsigned long                 s_sayN = 0;                // every line's sequence number (printed or not)

// ---------------------------------------------------------------------------
//  FOREIGN-EXECUTE DETECTION -- how the stop / speed skips stay safe without
//  the shared ledger Q7 asked for inside Execute (EastSun's Pci1203Control.cpp,
//  outside this change).
//
//  Motor Test, the pci1203 page, Pci1203AxisIniTick and the IO route all call
//  the same TPci1203Control::Execute. This route cannot see WHICH axis they
//  touched, but it can see THAT they did: acceptedCount()+refusedCount() moved
//  between two of its own calls. When it has, s_foreignPoll = the pollCount at
//  which that was noticed, and no skip may use a sample from that Poll or
//  earlier -- the sample could predate the other caller's command (a jog the
//  engine's StopAllMotor must not skip, a speed Motor Test just changed).
//  Conservative by construction: the noticing is never earlier than the command.
//  ⓘ Only the SKIPS consult it. The pending rule (4.2) is this route's own
//  ledger, as the design specifies (the same scope WebMotorAccess.cpp:154-163
//  has); overlapping engine / Motor Test motion is golden's MainProc pause.
//  AI(W906-ENG1203) 20260929: review HIGH-1 -- the one foreign command that DOES change this route's ledger is a
//  STOP of an axis it claimed: the callers that bypass the route report it (Pci1203MotorRouteNoteForeignStop,
//  called from WebMotorAccessLive.cpp / tools/wb_serve.cpp), which cancels the home job and marks the axis pending.
// ---------------------------------------------------------------------------
bool          s_execInit = false;
unsigned long s_execSeen = 0;
unsigned long s_foreignPoll = 0;
void NoteForeign()
{
    Pci1203MotorRouteEnv& e = Env();
    const unsigned long n = e.ExecCount();
    if (!s_execInit || n != s_execSeen) {
        s_foreignPoll = e.PollCount();   // first call too: history before it is unknown = foreign
        s_execSeen = n;
        s_execInit = true;
    }
}

bool Pending(const AxisRec& r, unsigned long pollNow)
{
    return (r.havePend && pollNow <= r.pendPoll) || r.stuck;
}

const char* OpName(int op)
{
    switch (op) {
        case kEcStopDec:    return "StopDec";
        case kEcStopEmg:    return "StopEmg";
        case kEcExtDrive:   return "ExtDrive";
        case kEcJog:        return "Jog";
        case kEcMoveAbs:    return "MoveAbs";
        case kEcMoveRel:    return "MoveRel";
        case kEcSetSpeed:   return "SetSpeed";
        case kEcSetLimit:   return "SetLimit";
        case kEcSvOn:       return "SvOn";
        case kEcResetError: return "ResetError";
        case kEcSetCmdPos:  return "SetCmdPos";
        case kEcSetActPos:  return "SetActPos";
        default:            return "?";
    }
}

// One rate-limited console line, the IO route's shape (Pci1203IoRoute.cpp:181-193):
// the first 30, then every 500th. Never a dialog (design 4.4 / Q10).
//AI(W906-ENG1203) 20260929: review MEDIUM-1 -- the budget is PER (station, axis) now (one chatty axis can no longer
//  silence every other one), and `force` lines -- a failed / refused engine stop -- are never dropped (golden pops
//  WAR16122 for each). The shown / dropped counts are kept even when quiet (tests), in the stats.
void Say(int board, int port, const char* what, const char* verdict, const std::string& why, bool force = false)
{
    const unsigned long seq = ++s_sayN;
    const unsigned long n = ++s_sayPerAxis[Key(board, port)];
    //AI(W906-ECAT-RECLAIM) 20261003: claims / refusals / failures / every Home* line in the op log (ROUTE lines), BEFORE the 30-per-axis
    //  console limit -- the 19:22 HOME stop had no trace of why 3 axes could not be read, and the 19:37 MInArmX stop had no HomeDone line
    //  (st 0 ax 0 had used its 30 console lines on InitMotor / speed writes). Repeated SetCmdPos / SetActPos refusals (Q1, by design) are skipped.
    if (::g_W906RouteNote && std::strncmp(what, "SetCmdPos", 9) != 0 && std::strncmp(what, "SetActPos", 9) != 0 &&
        (std::strcmp(verdict, "REFUSED") == 0 || std::strcmp(verdict, "CLAIMED") == 0 || std::strstr(verdict, "FAIL") != 0 ||
         std::strcmp(verdict, "STOPPED") == 0 || std::strncmp(what, "Home", 4) == 0)) {
        char b[512];
        std::snprintf(b, sizeof(b), "st %d ax %d %s -> %s%s%s", board, port, what, verdict, why.empty() ? "" : "  -- ", why.c_str());
        ::g_W906RouteNote(b);
    }
    if (!force && !(n <= 30 || (n % 500) == 0)) { ++s_stats.linesDropped; return; }
    ++s_stats.linesShown;
    if (!s_verbose) return;
    std::printf("engine motor -> 1203 #%lu: st %d ax %d %s -> %s%s%s   [skipped: stop %lu, extDrive %lu, speed %lu; Q1 refused %lu; stop FAILED %lu]\n",
                seq, board, port, what, verdict, why.empty() ? "" : "  -- ", why.c_str(),
                s_stats.skippedStop, s_stats.skippedExtDrive, s_stats.skippedSpeed, s_stats.refusedCoord, s_stats.stopFailed);
    if (n == 30 && !force) std::printf("engine motor -> 1203: st %d ax %d: further lines of this axis only every 500th (failed stops always)\n", board, port);
}

//AI(W906-ENG1203) 20260929: review MEDIUM-1 -- latch a refused / failed engine stop or motion on the axis (rec may be 0:
//  an axis the route never saw -- then only the totals move). Never raises anything (Q10).
void LatchFault(AxisRec* rec, bool stop, unsigned long code, const char* what)
{
    if (stop) ++s_stats.stopFailed; else ++s_stats.motionFailed;
    if (rec == 0) return;
    if (stop) ++rec->fault.stopFailed; else ++rec->fault.motionFailed;
    rec->fault.lastRet  = code;
    rec->fault.lastPoll = Env().PollCount();
    rec->fault.lastWhat = what ? what : "?";
}
// The kinds whose failure golden reports as a failed STOP: StopDec, StopEmg, and DecStop's ExtDrive(0).
bool IsStopCmd(const Pci1203Cmd& c)
{
    return c.kind == kCmdAxStop || c.kind == kCmdAxEmgStop || (c.kind == kCmdAxSetExtDrive && c.value == 0.0);
}

// Everything a write needs before a Pci1203Cmd can be built. On failure: the route code.
unsigned long Preflight(int board, int port, AxisRec*& rec, int& slot, std::string& why)
{
    Pci1203MotorRouteEnv& e = Env();
    rec = Find(board, port);
    slot = -1;
    if (rec == 0 || !rec->claimed) {
        why = "這一軸沒有被 Open_Axis 認領（golden 的 m_Axishand 是 0）——沒有送出";
        return kEcRcNotClaimed;
    }
    if (!e.ControlArmed()) {
        why = "1203 命令面沒有武裝（Pci1203Control()==0）——沒有送出";
        return kEcRcNoControl;
    }
    if (!e.CardUsable()) {
        why = "1203 卡沒有開，或監看器已停止輪詢（Disabled）——沒有送出";
        return kEcRcNoCard;
    }
    int hits = 0, amb = 0;
    slot = Pci1203MotorRouteFindSlot(e, board, port, &hits, &amb);
    if (slot < 0) {
        char b[224];
        if (amb > 0)   //AI(W906-ENG1203) 20260929: LOW-2
            std::snprintf(b, sizeof(b), "站 %d 軸 %d 的站號不唯一（stationAmbiguous，%d 個軸槽）——沒有送出", board, port, amb);
        else
            std::snprintf(b, sizeof(b), "監看器對 站 %d 軸 %d 有 %d 個開成功的軸槽（要恰好 1 個，不挑）——沒有送出", board, port, hits);
        why = b;
        return kEcRcNoAxis;
    }
    return 0;
}

// Execute one command and keep the ledgers. Returns 0 for issued-OK and for DRY
// (accepted, not issued: golden's caller goes on as after a SUCCESS -- whether a
// MOTION then counts as done is Q11's job, not the return code's).
unsigned long Exec(int board, int port, AxisRec& rec, const Pci1203Cmd& c, const char* what,
                   bool stateChanging, bool motion, bool* issuedOk = 0)
{
    Pci1203MotorRouteEnv& e = Env();
    const unsigned long pc = e.PollCount();
    const Pci1203CmdResult r = e.Execute(c);
    s_execSeen = e.ExecCount();                         // our own command is not "foreign"
    ++s_stats.executed;
    rec.haveCmd = true; rec.lastCmdPoll = pc;
    if (r.accepted && stateChanging) MarkPending(rec, pc);   //AI(W906-ENG1203) 20260929: was `havePend = true; pendPoll = pc;` -- same value, but never shortens a foreign stop's window
    const bool ok = r.accepted && r.issued && r.ret == 0;
    if (issuedOk) *issuedOk = ok;
    if (motion && !ok) rec.stuck = true;                // Q11
    //AI(W906-ENG1203) 20260929: MEDIUM-1 -- a refused / failed (not DRY) stop or motion is latched and its line forced.
    const bool failedOut = !r.accepted || (r.issued && r.ret != 0);
    const bool stopKind  = IsStopCmd(c);
    if (failedOut && (stopKind || motion)) LatchFault(&rec, stopKind, r.accepted ? r.ret : (unsigned long)kEcRcRefused, what);
    const char* verdict;
    if (!r.accepted)     { ++s_stats.refused; verdict = "REFUSED"; }
    else if (!r.issued)  { ++s_stats.dry;     verdict = "DRY"; }
    else if (r.ret != 0) { ++s_stats.failed;  verdict = "FAILED"; }
    else                 { ++s_stats.issued;  verdict = "ISSUED"; }
    std::string line = std::string(what) + "  " + r.wouldCall;
    std::string why = r.why;
    if (r.issued && r.ret != 0) {
        char b[48];
        std::snprintf(b, sizeof(b), "ret=0x%08lX ", r.ret);
        why = b + why;
    }
    if (motion && !ok) why += (why.empty() ? "" : "; ") + std::string("Q11: this axis reports NOT done until the next stop");
    if (failedOut && stopKind) why += (why.empty() ? "" : "; ") + std::string("STOP DID NOT REACH THE CARD (golden WAR16122; latched, see diag.why)");
    Say(board, port, line.c_str(), verdict, why, failedOut && stopKind);
    if (!r.accepted) return kEcRcRefused;
    if (r.issued && r.ret != 0) return r.ret;
    return 0;
}

// Q7 speed write: skipped when the value equals the last SUCCESSFUL write of that
// parameter AND the card reads it back AND no other Execute caller has run since
// the sample was taken. Anything else is written.
unsigned long SetSpeedParam(int board, int port, AxisRec& rec, int slot, int which, double v)
{
    Pci1203Cmd c;
    bool motion = false, stateChanging = false;
    if (!Pci1203MotorRouteMakeCmd(kEcSetSpeed, which, v, slot, c, &motion, &stateChanging)) return kEcRcBadOp;
    Pci1203MotorRouteEnv& e = Env();
    const int sp = (int)c.speed;
    const Pci1203AxisSample& a = e.Axis(slot);
    if (sp >= 0 && sp < kSpd && rec.spdOk[sp] && rec.spd[sp] == v &&
        e.PollCount() > s_foreignPoll && a.valid && a.speedValid[sp] && a.speed[sp] == v) {
        ++s_stats.skippedSpeed;
        return 0;
    }
    bool ok = false;
    char what[64];
    std::snprintf(what, sizeof(what), "SetSpeed[%d]=%.3f", sp, v);
    const unsigned long rc = Exec(board, port, rec, c, what, stateChanging, motion, &ok);
    if (sp >= 0 && sp < kSpd) { rec.spdOk[sp] = ok; rec.spd[sp] = v; }
    return rc;
}

// Q7 stop: skipped when a sample newer than this route's last command on the axis
// and newer than any other caller's last Execute reads READY -- and never for an
// axis that is stuck (Q11: the stop is what releases it) or homing (the stop is what
// cancels the home job).
bool CanSkipStop(const AxisRec& rec, int slot)
{
    Pci1203MotorRouteEnv& e = Env();
    const unsigned long pc = e.PollCount();
    if (rec.stuck || rec.homeActive) return false;
    if (rec.haveCmd && pc <= rec.lastCmdPoll) return false;
    if (pc <= s_foreignPoll) return false;
    const Pci1203AxisSample& a = e.Axis(slot);
    return a.valid && a.opened && a.state == kEcStaReady;
}

// ---------------------------------------------------------------------------
//  The route entries (TEcatMotorRoute). See Motor/EcatMotorRoute.h.
// ---------------------------------------------------------------------------
bool RouteBind(int board, int port, int motorId, bool direction)
{
    NoteForeign();
    Pci1203MotorRouteEnv& e = Env();
    AxisRec& rec = s_axes[Key(board, port)];
    rec.motorId = motorId;
    const bool noisy = (rec.bindTries++ < 3) || (rec.bindTries % 1000) == 0;
    if (direction) {
        ++s_stats.bindRefused;
        if (noisy) Say(board, port, "claim", "REFUSED",
                       "Direction=1：裁決 6B（RULINGS_20260925 第 13 條）1203 軸不看 Direction、方向交給驅動器 Pn000 —— 請把 Mot_Table 的 Direction 改 0（Q5）");
        return false;
    }
    if (!e.CardUsable()) {
        ++s_stats.bindRefused;
        if (noisy) Say(board, port, "claim", "REFUSED", "1203 卡沒有開，或監看器已停止輪詢（Disabled）");
        return false;
    }
    int hits = 0, amb = 0;
    const int slot = Pci1203MotorRouteFindSlot(e, board, port, &hits, &amb);
    if (slot < 0) {
        ++s_stats.bindRefused;
        char b[224];
        if (amb > 0)   //AI(W906-ENG1203) 20260929: LOW-2
            std::snprintf(b, sizeof(b), "站 %d 軸 %d 的站號不唯一（stationAmbiguous，%d 個軸槽）：照站號對應的軸不可信，引擎自動運動不猜 —— 不認領（同 ChanMotorPoints.cpp）", board, port, amb);
        else
            std::snprintf(b, sizeof(b), "監看器對 站 %d 軸 %d 有 %d 個開成功的軸槽（要恰好 1 個，不挑）", board, port, hits);
        if (noisy) Say(board, port, "claim", "REFUSED", b);
        return false;
    }
    rec.claimed = true;
    ++s_stats.bindOk;
    char b[96];
    std::snprintf(b, sizeof(b), "MotorID %d -> monitor slot %d (no axis opened: the monitor holds the handle)", motorId, slot);
    Say(board, port, "claim", "CLAIMED", b);
    return true;
}

bool RouteRead(int board, int port, TEcatAxisRead* out)
{
    NoteForeign();
    Pci1203MotorRouteEnv& e = Env();
    AxisRec* rec = Find(board, port);
    if (rec == 0 || !rec->claimed || out == 0) return false;   // golden: a read on handle 0 fails
    if (!e.CardUsable()) return false;
    int hits = 0;
    const int slot = Pci1203MotorRouteFindSlot(e, board, port, &hits);
    if (slot < 0) return false;
    const Pci1203AxisSample& a = e.Axis(slot);
    if (!a.valid || !a.opened) return false;
    out->valid    = true;
    out->state    = a.state;
    out->motionIO = a.motionIO;
    out->cmdPos   = a.cmdPos;
    out->actPos   = a.actPos;
    out->pending  = Pending(*rec, e.PollCount());
    return true;
}

unsigned long RouteCall(int board, int port, int op, int which, double v)
{
    NoteForeign();
    ++s_stats.calls;
    AxisRec* rec = 0;
    int slot = -1;
    std::string why;
    const unsigned long pre = Preflight(board, port, rec, slot, why);
    Pci1203Cmd c;
    bool motion = false, stateChanging = false;
    const bool known = Pci1203MotorRouteMakeCmd(op, which, v, slot, c, &motion, &stateChanging);
    if (pre != 0) {
        if (rec && motion) rec->stuck = true;           // Q11: a motion that went nowhere never completes
        ++s_stats.refused;
        //AI(W906-ENG1203) 20260929: MEDIUM-1 -- a stop that never reached Execute is a failed stop (golden: WAR16122 on handle 0)
        const bool stopOp = known && IsStopCmd(c);
        if (stopOp || (known && motion)) LatchFault(rec, stopOp, pre, OpName(op));
        Say(board, port, OpName(op), "REFUSED", stopOp ? why + "; STOP DID NOT REACH THE CARD (golden WAR16122; latched)" : why, stopOp);
        return pre;
    }
    const bool afterSkippedStop = rec->stopSkipped;
    rec->stopSkipped = false;
    if (!known) {
        ++s_stats.refused;
        Say(board, port, OpName(op), "REFUSED", "the route does not know this op / which (software defect)");
        return kEcRcBadOp;
    }
    switch (op) {
        case kEcStopDec:
            if (CanSkipStop(*rec, slot)) { rec->stopSkipped = true; ++s_stats.skippedStop; return 0; }
            break;
        case kEcExtDrive:
            //  golden DecStop is StopDec THEN ExtDrive(0) (golden :415-439). When the StopDec
            //  was skipped the pair is: skip its ExtDrive(0) too, and answer SUCCESS so golden
            //  sets bFirstClickJog=true exactly as after a sent one.
            if (v == 0.0 && afterSkippedStop) { ++s_stats.skippedExtDrive; return 0; }
            break;
        case kEcSetSpeed:
            return SetSpeedParam(board, port, *rec, slot, which, v);
        case kEcSetCmdPos:
        case kEcSetActPos: {
            //  Q1: "the drive defines the origin, homing does not zero" (EastSun). A coordinate
            //  write on a DS402 drive -- or one the monitor cannot identify -- is refused; only a
            //  card-side drive (kind 0) follows golden. Recorded, never raised (Q10).
            const int kind = Pci1203MotorRouteDriveKind(Env(), slot);
            if (kind != 0) {
                ++s_stats.refusedCoord;
                Say(board, port, OpName(op), "REFUSED",
                    kind > 0 ? "Q1：DS402 驅動器自己定義原點（EastSun：回原點後不歸零）—— 卡片座標不改寫"
                             : "Q1：判斷不出這一軸的驅動器類型 —— 不猜，卡片座標不改寫");
                return kEcRcDs402Coord;
            }
            break;
        }
        default:
            break;
    }
    const unsigned long rc = Exec(board, port, *rec, c, OpName(op), stateChanging, motion);
    if ((op == kEcStopDec || op == kEcStopEmg) && rc == 0) {
        rec->stuck = false;          // Q11: released by the next stop that went out (issued OK, or DRY)
        rec->homeActive = false;     // a stop cancels the home job (homeDone never answers from it)
    }
    return rc;
}

bool RouteInitCfg(int board, int port, int motorClass, bool sensorType, bool in1Logic,
                  double maxVel, double maxAcc, double maxDec)
{
    NoteForeign();
    ++s_stats.calls;
    AxisRec* rec = 0;
    int slot = -1;
    std::string why;
    if (Preflight(board, port, rec, slot, why) != 0) {
        ++s_stats.refused;
        Say(board, port, "InitMotor", "REFUSED", why);
        return false;
    }
    Pci1203MotorRouteEnv& e = Env();
    //  golden :187-217 ResetMotorError: ResetError (failure = WAR16122, carry on); GetState;
    //  not READY -> ResetError, its failure -> WAR16123 + goto; GetState failing -> WAR16121 +
    //  goto. ⚠ The goto has no bound in golden (it would hang the tick thread); bounded to 3
    //  rounds exactly like WebMotorAccess.cpp InitMotor1203 (kInitResetRounds), then false.
    //  The state is the monitor's last sample, as there.
    bool reset = false;
    for (int round = 1; round <= 3 && !reset; ++round) {
        Pci1203Cmd c; c.kind = kCmdAxResetError; c.axis = slot;
        Exec(board, port, *rec, c, "InitMotor ResetError (golden :189)", true, false);
        const Pci1203AxisSample& a = e.Axis(slot);
        if (!a.valid) continue;                                        // golden WAR16121 + goto
        if (a.state == kEcStaReady) { reset = true; break; }
        if (Exec(board, port, *rec, c, "InitMotor ResetError, not READY (golden :203)", true, false) == 0)
            reset = true;                                              // golden goes on (even if still not READY)
    }
    if (!reset) {
        Say(board, port, "InitMotor", "STOPPED",
            "golden `goto ResetMotorError` retries for ever; bounded to 3 rounds here -- nothing more written, InitMotor returns false");
        return false;
    }
    Pci1203InitCfgStep plan[24];
    const int n = Pci1203GoldenInitCfgPlan(motorClass, sensorType, in1Logic, plan, 24);   // golden :219-361, EastSun's closed table
    for (int i = 0; i < n; ++i) {
        Pci1203Cmd c; c.kind = kCmdAxSetInitCfg; c.axis = slot; c.initCfg = plan[i].which; c.value = plan[i].value;
        char what[80];
        std::snprintf(what, sizeof(what), "InitMotor %s=%g", Pci1203InitCfgName(plan[i].which), plan[i].value);
        Exec(board, port, *rec, c, what, false, false);               // golden: log-and-continue
    }
    SetSpeedParam(board, port, *rec, slot, kEcSpdMaxVel, maxVel);     // golden :363-382 CFG_AxMaxVel = PJogHighSpeed
    SetSpeedParam(board, port, *rec, slot, kEcSpdMaxAcc, maxAcc);     //                 CFG_AxMaxAcc = dAcc
    SetSpeedParam(board, port, *rec, slot, kEcSpdMaxDec, maxDec);     //                 CFG_AxMaxDec = dDec
    Pci1203Cmd c; c.kind = kCmdAxResetError; c.axis = slot;
    Exec(board, port, *rec, c, "InitMotor ResetError (golden :384)", true, false);
    return true;
}

// design 2.3 homeStart: EastSun's homing, chosen by the DRIVE.
//   DS402 -> PTP speeds = home speeds (Acm_AxHome seeds 6099h/609Ah from the PTP family,
//            Pci1203Control.h:415-422) then kCmdAxHome(124 | 128, +1 | -1); no zeroing after.
//   other -> golden's card-side SetHomeSpeed + kCmdAxMoveHome(MODE12_AbsSearchReFind = 11), zeroed after.
//   unknown -> nothing sent (never guess: MODE12 on a DS402 drive fails and golden would zero anyway).
// ⚠ A failed speed / home-parameter write aborts the start (WebMotorAccess.cpp StartHome1203 /
//   StartHomeCardSide do the same); golden would pop WAR16122 and home anyway. Safe-side deviation.
//   AI(W906-ENG1203) 20260929: pinned by the route test (review R7 / R19); every "home not sent" is latched (MEDIUM-1).
int RouteHomeStart(int board, int port, bool homeDir, double hi, double lo, double acc, double dec)
{
    NoteForeign();
    ++s_stats.calls;
    AxisRec* rec = 0;
    int slot = -1;
    std::string why;
    const unsigned long pre = Preflight(board, port, rec, slot, why);
    if (rec) { rec->homeActive = true; rec->homeSent = false; rec->homeSawHoming = false; rec->homeCardSide = false; rec->homeNoHomingNoted = false; rec->homeReadyNoHoming = 0; }
    if (pre != 0) {
        if (rec) rec->stuck = true;
        ++s_stats.refused;
        LatchFault(rec, false, pre, "HomeStart");                     //AI(W906-ENG1203) 20260929: MEDIUM-1 -- a home that went nowhere is visible
        Say(board, port, "HomeStart", "REFUSED", why);
        return 0;
    }
    rec->stuck = true;                                                 // Q11 until the home command is issued OK (cleared below)
    const int kind = Pci1203MotorRouteDriveKind(Env(), slot);
    if (kind < 0) {
        Say(board, port, "HomeStart", "REFUSED",
            "判斷不出這一軸的驅動器類型（監看器沒有這一站的 DS402／SERVOPACK 身分）—— 不猜，沒有送出；golden 的 90 秒逾時會給 HomeFlag=2");
        ++s_stats.refused;
        LatchFault(rec, false, kEcRcRefused, "HomeStart (drive kind unknown)");   //AI(W906-ENG1203) 20260929
        return 0;
    }
    bool ok = false;
    if (kind == 1) {
        if (hi == 0.0) {
            Say(board, port, "HomeStart", "REFUSED", "PHomeHighSpeed 是 0 —— 不拿卡上殘留的速度去找原點（WebMotorAccess NB2 R23 W4C-3）");
            ++s_stats.refused;
            LatchFault(rec, false, kEcRcRefused, "HomeStart (PHomeHighSpeed 0)");   //AI(W906-ENG1203) 20260929
            return 0;
        }
        //AI(W906-HOME-MAXVEL) 20261003: EastSun 1003「全機回home 當掉了」-- the engine home first runs InitMotor (RouteInitCfg above:
        //  CFG_AxMaxVel = PJogHighSpeed, MaxAcc = dAcc, MaxDec = dDec, golden :363-382), then writes the home speeds as PTP speeds; on
        //  HT9050 HomeHighSpeed > JogHighSpeed on almost every servo (MInArmZA / MOutArmZA 100000 > 80000), the card refuses a velocity
        //  above its ceiling (0x80000087, measured on M35 13:15, WebMotorAccess.cpp JOG-MAXVEL), so the home was never sent and step 375
        //  waited forever.  Same rule as Motor Test's JOG-MAXVEL / MT-ACCLIVE (EastSun 1003): a ceiling KNOWN to be below what this home
        //  needs is raised to that value first (never lowered; unknown = nothing sent, as before).  Known = this route's last successful
        //  write of it (InitMotor just now) or else the monitor's read-back.
        {
            const Pci1203AxisSample& smp = Env().Axis(slot);
            const struct { int which; Pci1203SpeedParam sp; double need; const char* nm; } mx[3] = {
                { kEcSpdMaxVel, kSpeedMaxVel, hi,  "CFG_AxMaxVel" }, { kEcSpdMaxAcc, kSpeedMaxAcc, acc, "CFG_AxMaxAcc" },
                { kEcSpdMaxDec, kSpeedMaxDec, dec, "CFG_AxMaxDec" } };
            for (int i = 0; i < 3; ++i) {
                const int s = (int)mx[i].sp;
                if (!(mx[i].need > 0) || s < 0 || s >= kSpd) continue;
                double cur = -1.0;
                if (rec->spdOk[s]) cur = rec->spd[s];
                else if (smp.valid && smp.speedValid[s]) cur = smp.speed[s];
                if (cur < 0 || cur >= mx[i].need) continue;
                char note[160];
                std::snprintf(note, sizeof(note), "%s %.0f < home needs %.0f -> raised first (AI(W906-HOME-MAXVEL))", mx[i].nm, cur, mx[i].need);
                Say(board, port, "HomeStart", "CEILING", note);
                const unsigned long mrc = SetSpeedParam(board, port, *rec, slot, mx[i].which, mx[i].need);
                if (mrc != 0) {
                    Say(board, port, "HomeStart", "STOPPED", "raising the speed ceiling failed -- Acm_AxHome not sent");
                    LatchFault(rec, false, mrc, "HomeStart (speed ceiling)");
                    return 0;
                }
            }
        }
        //AI(W906-HOME-ASSINGLE) 20261003: EastSun「我單軸回home都正常 裡面真的沒有加甚麼奇怪的東西?」-- 20:12 MInShuttle1's Acm_AxHome was
        //  ISSUED (accepted) but the axis never entered HOMING, while the same axis homes fine from Motor Test / Teach. The single-axis
        //  home (WebMotorAccess StartHome1203, AI(W906-HOME-VENDOR)) writes the card's home family PAR_AxHomeVelLow / High / Acc / Dec
        //  first -- Advantech's own Home examples (Examples_EtherCAT/Windows/BCB/Home/Unit1.cpp:663-708) and golden SetHomeSpeed do the
        //  same -- and this engine arm did not. Now the same four writes, same values, same order, before the PTP seed below.
        {
            const struct { int which; double v; } hf[4] = { { kHomeVelLow, lo }, { kHomeVelHigh, hi }, { kHomeAcc, acc }, { kHomeDec, dec } };
            for (int i = 0; i < 4; ++i) {
                Pci1203Cmd hc; hc.kind = kCmdAxSetHome; hc.axis = slot; hc.home = hf[i].which; hc.value = hf[i].v;
                const unsigned long hrc = Exec(board, port, *rec, hc, "HomeStart DS402 SetHome", false, false);
                if (hrc != 0) {
                    Say(board, port, "HomeStart", "STOPPED", "a card home-parameter write failed -- Acm_AxHome not sent");
                    LatchFault(rec, false, hrc, "HomeStart (DS402 card home-parameter write)");
                    return 0;
                }
            }
        }
        const struct { int which; double v; } seq[4] = { { kEcSpdInit, lo }, { kEcSpdRun, hi }, { kEcSpdAcc, acc }, { kEcSpdDec, dec } };
        for (int i = 0; i < 4; ++i) {
            const unsigned long src = SetSpeedParam(board, port, *rec, slot, seq[i].which, seq[i].v);
            if (src != 0) {
                Say(board, port, "HomeStart", "STOPPED", "a home-speed write failed -- Acm_AxHome not sent");
                LatchFault(rec, false, src, "HomeStart (home-speed write)");      //AI(W906-ENG1203) 20260929
                return 0;
            }
        }
        Pci1203Cmd c; c.kind = kCmdAxHome; c.axis = slot;
        c.homeMode = homeDir ? 124 : 128;                              // DS402 method 24 / 28 (EastSun; WebMotorAccess.cpp:1265)
        c.dir      = homeDir ? 1 : -1;                                 // golden HomeDirection ? 0 (POS) : 1 (NEG), vendor -> wire
        Exec(board, port, *rec, c, "HomeStart DS402", true, true, &ok);
    } else {
        const struct { int which; double v; } hs[4] = { { kHomeVelLow, lo }, { kHomeVelHigh, hi }, { kHomeAcc, acc }, { kHomeDec, dec } };
        for (int i = 0; i < 4; ++i) {                                  // golden SetHomeSpeed :1703-1742 (PAR_AxHomeJerk: no entry in the control layer -- gap)
            Pci1203Cmd c; c.kind = kCmdAxSetHome; c.axis = slot; c.home = hs[i].which; c.value = hs[i].v;
            const unsigned long hrc = Exec(board, port, *rec, c, "HomeStart SetHome", false, false);
            if (hrc != 0) {
                Say(board, port, "HomeStart", "STOPPED", "a card home-parameter write failed -- Acm_AxMoveHome not sent");
                LatchFault(rec, false, hrc, "HomeStart (card home-parameter write)");   //AI(W906-ENG1203) 20260929
                return 0;
            }
        }
        Pci1203Cmd c; c.kind = kCmdAxMoveHome; c.axis = slot;
        c.homeMode = 11;                                               // MODE12_AbsSearchReFind (golden :1704 / :1713)
        c.dir      = homeDir ? 1 : -1;
        rec->homeCardSide = true;
        Exec(board, port, *rec, c, "HomeStart card-side", true, true, &ok);
    }
    rec->homeSent = ok;
    rec->homeCmdPoll = Env().PollCount();                              //AI(W906-HOME-SEEN) 20261003: a HOMING sample from a poll that ENDS after this counts (RouteHomeDone)
    //AI(W906-HOME-READYDONE) 20261003: the HOME-WATCH short polls after the command (0178) and the HOME-POLL poll before it (0175) are
    //  removed -- EastSun「為神魔要間格讀取狀態?…之後好幾百台機台 已定會有幾台迴圈比較久 一定出問題」; RouteHomeDone's DS402 rule needs no HOMING sample.
    if (ok) rec->stuck = false;      // Exec() re-marks it when the home command itself was not issued OK
    return ok ? 1 : 0;
}

//AI(W906-HOME-DUALAXIS) 20261003: EastSun ruling 1003「那你要自動偵測雙軸驅動器 如果有一軸在回原點 另一軸 就不能先歸 要等雙軸的第一軸
//  歸完後 第二軸才能歸」. Detected from the card: every monitor axis with the same station number (a SGDXW 2-axis drive answers one
//  station for its axes 0 / 1). True while another axis of that station is HOMING in the monitor's latest sample (any source: the
//  engine, Motor Test, Teach), or while this route's own home on it is active and was issued less than 450 monitor polls (~90 s) ago (so a home just issued,
//  not yet sampled, still holds its twin back; a home left behind by an aborted HOME stops counting after that).
bool RouteSiblingHoming(int board, int port)
{
    Pci1203MotorRouteEnv& e = Env();
    if (!e.CardUsable()) return false;
    const int count = e.AxisCount();
    for (int ax = 0; ax < count; ++ax) {
        const Pci1203AxisSample& s = e.Axis(ax);
        if (!s.opened || !s.valid || s.station != board || s.stationAxis == port) continue;
        if ((s.state & 0xFFu) == 4u) return true;                       // STA_AX_HOMING
        const AxisRec* r = Find(board, s.stationAxis);
        if (r && r->homeActive && r->homeSent && e.PollCount() - r->homeCmdPoll < 450ul) return true;
    }
    return false;
}

// design 2.3 case 10: done = a HOMING sample, then a READY one, both taken after the
// command (pending rule). ERROR_STOP and anything else: stay (golden waits; MotorHome's
// alarm check and its 90 s ResetTime take over). Never HOMING -> never done.
int RouteHomeDone(int board, int port)
{
    NoteForeign();
    Pci1203MotorRouteEnv& e = Env();
    AxisRec* rec = Find(board, port);
    if (rec == 0 || !rec->homeActive) return 0;
    //AI(W906-HOME-FAILVISIBLE) 20261003: EastSun 1003「把1203 的全機回home 修好」-- a home that cannot finish is reported (-1) instead of
    //  0 forever: golden's full HOME waited silently at step 375 (MotorHome's 90 s ResetTime never alarms there, review 1003).
    //  -1 = the home command was never sent (a RouteHomeStart refusal / failed write), ERROR_STOP while homing, or READY for
    //  kHomeReadyNoHomingCalls calls (~5 s at the 500 ms engine beat) without one HOMING sample. TMyEtherCatMotor turns it into a
    //  home fault -> TMyMotor::MotorHome returns 2 -> uhome's motor-alarm path (jam code popup, HOME stopped).
    const int kHomeReadyNoHomingCalls = 10;
    if (!rec->homeSent) {
        rec->homeActive = false;
        Say(board, port, "HomeDone", "FAILED", "the home command was never sent (see the HomeStart line) -> reported as a failed home");
        return -1;
    }
    if (!e.CardUsable()) return 0;
    int hits = 0;
    const int slot = Pci1203MotorRouteFindSlot(e, board, port, &hits);
    if (slot < 0) return 0;
    const Pci1203AxisSample& a = e.Axis(slot);
    if (!a.valid || !a.opened) return 0;
    if (Pending(*rec, e.PollCount())) return 0;
    if (a.state == kEcStaHoming) { rec->homeSawHoming = true; return 0; }
    //AI(W906-HOME-SEEN) 20261003: EastSun 1003「又失敗了」-- 19:46 the full HOME issued MInArmX's home at 16.7 s and looked again only at
    //  27.1 s (one engine pass starts every axis of the phase, each after its InitMotor): the home had finished in between, so 9 axes read
    //  READY without a HOMING sample and MInArmX was failed at 31.6 s. The monitor polls every ~200 ms on its own and remembers the last
    //  poll that sampled HOMING: one that ended after the command counts as seen (Motor Test's own home ticks fast enough to see it itself).
    if (!rec->homeSawHoming && a.homingSeenPoll > rec->homeCmdPoll) rec->homeSawHoming = true;
    if (a.state == kEcStaErrorStop) {                                  //AI(W906-HOME-FAILVISIBLE) 20261003
        rec->homeActive = false;
        Say(board, port, "HomeDone", "FAILED", "ERROR_STOP while homing -> reported as a failed home");
        return -1;
    }
    //AI(W906-HOME-READYDONE) 20261003: EastSun「為神魔要間格讀取狀態? 他API下出去 如果回復完成 不就是歸原點完成嗎? 你用間格時間就算現在
    //  正常 之後好幾百台機台 已定會有幾台迴圈比較久 一定出問題」+「你不用下指令回讀狀態 你只要下指令 該軸就是還沒有回原點狀態」(EastSun is
    //  the 1203 layer's author) -- a done rule that needs a HOMING sample depends on how often somebody looks (19:46 / 19:54 / 20:18 all
    //  failed short homes that ended between two looks). Acm_AxHome returns at once (it only starts the home); from the moment it is
    //  issued the axis is "homing, not homed", and the card reports STA_AX_READY again when the home ends. So for a DS402 home:
    //  any sample taken AFTER the command (the pending rule above) that reads READY means the home is over, however late it is read --
    //  and the end position says how it ended: cmd 0 = completed (HOME-ENDPOS, every completed home today), anywhere else = interrupted
    //  or not done = failed. No timing anywhere. (The card-side MODE12 home keeps the HOMING-then-READY rule below.)
    if (a.state == kEcStaReady && !rec->homeCardSide) {
        rec->homeActive = false;
        if (a.cmdPos > 1.0 || a.cmdPos < -1.0) {
            char b[200];
            std::snprintf(b, sizeof(b), "READY after the home command but the command position is %.0f, not 0 (the drive's home position) -- the home was interrupted or did not complete -> reported as a failed home", a.cmdPos);
            Say(board, port, "HomeDone", "FAILED", b);
            return -1;
        }
        Say(board, port, "HomeDone", "DONE", rec->homeSawHoming ? "HOMING seen, READY at position 0" : "READY at position 0 after the home command (the home ended before a HOMING sample -- not needed)");
        return 1;
    }
    if (a.state == kEcStaReady) {
        if (!rec->homeSawHoming) {
            if (!rec->homeNoHomingNoted) {
                rec->homeNoHomingNoted = true;
                Say(board, port, "HomeDone", "WAITING", "READY without a HOMING sample since the home command -- not reported done yet");
            }
            if (++rec->homeReadyNoHoming >= kHomeReadyNoHomingCalls) {    //AI(W906-HOME-FAILVISIBLE) 20261003: was 0 forever
                rec->homeActive = false;
                Say(board, port, "HomeDone", "FAILED", "READY ~5 s without a HOMING sample -> reported as a failed home");
                return -1;
            }
            return 0;
        }
        rec->homeActive = false;
        //AI(W906-HOME-ENDPOS) 20261003: EastSun「回home 失敗 跟我之前 回X 後 X還在回 直接按Y回 X就停了 有沒有關西?」-- HOMING -> READY
        //  is also what an interrupted home looks like. In today's oplog every completed DS402 home ended at cmd=0 (the drive's home
        //  position; Q1: the card coordinates are never rewritten) and every interrupted one ended elsewhere (09:19 MInArmX 6851,
        //  15:54 MInArmX -1112, 10:35 MOutArmX 12141, 13:12 MOutShuttle2 5541). A DS402 home that ends away from 0 is a failed home,
        //  never "homed" (the card-side MODE12 home is zeroed afterwards by EtherCatMotHome, so it is not checked here).
        if (!rec->homeCardSide && (a.cmdPos > 1.0 || a.cmdPos < -1.0)) {
            char b[160];
            std::snprintf(b, sizeof(b), "HOMING -> READY but the command position is %.0f, not 0 -- the home was interrupted -> reported as a failed home", a.cmdPos);
            Say(board, port, "HomeDone", "FAILED", b);
            return -1;
        }
        return 1;
    }
    return 0;
}

bool RouteHomeCardSide(int board, int port)
{
    AxisRec* rec = Find(board, port);
    return rec != 0 && rec->homeCardSide;
}

// Defined unconditionally so every build configuration compiles the same code;
// only the INSTALL below is conditional.
const TEcatMotorRoute s_route = { RouteBind, RouteRead, RouteCall, RouteInitCfg,
                                  RouteHomeStart, RouteHomeDone, RouteHomeCardSide, RouteSiblingHoming };

}  // namespace

Pci1203MotorRouteEnv& Pci1203MotorRouteLiveEnv() { return s_live; }

int Pci1203MotorRouteFindSlot(Pci1203MotorRouteEnv& e, int station, int stationAxis, int* hits, int* ambiguous)
{
    int hit = -1, n = 0, amb = 0;
    const int count = e.AxisCount();
    for (int ax = 0; ax < count; ++ax) {
        const Pci1203AxisSample& s = e.Axis(ax);
        if (!s.opened) continue;
        if (s.station == station && s.stationAxis == stationAxis) {
            if (hit < 0) hit = ax;
            ++n;
            if (s.stationAmbiguous) ++amb;                             //AI(W906-ENG1203) 20260929: LOW-2 (ChanMotorPoints.cpp FindCardAxis's rule)
        }
    }
    if (hits) *hits = n;
    if (ambiguous) *ambiguous = amb;
    return (n == 1 && amb == 0) ? hit : -1;
}

bool Pci1203MotorRouteCardUsableOf(bool monitorPresent, bool open_, bool disabled, bool cardOpen)
{
    return monitorPresent && open_ && !disabled && cardOpen;
}
unsigned long Pci1203MotorRouteExecCountOf(const TPci1203Control* c)
{
    return c ? c->acceptedCount() + c->refusedCount() : 0ul;
}

bool Pci1203MotorRouteAxisFault(int station, int stationAxis, Pci1203MotorRouteFault& out)
{
    const AxisRec* rec = Find(station, stationAxis);
    out = rec ? rec->fault : Pci1203MotorRouteFault();
    return rec != 0;
}
std::string Pci1203MotorRouteFaultText(const Pci1203MotorRouteFault& f)
{
    if (f.stopFailed == 0 && f.motionFailed == 0) return std::string();
    char b[256];
    std::snprintf(b, sizeof(b), "引擎馬達路由（1203）：停止失敗 %lu 次（golden 會跳 WAR16122 停機）、運動被拒／失敗 %lu 次；最後一次 %s，碼 0x%08lX，輪詢 #%lu —— 這一軸的狀態要到機台確認",
                  f.stopFailed, f.motionFailed, f.lastWhat.c_str(), f.lastRet, f.lastPoll);
    return std::string(b);
}

bool Pci1203MotorRouteNoteForeignStop(const Pci1203Cmd& c, const Pci1203CmdResult& r, int* station, int* stationAxis)
{
    if (station) *station = -1;
    if (stationAxis) *stationAxis = -1;
    if (EcatMotorRoute() != &s_route) return false;                   // no engine route installed: touch nothing
    if (c.kind != kCmdAxStop && c.kind != kCmdAxEmgStop) return false;
    if (!r.accepted || (r.issued && r.ret != 0)) return false;        // it did not go out: this route's own stop rule
    Pci1203MotorRouteEnv& e = Env();
    if (c.axis < 0 || c.axis >= e.AxisCount()) return false;
    const Pci1203AxisSample& a = e.Axis(c.axis);
    if (!a.opened || a.station < 0) return false;
    AxisRec* rec = Find(a.station, a.stationAxis);
    if (rec == 0 || !rec->claimed) return false;                       // the engine never had this axis: nothing of ours to correct
    const unsigned long pc = e.PollCount();
    const bool hadHome = rec->homeActive;
    rec->homeActive = false;                                           // a stop cancels the home job (as for this route's own stops)
    MarkPending(*rec, pc + 1);                                         // the stop may have run INSIDE Poll pc+1 (yield hook): wait for the one after
    ++s_stats.foreignStops;
    char b[200];
    std::snprintf(b, sizeof(b), "slot %d stopped by another Execute caller (%s)%s -- pending until poll > %lu; the caller resets golden fCMD on the engine rows",
                  c.axis, Pci1203CmdName(c.kind), hadHome ? "; home job CANCELLED" : "", pc + 1);
    Say(a.station, a.stationAxis, "foreign stop", "NOTED", b);
    if (station) *station = a.station;
    if (stationAxis) *stationAxis = a.stationAxis;
    return true;
}

int Pci1203MotorRouteInstallGate() { return W906_ENG1203_GATE; }

int Pci1203MotorRouteDriveKind(Pci1203MotorRouteEnv& e, int slot)
{
    if (slot < 0 || slot >= e.AxisCount()) return -1;
    const Pci1203AxisSample& a = e.Axis(slot);
    if (!a.opened || a.station < 0) return -1;
    if (a.driveIsSigmaX) return 1;
    bool otherProfile = false;
    const int n = e.SlaveCount();
    for (int i = 0; i < n; ++i) {
        const Pci1203SlaveSample& s = e.Slave(i);
        if (!s.present || s.addr != a.station) continue;
        if (s.ring >= 0 && s.ring != 0) continue;                      // drive SDO / identity is ring 0
        if (s.profileValid && s.profile == 402) return 1;
        std::string u(s.name);
        for (std::size_t k = 0; k < u.size(); ++k) if (u[k] >= 'a' && u[k] <= 'z') u[k] = (char)(u[k] - 'a' + 'A');
        if (u.find("SERVOPACK") != std::string::npos) return 1;
        if (s.profileValid) otherProfile = true;
    }
    return otherProfile ? 0 : -1;
}

bool Pci1203MotorRouteMakeCmd(int op, int which, double v, int slot, Pci1203Cmd& out,
                              bool* motion, bool* stateChanging)
{
    Pci1203Cmd c;
    c.axis = slot;
    bool mo = false, st = true;
    switch (op) {
        case kEcStopDec:    c.kind = kCmdAxStop; break;                                  // Acm_AxStopDec
        case kEcStopEmg:    c.kind = kCmdAxEmgStop; break;                               // Acm_AxStopEmg
        case kEcExtDrive:   c.kind = kCmdAxSetExtDrive; c.value = (v != 0.0) ? 1.0 : 0.0; break;
        case kEcJog:        c.kind = kCmdAxJogStart; c.dir = (v == 0.0) ? 1 : -1; mo = true; break;   // vendor 0 = DIRECTION_POS -> wire +1, 1 -> -1 (Pci1203Control.h:165-169)
        case kEcMoveAbs:    c.kind = kCmdAxMoveAbs; c.value = v; mo = true; break;
        case kEcMoveRel:    c.kind = kCmdAxMoveRel; c.value = v; mo = true; break;
        case kEcSvOn:       c.kind = kCmdAxSvOn; c.value = (v != 0.0) ? 1.0 : 0.0; break;
        case kEcResetError: c.kind = kCmdAxResetError; break;
        case kEcSetCmdPos:  c.kind = kCmdAxSetCmdPos; c.value = v; break;
        case kEcSetActPos:  c.kind = kCmdAxSetActPos; c.value = v; break;
        case kEcSetSpeed: {
            static const Pci1203SpeedParam kMap[] = { kSpeedInit, kSpeedRun, kSpeedAcc, kSpeedDec,
                                                      kSpeedJogInit, kSpeedJogRun, kSpeedJogAcc, kSpeedJogDec,
                                                      kSpeedMaxVel, kSpeedMaxAcc, kSpeedMaxDec };
            if (which < 0 || which >= (int)(sizeof(kMap) / sizeof(kMap[0]))) return false;
            c.kind = kCmdAxSetSpeed; c.speed = kMap[which]; c.value = v; st = false;
            break;
        }
        case kEcSetLimit:
            if (which == kEcLimSwPel)      c.limit = kLimitSwPelValue;
            else if (which == kEcLimSwMel) c.limit = kLimitSwMelValue;
            else return false;
            c.kind = kCmdAxSetLimit; c.value = v; st = false;
            break;
        default:
            return false;
    }
    out = c;
    if (motion) *motion = mo;
    if (stateChanging) *stateChanging = st;
    return true;
}

const Pci1203MotorRouteStats& Pci1203MotorRouteStatsNow() { return s_stats; }
const TEcatMotorRoute* Pci1203MotorRouteTable() { return &s_route; }
bool Pci1203MotorRouteInstalled() { return s_installed && EcatMotorRoute() == &s_route; }
void Pci1203MotorRouteSetEnvForTest(Pci1203MotorRouteEnv* env) { s_env = env; }
void Pci1203MotorRouteResetForTest()
{
    s_axes.clear();
    s_stats = Pci1203MotorRouteStats();
    s_execInit = false; s_execSeen = 0; s_foreignPoll = 0;
    s_sayPerAxis.clear(); s_sayN = 0;                                  //AI(W906-ENG1203) 20260929
}
void Pci1203MotorRouteSetVerboseForTest(bool on) { s_verbose = on; }

}  // namespace ht9045

void W906_InstallPci1203MotorRoute()
{
//AI(W906-ENG1203) 20260929: the branches are chosen by W906_ENG1203_GATE (top of this file, review R9); same
//  conditions, same messages, same order as before.
#if W906_ENG1203_GATE == 1
    std::printf("engine motor -> 1203: SIM build -- golden sets every Motor->Enable=false (cinitial.cpp), route NOT installed\n");
#elif W906_ENG1203_GATE == 3 || W906_ENG1203_GATE == 4
#if W906_ENG1203_GATE == 3
    //  Q9 (default: "presuppose it ON; WB_ENGINE_MOTOR_1203 is ruled together with it"): with
    //  ring 0's cyclic exchange not started a motion command returns SUCCESS, the axis does not
    //  move and motionIO is frozen (MachineType.h WB_PUMP_1203_START_RING banner) -- the engine
    //  would then see READY as arrival and an ORG / SVON lamp that never changes.
    std::printf("engine motor -> 1203: NOT routed (Q9: WB_PUMP_1203_START_RING is off -- ring 0 not started, motion would report SUCCESS without moving) -- TMyEtherCatMotor keeps its stub arms\n");
#else
    ht9045::TPci1203Control* ctl = ht9045::Pci1203Control();
    if (ctl == 0) {
        //  design 7.2: not armed = every command refused, and golden would take the next
        //  READY as arrival (4.2) -- so not installed at all.
        std::printf("engine motor -> 1203: NOT routed (1203 control is not armed: Pci1203ControlEnable failed or this binary has no HAVE_PCI1203) -- TMyEtherCatMotor keeps its stub arms\n");
        return;
    }
    SetEcatMotorRoute(&ht9045::s_route);
    ht9045::s_installed = true;
    std::printf("engine motor -> 1203: ROUTED -- TMyEtherCatMotor reads the monitor's axis samples and writes through Pci1203Control (%s)\n",
                ctl->IsDryRun() ? "DRY RUN: validated and recorded, nothing issued; every motion reports NOT done (Q11)"
                                : "*** LIVE: engine motion reaches the card ***");
#endif
#else
    std::printf("engine motor -> 1203: NOT routed (needs WB_ENGINE_MOTOR_1203 + INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL; WB_ENGINE_MOTOR_1203 is off by default) -- TMyEtherCatMotor keeps its stub arms\n");
#endif
}
