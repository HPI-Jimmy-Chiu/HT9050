// =============================================================================
//  VacuumUnit/Vc8Route.h -- golden's ECAT-VC8 vacuum-unit calls, routed onto the PCIE-1203.
//
//  AI(W906-VACUNIT-1203) 20260930: new file. EastSun 20260930:
//      「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」
//      「針對1203 開啟 vacuunit 時，如果不符合現在程式碼就新創一個1203分支」
//  ("分支" = a code path, not a git branch.) Same shape as Motor/EcatMotorRoute.h.
//
//  WHY IT EXISTS
//      golden reaches the VC8 with six vendor calls on the global uiDevhand:
//        MyLaneIo.cpp  GetIOValue        Acm_DaqDiGetByteEx  x2  (pressure, DI bytes VC*2 / VC*2+1)
//                      GetIOValueThread  Acm_DevReadSDOData      (threshold 8000h+VC*10h:13h, I16)
//                      SetIOValueThread  Acm_DevWriteSDOData     (threshold, same object)
//        MyVacuumPanel RefreshDOIO       Acm_DaqDoGetBitEx   x2  (vacuum / blow DO read-back, chan 16..31)
//                      ReadVaccumIO      Acm_DaqDiGetBitEx       (vacuum-OK DI, chan 128+VC)
//                      btnVaccumOnOff..  Acm_DaqDoSetBitEx       (vacuum / blow DO)
//                      InitialThreshold. Acm_DevWriteSDOData     (threshold MODE 8000h+VC*10h:02h = 1)
//      ht9045_io / ht9045_sm are built WITHOUT HAVE_PCI1203 (CMakeLists.txt: only wb_serve carries the
//      flag), and wb_serve never opens the card through uiDevhand (TPci1203Monitor owns the handle, OWNED
//      mode) -- so every one of those calls takes its `#else` arm: 999.0 / 999.0 / false / Error3 / Error5.
//      The `#else` arms now ask THIS route; wb_serve installs one (EtherCAT/Pci1203Vc8Route.inc, inside
//      W906_InstallPci1203IoRoute):
//        reads  = the 1203 monitor's DI / DO samples; the threshold by a rate-limited monitor SDO read
//        writes = TPci1203Control::Execute (EastSun's layer: allowlist, DRY / LIVE, audit log)
//      and EVERY entry first proves the station is an ECAT-VC8 (EtherCAT/Pci1203Vc8.h): on this machine
//      golden's IndexArm2 stations 0x50 / 0x51 are ECx-P32 DI modules in OP.
//
//  THE CONTRACT THAT KEEPS THE TEST BASELINE
//      * No route installed (every ctest executable, every SOFT_SIMULTE build, a wb_serve without the
//        monitor or the control) = each `#else` arm behaves bit for bit as before: the wrappers below
//        return kVc8RcNoRoute with no side effect, and the arms turn a non-zero return into golden's own
//        failure value (999.0 / false / sEvent "Error3" / "Error5").
//      * Plain POD + function pointers; no EtherCAT header, no machine model -- ht9045_io and ht9045_sm
//        gain no dependency. The entries take the very numbers golden hands to Acm_* (ring, IP, channel,
//        object index / subindex, raw I16) and convert nothing.
//      * The inline functions do NOT depend on HAVE_PCI1203 (ht9045_pci1203_probe compiles MyLaneIo.cpp /
//        MyVacuumPanel.cpp WITH the flag, where only the golden arm is compiled; one inline function with
//        two bodies across TUs would be an ODR violation that links silently -- Motor/EcatMotorRoute.h).
//      * One route pointer program-wide: an inline function's static (C++98 rule), shared by ht9045_io,
//        ht9045_sm and wb_serve's route TU. The only production writer is W906_InstallVc8Route_
//        (EtherCAT/Pci1203Vc8Route.inc); tools/pci1203_control_gate.ps1 check 6 counts SetVc8Route( /
//        W906_Vc8RouteSlot_( outside tests\.
// =============================================================================
#ifndef VACUUMUNIT_VC8ROUTE_H
#define VACUUMUNIT_VC8ROUTE_H

#include <cstdio>

// Route return codes (0 = SUCCESS, what golden compares against). 0x7E prefix = not an Advantech range, the
// same family as EtherCAT/Pci1203IoRoute.cpp (0x7E00000x) and Motor/EcatMotorRoute.h (0x7E0001xx).
enum {
    kVc8RcNoRoute   = 0x7E000201,   // no route installed (the default everywhere but an armed wb_serve)
    kVc8RcNoCard    = 0x7E000202,   // monitor absent / card not open / monitor Disabled
    kVc8RcNoControl = 0x7E000203,   // a write, and there is no armed TPci1203Control
    kVc8RcBadAddr   = 0x7E000204,   // ring / channel / object outside the VC8's closed set
    kVc8RcNotVc8    = 0x7E000205,   // the station is absent, ambiguous, not addressable, a drive, not in the
                                    //   VC8 identity table, or not in the EtherCAT state the call needs
    kVc8RcNoByte    = 0x7E000206,   // the byte is not in the card's DI / DO map for that station
    kVc8RcNotRead   = 0x7E000207,   // the byte did not read back (valid == false) -- no blind write
    kVc8RcRefused   = 0x7E000208,   // a machine precondition (VacuUnitType, SystemStart, the vacuum/blow
                                    //   interlock) or Execute refused it, or a DRY RUN (nothing issued)
    kVc8RcRateLimit = 0x7E000209,   // SDO read deferred by the monitor's rate limit / failure backoff
    kVc8RcBadValue  = 0x7E00020A,   // value outside the object's range
    kVc8RcVc4SdoOff = 0x7E00020B    // AI(W906-VC4) 20261001: an SDO WRITE to an ECAT-VC4, refused because
                                    //   EtherCAT/Pci1203Vc8.h W906_VC4_SDO_WRITE is not defined (VC4 object dictionary
                                    //   unverified). A policy refusal, not a module failure: nothing was sent.
};

// ---------------------------------------------------------------------------
//  AI(W906-VC4) 20261001: THE VACUUM UNIT'S MODEL, AND THE VC4'S CHANNELS.
//  EastSun 1001 measured 「開真空是16bit 破真空17bit / 開真空是18bit 破真空19bit / 開真空是20bit 破真空21bit /
//  開真空是22bit 破真空23bit」 on the ECAT-VC4 at ring 1 160 / 161 / 162 -- VC n makes vacuum on DO 16+2n (EVEN)
//  and breaks it on 17+2n (ODD), the opposite of the VC8 -- and ruled 「VC8 和 VC4 要不同分支」.
//  The authoritative per-model tables are EtherCAT/Pci1203Vc8.h (Pci1203VacLayoutOf); this header cannot include it
//  (no EtherCAT header in ht9045_io / ht9045_sm), so the three VC4 numbers the PANEL side needs are repeated here and
//  tests/test_vacuum_vc8.cpp asserts they equal that table. The VC8 needs none here: golden's own arithmetic stands.
// ---------------------------------------------------------------------------
enum { kVc8ModelNone = 0, kVc8ModelVc4 = 4, kVc8ModelVc8 = 8 };    // = ht9045::kPci1203Vac* (the VC count)
enum { kVc4Units = 4, kVc4DoFirst = 16, kVc4DoLast = 23, kVc4DiOkChan = 64 };
//  AI(W906-VC4-ODD) 20261001: make / break swapped -- EastSun 21:3x on the Vacuum Unit 「箭頭往下的在吸」: v (bplOff, then
//  DO 19 on 0xA0 VC1) SUCKS, so the VC4 makes vacuum on the ODD channel like the VC8 (the 16-even reading above was reversed).
inline int W906_Vc4MakeChan(int vc)  { return kVc4DoFirst + 2 * vc + 1; }   // 吸真空 (make): 17 / 19 / 21 / 23
inline int W906_Vc4BreakChan(int vc) { return kVc4DoFirst + 2 * vc; }       // 破真空 (break): 16 / 18 / 20 / 22
inline int W906_Vc4OkChan(int vc)    { return kVc4DiOkChan + vc; }          // 真空 OK DI: 64..67

// One write that went through the route, for the page's ack (golden's WriteVaccumThreshold / the DO click
// return void or discard `ret`, so the answer has nowhere else to go). Kept in a small ring by the route.
struct TVc8Write {
    unsigned long seq;          // 1.. ; 0 = empty slot
    int           kind;         // 0 = DO bit (Acm_DaqDoSetBitEx), 1 = SDO I16 (Acm_DevWriteSDOData)
    int           ring, ip;     // as ASKED (golden OnRing / OnIP / SenRing / SenIP)
    int           chan;         // DO channel in the station, or the SDO index
    int           sub;          // SDO subindex, -1 for a DO write
    int           value;        // 0/1, or the raw I16
    int           rc;           // what the caller got back (0 = SUCCESS)
    bool          reached;      // got as far as TPci1203Control::Execute
    bool          issued;       // a vendor call was made (false in DRY RUN and on every refusal)
    bool          dryRun;
    unsigned long ret;          // vendor return when issued
    char          why[240];     // refusal / vendor error / "dry run: ..." ; "" on a clean SUCCESS
    char          call[120];    // the vendor call LIVE makes for it, spelled out
};

// The route. Every entry is keyed by golden's own address, never by a monitor slot (Rescan renumbers them).
struct TVc8Route {
    int  (*readAi)(int ring, int ip, int port, unsigned char* v);        // Acm_DaqDiGetByteEx  (golden Port*2, Port*2+1)
    int  (*readDi)(int ring, int ip, int chan, unsigned char* v);        // Acm_DaqDiGetBitEx   (golden 128+VC)
    int  (*readDo)(int ring, int ip, int chan, unsigned char* v);        // Acm_DaqDoGetBitEx   (golden 16+VC*2+swap)
    int  (*writeDo)(int ring, int ip, int chan, unsigned char v);        // Acm_DaqDoSetBitEx
    int  (*readSdoI16)(int ring, int ip, int index, int sub, short* v);  // Acm_DevReadSDOData  I16
    int  (*writeSdoI16)(int ring, int ip, int index, int sub, short v);  // Acm_DevWriteSDOData I16
    int  (*check)(int ring, int ip, int forWrite, char* why, int whyLen);   // everything a read (0) / write (1) of
                                                                            //   that station would check; no vendor call
    const char*   (*lastWhy)();                                         // reason of the last non-zero return
    unsigned long (*lastSeq)();                                         // seq of the newest TVc8Write
    int           (*writesSince)(unsigned long afterSeq, TVc8Write* out, int max);   // oldest first
    int           (*model)(int ring, int ip);                           // AI(W906-VC4) 20261001: kVc8ModelVc8 / kVc8ModelVc4
                                                                        //   by the station's identity (no state, no vendor
                                                                        //   call), kVc8ModelNone for anything refused
};

inline const TVc8Route*& W906_Vc8RouteSlot_() { static const TVc8Route* s = 0; return s; }
inline void SetVc8Route(const TVc8Route* r) { W906_Vc8RouteSlot_() = r; }   // 0 = back to golden's failure arms
inline const TVc8Route* Vc8Route() { return W906_Vc8RouteSlot_(); }

inline int W906_Vc8ReadAi(int ring, int ip, int port, unsigned char* v)
{
    if (v) *v = 0;
    const TVc8Route* r = Vc8Route();
    return (r && r->readAi) ? r->readAi(ring, ip, port, v) : (int)kVc8RcNoRoute;
}
inline int W906_Vc8ReadDi(int ring, int ip, int chan, unsigned char* v)
{
    if (v) *v = 0;
    const TVc8Route* r = Vc8Route();
    return (r && r->readDi) ? r->readDi(ring, ip, chan, v) : (int)kVc8RcNoRoute;
}
inline int W906_Vc8ReadDo(int ring, int ip, int chan, unsigned char* v)
{
    if (v) *v = 0;
    const TVc8Route* r = Vc8Route();
    return (r && r->readDo) ? r->readDo(ring, ip, chan, v) : (int)kVc8RcNoRoute;
}
inline int W906_Vc8WriteDo(int ring, int ip, int chan, unsigned char v)
{
    const TVc8Route* r = Vc8Route();
    return (r && r->writeDo) ? r->writeDo(ring, ip, chan, v) : (int)kVc8RcNoRoute;
}
inline int W906_Vc8ReadSdoI16(int ring, int ip, int index, int sub, short* v)
{
    if (v) *v = 0;
    const TVc8Route* r = Vc8Route();
    return (r && r->readSdoI16) ? r->readSdoI16(ring, ip, index, sub, v) : (int)kVc8RcNoRoute;
}
inline int W906_Vc8WriteSdoI16(int ring, int ip, int index, int sub, short v)
{
    const TVc8Route* r = Vc8Route();
    return (r && r->writeSdoI16) ? r->writeSdoI16(ring, ip, index, sub, v) : (int)kVc8RcNoRoute;
}
// check()'s `forWrite`: what the station is about to be used for.
enum {
    kVc8CheckRead        = 0,   // a read (DI / DO sample, or an SDO read)
    kVc8CheckPageWrite   = 1,   // a write HW.VacuumUnit causes: + VacuUnitType 1, not SystemStart / SoftStart, armed control
    kVc8CheckEngineWrite = 2    // an engine TMySucker write after the golden remap: the station only (the IO route that
                                //   carries it checks the control; the engine writes suckers while it runs)
};

inline int W906_Vc8Check(int ring, int ip, int forWrite, char* why, int whyLen)
{
    const TVc8Route* r = Vc8Route();
    if (r && r->check) return r->check(ring, ip, forWrite, why, whyLen);
    if (why && whyLen > 0) {
        const char* t = "no VC8 route in this process (a SOFT_SIMULTE build, a test, or a wb_serve without "
                        "INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL) -- golden's no-card values are shown";
        int i = 0;
        for (; t[i] && i < whyLen - 1; ++i) why[i] = t[i];
        why[i] = '\0';
    }
    return (int)kVc8RcNoRoute;
}
inline const char* W906_Vc8LastWhy()
{
    const TVc8Route* r = Vc8Route();
    return (r && r->lastWhy) ? r->lastWhy() : "no VC8 route installed";
}
inline unsigned long W906_Vc8LastSeq()
{
    const TVc8Route* r = Vc8Route();
    return (r && r->lastSeq) ? r->lastSeq() : 0ul;
}
inline int W906_Vc8WritesSince(unsigned long afterSeq, TVc8Write* out, int max)
{
    const TVc8Route* r = Vc8Route();
    return (r && r->writesSince) ? r->writesSince(afterSeq, out, max) : 0;
}
// AI(W906-VC4) 20261001: which vacuum unit answers at (ring, ip) -- kVc8ModelNone with no route (every test, a SIM
// build: golden's VC8 arithmetic then stands, and the call that follows fails as before).
inline int W906_Vc8Model(int ring, int ip)
{
    const TVc8Route* r = Vc8Route();
    return (r && r->model) ? r->model(ring, ip) : (int)kVc8ModelNone;
}

// ---------------------------------------------------------------------------
//  THE REMAPPED SUCKERS' GUARD -- EastSun ruling R3 20260930 (follow golden: SetIOTableByECAT_VC8_Sucker).
//
//  With VCCU_UNIT_TYPE==1 golden hands every Index / In / Out sucker to the ECAT-VC8 (SetSuckISABase ePCI1203,
//  golden VacuumUnit.cpp:461-476), and the sucker then switches and senses through MyLaneIO's ePCI1203 branch at
//  its own IO_Table address. On THIS machine those addresses share stations with modules that are NOT a VC8:
//  BTestSuck* (IndexArm2) sits on ring 1 stations 0x50 / 0x51 DO channels 16..31, and the ENABLED loader / auto1
//  cylinders C_Load_Up / C_LoaderDrawerLock / C_Auto1_Up / C_Auto1DrawerLock use channels 16..19 of those very
//  stations (IO_Table.csv). So the engine IO route's own checks (the byte is in the DO map and reads back) would
//  let a sucker write through to a cylinder coil.
//  The remap therefore REGISTERS each sucker's three aliases (SensorName, OnPortName, OffPortName) with the
//  address golden gave them, and MyLaneIO's ePCI1203 IOBitOn / IOBitOff / IOInputBit ask W906_Vc8SuckerGate()
//  before the backend call: a registered alias passes only when its station is an ECAT-VC8 (the route's identity
//  check). Anything else -- no route, no VC8, a DI / DO module at that address -- fails the call cleanly (golden's
//  own failure arm, MNetLog) with the reason on the console. Keyed on the ALIAS, never on the address, so the
//  cylinders that share the station are untouched.
//  Empty (nothing registered) unless MachineType.h W906_VC8_SUCKER_REMAP is defined and VCCU_UNIT_TYPE==1.
// ---------------------------------------------------------------------------
//
//  AI(W906-VC4) 20261001: EACH ALIAS ALSO CARRIES ITS ROLE AND ITS IO_Table PORT, and on an ECAT-VC4 station the
//  gate checks them against the VC4 layout (EastSun 1001 measured ... ; 「VC8 和 VC4 要不同分支」):
//      the sensor (SensorName)    Port 64+VC
//      the make   (OnPortName)    Port 17+2VC  (ODD)    -- the IO_Table "_On" row, TMySucker's vacuum
//      the break  (OffPortName)   Port 16+2VC  (EVEN)   -- the "_Off" row, TMySucker's blow
//  (AI(W906-VC4-ODD) 20261001: make / break were EVEN / ODD here until EastSun's 21:3x test showed the odd one sucks;
//   the paragraph below describes the reversed belief -- with the parity fixed, the VC8-layout _On 17 IS the VC4's make.)
//  with the DO pair's VC equal to the sensor's VC. WHY: the engine and the IO page switch a sucker at its IO_Table
//  Port verbatim (MyLaneIO ePCI1203 IOBitOn: channel = Port), and IO_Table.csv's rows are the VC8's layout today
//  (InArmSuckA_On Port 17) -- on a VC4, channel 17 BREAKS vacuum. Without this check, accepting the VC4's identity
//  would turn "vacuum on" into "blow" for every sucker on 160 / 161 / 162. A row that does not fit is refused (the
//  call fails as golden's vendor error), with the alias, its Port and what the VC4 wants; fix the row
//  (scratchpad vc4_iotable_proposal.csv). On an ECAT-VC8 nothing new is checked (as before).
// ---------------------------------------------------------------------------
enum { kVc8RoleUnknown = 0, kVc8RoleSensor = 1, kVc8RoleMake = 2, kVc8RoleBreak = 3 };   // AI(W906-VC4) 20261001
struct TVc8SuckerGuard {
    int n;
    struct E { int ring, ip; char name[40]; int role, port, senPort; } e[192];   // 16+16 Index + 8+8 In/Out suckers x 3 aliases = 144
                                                                                //   AI(W906-VC4) 20261001: + role / IO_Table Port / the sucker's sensor Port
};
inline TVc8SuckerGuard& W906_Vc8SuckerGuard_() { static TVc8SuckerGuard g; return g; }   // zero-initialised (static POD)
// The entry for (ring, ip, name), -1 = not registered.
inline int W906_Vc8GuardedSuckerAt_(int ring, int ip, const char* name)
{
    const TVc8SuckerGuard& g = W906_Vc8SuckerGuard_();
    for (int k = 0; name && k < g.n; ++k) {
        if (g.e[k].ring != ring || g.e[k].ip != ip) continue;
        int i = 0;
        while (name[i] && name[i] == g.e[k].name[i]) ++i;
        if (name[i] == '\0' && g.e[k].name[i] == '\0') return k;
    }
    return -1;
}
// AI(W906-VC4) 20261001: register with the role and the row's Port (VacuumUnit.cpp VaccumCopyFormSuck). A second
// registration of the same (ring, ip, name) updates it (golden SetIOTableByECAT_VC8_Sucker may run again).
inline void W906_Vc8GuardSuckerAs(int ring, int ip, const char* name, int role, int port, int senPort)
{
    TVc8SuckerGuard& g = W906_Vc8SuckerGuard_();
    if (!name || !*name) return;
    int k = W906_Vc8GuardedSuckerAt_(ring, ip, name);
    if (k < 0) {
        if (g.n >= (int)(sizeof(g.e) / sizeof(g.e[0]))) return;
        k = g.n++;
        TVc8SuckerGuard::E& y = g.e[k];
        y.ring = ring; y.ip = ip;
        int i = 0;
        for (; name[i] && i < (int)sizeof(y.name) - 1; ++i) y.name[i] = name[i];
        y.name[i] = '\0';
    }
    TVc8SuckerGuard::E& x = g.e[k];
    x.role = role; x.port = port; x.senPort = senPort;
}
inline void W906_Vc8GuardSucker(int ring, int ip, const char* name)   // no role: refused on a VC4 (cannot be checked)
{
    W906_Vc8GuardSuckerAs(ring, ip, name, kVc8RoleUnknown, -1, -1);
}
inline int W906_Vc8GuardedSuckers() { return W906_Vc8SuckerGuard_().n; }
inline bool W906_Vc8IsGuardedSucker(int ring, int ip, const char* name)
{
    return W906_Vc8GuardedSuckerAt_(ring, ip, name) >= 0;
}
// AI(W906-VC4) 20261001: on an ECAT-VC4, does this alias's IO_Table Port fit the VC4 layout for its role? (see above)
inline bool W906_Vc4SuckerRowOk(const TVc8SuckerGuard::E& x, char* why, int whyLen)
{
    const int  senVc = x.senPort - (int)kVc4DiOkChan;
    const bool senOk = senVc >= 0 && senVc < (int)kVc4Units;
    const char* bad = 0;
    if (x.role == kVc8RoleSensor) {
        if (x.port < (int)kVc4DiOkChan || x.port >= (int)kVc4DiOkChan + (int)kVc4Units) bad = "不是 VC4 的真空 OK 輸入（DI 64..67）";
    } else if (x.role == kVc8RoleMake || x.role == kVc8RoleBreak) {
        const bool make = (x.role == kVc8RoleMake);
        if (x.port < (int)kVc4DoFirst || x.port > (int)kVc4DoLast)       bad = "不是 VC4 的吸／破真空輸出（DO 16..23）";
        else if (((x.port - (int)kVc4DoFirst) & 1) != (make ? 1 : 0))    bad = make ? "在 VC4 上是「破真空」通道（_On 是吸真空）" : "在 VC4 上是「吸真空」通道（_Off 是破真空）";   // AI(W906-VC4-ODD) 20261001: make = odd
        else if (!senOk || (x.port - (int)kVc4DoFirst) / 2 != senVc)      bad = "和同一個吸嘴的感測（真空 OK）不是同一個 VC";
    } else {
        bad = "沒有登記它是吸嘴的哪一點（感測／吸／破），無法照 VC4 版面檢查";
    }
    if (!bad) return true;
    if (why && whyLen > 0)
        std::snprintf(why, (std::size_t)whyLen, "ring %d 站 0x%02X 是 ECAT-VC4，IO_Table 的 %s（Port %d）%s——VC4 的列要是 感測 64+VC、"
                      "_On 17+2VC、_Off 16+2VC（EastSun 20261001 實測：奇數吸），沒有送出", x.ring, x.ip, x.name, x.port, bad);
    return false;
}
// true = go ahead with the backend call. false = a registered sucker whose station is not a usable ECAT-VC8:
// the caller fails the call the way golden fails a vendor error; the reason is printed (rate-limited: the first
// refusal, then one in 200, per hash slot -- an enabled sucker is polled every tick).
// AI(W906-VC4) 20261001: ... or an ECAT-VC4 whose IO_Table row does not fit the VC4 layout (W906_Vc4SuckerRowOk).
inline char* W906_Vc8SuckerGateWhy() { static char s[320] = ""; return s; }   // the last refusal's reason
inline bool W906_Vc8SuckerGate(int ring, int ip, const char* name, bool write)
{
    const int at = (W906_Vc8SuckerGuard_().n == 0) ? -1 : W906_Vc8GuardedSuckerAt_(ring, ip, name);
    if (at < 0) return true;
    char* why = W906_Vc8SuckerGateWhy();
    why[0] = '\0';
    if (W906_Vc8Check(ring, ip, write ? kVc8CheckEngineWrite : kVc8CheckRead, why, 320) == 0 &&
        (W906_Vc8Model(ring, ip) != (int)kVc8ModelVc4 || W906_Vc4SuckerRowOk(W906_Vc8SuckerGuard_().e[at], why, 320)))   // AI(W906-VC4) 20261001
        return true;
    static unsigned long s_last[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static unsigned long s_count = 0;
    const unsigned long slot = (unsigned long)(((unsigned)ip * 31u + (unsigned)(name[0] ? name[1] : 0)) % 8u);
    ++s_count;
    if (s_last[slot] == 0 || s_count - s_last[slot] > 200ul) {
        s_last[slot] = s_count;
        std::printf("vacuum VC8 sucker gate: %s %s (ring %d st 0x%02X) NOT sent -- %s\n",
                    write ? "write" : "read", name, ring, ip, why);
    }
    return false;
}

#endif  // VACUUMUNIT_VC8ROUTE_H
