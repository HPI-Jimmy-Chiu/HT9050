// =============================================================================
//  WebReadChainCalls.cpp -- golden TfMain::DoReadLastData／ChangeSetUpFile 裡幾個呼叫點的「接頭」
//
//  AI(W906-RCHG) 20260925（Steven 團隊）：只在 wb_serve 連結。
//  為什麼要另開一個 TU：這幾個表單的標頭不能跟呼叫端放在同一個 TU ——
//    * OmronLaser/LaserSensor.h:174 與 ATC/ATCInterface.h:189 各定義一個 class TTimer（重複定義）；
//      WebRecipeChange.cpp 需要 ATCInterface.h（golden :25527-25528 SendCommToATC7）。
//    * tools/wb_serve.cpp 不 include 這些表單（檔頭已經很長，而且多位工程師同時在改）。
//  每一支都只是 golden 的一行，不加任何邏輯；golden 行號寫在旁邊。
// =============================================================================
#include "OmronLaser/LaserSensor.h"    // fLaserSensor（OmronLaser/LaserSensor.cpp:207）
#include "forms/fSpeed.h"              // fSpeed（cSpeed.cpp:1695）
#include "forms/fTrayAssignment.h"     // fTrayAssignment（forms/fTrayAssignment.cpp；W906_BootReadTrayAssignment 建立）
#include "forms/fSCKART.h"             // fSCKART

// golden TfLaserSensor 建構子 → InitLaserEdtList()（elLaser 由 FileRW_IniConfig_Boot 建好之後；
//   OmronLaser/LaserSensor.cpp 的 GA-3 HAND-OFF 註解：「after GA-3 constructs elLaser it MUST call
//   fLaserSensor->InitLaserEdtList()」—— 移植樹一直沒人叫，elLaser 是空的清單）。只能叫一次（Add 不冪等）。
void W906_RC_LaserInitEdtListOnce()
{
    static bool done = false;
    if (done || fLaserSensor == 0) return;
    fLaserSensor->InitLaserEdtList();
    done = true;
}
void W906_RC_LaserReadFile()           { if (fLaserSensor) fLaserSensor->ReadFile(); }          // golden main.cpp:9334／:9364
void W906_RC_LaserDoIniDataToForm()    { if (fLaserSensor) fLaserSensor->DoIniDataToForm(); }   // golden main.cpp:9393
void W906_RC_LaserReadLaserFile()      { if (fLaserSensor) fLaserSensor->ReadLaserFile(); }     // golden main.cpp:25768（ChangeSetUpFile）
void W906_RC_SpeedDoIniDataToForm()    { if (fSpeed) fSpeed->DoIniDataToForm(); }               // golden main.cpp:9400
void W906_RC_TrayAssignmentDoIniDataToForm() { if (fTrayAssignment) fTrayAssignment->DoIniDataToForm(); }   // golden main.cpp:9402
void W906_RC_SCKARTAccessFileRead()    { if (fSCKART) fSCKART->AccessFile(true); }              // golden main.cpp:9368（移植樹 forms/fSCKART.cpp:69 是空殼）
