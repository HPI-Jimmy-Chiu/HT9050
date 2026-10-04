// =============================================================================
//  test_e044_torque_wait.cpp -- AI(W906-E044) 20261004 (St01 / ST01-E), ctest E044TorqueWait
//
//  E-044 (Steven 1004 08:4x, SAFETY, NOT GOLDEN): on HT9050 (MOT[MTestZ1].CardType=="PCI1203")
//  the production Index torque wait DoTestHeadMotor 12110 (and its arm-2 twin 14110) ends after
//  5 s without a value: reader disarmed, golden alarm-stop (ShowErrorMessage), golden 12110 exit
//  (fAllMotorHome=false; Task=1). Every other machine: golden, unchanged.
//  Plan: D:\AI_TempFile\st01e-e044-plan-20261004.md section 5. Each section goes RED when the rule
//  it pins is broken:
//    [P]   pure window (Ht9050TorqueWait.h)            [T1]  fires on HT9050 after 5 s (12110)
//    [T2]  Panasonic / SMC rows: no timer at all       [T3]  a value resets the timer
//    [T4]  a new wait resets after a non-value exit    [T5]  stop is never blocked
//    [T6]  golden 999 exit unchanged                   [T7]  SIM config inert
//    [T8]  32-bit tick wrap                            [T9]  fires once, then re-arms
//    [T10] 14110 twin                                  [T11] source pins (same-line, order, archive)
//    ST01-M change request 1004 15:4x (ST01-E2 R4 plan s3):
//    [T12] (1) alarm only while auto-run drives the wait: not when stopped, not in HOME (iHome==1), not SoftStop
//    [T13] (2) re-armed on every entry: nothing from an earlier pass carries over (also when golden's
//              arming tick is skipped), and a resumed wait gets a fresh 5 s
//    [T14] (3) after an R4 drive-alarm raise (golden ShowMotorErrorMessage, keyed state fAllMotorHome==false,
//              golden note.cpp:1059) the 12110 timeout stays silent: one alarm, not two
//  The real DoTestHeadMotor runs (entered at 12102 / 14102, golden :6431 / :7061); the window's clock
//  and the MainProc pass counter are faked; each call is one 500 ms MainProc pass (port kServeTickMs,
//  tools/wb_serve.cpp:2931).
//  No machine file is read or written; Comm1 runs in SIM mode for [T6].
// =============================================================================
#include "atester.h"                // DoTestHeadMotor / iTestHeadMotorTask / DoTestHeadMotorDelay
#include "atester_shims.h"          // COM2
#include "cmydef.h"
#include "cprod.h"                  // CosFunction / IniConfig
#include "FormsFacade.h"            // fMain
#include "MachineType.h"            // SOFT_SIMULTE
#include "Motor/mymotor.h"          // MOT[]
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage capture seams
#include "myTimer.h"
#include "Ht9050TorqueWait.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; std::printf("  PASS %s\n", msg); } \
                           else   { g_fail++; std::printf("  FAIL %s\n", msg); } } while (0)

// ---- fake clock + alarm hook ------------------------------------------------------------------
static unsigned long g_now = 100000ul;
static unsigned long FakeClock() { return g_now; }
static unsigned long g_passNo = 7ul;                  // MainProc passes (golden guMainProcCallCount)
static unsigned long FakePass() { return g_passNo; }
static int g_hookCalls = 0, g_hookPos = -1, g_hookK = 0;
static std::string g_hookCode;
static int CaptureHook(const char* c, int k, int pos) { ++g_hookCalls; g_hookCode = c ? c : ""; g_hookK = k; g_hookPos = pos; return 0; }

static void Prep(const char* card)
{
    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();
    g_hookCalls = 0; g_hookPos = -1; g_hookK = 0; g_hookCode.clear();
    W906_Ht9050TorqueWaitReset();
    W906_Ht9050TorqueWaitCalls = 0;
    MOT[MTestZ1].CardType = card;
    fAllMotorHome = true;                             // auto-run, as the ladder requires (csystem.cpp:1934)
    SystemStart = true;  SoftStop = false;  iHome = 0;
    g_passNo += 10;                                   // a fresh scenario starts on a later pass
    fMain->edTorue0->Text = "";
    fMain->edTorue1->Text = "";
    fMain->chkReadTorque1->Checked = false;
    fMain->chkReadTorque2->Checked = false;
}
//  12102 -> 12110 (golden :6431-6434) or 14102 -> 14110 (golden :7061-7064), then golden's arming tick.
//  stall = golden's arming tick is skipped because DoTestHeadMotorDelay already ran out (a late first tick):
//  bFirstTime stays true, the reader is not armed (golden :6910-6921) -- the entry must still re-arm E-044.
static bool Enter(int arm, bool stall = false)
{
    DoTestHeadMotorDelay.SetMSAndOn(0);               // already elapsed: case 12102 / 14102 advances at once
    iTestHeadMotorTask = arm ? 14102 : 12102;
    g_passNo++;  DoTestHeadMotor();                   // -> 12110 / 14110, DoTestHeadMotorDelay 1 s, bFirstTime=true
    if (iTestHeadMotorTask != (arm ? 14110 : 12110)) return false;
    if (stall) DoTestHeadMotorDelay.SetMSAndOn(0);
    g_passNo++;  DoTestHeadMotor();                   // arming tick: chkReadTorqueN=true, edTorueN="", InitReadTorueTask
    return iTestHeadMotorTask == (arm ? 14110 : 12110);
}
static void Tick(unsigned long ms = 500ul) { g_now += ms; g_passNo++; DoTestHeadMotor(); }
static inline void Passes(unsigned long n, unsigned long ms) { g_passNo += n; g_now += ms; }   // MainProc runs, DoTestHeadMotor not called
static bool Has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

// ---- source pins ------------------------------------------------------------------------------
static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string l;
    while (std::getline(f, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); out.push_back(l); }
    return true;
}
static std::string CodeOf(const std::string& l, const char* lineComment = "//")   // text before a line comment, /* */ removed
{
    std::string s = l;
    for (;;) {
        const std::size_t a = s.find("/*");
        if (a == std::string::npos) break;
        const std::size_t b = s.find("*/", a + 2);
        s.erase(a, b == std::string::npos ? std::string::npos : b + 2 - a);
    }
    const std::size_t c = s.find(lineComment);
    return c == std::string::npos ? s : s.substr(0, c);
}
static std::string NoStrings(const std::string& s)   // drop the contents of "..." literals (printf text may name flags it never writes)
{
    std::string o;
    bool in = false;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char ch = s[i];
        if (in) {
            if (ch == '\\' && i + 1 < s.size()) { ++i; continue; }
            if (ch == '"') { in = false; o += ch; }
            continue;
        }
        if (ch == '"') in = true;
        o += ch;
    }
    return o;
}
static unsigned long long Fnv(const std::vector<std::string>& v, int from1, int to1)   // lines [from1, to1], 1-based, joined by '\n'
{
    unsigned long long h = 0xcbf29ce484222325ull;
    for (int i = from1; i <= to1 && i <= (int)v.size(); ++i) {
        std::string b = v[(std::size_t)i - 1];
        if (i < to1) b += '\n';
        for (std::size_t k = 0; k < b.size(); ++k) { h ^= (unsigned char)b[k]; h *= 0x100000001b3ull; }
    }
    return h;
}

#ifndef SOFT_SIMULTE
// ---- [T6] a fake Panasonic A5 for golden's torque reader (shape of tests/test_rs232_torque.cpp) -
struct FakeServo {
    int pending = 0;                                  // 1 = torque request seen
    std::vector<unsigned char> OnTx(const std::vector<unsigned char>& tx)
    {
        if (tx.size() == 1 && tx[0] == 0x05) return { 0x04 };                               // ENQ -> EOT
        if (tx.size() == 4 && tx[0] == 0x00 && tx[2] == 0x52) { pending = 1; return { 0x06, 0x05 }; }   // golden rs232.cpp:909-911
        if (tx.size() == 1 && tx[0] == 0x04 && pending) { pending = 0; return { 0x03, 0x00, 0xD2, 0x2C, 0x01, 0x00, 0x00 }; }  // k=300 -> "15.00"
        return {};
    }
};
static bool ReaderTo999()
{
    FakeServo s;
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < 8000) {
        if (COM2->GetReadTorueTask() == 999) return true;
        COM2->ReadTorque();
        const std::vector<char>& v = COM2->Comm1->SimTxBuffer();
        std::vector<unsigned char> tx(v.begin(), v.end());
        COM2->Comm1->SimClearTx();
        if (!tx.empty()) {
            std::vector<unsigned char> r = s.OnTx(tx);
            if (!r.empty()) { COM2->Comm1->SimInjectReceive(&r[0], (Spcomm::Word)r.size()); ::Sleep(130); }
        } else ::Sleep(5);
    }
    return COM2->GetReadTorueTask() == 999;
}
#endif

int main()
{
    std::printf("[e044] %s configuration\n",
#ifdef SOFT_SIMULTE
        "SIM (SOFT_SIMULTE)"
#else
        "SHIP (no SOFT_SIMULTE)"
#endif
    );
    InitialOK = true;
    INDEX_DRIVER_TYPE = Panasonic_DRIVER;             // InitReadTorueTask -> autoTask=1 (rs232.cpp:352-353), never 999 by itself
    iPanasonicDriverType = Panasonic_DRIVER_A5;
    TorqueUseHPComCard = false;
    COM2->fPanasonicParameterRW = false;
    iPauseBackUp = -1;  bIndexCheckState = false;  bEject = false;  bSht1LoseICErr = bSht2LoseICErr = false;
    MACHINE_HAS_AUTO_ALIGNMENT_CCD = false;  IniConfig.bEnableCCDUSETCPIP = false;
    const int savHeater = USE_16_HEATER;  USE_16_HEATER = -1;   // no CheckIndexConnect alarm in the preamble (atester.cpp:6109-6113)
    CosFunction.bIndexAreaOnlyCanUseSkip = false;
    W906_Ht9050TorqueWaitClock = &FakeClock;
    W906_Ht9050TorqueWaitPass  = &FakePass;
    W906_ShowErrorMessage_Hook = &CaptureHook;

    // ---------------- [P] / [T8] / [T9] pure window (both configs) ----------------
    {
        using ht9045::e044::Wait;
        CHECK(ht9045::e044::kWaitMs == 5000u, "[P] the limit is 5000 ms (Steven 1004 14:1x; = E-038/E-042 P5 kTorqueWaitMs)");
        //  S(): one tick on consecutive passes (pass = previous + 1) unless told otherwise
        Wait w;  std::uint32_t pn = 100;
        auto S = [&](std::uint32_t t, bool waiting = true, bool eligible = true, bool newWait = false, std::uint32_t passStep = 1) {
            pn += passStep;
            return ht9045::e044::Step(w, newWait, pn, waiting, eligible, t);
        };
        bool f0 = S(1000, true, true, true), f1 = S(5999), f2 = S(6000);
        CHECK(!f0 && !f1 && f2, "[P] new wait at 1000: not at 5999 (4999 ms), fires at 6000 (5000 ms)");
        w = Wait();
        S(0); S(4000);
        CHECK(!S(4000, false), "[T3] a tick with a value (waiting=false) never fires");
        CHECK(!S(4500) && !S(9000) && S(9500), "[T3] after a value the window restarts: 4500 ms after the restart no, 5000 ms yes");
        w = Wait();
        S(0); S(4000);
        CHECK(!S(4000, true, true, true) && !S(8500) && S(9000), "[T4] newWait restarts a 4 s old window: fires 5 s after the new wait, not 1 s");
        w = Wait();
        S(0); S(4000);
        CHECK(!S(4500, true, true, false, 2) && !S(9000) && S(9500),
              "[T13] (2) a gap of 2+ passes = a new entry: a 4 s old window does not carry over (no newWait needed)");
        w = Wait();
        bool anyIneligible = false;
        for (std::uint32_t t = 0; t <= 60000; t += 500) anyIneligible = S(t, true, false) || anyIneligible;
        CHECK(!anyIneligible, "[T12] (1) 60 s of ineligible ticks (not running / HOME / drive alarm): never fires");
        CHECK(!S(60500) && !S(65000) && S(65500), "[T12] (1) eligible again: a fresh window, fires 5000 ms later (no time carried over)");
        w = Wait();
        const std::uint32_t s0 = 0xFFFFF000u;
        S(s0);
        CHECK(!S(s0 + 4999u) && S(s0 + 5000u), "[T8] 32-bit tick wrap: 4999 no, 5000 yes");
        w = Wait();  pn = 0xFFFFFFFEu;
        S(0); S(4000);                                // the pass counter wraps between these ticks
        CHECK(!S(4500) && S(5000), "[T8] 32-bit pass-counter wrap: consecutive passes stay one entry");
        w = Wait();
        S(0);
        const bool first = S(5000);
        const bool again = S(5500) || S(10499);
        CHECK(first && !again && S(10500), "[T9] fires once, then re-arms: next alarm only another 5000 ms later");
        CHECK(ht9045::e044::Eligible(true, false, true, 0) && !ht9045::e044::Eligible(false, false, true, 0) &&
              !ht9045::e044::Eligible(true, true, true, 0) && !ht9045::e044::Eligible(true, false, false, 0) &&
              !ht9045::e044::Eligible(true, false, true, 1), "[T12] Eligible = SystemStart && !SoftStop && fAllMotorHome && iHome!=1 (ladder guard csystem.cpp:1934 + not HOME)");
    }

#ifdef SOFT_SIMULTE
    // ---------------- [T7] SIM: inert ----------------
    {
        Prep("PCI1203");
        bool any = false;
        for (int i = 0; i < 120; ++i) { g_now += 500; any = any || W906_Ht9050TorqueWaitTimedOut(0, i == 0, true); }
        CHECK(!any && W906_Ht9050TorqueWaitCalls == 120, "[T7] SOFT_SIMULTE: W906_Ht9050TorqueWaitTimedOut is false for 60 s even on a PCI1203 row");
        CHECK(Enter(0), "[T7] SIM: 12102 -> 12110 + golden arming tick");
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110, "[T7] SIM: 60 s at 12110 -> golden wait, no E-044 alarm");
    }
#else
    // ---------------- [T1] HT9050: alarm after 5 s at 12110 ----------------
    {
        Prep("PCI1203");
        CHECK(Enter(0), "[T1] 12102 -> 12110 + golden arming tick");
        CHECK(fMain->chkReadTorque1->Checked && !fMain->chkReadTorque2->Checked, "[T1] golden armed the Z1 reader (golden :6444-6445)");
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110 && fMain->chkReadTorque1->Checked,
              "[T1] 4.5 s: no alarm, still waiting at 12110, reader still armed");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1, "[T1] 5.0 s: exactly one ShowErrorMessage");
        CHECK(W906_ShowErrorMessage_LastCode == AnsiString(W906_Ht9050TorqueWaitAlarmCode(0)) && W906_ShowErrorMessage_LastCode == "WAR0361",
              "[T1] code = WAR0361 (placeholder; TODO Jimmy picks)");
        CHECK(g_hookCalls == 1 && g_hookPos == MTestZ1, "[T1] alarm position = MTestZ1 (the hook wb_serve stops in was reached)");
        CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY, "[T1] KCode K_RETRY when CosFunction.bIndexAreaOnlyCanUseSkip is off (golden 12112 convention)");
        CHECK(Has(W906_ShowErrorMessage_LastErrPart.c_str(), "Z1") && Has(W906_ShowErrorMessage_LastErrPart.c_str(), "12110"), "[T1] errPart names Z1 and 12110");
        CHECK(!fMain->chkReadTorque1->Checked && !fMain->chkReadTorque2->Checked, "[T1] reader disarmed (golden case 40 shape)");
        CHECK(fAllMotorHome == false && iTestHeadMotorTask == 1, "[T1] golden 12110 exit: fAllMotorHome=false, Task=1 (golden :6459-6460)");
        CHECK(W906_ShowMyMessage_Count == 0, "[T1] golden's own 999 message not raised");
        Prep("PCI1203");
        CosFunction.bIndexAreaOnlyCanUseSkip = true;
        Enter(0);
        for (int i = 0; i < 10; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastKCode == K_SKIP, "[T1] KCode K_SKIP when bIndexAreaOnlyCanUseSkip (Steven 20141105)");
        CosFunction.bIndexAreaOnlyCanUseSkip = false;
    }
    // ---------------- [T2] Panasonic / SMC rows: golden, no timer ----------------
    {
        const char* cards[2] = { "", "SMC" };
        for (int c = 0; c < 2; ++c) {
            Prep(cards[c]);
            const bool in = Enter(0);
            for (int i = 0; i < 120; ++i) Tick();
            CHECK(in && W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110 && fMain->chkReadTorque1->Checked && fAllMotorHome,
                  c ? "[T2] SMC row: 60 s at 12110 -> golden wait, reader armed, no alarm" : "[T2] Panasonic row: 60 s at 12110 -> golden wait, reader armed, no alarm");
            CHECK(W906_Ht9050TorqueWaitCalls == 0, c ? "[T2] SMC row: the helper is never entered (atester's && stops at CardType)"
                                                     : "[T2] Panasonic row: the helper is never entered (atester's && stops at CardType)");
        }
        Prep("");
        bool any = false;
        for (int i = 0; i < 40; ++i) { g_now += 500; any = any || W906_Ht9050TorqueWaitTimedOut(0, i == 0, true); }
        CHECK(!any, "[T2] the helper itself is false on a non-PCI1203 row (defence in depth)");
    }
    // ---------------- [T3] a value resets the timer (through DoTestHeadMotor) ----------------
    {
        Prep("PCI1203");
        Enter(0);
        for (int i = 0; i < 8; ++i) Tick();                            // 4 s
        fMain->edTorue0->Text = "12.00";                              // golden reader's value arrives
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask != 12110, "[T3] value at 4 s: leaves 12110, no alarm");
        fMain->edTorue0->Text = "";
        fMain->lbArm0Torque->Caption = "---";
        const bool in = Enter(0);
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(in && W906_ShowErrorMessage_Count == 0, "[T3] next wait: no alarm at 4.5 s");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1, "[T3] next wait: alarm at 5.0 s");
        //  wrapper level: reset by a value alone (no new wait)
        Prep("PCI1203");
        W906_Ht9050TorqueWaitTimedOut(0, false, true);  g_now += 4000;
        W906_Ht9050TorqueWaitTimedOut(0, false, true);
        W906_Ht9050TorqueWaitTimedOut(0, false, false);                // value
        g_now += 500;  bool a = W906_Ht9050TorqueWaitTimedOut(0, false, true);
        g_now += 4500; bool b = W906_Ht9050TorqueWaitTimedOut(0, false, true);
        g_now += 500;  bool c = W906_Ht9050TorqueWaitTimedOut(0, false, true);
        CHECK(!a && !b && c, "[T3] wrapper: a value tick restarts the window (4500 ms no, 5000 ms yes)");
    }
    // ---------------- [T4] a new wait after a non-value exit (golden 999 exit / HOME) ----------------
    {
        Prep("PCI1203");
        Enter(0);
        for (int i = 0; i < 8; ++i) Tick();                            // 4 s, no value
        iTestHeadMotorTask = 1;                                       // left 12110 without a value
        const bool in = Enter(0);
        Tick(); Tick();
        CHECK(in && W906_ShowErrorMessage_Count == 0, "[T4] new wait after a 4 s abandoned one: no alarm 1 s in");
        for (int i = 0; i < 7; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T4] no alarm 4.5 s into the new wait");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1, "[T4] alarm 5.0 s into the new wait");
    }
    // ---------------- [T5] stop is never blocked ----------------
    {
        Prep("PCI1203");
        Enter(0);
        for (int i = 0; i < 6; ++i) Tick();                            // 3 s, then STOP: MainProc runs on, DoTestHeadMotor is not called
        SystemStart = false;
        Passes(120, 60000);
        CHECK(W906_ShowErrorMessage_Count == 0, "[T5] nothing fires on its own while stopped (no thread / timer callback)");
        SystemStart = true;                                            // START again, still no value
        Tick();                                                        // first tick after the resume = the window's start
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110, "[T13] (2) resumed after 60 s: a fresh window, no alarm at the first tick");
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T13] (2) resumed: no alarm 4.5 s after the resume");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1 && iTestHeadMotorTask == 1, "[T13] (2) resumed: alarm 5.0 s after the resume");
        //  the alarm reaches the ShowErrorMessage hook with nothing gating it: no INI, no E-038 flag, no other hook
        CHECK(g_hookCalls == 1, "[T5] the stop path (ShowErrorMessage hook) is reached unconditionally");
    }
    // ---------------- [T12] (1) only while auto-run drives the wait ----------------
    {
        //  not running: the stopped branch calls DoTestHeadMotor for [I01] TesterFinishThenHome (csystem.cpp:32632-32648)
        Prep("PCI1203");
        Enter(0);
        SystemStart = false;
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110, "[T12] (1) SystemStart==false: 60 s at 12110, no E-044 alarm");
        //  HOME: SystemStart && iHome==1 -- golden's HOME path calls DoTestHeadMotor (csystem.cpp:31220-31231)
        SystemStart = true;  iHome = 1;  fAllMotorHome = false;
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 12110, "[T12] (1) HOME (iHome==1, fAllMotorHome==false): 60 s at 12110, no E-044 alarm");
        iHome = 0;  fAllMotorHome = true;  SoftStop = true;
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T12] (1) SoftStop: 60 s, no E-044 alarm");
        SoftStop = false;
        iHome = 1;                                                     // HOME flag alone (fAllMotorHome still true)
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T12] (1) iHome==1 alone: 60 s, no E-044 alarm");
        iHome = 0;                                                     // auto-run drives the wait again
        Tick();                                                        // first eligible tick = the window's start
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T12] (1) auto-run again: no alarm 4.5 s later (no time carried over from the 4 min above)");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1 && iTestHeadMotorTask == 1, "[T12] (1) auto-run again: alarm 5.0 s later");
    }
    // ---------------- [T13] (2) re-armed on every entry, even when golden's arming tick is skipped ----------------
    {
        Prep("PCI1203");
        Enter(0);
        for (int i = 0; i < 8; ++i) Tick();                            // 4 s, no value
        iTestHeadMotorTask = 1;                                       // left without a value (golden 999 exit / HOME abort)
        const bool in = Enter(0, true);                               // late first tick: golden skips arming, bFirstTime stays true
        CHECK(in && W906_ShowErrorMessage_Count == 0, "[T13] (2) stalled re-entry after a 4 s abandoned pass: no alarm at entry");
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0, "[T13] (2) stalled re-entry: no alarm 4.5 s in (nothing carried over)");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1, "[T13] (2) stalled re-entry: alarm 5.0 s in");
    }
    // ---------------- [T14] (3) after an R4 drive-alarm raise the 12110 timeout stays silent ----------------
    {
        //  R4 / E-045 is not implemented yet. Its raise is golden ShowMotorErrorMessage (golden note.cpp:1052-1169;
        //  port forms/fNote_ShowError.cpp:650): SoftStop=SoftStart=false, StopAllMotor, Z ST, IndexMotorBreakerOFF,
        //  fAllMotorHome=false (golden :1054-1059), then the note ("WAR" -> ShowErrorMessage WAR16101, :1067-1069).
        //  The keyed state is fAllMotorHome==false: SystemStart stays true until the operator answers the note's
        //  PAUSE (golden :1110 KeyCode=0, only PAUSE), and no host stop runs here.
        Prep("PCI1203");
        W906_ShowMotorErrorMessage_Reset();
        Enter(0);
        for (int i = 0; i < 6; ++i) Tick();                            // 3 s pressing, then the Z1 drive alarms
        ShowMotorErrorMessage("WAR", 1, "E-044 test: R4 Z1 drive alarm during the press");
        CHECK(W906_ShowMotorErrorMessage_Count == 1 && W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "WAR16101" &&
              fAllMotorHome == false && SystemStart == true, "[T14] (3) R4 raise = golden ShowMotorErrorMessage: WAR16101, fAllMotorHome=false, SystemStart still true");
        for (int i = 0; i < 120; ++i) Tick();                          // 60 s more at 12110 (e.g. [I01] path)
        CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "WAR16101" && iTestHeadMotorTask == 12110,
              "[T14] (3) 60 s after the drive alarm: still ONE alarm (the drive's), no E-044 torque timeout");
        SystemStart = false;                                           // the operator answers PAUSE
        for (int i = 0; i < 20; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 1, "[T14] (3) after PAUSE: still one alarm");
    }
    // ---------------- [T10] 14110 twin ----------------
    {
        Prep("PCI1203");
        CHECK(Enter(1), "[T10] 14102 -> 14110 + golden arming tick");
        CHECK(!fMain->chkReadTorque1->Checked && fMain->chkReadTorque2->Checked, "[T10] golden armed the Z2 reader (golden :7074-7075)");
        for (int i = 0; i < 9; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 14110, "[T10] 4.5 s: no alarm, still at 14110");
        Tick();
        CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "WAR0362" && g_hookPos == MTestZ2,
              "[T10] 5.0 s: WAR0362 (placeholder) at MTestZ2");
        CHECK(Has(W906_ShowErrorMessage_LastErrPart.c_str(), "Z2") && Has(W906_ShowErrorMessage_LastErrPart.c_str(), "14110"), "[T10] errPart names Z2 and 14110");
        CHECK(!fMain->chkReadTorque1->Checked && !fMain->chkReadTorque2->Checked && !fAllMotorHome && iTestHeadMotorTask == 1,
              "[T10] reader disarmed, golden 14110 exit (golden :7090-7091)");
        Prep("");
        Enter(1);
        for (int i = 0; i < 120; ++i) Tick();
        CHECK(W906_ShowErrorMessage_Count == 0 && iTestHeadMotorTask == 14110 && W906_Ht9050TorqueWaitCalls == 0, "[T10] Panasonic row: 14110 golden wait, helper never entered");
    }
    // ---------------- [T6] golden 999 exit unchanged ----------------
    {
        COM2->Comm1->SetSimMode(true);
        COM2->Comm1->StartComm();
        CHECK(COM2->Comm1->IsOpen() && COM2->Comm1->IsSimMode(), "[T6] Comm1 in SIM mode (no COM port touched)");
        const char* cards[2] = { "", "PCI1203" };
        for (int c = 0; c < 2; ++c) {
            Prep("");                                                 // golden reader path (with E-038 merged: -1 on a non-PCI1203 row)
            Enter(0);
            const bool done = ReaderTo999();
            CHECK(done && fMain->edTorue0->Text == "15.00", c ? "[T6] HT9050 case: golden reader reaches 999 with \"15.00\"" : "[T6] Panasonic: golden reader reaches 999 with \"15.00\"");
            MOT[MTestZ1].CardType = cards[c];
            fMain->edTorue0->Text = "";                               // the value is wiped (jou 2011-11-29 case)
            Tick();
            CHECK(W906_ShowMyMessage_Count == 1 && W906_ShowMyMessage_LastS1 == "The test head 1 Motor torque read error" &&
                  !fAllMotorHome && iTestHeadMotorTask == 1,
                  c ? "[T6] HT9050 within 5 s: golden 999 message + exit (golden :6456-6460)" : "[T6] Panasonic: golden 999 message + exit (golden :6456-6460)");
            CHECK(W906_ShowErrorMessage_Count == 0, c ? "[T6] HT9050 within 5 s: E-044 silent" : "[T6] Panasonic: E-044 silent");
        }
        COM2->Comm1->StopComm();
    }
#endif

    // ---------------- [T11] source pins ----------------
    {
        const std::string root = W906_SRC_ROOT;
        std::vector<std::string> at, cm, me, ws;
        CHECK(ReadLines(root + "/atester.cpp", at) && ReadLines(root + "/CMakeLists.txt", cm) &&
              ReadLines(root + "/Ht9050TorqueWait.cpp", me) && ReadLines(root + "/tools/wb_serve.cpp", ws), "[T11] sources readable");
        CHECK(at.size() == 13008u, "[T11] atester.cpp still 13008 lines (same-line inserts only)");
        struct Site { int line; const char* caseLine; const char* golden; const char* call; };
        const Site sites[2] = {
            { 6908, "        case 12110:", "            W7T1_FMAIN_LBARM0TORQUE->Caption=\"1:Reading\";",
              "if(MOT[MTestZ1].CardType==AnsiString(\"PCI1203\") && W906_Ht9050TorqueWaitTimedOut(0, bFirstTime && DoTestHeadMotorDelay.Off()==false, W7T1_FMAIN_EDTORUE0->Text==\"\")) { W906_Ht9050TorqueWaitAlarm(0); fAllMotorHome=false; Task=1; return; }" },
            { 7538, "        case 14110:", "            W7T1_FMAIN_LBARM1TORQUE->Caption=\"2:Reading\";",
              "if(MOT[MTestZ1].CardType==AnsiString(\"PCI1203\") && W906_Ht9050TorqueWaitTimedOut(1, bFirstTime && DoTestHeadMotorDelay.Off()==false, W7T1_FMAIN_EDTORUE1->Text==\"\")) { W906_Ht9050TorqueWaitAlarm(1); fAllMotorHome=false; Task=1; return; }" } };
        for (int k = 0; k < 2 && at.size() >= 13008u; ++k) {
            const std::string& raw = at[(std::size_t)sites[k].line - 1];
            const std::string code = CodeOf(raw);
            CHECK(at[(std::size_t)sites[k].line - 2] == sites[k].caseLine, k ? "[T11] :7537 is still `case 14110:`" : "[T11] :6907 is still `case 12110:`");
            CHECK(raw.compare(0, std::strlen(sites[k].golden), sites[k].golden) == 0, k ? "[T11] :7538 starts with golden's Caption line" : "[T11] :6908 starts with golden's Caption line");
            const std::size_t pc = code.find(sites[k].call);
            CHECK(pc != std::string::npos, k ? "[T11] :7538 carries the E-044 call as live code (before any //)" : "[T11] :6908 carries the E-044 call as live code (before any //)");
            const std::size_t pCard = code.find("MOT[MTestZ1].CardType==AnsiString(\"PCI1203\") &&"), pCall = code.find(k ? "W906_Ht9050TorqueWaitTimedOut(1," : "W906_Ht9050TorqueWaitTimedOut(0,");
            CHECK(pCard != std::string::npos && pCall != std::string::npos && pCard < pCall, k ? "[T11] :7538 CardType is the first && operand" : "[T11] :6908 CardType is the first && operand");
            CHECK(Has(raw, "NOT GOLDEN") && Has(raw, "golden 0618 atester.cpp:"), k ? "[T11] :7538 labelled NOT GOLDEN with golden 0618 lines" : "[T11] :6908 labelled NOT GOLDEN with golden 0618 lines");
        }
        int calls = 0;
        for (std::size_t i = 0; i < at.size(); ++i) { const std::string c = CodeOf(at[i]); if (Has(c, "W906_Ht9050TorqueWaitTimedOut(0,") || Has(c, "W906_Ht9050TorqueWaitTimedOut(1,")) ++calls; }
        CHECK(calls == 2, "[T11] exactly two call sites in atester.cpp");
        //  [T6] golden's 12110 / 14110 bodies below the insert are byte-identical to review6 (= golden 0618 :6438-6468 / :7068-7099)
        CHECK(Fnv(at, 6909, 6939) == 0x1c9f677adda1ce5bull, "[T6] atester.cpp:6909-6939 (golden 12110 wait + 999 exit) unchanged");
        CHECK(Fnv(at, 7539, 7570) == 0xf000a31f47da7e63ull, "[T6] atester.cpp:7539-7570 (golden 14110 wait + 999 exit) unchanged");
        //  SIM unreachability of 12110 (golden :6378-6380 / :6404-6406)
        CHECK(at.size() > 6877u && Has(at[6848], "#ifdef SOFT_SIMULTE") && Has(CodeOf(at[6850]), "Task=12300;") &&
              Has(at[6874], "#ifdef SOFT_SIMULTE") && Has(CodeOf(at[6876]), "Task=12300;"), "[T7] SOFT_SIMULTE arms of 12101 still go to 12300 (12110 unreachable in SIM)");
        //  archive: Ht9050TorqueWait.cpp on the atester.cpp line inside add_library(ht9045_sm
        int lib = -1, hit = -1;
        for (std::size_t i = 0; i < cm.size(); ++i) {
            const std::string c = CodeOf(cm[i], "#");
            if (Has(c, "add_library(")) lib = Has(c, "add_library(ht9045_sm ") ? (int)i : -1;
            if (lib >= 0 && Has(c, "atester.cpp") && Has(c, "Ht9050TorqueWait.cpp")) hit = (int)i;
        }
        CHECK(hit > 0, "[T11] CMakeLists.txt: Ht9050TorqueWait.cpp on the atester.cpp line of add_library(ht9045_sm)");
        //  [T5] the helper writes no stop state and has no wait loop / INI
        std::string body;
        for (std::size_t i = 0; i < me.size(); ++i) body += NoStrings(CodeOf(me[i])) + "\n";
        auto Writes = [&body](const char* name) {                     // `name =` / `name=` but not `name==`
            const std::size_t n = std::strlen(name);
            for (std::size_t p = body.find(name); p != std::string::npos; p = body.find(name, p + n)) {
                std::size_t q = p + n;
                while (q < body.size() && body[q] == ' ') ++q;
                if (q < body.size() && body[q] == '=' && (q + 1 >= body.size() || body[q + 1] != '=')) return true;
            }
            return false;
        };
        CHECK(!Writes("SystemStart") && !Writes("SoftStart") && !Writes("SoftStop") && !Writes("fAllMotorHome") && !Writes("iHome") &&
              !Has(body, "Sleep(") && !Has(body, "while") && !Has(body, "INIFile"), "[T5] Ht9050TorqueWait.cpp: no stop-state write, no loop, no INI");
        CHECK(Has(body, "ht9045::e044::Eligible(SystemStart, SoftStop, fAllMotorHome, iHome)"), "[T12] the wrapper gates on the live SystemStart / SoftStop / fAllMotorHome / iHome");
        CHECK(Has(body, "GetMainProcCallCount()"), "[T13] the wrapper counts real MainProc passes (GetMainProcCallCount) outside tests");
        {
            std::vector<std::string> hd;
            std::string h;
            if (ReadLines(root + "/Ht9050TorqueWait.h", hd)) for (std::size_t i = 0; i < hd.size(); ++i) h += CodeOf(hd[i]) + "\n";
            CHECK(Has(h, "return systemStart && !softStop && allMotorHome && iHome != 1;"), "[T12] Eligible() = the ladder guard + not HOME");
            CHECK(Has(h, "(std::uint32_t)(pass - w.lastPass) > 1u"), "[T13] Step() re-arms on a pass gap");
        }
        CHECK(Has(body, "ShowErrorMessage(code, kcode, a ? MTestZ2 : MTestZ1, false, AnsiString(part));"), "[T5] the alarm is golden's ShowErrorMessage");
        //  [T5] wb_serve stops before any wait / unattended return (ForwardShowErrorMessage, tools/wb_serve.cpp:442)
        int stopLine = -1;
        for (std::size_t i = 0; i < ws.size(); ++i) if (Has(CodeOf(ws[i]), "W906_AlarmStopLikeGolden(code);")) { stopLine = (int)i; break; }
        const std::string sl = stopLine >= 0 ? CodeOf(ws[(std::size_t)stopLine]) : std::string();
        const std::size_t ps = sl.find("W906_AlarmStopLikeGolden(code);"), pr = sl.find("if (!g_modalServer || !g_pumpQueue) return 0;");
        CHECK(stopLine >= 0 && ps != std::string::npos && pr != std::string::npos && ps < pr,
              "[T5] wb_serve ForwardShowErrorMessage: golden stop (W906_AlarmStopLikeGolden) precedes the unattended return");
    }

    USE_16_HEATER = savHeater;
    W906_ShowErrorMessage_Hook = 0;
    W906_Ht9050TorqueWaitClock = 0;
    std::printf("[e044] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
