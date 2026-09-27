// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 ContactForce 的讀寫檔（HTEditList： → D:\HT9045\system\ContactInfo.ini（[SLK Type]、Diameter_*、DieForceDiameter_*；讀＋寫）、Gerneral.ini [System] EP_MAXKPA／EP_MAXA／EP_MINMPA／EP_MINA_FeedBack＋_1032 四鍵（寫）、EPDual_* 四鍵（只讀）、[Test Arm] dIndexZOffset[0..1][0..14]（讀＋寫）、SaveLastSetIni：Gerneral.ini [Version]、config.ini（[Contact Force] 30MM_10KG…）、LastSet.ini、configByRecipe.ini）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden ContactForce.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 88 個（header 宣告 190 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "FileRW/ContactForce_Panels.h"


// golden ContactForce.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;                     // golden ContactForce.h:334 —— 本 TU 自己的（網頁沒有關頁事件；golden adam6024.cpp:1822 ADAM_WriteVoltage 看它，移植樹 ADAM_* 是空殼、沒有讀者）
static AnsiString FileName;                  // golden ContactForce.h:335（建構子 :428 = "D:\\HT9045\\system\\ContactInfo.ini"）
static bool bHasFile=false;                  // golden ContactForce.h:336
static std::vector<THTSLKClass *> SLKClass;                                  // golden ContactForce.h:341（面板類別：FileRW/ContactForce_Panels.h）
static std::vector<THTSLKIndClass *> SLKIndClass;                            // golden ContactForce.h:342
static TStringList *slSLKTypeInd=NULL;       // golden ContactForce.h:343（建構子 :494 new；ReadFile／WriteFile 用 Count）
static TStringList *slSLKTypeIndVisible=NULL;   // golden ContactForce.h:344
static TStringList *slSLKType=NULL;          // golden ContactForce.h:345（建構子 :456 new、:646-648 delete＋NULL）
static TStringList *slSLKTypeVisible=NULL;   // golden ContactForce.h:346（:460 new、:649-651 delete＋NULL）
static std::vector<THTDieForceSLKClass *> DieForceSLKClass;                  // golden ContactForce.h:349
static TStringList *slDieForceSLKType=NULL;  // golden ContactForce.h:350
static TStringList *slDieForceSLKTypeVisible=NULL;   // golden ContactForce.h:351
static std::vector<THTDieForceOneByOneSLKClass *> DieForceOneByOneSLKClass;  // golden ContactForce.h:354
static TStringList *slDieForceOneByOneSLKType=NULL;          // golden ContactForce.h:355
static TStringList *slDieForceOneByOneSLKTypeVisible=NULL;   // golden ContactForce.h:356
static TEdit *IndexZOffsetEdit[2][16];       // golden ContactForce.cpp:19 檔案層全域（ReadFile :972-980 指派成 edtArm1Offset_01…edtArm2Offset_15 的替身；指派前是 NULL，同 golden）

// golden ContactForce.cpp 檔案層級全域（本表單私用，照抄成 static）
static bool bNeedWriteFile=false;                                                      //Ifor 20200904 add:KYEC 缸徑自動轉換 30=>28  60=>58   // golden ContactForce.cpp:18

using filerw::EL;

// 前置宣告
static void CF_TfContactForce();
static void CF_FormShow();
static void CF_btSaveClick();
static void CF_ReadFile();
static void CF_WriteFile();
static void CF_tb30mm_10kgChange();
static void CF_ShowValue();

// golden ContactForce.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void CF_DfmItems()
{
}

// golden ContactForce.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kCF_ParentOf[][2] = {
    {"GroupBox1", "tsNewMethod"},
    {"GroupBox2", "tsNewMethod"},
    {"GroupBox3", "tsNewMethod"},
    {"GroupBox4", "tsNewMethod"},
    {"GroupBox5", "tsEPCalibration"},
    {"GroupBox6", "grpEPForDieForce"},
    {"GroupBox7", "grpEPForDieForce"},
    {"TabSheet1", "PageControl1"},
    {"edD25_30mm", "tsD25"},
    {"edD25_40mm", "tsD25"},
    {"edD25_60mm", "tsD25"},
    {"edD60_56mm", "tsD25"},
    {"edMaxKpa", "gbEPSetting"},
    {"edMaxKpaDual", "GroupBox6"},
    {"edMaxKpa_1032", "gbEPSetting_1032"},
    {"edMaxMpaFB", "gbEPReadSetting"},
    {"edMaxMpaFBDual", "GroupBox7"},
    {"edMaxMpaFB_1032", "gbEPReadSetting_1032"},
    {"edMinMpa", "gbEPSetting"},
    {"edMinMpaDual", "GroupBox6"},
    {"edMinMpa_1032", "gbEPSetting_1032"},
    {"edtArm1Offset_01", "grpATCTempOffset"},
    {"edtArm1Offset_02", "grpATCTempOffset"},
    {"edtArm1Offset_03", "grpATCTempOffset"},
    {"edtArm1Offset_04", "grpATCTempOffset"},
    {"edtArm1Offset_05", "grpATCTempOffset"},
    {"edtArm1Offset_06", "grpATCTempOffset"},
    {"edtArm1Offset_07", "grpATCTempOffset"},
    {"edtArm1Offset_08", "grpATCTempOffset"},
    {"edtArm1Offset_09", "grpATCTempOffset"},
    {"edtArm1Offset_10", "grpATCTempOffset"},
    {"edtArm1Offset_11", "grpATCTempOffset"},
    {"edtArm1Offset_12", "grpATCTempOffset"},
    {"edtArm1Offset_13", "grpATCTempOffset"},
    {"edtArm1Offset_14", "grpATCTempOffset"},
    {"edtArm1Offset_15", "grpATCTempOffset"},
    {"edtArm2Offset_01", "grpATCTempOffset"},
    {"edtArm2Offset_02", "grpATCTempOffset"},
    {"edtArm2Offset_03", "grpATCTempOffset"},
    {"edtArm2Offset_04", "grpATCTempOffset"},
    {"edtArm2Offset_05", "grpATCTempOffset"},
    {"edtArm2Offset_06", "grpATCTempOffset"},
    {"edtArm2Offset_07", "grpATCTempOffset"},
    {"edtArm2Offset_08", "grpATCTempOffset"},
    {"edtArm2Offset_09", "grpATCTempOffset"},
    {"edtArm2Offset_10", "grpATCTempOffset"},
    {"edtArm2Offset_11", "grpATCTempOffset"},
    {"edtArm2Offset_12", "grpATCTempOffset"},
    {"edtArm2Offset_13", "grpATCTempOffset"},
    {"edtArm2Offset_14", "grpATCTempOffset"},
    {"edtArm2Offset_15", "grpATCTempOffset"},
    {"edtCurrentType", "grpAddNewKit"},
    {"edtCurrentTypeInd", "grpAddNewKitInd"},
    {"edtDieForceCurrentType", "grpDieForceAddNewKit"},
    {"edtDieForceVisible", "grpDieForceAddNewKit"},
    {"edtMinMpaFB", "gbEPReadSetting"},
    {"edtMinMpaFBDual", "GroupBox7"},
    {"edtMinMpaFB_1032", "gbEPReadSetting_1032"},
    {"edtVisible", "grpAddNewKit"},
    {"edtVisibleInd", "grpAddNewKitInd"},
    {"gbEPReadSetting", "GroupBox5"},
    {"gbEPReadSetting_1032", "grpEPForArm2"},
    {"gbEPSetting", "GroupBox5"},
    {"gbEPSetting_1032", "grpEPForArm2"},
    {"grpATCTempOffset", "tsContactHighOffset"},
    {"grpAddNewKit", "scrlbxDynamicKit"},
    {"grpAddNewKitInd", "scrlbxDynamicKitInd"},
    {"grpDieForceAddNewKit", "scrlbxDieForceDynamicKit"},
    {"grpEPForArm2", "tsEPCalibration"},
    {"grpEPForDieForce", "tsEPCalibration"},
    {"lab30mm10kgNum", "GroupBox1"},
    {"lab30mm60kgNum", "GroupBox1"},
    {"lab40mm10kgNum", "GroupBox2"},
    {"lab40mm60kgNum", "GroupBox2"},
    {"lab60mm10kgNum", "GroupBox3"},
    {"lab60mm60kgNum", "GroupBox3"},
    {"scrlbxDieForceDynamicKit", "tsDieForceDynamicKit"},
    {"scrlbxDynamicKit", "tsDynamicKit"},
    {"scrlbxDynamicKitInd", "TabSheet1"},
    {"tb30mm_10kg", "GroupBox1"},
    {"tb30mm_60kg", "GroupBox1"},
    {"tb40mm_10kg", "GroupBox2"},
    {"tb40mm_60kg", "GroupBox2"},
    {"tb56mm_10kg", "GroupBox4"},
    {"tb56mm_60kg", "GroupBox4"},
    {"tb60mm_10kg", "GroupBox3"},
    {"tb60mm_60kg", "GroupBox3"},
    {"tbD25_Index30mm", "tsD25"},
    {"tbD25_Index40mm", "tsD25"},
    {"tbD25_Index60mm", "tsD25"},
    {"tbD60_Index56mm", "tsD25"},
    {"tsContactHighOffset", "PageControl1"},
    {"tsD25", "PageControl1"},
    {"tsDieForceDynamicKit", "PageControl1"},
    {"tsDieForceOneByOneKit", "PageControl1"},
    {"tsDynamicKit", "PageControl1"},
    {"tsEPCalibration", "PageControl1"},
    {"tsNewMethod", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void CF_CreateContainerProxies()
{
    EL<TGroupBox>("TfContactForce", "GroupBox1");
    EL<TGroupBox>("TfContactForce", "GroupBox2");
    EL<TGroupBox>("TfContactForce", "GroupBox3");
    EL<TGroupBox>("TfContactForce", "GroupBox4");
    EL<TGroupBox>("TfContactForce", "GroupBox5");
    EL<TGroupBox>("TfContactForce", "GroupBox6");
    EL<TGroupBox>("TfContactForce", "GroupBox7");
    EL<TPageControl>("TfContactForce", "PageControl1");
    EL<TTabSheet>("TfContactForce", "TabSheet1");
    EL<TGroupBox>("TfContactForce", "gbEPReadSetting");
    EL<TGroupBox>("TfContactForce", "gbEPReadSetting_1032");
    EL<TGroupBox>("TfContactForce", "gbEPSetting");
    EL<TGroupBox>("TfContactForce", "gbEPSetting_1032");
    EL<TGroupBox>("TfContactForce", "grpATCTempOffset");
    EL<TGroupBox>("TfContactForce", "grpAddNewKit");
    EL<TGroupBox>("TfContactForce", "grpAddNewKitInd");
    EL<TGroupBox>("TfContactForce", "grpDieForceAddNewKit");
    EL<TGroupBox>("TfContactForce", "grpEPForArm2");
    EL<TGroupBox>("TfContactForce", "grpEPForDieForce");
    EL<TControl>("TfContactForce", "scrlbxDieForceDynamicKit");
    EL<TControl>("TfContactForce", "scrlbxDynamicKit");
    EL<TControl>("TfContactForce", "scrlbxDynamicKitInd");
    EL<TTabSheet>("TfContactForce", "tsContactHighOffset");
    EL<TTabSheet>("TfContactForce", "tsD25");
    EL<TTabSheet>("TfContactForce", "tsDieForceDynamicKit");
    EL<TTabSheet>("TfContactForce", "tsDynamicKit");
    EL<TTabSheet>("TfContactForce", "tsEPCalibration");
    EL<TTabSheet>("TfContactForce", "tsNewMethod");
    filerw::ELSetParents("TfContactForce", kCF_ParentOf, (int)(sizeof(kCF_ParentOf) / sizeof(kCF_ParentOf[0])));
}

// golden ContactForce.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void CF_DfmState()
{
    EL<TEdit>("TfContactForce", "edtCurrentType")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtCurrentType")->Visible = false;
    EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Visible = false;
    EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Visible = false;
    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Visible = false;
    EL<TEdit>("TfContactForce", "edtVisible")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtVisible")->Visible = false;
    EL<TEdit>("TfContactForce", "edtVisibleInd")->Enabled = false;
    EL<TEdit>("TfContactForce", "edtVisibleInd")->Visible = false;
    EL<TEdit>("TfContactForce", "edMaxKpa")->Text = "499";
    EL<TEdit>("TfContactForce", "edMaxKpaDual")->Text = "499";
    EL<TEdit>("TfContactForce", "edMaxKpa_1032")->Text = "499";
    EL<TEdit>("TfContactForce", "edMaxMpaFB")->Text = "5.013";
    EL<TEdit>("TfContactForce", "edMaxMpaFBDual")->Text = "5.013";
    EL<TEdit>("TfContactForce", "edMaxMpaFB_1032")->Text = "5.013";
    EL<TEdit>("TfContactForce", "edMinMpa")->Text = "0.001";
    EL<TEdit>("TfContactForce", "edMinMpaDual")->Text = "0.001";
    EL<TEdit>("TfContactForce", "edMinMpa_1032")->Text = "0.001";
    EL<TEdit>("TfContactForce", "edtArm1Offset_01")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_02")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_03")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_04")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_05")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_06")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_07")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_08")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_09")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_10")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_11")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_12")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_13")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_14")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm1Offset_15")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_01")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_02")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_03")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_04")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_05")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_06")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_07")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_08")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_09")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_10")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_11")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_12")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_13")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_14")->Text = "0";
    EL<TEdit>("TfContactForce", "edtArm2Offset_15")->Text = "0";
    EL<TEdit>("TfContactForce", "edtCurrentType")->Text = "30,40,60,56";
    EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Text = "30,40,60,56";
    EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text = "30,40,60,56";
    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text = "1,1,1,0";
    EL<TEdit>("TfContactForce", "edtMinMpaFB")->Text = "0.968";
    EL<TEdit>("TfContactForce", "edtMinMpaFBDual")->Text = "0.968";
    EL<TEdit>("TfContactForce", "edtMinMpaFB_1032")->Text = "0.968";
    EL<TEdit>("TfContactForce", "edtVisible")->Text = "1,1,1,0";
    EL<TEdit>("TfContactForce", "edtVisibleInd")->Text = "1,1,1,0";
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_10kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_10kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_60kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_60kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg")->DfmInit(0, 4095, 0);
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg")->OnChange = &CF_tb30mm_10kgChange;
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index30mm")->DfmInit(80, 150, 80);
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index40mm")->DfmInit(80, 150, 80);
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index60mm")->DfmInit(80, 150, 80);
    EL<filerw::ELTrackBar>("TfContactForce", "tbD60_Index56mm")->DfmInit(80, 150, 80);
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kCF_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "GroupBox4", "GroupBox5", "GroupBox6",
    "GroupBox7", "PageControl1", "TabSheet1", "gbEPReadSetting", "gbEPReadSetting_1032", "gbEPSetting",
    "gbEPSetting_1032", "grpATCTempOffset", "grpAddNewKit", "grpAddNewKitInd", "grpDieForceAddNewKit", "grpEPForArm2",
    "grpEPForDieForce", "lab30mm10kgNum", "lab30mm60kgNum", "lab40mm10kgNum", "lab40mm60kgNum", "lab60mm10kgNum",
    "lab60mm60kgNum", "scrlbxDieForceDynamicKit", "scrlbxDynamicKit", "scrlbxDynamicKitInd", "tsContactHighOffset", "tsD25",
    "tsDieForceDynamicKit", "tsDieForceOneByOneKit", "tsDynamicKit", "tsEPCalibration", "tsNewMethod",
};

// 存檔流程（btSaveClick、WriteFile）讀的替身 24 個 —— FileRW/ContactForce.cpp 用它決定頁面必須送哪些值
static const char* const kCF_SaveReads[] = {
    "edD25_30mm", "edD25_40mm", "edD25_60mm", "edD60_56mm", "edMaxKpa", "edMaxKpa_1032",
    "edMaxMpaFB", "edMaxMpaFB_1032", "edMinMpa", "edMinMpa_1032", "edtMinMpaFB", "edtMinMpaFB_1032",
    "tb30mm_10kg", "tb30mm_60kg", "tb40mm_10kg", "tb40mm_60kg", "tb56mm_10kg", "tb56mm_60kg",
    "tb60mm_10kg", "tb60mm_60kg", "tbD25_Index30mm", "tbD25_Index40mm", "tbD25_Index60mm", "tbD60_Index56mm",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void CF_CreateSaveProxies()
{
    EL<TEdit>("TfContactForce", "edD25_30mm");
    EL<TEdit>("TfContactForce", "edD25_40mm");
    EL<TEdit>("TfContactForce", "edD25_60mm");
    EL<TEdit>("TfContactForce", "edD60_56mm");
    EL<TEdit>("TfContactForce", "edMaxKpa");
    EL<TEdit>("TfContactForce", "edMaxKpa_1032");
    EL<TEdit>("TfContactForce", "edMaxMpaFB");
    EL<TEdit>("TfContactForce", "edMaxMpaFB_1032");
    EL<TEdit>("TfContactForce", "edMinMpa");
    EL<TEdit>("TfContactForce", "edMinMpa_1032");
    EL<TEdit>("TfContactForce", "edtMinMpaFB");
    EL<TEdit>("TfContactForce", "edtMinMpaFB_1032");
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_10kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_60kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg");
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index30mm");
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index40mm");
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index60mm");
    EL<filerw::ELTrackBar>("TfContactForce", "tbD60_Index56mm");
}

// golden ContactForce.cpp:421  TfContactForce::TfContactForce(TComponent* Owner)
static void CF_TfContactForce()
{

    fShow=false;
    bHasFile=false;
    bNeedWriteFile=false;                                                       //Ifor 20200908 add:需要重新寫入檔案
    AnsiString Str;
    FileName="D:\\HT9045\\system\\ContactInfo.ini";

    if(CosFunction.bUseDynamicKitDiameter)                                      //Steven 20170605 : 可以自定義Kit直徑
    {
        if(FileExists(FileName))
        {
            bHasFile=true;
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            EL<TEdit>("TfContactForce", "edtCurrentType")->Text=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Type"),    AnsiString("28,40,58,56"));
            EL<TEdit>("TfContactForce", "edtVisible")->Text    =CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Visible"), AnsiString("1,1,1,1"));
            if(EL<TEdit>("TfContactForce", "edtCurrentType")->Text.Pos("30")==1 || EL<TEdit>("TfContactForce", "edtCurrentType")->Text.Pos("60")==1)  //Ifor 20200904 add:KYEC 缸徑自動轉換 30=>28  60=>58
            {
                bNeedWriteFile=true;                                            //Ifor 20200908 add:需要重新寫入檔案
            }
        }
        else
        {
            EL<TEdit>("TfContactForce", "edtCurrentType")->Text=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Type"),    AnsiString("30,40,60,56"));
            EL<TEdit>("TfContactForce", "edtVisible")->Text    =CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Visible"), AnsiString("1,1,1,0"));
        }

        //AI(mg899to912) 20260909: 使用者 20260909 裁決解掉寫死的 [SLK Type Ind] "20,30"，讓 Multi EP 逐站微調口徑跟隨一般 SLK 型號清單（源自 V899 20260526；MG-P2 當時裁決未動）
        EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Text=EL<TEdit>("TfContactForce", "edtCurrentType")->Text;
        EL<TEdit>("TfContactForce", "edtVisibleInd")->Text    =EL<TEdit>("TfContactForce", "edtVisible")->Text;

    slSLKType=new TStringList();   // golden ContactForce.cpp:456-456 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:456-456 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKType")=new TStringList();
#endif // GATE (S12-C save)
    slSLKType->Clear();   // golden ContactForce.cpp:457-457 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:457-457 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKType")->Clear();
#endif // GATE (S12-C save)
    slSLKType->CommaText=EL<TEdit>("TfContactForce", "edtCurrentType")->Text;   // golden ContactForce.cpp:458-458 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:458-458 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKType")->CommaText=EL<TEdit>("TfContactForce", "edtCurrentType")->Text;
#endif // GATE (S12-C save)

    slSLKTypeVisible=new TStringList();   // golden ContactForce.cpp:460-460 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:460-460 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeVisible")=new TStringList();
#endif // GATE (S12-C save)
    slSLKTypeVisible->Clear();   // golden ContactForce.cpp:461-461 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:461-461 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeVisible")->Clear();
#endif // GATE (S12-C save)
    slSLKTypeVisible->CommaText=EL<TEdit>("TfContactForce", "edtVisible")->Text;   // golden ContactForce.cpp:462-462 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:462-462 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeVisible")->CommaText=EL<TEdit>("TfContactForce", "edtVisible")->Text;
#endif // GATE (S12-C save)

    for(int i=0; i<slSLKType->Count; i++)   // golden ContactForce.cpp:464-464 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:464-464 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        for(int i=0; i<EL<TControl>("TfContactForce", "slSLKType")->Count; i++)
#endif // GATE (S12-C save)
        {
    if(slSLKType->Strings[i]!="" && atof(slSLKType->Strings[i].c_str())>15.0)   // golden ContactForce.cpp:466-466 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:466-466 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            if(EL<TControl>("TfContactForce", "slSLKType")->Strings[i]!="" && atof(EL<TControl>("TfContactForce", "slSLKType")->Strings[i].c_str())>15.0)
#endif // GATE (S12-C save)
            {
    if(slSLKType->Strings[i]=="402")   // golden ContactForce.cpp:468-468 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:468-468 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                if(EL<TControl>("TfContactForce", "slSLKType")->Strings[i]=="402")                                //Ifor 20230830 add:40 倍立缸
#endif // GATE (S12-C save)
                {
    SLKClass.push_back(new THTSLKClass("40x2", i, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:470-470 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:470-470 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    SLKClass.push_back(new THTSLKClass("40x2", i, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                }
                else if(EP_Install==5)
                {
    Str.sprintf("Arm1_%s",slSLKType->Strings[i]);   // golden ContactForce.cpp:474-474 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:474-474 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    Str.sprintf("Arm1_%s",EL<TControl>("TfContactForce", "slSLKType")->Strings[i]);
#endif // GATE (S12-C save)
    SLKClass.push_back(new THTSLKClass(Str, i, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:475-475 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:475-475 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    SLKClass.push_back(new THTSLKClass(Str, i, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
    Str.sprintf("Arm2_%s",slSLKType->Strings[i]);   // golden ContactForce.cpp:476-476 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:476-476 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    Str.sprintf("Arm2_%s",EL<TControl>("TfContactForce", "slSLKType")->Strings[i]);
#endif // GATE (S12-C save)
    SLKClass.push_back(new THTSLKClass(Str, i, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:477-477 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:477-477 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    SLKClass.push_back(new THTSLKClass(Str, i, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                }
                else
                {
    SLKClass.push_back(new THTSLKClass(slSLKType->Strings[i], i, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:481-481 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:481-481 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    SLKClass.push_back(new THTSLKClass(EL<TControl>("TfContactForce", "slSLKType")->Strings[i], i, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                }

                if(CUSTOMER_CODE==CC_ASE_SG)
                {
    if(slSLKType->Strings[i]=="80")   // golden ContactForce.cpp:486-486 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:486-486 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    if(EL<TControl>("TfContactForce", "slSLKType")->Strings[i]=="80")
#endif // GATE (S12-C save)
                    {
    SLKClass.push_back(new THTSLKClass("80_Hi", i+1, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:488-488 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:488-488 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                        SLKClass.push_back(new THTSLKClass("80_Hi", i+1, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                    }
                }
            }
        }

    slSLKTypeInd=new TStringList();   // golden ContactForce.cpp:494-494 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:494-494 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeInd")=new TStringList();
#endif // GATE (S12-C save)
    slSLKTypeInd->Clear();   // golden ContactForce.cpp:495-495 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:495-495 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeInd")->Clear();
#endif // GATE (S12-C save)
    slSLKTypeInd->CommaText=EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Text;   // golden ContactForce.cpp:496-496 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:496-496 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeInd")->CommaText=EL<TEdit>("TfContactForce", "edtCurrentTypeInd")->Text;
#endif // GATE (S12-C save)

    slSLKTypeIndVisible=new TStringList();   // golden ContactForce.cpp:498-498 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:498-498 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeIndVisible")=new TStringList();
#endif // GATE (S12-C save)
    slSLKTypeIndVisible->Clear();   // golden ContactForce.cpp:499-499 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:499-499 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeIndVisible")->Clear();
#endif // GATE (S12-C save)
    slSLKTypeIndVisible->CommaText=EL<TEdit>("TfContactForce", "edtVisibleInd")->Text;   // golden ContactForce.cpp:500-500 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:500-500 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeIndVisible")->CommaText=EL<TEdit>("TfContactForce", "edtVisibleInd")->Text;
#endif // GATE (S12-C save)
        //AI(ht9045-v899) 20260504: Multi EP (INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI) SLKIndClass uses i*8+j (Arm1_1..4, Arm2_1..4). Single-force core port; iIndEPCnt==8 branch below kept for modes 0/1/2.
        if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
        {
    for(int i=0; i<slSLKTypeInd->Count; i++)   // golden ContactForce.cpp:504-504 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:504-504 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
            {
                for(int j=0; j<8; j++)
                {
    if(slSLKTypeInd->Strings[i]!="" && atof(slSLKTypeInd->Strings[i].c_str())>15.0)   // golden ContactForce.cpp:508-508 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:508-508 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    if(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i]!="" && atof(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i].c_str())>15.0)
#endif // GATE (S12-C save)
                    {
    SLKIndClass.push_back(new THTSLKIndClass(slSLKTypeInd->Strings[i], i*8+j, (bool)atoi(slSLKTypeIndVisible->Strings[i].c_str())));   // golden ContactForce.cpp:510-510 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:510-510 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                        SLKIndClass.push_back(new THTSLKIndClass(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i], i*8+j, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeIndVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                    }
                }
            }
        }
        else
        {
    for(int i=0; i<slSLKTypeInd->Count; i++)   // golden ContactForce.cpp:517-517 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:517-517 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
            {
                for(int j=0; j<16; j++)
                {
    if(slSLKTypeInd->Strings[i]!="" && atof(slSLKTypeInd->Strings[i].c_str())>15.0)   // golden ContactForce.cpp:521-521 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:521-521 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    if(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i]!="" && atof(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i].c_str())>15.0)
#endif // GATE (S12-C save)
                    {
                        if((iIndEPCnt==4 ||                                         //JerryYang 20210413 : 4組獨立EP版本使用0,1,4,5
                            (iIndEPCnt==8 && TestIF_File.iTestMode==DualSite))&&    //RogerYang 20260603 : Add 8EP
                            (j==2 || j==3 || j>=6))
                        {
    SLKIndClass.push_back(new THTSLKIndClass(slSLKTypeInd->Strings[i], i*16+j, false));   // golden ContactForce.cpp:527-527 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:527-527 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                            SLKIndClass.push_back(new THTSLKIndClass(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i], i*16+j, false));
#endif // GATE (S12-C save)
                        }
                        else if(iIndEPCnt==8 && (j>=8))                             //RogerYang 20260603 : Add 8EP
                        {
    SLKIndClass.push_back(new THTSLKIndClass(slSLKTypeInd->Strings[i], i*16+j, false));   // golden ContactForce.cpp:531-531 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:531-531 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                            SLKIndClass.push_back(new THTSLKIndClass(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i], i*16+j, false));
#endif // GATE (S12-C save)
                        }
                        else
                        {
    SLKIndClass.push_back(new THTSLKIndClass(slSLKTypeInd->Strings[i], i*16+j, (bool)atoi(slSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:535-535 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:535-535 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                            SLKIndClass.push_back(new THTSLKIndClass(EL<TControl>("TfContactForce", "slSLKTypeInd")->Strings[i], i*16+j, (bool)atoi(EL<TControl>("TfContactForce", "slSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                        }
                    }
                }
            }
        }

        EP_MAXKPA  =CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0); //kevin 20170803 (wei) 避免一開始值被修改
        EP_MAXAFB  =CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013); //kevin 20170803 (wei) 避免一開始值被修改
        EP_MINMPA  =CheckAndReadIniDataGeneral("System" , "EP_MINMPA" , 0.001); //JerryYang 20171023 (wei) add PA Min  //kevin 20170803 (wei) 避免一開始值被修改
//        for(int i=0; i<3; i++)
//        {
//            for(int j=0; j<11; j++) //Ifor 20230206 add: 新增四組Contact 補償 7 => 11
//            {
//                if(i==2)
//                {
//                    if(j>=7)
//                    {
//                        dIndexZOffset[i][j]=120+(6*60)+((j-6)*40);
//                    }
//                    else
//                    {
//                        dIndexZOffset[i][j]=120+(j*60);
//                    }
//                }
//                else
//                {
//                    Str.sprintf("dIndexZOffset[%d][%d]", i, j);    //JerryYang 20190910 fix ATC offset存檔問題
//                    dIndexZOffset[i][j]             =CheckRange(CheckAndReadIniDataGeneral("Test Arm", Str, 0.0), 10.0, 0.0);
//                    IndexZOffsetEdit[i][j]->Text    =dIndexZOffset[i][j];
//                }
//            }
//        }
        EL<TEdit>("TfContactForce", "edMaxKpa")->Text      = EP_MAXKPA;                                        //kevin 20170803 (wei) 避免一開始值被修改
        EL<TEdit>("TfContactForce", "edMaxMpaFB")->Text    = EP_MAXAFB;                                        //kevin 20170803 (wei) 避免一開始值被修改
        EL<TEdit>("TfContactForce", "edMinMpa")->Text      = EP_MINMPA;                                        //JerryYang 20171023 (wei) add PA Min  //kevin 20170803 (wei) 避免一開始值被修改

        EL<TEdit>("TfContactForce", "edMaxKpa_1032")->Text      = EP_MAXKPA_1032;                              //kevin 20170803 (wei) 避免一開始值被修改
        EL<TEdit>("TfContactForce", "edMaxMpaFB_1032")->Text    = EP_MAXAFB_1032;                              //kevin 20170803 (wei) 避免一開始值被修改
        EL<TEdit>("TfContactForce", "edMinMpa_1032")->Text      = EP_MINMPA_1032;                              //JerryYang 20171023 (wei) add PA Min  //kevin 20170803 (wei) 避免一開始值被修改

        if(INSTALL_DOUBLE_EP>0)                                                 //Ifor 20191003 : add Die Force 可以自定義Kit直徑
        {
            if(FileExists(FileName))
            {
                EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("DieForceType"),    AnsiString("20,30,40,50"));
                if(IniConfig.bSPILFunction==true)                               //JerryYang 20200805 : 國龍要求dual force缸徑由軟體決定,不要從文字檔改
                {
                    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text    =CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("DieForceVisible"), AnsiString("1,1,1,1"));
                }
                else
                {
                    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text    =CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("DieForceVisible"), AnsiString("1,1,1,0"));
                }
            }
            else
            {
                EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text="20,30,40,50";
                EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text    ="1,1,1,0";
            }

            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20200805 : 國龍要求dual force缸徑由軟體決定,不要從文字檔改
            {
                if(EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text=="20,30,40,50")
                {
                    EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text="1,1,1,1";
                }
            }

    slDieForceSLKType=new TStringList();   // golden ContactForce.cpp:604-604 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:604-604 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKType")=new TStringList();
#endif // GATE (S12-C save)
    slDieForceSLKType->Clear();   // golden ContactForce.cpp:605-605 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:605-605 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKType")->Clear();
#endif // GATE (S12-C save)
    slDieForceSLKType->CommaText=EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text;   // golden ContactForce.cpp:606-606 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:606-606 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKType")->CommaText=EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text;
#endif // GATE (S12-C save)

    slDieForceSLKTypeVisible=new TStringList();   // golden ContactForce.cpp:608-608 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:608-608 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKTypeVisible")=new TStringList();
#endif // GATE (S12-C save)
    slDieForceSLKTypeVisible->Clear();   // golden ContactForce.cpp:609-609 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:609-609 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKTypeVisible")->Clear();
#endif // GATE (S12-C save)
    slDieForceSLKTypeVisible->CommaText=EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text;   // golden ContactForce.cpp:610-610 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:610-610 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceSLKTypeVisible")->CommaText=EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text;
#endif // GATE (S12-C save)

    for(int i=0; i<slDieForceSLKType->Count; i++)   // golden ContactForce.cpp:612-612 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:612-612 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            for(int i=0; i<EL<TControl>("TfContactForce", "slDieForceSLKType")->Count; i++)
#endif // GATE (S12-C save)
            {
    if(slDieForceSLKType->Strings[i]!="" && atof(slDieForceSLKType->Strings[i].c_str())>15.0)   // golden ContactForce.cpp:614-614 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:614-614 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                if(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i]!="" && atof(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i].c_str())>15.0)
#endif // GATE (S12-C save)
    DieForceSLKClass.push_back(new THTDieForceSLKClass(slDieForceSLKType->Strings[i], i, (bool)atoi(slDieForceSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:615-615 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:615-615 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                    DieForceSLKClass.push_back(new THTDieForceSLKClass(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i], i, (bool)atoi(EL<TControl>("TfContactForce", "slDieForceSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
            }
            //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
    slDieForceOneByOneSLKType=new TStringList();   // golden ContactForce.cpp:618-618 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:618-618 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKType")=new TStringList();
#endif // GATE (S12-C save)
    slDieForceOneByOneSLKType->Clear();   // golden ContactForce.cpp:619-619 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:619-619 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKType")->Clear();
#endif // GATE (S12-C save)
    slDieForceOneByOneSLKType->CommaText=EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text;   // golden ContactForce.cpp:620-620 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:620-620 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKType")->CommaText=EL<TEdit>("TfContactForce", "edtDieForceCurrentType")->Text;
#endif // GATE (S12-C save)

    slDieForceOneByOneSLKTypeVisible=new TStringList();   // golden ContactForce.cpp:622-622 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:622-622 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKTypeVisible")=new TStringList();
#endif // GATE (S12-C save)
    slDieForceOneByOneSLKTypeVisible->Clear();   // golden ContactForce.cpp:623-623 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:623-623 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKTypeVisible")->Clear();
#endif // GATE (S12-C save)
    slDieForceOneByOneSLKTypeVisible->CommaText=EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text;   // golden ContactForce.cpp:624-624 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:624-624 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
            EL<TControl>("TfContactForce", "slDieForceOneByOneSLKTypeVisible")->CommaText=EL<TEdit>("TfContactForce", "edtDieForceVisible")->Text;
#endif // GATE (S12-C save)
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
            {
    for(int i=0; i<slDieForceOneByOneSLKType->Count; i++)   // golden ContactForce.cpp:627-627 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:627-627 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                for(int i=0; i<EL<TControl>("TfContactForce", "slDieForceOneByOneSLKType")->Count; i++)
#endif // GATE (S12-C save)
                {
                    for(int j=0; j<8; j++)
                    {
    if(slDieForceSLKType->Strings[i]!="" && atof(slDieForceSLKType->Strings[i].c_str())>15.0)   // golden ContactForce.cpp:631-631 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:631-631 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                        if(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i]!="" && atof(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i].c_str())>15.0)
#endif // GATE (S12-C save)
                        {
    DieForceOneByOneSLKClass.push_back(new THTDieForceOneByOneSLKClass(slDieForceSLKType->Strings[i], i*8+j, (bool)atoi(slDieForceOneByOneSLKTypeVisible->Strings[i].c_str())));   // golden ContactForce.cpp:633-633 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:633-633 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
                            DieForceOneByOneSLKClass.push_back(new THTDieForceOneByOneSLKClass(EL<TControl>("TfContactForce", "slDieForceSLKType")->Strings[i], i*8+j, (bool)atoi(EL<TControl>("TfContactForce", "slDieForceOneByOneSLKTypeVisible")->Strings[i].c_str())));
#endif // GATE (S12-C save)
                        }
                    }
                }
            }
            //<== //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end
        }

        if(bHasFile)
            CF_ReadFile();
        else
            CF_WriteFile();

    slSLKType->Clear();   // golden ContactForce.cpp:646-646 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:646-646 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKType")->Clear();
#endif // GATE (S12-C save)
    delete slSLKType;   // golden ContactForce.cpp:647-647 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:647-647 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        delete EL<TControl>("TfContactForce", "slSLKType");
#endif // GATE (S12-C save)
    slSLKType=NULL;   // golden ContactForce.cpp:648-648 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:648-648 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKType")=NULL;
#endif // GATE (S12-C save)
    slSLKTypeVisible->Clear();   // golden ContactForce.cpp:649-649 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:649-649 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeVisible")->Clear();
#endif // GATE (S12-C save)
    delete slSLKTypeVisible;   // golden ContactForce.cpp:650-650 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:650-650 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        delete EL<TControl>("TfContactForce", "slSLKTypeVisible");
#endif // GATE (S12-C save)
    slSLKTypeVisible=NULL;   // golden ContactForce.cpp:651-651 取代：TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:651-651 -- TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）
        EL<TControl>("TfContactForce", "slSLKTypeVisible")=NULL;
#endif // GATE (S12-C save)
    }

}

// golden ContactForce.cpp:686  TfContactForce::FormShow(TObject *Sender)
static void CF_FormShow()
{

    fShow=true;
    CF_ReadFile();
    ;   // golden ContactForce.cpp:690-691 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:690-691 -- Left／Top：視窗位置（HTML 不用）
    Left=10;                                                                    //Steven 20220407 : 避免跑到畫面外
    Top=10;
#endif // GATE (S12-C save)

    EL<TTabSheet>("TfContactForce", "tsNewMethod")->TabVisible         =(WEIGHT_CALIBRATION);                      //Steven 20150224 : Contact畫面可以調整D25設定值
    if(CosFunction.bUseDynamicKitDiameter)                                      //Steven 20170605 : 可以自定義Kit直徑  //Jimmychiu 20230630 : Individual EP No effect contact force offset of 40mm compliance
    {
        //AI(ht9045-v899) 20260526: show Multi EP site trim page (TabSheet1) for outer EP per-site calibration.
        if((INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI) && TestIF_File.bIndEPSLK==true)
        {
            EL<TTabSheet>("TfContactForce", "tsDynamicKit")->TabVisible=false;
            EL<TTabSheet>("TfContactForce", "TabSheet1")->TabVisible=true;
            EL<TTabSheet>("TfContactForce", "tsDieForceDynamicKit")->TabVisible=false;
            EL<TTabSheet>("TfContactForce", "tsDieForceOneByOneKit")->TabVisible=true;
        }
        else
        {
            EL<TTabSheet>("TfContactForce", "tsDynamicKit")->TabVisible=true;
            EL<TTabSheet>("TfContactForce", "TabSheet1")->TabVisible=false;
            EL<TTabSheet>("TfContactForce", "tsDieForceOneByOneKit")->TabVisible=false;
        }
    }

    EL<TTabSheet>("TfContactForce", "tsD25")       ->TabVisible        =(!CosFunction.bUseDynamicKitDiameter && CosFunction.bHiSiliconFunction==false);     //Steven 20170605 : 可以自定義Kit直徑
    EL<TTabSheet>("TfContactForce", "tsEPCalibration")->TabVisible     =true;                                      //JerryYang 20171102 (wei) 新增頁面方便校正PA對應電壓
    EL<TGroupBox>("TfContactForce", "grpEPForDieForce")->Visible       =(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI); //AI(ht9045-v899) 20260430: also show for Multi EP half (inner cyl Die Force).
    //AI(ht9045-v899) 20260430: 16 EP independent + Multi EP half use 2 boards (Arm1 + Arm2); show Arm2 EP calibration UI.
    EL<TGroupBox>("TfContactForce", "grpEPForArm2")->Visible           =(EP_Install==5 || iIndEPCnt==16 || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI);

    EL<TTabSheet>("TfContactForce", "tsNewMethod")      ->Enabled=(AccessLevel>=iDefHonPrecLevel || CUSTOMER_CODE==CC_HONPREC_QC)?true:false;
    EL<TTabSheet>("TfContactForce", "tsDynamicKit")     ->Enabled=(AccessLevel>=iDefHonPrecLevel || CUSTOMER_CODE==CC_HONPREC_QC)?true:false;
    EL<TTabSheet>("TfContactForce", "tsD25")            ->Enabled=(AccessLevel>=iDefHonPrecLevel || CUSTOMER_CODE==CC_HONPREC_QC)?true:false;
    EL<TTabSheet>("TfContactForce", "tsEPCalibration")  ->Enabled=(AccessLevel>=iDefEngineerLevel || CUSTOMER_CODE==CC_HONPREC_QC)?true:false; //JerryYang 20171102 (wei) Engineer以上權限才能修改

    bool bNSKit=false,bNSKitSwitch=false;
    if(CosFunction.bEPUseNSSLK==true)                                           //kevin 20170804 (Steven) EP表頭另一種TYPE
    {
        bNSKit=true;                                                            //使用NS KIT
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(TestIF_File.bNSKitPress)
                bNSKitSwitch=true;                                              //使用NS KIT
        }
        else
        {
            bNSKitSwitch=true;
        }
    }

    bool bUseNSKit=false;
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        switch(TestIF.iTestMode)
        {
            case SingleSite:
            case DualSite:
                if(CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true)
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
                break;
            case QualSite1X4:
                if((CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
                break;
            case QualSite2X2:
                if((CosFunction.bCanUse2x2Bias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
            case DualSite2x1:
                if(TestIF_File.bNS7000CS==true)
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
                break;
            case TriSite1X3:
            case _8Site1X4:
                if(ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true)
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
                break;
            case _8Site2X4:
                if((ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true) ||
                   (USE_16_HEATER!=eht4Heater && TestIF_File.bNS8000CS==true))
                {
                    bUseNSKit=true;
                }
                else
                {
                    bUseNSKit=false;
                }
                break;
            default :
                bUseNSKit=false;
                break;
        }
//        if(TestIF_File.bNS7000kit || TestIF_File.bNS7000CS || TestIF_File.bNS8000CS)
//        {
//            bUseNSKit=true;
//        }
//        else
//        {
//            bUseNSKit=false;
//        }
    }
    else
    {
        bUseNSKit=false;
    }

    for(unsigned int i=0; i<SLKClass.size(); i++)
    {
        if((bNSKit && bNSKitSwitch) || bUseNSKit==true)                         //kevin 20170808 (Steven) add NS offset
        {
            SLKClass[i]->trckbrDiameter_NS->Visible =true;
            SLKClass[i]->lblDiameter_NS->Visible    =true;
            SLKClass[i]->edtLoadRate_NS->Visible    =true;

            SLKClass[i]->trckbrDiameter->Visible =false;
            SLKClass[i]->lblDiameter->Visible    =false;
            SLKClass[i]->edtLoadRate->Visible    =false;

            SLKClass[i]->edtHotOffset->Visible      =false;
            SLKClass[i]->lblHotOffset->Visible      =false;

            SLKClass[i]->lblContactOffset_NS->Visible =true;
            SLKClass[i]->edtContactOffset_NS->Visible =true;

            SLKClass[i]->edtContactOffset->Visible    =false;
            SLKClass[i]->lblContactOffset->Visible    =false;
        }
        else
        {
            SLKClass[i]->trckbrDiameter->Visible =true;
            SLKClass[i]->lblDiameter->Visible    =true;
            SLKClass[i]->edtLoadRate->Visible    =true;

            SLKClass[i]->trckbrDiameter_NS->Visible =false;
            SLKClass[i]->lblDiameter_NS->Visible    =false;
            SLKClass[i]->edtLoadRate_NS->Visible    =false;

            SLKClass[i]->lblContactOffset_NS->Visible =false;
            SLKClass[i]->edtContactOffset_NS->Visible =false;

            SLKClass[i]->edtContactOffset->Visible    =true;
            SLKClass[i]->lblContactOffset->Visible    =true;
        }

        SLKClass[i]->trckbrDiameter->Min        =80;
        SLKClass[i]->trckbrDiameter->Max        =150;
        SLKClass[i]->trckbrDiameter_NS->Min     =80;
        SLKClass[i]->trckbrDiameter_NS->Max     =150;
        SLKClass[i]->edtHotOffset->Visible      =(CosFunction.bUseLoadCellOffsetByHeater);
        SLKClass[i]->lblHotOffset->Visible      =(CosFunction.bUseLoadCellOffsetByHeater);

        //SLKClass[i]->trckbrDiameter_NS->Visible =(CosFunction.bEPUseNSSLK);
        //SLKClass[i]->lblDiameter_NS->Visible    =(CosFunction.bEPUseNSSLK);
        //SLKClass[i]->edtLoadRate_NS->Visible    =(CosFunction.bEPUseNSSLK);

//        if(i>3) //只有Hontech權限可以增減SLK
//            SLKClass[i]->btnDeleteKit->Visible=(AccessLevel>=iDefHonPrecLevel);
    }
//    grpAddNewKit->Visible=(AccessLevel>=iDefHonPrecLevel);
//    gbEPSetting->Visible=(AccessLevel>=iDefHonPrecLevel);
    if(INSTALL_DOUBLE_EP==1)                                                    //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    {
        EL<TTabSheet>("TfContactForce", "tsDieForceDynamicKit")->TabVisible=true;
        for(unsigned int i=0; i<DieForceSLKClass.size(); i++)
        {
            DieForceSLKClass[i]->trckbrDieForceDiameter->Min        =80;
            DieForceSLKClass[i]->trckbrDieForceDiameter->Max        =150;
        }
    }
    else
    {
        EL<TTabSheet>("TfContactForce", "tsDieForceDynamicKit")->TabVisible=false;
    }

    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg")->Position=IniConfig.iContactForceMap[0][0];
    EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg")->Position=IniConfig.iContactForceMap[0][1];
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg")->Position=IniConfig.iContactForceMap[1][0];
    EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg")->Position=IniConfig.iContactForceMap[1][1];
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg")->Position=IniConfig.iContactForceMap[2][0];
    EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg")->Position=IniConfig.iContactForceMap[2][1];
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_10kg")->Position=IniConfig.iContactForceMap[3][0];                     //wei 20151005 add 56mm
    EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_60kg")->Position=IniConfig.iContactForceMap[3][1];                     //wei 20151005 add 56mm

    //Steven 20150224 : Contact畫面可以調整D25設定值
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index60mm")->Position   =LastSet.dIndexLoadRate[0][0]*100;
    EL<filerw::ELTrackBar>("TfContactForce", "tbD60_Index56mm")->Position   =LastSet.dIndexLoadRate[0][1]*100;              //wei 20151005 add 56mm
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index40mm")->Position   =LastSet.dIndexLoadRate[0][2]*100;
    EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index30mm")->Position   =LastSet.dIndexLoadRate[0][3]*100;

    EL<TEdit>("TfContactForce", "edD25_60mm")->Text            =LastSet.dIndexLoadRate[2][0];                  //2014-06-26    Dell    for TSMC 高溫Load cell offse
    EL<TEdit>("TfContactForce", "edD60_56mm")->Text            =LastSet.dIndexLoadRate[2][1];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset    //wei 20151005 add 56mm
    EL<TEdit>("TfContactForce", "edD25_40mm")->Text            =LastSet.dIndexLoadRate[2][2];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset
    EL<TEdit>("TfContactForce", "edD25_30mm")->Text            =LastSet.dIndexLoadRate[2][3];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset

    CF_ShowValue();

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);

}

// golden ContactForce.cpp:936  TfContactForce::btSaveClick(TObject *Sender)
static void CF_btSaveClick()
{
    filerw::ELMark("btSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    IniConfig.iContactForceMap[0][0]=EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg")->Position;
    IniConfig.iContactForceMap[0][1]=EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg")->Position;
    IniConfig.iContactForceMap[1][0]=EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg")->Position;
    IniConfig.iContactForceMap[1][1]=EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg")->Position;
    IniConfig.iContactForceMap[2][0]=EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg")->Position;
    IniConfig.iContactForceMap[2][1]=EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg")->Position;
    IniConfig.iContactForceMap[3][0]=EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_10kg")->Position;                     //wei 20151005 add 56mm
    IniConfig.iContactForceMap[3][1]=EL<filerw::ELTrackBar>("TfContactForce", "tb56mm_60kg")->Position;                     //wei 20151005 add 56mm

    //Steven 20150224 : Contact畫面可以調整D25設定值
    LastSet.dIndexLoadRate[0][0]    =EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index60mm")->Position/100.0;           //jou 2011-06-10
    LastSet.dIndexLoadRate[0][1]    =EL<filerw::ELTrackBar>("TfContactForce", "tbD60_Index56mm")->Position/100.0;           //jou 2011-06-10     //wei 20151005 add 56mm
    LastSet.dIndexLoadRate[0][2]    =EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index40mm")->Position/100.0;           //Steven 20110704
    LastSet.dIndexLoadRate[0][3]    =EL<filerw::ELTrackBar>("TfContactForce", "tbD25_Index30mm")->Position/100.0;           //jou 2011-06-10

    LastSet.dIndexLoadRate[2][0]    =CheckRange(atof(EL<TEdit>("TfContactForce", "edD25_60mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset
    LastSet.dIndexLoadRate[2][1]    =CheckRange(atof(EL<TEdit>("TfContactForce", "edD60_56mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset          //wei 20151005 add 56mm
    LastSet.dIndexLoadRate[2][2]    =CheckRange(atof(EL<TEdit>("TfContactForce", "edD25_40mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset
    LastSet.dIndexLoadRate[2][3]    =CheckRange(atof(EL<TEdit>("TfContactForce", "edD25_30mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset

    filerw::ELMark("CF_SaveLastSetIni"); SaveLastSetIni();   // golden ContactForce.cpp:958-958 取代：存檔標記：golden 第一個寫檔動作（SaveLastSetIni 寫 Gerneral.ini [Version]／config.ini／LastSet.ini／configByRecipe.ini），btSaveClick 沒有任何 return（savedMark="CF_SaveLastSetIni"）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:958-958 -- 存檔標記：golden 第一個寫檔動作（SaveLastSetIni 寫 Gerneral.ini [Version]／config.ini／LastSet.ini／configByRecipe.ini），btSaveClick 沒有任何 return（savedMark="CF_SaveLastSetIni"）
    SaveLastSetIni();
#endif // GATE (S12-C save)

    CF_WriteFile();
    //AI(ht9045-v899) 20260526: reload saved one-by-one load rates so the next TfContact SAVE uses current trim values.
    CF_ReadFile();
    filerw::ELTodo("golden ContactForce.cpp:963-963 ADAM_WriteVoltage(DeviceForm.dPress): EP voltage output after save (machine action -> Jimmy; ADAM_* are empty in the port)");
#if 0 // GATE (S12-C save) golden ContactForce.cpp:963-963 -- ADAM_WriteVoltage(DeviceForm.dPress): EP voltage output after save (machine action -> Jimmy; ADAM_* are empty in the port)
    ADAM_WriteVoltage(DeviceForm.dPress);
#endif // GATE (S12-C save)

}

// golden ContactForce.cpp:966  TfContactForce::ReadFile()
static void CF_ReadFile()
{

    AnsiString sGroup;
    double dTemp;
    bool bHasDiameter=false;
    AnsiString Str;
    IndexZOffsetEdit[0][0]=EL<TEdit>("TfContactForce", "edtArm1Offset_01");    IndexZOffsetEdit[0][1]=EL<TEdit>("TfContactForce", "edtArm1Offset_02");    IndexZOffsetEdit[0][2]=EL<TEdit>("TfContactForce", "edtArm1Offset_03");    IndexZOffsetEdit[0][3]=EL<TEdit>("TfContactForce", "edtArm1Offset_04");
    IndexZOffsetEdit[0][4]=EL<TEdit>("TfContactForce", "edtArm1Offset_05");    IndexZOffsetEdit[0][5]=EL<TEdit>("TfContactForce", "edtArm1Offset_06");    IndexZOffsetEdit[0][6]=EL<TEdit>("TfContactForce", "edtArm1Offset_07");    IndexZOffsetEdit[0][7]=EL<TEdit>("TfContactForce", "edtArm1Offset_08");
    IndexZOffsetEdit[0][8]=EL<TEdit>("TfContactForce", "edtArm1Offset_09");    IndexZOffsetEdit[0][9]=EL<TEdit>("TfContactForce", "edtArm1Offset_10");    IndexZOffsetEdit[0][10]=EL<TEdit>("TfContactForce", "edtArm1Offset_11");   IndexZOffsetEdit[0][11]=EL<TEdit>("TfContactForce", "edtArm1Offset_12");
    IndexZOffsetEdit[0][12]=EL<TEdit>("TfContactForce", "edtArm1Offset_13");   IndexZOffsetEdit[0][13]=EL<TEdit>("TfContactForce", "edtArm1Offset_14");   IndexZOffsetEdit[0][14]=EL<TEdit>("TfContactForce", "edtArm1Offset_15");

    IndexZOffsetEdit[1][0]=EL<TEdit>("TfContactForce", "edtArm2Offset_01");    IndexZOffsetEdit[1][1]=EL<TEdit>("TfContactForce", "edtArm2Offset_02");    IndexZOffsetEdit[1][2]=EL<TEdit>("TfContactForce", "edtArm2Offset_03");    IndexZOffsetEdit[1][3]=EL<TEdit>("TfContactForce", "edtArm2Offset_04");
    IndexZOffsetEdit[1][4]=EL<TEdit>("TfContactForce", "edtArm2Offset_05");    IndexZOffsetEdit[1][5]=EL<TEdit>("TfContactForce", "edtArm2Offset_06");    IndexZOffsetEdit[1][6]=EL<TEdit>("TfContactForce", "edtArm2Offset_07");    IndexZOffsetEdit[1][7]=EL<TEdit>("TfContactForce", "edtArm2Offset_08");
    IndexZOffsetEdit[1][8]=EL<TEdit>("TfContactForce", "edtArm2Offset_09");    IndexZOffsetEdit[1][9]=EL<TEdit>("TfContactForce", "edtArm2Offset_10");    IndexZOffsetEdit[1][10]=EL<TEdit>("TfContactForce", "edtArm2Offset_11");   IndexZOffsetEdit[1][11]=EL<TEdit>("TfContactForce", "edtArm2Offset_12");
    IndexZOffsetEdit[1][12]=EL<TEdit>("TfContactForce", "edtArm2Offset_13");   IndexZOffsetEdit[1][13]=EL<TEdit>("TfContactForce", "edtArm2Offset_14");   IndexZOffsetEdit[1][14]=EL<TEdit>("TfContactForce", "edtArm2Offset_15");

    //jou 20171117 (Steven) : EP校正欄位(從Contact頁面可以進入此setting)
    if(EP_Install!=0)                                                           //Steven 20140524 : 配合統一由外面讀取
    {
        EP_MAXKPA           =CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0);
        EP_MAXAFB           =CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013);
        EP_MINMPA           =CheckAndReadIniDataGeneral("System" , "EP_MINMPA"   , 0.001);  //JerryYang 20171023 add PA Min
        EP_MinAFB           =CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack", 0.908);
        EL<TEdit>("TfContactForce", "edMaxKpa")->Text      =EP_MAXKPA;
        EL<TEdit>("TfContactForce", "edMaxMpaFB")->Text    =EP_MAXAFB;
        EL<TEdit>("TfContactForce", "edtMinMpaFB")->Text   =EP_MinAFB;
        EL<TEdit>("TfContactForce", "edMinMpa")->Text      =EP_MINMPA;                                         //JerryYang 20171023 add PA Min

        EP_MAXKPA_1032           =CheckAndReadIniDataGeneral("System" , "EP_MAXKPA_1032" , 499.0);
        EP_MAXAFB_1032           =CheckAndReadIniDataGeneral("System" , "EP_MAXA_1032"   , 5.013);
        EP_MINMPA_1032           =CheckAndReadIniDataGeneral("System" , "EP_MINMPA_1032"   , 0.001);  //JerryYang 20171023 add PA Min
        EP_MinAFB_1032           =CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack_1032", 0.908);
        EL<TEdit>("TfContactForce", "edMaxKpa_1032")->Text      =EP_MAXKPA_1032;
        EL<TEdit>("TfContactForce", "edMaxMpaFB_1032")->Text    =EP_MAXAFB_1032;
        EL<TEdit>("TfContactForce", "edtMinMpaFB_1032")->Text   =EP_MinAFB_1032;
        EL<TEdit>("TfContactForce", "edMinMpa_1032")->Text      =EP_MINMPA_1032;                               //JerryYang 20171023 add PA Min

        EPDual_MAXKPA=CheckAndReadIniDataGeneral("System" , "EPDual_MAXKPA", 899.0);    //kevin 20200325 add dual force EP
        EPDual_MAXAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MAXAFB", 4.905);
        EPDual_MINMPA=CheckAndReadIniDataGeneral("System" , "EPDual_MINMPA", 0.001);
        EPDual_MinAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MinAFB", 0.968);     //kevin 20200325 add dual force EP

        EL<TEdit>("TfContactForce", "edMaxKpaDual")->Text      =EPDual_MAXKPA;
        EL<TEdit>("TfContactForce", "edMaxMpaFBDual")->Text    =EPDual_MAXAFB;
        EL<TEdit>("TfContactForce", "edtMinMpaFBDual")->Text   =EPDual_MinAFB;
        EL<TEdit>("TfContactForce", "edMinMpaDual")->Text      =EPDual_MINMPA;                                 //JerryYang 20171023 add PA Min
    }

    for(int i=0; i<3; i++)
    {
        for(int j=0; j<15; j++)                                                 //Ifor 20230206 add: 新增四組Contact 補償 7 => 11  //Ifor 20230829 add: 800KG 11 -> 15
        {
            if(i==2)
            {
                if(j>=7)
                {
                    dIndexZOffset[i][j]=120+(6*60)+((j-6)*40);
                }
                else
                {
                    dIndexZOffset[i][j]=120+(j*60);
                }
            }
            else
            {
                Str.sprintf("dIndexZOffset[%d][%d]", i, j);                     //JerryYang 20190910 fix ATC offset存檔問題
                dIndexZOffset[i][j]             =CheckRange(CheckAndReadIniDataGeneral("Test Arm", Str, 0.0), 10.0, 0.0);
                IndexZOffsetEdit[i][j]->Text    =dIndexZOffset[i][j];
            }
        }
    }

    if(CosFunction.bUseDynamicKitDiameter==false || bHasFile==false)            //Steven 20170605 : 可以自定義Kit直徑
        return;

    for(unsigned int i=0; i<SLKClass.size(); i++)
    {
        if(CUSTOMER_CODE==CC_ASE_SG && SLKClass[i]->dDiameter==80)
        {
            if(bHasDiameter==false)
            {
                bHasDiameter=true;
                sGroup.sprintf("Diameter_%0.3fmm", SLKClass[i]->dDiameter);
                EL<TTabSheet>("TfContactForce", "tsContactHighOffset")->TabVisible=SLKClass[i]->bShow;
            }
            else
            {
                sGroup.sprintf("Diameter_%0.3f_Himm", SLKClass[i]->dDiameter);
            }
        }
        else if(EP_Install==5)
        {
            sGroup.sprintf("Diameter_%smm", SLKClass[i]->sDiameter);
        }
        else
        {
            sGroup.sprintf("Diameter_%0.3fmm", SLKClass[i]->dDiameter);
        }
        SLKClass[i]->dLoadRate                  =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate",      1.0), 0.8, 1.5);
        SLKClass[i]->dLoadRate_NS               =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate_NS",   1.0), 0.8, 1.5);
        SLKClass[i]->dHotOffset                 =CheckRange(CheckAndReadIniData(FileName, sGroup, "HotOffset",     0.0), 0.5, -0.5);
        SLKClass[i]->dContactOffset             =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset", 0.0), 10.0, -10.0);
        SLKClass[i]->dContactOffset_NS          =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset_NS", 0.0), 10.0, -10.0); //kevin 20170807 (Steven) add NS offset

        dTemp=SLKClass[i]->dLoadRate*(double)100.0;                             //為了避免浮點數計算錯誤,要先強制為double
        SLKClass[i]->trckbrDiameter->Position   =int(dTemp);
        dTemp=SLKClass[i]->dLoadRate_NS*(double)100.0;
        SLKClass[i]->trckbrDiameter_NS->Position=int(dTemp);

        SLKClass[i]->edtHotOffset->Text         =SLKClass[i]->dHotOffset;
        SLKClass[i]->edtContactOffset->Text     =SLKClass[i]->dContactOffset;
        SLKClass[i]->edtContactOffset_NS->Text  =SLKClass[i]->dContactOffset_NS;//kevin 20170807 (Steven) add NS offset
    }

    if(INSTALL_DOUBLE_EP>0)                                                     //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    {
        for(unsigned int i=0; i<DieForceSLKClass.size(); i++)
        {
            sGroup.sprintf("DieForceDiameter_%0.3fmm", DieForceSLKClass[i]->dDiameter);
            DieForceSLKClass[i]->dLoadRate      =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate",      1.0), 0.8, 1.5);
            DieForceSLKClass[i]->dContactOffset =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset", 1.0), 10.0, -10.0);

            dTemp=DieForceSLKClass[i]->dLoadRate*(double)100.0;
            DieForceSLKClass[i]->trckbrDieForceDiameter->Position   =int(dTemp);
            DieForceSLKClass[i]->edtDieForceContactOffset->Text     =DieForceSLKClass[i]->dContactOffset;
            //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
            {
    for(int j=0; j<8 && (unsigned int)(i*8+j)<DieForceOneByOneSLKClass.size(); j++)   // golden ContactForce.cpp:1094-1094 取代：越界守衛：DieForceOneByOneSLKClass[i*8+j]（i 走 DieForceSLKClass，與 OneByOne 的 token 濾法相同時兩者對齊；不對齊時 golden 越界）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1094-1094 -- 越界守衛：DieForceOneByOneSLKClass[i*8+j]（i 走 DieForceSLKClass，與 OneByOne 的 token 濾法相同時兩者對齊；不對齊時 golden 越界）
                for(int j=0; j<8; j++)
#endif // GATE (S12-C save)
                {
                    if(j<4)
                        sGroup.sprintf("DieForceDiameter_%0.3fmm_Arm1_%d", DieForceOneByOneSLKClass[i*8+j]->dDiameter, j+1);
                    else
                        sGroup.sprintf("DieForceDiameter_%0.3fmm_Arm2_%d", DieForceOneByOneSLKClass[i*8+j]->dDiameter, j-3);
                    DieForceOneByOneSLKClass[i*8+j]->dLoadRate                  =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate",      1.0), 0.8, 1.5);
                    DieForceOneByOneSLKClass[i*8+j]->dContactOffset             =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset", 0.0), 10.0, -10.0);

                    dTemp=DieForceOneByOneSLKClass[i*8+j]->dLoadRate*(double)100.0;
                    DieForceOneByOneSLKClass[i*8+j]->trckbrDieForceOneByOneDiameter->Position   =int(dTemp);
                    DieForceOneByOneSLKClass[i*8+j]->edtDieForceOneByOneContactOffset->Text     =DieForceOneByOneSLKClass[i*8+j]->dContactOffset;
                }
            }
            //<== //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end
        }
    }

    if(bNeedWriteFile==true)                                                    //Ifor 20200904 add:KYEC 缸徑自動轉換 30=>28  60=>58
    {
        bNeedWriteFile=false;
        for(unsigned int i=0; i<SLKClass.size(); i++)
        {
            if(SLKClass[i]->dDiameter==30)
            {
                SLKClass[i]->dDiameter=28;
                SLKClass[i]->sDiameter="28";
            }
            else if(SLKClass[i]->dDiameter==60)
            {
                SLKClass[i]->dDiameter=58;
                SLKClass[i]->sDiameter="58";
            }
            SLKClass[i]->bShow=true;
        }
        EL<TEdit>("TfContactForce", "edtCurrentType")->Text="28,40,58,56";
        WriteIniData(FileName, "SLK Type", "Type", EL<TEdit>("TfContactForce", "edtCurrentType")->Text);
        EL<TEdit>("TfContactForce", "edtVisible")->Text="1,1,1,1";
        WriteIniData(FileName, "SLK Type", "Visible", EL<TEdit>("TfContactForce", "edtVisible")->Text);

    { TRadioGroup* rgKit=EL<TRadioGroup>("TfContact", "rgKitDiameter"); rgKit->Items->Clear(); for(unsigned int i=0; i<SLKClass.size(); i++) { if(SLKClass[i]->bShow) { rgKit->Items->Add(SLKClass[i]->sDiameter); } } }   // golden ContactForce.cpp:1134-1141 取代：CC_KYEC_LEE 缸徑 30→28／60→58 自動轉換後重建 fContact->rgKitDiameter：fContact 是 golden TfContact —— C 路 TfContact 的替身（FileRW/DeviceForm_File.cpp，同名 EL<TRadioGroup>("TfContact","rgKitDiameter")）；fContactForce->SLKClass＝本 TU 的 SLKClass。程式照 golden
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1134-1141 -- CC_KYEC_LEE 缸徑 30→28／60→58 自動轉換後重建 fContact->rgKitDiameter：fContact 是 golden TfContact —— C 路 TfContact 的替身（FileRW/DeviceForm_File.cpp，同名 EL<TRadioGroup>("TfContact","rgKitDiameter")）；fContactForce->SLKClass＝本 TU 的 SLKClass。程式照 golden
        fContact->rgKitDiameter->Items->Clear();
        for(unsigned int i=0; i<fContactForce->SLKClass.size(); i++)
        {
            if(fContactForce->SLKClass[i]->bShow)
            {
                fContact->rgKitDiameter->Items->Add(fContactForce->SLKClass[i]->sDiameter);
            }
        }
#endif // GATE (S12-C save)
        CF_WriteFile();
    }

    //AI(ht9045-v899) 20260504: full port V874.3 SLKIndClass ==MULTI ReadFile (Arm1_1..4 / Arm2_1..4). SPEC-V899-MultiEP-FullPort.
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
    for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*8+7)<SLKIndClass.size(); i++)   // golden ContactForce.cpp:1148-1148 取代：TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1148-1148 -- TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
        for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
        {
            for(int j=0; j<8; j++)
            {
                if(j<4)
                    sGroup.sprintf("Diameter_%0.3fmm_Arm1_%d", SLKIndClass[i*8+j]->dDiameter, j+1);
                else
                    sGroup.sprintf("Diameter_%0.3fmm_Arm2_%d", SLKIndClass[i*8+j]->dDiameter, j-3);
                SLKIndClass[i*8+j]->dLoadRate                  =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate",      1.0), 0.8, 1.5);
                SLKIndClass[i*8+j]->dContactOffset             =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset", 0.0), 10.0, -10.0);

                dTemp=SLKIndClass[i*8+j]->dLoadRate*(double)100.0;
                SLKIndClass[i*8+j]->trckbrDiameterInd->Position   =int(dTemp);
                SLKIndClass[i*8+j]->edtContactOffsetInd->Text     =SLKIndClass[i*8+j]->dContactOffset;
            }
        }
    }
    else
    {
    for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*16+15)<SLKIndClass.size(); i++)   // golden ContactForce.cpp:1167-1167 取代：TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1167-1167 -- TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
    for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
    {
        for(int j=0; j<16; j++)
        {
            sGroup.sprintf("Diameter_%0.3fmm_%d", SLKIndClass[i*16+j]->dDiameter, j);
            SLKIndClass[i*16+j]->dLoadRate                  =CheckRange(CheckAndReadIniData(FileName, sGroup, "LoadRate",      1.0), 0.8, 1.5);
            SLKIndClass[i*16+j]->dContactOffset             =CheckRange(CheckAndReadIniData(FileName, sGroup, "ContactOffset", 0.0), 10.0, -10.0);

            dTemp=SLKIndClass[i*16+j]->dLoadRate*(double)100.0;
            SLKIndClass[i*16+j]->trckbrDiameterInd->Position   =int(dTemp);
            SLKIndClass[i*16+j]->edtContactOffsetInd->Text     =SLKIndClass[i*16+j]->dContactOffset;
        }
    }
    }

}

// golden ContactForce.cpp:1183  TfContactForce::WriteFile()
static void CF_WriteFile()
{
    filerw::ELMark("WriteFile");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString sGroup;
    double dTemp;
    bool bHasDiameter=false;
    AnsiString Str;

    //jou 20171117 (Steven) : EP校正欄位(從Contact頁面可以進入此setting)
    if(CosFunction.bUseDynamicKitDiameter==true)                                //Steven 20170605 : 可以自定義Kit直徑
    {
    if(SLKClass.size()<((bHasFile==false)?4u:((EP_Install==5)?8u:4u))) { filerw::ELTodo("golden ContactForce.cpp:1195-1258 needs SLKClass[0..3] (EP_Install==5: [0..7]) but [SLK Type] has fewer valid diameters -- golden AVs here; WriteFile skipped, nothing written"); return; } if(bHasFile==false)   // golden ContactForce.cpp:1193-1193 取代：越界守衛：golden :1195-1258 直接用 SLKClass[0..3]（EP_Install==5 且有檔時 [0..7]）；[SLK Type] 有效口徑不足 4 個（EP_Install==5 是 8 筆）時 golden 會當。這裡整支 WriteFile 不做（不寫任何檔）並記 todo；頁面存檔在 FileRW/ContactForce.cpp 先擋（連 SaveLastSetIni 都不跑）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1193-1193 -- 越界守衛：golden :1195-1258 直接用 SLKClass[0..3]（EP_Install==5 且有檔時 [0..7]）；[SLK Type] 有效口徑不足 4 個（EP_Install==5 是 8 筆）時 golden 會當。這裡整支 WriteFile 不做（不寫任何檔）並記 todo；頁面存檔在 FileRW/ContactForce.cpp 先擋（連 SaveLastSetIni 都不跑）
        if(bHasFile==false)                                                     //把舊的資料讀取出來
#endif // GATE (S12-C save)
        {
            SLKClass[0]->trckbrDiameter->Position=LastSet.dIndexLoadRate[0][3]*100;
            SLKClass[1]->trckbrDiameter->Position=LastSet.dIndexLoadRate[0][2]*100;
            SLKClass[2]->trckbrDiameter->Position=LastSet.dIndexLoadRate[0][0]*100;
            SLKClass[3]->trckbrDiameter->Position=LastSet.dIndexLoadRate[0][1]*100;

            SLKClass[0]->trckbrDiameter_NS->Position=LastSet.dIndexLoadRate[1][3]*100;
            SLKClass[1]->trckbrDiameter_NS->Position=LastSet.dIndexLoadRate[1][2]*100;
            SLKClass[2]->trckbrDiameter_NS->Position=LastSet.dIndexLoadRate[1][0]*100;
            SLKClass[3]->trckbrDiameter_NS->Position=LastSet.dIndexLoadRate[1][1]*100;

            SLKClass[0]->edtHotOffset->Text=LastSet.dIndexLoadRate[2][3];
            SLKClass[1]->edtHotOffset->Text=LastSet.dIndexLoadRate[2][2];
            SLKClass[2]->edtHotOffset->Text=LastSet.dIndexLoadRate[2][0];
            SLKClass[3]->edtHotOffset->Text=LastSet.dIndexLoadRate[2][1];
        }
        else if(EP_Install==5)
        {
            dTemp=double(SLKClass[0]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][3]=dTemp;
            dTemp=double(SLKClass[2]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][2]=dTemp;
            dTemp=double(SLKClass[4]->trckbrDiameter->Position)/100.0;          //Ifor 20200408 : [1] --> [2]
            LastSet.dIndexLoadRate[0][0]=dTemp;
            dTemp=double(SLKClass[6]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][1]=dTemp;

            dTemp=double(SLKClass[1]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[1][3]=dTemp;
            dTemp=double(SLKClass[3]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[1][2]=dTemp;
            dTemp=double(SLKClass[5]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[1][0]=dTemp;
            dTemp=double(SLKClass[7]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[1][1]=dTemp;

            LastSet.dIndexLoadRate[2][3]=atof(SLKClass[0]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][2]=atof(SLKClass[1]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][0]=atof(SLKClass[2]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][1]=atof(SLKClass[3]->edtHotOffset->Text.c_str());
        }
        else
        {
            dTemp=double(SLKClass[0]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][3]=dTemp;
            dTemp=double(SLKClass[1]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][2]=dTemp;
            dTemp=double(SLKClass[2]->trckbrDiameter->Position)/100.0;          //Ifor 20200408 : [1] --> [2]
            LastSet.dIndexLoadRate[0][0]=dTemp;
            dTemp=double(SLKClass[3]->trckbrDiameter->Position)/100.0;
            LastSet.dIndexLoadRate[0][1]=dTemp;

            dTemp=double(SLKClass[0]->trckbrDiameter_NS->Position)/100.0;
            LastSet.dIndexLoadRate[1][3]=dTemp;
            dTemp=double(SLKClass[1]->trckbrDiameter_NS->Position)/100.0;
            LastSet.dIndexLoadRate[1][2]=dTemp;
            dTemp=double(SLKClass[2]->trckbrDiameter_NS->Position)/100.0;
            LastSet.dIndexLoadRate[1][0]=dTemp;
            dTemp=double(SLKClass[3]->trckbrDiameter_NS->Position)/100.0;
            LastSet.dIndexLoadRate[1][1]=dTemp;

            LastSet.dIndexLoadRate[2][3]=atof(SLKClass[0]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][2]=atof(SLKClass[1]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][0]=atof(SLKClass[2]->edtHotOffset->Text.c_str());
            LastSet.dIndexLoadRate[2][1]=atof(SLKClass[3]->edtHotOffset->Text.c_str());
        }

        for(unsigned int i=0; i<SLKClass.size(); i++)
        {
            if(CUSTOMER_CODE==CC_ASE_SG && SLKClass[i]->dDiameter==80)
            {
                if(bHasDiameter==false)
                {
                    bHasDiameter=true;
                    sGroup.sprintf("Diameter_%0.3fmm", SLKClass[i]->dDiameter);
                }
                else
                {
                    sGroup.sprintf("Diameter_%0.3f_Himm", SLKClass[i]->dDiameter);
                }
            }
            else if(EP_Install==5)
            {
                sGroup.sprintf("Diameter_%smm", SLKClass[i]->sDiameter);
            }
            else
            {
                sGroup.sprintf("Diameter_%0.3fmm", SLKClass[i]->dDiameter);
            }
            //為了避免浮點數計算錯誤,要先強制為double
            //==>
            dTemp=double(SLKClass[i]->trckbrDiameter->Position)/100.0;
            WriteIniData(FileName, sGroup, "LoadRate",      dTemp);
            dTemp=double(SLKClass[i]->trckbrDiameter_NS->Position)/100.0;
            WriteIniData(FileName, sGroup, "LoadRate_NS",   dTemp);
            //<==
            //為了避免浮點數計算錯誤,要先強制為double
            WriteIniData(FileName, sGroup, "HotOffset",     SLKClass[i]->edtHotOffset->Text);
            WriteIniData(FileName, sGroup, "ContactOffset", SLKClass[i]->edtContactOffset->Text);
            WriteIniData(FileName, sGroup, "ContactOffset_NS", SLKClass[i]->edtContactOffset_NS->Text);//kevin 20170807 (Steven) Add NS offset
        }
        //AI(ht9045-v899) 20260504: full port V874.3 SLKIndClass ==MULTI WriteFile (Arm1_1..4 / Arm2_1..4). SPEC-V899-MultiEP-FullPort.
        if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
        {
    for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*8+7)<SLKIndClass.size(); i++)   // golden ContactForce.cpp:1298-1298 取代：TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1298-1298 -- TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
            for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
            {
                for(int j=0; j<8; j++)
                {
                    if(j<4)
                        sGroup.sprintf("Diameter_%0.3fmm_Arm1_%d", SLKIndClass[i*8+j]->dDiameter, j+1);
                    else
                        sGroup.sprintf("Diameter_%0.3fmm_Arm2_%d", SLKIndClass[i*8+j]->dDiameter, j-3);
                    dTemp=double(SLKIndClass[i*8+j]->trckbrDiameterInd->Position)/100.0;
                    WriteIniData(FileName, sGroup, "LoadRate",      dTemp);
                    WriteIniData(FileName, sGroup, "ContactOffset", SLKIndClass[i*8+j]->edtContactOffsetInd->Text);
                }
            }

    for(int ii=0; slDieForceOneByOneSLKType!=NULL && ii<slDieForceOneByOneSLKType->Count && (unsigned int)(ii*8+7)<DieForceOneByOneSLKClass.size(); ii++)   // golden ContactForce.cpp:1312-1312 取代：TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1312-1312 -- TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
            for(int ii=0; ii<EL<TControl>("TfContactForce", "slDieForceOneByOneSLKType")->Count; ii++)
#endif // GATE (S12-C save)
            {
                for(int j=0; j<8; j++)
                {
                    if(j<4)
                        sGroup.sprintf("DieForceDiameter_%0.3fmm_Arm1_%d", DieForceOneByOneSLKClass[ii*8+j]->dDiameter, j+1);
                    else
                        sGroup.sprintf("DieForceDiameter_%0.3fmm_Arm2_%d", DieForceOneByOneSLKClass[ii*8+j]->dDiameter, j-3);
                    dTemp=double(DieForceOneByOneSLKClass[ii*8+j]->trckbrDieForceOneByOneDiameter->Position)/100.0;
                    WriteIniData(FileName, sGroup, "LoadRate",      dTemp);
                    WriteIniData(FileName, sGroup, "ContactOffset", DieForceOneByOneSLKClass[ii*8+j]->edtDieForceOneByOneContactOffset->Text);
                }
            }
        }
        else
        {
    for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*16+15)<SLKIndClass.size(); i++)   // golden ContactForce.cpp:1328-1328 取代：TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1328-1328 -- TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）
            for(int i=0; i<EL<TControl>("TfContactForce", "slSLKTypeInd")->Count; i++)
#endif // GATE (S12-C save)
            {
                for(int j=0; j<16; j++)
                {
                    sGroup.sprintf("Diameter_%0.3fmm_%d", SLKIndClass[i*16+j]->dDiameter, j);
                    dTemp=double(SLKIndClass[i*16+j]->trckbrDiameterInd->Position)/100.0;
                    WriteIniData(FileName, sGroup, "LoadRate",      dTemp);
                    WriteIniData(FileName, sGroup, "ContactOffset", SLKIndClass[i*16+j]->edtContactOffsetInd->Text);
                }
            }
        }

        if(bHasFile==false)
        {
            bHasFile=true;
        }
    }

    if(INSTALL_DOUBLE_EP==1 || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                            //Ifor 20191003 : add Die Force 可以自定義Kit直徑 //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合
    {
        for(unsigned int i=0; i<DieForceSLKClass.size(); i++)
        {
            sGroup.sprintf("DieForceDiameter_%0.3fmm", DieForceSLKClass[i]->dDiameter);
            dTemp=double(DieForceSLKClass[i]->trckbrDieForceDiameter->Position)/100.0;
            WriteIniData(FileName, sGroup, "LoadRate",      dTemp);
            WriteIniData(FileName, sGroup, "ContactOffset", DieForceSLKClass[i]->edtDieForceContactOffset->Text);
        }
    }

    WriteIniDataGeneral("System", "EP_MAXKPA", EL<TEdit>("TfContactForce", "edMaxKpa")->Text);
    WriteIniDataGeneral("System", "EP_MAXA"  , EL<TEdit>("TfContactForce", "edMaxMpaFB")->Text);
    WriteIniDataGeneral("System", "EP_MINMPA", EL<TEdit>("TfContactForce", "edMinMpa")->Text);                 //JerryYang 20171023 (wei) add PA Min
    WriteIniDataGeneral("System" , "EP_MINA_FeedBack", EL<TEdit>("TfContactForce", "edtMinMpaFB")->Text);

    WriteIniDataGeneral("System", "EP_MAXKPA_1032", EL<TEdit>("TfContactForce", "edMaxKpa_1032")->Text);
    WriteIniDataGeneral("System", "EP_MAXA_1032"  , EL<TEdit>("TfContactForce", "edMaxMpaFB_1032")->Text);
    WriteIniDataGeneral("System", "EP_MINMPA_1032", EL<TEdit>("TfContactForce", "edMinMpa_1032")->Text);       //JerryYang 20171023 (wei) add PA Min
    WriteIniDataGeneral("System", "EP_MINA_FeedBack_1032", EL<TEdit>("TfContactForce", "edtMinMpaFB_1032")->Text);

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<15; j++)                                                 //Ifor 20230206 add: 新增四組Contact 補償 7 => 11  //Ifor 20230829 add: 800KG 11 -> 15
        {
            Str.sprintf("dIndexZOffset[%d][%d]", i, j);                         //JerryYang 20190910 fix ATC offset存檔問題
    if(IndexZOffsetEdit[i][j]!=NULL) WriteIniDataGeneral("Test Arm", Str, IndexZOffsetEdit[i][j]->Text); else if(i==0 && j==0) filerw::ELTodo("golden ContactForce.cpp:1372 IndexZOffsetEdit is NULL (WriteFile before any ReadFile: ctor no-file path, golden AVs here) -- Gerneral.ini [Test Arm] dIndexZOffset[0..1][0..14] not written");   // golden ContactForce.cpp:1372-1372 取代：IndexZOffsetEdit 是 NULL（golden 只在 ReadFile :972-980 指派；「沒有 ContactInfo.ini、建構子直接 WriteFile」那一次 golden 會 AV）→ 那 30 鍵不寫（Gerneral.ini 原值不動）並記 todo；指派過就照 golden 寫
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1372-1372 -- IndexZOffsetEdit 是 NULL（golden 只在 ReadFile :972-980 指派；「沒有 ContactInfo.ini、建構子直接 WriteFile」那一次 golden 會 AV）→ 那 30 鍵不寫（Gerneral.ini 原值不動）並記 todo；指派過就照 golden 寫
            WriteIniDataGeneral("Test Arm", Str, IndexZOffsetEdit[i][j]->Text); //JerryYang 20171023 (wei) add PA Min
#endif // GATE (S12-C save)
        }
    }
    CF_ReadFile();

}

// golden ContactForce.cpp:1378  TfContactForce::tb30mm_10kgChange(TObject *Sender)
static void CF_tb30mm_10kgChange()
{

    ;   // golden ContactForce.cpp:1380-1380 取代：Sender：事件處理器參數已拿掉（ELTrackBar::OnChange 無參數）；Ptr 只給下一行的 EP 輸出用
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1380-1380 -- Sender：事件處理器參數已拿掉（ELTrackBar::OnChange 無參數）；Ptr 只給下一行的 EP 輸出用
    TTrackBar *Ptr=(TTrackBar *)Sender;
#endif // GATE (S12-C save)
    filerw::ELTodo("golden ContactForce.cpp:1381 ADAM_DirectWriteData(tb*->Position,0) not done (EP analog output = machine action, Jimmy; ADAM_* are empty in the port)");   // golden ContactForce.cpp:1381-1381 取代：ADAM_DirectWriteData(Ptr->Position,0)：EP 類比輸出（硬體，會讓機台動作 → 歸 Jimmy；移植樹 ADAM_* 是空殼）。golden 在 FormShow :891-898 設 tb*->Position 時也會經 OnChange 觸發它（開頁就把 8 個值輸出到 EP 一次）
#if 0 // GATE (S12-C save) golden ContactForce.cpp:1381-1381 -- ADAM_DirectWriteData(Ptr->Position,0)：EP 類比輸出（硬體，會讓機台動作 → 歸 Jimmy；移植樹 ADAM_* 是空殼）。golden 在 FormShow :891-898 設 tb*->Position 時也會經 OnChange 觸發它（開頁就把 8 個值輸出到 EP 一次）
    ADAM_DirectWriteData(Ptr->Position,0);                                      //Ifor 20150709 ：加入設備位置
#endif // GATE (S12-C save)
    CF_ShowValue();

}

// golden ContactForce.cpp:1385  TfContactForce::ShowValue()
static void CF_ShowValue()
{

    EL<TLabel>("TfContactForce", "lab30mm10kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_10kg")->Position;
    EL<TLabel>("TfContactForce", "lab30mm60kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb30mm_60kg")->Position;
    EL<TLabel>("TfContactForce", "lab40mm10kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_10kg")->Position;
    EL<TLabel>("TfContactForce", "lab40mm60kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb40mm_60kg")->Position;
    EL<TLabel>("TfContactForce", "lab60mm10kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_10kg")->Position;
    EL<TLabel>("TfContactForce", "lab60mm60kgNum")->Caption=EL<filerw::ELTrackBar>("TfContactForce", "tb60mm_60kg")->Position;

}

