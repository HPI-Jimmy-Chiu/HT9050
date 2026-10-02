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
// =============================================================================
#ifndef ETHERCAT_PCI1203VC8_H
#define ETHERCAT_PCI1203VC8_H

#include <cstdarg>
#include <cstdio>
#include <string>

#include "EtherCAT/Pci1203Monitor.h"   // Pci1203SlaveSample, TPci1203Monitor (public accessors only)

namespace ht9045 {

struct Pci1203Vc8Identity {
    const char*   nameHas;     // case-insensitive substring of ADV_SLAVE_INFO.Name
    unsigned long vendorId;    // 0 = not checked (not pinned yet)
    unsigned long productId;   // 0 = not checked
    const char*   source;      // why this line is here
};

inline const Pci1203Vc8Identity* Pci1203Vc8Identities(int* n)
{
    static const Pci1203Vc8Identity k[] = {
        { "ECAT-VC8", 0ul, 0ul, "golden rgVacuUnitType item 'ECAT-VC8_ODM1' (FileRW/HSys.gen.inc); unconfirmed on a real module" },
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
    kPci1203EcStateOp     = 0x08
};

// What the caller is about to do with the station: it decides the EtherCAT state required.
enum Pci1203Vc8Use {
    kVc8UseReadIo  = 0,   // a DI / DO sample: SAFEOP or OP (inputs update, Pci1203SlaveInputsLive's rule)
    kVc8UseReadSdo = 1,   // an SDO read: PREOP / SAFEOP / OP (CoE mailbox, Pci1203SlaveMailboxOk's rule)
    kVc8UseWrite   = 2    // any write: OP only (outputs update only in OP)
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
    char b[400];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(b, sizeof(b), f, ap);
    va_end(ap);
    return b;
}

// Is this ONE sample an ECAT-VC8? The drive tests come first because they are the ones that must never be
// argued with: a CiA 402 profile or a SERVOPACK name is a motion drive whatever else it says.
inline bool Pci1203Vc8IdentityOk(const Pci1203SlaveSample& s, std::string& why)
{
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
        return true;
    }
    why = Pci1203Vc8Fmt("ring %d 站 0x%02X 回報的模組是「%s」（VendorID 0x%08lX, ProductID 0x%08lX），不在 ECAT-VC8 身分表"
                        "（EtherCAT/Pci1203Vc8.h：名稱要含「%s」）——拒絕。若這真的是真空單元，請把上面這個名稱告訴我們",
                        s.ring, s.addr, s.name.c_str(), s.vendorId, s.productId, n > 0 ? t[0].nameHas : "?");
    return false;
}

// The EtherCAT state `use` needs (low nibble of EC_SLAVE_STATE_*).
inline bool Pci1203Vc8StateOk(const Pci1203SlaveSample& s, int use, std::string& why)
{
    const unsigned st = (unsigned)s.state & 0x0Fu;
    bool ok = false;
    const char* need = "";
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
template <class Get>
inline bool Pci1203Vc8StationOkBy(Get get, int n, int ring, int addr, int use, int* slot, std::string& why)
{
    if (slot) *slot = -1;
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
    if (!Pci1203Vc8IdentityOk(s, why)) return false;
    if (!Pci1203Vc8StateOk(s, use, why)) return false;
    if (slot) *slot = k;
    return true;
}

inline bool Pci1203Vc8StationOk(const Pci1203SlaveSample* sl, int n, int ring, int addr, int use, int* slot, std::string& why)
{
    return Pci1203Vc8StationOkBy([sl](int i) -> const Pci1203SlaveSample& { return sl[i]; }, n, ring, addr, use, slot, why);
}

inline bool Pci1203Vc8StationOkOn(const TPci1203Monitor& mon, int ring, int addr, int use, int* slot, std::string& why)
{
    return Pci1203Vc8StationOkBy([&mon](int i) -> const Pci1203SlaveSample& { return mon.slave(i); },
                                 mon.slaveCount(), ring, addr, use, slot, why);
}

// The SDO closed list: 8000h + VC*10h (VC 0..7), subindex 02h (mode) or 13h (threshold). `vc` out.
inline bool Pci1203Vc8SdoAddrOk(int index, int sub, int* vc, std::string& why)
{
    if (vc) *vc = -1;
    const int off = index - (int)kPci1203Vc8SdoBase;
    if (off < 0 || (off % (int)kPci1203Vc8SdoStep) != 0 || off / (int)kPci1203Vc8SdoStep >= (int)kPci1203Vc8Units) {
        why = Pci1203Vc8Fmt("SDO 物件 %04Xh 不在 ECAT-VC8 的清單（8000h..8070h，每 10h 一個 VC）——拒絕", (unsigned)index & 0xFFFFu);
        return false;
    }
    if (sub != (int)kPci1203Vc8SubMode && sub != (int)kPci1203Vc8SubThr) {
        why = Pci1203Vc8Fmt("SDO 子索引 %02Xh 不在清單（02h 閥值模式、13h 閥值）——拒絕", (unsigned)sub & 0xFFu);
        return false;
    }
    if (vc) *vc = off / (int)kPci1203Vc8SdoStep;
    return true;
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

inline bool Pci1203Vc8DoChanOk(int chan, std::string& why)
{
    if (chan >= (int)kPci1203Vc8DoFirst && chan <= (int)kPci1203Vc8DoLast) return true;
    why = Pci1203Vc8Fmt("DO 通道 %d 不是 ECAT-VC8 的吸／破真空通道（16..31）——拒絕", chan);
    return false;
}

// The other DO channel of the same VC: golden gives VC n the pair 16+2n / 17+2n (one vacuum, one blow; which is
// which depends on iVacuOnOffSwap), so the partner is the channel with the low bit flipped.
inline int Pci1203Vc8DoPartner(int chan) { return chan ^ 1; }

}  // namespace ht9045

#endif  // ETHERCAT_PCI1203VC8_H
