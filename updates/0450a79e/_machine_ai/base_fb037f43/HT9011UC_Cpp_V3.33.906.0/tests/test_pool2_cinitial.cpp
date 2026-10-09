// =============================================================================
//  test_pool2_cinitial.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  POOL-2 `cinitial.cpp`（FROM_IFOR §1 1008 10:5x）：普查列的 8 個在 1008 main 對 golden 0618 看過——開 7、留 1：
//    n2-6／n2-7（RT 旋轉時間／各站旋轉次數，golden :7429-7436）、n2-8／n2-9（FT 那一臂，:7465-7472）
//        ——tRotateShim 已經是 golden 完整型別 TRotate 的別名（aHotPlateSubstrate.h:868），欄位都在；
//    n5-G1b（SetMotorSpeed：Shuttle 維護頁開著就把 shuttle 速度壓到 10%，:5049）——TfShuttleMove 有 fShow 了，照 FShow_Audit 經 W906_FormShowing；
//    n5-G17／n5-G18（吸嘴基準高度加上自動校正的高度差，:15119-15122／:15129-15132）——Enable{In,Out}ArmAutoCalSuckZ 已經有了；
//  留：N1-G5（ChangeSite 先重建 ATC 對照表，:12080）——順序問題還在：wb_serve 在 W906_DoReadLastData 裡才 new fTemp_Set，
//    main 在 InitialHandler（→ SetWorkParameter → ChangeSite）之後才呼叫它；1008 試開過，SCK_ART_Remainder／
//    W906_AutoSiteMapCleanOut 當掉（開機也會）。要和「TfTemp_Set 提前到 InitialHandler 之前建」一起開。
//  這支「真的執行」被開的段落，另加原始碼檢查：
//    [S]  原始碼（argv[1] = 移植樹根目錄）：7 個開閘註記、每個下面是 golden 原文；n5-G1b 那一行經 W906_FormShowing；N1-G5 仍是 `#if 0`
//    [R1] GetInArmSuckBaseHeight／GetOutArmSuckBaseHeight：自動校正關 ⇒ 教導值；開 ⇒ 教導值＋高度差
//    [R2] SetMotorSpeed：fShuttleMove->fShow 開著 ⇒ 兩支 shuttle 馬達壓到 10%；關著 ⇒ 配方速度（Contact／Zteach 都關）
//    [R3] DoSetupSystemToProd：FT ⇒ Prod 的旋轉時間／次數來自 tRotate 的 FT 欄位；RT 模式 ⇒ 來自 *RT 欄位
//  反向驗證見 MR 說明：任一個開的改回 `#if 0` 重編 ⇒ 對應的 [R] 紅＋[S] 紅。
// =============================================================================
#include "cinitial.h"
#include "cmydef.h"
#include "cprod.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "Config.h"
#include "aHotPlateSubstrate.h"           // Zteach, tRotate (TRotate via forms/fRotate.h)
#include "atester_shims.h"                // fContact
#include "forms/fShuttleMove.h"
#include "Motor/mymotor.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
    const char* control = std::getenv("W906_POOL2_CINITIAL_SOURCE");   // reverse checks may point at an opened copy
    std::istringstream in(read(control ? control : root + "/cinitial.cpp"));
    std::vector<std::string> L;
    std::string line;
    while (std::getline(in, line)) L.push_back(line);
    check(L.size() > 15000, "S0 cinitial.cpp read (argv[1] = the port root)");
    struct Gate { const char* tag; const char* next; };
    static const Gate opened[] = {
        { "GATE n2-6 reason expired",   "Prod.RotationTimeIn=tRotate.RotationTimeInRT;" },                     // golden :7429
        { "GATE n2-7 reason expired",   "for(int i=0; i<8; i++)" },                                             // golden :7432
        { "GATE n2-8 reason expired",   "Prod.RotationTimeIn=tRotate.RotationTimeIn;" },                       // golden :7465
        { "GATE n2-9 reason expired",   "for(int i=0; i<8; i++)" },                                             // golden :7468
        { "GATE n5-G1b reason expired", "W906_FormShowing(\"fShuttleMove\", fShuttleMove->fShow)==true ||" },   // golden :5049
        { "GATE n5-G17 reason expired", "if(fProductionInfo->EnableInArmAutoCalSuckZ())" },                     // golden :15119
        { "GATE n5-G18 reason expired", "if(fProductionInfo->EnableOutArmAutoCalSuckZ())" },                    // golden :15129
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
    int closed = 0, n1g5 = 0;
    for (const std::string& l : L)
    {
        const std::string t = trim(l);
        if (t.rfind("#if 0 // GATE n2-6", 0) == 0 || t.rfind("#if 0 // GATE n2-7", 0) == 0 || t.rfind("#if 0 // GATE n2-8", 0) == 0 ||
            t.rfind("#if 0 // GATE n2-9", 0) == 0 || t.rfind("#if 0 // GATE n5-G1b", 0) == 0 || t.rfind("#if 0 // GATE n5-G17", 0) == 0 ||
            t.rfind("#if 0 // GATE n5-G18", 0) == 0)
            ++closed;
        if (t.rfind("#if 0 // N1-G5:", 0) == 0 && t.find("STILL HOLDS, re-checked AI(W906-POOL2) 20261008 (Ifor01)") != std::string::npos)
            ++n1g5;
    }
    check(closed == 0, "S2 none of the 7 is still `#if 0`");
    check(n1g5 == 1, "S3 N1-G5 stays `#if 0` with the 1008 re-check note (fTemp_Set is created after InitialHandler -> ChangeSite)");
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("POOL2_CInitial"))
        return 2;
    sources(argc > 1 ? argv[1] : "..");
    if (g_fail)
        return 1;                                     // a failed source check stops before any runtime I/O

    std::puts("-- [R1] GetInArmSuckBaseHeight / GetOutArmSuckBaseHeight (n5-G17 / n5-G18, golden :15117-15134)");
    Tech.iInArmZHeightSub[1][2] = 5000;
    Tech.iOutArmZHeightSub[1][2] = 6000;
    iInArmZHeightDiff[1][2] = 37;
    iOutArmZHeightDiff[1][2] = -41;
    bEnableInarmSuckZAuto = false;
    bEnableOutarmSuckZAuto = false;
    check(GetInArmSuckBaseHeight(1, 2) == 5000, "R1 in-arm, auto-cal off: the taught height");
    check(GetOutArmSuckBaseHeight(1, 2) == 6000, "R1 out-arm, auto-cal off: the taught height");
    bEnableInarmSuckZAuto = true;
    check(GetInArmSuckBaseHeight(1, 2) == 5037, "R1 in-arm, auto-cal on: taught + iInArmZHeightDiff (golden :15119-15122)");
    check(GetOutArmSuckBaseHeight(1, 2) == 6000, "R1 out-arm follows its own switch (still off)");
    bEnableOutarmSuckZAuto = true;
    check(GetOutArmSuckBaseHeight(1, 2) == 5959, "R1 out-arm, auto-cal on: taught + iOutArmZHeightDiff (golden :15129-15132)");
    bEnableInarmSuckZAuto = false;
    bEnableOutarmSuckZAuto = false;

    std::puts("-- [R2] SetMotorSpeed: the Shuttle-Maintain clamp (n5-G1b, golden :5048-5059)");
    W906_TestEnsureSimMotors();                       // golden's boot leaves no MOT[i].Motor NULL (InitialMotorParameter)
    MOT[MInShuttle1].Motor->PJogHighSpeed = 1000;
    MOT[MInShuttle2].Motor->PJogHighSpeed = 1000;
    SHSpeed.iSH1Sp = 60;
    SHSpeed.iSH2Sp = 70;
    fContact->fShow = false;
    Zteach->fShow = false;
    fShuttleMove->fShow = false;
    SetMotorSpeed();
    char m[200];
    std::snprintf(m, sizeof(m), "R2 Shuttle-Maintain closed: recipe speed 60%% / 70%% (speed %d / %d)", MOT[MInShuttle1].speed, MOT[MInShuttle2].speed);
    check(MOT[MInShuttle1].speed == 600 && MOT[MInShuttle2].speed == 700, m);
    fShuttleMove->fShow = true;
    SetMotorSpeed();
    std::snprintf(m, sizeof(m), "R2 Shuttle-Maintain open: both shuttles clamped to 10%% (speed %d / %d)", MOT[MInShuttle1].speed, MOT[MInShuttle2].speed);
    check(MOT[MInShuttle1].speed == 100 && MOT[MInShuttle2].speed == 100, m);
    fShuttleMove->fShow = false;

    std::puts("-- [R3] DoSetupSystemToProd: rotation dwell times and counts (n2-6..n2-9, golden :7426-7472)");
    tRotate.RotationTimeIn = 11;     tRotate.RotationTimeOut = 12;
    tRotate.RotationTimeInRT = 21;   tRotate.RotationTimeOutRT = 22;
    for (int i = 0; i < 8; ++i)
    {
        tRotate.RotationCount[i] = 100 + i;   tRotate.OutRotationCount[i] = 200 + i;
        tRotate.RotationCountRT[i] = 300 + i; tRotate.OutRotationCountRT[i] = 400 + i;
    }
    CosFunction.bRotateUseRTmode = false;
    iRunStartMode = FT;
    Prod.RotationTimeIn = Prod.RotationTimeOut = -1;
    for (int i = 0; i < 8; ++i) Prod.RotationCount[i] = Prod.OutRotationCount[i] = -1;
    DoSetupSystemToProd();
    check(Prod.RotationTimeIn == 11 && Prod.RotationTimeOut == 12, "R3 FT arm: Prod.RotationTimeIn/Out = tRotate.RotationTimeIn/Out (golden :7465-7466)");
    check(Prod.RotationCount[0] == 100 && Prod.RotationCount[7] == 107 && Prod.OutRotationCount[0] == 200 && Prod.OutRotationCount[7] == 207,
          "R3 FT arm: Prod.(Out)RotationCount[0..7] = tRotate.(Out)RotationCount (golden :7468-7472)");
    CosFunction.bRotateUseRTmode = true;
    tRotate.bRotateUseRTmode = true;
    iRunStartMode = RT;                               // not FT / FT_ART => bRunRT (golden :7135-7153)
    Prod.RotationTimeIn = Prod.RotationTimeOut = -1;
    for (int i = 0; i < 8; ++i) Prod.RotationCount[i] = Prod.OutRotationCount[i] = -1;
    DoSetupSystemToProd();
    check(Prod.RotationTimeIn == 21 && Prod.RotationTimeOut == 22, "R3 RT arm: Prod.RotationTimeIn/Out = tRotate.RotationTimeIn/OutRT (golden :7429-7430)");
    check(Prod.RotationCount[0] == 300 && Prod.RotationCount[7] == 307 && Prod.OutRotationCount[0] == 400 && Prod.OutRotationCount[7] == 407,
          "R3 RT arm: Prod.(Out)RotationCount[0..7] = tRotate.(Out)RotationCountRT (golden :7432-7436)");
    CosFunction.bRotateUseRTmode = false;
    tRotate.bRotateUseRTmode = false;
    iRunStartMode = FT;

    std::printf("test_pool2_cinitial: %d failure(s)\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
