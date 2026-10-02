// =============================================================================
//  EtherCAT/Pci1203Vc8.h -- WHICH station is an ECAT-VC8, and what may be sent to one.
//
//  AI(W906-VACUNIT-1203) 20260930: new file (EastSun 20260930, see VacuumUnit/Vc8Route.h). Pure: no
//  vendor call, no vendor header, both build arms. The three places that must agree all ask it:
//      EtherCAT/Pci1203Vc8Route.inc   the route -- every read and every write asks it first
//      EtherCAT/Pci1203Control.cpp    kCmdVc8DoSet / kCmdVc8SdoWriteI16 ask again, on the sample Execute uses
//      EtherCAT/Pci1203Monitor.cpp    Vc8SdoReadI16 -- the SDO read list
//  and tests/test_vacuum_vc8.cpp pins it.
//
//  WHY AN IDENTITY CHECK, AND NOT "THE STATION ANSWERS"
//    golden addresses the VC8 by station number and nothing else (golden VacuumUnit/MyVacuumPanel.cpp:54-149:
//    InArm 0x20, OutArm 0x30, IndexArm1 0x40/0x41, IndexArm2 0x50/0x51). On THIS machine ring 1 stations
//    0x50..0x54 are ECx-P32-HON 32DI modules (the tray sensors, D:\HT9045\system\IO_Table.csv Enable=1), in
//    OP, with DI bytes in the card map -- so "present, OP, mapped" is TRUE for golden's IndexArm2 address and
//    a DO / SDO write there would reach a module that is not a vacuum unit. Only the module's own identity
//    (ADV_SLAVE_INFO.Name / VendorID / ProductID, read once by the monitor's scan, Pci1203SlaveSample) can
//    tell them apart. So a station is a VC8 only when it matches an entry of the table below.
//
//  THE TABLE IS DELIBERATELY ONE LINE. Nobody on this project has seen the module's scan name yet (no VC8 has
//  been on this ring). golden's own HandlerSystem radio calls the option "ECAT-VC8_ODM1" (FileRW/HSys.gen.inc,
//  rgVacuUnitType) and cpublic.cpp:2533 credits the kPa formula to 泓格 (ICP DAS), whose modules name
//  themselves "ECAT-xxxx" -- hence "ECAT-VC8". vendorId / productId 0 = not pinned yet. A module that answers
//  with any other name is REFUSED, and the refusal quotes the name it did answer with, so EastSun can tell us
//  which line to add (and its VendorID / ProductID, which the pci1203 page shows for every station).
//
//  AI(W906-VC4) 20261001: THE TABLE IS TWO LINES NOW, AND EACH LINE NAMES A MODEL. The modules EastSun fitted on
//  ring 1 at 160 / 161 / 162 (0xA0..0xA2) answer "ECAT-VC4-ODM1" (ICP DAS, VendorID 0x00494350, ProductID
//  0x00A20401 -- runcfg/logs/oplog_20261001.txt ~:2304, the refusal that quoted it). EastSun 20261001 measured on
//  the machine: 「開真空是16bit 破真空17bit / 開真空是18bit 破真空19bit / 開真空是20bit 破真空21bit /
//  開真空是22bit 破真空23bit」 -- on the VC4, VC n MAKES vacuum on DO 16+2n (EVEN) and BREAKS it on 17+2n (ODD),
//  the opposite of the VC8 manual (DO even = break, odd = make) -- and ruled 「VC8 和 VC4 要不同分支」.
//  ⚠ AI(W906-VC4-ODD) 20261001: SUPERSEDED for the make / break parity. EastSun 21:3x on the Vacuum Unit 「箭頭往下的在吸」:
//  v (bplOff) wrote DO 19 (0xA0 VC1) and that SUCKS -- the VC4 makes vacuum on the ODD channel 17+2n, like the VC8.
//  The branch stays (4 VCs, OK DI 64+VC, DO byte 2 only, SDO VC 0..3); only the row below changed.
//  So the identity check returns a MODEL, and every closed list below is the model's (Pci1203VacLayoutOf):
//                       VC8 (golden, unchanged)          VC4 (EastSun 20261001; half of the VC8)
//      VC / SDO VC      0..7                             0..3
//      DI bytes         17 (0..15 pressures, 16 OK bits) 9 (0..7 pressures, 8 OK bits)
//      vacuum-OK DI     128+VC                           64+VC
//      DO channels      16..31 (DO bytes 2..3)           16..23 (DO byte 2)
//      make vacuum      ODD  of the VC's pair (17+2VC)   ODD  of the VC's pair (17+2VC) (was EVEN until VC4-ODD)
//      SDO writes       allowed                          REFUSED unless W906_VC4_SDO_WRITE (below; ON since VC4-SDO) -- no VC4 manual
//                                                        here, the object dictionary is unverified; reads allowed
//  The 4-argument / 2-argument checks that predate the VC4 (Pci1203Vc8SdoAddrOk, Pci1203Vc8DoChanOk) are the VC8
//  table, word for word as before; EtherCAT/Pci1203Monitor.cpp keeps calling them (its SDO read is reached only
//  through the route, which checks the station's own model first).
// =============================================================================
#ifndef ETHERCAT_PCI1203VC8_H
#define ETHERCAT_PCI1203VC8_H

#include <cstdarg>
#include <cstdio>
#include <string>

#include "EtherCAT/Pci1203Monitor.h"   // Pci1203SlaveSample, TPci1203Monitor (public accessors only)

// ---------------------------------------------------------------------------------------------------------
//  AI(W906-VC4) 20261001: the ECAT-VC4's SDO WRITE switch. OFF by default (commented out): the threshold MODE
//  (8000h+10h*VC :02h) and THRESHOLD (:13h) writes are refused for a VC4 station, with the reason, by the route
//  (EtherCAT/Pci1203Vc8Route.inc) and again by Pci1203Control (Vc8Validate_). Reads of the same objects are allowed.
//  Uncomment it HERE, not with -D: every TU that includes this header must see the same Pci1203Vc8SdoWriteOk.
//  Only after EastSun has confirmed on a VC4 that 8000h+10h*VC :02h / :13h are its threshold-mode / threshold
//  objects (the VC8 manual's layout, assumed per channel for VC 0..3).
//  AI(W906-VC4-SDO) 20261001: ON. EastSun 1001 (asked whether to open it): 「不要擋了 / VC8 和VC4 不是跟你說分支了嗎 /
//  VC4 沒有的 用VC8 去推論」 -- the VC4 takes the VC8 manual's object table (§3.2 80n0:02 Enable Threshold, 80n0:13
//  Threshold Value INT16) for its VC 0..3. Evidence it fits: the read of 80n0:13h on 0xA0..0xA2 succeeds (the panels
//  show VC8ToKpa(0) = -116.0, the manual's default 0; a missing object would abort and show 999.0). VC 4..7 stay
//  refused (Pci1203Vc8SdoAddrOkFor). A write that the module rejects comes back as a card error, not silently.
// ---------------------------------------------------------------------------------------------------------
#define W906_VC4_SDO_WRITE

namespace ht9045 {

// AI(W906-VC4) 20261001: the vacuum-unit MODEL a station's identity names. The value is the VC count.
enum { kPci1203VacNone = 0, kPci1203VacVc4 = 4, kPci1203VacVc8 = 8 };

struct Pci1203Vc8Identity {
    const char*   nameHas;     // case-insensitive substring of ADV_SLAVE_INFO.Name
    unsigned long vendorId;    // 0 = not checked (not pinned yet)
    unsigned long productId;   // 0 = not checked
    const char*   source;      // why this line is here
    int           model;       // AI(W906-VC4) 20261001: kPci1203VacVc8 / kPci1203VacVc4 -- which layout table applies
};

inline const Pci1203Vc8Identity* Pci1203Vc8Identities(int* n)
{
    static const Pci1203Vc8Identity k[] = {
        { "ECAT-VC8", 0ul, 0ul, "golden rgVacuUnitType item 'ECAT-VC8_ODM1' (FileRW/HSys.gen.inc); unconfirmed on a real module", kPci1203VacVc8 },
        //AI(W906-VC4) 20261001: EastSun 1001 measured ... ; 「VC8 和 VC4 要不同分支」. VendorID pinned (ICP DAS, as 0xA0
        //  answered); ProductID 0x00A20401 seen on 0xA0 only, so not pinned (161 / 162 were not quoted yet).
        { "ECAT-VC4", 0x00494350ul, 0ul, "ring 1 0xA0 'ECAT-VC4-ODM1' VendorID 0x00494350 ProductID 0x00A20401 (oplog_20261001 ~:2304); EastSun 20261001", kPci1203VacVc4 },
    };
    if (n) *n = (int)(sizeof(k) / sizeof(k[0]));
    return k;
}

enum {
    kPci1203Vc8Ring     = 1,       // golden MyVacuumPanel.cpp:42-44 iOnRing = iOffRing = iSenRing = 1
    kPci1203Vc8Units    = 8,       // VC 0..7 per module (the sort tables, golden :28-35)
    kPci1203Vc8DoFirst  = 16,      // golden :231 / :250 16+VCNo*2+swap -> 16..31 ("ECAT-VC8 DO0-DO15", :472)
    kPci1203Vc8DoLast   = 31,
    kPci1203Vc8DiOkChan = 128,     // golden :206 128+VCNo -> 128..135 = station byte 16 ("DI0-DI7 128-135", :536)
    kPci1203Vc8DiBytes  = 17,      // bytes 0..15 = the eight pressures (golden MyLaneIo.cpp GetIOValue Port*2 / Port*2+1),
                                   //   byte 16 = the vacuum-OK bits
    kPci1203Vc8SdoBase  = 0x8000,  // golden :260 Threshold_Index
    kPci1203Vc8SdoStep  = 0x10,    //   + VCNo*0x0010 (golden :564, MyLaneIo.cpp :624-703 Port*0x0010)
    kPci1203Vc8SubMode  = 0x02,    // golden :262 ThresholdMode_SubIndex; golden writes 1 (:561 "1:低於設定閥值")
    kPci1203Vc8SubThr   = 0x13,    // golden :261 Threshold_SubIndex
    kPci1203Vc8RawMax   = 32767,   // KpaToVC8's output range 0..32767 (cpublic.cpp:2544) = -116..148 kPa
    kPci1203EcStatePreOp  = 0x02,  // EC_SLAVE_STATE_* (EtherCAT/vendor/AdvMotDrv.h:2168-2173), low nibble
    kPci1203EcStateSafeOp = 0x04,
    kPci1203EcStateOp     = 0x08,
    //AI(W906-VC4) 20261001: the ECAT-VC4 (EastSun 1001 measured ... ; 「VC8 和 VC4 要不同分支」): half of the VC8.
    kPci1203Vc4Units    = 4,       // VC 0..3 (the card map holds DI stationChan 0..8 and DO stationChan 0..2 for 0xA0..0xA2)
    kPci1203Vc4DoLast   = 23,      // DO 16..23 = station byte 2 (make 17+2VC / break 16+2VC -- AI(W906-VC4-ODD) 20261001)
    kPci1203Vc4DiOkChan = 64,      // 64+VC = station byte 8 (DI0..DI3 threshold bits)
    kPci1203Vc4DiBytes  = 9        // bytes 0..7 = the four pressures (VC n in bytes 2n / 2n+1), byte 8 = the OK bits
};

// AI(W906-VC4) 20261001: ONE MODEL'S CLOSED LISTS. Every route / control check takes its numbers from here.
//   The DO bytes the card map must hold are doFirst/8 .. doLast/8 (VC8 2..3, VC4 2); the SDO VC range is 0..units-1.
struct Pci1203VacLayout {
    int         model;       // kPci1203VacVc8 / kPci1203VacVc4
    const char* name;        // "ECAT-VC8" / "ECAT-VC4" (messages)
    int         units;       // VCs per module = the SDO VC range
    int         diBytes;     // station DI bytes: 2 per VC (pressure, low / high) + 1 (the vacuum-OK bits)
    int         diOkChan;    // first vacuum-OK DI channel (VC n -> diOkChan+n) = 8 * (diBytes-1)
    int         doFirst;     // vacuum / blow DO channels doFirst..doLast; VC n owns doFirst+2n and doFirst+2n+1
    int         doLast;
    int         makeOdd;     // 1: the ODD channel of a VC's pair MAKES vacuum (VC8 manual: even = break, odd = make)
                             // 0: the EVEN one makes it (no model today: the VC4 was 0 until AI(W906-VC4-ODD) 20261001)
    const char* source;
};

// The layout of `model`. Callers pass a model the identity check returned; anything else gets the VC8 table, which is
// what every check written before the VC4 assumed (so the old entry points below are exactly what they were).
inline const Pci1203VacLayout& Pci1203VacLayoutOf(int model)
{
    static const Pci1203VacLayout kVc8 = { kPci1203VacVc8, "ECAT-VC8", kPci1203Vc8Units, kPci1203Vc8DiBytes, kPci1203Vc8DiOkChan,
                                           kPci1203Vc8DoFirst, kPci1203Vc8DoLast, 1,
                                           "golden MyVacuumPanel.cpp:206 128+VC / :231 :250 16+VC*2+swap (swap 1: On = odd); VC8 manual" };
    //AI(W906-VC4-ODD) 20261001: makeOdd 0 -> 1. EastSun 1001 21:3x on the Vacuum Unit 「箭頭往下的在吸」: v (bplOff) wrote
    //  DO 19 on 0xA0 VC1 (oplog 21:06:52 `chan 19 = 1 -> ISSUED`) and that one SUCKS -- the VC4 makes vacuum on the ODD
    //  channel like the VC8 manual. The earlier 「開真空是16bit 破真空17bit」 reading had it the other way round.
    static const Pci1203VacLayout kVc4 = { kPci1203VacVc4, "ECAT-VC4", kPci1203Vc4Units, kPci1203Vc4DiBytes, kPci1203Vc4DiOkChan,
                                           kPci1203Vc8DoFirst, kPci1203Vc4DoLast, 1,
                                           "EastSun 20261001 on the machine: make 17+2VC (odd) / break 16+2VC (even), as the VC8; DI 9 / DO 3 bytes in the card map" };
    return model == (int)kPci1203VacVc4 ? kVc4 : kVc8;
}

// The DO channel that MAKES / BREAKS vacuum on VC `vc`, and the vacuum-OK DI channel, for `model`.
inline int Pci1203Vc8MakeChan(int model, int vc)  { const Pci1203VacLayout& L = Pci1203VacLayoutOf(model); return L.doFirst + 2 * vc + (L.makeOdd ? 1 : 0); }
inline int Pci1203Vc8BreakChan(int model, int vc) { const Pci1203VacLayout& L = Pci1203VacLayoutOf(model); return L.doFirst + 2 * vc + (L.makeOdd ? 0 : 1); }
inline int Pci1203Vc8OkChan(int model, int vc)    { return Pci1203VacLayoutOf(model).diOkChan + vc; }

// What the caller is about to do with the station: it decides the EtherCAT state required.
enum Pci1203Vc8Use {
    kVc8UseReadIo  = 0,   // a DI / DO sample: SAFEOP or OP (inputs update, Pci1203SlaveInputsLive's rule)
    kVc8UseReadSdo = 1,   // an SDO read: PREOP / SAFEOP / OP (CoE mailbox, Pci1203SlaveMailboxOk's rule)
    kVc8UseWrite   = 2,   // any write: OP only (outputs update only in OP)
    kVc8UseIdentity = 3   // AI(W906-VC4) 20261001: only WHICH model the station is (no state asked; nothing is sent)
};

inline bool Pci1203Vc8NameHas(const std::string& name, const char* has)
{
    if (!has || !*has) return false;
    const std::size_t n = name.size();
    std::size_t m = 0;
    while (has[m]) ++m;
    for (std::size_t i = 0; i + m <= n; ++i) {
        std::size_t k = 0;
        for (; k < m; ++k) {
            char a = name[i + k], b = has[k];
            if (a >= 'a' && a <= 'z') a = (char)(a - 'a' + 'A');
            if (b >= 'a' && b <= 'z') b = (char)(b - 'a' + 'A');
            if (a != b) break;
        }
        if (k == m) return true;
    }
    return false;
}

inline std::string Pci1203Vc8Fmt(const char* f, ...)
{
    char b[640];   // AI(W906-VC4) 20261001: 400 -> 640, the identity refusal now lists both table lines (UTF-8 text)
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(b, sizeof(b), f, ap);
    va_end(ap);
    return b;
}

// Is this ONE sample an ECAT-VC8? The drive tests come first because they are the ones that must never be
// argued with: a CiA 402 profile or a SERVOPACK name is a motion drive whatever else it says.
// AI(W906-VC4) 20261001: ... or an ECAT-VC4 -- `model` (optional) gets the matched line's model, kPci1203VacNone
// on a refusal. The first matching line wins (the two names cannot both match: "ECAT-VC8" / "ECAT-VC4").
inline bool Pci1203Vc8IdentityOk(const Pci1203SlaveSample& s, std::string& why, int* model = 0)
{
    if (model) *model = kPci1203VacNone;
    if (!s.infoValid) {
        why = Pci1203Vc8Fmt("ring %d 站 0x%02X 的身分（ADV_SLAVE_INFO）沒有讀到，無法確認它是 ECAT-VC8——拒絕", s.ring, s.addr);
        return false;
    }
    if ((s.profileValid && s.profile == 402) || Pci1203Vc8NameHas(s.name, "SERVOPACK")) {
        why = Pci1203Vc8Fmt("ring %d 站 0x%02X 是馬達驅動器（%s%s）不是 ECAT-VC8——拒絕", s.ring, s.addr,
                            s.name.c_str(), (s.profileValid && s.profile == 402) ? "，CiA 402" : "");
        return false;
    }
    int n = 0;
    const Pci1203Vc8Identity* t = Pci1203Vc8Identities(&n);
    for (int i = 0; i < n; ++i) {
        if (!Pci1203Vc8NameHas(s.name, t[i].nameHas)) continue;
        if (t[i].vendorId  != 0ul && t[i].vendorId  != s.vendorId)  continue;
        if (t[i].productId != 0ul && t[i].productId != s.productId) continue;
        if (model) *model = t[i].model;   // AI(W906-VC4) 20261001
        return true;
    }
    std::string names;   // AI(W906-VC4) 20261001: every line of the table, so the refusal says what WOULD be accepted
    for (int i = 0; i < n; ++i)
        names += Pci1203Vc8Fmt("%s「%s」%s", i ? "或" : "", t[i].nameHas,
                               t[i].vendorId != 0ul ? Pci1203Vc8Fmt("（VendorID 0x%08lX）", t[i].vendorId).c_str() : "");
    why = Pci1203Vc8Fmt("ring %d 站 0x%02X 回報的模組是「%s」（VendorID 0x%08lX, ProductID 0x%08lX），不在 ECAT-VC8／VC4 身分表"
                        "（EtherCAT/Pci1203Vc8.h：名稱要含%s）——拒絕。若這真的是真空單元，請把上面這個名稱告訴我們",
                        s.ring, s.addr, s.name.c_str(), s.vendorId, s.productId, names.empty() ? "?" : names.c_str());
    return false;
}

// The EtherCAT state `use` needs (low nibble of EC_SLAVE_STATE_*).
inline bool Pci1203Vc8StateOk(const Pci1203SlaveSample& s, int use, std::string& why)
{
    const unsigned st = (unsigned)s.state & 0x0Fu;
    bool ok = false;
    const char* need = "";
    if (use == kVc8UseIdentity) return true;   // AI(W906-VC4) 20261001: no state asked (nothing is sent)
    if (!s.stateValid) {
        why = Pci1203Vc8Fmt("ring %d 站 0x%02X 這一輪的 EtherCAT 狀態沒有讀到——拒絕", s.ring, s.addr);
        return false;
    }
    if (use == kVc8UseWrite)        { ok = (st == kPci1203EcStateOp); need = "OP"; }
    else if (use == kVc8UseReadSdo) { ok = (st == kPci1203EcStatePreOp || st == kPci1203EcStateSafeOp || st == kPci1203EcStateOp); need = "PREOP/SAFEOP/OP"; }
    else                            { ok = (st == kPci1203EcStateSafeOp || st == kPci1203EcStateOp); need = "SAFEOP/OP"; }
    if (!ok) why = Pci1203Vc8Fmt("ring %d 站 0x%02X 的 EtherCAT 狀態是 0x%02X，要 %s 才行——拒絕", s.ring, s.addr, st, need);
    return ok;
}

// Exactly one present sample at (ring, addr). `get(i)` returns the i-th sample (const Pci1203SlaveSample&).
// Returns the slot, or -1 + why (none / two of them -- under the SubDevice ID conflict two stations can
// answer one address, and a write must not pick one).
template <class Get>
inline int Pci1203Vc8FindSlaveBy(Get get, int n, int ring, int addr, std::string& why)
{
    int found = -1, count = 0;
    std::string seen;
    int seenN = 0;
    for (int i = 0; i < n; ++i) {
        const Pci1203SlaveSample& s = get(i);
        if (!s.present) continue;
        if (s.ring == ring && seenN < 12) {
            seen += Pci1203Vc8Fmt("%s0x%02X", seenN ? " " : "", s.addr);
            ++seenN;
        }
        if (s.ring != ring || s.addr != addr) continue;
        ++count;
        if (found < 0) found = i;
    }
    if (count == 0) {
        why = Pci1203Vc8Fmt("ring %d 上沒有站 0x%02X（掃描到的 ring %d 站：%s）——沒有 ECAT-VC8 可以讀寫", ring, addr, ring,
                            seen.empty() ? "無" : seen.c_str());
        return -1;
    }
    if (count > 1) {
        why = Pci1203Vc8Fmt("ring %d 站 0x%02X 有 %d 個模組回應（站號衝突），分不出哪一個是 ECAT-VC8——拒絕", ring, addr, count);
        return -1;
    }
    return found;
}

// Everything about the STATION: ring 1, found once, addressable, a VC8 by identity, in the state `use` needs.
// AI(W906-VC4) 20261001: ... or a VC4; `model` (optional) gets which, kPci1203VacNone on any refusal.
template <class Get>
inline bool Pci1203Vc8StationOkBy(Get get, int n, int ring, int addr, int use, int* slot, std::string& why, int* model = 0)
{
    if (slot) *slot = -1;
    if (model) *model = kPci1203VacNone;
    if (ring != kPci1203Vc8Ring) {
        why = Pci1203Vc8Fmt("ECAT-VC8 只在 ring %d（golden MyVacuumPanel.cpp:42-44），這裡要的是 ring %d——拒絕", kPci1203Vc8Ring, ring);
        return false;
    }
    if (addr <= 0) {
        why = Pci1203Vc8Fmt("站號 %d 不能用——拒絕", addr);
        return false;
    }
    const int k = Pci1203Vc8FindSlaveBy(get, n, ring, addr, why);
    if (k < 0) return false;
    const Pci1203SlaveSample& s = get(k);
    if (s.unaddressable) {
        why = Pci1203Vc8Fmt("ring %d 站 0x%02X 在線上但這個位址指到的是另一個模組（站號衝突，unaddressable）——拒絕", ring, addr);
        return false;
    }
    int m = kPci1203VacNone;
    if (!Pci1203Vc8IdentityOk(s, why, &m)) return false;
    if (!Pci1203Vc8StateOk(s, use, why)) return false;
    if (slot) *slot = k;
    if (model) *model = m;
    return true;
}

inline bool Pci1203Vc8StationOk(const Pci1203SlaveSample* sl, int n, int ring, int addr, int use, int* slot, std::string& why, int* model = 0)
{
    return Pci1203Vc8StationOkBy([sl](int i) -> const Pci1203SlaveSample& { return sl[i]; }, n, ring, addr, use, slot, why, model);
}

inline bool Pci1203Vc8StationOkOn(const TPci1203Monitor& mon, int ring, int addr, int use, int* slot, std::string& why, int* model = 0)
{
    return Pci1203Vc8StationOkBy([&mon](int i) -> const Pci1203SlaveSample& { return mon.slave(i); },
                                 mon.slaveCount(), ring, addr, use, slot, why, model);
}

// AI(W906-VC4) 20261001: WHICH MODEL the station at (ring, addr) is, by identity only -- for the panel side to pick
// its channel layout before it asks (VacuumUnit/Vc8Route.h W906_Vc8Model). The same station rules as above (ring 1,
// one present sample, addressable, not a drive, in the table), no EtherCAT state, no reason string built on the way
// in (it runs per panel per tick). kPci1203VacNone for anything that would be refused. Every read / write that
// follows is checked in full again.
template <class Get>
inline int Pci1203Vc8StationModelBy(Get get, int n, int ring, int addr)
{
    if (ring != (int)kPci1203Vc8Ring || addr <= 0) return kPci1203VacNone;
    int found = -1, count = 0;
    for (int i = 0; i < n; ++i) {
        const Pci1203SlaveSample& s = get(i);
        if (!s.present || s.ring != ring || s.addr != addr) continue;
        ++count;
        if (found < 0) found = i;
    }
    if (count != 1) return kPci1203VacNone;
    const Pci1203SlaveSample& s = get(found);
    if (s.unaddressable) return kPci1203VacNone;
    std::string why;
    int m = kPci1203VacNone;
    return Pci1203Vc8IdentityOk(s, why, &m) ? m : (int)kPci1203VacNone;
}

inline int Pci1203Vc8StationModel(const Pci1203SlaveSample* sl, int n, int ring, int addr)
{
    return Pci1203Vc8StationModelBy([sl](int i) -> const Pci1203SlaveSample& { return sl[i]; }, n, ring, addr);
}

inline int Pci1203Vc8StationModelOn(const TPci1203Monitor& mon, int ring, int addr)
{
    return Pci1203Vc8StationModelBy([&mon](int i) -> const Pci1203SlaveSample& { return mon.slave(i); }, mon.slaveCount(), ring, addr);
}

// The SDO closed list: 8000h + VC*10h (VC 0..7), subindex 02h (mode) or 13h (threshold). `vc` out.
// AI(W906-VC4) 20261001: ...of `model` (VC8 0..7, VC4 0..3 -- the VC8's objects assumed per channel, no VC4 manual).
inline bool Pci1203Vc8SdoAddrOkFor(int model, int index, int sub, int* vc, std::string& why)
{
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(model);
    if (vc) *vc = -1;
    const int off = index - (int)kPci1203Vc8SdoBase;
    if (off < 0 || (off % (int)kPci1203Vc8SdoStep) != 0 || off / (int)kPci1203Vc8SdoStep >= L.units) {
        why = Pci1203Vc8Fmt("SDO 物件 %04Xh 不在 %s 的清單（8000h..%04Xh，每 10h 一個 VC）——拒絕", (unsigned)index & 0xFFFFu, L.name,
                            (unsigned)((int)kPci1203Vc8SdoBase + (int)kPci1203Vc8SdoStep * (L.units - 1)));
        return false;
    }
    if (sub != (int)kPci1203Vc8SubMode && sub != (int)kPci1203Vc8SubThr) {
        why = Pci1203Vc8Fmt("SDO 子索引 %02Xh 不在清單（02h 閥值模式、13h 閥值）——拒絕", (unsigned)sub & 0xFFu);
        return false;
    }
    if (vc) *vc = off / (int)kPci1203Vc8SdoStep;
    return true;
}

// The VC8's list, as before the VC4 (EtherCAT/Pci1203Monitor.cpp Vc8SdoReadI16; the route's pre-check).
inline bool Pci1203Vc8SdoAddrOk(int index, int sub, int* vc, std::string& why)
{
    return Pci1203Vc8SdoAddrOkFor(kPci1203VacVc8, index, sub, vc, why);
}

// AI(W906-VC4) 20261001: may an SDO WRITE go to a station of `model`? VC8 yes; VC4 only with W906_VC4_SDO_WRITE.
inline bool Pci1203Vc8SdoWriteOk(int model, std::string& why)
{
    if (model != (int)kPci1203VacVc4) return true;
#ifdef W906_VC4_SDO_WRITE
    (void)why;
    return true;
#else
    why = "ECAT-VC4 的 SDO 寫入（閥值模式 02h／閥值 13h）預設關閉：這裡沒有 VC4 手冊，物件表沒有驗證過"
          "（EtherCAT/Pci1203Vc8.h 的 W906_VC4_SDO_WRITE 沒有定義）——拒絕，沒有送出；讀取照常";
    return false;
#endif
}

// The value a WRITE may carry: 02h -> exactly 1 (golden :561); 13h -> 0..32767 (KpaToVC8 of -116..148 kPa).
inline bool Pci1203Vc8SdoValueOk(int sub, long value, std::string& why)
{
    if (sub == (int)kPci1203Vc8SubMode) {
        if (value == 1) return true;
        why = Pci1203Vc8Fmt("閥值模式（02h）只寫 golden 的 1（低於設定閥值），不寫 %ld——拒絕", value);
        return false;
    }
    if (sub == (int)kPci1203Vc8SubThr) {
        if (value >= 0 && value <= (long)kPci1203Vc8RawMax) return true;
        why = Pci1203Vc8Fmt("閥值（13h）原始值 %ld 超出 0..%d（-116..148 kPa）——拒絕", value, (int)kPci1203Vc8RawMax);
        return false;
    }
    why = "不認得的子索引——拒絕";
    return false;
}

// AI(W906-VC4) 20261001: the DO closed list of `model` (VC8 16..31, VC4 16..23).
inline bool Pci1203Vc8DoChanOkFor(int model, int chan, std::string& why)
{
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(model);
    if (chan >= L.doFirst && chan <= L.doLast) return true;
    why = Pci1203Vc8Fmt("DO 通道 %d 不是 %s 的吸／破真空通道（%d..%d）——拒絕", chan, L.name, L.doFirst, L.doLast);
    return false;
}

// The VC8's list, as before the VC4 (also the widest: every model's DO channels are inside 16..31).
inline bool Pci1203Vc8DoChanOk(int chan, std::string& why)
{
    return Pci1203Vc8DoChanOkFor(kPci1203VacVc8, chan, why);
}

// AI(W906-VC4) 20261001: the vacuum-OK DI channels of `model` (VC8 128..135, VC4 64..67) -- golden ReadVaccumIO.
inline bool Pci1203Vc8DiChanOkFor(int model, int chan, std::string& why)
{
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(model);
    if (chan >= L.diOkChan && chan < L.diOkChan + L.units) return true;
    why = Pci1203Vc8Fmt("%s 的真空 OK 輸入是 DI 通道 %d..%d，要的是 %d", L.name, L.diOkChan, L.diOkChan + L.units - 1, chan);
    return false;
}

// AI(W906-VC4) 20261001: the pressure DI bytes of `model` (VC8 0..15, VC4 0..7) -- golden GetIOValue Port*2 / Port*2+1.
inline bool Pci1203Vc8AiPortOkFor(int model, int port, std::string& why)
{
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(model);
    if (port >= 0 && port < 2 * L.units) return true;
    why = Pci1203Vc8Fmt("%s 壓力值只在 DI byte 0..%d，要的是 byte %d", L.name, 2 * L.units - 1, port);
    return false;
}

// The other DO channel of the same VC: golden gives VC n the pair 16+2n / 17+2n (one vacuum, one blow; which is
// which depends on iVacuOnOffSwap), so the partner is the channel with the low bit flipped.
// AI(W906-VC4) 20261001: the VC4's pair is the same 16+2n / 17+2n (only which one makes vacuum differs), so the
// partner rule is the same for both models.
inline int Pci1203Vc8DoPartner(int chan) { return chan ^ 1; }

}  // namespace ht9045

#endif  // ETHERCAT_PCI1203VC8_H
