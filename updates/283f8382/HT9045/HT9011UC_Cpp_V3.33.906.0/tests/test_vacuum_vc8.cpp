// ===========================================================================
//  tests/test_vacuum_vc8.cpp
//
//  AI(W906-VACUNIT-1203) 20260930: the ECAT-VC8 vacuum unit on the PCIE-1203 -- HW.VacuumUnit's hardware half
//  (EastSun 20260930 「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」; rulings R1-R3 20260930).
//  No card, no port, no file written: a FAKE ECAT-VC8 route stands in for EtherCAT/Pci1203Vc8Route.inc.
//
//    [1] the Set All table (VacuumUnit/VacuumUnitLive.h kVuSetAllArms) against the golden DFM's IR
//        (argv[1] = tools/dfm2rc/ir_out/VacuumUnit/VacuumUnit.dfm.ir.json): btnSetInArm no Tag = 0 (:223),
//        btnSetIndexArm Tag 1 (:250), btnSetOutArm Tag 2 (:260), all three OnClick = btnSetInArmClick;
//        an unknown arm maps to -1; the kPa range is golden's keyboard range
//    [2] EtherCAT/Pci1203Vc8.h, the identity check (pure): an ECx-P32 at golden's IndexArm2 address is refused
//        and the refusal quotes its name; a drive, a duplicated / unaddressable / absent station, ring 0, the
//        wrong EtherCAT state are refused; the closed SDO list and value ranges; the DO channel range / partner
//    [3] no route installed (every build but an armed wb_serve): golden's failure values, bit for bit --
//        MyLaneIO GetIOValue / GetIOValueThread 999.0, SetIOValueThread false; the sucker gate passes everything
//    [4] the live golden panels (golden Initial :38-110, HT9046_LS = the 8-column family HT9050 decodes to):
//        48 panels; the address table IS golden's TMyVacuumPanel ctor (spot checks on every kind)
//    [5] no route, the page open: golden tmr1Timer shows 999.0 and ends in Error5 (ReadVaccumIO -> Error3, then
//        golden's own InitialThresholdMode retry -> Error5, R2 = follow golden); DO read-backs unknown
//    [6] a fake VC8 at 0x20 (InArm): golden Reset, then the tick reads the pressure (VC8ToKpa), the threshold
//        (SDO 8000h:13h) and the vacuum-OK bit, and writes the MODE (8000h+VC*10h:02h = 1) with no press (R2)
//    [7] Set (btnSVClick): one threshold write, KpaToVC8(kPa), then the read-back
//    [8] Set All (btnSetInArmClick through the DFM Tags): In -> the 8 InArm VCs at 0x20; Index -> 32 attempts at
//        0x40/0x41/0x50/0x51, every one refused by the fake (not a VC8); an unknown Tag refused
//    [9] ^ / v (btnVaccumOnOffOnClick): vacuum ON writes chan 16+2VC+swap; blow ON while vacuum reads ON is
//        refused before any write (D3); vacuum OFF
//   [10] R3 (MachineType.h W906_VC8_SUCKER_REMAP): golden SetIOTableByECAT_VC8_Sucker copies the sucker table into
//        the panels, sets ISABase ePCI1203, and registers the suckers with the gate -- a registered alias passes
//        only on a VC8, an unregistered one (the loader cylinder sharing the station) always passes; AI(W906-VC8-IP)
//        20261001: the station is the sucker row's (InArm 160 = 0xA0, OutArm 161, Index 162) -- panel, gate, DO write
//   [11] Pci1203Control: the two VC8 kinds are INTERNAL (no wire name, audit name "(internal)"), refused on a
//        closed control
//
//  AI(W906-VC4) 20261001: THE ECAT-VC4 BRANCH (EastSun 1001 measured 「開真空是16bit 破真空17bit / ... /
//  開真空是22bit 破真空23bit」 on the ECAT-VC4-ODM1 at ring 1 160 / 161 / 162 ; 「VC8 和 VC4 要不同分支」):
//    [2]  + identity -> model (VC8 / VC4; a VC4 name with another VendorID, a drive, ECx-P32 refused, the refusal
//         names both table lines); the model by identity only (no state); the two layout tables; make / break / OK
//         channels; Vc8Route.h's pure VC4 copy equals Pci1203Vc8.h's table; the VC4 closed lists (DO 16..23 yes,
//         24 no; DI 64..67 yes, 128 no; pressure bytes 0..7; SDO VC 0..3); VC4 SDO writes refused by default
//   [12] a panel whose station answers as an ECAT-VC4: the same panel on a VC8 keeps golden's ports; on the VC4,
//        vacuum ON writes 16+2VC, blow 17+2VC, the DO read-back and the D3 interlock use them, the vacuum-OK bit is
//        read at 64+VC; the VC4 mode write is refused by policy without golden's Error5 (no freeze), the threshold
//        write fails, the threshold read works; the sucker gate refuses a VC8-layout IO_Table row on a VC4 and passes
//        the VC4 row; a VC8 there keeps the old rule
//  The fake route mirrors the real route's per-model lists by calling the real EtherCAT/Pci1203Vc8.h functions.
// ===========================================================================
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "MachineDefine.h"
#include "VacuumUnit/VacuumUnit.h"      // fVacuumUnit, SetIOTableByECAT_VC8_Sucker (before mykitsuck.h, as VacuumUnit.cpp)
#include "MachineType.h"
#include "cmydef.h"                     // MachineTypeChoice, VCCU_UNIT_TYPE, InitialOK, USE_46_SUCKER_DB
#include "cpublic.h"                    // VC8ToKpa / KpaToVC8
#include "MyLaneIo.h"                   // MyLaneIO
#include "mykitsuck.h"                  // FTestSuck / BTestSuck / InArmSuck / OutArmSuck
#include "VacuumUnit/VacuumUnitLive.h"
#include "VacuumUnit/Vc8Route.h"
#include "EtherCAT/Pci1203Vc8.h"
#include "EtherCAT/Pci1203Control.h"
#include "Public/cJSON.h"
#include "w906_ctest_guard.h"

using namespace ht9045;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_vacuum_vc8.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)
static bool Has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

// ---------------------------------------------------------------------------------------------------------
//  The fake ECAT-VC8 route: per station a DI image, a DO image, the eight VCs' mode / threshold objects.
// ---------------------------------------------------------------------------------------------------------
struct FakeWrite { int kind, ring, ip, chan, sub, value, rc; };
static struct Fake {
    bool          vc8[256];            // a vacuum unit answers at this station (an ECAT-VC8, unless vc4[] says VC4)
    bool          vc4[256];            // AI(W906-VC4) 20261001: ... and it is an ECAT-VC4
    unsigned char di[256][17];
    unsigned char dout[256][4];
    short         sdo[256][8][2];      // [station][vc][0 = mode 02h, 1 = threshold 13h]
    std::vector<FakeWrite> writes;
    unsigned long seq;
    TVc8Write     ring[128];
    int           lastDiChan;          // AI(W906-VC4) 20261001: the channel the last DI read asked
} g;

static void FakeReset()
{
    std::memset(g.vc8, 0, sizeof(g.vc8)); std::memset(g.di, 0, sizeof(g.di)); std::memset(g.dout, 0, sizeof(g.dout));
    std::memset(g.sdo, 0, sizeof(g.sdo)); g.writes.clear(); g.seq = 0; std::memset(g.ring, 0, sizeof(g.ring));
    std::memset(g.vc4, 0, sizeof(g.vc4)); g.lastDiChan = -1;
}
static int FRc(int ring, int ip) { return (ring == 1 && ip > 0 && ip < 256 && g.vc8[ip]) ? 0 : (int)kVc8RcNotVc8; }
static int FModel(int ring, int ip) { return FRc(ring, ip) ? (int)kVc8ModelNone : g.vc4[ip] ? (int)kVc8ModelVc4 : (int)kVc8ModelVc8; }   // AI(W906-VC4)
static int FReadAi(int ring, int ip, int port, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    std::string why;
    if (!Pci1203Vc8AiPortOkFor(FModel(ring, ip), port, why)) return (int)kVc8RcBadAddr;   // VC8 0..15, VC4 0..7
    *v = g.di[ip][port]; return 0;
}
static int FReadDi(int ring, int ip, int chan, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    g.lastDiChan = chan;
    std::string why;
    const int m = FModel(ring, ip);
    if (!Pci1203Vc8DiChanOkFor(m, chan, why)) return (int)kVc8RcBadAddr;                  // VC8 128..135, VC4 64..67
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(m);
    *v = (unsigned char)((g.di[ip][L.diBytes - 1] >> (chan - L.diOkChan)) & 1); return 0;
}
static int FReadDo(int ring, int ip, int chan, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    std::string why;
    if (!Pci1203Vc8DoChanOkFor(FModel(ring, ip), chan, why)) return (int)kVc8RcBadAddr;   // VC8 16..31, VC4 16..23
    *v = (unsigned char)((g.dout[ip][chan / 8] >> (chan % 8)) & 1); return 0;
}
static void FRecord(int kind, int ring, int ip, int chan, int sub, int value, int rc)
{
    FakeWrite w = { kind, ring, ip, chan, sub, value, rc };
    g.writes.push_back(w);
    TVc8Write& t = g.ring[g.seq % 128];
    std::memset(&t, 0, sizeof(t));
    t.seq = ++g.seq; t.kind = kind; t.ring = ring; t.ip = ip; t.chan = chan; t.sub = sub; t.value = value; t.rc = rc;
    t.issued = (rc == 0);
}
static int FWriteDo(int ring, int ip, int chan, unsigned char v)
{
    std::string why;
    int rc = FRc(ring, ip);
    if (rc == 0 && !Pci1203Vc8DoChanOkFor(FModel(ring, ip), chan, why)) rc = (int)kVc8RcBadAddr;   // AI(W906-VC4)
    FRecord(0, ring, ip, chan, -1, v, rc);
    if (rc) return rc;
    if (v) g.dout[ip][chan / 8] |= (unsigned char)(1u << (chan % 8)); else g.dout[ip][chan / 8] &= (unsigned char)~(1u << (chan % 8));
    return 0;
}
static int FReadSdo(int ring, int ip, int index, int sub, short* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    std::string why;
    int vc = -1;
    if (!Pci1203Vc8SdoAddrOkFor(FModel(ring, ip), index, sub, &vc, why)) return (int)kVc8RcBadAddr;   // AI(W906-VC4): VC4 0..3
    *v = g.sdo[ip][vc][sub == 0x02 ? 0 : 1]; return 0;
}
static int FWriteSdo(int ring, int ip, int index, int sub, short v)
{
    std::string why;
    int rc = FRc(ring, ip);
    if (rc == 0 && !Pci1203Vc8SdoWriteOk(FModel(ring, ip), why)) rc = (int)kVc8RcVc4SdoOff;          // AI(W906-VC4): as the route
    FRecord(1, ring, ip, index, sub, v, rc);
    if (rc) return rc;
    g.sdo[ip][(index - 0x8000) / 0x10][sub == 0x02 ? 0 : 1] = v;
    return 0;
}
static int FCheck(int ring, int ip, int, char* why, int n)
{
    const int rc = FRc(ring, ip);
    if (rc && why && n > 0) std::snprintf(why, (std::size_t)n, "fake: ring %d station 0x%02X is not an ECAT-VC8", ring, ip);
    return rc;
}
static const char* FLastWhy() { return "fake"; }
static unsigned long FLastSeq() { return g.seq; }
static int FWritesSince(unsigned long after, TVc8Write* out, int max)
{
    int n = 0;
    for (unsigned long s = after + 1; s <= g.seq && n < max; ++s) out[n++] = g.ring[(s - 1) % 128];
    return n;
}
static const TVc8Route kFake = { FReadAi, FReadDi, FReadDo, FWriteDo, FReadSdo, FWriteSdo, FCheck, FLastWhy, FLastSeq, FWritesSince, FModel };

static Pci1203SlaveSample Slave(int ring, int addr, const char* name, unsigned short state)
{
    Pci1203SlaveSample s;
    s.present = true; s.ring = ring; s.addr = addr; s.infoValid = true; s.name = name;
    s.stateValid = true; s.state = state;
    return s;
}

static int Find(const char* id) { return W906_VacuumLiveFind(id); }
static TVuLivePanel P(const char* id) { TVuLivePanel lp; std::memset(&lp, 0, sizeof(lp)); W906_VacuumLivePanel(Find(id), &lp); return lp; }
static void Ticks(int n) { for (int i = 0; i < n; ++i) W906_VacuumLiveTick(64); }

int main(int argc, char** argv)
{
    if (!W906TestRequireCtestRedirects("VacuumVc8"))
        return 2;
    const char* irPath = argc > 1 ? argv[1] : "";

    // ---------------------------------------------------------------- [1]
    std::printf("-- [1] Set All table vs the golden DFM IR (%s)\n", irPath);
    {
        std::ifstream f(irPath, std::ios::binary);
        std::stringstream ss; ss << f.rdbuf();
        const std::string text = ss.str();
        CHECK(!text.empty());
        cJSON* root = cJSON_Parse(text.c_str());
        CHECK(root != 0);
        const cJSON* nodes = root ? cJSON_GetObjectItemCaseSensitive(root, "nodes") : 0;
        CHECK(nodes && cJSON_IsArray(nodes));
        int n = 0;
        const TVuSetAllArm* arms = W906_VuSetAllArms(&n);
        CHECK(n == 3);
        int matched = 0;
        for (int k = 0; k < n; ++k) {
            const cJSON* node = 0;
            for (const cJSON* x = nodes ? nodes->child : 0; x; x = x->next) {
                const cJSON* nm = cJSON_GetObjectItemCaseSensitive(x, "name");
                if (nm && cJSON_IsString(nm) && std::strcmp(nm->valuestring, arms[k].button) == 0) { node = x; break; }
            }
            CHECK(node != 0);
            if (!node) continue;
            const cJSON* props = cJSON_GetObjectItemCaseSensitive(node, "properties");
            const cJSON* tag = props ? cJSON_GetObjectItemCaseSensitive(props, "Tag") : 0;
            const cJSON* tv = tag ? cJSON_GetObjectItemCaseSensitive(tag, "value") : 0;
            const int dfmTag = (tv && cJSON_IsNumber(tv)) ? (int)tv->valuedouble : 0;   // no Tag line in the DFM = 0
            const cJSON* line = cJSON_GetObjectItemCaseSensitive(node, "line");
            const cJSON* ev = cJSON_GetObjectItemCaseSensitive(node, "events");
            const cJSON* oc = ev ? cJSON_GetObjectItemCaseSensitive(ev, "OnClick") : 0;
            std::printf("   %-15s DFM Tag %d (table %d)  line %d (table %d)  OnClick %s\n", arms[k].button, dfmTag, arms[k].tag,
                        line ? (int)line->valuedouble : -1, arms[k].dfmLine, (oc && cJSON_IsString(oc)) ? oc->valuestring : "?");
            CHECK(dfmTag == arms[k].tag);
            CHECK(line && (int)line->valuedouble == arms[k].dfmLine);
            CHECK(oc && cJSON_IsString(oc) && std::strcmp(oc->valuestring, "btnSetInArmClick") == 0);
            if (dfmTag == arms[k].tag) ++matched;
        }
        CHECK(matched == 3);
        if (root) cJSON_Delete(root);
        CHECK(W906_VuSetAllTag("in") == 0 && W906_VuSetAllTag("index") == 1 && W906_VuSetAllTag("out") == 2);
        CHECK(W906_VuSetAllTag("InArm") == -1 && W906_VuSetAllTag("") == -1 && W906_VuSetAllTag(0) == -1);
        CHECK(W906_VuKpaOk(-116.0) && W906_VuKpaOk(148.0) && W906_VuKpaOk(0.0));
        CHECK(!W906_VuKpaOk(-116.1) && !W906_VuKpaOk(148.1) && !W906_VuKpaOk(std::sqrt(-1.0)));
    }

    // ---------------------------------------------------------------- [2]
    std::printf("-- [2] EtherCAT/Pci1203Vc8.h identity / closed lists\n");
    {
        std::vector<Pci1203SlaveSample> sl;
        sl.push_back(Slave(1, 0x50, "ECx-P32-HON 32DI 32 Ch. Dig. In.", 0x08));   // the tray-sensor module golden's IndexArm2 address hits
        sl.push_back(Slave(1, 0x20, "ECAT-VC8_ODM1", 0x08));
        sl.push_back(Slave(1, 0x30, "ecat-vc8", 0x04));                               // SAFEOP
        sl.push_back(Slave(1, 0x40, "ECAT-VC8", 0x02));                               // PREOP
        sl.push_back(Slave(1, 0x41, "SGDXS SERVOPACK", 0x08));
        Pci1203SlaveSample d402 = Slave(1, 0x51, "ECAT-VC8", 0x08); d402.profileValid = true; d402.profile = 402; sl.push_back(d402);
        Pci1203SlaveSample un = Slave(1, 0x60, "ECAT-VC8", 0x08); un.unaddressable = true; sl.push_back(un);
        sl.push_back(Slave(1, 0x70, "ECAT-VC8", 0x08)); sl.push_back(Slave(1, 0x70, "ECAT-VC8", 0x08));   // two answer one address
        sl.push_back(Slave(0, 0x20, "ECAT-VC8", 0x08));
        const int n = (int)sl.size();
        std::string why; int slot = -1;
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x50, kVc8UseReadIo, &slot, why));
        CHECK(Has(why, "ECx-P32-HON") && Has(why, "ECAT-VC8"));
        std::printf("   0x50 -> %s\n", why.c_str());
        why.clear();
        CHECK(Pci1203Vc8StationOk(&sl[0], n, 1, 0x20, kVc8UseWrite, &slot, why) && slot == 1);
        CHECK(Pci1203Vc8StationOk(&sl[0], n, 1, 0x30, kVc8UseReadIo, &slot, why));     // name match is case-insensitive
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x30, kVc8UseWrite, &slot, why));    // SAFEOP: no outputs
        CHECK(Pci1203Vc8StationOk(&sl[0], n, 1, 0x40, kVc8UseReadSdo, &slot, why));    // PREOP: mailbox
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x40, kVc8UseReadIo, &slot, why));
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x41, kVc8UseReadIo, &slot, why) && Has(why, "SERVOPACK"));
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x51, kVc8UseReadIo, &slot, why) && Has(why, "402"));
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x60, kVc8UseReadIo, &slot, why));
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x70, kVc8UseReadIo, &slot, why));
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 0, 0x20, kVc8UseReadIo, &slot, why));    // ring 0
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x21, kVc8UseReadIo, &slot, why) && Has(why, "0x21"));
        int vc = -1;
        CHECK(Pci1203Vc8SdoAddrOk(0x8000, 0x13, &vc, why) && vc == 0);
        CHECK(Pci1203Vc8SdoAddrOk(0x8070, 0x02, &vc, why) && vc == 7);
        CHECK(!Pci1203Vc8SdoAddrOk(0x8080, 0x13, &vc, why) && !Pci1203Vc8SdoAddrOk(0x8001, 0x13, &vc, why));
        CHECK(!Pci1203Vc8SdoAddrOk(0x7FF0, 0x13, &vc, why) && !Pci1203Vc8SdoAddrOk(0x8000, 0x14, &vc, why));
        CHECK(Pci1203Vc8SdoValueOk(0x02, 1, why) && !Pci1203Vc8SdoValueOk(0x02, 0, why) && !Pci1203Vc8SdoValueOk(0x02, 2, why));
        CHECK(Pci1203Vc8SdoValueOk(0x13, 0, why) && Pci1203Vc8SdoValueOk(0x13, 32767, why));
        CHECK(!Pci1203Vc8SdoValueOk(0x13, -1, why) && !Pci1203Vc8SdoValueOk(0x13, 32768, why));
        CHECK(Pci1203Vc8DoChanOk(16, why) && Pci1203Vc8DoChanOk(31, why) && !Pci1203Vc8DoChanOk(15, why) && !Pci1203Vc8DoChanOk(32, why));
        CHECK(Pci1203Vc8DoPartner(16) == 17 && Pci1203Vc8DoPartner(31) == 30);
        CHECK(KpaToVC8(-116.0) == 0 && KpaToVC8(148.0) == 32767);

        //  AI(W906-VC4) 20261001: identity -> model. The existing VC8 rows above still answer VC8.
        int m = -1;
        CHECK(Pci1203Vc8StationOk(&sl[0], n, 1, 0x20, kVc8UseWrite, &slot, why, &m) && m == kPci1203VacVc8);
        CHECK(!Pci1203Vc8StationOk(&sl[0], n, 1, 0x50, kVc8UseReadIo, &slot, why, &m) && m == kPci1203VacNone);
        std::vector<Pci1203SlaveSample> s4;
        Pci1203SlaveSample a0 = Slave(1, 0xA0, "ECAT-VC4-ODM1", 0x08); a0.vendorId = 0x00494350ul; a0.productId = 0x00A20401ul; s4.push_back(a0);
        Pci1203SlaveSample a1 = Slave(1, 0xA1, "ECAT-VC4-ODM1", 0x08); a1.vendorId = 0x12345678ul; s4.push_back(a1);   // the VC4 name, another vendor
        Pci1203SlaveSample a2 = Slave(1, 0xA2, "ecat-vc4-odm1", 0x02); a2.vendorId = 0x00494350ul; s4.push_back(a2);   // PREOP, lower case
        Pci1203SlaveSample a3 = Slave(1, 0xA3, "ECAT-VC4-ODM1", 0x08); a3.vendorId = 0x00494350ul; a3.profileValid = true; a3.profile = 402; s4.push_back(a3);
        Pci1203SlaveSample a4 = Slave(1, 0xA4, "ECAT-VC4-ODM1", 0x08); a4.vendorId = 0x00494350ul; a4.stateValid = false; s4.push_back(a4);
        s4.push_back(Slave(1, 0x20, "ECAT-VC8_ODM1", 0x08));
        s4.push_back(Slave(1, 0x50, "ECx-P32-HON 32DI 32 Ch. Dig. In.", 0x08));
        const int n4 = (int)s4.size();
        CHECK(Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA0, kVc8UseWrite, &slot, why, &m) && m == kPci1203VacVc4 && slot == 0);
        CHECK(Pci1203Vc8StationOk(&s4[0], n4, 1, 0x20, kVc8UseWrite, &slot, why, &m) && m == kPci1203VacVc8 && slot == 5);
        why.clear();
        CHECK(!Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA1, kVc8UseReadIo, &slot, why, &m) && m == kPci1203VacNone);
        CHECK(Has(why, "ECAT-VC4-ODM1") && Has(why, "0x12345678") && Has(why, "ECAT-VC8") && Has(why, "0x00494350"));
        std::printf("   0xA1 -> %s\n", why.c_str());
        CHECK(Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA2, kVc8UseReadSdo, &slot, why, &m) && m == kPci1203VacVc4);   // case-insensitive
        CHECK(!Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA2, kVc8UseWrite, &slot, why, &m) && m == kPci1203VacNone);   // PREOP: no write
        CHECK(!Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA3, kVc8UseReadIo, &slot, why, &m) && Has(why, "402"));       // a drive is a drive
        CHECK(!Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA4, kVc8UseReadIo, &slot, why, &m));                           // state not read
        why.clear();
        CHECK(!Pci1203Vc8StationOk(&s4[0], n4, 1, 0x50, kVc8UseReadIo, &slot, why, &m) && Has(why, "ECx-P32-HON") && Has(why, "ECAT-VC4"));
        CHECK(Pci1203Vc8StationOk(&s4[0], n4, 1, 0xA4, kVc8UseIdentity, &slot, why, &m) && m == kPci1203VacVc4);  // identity only: no state
        CHECK(Pci1203Vc8StationModel(&s4[0], n4, 1, 0xA0) == kPci1203VacVc4 && Pci1203Vc8StationModel(&s4[0], n4, 1, 0xA4) == kPci1203VacVc4);
        CHECK(Pci1203Vc8StationModel(&s4[0], n4, 1, 0x20) == kPci1203VacVc8 && Pci1203Vc8StationModel(&s4[0], n4, 1, 0x50) == kPci1203VacNone);
        CHECK(Pci1203Vc8StationModel(&s4[0], n4, 1, 0xA1) == kPci1203VacNone && Pci1203Vc8StationModel(&s4[0], n4, 1, 0xA3) == kPci1203VacNone);
        CHECK(Pci1203Vc8StationModel(&s4[0], n4, 1, 0x99) == kPci1203VacNone && Pci1203Vc8StationModel(&s4[0], n4, 0, 0xA0) == kPci1203VacNone);
        CHECK(Pci1203Vc8StationModel(&sl[0], n, 1, 0x70) == kPci1203VacNone && Pci1203Vc8StationModel(&sl[0], n, 1, 0x60) == kPci1203VacNone);   // two answer / unaddressable
        std::string w2;
        CHECK(Pci1203Vc8IdentityOk(a0, w2, &m) && m == kPci1203VacVc4 && Pci1203Vc8IdentityOk(sl[1], w2, &m) && m == kPci1203VacVc8);

        //  the two layout tables (Pci1203VacLayoutOf), and anything else = the VC8 table (the pre-VC4 entry points)
        const Pci1203VacLayout& L8 = Pci1203VacLayoutOf(kPci1203VacVc8);
        const Pci1203VacLayout& L4 = Pci1203VacLayoutOf(kPci1203VacVc4);
        CHECK(L8.units == 8 && L8.diBytes == 17 && L8.diOkChan == 128 && L8.doFirst == 16 && L8.doLast == 31 && L8.makeOdd == 1);
        CHECK(L4.units == 4 && L4.diBytes == 9  && L4.diOkChan == 64  && L4.doFirst == 16 && L4.doLast == 23 && L4.makeOdd == 1);   // AI(W906-VC4-ODD) 20261001: was 0
        CHECK(L8.doFirst / 8 == 2 && L8.doLast / 8 == 3 && L4.doFirst / 8 == 2 && L4.doLast / 8 == 2);       // DO bytes 2..3 / 2
        CHECK(L8.diOkChan == 8 * (L8.diBytes - 1) && L4.diOkChan == 8 * (L4.diBytes - 1));
        CHECK(&Pci1203VacLayoutOf(kPci1203VacNone) == &L8 && L4.model == kPci1203VacVc4 && L8.model == kPci1203VacVc8);
        for (int v = 0; v < 4; ++v) {   // AI(W906-VC4-ODD) 20261001: EastSun 21:3x 「箭頭往下的在吸」 -> 吸真空 17/19/21/23, 破真空 16/18/20/22
            CHECK(Pci1203Vc8MakeChan(kPci1203VacVc4, v) == 17 + 2 * v && Pci1203Vc8BreakChan(kPci1203VacVc4, v) == 16 + 2 * v);
            CHECK(Pci1203Vc8OkChan(kPci1203VacVc4, v) == 64 + v);
            CHECK(W906_Vc4MakeChan(v) == Pci1203Vc8MakeChan(kPci1203VacVc4, v) && W906_Vc4BreakChan(v) == Pci1203Vc8BreakChan(kPci1203VacVc4, v));
            CHECK(W906_Vc4OkChan(v) == Pci1203Vc8OkChan(kPci1203VacVc4, v));
        }
        CHECK(Pci1203Vc8MakeChan(kPci1203VacVc8, 0) == 17 && Pci1203Vc8BreakChan(kPci1203VacVc8, 0) == 16);  // the VC8 manual: odd = make
        CHECK(Pci1203Vc8MakeChan(kPci1203VacVc8, 7) == 31 && Pci1203Vc8OkChan(kPci1203VacVc8, 7) == 135);
        CHECK((int)kVc8ModelVc4 == (int)kPci1203VacVc4 && (int)kVc8ModelVc8 == (int)kPci1203VacVc8 && (int)kVc8ModelNone == (int)kPci1203VacNone);
        CHECK((int)kVc4Units == L4.units && (int)kVc4DoFirst == L4.doFirst && (int)kVc4DoLast == L4.doLast && (int)kVc4DiOkChan == L4.diOkChan);

        //  the VC4's closed lists; the VC8's unchanged
        CHECK(Pci1203Vc8DoChanOkFor(kPci1203VacVc4, 16, why) && Pci1203Vc8DoChanOkFor(kPci1203VacVc4, 23, why));
        why.clear();
        CHECK(!Pci1203Vc8DoChanOkFor(kPci1203VacVc4, 24, why) && Has(why, "ECAT-VC4") && Has(why, "16..23"));
        CHECK(!Pci1203Vc8DoChanOkFor(kPci1203VacVc4, 15, why) && !Pci1203Vc8DoChanOkFor(kPci1203VacVc4, 31, why));
        CHECK(Pci1203Vc8DoChanOkFor(kPci1203VacVc8, 24, why) && Pci1203Vc8DoChanOkFor(kPci1203VacVc8, 31, why));
        why.clear();
        CHECK(!Pci1203Vc8DoChanOk(32, why) && Has(why, "ECAT-VC8") && Has(why, "16..31"));                    // the old text
        for (int ch = 64; ch <= 67; ++ch) CHECK(Pci1203Vc8DiChanOkFor(kPci1203VacVc4, ch, why));
        CHECK(!Pci1203Vc8DiChanOkFor(kPci1203VacVc4, 68, why) && !Pci1203Vc8DiChanOkFor(kPci1203VacVc4, 63, why));
        CHECK(!Pci1203Vc8DiChanOkFor(kPci1203VacVc4, 128, why) && Has(why, "64..67"));
        CHECK(Pci1203Vc8DiChanOkFor(kPci1203VacVc8, 128, why) && Pci1203Vc8DiChanOkFor(kPci1203VacVc8, 135, why));
        CHECK(!Pci1203Vc8DiChanOkFor(kPci1203VacVc8, 64, why) && !Pci1203Vc8DiChanOkFor(kPci1203VacVc8, 136, why));
        CHECK(Pci1203Vc8AiPortOkFor(kPci1203VacVc4, 0, why) && Pci1203Vc8AiPortOkFor(kPci1203VacVc4, 7, why));   // VC n = bytes 2n / 2n+1
        CHECK(!Pci1203Vc8AiPortOkFor(kPci1203VacVc4, 8, why) && !Pci1203Vc8AiPortOkFor(kPci1203VacVc4, -1, why));
        CHECK(Pci1203Vc8AiPortOkFor(kPci1203VacVc8, 15, why) && !Pci1203Vc8AiPortOkFor(kPci1203VacVc8, 16, why));
        CHECK(Pci1203Vc8SdoAddrOkFor(kPci1203VacVc4, 0x8030, 0x13, &vc, why) && vc == 3);
        CHECK(Pci1203Vc8SdoAddrOkFor(kPci1203VacVc4, 0x8000, 0x02, &vc, why) && vc == 0);
        why.clear();
        CHECK(!Pci1203Vc8SdoAddrOkFor(kPci1203VacVc4, 0x8040, 0x13, &vc, why) && Has(why, "ECAT-VC4") && Has(why, "8030h"));
        CHECK(Pci1203Vc8SdoAddrOkFor(kPci1203VacVc8, 0x8040, 0x02, &vc, why) && vc == 4);
        why.clear();
        CHECK(!Pci1203Vc8SdoAddrOk(0x8080, 0x13, &vc, why) && Has(why, "8000h..8070h"));                   // the old text
        CHECK(Pci1203Vc8SdoWriteOk(kPci1203VacVc8, why) && Pci1203Vc8SdoWriteOk(kPci1203VacNone, why));
#ifdef W906_VC4_SDO_WRITE
        CHECK(Pci1203Vc8SdoWriteOk(kPci1203VacVc4, why));
#else
        why.clear();
        CHECK(!Pci1203Vc8SdoWriteOk(kPci1203VacVc4, why) && Has(why, "W906_VC4_SDO_WRITE"));                 // VC4 SDO writes: OFF
#endif
        CHECK(Pci1203Vc8DoPartner(20) == 21 && Pci1203Vc8DoPartner(23) == 22);                                   // the VC4's pairs
    }

    // ---------------------------------------------------------------- [3]
    std::printf("-- [3] no route: golden's failure values\n");
    {
        SetVc8Route(0);
        CHECK(MyLaneIO.GetIOValue(1, 0x20, 0, 0, ePCI1203) == 999.0);
        CHECK(MyLaneIO.GetIOValueThread(1, 0x20, 0, 0, ePCI1203) == 999.0);
        CHECK(MyLaneIO.SetIOValueThread(-50.0, 1, 0x20, 0, 0, ePCI1203) == false);
        char why[320] = "";
        CHECK(W906_Vc8Check(1, 0x20, 1, why, (int)sizeof(why)) == (int)kVc8RcNoRoute && Has(why, "no VC8 route"));
        CHECK(W906_Vc8GuardedSuckers() == 0 && W906_Vc8SuckerGate(1, 0x50, "BTestSuckAA_On", true));
    }

    // ---------------------------------------------------------------- [4]
    std::printf("-- [4] the live golden panels (HT9046_LS)\n");
    MachineTypeChoice = Type_HT9046_LS;
    USE_46_SUCKER_DB = 0;
    InitialOK = true;
    {
        char why[200] = "";
        CHECK(W906_VacuumLiveBuild(why, (int)sizeof(why)));
        CHECK(W906_VacuumLiveCount() == 48);
        TVuLivePanel a = P("myPalArm1_0_0");
        CHECK(a.kind == 0 && a.ring == 1 && a.ip == 0x40 && a.vc == 0 && a.onPort == 17 && a.offPort == 16 && a.senPort == 128);
        a = P("myPalArm1_5_1");          // iIndexVCSort1_16[1][5] = 3, col > 3 -> 0x41
        CHECK(a.ip == 0x41 && a.vc == 3 && a.onPort == 23 && a.offPort == 22 && a.senPort == 131);
        a = P("myPalArm2_0_0");          // IndexArm2 HT9046LS: iIndexVCSort2_16[0][0] = 3, swap 0
        CHECK(a.kind == 1 && a.ip == 0x50 && a.vc == 3 && a.swap == 0 && a.onPort == 22 && a.offPort == 23 && a.senPort == 131);
        a = P("myPalInArm_1_1");         // iInOutVCSort[1][1] = 5
        CHECK(a.kind == 2 && a.ip == 0x20 && a.vc == 5 && a.onPort == 27 && a.offPort == 26 && a.senPort == 133);
        a = P("myPalOutArm_3_0");
        CHECK(a.kind == 3 && a.ip == 0x30 && a.vc == 3 && a.onPort == 23 && a.offPort == 22);
        CHECK(std::strcmp(a.cur, "0.0") == 0 && std::strcmp(a.evt, "Event") == 0 && a.led == -1 && a.onDown == -1);
        CHECK(Find("myPalArm1_8_0") == -1 && Find("bogus") == -1);
    }

    // ---------------------------------------------------------------- [5]
    std::printf("-- [5] no route, page open: 999.0 then Error3 / Error5 (golden)\n");
    {
        for (int i = 0; i < W906_VacuumLiveCount(); ++i) W906_VacuumLiveSetVisible(i, true);
        W906_VacuumLiveShow();
        TVuLiveState st; W906_VacuumLiveState(&st);
        CHECK(st.show && st.iCount == 2);
        Ticks(2);
        CHECK(std::strcmp(P("myPalInArm_0_0").cur, "0.0") == 0);          // golden iCount: two quiet ticks
        Ticks(2);
        const TVuLivePanel a = P("myPalInArm_0_0"), b = P("myPalArm1_0_0");
        std::printf("   myPalInArm_0_0 cur %s thr %s evt %s ; myPalArm1_0_0 evt %s\n", a.cur, a.thr, a.evt, b.evt);
        CHECK(std::strcmp(a.cur, "999.0") == 0 && std::strcmp(a.thr, "999.0") == 0);
        CHECK(std::strcmp(a.evt, "Error5") == 0 && std::strcmp(b.evt, "Error5") == 0);
        CHECK(a.led == 0 && a.onDown == -1 && a.offDown == -1 && !a.modeOk);
    }

    // ---------------------------------------------------------------- [6]
    std::printf("-- [6] a fake VC8 at 0x20: the tick reads, and writes the MODE with no press (R2)\n");
    FakeReset();
    SetVc8Route(&kFake);
    VCCU_UNIT_TYPE = 1;
    g.vc8[0x20] = true;
    g.di[0x20][0] = 0x00; g.di[0x20][1] = 0x40;             // VC0 raw 0x4000 = 16384 -> 16.0 kPa
    g.di[0x20][16] = 0x01;                                  // VC0 vacuum OK
    g.sdo[0x20][0][1] = (short)KpaToVC8(-50.0);
    {
        for (int i = 0; i < W906_VacuumLiveCount(); ++i) {
            TVuLivePanel lp; W906_VacuumLivePanel(i, &lp);
            W906_VacuumLiveSetVisible(i, lp.arm == kVuArmIn);
        }
        char why[200] = "";
        CHECK(W906_VacuumLiveReset(why, (int)sizeof(why)) == 0);
        CHECK(std::strcmp(P("myPalInArm_0_0").evt, "Reset") == 0);
        Ticks(1);
        const TVuLivePanel a = P("myPalInArm_0_0");
        std::printf("   myPalInArm_0_0 cur %s thr %s evt %s led %d\n", a.cur, a.thr, a.evt, a.led);
        CHECK(std::strcmp(a.cur, "16.0") == 0);
        CHECK(std::strcmp(a.thr, "-50.0") == 0);
        CHECK(a.led == 1 && a.modeOk && std::strcmp(a.evt, "OK") == 0);
        int modes = 0;
        for (std::size_t k = 0; k < g.writes.size(); ++k)
            if (g.writes[k].kind == 1 && g.writes[k].ip == 0x20 && g.writes[k].sub == 0x02 && g.writes[k].value == 1 && g.writes[k].rc == 0) ++modes;
        CHECK(modes == 8);                                   // the 8 InArm VCs, each once
        CHECK(g.sdo[0x20][0][0] == 1 && g.sdo[0x20][7][0] == 1);
        const std::size_t before = g.writes.size();
        Ticks(3);
        CHECK(g.writes.size() == before);                    // mode OK: golden stops writing it
        CHECK(P("myPalInArm_0_0").onDown == 0 && P("myPalInArm_0_0").offDown == 0);
    }

    // ---------------------------------------------------------------- [7]
    std::printf("-- [7] Set (btnSVClick)\n");
    {
        const std::size_t before = g.writes.size();
        char why[200] = "";
        CHECK(W906_VacuumLiveSetSV(Find("myPalInArm_0_0"), -60.0, why, (int)sizeof(why)) == 0);
        CHECK(g.writes.size() == before + 1);
        const FakeWrite w = g.writes.back();
        CHECK(w.kind == 1 && w.ip == 0x20 && w.chan == 0x8000 && w.sub == 0x13 && w.value == KpaToVC8(-60.0) && w.rc == 0);
        CHECK(P("myPalInArm_0_0").needThr);
        Ticks(1);
        CHECK(std::strcmp(P("myPalInArm_0_0").thr, "-60.0") == 0 || std::strcmp(P("myPalInArm_0_0").thr, "-60.1") == 0);
        CHECK(!P("myPalInArm_0_0").needThr);
    }

    // ---------------------------------------------------------------- [8]
    std::printf("-- [8] Set All through the DFM Tags\n");
    {
        char why[200] = "";
        std::size_t before = g.writes.size();
        CHECK(W906_VacuumLiveSetAll(0, -40.0, why, (int)sizeof(why)) == 0);          // InArm
        int in = 0;
        for (std::size_t k = before; k < g.writes.size(); ++k)
            if (g.writes[k].kind == 1 && g.writes[k].ip == 0x20 && g.writes[k].sub == 0x13 && g.writes[k].value == KpaToVC8(-40.0) && g.writes[k].rc == 0) ++in;
        CHECK(in == 8 && g.writes.size() == before + 8);
        CHECK(std::strcmp(fVacuumUnit->myPalInArm[2][1]->edSV->Text.c_str(), "-40") == 0);   // golden edSV->Text=(int)dValue
        before = g.writes.size();
        CHECK(W906_VacuumLiveSetAll(1, -40.0, why, (int)sizeof(why)) == 0);          // Index Arm1 / Arm2
        int idx = 0, refused = 0;
        for (std::size_t k = before; k < g.writes.size(); ++k) {
            const FakeWrite& w = g.writes[k];
            if (w.ip == 0x40 || w.ip == 0x41 || w.ip == 0x50 || w.ip == 0x51) ++idx;
            if (w.rc != 0) ++refused;
        }
        CHECK(idx == 32 && refused == 32 && g.writes.size() == before + 32);   // no VC8 there: every write refused
        before = g.writes.size();
        CHECK(W906_VacuumLiveSetAll(2, -40.0, why, (int)sizeof(why)) == 0);          // OutArm
        CHECK(g.writes.size() == before + 8 && g.writes.back().ip == 0x30);
        before = g.writes.size();
        CHECK(W906_VacuumLiveSetAll(3, -40.0, why, (int)sizeof(why)) != 0);          // no such Tag
        CHECK(W906_VacuumLiveSetAll(-1, -40.0, why, (int)sizeof(why)) != 0);
        CHECK(g.writes.size() == before);
    }

    // ---------------------------------------------------------------- [9]
    std::printf("-- [9] ^ / v (btnVaccumOnOffOnClick)\n");
    {
        char why[320] = "";
        const int i = Find("myPalInArm_0_0");                   // VC0, swap 1: vacuum chan 17, blow chan 16
        std::size_t before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, true, true, why, (int)sizeof(why)) == 0);
        CHECK(g.writes.size() == before + 1 && g.writes.back().kind == 0 && g.writes.back().chan == 17 && g.writes.back().value == 1);
        CHECK(P("myPalInArm_0_0").onDown == 1 && P("myPalInArm_0_0").offDown == 0);
        before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, false, true, why, (int)sizeof(why)) == (int)kVc8RcRefused);   // blow ON while vacuum ON (D3)
        CHECK(g.writes.size() == before && Has(why, "吸真空與破真空不同時開"));
        CHECK(W906_VacuumLiveDo(i, true, false, why, (int)sizeof(why)) == 0);
        CHECK(g.writes.size() == before + 1 && g.writes.back().chan == 17 && g.writes.back().value == 0);
        CHECK(P("myPalInArm_0_0").onDown == 0);
        CHECK(W906_VacuumLiveDo(i, false, true, why, (int)sizeof(why)) == 0 && g.writes.back().chan == 16 && g.writes.back().value == 1);
        CHECK(W906_VacuumLiveDo(i, false, false, why, (int)sizeof(why)) == 0 && g.writes.back().value == 0);
        before = g.writes.size();
        CHECK(W906_VacuumLiveDo(Find("myPalArm2_0_0"), true, true, why, (int)sizeof(why)) != 0);   // 0x50: not a VC8
        CHECK(g.writes.size() == before);                        // its read-back is unknown -> refused before a write
    }

    // ---------------------------------------------------------------- [10]
#ifdef W906_VC8_SUCKER_REMAP
    std::printf("-- [10] R3: golden SetIOTableByECAT_VC8_Sucker (W906_VC8_SUCKER_REMAP defined)\n");
    {
        TMySucker& fa = FTestSuck.Suck[0][0];
        fa.SuckerName = "FTestSuckAA"; fa.SensorName = "FTestSuckAA"; fa.OnPortName = "FTestSuckAA_On"; fa.OffPortName = "FTestSuckAA_Off";
        fa.OnRing = 1; fa.OnIP = 0x40; fa.OnPort = 17; fa.OnBit = 0; fa.OnType = 1; fa.OnISABase = 0;
        fa.OffRing = 1; fa.OffIP = 0x40; fa.OffPort = 16; fa.OffBit = 0; fa.OffType = 1; fa.OffISABase = 0;
        fa.SenRing = 1; fa.SenIP = 0x40; fa.SenPort = 128; fa.SenBit = 0; fa.SenType = 1; fa.SenISABase = 0; fa.ISABase = 0;
        TMySucker& ba = BTestSuck.Suck[0][0];                    // IO_Table.csv's BTestSuckAA: VC7 on 0x50 (chan 31 / 30)
        ba.SuckerName = "BTestSuckAA"; ba.SensorName = "BTestSuckAA"; ba.OnPortName = "BTestSuckAA_On"; ba.OffPortName = "BTestSuckAA_Off";
        ba.OnRing = 1; ba.OnIP = 0x50; ba.OnPort = 31; ba.OnBit = 7; ba.OnType = 1; ba.OnISABase = 3;
        ba.OffRing = 1; ba.OffIP = 0x50; ba.OffPort = 30; ba.OffBit = 7; ba.OffType = 1; ba.OffISABase = 3;
        ba.SenRing = 1; ba.SenIP = 0x50; ba.SenPort = 135; ba.SenBit = 7; ba.SenType = 1; ba.SenISABase = 3; ba.ISABase = 3;
        VCCU_UNIT_TYPE = 0;
        SetIOTableByECAT_VC8_Sucker();                           // golden: only when VCCU_UNIT_TYPE==1
        CHECK(fa.OnISABase == 0 && W906_Vc8GuardedSuckers() == 0 && P("myPalArm2_0_0").vc == 3);
        VCCU_UNIT_TYPE = 1;
        SetIOTableByECAT_VC8_Sucker();
        CHECK(fa.OnISABase == ePCI1203 && fa.OffISABase == ePCI1203 && fa.SenISABase == ePCI1203 && fa.ISABase == ePCI1203);
        const TVuLivePanel a = P("myPalArm1_0_0"), b = P("myPalArm2_0_0");
        CHECK(a.ip == 0x40 && a.vc == 0 && a.onPort == 17 && a.senPort == 128);
        CHECK(b.ip == 0x50 && b.vc == 7 && b.onPort == 31 && b.offPort == 30 && b.senPort == 135);   // the DB wins (golden)
        CHECK(W906_Vc8GuardedSuckers() >= 6);                    // the two set up here x 3 aliases (+ any default names)
        CHECK(W906_Vc8IsGuardedSucker(1, 0x50, "BTestSuckAA_On") && W906_Vc8IsGuardedSucker(1, 0x40, "FTestSuckAA"));
        CHECK(!W906_Vc8IsGuardedSucker(1, 0x50, "C_Load_Up_On") && !W906_Vc8IsGuardedSucker(1, 0x40, "BTestSuckAA_On"));
        CHECK(!W906_Vc8SuckerGate(1, 0x50, "BTestSuckAA_On", true));   // 0x50 is not a VC8 -> never written
        CHECK(!W906_Vc8SuckerGate(1, 0x50, "BTestSuckAA", false));     // ... and its "vacuum" is never read off that module
        CHECK(Has(W906_Vc8SuckerGateWhy(), "0x50"));
        CHECK(W906_Vc8SuckerGate(1, 0x50, "C_Load_Up_On", true));      // the loader cylinder on the same station: untouched
        g.vc8[0x50] = true;
        CHECK(W906_Vc8SuckerGate(1, 0x50, "BTestSuckAA_On", true));    // a real VC8 there: through
        g.vc8[0x50] = false;
        SetVc8Route(0);
        CHECK(!W906_Vc8SuckerGate(1, 0x40, "FTestSuckAA_On", true));   // no route: a registered sucker fails closed
        SetVc8Route(&kFake);
        //  AI(W906-VC8-IP) 20261001: the station is the IO_Table.csv sucker row's (EastSun 20261001: the modules moved to
        //  InArm 0xA0 / OutArm 0xA1 / Index 0xA2, set in the Sucker / Sucker_On / Sucker_Off rows, IP 160 / 161 / 162) --
        //  golden's ctor numbers 0x20 / 0x30 are overwritten by the copy, the gate registers the new station, the page
        //  writes there.
        TMySucker& ia = InArmSuck.Suck[0][0];
        ia.SuckerName = "InArmSuckA"; ia.SensorName = "InArmSuckA"; ia.OnPortName = "InArmSuckA_On"; ia.OffPortName = "InArmSuckA_Off";
        ia.OnRing = 1; ia.OnIP = 160; ia.OnPort = 17; ia.OnBit = 0; ia.OnType = 1; ia.OnISABase = 3;
        ia.OffRing = 1; ia.OffIP = 160; ia.OffPort = 16; ia.OffBit = 0; ia.OffType = 1; ia.OffISABase = 3;
        ia.SenRing = 1; ia.SenIP = 160; ia.SenPort = 128; ia.SenBit = 0; ia.SenType = 1; ia.SenISABase = 3; ia.ISABase = 3;
        TMySucker& oa = OutArmSuck.Suck[0][0];
        oa.SuckerName = "OutArmSuckA"; oa.SensorName = "OutArmSuckA"; oa.OnPortName = "OutArmSuckA_On"; oa.OffPortName = "OutArmSuckA_Off";
        oa.OnRing = 1; oa.OnIP = 161; oa.OnPort = 23; oa.OnBit = 7; oa.OnType = 1; oa.OnISABase = 3;   // IO_Table.csv as it is: OutArmSuckA_On 23 /
        oa.OffRing = 1; oa.OffIP = 161; oa.OffPort = 22; oa.OffBit = 7; oa.OffType = 1; oa.OffISABase = 3; //   _Off 22 (VC3's DO pair) with its sensor on 135 (VC7)
        oa.SenRing = 1; oa.SenIP = 161; oa.SenPort = 135; oa.SenBit = 7; oa.SenType = 1; oa.SenISABase = 3; oa.ISABase = 3;
        fa.OnIP = fa.OffIP = fa.SenIP = 162;
        SetIOTableByECAT_VC8_Sucker();
        CHECK(P("myPalInArm_0_0").ip == 0xA0 && P("myPalInArm_0_0").ring == 1 && P("myPalInArm_0_0").onPort == 17);
        CHECK(P("myPalOutArm_0_0").ip == 0xA1 && P("myPalOutArm_0_0").vc == 7 && P("myPalOutArm_0_0").onPort == 23 && P("myPalOutArm_0_0").offPort == 22);   // the row's Port wins (golden)
        CHECK(P("myPalArm1_0_0").ip == 0xA2);
        CHECK(W906_Vc8IsGuardedSucker(1, 0xA0, "InArmSuckA_On") && W906_Vc8IsGuardedSucker(1, 0xA1, "OutArmSuckA") && W906_Vc8IsGuardedSucker(1, 0xA2, "FTestSuckAA_Off"));
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "InArmSuckA_On", true));   // no VC8 answers at 0xA0 (the fake): refused
        g.vc8[0xA0] = true;
        CHECK(W906_Vc8SuckerGate(1, 0xA0, "InArmSuckA_On", true));    // an ECAT-VC8 at 0xA0: through
        const std::size_t before = g.writes.size();
        char why[320] = "";
        CHECK(W906_VacuumLiveDo(Find("myPalInArm_0_0"), true, true, why, (int)sizeof(why)) == 0);
        CHECK(g.writes.size() == before + 1 && g.writes.back().ip == 0xA0 && g.writes.back().chan == 17 && g.writes.back().value == 1);
        CHECK(W906_VacuumLiveDo(Find("myPalInArm_0_0"), true, false, why, (int)sizeof(why)) == 0 && g.writes.back().ip == 0xA0);
        g.vc8[0xA0] = false;
    }
#else
    std::printf("-- [10] R3 skipped: W906_VC8_SUCKER_REMAP is commented out in MachineType.h\n");
    SetIOTableByECAT_VC8_Sucker();
    CHECK(W906_Vc8GuardedSuckers() == 0);                          // the stub: nothing registered, nothing copied
#endif

    // ---------------------------------------------------------------- [11]
    std::printf("-- [11] Pci1203Control: the VC8 kinds are internal\n");
    {
        CHECK(Has(Pci1203CmdName(kCmdVc8DoSet), "(internal)") && Has(Pci1203CmdName(kCmdVc8SdoWriteI16), "(internal)"));
        CHECK(Pci1203CmdFromName("vacuum.vc8.doSetBit") == kCmdNone && Pci1203CmdFromName("vacuum.vc8.doSetBit (internal)") == kCmdNone);
        CHECK(Pci1203CmdFromName("vacuum.vc8.sdoWriteI16") == kCmdNone);
        CHECK((int)kCmdVc8DoSet == (int)kCmdAxTorqueLimitSet + 1 && (int)kCmdVc8SdoWriteI16 == (int)kCmdVc8DoSet + 1);
        TPci1203Control ctl;
        Pci1203Cmd c;
        c.kind = kCmdVc8SdoWriteI16; c.vc8Slave = 0; c.vc8Vc = 0; c.vc8Sub = 0x13; c.value = 100;
        Pci1203CmdResult r = ctl.Execute(c);
        CHECK(!r.accepted && !r.issued);
        c.kind = kCmdVc8DoSet; c.port = 0; c.bit = 1; c.value = 1;
        r = ctl.Execute(c);
        CHECK(!r.accepted && !r.issued);
    }

    // ---------------------------------------------------------------- [12]
    //  AI(W906-VC4) 20261001: a panel on a station that answers as an ECAT-VC4 (EastSun 1001 measured
    //  開真空 16+2VC / 破真空 17+2VC ; 「VC8 和 VC4 要不同分支」). AI(W906-VC4-ODD) 20261001: the parity is the VC8's after all --
    //  EastSun 21:3x 「箭頭往下的在吸」 (v = DO 19 sucks): make 17+2VC, break 16+2VC.
    std::printf("-- [12] an ECAT-VC4 panel: make 17+2VC, break 16+2VC, vacuum OK 64+VC\n");
    {
        FakeReset();
        SetVc8Route(&kFake);
        VCCU_UNIT_TYPE = 1;
        TMyVacuumPanel* p = fVacuumUnit->myPalInArm[2][0];          // golden InArm "E": iInOutVCSort[0][2] = VC2
        //  the row exactly as IO_Table.csv has it today, i.e. the VC8 layout (InArmSuckE: sensor 130, _On 21, _Off 20, Bit 2)
        p->OnRing = p->OffRing = p->SenRing = 1;
        p->OnIP = p->OffIP = p->SenIP = 0xA0;
        p->OnVCNo = p->OffVCNo = p->SenVCNo = 2;
        p->OnPort = 21; p->OffPort = 20; p->SenPort = 130;
        p->OnType = p->OffType = p->SenType = 1;
        p->OnISABase = p->OffISABase = p->SenISABase = ePCI1203;
        p->sEvent = "";
        const int i = Find("myPalInArm_2_0");
        CHECK(i >= 0);
        char why[320] = "";

        //  (a) nothing answers at 0xA0: golden's / the DB's ports, untouched
        CHECK(p->W906_Model() == (int)kVc8ModelNone && p->W906_OnChan() == 21 && p->W906_OffChan() == 20 && p->W906_SenChan() == 130);

        //  (b) an ECAT-VC8 there: exactly as before
        g.vc8[0xA0] = true;
        CHECK(p->W906_Model() == (int)kVc8ModelVc8 && p->W906_OnChan() == 21 && p->W906_OffChan() == 20 && p->W906_SenChan() == 130);
        std::size_t before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, true, true, why, (int)sizeof(why)) == 0);
        CHECK(g.writes.size() == before + 1 && g.writes.back().chan == 21 && g.writes.back().value == 1);
        CHECK(W906_VacuumLiveDo(i, true, false, why, (int)sizeof(why)) == 0 && g.writes.back().chan == 21 && g.writes.back().value == 0);
        g.di[0xA0][16] = 0x04;
        CHECK(p->ReadVaccumIO() && g.lastDiChan == 130);
        g.di[0xA0][16] = 0x00;

        //  (c) the same station answering as an ECAT-VC4
        g.vc4[0xA0] = true;
        CHECK(p->W906_Model() == (int)kVc8ModelVc4 && p->W906_OnChan() == 21 && p->W906_OffChan() == 20 && p->W906_SenChan() == 66);
        TVuLivePanel lp = P("myPalInArm_2_0");
        CHECK(lp.model == (int)kVc8ModelVc4 && lp.vc == 2 && lp.onPort == 21 && lp.offPort == 20 && lp.senPort == 66);
        before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, true, true, why, (int)sizeof(why)) == 0);                                 // ^ vacuum ON
        CHECK(g.writes.size() == before + 1 && g.writes.back().kind == 0 && g.writes.back().ip == 0xA0);
        CHECK(g.writes.back().chan == 21 && g.writes.back().value == 1 && g.writes.back().rc == 0);
        lp = P("myPalInArm_2_0");
        CHECK(lp.onDown == 1 && lp.offDown == 0);                                                              // read back on 21 / 20
        before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, false, true, why, (int)sizeof(why)) == (int)kVc8RcRefused && g.writes.size() == before);   // D3
        CHECK(W906_VacuumLiveDo(i, true, false, why, (int)sizeof(why)) == 0 && g.writes.back().chan == 21 && g.writes.back().value == 0);
        CHECK(W906_VacuumLiveDo(i, false, true, why, (int)sizeof(why)) == 0 && g.writes.back().chan == 20 && g.writes.back().value == 1);   // v blow ON
        lp = P("myPalInArm_2_0");
        CHECK(lp.offDown == 1 && lp.onDown == 0);
        CHECK(W906_VacuumLiveDo(i, false, false, why, (int)sizeof(why)) == 0 && g.writes.back().chan == 20 && g.writes.back().value == 0);
        CHECK(g.dout[0xA0][2] == 0x00 && g.dout[0xA0][3] == 0x00);                                            // nothing left on; byte 3 never touched
        //  the vacuum-OK bit: DI 64+VC = byte 8 bit 2 (not 128+VC)
        g.di[0xA0][8] = 0x04;
        CHECK(p->ReadVaccumIO() && g.lastDiChan == 66 && p->sEvent == "");
        g.di[0xA0][8] = 0x00;
        CHECK(!p->ReadVaccumIO() && p->sEvent == "");
        //  the pressure: VC2 = DI bytes 4 / 5 (golden GetIOValue Port*2 / Port*2+1 -- the same arithmetic on a VC4)
        g.di[0xA0][4] = 0x00; g.di[0xA0][5] = 0x40;
        CHECK(std::fabs(p->ReadVaccumCurrect() - VC8ToKpa(16384)) < 1e-9);
        CHECK(MyLaneIO.GetIOValue(1, 0xA0, 4, 0, ePCI1203) == 999.0);                                         // a VC4 has no VC 4: bytes 8 / 9 refused
        //  the threshold READ is allowed (VC 0..3)
        g.sdo[0xA0][2][1] = (short)KpaToVC8(-40.0);
        CHECK(std::fabs(MyLaneIO.GetIOValueThread(1, 0xA0, 2, 0, ePCI1203) - VC8ToKpa(KpaToVC8(-40.0))) < 1e-9);
        CHECK(MyLaneIO.GetIOValueThread(1, 0xA0, 4, 0, ePCI1203) == 999.0);                                   // 8040h: not on a VC4's list
        //  SDO WRITES: refused by default, and the mode write is NOT golden's Error5 (the panel does not freeze)
        p->bInitialThresholdModeOK = false;
        before = g.writes.size();
        p->InitialThresholdMode();
        CHECK(g.writes.size() == before + 1 && g.writes.back().kind == 1 && g.writes.back().chan == 0x8020 && g.writes.back().sub == 0x02);
#ifdef W906_VC4_SDO_WRITE
        CHECK(p->bInitialThresholdModeOK && g.writes.back().rc == 0);
#else
        CHECK(!p->bInitialThresholdModeOK && p->sEvent == "" && g.writes.back().rc == (int)kVc8RcVc4SdoOff);
        CHECK(!MyLaneIO.SetIOValueThread(-50.0, 1, 0xA0, 2, 0, ePCI1203));                                    // btnSVClick's write
        CHECK(g.writes.back().kind == 1 && g.writes.back().chan == 0x8020 && g.writes.back().sub == 0x13 && g.writes.back().rc == (int)kVc8RcVc4SdoOff);
        CHECK(g.sdo[0xA0][2][0] == 0 && g.sdo[0xA0][2][1] == (short)KpaToVC8(-40.0));                          // nothing written
#endif
        //  a VC above 3 on the VC4: channels outside its lists -- the read-back fails, so the press is refused unsent
        p->OnVCNo = p->OffVCNo = p->SenVCNo = 5;
        CHECK(p->W906_OnChan() == 27 && p->W906_OffChan() == 26 && p->W906_SenChan() == 69);
        before = g.writes.size();
        CHECK(W906_VacuumLiveDo(i, true, true, why, (int)sizeof(why)) != 0 && g.writes.size() == before);
        CHECK(!p->ReadVaccumIO() && p->sEvent == "Error3");                                                   // golden's own failure arm
        p->sEvent = "";
        p->OnVCNo = p->OffVCNo = p->SenVCNo = 2;

        //  the sucker gate on a VC4 (the engine and the IO page switch a sucker at its IO_Table Port verbatim)
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E_On",  kVc8RoleMake,   21, 130);    // today's IO_Table rows: the VC8 layout
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E_Off", kVc8RoleBreak,  20, 130);
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E",     kVc8RoleSensor, 130, 130);
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_On", true));                                               // VC4-ODD: 21 makes, but its sensor 130 is no VC4 input
        CHECK(Has(W906_Vc8SuckerGateWhy(), "ECAT-VC4") && Has(W906_Vc8SuckerGateWhy(), "Port 21"));
        std::printf("   VC4T_E_On (Port 21) -> %s\n", W906_Vc8SuckerGateWhy());
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_Off", true) && !W906_Vc8SuckerGate(1, 0xA0, "VC4T_E", false));
        const int nGuard = W906_Vc8GuardedSuckers();
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E_On",  kVc8RoleMake,   20, 66);     // VC4-ODD: _On on the EVEN channel = the break -> refused
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_On", true) && Has(W906_Vc8SuckerGateWhy(), "Port 20"));
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E_On",  kVc8RoleMake,   21, 66);     // the VC4 rows (IO_Table 1001 21:4x: _On odd, sensor 64+VC)
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E_Off", kVc8RoleBreak,  20, 66);
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_E",     kVc8RoleSensor, 66, 66);
        CHECK(W906_Vc8GuardedSuckers() == nGuard);                                                             // re-registered in place
        CHECK(W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_On", true) && W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_Off", true));
        CHECK(W906_Vc8SuckerGate(1, 0xA0, "VC4T_E", false));
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_X_On", kVc8RoleMake, 23, 64);        // DO pair of VC3, sensor of VC0 (the OutArm crossing)   VC4-ODD: make 23
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_X_On", true) && Has(W906_Vc8SuckerGateWhy(), "VC"));
        W906_Vc8GuardSuckerAs(1, 0xA0, "VC4T_Y", kVc8RoleSensor, 68, 68);          // sensor of a VC4's VC4 (does not exist)
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_Y", false));
        W906_Vc8GuardSucker(1, 0xA0, "VC4T_N_On");                                 // no role: cannot be checked on a VC4
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_N_On", true));
        CHECK(W906_Vc8SuckerGate(1, 0xA0, "VC4T_unregistered", true));             // an alias nobody registered: untouched
        g.vc4[0xA0] = false;                                                       // an ECAT-VC8 there: the old rule (the station only)
        CHECK(W906_Vc8SuckerGate(1, 0xA0, "VC4T_N_On", true) && W906_Vc8SuckerGate(1, 0xA0, "VC4T_X_On", true));
        g.vc8[0xA0] = false;                                                       // nothing there: every registered alias refused
        CHECK(!W906_Vc8SuckerGate(1, 0xA0, "VC4T_E_On", true));
    }

    W906_VacuumLiveClose();
    SetVc8Route(0);
    std::printf("\n%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
