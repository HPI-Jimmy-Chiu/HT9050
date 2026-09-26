// =============================================================================
//  EtherCAT/Pci1203IoRoute.cpp -- see the header for what this connects and why.
//
//  AI(W906-IOWEB-P17) 20260925: new file, wb_serve only (CMakeLists.txt, next to
//  Pci1203Control.cpp). ⚠ This TU makes NO vendor call of its own: reads come
//  from TPci1203Monitor's samples, writes go through TPci1203Control::Execute.
//  Neither 1203 gate scans this file, so the check is: no `Acm_<name>(` CALL in
//  CODE -- the vendor names below appear only in comments and in the text of
//  the `exCall` strings that describe what Execute issues (review P17-CPP-3:
//  a plain `grep Acm_` has hits by design and so cannot be the test).
// =============================================================================
#include "MachineType.h"   // SOFT_SIMULTE / INSTALL_1203_MONITOR / WB_PUMP_1203_CONTROL / WB_ENGINE_IO_1203 decided BEFORE the #if below (tools/macro_order_gate.ps1)
#include "EtherCAT/Pci1203IoRoute.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "IOBackend.h"
#include "EtherCAT/Pci1203Monitor.h"
#include "EtherCAT/Pci1203Control.h"
#include "JsonBridge/ChanIo.h"

namespace ht9045 {

namespace {

// Raw failure codes handed back to TLaneIO in place of a vendor ret. Any
// non-zero value is a failure there (MyLaneIo.cpp:294-297 maps it to -1); these
// are distinct only so a debugger or a future log can tell them apart. The
// 0x7E prefix is not an Advantech range (theirs are 0x80xxxxxx / 0x83xxxxxx).
const int kRouteNoControl = 0x7E000001;   // no armed TPci1203Control
const int kRouteNoCard    = 0x7E000002;   // monitor absent or card not open
const int kRouteBadAddr   = 0x7E000003;   // ring / station / port not usable
const int kRouteNoByte    = 0x7E000004;   // no ring-attributed byte at that address
const int kRouteNotRead   = 0x7E000005;   // the byte did not read back
const int kRouteDrive     = 0x7E000006;   // the station is a motion drive
const int kRouteRefused   = 0x7E000007;   // Execute() refused, or LIVE vendor error without a code

Pci1203RouteWrite s_last;
unsigned long     s_seq = 0;
unsigned long     s_skipped = 0;   // engine writes of an unchanged value (S7), not sent to Execute
bool              s_installed = false;
const char*       s_source = "engine";

// Reused every call: the engine reads many points per tick on ONE thread.
std::vector<sjson::IoByteSample> s_di, s_do;

void FillDi(TPci1203Monitor* mon)
{
    s_di.clear();
    const int n = mon->diCount();
    for (int i = 0; i < n; ++i) {
        const Pci1203DiSample& s = mon->di(i);
        sjson::IoByteSample b;
        b.valid = s.valid;
        b.ring = s.flat ? -1 : s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        s_di.push_back(b);
    }
}

// ⚠ The index in s_do IS the monitor's DO slot, which is what kCmdDoSetBit's
//   `port` means (Pci1203Control.cpp:1672 `mon->do_(c.port)`). Slots are
//   reassigned on Rescan, so this is rebuilt on every call and never cached.
void FillDo(TPci1203Monitor* mon)
{
    s_do.clear();
    const int n = mon->doCount();
    for (int i = 0; i < n; ++i) {
        const Pci1203DoSample& s = mon->do_(i);
        sjson::IoByteSample b;
        b.valid = s.valid;
        b.ring = s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        s_do.push_back(b);
    }
}

bool NameSaysServopack(const std::string& name)
{
    std::string u(name);
    for (std::size_t i = 0; i < u.size(); ++i) u[i] = (char)std::toupper((unsigned char)u[i]);
    return u.find("SERVOPACK") != std::string::npos;
}

// Same rule as web/js/pci1203/view.js:3906 stationIsDrive(): any discovered
// slave at this address on this ring (or with no ring recorded) that says it is
// CiA 402, or calls itself a SERVOPACK. "Any" on purpose -- under the SubDevice
// ID conflict two slaves can answer one address, and one drive among them is
// reason enough not to write.
bool StationIsDrive(TPci1203Monitor* mon, int ring, int station)
{
    const int n = mon->slaveCount();
    for (int i = 0; i < n; ++i) {
        const Pci1203SlaveSample& s = mon->slave(i);
        if (!s.present || s.addr != station) continue;
        if (s.ring >= 0 && s.ring != ring) continue;
        if (s.profileValid && s.profile == 402) return true;
        if (NameSaysServopack(s.name)) return true;
    }
    return false;
}

std::string Fmt(const char* f, int a, int b, int c)
{
    char buf[256];
    std::snprintf(buf, sizeof(buf), f, a, b, c);
    return buf;
}

// Every check a write makes before Execute. On failure fills `why` and `code`.
bool CheckWrite_(int ring, int station, int stationChan, int* slot, std::string& why, int* code)
{
    TPci1203Control* ctl = Pci1203Control();
    if (ctl == 0) {
        why = "1203 命令面沒有武裝（WB_PUMP_1203_CONTROL 未定義，或 Pci1203ControlEnable 失敗）——沒有送出";
        *code = kRouteNoControl;
        return false;
    }
    TPci1203Monitor* mon = Pci1203Monitor();
    if (mon == 0 || !mon->Open_() || mon->Disabled() || !mon->card().open) {
        //  Disabled()/card().open too (review P17-CPP-4): the monitor can stop
        //  polling while Open_() stays true and the old samples stay valid=true
        //  (Pci1203Monitor.cpp conflict-disable / ATTACHED detach). A byte nobody
        //  is refreshing is not a read-back to act on.
        why = "1203 卡沒有開，或監看器已停止輪詢（Disabled）——沒有送出";
        *code = kRouteNoCard;
        return false;
    }
    //  Ring 0 is the motion ring on this machine: every station there is a drive
    //  (SERVOPACK / SW3D-680), and its DO bytes are RxPDO (card map 20260924).
    //  golden's own 1203 IO lives on ring 1 -- cmydef.cpp iEtherCatRing=1, and
    //  MyLaneIo.cpp IOOutBitStatus answers false for a 1203 point on Ring 0.
    //  Refused OUTRIGHT, before the drive test below, because that test is a
    //  blacklist that fails open when the slave scan is incomplete (review S5:
    //  the DO map can be intact while the sweep found 0 slaves).
    if (ring == 0) {
        why = Fmt("ring 0 是馬達環（伺服／步進驅動器），IO 輸出只在 ring 1；拒寫 ring 0 站 %d 第 %d 個 byte"
                  "（多半是 IO_Table 的 Lane 填成 0）——沒有送出", station, stationChan, 0);
        *code = kRouteDrive;
        return false;
    }
    FillDo(mon);
    const char* pick = 0;
    const int k = sjson::PickIoSample(s_do, ring, station, stationChan, &pick);
    if (k < 0) {
        if (pick && std::strcmp(pick, "bad-address") == 0) {
            why = Fmt("位址不能用（ring %d、站 %d、站內 byte %d）——ring 必須 >=0、站號 >0；沒有送出",
                      ring, station, stationChan);
            *code = kRouteBadAddr;
        } else {
            why = Fmt("卡片的 DO 對應表裡沒有 ring %d 站 %d 的第 %d 個 byte（IO_Table 的 Lane／IP／Port 對不上卡片）——沒有送出",
                      ring, station, stationChan);
            *code = kRouteNoByte;
        }
        return false;
    }
    if (!s_do[k].valid) {
        why = Fmt("ring %d 站 %d 第 %d 個 DO byte 讀不回來（valid=false），不盲寫——沒有送出",
                  ring, station, stationChan);
        *code = kRouteNotRead;
        return false;
    }
    if (StationIsDrive(mon, ring, station)) {
        why = Fmt("ring %d 站 %d 是伺服／馬達驅動器（CiA 402 或 SERVOPACK），它的 DO byte 是驅動器的 RxPDO，拒絕寫入"
                  "（多半是 IO_Table 的 Lane 填錯）——沒有送出", ring, station, 0);
        *code = kRouteDrive;
        return false;
    }
    *slot = k;
    return true;
}

void Report_(const Pci1203RouteWrite& w)
{
    if (std::strcmp(s_source, "engine") != 0) return;   // the web click prints its own line
    static unsigned long n = 0;
    ++n;
    if (!(n <= 30 || (n % 500) == 0)) return;
    std::printf("engine IO -> 1203 #%lu: %s ring %d st %d %s %d = %d -> %s%s%s\n",
                n, w.byte ? "byte" : "bit", w.ring, w.station,
                w.byte ? "byte" : "port", w.port, w.value,
                !w.reached ? "REFUSED" : !w.accepted ? "REFUSED" : w.issued ? "ISSUED" : "DRY",
                w.why.empty() ? "" : "  -- ", w.why.c_str());
    if (n == 30) std::printf("engine IO -> 1203: further lines only every 500th write\n");
}

int RouteWriteBit(int Ring, int IP, int Port, int Bit, int Value)
{
    (void)Bit;   // golden passes the CHANNEL in Port for ePCI1203; Bit only feeds the command-cache mask (IOBackend.cpp:141-145)
    Pci1203RouteWrite w;
    w.seq = ++s_seq;
    w.ring = Ring; w.station = IP; w.port = Port; w.value = Value ? 1 : 0;
    int code = kRouteBadAddr, slot = -1;
    if (Port < 0 || !CheckWrite_(Ring, IP, Port < 0 ? -1 : Port / 8, &slot, w.why, &code)) {
        if (Port < 0) w.why = "Port < 0——沒有送出";
        s_last = w; Report_(w);
        return code;
    }
    //  An ENGINE write of the value the card already reads back is skipped (review S7):
    //  ShowRunLed / the tower lamp re-write the same bits every few ticks, and routing
    //  each one through Execute overwrote Pci1203Control's last-command record and
    //  pushed the operator's own commands out of its 512-line audit log within minutes.
    //  Effect-identical -- an EtherCAT DO holds its state, and the read-back is the byte
    //  the card is driving (refreshed every IO tick and after every issued write).
    //  ⚠ Engine only: a web click is ALWAYS executed and recorded.
    if (std::strcmp(s_source, "engine") == 0 &&
        ((s_do[slot].byteData >> (Port % 8)) & 1) == w.value) {
        ++s_skipped;
        return 0;
    }
    TPci1203Control* ctl = Pci1203Control();
    Pci1203Cmd c;
    c.kind = kCmdDoSetBit;
    c.port = slot;
    c.bit = Port % 8;
    c.value = w.value;
    const Pci1203CmdResult r = ctl->Execute(c);
    w.slot = slot;
    w.stationChan = Port / 8;
    w.reached = true;
    w.accepted = r.accepted;
    w.issued = r.issued;
    w.ret = r.ret;
    w.why = r.why;
    w.dryRun = ctl->IsDryRun();
    //  ⚠ Spelled out here because Execute's own wouldCall text is the FLAT
    //    spelling even when LIVE issues the Ex call (Pci1203Control.cpp:660-662
    //    vs :1675). This is the call Execute makes for this sample, and it is
    //    golden's own Acm_DaqDoSetBitEx(uiDevhand, Ring, IP, Port, v) with the
    //    same three numbers (golden MyLaneIo.cpp:145/:212).
    char ex[160];
    std::snprintf(ex, sizeof(ex), "Acm_DaqDoSetBitEx(ring=%d, station=%d, chan=%d, %d)",
                  Ring, IP, Port, w.value);
    w.exCall = ex;
    s_last = w; Report_(w);
    if (!r.accepted) return kRouteRefused;
    if (r.issued && r.ret != 0) return (int)r.ret != 0 ? (int)r.ret : kRouteRefused;
    return 0;
}

int RouteWriteByte(int Ring, int IP, int Port, unsigned int Byte)
{
    Pci1203RouteWrite w;
    w.seq = ++s_seq;
    w.byte = true;
    w.ring = Ring; w.station = IP; w.port = Port; w.value = (int)(Byte & 0xff);
    int code = kRouteBadAddr, slot = -1;
    if (!CheckWrite_(Ring, IP, Port, &slot, w.why, &code)) {   // byte form: Port IS the byte within the station
        s_last = w; Report_(w);
        return code;
    }
    if (std::strcmp(s_source, "engine") == 0 && s_do[slot].byteData == (unsigned char)w.value) {   // S7, byte form
        ++s_skipped;
        return 0;
    }
    TPci1203Control* ctl = Pci1203Control();
    Pci1203Cmd c;
    c.kind = kCmdDoSetByte;
    c.port = slot;
    c.value = w.value;
    const Pci1203CmdResult r = ctl->Execute(c);
    w.slot = slot;
    w.stationChan = Port;
    w.reached = true;
    w.accepted = r.accepted;
    w.issued = r.issued;
    w.ret = r.ret;
    w.why = r.why;
    w.dryRun = ctl->IsDryRun();
    char ex[160];
    std::snprintf(ex, sizeof(ex), "Acm_DaqDoSetByteEx(ring=%d, station=%d, port=%d, 0x%02X)",
                  Ring, IP, Port, (unsigned)w.value);
    w.exCall = ex;
    s_last = w; Report_(w);
    if (!r.accepted) return kRouteRefused;
    if (r.issued && r.ret != 0) return (int)r.ret != 0 ? (int)r.ret : kRouteRefused;
    return 0;
}

int ReadDi_(int Ring, int IP, int stationChan, unsigned char* byte)
{
    TPci1203Monitor* mon = Pci1203Monitor();
    if (mon == 0 || !mon->Open_() || mon->Disabled() || !mon->card().open) return kRouteNoCard;   // P17-CPP-4: no frozen inputs
    FillDi(mon);
    const int k = sjson::PickIoSample(s_di, Ring, IP, stationChan, 0);
    if (k < 0) return (Ring < 0 || IP <= 0 || stationChan < 0) ? kRouteBadAddr : kRouteNoByte;
    if (!s_di[k].valid) return kRouteNotRead;
    *byte = s_di[k].byteData;
    return 0;
}

int RouteReadBit(int Ring, int IP, int Port, int Bit, unsigned char* Value)
{
    (void)Bit;   // same as the write: the channel travels in Port
    if (Value) *Value = 0;
    if (Port < 0) return kRouteBadAddr;
    unsigned char b = 0;
    const int rc = ReadDi_(Ring, IP, Port / 8, &b);
    if (rc != 0) return rc;
    if (Value) *Value = (unsigned char)((b >> (Port % 8)) & 1);
    return 0;
}

int RouteReadByte(int Ring, int IP, int Port, unsigned char* Value)
{
    if (Value) *Value = 0;
    unsigned char b = 0;
    const int rc = ReadDi_(Ring, IP, Port, &b);   // byte form: Port IS the byte within the station
    if (rc != 0) return rc;
    if (Value) *Value = b;
    return 0;
}

// Defined unconditionally so every build configuration compiles the same code;
// only the INSTALL below is conditional.
const TPci1203IoRoute s_route = { RouteWriteBit, RouteWriteByte, RouteReadBit, RouteReadByte };

}  // namespace

const Pci1203RouteWrite& Pci1203RouteLastWrite() { return s_last; }
bool Pci1203RouteInstalled() { return s_installed && Pci1203IoRoute() == &s_route; }
void Pci1203RouteSetSource(const char* source) { s_source = source ? source : "engine"; }

bool Pci1203RouteCanWriteBit(int ring, int station, int port,
                             int* slot, int* stationChan, std::string& why)
{
    int code = 0, k = -1;
    if (port < 0) { why = "Port < 0"; return false; }
    if (!CheckWrite_(ring, station, port / 8, &k, why, &code)) return false;
    if (slot) *slot = k;
    if (stationChan) *stationChan = port / 8;
    return true;
}

}  // namespace ht9045

void W906_InstallPci1203IoRoute()
{
#if defined(SOFT_SIMULTE)
    std::printf("engine IO -> 1203: SIM build -- MyLaneIO keeps TSimIOBackend (SelectVendorBackends is a no-op), route NOT installed\n");
#elif defined(WB_ENGINE_IO_1203) && defined(INSTALL_1203_MONITOR) && defined(WB_PUMP_1203_CONTROL)
    SetPci1203IoRoute(&ht9045::s_route);
    ht9045::s_installed = true;
    ht9045::TPci1203Control* ctl = ht9045::Pci1203Control();
    std::printf("engine IO -> 1203: ROUTED -- MyLaneIO reads the monitor's DI samples and writes through Pci1203Control (%s)\n",
                ctl == 0 ? "control NOT armed: every engine write is refused"
                         : ctl->IsDryRun() ? "DRY RUN: validated and recorded, nothing issued"
                                           : "*** LIVE: engine outputs reach the card ***");
#else
    std::printf("engine IO -> 1203: NOT routed (needs WB_ENGINE_IO_1203 + INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL) -- TPci1203Backend stays the stub\n");
#endif
}
