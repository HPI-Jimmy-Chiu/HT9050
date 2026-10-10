// =============================================================================
//  test_pool2_aoutarm.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  POOL-2 `aoutarm9045.cpp`（FROM_IFOR §1 1008 14:0x）：普查列的 8 個在 1008 main 對 golden 0618 看過——開 7、留 1，
//  另外補翻 InspectOutArmPosition、順手開 ainarm9045.cpp 的 GATE(W906-INADD)（!330 留它的理由當時就已過期）。
//    開：GetOutArmCellPos／GetOutArmToShtCellPos 讀編碼器（golden :3836-3859）、IsMotorArrival 讀位置（:3804）、
//        DoOutArmIonFanGiveWay 的加熱盤 HasIC（:4692-4693）、MoveOutArmXY_To_ESDSafePos 讀 pitch 軸（:4733-4747）、
//        DoOutArmPlaceToAuto 先關訊息框（:3029-3030，經 W906_FormShowing）、CheekNeedToDoOutArmAdditionalFunction 的
//        ART 重測不旋轉（:2164-2165）；ainarm9045.cpp CheekNeedToDoInArmAdditionalFunction 的同一條（golden :3192-3202）。
//    補翻：InspectOutArmPosition（golden :3870-3992；閘裡原本只有註解；E74 除錯選項才跑）。
//    留：DoOutArm_9045 的 AOA 區塊（golden :460-526）——CheckOutArmAutoAlignmentCKModeBeUse 樹裡沒有、
//        CheckOutArmAutoAlignmentTrayModeBeUse 只是回 false 的替身。
//  這支「真的執行」被開的段落，另加原始碼檢查：
//    [S]  原始碼（argv[1] = 移植樹根目錄）：9 個開閘／補翻註記、每個下面是 golden 原文；AOA 區塊仍是 `#if 0` 且有說明
//    [R1] GetOutArmCellPos／GetOutArmToShtCellPos（sim 馬達）：基準格的座標＝MOutArmX／MOutArmY 目前位置
//    [R2] IsMotorArrival：到位 ⇒ true；差 500 ⇒ false
//    [R3] DoOutArmIonFanGiveWay 第 5 步、清料中：加熱盤 1 有料 ⇒ 不讓位（回 false）；兩盤都空 ⇒ 讓位
//    [R4] CheekNeedToDoOutArmAdditionalFunction：旋轉站要轉，但 ART 重測不旋轉三條件都成立 ⇒ bOutRotator＝false
//    [R5] CheekNeedToDoInArmAdditionalFunction：同上（入料臂，另要 ART 啟動模式且 iFTRTCount!=0）⇒ bInRotator＝false
//    [R6] InspectOutArmPosition：E74 開＋位置超差 ⇒ ShowMyMessage 一次、格式照 golden；E74 關、或不是放料 ⇒ 不呼叫
//  只有原始碼檢查的兩個：ESD 安全位置讀 pitch 軸（結果只餵給 OutArmContinuousMove_9045 的連續移動）、DoOutArmPlaceToAuto 的
//  訊息框（在 AQL 分料完成那一支的深處）。
//  反向驗證見 MR 說明：任一個改回 `#if 0` 重編 ⇒ 對應的 [R] 紅＋[S] 紅。
// =============================================================================
#include "aoutarm9045.h"
#include "aHotPlateSubstrate.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "canary_support.h"
#include "mykitsuck.h"
#include "Motor/mymotor.h"
#include "forms/fRotate.h"                // TRotate tRotate
#include "forms/fSCKART.h"                // fSCKART->iFTRTCount
#include "RotateKit/aRotateKIT.h"         // iInRotateFinish / iOutRotateFinish
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern bool bOutRotator;                  // aoutarm9045.cpp:91 (golden :2133)
extern bool bInRotator;                   // ainarm9045.cpp file scope
extern int  iOutArmIonFanGiveWayTask;     // aoutarm9045.cpp:99 (golden :4607)
bool CheekNeedToDoInArmAdditionalFunction();   // ainarm9045.cpp
void SetMyKitSuckItemAmount();
void EnsureArmOffsetObjects();

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
static std::vector<std::string> lines(const char* env, const std::string& path)
{
    const char* control = std::getenv(env);       // reverse checks may point at an opened copy
    std::istringstream in(read(control ? control : path));
    std::vector<std::string> L;
    std::string line;
    while (std::getline(in, line)) L.push_back(line);
    return L;
}
static void pin(const std::vector<std::string>& L, const char* file, const char* tag, const char* next)
{
    int hits = 0, good = 0;
    for (size_t i = 0; i + 1 < L.size(); ++i)
        if (trim(L[i]).rfind("#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): ", 0) == 0 && L[i].find(tag) != std::string::npos)
        {
            ++hits;
            if (trim(L[i + 1]).rfind(next, 0) == 0) ++good;
        }
    char m[220];
    std::snprintf(m, sizeof(m), "S1 %s %s: opened in place, golden statement under it (%d / %d)", file, tag, hits, good);
    check(hits == 1 && good == 1, m);
}

static void sources(const std::string& root)
{
    const std::vector<std::string> O = lines("W906_POOL2_AOUTARM_SOURCE", root + "/aoutarm9045.cpp");
    const std::vector<std::string> I = lines("W906_POOL2_AINARM2_SOURCE", root + "/ainarm9045.cpp");
    check(O.size() > 5000 && I.size() > 10000, "S0 aoutarm9045.cpp / ainarm9045.cpp read (argv[1] = the port root)");
    pin(O, "aoutarm9045.cpp", "TODO(W7) GetOutArmCellPos encoders reason expired",     "iPitchPulse =MOT[MOutArmPitch ].ReadPos();");
    pin(O, "aoutarm9045.cpp", "TODO(W7) GetOutArmToShtCellPos encoders reason expired", "iPitchPulse =MOT[MOutArmPitch ].ReadPos();");
    pin(O, "aoutarm9045.cpp", "TODO(W7) MMPlate HasIC reason expired",                 "bPlate1HasIC=MOT[MMPlate1].HasIC();");
    pin(O, "aoutarm9045.cpp", "TODO(W7) IsMotorArrival reason expired",                "iReadPos=MOT[iMot].ReadPos();");
    pin(O, "aoutarm9045.cpp", "TODO(W7) ESD safe-pos pitch reads reason expired",      "iXVariable[0]=MOT[MOutArmPitch].ReadPos();");
    pin(O, "aoutarm9045.cpp", "GATE(W906-OUTADD) bART_RT_NoRotate reason expired",     "if(CosFunction.bART_RT_NoRotate && tRotate.bART_RT_NoRotate && bCanRunSCKART)");
    pin(O, "aoutarm9045.cpp", "GATE(W906-ARM3) MyMessageBox reason expired",           "if(W906_FormShowing(\"MyMessageBox\", MyMessageBox->Visible)==true)");
    pin(O, "aoutarm9045.cpp", "TODO(W7) InspectOutArmPosition translated",             "bool bX, bY;");
    pin(I, "ainarm9045.cpp",  "GATE(W906-INADD) reason expired",                       "if(CosFunction.bART_RT_NoRotate &&");
    int kept = 0;
    for (const std::string& l : O)
        if (trim(l).rfind("#if 0 // TODO(W7) -- golden aoutarm9045.cpp:460-526", 0) == 0 &&
            l.find("STILL HOLDS, re-checked AI(W906-POOL2) 20261008 (Ifor01)") != std::string::npos)
            ++kept;
    check(kept == 1, "S2 DoOutArm_9045 AOA block (golden :460-526) stays `#if 0` with the 1008 re-check note");
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("POOL2_AOutArm"))
        return 2;
    sources(argc > 1 ? argv[1] : "..");
    if (g_fail)
        return 1;                                     // a failed source check stops before any runtime I/O
    char m[260];

    std::puts("-- [R1] GetOutArmCellPos / GetOutArmToShtCellPos read the arm encoders (golden :3834-3859)");
    W906_TestEnsureSimMotors();
    MOT[MOutArmX].Motor->SetPosition(12345);
    MOT[MOutArmY].Motor->SetPosition(23456);
    OutArmSuck.iPickStep = 1;
    int x = 0, y = 0;
    GetOutArmCellPos(iOutArmYBase, iOutArmXBase, y, x);
    std::snprintf(m, sizeof(m), "R1 GetOutArmCellPos base cell = (MOutArmY, MOutArmX) = (23456, 12345) (got %d, %d)", y, x);
    check(x == 12345 && y == 23456, m);
    x = y = 0;
    GetOutArmToShtCellPos(iOutArmYBase, iOutArmXBase, y, x);
    std::snprintf(m, sizeof(m), "R1 GetOutArmToShtCellPos base cell = (23456, 12345) (got %d, %d)", y, x);
    check(x == 12345 && y == 23456, m);

    std::puts("-- [R2] IsMotorArrival (golden :3802-3808)");
    check(IsMotorArrival(MOutArmX, 12345) == true, "R2 MOutArmX at 12345, target 12345 => arrived");
    check(IsMotorArrival(MOutArmX, 12845) == false, "R2 MOutArmX at 12345, target 12845 => not arrived (gap 10)");

    std::puts("-- [R3] DoOutArmIonFanGiveWay step 5 while cleaning out (golden :4664-4700)");
    IniConfig.bA15_1ESDGiveWayFunction = true;
    iCleanOut = 1;
    bOutArmIonFanGiveWay = true;
    MOT[MMPlate1].fHasTray = true;
    MOT[MMPlate1].Tray.XItem = 1;
    MOT[MMPlate1].Tray.YItem = 1;
    MOT[MMPlate1].Tray.Data[0][0] = HAS_IC;
    for (int i = 0; i < iTestBinCount; ++i) { Prod.iT6CatData[i] = -1; Prod.AOICatData[i] = -1; }   // no bin routed to an auto tray, so
    Prod.iIfErrorT6 = 99;                             // WhichAutoNeedTray() (asked first in step 5) has nothing to ask for
    iOutArmIonFanGiveWayTask = 5;
    const bool busy = DoOutArmIonFanGiveWay();
    std::snprintf(m, sizeof(m), "R3 hot plate 1 holds an IC => no give-way yet (result %d)", (int)busy);
    check(busy == false, m);
    MOT[MMPlate1].Tray.Data[0][0] = 0;
    iOutArmIonFanGiveWayTask = 5;
    const bool clear = DoOutArmIonFanGiveWay();
    std::snprintf(m, sizeof(m), "R3 both hot plates empty => give way (result %d, flag %d)", (int)clear, (int)bOutArmIonFanGiveWay);
    check(clear == true && bOutArmIonFanGiveWay == true, m);
    IniConfig.bA15_1ESDGiveWayFunction = false;
    iCleanOut = 0;
    bOutArmIonFanGiveWay = false;
    MOT[MMPlate1].fHasTray = false;

    std::puts("-- [R4] CheekNeedToDoOutArmAdditionalFunction: ART RT no-rotate (golden :2164-2165)");
    USE_ROTATE_KIT = 1;
    tRotate.ActiveRotate = 1;
    TrayForm.iRotateKIT_InputType = 1;
    OutArmSuck.bAlreadyRotate = false;
    iOutRotateFinish = 0;
    CosFunction.bART_RT_NoRotate = true;
    tRotate.bART_RT_NoRotate = false;
    bCanRunSCKART = true;
    bOutRotator = false;
    CheekNeedToDoOutArmAdditionalFunction();
    check(bOutRotator == true, "R4 rotate kit active, bART_RT_NoRotate off => the out arm goes to the rotator");
    tRotate.bART_RT_NoRotate = true;
    bOutRotator = false;
    CheekNeedToDoOutArmAdditionalFunction();
    check(bOutRotator == false, "R4 CosFunction + tRotate.bART_RT_NoRotate + bCanRunSCKART => no rotate");

    std::puts("-- [R5] CheekNeedToDoInArmAdditionalFunction: ART RT no-rotate (golden ainarm9045.cpp:3192-3202)");
    TestIF_File.UseRotateForHT7000HPKit = false;
    iInRotateFinish = 1;                              // the IC already sits on the rotator
    LastSet.iRunStartMode = rsmInitial_ART;
    fSCKART->iFTRTCount = 0;
    bInRotator = false;
    CheekNeedToDoInArmAdditionalFunction();
    check(bInRotator == true, "R5 FT pass (iFTRTCount 0) => the in arm still rotates");
    fSCKART->iFTRTCount = 1;
    bInRotator = false;
    CheekNeedToDoInArmAdditionalFunction();
    check(bInRotator == false, "R5 ART start mode + iFTRTCount != 0 + the three no-rotate flags => no rotate");
    fSCKART->iFTRTCount = 0;
    LastSet.iRunStartMode = 0;
    iInRotateFinish = 0;
    USE_ROTATE_KIT = 0;
    tRotate.ActiveRotate = 0;
    tRotate.bART_RT_NoRotate = false;
    CosFunction.bART_RT_NoRotate = false;
    bCanRunSCKART = false;

    std::puts("-- [R6] InspectOutArmPosition (golden :3865-3993, E74 only)");
    SetMyKitSuckItemAmount();                         // wb_serve's boot order, as test_pool2_ainarm / test_w6_2_inarm_canary
    EnsureArmOffsetObjects();
    for (int i = 0; i < eTrayCount; ++i) AutoForm[i] = &TrayForm.Auto[i];   // golden boot (cinitial.cpp, wb_serve.cpp)
    OutArmSuck.Item[0][0] = HAS_IC;
    InputLimit.iOffsetXYHigh = 1;                     // +-100 pulses
    AUTO3_IS_MAGAZINE = 0;
    Prod.XStart[0][iOutArmYBase][iOutArmXBase] = 900000;   // far from the arm's 12345
    Prod.YStart[0][iOutArmYBase][iOutArmXBase] = 900000;
    IniConfig.bE74_InspectArmPosition = false;
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(0, 0, 0, 0, 0, iOutPlaceToAuto);
    check(W906_ShowMyMessage_Count == 0, "R6 E74 off: golden returns before any check (no message)");
    IniConfig.bE74_InspectArmPosition = true;
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(0, 0, 0, 0, 0, iOutPlaceToAuto);
    const std::string msg = W906_ShowMyMessage_LastS1.c_str();
    std::snprintf(m, sizeof(m), "R6 E74 on, out of tolerance: one ShowMyMessage in golden's format (count %d: %.90s)", W906_ShowMyMessage_Count, msg.c_str());
    check(W906_ShowMyMessage_Count == 1 && msg.find("OutArm Suck[0, 0] Pos(") == 0 && msg.find("Place to") != std::string::npos, m);
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(MOutShuttle1, 0, 0, 0, 0, iOutPlaceToAuto + 1);
    check(W906_ShowMyMessage_Count == 1 && std::string(W906_ShowMyMessage_LastS1.c_str()).find("Shuttle 1 [0, 0]") != std::string::npos, "R6 E74 on, not iOutPlaceToAuto: checked again per golden 913 aoutarm9045.cpp:4199 (0618 :3893 `return;` commented out)");   //AI(W906-W188) 20261010 (NB2-1): W-188 #9 -- was `== 0` (0618 skipped every non-auto action)
    OutArmSuck.Item[0][0] = HAS_NULL_IC;
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(0, 0, 0, 0, 0, iOutPlaceToAuto);
    check(W906_ShowMyMessage_Count == 0, "R6 E74 on, the nozzle holds no IC: no check");
    IniConfig.bE74_InspectArmPosition = false;

    std::printf("test_pool2_aoutarm: %d failure(s)\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
