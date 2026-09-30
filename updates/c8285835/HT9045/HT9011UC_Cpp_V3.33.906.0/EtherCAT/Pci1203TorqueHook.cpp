// =============================================================================
//  EtherCAT/Pci1203TorqueHook.cpp -- rs232.cpp's W906_Pci1203TorqueLimitHook for
//  HT9050's Index Z (D4 of RULINGS_20260929 section 5). wb_serve only.
//
//  AI(W906-INDEXZ-1203) 20260930: MOVED here, unchanged, from EtherCAT/Pci1203GaliRoute.cpp (review round 2 of
//  a8c3b04e, C / E: tests/test_gali_route_live.cpp links the route's binding and installer, and this body needs
//  MotorAccessLiveBackend(), which is WebMotorAccessLive.cpp's -- a wb_serve-only TU). The installer
//  (W906_InstallPci1203GaliRoute) still installs it, and only together with the route.
//  ⚠ No vendor call of its own (tools/pci1203_control_gate.ps1 check 5d): one kCmdAxTorqueLimitSet through
//  TPci1203Control::Execute.
// =============================================================================
#include "MachineType.h"
#include "EtherCAT/Pci1203GaliRoute.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "vclcompat/vcl_compat.h"      // AnsiString (rs232.cpp's torque-hook signature); <windows.h>
#include "EtherCAT/Pci1203Control.h"   // Pci1203Control()
#include "WebMotorAccess.h"            // MotorAccessLiveBackend(): the torque hook's alias resolution (NB2 R55)

extern int (*W906_Pci1203TorqueLimitHook)(int motIndex, int value01pct, AnsiString* why);   // rs232.cpp:2265

namespace ht9045 {

namespace {

bool  s_torque = false;
DWORD s_tid = 0;                       // the route's owner thread (the wb_serve tick loop)

// D4: rs232.cpp W906_Ht9050TorqueLimit's hook, as NB2 R55 specified it (docs/nb2_assist/README.md):
//   golden motor index -> Mot_Table Alias -> the monitor axis slot (the Motor Test rule) -> ONE
//   kCmdAxTorqueLimitSet (writes 60E0h + 60E1h, reads both back) -> 1 only when both read-backs match.
//   The caller retries up to 5 times and answers golden's 0 / 1 / 2 (rs232.cpp:2267-2291).
int TorqueThunk(int mi, int value01pct, AnsiString* why)
{
    char b[400];
    if (::GetCurrentThreadId() != s_tid) {
        if (why) *why = "torque limit asked from a thread other than the tick loop -- not sent (TPci1203Control is not thread-safe)";
        return 0;
    }
    IMotorAccessBackend& be = MotorAccessLiveBackend();
    std::string alias;
    if (!be.AliasOfMotIndex(mi, alias)) {
        std::snprintf(b, sizeof(b), "MOT[%d]: no Mot_Table row", mi);
        if (why) *why = b;
        return 0;
    }
    MotorAccessAxis a;
    if (!be.Resolve(alias, a) || !a.Is1203()) {
        if (why) *why = AnsiString((alias + ": not a PCI1203 row -- no 1203 drive to write").c_str());
        return 0;
    }
    if (a.axis < 0) {
        if (why) *why = AnsiString((alias + ": " + a.why).c_str());
        return 0;
    }
    TPci1203Control* ctl = Pci1203Control();
    if (ctl == 0) {
        if (why) *why = "1203 control is not armed";
        return 0;
    }
    Pci1203Cmd c;
    c.kind = kCmdAxTorqueLimitSet; c.axis = a.axis; c.value = (double)value01pct;
    const Pci1203CmdResult r = ctl->Execute(c);
    if (r.accepted && r.issued && r.ok && r.valueValid && std::lround(r.value) == (long)value01pct) return 1;
    std::snprintf(b, sizeof(b), "%s axis %d torque limit %d (0.1 %%): accepted=%d issued=%d ok=%d failStep=%d %s",
                  alias.c_str(), a.axis, value01pct, (int)r.accepted, (int)r.issued, (int)r.ok, r.failStep, r.why.c_str());
    if (why) *why = b;
    return 0;
}

}  // namespace

bool W906_InstallPci1203TorqueHook(unsigned long ownerThreadId)
{
    s_tid = (DWORD)ownerThreadId;
    W906_Pci1203TorqueLimitHook = &TorqueThunk;
    s_torque = true;
    return true;
}

bool Pci1203TorqueHookInstalled() { return s_torque && W906_Pci1203TorqueLimitHook == &TorqueThunk; }

}  // namespace ht9045
