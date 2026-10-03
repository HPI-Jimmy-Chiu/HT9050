// =============================================================================
//  tests/test_teach_st02_c9.cpp  --  ctest ST02C9_TeachSt02
//
//  AI(W906-ST02-C9-G2) 20261002 (St02-E helper).  St02 card ST02-C9 group G2 = WebMotorAccess.cpp's EOF block (action
//  teachSt02, WebTeachSt02.h) on a fake IMotorAccessBackend + a fake ITeachSt02Ops.  golden = 906_0625_Steven uteach.cpp:
//    SpeedButtonInRotatePos90Click :4605-4655 / Neg90 :4657-4707, btnSht1/2GoLatchClick :4721-4751,
//    btnSetAllInArmZ_MoveClick :5948-5971 / btnSetAllOutArmZ_MoveClick :5973-5996.
//
//  Locks:
//   [0] not installed (the laptop's binaries and ctests) -> refused, the backend untouched; a non-uteach source; an
//       unknown button.
//   [1] golden's pure parts: the P2 table (:4613-4620), grpRotate_Kit's Visible (:1652), pnlIn/OutArmZ's Visible (:1457-1458).
//   [2] the state query: config, shown[], P2, the form statics; reqId (never "id").
//   [3] rotate +/-90: hidden -> refused before anything; the golden guard (no motor / CheckCanMove); position unreadable;
//       |P1|>999999; not a rotate motor (no fCMD write); RotateKit backlash by direction (page value, Prod value when 0);
//       backlash ONLY on MInRotateKit / MOutRotateKit; the 1203 path (PTP speed re-sent at the current pct, MoveAbs P1+-P2+bl);
//       a non-1203 rotate motor = the golden object (no speed); Enable=0 rows refused.
//   [4] Go Latch: hidden by golden on every machine; with the group shown (test only): the HomeFlag guard, shtSpeed checks,
//       iShuttleSelect / InitMOTParameter before the move (golden order), SetSpeed(shtSpeed) + MotorMove(Tech.iInShuttleNRight),
//       the floodgate refusal, Enable=0 / missing motor.
//   [5] Set All Z: hidden unless ep16Picker; golden's static b toggle (pick height, then 0, flips whatever happened); only
//       i<iPickRow && j<iPickCol; per-axis results (moved / refused + why: Enable=0, not an arm Z, outside Prod, no motor);
//       partial = state "error"; none moved = a refusal (b still flips); In and Out keep separate b's.
//   [6] through MotorAccessDispatch (needs the two same-line hooks in WebMotorAccess.cpp): the kActions row is live, the
//       dispatch line reaches TeachSt02Dispatch, SystemStart refuses it, a manual teach refuses it, STOP stops every axis.
//   [7]-[10] (G3: lanes, Index servo, TTL) removed -- AI(W906-ST02-C9) 20261002: G3 dropped in the rebase onto main 2dd90ef3 (the
//       machine wired those buttons first); the section numbers below are kept.
//   [11] AI(W906-ST02-C9-G2) 20261002: the 36 buttons whose OnClick is SpeedButtonInRotatePos90Click / Neg90Click (uteach.dfm):
//       every name and its golden parent; tsRotate's TabVisible (:1668) and the grb rules (:1690-1703) as a press matrix;
//       tsRotate buttons on e8 / e4 / e2MotRotate2Dut move by P2 800, hidden ones are refused before anything; the backlash
//       follows the selected motor (MInRotateKit / MOutRotateKit, :4639-4652), never the button; the state query's 17 new keys.
//  No file, no env, no god-stack (WebTeachSt02Live.cpp is not linked).
// =============================================================================
#include "WebMotorAccess.h"
#include "WebTeachSt02.h"

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;
static void CheckAt(bool cond, const char* msg, int line)
{
    if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else      { std::printf("  FAIL: %s  (line %d)\n", msg, line); ++g_fail; }
}
#define CHECK(cond, msg) CheckAt((cond) ? true : false, (msg), __LINE__)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

// ---------------------------------------------------------------------------
class FakeBe : public IMotorAccessBackend {
public:
    std::map<std::string, MotorAccessAxis> table;
    std::map<int, MotorGolden>             golden;
    std::map<int, std::string>             aliasOf;
    std::map<int, double>                  cmdPos, actPos;
    std::map<int, int>                     readPos;
    std::set<int>                          notReady, doorOpen, locked;
    std::map<int, bool>                    lastDirP;
    std::vector<bool>                      opened;
    std::vector<Pci1203Cmd>                executed;
    struct Move { int mi; int target; int pct; bool setSpeed; };
    std::vector<Move>                      moves;
    std::vector<std::string>               stopAll;
    std::vector<int>                       stopMotor, teachCanMoveCalls, floodCalls;
    std::vector<std::pair<int, bool> >     servoSet;
    std::vector<std::pair<int, int> >      setSpeeds;
    TeachRegistry                          reg;
    bool ready = true, canTeach = true, systemStart = false, floodReady = true;
    int  moveRet = 0, prodBacklashIn = 0, prodBacklashOut = 0;
    unsigned long polls = 0;

    int AllCalls() const { return (int)(executed.size() + moves.size() + stopAll.size() + stopMotor.size() + servoSet.size() + teachCanMoveCalls.size()); }
    int CountKind(Pci1203CmdKind k) const { int n = 0; for (std::size_t i = 0; i < executed.size(); ++i) if (executed[i].kind == k) ++n; return n; }

    bool Resolve(const std::string& id, MotorAccessAxis& out) override
    {
        std::map<std::string, MotorAccessAxis>::const_iterator it = table.find(id);
        if (it == table.end()) return false;
        out = it->second; return true;
    }
    bool Pci1203Ready(std::string& why) override { if (!ready) why = "fake: no 1203"; return ready; }
    int  Pci1203AxisCount() override { return (int)opened.size(); }
    bool Pci1203AxisOpened(int ax) override { return ax >= 0 && ax < (int)opened.size() && opened[ax]; }
    bool Pci1203ServoOn(int, bool& known) override { known = false; return false; }
    Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) override
    {
        executed.push_back(c);
        Pci1203CmdResult r;
        r.accepted = true; r.issued = true; r.ret = 0;
        char b[64]; std::snprintf(b, sizeof(b), "Acm_fake(kind=%d, ax=%d, v=%g)", (int)c.kind, c.axis, c.value);
        r.wouldCall = b;
        return r;
    }
    void Pci1203NoteRefusal(long long, const std::string&, const std::string&) override {}
    void GoldenStopAll(const std::string& source) override { stopAll.push_back(source); }
    void GoldenStopMotor(int mi) override { stopMotor.push_back(mi); }
    bool GoldenServoOn(int, bool& known) override { known = false; return false; }
    void GoldenServoOnOff(int mi, bool on) override { servoSet.push_back(std::make_pair(mi, on)); }
    bool Pci1203CmdPos(int ax, double& card) override
    {
        std::map<int, double>::const_iterator it = cmdPos.find(ax);
        if (it == cmdPos.end()) return false;
        card = it->second; return true;
    }
    bool Pci1203AxisReady(int ax) override { return notReady.count(ax) == 0; }
    bool GoldenMotor(int mi, MotorGolden& g) override
    {
        std::map<int, MotorGolden>::const_iterator it = golden.find(mi);
        if (it == golden.end()) return false;
        g = it->second; return true;
    }
    bool GoldenSafeDoorOpen(int mi) override { return doorOpen.count(mi) != 0; }
    bool GoldenSafeDoorClosed() override { return true; }
    bool GoldenMotorCanRun() override { return true; }
    bool GoldenMoveLocked(int mi) override { return locked.count(mi) != 0; }
    int  GoldenReadPos(int mi) override { return readPos.count(mi) ? readPos[mi] : 0; }
    void GoldenJog(int, bool, int) override {}
    int  GoldenMotorMove(int mi, int target, int pct, bool setSpeed) override { Move m = { mi, target, pct, setSpeed }; moves.push_back(m); return moveRet; }
    void GoldenSetParam(int, MotorParamWhich, int) override {}
    bool Pci1203AxisState(int, unsigned&) override { return false; }
    void GoldenSetHomeFlag(int mi, int v) override { std::map<int, MotorGolden>::iterator it = golden.find(mi); if (it != golden.end()) it->second.homeFlag = v; }
    int  GoldenZSafePos() override { return 0; }
    bool GoldenSystemStart() override { return systemStart; }
    void GoldenResetMNet() override {}
    void GoldenSetRangeRate(int, bool, int) override {}
    unsigned long Pci1203PollCount() override { return ++polls; }
    bool GoldenShuttleFloodgateReady(int which) override { floodCalls.push_back(which); return floodReady; }
    unsigned GoldenReadSpeed(int) override { return 0; }
    bool GoldenSafeLockActive() override { return false; }
    int  Pci1203DriveKind(int) override { return 1; }
    void GoldenClearAllHomeFlags() override {}
    void GoldenSetSpeed(int mi, int pct, bool) override { setSpeeds.push_back(std::make_pair(mi, pct)); }
    void GoldenSetCell(int, int, double) override {}
    void GoldenSetLastHomePos(int, int) override {}
    bool Pci1203MotionIO(int, unsigned long&) override { return false; }
    bool GoldenAlarmLed(int, bool& known) override { known = false; return false; }
    MotorReloadResult GoldenReloadMotorParams() override { return MotorReloadResult(); }
    double MonotonicMs() override { return -1.0; }
    MotorPowerState GoldenMotorPower(bool) override { return MotorPowerState(); }
    void GoldenMotorPowerOnBegin() override {}
    bool GoldenMotorPowerOnStep() override { return true; }
    void GoldenServerOn() override {}
    void GoldenMotorServoOff(const std::string&) override {}
    std::string GoldenRouteLastWrite() override { return std::string(); }
    void GoldenSetRangeMemory(int, unsigned) override {}
    void GoldenSetAccMemory(int, double) override {}
    bool GoldenInitMotorEmgOff() override { return false; }
    std::vector<InitCfg> GoldenInitCfgPlan(const MotorGolden&) override { return std::vector<InitCfg>(); }
    void SleepMs(int) override {}
    void GoldenInitMOTParameterAll() override {}
    void GoldenFormClose() override {}
    std::string AliasOfMotor(int mi) override { return aliasOf.count(mi) ? aliasOf[mi] : std::string(); }
    int  GoldenLightScaleEncoder(std::string& src, bool& fromMonitor) override { src = "none"; fromMonitor = false; return 0; }
    int  GoldenLightScaleDataCount(int) override { return 0; }
    std::vector<std::string> GoldenLightScaleData(int) override { return std::vector<std::string>(); }
    void GoldenLightScaleDataClear(int) override {}
    void GoldenLightScaleCountsReset() override {}
    std::string LightScaleRoot() override { return std::string(); }
    std::string LocalStampYmdhm() override { return "202610020000"; }
    bool GoldenTeachRegistry(TeachRegistry& out, std::string&) override { out = reg; return true; }
    bool GoldenTeachCanMove(int mi) override { teachCanMoveCalls.push_back(mi); return canTeach; }
    bool GoldenMotorPowerOff() override { return false; }
    int  GoldenReadEncoderPos(int) override { return 0; }
    bool GoldenTechPosUsesReadPos(int) override { return false; }
    bool GoldenRotatorLastDirP(int mi) override { return lastDirP.count(mi) ? lastDirP[mi] : true; }   // golden initial value true
    void GoldenSetRotatorLastDirP(int mi, bool p) override { lastDirP[mi] = p; }
    int  GoldenProdRotatorBacklash(bool in) override { return in ? prodBacklashIn : prodBacklashOut; }
    bool GoldenIndexArm3Axis() override { return false; }
    int  GoldenTeachRemap(int mi) override { return mi; }
    bool AliasOfMotIndex(int mi, std::string& a) override
    {
        std::map<int, std::string>::const_iterator it = aliasOf.find(mi);
        if (it == aliasOf.end()) return false;
        a = it->second; return true;
    }
    bool Pci1203ActPos(int ax, double& card) override
    {
        std::map<int, double>::const_iterator it = actPos.find(ax);
        if (it == actPos.end()) return false;
        card = it->second; return true;
    }
    void GoldenClearAllMotorHome() override {}
};

class FakeOps : public ITeachSt02Ops {
public:
    TeachSt02Config        cfg;
    std::map<int, int>     kind;          // RotateMotorKind
    std::map<int, int>     sht;           // GoldenMotorIndex
    std::vector<int>       fcmd, initMot;
    int                    techRight[3];
    TeachSt02ArmZGrid      grid[2];       // [0] In, [1] Out
    int                    configCalls;
    FakeOps() : configCalls(0) { techRight[0] = techRight[1] = techRight[2] = 0; }
    TeachSt02Config Config() override { ++configCalls; return cfg; }
    int  RotateMotorKind(int mi) override { return kind.count(mi) ? kind[mi] : kSt02RotNone; }
    int  GoldenMotorIndex(int which) override { return sht.count(which) ? sht[which] : -1; }
    void GoldenClearFCmd(int mi) override { fcmd.push_back(mi); }
    void GoldenInitMOTParameter(int mi) override { initMot.push_back(mi); }
    int  TechInShuttleRight(int which) override { return (which >= 1 && which <= 2) ? techRight[which] : 0; }
    TeachSt02ArmZGrid ArmZGrid(bool inArm) override { return grid[inArm ? 0 : 1]; }
};

static MotorAccessAxis Axis1203(int mi, int board, int port, int slot)
{
    MotorAccessAxis a;
    a.motIndex = mi; a.cardModel = "PCI1203"; a.boardId = board; a.port = port; a.axis = slot; a.motorLive = true; a.tableEnable = true;
    return a;
}
static MotorAccessAxis AxisOther(int mi, const char* card, bool tableEnable)
{
    MotorAccessAxis a;
    a.motIndex = mi; a.cardModel = card; a.motorLive = true; a.tableEnable = tableEnable;
    return a;
}
static MotorGolden Golden(double gear, unsigned jogHigh, unsigned initSpeed, double acc, int homeFlag)
{
    MotorGolden g;
    g.valid = true; g.enable = true; g.selectable = true; g.gearRatio = gear; g.softP = 999999; g.softN = -999999;
    g.jogHigh = jogHigh; g.jogLow = 100; g.initSpeed = initSpeed; g.acc = acc; g.dec = acc; g.homeFlag = homeFlag;
    return g;
}
static MotorAccessReq St02Req(const char* btn, const char* motor)
{
    MotorAccessReq r;
    r.seq = 9; r.id = "cmd-9"; r.source = "uteach"; r.button = "St02Teach*"; r.action = "teachSt02"; r.kind = "motion";
    r.str["btn"] = btn;
    if (motor) r.motors.push_back(motor);
    return r;
}
static TeachSt02ArmZCell Cell(int row, int col, int mi, bool inPick, bool targetKnown, int target)
{
    TeachSt02ArmZCell c;
    c.row = row; c.col = col; c.motIndex = mi; c.inPick = inPick; c.targetKnown = targetKnown; c.target = target;
    return c;
}
static void Fresh()
{
    MotorAccessResetJobs();
    TeachSt02ResetForTests();
}
static const Pci1203Cmd* LastOf(const FakeBe& be, Pci1203CmdKind k)
{
    for (std::size_t i = be.executed.size(); i > 0; --i) if (be.executed[i - 1].kind == k) return &be.executed[i - 1];
    return 0;
}

// the rotate station of section [3]: MInRotate (M41) PCI1203 slot 0, MOutRotate (M42) slot 1, MInRotateB (M65) MN200
static void RotateRig(FakeBe& be, FakeOps& ops)
{
    be.opened.assign(5, true);
    be.table["MInRotate"]  = Axis1203(41, 41, 0, 0);  be.aliasOf[41] = "MInRotate";
    be.table["MOutRotate"] = Axis1203(42, 41, 1, 1);  be.aliasOf[42] = "MOutRotate";
    MotorGolden g = Golden(1.0, 450, 1, 4500.0, 1);
    g.rotateKit = 1; be.golden[41] = g;
    g.rotateKit = 2; be.golden[42] = g;
    be.cmdPos[0] = 1000.0; be.cmdPos[1] = 500.0;
    be.table["MInRotateB"] = AxisOther(65, "MN200", true);  be.aliasOf[65] = "MInRotateB";
    be.golden[65] = Golden(1.0, 450, 1, 0.1, 1);  be.readPos[65] = 200;
    be.table["MInRotateC"] = AxisOther(66, "MN200", false); be.aliasOf[66] = "MInRotateC";
    be.golden[66] = Golden(1.0, 450, 1, 0.1, 1);
    ops.kind[41] = kSt02RotInKit; ops.kind[42] = kSt02RotOutKit; ops.kind[65] = kSt02RotOther; ops.kind[66] = kSt02RotOther;
    ops.cfg.useRotateKit = 1; ops.cfg.rotateType = kSt02Rot1Mot; ops.cfg.usePickerCount = 4;
}

int main()
{
    std::printf("test_teach_st02_c9 -- St02 ST02-C9 G2 (WebMotorAccess.cpp EOF, action teachSt02)\n");

    // ---------------------------------------------------------------- [0]
    std::printf("[0] not installed / wrong source / unknown button\n");
    {
        Fresh();
        TeachSt02InstallOps(0);
        FakeBe be;
        MotorAccessOutcome o = TeachSt02Dispatch(St02Req("SpeedButtonInRotatePos90", "MInRotate"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "沒有安裝") && be.AllCalls() == 0, "no ops installed -> refused, the backend untouched");
        CHECK(TeachSt02InstalledOps() == 0, "nothing installed by default (WebTeachSt02Live.cpp is not linked here)");
        FakeOps ops;
        TeachSt02InstallOps(&ops);
        MotorAccessReq r = St02Req("St02State", 0); r.source = "uMotorTest";
        o = TeachSt02Dispatch(r, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "只收教導頁") && ops.configCalls == 0, "source uMotorTest -> refused before the ops are asked");
        o = TeachSt02Dispatch(St02Req("btnNoSuchButton", 0), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "不認得的按鈕") && be.AllCalls() == 0, "an unknown button -> refused");
        MotorAccessReq r2 = St02Req("x", 0); r2.str.erase("btn"); r2.button = "St02State";
        o = TeachSt02Dispatch(r2, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"st02State\""), "no params.btn -> the request's button names the handler");
        TeachSt02InstallOps(0);
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] golden's pure parts\n");
    {
        const int want[8] = { 2000, 2000, 800, 800, 2000, 800, 1250, 2000 };   // eCynRotate .. eInOutArm1Motor, then an unknown 7
        bool ok = true;
        for (int t = 0; t < 8; ++t) if (TeachSt02RotateP2(t) != want[t]) { ok = false; std::printf("    P2(%d) = %d, want %d\n", t, TeachSt02RotateP2(t), want[t]); }
        CHECK(ok, "P2: e4MotRotate / e8MotRotate / e2MotRotate2Dut 800, eInOutArm1Motor 1250, every other 2000 (golden :4613-4620)");
        TeachSt02Config c;
        c.useRotateKit = 1;
        bool vis = true;
        for (int t = 0; t < 8; ++t) { c.rotateType = t; const bool v = TeachSt02RotateKitShown(c); if (v != (t == 1 || t == 4 || t == 6)) vis = false; }
        c.useRotateKit = 0; c.rotateType = 1;
        CHECK(vis && !TeachSt02RotateKitShown(c), "grpRotate_Kit shown only for USE_ROTATE_KIT==1 && type 1 / 4 / 6 (golden :1652)");
        c.usePickerCount = 3;
        const bool p16 = TeachSt02ArmZPanelShown(c);
        c.usePickerCount = 4;
        const bool p4 = TeachSt02ArmZPanelShown(c);
        c.usePickerCount = 1;
        CHECK(p16 && !p4 && !TeachSt02ArmZPanelShown(c), "pnlIn/OutArmZ shown only for USE_PICKER_COUNT==ep16Picker (3) (golden :1457-1458)");
        CHECK(kSt02Rot1Mot == 1 && kSt02Rot4Mot == 2 && kSt02Rot8Mot == 3 && kSt02Rot1Mot1Dut == 4 && kSt02Rot2Mot2Dut == 5 &&
              kSt02RotInOutArm1Motor == 6 && kSt02Picker16 == 3, "the header's copies of golden eRotateType / ep16Picker (MachineType.h)");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] the state query\n");
    {
        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        ops.cfg.useRotateKit = 0; ops.cfg.rotateType = 1; ops.cfg.usePickerCount = 4;   // HT9050 (machines/HT9050/sim_9378/Gerneral.ini)
        MotorAccessReq q = St02Req("St02State", 0); q.flag["query"] = true;
        MotorAccessOutcome o = TeachSt02Dispatch(q, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"grpRotate_Kit\":false") && Has(o.ackJson, "\"pnlInArmZ\":false") && Has(o.ackJson, "\"pnlOutArmZ\":false") &&
              Has(o.ackJson, "\"grpShtSensor\":false") && Has(o.ackJson, "\"rotateP2\":2000") && Has(o.ackJson, "\"usePickerCount\":4"),
              "HT9050 config: all four groups hidden, P2 2000");
        CHECK(Has(o.ackJson, "\"reqId\":\"cmd-9\"") && !Has(o.ackJson, "\"id\":") && Has(o.ackJson, "\"state\":\"done\"") && be.AllCalls() == 0,
              "reqId (never id), state done, nothing touched");
        ops.cfg.useRotateKit = 1; ops.cfg.rotateType = kSt02RotInOutArm1Motor; ops.cfg.usePickerCount = kSt02Picker16;
        o = TeachSt02Dispatch(q, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"grpRotate_Kit\":true") && Has(o.ackJson, "\"pnlInArmZ\":true") && Has(o.ackJson, "\"rotateP2\":1250") &&
              Has(o.ackJson, "\"setAllInArmZB\":false") && Has(o.ackJson, "\"shuttleSelect\":0"), "a 16-picker / InOutArm1Motor config: shown, P2 1250, statics at golden's initial values");
        TeachSt02InstallOps(0);
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] SpeedButtonInRotatePos90 / Neg90\n");
    {
        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        RotateRig(be, ops);
        MotorAccessReq p = St02Req("SpeedButtonInRotatePos90", "MInRotate"); p.num["backlashIn"] = 30; p.num["backlashOut"] = 0;
        MotorAccessReq n = St02Req("SpeedButtonInRotateNeg90", "MInRotate"); n.num["backlashIn"] = 30; n.num["backlashOut"] = 0;

        ops.cfg.useRotateKit = 0;
        MotorAccessOutcome o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "golden 看不到這顆按鈕") && Has(o.ackJson, "uteach.cpp:1652") && be.AllCalls() == 0,
              "USE_ROTATE_KIT=0 (HT9050): grpRotate_Kit hidden -> refused, not even CheckCanMove asked");
        ops.cfg.useRotateKit = 1;

        o = TeachSt02Dispatch(St02Req("SpeedButtonInRotatePos90", 0), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "沒有選馬達") && be.executed.empty(), "no motor selected (golden ActiveMotorIndex==-1) -> refused");
        be.canTeach = false;
        o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "CheckCanMove") && be.teachCanMoveCalls.size() == 1 && be.teachCanMoveCalls[0] == 41 && be.executed.empty(),
              "golden CheckCanMove / IsCanQuickJogMove false -> refused, nothing sent");
        be.canTeach = true;
        be.cmdPos.erase(0);
        o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "讀不到目前位置") && be.executed.empty(), "position unreadable (golden edtNowPosition) -> refused");
        be.cmdPos[0] = 1000000.0;
        o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Position over limitation") && be.executed.empty() && ops.fcmd.empty(), "|P1| > 999999 -> refused (golden :4622), no fCMD write");
        be.cmdPos[0] = 1000.0;
        ops.kind[41] = kSt02RotNone;
        o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "不是旋轉軸") && be.executed.empty() && ops.fcmd.empty(), "a motor outside golden's list -> refused, no fCMD write (golden does nothing)");
        ops.kind[41] = kSt02RotInKit;

        // Pos90, RotateKit_Type 1 (P2 2000), last direction negative -> +backlash
        be.lastDirP[41] = false;
        o = TeachSt02Dispatch(p, 7, be);
        const Pci1203Cmd* mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 0 && mv->value == 3030.0 && be.CountKind(kCmdAxSetSpeed) == 4 && mv->wireId == 7,
              "Pos90 on MInRotateKit: speed re-sent (4 PTP values) then MoveAbs(P1 1000 + P2 2000 + backlash 30) = 3030");
        CHECK(ops.fcmd.size() == 1 && ops.fcmd[0] == 41, "golden MOT[ActiveMotorIndex].fCMD=false (:4637)");
        CHECK(Has(o.ackJson, "\"p1\":1000") && Has(o.ackJson, "\"p2\":2000") && Has(o.ackJson, "\"goal\":3000") && Has(o.ackJson, "\"backlash\":30") &&
              Has(o.ackJson, "\"target\":3030") && Has(o.ackJson, "\"st02Btn\":\"SpeedButtonInRotatePos90\""), "ack: p1 / p2 / goal / backlash / target");
        CHECK(be.lastDirP[41] == true, "the move recorded golden iLastRotatorDirP (MotorMovePosition :588-591)");
        // Neg90, eInOutArm1Motor (P2 1250), last direction positive -> -backlash
        ops.cfg.rotateType = kSt02RotInOutArm1Motor;
        o = TeachSt02Dispatch(n, 8, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->value == -280.0 && Has(o.ackJson, "\"p2\":1250") && Has(o.ackJson, "\"backlash\":-30"),
              "Neg90 eInOutArm1Motor: 1000 - 1250 - 30 = -280");
        ops.cfg.rotateType = kSt02Rot1Mot;
        // the same direction twice: no backlash
        be.lastDirP[41] = true;
        o = TeachSt02Dispatch(p, 9, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->value == 3000.0 && Has(o.ackJson, "\"backlash\":0"), "Pos90 after a positive move: no backlash (3000)");
        // the backlash field missing
        MotorAccessReq nb = St02Req("SpeedButtonInRotatePos90", "MInRotate");
        const std::size_t fc = ops.fcmd.size(), ex = be.executed.size();
        o = TeachSt02Dispatch(nb, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "缺背隙設定") && be.executed.size() == ex && ops.fcmd.size() == fc + 1,
              "RotateKit without the backlash field -> refused (after golden's fCMD=false), nothing sent");
        // MOutRotateKit: page value 0 -> Prod.iOut_iRotateA_Backlash
        be.prodBacklashOut = 12; be.lastDirP[42] = false;
        MotorAccessReq po = St02Req("SpeedButtonInRotatePos90", "MOutRotate"); po.num["backlashIn"] = 30; po.num["backlashOut"] = 0;
        o = TeachSt02Dispatch(po, 1, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 1 && mv->value == 2512.0, "MOutRotateKit, edtEditRotateOutBacklash 0 -> Prod's 12: 500 + 2000 + 12 = 2512");
        // a rotate motor that is not a kit: no backlash, golden object (no speed)
        MotorAccessReq pb = St02Req("SpeedButtonInRotatePos90", "MInRotateB");
        o = TeachSt02Dispatch(pb, 1, be);
        CHECK(o.ok && be.moves.size() == 1 && be.moves[0].mi == 65 && be.moves[0].target == 2200 && !be.moves[0].setSpeed && !Has(o.ackJson, "\"backlash\""),
              "MInRotateB (MN200): MOT.MotorMove(200 + 2000), no speed, no backlash (golden: backlash only for the two kits)");
        // Enable=0
        const std::size_t mvn = be.moves.size();
        o = TeachSt02Dispatch(St02Req("SpeedButtonInRotatePos90", "MInRotateC"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && be.moves.size() == mvn, "a non-1203 Enable=0 rotate row -> refused, the golden object not called");
        be.table["MInRotate"].tableEnable = false;
        const std::size_t exn = be.executed.size();
        o = TeachSt02Dispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && be.executed.size() == exn, "a PCI1203 Enable=0 rotate row -> refused, nothing sent");
        TeachSt02InstallOps(0);
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] btnSht1GoLatch / btnSht2GoLatch\n");
    {
        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        be.opened.assign(4, true);
        ops.sht[kSt02MotInShuttle1] = 11; ops.sht[kSt02MotInShuttle2] = 12;
        ops.techRight[1] = 12345; ops.techRight[2] = 23456;
        be.table["MInShutte1"] = Axis1203(11, 30, 1, 2); be.aliasOf[11] = "MInShutte1";
        MotorGolden g = Golden(1.0, 10000, 100, 100000.0, 1); g.inShuttle = 1; be.golden[11] = g;
        be.cmdPos[2] = 0.0;
        be.table["MInShutte2"] = AxisOther(12, "SMC", false); be.aliasOf[12] = "MInShutte2";
        MotorGolden g2 = Golden(0.306, 10000, 100, 75.0, 1); g2.inShuttle = 2; be.golden[12] = g2;
        MotorAccessReq s1 = St02Req("btnSht1GoLatch", "MInArmX"); s1.num["shtSpeed"] = 50;
        MotorAccessReq s2 = St02Req("btnSht2GoLatch", "MInArmX"); s2.num["shtSpeed"] = 50;

        MotorAccessOutcome o = TeachSt02Dispatch(s1, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "grpShtSensor") && Has(o.ackJson, "uteach.dfm:5850") && be.AllCalls() == 0 && TeachSt02States().shuttleSelect == 0,
              "golden never shows grpShtSensor -> refused on every machine, nothing touched");
        ops.cfg.shtSensorGroupShown = true;                             // test only: exercise the body
        be.golden[11].homeFlag = 0;
        o = TeachSt02Dispatch(s1, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "motor need home") && ops.initMot.empty() && TeachSt02States().shuttleSelect == 0 && be.executed.empty(),
              "HomeFlag==0 -> 'motor need home' (golden :4723-4727), before iShuttleSelect / InitMOTParameter");
        be.golden[11].homeFlag = 1;
        MotorAccessReq ns = St02Req("btnSht1GoLatch", 0);
        o = TeachSt02Dispatch(ns, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "缺 shtSpeed") && ops.initMot.empty(), "no shtSpeed -> refused");
        MotorAccessReq neg = s1; neg.num["shtSpeed"] = -7;
        o = TeachSt02Dispatch(neg, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "負數") && TeachSt02States().shuttleSelect == 0, "a negative EditSh1Speed -> refused ([W906], golden keypad 1..max)");
        o = TeachSt02Dispatch(s1, 21, be);
        const Pci1203Cmd* mv = LastOf(be, kCmdAxMoveAbs);
        const Pci1203Cmd* run = 0;
        for (std::size_t i = 0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxSetSpeed && be.executed[i].speed == kSpeedRun) run = &be.executed[i];
        CHECK(o.ok && mv && mv->axis == 2 && mv->value == 12345.0 && be.CountKind(kCmdAxSetSpeed) == 4 && run && run->value == 5000.0,
              "Sht1: SetSpeed(50) -> PTP velHigh 10000*50% = 5000, then MoveAbs(Tech.iInShuttle1Right 12345)");
        CHECK(TeachSt02States().shuttleSelect == 1 && ops.initMot.size() == 1 && ops.initMot[0] == 11 && !be.floodCalls.empty() && be.floodCalls.back() == 1,
              "iShuttleSelect=1, MOT[MInShuttle1].InitMOTParameter(), the floodgate interlock asked (golden MotorMovePosition :652)");
        CHECK(Has(o.ackJson, "\"techInShuttleRight\":12345") && Has(o.ackJson, "\"shuttleSelect\":1") && Has(o.ackJson, "\"shtSpeed\":50") &&
              Has(o.ackJson, "\"motorId\":\"MInShutte1\""), "ack: target, iShuttleSelect, speed, motor");
        be.floodReady = false;
        const std::size_t moveAbs = (std::size_t)be.CountKind(kCmdAxMoveAbs);
        o = TeachSt02Dispatch(s1, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "閘門") && Has(o.ackJson, "iShuttleSelect=1") && (std::size_t)be.CountKind(kCmdAxMoveAbs) == moveAbs && ops.initMot.size() == 2,
              "floodgate not open -> refused (no move); iShuttleSelect / InitMOTParameter already done (golden order)");
        be.floodReady = true;
        o = TeachSt02Dispatch(s2, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && TeachSt02States().shuttleSelect == 2 && ops.initMot.back() == 12 && be.moves.empty(),
              "Sht2 on HT9050 (MInShutte2 SMC Enable=0): refused, golden's iShuttleSelect=2 / InitMOTParameter done, the golden object not driven");
        ops.sht[kSt02MotInShuttle2] = 99;
        o = TeachSt02Dispatch(s2, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "不在這台的 Mot_Table"), "the shuttle motor not on this machine -> refused");
        TeachSt02InstallOps(0);
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] btnSetAllInArmZ_Move / btnSetAllOutArmZ_Move\n");
    {
        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        be.opened.assign(5, true);
        be.table["MInArmZA"] = Axis1203(3, 3, 0, 3);   be.aliasOf[3] = "MInArmZA";
        MotorGolden z = Golden(1.0, 800, 100, 8000.0, 1); z.armZ = true; be.golden[3] = z;  be.cmdPos[3] = 0.0;
        be.table["MInArmX"] = Axis1203(0, 1, 0, 4);    be.aliasOf[0] = "MInArmX";
        be.golden[0] = Golden(1.0, 800, 100, 8000.0, 1); be.cmdPos[4] = 0.0;              // armZ false
        be.table["MInArmZB"] = AxisOther(4, "MN200", false); be.aliasOf[4] = "MInArmZB";
        MotorGolden zb = Golden(0.85, 800, 100, 0.025, 1); zb.armZ = true; be.golden[4] = zb;
        be.table["MInArmZE"] = AxisOther(7, "MN200", true);  be.aliasOf[7] = "MInArmZE";
        be.golden[7] = zb;
        TeachSt02ArmZGrid& gi = ops.grid[0];
        gi.motRow = 3; gi.motCol = 3; gi.pickRow = 3; gi.pickCol = 2;
        gi.cells.push_back(Cell(0, 0, 3, true, true, 500));     // 1203 arm Z            -> moved
        gi.cells.push_back(Cell(0, 1, 0, true, true, 700));     // MInArmX, not an arm Z -> refused
        gi.cells.push_back(Cell(0, 2, 3, false, true, 900));    // outside iPickCol      -> skipped
        gi.cells.push_back(Cell(1, 0, 4, true, true, 600));     // Enable=0              -> refused
        gi.cells.push_back(Cell(1, 1, 9, true, true, 650));     // no Mot_Table row      -> refused
        gi.cells.push_back(Cell(2, 0, 7, true, false, 0));      // outside Prod          -> refused (pick), moved (to 0)
        gi.cells.push_back(Cell(2, 1, -1, true, false, 0));     // no motor              -> refused

        MotorAccessReq in = St02Req("btnSetAllInArmZ_Move", "MInArmX");
        ops.cfg.usePickerCount = 4;
        MotorAccessOutcome o = TeachSt02Dispatch(in, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "pnlInArmZ") && Has(o.ackJson, "uteach.cpp:1457") && be.AllCalls() == 0 && !TeachSt02States().setAllInArmZB,
              "USE_PICKER_COUNT=4 (HT9050): pnlInArmZ hidden -> refused, b untouched");
        ops.cfg.usePickerCount = kSt02Picker16;
        o = TeachSt02Dispatch(in, 31, be);
        const Pci1203Cmd* mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && Has(o.ackJson, "\"state\":\"error\"") && Has(o.ackJson, "\"result\":\"partial\"") && Has(o.ackJson, "\"moved\":1") &&
              Has(o.ackJson, "\"refused\":5") && Has(o.ackJson, "\"partial\":true"), "1st press: 1 moved, 5 refused -> state error, result partial");
        CHECK(mv && mv->axis == 3 && mv->value == 500.0 && be.CountKind(kCmdAxMoveAbs) == 1 && be.moves.empty(),
              "only MInArmZA moved, to Prod.ZInArm_Tray_Pick[0][0] = 500 (the [0][2] nozzle outside iPickCol skipped)");
        CHECK(Has(o.ackJson, "\"goldenB\":false") && Has(o.ackJson, "\"goldenBAfter\":true") && Has(o.ackJson, "\"direction\":\"pick\"") &&
              TeachSt02States().setAllInArmZB && !TeachSt02States().setAllOutArmZB, "golden static b: false -> true (In only)");
        CHECK(Has(o.ackJson, "不是手臂 Z 軸") && Has(o.ackJson, "Enable=0") && Has(o.ackJson, "MOT[9]") && Has(o.ackJson, "Z 高度表外") && Has(o.ackJson, "MOT[-1]"),
              "each refusal says why (not an arm Z, Enable=0, no Mot_Table row, outside Prod, no motor)");
        CHECK(Has(o.ackJson, "\"axes\":[") && Has(o.ackJson, "\"motor\":\"MInArmZA\"") && !Has(o.ackJson, "\"col\":2"), "axes[] lists the six nozzles inside the pick grid");
        o = TeachSt02Dispatch(in, 32, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->value == 0.0 && be.moves.size() == 1 && be.moves[0].mi == 7 && be.moves[0].target == 0 && !be.moves[0].setSpeed &&
              Has(o.ackJson, "\"moved\":2") && Has(o.ackJson, "\"refused\":4") && Has(o.ackJson, "\"direction\":\"zero\"") && !TeachSt02States().setAllInArmZB,
              "2nd press: MotorMove(0) -- MInArmZA (1203) and MInArmZE (golden object, no speed); b back to false");
        // Out: every nozzle refused -> a refusal, b still flips; then no nozzle in the pick grid -> a refusal, b flips back
        TeachSt02ArmZGrid& go = ops.grid[1];
        go.motRow = 1; go.motCol = 1; go.pickRow = 1; go.pickCol = 1;
        go.cells.push_back(Cell(0, 0, 4, true, true, 111));
        MotorAccessReq out = St02Req("btnSetAllOutArmZ_Move", 0);
        const std::size_t ex = be.executed.size();
        o = TeachSt02Dispatch(out, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "moved 0") && Has(o.ackJson, "Enable=0") && TeachSt02States().setAllOutArmZB && be.executed.size() == ex,
              "Out, the only nozzle Enable=0: refused with the reason, golden b flipped anyway (Out's own b)");
        go.cells[0].inPick = false;
        o = TeachSt02Dispatch(out, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "沒有任何吸嘴") && !TeachSt02States().setAllOutArmZB, "no nozzle inside iPickRow x iPickCol: refused, b flips back");
        TeachSt02InstallOps(0);
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] through MotorAccessDispatch (the two hooks in WebMotorAccess.cpp)\n");
    {
        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        RotateRig(be, ops);
        CHECK(std::string(MotorAccessActionStatus("teachSt02")) == "live", "kActions has teachSt02 = live (claim: the teachGo row)");
        MotorAccessReq q = St02Req("St02State", 0); q.flag["query"] = true;
        MotorAccessOutcome o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"st02State\""), "MotorAccessDispatch routes teachSt02 to TeachSt02Dispatch (claim: the teachSet line)");
        if (!o.ok) std::printf("    dispatcher said: %s\n", o.ackJson.c_str());
        MotorAccessReq p = St02Req("SpeedButtonInRotatePos90", "MInRotate"); p.num["backlashIn"] = 0; p.num["backlashOut"] = 0;
        be.systemStart = true;
        o = MotorAccessDispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "SystemStart==true") && be.executed.empty(), "SystemStart -> refused by the dispatcher's run gate, nothing sent");
        be.systemStart = false;
        // a manual teach (golden fTeachShow is modal): a SetButton140 row on a non-1203 motor
        TeachRegRow w;
        w.owner = 'P'; w.mot0 = 5; w.name0 = "MInArmZC"; w.key0 = "k"; w.edit0 = "setEditSt02Test";
        w.setBtn = "SetButtonSt02Test"; w.setHandler = "SetButton140Click"; w.goBtn = "GoButtonSt02Test"; w.goHandler = "GoButton140Click";
        w.seq = 0; w.vis = true;
        be.reg.P.push_back(w);
        be.table["MInArmZC"] = AxisOther(5, "MN200", true); be.aliasOf[5] = "MInArmZC"; be.golden[5] = Golden(1.0, 800, 100, 0.1, 1);
        MotorAccessReq ts; ts.seq = 3; ts.id = "cmd-3"; ts.source = "uteach"; ts.action = "teachSet"; ts.button = "SetButton*"; ts.kind = "edit";
        ts.str["btn"] = "SetButtonSt02Test"; ts.flag["start"] = true; ts.flag["hand"] = true;   // AI(W906-ST02-C9) 20261002: main's TEACH-SVON (23e7bc2c) -- only hand=true is golden's hand teach (fTeachShow)
        o = MotorAccessDispatch(ts, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"teachActive\":true"), "a manual teach started (fixture)");
        if (!o.ok) std::printf("    teachSet said: %s\n", o.ackJson.c_str());
        o = MotorAccessDispatch(p, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "手動教導中") && be.executed.empty(), "during the manual teach -> refused by the dispatcher (golden fTeachShow is modal)");
        ts.flag["start"] = false; ts.flag["accept"] = false;
        o = MotorAccessDispatch(ts, 1, be);
        CHECK(o.ok && !MotorAccessJobs().teachActive, "the manual teach ended (fixture)");
        o = MotorAccessDispatch(p, 1, be);
        CHECK(o.ok && LastOf(be, kCmdAxMoveAbs) && LastOf(be, kCmdAxMoveAbs)->value == 3000.0, "after it: the rotate goes out (1000 + 2000)");
        MotorAccessReq stop; stop.seq = 4; stop.id = "cmd-4"; stop.source = "uteach"; stop.action = "stop"; stop.button = "btnStop"; stop.kind = "control";
        const int stops0 = be.CountKind(kCmdAxStop);
        o = MotorAccessDispatch(stop, 1, be);
        CHECK(o.ok && !be.stopAll.empty() && be.stopAll.back() == "uteach" && be.CountKind(kCmdAxStop) == stops0 + 5,
              "STOP (golden btnStopClick) = StopAllMotor + a 1203 stop on every opened axis, as for every teach move");
        TeachSt02InstallOps(0);
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- [11]  AI(W906-ST02-C9-G2) 20261002
    std::printf("[11] the 36 buttons on SpeedButtonInRotatePos90Click / Neg90Click (uteach.dfm), each with its golden parent\n");
    {
        static const char* const kSfx[16] = { "InRA", "InRB", "InRC", "InRD", "InRE", "InRF", "InRG", "InRH",
                                              "OutRA", "OutRB", "OutRC", "OutRD", "OutRE", "OutRF", "OutRG", "OutRH" };
        std::vector<std::string> all;
        all.push_back("SpeedButtonInRotatePos90");   all.push_back("SpeedButtonInRotateNeg90");     // uteach.dfm:5575 / :5583
        all.push_back("SpeedButtonOutRotatepPos90"); all.push_back("SpeedButtonOutRotatepNeg90");   // uteach.dfm:5671 / :5679
        for (int i = 0; i < 16; ++i) { all.push_back(std::string("btnPos90") + kSfx[i]); all.push_back(std::string("btnNeg90") + kSfx[i]); }
        int known = 0, positives = 0;
        bool parentsOk = true;
        for (std::size_t i = 0; i < all.size(); ++i) {
            bool p = false;
            std::string g;
            if (!TeachSt02RotateButton(all[i], p, g)) { std::printf("    not known: %s\n", all[i].c_str()); continue; }
            ++known;
            if (p) ++positives;
            const std::string want = (i < 4) ? std::string("grpRotate_Kit") : std::string("grb") + kSfx[(i - 4) / 2];
            if (g != want || p != (all[i].find("Pos90") != std::string::npos)) { parentsOk = false; std::printf("    %s -> %s %d\n", all[i].c_str(), g.c_str(), (int)p); }
        }
        CHECK(known == 36 && positives == 18 && parentsOk, "all 36 names known, 18 on the Pos90 handler, each with its golden parent (grpRotate_Kit / grbInRx / grbOutRx)");
        bool p = true;
        std::string g = "x";
        const bool none = !TeachSt02RotateButton("SpeedButtonOutRotatePos90", p, g) && !p && g.empty() && !TeachSt02RotateButton("btnPos90InRI", p, g) &&
                          !TeachSt02RotateButton("btnPos90", p, g) && !TeachSt02RotateButton("btnSht1GoLatch", p, g);
        CHECK(none, "not one of them: the Out kit name without golden's 'p', a grb letter I, a bare prefix, another G2 button");

        TeachSt02Config c;
        c.useRotateKit = 1;
        bool tabOk = true;
        for (int t = 0; t < 8; ++t) { c.rotateType = t; if (TeachSt02RotateTabShown(c) != (t == 2 || t == 3 || t == 5)) tabOk = false; }
        c.useRotateKit = 0; c.rotateType = kSt02Rot8Mot;
        CHECK(tabOk && !TeachSt02RotateTabShown(c), "tsRotate shown only for USE_ROTATE_KIT==1 && e4MotRotate (2) / e8MotRotate (3) / e2MotRotate2Dut (5) (golden :1668)");
        // what golden can press, per type: the tab AND the grb's own Visible (FormShow :1690-1703; grbInRA / grbInRE have no line)
        struct Want { int type; const char* shown; };
        const Want wants[] = {
            { kSt02Rot8Mot,     "InRA InRB InRC InRD InRE InRF InRG InRH OutRA OutRB OutRC OutRD OutRE OutRF OutRG OutRH" },
            { kSt02Rot4Mot,     "InRA InRB InRE InRF OutRA OutRB OutRE OutRF" },
            { kSt02Rot2Mot2Dut, "InRA InRE OutRC OutRG" },
            { kSt02Rot1Mot, "" }, { kSt02Rot1Mot1Dut, "" }, { kSt02RotInOutArm1Motor, "" }, { kSt02RotCyn, "" },
        };
        bool matOk = true;
        c.useRotateKit = 1;
        for (const Want& w : wants) {
            c.rotateType = w.type;
            const std::string sh = std::string(" ") + w.shown + " ";
            for (int i = 0; i < 16; ++i) {
                const bool want = sh.find(std::string(" ") + kSfx[i] + " ") != std::string::npos;
                const bool a = TeachSt02RotateButtonShown(c, std::string("btnPos90") + kSfx[i]);
                const bool b = TeachSt02RotateButtonShown(c, std::string("btnNeg90") + kSfx[i]);
                if (a != want || b != want) { matOk = false; std::printf("    type %d grb%s: %d %d, want %d\n", w.type, kSfx[i], (int)a, (int)b, (int)want); }
            }
            const bool kitWant = (w.type == kSt02Rot1Mot || w.type == kSt02Rot1Mot1Dut || w.type == kSt02RotInOutArm1Motor);
            if (TeachSt02RotateButtonShown(c, "SpeedButtonOutRotatepPos90") != kitWant || TeachSt02RotateButtonShown(c, "SpeedButtonInRotateNeg90") != kitWant) {
                matOk = false; std::printf("    type %d: kit buttons, want %d\n", w.type, (int)kitWant);
            }
        }
        c.useRotateKit = 0; c.rotateType = kSt02Rot8Mot;
        CHECK(matOk && !TeachSt02RotateButtonShown(c, "btnPos90InRA") && !TeachSt02RotateButtonShown(c, "btnNeg90InRE") &&
              !TeachSt02RotateButtonShown(c, "SpeedButtonInRotatePos90") && !TeachSt02RotateButtonShown(c, "btnSht1GoLatch"),
              "press matrix: e8 all 32, e4 the A / B / E / F groups, e2MotRotate2Dut grbInRA / grbInRE / grbOutRC / grbOutRG, the kit types none of them "
              "(the kit four instead); USE_ROTATE_KIT=0 none");
        CHECK(TeachSt02RotateGroupShown(c, "grbInRA") && !TeachSt02RotateGroupShown(c, "grbInRB") && !TeachSt02RotateGroupShown(c, "grpInRotM8"),
              "a group's own Visible: grbInRA has no FormShow line (true even with USE_ROTATE_KIT=0 -- the tab hides it), grbInRB false, a name with no rule false");

        Fresh();
        FakeBe be; FakeOps ops;
        TeachSt02InstallOps(&ops);
        RotateRig(be, ops);
        // tsRotate on e8 / e4 / e2MotRotate2Dut: P2 800 (golden :4613-4616) on the page's selected motor (MInRotateB, MN200, P1 200)
        const int types[3] = { kSt02Rot8Mot, kSt02Rot4Mot, kSt02Rot2Mot2Dut };
        const char* const btns[3] = { "btnPos90InRC", "btnNeg90OutRB", "btnPos90OutRG" };   // grbInRC e8 only / grbOutRB not 2Dut / grbOutRG e8 or 2Dut
        const int targets[3] = { 1000, -600, 1000 };
        for (int k = 0; k < 3; ++k) {
            ops.cfg.rotateType = types[k];
            const std::size_t mv0 = be.moves.size();
            MotorAccessOutcome o = TeachSt02Dispatch(St02Req(btns[k], "MInRotateB"), 1, be);
            const std::string label = std::string("type ") + std::to_string(types[k]) + ": " + btns[k] + " on MInRotateB -> MOT.MotorMove(200 " +
                                      (targets[k] > 200 ? "+" : "-") + " 800 = " + std::to_string(targets[k]) + "), P2 800, no backlash (not a kit motor)";
            CHECK(o.ok && be.moves.size() == mv0 + 1 && be.moves.back().mi == 65 && be.moves.back().target == targets[k] && !be.moves.back().setSpeed &&
                  Has(o.ackJson, "\"p2\":800") && Has(o.ackJson, std::string("\"st02Btn\":\"") + btns[k] + "\"") && !Has(o.ackJson, "\"backlash\""), label.c_str());
            if (!o.ok) std::printf("    said: %s\n", o.ackJson.c_str());
        }
        CHECK(ops.fcmd.size() == 3, "golden fCMD=false once per press");
        // hidden -> refused before anything (no CheckCanMove, no fCMD, nothing sent)
        const int calls = be.AllCalls();
        ops.cfg.rotateType = kSt02Rot4Mot;
        MotorAccessOutcome o = TeachSt02Dispatch(St02Req("btnPos90InRC", "MInRotateB"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "golden 看不到這顆按鈕") && Has(o.ackJson, "grbInRC->Visible=") && Has(o.ackJson, "uteach.cpp:1691") && be.AllCalls() == calls,
              "e4: btnPos90InRC (grbInRC, e8 only) refused (golden :1691)");
        ops.cfg.rotateType = kSt02Rot2Mot2Dut;
        o = TeachSt02Dispatch(St02Req("btnNeg90InRB", "MInRotateB"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "grbInRB->Visible=") && Has(o.ackJson, "uteach.cpp:1690") && be.AllCalls() == calls,
              "e2MotRotate2Dut: btnNeg90InRB (grbInRB, not 2Dut) refused (golden :1690)");
        ops.cfg.rotateType = kSt02Rot1Mot;
        o = TeachSt02Dispatch(St02Req("btnPos90InRA", "MInRotateB"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "tsRotate->TabVisible") && Has(o.ackJson, "uteach.cpp:1668") && be.AllCalls() == calls,
              "e1MotRotate (a kit type): the tsRotate tab hidden -> btnPos90InRA (no group rule of its own) refused (golden :1668)");
        ops.cfg.useRotateKit = 0; ops.cfg.rotateType = kSt02Rot8Mot;
        o = TeachSt02Dispatch(St02Req("btnNeg90OutRH", "MInRotateB"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "uteach.cpp:1668") && be.AllCalls() == calls, "USE_ROTATE_KIT=0 (HT9050), type e8: the tab hidden -> refused");
        ops.cfg.useRotateKit = 1;
        o = TeachSt02Dispatch(St02Req("SpeedButtonOutRotatepPos90", "MInRotateB"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "grpRotate_Kit->Visible") && Has(o.ackJson, "uteach.cpp:1652") && be.AllCalls() == calls && ops.fcmd.size() == 3,
              "e8: the kit buttons hidden (grpRotate_Kit, golden :1652) while the tsRotate ones are shown; no fCMD write on any refusal");

        // the backlash follows ActiveMotorIndex (golden :4639-4652 tests the motor, never Sender)
        ops.cfg.rotateType = kSt02Rot1Mot;
        MotorAccessReq oo = St02Req("SpeedButtonOutRotatepPos90", "MOutRotate"); oo.num["backlashIn"] = 30; oo.num["backlashOut"] = 7;
        be.lastDirP[42] = false;
        o = TeachSt02Dispatch(oo, 1, be);
        const Pci1203Cmd* mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 1 && mv->value == 2507.0 && Has(o.ackJson, "\"backlash\":7") && Has(o.ackJson, "\"st02Group\":\"grpRotate_Kit\""),
              "Out-kit +90 on MOutRotateKit: edtEditRotateOutBacklash 7 -> 500 + 2000 + 7 = 2507");
        MotorAccessReq oi = oo; oi.motors[0] = "MInRotate";
        be.lastDirP[41] = false;
        o = TeachSt02Dispatch(oi, 1, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 0 && mv->value == 3030.0 && Has(o.ackJson, "\"backlash\":30"),
              "the SAME Out-kit button on MInRotateKit: the In backlash 30 -> 1000 + 2000 + 30 = 3030 (the motor decides, not the button)");
        MotorAccessReq io = St02Req("SpeedButtonInRotateNeg90", "MOutRotate"); io.num["backlashIn"] = 30; io.num["backlashOut"] = 0;
        be.prodBacklashOut = 12; be.lastDirP[42] = true;
        o = TeachSt02Dispatch(io, 1, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 1 && mv->value == -1512.0 && Has(o.ackJson, "\"backlash\":-12"),
              "In-kit -90 on MOutRotateKit: the Out backlash (page 0 -> Prod's 12) -> 500 - 2000 - 12 = -1512");
        ops.cfg.rotateType = kSt02Rot8Mot;
        MotorAccessReq to = St02Req("btnPos90OutRA", "MOutRotate"); to.num["backlashIn"] = 30; to.num["backlashOut"] = 7;
        be.lastDirP[42] = false;
        o = TeachSt02Dispatch(to, 1, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 1 && mv->value == 1307.0 && Has(o.ackJson, "\"p2\":800") && Has(o.ackJson, "\"st02Group\":\"grbOutRA\""),
              "e8 btnPos90OutRA on MOutRotateKit (MotorOutRA): 500 + 800 + the Out backlash 7 = 1307");
        MotorAccessReq ti = St02Req("btnNeg90OutRH", "MInRotate"); ti.num["backlashIn"] = 30; ti.num["backlashOut"] = 7;
        be.lastDirP[41] = true;
        o = TeachSt02Dispatch(ti, 1, be);
        mv = LastOf(be, kCmdAxMoveAbs);
        CHECK(o.ok && mv && mv->axis == 0 && mv->value == 170.0 && Has(o.ackJson, "\"backlash\":-30"),
              "e8 btnNeg90OutRH (an Out-side button) on MInRotateKit: the In backlash -> 1000 - 800 - 30 = 170");
        ops.cfg.rotateType = kSt02Rot1Mot;
        const std::size_t mvn = be.moves.size();
        o = TeachSt02Dispatch(St02Req("SpeedButtonOutRotatepNeg90", "MInRotateB"), 1, be);   // no backlash fields
        CHECK(o.ok && be.moves.size() == mvn + 1 && be.moves.back().mi == 65 && be.moves.back().target == -1800 && !Has(o.ackJson, "\"backlash\""),
              "Out-kit -90 on MInRotateB: not a kit motor -> no backlash and the fields are not needed: 200 - 2000 = -1800");

        // the state query's new keys
        MotorAccessReq q = St02Req("St02State", 0); q.flag["query"] = true;
        ops.cfg.rotateType = kSt02Rot2Mot2Dut;
        o = TeachSt02Dispatch(q, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"tsRotate\":true") && Has(o.ackJson, "\"grpRotate_Kit\":false") && Has(o.ackJson, "\"grbInRA\":true") &&
              Has(o.ackJson, "\"grbInRB\":false") && Has(o.ackJson, "\"grbOutRC\":true") && Has(o.ackJson, "\"grbOutRH\":false") && Has(o.ackJson, "\"rotateP2\":800"),
              "state query, e2MotRotate2Dut: tsRotate shown, grpRotate_Kit hidden, the 16 groups' own Visible, P2 800");
        ops.cfg.useRotateKit = 0;
        o = TeachSt02Dispatch(q, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"tsRotate\":false") && Has(o.ackJson, "\"grbOutRC\":false"), "state query, USE_ROTATE_KIT=0: tsRotate hidden");
        TeachSt02InstallOps(0);
    }

    std::printf("%s: %d passed, %d failed\n", g_fail ? "FAIL" : "PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
