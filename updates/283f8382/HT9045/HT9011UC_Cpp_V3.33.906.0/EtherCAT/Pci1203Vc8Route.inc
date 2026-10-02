// =============================================================================
//  EtherCAT/Pci1203Vc8Route.inc -- the ECAT-VC8 vacuum unit's calls, onto the PCIE-1203 (VacuumUnit/Vc8Route.h).
//
//  AI(W906-VACUNIT-1203) 20260930: new file, EastSun 20260930 「vacuunit 所有功能按鈕和內部功能要有所對應，
//  我需要實際上有功能」「針對1203 開啟 vacuunit 時，如果不符合現在程式碼就新創一個1203分支」.
//  #included by EtherCAT/Pci1203IoRoute.cpp ONLY (a wb_serve TU -- no new .cpp: the CMakeLists.txt source lists
//  are the laptop's), after its namespace and before W906_InstallPci1203IoRoute(), which installs it.
//  It reuses that file's DI / DO sample plumbing (FillDi / FillDo / ReadDi_, s_di / s_do).
//
//  ⚠ THIS FILE MAKES NO VENDOR CALL (tools/pci1203_control_gate.ps1 check 5d lists it): reads are the 1203
//    monitor's samples, the threshold read is TPci1203Monitor::Vc8SdoReadI16 (the monitor's own allowlisted
//    Acm_DevReadSDOData), writes are TPci1203Control::Execute(kCmdVc8DoSet / kCmdVc8SdoWriteI16).
//
//  WHAT EVERY ENTRY CHECKS, IN THIS ORDER, BEFORE IT TOUCHES ANYTHING (fail CLOSED, a reason every time)
//    all     the monitor exists, holds the card, is not Disabled
//            ring 1, and the station passes EtherCAT/Pci1203Vc8.h: ONE present sample at that address,
//            addressable, not a drive, named like an ECAT-VC8 -- ⚠ checking that the station EXISTS is not
//            enough here: golden's IndexArm2 stations 0x50 / 0x51 are ECx-P32 DI modules on this machine
//            the EtherCAT state the use needs (reads SAFEOP/OP, SDO a mailbox, writes OP)
//            the channel / object is on the VC8's closed list (DO 16..31, DI 128..135 and bytes 0..15,
//            SDO 8000h+10h*VC :02h / :13h)
//    writes  + Gerneral.ini VacuUnitType == 1 (golden VCCU_UNIT_TYPE; the "ECAT-VC8_ODM1" choice)
//            + SystemStart == false and SoftStart == false (the machine is not running or starting)
//            + an armed TPci1203Control (DRY RUN is honoured: nothing issued, the caller gets a failure)
//            + DO: the byte is in the card's DO map and read back; the station's DI (bytes 0..16) and DO
//              (bytes 2..3) are in the card maps
//            + DO: DEVIATION (recorded) -- a channel is not switched ON while the OTHER channel of the same VC
//              (the pair 16+2n / 17+2n: one vacuum, one blow, golden :231 / :250) reads ON, or cannot be read.
//              golden lets vacuum and blow be on together; this refuses it. Switching OFF is never refused.
//    Every write -- refused, dry or issued -- is kept (TVc8Write ring, the page's ack), printed on the console
//    and sent to the oplog (g_W906VacuumNote; a repeated identical non-issued outcome once a minute, with the
//    count -- golden's tmr1Timer retries the mode write every second); an issued / dry one is also in
//    Pci1203Control's audit log.
//    R1 (EastSun 20260930): there is no ECAT-VC8 on this machine's ring today, so every entry here is expected
//    to refuse, with the station's own reason, until one is fitted.
//
//  AI(W906-VC4) 20261001: THE ECAT-VC4 BRANCH. EastSun 1001 measured 「開真空是16bit 破真空17bit / ... /
//    開真空是22bit 破真空23bit」 on the ECAT-VC4-ODM1 modules at ring 1 160 / 161 / 162 and ruled 「VC8 和 VC4 要不同分支」.
//    The station check now also returns the MODEL (EtherCAT/Pci1203Vc8.h identity table), and every closed list
//    above is that model's (Pci1203VacLayoutOf): VC4 = DO 16..23 (byte 2), DI 64..67 and bytes 0..7 (DI 0..8 in the
//    map), SDO VC 0..3. Each entry first applies the VC8's (= the widest) list as before -- so a VC8 is refused or
//    accepted exactly as before, with the same reason -- then the station's own model's list. The vacuum+blow
//    interlock is the same for both (the partner is chan^1: the VC4's pair is also 16+2n / 17+2n).
//    VC4 SDO WRITES are refused (kVc8RcVc4SdoOff, before anything else is asked) unless W906_VC4_SDO_WRITE is
//    defined in EtherCAT/Pci1203Vc8.h; reads are allowed. Which DO channel MAKES vacuum on a VC4 (16+2VC) is the
//    PANEL's choice (VacuumUnit/MyVacuumPanel.h W906_OnChan) and the sucker gate's (VacuumUnit/Vc8Route.h): the
//    route takes the channel it is given, as for the VC8.
//    New entry Vc8Model (TVc8Route::model): the station's model by identity, no state, no vendor call.
// =============================================================================
#include "VacuumUnit/Vc8Route.h"
#include "EtherCAT/Pci1203Vc8.h"

extern bool SystemStart;      // cmydef.h:221
extern bool SoftStart;        // cmydef.h:223
extern int  VCCU_UNIT_TYPE;   // cmydef.h:5713 -- Gerneral.ini VacuUnitType (golden Sam 20230210)
//  The oplog line for every VC8 write (tools/wb_serve.cpp W906_OpLogInit sets it; 0 = oplog off).
void (*g_W906VacuumNote)(const char*) = 0;

namespace ht9045 {
namespace {

const int     kVc8WriteRing = 64;
TVc8Write     s_vc8W[kVc8WriteRing];
unsigned long s_vc8Seq = 0;
char          s_vc8Why[320] = "";

void Vc8Copy_(char* dst, int n, const std::string& s)
{
    if (!dst || n <= 0) return;
    std::strncpy(dst, s.c_str(), (std::size_t)(n - 1));
    dst[n - 1] = '\0';
}

int Vc8Rc_(int rc, const std::string& why)
{
    Vc8Copy_(s_vc8Why, (int)sizeof(s_vc8Why), why);
    return rc;
}

int Vc8RouteRc_(int routeRc)   // Pci1203IoRoute.cpp's kRoute* -> kVc8Rc*
{
    if (routeRc == kRouteNoCard)  return kVc8RcNoCard;
    if (routeRc == kRouteBadAddr) return kVc8RcBadAddr;
    if (routeRc == kRouteNoByte)  return kVc8RcNoByte;
    if (routeRc == kRouteNotRead) return kVc8RcNotRead;
    return kVc8RcRefused;
}

TPci1203Monitor* Vc8Mon_(std::string& why)
{
    TPci1203Monitor* mon = Pci1203Monitor();
    if (mon == 0 || !mon->Open_() || mon->Disabled() || !mon->card().open) {
        why = "1203 卡沒有開，或監看器已停止輪詢（Disabled）——ECAT-VC8 沒有讀寫";
        return 0;
    }
    return mon;
}

// The station, for `use`. 0 or a kVc8Rc code with `why`.
// AI(W906-VC4) 20261001: + `model` (optional): kPci1203VacVc8 / kPci1203VacVc4 on success.
int Vc8Station_(int ring, int ip, int use, int* slot, std::string& why, int* model = 0)
{
    if (slot) *slot = -1;
    if (model) *model = kPci1203VacNone;
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return kVc8RcNoCard;
    if (!Pci1203Vc8StationOkOn(*mon, ring, ip, use, slot, why, model))
        return (ring != (int)kPci1203Vc8Ring || ip <= 0) ? kVc8RcBadAddr : kVc8RcNotVc8;
    return 0;
}

// The station's bytes are where the card map says a VC8's are: DI 0..16, DO 2..3.
// AI(W906-VC4) 20261001: ... `model`'s: VC8 DI 0..16 / DO 2..3; VC4 DI 0..8 / DO 2.
int Vc8Maps_(int ring, int ip, int model, std::string& why)
{
    const Pci1203VacLayout& L = Pci1203VacLayoutOf(model);
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return kVc8RcNoCard;
    FillDi(mon);
    for (int b = 0; b < L.diBytes; ++b) {
        if (sjson::PickIoSample(s_di, ring, ip, b, 0) < 0) {
            why = (model == (int)kPci1203VacVc4)
                ? Fmt("卡片的 DI 對應表沒有 ring %d 站 0x%02X 的第 %d 個 byte（ECAT-VC4 要 0..8：四個壓力值＋真空 OK 位元）", ring, ip, b)
                : Fmt("卡片的 DI 對應表沒有 ring %d 站 0x%02X 的第 %d 個 byte（ECAT-VC8 要 0..16：八個壓力值＋真空 OK 位元）", ring, ip, b);
            return kVc8RcNoByte;
        }
    }
    FillDo(mon);
    for (int b = L.doFirst / 8; b <= L.doLast / 8; ++b) {
        if (sjson::PickIoSample(s_do, ring, ip, b, 0) < 0) {
            why = (model == (int)kPci1203VacVc4)
                ? Fmt("卡片的 DO 對應表沒有 ring %d 站 0x%02X 的第 %d 個 byte（ECAT-VC4 的吸／破真空在 byte 2）", ring, ip, b)
                : Fmt("卡片的 DO 對應表沒有 ring %d 站 0x%02X 的第 %d 個 byte（ECAT-VC8 的吸／破真空在 byte 2..3）", ring, ip, b);
            return kVc8RcNoByte;
        }
    }
    return 0;
}

// AI(W906-VC4) 20261001: the station's model by identity (no state asked, no reason kept) -- TVc8Route::model.
int Vc8ModelOf_(int ring, int ip)
{
    std::string why;
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return kPci1203VacNone;
    return Pci1203Vc8StationModelOn(*mon, ring, ip);
}

// The machine side of a write.
int Vc8Machine_(std::string& why)
{
    if (VCCU_UNIT_TYPE != 1) {
        why = Fmt("Gerneral.ini VacuUnitType=%d，不是 1（ECAT-VC8_ODM1）——golden 的 IO 真空，不寫 VC8", VCCU_UNIT_TYPE, 0, 0);
        return kVc8RcRefused;
    }
    if (SystemStart) { why = "機台運轉中（SystemStart）——真空單元不從網頁寫"; return kVc8RcRefused; }
    if (SoftStart)   { why = "機台正要啟動／回原點（SoftStart）——真空單元不從網頁寫"; return kVc8RcRefused; }
    if (Pci1203Control() == 0) {
        why = "1203 命令面沒有武裝（WB_PUMP_1203_CONTROL 未定義，或 Pci1203ControlEnable 失敗）——沒有送出";
        return kVc8RcNoControl;
    }
    return 0;
}

TVc8Write& Vc8NewWrite_(int kind, int ring, int ip, int chan, int sub, int value)
{
    TVc8Write& w = s_vc8W[s_vc8Seq % (unsigned long)kVc8WriteRing];
    std::memset(&w, 0, sizeof(w));
    w.seq = ++s_vc8Seq;
    w.kind = kind; w.ring = ring; w.ip = ip; w.chan = chan; w.sub = sub; w.value = value;
    w.dryRun = true;
    return w;
}

//  A write that was NOT issued and repeats the previous outcome for the same object is printed / oplogged at most
//  once a minute: golden's tmr1Timer retries the threshold MODE write every second on every visible panel until it
//  succeeds (EastSun R2 = follow golden), and with no ECAT-VC8 on the ring that is ~48 identical refusals a second.
//  The write is still refused, kept in the ring and shown on the page each time; only the console / oplog line is
//  folded, with the count. An ISSUED write is always printed.
struct Vc8LogKey_ { int kind, ring, ip, chan, sub, rc; unsigned long tick; unsigned long folded; };
const int kVc8LogKeys = 256;   // > every object a tick can refuse (6 stations x (16 DO + 8 VC x 2 subs) = 192); 32 thrashed at ~48 a tick
Vc8LogKey_ s_vc8Log[kVc8LogKeys];

bool Vc8LogDue_(const TVc8Write& w, int rc, unsigned long* folded)
{
    *folded = 0;
    if (w.issued) return true;
    const unsigned long now = (unsigned long)::GetTickCount();
    int freeSlot = -1, oldest = 0;
    for (int i = 0; i < kVc8LogKeys; ++i) {
        Vc8LogKey_& k = s_vc8Log[i];
        if (k.tick == 0) { if (freeSlot < 0) freeSlot = i; continue; }
        if (k.kind == w.kind && k.ring == w.ring && k.ip == w.ip && k.chan == w.chan && k.sub == w.sub && k.rc == rc) {
            if (now - k.tick < 60000ul) { ++k.folded; return false; }
            *folded = k.folded;
            k.tick = now; k.folded = 0;
            return true;
        }
        if (s_vc8Log[i].tick < s_vc8Log[oldest].tick) oldest = i;
    }
    Vc8LogKey_& k = s_vc8Log[freeSlot >= 0 ? freeSlot : oldest];
    k.kind = w.kind; k.ring = w.ring; k.ip = w.ip; k.chan = w.chan; k.sub = w.sub; k.rc = rc; k.tick = now ? now : 1; k.folded = 0;
    return true;
}

int Vc8Done_(TVc8Write& w, int rc, const std::string& why)
{
    w.rc = rc;
    Vc8Copy_(w.why, (int)sizeof(w.why), why);
    unsigned long folded = 0;
    if (!Vc8LogDue_(w, rc, &folded)) return Vc8Rc_(rc, why);
    char line[480];
    if (w.kind == 0)
        std::snprintf(line, sizeof(line), "vacuum VC8 -> 1203 #%lu: DO ring %d st 0x%02X chan %d = %d -> %s%s%s",
                      w.seq, w.ring, w.ip, w.chan, w.value,
                      !w.reached ? "REFUSED" : w.issued ? (rc == 0 ? "ISSUED" : "ISSUED, card error") : (w.dryRun ? "DRY" : "REFUSED"),
                      w.why[0] ? "  -- " : "", w.why);
    else
        std::snprintf(line, sizeof(line), "vacuum VC8 -> 1203 #%lu: SDO ring %d st 0x%02X %04Xh:%02Xh = %d -> %s%s%s",
                      w.seq, w.ring, w.ip, (unsigned)w.chan, (unsigned)w.sub, w.value,
                      !w.reached ? "REFUSED" : w.issued ? (rc == 0 ? "ISSUED" : "ISSUED, card error") : (w.dryRun ? "DRY" : "REFUSED"),
                      w.why[0] ? "  -- " : "", w.why);
    if (folded) {
        const std::size_t n = std::strlen(line);
        std::snprintf(line + n, sizeof(line) - n, "  (+%lu identical in the last minute not printed)", folded);
    }
    std::printf("%s\n", line);
    if (g_W906VacuumNote) g_W906VacuumNote(line);
    return Vc8Rc_(rc, why);
}

int Vc8Execute_(TVc8Write& w, const Pci1203Cmd& c)
{
    TPci1203Control* ctl = Pci1203Control();
    if (ctl == 0) return Vc8Done_(w, kVc8RcNoControl, "1203 命令面沒有武裝——沒有送出");
    const Pci1203CmdResult r = ctl->Execute(c);
    w.reached = true;
    w.issued  = r.issued;
    w.dryRun  = ctl->IsDryRun();
    w.ret     = r.ret;
    Vc8Copy_(w.call, (int)sizeof(w.call), r.wouldCall);
    if (!r.accepted) return Vc8Done_(w, kVc8RcRefused, r.why);
    if (!r.issued)   return Vc8Done_(w, kVc8RcRefused, r.why.empty() ? std::string("dry run: validated and recorded, nothing issued") : r.why);
    if (r.ret != 0)  return Vc8Done_(w, (int)r.ret, r.why.empty() ? Fmt("卡片回錯誤 0x%08X", (int)r.ret, 0, 0) : r.why);
    return Vc8Done_(w, 0, std::string());
}

// ---- the route entries ------------------------------------------------------------------------------------
int Vc8ReadAi(int ring, int ip, int port, unsigned char* v)
{
    if (v) *v = 0;
    std::string why;
    if (port < 0 || port >= 2 * (int)kPci1203Vc8Units)
        return Vc8Rc_(kVc8RcBadAddr, Fmt("ECAT-VC8 壓力值只在 DI byte 0..15，要的是 byte %d", port, 0, 0));
    int model = kPci1203VacNone;
    const int rc = Vc8Station_(ring, ip, kVc8UseReadIo, 0, why, &model);
    if (rc != 0) return Vc8Rc_(rc, why);
    if (!Pci1203Vc8AiPortOkFor(model, port, why)) return Vc8Rc_(kVc8RcBadAddr, why);   // AI(W906-VC4) 20261001: VC4 bytes 0..7
    unsigned char b = 0;
    const int rr = ReadDi_(ring, ip, port, &b);
    if (rr != 0) return Vc8Rc_(Vc8RouteRc_(rr), Fmt("ring %d 站 0x%02X 的 DI byte %d 讀不到（不在卡片對應表，或 valid=false）", ring, ip, port));
    if (v) *v = b;
    return 0;
}

int Vc8ReadDi(int ring, int ip, int chan, unsigned char* v)
{
    if (v) *v = 0;
    std::string why;
    //  AI(W906-VC4) 20261001: the pre-check is the union of the two models' vacuum-OK channels (VC8 128..135, VC4
    //  64..67) -- 64..67 is not inside the VC8's list, unlike every other list here -- then the station's own below.
    if ((chan < (int)kPci1203Vc8DiOkChan || chan >= (int)kPci1203Vc8DiOkChan + (int)kPci1203Vc8Units) &&
        (chan < (int)kPci1203Vc4DiOkChan || chan >= (int)kPci1203Vc4DiOkChan + (int)kPci1203Vc4Units))
        return Vc8Rc_(kVc8RcBadAddr, Fmt("ECAT-VC8 的真空 OK 輸入是 DI 通道 128..135（ECAT-VC4 是 64..67），要的是 %d", chan, 0, 0));
    int model = kPci1203VacNone;
    const int rc = Vc8Station_(ring, ip, kVc8UseReadIo, 0, why, &model);
    if (rc != 0) return Vc8Rc_(rc, why);
    if (!Pci1203Vc8DiChanOkFor(model, chan, why)) return Vc8Rc_(kVc8RcBadAddr, why);   // AI(W906-VC4) 20261001
    unsigned char b = 0;
    const int rr = ReadDi_(ring, ip, chan / 8, &b);
    if (rr != 0) return Vc8Rc_(Vc8RouteRc_(rr), Fmt("ring %d 站 0x%02X 的 DI byte %d 讀不到（不在卡片對應表，或 valid=false）", ring, ip, chan / 8));
    if (v) *v = (unsigned char)((b >> (chan % 8)) & 1);
    return 0;
}

// The DO byte holding `chan`, read back. -1 + rc / why when it cannot be used.
int Vc8DoSlot_(TPci1203Monitor* mon, int ring, int ip, int chan, int* rc, std::string& why)
{
    FillDo(mon);
    const int k = sjson::PickIoSample(s_do, ring, ip, chan / 8, 0);
    if (k < 0) {
        *rc = kVc8RcNoByte;
        why = Fmt("卡片的 DO 對應表沒有 ring %d 站 0x%02X 的第 %d 個 byte", ring, ip, chan / 8);
        return -1;
    }
    if (!s_do[k].valid) {
        *rc = kVc8RcNotRead;
        why = Fmt("ring %d 站 0x%02X 第 %d 個 DO byte 讀不回來（valid=false）", ring, ip, chan / 8);
        return -1;
    }
    return k;
}

int Vc8ReadDo(int ring, int ip, int chan, unsigned char* v)
{
    if (v) *v = 0;
    std::string why;
    if (!Pci1203Vc8DoChanOk(chan, why)) return Vc8Rc_(kVc8RcBadAddr, why);
    int model = kPci1203VacNone;
    const int rc = Vc8Station_(ring, ip, kVc8UseReadIo, 0, why, &model);
    if (rc != 0) return Vc8Rc_(rc, why);
    if (!Pci1203Vc8DoChanOkFor(model, chan, why)) return Vc8Rc_(kVc8RcBadAddr, why);   // AI(W906-VC4) 20261001: VC4 16..23
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return Vc8Rc_(kVc8RcNoCard, why);
    int rk = 0;
    const int k = Vc8DoSlot_(mon, ring, ip, chan, &rk, why);
    if (k < 0) return Vc8Rc_(rk, why);
    if (v) *v = (unsigned char)((s_do[k].byteData >> (chan % 8)) & 1);
    return 0;
}

int Vc8WriteDo(int ring, int ip, int chan, unsigned char v)
{
    TVc8Write& w = Vc8NewWrite_(0, ring, ip, chan, -1, v ? 1 : 0);
    std::string why;
    std::snprintf(w.call, sizeof(w.call), "Acm_DaqDoSetBitEx(ring=%d, station=0x%02X, chan=%d, %d)", ring, ip, chan, v ? 1 : 0);
    int rc = Vc8Machine_(why);
    if (rc != 0) return Vc8Done_(w, rc, why);
    if (!Pci1203Vc8DoChanOk(chan, why)) return Vc8Done_(w, kVc8RcBadAddr, why);
    int slot = -1, model = kPci1203VacNone;
    rc = Vc8Station_(ring, ip, kVc8UseWrite, &slot, why, &model);
    if (rc != 0) return Vc8Done_(w, rc, why);
    if (!Pci1203Vc8DoChanOkFor(model, chan, why)) return Vc8Done_(w, kVc8RcBadAddr, why);   // AI(W906-VC4) 20261001: VC4 16..23
    rc = Vc8Maps_(ring, ip, model, why);
    if (rc != 0) return Vc8Done_(w, rc, why);
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return Vc8Done_(w, kVc8RcNoCard, why);
    const int k = Vc8DoSlot_(mon, ring, ip, chan, &rc, why);
    if (k < 0) return Vc8Done_(w, rc, why);
    if (v) {
        const int partner = Pci1203Vc8DoPartner(chan);
        if (((s_do[k].byteData >> (partner % 8)) & 1) != 0)
            return Vc8Done_(w, kVc8RcRefused,
                            Fmt("同一個 VC 的另一個 DO 通道 %d 目前是 ON——吸真空與破真空不同時開（偏離 golden，已記錄），先關掉它再開通道 %d",
                                partner, chan, 0));
    }
    Pci1203Cmd c;
    c.kind  = kCmdVc8DoSet;
    c.port  = k;
    c.bit   = chan % 8;
    c.value = v ? 1.0 : 0.0;
    return Vc8Execute_(w, c);
}

int Vc8ReadSdoI16(int ring, int ip, int index, int sub, short* v)
{
    if (v) *v = 0;
    std::string why;
    int vc = -1;
    if (!Pci1203Vc8SdoAddrOk(index, sub, &vc, why)) return Vc8Rc_(kVc8RcBadAddr, why);
    TPci1203Monitor* mon = Vc8Mon_(why);
    if (mon == 0) return Vc8Rc_(kVc8RcNoCard, why);
    //  AI(W906-VC4) 20261001: a VC4's list is VC 0..3 -- the station's model first (the monitor's own read checks the
    //  station and the VC8's list again). A refused station is kVc8RcRefused, as when the monitor refused it before.
    int model = kPci1203VacNone;
    if (Vc8Station_(ring, ip, kVc8UseReadSdo, 0, why, &model) != 0) return Vc8Rc_(kVc8RcRefused, why);
    if (!Pci1203Vc8SdoAddrOkFor(model, index, sub, &vc, why)) return Vc8Rc_(kVc8RcBadAddr, why);
    short val = 0;
    unsigned long err = 0;
    if (!mon->Vc8SdoReadI16(ring, ip, index, sub, val, err, why))
        return Vc8Rc_(err != 0 ? (int)err : kVc8RcRefused, why);
    if (v) *v = val;
    return 0;
}

int Vc8WriteSdoI16(int ring, int ip, int index, int sub, short v)
{
    TVc8Write& w = Vc8NewWrite_(1, ring, ip, index, sub, v);
    std::snprintf(w.call, sizeof(w.call), "Acm_DevWriteSDOData(ring=%d, station=0x%02X, %04Xh:%02Xh, I16, 2) = %d",
                  ring, ip, (unsigned)index & 0xFFFFu, (unsigned)sub & 0xFFu, (int)v);
    std::string why;
    //  AI(W906-VC4) 20261001: an ECAT-VC4 (by identity) with W906_VC4_SDO_WRITE undefined is refused FIRST, with its
    //  own code, whatever the machine state -- golden's panels retry the mode write every tick and must be able to tell
    //  "switched off by policy" from "failed" (VacuumUnit/MyVacuumPanel.cpp InitialThresholdMode).
    if (!Pci1203Vc8SdoWriteOk(Vc8ModelOf_(ring, ip), why)) return Vc8Done_(w, kVc8RcVc4SdoOff, why);
    int rc = Vc8Machine_(why);
    if (rc != 0) return Vc8Done_(w, rc, why);
    int vc = -1;
    if (!Pci1203Vc8SdoAddrOk(index, sub, &vc, why)) return Vc8Done_(w, kVc8RcBadAddr, why);
    if (!Pci1203Vc8SdoValueOk(sub, (long)v, why)) return Vc8Done_(w, kVc8RcBadValue, why);
    int slot = -1, model = kPci1203VacNone;
    rc = Vc8Station_(ring, ip, kVc8UseWrite, &slot, why, &model);
    if (rc != 0) return Vc8Done_(w, rc, why);
    if (!Pci1203Vc8SdoAddrOkFor(model, index, sub, &vc, why)) return Vc8Done_(w, kVc8RcBadAddr, why);   // AI(W906-VC4) 20261001: VC4 VC 0..3
    if (!Pci1203Vc8SdoWriteOk(model, why)) return Vc8Done_(w, kVc8RcVc4SdoOff, why);
    Pci1203Cmd c;
    c.kind     = kCmdVc8SdoWriteI16;
    c.vc8Slave = slot;
    c.vc8Vc    = vc;
    c.vc8Sub   = sub;
    c.value    = (double)v;
    return Vc8Execute_(w, c);
}

//  forWrite: 0 = a read; 1 = a write the PAGE causes (+ VacuUnitType / SystemStart / SoftStart / armed control);
//  2 = an ENGINE sucker write after the remap (the station only: the IO route that carries it checks the control,
//  and the engine writes suckers while running -- kVc8CheckEngineWrite, VacuumUnit/Vc8Route.h).
int Vc8Check(int ring, int ip, int forWrite, char* whyOut, int whyLen)
{
    std::string why;
    int model = kPci1203VacNone;
    int rc = (forWrite == 1) ? Vc8Machine_(why) : 0;
    if (rc == 0) rc = Vc8Station_(ring, ip, forWrite ? kVc8UseWrite : kVc8UseReadIo, 0, why, &model);
    if (rc == 0) rc = Vc8Maps_(ring, ip, model, why);   // AI(W906-VC4) 20261001: the station's model's bytes
    Vc8Copy_(whyOut, whyLen, why);
    return rc;
}

// AI(W906-VC4) 20261001: TVc8Route::model -- kVc8ModelVc8 / kVc8ModelVc4 / kVc8ModelNone (the values are the same
// numbers as kPci1203VacVc8 / kPci1203VacVc4 / kPci1203VacNone; tests/test_vacuum_vc8.cpp pins that).
int Vc8Model(int ring, int ip) { return Vc8ModelOf_(ring, ip); }

const char*   Vc8LastWhy() { return s_vc8Why; }
unsigned long Vc8LastSeq() { return s_vc8Seq; }

int Vc8WritesSince(unsigned long afterSeq, TVc8Write* out, int max)
{
    int n = 0;
    const unsigned long first = (s_vc8Seq > (unsigned long)kVc8WriteRing) ? s_vc8Seq - (unsigned long)kVc8WriteRing + 1 : 1;
    for (unsigned long s = (afterSeq + 1 > first ? afterSeq + 1 : first); s <= s_vc8Seq && n < max; ++s) {
        const TVc8Write& w = s_vc8W[(s - 1) % (unsigned long)kVc8WriteRing];
        if (w.seq != s) continue;
        if (out) out[n] = w;
        ++n;
    }
    return n;
}

const TVc8Route s_vc8Route = { Vc8ReadAi, Vc8ReadDi, Vc8ReadDo, Vc8WriteDo, Vc8ReadSdoI16, Vc8WriteSdoI16,
                               Vc8Check, Vc8LastWhy, Vc8LastSeq, Vc8WritesSince, Vc8Model };   // AI(W906-VC4) 20261001: + Vc8Model

}  // namespace

// Called by W906_InstallPci1203IoRoute() only (the gate counts the slot's writers: this one).
void Vc8RouteInstall_()
{
    SetVc8Route(&s_vc8Route);
    TPci1203Control* ctl = Pci1203Control();
    std::string vc4Sdo;   // AI(W906-VC4) 20261001
    const bool vc4SdoOn = Pci1203Vc8SdoWriteOk(kPci1203VacVc4, vc4Sdo);
    std::printf("vacuum VC8 -> 1203: ROUTED -- golden MyLaneIO / TMyVacuumPanel VC8 calls read the monitor and write through "
                "Pci1203Control (%s); every station is identity-checked as an ECAT-VC8 or ECAT-VC4 first (VC4: DO 16..23, "
                "make 17+2VC; SDO writes %s), VacuUnitType=%d\n",   // AI(W906-VC4-ODD) 20261001: was "make 16+2VC"
                ctl == 0 ? "control NOT armed: every VC8 write is refused"
                         : ctl->IsDryRun() ? "DRY RUN: validated and recorded, nothing issued"
                                           : "*** LIVE: a press on HW.VacuumUnit reaches the card ***",
                vc4SdoOn ? "ON (W906_VC4_SDO_WRITE)" : "OFF", VCCU_UNIT_TYPE);
}

}  // namespace ht9045
