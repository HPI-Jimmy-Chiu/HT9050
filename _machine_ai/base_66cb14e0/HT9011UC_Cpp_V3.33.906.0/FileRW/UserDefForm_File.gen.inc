// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 UserDefForm_File 的讀寫檔（HTEditList：elTrayForm → Tray.Data）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cTrayForm.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 84 個（header 宣告 176 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "forms/fMain.h"
#include "forms/fSecurity.h"
#include "forms/fTrayForm.h"
#include "csystem.h"
#include "BarcodeReader.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）

// golden cTrayForm.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden cTrayForm.h:274 TfTrayForm::fShow
#define TrayEdit (fTrayForm->TrayEdit)       // golden cTrayForm.h:249 —— 移植樹 fTrayForm 那一份（它的 SaveSetupFile 也用）
#define asErrorMsg (fTrayForm->asErrorMsg)   // golden cTrayForm.h:279 —— asendic_Loader.cpp 讀 fTrayForm->asErrorMsg

using filerw::EL;

// 前置宣告
static void TF_TfTrayForm();
static void TF_FormShow();
static void TF_DoIniDataToForm();
static void TF_ReadFile();
static void TF_FormClose();
static void TF_spbSaveClick();
static void TF_SaveSetupFile(AnsiString szDir, AnsiString S);

// 移植樹 fTrayForm（forms/fTrayForm.h）已有的同名同型別元件 60 個：替身就是那個物件（開機時最先呼叫）
static void TF_AdoptPortWidgets()
{
    filerw::ELKeep("TfTrayForm", "Tp1Thick", fTrayForm->Tp1Thick);
    filerw::ELKeep("TfTrayForm", "Tp1TickUp", fTrayForm->Tp1TickUp);
    filerw::ELKeep("TfTrayForm", "Tp2Thick", fTrayForm->Tp2Thick);
    filerw::ELKeep("TfTrayForm", "Tp2TickUp", fTrayForm->Tp2TickUp);
    filerw::ELKeep("TfTrayForm", "Tp3Thick", fTrayForm->Tp3Thick);
    filerw::ELKeep("TfTrayForm", "Tp3TickUp", fTrayForm->Tp3TickUp);
    filerw::ELKeep("TfTrayForm", "TrayName1", fTrayForm->TrayName1);
    filerw::ELKeep("TfTrayForm", "TrayName2", fTrayForm->TrayName2);
    filerw::ELKeep("TfTrayForm", "TrayName3", fTrayForm->TrayName3);
    filerw::ELKeep("TfTrayForm", "XBItem1", fTrayForm->XBItem1);
    filerw::ELKeep("TfTrayForm", "XBItem2", fTrayForm->XBItem2);
    filerw::ELKeep("TfTrayForm", "XBItem3", fTrayForm->XBItem3);
    filerw::ELKeep("TfTrayForm", "XBP1", fTrayForm->XBP1);
    filerw::ELKeep("TfTrayForm", "XBP2", fTrayForm->XBP2);
    filerw::ELKeep("TfTrayForm", "XBP3", fTrayForm->XBP3);
    filerw::ELKeep("TfTrayForm", "XBStart1", fTrayForm->XBStart1);
    filerw::ELKeep("TfTrayForm", "XBStart2", fTrayForm->XBStart2);
    filerw::ELKeep("TfTrayForm", "XBStart3", fTrayForm->XBStart3);
    filerw::ELKeep("TfTrayForm", "XCT1", fTrayForm->XCT1);
    filerw::ELKeep("TfTrayForm", "XCT2", fTrayForm->XCT2);
    filerw::ELKeep("TfTrayForm", "XCT3", fTrayForm->XCT3);
    filerw::ELKeep("TfTrayForm", "XPitch1", fTrayForm->XPitch1);
    filerw::ELKeep("TfTrayForm", "XPitch2", fTrayForm->XPitch2);
    filerw::ELKeep("TfTrayForm", "XPitch3", fTrayForm->XPitch3);
    filerw::ELKeep("TfTrayForm", "XST1", fTrayForm->XST1);
    filerw::ELKeep("TfTrayForm", "XST2", fTrayForm->XST2);
    filerw::ELKeep("TfTrayForm", "XST3", fTrayForm->XST3);
    filerw::ELKeep("TfTrayForm", "YBItem1", fTrayForm->YBItem1);
    filerw::ELKeep("TfTrayForm", "YBItem2", fTrayForm->YBItem2);
    filerw::ELKeep("TfTrayForm", "YBItem3", fTrayForm->YBItem3);
    filerw::ELKeep("TfTrayForm", "YBP1", fTrayForm->YBP1);
    filerw::ELKeep("TfTrayForm", "YBP2", fTrayForm->YBP2);
    filerw::ELKeep("TfTrayForm", "YBP3", fTrayForm->YBP3);
    filerw::ELKeep("TfTrayForm", "YBStart1", fTrayForm->YBStart1);
    filerw::ELKeep("TfTrayForm", "YBStart2", fTrayForm->YBStart2);
    filerw::ELKeep("TfTrayForm", "YBStart3", fTrayForm->YBStart3);
    filerw::ELKeep("TfTrayForm", "YBTypeSize1", fTrayForm->YBTypeSize1);
    filerw::ELKeep("TfTrayForm", "YBTypeSize2", fTrayForm->YBTypeSize2);
    filerw::ELKeep("TfTrayForm", "YBTypeSize3", fTrayForm->YBTypeSize3);
    filerw::ELKeep("TfTrayForm", "YCT1", fTrayForm->YCT1);
    filerw::ELKeep("TfTrayForm", "YCT2", fTrayForm->YCT2);
    filerw::ELKeep("TfTrayForm", "YCT3", fTrayForm->YCT3);
    filerw::ELKeep("TfTrayForm", "YPitch1", fTrayForm->YPitch1);
    filerw::ELKeep("TfTrayForm", "YPitch2", fTrayForm->YPitch2);
    filerw::ELKeep("TfTrayForm", "YPitch3", fTrayForm->YPitch3);
    filerw::ELKeep("TfTrayForm", "YST1", fTrayForm->YST1);
    filerw::ELKeep("TfTrayForm", "YST2", fTrayForm->YST2);
    filerw::ELKeep("TfTrayForm", "YST3", fTrayForm->YST3);
    filerw::ELKeep("TfTrayForm", "chkEnableIndicator", fTrayForm->chkEnableIndicator);
    filerw::ELKeep("TfTrayForm", "edBoatYDivision", fTrayForm->edBoatYDivision);
    filerw::ELKeep("TfTrayForm", "edBoatYPitch", fTrayForm->edBoatYPitch);
    filerw::ELKeep("TfTrayForm", "edBoatYStart", fTrayForm->edBoatYStart);
    filerw::ELKeep("TfTrayForm", "edMemo1", fTrayForm->edMemo1);
    filerw::ELKeep("TfTrayForm", "edMemo2", fTrayForm->edMemo2);
    filerw::ELKeep("TfTrayForm", "edMemo3", fTrayForm->edMemo3);
    filerw::ELKeep("TfTrayForm", "edUnloadTrayCount", fTrayForm->edUnloadTrayCount);
    filerw::ELKeep("TfTrayForm", "edtBinBoxAlarm", fTrayForm->edtBinBoxAlarm);
    filerw::ELKeep("TfTrayForm", "edtBinBoxName", fTrayForm->edtBinBoxName);
    filerw::ELKeep("TfTrayForm", "rgDeviceDir", fTrayForm->rgDeviceDir);
    filerw::ELKeep("TfTrayForm", "rgTrayDir", fTrayForm->rgTrayDir);
}

// golden cTrayForm.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void TF_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->Items;
      s->Add("0 deg.");
      s->Add("90 deg.");
      s->Add("180 deg.");
      s->Add("270 deg.");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->Items;
      s->Add("0 deg.");
      s->Add("180 deg.");
      s->Add("0 deg. flipped");
      s->Add("180 deg. flipped");
    }
}

// golden cTrayForm.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kTF_ParentOf[][2] = {
    {"Lab1XPickup", "Panel2"},
    {"Lab2XPickup", "Panel4"},
    {"Lab3XPickup", "Panel6"},
    {"Panel1", "TabSheet1"},
    {"Panel2", "TabSheet1"},
    {"Panel3", "TabSheet2"},
    {"Panel4", "TabSheet2"},
    {"Panel5", "TabSheet3"},
    {"Panel6", "TabSheet3"},
    {"Panel7", "TabSheet4"},
    {"TMyTray1", "Panel2"},
    {"TMyTray2", "Panel4"},
    {"TMyTray3", "Panel6"},
    {"TabSheet1", "PageControl1"},
    {"TabSheet2", "PageControl1"},
    {"TabSheet3", "PageControl1"},
    {"TabSheet4", "PageControl1"},
    {"Tp1Thick", "Panel2"},
    {"Tp1TickUp", "Panel2"},
    {"Tp2Thick", "Panel4"},
    {"Tp2TickUp", "Panel4"},
    {"Tp3Thick", "Panel6"},
    {"Tp3TickUp", "Panel6"},
    {"TrayName1", "Panel1"},
    {"TrayName2", "Panel3"},
    {"TrayName3", "Panel5"},
    {"XBItem1", "gb_TrayBlock1"},
    {"XBItem2", "gb_TrayBlock2"},
    {"XBItem3", "gb_TrayBlock3"},
    {"XBP1", "gb_TrayBlock1"},
    {"XBP2", "gb_TrayBlock2"},
    {"XBP3", "gb_TrayBlock3"},
    {"XBStart1", "gb_TrayBlock1"},
    {"XBStart2", "gb_TrayBlock2"},
    {"XBStart3", "gb_TrayBlock3"},
    {"XCT1", "Panel2"},
    {"XCT2", "Panel4"},
    {"XCT3", "Panel6"},
    {"XPitch1", "Panel2"},
    {"XPitch2", "Panel4"},
    {"XPitch3", "Panel6"},
    {"XST1", "Panel2"},
    {"XST2", "Panel4"},
    {"XST3", "Panel6"},
    {"YBItem1", "gb_TrayBlock1"},
    {"YBItem2", "gb_TrayBlock2"},
    {"YBItem3", "gb_TrayBlock3"},
    {"YBP1", "gb_TrayBlock1"},
    {"YBP2", "gb_TrayBlock2"},
    {"YBP3", "gb_TrayBlock3"},
    {"YBStart1", "gb_TrayBlock1"},
    {"YBStart2", "gb_TrayBlock2"},
    {"YBStart3", "gb_TrayBlock3"},
    {"YBTypeSize1", "gb_TrayBlock1"},
    {"YBTypeSize2", "gb_TrayBlock2"},
    {"YBTypeSize3", "gb_TrayBlock3"},
    {"YCT1", "Panel2"},
    {"YCT2", "Panel4"},
    {"YCT3", "Panel6"},
    {"YPitch1", "Panel2"},
    {"YPitch2", "Panel4"},
    {"YPitch3", "Panel6"},
    {"YST1", "Panel2"},
    {"YST2", "Panel4"},
    {"YST3", "Panel6"},
    {"cbTrayType1", "Panel2"},
    {"cbTrayType2", "Panel4"},
    {"cbTrayType3", "Panel6"},
    {"cbUseThickTray", "Panel2"},
    {"chkEnableIndicator", "tsBinBox"},
    {"edBoatYDivision", "Panel7"},
    {"edBoatYPitch", "Panel7"},
    {"edBoatYStart", "Panel7"},
    {"edMemo1", "Panel2"},
    {"edMemo2", "Panel4"},
    {"edMemo3", "Panel6"},
    {"edUnloadTrayCount", "pnlUnloadTrayCount"},
    {"edtBinBoxAlarm", "pnlBixBox"},
    {"edtBinBoxName", "pnlBixBox"},
    {"edtBinBoxNow", "pnlBixBox"},
    {"gb_TrayBlock1", "Panel2"},
    {"gb_TrayBlock2", "Panel4"},
    {"gb_TrayBlock3", "Panel6"},
    {"pnlBixBox", "tsBinBox"},
    {"pnlUnloadTrayCount", "Panel2"},
    {"rgDeviceDir", "tsBinBox"},
    {"rgTrayDir", "tsBinBox"},
    {"tsBinBox", "PageControl1"},
    {"tsColorSensor", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void TF_CreateContainerProxies()
{
    EL<TPageControl>("TfTrayForm", "PageControl1");
    EL<TPanel>("TfTrayForm", "Panel1");
    EL<TPanel>("TfTrayForm", "Panel2");
    EL<TPanel>("TfTrayForm", "Panel3");
    EL<TPanel>("TfTrayForm", "Panel4");
    EL<TPanel>("TfTrayForm", "Panel5");
    EL<TPanel>("TfTrayForm", "Panel6");
    EL<TPanel>("TfTrayForm", "Panel7");
    EL<TTabSheet>("TfTrayForm", "TabSheet1");
    EL<TTabSheet>("TfTrayForm", "TabSheet2");
    EL<TTabSheet>("TfTrayForm", "TabSheet3");
    EL<TTabSheet>("TfTrayForm", "TabSheet4");
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock1");
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock2");
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock3");
    EL<TPanel>("TfTrayForm", "pnlBixBox");
    EL<TPanel>("TfTrayForm", "pnlUnloadTrayCount");
    EL<TTabSheet>("TfTrayForm", "tsBinBox");
    filerw::ELSetParents("TfTrayForm", kTF_ParentOf, (int)(sizeof(kTF_ParentOf) / sizeof(kTF_ParentOf[0])));
}

// golden cTrayForm.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void TF_DfmState()
{
    EL<TEdit>("TfTrayForm", "edtBinBoxNow")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayForm", "edtBinBoxNow");
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock1")->Visible = false;
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock2")->Visible = false;
    EL<TGroupBox>("TfTrayForm", "gb_TrayBlock3")->Visible = false;
    EL<TPanel>("TfTrayForm", "pnlUnloadTrayCount")->Visible = false;
    EL<TComboBox>("TfTrayForm", "cbTrayType1")->Text = "Select from Database...";
    EL<TComboBox>("TfTrayForm", "cbTrayType2")->Text = "Select from Database...";
    EL<TComboBox>("TfTrayForm", "cbTrayType3")->Text = "Select from Database...";
    EL<TEdit>("TfTrayForm", "edBoatYDivision")->Text = "10";
    EL<TEdit>("TfTrayForm", "edBoatYPitch")->Text = "10";
    EL<TEdit>("TfTrayForm", "edBoatYStart")->Text = "10";
    EL<TEdit>("TfTrayForm", "edtBinBoxAlarm")->Text = "0";
    EL<TEdit>("TfTrayForm", "edtBinBoxNow")->Text = "0";
    EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kTF_EnableAll[] = {
    "GroupBox1", "Lab1XPickup", "Lab2XPickup", "Lab3XPickup", "PageControl1", "Panel1",
    "Panel2", "Panel3", "Panel4", "Panel5", "Panel6", "Panel7",
    "TabSheet1", "TabSheet2", "TabSheet3", "TabSheet4", "cbUseThickTray", "chkEnableIndicator",
    "gb_TrayBlock1", "gb_TrayBlock2", "gb_TrayBlock3", "pnlBixBox", "pnlUnloadTrayCount", "rgDeviceDir",
    "rgTrayDir", "spbSave", "tsBinBox", "tsColorSensor",
};

// 存檔流程（spbSaveClick、SaveSetupFile）讀的替身 8 個 —— FileRW/UserDefForm_File.cpp 用它決定頁面必須送哪些值
static const char* const kTF_SaveReads[] = {
    "Tp1Thick", "cbUseThickTray", "chkEnableIndicator", "edUnloadTrayCount", "edtBinBoxAlarm", "edtBinBoxName",
    "rgDeviceDir", "rgTrayDir",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void TF_CreateSaveProxies()
{
    EL<TEdit>("TfTrayForm", "Tp1Thick");
    EL<TCheckBox>("TfTrayForm", "cbUseThickTray");
    EL<TCheckBox>("TfTrayForm", "chkEnableIndicator");
    EL<TEdit>("TfTrayForm", "edUnloadTrayCount");
    EL<TEdit>("TfTrayForm", "edtBinBoxAlarm");
    EL<TEdit>("TfTrayForm", "edtBinBoxName");
    EL<TRadioGroup>("TfTrayForm", "rgDeviceDir");
    EL<TRadioGroup>("TfTrayForm", "rgTrayDir");
}

// golden cTrayForm.cpp:28  TfTrayForm::TfTrayForm(TComponent* Owner)
static void TF_TfTrayForm()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true;                                                          //, bDisable=false;
    bool bReadFromFile=true;                                                    //, bFixedValue=false;

    TEdit *TempEdit[3][17]=                                                     //2014-03-04    Dell    10->14 (for SPIL WLP Add Tray Block)     //Frank 20160920 add
    {
        {EL<TEdit>("TfTrayForm", "TrayName1"), EL<TEdit>("TfTrayForm", "XST1"), EL<TEdit>("TfTrayForm", "YST1"), EL<TEdit>("TfTrayForm", "XPitch1"), EL<TEdit>("TfTrayForm", "YPitch1"), EL<TEdit>("TfTrayForm", "Tp1Thick"), EL<TEdit>("TfTrayForm", "XCT1"), EL<TEdit>("TfTrayForm", "YCT1"), EL<TEdit>("TfTrayForm", "Tp1TickUp"), EL<TEdit>("TfTrayForm", "edMemo1"), EL<TEdit>("TfTrayForm", "XBStart1"), EL<TEdit>("TfTrayForm", "YBStart1"), EL<TEdit>("TfTrayForm", "XBP1"), EL<TEdit>("TfTrayForm", "YBP1"), EL<TEdit>("TfTrayForm", "XBItem1"), EL<TEdit>("TfTrayForm", "YBItem1"), EL<TEdit>("TfTrayForm", "YBTypeSize1")},
        {EL<TEdit>("TfTrayForm", "TrayName2"), EL<TEdit>("TfTrayForm", "XST2"), EL<TEdit>("TfTrayForm", "YST2"), EL<TEdit>("TfTrayForm", "XPitch2"), EL<TEdit>("TfTrayForm", "YPitch2"), EL<TEdit>("TfTrayForm", "Tp2Thick"), EL<TEdit>("TfTrayForm", "XCT2"), EL<TEdit>("TfTrayForm", "YCT2"), EL<TEdit>("TfTrayForm", "Tp2TickUp"), EL<TEdit>("TfTrayForm", "edMemo2"), EL<TEdit>("TfTrayForm", "XBStart2"), EL<TEdit>("TfTrayForm", "YBStart2"), EL<TEdit>("TfTrayForm", "XBP2"), EL<TEdit>("TfTrayForm", "YBP2"), EL<TEdit>("TfTrayForm", "XBItem2"), EL<TEdit>("TfTrayForm", "YBItem2"), EL<TEdit>("TfTrayForm", "YBTypeSize2")},
        {EL<TEdit>("TfTrayForm", "TrayName3"), EL<TEdit>("TfTrayForm", "XST3"), EL<TEdit>("TfTrayForm", "YST3"), EL<TEdit>("TfTrayForm", "XPitch3"), EL<TEdit>("TfTrayForm", "YPitch3"), EL<TEdit>("TfTrayForm", "Tp3Thick"), EL<TEdit>("TfTrayForm", "XCT3"), EL<TEdit>("TfTrayForm", "YCT3"), EL<TEdit>("TfTrayForm", "Tp3TickUp"), EL<TEdit>("TfTrayForm", "edMemo3"), EL<TEdit>("TfTrayForm", "XBStart3"), EL<TEdit>("TfTrayForm", "YBStart3"), EL<TEdit>("TfTrayForm", "XBP3"), EL<TEdit>("TfTrayForm", "YBP3"), EL<TEdit>("TfTrayForm", "XBItem3"), EL<TEdit>("TfTrayForm", "YBItem3"), EL<TEdit>("TfTrayForm", "YBTypeSize3")}
    };

    EL<TEdit>("TfTrayForm", "XCT1")->Tag=0;                                                                //Steven 20251003 : 避免出錯
    EL<TEdit>("TfTrayForm", "XCT2")->Tag=1;
    EL<TEdit>("TfTrayForm", "XCT3")->Tag=2;
    EL<TEdit>("TfTrayForm", "YCT1")->Tag=0;
    EL<TEdit>("TfTrayForm", "YCT2")->Tag=1;
    EL<TEdit>("TfTrayForm", "YCT3")->Tag=2;
    EL<TEdit>("TfTrayForm", "XPitch1")->Tag=0;
    EL<TEdit>("TfTrayForm", "XPitch2")->Tag=1;
    EL<TEdit>("TfTrayForm", "XPitch3")->Tag=2;
    EL<TEdit>("TfTrayForm", "YPitch1")->Tag=0;
    EL<TEdit>("TfTrayForm", "YPitch2")->Tag=1;
    EL<TEdit>("TfTrayForm", "YPitch3")->Tag=2;

    for(int i=0; i<3; i++)
        for(int j=0; j<17; j++)
            TrayEdit[i][j]=TempEdit[i][j];

    elTrayForm->Add(EL<TEdit>("TfTrayForm", "edBoatYDivision"),         &UserDefForm_File[0].iCassetteZItem,    ECPosInt,     "Type0",    "Cassette Z Item",        bShow, bEnable, bReadFromFile, 14,      false, 1, 50);  //RogerYang 20260203 : Add for HT9046CR
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "edBoatYStart"),  &UserDefForm_File[0].dCassetteZStart,   ECDouble,     "Type0",    "Cassette Z Start",       bShow, bEnable, bReadFromFile, 500,      false, 1, 3000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "edBoatYPitch"),       &UserDefForm_File[0].dCassetteZPitch,   ECDouble,     "Type0",    "Cassette Z Pitch",       bShow, bEnable, bReadFromFile, 500,      false, 1, 3000);

//    elTrayForm->Add(TrayName1,  &UserDefForm_File[0].Alias,       ECText,       "Type0",    "Name",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XCT1"),       &UserDefForm_File[0].XDivision,   ECPosInt,     "Type0",    "X Division",           bShow, bEnable, bReadFromFile, 14,      false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YCT1"),       &UserDefForm_File[0].YDivision,   ECPosInt,     "Type0",    "Y Division",           bShow, bEnable, bReadFromFile, 35,      false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XST1"),       &UserDefForm_File[0].XStart,      ECDouble,     "Type0",    "X Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YST1"),       &UserDefForm_File[0].YStart,      ECDouble,     "Type0",    "Y Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XPitch1"),    &UserDefForm_File[0].XPitch,      ECDouble,     "Type0",    "X Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.000, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YPitch1"),    &UserDefForm_File[0].YPitch,      ECDouble,     "Type0",    "Y Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp1Thick"),   &UserDefForm_File[0].ZDepth,      ECDouble,     "Type0",    "Think",                bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp1TickUp"),   &UserDefForm_File[0].iPickUp,    ECDouble,     "Type0",    "Pick Up",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
//    elTrayForm->Add(edMemo1,    &UserDefForm_File[0].Alias,       ECText,       "Type0",    "Memo",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    if(IniConfig.bUseTrayBlockMode)
    {
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBStart1"),   &UserDefForm_File[0].BlockXStart, ECDouble,     "Type0",    "Block XStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBStart1"),   &UserDefForm_File[0].BlockYStart, ECDouble,     "Type0",    "Block YStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBP1"),       &UserDefForm_File[0].BlockPitchX, ECDouble,     "Type0",    "Block PitchX",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBP1"),       &UserDefForm_File[0].BlockPitchY, ECDouble,     "Type0",    "Block PitchY",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBItem1"),    &UserDefForm_File[0].BlockXItem,  ECPosInt,     "Type0",    "Block XItem",          bShow, bEnable, bReadFromFile, 2,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBItem1"),    &UserDefForm_File[0].BlockYItem,  ECPosInt,     "Type0",    "Block YItem",          bShow, bEnable, bReadFromFile, 5,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBTypeSize1"),&UserDefForm_File[0].BlockTraySize, ECText,     "Type0",    "Block Tray Type Size", bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    }

//    elTrayForm->Add(TrayName2,  &UserDefForm_File[1].Alias,       ECText,       "Type1",    "Name",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XST2"),       &UserDefForm_File[1].XStart,      ECDouble,     "Type1",    "X Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YST2"),       &UserDefForm_File[1].YStart,      ECDouble,     "Type1",    "Y Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XPitch2"),    &UserDefForm_File[1].XPitch,      ECDouble,     "Type1",    "X Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.000, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YPitch2"),    &UserDefForm_File[1].YPitch,      ECDouble,     "Type1",    "Y Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp2Thick"),   &UserDefForm_File[1].ZDepth,      ECDouble,     "Type1",    "Think",                bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XCT2"),       &UserDefForm_File[1].XDivision,   ECPosInt,     "Type1",    "X Division",           bShow, bEnable, bReadFromFile, 14,     false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YCT2"),       &UserDefForm_File[1].YDivision,   ECPosInt,     "Type1",    "Y Division",           bShow, bEnable, bReadFromFile, 35,     false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp2TickUp"),   &UserDefForm_File[1].iPickUp,    ECDouble,     "Type1",    "Pick Up",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
//    elTrayForm->Add(edMemo2,    &UserDefForm_File[1].Alias,       ECText,       "Type1",    "Memo",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    if(IniConfig.bUseTrayBlockMode)
    {
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBStart2"),   &UserDefForm_File[1].BlockXStart, ECDouble,     "Type1",    "Block XStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBStart2"),   &UserDefForm_File[1].BlockYStart, ECDouble,     "Type1",    "Block YStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBP2"),       &UserDefForm_File[1].BlockPitchX, ECDouble,     "Type1",    "Block PitchX",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBP2"),       &UserDefForm_File[1].BlockPitchY, ECDouble,     "Type1",    "Block PitchY",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBItem2"),    &UserDefForm_File[1].BlockXItem,  ECPosInt,     "Type1",    "Block XItem",          bShow, bEnable, bReadFromFile, 2,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBItem2"),    &UserDefForm_File[1].BlockYItem,  ECPosInt,     "Type1",    "Block YItem",          bShow, bEnable, bReadFromFile, 5,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBTypeSize2"),&UserDefForm_File[1].BlockTraySize, ECText,     "Type1",    "Block Tray Type Size", bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    }

//    elTrayForm->Add(TrayName1,  &UserDefForm_File[2].Alias,       ECText,       "Type2",    "Name",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XST3"),       &UserDefForm_File[2].XStart,      ECDouble,     "Type2",    "X Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YST3"),       &UserDefForm_File[2].YStart,      ECDouble,     "Type2",    "Y Start",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XPitch3"),    &UserDefForm_File[2].XPitch,      ECDouble,     "Type2",    "X Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.000, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YPitch3"),    &UserDefForm_File[2].YPitch,      ECDouble,     "Type2",    "Y Pitch",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp3Thick"),   &UserDefForm_File[2].ZDepth,      ECDouble,     "Type2",    "Think",                bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "XCT3"),       &UserDefForm_File[2].XDivision,   ECPosInt,     "Type2",    "X Division",           bShow, bEnable, bReadFromFile, 14,     false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "YCT3"),       &UserDefForm_File[2].YDivision,   ECPosInt,     "Type2",    "Y Division",           bShow, bEnable, bReadFromFile, 35,     false, 1, 1000);
    elTrayForm->Add(EL<TEdit>("TfTrayForm", "Tp3TickUp"),   &UserDefForm_File[2].iPickUp,    ECDouble,     "Type2",    "Pick Up",              bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
//    elTrayForm->Add(edMemo3,    &UserDefForm_File[2].Alias,       ECText,       "Type2",    "Memo",                 bShow, bEnable, bReadFromFile, AnsiString("NONE"),      true);
    if(IniConfig.bUseTrayBlockMode)
    {
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBStart3"),   &UserDefForm_File[2].BlockXStart, ECDouble,     "Type2",    "Block XStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBStart3"),   &UserDefForm_File[2].BlockYStart, ECDouble,     "Type2",    "Block YStart",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBP3"),       &UserDefForm_File[2].BlockPitchX, ECDouble,     "Type2",    "Block PitchX",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBP3"),       &UserDefForm_File[2].BlockPitchY, ECDouble,     "Type2",    "Block PitchY",         bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "XBItem3"),    &UserDefForm_File[2].BlockXItem,  ECPosInt,     "Type2",    "Block XItem",          bShow, bEnable, bReadFromFile, 2,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBItem3"),    &UserDefForm_File[2].BlockYItem,  ECPosInt,     "Type2",    "Block YItem",          bShow, bEnable, bReadFromFile, 5,      false, 1, 1000);
        elTrayForm->Add(EL<TEdit>("TfTrayForm", "YBTypeSize3"),&UserDefForm_File[2].BlockTraySize, ECText,     "Type2",    "Block Tray Type Size", bShow, bEnable, bReadFromFile, 0.1,     true, 0.001, 1000.00);
    }
    elTrayForm->SetBarcodeType(bcTrayForm);
    asErrorMsg="";
    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorFTEnable"), &TestIF_File.sLoaderColorSenFTEnable, ECText, "Color Sensor", "sLoaderColorSenFTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorRTEnable"), &TestIF_File.sLoaderColorSenRTEnable, ECText, "Color Sensor", "sLoaderColorSenRTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); filerw::ELTodo("golden cTrayForm.cpp:129/:132-133 color sensor GUI (uColorSensorInfo) + LoadColorSensorMapping not ported"); }   // golden cTrayForm.cpp:127-138 取代：色感 MU-N GUI（uColorSensorInfo，移植樹沒有這個類別）：資料註冊照 golden，掛到移植樹的替代 TEdit（同 cTrayForm.cpp Init()）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:127-138 -- 色感 MU-N GUI（uColorSensorInfo，移植樹沒有這個類別）：資料註冊照 golden，掛到移植樹的替代 TEdit（同 cTrayForm.cpp Init()）
    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader)                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
    {
        InitialColorSensorGUI();                                                //Jimmychiu 20230630 : add color sensor MU-N in Loader
        elTrayForm->Add(GetColorSensor("ColorSensor_FT")->edEnable,        &TestIF_File.sLoaderColorSenFTEnable,     ECText,     "Color Sensor", "sLoaderColorSenFTEnable" ,  bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        elTrayForm->Add(GetColorSensor("ColorSensor_RT")->edEnable,        &TestIF_File.sLoaderColorSenRTEnable,     ECText,     "Color Sensor", "sLoaderColorSenRTEnable" ,  bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=true;
        LoadColorSensorMapping();
    }
    else
    {
        EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=false;
    }
#endif // GATE (S12-C save)

}

// golden cTrayForm.cpp:141  TfTrayForm::FormShow(TObject *Sender)
static void TF_FormShow()
{

    AnsiString S;
    ;   // golden cTrayForm.cpp:144-144 取代：LoadImage()：示意圖（HTML 自己有）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:144-144 -- LoadImage()：示意圖（HTML 自己有）
    LoadImage();
#endif // GATE (S12-C save)
    TF_ReadFile();
    TF_DoIniDataToForm();
    S.sprintf("Tray Form  ''%s''  ", GetLastOpenFN());
    ;   // golden cTrayForm.cpp:148-148 取代：Caption：視窗標題
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:148-148 -- Caption：視窗標題
    Caption=S;
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfTrayForm", "PageControl1")->ActivePage=EL<TTabSheet>("TfTrayForm", "TabSheet1");
    ;   // golden cTrayForm.cpp:150-150 取代：UpDateType()：「從 Type? 複製」下拉的選項（依目前分頁，頁面端的複製功能）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:150-150 -- UpDateType()：「從 Type? 複製」下拉的選項（依目前分頁，頁面端的複製功能）
    UpDateType();
#endif // GATE (S12-C save)

    EL<TComboBox>("TfTrayForm", "cbTrayType1")->ItemIndex=0;
    EL<TComboBox>("TfTrayForm", "cbTrayType2")->ItemIndex=0;
    EL<TComboBox>("TfTrayForm", "cbTrayType3")->ItemIndex=0;
    EL<TComboBox>("TfTrayForm", "cbTrayType1")->Items->Clear();
    EL<TComboBox>("TfTrayForm", "cbTrayType2")->Items->Clear();
    EL<TComboBox>("TfTrayForm", "cbTrayType3")->Items->Clear();
    EL<TComboBox>("TfTrayForm", "cbTrayType1")->Text="Select from Database...";
    EL<TComboBox>("TfTrayForm", "cbTrayType2")->Text="Select from Database...";
    EL<TComboBox>("TfTrayForm", "cbTrayType3")->Text="Select from Database...";

    if(bHasTrayCSV) { filerw::ELTodo("golden cTrayForm.cpp:162-175 cbTrayType* items from fConfiguration->strngrdTray (Tray CSV) not ported"); }   // golden cTrayForm.cpp:162-175 取代：bHasTrayCSV：從 fConfiguration->strngrdTray（Tray CSV）列出選項 —— 移植樹 fConfiguration 沒有這張表
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:162-175 -- bHasTrayCSV：從 fConfiguration->strngrdTray（Tray CSV）列出選項 —— 移植樹 fConfiguration 沒有這張表
    if(bHasTrayCSV)                                                             //Steven 20210629 : Tray Form改成CSV
    {
        fConfiguration->sbtReloadTray->Click();
        for(int i=0; i<fConfiguration->strngrdTray->RowCount; i++)
        {
            if(fConfiguration->strngrdTray->Cells[0][i]!="" &&
               fConfiguration->strngrdTray->Cells[0][i]!=" ")
            {
                EL<TComboBox>("TfTrayForm", "cbTrayType1")->Items->Add(fConfiguration->strngrdTray->Cells[0][i]);
                EL<TComboBox>("TfTrayForm", "cbTrayType2")->Items->Add(fConfiguration->strngrdTray->Cells[0][i]);
                EL<TComboBox>("TfTrayForm", "cbTrayType3")->Items->Add(fConfiguration->strngrdTray->Cells[0][i]);
            }
        }
    }
#endif // GATE (S12-C save)

    EL<TTabSheet>("TfTrayForm", "tsBinBox")->TabVisible=(IniConfig.bBinBox || IniConfig.bShowTrayAndDeviceDir);
    //jou 2012-12-11 support Bin Box start
    EL<TEdit>("TfTrayForm", "edtBinBoxNow")->Text   = LastSet.iBinBoxCount;
    EL<TPanel>("TfTrayForm", "pnlBixBox")->Visible   = IniConfig.bBinBox;
    //jou 2012-12-11 support Bin Box end

    //jou 2013-03-25 show Tray & Device Direction start
    EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->Visible        = IniConfig.bShowTrayAndDeviceDir;
    EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->Visible          = IniConfig.bShowTrayAndDeviceDir;
    EL<TCheckBox>("TfTrayForm", "chkEnableIndicator")->Visible = IniConfig.bShowTrayAndDeviceDir;              //Steven 20190211 : 可以取消顯示IC方向

    EL<TCheckBox>("TfTrayForm", "cbUseThickTray")->Visible=CosFunction.bThickTrayUseDiffHeight;                //JerryYang 20230620 : add厚tray選項

    if(IniConfig.bShowTrayAndDeviceDir)
    {
        EL<TTabSheet>("TfTrayForm", "tsBinBox")->Caption="Tray && Device Direct";
    }
    //jou 2013-03-25 show Tray & Device Direction end

    if(IniConfig.bUseTrayBlockMode && IniConfig.bP06_LoaderUseCarrierTray==true)
    {
        EL<TGroupBox>("TfTrayForm", "gb_TrayBlock1")->Visible=true;
        EL<TGroupBox>("TfTrayForm", "gb_TrayBlock2")->Visible=true;
        EL<TGroupBox>("TfTrayForm", "gb_TrayBlock3")->Visible=true;
    }

    //jou 981207 權限控制
    if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)                          //Steven 20101221   //ChungHung 20130621 add SCK RMS
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=false;
            EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=false;
        }
        else if(HasICUnderMachine())                                            //Steven 20110826
        {
            EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=false;
            EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=false;
        }
        else
        {
            EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=fSecurity->Insufficient(14, false);
            EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=fSecurity->Insufficient(14, false);
            EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=fSecurity->Insufficient(14, false);
            EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=fSecurity->Insufficient(14, false);
        }
    }
    else
    {
        if(HasICUnderMachine())                                                 //Steven 20110826
        {
            EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=false;
            EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=false;
            EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=false;
        }
        else
        {
            EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=fSecurity->Insufficient(14, false);
            EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=fSecurity->Insufficient(14, false);
            EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=fSecurity->Insufficient(14, false);
            EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=fSecurity->Insufficient(14, false);
            if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)      //JerryYang 20220311 : ATP鎖定Critical parameter
            {
                if(bAuthCriticalPara[18])
                {
                    EL<TTabSheet>("TfTrayForm", "TabSheet1")->Enabled=false;
                    EL<TTabSheet>("TfTrayForm", "TabSheet2")->Enabled=false;
                    EL<TTabSheet>("TfTrayForm", "TabSheet3")->Enabled=false;
                    EL<TGroupBox>("TfTrayForm", "GroupBox1")->Enabled=false;
                }
            }
        }
    }

    if(IniConfig.bC03UseCatchTray)                                              //kevin 20220302 add tray X width
    {
        EL<TLabel>("TfTrayForm", "Lab1XPickup")->Caption="Catch Tray Range:60-73";
        EL<TLabel>("TfTrayForm", "Lab2XPickup")->Caption="Catch Tray Range:60-73";
        EL<TLabel>("TfTrayForm", "Lab3XPickup")->Caption="Catch Tray Range:60-73";
    }
    else
    {
        EL<TLabel>("TfTrayForm", "Lab1XPickup")->Caption="Tray Range:30-110";
        EL<TLabel>("TfTrayForm", "Lab2XPickup")->Caption="Tray Range:30-110";
        EL<TLabel>("TfTrayForm", "Lab3XPickup")->Caption="Tray Range:30-110";
    }

    EL<TPanel>("TfTrayForm", "pnlUnloadTrayCount")->Visible=(CUSTOMER_CODE==CC_VTEST_Shanghai);

    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=true; filerw::ELTodo("golden cTrayForm.cpp:274 LoadColorSensorMapping not ported"); } else { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=false; }   // golden cTrayForm.cpp:271-281 取代：色感分頁顯示＋LoadColorSensorMapping（缺鍵會寫 ColorSensorType.ini）＋視窗位置
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:271-281 -- 色感分頁顯示＋LoadColorSensorMapping（缺鍵會寫 ColorSensorType.ini）＋視窗位置
    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader)                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
    {
        EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=true;
        LoadColorSensorMapping();
    }
    else
    {
        EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=false;
    }
    Top=10;
    Left=200;
#endif // GATE (S12-C save)
    fShow=true;

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cTrayForm.cpp:318  TfTrayForm::DoIniDataToForm()
static void TF_DoIniDataToForm()
{

    AnsiString S="", asString="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);

    if(elTrayForm!=NULL)
    {
        elTrayForm->InitialDataToEdit();
    }

    ;   // golden cTrayForm.cpp:330-335 取代：TMyTray1..3（TTMyTray 盤面示意圖）的 XItem／YItem：純畫面
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:330-335 -- TMyTray1..3（TTMyTray 盤面示意圖）的 XItem／YItem：純畫面
    EL<TControl>("TfTrayForm", "TMyTray1")->XItem=UserDefForm_File[0].XDivision;
    EL<TControl>("TfTrayForm", "TMyTray1")->YItem=UserDefForm_File[0].YDivision;
    EL<TControl>("TfTrayForm", "TMyTray2")->XItem=UserDefForm_File[1].XDivision;
    EL<TControl>("TfTrayForm", "TMyTray2")->YItem=UserDefForm_File[1].YDivision;
    EL<TControl>("TfTrayForm", "TMyTray3")->XItem=UserDefForm_File[2].XDivision;
    EL<TControl>("TfTrayForm", "TMyTray3")->YItem=UserDefForm_File[2].YDivision;
#endif // GATE (S12-C save)

    szDir+="\\Tray.Data";

    for(int i=0; i<3; i++)
    {
        TrayEdit[i][0]->Text     =UserDefForm_File[i].Alias;
        TrayEdit[i][9]->Text     =UserDefForm_File[i].cMemo;
    }

    if(IniConfig.bBinBox)                                                       //jou 2012-12-11 support Bin Box
    {
        EL<TEdit>("TfTrayForm", "edtBinBoxName")->Text =UserDefForm_File[3].Alias;
        EL<TEdit>("TfTrayForm", "edtBinBoxAlarm")->Text=UserDefForm_File[3].iBinBoxAlarm;
    }

    if(IniConfig.bShowTrayAndDeviceDir)                                         //jou 2013-03-25 show Tray & Device Direction
    {
        EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->ItemIndex      =UserDefForm_File[0].iDeviceDirection;
        EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->ItemIndex        =UserDefForm_File[0].iTrayDirection;
        EL<TCheckBox>("TfTrayForm", "chkEnableIndicator")->Checked =UserDefForm_File[0].bEnableIndicator;      //Steven 20190211 : 可以取消顯示IC方向
    }

    if(CosFunction.bThickTrayUseDiffHeight)                                     //JerryYang 20230620 : add厚tray選項
    {
        EL<TCheckBox>("TfTrayForm", "cbUseThickTray")->Checked=UserDefForm_File[0].bUseThickTray;
    }

}

// golden cTrayForm.cpp:364  TfTrayForm::ReadFile()
static void TF_ReadFile()
{

    AnsiString S="", asString="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\", DataPath, S);                                       //Steven 20210716 : Add \\
    if(elTrayForm!=NULL)
    {
        elTrayForm->ReadEditTextFromFile(szDir, "Tray.Data");
        elTrayForm->InitialDataToEdit();
    }
    szDir+="Tray.Data";
    for(int i=0; i<3; i++)
    {
        S="Type"+AnsiString(i);
        asString=ReadIniData(szDir, S, "Name", AnsiString("NONE"));
        strncpy(UserDefForm_File[i].Alias,   asString.c_str(), sizeof(UserDefForm_File[i].Alias));

        asString=ReadIniData(szDir, S, "Memo", AnsiString("NONE"));
        strncpy(UserDefForm_File[i].cMemo,   asString.c_str(), sizeof(UserDefForm_File[i].cMemo));

        if(IniConfig.bSPILFunction)                                             //JerryYang 20250731 : 矽品凱富要求不要強制改0
        {
        }
        else
        {
            if(UserDefForm_File[i].XDivision==1)                                //Steven 20240808 : Tray只有一行的, X Pitch為0
                UserDefForm_File[i].XPitch=0;
        }
    }

    //jou 2012-12-11 support Bin Box start
    if(IniConfig.bBinBox)
    {
        EL<TEdit>("TfTrayForm", "edtBinBoxName")->Text=ReadIniData(szDir, "Bin Box", "Name", AnsiString("Bin Box"));
        strncpy(UserDefForm_File[3].Alias, EL<TEdit>("TfTrayForm", "edtBinBoxName")->Text.c_str(), sizeof(UserDefForm_File[3].Alias));
        EL<TEdit>("TfTrayForm", "edtBinBoxAlarm")->Text=ReadIniData(szDir, "Bin Box", "Alarm Count", 1000);

        UserDefForm_File[3].XStart      =0;
        UserDefForm_File[3].YStart      =0;

        if(UserDefForm_File[0].XPitch<12.80)
            UserDefForm_File[3].XPitch  =UserDefForm_File[0].XPitch*2.0;
        else
            UserDefForm_File[3].XPitch  =UserDefForm_File[0].XPitch;

        UserDefForm_File[3].YPitch      =UserDefForm_File[0].YPitch;
        UserDefForm_File[3].ZDepth      =0;
        UserDefForm_File[3].XDivision   =4;
        UserDefForm_File[3].YDivision   =2;
        UserDefForm_File[3].iPickUp     =0;
        strncpy(UserDefForm_File[3].cMemo, "", sizeof(UserDefForm_File[3].cMemo));
        UserDefForm_File[3].iBinBoxAlarm=atoi(EL<TEdit>("TfTrayForm", "edtBinBoxAlarm")->Text.c_str());
    }
    //jou 2012-12-11 support Bin Box end

    //jou 2013-03-25 show Tray & Device Direction start
    if(IniConfig.bShowTrayAndDeviceDir)
    {
        EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->ItemIndex      =ReadIniData(szDir, "ShowTrayAndDeviceDir", "Device Direction", 0);
        EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->ItemIndex        =ReadIniData(szDir, "ShowTrayAndDeviceDir", "Tray Direction", 0);
        EL<TCheckBox>("TfTrayForm", "chkEnableIndicator")->Checked =ReadIniData(szDir, "ShowTrayAndDeviceDir", "Enable Indicator", true);              //Steven 20190211 : 可以取消顯示IC方向

        for(int i=0; i<4; i++)
        {
            UserDefForm_File[i].iDeviceDirection=EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->ItemIndex;
            UserDefForm_File[i].iTrayDirection  =EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->ItemIndex;
            UserDefForm_File[i].bEnableIndicator=EL<TCheckBox>("TfTrayForm", "chkEnableIndicator")->Checked;   //Steven 20190211 : 可以取消顯示IC方向
        }
    }
    //jou 2013-03-25 show Tray & Device Direction end

    if(CosFunction.bThickTrayUseDiffHeight)                                     //JerryYang 20230620 : add厚tray選項
    {
        UserDefForm_File[0].bUseThickTray=ReadIniData(szDir, "Flag", "UseThickTray", false);
    }

    if(CUSTOMER_CODE==CC_VTEST_Shanghai)
    {
        TrayForm.iUnloadTrayCount =CheckAndReadIniData(szDir, "Other", "Unload Tray Count", 10);
        EL<TEdit>("TfTrayForm", "edUnloadTrayCount")->Text=TrayForm.iUnloadTrayCount;
    }

    if(IniConfig.bShowTrayAndDeviceDir)                                         //jou 2013-03-25 show Tray & Device Direction
    {
    ;   // golden cTrayForm.cpp:449-450 取代：rgDeviceDirClick／rgTrayDirClick：只切換示意圖與 fShowMessage 的圖（純畫面）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:449-450 -- rgDeviceDirClick／rgTrayDirClick：只切換示意圖與 fShowMessage 的圖（純畫面）
        rgDeviceDirClick(this);
        rgTrayDirClick(this);
#endif // GATE (S12-C save)
    }
    ;   // golden cTrayForm.cpp:452-452 取代：LoadColorSensorEnable()：色感 GUI 勾選框（uColorSensorInfo 未移植；值在 TestIF_File 字串，已由 elTrayForm 讀）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:452-452 -- LoadColorSensorEnable()：色感 GUI 勾選框（uColorSensorInfo 未移植；值在 TestIF_File 字串，已由 elTrayForm 讀）
    LoadColorSensorEnable();                                                    //Jimmychiu 20230630 : add color sensor MU-N in Loader
#endif // GATE (S12-C save)

}

// golden cTrayForm.cpp:559  TfTrayForm::FormClose(TObject *Sender, TCloseAction &Action)
static void TF_FormClose()
{

    TF_ReadFile();
    TF_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
    fShow=false;

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);    //Steven 20100629

}

// golden cTrayForm.cpp:610  TfTrayForm::spbSaveClick(TObject *Sender)
static void TF_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cTrayForm.cpp:616-616 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:616-616 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\", DataPath, S);                                       //Steven 20210716 : Add \\

//kevin 20150105  Start 另存 jobfile
    TF_SaveSetupFile(szDir, S);                                                    //kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(S,"JOBFILE");
    #endif
//kevin 20150105 end
    AddSpace(szDir);
    TF_ReadFile();
    EL<TSpeedButton>("TfTrayForm", "spbSave")->Down=false;
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

// golden cTrayForm.cpp:643  TfTrayForm::SaveSetupFile(AnsiString szDir, AnsiString S)
static void TF_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    MyForceDirectories(szDir);
    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) filerw::ELTodo("golden cTrayForm.cpp:646 SaveColorSensorEnable (uColorSensorInfo GUI) not ported");   // golden cTrayForm.cpp:646-646 取代：SaveColorSensorEnable()：色感 GUI 勾選框 → TestIF_File 字串（uColorSensorInfo 未移植）
#if 0 // GATE (S12-C save) golden cTrayForm.cpp:646-646 -- SaveColorSensorEnable()：色感 GUI 勾選框 → TestIF_File 字串（uColorSensorInfo 未移植）
    SaveColorSensorEnable();                                                    //Jimmychiu 20230630 : add color sensor MU-N in Loader
#endif // GATE (S12-C save)
    if(elTrayForm!=NULL)
    {
        elTrayForm->SaveEditTextToFile(szDir, "Tray.Data");
    }

    szDir+="Tray.Data";
    for(int i=0; i<3; i++)
    {
        S="Type"+AnsiString (i);
        WriteIniData(szDir, S, "Name",      TrayEdit[i][0]->Text);
        WriteIniData(szDir, S, "Memo",      TrayEdit[i][9]->Text);
    }

    //jou 2012-12-11 support Bin Box start
    if(IniConfig.bBinBox)
    {
        WriteIniData(szDir, "Bin Box", "Name",          EL<TEdit>("TfTrayForm", "edtBinBoxName")->Text);
        WriteIniData(szDir, "Bin Box", "Alarm Count",   EL<TEdit>("TfTrayForm", "edtBinBoxAlarm")->Text);
    }
    //jou 2012-12-11 support Bin Box end

    //jou 2013-03-25 show Tray & Device Direction start
    if(IniConfig.bShowTrayAndDeviceDir)
    {
        WriteIniData(szDir, "ShowTrayAndDeviceDir", "Device Direction",  EL<TRadioGroup>("TfTrayForm", "rgDeviceDir")->ItemIndex);
        WriteIniData(szDir, "ShowTrayAndDeviceDir", "Tray Direction",    EL<TRadioGroup>("TfTrayForm", "rgTrayDir")->ItemIndex);
        WriteIniData(szDir, "ShowTrayAndDeviceDir", "Enable Indicator",  EL<TCheckBox>("TfTrayForm", "chkEnableIndicator")->Checked);                  //Steven 20190211 : 可以取消顯示IC方向
    }
    //jou 2013-03-25 show Tray & Device Direction end

    if(CUSTOMER_CODE==CC_VTEST_Shanghai)
    {
        WriteIniData(szDir, "Other", "Unload Tray Count",  EL<TEdit>("TfTrayForm", "edUnloadTrayCount")->Text);
    }

    if(CosFunction.bThickTrayUseDiffHeight)                                     //JerryYang 20230620 : add厚tray選項
    {
        WriteIniData(szDir, "Flag", "UseThickTray",  EL<TCheckBox>("TfTrayForm", "cbUseThickTray")->Checked);

        if(HasICUnderMachine()==false && EL<TCheckBox>("TfTrayForm", "cbUseThickTray")->Checked==false && atof(EL<TEdit>("TfTrayForm", "Tp1Thick")->Text.c_str())>9)
        {
            filerw::ELMessage("tray厚度大於9mm, 請確認是否需要勾選Thick tray選項換算in/out arm高度");
        }
    }

}

