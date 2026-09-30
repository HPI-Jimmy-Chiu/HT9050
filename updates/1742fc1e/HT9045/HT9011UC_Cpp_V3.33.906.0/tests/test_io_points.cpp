// ===========================================================================
//  tests/test_io_points.cpp
//
//  AI(W906-IO-POINTS) 20260924: JsonBridge/ChanIoPoints.cpp 的測試
//  （GET /api/struct/io/config 與 /api/struct/io/runtime 的產生器）。
//
//  這台筆電沒有 1203 卡，所以「真的讀到卡片」只能在機台端驗；這支測試負責
//  機台端之前能驗的全部：
//    [1] 位址解析 ResolveIoPoint：通道 -> (站內 byte, 位元)、ring 比對、InType 反相、
//        讀失敗／沒來源／停用的三種 null、輸入讀 DI 輸出讀 DO 回授。
//    [2] 讀版控的 machines/HT9050/IO_Table.csv（argv[1]），IoConfigJson 的形狀：
//        點數、ioId 唯一、SnMotorPower 的位址碼與站號座標、isaBase 列舉（舊檔寫反）。
//    [3] 方向規則與 golden 綁定交叉驗證：InitialSwitch 綁上的每個 SW[] 必須是 output、
//        InitialSensor 綁上的每個 Sen[] 必須是 input。方向錯＝讀錯平面（DI／DO）。
//    [4] IoRuntimeJsonFrom：沒有樣本 -> 全部 null；給每個啟用站號假樣本 -> 啟用的點全部 good，
//        而且逐點的 on/off 與「byte 的那一位元再依 InType 換算」相同。
//    [5] AI(W906-IOWEB-P17) 20260925: PickIoSample（寫入／引擎讀取用的嚴格挑選）：
//        ring 決定是哪一個裝置、不接受 flat、第一筆勝出、與顯示端挑到同一個 byte。
//    [6] AI(W906-IOWEB-P17) 20260925: A4-7 的 TPci1203Backend 路由：沒裝 = 原本的樁；
//        裝了 = MyLaneIO.IOBitOn／IOBitOff／IOInputBit 的 Ring／IP／Port 原封不動送到路由。
//
//  用法：test_io_points <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"      // AI(W906-IOWEB-P17) 20260925: SOFT_SIMULTE / ePCI1203 before [6]'s #ifndef (tools/macro_order_gate.ps1)
#include "IOBackend.h"        // AI(W906-IOWEB-P17) 20260925: TPci1203Backend / SetPci1203IoRoute
#include "MyLaneIo.h"         // AI(W906-IOWEB-P17) 20260925: MyLaneIO
#include "JsonBridge/ChanIo.h"
#include "Public/cJSON.h"
#include "database.h"
#include "cinitial.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "myswitch.h"
#include "mysensor.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

using namespace ht9045::sjson;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_io_points.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static IoByteSample S(int ring, int station, int chan, unsigned char b, bool valid = true) {
    IoByteSample s; s.valid = valid; s.ring = ring; s.station = station; s.stationChan = chan; s.byteData = b;
    return s;
}

static std::string SOf(cJSON* o, const char* k) {
    cJSON* v = cJSON_GetObjectItem(o, k);
    return (v && v->valuestring) ? std::string(v->valuestring) : std::string();
}

// AI(W906-IOWEB-P17) 20260925: [6] 的假路由 —— 記下收到的參數，回呼叫端指定的結果。
struct FakeRouteLog { int calls, ring, ip, port, bit, value; int ret; unsigned char readValue; };
static FakeRouteLog g_fr;
static void FrReset(int ret, unsigned char readValue) {
    g_fr.calls = 0; g_fr.ring = g_fr.ip = g_fr.port = g_fr.bit = g_fr.value = -99;
    g_fr.ret = ret; g_fr.readValue = readValue;
}
static int FrWriteBit(int r, int ip, int p, int b, int v) { ++g_fr.calls; g_fr.ring = r; g_fr.ip = ip; g_fr.port = p; g_fr.bit = b; g_fr.value = v; return g_fr.ret; }
static int FrWriteByte(int r, int ip, int p, unsigned int v) { ++g_fr.calls; g_fr.ring = r; g_fr.ip = ip; g_fr.port = p; g_fr.value = (int)v; return g_fr.ret; }
static int FrReadBit(int r, int ip, int p, int b, unsigned char* v) { ++g_fr.calls; g_fr.ring = r; g_fr.ip = ip; g_fr.port = p; g_fr.bit = b; if (v) *v = g_fr.readValue; return g_fr.ret; }
static int FrReadByte(int r, int ip, int p, unsigned char* v) { ++g_fr.calls; g_fr.ring = r; g_fr.ip = ip; g_fr.port = p; if (v) *v = g_fr.readValue; return g_fr.ret; }

int main(int argc, char** argv)
{
    // ---------------------------------------------------------------- [1]
    std::printf("[1] ResolveIoPoint\n");
    {
        std::vector<IoByteSample> di, dout;
        di.push_back(S(0, 2, 3, 0x40));       // 站 2 第 3 個 byte，只有 bit6 = 1
        dout.push_back(S(0, 2, 3, 0x00));     // 同座標的 DO 回授全 0（驗「輸入讀 DI、輸出讀 DO」）

        IoPointState a = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirIn, di, dout);   // 30 = byte3 bit6
        CHECK(a.raw == 1 && a.isOn == 1);
        CHECK(std::strcmp(a.quality, "good") == 0);
        CHECK(a.source && std::strcmp(a.source, "pci1203.di") == 0);

        IoPointState b = ResolveIoPoint(3, 0, 2, 30, 0, 1, kIoDirIn, di, dout);   // InType 0 反相
        CHECK(b.raw == 1 && b.isOn == 0);

        IoPointState c = ResolveIoPoint(3, 0, 2, 29, 1, 1, kIoDirIn, di, dout);   // bit5 = 0
        CHECK(c.raw == 0 && c.isOn == 0);

        IoPointState d = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirOut, di, dout);  // 輸出讀 DO
        CHECK(d.raw == 0 && d.isOn == 0);
        CHECK(d.source && std::strcmp(d.source, "pci1203.do") == 0);

        IoPointState e = ResolveIoPoint(3, 0, 2, 30, 1, 0, kIoDirIn, di, dout);   // Enable=0
        CHECK(e.raw == 1 && e.isOn == -1 && std::strcmp(e.quality, "disabled") == 0);

        std::vector<IoByteSample> bad; bad.push_back(S(0, 2, 3, 0xFF, false));
        IoPointState f = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirIn, bad, dout);  // 讀失敗
        CHECK(f.raw == -1 && f.isOn == -1 && std::strcmp(f.quality, "bad") == 0);

        std::vector<IoByteSample> ring1; ring1.push_back(S(1, 2, 3, 0xFF));
        IoPointState g = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirIn, ring1, dout); // ring 不同
        CHECK(g.isOn == -1 && std::strcmp(g.quality, "nosource") == 0);

        std::vector<IoByteSample> flat; flat.push_back(S(-1, 2, 3, 0x40));
        IoPointState h = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirIn, flat, dout);  // flat 不比 ring
        CHECK(h.isOn == 1);

        IoPointState i = ResolveIoPoint(3, 0, 5, 30, 1, 1, kIoDirIn, di, dout);   // 沒這個站
        CHECK(i.isOn == -1 && i.source == 0 && std::strcmp(i.quality, "nosource") == 0);

        IoPointState j = ResolveIoPoint(0, 0, 2, 30, 1, 1, kIoDirIn, di, dout);   // MotionNet：沒後端
        CHECK(j.isOn == -1 && std::strcmp(j.quality, "nosource") == 0);

        IoPointState k = ResolveIoPoint(3, 0, 2, 30, 1, 1, kIoDirUnknown, di, dout);
        CHECK(k.isOn == -1);

        IoPointState l = ResolveIoPoint(3, 0, 2, 30, 2, 1, kIoDirIn, di, dout);   // InType 2 = 非 0 = 不反相（golden `if(Type)`）
        CHECK(l.isOn == 1);

        CHECK(IoDirectionOfType("Sensor") == kIoDirIn);
        CHECK(IoDirectionOfType("Cylinder_On") == kIoDirIn);
        CHECK(IoDirectionOfType("Cylinder_Off") == kIoDirIn);
        CHECK(IoDirectionOfType("Sucker") == kIoDirIn);
        CHECK(IoDirectionOfType("Switch") == kIoDirOut);
        CHECK(IoDirectionOfType("Cylinder") == kIoDirOut);
        CHECK(IoDirectionOfType("Sucker_On") == kIoDirOut);
        CHECK(IoDirectionOfType("Sucker_Off") == kIoDirOut);
        CHECK(IoDirectionOfType("") == kIoDirUnknown);

        CHECK(IoCodeOf(3, kIoDirIn, 2, 30, 6) == "I2.30");
        CHECK(IoCodeOf(0, kIoDirIn, 1, 0, 1) == "I0101");     // 舊產生器的格式（IO-config.json 09-02）
        CHECK(IoCodeOf(0, kIoDirOut, 1, 2, 0) == "O0120");
        CHECK(IoCodeOf(3, kIoDirUnknown, 2, 30, 6) == "");
    }

    if (argc < 2) { std::printf("usage: test_io_points <IO_Table.csv>\n"); return 2; }
    const std::string path = argv[1];
    const std::string env = "W906_IOTABLE_PATH=" + path;
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
    HSys.LoadIoData();
    std::printf("IO_Table rows loaded: %d\n", (int)HSys.IOTable.size());
    CHECK(HSys.IOTable.size() == 1124);   // AI(W906-IOWEB-P7) 20260924: field table (09-24 rows)

    // ---------------------------------------------------------------- [2]
    std::printf("[2] IoConfigJson\n");
    std::map<std::string, std::string> dirById, dirByAlias;
    std::map<std::string, cJSON*> ptById;
    cJSON* cfg = cJSON_Parse(IoConfigJson().c_str());
    CHECK(cfg != 0);
    int nPts = 0;
    if (cfg) {
        cJSON* pts = cJSON_GetObjectItem(cfg, "points");
        nPts = pts ? cJSON_GetArraySize(pts) : 0;
        std::printf("config points: %d\n", nPts);
        CHECK(nPts == 967);                    // 非空 Alias 的列（Python 另算：905）   // AI(W906-IOWEB-P7) 20260924: 967 non-empty-Alias rows (independent count; same rule gives 905 on the 09-23 table)
        std::set<std::string> ids;
        for (int n = 0; n < nPts; ++n) {
            cJSON* p = cJSON_GetArrayItem(pts, n);
            const std::string id = SOf(p, "ioId");
            ids.insert(id);
            dirById[id] = SOf(p, "direction");
            dirByAlias[SOf(p, "alias")] = SOf(p, "direction");
            ptById[id] = p;
        }
        CHECK((int)ids.size() == nPts);        // ioId 唯一
        cJSON* byId = cJSON_GetObjectItem(cJSON_GetObjectItem(cfg, "index"), "byId");
        CHECK(byId && cJSON_GetArraySize(byId) == nPts);

        CHECK(ptById.count("SnMotorPower@I2.30") == 1);
        if (ptById.count("SnMotorPower@I2.30")) {
            cJSON* p = ptById["SnMotorPower@I2.30"];
            cJSON* ad = cJSON_GetObjectItem(p, "address");
            CHECK(SOf(p, "direction") == "input");
            CHECK(cJSON_GetObjectItem(ad, "station")->valueint == 2);
            CHECK(cJSON_GetObjectItem(ad, "stationChan")->valueint == 3);
            CHECK(cJSON_GetObjectItem(ad, "stationBit")->valueint == 6);
            CHECK(cJSON_GetObjectItem(cJSON_GetObjectItem(p, "hw"), "isaBase")->valueint == 3);
        }
        cJSON* isa = cJSON_GetObjectItem(cJSON_GetObjectItem(cfg, "enums"), "isaBase");
        CHECK(SOf(isa, "3") == "ePCI1203");
        CHECK(SOf(isa, "4") == "ePLCbase");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] direction vs golden binding\n");
    {
        IO_CARD_TYPE = PCI1203_IO;    // AI(W906-IOWEB-P7) 20260924: this machine's value (IOWEB-P4)
        iControlPanelMode = 0;
        Enable_PLCSafety_IO = false;
        CosFunction.bTTLCanUse8Site = false;
        InitialSwitch();
        InitialSensor();
        int swChecked = 0, swWrong = 0, snChecked = 0, snWrong = 0;
        for (int i = 0; i < MAX_SWITCH_ITEM; ++i) {
            const std::string nm = SW[i].Name.c_str();
            if (nm.empty() || !dirByAlias.count(nm)) continue;
            ++swChecked;
            if (dirByAlias[nm] != "output") { ++swWrong; std::printf("  SW %s -> %s\n", nm.c_str(), dirByAlias[nm].c_str()); }
        }
        for (int i = 0; i < MAX_SENSOR_ITEM; ++i) {
            const std::string nm = Sen[i].Name.c_str();
            if (nm.empty() || !dirByAlias.count(nm)) continue;
            ++snChecked;
            if (dirByAlias[nm] != "input") { ++snWrong; if (snWrong <= 10) std::printf("  Sen %s -> %s\n", nm.c_str(), dirByAlias[nm].c_str()); }
        }
        std::printf("SW checked %d wrong %d | Sen checked %d wrong %d\n", swChecked, swWrong, snChecked, snWrong);
        CHECK(swChecked == 136 && snChecked == 301);   // 與 MachineIoTable_HT9050 同一組獨立算出的數字   // AI(W906-IOWEB-P7) 20260924: field table; AI(W906-BRAKE-EMPTY-AUTO3) 20260930: 134 -> 136, cpp 0039's SW 369 / 370 (IO_Table rows 1122 / 1125)
        CHECK(swWrong == 0);
        CHECK(snWrong == 0);
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] IoRuntimeJsonFrom\n");
    {
        std::vector<IoByteSample> none;
        cJSON* rt = cJSON_Parse(IoRuntimeJsonFrom(none, none, false, "2026-09-24T00:00:00.000Z").c_str());
        CHECK(rt != 0);
        if (rt) {
            cJSON* pts = cJSON_GetObjectItem(rt, "points");
            CHECK(cJSON_GetArraySize(pts) == nPts);
            int nonNull = 0;
            for (int n = 0; n < cJSON_GetArraySize(pts); ++n)
                if (!cJSON_IsNull(cJSON_GetObjectItem(cJSON_GetArrayItem(pts, n), "isOn"))) ++nonNull;
            CHECK(nonNull == 0);                                  // 沒來源 = 全部 null，不是 off
            cJSON* runtime = cJSON_GetObjectItem(rt, "runtime");
            CHECK(!cJSON_IsTrue(cJSON_GetObjectItem(runtime, "connected")));
            CHECK(cJSON_GetObjectItem(cJSON_GetObjectItem(runtime, "counts"), "nosource")->valueint == nPts);
            cJSON_Delete(rt);
        }

        // AI(W906-IOOBS) 20260925: 帶監看器狀態的版本 —— 監看器自己停用時，monitor 區塊與 why 都要講出來；
        //   舊的 4 參數版本不帶 monitor 區塊（回應形狀不變）。
        {
            IoMonitorStatus st;
            st.present = true; st.linked = true; st.open = true; st.disabled = true;
            st.disabledReason = "10 consecutive read failures";
            st.pollCount = 42; st.pollErrors = 10; st.pollMs = 3;
            cJSON* r3 = cJSON_Parse(IoRuntimeJsonFrom(none, none, false, "2026-09-25T00:00:00.000Z", &st).c_str());
            CHECK(r3 != 0);
            if (r3) {
                cJSON* runtime = cJSON_GetObjectItem(r3, "runtime");
                cJSON* mon = cJSON_GetObjectItem(runtime, "monitor");
                CHECK(mon != 0);
                if (mon) {
                    CHECK(cJSON_IsTrue(cJSON_GetObjectItem(mon, "disabled")));
                    CHECK(cJSON_GetObjectItem(mon, "pollCount")->valueint == 42);
                    CHECK(cJSON_GetObjectItem(mon, "disabledReason") != 0 &&
                          !cJSON_IsNull(cJSON_GetObjectItem(mon, "disabledReason")));
                    // AI(W906-MT-E3a) 20260925: 沒填 DI 讀法（diBatchKnown=false）⇒ diMode 等欄位是 null，不是 "perByte"
                    CHECK(cJSON_GetObjectItem(mon, "diMode") != 0 && cJSON_IsNull(cJSON_GetObjectItem(mon, "diMode")));
                    CHECK(cJSON_IsNull(cJSON_GetObjectItem(mon, "diBatchMismatches")));
                }
                // AI(W906-MT-E3a) 20260925: pollIntervalMs = 200（1203 監看器 Poll 的節拍），頁面照它走
                CHECK(cJSON_GetObjectItem(runtime, "pollIntervalMs") != 0 &&
                      cJSON_GetObjectItem(runtime, "pollIntervalMs")->valueint == 200);
                cJSON* why = cJSON_GetObjectItem(runtime, "why");
                CHECK(why != 0 && why->valuestring != 0 && std::string(why->valuestring).find("停用") != std::string::npos);
                cJSON_Delete(r3);
            }
            // AI(W906-MT-E3a) 20260925: 填了 DI 讀法 ⇒ monitor 區塊原樣帶出（batch／不一致次數／原因）
            {
                IoMonitorStatus sb;
                sb.present = true; sb.linked = true; sb.open = true; sb.disabled = false;
                sb.pollCount = 7; sb.pollErrors = 0; sb.pollMs = 40;
                sb.diBatchKnown = true; sb.diBatchOk = true; sb.diBatchMismatches = 2; sb.diBatchChecks = 5;
                sb.diBatchFails = 1; sb.diBatchWhy = "batch: test";
                cJSON* rb = cJSON_Parse(IoRuntimeJsonFrom(none, none, false, "2026-09-25T00:00:00.000Z", &sb).c_str());
                CHECK(rb != 0);
                if (rb) {
                    cJSON* mb = cJSON_GetObjectItem(cJSON_GetObjectItem(rb, "runtime"), "monitor");
                    CHECK(mb != 0 && std::string(SOf(mb, "diMode")) == "batch");
                    CHECK(mb != 0 && cJSON_GetObjectItem(mb, "diBatchMismatches")->valueint == 2);
                    CHECK(mb != 0 && cJSON_GetObjectItem(mb, "diBatchChecks")->valueint == 5);
                    CHECK(mb != 0 && std::string(SOf(mb, "diBatchWhy")) == "batch: test");
                    cJSON_Delete(rb);
                }
                sb.diBatchOk = false;
                cJSON* rp = cJSON_Parse(IoRuntimeJsonFrom(none, none, false, "2026-09-25T00:00:00.000Z", &sb).c_str());
                CHECK(rp != 0);
                if (rp) {
                    CHECK(std::string(SOf(cJSON_GetObjectItem(cJSON_GetObjectItem(rp, "runtime"), "monitor"), "diMode")) == "perByte");
                    cJSON_Delete(rp);
                }
            }
            cJSON* r4 = cJSON_Parse(IoRuntimeJsonFrom(none, none, false, "2026-09-25T00:00:00.000Z").c_str());
            CHECK(r4 != 0);
            if (r4) {
                CHECK(cJSON_GetObjectItem(cJSON_GetObjectItem(r4, "runtime"), "monitor") == 0);
                cJSON_Delete(r4);
            }
        }

        // 每個啟用的 1203 站號都給樣本：DI byte = 0xA5、DO byte = 0x3C（兩個不同圖樣，
        // 讀錯平面或位元序就會對不上）。
        std::set<std::pair<int,int> > inKeys, outKeys;
        for (std::size_t r = 0; r < HSys.IOTable.size(); ++r) {
            const TIODATA* t = HSys.IOTable[r];
            if (t->iISABase != 3 || t->iEnable != 1 || t->iIP < 0 || t->iPort < 0) continue;
            const IoPointDir d = IoDirectionOfType(t->Type.c_str());
            if (d == kIoDirIn)  inKeys.insert(std::make_pair(t->iIP, t->iPort / 8));
            if (d == kIoDirOut) outKeys.insert(std::make_pair(t->iIP, t->iPort / 8));
        }
        std::vector<IoByteSample> di, dout;
        for (std::set<std::pair<int,int> >::const_iterator it = inKeys.begin(); it != inKeys.end(); ++it)
            di.push_back(S(1, it->first, it->second, 0xA5));   // AI(W906-IOWEB-P7) 20260924: fake samples model the REAL card: IO modules report ring 1
        for (std::set<std::pair<int,int> >::const_iterator it = outKeys.begin(); it != outKeys.end(); ++it)
            dout.push_back(S(1, it->first, it->second, 0x3C));   // AI(W906-IOWEB-P7) 20260924: same

        cJSON* rt2 = cJSON_Parse(IoRuntimeJsonFrom(di, dout, true, "2026-09-24T00:00:00.000Z").c_str());
        CHECK(rt2 != 0);
        if (rt2) {
            cJSON* runtime = cJSON_GetObjectItem(rt2, "runtime");
            cJSON* counts = cJSON_GetObjectItem(runtime, "counts");
            const int good = cJSON_GetObjectItem(counts, "good")->valueint;
            std::printf("runtime with samples: good %d bad %d nosource %d disabled %d\n",
                        good, cJSON_GetObjectItem(counts, "bad")->valueint,
                        cJSON_GetObjectItem(counts, "nosource")->valueint,
                        cJSON_GetObjectItem(counts, "disabled")->valueint);
            CHECK(good == 259);                                  // HT9050 表 Enable=1 的列（Python 另算：236，全是 1203）   // AI(W906-IOWEB-P7) 20260924: field table -- 259 enabled 1203 points (independent count; same rule gives 236 on the 09-23 table)

            // 逐點對帳：on/off 必須等於「那個 byte 的 Port%8 位元，再依 InType 換算」。
            std::map<std::string, const TIODATA*> rowById;
            {
                // 重建 ioId -> row（與產生器同一條規則：alias@ioCode）
                for (std::size_t r = 0; r < HSys.IOTable.size(); ++r) {
                    const TIODATA* t = HSys.IOTable[r];
                    const std::string a = t->Alias.c_str();
                    if (a.empty()) continue;
                    const std::string code = IoCodeOf(t->iISABase, IoDirectionOfType(t->Type.c_str()), t->iIP, t->iPort, t->iBit);
                    rowById[a + "@" + code] = t;
                }
            }
            int verified = 0, wrong = 0;
            cJSON* pts = cJSON_GetObjectItem(rt2, "points");
            for (int n = 0; n < cJSON_GetArraySize(pts); ++n) {
                cJSON* p = cJSON_GetArrayItem(pts, n);
                if (SOf(p, "quality") != "good") continue;
                const std::string id = SOf(p, "ioId");
                if (!rowById.count(id)) { ++wrong; continue; }
                const TIODATA* t = rowById[id];
                const bool isIn = IoDirectionOfType(t->Type.c_str()) == kIoDirIn;
                const int rawBit = ((isIn ? 0xA5 : 0x3C) >> (t->iPort % 8)) & 1;
                const bool expectOn = t->iInType ? (rawBit == 1) : (rawBit == 0);
                const bool gotOn = cJSON_IsTrue(cJSON_GetObjectItem(p, "isOn"));
                if (gotOn != expectOn || cJSON_GetObjectItem(p, "raw")->valueint != rawBit) ++wrong;
                ++verified;
            }
            std::printf("per-point verified %d wrong %d\n", verified, wrong);
            CHECK(verified == 259);   // AI(W906-IOWEB-P7) 20260924: field table
            CHECK(wrong == 0);

            // SnMotorPower：站 2 byte3 = 0xA5，bit6 = 0 ⇒ raw 0，InType 1 ⇒ off
            for (int n = 0; n < cJSON_GetArraySize(pts); ++n) {
                cJSON* p = cJSON_GetArrayItem(pts, n);
                if (SOf(p, "ioId") != "SnMotorPower@I2.30") continue;
                CHECK(SOf(p, "state") == "off");
                CHECK(cJSON_GetObjectItem(p, "raw")->valueint == 0);
            }
            cJSON_Delete(rt2);
        }
    }
    if (cfg) cJSON_Delete(cfg);

    // ---------------------------------------------------------------- [5]
    std::printf("[5] PickIoSample\n");
    {
        std::vector<IoByteSample> v;
        v.push_back(S(-1, -1, -1, 0x11));        // flat, unattributed
        v.push_back(S(0, 1, 2, 0x22));           // ring 0 station 1 byte 2 -- the SERVOPACK side (card map 20260924)
        v.push_back(S(1, 1, 2, 0x33));           // ring 1 station 1 byte 2 -- the 32DO side
        v.push_back(S(1, 176, 0, 0x44, false));  // did not read back
        v.push_back(S(1, 176, 0, 0x55));         // same address again, after the unreadable one
        v.push_back(S(-1, 16, 0, 0x66));         // attributed station but no ring

        const char* why = 0;
        CHECK(PickIoSample(v, 1, 1, 2, &why) == 2 && why == 0);
        CHECK(PickIoSample(v, 0, 1, 2, &why) == 1);                   // the ring decides which device
        CHECK(PickIoSample(v, 2, 1, 2, &why) == -1 && why && std::strcmp(why, "not-in-card-map") == 0);
        CHECK(PickIoSample(v, -1, 1, 2, &why) == -1 && why && std::strcmp(why, "bad-address") == 0);   // no flat fallback
        CHECK(PickIoSample(v, 1, 0, 2, &why) == -1 && why && std::strcmp(why, "bad-address") == 0);    // station 0 = would take Execute's flat path
        CHECK(PickIoSample(v, 1, 1, -1, &why) == -1 && why && std::strcmp(why, "bad-address") == 0);
        CHECK(PickIoSample(v, 1, 176, 0, &why) == 3);                 // first match; `valid` is the caller's call
        CHECK(PickIoSample(v, 1, 16, 0, &why) == -1);                 // ring -1 never qualifies, even with a station
        CHECK(PickIoSample(v, 1, 1, 2, 0) == 2);                      // why may be null

        // The lamp and the write must land on the SAME byte for a Lane=1 row.
        const IoPointState st = ResolveIoPoint(3, 1, 1, 17, 1, 1, kIoDirOut, v, v);   // port 17 = byte 2, bit 1
        CHECK(st.raw == ((0x33 >> 1) & 1));
        CHECK(std::strcmp(st.quality, "good") == 0);
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] TPci1203Backend route (A4-7)\n");
    {
        TPci1203Backend be;
        unsigned char val = 9;
        CHECK(Pci1203IoRoute() == 0);                                 // nothing installs one in a test
        CHECK(be.WriteBit(1, 16, 3, 3, 1) == 0);                      // stub: "success", nothing done
        CHECK(be.ReadBit(1, 16, 3, 3, &val) == 0 && val == 0);        // stub: reads 0
        val = 9;
        CHECK(be.ReadByte(1, 16, 0, &val) == 0 && val == 0);

        static const TPci1203IoRoute fake = { FrWriteBit, FrWriteByte, FrReadBit, FrReadByte };
        SetPci1203IoRoute(&fake);
        CHECK(Pci1203IoRoute() == &fake);

        FrReset(0, 0);
        CHECK(be.WriteBit(1, 176, 17, 1, 1) == 0);
        CHECK(g_fr.calls == 1 && g_fr.ring == 1 && g_fr.ip == 176 && g_fr.port == 17 && g_fr.bit == 1 && g_fr.value == 1);
        FrReset(0x7E000004, 0);
        CHECK(be.WriteBit(1, 176, 17, 1, 0) == 0x7E000004);           // the route's failure reaches TLaneIO unchanged
        FrReset(0, 1);
        val = 9;
        CHECK(be.ReadBit(1, 80, 9, 1, &val) == 0 && val == 1 && g_fr.ip == 80 && g_fr.port == 9);
        FrReset(0, 0xA5);
        CHECK(be.ReadByte(1, 80, 1, &val) == 0 && val == 0xA5 && g_fr.port == 1);
        FrReset(0, 0);
        CHECK(be.WriteByte(1, 176, 2, 0x3C) == 0 && g_fr.value == 0x3C && g_fr.port == 2);

#ifndef SOFT_SIMULTE
        // Through golden's own layer: MyLaneIO switches pIO1203 to TPci1203Backend
        // exactly as InitHontechHardware does (cinitial.cpp -> SelectVendorBackends).
        // Address chosen inside OutPortData[4][64][4] so this test does not itself
        // exercise golden's out-of-bounds cache index (Pci1203IoRoute.h).
        MyLaneIO.SelectVendorBackends();
        FrReset(0, 0);
        MyLaneIO.IOBitOn(1, 16, 3, 3, ePCI1203, "t_on");
        CHECK(g_fr.calls == 1 && g_fr.ring == 1 && g_fr.ip == 16 && g_fr.port == 3 && g_fr.value == 1);
        FrReset(0, 0);
        MyLaneIO.IOBitOff(1, 16, 3, 3, ePCI1203, "t_off");
        CHECK(g_fr.calls == 1 && g_fr.value == 0);
        FrReset(0, 1);
        CHECK(MyLaneIO.IOInputBit(1, 16, 3, 3, ePCI1203, "t_in") == true && g_fr.calls == 1);
        FrReset(0x7E000002, 1);                                       // no card -> golden's failure branch -> false
        CHECK(MyLaneIO.IOInputBit(1, 16, 3, 3, ePCI1203, "t_in") == false);
        FrReset(0, 0);
        MyLaneIO.IOBitOn(1, 16, 3, 3, 0, "t_mnet");                   // a MotionNet point never reaches the 1203 route
        CHECK(g_fr.calls == 0);
#else
        std::printf("  (SOFT_SIMULTE: MyLaneIO keeps the simulated backend, engine-path checks skipped)\n");
#endif

        SetPci1203IoRoute(0);
        FrReset(0, 0);
        CHECK(be.WriteBit(1, 176, 17, 1, 1) == 0 && g_fr.calls == 0); // uninstalled = the stub again
    }

    std::printf("test_io_points: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
