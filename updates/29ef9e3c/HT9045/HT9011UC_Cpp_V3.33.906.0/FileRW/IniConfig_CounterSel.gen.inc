// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 IniConfig_CounterSel 的讀寫檔（HTEditList： → config\config.ini [Visible] 11 鍵（golden cprod.cpp:2702（V912 :2721）ProcessLastSetIni_Visible；FormShow 讀、FormClose 寫）、config\FormPos.def（golden FormClose :91-103，cbDefaultValue 勾選時；本檔擋掉，見檔頭））。
//  來源：golden D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cCounterSel.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cCounterSel.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 18 個（header 宣告 34 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"
#include "forms/fMain.h"
#include "forms/fShowBinSelect.h"
#include "forms/fCounterSel.h"


// golden cCounterSel.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
#define NeedRef (fCounterSel->NeedRef)   // golden cCounterSel.h:56：門面 forms/fCounterSel.h 的那一個。golden 讀者是 TfMain::Timer1Timer main.cpp:3208-3213（V912 :3299-3304；NeedRef → fTestCategory->SetShowCateMode()＋DoShowUserDefFrom()）；移植樹還沒有這個讀者（cTestCategory.cpp:640 註記）
static bool fShow=false;   // golden cCounterSel.h:57 —— 本 TU 自己的（同 FileRW/ShuttleMove.cpp 的理由）：網頁用視窗框的 ✕ 關頁沒有回報，寫門面 fCounterSel->fShow 會停在 true；golden 讀者 Command.cpp:7356／:9949（V912 :7357／:9957）「有表單開著」那串條件在移植樹仍是 #if 0

using filerw::EL;

// 前置宣告
static void CSL_TfCounterSel();
static void CSL_FormShow();
static void CSL_FormClose();

// golden cCounterSel.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void CSL_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfCounterSel", "rgTestCategory")->Items;
      s->Add("Normal");
      s->Add("By Arm");
    }
}

// golden cCounterSel.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kCSL_ParentOf[][2] = {
    {"cbContactHeight", "gbStatus"},
    {"cbCycleTime", "gbStatus"},
    {"cbIndexTime", "gbStatus"},
    {"cbUPH", "gbStatus"},
    {"gbBinAssign", "GroupBox17"},
    {"gbContactCount", "GroupBox17"},
    {"gbLoadingCount", "GroupBox17"},
    {"gbScanner", "GroupBox17"},
    {"gbStatus", "GroupBox17"},
    {"gbTemperature", "GroupBox17"},
    {"gbTestCategory", "GroupBox17"},
    {"rbBinAssign_Off", "gbBinAssign"},
    {"rbBinAssign_On", "gbBinAssign"},
    {"rbContactCount_Off", "gbContactCount"},
    {"rbContactCount_On", "gbContactCount"},
    {"rbLoadingCount_Off", "gbLoadingCount"},
    {"rbLoadingCount_On", "gbLoadingCount"},
    {"rbScanner_Off", "gbScanner"},
    {"rbScanner_On", "gbScanner"},
    {"rbTemperature_Off", "gbTemperature"},
    {"rbTemperature_On", "gbTemperature"},
    {"rbTestCategory_Off", "gbTestCategory"},
    {"rbTestCategory_On", "gbTestCategory"},
    {"rgTestCategory", "gbTestCategory"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void CSL_CreateContainerProxies()
{
    EL<TGroupBox>("TfCounterSel", "GroupBox17");
    EL<TGroupBox>("TfCounterSel", "gbBinAssign");
    EL<TGroupBox>("TfCounterSel", "gbContactCount");
    EL<TGroupBox>("TfCounterSel", "gbLoadingCount");
    EL<TGroupBox>("TfCounterSel", "gbScanner");
    EL<TGroupBox>("TfCounterSel", "gbStatus");
    EL<TGroupBox>("TfCounterSel", "gbTemperature");
    EL<TGroupBox>("TfCounterSel", "gbTestCategory");
    filerw::ELSetParents("TfCounterSel", kCSL_ParentOf, (int)(sizeof(kCSL_ParentOf) / sizeof(kCSL_ParentOf[0])));
}

// golden cCounterSel.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void CSL_DfmState()
{
    EL<TGroupBox>("TfCounterSel", "gbScanner")->Enabled = false;
    EL<TRadioGroup>("TfCounterSel", "rgTestCategory")->ItemIndex = 1;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kCSL_EnableAll[] = {
    "GroupBox17", "cbContactHeight", "cbCycleTime", "cbDefaultValue", "cbIndexTime", "cbUPH",
    "gbBinAssign", "gbContactCount", "gbLoadingCount", "gbScanner", "gbStatus", "gbTemperature",
    "gbTestCategory", "rbBinAssign_Off", "rbBinAssign_On", "rbContactCount_Off", "rbContactCount_On", "rbLoadingCount_Off",
    "rbLoadingCount_On", "rbScanner_Off", "rbScanner_On", "rbTemperature_Off", "rbTemperature_On", "rbTestCategory_Off",
    "rbTestCategory_On", "rgTestCategory",
};

// 存檔流程（FormClose）讀的替身 12 個 —— FileRW/IniConfig_CounterSel.cpp 用它決定頁面必須送哪些值
static const char* const kCSL_SaveReads[] = {
    "cbContactHeight", "cbCycleTime", "cbDefaultValue", "cbIndexTime", "cbUPH", "rbBinAssign_On",
    "rbContactCount_On", "rbLoadingCount_On", "rbScanner_On", "rbTemperature_On", "rbTestCategory_On", "rgTestCategory",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void CSL_CreateSaveProxies()
{
    EL<TCheckBox>("TfCounterSel", "cbContactHeight");
    EL<TCheckBox>("TfCounterSel", "cbCycleTime");
    EL<TCheckBox>("TfCounterSel", "cbDefaultValue");
    EL<TCheckBox>("TfCounterSel", "cbIndexTime");
    EL<TCheckBox>("TfCounterSel", "cbUPH");
    EL<TRadioButton>("TfCounterSel", "rbBinAssign_On");
    EL<TRadioButton>("TfCounterSel", "rbContactCount_On");
    EL<TRadioButton>("TfCounterSel", "rbLoadingCount_On");
    EL<TRadioButton>("TfCounterSel", "rbScanner_On");
    EL<TRadioButton>("TfCounterSel", "rbTemperature_On");
    EL<TRadioButton>("TfCounterSel", "rbTestCategory_On");
    EL<TRadioGroup>("TfCounterSel", "rgTestCategory");
}

// golden cCounterSel.cpp:23  TfCounterSel::TfCounterSel(TComponent* Owner)
static void CSL_TfCounterSel()
{

    NeedRef=false;

}

// golden cCounterSel.cpp:29  TfCounterSel::FormShow(TObject *Sender)
static void CSL_FormShow()
{

    ProcessLastSetIni_Visible(bReadFile);
    EL<TCheckBox>("TfCounterSel", "cbUPH")->Checked              = IniConfig.bShowUPH;
    EL<TCheckBox>("TfCounterSel", "cbIndexTime")->Checked        = IniConfig.bShowIndexTime;
    EL<TCheckBox>("TfCounterSel", "cbContactHeight")->Checked    = IniConfig.bShowContactHeight;   //Steven 20100827
    EL<TCheckBox>("TfCounterSel", "cbCycleTime")->Checked        = IniConfig.bShowTimeInfo;        //Steven 20100827
    EL<TRadioButton>("TfCounterSel", "rbLoadingCount_On")->Checked  = IniConfig.bShowLoaderCT;
    EL<TRadioButton>("TfCounterSel", "rbLoadingCount_Off")->Checked =!IniConfig.bShowLoaderCT;
    EL<TRadioButton>("TfCounterSel", "rbContactCount_On")->Checked  = IniConfig.bShowContactCT;
    EL<TRadioButton>("TfCounterSel", "rbContactCount_Off")->Checked =!IniConfig.bShowContactCT;
    EL<TRadioButton>("TfCounterSel", "rbTestCategory_On")->Checked  = IniConfig.bShowTestCate;
    EL<TRadioButton>("TfCounterSel", "rbTestCategory_Off")->Checked =!IniConfig.bShowTestCate;
    EL<TRadioGroup>("TfCounterSel", "rgTestCategory")->ItemIndex   = IniConfig.iShowCateByArm;
    EL<TRadioButton>("TfCounterSel", "rbScanner_On")->Checked       = IniConfig.bShowScanCate;
    EL<TRadioButton>("TfCounterSel", "rbScanner_Off")->Checked      =!IniConfig.bShowScanCate;
    EL<TRadioButton>("TfCounterSel", "rbTemperature_On")->Checked   = IniConfig.bShowTemper;
    EL<TRadioButton>("TfCounterSel", "rbTemperature_Off")->Checked  =!IniConfig.bShowTemper;
    EL<TRadioButton>("TfCounterSel", "rbBinAssign_On")->Checked     = IniConfig.bShowBinCT;
    EL<TRadioButton>("TfCounterSel", "rbBinAssign_Off")->Checked    =!IniConfig.bShowBinCT;

    NeedRef=false;
    ;   // golden cCounterSel.cpp:51-52 取代：Top／Left：視窗位置（網頁視窗位置由 background.html 管）
#if 0 // GATE (S12-C save) golden cCounterSel.cpp:51-52 -- Top／Left：視窗位置（網頁視窗位置由 background.html 管）
    Top=20;
    Left=250;
#endif // GATE (S12-C save)
    fShow=true;

}

// golden cCounterSel.cpp:56  TfCounterSel::FormClose(TObject *Sender, TCloseAction &Action)
static void CSL_FormClose()
{
    filerw::ELMark("FormClose");   // FileRW：存檔流程 trace（ack.trace）

    IniConfig.bShowUPH          =EL<TCheckBox>("TfCounterSel", "cbUPH")->Checked;
    IniConfig.bShowIndexTime    =EL<TCheckBox>("TfCounterSel", "cbIndexTime")->Checked;
    IniConfig.bShowTimeInfo     =EL<TCheckBox>("TfCounterSel", "cbCycleTime")->Checked;
    IniConfig.bShowContactHeight=EL<TCheckBox>("TfCounterSel", "cbContactHeight")->Checked;
    IniConfig.bShowLoaderCT     =EL<TRadioButton>("TfCounterSel", "rbLoadingCount_On")->Checked;
    IniConfig.bShowContactCT    =EL<TRadioButton>("TfCounterSel", "rbContactCount_On")->Checked;
    IniConfig.bShowScanCate     =EL<TRadioButton>("TfCounterSel", "rbScanner_On")->Checked;
    IniConfig.bShowTemper       =EL<TRadioButton>("TfCounterSel", "rbTemperature_On")->Checked;
    IniConfig.bShowTestCate     =EL<TRadioButton>("TfCounterSel", "rbTestCategory_On")->Checked;
    IniConfig.bShowBinCT        =EL<TRadioButton>("TfCounterSel", "rbBinAssign_On")->Checked;
    IniConfig.iShowCateByArm    =EL<TRadioGroup>("TfCounterSel", "rgTestCategory")->ItemIndex;

    //Steven 20100827 Start
    filerw::ELTodo("golden cCounterSel.cpp:72-75 fMain->StatusBar1 與 fShowMessage->lblIndexCycleTime／lblTestTime：主畫面元件，移植樹門面沒有（主畫面 UI）");
#if 0 // GATE (S12-C save) golden cCounterSel.cpp:72-75 -- fMain->StatusBar1 與 fShowMessage->lblIndexCycleTime／lblTestTime：主畫面元件，移植樹門面沒有（主畫面 UI）
    if(IniConfig.bShowIndexTime==false)
        fMain->StatusBar1->Panels->Items[0]->Text="";
    fShowMessage->lblIndexCycleTime->Visible=IniConfig.bShowTimeInfo;
    fShowMessage->lblTestTime->Visible=IniConfig.bShowTimeInfo;
#endif // GATE (S12-C save)

    fShowBinSelect->Tab_UPH->TabVisible=IniConfig.bShowUPH;

    if(IniConfig.bShowContactHeight==false)
    {
        fMain->lbArm0Torque->Caption="";
        fMain->lbArm0Torque->Caption="";
    }
    //Steven 20100827 End

    //jou 980721 start : Return Default Value
    if(EL<TCheckBox>("TfCounterSel", "cbDefaultValue")->Checked)
    {
        //Steven 20090811 : Use INI file to define form position
        EL<TCheckBox>("TfCounterSel", "cbDefaultValue")->Checked=false;
    filerw::ELTodo("golden cCounterSel.cpp:91-103 CheckFormIni＋ReplaceIniData：FormPos.def 是 BCB6 視窗位置，網頁不用，不寫（移植樹沒有 FormPos.def 的讀者）");
#if 0 // GATE (S12-C save) golden cCounterSel.cpp:91-103 -- CheckFormIni＋ReplaceIniData：FormPos.def 是 BCB6 視窗位置，網頁不用，不寫（移植樹沒有 FormPos.def 的讀者）
        AnsiString szDir=fCounterSel->CheckFormIni(AuthPath, "FormPos.def");
        ReplaceIniData(szDir, "fMain",            "X", "defaultX");
        ReplaceIniData(szDir, "fMain",            "Y", "defaultY");
        ReplaceIniData(szDir, "fSortCT",          "X", "defaultX");
        ReplaceIniData(szDir, "fSortCT",          "Y", "defaultY");
        ReplaceIniData(szDir, "fTestCategory",    "X", "defaultX");
        ReplaceIniData(szDir, "fTestCategory",    "Y", "defaultY");
        ReplaceIniData(szDir, "fShowBinSelect",   "X", "defaultX");
        ReplaceIniData(szDir, "fShowBinSelect",   "Y", "defaultY");
        ReplaceIniData(szDir, "fContactCT",       "X", "defaultX");
        ReplaceIniData(szDir, "fContactCT",       "Y", "defaultY");
        ReplaceIniData(szDir, "fLotInfo",         "X", "defaultX");
        ReplaceIniData(szDir, "fLotInfo",         "Y", "defaultY");
#endif // GATE (S12-C save)
    }
    //jou 980721 end
    ProcessLastSetIni_Visible(bWriteFile);
    NeedRef=true;
    fShow=false;

}

