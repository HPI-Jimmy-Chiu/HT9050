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
//        only on a VC8, an unregistered one (the loader cylinder sharing the station) always passes
//   [11] Pci1203Control: the two VC8 kinds are INTERNAL (no wire name, audit name "(internal)"), refused on a
//        closed control
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
    bool          vc8[256];
    unsigned char di[256][17];
    unsigned char dout[256][4];
    short         sdo[256][8][2];      // [station][vc][0 = mode 02h, 1 = threshold 13h]
    std::vector<FakeWrite> writes;
    unsigned long seq;
    TVc8Write     ring[128];
} g;

static void FakeReset()
{
    std::memset(g.vc8, 0, sizeof(g.vc8)); std::memset(g.di, 0, sizeof(g.di)); std::memset(g.dout, 0, sizeof(g.dout));
    std::memset(g.sdo, 0, sizeof(g.sdo)); g.writes.clear(); g.seq = 0; std::memset(g.ring, 0, sizeof(g.ring));
}
static int FRc(int ring, int ip) { return (ring == 1 && ip > 0 && ip < 256 && g.vc8[ip]) ? 0 : (int)kVc8RcNotVc8; }
static int FReadAi(int ring, int ip, int port, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    if (port < 0 || port > 15) return (int)kVc8RcBadAddr;
    *v = g.di[ip][port]; return 0;
}
static int FReadDi(int ring, int ip, int chan, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    if (chan < 128 || chan > 135) return (int)kVc8RcBadAddr;
    *v = (unsigned char)((g.di[ip][16] >> (chan - 128)) & 1); return 0;
}
static int FReadDo(int ring, int ip, int chan, unsigned char* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    if (chan < 16 || chan > 31) return (int)kVc8RcBadAddr;
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
    const int rc = FRc(ring, ip);
    FRecord(0, ring, ip, chan, -1, v, rc);
    if (rc) return rc;
    if (v) g.dout[ip][chan / 8] |= (unsigned char)(1u << (chan % 8)); else g.dout[ip][chan / 8] &= (unsigned char)~(1u << (chan % 8));
    return 0;
}
static int FReadSdo(int ring, int ip, int index, int sub, short* v)
{
    const int rc = FRc(ring, ip); if (rc) return rc;
    const int vc = (index - 0x8000) / 0x10;
    *v = g.sdo[ip][vc][sub == 0x02 ? 0 : 1]; return 0;
}
static int FWriteSdo(int ring, int ip, int index, int sub, short v)
{
    const int rc = FRc(ring, ip);
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
static const TVc8Route kFake = { FReadAi, FReadDi, FReadDo, FWriteDo, FReadSdo, FWriteSdo, FCheck, FLastWhy, FLastSeq, FWritesSince };

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

    W906_VacuumLiveClose();
    SetVc8Route(0);
    std::printf("\n%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
