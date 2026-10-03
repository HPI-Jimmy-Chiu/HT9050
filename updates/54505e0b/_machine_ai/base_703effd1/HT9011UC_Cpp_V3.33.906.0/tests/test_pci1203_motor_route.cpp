// ===========================================================================
//  tests/test_pci1203_motor_route.cpp   (ctest: Pci1203MotorRoute)
//
//  AI(W906-ECAT-ROUTE) 20260929: the engine motor -> 1203 route BODY
//  (EtherCAT/Pci1203MotorRoute.cpp) against a fake Pci1203MotorRouteEnv:
//  no card, no monitor, no control object, no vendor call, no file.
//  Design docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md 7.4 item 2 ("the pure op ->
//  Pci1203Cmd mapping") plus the rules the body exists for:
//    M  op -> Pci1203Cmd (jog vendor->wire, values, speed / limit tables, flags)
//    S  identity: one opened slot per (station, axis), 0 / 2+ refused; DS402 kind
//    B  claim: Direction=1 refused (Q5), card closed / no slot refused, unclaimed = handle 0
//    P  pending (4.2): a state-changing command is "moving" until pollCount advances
//    Q  Q11: DRY / refused / failed motion never done until the next stop
//    D  Q7: stop + its ExtDrive(0) skipped only on a fresh READY sample and no foreign Execute;
//       speed skipped only when == last successful write == read-back
//    C  Q1: SetCmdPos / SetActPos refused on DS402 and on an unidentified drive
//    H  homing: DS402 (4 PTP speeds + Home 124/128), card-side (4 SetHome + MoveHome 11),
//       unknown (nothing), homeDone = HOMING then READY after the command, a stop cancels
//    I  InitMotor: ladder (bounded 3), EastSun's plan in golden order, Max*, ResetError
//  AI(W906-ENG1203) 20260929: review fixes (INBOX 112, three adversarial reviews of f5341505):
//    F  HIGH-1 foreign stops (Pci1203MotorRouteNoteForeignStop): home job cancelled, pending past the next
//       full Poll, stuck kept, only stops that went out, only claimed axes, nothing without the route installed
//    L  MEDIUM-1 failed stops / motions latched per axis; the console budget is per axis and never drops one
//    G  R9  the install decision this build compiled (Pci1203MotorRouteInstallGate)
//    V  R10 / R15 the live environment's conditions (Pci1203MotorRouteCardUsableOf / ExecCountOf)
//    + LOW-2 (S: stationAmbiguous refused), R2 / R5 / R6 / R7 / R8 / R19 (H), R4 (B)
//  Not a ctest of the card: EtherCAT/Pci1203Control.cpp is compiled WITHOUT HAVE_PCI1203
//  only for Pci1203GoldenInitCfgPlan and the names (same shape as test_pci1203_pure).
// ===========================================================================
#include "MachineType.h"            //AI(W906-ENG1203) 20260929: part G compares the route TU's install decision with these macros
#include "EtherCAT/Pci1203MotorRoute.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace ht9045;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_pci1203_motor_route.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

class FakeEnv : public Pci1203MotorRouteEnv {
public:
    bool card, armed, dry;
    unsigned long poll, exec, ret;
    int refuseKind;                       // this kind is refused by "Execute"
    std::vector<Pci1203AxisSample> axes;
    std::vector<Pci1203SlaveSample> slaves;
    std::vector<Pci1203Cmd> sent;
    FakeEnv() : card(true), armed(true), dry(false), poll(100), exec(0), ret(0), refuseKind(-1) {}

    bool CardUsable() override { return card; }
    bool ControlArmed() override { return armed; }
    unsigned long PollCount() override { return poll; }
    unsigned long ExecCount() override { return exec; }
    int AxisCount() override { return (int)axes.size(); }
    const Pci1203AxisSample& Axis(int i) override { return axes[(std::size_t)i]; }
    int SlaveCount() override { return (int)slaves.size(); }
    const Pci1203SlaveSample& Slave(int i) override { return slaves[(std::size_t)i]; }
    Pci1203CmdResult Execute(const Pci1203Cmd& c) override
    {
        ++exec;
        sent.push_back(c);
        Pci1203CmdResult r;
        r.accepted = ((int)c.kind != refuseKind);
        r.issued = r.accepted && !dry;
        r.ret = r.issued ? ret : 0;
        r.wouldCall = Pci1203CmdName(c.kind);
        if (!r.accepted) r.why = "fake refusal";
        return r;
    }
    int AddAxis(int station, int stationAxis, unsigned short state = 1)
    {
        Pci1203AxisSample a;
        a.valid = true; a.opened = true; a.station = station; a.stationAxis = stationAxis; a.state = state;
        axes.push_back(a);
        return (int)axes.size() - 1;
    }
    void AddSlave(int station, bool ds402, int ring = 0, const char* name = "")
    {
        Pci1203SlaveSample s;
        s.present = true; s.ring = ring; s.addr = station; s.profileValid = true; s.profile = ds402 ? 402 : 0; s.name = name;
        slaves.push_back(s);
    }
};

static FakeEnv* g_env = 0;
static const TEcatMotorRoute* R() { return Pci1203MotorRouteTable(); }
static void Fresh(FakeEnv& e)
{
    Pci1203MotorRouteResetForTest();
    Pci1203MotorRouteSetEnvForTest(&e);
    g_env = &e;
}
static unsigned long Call(int b, int p, int op, int which = 0, double v = 0.0) { return R()->call(b, p, op, which, v); }
static TEcatAxisRead Read(int b, int p, bool* ok = 0)
{
    TEcatAxisRead s;
    const bool r = R()->read(b, p, &s);
    if (ok) *ok = r;
    return s;
}

int main()
{
    Pci1203MotorRouteSetVerboseForTest(false);

    // ---- M: the pure mapping ----
    {
        Pci1203Cmd c; bool mo = false, st = false;
        CHECK(Pci1203MotorRouteMakeCmd(kEcJog, 0, 0.0, 7, c, &mo, &st) && c.kind == kCmdAxJogStart && c.dir == 1 && c.axis == 7 && mo && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcJog, 0, 1.0, 7, c, &mo, &st) && c.dir == -1);      // vendor 1 = DIRECTION_NEG -> wire -1
        CHECK(Pci1203MotorRouteMakeCmd(kEcMoveAbs, 0, 12345.0, 3, c, &mo, &st) && c.kind == kCmdAxMoveAbs && c.value == 12345.0 && mo && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcMoveRel, 0, -77.0, 3, c, &mo, &st) && c.kind == kCmdAxMoveRel && c.value == -77.0 && mo);
        CHECK(Pci1203MotorRouteMakeCmd(kEcStopDec, 0, 0.0, 3, c, &mo, &st) && c.kind == kCmdAxStop && !mo && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcStopEmg, 0, 0.0, 3, c, &mo, &st) && c.kind == kCmdAxEmgStop && !mo && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcExtDrive, 0, 1.0, 3, c, &mo, &st) && c.kind == kCmdAxSetExtDrive && c.value == 1.0 && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcSvOn, 0, 0.0, 3, c, &mo, &st) && c.kind == kCmdAxSvOn && c.value == 0.0 && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcSvOn, 0, 1.0, 3, c, &mo, &st) && c.value == 1.0);
        CHECK(Pci1203MotorRouteMakeCmd(kEcResetError, 0, 0.0, 3, c, &mo, &st) && c.kind == kCmdAxResetError && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcSetCmdPos, 0, 5.0, 3, c, &mo, &st) && c.kind == kCmdAxSetCmdPos && c.value == 5.0 && st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcSetActPos, 0, 6.0, 3, c, &mo, &st) && c.kind == kCmdAxSetActPos && c.value == 6.0 && st);
        const Pci1203SpeedParam want[11] = { kSpeedInit, kSpeedRun, kSpeedAcc, kSpeedDec, kSpeedJogInit, kSpeedJogRun,
                                             kSpeedJogAcc, kSpeedJogDec, kSpeedMaxVel, kSpeedMaxAcc, kSpeedMaxDec };
        int spOk = 0;
        for (int w = 0; w < 11; ++w)
            if (Pci1203MotorRouteMakeCmd(kEcSetSpeed, w, 10.0 + w, 2, c, &mo, &st) && c.kind == kCmdAxSetSpeed &&
                c.speed == want[w] && c.value == 10.0 + w && !mo && !st) ++spOk;
        CHECK(spOk == 11);
        CHECK(!Pci1203MotorRouteMakeCmd(kEcSetSpeed, 11, 1.0, 2, c, &mo, &st));
        CHECK(!Pci1203MotorRouteMakeCmd(kEcSetSpeed, -1, 1.0, 2, c, &mo, &st));
        CHECK(Pci1203MotorRouteMakeCmd(kEcSetLimit, kEcLimSwPel, 999.0, 2, c, &mo, &st) && c.kind == kCmdAxSetLimit && c.limit == kLimitSwPelValue && c.value == 999.0 && !st);
        CHECK(Pci1203MotorRouteMakeCmd(kEcSetLimit, kEcLimSwMel, -9.0, 2, c, &mo, &st) && c.limit == kLimitSwMelValue);
        CHECK(!Pci1203MotorRouteMakeCmd(kEcSetLimit, 7, 1.0, 2, c, &mo, &st));
        CHECK(!Pci1203MotorRouteMakeCmd(99, 0, 0.0, 2, c, &mo, &st));
        // the internal kinds this route relies on keep NO wire name (Pci1203Control.h:185)
        CHECK(Pci1203CmdFromName("pci1203.ax.setExtDrive") == kCmdNone && Pci1203CmdFromName("pci1203.ax.setCmdPos") == kCmdNone);
    }

    // ---- S: identity ----
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(1, 0); const int k = e.AddAxis(14, 0); e.AddAxis(14, 1);
        int hits = -1;
        CHECK(Pci1203MotorRouteFindSlot(e, 14, 0, &hits) == k && hits == 1);
        CHECK(Pci1203MotorRouteFindSlot(e, 20, 0, &hits) == -1 && hits == 0);
        e.AddAxis(14, 0);                                                    // a twin: 2 opened slots
        CHECK(Pci1203MotorRouteFindSlot(e, 14, 0, &hits) == -1 && hits == 2);
        e.axes.back().opened = false;                                        // not opened = not counted
        CHECK(Pci1203MotorRouteFindSlot(e, 14, 0, &hits) == k && hits == 1);
        CHECK(Pci1203MotorRouteDriveKind(e, k) == -1);                       // no identity at all -> cannot tell
        e.AddSlave(14, false, 1);                                            // ring 1 slave at the same address: ignored
        CHECK(Pci1203MotorRouteDriveKind(e, k) == -1);
        e.AddSlave(14, false, 0);                                            // a profile, not 402 -> another drive
        CHECK(Pci1203MotorRouteDriveKind(e, k) == 0);
        e.AddSlave(14, false, 0, "Yaskawa SERVOPACK SGDXW");                 // any SERVOPACK -> DS402
        CHECK(Pci1203MotorRouteDriveKind(e, k) == 1);
        e.slaves.clear(); e.AddSlave(14, true, 0);
        CHECK(Pci1203MotorRouteDriveKind(e, k) == 1);
        e.slaves.clear(); e.axes[(std::size_t)k].driveIsSigmaX = true;
        CHECK(Pci1203MotorRouteDriveKind(e, k) == 1);
        // LOW-2: a slot whose station is ambiguous is never a match -- not even a unique sub-axis (ChanMotorPoints.cpp FindCardAxis)
        int ambN = -1;
        CHECK(Pci1203MotorRouteFindSlot(e, 14, 0, &hits, &ambN) == k && ambN == 0);
        const int am = e.AddAxis(15, 1);
        e.axes[(std::size_t)am].stationAmbiguous = true;
        CHECK(Pci1203MotorRouteFindSlot(e, 15, 1, &hits, &ambN) == -1 && hits == 1 && ambN == 1);
        CHECK(!R()->bind(15, 1, 151, false));                                // engine motion never guesses the drive
    }

    // ---- B: claim ----
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(3, 0);
        CHECK(Call(3, 0, kEcStopDec) == (unsigned long)kEcRcNotClaimed);   // golden handle 0
        bool ok = true; Read(3, 0, &ok); CHECK(!ok);
        CHECK(!R()->bind(3, 0, 30, true));                                   // Q5 / 6B
        { bool ok4 = true; Read(3, 0, &ok4); CHECK(!ok4); }                  // R4: the refused claim left a record, NOT claimed = handle 0: no read
        CHECK(Call(3, 0, kEcStopDec) == (unsigned long)kEcRcNotClaimed);
        e.card = false;
        CHECK(!R()->bind(3, 0, 30, false));
        e.card = true;
        CHECK(!R()->bind(4, 0, 40, false));                                  // no slot
        CHECK(R()->bind(3, 0, 30, false));
        CHECK(e.sent.empty());                                               // claiming sends nothing
        TEcatAxisRead s = Read(3, 0, &ok);
        CHECK(ok && s.valid && s.state == 1 && !s.pending);
        e.armed = false;
        CHECK(Call(3, 0, kEcMoveAbs, 0, 5.0) == (unsigned long)kEcRcNoControl && e.sent.empty());
        e.armed = true; e.card = false;
        CHECK(Call(3, 0, kEcMoveAbs, 0, 5.0) == (unsigned long)kEcRcNoCard && e.sent.empty());
        Read(3, 0, &ok); CHECK(!ok);
        CHECK(Pci1203MotorRouteStatsNow().bindRefused == 3 && Pci1203MotorRouteStatsNow().bindOk == 1);
    }

    // ---- P + Q: pending and Q11 ----
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(5, 1);
        CHECK(R()->bind(5, 1, 51, false));
        CHECK(Call(5, 1, kEcMoveAbs, 0, 1000.0) == 0 && e.sent.size() == 1 && e.sent[0].kind == kCmdAxMoveAbs && e.sent[0].value == 1000.0);
        CHECK(Read(5, 1).pending);                                           // same Poll: the READY sample predates the command
        e.poll++;
        CHECK(!Read(5, 1).pending);                                          // a Poll later: the sample is the truth
        // speed writes do NOT make the axis pending (they would livelock golden's SetSpeed-then-MotorMove ticks)
        CHECK(Call(5, 1, kEcSetSpeed, kEcSpdRun, 777.0) == 0);
        CHECK(!Read(5, 1).pending);
        // Q11 DRY: accepted, not issued -> never done, even many Polls later, until a stop
        e.dry = true;
        CHECK(Call(5, 1, kEcMoveAbs, 0, 2000.0) == 0);
        e.poll += 5;
        CHECK(Read(5, 1).pending);
        CHECK(Call(5, 1, kEcStopDec) == 0);                                  // stuck axes are never skipped: the stop goes out
        CHECK(e.sent.back().kind == kCmdAxStop);
        CHECK(Read(5, 1).pending);                                           // the stop itself is pending for one Poll
        e.poll++;
        CHECK(!Read(5, 1).pending);
        e.dry = false;
        // Q11 refused
        e.refuseKind = kCmdAxMoveAbs;
        CHECK(Call(5, 1, kEcMoveAbs, 0, 3000.0) == (unsigned long)kEcRcRefused);
        e.poll += 3;
        CHECK(Read(5, 1).pending);
        e.refuseKind = -1;
        Call(5, 1, kEcStopDec); e.poll++;
        CHECK(!Read(5, 1).pending);
        // Q11 vendor error
        e.ret = 0x80005111ul;
        CHECK(Call(5, 1, kEcJog, 0, 0.0) == 0x80005111ul);
        e.ret = 0; e.poll += 3;
        CHECK(Read(5, 1).pending);
        // a stop that FAILS does not release it
        e.ret = 0x80000001ul;
        Call(5, 1, kEcStopDec); e.poll++;
        CHECK(Read(5, 1).pending);
        e.ret = 0;
        Call(5, 1, kEcStopDec); e.poll++;
        CHECK(!Read(5, 1).pending);
        // a motion refused BEFORE Execute (card gone) also never completes
        e.card = false;
        CHECK(Call(5, 1, kEcMoveRel, 0, 10.0) == (unsigned long)kEcRcNoCard);
        e.card = true; e.poll++;
        CHECK(Read(5, 1).pending);
    }

    // ---- D: Q7 skips ----
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(6, 0);
        CHECK(R()->bind(6, 0, 60, false));
        std::size_t n0 = e.sent.size();
        Call(6, 0, kEcStopDec); Call(6, 0, kEcExtDrive, 0, 0.0);          // first ever: foreign history unknown -> sent
        CHECK(e.sent.size() == n0 + 2);
        e.poll++;
        n0 = e.sent.size();
        CHECK(Call(6, 0, kEcStopDec) == 0 && Call(6, 0, kEcExtDrive, 0, 0.0) == 0);   // fresh READY, nobody else -> both skipped
        CHECK(e.sent.size() == n0);
        CHECK(Pci1203MotorRouteStatsNow().skippedStop == 1 && Pci1203MotorRouteStatsNow().skippedExtDrive == 1);
        CHECK(Call(6, 0, kEcExtDrive, 0, 1.0) == 0 && e.sent.size() == n0 + 1);        // ExtDrive(1) is never skipped
        e.poll++;
        n0 = e.sent.size();
        Call(6, 0, kEcStopDec);                                              // skipped...
        Call(6, 0, kEcSvOn, 0, 1.0);                                         // ...but another op in between
        Call(6, 0, kEcExtDrive, 0, 0.0);                                     // -> this ExtDrive(0) is NOT part of a skipped pair
        CHECK(e.sent.size() == n0 + 2);
        e.poll++;
        e.exec += 1;                                                         // someone else (Motor Test, the pci1203 page) executed
        n0 = e.sent.size();
        Call(6, 0, kEcStopDec);
        CHECK(e.sent.size() == n0 + 1);                                      // not skipped: that sample may predate their command
        e.poll++;
        e.axes[(std::size_t)k].state = 5;                                    // moving
        n0 = e.sent.size();
        Call(6, 0, kEcStopDec);
        CHECK(e.sent.size() == n0 + 1);
        e.axes[(std::size_t)k].state = 1;
        n0 = e.sent.size();
        Call(6, 0, kEcStopDec);                                              // READY, but the last command was this Poll
        CHECK(e.sent.size() == n0 + 1);
        // speed
        e.poll++;
        n0 = e.sent.size();
        Call(6, 0, kEcSetSpeed, kEcSpdRun, 5000.0);
        CHECK(e.sent.size() == n0 + 1);
        Call(6, 0, kEcSetSpeed, kEcSpdRun, 5000.0);                          // no read-back yet -> written again
        CHECK(e.sent.size() == n0 + 2);
        e.axes[(std::size_t)k].speed[kSpeedRun] = 5000.0; e.axes[(std::size_t)k].speedValid[kSpeedRun] = true;
        e.poll++;
        Call(6, 0, kEcSetSpeed, kEcSpdRun, 5000.0);                          // == last success == read-back -> skipped
        CHECK(e.sent.size() == n0 + 2 && Pci1203MotorRouteStatsNow().skippedSpeed == 1);
        Call(6, 0, kEcSetSpeed, kEcSpdRun, 6000.0);                          // a different value
        CHECK(e.sent.size() == n0 + 3);
        e.axes[(std::size_t)k].speed[kSpeedRun] = 6000.0; e.poll++;
        e.exec += 1;                                                         // foreign write since the sample
        Call(6, 0, kEcSetSpeed, kEcSpdRun, 6000.0);
        CHECK(e.sent.size() == n0 + 4);
        e.poll++;
        e.dry = true;                                                        // DRY is never a "successful write"
        Call(6, 0, kEcSetSpeed, kEcSpdAcc, 1.0); e.axes[(std::size_t)k].speed[kSpeedAcc] = 1.0; e.axes[(std::size_t)k].speedValid[kSpeedAcc] = true; e.poll++;
        Call(6, 0, kEcSetSpeed, kEcSpdAcc, 1.0);
        CHECK(e.sent.size() == n0 + 6);
        e.dry = false;
    }

    // ---- C: Q1 coordinate writes ----
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(7, 0); e.AddAxis(8, 0); e.AddAxis(9, 0);
        e.AddSlave(7, true); e.AddSlave(8, false);                           // 7 = DS402, 8 = card-side drive, 9 = unknown
        CHECK(R()->bind(7, 0, 70, false) && R()->bind(8, 0, 80, false) && R()->bind(9, 0, 90, false));
        CHECK(Call(7, 0, kEcSetCmdPos, 0, 0.0) == (unsigned long)kEcRcDs402Coord);
        CHECK(Call(7, 0, kEcSetActPos, 0, 0.0) == (unsigned long)kEcRcDs402Coord);
        CHECK(Call(9, 0, kEcSetCmdPos, 0, 0.0) == (unsigned long)kEcRcDs402Coord);
        CHECK(e.sent.empty() && Pci1203MotorRouteStatsNow().refusedCoord == 3);
        CHECK(Call(8, 0, kEcSetCmdPos, 0, 0.0) == 0 && Call(8, 0, kEcSetActPos, 0, 0.0) == 0);
        CHECK(e.sent.size() == 2 && e.sent[0].kind == kCmdAxSetCmdPos && e.sent[1].kind == kCmdAxSetActPos && e.sent[0].axis == 1);
        CHECK(Read(8, 0).pending);                                           // a coordinate write makes the position stale
    }

    // ---- H: homing ----
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(10, 0); e.AddSlave(10, true);
        CHECK(R()->bind(10, 0, 100, false));
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        //AI(W906-HOME-ASSINGLE) 20261003: the card's home family first (PAR_AxHomeVelLow / High / Acc / Dec), as the single-axis home
        //  (WebMotorAccess StartHome1203) and Advantech's examples, then the PTP seed, then Acm_AxHome -- 9 commands (was 5)
        CHECK(e.sent.size() == 9);
        CHECK(e.sent[0].kind == kCmdAxSetHome && e.sent[0].home == kHomeVelLow  && e.sent[0].value == 2000.0);
        CHECK(e.sent[1].kind == kCmdAxSetHome && e.sent[1].home == kHomeVelHigh && e.sent[1].value == 20000.0);
        CHECK(e.sent[2].kind == kCmdAxSetHome && e.sent[2].home == kHomeAcc     && e.sent[2].value == 500000.0);
        CHECK(e.sent[3].kind == kCmdAxSetHome && e.sent[3].home == kHomeDec     && e.sent[3].value == 400000.0);
        CHECK(e.sent[4].kind == kCmdAxSetSpeed && e.sent[4].speed == kSpeedInit && e.sent[4].value == 2000.0);
        CHECK(e.sent[5].speed == kSpeedRun && e.sent[5].value == 20000.0);
        CHECK(e.sent[6].speed == kSpeedAcc && e.sent[6].value == 500000.0 && e.sent[7].speed == kSpeedDec && e.sent[7].value == 400000.0);
        CHECK(e.sent[8].kind == kCmdAxHome && e.sent[8].homeMode == 124 && e.sent[8].dir == 1 && e.sent[8].axis == k);
        CHECK(!R()->homeCardSide(10, 0));
        CHECK(R()->homeDone(10, 0) == 0);                                    // same Poll
        e.poll++;
        CHECK(R()->homeDone(10, 0) == 1);                                    //AI(W906-HOME-READYDONE) 20261003: READY after the command at position 0 = done (EastSun: issued = homing; no HOMING sample needed)
        CHECK(R()->homeDone(10, 0) == 0);                                    // once
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.axes[(std::size_t)k].state = 4; e.poll++;
        CHECK(R()->homeDone(10, 0) == 0);                                    // HOMING
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(R()->homeDone(10, 0) == 1);                                    // HOMING then READY
        CHECK(R()->homeDone(10, 0) == 0);                                    // once
        // direction 0 -> 128 / -1
        e.sent.clear();
        CHECK(R()->homeStart(10, 0, false, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        CHECK(!e.sent.empty() && e.sent.back().homeMode == 128 && e.sent.back().dir == -1);
        // a stop while homing cancels the job: HOMING -> stop -> READY is not "homed"
        e.axes[(std::size_t)k].state = 4; e.poll++;
        CHECK(R()->homeDone(10, 0) == 0);
        Call(10, 0, kEcStopDec);
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(R()->homeDone(10, 0) == 0);
        //AI(W906-ENG1203) 20260929: R2 -- a stop while the job's samples still read READY (HOMING not seen yet) is SENT
        //  (never Q7-skipped while homing) and cancels the job
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.poll++;                                                            // READY: the drive has not entered HOMING yet
        {
            const std::size_t n2 = e.sent.size();
            CHECK(Call(10, 0, kEcStopDec) == 0 && e.sent.size() == n2 + 1 && e.sent.back().kind == kCmdAxStop);
        }
        e.axes[(std::size_t)k].state = 4; e.poll++; CHECK(R()->homeDone(10, 0) == 0);
        e.axes[(std::size_t)k].state = 1; e.poll++; CHECK(R()->homeDone(10, 0) == 0);   // cancelled: HOMING -> READY is not "homed"
        // R5 -- HOMING then ERROR_STOP is a homing FAULT, never "homed" (golden waits; MotorHome's alarm check takes over)
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.axes[(std::size_t)k].state = 4; e.poll++; CHECK(R()->homeDone(10, 0) == 0);
        e.axes[(std::size_t)k].state = 3; e.poll++; CHECK(R()->homeDone(10, 0) == -1);   //AI(W906-HOME-FAILVISIBLE) 20261003: ERROR_STOP while homing = reported failed (was 0 forever: the full HOME waited silently)
        e.poll++;                                   CHECK(R()->homeDone(10, 0) == 0);
        Call(10, 0, kEcStopDec); e.axes[(std::size_t)k].state = 1; e.poll++;           // the engine's alarm stop
        // R6 -- a HOMING sample from BEFORE the home command (a previous / cancelled job) does not count
        e.axes[(std::size_t)k].state = 4;
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        CHECK(R()->homeDone(10, 0) == 0);                                    // same Poll: pending -- that HOMING is stale
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(R()->homeDone(10, 0) == 1);                                    //AI(W906-HOME-READYDONE) 20261003: READY after the command = the home is over (was: waited for a HOMING sample)
        Call(10, 0, kEcStopDec); e.poll++;
        // R7 -- a failed PTP (home) speed write aborts the home: no Acm_AxHome, the axis never "done" (Q11), latched
        e.sent.clear(); e.refuseKind = kCmdAxSetSpeed;
        CHECK(R()->homeStart(10, 0, true, 21000.0, 2100.0, 500000.0, 400000.0) == 0);
        {
            bool anyHome = false;
            for (std::size_t i = 0; i < e.sent.size(); ++i) if (e.sent[i].kind == kCmdAxHome) anyHome = true;
            CHECK(!anyHome && !e.sent.empty());
        }
        e.poll += 3; CHECK(Read(10, 0).pending);
        { Pci1203MotorRouteFault flt; CHECK(Pci1203MotorRouteAxisFault(10, 0, flt) && flt.motionFailed >= 1); }
        e.refuseKind = -1; Call(10, 0, kEcStopDec); e.poll++;
        // PHomeHighSpeed == 0: nothing sent
        e.sent.clear();
        CHECK(R()->homeStart(10, 0, true, 0.0, 2000.0, 1.0, 1.0) == 0 && e.sent.empty());
        CHECK(Read(10, 0).pending);                                          // Q11: a home that went nowhere never completes
        Call(10, 0, kEcStopDec); e.poll++;
        // DRY home: recorded, never done
        e.dry = true; e.sent.clear();
        CHECK(R()->homeStart(10, 0, true, 20000.0, 2000.0, 1.0, 1.0) == 0 && !e.sent.empty() && e.sent.back().kind == kCmdAxHome);
        e.axes[(std::size_t)k].state = 4; e.poll++; R()->homeDone(10, 0);
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(R()->homeDone(10, 0) == 0);
        e.dry = false;

        // card-side drive
        FakeEnv f; Fresh(f);
        const int j = f.AddAxis(11, 0); f.AddSlave(11, false);
        CHECK(R()->bind(11, 0, 110, false));
        CHECK(R()->homeStart(11, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        CHECK(f.sent.size() == 5);
        CHECK(f.sent[0].kind == kCmdAxSetHome && f.sent[0].home == kHomeVelLow && f.sent[0].value == 2000.0);
        CHECK(f.sent[1].home == kHomeVelHigh && f.sent[1].value == 20000.0 && f.sent[2].home == kHomeAcc && f.sent[3].home == kHomeDec);
        CHECK(f.sent[4].kind == kCmdAxMoveHome && f.sent[4].homeMode == 11 && f.sent[4].dir == 1 && f.sent[4].axis == j);
        CHECK(R()->homeCardSide(11, 0));
        //AI(W906-ENG1203) 20260929: R8 -- HomeDirection=0 -> dir -1 (golden HomeDirection ? 0 : 1 vendor)
        f.sent.clear();
        CHECK(R()->homeStart(11, 0, false, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        CHECK(!f.sent.empty() && f.sent.back().kind == kCmdAxMoveHome && f.sent.back().dir == -1 && f.sent.back().homeMode == 11);
        Call(11, 0, kEcStopDec); f.poll++;
        // R19 -- a failed card home-parameter write aborts: no Acm_AxMoveHome, never "done"
        f.sent.clear(); f.refuseKind = kCmdAxSetHome;
        CHECK(R()->homeStart(11, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 0);
        {
            bool anyMoveHome = false;
            for (std::size_t i = 0; i < f.sent.size(); ++i) if (f.sent[i].kind == kCmdAxMoveHome) anyMoveHome = true;
            CHECK(!anyMoveHome && f.sent.size() == 1 && f.sent[0].kind == kCmdAxSetHome);
        }
        f.poll += 3; CHECK(Read(11, 0).pending);
        f.refuseKind = -1; Call(11, 0, kEcStopDec); f.poll++;
        // unknown drive: nothing
        FakeEnv g; Fresh(g);
        g.AddAxis(12, 0);
        CHECK(R()->bind(12, 0, 120, false));
        CHECK(R()->homeStart(12, 0, true, 20000.0, 2000.0, 1.0, 1.0) == 0 && g.sent.empty());
        CHECK(R()->homeDone(12, 0) == -1);                                  //AI(W906-HOME-FAILVISIBLE) 20261003: never sent = reported failed once
    }

    // ---- I: InitMotor ----
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(13, 0);
        CHECK(R()->bind(13, 0, 130, false));
        CHECK(R()->initCfg(13, 0, kInitCfgMotorServo, true, true, 100000.0, 500000.0, 400000.0));
        Pci1203InitCfgStep plan[24];
        const int n = Pci1203GoldenInitCfgPlan(kInitCfgMotorServo, true, true, plan, 24);      
        CHECK(n > 0);
        CHECK((int)e.sent.size() == 1 + n + 3 + 1);                          // ResetError (READY) + plan + Max* + ResetError
        CHECK(e.sent[0].kind == kCmdAxResetError && e.sent[0].axis == k);
        int planOk = 0;
        for (int i = 0; i < n; ++i)
            if (e.sent[(std::size_t)(1 + i)].kind == kCmdAxSetInitCfg && e.sent[(std::size_t)(1 + i)].initCfg == plan[i].which &&
                e.sent[(std::size_t)(1 + i)].value == plan[i].value) ++planOk;
        CHECK(planOk == n);
        CHECK(e.sent[(std::size_t)(1 + n)].speed == kSpeedMaxVel && e.sent[(std::size_t)(1 + n)].value == 100000.0);
        CHECK(e.sent[(std::size_t)(2 + n)].speed == kSpeedMaxAcc && e.sent[(std::size_t)(3 + n)].speed == kSpeedMaxDec);
        CHECK(e.sent.back().kind == kCmdAxResetError);
        // ERROR_STOP: golden's second ResetError, then the table
        e.sent.clear(); e.poll++;
        e.axes[(std::size_t)k].state = 3;
        CHECK(R()->initCfg(13, 0, kInitCfgMotorRotate, false, false, 1.0, 2.0, 3.0));
        CHECK(e.sent.size() >= 2 && e.sent[0].kind == kCmdAxResetError && e.sent[1].kind == kCmdAxResetError && e.sent[2].kind == kCmdAxSetInitCfg);
        // unreadable state: the unbounded golden goto is bounded to 3 rounds, nothing else written
        e.sent.clear();
        e.axes[(std::size_t)k].valid = false;
        CHECK(!R()->initCfg(13, 0, kInitCfgMotorServo, true, true, 1.0, 2.0, 3.0));
        CHECK(e.sent.size() == 3 && e.sent[0].kind == kCmdAxResetError && e.sent[2].kind == kCmdAxResetError);
        e.axes[(std::size_t)k].valid = true;
        // not claimed: false, nothing sent
        e.sent.clear();
        CHECK(!R()->initCfg(99, 0, kInitCfgMotorServo, true, true, 1.0, 2.0, 3.0) && e.sent.empty());
    }

    //AI(W906-HOME-FAILVISIBLE) 20261003: ---- N: READY ~5 s (10 engine calls) with no HOMING sample after the command -> -1, not 0 forever
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(15, 0); e.AddSlave(15, true);
        CHECK(R()->bind(15, 0, 150, false));
        CHECK(R()->homeStart(15, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.axes[(std::size_t)k].state = 1;
        int got = 0, calls = 0;
        for (; calls < 12 && got == 0; ++calls) { e.poll++; got = R()->homeDone(15, 0); }
        CHECK(got == 1 && calls == 1);                                       //AI(W906-HOME-READYDONE) 20261003: READY at 0 after the command = done at once (the 10-call failure is gone for DS402)
        CHECK(R()->homeDone(15, 0) == 0);                                    // reported once; the job is over
    }
    //AI(W906-HOME-SEEN) 20261003: ---- N2: EastSun「又失敗了」(19:46 MInArmX) -- the home ran and ended between two engine looks; the
    //  monitor's own poll saw HOMING after the command (homingSeenPoll) -> done, not "READY without HOMING". A HOMING seen only BEFORE the
    //  command (homingSeenPoll <= the command's poll) does not count.
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(16, 0); e.AddSlave(16, true);
        CHECK(R()->bind(16, 0, 160, false));
        e.axes[(std::size_t)k].homingSeenPoll = e.poll;                      // an old HOMING, before the command
        CHECK(R()->homeStart(16, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.axes[(std::size_t)k].state = 1; e.axes[(std::size_t)k].cmdPos = 3500.0; e.poll++;
        CHECK(R()->homeDone(16, 0) == -1);                                   //AI(W906-HOME-READYDONE) 20261003: READY after the command away from 0 = not completed -> failed (a stale HOMING does not help)
    }
    //AI(W906-HOME-DUALAXIS) 20261003: ---- N4: EastSun ruling -- an axis of a 2-axis drive (same station) waits while its twin homes
    {
        FakeEnv e; Fresh(e);
        const int k0 = e.AddAxis(18, 0); const int k1 = e.AddAxis(18, 1); e.AddSlave(18, true);
        (void)k1;
        CHECK(R()->siblingHoming != 0);
        CHECK(!R()->siblingHoming(18, 1) && !R()->siblingHoming(18, 0));     // nobody homing
        e.axes[(std::size_t)k0].state = 4;                                   // axis 0 HOMING in the monitor
        CHECK(R()->siblingHoming(18, 1));                                    // its twin waits
        CHECK(!R()->siblingHoming(18, 0));                                   // not its own
        e.axes[(std::size_t)k0].state = 1;
        CHECK(R()->bind(18, 0, 180, false));
        CHECK(R()->homeStart(18, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        CHECK(R()->siblingHoming(18, 1));                                    // just issued, not yet sampled HOMING: still waits
        e.axes[(std::size_t)k0].state = 4; e.poll++; R()->homeDone(18, 0);
        e.axes[(std::size_t)k0].state = 1; e.poll++; CHECK(R()->homeDone(18, 0) == 1);
        CHECK(!R()->siblingHoming(18, 1));                                   // twin done: go
    }
    //AI(W906-HOME-ENDPOS) 20261003: ---- N3: HOMING -> READY away from position 0 = an interrupted DS402 home -> -1, never "homed"
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(17, 0); e.AddSlave(17, true);
        CHECK(R()->bind(17, 0, 170, false));
        CHECK(R()->homeStart(17, 0, true, 20000.0, 2000.0, 500000.0, 400000.0) == 1);
        e.axes[(std::size_t)k].state = 4; e.poll++; CHECK(R()->homeDone(17, 0) == 0);
        e.axes[(std::size_t)k].state = 1; e.axes[(std::size_t)k].cmdPos = -1112.0; e.poll++;
        CHECK(R()->homeDone(17, 0) == -1);                                   // 15:54 MInArmX stopped at -1112
        CHECK(R()->homeDone(17, 0) == 0);                                    // reported once
    }
    //AI(W906-HOME-MAXVEL) 20261003: ---- M: EastSun「全機回home 當掉了」-- the engine home's InitMotor sets CFG_AxMaxVel = PJogHighSpeed;
    //  a HomeHighSpeed above it was refused by the card, so the home was never sent. Now the ceiling known to be lower is raised first.
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(14, 0); e.AddSlave(14, true);                     // a DS402 drive (as section H)
        CHECK(R()->bind(14, 0, 140, false));
        CHECK(R()->initCfg(14, 0, kInitCfgMotorServo, true, true, 80000.0, 8000.0, 8000.0));   // HT9050 MInArmZA: JogHigh 80000, Acc / Dec 8000
        e.sent.clear(); e.poll++;
        CHECK(R()->homeStart(14, 0, true, 100000.0, 10000.0, 8000.0, 8000.0) == 1);            // HomeHigh 100000 > the 80000 ceiling
        std::size_t iMax = 999, iRun = 999;
        int nMaxAcc = 0, nMaxDec = 0;
        for (std::size_t i = 0; i < e.sent.size(); ++i) {
            if (e.sent[i].kind != kCmdAxSetSpeed) continue;
            if (e.sent[i].speed == kSpeedMaxVel && e.sent[i].value == 100000.0 && iMax == 999) iMax = i;
            if (e.sent[i].speed == kSpeedRun && iRun == 999) iRun = i;
            if (e.sent[i].speed == kSpeedMaxAcc) ++nMaxAcc;
            if (e.sent[i].speed == kSpeedMaxDec) ++nMaxDec;
        }
        CHECK(iMax < iRun && iRun < e.sent.size());                  // CFG_AxMaxVel = 100000 before the home VelHigh
        CHECK(nMaxAcc == 0 && nMaxDec == 0);                         // acc / dec already within their ceilings: not rewritten
        CHECK(!e.sent.empty() && e.sent.back().kind == kCmdAxHome);  // and the home is sent
    }

    //AI(W906-ENG1203) 20260929: ---- F: HIGH-1 stops that bypass the route ----
    {
        FakeEnv e; Fresh(e);
        const int k = e.AddAxis(20, 0); e.AddSlave(20, true);
        const int u = e.AddAxis(21, 0);                                      // opened, never claimed
        CHECK(R()->bind(20, 0, 200, false));
        Pci1203Cmd stop; stop.kind = kCmdAxStop; stop.axis = k;
        Pci1203CmdResult okr; okr.accepted = true; okr.issued = true; okr.ret = 0;
        int st = -9, sa = -9;
        // the route table NOT installed as the engine route: nothing happens -- the home job completes exactly as before
        SetEcatMotorRoute(0);
        CHECK(R()->homeStart(20, 0, true, 20000.0, 2000.0, 1.0, 1.0) == 1);
        e.axes[(std::size_t)k].state = 4; e.poll++; CHECK(R()->homeDone(20, 0) == 0);
        CHECK(!Pci1203MotorRouteNoteForeignStop(stop, okr, &st, &sa) && st == -1 && sa == -1);
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(R()->homeDone(20, 0) == 1);                                    // untouched
        CHECK(Pci1203MotorRouteStatsNow().foreignStops == 0);
        // installed: the alarm path's Stop1203 during a home
        SetEcatMotorRoute(Pci1203MotorRouteTable());
        CHECK(R()->homeStart(20, 0, true, 20000.0, 2000.0, 1.0, 1.0) == 1);
        e.axes[(std::size_t)k].state = 4; e.poll++; CHECK(R()->homeDone(20, 0) == 0);   // HOMING seen
        CHECK(Pci1203MotorRouteNoteForeignStop(stop, okr, &st, &sa) && st == 20 && sa == 0);
        CHECK(Read(20, 0).pending);                                          // the same Poll
        e.axes[(std::size_t)k].state = 1; e.poll++;
        CHECK(Read(20, 0).pending);                                          // the stop may have run INSIDE this Poll: one more
        CHECK(R()->homeDone(20, 0) == 0);
        e.poll++;
        CHECK(!Read(20, 0).pending);
        CHECK(R()->homeDone(20, 0) == 0);                                    // HOMING -> READY after a foreign stop is NOT "homed"
        // a later route command never SHORTENS that window
        CHECK(Pci1203MotorRouteNoteForeignStop(stop, okr, 0, 0));
        Call(20, 0, kEcSvOn, 0, 1.0);                                        // state-changing: its own window ends one Poll earlier
        e.poll++; CHECK(Read(20, 0).pending);
        e.poll++; CHECK(!Read(20, 0).pending);
        // only STOPS, only ones that went out (issued OK, or DRY -- the route's own stop rule), only claimed axes
        Pci1203Cmd jog = stop; jog.kind = kCmdAxJogStart; jog.dir = 1;
        CHECK(!Pci1203MotorRouteNoteForeignStop(jog, okr, 0, 0));
        Pci1203CmdResult ref; ref.accepted = false; ref.issued = false; ref.ret = 0;
        CHECK(!Pci1203MotorRouteNoteForeignStop(stop, ref, 0, 0));
        Pci1203CmdResult bad = okr; bad.ret = 0x80001111ul;
        CHECK(!Pci1203MotorRouteNoteForeignStop(stop, bad, 0, 0));
        Pci1203CmdResult dryr = okr; dryr.issued = false;
        CHECK(Pci1203MotorRouteNoteForeignStop(stop, dryr, 0, 0));
        Pci1203Cmd emg = stop; emg.kind = kCmdAxEmgStop;
        CHECK(Pci1203MotorRouteNoteForeignStop(emg, okr, 0, 0));
        Pci1203Cmd other = stop; other.axis = u;
        CHECK(!Pci1203MotorRouteNoteForeignStop(other, okr, 0, 0));          // the engine never had it
        Pci1203Cmd oob = stop; oob.axis = 99;
        CHECK(!Pci1203MotorRouteNoteForeignStop(oob, okr, 0, 0));
        CHECK(Pci1203MotorRouteStatsNow().foreignStops == 4);
        // Q11's stuck is NOT released by a foreign stop -- only this route's own stop releases it
        e.poll += 2;
        e.dry = true; Call(20, 0, kEcMoveAbs, 0, 5.0); e.dry = false;        // DRY motion -> stuck
        CHECK(Pci1203MotorRouteNoteForeignStop(stop, okr, 0, 0));
        e.poll += 3; CHECK(Read(20, 0).pending);
        Call(20, 0, kEcStopDec); e.poll++;
        CHECK(!Read(20, 0).pending);
        CHECK(Pci1203MotorRouteStatsNow().foreignStops == 5);
        SetEcatMotorRoute(0);
    }

    //AI(W906-ENG1203) 20260929: ---- L: MEDIUM-1 a failed stop is latched and never dropped from the console ----
    {
        FakeEnv e; Fresh(e);
        e.AddAxis(30, 0); e.AddAxis(31, 0);
        CHECK(R()->bind(30, 0, 300, false) && R()->bind(31, 0, 310, false));
        Pci1203MotorRouteFault f;
        CHECK(Pci1203MotorRouteAxisFault(30, 0, f) && f.stopFailed == 0 && f.motionFailed == 0 && Pci1203MotorRouteFaultText(f).empty());
        CHECK(!Pci1203MotorRouteAxisFault(99, 0, f) && f.stopFailed == 0);
        e.ret = 0x80005111ul;
        CHECK(Call(30, 0, kEcStopDec) == 0x80005111ul);                      // vendor error on StopDec (golden WAR16122)
        CHECK(Pci1203MotorRouteAxisFault(30, 0, f) && f.stopFailed == 1 && f.lastRet == 0x80005111ul && f.lastWhat == "StopDec");
        CHECK(Call(30, 0, kEcExtDrive, 0, 0.0) == 0x80005111ul);             // DecStop's ExtDrive(0)
        Pci1203MotorRouteAxisFault(30, 0, f); CHECK(f.stopFailed == 2);
        CHECK(Call(30, 0, kEcExtDrive, 0, 1.0) == 0x80005111ul);             // ExtDrive(1) = entering jog: not a stop
        Pci1203MotorRouteAxisFault(30, 0, f); CHECK(f.stopFailed == 2 && f.motionFailed == 0);
        CHECK(Call(30, 0, kEcJog, 0, 0.0) == 0x80005111ul);                  // a failed motion
        Pci1203MotorRouteAxisFault(30, 0, f); CHECK(f.motionFailed == 1 && f.stopFailed == 2);
        e.ret = 0;
        e.refuseKind = kCmdAxEmgStop;
        CHECK(Call(30, 0, kEcStopEmg) == (unsigned long)kEcRcRefused);       // refused by Execute
        Pci1203MotorRouteAxisFault(30, 0, f); CHECK(f.stopFailed == 3 && f.lastRet == (unsigned long)kEcRcRefused);
        e.refuseKind = -1;
        e.card = false;
        CHECK(Call(30, 0, kEcStopDec) == (unsigned long)kEcRcNoCard);        // never reached Execute
        Pci1203MotorRouteAxisFault(30, 0, f); CHECK(f.stopFailed == 4 && f.lastRet == (unsigned long)kEcRcNoCard);
        e.card = true;
        e.dry = true; CHECK(Call(30, 0, kEcStopDec) == 0); e.dry = false;    // DRY is not a failure
        e.poll++; CHECK(Call(30, 0, kEcStopDec) == 0);                       // a success (here Q7-skipped: fresh READY)
        Pci1203MotorRouteAxisFault(30, 0, f);
        CHECK(f.stopFailed == 4 && !Pci1203MotorRouteFaultText(f).empty());  // LATCHED: a later success does not clear it
        CHECK(Pci1203MotorRouteAxisFault(31, 0, f) && f.stopFailed == 0 && f.motionFailed == 0);   // per axis
        CHECK(Pci1203MotorRouteStatsNow().stopFailed == 4 && Pci1203MotorRouteStatsNow().motionFailed == 1);
        // the console budget: per (station, axis), and a failed stop is never dropped
        Pci1203MotorRouteResetForTest(); Pci1203MotorRouteSetEnvForTest(&e);
        CHECK(R()->bind(30, 0, 300, false) && R()->bind(31, 0, 310, false)); // one line each
        for (int i = 0; i < 40; ++i) Call(30, 0, kEcSvOn, 0, 1.0);           // 40 more on axis 30
        const unsigned long shown = Pci1203MotorRouteStatsNow().linesShown, dropped = Pci1203MotorRouteStatsNow().linesDropped;
        CHECK(shown == 31 && dropped == 11);                                 // axis 30: 41 lines = 30 shown + 11 dropped; axis 31: 1
        e.ret = 0x80000001ul; Call(30, 0, kEcStopDec); e.ret = 0;            // a failed stop on the exhausted axis
        CHECK(Pci1203MotorRouteStatsNow().linesShown == shown + 1 && Pci1203MotorRouteStatsNow().linesDropped == dropped);
        Call(31, 0, kEcSvOn, 0, 1.0);                                        // the other axis still has its own budget
        CHECK(Pci1203MotorRouteStatsNow().linesShown == shown + 2 && Pci1203MotorRouteStatsNow().linesDropped == dropped);
    }

    //AI(W906-ENG1203) 20260929: ---- G: R9 the install decision this build compiled ----
    {
        const int g = Pci1203MotorRouteInstallGate();
#if defined(SOFT_SIMULTE)
        CHECK(g == kEcGateSim);
#elif !defined(WB_ENGINE_MOTOR_1203) || !defined(INSTALL_1203_MONITOR) || !defined(WB_PUMP_1203_CONTROL)
        CHECK(g == kEcGateOff);                                              // the committed tree: WB_ENGINE_MOTOR_1203 is commented out
#elif !defined(WB_PUMP_1203_START_RING)
        CHECK(g == kEcGateNoStartRing);
#else
        CHECK(g == kEcGateArmedBuild);
#endif
#if !defined(WB_ENGINE_MOTOR_1203)
        CHECK(g != kEcGateNoStartRing && g != kEcGateArmedBuild);            // without the macro no build reaches the arming branch
#endif
        // (the installer itself is NOT called here: pci1203_control_gate.ps1 check 6 allows it exactly one call site, wb_serve's)
        CHECK(!Pci1203MotorRouteInstalled());
    }

    //AI(W906-ENG1203) 20260929: ---- V: R10 / R15 the live environment's two conditions ----
    {
        int usable = 0;
        for (int m = 0; m < 16; ++m)
            if (Pci1203MotorRouteCardUsableOf((m & 1) != 0, (m & 2) != 0, (m & 4) != 0, (m & 8) != 0)) { ++usable; CHECK(m == (1 | 2 | 8)); }
        CHECK(usable == 1);                                                  // present, Open_, NOT Disabled, card open -- only that one
        CHECK(Pci1203MotorRouteExecCountOf(0) == 0ul);
        TPci1203Control ctl;                                                 // never opened: Execute refuses, and counts
        Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = 0;
        ctl.Execute(c); ctl.Execute(c);
        ctl.NoteRefusal(-1, "test", "test refusal");
        CHECK(ctl.refusedCount() == 3ul && Pci1203MotorRouteExecCountOf(&ctl) == 3ul);   // any caller's Execute is seen
        Pci1203MotorRouteEnv& live = Pci1203MotorRouteLiveEnv();            // this process has no monitor / control object
        CHECK(!live.CardUsable() && !live.ControlArmed() && live.ExecCount() == 0ul && live.PollCount() == 0ul && live.AxisCount() == 0);
    }

    Pci1203MotorRouteSetEnvForTest(0);
    Pci1203MotorRouteResetForTest();
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
