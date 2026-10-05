// =============================================================================
//  EtherCAT/Pci1203TorqueRead.cpp -- E-038 (Q87, SAFETY): the live half of HT9050's Index Z torque
//  READ. wb_serve only (it needs MotorAccessLiveBackend(), WebMotorAccessLive.cpp's), next to
//  EtherCAT/Pci1203TorqueHook.cpp, which installs it in the same breath as the torque-LIMIT hook
//  (EtherCAT/Pci1203TorqueHook.cpp:79), i.e. only when the Index Z Galil route installs.
//
//  AI(W906-E038) 20261003 (St01 / ST01-E): NEW FILE, NOT GOLDEN (golden reads the torque over
//  RS-232, golden 0618 rs232.cpp:820-1000; HT9050 has no RS-232 torque drive, port rs232.cpp:263-266).
//  ⚠ No vendor call of its own and no edit to EastSun's monitor: it uses the monitor's public API --
//    SetTorqueFocusAxis (Pci1203Monitor.cpp:4038; one 6077h SDO per Poll for the focus axis, expires
//    3 s after the last call), axis(i) / card() (the last completed Poll's copy; the torque fields are
//    valid for that Poll only, Pci1203Monitor.cpp:4100-4268). 2704h:1 / :2 come from the monitor's own
//    cached SDO read (gearVal[6] / [7], Pci1203Monitor.cpp:2565-2581) -- no new SDO read here.
//  ⚠ Tick thread only, like the limit hook (TPci1203Monitor is not thread-safe).
// =============================================================================
#include "MachineType.h"

#include <cstdio>
#include <string>

#include "vclcompat/vcl_compat.h"      // <windows.h>
#include "IndexZTorque1203.h"          // W906_Pci1203TorqueReadHook, ht9045::idxz::Sample
#include "EtherCAT/Pci1203Monitor.h"   // Pci1203Monitor() / Pci1203AxisSample / Pci1203CardSample
#include "WebMotorAccess.h"            // MotorAccessLiveBackend(): MOT index -> Mot_Table alias -> monitor slot (NB2 R55 rule)

namespace ht9045 {

namespace {

DWORD s_tid = 0;                       // the wb_serve tick loop (the route's owner thread)

void ReadThunk(int mi, idxz::Sample* out)
{
    if (out == 0) return;
    idxz::Sample& o = *out;
    o = idxz::Sample();
    if (::GetCurrentThreadId() != s_tid) {
        o.why = "torque read asked from a thread other than the tick loop -- not read (TPci1203Monitor is not thread-safe)";
        return;
    }
    IMotorAccessBackend& be = MotorAccessLiveBackend();
    std::string alias;
    if (!be.AliasOfMotIndex(mi, alias)) { o.why = "MOT[" + std::to_string(mi) + "]: no Mot_Table row"; return; }
    MotorAccessAxis a;
    if (!be.Resolve(alias, a) || !a.Is1203()) { o.why = alias + ": not a PCI1203 row"; return; }
    if (a.axis < 0) { o.why = alias + ": " + a.why; return; }
    TPci1203Monitor* m = Pci1203Monitor();
    if (m == 0) { o.why = "the 1203 monitor is not enabled"; return; }
    if (a.axis >= m->axisCount()) { o.why = alias + ": monitor slot " + std::to_string(a.axis) + " is out of range"; return; }
    m->SetTorqueFocusAxis(a.axis);     // keeps this axis's 6077h SDO read alive (the NEXT Poll reads it)
    const Pci1203CardSample& c = m->card();
    const Pci1203AxisSample& s = m->axis(a.axis);
    o.haveMonitor = true;
    o.slot        = a.axis;
    o.poll        = c.pollCount;
    o.focusOnAxis = (c.torqueFocusAxis == a.axis);
    o.torqueValid = s.valid && s.opened && s.torqueValid;
    o.src         = s.torqueSrc;
    o.raw         = s.torqueRaw;
    o.pctValid    = s.torquePctValid;
    o.num         = s.gearValid[6] ? s.gearVal[6] : 0ul;
    o.den         = s.gearValid[7] ? s.gearVal[7] : 0ul;
    o.servoOn     = (s.motionIO & 0x00004000ul) != 0;                                // AX_MOTION_IO_SVON (as WebMotorAccessLive.cpp:1242)
    o.alarm       = (s.motionIO & 0x00000002ul) != 0 || (s.state & 0xFFu) == 3u;    // AX_MOTION_IO_ALM or ERROR_STOP (as :1243)
    if (!o.focusOnAxis)       o.why = c.torqueFocusWhy;
    else if (!o.torqueValid)  o.why = c.torqueSdoWhy;
}

}  // namespace

bool W906_InstallPci1203TorqueReadHook(unsigned long ownerThreadId)
{
    s_tid = (DWORD)ownerThreadId;
    W906_Pci1203TorqueReadHook = &ReadThunk;
    std::printf("[torque-1203] Index Z1 torque READ hook installed (6077h via the 1203 monitor; values only while "
                "Gerneral.ini [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED=1)\n");
    std::fflush(stdout);
    return true;
}

}  // namespace ht9045
