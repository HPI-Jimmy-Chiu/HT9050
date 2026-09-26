// ===========================================================================
//  tests/test_opmode_body.cpp
//
//  AI(W906-OPMODE) 20260926: golden TfMain::UpdateMainOperateMode（main.cpp:12803-13127）翻進
//  forms/fMain_OperateMode.cpp（TfMain::W906_UpdateMainOperateModeBody，ht9045_sm）；虛擬的
//  TfMain::UpdateMainOperateMode 留在 forms/fMain.cpp:507（ht9045_forms）計數＋經 hook 呼叫本體，
//  wb_serve 開機才裝 hook（wb_serve.cpp:4064）。
//
//    [1] 對照組：沒裝 hook ⇒ 只計數，golden 的第一句（edWorkTemperBase->Text=Temperature.fWorkTemperBase）沒有發生
//    [2] 裝 hook ⇒ 真本體：第一句發生（golden :12807）、計數照舊 +1
//    [3] Hot（golden :12918 的 else 臂）：fHeaterOK 清成 false（:13000）；非 ASE 高雄 bHeatOKBellowError 清成 false（:13001-13002）
//    [4] AmbientHot（:12948-12951）：edSoakTime->Text="0"、Enabled=false
//    [5] 本體尾端 WriteLastDataFile／ReadLastDataFile（:13119／:13123）落在 test_bootstrap 的沙盒：
//        D:\HT9045\system 的 lastdata 三個檔與 D:\HT9045\config\config.ini 前後逐位元組相同
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <string>
#include "forms/fMain.h"
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "cmydef.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "MachineType.h"

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_opmode_body.cpp:%d]  %s\n", line, e); }
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

int main()
{
    static const char* const kReal[4] = {
        "D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\lastdata_backup.dat",
        "D:\\HT9045\\system\\lastdata_backup2.dat", "D:\\HT9045\\config\\config.ini" };
    std::string before[4];
    for (int i = 0; i < 4; ++i) before[i] = Slurp(kReal[i]);

    CHECK(fMain != 0);
    if (fMain == 0) { std::printf("FAIL: no fMain\n"); return 1; }
    CHECK(W906_UpdateMainOperateModeHook == 0);            // 測試行程沒有裝（只有 wb_serve 開機會裝）

    std::printf("[1] control: no hook -> counter only\n");
    Temperature.fWorkTemperBase = 125.5;
    fMain->edWorkTemperBase->Text = "unchanged";
    const int c0 = fMain->W906_UpdateMainOperateModeCallCount;
    fMain->UpdateMainOperateMode();
    CHECK(fMain->W906_UpdateMainOperateModeCallCount == c0 + 1);
    CHECK(fMain->edWorkTemperBase->Text == AnsiString("unchanged"));

    std::printf("[2] hook installed -> the golden body runs\n");
    W906_InstallUpdateMainOperateMode();
    CHECK(W906_UpdateMainOperateModeHook != 0);
    LastSet.iTemperature = Tempture_Ambient;
    fMain->UpdateMainOperateMode();
    CHECK(fMain->W906_UpdateMainOperateModeCallCount == c0 + 2);
    CHECK(fMain->edWorkTemperBase->Text == AnsiString(Temperature.fWorkTemperBase));   // golden :12807

    std::printf("[3] Hot: fHeaterOK / bHeatOKBellowError cleared (golden :13000-13002)\n");
    LastSet.iTemperature = Tempture_Hot;
    fHeaterOK = true;
    bHeatOKBellowError = true;
    fMain->UpdateMainOperateMode();
    CHECK(fHeaterOK == false);
    if (CUSTOMER_CODE != CC_ASE_KaohSiung) CHECK(bHeatOKBellowError == false);

    std::printf("[4] AmbientHot: soak time forced to 0 and disabled (golden :12950-12951)\n");
    LastSet.iTemperature = Tempture_AmbientHot;
    fMain->edSoakTime->Text = "30";
    fMain->edSoakTime->Enabled = true;
    fMain->UpdateMainOperateMode();
    CHECK(fMain->edSoakTime->Text == AnsiString("0"));
    CHECK(fMain->edSoakTime->Enabled == false);

    std::printf("[5] the WriteLastDataFile / ReadLastDataFile tail stayed in the sandbox\n");
    for (int i = 0; i < 4; ++i) CHECK(Slurp(kReal[i]) == before[i]);

    W906_UpdateMainOperateModeHook = 0;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
