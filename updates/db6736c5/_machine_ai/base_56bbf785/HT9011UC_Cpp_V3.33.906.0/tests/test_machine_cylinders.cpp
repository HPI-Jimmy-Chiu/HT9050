// ===========================================================================
//  tests/test_machine_cylinders.cpp
//
//  AI(W906-W3-INITCYL) 20260924: 週末計畫 W3 第 7 項（InitialCylinderName／InitCylinder）的
//  「第三級：值有被維護」測試。
//
//  golden InitCylinder 的 if（IO_CARD_TYPE==NewIO_MN200 || PCI_P64C64）那半（移植 cinitial.cpp:4859 起），
//  對每個有名稱的氣缸查 IO 表三次：
//      <名稱>        -> Out*    （OutRingUse／OutRing／OutIP／OutPort／OutBit／OutType／OutISABase）
//      <名稱>_On     -> OnSen*  （同上 7 欄）
//      <名稱>_Off    -> OffSen* （同上 7 欄）
//  這支測試拿版控的 machines/HT9050/IO_Table.csv（argv[1]）餵進去，逐列逐欄比對，要求零不符。
//  Enable 系列另有 golden 的額外規則（位址全 0、Port／Bit 為 0 就關；尾段對 Empty／Color／Auto 強制開），
//  那不是「位址有沒有寫進去」的問題，這裡不比。
//
//  用法：test_machine_cylinders <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"
#include "cinitial.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "mycylin.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_machine_cylinders.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static TIODATA* Row(const AnsiString& alias)
{
    HSys.mapIOTableIter = HSys.mapIOTable.find(alias);
    if (HSys.mapIOTableIter == HSys.mapIOTable.end()) return 0;
    return HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
}
static AnsiString UseOf(const TIODATA* r) { return (r->iPort == -1) ? AnsiString("") : AnsiString(r->iPort); }

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_machine_cylinders <IO_Table.csv>\n"); return 2; }
    const std::string env = std::string("W906_IOTABLE_PATH=") + argv[1];
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
    HSys.LoadIoData();
    std::printf("IO_Table rows: %d\n", (int)HSys.IOTable.size());
    CHECK(HSys.IOTable.size() > 0);

    IO_CARD_TYPE = NewIO_MN200;          // 走 golden 的 if 那半（筆電 Gerneral.ini 的實際值）
    iControlPanelMode = 0;
    Enable_PLCSafety_IO = false;
    InitCylinder();                      // 內部先呼叫 InitialCylinderName()（golden :4103）

    int named = 0, outRows = 0, outBad = 0, onRows = 0, onBad = 0, offRows = 0, offBad = 0;
    for (int i = 0; i < MaxCylinderItem; ++i) {
        TMyCylinder& C = Cylinder[i];
        if (C.CylinderName == AnsiString("")) continue;
        ++named;
        CHECK(C.OnSensorName  == C.CylinderName + "_On");
        CHECK(C.OffSensorName == C.CylinderName + "_Off");

        if (const TIODATA* r = Row(C.CylinderName)) {
            ++outRows;
            const bool ok = C.OutRingUse == UseOf(r) && C.OutRing == r->iLane && C.OutIP == r->iIP &&
                            C.OutPort == r->iPort && C.OutBit == r->iBit && C.OutType == r->iInType &&
                            C.OutISABase == r->iISABase;
            if (!ok) { if (outBad < 5) std::printf("  Out mismatch: %s\n", C.CylinderName.c_str()); ++outBad; }
        }
        if (const TIODATA* r = Row(C.OnSensorName)) {
            ++onRows;
            const bool ok = C.OnSenRingUse == UseOf(r) && C.OnSenRing == r->iLane && C.OnSenIP == r->iIP &&
                            C.OnSenPort == r->iPort && C.OnSenBit == r->iBit && C.OnSenType == r->iInType &&
                            C.OnSenISABase == r->iISABase;
            if (!ok) { if (onBad < 5) std::printf("  OnSen mismatch: %s\n", C.OnSensorName.c_str()); ++onBad; }
        }
        if (const TIODATA* r = Row(C.OffSensorName)) {
            ++offRows;
            const bool ok = C.OffSenRingUse == UseOf(r) && C.OffSenRing == r->iLane && C.OffSenIP == r->iIP &&
                            C.OffSenPort == r->iPort && C.OffSenBit == r->iBit && C.OffSenType == r->iInType &&
                            C.OffSenISABase == r->iISABase;
            if (!ok) { if (offBad < 5) std::printf("  OffSen mismatch: %s\n", C.OffSensorName.c_str()); ++offBad; }
        }
    }
    std::printf("cylinders named %d | Out %d rows %d bad | OnSen %d rows %d bad | OffSen %d rows %d bad\n",
                named, outRows, outBad, onRows, onBad, offRows, offBad);
    CHECK(named > 0);
    CHECK(outRows > 0 && outBad == 0);
    CHECK(onRows > 0 && onBad == 0);
    CHECK(offRows > 0 && offBad == 0);
    // AI(W906-W3-AUDIT) 20260925: 把數量釘住（W3 稽核：原本只要求 >0，宣稱的 72／54／54 沒被驗）。
    //   期望值由 Python 獨立算：InitialCylinderName（cinitial.cpp:7694 起）的名稱字面值 ∩ machines/HT9050/IO_Table.csv
    //   的 Alias，逐索引計；不是拿這支測試的輸出回填。
    //   named＝258 而不是 259 個名稱：golden cmydef.cpp:442/:444 的 C_StackedTrayLockOff 與 C_LoadRobotX 都是 75
    //   （906 與 V912 相同，golden 缺陷，照翻），後指派的 C_LoadRobotX 蓋掉前者。若有人修了這個撞號，這裡會變 259。
    if (std::string(argv[1]).find("HT9050") != std::string::npos) {
        CHECK(outRows == 72);
        CHECK(onRows  == 54);
        CHECK(offRows == 54);
        CHECK(named   == 258);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
