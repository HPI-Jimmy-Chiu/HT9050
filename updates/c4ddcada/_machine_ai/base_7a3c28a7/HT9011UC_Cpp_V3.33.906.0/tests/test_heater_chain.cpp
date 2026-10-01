// ===========================================================================
//  tests/test_heater_chain.cpp
//
//  AI(W906-I01) 20261001 (Ifor01): golden TfMain::Index16Heater / IndexHeatMode / HotplateHeatMode
//  （main.cpp:18613-20847）與 TfMain::SetTemp（:23890-23981）翻進 forms/fMain_Heater.cpp（ht9045_sm）。
//  虛擬的 TfMain::SetTemp 留在 forms/fMain.cpp:511（ht9045_forms）當樁，經 W906_SetTempHook 呼叫
//  W906_SetTempBody；I-01 第一階段沒有任何地方裝 hook（wb_serve 也沒有）。
//
//    [1] SetTemp 樁（沒裝 hook）：回 W906_SetTemp_Sim，預設 0；設 7 就回 7
//    [2] 裝 hook：SystemStart=true ⇒ golden :23896-23897 直接回 1，加熱狀態不動；W906_SetTemp_Sim 非 0 時仍回它
//    [3] HotplateHeatMode 一般機（Tri_Temp_Machine=0）：iPlateSelect 1／2／其他 ⇒ HotPlate1／2；
//        25 度控制（:20813-20826）；恆溫控制＋Shuttle 不加熱（:20828-20840）
//    [4] HotplateHeatMode 三溫機（Tri_Temp_Machine=1）：AmbientHot 清 HotPlate1～4 與 HasUse；Hot＋選 2 ⇒ 1～4 裝、只用 3／4
//    [5] Index16Heater 4 組加熱器（eht4Heater）：Head1～4 全開；單支加熱棒（:20230-20237）；
//        只用前臂（D30＋iShuttleMode=1＋iShuttle_Sel=0 ⇒ bMode[1]=false）；bHeaterMode=false；
//        整台 No Heater（:18640-18643）、SLK 不加熱（:18645-18648）都提早 return、通道全關
//    [6] Index16Heater 16 組（eht16Heater）單站：Aa1／Ba1／Aa2／Ba2；兩條線 kit（b2CableLayoutKit）只開 Aa1／Aa2
//    [7] IndexHeatMode：HeadOnly ⇒ bHeaterMode=true（:20332）⇒ Index16Heater 開 Head1～4；
//        ChamberOnly ⇒ bHeaterMode=false（:20361）、Chamber 開（:20363）、Head1～4 關
//    [8] 這幾支只動記憶體：D:\HT9045\system\Gerneral.ini 與 D:\HT9045\config\config.ini 前後逐位元組相同
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <string>
#include "forms/fMain.h"
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "cmydef.h"
#include "LastSet.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "bthermo.h"          // iThermoTask

extern int (*W906_SetTempHook)(TfMain*, bool, double, double);   // forms/fMain.cpp:511
void W906_InstallSetTemp();                                       // forms/fMain_Heater.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_heater_chain.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string Slurp(const char* p)
{
    FILE* f = std::fopen(p, "rb");
    if (!f) return std::string("<missing>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static void AllInstall(bool v) { for (int i = 0; i < tcTotalCount; ++i) { bUT150Install[i] = v; bUT150HasUse[i] = v; } }

static void Index16Baseline()
{
    TC401HeaterControl = 1;                          // KT4H (not NoHeater)
    Temperature.bSLKNoHeatUp = false;
    Temperature.bATCActiveCooling = false;
    Temperature.bATC70Active = false;
    Temperature.bMultiZoneEnable = false;
    USE_16_HEATER = eht4Heater;
    TestIF_File.iTestMode = SingleSite;
    TestIF_File.iShuttleMode = 0;
    TestIF_File.iShuttle_Sel = 0;
    TestIF_File.bSingleHeater = false;
    TestIF_File.b2CableLayoutKit = false;
    TestIF_File.bArm1PickPlaceArm2Test = false;
    IniConfig.bD30EnableSiteModeSelect = false;
    IniConfig.bL17HeadHeaterOnWhenCloseSite = true;
    IniConfig.bD58UseArm1PickPlaceArm2Test = false;
}

int main()
{
    static const char* const kReal[2] = { "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for (int i = 0; i < 2; ++i) before[i] = Slurp(kReal[i]);

    CHECK(fMain != 0);
    if (fMain == 0) { std::printf("FAIL: no fMain\n"); return 1; }

    std::printf("[1] SetTemp stub, no hook\n");
    CHECK(W906_SetTempHook == 0);                                     // 測試行程沒有裝
    fMain->W906_SetTemp_Sim = 0;
    CHECK(fMain->SetTemp(false, 100.0, 0.0) == 0);
    fMain->W906_SetTemp_Sim = 7;
    CHECK(fMain->SetTemp(false, 100.0, 0.0) == 7);
    fMain->W906_SetTemp_Sim = 0;

    std::printf("[2] SetTemp body through the hook: SystemStart returns 1, nothing touched\n");
    W906_InstallSetTemp();
    CHECK(W906_SetTempHook != 0);
    SystemStart = true;
    fHeaterOK = true;
    iThermoTask = 42;
    CHECK(fMain->SetTemp(true, 100.0, 0.0) == 1);                    // golden :23896-23897
    CHECK(fHeaterOK == true);
    CHECK(iThermoTask == 42);
    fMain->W906_SetTemp_Sim = 5;
    CHECK(fMain->SetTemp(true, 100.0, 0.0) == 5);                    // 測試縫優先
    fMain->W906_SetTemp_Sim = 0;
    SystemStart = false;
    W906_SetTempHook = 0;                                             // 還原：之後的情境不經 SetTemp

    std::printf("[3] HotplateHeatMode, normal machine\n");
    Tri_Temp_Machine = 0;
    IniConfig.bTemp25degControl = false;
    IniConfig.bI03AmbientTempControl = false;
    LastSet.iTemperature = Tempture_Hot;
    AllInstall(false);
    HotPlateForm.iPlateSelect = 1;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == true);
    CHECK(bUT150Install[tcHotPlate2] == false);
    HotPlateForm.iPlateSelect = 2;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == false);
    CHECK(bUT150Install[tcHotPlate2] == true);
    HotPlateForm.iPlateSelect = 0;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == true);
    CHECK(bUT150Install[tcHotPlate2] == true);
    // 25 度控制：8 站 2x4、Hot、HeadOnly、soak 0、溫度 <= 25 ⇒ 兩片都關
    IniConfig.bTemp25degControl = true;
    TestIF_File.iTestMode = _8Site2X4;
    Temperature.iIndexHeatMode = HeadOnly;
    Temperature.fSoakTime = 0;
    Temperature.fWorkTemperBase = 25.0;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == false);
    CHECK(bUT150Install[tcHotPlate2] == false);
    Temperature.fWorkTemperBase = 25.5;                                // 高於 25 就不關
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == true);
    IniConfig.bTemp25degControl = false;
    // 恆溫控制（AmbientHot）＋Shuttle 不加熱
    IniConfig.bI03AmbientTempControl = true;
    LastSet.iTemperature = Tempture_AmbientHot;
    Temperature.bShuttleNoHeatUp = true;
    bUT150Install[tcShuttle1] = bUT150Install[tcShuttle2] = true;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] == false);
    CHECK(bUT150Install[tcHotPlate2] == false);
    CHECK(bUT150Install[tcShuttle1] == false);
    CHECK(bUT150Install[tcShuttle2] == false);
    IniConfig.bI03AmbientTempControl = false;
    Temperature.bShuttleNoHeatUp = false;

    std::printf("[4] HotplateHeatMode, tri-temp machine\n");
    Tri_Temp_Machine = 1;
    AllInstall(true);
    LastSet.iTemperature = Tempture_AmbientHot;
    fMain->HotplateHeatMode();
    CHECK(!bUT150Install[tcHotPlate1] && !bUT150Install[tcHotPlate2] && !bUT150Install[tcHotPlate3] && !bUT150Install[tcHotPlate4]);
    CHECK(!bUT150HasUse[tcHotPlate1] && !bUT150HasUse[tcHotPlate2] && !bUT150HasUse[tcHotPlate3] && !bUT150HasUse[tcHotPlate4]);
    LastSet.iTemperature = Tempture_Hot;
    HotPlateForm.iPlateSelect = 2;
    fMain->HotplateHeatMode();
    CHECK(bUT150Install[tcHotPlate1] && bUT150Install[tcHotPlate2] && bUT150Install[tcHotPlate3] && bUT150Install[tcHotPlate4]);
    CHECK(!bUT150HasUse[tcHotPlate1] && !bUT150HasUse[tcHotPlate2] && bUT150HasUse[tcHotPlate3] && bUT150HasUse[tcHotPlate4]);
    Tri_Temp_Machine = 0;

    std::printf("[5] Index16Heater, 4 heaters\n");
    Index16Baseline();
    AllInstall(false);
    fMain->Index16Heater(true);
    CHECK(bUT150Install[tcHead1] && bUT150Install[tcHead2] && bUT150Install[tcHead3] && bUT150Install[tcHead4]);
    TestIF_File.bSingleHeater = true;                                  // golden :20230-20237
    fMain->Index16Heater(true);
    CHECK(bUT150Install[tcHead1] && !bUT150Install[tcHead2] && bUT150Install[tcHead3] && !bUT150Install[tcHead4]);
    TestIF_File.bSingleHeater = false;
    IniConfig.bD30EnableSiteModeSelect = true;                         // 只用前臂 ⇒ bMode[1]=false
    TestIF_File.iShuttleMode = 1;
    TestIF_File.iShuttle_Sel = 0;
    fMain->Index16Heater(true);
    CHECK(bUT150Install[tcHead1] && bUT150Install[tcHead2] && !bUT150Install[tcHead3] && !bUT150Install[tcHead4]);
    Index16Baseline();
    fMain->Index16Heater(false);                                       // bHeaterMode=false：開頭清掉後什麼都不設
    CHECK(!bUT150Install[tcHead1] && !bUT150Install[tcHead2] && !bUT150Install[tcHead3] && !bUT150Install[tcHead4]);
    AllInstall(true);
    TC401HeaterControl = NoHeater;                                     // 整台 No Heater：清掉 Head／Index 區後 return
    fMain->Index16Heater(true);
    CHECK(!bUT150Install[tcHead1] && !bUT150Install[tcHead4] && !bUT150Install[tcAa1] && !bUT150Install[tcBh2]);
    CHECK(bUT150Install[tcHotPlate1] == true);                         // Head／Index 以外的通道不歸它管
    Index16Baseline();
    AllInstall(true);
    Temperature.bSLKNoHeatUp = true;                                   // SLK 不加熱：同樣清掉後 return
    fMain->Index16Heater(true);
    CHECK(!bUT150Install[tcHead1] && !bUT150Install[tcHead3]);
    Temperature.bSLKNoHeatUp = false;

    std::printf("[6] Index16Heater, 16 heaters, single site\n");
    Index16Baseline();
    USE_16_HEATER = eht16Heater;
    AllInstall(false);
    fMain->Index16Heater(true);
    CHECK(bUT150Install[tcAa1] && bUT150Install[tcBa1] && bUT150Install[tcAa2] && bUT150Install[tcBa2]);
    TestIF_File.b2CableLayoutKit = true;                               // 兩條線版本只用一支加熱棒
    AllInstall(false);
    fMain->Index16Heater(true);
    CHECK(bUT150Install[tcAa1] && !bUT150Install[tcBa1] && bUT150Install[tcAa2] && !bUT150Install[tcBa2]);
    Index16Baseline();

    std::printf("[7] IndexHeatMode\n");
    Index16Baseline();
    LastSet.iTemperature = Tempture_Hot;
    Temperature.iIndexHeatMode = HeadOnly;
    AllInstall(false);
    fMain->IndexHeatMode();
    CHECK(bUT150Install[tcHead1] && bUT150Install[tcHead2] && bUT150Install[tcHead3] && bUT150Install[tcHead4]);
    Temperature.iIndexHeatMode = ChamberOnly;
    AllInstall(true);
    fMain->IndexHeatMode();
    CHECK(bUT150Install[tcChamber] == true);                           // golden :20363
    CHECK(!bUT150Install[tcHead1] && !bUT150Install[tcHead2] && !bUT150Install[tcHead3] && !bUT150Install[tcHead4]);

    std::printf("[8] memory only: real files unchanged\n");
    for (int i = 0; i < 2; ++i) {
        std::string after = Slurp(kReal[i]);
        CHECK(after == before[i]);
        if (after != before[i]) std::printf("     changed: %s\n", kReal[i]);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
