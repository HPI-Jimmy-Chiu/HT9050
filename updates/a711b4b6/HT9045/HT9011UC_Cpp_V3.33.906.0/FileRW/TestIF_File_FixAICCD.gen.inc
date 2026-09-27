// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_FixAICCD 的讀寫檔（HTEditList： → <DataPath>\<配方>\HandlerCondition.Data [Configuration] Fix2 AI …（寫：spbSaveClick；讀：移植樹 forms/fFixAICCD.cpp ReadFile）、D:\HT9045\system\Gerneral.ini [AICCD] LockNoWaitAOIResult（讀：DoIniDataToForm；CheckAndReadIniDataGeneral 缺鍵補寫））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\FixAICCD.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden FixAICCD.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 15 個（header 宣告 52 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "database.h"
#include "forms/fFixAICCD.h"
#include "forms/fMain.h"


using filerw::EL;

// 前置宣告
static void FX_DoIniDataToForm();
static void FX_spbSaveClick();

// golden FixAICCD.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void FX_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfFixAICCD", "rgFixAICCD")->Items;
      s->Add("OK Devicec");
      s->Add("NG Device");
    }
    { TStringList* s = EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->Items;
      s->Add("Normal");
      s->Add("Do not wait for AOI results");
      s->Add("Show Damage tray Mapping");
    }
}

// golden FixAICCD.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kFX_ParentOf[][2] = {
    {"GroupBox2", "Panel2"},
    {"Panel2", "tsTrayCCD"},
    {"Panel4", "tsTraySetting"},
    {"chkEnableFix2AICCD", "Panel4"},
    {"chkEnableLearningMode", "tsTraySetting"},
    {"edFix2AICCD_Address", "GroupBox2"},
    {"edFix2AICCD_Address2", "GroupBox2"},
    {"edtFix2AICCD_Port", "GroupBox2"},
    {"edtFix2AICCD_Port2", "GroupBox2"},
    {"edtInspectResultThres", "tsTraySetting"},
    {"edtOutArmCycleInsp", "tsTraySetting"},
    {"edtlFix2AICCDAutoRetry", "tsTraySetting"},
    {"edtlFix2AICCDExposureTimeOut", "tsTraySetting"},
    {"edtlFix2AICCDGetResultTimeOut", "tsTraySetting"},
    {"edtlFix2AICCDStartDelay", "tsTraySetting"},
    {"lblBGALightValue", "GroupBox2"},
    {"spbSave", "Panel1"},
    {"tsTrayCCD", "PageControl1"},
    {"tsTraySetting", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void FX_CreateContainerProxies()
{
    EL<TGroupBox>("TfFixAICCD", "GroupBox2");
    EL<TPageControl>("TfFixAICCD", "PageControl1");
    EL<TPanel>("TfFixAICCD", "Panel1");
    EL<TPanel>("TfFixAICCD", "Panel2");
    EL<TPanel>("TfFixAICCD", "Panel4");
    EL<TTabSheet>("TfFixAICCD", "tsTrayCCD");
    EL<TTabSheet>("TfFixAICCD", "tsTraySetting");
    filerw::ELSetParents("TfFixAICCD", kFX_ParentOf, (int)(sizeof(kFX_ParentOf) / sizeof(kFX_ParentOf[0])));
}

// golden FixAICCD.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void FX_DfmState()
{
    EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD")->Checked = true;
    EL<TCheckBox>("TfFixAICCD", "chkEnableLearningMode")->Checked = true;
    EL<TEdit>("TfFixAICCD", "edFix2AICCD_Address")->Text = "127.000.000.100";
    EL<TEdit>("TfFixAICCD", "edFix2AICCD_Address2")->Text = "127.000.000.100";
    EL<TEdit>("TfFixAICCD", "edtFix2AICCD_Port")->Text = "1000";
    EL<TEdit>("TfFixAICCD", "edtFix2AICCD_Port2")->Text = "1000";
    EL<TEdit>("TfFixAICCD", "edtInspectResultThres")->Text = "0.5";
    EL<TEdit>("TfFixAICCD", "edtOutArmCycleInsp")->Text = "1";
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDAutoRetry")->Text = "1";
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDExposureTimeOut")->Text = "10";
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDGetResultTimeOut")->Text = "100000";
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDStartDelay")->Text = "10";
    EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kFX_EnableAll[] = {
    "GroupBox2", "PageControl1", "Panel1", "Panel2", "Panel4", "chkEnableFix2AICCD",
    "chkEnableLearningMode", "lblBGALightValue", "rgResultShowType", "spbSave", "tsTrayCCD", "tsTraySetting",
};

// 存檔流程（spbSaveClick）讀的替身 10 個 —— FileRW/TestIF_File_FixAICCD.cpp 用它決定頁面必須送哪些值
static const char* const kFX_SaveReads[] = {
    "chkEnableFix2AICCD", "chkEnableLearningMode", "edtInspectResultThres", "edtOutArmCycleInsp", "edtlFix2AICCDAutoRetry", "edtlFix2AICCDExposureTimeOut",
    "edtlFix2AICCDGetResultTimeOut", "edtlFix2AICCDStartDelay", "lblBGALightValue", "rgResultShowType",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void FX_CreateSaveProxies()
{
    EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD");
    EL<TCheckBox>("TfFixAICCD", "chkEnableLearningMode");
    EL<TEdit>("TfFixAICCD", "edtInspectResultThres");
    EL<TEdit>("TfFixAICCD", "edtOutArmCycleInsp");
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDAutoRetry");
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDExposureTimeOut");
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDGetResultTimeOut");
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDStartDelay");
    EL<TLabel>("TfFixAICCD", "lblBGALightValue");
    EL<TRadioGroup>("TfFixAICCD", "rgResultShowType");
}

// golden FixAICCD.cpp:77  TfFixAICCD::DoIniDataToForm()
static void FX_DoIniDataToForm()
{

    EL<TEdit>("TfFixAICCD", "edFix2AICCD_Address")->Text               =HSys.asFix2BGAAICCDIP[0];
    EL<TEdit>("TfFixAICCD", "edtFix2AICCD_Port")->Text                 =HSys.asFix2BGAAICCDPort[0];
    EL<TEdit>("TfFixAICCD", "edFix2AICCD_Address2")->Text              =HSys.asFix2BGAAICCDIP[1];
    EL<TEdit>("TfFixAICCD", "edtFix2AICCD_Port2")->Text                =HSys.asFix2BGAAICCDPort[1];
    EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD")->Checked             =TestIF_File.bEnableFix2BGAAICCD;
    EL<TCheckBox>("TfFixAICCD", "chkEnableLearningMode")->Checked          =TestIF_File.bEnableLearningMode;
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDStartDelay")->Text           =TestIF_File.iFix2BGAAICCDStartDelay;
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDExposureTimeOut")->Text      =TestIF_File.iFix2BGAAICCDExposureTimeOut;
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDGetResultTimeOut")->Text     =TestIF_File.iFix2BGAAICCDGetResultTimeOut;
    EL<TEdit>("TfFixAICCD", "edtlFix2AICCDAutoRetry")->Text            =TestIF_File.iFix2BGAAICCDAutoRetry;
    EL<TEdit>("TfFixAICCD", "edtOutArmCycleInsp")->Text                =TestIF_File.iFix2BGAAICCDOutArmCycleInsp;
    EL<TEdit>("TfFixAICCD", "edtInspectResultThres")->Text             =TestIF_File.dInspectResultThres;
    EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->ItemIndex             =TestIF_File.iResultShowType;       //Sam 20240325 : 新增 DamageTrayMapping 功能
    EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->Enabled=(CheckAndReadIniDataGeneral("AICCD", "LockNoWaitAOIResult",  1)==1)?false:true;

}

// golden FixAICCD.cpp:669  TfFixAICCD::spbSaveClick(TObject *Sender)
static void FX_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);

    filerw::ELMark("FX_WriteIniData"); WriteIniData(szDir, "Configuration", "Fix2 AI CCD Enable",                  (EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD")->Checked)?1:0);   // golden FixAICCD.cpp:676-676 取代：存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）
#if 0 // GATE (S12-C save) golden FixAICCD.cpp:676-676 -- 存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD Enable",                  (EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD")->Checked)?1:0);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "Configuration", "Fix2 AI Learning Mode",               (EL<TCheckBox>("TfFixAICCD", "chkEnableLearningMode")->Checked)?1:0);
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD Start Delay",             EL<TEdit>("TfFixAICCD", "edtlFix2AICCDStartDelay")->Text);
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD Exposure Time Out",       EL<TEdit>("TfFixAICCD", "edtlFix2AICCDExposureTimeOut")->Text);
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD Get Result Time Out",     EL<TEdit>("TfFixAICCD", "edtlFix2AICCDGetResultTimeOut")->Text);
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD Auto Retry",              EL<TEdit>("TfFixAICCD", "edtlFix2AICCDAutoRetry")->Text);
    WriteIniData(szDir, "Configuration", "Fix2 AI OutArmCycleInsp",             EL<TEdit>("TfFixAICCD", "edtOutArmCycleInsp")->Text);
    WriteIniData(szDir, "Configuration", "Fix2 AI InspectResultThres",          EL<TEdit>("TfFixAICCD", "edtInspectResultThres")->Text);

    //光源
    WriteIniData(szDir, "Configuration", "Fix2 AI CCD BGA Light Value",        EL<TLabel>("TfFixAICCD", "lblBGALightValue")->Caption);

    WriteIniData(szDir, "Configuration", "Fix2 AI iResultShowType", EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->ItemIndex);   //Sam 20240325 : 新增 DamageTrayMapping 功能
    fFixAICCD->ReadFile();   // golden FixAICCD.cpp:689-689 取代：golden ReadFile（:95）：移植樹 forms/fFixAICCD.cpp 已逐字翻好（開機鏈 golden :9370 也叫它）→ 叫同一支，全樹一份讀檔器。⚠ 它尾巴的 ChangeFix2AICCDSetupFile 在移植樹把捲軸那行 GATE 掉（G-FX-BGALIGHT），golden 那一行在 InitialOK 之後會經 OnChange 送 COM2 光源控制（外部設備，Jimmy）
#if 0 // GATE (S12-C save) golden FixAICCD.cpp:689-689 -- golden ReadFile（:95）：移植樹 forms/fFixAICCD.cpp 已逐字翻好（開機鏈 golden :9370 也叫它）→ 叫同一支，全樹一份讀檔器。⚠ 它尾巴的 ChangeFix2AICCDSetupFile 在移植樹把捲軸那行 GATE 掉（G-FX-BGALIGHT），golden 那一行在 InitialOK 之後會經 OnChange 送 COM2 光源控制（外部設備，Jimmy）
    ReadFile();
#endif // GATE (S12-C save)
    EL<TSpeedButton>("TfFixAICCD", "spbSave")->Down=false;
    fMain->BackupSetupFile();    //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

