// ===========================================================================
//  JsonBridge/ChanIoPoints.cpp
//
//  AI(W906-IO-POINTS) 20260924.  NOT in golden.
//  每一個 IO 點（Alias）的設定與即時狀態，給 HW.IoSetView.html。
//  設計、位址對應的證據、null-vs-0 規則都寫在 JsonBridge/ChanIo.h 檔尾，這裡不重複。
// ===========================================================================
#include "JsonBridge/ChanIo.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "EtherCAT/Pci1203Monitor.h"
#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "database.h"     // HSys.IOTable / TIODATA
#include "common.h"       // IoTablePath
#include "JsonBridge/CardTypeDiag.h"   //AI(W906-CARDTYPE) 20261006 (Jerry, J-17 A): CardTypeUnsupportedWhy()／IO_CARD_TYPE

// golden cmydef.h:5577 -- the outputs that need the safe door closed while idle (filled by InitCylinder,
// cinitial.cpp:5418-5452, which wb_serve runs through InitialHandler since AI(W906-A4-2) 20260924).
// AI(W906-IOWIDGET-3) 20261001: read here for the IO page's colours (golden SetCompomentIO, iosetview.cpp:2244-2258).
extern bool bIdleNeedCheckSafeDoor[4][64][32][8];
bool W906_PadIoPoint(const AnsiString& alias, bool* on);   // AI(W906-W155) 20261007 (St02-E): PadInterface_St02.cpp EOF (ht9045_sm) -- the IO page's pad squares; on the old blank line
namespace ht9045 {
namespace sjson {

namespace {

const int kIsaPci1203 = 3;          // MachineType.h eIOType：ePCI1203 = 3
const int kIsaMotionNet = 0;        // MachineType.h eIOType：eMotionNet = 0

// golden SetCompomentIO (iosetview.cpp:2244-2258 TBtnPanelLane, :2322-2330 TBtnPanel): the button's address in
// bIdleNeedCheckSafeDoor -- MotionNet / 1203 [Ring][IP][Port][Bit], the ISA family [0][Port/32][Port%32][Bit]
// ("Steven 20240103 : 加入保護" is that ISA split only). The MotionNet / 1203 tuple is NOT range-checked in golden: the
// writer (cinitial.cpp:5435/:5451) and the reader that blocks the output (IdleCheckSafeDoorByCylinder, csystem.cpp:21375,
// via MyLaneIo.cpp:261/:326) both index it raw, so a 1203 row past [64][32] (HT9050: IP 176, Port up to 135) lands on
// the cell ring*16384 + ip*256 + port*8 + bit of the same 64 KB block. The same cell is read here, so the colour says
// what the interlock will do; past the whole block = not marked (golden would read another variable there).
// (20261001 offline check: on the HT9050 IO_Table InitCylinder marks no cell at all -- the 7 enabled cylinders all sit
//  on golden's bypass list -- and none of them is past the block.)
bool NeedSafeDoor(const TIODATA* r)
{
    long ring, ip, port;
    if (r->iISABase == kIsaMotionNet || r->iISABase == kIsaPci1203) { ring = r->iLane; ip = r->iIP; port = r->iPort; }
    else { ring = 0; ip = r->iPort / 32; port = r->iPort % 32; }
    const long off = ((ring * 64 + ip) * 32 + port) * 8 + r->iBit;
    if (ring < 0 || ip < 0 || port < 0 || r->iBit < 0 || off < 0 || off >= (long)sizeof(bIdleNeedCheckSafeDoor)) return false;
    return (&bIdleNeedCheckSafeDoor[0][0][0][0])[off];
}
const int kPollIntervalMs = 200;    // AI(W906-MT-E3a) 20260925: 500 -> 200 = 1203 監看器 Poll 的節拍（wb_serve kIoTickMs）。原本寫「wb_serve 一個 tick」的 500 是引擎的拍子不是卡片資料的更新頻率，頁面照它等就等於每 500 ms 才看一次每 200 ms 就更新的值（batchinput 報告）；網頁端已經照 pollIntervalMs 走（HW.IoSetView／IoLive，AI(W906-MT-E3)）

unsigned long long g_rtSeq = 0;

std::string NowIsoUtc()
{
    using namespace std::chrono;
    const system_clock::time_point now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);
    const long ms = (long)(duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
    std::tm g = *std::gmtime(&t);      // 單執行緒（UI tick）呼叫
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  g.tm_year + 1900, g.tm_mon + 1, g.tm_mday,
                  g.tm_hour, g.tm_min, g.tm_sec, ms);
    return buf;
}

std::string Str(const AnsiString& s) { return std::string(s.c_str()); }

// config 與 runtime 共用同一套 ioId，才合併得起來。
// 規則：alias@ioCode；alias 空白的列略過（沒有名字，golden 也綁不到它）；
// ioCode 空白或撞號時改用 alias@row<資料列序號>（舊檔也是這個寫法，例 SwAuto1Bin@row623）。
std::vector<std::string> BuildIoIds()
{
    std::vector<std::string> ids(HSys.IOTable.size());
    std::set<std::string> used;
    for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
        const TIODATA* r = HSys.IOTable[i];
        if (!r) continue;
        const std::string alias = Str(r->Alias);
        if (alias.empty()) continue;
        const std::string code = IoCodeOf(r->iISABase, IoDirectionOfType(Str(r->Type)),
                                          r->iIP, r->iPort, r->iBit);
        std::string id = alias + "@" + code;
        if (code.empty() || used.count(id)) {
            char b[24];
            std::snprintf(b, sizeof(b), "@row%u", (unsigned)i);
            id = alias + b;
        }
        used.insert(id);
        ids[i] = id;
    }
    return ids;
}

void NumOrNull(webbridge::JsonWriter& w, const char* key, int v)
{
    w.Key(key);
    if (v < 0) w.Null(); else w.Number((wb_int64)v);
}

}  // namespace

IoPointDir IoDirectionOfType(const std::string& t)
{
    if (t == "Sensor" || t == "Cylinder_On" || t == "Cylinder_Off" || t == "Sucker")
        return kIoDirIn;
    if (t == "Switch" || t == "Cylinder" || t == "Sucker_On" || t == "Sucker_Off")
        return kIoDirOut;
    return kIoDirUnknown;
}

std::string IoCodeOf(int isaBase, IoPointDir dir, int ip, int port, int bit)
{
    const char* d = (dir == kIoDirIn) ? "I" : (dir == kIoDirOut) ? "O" : "";
    if (!*d || ip < 0 || port < 0) return std::string();
    char b[40];
    if (isaBase == kIsaPci1203) {
        std::snprintf(b, sizeof(b), "%s%d.%d", d, ip, port);
    } else {
        if (bit < 0) return std::string();
        std::snprintf(b, sizeof(b), "%s%02d%d%d", d, ip, port, bit);
    }
    return b;
}

IoPointState ResolveIoPoint(int isaBase, int lane, int ip, int port, int inType, int enable,
                            IoPointDir dir,
                            const std::vector<IoByteSample>& di,
                            const std::vector<IoByteSample>& dout)
{
    IoPointState st;
    st.raw = -1;
    st.isOn = -1;
    st.quality = "nosource";
    st.source = 0;

    // 只有 1203 列有讀取來源：MotionNet／ISA／PLC 在 V906 沒有接任何後端。
    if (isaBase != kIsaPci1203 || dir == kIoDirUnknown) return st;
    if (lane < 0 || ip < 0 || port < 0) return st;

    const int chan = port / 8;
    const int bit  = port % 8;
    const std::vector<IoByteSample>& v = (dir == kIoDirIn) ? di : dout;
    const IoByteSample* hit = 0;
    for (std::size_t i = 0; i < v.size(); ++i) {
        const IoByteSample& s = v[i];
        if (s.station != ip || s.stationChan != chan) continue;
        if (s.ring >= 0 && s.ring != lane) continue;   // flat 讀法（ring=-1）不比 ring
        hit = &s;
        break;
    }
    if (!hit) return st;

    st.source = (dir == kIoDirIn) ? "pci1203.di" : "pci1203.do";
    if (!hit->valid) { st.quality = "bad"; return st; }

    st.raw = (hit->byteData >> bit) & 1;
    if (enable != 1) { st.quality = "disabled"; return st; }
    st.isOn = inType ? st.raw : (st.raw ? 0 : 1);
    st.quality = "good";
    return st;
}

std::string IoConfigJson()
{
    const std::vector<std::string> ids = BuildIoIds();

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("2.0.0");
    w.Key("generatedAt").String(NowIsoUtc());
    w.Key("source").BeginObject();
    w.Key("ioTable").String(Str(IoTablePath));
    w.Key("producer").String("wb_serve: HSys.IOTable (the table C++ loaded at boot)");
    w.Key("rows").Number((wb_int64)HSys.IOTable.size());
    //AI(W906-CARDTYPE) 20261006 (Jerry, J-17 A): rows=0 的時候說出為什麼。支援的卡型回空字串 ⇒ 這兩個鍵是
    //  null／false，與本次改動前的輸出逐位元相同（新增的鍵本身是加的，頁面舊版忽略不認得的鍵）。
    w.Key("ioCardType").Number((wb_int64)IO_CARD_TYPE);
    {
        const std::string why = CardTypeUnsupportedWhy();
        w.Key("cardTypeSupported").Bool(why.empty());
        w.Key("cardTypeWhy"); if (why.empty()) w.Null(); else w.String(why);
    }
    w.EndObject();

    w.Key("enums").BeginObject();
    w.Key("ioType").BeginArray();
    const char* types[] = {"Sensor", "Switch", "Cylinder", "Cylinder_On", "Cylinder_Off",
                           "Sucker", "Sucker_On", "Sucker_Off"};
    for (std::size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) w.String(types[i]);
    w.EndArray();
    // MachineType.h eIOType（09-02 的舊檔把 3／4 寫反了）
    w.Key("isaBase").BeginObject();
    w.Key("0").String("eMotionNet");
    w.Key("1").String("eISABase");
    w.Key("2").String("ePCI1735U");
    w.Key("3").String("ePCI1203");
    w.Key("4").String("ePLCbase");
    w.EndObject();
    w.EndObject();

    std::map<std::string, std::vector<std::string> > byAlias, byCode;
    w.Key("points").BeginArray();
    for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
        if (ids[i].empty()) continue;
        const TIODATA* r = HSys.IOTable[i];
        const std::string type = Str(r->Type);
        const IoPointDir dir = IoDirectionOfType(type);
        const std::string code = IoCodeOf(r->iISABase, dir, r->iIP, r->iPort, r->iBit);
        byAlias[Str(r->Alias)].push_back(ids[i]);
        if (!code.empty()) byCode[code].push_back(ids[i]);

        w.BeginObject();
        w.Key("ioId").String(ids[i]);
        w.Key("alias").String(Str(r->Alias));
        w.Key("ioType").String(type);
        w.Key("direction").String(dir == kIoDirIn ? "input" : dir == kIoDirOut ? "output" : "unknown");
        w.Key("address").BeginObject();
        NumOrNull(w, "lane", r->iLane);
        NumOrNull(w, "ip", r->iIP);
        NumOrNull(w, "port", r->iPort);
        NumOrNull(w, "bit", r->iBit);
        w.Key("ioCode");
        if (code.empty()) w.Null(); else w.String(code);
        if (r->iISABase == kIsaPci1203 && r->iIP >= 0 && r->iPort >= 0) {
            // 監看器那一側的座標，現場對線用：站號、站內 byte、byte 內位元。
            w.Key("station").Number((wb_int64)r->iIP);
            w.Key("stationChan").Number((wb_int64)(r->iPort / 8));
            w.Key("stationBit").Number((wb_int64)(r->iPort % 8));
        }
        w.EndObject();
        w.Key("hw").BeginObject();
        NumOrNull(w, "moduleType", r->iModuleType);
        w.Key("isaBase").Number((wb_int64)r->iISABase);
        w.Key("inType").Number((wb_int64)r->iInType);
        w.Key("enable").Number((wb_int64)r->iEnable);
        // AI(W906-IOWIDGET-3) 20261001: an output that needs the safe door closed while idle -- golden paints its
        // button orange / yellow with black text (iosetview.cpp:2244-2258); the interlock itself is MyLaneIO's
        w.Key("needSafeDoor").Bool(dir == kIoDirOut && NeedSafeDoor(r));
        w.EndObject();
        w.Key("timing").BeginObject();
        NumOrNull(w, "onAlarmTime", r->iOnAlarmTime);
        NumOrNull(w, "offAlarmTime", r->iOffAlarmTime);
        NumOrNull(w, "onDelayTime", r->iOnDelayTime);
        NumOrNull(w, "offDelayTime", r->iOffDelayTime);
        w.EndObject();
        w.Key("ui").BeginObject();
        w.Key("tabs").BeginArray().EndArray();
        w.Key("componentNames").BeginArray().EndArray();
        w.EndObject();
        w.Key("note").String("");
        w.Key("row").Number((wb_int64)i);
        w.EndObject();
    }
    w.EndArray();

    w.Key("index").BeginObject();
    w.Key("byId").BeginObject();
    for (std::size_t i = 0; i < ids.size(); ++i)
        if (!ids[i].empty()) w.Key(ids[i]).Bool(true);
    w.EndObject();
    w.Key("byAlias").BeginObject();
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = byAlias.begin();
         it != byAlias.end(); ++it) {
        w.Key(it->first).BeginArray();
        for (std::size_t k = 0; k < it->second.size(); ++k) w.String(it->second[k]);
        w.EndArray();
    }
    w.EndObject();
    w.Key("byIOCode").BeginObject();
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = byCode.begin();
         it != byCode.end(); ++it) {
        w.Key(it->first).BeginArray();
        for (std::size_t k = 0; k < it->second.size(); ++k) w.String(it->second[k]);
        w.EndArray();
    }
    w.EndObject();
    w.EndObject();

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::string IoRuntimeJsonFrom(const std::vector<IoByteSample>& di,
                              const std::vector<IoByteSample>& dout,
                              bool connected, const std::string& nowIso)
{
    return IoRuntimeJsonFrom(di, dout, connected, nowIso, 0);
}

std::string IoRuntimeJsonFrom(const std::vector<IoByteSample>& di,
                              const std::vector<IoByteSample>& dout,
                              bool connected, const std::string& nowIso,
                              const IoMonitorStatus* st)
{
    const std::vector<std::string> ids = BuildIoIds();
    const unsigned long long seq = ++g_rtSeq;

    int nGood = 0, nBad = 0, nNoSrc = 0, nDisabled = 0;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("2.0.0");
    w.Key("points").BeginArray();
    for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
        if (ids[i].empty()) continue;
        const TIODATA* r = HSys.IOTable[i];
        IoPointState s = ResolveIoPoint(r->iISABase, r->iLane, r->iIP, r->iPort,
                                              r->iInType, r->iEnable,
                                              IoDirectionOfType(Str(r->Type)), di, dout);   { bool pb = false; if (::W906_PadIoPoint(r->Alias, &pb)) { const bool key = IoDirectionOfType(Str(r->Type)) != kIoDirOut; s.isOn = (key && r->iInType == 0) ? !pb : pb; s.raw = pb ? 1 : 0; s.source = "pad"; s.quality = "good"; } }   // AI(W906-W155) 20261007 (St02-E): ControlPanelMode 1 (golden iosetview.cpp ScanLed :2778-2782) -- a pad key = ProcessScanKey with the InType inversion of :2795-2796, a pad button = its lamp state (bPadStatus); mode 0 / other aliases unchanged
        const std::string q = s.quality;
        if (q == "good") ++nGood;
        else if (q == "bad") ++nBad;
        else if (q == "disabled") ++nDisabled;
        else ++nNoSrc;

        w.BeginObject();
        w.Key("ioId").String(ids[i]);
        w.Key("isOn");
        if (s.isOn < 0) w.Null(); else w.Bool(s.isOn == 1);
        w.Key("isOff");
        if (s.isOn < 0) w.Null(); else w.Bool(s.isOn == 0);
        w.Key("state").String(s.isOn < 0 ? "unknown" : (s.isOn ? "on" : "off"));
        w.Key("updatedAt");
        if (s.raw < 0) w.Null(); else w.String(nowIso);
        w.Key("source");
        if (s.source) w.String(s.source); else w.Null();
        w.Key("quality").String(q);
        w.Key("raw");
        if (s.raw < 0) w.Null(); else w.Number((wb_int64)s.raw);
        w.Key("seq").Number((wb_int64)seq);
        w.EndObject();
    }
    w.EndArray();

    w.Key("runtime").BeginObject();
    w.Key("connected").Bool(connected);
    w.Key("lastPollAt").String(nowIso);
    w.Key("pollIntervalMs").Number((wb_int64)kPollIntervalMs);
    w.Key("provider").String("wb_serve:pci1203-monitor");
    w.Key("seq").Number((wb_int64)seq);
    w.Key("counts").BeginObject();
    w.Key("good").Number((wb_int64)nGood);
    w.Key("bad").Number((wb_int64)nBad);
    w.Key("nosource").Number((wb_int64)nNoSrc);
    w.Key("disabled").Number((wb_int64)nDisabled);
    w.EndObject();
    //AI(W906-CARDTYPE) 20261006 (Jerry, J-17 A): 空表時說出為什麼（支援的卡型 ⇒ 不輸出這個鍵，輸出與改動前相同）。
    {
        const std::string why = CardTypeUnsupportedWhy();
        if (!why.empty()) w.Key("why").String(why);
    }
    if (st) {
        // AI(W906-IOOBS) 20260925: 監看器狀態原樣帶出（見 ChanIo.h IoMonitorStatus）。
        w.Key("monitor").BeginObject();
        w.Key("present").Bool(st->present);
        w.Key("linked").Bool(st->linked);
        w.Key("open").Bool(st->open);
        w.Key("disabled").Bool(st->disabled);
        w.Key("disabledReason");
        if (st->disabledReason.empty()) w.Null(); else w.String(st->disabledReason);
        w.Key("pollCount").Number((wb_int64)st->pollCount);
        w.Key("pollErrors").Number((wb_int64)st->pollErrors);
        w.Key("pollMs").Number((wb_int64)st->pollMs);
        // AI(W906-LAT-1) 20260925: Poll 四段的實測（與 pci1203.poll.stationsMs/axesMs/diMs/doMs 同源），沒有完整量過一輪就是 null、不是 0。
        w.Key("stationsMs"); if (st->pollSegValid) w.Number(st->pollStationsMs); else w.Null();
        w.Key("axesMs");     if (st->pollSegValid) w.Number(st->pollAxesMs);     else w.Null();
        w.Key("diMs");       if (st->pollSegValid) w.Number(st->pollDiMs);       else w.Null();
        w.Key("doMs");       if (st->pollSegValid) w.Number(st->pollDoMs);       else w.Null();
        // AI(W906-MT-E3a) 20260925: DI 讀法（"batch"／"perByte"）、比對不一致的次數與原因 —— 讓畫面講出「這些值是哪一種讀法讀來的」，不要用猜的。
        w.Key("diMode");            if (st->diBatchKnown) w.String(st->diBatchOk ? "batch" : "perByte"); else w.Null();
        w.Key("diBatchMismatches"); if (st->diBatchKnown) w.Number((wb_int64)st->diBatchMismatches);    else w.Null();
        w.Key("diBatchChecks");     if (st->diBatchKnown) w.Number((wb_int64)st->diBatchChecks);        else w.Null();
        w.Key("diBatchFails");      if (st->diBatchKnown) w.Number((wb_int64)st->diBatchFails);         else w.Null();
        w.Key("diBatchWhy");        if (st->diBatchKnown && !st->diBatchWhy.empty()) w.String(st->diBatchWhy); else w.Null();
        w.EndObject();
    }
    if (!connected) {
        if (st && st->present && st->disabled)
            w.Key("why").String("1203 監看器已自己停用（" + st->disabledReason + "）。每一點都是 null，不是 off；"
                                "畫面上的值不會再更新。");
        else if (st && st->present && st->linked && !st->open)
            w.Key("why").String("1203 監看器沒有開到卡（見開機主控台的 card NOT opened）。每一點都是 null，不是 off。");
        else
            w.Key("why").String("1203 監看器沒有採到任何 port（沒有卡，或這顆 binary 沒有 HAVE_PCI1203）。"
                                "每一點都是 null，不是 off。");
    }
    w.EndObject();

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

std::string IoRuntimeJson()
{
    std::vector<IoByteSample> di, dout;
    TPci1203Monitor* mon = Pci1203Monitor();
    const int nDi = mon ? mon->diCount() : 0;
    const int nDo = mon ? mon->doCount() : 0;
    for (int i = 0; i < nDi; ++i) {
        const Pci1203DiSample& s = mon->di(i);
        IoByteSample b;
        b.valid = s.valid;
        b.ring = s.flat ? -1 : s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        di.push_back(b);
    }
    for (int i = 0; i < nDo; ++i) {
        const Pci1203DoSample& s = mon->do_(i);
        IoByteSample b;
        b.valid = s.valid;
        b.ring = s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        dout.push_back(b);
    }
    // 「有沒有資料源」的判準與 StageIo 相同：真的採到至少一個 port。
    // AI(W906-IOOBS) 20260925: 另外要求監看器手上真的有卡、而且沒有自己停用 —— 停用後 diCount()
    //   仍保留最後一次的樣本數，原本的判準會一直回 connected＝true、值卻已凍結。
    IoMonitorStatus st;
    st.present = (mon != 0);
    st.linked = false; st.open = false; st.disabled = false;
    st.pollCount = 0; st.pollErrors = 0; st.pollMs = 0;
    if (mon) {
        const Pci1203CardSample& c = mon->card();
        st.linked = c.linked;
        st.open = c.open;
        st.disabled = mon->Disabled();
        st.disabledReason = mon->disabledReason();
        st.pollCount = c.pollCount;
        st.pollErrors = c.pollErrors;
        st.pollMs = c.pollMs;
        st.pollSegValid = c.pollSegValid;   // AI(W906-LAT-1) 20260925
        st.pollStationsMs = c.pollStationsMs; st.pollAxesMs = c.pollAxesMs; st.pollDiMs = c.pollDiMs; st.pollDoMs = c.pollDoMs;
        st.diBatchKnown = c.open;   // AI(W906-MT-E3a) 20260925: 卡沒開就沒有「讀法」可言 -> null
        st.diBatchOk = c.diBatchOk; st.diBatchMismatches = c.diBatchMismatches; st.diBatchChecks = c.diBatchChecks; st.diBatchFails = c.diBatchFails; st.diBatchWhy = c.diBatchWhy;
    }
    const bool connected = (mon != 0) && st.open && !st.disabled && (nDi > 0 || nDo > 0);
    return IoRuntimeJsonFrom(di, dout, connected, NowIsoUtc(), &st);
}

// AI(W906-IOWEB-P17) 20260925: see ChanIo.h for why this is stricter than
// ResolveIoPoint.
int PickIoSample(const std::vector<IoByteSample>& v, int ring, int station, int stationChan,
                 const char** why)
{
    const char* ignored = 0;
    if (!why) why = &ignored;
    *why = 0;
    if (ring < 0 || station <= 0 || stationChan < 0) { *why = "bad-address"; return -1; }
    for (std::size_t i = 0; i < v.size(); ++i) {
        const IoByteSample& s = v[i];
        if (s.ring < 0 || s.station <= 0 || s.stationChan < 0) continue;   // flat / unattributed
        if (s.ring == ring && s.station == station && s.stationChan == stationChan)
            return (int)i;
    }
    *why = "not-in-card-map";
    return -1;
}

}  // namespace sjson
}  // namespace ht9045
