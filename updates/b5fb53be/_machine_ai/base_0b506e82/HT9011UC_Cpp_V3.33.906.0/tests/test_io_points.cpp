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
//
//  用法：test_io_points <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
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
    CHECK(HSys.IOTable.size() == 1062);

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
        CHECK(nPts == 905);                    // 非空 Alias 的列（Python 另算：905）
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
        IO_CARD_TYPE = NewIO_MN200;
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
        CHECK(swChecked == 132 && snChecked == 289);   // 與 MachineIoTable_HT9050 同一組獨立算出的數字
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
                }
                cJSON* why = cJSON_GetObjectItem(runtime, "why");
                CHECK(why != 0 && why->valuestring != 0 && std::string(why->valuestring).find("停用") != std::string::npos);
                cJSON_Delete(r3);
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
            di.push_back(S(0, it->first, it->second, 0xA5));
        for (std::set<std::pair<int,int> >::const_iterator it = outKeys.begin(); it != outKeys.end(); ++it)
            dout.push_back(S(0, it->first, it->second, 0x3C));

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
            CHECK(good == 236);                                  // HT9050 表 Enable=1 的列（Python 另算：236，全是 1203）

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
            CHECK(verified == 236);
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

    std::printf("test_io_points: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
