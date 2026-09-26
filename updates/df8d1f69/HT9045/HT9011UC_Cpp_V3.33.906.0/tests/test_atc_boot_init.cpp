// ===========================================================================
//  tests/test_atc_boot_init.cpp
//
//  AI(W906-ATC1) 20260926: golden TfMain::FormShow main.cpp:9357-9361 → W906_BootInitialATC（forms/fMain_ATCSiteUse.cpp 檔尾）
//  NB2 R72 ATC-1：USE_ATC_MODE=4 的機台 ChangeATCSiteUse 對空的 ATC_SYS_PAL 取 [0] ⇒ 當機。
//
//    [1] 對照組：測試行程開始時 ATC_SYS_PAL 是空的（就是當機的前提）
//    [2] ATC_SYSTEM 不是 Hontech（5）⇒ 不建通道，但 ATCIniPath 照設（golden :9361 不分種類）
//    [3] ATC_SYSTEM＝eATCHonPrecType ⇒ 建 4 個通道（golden :9359 InitialATC(…, 4, …)）
//    [4] 當機路徑：InitialOK、SingleSite、沒開主動冷卻 ⇒ ChangeATCSiteUse 走 SetRunATC(false)（ATCInterface.cpp:1134 取 [0]）—— 不當、而且真的走到了
//        （ATC_SYS_PAL[0]->bATCRunSetting 被寫成 false；先把它設成 true 當對照）
// ===========================================================================
#include <cstdio>
#include "forms/fMain.h"
#include "ATC/ATCInterface.h"
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "cprod.h"
#include "MachineType.h"

void W906_BootInitialATC();   // forms/fMain_ATCSiteUse.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_atc_boot_init.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

int main()
{
    CHECK(ATCInterfaceForm != 0);  CHECK(fMain != 0);
    if (ATCInterfaceForm == 0 || fMain == 0) { std::printf("FAIL: no ATCInterfaceForm / fMain\n"); return 1; }

    std::printf("[1] control: ATC_SYS_PAL starts empty\n");
    CHECK(ATCInterfaceForm->ATC_SYS_PAL.size() == 0);

    std::printf("[2] ATC_SYSTEM=5 -> no channels, ATCIniPath still set\n");
    const int saveSys = ATC_SYSTEM;
    ATC_SYSTEM = 5;
    ATCInterfaceForm->ATCIniPath = "";
    W906_BootInitialATC();
    CHECK(ATCInterfaceForm->ATC_SYS_PAL.size() == 0);
    CHECK(ATCInterfaceForm->ATCIniPath == AnsiString("D:\\HT9045\\system\\ATC.ini"));

    std::printf("[3] ATC_SYSTEM=eATCHonPrecType -> 4 channels\n");
    ATC_SYSTEM = eATCHonPrecType;
    W906_BootInitialATC();
    CHECK(ATCInterfaceForm->ATC_SYS_PAL.size() == 4);

    std::printf("[4] the crash path: InitialOK + SingleSite + no active cooling -> SetRunATC(false) reaches ATC_SYS_PAL[0]\n");
    if (ATCInterfaceForm->ATC_SYS_PAL.size() == 4) {
        const bool saveInit = InitialOK;  const int saveMode = TestIF_File.iTestMode;  const bool saveCool = Temperature.bATCActiveCooling;
        InitialOK = true;  TestIF_File.iTestMode = SingleSite;  Temperature.bATCActiveCooling = false;
        ATCInterfaceForm->ATC_SYS_PAL[0]->bATCRunSetting = true;
        fMain->ChangeATCSiteUse();
        CHECK(ATCInterfaceForm->ATC_SYS_PAL[0]->bATCRunSetting == false);
        InitialOK = saveInit;  TestIF_File.iTestMode = saveMode;  Temperature.bATCActiveCooling = saveCool;
    }
    ATC_SYSTEM = saveSys;

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
