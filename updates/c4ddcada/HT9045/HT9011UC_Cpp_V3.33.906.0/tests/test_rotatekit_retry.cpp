// ===========================================================================
//  tests/test_rotatekit_retry.cpp
//
//  AI(W906-I04) 20261001 (Ifor01): RotateKit 取料失敗重試卡死（docs/GOLDEN_DEFECT_LEDGER.md 的 RotateKit 那一筆）。
//  golden 906 的重試 case 4700 先 Set*ArmHome() 再直接回 case 4000：歸零後 Z 停在原點（0），不是安全高度，
//  case 4000 移 XY 之前的 Z 檢查（CheckInArmZ／CheckOutArmZ：Z 不剛好在 Prod.Z*ArmSafe 就成立）永遠成立——
//  入料臂一直歸零 Z、點動 pitch，出料臂綠燈亮著不動也不報警，都要按 HOME 才停（Ifor 0922 客戶現場）。
//  移植樹照 Ifor 0922 在 V912 的修法改（改得跟 golden 不一樣，RULINGS_20261001 第 15 條，Jimmy 1001 13:2x）：
//  4700 之後先進 case 4710，入料臂 MoveInArmZToPlateSafe(Task)、出料臂 MoveOutArmToAutoSafe() 把 Z 移到安全高度，
//  到了才回 4000（RotateKit/aRotateKIT_In.cpp:3694-3695、aRotateKIT_Out.cpp:3809-3810）。
//
//    [1] 入料臂：一顆 Z（MInArmZA）停在 0、iInArmRotateKit=4700 ⇒ 一拍後是 4710（golden 是 4000），Z 還在 0，
//        CheckInArmZ() 仍成立；接下來幾拍（MotorMove 先下命令、下一拍確認到位）⇒ Z 到 Prod.ZInArmSafe 才回 4000，
//        CheckInArmZ() 不再成立
//    [2] 出料臂：同上，MOutArmZA、iOutArmRotateKit、Prod.ZOutArmSafe、CheckOutArmZ(false)
//    [3] 同一顆解開入料臂的 GATE (9)（aRotateKIT_In.cpp:376；CheckInArmZ 現在有真本體 ainarm2.cpp:734）：Z 不在
//        Prod.ZInArmSafe 時 M_MoveInArmXY_ToRotateKIT 照 golden 拒移、要求歸零 Z（閘成 false 時會照移 XY）
//  模擬馬達：tests/w906_test_motors.h（TMySimMotor，移動瞬間到位）；兩顆 Z 軸換成本檔的 TZAxisSimMotor（安全高度時歸零
//  感測器亮，golden InArmZSafe／OutArmZSafe 靠這個判安全），並照 golden 開機掛上安全門回呼（門關著）。只動記憶體；
//  log 由 ctest 的 ENV-ALL 導到 build dir。
// ===========================================================================
#include <cstdio>
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "mykitsuck.h"              // InArmSuck / OutArmSuck (TMyKitSuck)
#include "Motor/mymotor.h"          // MOT[]
#include "RotateKit/aRotateKIT.h"   // iInArmRotateKit / iOutArmRotateKit
#include "RotateKit/aRotateKIT_In.h"
#include "RotateKit/aRotateKIT_Out.h"
#include "w906_test_motors.h"

bool CheckInArmZ();                 // ainarm2.cpp:734
bool CheckOutArmZ(bool bMessage);   // aoutarm.cpp:653
bool M_MoveInArmXY_ToRotateKIT(int iKit);   // aRotateKIT_In.cpp:1274 (the header declares a zero-arg one -- golden defect, ledger)

// Golden boot hangs IdleCheckSafeDoor on every axis (cinitial.cpp:4557). Without a callback an Enable'd axis reads "door
// open" (HTMotor.cpp:113-126) and MotorMove returns -1 without moving -- same fixture as test_gali_route_engine.cpp:124.
static bool DoorClosed() { return false; }

// The Z axes' driver. golden InArmZSafe / OutArmZSafe (Motor/mymotor.cpp:6079) call a nozzle Z safe only while its home
// sensor reads ON -- on the machine the safe height sits inside the home sensor's window. TMySimMotor::ScanMotorStatus
// always reports the home sensor OFF (Motor/mySimMotor.cpp:172), so with it MoveInArmZToPlateSafe can never succeed;
// this driver reports it ON and is otherwise the stock sim motor (position moves instantly, in position, no alarm).
class TZAxisSimMotor : public TMySimMotor
{
public:
    virtual void ScanMotorStatus(bool *Led) { TMySimMotor::ScanMotorStatus(Led); if (Led != 0) Led[iHomeLed] = true; }
};

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_rotatekit_retry.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);                             // unbuffered: with the fix undone, [3] runs the XY mover
                                                                    // into objects this test never builds and crashes -- the
                                                                    // FAIL lines of [1] / [2] must still reach the log
    MOT[MInArmZA].Motor = new TZAxisSimMotor();                     // before the fixture, which only fills NULL axes
    MOT[MOutArmZA].Motor = new TZAxisSimMotor();
    W906_TestEnsureSimMotors();
    MOT[MInArmZA].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
    MOT[MOutArmZA].Motor->MotorIdleSafeDoorCheck = &DoorClosed;

    std::printf("[1] In arm: 4700 -> 4710 (Z to Prod.ZInArmSafe) -> 4000\n");
    InArmSuck.iMotRow = 1;
    InArmSuck.iMotCol = 1;
    InArmSuck.Suck[0][0].iMotNo = MInArmZA;
    Prod.ZInArmSafe[0][0] = 1500;
    MOT[MInArmZA].Motor->SetPosition(0);                                   // where Set*ArmHome() leaves Z
    CHECK(MOT[MInArmZA].ReadPos() == 0);
    iInArmRotateKit = 4700;
    M_DoInArmRotateKIT_Motor();
    CHECK(iInArmRotateKit == 4710);                                 // golden: 4000
    CHECK(MOT[MInArmZA].ReadPos() == 0);
    CHECK(CheckInArmZ() == true);                                   // the guard golden loops on
    for (int i = 0; i < 5 && iInArmRotateKit == 4710; ++i)
        M_DoInArmRotateKIT_Motor();
    CHECK(iInArmRotateKit == 4000);
    CHECK(MOT[MInArmZA].ReadPos() == 1500);
    CHECK(CheckInArmZ() == false);                                  // case 4000 can move XY now

    std::printf("[2] Out arm: 4700 -> 4710 (Z to Prod.ZOutArmSafe) -> 4000\n");
    OutArmSuck.iMotRow = 1;
    OutArmSuck.iMotCol = 1;
    OutArmSuck.iPickRow = 1;
    OutArmSuck.iPickCol = 1;
    OutArmSuck.Suck[0][0].iMotNo = MOutArmZA;
    Prod.ZOutArmSafe[0][0] = 1700;
    MOT[MOutArmZA].Motor->SetPosition(0);
    CHECK(MOT[MOutArmZA].ReadPos() == 0);
    iOutArmRotateKit = 4700;
    M_DoOutArmRotateKIT_Motor();
    CHECK(iOutArmRotateKit == 4710);                                // golden: 4000
    CHECK(MOT[MOutArmZA].ReadPos() == 0);
    CHECK(CheckOutArmZ(false) == true);
    for (int i = 0; i < 5 && iOutArmRotateKit == 4710; ++i)
        M_DoOutArmRotateKIT_Motor();
    CHECK(iOutArmRotateKit == 4000);
    CHECK(MOT[MOutArmZA].ReadPos() == 1700);
    CHECK(CheckOutArmZ(false) == false);

    std::printf("[3] GATE (9) lifted: the in-arm XY mover refuses while Z is not at Prod.ZInArmSafe (golden :878)\n");
    MOT[MInArmZA].Motor->SetPosition(0);
    bNeedArmZHome = false;
    CHECK(M_MoveInArmXY_ToRotateKIT(0) == false);                  // golden: CheckInArmZ() -> SetInArmHome(), no XY move
    CHECK(bNeedArmZHome == true);

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
