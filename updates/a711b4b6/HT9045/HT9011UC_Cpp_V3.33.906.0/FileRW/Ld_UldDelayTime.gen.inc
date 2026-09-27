// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Ld_UldDelayTime 的讀寫檔（HTEditList：elUdUld → UdUld.Data）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cLd_ULd.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cLd_ULd.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 37 個（header 宣告 60 個）。
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
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"


// golden cLd_ULd.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static AnsiString LastFileName;   // golden cLd_ULd.h: TfLd_ULd::LastFileName
static bool fShow=false;          // golden cLd_ULd.h: TfLd_ULd::fShow

using filerw::EL;

// 前置宣告
static void LU_TfLd_ULd();
static void LU_FormShow();
static void LU_ReadFile();
static void LU_DoIniDataToForm();
static void LU_FormClose();
static void LU_spbSaveClick();
static void LU_SaveSetupFile(AnsiString szDir, AnsiString S);

// golden cLd_ULd.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void LU_DfmItems()
{
}

// golden cLd_ULd.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kLU_ParentOf[][2] = {
    {"Panel2", "TabSheet1"},
    {"TabSheet1", "PageControl1"},
    {"btnDefaultValue", "Panel1"},
    {"cbF23", "palKnocker"},
    {"cbF40", "palKnocker"},
    {"cbP13", "palKnocker"},
    {"cbP14", "palKnocker"},
    {"cbP16", "palKnocker"},
    {"edF23", "palKnocker"},
    {"edP13_1", "palKnocker"},
    {"edP13_2", "palKnocker"},
    {"edP14_1", "palKnocker"},
    {"edP14_2", "palKnocker"},
    {"edP16_1", "palKnocker"},
    {"edP16_2", "palKnocker"},
    {"edP40_1", "palKnocker"},
    {"edP40_2", "palKnocker"},
    {"edP40_3", "palKnocker"},
    {"edtLD_BeforeDownDelay", "grpLoader"},
    {"edtLD_FixTrayDely", "grpLoader"},
    {"edtLD_LiftDownDelay", "grpLoader"},
    {"edtLD_MiddLockDelay", "grpLoader"},
    {"edtLD_TrayArrivalDely", "grpLoader"},
    {"edtULD_FixTrayDely", "grpUnloader"},
    {"edtULD_LiftDownDelay", "grpUnloader"},
    {"edtULD_TrayArrivalDelay", "grpUnloader"},
    {"edtULD_TrayArrivalWait", "grpUnloader"},
    {"edtULD_TrayBackDelay", "grpUnloader"},
    {"grpLoader", "Panel2"},
    {"grpUnloader", "Panel2"},
    {"labF40_1", "palKnocker"},
    {"labF40_2", "palKnocker"},
    {"labF40_3", "palKnocker"},
    {"labP13", "palKnocker"},
    {"labP16", "palKnocker"},
    {"palKnocker", "tsKnocker"},
    {"rbTemp", "Panel1"},
    {"spbSave", "Panel1"},
    {"tsKnocker", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void LU_CreateContainerProxies()
{
    EL<TPageControl>("TfLd_ULd", "PageControl1");
    EL<TPanel>("TfLd_ULd", "Panel1");
    EL<TPanel>("TfLd_ULd", "Panel2");
    EL<TTabSheet>("TfLd_ULd", "TabSheet1");
    EL<TGroupBox>("TfLd_ULd", "grpLoader");
    EL<TGroupBox>("TfLd_ULd", "grpUnloader");
    EL<TPanel>("TfLd_ULd", "palKnocker");
    EL<TTabSheet>("TfLd_ULd", "tsKnocker");
    filerw::ELSetParents("TfLd_ULd", kLU_ParentOf, (int)(sizeof(kLU_ParentOf) / sizeof(kLU_ParentOf[0])));
}

// golden cLd_ULd.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void LU_DfmState()
{
    EL<TEdit>("TfLd_ULd", "edF23")->Text = "20";
    EL<TEdit>("TfLd_ULd", "edP13_1")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP13_2")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP14_1")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP14_2")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP16_1")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP16_2")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP40_1")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP40_2")->Text = "1";
    EL<TEdit>("TfLd_ULd", "edP40_3")->Text = "1";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kLU_EnableAll[] = {
    "PageControl1", "Panel1", "Panel2", "TabSheet1", "btnDefaultValue", "cbF23",
    "cbF40", "cbP13", "cbP14", "cbP16", "grpLoader", "grpUnloader",
    "labF40_1", "labF40_2", "labF40_3", "labP13", "labP16", "palKnocker",
    "rbTemp", "spbSave", "tsKnocker",
};

// 存檔流程（spbSaveClick、SaveSetupFile）讀的替身 0 個 —— FileRW/Ld_UldDelayTime.cpp 用它決定頁面必須送哪些值
static const char* const kLU_SaveReads[] = {
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void LU_CreateSaveProxies()
{
}

// golden cLd_ULd.cpp:22  TfLd_ULd::TfLd_ULd(TComponent* Owner)
static void LU_TfLd_ULd()
{

    LastFileName="";
    bool bShow=true; //bNoShow=false;
    bool bEnable=true;//, bDisable=false;
    bool bReadFromFile=true;//, bFixedValue=false;

    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtLD_TrayArrivalDely"),  &Ld_UldDelayTime.LD_TrayArrivalDely,           ECDouble,  "Loader Wait Time",    "Arrived",            bShow, bEnable, bReadFromFile, 0.2,      true,  10.0,      0.2);
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtLD_FixTrayDely"),      &Ld_UldDelayTime.LD_FixTrayDely,               ECDouble,  "Loader Wait Time",    "Lock",               bShow, bEnable, bReadFromFile, 0.5,      true,  10.0,      0.2);
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtLD_MiddLockDelay"),    &Ld_UldDelayTime.LD_StackMiddLockDelay,        ECDouble,  "Loader Wait Time",    "Close",              bShow, bEnable, bReadFromFile, 2.0,      true,  10.0,      0.2);
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtLD_BeforeDownDelay"),  &Ld_UldDelayTime.LD_BeforeDownDelay,           ECDouble,  "Loader Wait Time",    "Down",               bShow, bEnable, bReadFromFile, 0.0,      true,  10.0,      0.0);    //Steven 20240215 : 下降前Delay
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtLD_LiftDownDelay"),    &Ld_UldDelayTime.LD_LiftDownDelay,             ECDouble,  "Loader Wait Time",    "Check",              bShow, bEnable, bReadFromFile, 1.0,      true,  10.0,      0.2);

    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_TrayArrivalDelay"),&Ld_UldDelayTime.ULD_TrayArrivalDelay,         ECDouble,  "UnLoader Wait Time",  "Unlock",             bShow, bEnable, bReadFromFile, 0.2,      true,  10.0,      0.2);
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_FixTrayDely"),     &Ld_UldDelayTime.ULD_FixTrayDely,              ECDouble,  "UnLoader Wait Time",  "Lock",               bShow, bEnable, bReadFromFile, 0.5,      true,  10.0,      0.2);
    if(TRAY_ARM_MODE==eUnderCoveyor)                                                    //Ztex 2024.03.25 Add Reduce tray exit time
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_TrayBackDelay"), &Ld_UldDelayTime.ULD_TrayBackDelay,          ECDouble,  "UnLoader Wait Time",  "Arrived",            bShow, bEnable, bReadFromFile, 1.0,      true,  10.0,      0.1);
    else
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_TrayBackDelay"), &Ld_UldDelayTime.ULD_TrayBackDelay,          ECDouble,  "UnLoader Wait Time",  "Arrived",            bShow, bEnable, bReadFromFile, 1.0,      true,  10.0,      1.0);    //JerryYang 20200408 有翻盤風險改最短1sec
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_LiftDownDelay"),   &Ld_UldDelayTime.ULD_LiftDownDelay,            ECDouble,  "UnLoader Wait Time",  "Check",              bShow, bEnable, bReadFromFile, 0.2,      true,  10.0,      0.2);
    elUdUld->Add(EL<TEdit>("TfLd_ULd", "edtULD_TrayArrivalWait"), &Ld_UldDelayTime.ULD_TrayArrivalWaitDelay,     ECDouble,  "UnLoader Wait Time",  "TrayArrivedWait",    bShow, bEnable, bReadFromFile, 0.3,      true,  10.0,      0.2);    //kevin 20190326 add auto 退tray 等待時間

    if(CosFunction.bKnockerSetBySetupFile==true && USE_LOADER_VIBRATION!=0)     //JerryYang 20191001 loader震動馬達
    {
        elUdUld->Add(EL<TCheckBox>("TfLd_ULd", "cbF40"),              &Ld_UldDelayTime.LD_EnableVibrate,             ECBool,    "Loader Wait Time",    "EnableVibrate",      bShow, bEnable, bReadFromFile, 0);
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP40_1"),            &Ld_UldDelayTime.LD_VibrateLoopCnt,            ECInteger, "Loader Wait Time",    "VibrateLoopCount",   bShow, bEnable, bReadFromFile, 1,        true,  1,         15);
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP40_2"),            &Ld_UldDelayTime.LD_VibrateOnDelay,            ECDouble,  "Loader Wait Time",    "VibrateOn",          bShow, bEnable, bReadFromFile, 10.0,     true,  1.0,       20.0);
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP40_3"),            &Ld_UldDelayTime.LD_VibrateOffDelay,           ECDouble,  "Loader Wait Time",    "VibrateOff",         bShow, bEnable, bReadFromFile, 10.0,     true,  1.0,       15.0);
    }

    if(CosFunction.bKnockerSetBySetupFile==true && ShuttleVibration_INSTALL!=0) //JerryYang 20171006 (wei) Shuttle 震動馬達
    {
        elUdUld->Add(EL<TCheckBox>("TfLd_ULd", "cbF23"),              &IniConfig.bF23ShuttleVibration,                   ECBool,    "Tray",    "bF23ShuttleVibration",       bShow, bEnable, bReadFromFile, 0);
        if(CUSTOMER_CODE==CC_TERAPOWER)                                         //Sam 20210520 : TPW 修改震動時間 Range
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edF23"),          &IniConfig.iF23ShuttleVibrationTime,           ECInteger, "Shuttle", "iF23ShuttleVibrationTime",       bShow, bEnable, bReadFromFile, 20,       true,  1,         100);
        else
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edF23"),          &IniConfig.iF23ShuttleVibrationTime,           ECInteger, "Shuttle", "iF23ShuttleVibrationTime",       bShow, bEnable, bReadFromFile, 20,       true,  20,         100);
    }

    if(CosFunction.bKnockerSetBySetupFile==true)                                //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    {
        elUdUld->Add(EL<TCheckBox>("TfLd_ULd", "cbP13"),         &IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop,  ECBool,     "Tray",     "bEnableAutoTrayEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, LastSet.bEnableAutoTrayEdgePushCylinderLoop?"1":"0");
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP13_1"),       &IniConfig.iP13EdgePushCylinderLoopDelay,           ECInteger,  "Tray",     "iP13EdgePushCylinderLoopDelay",        bShow, bEnable, bReadFromFile, LastSet.iEdgePushCylinderLoopDelay,          true,  2,   300);
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP13_2"),       &IniConfig.iP13EdgePushCylinderOnDelay,             ECInteger,  "Tray",     "iP13EdgePushCylinderOnDelay",          bShow, bEnable, bReadFromFile, 10,                                          true,  0,   50);

        elUdUld->Add(EL<TCheckBox>("TfLd_ULd", "cbP14"),         &IniConfig.bP14EnableAutoTrayRecevieDelayCount,     ECBool,     "Tray",     "bEnableAutoTrayRecevieDelayCount",     bShow, bEnable, bReadFromFile, LastSet.bEnableAutoTrayRecevieDelayCount?"1":"0");
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP14_1"),       &IniConfig.iP14AutoTrayRecevieDelayCount,           ECInteger,  "Tray",     "iP14AutoTrayRecevieDelayCount",        bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieDelayCount,          true,  1,   1000);
        elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP14_2"),       &IniConfig.iP14AutoTrayRecevieLoopDelayTime,        ECInteger,  "Tray",     "iP14AutoTrayRecevieLoopDelayTime",     bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieLoopDelayTime,       true,  2,   3000);

        elUdUld->Add(EL<TCheckBox>("TfLd_ULd", "cbP16"),         &IniConfig.bP16EnableHotplateEdgePushCylinderLoop,  ECBool,     "Hotplate", "bEnableHotplateEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, 0);

        if(TRAY_VIBRATION==VibrationMotor)                                      //JerryYang 20170531 (wei) 敲擊方式改成可以分別設定on off時間
        {
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP16_1"),   &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger, "Tray",      "iHotplateEdgePushCylinderLoopDelay",   bShow, bEnable, bReadFromFile, 10,                                          true,  200,   600);
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP16_2"),   &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger, "Tray",      "iHotplateEdgePushCylinderOnDelay",     bShow, bEnable, bReadFromFile, 10,                                          true,  0,   50);
        }
        else
        {
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP16_1"),   &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger, "Tray",      "iHotplateEdgePushCylinderLoopDelay",   bShow, bEnable, bReadFromFile, 10,                                          true,  2,   600);
            elUdUld->Add(EL<TEdit>("TfLd_ULd", "edP16_2"),   &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger, "Tray",      "iHotplateEdgePushCylinderOnDelay",     bShow, bEnable, bReadFromFile, 10,                                          true,  2,   50);
        }
    }

}

// golden cLd_ULd.cpp:87  TfLd_ULd::FormShow(TObject *Sender)
static void LU_FormShow()
{

    AnsiString S;
    ;   // golden cLd_ULd.cpp:90-90 取代：LoadImage()：示意圖（HTML 自己有）
#if 0 // GATE (S12-C save) golden cLd_ULd.cpp:90-90 -- LoadImage()：示意圖（HTML 自己有）
    LoadImage();
#endif // GATE (S12-C save)
    LastFileName=GetLastOpenFN();
    LU_ReadFile();
    S.sprintf("Loader/Unloader Condition  ''%s''  ",LastFileName);
    ;   // golden cLd_ULd.cpp:94-95 取代：Caption／SetDefaultPos()：視窗標題與位置（HTML 不用）
#if 0 // GATE (S12-C save) golden cLd_ULd.cpp:94-95 -- Caption／SetDefaultPos()：視窗標題與位置（HTML 不用）
    Caption=S;
    SetDefaultPos();                                                            //Steven 20240215 : 調整預設位置
#endif // GATE (S12-C save)

    EL<TGroupBox>("TfLd_ULd", "grpLoader")->Enabled      =fSecurity->Insufficient(115, false);               //jou 981207 權限控制
    EL<TGroupBox>("TfLd_ULd", "grpUnloader")->Enabled    =fSecurity->Insufficient(115, false);
    EL<TSpeedButton>("TfLd_ULd", "btnDefaultValue")->Enabled=fSecurity->Insufficient(115, false);
    EL<TTabSheet>("TfLd_ULd", "tsKnocker")->TabVisible   =(CosFunction.bKnockerSetBySetupFile);              //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    EL<TPanel>("TfLd_ULd", "palKnocker")->Enabled     =fSecurity->Insufficient(116, false);

    if(CosFunction.bKnockerSetBySetupFile==true && TRAY_VIBRATION==VibrationMotor)
    {
        EL<TLabel>("TfLd_ULd", "labP13")->Caption="[P13-1] Off delay time                   (Unit : 0.1 Sec)";
        EL<TLabel>("TfLd_ULd", "labP16")->Caption="[P16-1] Off delay time                   (Unit : 0.1 Sec)";
    }

    if(CosFunction.bKnockerSetBySetupFile==false || USE_LOADER_VIBRATION==0)    //JerryYang 20191001 loader震動馬達
    {
        EL<TCheckBox>("TfLd_ULd", "cbF40")->Visible=false;
        EL<TEdit>("TfLd_ULd", "edP40_1")->Visible=false;
        EL<TEdit>("TfLd_ULd", "edP40_2")->Visible=false;
        EL<TEdit>("TfLd_ULd", "edP40_3")->Visible=false;
        EL<TLabel>("TfLd_ULd", "labF40_1")->Visible=false;
        EL<TLabel>("TfLd_ULd", "labF40_2")->Visible=false;
        EL<TLabel>("TfLd_ULd", "labF40_3")->Visible=false;
    }

    if(ShuttleVibration_INSTALL==0)                                             //JerryYang 20171006 (wei) Shuttle 震動馬達
    {
        EL<TCheckBox>("TfLd_ULd", "cbF23")->Visible=false;
        EL<TEdit>("TfLd_ULd", "edF23")->Visible=false;                                                   //JerryYang 20171205 (Steven) shuttle震動馬達功能
    }

    fShow=true;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cLd_ULd.cpp:143  TfLd_ULd::ReadFile()
static void LU_ReadFile()
{

    AnsiString szDir="";
    LastFileName=GetLastOpenFN();
    szDir.sprintf("%s%s\\", DataPath, LastFileName);

    if(elUdUld!=NULL)
    {
        elUdUld->ReadEditTextFromFile(szDir, "UdUld.Data");
        elUdUld->InitialDataToEdit();
    }

}

// golden cLd_ULd.cpp:156  TfLd_ULd::DoIniDataToForm()
static void LU_DoIniDataToForm()
{

    if(elUdUld!=NULL)
    {
        elUdUld->InitialDataToEdit();
    }

}

// golden cLd_ULd.cpp:170  TfLd_ULd::FormClose(TObject *Sender, TCloseAction &Action)
static void LU_FormClose()
{

    LU_ReadFile();
    ;   // golden cLd_ULd.cpp:173-173 取代：rbTemp->SetFocus()：焦點（HTML 端）
#if 0 // GATE (S12-C save) golden cLd_ULd.cpp:173-173 -- rbTemp->SetFocus()：焦點（HTML 端）
    EL<TRadioButton>("TfLd_ULd", "rbTemp")->SetFocus();                                                         //Steven 20240215 : 避免Enable false的時候跳例外
#endif // GATE (S12-C save)
    fShow=false;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cLd_ULd.cpp:179  TfLd_ULd::spbSaveClick(TObject *Sender)
static void LU_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cLd_ULd.cpp:185-185 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關
#if 0 // GATE (S12-C save) golden cLd_ULd.cpp:185-185 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString szDir="";
    szDir.sprintf("%s%s\\", DataPath, LastFileName);                            //Steven 20210716 : Add \\

    LU_SaveSetupFile(szDir, LastFileName);                                         //kevin 20150105  Start 另存 jobfile
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(LastFileName, "JOBFILE");
    #endif
    LU_ReadFile();
    EL<TSpeedButton>("TfLd_ULd", "spbSave")->Down=false;
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }

}

// golden cLd_ULd.cpp:207  TfLd_ULd::SaveSetupFile(AnsiString szDir, AnsiString S)
static void LU_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    MyForceDirectories(szDir);

    if(elUdUld!=NULL)
    {
        elUdUld->SaveEditTextToFile(szDir, "UdUld.Data");
    }

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

