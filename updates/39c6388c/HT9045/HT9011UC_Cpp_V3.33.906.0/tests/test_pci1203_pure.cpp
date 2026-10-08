// =============================================================================
//  test_pci1203_pure.cpp -- the pure decisions MT-E3a added to the 1203
//  monitor and control modules.
//
//  AI(W906-MT-E3a) 20260925. EastSun ruling R3 20260925: "write all the code
//  now (batch DI read with self-compare, torque read, whitelist additions) so
//  it is ready; nothing is run on the card now". So this file is the ONLY
//  evidence those additions have until the first on-machine run, and it says
//  exactly what it proves:
//
//    * Pci1203DiBatchCompare -- the port-by-port compare that is the only thing
//      allowed to switch the one-call DI read on;
//    * Pci1203TorquePercent / NewtonMetre / Compare -- the unit conversion and
//      the PDO/SDO agreement test (null-never-zero on a missing unit);
//    * golden InitMotor's CLOSED configuration table (kCmdAxSetInitCfg) and the
//      golden-order plan, against golden Motor/myEthercatmotor.cpp :219-361 and
//      SetEtherCatInType :1590-1690, value by value;
//    * the internal kinds have NO wire name (kSpeedMaxVel..Dec, setInitCfg);
//    * the not-linked monitor refuses a torque focus and publishes nothing.
//
//  AI(W906-MT-FIX1) 20260926: + the review fixes of 20260926 (sections 7-12):
//    * the batch-DI SPOT CHECK, against a fake per-byte reader -- including the
//      static-swap case the two compares could not see, which must be caught
//      in the Poll it starts to matter and must NOT stay in batch mode;
//    * the station-state event rule, the drive-error probe filter, the focus
//      SDO backoff, the per-poll torque clear;
//    * kCmdAxTorqueLimitSet's sequence and verdict against a FAKE SDO seam
//      (Pci1203TorqueLimitSdo) -- the same Pci1203TorqueLimitRun the live
//      kind runs, so the step order, the stop-at-first-failure rule, the
//      read-back compare and "no retry inside" are the real code's.
//    Still no vendor call: the fakes stand where the card would.
//
//  ⚠ WHAT IT DOES NOT PROVE: anything about the vendor calls. Built WITHOUT
//  HAVE_PCI1203, like every ctest target here, so Acm_DaqDiGetBytes /
//  Acm_AxGetActTorque / the 6077h SDO read are not even compiled in. Whether
//  the batch read's port order matches the per-byte one, and what unit
//  Acm_AxGetActTorque returns, are the questions the on-machine run answers.
//  Reads and writes no file.
// =============================================================================
#include "EtherCAT/Pci1203Monitor.h"
#include "EtherCAT/Pci1203Control.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

using namespace ht9045;

static int g_total = 0;
static int g_fail  = 0;

static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

// ---------------------------------------------------------------------------
//  AI(W906-MT-FIX1) 20260926: the fakes for sections 7 and 12.
// ---------------------------------------------------------------------------
//  The "card" behind the DI spot check: `real` is what a per-byte read of each
//  port returns; `perm[i]` is which REAL port the batch read puts at position i
//  (identity = a correct batch; a swap = the CTL-29 failure shape).
struct FakeDi {
    int           n;
    unsigned char real[16];
    int           perm[16];
    int           frozenPort;       // >= 0: the batch keeps returning frozenValue there
    unsigned char frozenValue;
    int           failPort;         // >= 0: the per-byte read of this port fails
    unsigned long failErr;
    int           reads;            // per-byte reads made
    bool          readSeen[16];
    FakeDi() : n(0), frozenPort(-1), frozenValue(0), failPort(-1), failErr(0), reads(0)
    {
        for (int i = 0; i < 16; ++i) { real[i] = 0; perm[i] = i; readSeen[i] = false; }
    }
    void Batch(unsigned char* out) const
    {
        for (int i = 0; i < n; ++i) out[i] = (i == frozenPort) ? frozenValue : real[perm[i]];
    }
};
static bool FakeDiRead(void* ctx, int port, unsigned char& value, unsigned long& err)
{
    FakeDi* f = static_cast<FakeDi*>(ctx);
    ++f->reads;
    if (port >= 0 && port < 16) f->readSeen[port] = true;
    if (port == f->failPort) { err = f->failErr; return false; }
    value = f->real[port];
    err = 0;
    return true;
}

//  The drive behind kCmdAxTorqueLimitSet: two UINT16 objects, a call log, one
//  injectable failure (by call number, 1..4) and an optional internal clamp.
class FakeTorqueLimitSdo : public Pci1203TorqueLimitSdo {
public:
    unsigned short obj[2];
    int            calls;
    std::string    order;          // "W+W-R+R-"
    int            failCall;       // 1-based call number that fails, 0 = none
    unsigned long  failRet;
    unsigned short clamp;          // 0 = none; else the drive stores min(value, clamp)
    FakeTorqueLimitSdo() : calls(0), failCall(0), failRet(0), clamp(0) { obj[0] = 3000; obj[1] = 3000; }
    unsigned long Write(int which, unsigned short value)
    {
        ++calls;
        order += (which == kTorqueLimitNeg) ? "W-" : "W+";
        if (calls == failCall) return failRet;
        obj[which == kTorqueLimitNeg ? 1 : 0] = (clamp != 0 && value > clamp) ? clamp : value;
        return 0;
    }
    unsigned long Read(int which, unsigned short& value)
    {
        ++calls;
        order += (which == kTorqueLimitNeg) ? "R-" : "R+";
        if (calls == failCall) return failRet;
        value = obj[which == kTorqueLimitNeg ? 1 : 0];
        return 0;
    }
};

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);

    // --- 1. batch DI compare --------------------------------------------------
    std::printf("-- 1. Pci1203DiBatchCompare\n");
    {
        const unsigned char a[4] = { 0x00, 0x5A, 0xFF, 0x01 };
        const bool          v[4] = { true, true, true, true };
        const Pci1203DiBatchCheck k = Pci1203DiBatchCompare(a, a, v, 4);
        check(k.full && k.ports == 4 && k.mismatches == 0 && k.unreadable == 0 &&
              k.firstMismatch == -1,
              "identical reads of 4 readable ports are a FULL match");
    }
    {
        const unsigned char b[4] = { 0x00, 0x5A, 0xFE, 0x01 };
        const unsigned char p[4] = { 0x00, 0x5A, 0xFF, 0x01 };
        const bool          v[4] = { true, true, true, true };
        const Pci1203DiBatchCheck k = Pci1203DiBatchCompare(b, p, v, 4);
        check(!k.full && k.mismatches == 1 && k.firstMismatch == 2 &&
              k.firstBatch == 0xFE && k.firstPerByte == 0xFF,
              "one differing byte: not full, and the report names port 2 with both values");
    }
    {
        //  The CTL-29 shape: a PERMUTATION of real bytes. Distinct values are
        //  caught port by port -- a multiset compare would have called it equal.
        const unsigned char b[3] = { 0x22, 0x11, 0x33 };
        const unsigned char p[3] = { 0x11, 0x22, 0x33 };
        const bool          v[3] = { true, true, true };
        const Pci1203DiBatchCheck k = Pci1203DiBatchCompare(b, p, v, 3);
        check(!k.full && k.mismatches == 2 && k.firstMismatch == 0,
              "two swapped ports with different values are 2 mismatches (compared by port, not by content)");
    }
    {
        //  ...but a swap of two ports that happen to hold the SAME value is
        //  invisible to any single compare. This is why batch mode needs TWO
        //  full matches at two moments: once the inputs differ, it shows.
        const unsigned char same[3]  = { 0x00, 0x00, 0x7F };
        const bool          v[3]     = { true, true, true };
        const Pci1203DiBatchCheck k1 = Pci1203DiBatchCompare(same, same, v, 3);
        const unsigned char batch2[3] = { 0x00, 0x04, 0x7F };   // wiring swapped: port1 shows port0's input
        const unsigned char per2[3]   = { 0x04, 0x00, 0x7F };   // the real image after one sensor changed
        const Pci1203DiBatchCheck k2 = Pci1203DiBatchCompare(batch2, per2, v, 3);
        check(k1.full && !k2.full && k2.mismatches == 2,
              "a swap of equal bytes passes ONE compare and fails the next once an input changes "
              "(the reason for two compares at different times)");
    }
    {
        const unsigned char a[3] = { 1, 2, 3 };
        const bool          v[3] = { true, false, true };
        const Pci1203DiBatchCheck k = Pci1203DiBatchCompare(a, a, v, 3);
        check(!k.full && k.unreadable == 1 && k.mismatches == 0,
              "a port whose per-byte read failed is not a mismatch -- but it blocks FULL");
    }
    {
        const unsigned char a[1] = { 1 };
        const bool          v[1] = { true };
        const Pci1203DiBatchCheck k0 = Pci1203DiBatchCompare(a, a, v, 0);
        const Pci1203DiBatchCheck kn = Pci1203DiBatchCompare(0, a, v, 1);
        check(!k0.full && k0.ports == 0 && !kn.full,
              "zero ports, or a missing buffer, can never switch batch mode on");
    }
    check(kPci1203DiBatchGapMs >= 200,
          "the 2nd compare is at least one io tick (wb_serve kIoTickMs = 200) after the 1st");
    check(kPci1203DiBatchReverifyMs == 300000 && kPci1203DiBatchRetryMs == 30000 &&
          kPci1203DiBatchFailDrop == 3,
          "re-compare every 5 min, retry a failed compare after 30 s, 3 failed calls drop batch mode");

    // --- 2. torque ------------------------------------------------------------
    std::printf("-- 2. torque conversion and the PDO/SDO agreement test\n");
    {
        double pct = -1.0;
        check(Pci1203TorquePercent(1000, 1, 10, pct) && near(pct, 100.0),
              "1000 [Trq.unit] at 2704h = 1/10 is 100 % of rated (this machine's measured unit)");
        check(Pci1203TorquePercent(-250, 1, 10, pct) && near(pct, -25.0),
              "the sign is kept (6077h follows the motor direction)");
        check(Pci1203TorquePercent(100, 1, 1, pct) && near(pct, 100.0),
              "the unit is the LIVE 2704h, not a constant: 1/1 gives 1 unit = 1 %");
        double untouched = 12345.0;
        check(!Pci1203TorquePercent(1000, 0, 10, untouched) &&
              !Pci1203TorquePercent(1000, 1, 0, untouched) && untouched == 12345.0,
              "an unread 2704h (0 in either half) is NOT a percentage -- false, nothing written");
    }
    {
        double nm = -1.0;
        check(Pci1203TorqueNewtonMetre(100.0, 1270, nm) && near(nm, 1.27),
              "100 % of a 1270 mN.m motor (6076h) is 1.27 N.m");
        check(Pci1203TorqueNewtonMetre(50.0, 9800, nm) && near(nm, 4.9),
              "50 % of 9800 mN.m is 4.9 N.m");
        double untouched = 7.0;
        check(!Pci1203TorqueNewtonMetre(50.0, 0, untouched) && untouched == 7.0,
              "an unread 6076h gives no N.m");
    }
    {
        check(Pci1203TorqueCompare(500, 500)  ==  1, "equal loaded readings agree");
        check(Pci1203TorqueCompare(500, 540)  ==  1, "within 10 % + 2 units agree");
        check(Pci1203TorqueCompare(500, 5000) == -1, "a x10 scale error disagrees");
        check(Pci1203TorqueCompare(50, 500)   == -1, "a /10 scale error disagrees");
        check(Pci1203TorqueCompare(300, -300) == -1, "opposite signs disagree");
        check(Pci1203TorqueCompare(5, -3)     ==  0,
              "an idle axis (both under kPci1203TorqueMinCompare) says nothing about the unit");
        check(Pci1203TorqueCompare(2147483647L, -32768L) == -1,
              "extreme values do not overflow the comparison");
    }
    check(kPci1203TorqueFocusMs == 3000 && kPci1203TorquePdoFailLatch == 3 &&
          kPci1203TorqueAgreeNeeded == 3,
          "focus expires 3 s after its last call; PDO latches after 3 failures; 3 agreeing pairs verify the PDO unit");

    // --- 3. the not-linked monitor --------------------------------------------
    std::printf("-- 3. the not-linked monitor (this ctest has no HAVE_PCI1203)\n");
    {
        TPci1203Monitor m;
        m.SetTorqueFocusAxis(0);
        check(m.card().torqueFocusAxis == -1 && !m.card().torqueFocusWhy.empty(),
              "a focus on an axis that has no open handle is REFUSED and says why");
        m.SetTorqueFocusAxis(-1);
        check(m.card().torqueFocusAxis == -1 && m.card().torqueFocusWhy.empty(),
              "SetTorqueFocusAxis(-1) clears the focus and the reason");
        const Pci1203AxisSample& a = m.axis(0);
        check(!a.torqueValid && a.torqueSrc == 0 && !a.torquePctValid && !a.torqueNmValid &&
              !a.torqueUnitVerified,
              "an axis nobody read publishes no torque -- invalid, source none (null, never 0)");
        check(!m.card().diBatchOk && m.card().diBatchMatches == 0,
              "a monitor that never opened is per-byte with no matches");
        check(!m.Poll(), "Poll() on a monitor that is not open returns false (no read at all)");
    }

    // --- 4. speed ceilings: no wire name ---------------------------------------
    std::printf("-- 4. Pci1203SpeedParam ceilings\n");
    check(kSpeedMaxVel == 10 && kSpeedMaxAcc == 11 && kSpeedMaxDec == 12 &&
          (int)kSpeedMaxDec == (int)Pci1203AxisSample::kSpeedCount - 1,
          "MaxVel/MaxAcc/MaxDec are 10/11/12 = the monitor's speed[10..12] read-back slots");
    {
        const char* names[] = { "pci1203.ax.setSpeed.maxVel", "pci1203.ax.setSpeed.maxAcc",
                                "pci1203.ax.setSpeed.maxDec", "pci1203.ax.setInitCfg",
                                "pci1203.ax.setInitCfg.ppu" };
        bool anyParsed = false;
        for (std::size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
            Pci1203WireCmd w;
            w.name = names[i]; w.target = "pci1203.ax0"; w.hasNum = true; w.num = 1000.0;
            Pci1203Cmd c; std::string why;
            if (Pci1203ParseWire(w, c, why)) anyParsed = true;
            if (Pci1203CmdFromName(names[i]) != kCmdNone) anyParsed = true;
        }
        check(!anyParsed,
              "no wire name reaches a ceiling or the InitMotor table (neither the pci1203 page nor a browser can send them)");
    }
    check(std::strstr(Pci1203CmdName(kCmdAxSetInitCfg), "(internal)") != 0,
          "the audit name of kCmdAxSetInitCfg says (internal)");
    {
        //  The other kinds golden InitMotor needs already exist (ResetError, SvOn,
        //  SetCmdPos, SetActPos) -- pinned so the Test Range path cannot lose one.
        check(std::strcmp(Pci1203CmdName(kCmdAxResetError), "pci1203.ax.resetError") == 0 &&
              std::strcmp(Pci1203CmdName(kCmdAxSvOn), "pci1203.ax.svOn") == 0 &&
              std::strstr(Pci1203CmdName(kCmdAxSetCmdPos), "(internal)") != 0 &&
              std::strstr(Pci1203CmdName(kCmdAxSetActPos), "(internal)") != 0,
              "ResetError / SvOn / SetCmdPos / SetActPos kinds exist for golden InitMotor's tail");
    }

    // --- 5. golden InitMotor's closed configuration table ----------------------
    std::printf("-- 5. kCmdAxSetInitCfg table (golden Motor/myEthercatmotor.cpp)\n");
    {
        static const char* kExpect[kInitCfgCount] = {
            "CFG_AxPPU", "CFG_AxElReact", "CFG_AxAlmEnable", "CFG_AxAlmReact",
            "CFG_AxOrgLogic", "PAR_AxJerk", "CFG_AxInpEnable", "CFG_AxInpLogic",
            "CFG_AxAlmLogic", "CFG_AxEzLogic", "CFG_AxErcLogic",
            "CFG_AxPulseInMode", "CFG_AxPulseOutMode",
            "CFG_AxJogVLTime"                                   //AI(W906-JOG-VLTIME) 20261003
        };
        bool namesOk = true, typesOk = true;
        for (int q = 0; q < kInitCfgCount; ++q) {
            if (std::strcmp(Pci1203InitCfgName(q), kExpect[q]) != 0) namesOk = false;
            if (Pci1203InitCfgIsF64(q) != (q == kInitCfgJerk)) typesOk = false;
            if (Pci1203InitCfgNotSupportedOk(q) != (q != kInitCfgJerk && q != kInitCfgJogVLTime)) typesOk = false;
        }
        check(namesOk, "the 14 rows are in enum order with the vendor spellings");
        check(typesOk,
              "only PAR_AxJerk is F64; PAR_AxJerk (golden :262) and CFG_AxJogVLTime (golden SetSpeed :724) are NOT excused for Dsp_PropertyIDNotSupport");
        check(Pci1203InitCfgValueOk(kInitCfgJogVLTime, 0.0) && !Pci1203InitCfgValueOk(kInitCfgJogVLTime, 100.0),
              "CFG_AxJogVLTime: golden writes 0 only (jog starts at JogVelHigh at once)");
        check(std::strcmp(Pci1203InitCfgName(-1), "?") == 0 &&
              std::strcmp(Pci1203InitCfgName(kInitCfgCount), "?") == 0 &&
              !Pci1203InitCfgValueOk(kInitCfgCount, 0.0),
              "an index outside the table has no name and no legal value");
    }
    {
        check(Pci1203InitCfgValueOk(kInitCfgPPU, 1.0) && !Pci1203InitCfgValueOk(kInitCfgPPU, 2.0),
              "PPU: golden writes 1, nothing else");
        check(Pci1203InitCfgValueOk(kInitCfgElReact, 0.0) && !Pci1203InitCfgValueOk(kInitCfgElReact, 1.0),
              "ElReact: golden writes 0 (immediate stop) -- 1 is refused here (the limit panel is a different command)");
        check(Pci1203InitCfgValueOk(kInitCfgOrgLogic, 0.0) && Pci1203InitCfgValueOk(kInitCfgOrgLogic, 1.0) &&
              !Pci1203InitCfgValueOk(kInitCfgOrgLogic, 2.0),
              "OrgLogic: 0 or 1 (bSensorType)");
        check(Pci1203InitCfgValueOk(kInitCfgPulseInMode, 2.0) && Pci1203InitCfgValueOk(kInitCfgPulseInMode, 3.0) &&
              !Pci1203InitCfgValueOk(kInitCfgPulseInMode, 0.0),
              "PulseInMode: AB_4X (2) or I_CW_CCW (3)");
        check(Pci1203InitCfgValueOk(kInitCfgPulseOutMode, 16.0) && Pci1203InitCfgValueOk(kInitCfgPulseOutMode, 8.0) &&
              Pci1203InitCfgValueOk(kInitCfgPulseOutMode, 4.0) && !Pci1203InitCfgValueOk(kInitCfgPulseOutMode, 1.0),
              "PulseOutMode: O_CW_CCW (0x10), OUT_DIR_ALL_NEG (0x08) or OUT_DIR_DIR_NEG (0x04)");
        check(Pci1203InitCfgValueOk(kInitCfgJerk, 0.0) && !Pci1203InitCfgValueOk(kInitCfgJerk, 1.0),
              "PAR_AxJerk: golden writes 0 (T-curve) only");
    }
    {
        //  golden InitMotor :219-361 + SetEtherCatInType :1590-1690, value by value.
        struct Exp { int which; double v; };
        Pci1203InitCfgStep s[kInitCfgCount + 2];

        //  Servo_Motor, bSensorType = true (OrgLogic 0), bIn1Logic = false (AlmLogic 1)
        const Exp servo[13] = {
            { kInitCfgPPU, 1 }, { kInitCfgElReact, 0 }, { kInitCfgAlmEnable, 1 }, { kInitCfgAlmReact, 0 },
            { kInitCfgOrgLogic, 0 }, { kInitCfgJerk, 0 },
            { kInitCfgInpEnable, 1 }, { kInitCfgInpLogic, 0 }, { kInitCfgAlmLogic, 1 },
            { kInitCfgEzLogic, 1 }, { kInitCfgErcLogic, 1 },
            { kInitCfgPulseInMode, 2 }, { kInitCfgPulseOutMode, 16 }
        };
        int n = Pci1203GoldenInitCfgPlan(kInitCfgMotorServo, true, false, s, kInitCfgCount + 2);
        bool same = (n == 13);
        for (int i = 0; same && i < 13; ++i)
            if (s[i].which != servo[i].which || s[i].value != servo[i].v) same = false;
        check(same, "Servo_Motor: 13 writes in golden order, OrgLogic from bSensorType, AlmLogic from bIn1Logic, AB_4X + O_CW_CCW");

        //  Rotate_Motor: AlmLogic is 0 whatever bIn1Logic says (golden :1639)
        n = Pci1203GoldenInitCfgPlan(kInitCfgMotorRotate, false, false, s, kInitCfgCount + 2);
        check(n == 13 && s[4].which == kInitCfgOrgLogic && s[4].value == 1.0 &&
              s[8].which == kInitCfgAlmLogic && s[8].value == 0.0 &&
              s[11].value == 3.0 && s[12].value == 8.0,
              "Rotate_Motor: AlmLogic 0 regardless of bIn1Logic, I_CW_CCW + OUT_DIR_ALL_NEG");

        //  anything else (stepper): no Inp pair, AlmLogic from bIn1Logic
        n = Pci1203GoldenInitCfgPlan(kInitCfgMotorOther, true, true, s, kInitCfgCount + 2);
        check(n == 11 && s[6].which == kInitCfgAlmLogic && s[6].value == 0.0 &&
              s[7].which == kInitCfgEzLogic && s[9].which == kInitCfgPulseInMode &&
              s[9].value == 3.0 && s[10].value == 4.0,
              "other motor types: 11 writes, no InpEnable/InpLogic, I_CW_CCW + OUT_DIR_DIR_NEG");

        check(Pci1203GoldenInitCfgPlan(7, true, true, s, kInitCfgCount) == 0,
              "an unknown motor class plans nothing");

        Pci1203InitCfgStep two[2];
        two[0].which = -9; two[1].which = -9;
        n = Pci1203GoldenInitCfgPlan(kInitCfgMotorServo, false, true, two, 2);
        check(n == 13 && two[0].which == kInitCfgPPU && two[1].which == kInitCfgElReact,
              "a short buffer is filled only up to its size, and the full count is still returned");

        //  Closure: every value the plan produces is one the table accepts.
        bool closed = true;
        const int classes[3] = { kInitCfgMotorServo, kInitCfgMotorRotate, kInitCfgMotorOther };
        for (int c = 0; c < 3; ++c)
            for (int st = 0; st < 2; ++st)
                for (int il = 0; il < 2; ++il) {
                    const int m = Pci1203GoldenInitCfgPlan(classes[c], st != 0, il != 0, s, kInitCfgCount + 2);
                    for (int i = 0; i < m; ++i)
                        if (!Pci1203InitCfgValueOk(s[i].which, s[i].value)) closed = false;
                }
        check(closed, "every value golden's plan writes passes the closed-table check (12 combinations)");
    }
    //  Execute() compares ElReact against limitVal[kLimitElReact] and PAR_AxJerk
    //  against speed[kSpeedJerk] (ExpectCfg_), so those two slots are pinned.
    check(kLimitElReact == 3 && kSpeedJerk == 8,
          "the two InitMotor entries the monitor reads back sit at limit slot 3 and speed slot 8");

    // --- 6. control refuses when not open --------------------------------------
    std::printf("-- 6. the not-linked control\n");
    {
        TPci1203Control ctl;
        std::string why;
        check(!ctl.Open(false, why) && why.find("not linked") != std::string::npos,
              "this binary cannot command: Open() says not linked");
        Pci1203Cmd c;
        c.kind = kCmdAxSetInitCfg; c.axis = 0; c.initCfg = kInitCfgPPU; c.value = 1.0;
        const Pci1203CmdResult r = ctl.Execute(c);
        check(!r.accepted && !r.issued, "kCmdAxSetInitCfg on a closed control is refused, nothing issued");
        check(Pci1203Cmd().initCfg == -1, "a default Pci1203Cmd names no InitMotor property");
    }

    // --- 7. batch DI spot check (review 20260926, finding 1) -------------------
    std::printf("-- 7. Pci1203DiBatchSpotCheck (fake per-byte reader)\n");
    {
        //  THE CASE THE TWO COMPARES COULD NOT SEE. Six ports on an idle machine;
        //  the batch read swaps ports 1 and 4, which both hold 0x00.
        FakeDi f;
        f.n = 6;
        const unsigned char idle[6] = { 0x00, 0x00, 0x7F, 0x10, 0x00, 0x22 };
        for (int i = 0; i < 6; ++i) f.real[i] = idle[i];
        f.perm[1] = 4; f.perm[4] = 1;
        const bool v6[6] = { true, true, true, true, true, true };

        unsigned char batch[6], prev[6];
        f.Batch(batch);
        const Pci1203DiBatchCheck c1 = Pci1203DiBatchCompare(batch, f.real, v6, 6);   // Open()
        f.Batch(batch);
        const Pci1203DiBatchCheck c2 = Pci1203DiBatchCompare(batch, f.real, v6, 6);   // a Poll >= 200 ms later
        check(c1.full && c2.full,
              "the gap, pinned: a swap of two equal-valued ports passes BOTH enabling compares on an "
              "idle machine (so the enabling rule alone cannot be what protects the engine)");
        std::memcpy(prev, batch, 6);                                                   // the verified image

        //  While the swapped values stay equal the batch delivers CORRECT data,
        //  and every Poll spot-checks only the 4 rolling ports.
        int cursor = 0;
        bool allOk = true, dataRight = true, costBounded = true;
        for (int poll = 0; poll < 30; ++poll) {
            f.Batch(batch);
            const Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(batch, prev, 6, cursor,
                                                                   kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
            if (!sp.ok) allOk = false;
            if (std::memcmp(batch, f.real, 6) != 0) dataRight = false;
            if (sp.checked != (int)kPci1203DiBatchSpotPorts || sp.changed != 0) costBounded = false;
            std::memcpy(prev, batch, 6);
        }
        check(allOk && dataRight,
              "30 idle Polls: the spot check passes, and the delivered bytes ARE the real ones "
              "(a permutation of equal values is harmless for as long as they stay equal)");
        check(costBounded, "an idle batch Poll costs exactly kPci1203DiBatchSpotPorts (4) per-byte reads");

        //  The sensor on port 4 turns ON. The batch shows it at position 1.
        f.real[4] = 0x08;
        f.Batch(batch);
        Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(batch, prev, 6, cursor,
                                                         kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
        check(!sp.ok && sp.mismatches == 1 && sp.firstMismatch == 1 &&
              sp.firstBatch == 0x08 && sp.firstPerByte == 0x00,
              "STATIC SWAP, the first Poll the swapped inputs differ: the changed position (port 1) is "
              "read per byte in the same Poll and disagrees -> !ok -> batch mode is dropped at once, "
              "it does NOT stay on undetected");

        //  Same, but the pulse is SHORTER than the gap between the batch read and
        //  the spot read (the rolling sample alone could miss it entirely).
        f.real[4] = 0x00;
        f.Batch(batch); std::memcpy(prev, batch, 6);                // idle again, verified
        f.real[4] = 0x08;
        f.Batch(batch);                                             // the batch caught the pulse...
        f.real[4] = 0x00;                                           // ...which is over before the spot read
        sp = Pci1203DiBatchSpotCheck(batch, prev, 6, cursor, kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
        check(!sp.ok && sp.firstMismatch == 1,
              "a pulse too short for the rolling sample is still caught: the swapped position reads "
              "its OWN (unchanged) value per byte");

        //  The other half of the pair changes instead.
        f.Batch(batch); std::memcpy(prev, batch, 6);
        f.real[1] = 0x40;
        f.Batch(batch);
        sp = Pci1203DiBatchSpotCheck(batch, prev, 6, cursor, kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
        check(!sp.ok && sp.firstMismatch == 4 && sp.firstBatch == 0x40 && sp.firstPerByte == 0x00,
              "port 1 changing instead shows up at position 4, and is caught there");
    }
    {
        //  A CORRECT batch (identity) with inputs moving: never a false drop.
        FakeDi f;
        f.n = 6;
        unsigned char batch[6], prev[6];
        f.Batch(prev);
        int cursor = 0;
        bool ok = true;
        for (int poll = 0; poll < 20; ++poll) {
            f.real[poll % 6] = (unsigned char)(poll * 7 + 1);
            f.Batch(batch);
            const Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(batch, prev, 6, cursor,
                                                                   kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
            if (!sp.ok || sp.changed != 1) ok = false;
            std::memcpy(prev, batch, 6);
        }
        check(ok, "a correct batch with one input changing per Poll: every changed port is read and agrees, no drop");
    }
    {
        //  A batch byte that FREEZES (not a permutation): its batch value stops
        //  changing, so only the rolling sample can see it -- within ceil(n/4) Polls.
        FakeDi f;
        f.n = 10;
        unsigned char batch[10], prev[10];
        f.Batch(prev);
        f.frozenPort = 7; f.frozenValue = 0x00;
        f.real[7] = 0x80;                                           // the real input moved and stays
        int cursor = 0, polls = 0;
        bool caught = false;
        for (; polls < 10 && !caught; ++polls) {
            f.Batch(batch);
            const Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(batch, prev, 10, cursor,
                                                                   kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
            if (!sp.ok) caught = (sp.firstMismatch == 7);
            else std::memcpy(prev, batch, 10);
        }
        check(caught && polls <= (10 + (int)kPci1203DiBatchSpotPorts - 1) / (int)kPci1203DiBatchSpotPorts,
              "a frozen batch byte is caught by the rolling sample within ceil(n/4) Polls");
    }
    {
        //  The rolling cursor visits EVERY port.
        FakeDi f;
        f.n = 9;
        unsigned char batch[9], prev[9];
        f.Batch(prev);
        int cursor = 0;
        for (int poll = 0; poll < 3; ++poll) {
            f.Batch(batch);
            (void)Pci1203DiBatchSpotCheck(batch, prev, 9, cursor, kPci1203DiBatchSpotPorts, &FakeDiRead, &f);
        }
        bool all = true;
        for (int i = 0; i < 9; ++i) if (!f.readSeen[i]) all = false;
        check(all, "ceil(9/4) = 3 idle Polls read every one of the 9 ports per byte at least once");
    }
    {
        //  An unreadable port, no baseline, no ports.
        FakeDi f;
        f.n = 4;
        unsigned char batch[4], prev[4];
        f.Batch(prev);
        f.Batch(batch);
        f.failPort = 2; f.failErr = 0x80001234ul;
        int cursor = 0;
        Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(batch, prev, 4, cursor, 4, &FakeDiRead, &f);
        check(!sp.ok && sp.unreadable == 1 && sp.firstUnreadable == 2 && sp.unreadableErr == 0x80001234ul,
              "a spot read that FAILS is not a pass: !ok, and the port and its return code are named");
        f.failPort = -1; f.reads = 0;
        cursor = 0;
        sp = Pci1203DiBatchSpotCheck(batch, 0, 4, cursor, 1, &FakeDiRead, &f);
        check(sp.ok && sp.checked == 4 && f.reads == 4,
              "no verified baseline (prev = 0): every port counts as changed -- a full per-byte compare");
        sp = Pci1203DiBatchSpotCheck(batch, prev, 0, cursor, 4, &FakeDiRead, &f);
        check(!sp.ok && sp.checked == 0, "zero ports can never pass a spot check");
    }
    check(kPci1203DiBatchSpotPorts == 4, "4 rolling spot reads per batch Poll");

    // --- 8. station-state events (review 20260926, finding 2) -------------------
    std::printf("-- 8. Pci1203SlaveStateEvent / InputsLive / MailboxOk\n");
    check(!Pci1203SlaveStateEvent(true, 0x08, 0, true, 0x08),
          "same state as last time: not an event (batch mode is not disturbed every Poll)");
    check(Pci1203SlaveStateEvent(true, 0x08, 0, false, 0),
          "a station that answered and now does not: an event");
    check(Pci1203SlaveStateEvent(true, 0x08, 0, true, 0x02),
          "OP -> PREOP: an event");
    check(Pci1203SlaveStateEvent(false, 0, 0x80000001ul, true, 0x08),
          "answering again after a failed read: an event (its bytes are re-verified)");
    check(!Pci1203SlaveStateEvent(false, 0, 0, true, 0x08),
          "a first successful read with no failure before it (e.g. not read at scan time): not an event");
    check(Pci1203SlaveInputsLive(0x08) && Pci1203SlaveInputsLive(0x04) && Pci1203SlaveInputsLive(0x18) &&
          !Pci1203SlaveInputsLive(0x02) && !Pci1203SlaveInputsLive(0x01) &&
          !Pci1203SlaveInputsLive(0x03) && !Pci1203SlaveInputsLive(0x0F) && !Pci1203SlaveInputsLive(0x00),
          "inputs update in SAFEOP and OP only (low nibble; the 0x10 error bit does not change it)");
    check(Pci1203SlaveMailboxOk(0x02) && Pci1203SlaveMailboxOk(0x04) && Pci1203SlaveMailboxOk(0x08) &&
          !Pci1203SlaveMailboxOk(0x01) && !Pci1203SlaveMailboxOk(0x03) &&
          !Pci1203SlaveMailboxOk(0x0F) && !Pci1203SlaveMailboxOk(0x00),
          "the CoE mailbox is there in PREOP, SAFEOP and OP -- not in INIT / BOOT / OFFLINE / UNKNOWN");

    // --- 9. drive error vs the monitor's own probe (finding 3) ------------------
    std::printf("-- 9. Pci1203DriveErrAccept\n");
    {
        unsigned long probe = 0;
        unsigned long shown = 0x80005111ul;                    // a real refused move, on screen
        const unsigned long kPdo = 0x8000009Ful;
        probe = kPdo;                                          // PollTorque_: Acm_AxGetActTorque failed
        bool a1 = Pci1203DriveErrAccept(kPdo, probe);          // next Poll: Acm_GetLastError = that code
        if (a1) shown = kPdo;
        bool a2 = Pci1203DriveErrAccept(kPdo, probe);
        if (a2) shown = kPdo;
        check(!a1 && !a2 && shown == 0x80005111ul && probe == kPdo,
              "the monitor's own failing torque read is WITHHELD from driveErr: the page keeps the "
              "drive's real reason (0x80005111), Poll after Poll");
        const bool a3 = Pci1203DriveErrAccept(0x80005111ul, probe);   // a command refused again
        if (a3) shown = 0x80005111ul;
        check(a3 && probe == 0, "any other code is published and clears the probe");
        const bool a4 = Pci1203DriveErrAccept(kPdo, probe);
        check(a4, "with the probe cleared, even the same code is published (someone else wrote it)");
        probe = kPdo;
        check(Pci1203DriveErrAccept(0, probe) && probe == 0,
              "a last error of 0 is published too, and clears the probe");
    }

    // --- 10. focus SDO backoff (finding 4) ---------------------------------------
    std::printf("-- 10. Pci1203SdoBackoff\n");
    {
        Pci1203SdoBackoff b;
        check(b.Due(false) && !b.suspended && b.failRun == 0, "fresh: asked every Poll");
        b.Note(false); b.Note(false);
        check(b.Due(false) && !b.suspended && b.failRun == 2, "2 failures: still asked every Poll");
        b.Note(false);
        check(!b.Due(false) && b.Due(true) && b.suspended && b.failRun == 3,
              "the 3rd failure in a row SUSPENDS it: asked only when the 30 s window is open");
        b.Note(false);
        check(b.suspended && !b.Due(false), "a failed retry at the window keeps it suspended");
        b.Note(true);
        check(!b.suspended && b.failRun == 0 && b.Due(false), "one success resumes it at once");
        b.Note(false); b.Reset();
        check(!b.suspended && b.failRun == 0, "Reset() starts a fresh count (a focus on another drive)");
    }
    {
        //  A dead station for one minute of 200 ms Polls, the window every 150.
        Pci1203SdoBackoff b;
        int reads = 0;
        for (int poll = 0; poll < 300; ++poll) {
            const bool window = (poll % 150) == 0;
            if (!b.Due(window)) continue;
            ++reads;
            b.Note(false);
        }
        check(reads <= (int)kPci1203TorqueSdoFailSuspend + 2,
              "a dead station for 300 Polls costs 3 reads + one per 30 s window, not 300 mailbox timeouts");
    }
    check(kPci1203TorqueSdoFailSuspend == 3, "the focus read suspends after 3 failures in a row");

    // --- 11. torque cleared when not read (finding 5) ----------------------------
    std::printf("-- 11. Pci1203TorqueClearPoll\n");
    {
        Pci1203AxisSample s;
        s.torqueValid = true; s.torqueSrc = 2; s.torqueUnitVerified = true; s.torquePctValid = true;
        s.torqueNmValid = true; s.torquePdoValid = true; s.torqueSdoValid = true;
        s.torquePdoOff = true; s.torquePdoOffPerm = true; s.torquePdoUnitOk = true; s.torqueAgree = 3;
        Pci1203TorqueClearPoll(s);
        check(!s.torqueValid && s.torqueSrc == 0 && !s.torqueUnitVerified && !s.torquePctValid &&
              !s.torqueNmValid && !s.torquePdoValid && !s.torqueSdoValid,
              "not read this Poll (closed / detached / no handle / disabled): every torque value is null");
        check(s.torquePdoOff && s.torquePdoOffPerm && s.torquePdoUnitOk && s.torqueAgree == 3,
              "the latches (PDO off, PDO unit verified) are facts of the open handle and stay");
        check(Pci1203AxisSample().driveErrProbe == 0, "a fresh axis sample carries no torque probe code");
    }

    // --- 12. kCmdAxTorqueLimitSet (fake SDO seam) ----------------------------------
    std::printf("-- 12. kCmdAxTorqueLimitSet / Pci1203TorqueLimitRun\n");
    check((int)kCmdAxTorqueLimitSet == (int)kCmdAxSetInitCfg + 1,
          "kCmdAxTorqueLimitSet is appended LAST: no existing kind changed its value");
    check(std::strstr(Pci1203CmdName(kCmdAxTorqueLimitSet), "(internal)") != 0,
          "its audit name says (internal)");
    {
        Pci1203WireCmd w;
        w.name = "pci1203.ax.torqueLimitSet"; w.target = "pci1203.ax0"; w.hasNum = true; w.num = 1000.0;
        Pci1203Cmd c; std::string why;
        check(!Pci1203ParseWire(w, c, why) && Pci1203CmdFromName("pci1203.ax.torqueLimitSet") == kCmdNone &&
              Pci1203CmdFromName("pci1203.ax.torqueLimitSet (internal)") == kCmdNone,
              "NO wire name: neither the pci1203 page nor a browser can send it");
    }
    check(Pci1203TorqueLimitIndex(kTorqueLimitPos, 0) == 0x60E0 && Pci1203TorqueLimitIndex(kTorqueLimitNeg, 0) == 0x60E1 &&
          Pci1203TorqueLimitIndex(kTorqueLimitPos, 1) == 0x68E0 && Pci1203TorqueLimitIndex(kTorqueLimitNeg, 1) == 0x68E1,
          "60E0h / 60E1h for axis A, 68E0h / 68E1h for a two-axis unit's B half (Pci1203GearAxisBase)");
    check(Pci1203TorqueLimitValueOk(0) && Pci1203TorqueLimitValueOk(1) && Pci1203TorqueLimitValueOk(3000) &&
          Pci1203TorqueLimitValueOk(65535) && !Pci1203TorqueLimitValueOk(-1) &&
          !Pci1203TorqueLimitValueOk(65536) && !Pci1203TorqueLimitValueOk(12.5) &&
          !Pci1203TorqueLimitValueOk(std::sqrt(-1.0)),
          "the value is a UINT16 whole number 0..65535 -- a fraction, a negative, 65536 or NaN is refused (never clamped)");
    {
        FakeTorqueLimitSdo d;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(o.ok && o.failStep == 0 && o.ret == 0 && o.posValid && o.pos == 1500 && o.negValid &&
              o.neg == 1500 && o.why.empty(),
              "happy path: ok, both read-backs == 1500, nothing to explain");
        check(d.order == "W+W-R+R-" && d.calls == 4 && d.obj[0] == 1500 && d.obj[1] == 1500,
              "ONE Execute = write 60E0h, write 60E1h (the same value), read 60E0h, read 60E1h -- in that order, 4 calls");
    }
    {
        FakeTorqueLimitSdo d; d.failCall = 1; d.failRet = 0x80000011ul;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(!o.ok && o.failStep == 1 && o.ret == 0x80000011ul && d.calls == 1 && !o.posValid &&
              o.why.find("60E0h") != std::string::npos,
              "write 60E0h fails: step 1, its code, why names 60E0h, and it STOPS (1 call, no retry)");
    }
    {
        FakeTorqueLimitSdo d; d.failCall = 2; d.failRet = 0x80000022ul;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(!o.ok && o.failStep == 2 && o.ret == 0x80000022ul && d.calls == 2 &&
              o.why.find("60E1h") != std::string::npos && o.why.find("60E0h") != std::string::npos,
              "write 60E1h fails: step 2, 2 calls, and why says 60E0h was already written (the two may differ)");
    }
    {
        FakeTorqueLimitSdo d; d.failCall = 3; d.failRet = 0x80000033ul;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(!o.ok && o.failStep == 3 && o.ret == 0x80000033ul && d.calls == 3 && !o.posValid,
              "read 60E0h fails: step 3, no read-back value");
    }
    {
        FakeTorqueLimitSdo d; d.failCall = 4; d.failRet = 0x80000044ul;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(!o.ok && o.failStep == 4 && o.ret == 0x80000044ul && d.calls == 4 && o.posValid &&
              o.pos == 1500 && !o.negValid,
              "read 60E1h fails: step 4, and the 60E0h read-back is still reported");
    }
    {
        FakeTorqueLimitSdo d; d.clamp = 1000;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 1500, 0);
        check(!o.ok && o.failStep == 5 && o.ret == 0 && o.posValid && o.pos == 1000 && o.negValid &&
              o.neg == 1000 && o.why.find("1000") != std::string::npos,
              "every call SUCCESS but the drive holds 1000: NOT ok (step 5, ret SUCCESS), and why gives both numbers");
    }
    {
        FakeTorqueLimitSdo d; d.failCall = 2; d.failRet = 0x80000022ul;
        const Pci1203TorqueLimitOutcome o = Pci1203TorqueLimitRun(d, 800, 1);
        check(o.why.find("68E1h") != std::string::npos && o.why.find("68E0h") != std::string::npos,
              "axis B: the failure text names 68E0h / 68E1h, not the axis-A objects");
    }
    {
        const Pci1203CmdResult r0 = Pci1203CmdResult();
        check(!r0.ok && r0.value == 0.0 && !r0.valueValid && r0.failStep == 0,
              "a default result carries no verdict (ok false, no value, no step)");
        TPci1203Control ctl;
        Pci1203Cmd c;
        c.kind = kCmdAxTorqueLimitSet; c.axis = 0; c.value = 1000.0;
        const Pci1203CmdResult r = ctl.Execute(c);
        check(!r.accepted && !r.issued && !r.ok, "kCmdAxTorqueLimitSet on a closed control is refused, nothing issued, not ok");
    }   { check(Pci1203CmdFromName("pci1203.ring.reset") == kCmdNone && std::strstr(Pci1203CmdName(kCmdRingReset), "(internal)") != 0, "AI(W906-1203REOPEN-2) 20261007: kCmdRingReset is INTERNAL -- no wire name (review C3), audit name says (internal)"); Pci1203WireCmd w; w.name = "pci1203.ring.reset"; w.hasNum = true; w.num = 1.0; Pci1203Cmd c; std::string why; check(!Pci1203ParseWire(w, c, why), "a browser frame pci1203.ring.reset num 1 (reset ring 0) is refused at parse"); TPci1203Control ctl; Pci1203Cmd k; k.kind = kCmdRingReset; k.value = 2.0; const Pci1203CmdResult r = ctl.Execute(k); check(!r.accepted && !r.issued, "kCmdRingReset on a closed control is refused, nothing issued"); }   //AI(W906-1203REOPEN) 20261007: same line
    { Pci1203WireCmd w; w.name = "pci1203.carddo.setBit"; w.target = "pci1203.carddo.bit1"; w.hasNum = true; w.num = 1.0; Pci1203Cmd c; std::string why; check(Pci1203ParseWire(w, c, why) && c.kind == kCmdDevDoSetBit && c.bit == 1 && c.value == 1.0, "AI(W906-CARDDO) 20261008: pci1203.carddo.setBit pci1203.carddo.bit1 = 1 parses to kCmdDevDoSetBit, bit 1, value 1"); check(std::string(Pci1203CmdName(kCmdDevDoSetBit)) == "pci1203.carddo.setBit" && Pci1203CmdFromName("pci1203.carddo.setBit") == kCmdDevDoSetBit, "carddo: wire name and audit name agree"); Pci1203WireCmd w2 = w; w2.target = "pci1203.carddo"; check(!Pci1203ParseWire(w2, c, why), "carddo.setBit without a bit number is refused at parse"); TPci1203Control ctl; Pci1203Cmd k; k.kind = kCmdDevDoSetBit; k.bit = 1; k.value = 1.0; const Pci1203CmdResult r = ctl.Execute(k); check(!r.accepted && !r.issued, "kCmdDevDoSetBit on a closed control is refused, nothing issued"); }   /*AI(W906-CARDDO) 20261008: replaces a blank line*/
    std::printf("\ntest_pci1203_pure: %d checks, %d failure(s)\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}
