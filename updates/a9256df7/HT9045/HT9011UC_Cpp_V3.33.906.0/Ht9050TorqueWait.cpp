// =============================================================================
//  Ht9050TorqueWait.cpp -- E-044 (SAFETY). See Ht9050TorqueWait.h.
//
//  AI(W906-E044) 20261004 (St01 / ST01-E): NEW FILE, NOT GOLDEN (golden has no HT9050 / 1203).
//  Where a line mirrors golden, the golden 0618 file:line is on it
//  (D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618).
//
//  Callers: atester.cpp:6908 (DoTestHeadMotor case 12110, arm 0) and :7538 (case 14110, arm 1),
//  same-line inserts that first test MOT[MTestZ1].CardType=="PCI1203" themselves, so on every
//  other machine neither function is ever entered.
//  This file reads no INI, writes no file, and writes none of SystemStart / SoftStart / SoftStop /
//  fAllMotorHome / iHome (it only READS them, Eligible): the stop is golden's own (ShowErrorMessage),
//  the exit is golden's own (atester).
//  ⚠ R4 (E-045) should ship before or with E-044 -- see Ht9050TorqueWait.h; until then a Z1 drive alarm
//  during the press is not raised and shows up here as the torque timeout.
// =============================================================================
#include "Ht9050TorqueWait.h"
#include "MachineType.h"            // SOFT_SIMULTE
#include "cmydef.h"                 // MTestZ1 / MTestZ2, K_RETRY / K_SKIP
#include "cprod.h"                  // CosFunction.bIndexAreaOnlyCanUseSkip
#include "Motor/mymotor.h"          // MOT[] (CardType)
#include "FormsFacade.h"            // fMain (chkReadTorque1/2)
#include "canary_support.h"         // ShowErrorMessage (golden note.h:466)
#include <windows.h>                // GetTickCount
#include <cstdio>

unsigned int GetMainProcCallCount();                                           // csystem.h:52 (golden csystem.h:14; body port csystem.cpp:538 = golden csystem.cpp:16673-16709)

unsigned long (*W906_Ht9050TorqueWaitClock)() = 0;
unsigned long (*W906_Ht9050TorqueWaitPass)() = 0;
int W906_Ht9050TorqueWaitCalls = 0;

namespace {
ht9045::e044::Wait g_wait[2];

//  TODO(E-044) alarm code: Jimmy picks (WAR0361/0362 vs new WAR0363/0364; K_SKIP vs K_RETRY).
//  Placeholder = golden's existing pair "Index arm 1/2 contact torque monitor error."
//  (port AlarmCodeCatalog.cpp:186-187; golden atester.cpp:9567 / :9665 raise them for the
//  torque-monitor deviation). New codes would need AlarmCodeCatalog.cpp + web/JSON
//  Alarm-description.json + AlarmCodeList-index.json rows; only these two literals change here.
const char* const kAlarmCode[2] = { "WAR0361", "WAR0362" };
}  // namespace

const char* W906_Ht9050TorqueWaitAlarmCode(int arm) { return kAlarmCode[arm == 1 ? 1 : 0]; }

void W906_Ht9050TorqueWaitReset() { g_wait[0] = g_wait[1] = ht9045::e044::Wait(); }

bool W906_Ht9050TorqueWaitTimedOut(int arm, bool newWait, bool waiting)
{
    ++W906_Ht9050TorqueWaitCalls;
#ifdef SOFT_SIMULTE
    (void)arm; (void)newWait; (void)waiting;
    return false;                                     // golden SOFT_SIMULTE never reaches 12110 (golden :6378-6380 / :6404-6406)
#else
    if (MOT[MTestZ1].CardType != AnsiString("PCI1203")) return false;   // every RS-232 machine: golden, unchanged (same test as rs232.cpp:950)
    const std::uint32_t now  = W906_Ht9050TorqueWaitClock ? (std::uint32_t)W906_Ht9050TorqueWaitClock()
                                                          : (std::uint32_t)::GetTickCount();
    const std::uint32_t pass = W906_Ht9050TorqueWaitPass ? (std::uint32_t)W906_Ht9050TorqueWaitPass()
                                                         : (std::uint32_t)GetMainProcCallCount();
    //  Only while auto-run drives the wait (ST01-M 1004 15:4x): the ladder's guard (csystem.cpp:1686 / :1934)
    //  and not HOME (iHome==1, csystem.cpp:31224). Read only -- this file writes none of these.
    const bool eligible = ht9045::e044::Eligible(SystemStart, SoftStop, fAllMotorHome, iHome);
    return ht9045::e044::Step(g_wait[arm == 1 ? 1 : 0], newWait, pass, waiting, eligible, now);
#endif
}

void W906_Ht9050TorqueWaitAlarm(int arm)
{
    const int a = (arm == 1) ? 1 : 0;
    const int secs = (int)(ht9045::e044::kWaitMs / 1000u);
    const char* const code = kAlarmCode[a];
    //  TODO(E-044) Jimmy picks K_SKIP vs K_RETRY. Default = golden's Index-area convention
    //  (golden atester.cpp:6588-6595, case 12112: "Steven 20141105 : Index內的所有異常都只能用Skip").
    //  The answer is not used: the caller leaves through golden's 12110 exit either way.
    const int kcode = CosFunction.bIndexAreaOnlyCanUseSkip ? K_SKIP : K_RETRY;
    char part[160];
    std::snprintf(part, sizeof(part), "HT9050 Index Z%d torque not read in %d s (DoTestHeadMotor %d, E-044). Stopped; HOME before START.",
                  a + 1, secs, a ? 14110 : 12110);

    std::printf("[E-044] HT9050 Index Z%d torque not read within %u ms at DoTestHeadMotor %d -> %s (placeholder, Jimmy picks), "
                "chkReadTorque1/2 cleared, fAllMotorHome=false, Task=1 (golden atester.cpp:%s)\n",
                a + 1, (unsigned)ht9045::e044::kWaitMs, a ? 14110 : 12110, code, a ? "7090-7091" : "6459-6460");
    std::printf("[E-044] HT9050 Index Z%d 扭力 %d 秒內沒讀到（DoTestHeadMotor %d）-> 警報、停機、下次 START 先回原點\n",
                a + 1, secs, a ? 14110 : 12110);
    std::fflush(stdout);

    //  Disarm the reader first, golden's shape: reader case 40 clears both flags (golden rs232.cpp:993-994;
    //  port rs232.cpp:544-545), DoTestHeadMotor 12200 too (golden atester.cpp:6626-6627; port :7097-7098).
    //  Golden's RS-232 reader then idles (golden rs232.cpp:854-858; port :405-410), so do E-038's bridge
    //  and its 10 s message; no Comm1 StopComm/StartComm is ever tried on HT9050's unopened port.
    fMain->chkReadTorque1->Checked = false;
    fMain->chkReadTorque2->Checked = false;

    //  Golden's alarm-stop. In wb_serve ForwardShowErrorMessage stops FIRST (tools/wb_serve.cpp:442 ->
    //  W906_AlarmStopLikeGolden = golden note.cpp:795-801: StopAllMotor(true), SoftStop=SoftStart=SystemStart=false),
    //  records the alarm (golden note.cpp:839-868), and only then shows the box or returns unattended.
    ShowErrorMessage(code, kcode, a ? MTestZ2 : MTestZ1, false, AnsiString(part));
}
