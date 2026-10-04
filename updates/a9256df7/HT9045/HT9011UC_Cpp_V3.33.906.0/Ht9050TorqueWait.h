// =============================================================================
//  Ht9050TorqueWait.h -- E-044 (SAFETY): on HT9050 the production Index torque wait
//  (DoTestHeadMotor case 12110, and its arm-2 twin 14110) is bounded; after kWaitMs
//  without a torque value, while auto-run drives the wait, the machine alarms and stops.
//
//  AI(W906-E044) 20261004 (St01 / ST01-E): NEW FILE, NOT GOLDEN.
//  Steven 20261004 08:4x ruling: 「加逾時：報警並停機」(registry E-044,
//  .claude/skills/ht9050-construction/references/decisions-decided.md).
//  Plan: D:\AI_TempFile\st01e-e044-plan-20261004.md. ST01-M change request 1004 15:4x
//  (from ST01-E2's R4 plan D:\AI_TempFile\st01e2-r4-plan-20261004.md s3): alarm only in auto-run,
//  re-arm on every entry, one alarm after an R4 drive alarm.
//
//  WHAT GOLDEN DOES (golden 0618 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618):
//    atester.cpp:6436-6468 (case 12110) / :7066-7099 (case 14110) wait until
//    fMain->edTorue0 / edTorue1 has a value. The only error exit (:6456-6460 / :7087-7091)
//    needs COM2->GetReadTorueTask()==999, which golden's reader sets only after a SUCCESSFUL
//    read (rs232.cpp:991-995). HT9050's Index Z1 is on the PCI-1203; it has no RS-232 torque
//    drive (port rs232.cpp:263-266 never opens the port), so until B7 (the V910 DoTestHead
//    port) or E-038 with CONFIRMED=1 nothing ever arrives and the wait is endless, with Z1
//    pressed on the socket (S-26 finding 10-1).
//
//  WHAT THIS ADDS (NOT GOLDEN), HT9050 only (MOT[MTestZ1].CardType=="PCI1203", the same
//  test as port rs232.cpp:950):
//    * atester.cpp:6908 / :7538 -- one same-line insert each, at the first statement of the
//      case, so every wait tick is seen, including the arming tick.
//    * the 5 s window below (pure, both build arms, unit-tested);
//    * W906_Ht9050TorqueWaitAlarm: disarm the reader + golden's alarm-stop (ShowErrorMessage);
//      atester then runs golden's own 12110 exit (fAllMotorHome=false; Task=1; golden :6459-6460).
//  Every other machine: the inline `&&` in atester stops at the CardType compare; nothing
//  here runs. SOFT_SIMULTE: inert (12110 is unreachable there, golden :6378-6380 / :6404-6406).
//
//  ⚠ R4 (todo E-045: a mid-move Z drive alarm must raise golden ShowMotorErrorMessage once) should
//  ship BEFORE or WITH E-044. Without it a Z1 drive alarm during the press is never raised, the
//  wait just continues, and after 5 s it reads as this torque timeout. E-045's owner and timing are
//  Steven's call. Once R4 raises ShowMotorErrorMessage, E-044 stays silent: that body sets
//  fAllMotorHome=false first (golden note.cpp:1059; port forms/fNote_ShowError.cpp:663), which
//  makes this wait ineligible (see Eligible below) -- one alarm, not two.
//
//  Tests: tests/test_e044_torque_wait.cpp (ctest E044TorqueWait).
// =============================================================================
#ifndef Ht9050TorqueWaitH
#define Ht9050TorqueWaitH

#include <cstdint>

namespace ht9045 {
namespace e044 {

//  NOT GOLDEN. 5 s: Steven 1004 14:1x confirmed "keep as designed". It is the port's limit for every
//  other HT9050 Index Z torque wait (E-038 / E-042 P5, IndexZTorqueCore.h kTorqueWaitMs on
//  v906/st01e-e038), and earlier than E-038's 10 s "source not confirmed" repeat message, so the
//  operator sees one coded alarm. golden's own 10 s ReadTorqueDelay (golden atester.cpp:6424,
//  "read torque wait alarm time") is left untouched.
const std::uint32_t kWaitMs = 5000u;

struct Wait {
    bool          active;     // a window is running
    std::uint32_t startMs;
    bool          havePass;   // lastPass is valid
    std::uint32_t lastPass;   // MainProc pass of this arm's previous 12110 / 14110 tick
    Wait() : active(false), startMs(0), havePass(false), lastPass(0) {}
};

//  Is auto-run driving this wait? The auto-run ladder's own guard for DoTestHeadMotor
//  (port csystem.cpp:1686 / :1934 = golden: `if(SoftStop==true || SystemStart==false || fAllMotorHome==false) return;`)
//  and not HOME (iHome==1, port csystem.cpp:31224 = golden csystem.cpp:17590 -- golden's HOME path calls
//  DoTestHeadMotor itself for [I01] TesterFinishThenHome, port :31225-31231; and so does the stopped
//  branch, port :32632-32648). Not running, HOME, or after a drive alarm (fAllMotorHome=false) = never fire.
inline bool Eligible(bool systemStart, bool softStop, bool allMotorHome, int iHome)
{
    return systemStart && !softStop && allMotorHome && iHome != 1;
}

//  One 12110 / 14110 tick.
//    newWait  : golden's own arming condition this tick (bFirstTime && DoTestHeadMotorDelay on,
//               atester.cpp:6910-6912);
//    pass     : the MainProc pass number (GetMainProcCallCount, golden csystem.cpp:16673-16709). A gap of
//               more than one pass since this arm's previous tick = the SM was elsewhere or the machine
//               was stopped = a NEW entry: the window re-arms, nothing from an earlier pass carries over
//               (also when golden's arming tick is skipped because DoTestHeadMotorDelay already ran out);
//    waiting  : edTorueN->Text=="" (golden atester.cpp:6923); a tick with a value resets;
//    eligible : Eligible(...) above; an ineligible tick resets and never fires.
//  Returns true ONCE when an eligible wait has lasted >= limitMs within one entry; the window then
//  re-arms (a wait that is somehow still there alarms again only after another limitMs).
//  Wrap-safe on 32-bit tick and pass counters.
inline bool Step(Wait& w, bool newWait, std::uint32_t pass, bool waiting, bool eligible,
                 std::uint32_t nowMs, std::uint32_t limitMs = kWaitMs)
{
    const bool newEntry = newWait || !w.havePass || (std::uint32_t)(pass - w.lastPass) > 1u;
    w.havePass = true;
    w.lastPass = pass;
    if (newEntry || !waiting || !eligible) w.active = false;
    if (!waiting || !eligible) return false;
    if (!w.active) { w.active = true; w.startMs = nowMs; return false; }
    if ((std::uint32_t)(nowMs - w.startMs) >= limitMs) { w.active = false; return true; }
    return false;
}

}  // namespace e044
}  // namespace ht9045

//  atester.cpp:6908 (arm 0, case 12110) / :7538 (arm 1, case 14110), declared there at block scope.
//  false at once under SOFT_SIMULTE or when MTestZ1 is not a PCI1203 row.
bool W906_Ht9050TorqueWaitTimedOut(int arm, bool newWait, bool waiting);
//  Op log, disarm chkReadTorque1/2, golden alarm-stop (ShowErrorMessage). The caller then runs
//  golden's 12110 / 14110 exit: fAllMotorHome=false; Task=1; return;
void W906_Ht9050TorqueWaitAlarm(int arm);

//  Test seams. Clock: 0 = ::GetTickCount(). Pass: 0 = GetMainProcCallCount() (csystem.h:52).
//  Calls: how often TimedOut was entered (proves the atester short-circuit on non-HT9050 machines).
//  Reset: forget both windows.
extern unsigned long (*W906_Ht9050TorqueWaitClock)();
extern unsigned long (*W906_Ht9050TorqueWaitPass)();
extern int W906_Ht9050TorqueWaitCalls;
void W906_Ht9050TorqueWaitReset();
//  The alarm codes in use (TODO: Jimmy picks, see Ht9050TorqueWait.cpp).
const char* W906_Ht9050TorqueWaitAlarmCode(int arm);

#endif  // Ht9050TorqueWaitH
