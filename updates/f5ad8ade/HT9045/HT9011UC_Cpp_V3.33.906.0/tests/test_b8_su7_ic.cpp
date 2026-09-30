// =============================================================================
//  test_b8_su7_ic.cpp -- test_b8_su7_rtcclick 的「機台裡有沒有料」注入（不是受測碼）
//
//  //AI(W906-B8-SU7) 20260930 [W906] St01 新檔。golden TfMain::CheckCanChangeRealDummy（V912 main.cpp:12895-12901；移植樹 cMainStatus.cpp:323）
//    查六樣：MOT[MMPlate1].HasIC()、MOT[MMPlate2].HasIC()、ShuttleHasIC()、IndexHasIC()、InArmSuck.HasIC()、OutArmSuck.HasIC()。
//    這裡直接改那些活物件的格子，讓受測的 golden 處理器經真的 CheckCanChangeRealDummy 看到「有料」。
//  分成另一個 TU 的原因：aHotPlateSubstrate.h（TMyKitSuck，177 個 TU 用的那一份，CLAUDE.md 陷阱 3）與 FileRW/_EditList.h 帶進來的
//    HTEditList.h 有 TList 重複定義（FileRW/_KitSuck.cpp 同一個理由），主測試檔 include 不了。
//  Shuttle 用 FRCarryKit（OutputShuttleFrontHasIC，csystem.cpp 沒有 Arm2 的條件）；Index 用 TestSocket（TestSocketHasIC，同）。
// =============================================================================
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "csystem.h"
#include "cmydef.h"

namespace {
void KitIC(TMyKitSuck& k, bool on)
{
    if (k.iMaxRow < 1) k.iMaxRow = 1;
    if (k.iMaxCol < 1) k.iMaxCol = 1;
    if (k.iShtRow < 1) k.iShtRow = 1;
    if (k.iShtCol < 1) k.iShtCol = 1;
    k.Item[0][0] = on ? HAS_IC : NULL_IC;
}
void PlateIC(int mot, bool on)
{
    MOT[mot].fHasTray = on;
    if (MOT[mot].Tray.XItem < 1) MOT[mot].Tray.XItem = 1;
    if (MOT[mot].Tray.YItem < 1) MOT[mot].Tray.YItem = 1;
    MOT[mot].Tray.Data[0][0] = on ? HAS_IC : NULL_IC;
}
}  // namespace

// which：0 Plate1、1 Plate2、2 Shuttle、3 Index、4 In Arm、5 Out Arm（golden main.cpp:12897-12899 的順序）
void Su7SetIC(int which, bool on)
{
    switch (which) {
    case 0: PlateIC(MMPlate1, on); break;
    case 1: PlateIC(MMPlate2, on); break;
    case 2: KitIC(FRCarryKit, on); break;
    case 3: KitIC(TestSocket, on); break;
    case 4: KitIC(InArmSuck, on); break;
    case 5: KitIC(OutArmSuck, on); break;
    default: break;
    }
}

// 六個 golden 判斷目前的值（診斷用）
bool Su7Pred(int which)
{
    switch (which) {
    case 0: return MOT[MMPlate1].HasIC();
    case 1: return MOT[MMPlate2].HasIC();
    case 2: return ShuttleHasIC();
    case 3: return IndexHasIC();
    case 4: return InArmSuck.HasIC();
    case 5: return OutArmSuck.HasIC();
    default: return false;
    }
}
