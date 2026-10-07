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
bool W906_IndexZTorqueWaitTimedOut(int arm, bool waitingForValue);           // P5, once after kTorqueWaitMs (AI(W906-E042): PCI1203 Z1 SHIP only)

// ---------------------------------------------------------------------------------------------
//  AI(W906-E042) 20261004 (St01 / ST01-E): E-042 = Phase B, step B1 -- the call-site helpers the translated golden
//  state machines use (forms/fContact_AutoHeight.cpp), all NOT GOLDEN, all active ONLY on a live PCI1203 Index Z1
//  (W906_IndexZLive1203: a SHIP build and MOT[MTestZ1].CardType == "PCI1203"); everywhere else they return false
//  and golden runs untouched (RS-232 / Panasonic machines, SOFT_SIMULTE).  None of them ever blocks a stop.
// ---------------------------------------------------------------------------------------------
//  P7 health hook -- filled by EtherCAT/Pci1203TorqueRead.cpp (wb_serve) WITHOUT moving the 6077h focus; installed
//  together with the read hook (W906_InstallPci1203TorqueReadHook).  0 = no health source in this process.
extern void (*W906_Pci1203IndexZHealthHook)(int motIndex, ht9045::idxz::Health* out);
//  Test seam: the clock the P5 / P7 helpers use (0 = ::GetTickCount).
extern unsigned long (*W906_IndexZNowMsHook)();

bool W906_IndexZLive1203();                                                   // SHIP && MOT[MTestZ1].CardType=="PCI1203"
bool W906_IndexZDriveFault(AnsiString* why);                                  // P7 decision now (live 1203 only)
//  P7 call site, the first statement of every tick of a Z state machine: on a fault sends MOT[MTestZ1] "ST",
//  shows the message once per trip, returns true -> the caller takes golden case 536's exit.
bool W906_IndexZDriveFaultStop(const char* where);
//  P4 + P8 + P7 at a contact run's entry: true = refused (message shown, nothing commanded).
bool W906_IndexZRunAllowed(int contactMode, int arm, AnsiString* why);
bool W906_IndexZRunRefused(int contactMode, int arm, const char* where);
//  P5 call site, on golden's own wait line: true once after kTorqueWaitMs without a value -> "ST" + message sent,
//  the caller takes golden case 536's exit.  A new wait (a gap > kTickGapMs since the last call) starts a fresh 5 s.
bool W906_IndexZTorqueWaitStop(int arm, bool waitingForValue, const char* where);
//  P9 call site, on golden's step line: the step would start from a COMMANDED position at / below floorMm ->
//  "ST" + message, true -> golden case 536's exit.
bool W906_IndexZCommandFloorStop(int commandPos, double floorMm, const char* where);
void W906_IndexZHealthReset();                                                // test seam (P7 state + once-per-trip latch)

// ---------------------------------------------------------------------------------------------
//  AI(W906-E042) 20261005 (B3, St01): W-44 (Steven; ST01-M 1005 03:4x: socket press ONLY), NOT GOLDEN, live PCI1203
//  Index Z1 only (SHIP); false everywhere else = golden.  Never blocks a stop.
// ---------------------------------------------------------------------------------------------
//  Test seam: the shuttle position source (0 = MOT[m].ReadPos()).  which: 0 = MInShuttle1, 1 = MOutShuttle1.
extern bool (*W906_IndexZShuttlePosHook)(int which, long* pos);
bool W906_IndexZShuttlesAtHome(AnsiString* why);                              // the W-44 decision now (true on non-1203 rows); targets Prod.InSHT[0].iLeft / Prod.OutSHT[0].iRight (B3b)
//  Run entry (DoTestContactFunction case 1, step B4): true = refused (message shown, nothing commanded).
bool W906_IndexZShuttlesHomeRefused(const char* where);
//  Per tick on the socket-press state machines' switch line (Do_Z1 / Do_Z2_AutoGetHeight): a press task
//  (ht9045::idxz::W44PressTask) with a shuttle away from home -> "ST" Z1 and Z2 + message once, true -> golden case 536's exit.
bool W906_IndexZShuttleHomeStop(int task, const char* where);

// ---------------------------------------------------------------------------------------------
//  AI(W906-E042) 20261005 (B4, St01): place-back on HT9050 (Steven 1005 09:2x Q101: HT9050 has no Index Y; the In
//  shuttle returns to iRight before place-back).  NOT GOLDEN, live PCI1203 Index Z1 only (else returns 1 = golden).
//  Called every tick on golden DoTestContactFunction case 900's line before DoZPlaceToShuttle():
//    1 = In shuttle confirmed at InSHT[0].iRight +-tol -> golden's place-back runs;  0 = moving, come back next tick;
//   -1 = failed (alarm shown, Z1 "ST") -> the caller takes golden's exit.  Golden case 1700 then returns it to iLeft.
// ---------------------------------------------------------------------------------------------
extern bool (*W906_IndexZShuttleMoveHook)(int which, long target);              // test seam (0 = MOT[MInShuttle1].MotorMove)
int  W906_IndexZPlaceBackPrep(const char* where);
void W906_IndexZPlaceBackReset();                                               // a new contact run (case 1)
//  Contact modes refused at DoTestContactFunction case 1 on every machine (ht9045::idxz::ModeUntranslated): true =
//  refused, message shown, nothing commanded.  Mode 12 carries "TODO E-052: port V912 Task 60".
bool W906_ContactModeUntranslatedRefused(int contactMode, const char* where);
//  The contact-mode single entry (FileRW/DeviceForm_File.cpp EOF registers FileRW_Contact_SetContactModeSingleEntry here at
//  static init; that file is a wb_serve source, so ht9045_sm must not link to it directly).  0 = the C-route is not in this
//  program -> DoTestContactFunction case 1 falls back to golden's member SetContactMode().
extern void (*W906_ContactModeSingleEntryHook)(int rbForTest);

#endif  // IndexZTorque1203H
