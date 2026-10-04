// =============================================================================
//  tests/test_gali_route_core.cpp -- AI(W906-INDEXZ-1203) 20260929
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo -- parts E (failed stops latched, review #6), F (VS0;SP0 halts and the
//  move resumes, review #2), G (a stopped home is never done, review #1), I (stops outside the route), J (the
//  installer's decision incl. "no open card", review #5), K (the build's install gate), L (the forced-line
//  cooldown) are new or rewritten; A-D, H and the TS decode are cc426093's.
//  AI(W906-INDEXZ-1203) 20260930: review round 2 of a8c3b04e -- F rewritten (A: no SystemStart resume; the only
//  resume is golden's VS/SP string, on the owner thread; the door-open / door-closed START sequences), I rewritten
//  (D2: alarm sweep vs operator stop), L rewritten (D4: per-key cooldown, G04's alternating pattern), H strengthened
//  (the weak "logs or forced not empty" is gone), new parts N (B: DP), O (D1: a halted home), P (D3: the ledger ends a
//  halt), Q (D5 / D6: card-side home zeroing and DecStop failures), M (D7: nothing owner-only off the owner thread),
//  R (E(e): a halted axis counts as moving for a stop).
//
//  The pure half of HT9050's Index Z1 Gali_* -> PCIE-1203 route
//  (EtherCAT/Pci1203GaliRouteCore.cpp) against a FAKE IO seam: no monitor, no
//  control object, no vendor call, no file. Design section 6 A-H
//  (D:\HT9045\backup\night_tools_20260928\INDEXZ_1203_DESIGN_20260929.md).
//    A  the Galil string parser, and which strings the route claims
//    B  signs and units: PAY=-n -> MoveAbs +n; TDY = -cmdPos; TPY = +actPos
//    C  speed ceilings; a speed-0 / refused / DRY / vendor-error move poisons
//    D  "moving" until a VALID sample from a poll after the last command
//       (route or Motor Test ledger); a closed card never looks done
//    E  poison clears with ST; SH on an ERROR_STOP axis resets first (+100 ms);
//       a failed stop is LATCHED and forced to the console, idle axis or not
//    F  VS0;SP0 during a move HALTS it (in progress for golden); it resumes ONLY on
//       golden's SP resume string, on the owner thread; ST / BG end it
//    G  home: DS402 sequence = Motor Test StartHome1203's; card-side sequence +
//       zeroing; unknown drive refused; HOMING->READY done; READY 5 s without
//       HOMING, ERROR_STOP, 180 s -> failed; a STOPPED (ST / AB) home never reports done
//    H  another thread: motion refused (poison), queries answered conservatively;
//       the installer's M14 row check (Direction 1 / not PCI1203 / Enable 0 / no address refused)
//    I  a stop outside the route: pending through the next poll, poison kept; the ALARM sweep keeps a halt,
//       an OPERATOR stop ends it and asks for golden ST's bookkeeping
//    J  GaliRouteInstallOk: every refusal, in order (monitor missing / card not open = refused)
//    K  GaliRouteCompiledGate() is what this build's macros say
//    L  forced lines are held back per failure KEY for 10 s and counted
//    M  off the owner thread no owner-only IO call is made
//    N  DP (golden TestZ1SetPos) defines the command position
//    O  a home halted by VS0;SP0 restarts on the resume string
//    P  another writer's ledger entry ends a halt
//    Q  card-side home: a failed zeroing / DecStop fails the home
//    R  a halted axis is "moving" for a stop
//    S  AI(W906-INDEXZ2) 20261002: TSY's HOME bit is the tree's ORG answer (IGaliRouteIo::OrgHome), not the raw bit
//  Every part can fail: each check pins a value computed by hand here.
// =============================================================================
#include "MachineType.h"                // the macros part K compares the gate with (SOFT_SIMULTE, WB_ENGINE_INDEXZ_1203, ...)
#include "EtherCAT/Pci1203GaliRouteCore.h"

#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

using namespace ht9045;

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_gali_route_core.cpp:%d]  %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
struct FakeIo : IGaliRouteIo {
    bool          open;
    unsigned long poll;
    int           slot;
    bool          valid;
    GaliRouteSample s;
    int           kind;
    GaliRouteCaps caps;
    unsigned long now;
    bool          thread;
    int           sleeps;
    bool          dry;
    int           offThreadOwnerCalls;     // part M: owner-only IO calls made while `thread` is false
    int           org;                     // AI(W906-INDEXZ2) 20261002: OrgHome's answer (-2 / -1 / 0 / 1) -- the tree's rule stands outside the route
    int           orgCalls;
    std::map<int, bool>          refuse;   // by Pci1203CmdKind
    std::map<int, unsigned long> ret;      // by Pci1203CmdKind
    std::map<int, unsigned long> ledger;   // Motor Test's g_issuedPoll stand-in
    std::vector<Pci1203Cmd>  cmds;
    std::vector<std::string> logs;
    std::vector<std::string> forced;
    FakeIo() : open(true), poll(10), slot(3), valid(true), kind(1), now(100000), thread(true), sleeps(0), dry(false), offThreadOwnerCalls(0),
               org(-2), orgCalls(0)        // -2 = no machine rule: golden EtherCAT's decode, what parts A-R were written against
    {
        s.state = 1; s.motionIO = 0x00004000ul; s.cmdPos = 0.0; s.actPos = 0.0;   // READY, servo on
        caps.jogHigh = 900000; caps.initSpeed = 100; caps.accDb = 9000000.0; caps.decDb = 9000000.0;   // machines/HT9050/Mot_Table.csv:16 (M14)
    }
    void Own() { if (!thread) ++offThreadOwnerCalls; }
    bool CardOpen() override { Own(); return open; }
    unsigned long PollCount() override { Own(); return poll; }
    int Slot(std::string& why) override { Own(); if (slot < 0) why = "fake: no slot"; return slot; }
    bool Sample(int sl, GaliRouteSample& out) override { Own(); if (!valid || sl != slot) return false; out = s; return true; }
    int DriveKind(int) override { Own(); return kind; }
    Pci1203CmdResult Exec(const Pci1203Cmd& c) override
    {
        Own();
        cmds.push_back(c);
        Pci1203CmdResult r;
        r.accepted = !refuse[(int)c.kind];
        r.issued = r.accepted && !dry;
        r.ret = r.issued ? ret[(int)c.kind] : 0ul;
        r.why = r.accepted ? (dry ? "dry run" : "") : "fake refusal";
        return r;
    }
    void Caps(GaliRouteCaps& c) override { Own(); c = caps; }
    unsigned long NowMs() override { return now; }
    void NoteIssued(int sl, unsigned long p) override { Own(); ledger[sl] = p; }
    bool LastIssued(int sl, unsigned long& p) override
    {
        Own();
        std::map<int, unsigned long>::const_iterator it = ledger.find(sl);
        if (it == ledger.end()) return false;
        p = it->second;
        return true;
    }
    bool OnOwnerThread() override { return thread; }
    void SleepMs(int) override { Own(); ++sleeps; }
    int OrgHome(int, unsigned long) override { Own(); ++orgCalls; return org; }
    void Log(const std::string& l) override { logs.push_back(l); }
    void LogForce(const std::string& l) override { forced.push_back(l); }
};

static long Cmd(TGaliRouteCore& core, const char* s, bool* claimed = 0)
{
    long r = -12345;
    const bool c = core.Command(s, &r);
    if (claimed) *claimed = c;
    return r;
}
static bool IsSpeed(const Pci1203Cmd& c, Pci1203SpeedParam w, double v, int axis)
{
    return c.kind == kCmdAxSetSpeed && c.speed == w && std::fabs(c.value - v) < 1e-9 && c.axis == axis;
}
static bool AnyContains(const std::vector<std::string>& v, const char* needle)
{
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i].find(needle) != std::string::npos) return true;
    return false;
}
static int CountKind(const std::vector<Pci1203Cmd>& v, Pci1203CmdKind k)
{
    int n = 0;
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i].kind == k) ++n;
    return n;
}
static const char* kMove = "SPY=900;ACY=1;DCY=1;PAY=-100;BGY;";
static const char* kKs   = "KS4,4,4,4;VT0.1,0.1,0.1,0.1;";          // ckernel.cpp:1016 -- right after SystemStart=true, BEFORE the door check
static const char* kSp0  = "VS0;SP0,0,0,0;";                        // golden StopAllMotor(true)
static const char* kG22  = "VS30000;SP10000,10000,10000,10000;";    // golden DoSystem's one-shot (csystem.cpp GATE G22, iGali_* defaults)

// A running move halted by VS0;SP0 at poll 11 (the card stopped it short; READY at poll 12).
static void HaltAMove(FakeIo& io, TGaliRouteCore& core)
{
    Cmd(core, kMove);                                       // issued at poll 10, target card +100
    io.poll = 11; io.s.state = 5;                           // PTP
    Cmd(core, kSp0);
    io.poll = 12; io.s.state = 1;
}

int main()
{
    // ---- A: parser ----------------------------------------------------------
    std::printf("-- A. parser / claims\n");
    {
        std::vector<GaliStmt> st;
        CHECK(GaliSplit("SPY=900;ACY=550000;DCY=550000;PAY=-1234;BGY;", st) == 5);
        CHECK(st[0].cmd == "SP" && st[0].axes == "Y" && st[0].assign && st[0].value == "900");
        CHECK(st[3].cmd == "PA" && st[3].value == "-1234");
        CHECK(st[4].cmd == "BG" && st[4].axes == "Y" && !st[4].assign);
        CHECK(GaliSplit("SP,30000,30000;AC,450000,450000;DC,450000,450000;PA,-500,-600;BGYZ;", st) == 5);
        CHECK(st[0].fields.size() == 3 && st[0].fields[0] == "" && st[0].fields[1] == "30000" && st[0].fields[2] == "30000");
        CHECK(st[4].cmd == "BG" && st[4].axes == "YZ");
        CHECK(GaliSplit("VS0;SP0,0,0,0;", st) == 2);
        CHECK(st[0].cmd == "VS" && st[0].fields.size() == 1 && st[0].fields[0] == "0");
        CHECK(st[1].cmd == "SP" && st[1].fields.size() == 4 && st[1].fields[1] == "0");
        CHECK(GaliSplit("MG_BGy", st) == 1 && st[0].cmd == "MG" && st[0].arg == "BGY");
        CHECK(GaliSplit("MG _XQ", st) == 1 && st[0].arg == "XQ");
        CHECK(GaliSplit("AB1", st) == 1 && st[0].cmd == "AB" && st[0].fields.size() == 1 && st[0].fields[0] == "1");
        CHECK(GaliSplit("ST", st) == 1 && st[0].cmd == "ST" && st[0].axes.empty() && st[0].fields.empty());
        CHECK(GaliSplit("ERB=?", st) == 1 && st[0].cmd == "ER" && st[0].axes == "Y" && st[0].value == "?");   // B == Y
        CHECK(GaliSplit("DPY=0;DEY=0;", st) == 2 && st[1].cmd == "DE" && st[1].axes == "Y");
        CHECK(GaliSplit("DP123,-456", st) == 1 && st[0].cmd == "DP" && st[0].fields.size() == 2 && st[0].fields[1] == "-456");
        CHECK(GaliSplit(";;", st) == 0);

        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        bool c = true;
        Cmd(core, "TSX", &c);                 CHECK(!c);
        Cmd(core, "MG_BGx", &c);              CHECK(!c);
        Cmd(core, "MG_BGz", &c);              CHECK(!c);   // Gali_Two_ZAxis_Move's second query: golden no-card 0
        Cmd(core, "SPX=5;PAX=7;BGX;", &c);    CHECK(!c);
        Cmd(core, "SP1000,,,1000;AC1,,,1;DC1,,,1;PA10,,,20;BGXW;", &c); CHECK(!c);   // GalilTwoY_Move: Y field empty
        Cmd(core, "E2=", &c);                 CHECK(!c);   // GetGali_Pr_Result: golden no-card 0
        Cmd(core, kKs, &c);                   CHECK(!c);   // ckernel's smoothing string: not Y's business
        Cmd(core, "DP,,5,6", &c);             CHECK(!c);   // TestZ2SetPos (aTester_Rear.cpp:294): Z / W fields only
        CHECK(io.cmds.empty());
        CHECK(Cmd(core, "TI", &c) == 0 && c);
        CHECK(Cmd(core, "MG_MOY", &c) == 0 && c);           // SVON set
        CHECK(Cmd(core, "MG_BGy", &c) == 0 && c);           // READY, nothing issued
        CHECK(io.cmds.empty());
    }

    // ---- B: move, signs, units ----------------------------------------------
    std::printf("-- B. move / signs / units\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        bool c = false;
        CHECK(Cmd(core, "SPY=900;ACY=550000;DCY=550000;PAY=-1234;BGY;", &c) == 0 && c);
        CHECK(io.cmds.size() == 5);
        CHECK(IsSpeed(io.cmds[0], kSpeedInit, 100.0, 3));
        CHECK(IsSpeed(io.cmds[1], kSpeedRun, 900.0, 3));
        CHECK(IsSpeed(io.cmds[2], kSpeedAcc, 550000.0, 3));
        CHECK(IsSpeed(io.cmds[3], kSpeedDec, 550000.0, 3));
        CHECK(io.cmds[4].kind == kCmdAxMoveAbs && io.cmds[4].axis == 3 && io.cmds[4].value == 1234.0);
        CHECK(io.ledger.count(3) == 1 && io.ledger[3] == 10ul);   // shared ledger written at the command's poll
        CHECK(core.MovesIssued() == 1 && !core.Poisoned() && core.MoveInProgress());
        io.poll = 11; io.s.cmdPos = 1234.0; io.s.actPos = 1230.0;
        CHECK(Cmd(core, "TDY") == -1234);                     // Gali_ReadPos = -TD*GR = +1234 (flow)
        CHECK(Cmd(core, "TPY") == 1230);                      // Gali_ReadEncoderPos = +TP*GR
        io.valid = false;
        CHECK(Cmd(core, "TDY") == -1234 && Cmd(core, "TPY") == 1230);   // transiently invalid -> last good value
        FakeIo io2; TGaliRouteCore c2; c2.Bind(&io2); io2.valid = false;
        CHECK(Cmd(c2, "TDY") == 0 && Cmd(c2, "TPY") == 0);   // never observed -> 0
        FakeIo io3; TGaliRouteCore c3; c3.Bind(&io3);
        Cmd(c3, "SP,30000,30000;AC,450000,450000;DC,450000,450000;PA,-500,-600;BGYZ;");
        CHECK(io3.cmds.size() == 5 && IsSpeed(io3.cmds[1], kSpeedRun, 30000.0, 3) && io3.cmds[4].value == 500.0);
    }

    // ---- C: ceilings and refusals -------------------------------------------
    std::printf("-- C. ceilings / poison\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        Cmd(core, "SPY=2000000;ACY=99999999;DCY=7;PAY=0;BGY;");
        CHECK(io.cmds.size() == 5);
        CHECK(IsSpeed(io.cmds[1], kSpeedRun, 900000.0, 3));    // min(SP, PJogHighSpeed)
        CHECK(IsSpeed(io.cmds[0], kSpeedInit, 100.0, 3));      // min(InitSpeed, run)
        CHECK(IsSpeed(io.cmds[2], kSpeedAcc, 9000000.0, 3));   // min(AC, Acc DB)
        CHECK(IsSpeed(io.cmds[3], kSpeedDec, 7.0, 3));
        CHECK(io.cmds[4].value == 0.0);                        // -0
        FakeIo z; TGaliRouteCore cz; cz.Bind(&z);
        Cmd(cz, "SPY=0;ACY=1;DCY=1;PAY=-5;BGY;");
        CHECK(z.cmds.empty() && cz.Poisoned());                // never a 0-speed move
        CHECK(Cmd(cz, "MG_BGY") == 1 && Cmd(cz, "TI") == 0x08);
        CHECK(cz.Fault().motionFailed == 1 && cz.Fault().stopFailed == 0);
        FakeIo sp; TGaliRouteCore csp; csp.Bind(&sp); sp.s.cmdPos = 50.0; sp.s.actPos = 50.0;
        Cmd(csp, "SPY=10;ACY=1;DCY=1;PAY=-5;BGY;");
        CHECK(IsSpeed(sp.cmds[0], kSpeedInit, 10.0, 3));       // init = min(InitSpeed 100, run 10)
        FakeIo nopa; TGaliRouteCore cnp; cnp.Bind(&nopa);
        Cmd(cnp, "BGY");
        CHECK(nopa.cmds.empty() && cnp.Poisoned());            // no SPY/PAY in the same string
    }

    // ---- D: pending ---------------------------------------------------------
    std::printf("-- D. stale samples never look done\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        Cmd(core, "SPY=900;ACY=1;DCY=1;PAY=-100;BGY;");        // issued at poll 10
        CHECK(Cmd(core, "MG_BGY") == 1);                       // same poll, READY sample is from before the command
        CHECK((Cmd(core, "TSY") & 0x80) != 0);
        io.poll = 11; io.s.state = 5;                          // PTP
        CHECK(Cmd(core, "MG_BGY") == 1);
        io.poll = 12; io.s.state = 1;                          // READY, fresh
        CHECK(Cmd(core, "MG_BGY") == 0);
        CHECK(!core.MoveInProgress());                         // golden has read the move as over
        CHECK((Cmd(core, "TSY") & 0x80) == 0);
        io.open = false; io.poll = 13;                         // detach: pollCount advanced, card closed
        CHECK(Cmd(core, "MG_BGY") == 1 && Cmd(core, "TI") == 0x08);
        io.open = true;
        io.ledger[3] = 13;                                     // Motor Test commanded this axis at poll 13
        CHECK(Cmd(core, "MG_BGY") == 1);
        io.poll = 14;
        CHECK(Cmd(core, "MG_BGY") == 0);
        io.poll = 2;                                           // monitor restarted: counter below the mark
        CHECK(Cmd(core, "MG_BGY") == 0);
        io.valid = false;                                      // card open, slot known, no valid sample
        CHECK(Cmd(core, "MG_BGY") == 1);
        CHECK(Cmd(core, "TI") == 0);                           // first poll without a sample
        io.poll = 4; CHECK(Cmd(core, "TI") == 0);
        io.poll = 5; CHECK(Cmd(core, "TI") == 0x08);           // 3 polls -> alarm
        io.valid = true;
        CHECK(Cmd(core, "TI") == 0);
        io.s.motionIO |= 0x2ul;  CHECK(Cmd(core, "TI") == 0x08);   // ALM
        io.s.motionIO &= ~0x2ul; io.s.state = 3; CHECK(Cmd(core, "TI") == 0x08);   // ERROR_STOP
        io.s.state = 1;
        io.slot = -1; CHECK(Cmd(core, "TI") == 0x08 && Cmd(core, "MG_BGY") == 1);   // slot unresolved
    }

    // ---- E: poison, failed stops --------------------------------------------
    std::printf("-- E. refused / DRY / vendor error -> poison until ST; failed stops are latched (review #6)\n");
    {
        const Pci1203CmdKind k = kCmdAxMoveAbs;
        for (int mode = 0; mode < 3; ++mode) {
            FakeIo io; TGaliRouteCore core; core.Bind(&io);
            if (mode == 0) io.refuse[(int)k] = true;
            if (mode == 1) io.dry = true;
            if (mode == 2) io.ret[(int)k] = 0x80001234ul;
            Cmd(core, "SPY=900;ACY=1;DCY=1;PAY=-100;BGY;");
            io.poll = 20;                                      // any number of fresh samples later
            CHECK(core.Poisoned());
            CHECK(Cmd(core, "MG_BGY") == 1 && Cmd(core, "TI") == 0x08);
            CHECK(core.Fault().motionFailed == (mode == 1 ? 0ul : 1ul));   // DRY is not a failure of the card (not latched), refused / vendor error are
            const std::size_t n = io.cmds.size();
            Cmd(core, "SPY=900;ACY=1;DCY=1;PAY=-200;BGY;");    // a move while poisoned is not sent
            CHECK(io.cmds.size() == n);
            io.dry = false; io.refuse.clear(); io.ret.clear();
            Cmd(core, "ST");
            CHECK(!core.Poisoned());
            CHECK(io.cmds.size() == n + 2 && io.cmds[n].kind == kCmdAxStop && io.cmds[n + 1].kind == kCmdAxSetExtDrive && io.cmds[n + 1].value == 0.0);
            CHECK(Cmd(core, "MG_BGY") == 0 && Cmd(core, "TI") == 0);
        }
        // SH on ERROR_STOP: ResetError, 100 ms, SvOn 1; a successful reset clears the mark
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        Cmd(core, "SPY=0;PAY=1;BGY;");                         // poison
        io.s.state = 3;
        Cmd(core, "SH");
        CHECK(io.cmds.size() == 2 && io.cmds[0].kind == kCmdAxResetError && io.cmds[1].kind == kCmdAxSvOn && io.cmds[1].value == 1.0);
        CHECK(io.sleeps == 1 && !core.Poisoned());
        io.cmds.clear(); io.s.state = 1;
        Cmd(core, "SHY");
        CHECK(io.cmds.size() == 1 && io.cmds[0].kind == kCmdAxSvOn && io.sleeps == 1);   // no alarm: SvOn only
        io.cmds.clear();
        Cmd(core, "SHX");                                      // not Y
        CHECK(io.cmds.empty());
        Cmd(core, "AB1"); Cmd(core, "MOY");
        CHECK(io.cmds.size() == 3 && io.cmds[0].kind == kCmdAxEmgStop && io.cmds[1].kind == kCmdAxSetExtDrive && io.cmds[2].kind == kCmdAxSvOn && io.cmds[2].value == 0.0);
        CHECK(Cmd(core, "MG_MOY") == 0);                       // sample still says SVON (stale is fine for a lamp)
        io.s.motionIO = 0;
        CHECK(Cmd(core, "MG_MOY") == 1);
        // review #6: a refused stop on an IDLE axis is latched and forced (it used to be one rate-limited Log line),
        // not poison; while moving it is latched AND poison.
        FakeIo st; TGaliRouteCore cs; cs.Bind(&st);
        st.refuse[(int)kCmdAxStop] = true;
        Cmd(cs, "ST");
        CHECK(!cs.Poisoned());
        CHECK(cs.Fault().stopFailed == 1ul && cs.Fault().lastPoll == 10ul && cs.Fault().lastWhat.find("Stop refused") != std::string::npos);
        CHECK(st.forced.size() == 1 && st.forced[0].find("STOP FAILED") != std::string::npos);
        CHECK(!GaliRouteFaultText(cs.Fault()).empty() && GaliRouteFaultText(GaliRouteFault()).empty());
        st.s.state = 5;
        Cmd(cs, "ST");
        CHECK(cs.Poisoned() && cs.Fault().stopFailed == 2ul);
        // the stop went out but its ExtDrive(0) failed: latched, the stop's effect stands (no poison)
        FakeIo xe; TGaliRouteCore cx; cx.Bind(&xe);
        xe.ret[(int)kCmdAxSetExtDrive] = 0x80000001ul;
        Cmd(cx, "ST");
        CHECK(!cx.Poisoned() && cx.Fault().stopFailed == 1ul && cx.Fault().lastWhat.find("ExtDrive(0)") != std::string::npos);
        // the card closed: the stop cannot be sent -> latched + poison
        FakeIo cl; TGaliRouteCore cc; cc.Bind(&cl);
        cl.open = false;
        Cmd(cc, kSp0);
        CHECK(cl.cmds.empty() && cc.Poisoned() && cc.Fault().stopFailed == 1ul);
    }

    // ---- F: VS0;SP0 halts; ONLY golden's resume string resumes (review #2; round 2 A) -----------
    std::printf("-- F. VS0;SP0 halts a running move; only golden's VS/SP string resumes it, never START itself\n");
    {
        // F1: the halt, and every string that is not golden's resume string leaves it halted
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        Cmd(core, kMove);
        io.poll = 11; io.s.state = 5;
        io.cmds.clear();
        Cmd(core, kSp0);                                       // golden StopAllMotor(true)
        CHECK(io.cmds.size() == 2 && io.cmds[0].kind == kCmdAxStop && io.cmds[1].kind == kCmdAxSetExtDrive);
        CHECK(core.Halted() && core.MoveInProgress());
        io.poll = 12; io.s.state = 1;                          // the card stopped the axis short: READY, fresh
        const char* notResume[] = { "MG_BGY", "TSY", "TDY", "TPY", "TI", "MG_MOY", kKs, "SPY=0", "SHY", kSp0, "MG_BGy", "TEY" };
        for (unsigned k = 0; k < sizeof(notResume) / sizeof(notResume[0]); ++k) {
            ++io.poll;
            Cmd(core, notResume[k]);
            CHECK(core.Halted() && core.Resumes() == 0);
        }
        CHECK(CountKind(io.cmds, kCmdAxMoveAbs) == 0);
        CHECK(Cmd(core, "MG_BGY") == 1);                       // Galil: still in progress at speed 0
        // A: START with the DOOR OPEN -- ckernel.cpp:1015-1041: SystemStart=true, the KS string, the door check fails,
        //    SystemStart=false, StopAllMotor() (VS0;SP0 again); DoSystem's G22 never sees SystemStart -> no VS/SP.
        {
            FakeIo d; TGaliRouteCore cd; cd.Bind(&d);
            HaltAMove(d, cd);
            const std::size_t n = d.cmds.size();
            Cmd(cd, kKs);                                      // ckernel.cpp:1016-1017
            Cmd(cd, kSp0);                                     // ckernel.cpp:1041 StopAllMotor()
            d.poll = 13;
            for (int k = 0; k < 5; ++k) { ++d.poll; Cmd(cd, "MG_BGY"); Cmd(cd, "TSY"); }   // the paused state machines keep asking
            CHECK(cd.Halted() && cd.Resumes() == 0 && CountKind(d.cmds, kCmdAxMoveAbs) == 1);
            CHECK(CountKind(std::vector<Pci1203Cmd>(d.cmds.begin() + (long)n, d.cmds.end()), kCmdAxMoveAbs) == 0);   // nothing moved after the halt
        }
        // A: START with the DOOR CLOSED -- the KS string, the arm state machines query Z1 (no resume), then DoSystem's
        //    G22 sends golden's VS/SP: THAT resumes, at ITS speed, to the old target.
        {
            FakeIo d; TGaliRouteCore cd; cd.Bind(&d);
            HaltAMove(d, cd);
            Cmd(cd, kKs);
            d.poll = 13;
            CHECK(Cmd(cd, "MG_BGY") == 1 && cd.Resumes() == 0 && CountKind(d.cmds, kCmdAxMoveAbs) == 1);
            d.cmds.clear();
            Cmd(cd, kG22);                                     // csystem.cpp GATE G22 (golden :4669-4670)
            CHECK(!cd.Halted() && cd.Resumes() == 1 && cd.MoveInProgress());
            CHECK(d.cmds.size() == 5 && IsSpeed(d.cmds[1], kSpeedRun, 10000.0, 3) && d.cmds[4].kind == kCmdAxMoveAbs && d.cmds[4].value == 100.0);
            CHECK(Cmd(cd, "MG_BGY") == 1);                     // pending (poll 13 == the resume's poll)
            d.poll = 14; d.s.state = 5; CHECK(Cmd(cd, "MG_BGY") == 1);
            d.poll = 15; d.s.state = 1; CHECK(Cmd(cd, "MG_BGY") == 0 && d.cmds.size() == 5);   // done, no second resume
            Cmd(cd, kG22);                                     // a second START's one-shot with nothing halted: logged, nothing sent
            CHECK(d.cmds.size() == 5 && AnyContains(d.logs, "without BGY"));
        }
        // A: the resume string from ANOTHER thread is refused (was: Resume hard-wired thr=true)
        {
            FakeIo d; TGaliRouteCore cd; cd.Bind(&d);
            HaltAMove(d, cd);
            const std::size_t n = d.cmds.size();
            d.thread = false;
            Cmd(cd, kG22);
            CHECK(d.cmds.size() == n && CountKind(d.cmds, kCmdAxMoveAbs) == 1);
            CHECK(cd.Poisoned() && cd.Fault().motionFailed == 1ul && cd.Resumes() == 0);
        }
        // ST ends a halt: nothing resumes afterwards
        FakeIo s2; TGaliRouteCore c2; c2.Bind(&s2);
        HaltAMove(s2, c2);
        Cmd(c2, "ST");
        CHECK(!c2.Halted() && !c2.MoveInProgress());
        const std::size_t n2 = s2.cmds.size();
        Cmd(c2, kG22);
        CHECK(Cmd(c2, "MG_BGY") == 0 && s2.cmds.size() == n2 && c2.Resumes() == 0);
        // a new BG ends a halt (the new move replaces it)
        FakeIo b; TGaliRouteCore cb; cb.Bind(&b);
        Cmd(cb, kMove);
        b.poll = 11; b.s.state = 5;
        Cmd(cb, kSp0);
        Cmd(cb, "SPY=900;ACY=1;DCY=1;PAY=-300;BGY;");
        CHECK(!cb.Halted() && cb.Resumes() == 0 && b.cmds.back().value == 300.0);
        // VS0;SP0 with nothing in progress: a plain stop, no halt (idle; or the move already read as done)
        FakeIo i; TGaliRouteCore ci; ci.Bind(&i);
        Cmd(ci, kSp0);
        CHECK(!ci.Halted() && i.cmds.size() == 2);
        Cmd(ci, kMove);
        i.poll = 11;
        CHECK(Cmd(ci, "MG_BGY") == 0);                         // done (READY, fresh)
        Cmd(ci, kSp0);
        CHECK(!ci.Halted());
        // a speed change without BG while not halted: ignored, as before
        i.cmds.clear();
        Cmd(ci, "SPY=500");
        CHECK(i.cmds.empty());
    }

    // ---- G: home ------------------------------------------------------------
    std::printf("-- G. home (a stopped home is never done, review #1)\n");
    {
        // DS402: WebMotorAccess.cpp StartHome1203 -- Init=homeLow, Run=homeHigh, Acc/Dec=DB, then Home(124|128, +1|-1)
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        CHECK(core.HomeStart(true, 6000, 300, 9000000.0, 8000000.0) == 0);
        CHECK(io.cmds.size() == 5);
        CHECK(IsSpeed(io.cmds[0], kSpeedInit, 300.0, 3) && IsSpeed(io.cmds[1], kSpeedRun, 6000.0, 3));
        CHECK(IsSpeed(io.cmds[2], kSpeedAcc, 9000000.0, 3) && IsSpeed(io.cmds[3], kSpeedDec, 8000000.0, 3));
        CHECK(io.cmds[4].kind == kCmdAxHome && io.cmds[4].homeMode == 124 && io.cmds[4].dir == 1);
        CHECK(core.HomePoll() == kGaliHomeRunning);            // same poll
        io.poll = 11; io.s.state = 4;
        CHECK(core.HomePoll() == kGaliHomeRunning && Cmd(core, "MG_BGY") == 1);
        io.poll = 12; io.s.state = 1;
        CHECK(core.HomePoll() == kGaliHomeDone && !core.Poisoned());
        FakeIo io2; TGaliRouteCore c2; c2.Bind(&io2);
        c2.HomeStart(false, 6000, 300, 1.0, 1.0);
        CHECK(io2.cmds.back().kind == kCmdAxHome && io2.cmds.back().homeMode == 128 && io2.cmds.back().dir == -1);
        // review #1: HOMING seen, then a stop, then READY -- golden (case 300, MG_SC==10) never finishes that home
        {
            FakeIo h; TGaliRouteCore ch; ch.Bind(&h);
            ch.HomeStart(true, 6000, 300, 1.0, 1.0);
            h.poll = 11; h.s.state = 4;
            CHECK(ch.HomePoll() == kGaliHomeRunning);
            Cmd(ch, "ST");                                     // e.g. uhome's error exits / G04 LockIndexMotorAndDoHomeProcess
            CHECK(ch.HomeCancelled() && !ch.HomeActive());
            h.poll = 12; h.s.state = 1;
            CHECK(ch.HomePoll() == kGaliHomeRunning);
            h.poll = 13;
            CHECK(ch.HomePoll() == kGaliHomeRunning);
            const std::size_t n = h.cmds.size();
            h.now += 200000;                                   // past the route's own 180 s time-out: still just "running", nothing sent
            CHECK(ch.HomePoll() == kGaliHomeRunning && h.cmds.size() == n && !ch.Poisoned());
            Cmd(ch, kG22);                                     // a cancelled home is not resumed
            CHECK(h.cmds.size() == n && ch.HomePoll() == kGaliHomeRunning);
            CHECK(ch.HomeStart(true, 6000, 300, 1.0, 1.0) == 0 && !ch.HomeCancelled());   // the next HOME starts a fresh job
            h.poll = 14; h.s.state = 4; ch.HomePoll();
            h.poll = 15; h.s.state = 1;
            CHECK(ch.HomePoll() == kGaliHomeDone);
            // AB cancels it the same way (VS0;SP0 now HALTS it -- part O)
            FakeIo a; TGaliRouteCore ca; ca.Bind(&a);
            ca.HomeStart(true, 6000, 300, 1.0, 1.0);
            a.poll = 11; a.s.state = 4; ca.HomePoll();
            Cmd(ca, "AB1");
            a.poll = 12; a.s.state = 1;
            CHECK(ca.HomePoll() == kGaliHomeRunning && ca.HomeCancelled());
        }
        // card-side (another drive): not READY -> stop + refused
        FakeIo cs; TGaliRouteCore ccs; ccs.Bind(&cs); cs.kind = 0; cs.s.state = 5;
        CHECK(ccs.HomeStart(true, 6000, 300, 1.0, 1.0) != 0 && ccs.Poisoned());
        CHECK(cs.cmds.size() == 2 && cs.cmds[0].kind == kCmdAxStop && cs.cmds[1].kind == kCmdAxSetExtDrive);
        CHECK(ccs.HomePoll() == kGaliHomeFailed && ccs.Fault().stopFailed == 0ul);
        // card-side READY: Stop, ExtDrive 0, 4 x SetHome, MoveHome(11), then zeroing after 0.3 s
        FakeIo cr; TGaliRouteCore ccr; ccr.Bind(&cr); cr.kind = 0;
        CHECK(ccr.HomeStart(true, 6000, 300, 5.0, 6.0) == 0);
        CHECK(cr.cmds.size() == 7);
        CHECK(cr.cmds[2].kind == kCmdAxSetHome && cr.cmds[2].home == kHomeVelLow  && cr.cmds[2].value == 300.0);
        CHECK(cr.cmds[3].kind == kCmdAxSetHome && cr.cmds[3].home == kHomeVelHigh && cr.cmds[3].value == 6000.0);
        CHECK(cr.cmds[4].home == kHomeAcc && cr.cmds[4].value == 5.0 && cr.cmds[5].home == kHomeDec && cr.cmds[5].value == 6.0);
        CHECK(cr.cmds[6].kind == kCmdAxMoveHome && cr.cmds[6].homeMode == 11 && cr.cmds[6].dir == 1);
        cr.poll = 11; cr.s.state = 4; CHECK(ccr.HomePoll() == kGaliHomeRunning);
        cr.poll = 12; cr.s.state = 1; CHECK(ccr.HomePoll() == kGaliHomeRunning);   // 0.3 s starts
        cr.now += 299; CHECK(ccr.HomePoll() == kGaliHomeRunning && cr.cmds.size() == 7);
        cr.now += 1;   CHECK(ccr.HomePoll() == kGaliHomeRunning);
        CHECK(cr.cmds.size() == 9 && cr.cmds[7].kind == kCmdAxSetCmdPos && cr.cmds[8].kind == kCmdAxSetActPos);
        CHECK(ccr.HomePoll() == kGaliHomeRunning);             // waits for a sample newer than the zeroing
        cr.poll = 13; CHECK(ccr.HomePoll() == kGaliHomeDone);
        // unknown drive
        FakeIo uk; TGaliRouteCore cuk; cuk.Bind(&uk); uk.kind = -1;
        CHECK(cuk.HomeStart(true, 6000, 300, 1.0, 1.0) != 0 && uk.cmds.empty() && cuk.Poisoned());
        // Home High Speed 0
        FakeIo hz; TGaliRouteCore chz; chz.Bind(&hz);
        CHECK(chz.HomeStart(true, 0, 300, 1.0, 1.0) != 0 && hz.cmds.empty());
        // READY 5 s without HOMING
        FakeIo nh; TGaliRouteCore cnh; cnh.Bind(&nh);
        cnh.HomeStart(true, 6000, 300, 1.0, 1.0);
        nh.poll = 11; CHECK(cnh.HomePoll() == kGaliHomeRunning);
        nh.now += 4999; CHECK(cnh.HomePoll() == kGaliHomeRunning);
        nh.now += 1;    CHECK(cnh.HomePoll() == kGaliHomeFailed && cnh.Poisoned());
        // ERROR_STOP
        FakeIo es; TGaliRouteCore ces; ces.Bind(&es);
        ces.HomeStart(true, 6000, 300, 1.0, 1.0);
        es.poll = 11; es.s.state = 3;
        CHECK(ces.HomePoll() == kGaliHomeFailed && ces.Poisoned());
        // 180 s: the route's own time-out is a failure reported once (a stop is sent, the axis stays failed)
        FakeIo to; TGaliRouteCore cto; cto.Bind(&to);
        cto.HomeStart(true, 6000, 300, 1.0, 1.0);
        to.poll = 11; to.s.state = 4; CHECK(cto.HomePoll() == kGaliHomeRunning);
        to.now += 180000;
        const std::size_t n = to.cmds.size();
        CHECK(cto.HomePoll() == kGaliHomeFailed && cto.Poisoned() && !cto.HomeCancelled());
        CHECK(to.cmds.size() == n + 2 && to.cmds[n].kind == kCmdAxStop);
    }

    // ---- H: thread ----------------------------------------------------------
    std::printf("-- H. another thread\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        io.thread = false;
        Cmd(core, "SPY=900;ACY=1;DCY=1;PAY=-100;BGY;");
        CHECK(io.cmds.empty() && core.Poisoned());
        CHECK(core.Fault().motionFailed == 1ul && core.Fault().lastWhat == "move from another thread");   // E(c): the refused move is LATCHED
        CHECK(AnyContains(io.logs, "from a thread other than the tick loop: motion refused"));
        CHECK(Cmd(core, "MG_BGY") == 1 && Cmd(core, "TI") == 0x08 && Cmd(core, "MG_MOY") == 1 && Cmd(core, "TSY") == 0x8E);
        Cmd(core, "ST");
        CHECK(io.cmds.empty());                                // not even a stop from another thread
        CHECK(core.Fault().stopFailed == 1ul && core.Fault().lastWhat.find("ST: from a thread other than the tick loop") == 0);   // ... and that is a failed stop (review #6)
        CHECK(AnyContains(io.forced, "STOP FAILED") && AnyContains(io.forced, "ST: from a thread other than the tick loop"));
        CHECK(core.HomeStart(true, 6000, 300, 1.0, 1.0) != 0 && io.cmds.empty());
        CHECK(core.Fault().motionFailed == 2ul && core.Fault().lastWhat.find("home: home from a thread other than the tick loop") == 0);
        // the installer's M14 row check (machines/HT9050/Mot_Table.csv:16 = PCI1203, Enable 1, 14/0, Direction 0)
        std::string why;
        CHECK(GaliRouteRowOk("PCI1203", 1, 14, 0, 0, why) && why.empty());
        CHECK(!GaliRouteRowOk("PCI1203", 1, 14, 0, 1, why) && why.find("Direction 1") != std::string::npos);
        CHECK(!GaliRouteRowOk("SMC", 1, 14, 0, 0, why) && why.find("SMC") != std::string::npos);
        CHECK(!GaliRouteRowOk("PCI1203", 0, 14, 0, 0, why) && why.find("Enable 0") != std::string::npos);
        CHECK(!GaliRouteRowOk("PCI1203", 1, -1, 0, 0, why) && why.find("BoardID/Port") != std::string::npos);
    }

    // ---- I: stops outside the route (round 2 D2) -------------------------------
    std::printf("-- I. a stop that bypassed the route: the alarm sweep keeps a halt, an operator stop (golden ST) ends it\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        CHECK(core.NoteForeignStop(7, true) == kGaliForeignNotOurs);  // another slot
        CHECK(core.NoteForeignStop(3, true) == kGaliForeignNoted);    // ours, alarm sweep, nothing running
        io.poll = 12;                                                  // outside the window (10..11)
        Cmd(core, kMove);                                              // issued at poll 12
        io.poll = 13; io.s.state = 5;
        CHECK(core.NoteForeignStop(3, true) == kGaliForeignMoveAborted && !core.MoveInProgress());   // the sweep cut a RUNNING move
        io.s.state = 1;                                                // the card stopped it: READY -- but from THIS poll
        CHECK(Cmd(core, "MG_BGY") == 1);
        io.poll = 14;
        CHECK(Cmd(core, "MG_BGY") == 1);                               // the stop may have run inside poll 14: still suspect
        io.poll = 15;
        CHECK(Cmd(core, "MG_BGY") == 0);
        io.poll = 2;                                                   // monitor restarted below the mark: not suspect
        CHECK(Cmd(core, "MG_BGY") == 0);
        // the alarm path: StopAllMotor's VS0;SP0 first (halt), then W906_MotorAccessOnAlarm's sweep -> STAYS halted,
        // and golden's resume string after START resumes it
        FakeIo h; TGaliRouteCore ch; ch.Bind(&h);
        HaltAMove(h, ch);
        CHECK(ch.NoteForeignStop(3, true) == kGaliForeignNoted && ch.Halted());
        h.poll = 14;
        CHECK(Cmd(ch, "MG_BGY") == 1 && ch.Resumes() == 0);            // no resume without golden's string
        Cmd(ch, kG22);
        CHECK(ch.Resumes() == 1 && h.cmds.back().kind == kCmdAxMoveAbs && h.cmds.back().value == 100.0);
        // D2: an OPERATOR stop (Motor Test / Teach / the pci1203 page) on a halted move = golden ST: the halt ends,
        // golden's bookkeeping is asked for, and the resume string no longer moves anything
        FakeIo o; TGaliRouteCore co; co.Bind(&o);
        HaltAMove(o, co);
        CHECK(co.NoteForeignStop(3, false) == kGaliForeignStBookkeeping && !co.Halted() && !co.MoveInProgress());
        o.poll = 14;
        const std::size_t no = o.cmds.size();
        Cmd(co, kG22);
        CHECK(o.cmds.size() == no && co.Resumes() == 0);
        // ... on a running move, and on an idle axis, too (golden ST always clears the flags)
        FakeIo o2; TGaliRouteCore co2; co2.Bind(&o2);
        Cmd(co2, kMove); o2.poll = 11; o2.s.state = 5;
        CHECK(co2.NoteForeignStop(3, false) == kGaliForeignStBookkeeping && !co2.MoveInProgress());
        FakeIo o3; TGaliRouteCore co3; co3.Bind(&o3);
        CHECK(co3.NoteForeignStop(3, false) == kGaliForeignStBookkeeping);
        // the failed mark is kept (only the route's own stop clears it)
        FakeIo p; TGaliRouteCore cp; cp.Bind(&p);
        Cmd(cp, "SPY=0;PAY=1;BGY;");
        CHECK(cp.Poisoned() && cp.NoteForeignStop(3, true) == kGaliForeignNoted && cp.Poisoned());
        CHECK(cp.NoteForeignStop(3, false) == kGaliForeignStBookkeeping && cp.Poisoned());
        // a home in progress (not halted) is cancelled for good, by the sweep and by an operator stop
        for (int op = 0; op < 2; ++op) {
            FakeIo m; TGaliRouteCore cm; cm.Bind(&m);
            cm.HomeStart(true, 6000, 300, 1.0, 1.0);
            m.poll = 11; m.s.state = 4; cm.HomePoll();
            CHECK(cm.NoteForeignStop(3, op == 0) != kGaliForeignNotOurs && cm.HomeCancelled());
            m.poll = 14; m.s.state = 1;
            CHECK(cm.HomePoll() == kGaliHomeRunning);
        }
        // no slot resolvable: not ours
        FakeIo ns; TGaliRouteCore cn; cn.Bind(&ns); ns.slot = -1;
        CHECK(cn.NoteForeignStop(3, false) == kGaliForeignNotOurs);
    }

    // ---- J: the installer's decision (review #5, round 2 C) ------------------------
    std::printf("-- J. GaliRouteInstallOk\n");
    {
        GaliRouteInstallFacts ok;
        ok.linked = true; ok.controlArmed = true; ok.monitorPresent = true; ok.cardUsable = true; ok.indexMotionCard = 0; ok.haveRow = true;
        ok.cardModel = "PCI1203"; ok.enable = 1; ok.boardId = 14; ok.port = 0; ok.direction = 0;
        ok.slotHits = 1; ok.slotAmbiguous = 0; ok.z1Present = true; ok.routeTaken = false; ok.engineRowsAtAddr = 0;
        std::string why = "x";
        CHECK(GaliRouteInstallOk(ok, why) && why.empty());
        GaliRouteInstallFacts f;
        f = ok; f.linked = false;        CHECK(!GaliRouteInstallOk(f, why) && why.find("SDK") != std::string::npos);
        f = ok; f.controlArmed = false;  CHECK(!GaliRouteInstallOk(f, why) && why.find("not armed") != std::string::npos);
        f = ok; f.monitorPresent = false; CHECK(!GaliRouteInstallOk(f, why) && why.find("BEFORE Pci1203MonitorEnable") != std::string::npos);
        f = ok; f.cardUsable = false;    CHECK(!GaliRouteInstallOk(f, why) && why.find("card is not open") != std::string::npos);
        f = ok; f.indexMotionCard = 1;   CHECK(!GaliRouteInstallOk(f, why) && why.find("INDEX_MOTION_CARD=1") != std::string::npos);
        f = ok; f.haveRow = false;       CHECK(!GaliRouteInstallOk(f, why) && why.find("no M14") != std::string::npos);
        f = ok; f.direction = 1;         CHECK(!GaliRouteInstallOk(f, why) && why.find("Direction 1") != std::string::npos);
        f = ok; f.slotHits = 0;          CHECK(!GaliRouteInstallOk(f, why) && why.find("0 opened axis slot") != std::string::npos);
        f = ok; f.slotHits = 2;          CHECK(!GaliRouteInstallOk(f, why));
        f = ok; f.slotAmbiguous = 1;     CHECK(!GaliRouteInstallOk(f, why) && why.find("1 stationAmbiguous") != std::string::npos);
        f = ok; f.z1Present = false;     CHECK(!GaliRouteInstallOk(f, why) && why.find("MOT[MTestZ1]") != std::string::npos);
        f = ok; f.routeTaken = true;     CHECK(!GaliRouteInstallOk(f, why) && why.find("already installed") != std::string::npos);
        f = ok; f.engineRowsAtAddr = 1;  CHECK(!GaliRouteInstallOk(f, why) && why.find("TMyEtherCatMotor") != std::string::npos);
        f = ok; f.cardUsable = false; f.controlArmed = false;   // order: the first refusal is named
        CHECK(!GaliRouteInstallOk(f, why) && why.find("not armed") != std::string::npos);
        f = ok; f.cardUsable = false; f.monitorPresent = false; // the boot-order reason before "card not open"
        CHECK(!GaliRouteInstallOk(f, why) && why.find("BEFORE Pci1203MonitorEnable") != std::string::npos);
    }

    // ---- K: the build's install gate --------------------------------------------
    std::printf("-- K. GaliRouteCompiledGate = this build's macros\n");
    {
#if defined(SOFT_SIMULTE)
        const int expect = kGaliGateSim;
#elif defined(WB_ENGINE_INDEXZ_1203) && defined(INSTALL_1203_MONITOR) && defined(WB_PUMP_1203_CONTROL)
#if defined(WB_PUMP_1203_START_RING)
        const int expect = kGaliGateArmedBuild;
#else
        const int expect = kGaliGateNoStartRing;
#endif
#else
        const int expect = kGaliGateOff;
#endif
        CHECK(GaliRouteCompiledGate() == expect);
#if !defined(SOFT_SIMULTE) && !defined(WB_PUMP_1203_START_RING)
        CHECK(GaliRouteCompiledGate() != kGaliGateArmedBuild);  // AI(W906-INDEXZ2) 20261002: the committed tree (D2 ON, START_RING OFF -> gate 3): whatever WB_ENGINE_INDEXZ_1203 says, no START_RING = never the armed gate
#endif
        std::printf("   gate %d\n", GaliRouteCompiledGate());
    }

    // ---- L: forced-line cooldown, per failure KEY (round 2 D4) -------------------
    std::printf("-- L. forced lines: held back 10 s per cause (G04's alternating ST / VS0;SP0), every failure counted\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        io.refuse[(int)kCmdAxStop] = true;
        Cmd(core, "ST"); Cmd(core, "ST"); Cmd(core, "ST");     // G04: every tick while the motor power is off
        CHECK(core.Fault().stopFailed == 3ul && io.forced.size() == 1);
        io.now += 10000;
        Cmd(core, "ST");
        CHECK(io.forced.size() == 2 && io.forced[1].find("2 more line(s) with this cause") != std::string::npos);
        io.now += 10000;                                       // E(d): nothing was held since that line -> no count this time
        Cmd(core, "ST");
        CHECK(io.forced.size() == 3 && io.forced[2].find("more line(s)") == std::string::npos && core.Fault().stopFailed == 5ul);
        io.now += 1;
        io.open = false;                                       // a different failure is printed at once
        Cmd(core, "ST");
        CHECK(io.forced.size() >= 4 && core.Fault().stopFailed == 6ul && AnyContains(io.forced, "could not be sent"));
        // G04's REAL pattern: "ST" then "VS0;SP0,0,0,0;" every pass (csystem.cpp:16363 -> LockIndexMotorAndDoHomeProcess,
        // aHotPlateSubstrate.cpp:1236 -> StopAllMotor(true)), the card closed: one stop line + one poison line, not 2 per pass
        FakeIo g; TGaliRouteCore cg; cg.Bind(&g);
        g.open = false;
        for (int pass = 0; pass < 20; ++pass) { Cmd(cg, "ST"); Cmd(cg, kSp0); g.now += 200; }   // 20 passes in 4 s
        CHECK(cg.Fault().stopFailed == 40ul);                   // every failure latched and counted
        CHECK(g.forced.size() == 2 && AnyContains(g.forced, "STOP FAILED") && AnyContains(g.forced, "FAILED -> Z1 reports its alarm"));
        CHECK(cg.ForcedHeld() >= 38ul);
        g.now += 10000;
        Cmd(cg, kSp0);
        CHECK(g.forced.size() >= 3 && AnyContains(g.forced, "more line(s) with this cause") && cg.Fault().stopFailed == 41ul);
    }

    // ---- M: nothing owner-only off the owner thread (round 2 D7) -----------------------
    std::printf("-- M. off the owner thread: no monitor / ledger / control call at all\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        HaltAMove(io, core);                                   // some state first, on the owner thread
        core.HomeStart(true, 6000, 300, 1.0, 1.0);
        io.poll = 13; io.s.state = 4; core.HomePoll();
        const std::size_t n = io.cmds.size();
        io.thread = false;
        const char* all[] = { kMove, "ST", kSp0, "AB1", "SH", "MO", "TI", "TSY", "TDY", "TPY", "TEY", "MG_BGY", "MG_MOY", "MG_SCy",
                              "DP5,-6", "DPY=7", kG22, "SPY=500", "LMXYZW", "BGS", kKs };
        for (unsigned k = 0; k < sizeof(all) / sizeof(all[0]); ++k) Cmd(core, all[k]);
        core.HomeStart(true, 6000, 300, 1.0, 1.0);
        core.HomePoll();
        core.NoteForeignStop(3, false);
        core.NoteForeignStop(3, true);
        CHECK(io.offThreadOwnerCalls == 0 && io.cmds.size() == n);
        CHECK(core.NoteForeignStop(3, false) == kGaliForeignStBookkeeping);   // the slot last seen on the owner thread
        CHECK(core.Poisoned() && core.Fault().stopFailed >= 3ul);
    }

    // ---- N: DP -- golden TestZ1SetPos (round 2 B) -----------------------------------
    std::printf("-- N. DP defines Z1's command position (golden TestZ1SetPos \"DP%%d,%%d\", field 1 = Y)\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        io.s.cmdPos = 2000.0; io.s.actPos = 1990.0;            // command and encoder apart by 10 (TestZ1OutRandge)
        CHECK(Cmd(core, "TDY") == -2000 && Cmd(core, "TPY") == 1990);
        bool c = false;
        Cmd(core, "DP77,-1990", &c);                           // Pos1 (Y1 field) = 77, 0-Pos2 = -1990
        CHECK(c && io.cmds.size() == 1 && io.cmds[0].kind == kCmdAxSetCmdPos && io.cmds[0].axis == 3 && io.cmds[0].value == 1990.0);
        CHECK(!core.Poisoned() && io.ledger[3] == 10ul);       // a state-changing command: the ledger mark
        CHECK(Cmd(core, "TDY") == -1990);                      // same poll (the sample is from before the DP): the defined value
        CHECK(Cmd(core, "TPY") == 1990);                       // the encoder is not touched
        io.poll = 11; io.s.cmdPos = 1990.0;                    // the card now reports the new command position
        CHECK(Cmd(core, "TDY") == -1990);
        io.poll = 12; io.s.cmdPos = 1995.0;                    // later samples are read again
        CHECK(Cmd(core, "TDY") == -1995);
        io.cmds.clear();
        Cmd(core, "DPY=-300", &c);
        CHECK(c && io.cmds.size() == 1 && io.cmds[0].value == 300.0);
        FakeIo r; TGaliRouteCore cr; cr.Bind(&r);
        r.refuse[(int)kCmdAxSetCmdPos] = true;
        Cmd(cr, "DP0,-5");
        CHECK(cr.Poisoned() && cr.Fault().motionFailed == 1ul && Cmd(cr, "TI") == 0x08);
        FakeIo t; TGaliRouteCore ct; ct.Bind(&t);
        t.thread = false;
        Cmd(ct, "DP0,-5");
        CHECK(t.cmds.empty() && ct.Poisoned() && ct.Fault().motionFailed == 1ul);
    }

    // ---- O: a halted HOME (round 2 D1) ---------------------------------------------
    std::printf("-- O. VS0;SP0 during a home halts it; golden's resume string restarts it; ST cancels it\n");
    {
        FakeIo h; TGaliRouteCore ch; ch.Bind(&h);
        ch.HomeStart(true, 6000, 300, 1.0, 1.0);
        h.poll = 11; h.s.state = 4;
        CHECK(ch.HomePoll() == kGaliHomeRunning);              // HOMING seen
        Cmd(ch, kSp0);                                         // StopAllMotor(true): PAUSE / alarm / Yes-No
        CHECK(ch.HomeHalted() && !ch.HomeCancelled() && ch.HomeActive() && CountKind(h.cmds, kCmdAxStop) == 1);
        h.poll = 12; h.s.state = 1;                            // the drive stopped: READY after HOMING -- NOT done
        CHECK(ch.HomePoll() == kGaliHomeRunning && Cmd(ch, "MG_BGY") == 1);
        h.now += 200000;                                       // a long PAUSE: no 180 s time-out while halted
        CHECK(ch.HomePoll() == kGaliHomeRunning && !ch.Poisoned());
        Cmd(ch, kKs);                                          // START's first string: no restart
        CHECK(ch.HomeHalted() && CountKind(h.cmds, kCmdAxHome) == 1);
        Cmd(ch, kG22);                                         // golden's resume string: the home starts again
        CHECK(!ch.Halted() && ch.Resumes() == 1 && CountKind(h.cmds, kCmdAxHome) == 2 && ch.HomeActive());
        CHECK(ch.HomePoll() == kGaliHomeRunning);              // pending
        h.poll = 13; h.s.state = 4; CHECK(ch.HomePoll() == kGaliHomeRunning);
        h.poll = 14; h.s.state = 1; CHECK(ch.HomePoll() == kGaliHomeDone);
        // ST during a halted home cancels it (golden case 300: ST aborts HM, SC never 10)
        FakeIo s; TGaliRouteCore cs; cs.Bind(&s);
        cs.HomeStart(true, 6000, 300, 1.0, 1.0);
        s.poll = 11; s.s.state = 4; cs.HomePoll();
        Cmd(cs, kSp0);
        Cmd(cs, "ST");
        CHECK(cs.HomeCancelled() && !cs.Halted());
        Cmd(cs, kG22);
        s.poll = 12; s.s.state = 1;
        CHECK(CountKind(s.cmds, kCmdAxHome) == 1 && cs.HomePoll() == kGaliHomeRunning);
        // VS0;SP0 when the drive has already finished homing (READY after HOMING, fresh): nothing to halt, the job completes
        FakeIo d; TGaliRouteCore cd; cd.Bind(&d);
        cd.HomeStart(true, 6000, 300, 1.0, 1.0);
        d.poll = 11; d.s.state = 4; cd.HomePoll();
        d.poll = 12; d.s.state = 1;
        Cmd(cd, kSp0);
        CHECK(!cd.Halted() && cd.HomePoll() == kGaliHomeDone);
        // an alarm sweep's stop keeps a halted home halted; an operator stop cancels it
        FakeIo w; TGaliRouteCore cw; cw.Bind(&w);
        cw.HomeStart(true, 6000, 300, 1.0, 1.0);
        w.poll = 11; w.s.state = 4; cw.HomePoll();
        Cmd(cw, kSp0);
        CHECK(cw.NoteForeignStop(3, true) == kGaliForeignNoted && cw.HomeHalted());
        CHECK(cw.NoteForeignStop(3, false) == kGaliForeignStBookkeeping && cw.HomeCancelled() && !cw.Halted());
    }

    // ---- P: another writer's command ends a halt (round 2 D3) -------------------------------
    std::printf("-- P. a ledger entry newer than the route's last command ends a halt\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        HaltAMove(io, core);                                   // route's last command at poll 10
        io.ledger[3] = 10;                                     // the route's own mark: not another writer
        Cmd(core, "MG_BGY");
        CHECK(core.Halted());
        io.ledger[3] = 12;                                     // Motor Test moved Z1 during the halt (its move to 5000)
        io.poll = 13;
        Cmd(core, "MG_BGY");
        CHECK(!core.Halted() && !core.MoveInProgress() && AnyContains(io.forced, "REPLACED"));
        const std::size_t n = io.cmds.size();
        Cmd(core, kG22);
        CHECK(io.cmds.size() == n && core.Resumes() == 0);     // was: resumes=1, MoveAbs to the old target
        // the same when the resume string itself is the first route call after the other writer's command
        FakeIo q; TGaliRouteCore cq; cq.Bind(&q);
        HaltAMove(q, cq);
        q.ledger[3] = 12;
        const std::size_t nq = q.cmds.size();
        Cmd(cq, kG22);
        CHECK(q.cmds.size() == nq && cq.Resumes() == 0 && !cq.Halted());
        // a halted HOME replaced the same way is cancelled
        FakeIo hh; TGaliRouteCore chh; chh.Bind(&hh);
        chh.HomeStart(true, 6000, 300, 1.0, 1.0);
        hh.poll = 11; hh.s.state = 4; chh.HomePoll();
        Cmd(chh, kSp0);
        hh.ledger[3] = 11;
        CHECK(chh.HomePoll() == kGaliHomeRunning);             // halted: HomePoll does not look
        Cmd(chh, "MG_BGY");
        CHECK(chh.HomeCancelled() && !chh.Halted());
    }

    // ---- Q: card-side home failures (round 2 D5 / D6) ------------------------------------
    std::printf("-- Q. card-side home: a failed zeroing fails the home; a failed DecStop before it is latched and nothing starts\n");
    {
        // D5: the zeroing after the card-side home fails -> the home FAILS (was: WAR16122 log, then "done")
        FakeIo z; TGaliRouteCore cz; cz.Bind(&z); z.kind = 0;
        CHECK(cz.HomeStart(true, 6000, 300, 5.0, 6.0) == 0);
        z.poll = 11; z.s.state = 4; cz.HomePoll();
        z.poll = 12; z.s.state = 1; cz.HomePoll();             // 0.3 s starts
        z.refuse[(int)kCmdAxSetCmdPos] = true;
        z.now += 300;
        CHECK(cz.HomePoll() == kGaliHomeFailed && cz.Poisoned() && cz.Fault().motionFailed == 1ul);
        z.poll = 13;
        CHECK(cz.HomePoll() == kGaliHomeFailed);               // never "done" afterwards
        FakeIo za; TGaliRouteCore cza; cza.Bind(&za); za.kind = 0;
        cza.HomeStart(true, 6000, 300, 5.0, 6.0);
        za.poll = 11; za.s.state = 4; cza.HomePoll();
        za.poll = 12; za.s.state = 1; cza.HomePoll();
        za.ret[(int)kCmdAxSetActPos] = 0x80000002ul;           // the encoder half fails
        za.now += 300;
        CHECK(cza.HomePoll() == kGaliHomeFailed && cza.Poisoned());
        // D6: the DecStop before SetHomeSpeed is refused -> a failed stop latched, the home not started
        FakeIo s; TGaliRouteCore cs; cs.Bind(&s); s.kind = 0;
        s.refuse[(int)kCmdAxStop] = true;
        CHECK(cs.HomeStart(true, 6000, 300, 5.0, 6.0) == 10);
        CHECK(cs.Fault().stopFailed == 1ul && CountKind(s.cmds, kCmdAxSetHome) == 0 && CountKind(s.cmds, kCmdAxMoveHome) == 0);
        CHECK(cs.Poisoned() && cs.HomePoll() == kGaliHomeFailed);
        FakeIo x; TGaliRouteCore cx; cx.Bind(&x); x.kind = 0;
        x.ret[(int)kCmdAxSetExtDrive] = 0x80000003ul;          // its ExtDrive(0) fails
        CHECK(cx.HomeStart(true, 6000, 300, 5.0, 6.0) == 10 && cx.Fault().stopFailed == 1ul && CountKind(x.cmds, kCmdAxMoveHome) == 0);
        // ... and on the not-READY path the refused stop is latched too
        FakeIo nr; TGaliRouteCore cnr; cnr.Bind(&nr); nr.kind = 0; nr.s.state = 5;
        nr.refuse[(int)kCmdAxStop] = true;
        CHECK(cnr.HomeStart(true, 6000, 300, 5.0, 6.0) == 8 && cnr.Fault().stopFailed == 1ul);
    }

    // ---- R: a halted axis is "moving" for a stop (E(e)) ---------------------------------------
    std::printf("-- R. while halted, a refused stop poisons and a repeated VS0;SP0 keeps the halt\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        HaltAMove(io, core);                                   // READY, fresh, not pending
        io.poll = 13;
        Cmd(core, kSp0);                                       // G04 / every alarm: VS0;SP0 again on the halted axis
        CHECK(core.Halted() && core.MoveInProgress());
        io.refuse[(int)kCmdAxStop] = true;
        io.poll = 14;
        Cmd(core, kSp0);
        CHECK(core.Poisoned() && core.Fault().lastWhat.find("while the axis may be moving") != std::string::npos);
    }

    // ---- S: ORG polarity (AI(W906-INDEXZ2) 20261002) -------------------------------------------------
    //  The HOME bit of TSY (golden Gali_ScanMotStatus: Led[iHomeLed] = !(TS & 0x02)) is IGaliRouteIo::OrgHome's answer --
    //  the tree's rule (WebMotorAccess.cpp MotorAccessTeachHomeLed through the engine hook; on HT9050 the ORG bit read by
    //  the axis's Mot_Table SensorType, AI(W906-HT9050-ORG-ST)) -- whatever the raw ORG bit is; only -2 (no machine rule)
    //  reads the bit as golden EtherCAT does. Before 20261002 the route always read "ORG bit set = at home".
    std::printf("-- S. TSY's HOME bit = the tree's ORG answer (1 home; 0 / -1 not home) whatever the bit; -2 = golden EtherCAT's decode\n");
    {
        FakeIo io; TGaliRouteCore core; core.Bind(&io);
        const unsigned long lo = 0x00004000ul, hi = 0x00004000ul | 0x10ul;   // SVON, ORG bit low / high
        io.org = 1;  io.s.motionIO = lo; CHECK((Cmd(core, "TSY") & 0x02) == 0);   // at home (M14: SensorType 1, bit low)
                     io.s.motionIO = hi; CHECK((Cmd(core, "TSY") & 0x02) == 0);   // at home (an axis with SensorType 0, bit high)
        io.org = 0;  io.s.motionIO = lo; CHECK((Cmd(core, "TSY") & 0x02) != 0);   // not at home
                     io.s.motionIO = hi; CHECK((Cmd(core, "TSY") & 0x02) != 0);
        io.org = -1; io.s.motionIO = lo; CHECK((Cmd(core, "TSY") & 0x02) != 0);   // unknown -> not at home (fail-closed)
                     io.s.motionIO = hi; CHECK((Cmd(core, "TSY") & 0x02) != 0);
        CHECK(io.orgCalls == 6);
        io.org = -2; io.s.motionIO = hi; CHECK((Cmd(core, "TSY") & 0x02) == 0);   // no machine rule: golden EtherCAT's decode, bit set = at home
                     io.s.motionIO = lo; CHECK((Cmd(core, "TSY") & 0x02) != 0);
        const int n = io.orgCalls;
        io.valid = false;                                      // no valid sample: 0x8E, the rule is not asked
        CHECK(Cmd(core, "TSY") == 0x8E && io.orgCalls == n);
        io.valid = true; io.thread = false;                    // off the owner thread: 0x8E and nothing owner-only is called (D7)
        CHECK(Cmd(core, "TSY") == 0x8E && io.orgCalls == n && io.offThreadOwnerCalls == 0);
        io.thread = true; io.org = 1; io.s.motionIO = 0x00004000ul | 0x04ul;      // LMT+ on, at home, idle
        CHECK(Cmd(core, "TSY") == 0x08);                       // the limit / in-position bits do not depend on the rule
    }

    // ---- TS decode ------------------------------------------------------------
    std::printf("-- TS byte\n");
    {
        CHECK(GaliRouteTsByte(false, true, 0x10ul | 0x04ul, -2) == 0x08);   // ORG + LMT+: CW(LMT-) off, CCW on, HOME on (golden EtherCAT decode)
        CHECK(GaliRouteTsByte(true, true, 0ul, -2) == 0x8E);
        CHECK(GaliRouteTsByte(false, true, 0x08ul, -2) == 0x06);            // LMT-: golden Led[iCwLed] = !(0x06 & 0x08) = true
        CHECK(GaliRouteTsByte(false, false, 0x1Cul, -2) == 0x8E);           // no sample
        CHECK(GaliRouteTsByte(false, true, 0x04ul, 1) == 0x08);             // AI(W906-INDEXZ2) 20261002: ORG bit low, the tree says at home (M14, SensorType 1)
        CHECK(GaliRouteTsByte(false, true, 0x10ul | 0x04ul, 0) == 0x0A);    // ORG HIGH, the tree says not at home
        CHECK(GaliRouteTsByte(false, true, 0x04ul, -1) == 0x0A);            // unknown -> not at home
        CHECK(GaliRouteTsByte(false, false, 0x04ul, 1) == 0x8E);            // no sample: the rule cannot help
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_checks - g_fail, g_checks);
    return g_fail ? 1 : 0;
}
