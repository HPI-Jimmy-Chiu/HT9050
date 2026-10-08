// =============================================================================
//  test_pool2_rotatekit.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  POOL-2 RotateKit（FROM_IFOR §1 1008 13:3x）：普查列的 14 個巨集閘（aRotateKIT_In.cpp 6、aRotateKIT_Out.cpp 7、
//  aRotateKIT.cpp 1）在 1008 main 對 golden 0618 看過，理由都過期了，全部照 golden 同一行打開：
//    tRotate.DutNum／RotateKit_PitchX／_PitchY／RotationCount[]／iRotateOffset[]／bPassBinNoRotate／bUseDifferentAngle
//      ——tRotateShim 0926 起是 golden 完整型別 TRotate 的別名（aHotPlateSubstrate.h:868），開機讀 Rotate.Data；
//    FrmRotate->bShowRotateBySite——TFrmRotate 有這個成員（forms/fRotate.h:184），FrmRotate 是靜態 new；
//    ArmCanSuck4IC(...)——csystem.cpp 有照 golden :723 翻好的本體。
//  照留：兩個重複定義（iIn／OutRotateFinish）、SetIn／OutRotateSpeed（TFrmRotate 沒有）、FrmAOI->ttbInsp（沒有）。
//  這支「真的執行」被開的段落，另加原始碼檢查：
//    [S]  原始碼（argv[1] = 移植樹根目錄）：14 個開閘註記、每個下面是 golden 原文的巨集定義
//    [R1] CalcPosition_InArm（golden :718 起）：DutNum＝8 DUT ⇒ :782 那一支的 8 DUT 座標；DutNum＝4 DUT、ArmCanSuck4IC 不成立、
//         RotateKit_PitchX＝40 ⇒ :813-815 的「兩個 Rotate Pitch」座標
//    [R2] CalcPosition_OutArm（golden aRotateKIT_Out.cpp:692 起）：同上（出料臂）
//    [R3] InitSuckState：ArmCanSuck4IC(0) 不成立的 8 站 ⇒ i2x2Suck＝i2x2Suck_Out＝3（golden aRotateKIT.cpp:116 起）
//    [R4] M1_DoInRotateMove／M1_DoOutRotateMove（golden In :3395／Out :3484；一顆馬達、sim 馬達）：目標位置加上 tRotate.iRotateOffset[0]／[1]
//    [R5] CheckRotateOutAnglePostion（golden Out :3925）：bShowRotateBySite ⇒ 用每站的角度表比；bUseDifferentAngle ⇒ 角度差算法
//  只有原始碼檢查的三個：入料 RotationCount[]、入料 bShowRotateBySite、出料 bPassBinNoRotate——都只在
//  M_DoIn／OutArmRotateKIT_Motor 這兩個大狀態機裡用到，要跑得先做旋轉站的測試環境（另一張卡）。
//  反向驗證見 MR 說明：任一個改回 `#if 0` 重編 ⇒ 對應的 [R] 紅＋[S] 紅。
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "mykitsuck.h"
#include "Motor/mymotor.h"
#include "forms/fRotate.h"                // TRotate tRotate, FrmRotate
#include "RotateKit/aRotateKIT.h"
#include "RotateKit/aRotateKIT_In.h"
#include "RotateKit/aRotateKIT_Out.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void CalcPosition_InArm(int &iXpos, int &iYpos, int &iXPitch, int iYPitch, int iKit);    // aRotateKIT_In.cpp (no header)
void CalcPosition_OutArm(int &iXpos, int &iYpos, int &iXPitch, int iYPitch, int iKit);   // aRotateKIT_Out.cpp (no header)
extern int InAngle90;                     // aRotateKIT_In.cpp file scope
extern int OutAngle90;                    // aRotateKIT_Out.cpp file scope

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
static bool DoorClosed() { return false; }   // golden boot hangs IdleCheckSafeDoor on every axis (as test_rotatekit_retry.cpp)

struct Gate { const char* file; const char* tag; const char* next; };
static const Gate kOpened[] = {
    { "aRotateKIT_In.cpp",  "GATE (2) reason expired",     "#define W906RKIN_DUTNUM                 (tRotate.DutNum)" },
    { "aRotateKIT_In.cpp",  "GATE (3)/(4) reason expired", "#define W906RKIN_ROTKIT_PITCHX          (tRotate.RotateKit_PitchX)" },
    { "aRotateKIT_In.cpp",  "GATE (5) reason expired",     "#define W906RKIN_TROT_ROTATIONCOUNT(i)  (tRotate.RotationCount[i])" },
    { "aRotateKIT_In.cpp",  "GATE (6) reason expired",     "#define W906RKIN_TROT_IROTATEOFFSET(i)  (tRotate.iRotateOffset[i])" },
    { "aRotateKIT_In.cpp",  "GATE (7) reason expired",     "#define W906RKIN_SHOWROTATEBYSITE       (FrmRotate->bShowRotateBySite)" },
    { "aRotateKIT_In.cpp",  "GATE (10) reason expired",    "#define W906RKIN_ARMCANSUCK4IC(Direct)  (ArmCanSuck4IC(Direct))" },
    { "aRotateKIT_Out.cpp", "GATE 1 reason expired",       "#define W906RKO_DUTNUM                   (tRotate.DutNum)" },
    { "aRotateKIT_Out.cpp", "GATE 2 reason expired",       "#define W906RKO_TROT_PASSBINNOROTATE     (tRotate.bPassBinNoRotate)" },
    { "aRotateKIT_Out.cpp", "GATE 3 reason expired",       "#define W906RKO_ROTKIT_PITCHX            (tRotate.RotateKit_PitchX)" },
    { "aRotateKIT_Out.cpp", "GATE 4 reason expired",       "#define W906RKO_TROT_USEDIFFERENTANGLE   (tRotate.bUseDifferentAngle)" },
    { "aRotateKIT_Out.cpp", "GATE 5 reason expired",       "#define W906RKO_TROT_IROTATEOFFSET(i)    (tRotate.iRotateOffset[i])" },
    { "aRotateKIT_Out.cpp", "GATE 6 reason expired",       "#define W906RKO_SHOWROTATEBYSITE         (FrmRotate->bShowRotateBySite)" },
    { "aRotateKIT_Out.cpp", "GATE 7 reason expired",       "#define W906RKO_ARMCANSUCK4IC(...)       (ArmCanSuck4IC(__VA_ARGS__))" },
    { "aRotateKIT.cpp",     "GATE (1) reason expired",     "#define W906RK_ARMCANSUCK4IC(Direct)   (ArmCanSuck4IC(Direct))" },
};

static void sources(const std::string& root)
{
    const char* names[3] = { "aRotateKIT_In.cpp", "aRotateKIT_Out.cpp", "aRotateKIT.cpp" };
    const char* envs[3]  = { "W906_POOL2_ROTKIT_IN_SOURCE", "W906_POOL2_ROTKIT_OUT_SOURCE", "W906_POOL2_ROTKIT_SOURCE" };   // reverse checks may point at an opened copy
    std::vector<std::string> L[3];
    for (int f = 0; f < 3; ++f)
    {
        const char* control = std::getenv(envs[f]);
        std::istringstream in(read(control ? control : root + "/RotateKit/" + names[f]));
        std::string line;
        while (std::getline(in, line)) L[f].push_back(line);
        char m[160];
        std::snprintf(m, sizeof(m), "S0 RotateKit/%s read (%u lines)", names[f], (unsigned)L[f].size());
        check(L[f].size() > 200, m);
    }
    for (const Gate& g : kOpened)
    {
        const int f = std::string(g.file) == names[0] ? 0 : std::string(g.file) == names[1] ? 1 : 2;
        int hits = 0, good = 0;
        for (size_t i = 0; i + 1 < L[f].size(); ++i)
            if (trim(L[f][i]).rfind("#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): ", 0) == 0 && L[f][i].find(g.tag) != std::string::npos)
            {
                ++hits;
                if (trim(L[f][i + 1]).rfind(g.next, 0) == 0) ++good;
            }
        char m[200];
        std::snprintf(m, sizeof(m), "S1 %s %s: opened in place, golden expression under it (%d / %d)", g.file, g.tag, hits, good);
        check(hits == 1 && good == 1, m);
    }
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("POOL2_RotateKit"))
        return 2;
    sources(argc > 1 ? argv[1] : "..");
    if (g_fail)
        return 1;                                     // a failed source check stops before any runtime I/O
    char m[220];

    std::puts("-- [R1] CalcPosition_InArm (golden aRotateKIT_In.cpp:718)");
    iRotate_Type = eCynRotate;                        // not the 1-motor / 2-motor layouts, so DutNum decides
    USE_IN_Y_IS_AUTO_PITCH = false;
    iRotato_In_Row = 0;
    Prod.iInArm_RotateX = 100000;
    Prod.iInArm_RotateY = 50000;
    iRotateKIT_Pitch_X_H = 2000;
    iRotateKIT_Pitch_Y_H = 3000;
    iRotateKIT_Start_X_H = 500;
    iRotateKIT_Start_Y_H = 700;
    int x = 0, y = 0, xp = 0;
    tRotate.DutNum = tDutType_8;                      // 8 DUT ladder, IC >= 50 mm on a 2x2: one rotate per IC
    iInArmType = e9045_2x4_4_14;
    TestIF.iTestMode = QualSite2X2;
    DeviceForm.XDimension = 6000;
    CalcPosition_InArm(x, y, xp, 0, 1);
    std::snprintf(m, sizeof(m), "R1 DutNum=8: X = RotateX + P - 2P*kit = 98000 (got %d) (golden `else if(tRotate.DutNum==tDutType_8)`)", x);
    check(x == 98000, m);
    tRotate.DutNum = tDutType_4;                      // 4 DUT, 8-site, the arm cannot suck 4 (1-site recipe) and PitchX 40
    TestIF.iTestMode = _8Site2X4;
    TestIF_File.iTestMode = SingleSite;               // ArmCanSuck4IC(0) -> false (csystem.cpp, golden :735)
    tRotate.RotateKit_PitchX = 40;
    CalcPosition_InArm(x, y, xp, 0, 1);
    std::snprintf(m, sizeof(m), "R1 DutNum=4, ArmCanSuck4IC false, PitchX 40: XPitch 2P, X = RotateX + StartX - 2P/3 = 99167 (got %d / %d)", xp, x);
    check(xp == 4000 && x == 99167, m);

    std::puts("-- [R2] CalcPosition_OutArm (golden aRotateKIT_Out.cpp:692)");
    USE_OUT_Y_IS_AUTO_PITCH = false;
    iRotato_Out_Row = 0;
    Prod.iOutArm_RotateX = 200000;
    Prod.iOutArm_RotateY = 60000;
    tRotate.DutNum = tDutType_8;
    TestIF.iTestMode = QualSite2X2;
    CalcPosition_OutArm(x, y, xp, 0, 1);
    std::snprintf(m, sizeof(m), "R2 DutNum=8: X = RotateX + P - 2P*kit = 198000 (got %d)", x);
    check(x == 198000, m);
    tRotate.DutNum = tDutType_4;
    TestIF.iTestMode = _8Site2X4;
    CalcPosition_OutArm(x, y, xp, 0, 1);
    std::snprintf(m, sizeof(m), "R2 DutNum=4, ArmCanSuck4IC false, PitchX 40: XPitch 2P, X = RotateX + StartX - 2P/3 = 199167 (got %d / %d)", xp, x);
    check(xp == 4000 && x == 199167, m);

    std::puts("-- [R3] InitSuckState (golden aRotateKIT.cpp:116)");
    i2x2Suck = i2x2Suck_Out = 0;
    TestIF.iTestMode = _8Site2X4;
    TestIF_File.iTestMode = SingleSite;
    InitSuckState();
    std::snprintf(m, sizeof(m), "R3 8-site, ArmCanSuck4IC(0) false: i2x2Suck = i2x2Suck_Out = 3 (got %d / %d)", i2x2Suck, i2x2Suck_Out);
    check(i2x2Suck == 3 && i2x2Suck_Out == 3, m);
    tRotate.RotateKit_PitchX = 0;
    TestIF.iTestMode = TestIF_File.iTestMode = 0;

    std::puts("-- [R4] M1_DoInRotateMove / M1_DoOutRotateMove: one-motor rotate offset");
    W906_TestEnsureSimMotors();
    MOT[MInRotateKit].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
    MOT[MOutRotateKit].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
    iRotate_Type = eInOutArm1Motor;
    Prod.iIn_iRotateA_Backlash = 0;
    Prod.iOut_iRotateA_Backlash = 0;
    InAngle90 = 1000;
    OutAngle90 = 1000;
    Prod.iIn_iRotateA = 0;
    Prod.iOut_iRotateA = 0;
    Prod.RotationCount[0] = 90;
    Prod.OutRotationCount[0] = 90;
    tRotate.iRotateOffset[0] = 37;
    tRotate.iRotateOffset[1] = -41;
    MOT[MInRotateKit].Motor->SetPosition(0);
    MOT[MOutRotateKit].Motor->SetPosition(0);
    int rc = 1, r = 0;
    for (int i = 0; i < 5 && r != 1; ++i) r = M1_DoInRotateMove(rc, false);
    std::snprintf(m, sizeof(m), "R4 in rotate 90 deg: target = 1000 + iRotateOffset[0] 37 = 1037 (result %d, pos %d)", r, MOT[MInRotateKit].ReadPos());
    check(r == 1 && MOT[MInRotateKit].ReadPos() == 1037, m);
    r = 0;
    for (int i = 0; i < 5 && r != 1; ++i) r = M1_DoOutRotateMove(rc, false);
    std::snprintf(m, sizeof(m), "R4 out rotate 90 deg: target = 1000 + iRotateOffset[1] -41 = 959 (result %d, pos %d)", r, MOT[MOutRotateKit].ReadPos());
    check(r == 1 && MOT[MOutRotateKit].ReadPos() == 959, m);
    tRotate.iRotateOffset[0] = tRotate.iRotateOffset[1] = 0;
    iRotate_Type = eCynRotate;

    std::puts("-- [R5] CheckRotateOutAnglePostion (golden aRotateKIT_Out.cpp:3925)");
    FrmRotate->bShowRotateBySite = true;              // per-site: the target angle table vs the motor's current per-site angle
    Prod.RotateDutDate[1][0][1] = 90;
    MOT[MOutRotateKit].Tray.iCurrRotAng[1][0] = 90;
    Prod.RotateDutDate[0][0][1] = 0;
    check(CheckRotateOutAnglePostion(0, 1, 5, 1) == true, "R5 bShowRotateBySite: RotateDutDate[1][r][c] == iCurrRotAng[c][r] => true");
    FrmRotate->bShowRotateBySite = false;
    tRotate.bUseDifferentAngle = true;                // 360 - in + out - 360 = out - in
    Prod.RotateDutDate[0][0][1] = 90;
    Prod.RotateDutDate[1][0][1] = 180;
    check(CheckRotateOutAnglePostion(0, 1, 90, 1) == true, "R5 bUseDifferentAngle: angle = out - in = 90 => matches 90");
    tRotate.bUseDifferentAngle = false;
    check(CheckRotateOutAnglePostion(0, 1, 90, 1) == false, "R5 bUseDifferentAngle off: angle = 0 - in = -90 => does not match 90");

    std::printf("test_pool2_rotatekit: %d failure(s)\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
