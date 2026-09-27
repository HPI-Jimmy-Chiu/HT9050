// ===========================================================================
//  FileRW/CfgTrayPlate.h -- golden TfConfiguration 的 Tray／Plate 資料表（tsTrayData／tsHPData 兩個分頁）：
//    D:\HT9045\System\TrayForm.csv（TrayTablePath）、D:\HT9045\System\PlateForm.csv（PlateTablePath）。
//
//  //AI(W906-FRW-S98) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S98）。本體與接線說明見 FileRW/CfgTrayPlate.cpp 檔頭。
//
//  golden 的讀者都寫成 `fConfiguration->sbtReloadTray->Click();` 再讀 `fConfiguration->strngrdTray->Cells[c][r]`
//  （cTrayForm.cpp:164-175／:583-600、uCleaning.cpp:1488-1499／:2326-2343、cHotPlate.cpp:47／:424／:747、
//  cConfiguration.cpp:226-234／:4676-4684）。移植樹唯一叫 fConfiguration 的全域是 SCK_ART 的替身
//  （Automation/SCK_ART_Remainder.cpp:352，型別 W5SckArtRem_ConfigStub），所以 C 路的讀者在自己的 TU 用
//    #define fConfiguration (W906_CfgTrayPlate())
//  接到這裡（tools/editlist/UserDefForm_File.py、TestIF_File_Cleaning.py 的 members）。
// ===========================================================================
#pragma once

#include "vclcompat/vcl_compat.h"   // AnsiString、TStringList
#include "vclcompat/Controls.h"     // TSpeedButton
#include "vclcompat/StringGrid.h"   // vclcompat::TStringGrid（Cells／RowCount／ColCount／Font／DefaultColWidth／ColWidths／FixedRows／FixedCols）

// golden TfConfiguration（cConfiguration.h，V912）裡 Tray／Plate 兩個分頁用到的元件與四個按鈕處理器。
// 只有這一小塊：TfConfiguration 其餘的部分在 C 路是 FileRW/IniConfig.cpp（具名替身 "TfConfiguration"），
// 移植樹門面 forms/fConfiguration.h 的 TfConfiguration 沒有執行期實例（見那裡的 INTEGRATION STATUS）。
class TfConfigurationTrayPlate
{
public:
    TfConfigurationTrayPlate();

    vclcompat::TStringGrid *strngrdTray;   // golden cConfiguration.h:1040（dfm:21366，沒有設 ColCount／RowCount → VCL 預設 5x5）
    vclcompat::TStringGrid *strngrdHP;     // golden cConfiguration.h:1054（dfm:22637，同上）
    // 四顆鈕：Click() 照 VCL 呼叫 DFM 的 OnClick（golden cConfiguration.dfm:21870／:21986／:22518／:22634）。
    // vclcompat 的 TControl::Click() 是空的，這四顆是本檔的子類別（見 .cpp）。
    TSpeedButton *sbUpdateTray;            // golden cConfiguration.h:1045  OnClick = sbUpdateTrayClick
    TSpeedButton *sbtReloadTray;           // golden cConfiguration.h:1046  OnClick = sbtReloadTrayClick
    TSpeedButton *sbUpdateHP;              // golden cConfiguration.h:1052  OnClick = sbUpdateHPClick
    TSpeedButton *sbtReloadHP;             // golden cConfiguration.h:1053  OnClick = sbtReloadHPClick

    void sbUpdateTrayClick();              // golden cConfiguration.cpp:7012（寫 TrayForm.csv）
    void sbtReloadTrayClick();             // golden cConfiguration.cpp:7037（讀 TrayForm.csv）
    void sbtReloadHPClick();               // golden cConfiguration.cpp:7148（讀 PlateForm.csv）
    void sbUpdateHPClick();                // golden cConfiguration.cpp:7194（寫 PlateForm.csv）
};

// 唯一的一份（golden fConfiguration 的這一塊）。第一次呼叫時建立，不讀檔。
TfConfigurationTrayPlate* W906_CfgTrayPlate();

// golden CreateForm(TfConfiguration)（HT9045.cpp:207）的這一塊：建表、把兩顆 Load Data 鈕登記成 C 路具名替身
// （FileRW/IniConfig.gen.inc 的 IC_FormShow 用 EL<TSpeedButton>("TfConfiguration", "sbtReloadTray")->Click()），
// 再跑建構子尾段 cConfiguration.cpp:226-234（if(bHasTrayCSV) sbtReloadTray->Click(); if(bHasPlateCSV) sbtReloadHP->Click();）。冪等。
void W906_CfgTrayPlate_CreateForm();

// golden TfMain::FormShow main.cpp:9987-9988：bHasTrayCSV=FileExists(TrayTablePath); bHasPlateCSV=FileExists(PlateTablePath);
void W906_CfgTrayPlate_MainFormShowFlags();
