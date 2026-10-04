// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_AutoAlignment 的讀寫檔（HTEditList： → <DataPath>\<配方>\HandlerCondition.Data [AutoAlignmrnt]（寫：spbSaveClick；讀：移植樹 AutoAlignment/AutoAlignment.cpp ReadFile）、D:\HT9045\system\Gerneral.ini [Auto_Alignment] AUTO_ALIGNMENT_CCD1/2_ADRESS／_PORT（讀：建構子；CheckAndReadIniDataGeneral 缺鍵補寫））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoAlignment/AutoAlignment.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden AutoAlignment/AutoAlignment.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 46 個（header 宣告 115 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "MachineType.h"
#include "AutoAlignment/AutoAlignment.h"


// golden AutoAlignment/AutoAlignment.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bInArmSendTeachCmd;   // golden AutoAlignment.h:210（public；建構子設 false，CCD 通訊用，移植樹沒有讀者）
static bool bOutArmSendTeachCmd;  // golden AutoAlignment.h:211
static void AA_CheckAutoAlignmentEvent()
{
    TestIF_File.iAutoAlignmentTrayEvent=0;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_ChangeSetupFile;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_AfterHome;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_InitialStart;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_EveryTray;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_AfterOneCycle;

    TestIF_File.iAutoAlignmentShuttleHotplateEvent=0;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent = TestIF_File.iAutoAlignmentShuttleHotplateEvent | AutoAlignmentCK_ChangeSetupFile;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent = TestIF_File.iAutoAlignmentShuttleHotplateEvent | AutoAlignmentCK_AfterHome;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent = TestIF_File.iAutoAlignmentShuttleHotplateEvent | AutoAlignmentCK_InitialStart;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent = TestIF_File.iAutoAlignmentShuttleHotplateEvent | AutoAlignmentCK_AfterOneCycle;

    //----------------------------------------------------------------------------------------------------------
    TestIF_File.iAutoAlignmentTrayEvent_Z=0;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup_Z")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent_Z = TestIF_File.iAutoAlignmentTrayEvent_Z | AutoAlignmentTray_ChangeSetupFile;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome_Z")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent_Z = TestIF_File.iAutoAlignmentTrayEvent_Z | AutoAlignmentTray_AfterHome;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart_Z")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent_Z = TestIF_File.iAutoAlignmentTrayEvent_Z | AutoAlignmentTray_InitialStart;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray_Z")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent_Z = TestIF_File.iAutoAlignmentTrayEvent_Z | AutoAlignmentTray_EveryTray;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle_Z")->Checked==true)
        TestIF_File.iAutoAlignmentTrayEvent_Z = TestIF_File.iAutoAlignmentTrayEvent_Z | AutoAlignmentTray_AfterOneCycle;

    TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z=0;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup_Z")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z  =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z | AutoAlignmentCK_ChangeSetupFile;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome_Z")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z  =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z | AutoAlignmentCK_AfterHome;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart_Z")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z | AutoAlignmentCK_InitialStart;
    if(filerw::EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle_Z")->Checked==true)
        TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z | AutoAlignmentCK_AfterOneCycle;
}   // golden AutoAlignment/AutoAlignment.cpp:1325-1371（逐字；fAutoAlignment->cb* → 具名替身）

using filerw::EL;

// 前置宣告
static void AA_TfAutoAlignment();
static void AA_DoIniDataToForm();
static void AA_spbSaveClick();

// golden AutoAlignment/AutoAlignment.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void AA_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfAutoAlignment", "cbInArmCommand")->Items;
      s->Add("Load File");
      s->Add("Teach CCD Center");
      s->Add("Teach CCD Left");
      s->Add("Teach CCD Right");
      s->Add("Teach CCD Up");
      s->Add("Teach CCD Down");
      s->Add("Teach CCD Golden Pattern");
      s->Add("Teach CCD Tray Pattern");
      s->Add("Teach CCD HotPlate Pattern");
      s->Add("Teach CCD Shuttle Pattern");
      s->Add("Wait Teach Reply");
      s->Add("Auto Get Golden Postion");
      s->Add("Auto Get Tray Postion");
      s->Add("Auto Get Hotplate Postion");
      s->Add("Auto Get Shuttle Postion");
      s->Add("Teach CCD AutoClean Pattern");
      s->Add("Auto Get AutoClean Postion");
    }
    { TStringList* s = EL<TComboBox>("TfAutoAlignment", "cbOutArmCommand")->Items;
      s->Add("Load File");
      s->Add("Teach CCD Center");
      s->Add("Teach CCD Left");
      s->Add("Teach CCD Right");
      s->Add("Teach CCD Up");
      s->Add("Teach CCD Down");
      s->Add("Teach CCD Golden Pattern");
      s->Add("Teach CCD Tray Pattern");
      s->Add("Teach CCD HotPlate Pattern");
      s->Add("Teach CCD Shuttle Pattern");
      s->Add("Wait Teach Reply");
      s->Add("Auto Get Golden Postion");
      s->Add("Auto Get Tray Postion");
      s->Add("Auto Get Hotplate Postion");
      s->Add("Auto Get Shuttle Postion");
      s->Add("Teach CCD AutoClean Pattern");
      s->Add("Auto Get AutoClean Postion");
    }
}

// golden AutoAlignment/AutoAlignment.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kAA_ParentOf[][2] = {
    {"GroupBox1", "GroupBox3"},
    {"GroupBox2", "GroupBox3"},
    {"GroupBox3", "Panel1"},
    {"GroupBox4", "GroupBox3"},
    {"GroupBox46", "GroupBox47"},
    {"GroupBox47", "Panel1"},
    {"GroupBox49", "TabSheet34"},
    {"GroupBox50", "TabSheet35"},
    {"GroupBox53", "TabSheet33"},
    {"GroupBox54", "TabSheet33"},
    {"GroupBox57", "TabSheet36"},
    {"GroupBox58", "TabSheet36"},
    {"PageControl18", "Panel1"},
    {"PageControl19", "GroupBox47"},
    {"TabSheet33", "PageControl19"},
    {"TabSheet34", "PageControl18"},
    {"TabSheet35", "PageControl18"},
    {"TabSheet36", "PageControl19"},
    {"cbAOA_UseFix1", "pnlUseFixTray"},
    {"cbAOA_UseFix2", "pnlUseFixTray"},
    {"cbAOA_UseFix3", "pnlUseFixTray"},
    {"cbAOA_UseFix4", "pnlUseFixTray"},
    {"cbAOA_UseFix5", "pnlUseFixTray"},
    {"cbAOA_UseFix6", "pnlUseFixTray"},
    {"cbAutoAlignmentCK_AfterHome", "grpInputArea"},
    {"cbAutoAlignmentCK_AfterHome_Z", "grpInputArea"},
    {"cbAutoAlignmentCK_ChangeSetup", "grpInputArea"},
    {"cbAutoAlignmentCK_ChangeSetup_Z", "grpInputArea"},
    {"cbAutoAlignmentCK_InitialStart", "grpInputArea"},
    {"cbAutoAlignmentCK_InitialStart_Z", "grpInputArea"},
    {"cbAutoAlignmentCK_OneCycle", "grpInputArea"},
    {"cbAutoAlignmentCK_OneCycle_Z", "grpInputArea"},
    {"cbAutoAlignmentTray_AfterHome", "GroupBox46"},
    {"cbAutoAlignmentTray_AfterHome_Z", "GroupBox46"},
    {"cbAutoAlignmentTray_ChangeSetup", "GroupBox46"},
    {"cbAutoAlignmentTray_ChangeSetup_Z", "GroupBox46"},
    {"cbAutoAlignmentTray_EveryTray", "GroupBox46"},
    {"cbAutoAlignmentTray_EveryTray_Z", "GroupBox46"},
    {"cbAutoAlignmentTray_InitialStart", "GroupBox46"},
    {"cbAutoAlignmentTray_InitialStart_Z", "GroupBox46"},
    {"cbAutoAlignmentTray_OneCycle", "GroupBox46"},
    {"cbAutoAlignmentTray_OneCycle_Z", "GroupBox46"},
    {"cbEnabledAutoAlignment", "Panel1"},
    {"edAOADecodetimeout", "GroupBox3"},
    {"edAutoAlignmentCK_InArmZPickUpOffset", "GroupBox57"},
    {"edAutoAlignmentCK_InArmZRealaseOffset", "GroupBox57"},
    {"edAutoAlignmentCK_OutArmZPickUpOffset", "GroupBox58"},
    {"edAutoAlignmentCK_OutArmZRealaseOffset", "GroupBox58"},
    {"edAutoAlignmentTray_InArmZPickUpOffset", "GroupBox53"},
    {"edAutoAlignmentTray_InArmZRealaseOffset", "GroupBox53"},
    {"edAutoAlignmentTray_OutArmZPickUpOffset", "GroupBox54"},
    {"edAutoAlignmentTray_OutArmZRealaseOffset", "GroupBox54"},
    {"edAutoAlignment_DeviceThick", "GroupBox47"},
    {"edtAlignmentPlatePointX", "GroupBox50"},
    {"edtAlignmentPlatePointY", "GroupBox50"},
    {"edtAlignmentPointX", "GroupBox49"},
    {"edtAlignmentPointY", "GroupBox49"},
    {"edtFileName", "GroupBox4"},
    {"edtInArmAddress", "GroupBox1"},
    {"edtInArmPort", "GroupBox1"},
    {"edtOutArmAddress", "GroupBox2"},
    {"edtOutArmPort", "GroupBox2"},
    {"grpInputArea", "GroupBox47"},
    {"pnlUseFixTray", "GroupBox47"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void AA_CreateContainerProxies()
{
    EL<TGroupBox>("TfAutoAlignment", "GroupBox1");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox2");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox3");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox4");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox46");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox47");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox49");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox50");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox53");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox54");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox57");
    EL<TGroupBox>("TfAutoAlignment", "GroupBox58");
    EL<TPageControl>("TfAutoAlignment", "PageControl18");
    EL<TPageControl>("TfAutoAlignment", "PageControl19");
    EL<TPanel>("TfAutoAlignment", "Panel1");
    EL<TTabSheet>("TfAutoAlignment", "TabSheet33");
    EL<TTabSheet>("TfAutoAlignment", "TabSheet34");
    EL<TTabSheet>("TfAutoAlignment", "TabSheet35");
    EL<TTabSheet>("TfAutoAlignment", "TabSheet36");
    EL<TGroupBox>("TfAutoAlignment", "grpInputArea");
    EL<TPanel>("TfAutoAlignment", "pnlUseFixTray");
    filerw::ELSetParents("TfAutoAlignment", kAA_ParentOf, (int)(sizeof(kAA_ParentOf) / sizeof(kAA_ParentOf[0])));
}

// golden AutoAlignment/AutoAlignment.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void AA_DfmState()
{
    EL<TPageControl>("TfAutoAlignment", "PageControl19")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome")->Enabled = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart_Z")->Visible = false;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle_Z")->Visible = false;
    EL<TEdit>("TfAutoAlignment", "edAOADecodetimeout")->Visible = false;
    EL<TEdit>("TfAutoAlignment", "edAutoAlignment_DeviceThick")->Visible = false;
    EL<TEdit>("TfAutoAlignment", "edAOADecodetimeout")->Text = "3000";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZPickUpOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZRealaseOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZPickUpOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZRealaseOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZPickUpOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZRealaseOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZPickUpOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZRealaseOffset")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edAutoAlignment_DeviceThick")->Text = "0";
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointX")->Text = "2";
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointY")->Text = "4";
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointX")->Text = "2";
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointY")->Text = "4";
    EL<TEdit>("TfAutoAlignment", "edtFileName")->Text = "edtFileName";
    EL<TEdit>("TfAutoAlignment", "edtInArmAddress")->Text = "192.168.1.1";
    EL<TEdit>("TfAutoAlignment", "edtInArmPort")->Text = "6001";
    EL<TEdit>("TfAutoAlignment", "edtOutArmAddress")->Text = "192.168.1.1";
    EL<TEdit>("TfAutoAlignment", "edtOutArmPort")->Text = "6001";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kAA_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "GroupBox4", "GroupBox46", "GroupBox47",
    "GroupBox49", "GroupBox50", "GroupBox53", "GroupBox54", "GroupBox57", "GroupBox58",
    "PageControl18", "PageControl19", "Panel1", "TabSheet33", "TabSheet34", "TabSheet35",
    "TabSheet36", "cbAOA_UseFix1", "cbAOA_UseFix2", "cbAOA_UseFix3", "cbAOA_UseFix4", "cbAOA_UseFix5",
    "cbAOA_UseFix6", "cbAutoAlignmentCK_AfterHome", "cbAutoAlignmentCK_AfterHome_Z", "cbAutoAlignmentCK_ChangeSetup", "cbAutoAlignmentCK_ChangeSetup_Z", "cbAutoAlignmentCK_InitialStart",
    "cbAutoAlignmentCK_InitialStart_Z", "cbAutoAlignmentCK_OneCycle", "cbAutoAlignmentCK_OneCycle_Z", "cbAutoAlignmentTray_AfterHome", "cbAutoAlignmentTray_AfterHome_Z", "cbAutoAlignmentTray_ChangeSetup",
    "cbAutoAlignmentTray_ChangeSetup_Z", "cbAutoAlignmentTray_EveryTray", "cbAutoAlignmentTray_EveryTray_Z", "cbAutoAlignmentTray_InitialStart", "cbAutoAlignmentTray_InitialStart_Z", "cbAutoAlignmentTray_OneCycle",
    "cbAutoAlignmentTray_OneCycle_Z", "cbEnabledAutoAlignment", "grpInputArea", "pnlUseFixTray",
};

// 存檔流程（spbSaveClick）讀的替身 22 個 —— FileRW/TestIF_File_AutoAlignment.cpp 用它決定頁面必須送哪些值
static const char* const kAA_SaveReads[] = {
    "cbAOA_UseFix1", "cbAOA_UseFix2", "cbAOA_UseFix3", "cbAOA_UseFix4", "cbAOA_UseFix5", "cbAOA_UseFix6",
    "cbEnabledAutoAlignment", "edAOADecodetimeout", "edAutoAlignmentCK_InArmZPickUpOffset", "edAutoAlignmentCK_InArmZRealaseOffset", "edAutoAlignmentCK_OutArmZPickUpOffset", "edAutoAlignmentCK_OutArmZRealaseOffset",
    "edAutoAlignmentTray_InArmZPickUpOffset", "edAutoAlignmentTray_InArmZRealaseOffset", "edAutoAlignmentTray_OutArmZPickUpOffset", "edAutoAlignmentTray_OutArmZRealaseOffset", "edAutoAlignment_DeviceThick", "edtAlignmentPlatePointX",
    "edtAlignmentPlatePointY", "edtAlignmentPointX", "edtAlignmentPointY", "edtFileName",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void AA_CreateSaveProxies()
{
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix1");
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix2");
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix3");
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix4");
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix5");
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix6");
    EL<TCheckBox>("TfAutoAlignment", "cbEnabledAutoAlignment");
    EL<TEdit>("TfAutoAlignment", "edAOADecodetimeout");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZPickUpOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZRealaseOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZPickUpOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZRealaseOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZPickUpOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZRealaseOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZPickUpOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZRealaseOffset");
    EL<TEdit>("TfAutoAlignment", "edAutoAlignment_DeviceThick");
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointX");
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointY");
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointX");
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointY");
    EL<TEdit>("TfAutoAlignment", "edtFileName");
}

// golden AutoAlignment/AutoAlignment.cpp:48  TfAutoAlignment::TfAutoAlignment(TComponent* Owner)
static void AA_TfAutoAlignment()
{

    bInArmSendTeachCmd=false;
    bOutArmSendTeachCmd=false;

    ;   // golden AutoAlignment/AutoAlignment.cpp:54-113 取代：CCD 通訊指令字串表（golden 檔案層級全域 sSendCMD／sReceiveOKCMD／sReceiveNGCMD[TOTAL]，enum ALIGNMENTCMD 在 golden AutoAlignment.h）：只給 InArmSendCommand／OutArmSendCommand／*GetResult（CCD TCP 通訊，移植樹沒有，Jimmy）用；不是讀寫檔
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:54-113 -- CCD 通訊指令字串表（golden 檔案層級全域 sSendCMD／sReceiveOKCMD／sReceiveNGCMD[TOTAL]，enum ALIGNMENTCMD 在 golden AutoAlignment.h）：只給 InArmSendCommand／OutArmSendCommand／*GetResult（CCD TCP 通訊，移植樹沒有，Jimmy）用；不是讀寫檔
    sSendCMD[LOAD_FILE]       = "@LF%s+";

    sSendCMD[UNIT_CENTER]     = "@GU,C1+";
    sSendCMD[UNIT_LEFT]       = "@GU,L1+";
    sSendCMD[UNIT_RIGHT]      = "@GU,R1+";
    sSendCMD[UNIT_UP]         = "@GU,U1+";
    sSendCMD[UNIT_DOWN]       = "@GU,D1+";

    sSendCMD[TEACH_GOLDEN]    = "@TH,GD+";
    sSendCMD[TEACH_TRAY]      = "@TH,TY+";
    sSendCMD[TEACH_HOTPLATE]  = "@TH,HP+";
    sSendCMD[TEACH_SHUTTLE]   = "@TH,ST+";
    sSendCMD[TEACH_AUTOCLEAN] = "@TH,AC+";          //KenHsieh 20211214 : AOA add AutoClean
    sSendCMD[WAIT_TEACH_GOLDEN] = "@TH,XX+";

    sSendCMD[AUTO_GOLDEN]     = "@AT,GD+";
    sSendCMD[AUTO_TRAY]       = "@AT,TY+";
    sSendCMD[AUTO_HOTPLATE]   = "@AT,HP+";
    sSendCMD[AUTO_SHUTTLE]    = "@AT,ST+";
    sSendCMD[AUTO_AUTOCLEAN]  = "@AT,AC+";          //KenHsieh 20211214 : AOA add AutoClean
  //-----------------------------------------------------------
    sReceiveOKCMD[LOAD_FILE]       = "@LFOK+";

    sReceiveOKCMD[UNIT_CENTER]     = "@GU,C1,";
    sReceiveOKCMD[UNIT_LEFT]       = "@GU,L1,";
    sReceiveOKCMD[UNIT_RIGHT]      = "@GU,R1,";
    sReceiveOKCMD[UNIT_UP]         = "@GU,U1,";
    sReceiveOKCMD[UNIT_DOWN]       = "@GU,D1,";

    sReceiveOKCMD[TEACH_GOLDEN]    = "@TH,GD,";
    sReceiveOKCMD[TEACH_TRAY]      = "@TH,TY,";
    sReceiveOKCMD[TEACH_HOTPLATE]  = "@TH,HP,";
    sReceiveOKCMD[TEACH_SHUTTLE]   = "@TH,ST,";
    sReceiveOKCMD[TEACH_AUTOCLEAN] = "@TH,AC,";     //KenHsieh 20211214 : AOA add AutoClean

    sReceiveOKCMD[AUTO_GOLDEN]     = "@AT,GD,";
    sReceiveOKCMD[AUTO_TRAY]       = "@AT,TY,";
    sReceiveOKCMD[AUTO_HOTPLATE]   = "@AT,HP,";
    sReceiveOKCMD[AUTO_SHUTTLE]    = "@AT,ST,";
    sReceiveOKCMD[AUTO_AUTOCLEAN]  = "@AT,AC,";     //KenHsieh 20211214 : AOA add AutoClean
  //-----------------------------------------------------------
    sReceiveNGCMD[LOAD_FILE]       = "@LFNG+";

    sReceiveNGCMD[UNIT_CENTER]     = "@GU,C1,999";//"@GU,C2+";
    sReceiveNGCMD[UNIT_LEFT]       = "@GU,L1,999";
    sReceiveNGCMD[UNIT_RIGHT]      = "@GU,R1,999";
    sReceiveNGCMD[UNIT_UP]         = "@GU,U1,999";
    sReceiveNGCMD[UNIT_DOWN]       = "@GU,D1,999";

    sReceiveNGCMD[TEACH_GOLDEN]    = "@TH,GD,999";
    sReceiveNGCMD[TEACH_TRAY]      = "@TH,TY,999";
    sReceiveNGCMD[TEACH_HOTPLATE]  = "@TH,HP,999";
    sReceiveNGCMD[TEACH_SHUTTLE]   = "@TH,ST,999";
    sReceiveNGCMD[TEACH_AUTOCLEAN] = "@TH,AC,999";  //KenHsieh 20211214 : AOA add AutoClean

    sReceiveNGCMD[AUTO_GOLDEN]     = "@AT,GD,999";
    sReceiveNGCMD[AUTO_TRAY]       = "@AT,TY,999";
    sReceiveNGCMD[AUTO_HOTPLATE]   = "@AT,HP,999";
    sReceiveNGCMD[AUTO_SHUTTLE]    = "@AT,ST,999";
    sReceiveNGCMD[AUTO_AUTOCLEAN]  = "@AT,AC,999";  //KenHsieh 20211214 : AOA add AutoClean
#endif // GATE (S12-C save)

    //"@TH,TY,X,Y+"
    EL<TEdit>("TfAutoAlignment", "edtInArmAddress")->Text     =CheckAndReadIniDataGeneral("Auto_Alignment",    "AUTO_ALIGNMENT_CCD1_ADRESS"       , AnsiString("172.16.8.131"));
    EL<TEdit>("TfAutoAlignment", "edtInArmPort")->Text        =CheckAndReadIniDataGeneral("Auto_Alignment",    "AUTO_ALIGNMENT_CCD1_PORT"         , AnsiString("5001"));            //KenHsieh 20211204 : 修改預設port "6001->5001"

    EL<TEdit>("TfAutoAlignment", "edtOutArmAddress")->Text    =CheckAndReadIniDataGeneral("Auto_Alignment",    "AUTO_ALIGNMENT_CCD2_ADRESS"       , AnsiString("172.16.8.131"));
    EL<TEdit>("TfAutoAlignment", "edtOutArmPort")->Text       =CheckAndReadIniDataGeneral("Auto_Alignment",    "AUTO_ALIGNMENT_CCD2_PORT"         , AnsiString("5002"));            //KenHsieh 20211204 : 修改預設port "6002->5002

    ;   // golden AutoAlignment/AutoAlignment.cpp:122-125 取代：TClientSocket 的 Address／Port（CCD TCP 連線，移植樹沒有 ClientSocket1/2）：值留在 edtIn/OutArmAddress、edtIn/OutArmPort 替身裡（上面四行 CheckAndReadIniDataGeneral 照 golden 讀／補寫 Gerneral.ini），接 CCD 通訊時從替身取
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:122-125 -- TClientSocket 的 Address／Port（CCD TCP 連線，移植樹沒有 ClientSocket1/2）：值留在 edtIn/OutArmAddress、edtIn/OutArmPort 替身裡（上面四行 CheckAndReadIniDataGeneral 照 golden 讀／補寫 Gerneral.ini），接 CCD 通訊時從替身取
    EL<TControl>("TfAutoAlignment", "ClientSocket1")->Address  = EL<TEdit>("TfAutoAlignment", "edtInArmAddress")->Text;
    EL<TControl>("TfAutoAlignment", "ClientSocket1")->Port     = atoi(EL<TEdit>("TfAutoAlignment", "edtInArmPort")->Text.c_str());
    EL<TControl>("TfAutoAlignment", "ClientSocket2")->Address  = EL<TEdit>("TfAutoAlignment", "edtOutArmAddress")->Text;
    EL<TControl>("TfAutoAlignment", "ClientSocket2")->Port     = atoi(EL<TEdit>("TfAutoAlignment", "edtOutArmPort")->Text.c_str());
#endif // GATE (S12-C save)

}

// golden AutoAlignment/AutoAlignment.cpp:1433  TfAutoAlignment::DoIniDataToForm()
static void AA_DoIniDataToForm()
{

    EL<TCheckBox>("TfAutoAlignment", "cbEnabledAutoAlignment")->Checked = TestIF_File.bEnableAutoAlignment;

    EL<TEdit>("TfAutoAlignment", "edtFileName")->Text      = TestIF_File.AutoAlignmentFileName;                 //Kenhsieh 20210922 : save AutoAlignment FileName

    TestIF_File.iAutoAlignmentTrayEvent = TestIF_File.iAutoAlignmentTrayEvent | AutoAlignmentTray_AfterHome;

    //cbAutoAlignmentTrayEvent->ItemIndex = TestIF.iAutoAlignmentTrayEvent;             //0:Initial Start 1:After Home 2:Every Tray
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup")->Checked    = TestIF_File.iAutoAlignmentTrayEvent & AutoAlignmentTray_ChangeSetupFile;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome")->Checked      = TestIF_File.iAutoAlignmentTrayEvent & AutoAlignmentTray_AfterHome;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart")->Checked   = TestIF_File.iAutoAlignmentTrayEvent & AutoAlignmentTray_InitialStart;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray")->Checked      = TestIF_File.iAutoAlignmentTrayEvent & AutoAlignmentTray_EveryTray;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle")->Checked       = TestIF_File.iAutoAlignmentTrayEvent & AutoAlignmentTray_AfterOneCycle;

    TestIF_File.iAutoAlignmentShuttleHotplateEvent = TestIF_File.iAutoAlignmentShuttleHotplateEvent | AutoAlignmentCK_AfterHome;

    //cbAutoAlignmentShuttleHotplateEvent->ItemIndex = TestIF.iAutoAlignmentShuttleHotplateEvent;  //0:Initial Start 1:After Home 2:Every Tray
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup")->Checked      =  TestIF_File.iAutoAlignmentShuttleHotplateEvent & AutoAlignmentCK_ChangeSetupFile;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome")->Checked        =  TestIF_File.iAutoAlignmentShuttleHotplateEvent & AutoAlignmentCK_AfterHome;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart")->Checked     =  TestIF_File.iAutoAlignmentShuttleHotplateEvent & AutoAlignmentCK_InitialStart;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle")->Checked         =  TestIF_File.iAutoAlignmentShuttleHotplateEvent & AutoAlignmentCK_AfterOneCycle;

    //cbAutoAlignmentTrayEvent->ItemIndex = TestIF.iAutoAlignmentTrayEvent;             //0:Initial Start 1:After Home 2:Every Tray
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup_Z")->Checked    = TestIF_File.iAutoAlignmentTrayEvent_Z & AutoAlignmentTray_ChangeSetupFile;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome_Z")->Checked      = TestIF_File.iAutoAlignmentTrayEvent_Z & AutoAlignmentTray_AfterHome;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart_Z")->Checked   = TestIF_File.iAutoAlignmentTrayEvent_Z & AutoAlignmentTray_InitialStart;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray_Z")->Checked      = TestIF_File.iAutoAlignmentTrayEvent_Z & AutoAlignmentTray_EveryTray;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle_Z")->Checked       = TestIF_File.iAutoAlignmentTrayEvent_Z & AutoAlignmentTray_AfterOneCycle;

    //cbAutoAlignmentShuttleHotplateEvent->ItemIndex = TestIF.iAutoAlignmentShuttleHotplateEvent;  //0:Initial Start 1:After Home 2:Every Tray
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup_Z")->Checked      =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z & AutoAlignmentCK_ChangeSetupFile;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome_Z")->Checked        =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z & AutoAlignmentCK_AfterHome;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart_Z")->Checked     =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z & AutoAlignmentCK_InitialStart;
    EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle_Z")->Checked         =  TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z & AutoAlignmentCK_AfterOneCycle;

    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointX")->Text        = TestIF_File.iAlignmentPointX;             //Col
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPointY")->Text        = TestIF_File.iAlignmentPointY;             //Row

    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointX")->Text   = TestIF_File.iAlignmentPlatePointX;        //Col
    EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointY")->Text   = TestIF_File.iAlignmentPlatePointY;        //Row

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_ChangeSetup_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_AfterHome_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_InitialStart_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_EveryTray_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentTray_OneCycle_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_ChangeSetup_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_AfterHome_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_InitialStart_Z")->Enabled=false;

    if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle")->Checked==true)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle_Z")->Enabled=true;
    else if(EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle")->Checked==false)
        EL<TCheckBox>("TfAutoAlignment", "cbAutoAlignmentCK_OneCycle_Z")->Enabled=false;

    EL<TEdit>("TfAutoAlignment", "edAutoAlignment_DeviceThick")->Text             = TestIF_File.iAutoAlignment_DeviceThick;

    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZRealaseOffset")->Text = TestIF_File.iAutoAlignmentTray_InArmZRealaseOffset;
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZPickUpOffset")->Text  = TestIF_File.iAutoAlignmentTray_InArmZPickUpOffset;

    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZRealaseOffset")->Text= TestIF_File.iAutoAlignmentTray_OutArmZRealaseOffset;
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZPickUpOffset")->Text = TestIF_File.iAutoAlignmentTray_OutArmZPickUpOffset;

    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZRealaseOffset")->Text   = TestIF_File.iAutoAlignmentCK_InArmZRealaseOffset;
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZPickUpOffset")->Text    = TestIF_File.iAutoAlignmentCK_InArmZPickUpOffset;

    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZRealaseOffset")->Text  = TestIF_File.iAutoAlignmentCK_OutArmZRealaseOffset;
    EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZPickUpOffset")->Text   = TestIF_File.iAutoAlignmentCK_OutArmZPickUpOffset;

    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix1")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix1]];   // golden AutoAlignment/AutoAlignment.cpp:1534-1534 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1534-1534 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix1->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix1]];
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix2")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix2]];   // golden AutoAlignment/AutoAlignment.cpp:1535-1535 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1535-1535 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix2->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix2]];
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix3")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix3]];   // golden AutoAlignment/AutoAlignment.cpp:1536-1536 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1536-1536 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix3->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix3]];
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix4")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix4]];   // golden AutoAlignment/AutoAlignment.cpp:1537-1537 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1537-1537 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix4->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix4]];
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix5")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix5]];   // golden AutoAlignment/AutoAlignment.cpp:1538-1538 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1538-1538 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix5->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix5]];
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix6")->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix6]];   // golden AutoAlignment/AutoAlignment.cpp:1539-1539 取代：golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1539-1539 -- golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    fAutoAlignment->cbAOA_UseFix6->Checked=TestIF_File.iAutoAlignment_UseFix[iAutoIndex[eFix6]];
#endif // GATE (S12-C save)

    EL<TEdit>("TfAutoAlignment", "edAOADecodetimeout")->Text = TestIF_File.iAOA_DecodeTimeOut;                  //Ifor 20211221 add: AOA Decode Time Out

}

// golden AutoAlignment/AutoAlignment.cpp:1573  TfAutoAlignment::spbSaveClick(TObject *Sender)
static void AA_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");
    AnsiString sGroup;

    sGroup="AutoAlignmrnt";
    AA_CheckAutoAlignmentEvent();   // golden AutoAlignment/AutoAlignment.cpp:1579-1579 取代：golden 自由函式 CheckAutoAlignmentEvent（:1325）搬成本 TU 的 AA_CheckAutoAlignmentEvent（見 members；逐字，fAutoAlignment->cb* → 具名替身）
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1579-1579 -- golden 自由函式 CheckAutoAlignmentEvent（:1325）搬成本 TU 的 AA_CheckAutoAlignmentEvent（見 members；逐字，fAutoAlignment->cb* → 具名替身）
    CheckAutoAlignmentEvent();
#endif // GATE (S12-C save)

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD==false)
    {
        return;
    }

    filerw::ELMark("AA_WriteIniData"); WriteIniData(szDir, sGroup, "Enable Auto Alignment",                  (EL<TCheckBox>("TfAutoAlignment", "cbEnabledAutoAlignment")->Checked)?1:0);   // golden AutoAlignment/AutoAlignment.cpp:1586-1586 取代：存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 CheckAutoAlignmentEvent（改記憶體）與「沒有 CCD 就 return」
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1586-1586 -- 存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 CheckAutoAlignmentEvent（改記憶體）與「沒有 CCD 就 return」
    WriteIniData(szDir, sGroup, "Enable Auto Alignment",                  (EL<TCheckBox>("TfAutoAlignment", "cbEnabledAutoAlignment")->Checked)?1:0);
#endif // GATE (S12-C save)

    WriteIniData(szDir, sGroup, "AutoAlignment FileName",                 EL<TEdit>("TfAutoAlignment", "edtFileName")->Text                    );     //Kenhsieh 20210922 : save AutoAlignment FileName

    WriteIniData(szDir, sGroup, "AutoAlignmentTrayEvent",                 TestIF_File.iAutoAlignmentTrayEvent                         );
    WriteIniData(szDir, sGroup, "AutoAlignmentShuttleHotplateEvent",      TestIF_File.iAutoAlignmentShuttleHotplateEvent              );
    WriteIniData(szDir, sGroup, "AutoAlignmentTrayEvent_Z",               TestIF_File.iAutoAlignmentTrayEvent_Z                       );
    WriteIniData(szDir, sGroup, "AutoAlignmentShuttleHotplateEvent_Z",    TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z            );

    WriteIniData(szDir, sGroup, "Alignment Point X",                      atoi(EL<TEdit>("TfAutoAlignment", "edtAlignmentPointX")->Text.c_str())                      );
    WriteIniData(szDir, sGroup, "Alignment Point Y",                      atoi(EL<TEdit>("TfAutoAlignment", "edtAlignmentPointY")->Text.c_str())                      );

    WriteIniData(szDir, sGroup, "Alignment Plate Point X",                atoi(EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointX")->Text.c_str())                 );
    WriteIniData(szDir, sGroup, "Alignment Plate Point Y",                atoi(EL<TEdit>("TfAutoAlignment", "edtAlignmentPlatePointY")->Text.c_str())                 );

    WriteIniData(szDir, sGroup, "Alignment DeviceThick",                  atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignment_DeviceThick")->Text.c_str())             );

    WriteIniData(szDir, sGroup, "Alignment InArmZRealaseOffset",          atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZRealaseOffset")->Text.c_str()) );
    WriteIniData(szDir, sGroup, "Alignment InArmZPickUpOffset",           atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_InArmZPickUpOffset")->Text.c_str())  );

    WriteIniData(szDir, sGroup, "Alignment OutArmZRealaseOffset",         atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZRealaseOffset")->Text.c_str()));
    WriteIniData(szDir, sGroup, "Alignment OutArmZPickUpOffset",          atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentTray_OutArmZPickUpOffset")->Text.c_str()) );

    WriteIniData(szDir, sGroup, "Alignment InArmZRealaseOffset",          atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZRealaseOffset")->Text.c_str())   );
    WriteIniData(szDir, sGroup, "Alignment InArmZPickUpOffset",           atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_InArmZPickUpOffset")->Text.c_str())    );

    WriteIniData(szDir, sGroup, "Alignment OutArmZRealaseOffset",         atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZRealaseOffset")->Text.c_str())  );
    WriteIniData(szDir, sGroup, "Alignment OutArmZPickUpOffset",          atoi(EL<TEdit>("TfAutoAlignment", "edAutoAlignmentCK_OutArmZPickUpOffset")->Text.c_str())   );

    WriteIniData(szDir, sGroup, "Alignment Use Fix1",                    (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix1")->Checked)?1:0                       );
    WriteIniData(szDir, sGroup, "Alignment Use Fix2",                    (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix2")->Checked)?1:0                       );
    WriteIniData(szDir, sGroup, "Alignment Use Fix3",                    (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix3")->Checked)?1:0                       );
    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20240428 : Add for HT9011 AOA
    {
        WriteIniData(szDir, sGroup, "Alignment Use Fix4",                (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix4")->Checked)?1:0                       );
        WriteIniData(szDir, sGroup, "Alignment Use Fix5",                (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix5")->Checked)?1:0                       );
        WriteIniData(szDir, sGroup, "Alignment Use Fix6",                (EL<TCheckBox>("TfAutoAlignment", "cbAOA_UseFix6")->Checked)?1:0                       );
    }

    WriteIniData(szDir, sGroup, "Alignment Decode Time Out",              atoi(EL<TEdit>("TfAutoAlignment", "edAOADecodetimeout")->Text.c_str())          );    //KenHsieh 20220103 : AOA Decode Time Out
    fAutoAlignment->ReadFile();   // golden AutoAlignment/AutoAlignment.cpp:1626-1626 取代：golden ReadFile（:1373）：移植樹 AutoAlignment/AutoAlignment.cpp 已逐字翻好（開機鏈也叫它）→ 叫同一支，全樹一份讀檔器
#if 0 // GATE (S12-C save) golden AutoAlignment/AutoAlignment.cpp:1626-1626 -- golden ReadFile（:1373）：移植樹 AutoAlignment/AutoAlignment.cpp 已逐字翻好（開機鏈也叫它）→ 叫同一支，全樹一份讀檔器
    ReadFile();
#endif // GATE (S12-C save)

}

