//==============================================================================
//  JsonBridge/W906DualCoil9050.cpp -- HT9050 double-solenoid (five-port three-position) cylinders
//
//AI(W906-DUALCOIL9050) 20261008: EastSun 1008 "ht9050 的汽缸都是用五口三位的，所以 Pop() 以前只需要對單 I/O 下命令 Off 即可，五口三位的
//  關係，需要把相對應的汽缸先關閉在打開，例如 Cylinder[C_LoaderEdgePush].Pop() 會等於 Cylinder[C_LoaderEdgePush].Off() &&
//  Cylinder[C_LoaderEdgePushOff].On()，Push() 則與 Pop 相反，On() 變成 Off()，Off() 變成 On()".
//
//  mycylin.cpp TMyCylinder::OnSwitch / OffSwitch (every On / Off / Push / Pop output write, only while the cylinder is Enable) call
//  g_W906PairedCoilHook(name, energise):
//    On / Push  -> "<name>Off" coil OFF, before the On coil is energised
//    Off / Pop  -> "<name>Off" coil ON,  after the On coil is released
//  Only on HT9050 (W906_GpibModel "9050GPIB" or Type_HT9050); only when the "<name>Off" IO_Table row exists, is IOType "Cylinder" and
//  Enable=1 (the machine table has 27 such pairs: C_LoaderEdgePush / C_LoaderEdgeClip / C_TrayZ_Selector / C_Load_Up / C_Empty_Up /
//  C_Auto1~3_Up / ...EdgePush / ...EdgeClip / ...DrawerLock / ...LoaderZ_Select ...); never when "<name>Off" is itself an engine
//  Cylinder[] (golden's C_DockXAxisOff style pairs drive their own coil). The Off coils have no engine object (IoBtnPanelClick.cpp
//  header DEVIATION c), so they are written straight from their IO_Table row, with the same OutType rule as io.btnPanelClick.
//  A local iterator, not HSys.mapIOTableIter: this runs inside the engine's cylinder calls and must not move an iterator a caller
//  may be holding. Other machines: golden (the hook returns at once).
//
//  Its own file (not IoBtnPanelClick.cpp, which only wb_serve compiles) so tests/test_dualcoil9050.cpp can drive it.
//==============================================================================
#include <cstdlib>
#include <map>
#include <string>

#include "cmydef.h"               // MachineTypeChoice
#include "MachineType.h"          // Type_HT9050, ePCI1203
#include "database.h"             // HSys.mapIOTable / HSys.IOTable / TIODATA, W906_GpibModel
#include "MyLaneIo.h"             // MyLaneIO
#include "mycylin.h"              // Cylinder[] / MaxCylinderItem

extern AnsiString W906_GpibModel;                                 // database.cpp
extern void (*g_W906PairedCoilHook)(const AnsiString&, bool);     // mycylin.cpp
extern bool (*g_W906CylNoReDelayHook)();                          // mycylin.cpp (AI(W906-CYLNOREDELAY) 20261008)

namespace {
std::map<std::string, bool> s_w906PairOwnedByEngine;              // "<name>Off" -> is an engine Cylinder[] (names are fixed after init)

void W906_PairedCoilHookFn(const AnsiString& name, bool energise)
{
    if (!(W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050)) return;
    if (name == "") return;
    const AnsiString off = name + "Off";
    std::map<AnsiString, AnsiString>::iterator it = HSys.mapIOTable.find(off);
    if (it == HSys.mapIOTable.end()) return;
    const int idx = std::atoi(it->second.c_str());
    if (idx < 0 || idx >= (int)HSys.IOTable.size() || HSys.IOTable[idx] == 0) return;
    const TIODATA* r = HSys.IOTable[idx];
    if (r->iEnable == 0 || r->Type != "Cylinder") return;
    const std::string key(off.c_str());
    std::map<std::string, bool>::iterator own = s_w906PairOwnedByEngine.find(key);
    if (own == s_w906PairOwnedByEngine.end()) {
        bool owned = false;
        for (int j = 0; j < MaxCylinderItem && !owned; ++j) if (Cylinder[j].CylinderName == off) owned = true;
        own = s_w906PairOwnedByEngine.insert(std::make_pair(key, owned)).first;
    }
    if (own->second) return;
    const bool on = energise ? (r->iInType == 1) : (r->iInType == 0);   // the same OutType rule as io.btnPanelClick / W906_UpCylWrite
    if (on) MyLaneIO.IOBitOn (r->iLane, r->iIP, r->iPort, r->iBit, r->iISABase, r->Alias);
    else    MyLaneIO.IOBitOff(r->iLane, r->iIP, r->iPort, r->iBit, r->iISABase, r->Alias);
}
// Installed at static init by whoever compiles this file in directly (wb_serve, tests/test_dualcoil9050.cpp -- an executable's own
// source, not an archive member, so the linker cannot drop it), same as IoBtnPanelClick.cpp's UPCYLOFF registrar.
//AI(W906-CYLNOREDELAY) 20261008: the same registrar arms mycylin.cpp's "no second on / off delay for a cylinder already confirmed
//  and still on its sensor" (EastSun 1008, B) -- HT9050 only, decided like the paired coil hook above.
bool W906_CylNoReDelay9050() { return W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050; }
struct W906DualCoilReg { W906DualCoilReg() { g_W906PairedCoilHook = &W906_PairedCoilHookFn; g_W906CylNoReDelayHook = &W906_CylNoReDelay9050; } } s_w906DualCoilReg;
}  // namespace
