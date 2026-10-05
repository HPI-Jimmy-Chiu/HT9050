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
#include <windows.h>
#include <cstdio>
#include <string>

void (*W906_Pci1203TorqueReadHook)(int motIndex, ht9045::idxz::Sample* out) = 0;

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
    return ht9045::idxz::TorqueWaitStep(w, waitingForValue, (unsigned long)::GetTickCount());
}
