// =============================================================================
//  IndexZTorqueCore.h -- E-038 (Q87, SAFETY): HT9050's Index Z torque from the PCI-1203's
//  6077h, turned into the SAME number golden's Panasonic RS-232 reader puts in
//  fMain->edTorue0. PURE: no vendor header, no globals, no VCL -- both build arms, unit-
//  tested by tests/test_indexz_torque_1203.cpp.
//
//  AI(W906-E038) 20261003 (St01 / ST01-E): NEW FILE, NOT GOLDEN. Golden has no HT9050 and
//  no 1203. Scope / reasoning: D:\AI_TempFile\st01e-e038-scope-20261003.md; skill
//  .claude/skills/ht9045-motor-control/references/index-torque-autoheight.md section 6.
//  Steven 20261003 Q87:「把安川讀回來的值，轉換成跟原本國際牌馬達一樣的比例」「正負號是要比較的」
//  「向下的地心引力是負的，向上的反作用力是正的」「機台端會根據motor_table的設定，去決定目前馬達是向下
//  移動（負的）或是向上移動（正的）」.
//
//  WHAT GOLDEN DOES (golden 0618 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\rs232.cpp):
//    :833 / :909-910  request {0x00, address, 0x52, 0xAE-address} (Z1 = 0, Z2 = 1)
//    :1758 / :1804-1806  k = str[4]<<8 | str[3]  (short int: SIGNED 16 bit)
//    :1809-1810       if(k<0) k=0;              <- only the Panasonic-positive direction counts
//    :1811-1812       Torque = k/20.0; sprintf("%5.2f")  -> edTorue0 (:975) / PnlTorue0 (:976)
//    :991-995         case 40: Task=999, chkReadTorque1/2 cleared  <- ONE value per arm
//    cContact.cpp:5775 atoi(edTorue0); :5786-5787 Panasonic abs() (a no-op after the clamp);
//    :5789-5790 stop when TorqueData>=kg (kg = the % golden just wrote as the torque LIMIT).
//
//  WHAT THIS FILE DOES (all of it NOT GOLDEN, labelled per function):
//    pct   = raw6077 x 2704h:1 / 2704h:2            (% of rated; Yaskawa default 1/10)
//    v     = (raw - B) x 2704h:1 / 2704h:2 x pressSign   (B = 0 unless the baseline option is on)
//    pressSign = -1: Mot_Table M14 Direction = 0 -> card = flow (EtherCAT/Pci1203GaliRouteCore.cpp
//            :185-196 / :575), golden steps Z DOWN by lowering the position (cContact.cpp:6059 /
//            :6070), so pressing down is NEGATIVE in the drive's reference coordinates, which is
//            the sign 6077h carries (Yaskawa SIEP C710812 02H s5.4). Direction = 1 -> 0 = refused
//            (the route refuses that row too; no sign is guessed).
//    v<0 -> 0 exactly like golden :1809-1810; text = "%5.2f" exactly like golden :1812.
//    NO abs() of the raw value anywhere: a wrong-direction reading is clamped to 0 and can never
//    trigger early.
// =============================================================================
#ifndef IndexZTorqueCoreH
#define IndexZTorqueCoreH

#include <cstdio>
#include <string>
#include <algorithm>

namespace ht9045 {
namespace idxz {

// golden 0618 cContact.cpp:75 / :76 / :82 (the three contact modes whose flow compares a torque reading to kg)
enum { kModeAutoHeight = 1, kModeManualHeight = 2, kModeLoadCellHeight = 8 };
enum { kBaselineSamples = 5 };
//  NOT GOLDEN: golden alarms after MORE THAN 10 failed RS-232 transactions (rs232.cpp:861-871),
//  each one iPanasonicWT=500 ticks long (:762) -- tens of seconds. 10 s here, so the message
//  does not repeat faster than golden's did.
const unsigned long kReadErrorMs  = 10000ul;
//  NOT GOLDEN: golden case 555 / 7170 wait for edTorue0 with NO timeout (cContact.cpp:5766-5771,
//  :6405-6410). Phase B's call sites stop Z after this long (W906_IndexZTorqueWaitTimedOut).
const unsigned long kTorqueWaitMs = 5000ul;

// ---- conversion ----------------------------------------------------------------------------
//  Same arithmetic as EtherCAT/Pci1203Monitor.cpp:3445 Pci1203TorquePercent (2704h:1 / :2 are
//  1..1073741823; 0 in either half = "not read", never "zero torque").
inline bool TorquePercent(long raw, unsigned long num, unsigned long den, double& pct)
{
    if (num == 0 || den == 0) return false;
    pct = (double)raw * (double)num / (double)den;
    return true;
}

//  Mot_Table Direction of M14 (golden database.cpp:2539-2551 -> cinitial.cpp:3579 Motor->Direction;
//  port database.cpp:2775-2790 -> cinitial.cpp:4044). false (0) -> -1 = down is negative.
//  true (1) -> 0 = refused (no value is ever produced).
inline int PressSign(bool motDirectionIsOne) { return motDirectionIsOne ? 0 : -1; }

//  The pressing value, signed, BEFORE golden's clamp. baselineRaw = 0 is golden (no baseline).
inline double PressValue(long raw, long baselineRaw, unsigned long num, unsigned long den, int pressSign)
{
    return ((double)raw - (double)baselineRaw) * (double)num / (double)den * (double)pressSign;
}

//  golden 0618 rs232.cpp:1809-1810 `if(k<0) k=0;`. Written as "> 0 ? v : 0" so that -0.0 also
//  becomes +0.0 and prints " 0.00", never "-0.00".
inline double GoldenClamp(double v) { return (v > 0.0) ? v : 0.0; }

//  golden 0618 rs232.cpp:1812 sprintf(str2, "%5.2f", ...).
inline std::string GoldenText(double v)
{
    char b[48];
    std::snprintf(b, sizeof(b), "%5.2f", v);
    return std::string(b);
}

//  Median of n raw values (n <= kBaselineSamples).
inline long MedianRaw(const long* v, int n)
{
    long t[kBaselineSamples];
    if (n <= 0) return 0;
    if (n > (int)kBaselineSamples) n = (int)kBaselineSamples;
    for (int i = 0; i < n; ++i) t[i] = v[i];
    std::sort(t, t + n);
    return t[n / 2];
}

// ---- one sample of the 1203 monitor, as the live hook copies it ----------------------------
struct Sample {
    bool          haveMonitor;  // the hook reached the monitor and M14's axis slot
    unsigned long poll;         // Pci1203CardSample::pollCount of the last completed Poll
    bool          torqueValid;  // Pci1203AxisSample::torqueValid (THIS poll only; cleared every poll)
    int           src;          // 0 none / 1 PDO / 2 SDO 6077h (Pci1203AxisSample::torqueSrc)
    long          raw;          // [Trq.unit]
    bool          pctValid;     // monitor's torquePctValid: unit verified AND 2704h read
    unsigned long num, den;     // 2704h:1 / :2 (0 = not read)
    bool          servoOn;
    bool          alarm;
    bool          focusOnAxis;  // the monitor's ONE 6077h SDO focus is on M14
    int           slot;         // monitor axis slot (log only)
    std::string   why;          // why something above is missing
    Sample() : haveMonitor(false), poll(0), torqueValid(false), src(0), raw(0), pctValid(false),
               num(0), den(0), servoOn(false), alarm(false), focusOnAxis(false), slot(-1) {}
};

// ---- the bridge: golden's one-shot read, fed by the 1203 ---------------------------------
struct BridgeIn {
    bool          chk1, chk2;       // fMain->chkReadTorque1 / 2 (the arm flags, golden rs232.cpp:841-858)
    bool          confirmed;        // [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED == 1
    bool          baselineOpt;      // [IndexDriver] HT9050_INDEXZ_TORQUE_BASELINE == 1 (port option, default off)
    bool          driverPanasonic;  // INDEX_DRIVER_TYPE == Panasonic_DRIVER (golden's branch: abs, step 300, limit = kg)
    int           pressSign;        // PressSign(); 0 = refused
    bool          hookPresent;      // W906_Pci1203TorqueReadHook installed (only together with the Index Z route)
    bool          sampleRead;       // the hook was called this tick and `sample` is this tick's copy
    Sample        sample;
    unsigned long nowMs;
    BridgeIn() : chk1(false), chk2(false), confirmed(false), baselineOpt(false), driverPanasonic(false),
                 pressSign(0), hookPresent(false), sampleRead(false), nowMs(0) {}
};

struct BridgeState {
    bool          armed;
    bool          armPollKnown;
    unsigned long armPoll;          // pollCount seen when this arm started: a value must come from a LATER poll
    unsigned long waitStartMs;      // start of the current read-error window
    bool          baselineWanted;   // W906_IndexZTorqueBaselineReset() asked for a new baseline
    bool          baselineReady;
    long          baseline;
    int           nBase;
    long          baseRaw[kBaselineSamples];
    bool          haveLastBasePoll;
    unsigned long lastBasePoll;
    std::string   lastWhy;
    BridgeState() : armed(false), armPollKnown(false), armPoll(0), waitStartMs(0), baselineWanted(false),
                    baselineReady(false), baseline(0), nBase(0), haveLastBasePoll(false), lastBasePoll(0)
    { for (int i = 0; i < (int)kBaselineSamples; ++i) baseRaw[i] = 0; }
};

enum { kBridgeWaiting = 0, kBridgeWrote = 1, kBridgeIdle = 2 };

struct BridgeOut {
    int         code;    // kBridgeWaiting / kBridgeWrote / kBridgeIdle
    double      value;   // after golden's clamp
    std::string text;    // golden "%5.2f"
    double      signedValue;   // before the clamp (log only)
    bool        alarm;   // show the golden-shaped "Read Index Zn Torque error" now
    std::string why;     // why no value this tick
    BridgeOut() : code(kBridgeWaiting), value(0.0), signedValue(0.0), alarm(false) {}
};

inline void BaselineRequest(BridgeState& st)
{
    st.baselineWanted = true;  st.baselineReady = false;  st.baseline = 0;  st.nBase = 0;  st.haveLastBasePoll = false;
}

//  One MainProc tick of golden's ReadTorque_Panasonic, with the 1203 as the source.
inline BridgeOut BridgeStep(BridgeState& st, const BridgeIn& in)
{
    BridgeOut o;
    if (!in.chk1 && !in.chk2) {                       // golden 0618 rs232.cpp:854-858: Task=1; iAlarmCT=0; return;
        st.armed = false;
        o.code = kBridgeIdle;
        return o;
    }
    if (!st.armed) {                                  // a new arm (golden: case 550 InitReadTorueTask + chkReadTorque1=true)
        st.armed        = true;
        st.armPollKnown = in.sampleRead && in.sample.haveMonitor;
        st.armPoll      = st.armPollKnown ? in.sample.poll : 0;
        st.waitStartMs  = in.nowMs;
    }
    const Sample& s = in.sample;
    std::string why;
    bool usable = false;
    if (!in.chk1)
        why = "Index Z2 (chkReadTorque2) has no 1203 torque source -- the bridge reads Index Z1 only (HT9050 has one index axis)";
    else if (!in.confirmed)
        why = "source not confirmed: Gerneral.ini [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED is not 1 (EastSun sets it after the E-10 measurements)";
    else if (!in.driverPanasonic)
        why = "[IndexDriver] INDEX_DRIVER_TYPE is not Panasonic -- golden's Mitsubishi branch writes kgTranToMitsubishikg(kg) as the limit, which is not the % this value is in";
    else if (in.pressSign == 0)
        why = "Mot_Table M14 (MTestZ1) Direction is not 0 -- the pressing sign is not derived for a flipped row";
    else if (!in.hookPresent)
        why = "the 1203 torque read hook is not installed (it installs only with the Index Z Galil route)";
    else if (!in.sampleRead || !s.haveMonitor)
        why = s.why.empty() ? std::string("no 1203 monitor sample for M14") : s.why;
    else if (!st.armPollKnown) {
        st.armPollKnown = true;  st.armPoll = s.poll;
        why = "waiting for a monitor poll after the arm";
    } else if (s.poll <= st.armPoll)
        why = "waiting for a monitor poll after the arm";
    else if (!s.focusOnAxis)
        why = "the 6077h SDO focus is not on M14 (Motor Test page on another axis?)" + (s.why.empty() ? std::string() : " -- " + s.why);
    else if (!s.torqueValid || s.src != 2)
        why = "no live 6077h SDO value this poll (src " + std::to_string(s.src) + ")" + (s.why.empty() ? std::string() : " -- " + s.why);
    else if (!s.pctValid || s.num == 0 || s.den == 0)
        why = "6077h unit not verified or 2704h:1 / :2 not read";
    else if (!s.servoOn)
        why = "M14 servo is off";
    else if (s.alarm)
        why = "M14 drive is in alarm";
    else
        usable = true;

    if (usable && in.baselineOpt) {                   // NOT GOLDEN, port option (default off)
        if (!st.baselineReady && !st.baselineWanted) {
            usable = false;
            why = "baseline option on, but no baseline was requested (W906_IndexZTorqueBaselineReset, Phase B case 536)";
        } else if (!st.baselineReady) {
            if (!st.haveLastBasePoll || s.poll != st.lastBasePoll) {
                st.baseRaw[st.nBase++] = s.raw;
                st.haveLastBasePoll = true;
                st.lastBasePoll = s.poll;
            }
            if (st.nBase >= (int)kBaselineSamples) {
                st.baseline = MedianRaw(st.baseRaw, st.nBase);
                st.baselineReady = true;
                st.baselineWanted = false;
                st.armPoll = s.poll;                  // a value never comes from a poll that was used for the baseline
            }
            usable = false;
            why = "collecting the gravity baseline (" + std::to_string(st.nBase) + "/" + std::to_string((int)kBaselineSamples) + ")";
        }
    }

    if (usable) {
        o.signedValue = PressValue(s.raw, in.baselineOpt ? st.baseline : 0L, s.num, s.den, in.pressSign);
        o.value = GoldenClamp(o.signedValue);         // golden 0618 rs232.cpp:1809-1810
        o.text  = GoldenText(o.value);                // golden 0618 rs232.cpp:1812
        o.code  = kBridgeWrote;
        st.armed = false;                             // golden case 40 (:991-995): one value per arm
        st.lastWhy.clear();
        return o;
    }
    o.code = kBridgeWaiting;
    o.why  = why;
    st.lastWhy = why;
    if ((unsigned long)(in.nowMs - st.waitStartMs) >= kReadErrorMs) {   // golden 0618 rs232.cpp:861-871 shape
        o.alarm = true;
        st.waitStartMs = in.nowMs;                    // golden: Task=1; iAlarmCT=0 -> keeps trying
    }
    return o;
}

// ---- P5: the torque-wait timeout for Phase B's call sites (NOT GOLDEN) ---------------------
struct TorqueWait {
    bool          active;
    unsigned long startMs;
    bool          fired;
    TorqueWait() : active(false), startMs(0), fired(false) {}
};
//  true ONCE when `waiting` has been true for >= limitMs; any tick with waiting == false resets it.
inline bool TorqueWaitStep(TorqueWait& w, bool waiting, unsigned long nowMs, unsigned long limitMs = kTorqueWaitMs)
{
    if (!waiting) { w = TorqueWait(); return false; }
    if (!w.active) { w.active = true; w.startMs = nowMs; w.fired = false; return false; }
    if (!w.fired && (unsigned long)(nowMs - w.startMs) >= limitMs) { w.fired = true; return true; }
    return false;
}

// ---- P4: may a torque-comparing contact mode start? (NOT GOLDEN) ---------------------------
struct HeightGateIn {
    int  contactMode;       // iContactMode
    int  arm;               // 0 = Index Z1 (Do_Z1_AutoGetHeight / Load Cell iIndex 0), 1 = Z2
    bool z1Is1203;          // MOT[MTestZ1].CardType == "PCI1203" (same test as rs232.cpp:950)
    bool confirmed;
    bool driverPanasonic;
    bool directionIsOne;    // Mot_Table M14 Direction == 1 (or the motor object is missing)
    bool hookInstalled;
    bool baselineOn;        // AI(W906-E042): [IndexDriver] HT9050_INDEXZ_TORQUE_BASELINE == 1 -- refused (Steven 1004 Q90 = A: no baseline)
    HeightGateIn() : contactMode(0), arm(0), z1Is1203(false), confirmed(false), driverPanasonic(false),
                     directionIsOne(false), hookInstalled(false), baselineOn(false) {}
};
inline bool ModeComparesTorque(int mode)
{
    return mode == (int)kModeAutoHeight || mode == (int)kModeManualHeight || mode == (int)kModeLoadCellHeight;
}
inline bool HeightAllowed(const HeightGateIn& g, std::string& why)
{
    why.clear();
    if (!ModeComparesTorque(g.contactMode)) return true;   // contact test etc.: not this card's concern
    if (!g.z1Is1203) return true;                          // every RS-232 machine: golden, unchanged
    if (g.arm != 0) { why = "Index Z2 has no torque source on a PCI-1203 Index Z machine (HT9050 has one index axis)"; return false; }
    if (!g.confirmed) { why = "the 1203 torque source is not confirmed (Gerneral.ini [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED is not 1; E-10)"; return false; }
    if (g.baselineOn) { why = "the gravity-baseline option is ruled out (Steven 1004 Q90 = A: no baseline, as golden) -- remove [IndexDriver] HT9050_INDEXZ_TORQUE_BASELINE=1 from Gerneral.ini"; return false; }   // AI(W906-E042) 20261004
    if (!g.driverPanasonic) { why = "[IndexDriver] INDEX_DRIVER_TYPE is not Panasonic (the limit would not be in % of rated)"; return false; }
    if (g.directionIsOne) { why = "Mot_Table M14 (MTestZ1) Direction is not 0 (pressing sign not derived)"; return false; }
    if (!g.hookInstalled) { why = "the 1203 torque read hook is not installed (Index Z Galil route not installed)"; return false; }
    return true;
}

// =============================================================================================
//  E-042 (Phase B, step B1) additions -- AI(W906-E042) 20261004 (St01 / ST01-E), all NOT GOLDEN.
//  Plan D:\AI_TempFile\st01e-e042-plan-20261004.md s3; Steven 1004 07:5x 「io 馬達的裝置有異常error的時候，
//  機台就不可以動」 (stop is never blocked).  Pure: the live inputs are gathered in IndexZTorque1203.cpp.
// =============================================================================================

// ---- P7: M14 drive health (checked at entry and on every tick of a Z state machine) --------
//  One copy of the 1203 monitor's view of M14, as the live hook (EtherCAT/Pci1203TorqueRead.cpp) fills it.
struct Health {
    bool          haveMonitor;  // the hook reached the monitor and M14's slot (false: `why` says why)
    unsigned long poll;         // Pci1203CardSample::pollCount of the last completed Poll
    bool          sampleValid;  // Pci1203AxisSample valid && opened (this poll)
    bool          alarm;        // motionIO & AX_MOTION_IO_ALM (0x2)       -- as the route's TI byte, Pci1203GaliRouteCore.cpp:444
    bool          errorStop;    // (state & 0xFF) == STA_AX_ERROR_STOP (3) -- as Pci1203GaliRouteCore.cpp:55 / :444
    bool          servoOn;      // motionIO & AX_MOTION_IO_SVON (0x4000)   -- as WebMotorAccessLive.cpp:1242
    bool          routeFault;   // the Index Z route latched a failed motion / stop (Pci1203GaliRouteFaultFor)
    std::string   routeWhy;
    std::string   why;
    Health() : haveMonitor(false), poll(0), sampleValid(false), alarm(false), errorStop(false), servoOn(false),
               routeFault(false) {}
};
const int           kHealthInvalidPolls = 3;       // as the route's kInvalidPolls (Pci1203GaliRouteCore.cpp:67)
const unsigned long kHealthFrozenMs     = 2000ul;  // the poll counter did not move for this long while the tick ran
const unsigned long kTickGapMs          = 1000ul;  // two checks further apart than this = the tick itself was blocked
                                                   // (golden MySleep(3000), cContact.cpp:5857) -- not counted as frozen
struct HealthState {
    bool          haveCheck;
    unsigned long lastCheckMs;
    bool          havePoll;
    unsigned long lastPoll;
    unsigned long unchangedMs;
    int           invalidPolls;
    bool          haveInvalidPoll;
    unsigned long lastInvalidPoll;
    HealthState() : haveCheck(false), lastCheckMs(0), havePoll(false), lastPoll(0), unchangedMs(0), invalidPolls(0),
                    haveInvalidPoll(false), lastInvalidPoll(0) {}
};
//  true = the machine must not move M14 now (why says which device error).  hookPresent false = no way to see
//  the drive on a PCI1203 row, which counts as a fault (refuse rather than guess).
inline bool HealthFault(HealthState& st, const Health& h, bool hookPresent, unsigned long nowMs, std::string& why)
{
    why.clear();
    // the frozen-poll clock: only time during which the tick really ran is counted
    if (st.haveCheck && (unsigned long)(nowMs - st.lastCheckMs) > kTickGapMs) st.unchangedMs = 0;
    else if (st.haveCheck && h.haveMonitor && st.havePoll && h.poll == st.lastPoll) st.unchangedMs += (unsigned long)(nowMs - st.lastCheckMs);
    else st.unchangedMs = 0;
    st.haveCheck = true;  st.lastCheckMs = nowMs;
    if (h.haveMonitor) { st.havePoll = true; st.lastPoll = h.poll; }
    if (!hookPresent)   { why = "no 1203 health hook in this process (the Index Z route is not installed) -- the drive cannot be seen"; return true; }
    if (!h.haveMonitor) { why = h.why.empty() ? std::string("no 1203 monitor sample for M14") : h.why; return true; }
    if (h.routeFault)   { why = "the Index Z route latched a failure: " + h.routeWhy; return true; }
    if (!h.sampleValid) {
        if (!st.haveInvalidPoll || h.poll != st.lastInvalidPoll) { ++st.invalidPolls; st.haveInvalidPoll = true; st.lastInvalidPoll = h.poll; }
        if (st.invalidPolls >= kHealthInvalidPolls) { why = "M14 monitor sample invalid for " + std::to_string(st.invalidPolls) + " polls (card / slot lost)"; return true; }
    } else {
        st.invalidPolls = 0;  st.haveInvalidPoll = false;
        if (h.alarm)     { why = "M14 drive ALARM (AX_MOTION_IO_ALM)"; return true; }
        if (h.errorStop) { why = "M14 axis in ERROR_STOP"; return true; }
        if (!h.servoOn)  { why = "M14 servo is OFF"; return true; }
    }
    if (st.unchangedMs >= kHealthFrozenMs) { why = "1203 monitor frozen: poll " + std::to_string(h.poll) + " unchanged for " + std::to_string(st.unchangedMs) + " ms"; return true; }
    return false;
}

// ---- P8: which contact runs a PCI-1203 Index Z (HT9050: one index arm) may start at all -----------
//  Steven 1004 08:4x: the whitelist is mode 1 (Auto Height) AND mode 3 (Contact Test), single arm (Index arm 1) first;
//  Load Cell (8) refused; Manual Height (2) stays refused on HT9050 (golden's two manual methods: plan s11 -- one jogs
//  with Galil JG, which the 1203 route refuses and latches; the other turns the Z1 servo OFF, which P7 trips on).
enum { kModeContactTest = 3 };                             // golden 0618 cContact.cpp:77 CONTACT_TEST
inline bool RunModeWhitelisted(int mode) { return mode == (int)kModeAutoHeight || mode == (int)kModeContactTest; }
struct RunGateIn {
    HeightGateIn h;                 // P4 inputs (contactMode / arm / z1Is1203 / confirmed / baselineOn / driver / direction / hook)
    bool shuttleModeSingle;         // TestIF.iShuttleMode == 1 (golden :12716 / :12785 / :12819 single-shuttle branches)
    int  shuttleSel;                // TestIF.iShuttle_Sel (0 = shuttle 1 -> Index arm 1 only)
    bool arm2Options;               // IniConfig.bIndexArm2SupplyLight || TestIF_File.bForEgisTecTest ||
                                    // (IniConfig.bD58UseArm1PickPlaceArm2Test && TestIF_File.bArm1PickPlaceArm2Test) (golden :13012-13016, :13102-13106)
    bool twoArm32Site;              // bUseTwoArm32Site (golden case 500, Do_ContactTest_32Site)
    bool rtcLearning;               // REAL_TIME_CCD && !COM2->bCCDDummyRum && cbRTCModel (golden :13306)
    bool calibrateAbove;            // cbCalibrateAboveHeight (Do_Z1 case 3105, DoCalibrateAboveHeightZ1 not translated)
    bool latchTeach;                // the In-Shuttle latch teach (golden :12549-12553, CheckInShuttleSensor_Latch_Contact not translated)
    bool teachInOutArmZ;            // chkTeachInOutArmZ on an ASE customer (golden :11856-11858 / :12743-12744)
    RunGateIn() : shuttleModeSingle(false), shuttleSel(0), arm2Options(false), twoArm32Site(false), rtcLearning(false),
                  calibrateAbove(false), latchTeach(false), teachInOutArmZ(false) {}
};
inline bool RunAllowed(const RunGateIn& g, std::string& why)
{
    why.clear();
    if (!g.h.z1Is1203) return true;                        // every RS-232 machine: golden, unchanged
    HeightGateIn t = g.h;
    t.contactMode = (int)kModeAutoHeight;                  // the torque / switch gates apply to EVERY contact run on a 1203 Z
    if (!HeightAllowed(t, why)) return false;
    if (!RunModeWhitelisted(g.h.contactMode)) {
        why = "contact mode " + std::to_string(g.h.contactMode) + " is not enabled on a PCI-1203 Index Z (HT9050) -- only Auto Height (mode 1) and Contact Test (mode 3), single arm (Steven 1004 08:4x)";
        if (g.h.contactMode == (int)kModeManualHeight)   why += "; Manual Height stays refused: golden jogs with Galil JG (refused by the 1203 route) or turns the Z1 servo off";
        if (g.h.contactMode == (int)kModeLoadCellHeight) why += "; Load Cell always runs Index Z2 after Z1 and HT9050 has no Z2";
        return false;
    }
    if (!g.shuttleModeSingle || g.shuttleSel != 0) { why = "a contact run on HT9050 needs single shuttle, shuttle 1 (TestIF iShuttleMode 1, iShuttle_Sel 0): golden would run the Index arm 2 legs"; return false; }
    if (g.arm2Options)    { why = "an Index arm 2 option is on (arm-2 light / EgisTec / Arm1-pick-Arm2-test): HT9050 has no arm 2"; return false; }
    if (g.twoArm32Site)   { why = "32-site two-arm mode (Do_ContactTest_32Site) is not translated / not for HT9050"; return false; }
    if (g.rtcLearning)    { why = "Real Time CCD ROI learning is on (Do_ROILearning not translated)"; return false; }
    if (g.calibrateAbove) { why = "Calibrate Above Height is on (DoCalibrateAboveHeightZ1 not translated)"; return false; }
    if (g.latchTeach)     { why = "the In-Shuttle latch-sensor teach is on (not translated)"; return false; }
    if (g.teachInOutArmZ) { why = "the In / Out Arm Z auto teach is on (ASE option, not for HT9050)"; return false; }
    return true;
}

// ---- P9: the commanded-position floor --------------------------------------------------------
//  golden stops on the ENCODER reaching fIndexDownPos (cContact.cpp:5790).  If Z stalls above it and the torque
//  value never reaches kg, the next step starts from Gali_ReadPos() (the COMMAND, :5772) and walks down by
//  iStepSpeed every step.  Refuse a step whose start (the command, in mm as golden prints it) is at / below the floor.
inline bool CommandAtOrBelowFloor(double commandMm, double floorMm) { return commandMm <= floorMm; }

// ---- W-44: Index Z1 may press the SOCKET only with the In AND Out shuttles at home (NOT GOLDEN) -----------------
//  AI(W906-E042) 20261005 (B3, St01).  Steven's ruling W-44 (decisions-decided, review6 f4baf1d2); scope from ST01-M
//  1005 03:4x: SOCKET press only -- the pick / place descents (DoZ1/Z2PickFromShuttle, DoZPlaceToShuttle,
//  DoArm1/2PlaceToShuttle) need the shuttle parked UNDER the index and follow golden 0618 as is.
//  golden 0618 never checks this on its press lines (Do_Z1 530-565 / 7150-7170 / 720-730 / 2900+, cContact.cpp:5679-7556):
//  only Do_Z1 case 150/151 moves In shuttle 1 to InSHT[0].iLeft and Out shuttle 1 to OutSHT[0].iRight, and only when
//  USE_OUT_SHT_MOT==1 || Type_HT502 (:5527-5595); HT9050 has USE_OUT_SHT_MOT=0.
//  W44PressTask: the Do_Z1 / Do_Z2 tasks that belong to the socket press.  Excluded = golden's pre-descent tasks, where
//  Z is at / going to the safe height or home: 1 (Z to TestZ1_Safe), 100 (Index Y), 110 (IC-fall check), 150 / 151
//  (shuttles to their waiting positions), 200 (Index Z home), 520 (mode dispatch), 525 (delay), 2200 (check after rising).
inline bool W44PressTask(int task)
{
    switch (task) {
        case 1: case 100: case 110: case 150: case 151: case 200: case 520: case 525: case 2200: return false;
        default: return true;
    }
}
//  "At home" (Steven's word) = the axis within kW44HomeTolCounts of its CLEAR position: In shuttle 1 (M11) at Prod.InSHT[0].iLeft,
//  Out shuttle 1 (M17) at Prod.OutSHT[0].iRight -- exactly where golden Do_Z1 case 151 (:5573-5590) moves them before the
//  socket press (AI(W906-E042) 20261005 B3b, ST01-M: not 0).  A missing / unreadable axis is NOT at home (fail safe).
//  EastSun confirms the taught positions are clear of the socket (human-review A).
const long kW44HomeTolCounts = 100;
struct ShuttleHomeIn {
    bool inPresent, outPresent;
    long inPos, outPos;
    long inTarget, outTarget;       // Prod.InSHT[0].iLeft / Prod.OutSHT[0].iRight
    ShuttleHomeIn() : inPresent(false), outPresent(false), inPos(0), outPos(0), inTarget(0), outTarget(0) {}
};
inline bool ShuttlesAtHome(const ShuttleHomeIn& s, std::string& why)
{
    why.clear();
    const long dIn = s.inPos - s.inTarget, dOut = s.outPos - s.outTarget;
    const bool inHome  = s.inPresent  && (dIn  >= -kW44HomeTolCounts && dIn  <= kW44HomeTolCounts);
    const bool outHome = s.outPresent && (dOut >= -kW44HomeTolCounts && dOut <= kW44HomeTolCounts);
    if (inHome && outHome) return true;
    why = "W-44: Index Z1 may press the socket only with the In AND Out shuttles at home (In shuttle 1 ";
    why += s.inPresent ? ("at " + std::to_string(s.inPos)) : std::string("missing");
    why += " / InSHT[0].iLeft " + std::to_string(s.inTarget);
    why += inHome ? " ok" : " NOT home";
    why += ", Out shuttle 1 ";
    why += s.outPresent ? ("at " + std::to_string(s.outPos)) : std::string("missing");
    why += " / OutSHT[0].iRight " + std::to_string(s.outTarget);
    why += outHome ? " ok)" : " NOT home)";
    return false;
}

// ---- Place-back on HT9050 (NOT GOLDEN; Steven 1005 09:2x Q101) ------------------------------------------------------
//  AI(W906-E042) 20261005 (B4, St01).  golden DoZPlaceToShuttle / DoArm1/2PlaceToShuttle never move a shuttle: on
//  HT9045 the Index Y carries the IC back over the shuttle.  HT9050 has no Index Y and B3b parks the In shuttle at
//  InSHT[0].iLeft for the socket press, so before the place-back the In shuttle returns to InSHT[0].iRight.
//  Preconditions BEFORE that move (unreadable or not satisfied -> alarm + ST + golden exit, nothing moves):
//    Z1 at / above its safe height (Prod.TestZ1_Safe, -kW44HomeTolCounts), Out shuttle X (M17) at OutSHT[0].iRight +-tol.
struct PlaceBackIn {
    long z1Pos, z1Safe;
    bool outPresent; long outPos, outTarget;
    PlaceBackIn() : z1Pos(0), z1Safe(0), outPresent(false), outPos(0), outTarget(0) {}
};
inline bool PlaceBackPreconditions(const PlaceBackIn& p, std::string& why)
{
    why.clear();
    if (p.z1Pos < p.z1Safe - kW44HomeTolCounts) {
        why = "Index Z1 is not at its safe height (Z1 " + std::to_string(p.z1Pos) + ", TestZ1_Safe " + std::to_string(p.z1Safe) + ")";
        return false;
    }
    if (!p.outPresent) { why = "the Out shuttle X (M17) position cannot be read"; return false; }
    const long d = p.outPos - p.outTarget;
    if (d < -kW44HomeTolCounts || d > kW44HomeTolCounts) {
        why = "the Out shuttle X (M17) is not at OutSHT[0].iRight (at " + std::to_string(p.outPos) + ", target " + std::to_string(p.outTarget) + ")";
        return false;
    }
    return true;
}
inline bool AtTarget(long pos, long target) { const long d = pos - target; return d >= -kW44HomeTolCounts && d <= kW44HomeTolCounts; }

// ---- Contact modes that DoTestContactFunction cannot run yet on ANY machine (NOT GOLDEN, B4) -----------------------
//  Their leaf state machines are not translated (plan s1.2): 4 Do_AutoContactTest, 5 DoStepContact*, 9 DoDeviceMapCheck,
//  10 DoContactDeviceLoopTest, 11 DoContactKTemperatureTest; 12 VISUAL_DETECTION_TEST has no arm in golden 0618 at all
//  (V912 :11935 -> Task 60; TODO E-052).  Refusing at case 1 beats starting a run that would stop half way.
inline bool ModeUntranslated(int mode) { return mode == 4 || mode == 5 || mode == 9 || mode == 10 || mode == 11 || mode == 12; }

}  // namespace idxz
}  // namespace ht9045

#endif  // IndexZTorqueCoreH
