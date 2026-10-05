// =============================================================================
//  IndexZTorque1203.h -- E-038 (Q87, SAFETY): golden TCOM2's torque READ, fed by the PCI-1203
//  on HT9050 (MOT[MTestZ1].CardType == "PCI1203"). ht9045_sm (the archive rs232.cpp is in).
//
//  AI(W906-E038) 20261003 (St01 / ST01-E): NEW FILE, NOT GOLDEN. The pure rules are in
//  IndexZTorqueCore.h; this file holds the parts that touch the tree's globals:
//    * W906_Ht9050TorqueRead()  -- called from TCOM2Shim::ReadTorque (rs232.cpp:358, same line);
//    * the hook pointer the wb_serve-only live thunk fills (EtherCAT/Pci1203TorqueRead.cpp);
//    * the two Gerneral.ini switches, READ-ONLY (never CheckAndReadIniDataGeneral, which writes a
//      missing key back -- common.cpp:1643-1655 -- into the shared Gerneral.ini);
//    * Phase B's entry gate / torque-wait timeout / baseline reset (no caller yet: the golden
//      Auto Height state machines are still declarations, forms/fContact.h:1348-1349 / :1507-1508 /
//      :1526, and MainProc's call is #if 0, csystem.cpp:31320-31322).
// =============================================================================
#ifndef IndexZTorque1203H
#define IndexZTorque1203H

#include "IndexZTorqueCore.h"
#include "vclcompat/vcl_compat.h"   // AnsiString

//  Filled by EtherCAT/Pci1203TorqueRead.cpp (wb_serve), installed by W906_InstallPci1203TorqueHook
//  (EtherCAT/Pci1203TorqueHook.cpp:79) together with the torque-LIMIT hook, i.e. only when the
//  Index Z Galil route installs. 0 = no 1203 torque source in this process.
extern void (*W906_Pci1203TorqueReadHook)(int motIndex, ht9045::idxz::Sample* out);

//  Gerneral.ini [IndexDriver] -- port-only keys, missing = 0, read through INIFileGeneral's read-only
//  calls (ValueExists + ReadInteger). Code never writes them; EastSun sets them by hand.
struct W906IndexZTorqueFlags {
    bool iniOpen;        // INIFileGeneral existed when read
    bool confirmedKey;   // HT9050_INDEXZ_TORQUE_CONFIRMED present
    bool confirmed;      // ... and == 1
    bool baselineKey;    // HT9050_INDEXZ_TORQUE_BASELINE present
    bool baseline;       // ... and == 1  (port option, default off = golden)
};
W906IndexZTorqueFlags        W906_IndexZTorqueReadFlags();   // reads now (no cache)
const W906IndexZTorqueFlags& W906_IndexZTorqueFlags();       // cached after the first read with INIFileGeneral open
void                         W906_IndexZTorqueFlagsReset();  // test seam: forget the cache

//  -1 = not this machine (SOFT_SIMULTE, or MTestZ1 is not a PCI1203 row) -> golden dispatch runs;
//   0 = armed, no value this tick;  1 = value written (caller sets autoTask=999, golden case 40);
//   2 = not armed (caller sets autoTask=1, golden rs232.cpp:854-858).
int  W906_Ht9050TorqueRead();
void W906_IndexZTorqueBridgeReset();                         // test seam
const ht9045::idxz::BridgeState& W906_IndexZTorqueBridgeState();
std::string W906_IndexZTorqueLastWhy();                      // why the last armed tick had no value

//  Phase B call sites (NOT GOLDEN):
void W906_IndexZTorqueBaselineReset();                       // case 536 success, only matters with the baseline option
bool W906_IndexZHeightAllowed(int contactMode, int arm, AnsiString* why);   // P4 predicate
bool W906_IndexZHeightGate(int contactMode, int arm);        // P4 + the visible message when refused
bool W906_IndexZTorqueWaitTimedOut(int arm, bool waitingForValue);           // P5, once after kTorqueWaitMs

#endif  // IndexZTorque1203H
