// =============================================================================
//  tests/w906_test_motors.h -- 測試夾具：讓 MOT[].Motor 滿足 golden 的開機不變式
//
//  AI(W906-T6-MAINPROC) 20260923: 新檔。
//
//  golden 的 InitialMotorParameter()（cinitial.cpp:3739 起）在開機時替每一軸
//  `MOT[i].Motor = new TMy<brand>Motor(...)`，之後 MOT[].Motor 永不為 NULL ——
//  MainProc -> DoSystem -> ScanAllMotorStatus（golden csystem.cpp，移植樹 csystem.cpp
//  `void ScanAllMotorStatus(int)`）每一拍都直接讀 `MOT[i].Motor->Enable`，不檢查 NULL。
//
//  直接呼叫 MainProc() 的測試（mainproc_guard / W6_6_Hub / W6_6_CSystemCycle / WB_SimPump）
//  刻意不跑 LoadMachineConfig()／InitialMotorParameter()（那會寫真的 system\Gerneral.ini），
//  所以 MOT[].Motor 是 NULL。T6 之前 MainProc 是骨架、不呼叫 DoSystem，所以沒事；
//  T6 照 golden 翻完之後，這四支在第一拍就 SIGSEGV（20260923 gdb：
//  ScanAllMotorStatus+1581 `movzbl 0x5d(%eax)`，eax=0 ＝ MOT[i].Motor->Enable）。
//
//  修法放在測試夾具、不放在產品碼：產品碼照 golden 不檢查 NULL 是對的（開機後必非 NULL），
//  缺的是測試沒有滿足那個不變式。這裡用 TMySimMotor（test_motor_w4.cpp 已在用的替身，
//  ctor 設 Enable=true、其餘欄位是 HTMotor 預設值），只補 NULL 的那幾軸，不覆蓋既有的。
//  不讀檔、不寫檔、不碰硬體。
// =============================================================================
#ifndef W906_TEST_MOTORS_H
#define W906_TEST_MOTORS_H

#include "Motor/mymotor.h"     // MOT[] / TOTAL_MOTOR
#include "Motor/mySimMotor.h"  // TMySimMotor
#include "vclcompat/TList.h"   // AI(W906-INDEXZ-1203) 20260930: vclcompat::TList (pSuck, below)

// AI(W906-INDEXZ-1203) 20260930: golden boot invariant #2 -- pSuck (the sucker list, mykitsuck.cpp:221) is created by
//   InitSucker (cinitial.cpp:447-450, called from InitHontechHardware at boot; wb_serve runs it through InitialHandler) and
//   is never NULL afterwards. Since GATE G22 is lifted to golden (csystem.cpp, golden :4657-4672), DoSystem calls
//   ClearAllManualSuckTask() (mykitsuck.cpp:2701, `pSuck->Count`, no NULL check -- golden's) on every pass with
//   SystemStart; the four tests above that drive MainProc with SystemStart and no InitSucker then SIGSEGV'd there
//   (gdb 20260930: ClearAllManualSuckTask mykitsuck.cpp:2703 <- DoSystem csystem.cpp:16902 <- MainProc, W6_6_Hub, SIM
//   build). Same rule as the motors above: the fixture meets the invariant, the product code stays golden. An EMPTY list
//   (InitSucker would also fill it from the machine config, which these tests deliberately do not load).
extern vclcompat::TList *pSuck;
inline int W906_TestEnsureSuckList()
{
    if (pSuck != 0) return 0;
    pSuck = new vclcompat::TList;
    return 1;
}

inline int W906_TestEnsureSimMotors()
{
    W906_TestEnsureSuckList();   // AI(W906-INDEXZ-1203) 20260930: every caller of this fixture drives (or may drive) MainProc -- see above
    int made = 0;
    for (int i = 0; i < TOTAL_MOTOR; i++)
    {
        if (MOT[i].Motor == 0)
        {
            MOT[i].Motor = new TMySimMotor();
            made++;
        }
    }
    return made;
}

#endif // W906_TEST_MOTORS_H
