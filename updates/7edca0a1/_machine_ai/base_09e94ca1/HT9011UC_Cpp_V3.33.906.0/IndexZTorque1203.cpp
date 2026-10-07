// =============================================================================
//  IndexZTorque1203.cpp -- E-038 (Q87, SAFETY). See IndexZTorque1203.h / IndexZTorqueCore.h.
//
//  AI(W906-E038) 20261003 (St01 / ST01-E): NEW FILE, NOT GOLDEN (golden has no HT9050 / 1203).
//  Where a line mirrors golden, the golden 0618 file:line is on it
//  (D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618).
//
//  Data path (ship builds, HT9050):
//    MainProc -> COM2->ReadTorque() (csystem.cpp:30399) -> TCOM2Shim::ReadTorque (rs232.cpp:356,
//    the PCI1203 branch at :358) -> W906_Ht9050TorqueRead() -> W906_Pci1203TorqueReadHook (live thunk,
//    EtherCAT/Pci1203TorqueRead.cpp) -> ht9045::idxz::BridgeStep -> fMain->edTorue0 /
//    fContactForm->PnlTorue0 / Torque[0], chkReadTorque1/2 cleared, autoTask=999 -- exactly where and
//    how golden's Panasonic reader leaves its value (golden rs232.cpp:975-976, :991-995, :1811-1813).
//  Readers of that value are golden's own: Auto Height (Phase B, not translated yet), production
//  DoTestHeadMotor 12110 (atester.cpp:6907-6935, live), DoZ1PickFromShuttle 560 (not translated).
//  ⚠ human-review A: once CONFIRMED = 1 the 1203 value also reaches DoTestHeadMotor 12110 on HT9050.
// =============================================================================
#include "atester_shims.h"          // COM2 (fPanasonicParameterRW, asReceiveTorue)
#include "MachineType.h"            // SOFT_SIMULTE
#include "cmydef.h"                 // INDEX_DRIVER_TYPE / Panasonic_DRIVER / Torque[] / MTestZ1
#include "common.h"                 // INIFileGeneral
#include "Motor/mymotor.h"          // MOT[] (CardType, Motor->Direction)
#include "canary_support.h"         // ShowMyMessage
#include "FormsFacade.h"            // fMain (chkReadTorque1/2, edTorue0)
#include "forms/fContact.h"         // fContactForm->PnlTorue0 (golden fContact->PnlTorue0; rs232.cpp header (6))
#include "IndexZTorque1203.h"
#include "cprod.h"                  // AI(W906-E042): TestIF / TestIF_File (P8)
#include "Config.h"                // AI(W906-E042): IniConfig (P8)
#include "LastSet.h"               // AI(W906-E042): LastSet.iRealDummy (P8)
#include "cpublic.h"               // AI(W906-E042): ConvertTouMType (P9, golden's own mm text)
#include <windows.h>
#include <cstdio>
#include <string>

void (*W906_Pci1203TorqueReadHook)(int motIndex, ht9045::idxz::Sample* out) = 0;
void (*W906_Pci1203IndexZHealthHook)(int motIndex, ht9045::idxz::Health* out) = 0;   // AI(W906-E042) P7
unsigned long (*W906_IndexZNowMsHook)() = 0;                                          // AI(W906-E042) test seam

namespace {
ht9045::idxz::BridgeState g_bridge;
ht9045::idxz::TorqueWait  g_wait[2];
bool                      g_flagsHave = false;
W906IndexZTorqueFlags     g_flags = { false, false, false, false, false };

const char* const kSec          = "IndexDriver";
const char* const kKeyConfirmed = "HT9050_INDEXZ_TORQUE_CONFIRMED";
const char* const kKeyBaseline  = "HT9050_INDEXZ_TORQUE_BASELINE";

bool Z1Is1203() { return MOT[MTestZ1].CardType == AnsiString("PCI1203"); }   // same test as rs232.cpp:950
bool Z1DirectionIsOne() { return MOT[MTestZ1].Motor == 0 || MOT[MTestZ1].Motor->Direction; }   // no motor object = not derived
// AI(W906-E042) 20261004: P5 / P7 state
ht9045::idxz::HealthState g_health;
bool                      g_faultShown = false;           // P7 message once per trip
bool                      g_waitHave[2] = { false, false };
unsigned long             g_waitLast[2] = { 0ul, 0ul };    // P5: last call per arm (a gap = a new wait)
unsigned long NowMs() { return W906_IndexZNowMsHook ? W906_IndexZNowMsHook() : (unsigned long)::GetTickCount(); }
}  // namespace

// ---------------------------------------------------------------------------------------------
//  The two switches. READ-ONLY by construction: ValueExists + ReadInteger on the write-through
//  TIniFile re-read the current file and write nothing (vclcompat/IniFiles.cpp:292-305). NOT
//  CheckAndReadIniDataGeneral (common.cpp:1643-1655: a missing key is WRITTEN back with the default)
//  and not LoadMachineConfig -- this key must never appear in a Gerneral.ini unless a person typed it.
//  Pinned by tests/test_indexz_torque_1203.cpp [T12] (file SHA-256 unchanged: key absent / 1 / 0).
// ---------------------------------------------------------------------------------------------
W906IndexZTorqueFlags W906_IndexZTorqueReadFlags()
{
    W906IndexZTorqueFlags f = { false, false, false, false, false };
    if (INIFileGeneral == 0) return f;
    f.iniOpen      = true;
    f.confirmedKey = INIFileGeneral->ValueExists(kSec, kKeyConfirmed);
    f.confirmed    = f.confirmedKey && INIFileGeneral->ReadInteger(kSec, kKeyConfirmed, 0) == 1;
    f.baselineKey  = INIFileGeneral->ValueExists(kSec, kKeyBaseline);
    f.baseline     = f.baselineKey && INIFileGeneral->ReadInteger(kSec, kKeyBaseline, 0) == 1;
    return f;
}

const W906IndexZTorqueFlags& W906_IndexZTorqueFlags()
{
    if (!g_flagsHave) {
        g_flags = W906_IndexZTorqueReadFlags();
        if (g_flags.iniOpen) {                        // cache only a real read; re-read next time otherwise
            g_flagsHave = true;
            std::printf("[torque-1203] Gerneral.ini [%s] %s=%s, %s=%s (read-only; missing = 0)\n", kSec,
                        kKeyConfirmed, g_flags.confirmedKey ? (g_flags.confirmed ? "1" : "not 1") : "missing",
                        kKeyBaseline,  g_flags.baselineKey  ? (g_flags.baseline  ? "1" : "not 1") : "missing");
            std::fflush(stdout);
        }
    }
    return g_flags;
}

void W906_IndexZTorqueFlagsReset() { g_flagsHave = false; g_flags = W906IndexZTorqueFlags(); }

void W906_IndexZTorqueBridgeReset() { g_bridge = ht9045::idxz::BridgeState(); g_wait[0] = g_wait[1] = ht9045::idxz::TorqueWait(); }
const ht9045::idxz::BridgeState& W906_IndexZTorqueBridgeState() { return g_bridge; }
std::string W906_IndexZTorqueLastWhy() { return g_bridge.lastWhy; }
void W906_IndexZTorqueBaselineReset() { ht9045::idxz::BaselineRequest(g_bridge); }

// ---------------------------------------------------------------------------------------------
//  golden TCOM2::ReadTorque_Panasonic (golden 0618 rs232.cpp:820-1000), the 1203 edition.
// ---------------------------------------------------------------------------------------------
int W906_Ht9050TorqueRead()
{
#ifdef SOFT_SIMULTE
    return -1;                                        // golden SOFT_SIMULTE: ReadTorque_Panasonic returns at once (golden :822-825); case 555 writes 30 itself (cContact.cpp:5769-5770)
#else
    if (!Z1Is1203()) return -1;                       // every RS-232 machine: golden dispatch, unchanged
    if (COM2->fPanasonicParameterRW) return 0;        // golden 0618 rs232.cpp:838-839
    ht9045::idxz::BridgeIn in;
    in.chk1 = fMain->chkReadTorque1->Checked;         // golden :841-853 (chkReadTorque1 has priority = Z1)
    in.chk2 = fMain->chkReadTorque2->Checked;
    const W906IndexZTorqueFlags& f = W906_IndexZTorqueFlags();
    in.confirmed       = f.confirmed;
    in.baselineOpt     = f.baseline;
    in.driverPanasonic = (INDEX_DRIVER_TYPE == Panasonic_DRIVER);
    in.pressSign       = ht9045::idxz::PressSign(Z1DirectionIsOne());
    in.hookPresent     = (W906_Pci1203TorqueReadHook != 0);
    in.nowMs           = (unsigned long)::GetTickCount();
    //  The hook (and with it the monitor's 6077h focus / SDO traffic) is used ONLY while Z1 is armed
    //  and every gate above is open -- an unconfirmed machine causes no mailbox traffic at all.
    if (in.chk1 && in.confirmed && in.driverPanasonic && in.pressSign != 0 && in.hookPresent) {
        W906_Pci1203TorqueReadHook(MTestZ1, &in.sample);
        in.sampleRead = true;
    }
    const ht9045::idxz::BridgeOut o = ht9045::idxz::BridgeStep(g_bridge, in);
    if (o.code == ht9045::idxz::kBridgeWrote) {
        const AnsiString t(o.text.c_str());
        fMain->edTorue0->Text = t;                    // golden 0618 rs232.cpp:975
        if (fContactForm) fContactForm->PnlTorue0->Caption = t;   // golden :976 (fContact->PnlTorue0)
        Torque[0] = o.value;                          // golden :1811 Torque[iReadTorueIndex], index 0 = Z1
        COM2->asReceiveTorue = t;                     // golden :1813
        fMain->chkReadTorque1->Checked = false;       // golden case 40, :993
        fMain->chkReadTorque2->Checked = false;       // golden :994
        std::printf("[torque-1203] Z1 raw %ld x %lu/%lu x sign %d%s = %.2f -> edTorue0 \"%s\" (src %d, poll %lu, slot %d)\n",
                    in.sample.raw, in.sample.num, in.sample.den, in.pressSign,
                    in.baselineOpt ? (" (baseline " + std::to_string(g_bridge.baseline) + ")").c_str() : "",
                    o.signedValue, o.text.c_str(), in.sample.src, in.sample.poll, in.sample.slot);
        std::fflush(stdout);
    }
    if (o.alarm) {                                    // golden 0618 rs232.cpp:861-871, the 1203 reason added
        const AnsiString z = in.chk1 ? "Z1" : "Z2";
        std::printf("[torque-1203] read error (%s): %s\n", z.c_str(), o.why.c_str());
        std::fflush(stdout);
        ShowMyMessage("1203 Read Index " + z + " Torque error!! " + AnsiString(o.why.c_str()),
                      "1203 讀取 Index " + z + " 扭力錯誤!! " + AnsiString(o.why.c_str()), "W906_Ht9050TorqueRead");
    }
    return o.code;
#endif
}

// ---------------------------------------------------------------------------------------------
//  P4 (NOT GOLDEN): may Auto Height / Manual Height / Load Cell start on this arm?
// ---------------------------------------------------------------------------------------------
bool W906_IndexZHeightAllowed(int contactMode, int arm, AnsiString* why)
{
    ht9045::idxz::HeightGateIn g;
    g.contactMode     = contactMode;
    g.arm             = arm;
    g.z1Is1203        = Z1Is1203();
    g.confirmed       = W906_IndexZTorqueFlags().confirmed;
    g.driverPanasonic = (INDEX_DRIVER_TYPE == Panasonic_DRIVER);
    g.directionIsOne  = Z1DirectionIsOne();
    g.hookInstalled   = (W906_Pci1203TorqueReadHook != 0);
    g.baselineOn      = W906_IndexZTorqueFlags().baseline;   // AI(W906-E042): Steven 1004 Q90 = A -- refused, never applied
    std::string w;
    const bool ok = ht9045::idxz::HeightAllowed(g, w);
    if (why) *why = AnsiString(w.c_str());
    return ok;
}

bool W906_IndexZHeightGate(int contactMode, int arm)
{
    AnsiString why;
    if (W906_IndexZHeightAllowed(contactMode, arm, &why)) return true;
    std::printf("[torque-1203] contact mode %d arm %d refused: %s\n", contactMode, arm, why.c_str());
    std::fflush(stdout);
    ShowMyMessage("Auto Height is not available: " + why, "自動測高目前不能用: " + why, "W906_IndexZHeightGate");
    return false;
}

// ---------------------------------------------------------------------------------------------
//  P5 (NOT GOLDEN): golden waits for edTorue0 forever (cContact.cpp:5766-5771, :6405-6410,
//  :17331-17336). Phase B calls this every tick of such a wait; true once after kTorqueWaitMs, and
//  the caller then stops Z (Gali_Command "ST") and leaves like golden case 536's failure exit.
// ---------------------------------------------------------------------------------------------
bool W906_IndexZTorqueWaitTimedOut(int arm, bool waitingForValue)
{
    ht9045::idxz::TorqueWait& w = g_wait[arm == 1 ? 1 : 0];
    if (!W906_IndexZLive1203()) { w = ht9045::idxz::TorqueWait(); return false; }   // AI(W906-E042) 20261004: RS-232 / Panasonic
                                                                                  // machines and SIM keep golden's wait for ever
    return ht9045::idxz::TorqueWaitStep(w, waitingForValue, NowMs());
}

// =============================================================================================
//  AI(W906-E042) 20261004 (St01 / ST01-E): E-042 = E-038 Phase B, step B1 -- the call-site helpers of the translated
//  golden state machines (forms/fContact_AutoHeight.cpp).  ALL NOT GOLDEN.  Active only on a live PCI1203 Index Z1;
//  every other machine (and SOFT_SIMULTE) gets `false` = golden runs untouched.  A stop is never gated: the "ST"
//  below is sent BEFORE anything else, whatever the switches say.
//  Plan: D:\AI_TempFile\st01e-e042-plan-20261004.md s3 (P4 / P5 / P7 / P8 / P9).
// =============================================================================================
bool W906_IndexZLive1203()
{
#ifdef SOFT_SIMULTE
    return false;                                    // golden SOFT_SIMULTE: no 1203 is driven (the route is not installed in a
#else                                                // SIM build, EtherCAT/Pci1203GaliRoute.cpp:201)
    return Z1Is1203();                               // same test as rs232.cpp:950
#endif
}

void W906_IndexZHealthReset()
{
    g_health = ht9045::idxz::HealthState();
    g_faultShown = false;
    g_waitHave[0] = g_waitHave[1] = false;
    g_waitLast[0] = g_waitLast[1] = 0;
    g_wait[0] = g_wait[1] = ht9045::idxz::TorqueWait();
}

//  P7 -- Steven 1004 07:5x 「io 馬達的裝置有異常error的時候，機台就不可以動」.
bool W906_IndexZDriveFault(AnsiString* why)
{
    if (why) *why = "";
    if (!W906_IndexZLive1203()) return false;
    ht9045::idxz::Health h;
    const bool hook = (W906_Pci1203IndexZHealthHook != 0);
    if (hook) W906_Pci1203IndexZHealthHook(MTestZ1, &h);
    std::string w;
    const bool fault = ht9045::idxz::HealthFault(g_health, h, hook, NowMs(), w);
    if (why) *why = AnsiString(w.c_str());
    return fault;
}

bool W906_IndexZDriveFaultStop(const char* where)
{
    AnsiString why;
    if (!W906_IndexZDriveFault(&why)) { g_faultShown = false; return false; }
    MOT[MTestZ1].Gali_Command("ST", "W906_IndexZDriveFaultStop");    // golden's own stop string (cContact.cpp:5795); never gated
    if (!g_faultShown) {
        g_faultShown = true;
        std::printf("[indexz-e042] P7 %s: Index Z1 drive error -> ST + golden exit: %s\n", where ? where : "", why.c_str());
        std::fflush(stdout);
        ShowMyMessage("Index Z1 drive error -- Auto Height stopped: " + why,
                      "Index Z1 驅動器異常 -- 自動測高已停止: " + why, where ? where : "W906_IndexZDriveFaultStop");
    }
    return true;
}

//  P4 + P8 + P7 at a contact run's entry.
bool W906_IndexZRunAllowed(int contactMode, int arm, AnsiString* why)
{
    if (why) *why = "";
    if (!W906_IndexZLive1203()) return true;                             // golden, unchanged
    ht9045::idxz::RunGateIn g;
    g.h.contactMode     = contactMode;
    g.h.arm             = arm;
    g.h.z1Is1203        = true;
    g.h.confirmed       = W906_IndexZTorqueFlags().confirmed;
    g.h.baselineOn      = W906_IndexZTorqueFlags().baseline;
    g.h.driverPanasonic = (INDEX_DRIVER_TYPE == Panasonic_DRIVER);
    g.h.directionIsOne  = Z1DirectionIsOne();
    g.h.hookInstalled   = (W906_Pci1203TorqueReadHook != 0);
    g.shuttleModeSingle = (TestIF.iShuttleMode == 1);                    // golden reads TestIF (cContact.cpp:12716)
    g.shuttleSel        = TestIF.iShuttle_Sel;
    g.arm2Options       = IniConfig.bIndexArm2SupplyLight || TestIF_File.bForEgisTecTest ||
                          (IniConfig.bD58UseArm1PickPlaceArm2Test && TestIF_File.bArm1PickPlaceArm2Test);
    g.twoArm32Site      = bUseTwoArm32Site;
    g.rtcLearning       = REAL_TIME_CCD && !COM2->bCCDDummyRum && fContactForm && fContactForm->cbRTCModel->Checked;
    g.calibrateAbove    = fContactForm && fContactForm->cbCalibrateAboveHeight->Checked;
    g.latchTeach        = LastSet.iRealDummy == REALLY && fContactForm && fContactForm->cbTeachInSHSen->Checked &&
                          In_Shuttle_Auto_Latch == eInSHAutoLtc;
    g.teachInOutArmZ    = (CUSTOMER_CODE == CC_ASE_KaohSiung || CUSTOMER_CODE == CC_ASE_CL) &&
                          fContactForm && fContactForm->chkTeachInOutArmZ->Checked;
    std::string w;
    if (!ht9045::idxz::RunAllowed(g, w)) { if (why) *why = AnsiString(w.c_str()); return false; }
    AnsiString hw;
    if (W906_IndexZDriveFault(&hw)) { if (why) *why = "drive error: " + hw; return false; }
    return true;
}

bool W906_IndexZRunRefused(int contactMode, int arm, const char* where)
{
    AnsiString why;
    if (W906_IndexZRunAllowed(contactMode, arm, &why)) return false;
    std::printf("[indexz-e042] P4/P8 %s: contact mode %d arm %d refused: %s\n", where ? where : "", contactMode, arm, why.c_str());
    std::fflush(stdout);
    ShowMyMessage("Auto Height is not available: " + why, "自動測高目前不能用: " + why, where ? where : "W906_IndexZRunRefused");
    return true;
}

//  P5 -- golden waits for edTorue0 with no timeout (cContact.cpp:5767-5768 / :6410).
bool W906_IndexZTorqueWaitStop(int arm, bool waitingForValue, const char* where)
{
    if (!W906_IndexZLive1203()) return false;
    const int k = (arm == 1) ? 1 : 0;
    const unsigned long now = NowMs();
    if (g_waitHave[k] && (unsigned long)(now - g_waitLast[k]) > ht9045::idxz::kTickGapMs)
        g_wait[k] = ht9045::idxz::TorqueWait();                          // the last call belonged to another wait
    g_waitHave[k] = true;
    g_waitLast[k] = now;
    if (!ht9045::idxz::TorqueWaitStep(g_wait[k], waitingForValue, now)) return false;
    MOT[k == 1 ? MTestZ2 : MTestZ1].Gali_Command("ST", "W906_IndexZTorqueWaitStop");   // golden's own stop string; never gated
    const AnsiString why(W906_IndexZTorqueLastWhy().c_str());
    std::printf("[indexz-e042] P5 %s: no torque value for %lu ms -> ST + golden exit (%s)\n", where ? where : "",
                ht9045::idxz::kTorqueWaitMs, why.c_str());
    std::fflush(stdout);
    ShowMyMessage("Index Z" + AnsiString(k + 1) + " torque value did not arrive in 5 s -- Auto Height stopped: " + why,
                  "Index Z" + AnsiString(k + 1) + " 5 秒內讀不到扭力值 -- 自動測高已停止: " + why, where ? where : "W906_IndexZTorqueWaitStop");
    return true;
}

//  W-44 -- AI(W906-E042) 20261005 (B3, St01): Index Z1 presses the SOCKET only with the In AND Out shuttles at home.
bool (*W906_IndexZShuttlePosHook)(int which, long* pos) = 0;
static bool g_w44Shown = false;
bool W906_IndexZShuttlesAtHome(AnsiString* why)
{
    if (why) *why = "";
    if (!W906_IndexZLive1203()) return true;                             // golden, unchanged
    ht9045::idxz::ShuttleHomeIn s;
    s.inTarget  = Prod.InSHT[0].iLeft;                                   // AI(W906-E042) B3b: golden Do_Z1 case 151's targets (:5577 / :5586)
    s.outTarget = Prod.OutSHT[0].iRight;
    for (int k = 0; k < 2; ++k) {
        long p = 0;
        bool have = false;
        if (W906_IndexZShuttlePosHook) have = W906_IndexZShuttlePosHook(k, &p);
        else { p = MOT[k == 0 ? MInShuttle1 : MOutShuttle1].ReadPos(); have = true; }
        if (k == 0) { s.inPresent = have; s.inPos = p; } else { s.outPresent = have; s.outPos = p; }
    }
    std::string w;
    const bool ok = ht9045::idxz::ShuttlesAtHome(s, w);
    if (why) *why = AnsiString(w.c_str());
    return ok;
}
bool W906_IndexZShuttlesHomeRefused(const char* where)
{
    AnsiString why;
    if (W906_IndexZShuttlesAtHome(&why)) return false;
    std::printf("[indexz-e042] W-44 %s: contact run refused: %s\n", where ? where : "", why.c_str());
    std::fflush(stdout);
    ShowMyMessage("Contact run is not available: " + why, "接觸流程不能開始: " + why, where ? where : "W906_IndexZShuttlesHomeRefused");
    return true;
}
bool W906_IndexZShuttleHomeStop(int task, const char* where)
{
    if (!W906_IndexZLive1203()) return false;
    if (!ht9045::idxz::W44PressTask(task)) { g_w44Shown = false; return false; }   // a pre-descent tick = a new run: the next trip shows its message again
    AnsiString why;
    if (W906_IndexZShuttlesAtHome(&why)) { g_w44Shown = false; return false; }
    MOT[MTestZ1].Gali_Command("ST", "W906_IndexZShuttleHomeStop");      // golden's own stop string; never gated
    MOT[MTestZ2].Gali_Command("ST", "W906_IndexZShuttleHomeStop");      // (Z2 absent on HT9050: done at once, NB2-1 !169)
    if (!g_w44Shown) {
        g_w44Shown = true;
        std::printf("[indexz-e042] W-44 %s task %d: %s -> ST + golden exit\n", where ? where : "", task, why.c_str());
        std::fflush(stdout);
        ShowMyMessage("Index Z press stopped: " + why, "Index Z 下壓已停止: " + why, where ? where : "W906_IndexZShuttleHomeStop");
    }
    return true;
}

//  Place-back -- AI(W906-E042) 20261005 (B4, St01): Steven 1005 09:2x Q101.
bool (*W906_IndexZShuttleMoveHook)(int which, long target) = 0;
static int  g_pbPhase = 0;                    // 0 idle, 1 moving the In shuttle to iRight, 2 confirmed there
static bool g_pbShown = false;
void W906_IndexZPlaceBackReset() { g_pbPhase = 0; g_pbShown = false; }
static bool ShuttlePos(int which, long& pos)
{
    if (W906_IndexZShuttlePosHook) return W906_IndexZShuttlePosHook(which, &pos);
    pos = MOT[which == 0 ? MInShuttle1 : MOutShuttle1].ReadPos();
    return true;
}
static int PlaceBackFail(const char* where, const std::string& why)
{
    MOT[MTestZ1].Gali_Command("ST", "W906_IndexZPlaceBackPrep");        // golden's own stop string; never gated
    g_pbPhase = 0;
    if (!g_pbShown) {
        g_pbShown = true;
        std::printf("[indexz-e042] place-back %s: %s -> ST + golden exit, the In shuttle is NOT moved\n", where ? where : "", why.c_str());
        std::fflush(stdout);
        ShowMyMessage("Index place-back stopped: " + AnsiString(why.c_str()) + " (Steven 1005 09:2x Q101: HT9050 has no Index Y; the In shuttle returns to iRight before place-back)",
                      "Index 放回已停止: " + AnsiString(why.c_str()), where ? where : "W906_IndexZPlaceBackPrep");
    }
    return -1;
}
int W906_IndexZPlaceBackPrep(const char* where)
{
    if (!W906_IndexZLive1203()) return 1;                                // golden: Index Y carries the IC back
    const long inTarget = Prod.InSHT[0].iRight;
    long inPos = 0;
    if (g_pbPhase == 2) {                                                // golden's place-back running: the shuttle must stay
        if (!ShuttlePos(0, inPos)) return PlaceBackFail(where, "the In shuttle (M11) position cannot be read");
        if (!ht9045::idxz::AtTarget(inPos, inTarget))
            return PlaceBackFail(where, "the In shuttle (M11) left InSHT[0].iRight during the place-back (at " + std::to_string(inPos) + ", target " + std::to_string(inTarget) + ")");
        return 1;
    }
    ht9045::idxz::PlaceBackIn p;
    p.z1Pos = MOT[MTestZ1].Gali_ReadPos();
    p.z1Safe = Prod.TestZ1_Safe;
    p.outTarget = Prod.OutSHT[0].iRight;
    p.outPresent = ShuttlePos(1, p.outPos);
    std::string why;
    if (!ht9045::idxz::PlaceBackPreconditions(p, why)) return PlaceBackFail(where, why);
    if (!ShuttlePos(0, inPos)) return PlaceBackFail(where, "the In shuttle (M11) position cannot be read");
    g_pbPhase = 1;
    const bool done = W906_IndexZShuttleMoveHook ? W906_IndexZShuttleMoveHook(0, inTarget) : MOT[MInShuttle1].MotorMove((int)inTarget);
    if (!done) return 0;
    if (!ShuttlePos(0, inPos) || !ht9045::idxz::AtTarget(inPos, inTarget))
        return PlaceBackFail(where, "the In shuttle (M11) did not arrive at InSHT[0].iRight (at " + std::to_string(inPos) + ", target " + std::to_string(inTarget) + ")");
    g_pbPhase = 2;
    g_pbShown = false;
    return 1;
}
void (*W906_ContactModeSingleEntryHook)(int rbForTest) = 0;          // AI(W906-E042) B4: set by FileRW/DeviceForm_File.cpp EOF
bool W906_ContactModeUntranslatedRefused(int contactMode, const char* where)
{
    if (!ht9045::idxz::ModeUntranslated(contactMode)) return false;
    AnsiString why = "contact mode " + AnsiString(contactMode) + " is not translated in the port yet (E-042: its golden state machine is not ported)";
    if (contactMode == 12) why = "contact mode 12 VISUAL_DETECTION_TEST has no arm in golden 0618 (V912 only) -- TODO E-052: port V912 Task 60";
    std::printf("[indexz-e042] %s: %s\n", where ? where : "", why.c_str());
    std::fflush(stdout);
    ShowMyMessage("Contact run is not available: " + why, "接觸流程不能開始: " + why, where ? where : "W906_ContactModeUntranslatedRefused");
    return true;
}

//  P9 -- golden's floor is the ENCODER (cContact.cpp:5790); this one is the COMMAND the next step starts from.
bool W906_IndexZCommandFloorStop(int commandPos, double floorMm, const char* where)
{
    if (!W906_IndexZLive1203()) return false;
    const double mm = std::atof(ConvertTouMType(commandPos));            // golden's own counts -> mm text (cpublic.h:20)
    if (!ht9045::idxz::CommandAtOrBelowFloor(mm, floorMm)) return false;
    MOT[MTestZ1].Gali_Command("ST", "W906_IndexZCommandFloorStop");     // never gated
    char b[160];
    std::snprintf(b, sizeof(b), "commanded Z1 %.2f mm is at / below the floor %.2f mm and the torque never reached kg", mm, floorMm);
    std::printf("[indexz-e042] P9 %s: %s -> ST + golden exit\n", where ? where : "", b);
    std::fflush(stdout);
    ShowMyMessage("Index Z1 pressed to the floor without reaching the torque -- Auto Height stopped: " + AnsiString(b),
                  "Index Z1 已到下限但扭力未達設定值 -- 自動測高已停止: " + AnsiString(b), where ? where : "W906_IndexZCommandFloorStop");
    return true;
}
