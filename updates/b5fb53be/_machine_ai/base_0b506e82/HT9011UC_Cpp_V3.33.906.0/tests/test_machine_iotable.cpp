// ===========================================================================
//  tests/test_machine_iotable.cpp
//
//  AI(W906-IOTABLE-NOTE) 20260924: 版控裡每一台機台的 IO_Table.csv
//  （machines/<機台>/IO_Table.csv）都必須能被 golden 的 LoadIoData 讀進來。
//
//  為什麼要這支：20260923 放進來的 machines/HT9050/IO_Table.csv 表頭只有
//  14 欄（少了最後的 Note）。golden 的 TIOTABLENO::SetIOTableNo
//  （database.cpp:1925-1935）規定表頭要剛好 eioTotal(15) 欄，不是就回 0，
//  LoadIoData 印「File ... data is mistake! (0)」之後整張 IO 表是空的 ——
//  所有 Sensor／Switch／Cylinder／Sucker 都對不到點，機台底層起不來。
//  wb_serve 照樣開得起來、網頁照樣連得上，只是 IO 全部沒有，看起來像「沒資料」。
//  BCB6 量產版讀同一張檔一樣會失敗（golden 原文相同），所以修的是資料不是程式。
//
//  這支測試讀的是**版控的檔**（不是機台現場 D:\HT9045\system 的那一份），
//  所以不會變成第六個「讀活機台資料」的常駐失敗；壞掉就是資料真的壞了。
//
//  用法：test_machine_iotable <IO_Table.csv 路徑> <預期資料列數> [<SW 綁定數> <Sen 綁定數>]
//  路徑經由 database.cpp:1698 的唯讀接縫 W906_IOTABLE_PATH 餵給 LoadIoData。
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"     // extern AnsiString IoTablePath (common.cpp:232)
#include "cinitial.h"   // InitialSwitch / InitialSensor（golden cinitial.cpp:1377 / :2471）
#include "cmydef.h"     // IO_CARD_TYPE / NewIO_MN200 / iControlPanelMode / Enable_PLCSafety_IO
#include "CosFunction.h"
#include "myswitch.h"   // SW[MAX_SWITCH_ITEM]
#include "mysensor.h"   // Sen[MAX_SENSOR_ITEM]

#include <cstdio>
#include <cstdlib>
#include <string>

// 嚴格模式（CXX_EXTENSIONS OFF）下 MinGW.org 6.3.0 兩個名字都不宣告；
// 寫法照 tests/test_agv_e84.cpp:143-163 的說明（WinLibs 無條件宣告會出警告）。
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_fail = 0;
static int g_total = 0;
static void check(bool cond, const char* expr, const char* file, int line) {
    ++g_total;
    if (!cond) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::printf("usage: test_machine_iotable <IO_Table.csv> <expected data rows>\n");
        return 2;
    }
    const std::string path = argv[1];
    const int expectRows = std::atoi(argv[2]);

    if (!FileExists(AnsiString(path.c_str()))) {
        // 版控的檔不見了就是錯，不是 SKIP。
        std::printf("FAIL: %s not found\n", path.c_str());
        return 1;
    }

    const std::string env = "W906_IOTABLE_PATH=" + path;
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));

    HSys.LoadIoData();

    CHECK(IoTablePath == AnsiString(path.c_str()));   // 接縫真的有生效

    const int nIo = (int)HSys.IOTable.size();
    std::printf("%s: LoadIoData parsed %d rows (expect %d), %d aliases mapped\n",
                path.c_str(), nIo, expectRows, (int)HSys.mapIOTable.size());
    CHECK(nIo == expectRows);          // 表頭被拒絕時這裡是 0
    CHECK(!HSys.mapIOTable.empty());

    // 表頭欄名解析的結果（golden 的欄序；Note 是最後一欄）。
    CHECK(HSys.IoNo.eioType    == 0);
    CHECK(HSys.IoNo.eioAlias   == 1);
    CHECK(HSys.IoNo.eioISABase == 8);
    CHECK(HSys.IoNo.eioEnable  == 9);
    CHECK(HSys.IoNo.eioNote    == 14);

    // 抽查一列：SnMotorPower（馬達電源 sensor，1203 卡）
    //   Sensor,SnMotorPower,0,1,2,30,6,1,3,1,,,,
    HSys.mapIOTableIter = HSys.mapIOTable.find(AnsiString("SnMotorPower"));
    const bool found = HSys.mapIOTableIter != HSys.mapIOTable.end();
    std::printf("SnMotorPower mapped: %s\n", found ? "yes" : "no");
    if (path.find("HT9050") != std::string::npos) {
        CHECK(found);
        if (found) {
            const int idx = HSys.mapIOTableIter->second.ToIntDef(-1);
            CHECK(idx >= 0 && idx < nIo);
            if (idx >= 0 && idx < nIo) {
                TIODATA* d = HSys.IOTable[idx];
                std::printf("SnMotorPower -> Type=%s Lane=%d Module=%d IP=%d Port=%d "
                            "Bit=%d InType=%d ISABase=%d Enable=%d\n",
                            d->Type.c_str(), d->iLane, d->iModuleType, d->iIP,
                            d->iPort, d->iBit, d->iInType, d->iISABase, d->iEnable);
                CHECK(d->Type      == AnsiString("Sensor"));
                CHECK(d->iLane     == 0);
                CHECK(d->iIP       == 2);
                CHECK(d->iPort     == 30);
                CHECK(d->iBit      == 6);
                CHECK(d->iISABase  == 3);      // ePCI1203
                CHECK(d->iEnable   == 1);
            }
        }
    }

    // ---------------------------------------------------------------------
    //  第三級證據：表讀得進來還不夠，要看 IO 物件真的被綁上。
    //  IO_CARD_TYPE==NewIO_MN200(2) 時 InitialSwitch／InitialSensor 用 Alias
    //  查 mapIOTable；查不到只做一個沒人看的 str.sprintf("Can not find ...")
    //  然後 Enable=false（golden cinitial.cpp:1394-1395 / :1430）—— 表是空的時候
    //  每一個 Switch／Sensor 都這樣安靜地關掉。
    //  「綁上」= 該列的 ISABase／Port／Bit 真的被複製進物件（不是只查 map）。
    //  預期數字由 Python 從 cinitial.cpp 的 Name 字面值 ∩ CSV Alias 獨立算出
    //  （argv[3] / argv[4]；沒給就只印不驗）。
    // ---------------------------------------------------------------------
    // AI(W906-IOWEB-P4) 20260925: argv[5] 可指定 IO_CARD_TYPE（沒給＝NewIO_MN200，原本的行為）。
    //   4＝PCI1203_IO：HT9050 機台 Gerneral.ini 20260925 的實際值，必須綁上跟 2 一樣的列數。
    //   0 是對照組：golden 讀 IO_Table 的 gate 都不成立 ⇒ 一個都綁不上（機台端實測 0 時 259 個啟用列綁 0 列）。
    //   沒有這個對照組，「4 綁得上」可能只是 gate 根本沒在擋。
    const int cardType = (argc >= 6) ? std::atoi(argv[5]) : NewIO_MN200;
    const bool gateOpen = (cardType == NewIO_MN200 || cardType == PCI_P64C64 || cardType == PCI1203_IO);
    std::printf("IO_CARD_TYPE=%d (gate %s)\n", cardType, gateOpen ? "open" : "closed");
    IO_CARD_TYPE = cardType;             // 預設是這台 Gerneral.ini 的實際值（database.cpp:1205 讀進來）
    iControlPanelMode = 0;
    Enable_PLCSafety_IO = false;
    CosFunction.bTTLCanUse8Site = false;
    InitialSwitch();
    InitialSensor();

    int swBound = 0, swEnabled = 0, snBound = 0, snEnabled = 0;
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i) {
        if (SW[i].Name == AnsiString("")) continue;
        HSys.mapIOTableIter = HSys.mapIOTable.find(SW[i].Name);
        if (HSys.mapIOTableIter == HSys.mapIOTable.end()) continue;
        TIODATA* r = HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
        if (SW[i].ISABase == r->iISABase && SW[i].Port == r->iPort && SW[i].Bit == r->iBit) {
            ++swBound;
            if (SW[i].Enable) ++swEnabled;
        }
    }
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) {
        if (Sen[i].Name == AnsiString("")) continue;
        HSys.mapIOTableIter = HSys.mapIOTable.find(Sen[i].Name);
        if (HSys.mapIOTableIter == HSys.mapIOTable.end()) continue;
        TIODATA* r = HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
        if (Sen[i].ISABase == r->iISABase && Sen[i].Port == r->iPort && Sen[i].Bit == r->iBit) {
            ++snBound;
            if (Sen[i].Enable) ++snEnabled;
        }
    }
    std::printf("bound: SW %d (Enable %d)   Sen %d (Enable %d)\n",
                swBound, swEnabled, snBound, snEnabled);

    // AI(W906-W3-INITIO) 20260924: 週末計畫 W3 第 5 項「第三級：值有被維護」——
    //   上面只比 ISABase／Port／Bit。golden if（NewIO_MN200）那半對每個查得到的點指派 7 個欄位
    //   （cinitial.cpp InitialSwitch :1541-1548、InitialSensor :2665-2685）：逐列逐欄比對，要求零不符。
    //   Sensor 的 ePLCbase 特例：Ring／IP 固定 0（KenHsieh 20260421，:2670-2676）。
    int swRows = 0, swBad = 0, snRows = 0, snBad = 0;
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i) {
        if (SW[i].Name == AnsiString("")) continue;
        HSys.mapIOTableIter = HSys.mapIOTable.find(SW[i].Name);
        if (HSys.mapIOTableIter == HSys.mapIOTable.end()) continue;
        TIODATA* r = HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
        ++swRows;
        const AnsiString using_ = (r->iPort == -1) ? AnsiString("") : AnsiString(r->iPort);
        const bool ok = SW[i].ISABase == r->iISABase && SW[i].Ring == r->iLane && SW[i].IP == r->iIP &&
                        SW[i].Port == r->iPort && SW[i].Bit == r->iBit && SW[i].Type == r->iInType &&
                        SW[i].Using == using_;
        if (!ok) { if (swBad < 5) std::printf("  SW[%d] %s field mismatch\n", i, SW[i].Name.c_str()); ++swBad; }
    }
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) {
        if (Sen[i].Name == AnsiString("")) continue;
        HSys.mapIOTableIter = HSys.mapIOTable.find(Sen[i].Name);
        if (HSys.mapIOTableIter == HSys.mapIOTable.end()) continue;
        TIODATA* r = HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
        if (Enable_PLCSafety_IO && r->iISABase == ePLCbase) continue;   // 這支測試把 PLC 關掉，保險起見跳過
        ++snRows;
        const bool plc = (r->iISABase == ePLCbase);
        const AnsiString using_ = (r->iPort == -1) ? AnsiString("") : AnsiString(r->iPort);
        const bool ok = Sen[i].ISABase == r->iISABase &&
                        Sen[i].Ring == (plc ? 0 : r->iLane) && Sen[i].IP == (plc ? 0 : r->iIP) &&
                        Sen[i].Port == r->iPort && Sen[i].Bit == r->iBit && Sen[i].Type == r->iInType &&
                        Sen[i].Using == using_;
        if (!ok) { if (snBad < 5) std::printf("  Sen[%d] %s field mismatch\n", i, Sen[i].Name.c_str()); ++snBad; }
    }
    std::printf("field-level: SW %d rows, %d mismatch   Sen %d rows, %d mismatch\n", swRows, swBad, snRows, snBad);
    if (!gateOpen) {
        // 對照組：gate 關著 ⇒ 表讀進來了（上面 nIo 已驗），但沒有任何物件被綁上。
        CHECK(swBound == 0);
        CHECK(snBound == 0);
        std::printf("test_machine_iotable: %d/%d checks passed\n", g_total - g_fail, g_total);
        if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
        std::printf("PASS\n");
        return 0;
    }
    CHECK(swRows > 0 && swBad == 0);
    CHECK(snRows > 0 && snBad == 0);
    if (argc >= 5) {
        CHECK(swBound == std::atoi(argv[3]));
        CHECK(snBound == std::atoi(argv[4]));
    }
    CHECK(swBound > 0);
    CHECK(snBound > 0);
#ifdef SOFT_SIMULTE
    // 模擬建置：golden 綁完之後一律把 IO 物件關掉（Switch cinitial.cpp:1432-1433、
    // Sensor :2531、吸嘴 :497／:724）。所以這裡 Enable 全 0 是**對的**；
    // 真機要看得到 IO，必須用 -DW906_NO_SOFT_SIMULTE=ON 的建置。
    CHECK(swEnabled == 0);
    CHECK(snEnabled == 0);
#endif
    if (path.find("HT9050") != std::string::npos) {
#ifdef SOFT_SIMULTE
        CHECK(Sen[SnMotorPower].Enable  == false);
#else
        CHECK(Sen[SnMotorPower].Enable  == true);
#endif
        CHECK(Sen[SnMotorPower].ISABase == 3);   // ePCI1203
        CHECK(Sen[SnMotorPower].IP      == 2);
        CHECK(Sen[SnMotorPower].Port    == 30);
        CHECK(Sen[SnMotorPower].Bit     == 6);
    }

    std::printf("test_machine_iotable: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
