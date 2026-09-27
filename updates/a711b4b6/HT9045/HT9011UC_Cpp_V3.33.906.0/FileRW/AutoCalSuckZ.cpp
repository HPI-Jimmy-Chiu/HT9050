// ===========================================================================
//  FileRW/AutoCalSuckZ.cpp -- golden TfProductionInfo（ProductionInfo\ProductionInfo.cpp，V912）「自動校正吸嘴 Z 高度」
//  的 C 路入口：D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data [ArmSuckZAuto] 的讀寫段。沒有頁面
//  （Steven 20260926 S52：先把讀寫檔移植完；web/page/ 沒有 tsCalibrateSuckZHeight 這一頁，20260926 grep）。
//
//  //AI(W906-FRW-S70) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S70）。設定：tools/editlist/AutoCalSuckZ.py（每條 block／replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 AutoCalSuckZ.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:52）                 ＝ FileRW_AutoCalSuckZ_Boot()：CreateForm(TfProductionInfo)（HT9045.cpp:257）。ZeroMemory 兩個
//                                     高度差陣列、兩個 Enable=false、elData=new HTEditList、InitAutoCalSuckZ（:6019，22 筆 Add）、
//                                     兩個 SearchStartZ=0。不讀檔、不寫檔。OEE／IPSC 兩段擋掉（N14 超豐客戶功能）。
//    DoIniDataToForm（:83）        ＝ FileRW_AutoCalSuckZ_DoIniDataToForm()：golden TfMain::DoReadLastData main.cpp:9430。
//                                     ⚠ golden 第一行 `if(CosFunction.bOEEFunction==false) return;` 照留 —— 只有 OEE 客戶
//                                     （CosFunction.cpp:1434）才會跑到結尾的 elData->InitialDataToEdit()＋LoadAutoCalSuckZ()
//                                     （:6063，ReadEditTextFromFile；MyForceDirectories 會先建 D:\HT9045\system\ProductionInfo 資料夾）。
//                                     中間的 OEE 本體擋掉。
//    SaveAutoCalSuckZ（:6055）     ＝ FileRW_AutoCalSuckZ_SaveAutoCalSuckZ()：elData->SaveEditTextToFile。⚠ 目前沒有呼叫者。
//                                     golden 的兩個呼叫者在 V912 也都到不了（見 tools/editlist/AutoCalSuckZ.py 檔頭）：
//                                     btnAutoCalSuckZSaveClick 所在分頁沒有開頁者；ainarm9045.cpp:9229 DoInArmAutoCalSuckZ（機台量測，
//                                     golden 0 個呼叫者，移植樹 ainarm9045.cpp:3105 本體 GATE，屬 Jimmy）。
//    btnAutoCalSuckZSaveClick（:6135）＝ FileRW_AutoCalSuckZ_btnAutoCalSuckZSaveClick()：存 → 重讀 → Close（記 closed）。給之後的頁面用。
//
//  ---- golden 讀檔時機（本檔提供函式，呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）-----------------------
//    開機：CreateForm(TfProductionInfo)（HT9045.cpp:257）→ FileRW_AutoCalSuckZ_Boot()；
//          TfMain::FormShow :9993 DoReadLastData → :9430 fProductionInfo->DoIniDataToForm() → FileRW_AutoCalSuckZ_DoIniDataToForm()。
//    換配方：TfMain::ChangeSetUpFile :25722／:25770 兩次 DoReadLastData（同上）。移植樹兩者都走 W906_DoReadLastData。
//    AutoCalSuckZ.Data 不跟配方（golden 寫死 GetProInfoFilePath()），但 golden 換配方照樣重讀一次 —— 照做。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    8 個值是 cmydef.h 的全域（golden cmydef.cpp:5914-5921）：
//      * bEnableIn/OutarmSuckZAuto、iIn/OutArmZHeightDiff、iIn/OutArmSearchStartZ —— 定義在 cmydef.cpp（本次從 TODO(W6) GATE 解出來，
//        ht9045_globals，所有目標都看得到；讀者 forms/fProductionInfo.cpp EnableIn/OutArmAutoCalSuckZ、cinitial.cpp GATE n5-G17／G18）。
//      * In/OutArmAutoCalSuckZPoint（uPoint2D）—— 定義在**本檔**（下面），不在 cmydef.cpp：uPoint2D 的建構子本體在
//        Public/HTEditList.cpp（ht9045_sm），而 cmydef.cpp 在 ht9045_globals（只連 vclcompat）；在 cmydef.cpp 定義會讓
//        每個抽到 cmydef.o 卻沒連 ht9045_sm 的程式多一個 undefined reference to uPoint2D::uPoint2D()。目前全樹只有本 TU 讀寫它們；
//        之後 Jimmy 解 DoInArmAutoCalSuckZ（ht9045_sm，golden :9121 讀 InArmAutoCalSuckZPoint）時，把這兩行搬到 Public/HTEditList.cpp
//        （與 uPoint2D 本體同一個目標）。
//    elData（golden 表單成員）：gen.inc 的 static，只有本 TU 用。
// ===========================================================================
#include "FileRW/AutoCalSuckZ.gen.inc"

#include <cstdio>

// golden cmydef.cpp:5918-5919（cmydef.h:5813-5814 extern）。放這裡的原因見檔頭「狀態放哪」。
uPoint2D InArmAutoCalSuckZPoint;                                                //Jimmychiu 20240712 : Auto Calibrate Suck Z height
uPoint2D OutArmAutoCalSuckZPoint;

namespace {
bool g_booted = false;
}  // namespace

// golden TfProductionInfo 建構（HT9045.cpp:257 CreateForm）的 AutoCalSuckZ 那一段：DFM 設計期狀態 → 建構子（:52）→ 容器替身與父子。
// 冪等。不讀檔、不寫檔。elData 22 筆的 (區段,鍵) 全在 [ArmSuckZAuto]，與其他 HTEditList 沒有共用鍵，開機順序不影響註冊。
void FileRW_AutoCalSuckZ_Boot()
{
    if (g_booted) return;
    ACZ_DfmItems();
    ACZ_DfmState();
    ACZ_TfProductionInfo();
    ACZ_CreateSaveProxies();
    ACZ_CreateContainerProxies();
    std::printf("FileRW AutoCalSuckZ: TfProductionInfo elData registered by name (%d entries) -- golden ProductionInfo.cpp ctor :52 "
                "+ InitAutoCalSuckZ :6019\n", elData->FEditList->Count);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9430 `fProductionInfo->DoIniDataToForm();`（開機與換配方）。
// 只有 CosFunction.bOEEFunction 時才讀 AutoCalSuckZ.Data（golden 第一行 return）。
void FileRW_AutoCalSuckZ_DoIniDataToForm()
{
    if (!g_booted) FileRW_AutoCalSuckZ_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    ACZ_DoIniDataToForm();
    std::printf("autocalsuckz.* chain: bOEEFunction=%d -> %s (InArm Enable=%d Diff00=%d StartZ=%d Pt=%d,%d; "
                "OutArm Enable=%d Diff00=%d StartZ=%d Pt=%d,%d)\n",
                (int)CosFunction.bOEEFunction,
                CosFunction.bOEEFunction ? "D:\\HT9045\\system\\ProductionInfo\\AutoCalSuckZ.Data [ArmSuckZAuto] read"
                                         : "not read (golden DoIniDataToForm returns first when !bOEEFunction)",
                (int)bEnableInarmSuckZAuto, iInArmZHeightDiff[0][0], iInArmSearchStartZ,
                InArmAutoCalSuckZPoint.X, InArmAutoCalSuckZPoint.Y,
                (int)bEnableOutarmSuckZAuto, iOutArmZHeightDiff[0][0], iOutArmSearchStartZ,
                OutArmAutoCalSuckZPoint.X, OutArmAutoCalSuckZPoint.Y);
}

// golden TfProductionInfo::SaveAutoCalSuckZ（:6055）：寫 D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data。
// 目前沒有呼叫者（golden 的呼叫者 DoInArmAutoCalSuckZ 是機台量測流程，屬 Jimmy）。
// ⚠ 存的是替身的 Text／Checked（golden SaveEditTextToFile 讀元件，不讀全域）：golden DoInArmAutoCalSuckZ :9226 先寫
//   edZ[irow][icol]->Text=IntToStr(iInArmZHeightDiff[irow][icol]) 再呼叫本函式 —— 移植時 edZ 要指到
//   EL<TEdit>("TfProductionInfo", "setEditZ1A".."setEditZ1H")（名稱對照見 gen.inc 的 ACZ_InitAutoCalSuckZ）。
void FileRW_AutoCalSuckZ_SaveAutoCalSuckZ()
{
    if (!g_booted) FileRW_AutoCalSuckZ_Boot();
    ACZ_SaveAutoCalSuckZ();
}

// golden 存檔鈕 TfProductionInfo::btnAutoCalSuckZSaveClick（:6135）：SaveAutoCalSuckZ → LoadAutoCalSuckZ → Close。
// 目前沒有呼叫者（沒有頁面、沒有 WS 指令、沒有登記 PageDesc）；給之後的頁面用。
void FileRW_AutoCalSuckZ_btnAutoCalSuckZSaveClick()
{
    if (!g_booted) FileRW_AutoCalSuckZ_Boot();
    ACZ_btnAutoCalSuckZSaveClick();
}
