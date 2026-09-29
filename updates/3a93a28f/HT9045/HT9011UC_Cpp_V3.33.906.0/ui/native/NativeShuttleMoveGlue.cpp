// ===========================================================================
//  ui/native/NativeShuttleMoveGlue.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): wb_serve glue for the native HW.ShuttleMove (v1 display only).  Only in
//  wb_serve and only with -DW906_NATIVE_FORMS=ON (ui/native/NativeForms.cmake).  NativeFormsWbServe.cpp calls the three
//  entry points at the bottom (open / close at start / stop, refresh every 20 ms).  Own file so the machine headers this
//  page needs (Tech, Prod, TestIF, IniConfig, cmydef) stay out of NativeFormsWbServe.cpp.
//
//  Sources (golden V912 ShuttleMove.cpp FormShow :67-141, ShowShuttleSensorPosition :1996-2090), memory reads only:
//    encoders       MOT[MInShuttle1/2]: the EastSun monitor overlay's encPos (the same hook as the web Motor Test,
//                   ht9045::sjson::W906_NativeMotorOverlay, JsonBridge/ChanMotorPoints.cpp:209) -- golden ReadEncoderPos
//                   reads the card, so it is NOT called; no overlay = "—"
//    teach points   Tech.iInSH1/2Sen7DetectPos, iInSH1/2BarCodePos, OutSH1/2ZOneRowDetectPos, OutSH1/2ZDetectPos (:100-107);
//                   latch: Tech.iInSH1/2SenICDetectPos, Prod.iInSH1/2SenICAddPos (:109-116) -- golden FormShow calls
//                   ReadData() (a file read) first; here the values are what is in memory now
//    trays          XItem = InArmSuck.iShtCol (FileRW_InArmSuckShtCol, FileRW/_KitSuck.cpp:138), the barcode tray x2 with a
//                   diagonal multi-2D (:2059-2064); YItem from the DFM (mtOutSHDetectPos 4, the others 2).  LAYOUT only: golden
//                   fills the numbers right after SetTechDataToProd() (a write) -- not done here
//    FormShow rules BAR_CODE_INSTALL, In_Shuttle_Auto_Latch, SOFT_SIMULTE, SHUTTLE_SENSOR_TYPE, ENABLE_OUT_SHUTTLEY_LATCH,
//                   TestIF_File multi-2D, IniConfig.bSPILFunction, TestIF.iShuttleMode / iShuttle_Sel (:72-98, :118, :140)
//  Nothing here writes, moves, reads a card or a file.  Not run: FormShow, ReadData, ShowShuttleSensorPosition,
//  SetTechDataToProd, sbtExitClick / FormClose (SystemStart=false, EtherCAT pause -- St02-E2 inventory §0.1).
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeShuttleMove.h"
#include "ui/native/NativeHost.h"

#include <cstring>
#include <string>
#include <vector>

#include "JsonBridge/ChanMotor.h"         // MotorRuntimeOverlay / MotorRuntimeOverlayFn
#include "EtherCAT/Pci1203Monitor.h"      // TPci1203Monitor (card().open, read only)
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"                  // SOFT_SIMULTE, ebctUninstall, eSensor*, eInSHAutoLtc
#include "cmydef.h"                       // MInShuttle1/2, BAR_CODE_INSTALL, SHUTTLE_SENSOR_TYPE, ENABLE_OUT_SHUTTLEY_LATCH, In_Shuttle_Auto_Latch
#include "LastSet.h"                      // Tech
#include "cprod.h"                        // Prod, TestIF, TestIF_File
#include "Config.h"                       // IniConfig
#include "Motor/mymotor.h"                // MOT[MAX_TRAY_MOTOR]

int FileRW_InArmSuckShtCol();   // FileRW/_KitSuck.cpp:138 (golden InArmSuck.iShtCol; mykitsuck.h clashes with HTEditList.h's TList)
namespace ht9045 { namespace sjson { MotorRuntimeOverlayFn W906_NativeMotorOverlay(); } }   // JsonBridge/ChanMotorPoints.cpp:209

namespace {

const DWORD kRefreshMs = 20;
DWORD g_last = 0;
w906native::ShuttleMoveState g_state;
// golden BarCode.h:83-91 eMulti2DType (the same three values FileRW/ShuttleMove.gen.inc copies)
const int kE1x2In1CCD = 0, kE2x2In1CCD = 3, kE2x2In2CCD = 4;

void Assign(std::string& dst, const char* src)
{
    if (!src) src = "";
    if (std::strcmp(dst.c_str(), src) != 0) dst = src;   // equal = no assignment, no allocation
}

void SetPoint(std::size_t k, const char* name, const char* group, const char* field, bool visible, bool locked, int value)
{
    if (g_state.points.size() <= k) g_state.points.resize(k + 1);
    w906native::ShuttlePoint& p = g_state.points[k];
    Assign(p.name, name);
    Assign(p.group, group);
    Assign(p.field, field);
    p.visible = visible;
    p.locked = locked;
    p.value = value;
}

void SetTray(std::size_t k, const char* name, int x, int y, bool visible)
{
    if (g_state.trays.size() <= k) g_state.trays.resize(k + 1);
    w906native::ShuttleTray& t = g_state.trays[k];
    Assign(t.name, name);
    t.xItem = x;
    t.yItem = y;
    t.visible = visible;
}

void Encoder(int k, int mi, ht9045::sjson::MotorRuntimeOverlayFn ovFn)
{
    w906native::ShuttleEncoder& e = g_state.enc[k];
    e.has = false;
    e.enc = 0;
    if (mi < 0 || mi >= MAX_TRAY_MOTOR) {
        Assign(e.alias, "");
        Assign(e.source, "none");
        Assign(e.why, "MInShuttle index out of range");
        return;
    }
    Assign(e.alias, MOT[mi].Alias.c_str());
    ht9045::sjson::MotorRuntimeOverlay ov;
    if (ovFn != 0 && ovFn(e.alias, ov) && ov.posKnown) {
        e.has = true;
        e.enc = ov.encPos;
        Assign(e.source, "pci1203-monitor");
        Assign(e.why, "");
    } else {
        Assign(e.source, "none");
        Assign(e.why, ovFn == 0 ? "no monitor overlay hooked (golden ReadEncoderPos reads the card: not called here)"
                                : "no monitor value for this axis (not a 1203 axis, no card, or the position is not given)");
    }
}

void Build()
{
    w906native::ShuttleMoveState& s = g_state;
#ifdef SOFT_SIMULTE
    Assign(s.buildConfig, "SIM 組態");
    const bool sim = true;
#else
    Assign(s.buildConfig, "SHIP 組態");
    const bool sim = false;
#endif
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    s.monitorOpen = mon && mon->card().open;
    s.keepaliveCalls = w906native::HostKeepaliveCalls();
    ht9045::sjson::MotorRuntimeOverlayFn ovFn = ht9045::sjson::W906_NativeMotorOverlay();
    Encoder(0, MInShuttle1, ovFn);
    Encoder(1, MInShuttle2, ovFn);

    // golden FormShow rules (V912 ShuttleMove.cpp)
    s.barCodeVisible = (BAR_CODE_INSTALL != ebctUninstall);                                                     // :88
    s.sensorVisible = (SHUTTLE_SENSOR_TYPE == eSensorCCLink || SHUTTLE_SENSOR_TYPE == eSensorCCLink3 ||
                       SHUTTLE_SENSOR_TYPE == eSensorCanBus || SHUTTLE_SENSOR_TYPE == eSensorCanBus3 ||
                       SHUTTLE_SENSOR_TYPE == eSensorEtherCAT || SHUTTLE_SENSOR_TYPE == eSensorEtherCAT3);     // :89-91
    s.retryVisible = sim;                                                                                        // :94-98
    s.latchVisible = (In_Shuttle_Auto_Latch == eInSHAutoLtc);                                                    // :109
    s.sensorLatchVisible = ENABLE_OUT_SHUTTLEY_LATCH;                                                            // :140
    const bool multi2D = TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType == kE1x2In1CCD ||
                                                        TestIF_File.iMulti2DType == kE2x2In1CCD ||
                                                        TestIF_File.iMulti2DType == kE2x2In2CCD);               // :2059-2062
    s.tStepVisible = sim && multi2D;                                                                             // :2074-2078
    s.spilLocked = IniConfig.bSPILFunction;                                                                      // :118
    s.oneSide = -1;                                                                                              // :77-86
    if (TestIF.iShuttleMode == 1 && TestIF.iShuttle_Sel == 0) s.oneSide = 0;
    else if (TestIF.iShuttleMode == 1 && TestIF.iShuttle_Sel == 1) s.oneSide = 1;

    // the teach edits (:100-116)
    const bool lk = s.spilLocked;
    std::size_t k = 0;
    SetPoint(k++, "edInSH1Sen7DetectPos", "In Fiber Check", "Tech.iInSH1Sen7DetectPos", true, lk, Tech.iInSH1Sen7DetectPos);
    SetPoint(k++, "edInSH2Sen7DetectPos", "In Fiber Check", "Tech.iInSH2Sen7DetectPos", true, lk, Tech.iInSH2Sen7DetectPos);
    SetPoint(k++, "edInSH1BarCodePos", "Move to Bar Code Pos", "Tech.iInSH1BarCodePos", s.barCodeVisible, lk, Tech.iInSH1BarCodePos);
    SetPoint(k++, "edInSH2BarCodePos", "Move to Bar Code Pos", "Tech.iInSH2BarCodePos", s.barCodeVisible, lk, Tech.iInSH2BarCodePos);
    SetPoint(k++, "edOutSH1OneRowDetectPos", "Out Fiber Check", "Tech.OutSH1ZOneRowDetectPos", true, lk, Tech.OutSH1ZOneRowDetectPos);
    SetPoint(k++, "edOutSH2OneRowDetectPos", "Out Fiber Check", "Tech.OutSH2ZOneRowDetectPos", true, lk, Tech.OutSH2ZOneRowDetectPos);
    SetPoint(k++, "edOutSH1ZDetectPos", "Out Fiber Check", "Tech.OutSH1ZDetectPos", true, lk, Tech.OutSH1ZDetectPos);
    SetPoint(k++, "edOutSH2ZDetectPos", "Out Fiber Check", "Tech.OutSH2ZDetectPos", true, lk, Tech.OutSH2ZDetectPos);
    const char* lg = "In Fiber Check - Shuttle sensor latch";
    const bool lv = s.latchVisible;
    SetPoint(k++, "edInSH1SenICDetectPos", lg, "Tech.iInSH1SenICDetectPos", lv, lk && lv, Tech.iInSH1SenICDetectPos);
    SetPoint(k++, "edInSH2SenICDetectPos", lg, "Tech.iInSH2SenICDetectPos", lv, lk && lv, Tech.iInSH2SenICDetectPos);
    SetPoint(k++, "edInSH1SenICDetectZPos", lg, "Prod.iInSH1SenICAddPos", lv, lk && lv, Prod.iInSH1SenICAddPos);
    SetPoint(k++, "edInSH2SenICDetectZPos", lg, "Prod.iInSH2SenICAddPos", lv, lk && lv, Prod.iInSH2SenICAddPos);
    g_state.points.resize(k);

    // the four TTMyTray grids: layout only (ShowShuttleSensorPosition :2001-2003, :2059-2082)
    const int col = FileRW_InArmSuckShtCol();
    std::size_t t = 0;
    SetTray(t++, "mtedInSHSen7DetectPos", col, 2, true);
    SetTray(t++, "mtInSHBarCodePos", multi2D ? col * 2 : col, 2, s.barCodeVisible);
    SetTray(t++, "mtOutSHDetectPos", col, 4, true);
    SetTray(t++, "mtedInSHSenICDetectPos", col, 2, s.latchVisible);
    g_state.trays.resize(t);

    Assign(s.why, s.enc[0].has || s.enc[1].has ? "" : "兩個編碼器都沒有值（見每一列說明）");
}

void Reopen() { if (w906native::ShuttleMoveOpen(true)) { g_last = 0; } }

}  // namespace

bool W906_NativeShuttleMoveOpen(bool show) { return w906native::ShuttleMoveOpen(show); }
void W906_NativeShuttleMoveClose() { w906native::ShuttleMoveClose(); }
void W906_NativeShuttleMoveReopen() { Reopen(); }

void W906_NativeShuttleMoveRefresh(bool force)
{
    if (!w906native::ShuttleMoveIsOpen()) return;
    const DWORD now = ::GetTickCount();
    if (!force && now - g_last < kRefreshMs) return;
    g_last = now;
    Build();
    w906native::ShuttleMoveUpdate(g_state);   // only changed rows are marked; nothing changed = 0 cells
}
