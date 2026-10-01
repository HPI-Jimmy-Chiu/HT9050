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
#include <string>                   // AI(W906-S98) 20261001: W906_CfgTrayPlateOp

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

    // ---- AI(W906-S98) 20261001 [W906] St01：todo E-003 ①，兩個分頁其餘的 golden 元件與處理器（網頁 WS cfgtrayplate.op；本體 .cpp 檔尾）----
    //   依據：RULINGS_20260926 S98（這兩張表）＋ S169「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」——蓋過 Q41 盤點
    //   （docs/Q41_INVENTORY_20260927.md 五、「Configuration 的 Tray／HP 分頁：S98，屬 Q41 的『新頁面先不做』」＝S158）。
    //   六顆鈕同上面四顆：Click() 照 VCL 跑 DFM 的 OnClick（本檔 .cpp 的子類別）。
    TSpeedButton *btnAddTray;              // golden cConfiguration.h:1042  OnClick = btnAddTrayClick（dfm:21500；GroupIndex 1、AllowAllUp）
    TSpeedButton *btnDeleteTray;           // golden cConfiguration.h:1043  OnClick = btnDeleteTrayClick（dfm:21614）
    TSpeedButton *btnModifyTray;           // golden cConfiguration.h:1044  OnClick = btnModifyTrayClick（dfm:21730；GroupIndex 1、AllowAllUp）
    TSpeedButton *btnAddHP;                // golden cConfiguration.h:1049  OnClick = btnAddHPClick（dfm:22148）
    TSpeedButton *btnDeleteHP;             // golden cConfiguration.h:1050  OnClick = btnDeleteHPClick（dfm:22262）
    TSpeedButton *btnModifyHP;             // golden cConfiguration.h:1051  OnClick = btnModifyHPClick（dfm:22378）
    TEdit        *edtTemp;                 // golden cConfiguration.h:1047（dfm:21989，Visible=False，Text='edtTemp'）：Modify Data 的小鍵盤框
    int iSelTrayRow, iSelTrayCol, iSelHPRow, iSelHPCol;   // golden cConfiguration.h:2413-2416（建構子 cConfiguration.cpp:138-141 設 0）
    int Tag;                               // golden TComponent::Tag（btnDeleteTrayClick :7000／btnDeleteHPClick :7136 寫；V912 全樹沒有讀者）

    void strngrdTraySelectCell(int ACol, int ARow, bool &CanSelect);   // golden :6945（Sender 拿掉，下同；本體沒讀）
    void strngrdTrayDblClick();            // golden :6952
    void btnModifyTrayClick();             // golden :6957
    void btnAddTrayClick();                // golden :6984
    void btnDeleteTrayClick();             // golden :6996
    void strngrdHPSelectCell(int ACol, int ARow, bool &CanSelect);     // golden :7083
    void strngrdHPDblClick();              // golden :7090
    void btnModifyHPClick();               // golden :7095
    void btnAddHPClick();                  // golden :7120
    void btnDeleteHPClick();               // golden :7132

    // VCL TCustomGrid 的「目前格」（Delphi 6 Grids.pas FCurrent）與它帶出的 OnSelectCell —— vclcompat::TStringGrid 只有裸欄位
    // （RowCount 只夾 >=1、FixedRows／Row 是 int），golden 卻靠這些副作用改 iSel*（見 .cpp 檔尾「VCL 格子」）。hp＝false：Tray、true：HP。
    struct VclCursor { int col; int row; };
    VclCursor curTray, curHP;
    void VclMoveCurrent(bool hp, int ACol, int ARow);   // MoveCurrent：SelectCell（→ OnSelectCell）准了才搬
    void VclSetRowCount(bool hp, int v);                // SetRowCount：<1 夾 1、<=FixedRows 先降 FixedRows、縮到目前格之下 ⇒ MoveCurrent
    void VclSetFixedRows(bool hp, int v);               // SetFixedRows：有變才 Initialize（目前格＝(FixedCols, FixedRows)，不觸發事件）
    void VclSetFixedCols(bool hp, int v);               // SetFixedCols：同上
    void VclSetRow(bool hp, int v);                     // SetRow：不同才 FocusCell → MoveCurrent
    // golden fQwertyKey->ShowQwertyKey(edtTemp, ...)（V912 myQwertyKeyBoard.cpp:169-302）對框的淨效果；操作員打的字由 WS cfgtrayplate.op
    // 帶進來（.cpp 檔尾「小鍵盤」）。沒有網頁操作員時＝forms/fQwertyKey.h BEHAVIOUR NOTE「打開鍵盤、原字送出」。
    void W906ShowQwertyKey(TEdit *Ptr, int iFunction, int iDP=0, bool bCheckRange=false, double min=0, double max=0);

private:
    void W906CtorTail();                   // 建構子尾：上面新成員的初值（golden 建構子 :138-141、VCL 設計期預設）
};

// 唯一的一份（golden fConfiguration 的這一塊）。第一次呼叫時建立，不讀檔。
TfConfigurationTrayPlate* W906_CfgTrayPlate();

// golden CreateForm(TfConfiguration)（HT9045.cpp:207）的這一塊：建表、把兩顆 Load Data 鈕登記成 C 路具名替身
// （FileRW/IniConfig.gen.inc 的 IC_FormShow 用 EL<TSpeedButton>("TfConfiguration", "sbtReloadTray")->Click()），
// 再跑建構子尾段 cConfiguration.cpp:226-234（if(bHasTrayCSV) sbtReloadTray->Click(); if(bHasPlateCSV) sbtReloadHP->Click();）。冪等。
void W906_CfgTrayPlate_CreateForm();

// golden TfMain::FormShow main.cpp:9987-9988：bHasTrayCSV=FileExists(TrayTablePath); bHasPlateCSV=FileExists(PlateTablePath);
void W906_CfgTrayPlate_MainFormShowFlags();

// ---- AI(W906-S98) 20261001 [W906] St01（todo E-003 ①）-------------------------------------------------------------------------------
// 測試縫（照 W906_AGVINI_PATH／W906_A01CONFIGINI_PATH 的形狀）：環境變數 W906_TRAYFORMCSV_PATH／W906_PLATEFORMCSV_PATH 沒設或空字串
//   ＝golden 全域 TrayTablePath／PlateTablePath（移植樹 common.cpp:234-235＝golden common.cpp:42-43 字面值 D:\HT9045\System\TrayForm.csv／
//   PlateForm.csv），位元組不變。每次呼叫都重讀環境變數（ctest 每個 case 換沙盒）；tests/CMakeLists.txt 的 _ht9045_env_extra 給每一支 ctest。
//   本檔讀寫這兩張表的地方全部經過它（common.cpp 那兩行不動：本檔之外沒有活的讀寫者）。
AnsiString W906_TrayTablePath();
AnsiString W906_PlateTablePath();

// WS cfgtrayplate.op（tools/wb_serve.cpp 分派；本體 .cpp 檔尾，自己持 FormJson 鎖）。value 格式、守衛、回覆見 .cpp 檔尾。
//   *ok＝true：get 或 golden 處理器跑了（回覆是 JSON 物件，併進 ack）；false：守衛擋下／參數錯，回 "<碼>: <說明>"。
std::string W906_CfgTrayPlateOp(const std::string& payloadJson, bool* ok);
