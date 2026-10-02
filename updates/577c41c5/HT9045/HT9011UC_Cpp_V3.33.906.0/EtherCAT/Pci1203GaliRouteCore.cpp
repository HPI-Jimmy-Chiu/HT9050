// =============================================================================
//  EtherCAT/Pci1203GaliRouteCore.cpp -- see the header.
//
//  AI(W906-INDEXZ-1203) 20260929: new file, wb_serve (CMakeLists.txt, next to
//  Pci1203IoRoute.cpp) + tests/test_gali_route_core.cpp + tests/test_gali_route_engine.cpp.
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo -- review #1 (a stopped home is never done), #2 (VS0;SP0 halts,
//  START / golden's SP resumes), #6 (failed stops latched + always printed), foreign stops (NoteForeignStop),
//  the installer's decision as a pure function (#5). See the header for the rules.
//  AI(W906-INDEXZ-1203) 20260930: review round 2 (header A, B, D1-D7): no SystemStart resume rule any more, DP,
//  halted homes, operator vs alarm-sweep foreign stops, the ledger ends a halt, per-key forced-line cooldown,
//  card-side home zeroing / DecStop failures, owner-thread confinement. Also + tests/test_gali_route_live.cpp.
//  AI(W906-INDEXZ2) 20261002: the TS byte's HOME bit follows the tree's ORG rule (IGaliRouteIo::OrgHome; HT9050 = per axis by SensorType).
//  ⚠ No vendor call, no monitor, no control object: everything goes through
//  IGaliRouteIo (the live binding is EtherCAT/Pci1203GaliRoute.cpp).
// =============================================================================
#include "MachineType.h"   // SOFT_SIMULTE / INSTALL_1203_MONITOR / WB_PUMP_1203_CONTROL / WB_PUMP_1203_START_RING / WB_ENGINE_INDEXZ_1203 decided BEFORE the #if below (tools/macro_order_gate.ps1)
#include "EtherCAT/Pci1203GaliRouteCore.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// ---------------------------------------------------------------------------
//  AI(W906-INDEXZ) 20260930: THE INSTALL DECISION, made once, here, by the preprocessor -- the engine motor route's
//  shape (Pci1203MotorRoute.cpp W906_ENG1203_GATE, same conditions: its Q9 START_RING rule included). The installer
//  (EtherCAT/Pci1203GaliRoute.cpp) branches on GaliRouteCompiledGate() only, so all of its code is compiled in every
//  build; tests/test_gali_route_core.cpp part K compares the value with the macros of the build it runs in, and
//  ht9045_indexz_route_armed_probe (CMakeLists.txt EOF) compiles this TU with the arming macros and the
//  static_assert below.
// ---------------------------------------------------------------------------
#if defined(SOFT_SIMULTE)
#define W906_IDXZ_GATE 1        /* kGaliGateSim */
#elif defined(WB_ENGINE_INDEXZ_1203) && defined(INSTALL_1203_MONITOR) && defined(WB_PUMP_1203_CONTROL)
#if defined(WB_PUMP_1203_START_RING)
#define W906_IDXZ_GATE 4        /* kGaliGateArmedBuild */
#else
#define W906_IDXZ_GATE 3        /* kGaliGateNoStartRing (the engine route's Q9) */
#endif
#else
#define W906_IDXZ_GATE 2        /* kGaliGateOff -- the committed tree */
#endif
#if defined(W906_IDXZ_PROBE_EXPECT_ARMED)
static_assert(W906_IDXZ_GATE == 4, "ht9045_indexz_route_armed_probe must compile the ARMED gate");
#endif

namespace ht9045 {

int GaliRouteCompiledGate() { return W906_IDXZ_GATE; }

namespace {

const unsigned      kStaDisable     = 0u;           // STA_AX_DISABLE     (EtherCAT/vendor/AdvMotDrv.h:792)
const unsigned      kStaReady       = 1u;           // STA_AX_READY       (:793)
const unsigned      kStaErrorStop   = 3u;           // STA_AX_ERROR_STOP  (:795)
const unsigned      kStaHoming      = 4u;           // STA_AX_HOMING      (:796)
const unsigned long kIoAlm          = 0x00000002ul; // AX_MOTION_IO_ALM   (:2024)
const unsigned long kIoLmtp         = 0x00000004ul; // AX_MOTION_IO_LMTP  (:2025)
const unsigned long kIoLmtn         = 0x00000008ul; // AX_MOTION_IO_LMTN  (:2026)
const unsigned long kIoOrg          = 0x00000010ul; // AX_MOTION_IO_ORG   (:2027)
const unsigned long kIoSvon         = 0x00004000ul; // AX_MOTION_IO_SVON  (:2037)
const long          kTiZ1           = 0x08;         // MyLongMask[3] (myio.cpp:187) -- golden Gali_TiData[1]=3 is MTestZ1's TI bit
const long          kTsUnobservable = 0x8E;         // moving, no limit, not home
const unsigned long kHomeNoHomingMs = 5000ul;       // WebMotorAccess.cpp kHomeNoHomingMs
const unsigned long kHomeTimeoutMs  = 180000ul;     // WebMotorAccess.cpp kHomeTimeoutMs
const unsigned long kHomeZeroWaitMs = 300ul;        // TickHomes card-side 0.3 s (golden EtherCatMotHome HomeDelay)
const unsigned long kInvalidPolls   = 3ul;          // polls without a valid sample before "TI" reports the alarm
const unsigned long kForceRepeatMs  = 10000ul;      // AI(W906-INDEXZ) 20260930: a forced line of the same KEY is printed at most once per 10 s (counted)
const std::size_t   kForceKeysMax   = 64u;          // AI(W906-INDEXZ-1203) 20260930 (D4): the per-key table stays small

char AxisFold(char c)
{
    c = (char)std::toupper((unsigned char)c);
    switch (c) {                                    // Galil: A/B/C/D are the same axes as X/Y/Z/W
        case 'A': return 'X';
        case 'B': return 'Y';
        case 'C': return 'Z';
        case 'D': return 'W';
        default:  return c;
    }
}

std::string Trim(const std::string& s)
{
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

// Strict: optional sign, then digits only. "?" or "" -> false.
bool ParseLong(const std::string& s, long& v)
{
    const std::string t = Trim(s);
    if (t.empty()) return false;
    std::size_t i = (t[0] == '-' || t[0] == '+') ? 1 : 0;
    if (i >= t.size()) return false;
    for (std::size_t k = i; k < t.size(); ++k)
        if (!std::isdigit((unsigned char)t[k])) return false;
    v = std::strtol(t.c_str(), 0, 10);
    return true;
}

// The Y value of an SP / AC / DC / PA / DP statement: "SPY=n" or positional field 1.
bool YValueOf(const GaliStmt& s, long& v)
{
    if (s.assign) return s.axes == "Y" && ParseLong(s.value, v);
    if (s.fields.size() > 1) return ParseLong(s.fields[1], v);
    return false;
}

bool IdleState(unsigned st)
{
    st &= 0xFFu;
    return st == kStaDisable || st == kStaReady || st == kStaErrorStop;
}

std::string Num(long v)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%ld", v);
    return b;
}

// A result that did not go out: refused, or issued with a vendor error. DRY (accepted, not issued) went out
// as far as this route's STOP rule goes (the engine route's rule, Pci1203MotorRoute.cpp Exec).
bool DidNotGoOut(const Pci1203CmdResult& r)
{
    return !r.accepted || (r.issued && r.ret != 0);
}

}  // namespace

int GaliSplit(const char* data, std::vector<GaliStmt>& out)
{
    out.clear();
    if (data == 0) return 0;
    const std::string all(data);
    const std::size_t n = all.size();
    std::size_t p = 0;
    while (p < n) {
        std::size_t q = all.find(';', p);
        if (q == std::string::npos) q = n;
        const std::string t = Trim(all.substr(p, q - p));
        p = q + 1;
        if (t.empty()) continue;
        GaliStmt s;
        std::size_t i = 0;
        while (i < t.size() && i < 2 && std::isalpha((unsigned char)t[i])) {
            s.cmd += (char)std::toupper((unsigned char)t[i]);
            ++i;
        }
        const std::string rest = t.substr(i);
        if (s.cmd == "MG") {                        // "MG_BGy", "MG _XQ": the operand after '_'
            std::size_t k = 0;
            while (k < rest.size() && rest[k] == ' ') ++k;
            if (k < rest.size() && rest[k] == '_') ++k;
            for (; k < rest.size(); ++k)
                if (!std::isspace((unsigned char)rest[k])) s.arg += (char)std::toupper((unsigned char)rest[k]);
            if (s.arg.size() == 3) s.arg[2] = AxisFold(s.arg[2]);   // _BGB == _BGY
        } else if (!rest.empty()) {
            const char c0 = rest[0];
            if (std::isalpha((unsigned char)c0)) {  // "BGYZ", "SPY=900", "LMXYZW"
                std::size_t k = 0;
                while (k < rest.size() && std::isalpha((unsigned char)rest[k])) { s.axes += AxisFold(rest[k]); ++k; }
                if (k < rest.size() && rest[k] == '=') { s.assign = true; s.value = Trim(rest.substr(k + 1)); }
            } else if (c0 == '=') {
                s.assign = true;
                s.value = Trim(rest.substr(1));
            } else {                                // positional: ",a,b" / "0,0,0,0" / "1"
                std::size_t a = 0;
                for (;;) {
                    const std::size_t b = rest.find(',', a);
                    if (b == std::string::npos) { s.fields.push_back(Trim(rest.substr(a))); break; }
                    s.fields.push_back(Trim(rest.substr(a, b - a)));
                    a = b + 1;
                }
            }
        }
        out.push_back(s);
    }
    return (int)out.size();
}

bool GaliRouteRowOk(const std::string& cardModel, int enable, int boardId, int port, int direction, std::string& why)
{
    char b[160];
    if (cardModel != "PCI1203") { why = "the M14 row's CardModel is '" + cardModel + "', not PCI1203 (not HT9050's Index Z)"; return false; }
    if (enable == 0)            { why = "the M14 row has Enable 0"; return false; }
    if (boardId < 0 || port < 0) {
        std::snprintf(b, sizeof(b), "the M14 row has no BoardID/Port (%d/%d)", boardId, port);
        why = b;
        return false;
    }
    if (direction != 0) {
        why = "the M14 row has Direction 1 -- the route maps card = flow like Motor Test (RULINGS_20260925 #13 6B); a flipped row is not guessed";
        return false;
    }
    why.clear();
    return true;
}

bool GaliRouteInstallOk(const GaliRouteInstallFacts& f, std::string& why)
{
    char b[260];
    if (!f.linked)       { why = "this binary has no PCIE-1203 SDK (HAVE_PCI1203=0)"; return false; }
    if (!f.controlArmed) { why = "1203 control is not armed (Pci1203ControlEnable failed, see the line above)"; return false; }
    if (!f.monitorPresent) {                        // AI(W906-INDEXZ-1203) 20260930: review C -- the installer used to run before the monitor existed
        why = "the 1203 monitor does not exist yet -- the installer ran BEFORE Pci1203MonitorEnable (a boot-order bug: tools/wb_serve.cpp calls it after the monitor block; tools/pci1203_control_gate.ps1 check 6, ctest GaliRouteLive)";
        return false;
    }
    if (!f.cardUsable) {
        why = "the 1203 card is not open (or the monitor is Disabled) -- a route with no card would answer every Z1 query 'moving / alarm' (review #5)";
        return false;
    }
    if (f.indexMotionCard != 0) {
        std::snprintf(b, sizeof(b), "INDEX_MOTION_CARD=%d in memory: MTestZ1 is not the Galil index axis", f.indexMotionCard);
        why = b;
        return false;
    }
    if (!f.haveRow) { why = "Mot_Table has no M14 (MTestZ1) row"; return false; }
    if (!GaliRouteRowOk(f.cardModel, f.enable, f.boardId, f.port, f.direction, why)) return false;
    if (f.slotHits != 1 || f.slotAmbiguous > 0) {
        std::snprintf(b, sizeof(b), "the monitor has %d opened axis slot(s) at station %d axis %d (%d stationAmbiguous) -- exactly 1, unambiguous, is required",
                      f.slotHits, f.boardId, f.port, f.slotAmbiguous);
        why = b;
        return false;
    }
    if (!f.z1Present)  { why = "MOT[MTestZ1] is missing or disabled"; return false; }
    if (f.routeTaken)  { why = "another Gali route is already installed (one owner per axis)"; return false; }
    if (f.engineRowsAtAddr > 0) {
        std::snprintf(b, sizeof(b), "%d TMyEtherCatMotor row(s) sit at station %d axis %d -- the engine motor route could claim the same axis (one owner per axis)",
                      f.engineRowsAtAddr, f.boardId, f.port);
        why = b;
        return false;
    }
    why.clear();
    return true;
}

long GaliRouteTsByte(bool moving, bool sampleValid, unsigned long motionIO, int orgHome)
{
    if (!sampleValid) return kTsUnobservable;
    long t = moving ? 0x80 : 0;
    if ((motionIO & kIoLmtn) == 0) t |= 0x08;       // golden Led[iCwLed]  = !(TS&0x08) -> LMT- (myEthercatmotor.cpp:1168)
    if ((motionIO & kIoLmtp) == 0) t |= 0x04;       // golden Led[iCcwLed] = !(TS&0x04) -> LMT+ (:1167)
    //AI(W906-INDEXZ2) 20261002: golden Led[iHomeLed] = !(TS&0x02). The HOME bit used to be fixed to "ORG bit set = at home"
    //  (golden EtherCAT's decode, :1166/:1174) -- wrong for HT9050, where the tree reads the ORG bit by the axis's Mot_Table
    //  SensorType (WebMotorAccess.cpp MotorAccessTeachHomeLed through the engine hook W906_Ht9050OrgHome; M14 SensorType 1
    //  = LOW at home, EastSun 20261001 / AI(W906-HT9050-ORG-ST)). It now takes the tree's answer (orgHome); -2 (no machine
    //  rule) keeps golden EtherCAT's decode; -1 (unknown) reads as not at home.
    const bool atHome = (orgHome == -2) ? ((motionIO & kIoOrg) != 0) : (orgHome == 1);
    if (!atHome) t |= 0x02;
    return t;
}

std::string GaliRouteFaultText(const GaliRouteFault& f)
{
    if (f.stopFailed == 0 && f.motionFailed == 0) return std::string();
    char b[400];
    std::snprintf(b, sizeof(b), "Index Z1 路由（Gali_* -> 1203）：停止失敗 %lu 次（golden 的 Galil 命令失敗會跳 WAR1635 停機）、運動被拒／失敗 %lu 次；最後一次：%s（輪詢 #%lu）—— 這一軸的狀態要到機台確認",
                  f.stopFailed, f.motionFailed, f.lastWhat.c_str(), f.lastPoll);
    return std::string(b);
}

TGaliRouteCore::TGaliRouteCore() : io_(0)
{
    Bind(0);
}

void TGaliRouteCore::Bind(IGaliRouteIo* io)
{
    io_ = io;
    onThr_ = false;
    poisoned_ = false;
    poisonWhy_.clear();
    halted_ = haltHome_ = moveInProgress_ = false;
    lastMove_ = MoveRec();
    home_ = HomeRec();
    lmHasY_ = false;
    haveOwn_ = false;
    ownPoll_ = 0;
    haveForeign_ = false;
    foreignPoll_ = 0;
    invalidSeen_ = false;
    invalidSincePoll_ = 0;
    haveLast_ = false;
    lastTd_ = lastTp_ = 0;
    tdHeld_ = false;
    lastSlot_ = -1;
    lastPc_ = 0;
    homeActive_ = homeCancelled_ = homeCardSide_ = sawHoming_ = readySeen_ = zeroed_ = zeroWait_ = false;
    homeStartMs_ = readySinceMs_ = zeroWaitMs_ = 0;
    moves_ = stops_ = resumes_ = 0;
    fault_ = GaliRouteFault();
    force_.clear();
    heldTotal_ = 0;
}

//  AI(W906-INDEXZ-1203) 20260930 (D7): the monitor's pollCount is read on the owner thread only.
unsigned long TGaliRouteCore::Pc()
{
    if (onThr_) lastPc_ = io_->PollCount();
    return lastPc_;
}

bool TGaliRouteCore::Pending(int slot)
{
    const unsigned long pc = Pc();
    //  AI(W906-INDEXZ) 20260930: a stop that bypassed the route may have run INSIDE poll foreignPoll_+1 (the monitor's
    //  output-first yield hook), so the samples stay suspect through the end of that poll (the engine route's
    //  "pollCount > now + 1", Pci1203MotorRouteNoteForeignStop). pc < foreignPoll_: the monitor restarted.
    if (haveForeign_ && pc >= foreignPoll_ && pc <= foreignPoll_ + 1ul) return true;
    bool have = haveOwn_;
    unsigned long mark = ownPoll_;
    unsigned long other = 0;
    if (io_->LastIssued(slot, other) && (!have || other > mark)) { mark = other; have = true; }
    if (!have) return false;
    //  pollCount ends a Poll, so a sample taken with pollCount == mark is from
    //  before the command. pc < mark: the monitor restarted -- every sample it
    //  has now is newer than the command.
    return pc == mark;
}

// Owner thread only (every caller checks the thread first -- D7).
TGaliRouteCore::Obs TGaliRouteCore::Observe()
{
    Obs o;
    o.cardOpen = io_->CardOpen();
    if (!o.cardOpen) { o.why = "1203 card not open, or the monitor is not polling"; return o; }
    o.slot = io_->Slot(o.why);
    if (o.slot < 0) return o;
    lastSlot_ = o.slot;
    EndHaltIfReplaced(o.slot);                      // D3
    o.valid = io_->Sample(o.slot, o.s);
    if (!o.valid) { o.why = "no valid monitor sample for the axis"; return o; }
    o.pending = Pending(o.slot);
    if (tdHeld_ && !o.pending) tdHeld_ = false;     // B: a sample newer than the DP carries the defined command position
    return o;
}

bool TGaliRouteCore::MovingOf(const Obs& o) const
{
    if (poisoned_ || halted_) return true;
    if (!o.cardOpen || o.slot < 0 || !o.valid || o.pending) return true;
    return !IdleState(o.s.state);
}

void TGaliRouteCore::NoteDone(const Obs& o)
{
    if (moveInProgress_ && !MovingOf(o)) moveInProgress_ = false;   // a fresh idle sample: golden reads the move as over
}

//  AI(W906-INDEXZ-1203) 20260930 (D4): the cooldown is per failure KEY, not per text. G04 sends "ST" and then
//  "VS0;SP0,0,0,0;" every pass while the motor power is off (csystem.cpp:16363 -> LockIndexMotorAndDoHomeProcess, and
//  aHotPlateSubstrate.cpp:1236 -> StopAllMotor(true)): with the command name in the text every line differed from the
//  one before and nothing was ever held back. The callers pass the cause without the command as the key; the
//  failure itself is latched and counted by the caller before this runs, whether the line is printed or not.
void TGaliRouteCore::Force(const std::string& key, const std::string& line)
{
    const unsigned long now = io_->NowMs();
    std::map<std::string, ForceRec>::iterator it = force_.find(key);
    if (it != force_.end() && now - it->second.ms < kForceRepeatMs) { ++it->second.held; ++heldTotal_; return; }
    if (it == force_.end()) {
        if (force_.size() >= kForceKeysMax) {       // keep the table small: forget quiet keys first
            unsigned long lost = 0;
            for (std::map<std::string, ForceRec>::iterator e = force_.begin(); e != force_.end(); ) {
                if (now - e->second.ms >= kForceRepeatMs && e->second.held == 0) force_.erase(e++);
                else ++e;
            }
            if (force_.size() >= kForceKeysMax) {
                for (std::map<std::string, ForceRec>::iterator e = force_.begin(); e != force_.end(); ++e) lost += e->second.held;
                force_.clear();
                if (lost > 0) io_->LogForce("[" + Num((long)lost) + " forced line(s) of " + Num((long)kForceKeysMax) + " other causes were held back and are not itemised]");
            }
        }
        it = force_.insert(std::make_pair(key, ForceRec())).first;
    }
    std::string out = line;
    if (it->second.held > 0)
        out += "   [" + Num((long)it->second.held) + " more line(s) with this cause in the last " + Num((long)(kForceRepeatMs / 1000ul)) + " s, held back]";
    io_->LogForce(out);
    it->second.ms = now;
    it->second.held = 0;
}

void TGaliRouteCore::Poison(const std::string& key, const std::string& why)
{
    if (!poisoned_ || poisonWhy_ != why)
        Force("poison:" + key, "FAILED -> Z1 reports its alarm (TI) and 'moving' (MG_BG) until ST/AB/VS0 or a reset: " + why);
    poisoned_ = true;
    poisonWhy_ = why;
}

void TGaliRouteCore::LatchStopFailure(const std::string& cmd, const std::string& reason)
{
    ++fault_.stopFailed;
    fault_.lastPoll = Pc();
    fault_.lastWhat = cmd + ": " + reason;
    Force("stop:" + reason, "STOP FAILED (golden WAR1635; every one is latched -- Fault(), the web diag): " + cmd + ": " + reason);
}

void TGaliRouteCore::LatchMotionFailure(const std::string& what)
{
    ++fault_.motionFailed;
    fault_.lastPoll = Pc();
    fault_.lastWhat = what;
}

void TGaliRouteCore::Issued(int slot)
{
    const unsigned long pc = Pc();
    haveOwn_ = true;
    ownPoll_ = pc;
    io_->NoteIssued(slot, pc);
}

bool TGaliRouteCore::Failed(const Pci1203CmdResult& r) const
{
    return !r.accepted || !r.issued || r.ret != 0;  // DRY (accepted && !issued) counts as failed: nothing moved
}

std::string TGaliRouteCore::Why(const Pci1203CmdResult& r) const
{
    if (!r.accepted) return "refused: " + r.why;
    if (!r.issued)   return "not issued (dry run): " + r.why;
    char b[48];
    std::snprintf(b, sizeof(b), "card returned 0x%08lX", r.ret);
    return r.why.empty() ? std::string(b) : std::string(b) + " " + r.why;
}

long TGaliRouteCore::TiByte(const Obs& o)
{
    bool alarm = poisoned_;
    if (!o.cardOpen || o.slot < 0) {
        alarm = true;
        invalidSeen_ = false;
    } else if (!o.valid) {
        const unsigned long pc = Pc();
        if (!invalidSeen_) { invalidSeen_ = true; invalidSincePoll_ = pc; }
        else if (pc - invalidSincePoll_ >= kInvalidPolls) alarm = true;
    } else {
        invalidSeen_ = false;
        if ((o.s.motionIO & kIoAlm) != 0 || (o.s.state & 0xFFu) == kStaErrorStop) alarm = true;
    }
    return alarm ? kTiZ1 : 0;
}

void TGaliRouteCore::CancelHome(const char* by)
{
    if (!homeActive_) return;
    homeActive_ = false;
    homeCancelled_ = true;                          // review #1: HomePoll answers "running" for this job for ever
    if (haltHome_) { halted_ = false; haltHome_ = false; }   // D1: a halted home that is cancelled is not resumed either
    Force("home-cancel", std::string("home job CANCELLED by ") + by + " -- golden never finishes a stopped home (case 300 waits for MG_SC==10);"
          " Gali_SingalHome stays at 250 until the next HOME sets its task back to 1");
}

// ---------------------------------------------------------------------------
//  AI(W906-INDEXZ-1203) 20260930 (D3): a halted move / home that another writer (Motor Test / Teach, the shared
//  ledger) commanded over is not ours to resume: golden -- a new command replaces the halted one. The halt ends here,
//  golden then reads the move as over (its own encoder range check decides), a halted home is cancelled.
// ---------------------------------------------------------------------------
bool TGaliRouteCore::ReplacedSince(int slot)
{
    unsigned long other = 0;
    return io_->LastIssued(slot, other) && (!haveOwn_ || other > ownPoll_);
}

void TGaliRouteCore::EndHaltIfReplaced(int slot)
{
    if (!halted_ || !ReplacedSince(slot)) return;
    const bool wasHome = haltHome_;
    const long pa = lastMove_.pa;
    halted_ = false;
    haltHome_ = false;
    moveInProgress_ = false;
    if (wasHome) CancelHome("another writer's command during the halt");
    Force("halt-replaced", std::string("the HALTED ") + (wasHome ? std::string("home") : "move to PAY=" + Num(pa)) +
          " was REPLACED: another writer (Motor Test / Teach -- the shared ledger) commanded slot " + Num(slot) +
          " after the route's last command at poll " + Num((long)ownPoll_) + " -- the halt ends without a resume");
}

// ---------------------------------------------------------------------------
//  HALT / RESUME (header). The only resume: golden's own string (DoSystem's one-shot VS/SP, GATE G22), on the owner
//  thread. AI(W906-INDEXZ-1203) 20260930 (A): the SystemStart watcher (rule (b)) is gone; `thr` is checked here.
// ---------------------------------------------------------------------------
void TGaliRouteCore::Resume(long sp, const char* why, bool thr)
{
    if (!halted_) return;
    if (!thr) {                                     // D7: nothing is sent from another thread -- refused like any motion
        const bool wasHome = haltHome_;
        halted_ = false;
        haltHome_ = false;
        if (wasHome) HomeFail("the resume string came from a thread other than the tick loop: the home is not restarted");
        else DoMove(sp, true, lastMove_.ac, lastMove_.hasAc, lastMove_.dc, lastMove_.hasDc, lastMove_.pa, true, false);
        return;
    }
    const Obs o = Observe();                        // D3: another writer's command during the halt ends it here
    if (!halted_) return;
    halted_ = false;
    ++resumes_;
    if (haltHome_) {                                // D1: golden HM continues at the new speed; the drive's home starts again
        haltHome_ = false;
        Force("resume", std::string("RESUME the halted HOME (restarted from its first step) -- ") + why);
        if (!o.cardOpen || o.slot < 0) { HomeFail("the resume could not be sent: " + o.why); return; }
        homeStartMs_ = io_->NowMs();                // a fresh 180 s: the time the machine stood paused is not the drive's
        homeCardSide_ = sawHoming_ = readySeen_ = zeroed_ = zeroWait_ = false;
        readySinceMs_ = zeroWaitMs_ = 0;
        IssueHome(o);
        return;
    }
    Force("resume", std::string("RESUME the halted move (PAY=") + Num(lastMove_.pa) + ", SP " + Num(sp) + ") -- " + why);
    DoMove(sp, true, lastMove_.ac, lastMove_.hasAc, lastMove_.dc, lastMove_.hasDc, lastMove_.pa, true, true);
}

bool TGaliRouteCore::DoMove(long sp, bool hasSp, long ac, bool hasAc, long dc, bool hasDc, long pa, bool hasPa, bool thr)
{
    halted_ = false;                                // a new BG replaces whatever move was in progress (Galil)
    haltHome_ = false;
    moveInProgress_ = false;
    if (!thr) {
        LatchMotionFailure("move from another thread");
        Poison("thread-move", "move from a thread other than the tick loop: not sent (TPci1203Control is not thread-safe)");
        return false;
    }
    if (poisoned_) { io_->Log("move not sent: the axis is marked failed (" + poisonWhy_ + ") -- clear it with ST/AB first"); return false; }
    if (!hasSp || !hasPa) {
        LatchMotionFailure("BGY without SPY/PAY");
        Poison("bg-incomplete", "BGY without SPY and PAY in the same string: speed or target unknown -- not sent");
        return false;
    }
    if (sp <= 0) {
        LatchMotionFailure("SPY=" + Num(sp));
        Poison("sp-nonpositive", "SPY=" + Num(sp) + ": a move at speed <= 0 is never sent");
        return false;
    }
    GaliRouteCaps caps;
    io_->Caps(caps);
    if (caps.jogHigh == 0) {
        LatchMotionFailure("PJogHighSpeed 0");
        Poison("jogHigh0", "PJogHighSpeed is 0: no speed ceiling for the move -- not sent");
        return false;
    }
    const double run  = (double)sp > (double)caps.jogHigh ? (double)caps.jogHigh : (double)sp;
    const double init = (double)caps.initSpeed > run ? run : (double)caps.initSpeed;
    double acc = hasAc ? (double)ac : caps.accDb;
    double dec = hasDc ? (double)dc : caps.decDb;
    if (caps.accDb > 0.0 && acc > caps.accDb) acc = caps.accDb;
    if (caps.decDb > 0.0 && dec > caps.decDb) dec = caps.decDb;
    if (acc <= 0.0 || dec <= 0.0) {
        LatchMotionFailure("acc/dec <= 0");
        Poison("accdec0", "acceleration / deceleration <= 0 -- not sent");
        return false;
    }
    const Obs o = Observe();
    if (!o.cardOpen || o.slot < 0) {
        LatchMotionFailure("move: " + o.why);
        Poison("move-unsent:" + o.why, "move not sent: " + o.why);
        return false;
    }
    const struct { Pci1203SpeedParam which; double v; } seq[4] = {
        { kSpeedInit, init }, { kSpeedRun, run }, { kSpeedAcc, acc }, { kSpeedDec, dec } };
    for (int k = 0; k < 4; ++k) {
        Pci1203Cmd c;
        c.kind = kCmdAxSetSpeed; c.axis = o.slot; c.speed = seq[k].which; c.value = seq[k].v;
        const Pci1203CmdResult r = io_->Exec(c);
        if (Failed(r)) {
            if (DidNotGoOut(r)) LatchMotionFailure("move speed parameter " + Num(k) + " " + Why(r));
            Poison("move-speed:" + Why(r), "move not sent, speed parameter " + Num(k) + " " + Why(r));
            return false;
        }
    }
    Pci1203Cmd m;
    m.kind = kCmdAxMoveAbs; m.axis = o.slot; m.value = (double)(-pa);   // PAY=n is the Galil count, -n the card position
    const Pci1203CmdResult r = io_->Exec(m);
    if (r.issued) Issued(o.slot);
    if (Failed(r)) {
        if (DidNotGoOut(r)) LatchMotionFailure("move " + Why(r));
        Poison("move:" + Why(r), "move " + Why(r));
        return false;
    }
    lastMove_.sp = sp; lastMove_.pa = pa;
    lastMove_.ac = ac; lastMove_.hasAc = hasAc;
    lastMove_.dc = dc; lastMove_.hasDc = hasDc;
    moveInProgress_ = true;
    ++moves_;
    return true;
}

void TGaliRouteCore::DoStop(bool emg, bool sp0, bool thr)
{
    const std::string nm = emg ? "AB" : sp0 ? "VS0;SP0" : "ST";
    if (!sp0) CancelHome(nm.c_str());               // review #1: ST / AB end the home job (golden case 300: MG_SC never 10); VS0;SP0 halts it (D1)
    if (!thr) {                                     // D7: nothing is sent and nothing is read from another thread
        if (sp0) CancelHome(nm.c_str());            //     a home that cannot be stopped cannot be halted either
        LatchStopFailure(nm, "from a thread other than the tick loop: not sent (TPci1203Control is not thread-safe)");
        Poison("thread-stop", nm + " from a thread other than the tick loop: not sent");
        return;
    }
    const Obs o = Observe();
    const bool movingNow = halted_ || !o.valid || o.pending || !IdleState(o.s.state);
    if (!o.cardOpen || o.slot < 0) {
        if (sp0) CancelHome(nm.c_str());
        LatchStopFailure(nm, "could not be sent: " + o.why);
        Poison("stop-unsent:" + o.why, nm + " could not be sent: " + o.why);
        return;
    }
    Pci1203Cmd c;
    c.kind = emg ? kCmdAxEmgStop : kCmdAxStop; c.axis = o.slot;
    const Pci1203CmdResult r = io_->Exec(c);
    Pci1203Cmd x;                                   // golden TMyEtherCatMotor::DecStop: ExtDrive(0) even when the stop failed
    x.kind = kCmdAxSetExtDrive; x.axis = o.slot; x.value = 0.0;   //   (WebMotorAccess.cpp Stop1203)
    const Pci1203CmdResult rx = io_->Exec(x);
    ++stops_;
    if (DidNotGoOut(r)) {                           // review #6: latched and printed, whatever the axis was doing
        const std::string reason = std::string(emg ? "EmgStop " : "Stop ") + Why(r) + (movingNow ? " while the axis may be moving" : " (axis idle)");
        LatchStopFailure(nm, reason);
        if (sp0) CancelHome(nm.c_str());            // the drive may still be homing: that home is never "done"
        if (movingNow) Poison("stop-failed:" + reason, reason);
        return;                                     // nothing else changes: the halt / failed mark stay as they were
    }
    if (DidNotGoOut(rx))
        LatchStopFailure(nm, "the ExtDrive(0) after the stop " + Why(rx));   // golden DecStop's second WAR16122; the stop itself went out
    if (poisoned_) io_->Log("stop sent: the failed mark is cleared (" + poisonWhy_ + ")");
    poisoned_ = false;
    poisonWhy_.clear();
    if (sp0 && movingNow && (moveInProgress_ || homeActive_)) {   // review #2 / D1: Galil SP0 leaves a running move or HM IN PROGRESS at speed 0
        if (!halted_) {
            halted_ = true;
            haltHome_ = homeActive_;
            if (haltHome_)
                Force("halt", "HALTED by VS0;SP0 (golden StopAllMotor): the HOME stays in progress (golden HM at speed 0) and starts again on golden's resume string (DoSystem's one-shot VS/SP after START)");
            else
                Force("halt", "HALTED by VS0;SP0 (golden StopAllMotor): the move to PAY=" + Num(lastMove_.pa) +
                      " stays in progress and resumes on golden's resume string (DoSystem's one-shot VS/SP after START)");
        }
    } else if (sp0 && homeActive_) {
        // D1: VS0;SP0 on an IDLE axis during a home job -- nothing moves (the drive finished homing, or never started):
        //     the job goes on and HomePoll decides (done / 5 s READY without HOMING).
    } else {
        halted_ = false;                            // ST / AB abort the move; VS0;SP0 on an idle axis has nothing to keep
        haltHome_ = false;
        moveInProgress_ = false;
    }
}

void TGaliRouteCore::DoServo(bool on, bool thr)
{
    if (!thr) { io_->Log(std::string(on ? "SH" : "MO") + " from a thread other than the tick loop: not sent"); return; }
    const Obs o = Observe();
    if (!o.cardOpen || o.slot < 0) { io_->Log(std::string(on ? "SH" : "MO") + " not sent: " + o.why); return; }
    if (on && o.valid && ((o.s.motionIO & kIoAlm) != 0 || (o.s.state & 0xFFu) == kStaErrorStop)) {
        Pci1203Cmd e;                               // golden MotOutputOn bAlarm (myEthercatmotor.cpp:1398-1405): reset, 100 ms, SvOn
        e.kind = kCmdAxResetError; e.axis = o.slot;
        const Pci1203CmdResult r = io_->Exec(e);
        if (Failed(r)) io_->Log("ResetError before SvOn " + Why(r));
        else {
            if (poisoned_) io_->Log("ResetError sent: the failed mark is cleared (" + poisonWhy_ + ")");
            poisoned_ = false;
            poisonWhy_.clear();
        }
        io_->SleepMs(100);
    }
    Pci1203Cmd c;
    c.kind = kCmdAxSvOn; c.axis = o.slot; c.value = on ? 1.0 : 0.0;
    const Pci1203CmdResult r = io_->Exec(c);
    if (Failed(r)) io_->Log(std::string("SvOn ") + (on ? "1 " : "0 ") + Why(r) + " (golden: the servo lamp tells; not marked failed)");
}

// ---------------------------------------------------------------------------
//  AI(W906-INDEXZ-1203) 20260930 (B): golden TestZ1SetPos (aTester_Front.cpp:314-320 = golden :291-297, callers
//  Front :588 / Rear :9988 = golden :546-552 / :7773-7803) sends sprintf(str,"DP%d,%d",Pos1,0-Pos2) -- positional,
//  field 1 (Y = MTestZ1) = minus Z1's encoder read -- to re-sync the command position with the encoder after
//  TestZ1OutRandge. It used to be dropped silently (the route only looked at DP with a Y letter). Galil DP defines the
//  axis's position; the route defines the card's COMMAND position so TDY reads v afterwards (TDY = round(-cmdPos)):
//  kCmdAxSetCmdPos value = -v, the card counterpart the card-side home already uses (HomePoll). The encoder (TPY)
//  is not touched. Refused / failed = a failed motion (latched, poison: golden's Galil command error is WAR1635).
// ---------------------------------------------------------------------------
void TGaliRouteCore::DoDefinePos(long v, bool thr)
{
    if (!thr) {
        LatchMotionFailure("DP from another thread");
        Poison("thread-dp", "DP from a thread other than the tick loop: not sent (TPci1203Control is not thread-safe)");
        return;
    }
    const Obs o = Observe();
    if (!o.cardOpen || o.slot < 0) {
        LatchMotionFailure("DP: " + o.why);
        Poison("dp-unsent:" + o.why, "DP not sent: " + o.why);
        return;
    }
    Pci1203Cmd c;
    c.kind = kCmdAxSetCmdPos; c.axis = o.slot; c.value = (double)(-v);
    const Pci1203CmdResult r = io_->Exec(c);
    if (r.issued) Issued(o.slot);                   // TD must not answer from a sample taken before it
    if (Failed(r)) {
        if (DidNotGoOut(r)) LatchMotionFailure("DP " + Why(r));
        Poison("dp:" + Why(r), "DP (define Z1's command position) " + Why(r));
        return;
    }
    lastTd_ = v;
    haveLast_ = true;
    tdHeld_ = true;
    io_->Log("DP (Y) " + Num(v) + ": Z1 command position defined (card " + Num(-v) + ") -- golden TestZ1SetPos's command/encoder re-sync");
}

bool TGaliRouteCore::Command(const char* data, long* reply)
{
    if (reply) *reply = 0;
    if (io_ == 0 || data == 0) return false;
    std::vector<GaliStmt> st;
    if (GaliSplit(data, st) == 0) return false;
    const bool thr = io_->OnOwnerThread();
    onThr_ = thr;
    //  AI(W906-INDEXZ-1203) 20260930 (A): the SystemStart resume rule (b) that stood here is removed -- it resumed a
    //  halted move on ckernel's "KS4,4,4,4;VT0.1..." (sent right after SystemStart=true, ckernel.cpp:1015-1018),
    //  before golden's safe-door check (ckernel.cpp:1038-1041). The resume is golden's string only (after the loop).
    bool claimed = false, began = false, spSet = false;
    bool hSp = false, hAc = false, hDc = false, hPa = false;
    long sp = 0, ac = 0, dc = 0, pa = 0, rv = 0;
    for (std::size_t i = 0; i < st.size(); ++i) {
        const GaliStmt& s = st[i];
        const std::string& c = s.cmd;
        const bool y = s.axes.find('Y') != std::string::npos;
        long v = 0;
        if (c == "SP" || c == "AC" || c == "DC" || c == "PA") {
            if (YValueOf(s, v)) {
                claimed = true;
                if (c == "SP")      { sp = v; hSp = true; spSet = true; }
                else if (c == "AC") { ac = v; hAc = true; }
                else if (c == "DC") { dc = v; hDc = true; }
                else                { pa = v; hPa = true; }
            }
        } else if (c == "BG") {
            if (y) {
                claimed = true;
                began = true;
                DoMove(sp, hSp, ac, hAc, dc, hDc, pa, hPa, thr);
            } else if (lmHasY_ && (s.axes.find('S') != std::string::npos || s.axes.find('T') != std::string::npos)) {
                claimed = true;
                LatchMotionFailure("BG" + s.axes + " (vector move with Y)");
                Poison("bg-vector", "BG" + s.axes + ": a Galil vector move with Y in LM -- coordinated motion is not on the 1203 route");
            }
        } else if (c == "LM") {
            lmHasY_ = y;
            if (y) { claimed = true; io_->Log("LM" + s.axes + ": the vector includes Y -- its BGS will be refused"); }
        } else if (c == "ST") {
            if (s.axes.empty() || y) { claimed = true; DoStop(false, false, thr); }
        } else if (c == "AB") {
            claimed = true;
            DoStop(true, false, thr);               // AB / AB1 abort every axis (EastSun Q5: EmgStop)
        } else if (c == "SH") {
            if (s.axes.empty() || y) { claimed = true; DoServo(true, thr); }
        } else if (c == "MO") {
            if (s.axes.empty() || y) { claimed = true; DoServo(false, thr); }
        } else if (c == "TI") {
            if (s.axes.empty() && s.fields.empty()) {
                claimed = true;
                rv = thr ? TiByte(Observe()) : kTiZ1;
            }
        } else if (c == "TS" || c == "TD" || c == "TP" || c == "TE") {
            if (s.axes == "Y") {
                claimed = true;
                if (!thr) {
                    rv = (c == "TS") ? kTsUnobservable : (c == "TD") ? lastTd_ : (c == "TP") ? lastTp_ : 0;
                } else {
                    const Obs o = Observe();
                    if (o.valid) {
                        if (!tdHeld_) lastTd_ = std::lround(-o.s.cmdPos);   // B: right after a DP, the defined value until a newer sample
                        lastTp_ = std::lround(o.s.actPos);
                        haveLast_ = true;
                    }
                    if (c == "TS")      { rv = GaliRouteTsByte(MovingOf(o), o.valid, o.s.motionIO, o.valid ? io_->OrgHome(o.slot, o.s.motionIO) : -1); NoteDone(o); }   //AI(W906-INDEXZ2) 20261002: + the tree's ORG rule (IGaliRouteIo::OrgHome), asked only for a valid sample
                    else if (c == "TD") rv = haveLast_ ? lastTd_ : 0;
                    else if (c == "TP") rv = haveLast_ ? lastTp_ : 0;
                    else { rv = 0; io_->Log("TEY (position error) has no 1203 counterpart -> 0"); }
                }
            }
        } else if (c == "MG") {
            if (s.arg.size() == 3 && s.arg[2] == 'Y') {
                const std::string op = s.arg.substr(0, 2);
                if (op == "BG") {
                    claimed = true;
                    if (!thr) rv = 1;
                    else { const Obs o = Observe(); rv = MovingOf(o) ? 1 : 0; NoteDone(o); }
                } else if (op == "MO") {
                    claimed = true;
                    if (!thr) rv = 1;
                    else { const Obs o = Observe(); rv = (o.valid && (o.s.motionIO & kIoSvon) != 0) ? 0 : 1; }
                } else if (op == "SC") {
                    claimed = true;
                    rv = 0;
                    io_->Log("MG_SCy (Galil stop code) has no 1203 counterpart -> 0");
                }
            } else if (s.arg == "XQ") {
                io_->Log("MG _XQ: the Galil on-board program (D34) does not exist on the 1203 -- golden no-card answer 0");
            }
        } else if (c == "XQ") {
            io_->Log("XQ: the Galil on-board program (D34 protection) does not exist on the 1203 -- not started");
        } else if (c == "DP") {                     // B: golden TestZ1SetPos "DP%d,%d" (field 1 = Y); "DP,,z,w" is not Y's
            if (YValueOf(s, v)) { claimed = true; DoDefinePos(v, thr); }
            else if (y) {
                claimed = true;
                LatchMotionFailure("DP" + s.axes + " without a value");
                Poison("dp-novalue", "DP" + s.axes + (s.assign ? "=" + s.value : std::string()) + ": no Y value -- not sent");
            }
        } else if (c == "DE" || c == "JG" || c == "PR" || c == "HM" || c == "FI") {
            if (y) {
                claimed = true;
                LatchMotionFailure(c + s.axes + " (Galil-only)");
                Poison("galil-only:" + c, c + s.axes + (s.assign ? "=" + s.value : std::string()) +
                       ": a Galil-only motion / origin command -- not on the 1203 route");
            }
        }
    }
    if (spSet && !began) {
        if (sp == 0) DoStop(false, true, thr);      // golden StopAllMotor "VS0;SP0,0,0,0;" (Motor/myGALILmotor.cpp:5764)
        else if (halted_) Resume(sp, "golden's resume string (an SP with a non-zero Y speed and no BG: DoSystem's one-shot VS/SP, csystem.cpp GATE G22)", thr);
        else io_->Log("SPY=" + Num(sp) + " without BGY: a Galil on-the-fly speed change -- not supported, ignored");
    }
    if (!thr && claimed)
        io_->Log(std::string("'") + data + "' from a thread other than the tick loop: motion refused, queries answered 'moving / alarm'");
    if (reply) *reply = rv;
    return claimed;
}

int TGaliRouteCore::NoteForeignStop(int slot, bool alarmSweep)
{
    if (io_ == 0) return kGaliForeignNotOurs;
    const bool thr = io_->OnOwnerThread();
    onThr_ = thr;
    int mine = lastSlot_;                           // D7: off the owner thread, the slot last resolved on it
    if (thr) {
        std::string why;
        mine = io_->Slot(why);
        if (mine >= 0) lastSlot_ = mine;
    }
    if (mine < 0 || slot != mine) return kGaliForeignNotOurs;
    haveForeign_ = true;
    foreignPoll_ = Pc();
    int ans = kGaliForeignNoted;
    const bool wasHalted = halted_;
    if (!alarmSweep) {                              // D2: an operator stop = golden "ST" on the index axis
        CancelHome("an operator stop outside the route (golden ST)");
        halted_ = false;
        haltHome_ = false;
        moveInProgress_ = false;
        ans = kGaliForeignStBookkeeping;
    } else if (!halted_) {                          // the alarm sweep, nothing halted: whatever ran is over on the card
        CancelHome("the alarm sweep's stop outside the route");
        if (moveInProgress_) {
            moveInProgress_ = false;
            ans = kGaliForeignMoveAborted;
        }
    }                                               // the alarm sweep on a HALTED move / home: it stays halted (golden sent VS0;SP0 only)
    io_->Log("slot " + Num(slot) + " stopped outside the route (" + (alarmSweep ? "the alarm sweep" : "an operator stop = golden ST") +
             "): pending through poll " + Num((long)(foreignPoll_ + 1ul)) +
             (alarmSweep && wasHalted ? "; the halted job stays halted (golden StopAllMotor sends VS0;SP0 only to the index axes)" : "") +
             (!alarmSweep && wasHalted ? "; the halt ENDS (golden ST aborts the halted move)" : "") +
             (ans != kGaliForeignNoted ? "; golden ST bookkeeping -> MovFlag=false (golden re-issues its move)" : "") +
             (poisoned_ ? "; the failed mark is KEPT (only this route's own stop clears it)" : "") +
             (thr ? "" : "; [off the owner thread: the slot / pollCount last seen on it]"));
    return ans;
}

void TGaliRouteCore::HomeFail(const std::string& why)
{
    homeActive_ = false;
    if (haltHome_) { halted_ = false; haltHome_ = false; }
    LatchMotionFailure("home: " + why);
    Poison("home:" + why, "home failed: " + why);
}

int TGaliRouteCore::HomeStart(bool homeDirection, unsigned homeHigh, unsigned homeLow, double acc, double dec)
{
    if (io_ == 0) return 9;
    const bool thr = io_->OnOwnerThread();
    onThr_ = thr;
    homeActive_ = true;
    homeCancelled_ = homeCardSide_ = sawHoming_ = readySeen_ = zeroed_ = zeroWait_ = false;
    homeStartMs_ = io_->NowMs();
    readySinceMs_ = zeroWaitMs_ = 0;
    halted_ = haltHome_ = moveInProgress_ = false;   // a home replaces any move in progress
    home_.dir = homeDirection; home_.high = homeHigh; home_.low = homeLow; home_.acc = acc; home_.dec = dec;
    if (!thr) { HomeFail("home from a thread other than the tick loop: not sent"); return 1; }
    if (poisoned_) { homeActive_ = false; io_->Log("home not sent: the axis is marked failed (" + poisonWhy_ + ")"); return 2; }
    const Obs o = Observe();
    if (!o.cardOpen || o.slot < 0) { HomeFail("not sent: " + o.why); return 3; }
    return IssueHome(o);
}

//  The drive's home with home_'s parameters -- from HomeStart and from the resume of a halted home (D1).
int TGaliRouteCore::IssueHome(const Obs& o)
{
    if (home_.high == 0) { HomeFail("Home High Speed (PHomeHighSpeed) is 0 -- not homed with whatever speed the card holds (NB2 R23 W4C-3)"); return 4; }
    const int kind = io_->DriveKind(o.slot);
    if (kind < 0) { HomeFail("drive type unknown (no DS402 / SERVOPACK identity) -- not guessed"); return 5; }
    if (kind == 1) {                                // DS402: Motor Test StartHome1203 (WebMotorAccess.cpp:1251-1269)
        const struct { Pci1203SpeedParam which; double v; } seq[4] = {
            { kSpeedInit, (double)home_.low }, { kSpeedRun, (double)home_.high }, { kSpeedAcc, home_.acc }, { kSpeedDec, home_.dec } };
        for (int k = 0; k < 4; ++k) {
            Pci1203Cmd c;
            c.kind = kCmdAxSetSpeed; c.axis = o.slot; c.speed = seq[k].which; c.value = seq[k].v;
            const Pci1203CmdResult r = io_->Exec(c);
            if (Failed(r)) { HomeFail("home speed " + Why(r)); return 6; }
        }
        Pci1203Cmd h;
        h.kind = kCmdAxHome; h.axis = o.slot;
        h.homeMode = home_.dir ? 124 : 128;
        h.dir      = home_.dir ? 1 : -1;
        const Pci1203CmdResult r = io_->Exec(h);
        if (r.issued) Issued(o.slot);
        if (Failed(r)) { HomeFail("DS402 home " + Why(r)); return 7; }
    } else {                                        // another drive: Motor Test StartHomeCardSide (:1188-1217)
        Pci1203Cmd s;
        s.kind = kCmdAxStop; s.axis = o.slot;
        Pci1203Cmd x;
        x.kind = kCmdAxSetExtDrive; x.axis = o.slot; x.value = 0.0;
        //  AI(W906-INDEXZ-1203) 20260930 (D6): the DecStop results are checked (they used to be discarded): a stop that
        //  did not reach the card is a failed stop (latched, golden DecStop's WAR16122), and no home is started after it.
        if (!(o.valid && !o.pending && (o.s.state & 0xFFu) == kStaReady)) {
            const Pci1203CmdResult r1 = io_->Exec(s);
            const Pci1203CmdResult r2 = io_->Exec(x);
            if (DidNotGoOut(r1)) LatchStopFailure("card-side home", "the DecStop of the not-READY axis: Stop " + Why(r1));
            if (DidNotGoOut(r2)) LatchStopFailure("card-side home", "the ExtDrive(0) of that DecStop " + Why(r2));
            HomeFail("card-side home: the axis is not READY (golden EtherCatMotHome case 1: DecStop, wait)");
            return 8;
        }
        const Pci1203CmdResult r1 = io_->Exec(s);   // golden: DecStop() before SetHomeSpeed
        const Pci1203CmdResult r2 = io_->Exec(x);
        if (DidNotGoOut(r1) || DidNotGoOut(r2)) {
            if (DidNotGoOut(r1)) LatchStopFailure("card-side home", "the DecStop before SetHomeSpeed: Stop " + Why(r1));
            if (DidNotGoOut(r2)) LatchStopFailure("card-side home", "the ExtDrive(0) of the DecStop before SetHomeSpeed " + Why(r2));
            HomeFail("card-side home: the DecStop before SetHomeSpeed did not reach the card (golden WAR16122) -- the home is not started");
            return 10;
        }
        const struct { int which; double v; } hs[4] = {
            { kHomeVelLow, (double)home_.low }, { kHomeVelHigh, (double)home_.high }, { kHomeAcc, home_.acc }, { kHomeDec, home_.dec } };
        for (int k = 0; k < 4; ++k) {
            Pci1203Cmd c;
            c.kind = kCmdAxSetHome; c.axis = o.slot; c.home = hs[k].which; c.value = hs[k].v;
            const Pci1203CmdResult r = io_->Exec(c);
            if (Failed(r)) { HomeFail("card home speed " + Why(r)); return 6; }
        }
        Pci1203Cmd h;
        h.kind = kCmdAxMoveHome; h.axis = o.slot;
        h.homeMode = 11;                            // MODE12_AbsSearchReFind (AdvMotDrv.h)
        h.dir      = home_.dir ? 1 : -1;
        const Pci1203CmdResult r = io_->Exec(h);
        if (r.issued) Issued(o.slot);
        if (Failed(r)) { HomeFail("card-side home " + Why(r)); return 7; }
        homeCardSide_ = true;
    }
    io_->Log(std::string("home started (") + (homeCardSide_ ? "card-side MODE12" : home_.dir ? "DS402 method 24" : "DS402 method 28") + ")");
    return 0;
}

int TGaliRouteCore::HomePoll()
{
    if (io_ == 0) return kGaliHomeFailed;
    const bool thr = io_->OnOwnerThread();
    onThr_ = thr;
    if (homeCancelled_) return kGaliHomeRunning;    // review #1: a stopped home is never "done" (golden case 300: MG_SC never becomes 10)
    if (!homeActive_) return kGaliHomeFailed;
    if (halted_ && haltHome_) return kGaliHomeRunning;   // D1: halted by VS0;SP0 -- golden HM at speed 0 is still in progress
    if (poisoned_) { homeActive_ = false; return kGaliHomeFailed; }
    if (!thr) { HomeFail("home polled from a thread other than the tick loop"); return kGaliHomeFailed; }
    const unsigned long now = io_->NowMs();
    const unsigned long elapsed = now - homeStartMs_;
    if (elapsed >= kHomeTimeoutMs) {
        homeActive_ = false;                        // our own time-out: a failure, reported once (not a cancel)
        DoStop(false, false, true);
        HomeFail("180 s without completion (stopped)");
        return kGaliHomeFailed;
    }
    const Obs o = Observe();
    if (!o.valid) {
        if (elapsed >= kHomeNoHomingMs) { HomeFail("axis state unreadable for 5 s: " + o.why); return kGaliHomeFailed; }
        return kGaliHomeRunning;
    }
    if (o.pending) return kGaliHomeRunning;
    const unsigned st = o.s.state & 0xFFu;
    if (st == kStaErrorStop) { HomeFail("ERROR_STOP during the home"); return kGaliHomeFailed; }
    if (st == kStaHoming) { sawHoming_ = true; return kGaliHomeRunning; }
    if (st != kStaReady) return kGaliHomeRunning;
    if (!sawHoming_) {
        if (!readySeen_) { readySeen_ = true; readySinceMs_ = now; return kGaliHomeRunning; }
        if (now - readySinceMs_ >= kHomeNoHomingMs) { HomeFail("READY for 5 s without ever entering HOMING"); return kGaliHomeFailed; }
        return kGaliHomeRunning;
    }
    if (homeCardSide_ && !zeroed_) {                // card-side only: 0.3 s, then SetCmdPos(0) + SetActPos(0) (TickHomes)
        if (!zeroWait_) { zeroWait_ = true; zeroWaitMs_ = now; return kGaliHomeRunning; }
        if (now - zeroWaitMs_ < kHomeZeroWaitMs) return kGaliHomeRunning;
        Pci1203Cmd zc; zc.kind = kCmdAxSetCmdPos; zc.axis = o.slot; zc.value = 0.0;
        const Pci1203CmdResult zr = io_->Exec(zc);
        Pci1203Cmd za; za.kind = kCmdAxSetActPos; za.axis = o.slot; za.value = 0.0;
        const Pci1203CmdResult ar = io_->Exec(za);
        if (Failed(zr) || Failed(ar)) {             // AI(W906-INDEXZ-1203) 20260930 (D5): was a log line and then "done"
            //  golden EtherCatMotHome raises WAR16122 here and never reaches the done step; reporting done would let
            //  golden case 400 store LastHomePos from an encoder that was never zeroed.
            HomeFail("the zeroing after the card-side home failed (golden WAR16122) -- " + Why(Failed(zr) ? zr : ar));
            return kGaliHomeFailed;
        }
        zeroed_ = true;
        Issued(o.slot);                             // done on the next sample (LastHomePos must read the zeroed encoder)
        return kGaliHomeRunning;
    }
    homeActive_ = false;
    io_->Log("home done");
    return kGaliHomeDone;
}

}  // namespace ht9045
