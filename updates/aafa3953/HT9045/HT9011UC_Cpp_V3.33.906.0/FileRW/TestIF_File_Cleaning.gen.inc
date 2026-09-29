// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_Cleaning 的讀寫檔（HTEditList： → HandlerCondition.Data（[Configuration] iAutoClean_*／dBufferKit*／Hotplatl*、[AutoClean]）、IniData\DefineAutoClean\AutoClean.Data（CC_TSMC_TAINAN／CosFunction.bUseDefineAutoCleanOffset）、HandlerCondition.Data／AutoCleanCount.Data 的 iAutoCleanPad_CountTime_*（ReadWriteAutoCleanCount））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean/uCleaning.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden AutoClean/uCleaning.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 142 個（header 宣告 246 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "forms/fMain.h"
#include "forms/fSecurity.h"
#include "forms/fSetup.h"
#include "forms/fCleaning.h"
#include "forms/fSpeed.h"
#include "forms/fShowBinSelect.h"
#include "forms/fLotInfo.h"
#include "Motor/mymotor.h"
#include "AutoClean/AutoClean.h"
#include "cUnitConvert.h"
#include "cinitial.h"
#include "ainarm9045.h"
#include "ainarm9045_2x6_8.h"
#include "csystem.h"
#include "cMyDB.h"
#include "FileRW/CfgTrayPlate.h"
#include "BarcodeReader.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "FileRW/_EditPage.h"   // AI(W906-FRW-S157) 20260927 [W906]：filerw::PageEvent（檔尾 kCL_Events）

struct CL_KbEntry { const char* name; void (*fn)(TObject*); const char* parent; int dfmLine; };   // 小鍵盤表（members）
int    FileRW_Cleaning_SearchCleanNum();              // FileRW/TestIF_File_Cleaning.cpp（golden AutoClean.cpp:1324，V912）
void   FileRW_Cleaning_ContactDutCount();             // FileRW/TestIF_File_Cleaning.cpp 下半（fContactForm->DutCount）
double FileRW_Cleaning_ContactDutCountValue();        // 同上（fContactForm->dDutCount）
double FileRW_Cleaning_ContactMaxIndexForceLimit();   // 同上（cContact.h ComputeMaxIndexForceLimit）
extern int  iAutoCleanCnt;                            // golden ainarm2.h:89（移植樹定義在 ainarm2.cpp:96，沒有 header 宣告）
extern bool bAuthCriticalPara[26];                    // cAuthority.h:64（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）
extern int  iCloseSiteModeFor2x6;                     // golden ainarm9045_2x6_8.h:23（aHotPlateSubstrate.h:902 與 HTEditList.h 衝突）
extern int  iCloseSiteModeFor2x8;                     // golden ainarm9045_2x8_8.h:28（同上）
extern int  iAutoCleanUseXPitch;                      // golden ainarm2.h:177（aHotPlateSubstrate.h:802，同上）
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr=false, AnsiString errPart=" ");   // canary_support.h:66
static void CL_XCT1Change(TObject *Sender);
static void CL_XCT1Click(TObject *Sender);
static void CL_XPitch2Change(TObject *Sender);
static void CL_XST1Click(TObject *Sender);
static void CL_YCT1Change(TObject *Sender);
static void CL_edACContactCleanHeightClick(TObject *Sender);
static void CL_edACContactShiftHeightClick(TObject *Sender);
static void CL_edACSmartCTFMouseDown(TObject *Sender);
static void CL_edAdaptiveIntervalAdjMouseDown(TObject *Sender);
static void CL_edAdaptiveIntervalMaxMouseDown(TObject *Sender);
static void CL_edAdaptiveIntervalMinMouseDown(TObject *Sender);
static void CL_edAutoCleanAirForce_KgClick(TObject *Sender);
static void CL_edAutoCleanAirForce_NClick(TObject *Sender);
static void CL_edCleanCountClick(TObject *Sender);
static void CL_edDevicePicesClick(TObject *Sender);
static void CL_edDropOffset1Click(TObject *Sender);
static void CL_edPinSingleGfClick(TObject *Sender);
static void CL_edPinSingleNClick(TObject *Sender);
static void CL_edPinsCountClick(TObject *Sender);
static void CL_edTimeCTClick(TObject *Sender);
static void CL_edtACSmartClick(TObject *Sender);
static void CL_edtACSmartMouseDown(TObject *Sender);
static void CL_edtACSmart_ContactTimeMouseDown(TObject *Sender);
static void CL_edtBufferKitLTXClick(TObject *Sender);
static void CL_edtHotplatePickOffsetClick(TObject *Sender);
static void CL_edtIndexVacuumClick(TObject *Sender);
static void CL_edtLowYieldCountClick(TObject *Sender);
static void CL_edtLowYieldLimitClick(TObject *Sender);
static void CL_rgCleanKitTypeClick();
static void CL_rgKitPositionClick();

// golden AutoClean/uCleaning.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;             // golden uCleaning.h:346 —— 本 TU 自己的（網頁沒有關表單事件；Exit 才跑 FormClose 設回 false）
#define bResetCleanCount (fCleaning->bResetCleanCount)                // golden uCleaning.h:333 —— 移植樹 AutoClean.cpp:3967/:4035 寫它
#define iPosTemp (fCleaning->iPosTemp)                                // golden uCleaning.h:338
#define b1x2SiteAbClosePutDummy (fCleaning->b1x2SiteAbClosePutDummy)  // golden uCleaning.h:345 —— 移植樹 AutoClean.cpp:2392/:3531/:4185 讀它
#define b12SiteRun2x4 (fCleaning->b12SiteRun2x4)                      // golden uCleaning.h:360 —— 移植樹 forms/fCleaning.cpp XCT1Change 同一份
#define iDeviceCount (fCleaning->iDeviceCount)                        // golden uCleaning.h:361
#define SearchCleanNum FileRW_Cleaning_SearchCleanNum                 // golden AutoClean.cpp:1324（移植樹 AutoClean.cpp:1662 是 static、906 版）
static const AnsiString* CL_KbPending=nullptr;
#define fConfiguration (W906_CfgTrayPlate())   // AI(W906-FRW-S98) 20260926: golden fConfiguration 的 Tray 表（FileRW/CfgTrayPlate.cpp；FormShow :1488-1499 用）。移植樹叫 fConfiguration 的全域是 SCK_ART 替身（Automation/SCK_ART_Remainder.cpp:352），本 TU 不用它
static int Get0_01MMType(const char* s) { return Get0_01MMType(const_cast<char*>(s)); }   // cpublic.h:19
static void CL_EditText(TCustomEdit* e, const AnsiString& v) { if(e->Text==v) return; e->Text=v; if(e==filerw::EL<TEdit>("TfCleaning", "XCT1") || e==filerw::EL<TEdit>("TfCleaning", "XCT2")) CL_XCT1Change(e); else if(e==filerw::EL<TEdit>("TfCleaning", "YCT1") || e==filerw::EL<TEdit>("TfCleaning", "YCT2")) CL_YCT1Change(e); else if(e==filerw::EL<TEdit>("TfCleaning", "XPitch2")) CL_XPitch2Change(e); }   // (1) golden DFM OnChange：XCT1/XCT2=XCT1Change、YCT1/YCT2=YCT1Change、XPitch2=XPitch2Change
static void CL_RadioIndex(TRadioGroup* rg, int v) { if(v<-1) v=-1; if(rg->Items && v>=rg->Items->Count) v=rg->Items->Count-1; if(rg->ItemIndex==v) return; rg->ItemIndex=v; if(rg==filerw::EL<TRadioGroup>("TfCleaning", "rgCleanKitType")) CL_rgCleanKitTypeClick(); else if(rg==filerw::EL<TRadioGroup>("TfCleaning", "rgKitPosition")) CL_rgKitPositionClick(); }   // (2) golden DFM OnClick：rgCleanKitType/rgKitPosition
static bool CL_VarGE(int lhs, const AnsiString& rhs) { return (double)lhs >= StrToFloat(rhs); }   // (3)
static void CL_QwertyKey(TControl* Ptr, int iFunction, int iDP=0, bool bCheckRange=false, double min=0, double max=0) { (void)iDP; TPanel* PanelPtr=dynamic_cast<TPanel*>(Ptr); TCustomEdit* EditPtr=dynamic_cast<TCustomEdit*>(Ptr); AnsiString txt = CL_KbPending ? *CL_KbPending : (PanelPtr ? PanelPtr->Caption : (EditPtr ? EditPtr->Text : AnsiString(""))); if(iFunction&N_PORT) { bCheckRange=true; if(min<0 || max<=0) { min=0; max=65535; } } if(iFunction&N_INTEGER || iFunction&N_DOUBLE) { double d=atof(txt.c_str()); if(bCheckRange) txt=AnsiString(CheckRange(d, min, max)); } if(PanelPtr!=NULL) PanelPtr->Caption=txt; else if(EditPtr!=NULL) CL_EditText(EditPtr, txt); }   // (5) golden :256-301（TEdit 寫回照 (1) 觸發 OnChange）
static const CL_KbEntry kCL_Kb[] = {   // golden uCleaning.dfm：元件 → 小鍵盤處理器（DFM 行）
    {"edtBufferKitLTX", &CL_edtBufferKitLTXClick, "grpBufferKitLT", 94},
    {"edtBufferKitLTY", &CL_edtBufferKitLTXClick, "grpBufferKitLT", 109},
    {"edtBufferKitRTX", &CL_edtBufferKitLTXClick, "grpBufferKitRT", 160},
    {"edtBufferKitRTY", &CL_edtBufferKitLTXClick, "grpBufferKitRT", 175},
    {"edtBufferKitLDX", &CL_edtBufferKitLTXClick, "grpBufferKitLD", 226},
    {"edtBufferKitLDY", &CL_edtBufferKitLTXClick, "grpBufferKitLD", 241},
    {"edtBufferKitRDX", &CL_edtBufferKitLTXClick, "grpBufferKitRD", 292},
    {"edtBufferKitRDY", &CL_edtBufferKitLTXClick, "grpBufferKitRD", 307},
    {"edtFailAlarmSiteYieldDifferent", &CL_edtLowYieldCountClick, "gbYieldAlarm", 393},
    {"edtFailAlarmSiteYield", &CL_edtLowYieldLimitClick, "gbYieldAlarm", 425},
    {"edtLowYieldLimit", &CL_edtLowYieldLimitClick, "gbYieldAlarm", 457},
    {"edtLowYieldCount", &CL_edtLowYieldCountClick, "gbYieldAlarm", 473},
    {"edtConseFailureCountBySocket_Retest", &CL_edtLowYieldCountClick, "gbConsFailureSocket", 534},
    {"edtConseFailureCountBySocket_Normal", &CL_edtLowYieldCountClick, "gbConsFailureSocket", 550},
    {"edtConseFailureCountByHead_Retest", &CL_edtLowYieldCountClick, "gbConsFailureHead", 635},
    {"edtConseFailureCountByHead_Normal", &CL_edtLowYieldCountClick, "gbConsFailureHead", 651},
    {"edtACSmart_ContactTime", &CL_edtACSmart_ContactTimeMouseDown, "tsSmart", 782},
    {"edtACSmart_ACContactCount", &CL_edtLowYieldLimitClick, "tsSmart", 798},
    {"edAdaptiveIntervalAdj", &CL_edAdaptiveIntervalAdjMouseDown, "tsSmart", 816},
    {"edAdaptiveIntervalMin", &CL_edAdaptiveIntervalMinMouseDown, "tsSmart", 825},
    {"edAdaptiveIntervalMax", &CL_edAdaptiveIntervalMaxMouseDown, "tsSmart", 834},
    {"edACSmartCTF", &CL_edACSmartCTFMouseDown, "tsSmart", 851},
    {"edtACSmart", &CL_edtACSmartMouseDown, "tsSmart", 860},
    {"edTimeCT", &CL_edTimeCTClick, "tsTimeCT", 881},
    {"XST1", &CL_XST1Click, "tsKit", 1081},
    {"XPitch1", &CL_XST1Click, "tsKit", 1097},
    {"XCT1", &CL_XCT1Click, "tsKit", 1113},
    {"YST1", &CL_XST1Click, "tsKit", 1128},
    {"YPitch1", &CL_XST1Click, "tsKit", 1143},
    {"YCT1", &CL_XCT1Click, "tsKit", 1159},
    {"XPitch2", &CL_XST1Click, "tabTray", 1281},
    {"XST2", &CL_XST1Click, "tabTray", 1296},
    {"XCT2", &CL_XCT1Click, "tabTray", 1312},
    {"YST2", &CL_XST1Click, "tabTray", 1327},
    {"YPitch2", &CL_XST1Click, "tabTray", 1342},
    {"YCT2", &CL_XCT1Click, "tabTray", 1358},
    {"edIntervalContact", &CL_edtLowYieldCountClick, "grpCleanMode", 1451},
    {"edDevicePices", &CL_edDevicePicesClick, "grpCleanDevice", 1861},
    {"edAlarmCount", &CL_edtLowYieldCountClick, "grpCleanDevice", 1893},
    {"edCleaningCount", &CL_edCleanCountClick, "grpCleanDevice", 1930},
    {"edCleanPadDeviation", &CL_edDropOffset1Click, "grpCleanDevice", 1947},
    {"edIndexArmAutoCleanCnt", &CL_edCleanCountClick, "grpCleanDevice", 1982},
    {"OutArmSpeed", &CL_edtLowYieldLimitClick, "grpSpeed", 2065},
    {"ShuttleSpeed", &CL_edtLowYieldLimitClick, "grpSpeed", 2083},
    {"IndexArmSpeed", &CL_edtLowYieldLimitClick, "grpSpeed", 2101},
    {"edtInArmZSpeed", &CL_edtLowYieldLimitClick, "grpSpeed", 2119},
    {"edtIndexVacuum", &CL_edtIndexVacuumClick, "grpSpeed", 2137},
    {"edtInArmVacuum", &CL_edtIndexVacuumClick, "grpSpeed", 2155},
    {"edtIndexAirOn", &CL_edtIndexVacuumClick, "grpSpeed", 2173},
    {"edtInArmAirOn", &CL_edtIndexVacuumClick, "grpSpeed", 2191},
    {"edtHotplateXOffset", &CL_edtBufferKitLTXClick, "gbCleanKitOffset", 2251},
    {"edtHotplateYOffset", &CL_edtBufferKitLTXClick, "gbCleanKitOffset", 2266},
    {"edtHotplatePlaceOffset", &CL_edtHotplatePickOffsetClick, "gbCleanKitOffset", 2281},
    {"edtHotplatePickOffset", &CL_edtHotplatePickOffsetClick, "gbCleanKitOffset", 2296},
    {"edtHotplatePitchOffset", &CL_edtBufferKitLTXClick, "gbCleanKitOffset", 2312},
    {"edPinSingleN", &CL_edPinSingleNClick, "grpContactForce", 2432},
    {"edPinSingleGf", &CL_edPinSingleGfClick, "grpContactForce", 2450},
    {"edPinsCount", &CL_edPinsCountClick, "grpContactForce", 2467},
    {"edAutoCleanAirForce_Kg", &CL_edAutoCleanAirForce_KgClick, "grpContactForce", 2486},
    {"edAutoCleanAirForce_N", &CL_edAutoCleanAirForce_NClick, "grpContactForce", 2505},
    {"edtAutoCleanDieForce", &CL_edAutoCleanAirForce_KgClick, "grpDieForce", 2530},
    {"edACContactShiftHeight", &CL_edACContactShiftHeightClick, "grpHeight", 2583},
    {"edACContactCleanHeight", &CL_edACContactCleanHeightClick, "grpHeight", 2598},
    {"edIndexPickOffset", &CL_edtHotplatePickOffsetClick, "grpHeight", 2624},
    {"edShuttle1PickOffset", &CL_edtHotplatePickOffsetClick, "pnlInArmOffset", 2712},
    {"edtShuttle1PlaceOffset", &CL_edtHotplatePickOffsetClick, "pnlInArmOffset", 2727},
    {"edtShuttle1XOffset", &CL_edtBufferKitLTXClick, "pnlInArmOffset", 2742},
    {"edtShuttle1YOffset", &CL_edtBufferKitLTXClick, "pnlInArmOffset", 2757},
    {"edShuttle2PickOffset", &CL_edtHotplatePickOffsetClick, "pnlInArmOffset", 2772},
    {"edtShuttle2PlaceOffset", &CL_edtHotplatePickOffsetClick, "pnlInArmOffset", 2787},
    {"edtShuttle2XOffset", &CL_edtBufferKitLTXClick, "pnlInArmOffset", 2802},
    {"edtShuttle2YOffset", &CL_edtBufferKitLTXClick, "pnlInArmOffset", 2817},
    {"edtShuttlePitch", &CL_edtBufferKitLTXClick, "pnlInArmOffset", 2832},
    {"edIndexReleaseOffset", &CL_edtHotplatePickOffsetClick, "grpHeight", 2848},
    {"edContactTime", &CL_edDropOffset1Click, "pnlContactMode", 2959},
    {"edACContactCount", &CL_edtLowYieldLimitClick, "pnlContactMode", 2975},
    {"edDropOffset1", &CL_edDropOffset1Click, "pnlContactMode", 3002},
};

// golden AutoClean/uCleaning.cpp 檔案層級全域（本表單私用，照抄成 static）
static int iBackupCleaningDeveicePices;   // golden AutoClean/uCleaning.cpp:39
static int iBackupCleaningItemX;   // golden AutoClean/uCleaning.cpp:40
static int iBackupCleaningItemY;   // golden AutoClean/uCleaning.cpp:41
static int iBackupAutoClean_Function;   // golden AutoClean/uCleaning.cpp:42

using filerw::EL;

// 前置宣告
static void CL_TfCleaning();
static void CL_SetArmCaption();
static void CL_LoadAutoCleanData();
static void CL_SaveAutoCleanData();
static void CL_LoadImage();
static int CL_GetMinCleanPadCount();
static int CL_SetDeviceMaxMin();
static void CL_FormShow();
static void CL_ShowTranGfToN();
static void CL_ShowTranNToGf();
static void CL_ShowCleanContactForce();
static void CL_sbCleanSaveClick();
static void CL_sbCleanExitClick();
static void CL_rgKitPositionClick();
static void CL_YCT1Change(TObject *Sender);
static void CL_DrawAutoClean();
static void CL_FormClose();
static void CL_XCT1Change(TObject *Sender);
static void CL_rgCleanKitTypeClick();
static void CL_XPitch2Change(TObject *Sender);
static void CL_ChangeEditToHPMode();
static void CL_cbbCleanPadCountChange();
static void CL_edPinSingleGfClick(TObject *Sender);
static void CL_edPinSingleNClick(TObject *Sender);
static void CL_edPinsCountClick(TObject *Sender);
static void CL_edCleanCountClick(TObject *Sender);
static void CL_edtBufferKitLTXClick(TObject *Sender);
static void CL_edtHotplatePickOffsetClick(TObject *Sender);
static void CL_edACContactCleanHeightClick(TObject *Sender);
static void CL_edtLowYieldLimitClick(TObject *Sender);
static void CL_edtLowYieldCountClick(TObject *Sender);
static void CL_XST1Click(TObject *Sender);
static void CL_XCT1Click(TObject *Sender);
static void CL_edACContactShiftHeightClick(TObject *Sender);
static void CL_edDropOffset1Click(TObject *Sender);
static void CL_edDevicePicesClick(TObject *Sender);
static void CL_edAutoCleanAirForce_KgClick(TObject *Sender);
static void CL_edAutoCleanAirForce_NClick(TObject *Sender);
static void CL_edtACSmartClick(TObject *Sender);
static void CL_edTimeCTClick(TObject *Sender);
static void CL_edtIndexVacuumClick(TObject *Sender);
static void CL_edAdaptiveIntervalMaxMouseDown(TObject *Sender);
static void CL_edAdaptiveIntervalMinMouseDown(TObject *Sender);
static void CL_edAdaptiveIntervalAdjMouseDown(TObject *Sender);
static void CL_edACSmartCTFMouseDown(TObject *Sender);
static void CL_edtACSmart_ContactTimeMouseDown(TObject *Sender);
static void CL_edtACSmartMouseDown(TObject *Sender);
static void CL_cbbSelectTrayChange();
static void CL_btIncludeClick();
static void CL_btnResetCleanCountClick();
static void CL_btnResetIntervalClick();
static void CL_ChangeACSmartInterval(int iChangeMode, AnsiString sMsg);
static void CL_sbTrayAssignClick();

// golden AutoClean/uCleaning.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void CL_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfCleaning", "ContactMode")->Items;
      s->Add("Direct Contact Mode");
      s->Add("Drop Contact Mode");
    }
    { TStringList* s = EL<TComboBox>("TfCleaning", "cbbACSmart_ContactMode")->Items;
      s->Add("Direct Contact Mode");
      s->Add("Drop Contact Mode");
    }
    { TStringList* s = EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Items;
      s->Add("12");
      s->Add("16");
      s->Add("24");
      s->Add("32");
    }
    { TStringList* s = EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Items;
      s->Add("Arm1");
      s->Add("Arm2");
      s->Add("Arm1 & Arm2");
    }
    { TStringList* s = EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->Items;
      s->Add("Kit");
      s->Add("Tray");
    }
    { TStringList* s = EL<TRadioGroup>("TfCleaning", "rgKitPosition")->Items;
      s->Add("Fix3");
      s->Add("Hot Plate 2");
      s->Add("Clean Kit");
      s->Add("Clean Air");
    }
    { TStringList* s = EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
}

// golden AutoClean/uCleaning.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kCL_ParentOf[][2] = {
    {"ContactMode", "pnlContactMode"},
    {"Image1", "tabTray"},
    {"Image3", "tsKit"},
    {"IndexArmSpeed", "grpSpeed"},
    {"Label21", "grpCleanDevice"},
    {"Label288", "grpCleanDevice"},
    {"OutArmSpeed", "grpSpeed"},
    {"PageTypeName", "Panel7"},
    {"Panel7", "pnlKitSetting"},
    {"ShuttleSpeed", "grpSpeed"},
    {"XCT1", "tsKit"},
    {"XCT2", "tabTray"},
    {"XPitch1", "tsKit"},
    {"XPitch2", "tabTray"},
    {"XST1", "tsKit"},
    {"XST2", "tabTray"},
    {"YCT1", "tsKit"},
    {"YCT2", "tabTray"},
    {"YPitch1", "tsKit"},
    {"YPitch2", "tabTray"},
    {"YST1", "tsKit"},
    {"YST2", "tabTray"},
    {"btFocusOnly", "grpCleanMode"},
    {"btInclude", "tabTray"},
    {"btnResetCleanCount", "grpCleanDevice"},
    {"cbConseFailureByHead_Normal", "gbConsFailureHead"},
    {"cbConseFailureByHead_Retest", "gbConsFailureHead"},
    {"cbConseFailureBySocket_Normal", "gbConsFailureSocket"},
    {"cbConseFailureBySocket_Retest", "gbConsFailureSocket"},
    {"cbFailAlarmLowYield", "gbYieldAlarm"},
    {"cbFailAlarmSiteYieldDifferent", "gbYieldAlarm"},
    {"cbbACSmart_ContactMode", "tsSmart"},
    {"cbbCleanPadCount", "grpCleanDevice"},
    {"cbbSelectTray", "pnlCleanPadAlign"},
    {"chkACSmart", "tsSmart"},
    {"chkAutoCleanMode1", "grpCleanMode"},
    {"chkAutoCleanMode2", "grpCleanMode"},
    {"chkAutoCleanMode3", "grpCleanMode"},
    {"chkAutoCleanMode4", "grpCleanMode"},
    {"chkAutoCleanMode5", "grpCleanMode"},
    {"chkAutoCleanMode6", "grpCleanMode"},
    {"chkCleanOtherArm", "pnlCleanArm"},
    {"chkE43", "tsKit"},
    {"chkTimeCT", "tsTimeCT"},
    {"chkUseNSKit", "tsKit"},
    {"edACContactCleanHeight", "grpHeight"},
    {"edACContactCount", "pnlContactMode"},
    {"edACContactShiftHeight", "grpHeight"},
    {"edACSmartCTF", "tsSmart"},
    {"edAdaptiveIntervalAdj", "tsSmart"},
    {"edAdaptiveIntervalMax", "tsSmart"},
    {"edAdaptiveIntervalMin", "tsSmart"},
    {"edAlarmCount", "grpCleanDevice"},
    {"edAutoCleanAirForce_Kg", "grpContactForce"},
    {"edAutoCleanAirForce_N", "grpContactForce"},
    {"edCleanPadDeviation", "grpCleanDevice"},
    {"edCleaningCount", "grpCleanDevice"},
    {"edContactTime", "pnlContactMode"},
    {"edDevicePices", "grpCleanDevice"},
    {"edDropOffset1", "pnlContactMode"},
    {"edIndexArmAutoCleanCnt", "grpCleanDevice"},
    {"edIndexPickOffset", "grpHeight"},
    {"edIndexReleaseOffset", "grpHeight"},
    {"edIntervalContact", "grpCleanMode"},
    {"edPinSingleGf", "grpContactForce"},
    {"edPinSingleN", "grpContactForce"},
    {"edPinsCount", "grpContactForce"},
    {"edShuttle1PickOffset", "pnlInArmOffset"},
    {"edShuttle2PickOffset", "pnlInArmOffset"},
    {"edTimeCT", "tsTimeCT"},
    {"edtACSmart", "tsSmart"},
    {"edtACSmart_ACContactCount", "tsSmart"},
    {"edtACSmart_ContactTime", "tsSmart"},
    {"edtACSmart_DropOffset1", "tsSmart"},
    {"edtAutoCleanDieForce", "grpDieForce"},
    {"edtBufferKitLDX", "grpBufferKitLD"},
    {"edtBufferKitLDY", "grpBufferKitLD"},
    {"edtBufferKitLTX", "grpBufferKitLT"},
    {"edtBufferKitLTY", "grpBufferKitLT"},
    {"edtBufferKitRDX", "grpBufferKitRD"},
    {"edtBufferKitRDY", "grpBufferKitRD"},
    {"edtBufferKitRTX", "grpBufferKitRT"},
    {"edtBufferKitRTY", "grpBufferKitRT"},
    {"edtConseFailureCountByHead_Normal", "gbConsFailureHead"},
    {"edtConseFailureCountByHead_Retest", "gbConsFailureHead"},
    {"edtConseFailureCountBySocket_Normal", "gbConsFailureSocket"},
    {"edtConseFailureCountBySocket_Retest", "gbConsFailureSocket"},
    {"edtFailAlarmSiteYield", "gbYieldAlarm"},
    {"edtFailAlarmSiteYieldDifferent", "gbYieldAlarm"},
    {"edtHotplatePickOffset", "gbCleanKitOffset"},
    {"edtHotplatePitchOffset", "gbCleanKitOffset"},
    {"edtHotplatePlaceOffset", "gbCleanKitOffset"},
    {"edtHotplateXOffset", "gbCleanKitOffset"},
    {"edtHotplateYOffset", "gbCleanKitOffset"},
    {"edtInArmAirOn", "grpSpeed"},
    {"edtInArmVacuum", "grpSpeed"},
    {"edtInArmZSpeed", "grpSpeed"},
    {"edtIndexAirOn", "grpSpeed"},
    {"edtIndexVacuum", "grpSpeed"},
    {"edtLowYieldCount", "gbYieldAlarm"},
    {"edtLowYieldLimit", "gbYieldAlarm"},
    {"edtShuttle1PlaceOffset", "pnlInArmOffset"},
    {"edtShuttle1XOffset", "pnlInArmOffset"},
    {"edtShuttle1YOffset", "pnlInArmOffset"},
    {"edtShuttle2PlaceOffset", "pnlInArmOffset"},
    {"edtShuttle2XOffset", "pnlInArmOffset"},
    {"edtShuttle2YOffset", "pnlInArmOffset"},
    {"edtShuttlePitch", "pnlInArmOffset"},
    {"gbCleanKitOffset", "pnlLeft"},
    {"gbConsFailureHead", "tsFailure"},
    {"gbConsFailureSocket", "tsFailure"},
    {"gbYieldAlarm", "tsYield"},
    {"grpBufferKitLD", "tsOffset"},
    {"grpBufferKitLT", "tsOffset"},
    {"grpBufferKitRD", "tsOffset"},
    {"grpBufferKitRT", "tsOffset"},
    {"grpCleanDevice", "pnlLeft"},
    {"grpCleanMode", "grpCleanPara"},
    {"grpCleanPara", "pnlLeft"},
    {"grpContactForce", "grpContactPara"},
    {"grpContactPara", "pnlCenter"},
    {"grpDieForce", "grpContactForce"},
    {"grpHeight", "grpContactPara"},
    {"grpSpeed", "pnlLeft"},
    {"lblInArm", "grpSpeed"},
    {"lblInArmZ", "grpSpeed"},
    {"lbledIndexArmAutoCleanCnt", "grpCleanDevice"},
    {"palBtn", "pnlCenter"},
    {"pcCleanYield", "grpTrayData"},
    {"pgCleanType", "pnlKitSetting"},
    {"pnlCleanArm", "grpContactPara"},
    {"pnlCleanPadAlign", "grpTrayData"},
    {"pnlContactMode", "grpContactPara"},
    {"pnlInArmOffset", "grpHeight"},
    {"pnlKitSetting", "grpTrayData"},
    {"pnlNotify", "grpHeight"},
    {"rgAutoCleanOnOff", "grpCleanPara"},
    {"rgAutoCleanSelectArm", "pnlCleanArm"},
    {"rgCleanKitType", "pnlKitSetting"},
    {"rgKitPosition", "grpCleanDevice"},
    {"rgShuttleCheck", "grpCleanDevice"},
    {"sbCleanExit", "palBtn"},
    {"sbCleanSave", "palBtn"},
    {"sbTrayAssign", "grpCleanDevice"},
    {"tabTray", "pgCleanType"},
    {"tmyAutoClean", "pnlCleanPadAlign"},
    {"tsFailure", "pcCleanYield"},
    {"tsKit", "pgCleanType"},
    {"tsOffset", "pcCleanYield"},
    {"tsSmart", "pcCleanYield"},
    {"tsTimeCT", "pcCleanYield"},
    {"tsYield", "pcCleanYield"},
    {"udDeviceCT", "grpCleanDevice"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void CL_CreateContainerProxies()
{
    EL<TPanel>("TfCleaning", "Panel7");
    EL<TGroupBox>("TfCleaning", "gbCleanKitOffset");
    EL<TGroupBox>("TfCleaning", "gbConsFailureHead");
    EL<TGroupBox>("TfCleaning", "gbConsFailureSocket");
    EL<TGroupBox>("TfCleaning", "gbYieldAlarm");
    EL<TGroupBox>("TfCleaning", "grpBufferKitLD");
    EL<TGroupBox>("TfCleaning", "grpBufferKitLT");
    EL<TGroupBox>("TfCleaning", "grpBufferKitRD");
    EL<TGroupBox>("TfCleaning", "grpBufferKitRT");
    EL<TGroupBox>("TfCleaning", "grpCleanDevice");
    EL<TGroupBox>("TfCleaning", "grpCleanMode");
    EL<TGroupBox>("TfCleaning", "grpCleanPara");
    EL<TGroupBox>("TfCleaning", "grpContactForce");
    EL<TGroupBox>("TfCleaning", "grpContactPara");
    EL<TGroupBox>("TfCleaning", "grpDieForce");
    EL<TGroupBox>("TfCleaning", "grpHeight");
    EL<TGroupBox>("TfCleaning", "grpSpeed");
    EL<TGroupBox>("TfCleaning", "grpTrayData");
    EL<TPanel>("TfCleaning", "palBtn");
    EL<TPageControl>("TfCleaning", "pcCleanYield");
    EL<TPageControl>("TfCleaning", "pgCleanType");
    EL<TPanel>("TfCleaning", "pnlCenter");
    EL<TPanel>("TfCleaning", "pnlCleanArm");
    EL<TPanel>("TfCleaning", "pnlCleanPadAlign");
    EL<TPanel>("TfCleaning", "pnlContactMode");
    EL<TPanel>("TfCleaning", "pnlInArmOffset");
    EL<TPanel>("TfCleaning", "pnlKitSetting");
    EL<TPanel>("TfCleaning", "pnlLeft");
    EL<TTabSheet>("TfCleaning", "tabTray");
    EL<TTabSheet>("TfCleaning", "tsFailure");
    EL<TTabSheet>("TfCleaning", "tsKit");
    EL<TTabSheet>("TfCleaning", "tsOffset");
    EL<TTabSheet>("TfCleaning", "tsSmart");
    EL<TTabSheet>("TfCleaning", "tsTimeCT");
    EL<TTabSheet>("TfCleaning", "tsYield");
    filerw::ELSetParents("TfCleaning", kCL_ParentOf, (int)(sizeof(kCL_ParentOf) / sizeof(kCL_ParentOf[0])));
}

// golden AutoClean/uCleaning.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void CL_DfmState()
{
    EL<TEdit>("TfCleaning", "PageTypeName")->Visible = false;
    EL<TComboBox>("TfCleaning", "cbbSelectTray")->Enabled = false;
    EL<TComboBox>("TfCleaning", "cbbSelectTray")->Visible = false;
    EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Visible = false;
    filerw::ELSetReadOnly("TfCleaning", "edAlarmCount");
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Enabled = false;
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_N")->Enabled = false;
    EL<TEdit>("TfCleaning", "edCleaningCount")->Enabled = false;
    filerw::ELSetReadOnly("TfCleaning", "edCleaningCount");
    filerw::ELSetReadOnly("TfCleaning", "edDevicePices");
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Enabled = false;
    filerw::ELSetReadOnly("TfCleaning", "edIndexArmAutoCleanCnt");
    EL<TGroupBox>("TfCleaning", "gbCleanKitOffset")->Visible = false;
    EL<TPanel>("TfCleaning", "pnlInArmOffset")->Visible = false;
    EL<TPanel>("TfCleaning", "pnlNotify")->Visible = false;
    EL<TRadioGroup>("TfCleaning", "rgKitPosition")->Enabled = false;
    EL<TComboBox>("TfCleaning", "ContactMode")->Text = "Direct Contact Mode";
    EL<TEdit>("TfCleaning", "IndexArmSpeed")->Text = "10";
    EL<TEdit>("TfCleaning", "OutArmSpeed")->Text = "10";
    EL<TEdit>("TfCleaning", "PageTypeName")->Text = "XST1";
    EL<TEdit>("TfCleaning", "ShuttleSpeed")->Text = "10";
    EL<TComboBox>("TfCleaning", "cbbACSmart_ContactMode")->Text = "Direct Contact Mode";
    EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text = "12";
    EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->ItemIndex = 0;
    EL<TComboBox>("TfCleaning", "cbbSelectTray")->Text = "Select from Database...";
    EL<TEdit>("TfCleaning", "edACContactCleanHeight")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edACContactCount")->Text = "3";
    EL<TEdit>("TfCleaning", "edACContactShiftHeight")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edACSmartCTF")->Text = "0";
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalAdj")->Text = "30";
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalMax")->Text = "1000";
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalMin")->Text = "30";
    EL<TEdit>("TfCleaning", "edAlarmCount")->Text = "0";
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Text = "0";
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_N")->Text = "0";
    EL<TEdit>("TfCleaning", "edCleanPadDeviation")->Text = "0";
    EL<TEdit>("TfCleaning", "edCleaningCount")->Text = "0";
    EL<TEdit>("TfCleaning", "edContactTime")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edDevicePices")->Text = "0";
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Text = "0";
    EL<TEdit>("TfCleaning", "edIndexPickOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edIndexReleaseOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edIntervalContact")->Text = "20";
    EL<TEdit>("TfCleaning", "edPinSingleGf")->Text = "0";
    EL<TEdit>("TfCleaning", "edPinSingleN")->Text = "0";
    EL<TEdit>("TfCleaning", "edPinsCount")->Text = "0";
    EL<TEdit>("TfCleaning", "edShuttle1PickOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edShuttle2PickOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edTimeCT")->Text = "10";
    EL<TEdit>("TfCleaning", "edtACSmart")->Text = "0";
    EL<TEdit>("TfCleaning", "edtACSmart_ACContactCount")->Text = "3";
    EL<TEdit>("TfCleaning", "edtACSmart_ContactTime")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtAutoCleanDieForce")->Text = "0";
    EL<TEdit>("TfCleaning", "edtBufferKitLDX")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitLDY")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitLTX")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitLTY")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitRDX")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitRDY")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitRTX")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtBufferKitRTY")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Normal")->Text = "100";
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Retest")->Text = "100";
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Normal")->Text = "100";
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Retest")->Text = "100";
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYield")->Text = "0";
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYieldDifferent")->Text = "100";
    EL<TEdit>("TfCleaning", "edtHotplatePickOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtHotplatePitchOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtHotplatePlaceOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtHotplateXOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtHotplateYOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtInArmAirOn")->Text = "10";
    EL<TEdit>("TfCleaning", "edtInArmVacuum")->Text = "10";
    EL<TEdit>("TfCleaning", "edtInArmZSpeed")->Text = "10";
    EL<TEdit>("TfCleaning", "edtIndexAirOn")->Text = "10";
    EL<TEdit>("TfCleaning", "edtIndexVacuum")->Text = "10";
    EL<TEdit>("TfCleaning", "edtLowYieldCount")->Text = "100";
    EL<TEdit>("TfCleaning", "edtLowYieldLimit")->Text = "0";
    EL<TEdit>("TfCleaning", "edtShuttle1PlaceOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttle1XOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttle1YOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttle2PlaceOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttle2XOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttle2YOffset")->Text = "0.50";
    EL<TEdit>("TfCleaning", "edtShuttlePitch")->Text = "0.50";
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex = 0;
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex = 0;
    EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex = 0;
    EL<TRadioGroup>("TfCleaning", "rgKitPosition")->ItemIndex = 2;
    EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->ItemIndex = 0;
    EL<TEdit>("TfCleaning", "XPitch1")->Tag = 1;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
    EL<TEdit>("TfCleaning", "XPitch2")->Tag = 1;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
    EL<TSpeedButton>("TfCleaning", "sbTrayAssign")->Tag = 1;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->DfmInit(0, 100, 0);
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Associate = EL<TEdit>("TfCleaning", "edDevicePices");
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kCL_EnableAll[] = {
    "Label21", "Label288", "Panel7", "btFocusOnly", "btInclude", "btnResetCleanCount",
    "cbConseFailureByHead_Normal", "cbConseFailureByHead_Retest", "cbConseFailureBySocket_Normal", "cbConseFailureBySocket_Retest", "cbFailAlarmLowYield", "cbFailAlarmSiteYieldDifferent",
    "chkACSmart", "chkAutoCleanMode1", "chkAutoCleanMode2", "chkAutoCleanMode3", "chkAutoCleanMode4", "chkAutoCleanMode5",
    "chkAutoCleanMode6", "chkCleanOtherArm", "chkE43", "chkTimeCT", "chkUseNSKit", "gbCleanKitOffset",
    "gbConsFailureHead", "gbConsFailureSocket", "gbYieldAlarm", "grpBufferKitLD", "grpBufferKitLT", "grpBufferKitRD",
    "grpBufferKitRT", "grpCleanDevice", "grpCleanMode", "grpCleanPara", "grpContactForce", "grpContactPara",
    "grpDieForce", "grpHeight", "grpSpeed", "grpTrayData", "lblInArm", "lblInArmZ",
    "lbledIndexArmAutoCleanCnt", "palBtn", "pcCleanYield", "pgCleanType", "pnlCenter", "pnlCleanArm",
    "pnlCleanPadAlign", "pnlContactMode", "pnlInArmOffset", "pnlKitSetting", "pnlLeft", "pnlNotify",
    "rgAutoCleanOnOff", "rgAutoCleanSelectArm", "rgCleanKitType", "rgKitPosition", "rgShuttleCheck", "sbCleanExit",
    "sbCleanSave", "sbTrayAssign", "tabTray", "tsFailure", "tsKit", "tsOffset",
    "tsSmart", "tsTimeCT", "tsYield",
};

// 存檔流程（sbCleanSaveClick、SaveAutoCleanData、sbCleanExitClick、FormClose）讀的替身 104 個 —— FileRW/TestIF_File_Cleaning.cpp 用它決定頁面必須送哪些值
static const char* const kCL_SaveReads[] = {
    "ContactMode", "IndexArmSpeed", "OutArmSpeed", "PageTypeName", "ShuttleSpeed", "XCT1",
    "XCT2", "XPitch1", "XPitch2", "XST1", "XST2", "YCT1",
    "YCT2", "YPitch1", "YPitch2", "YST1", "YST2", "cbConseFailureByHead_Normal",
    "cbConseFailureByHead_Retest", "cbConseFailureBySocket_Normal", "cbConseFailureBySocket_Retest", "cbFailAlarmLowYield", "cbFailAlarmSiteYieldDifferent", "cbbACSmart_ContactMode",
    "cbbCleanPadCount", "chkACSmart", "chkAutoCleanMode1", "chkAutoCleanMode2", "chkAutoCleanMode3", "chkAutoCleanMode4",
    "chkAutoCleanMode5", "chkAutoCleanMode6", "chkCleanOtherArm", "chkE43", "chkTimeCT", "chkUseNSKit",
    "edACContactCleanHeight", "edACContactCount", "edACContactShiftHeight", "edACSmartCTF", "edAdaptiveIntervalAdj", "edAdaptiveIntervalMax",
    "edAdaptiveIntervalMin", "edAlarmCount", "edAutoCleanAirForce_Kg", "edCleanPadDeviation", "edCleaningCount", "edContactTime",
    "edDevicePices", "edDropOffset1", "edIndexArmAutoCleanCnt", "edIndexPickOffset", "edIndexReleaseOffset", "edIntervalContact",
    "edPinSingleGf", "edPinSingleN", "edPinsCount", "edShuttle1PickOffset", "edShuttle2PickOffset", "edTimeCT",
    "edtACSmart", "edtACSmart_ACContactCount", "edtACSmart_ContactTime", "edtACSmart_DropOffset1", "edtAutoCleanDieForce", "edtBufferKitLDX",
    "edtBufferKitLDY", "edtBufferKitLTX", "edtBufferKitLTY", "edtBufferKitRDX", "edtBufferKitRDY", "edtBufferKitRTX",
    "edtBufferKitRTY", "edtConseFailureCountByHead_Normal", "edtConseFailureCountByHead_Retest", "edtConseFailureCountBySocket_Normal", "edtConseFailureCountBySocket_Retest", "edtFailAlarmSiteYield",
    "edtFailAlarmSiteYieldDifferent", "edtHotplatePickOffset", "edtHotplatePitchOffset", "edtHotplatePlaceOffset", "edtHotplateXOffset", "edtHotplateYOffset",
    "edtInArmAirOn", "edtInArmVacuum", "edtInArmZSpeed", "edtIndexAirOn", "edtIndexVacuum", "edtLowYieldCount",
    "edtLowYieldLimit", "edtShuttle1PlaceOffset", "edtShuttle1XOffset", "edtShuttle1YOffset", "edtShuttle2PlaceOffset", "edtShuttle2XOffset",
    "edtShuttle2YOffset", "edtShuttlePitch", "rgAutoCleanOnOff", "rgAutoCleanSelectArm", "rgCleanKitType", "rgKitPosition",
    "rgShuttleCheck", "udDeviceCT",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void CL_CreateSaveProxies()
{
    EL<TComboBox>("TfCleaning", "ContactMode");
    EL<TEdit>("TfCleaning", "IndexArmSpeed");
    EL<TEdit>("TfCleaning", "OutArmSpeed");
    EL<TEdit>("TfCleaning", "PageTypeName");
    EL<TEdit>("TfCleaning", "ShuttleSpeed");
    EL<TEdit>("TfCleaning", "XCT1");
    EL<TEdit>("TfCleaning", "XCT2");
    EL<TEdit>("TfCleaning", "XPitch1");
    EL<TEdit>("TfCleaning", "XPitch2");
    EL<TEdit>("TfCleaning", "XST1");
    EL<TEdit>("TfCleaning", "XST2");
    EL<TEdit>("TfCleaning", "YCT1");
    EL<TEdit>("TfCleaning", "YCT2");
    EL<TEdit>("TfCleaning", "YPitch1");
    EL<TEdit>("TfCleaning", "YPitch2");
    EL<TEdit>("TfCleaning", "YST1");
    EL<TEdit>("TfCleaning", "YST2");
    EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Normal");
    EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Retest");
    EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Normal");
    EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Retest");
    EL<TCheckBox>("TfCleaning", "cbFailAlarmLowYield");
    EL<TCheckBox>("TfCleaning", "cbFailAlarmSiteYieldDifferent");
    EL<TComboBox>("TfCleaning", "cbbACSmart_ContactMode");
    EL<TComboBox>("TfCleaning", "cbbCleanPadCount");
    EL<TCheckBox>("TfCleaning", "chkACSmart");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode1");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode4");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5");
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode6");
    EL<TCheckBox>("TfCleaning", "chkCleanOtherArm");
    EL<TCheckBox>("TfCleaning", "chkE43");
    EL<TCheckBox>("TfCleaning", "chkTimeCT");
    EL<TCheckBox>("TfCleaning", "chkUseNSKit");
    EL<TEdit>("TfCleaning", "edACContactCleanHeight");
    EL<TEdit>("TfCleaning", "edACContactCount");
    EL<TEdit>("TfCleaning", "edACContactShiftHeight");
    EL<TEdit>("TfCleaning", "edACSmartCTF");
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalAdj");
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalMax");
    EL<TEdit>("TfCleaning", "edAdaptiveIntervalMin");
    EL<TEdit>("TfCleaning", "edAlarmCount");
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg");
    EL<TEdit>("TfCleaning", "edCleanPadDeviation");
    EL<TEdit>("TfCleaning", "edCleaningCount");
    EL<TEdit>("TfCleaning", "edContactTime");
    EL<TEdit>("TfCleaning", "edDevicePices");
    EL<TEdit>("TfCleaning", "edDropOffset1");
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt");
    EL<TEdit>("TfCleaning", "edIndexPickOffset");
    EL<TEdit>("TfCleaning", "edIndexReleaseOffset");
    EL<TEdit>("TfCleaning", "edIntervalContact");
    EL<TEdit>("TfCleaning", "edPinSingleGf");
    EL<TEdit>("TfCleaning", "edPinSingleN");
    EL<TEdit>("TfCleaning", "edPinsCount");
    EL<TEdit>("TfCleaning", "edShuttle1PickOffset");
    EL<TEdit>("TfCleaning", "edShuttle2PickOffset");
    EL<TEdit>("TfCleaning", "edTimeCT");
    EL<TEdit>("TfCleaning", "edtACSmart");
    EL<TEdit>("TfCleaning", "edtACSmart_ACContactCount");
    EL<TEdit>("TfCleaning", "edtACSmart_ContactTime");
    EL<TEdit>("TfCleaning", "edtACSmart_DropOffset1");
    EL<TEdit>("TfCleaning", "edtAutoCleanDieForce");
    EL<TEdit>("TfCleaning", "edtBufferKitLDX");
    EL<TEdit>("TfCleaning", "edtBufferKitLDY");
    EL<TEdit>("TfCleaning", "edtBufferKitLTX");
    EL<TEdit>("TfCleaning", "edtBufferKitLTY");
    EL<TEdit>("TfCleaning", "edtBufferKitRDX");
    EL<TEdit>("TfCleaning", "edtBufferKitRDY");
    EL<TEdit>("TfCleaning", "edtBufferKitRTX");
    EL<TEdit>("TfCleaning", "edtBufferKitRTY");
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Normal");
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Retest");
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Normal");
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Retest");
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYield");
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYieldDifferent");
    EL<TEdit>("TfCleaning", "edtHotplatePickOffset");
    EL<TEdit>("TfCleaning", "edtHotplatePitchOffset");
    EL<TEdit>("TfCleaning", "edtHotplatePlaceOffset");
    EL<TEdit>("TfCleaning", "edtHotplateXOffset");
    EL<TEdit>("TfCleaning", "edtHotplateYOffset");
    EL<TEdit>("TfCleaning", "edtInArmAirOn");
    EL<TEdit>("TfCleaning", "edtInArmVacuum");
    EL<TEdit>("TfCleaning", "edtInArmZSpeed");
    EL<TEdit>("TfCleaning", "edtIndexAirOn");
    EL<TEdit>("TfCleaning", "edtIndexVacuum");
    EL<TEdit>("TfCleaning", "edtLowYieldCount");
    EL<TEdit>("TfCleaning", "edtLowYieldLimit");
    EL<TEdit>("TfCleaning", "edtShuttle1PlaceOffset");
    EL<TEdit>("TfCleaning", "edtShuttle1XOffset");
    EL<TEdit>("TfCleaning", "edtShuttle1YOffset");
    EL<TEdit>("TfCleaning", "edtShuttle2PlaceOffset");
    EL<TEdit>("TfCleaning", "edtShuttle2XOffset");
    EL<TEdit>("TfCleaning", "edtShuttle2YOffset");
    EL<TEdit>("TfCleaning", "edtShuttlePitch");
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff");
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm");
    EL<TRadioGroup>("TfCleaning", "rgCleanKitType");
    EL<TRadioGroup>("TfCleaning", "rgKitPosition");
    EL<TRadioGroup>("TfCleaning", "rgShuttleCheck");
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT");
}

// golden AutoClean/uCleaning.cpp:44  TfCleaning::TfCleaning(TComponent* Owner)
static void CL_TfCleaning()
{

    ;   // golden AutoClean/uCleaning.cpp:47-47 取代：(6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:47-47 -- (6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
    EL<TEdit>("TfCleaning", "edIntervalContact")->Hint="AutoClean_Cleaning Mode Interval Contact Count";   //Sam 20201209 : Default Recipe ChangeLog
#endif // GATE (S12-C save)
    ;   // golden AutoClean/uCleaning.cpp:48-48 取代：(6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:48-48 -- (6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Hint="AutoClean_Cleaning Mode Socket Alarm Count";       //Sam 20201209 : Default Recipe ChangeLog
#endif // GATE (S12-C save)
    bResetCleanCount=false;
    fShowBinSelect->btnCleanReset->Enabled=false;
    CL_LoadAutoCleanData();                                                        //Steven 20211129 : Add for Auto Clean初始化的資料是錯的
    iDeviceCount=0;

}

// golden AutoClean/uCleaning.cpp:55  TfCleaning::SetArmCaption()
static void CL_SetArmCaption()
{

    EL<TLabel>("TfCleaning", "lblInArm")->Caption="Input Arm ";
    EL<TLabel>("TfCleaning", "lblInArmZ")->Caption="Input Arm Z";
    if(CUSTOMER_CODE==CC_Greatek)
    {
    ;   // golden AutoClean/uCleaning.cpp:61-61 取代：(6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:61-61 -- (6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）
        EL<TEdit>("TfCleaning", "OutArmSpeed")->Hint="AutoClean_InArm Speed";                              //Sam 20210412 : 修正顯示
#endif // GATE (S12-C save)
    }

}

// golden AutoClean/uCleaning.cpp:65  TfCleaning::LoadAutoCleanData()
static void CL_LoadAutoCleanData()
{

    static bool bAutoCleanOnOff=false;
    static bool bFirstIn=true;
    bool bNeedReset=false;
    bool bFix3HasBin=false;
    AnsiString S="", Str;
    S=GetLastOpenFN();
    AnsiString szDir="", szDir1="";
    int iTempCnt=0;

    if(bFirstIn==true)
    {
        bFirstIn=false;
        fSpeed->ReadFile();
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN ||
       CosFunction.bUseDefineAutoCleanOffset==true)                             //wei 20160219 Auto Clean by機台存檔   //Ifor 20181106 add 新增Auto Clean Offset By 本機
    {
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean");
        MyForceDirectories(szDir);
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoClean.Data");
        szDir1.sprintf("%s%s", DataPath, S);
        szDir1+="\\HandlerCondition.Data";
        if(!FileExists(szDir))
        {
            CopyFile(szDir1.c_str(),szDir.c_str(),true);
        }

        if(CUSTOMER_CODE!=CC_TSMC_TAINAN)
        {
            szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
        }
    }
    else
    {
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    }

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {
        IniConfig.bE43AutoCleanUseHotplate    =ReadIniData(szDir, "Configuration", "bAutoCleanUseHotplate",      false);
        EL<TCheckBox>("TfCleaning", "chkE43")->Checked=IniConfig.bE43AutoCleanUseHotplate;
        EL<TTabSheet>("TfCleaning", "tsKit")->Caption=IniConfig.bE43AutoCleanUseHotplate?"Hot Plate":"Kit";
        SetAutoCleanTrayPosition();
    }

    TestIF_File.iAutoClean_Function           =ReadIniData(szDir, "Configuration", "iAutoClean_Function",            0);
    TestIF_File.iAutoClean_Mode               =ReadIniData(szDir, "Configuration", "iAutoClean_Mode",       M_INTERVAL);
    TestIF_File.iAutoClean_Tray               =ReadIniData(szDir, "Configuration", "iAutoClean_Tray",  eCKPos_CleanKit);        //Steven 20120217 : 選擇把CleanPad放在哪邊
    iTempCnt                                  =ReadIniData(szDir, "Configuration", "iIndexArmAutoCleanCnt",  0);                //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    if(iTempCnt>=TestIF_File.iIndexArmAutoCleanCnt)
        TestIF_File.iIndexArmAutoCleanCnt=iTempCnt;
    if(TestIF_File.iAutoClean_Mode==0)                                          //Ifor 20180823 (Steven) : Add Auto Clean 功能選項無開啟就不使用
    {
        TestIF_File.iAutoClean_Function=false;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex=0;                                          //Ifor 20180730 (Steven) : add Auto Clean 未選取功能強制關閉
        WriteIniData(szDir, "Configuration", "iAutoClean_Function", TestIF_File.iAutoClean_Function);
    }

    if(CLEAN_AIR)
    {
        TestIF_File.iAutoClean_Tray=eCKPos_CleanAir;
    }
    else
    {
        if(bUse_NewAutoCleanForm==1)                                            //kevin 20190620
            TestIF_File.iAutoClean_Tray=eCKPos_CleanKit;
        else if(TestIF_File.iAutoClean_Tray==eCKPos_Fix3)
            TestIF_File.iAutoClean_Tray=eCKPos_CleanKit;
    }

    TestIF_File.iAutoClean_AlarmCount           =ReadIniData(szDir, "Configuration", "iAutoClean_AlarmCount",         10);
    TestIF_File.iPadThickness                   =ReadIniData(szDir, "Configuration", "iAutoClean_iPadThickness",      10);//kevin 20180630 add clean pad - device
    TestIF_File.iAutoClean_MotorSpeed[0]        =ReadIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[0]",      10);
    TestIF_File.iAutoClean_MotorSpeed[1]        =ReadIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[1]",      10);
    TestIF_File.iAutoClean_MotorSpeed[2]        =ReadIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[2]",      10);
    TestIF_File.iAutoClean_MotorSpeed[3]        =ReadIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[3]",      10);
    TestIF_File.iAutoClean_ContactMode          =ReadIniData(szDir, "Configuration", "iAutoClean_ContactMode",         0);
    TestIF_File.iAutoCleanDropHigh              =ReadIniData(szDir, "Configuration", "iAutoClean_DropHigh",            0); //kevin 20180717 autoClean drop high
    TestIF_File.iAutoClean_ContactTime          =ReadIniData(szDir, "Configuration", "iAutoClean_ContactTime",         5);
    TestIF_File.iAutoClean_ContactCount         =ReadIniData(szDir, "Configuration", "iAutoClean_ContactCount",        3);

    TestIF_File.dAutoClean_InArmVacuum          =ReadIniData(szDir, "AutoClean", "InArmVacuum",            1.00);         //Steven 20250319 : Auto Clean使用另外一組Delay Time //AI(mg899to912) 20260909: 使用者 20260909 裁決改回 1.00（CASE-20260423-001）。recipe 沒有 [AutoClean] 區段時，ArmSpeed 推導會重現 Greatek 抱怨的 60 秒
    TestIF_File.dAutoClean_InArmAirOn           =ReadIniData(szDir, "AutoClean", "InArmAirOn",             1.00); //AI(mg899to912) 20260909: 使用者 20260909 裁決改回 1.00（CASE-20260423-001）。recipe 沒有 [AutoClean] 區段時，ArmSpeed 推導會重現 Greatek 抱怨的 60 秒
    TestIF_File.dAutoClean_IndexVacuum          =ReadIniData(szDir, "AutoClean", "IndexVacuum",            1.00); //AI(mg899to912) 20260909: 使用者 20260909 裁決改回 1.00（CASE-20260423-001）。recipe 沒有 [AutoClean] 區段時，ArmSpeed 推導會重現 Greatek 抱怨的 60 秒
    TestIF_File.dAutoClean_IndexAirOn           =ReadIniData(szDir, "AutoClean", "IndexAirOn",             1.00); //AI(mg899to912) 20260909: 使用者 20260909 裁決改回 1.00（CASE-20260423-001）。recipe 沒有 [AutoClean] 區段時，ArmSpeed 推導會重現 Greatek 抱怨的 60 秒

    if(CosFunction.bSmartAutoClean)                                             //Sam 20230111 : Smart Auto Clean
    {
        TestIF_File.bACSmart                    =ReadIniData(szDir, "Configuration", "bACSmart",                    false);
        TestIF_File.iACSmart_Count              =ReadIniData(szDir, "Configuration", "iACSmart_Count",              2);
        TestIF_File.iACSmart_Count_CTF          =ReadIniData(szDir, "Configuration", "iACSmart_Count CTF",          1); //Sam 20240726 : AI Clean
        TestIF_File.iACSmart_ContactMode        =ReadIniData(szDir, "Configuration", "iACSmart_ContactMode",        0);
        TestIF_File.iACSmart_DropHigh           =ReadIniData(szDir, "Configuration", "iACSmart_DropHigh",           0);
        TestIF_File.iACSmart_ContactTime        =ReadIniData(szDir, "Configuration", "iACSmart_ContactTime",        5);
        TestIF_File.iACSmart_ContactCount       =ReadIniData(szDir, "Configuration", "iACSmart_ContactCount",       3);

        TestIF_File.iAdaptiveACIntervalMax      =ReadIniData(szDir, "Configuration", "iAdaptiveACIntervalMax",       100);
        TestIF_File.iAdaptiveACIntervalMin      =ReadIniData(szDir, "Configuration", "iAdaptiveACIntervalMin",      5);
        TestIF_File.iAdaptiveACIntervalAdj      =ReadIniData(szDir, "Configuration", "iAdaptiveACIntervalAdj",      1);
    }

    if(CosFunction.bSetContactForceToAutoClean==true)                           //Steven 20250915 : 複製Contact Force給Auto Clean用
    {
        TestIF_File.iAutoClean_DevicePinCount   =DeviceForm_File.iPinCT;
        TestIF_File.fAutoClean_ForcePerPin      =DeviceForm_File.ForcePerPinN;
        TestIF_File.fAutoClean_DevicePinForceGf =DeviceForm_File.ForcePerPinG;
        TestIF_File.fAutoClean_AireForce        =DeviceForm_File.dPress;
        if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //AI(ht9045-v899) 20260430: extend AutoClean Die Force visibility to Multi EP half.
            TestIF_File.fAutoClean_DieForce     =DeviceForm_File.DoubleForce;
    }
    else
    {
        TestIF_File.iAutoClean_DevicePinCount   =ReadIniData(szDir, "Configuration", "iAutoClean_DevicePinCount",      0);
        TestIF_File.fAutoClean_ForcePerPin      =ReadIniData(szDir, "Configuration", "fAutoClean_ForcePerPin",         0.0);//kevin 20150826
        TestIF_File.fAutoClean_DevicePinForceGf =ReadIniData(szDir, "Configuration", "fAutoClean_DevicePinForceGf",    0.0);
        TestIF_File.fAutoClean_AireForce        =ReadIniData(szDir, "Configuration", "fAutoClean_AireForce",           0.0);


        if(CUSTOMER_CODE==CC_ASE_CL)    //JerryYang 20260723 : add
        {
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //AI(ht9045-v899) 20260430: extend AutoClean Die Force visibility to Multi EP half.
                TestIF_File.fAutoClean_DieForce     =DeviceForm_File.DoubleForce;
        }
        else
        {
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //AI(ht9045-v899) 20260430: extend AutoClean Die Force visibility to Multi EP half.
                TestIF_File.fAutoClean_DieForce     =ReadIniData(szDir, "Configuration", "fAutoClean_DieForce",            DeviceForm_File.DoubleForce);//Steven 20240719 : Die force for auto clean
        }
    }

    TestIF_File.iAutoClean_ContactShiftHeight   =ReadIniData(szDir, "Configuration", "iAutoClean_ContactShiftHeight",  1000);
    TestIF_File.iAutoClean_ContactCleanHeight   =ReadIniData(szDir, "Configuration", "iAutoClean_ContactCleanHeight",  0);
    TestIF_File.iAutoClean_IndexPickOffset      =ReadIniData(szDir, "Configuration", "iAutoClean_IndexPickOffset",     0);    //wei 20150318 Auto clean Index Pick Offset
    TestIF_File.iAutoClean_IndexReleaseOffset   =ReadIniData(szDir, "Configuration", "iAutoClean_IndexReleaseOffset",  0);    //Jou 2015-08-22 Auto clean Index Release Offset
    if(IniConfig.bE48_ShuttleUse4Offset_Autoclean==false)                       //Steven 20241102 : fixed for Auto Clean offset
    {
        TestIF_File.iAutoClean_Shuttle1PickOffset =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1PickOffset",  -200);
        TestIF_File.iAutoClean_Shuttle1PlaceOffset=ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1PlaceOffset", 0);
        TestIF_File.iAutoClean_Shuttle1XOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1XOffset",     0);
        TestIF_File.iAutoClean_Shuttle1YOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1YOffset",     0);

        TestIF_File.iAutoClean_Shuttle2PickOffset =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2PickOffset",  -200); //ChungHung 20141103 add AutoClean Input Arm Shuttle2 Offset
        TestIF_File.iAutoClean_Shuttle2PlaceOffset=ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2PlaceOffset", 0);
        TestIF_File.iAutoClean_Shuttle2XOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2XOffset",     0);
        TestIF_File.iAutoClean_Shuttle2YOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2YOffset",     0);
    }
    else
    {
        TestIF_File.iAutoClean_Shuttle1PickOffset =0;
        TestIF_File.iAutoClean_Shuttle1PlaceOffset=0;
        TestIF_File.iAutoClean_Shuttle1XOffset    =0;
        TestIF_File.iAutoClean_Shuttle1YOffset    =0;

        TestIF_File.iAutoClean_Shuttle2PickOffset =0;
        TestIF_File.iAutoClean_Shuttle2PlaceOffset=0;
        TestIF_File.iAutoClean_Shuttle2XOffset    =0;
        TestIF_File.iAutoClean_Shuttle2YOffset    =0;
    }

    TestIF_File.dBufferKitLTX    =ReadIniData(szDir, "Configuration", "dBufferKitLTX",  0.0);
    TestIF_File.dBufferKitLTY    =ReadIniData(szDir, "Configuration", "dBufferKitLTY",  0.0);
    TestIF_File.dBufferKitLDX    =ReadIniData(szDir, "Configuration", "dBufferKitLDX",  0.0);
    TestIF_File.dBufferKitLDY    =ReadIniData(szDir, "Configuration", "dBufferKitLDY",  0.0);
    TestIF_File.dBufferKitRTX    =ReadIniData(szDir, "Configuration", "dBufferKitRTX",  0.0);
    TestIF_File.dBufferKitRTY    =ReadIniData(szDir, "Configuration", "dBufferKitRTY",  0.0);
    TestIF_File.dBufferKitRDX    =ReadIniData(szDir, "Configuration", "dBufferKitRDX",  0.0);
    TestIF_File.dBufferKitRDY    =ReadIniData(szDir, "Configuration", "dBufferKitRDY",  0.0);
    if(TestIF_File.iAutoClean_ContactShiftHeight<=0)
    {
        TestIF_File.iAutoClean_ContactShiftHeight=1000;
        WriteIniData(szDir, "Configuration", "iAutoClean_ContactShiftHeight",   TestIF_File.iAutoClean_ContactShiftHeight);
    }

    int iAutoCleanOffset=0;

    if(IniConfig.bE43AutoCleanUseHotplate)                                      //ChungHung 20131120 AutoClean use Hotplate1 start
    {
        TestIF_File.iAutoClean_YDivision_Kit     =ReadIniData(szDir, "Configuration", "iAutoClean_YDivision_Kit",  2);
        TestIF_File.iAutoClean_YDivision_Tray    =ReadIniData(szDir, "Configuration", "iAutoClean_YDivision_Tray", 2);

        S=GetLastOpenFN();
        szDir="";
        szDir.sprintf("%s%s", DataPath, S);
        szDir+="\\HotPlate.Data";

        TestIF_File.dAutoClean_XPitch_Kit=iUnitMultiply100(ReadIniData(szDir, "Hotplate Form", "X Pitch",    22.00));
        TestIF_File.dAutoClean_YPitch_Kit=iUnitMultiply100(ReadIniData(szDir, "Hotplate Form", "Y Pitch",    33.00));
        TestIF_File.dAutoClean_XStart_Kit=iUnitMultiply100(ReadIniData(szDir, "Hotplate Form", "X Start",    33.00));
        TestIF_File.dAutoClean_YStart_Kit=iUnitMultiply100(ReadIniData(szDir, "Hotplate Form", "Y Start",    18.00));
        TestIF_File.iAutoClean_XDivision_Kit=ReadIniData(szDir, "Hotplate Form", "X Division", 4);
//        TestIF_File.iAutoClean_YDivision_Kit=ReadIniData(szDir, "Hotplate Form", "Y Division", 4);
        if(TestIF.iTestMode==DualSite &&
           TestIF_File.iAutoClean_XDivision_Kit==3)                             //JerryYang 20160321 強制修改HotPlate,避免Input Arm行程不足
        {
            TestIF_File.iAutoClean_XDivision_Kit=2;
        }

        if(TestIF_File.iAutoClean_XDivision_Kit>=6 &&
           TestIF_File.iTestMode!=_16Site2X8)                                   //Ztex 2025.04.09 Add HT-1132 Auto Clean
        {
            iAutoCleanOffset=TestIF_File.iAutoClean_XDivision_Kit-4;
            TestIF_File.iAutoClean_XDivision_Kit=4;
            TestIF_File.dAutoClean_XStart_Kit+=iAutoCleanOffset*TestIF_File.dAutoClean_XPitch_Kit;
        }

        S=GetLastOpenFN();
        szDir.sprintf("%s%s", DataPath, S);
        szDir+="\\Tray.Data";
        int iType=ReadIniData(szDir, "Loader",   "Tray Type",     0);
        S="Type"+AnsiString(iType);
        TestIF_File.dAutoClean_XStart_Tray       =iUnitMultiply100(ReadIniData(szDir, S, "X Start",      0.1));
        TestIF_File.dAutoClean_YStart_Tray       =iUnitMultiply100(ReadIniData(szDir, S, "Y Start",      0.1));
        TestIF_File.dAutoClean_XPitch_Tray       =iUnitMultiply100(ReadIniData(szDir, S, "X Pitch",      0.1));
        TestIF_File.dAutoClean_YPitch_Tray       =iUnitMultiply100(ReadIniData(szDir, S, "Y Pitch",      0.1));
        TestIF_File.iAutoClean_XDivision_Tray    =ReadIniData(szDir, S, "X Division",   14);

        if(TestIF_File.iAutoClean_XDivision_Tray<4)
        {
            iAutoCleanOffset=TestIF_File.iAutoClean_XDivision_Tray-(TestIF_File.iAutoClean_XDivision_Tray/2-(TestIF_File.iAutoClean_XDivision_Tray/2)/2)*2*2;
            TestIF_File.iAutoClean_XDivision_Tray=TestIF_File.iAutoClean_XDivision_Tray-iAutoCleanOffset;
            TestIF_File.dAutoClean_XStart_Tray+=iAutoCleanOffset*TestIF_File.dAutoClean_XPitch_Tray;
        }
        else
        {
            iAutoCleanOffset=TestIF_File.iAutoClean_XDivision_Tray-(TestIF_File.iAutoClean_XDivision_Tray/4-(TestIF_File.iAutoClean_XDivision_Tray/4)/2)*2*4;
            TestIF_File.iAutoClean_XDivision_Tray=TestIF_File.iAutoClean_XDivision_Tray-iAutoCleanOffset;
            TestIF_File.dAutoClean_XStart_Tray+=iAutoCleanOffset*TestIF_File.dAutoClean_XPitch_Tray;
        }
    }
    else
    {
        TestIF_File.dAutoClean_XPitch_Kit    =ReadIniData(szDir, "Configuration", "dAutoClean_XPitch_Kit",     2200.0);
        TestIF_File.dAutoClean_YPitch_Kit    =ReadIniData(szDir, "Configuration", "dAutoClean_YPitch_Kit",     2200.0);
        TestIF_File.dAutoClean_XStart_Kit    =ReadIniData(szDir, "Configuration", "dAutoClean_XStart_Kit",     3300.0);
        TestIF_File.dAutoClean_YStart_Kit    =ReadIniData(szDir, "Configuration", "dAutoClean_YStart_Kit",     1800.0);
        TestIF_File.iAutoClean_XDivision_Kit =ReadIniData(szDir, "Configuration", "iAutoClean_XDivision_Kit",  8);
        TestIF_File.iAutoClean_YDivision_Kit =ReadIniData(szDir, "Configuration", "iAutoClean_YDivision_Kit",  2);

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //wei 20150908
        {
            TestIF_File.dAutoClean_XPitch_Tray   =UserDefForm_File[0].XPitch*100;
            TestIF_File.dAutoClean_YPitch_Tray   =UserDefForm_File[0].YPitch*100;
            TestIF_File.dAutoClean_XStart_Tray   =UserDefForm_File[0].XStart*100;
            TestIF_File.dAutoClean_YStart_Tray   =UserDefForm_File[0].YStart*100;
        }
        else
        {
            TestIF_File.dAutoClean_XPitch_Tray   =ReadIniData(szDir, "Configuration", "dAutoClean_XPitch_Tray",    2200.0);
            TestIF_File.dAutoClean_YPitch_Tray   =ReadIniData(szDir, "Configuration", "dAutoClean_YPitch_Tray",    2200.0);
            TestIF_File.dAutoClean_XStart_Tray   =ReadIniData(szDir, "Configuration", "dAutoClean_XStart_Tray",    3300.0);
            TestIF_File.dAutoClean_YStart_Tray   =ReadIniData(szDir, "Configuration", "dAutoClean_YStart_Tray",    1800.0);
        }
        TestIF_File.iAutoClean_XDivision_Tray=ReadIniData(szDir, "Configuration", "iAutoClean_XDivision_Tray", 8);
        TestIF_File.iAutoClean_YDivision_Tray=ReadIniData(szDir, "Configuration", "iAutoClean_YDivision_Tray", 2);

        TestIF_File.cAutoClean_PackageTray=ReadIniData(szDir, "Configuration", "cAutoClean_PackageTray", AnsiString("BGA"));
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       CUSTOMER_CODE==CC_OSE ||                                                 //kevin 20180309 add OSE
       CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                      //kevin 20210929 add 938
    {
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Visible=false;
        EL<TPageControl>("TfCleaning", "pcCleanYield")->Visible=false;
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->Enabled=false;
    CL_RadioIndex(EL<TRadioGroup>("TfCleaning", "rgCleanKitType"), 1);   // golden AutoClean/uCleaning.cpp:335-335 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:335-335 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex=1;
#endif // GATE (S12-C save)
        EL<TButton>("TfCleaning", "btInclude")->Visible=false;
    }
    S=GetLastOpenFN();
    szDir="";

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160219 Auto Clean by機台存檔
    {
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoClean.Data");
    }
    else
    {
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    }

    CL_ChangeEditToHPMode();                                                       //Steven20160628 : 整合Auto Clean畫面顯示
    TestIF_File.iAutoClean_IntervalContact    =ReadIniData(szDir, "Configuration", "iAutoClean_IntervalContact",    20);
    TestIF_File.iAutoClean_DeveicePices       =ReadIniData(szDir, "Configuration", "iAutoClean_DeveicePices",       8);

    EL<TEdit>("TfCleaning", "XPitch1")->Text=ConvertTouMType(TestIF_File.dAutoClean_XPitch_Kit);           //Steven 20240826 : 往上移動
    EL<TEdit>("TfCleaning", "YPitch1")->Text=ConvertTouMType(TestIF_File.dAutoClean_YPitch_Kit);
    EL<TEdit>("TfCleaning", "XST1")->Text=ConvertTouMType(TestIF_File.dAutoClean_XStart_Kit);
    EL<TEdit>("TfCleaning", "YST1")->Text=ConvertTouMType(TestIF_File.dAutoClean_YStart_Kit);
    CL_EditText(EL<TEdit>("TfCleaning", "XCT1"), TestIF_File.iAutoClean_XDivision_Kit);   // golden AutoClean/uCleaning.cpp:358-358 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:358-358 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "XCT1")->Text=TestIF_File.iAutoClean_XDivision_Kit;
#endif // GATE (S12-C save)
    CL_EditText(EL<TEdit>("TfCleaning", "YCT1"), TestIF_File.iAutoClean_YDivision_Kit);   // golden AutoClean/uCleaning.cpp:359-359 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:359-359 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "YCT1")->Text=TestIF_File.iAutoClean_YDivision_Kit;
#endif // GATE (S12-C save)

    CL_EditText(EL<TEdit>("TfCleaning", "XPitch2"), ConvertTouMType(TestIF_File.dAutoClean_XPitch_Tray));   // golden AutoClean/uCleaning.cpp:361-361 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:361-361 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "XPitch2")->Text=ConvertTouMType(TestIF_File.dAutoClean_XPitch_Tray);
#endif // GATE (S12-C save)
    EL<TEdit>("TfCleaning", "YPitch2")->Text=ConvertTouMType(TestIF_File.dAutoClean_YPitch_Tray);
    EL<TEdit>("TfCleaning", "XST2")->Text=ConvertTouMType(TestIF_File.dAutoClean_XStart_Tray);
    EL<TEdit>("TfCleaning", "YST2")->Text=ConvertTouMType(TestIF_File.dAutoClean_YStart_Tray);

    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), TestIF_File.iAutoClean_XDivision_Tray);   // golden AutoClean/uCleaning.cpp:366-366 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:366-366 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "XCT2")->Text=TestIF_File.iAutoClean_XDivision_Tray;
#endif // GATE (S12-C save)
    if(atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str())!=TestIF_File.iAutoClean_XDivision_Tray)         //Steven 20181226 : 避免畫面顯示跟跑的數值不一樣
        TestIF_File.iAutoClean_XDivision_Tray=atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());

    CL_EditText(EL<TEdit>("TfCleaning", "YCT2"), TestIF_File.iAutoClean_YDivision_Tray);   // golden AutoClean/uCleaning.cpp:370-370 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:370-370 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "YCT2")->Text=TestIF_File.iAutoClean_YDivision_Tray;
#endif // GATE (S12-C save)
    EL<TEdit>("TfCleaning", "PageTypeName")->Text=TestIF_File.cAutoClean_PackageTray;                      //kevin 20150521

    int iCount=CL_GetMinCleanPadCount();                                           //Steven 20221006 : 計算最少需要的Clean Pad數量
    if(TestIF_File.iAutoClean_DeveicePices<iCount)
        TestIF_File.iAutoClean_DeveicePices=iCount;

    if(TestIF_File.iTestMode==TriSite1X3 &&                                     //JerryYang 20260408 : fix 1x3 2吸嘴
       iInArmType==e9045_1x3_2_14 &&
       TestIF_File.iSiteMap[0][1]<=0 &&
       TestIF_File.iAutoClean_DeveicePices==3 &&
       iCount==2)
    {
        TestIF_File.iAutoClean_DeveicePices=2;
    }

    if(IniConfig.bE43AutoCleanUseHotplate==false)                               //ChungHung 20131120 AutoClean use Hotplate1
    {
        if(TestIF_File.iAutoClean_DeveicePices>100)                             //QQ : 待確認
            TestIF_File.iAutoClean_DeveicePices=100;
    }
    iBackupCleaningDeveicePices         =TestIF_File.iAutoClean_DeveicePices;

    CL_SetDeviceMaxMin();                                                          //kevin 20221011 read data and set different
    if(TestIF_File.iTestMode==_12Site2X6)
    {
        filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), TestIF_File.iAutoClean_DeveicePices);   //Steven 20241113 : for 2x6 auto clean
    if(CL_VarGE(((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min), EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text))   // golden AutoClean/uCleaning.cpp:397-397 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:397-397 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        if(EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min>=EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text)                             //kevin 20221010 kevin 20221011 add clean pad load data
#endif // GATE (S12-C save)
        {
    filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:399-399 取代：(4) ELTrackBar 讀值 (int)＋產生器的 TComboBox 連動
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:399-399 -- (4) ELTrackBar 讀值 (int)＋產生器的 TComboBox 連動
            filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);
#endif // GATE (S12-C save)
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:400-400 取代：(4) ELTrackBar 讀值 (int)
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:400-400 -- (4) ELTrackBar 讀值 (int)
            EL<TEdit>("TfCleaning", "edDevicePices")->Text         =EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;
#endif // GATE (S12-C save)
        }
        TestIF_File.iAutoClean_DeveicePices   =atoi(EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text.c_str());
    }
    else
    {
        EL<TEdit>("TfCleaning", "edDevicePices")->Text             =TestIF_File.iAutoClean_DeveicePices;   //Steven 20221028 : 修正重開程式Auto Clean數量錯誤的問題
    if(CL_VarGE(((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min), EL<TEdit>("TfCleaning", "edDevicePices")->Text))   // golden AutoClean/uCleaning.cpp:407-407 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:407-407 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        if(EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min>=EL<TEdit>("TfCleaning", "edDevicePices")->Text)                                //kevin 20221010 kevin 20221011 add clean pad load data
#endif // GATE (S12-C save)
        {
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:409-409 取代：(4) ELTrackBar 讀值 (int)
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:409-409 -- (4) ELTrackBar 讀值 (int)
            EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;
#endif // GATE (S12-C save)
        }
        TestIF_File.iAutoClean_DeveicePices   =atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str());
    }

    if(InitialOK==false)                                                        //Steven 20180524 : Fixed for clean count, 系統還沒讀取檔案不可以做動作
    {
        if(TestIF_File.bAutoClean_UseTray)
        {
            iBackupCleaningItemX              =TestIF_File.iAutoClean_XDivision_Tray;
            iBackupCleaningItemY              =TestIF_File.iAutoClean_YDivision_Tray;
        }
        else
        {
            iBackupCleaningItemX              =TestIF_File.iAutoClean_XDivision_Kit;
            iBackupCleaningItemY              =TestIF_File.iAutoClean_YDivision_Kit;
        }
    }
    else
    {
        if(TestIF_File.bAutoClean_UseTray)
        {
            iBackupCleaningItemX              =TestIF_File.iAutoClean_XDivision_Tray;
            iBackupCleaningItemY              =TestIF_File.iAutoClean_YDivision_Tray;
            if(iBackupCleaningItemX!=TestIF.iAutoClean_XDivision_Tray || iBackupCleaningItemY!=TestIF.iAutoClean_YDivision_Tray)
                bNeedReset=true;
        }
        else
        {
            iBackupCleaningItemX              =TestIF_File.iAutoClean_XDivision_Kit;
            iBackupCleaningItemY              =TestIF_File.iAutoClean_YDivision_Kit;
            if(iBackupCleaningItemX!=TestIF.iAutoClean_XDivision_Kit || iBackupCleaningItemY!=TestIF.iAutoClean_YDivision_Kit)
                bNeedReset=true;
        }
    }

    TestIF_File.bCleanIndexOtherArm =ReadIniData(szDir, "Configuration", "iCleanIndexOtherArm", false);
    TestIF_File.iAutoClean_SelectArm=ReadIniData(szDir, "Configuration", "iAutoClean_SelectArm", 0);  //0:Arm1 1:Arm2 2:Arm1 & Arm2

    if((MachineTypeChoice==Type_HT9045 ||
        MachineTypeChoice==Type_HT9046 ||
        MachineTypeChoice==Type_HT9045_12Site) &&
            TestIF.iTestMode>=QualSite2X2  &&
            TestIF.iTestMode!=QualSite2X2N &&
            TestIF_File.dSiteYPitch>=65.0)                                      //Steven 20211227 : HT9045跑兩排的, Site Y pitch大於65只能用Arm 1
    {
        if(TestIF_File.iAutoClean_Function && TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1 && TestIF_File.iAutoClean_SelectArm==1)
        {
            filerw::ELMessage("The site Y-pitch can not use arm 2 for auto clean!!");
        }
        TestIF_File.iAutoClean_SelectArm=0;
    }
    else if(TestIF_File.iShuttleMode==1)                                        //JerryYang 20160309 切換為單Arm時 auto clean 也強制切換為單Arm模式
    {
        if(TestIF_File.iShuttle_Sel==0)
        {
            TestIF_File.iAutoClean_SelectArm=0;
        }
        else if(TestIF_File.iShuttle_Sel==1)
        {
            TestIF_File.iAutoClean_SelectArm=1;
        }
    }
    else
    {
        if(bUseTwoArm32Site==true)
        {
            TestIF_File.iAutoClean_SelectArm=2;                                 //強制用兩隻Arm
        }
    }

    TestIF_File.bAutoClean_FailAlarmLowYield                =ReadIniData(szDir, "Configuration","bAutoClean_FailAlarmLowYield", false);
    TestIF_File.iAutoClean_LowYieldLimit                    =ReadIniData(szDir, "Configuration", "iAutoClean_LowYieldLimit", 80);
    TestIF_File.iAutoClean_LowYieldCount                    =ReadIniData(szDir, "Configuration", "iAutoClean_LowYieldCount", 1000);

    TestIF_File.bAutoClean_FailAlarmSiteYieldDifferent      =ReadIniData(szDir, "Configuration", "bAutoClean_FailAlarmSiteYieldDifferent", false);
    TestIF_File.iAutoClean_FailAlarmSiteYield               =ReadIniData(szDir, "Configuration", "iAutoClean_FailAlarmSiteYield", 80);
    TestIF_File.iAutoClean_FailAlarmSiteYieldDifferentCount =ReadIniData(szDir, "Configuration", "iAutoClean_FailAlarmSiteYieldDifferentCount", 1000);

    TestIF_File.bAutoClean_ConseFailureBySocket_Normal      =ReadIniData(szDir, "Configuration", "bAutoClean_ConseFailureBySocket_Normal", false);
    TestIF_File.iAutoClean_ConseFailureCountBySocket_Normal =ReadIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountBySocket_Normal", 10);
    TestIF_File.bAutoClean_ConseFailureBySocket_Retest      =ReadIniData(szDir, "Configuration", "bAutoClean_ConseFailureBySocket_Retest", false);
    TestIF_File.iAutoClean_ConseFailureCountBySocket_Retest =ReadIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountBySocket_Retest", 10);
    TestIF_File.bAutoClean_ConseFailureByHead_Normal        =ReadIniData(szDir, "Configuration", "bAutoClean_ConseFailureByHead_Normal", false);
    TestIF_File.iAutoClean_ConseFailureCountByHead_Normal   =ReadIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountByHead_Normal", 10);
    TestIF_File.bAutoClean_ConseFailureByHead_Retest        =ReadIniData(szDir, "Configuration", "bAutoClean_ConseFailureByHead_Retest", false);
    TestIF_File.iAutoClean_ConseFailureCountByHead_Retest   =ReadIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountByHead_Retest", 10);

    if(CosFunction.bAutoCleanTimeCT==true)                                      //jou 20250102 : auto clean triger time count
    {
        TestIF_File.bEnabledAutoCleanTimeCT                 =ReadIniData(szDir, "Configuration", "bEnabledAutoCleanTimeCT", false);
        TestIF_File.iAutoCleanTimeCT                        =ReadIniData(szDir, "Configuration", "iAutoCleanTimeCT", 10);
    }
    else
    {
        TestIF_File.bEnabledAutoCleanTimeCT=false;
    }

    if(IniConfig.bE43AutoCleanUseHotplate)
        TestIF_File.bAutoClean_UseTray=false;
    else
        TestIF_File.bAutoClean_UseTray=ReadIniData(szDir, "Configuration", "bAutoClean_UseTray", false);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        TestIF_File.bAutoClean_UseTray=true;

    TestIF_File.bAutoClean_UseNSKit=ReadIniData(szDir, "Configuration", "bAutoClean_UseNSKit", false);   //jou 2013-08-08 SPIL use NS Clean kit

    if(TestIF_File.bAutoClean_UseTray)
    {
        TestIF_File.dAutoClean_XPitch=TestIF_File.dAutoClean_XPitch_Tray;
        TestIF_File.dAutoClean_YPitch=TestIF_File.dAutoClean_YPitch_Tray;
        TestIF_File.dAutoClean_XStart=TestIF_File.dAutoClean_XStart_Tray;
        TestIF_File.dAutoClean_YStart=TestIF_File.dAutoClean_YStart_Tray;
        TestIF_File.iAutoClean_XDivision=TestIF_File.iAutoClean_XDivision_Tray;
        TestIF_File.iAutoClean_YDivision=TestIF_File.iAutoClean_YDivision_Tray;
    }
    else
    {
        TestIF_File.dAutoClean_XPitch=TestIF_File.dAutoClean_XPitch_Kit;
        TestIF_File.dAutoClean_YPitch=TestIF_File.dAutoClean_YPitch_Kit;
        TestIF_File.dAutoClean_XStart=TestIF_File.dAutoClean_XStart_Kit;
        TestIF_File.dAutoClean_YStart=TestIF_File.dAutoClean_YStart_Kit;
        TestIF_File.iAutoClean_XDivision=TestIF_File.iAutoClean_XDivision_Kit;
        TestIF_File.iAutoClean_YDivision=TestIF_File.iAutoClean_YDivision_Kit;
    }

    iAutoCleanShuttle=ReadIniData(szDir, "Configuration", "iAutoCleanShuttle", 1); //kevin 20120710
    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       CUSTOMER_CODE==CC_OSE ||                                                 //kevin 20211123 add OSE
       CUSTOMER_CODE==CC_ASE_KaohSiung_K3  ||
       CUSTOMER_CODE==CC_ASE_KaohSiung_K11)                                     //Steven 20201210 : Fixed for auto clean offset
    {
        TestIF_File.HotplatlXOffset     =ReadIniData(szDir, "Configuration", "HotplatlXOffset",     0.0);
        TestIF_File.HotplatlYOffset     =ReadIniData(szDir, "Configuration", "HotplatlYOffset",     0.0);
        TestIF_File.HotplatlPickOffset  =ReadIniData(szDir, "Configuration", "HotplatlPickOffset",  0.0);
        TestIF_File.HotplatlPlaceOffset =ReadIniData(szDir, "Configuration", "HotplatlPlaceOffset", 0.0); //kevin 20150209 end add offset
        TestIF_File.HotplatlPitchOffset =ReadIniData(szDir, "Configuration", "HotplatlPitchOffset", 0.0); //kevin 20150526
        TestIF_File.ShuttlePitchOffset  =ReadIniData(szDir, "Configuration", "ShuttlePitchOffset",  0.0); //kevin 20150526
    }
    else
    {
        TestIF_File.HotplatlXOffset     =0.0;
        TestIF_File.HotplatlYOffset     =0.0;
        TestIF_File.HotplatlPickOffset  =0.0;
        TestIF_File.HotplatlPlaceOffset =0.0;
        TestIF_File.HotplatlPitchOffset =0.0;
        TestIF_File.ShuttlePitchOffset  =0.0;
    }

    fMain->AutoCleanStringGrid->ColCount=TestIF_File.iAutoClean_XDivision;      //Steven 20180524 : Fixed for clean count
    fMain->AutoCleanStringGrid->RowCount=(TestIF_File.iAutoClean_YDivision+1)*3;//ChungHung 20140317 alter 如果設為1 RowCount 會不夠 TestIF_File.iAutoClean_YDivision*3 --->  (TestIF_File.iAutoClean_YDivision+1)*3

    fMain->tmyAutoClean->XItem=TestIF_File.iAutoClean_XDivision;
    fMain->tmyAutoClean->YItem=TestIF_File.iAutoClean_YDivision;
    MOT[MMAutoCleanKit].Tray.SetXYItem(TestIF_File.iAutoClean_XDivision, TestIF_File.iAutoClean_YDivision);     //Steven 20160614 : 設定Tray XY Item改用function加上保護

    ReadWriteAutoCleanCount(true);

    if(InitialOK==true)
    {
        if(bAutoCleanOnOff!=TestIF_File.iAutoClean_Function)                    //Steven 20180828 : 修正生產到一半開Auto Clean無效
        {
            bNeedReset=true;
            bAutoCleanOnOff=TestIF_File.iAutoClean_Function;
        }

        if(iBackupCleaningDeveicePices!=TestIF.iAutoClean_DeveicePices || bNeedReset)
        {
            WriteIniData(szDir, "Configuration", "iAutoClean_DeveicePices", TestIF_File.iAutoClean_DeveicePices);
            if(bRunAutoClean==false)
            {
                RecordProcess("Auto clean : reset data because DevicePices was change");
                SetAutoCleanICCount(false);
                ReadWriteAutoCleanCount(false, true);                           //AI(ht9045-v899) 20260414: zero all pad counts when DevicePices or tray layout changes to prevent stale counts causing false WAR1922
            }
        }
    }
    else
    {
        SetAutoCleanICCount(true);
    }

    if(TestIF_File.iAutoClean_Tray==eCKPos_Fix3)
    {
        for(int i=0; i<15; i++)
        {
            if(Prod.iT6CatData[i]==eFix3 || Prod.iT6CatData[i]==eFix6)          //Steven 20120210 Start: Fix3有Bin不能做Auto Clean
                bFix3HasBin=true;
        }

        if(Prod.iIfErrorT6==eFix3 || Prod.iIfErrorT6==eFix6)
            bFix3HasBin=true;

        if(bFix3HasBin==true && TestIF_File.iAutoClean_Function==true)
        {
            TestIF_File.iAutoClean_Function=false;  //Steven 20260618 fix: == -> = (P11 bug fix)
            WriteIniData(szDir, "Configuration", "iAutoClean_Function",             TestIF_File.iAutoClean_Function);
        }
    }

    if(IniConfig.bEnableAutoCleanFunction==false)
    {
        TestIF_File.iAutoClean_Function=false;
        WriteIniData(szDir, "Configuration", "iAutoClean_Function",             TestIF_File.iAutoClean_Function);
    }

    if(CosFunction.bUseDefineAutoCleanOffset==true)                             //Ifor 20181106 add 新增Auto Clean Offset By 本機
    {
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoClean.Data");

        TestIF_File.iAutoClean_ContactShiftHeight =ReadIniData(szDir, "Configuration", "iAutoClean_ContactShiftHeight",  1000);
        TestIF_File.iAutoClean_ContactCleanHeight =ReadIniData(szDir, "Configuration", "iAutoClean_ContactCleanHeight",  0);
        TestIF_File.iAutoClean_IndexPickOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_IndexPickOffset",     0);    //wei 20150318 Auto clean Index Pick Offset
        TestIF_File.iAutoClean_IndexReleaseOffset =ReadIniData(szDir, "Configuration", "iAutoClean_IndexReleaseOffset",  0);    //Jou 2015-08-22 Auto clean Index Release Offset

        if(IniConfig.bE48_ShuttleUse4Offset_Autoclean==false)                   //Steven 20241102 : fixed for Auto Clean offset
        {
            TestIF_File.iAutoClean_Shuttle1PickOffset =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1PickOffset",  -200);
            TestIF_File.iAutoClean_Shuttle1PlaceOffset=ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1PlaceOffset", 0);
            TestIF_File.iAutoClean_Shuttle1XOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1XOffset",     0);
            TestIF_File.iAutoClean_Shuttle1YOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle1YOffset",     0);

            TestIF_File.iAutoClean_Shuttle2PickOffset =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2PickOffset",  -200);
            TestIF_File.iAutoClean_Shuttle2PlaceOffset=ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2PlaceOffset", 0);
            TestIF_File.iAutoClean_Shuttle2XOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2XOffset",     0);
            TestIF_File.iAutoClean_Shuttle2YOffset    =ReadIniData(szDir, "Configuration", "iAutoClean_Shuttle2YOffset",     0);
        }
        else
        {
            TestIF_File.iAutoClean_Shuttle1PickOffset =0;
            TestIF_File.iAutoClean_Shuttle1PlaceOffset=0;
            TestIF_File.iAutoClean_Shuttle1XOffset    =0;
            TestIF_File.iAutoClean_Shuttle1YOffset    =0;

            TestIF_File.iAutoClean_Shuttle2PickOffset =0;
            TestIF_File.iAutoClean_Shuttle2PlaceOffset=0;
            TestIF_File.iAutoClean_Shuttle2XOffset    =0;
            TestIF_File.iAutoClean_Shuttle2YOffset    =0;
        }

        TestIF_File.HotplatlXOffset     =ReadIniData(szDir, "Configuration", "HotplatlXOffset",     0.0);
        TestIF_File.HotplatlYOffset     =ReadIniData(szDir, "Configuration", "HotplatlYOffset",     0.0);
        TestIF_File.HotplatlPickOffset  =ReadIniData(szDir, "Configuration", "HotplatlPickOffset",  0.0);
        TestIF_File.HotplatlPlaceOffset =ReadIniData(szDir, "Configuration", "HotplatlPlaceOffset", 0.0); //kevin 20150209 end add offset
        TestIF_File.HotplatlPitchOffset =ReadIniData(szDir, "Configuration", "HotplatlPitchOffset", 0.0); //kevin 20150526
        TestIF_File.ShuttlePitchOffset  =ReadIniData(szDir, "Configuration", "ShuttlePitchOffset",  0.0); //kevin 20150526
    }
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex =TestIF_File.iAutoClean_Function;
    iBackupAutoClean_Function   =TestIF_File.iAutoClean_Function;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode1")->Checked  =TestIF_File.iAutoClean_Mode & M_INIT_START;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2")->Checked  =TestIF_File.iAutoClean_Mode & M_INIT_RESTART;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3")->Checked  =TestIF_File.iAutoClean_Mode & M_FINISH;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode4")->Checked  =TestIF_File.iAutoClean_Mode & M_MANUAL;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Checked  =TestIF_File.iAutoClean_Mode & M_SOCKET_ALARM;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode6")->Checked  =TestIF_File.iAutoClean_Mode & M_INTERVAL;
    EL<TEdit>("TfCleaning", "edIntervalContact")->Text     =TestIF_File.iAutoClean_IntervalContact;

    if(TestIF_File.iTestMode==_12Site2X6)
        filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), TestIF_File.iAutoClean_DeveicePices);           //Steven 20241113 : for 2x6 auto clean
    else
        EL<TEdit>("TfCleaning", "edDevicePices")->Text     =TestIF_File.iAutoClean_DeveicePices;
    iBackupCleaningDeveicePices =TestIF_File.iAutoClean_DeveicePices;
    EL<TEdit>("TfCleaning", "edAlarmCount")->Text          =TestIF_File.iAutoClean_AlarmCount;

    EL<TEdit>("TfCleaning", "OutArmSpeed")->Text           =TestIF_File.iAutoClean_MotorSpeed[0];
    EL<TEdit>("TfCleaning", "ShuttleSpeed")->Text          =TestIF_File.iAutoClean_MotorSpeed[1];
    EL<TEdit>("TfCleaning", "IndexArmSpeed")->Text         =TestIF_File.iAutoClean_MotorSpeed[2];
    EL<TEdit>("TfCleaning", "edtInArmZSpeed")->Text        =TestIF_File.iAutoClean_MotorSpeed[3];
    filerw::ELComboIndex(EL<TComboBox>("TfCleaning", "ContactMode"), TestIF_File.iAutoClean_ContactMode);
    EL<TEdit>("TfCleaning", "edtInArmVacuum")->Text        =FormatFloat("0.00", TestIF_File.dAutoClean_InArmVacuum);            //Steven 20250319 : Auto Clean使用另外一組Delay Time
    EL<TEdit>("TfCleaning", "edtInArmAirOn")->Text         =FormatFloat("0.00", TestIF_File.dAutoClean_InArmAirOn);
    EL<TEdit>("TfCleaning", "edtIndexVacuum")->Text        =FormatFloat("0.00", TestIF_File.dAutoClean_IndexVacuum);
    EL<TEdit>("TfCleaning", "edtIndexAirOn")->Text         =FormatFloat("0.00", TestIF_File.dAutoClean_IndexAirOn);

    EL<TEdit>("TfCleaning", "edDropOffset1")->Text         =ConvertTouMType(TestIF_File.iAutoCleanDropHigh);                           //kevin 20180717 autoClean drop high
    EL<TEdit>("TfCleaning", "edContactTime")->Text         =(double)TestIF_File.iAutoClean_ContactTime/10.0;
    EL<TEdit>("TfCleaning", "edACContactCount")->Text      =TestIF_File.iAutoClean_ContactCount;
    CL_RadioIndex(EL<TRadioGroup>("TfCleaning", "rgKitPosition"), TestIF_File.iAutoClean_Tray);   // golden AutoClean/uCleaning.cpp:686-686 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:686-686 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TRadioGroup>("TfCleaning", "rgKitPosition")->ItemIndex    =TestIF_File.iAutoClean_Tray;                   //0 :Fix3 1:hotplate 2 2:Clean kit 3:Clean Air      //Steven 20120217 : 選擇把CleanPad放在哪邊
#endif // GATE (S12-C save)
    if(CosFunction.bSmartAutoClean)                                             //Sam 20230111 : Smart Auto Clean
    {
        EL<TCheckBox>("TfCleaning", "chkACSmart")->Checked                 =TestIF_File.bACSmart;
        EL<TEdit>("TfCleaning", "edtACSmart")->Text                    =TestIF_File.iACSmart_Count;
        EL<TEdit>("TfCleaning", "edACSmartCTF")->Text                  =TestIF_File.iACSmart_Count_CTF;    //Sam 20240726 : AI Clean
        filerw::ELComboIndex(EL<TComboBox>("TfCleaning", "cbbACSmart_ContactMode"), TestIF_File.iACSmart_ContactMode);
        EL<TEdit>("TfCleaning", "edtACSmart_DropOffset1")->Text        =ConvertTouMType(TestIF_File.iACSmart_DropHigh);
        EL<TEdit>("TfCleaning", "edtACSmart_ContactTime")->Text        =(double)TestIF_File.iACSmart_ContactTime/10.0;
        EL<TEdit>("TfCleaning", "edtACSmart_ACContactCount")->Text     =TestIF_File.iACSmart_ContactCount;

        EL<TEdit>("TfCleaning", "edAdaptiveIntervalMax")->Text         =TestIF_File.iAdaptiveACIntervalMax;
        EL<TEdit>("TfCleaning", "edAdaptiveIntervalMin")->Text         =TestIF_File.iAdaptiveACIntervalMin;
        EL<TEdit>("TfCleaning", "edAdaptiveIntervalAdj")->Text         =TestIF_File.iAdaptiveACIntervalAdj;
    }
    CL_SetArmCaption();
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Text=TestIF_File.iIndexArmAutoCleanCnt;             //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    if(TestIF_File.iAutoClean_DevicePinCount==0)
        TestIF_File.iAutoClean_DevicePinCount=DeviceForm_File.iPinCT;
    EL<TEdit>("TfCleaning", "edPinsCount")->Text=TestIF_File.iAutoClean_DevicePinCount;

    EL<TEdit>("TfCleaning", "edPinSingleGf")->Text=TestIF_File.fAutoClean_DevicePinForceGf;

    if(TestIF_File.fAutoClean_ForcePerPin==0.0)                                 //kevin 20150826 add
    {
        EL<TEdit>("TfCleaning", "edPinSingleN")->Text=DeviceForm_File.ForcePerPinN;
    }
    else
    {
        EL<TEdit>("TfCleaning", "edPinSingleN")->Text =FormatFloat("0.0000", TestIF_File.fAutoClean_ForcePerPin);
    }

    EL<TEdit>("TfCleaning", "edtAutoCleanDieForce")->Text      =FormatFloat("0.00", TestIF_File.fAutoClean_DieForce);          //Steven 20240719 : Die force for auto clean
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Text    =FormatFloat("0.00", TestIF_File.fAutoClean_AireForce);         //Ifor 20190506 :TStaticText  ==>  TEdit
    EL<TEdit>("TfCleaning", "edACContactShiftHeight")->Text    =ConvertTouMType(TestIF_File.iAutoClean_ContactShiftHeight);
    EL<TEdit>("TfCleaning", "edACContactCleanHeight")->Text    =ConvertTouMType(TestIF_File.iAutoClean_ContactCleanHeight);

    EL<TEdit>("TfCleaning", "edIndexPickOffset")->Text         =ConvertTouMType(TestIF_File.iAutoClean_IndexPickOffset);       //wei 20150318 Auto clean Index Pick Offset
    EL<TEdit>("TfCleaning", "edIndexReleaseOffset")->Text      =ConvertTouMType(TestIF_File.iAutoClean_IndexReleaseOffset);    //Jou 2015-08-22 Auto clean Index Release Offset
    EL<TEdit>("TfCleaning", "edShuttle1PickOffset")->Text      =ConvertToMMType(TestIF_File.iAutoClean_Shuttle1PickOffset);    //2008/10/22 lee

    EL<TEdit>("TfCleaning", "edtShuttle1PlaceOffset")->Text    =ConvertToMMType(TestIF_File.iAutoClean_Shuttle1PlaceOffset);
    EL<TEdit>("TfCleaning", "edtShuttle1XOffset")->Text        =ConvertToMMType(TestIF_File.iAutoClean_Shuttle1XOffset);
    EL<TEdit>("TfCleaning", "edtShuttle1YOffset")->Text        =ConvertToMMType(TestIF_File.iAutoClean_Shuttle1YOffset);
    EL<TEdit>("TfCleaning", "edCleanPadDeviation")->Text       =ConvertToMMType(TestIF_File.iPadThickness);                    //kevin 20180630 add clean pad - devive

    EL<TEdit>("TfCleaning", "edShuttle2PickOffset")->Text      =ConvertToMMType(TestIF_File.iAutoClean_Shuttle2PickOffset);    //2008/10/22 lee
    EL<TEdit>("TfCleaning", "edtShuttle2PlaceOffset")->Text    =ConvertToMMType(TestIF_File.iAutoClean_Shuttle2PlaceOffset);
    EL<TEdit>("TfCleaning", "edtShuttle2XOffset")->Text        =ConvertToMMType(TestIF_File.iAutoClean_Shuttle2XOffset);
    EL<TEdit>("TfCleaning", "edtShuttle2YOffset")->Text        =ConvertToMMType(TestIF_File.iAutoClean_Shuttle2YOffset);

    if(IniConfig.bSPILFunction==true)                                           //wei 20150708 12site 16site 不能使用雙Arm  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        if(MachineTypeChoice!=Type_HT9046_LS &&                                 //JerryYang 20220922 : LS可以跑16 site雙ARM auto clean
           ((iInArmType==e9045_2x6_8 && iCloseSiteModeFor2x6<=e2x6OneByOne)  || //Steven 20240417 : 12site關中間4site
            (iInArmType==e9045_2x8_8 && iCloseSiteModeFor2x8<=e2x8OneByOne)))
        {
            TestIF_File.bCleanIndexOtherArm=false;
        }
    }
    EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Checked=TestIF_File.bCleanIndexOtherArm;

    EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex  =TestIF_File.iAutoClean_SelectArm;         //ChungHung 20131218 add for SCK request  //0:Arm1 1:Arm2 2:Arm1 & Arm2

    EL<TCheckBox>("TfCleaning", "cbFailAlarmLowYield")->Checked     =TestIF_File.bAutoClean_FailAlarmLowYield;
    EL<TEdit>("TfCleaning", "edtLowYieldLimit")->Text           =TestIF_File.iAutoClean_LowYieldLimit;
    EL<TEdit>("TfCleaning", "edtLowYieldCount")->Text           =TestIF_File.iAutoClean_LowYieldCount;

    EL<TCheckBox>("TfCleaning", "cbFailAlarmSiteYieldDifferent")->Checked =TestIF_File.bAutoClean_FailAlarmSiteYieldDifferent;
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYield")->Text            =TestIF_File.iAutoClean_FailAlarmSiteYield;
    EL<TEdit>("TfCleaning", "edtFailAlarmSiteYieldDifferent")->Text   =TestIF_File.iAutoClean_FailAlarmSiteYieldDifferentCount;

    EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Normal")->Checked      =TestIF_File.bAutoClean_ConseFailureBySocket_Normal;
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Normal")->Text   =TestIF_File.iAutoClean_ConseFailureCountBySocket_Normal;
    EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Retest")->Checked      =TestIF_File.bAutoClean_ConseFailureBySocket_Retest;
    EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Retest")->Text   =TestIF_File.iAutoClean_ConseFailureCountBySocket_Retest;
    EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Normal")->Checked        =TestIF_File.bAutoClean_ConseFailureByHead_Normal;
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Normal")->Text     =TestIF_File.iAutoClean_ConseFailureCountByHead_Normal;
    EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Retest")->Checked        =TestIF_File.bAutoClean_ConseFailureByHead_Retest;
    EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Retest")->Text     =TestIF_File.iAutoClean_ConseFailureCountByHead_Retest;

    if(CosFunction.bAutoCleanTimeCT==true)                                      //jou 20250102 : auto clean triger time count
    {
        EL<TCheckBox>("TfCleaning", "chkTimeCT")->Checked     =TestIF_File.bEnabledAutoCleanTimeCT;
        EL<TEdit>("TfCleaning", "edTimeCT")->Text         =TestIF_File.iAutoCleanTimeCT;
    }
    EL<TCheckBox>("TfCleaning", "chkUseNSKit")->Checked= TestIF_File.bAutoClean_UseNSKit;                      //jou 2013-08-08 SPIL use NS Clean kit

    if(TestIF_File.bAutoClean_UseTray)
    CL_RadioIndex(EL<TRadioGroup>("TfCleaning", "rgCleanKitType"), 1);   // golden AutoClean/uCleaning.cpp:775-775 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:775-775 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex=1;
#endif // GATE (S12-C save)
    else
    CL_RadioIndex(EL<TRadioGroup>("TfCleaning", "rgCleanKitType"), 0);   // golden AutoClean/uCleaning.cpp:777-777 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:777-777 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex=0;
#endif // GATE (S12-C save)

    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==0)
    {
        EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex=0;
    }
    else
    {
        EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex=1;
    }

    EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->ItemIndex       =iAutoCleanShuttle;

    EL<TEdit>("TfCleaning", "edtBufferKitLTX")->Text=TestIF_File.dBufferKitLTX;
    EL<TEdit>("TfCleaning", "edtBufferKitLTY")->Text=TestIF_File.dBufferKitLTY;
    EL<TEdit>("TfCleaning", "edtBufferKitLDX")->Text=TestIF_File.dBufferKitLDX;
    EL<TEdit>("TfCleaning", "edtBufferKitLDY")->Text=TestIF_File.dBufferKitLDY;
    EL<TEdit>("TfCleaning", "edtBufferKitRTX")->Text=TestIF_File.dBufferKitRTX;
    EL<TEdit>("TfCleaning", "edtBufferKitRTY")->Text=TestIF_File.dBufferKitRTY;
    EL<TEdit>("TfCleaning", "edtBufferKitRDX")->Text=TestIF_File.dBufferKitRDX;
    EL<TEdit>("TfCleaning", "edtBufferKitRDY")->Text=TestIF_File.dBufferKitRDY;

    EL<TEdit>("TfCleaning", "edtHotplateXOffset")->Text        = ConvertToMMType(TestIF_File.HotplatlXOffset);       //kevin 20150720 add Kit or tray Xpos
    EL<TEdit>("TfCleaning", "edtHotplateYOffset")->Text        = ConvertToMMType(TestIF_File.HotplatlYOffset);       //kevin 20150720 add Kit or tray Ypos
    EL<TEdit>("TfCleaning", "edtHotplatePickOffset")->Text     = ConvertToMMType(TestIF_File.HotplatlPickOffset);    //kevin 20150720 add Kit or tray Pick
    EL<TEdit>("TfCleaning", "edtHotplatePlaceOffset")->Text    = ConvertToMMType(TestIF_File.HotplatlPlaceOffset);   //kevin 20150720 add Kit or tray Place

    EL<TEdit>("TfCleaning", "edtShuttlePitch")->Text           = ConvertToMMType(TestIF_File.ShuttlePitchOffset);    //kevin 20150720 add Kit or tray Pick
    EL<TEdit>("TfCleaning", "edtHotplatePitchOffset")->Text    = ConvertToMMType(TestIF_File.HotplatlPitchOffset);   //kevin 20150720 add Kit or tray Place

    CL_ShowTranNToGf();
    CL_ShowCleanContactForce();
    SearchCleanNum();                                                           //Steven 20180524 : Fixed for clean count

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220330 SPIL佶融要求把contact offset與auto clean offset切開
    {
        Prod.iAutoCleanZ_Contact[0] =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness;     //JerryYang 20181119 (Steven) : fix auto clean 高度異常    //Steven 20181108 : 修正Auto Clean位置異常
        Prod.iAutoCleanZ_Drop[0]    =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[0]   =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactShiftHeight;
        Prod.iAutoCleanZ_Contact[1] =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness;
        Prod.iAutoCleanZ_Drop[1]    =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[1]   =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactShiftHeight;
    }
    else
    {
        Prod.iAutoCleanZ_Contact[0] =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness;     //JerryYang 20181119 (Steven) : fix auto clean 高度異常    //Steven 20181108 : 修正Auto Clean位置異常
        Prod.iAutoCleanZ_Drop[0]    =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[0]   =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF.iAutoClean_ContactShiftHeight;
        Prod.iAutoCleanZ_Contact[1] =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness;
        Prod.iAutoCleanZ_Drop[1]    =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[1]   =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF.iAutoClean_ContactShiftHeight;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung) filerw::ELTodo("golden main.cpp:31331 fMain->bEnableAutoclean() (CC_ASE_KaohSiung main-screen AutoClean button) -- TfMain facade has none");   // golden AutoClean/uCleaning.cpp:830-830 取代：fMain->bEnableAutoclean()（golden main.cpp:31331）：第一行 CUSTOMER_CODE!=CC_ASE_KaohSiung 就 return，其餘只切主畫面 AutoClean 鈕的 Visible；TfMain 門面沒有（同 GATE G-SU-AcBtn）。客戶專屬 → 只記
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:830-830 -- fMain->bEnableAutoclean()（golden main.cpp:31331）：第一行 CUSTOMER_CODE!=CC_ASE_KaohSiung 就 return，其餘只切主畫面 AutoClean 鈕的 Visible；TfMain 門面沒有（同 GATE G-SU-AcBtn）。客戶專屬 → 只記
    fMain->bEnableAutoclean();                                                  //Steven 20211104 : 修正Manual Auto Clean的按鈕顯示時機點
#endif // GATE (S12-C save)
    CL_DrawAutoClean();

    fLotInfo->RefreshYieldMonitor();                                            //Sam 20230111 : Smart Auto Clean

}

// golden AutoClean/uCleaning.cpp:836  TfCleaning::SaveAutoCleanData()
static void CL_SaveAutoCleanData()
{
    filerw::ELMark("SaveAutoCleanData");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString S="";
    AnsiString sData="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160219 Auto Clean by機台存檔
    {
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoClean.Data");
    }
    else
    {
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    }

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {
        IniConfig.bE43AutoCleanUseHotplate=EL<TCheckBox>("TfCleaning", "chkE43")->Checked;
    }

    TestIF_File.iAutoClean_Function=EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex;

    TestIF_File.iAutoClean_Mode = 0;

    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode1")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode1")->Visible) TestIF_File.iAutoClean_Mode |= M_INIT_START;    //kevin 20170126 (Steven) Modify for Auto Clean 元件功能沒開啟就不能使用
    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2")->Visible) TestIF_File.iAutoClean_Mode |= M_INIT_RESTART;
    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3")->Visible) TestIF_File.iAutoClean_Mode |= M_FINISH;
    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode4")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode4")->Visible) TestIF_File.iAutoClean_Mode |= M_MANUAL;
    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Visible) TestIF_File.iAutoClean_Mode |= M_SOCKET_ALARM;
    if(EL<TCheckBox>("TfCleaning", "chkAutoCleanMode6")->Checked && EL<TCheckBox>("TfCleaning", "chkAutoCleanMode6")->Visible) TestIF_File.iAutoClean_Mode |= M_INTERVAL;

    if(TestIF_File.iAutoClean_Mode==0)
    {
        TestIF_File.iAutoClean_Function=false;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex=0;                                          //Ifor 20180730 (Steven) : add Auto Clean 未選取功能強制關閉
    }

    if(CosFunction.bSmartAutoClean)                                             //Sam 20230111 : Smart Auto Clean
    {
        if(TestIF_File.iAutoClean_Function==false && EL<TCheckBox>("TfCleaning", "chkACSmart")->Checked)
        {
            filerw::ELMessage("Please enable auto clean function", "請開啟自動清潔功能");
            return;
        }
    }

    TestIF_File.iAutoClean_IntervalContact      =atoi(EL<TEdit>("TfCleaning", "edIntervalContact")->Text.c_str());

    if(TestIF.iShuttleMode!=0)                                                  //JerryYang 20160308 Start:修正切換模式時 clean pad數量沒有更新的問題
    {
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex=TestIF.iShuttle_Sel;                    //ChungHung 20131218 add for SCK request  //0:Arm1 1:Arm2 2:Arm1 & Arm2
    }
    TestIF_File.bCleanIndexOtherArm             =EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Checked;

    if(TestIF_File.iAutoClean_Function &&
       (MachineTypeChoice==Type_HT9045 ||
        MachineTypeChoice==Type_HT9046 ||
        MachineTypeChoice==Type_HT9045_12Site) &&
       TestIF.iTestMode>=QualSite2X2  &&
       TestIF.iTestMode!=QualSite2X2N &&
       TestIF_File.dSiteYPitch>=65.0)                                           //Steven 20211227 : HT9045跑兩排的, Site Y pitch大於65只能用Arm 1
    {
        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1 && TestIF_File.iAutoClean_SelectArm==1) ||
           EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex==1)
        {
            filerw::ELMessage("The site Y-pitch can not use arm 2 for auto clean!!");
        }
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex=0;
    }

    TestIF_File.iAutoClean_SelectArm=EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex;
    iBackupCleaningDeveicePices =TestIF_File.iAutoClean_DeveicePices;

    CL_SetDeviceMaxMin();
    if(CL_VarGE(((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min), EL<TEdit>("TfCleaning", "edDevicePices")->Text))   // golden AutoClean/uCleaning.cpp:910-910 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:910-910 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    if(EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min>=EL<TEdit>("TfCleaning", "edDevicePices")->Text)
#endif // GATE (S12-C save)
    {
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:912-912 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:912-912 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;
#endif // GATE (S12-C save)
    }

    if(TestIF_File.iTestMode==_12Site2X6)
        TestIF_File.iAutoClean_DeveicePices=atoi(EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text.c_str());   //Steven 20241113 : for 2x6 auto clean
    else
        TestIF_File.iAutoClean_DeveicePices=atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str());

//    if(chkCleanOtherArm->Checked==true)
//    {
//        if(MachineTypeChoice!=Type_HT9046_LS &&                                 //JerryYang 20220922 : LS可以跑16 site雙ARM auto clean
//           (TestIF_File.iTestMode==_12Site2X6 ||
//            TestIF_File.iTestMode==_10Site2X5 ||
//            TestIF_File.iTestMode==_16Site2X8))
//        {
//            ShowMyMessage("No Use Index Arm 2, Suck Clean Pad Over Y Limit !!");
//            chkCleanOtherArm->Checked=false;
//        }
//        else if(TestIF_File.iAutoClean_DeveicePices<(TestSocket.iShtCnt*2))
//        {
//            ShowMyMessage("Click Clean Index Arm 2 Clean Pad need Double!!");
//            edDevicePices->Text=TestSocket.iShtCnt*2;
//            TestIF_File.iAutoClean_DeveicePices=TestSocket.iShtCnt*2;
//            DrawAutoClean();
//        }
//    }
    TestIF_File.iAutoClean_Tray                 =EL<TRadioGroup>("TfCleaning", "rgKitPosition")->ItemIndex;      //Steven 20120217 : 選擇把CleanPad放在哪邊
    TestIF_File.iIndexArmAutoCleanCnt           =atoi(EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Text.c_str());    //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    TestIF_File.iAutoClean_AlarmCount           =atoi(EL<TEdit>("TfCleaning", "edAlarmCount")->Text.c_str());

    TestIF_File.iAutoClean_MotorSpeed[0]        =atoi(EL<TEdit>("TfCleaning", "OutArmSpeed")->Text.c_str());
    TestIF_File.iAutoClean_MotorSpeed[1]        =atoi(EL<TEdit>("TfCleaning", "ShuttleSpeed")->Text.c_str());
    TestIF_File.iAutoClean_MotorSpeed[2]        =atoi(EL<TEdit>("TfCleaning", "IndexArmSpeed")->Text.c_str());
    TestIF_File.iAutoClean_MotorSpeed[3]        =atoi(EL<TEdit>("TfCleaning", "edtInArmZSpeed")->Text.c_str());
    TestIF_File.iAutoClean_ContactMode          =EL<TComboBox>("TfCleaning", "ContactMode")->ItemIndex;

    TestIF_File.dAutoClean_InArmVacuum          =atof(EL<TEdit>("TfCleaning", "edtInArmVacuum")->Text.c_str());    //Steven 20250319 : Auto Clean使用另外一組Delay Time
    TestIF_File.dAutoClean_InArmAirOn           =atof(EL<TEdit>("TfCleaning", "edtInArmAirOn")->Text.c_str());
    TestIF_File.dAutoClean_IndexVacuum          =atof(EL<TEdit>("TfCleaning", "edtIndexVacuum")->Text.c_str());
    TestIF_File.dAutoClean_IndexAirOn           =atof(EL<TEdit>("TfCleaning", "edtIndexAirOn")->Text.c_str());

    TestIF_File.iAutoCleanDropHigh              =Get0_01MMType(EL<TEdit>("TfCleaning", "edDropOffset1")->Text.c_str()); //kevin 20180717 autoClean drop high

    double dAutoClean_ContactTime=0.0;
    dAutoClean_ContactTime=atof(EL<TEdit>("TfCleaning", "edContactTime")->Text.c_str())*10.0;
    TestIF_File.iAutoClean_ContactTime          =dAutoClean_ContactTime;
    TestIF_File.iAutoClean_ContactCount         =atoi(EL<TEdit>("TfCleaning", "edACContactCount")->Text.c_str());
    if(CosFunction.bSmartAutoClean)                                             //Sam 20230111 : Smart Auto Clean
    {
        TestIF_File.bACSmart=EL<TCheckBox>("TfCleaning", "chkACSmart")->Checked;
        TestIF_File.iACSmart_Count          =atoi(EL<TEdit>("TfCleaning", "edtACSmart")->Text.c_str());
        TestIF_File.iACSmart_Count_CTF      =atoi(EL<TEdit>("TfCleaning", "edACSmartCTF")->Text.c_str());  //Sam 20240726 : AI Clean
        TestIF_File.iACSmart_ContactMode    =EL<TComboBox>("TfCleaning", "cbbACSmart_ContactMode")->ItemIndex;
        TestIF_File.iACSmart_DropHigh       =Get0_01MMType(EL<TEdit>("TfCleaning", "edtACSmart_DropOffset1")->Text.c_str());
        dAutoClean_ContactTime=atof(EL<TEdit>("TfCleaning", "edtACSmart_ContactTime")->Text.c_str())*10.0;
        TestIF_File.iACSmart_ContactTime    =dAutoClean_ContactTime;
        TestIF_File.iACSmart_ContactCount   =atoi(EL<TEdit>("TfCleaning", "edtACSmart_ACContactCount")->Text.c_str());

        TestIF_File.iAdaptiveACIntervalMax=atoi(EL<TEdit>("TfCleaning", "edAdaptiveIntervalMax")->Text.c_str());
        TestIF_File.iAdaptiveACIntervalMin  =atoi(EL<TEdit>("TfCleaning", "edAdaptiveIntervalMin")->Text.c_str());
        TestIF_File.iAdaptiveACIntervalAdj  =atoi(EL<TEdit>("TfCleaning", "edAdaptiveIntervalAdj")->Text.c_str());
    }
    TestIF_File.fAutoClean_AireForce            =atof(EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Text.c_str());//Ifor 20190506 :TStaticText  ==>  TEdit
    TestIF_File.fAutoClean_DieForce             =atof(EL<TEdit>("TfCleaning", "edtAutoCleanDieForce")->Text.c_str());  //Steven 20240719 : Die force for auto clean
    TestIF_File.iAutoClean_DevicePinCount       =atoi(EL<TEdit>("TfCleaning", "edPinsCount")->Text.c_str());
    TestIF_File.fAutoClean_ForcePerPin          =atof(EL<TEdit>("TfCleaning", "edPinSingleN")->Text.c_str());//kevin 2015086 add

    TestIF_File.fAutoClean_DevicePinForceGf     =atof(EL<TEdit>("TfCleaning", "edPinSingleGf")->Text.c_str());
    TestIF_File.iAutoClean_ContactShiftHeight   =Get0_01MMType(EL<TEdit>("TfCleaning", "edACContactShiftHeight")->Text.c_str());
    TestIF_File.iAutoClean_ContactCleanHeight   =Get0_01MMType(EL<TEdit>("TfCleaning", "edACContactCleanHeight")->Text.c_str());
    TestIF_File.iAutoClean_IndexPickOffset      =Get0_01MMType(EL<TEdit>("TfCleaning", "edIndexPickOffset")->Text.c_str());      //wei 20150318 Auto clean Index Pick Offset
    TestIF_File.iAutoClean_IndexReleaseOffset   =Get0_01MMType(EL<TEdit>("TfCleaning", "edIndexReleaseOffset")->Text.c_str());   //Jou 2015-08-22 Auto clean Index Release Offset
    TestIF_File.iAutoClean_Shuttle1PickOffset   =Get0_01MMType(EL<TEdit>("TfCleaning", "edShuttle1PickOffset")->Text.c_str());   //2008/10/22 lee

    TestIF_File.iAutoClean_Shuttle1PlaceOffset  =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle1PlaceOffset")->Text.c_str());
    TestIF_File.iAutoClean_Shuttle1XOffset      =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle1XOffset")->Text.c_str());
    TestIF_File.iAutoClean_Shuttle1YOffset      =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle1YOffset")->Text.c_str());
    TestIF_File.iPadThickness                   =Get0_01MMType(EL<TEdit>("TfCleaning", "edCleanPadDeviation")->Text.c_str()); //kevin 20180630 add clean pad - devive

    TestIF_File.iAutoClean_Shuttle2PickOffset   =Get0_01MMType(EL<TEdit>("TfCleaning", "edShuttle2PickOffset")->Text.c_str());   //2008/10/22 lee
    TestIF_File.iAutoClean_Shuttle2PlaceOffset  =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle2PlaceOffset")->Text.c_str());
    TestIF_File.iAutoClean_Shuttle2XOffset      =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle2XOffset")->Text.c_str());
    TestIF_File.iAutoClean_Shuttle2YOffset      =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttle2YOffset")->Text.c_str());

    TestIF_File.dAutoClean_XPitch_Kit           =Get0_01MMType(EL<TEdit>("TfCleaning", "XPitch1")->Text.c_str());
    TestIF_File.dAutoClean_YPitch_Kit           =Get0_01MMType(EL<TEdit>("TfCleaning", "YPitch1")->Text.c_str());
    TestIF_File.dAutoClean_XStart_Kit           =Get0_01MMType(EL<TEdit>("TfCleaning", "XST1")->Text.c_str());
    TestIF_File.dAutoClean_YStart_Kit           =Get0_01MMType(EL<TEdit>("TfCleaning", "YST1")->Text.c_str());
    TestIF_File.iAutoClean_XDivision_Kit        =atoi(EL<TEdit>("TfCleaning", "XCT1")->Text.c_str());
    TestIF_File.iAutoClean_YDivision_Kit        =atoi(EL<TEdit>("TfCleaning", "YCT1")->Text.c_str());

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150908
    {
        TestIF_File.dAutoClean_XPitch_Tray      =UserDefForm_File[0].XPitch*100;
        TestIF_File.dAutoClean_YPitch_Tray      =UserDefForm_File[0].YPitch*100;
        TestIF_File.dAutoClean_XStart_Tray      =UserDefForm_File[0].XStart*100;
        TestIF_File.dAutoClean_YStart_Tray      =UserDefForm_File[0].YStart*100;
    }
    else
    {
        TestIF_File.dAutoClean_XPitch_Tray      =Get0_01MMType(EL<TEdit>("TfCleaning", "XPitch2")->Text.c_str());
        TestIF_File.dAutoClean_YPitch_Tray      =Get0_01MMType(EL<TEdit>("TfCleaning", "YPitch2")->Text.c_str());
        TestIF_File.dAutoClean_XStart_Tray      =Get0_01MMType(EL<TEdit>("TfCleaning", "XST2")->Text.c_str());
        TestIF_File.dAutoClean_YStart_Tray      =Get0_01MMType(EL<TEdit>("TfCleaning", "YST2")->Text.c_str());
    }
    TestIF_File.iAutoClean_XDivision_Tray       =atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());
    TestIF_File.iAutoClean_YDivision_Tray       =atoi(EL<TEdit>("TfCleaning", "YCT2")->Text.c_str());
    TestIF_File.cAutoClean_PackageTray          =EL<TEdit>("TfCleaning", "PageTypeName")->Text;            //kevin 20150521

    iAutoCleanShuttle                           =EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->ItemIndex;

    if(TestIF.iShuttleMode!=0)                                                  //wei 20150519 1-->0
    {
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Checked=false;
    }

    TestIF_File.bAutoClean_FailAlarmLowYield = EL<TCheckBox>("TfCleaning", "cbFailAlarmLowYield")->Checked;
    TestIF_File.iAutoClean_LowYieldLimit     = atoi(EL<TEdit>("TfCleaning", "edtLowYieldLimit")->Text.c_str());
    TestIF_File.iAutoClean_LowYieldCount     = atoi(EL<TEdit>("TfCleaning", "edtLowYieldCount")->Text.c_str());

    TestIF_File.bAutoClean_FailAlarmSiteYieldDifferent      =EL<TCheckBox>("TfCleaning", "cbFailAlarmSiteYieldDifferent")->Checked;
    TestIF_File.iAutoClean_FailAlarmSiteYield               =atoi(EL<TEdit>("TfCleaning", "edtFailAlarmSiteYield")->Text.c_str());
    TestIF_File.iAutoClean_FailAlarmSiteYieldDifferentCount =atoi(EL<TEdit>("TfCleaning", "edtFailAlarmSiteYieldDifferent")->Text.c_str());

    TestIF_File.bAutoClean_ConseFailureBySocket_Normal      =EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Normal")->Checked;
    TestIF_File.iAutoClean_ConseFailureCountBySocket_Normal =atoi(EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Normal")->Text.c_str());
    TestIF_File.bAutoClean_ConseFailureBySocket_Retest      =EL<TCheckBox>("TfCleaning", "cbConseFailureBySocket_Retest")->Checked;
    TestIF_File.iAutoClean_ConseFailureCountBySocket_Retest =atoi(EL<TEdit>("TfCleaning", "edtConseFailureCountBySocket_Retest")->Text.c_str());
    TestIF_File.bAutoClean_ConseFailureByHead_Normal        =EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Normal")->Checked;
    TestIF_File.iAutoClean_ConseFailureCountByHead_Normal   =atoi(EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Normal")->Text.c_str());
    TestIF_File.bAutoClean_ConseFailureByHead_Retest        =EL<TCheckBox>("TfCleaning", "cbConseFailureByHead_Retest")->Checked;
    TestIF_File.iAutoClean_ConseFailureCountByHead_Retest   =atoi(EL<TEdit>("TfCleaning", "edtConseFailureCountByHead_Retest")->Text.c_str());

    if(CosFunction.bAutoCleanTimeCT==true)                                      //jou 20250102 : auto clean triger time count
    {
        TestIF_File.bEnabledAutoCleanTimeCT = EL<TCheckBox>("TfCleaning", "chkTimeCT")->Checked;
        TestIF_File.iAutoCleanTimeCT        = atoi(EL<TEdit>("TfCleaning", "edTimeCT")->Text.c_str());
    }

    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==1)
        TestIF_File.bAutoClean_UseTray=true;
    else
        TestIF_File.bAutoClean_UseTray=false;

    TestIF_File.dBufferKitLTX=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitLTX")->Text.c_str());
    TestIF_File.dBufferKitLTY=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitLTY")->Text.c_str());
    TestIF_File.dBufferKitLDX=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitLDX")->Text.c_str());
    TestIF_File.dBufferKitLDY=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitLDY")->Text.c_str());
    TestIF_File.dBufferKitRTX=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitRTX")->Text.c_str());
    TestIF_File.dBufferKitRTY=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitRTY")->Text.c_str());
    TestIF_File.dBufferKitRDX=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitRDX")->Text.c_str());
    TestIF_File.dBufferKitRDY=Get0_01MMType(EL<TEdit>("TfCleaning", "edtBufferKitRDY")->Text.c_str());

    if((iBackupCleaningDeveicePices!=TestIF_File.iAutoClean_DeveicePices) ||    //ChungHung 201304226修正變換AutoClean Tray盤資料時圖片不會及時更新
       (TestIF.bAutoClean_UseTray!=TestIF_File.bAutoClean_UseTray       ) ||
       (TestIF_File.bAutoClean_UseTray==false && (TestIF_File.iAutoClean_XDivision!=TestIF_File.iAutoClean_XDivision_Kit  || TestIF_File.iAutoClean_YDivision!=TestIF_File.iAutoClean_YDivision_Kit))    ||
       (TestIF_File.bAutoClean_UseTray==true  && (TestIF_File.iAutoClean_XDivision!=TestIF_File.iAutoClean_XDivision_Tray || TestIF_File.iAutoClean_YDivision!=TestIF_File.iAutoClean_YDivision_Tray))   ||
       (iBackupAutoClean_Function!=TestIF_File.iAutoClean_Function) ||          //KEVIN 20120217
       (CosFunction.bCanChangeAutoCleanCount &&
        atoi(EL<TEdit>("TfCleaning", "edCleaningCount")->Text.c_str())!=iAutoCleanCnt))                    //JerryYang 20171128 (Steven) auto clean 清潔次數可修改
    {
        TestIF.bAutoClean_UseTray=TestIF_File.bAutoClean_UseTray;
        if(TestIF_File.bAutoClean_UseTray)
        {
            TestIF_File.dAutoClean_XPitch=TestIF_File.dAutoClean_XPitch_Tray;
            TestIF_File.dAutoClean_YPitch=TestIF_File.dAutoClean_YPitch_Tray;
            TestIF_File.dAutoClean_XStart=TestIF_File.dAutoClean_XStart_Tray;
            TestIF_File.dAutoClean_YStart=TestIF_File.dAutoClean_YStart_Tray;
            TestIF_File.iAutoClean_XDivision=TestIF_File.iAutoClean_XDivision_Tray;
            TestIF_File.iAutoClean_YDivision=TestIF_File.iAutoClean_YDivision_Tray;
        }
        else
        {
            TestIF_File.dAutoClean_XPitch=TestIF_File.dAutoClean_XPitch_Kit;
            TestIF_File.dAutoClean_YPitch=TestIF_File.dAutoClean_YPitch_Kit;
            TestIF_File.dAutoClean_XStart=TestIF_File.dAutoClean_XStart_Kit;
            TestIF_File.dAutoClean_YStart=TestIF_File.dAutoClean_YStart_Kit;
            TestIF_File.iAutoClean_XDivision=TestIF_File.iAutoClean_XDivision_Kit;
            TestIF_File.iAutoClean_YDivision=TestIF_File.iAutoClean_YDivision_Kit;
        }

        if(bRunAutoClean==false)
        {
            RecordProcess("Auto clean : reset data because AutoClean Kit Data was change");

            for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)                 //Steven 20180524 : Fixed for clean count
            {
                for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[X][Y]!=NULL_IC)            //Steven 20210127 : 修正Auto clean存檔時候, 會改變Count
                        SetAutoCleanStringGrid(X, Y+1, EL<TEdit>("TfCleaning", "edCleaningCount")->Text);
                }
            }
            ReadWriteAutoCleanCount(false);
        }
    }

    iBackupAutoClean_Function=TestIF_File.iAutoClean_Function;                  //Ifor 20171024 : add 避免造成資料異常 rgAutoCleanOnOff->ItemIndex => TestIF_File.iAutoClean_Function
    TestIF_File.bAutoClean_UseNSKit=EL<TCheckBox>("TfCleaning", "chkUseNSKit")->Checked;                       //jou 2013-08-08 SPIL use NS Clean kit

    TestIF_File.HotplatlPickOffset              =Get0_01MMType(EL<TEdit>("TfCleaning", "edtHotplatePickOffset")->Text.c_str());    //kevin 20150720 add kit or tray pick
    TestIF_File.HotplatlPlaceOffset             =Get0_01MMType(EL<TEdit>("TfCleaning", "edtHotplatePlaceOffset")->Text.c_str());   //kevin 20150720 add kit or tray place
    TestIF_File.HotplatlXOffset                 =Get0_01MMType(EL<TEdit>("TfCleaning", "edtHotplateXOffset")->Text.c_str());       //kevin 20150720 add kit or tray Xpos
    TestIF_File.HotplatlYOffset                 =Get0_01MMType(EL<TEdit>("TfCleaning", "edtHotplateYOffset")->Text.c_str());       //kevin 20150720 add kit or tray Ypos

    TestIF_File.HotplatlPitchOffset             =Get0_01MMType(EL<TEdit>("TfCleaning", "edtHotplatePitchOffset")->Text.c_str());   //kevin 20150720 addkit or tray pitch
    TestIF_File.ShuttlePitchOffset              =Get0_01MMType(EL<TEdit>("TfCleaning", "edtShuttlePitch")->Text.c_str());          //kevin 20150720 add shuttle Pitch

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {
        WriteIniData(szDir, "Configuration", "bAutoCleanUseHotplate",       IniConfig.bE43AutoCleanUseHotplate);
//        if(TestIF_File.iAutoClean_Function && IniConfig.bE43AutoCleanUseHotplate)
        if(IniConfig.bE43AutoCleanUseHotplate)   //Ifor 20251204 fix:
        {
            HotPlateForm_File.iPlateSelect=2;
            HotPlateForm.iPlateSelect=2;
            WriteIniData(szDir, "Hotplate Form", "Using Flag", HotPlateForm.iPlateSelect);
        }
    }
    WriteIniData(szDir, "Configuration", "iAutoClean_Function",             TestIF_File.iAutoClean_Function);
    WriteIniData(szDir, "Configuration", "iAutoClean_Mode",                 TestIF_File.iAutoClean_Mode);
    WriteIniData(szDir, "Configuration", "iAutoClean_IntervalContact",      TestIF_File.iAutoClean_IntervalContact);
    WriteIniData(szDir, "Configuration", "iAutoClean_DeveicePices",         TestIF_File.iAutoClean_DeveicePices);
    WriteIniData(szDir, "Configuration", "iAutoClean_Tray",                 TestIF_File.iAutoClean_Tray);   //Steven 20120217 : 選擇把CleanPad放在哪邊
    WriteIniData(szDir, "Configuration", "iAutoClean_AlarmCount",           TestIF_File.iAutoClean_AlarmCount);
    WriteIniData(szDir, "Configuration", "iAutoClean_iPadThickness",        TestIF_File.iPadThickness);//kevin 20180630 add clean pad - device

    WriteIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[0]",        TestIF_File.iAutoClean_MotorSpeed[0]);
    WriteIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[1]",        TestIF_File.iAutoClean_MotorSpeed[1]);
    WriteIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[2]",        TestIF_File.iAutoClean_MotorSpeed[2]);
    WriteIniData(szDir, "Configuration", "iAutoClean_MotorSpeed[3]",        TestIF_File.iAutoClean_MotorSpeed[3]);
    WriteIniData(szDir, "Configuration", "iAutoClean_ContactMode",          TestIF_File.iAutoClean_ContactMode);
    WriteIniData(szDir, "Configuration", "iAutoClean_DropHigh",             TestIF_File.iAutoCleanDropHigh);//kevin 20180717 autoClean drop high

    WriteIniData(szDir, "AutoClean", "InArmVacuum",                         TestIF_File.dAutoClean_InArmVacuum);       //Steven 20250319 : Auto Clean使用另外一組Delay Time
    WriteIniData(szDir, "AutoClean", "InArmAirOn",                          TestIF_File.dAutoClean_InArmAirOn);
    WriteIniData(szDir, "AutoClean", "IndexVacuum",                         TestIF_File.dAutoClean_IndexVacuum);
    WriteIniData(szDir, "AutoClean", "IndexAirOn",                          TestIF_File.dAutoClean_IndexAirOn);

    WriteIniData(szDir, "Configuration", "iAutoClean_ContactTime",          TestIF_File.iAutoClean_ContactTime);
    WriteIniData(szDir, "Configuration", "iAutoClean_ContactCount",         TestIF_File.iAutoClean_ContactCount);
    if(CosFunction.bSmartAutoClean)                                             //Sam 20230111 : Smart Auto Clean
    {
        WriteIniData(szDir, "Configuration", "bACSmart",                    TestIF_File.bACSmart);
        WriteIniData(szDir, "Configuration", "iACSmart_Count",              TestIF_File.iACSmart_Count);
        WriteIniData(szDir, "Configuration", "iACSmart_Count CTF",          TestIF_File.iACSmart_Count_CTF);    //Sam 20240726 : AI Clean
        WriteIniData(szDir, "Configuration", "iACSmart_ContactMode",        TestIF_File.iACSmart_ContactMode);
        WriteIniData(szDir, "Configuration", "iACSmart_DropHigh",           TestIF_File.iACSmart_DropHigh);
        WriteIniData(szDir, "Configuration", "iACSmart_ContactTime",        TestIF_File.iACSmart_ContactTime);
        WriteIniData(szDir, "Configuration", "iACSmart_ContactCount",       TestIF_File.iACSmart_ContactCount);

        WriteIniData(szDir, "Configuration", "iAdaptiveACIntervalMax",      TestIF_File.iAdaptiveACIntervalMax);
        WriteIniData(szDir, "Configuration", "iAdaptiveACIntervalMin",      TestIF_File.iAdaptiveACIntervalMin);
        WriteIniData(szDir, "Configuration", "iAdaptiveACIntervalAdj",      TestIF_File.iAdaptiveACIntervalAdj);
    }
    WriteIniData(szDir, "Configuration", "fAutoClean_AireForce",            TestIF_File.fAutoClean_AireForce);
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //AI(ht9045-v899) 20260430: extend AutoClean Die Force visibility to Multi EP half.
        WriteIniData(szDir, "Configuration", "fAutoClean_DieForce",         TestIF_File.fAutoClean_DieForce);   //Steven 20240719 : Die force for auto clean
    WriteIniData(szDir, "Configuration", "iAutoClean_DevicePinCount",       TestIF_File.iAutoClean_DevicePinCount);
    WriteIniData(szDir, "Configuration", "fAutoClean_ForcePerPin",          TestIF_File.fAutoClean_ForcePerPin);//kevin 20150826
    WriteIniData(szDir, "Configuration", "fAutoClean_DevicePinForceGf",     TestIF_File.fAutoClean_DevicePinForceGf);

    WriteIniData(szDir, "Configuration", "dAutoClean_XPitch_Kit",    TestIF_File.dAutoClean_XPitch_Kit);
    WriteIniData(szDir, "Configuration", "dAutoClean_YPitch_Kit",    TestIF_File.dAutoClean_YPitch_Kit);
    WriteIniData(szDir, "Configuration", "dAutoClean_XStart_Kit",    TestIF_File.dAutoClean_XStart_Kit);
    WriteIniData(szDir, "Configuration", "dAutoClean_YStart_Kit",    TestIF_File.dAutoClean_YStart_Kit);
    WriteIniData(szDir, "Configuration", "iAutoClean_XDivision_Kit", TestIF_File.iAutoClean_XDivision_Kit);
    WriteIniData(szDir, "Configuration", "iAutoClean_YDivision_Kit", TestIF_File.iAutoClean_YDivision_Kit);

    WriteIniData(szDir, "Configuration", "dAutoClean_XPitch_Tray",   TestIF_File.dAutoClean_XPitch_Tray);
    WriteIniData(szDir, "Configuration", "dAutoClean_YPitch_Tray",   TestIF_File.dAutoClean_YPitch_Tray);
    WriteIniData(szDir, "Configuration", "dAutoClean_XStart_Tray",   TestIF_File.dAutoClean_XStart_Tray);
    WriteIniData(szDir, "Configuration", "dAutoClean_YStart_Tray",   TestIF_File.dAutoClean_YStart_Tray);
    WriteIniData(szDir, "Configuration", "iAutoClean_XDivision_Tray",TestIF_File.iAutoClean_XDivision_Tray);
    WriteIniData(szDir, "Configuration", "iAutoClean_YDivision_Tray",TestIF_File.iAutoClean_YDivision_Tray);
    WriteIniData(szDir, "Configuration", "cAutoClean_PackageTray",   TestIF_File.cAutoClean_PackageTray);  //kevin 20150521
    WriteIniData(szDir, "Configuration", "iCleanIndexOtherArm",      TestIF_File.bCleanIndexOtherArm);

    WriteIniData(szDir, "Configuration", "iAutoClean_SelectArm",                        TestIF_File.iAutoClean_SelectArm);

    WriteIniData(szDir, "Configuration", "bAutoClean_FailAlarmLowYield",                TestIF_File.bAutoClean_FailAlarmLowYield);
    WriteIniData(szDir, "Configuration", "iAutoClean_LowYieldLimit",                    TestIF_File.iAutoClean_LowYieldLimit);
    WriteIniData(szDir, "Configuration", "iAutoClean_LowYieldCount",                    TestIF_File.iAutoClean_LowYieldCount);

    WriteIniData(szDir, "Configuration", "bAutoClean_FailAlarmSiteYieldDifferent",      TestIF_File.bAutoClean_FailAlarmSiteYieldDifferent);
    WriteIniData(szDir, "Configuration", "iAutoClean_FailAlarmSiteYield",               TestIF_File.iAutoClean_FailAlarmSiteYield);
    WriteIniData(szDir, "Configuration", "iAutoClean_FailAlarmSiteYieldDifferentCount", TestIF_File.iAutoClean_FailAlarmSiteYieldDifferentCount);

    WriteIniData(szDir, "Configuration", "bAutoClean_ConseFailureBySocket_Normal",      TestIF_File.bAutoClean_ConseFailureBySocket_Normal);
    WriteIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountBySocket_Normal", TestIF_File.iAutoClean_ConseFailureCountBySocket_Normal);
    WriteIniData(szDir, "Configuration", "bAutoClean_ConseFailureBySocket_Retest",      TestIF_File.bAutoClean_ConseFailureBySocket_Retest);
    WriteIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountBySocket_Retest", TestIF_File.iAutoClean_ConseFailureCountBySocket_Retest);
    WriteIniData(szDir, "Configuration", "bAutoClean_ConseFailureByHead_Normal",        TestIF_File.bAutoClean_ConseFailureByHead_Normal);
    WriteIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountByHead_Normal",   TestIF_File.iAutoClean_ConseFailureCountByHead_Normal);
    WriteIniData(szDir, "Configuration", "bAutoClean_ConseFailureByHead_Retest",        TestIF_File.bAutoClean_ConseFailureByHead_Retest);
    WriteIniData(szDir, "Configuration", "iAutoClean_ConseFailureCountByHead_Retest",   TestIF_File.iAutoClean_ConseFailureCountByHead_Retest);

    if(CosFunction.bAutoCleanTimeCT==true)                                      //jou 20250102 : auto clean triger time count
    {
        WriteIniData(szDir, "Configuration", "bEnabledAutoCleanTimeCT",                 TestIF_File.bEnabledAutoCleanTimeCT);
        WriteIniData(szDir, "Configuration", "iAutoCleanTimeCT",                        TestIF_File.iAutoCleanTimeCT);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150616
        TestIF_File.bAutoClean_UseTray=true;
    WriteIniData(szDir, "Configuration", "bAutoClean_UseTray", TestIF_File.bAutoClean_UseTray);

    WriteIniData(szDir, "Configuration", "iAutoCleanShuttle",  iAutoCleanShuttle);                             //kevin 20120711

    WriteIniData(szDir, "Configuration", "dBufferKitLTX", TestIF_File.dBufferKitLTX);
    WriteIniData(szDir, "Configuration", "dBufferKitLTY", TestIF_File.dBufferKitLTY);
    WriteIniData(szDir, "Configuration", "dBufferKitLDX", TestIF_File.dBufferKitLDX);
    WriteIniData(szDir, "Configuration", "dBufferKitLDY", TestIF_File.dBufferKitLDY);
    WriteIniData(szDir, "Configuration", "dBufferKitRTX", TestIF_File.dBufferKitRTX);
    WriteIniData(szDir, "Configuration", "dBufferKitRTY", TestIF_File.dBufferKitRTY);
    WriteIniData(szDir, "Configuration", "dBufferKitRDX", TestIF_File.dBufferKitRDX);
    WriteIniData(szDir, "Configuration", "dBufferKitRDY", TestIF_File.dBufferKitRDY);

    WriteIniData(szDir, "Configuration", "bAutoClean_UseNSKit",TestIF_File.bAutoClean_UseNSKit);    //jou 2013-08-08 SPIL use NS Clean kit

    if(CosFunction.bUseDefineAutoCleanOffset==true)                             //Ifor 20181106 add 新增Auto Clean Offset By 本機
    {
        szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoClean.Data");
    }
    WriteIniData(szDir, "Configuration", "iAutoClean_ContactShiftHeight",   TestIF_File.iAutoClean_ContactShiftHeight);
    WriteIniData(szDir, "Configuration", "iAutoClean_ContactCleanHeight",   TestIF_File.iAutoClean_ContactCleanHeight);
    WriteIniData(szDir, "Configuration", "iAutoClean_IndexPickOffset",      TestIF_File.iAutoClean_IndexPickOffset);          //wei 20150318 Auto clean Index Pick Offset
    WriteIniData(szDir, "Configuration", "iAutoClean_IndexReleaseOffset",   TestIF_File.iAutoClean_IndexReleaseOffset);       //Jou 2015-08-22 Auto clean Index Release Offset

    if(IniConfig.bE48_ShuttleUse4Offset_Autoclean==false)                       //Steven 20241102 : fixed for Auto Clean offset
    {
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle1PickOffset",   TestIF_File.iAutoClean_Shuttle1PickOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle1PlaceOffset",  TestIF_File.iAutoClean_Shuttle1PlaceOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle1XOffset",      TestIF_File.iAutoClean_Shuttle1XOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle1YOffset",      TestIF_File.iAutoClean_Shuttle1YOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle2PickOffset",   TestIF_File.iAutoClean_Shuttle2PickOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle2PlaceOffset",  TestIF_File.iAutoClean_Shuttle2PlaceOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle2XOffset",      TestIF_File.iAutoClean_Shuttle2XOffset);
        WriteIniData(szDir, "Configuration", "iAutoClean_Shuttle2YOffset",      TestIF_File.iAutoClean_Shuttle2YOffset);
        WriteIniData(szDir, "Configuration", "ShuttlePitchOffset",              TestIF_File.ShuttlePitchOffset);    //kevin 20150720
    }

    WriteIniData(szDir, "Configuration", "HotplatlXOffset",                 TestIF_File.HotplatlXOffset);       //kevin 20150720
    WriteIniData(szDir, "Configuration", "HotplatlYOffset",                 TestIF_File.HotplatlYOffset);       //kevin 20150720
    WriteIniData(szDir, "Configuration", "HotplatlPickOffset",              TestIF_File.HotplatlPickOffset);    //kevin 20150720
    WriteIniData(szDir, "Configuration", "HotplatlPlaceOffset",             TestIF_File.HotplatlPlaceOffset);   //kevin 20150720
    WriteIniData(szDir, "Configuration", "HotplatlPitchOffset",             TestIF_File.HotplatlPitchOffset);   //kevin 20150720

}

// golden AutoClean/uCleaning.cpp:1261  TfCleaning::LoadImage()
static void CL_LoadImage()
{

    ;   // UI-only（HTML 端處理）：EL<TControl>("TfCleaning", "Image3")->Picture->LoadFromFile(BmpPath+"HoatPlate1.bmp");
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfCleaning", "Image1")->Picture->LoadFromFile(BmpPath+"HoatPlate1.bmp");

}

// golden AutoClean/uCleaning.cpp:1268  TfCleaning::GetMinCleanPadCount()
static int CL_GetMinCleanPadCount()
{

    int iCount=1;
    DoInArm_9045_Type();
    fSetup->CheckSTMMode();                                                     //Steven 20221006 : 確認STM模式
    if(TestIF.iTestMode==DualSite && TestIF_File.iSiteMap[0][1]<=0)
        b1x2SiteAbClosePutDummy=true;
    else
        b1x2SiteAbClosePutDummy=false;

    if(TestIF_File.iTestMode==SingleSite ||
       iInArmType==e9045_1x4_1_Ac        ||
       b1x2SiteAbClosePutDummy==true)
    {
        iCount=1;
    }
    else if(TestIF_File.iTestMode==DualSite     ||
            TestIF_File.iTestMode==DualSite2x1  ||
            TestIF_File.iTestMode==QualSite2X2N)
    {
        iCount=2;
    }
    else if(TestIF_File.iTestMode==TriSite1X3 ||
            TestIF_File.iTestMode==_6Site2X3N)
    {
        if(TestIF_File.iTestMode==TriSite1X3 &&                                 //JerryYang 20260408 : fix 1x3 2吸嘴
           iInArmType==e9045_1x3_2_14 &&
           TestIF_File.iSiteMap[0][1]<=0)
            iCount=2;
        else
            iCount=3;
    }
    else if(TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==QualSite2X2)
    {
        iCount=4;
    }
    else if(iCloseSiteModeFor2x8==e2x8Run2x2_13 ||                              //Steven 20250502 : Fixed for 2x8 auto clean
            iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        iCount=4;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||
            TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_16Site4X4 ||
            TestIF_File.iTestMode==_8Site2X4N ||                                //Wei 20231211 : 2X4NN Mode
            iCloseSiteModeFor2x6>e2x6OneByOne ||
            iCloseSiteModeFor2x8>e2x8OneByOne)
    {
        iCount=8;
    }
    else if(TestIF_File.iTestMode==_12Site2X6)                                  //Steven 20241113 : for 2x6 auto clean
    {
        iCount=12;
    }
    else if(TestIF_File.iTestMode==_10Site2X5  ||
            TestIF_File.iTestMode==_16Site2X8  ||
            TestIF_File.iTestMode==_32Site4X8N)
    {
        iCount=16;
    }
    else
    {
        filerw::ELMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "GetMinCleanPadCount");
    }
    return iCount;

}

// golden AutoClean/uCleaning.cpp:1403  TfCleaning::SetDeviceMaxMin()
static int CL_SetDeviceMaxMin()
{

    AnsiString sBuffet;
    int iMin, iCount;
    int iMax;
    int iCol=0, iRow;

    iMin=CL_GetMinCleanPadCount();                                                 //Steven 20221006 : 計算最少需要的Clean Pad數量

    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Increment=iMin;
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min=iMin;   // golden AutoClean/uCleaning.cpp:1413-1413 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1413-1413 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min=iMin;
#endif // GATE (S12-C save)
    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==0)
    {
        iCol=atoi(EL<TEdit>("TfCleaning", "XCT1")->Text.c_str());
        iRow=atoi(EL<TEdit>("TfCleaning", "YCT1")->Text.c_str());
    }
    else
    {
        iCol=atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());
        iRow=atoi(EL<TEdit>("TfCleaning", "YCT2")->Text.c_str());
    }
    iMax=iCol*iRow;

    if(iMax<iMin)
    {
        if(InitialOK)
        {
    sBuffet.sprintf("Auto Clean X-Division or Y-Division set Count Error : MIN=%d", ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:1430-1430 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1430-1430 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            sBuffet.sprintf("Auto Clean X-Division or Y-Division set Count Error : MIN=%d", EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);
#endif // GATE (S12-C save)
            if(TestIF_File.iAutoClean_Function)
                filerw::ELMessage(sBuffet);
        }
    }
    else
    {
        if(TestIF_File.iTestMode==_12Site2X6)
        {
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=16;   // golden AutoClean/uCleaning.cpp:1439-1439 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1439-1439 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=16;
#endif // GATE (S12-C save)
        }
        else
        {
            iCount=(ChangeToFloatNonPcnt((double)(iMax), (double)(iMin)));
            if(iCount==1 || iCount%2==0)
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=iMin*iCount;   // golden AutoClean/uCleaning.cpp:1445-1445 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1445-1445 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
                EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=iMin*iCount;                                    //指定成最小值的倍數
#endif // GATE (S12-C save)
            else
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=iMin*(iCount-1);   // golden AutoClean/uCleaning.cpp:1447-1447 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1447-1447 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
                EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max=iMin*(iCount-1);                                //不可以是奇數倍,避免異常
#endif // GATE (S12-C save)
        }
    }

    if(atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str())>((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max))   // golden AutoClean/uCleaning.cpp:1451-1451 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1451-1451 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    if(atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str())>udDeviceCT->Max)                       //JerryYang 20180829 (Steven) : 新增clean pad數量保護
#endif // GATE (S12-C save)
    {
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max);   // golden AutoClean/uCleaning.cpp:1453-1453 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1453-1453 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max;
#endif // GATE (S12-C save)
    }

    if(atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str())<((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min))   // golden AutoClean/uCleaning.cpp:1456-1456 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1456-1456 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    if(atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str())<EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min)
#endif // GATE (S12-C save)
    {
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:1458-1458 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1458-1458 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;
#endif // GATE (S12-C save)
    }

    CL_DrawAutoClean();
    return iCount;

}

// golden AutoClean/uCleaning.cpp:1465  TfCleaning::FormShow(TObject *Sender)
static void CL_FormShow()
{

    ;   // golden AutoClean/uCleaning.cpp:1467-1468 取代：表單位置（HTML 端）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1467-1468 -- 表單位置（HTML 端）
    Left=100;                                                                   //Steven 20220407 : 避免跑到畫面外
    Top =10;
#endif // GATE (S12-C save)
    fShow=true;
    try
    {
        EL<TButton>("TfCleaning", "btFocusOnly")->SetFocus();                                                //Ifor 20231030 add: 避免修改設定後重新下載顯示異常
    }
    catch(...){}
    { TComboBox* cb=EL<TComboBox>("TfCleaning", "cbbSelectTray"); cb->Items->Clear(); cb->ItemIndex=-1; cb->Text=""; }   // golden AutoClean/uCleaning.cpp:1475-1475 取代：TComboBox::Clear()：VCL CB_RESETCONTENT（清選項、ItemIndex=-1、Text=""）；vclcompat TComboBox 只是欄位
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1475-1475 -- TComboBox::Clear()：VCL CB_RESETCONTENT（清選項、ItemIndex=-1、Text=""）；vclcompat TComboBox 只是欄位
    EL<TComboBox>("TfCleaning", "cbbSelectTray")->Clear();                                                     //kevin 20201104 清除避免纍 加資料
#endif // GATE (S12-C save)
    filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbSelectTray"), "Select from Database...");                              //kevin 20150521 start
    AnsiString S;
    ;   // golden AutoClean/uCleaning.cpp:1478-1479 取代：視窗標題（HTML 端；且 golden 把 AnsiString 當 %s 的可變參數，BCB 可行、標準 C++ 不行）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1478-1479 -- 視窗標題（HTML 端；且 golden 把 AnsiString 當 %s 的可變參數，BCB 可行、標準 C++ 不行）
    S.sprintf("Tray Form  ''%s''  ", GetLastOpenFN());
    Caption=S;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfCleaning", "grpCleanPara")->Enabled=true;
    EL<TGroupBox>("TfCleaning", "grpContactForce")->Enabled=true;                                              //JerryYang 20220311 : ATP鎖定Critical parameter
    EL<TPanel>("TfCleaning", "pnlContactMode")->Enabled=true;
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->Enabled=true;
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->SetFocus();
    CL_ShowTranGfToN();
    CL_ShowCleanContactForce();

    if(bHasTrayCSV)                                                             //Steven 20210629 : Tray Form改成CSV
    {
        fConfiguration->sbtReloadTray->Click();
        for(int i=0; i<fConfiguration->strngrdTray->RowCount; i++)
        {
            if(fConfiguration->strngrdTray->Cells[0][i]!="" &&
               fConfiguration->strngrdTray->Cells[0][i]!=" ")
            {
                EL<TComboBox>("TfCleaning", "cbbSelectTray")->Items->Add(fConfiguration->strngrdTray->Cells[0][i]);
            }
        }
    }

    EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Visible=(TestIF_File.iTestMode==_12Site2X6);              //Steven 20241113 : for 2x6 auto clean
    EL<TEdit>("TfCleaning", "edDevicePices")->Visible=(TestIF_File.iTestMode!=_12Site2X6);
    EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Visible=(TestIF_File.iTestMode!=_12Site2X6);

    if(CosFunction.bForecePerPinKGf)                                            //JerryYang 20230204 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    {
        EL<TEdit>("TfCleaning", "edPinSingleN")->Enabled=false;
    }

    EL<TPanel>("TfCleaning", "pnlKitSetting")->Enabled      =true;                                          //Steven 20220218 : 修正, auto clean執行中, 不可以修改Kit格式
    EL<TGroupBox>("TfCleaning", "grpCleanPara")->Enabled       =fSecurity->Insufficient(43, false);
    EL<TGroupBox>("TfCleaning", "grpCleanDevice")->Enabled     =fSecurity->Insufficient(97, false);
    EL<TGroupBox>("TfCleaning", "grpSpeed")->Enabled           =fSecurity->Insufficient(43, false);

    if(IniConfig.bVTESTFunction==true)
    {
        EL<TPanel>("TfCleaning", "pnlContactMode")->Enabled =fSecurity->Insufficient(43, false);
        EL<TGroupBox>("TfCleaning", "grpContactForce")->Enabled=fSecurity->Insufficient(43, false);
        EL<TPanel>("TfCleaning", "pnlCleanArm")->Enabled    =fSecurity->Insufficient(43, false);
        EL<TGroupBox>("TfCleaning", "grpHeight")->Enabled      =(AccessLevel>=1);
    }
    else
    {
        EL<TGroupBox>("TfCleaning", "grpContactPara")->Enabled =fSecurity->Insufficient(43, false);
    }

    EL<TGroupBox>("TfCleaning", "grpDieForce")->Visible=(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI);                                //Steven 20240719 : Die force for auto clean   //AI(ht9045-v899) 20260430: extend AutoClean Die Force visibility to Multi EP half.

    EL<TGroupBox>("TfCleaning", "grpTrayData")->Enabled        =fSecurity->Insufficient(43, false);
    EL<TButton>("TfCleaning", "btnResetCleanCount")->Enabled =fSecurity->Insufficient(97, false);
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Visible=(CUSTOMER_CODE==CC_TERAPOWER);              //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    EL<TLabel>("TfCleaning", "lbledIndexArmAutoCleanCnt")->Visible=(CUSTOMER_CODE==CC_TERAPOWER);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       CUSTOMER_CODE==CC_OSE ||                                                 //kevin 20180309 add OSE
       CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                      //kevin 20210929 add 938
    {
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode2")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode3")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Visible=false;
        EL<TPageControl>("TfCleaning", "pcCleanYield")->Visible=false;
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->Enabled=false;
    CL_RadioIndex(EL<TRadioGroup>("TfCleaning", "rgCleanKitType"), 1);   // golden AutoClean/uCleaning.cpp:1542-1542 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1542-1542 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex=1;
#endif // GATE (S12-C save)
        EL<TButton>("TfCleaning", "btInclude")->Visible=false;
        EL<TEdit>("TfCleaning", "PageTypeName")->Visible=true;
        EL<TComboBox>("TfCleaning", "cbbSelectTray")->Visible=true;
        EL<TTabSheet>("TfCleaning", "tsKit")->TabVisible=false;                                                //kevin 20180309 只顯示 tray
        EL<TGroupBox>("TfCleaning", "gbCleanKitOffset")->Visible=true;                                         //kevin 20190211 add  change offset
    }

    if(bRunAutoClean==true)
    {
        EL<TPanel>("TfCleaning", "pnlKitSetting")->Enabled=false;                                           //Steven 20220218 : 修正, auto clean執行中, 不可以修改Kit格式
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->Enabled=false;
        EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Enabled=false;
        EL<TEdit>("TfCleaning", "edAlarmCount")->Enabled=false;
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Enabled=false;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Enabled=false;
        if(bResetCleanCount==true)
        {
            EL<TButton>("TfCleaning", "btnResetCleanCount")->Enabled=true;
        }
        else
        {
            EL<TButton>("TfCleaning", "btnResetCleanCount")->Enabled=false;
        }
    }
    else
    {
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->Enabled       =fSecurity->Insufficient(43, false);
        EL<TGroupBox>("TfCleaning", "grpCleanDevice")->Enabled         =fSecurity->Insufficient(97, false);
        EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Enabled             =fSecurity->Insufficient(43, false);
        EL<TButton>("TfCleaning", "btnResetCleanCount")->Enabled     =fSecurity->Insufficient(97, false);
        EL<TEdit>("TfCleaning", "edAlarmCount")->Enabled           =fSecurity->Insufficient(43, false);
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Enabled       =fSecurity->Insufficient(43, false);
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Enabled   =true;
    }

    if(TestIF.iTestMode==_32Site4X8N)                                           //Steven 20140512: For HT-9047
    {
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Enabled=false;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Enabled=false;
    }
    CL_SetDeviceMaxMin();                                                          //JerryYang 20160513
    CL_LoadAutoCleanData();   // golden AutoClean/uCleaning.cpp:1584-1584 取代：fCleaning->：golden 只有一個 TfCleaning 物件，就是本 TU（移植樹門面 fCleaning 沒有 LoadAutoCleanData）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1584-1584 -- fCleaning->：golden 只有一個 TfCleaning 物件，就是本 TU（移植樹門面 fCleaning 沒有 LoadAutoCleanData）
    fCleaning->LoadAutoCleanData();
#endif // GATE (S12-C save)

    CL_LoadImage();

    EL<TRadioGroup>("TfCleaning", "rgKitPosition")->Visible=false;
    EL<TLabel>("TfCleaning", "Label288")->Visible=false;
    bEnterOffset=true;

    if(CLEAN_AIR)
    {
        EL<TGroupBox>("TfCleaning", "grpTrayData")->Visible=false;
        EL<TGroupBox>("TfCleaning", "grpCleanDevice")->Visible=false;
    }
    else
    {
        EL<TGroupBox>("TfCleaning", "grpTrayData")->Visible=true;
        EL<TGroupBox>("TfCleaning", "grpCleanDevice")->Visible=true;
    }

    if(CosFunction.bAutoCleanAutoSelIndexArm==true)
    {
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->ItemIndex=0;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Visible=true;
        EL<TSpeedButton>("TfCleaning", "sbTrayAssign")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Visible=false;
        EL<TCheckBox>("TfCleaning", "chkCleanOtherArm")->Checked=false;
        EL<TRadioGroup>("TfCleaning", "rgAutoCleanSelectArm")->Visible=true;
        EL<TSpeedButton>("TfCleaning", "sbTrayAssign")->Visible=false;
    }

    if(CosFunction.bAutoCleanShuttleDisable==true)                              //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    {
        EL<TLabel>("TfCleaning", "Label21")->Visible=false;
        EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->Visible=false;
        EL<TTabSheet>("TfCleaning", "tsOffset")->TabVisible=false;
    }
    else
    {
        EL<TLabel>("TfCleaning", "Label21")->Visible=true;
        EL<TRadioGroup>("TfCleaning", "rgShuttleCheck")->Visible=true;
        EL<TTabSheet>("TfCleaning", "tsOffset")->TabVisible=false;
    }

    EL<TGroupBox>("TfCleaning", "gbYieldAlarm")->Visible=true;
    EL<TCheckBox>("TfCleaning", "chkAutoCleanMode5")->Caption="Yield Alarm";

    EL<TCheckBox>("TfCleaning", "chkUseNSKit")->Visible=(IniConfig.bSPILFunction==true);                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    SearchCleanNum();

    if(CUSTOMER_CODE==CC_KYEC_LEE ||                                            //20140320 wei   KYEC 低於權限顯示不能修改
       CUSTOMER_CODE==CC_KYEC_XILINX)
    {
        if(AccessLevel<LevelSet.AccessLevel[43])
        {
            EL<TGroupBox>("TfCleaning", "grpContactPara")->Enabled=false;
            EL<TGroupBox>("TfCleaning", "grpTrayData")->Enabled=false;
        }
        else
        {
            EL<TGroupBox>("TfCleaning", "grpContactPara")->Enabled=true;
            EL<TGroupBox>("TfCleaning", "grpTrayData")->Enabled=true;
        }
    }

    //Eastsun 20260710 Merge - B03 add: KYEC JCTHIU disable Cleaning Parameter during AutoClean
    if(CUSTOMER_CODE==CC_KYEC_JCTHIU && bRunAutoClean==true)
    {
        EL<TGroupBox>("TfCleaning", "grpCleanPara")->Enabled=false;
    }

    EL<TCheckBox>("TfCleaning", "chkE43")->Visible=CosFunction.bAutoCleanUseHPSetByRecipe;                     //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    if(CosFunction.bAutoCleanUseHPSetByRecipe==false)                           //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定)
        EL<TTabSheet>("TfCleaning", "tsKit")->Caption=IniConfig.bE43AutoCleanUseHotplate?"Hot Plate":"Kit";
    CL_ChangeEditToHPMode();                                                       //Steven20160628 : 整合Auto Clean畫面顯示
    EL<TPanel>("TfCleaning", "pnlInArmOffset")->Visible=(IniConfig.bE48_ShuttleUse4Offset_Autoclean==false);//20140923 wei : For Shuttle Auto Clean
    EL<TPanel>("TfCleaning", "pnlNotify")->Visible=(IniConfig.bE48_ShuttleUse4Offset_Autoclean);

    if(CosFunction.bLotStartLockCriticalPara &&                                 //JerryYang 20220311 : ATP鎖定Critical parameter
       RunInfo.bLotStart)
    {
        if(bAuthCriticalPara[25])
        {
            EL<TGroupBox>("TfCleaning", "grpTrayData")->Enabled=false;
            EL<TGroupBox>("TfCleaning", "grpContactForce")->Enabled=false;
            EL<TPanel>("TfCleaning", "pnlContactMode")->Enabled=false;
        }
    }

    if(CosFunction.bSetContactForceToAutoClean==true)                           //Steven 20250915 : 複製Contact Force給Auto Clean用
    {
        EL<TGroupBox>("TfCleaning", "grpContactForce")->Enabled=false;
    }

    if(CosFunction.bCanChangeAutoCleanCount==true &&                            //Ifor 20171024 : add 可更改Auto Clean 計數
       bRunAutoClean==false)                                                    //Steven 20220105 : 執行中不可以修改Clean Count
    {
        EL<TEdit>("TfCleaning", "edCleaningCount")->Enabled=true;
    }
    EL<TTabSheet>("TfCleaning", "tsSmart")->TabVisible=CosFunction.bSmartAutoClean;                            //Sam 20230111 : Smart Auto Clean

    EL<TTabSheet>("TfCleaning", "tsTimeCT")->Visible=CosFunction.bAutoCleanTimeCT;                             //jou 20250102 : auto clean triger time count
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden AutoClean/uCleaning.cpp:1693  TfCleaning::ShowTranGfToN()
static void CL_ShowTranGfToN()
{

    double f=atof(EL<TEdit>("TfCleaning", "edPinSingleGf")->Text.c_str());
    f=f*9.8/1000.0;
    AnsiString str;
    str.sprintf("%8.4f", f);
    EL<TEdit>("TfCleaning", "edPinSingleN")->Text=str.Trim();

}

// golden AutoClean/uCleaning.cpp:1709  TfCleaning::ShowTranNToGf()
static void CL_ShowTranNToGf()
{

    if(CosFunction.bForecePerPinKGf==false)                                     //JerryYang 20230204 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    {
        double f=atof(EL<TEdit>("TfCleaning", "edPinSingleN")->Text.c_str());
        f=f*1000.0/9.8;
        AnsiString str="";
        str.sprintf("%8.4f", f);
        EL<TEdit>("TfCleaning", "edPinSingleGf")->Text=str.Trim();
    }
    CL_ShowCleanContactForce();

}

// golden AutoClean/uCleaning.cpp:1735  TfCleaning::ShowCleanContactForce()
static void CL_ShowCleanContactForce()
{

    double iBallCount;
    double iTotalN;
    double iTotalGfAC;

    double iDeviceN;
    double iDeviceGf;

    double iSingleN;
    double iSingleGf;
    FileRW_Cleaning_ContactDutCount();   // golden AutoClean/uCleaning.cpp:1746-1746 取代：移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單 fContactForm（forms/fContact.h）→ 轉接（同 TestIF_File_SetUp）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1746-1746 -- 移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單 fContactForm（forms/fContact.h）→ 轉接（同 TestIF_File_SetUp）
    fContact->DutCount();
#endif // GATE (S12-C save)
    iBallCount =atof(EL<TEdit>("TfCleaning", "edPinsCount")->Text.c_str());
    iSingleN   =atof(EL<TEdit>("TfCleaning", "edPinSingleN")->Text.c_str());
    iSingleGf  =atof(EL<TEdit>("TfCleaning", "edPinSingleGf")->Text.c_str());

    iDeviceN   =iBallCount*iSingleN;                                            //device要壓的牛頓
    iDeviceGf  =iBallCount*iSingleGf/1000.0;                                    //device要壓的公斤

    iTotalN=iDeviceN*FileRW_Cleaning_ContactDutCountValue(); iTotalGfAC=iDeviceGf*FileRW_Cleaning_ContactDutCountValue();   // golden AutoClean/uCleaning.cpp:1754-1755 取代：同上：fContactForm->dDutCount
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1754-1755 -- 同上：fContactForm->dDutCount
    iTotalN   =iDeviceN  *fContact->dDutCount;                                  //1個arm要壓的牛頓
    iTotalGfAC=iDeviceGf *fContact->dDutCount;                                  //1個arm要壓的公斤
#endif // GATE (S12-C save)

    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Color=clWhite;                                      //Ifor 20190506 :TStaticText  ==>  TEdit
    double dMaxLimit=FileRW_Cleaning_ContactMaxIndexForceLimit();   // golden AutoClean/uCleaning.cpp:1758-1758 取代：TfContact::GetMaxIndexForceLimit（golden cContact.cpp:19184）：移植樹 fContactForm 只有宣告（GATE X-30）→ 轉接到 cContact.h ComputeMaxIndexForceLimit（逐行同 golden 912），rgKitDiameter 取 C 路 TfContact 替身
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1758-1758 -- TfContact::GetMaxIndexForceLimit（golden cContact.cpp:19184）：移植樹 fContactForm 只有宣告（GATE X-30）→ 轉接到 cContact.h ComputeMaxIndexForceLimit（逐行同 golden 912），rgKitDiameter 取 C 路 TfContact 替身
    double dMaxLimit=fContact->GetMaxIndexForceLimit();                         //JimmyChiu 20220119 獨立計算空氣壓力
#endif // GATE (S12-C save)
    if(dMaxLimit>0 && iTotalGfAC>dMaxLimit)
    {
        ;   // UI-only（HTML 端處理）：EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Color=clRed;                                    //Ifor 20190506 :TStaticText  ==>  TEdit
        iTotalGfAC=dMaxLimit;
    }

    if(IniConfig.bSPILFunction==true ||
       CUSTOMER_CODE==CC_SCS)
    {
        if(iTotalGfAC<16)
        {
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Color=cl3DLight;                            //Ifor 20190506 :TStaticText  ==>  TEdit
            iTotalGfAC=16;
        }
    }
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_Kg")->Text=FormatFloat("0.00", iTotalGfAC);               //Ifor 20190506 :TStaticText  ==>  TEdit
    EL<TEdit>("TfCleaning", "edAutoCleanAirForce_N")->Text =FormatFloat("0.00", iTotalN);                  //Ifor 20190506 :TStaticText  ==>  TEdit

}

// golden AutoClean/uCleaning.cpp:1778  TfCleaning::sbCleanSaveClick(TObject *Sender)
static void CL_sbCleanSaveClick()
{
    filerw::ELMark("sbCleanSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden AutoClean/uCleaning.cpp:1784-1784 取代：Close()：A02 權限不足 golden 關表單 → 記 closed（SaveFlow 照 VCL 順序接 FormClose）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1784-1784 -- Close()：A02 權限不足 golden 關表單 → 記 closed（SaveFlow 照 VCL 順序接 FormClose）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    bool bFix3HasBin=false;                                                     //Steven 20120210 Start: Fix3有Bin不能做Auto Clean

//    if(fCleaning->rgAutoCleanOnOff->ItemIndex==1 &&                           //Steven 20241111 : Mark for 1x4 run 3 col tray auto clean  //Ifor 20180227 (Steven) : add 1*4 模式無法使用 3*x Tray from
//       TestIF.iTestMode==QualSite1X4 &&
//       iInArmType!=e9045_1x4_1_Ac &&                                            //Steven 20200720 : 1x4只開site Ac
//       fCleaning->rgCleanKitType->ItemIndex==1)
//    {
//        if(TrayForm.Loader.XDivision==3)                                       //Steven 20220927 : <4 --> ==3
//        {
//            ShowMyMessage("This Tray form is not supported, Please use the Clean Kit", "Cannot enable the auto clean function.");
//            return;
//        }
//    }

    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==1)                                            //Jimmychiu 20221027 add only Tray mode
    {
        AnsiString sBuffet="";
        int iXCT2=atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());
        int iYCT2=atoi(EL<TEdit>("TfCleaning", "YCT2")->Text.c_str());
    if((iXCT2*iYCT2)<((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min))   // golden AutoClean/uCleaning.cpp:1807-1807 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1807-1807 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        if((iXCT2*iYCT2)<EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min)
#endif // GATE (S12-C save)
        {
    sBuffet.sprintf("Auto Clean X-Division or Y-Division set Count Error : Count=%d", ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:1809-1809 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1809-1809 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            sBuffet.sprintf("Auto Clean X-Division or Y-Division set Count Error : Count=%d", EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   //kevin 20220906
#endif // GATE (S12-C save)
    if(fShow)   // golden AutoClean/uCleaning.cpp:1810-1810 取代：fCleaning->fShow：本 TU 的 fShow（golden 只有一個表單物件）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1810-1810 -- fCleaning->fShow：本 TU 的 fShow（golden 只有一個表單物件）
            if(fCleaning->fShow)
#endif // GATE (S12-C save)
                filerw::ELMessage(sBuffet);

            return;
        }
    }

    if(EL<TRadioGroup>("TfCleaning", "rgKitPosition")->ItemIndex==0)                                             //Fix 3
    {
        for(int i=0; i<15; i++)
        {
            if((Prod.iT6CatData[i]==eFix3 ||
                Prod.iT6CatData[i]==eFix6))                                     //放在FIX 3
                bFix3HasBin=true;
        }

        if(Prod.iIfErrorT6==eFix3 ||
           Prod.iIfErrorT6==eFix6)                                              //放在FIX 3
            bFix3HasBin=true;

        if(bFix3HasBin==true && EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex==1)
        {
            filerw::ELMessage("Fix 3 already has bin!!", "Cannot enable the auto clean function.");
            EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->ItemIndex=0;
        }
    }

    CL_SaveAutoCleanData();
    SearchCleanNum();                                                           //Steven 20180524 : Fixed for clean count

    EL<TSpeedButton>("TfCleaning", "sbCleanSave")->Down=false;

    InitialSet();                                                               //kevin 20150720

    if(CUSTOMER_CODE==CC_ASE_KaohSiung) filerw::ELTodo("golden main.cpp:31331 fMain->bEnableAutoclean() (CC_ASE_KaohSiung main-screen AutoClean button) -- TfMain facade has none");   // golden AutoClean/uCleaning.cpp:1844-1844 取代：同 LoadAutoCleanData：CC_ASE_KaohSiung 客戶專屬
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1844-1844 -- 同 LoadAutoCleanData：CC_ASE_KaohSiung 客戶專屬
    fMain->bEnableAutoclean();                                                  //kevin 20150521 autoclean
#endif // GATE (S12-C save)

    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
    }
    CL_DrawAutoClean();                                                            //JerryYang 20160308 更新AutoClean畫框

    filerw::ELTodo("golden AutoClean/uCleaning.cpp:1852-1877 CosFunction.bRecipeParameterDefault：fRPDefault／fSpeed->SearchRecipeParameter／FTestIF／fYieldMonitoring 走訪表單元件樹（PCtrl->Controls[]），替身沒有樹；RPDefault 未移植（GATE G-CL-RPD）");
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1852-1877 -- CosFunction.bRecipeParameterDefault：fRPDefault／fSpeed->SearchRecipeParameter／FTestIF／fYieldMonitoring 走訪表單元件樹（PCtrl->Controls[]），替身沒有樹；RPDefault 未移植（GATE G-CL-RPD）
    if(CosFunction.bRecipeParameterDefault)                                     //Isaac 20170527 (Steven) defalut值比較功能
    {
        fRPDefault->RP_Name->Clear();
        fRPDefault->RP_Default->Clear();
        fRPDefault->RP_Value->Clear();

        if(CosFunction.bRecipeParameterDefaultLog)                              //Sam 20201209 : Default Recipe ChangeLog
        {
            fSpeed->SearchRecipeParameter(fSpeed->tsAllSpeed, -1);              //Isaac 20171227 (Steven) : AutoSpeed RPD一鍵復原
            fSpeed->SearchRecipeParameter(fSpeed->tsIndex, 0);
            fSpeed->SearchRecipeParameter(fSpeed->tsInArm, 1);
            fSpeed->SearchRecipeParameter(fSpeed->tsOutArm, 2);
            fSpeed->SearchRecipeParameter(fSpeed->tsTrayArm, 3);
            fSpeed->SearchRecipeParameter(fSpeed->tsShuttle, 4);
        }

        SearchRecipeParameter(fCleaning, 0);

        if(CosFunction.bRecipeParameterDefaultLog)                              //Sam 20201209 : Default Recipe ChangeLog
        {
            FTestIF->SearchRecipeParameter(FTestIF, 0);
            fYieldMonitoring->SearchRecipeParameter(fYieldMonitoring,0);
        }

        fRPDefault->CompareRPDefaultAndValue(fRPDefault->RP_Name, fRPDefault->RP_Default, fRPDefault->RP_Value);
    }
#endif // GATE (S12-C save)
    CL_LoadAutoCleanData();
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    filerw::ELTodo("golden AutoClean/uCleaning.cpp:1880-1883 CC_GIGAS fRPDefault->ShowMonitoredParameter()：客戶專屬、RPDefault 未移植");
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1880-1883 -- CC_GIGAS fRPDefault->ShowMonitoredParameter()：客戶專屬、RPDefault 未移植
    if(CUSTOMER_CODE==CC_GIGAS)
    {
        fRPDefault->ShowMonitoredParameter();                                   //Isaac 20210222 : 全智要求把參數拉出來看，方便監控
    }
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:1886  TfCleaning::sbCleanExitClick(TObject *Sender)
static void CL_sbCleanExitClick()
{
    filerw::ELMark("sbCleanExitClick");   // FileRW：存檔流程 trace（ack.trace）

    filerw::ELMark("closed");   // golden AutoClean/uCleaning.cpp:1888-1888 取代：Close()：golden ShowModal 表單設 ModalResult，處理器回傳後 modal 迴圈跑 FormClose → 記 closed（SaveFlow 接 FormClose）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1888-1888 -- Close()：golden ShowModal 表單設 ModalResult，處理器回傳後 modal 迴圈跑 FormClose → 記 closed（SaveFlow 接 FormClose）
    Close();
#endif // GATE (S12-C save)
    EL<TSpeedButton>("TfCleaning", "sbCleanExit")->Down=false;

}

// golden AutoClean/uCleaning.cpp:1904  TfCleaning::rgKitPositionClick(TObject *Sender)
static void CL_rgKitPositionClick()
{

    CL_SetArmCaption();

}

// golden AutoClean/uCleaning.cpp:1909  TfCleaning::YCT1Change(TObject *Sender)
static void CL_YCT1Change(TObject *Sender)
{

    TEdit *ptr=(TEdit *)Sender;

    if(IniConfig.bE43AutoCleanUseHotplate==false)
    {
        if(atoi(ptr->Text.c_str())>10)                                          //kevin 20210911 change 4->10
    CL_EditText(ptr, 10);   // golden AutoClean/uCleaning.cpp:1916-1916 取代：(1) TEdit OnChange（ptr＝Sender）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1916-1916 -- (1) TEdit OnChange（ptr＝Sender）
            ptr->Text=10;
#endif // GATE (S12-C save)
    }
//    SetDeviceMaxMin();                                                        //Jimmychiu 20250121 : 調換位置，避免換工單檢測錯誤

}

// golden AutoClean/uCleaning.cpp:1923  TfCleaning::DrawAutoClean()
static void CL_DrawAutoClean()
{

    iDeviceCount=atoi(EL<TEdit>("TfCleaning", "edDevicePices")->Text.c_str());                             //RogerYang 20260102 : 搬到show之前，避免iDeviceCount未更新
    if(TestIF_File.iTestMode==_12Site2X6)
        iDeviceCount=atoi(EL<TComboBox>("TfCleaning", "cbbCleanPadCount")->Text.c_str());                      //Steven 20241113 : for 2x6 auto clean

    if(fShow==false)
        return;

    ;   // golden AutoClean/uCleaning.cpp:1932-1949 取代：tmyAutoClean（TTMyTray）的預覽圖：純畫面。golden SetDeviceInTray（:1952）用到 V912 的 bCleanKitAxxG3ColUse2Col（golden AutoClean.cpp:300），移植樹沒有；移植樹 AutoClean.cpp:3409 的自由函式版是 906 版，也不借用
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1932-1949 -- tmyAutoClean（TTMyTray）的預覽圖：純畫面。golden SetDeviceInTray（:1952）用到 V912 的 bCleanKitAxxG3ColUse2Col（golden AutoClean.cpp:300），移植樹沒有；移植樹 AutoClean.cpp:3409 的自由函式版是 906 版，也不借用
    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==0 || IniConfig.bE43AutoCleanUseHotplate)
    {
        EL<TControl>("TfCleaning", "tmyAutoClean")->XItem=atoi(EL<TEdit>("TfCleaning", "XCT1")->Text.c_str());
        EL<TControl>("TfCleaning", "tmyAutoClean")->YItem=atoi(EL<TEdit>("TfCleaning", "YCT1")->Text.c_str());
    }
    else
    {
        EL<TControl>("TfCleaning", "tmyAutoClean")->XItem=atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());
        EL<TControl>("TfCleaning", "tmyAutoClean")->YItem=atoi(EL<TEdit>("TfCleaning", "YCT2")->Text.c_str());
    }

    EL<TControl>("TfCleaning", "tmyAutoClean")->SetColorMap(HAS_CLEAN_IC, clInactiveCaption);
    int iXItem=TestIF.iAutoClean_XDivision;
    int iYItem=EL<TControl>("TfCleaning", "tmyAutoClean")->YItem;
//    iDeviceCount=atoi(edDevicePices->Text.c_str());
//    if(TestIF_File.iTestMode==_12Site2X6)
//        iDeviceCount=atoi(cbbCleanPadCount->Text.c_str());                      //Steven 20241113 : for 2x6 auto clean
    SetDeviceInTray(iXItem, iYItem, iDeviceCount, eUcleanUsed);                 //Jimmychiu 20221027 優化Cleanpad配置方式
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2106  TfCleaning::FormClose(TObject *Sender, TCloseAction &Action)
static void CL_FormClose()
{
    filerw::ELMark("FormClose");   // FileRW：存檔流程 trace（ack.trace）

    SetWorkParameter();
    EL<TRadioGroup>("TfCleaning", "rgAutoCleanOnOff")->Enabled=false;
    fShow=false;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden AutoClean/uCleaning.cpp:2140  TfCleaning::XCT1Change(TObject *Sender)
static void CL_XCT1Change(TObject *Sender)
{

    int iData1=atoi(EL<TEdit>("TfCleaning", "XCT1")->Text.c_str());
    int iData2=atoi(EL<TEdit>("TfCleaning", "XCT2")->Text.c_str());

    if(EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex==1)                                         //Steven 20220128 : Fixed for Auto Clean X Item亂跳
    {
//        if(TestIF_File.iTestMode==TriSite1X3 ||                               //Steven 20240416 : 修改1x3使用3ea Clean Pad
//           TestIF_File.iTestMode==_6Site2X3N)
//        {
//            iCount=3;
//        }
//        else if(TestIF_File.iTestMode==_6Site2X3)                               //Steven 20240416 : 修改2x3使用6ea Clean Pad
//        {
//            iCount=6;
//        }
//        else if(TestIF_File.iTestMode==_10Site2X5 &&                            //Steven 20240416 : 修改2x5使用10ea Clean Pad
//                b12SiteRun2x4==false)
//        {
//            iCount=10;
//        }

        if(TestIF_File.iTestMode==_12Site2X6 &&                                 //Steven 20250809 : 修改2x6使用12ea Clean Pad
           b12SiteRun2x4==false &&
           (iData2==6 || iData2==8 || iData2==4 || iData2==12))
        {
            ;
        }
        else if(iData2<8 && iData2>=4)                                          //Alick 20170329 (wei) modify 原本用字串比對數字有錯
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), 4);   // golden AutoClean/uCleaning.cpp:2170-2170 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2170-2170 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT2")->Text=4;
#endif // GATE (S12-C save)
        }
        else if(iData2%4==0)                                                    //kevin 20220906
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), (iData2/4)*4);   // golden AutoClean/uCleaning.cpp:2174-2174 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2174-2174 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT2")->Text=(iData2/4)*4;
#endif // GATE (S12-C save)
        }
        else
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), UserDefForm_File[TrayForm.Loader.iTrayType].XDivision);   // golden AutoClean/uCleaning.cpp:2178-2178 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2178-2178 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT2")->Text=UserDefForm_File[TrayForm.Loader.iTrayType].XDivision;
#endif // GATE (S12-C save)
        }
    }
    else
    {
        if(iData1<8 &&
           iData1>=4)                                                           //Alick 20170329 (wei) modify 原本用字串比對數字有錯
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT1"), 4);   // golden AutoClean/uCleaning.cpp:2186-2186 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2186-2186 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT1")->Text=4;
#endif // GATE (S12-C save)
        }
        else if(TestIF_File.iTestMode==_10Site2X5 &&                            //Steven 20220125 : for 2x5 mode
                iData1>=10)
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT1"), 10);   // golden AutoClean/uCleaning.cpp:2191-2191 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2191-2191 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT1")->Text=10;
#endif // GATE (S12-C save)
        }
        else if(TestIF_File.iTestMode==_12Site2X6 &&                            //Steven 20250809 : 修改2x6使用12ea Clean Pad
                b12SiteRun2x4==false &&
                (iData1==6 || iData1==8 || iData1==4 || iData1==12))
        {
            ;
        }
        else if((TestIF_File.iTestMode==_16Site2X8 ||
                 TestIF_File.iTestMode==_32Site4X8N) &&
                iData1==12)                                                     //Steven 20250912 : for 16site auto clean with 12x16 Hot plate
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT1"), 12);   // golden AutoClean/uCleaning.cpp:2203-2203 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2203-2203 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT1")->Text=12;
#endif // GATE (S12-C save)
        }
        else if(iData1>=8)
        {
    CL_EditText(EL<TEdit>("TfCleaning", "XCT1"), 8);   // golden AutoClean/uCleaning.cpp:2207-2207 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2207-2207 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
            EL<TEdit>("TfCleaning", "XCT1")->Text=8;
#endif // GATE (S12-C save)
        }
    }

//    SetDeviceMaxMin();                                                        //Jimmychiu 20250121 : 調換位置，避免換工單檢測錯誤

}

// golden AutoClean/uCleaning.cpp:2214  TfCleaning::rgCleanKitTypeClick(TObject *Sender)
static void CL_rgCleanKitTypeClick()
{

    //==> Eastsun 20260526 #026-4.2 Ifor 20240614/20240618 :KYEC 切換模式需刷 Barcode 與輸入帳密
    static bool bNeedPW=false;
    static int iLastCleanKitType=0;   //Eastsun 20260526 R4-fix : remember last kit type for cancel

#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2220-2249 -- CC_KYEC_LEE 切換 Kit 型式要刷條碼＋重新登入（fMain->cbUserSelect／fFTPClient）：客戶專屬，先跳過
    if(CUSTOMER_CODE==CC_KYEC_LEE && InitialOK && fFTPClient->bShow==false)
    {
        if(bNeedPW==true)
        {
            bNeedPW=false;
            return;
        }
        ReEnterBarcode[0]=false;
        fMain->cbUserSelect->ItemIndex=0;
        AccessLevel=0;
        fMain->ChangeLevelAttr();
        fMain->cbUserSelect->Text="Operator";
        fMain->spbUserName->Caption="Operator";
        fMain->btLogin->Caption="Login";
        if(FileExists(pwPath))
        {
            fMain->cbUserSelectChange(NULL);
        }
        else
        {
            fMain->stOperatorClick(fMain);
        }

        if(AccessLevel<iDefSupervisorLevel)
        {
            bNeedPW=true;
            EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex=iLastCleanKitType;
            return;
        }
    }
#endif // GATE (S12-C save)
    iLastCleanKitType=EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex;
    //<== Eastsun 20260526 #026-4.2
    if(EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->ItemIndex==0)
    {
        EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex=0;
    }
    else
    {
        EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex=1;
    }
    CL_SetDeviceMaxMin();

}

// golden AutoClean/uCleaning.cpp:2317  TfCleaning::XPitch2Change(TObject *Sender)
static void CL_XPitch2Change(TObject *Sender)
{

    CL_DrawAutoClean();                                                            //jou 2015-03-24 Tray x pitch 變化

}

// golden AutoClean/uCleaning.cpp:2359  TfCleaning::ChangeEditToHPMode()
static void CL_ChangeEditToHPMode()
{

    EL<TEdit>("TfCleaning", "XCT1")->Enabled   =true;                                                      //Ifor 20191119 : add 避免非Auto Clean 執行狀態下被鎖定無法修改
    EL<TEdit>("TfCleaning", "YCT1")->Enabled   =true;
    EL<TEdit>("TfCleaning", "XCT2")->Enabled   =true;
    EL<TEdit>("TfCleaning", "YCT2")->Enabled   =true;

    if(IniConfig.bE43AutoCleanUseHotplate)                                      //ChungHung 20131120 AutoClean use Hotplate1 start
    {
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->Visible=false;
        EL<TEdit>("TfCleaning", "XCT1")->Enabled   =false;
        EL<TEdit>("TfCleaning", "XPitch1")->Enabled=false;
        EL<TEdit>("TfCleaning", "XST1")->Enabled   =false;

        EL<TEdit>("TfCleaning", "YCT1")->Enabled   =fSecurity->Insufficient(43, false);
        EL<TEdit>("TfCleaning", "YPitch1")->Enabled=false;
        EL<TEdit>("TfCleaning", "YST1")->Enabled   =false;

        EL<TEdit>("TfCleaning", "XCT2")->Enabled   =false;
        EL<TEdit>("TfCleaning", "XPitch2")->Enabled=false;
        EL<TEdit>("TfCleaning", "XST2")->Enabled   =false;

        EL<TEdit>("TfCleaning", "YCT2")->Enabled   =true;
        EL<TEdit>("TfCleaning", "YPitch2")->Enabled=false;
        EL<TEdit>("TfCleaning", "YST2")->Enabled   =false;
        EL<TTabSheet>("TfCleaning", "tabTray")->TabVisible=false;
        EL<TPageControl>("TfCleaning", "pgCleanType")->ActivePageIndex=0;
    }
    else
    {
        EL<TRadioGroup>("TfCleaning", "rgCleanKitType")->Visible=true;
        EL<TEdit>("TfCleaning", "XCT1")->Enabled   =fSecurity->Insufficient(43, false);
        EL<TEdit>("TfCleaning", "XPitch1")->Enabled=true;
        EL<TEdit>("TfCleaning", "XST1")->Enabled   =true;

        EL<TEdit>("TfCleaning", "YCT1")->Enabled   =fSecurity->Insufficient(43, false);
        EL<TEdit>("TfCleaning", "YPitch1")->Enabled=true;
        EL<TEdit>("TfCleaning", "YST1")->Enabled   =true;
        EL<TTabSheet>("TfCleaning", "tabTray")->TabVisible=true;
    }

    if(bRunAutoClean==true)
    {
        EL<TEdit>("TfCleaning", "XCT1")->Enabled   =false;
        EL<TEdit>("TfCleaning", "YCT1")->Enabled   =false;
        EL<TEdit>("TfCleaning", "XCT2")->Enabled   =false;
        EL<TEdit>("TfCleaning", "YCT2")->Enabled   =false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150904
    {
        EL<TEdit>("TfCleaning", "XPitch2")->Enabled=false;
        EL<TEdit>("TfCleaning", "YPitch2")->Enabled=false;
        EL<TEdit>("TfCleaning", "XST2")->Enabled   =false;
        EL<TEdit>("TfCleaning", "YST2")->Enabled   =false;
    }

}

// golden AutoClean/uCleaning.cpp:2886  TfCleaning::cbbCleanPadCountChange(TObject *Sender)
static void CL_cbbCleanPadCountChange()
{

    CL_DrawAutoClean();

}

// golden AutoClean/uCleaning.cpp:1702  TfCleaning::edPinSingleGfClick(TObject *Sender)
static void CL_edPinSingleGfClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, InputLimit.dForcePerpinHigh, InputLimit.dForcePerpinLow);   // golden AutoClean/uCleaning.cpp:1704-1704 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1704-1704 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, InputLimit.dForcePerpinHigh, InputLimit.dForcePerpinLow);
#endif // GATE (S12-C save)
    CL_ShowCleanContactForce();
    CL_ShowTranGfToN();

}

// golden AutoClean/uCleaning.cpp:1722  TfCleaning::edPinSingleNClick(TObject *Sender)
static void CL_edPinSingleNClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, InputLimit.dForcePerpinNHigh, InputLimit.dForcePerpinNLow);   // golden AutoClean/uCleaning.cpp:1724-1724 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1724-1724 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, InputLimit.dForcePerpinNHigh, InputLimit.dForcePerpinNLow);
#endif // GATE (S12-C save)
    CL_ShowCleanContactForce();
    CL_ShowTranNToGf();

}

// golden AutoClean/uCleaning.cpp:1729  TfCleaning::edPinsCountClick(TObject *Sender)
static void CL_edPinsCountClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 20000, 1);   // golden AutoClean/uCleaning.cpp:1731-1731 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:1731-1731 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 20000, 1);
#endif // GATE (S12-C save)
    CL_ShowCleanContactForce();

}

// golden AutoClean/uCleaning.cpp:2101  TfCleaning::edCleanCountClick(TObject *Sender)
static void CL_edCleanCountClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, (atoi(EL<TEdit>("TfCleaning", "edAlarmCount")->Text.c_str()))-1, 0);   // golden AutoClean/uCleaning.cpp:2103-2103 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2103-2103 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, (atoi(EL<TEdit>("TfCleaning", "edAlarmCount")->Text.c_str()))-1, 0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2595  TfCleaning::edtBufferKitLTXClick(TObject *Sender)
static void CL_edtBufferKitLTXClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetXYHigh, (double)InputLimit.iOffsetXYLow);   // golden AutoClean/uCleaning.cpp:2597-2597 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2597-2597 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetXYHigh, (double)InputLimit.iOffsetXYLow);       //Steven 20141120 : Modify
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2600  TfCleaning::edtHotplatePickOffsetClick(TObject *Sender)
static void CL_edtHotplatePickOffsetClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);   // golden AutoClean/uCleaning.cpp:2602-2602 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2602-2602 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);       //Steven 20141120 : Modify
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2605  TfCleaning::edACContactCleanHeightClick(TObject *Sender)
static void CL_edACContactCleanHeightClick(TObject *Sender)
{

    if(IniConfig.bSPILFunction)                                                 //JerryYang 20240111 : 矽品二林要求設定auto clean contact height上下限
    {
    CL_QwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, InputLimit.dAutoCleanContactHigh, InputLimit.dAutoCleanContactLow);   // golden AutoClean/uCleaning.cpp:2609-2609 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2609-2609 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, InputLimit.dAutoCleanContactHigh, InputLimit.dAutoCleanContactLow);
#endif // GATE (S12-C save)
    }
    else
    {
    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);   // golden AutoClean/uCleaning.cpp:2613-2613 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2613-2613 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);
#endif // GATE (S12-C save)
    }

}

// golden AutoClean/uCleaning.cpp:2617  TfCleaning::edtLowYieldLimitClick(TObject *Sender)
static void CL_edtLowYieldLimitClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100, 1);   // golden AutoClean/uCleaning.cpp:2619-2619 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2619-2619 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2622  TfCleaning::edtLowYieldCountClick(TObject *Sender)
static void CL_edtLowYieldCountClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100000, 1);   // golden AutoClean/uCleaning.cpp:2624-2624 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2624-2624 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100000, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2627  TfCleaning::XST1Click(TObject *Sender)
static void CL_XST1Click(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.00, 1000.00);   // golden AutoClean/uCleaning.cpp:2629-2629 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2629-2629 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.00, 1000.00);       //Steven 20250812 : min 0.00
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2632  TfCleaning::XCT1Click(TObject *Sender)
static void CL_XCT1Click(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 100, 1);   // golden AutoClean/uCleaning.cpp:2634-2634 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2634-2634 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 100, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2637  TfCleaning::edACContactShiftHeightClick(TObject *Sender)
static void CL_edACContactShiftHeightClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.01, 30.0);   // golden AutoClean/uCleaning.cpp:2639-2639 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2639-2639 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.01, 30.0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2642  TfCleaning::edDropOffset1Click(TObject *Sender)
static void CL_edDropOffset1Click(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_DOUBLE, 1, true, 10.0, 0.0);   // golden AutoClean/uCleaning.cpp:2644-2644 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2644-2644 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 1, true, 10.0, 0.0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2647  TfCleaning::edDevicePicesClick(TObject *Sender)
static void CL_edDevicePicesClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max), ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:2649-2649 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2649-2649 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Max, EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2652  TfCleaning::edAutoCleanAirForce_KgClick(TObject *Sender)
static void CL_edAutoCleanAirForce_KgClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 500.0, 0.0);   // golden AutoClean/uCleaning.cpp:2654-2654 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2654-2654 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 500.0, 0.0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2657  TfCleaning::edAutoCleanAirForce_NClick(TObject *Sender)
static void CL_edAutoCleanAirForce_NClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 5000.0, 0.0);   // golden AutoClean/uCleaning.cpp:2659-2659 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2659-2659 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 5000.0, 0.0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2662  TfCleaning::edtACSmartClick(TObject *Sender)
static void CL_edtACSmartClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 10, 1);   // golden AutoClean/uCleaning.cpp:2664-2664 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2664-2664 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 10, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2891  TfCleaning::edTimeCTClick(TObject *Sender)
static void CL_edTimeCTClick(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, 1, 5000, 5);   // golden AutoClean/uCleaning.cpp:2893-2893 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2893-2893 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 5000, 5);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2896  TfCleaning::edtIndexVacuumClick(TObject *Sender)
static void CL_edtIndexVacuumClick(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 10.0, 0.01);   // golden AutoClean/uCleaning.cpp:2898-2898 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2898-2898 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 10.0, 0.01);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2849  TfCleaning::edAdaptiveIntervalMaxMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edAdaptiveIntervalMaxMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 10);   // golden AutoClean/uCleaning.cpp:2852-2852 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2852-2852 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 10);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2855  TfCleaning::edAdaptiveIntervalMinMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edAdaptiveIntervalMinMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 1);   // golden AutoClean/uCleaning.cpp:2858-2858 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2858-2858 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2861  TfCleaning::edAdaptiveIntervalAdjMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edAdaptiveIntervalAdjMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 1000, 1);   // golden AutoClean/uCleaning.cpp:2864-2864 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2864-2864 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 1000, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2867  TfCleaning::edACSmartCTFMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edACSmartCTFMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, 1, 10, 1);   // golden AutoClean/uCleaning.cpp:2870-2870 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2870-2870 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 10, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2873  TfCleaning::edtACSmart_ContactTimeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edtACSmart_ContactTimeMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, 0.01, 100.0, 0.0);   // golden AutoClean/uCleaning.cpp:2877-2877 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2877-2877 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 0.01, 100.0, 0.0);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2880  TfCleaning::edtACSmartMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
static void CL_edtACSmartMouseDown(TObject *Sender)
{

    CL_QwertyKey((TEdit*)Sender, 1, 10, 1);   // golden AutoClean/uCleaning.cpp:2883-2883 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2883-2883 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 10, 1);
#endif // GATE (S12-C save)

}

// golden AutoClean/uCleaning.cpp:2322  TfCleaning::cbbSelectTrayChange(TObject *Sender)
static void CL_cbbSelectTrayChange()
{

    int iCount=0, iX=0, iB=0;//, iY=0;
    int index=EL<TComboBox>("TfCleaning", "cbbSelectTray")->ItemIndex;
    if(bHasTrayCSV)                                                             //Steven 20210810 : Tray Form改成CSV
    {
        if(index<1)
            return;

        fConfiguration->sbtReloadTray->Click();
        EL<TEdit>("TfCleaning", "PageTypeName")->Text  =fConfiguration->strngrdTray->Cells[0][index].Trim();           //"Package Type"
        EL<TEdit>("TfCleaning", "XST2")->Text          =fConfiguration->strngrdTray->Cells[1][index].Trim();           //"X Start Pos"
        EL<TEdit>("TfCleaning", "YST2")->Text          =fConfiguration->strngrdTray->Cells[2][index].Trim();           //"Y Start Pos"
    CL_EditText(EL<TEdit>("TfCleaning", "XPitch2"), fConfiguration->strngrdTray->Cells[3][index].Trim());   // golden AutoClean/uCleaning.cpp:2335-2335 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2335-2335 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "XPitch2")->Text       =fConfiguration->strngrdTray->Cells[3][index].Trim();           //"X Pitch"
#endif // GATE (S12-C save)
        EL<TEdit>("TfCleaning", "YPitch2")->Text       =fConfiguration->strngrdTray->Cells[4][index].Trim();           //"Y Pitch"
    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), fConfiguration->strngrdTray->Cells[5][index].Trim());   // golden AutoClean/uCleaning.cpp:2337-2337 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2337-2337 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "XCT2")->Text          =fConfiguration->strngrdTray->Cells[5][index].Trim();           //"Columns (X)"
#endif // GATE (S12-C save)
//        YCT2->Text          =fConfiguration->strngrdTray->Cells[6][index].Trim();           //"Rows (Y)"
        iX                  =atoi(fConfiguration->strngrdTray->Cells[5][index].c_str());    //"Columns (X)
//        iY                  =atoi(fConfiguration->strngrdTray->Cells[6][index].c_str());    //"Rows (Y)")
    }

    iCount=CL_SetDeviceMaxMin();                                                   //wei 20150422 載入Tray時，顯示最少Pad數量
    if(TestIF_File.iTestMode==_12Site2X6)
    filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:2345-2345 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2345-2345 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);                                 //Steven 20241113 : for 2x6 auto clean
#endif // GATE (S12-C save)
    else
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:2347-2347 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2347-2347 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;                                    //wei 20150422 載入Tray時，顯示最少Pad數量
#endif // GATE (S12-C save)
    if((iX*2) >=iCount)
    {
    CL_EditText(EL<TEdit>("TfCleaning", "YCT2"), 2);   // golden AutoClean/uCleaning.cpp:2350-2350 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2350-2350 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "YCT2")->Text=2;
#endif // GATE (S12-C save)
    }
    else
    {
        iB=ChangeToFloatNonPcnt((double)(iCount), (double)(iX));
    CL_EditText(EL<TEdit>("TfCleaning", "YCT2"), iB);   // golden AutoClean/uCleaning.cpp:2355-2355 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2355-2355 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "YCT2")->Text=iB;
#endif // GATE (S12-C save)
    }

}

// golden AutoClean/uCleaning.cpp:2269  TfCleaning::btIncludeClick(TObject *Sender)
static void CL_btIncludeClick()
{

    EL<TEdit>("TfCleaning", "XST2")->Text      = GetFloatFormatString(TrayForm.Loader.XStart/100, 3, 3);
    CL_EditText(EL<TEdit>("TfCleaning", "XPitch2"), GetFloatFormatString(TrayForm.Loader.XPitch/100, 3, 3));   // golden AutoClean/uCleaning.cpp:2272-2272 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2272-2272 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "XPitch2")->Text   = GetFloatFormatString(TrayForm.Loader.XPitch/100, 3, 3);
#endif // GATE (S12-C save)
    CL_EditText(EL<TEdit>("TfCleaning", "XCT2"), AnsiString(TrayForm.Loader.XDivision));   // golden AutoClean/uCleaning.cpp:2273-2273 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2273-2273 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "XCT2")->Text      = AnsiString(TrayForm.Loader.XDivision);
#endif // GATE (S12-C save)
    EL<TEdit>("TfCleaning", "YST2")->Text      = GetFloatFormatString(TrayForm.Loader.YStart/100, 3, 3);
    EL<TEdit>("TfCleaning", "YPitch2")->Text   = GetFloatFormatString(TrayForm.Loader.YPitch/100, 3, 3);
    CL_EditText(EL<TEdit>("TfCleaning", "YCT2"), 2);   // golden AutoClean/uCleaning.cpp:2276-2276 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2276-2276 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
    EL<TEdit>("TfCleaning", "YCT2")->Text      = 2;                                                        //JerryYang 20160811 Clean tray Y數量預設1->2
#endif // GATE (S12-C save)
    CL_SetDeviceMaxMin();                                                          //wei 20150422 載入Tray時，顯示最少Pad數量
    if(TestIF_File.iTestMode==_12Site2X6)
    filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), ((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min));   // golden AutoClean/uCleaning.cpp:2279-2279 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2279-2279 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);                                 //Steven 20241113 : for 2x6 auto clean
#endif // GATE (S12-C save)
    else
    EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min);   // golden AutoClean/uCleaning.cpp:2281-2281 取代：自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2281-2281 -- 自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）
        EL<TEdit>("TfCleaning", "edDevicePices")->Text=EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")->Min;                                    //wei 20150422 載入Tray時，顯示最少Pad數量
#endif // GATE (S12-C save)
    CL_DrawAutoClean();                                                            //wei 20150422 載入Tray時，顯示最少Pad數量

}

// golden AutoClean/uCleaning.cpp:2116  TfCleaning::btnResetCleanCountClick(TObject *Sender)
static void CL_btnResetCleanCountClick()
{

    if(Barcode_Reader(bcAutoClean)==0)                                          // 20140103 wei KYEC Barcode Reader
    {
        return;
    }

    ReadWriteAutoCleanCount(false, true);                                       //Steven 20190523 : 只要按下Reset就先把Clean Count存檔
    fShowBinSelect->ed_AutoCleanCount->Text=0;
    fCleaning->edCleaningCount->Text=0;
    EL<TButton>("TfCleaning", "btnResetCleanCount")->Enabled=false;
    bResetCleanCount=false;
    SearchCleanNum();                                                           //Steven 20180524 : Fixed for clean count

    if(iAutoCleanAlarm==1)                                                      //Ifor 20180727 : 需有Alarm 才可修改變數避免發生異常
        iAutoCleanAlarm=2;                                                      //Ifor 20180727 ：Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
    SetAutoCleanICCount(false);                                                 //kevin 20150616
    iAutoCleanAlarm=0;                                                          //Ifor 20180727 ：Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
    TestIF_File.iIndexArmAutoCleanCnt=0;                                        //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    EL<TEdit>("TfCleaning", "edIndexArmAutoCleanCnt")->Text=0;   // golden AutoClean/uCleaning.cpp:2135-2135 取代：fCleaning->edIndexArmAutoCleanCnt：golden 只有一個 TfCleaning 物件，就是本 TU；移植樹門面 forms/fCleaning.h 沒有這個元件 → 本 TU 的替身（頁面上同一格；CC_TERAPOWER 才顯示，golden FormShow :1531）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2135-2135 -- fCleaning->edIndexArmAutoCleanCnt：golden 只有一個 TfCleaning 物件，就是本 TU；移植樹門面 forms/fCleaning.h 沒有這個元件 → 本 TU 的替身（頁面上同一格；CC_TERAPOWER 才顯示，golden FormShow :1531）
    fCleaning->edIndexArmAutoCleanCnt->Text=0;
#endif // GATE (S12-C save)
    EventReport(SECS_EVENT.AutoCleanClearCount);                                //wei 20150627
    bErrorAutoClean=false;                                                      //kevin 20200527 add clean

}

// golden AutoClean/uCleaning.cpp:2842  TfCleaning::btnResetIntervalClick(TObject *Sender)
static void CL_btnResetIntervalClick()
{

    //iAdaptiveACInterval=-1;
    CL_ChangeACSmartInterval(0, "btnResetIntervalClick");                          //Sam 20240726 : AI Clean
    fLotInfo->RefreshYieldMonitor();

}

// golden AutoClean/uCleaning.cpp:2779  TfCleaning::ChangeACSmartInterval(int iChangeMode, AnsiString sMsg)
static void CL_ChangeACSmartInterval(int iChangeMode, AnsiString sMsg)
{

    if(CosFunction.bSmartAutoClean && TestIF.bACSmart)
    {
        if(iChangeMode==0)                                                      //清空
        {
            iAdaptiveACInterval=-1;
        }
        else if(iChangeMode==1)                                                 //微調+
        {
            if(iAdaptiveACInterval==TestIF.iAdaptiveACIntervalMin)              //表示良率剛從 Min 回到到 Normal 要回復之前的 AdaptiveACInterval
            {
                iAdaptiveACInterval=iAdaptiveACIntervalNor;
            }
            else
            {
                iAdaptiveACInterval+=TestIF.iAdaptiveACIntervalAdj;
                if(iAdaptiveACInterval>TestIF.iAdaptiveACIntervalMax)
                     iAdaptiveACInterval=TestIF.iAdaptiveACIntervalMax;
            }
        }
        else if(iChangeMode==99)                                                //升到最大
        {
            iAdaptiveACInterval=TestIF.iAdaptiveACIntervalMax;
        }
        else if(iChangeMode==-1)                                                //微調-
        {
            iAdaptiveACInterval-=TestIF.iAdaptiveACIntervalAdj;
            if(iAdaptiveACInterval<TestIF.iAdaptiveACIntervalMin)
                 iAdaptiveACInterval=TestIF.iAdaptiveACIntervalMin;
        }
        else if(iChangeMode==-99)                                               //降到最低
        {
            if(iAdaptiveACInterval!=TestIF.iAdaptiveACIntervalMin)              //表示良率剛從 Normal 掉到 Min 要記錄當下的 AdaptiveACInterval
                iAdaptiveACIntervalNor=iAdaptiveACInterval;

            iAdaptiveACInterval=TestIF.iAdaptiveACIntervalMin;
        }
        else if(iChangeMode==2)                                                 //恢復到標準值
        {
            iAdaptiveACInterval=TestIF_File.iAutoClean_IntervalContact;
        }

        AnsiString sLog="", sFile="", sFileName="", S="";                       //紀錄 AC Smart Interval change log
        sFile.sprintf("%s\\%04d%02d\\", asACSmartIntervalLogPath, SystemYear, SystemMonth); //kevin 20150908
        S=GetLastOpenFN();
        if(DirectoryExists(sFile)==false)
        {
            ForceDirectories(sFile);
        }
        sFileName.sprintf("%s%s_%s.csv", sFile, Now().FormatString("yyyy-mm-dd"), S);
        sLog.sprintf(" %s, %d, %s", Now().FormatString("hh:nn:ss"), iAdaptiveACInterval, sMsg);
        WriteDataToFile(sFileName, sLog);

        AnsiString szDir="";
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
        TestIF.iAutoClean_IntervalContact=iAdaptiveACInterval;
        TestIF_File.iAutoClean_IntervalContact=iAdaptiveACInterval;
    EL<TEdit>("TfCleaning", "edIntervalContact")->Text=iAdaptiveACInterval;   // golden AutoClean/uCleaning.cpp:2837-2837 取代：fCleaning->edIntervalContact：同上（移植樹門面沒有這個元件 → 本 TU 的替身）
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2837-2837 -- fCleaning->edIntervalContact：同上（移植樹門面沒有這個元件 → 本 TU 的替身）
        fCleaning->edIntervalContact->Text=iAdaptiveACInterval;
#endif // GATE (S12-C save)
        WriteIniData(szDir, "Configuration", "iAutoClean_IntervalContact", TestIF.iAutoClean_IntervalContact);
    }

}

// golden AutoClean/uCleaning.cpp:2311  TfCleaning::sbTrayAssignClick(TObject *Sender)
static void CL_sbTrayAssignClick()
{

    filerw::ELTodo("main:AutoCleanStringGrid golden uCleaning.cpp:2313 fMain->MainFormSizeToEpson(false) not ported (main.cpp:8966: main page -> Motion View, resize, SECS EnterDebug) -- main-screen part is batch B6; :2314 AutoCleanStringGrid->Visible=true is set on the TfMain facade");   // golden AutoClean/uCleaning.cpp:2313-2313 取代：fMain->MainFormSizeToEpson(false)（golden main.cpp:8966：主畫面 pgMain 換到 tsMotionView、調視窗大小、SECS/GEM 開著時送 EnterDebug 事件…）：移植樹 TfMain 門面沒有；主畫面那一半是 B6（main.html），只記 todo。下一行 AutoCleanStringGrid->Visible=true 照 golden 寫門面
#if 0 // GATE (S12-C save) golden AutoClean/uCleaning.cpp:2313-2313 -- fMain->MainFormSizeToEpson(false)（golden main.cpp:8966：主畫面 pgMain 換到 tsMotionView、調視窗大小、SECS/GEM 開著時送 EnterDebug 事件…）：移植樹 TfMain 門面沒有；主畫面那一半是 B6（main.html），只記 todo。下一行 AutoCleanStringGrid->Visible=true 照 golden 寫門面
    fMain->MainFormSizeToEpson(false);
#endif // GATE (S12-C save)
    fMain->AutoCleanStringGrid->Visible=true;

}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/editlist/TestIF_File_Cleaning.py 的 events；
//   FileRW/TestIF_File_Cleaning.cpp 用 filerw::PageEventsRegistrar 註冊；本體 FileRW/_EditPage.cpp RunPageEvent）
static void CL_Ev_cbbSelectTrayChange(TControl* Sender) { (void)Sender; CL_cbbSelectTrayChange(); }
static void CL_Ev_btIncludeClick(TControl* Sender) { (void)Sender; CL_btIncludeClick(); }
static void CL_Ev_btnResetCleanCountClick(TControl* Sender) { (void)Sender; CL_btnResetCleanCountClick(); }
static void CL_Ev_btnResetIntervalClick(TControl* Sender) { (void)Sender; CL_btnResetIntervalClick(); }
static void CL_Ev_sbTrayAssignClick(TControl* Sender) { (void)Sender; CL_sbTrayAssignClick(); }
static void CL_Ev_rgCleanKitTypeClick(TControl* Sender) { (void)Sender; CL_rgCleanKitTypeClick(); }
static void CL_Ev_edtBufferKitLTXClick(TControl* Sender) { CL_edtBufferKitLTXClick(Sender); }
static void CL_Ev_edtLowYieldCountClick(TControl* Sender) { CL_edtLowYieldCountClick(Sender); }
static void CL_Ev_edtLowYieldLimitClick(TControl* Sender) { CL_edtLowYieldLimitClick(Sender); }
static void CL_Ev_edtACSmart_ContactTimeMouseDown(TControl* Sender) { CL_edtACSmart_ContactTimeMouseDown(Sender); }
static void CL_Ev_edAdaptiveIntervalAdjMouseDown(TControl* Sender) { CL_edAdaptiveIntervalAdjMouseDown(Sender); }
static void CL_Ev_edAdaptiveIntervalMinMouseDown(TControl* Sender) { CL_edAdaptiveIntervalMinMouseDown(Sender); }
static void CL_Ev_edAdaptiveIntervalMaxMouseDown(TControl* Sender) { CL_edAdaptiveIntervalMaxMouseDown(Sender); }
static void CL_Ev_edACSmartCTFMouseDown(TControl* Sender) { CL_edACSmartCTFMouseDown(Sender); }
static void CL_Ev_edtACSmartMouseDown(TControl* Sender) { CL_edtACSmartMouseDown(Sender); }
static void CL_Ev_edTimeCTClick(TControl* Sender) { CL_edTimeCTClick(Sender); }
static void CL_Ev_XST1Click(TControl* Sender) { CL_XST1Click(Sender); }
static void CL_Ev_XCT1Click(TControl* Sender) { CL_XCT1Click(Sender); }
static void CL_Ev_edDevicePicesClick(TControl* Sender) { CL_edDevicePicesClick(Sender); }
static void CL_Ev_edCleanCountClick(TControl* Sender) { CL_edCleanCountClick(Sender); }
static void CL_Ev_edDropOffset1Click(TControl* Sender) { CL_edDropOffset1Click(Sender); }
static void CL_Ev_edtIndexVacuumClick(TControl* Sender) { CL_edtIndexVacuumClick(Sender); }
static void CL_Ev_edtHotplatePickOffsetClick(TControl* Sender) { CL_edtHotplatePickOffsetClick(Sender); }
static void CL_Ev_edPinSingleNClick(TControl* Sender) { CL_edPinSingleNClick(Sender); }
static void CL_Ev_edPinSingleGfClick(TControl* Sender) { CL_edPinSingleGfClick(Sender); }
static void CL_Ev_edPinsCountClick(TControl* Sender) { CL_edPinsCountClick(Sender); }
static void CL_Ev_edAutoCleanAirForce_KgClick(TControl* Sender) { CL_edAutoCleanAirForce_KgClick(Sender); }
static void CL_Ev_edAutoCleanAirForce_NClick(TControl* Sender) { CL_edAutoCleanAirForce_NClick(Sender); }
static void CL_Ev_edACContactShiftHeightClick(TControl* Sender) { CL_edACContactShiftHeightClick(Sender); }
static void CL_Ev_edACContactCleanHeightClick(TControl* Sender) { CL_edACContactCleanHeightClick(Sender); }
static const filerw::PageEvent kCL_Events[] = {
    {"cbbSelectTray", "change", "AutoClean/uCleaning.cpp:2322 TfCleaning::cbbSelectTrayChange", &CL_Ev_cbbSelectTrayChange},
    {"btInclude", "click", "AutoClean/uCleaning.cpp:2269 TfCleaning::btIncludeClick", &CL_Ev_btIncludeClick},
    {"btnResetCleanCount", "click", "AutoClean/uCleaning.cpp:2116 TfCleaning::btnResetCleanCountClick", &CL_Ev_btnResetCleanCountClick},
    {"btnResetInterval", "click", "AutoClean/uCleaning.cpp:2842 TfCleaning::btnResetIntervalClick", &CL_Ev_btnResetIntervalClick},
    {"sbTrayAssign", "click", "AutoClean/uCleaning.cpp:2311 TfCleaning::sbTrayAssignClick", &CL_Ev_sbTrayAssignClick},
    {"rgCleanKitType", "click", "AutoClean/uCleaning.cpp:2214 TfCleaning::rgCleanKitTypeClick", &CL_Ev_rgCleanKitTypeClick},
    {"edtBufferKitLTX", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitLTY", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitRTX", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitRTY", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitLDX", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitLDY", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitRDX", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtBufferKitRDY", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtFailAlarmSiteYieldDifferent", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtFailAlarmSiteYield", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edtLowYieldLimit", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edtLowYieldCount", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtConseFailureCountBySocket_Retest", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtConseFailureCountBySocket_Normal", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtConseFailureCountByHead_Retest", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtConseFailureCountByHead_Normal", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edtACSmart_ContactTime", "click", "AutoClean/uCleaning.cpp:2873 TfCleaning::edtACSmart_ContactTimeMouseDown", &CL_Ev_edtACSmart_ContactTimeMouseDown},
    {"edtACSmart_ACContactCount", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edAdaptiveIntervalAdj", "click", "AutoClean/uCleaning.cpp:2861 TfCleaning::edAdaptiveIntervalAdjMouseDown", &CL_Ev_edAdaptiveIntervalAdjMouseDown},
    {"edAdaptiveIntervalMin", "click", "AutoClean/uCleaning.cpp:2855 TfCleaning::edAdaptiveIntervalMinMouseDown", &CL_Ev_edAdaptiveIntervalMinMouseDown},
    {"edAdaptiveIntervalMax", "click", "AutoClean/uCleaning.cpp:2849 TfCleaning::edAdaptiveIntervalMaxMouseDown", &CL_Ev_edAdaptiveIntervalMaxMouseDown},
    {"edACSmartCTF", "click", "AutoClean/uCleaning.cpp:2867 TfCleaning::edACSmartCTFMouseDown", &CL_Ev_edACSmartCTFMouseDown},
    {"edtACSmart", "click", "AutoClean/uCleaning.cpp:2880 TfCleaning::edtACSmartMouseDown", &CL_Ev_edtACSmartMouseDown},
    {"edTimeCT", "click", "AutoClean/uCleaning.cpp:2891 TfCleaning::edTimeCTClick", &CL_Ev_edTimeCTClick},
    {"XST1", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"XPitch1", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"XCT1", "click", "AutoClean/uCleaning.cpp:2632 TfCleaning::XCT1Click", &CL_Ev_XCT1Click},
    {"YST1", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"YPitch1", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"YCT1", "click", "AutoClean/uCleaning.cpp:2632 TfCleaning::XCT1Click", &CL_Ev_XCT1Click},
    {"XPitch2", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"XST2", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"XCT2", "click", "AutoClean/uCleaning.cpp:2632 TfCleaning::XCT1Click", &CL_Ev_XCT1Click},
    {"YST2", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"YPitch2", "click", "AutoClean/uCleaning.cpp:2627 TfCleaning::XST1Click", &CL_Ev_XST1Click},
    {"YCT2", "click", "AutoClean/uCleaning.cpp:2632 TfCleaning::XCT1Click", &CL_Ev_XCT1Click},
    {"edIntervalContact", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edDevicePices", "click", "AutoClean/uCleaning.cpp:2647 TfCleaning::edDevicePicesClick", &CL_Ev_edDevicePicesClick},
    {"edAlarmCount", "click", "AutoClean/uCleaning.cpp:2622 TfCleaning::edtLowYieldCountClick", &CL_Ev_edtLowYieldCountClick},
    {"edCleaningCount", "click", "AutoClean/uCleaning.cpp:2101 TfCleaning::edCleanCountClick", &CL_Ev_edCleanCountClick},
    {"edCleanPadDeviation", "click", "AutoClean/uCleaning.cpp:2642 TfCleaning::edDropOffset1Click", &CL_Ev_edDropOffset1Click},
    {"edIndexArmAutoCleanCnt", "click", "AutoClean/uCleaning.cpp:2101 TfCleaning::edCleanCountClick", &CL_Ev_edCleanCountClick},
    {"OutArmSpeed", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"ShuttleSpeed", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"IndexArmSpeed", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edtInArmZSpeed", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edtIndexVacuum", "click", "AutoClean/uCleaning.cpp:2896 TfCleaning::edtIndexVacuumClick", &CL_Ev_edtIndexVacuumClick},
    {"edtInArmVacuum", "click", "AutoClean/uCleaning.cpp:2896 TfCleaning::edtIndexVacuumClick", &CL_Ev_edtIndexVacuumClick},
    {"edtIndexAirOn", "click", "AutoClean/uCleaning.cpp:2896 TfCleaning::edtIndexVacuumClick", &CL_Ev_edtIndexVacuumClick},
    {"edtInArmAirOn", "click", "AutoClean/uCleaning.cpp:2896 TfCleaning::edtIndexVacuumClick", &CL_Ev_edtIndexVacuumClick},
    {"edtHotplateXOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtHotplateYOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtHotplatePlaceOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtHotplatePickOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtHotplatePitchOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edPinSingleN", "click", "AutoClean/uCleaning.cpp:1722 TfCleaning::edPinSingleNClick", &CL_Ev_edPinSingleNClick},
    {"edPinSingleGf", "click", "AutoClean/uCleaning.cpp:1702 TfCleaning::edPinSingleGfClick", &CL_Ev_edPinSingleGfClick},
    {"edPinsCount", "click", "AutoClean/uCleaning.cpp:1729 TfCleaning::edPinsCountClick", &CL_Ev_edPinsCountClick},
    {"edAutoCleanAirForce_Kg", "click", "AutoClean/uCleaning.cpp:2652 TfCleaning::edAutoCleanAirForce_KgClick", &CL_Ev_edAutoCleanAirForce_KgClick},
    {"edAutoCleanAirForce_N", "click", "AutoClean/uCleaning.cpp:2657 TfCleaning::edAutoCleanAirForce_NClick", &CL_Ev_edAutoCleanAirForce_NClick},
    {"edtAutoCleanDieForce", "click", "AutoClean/uCleaning.cpp:2652 TfCleaning::edAutoCleanAirForce_KgClick", &CL_Ev_edAutoCleanAirForce_KgClick},
    {"edACContactShiftHeight", "click", "AutoClean/uCleaning.cpp:2637 TfCleaning::edACContactShiftHeightClick", &CL_Ev_edACContactShiftHeightClick},
    {"edACContactCleanHeight", "click", "AutoClean/uCleaning.cpp:2605 TfCleaning::edACContactCleanHeightClick", &CL_Ev_edACContactCleanHeightClick},
    {"edIndexPickOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edShuttle1PickOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtShuttle1PlaceOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtShuttle1XOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtShuttle1YOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edShuttle2PickOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtShuttle2PlaceOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edtShuttle2XOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtShuttle2YOffset", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edtShuttlePitch", "click", "AutoClean/uCleaning.cpp:2595 TfCleaning::edtBufferKitLTXClick", &CL_Ev_edtBufferKitLTXClick},
    {"edIndexReleaseOffset", "click", "AutoClean/uCleaning.cpp:2600 TfCleaning::edtHotplatePickOffsetClick", &CL_Ev_edtHotplatePickOffsetClick},
    {"edContactTime", "click", "AutoClean/uCleaning.cpp:2642 TfCleaning::edDropOffset1Click", &CL_Ev_edDropOffset1Click},
    {"edACContactCount", "click", "AutoClean/uCleaning.cpp:2617 TfCleaning::edtLowYieldLimitClick", &CL_Ev_edtLowYieldLimitClick},
    {"edDropOffset1", "click", "AutoClean/uCleaning.cpp:2642 TfCleaning::edDropOffset1Click", &CL_Ev_edDropOffset1Click},
};

