// =============================================================================
//  test_it1_mytemppanel.cpp  --  AI(W906-IT1) 20261006 (Ifor01)
//
//  IT-1 第二批（FROM_IFOR §1 1006 10:2x）：MyTempPanel.cpp 照 golden 0618 解開的 3 個 `#if 0`——點溫度欄位時叫數字鍵盤、
//  帶對的上下限（golden MyTempPanel.cpp:417-526／:539-560／:562-599）。模擬組態的數字鍵盤是「立即送出」
//  （forms/fQwertyKey.cpp ShowQwertyKey：ShowModal 是 no-op，之後照 golden 用 CheckRange 把值壓進上下限），
//  所以欄位先填 999，呼叫處理函式後看值被壓到哪個上限，就知道函式有跑、選的限值對不對：
//
//    [1] edBaseMouseDown：DUT 1 面板 ⇒ InputLimit.iTempHigh（200）；熱風槍 1 面板（INSTALL_HEAT_GUN）⇒ iHeaterGunH（250）
//    [2] edIndiviMouseDown：⇒ fTemp_Set->MaxTempSetting()（測試自己呼叫同一支算期望值）
//    [3] edinitialMouseDown：DUT 1 ⇒ iTempHigh；熱風槍 1 ⇒ iHeaterGunH（非 CC_ASE_KaohSiung 那一臂）
//    [4] 範圍內的值（50）不動
//    [5] 讀原始碼：MyTempPanel.cpp 剩 64 個 `#if 0`；三處是 `#if 1 // was: #if 0 -- opened AI(W906-IT1)`
//    [6] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
//  反向驗證（第 15 條）見 MR 說明：三處任一改回 `#if 0` ⇒ 對應那一節的欄位留在 999 ⇒ 紅。
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"                 // INSTALL_HEAT_GUN / Tri_Temp_Machine / CUSTOMER_CODE / dTempMax
#include "cprod.h"                  // InputLimit / Temperature
#include "CosFunction.h"
#include "LastSet.h"
#include "MyTempPanel.h"
#include "forms/fTemp_Set.h"        // fTemp_Set
#include "forms/fQwertyKey.h"       // fQwertyKey
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <string>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_it1_mytemppanel.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static double Val(TEdit* e) { return std::atof(e->Text.c_str()); }

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("IT1_MyTempPanel"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("test_it1_mytemppanel (IT-1 batch 2): MyTempPanel.cpp MouseDown bodies opened\n");
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);

    if(fTemp_Set==NULL) fTemp_Set=new TfTemp_Set();            // as wb_serve.cpp:3124
    CUSTOMER_CODE=CC_PTI;                                     // HT9050 (not CC_ASE_KaohSiung -> edinitialMouseDown's else arm)
    Tri_Temp_Machine=0;
    INSTALL_HEAT_GUN=1;
    CosFunction.bOffsetTempByRecipeMinMaxLimit=false;
    InputLimit.iTempHigh=200;   InputLimit.iTempLow=0;
    InputLimit.iHeaterGunH=250; InputLimit.iHeaterGunL=0;
    LastSet.iTemperature=Tempture_Hot;
    Temperature.fWorkTemperBase=80.0;

    TMyTempPanel* dut=new TMyTempPanel("DUT 1", tcDUT1);
    TMyTempPanel* gun=new TMyTempPanel("Hot Air 1", tcHeatGun1);
    CHECK(dut->edBase->Tag==tcDUT1 && gun->edBase->Tag==tcHeatGun1 && gun->edInitTempOffset->Tag==tcHeatGun1,
          "[0] the ctor sets each edit's Tag to the zone id (MyTempPanel.cpp :555-590) -- what the handlers branch on");

    // ---- [1] edBaseMouseDown ----------------------------------------------------------------------
    std::printf("[1] edBaseMouseDown\n");
    dut->edBase->Text="999";  dut->W906_MouseDown(0, dut->edBase);
    gun->edBase->Text="999";  gun->W906_MouseDown(0, gun->edBase);
    std::printf("    DUT 1 edBase %s, Hot Air 1 edBase %s\n", dut->edBase->Text.c_str(), gun->edBase->Text.c_str());
    CHECK(Val(dut->edBase)==200.0, "[1] DUT 1: 999 -> InputLimit.iTempHigh 200 (the keypad ran with the zone's range)");
    CHECK(Val(gun->edBase)==250.0, "[1] Hot Air 1: 999 -> InputLimit.iHeaterGunH 250 (heat-gun arm, by Tag)");

    // ---- [2] edIndiviMouseDown --------------------------------------------------------------------
    std::printf("[2] edIndiviMouseDown\n");
    const double wantMax=fTemp_Set->MaxTempSetting();
    dut->edIndiTemp->Text="999";  dut->W906_MouseDown(2, dut->edIndiTemp);
    std::printf("    DUT 1 edIndiTemp %s (MaxTempSetting %.1f)\n", dut->edIndiTemp->Text.c_str(), wantMax);
    CHECK(wantMax<999.0 && Val(dut->edIndiTemp)==wantMax, "[2] 999 -> fTemp_Set->MaxTempSetting()");

    // ---- [3] edinitialMouseDown -------------------------------------------------------------------
    std::printf("[3] edinitialMouseDown\n");
    dut->edInitTempOffset->Text="999";  dut->W906_MouseDown(3, dut->edInitTempOffset);
    gun->edInitTempOffset->Text="999";  gun->W906_MouseDown(3, gun->edInitTempOffset);
    std::printf("    DUT 1 %s, Hot Air 1 %s\n", dut->edInitTempOffset->Text.c_str(), gun->edInitTempOffset->Text.c_str());
    CHECK(Val(dut->edInitTempOffset)==200.0, "[3] DUT 1: 999 -> iTempHigh 200");
    CHECK(Val(gun->edInitTempOffset)==250.0, "[3] Hot Air 1: 999 -> iHeaterGunH 250 (FW-SETUP-W19's 'Tag is always 0' no longer holds)");

    // ---- [4] in range -----------------------------------------------------------------------------
    dut->edBase->Text="50";  dut->W906_MouseDown(0, dut->edBase);
    CHECK(Val(dut->edBase)==50.0, "[4] a value inside the range is left alone");

    // ---- [5] source pins --------------------------------------------------------------------------
    std::printf("[5] source\n");
    {
        const std::string s=Slurp(root+"/MyTempPanel.cpp");
        int nIf0=0, nOpen=0;
        size_t p=0;
        while((p=s.find("\n#if 0", p))!=std::string::npos) { nIf0++; p++; }
        p=0;
        while((p=s.find("#if 1 // was: #if 0 -- opened AI(W906-IT1)", p))!=std::string::npos) { nOpen++; p++; }
        CHECK(nIf0==64, "[5] MyTempPanel.cpp: 64 `#if 0` left (67 before; the 62 layout gates + :628 / :644 need vclcompat members)");
        CHECK(nOpen==3, "[5] the three MouseDown bodies opened in place");
    }

    // ---- [6] real files ---------------------------------------------------------------------------
    for(int i=0; i<2; i++) CHECK(Slurp(kReal[i])==before[i], "[6] real machine file unchanged");

    std::printf("test_it1_mytemppanel: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
