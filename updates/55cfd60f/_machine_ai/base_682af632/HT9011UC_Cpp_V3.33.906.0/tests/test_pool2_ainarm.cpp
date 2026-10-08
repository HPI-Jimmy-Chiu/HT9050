// =============================================================================
//  test_pool2_ainarm.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  POOL-2 `ainarm9045.cpp`（FROM_IFOR §1 1008 09:1x）：普查列的 8 個在 1008 main 對 golden 0618 看過——開 6、留 2。
//    開：W7D-K2-G08（Lot ID 寫進吸料紀錄，golden :2724-2728）、G11（VTEST 第一筆 MES 報表初始化，:2741-2759）、
//        G12（矽格 Loader 統計，:2769-2770）、G13（PTI RefreshOtherTool，:2796-2799）、TODO(W7) WAR0120（:7332-7333）、
//        TODO(W7) InspectInArmPosition——閘裡原本只有註解，照 golden :8596-8695 逐字補翻（E74 除錯選項才跑）。
//    留：W7D-G01（EnableTraymapCheckFunction 有了，但唯一用它的 G01b 還缺 TfTrayMapping::iTrayMappingDateCheck，要一起開）、
//        GATE(W906-INADD)（tRotate.bART_RT_NoRotate：TRotate 沒翻，替身沒這個欄位）。
//  這支「真的執行」被開的段落（1008 盲做比較的教訓），另加原始碼檢查：
//    [S]  原始碼（argv[1] = 移植樹根目錄）：6 個開閘註記；G01／INADD 仍是 `#if 0`
//    [R1] InspectInArmPosition：E74 開＋位置超差 ⇒ ShowMyMessage 一次、訊息照 golden 格式；E74 關 ⇒ 不呼叫（golden 提早 return）
//    [R2] ProcessSCKARTLoadingCount 第 200 步 ⇒ ShowErrorMessage("WAR0120")、照舊回 true
//    G08／G11／G12／G13 在 AddLoadingCount 裡，只有 [S] 檢查：樹裡沒有任何測試跑過 AddLoadingCount——它一進來就呼叫
//    TMyKitSuck::CopyFromTray，沒有料盤環境會當掉（1008 試過：SIGSEGV 在 CopyFromTray）；要真的跑，得先做料盤測試環境（另一張卡）。
//  反向驗證見 MR 說明：任一個改回 `#if 0` 重編 ⇒ 對應的 [R] 紅＋[S] 紅。
// =============================================================================
#include "ainarm9045.h"
#include "aHotPlateSubstrate.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "canary_support.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern bool ProcessSCKARTLoadingCount(bool bReset);                                      // ainarm9045.cpp (default arg only there)

static int g_fail = 0;
static void check(bool ok, const char* label)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok) ++g_fail;
}
static std::string read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream o;
    o << f.rdbuf();
    return o.str();
}
static std::string trim(const std::string& l)
{
    size_t a = l.find_first_not_of(" \t\r");
    size_t b = l.find_last_not_of(" \t\r");
    return a == std::string::npos ? std::string() : l.substr(a, b - a + 1);
}

static void sources(const std::string& root)
{
    const char* control = std::getenv("W906_POOL2_AINARM_SOURCE");     // reverse checks may point at an opened copy
    std::istringstream in(read(control ? control : root + "/ainarm9045.cpp"));
    std::vector<std::string> L;
    std::string line;
    while (std::getline(in, line)) L.push_back(line);
    struct Gate { const char* tag; const char* next; };
    static const Gate opened[] = {
        { "GATE W7D-K2-G08 reason expired",            "else if(fLotInfo->tsLotID->TabVisible==true &&" },
        { "GATE W7D-K2-G11 reason expired",            "if(IniConfig.bVTESTFunction==true && IniConfig.bCheckFile==true)" },
        { "GATE W7D-K2-G12 reason expired",            "if(IniConfig.bSIGURDFunction)" },
        { "GATE W7D-K2-G13 reason expired",            "if(CUSTOMER_CODE==CC_PTI)" },
        { "TODO(W7) WAR0120 --",                       "ErrPart=InArmSuck.Suck[iLoadPickX][iLoadPickY].sName;" },
        { "TODO(W7) InspectInArmPosition translated",  "bool bX, bY;" },
    };
    for (const Gate& g : opened)
    {
        int hits = 0, good = 0;
        for (size_t i = 0; i + 1 < L.size(); ++i)
            if (trim(L[i]).rfind("#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): ", 0) == 0 && L[i].find(g.tag) != std::string::npos)
            {
                ++hits;
                if (trim(L[i + 1]).rfind(g.next, 0) == 0) ++good;
            }
        char m[200];
        std::snprintf(m, sizeof(m), "S1 %s: opened in place, golden statement under it (%d / %d)", g.tag, hits, good);
        check(hits == 1 && good == 1, m);
    }
    int g01 = 0, inadd = 0;
    for (const std::string& l : L)
    {
        if (trim(l).rfind("#if 0 // GATE W7D-G01 --", 0) == 0) ++g01;
        if (trim(l).rfind("#if 0 // GATE(W906-INADD)", 0) == 0) ++inadd;
    }
    check(g01 == 1, "S2 W7D-G01 stays (its only consumer G01b still lacks TfTrayMapping::iTrayMappingDateCheck)");
    check(inadd == 1, "S2 GATE(W906-INADD) stays (tRotate.bART_RT_NoRotate is not on the TRotate stand-in)");
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("POOL2_AinArm"))
        return 2;
    sources(argc > 1 ? argv[1] : "..");
    if (g_fail)
        return 1;                                     // a failed source check stops before any runtime I/O

    std::puts("-- [R1] InspectInArmPosition (golden :8596-8695, E74 only)");
    {   // wb_serve's boot order (tools/wb_serve.cpp: SetMyKitSuckItemAmount, then the ARM_OFFSET objects), as test_w6_2_inarm_canary does:
        //   the translated body reads InArmOffSet[...]->GetVariable(), which this test never booted
        void SetMyKitSuckItemAmount();
        void EnsureArmOffsetObjects();
        SetMyKitSuckItemAmount();
        EnsureArmOffsetObjects();
    }
    InArmSuck.Item[0][0] = HAS_IC;
    InputLimit.iOffsetXYHigh = 1;                     // +-100 pulses
    Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase] = 900000;   // far from wherever GetInArmCellPos puts the cell
    Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase] = 900000;
    bUseTwoArm32Site = false;
    IniConfig.bE74_InspectArmPosition = false;
    W906_ShowMyMessage_Reset();
    InspectInArmPosition(MMTrayY, 0, 0, 0, 0, false);
    check(W906_ShowMyMessage_Count == 0, "R1 E74 off: golden returns before any check (no message)");
    IniConfig.bE74_InspectArmPosition = true;
    W906_ShowMyMessage_Reset();
    InspectInArmPosition(MMTrayY, 0, 0, 0, 0, false);
    const std::string msg = W906_ShowMyMessage_LastS1.c_str();
    check(W906_ShowMyMessage_Count == 1 && msg.find("InArm Suck[0, 0]") == 0 && msg.find("Pick from Loader [0, 0]") != std::string::npos,
          "R1 E74 on, position out of tolerance: one ShowMyMessage in golden's format (InArm Suck[r, c] ... Pick from Loader [r, c] ...)");
    InArmSuck.Item[0][0] = HAS_NULL_IC;
    W906_ShowMyMessage_Reset();
    InspectInArmPosition(MMTrayY, 0, 0, 0, 0, false);
    check(W906_ShowMyMessage_Count == 0, "R1 E74 on, the nozzle holds no IC: no check (golden :8601-8602)");
    IniConfig.bE74_InspectArmPosition = false;

    std::puts("-- [R2] ProcessSCKARTLoadingCount step 200 (golden :7332-7333)");
    W906_ShowErrorMessage_Reset();
    iProcessSCKARTLoadingCountTask = 200;
    const bool r200 = ProcessSCKARTLoadingCount(false);
    check(r200 && W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == "WAR0120",
          "R2 step 200 raises WAR0120 (K_RETRY, MInArmX) and still returns true");
    ProcessSCKARTLoadingCount(true);                  // back to step 1

    std::printf("test_pool2_ainarm: %d failure(s)\n", g_fail);
    return g_fail ? 1 : 0;
}
