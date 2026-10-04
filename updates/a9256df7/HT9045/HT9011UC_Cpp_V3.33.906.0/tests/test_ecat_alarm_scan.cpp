// ===========================================================================
//  tests/test_ecat_alarm_scan.cpp -- AI(W906-E045) 20261004 [W906] (St01 ST01-E2)
//
//  todo E-045 (S-26 R4 + Steven 1004 17:0x Q98): the PURE rule of EcatAlarmScan.h -- the alarm verdict of one HT9050
//  1203 row from its monitor sample, and the Q98 episode (one clear-alarm attempt; not cleared = power-cycle latch).
//  Header only: no machine state, no file, no route. The live path through the real ScanAllMotorStatus is
//  tests/test_ecat_alarm_scan_live.cpp.
//    P1  ALM bit                          -> alarm          P5  not HT9050                       -> golden (no alarm)
//    P2  ERROR_STOP, ALM 0                -> alarm          P6  not a 1203 row / Enable 0 / no sample -> no alarm
//    P3  READY, ALM 0                     -> no alarm       P7  EMG / motor power off            -> no alarm (EMG path reports)
//    P4  pending sample                   -> no verdict     P8  the episode: one reset, cleared / latched, no second reset,
//                                                               HOME owns the reset, power-out ends it, one line per episode
// ===========================================================================
#include <cstdio>
#include "EcatAlarmScan.h"

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_ecat_alarm_scan.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static const unsigned short kReady = 1;   // STA_AX_READY (kEcStaReady)

static TEcAlarmIn Base()                  // an HT9050 1203 row, power in, a fresh READY sample
{
    TEcAlarmIn in;
    in.ht9050 = true; in.isEcatRow = true; in.enable = true; in.readOk = true;
    in.pending = false; in.outOfPower = false; in.homing = false; in.armed = true;   // armed = fully homed (fAllMotorHome)
    in.state = kReady; in.motionIO = 0;
    return in;
}
static TEcAlarmIn Alm()      { TEcAlarmIn in = Base(); in.motionIO = 0x2; return in; }
static TEcAlarmIn ErrStop()  { TEcAlarmIn in = Base(); in.state = kEcStaErrorStop; return in; }

int main()
{
    std::printf("=== test_ecat_alarm_scan (AI(W906-E045), S-26 R4 + Q98) ===\n");

    // ---- P1..P7: the verdict ----
    CHECK(W906_EcAlarmVerdict(Alm()));                                       // P1 ALM bit 1
    { TEcAlarmIn in = Alm(); in.motionIO = 0x2 | 0x4 | 0x40; CHECK(W906_EcAlarmVerdict(in)); }   // P1b ALM with LMT+ / EMG bits
    CHECK(W906_EcAlarmVerdict(ErrStop()));                                   // P2 ERROR_STOP, ALM 0
    CHECK(!W906_EcAlarmVerdict(Base()));                                     // P3 READY, no ALM
    { TEcAlarmIn in = Base(); in.motionIO = 0x4 | 0x8 | 0x10 | 0x4000; CHECK(!W906_EcAlarmVerdict(in)); }   // P3b LMT / ORG / SVON bits are not an alarm
    { TEcAlarmIn in = ErrStop(); in.pending = true; CHECK(!W906_EcAlarmVerdict(in)); }          // P4 pending
    { TEcAlarmIn in = ErrStop(); in.ht9050 = false; CHECK(!W906_EcAlarmVerdict(in)); }          // P5 not HT9050
    { TEcAlarmIn in = ErrStop(); in.isEcatRow = false; CHECK(!W906_EcAlarmVerdict(in)); }       // P6a not a 1203 row
    { TEcAlarmIn in = ErrStop(); in.enable = false; CHECK(!W906_EcAlarmVerdict(in)); }          // P6b Enable 0
    { TEcAlarmIn in = ErrStop(); in.readOk = false; CHECK(!W906_EcAlarmVerdict(in)); }          // P6c no sample (unclaimed)
    { TEcAlarmIn in = ErrStop(); in.outOfPower = true; CHECK(!W906_EcAlarmVerdict(in)); }       // P7 power out

    // ---- the same gates in the episode step ----
    { TEcAlarmRow r; TEcAlarmIn in = ErrStop(); in.ht9050 = false;
      const TEcAlarmStep s = W906_EcAlarmStep(in, r);
      CHECK(!s.raise && !s.resetError && !s.noteAlarm && !r.attempted); }                       // P5 step: golden, nothing
    { TEcAlarmRow r; TEcAlarmIn in = ErrStop(); in.pending = true;
      const TEcAlarmStep s = W906_EcAlarmStep(in, r);
      CHECK(!s.raise && !s.resetError && !r.attempted); }                                        // P4 step: a pending sample opens nothing
    { TEcAlarmRow r; TEcAlarmIn in = ErrStop(); in.outOfPower = true;
      const TEcAlarmStep s = W906_EcAlarmStep(in, r);
      CHECK(!s.raise && !s.resetError && !s.powerReset); }                                       // P7 step

    // ---- P8: the Q98 episode ----
    {   // cleared: alarm -> one reset -> pending -> READY = cleared
        TEcAlarmRow r;
        TEcAlarmStep s = W906_EcAlarmStep(ErrStop(), r);
        CHECK(s.raise && s.noteAlarm && s.resetError && !s.latchedNow && r.attempted);          // P8a one reset, one line
        TEcAlarmIn p = ErrStop(); p.pending = true;
        s = W906_EcAlarmStep(p, r);
        CHECK(s.raise && !s.resetError && !s.noteAlarm && r.attempted);                         // P8b pending: episode open, no 2nd reset
        s = W906_EcAlarmStep(Base(), r);
        CHECK(!s.raise && s.cleared && !s.latchedNow && !r.attempted && !r.latched && !r.noted); // P8c cleared
        s = W906_EcAlarmStep(Base(), r);
        CHECK(!s.raise && !s.cleared);                                                           // P8d nothing more
    }
    {   // not cleared: alarm -> reset -> the next fresh sample still alarms = latched, never a 2nd reset
        TEcAlarmRow r;
        TEcAlarmStep s = W906_EcAlarmStep(Alm(), r);
        CHECK(s.resetError);
        s = W906_EcAlarmStep(Alm(), r);
        CHECK(s.raise && s.latchedNow && !s.resetError && !s.noteAlarm && r.latched && !r.attempted);   // P8e latched
        int resets = 0, latches = 0;
        for (int k = 0; k < 10; ++k) { s = W906_EcAlarmStep(Alm(), r); resets += s.resetError; latches += s.latchedNow; CHECK(s.raise); }
        CHECK(resets == 0 && latches == 0 && r.latched);                                         // P8f no retry loop, one message
        TEcAlarmIn p = Alm(); p.pending = true;
        s = W906_EcAlarmStep(p, r);
        CHECK(s.raise && r.latched);                                                             // P8g latched stays raised while pending
        TEcAlarmIn off = Alm(); off.outOfPower = true;
        s = W906_EcAlarmStep(off, r);
        CHECK(s.powerReset && !s.raise && !r.latched && !r.attempted);                          // P8h power cycle ends the latch
        s = W906_EcAlarmStep(Alm(), r);
        CHECK(s.raise && s.noteAlarm && s.resetError);                                           // P8i ... and gives one new attempt
    }
    {   // a latched row whose alarm goes away (e.g. HOME's own InitMotor reset) = cleared
        TEcAlarmRow r;
        W906_EcAlarmStep(ErrStop(), r); W906_EcAlarmStep(ErrStop(), r);
        CHECK(r.latched);
        const TEcAlarmStep s = W906_EcAlarmStep(Base(), r);
        CHECK(s.cleared && !r.latched);                                                          // P8j
    }
    {   // during HOME (SystemStart && iHome==1) no reset of ours
        TEcAlarmRow r;
        TEcAlarmIn h = ErrStop(); h.homing = true;
        TEcAlarmStep s = W906_EcAlarmStep(h, r);
        CHECK(s.raise && s.noteAlarm && !s.resetError && !r.attempted);                         // P8k
        s = W906_EcAlarmStep(h, r);
        CHECK(s.raise && !s.noteAlarm && !s.resetError && !s.latchedNow);                       // P8l still nothing, one line only
        s = W906_EcAlarmStep(ErrStop(), r);                                                      // HOME ended, alarm still there
        CHECK(s.resetError && !s.noteAlarm);                                                     // P8m now the one attempt
    }
    {   // power-out with no open episode: no powerReset line
        TEcAlarmRow r; TEcAlarmIn off = Base(); off.outOfPower = true;
        CHECK(!W906_EcAlarmStep(off, r).powerReset);                                             // P8n
    }

    // ---- P9: armed only when the episode starts fully homed (ST01-M 1004 22:3x: never refuse the first HOME) ----
    {   // boot: ERROR_STOP before the first HOME -> golden raise only, no reset, no latch, ever (HOME resets it)
        TEcAlarmRow r; TEcAlarmIn b = ErrStop(); b.armed = false;
        TEcAlarmStep s = W906_EcAlarmStep(b, r);
        CHECK(s.raise && s.noteAlarm && !s.resetError && !r.armed);                             // P9a
        int resets = 0, latches = 0;
        for (int k = 0; k < 10; ++k) { s = W906_EcAlarmStep(b, r); resets += s.resetError; latches += s.latchedNow; CHECK(s.raise); }
        CHECK(resets == 0 && latches == 0 && !r.latched);                                        // P9b
        TEcAlarmIn homed = ErrStop();                                                            // fAllMotorHome turns true while
        s = W906_EcAlarmStep(homed, r);                                                          // the same episode is still open:
        CHECK(!s.resetError && !s.latchedNow && !r.latched);                                     // P9c armed is decided at the start
        s = W906_EcAlarmStep(Base(), r);                                                         // HOME's InitMotor cleared it
        CHECK(s.cleared && !r.armed && !r.noted);                                                // P9d
        s = W906_EcAlarmStep(ErrStop(), r);                                                      // a NEW episode while homed
        CHECK(s.resetError && r.armed);                                                          // P9e armed: the one reset
    }
    {   // a power-out ends an armed episode; the next one is decided afresh (not homed after power-up = unarmed)
        TEcAlarmRow r;
        W906_EcAlarmStep(ErrStop(), r); W906_EcAlarmStep(ErrStop(), r);
        CHECK(r.latched && r.armed);
        TEcAlarmIn off = ErrStop(); off.outOfPower = true;
        W906_EcAlarmStep(off, r);
        CHECK(!r.latched && !r.armed);                                                           // P9f
        TEcAlarmIn b = ErrStop(); b.armed = false;                                               // power back, fAllMotorHome false
        TEcAlarmStep s = W906_EcAlarmStep(b, r);
        CHECK(s.raise && !s.resetError && !r.armed);                                             // P9g
        s = W906_EcAlarmStep(b, r);
        CHECK(!s.latchedNow && !r.latched);                                                      // P9h
    }

    std::printf("%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
