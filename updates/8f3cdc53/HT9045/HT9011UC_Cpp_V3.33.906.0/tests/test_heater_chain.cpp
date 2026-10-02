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
//    第二階段（一）：
//    [9] Timer2 加熱段 TfMain::W906_Timer2HeaterSegment（golden :21528-21643）：QA 那一臂 20261002 開閘（AI(W906-R126)，
//        NB2 R126 M2）；ctest 沒有 FileRW 的分頁代理（_fallback.cpp 回 -1）⇒ 頁面表說 Configuration 開著也走 else；
//        讀原始碼確認條件是 H1-08 的寫法、閘已拿掉。SCC 常溫關 HotPlate、熱模式走 HotplateHeatMode；KYEC 25.0 度熱配方當常溫
//        （:21620-21626，照 golden）、80 度走 HotplateHeatMode；三溫機常溫不關（關的規則只對 Tri_Temp_Machine==0）
//    [10] ht9045::W906_Timer2HeaterTick：InitialOK==false 不跑（:20863，AI(W906-R126) NB2 R126 M3）、SystemStart 時不跑
//        （:21382）、1000 ms 限速（容許半個 PumpTick：800 ms 後的那一拍算到期）
//    [11] 接線：WebBridgeTags.cpp 的 PumpTick 在 g_pumpActive 守衛之後呼叫它一次（argv[1]＝原始碼根目錄）
//    [12] 開機那一次：tools/wb_serve.cpp 在 PumpInit 之後、SetTechDataToProd 前面同一行呼叫 fMain->IndexHeatMode() 一次
//        （golden FormShow :10722；AI(W906-R126) 20261002）
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
#include "csystem.h"          // W906_FormFShowHook (the web page table)
#include <cstring>

extern int (*W906_SetTempHook)(TfMain*, bool, double, double);   // forms/fMain.cpp:511
void W906_InstallSetTemp();                                       // forms/fMain_Heater.cpp
namespace ht9045 { void W906_Timer2HeaterTick(); }                // forms/fMain_Heater.cpp (phase 2)

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

static const char* g_openPage = 0;   // the fake web page table (as tests/test_coolfan_rules.cpp): this golden form is "open"
static bool FakePageTable(const char* goldenForm) { return g_openPage != 0 && std::strcmp(goldenForm, g_openPage) == 0; }

// The text of the function `sig` ... its closing "\n}" (empty when missing).
static std::string FunctionText(const std::string& src, const char* sig)
{
    const size_t a = src.find(sig);
    if (a == std::string::npos) return std::string();
    const size_t b = src.find("\n}", a);
    return b == std::string::npos ? std::string() : src.substr(a, b - a);
}

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

int main(int argc, char** argv)
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

    std::printf("[9] Timer2 heater segment (golden :21528-21643), QA arm open (no page proxy in a ctest)\n");
    const int savedCustomer = CUSTOMER_CODE;
    Index16Baseline();
    Tri_Temp_Machine = 0;
    IniConfig.bTemp25degControl = false;
    IniConfig.bI03AmbientTempControl = false;
    Temperature.iIndexHeatMode = HeadOnly;
    Temperature.bShuttleNoHeatUp = false;
    Temperature.bATCActiveCooling = false;
    HotPlateForm.iPlateSelect = 1;
    // CC_HONPREC_QC (0), Configuration not open: the else body runs -- HotplateHeatMode, then IndexHeatMode
    CUSTOMER_CODE = CC_HONPREC_QC;
    LastSet.iTemperature = Tempture_Hot;
    AllInstall(false);
    fMain->W906_Timer2HeaterSegment();
    CHECK(bUT150Install[tcHotPlate1] == true && bUT150Install[tcHotPlate2] == false);   // HotplateHeatMode, iPlateSelect=1 (the
                                                                                   // QA arm would have set HotPlate1..Shuttle2 all true)
    CHECK(bUT150Install[tcHead1] && bUT150Install[tcHead2] && bUT150Install[tcHead3] && bUT150Install[tcHead4]);   // IndexHeatMode, HeadOnly
    // Configuration open in the page table, but a ctest has no FileRW page proxy (FileRW/_fallback.cpp: -1, not page 1) => else
    W906_FormFShowHook = &FakePageTable;
    g_openPage = "fConfiguration";
    AllInstall(false);
    fMain->W906_Timer2HeaterSegment();
    CHECK(bUT150Install[tcHotPlate1] == true && bUT150Install[tcHotPlate2] == false);
    g_openPage = 0;
    W906_FormFShowHook = 0;
    // the QA arm is code again, with H1-08's substitution (csystem.cpp:24854); its #if 0 gate is gone
    if (argc > 1) {
        const std::string hs = Slurp((std::string(argv[1]) + "/forms/fMain_Heater.cpp").c_str());
        const std::string seg = FunctionText(hs, "void TfMain::W906_Timer2HeaterSegment()");
        CHECK(!seg.empty());
        CHECK(seg.find("#if 0") == std::string::npos);
        CHECK(seg.find("#endif") == std::string::npos);
        const size_t qa = seg.find("if(CUSTOMER_CODE==CC_HONPREC_QC &&");
        const size_t fs = seg.find("W906_FormShowing(\"fConfiguration\", false)==true", qa == std::string::npos ? 0 : qa);
        const size_t pg = seg.find("FileRW_ProxyPageIndex(\"TfConfiguration\", \"PageControl1\")==1)", fs == std::string::npos ? 0 : fs);
        const size_t el = seg.find("\n    else\n", pg == std::string::npos ? 0 : pg);
        const size_t el2 = seg.find("\n    else\r\n", pg == std::string::npos ? 0 : pg);
        CHECK(qa != std::string::npos && fs != std::string::npos && pg != std::string::npos);
        CHECK(qa < fs && fs < pg);
        CHECK(el != std::string::npos || el2 != std::string::npos);                // golden :21582 `else` before the hot-plate arm
    }
    // SCC: ambient => both hot plates off, HotplateHeatMode not called (golden :21588-21592)
    CUSTOMER_CODE = CC_SCC;
    LastSet.iTemperature = Tempture_Ambient;
    AllInstall(true);
    fMain->W906_Timer2HeaterSegment();
    CHECK(!bUT150Install[tcHotPlate1] && !bUT150Install[tcHotPlate2]);
    LastSet.iTemperature = Tempture_Hot;                                           // SCC hot => HotplateHeatMode (:21601)
    AllInstall(false);
    fMain->W906_Timer2HeaterSegment();
    CHECK(bUT150Install[tcHotPlate1] == true && bUT150Install[tcHotPlate2] == false);
    // KYEC_LEE: a hot recipe at 25.0 C counts as ambient (dbSetTemp<=25.0, :21620-21626) -- golden, kept
    CUSTOMER_CODE = CC_KYEC_LEE;
    LastSet.iTemperature = Tempture_Hot;
    Temperature.fWorkTemperBase = 25.0;
    AllInstall(true);
    fMain->W906_Timer2HeaterSegment();
    CHECK(!bUT150Install[tcHotPlate1] && !bUT150Install[tcHotPlate2]);
    Temperature.fWorkTemperBase = 80.0;                                            // 80 C => HotplateHeatMode (:21635)
    AllInstall(false);
    fMain->W906_Timer2HeaterSegment();
    CHECK(bUT150Install[tcHotPlate1] == true);
    LastSet.iTemperature = Tempture_Ambient;                                       // ambient on a tri-temp machine: the off rule needs
    Tri_Temp_Machine = 1;                                                          // Tri_Temp_Machine==0 (:21623), so HotplateHeatMode runs,
    AllInstall(true);                                                              // and its tri-temp arm has no Ambient case (golden :20745-20798)
    fMain->W906_Timer2HeaterSegment();
    CHECK(bUT150Install[tcHotPlate1] && bUT150Install[tcHotPlate2] && bUT150Install[tcHotPlate3] && bUT150Install[tcHotPlate4]);
    Tri_Temp_Machine = 0;

    std::printf("[10] W906_Timer2HeaterTick: InitialOK guard (:20863), SystemStart guard (:21382), 1000 ms throttle\n");
    CUSTOMER_CODE = CC_SCC;
    LastSet.iTemperature = Tempture_Ambient;
    const bool savedInitialOK = InitialOK;
    InitialOK = false;                                                             // Exit (MainClose.cpp:2168) / before PumpInit
    SystemStart = false;
    AllInstall(true);
    ht9045::W906_Timer2HeaterTick();                                               // first beat: due and idle, but not initialised
    CHECK(bUT150Install[tcHotPlate1] == true);
    InitialOK = true;
    SystemStart = true;
    Sleep(1100);
    ht9045::W906_Timer2HeaterTick();                                               // due, initialised, but the machine runs
    CHECK(bUT150Install[tcHotPlate1] == true);
    SystemStart = false;
    ht9045::W906_Timer2HeaterTick();                                               // < 1000 ms later: not due
    CHECK(bUT150Install[tcHotPlate1] == true);
    Sleep(1100);
    ht9045::W906_Timer2HeaterTick();                                               // due and idle: SCC ambient => hot plates off
    CHECK(bUT150Install[tcHotPlate1] == false && bUT150Install[tcHotPlate2] == false);
    AllInstall(true);
    Sleep(200);
    ht9045::W906_Timer2HeaterTick();                                               // ~200 ms: well inside the 1000 ms -- not due
    CHECK(bUT150Install[tcHotPlate1] == true);
    Sleep(700);
    ht9045::W906_Timer2HeaterTick();                                               // ~900 ms: a PumpTick landing early -- due (750 ms
    CHECK(bUT150Install[tcHotPlate1] == false);                                    // = 1000 - half a PumpTick; the old 1000 ms test
                                                                                   // made this a 1.5 s beat)
    InitialOK = savedInitialOK;
    CUSTOMER_CODE = savedCustomer;

    std::printf("[11] wiring: PumpTick calls the tick once, after the g_pumpActive guard\n");
    if (argc > 1) {
        const std::string src = Slurp((std::string(argv[1]) + "/WebBridgeTags.cpp").c_str());
        const std::string name = "W906_Timer2HeaterTick();";
        size_t call = std::string::npos;
        int calls = 0;
        for (size_t p = src.find(name); p != std::string::npos; p = src.find(name, p + 1)) {
            if (p >= 5 && src.compare(p - 5, 5, "void ") == 0) continue;     // the block-scope `extern void ...;` declaration
            ++calls;
            call = p;
        }
        const size_t pt = src.find("void PumpTick()");
        const size_t guard = (pt == std::string::npos) ? pt : src.find("if (!g_pumpActive) return;", pt);
        const size_t end = (pt == std::string::npos) ? pt : src.find("\n}", pt);
        CHECK(calls == 1);
        CHECK(pt != std::string::npos && guard != std::string::npos && call != std::string::npos && end != std::string::npos);
        CHECK(pt < guard && guard < call && call < end);
    } else {
        CHECK(!"argv[1] = source root (tests/CMakeLists.txt passes it)");
    }

    std::printf("[12] boot: wb_serve calls fMain->IndexHeatMode() once, after PumpInit, on SetTechDataToProd's line (golden :10722)\n");
    if (argc > 1) {
        const std::string ws = Slurp((std::string(argv[1]) + "/tools/wb_serve.cpp").c_str());
        const std::string name = "{ fMain->IndexHeatMode(); }";
        size_t call = std::string::npos;
        int calls = 0;
        for (size_t p = ws.find(name); p != std::string::npos; p = ws.find(name, p + 1)) { ++calls; call = p; }
        const size_t pump = ws.find("if (ht9045::PumpInit(whyNotPump))");
        const size_t techLine = (call == std::string::npos) ? call : ws.find("SetTechDataToProd();", call);
        const size_t eol = (call == std::string::npos) ? call : ws.find('\n', call);
        const size_t bol = (call == std::string::npos) ? call : ws.rfind('\n', call);
        CHECK(calls == 1);
        CHECK(pump != std::string::npos && call != std::string::npos && techLine != std::string::npos);
        CHECK(pump < call && call < techLine && techLine < eol);                  // same line, before SetTechDataToProd (golden :11327)
        CHECK(bol != std::string::npos && ws.compare(bol + 1, 4 + name.size(), "    " + name) == 0);   // first statement on its line
    }

    std::printf("[8] memory only: real files unchanged\n");
    for (int i = 0; i < 2; ++i) {
        std::string after = Slurp(kReal[i]);
        CHECK(after == before[i]);
        if (after != before[i]) std::printf("     changed: %s\n", kReal[i]);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
