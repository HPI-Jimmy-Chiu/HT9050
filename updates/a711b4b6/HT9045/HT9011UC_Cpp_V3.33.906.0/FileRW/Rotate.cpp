// ===========================================================================
//  FileRW/Rotate.cpp -- golden TFrmRotate（RotateKit\fRotate.cpp，V912）的 C 路入口：<配方>\Rotate.Data 的讀寫段。
//  沒有頁面（Steven 20260926：先把讀寫檔移植完，頁面不做）。
//
//  //AI(W906-FRW-NoPage) 20260926: 新檔（Steven 團隊）。設定：tools/editlist/Rotate.py（每條 replace／block 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 Rotate.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:42）             ＝ FileRW_Rotate_Boot()：CreateForm（HT9045.cpp:227）。bShowRotateBySite（USE_ROTATE_KIT==1 且
//                                 iRotate_Type 是 e4MotRotate／e8MotRotate／e2MotRotate2Dut／e1MotRotate1Dut）、Rotate_Offset[]／Pre_Rotate[]、
//                                 iRotateDutDate 清零、iFromTrayAngle=0。要在 LoadMachineConfig 之後（USE_ROTATE_KIT、iRotate_Type、CosFunction）。
//    fRotate_ReadFile（:241）   ＝ FileRW_Rotate_ReadFile()：Rotate.Data [SETTING] → 全域 tRotate（forms/fRotate.h）＋ strIn/OutRotateDutAngle
//                                 （cmydef.h:4595-4596；SECS EC 16148/16149 指向這兩個字串，SECSGEM/uHGemHT9045_EC.cpp:1705-1706）。
//                                 只用 ReadIniData —— 不補寫缺鍵，不改任何檔。
//    DoIniDataToForm（:586）    ＝ FileRW_Rotate_DoIniDataToForm()：tRotate → 替身＋FrmRotate->iFromTrayAngle／iRotateDutDate；
//                                 結尾 golden 自己呼叫 SetWorkParameter()（cinitial.cpp:7084；ChangeSite／ReadTechData／DoStructUnitConvert…，
//                                 沒有馬達動作）。
//    spbSaveClick（:1036）      ＝ FileRW_Rotate_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                 沒有登記 PageDesc）。兩段 GATE（ART RT 格子檢查、ColCount／RowCount 兩鍵）要等頁面接上
//                                 FormShow＋SetGridDraw 才有格子大小，見 tools/editlist/Rotate.py 的 BLOCKS。
//
//  ---- golden 讀檔時機（本檔提供函式，呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）-----------------------
//    開機：CreateForm(TFrmRotate)（HT9045.cpp:227）→ FileRW_Rotate_Boot()；
//          TfMain::FormShow :9993 DoReadLastData → :9361 FrmRotate->fRotate_ReadFile() → :9407 FrmRotate->DoIniDataToForm()。
//    換配方：TfMain::ChangeSetUpFile :25722／:25770 兩次 DoReadLastData（同上兩支）。移植樹兩者都走 W906_DoReadLastData。
//    開頁：FormShow :131-132（沒有頁面，不接）。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    tRotate：全樹一份（aHotPlateSubstrate.cpp:1077，型別 forms/fRotate.h 的 TRotate）。
//    FrmRotate 的 bShowRotateBySite／bIsAngleZero／iRotateDutDate／iFromTrayAngle：門面 forms/fRotate.h（golden 表單外有讀者，
//    見那裡的註解）；Rotate.gen.inc 用 #define 接過去 —— ⚠ gen.inc 之後這四個名字是巨集，本檔直接寫名字，不要寫 FrmRotate->…。
//    Rotate_Offset[2]／Pre_Rotate[2]／IsRotateFlowNotFinish：golden fRotate.cpp 檔案層級，只有本表單用 → gen.inc 的 static。
// ===========================================================================
#include "FileRW/Rotate.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TFrmRotate 建構（HT9045.cpp:227 CreateForm）：DFM 設計期狀態 → 建構子（:42）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// 不讀檔、不寫檔。
void FileRW_Rotate_Boot()
{
    if (g_booted) return;
    RT_DfmItems();
    RT_DfmState();
    RT_TFrmRotate();
    RT_CreateSaveProxies();
    RT_CreateContainerProxies();
    std::printf("FileRW Rotate: TFrmRotate proxies ready (%d save reads) -- golden RotateKit/fRotate.cpp ctor :42: "
                "USE_ROTATE_KIT=%d iRotate_Type=%d bShowRotateBySite=%d\n",
                (int)(sizeof(kRT_SaveReads) / sizeof(kRT_SaveReads[0])), USE_ROTATE_KIT, iRotate_Type, (int)bShowRotateBySite);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9361 `FrmRotate->fRotate_ReadFile();`（開機與換配方）。只讀。
void FileRW_Rotate_ReadFile()
{
    if (!g_booted) FileRW_Rotate_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    RT_fRotate_ReadFile();
    std::printf("rotate.* chain loaded: Rotate.Data [SETTING] -> tRotate (ActiveRotate=%d DutNum=%d FromTrayAngle=%.1f "
                "Col=%d Row=%d TimeIn=%d TimeOut=%d AngleZero=%d) DutAngle=\"%s\" DutAngleOut=\"%s\"\n",
                (int)tRotate.ActiveRotate, tRotate.DutNum, tRotate.FromTrayAngle, tRotate.ColCount, tRotate.RowCount,
                tRotate.RotationTimeIn, tRotate.RotationTimeOut, (int)bIsAngleZero,
                strInRotateDutAngle.c_str(), strOutRotateDutAngle.c_str());
}

// golden TfMain::DoReadLastData main.cpp:9407 `FrmRotate->DoIniDataToForm();`（開機與換配方）。
// ⚠ golden 本體最後一行是 SetWorkParameter() —— 開機／換配方時 golden 會在 DoReadLastData 中途多跑一次（見交件報告決策題）。
void FileRW_Rotate_DoIniDataToForm()
{
    if (!g_booted) FileRW_Rotate_Boot();
    RT_DoIniDataToForm();
}

// golden 存檔鈕 TFrmRotate::spbSaveClick（:1036）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// 會寫 <DataPath>\<配方>\Rotate.Data，最後 golden fMain->BackupSetupFile()。
void FileRW_Rotate_spbSaveClick()
{
    if (!g_booted) FileRW_Rotate_Boot();
    RT_spbSaveClick();
}
