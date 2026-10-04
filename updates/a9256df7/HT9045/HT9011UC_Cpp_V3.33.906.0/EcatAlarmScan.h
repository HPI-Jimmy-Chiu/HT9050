// =============================================================================
//  EcatAlarmScan.h -- HT9050: a PCI-1203 drive alarm in the engine is raised, and cleared once (else power-cycle)
//
//  AI(W906-E045) 20261004 [W906] (St01 ST01-E2): new file. todo E-045 = S-26 R4 (docs/handoff/S26_FINDINGS_20261004.md,
//  v906/steven-handoff) + Steven 1004 17:0x Q98: 「Error stop的軸要嘗試 clear alarm, 如果不能clear, 就只能全機斷電重置」.
//  Plan: D:\AI_TempFile\st01e2-r4-plan-20261004.md.
//
//  THE HOLE (golden too)
//      golden ScanAllMotorStatus (golden 0618 csystem.cpp:3985; V912 :4122; port csystem.cpp:15931) re-reads the LEDs of
//      only MInArmX/Y, MOutArmX/Y, MTrayX in auto-run (golden :4031, port :15996) and raises only when PServoAlarmOn==1
//      (golden :4051-4053, port :16016-16018). A 1203 axis that stops in ERROR_STOP never reads READY
//      (TMyEtherCatMotor::MotionDone, golden myEthercatmotor.cpp:1201-1223, port :1595-1599), so the station waits for
//      ever and nothing raises. On HT9050 that is M11 / M17 / M18 (not re-read) and every ServoAlarmOn=0 row (M03, M22,
//      M35-M42, M108).
//
//  THE FIX (HT9050 only; every other model = golden, untouched)
//      csystem.cpp ScanAllMotorStatus calls W906_EcAlarmScanRow(i) on the line before golden's raise `if` and ORs it
//      into the PServoAlarmOn term -- the raise BODY stays golden's (auto-run: one JAM, SystemStart=false, iHome=1;
//      idle: HomeFlag=0, fAllMotorHome=false). Per claimed 1203 row, from the 1203 monitor's sample (no card read,
//      no timer):
//        * alarm = ALM bit (motionIO bit 1) || state == ERROR_STOP, on a sample that is not pending, while motor
//          power is in (EMG / SnMotorPower off = golden's EMG path reports; IsIndexMotorOutOfPower's HT9050 arm,
//          csystem.cpp:19975, read from the sensors only -- IsEMGPressed's brake / servo-off side effects stay golden's
//          once-per-pass call);
//        * Q98: the first alarm sample of an episode issues ONE golden reset (TMyEtherCatMotor::ResetState = golden
//          :1764-1773 Acm_AxResetError, through the route; the route marks the sample pending);
//          the first non-pending sample after it decides: no alarm = CLEARED (the axis already needs a home from the
//          golden body -- Steven Q88); still alarm = LATCHED: one operator message "power-cycle the whole machine,
//          then HOME" and W906_EcAlarmStartAllowed refuses START / HOME (csystem.cpp MainProc SoftStart gate).
//          No second reset while the alarm stays; the episode ends on a no-alarm sample or a power-out (= power cycle).
//        * while HOME runs (SystemStart && iHome==1) no reset of ours: HOME's InitMotor resets (golden).
//        * ARMED only for an episode that starts while fAllMotorHome is true (fully homed since boot / the last motor
//          power-off; golden clears it at both, csystem.cpp:16517-16546). After power-up 1203 axes can sit in
//          ERROR_STOP until HOME's InitMotor resets them (Pci1203Monitor.cpp:2774, 13 axes measured 20260911), and a
//          drive still coming up would fail our one reset -- so before the first full HOME only golden's raise applies:
//          no reset, no latch, HOME is never refused (ST01-M 1004 22:3x, the machine run of 10/05).
//      Stop / pause / abort are never refused.
// =============================================================================
#ifndef ECATALARMSCAN_H
#define ECATALARMSCAN_H

#include <string>
#include "Motor/EcatMotorRoute.h"   // kEcStaErrorStop (plain POD header, no machine model)

// ---- the pure rule (inline: tests/test_ecat_alarm_scan.cpp compiles this header alone) ---------------------------
struct TEcAlarmIn {
    bool           ht9050;      // W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050
    bool           isEcatRow;   // MOT[i].Motor is a TMyEtherCatMotor
    bool           enable;      // ... and Enable
    bool           readOk;      // W906_EcRead: the route claimed the axis and the monitor has a valid sample
    bool           pending;     // a state-changing command (ours: the reset) has not been seen by a Poll yet
    bool           outOfPower;  // EMG / motor power off, from the sensors (EcatAlarmScan.cpp MotorPowerOut)
    bool           homing;      // SystemStart && iHome == 1
    bool           armed;       // fAllMotorHome: fully homed since boot / the last motor power-off (golden clears it there)
    unsigned short state;       // STA_AX_* (kEcStaErrorStop = 3)
    unsigned long  motionIO;    // AX_MOTION_IO_* bits (bit 1 = ALM)
    TEcAlarmIn() : ht9050(false), isEcatRow(false), enable(false), readOk(false), pending(false), outOfPower(false),
                   homing(false), armed(false), state(0), motionIO(0) {}
};

// One row's alarm episode.
struct TEcAlarmRow {
    bool attempted;   // the one reset was issued; waiting for the first non-pending sample after it
    bool latched;     // that sample still alarmed: power-cycle needed, START / HOME refused
    bool noted;       // the episode's first line was written
    bool armed;       // the episode started while armed (fAllMotorHome): only then the Q98 reset + latch apply
    bool ledForced;   // this file set MOT[i].Led[iAlarmLed] (the live layer clears it when the episode ends)
    TEcAlarmRow() : attempted(false), latched(false), noted(false), armed(false), ledForced(false) {}
};

// What one scan of one row does.
struct TEcAlarmStep {
    bool raise;        // OR'd into golden's raise condition (csystem.cpp ScanAllMotorStatus)
    bool noteAlarm;    // first alarm sample of the episode: one log line
    bool resetError;   // issue the one golden reset now
    bool cleared;      // the episode ended on a no-alarm sample
    bool latchedNow;   // the reset did not clear it: one operator message, START / HOME refused from now on
    bool powerReset;   // an open episode ended by a power-out
    TEcAlarmStep() : raise(false), noteAlarm(false), resetError(false), cleared(false), latchedNow(false), powerReset(false) {}
};

// The alarm judgement alone.
inline bool W906_EcAlarmVerdict(const TEcAlarmIn& in)
{
    if (!in.ht9050 || !in.isEcatRow || !in.enable) return false;   // not an HT9050 engine 1203 row: golden
    if (!in.readOk || in.pending || in.outOfPower) return false;   // no fresh sample / power out: no verdict
    return ((in.motionIO >> 1) & 1ul) != 0 || in.state == kEcStaErrorStop;
}

// The verdict + the Q98 episode of one row (one call per ScanAllMotorStatus visit of the row).
inline TEcAlarmStep W906_EcAlarmStep(const TEcAlarmIn& in, TEcAlarmRow& r)
{
    TEcAlarmStep s;
    if (!in.ht9050 || !in.isEcatRow || !in.enable) return s;
    if (in.outOfPower) {                                             // golden's EMG / power path reports; a power-out ends the episode
        s.powerReset = r.attempted || r.latched;
        r.attempted = false; r.latched = false; r.noted = false; r.armed = false;
        return s;
    }
    if (!in.readOk) return s;                                        // nothing to judge: keep the episode as it is
    if (in.pending) {                                                // our reset (or another command) not seen by a Poll yet
        s.raise = r.attempted || r.latched;                          // an open episode keeps golden's idle branch (HomeFlag=0)
        return s;
    }
    const bool alarm = ((in.motionIO >> 1) & 1ul) != 0 || in.state == kEcStaErrorStop;
    if (!alarm) {
        s.cleared = r.attempted || r.latched || r.noted;
        r.attempted = false; r.latched = false; r.noted = false; r.armed = false;
        return s;
    }
    s.raise = true;
    if (!r.noted) { s.noteAlarm = true; r.noted = true; r.armed = in.armed; }   // armed is decided when the episode starts
    if (in.homing || r.latched || !r.armed) return s;                // HOME's InitMotor resets; latched = no second reset;
                                                                     // not armed (no full HOME since boot / the last motor
                                                                     // power-off: drives may still be coming up) = golden's
                                                                     // raise only -- never a latch that would refuse HOME
    if (!r.attempted) { s.resetError = true; r.attempted = true; return s; }
    r.attempted = false; r.latched = true; s.latchedNow = true;      // the first sample after the reset still alarms
    return s;
}

// ---- the live layer (EcatAlarmScan.cpp; MOT[], the route, the globals) ---------------------------------------
bool W906_EcAlarmScanRow(int motIndex);              // csystem.cpp ScanAllMotorStatus, before golden's raise `if`
bool W906_EcAlarmStartAllowed(const char* where);    // csystem.cpp MainProc SoftStart gate: false = drop SoftStart
bool W906_EcAlarmMotionAllowed(std::string* why);    // for E-043 commit 2's shared gate (manual moves): false while latched
int  W906_EcAlarmLatchedCount();
void W906_EcAlarmSetNote(void (*sink)(const char* line));   // 0 = default (stdout + RecordProcess)
void W906_EcAlarmResetAll();                         // tests: forget every episode

#endif // ECATALARMSCAN_H
