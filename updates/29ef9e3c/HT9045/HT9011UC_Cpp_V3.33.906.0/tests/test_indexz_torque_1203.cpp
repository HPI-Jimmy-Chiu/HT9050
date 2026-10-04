// =============================================================================
//  test_indexz_torque_1203.cpp -- AI(W906-E038) 20261003 (St01 / ST01-E), ctest IndexZTorque1203
//
//  E-038 (Q87, SAFETY) Phase A: HT9050's Index Z torque from the PCI-1203's 6077h, converted to the
//  number golden's Panasonic reader leaves in fMain->edTorue0 (IndexZTorqueCore.h / IndexZTorque1203.cpp
//  / EtherCAT/Pci1203TorqueRead.cpp). Every section must go RED when the rule it pins is broken:
//    [T1]  2704h conversion            [T2]  sign (wrong direction never triggers; right one at kg)
//    [T3]  gravity, golden default     [T4]  baseline option (off by default)
//    [T5]  golden "%5.2f" / atoi       [T6]  one value per arm, only from a poll after the arm
//    [T7]  validity + read-error alarm [T8]  torque-wait timeout (P5)
//    [T9]  entry refusal (P4)          [T10] SHIP: PCI1203 path vs Panasonic RS-232 path
//    [T11] wiring source pins          [T12] Gerneral.ini switches are READ-ONLY (file SHA-256 unchanged)
//  No machine file is read or written: [T10] / [T12] use a temp-dir Gerneral.ini; Comm1 runs in SIM mode.
// =============================================================================
#include "atester_shims.h"
#include "cmydef.h"
#include "common.h"                       // INIFileGeneral
#include "FormsFacade.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "forms/fContact.h"               // fContactForm->PnlTorue0
#include "canary_support.h"
#include "IndexZTorqueCore.h"
#include "IndexZTorque1203.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace ht9045::idxz;

static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; std::printf("  PASS %s\n", msg); } \
                           else   { g_fail++; std::printf("  FAIL %s\n", msg); } } while (0)

// ---- helpers --------------------------------------------------------------------------------
static Sample Good(unsigned long poll, long raw)
{
    Sample s;
    s.haveMonitor = true; s.poll = poll; s.torqueValid = true; s.src = 2; s.raw = raw; s.pctValid = true;
    s.num = 1; s.den = 10; s.servoOn = true; s.alarm = false; s.focusOnAxis = true; s.slot = 2;
    return s;
}
static BridgeIn Armed(const Sample& s, unsigned long now)
{
    BridgeIn in;
    in.chk1 = true; in.confirmed = true; in.driverPanasonic = true; in.pressSign = PressSign(false);
    in.hookPresent = true; in.sampleRead = true; in.sample = s; in.nowMs = now;
    return in;
}
//  Golden's reader of the value: case 555 TorqueData = atoi(edTorue0) (cContact.cpp:5775), then
//  Panasonic abs (:5786-5787), then TorqueData >= kg (:5789).
static bool GoldenTriggers(const std::string& text, int kg) { return std::abs(std::atoi(text.c_str())) >= kg; }
//  One full arm: first tick records the arm poll, second tick (a newer poll) yields the value.
static BridgeOut OneValue(BridgeState& st, long raw, unsigned long& poll, bool baselineOpt = false)
{
    BridgeIn a = Armed(Good(poll, raw), 1000); a.baselineOpt = baselineOpt;
    BridgeOut o = BridgeStep(st, a);
    if (o.code == kBridgeWrote) return o;
    ++poll;
    BridgeIn b = Armed(Good(poll, raw), 1010); b.baselineOpt = baselineOpt;
    return BridgeStep(st, b);
}
static bool Has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

// ---- SHA-256 (FIPS 180-4), for [T12] ------------------------------------------------------
namespace sha {
static const unsigned int K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };
static unsigned int R(unsigned int x, int n) { return (x >> n) | (x << (32 - n)); }
static std::string Hex(const std::vector<unsigned char>& msg)
{
    unsigned int h[8] = { 0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 };
    std::vector<unsigned char> m(msg);
    const unsigned long long bits = (unsigned long long)msg.size() * 8ull;
    m.push_back(0x80);
    while (m.size() % 64 != 56) m.push_back(0);
    for (int i = 7; i >= 0; --i) m.push_back((unsigned char)(bits >> (i * 8)));
    for (std::size_t off = 0; off < m.size(); off += 64) {
        unsigned int w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = ((unsigned int)m[off + 4*i] << 24) | ((unsigned int)m[off + 4*i + 1] << 16) | ((unsigned int)m[off + 4*i + 2] << 8) | m[off + 4*i + 3];
        for (int i = 16; i < 64; ++i) {
            const unsigned int s0 = R(w[i-15], 7) ^ R(w[i-15], 18) ^ (w[i-15] >> 3);
            const unsigned int s1 = R(w[i-2], 17) ^ R(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        unsigned int a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            const unsigned int S1 = R(e, 6) ^ R(e, 11) ^ R(e, 25), ch = (e & f) ^ (~e & g);
            const unsigned int t1 = hh + S1 + ch + K[i] + w[i];
            const unsigned int S0 = R(a, 2) ^ R(a, 13) ^ R(a, 22), mj = (a & b) ^ (a & c) ^ (b & c);
            const unsigned int t2 = S0 + mj;
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    char out[65];
    for (int i = 0; i < 8; ++i) std::snprintf(out + 8 * i, 9, "%08x", h[i]);
    return std::string(out, 64);
}
}  // namespace sha
static std::string FileSha(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return "missing";
    std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return sha::Hex(b);
}
static void WriteBytes(const std::string& path, const std::string& bytes)
{
    std::ofstream f(path.c_str(), std::ios::binary | std::ios::trunc);
    f.write(bytes.data(), (std::streamsize)bytes.size());
}

// ---- source pins ----------------------------------------------------------------------------
static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string l;
    while (std::getline(f, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); out.push_back(l); }
    return true;
}
static std::string CodeOf(const std::string& l, const char* lineComment = "//")   // the text before a line comment, /* */ removed
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
static int FindCode(const std::vector<std::string>& v, const char* needle, const char* lc = "//", int from = 0)
{
    for (std::size_t i = (std::size_t)from; i < v.size(); ++i) if (CodeOf(v[i], lc).find(needle) != std::string::npos) return (int)i;
    return -1;
}
static int CountCode(const std::vector<std::string>& v, const char* needle, const char* lc = "//")
{
    int n = 0;
    for (std::size_t i = 0; i < v.size(); ++i) if (CodeOf(v[i], lc).find(needle) != std::string::npos) ++n;
    return n;
}

// ---- [T10] harness: the fake hook + the RS-232 SIM port (as tests/test_rs232_torque.cpp:33-38) ----
static int    g_hookCalls = 0;
static Sample g_hookSample;
static void FakeReadHook(int mi, Sample* out) { ++g_hookCalls; if (out) *out = g_hookSample; (void)mi; }
#ifndef SOFT_SIMULTE
static std::vector<unsigned char> TakeTx()
{
    const std::vector<char>& v = COM2->Comm1->SimTxBuffer();
    std::vector<unsigned char> out(v.begin(), v.end());
    COM2->Comm1->SimClearTx();
    return out;
}
#endif

static std::string TempDir()
{
    char b[MAX_PATH + 1] = { 0 };
    ::GetTempPathA(MAX_PATH, b);
    std::string d = std::string(b) + "w906_e038_" + std::to_string((unsigned long)::GetCurrentProcessId());
    ::CreateDirectoryA(d.c_str(), 0);
    return d;
}
static std::string IniText(const char* extraLine)
{
    std::string s = "[Version]\r\nModel=9050GPIB\r\n[IndexDriver]\r\nINDEX_DRIVER_TYPE=0\r\nCONTECT_SHUTTLE_KG=10\r\nCOM_PORT=COM11\r\n";
    if (extraLine) s += std::string(extraLine) + "\r\n";
    s += "USE_HP_COM_CARD=0\r\n[System]\r\nX=1\r\n";
    return s;
}

int main()
{
    std::printf("[indexz_torque_1203] %s configuration\n",
#ifdef SOFT_SIMULTE
        "SIM (SOFT_SIMULTE)"
#else
        "SHIP (no SOFT_SIMULTE)"
#endif
    );

    // ---------------- [T1] 2704h ----------------
    {
        double p = 0.0;
        CHECK(TorquePercent(400, 1, 10, p) && p == 40.0, "[T1] raw 400 at 2704h 1/10 -> 40.0 %");
        CHECK(TorquePercent(400, 1, 1, p) && p == 400.0, "[T1] raw 400 at 2704h 1/1 -> 400 % (the ratio is used, not a fixed /10)");
        CHECK(!TorquePercent(400, 0, 10, p) && !TorquePercent(400, 1, 0, p), "[T1] 2704h:1 or :2 == 0 -> not valid (0 = not read)");
        BridgeState st; unsigned long poll = 10;
        BridgeIn a = Armed(Good(poll, -400), 0); a.sample.num = 0;
        BridgeStep(st, a);
        BridgeIn b = Armed(Good(poll + 1, -400), 10); b.sample.num = 0;
        const BridgeOut o = BridgeStep(st, b);
        CHECK(o.code == kBridgeWaiting && Has(o.why, "2704h"), "[T1] bridge: 2704h:1 == 0 -> no value written");
        BridgeState st2; unsigned long p2 = 20;
        const BridgeOut o2 = OneValue(st2, -400, p2);
        CHECK(o2.code == kBridgeWrote && o2.text == "40.00", "[T1] bridge: raw -400 pressing at 1/10 -> \"40.00\" (not 400, not 4)");
    }

    // ---------------- [T2] sign ----------------
    {
        CHECK(PressSign(false) == -1 && PressSign(true) == 0, "[T2] Mot_Table M14 Direction 0 -> press sign -1; Direction 1 -> 0 (refused)");
        BridgeState st; unsigned long poll = 100;
        BridgeOut o = OneValue(st, -400, poll);
        CHECK(o.code == kBridgeWrote && GoldenTriggers(o.text, 40), "[T2] pressing down (raw -400) -> 40.00 -> golden TorqueData>=40 triggers");
        ++poll;
        o = OneValue(st, 400, poll);
        CHECK(o.code == kBridgeWrote && o.text == " 0.00" && !GoldenTriggers(o.text, 1), "[T2] wrong direction (raw +400) -> \" 0.00\" -> never triggers, even at kg 1");
        bool sweepOk = true; int bad = 0;
        const int kgs[3] = { 10, 15, 40 };
        for (int k = 0; k < 3 && sweepOk; ++k)
            for (long raw = -2000; raw <= 2000; ++raw) {
                BridgeState s2; unsigned long p = 1000;
                const BridgeOut r = OneValue(s2, raw, p);
                const bool expect = (-raw) >= 10L * kgs[k];
                if (r.code != kBridgeWrote || GoldenTriggers(r.text, kgs[k]) != expect) { sweepOk = false; bad = (int)raw; break; }
            }
        CHECK(sweepOk, "[T2] sweep raw -2000..2000, kg 10 / 15 / 40: triggers iff -raw/10 >= kg (no abs(), no sign flip)");
        if (!sweepOk) std::printf("       first bad raw = %d\n", bad);
        BridgeState s3; unsigned long p3 = 5;
        BridgeIn a = Armed(Good(p3, -400), 0); a.pressSign = PressSign(true);
        const BridgeOut r3 = BridgeStep(s3, a);
        CHECK(r3.code == kBridgeWaiting && Has(r3.why, "Direction"), "[T2] Direction 1 -> no value at all");
    }

    // ---------------- [T3] gravity, golden default (no baseline) ----------------
    {
        BridgeState st; unsigned long poll = 200;
        BridgeOut o = OneValue(st, 80, poll);           // Z holding itself in the air: +8 % up
        CHECK(o.code == kBridgeWrote && o.text == " 0.00" && !GoldenTriggers(o.text, 1), "[T3] gravity hold raw +80 -> 0.00 (golden k<0 -> 0), no trigger at any kg");
        ++poll;
        o = OneValue(st, -150, poll);                   // at the 60E1h limit with kg 15
        CHECK(o.code == kBridgeWrote && o.text == "15.00" && GoldenTriggers(o.text, 15), "[T3] pressing at the limit raw -150 -> 15.00 -> triggers at kg 15 (real force = kg + g, as golden)");
        BridgeState sb; BaselineRequest(sb); unsigned long pb = 300;
        o = OneValue(sb, -150, pb, false);
        CHECK(o.code == kBridgeWrote && o.text == "15.00", "[T3] baseline requested but the option is OFF -> value unchanged (no silent subtraction)");
    }

    // ---------------- [T4] baseline option (port option, default off) ----------------
    {
        BridgeState st; unsigned long poll = 400;
        BridgeOut o = OneValue(st, -150, poll, true);
        CHECK(o.code == kBridgeWaiting && Has(o.why, "baseline"), "[T4] option on without a baseline request -> no value");
        BaselineRequest(st);
        const long holds[5] = { 80, 82, 78, 81, 79 };
        bool noWrite = true;
        //  still armed from the request-less attempt above (arm poll 400, last seen 401): the next five
        //  distinct polls are the five baseline samples
        for (int i = 0; i < 5; ++i) {
            ++poll;
            BridgeIn b = Armed(Good(poll, holds[i]), 2010 + i); b.baselineOpt = true;
            if (BridgeStep(st, b).code == kBridgeWrote) noWrite = false;
            BridgeIn same = b; same.nowMs += 1;                                   // the same poll again: counted once
            if (BridgeStep(st, same).code == kBridgeWrote) noWrite = false;
        }
        CHECK(noWrite, "[T4] the 5 baseline readings at standby never produce a value (nothing can step down on them)");
        CHECK(st.baselineReady && st.baseline == 80, "[T4] baseline = median of 5 distinct polls = 80");
        ++poll;
        BridgeIn c = Armed(Good(poll, -70), 2100); c.baselineOpt = true;
        o = BridgeStep(st, c);
        CHECK(o.code == kBridgeWrote && o.text == "15.00", "[T4] raw -70 with baseline 80 -> (80+70)/10 = 15.00 (triggers at R = kg)");
    }

    // ---------------- [T5] golden text ----------------
    {
        CHECK(GoldenText(15.3) == "15.30" && std::atoi(GoldenText(15.3).c_str()) == 15, "[T5] 15.3 -> \"15.30\" -> atoi 15 (golden :1812 + cContact :5775)");
        CHECK(std::atoi(GoldenText(40.95).c_str()) == 40, "[T5] 40.95 -> atoi 40 (truncation as golden)");
        CHECK(GoldenText(5.0) == " 5.00", "[T5] width 5: \" 5.00\"");
        CHECK(GoldenText(GoldenClamp(-0.0)) == " 0.00" && GoldenText(GoldenClamp(-3.0)) == " 0.00", "[T5] -0.0 / negative -> \" 0.00\", never \"-0.00\"");
        BridgeState st; unsigned long poll = 500;
        const BridgeOut o = OneValue(st, 0, poll);
        CHECK(o.code == kBridgeWrote && o.text == " 0.00", "[T5] raw 0 -> \" 0.00\"");
    }

    // ---------------- [T6] one value per arm, from a poll after the arm ----------------
    {
        BridgeState st;
        BridgeIn idle; idle.nowMs = 0;
        CHECK(BridgeStep(st, idle).code == kBridgeIdle, "[T6] not armed -> idle (golden :854-858 Task=1)");
        BridgeOut o = BridgeStep(st, Armed(Good(600, -400), 10));
        CHECK(o.code == kBridgeWaiting && Has(o.why, "poll after the arm"), "[T6] the sample of the arm's own poll is not used");
        o = BridgeStep(st, Armed(Good(600, -400), 20));
        CHECK(o.code == kBridgeWaiting, "[T6] still the same poll -> still waiting");
        o = BridgeStep(st, Armed(Good(601, -400), 30));
        CHECK(o.code == kBridgeWrote && !st.armed, "[T6] the next poll -> one value, the arm ends (golden case 40)");
        o = BridgeStep(st, Armed(Good(601, -400), 40));
        CHECK(o.code == kBridgeWaiting, "[T6] re-armed on the same poll -> no second value from that poll");
        BridgeIn z2 = Armed(Good(700, -400), 50); z2.chk1 = false; z2.chk2 = true;
        BridgeState s2; BridgeStep(s2, z2); z2.sample.poll = 701; z2.nowMs = 60;
        o = BridgeStep(s2, z2);
        CHECK(o.code == kBridgeWaiting && Has(o.why, "Z2"), "[T6] chkReadTorque2 (Z2) -> never a value on a 1203 Z1 machine");
    }

    // ---------------- [T7] validity + the golden-shaped read-error alarm ----------------
    {
        int okCount = 0; const int nCases = 9;
        for (int k = 0; k < nCases; ++k) {
            BridgeState st;
            BridgeIn a = Armed(Good(800, -400), 0);
            BridgeIn b = Armed(Good(801, -400), 10);
            const char* part = "";
            switch (k) {
                case 0: a.confirmed = b.confirmed = false; part = "not confirmed"; break;
                case 1: a.driverPanasonic = b.driverPanasonic = false; part = "INDEX_DRIVER_TYPE"; break;
                case 2: a.hookPresent = b.hookPresent = false; a.sampleRead = b.sampleRead = false; part = "hook"; break;
                case 3: b.sample.src = 1; part = "6077h SDO"; break;                     // a (possibly frozen) PDO value
                case 4: b.sample.torqueValid = false; part = "6077h SDO"; break;
                case 5: b.sample.pctValid = false; part = "unit"; break;
                case 6: b.sample.servoOn = false; part = "servo"; break;
                case 7: b.sample.alarm = true; part = "alarm"; break;
                case 8: b.sample.focusOnAxis = false; part = "focus"; break;
            }
            BridgeStep(st, a);
            const BridgeOut o = BridgeStep(st, b);
            if (o.code == kBridgeWaiting && Has(o.why, part)) ++okCount;
            else std::printf("       case %d: code %d why \"%s\"\n", k, o.code, o.why.c_str());
        }
        CHECK(okCount == nCases, "[T7] unconfirmed / Mitsubishi / no hook / PDO src / no SDO / unit / servo off / alarm / focus elsewhere -> no value, reason given");
        BridgeState st;
        BridgeIn a = Armed(Good(900, -400), 1000); a.confirmed = false;
        int alarms = 0;
        for (unsigned long t = 1000; t <= 1000 + kReadErrorMs * 2 + 5; t += 50) { a.nowMs = t; if (BridgeStep(st, a).alarm) ++alarms; }
        CHECK(alarms == 2, "[T7] armed without a value: the read-error message once per kReadErrorMs (golden :861-871 shape), not every tick");
    }

    // ---------------- [T8] torque-wait timeout (P5, not golden) ----------------
    {
        TorqueWait w;
        bool a = TorqueWaitStep(w, true, 1000), b = TorqueWaitStep(w, true, 5999), c = TorqueWaitStep(w, true, 6000), d = TorqueWaitStep(w, true, 9000);
        CHECK(!a && !b && c && !d, "[T8] waiting 5 s -> fires exactly once at 5000 ms");
        TorqueWaitStep(w, false, 9100);
        bool e = TorqueWaitStep(w, true, 9200), f = TorqueWaitStep(w, true, 14199), g = TorqueWaitStep(w, true, 14200);
        CHECK(!e && !f && g, "[T8] a value (waiting false) resets it; the next wait gets a fresh 5 s");
        CHECK(kTorqueWaitMs == 5000ul, "[T8] the limit is 5 s");
    }

    // ---------------- [T9] entry refusal (P4, not golden) ----------------
    {
        HeightGateIn g; g.z1Is1203 = true; g.confirmed = true; g.driverPanasonic = true; g.directionIsOne = false; g.hookInstalled = true;
        std::string why;
        const int modes[3] = { kModeAutoHeight, kModeManualHeight, kModeLoadCellHeight };
        bool allOk = true, refusedUnconf = true, refusedZ2 = true;
        for (int i = 0; i < 3; ++i) {
            g.contactMode = modes[i]; g.arm = 0;
            if (!HeightAllowed(g, why)) allOk = false;
            HeightGateIn u = g; u.confirmed = false;
            if (HeightAllowed(u, why) || !Has(why, "not confirmed")) refusedUnconf = false;
            HeightGateIn z = g; z.arm = 1;
            if (HeightAllowed(z, why) || !Has(why, "Z2")) refusedZ2 = false;
        }
        CHECK(allOk, "[T9] modes 1 / 2 / 8 on a confirmed PCI1203 Z1 -> allowed");
        CHECK(refusedUnconf, "[T9] modes 1 / 2 / 8 unconfirmed -> refused with a reason");
        CHECK(refusedZ2, "[T9] Z2 on a PCI1203 Z1 machine -> refused");
        HeightGateIn m = g; m.contactMode = kModeAutoHeight; m.driverPanasonic = false;
        HeightGateIn d = g; d.contactMode = kModeAutoHeight; d.directionIsOne = true;
        HeightGateIn h = g; h.contactMode = kModeAutoHeight; h.hookInstalled = false;
        CHECK(!HeightAllowed(m, why) && !HeightAllowed(d, why) && !HeightAllowed(h, why), "[T9] Mitsubishi / Direction 1 / no hook -> refused");
        HeightGateIn r = g; r.z1Is1203 = false; r.confirmed = false; r.contactMode = kModeAutoHeight;
        CHECK(HeightAllowed(r, why), "[T9] an RS-232 machine (Z1 not PCI1203) -> always allowed (golden unchanged)");
        HeightGateIn c = g; c.confirmed = false; c.contactMode = 3;
        CHECK(HeightAllowed(c, why), "[T9] contact test (mode 3) -> not this gate's business");
    }

    // ---------------- [T12] the switches are READ-ONLY (Gerneral.ini SHA-256 unchanged) ----------------
    const std::string dir = TempDir();
    const std::string ini = dir + "\\Gerneral.ini";
    TIniFile* const oldIni = INIFileGeneral;
    {
        const char* variants[3] = { 0, "HT9050_INDEXZ_TORQUE_CONFIRMED=1", "HT9050_INDEXZ_TORQUE_CONFIRMED=0" };
        const char* names[3] = { "key absent", "key = 1", "key = 0" };
        for (int v = 0; v < 3; ++v) {
            WriteBytes(ini, IniText(variants[v]));
            const std::string before = FileSha(ini);
            TIniFile* t = new TIniFile(AnsiString(ini.c_str()));
            INIFileGeneral = t;
            const W906IndexZTorqueFlags f = W906_IndexZTorqueReadFlags();
            W906_IndexZTorqueFlagsReset();
            const W906IndexZTorqueFlags& fc = W906_IndexZTorqueFlags();          // the cached path the bridge uses
            AnsiString why;
            W906_IndexZHeightAllowed(kModeAutoHeight, 0, &why);                  // the gate reads them too
            INIFileGeneral = oldIni;
            delete t;
            const std::string after = FileSha(ini);
            char m1[200];
            std::snprintf(m1, sizeof(m1), "[T12] %s: Gerneral.ini SHA-256 identical before / after every read (%.12s...)", names[v], before.c_str());
            CHECK(before == after && before != "missing", m1);
            const bool expectKey = (v != 0), expectOn = (v == 1);
            char m2[200];
            std::snprintf(m2, sizeof(m2), "[T12] %s: read as key=%d confirmed=%d (cached path agrees)", names[v], (int)expectKey, (int)expectOn);
            CHECK(f.iniOpen && f.confirmedKey == expectKey && f.confirmed == expectOn && fc.confirmed == expectOn && !f.baseline, m2);
        }
        W906_IndexZTorqueFlagsReset();
    }

    // ---------------- [T10] PCI1203 path vs Panasonic RS-232 path ----------------
    InitialOK = true;
    INDEX_DRIVER_TYPE = Panasonic_DRIVER;
    iPanasonicDriverType = Panasonic_DRIVER_A5;
    TorqueUseHPComCard = false;
    if (MOT[MTestZ1].Motor == 0) MOT[MTestZ1].Motor = new HTMotor();
    MOT[MTestZ1].Motor->Direction = false;                                       // machines/HT9050/Mot_Table.csv:16 Direction 0
    const AnsiString oldCard = MOT[MTestZ1].CardType;
    WriteBytes(ini, IniText("HT9050_INDEXZ_TORQUE_CONFIRMED=1"));
    TIniFile* const t10 = new TIniFile(AnsiString(ini.c_str()));
    INIFileGeneral = t10;
    W906_IndexZTorqueFlagsReset();
    W906_IndexZTorqueBridgeReset();
    W906_Pci1203TorqueReadHook = &FakeReadHook;
#ifdef SOFT_SIMULTE
    {
        MOT[MTestZ1].CardType = "PCI1203";
        fMain->chkReadTorque1->Checked = true;
        g_hookCalls = 0;
        CHECK(W906_Ht9050TorqueRead() == -1 && g_hookCalls == 0, "[T10] SIM: the 1203 branch is off (golden SOFT_SIMULTE reader returns at once)");
        fMain->chkReadTorque1->Checked = false;
    }
#else
    COM2->Comm1->SetSimMode(true);
    COM2->Comm1->StartComm();
    {   // (a) an RS-232 machine: golden's Panasonic reader still talks on Comm1, the hook is never asked
        MOT[MTestZ1].CardType = "";
        g_hookCalls = 0;
        TakeTx();
        fMain->edTorue0->Text = "";
        fMain->chkReadTorque1->Checked = true; fMain->chkReadTorque2->Checked = false;
        COM2->InitReadTorueTask();
        std::vector<unsigned char> tx;
        const DWORD t0 = ::GetTickCount();
        while (tx.empty() && ::GetTickCount() - t0 < 3000) { COM2->ReadTorque(); tx = TakeTx(); if (tx.empty()) ::Sleep(5); }
        CHECK(tx.size() == 1 && tx[0] == 0x05 && g_hookCalls == 0, "[T10] CardType != PCI1203: golden Panasonic reader sends ENQ on RS-232, 1203 hook not called");
        fMain->chkReadTorque1->Checked = false;
        COM2->ReadTorque();                                                      // golden: not armed -> Task=1
        TakeTx();
    }
    {   // (b) HT9050: no RS-232 at all, one value from the hook
        MOT[MTestZ1].CardType = "PCI1203";
        W906_IndexZTorqueBridgeReset();
        g_hookCalls = 0;
        g_hookSample = Good(41, -150);
        fMain->edTorue0->Text = "";
        if (fContactForm) fContactForm->PnlTorue0->Caption = "";
        fMain->chkReadTorque1->Checked = true; fMain->chkReadTorque2->Checked = false;
        COM2->InitReadTorueTask();
        COM2->ReadTorque();                                                      // arm: poll 41 recorded
        const bool waited = fMain->edTorue0->Text == "" && COM2->GetReadTorueTask() != 999;
        g_hookSample = Good(42, -150);
        COM2->ReadTorque();                                                      // next poll -> value
        CHECK(waited && g_hookCalls == 2, "[T10] PCI1203: the arm's own poll is not used; the hook is asked every armed tick");
        CHECK(fMain->edTorue0->Text == "15.00" && COM2->GetReadTorueTask() == 999 &&
              fMain->chkReadTorque1->Checked == false && fMain->chkReadTorque2->Checked == false,
              "[T10] PCI1203: edTorue0 \"15.00\", autoTask 999, chkReadTorque1/2 cleared (golden :975, :991-995)");
        CHECK(Torque[0] == 15.0 && (!fContactForm || fContactForm->PnlTorue0->Caption == "15.00"), "[T10] PCI1203: Torque[0] and PnlTorue0 as golden :976 / :1811");
        CHECK(TakeTx().empty(), "[T10] PCI1203: nothing was sent on the RS-232 torque port");
        const int calls = g_hookCalls;
        COM2->ReadTorque();
        CHECK(g_hookCalls == calls && COM2->GetReadTorueTask() == 1, "[T10] PCI1203 not armed: hook not asked (no mailbox traffic), autoTask 1 (golden :854-858)");
    }
    {   // (c) Direction 1 and (d) unconfirmed: the hook is never asked, nothing is written
        MOT[MTestZ1].Motor->Direction = true;
        W906_IndexZTorqueBridgeReset();
        g_hookCalls = 0;
        fMain->edTorue0->Text = "";
        fMain->chkReadTorque1->Checked = true;
        COM2->ReadTorque(); g_hookSample = Good(50, -400); COM2->ReadTorque();
        CHECK(g_hookCalls == 0 && fMain->edTorue0->Text == "", "[T10] Mot_Table Direction 1: hook not asked, edTorue0 stays empty");
        MOT[MTestZ1].Motor->Direction = false;
        INIFileGeneral = oldIni;
        delete t10;
        WriteBytes(ini, IniText(0));                                             // key absent
        TIniFile* const t10b = new TIniFile(AnsiString(ini.c_str()));
        INIFileGeneral = t10b;
        W906_IndexZTorqueFlagsReset();
        W906_IndexZTorqueBridgeReset();
        g_hookCalls = 0;
        COM2->ReadTorque(); g_hookSample = Good(60, -400); COM2->ReadTorque();
        CHECK(g_hookCalls == 0 && fMain->edTorue0->Text == "" && W906_IndexZTorqueLastWhy().find("not confirmed") != std::string::npos,
              "[T10] CONFIRMED key absent: hook not asked, edTorue0 stays empty, reason = not confirmed");
        AnsiString why;
        CHECK(!W906_IndexZHeightAllowed(kModeAutoHeight, 0, &why), "[T10] CONFIRMED key absent: Auto Height refused by the live gate");
        fMain->chkReadTorque1->Checked = false;
        COM2->ReadTorque();
        INIFileGeneral = oldIni;
        delete t10b;
    }
    COM2->Comm1->StopComm();
#endif
#ifdef SOFT_SIMULTE
    INIFileGeneral = oldIni;
    delete t10;
#endif
    W906_Pci1203TorqueReadHook = 0;
    MOT[MTestZ1].CardType = oldCard;
    W906_IndexZTorqueFlagsReset();
    ::DeleteFileA(ini.c_str());
    ::RemoveDirectoryA(dir.c_str());

    // ---------------- [T11] wiring source pins ----------------
    {
        const std::string root = W906_SRC_ROOT;
        std::vector<std::string> rs, th, iz, cm;
        const bool r1 = ReadLines(root + "/rs232.cpp", rs), r2 = ReadLines(root + "/EtherCAT/Pci1203TorqueHook.cpp", th);
        const bool r3 = ReadLines(root + "/IndexZTorque1203.cpp", iz), r4 = ReadLines(root + "/CMakeLists.txt", cm);
        CHECK(r1 && r2 && r3 && r4, "[T11] sources readable");
        const int fn = FindCode(rs, "void TCOM2Shim::ReadTorque()");
        const int call = FindCode(rs, "W906_Ht9050TorqueRead()", "//", fn < 0 ? 0 : fn);
        const int hp = FindCode(rs, "if(TorqueUseHPComCard)", "//", fn < 0 ? 0 : fn);
        CHECK(fn >= 0 && call == fn + 2 && hp > call && CountCode(rs, "W906_Ht9050TorqueRead()") == 1,
              "[T11] rs232.cpp: the 1203 branch is LIVE CODE (before the line's //) in TCOM2Shim::ReadTorque, ahead of the golden dispatch");
        CHECK(call >= 0 && CodeOf(rs[(std::size_t)call]).find("autoTask=999") != std::string::npos &&
              CodeOf(rs[(std::size_t)call]).find("W906_PumpComm1();") != std::string::npos,
              "[T11] rs232.cpp: same line as W906_PumpComm1(); sets autoTask=999 on a value");
        const int lim = FindCode(th, "W906_Pci1203TorqueLimitHook = &TorqueThunk;");
        CHECK(lim >= 0 && CodeOf(th[(std::size_t)lim]).find("W906_InstallPci1203TorqueReadHook(ownerThreadId)") != std::string::npos,
              "[T11] Pci1203TorqueHook.cpp: the READ hook installs on the same line as the LIMIT hook");
        CHECK(CountCode(iz, "CheckAndReadIniDataGeneral") == 0 && CountCode(iz, "->Write") == 0 && CountCode(iz, "LoadMachineConfig") == 0 &&
              CountCode(iz, "ValueExists(kSec, kKeyConfirmed)") == 1,
              "[T11] IndexZTorque1203.cpp: the switches are read with ValueExists / ReadInteger only (no write-back path)");
        CHECK(FindCode(cm, "rs232.cpp  IndexZTorque1203.cpp", "#") >= 0 && FindCode(cm, "EtherCAT/Pci1203TorqueHook.cpp EtherCAT/Pci1203TorqueRead.cpp", "#") >= 0,
              "[T11] CMakeLists.txt: IndexZTorque1203.cpp next to rs232.cpp (ht9045_sm), Pci1203TorqueRead.cpp next to the limit hook (wb_serve)");
    }

    std::printf("[indexz_torque_1203] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
