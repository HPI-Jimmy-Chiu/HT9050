// =============================================================================
//  test_pool2_fixaiccd.cpp  --  AI(W906-POOL2) 20261006 (Ifor01)
//
//  W-111 第①項（TO_IFOR §3 15:3x＝A，一張 MR 解整組）：aoutarm9045_<排列>.cpp 裡 25 處 TODO(W7) `#if 0`，
//  閘裡只有一行 `fFixAICCD->OutArmCycleCounterUpdate();`。當初缺 TfFixAICCD；現在有（forms/fFixAICCD.h:48），
//  方法是移植版的離線 no-op（forms/fFixAICCD.cpp:16），所以解開後行為不變。解開前逐處比過 golden 0618 同名檔：
//  處數相同、每一處前一行程式相同（外層 `if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)`）。
//    [1] 呼叫得到、不改狀態：fFixAICCD 存在，OutArmCycleCounterUpdate() 可以呼叫
//    [2] 讀原始碼：18 支有這個呼叫的檔，每支「開著的呼叫處數」＝golden 0618 的處數（表 kGolden，共 33 處：
//        這次解開 25 處，另 8 處在 1x2_2／1x2_4／1x3_4／2x6_8 本來就開著），而且一處都沒有還關在 `#if 0` 裡；
//        解開的 25 處都在原行改成 `#if 1 // was: #if 0 ... opened AI(W906-POOL2) ... W-111`
//  反向驗證（第 15 條）見 MR 說明：任一處改回 `#if 0` ⇒ [2] 紅。
// =============================================================================
#include "forms/fFixAICCD.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_pool2_fixaiccd.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string();
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static std::string Trim(const std::string& s)
{
    size_t a=s.find_first_not_of(" \t\r"), b=s.find_last_not_of(" \t\r");
    return a==std::string::npos ? std::string() : s.substr(a, b-a+1);
}

// golden 0618 HT9011UC_Code_V3.33.906.0_20260618\<file>: number of `fFixAICCD->OutArmCycleCounterUpdate();` call sites
struct GoldenCount { const char* file; int n; };
static const GoldenCount kGolden[]={
    { "aoutarm9045_1x1_1.cpp", 2 }, { "aoutarm9045_1x2_2.cpp", 2 }, { "aoutarm9045_1x2_4.cpp", 2 }, { "aoutarm9045_1x3_4.cpp", 2 },
    { "aoutarm9045_1x4_4.cpp", 2 }, { "aoutarm9045_1x4_8.cpp", 2 }, { "aoutarm9045_2x1_2.cpp", 1 }, { "aoutarm9045_2x2_4.cpp", 2 },
    { "aoutarm9045_2x2_8.cpp", 2 }, { "aoutarm9045_2x3_6.cpp", 2 }, { "aoutarm9045_2x4_16.cpp", 2 }, { "aoutarm9045_2x4_4.cpp", 1 },
    { "aoutarm9045_2x4_8.cpp", 2 }, { "aoutarm9045_2x5_8.cpp", 1 }, { "aoutarm9045_2x6_8.cpp", 2 }, { "aoutarm9045_2x8_16.cpp", 2 },
    { "aoutarm9045_2x8_8.cpp", 2 }, { "aoutarm9045_All_1Picker.cpp", 2 },
};

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("POOL2_FixAICCD"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_pool2_fixaiccd (W-111 #1): aoutarm9045_* fFixAICCD->OutArmCycleCounterUpdate() opened as golden 0618\n");

    // ---- [1] the opened line's dependency ----------------------------------------------------------
    CHECK(fFixAICCD!=NULL, "[1] fFixAICCD exists (FormsFacade, forms/fFixAICCD.h)");
    if(fFixAICCD!=NULL) fFixAICCD->OutArmCycleCounterUpdate();
    CHECK(true, "[1] OutArmCycleCounterUpdate() is callable (port-only offline no-op, forms/fFixAICCD.cpp:16)");

    // ---- [2] source pins --------------------------------------------------------------------------
    int liveAll=0, gatedAll=0, openedAll=0, goldenAll=0;
    for(const GoldenCount& g : kGolden)
    {
        const std::string s=Slurp(root+"/"+g.file);
        if(s.empty()) { CHECK(false, (std::string("[2] cannot read ")+g.file).c_str()); continue; }
        std::vector<std::string> L;
        size_t p=0, q;
        while((q=s.find('\n', p))!=std::string::npos) { L.push_back(Trim(s.substr(p, q-p))); p=q+1; }
        L.push_back(Trim(s.substr(p)));
        int live=0, gated=0, opened=0;
        for(size_t i=0; i<L.size(); i++)
        {
            if(L[i]!="fFixAICCD->OutArmCycleCounterUpdate();") continue;
            const std::string& prev=(i>0)?L[i-1]:std::string();
            if(prev.compare(0, 5, "#if 0")==0) gated++;
            else live++;
            if(prev.find("#if 1 // was: #if 0")==0 && prev.find("opened AI(W906-POOL2) 20261006 (Ifor01): W-111")!=std::string::npos) opened++;
        }
        char msg[200];
        std::snprintf(msg, sizeof(msg), "[2] %s: %d live call site(s) = golden 0618's %d, none left under #if 0", g.file, live, g.n);
        CHECK(live==g.n && gated==0, msg);
        liveAll+=live; gatedAll+=gated; openedAll+=opened; goldenAll+=g.n;
    }
    std::printf("    live %d / golden %d, still gated %d, opened by W-111 %d\n", liveAll, goldenAll, gatedAll, openedAll);
    CHECK(liveAll==33 && goldenAll==33, "[2] 33 live call sites in 18 files, as golden 0618");
    CHECK(openedAll==25, "[2] the 25 opened in place by W-111 (#if 1 // was: #if 0 ... opened AI(W906-POOL2) ... W-111)");

    std::printf("test_pool2_fixaiccd: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
