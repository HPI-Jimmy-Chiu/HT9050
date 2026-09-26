// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 BinSelect 的讀寫檔（HTEditList： → Binasgn.Data、BinasgnOff.Data、BinasgnOff-Line.Data、Binasgn_ART.Data、BinasgnOff_ART.Data、Binasgn_MRT.Data、Binasgn_MRT_RT.Data、HandlerCondition.Data）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cBinSel.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cBinSel.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 35 個（header 宣告 77 個）。
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
#include "forms/fBinSel.h"
#include "forms/fShowBinSelect.h"
#include "forms/fTrayAssignment.h"
#include "forms/fLotInfo.h"
#include "ProductionInfo/FileInfo.h"
#include "csystem.h"
#include "Motor/mymotor.h"
#include "canary_support.h"

extern bool authBinSetting[8];         // cAuthority.h:63（golden cAuthority.h）
extern bool bAuthCriticalPara[26];     // cAuthority.h:64
void GetBinSettingAuth();              // cAuthority.h:81
bool FileRW_OutArmSuckHasIC();         // FileRW/_KitSuck.cpp（golden OutArmSuck.HasIC()）
// golden cBinSel.cpp:53-86 的 TU 內 enum eBinSettingItems（只用到這三個；eBinNotUse=25，移植樹 cBinSel.cpp:111 同值）
enum { eBinNotUse=25, eBinSetting=26, eBinSetTotal=(26+eTrayCount) };
// golden systools.h TFormSysTools：TIniFile 逐鍵直寫。移植樹沒有 → 同名 TU 私用類別，多載集合照 golden（systools.h:21-35），
// 轉給帶檔名的全域 WriteIniData（common.cpp:1044-1265；golden 的 change log 在移植樹全域 WriteIniData 裡是 GATE）。
namespace {
struct BS_TFormSysTools {
    AnsiString FileName;
    void OpenFormData(AnsiString Filename) { FileName=Filename; }                 // golden systools.cpp:17 new TIniFile
    void CloseFormData() { FileName=""; }                                           // golden systools.cpp:22 delete
    void WriteIniData(AnsiString G, AnsiString N, bool v)         { ::WriteIniData(FileName, G, N, v); }
    void WriteIniData(AnsiString G, AnsiString N, int v)          { ::WriteIniData(FileName, G, N, v); }
    void WriteIniData(AnsiString G, AnsiString N, unsigned int v) { ::WriteIniData(FileName, G, N, (int)v); }   // golden :119 WriteInteger(Value)
    void WriteIniData(AnsiString G, AnsiString N, double v)       { ::WriteIniData(FileName, G, N, v); }
    void WriteIniData(AnsiString G, AnsiString N, AnsiString v)   { ::WriteIniData(FileName, G, N, v); }
    void WriteIniData(AnsiString G, AnsiString N, TDateTime v)    { ::WriteIniData(FileName, G, N, v); }
};
BS_TFormSysTools BS_FormSysToolsObj;
BS_TFormSysTools* const FormSysTools=&BS_FormSysToolsObj;
}  // namespace

// golden cBinSel.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bShow=false;             // golden cBinSel.h:231 TfBinSel::bShow
static bool bSaveBin=false;          // golden cBinSel.h:180
static bool bCheckOffLineLevel=false; // golden cBinSel.h:181
static int  iMode=0;                 // golden cBinSel.h:182（只有 FormShow 用；ReadWriteMRTMode 的參數同名、遮蔽它，與 golden 相同）
static bool bCountSetPassed=false;   // golden cBinSel.h:207
static bool bBinPanelEnabled[eBinTypeTotal]={};   // golden MyBinPanel[tag]->Panel->Enabled（FormShow :1726-1779）
#define MyBinPanel (fBinSel->MyBinPanel)                                   // golden cBinSel.h（vector<TMyBinPanel*>）→ 移植樹 TMyBinPanelData*[7]
#define sBinDoubleContact (fBinSel->sBinDoubleContact)                     // golden cBinSel.h:235
#define sBinConsFail (fBinSel->sBinConsFail)
#define sBinEnableFail (fBinSel->sBinEnableFail)
#define sBinFailPercent (fBinSel->sBinFailPercent)
#define sBinFailIgnore (fBinSel->sBinFailIgnore)
#define sBinCountEnable (fBinSel->sBinCountEnable)
#define sBinCountIgnore (fBinSel->sBinCountIgnore)
#define sBinCountNumber (fBinSel->sBinCountNumber)
#define sSpecialBinByArm (fBinSel->sSpecialBinByArm)
#define sSpecialBinCountByArm (fBinSel->sSpecialBinCountByArm)
#define sSpecialBinBySocket (fBinSel->sSpecialBinBySocket)
#define sSpecialBinCountBySocket (fBinSel->sSpecialBinCountBySocket)
#define sLowYield (fBinSel->sLowYield)
#define sArmYield (fBinSel->sArmYield)
#define sSiteYield (fBinSel->sSiteYield)
#define sBinTraySetT3Pos (fBinSel->sBinTraySetT3Pos)
#define sBinTraySetT3PosName (fBinSel->sBinTraySetT3PosName)
#define sBinType (fBinSel->sBinType)
#define sBySiteClean (fBinSel->sBySiteClean)
#define sByBinClean (fBinSel->sByBinClean)
#define sT3TrayType (fBinSel->sT3TrayType)
#define sT6Retest (fBinSel->sT6Retest)
#define sT3CateR (fBinSel->sT3CateR)
#define sSpecBinBySiteCompareEnable (fBinSel->sSpecBinBySiteCompareEnable)
#define sSpecBinBySiteCompareIgnore (fBinSel->sSpecBinBySiteCompareIgnore)
#define sSpecBinBySiteComparePercent (fBinSel->sSpecBinBySiteComparePercent)
#define sSpecBinByArmPerSiteCompareEnable (fBinSel->sSpecBinByArmPerSiteCompareEnable)
#define sSpecBinByArmPerSiteCompareIgnore (fBinSel->sSpecBinByArmPerSiteCompareIgnore)
#define sSpecBinByArmPerSiteComparePercent (fBinSel->sSpecBinByArmPerSiteComparePercent)
#define sAOIBinTraySetting (fBinSel->sAOIBinTraySetting)
#define sBinTrayLinked (fBinSel->sBinTrayLinked)
#define sBinLinked (fBinSel->sBinLinked)
#define sMagazineSetup (fBinSel->sMagazineSetup)
#define ReadFile(a, b, c) fBinSel->ReadFile(a, b, c)     // golden :1121 → 移植樹 cBinSel.cpp:1411（live；寫入在移植樹是 GATE）
#define ReadParam() fBinSel->ReadParam()                 // golden :2128
#define CheckFix2Tray() fBinSel->CheckFix2Tray()         // golden :4833
#define CheckOSBin() fBinSel->CheckOSBin()               // golden :6302
#define ARTBinCheck(t) fBinSel->ARTBinCheck(t)           // golden :6429
#define TransferBinTrayStrToName(t) fBinSel->TransferBinTrayStrToName(t)   // golden :6452

using filerw::EL;

// 前置宣告
static void BS_TfBinSel();
static void BS_FormShow();
static void BS_ShowChangeBinMessage();
static void BS_cbTestModeChange();
static void BS_ReadPrimeDara();
static void BS_SetPrimeButton();
static void BS_spbSaveClick();
static void BS_SaveOther(AnsiString szDir);
static void BS_SaveFunctionData(int tag);
static void BS_ReadWriteMRTMode(int iMode);
static void BS_ReadWriteSpecialFunction(bool bReadWrite);
static void BS_btnSettingSpecificBinClick();

// golden cBinSel.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void BS_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfBinSel", "cbTestMode")->Items;
      s->Add("Normal");
      s->Add("Re-Test");
    }
    { TStringList* s = EL<TComboBox>("TfBinSel", "cbbASMPassBin")->Items;
      s->Add("None");
      s->Add("Bin0");
      s->Add("Bin1");
      s->Add("Bin2");
      s->Add("Bin3");
      s->Add("Bin4");
      s->Add("Bin5");
      s->Add("Bin6");
      s->Add("Bin7");
      s->Add("Bin8");
      s->Add("Bin9");
      s->Add("Bin10");
      s->Add("Bin11");
      s->Add("Bin12");
      s->Add("Bin13");
      s->Add("Bin14");
      s->Add("Bin15");
    }
    { TStringList* s = EL<TComboBox>("TfBinSel", "cbbAutoSiteMap")->Items;
      s->Add("None");
      s->Add("Bin0");
      s->Add("Bin1");
      s->Add("Bin2");
      s->Add("Bin3");
      s->Add("Bin4");
      s->Add("Bin5");
      s->Add("Bin6");
      s->Add("Bin7");
      s->Add("Bin8");
      s->Add("Bin9");
      s->Add("Bin10");
      s->Add("Bin11");
      s->Add("Bin12");
      s->Add("Bin13");
      s->Add("Bin14");
      s->Add("Bin15");
    }
    { TStringList* s = EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->Items;
      s->Add("[1] Normal");
      s->Add("[2] BinBox");
    }
}

// golden cBinSel.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kBS_ParentOf[][2] = {
    {"BulkBox", "Panel1"},
    {"CancelErrorBin", "Panel1"},
    {"LabBulkBox", "BulkBox"},
    {"Label1", "Panel1"},
    {"Panel", "tsNormal"},
    {"btnSetAll2NotUse", "Panel1"},
    {"btnSettingSpecificBin", "Panel1"},
    {"cbIndexDropErrSetErrUntilOneCycle", "palSpecificBin"},
    {"cbOutShtLoseICSetErrUntilOneCycle", "palSpecificBin"},
    {"cbTestMode", "Panel1"},
    {"cbUseMRTMode", "Panel1"},
    {"cbbASMPassBin", "pnlAutoSiteMap"},
    {"cbbAutoSiteMap", "pnlAutoSiteMap"},
    {"chkShow0Xbin", "pnlAutoSiteMap"},
    {"edControlBinCheckPoint", "Panel1"},
    {"ed_FixBinBoxAlarmCount", "BulkBox"},
    {"labASMPassBin", "pnlAutoSiteMap"},
    {"labWarning", "Panel1"},
    {"mtBinSelectBy", "Panel"},
    {"palSpecificBin", "tsNormal"},
    {"pnlAutoSiteMap", "Panel1"},
    {"rg_FixBinBox", "BulkBox"},
    {"sgSpecificBin", "palSpecificBin"},
    {"spbAOIBin", "Panel1"},
    {"spbNormal", "Panel1"},
    {"spbPrime", "Panel1"},
    {"spbSave", "Panel1"},
    {"tsArtFT", "PageControl1"},
    {"tsArtRT", "PageControl1"},
    {"tsMrtFT", "PageControl1"},
    {"tsMrtRT", "PageControl1"},
    {"tsNormal", "PageControl1"},
    {"tsOffline", "PageControl1"},
    {"tsRetest", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void BS_CreateContainerProxies()
{
    EL<TPanel>("TfBinSel", "BulkBox");
    EL<TPageControl>("TfBinSel", "PageControl1");
    EL<TPanel>("TfBinSel", "Panel");
    EL<TPanel>("TfBinSel", "Panel1");
    EL<TPanel>("TfBinSel", "palSpecificBin");
    EL<TPanel>("TfBinSel", "pnlAutoSiteMap");
    EL<TTabSheet>("TfBinSel", "tsNormal");
    filerw::ELSetParents("TfBinSel", kBS_ParentOf, (int)(sizeof(kBS_ParentOf) / sizeof(kBS_ParentOf[0])));
}

// golden cBinSel.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void BS_DfmState()
{
    EL<TPanel>("TfBinSel", "BulkBox")->Visible = false;
    EL<TCheckBox>("TfBinSel", "CancelErrorBin")->Visible = false;
    EL<TPanel>("TfBinSel", "Panel")->Visible = false;
    EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Visible = false;
    EL<TComboBox>("TfBinSel", "cbbASMPassBin")->Visible = false;
    EL<TCheckBox>("TfBinSel", "chkShow0Xbin")->Visible = false;
    filerw::ELSetReadOnly("TfBinSel", "edControlBinCheckPoint");
    EL<TLabel>("TfBinSel", "labASMPassBin")->Visible = false;
    EL<TPanel>("TfBinSel", "palSpecificBin")->Visible = false;
    EL<TPanel>("TfBinSel", "pnlAutoSiteMap")->Visible = false;
    EL<TSpeedButton>("TfBinSel", "spbNormal")->Visible = false;
    EL<TSpeedButton>("TfBinSel", "spbPrime")->Visible = false;
    EL<TComboBox>("TfBinSel", "cbTestMode")->Text = "Normal";
    EL<TComboBox>("TfBinSel", "cbTestMode")->ItemIndex = 0;
    EL<TComboBox>("TfBinSel", "cbbASMPassBin")->Text = "None";
    EL<TComboBox>("TfBinSel", "cbbASMPassBin")->ItemIndex = 0;
    EL<TComboBox>("TfBinSel", "cbbAutoSiteMap")->Text = "None";
    EL<TComboBox>("TfBinSel", "cbbAutoSiteMap")->ItemIndex = 0;
    EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Text = "0";
    EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text = "0";
    EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kBS_EnableAll[] = {
    "BulkBox", "CancelErrorBin", "LabBulkBox", "Label1", "PageControl1", "Panel",
    "Panel1", "btnSetAll2NotUse", "btnSettingSpecificBin", "cbIndexDropErrSetErrUntilOneCycle", "cbOutShtLoseICSetErrUntilOneCycle", "cbUseMRTMode",
    "chkShow0Xbin", "labASMPassBin", "labWarning", "palSpecificBin", "pnlAutoSiteMap", "rg_FixBinBox",
    "spbAOIBin", "spbNormal", "spbPrime", "spbSave", "tsArtFT", "tsArtRT",
    "tsMrtFT", "tsMrtRT", "tsNormal", "tsOffline", "tsRetest",
};

// 存檔流程（spbSaveClick、SaveOther、SaveFunctionData、ReadWriteMRTMode、ReadWriteSpecialFunction）讀的替身 10 個 —— FileRW/BinSelect.cpp 用它決定頁面必須送哪些值
static const char* const kBS_SaveReads[] = {
    "CancelErrorBin", "cbIndexDropErrSetErrUntilOneCycle", "cbOutShtLoseICSetErrUntilOneCycle", "cbUseMRTMode", "cbbASMPassBin", "cbbAutoSiteMap",
    "chkShow0Xbin", "ed_FixBinBoxAlarmCount", "rg_FixBinBox", "sgSpecificBin",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void BS_CreateSaveProxies()
{
    EL<TCheckBox>("TfBinSel", "CancelErrorBin");
    EL<TCheckBox>("TfBinSel", "cbIndexDropErrSetErrUntilOneCycle");
    EL<TCheckBox>("TfBinSel", "cbOutShtLoseICSetErrUntilOneCycle");
    EL<TCheckBox>("TfBinSel", "cbUseMRTMode");
    EL<TComboBox>("TfBinSel", "cbbASMPassBin");
    EL<TComboBox>("TfBinSel", "cbbAutoSiteMap");
    EL<TCheckBox>("TfBinSel", "chkShow0Xbin");
    EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount");
    EL<TRadioGroup>("TfBinSel", "rg_FixBinBox");
    EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin");
}

// golden cBinSel.cpp:971  TfBinSel::TfBinSel(TComponent* Owner)
static void BS_TfBinSel()
{

    ;   // golden cBinSel.cpp:974-1078 取代：7 個 TMyBinPanel（GUI）與 30＋組 sXxx TStringList 的 new／初值：移植樹 fBinSel 的建構子已建（forms/fBinSel.h，MyBinPanel＝TMyBinPanelData）——共用那一份，不另建（Steven 20260925）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:974-1078 -- 7 個 TMyBinPanel（GUI）與 30＋組 sXxx TStringList 的 new／初值：移植樹 fBinSel 的建構子已建（forms/fBinSel.h，MyBinPanel＝TMyBinPanelData）——共用那一份，不另建（Steven 20260925）
    MyBinPanel.push_back(new TMyBinPanel(this, 0, EL<TTabSheet>("TfBinSel", "tsRetest"), "RT"));
    MyBinPanel.push_back(new TMyBinPanel(this, 1, EL<TTabSheet>("TfBinSel", "tsNormal"), "FT"));
    MyBinPanel.push_back(new TMyBinPanel(this, 2, EL<TTabSheet>("TfBinSel", "tsOffline"), "OffLine"));
    MyBinPanel.push_back(new TMyBinPanel(this, 3, EL<TTabSheet>("TfBinSel", "tsArtRT"), "RT_ART"));          //ChungHung 20141002 add for KYEC AutoRetest
    MyBinPanel.push_back(new TMyBinPanel(this, 4, EL<TTabSheet>("TfBinSel", "tsArtFT"), "FT_ART"));          //ChungHung 20141002 add for KYEC AutoRetest
    MyBinPanel.push_back(new TMyBinPanel(this, 5, EL<TTabSheet>("TfBinSel", "tsMrtRT"), "RT_MRT"));          //Ifor 20170316 (wei) Add MRT Mode
    MyBinPanel.push_back(new TMyBinPanel(this, 6, EL<TTabSheet>("TfBinSel", "tsMrtFT"), "FT_MRT"));          //Ifor 20170316 (wei) Add MRT Mode

    EL<TPanel>("TfBinSel", "Panel")->Visible=false;
    bMouseDown=false;
    bAskCountPassword=false;                                                    //AI(rf360-yield-password) 20260814 (RogerYang)

    for(int i=0; i<eBinTypeTotal; i++)                                          //Steven 20140510 : Secs Gem
    {
        sBinDoubleContact[i]=new TStringList();
        sBinConsFail[i]     =new TStringList();
        sBinEnableFail[i]   =new TStringList();
        sBinFailPercent[i]  =new TStringList();

        sBinFailIgnore[i]   =new TStringList();                                 //Steven 20140529 Start: Fail Persent & Count同時存在
        sBinCountEnable[i]  =new TStringList();
        sBinCountIgnore[i]  =new TStringList();
        sBinCountNumber[i]  =new TStringList();

        sSpecialBinByArm[i]         =new TStringList();                         //ChungHung 20140730 add ContinuousFailHaveOneCycle start
        sSpecialBinCountByArm[i]    =new TStringList();
        sSpecialBinBySocket[i]      =new TStringList();
        sSpecialBinCountBySocket[i] =new TStringList();

        sLowYield[i]   =new TStringList();                                      //Steven 20140828 : By Bin Yield Monitor
        sArmYield[i]   =new TStringList();                                      //Steven 20140828 : By Bin Arm Yield Monitor
        sSiteYield[i]  =new TStringList();                                      //Steven 20140828 : By Bin Site Yield Monitor

        sBinTraySetT3PosName[i]=new TStringList();                              //Steven 20241205 : Bin to tray增加文字顯示
        sBinTraySetT3Pos[i] =new TStringList();
        sBinType[i]         =new TStringList();
        sT3TrayType[i]      =new TStringList();
        sT6Retest[i]        =new TStringList();
        sBySiteClean[i]     =new TStringList();                                 //Steven 20160308 : By Bin count do auto clean
        sByBinClean[i]      =new TStringList();                                 //Steven 20160308 : By Bin pre site count do auto clean
        sT3CateR[i]         =new TStringList();                                 //Steven 20161221 (wei) : for SCK only Auto 2 has ART

        sSpecBinBySiteCompareEnable[i]       =new TStringList();                //JerryYang 20170712 (Steven) by site by bin compare percent
        sSpecBinBySiteCompareIgnore[i]       =new TStringList();
        sSpecBinBySiteComparePercent[i]      =new TStringList();
        sSpecBinByArmPerSiteCompareEnable[i] =new TStringList();
        sSpecBinByArmPerSiteCompareIgnore[i] =new TStringList();
        sSpecBinByArmPerSiteComparePercent[i]=new TStringList();
        sBinTrayLinked[i]   =new TStringList();                                 //Ifor 20240730 add : BinTrayLinked
        sBinLinked[i]       =new TStringList();                                 //Ifor 20240730 add : BinTrayLinked
        sMagazineSetup[i]   =new TStringList();                                 //Ifor 20240730 add : BinTrayLinked

        sAOIBinTraySetting[i]   =new TStringList();
        sAOIBinTraySetting[i]   ->Add("0");

        for(int j=0; j<iTestBinCount; j++)
        {
            sBinDoubleContact[i]->Add("0");
            sBinConsFail[i]     ->Add("0");
            sBinEnableFail[i]   ->Add("0");
            sBinFailPercent[i]  ->Add("0");

            sBinFailIgnore[i]   ->Add("0");                                     //Steven 20140529 Start: Fail Persent & Count同時存在
            sBinCountEnable[i]  ->Add("0");
            sBinCountIgnore[i]  ->Add("0");
            sBinCountNumber[i]  ->Add("0");

            sSpecialBinByArm[i]         ->Add("0");                             //ChungHung 20140730 add ContinuousFailHaveOneCycle start
            sSpecialBinCountByArm[i]    ->Add("0");
            sSpecialBinBySocket[i]      ->Add("0");
            sSpecialBinCountBySocket[i] ->Add("0");

            sLowYield[i]   ->Add("0");                                          //Steven 20140828 : By Bin Yield Monitor
            sArmYield[i]   ->Add("0");                                          //Steven 20140828 : By Bin Arm Yield Monitor
            sSiteYield[i]  ->Add("0");                                          //Steven 20140828 : By Bin Site Yield Monitor

            sBinTraySetT3PosName[i] ->Add("NotUse");                            //Steven 20241205 : Bin to tray增加文字顯示
            sBinTraySetT3Pos[i]     ->Add("0");
            sBinType[i]             ->Add("0");
            sBySiteClean[i]         ->Add("0");                                 //Steven 20160308 : By Bin count do auto clean
            sByBinClean[i]          ->Add("0");                                 //Steven 20160308 : By Bin pre site count do auto clean

            sSpecBinBySiteCompareEnable[i]->Add("0");                           //JerryYang 20170712 (Steven) by site by bin compare percent
            sSpecBinBySiteCompareIgnore[i]->Add("0");
            sSpecBinBySiteComparePercent[i]->Add("0");
            sSpecBinByArmPerSiteCompareEnable[i]->Add("0");
            sSpecBinByArmPerSiteCompareIgnore[i]->Add("0");
            sSpecBinByArmPerSiteComparePercent[i]->Add("0");

            sBinTrayLinked[i]   ->Add("0");                                     //Ifor 20240730 add : BinTrayLinked
            sBinLinked[i]       ->Add("0");                                     //Ifor 20240730 add : BinTrayLinked
            sMagazineSetup[i]   ->Add("0");                                     //Ifor 20240730 add : BinTrayLinked
        }

        for(int j=e3Auto1; j<eTrayCount; j++)
        {
            sT3TrayType[i]->Add("0");
            sT3CateR[i]->Add("0");                                              //Steven 20161221 (wei) : for SCK only Auto 2 has ART
        }

        for(int j=eAuto1; j<=iAutoRight; j++)
        {
            sT6Retest[i]->Add("0");
        }
    }
#endif // GATE (S12-C save)

    if(CosFunction.bDisableRTBinSet)                                            //wei 20150622 不顯示設定RT Bin set
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Delete(1);

    if(CosFunction.bOffLineBin || USE_AUTO_RETEST==eartInstall)                 //ChungHung 20141002 add for KYEC AutoRetest
    {
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Add("Off-Line");
    }

    if(CosFunction.bUseSCKART==false && USE_AUTO_RETEST==eartInstall)           //ChungHung 20141002 add for KYEC AutoRetest
    {
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Add("ART Re-Test");
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Add("ART Normal");
    }

    if(CosFunction.bUseMRTMode==true)                                           //Ifor 20170316 (wei) Add MRT Mode
    {
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Add("MRT Re-Test");
        EL<TComboBox>("TfBinSel", "cbTestMode")->Items->Add("MRT Normal");
    }

    ;   // golden cBinSel.cpp:1100-1118 取代：MyBinPanel[tag]->mtBinSelect（TTMyTray 格子）寬度／XItem、KYEC 面板底色：純畫面
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1100-1118 -- MyBinPanel[tag]->mtBinSelect（TTMyTray 格子）寬度／XItem、KYEC 面板底色：純畫面
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
        MyBinPanel[tag]->mtBinSelect->Width=iColWidth*iTestBinCount+10;
        MyBinPanel[tag]->mtBinSelect->XItem=iTestBinCount;
        MyBinPanel[tag]->InitmtBinSelectData();

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20170417 (wei) add Bin 設定頁面底色
        {
            if(tag==eBinRT_ART || tag==eBinFT_ART)                              //Ifor 20170417 add KYEC 喬智要求 ART Mode Bin設定頁面，底色需與7000相同變更修改為0x00B5FFB5
            {
                ;   // UI-only（HTML 端處理）：MyBinPanel[tag]->Panel->Color=TColor(0x00B5FFB5);
            }

            if(tag==eBinRT_MRT || tag==eBinFT_MRT)                              //Ifor 20170417 add KYEC 喬智要求 MRT Mode Bin設定頁面，底色需與7000相同變更修改為0x00E0C1FF
            {
                ;   // UI-only（HTML 端處理）：MyBinPanel[tag]->Panel->Color=TColor(0x00E0C1FF);
            }
        }
    }
#endif // GATE (S12-C save)

}

// golden cBinSel.cpp:1700  TfBinSel::FormShow(TObject *Sender)
static void BS_FormShow()
{

    bool bFlag;
    bShow=false;
    bCountSetPassed=false;                                                      //AI(rf360-yield-password) 20260814 (RogerYang) : 每次開窗重新要求密碼
    bSaveBin=false;
    AnsiString S;
    int iBin=0;//, iT6;                                                         //Sam 20200622 : Control Bin 顯示

    fTrayAssignment->FixCanUse();

    ;   // golden cBinSel.cpp:1711-1718 取代：MyBinPanel[tag]->mtBinSelect／sbBinSetScroll 初始化（TTMyTray 格子與捲軸）：純畫面
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1711-1718 -- MyBinPanel[tag]->mtBinSelect／sbBinSetScroll 初始化（TTMyTray 格子與捲軸）：純畫面
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
        MyBinPanel[tag]->mtBinSelect->Width=iColWidth*iTestBinCount+10;
        MyBinPanel[tag]->mtBinSelect->XItem=iTestBinCount;
        MyBinPanel[tag]->InitmtBinSelectData();
        MyBinPanel[tag]->sbBinSetScroll->Max        =MyBinPanel[tag]->sbBinSetting->HorzScrollBar->Range;   //Steven 20240420 : 調整scrollbar位置
        MyBinPanel[tag]->sbBinSetScroll->PageSize   =600;
    }
#endif // GATE (S12-C save)

    ReadParam();
    BS_ReadPrimeDara();
    ReadFile(false, false, "");                                                 //kevin 20160928

    ;   // golden cBinSel.cpp:1724-1725 取代：Caption（視窗標題 Contact Parameter ...）：純畫面
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1724-1725 -- Caption（視窗標題 Contact Parameter ...）：純畫面
    S.sprintf("Contact Parameter  ''%s''  ",GetLastOpenFN());
    Caption=S;
#endif // GATE (S12-C save)

    bFlag=CanChangeData(false);
    if(bFlag)                                                                   //kevin 20150525
    {
        EL<TLabel>("TfBinSel", "labWarning")->Caption="";                                                 //jou 2010-12-15
        for(int tag=0; tag<eBinTypeTotal; tag++)
    bBinPanelEnabled[tag]=fSecurity->Insufficient(20, false);   // golden cBinSel.cpp:1732-1732 取代：MyBinPanel[tag]->Panel（每個 tag 的 Bin 面板）沒有頁面 id → bBinPanelEnabled[tag]
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1732-1732 -- MyBinPanel[tag]->Panel（每個 tag 的 Bin 面板）沒有頁面 id → bBinPanelEnabled[tag]
            MyBinPanel[tag]->Panel->Enabled=fSecurity->Insufficient(20, false);
#endif // GATE (S12-C save)
    }
    else
    {
        BS_ShowChangeBinMessage();                                                 //jou 2010-12-15
        if(CosFunction.bOneCycleCanChangeContinuesFailBin)                      //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
        {
    if(FileRW_OutArmSuckHasIC()==false && ShuttleHasIC()==false && IndexHasIC()==false)   // golden cBinSel.cpp:1739-1739 取代：OutArmSuck（aHotPlateSubstrate.h）與 HTEditList.h 同 TU 衝突 → FileRW/_KitSuck.cpp 轉接
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1739-1739 -- OutArmSuck（aHotPlateSubstrate.h）與 HTEditList.h 同 TU 衝突 → FileRW/_KitSuck.cpp 轉接
            if(OutArmSuck.HasIC()==false && ShuttleHasIC()==false && IndexHasIC()==false)
#endif // GATE (S12-C save)
            {
                for(int tag=0; tag<eBinTypeTotal; tag++)
    bBinPanelEnabled[tag]=fSecurity->Insufficient(20, false);   // golden cBinSel.cpp:1742-1742 取代：MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1742-1742 -- MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
                    MyBinPanel[tag]->Panel->Enabled=fSecurity->Insufficient(20, false);
#endif // GATE (S12-C save)
            }
            else
            {
                for(int tag=0; tag<eBinTypeTotal; tag++)
    bBinPanelEnabled[tag]=false;   // golden cBinSel.cpp:1747-1747 取代：MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1747-1747 -- MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
                    MyBinPanel[tag]->Panel->Enabled=false;
#endif // GATE (S12-C save)
            }
        }
        else
        {
            for(int tag=0; tag<eBinTypeTotal; tag++)
    bBinPanelEnabled[tag]=false;   // golden cBinSel.cpp:1753-1753 取代：MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1753-1753 -- MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
                MyBinPanel[tag]->Panel->Enabled=false;
#endif // GATE (S12-C save)
        }
    }

    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    filerw::ELTodo("golden cBinSel.cpp:1759 HSys.BinDisCtrl->ProcessStopStart(false) (bin display TFT, NUMBER_PANEL_TYPE 3/4) not done: port BinDisCtrl is opaque/NULL (database.h:303)");   // golden cBinSel.cpp:1759-1759 取代：HSys.BinDisCtrl->ProcessStopStart(false)：Bin 顯示 TFT（NUMBER_PANEL_TYPE 3/4）的硬體動作；移植樹 BinDisCtrl 是不透明 NULL（database.h:303）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1759-1759 -- HSys.BinDisCtrl->ProcessStopStart(false)：Bin 顯示 TFT（NUMBER_PANEL_TYPE 3/4）的硬體動作；移植樹 BinDisCtrl 是不透明 NULL（database.h:303）
        HSys.BinDisCtrl->ProcessStopStart(false);
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_SCC ||                                                 //Steven 20101221
       CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20130621 add SCK RMS
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            for(int tag=0; tag<eBinTypeTotal; tag++)
    bBinPanelEnabled[tag]=false;   // golden cBinSel.cpp:1767-1767 取代：MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1767-1767 -- MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]
                MyBinPanel[tag]->Panel->Enabled=false;
#endif // GATE (S12-C save)
        }
        else                                                                    //跟上面有無IC衝突,所以不設定!
        {
        }
    }

    GetBinSettingAuth();                                                        //Steven 20120330 : FT跟RT的Enable
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
    if(bBinPanelEnabled[tag]==true) bBinPanelEnabled[tag]=authBinSetting[tag];   // golden cBinSel.cpp:1777-1778 取代：MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]（乘上 authBinSetting[tag]）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1777-1778 -- MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]（乘上 authBinSetting[tag]）
        if(MyBinPanel[tag]->Panel->Enabled==true)
            MyBinPanel[tag]->Panel->Enabled=authBinSetting[tag];
#endif // GATE (S12-C save)
    }

    ;   // golden cBinSel.cpp:1781-1801 取代：MyBinPanel[tag]->mtTrayItem 格子的 tray 名稱與底色（Prod.iTrayType／s6TrayName）：純畫面（頁面有 s6TrayName）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1781-1801 -- MyBinPanel[tag]->mtTrayItem 格子的 tray 名稱與底色（Prod.iTrayType／s6TrayName）：純畫面（頁面有 s6TrayName）
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
        for(int i=eAuto1; i<eTrayCount; i++)
        {
            if(Prod.iTrayType[i]==tNotUse)
            {
                MyBinPanel[tag]->mtTrayItem->SetCellNumber    (eItemName,  i+eBinSetting, "");
                MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemName,  i+eBinSetting, eCLBtnFace);
            }
            else
            {
                MyBinPanel[tag]->mtTrayItem->SetCellNumber    (eItemName,  i+eBinSetting, s6TrayName[i].c_str());
                MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemName,  i+eBinSetting, eCLGray);
            }
        }

        if(AUTO3_IS_MAGAZINE>0)
        {
            MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemName,  iMagAtAuto+eBinSetting, eCLBtnFace);
        }
    }
#endif // GATE (S12-C save)

    iMode=iTestRunMode;
    if(iMode==RT)                                                               //Steven 20090810 : ReTest Bin Select
    {
        if((IniConfig.bA02BinModelPrime && IniConfig.bFTBin2RTBin==true &&      //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
           (iBinModelPrime==0 || CosFunction.bDisableRTBinSet)) ||
           (IniConfig.bA02BinModelPrime==false &&
           (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)))
        {
            filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "Normal");
        }
        else
        {
            filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "Re-Test");
        }
    }
    else if(iMode==OffT)
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "Off-Line");
    }
    else if(iMode==RT_ART)                                                      //ChungHung 20141002 add for KYEC AutoRetest
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "ART Re-Test");
    }
    else if(iMode==FT_ART)                                                      //ChungHung 20141002 add for KYEC AutoRetest
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "ART Normal");
    }
    else if(iMode==RT_MRT)                                                      //Ifor 20170316 (wei) add MRT Mode
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "MRT Re-Test");
    }
    else if(iMode==FT_MRT)                                                      //Ifor 20170316 (wei) add MRT Mode
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "MRT Normal");
    }
    else
    {
        filerw::ELComboText(EL<TComboBox>("TfBinSel", "cbTestMode"), "Normal");
    }

    if(AccessLevel<LevelSet.AccessLevel[20])
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->Enabled=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->Enabled=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->Enabled=false;                                                 //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->Enabled=false;                                                 //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->Enabled=false;                                                 //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->Enabled=false;                                                 //Ifor 20170316 (wei) add MRT Mode
    }
    else
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->Enabled=true;
        EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=true;
        EL<TTabSheet>("TfBinSel", "tsOffline")->Enabled=true;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->Enabled=true;                                                  //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->Enabled=true;                                                  //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->Enabled=true;                                                  //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->Enabled=true;                                                  //Ifor 20170316 (wei) add MRT Mode
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220310 : ATP要求鎖定critical parameter
    {
        if(bAuthCriticalPara[2])
        {
            EL<TTabSheet>("TfBinSel", "tsNormal")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsOffline")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsArtFT")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsArtRT")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsMrtFT")->Enabled=false;
            EL<TTabSheet>("TfBinSel", "tsMrtRT")->Enabled=false;
        }
    }
    bCheckOffLineLevel=true;

    ;   // golden cBinSel.cpp:1879-1879 取代：fBinSel->Height=920：視窗屬性
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1879-1879 -- fBinSel->Height=920：視窗屬性
    fBinSel->Height=920;
#endif // GATE (S12-C save)

    //以下保持在最下面-----
    BS_cbTestModeChange();   // golden cBinSel.cpp:1882-1882 取代：cbTestModeChange(this)：本 TU 的轉換版沒有 Sender 參數
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1882-1882 -- cbTestModeChange(this)：本 TU 的轉換版沒有 Sender 參數
    BS_cbTestModeChange(this);
#endif // GATE (S12-C save)
    bShow=true;
    ;   // golden cBinSel.cpp:1884-1885 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1884-1885 -- Left／Top：視窗位置
    Left=10;
    Top=10;
#endif // GATE (S12-C save)

    if(IniConfig.bI32CanCelErrorBin)                                            //kevin 20160724 error bin 功能取消
        EL<TCheckBox>("TfBinSel", "CancelErrorBin")->Visible=true;
    else
        EL<TCheckBox>("TfBinSel", "CancelErrorBin")->Visible=false;

    if(IniConfig.bI33ErrorBinBox)                                               //kevin 20160912 20160819 error bin 要放到 Bin Box
    {
        if(Prod.iOutArmBinBoxX>-1000 && Prod.iOutArmBinBoxY>-1000)              //kevin 20190706 Set up Teach Bin box pos
        {
            EL<TLabel>("TfBinSel", "LabBulkBox")->Caption= "Set up Teach Bin box pos";
            EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->Enabled=false;
            ;   // UI-only（HTML 端處理）：EL<TLabel>("TfBinSel", "LabBulkBox")->Color = clRed;
            iHWFix_BinBox=0;
        }
        else
        {
            EL<TLabel>("TfBinSel", "LabBulkBox")->Caption="BulkCount";
            EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->Enabled=true;
            ;   // UI-only（HTML 端處理）：EL<TLabel>("TfBinSel", "LabBulkBox")->Color =(TColor)0x00C2B8A6;
        }
        EL<TPanel>("TfBinSel", "BulkBox")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfBinSel", "BulkBox")->Visible=false;
    }

    EL<TPanel>("TfBinSel", "palSpecificBin")->Visible=false;                                              //JerryYang 20170301 (wei) 存檔後不顯示
    if(CosFunction.bAllSiteSameFailBinShowAlarm==true &&                        //JerryYang 20170221 (wei) 改成可複選bin
       IniConfig.bI34AllSiteAreSameFailBinShowAlarm==true)                      //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    {
        EL<TSpeedButton>("TfBinSel", "btnSettingSpecificBin")->Visible=true;
    EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->grid.ColCount =iTestBinCount;   // golden cBinSel.cpp:1919-1919 取代：sgSpecificBin（TStringGrid）→ filerw::ELStringGrid：ColCount 在它的 grid 成員上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1919-1919 -- sgSpecificBin（TStringGrid）→ filerw::ELStringGrid：ColCount 在它的 grid 成員上
        EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->ColCount =iTestBinCount;                                 //kevin 20170828 (Steven) add 設定 BIN 數量
#endif // GATE (S12-C save)
        for(int i=0; i<iTestBinCount; i++)                                      //kevin 20170828 add
        {
            EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->Cells[i][0]="Bin"+IntToStr(i);
            if(bSpecificBin[i]==true)
                EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->Cells[i][1]="V";
            else
                EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->Cells[i][1]="";
        }
    }
    else
    {
        EL<TSpeedButton>("TfBinSel", "btnSettingSpecificBin")->Visible=false;
    }

    if(CosFunction.bUseMRTMode==true)                                           //Ifor 20170405 (wei) add MRT Use Mode By Setup File
    {
        EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Visible=true;
        if(fMain->CheckCanChangeRealDummy()==false)
        {
            EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Enabled=false;                                        //Ifor 20170405 add 機台內有IC不可修改MRT 功能開關
        }
    }
    else
    {
        EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Visible=false;
    }

    EL<TPanel>("TfBinSel", "pnlAutoSiteMap")->Visible=CosFunction.bAutoSiteMappingSetOpenBIN;             //jou 20200928 : Auto Site Mapping Set Open BIN
    EL<TCheckBox>("TfBinSel", "chkShow0Xbin")->Visible=CosFunction.bShow0Xbin;                               //jou 20220719 : show 0X bin
    EL<TLabel>("TfBinSel", "labASMPassBin")->Visible=CosFunction.bAutoSiteMappingSetPassBIN;              //jou 20230221 : Auto Site Mapping Set Pass BIN
    EL<TComboBox>("TfBinSel", "cbbASMPassBin")->Visible=CosFunction.bAutoSiteMappingSetPassBIN;              //jou 20230221 : Auto Site Mapping Set Pass BIN

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20200622 : Control Bin 顯示
    {
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Visible=true;                                   //Sam 20201221 : 修正 Control Bin 顯示
    filerw::ELTodo("golden cBinSel.cpp:1955-2009 CC_Greatek Control Bin overlay (mtBinSelectBy TTMyTray cells, edControlBinCheckPoint text from fProductionInfo->iControlBinCheckPoint) not ported");   // golden cBinSel.cpp:1955-2009 取代：CC_Greatek Control Bin 疊圖：edControlBinCheckPoint 位置／Parent、mtBinSelectBy（TTMyTray）外觀與格子（fProductionInfo->iControlBin[]）、edControlBinCheckPoint->Text（移植樹 TfProductionInfo 沒有 iControlBinCheckPoint）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1955-2009 -- CC_Greatek Control Bin 疊圖：edControlBinCheckPoint 位置／Parent、mtBinSelectBy（TTMyTray）外觀與格子（fProductionInfo->iControlBin[]）、edControlBinCheckPoint->Text（移植樹 TfProductionInfo 沒有 iControlBinCheckPoint）
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Parent=EL<TPageControl>("TfBinSel", "PageControl1");
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->BringToFront();
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Left        =183;
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Top         =iRowHeight*19;
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Height      =iRowHeight-2;
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Width       =97;

        EL<TControl>("TfBinSel", "mtBinSelectBy")->Parent=EL<TPageControl>("TfBinSel", "PageControl1");
        EL<TControl>("TfBinSel", "mtBinSelectBy")->BringToFront();
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Left        =357;
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Top         =iRowHeight*18;
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Height      =iRowHeight*3;                               //Sam 20201221 : 修正 Control Bin 顯示
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Width=iColWidth*iTestBinCount+10;
        EL<TControl>("TfBinSel", "mtBinSelectBy")->XItem=iTestBinCount;
        EL<TControl>("TfBinSel", "mtBinSelectBy")->YItem=3;                                                 //Sam 20201221 : 修正 Control Bin 顯示
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Visible=True;

        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLWhite,   clWhite);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLGreen,   clGreen);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLRed,     clRed);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLYellow,  TColor(0x000080FF));
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLPurple,  clPurple);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLBlue,    clBlue);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLGray,    clGray);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLSilver,  clSilver);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLBtnFace, clBtnFace);
        EL<TControl>("TfBinSel", "mtBinSelectBy")->SetColorMap(eCLOlive,   clOlive);
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfBinSel", "mtBinSelectBy")->Color       =clWhite;
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Font->Name  ="Arial";
        EL<TControl>("TfBinSel", "mtBinSelectBy")->Font->Size  =10;
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfBinSel", "mtBinSelectBy")->Font->Color =clRed;

        if(fProductionInfo->iControlBinNum>0)
        {
            for(int i=0; i<fProductionInfo->iControlBinNum; i++)
            {
                iBin=fProductionInfo->iControlBin[i];
                EL<TControl>("TfBinSel", "mtBinSelectBy")->SetCellNumber(iBin, 0, FloatToStr(fProductionInfo->dControlPercent[i]));
                EL<TControl>("TfBinSel", "mtBinSelectBy")->SetCellNumber(iBin, 1, FloatToStr(fProductionInfo->iControlCount[i]));
                EL<TControl>("TfBinSel", "mtBinSelectBy")->SetCellNumber(iBin, 2, FloatToStr(fProductionInfo->dControlSiteToSite[i]));
            }
        }

        if(fProductionInfo->iControlBinCheckPoint>0)                            //Sam 20201221 : 增加保護
        {
            EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Text="Interval time : "+IntToStr(fProductionInfo->iControlBinCheckPoint/60);
        }
        else if(fProductionInfo->iControlBinCheckCount>0)                       //Sam 20201221 : 增加 Control Bin Count 監控
        {
            EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Text="Interval count : "+IntToStr(fProductionInfo->iControlBinCheckCount);
        }
        else
        {
            EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Text="Not use";
        }
#endif // GATE (S12-C save)
    }
    else
    {
        EL<TEdit>("TfBinSel", "edControlBinCheckPoint")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_JSSI_Semiconductor)                                    //Jimmychiu 20260120 : 客戶要求關閉全Bin關閉
    {
        EL<TButton>("TfBinSel", "btnSetAll2NotUse")->Visible=false;
    }

    filerw::ELTodo("golden cBinSel.cpp:2021 FrmAOI->bCheckAOIFailBinUse() has no body in the port (forms/fAOI.h) -- spbAOIBin hidden"); if(false)   // golden cBinSel.cpp:2021-2021 取代：FrmAOI->bCheckAOIFailBinUse()：移植樹 forms/fAOI.h 沒有這個方法 → 走 else（隱藏 spbAOIBin）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2021-2021 -- FrmAOI->bCheckAOIFailBinUse()：移植樹 forms/fAOI.h 沒有這個方法 → 走 else（隱藏 spbAOIBin）
    if(FrmAOI->bCheckAOIFailBinUse())
#endif // GATE (S12-C save)
    {
        EL<TButton>("TfBinSel", "spbAOIBin")->Visible = true;
    }
    else
    {
        EL<TButton>("TfBinSel", "spbAOIBin")->Visible = false;
    }
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cBinSel.cpp:1635  TfBinSel::ShowChangeBinMessage()
static void BS_ShowChangeBinMessage()
{

    if(CosFunction.bOneCycleCanChangeContinuesFailBin)                          //Steven 20151123 : Modified because of CosFunction.bOneCycleCanChangeContinuesFailBin
    {
        EL<TLabel>("TfBinSel", "labWarning")->Caption="";
    }
    else
    {
        EL<TLabel>("TfBinSel", "labWarning")->Caption="* ";
    }

    if(FileRW_OutArmSuckHasIC())   // golden cBinSel.cpp:1646-1646 取代：OutArmSuck → FileRW/_KitSuck.cpp 轉接
#if 0 // GATE (S12-C save) golden cBinSel.cpp:1646-1646 -- OutArmSuck → FileRW/_KitSuck.cpp 轉接
    if(OutArmSuck.HasIC())
#endif // GATE (S12-C save)
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" OutArmSuck ";
    if(ShuttleHasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Shuttle ";
    if(IndexHasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Index ";
    if(MOT[MMAuto1].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 1 ";
    if(MOT[MMAuto2].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 2 ";
    if(MOT[MMAuto3].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 3 ";
    if(MOT[MMAuto4].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 4 ";                     //Steven 20230907 : For HT-9011UC
    if(MOT[MMAuto5].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 5 ";
    if(MOT[MMAuto6].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 6 ";
    if(MOT[MMAuto1_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 1 track ";
    if(MOT[MMAuto2_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 2 track ";
    if(MOT[MMAuto3_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 3 track ";
    if(MOT[MMAuto4_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 4 track ";
    if(MOT[MMAuto5_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 5 track ";
    if(MOT[MMAuto6_Car].HasIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Auto 6 track ";
    if(MOT[MManualTray1].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 1 ";
    if(MOT[MManualTray2].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 2 ";
    if(MOT[MManualTray3].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 3 ";
    if(MOT[MManualTray4].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 4 ";                      //Steven 20230907 : For HT-9011UC
    if(MOT[MManualTray5].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 5 ";
    if(MOT[MManualTray6].HasRealIC())
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+" Fix 6 ";

    if(CosFunction.bOneCycleCanChangeContinuesFailBin)                          //Steven 20151123 : Modified because of CosFunction.bOneCycleCanChangeContinuesFailBin
    {
        if(EL<TLabel>("TfBinSel", "labWarning")->Caption!="")
            EL<TLabel>("TfBinSel", "labWarning")->Caption+"Has IC can't change bin!!";
    }
    else
    {
        EL<TLabel>("TfBinSel", "labWarning")->Caption=EL<TLabel>("TfBinSel", "labWarning")->Caption+"Has IC can't change bin!!";
    }

}

// golden cBinSel.cpp:2147  TfBinSel::cbTestModeChange(TObject *Sender)
static void BS_cbTestModeChange()
{

    bShow=false;

    ReadFile(false, false, "");                                                 //kevin 20160928
    if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="Re-Test")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=true;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=1;   // golden cBinSel.cpp:2161-2161 取代：ActivePage=tsRetest（頁序 1）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2161-2161 -- ActivePage=tsRetest（頁序 1）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsRetest");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="Off-Line")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=true;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=2;   // golden cBinSel.cpp:2172-2172 取代：ActivePage=tsOffline（頁序 2）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2172-2172 -- ActivePage=tsOffline（頁序 2）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsOffline");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="ART Re-Test")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=true;                                               //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=4;   // golden cBinSel.cpp:2183-2183 取代：ActivePage=tsArtRT（頁序 4）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2183-2183 -- ActivePage=tsArtRT（頁序 4）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsArtRT");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="ART Normal")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=true;                                               //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=3;   // golden cBinSel.cpp:2194-2194 取代：ActivePage=tsArtFT（頁序 3）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2194-2194 -- ActivePage=tsArtFT（頁序 3）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsArtFT");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="Normal")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=true;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=0;   // golden cBinSel.cpp:2205-2205 取代：ActivePage=tsNormal（頁序 0）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2205-2205 -- ActivePage=tsNormal（頁序 0）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsNormal");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="MRT Re-Test")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=true;                                               //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=6;   // golden cBinSel.cpp:2216-2216 取代：ActivePage=tsMrtRT（頁序 6）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2216-2216 -- ActivePage=tsMrtRT（頁序 6）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsMrtRT");
#endif // GATE (S12-C save)
    }
    else if(EL<TComboBox>("TfBinSel", "cbTestMode")->Text=="MRT Normal")
    {
        EL<TTabSheet>("TfBinSel", "tsNormal")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsRetest")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsOffline")->TabVisible=false;
        EL<TTabSheet>("TfBinSel", "tsArtFT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsArtRT")->TabVisible=false;                                              //ChungHung 20141002 add for KYEC AutoRetest
        EL<TTabSheet>("TfBinSel", "tsMrtFT")->TabVisible=true;                                               //Ifor 20170316 (wei) add MRT Mode
        EL<TTabSheet>("TfBinSel", "tsMrtRT")->TabVisible=false;                                              //Ifor 20170316 (wei) add MRT Mode
    EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=5;   // golden cBinSel.cpp:2227-2227 取代：ActivePage=tsMrtFT（頁序 5）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2227-2227 -- ActivePage=tsMrtFT（頁序 5）
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBinSel", "PageControl1")->ActivePage=EL<TTabSheet>("TfBinSel", "tsMrtFT");
#endif // GATE (S12-C save)
    }
    bShow=true;

}

// golden cBinSel.cpp:6092  TfBinSel::ReadPrimeDara()
static void BS_ReadPrimeDara()
{

    AnsiString S=GetLastOpenFN();
    AnsiString szDir;

    szDir.sprintf("%s%s\\Binasgn.Data", DataPath.c_str(), S.c_str());   // golden cBinSel.cpp:6097-6097 取代：AnsiString 經 varargs 傳給 sprintf（BCB AnsiString 只有一個指標成員才成立）→ .c_str()
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6097-6097 -- AnsiString 經 varargs 傳給 sprintf（BCB AnsiString 只有一個指標成員才成立）→ .c_str()
    szDir.sprintf("%s%s\\Binasgn.Data", DataPath, S);
#endif // GATE (S12-C save)
    iBinModelPrime=ReadIniData(szDir, "BinModel", "bPrime", 0);
    BS_SetPrimeButton();                                                           //Steven 20160624 : 重新顯示按鈕

    if(CosFunction.bShow0Xbin)                                                  //jou 20220719 : show 0X bin
    {
        EL<TCheckBox>("TfBinSel", "chkShow0Xbin")->Checked=ReadIniData(szDir, "Other", "bShow0XBin",false);
    }

}

// golden cBinSel.cpp:6058  TfBinSel::SetPrimeButton()
static void BS_SetPrimeButton()
{

    if(IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime)                   //ChungHung 20120912 add Bin Setting like Epson
    {
        EL<TSpeedButton>("TfBinSel", "spbNormal")->Visible=true;
        EL<TSpeedButton>("TfBinSel", "spbPrime")->Visible=true;

        if(iBinModelPrime==1)
        {
            EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=true;
            EL<TSpeedButton>("TfBinSel", "spbPrime")->Down=true;
            EL<TComboBox>("TfBinSel", "cbTestMode")->Enabled=true;
        }
        else
        {
            EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=false;
            EL<TSpeedButton>("TfBinSel", "spbNormal")->Down=true;
            EL<TComboBox>("TfBinSel", "cbTestMode")->Enabled=false;
        }
    }
    else
    {
        if(IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime==false)        //jou 2014-08-26 修正 BIN select FT/RT 顯示異常
        {
            EL<TTabSheet>("TfBinSel", "tsRetest")->Enabled=false;
            EL<TComboBox>("TfBinSel", "cbTestMode")->Enabled=false;
        }

        EL<TSpeedButton>("TfBinSel", "spbNormal")->Visible=false;
        EL<TSpeedButton>("TfBinSel", "spbPrime")->Visible=false;
    }
    filerw::ELTodo("golden cBinSel.cpp:6089 fMain->SetNormalOrPrime() has no body in the port (forms/fMain.h)");   // golden cBinSel.cpp:6089-6089 取代：fMain->SetNormalOrPrime()：主畫面 Normal/Prime 顯示，移植樹 forms/fMain.h 沒有
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6089-6089 -- fMain->SetNormalOrPrime()：主畫面 Normal/Prime 顯示，移植樹 forms/fMain.h 沒有
    fMain->SetNormalOrPrime();                                                  //Steven 20160817 : 必須在讀完Bin別設定之後
#endif // GATE (S12-C save)

}

// golden cBinSel.cpp:2252  TfBinSel::spbSaveClick(TObject *Sender)
static void BS_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cBinSel.cpp:2258-2258 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2258-2258 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString szDirTarget="", GroupStr;

    if(CheckFix2Tray())                                                         //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    {
        filerw::ELMessage("己開啟AOI功能，不能使用Fix2 Tray");
        return ;
    }

    EL<TLabel>("TfBinSel", "Label1")->Caption="Saving Data..";

    AnsiString szDir=GetRecipePath();
    BS_SaveOther(szDir);
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(GetLastOpenFN(), "JOBFILE");
    #endif

    EL<TLabel>("TfBinSel", "Label1")->Caption="Refresh data..";
    BS_ReadWriteMRTMode(1);                                                        //寫入MRT Mode  //Ifor 20170417 (wei) add MRT Function 移至 BinSel 內 做開關切換
    BS_ReadWriteSpecialFunction(true);                                             //JerryYang 20220923 : bOutShtLoseICSetErrUntilOneCycle改為可By工作檔
    ReadFile(false, false, "");                                                 //kevin 20160928

    if(CheckOSBin()==false)                                                     //Sam 20250115 : 新增 OSBIN 保護
    {
        filerw::ELMessage("SetOSBin Category Error", IntToStr(Prod.iOpenBin));
    }

    if(bSaveBin==false)
    {
        bSaveBin=true;
        RecordProcess("Save Bin");
    }

    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
    }
    fShowBinSelect->InitShowBinDigital();                                       //20111110  Dell
    EL<TPanel>("TfBinSel", "palSpecificBin")->Visible=false;                                              //JerryYang 20170301 (wei) 存檔後不顯示
    EL<TLabel>("TfBinSel", "Label1")->Caption="Save..OK";
    EL<TSpeedButton>("TfBinSel", "spbSave")->Down=false;
    if(CosFunction.bUseMRTMode==true)                                           //Ifor 20170405 (wei) add 進入Setup 修改設定後要重新整理Start Mode
        fMain->SetStartModeData();
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

    if(IniConfig.bSIGURDFunction &&                                             //Sam 20210331 : 北興新增 Yield Monitor
       fLotInfo->pgLotinfo->ActivePage==fLotInfo->tsYieldMonitior)
    {
        fLotInfo->RefreshYieldMonitor();
    }

}

// golden cBinSel.cpp:2378  TfBinSel::SaveOther(AnsiString szDir)
static void BS_SaveOther(AnsiString szDir)
{
    filerw::ELMark("SaveOther");   // FileRW：存檔流程 trace（ack.trace）

    int tag=0;
    AnsiString szDirTarget="", GroupStr, sBuffer="", sBuffer1="";
    AnsiString FileName="", FileName2="";
    int iResult;
    MyForceDirectories(szDir);

    if(fBinSel->PageControl1->ActivePageIndex==1)   // golden cBinSel.cpp:2386-2386 取代：PageControl1 → fBinSel->PageControl1（收養的同一個物件；見上）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2386-2386 -- PageControl1 → fBinSel->PageControl1（收養的同一個物件；見上）
    if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==1)
#endif // GATE (S12-C save)
    {
        tag=eBinRT;
        if(IniConfig.bA02BinModelPrime)                                         //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
        {
            if(IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet))
            {
                FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
            }
            else
            {
                FileName=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
            }
        }
        else
        {
            if(IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)
            {
                FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
            }
            else
            {
                FileName=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
            }
        }
    }
    else if(fBinSel->PageControl1->ActivePageIndex==2)   // golden cBinSel.cpp:2412-2412 取代：同上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2412-2412 -- 同上
    else if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==2)                                   //ChungHung 20141002 add for KYEC AutoRetest  //ChungHung 20111110 add
#endif // GATE (S12-C save)
    {
        FileName=FileInfo().PathCombin(szDir, "BinasgnOff-Line.Data");
        tag=eBinOffLine;
    }
    else if(fBinSel->PageControl1->ActivePageIndex==3)   // golden cBinSel.cpp:2417-2417 取代：同上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2417-2417 -- 同上
    else if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==3)                                   //ChungHung 20141002 add for KYEC AutoRetest
#endif // GATE (S12-C save)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_ART.Data");
        tag=eBinFT_ART;
    }
    else if(fBinSel->PageControl1->ActivePageIndex==4)   // golden cBinSel.cpp:2422-2422 取代：同上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2422-2422 -- 同上
    else if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==4)                                   //ChungHung 20141002 add for KYEC AutoRetest
#endif // GATE (S12-C save)
    {
        FileName=FileInfo().PathCombin(szDir, "BinasgnOff_ART.Data");
        tag=eBinRT_ART;
    }
    else if(fBinSel->PageControl1->ActivePageIndex==5)   // golden cBinSel.cpp:2427-2427 取代：同上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2427-2427 -- 同上
    else if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==5)                                   //Ifor 20170316 (wei) add MRT Mode
#endif // GATE (S12-C save)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_MRT.Data");
        tag=eBinFT_MRT;
    }
    else if(fBinSel->PageControl1->ActivePageIndex==6)   // golden cBinSel.cpp:2432-2432 取代：同上
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2432-2432 -- 同上
    else if(EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex==6)                                   //Ifor 20170316 (wei) add MRT Mode
#endif // GATE (S12-C save)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_MRT_RT.Data");
        tag=eBinRT_MRT;
    }
    else //if(PageControl1->ActivePageIndex==0)                                 //tsNormal
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
        tag=eBinFT;
    }

    if(IniConfig.bP28Auto1OnlyBin1==true)                                       //Ifor 20171017 (wei) add for Unisem Auto1 Only Bin1
    {
        int iErrCode=0;
        TStringList *sT3List;
        sT3List = new TStringList();
        sT3List->CommaText=sBinTraySetT3Pos[tag]->CommaText;
        if(CUSTOMER_CODE==CC_UNISEM_M && LastSet.iTester==OFF_LINE)             //Ifor 20171213 (Steven) : add UNISEM_M 要求Offline 不卡 Auto1 Only Bin1 功能
        {
        }
        else
        {
            for(int i=0; i<iTestBinCount; i++)
            {
                if(i==1)
                {
                    if(sT3List->Strings[i]!=AnsiString("1"))
                    {
                        iErrCode=1;
                        break;
                    }
                }
                else if(sT3List->Strings[i]==AnsiString("1"))
                {
                    iErrCode=2;
                    break;
                }
            }
        }
        sT3List->Clear();
        delete sT3List;
        if(iErrCode==1)
        {
            filerw::ELMessage("Bin1未被設定在Auto1 !! 請確認 !!\n Bin1 is not set to Auto1 !! Please Check !!");
            return;
        }
        else if(iErrCode==2)
        {
            filerw::ELMessage("只有Bin 1被允許設為 Auto1 !! 請確認 !!\n Only Bin 1 is allowed to set to Auto 1!!! Please check !!");
            return;
        }
    }

    if(CosFunction.bUsePassBinOnlyCanSetOneBin==true)                           //Ifor 20171213 (Steven) : Auto Tray 設定Pass時僅可設定一個Bin
    {
        int iBinCt=0;
        AnsiString str1, str2;
        TStringList *sT3List;
        sT3List = new TStringList();
        sT3List->CommaText=sBinTraySetT3Pos[tag]->CommaText;
        for(int i=0; i<iAutoRight; i++)                                         //Auto1 ~ Auto3
        {
            if(MyBinPanel[tag]->iT6IsFail[i]==0)                                //判斷 Failed || Pass
            {
                iBinCt=0;
                for(int j=0; j<iTestBinCount; j++)
                {
                    if(sT3List->Strings[j]==AnsiString(iTo3Unload[i]+1))
                    {
                        iBinCt++;
                        if(iBinCt>1)
                        {
    str1.sprintf("%s Pass Bin Only Can Set One Bin.", s6TrayName[i].c_str()); str2.sprintf("%s Pass Bin 僅可設定一個Bin", s6TrayName[i].c_str());   // golden cBinSel.cpp:2504-2505 取代：AnsiString 經 varargs 傳給 sprintf → .c_str()
#if 0 // GATE (S12-C save) golden cBinSel.cpp:2504-2505 -- AnsiString 經 varargs 傳給 sprintf → .c_str()
                            str1.sprintf("%s Pass Bin Only Can Set One Bin.", s6TrayName[i]);
                            str2.sprintf("%s Pass Bin 僅可設定一個Bin", s6TrayName[i]);
#endif // GATE (S12-C save)
                            filerw::ELMessage(str1, str2);
                            return;
                        }
                    }
                }
            }
        }
        sT3List->Clear();
        delete sT3List;
    }

    //==> Eastsun 20260508 整合: Ifor 20220426 add KYEC 要求一定要有一個 ART Retest
    //Ifor 20220426 add: KYEC 要求一定要有一個ART Retest
    //==>
    if(CUSTOMER_CODE==CC_KYEC_LEE && (tag==eBinFT_ART || tag==eBinOffLine))
    {
        ARTBinCheck(tag);
    }
    //<==
    //Ifor 20220426 add: KYEC 要求一定要有一個ART Retest
    //<== Eastsun 20260508 整合
    if(IniConfig.bI32CanCelErrorBin)
    {
        FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        FormSysTools->OpenFormData(FileName2);                                  //kevin 20160727: 為了加快ini讀取速度
        FormSysTools->WriteIniData("I/F Error", "CancelError",     EL<TCheckBox>("TfBinSel", "CancelErrorBin")->Checked);                    //kevin 20160724 儲存取消BIN 檔案 "\\Binasgn.Data";
        FormSysTools->CloseFormData();                                          //Steven 20140531 : 為了加快ini讀取速度
    }
    else if(IniConfig.bI33ErrorBinBox)                                          //kevin 20160909: 為了加快ini讀取速度
    {
        FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        FormSysTools->OpenFormData(FileName2);                                  //kevin 20160727: 為了加快ini讀取速度
        FormSysTools->WriteIniData("I/F Error", "Bin",             iTo3PosUnload[MyBinPanel[tag]->iErrorT6]);
        FormSysTools->WriteIniData("I/F Error", "Tray",            s6TrayName[MyBinPanel[tag]->iErrorT6]);      //Steven 20241205 : Bin to tray增加文字顯示
        FormSysTools->WriteIniData("I/F Error", "ErrorBinBox",     EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->ItemIndex);                    //kevin 20160819 error bin 要放到 Bin Box "\\Binasgn.Data";
        if(EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->ItemIndex==1)                                          //kevin 20200722 add 保護值
        {
            if(EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text.ToIntDef(0)<=2)                     //kevin 20200722 add 保護值
                EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text="2";
        }
        FormSysTools->WriteIniData("I/F Error", "Alarm Count",     EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text);               //kevin 20160819 error bin 要放到 Bin Box "\\Binasgn.Data";
        FormSysTools->CloseFormData();                                          //Steven 20140531 : 為了加快ini讀取速度
    }

    if(CosFunction.bAllSiteSameFailBinShowAlarm)                                //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    {
        FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        FormSysTools->OpenFormData(FileName2);                                  //kevin 20160727: 為了加快ini讀取速度

        for(int i=0; i<iTestBinCount; i++)                                      //kevin 20170828 (Steven) add   //JerryYang 20170221 (wei) 改成可複選bin
        {
            sBuffer.sprintf("%d,", EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->Cells[i][1]=="V");
            sBuffer1+=sBuffer;
        }

        FormSysTools->WriteIniData("AllSiteSameBin", "bSpecificFailBin", sBuffer1);                 //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
        FormSysTools->CloseFormData();                                          //Steven 20140531 : 為了加快ini讀取速度

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
            FormSysTools->OpenFormData(FileName2);                              //kevin 20160727: 為了加快ini讀取速度
            FormSysTools->WriteIniData("I/F Error", "Bin",             iTo3PosUnload[MyBinPanel[tag]->iErrorT6]);
            FormSysTools->WriteIniData("I/F Error", "Tray",            s6TrayName[MyBinPanel[tag]->iErrorT6]);  //Steven 20241205 : Bin to tray增加文字顯示
            FormSysTools->WriteIniData("I/F Error", "ErrorBinBox",     EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->ItemIndex);                //kevin 20160819 error bin 要放到 Bin Box "\\Binasgn.Data";
            if(EL<TRadioGroup>("TfBinSel", "rg_FixBinBox")->ItemIndex==1)                                      //kevin 20200722 add 保護值
            {
                if(EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text.ToIntDef(0)<=2)                 //kevin 20200722 add 保護值
                    EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text="2";
            }
            FormSysTools->WriteIniData("I/F Error", "Alarm Count",     EL<TEdit>("TfBinSel", "ed_FixBinBoxAlarmCount")->Text);           //kevin 20220916 目前現階段只寫不讀  error bin 要放到 Bin Box "\\Binasgn.Data";
            FormSysTools->CloseFormData();                                      //Steven 20140531 : 為了加快ini讀取速度
        }
    }

    FormSysTools->OpenFormData(FileName);                                       //Steven 20140531 : 為了加快ini讀取速度
    if(bGPIBChangeAlarm)                                                        //wei 20151127 GPIB Change Alarm
    {
        for(int i=0; i<iTestBinCount; i++)                                       //Steven 20231002 :　eTrayCount -->　TEST_MAX_BIN  //Jimmychiu 20251114 : fix overflow
        {
            sBinConsFail[tag]->Strings[i]   =(BinSelect[tag].bConsFail[i])?"1":"0"; //Sam 20210715 : 修正 SetAlarmSetup 寫入異常。
            sBinEnableFail[tag]->Strings[i] =(BinSelect[tag].bFailure[i])?"1":"0";
            sBinFailPercent[tag]->Strings[i]=BinSelect[tag].dFailureLimit[i];   //Sam 20210715 : 修正 SetAlarmSetup 寫入異常。
        }
    }

    if(bTCPIPChangeAlarm)                                                       //Isaac 20170613 (wei) TCP/IP
    {
        for(int i=0; i<iTestBinCount; i++)
        {
            sBinDoubleContact[tag]->Strings[i]=BinSelect[tag].iDBContact[i];
        }
    }

    if(bSetBINCOUNT)
    {
        for(int i=0; i<iTestBinCount; i++)
        {
            sBinFailIgnore[tag]->Strings[i]=BinSelect[tag].iPersentIgnore[i];
        }
    }

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20200429 : IPSC Add Control Bin Alarm
    {
        if(bMOFileControlBinChangeAlarm)
        {
            for(int i=0; i<iTestBinCount; i++)
            {
                sSpecialBinBySocket[tag]->Strings[i]     =(BinSelect[tag].bSpecialBinBySocket[i])?"1":"0";
                sSpecialBinCountBySocket[tag]->Strings[i]=IntToStr(BinSelect[tag].iSpecialBinCountBySocket[i]);
            }
        }
    }

    if(bOLPSetCategroy)                                                         //Sam 20230711 : 修正 OLP SetCategroy 異常
    {
        MyBinPanel[tag]->iErrorT6=iTo6Unload[BinSelect[iTestRunMode].IfErrorT3];
        for(int i=0; i<iTestBinCount; i++)
            sBinTraySetT3Pos[tag]->Strings[i]=BinSelect[tag].iCatDataT3Pos[i];

        for(int i=0; i<eTrayCount; i++)
            MyBinPanel[tag]->iT6IsFail[i]=Prod.iIsFailT6[i];
    }

    BS_SaveFunctionData(tag);

    if(bSetByDLL==true)                                                         //JerryYang 20181126 (Steven) : support Epson DLL function
    {
        MyBinPanel[tag]->iErrorT6=iTo6Unload[BinSelect[iTestRunMode].IfErrorT3];
    }

    if(CUSTOMER_CODE==CC_TFME_CHINA)                                            //Ifor 20251029 add:通富微電子要求I/F Error BIN存檔與Category相同
    {
        if(MyBinPanel[tag]->iErrorT6==2)
        {
            FormSysTools->WriteIniData("I/F Error", "Bin", iTo3Unload[6]);      //JerryYang 20230926
        }
        else if(MyBinPanel[tag]->iErrorT6==11)
        {
            FormSysTools->WriteIniData("I/F Error", "Bin", iTo3Unload[18]);     //JerryYang 20230926
        }
        else
        {
            FormSysTools->WriteIniData("I/F Error", "Bin", iTo3Unload[MyBinPanel[tag]->iErrorT6+1]);      //JerryYang 20230926
        }
    }
    else
    {
        FormSysTools->WriteIniData("I/F Error", "Bin",             iTo3Unload[MyBinPanel[tag]->iErrorT6]);      //JerryYang 20230926
    }
    FormSysTools->WriteIniData("I/F Error", "Tray",            s6TrayName[MyBinPanel[tag]->iErrorT6]);      //Steven 20241205 : Bin to tray增加文字顯示
    FormSysTools->WriteIniData("I/F Error", "Contact",         0);
    FormSysTools->WriteIniData("I/F Error", "Cons.Fail",       0);
    FormSysTools->WriteIniData("I/F Error", "Fail Percent",    0);
    FormSysTools->WriteIniData("I/F Error", "Scan",            0);
    FormSysTools->WriteIniData("I/F Error", "Fail Count",      0);              //ChungHung 20120724 add BinFailCount

    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20160614 : 避免降版導致分Bin異常, 將資料回寫到舊的
    {
        GroupStr.sprintf("Category%d", i);                                      //kevin 20140317 256 bin  bin0 開始
        for(int j=eBinNotUse; j<eBinSetTotal; j++)
        {
            if(MyBinPanel[tag]->BackT6PosTray[i][j])
            {
                FormSysTools->WriteIniData(GroupStr, "Bin", iTo3PosUnload[j-eBinNotUse]);
                if(j==eBinNotUse)
                    FormSysTools->WriteIniData(GroupStr, "Tray", "NotUse");
                else
                {
                    if(j==32)                                                   //Ifor 20251029 Fix:Bin to tray Fix1 顯示變成 Fix7
                    {
                        FormSysTools->WriteIniData(GroupStr, "Tray", s3TrayName[3]);       //Steven 20241205 : Bin to tray增加文字顯示
                    }
                    else
                    {
                        FormSysTools->WriteIniData(GroupStr, "Tray", s3TrayName[iTo3PosUnload[j-eBinNotUse-1]]);       //Steven 20241205 : Bin to tray增加文字顯示
                    }
                }
                break;
            }
        }

        iResult=MyBinPanel[tag]->i2Contact[i];                                  //Steven 20231205 : b2Contact --> i2Contact
        if(Prod.bD22SupportMultiDoubleContact==false)
           //IniConfig.bD22VerifyMode==false)                                     //Sam 20231117 : 整合到 QA 模式 //Sam 20221012 : 新增 VerifyMode 功能
        {
            iResult=0;
        }
        FormSysTools->WriteIniData(GroupStr, "Contact", iResult);

        if(bGPIBChangeAlarm)                                                    //wei 20151127 GPIB Change Alarm
        {
            MyBinPanel[tag]->bConFail[i]=BinSelect[iTestRunMode].bFailure[i];
            MyBinPanel[tag]->bPersentEnable[i]=BinSelect[iTestRunMode].bFailure[i];
            MyBinPanel[tag]->dPersentNumber[i]=Prod.dFailureLimit[i];
        }

        iResult=(MyBinPanel[tag]->bConFail[i])?1:0;
        FormSysTools->WriteIniData(GroupStr, "Cons.Fail", iResult);

        iResult=(MyBinPanel[tag]->bPersentEnable[i])?1:0;                       //Steven 20140529 Start: Fail Persent & Count同時存在
        FormSysTools->WriteIniData(GroupStr, "Fail Percent",        iResult);
        FormSysTools->WriteIniData(GroupStr, "Fail Percent Ignore", MyBinPanel[tag]->iPersentIgnore[i]);
        FormSysTools->WriteIniData(GroupStr, "Fail Percent Limit",  MyBinPanel[tag]->dPersentNumber[i]);

        iResult=(MyBinPanel[tag]->bCountEnable[i])?1:0;
        FormSysTools->WriteIniData(GroupStr, "Fail Count",        iResult);
        FormSysTools->WriteIniData(GroupStr, "Fail Count Ignore", MyBinPanel[tag]->iCountIgnore[i]);
        FormSysTools->WriteIniData(GroupStr, "Fail Count Limit",  MyBinPanel[tag]->iCountNumber[i]);

        iResult=(MyBinPanel[tag]->bSpecialBinByArm[i])?1:0;                     //ChungHung 20140730 add ContinuousFailHaveOneCycle start
        FormSysTools->WriteIniData(GroupStr, "Special Bin By Arm",        iResult);
        FormSysTools->WriteIniData(GroupStr, "Special Bin Count By Arm", MyBinPanel[tag]->iSpecialBinCountByArm[i]);
        iResult=(MyBinPanel[tag]->bSpecialBinBySocket[i])?1:0;
        FormSysTools->WriteIniData(GroupStr, "Special Bin By Socket",        iResult);
        FormSysTools->WriteIniData(GroupStr, "Special Bin Count By Socket",  MyBinPanel[tag]->iSpecialBinCountBySocket[i]);

        FormSysTools->WriteIniData(GroupStr, "By Bin Low Yield",                  MyBinPanel[tag]->bLowYield[i]);     //Steven 20140828 : By Bin Yield Monitor
        FormSysTools->WriteIniData(GroupStr, "By Bin and Site Compare Arm Yield", MyBinPanel[tag]->bArmYield[i]);     //Steven 20140828 : By Bin Arm Yield Monitor
        FormSysTools->WriteIniData(GroupStr, "By Bin Compare Site Yield",         MyBinPanel[tag]->bSiteYield[i]);    //Steven 20140828 : By Bin Site Yield Monitor

        iResult=(MyBinPanel[tag]->bScan[i])?1:0;
        FormSysTools->WriteIniData(GroupStr, "Scan", iResult);
    }

    if(bGPIBChangeAlarm ||                                                      //wei 20151127 GPIB Change Alarm
       bSetByDLL)                                                               //JerryYang 20181126 (Steven) : support Epson DLL function
    {
        for(int i=0; i<eTrayCount; i++)
            MyBinPanel[tag]->iT6IsFail[i]=BinSelect[iTestRunMode].iStackDefFailCate[iTo3Unload[i]]; //Sam 20250303 : 修正 RMS 異常
    }

    for(int i=0; i<eTrayCount; i++)
    {
        FormSysTools->WriteIniData(s6TrayName[i], "Pass/Fail", MyBinPanel[tag]->iT6IsFail[i]);
        FormSysTools->WriteIniData(s6TrayName[i], "Link",      MyBinPanel[tag]->bT6Link[i]);
    }

    if(USE_AUTO_RETEST==eartInstall || IniConfig.bA38_SLT_Summary)              //ChungHung 20140317 add Auto Retest  //JerryYang 20220923 : add for SLT lot summary
    {
        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            if(UNLOADER_ART[i]==eartInstall)                                    //Steven 20161221 (wei) : for SCK only Auto 2 has ART
                FormSysTools->WriteIniData(s6TrayName[i], "AutoRetest", MyBinPanel[tag]->bT6ART[i]);
            else
                FormSysTools->WriteIniData(s6TrayName[i], "AutoRetest", false);
        }

        if(USE_AUTO_RETEST==eartInstall &&                                      //Steven 20161221 (wei) : Cate R for ART
           CosFunction.bUseSCKART &&
           TestIF_File.iSCKART_SortMode==1 &&
           (tag==FT || tag==OffT) || IniConfig.bA38_SLT_Summary)                //Steven 20170317 (wei) : Off T也要可以ART  //JerryYang 20220923 : add for SLT lot summary
        {
            for(int i=0; i<eTrayCount; i++)
            {
                FormSysTools->WriteIniData(s6TrayName[i], "Cate R", MyBinPanel[tag]->bT6CateR[i]);
            }
        }
        else
        {
            for(int i=0; i<eTrayCount; i++)
            {
                FormSysTools->WriteIniData(s6TrayName[i], "Cate R", false);
            }
        }
    }
    else
    {
        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            FormSysTools->WriteIniData(s6TrayName[i], "AutoRetest", false);
        }
    }
    FormSysTools->CloseFormData();                                              //Steven 20140531 : 為了加快ini讀取速度

    FileName2=FileInfo().PathCombin(szDir, "Binasgn.Data");
    FormSysTools->OpenFormData(FileName2);
    FormSysTools->WriteIniData("BinModel", "bPrime", iBinModelPrime);           //ChungHung 20120912 add Bin Setting like Epson

    if(CosFunction.bShow0Xbin)                                                  //jou 20220719 : show 0X bin
    {
        FormSysTools->WriteIniData("Other", "bShow0XBin", EL<TCheckBox>("TfBinSel", "chkShow0Xbin")->Checked);
    }
    FormSysTools->CloseFormData();                                              //Steven 20140531 : 為了加快ini讀取速度

    AddSpace(szDir);

}

// golden cBinSel.cpp:5716  TfBinSel::SaveFunctionData(int tag)
static void BS_SaveFunctionData(int tag)
{
    filerw::ELMark("SaveFunctionData");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString GroupName;
    if(tag==eBinRT)
    {
        if((IniConfig.bA02BinModelPrime==true  && IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet)) ||        //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
           (IniConfig.bA02BinModelPrime==false && (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)))
        {
            GroupName="Bin Func FT";
            FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1);   //kevin 20160928 使用新的bin format
            FormSysTools->WriteIniData(GroupName, "3617 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
            TransferBinTrayStrToName(tag);
            FormSysTools->WriteIniData(GroupName, "3617 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3636 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3656 BinType",               sBinType                [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3676 BinConsFail",           sBinConsFail            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10641 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10642 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10643 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10644 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10646 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10645 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10647 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10648 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10649 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10650 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10680 LowYield",             sLowYield               [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10681 ArmYield",             sArmYield               [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10682 SiteYield",            sSiteYield              [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10683 BySiteClean",          sBySiteClean            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10684 ByBinClean",           sByBinClean             [tag]->CommaText);

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
                FormSysTools->WriteIniData(GroupName, "10685 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10686 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10687 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10688 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10689 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10690 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
            }

            FormSysTools->WriteIniData(GroupName, "BinTrayLinked(10698)", sBinTrayLinked        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "BinLinked(10699)",     sBinLinked            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "MagazineSetup(10700)", sMagazineSetup        [tag]->CommaText);

            FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
        }
        else
        {
            GroupName="Bin Func RT";
            FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1);   //kevin 20160928 使用新的bin format
            FormSysTools->WriteIniData(GroupName, "3717 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
            TransferBinTrayStrToName(tag);
            FormSysTools->WriteIniData(GroupName, "3717 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3718 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3719 BinType",               sBinType                [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "3720 BinConsFail",           sBinConsFail            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10741 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10742 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10743 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10744 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10745 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10746 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10747 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10748 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10749 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10750 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10780 LowYield",             sLowYield               [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10781 ArmYield",             sArmYield               [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10782 SiteYield",            sSiteYield              [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10783 BySiteClean",          sBySiteClean            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10784 ByBinClean",           sByBinClean             [tag]->CommaText);

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
                FormSysTools->WriteIniData(GroupName, "10785 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10786 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10787 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10788 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10789 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
                FormSysTools->WriteIniData(GroupName, "10790 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
            }

            FormSysTools->WriteIniData(GroupName, "BinTrayLinked(10798)", sBinTrayLinked        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "BinLinked(10799)",     sBinLinked            [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "MagazineSetup(10800)", sMagazineSetup        [tag]->CommaText);

            FormSysTools->WriteIniData(GroupName, "AOIBinTraySetting", sAOIBinTraySetting        [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
        }
    }
    else if(tag==eBinFT)
    {
        GroupName="Bin Func FT";
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        FormSysTools->WriteIniData(GroupName, "3617 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "3617 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3636 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3656 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3676 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10641 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10642 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10643 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10644 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10645 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10646 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10647 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10648 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10649 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10650 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10680 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10681 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10682 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10683 BySiteClean",          sBySiteClean            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10684 ByBinClean",           sByBinClean             [tag]->CommaText);

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
            FormSysTools->WriteIniData(GroupName, "10685 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10686 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10687 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10688 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10689 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10690 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
        }

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(10698)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(10699)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(10700)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }
    else if(tag==eBinOffLine)
    {
        GroupName="Bin Func OffLine";
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        FormSysTools->WriteIniData(GroupName, "3722 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "3722 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3723 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3724 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3725 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10941 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10942 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10943 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10944 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10945 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10946 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10947 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10948 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10949 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10950 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10980 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10981 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10982 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10983 BySiteClean",          sBySiteClean            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "10984 ByBinClean",           sByBinClean             [tag]->CommaText);

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
            FormSysTools->WriteIniData(GroupName, "10985 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10986 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10987 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10988 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10989 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "10990 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
        }

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(10998)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(10999)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(11000)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }
    else if(tag==eBinFT_ART)
    {
        GroupName="Bin Func ART_FT";                                            //wei 20161124 修改eBinFT_ART存成Bin Func ART_RT問題
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        FormSysTools->WriteIniData(GroupName, "3801 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "3801 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3802 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3803 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3804 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11041 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11042 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11043 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11044 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11045 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11046 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11047 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11048 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11049 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11050 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11080 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11081 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11082 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11083 BySiteClean",          sBySiteClean            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11084 ByBinClean",           sByBinClean             [tag]->CommaText);

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
            FormSysTools->WriteIniData(GroupName, "11085 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11086 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11087 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11088 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11089 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11090 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
        }

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(11098)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(11099)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(11100)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }
    else if(tag==eBinRT_ART)
    {
        GroupName="Bin Func ART_RT";                                            //wei 20161124 修改eBinRT_ART存成Bin Func ART_FT問題
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        FormSysTools->WriteIniData(GroupName, "3901 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "3901 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3902 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3903 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "3904 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11141 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11142 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11143 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11144 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11145 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11146 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11147 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11148 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11149 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11150 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11180 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11181 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11182 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11183 BySiteClean",          sBySiteClean            [tag]->CommaText);   //wei 20161124 11083 --> 11183
        FormSysTools->WriteIniData(GroupName, "11184 ByBinClean",           sByBinClean             [tag]->CommaText);   //wei 20161124 11084 --> 11184

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(11198)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(11199)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(11200)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }
    else if(tag==eBinFT_MRT)
    {
        GroupName="Bin Func MRT_FT";                                            //Ifor 20170316 (wei) add KYEC MRT Mode
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1);
        FormSysTools->WriteIniData(GroupName, "4001 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "4001 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4002 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4003 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4004 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11241 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11242 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11243 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11244 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11245 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11246 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11247 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11248 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11249 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11250 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11280 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11281 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11282 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11283 BySiteClean",          sBySiteClean            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11284 ByBinClean",           sByBinClean             [tag]->CommaText);

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
            FormSysTools->WriteIniData(GroupName, "11285 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11286 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11287 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11288 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11289 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11290 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
        }

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(11298)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(11299)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(11300)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }
    else if(tag==eBinRT_MRT)
    {
        GroupName="Bin Func MRT_RT";
        FormSysTools->WriteIniData(GroupName, "NewBinFormat",               1);
        FormSysTools->WriteIniData(GroupName, "4101 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
        TransferBinTrayStrToName(tag);
        FormSysTools->WriteIniData(GroupName, "4101 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4102 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4103 BinType",               sBinType                [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "4104 BinConsFail",           sBinConsFail            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11341 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11342 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11343 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11344 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11345 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11346 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11347 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11348 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11349 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11350 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11380 LowYield",             sLowYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11381 ArmYield",             sArmYield               [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11382 SiteYield",            sSiteYield              [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11383 BySiteClean",          sBySiteClean            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "11384 ByBinClean",           sByBinClean             [tag]->CommaText);

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
            FormSysTools->WriteIniData(GroupName, "11385 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11386 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11387 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11388 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11389 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            FormSysTools->WriteIniData(GroupName, "11390 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
        }

        FormSysTools->WriteIniData(GroupName, "BinTrayLinked(11398)", sBinTrayLinked        [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "BinLinked(11399)",     sBinLinked            [tag]->CommaText);
        FormSysTools->WriteIniData(GroupName, "MagazineSetup(11400)", sMagazineSetup        [tag]->CommaText);

        FormSysTools->WriteIniData(GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
    }

}

// golden cBinSel.cpp:6181  TfBinSel::ReadWriteMRTMode(int iMode)
static void BS_ReadWriteMRTMode(int iMode)
{
    filerw::ELMark("ReadWriteMRTMode");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString S=GetLastOpenFN();
    AnsiString szDir="";
    szDir=DataPath+S;
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath.c_str(), S.c_str());   // golden cBinSel.cpp:6186-6186 取代：AnsiString 經 varargs 傳給 sprintf → .c_str()
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6186-6186 -- AnsiString 經 varargs 傳給 sprintf → .c_str()
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
#endif // GATE (S12-C save)

    if(CosFunction.bUseMRTMode==true)
    {
        if(iMode==0)
        {
            TestIF_File.bEnableMRTMode=ReadIniData(szDir, "Configuration", "Enable MRT Mode", false);
            EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Checked     =TestIF_File.bEnableMRTMode;
        }
        else
        {
            WriteIniData(szDir, "Configuration", "Enable MRT Mode",(EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Checked)?1:0);//Ifor 20170405 add MRT Use Mode By Setup File
            TestIF_File.bEnableMRTMode=EL<TCheckBox>("TfBinSel", "cbUseMRTMode")->Checked;
        }
    }
    else                                                                        //Ifor 20170504 (wei) add 未開啟MRT Mode 客戶 強制設定EnableMRTMode = false
    {
        TestIF_File.bEnableMRTMode=false;

        if(CosFunction.bAutoSiteMappingSetOpenBIN==true)                        //jou 20200928 : Auto Site Mapping Set Open BIN
        {
            if(iMode==0)
            {
                filerw::ELComboIndex(EL<TComboBox>("TfBinSel", "cbbAutoSiteMap"), ReadIniData(szDir, "Configuration", "ASM Open Bin", 0));
                TestIF_File.iOpenBin=EL<TComboBox>("TfBinSel", "cbbAutoSiteMap")->ItemIndex-1;
                Prod.iOpenBin=TestIF_File.iOpenBin;                             //Steven 20230213 : [I21-9]的OS Bin跟著工作檔
    filerw::ELTodo("golden cBinSel.cpp:6212 fMain->SetOpenBin() has no body in the port (forms/fMain.h)");   // golden cBinSel.cpp:6212-6212 取代：fMain->SetOpenBin()：主畫面 open-bin 顯示，移植樹 forms/fMain.h 沒有（只在讀的分支）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6212-6212 -- fMain->SetOpenBin()：主畫面 open-bin 顯示，移植樹 forms/fMain.h 沒有（只在讀的分支）
                fMain->SetOpenBin();
#endif // GATE (S12-C save)
            }
            else
            {
                WriteIniData(szDir, "Configuration", "ASM Open Bin", EL<TComboBox>("TfBinSel", "cbbAutoSiteMap")->ItemIndex);
            }

            if(CosFunction.bAutoSiteMappingSetPassBIN==true)                    //jou 20230221 : Auto Site Mapping Set Pass BIN
            {
                if(iMode==0)
                {
                    filerw::ELComboIndex(EL<TComboBox>("TfBinSel", "cbbASMPassBin"), ReadIniData(szDir, "Configuration", "ASM Pass Bin", 2));
                    TestIF_File.iASMPassBin=EL<TComboBox>("TfBinSel", "cbbASMPassBin")->ItemIndex-1;
                }
                else
                {
                    WriteIniData(szDir, "Configuration", "ASM Pass Bin",    EL<TComboBox>("TfBinSel", "cbbASMPassBin")->ItemIndex);
                }
            }
        }
    }

}

// golden cBinSel.cpp:6235  TfBinSel::ReadWriteSpecialFunction(bool bReadWrite)
static void BS_ReadWriteSpecialFunction(bool bReadWrite)
{
    filerw::ELMark("ReadWriteSpecialFunction");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
    {
        AnsiString S=GetLastOpenFN();
        AnsiString szDir="";
        szDir=DataPath+S;
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath.c_str(), S.c_str());   // golden cBinSel.cpp:6242-6242 取代：AnsiString 經 varargs 傳給 sprintf → .c_str()
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6242-6242 -- AnsiString 經 varargs 傳給 sprintf → .c_str()
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
#endif // GATE (S12-C save)

        if(CosFunction.bOutShtLoseICSetErrUntilOneCycle)
        {
            if(bReadWrite==false)                                               //Read
            {
                TestIF_File.bOutShtLoseICSetErrUntilOneCycle=ReadIniData(szDir, "Configuration", "bOutShtLoseICSetErrUntilOneCycle", false);
                EL<TCheckBox>("TfBinSel", "cbOutShtLoseICSetErrUntilOneCycle")->Checked  =TestIF_File.bOutShtLoseICSetErrUntilOneCycle;
            }
            else                                                                //Write
            {
                WriteIniData(szDir, "Configuration", "bOutShtLoseICSetErrUntilOneCycle", EL<TCheckBox>("TfBinSel", "cbOutShtLoseICSetErrUntilOneCycle")->Checked);
                TestIF_File.bOutShtLoseICSetErrUntilOneCycle=EL<TCheckBox>("TfBinSel", "cbOutShtLoseICSetErrUntilOneCycle")->Checked;
            }
        }
        else
        {
            TestIF_File.bOutShtLoseICSetErrUntilOneCycle=false;
        }

        if(CosFunction.bIndexDropICSetErrUntilOneCycle)
        {
            if(bReadWrite==false)                                                //Read
            {
                TestIF_File.bIndexDropICSetErrUntilOneCycle=ReadIniData(szDir, "Configuration", "bIndexDropICSetErrUntilOneCycle", false);
                EL<TCheckBox>("TfBinSel", "cbIndexDropErrSetErrUntilOneCycle")->Checked =TestIF_File.bIndexDropICSetErrUntilOneCycle;
            }
            else                                                                //Write
            {
                WriteIniData(szDir, "Configuration", "bIndexDropICSetErrUntilOneCycle", EL<TCheckBox>("TfBinSel", "cbIndexDropErrSetErrUntilOneCycle")->Checked);
                TestIF_File.bIndexDropICSetErrUntilOneCycle=EL<TCheckBox>("TfBinSel", "cbIndexDropErrSetErrUntilOneCycle")->Checked;
            }
        }
        else
        {
            TestIF_File.bIndexDropICSetErrUntilOneCycle=false;
        }
    }
    else
    {
        if(CosFunction.bOutShtLoseICSetErrUntilOneCycle)
        {
            TestIF_File.bOutShtLoseICSetErrUntilOneCycle=true;
        }
        else
        {
            TestIF_File.bOutShtLoseICSetErrUntilOneCycle=false;
        }

        if(CosFunction.bIndexDropICSetErrUntilOneCycle)
        {
            TestIF_File.bIndexDropICSetErrUntilOneCycle=true;
        }
        else
        {
            TestIF_File.bIndexDropICSetErrUntilOneCycle=false;
        }
    }

}

// golden cBinSel.cpp:6166  TfBinSel::btnSettingSpecificBinClick(TObject *Sender)
static void BS_btnSettingSpecificBinClick()
{

    if(EL<TPanel>("TfBinSel", "palSpecificBin")->Visible==false)                                          //JerryYang 20170301 (wei) specific bin
    {
        EL<TPanel>("TfBinSel", "palSpecificBin")->Visible=true;
    { static const char* const kReparent[][2] = {{"palSpecificBin", "PageControl1"}}; filerw::ELSetParents("TfBinSel", kReparent, 1); }   // golden cBinSel.cpp:6171-6171 取代：palSpecificBin->Parent=PageControl1 → 替身權限樹改父元件（ELSetParents）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6171-6171 -- palSpecificBin->Parent=PageControl1 → 替身權限樹改父元件（ELSetParents）
        EL<TPanel>("TfBinSel", "palSpecificBin")->Parent=EL<TPageControl>("TfBinSel", "PageControl1");
#endif // GATE (S12-C save)
    ;   // golden cBinSel.cpp:6172-6173 取代：palSpecificBin 的 BringToFront／Top：純畫面（Visible 照 golden 切換）
#if 0 // GATE (S12-C save) golden cBinSel.cpp:6172-6173 -- palSpecificBin 的 BringToFront／Top：純畫面（Visible 照 golden 切換）
        EL<TPanel>("TfBinSel", "palSpecificBin")->BringToFront();
        EL<TPanel>("TfBinSel", "palSpecificBin")->Top=625;                                                //JerryYang 20220923 : 650->625
#endif // GATE (S12-C save)
    }
    else
    {
        EL<TPanel>("TfBinSel", "palSpecificBin")->Visible=false;
    }

}

