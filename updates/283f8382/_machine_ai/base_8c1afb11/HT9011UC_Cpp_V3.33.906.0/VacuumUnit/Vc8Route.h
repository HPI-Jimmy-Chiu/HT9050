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
    kVc8RcBadValue  = 0x7E00020A    // value outside the object's range
};

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
struct TVc8SuckerGuard {
    int n;
    struct E { int ring, ip; char name[40]; } e[192];   // 16+16 Index + 8+8 In/Out suckers x 3 aliases = 144
};
inline TVc8SuckerGuard& W906_Vc8SuckerGuard_() { static TVc8SuckerGuard g; return g; }   // zero-initialised (static POD)
inline void W906_Vc8GuardSucker(int ring, int ip, const char* name)
{
    TVc8SuckerGuard& g = W906_Vc8SuckerGuard_();
    if (!name || !*name || g.n >= (int)(sizeof(g.e) / sizeof(g.e[0]))) return;
    TVc8SuckerGuard::E& x = g.e[g.n++];
    x.ring = ring; x.ip = ip;
    int i = 0;
    for (; name[i] && i < (int)sizeof(x.name) - 1; ++i) x.name[i] = name[i];
    x.name[i] = '\0';
}
inline int W906_Vc8GuardedSuckers() { return W906_Vc8SuckerGuard_().n; }
inline bool W906_Vc8IsGuardedSucker(int ring, int ip, const char* name)
{
    const TVc8SuckerGuard& g = W906_Vc8SuckerGuard_();
    for (int k = 0; name && k < g.n; ++k) {
        if (g.e[k].ring != ring || g.e[k].ip != ip) continue;
        int i = 0;
        while (name[i] && name[i] == g.e[k].name[i]) ++i;
        if (name[i] == '\0' && g.e[k].name[i] == '\0') return true;
    }
    return false;
}
// true = go ahead with the backend call. false = a registered sucker whose station is not a usable ECAT-VC8:
// the caller fails the call the way golden fails a vendor error; the reason is printed (rate-limited: the first
// refusal, then one in 200, per hash slot -- an enabled sucker is polled every tick).
inline char* W906_Vc8SuckerGateWhy() { static char s[320] = ""; return s; }   // the last refusal's reason
inline bool W906_Vc8SuckerGate(int ring, int ip, const char* name, bool write)
{
    if (W906_Vc8SuckerGuard_().n == 0 || !W906_Vc8IsGuardedSucker(ring, ip, name)) return true;
    char* why = W906_Vc8SuckerGateWhy();
    why[0] = '\0';
    if (W906_Vc8Check(ring, ip, write ? kVc8CheckEngineWrite : kVc8CheckRead, why, 320) == 0) return true;
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
