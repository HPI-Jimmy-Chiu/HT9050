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
    HeightGateIn() : contactMode(0), arm(0), z1Is1203(false), confirmed(false), driverPanasonic(false),
                     directionIsOne(false), hookInstalled(false) {}
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
    if (!g.driverPanasonic) { why = "[IndexDriver] INDEX_DRIVER_TYPE is not Panasonic (the limit would not be in % of rated)"; return false; }
    if (g.directionIsOne) { why = "Mot_Table M14 (MTestZ1) Direction is not 0 (pressing sign not derived)"; return false; }
    if (!g.hookInstalled) { why = "the 1203 torque read hook is not installed (Index Z Galil route not installed)"; return false; }
    return true;
}

}  // namespace idxz
}  // namespace ht9045

#endif  // IndexZTorqueCoreH
