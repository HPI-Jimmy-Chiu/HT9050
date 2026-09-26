// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_YieldMonitoring 的讀寫檔（HTEditList： → Tester.Data）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden uYieldMonitoring.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 305 個（header 宣告 432 個）。
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
#include "forms/fLotInfo.h"
#include "csystem.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h；cAuthority.h 與 HTEditList.h 衝突）
// golden uYieldMonitoring.cpp:74 TMyYieldPanel 的資料半邊（ASE_KaohSiung ART）。GUI（TTMyTray256 格子、PalYield、laARTLinit）沒有移植；
// ReTestLimit 是具名替身 "ReTestLimit<Name>"（golden :249 Name 規則），Items 1..10、Text "1"（golden :250-270）。
struct YM_TMyYieldPanel {
    bool bEnablePassYieldART=false; double fPassYieldART=0; bool bEnableOpenShortART=false; double fOpenShortYieldART=0;
    bool bEnableRecoverART=false; double fRecoverYieldART=0; bool bPass[TEST_MAX_BIN]={}; bool bOpenShort[TEST_MAX_BIN]={};
    TComboBox* ReTestLimit;
    explicit YM_TMyYieldPanel(const char* Name) {
        AnsiString str; str.sprintf("ReTestLimit%s", Name);
        ReTestLimit=filerw::EL<TComboBox>("TfYieldMonitoring", str.c_str());
        ReTestLimit->Text="1";
        for(int i=1; i<=10; i++) ReTestLimit->Items->Add(IntToStr(i));
    }
};

// golden uYieldMonitoring.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bFirstCount=true;            // golden uYieldMonitoring.h:595
static bool bYieldCountCheckTick=false;  // golden uYieldMonitoring.h:539
static unsigned long ulYieldFailBase=0;  // golden uYieldMonitoring.h:540
static bool bAskYieldPassword=false;     // golden uYieldMonitoring.h:535
static bool bYieldAlarmUnlocked=false;   // golden uYieldMonitoring.h:536（伺服器端沒有 Unlock 密碼框 → 永遠 false）
static int iMinYield=0, iMaxYield=100, iMinCount=1, iMaxCount=100000;   // golden uYieldMonitoring.h:542-547（建構子 :296-301 設）
static double dMinYield=0.0, dMaxYield=100.0;
static bool fShow=false;                 // golden uYieldMonitoring.h:596
static bool bGetGPIBAutoSiteOff=false;   // golden uYieldMonitoring.h:612
static std::vector<YM_TMyYieldPanel*> MyYieldPanel;   // golden uYieldMonitoring.cpp:98（ASE ART 面板；資料半邊）
static TCheckBox *cbByBinSiteGapCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TCheckBox *cbByBinSiteGapCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByBinSiteGapCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByBinSiteGapCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TCheckBox *cbByArmSiteGapCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TCheckBox *cbByArmSiteGapCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByArmSiteGapCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByArmSiteGapCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TCheckBox *cbByBinFailureCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TCheckBox *cbByBinFailureCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByBinFailureCat_FT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）
static TEdit *edByBinFailureCat_RT[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）

using filerw::EL;

// 前置宣告
static void YM_TfYieldMonitoring();
static void YM_FormCreate();
static void YM_FormShow();
static void YM_ReadFile();
static void YM_DoIniDataToForm();
static void YM_SetClosedSiteBin();
static void YM_CheckSettingNo();
static void YM_FormClose();
static void YM_btnApplyClick();
static void YM_DoFormToData();
static void YM_SaveSetupFile(AnsiString szDir, AnsiString S);

// golden uYieldMonitoring.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void YM_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfYieldMonitoring", "cbUseFTFailYieldModel")->Items;
      s->Add(">=");
      s->Add(">");
      s->Add("<");
      s->Add("<=");
    }
    { TStringList* s = EL<TComboBox>("TfYieldMonitoring", "cbUseRTFailYieldModel")->Items;
      s->Add(">=");
      s->Add(">");
      s->Add("<");
      s->Add("<=");
    }
    { TStringList* s = EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
    }
    { TStringList* s = EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
      s->Add("10");
      s->Add("11");
      s->Add("12");
      s->Add("13");
      s->Add("14");
      s->Add("15");
    }
    { TStringList* s = EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
      s->Add("10");
      s->Add("11");
      s->Add("12");
      s->Add("13");
      s->Add("14");
      s->Add("15");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgAutoCleanYieldOnOff")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOffWhenRT")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOnByGPIB")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgCloseSiteOnHPAction")->Items;
      s->Add("Keep Testing");
      s->Add("Don't Testing, place to error bin");
      s->Add("Don't Testing, place to setted bin.");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgFT_ART")->Items;
      s->Add("Pass yield rate                                               %");
      s->Add("Pass yield rate                     ～                     %");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")->Items;
      s->Add("Alarm");
      s->Add("Index Check");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")->Items;
      s->Add("Alarm");
      s->Add("Index Check");
    }
    { TStringList* s = EL<TRadioGroup>("TfYieldMonitoring", "rgRT_ART")->Items;
      s->Add("Pass yield rate                                               %");
      s->Add("Pass yield rate                     ～                     %");
    }
}

// golden uYieldMonitoring.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kYM_ParentOf[][2] = {
    {"TabSheet1", "pgcBySiteByBinPercentCompare_FT"},
    {"TabSheet2", "pgcBySiteByBinPercentCompare_RT"},
    {"btnApply", "pnlBottom"},
    {"btnUnlockYieldAlarm", "pnlFT_Left"},
    {"btnUnlockYieldAlarmRT", "pnlRT_Left"},
    {"cbAdaptiveLowYield_FT", "grpAdaptiveYield_FT"},
    {"cbAdaptiveLowYield_RT", "grpAdaptiveYield_RT"},
    {"cbAlarm4ContinueType", "grpAlarm4_ContinueType"},
    {"cbAlarm5_BySiteAlarmYieldEnable", "grpAlarm5_BySiteAlarmYield"},
    {"cbAlarm5_BySiteCmpYieldEnable", "grpAlarm5_BySiteCmpYield"},
    {"cbAlarm5_BySiteLowYieldEnable", "grpAlarm5_BySiteLowYield"},
    {"cbAlarm5_BySitePreCmpYieldEnable", "grpAlarm5_BySitePreCmpYieldEnable"},
    {"cbAllSiteFail", "pnlAlarmFT"},
    {"cbAllSiteFail_RT", "gbAlarmRT"},
    {"cbBybinLimitCat0_FT", "TabSheet1"},
    {"cbBybinLimitCat0_RT", "TabSheet2"},
    {"cbBybinLimitCat10_FT", "TabSheet1"},
    {"cbBybinLimitCat10_RT", "TabSheet2"},
    {"cbBybinLimitCat11_FT", "TabSheet1"},
    {"cbBybinLimitCat11_RT", "TabSheet2"},
    {"cbBybinLimitCat12_FT", "TabSheet1"},
    {"cbBybinLimitCat12_RT", "TabSheet2"},
    {"cbBybinLimitCat13_FT", "TabSheet1"},
    {"cbBybinLimitCat13_RT", "TabSheet2"},
    {"cbBybinLimitCat14_FT", "TabSheet1"},
    {"cbBybinLimitCat14_RT", "TabSheet2"},
    {"cbBybinLimitCat15_FT", "TabSheet1"},
    {"cbBybinLimitCat15_RT", "TabSheet2"},
    {"cbBybinLimitCat1_FT", "TabSheet1"},
    {"cbBybinLimitCat1_RT", "TabSheet2"},
    {"cbBybinLimitCat2_FT", "TabSheet1"},
    {"cbBybinLimitCat2_RT", "TabSheet2"},
    {"cbBybinLimitCat3_FT", "TabSheet1"},
    {"cbBybinLimitCat3_RT", "TabSheet2"},
    {"cbBybinLimitCat4_FT", "TabSheet1"},
    {"cbBybinLimitCat4_RT", "TabSheet2"},
    {"cbBybinLimitCat5_FT", "TabSheet1"},
    {"cbBybinLimitCat5_RT", "TabSheet2"},
    {"cbBybinLimitCat6_FT", "TabSheet1"},
    {"cbBybinLimitCat6_RT", "TabSheet2"},
    {"cbBybinLimitCat7_FT", "TabSheet1"},
    {"cbBybinLimitCat7_RT", "TabSheet2"},
    {"cbBybinLimitCat8_FT", "TabSheet1"},
    {"cbBybinLimitCat8_RT", "TabSheet2"},
    {"cbBybinLimitCat9_FT", "TabSheet1"},
    {"cbBybinLimitCat9_RT", "TabSheet2"},
    {"cbContactCountFT", "palContactCount_FT"},
    {"cbContactCountRT", "palContactCount_RT"},
    {"cbContinuPassSkt_FT", "palContinuPassSkt_FT"},
    {"cbContinuPassSkt_RT", "palContinuPassSkt_RT"},
    {"cbContinuousLoad_FT", "palContinuousLoader_FT"},
    {"cbContinuousLoad_RT", "palContinuousLoader_RT"},
    {"cbContinuousPass_FT", "palContinuousPass_FT"},
    {"cbContinuousPass_RT", "palContinuousPass_RT"},
    {"cbContsFailIgnore_FT", "palConsFailIgnore_FT"},
    {"cbContsFailIgnore_RT", "palConsFailIgnore_RT"},
    {"cbIntervalLowYieldBySite_FT", "palIntervalLowYeildBySite_FT"},
    {"cbIntervalLowYieldBySite_RT", "palIntervalLowYeildBySite_RT"},
    {"cbIntervalLowYieldByTotal_FT", "palIntervalLowYeildByTotal_FT"},
    {"cbIntervalLowYieldByTotal_RT", "palIntervalLowYeildByTotal_RT"},
    {"cbLoadCellMeasure", "gbLoadcell"},
    {"cbLowYieldByTotal_FT", "grpLowYieldByTotal_FT"},
    {"cbLowYieldByTotal_RT", "grpLowYieldByTotal_RT"},
    {"cbLowYield_FT", "grpLowYield_FT"},
    {"cbLowYield_RT", "grpLowYield_RT"},
    {"cbLowYield_Special", "grpSpecialLowYield"},
    {"cbSiteYieldCmp_FT", "grpSiteYieldCmp_FT"},
    {"cbSiteYieldCmp_RT", "grpSiteYieldCmp_RT"},
    {"cbSiteYieldDifferent_FT", "grpSiteYieldDifferent_FT"},
    {"cbSiteYieldDifferent_RT", "grpSiteYieldDifferent_RT"},
    {"cbSlidingWindowYield", "pnlChackInterval"},
    {"cbUseFTFailYieldModel", "palART_FT"},
    {"cbUseRTFailYieldModel", "palART_RT"},
    {"cbYieldAlarmByBin", "pnlChackInterval"},
    {"cb_Alarm4EnableIntervalYield", "grpAlarm4_EnableIntervalYield"},
    {"cb_HeadToHeadYieldEnable", "grpAlarm4_HeadToHeadYieldEnable"},
    {"cb_SiteToSiteYieldEnable", "grAlarm4_SiteToSiteYieldEnable"},
    {"cb_SiteYieldOverAlert", "grpAlarm4_SiteYieldOverAlert"},
    {"cbbClosedSiteBin", "gbAutoSiteOff"},
    {"chkAutoSiteOff", "grpSiteYieldCmp_FT"},
    {"chkAutoSiteOffAlarm", "gbAutoSiteOff"},
    {"chkAutoSiteOffByArmBySite", "grpSiteYieldDifferent_FT"},
    {"chkAutoSiteOffByArmContiFail", "gbContsFailByHead_FT"},
    {"chkAutoSiteOffByContiFail", "gbContsFailBySocket_FT"},
    {"chkCreateManualEOCAP", "grpCreateManualEOCAP"},
    {"chkLowYieldByPicker_FT", "grpLowYieldByPicker_FT"},
    {"chkLowYieldByPicker_RT", "grpLowYieldByPicker_RT"},
    {"chkSpcBinCntAsContinue_FT", "gbContsFailBySocket_FT"},
    {"chkSpcBinCntAsContinue_RT", "gbContsFailBySocket_RT"},
    {"chkchkAutoSiteOffByPicker", "grpLowYieldByPicker_FT"},
    {"ckUseFailNoDistinction", "gbA10"},
    {"ckUseLeastRetestTimes", "gbA10"},
    {"cobContinuousPass_FT", "palContinuousPass_FT"},
    {"cobContinuousPass_RT", "palContinuousPass_RT"},
    {"eCloseSite", "gbLowYieldAutoClean"},
    {"edAdaptiveYieldMax_FT", "grpAdaptiveYield_FT"},
    {"edAdaptiveYieldMax_RT", "grpAdaptiveYield_RT"},
    {"edAdaptiveYieldMin_FT", "grpAdaptiveYield_FT"},
    {"edAdaptiveYieldMin_RT", "grpAdaptiveYield_RT"},
    {"edAlarm4ContinueCount", "grpAlarm4_ContinueType"},
    {"edAlarm4IntervalCount", "grpAlarm4_ContinueType"},
    {"edAlarm5_BySiteAlarmYield", "grpAlarm5_BySiteAlarmYield"},
    {"edAlarm5_BySiteAlarmYieldRej", "grpAlarm5_BySiteAlarmYield"},
    {"edAlarm5_BySiteCmpYield", "grpAlarm5_BySiteCmpYield"},
    {"edAlarm5_BySiteCmpYieldRej", "grpAlarm5_BySiteCmpYield"},
    {"edAlarm5_BySiteIntervalContactCnt", "pnlAlarm5_BySiteIntervalContactCnt"},
    {"edAlarm5_BySiteLowYield", "grpAlarm5_BySiteLowYield"},
    {"edAlarm5_BySiteLowYieldRej", "grpAlarm5_BySiteLowYield"},
    {"edAlarm5_BySitePreCmpYield", "grpAlarm5_BySitePreCmpYieldEnable"},
    {"edAlarm5_BySitePreCmpYieldRej", "grpAlarm5_BySitePreCmpYieldEnable"},
    {"edAlarm5_OSBin", "grpAlarm5_BySiteAlarmYield"},
    {"edAllSiteFailCount", "pnlAlarmFT"},
    {"edAllSiteFailCountRT", "gbAlarmRT"},
    {"edAutoCleanLowYieldCount", "gbLowYieldAutoClean"},
    {"edAutoCleanLowYieldLimit", "gbLowYieldAutoClean"},
    {"edAutoLeastRetestLimitFile", "gbA10"},
    {"edAutoRetestLimitFile", "gbA10"},
    {"edByArmSiteGapCatCT_FT", "tsSpecBinByArmPerSiteCompare_FT"},
    {"edByArmSiteGapCatCT_RT", "tsSpecBinByArmPerSiteCompare_RT"},
    {"edByBinFailureCatCT_FT", "tsBinAlarm1_FT"},
    {"edByBinFailureCatCT_RT", "tsBinAlarm1_RT"},
    {"edByBinLimitCountCat0_FT", "TabSheet1"},
    {"edByBinLimitCountCat0_RT", "TabSheet2"},
    {"edByBinLimitCountCat10_FT", "TabSheet1"},
    {"edByBinLimitCountCat10_RT", "TabSheet2"},
    {"edByBinLimitCountCat11_FT", "TabSheet1"},
    {"edByBinLimitCountCat11_RT", "TabSheet2"},
    {"edByBinLimitCountCat12_FT", "TabSheet1"},
    {"edByBinLimitCountCat12_RT", "TabSheet2"},
    {"edByBinLimitCountCat13_FT", "TabSheet1"},
    {"edByBinLimitCountCat13_RT", "TabSheet2"},
    {"edByBinLimitCountCat14_FT", "TabSheet1"},
    {"edByBinLimitCountCat14_RT", "TabSheet2"},
    {"edByBinLimitCountCat15_FT", "TabSheet1"},
    {"edByBinLimitCountCat15_RT", "TabSheet2"},
    {"edByBinLimitCountCat1_FT", "TabSheet1"},
    {"edByBinLimitCountCat1_RT", "TabSheet2"},
    {"edByBinLimitCountCat2_FT", "TabSheet1"},
    {"edByBinLimitCountCat2_RT", "TabSheet2"},
    {"edByBinLimitCountCat3_FT", "TabSheet1"},
    {"edByBinLimitCountCat3_RT", "TabSheet2"},
    {"edByBinLimitCountCat4_FT", "TabSheet1"},
    {"edByBinLimitCountCat4_RT", "TabSheet2"},
    {"edByBinLimitCountCat5_FT", "TabSheet1"},
    {"edByBinLimitCountCat5_RT", "TabSheet2"},
    {"edByBinLimitCountCat6_FT", "TabSheet1"},
    {"edByBinLimitCountCat6_RT", "TabSheet2"},
    {"edByBinLimitCountCat7_FT", "TabSheet1"},
    {"edByBinLimitCountCat7_RT", "TabSheet2"},
    {"edByBinLimitCountCat8_FT", "TabSheet1"},
    {"edByBinLimitCountCat8_RT", "TabSheet2"},
    {"edByBinLimitCountCat9_FT", "TabSheet1"},
    {"edByBinLimitCountCat9_RT", "TabSheet2"},
    {"edByBinLimitCountCatCT_FT", "TabSheet1"},
    {"edByBinLimitCountCatCT_RT", "TabSheet2"},
    {"edByBinSiteGapCatCT_FT", "tsSpecBinBySiteCompare_FT"},
    {"edByBinSiteGapCatCT_RT", "tsSpecBinBySiteCompare_RT"},
    {"edContactCountFT", "palContactCount_FT"},
    {"edContactCountRT", "palContactCount_RT"},
    {"edContinuPassSkt_FT", "palContinuPassSkt_FT"},
    {"edContinuPassSkt_RT", "palContinuPassSkt_RT"},
    {"edContinuousLoad_FT", "palContinuousLoader_FT"},
    {"edContinuousLoad_RT", "palContinuousLoader_RT"},
    {"edContinuousPass_FT", "palContinuousPass_FT"},
    {"edContinuousPass_RT", "palContinuousPass_RT"},
    {"edContsFailHeadAlarmCT_FT", "gbContsFailByHead_FT"},
    {"edContsFailHeadAlarmCT_RT", "gbContsFailByHead_RT"},
    {"edContsFailIgnore_FT", "palConsFailIgnore_FT"},
    {"edContsFailIgnore_RT", "palConsFailIgnore_RT"},
    {"edContsFailSocketAlarmCT_FT", "gbContsFailBySocket_FT"},
    {"edContsFailSocketAlarmCT_RT", "gbContsFailBySocket_RT"},
    {"edFailYieldMaxRate_ARTFTFile", "palART_FT"},
    {"edFailYieldMaxRate_ARTRTFile", "palART_RT"},
    {"edFailYieldMinRate_ARTFTFile", "palART_FT"},
    {"edFailYieldMinRate_ARTRTFile", "palART_RT"},
    {"edFailYieldRate_ARTFTFile", "palART_FT"},
    {"edFailYieldRate_ARTFile", "pnlARTFailYiel"},
    {"edFailYieldRate_ARTRTFile", "palART_RT"},
    {"edIntervalLowYieldBySiteIg_FT", "palIntervalLowYeildBySite_FT"},
    {"edIntervalLowYieldBySiteIg_RT", "palIntervalLowYeildBySite_RT"},
    {"edIntervalLowYieldBySite_FT", "palIntervalLowYeildBySite_FT"},
    {"edIntervalLowYieldBySite_RT", "palIntervalLowYeildBySite_RT"},
    {"edIntervalLowYieldByTotalIg_FT", "palIntervalLowYeildByTotal_FT"},
    {"edIntervalLowYieldByTotalIg_RT", "palIntervalLowYeildByTotal_RT"},
    {"edIntervalLowYieldByTotal_FT", "palIntervalLowYeildByTotal_FT"},
    {"edIntervalLowYieldByTotal_RT", "palIntervalLowYeildByTotal_RT"},
    {"edLoadCellMeasure", "gbLoadcell"},
    {"edLowYieldByTotalIg_FT", "grpLowYieldByTotal_FT"},
    {"edLowYieldByTotalIg_RT", "grpLowYieldByTotal_RT"},
    {"edLowYieldByTotal_FT", "grpLowYieldByTotal_FT"},
    {"edLowYieldByTotal_RT", "grpLowYieldByTotal_RT"},
    {"edLowYieldIg_FT", "grpLowYield_FT"},
    {"edLowYieldIg_RT", "grpLowYield_RT"},
    {"edLowYieldIg_Special1", "grpSpecialLowYield"},
    {"edLowYieldIg_Special2", "grpSpecialLowYield"},
    {"edLowYield_FT", "grpLowYield_FT"},
    {"edLowYield_RT", "grpLowYield_RT"},
    {"edLowYield_Special", "grpSpecialLowYield"},
    {"edSiteYieldCmpIg_FT", "grpSiteYieldCmp_FT"},
    {"edSiteYieldCmpIg_RT", "grpSiteYieldCmp_RT"},
    {"edSiteYieldCmp_FT", "grpSiteYieldCmp_FT"},
    {"edSiteYieldCmp_RT", "grpSiteYieldCmp_RT"},
    {"edSiteYieldDifferentIg_FT", "grpSiteYieldDifferent_FT"},
    {"edSiteYieldDifferentIg_RT", "grpSiteYieldDifferent_RT"},
    {"edSiteYieldDifferent_FT", "grpSiteYieldDifferent_FT"},
    {"edSiteYieldDifferent_RT", "grpSiteYieldDifferent_RT"},
    {"edSlidingWindowSize", "pnlChackInterval"},
    {"ed_Alarm4IntervalYieldContinueCount", "grpAlarm4_EnableIntervalYield"},
    {"ed_Alarm4IntervalYieldIntervalCount", "grpAlarm4_EnableIntervalYield"},
    {"ed_Alarm4IntervalYieldYield", "grpAlarm4_EnableIntervalYield"},
    {"ed_HeadToHeadYield", "grpAlarm4_HeadToHeadYieldEnable"},
    {"ed_HeadToHeadYieldCount", "grpAlarm4_HeadToHeadYieldEnable"},
    {"ed_SiteToSiteYield", "grAlarm4_SiteToSiteYieldEnable"},
    {"ed_SiteToSiteYieldCount", "grAlarm4_SiteToSiteYieldEnable"},
    {"ed_SiteYieldOverAlert", "grpAlarm4_SiteYieldOverAlert"},
    {"ed_SiteYieldOverAlertCount", "grpAlarm4_SiteYieldOverAlert"},
    {"edlContsLowerAlarmMin_FT", "grpAdaptiveYield_FT"},
    {"edlContsLowerAlarmMin_RT", "grpAdaptiveYield_RT"},
    {"edlContsLowerAlarmNor_FT", "grpAdaptiveYield_FT"},
    {"edlContsLowerAlarmNor_RT", "grpAdaptiveYield_RT"},
    {"edtAutoSiteOffAlarm", "gbAutoSiteOff"},
    {"edtAutoSiteOffWhenRT", "grpAutoSiteOffWhenRT"},
    {"edtLowYieldByPickerIg_FT", "grpLowYieldByPicker_FT"},
    {"edtLowYieldByPickerIg_RT", "grpLowYieldByPicker_RT"},
    {"edtLowYieldByPicker_FT", "grpLowYieldByPicker_FT"},
    {"edtLowYieldByPicker_RT", "grpLowYieldByPicker_RT"},
    {"edtSiteYieldCmp1SiteCnt_FT", "gbAutoSiteOff"},
    {"edtYieldAlarmCheckIntervalByCount", "pnlChackInterval"},
    {"gbA10", "tsAutoRetest1"},
    {"gbAlarm", "pnlFT_Center"},
    {"gbAlarmRT", "pnlRT_Center"},
    {"gbAutoSiteOff", "tsAutoSiteOff"},
    {"gbContsFailByHead_FT", "gbAlarm"},
    {"gbContsFailByHead_RT", "gbAlarmRT"},
    {"gbContsFailBySocket_FT", "gbAlarm"},
    {"gbContsFailBySocket_RT", "gbAlarmRT"},
    {"gbLoadcell", "pnlFT_Center"},
    {"gbLowYieldAutoClean", "pnlFT_Center"},
    {"gbOtherYieldAlarmFT", "tsYield"},
    {"gbOtherYieldAlarmRT", "tsYield"},
    {"gbPiggyBack", "pnlFT_Left"},
    {"gbPiggyBackRT", "pnlRT_Left"},
    {"gbYieldAlarm", "pnlFT_Left"},
    {"gbYieldAlarmRT", "pnlRT_Left"},
    {"grAlarm4_SiteToSiteYieldEnable", "grpAlarm4"},
    {"grpAdaptiveYield_FT", "pnlFT_Center"},
    {"grpAdaptiveYield_RT", "pnlRT_Center"},
    {"grpAlarm4", "tsAlarm4"},
    {"grpAlarm4_ContinueType", "grpAlarm4"},
    {"grpAlarm4_EnableIntervalYield", "grpAlarm4"},
    {"grpAlarm4_HeadToHeadYieldEnable", "grpAlarm4"},
    {"grpAlarm4_SiteYieldOverAlert", "grpAlarm4"},
    {"grpAlarm5", "tsAlarm4"},
    {"grpAlarm5_BySiteAlarmYield", "grpAlarm5"},
    {"grpAlarm5_BySiteCmpYield", "grpAlarm5"},
    {"grpAlarm5_BySiteLowYield", "grpAlarm5"},
    {"grpAlarm5_BySitePreCmpYieldEnable", "grpAlarm5"},
    {"grpAutoSiteOffWhenRT", "pnlAutoSiteOff_Right"},
    {"grpCreateManualEOCAP", "pnlFT_Left"},
    {"grpLowYieldByPicker_FT", "gbYieldAlarm"},
    {"grpLowYieldByPicker_RT", "gbYieldAlarmRT"},
    {"grpLowYieldByTotal_FT", "gbYieldAlarm"},
    {"grpLowYieldByTotal_RT", "gbYieldAlarmRT"},
    {"grpLowYield_FT", "gbYieldAlarm"},
    {"grpLowYield_RT", "gbYieldAlarmRT"},
    {"grpSiteYieldCmp_FT", "gbYieldAlarm"},
    {"grpSiteYieldCmp_RT", "gbYieldAlarmRT"},
    {"grpSiteYieldDifferent_FT", "gbYieldAlarm"},
    {"grpSiteYieldDifferent_RT", "gbYieldAlarmRT"},
    {"grpSpecialLowYield", "pnlFT_Center"},
    {"labLowYield1_FT", "grpLowYield_FT"},
    {"labLowYield1_RT", "grpLowYield_RT"},
    {"labLowYieldByTotal1_FT", "grpLowYieldByTotal_FT"},
    {"labLowYieldByTotal1_RT", "grpLowYieldByTotal_RT"},
    {"labSiteYieldCmp1SiteCnt_FT", "gbAutoSiteOff"},
    {"labSiteYieldCmp1_FT", "grpSiteYieldCmp_FT"},
    {"labSiteYieldCmp1_RT", "grpSiteYieldCmp_RT"},
    {"labSiteYieldDifferent1_FT", "grpSiteYieldDifferent_FT"},
    {"labSiteYieldDifferent1_RT", "grpSiteYieldDifferent_RT"},
    {"lblCheckInteval", "pnlChackInterval"},
    {"lblLowYieldByPicker_FT", "grpLowYieldByPicker_FT"},
    {"lblLowYieldByPicker_RT", "grpLowYieldByPicker_RT"},
    {"palART_FT", "gbA10"},
    {"palART_RT", "gbA10"},
    {"palConsFailIgnore_FT", "gbAlarm"},
    {"palConsFailIgnore_RT", "gbAlarmRT"},
    {"palContactCount_FT", "gbPiggyBack"},
    {"palContactCount_RT", "gbPiggyBackRT"},
    {"palContinuPassSkt_FT", "gbPiggyBack"},
    {"palContinuPassSkt_RT", "gbPiggyBackRT"},
    {"palContinuousLoader_FT", "gbPiggyBack"},
    {"palContinuousLoader_RT", "gbPiggyBackRT"},
    {"palContinuousPass_FT", "gbPiggyBack"},
    {"palContinuousPass_RT", "gbPiggyBackRT"},
    {"palIntervalLowYeildBySite_FT", "gbOtherYieldAlarmFT"},
    {"palIntervalLowYeildBySite_RT", "gbOtherYieldAlarmRT"},
    {"palIntervalLowYeildByTotal_FT", "gbOtherYieldAlarmFT"},
    {"palIntervalLowYeildByTotal_RT", "gbOtherYieldAlarmRT"},
    {"pgcBySiteByBinPercentCompare_FT", "tsNormal"},
    {"pgcBySiteByBinPercentCompare_RT", "tsReTest"},
    {"pgcMode", "pnlTop"},
    {"pnlARTFailYiel", "gbA10"},
    {"pnlAlarm5_BySiteIntervalContactCnt", "grpAlarm5"},
    {"pnlAlarmFT", "gbAlarm"},
    {"pnlAutoSiteOff_Right", "tsAutoSiteOff"},
    {"pnlChackInterval", "gbYieldAlarm"},
    {"pnlFT_Center", "tsNormal"},
    {"pnlFT_Left", "tsNormal"},
    {"pnlRT_Center", "tsReTest"},
    {"pnlRT_Left", "tsReTest"},
    {"rbContsFailByHead_FTOff", "gbContsFailByHead_FT"},
    {"rbContsFailByHead_FTOn", "gbContsFailByHead_FT"},
    {"rbContsFailByHead_RTOff", "gbContsFailByHead_RT"},
    {"rbContsFailByHead_RTOn", "gbContsFailByHead_RT"},
    {"rbContsFailBySocket_FTOff", "gbContsFailBySocket_FT"},
    {"rbContsFailBySocket_FTOn", "gbContsFailBySocket_FT"},
    {"rbContsFailBySocket_RTOff", "gbContsFailBySocket_RT"},
    {"rbContsFailBySocket_RTOn", "gbContsFailBySocket_RT"},
    {"rgAutoCleanYieldOnOff", "gbLowYieldAutoClean"},
    {"rgAutoSiteOffWhenRT", "grpAutoSiteOffWhenRT"},
    {"rgAutoSiteOn", "pnlAutoSiteOff_Right"},
    {"rgAutoSiteOnByGPIB", "pnlAutoSiteOff_Right"},
    {"rgCloseSiteOnHPAction", "gbAutoSiteOff"},
    {"rgFT_ART", "palART_FT"},
    {"rgPiggyBack_FT", "gbPiggyBack"},
    {"rgPiggyBack_RT", "gbPiggyBackRT"},
    {"rgRT_ART", "palART_RT"},
    {"scrlbxBinAlarm1_FT", "tsBinAlarm1_FT"},
    {"scrlbxBinAlarm1_RT", "tsBinAlarm1_RT"},
    {"scrlbxBinAlarm2_FT", "tsSpecBinBySiteCompare_FT"},
    {"scrlbxBinAlarm2_RT", "tsSpecBinBySiteCompare_RT"},
    {"scrlbxBinAlarm3_FT", "tsSpecBinByArmPerSiteCompare_FT"},
    {"scrlbxBinAlarm3_RT", "tsSpecBinByArmPerSiteCompare_RT"},
    {"tsAlarm4", "pgcMode"},
    {"tsAutoRetest", "pgcMode"},
    {"tsAutoRetest1", "pgcMode"},
    {"tsAutoSiteOff", "pgcMode"},
    {"tsBinAlarm1_FT", "pgcBySiteByBinPercentCompare_FT"},
    {"tsBinAlarm1_RT", "pgcBySiteByBinPercentCompare_RT"},
    {"tsNormal", "pgcMode"},
    {"tsReTest", "pgcMode"},
    {"tsSpecBinByArmPerSiteCompare_FT", "pgcBySiteByBinPercentCompare_FT"},
    {"tsSpecBinByArmPerSiteCompare_RT", "pgcBySiteByBinPercentCompare_RT"},
    {"tsSpecBinBySiteCompare_FT", "pgcBySiteByBinPercentCompare_FT"},
    {"tsSpecBinBySiteCompare_RT", "pgcBySiteByBinPercentCompare_RT"},
    {"tsYield", "pgcMode"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void YM_CreateContainerProxies()
{
    EL<TTabSheet>("TfYieldMonitoring", "TabSheet1");
    EL<TTabSheet>("TfYieldMonitoring", "TabSheet2");
    EL<TGroupBox>("TfYieldMonitoring", "gbA10");
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarm");
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT");
    EL<TGroupBox>("TfYieldMonitoring", "gbAutoSiteOff");
    EL<TGroupBox>("TfYieldMonitoring", "gbContsFailByHead_FT");
    EL<TGroupBox>("TfYieldMonitoring", "gbContsFailByHead_RT");
    EL<TGroupBox>("TfYieldMonitoring", "gbContsFailBySocket_FT");
    EL<TGroupBox>("TfYieldMonitoring", "gbContsFailBySocket_RT");
    EL<TGroupBox>("TfYieldMonitoring", "gbLoadcell");
    EL<TGroupBox>("TfYieldMonitoring", "gbLowYieldAutoClean");
    EL<TGroupBox>("TfYieldMonitoring", "gbOtherYieldAlarmFT");
    EL<TGroupBox>("TfYieldMonitoring", "gbOtherYieldAlarmRT");
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack");
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT");
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm");
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT");
    EL<TGroupBox>("TfYieldMonitoring", "grAlarm4_SiteToSiteYieldEnable");
    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4_ContinueType");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4_EnableIntervalYield");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4_HeadToHeadYieldEnable");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4_SiteYieldOverAlert");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5_BySiteAlarmYield");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5_BySiteCmpYield");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5_BySiteLowYield");
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5_BySitePreCmpYieldEnable");
    EL<TGroupBox>("TfYieldMonitoring", "grpAutoSiteOffWhenRT");
    EL<TGroupBox>("TfYieldMonitoring", "grpCreateManualEOCAP");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByPicker_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByPicker_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByTotal_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByTotal_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT");
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT");
    EL<TGroupBox>("TfYieldMonitoring", "grpSpecialLowYield");
    EL<TPanel>("TfYieldMonitoring", "palART_FT");
    EL<TPanel>("TfYieldMonitoring", "palART_RT");
    EL<TPanel>("TfYieldMonitoring", "palConsFailIgnore_FT");
    EL<TPanel>("TfYieldMonitoring", "palConsFailIgnore_RT");
    EL<TPanel>("TfYieldMonitoring", "palContactCount_FT");
    EL<TPanel>("TfYieldMonitoring", "palContactCount_RT");
    EL<TPanel>("TfYieldMonitoring", "palContinuPassSkt_FT");
    EL<TPanel>("TfYieldMonitoring", "palContinuPassSkt_RT");
    EL<TPanel>("TfYieldMonitoring", "palContinuousLoader_FT");
    EL<TPanel>("TfYieldMonitoring", "palContinuousLoader_RT");
    EL<TPanel>("TfYieldMonitoring", "palContinuousPass_FT");
    EL<TPanel>("TfYieldMonitoring", "palContinuousPass_RT");
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildBySite_FT");
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildBySite_RT");
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildByTotal_FT");
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildByTotal_RT");
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT");
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT");
    EL<TPageControl>("TfYieldMonitoring", "pgcMode");
    EL<TPanel>("TfYieldMonitoring", "pnlARTFailYiel");
    EL<TPanel>("TfYieldMonitoring", "pnlAlarm5_BySiteIntervalContactCnt");
    EL<TPanel>("TfYieldMonitoring", "pnlAlarmFT");
    EL<TPanel>("TfYieldMonitoring", "pnlAutoSiteOff_Right");
    EL<TPanel>("TfYieldMonitoring", "pnlBottom");
    EL<TPanel>("TfYieldMonitoring", "pnlChackInterval");
    EL<TPanel>("TfYieldMonitoring", "pnlFT_Center");
    EL<TPanel>("TfYieldMonitoring", "pnlFT_Left");
    EL<TPanel>("TfYieldMonitoring", "pnlRT_Center");
    EL<TPanel>("TfYieldMonitoring", "pnlRT_Left");
    EL<TPanel>("TfYieldMonitoring", "pnlTop");
    EL<TTabSheet>("TfYieldMonitoring", "tsAlarm4");
    EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest1");
    EL<TTabSheet>("TfYieldMonitoring", "tsAutoSiteOff");
    EL<TTabSheet>("TfYieldMonitoring", "tsBinAlarm1_FT");
    EL<TTabSheet>("TfYieldMonitoring", "tsBinAlarm1_RT");
    EL<TTabSheet>("TfYieldMonitoring", "tsNormal");
    EL<TTabSheet>("TfYieldMonitoring", "tsReTest");
    EL<TTabSheet>("TfYieldMonitoring", "tsSpecBinByArmPerSiteCompare_FT");
    EL<TTabSheet>("TfYieldMonitoring", "tsSpecBinByArmPerSiteCompare_RT");
    EL<TTabSheet>("TfYieldMonitoring", "tsSpecBinBySiteCompare_FT");
    EL<TTabSheet>("TfYieldMonitoring", "tsSpecBinBySiteCompare_RT");
    EL<TTabSheet>("TfYieldMonitoring", "tsYield");
    filerw::ELSetParents("TfYieldMonitoring", kYM_ParentOf, (int)(sizeof(kYM_ParentOf) / sizeof(kYM_ParentOf[0])));
}

// golden uYieldMonitoring.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void YM_DfmState()
{
    EL<TButton>("TfYieldMonitoring", "btnApply")->Enabled = false;
    EL<TButton>("TfYieldMonitoring", "btnUnlockYieldAlarm")->Visible = false;
    EL<TButton>("TfYieldMonitoring", "btnUnlockYieldAlarmRT")->Visible = false;
    EL<TEdit>("TfYieldMonitoring", "eCloseSite")->Enabled = false;
    EL<TEdit>("TfYieldMonitoring", "edtYieldAlarmCheckIntervalByCount")->Visible = false;
    EL<TGroupBox>("TfYieldMonitoring", "gbLoadcell")->Visible = false;
    EL<TGroupBox>("TfYieldMonitoring", "gbLowYieldAutoClean")->Visible = false;
    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_FT")->Visible = false;
    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_RT")->Visible = false;
    EL<TGroupBox>("TfYieldMonitoring", "grpAutoSiteOffWhenRT")->Visible = false;
    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->Text = "0";
    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->ItemIndex = 0;
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT")->Text = "1";
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT")->Text = "1";
    EL<TEdit>("TfYieldMonitoring", "eCloseSite")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYieldRej")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYieldRej")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteIntervalContactCnt")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYieldRej")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYieldRej")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_OSBin")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldCount")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldLimit")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContactCountFT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContactCountRT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLoadCellMeasure")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special1")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special2")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLowYield_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edLowYield_Special")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffAlarm")->Text = "2";
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffWhenRT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_FT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_RT")->Text = "100";
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_FT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_RT")->Text = "0";
    EL<TEdit>("TfYieldMonitoring", "edtSiteYieldCmp1SiteCnt_FT")->Text = "2";
    EL<TEdit>("TfYieldMonitoring", "edtYieldAlarmCheckIntervalByCount")->Text = "20";
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Checked = true;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOff")->Checked = true;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Checked = true;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOff")->Checked = true;
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoCleanYieldOnOff")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOffWhenRT")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn")->ItemIndex = 1;
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOnByGPIB")->ItemIndex = 1;
    EL<TRadioGroup>("TfYieldMonitoring", "rgCloseSiteOnHPAction")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgFT_ART")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfYieldMonitoring", "rgRT_ART")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kYM_EnableAll[] = {
    "TabSheet1", "TabSheet2", "btnApply", "btnUnlockYieldAlarm", "btnUnlockYieldAlarmRT", "cbAdaptiveLowYield_FT",
    "cbAdaptiveLowYield_RT", "cbAlarm4ContinueType", "cbAlarm5_BySiteAlarmYieldEnable", "cbAlarm5_BySiteCmpYieldEnable", "cbAlarm5_BySiteLowYieldEnable", "cbAlarm5_BySitePreCmpYieldEnable",
    "cbAllSiteFail", "cbAllSiteFail_RT", "cbBybinLimitCat0_FT", "cbBybinLimitCat0_RT", "cbBybinLimitCat10_FT", "cbBybinLimitCat10_RT",
    "cbBybinLimitCat11_FT", "cbBybinLimitCat11_RT", "cbBybinLimitCat12_FT", "cbBybinLimitCat12_RT", "cbBybinLimitCat13_FT", "cbBybinLimitCat13_RT",
    "cbBybinLimitCat14_FT", "cbBybinLimitCat14_RT", "cbBybinLimitCat15_FT", "cbBybinLimitCat15_RT", "cbBybinLimitCat1_FT", "cbBybinLimitCat1_RT",
    "cbBybinLimitCat2_FT", "cbBybinLimitCat2_RT", "cbBybinLimitCat3_FT", "cbBybinLimitCat3_RT", "cbBybinLimitCat4_FT", "cbBybinLimitCat4_RT",
    "cbBybinLimitCat5_FT", "cbBybinLimitCat5_RT", "cbBybinLimitCat6_FT", "cbBybinLimitCat6_RT", "cbBybinLimitCat7_FT", "cbBybinLimitCat7_RT",
    "cbBybinLimitCat8_FT", "cbBybinLimitCat8_RT", "cbBybinLimitCat9_FT", "cbBybinLimitCat9_RT", "cbContactCountFT", "cbContactCountRT",
    "cbContinuPassSkt_FT", "cbContinuPassSkt_RT", "cbContinuousLoad_FT", "cbContinuousLoad_RT", "cbContinuousPass_FT", "cbContinuousPass_RT",
    "cbContsFailIgnore_FT", "cbContsFailIgnore_RT", "cbIntervalLowYieldBySite_FT", "cbIntervalLowYieldBySite_RT", "cbIntervalLowYieldByTotal_FT", "cbIntervalLowYieldByTotal_RT",
    "cbLoadCellMeasure", "cbLowYieldByTotal_FT", "cbLowYieldByTotal_RT", "cbLowYield_FT", "cbLowYield_RT", "cbLowYield_Special",
    "cbSiteYieldCmp_FT", "cbSiteYieldCmp_RT", "cbSiteYieldDifferent_FT", "cbSiteYieldDifferent_RT", "cbSlidingWindowYield", "cbYieldAlarmByBin",
    "cb_Alarm4EnableIntervalYield", "cb_HeadToHeadYieldEnable", "cb_SiteToSiteYieldEnable", "cb_SiteYieldOverAlert", "chkAutoSiteOff", "chkAutoSiteOffAlarm",
    "chkAutoSiteOffByArmBySite", "chkAutoSiteOffByArmContiFail", "chkAutoSiteOffByContiFail", "chkCreateManualEOCAP", "chkLowYieldByPicker_FT", "chkLowYieldByPicker_RT",
    "chkSpcBinCntAsContinue_FT", "chkSpcBinCntAsContinue_RT", "chkchkAutoSiteOffByPicker", "ckUseFailNoDistinction", "ckUseLeastRetestTimes", "gbA10",
    "gbAlarm", "gbAlarmRT", "gbAutoSiteOff", "gbContsFailByHead_FT", "gbContsFailByHead_RT", "gbContsFailBySocket_FT",
    "gbContsFailBySocket_RT", "gbLoadcell", "gbLowYieldAutoClean", "gbOtherYieldAlarmFT", "gbOtherYieldAlarmRT", "gbPiggyBack",
    "gbPiggyBackRT", "gbYieldAlarm", "gbYieldAlarmRT", "grAlarm4_SiteToSiteYieldEnable", "grpAdaptiveYield_FT", "grpAdaptiveYield_RT",
    "grpAlarm4", "grpAlarm4_ContinueType", "grpAlarm4_EnableIntervalYield", "grpAlarm4_HeadToHeadYieldEnable", "grpAlarm4_SiteYieldOverAlert", "grpAlarm5",
    "grpAlarm5_BySiteAlarmYield", "grpAlarm5_BySiteCmpYield", "grpAlarm5_BySiteLowYield", "grpAlarm5_BySitePreCmpYieldEnable", "grpAutoSiteOffWhenRT", "grpCreateManualEOCAP",
    "grpLowYieldByPicker_FT", "grpLowYieldByPicker_RT", "grpLowYieldByTotal_FT", "grpLowYieldByTotal_RT", "grpLowYield_FT", "grpLowYield_RT",
    "grpSiteYieldCmp_FT", "grpSiteYieldCmp_RT", "grpSiteYieldDifferent_FT", "grpSiteYieldDifferent_RT", "grpSpecialLowYield", "labLowYield1_FT",
    "labLowYield1_RT", "labLowYieldByTotal1_FT", "labLowYieldByTotal1_RT", "labSiteYieldCmp1SiteCnt_FT", "labSiteYieldCmp1_FT", "labSiteYieldCmp1_RT",
    "labSiteYieldDifferent1_FT", "labSiteYieldDifferent1_RT", "lblCheckInteval", "lblLowYieldByPicker_FT", "lblLowYieldByPicker_RT", "palART_FT",
    "palART_RT", "palConsFailIgnore_FT", "palConsFailIgnore_RT", "palContactCount_FT", "palContactCount_RT", "palContinuPassSkt_FT",
    "palContinuPassSkt_RT", "palContinuousLoader_FT", "palContinuousLoader_RT", "palContinuousPass_FT", "palContinuousPass_RT", "palIntervalLowYeildBySite_FT",
    "palIntervalLowYeildBySite_RT", "palIntervalLowYeildByTotal_FT", "palIntervalLowYeildByTotal_RT", "pgcBySiteByBinPercentCompare_FT", "pgcBySiteByBinPercentCompare_RT", "pgcMode",
    "pnlARTFailYiel", "pnlAlarm5_BySiteIntervalContactCnt", "pnlAlarmFT", "pnlAutoSiteOff_Right", "pnlBottom", "pnlChackInterval",
    "pnlFT_Center", "pnlFT_Left", "pnlRT_Center", "pnlRT_Left", "pnlTop", "rbContsFailByHead_FTOff",
    "rbContsFailByHead_FTOn", "rbContsFailByHead_RTOff", "rbContsFailByHead_RTOn", "rbContsFailBySocket_FTOff", "rbContsFailBySocket_FTOn", "rbContsFailBySocket_RTOff",
    "rbContsFailBySocket_RTOn", "rgAutoCleanYieldOnOff", "rgAutoSiteOffWhenRT", "rgAutoSiteOn", "rgAutoSiteOnByGPIB", "rgCloseSiteOnHPAction",
    "rgFT_ART", "rgPiggyBack_FT", "rgPiggyBack_RT", "rgRT_ART", "scrlbxBinAlarm1_FT", "scrlbxBinAlarm1_RT",
    "scrlbxBinAlarm2_FT", "scrlbxBinAlarm2_RT", "scrlbxBinAlarm3_FT", "scrlbxBinAlarm3_RT", "tsAlarm4", "tsAutoRetest",
    "tsAutoRetest1", "tsAutoSiteOff", "tsBinAlarm1_FT", "tsBinAlarm1_RT", "tsNormal", "tsReTest",
    "tsSpecBinByArmPerSiteCompare_FT", "tsSpecBinByArmPerSiteCompare_RT", "tsSpecBinBySiteCompare_FT", "tsSpecBinBySiteCompare_RT", "tsYield",
};

// 存檔流程（btnApplyClick、DoFormToData、CheckSettingNo、SaveSetupFile）讀的替身 170 個 —— FileRW/TestIF_File_YieldMonitoring.cpp 用它決定頁面必須送哪些值
static const char* const kYM_SaveReads[] = {
    "cbAdaptiveLowYield_FT", "cbAdaptiveLowYield_RT", "cbAlarm4ContinueType", "cbAlarm5_BySiteAlarmYieldEnable", "cbAlarm5_BySiteCmpYieldEnable", "cbAlarm5_BySiteLowYieldEnable",
    "cbAlarm5_BySitePreCmpYieldEnable", "cbAllSiteFail", "cbAllSiteFail_RT", "cbContactCountFT", "cbContactCountRT", "cbContinuPassSkt_FT",
    "cbContinuPassSkt_RT", "cbContinuousLoad_FT", "cbContinuousLoad_RT", "cbContinuousPass_FT", "cbContinuousPass_RT", "cbContsFailIgnore_FT",
    "cbContsFailIgnore_RT", "cbIntervalLowYieldBySite_FT", "cbIntervalLowYieldBySite_RT", "cbIntervalLowYieldByTotal_FT", "cbIntervalLowYieldByTotal_RT", "cbLoadCellMeasure",
    "cbLowYieldByTotal_FT", "cbLowYieldByTotal_RT", "cbLowYield_FT", "cbLowYield_RT", "cbLowYield_Special", "cbSiteYieldCmp_FT",
    "cbSiteYieldCmp_RT", "cbSiteYieldDifferent_FT", "cbSiteYieldDifferent_RT", "cbSlidingWindowYield", "cbUseFTFailYieldModel", "cbUseRTFailYieldModel",
    "cbYieldAlarmByBin", "cb_Alarm4EnableIntervalYield", "cb_HeadToHeadYieldEnable", "cb_SiteToSiteYieldEnable", "cb_SiteYieldOverAlert", "cbbClosedSiteBin",
    "chkAutoSiteOff", "chkAutoSiteOffAlarm", "chkAutoSiteOffByArmBySite", "chkAutoSiteOffByArmContiFail", "chkAutoSiteOffByContiFail", "chkCreateManualEOCAP",
    "chkLowYieldByPicker_FT", "chkLowYieldByPicker_RT", "chkSpcBinCntAsContinue_FT", "chkSpcBinCntAsContinue_RT", "chkchkAutoSiteOffByPicker", "ckUseFailNoDistinction",
    "ckUseLeastRetestTimes", "cobContinuousPass_FT", "cobContinuousPass_RT", "edAdaptiveYieldMax_FT", "edAdaptiveYieldMax_RT", "edAdaptiveYieldMin_FT",
    "edAdaptiveYieldMin_RT", "edAlarm4ContinueCount", "edAlarm4IntervalCount", "edAlarm5_BySiteAlarmYield", "edAlarm5_BySiteAlarmYieldRej", "edAlarm5_BySiteCmpYield",
    "edAlarm5_BySiteCmpYieldRej", "edAlarm5_BySiteIntervalContactCnt", "edAlarm5_BySiteLowYield", "edAlarm5_BySiteLowYieldRej", "edAlarm5_BySitePreCmpYield", "edAlarm5_BySitePreCmpYieldRej",
    "edAlarm5_OSBin", "edAllSiteFailCount", "edAllSiteFailCountRT", "edAutoCleanLowYieldCount", "edAutoCleanLowYieldLimit", "edAutoLeastRetestLimitFile",
    "edAutoRetestLimitFile", "edByArmSiteGapCatCT_FT", "edByArmSiteGapCatCT_RT", "edByBinFailureCatCT_FT", "edByBinFailureCatCT_RT", "edByBinLimitCountCatCT_FT",
    "edByBinLimitCountCatCT_RT", "edByBinSiteGapCatCT_FT", "edByBinSiteGapCatCT_RT", "edContactCountFT", "edContactCountRT", "edContinuPassSkt_FT",
    "edContinuPassSkt_RT", "edContinuousLoad_FT", "edContinuousLoad_RT", "edContinuousPass_FT", "edContinuousPass_RT", "edContsFailHeadAlarmCT_FT",
    "edContsFailHeadAlarmCT_RT", "edContsFailIgnore_FT", "edContsFailIgnore_RT", "edContsFailSocketAlarmCT_FT", "edContsFailSocketAlarmCT_RT", "edFailYieldMaxRate_ARTFTFile",
    "edFailYieldMaxRate_ARTRTFile", "edFailYieldMinRate_ARTFTFile", "edFailYieldMinRate_ARTRTFile", "edFailYieldRate_ARTFTFile", "edFailYieldRate_ARTFile", "edFailYieldRate_ARTRTFile",
    "edIntervalLowYieldBySiteIg_FT", "edIntervalLowYieldBySiteIg_RT", "edIntervalLowYieldBySite_FT", "edIntervalLowYieldBySite_RT", "edIntervalLowYieldByTotalIg_FT", "edIntervalLowYieldByTotalIg_RT",
    "edIntervalLowYieldByTotal_FT", "edIntervalLowYieldByTotal_RT", "edLoadCellMeasure", "edLowYieldByTotalIg_FT", "edLowYieldByTotalIg_RT", "edLowYieldByTotal_FT",
    "edLowYieldByTotal_RT", "edLowYieldIg_FT", "edLowYieldIg_RT", "edLowYieldIg_Special1", "edLowYieldIg_Special2", "edLowYield_FT",
    "edLowYield_RT", "edLowYield_Special", "edSiteYieldCmpIg_FT", "edSiteYieldCmpIg_RT", "edSiteYieldCmp_FT", "edSiteYieldCmp_RT",
    "edSiteYieldDifferentIg_FT", "edSiteYieldDifferentIg_RT", "edSiteYieldDifferent_FT", "edSiteYieldDifferent_RT", "edSlidingWindowSize", "ed_Alarm4IntervalYieldContinueCount",
    "ed_Alarm4IntervalYieldIntervalCount", "ed_Alarm4IntervalYieldYield", "ed_HeadToHeadYield", "ed_HeadToHeadYieldCount", "ed_SiteToSiteYield", "ed_SiteToSiteYieldCount",
    "ed_SiteYieldOverAlert", "ed_SiteYieldOverAlertCount", "edlContsLowerAlarmMin_FT", "edlContsLowerAlarmMin_RT", "edlContsLowerAlarmNor_FT", "edlContsLowerAlarmNor_RT",
    "edtAutoSiteOffAlarm", "edtAutoSiteOffWhenRT", "edtLowYieldByPickerIg_FT", "edtLowYieldByPickerIg_RT", "edtLowYieldByPicker_FT", "edtLowYieldByPicker_RT",
    "edtSiteYieldCmp1SiteCnt_FT", "rbContsFailByHead_FTOn", "rbContsFailByHead_RTOn", "rbContsFailBySocket_FTOn", "rbContsFailBySocket_RTOn", "rgAutoCleanYieldOnOff",
    "rgAutoSiteOffWhenRT", "rgAutoSiteOn", "rgAutoSiteOnByGPIB", "rgCloseSiteOnHPAction", "rgFT_ART", "rgPiggyBack_FT",
    "rgPiggyBack_RT", "rgRT_ART",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void YM_CreateSaveProxies()
{
    EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType");
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteAlarmYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteCmpYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteLowYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySitePreCmpYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail");
    EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbLoadCellMeasure");
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_Special");
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_FT");
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_RT");
    EL<TCheckBox>("TfYieldMonitoring", "cbSlidingWindowYield");
    EL<TComboBox>("TfYieldMonitoring", "cbUseFTFailYieldModel");
    EL<TComboBox>("TfYieldMonitoring", "cbUseRTFailYieldModel");
    EL<TCheckBox>("TfYieldMonitoring", "cbYieldAlarmByBin");
    EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield");
    EL<TCheckBox>("TfYieldMonitoring", "cb_HeadToHeadYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cb_SiteToSiteYieldEnable");
    EL<TCheckBox>("TfYieldMonitoring", "cb_SiteYieldOverAlert");
    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin");
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOff");
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffAlarm");
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmBySite");
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmContiFail");
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByContiFail");
    EL<TCheckBox>("TfYieldMonitoring", "chkCreateManualEOCAP");
    EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_FT");
    EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_RT");
    EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_FT");
    EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_RT");
    EL<TCheckBox>("TfYieldMonitoring", "chkchkAutoSiteOffByPicker");
    EL<TCheckBox>("TfYieldMonitoring", "ckUseFailNoDistinction");
    EL<TCheckBox>("TfYieldMonitoring", "ckUseLeastRetestTimes");
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT");
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT");
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_FT");
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_RT");
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_FT");
    EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_RT");
    EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount");
    EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYield");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYieldRej");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYield");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYieldRej");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteIntervalContactCnt");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYield");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYieldRej");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYield");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYieldRej");
    EL<TEdit>("TfYieldMonitoring", "edAlarm5_OSBin");
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount");
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT");
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldCount");
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldLimit");
    EL<TEdit>("TfYieldMonitoring", "edAutoLeastRetestLimitFile");
    EL<TEdit>("TfYieldMonitoring", "edAutoRetestLimitFile");
    EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edContactCountFT");
    EL<TEdit>("TfYieldMonitoring", "edContactCountRT");
    EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT");
    EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT");
    EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT");
    EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT");
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_FT");
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_RT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_FT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_RT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT");
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTFTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTRTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTFTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTRTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFile");
    EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTRTFile");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT");
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT");
    EL<TEdit>("TfYieldMonitoring", "edLoadCellMeasure");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_FT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_RT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special1");
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special2");
    EL<TEdit>("TfYieldMonitoring", "edLowYield_FT");
    EL<TEdit>("TfYieldMonitoring", "edLowYield_RT");
    EL<TEdit>("TfYieldMonitoring", "edLowYield_Special");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_FT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_RT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT");
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT");
    EL<TLabeledEdit>("TfYieldMonitoring", "edSlidingWindowSize");
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount");
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount");
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield");
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield");
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount");
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield");
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount");
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert");
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount");
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_FT");
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_RT");
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_FT");
    EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_RT");
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffAlarm");
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffWhenRT");
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_FT");
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_RT");
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_FT");
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_RT");
    EL<TEdit>("TfYieldMonitoring", "edtSiteYieldCmp1SiteCnt_FT");
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn");
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOn");
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn");
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOn");
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoCleanYieldOnOff");
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOffWhenRT");
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn");
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOnByGPIB");
    EL<TRadioGroup>("TfYieldMonitoring", "rgCloseSiteOnHPAction");
    EL<TRadioGroup>("TfYieldMonitoring", "rgFT_ART");
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT");
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT");
    EL<TRadioGroup>("TfYieldMonitoring", "rgRT_ART");
}

// golden uYieldMonitoring.cpp:287  TfYieldMonitoring::TfYieldMonitoring(TComponent* Owner)
static void YM_TfYieldMonitoring()
{

    bFirstCount=true;
    bYieldCountCheckTick=false;                                                 //AI(rf360-yield-count) 20260814 (RogerYang)
    ulYieldFailBase=0;                                                          //AI(rf360-yield-count) 20260814 (RogerYang)
    bAskYieldPassword=false;                                                    //AI(rf360-yield-password) 20260814 (RogerYang)
    bYieldAlarmUnlocked=false;                                                  //AI(rf360-yield-password) 20260814 (RogerYang)

    iMinYield=0;
    iMaxYield=100;
    dMinYield=0.0;
    dMaxYield=100.0;
    iMinCount=1;
    iMaxCount=100000;

//    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //Sam 20211120 : 卓劭要求能設定為最低 0
//    {
//        iMinYield=0;
//        dMinYield=0.0;
//    }

//    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //wei 20151221 XILINX LowYieldCount 最小值為1
//    {
//        iMinCount=1;
//    }

    ;   // golden uYieldMonitoring.cpp:314-331 取代：->Hint：Default Recipe ChangeLog 的提示字（vclcompat 沒有 Hint，HTML 不用）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:314-331 -- ->Hint：Default Recipe ChangeLog 的提示字（vclcompat 沒有 Hint，HTML 不用）
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_FT")->Hint                         ="Yield_Normal Low Yield Enable";
    EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")->Hint                         ="Yield_Normal Low Yield";      //Sam 20201209 : Default Recipe ChangeLog
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT")->Hint                       ="Yield_Normal Low Yield Count";

    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT")->Hint           ="Yield_Normal Consecutive Failure Alarm By Socket";
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")->Hint             ="Yield_Normal Consecutive Failure Alarm By Head";

    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType")->Hint                  ="Yield_Alarm4 Site To Site Continue Type Select";
    EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount")->Hint                 ="Yield_Alarm4 Site To Site Interval Count";
    EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount")->Hint                 ="Yield_Alarm4 Site To Site Continue Count";

    EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield")->Hint          ="Yield_Alarm4 Interval Total Yield Different Select";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")->Hint           ="Yield_Alarm4 Interval Total Yield Different";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount")->Hint   ="Yield_Alarm4 Interval Total Yield Different Interval Count";
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount")->Hint   ="Yield_Alarm4 Interval Total Yield Different Continue Count";

    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")->Hint                    ="Yield_Alarm4 Site To Site Yield";
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")->Hint                    ="Yield_Alarm4 Head To Head Yield";
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150702 ART 設定
    {
    MyYieldPanel.push_back(new YM_TMyYieldPanel("ART"));   // golden uYieldMonitoring.cpp:335-335 取代：TMyYieldPanel：GUI 格子（TTMyTray256）沒有移植 → 只建資料半邊＋ReTestLimit 替身（YM_TMyYieldPanel）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:335-335 -- TMyYieldPanel：GUI 格子（TTMyTray256）沒有移植 → 只建資料半邊＋ReTestLimit 替身（YM_TMyYieldPanel）
        MyYieldPanel.push_back(new TMyYieldPanel(this, 0, EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest"), "ART"));
#endif // GATE (S12-C save)
    }
    bGetGPIBAutoSiteOff=false;

}

// golden uYieldMonitoring.cpp:340  TfYieldMonitoring::FormCreate(TObject *Sender)
static void YM_FormCreate()
{

    AnsiString str, StrC; for(int i=0; i<TEST_MAX_BIN; i++) { str.sprintf("Category %d", i); StrC.sprintf("% *s%%", -37, str.c_str()); str.sprintf("cbByBinSiteGapCat_FT%03d", i); cbByBinSiteGapCat_FT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm2_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_FT"); cbByBinSiteGapCat_FT[i]->Caption=StrC; str.sprintf("cbByBinSiteGapCat_RT%03d", i); cbByBinSiteGapCat_RT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm2_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_RT"); cbByBinSiteGapCat_RT[i]->Caption=StrC; str.sprintf("edByBinSiteGapCat_FT%03d", i); edByBinSiteGapCat_FT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm2_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_FT"); str.sprintf("edByBinSiteGapCat_RT%03d", i); edByBinSiteGapCat_RT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm2_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_RT"); str.sprintf("cbByArmSiteGapCat_FT%03d", i); cbByArmSiteGapCat_FT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm3_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_FT"); cbByArmSiteGapCat_FT[i]->Caption=StrC; str.sprintf("cbByArmSiteGapCat_RT%03d", i); cbByArmSiteGapCat_RT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm3_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_RT"); cbByArmSiteGapCat_RT[i]->Caption=StrC; str.sprintf("edByArmSiteGapCat_FT%03d", i); edByArmSiteGapCat_FT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm3_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_FT"); str.sprintf("edByArmSiteGapCat_RT%03d", i); edByArmSiteGapCat_RT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm3_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_RT"); str.sprintf("cbByBinFailureCat_FT%03d", i); cbByBinFailureCat_FT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm1_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_FT"); cbByBinFailureCat_FT[i]->Caption=StrC; str.sprintf("cbByBinFailureCat_RT%03d", i); cbByBinFailureCat_RT[i]=EL<TCheckBox>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm1_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_RT"); cbByBinFailureCat_RT[i]->Caption=StrC; str.sprintf("edByBinFailureCat_FT%03d", i); edByBinFailureCat_FT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm1_FT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_FT"); str.sprintf("edByBinFailureCat_RT%03d", i); edByBinFailureCat_RT[i]=EL<TEdit>("TfYieldMonitoring", str.c_str()); { const char* pr[1][2]={{str.c_str(), "scrlbxBinAlarm1_RT"}}; filerw::ELSetParents("TfYieldMonitoring", pr, 1); } EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_RT"); }   // golden uYieldMonitoring.cpp:342-494 取代：動態 Category 元件：照 golden Name（sprintf %03d）建具名替身、父容器照 golden ->Parent；位置／字型／事件（OnMouseUp 鍵盤、OnChange 亮 Apply）是純畫面
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:342-494 -- 動態 Category 元件：照 golden Name（sprintf %03d）建具名替身、父容器照 golden ->Parent；位置／字型／事件（OnMouseUp 鍵盤、OnChange 亮 Apply）是純畫面
    AnsiString str, StrC;
    int iRowHeight=10;

    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        iRowHeight=30*i+10;
        str.sprintf("Category %d", i);
        StrC.sprintf("% *s%%", -37, str);

        str.sprintf("cbByBinSiteGapCat_FT%03d", i);
        cbByBinSiteGapCat_FT[i]             =new TCheckBox(this);
        cbByBinSiteGapCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_FT");
        cbByBinSiteGapCat_FT[i]->Name       =str;
        cbByBinSiteGapCat_FT[i]->Height     =16;
        cbByBinSiteGapCat_FT[i]->Left       =8;
        cbByBinSiteGapCat_FT[i]->Top        =iRowHeight;
        cbByBinSiteGapCat_FT[i]->Width      =228;
        cbByBinSiteGapCat_FT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByBinSiteGapCat_FT[i]->Caption    =StrC;
        cbByBinSiteGapCat_FT[i]->Font->Name ="MS Sans Serif";
        cbByBinSiteGapCat_FT[i]->Font->Size =12;

        str.sprintf("cbByBinSiteGapCat_RT%03d", i);
        cbByBinSiteGapCat_RT[i]             =new TCheckBox(this);
        cbByBinSiteGapCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_RT");
        cbByBinSiteGapCat_RT[i]->Name       =str;
        cbByBinSiteGapCat_RT[i]->Height     =16;
        cbByBinSiteGapCat_RT[i]->Left       =8;
        cbByBinSiteGapCat_RT[i]->Top        =iRowHeight;
        cbByBinSiteGapCat_RT[i]->Width      =228;
        cbByBinSiteGapCat_RT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByBinSiteGapCat_RT[i]->Caption    =StrC;
        cbByBinSiteGapCat_RT[i]->Font->Name ="MS Sans Serif";
        cbByBinSiteGapCat_RT[i]->Font->Size =12;

        str.sprintf("edByBinSiteGapCat_FT%03d", i);
        edByBinSiteGapCat_FT[i]             =new TEdit(this);
        edByBinSiteGapCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_FT");
        edByBinSiteGapCat_FT[i]->Name       =str;
        edByBinSiteGapCat_FT[i]->Height     =28;
        edByBinSiteGapCat_FT[i]->Left       =135;
        edByBinSiteGapCat_FT[i]->Top        =iRowHeight-6;
        edByBinSiteGapCat_FT[i]->Width      =60;
        edByBinSiteGapCat_FT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByBinSiteGapCat_FT[i]->OnChange   =edContactCountFTChange;

        str.sprintf("edByBinSiteGapCat_RT%03d", i);
        edByBinSiteGapCat_RT[i]             =new TEdit(this);
        edByBinSiteGapCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm2_RT");
        edByBinSiteGapCat_RT[i]->Name       =str;
        edByBinSiteGapCat_RT[i]->Height     =28;
        edByBinSiteGapCat_RT[i]->Left       =135;
        edByBinSiteGapCat_RT[i]->Top        =iRowHeight-6;
        edByBinSiteGapCat_RT[i]->Width      =60;
        edByBinSiteGapCat_RT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByBinSiteGapCat_RT[i]->OnChange   =edContactCountFTChange;

        str.sprintf("cbByArmSiteGapCat_FT%03d", i);
        cbByArmSiteGapCat_FT[i]             =new TCheckBox(this);
        cbByArmSiteGapCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_FT");
        cbByArmSiteGapCat_FT[i]->Name       =str;
        cbByArmSiteGapCat_FT[i]->Height     =16;
        cbByArmSiteGapCat_FT[i]->Left       =8;
        cbByArmSiteGapCat_FT[i]->Top        =iRowHeight;
        cbByArmSiteGapCat_FT[i]->Width      =228;
        cbByArmSiteGapCat_FT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByArmSiteGapCat_FT[i]->Caption    =StrC;
        cbByArmSiteGapCat_FT[i]->Font->Name ="MS Sans Serif";
        cbByArmSiteGapCat_FT[i]->Font->Size =12;

        str.sprintf("cbByArmSiteGapCat_RT%03d", i);
        cbByArmSiteGapCat_RT[i]             =new TCheckBox(this);
        cbByArmSiteGapCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_RT");
        cbByArmSiteGapCat_RT[i]->Name       =str;
        cbByArmSiteGapCat_RT[i]->Height     =16;
        cbByArmSiteGapCat_RT[i]->Left       =8;
        cbByArmSiteGapCat_RT[i]->Top        =iRowHeight;
        cbByArmSiteGapCat_RT[i]->Width      =228;
        cbByArmSiteGapCat_RT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByArmSiteGapCat_RT[i]->Caption    =StrC;
        cbByArmSiteGapCat_RT[i]->Font->Name ="MS Sans Serif";
        cbByArmSiteGapCat_RT[i]->Font->Size =12;

        str.sprintf("edByArmSiteGapCat_FT%03d", i);
        edByArmSiteGapCat_FT[i]             =new TEdit(this);
        edByArmSiteGapCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_FT");
        edByArmSiteGapCat_FT[i]->Name       =str;
        edByArmSiteGapCat_FT[i]->Height     =28;
        edByArmSiteGapCat_FT[i]->Left       =135;
        edByArmSiteGapCat_FT[i]->Top        =iRowHeight-6;
        edByArmSiteGapCat_FT[i]->Width      =60;
        edByArmSiteGapCat_FT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByArmSiteGapCat_FT[i]->OnChange   =edContactCountFTChange;

        str.sprintf("edByArmSiteGapCat_RT%03d", i);
        edByArmSiteGapCat_RT[i]             =new TEdit(this);
        edByArmSiteGapCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm3_RT");
        edByArmSiteGapCat_RT[i]->Name       =str;
        edByArmSiteGapCat_RT[i]->Height     =28;
        edByArmSiteGapCat_RT[i]->Left       =135;
        edByArmSiteGapCat_RT[i]->Top        =iRowHeight-6;
        edByArmSiteGapCat_RT[i]->Width      =60;
        edByArmSiteGapCat_RT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByArmSiteGapCat_RT[i]->OnChange   =edContactCountFTChange;

        str.sprintf("cbByBinFailureCat_FT%03d", i);
        cbByBinFailureCat_FT[i]             =new TCheckBox(this);
        cbByBinFailureCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_FT");
        cbByBinFailureCat_FT[i]->Name       =str;
        cbByBinFailureCat_FT[i]->Height     =16;
        cbByBinFailureCat_FT[i]->Left       =8;
        cbByBinFailureCat_FT[i]->Top        =iRowHeight;
        cbByBinFailureCat_FT[i]->Width      =228;
        cbByBinFailureCat_FT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByBinFailureCat_FT[i]->Caption    =StrC;
        cbByBinFailureCat_FT[i]->Font->Name ="MS Sans Serif";
        cbByBinFailureCat_FT[i]->Font->Size =12;

        str.sprintf("cbByBinFailureCat_RT%03d", i);
        cbByBinFailureCat_RT[i]             =new TCheckBox(this);
        cbByBinFailureCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_RT");
        cbByBinFailureCat_RT[i]->Name       =str;
        cbByBinFailureCat_RT[i]->Height     =16;
        cbByBinFailureCat_RT[i]->Left       =8;
        cbByBinFailureCat_RT[i]->Top        =iRowHeight;
        cbByBinFailureCat_RT[i]->Width      =228;
        cbByBinFailureCat_RT[i]->OnMouseUp  =cbLowYield_FTMouseUp;
        cbByBinFailureCat_RT[i]->Caption    =StrC;
        cbByBinFailureCat_RT[i]->Font->Name ="MS Sans Serif";
        cbByBinFailureCat_RT[i]->Font->Size =12;

        str.sprintf("edByBinFailureCat_FT%03d", i);
        edByBinFailureCat_FT[i]             =new TEdit(this);
        edByBinFailureCat_FT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_FT");
        edByBinFailureCat_FT[i]->Name       =str;
        edByBinFailureCat_FT[i]->Height     =28;
        edByBinFailureCat_FT[i]->Left       =135;
        edByBinFailureCat_FT[i]->Top        =iRowHeight-6;
        edByBinFailureCat_FT[i]->Width      =60;
        edByBinFailureCat_FT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByBinFailureCat_FT[i]->OnChange   =edContactCountFTChange;

        str.sprintf("edByBinFailureCat_RT%03d", i);
        edByBinFailureCat_RT[i]             =new TEdit(this);
        edByBinFailureCat_RT[i]->Parent     =EL<TControl>("TfYieldMonitoring", "scrlbxBinAlarm1_RT");
        edByBinFailureCat_RT[i]->Name       =str;
        edByBinFailureCat_RT[i]->Height     =28;
        edByBinFailureCat_RT[i]->Left       =135;
        edByBinFailureCat_RT[i]->Top        =iRowHeight-6;
        edByBinFailureCat_RT[i]->Width      =60;
        edByBinFailureCat_RT[i]->OnMouseDown=edLowYield_RTMouseDown;
        edByBinFailureCat_RT[i]->OnChange   =edContactCountFTChange;
    }
#endif // GATE (S12-C save)

}

// golden uYieldMonitoring.cpp:2581  TfYieldMonitoring::FormShow(TObject *Sender)
static void YM_FormShow()
{

    AnsiString Str;
    //Steven 20120809 Start: 先在最上面Enable全部畫面
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")    ->Enabled=true;
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")     ->Enabled=true;
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")         ->Enabled=true;
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")  ->Enabled=true;
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")   ->Enabled=true;
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")       ->Enabled=true;
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT")->Enabled=true;
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT")->Enabled=true;
    //Steven 20120809 End: 先在最上面Enable全部畫面

    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        cbByBinSiteGapCat_FT[i]->Visible=(i<iTestBinCount);
        cbByBinSiteGapCat_RT[i]->Visible=(i<iTestBinCount);
        edByBinSiteGapCat_FT[i]->Visible=(i<iTestBinCount);
        edByBinSiteGapCat_RT[i]->Visible=(i<iTestBinCount);
        cbByArmSiteGapCat_FT[i]->Visible=(i<iTestBinCount);
        cbByArmSiteGapCat_RT[i]->Visible=(i<iTestBinCount);
        edByArmSiteGapCat_FT[i]->Visible=(i<iTestBinCount);
        edByArmSiteGapCat_RT[i]->Visible=(i<iTestBinCount);
        cbByBinFailureCat_FT[i]->Visible=(i<iTestBinCount);
        cbByBinFailureCat_RT[i]->Visible=(i<iTestBinCount);
        edByBinFailureCat_FT[i]->Visible=(i<iTestBinCount);
        edByBinFailureCat_RT[i]->Visible=(i<iTestBinCount);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150704 ART  bin 設定
    {
    ;   // golden uYieldMonitoring.cpp:2613-2614 取代：MyYieldPanel[0]->mtBinSelectYield（TTMyTray256 格子）寬度／欄數：純畫面，格子沒有移植
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2613-2614 -- MyYieldPanel[0]->mtBinSelectYield（TTMyTray256 格子）寬度／欄數：純畫面，格子沒有移植
        MyYieldPanel[0]->mtBinSelectYield->Width=iColWidth*iTestBinCount+10;
        MyYieldPanel[0]->mtBinSelectYield->XItem=iTestBinCount;
#endif // GATE (S12-C save)
        EL<TTabSheet>("TfYieldMonitoring", "tsReTest")->TabVisible=false;                                             //kevin 20150717
    }
    YM_ReadFile();
    ;   // golden uYieldMonitoring.cpp:2618-2618 取代：ChangeData(fYieldMonitoring)：只把 OnClick/OnChange 接到 edContactCountFTChange（亮 Apply 鈕），頁面自己做
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2618-2618 -- ChangeData(fYieldMonitoring)：只把 OnClick/OnChange 接到 edContactCountFTChange（亮 Apply 鈕），頁面自己做
    ChangeData(fYieldMonitoring);
#endif // GATE (S12-C save)

    EL<TButton>("TfYieldMonitoring", "btnApply")->Enabled=false;
    ;   // golden uYieldMonitoring.cpp:2621-2622 取代：Top／Left：視窗位置
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2621-2622 -- Top／Left：視窗位置
    Top=50;
    Left=200;
#endif // GATE (S12-C save)

    if(LastSet.iRunStartMode==rsmContinuStart ||
       LastSet.iRunStartMode==rsmInitialStart ||
       LastSet.iRunStartMode==rsmAutoSiteMap ||
       LastSet.iRunStartMode==rsmQAMode)                                        //ChungHung 20120725 add QAMode 使用 Noraml
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfYieldMonitoring", "pgcMode")->ActivePage=EL<TTabSheet>("TfYieldMonitoring", "tsNormal");
    }
    else if(LastSet.iRunStartMode==rsmInitial_ART ||
            LastSet.iRunStartMode==rsmContinuStart_ART||
            LastSet.iRunStartMode==rsmContinuRetest_ART||
            LastSet.iRunStartMode==rsmAutoRetest)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfYieldMonitoring", "pgcMode")->ActivePage=EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest1");                                  //wei 20150825 KYEC 修改成By File
        }
        else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfYieldMonitoring", "pgcMode")->ActivePage=EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest");                                   //kevin 20150717
        }
        else
        {
            ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfYieldMonitoring", "pgcMode")->ActivePage=EL<TTabSheet>("TfYieldMonitoring", "tsNormal");
        }
    }
    else
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfYieldMonitoring", "pgcMode")->ActivePage=EL<TTabSheet>("TfYieldMonitoring", "tsReTest");
    }

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20101221
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            EL<TTabSheet>("TfYieldMonitoring", "tsNormal")->Enabled=false;
            EL<TTabSheet>("TfYieldMonitoring", "tsReTest")->Enabled=false;
        }
        else
        {
            EL<TTabSheet>("TfYieldMonitoring", "tsNormal")->Enabled=true;
            EL<TTabSheet>("TfYieldMonitoring", "tsReTest")->Enabled=true;
        }
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Caption="Piggingback Check";
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")->Caption="Piggingback Check";
    }

    if(CosFunction.bPiggyBackForASE==true)                                      //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    {
        EL<TPanel>("TfYieldMonitoring", "palContinuousPass_FT")  ->Visible=false;
        EL<TPanel>("TfYieldMonitoring", "palContinuousPass_RT")  ->Visible=false;
        EL<TPanel>("TfYieldMonitoring", "palContinuousLoader_FT")->Visible=false;
        EL<TPanel>("TfYieldMonitoring", "palContinuousLoader_RT")->Visible=false;
        EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest")->TabVisible=true;                                          //kevin 20150601
        EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest1")->TabVisible=false;                                        //wei 20150825
    }
    else
    {
        EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest")->TabVisible=false;                                         //kevin 20150601
        if(CUSTOMER_CODE==CC_KYEC_LEE &&
           USE_AUTO_RETEST==eartInstall &&
           IniConfig.bA10_AutoReTest)
            EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest1")->TabVisible=true;                                     //wei 20150825 KYEC 修改成By File
        else
            EL<TTabSheet>("TfYieldMonitoring", "tsAutoRetest1")->TabVisible=false;                                    //wei 20150825 KYEC 修改成By File
    }
    EL<TTabSheet>("TfYieldMonitoring", "tsAlarm4")->TabVisible        =(CosFunction.bYieldAlarm4 || CosFunction.bYieldAlarm5);                                //wei 20160406 Yield Alarm4
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm4")->Visible          =(CosFunction.bYieldAlarm4);
    EL<TGroupBox>("TfYieldMonitoring", "grpAlarm5")->Visible          =(CosFunction.bYieldAlarm5);                    //Sam 20171213 (Steven) : 超豐良率監控
    EL<TTabSheet>("TfYieldMonitoring", "tsAutoSiteOff")->TabVisible   =CosFunction.bLowYieldAutoSiteOff;              //Steven 20200420 : 整合Auto Site Off
    EL<TTabSheet>("TfYieldMonitoring", "tsYield")->TabVisible         =(CosFunction.IntervalYieldCount);              //Steven 20230223 : 隱藏沒用到的

    EL<TGroupBox>("TfYieldMonitoring", "grpAutoSiteOffWhenRT")->Visible=CosFunction.bAutoCloseSiteWhenRT;             //Steven 20200225 : 切到RT的時候,要關閉Socket
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_FT")  ->Visible=CosFunction.bSiteCmpYield;                    //jou 2014-08-14 Site Compare Low Yield alarm
    EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_RT")  ->Visible=CosFunction.bSiteCmpYield;                    //jou 2014-08-14 Site Compare Low Yield alarm
    EL<TCheckBox>("TfYieldMonitoring", "cbYieldAlarmByBin")   ->Visible=CosFunction.bUseLowYieldAlarmByBin;           //Steven 20140828 : By Bin Yield Monitor
//    rgCloseSiteOnHPAction->Visible=(LastSet.iTemperature==Tempture_Hot);      //Steven 20250430 : mark for 常溫也要用  //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
//    cbbClosedSiteBin    ->Visible=(LastSet.iTemperature==Tempture_Hot);         //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn")        ->Visible=(CosFunction.bLowYieldAutoSiteOff ||          //Steven 20230814 : Initial Start的時候要全開Site
                                   CosFunction.bAutoCloseSiteWhenRT);

    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOff")              ->Visible=(CosFunction.bLowYieldAutoSiteOff);
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmBySite")   ->Visible=(CosFunction.bLowYieldAutoSiteOff);
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByContiFail")   ->Visible=(CosFunction.bLowYieldAutoSiteOff);
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmContiFail")->Visible=(CosFunction.bLowYieldAutoSiteOff);
    EL<TCheckBox>("TfYieldMonitoring", "chkchkAutoSiteOffByPicker")   ->Visible=(CosFunction.bLowYieldAutoSiteOff);   //Steven 20230223 : 根據Index吸嘴比較良率

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20211222 add Visible
    {
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1SiteCnt_FT")->Visible=false;
        EL<TEdit>("TfYieldMonitoring", "edtSiteYieldCmp1SiteCnt_FT")->Visible=false;
        EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOff")->Visible=false;
        EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByContiFail")->Visible=false;
        EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmBySite")->Visible=false;                               //Steven 20230223 : by arm by site, auto site off
    }

    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_FT")->Visible=CosFunction.bAdaptiveYield;                    //Sam 20230914 : 自適應性良率監控
    EL<TGroupBox>("TfYieldMonitoring", "grpAdaptiveYield_RT")->Visible=CosFunction.bAdaptiveYield;

    if(EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled==false)                                             //kevin 20131030 權限不足不使用false
    {
        EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")     ->Enabled=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Enabled=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")   ->Enabled=false;
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")->Enabled=false;
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")   ->Enabled=false;
    }
    else                                                                        //Steven 20120804 Start: 在最下面決定權限能不能Enable
    {
        EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")      ->Enabled=fSecurity->Insufficient(39, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT") ->Enabled=fSecurity->Insufficient(39, false);
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")    ->Enabled=fSecurity->Insufficient(39, false);
    }

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Enabled=false;
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")->Enabled=false;

        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Enabled=false;
        EL<TEdit>("TfYieldMonitoring", "edContactCountRT")->Enabled=false;

//        gbYieldAlarmRT->Enabled=false;   //JerryYang 20221130 :
//        gbAlarmRT->Enabled=false;
    }

    if(CUSTOMER_CODE!=CC_Greatek)
    {
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;   // golden uYieldMonitoring.cpp:2751-2751 取代：三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2751-2751 -- 三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")    ->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled;   // golden uYieldMonitoring.cpp:2752-2752 取代：三元式 :gbPiggyBack 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2752-2752 -- 三元式 :gbPiggyBack 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")     ->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:gbPiggyBack   ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")->Enabled;   // golden uYieldMonitoring.cpp:2753-2753 取代：三元式 :gbAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2753-2753 -- 三元式 :gbAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")         ->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:gbAlarm       ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")->Enabled;   // golden uYieldMonitoring.cpp:2754-2754 取代：三元式 :gbYieldAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2754-2754 -- 三元式 :gbYieldAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")  ->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarmRT->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")->Enabled;   // golden uYieldMonitoring.cpp:2755-2755 取代：三元式 :gbPiggyBackRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2755-2755 -- 三元式 :gbPiggyBackRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")   ->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:gbPiggyBackRT ->Enabled;
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")->Enabled;   // golden uYieldMonitoring.cpp:2756-2756 取代：三元式 :gbAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2756-2756 -- 三元式 :gbAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")       ->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:gbAlarmRT     ->Enabled;
#endif // GATE (S12-C save)

        //jou 20180130 : 新增 BySiteByBinPercentCompare level 管控
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;   // golden uYieldMonitoring.cpp:2759-2759 取代：三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2759-2759 -- 三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;
#endif // GATE (S12-C save)
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;   // golden uYieldMonitoring.cpp:2760-2760 取代：三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2760-2760 -- 三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫
        EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;
#endif // GATE (S12-C save)
        if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)          //JerryYang 20220311 : ATP鎖定Critical parameter
        {
            if(bAuthCriticalPara[22])
            {
                EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")->Enabled=false;
            }
        }
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[16])
        {
            EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled=false;
        }
    }

    if(EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")     ->Enabled==false)                                      //kevin 20131030 權限不足不使用false
    {
         EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")     ->Enabled=false;
         EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Enabled=false;
         EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")   ->Enabled=false;
         EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")->Enabled=false;
         EL<TEdit>("TfYieldMonitoring", "edContactCountRT")   ->Enabled=false;
    }
    else
    {
        EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")      ->Enabled=fSecurity->Insufficient(39, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT") ->Enabled=fSecurity->Insufficient(39, false);
        EL<TEdit>("TfYieldMonitoring", "edContactCountRT")    ->Enabled=fSecurity->Insufficient(39, false);
    }

    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
    {
        EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")     ->Enabled=false;
        EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")     ->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE ||                                            //20140320 wei
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       CUSTOMER_CODE==CC_UTAC_TW)                                               //Sam 20230106 : 聯測廠新增 Yield 權限卡控
    {
        if(AccessLevel<LevelSet.AccessLevel[39])
        {
            EL<TTabSheet>("TfYieldMonitoring", "tsNormal")->Enabled=false;
            EL<TTabSheet>("TfYieldMonitoring", "tsReTest")->Enabled=false;
            EL<TTabSheet>("TfYieldMonitoring", "tsAutoSiteOff")->Enabled=false;                                       //Sam 20230106 : 新增 AutoSiteOff
        }
        else
        {
            EL<TTabSheet>("TfYieldMonitoring", "tsNormal")->Enabled=true;
            EL<TTabSheet>("TfYieldMonitoring", "tsReTest")->Enabled=true;
            EL<TTabSheet>("TfYieldMonitoring", "tsAutoSiteOff")->Enabled=true;                                        //Sam 20230106 : 新增 AutoSiteOff
        }
    }

    if(CUSTOMER_CODE==CC_Greatek)
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_FT")                       ->Enabled=fSecurity->Insufficient(118, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_FT")             ->Enabled=fSecurity->Insufficient(118, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_RT")                       ->Enabled=fSecurity->Insufficient(118, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_RT")             ->Enabled=fSecurity->Insufficient(118, false);

        EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")                       ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT")             ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT")                     ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_FT")           ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT")             ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edLowYield_RT")                       ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_RT")                     ->Enabled=fSecurity->Insufficient(81, false);
        EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_RT")           ->Enabled=fSecurity->Insufficient(81, false);

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_FT")                 ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")                    ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_FT")                 ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")                 ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")                 ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")                    ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_RT")                 ->Enabled=fSecurity->Insufficient(119, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_RT")                 ->Enabled=fSecurity->Insufficient(119, false);

        EL<TEdit>("TfYieldMonitoring", "edContinuousPass_FT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")                    ->Enabled=fSecurity->Insufficient(82, false);
        EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT")                ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuousPass_RT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT")                 ->Enabled=fSecurity->Insufficient(82, false);
        EL<TEdit>("TfYieldMonitoring", "edContactCountRT")                    ->Enabled=fSecurity->Insufficient(82, false);
        EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT")                ->Enabled=fSecurity->Insufficient(82, false);

        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn")            ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")           ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn")              ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")             ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOn")            ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOff")           ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOn")              ->Enabled=fSecurity->Insufficient(120, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOff")             ->Enabled=fSecurity->Insufficient(120, false);

        EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT")         ->Enabled=fSecurity->Insufficient(83, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")           ->Enabled=fSecurity->Insufficient(83, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT")         ->Enabled=fSecurity->Insufficient(83, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT")           ->Enabled=fSecurity->Insufficient(83, false);
    }

    if(CosFunction.bYieldAlarm4)                                                //Sam 20191109 YieldAlarm4 Fix
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType")                ->Enabled=fSecurity->Insufficient(121, false);
        EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield")        ->Enabled=fSecurity->Insufficient(121, false);
        EL<TCheckBox>("TfYieldMonitoring", "cb_SiteToSiteYieldEnable")            ->Enabled=fSecurity->Insufficient(121, false);
        EL<TCheckBox>("TfYieldMonitoring", "cb_HeadToHeadYieldEnable")            ->Enabled=fSecurity->Insufficient(121, false);
        EL<TCheckBox>("TfYieldMonitoring", "cb_SiteYieldOverAlert")               ->Enabled=fSecurity->Insufficient(121, false);

        EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount")               ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount")               ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount") ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount") ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")         ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")                  ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")                  ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert")               ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount")             ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount")             ->Enabled=fSecurity->Insufficient(117, false);
        EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount")          ->Enabled=fSecurity->Insufficient(117, false);
    }

    if(CosFunction.bYieldAlarm5)                                                //Sam 20191109 YieldAlarm5 Fix
    {                                                                           //Sam 20171213 (Steven) : 超豐良率監控 //Sam 20180423 (wei) : MOFile of Yeild Download
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteLowYieldEnable")       ->Enabled=fSecurity->Insufficient(153, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteCmpYieldEnable")       ->Enabled=fSecurity->Insufficient(153, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteAlarmYieldEnable")     ->Enabled=fSecurity->Insufficient(153, false);
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySitePreCmpYieldEnable")    ->Enabled=fSecurity->Insufficient(153, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_OSBin")                      ->Enabled=false;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYield")             ->Enabled=fSecurity->Insufficient(154, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYieldRej")          ->Enabled=fSecurity->Insufficient(154, false);                              //Sam 20180423 (wei) : MOFile of Yeild Download
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYield")           ->Enabled=fSecurity->Insufficient(154, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYieldRej")        ->Enabled=fSecurity->Insufficient(154, false);                              //Sam 20180423 (wei) : MOFile of Yeild Download
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteIntervalContactCnt")   ->Enabled=fSecurity->Insufficient(154, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYield")             ->Enabled=fSecurity->Insufficient(154, false);
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYieldRej")          ->Enabled=fSecurity->Insufficient(154, false);                              //Sam 20180423 (wei) : MOFile of Yeild Download
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYield")          ->Enabled=fSecurity->Insufficient(154, false);                              //Sam 20180423 (wei) : MOFile of Yeild Download
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYieldRej")       ->Enabled=fSecurity->Insufficient(154, false);                              //Sam 20180423 (wei) : MOFile of Yeild Download
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Alick 20160705 add ASE CL 啟用功能與計數權限追加
    {
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn")    ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")   ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn")      ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")     ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOn")    ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOff")   ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOn")      ->Enabled=fSecurity->Insufficient(125, false);
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOff")     ->Enabled=fSecurity->Insufficient(125, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT") ->Enabled=fSecurity->Insufficient(126, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")   ->Enabled=fSecurity->Insufficient(126, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT") ->Enabled=fSecurity->Insufficient(126, false);
        EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT")   ->Enabled=fSecurity->Insufficient(126, false);

        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn")->Enabled=false;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Enabled=false;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn")->Enabled=false;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Enabled=false;
    }

    if(IniConfig.bE53LowYieldAutoClean)                                         //kevin 20160802   //wei 20141201 Low Yield Auto Clean(%)
        EL<TGroupBox>("TfYieldMonitoring", "gbLowYieldAutoClean")->Visible=true;
    else
        EL<TGroupBox>("TfYieldMonitoring", "gbLowYieldAutoClean")->Visible=false;

    EL<TGroupBox>("TfYieldMonitoring", "grpSpecialLowYield")->Visible=CosFunction.bSpecailLowYeild;                   //Sam 20210505 : PTI 要求的兩段 Low Yeild

    if(IniConfig.bD67LoadCellMeasure)                                           //kevin 20190907 Arm 測區次數道量測 功能;
        EL<TGroupBox>("TfYieldMonitoring", "gbLoadcell")->Visible=true;
    else
        EL<TGroupBox>("TfYieldMonitoring", "gbLoadcell")->Visible=false;

    if(CosFunction.bYieldControlUseContactCount)                                //Steven 20141212 : Yield控制使用Contact Count
    {
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_FT")             ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_FT")   ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_FT")         ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_RT")             ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_RT")   ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_RT")         ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labLowYieldByTotal1_FT")      ->Caption="%  after contact count";         //Kaichen 20190628 : Low Yield ByTotal
        EL<TLabel>("TfYieldMonitoring", "labLowYieldByTotal1_RT")      ->Caption="%  after contact count";         //Kaichen 20190628 : Low Yield ByTotal
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_FT")      ->Caption="%  after contact count";         //Steven 20230223 : 根據Index吸嘴比較良率
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_RT")      ->Caption="%  after contact count";
    }
    else if(CosFunction.bYieldControlUseEACount)                                //wei 20151111  //wei 20180606 Yield控制使用EA Count
    {
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_FT")             ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_FT")   ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_FT")         ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_RT")             ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_RT")   ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_RT")         ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labLowYieldByTotal1_FT")      ->Caption="%  Ignore IC count";             //Kaichen 20190628 : Low Yield ByTotal
        EL<TLabel>("TfYieldMonitoring", "labLowYieldByTotal1_RT")      ->Caption="%  Ignore IC count";             //Kaichen 20190628 : Low Yield ByTotal
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_FT")      ->Caption="%  Ignore IC count";             //Steven 20230223 : 根據Index吸嘴比較良率
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_RT")      ->Caption="%  Ignore IC count";
    }
    else if(CUSTOMER_CODE==CC_JCET)                                             //JerryYang 20170420 (Steven) JCET吳如春要求修改名稱
    {
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_FT")             ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_FT")   ->Caption="%  after per site count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_FT")         ->Caption="%  after per site count";
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_RT")             ->Caption="%  Ignore IC count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_RT")   ->Caption="%  after per site count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_RT")         ->Caption="%  after per site count";
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_FT")      ->Caption="%  after per site count";        //Steven 20230223 : 根據Index吸嘴比較良率
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_RT")      ->Caption="%  after per site count";
    }
    else
    {
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_FT")             ->Caption="%  after test count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_FT")   ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_FT")         ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labLowYield1_RT")             ->Caption="%  after test count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldDifferent1_RT")   ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "labSiteYieldCmp1_RT")         ->Caption="%  after contact count";
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_FT")      ->Caption="%  after contact count";         //Steven 20230223 : 根據Index吸嘴比較良率
        EL<TLabel>("TfYieldMonitoring", "lblLowYieldByPicker_RT")      ->Caption="%  after contact count";
    }

    if(CosFunction.bYieldAlarmNoWait1Min)                                       //wei 20150820  Yield Alarm No Wait 1Min
    {
        EL<TLabel>("TfYieldMonitoring", "lblCheckInteval")->Caption="";
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_FT")              ->Caption="Low Yields%(By Site)";           //Jimmychiu 20230524 : 補回顯示內容
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT")    ->Caption="By Arm Per Site Differ Yield%";
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_FT")          ->Caption="By Site Compare Yield%";
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_RT")              ->Caption="Low Yields%(By Site)";           //Ifor 20171117 : FT => RT
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT")    ->Caption="By Arm Per Site Differ Yield%";
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_RT")          ->Caption="By Site Compare Yield%";
    }
    else                                                                        //Steven 20230223 : 整理Yield顯示畫面
    {
        if(CosFunction.bYieldAlarmCheckByCount)                                 //AI(rf360-yield-count) 20260814 (RogerYang) : RF360固定顯示by Count
        {
            Str="Check Interval by Count";
        }
        else if(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&
           IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)
        {
            Str.sprintf("Check Interval : %d Sec.", IniConfig.iI44_LowYieldAlarmIntervalTime);
        }
        else
        {
            Str.sprintf("Check Interval : 1 Min.", IniConfig.iI44_LowYieldAlarmIntervalTime);
        }

        EL<TLabel>("TfYieldMonitoring", "lblCheckInteval")->Caption=Str;
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_FT")              ->Caption="Low Yields% (1min)";             //Jimmychiu 20230524 : 補回顯示內容
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT")    ->Caption="By Arm Per Site Differ Yield% (1min)";
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_FT")          ->Caption="By Site Compare Yield% (1min)";
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_RT")              ->Caption="Low Yields% (1min)";             //Ifor 20171117 : FT => RT
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT")    ->Caption="By Arm Per Site Differ Yield% (1min)";
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldCmp_RT")          ->Caption="By Site Compare Yield% (1min)";
    }
    EL<TEdit>("TfYieldMonitoring", "edtYieldAlarmCheckIntervalByCount")->Visible=CosFunction.bYieldAlarmCheckByCount; //AI(rf360-yield-count) 20260814 (RogerYang) : 只有RF360顯示
    if(CosFunction.bYieldSettingNeedPassword)                                   //AI(rf360-yield-password) 20260814 (RogerYang) : RF360鎖定Yield Alarm區塊,按Unlock輸密碼才能操作
    {
        if(bYieldAlarmUnlocked==false)
        {
            EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")  ->Enabled=false;
            EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")->Enabled=false;
        }
        EL<TButton>("TfYieldMonitoring", "btnUnlockYieldAlarm")  ->Visible=(bYieldAlarmUnlocked==false);
        EL<TButton>("TfYieldMonitoring", "btnUnlockYieldAlarmRT")->Visible=(bYieldAlarmUnlocked==false);
    }

    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByTotal_FT")->Visible=(CosFunction.bLowYeildByTotal);              //wei 20151116 Low Yield By Total
    EL<TGroupBox>("TfYieldMonitoring", "grpLowYieldByTotal_RT")->Visible=(CosFunction.bLowYeildByTotal);              //wei 20151116 Low Yield By Total
    EL<TPanel>("TfYieldMonitoring", "palConsFailIgnore_FT") ->Visible=(CUSTOMER_CODE==CC_KYEC_CHEN);               //wei 20160115 銅鑼前幾顆不計算ContsFail
    EL<TPanel>("TfYieldMonitoring", "palConsFailIgnore_RT") ->Visible=(CUSTOMER_CODE==CC_KYEC_CHEN);               //wei 20160115 銅鑼前幾顆不計算ContsFail
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT")->Visible=(CosFunction.bBySiteByBinPercentCompare==true && CosFunction.bByBinAlarmFromYieldForm);                            //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT")->Visible=(CosFunction.bBySiteByBinPercentCompare==true && CosFunction.bByBinAlarmFromYieldForm);                            //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form

    if(IniConfig.bSIGURDFunction)                                               //KaiChen 20190626 ：矽格要求 LowYieldByTotal & SiteYieldCmp 開關同步
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_FT")->Enabled=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_RT")->Enabled=false;
        if(CUSTOMER_CODE!=CC_UTAC_TW)                                           //Sam 20230106 :　聯測孝文要求可以編輯數量
        {
            EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_FT")->Enabled=false;
            EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_RT")->Enabled=false;
        }
    }

    if(bUseTwoArm32Site==true)                                                  //Steven 20200526 : 4x4 & 4x8 不需要比by arm
    {
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT")->Visible=false;
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT")->Visible=false;
    }
    else
    {
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT")->Visible=true;
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT")->Visible=true;
    }

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)                                         //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
    {
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_FT")->Visible=false;
        EL<TGroupBox>("TfYieldMonitoring", "grpSiteYieldDifferent_RT")->Visible=false;
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_FT")->Visible=false;                                          //Steven 20230224 : Add
        EL<TGroupBox>("TfYieldMonitoring", "grpLowYield_RT")->Visible=false;

        EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail")       ->Visible=false;                                    //KaiChen 20200603 ：矽格-湖口，Yield 頁面 AllSiteFail 不使用
        EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount")  ->Visible=false;                                    //KaiChen 20200603 ：矽格-湖口，Yield 頁面 AllSiteFail 不使用
        EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT")->Visible=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT")    ->Visible=false;                                    //KaiChen 20200603 ：矽格-湖口，Yield 頁面 AllSiteFail 不使用
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")     ->Visible=false;                                        //KaiChen 20201130 ：矽格-湖口，Yield 頁面 Piggy-Back Functions 不顯示
        EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")   ->Visible=false;                                        //KaiChen 20201130 ：矽格-湖口，Yield 頁面 Piggy-Back Functions 不顯示
    }

    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildBySite_FT")->Visible =(CosFunction.IntervalYieldCount && CUSTOMER_CODE!=CC_TSI);                   //wei 20180606 Interval Low Yield By Site
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildBySite_RT")->Visible =(CosFunction.IntervalYieldCount && CUSTOMER_CODE!=CC_TSI);                   //wei 20180606 Interval Low Yield By Site
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildByTotal_FT")->Visible=(CosFunction.IntervalYieldCount);    //wei 20180606 Interval Low Yield By Total
    EL<TPanel>("TfYieldMonitoring", "palIntervalLowYeildByTotal_RT")->Visible=(CosFunction.IntervalYieldCount);    //wei 20180606 Interval Low Yield By Total

    EL<TGroupBox>("TfYieldMonitoring", "grpCreateManualEOCAP")->Visible=CosFunction.bCreateManualEOCAP;               //jou 20221104 : VTest CreateManualEOCAP function;

    fShow=true;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden uYieldMonitoring.cpp:900  TfYieldMonitoring::ReadFile()
static void YM_ReadFile()
{

    AnsiString szDir=GetRecipeFileName("Tester.Data");
    YM_SetClosedSiteBin();                                                         //Steven 20250424 : Fixed for display

//jou 980716 start : add FT/RT alarm
//FT
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Alick 20160616 by ASE CL
    {
//        TestIF_File.bContsFailBySocket              =ReadIniData(szDir, "Alarm", "SocketEnable", 1);
//        TestIF_File.bContsFailByHead                =ReadIniData(szDir, "Alarm", "HeadEnable",   1);
        TestIF_File.bContsFailBySocket              =true;                      //JerryYang 20250120 : ASECL SONG要求強制開啟
        TestIF_File.bContsFailByHead                =true;
    }
    else
    {
        TestIF_File.bContsFailBySocket              =ReadIniData(szDir, "Alarm", "SocketEnable", 0);
        TestIF_File.bContsFailByHead                =ReadIniData(szDir, "Alarm", "HeadEnable",   0);
    }
    TestIF_File.iContsFailSocketAlarmCT             =ReadIniData(szDir, "Alarm", "SocketCT",     5);
    TestIF_File.iContsFailHeadAlarmCT               =ReadIniData(szDir, "Alarm", "HeadCT",       5);

    if(CUSTOMER_CODE==CC_AMKOR_China ||                                         //jou 2015-05-28 Amkor-China 要求將By Bin Count Fail改成連續Fail才Alarm
       CUSTOMER_CODE==CC_QUALCOMM ||                                            //Sam 20200704 : Add Greatek
       CUSTOMER_CODE==CC_Greatek  ||                                            //JerryYang 20170412 (Steven) add QUALCOMM
       CUSTOMER_CODE==CC_HANA_MICRON)                                           //Steven 20230529 : Hana希望改成continue fail
    {
        TestIF_File.bCountSpcBinContinuously_FT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_FT", true);  //Steven 20230529 : Spc Bin Couont改成連續錯誤
        TestIF_File.bCountSpcBinContinuously_RT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_RT", true);
    }
    else
    {
        TestIF_File.bCountSpcBinContinuously_FT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_FT", false);
        TestIF_File.bCountSpcBinContinuously_RT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_RT", false);
    }

//Site Yield Alarm(%)
    if(bUseTwoArm32Site==true ||                                                //Steven 20220419 : NN mode不需要比by arm
       CUSTOMER_CODE==CC_SIGURD_HUKOU ||                                        //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)
    {
        TestIF_File.bFailAlarmSiteYieldDifferent    =false;
    }
    else
    {
        TestIF_File.bFailAlarmSiteYieldDifferent    =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different",         false);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dFailAlarmSiteYield         =ReadIniData(szDir, "Site Yield Alarm", "Site Yield",                       6.0);
            TestIF_File.dFailAlarmSiteYield_RT      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6.0);
        }
        else
        {
            TestIF_File.dFailAlarmSiteYield         =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield",                     6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYield_RT      =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield RT",                  6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        TestIF_File.iFailAlarmSiteYield             =0;
        TestIF_File.iFailAlarmSiteYield_RT          =0;
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iFailAlarmSiteYield             =ReadIniData(szDir, "Site Yield Alarm", "Site Yield",                       6);
        TestIF_File.iFailAlarmSiteYield_RT          =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6);
        TestIF_File.dFailAlarmSiteYield             =double(TestIF_File.iFailAlarmSiteYield);
        TestIF_File.dFailAlarmSiteYield_RT          =double(TestIF_File.iFailAlarmSiteYield_RT);
    }
    TestIF_File.iFailAlarmSiteYieldDifferentCount   =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different Count",    1000);

    if(bUseTwoArm32Site==true ||                                                //Steven 20220419 : NN mode不需要比by arm
       CUSTOMER_CODE==CC_SIGURD_HUKOU ||                                        //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)
    {
        TestIF_File.bFailAlarmSiteYieldDifferent_RT =false;
    }
    else
    {
        TestIF_File.bFailAlarmSiteYieldDifferent_RT =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different RT",      false);
    }
    TestIF_File.iFailAlarmSiteYield_RT              =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6);
    TestIF_File.iFailAlarmSiteYieldDifferentCount_RT=ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different Count RT", 1000);

    if(CosFunction.bLowYieldAutoSiteOff)                                        //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    {
        TestIF_File.bLowYieldAutoSiteOff            =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off",      false);
        TestIF_File.iAlarmWhenSiteOnCountLess       =ReadIniData(szDir, "Site Yield Alarm", "Alarm When Site On Count Less Than", 2);
        TestIF_File.bLowYieldAutoSiteOffByContiFail =ReadIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffByContiFail", false);                          //Steven 20200420 : Continue fail, auto site off
        TestIF_File.bLowYieldAutoSiteOffArmContiFail=ReadIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffArmContiFail", false);                         //Steven 20220818 : By Arm Continue fail, auto site off

        TestIF_File.bLowYieldAutoSiteOffAlarm       =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm",      false);                       //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        TestIF_File.iLowYieldAutoSiteOffAlarm       =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm Count", 2);                          //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        if(bUseTwoArm32Site==true)                                              //Steven 20220419 : NN mode不需要比by arm
        {
            TestIF_File.bLowYieldAutoSiteOffByArmSite=false;
            TestIF_File.bLowYieldAutoSiteOffByPicker =false;
        }
        else
        {
            TestIF_File.bLowYieldAutoSiteOffByArmSite=ReadIniData(szDir, "Site Yield Alarm", "By Arm Site Low Yield Auto Site Off ", false);                    //Steven 20230223 : by arm by site, auto site off
            TestIF_File.bLowYieldAutoSiteOffByPicker =ReadIniData(szDir, "Site Yield Alarm", "By Picker Low Yield Auto Site Off ", false);                      //Steven 20230223 : 根據Index吸嘴比較良率
        }

        if(IniConfig.bI28_OnOffSiteOnTheFly ||
           TestIF_File.bLowYieldAutoSiteOff ||
           TestIF_File.bLowYieldAutoSiteOffByContiFail)                         //Steven 20200420 : Continue fail, auto site off
        {
            TestIF_File.iCloseSiteOnHPDontTest      =ReadIniData(szDir, "Site Yield Alarm", "HP Close Site Do Not Test", 0);                                    //JerryYang 20180723 (wei) 關site的位置不測試送error bin
            TestIF_File.iCloseSiteBin               =ReadIniData(szDir, "Site Yield Alarm", "Bin of Closed Site", iTestBinCount);                               //Steven 20240409 : 關site的位置有IC不測試送指定 bin
        }
        else
        {
            TestIF_File.iCloseSiteBin               =iTestBinCount;             //Steven 20250604 : 關site的位置有IC不測試送指定 bin
            TestIF_File.iCloseSiteOnHPDontTest      =0;                         //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
        }
    }
    else
    {
        TestIF_File.iCloseSiteBin                   =iTestBinCount;             //Steven 20250604 : 關site的位置有IC不測試送指定 bin
        TestIF_File.iCloseSiteOnHPDontTest          =0;                         //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
        TestIF_File.bLowYieldAutoSiteOff            =false;
    }

//Site Yield Alarm(%)
    TestIF_File.bFailAlarmSiteYieldCmp              =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare",           false);

    TestIF_File.iFailAlarmSiteYieldCmpCount         =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare Count",      1000);
    TestIF_File.bFailAlarmSiteYieldCmp_RT           =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare RT",        false);

    TestIF_File.iFailAlarmSiteYieldCmpCount_RT      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare Count RT",   1000);
    filerw::ELTodo("golden uYieldMonitoring.cpp:1033 TestIF_File.iYieldAlarmCheckIntervalByCount (912 rf360) missing from port SYSTEM_TEST_IF -- [Site Yield Alarm] Check Interval By Count not read/shown");   // golden uYieldMonitoring.cpp:1033-1033 取代：TestIF_File.iYieldAlarmCheckIntervalByCount：912 欄位（rf360），移植樹 SYSTEM_TEST_IF（cprod.h）沒有
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:1033-1033 -- TestIF_File.iYieldAlarmCheckIntervalByCount：912 欄位（rf360），移植樹 SYSTEM_TEST_IF（cprod.h）沒有
    TestIF_File.iYieldAlarmCheckIntervalByCount     =ReadIniData(szDir, "Site Yield Alarm", "Check Interval By Count",         20);   //AI(rf360-yield-count) 20260814 (RogerYang)
#endif // GATE (S12-C save)

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dFailAlarmSiteYieldCmp      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp",                 6.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYieldCmp_RT   =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp RT",              6.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        else
        {
            TestIF_File.dFailAlarmSiteYieldCmp      =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield Cmp",                 6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYieldCmp_RT   =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield Cmp RT",              6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iFailAlarmSiteYieldCmp          =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp",                   6);
        TestIF_File.iFailAlarmSiteYieldCmp_RT       =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp RT",                6);
        TestIF_File.dFailAlarmSiteYieldCmp          =double(TestIF_File.iFailAlarmSiteYieldCmp);
        TestIF_File.dFailAlarmSiteYieldCmp_RT       =double(TestIF_File.iFailAlarmSiteYieldCmp_RT);
    }

//Low Yield Alarm(%)
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)                                         //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
    {
        TestIF_File.bFailAlarmLowYield              =false;
    }
    else
    {
        TestIF_File.bFailAlarmLowYield              =ReadIniData(szDir, "Low Yield Alarm", "Enable",   0);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimit              =ReadIniData(szDir, "Low Yield Alarm", "Limit",   95.0);            //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        else
        {
            TestIF_File.dLowYieldLimit              =ReadIniData(szDir, "Low Yield Alarm", "Double Limit",   95.0);     //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimit                  =ReadIniData(szDir, "Low Yield Alarm", "Limit",   95);
        TestIF_File.dLowYieldLimit                  =double(TestIF_File.iLowYieldLimit);
    }
    TestIF_File.iLowYieldCount                      =ReadIniData(szDir, "Low Yield Alarm", "Count", 1000);
    TestIF_File.bLowYieldAlarmByBin                 =ReadIniData(szDir, "Low Yield Alarm", "Enable By Bin Setting", false);                                     //Steven 20140828 : By Bin Yield Monitor
    if(CosFunction.bUseLowYieldAlarmByBin==false)                               //Steven 20140828 : By Bin Yield Monitor
        TestIF_File.bLowYieldAlarmByBin=false;
    TestIF_File.bSlidingWindowYield                 =ReadIniData(szDir, "Low Yield Alarm", "Sliding Window Enable", false);                                     //Steven 20260331 : Sliding Window Yield
    TestIF_File.iSlidingWindowSize                  =ReadIniData(szDir, "Low Yield Alarm", "Sliding Window Size",   400);                                       //Steven 20260331

    TestIF_File.bFailRateMode                       =ReadIniData(szDir, "Alarm", "RateEnable",      0);
//    TestIF_File.iIgnoreIC                           =ReadIniData(szDir, "Alarm", "Ignored IC",   1000);
    TestIF_File.iCountAlarmAction                   =ReadIniData(szDir, "Alarm", "Count Action", 0);                    //Steven 20101116
    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
    {
        TestIF_File.iCountAlarmAction               =1;
    }
    TestIF_File.bContinuousPass                     =ReadIniData(szDir, "Alarm", "Continuous Pass",  false);            //Eliot 20100708
    TestIF_File.iContinuousPassBin                  =ReadIniData(szDir, "Alarm", "Continuous Pass Bin", 1000);          //Eliot 20100708
    TestIF_File.iContinuousPassBinCount             =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count", 1000);    //Eliot 20100708

//Continuous Pass by Socket - Steven 20110915
    TestIF_File.bContinuousPassBySocket             =ReadIniData(szDir, "Alarm", "Continuous Pass By Socket",               false);
    TestIF_File.iContinuousPassBinCountBySocket     =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count By Socket",     1000);

//RT
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Alick 20160616 by ASE CL
    {
        TestIF_File.bContsFailBySocket_RT           =ReadIniData(szDir, "Alarm", "SocketEnable RT", 1);
        TestIF_File.bContsFailByHead_RT             =ReadIniData(szDir, "Alarm", "HeadEnable RT",   1);
    }
    else
    {
        TestIF_File.bContsFailBySocket_RT           =ReadIniData(szDir, "Alarm", "SocketEnable RT", 0);
        TestIF_File.bContsFailByHead_RT             =ReadIniData(szDir, "Alarm", "HeadEnable RT",   0);
    }
    TestIF_File.iContsFailSocketAlarmCT_RT          =ReadIniData(szDir, "Alarm", "SocketCT RT",     5);
    TestIF_File.iContsFailHeadAlarmCT_RT            =ReadIniData(szDir, "Alarm", "HeadCT RT",       5);

//Consecutive Failure Ignore                                                    //wei 20160115 銅鑼前幾顆不計算ContsFail
    TestIF_File.bContsFailIgnore                    =ReadIniData(szDir, "Alarm", "ContsFailIgnoreEnable", 0);
    TestIF_File.iContsFailIgnore                    =ReadIniData(szDir, "Alarm", "ContsFailIgnoreCT",     5);

    TestIF_File.bContsFailIgnore_RT                 =ReadIniData(szDir, "Alarm", "ContsFailIgnoreEnable RT", 0);
    TestIF_File.iContsFailIgnore_RT                 =ReadIniData(szDir, "Alarm", "ContsFailIgnoreCT RT",     5);

//Low Yield Alarm(%)
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)                                         //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
    {
        TestIF_File.bFailAlarmLowYield_RT           =false;
    }
    else
    {
        TestIF_File.bFailAlarmLowYield_RT           =ReadIniData(szDir, "Low Yield Alarm", "Enable RT",   false);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimit_RT           =ReadIniData(szDir, "Low Yield Alarm", "Limit RT",    95.0);
        }
        else
        {
            TestIF_File.dLowYieldLimit_RT           =ReadIniData(szDir, "Low Yield Alarm", "Double Limit RT",    95.0);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimit_RT               =ReadIniData(szDir, "Low Yield Alarm", "Limit RT",    95);
        TestIF_File.dLowYieldLimit_RT               =double(TestIF_File.iLowYieldLimit_RT);
    }

    TestIF_File.iLowYieldCount_RT                   =ReadIniData(szDir, "Low Yield Alarm", "Count RT",  1000);

    TestIF_File.bFailRateMode_RT                    =ReadIniData(szDir, "Alarm", "RateEnable RT",                   false);
//    TestIF_File.iIgnoreIC_RT                        =ReadIniData(szDir, "Alarm", "Ignored IC RT",                   1000);
    TestIF_File.iCountAlarmAction_RT                =ReadIniData(szDir, "Alarm", "Count Action RT",                 0);                                         //Steven 20101116
    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
    {
        TestIF_File.iCountAlarmAction_RT            =1;
    }
    TestIF_File.bContinuousPass_RT                  =ReadIniData(szDir, "Alarm", "Continuous Pass RT",              false);                                     //Eliot 20100708
    TestIF_File.iContinuousPassBin_RT               =ReadIniData(szDir, "Alarm", "Continuous Pass RT Bin",          1);                                         //Eliot 20100708
    TestIF_File.iContinuousPassBinCount_RT          =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT",    1000);                                      //Eliot 20100708

//Continuous Pass by Socket - Steven 20110915
    TestIF_File.bContinuousPassBySocket_RT          =ReadIniData(szDir, "Alarm", "Continuous Pass RT By Socket",            false);
    TestIF_File.iContinuousPassBinCountBySocket_RT  =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT By Socket",  1000);

//Low Yield Alarm(By Total)(%)                                                  //wei 20151116 Low Yield By Total
    TestIF_File.bFailAlarmLowYieldByTotal           =ReadIniData(szDir, "Low Yield Alarm", "By Total Enable", false);
    TestIF_File.iLowYieldCountByTotal               =ReadIniData(szDir, "Low Yield Alarm", "By Total Count", 1000);
    TestIF_File.bFailAlarmLowYieldByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Total Enable RT",   false);
    TestIF_File.iLowYieldCountByTotal_RT            =ReadIniData(szDir, "Low Yield Alarm", "By Total Count RT",  1000);
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimitByTotal       =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit",          95.0);                                    //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dLowYieldLimitByTotal_RT    =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit RT",       95.0);
        }
        else
        {
            TestIF_File.dLowYieldLimitByTotal       =ReadIniData(szDir, "Low Yield Alarm", "Double By Total Limit",          95.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dLowYieldLimitByTotal_RT    =ReadIniData(szDir, "Low Yield Alarm", "Double By Total Limit RT",       95.0);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit",   95);
        TestIF_File.iLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit RT",    95);
        TestIF_File.dLowYieldLimitByTotal           =double(TestIF_File.iLowYieldLimitByTotal);
        TestIF_File.dLowYieldLimitByTotal_RT        =double(TestIF_File.iLowYieldLimitByTotal_RT);
    }

//By Picker Compare Yield   [每個吸嘴的良率互比]                                //Steven 20230223 : 根據Index吸嘴比較良率
    TestIF_File.bLowYieldByPicker           =ReadIniData(szDir, "Low Yield Alarm", "By Picker Enable",      false);
    TestIF_File.iLowYieldCountByPicker      =ReadIniData(szDir, "Low Yield Alarm", "By Picker Count",       1000);
    TestIF_File.dLowYieldByPicker           =ReadIniData(szDir, "Low Yield Alarm", "By Picker Limit",       95.0);
    TestIF_File.bLowYieldByPicker_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Picker Enable RT",   false);
    TestIF_File.iLowYieldCountByPicker_RT   =ReadIniData(szDir, "Low Yield Alarm", "By Picker Count RT",    1000);
    TestIF_File.dLowYieldByPicker_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Picker Limit RT",    95.0);

    if(CosFunction.bSpecailLowYeild)                                            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        TestIF_File.bFailAlarmLowYieldSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Enable", false);
        TestIF_File.iLowYieldCountSpecial1          =ReadIniData(szDir, "Low Yield Alarm", "Special Count1", 1000);
        TestIF_File.iLowYieldCountSpecial2          =ReadIniData(szDir, "Low Yield Alarm", "Special Count2", 1000);
        if(CosFunction.bYieldAlarmUseDouble)
        {
            TestIF_File.dLowYieldLimitSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Limit",          95.0);
        }
        else
        {
            TestIF_File.iLowYieldLimitSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Limit",          95);
            TestIF_File.dLowYieldLimitSpecial       =double(TestIF_File.iLowYieldLimitSpecial);
        }
    }

//Interval Low Yield Alarm(By Site)(%)                                          //wei 20180606 Interval Low Yield By Site
    if(CosFunction.IntervalYieldCount==false)                                   //Steven 20230223 : 隱藏沒用到的
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite=false;
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT=false;
        TestIF_File.bFailAlarmIntervalLowYieldByTotal=false;
        TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT=false;
    }
    else
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Enable", false);
        TestIF_File.iIntervalLowYieldCountBySite                =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Count", 300);
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Enable RT",   false);
        TestIF_File.iIntervalLowYieldCountBySite_RT             =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Count RT",  300);
        if(CosFunction.bYieldAlarmUseDouble)                                    //JerryYang 20160615 Yield相關alarm設定到小數點
        {
            TestIF_File.dIntervalLowYieldLimitBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit",          95.0);         //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dIntervalLowYieldLimitBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit RT",       95.0);
        }
        else                                                                    //JerryYang 20160615 Yield相關alarm設定到整數
        {
            TestIF_File.iIntervalLowYieldLimitBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Limit",   95);
            TestIF_File.iIntervalLowYieldLimitBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Limit RT",    95);
            TestIF_File.dIntervalLowYieldLimitBySite            =double(TestIF_File.iIntervalLowYieldLimitBySite);
            TestIF_File.dIntervalLowYieldLimitBySite_RT         =double(TestIF_File.iIntervalLowYieldLimitBySite_RT);
        }

    //Interval Low Yield Alarm(By Total)(%)                                     //wei 20180718 Interval Low Yield By Total
        TestIF_File.bFailAlarmIntervalLowYieldByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Enable", false);
        TestIF_File.iIntervalLowYieldCountByTotal               =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Count", 300);
        TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Enable RT",   false);
        TestIF_File.iIntervalLowYieldCountByTotal_RT            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Count RT",  300);
        if(CosFunction.bYieldAlarmUseDouble)                                    //JerryYang 20160615 Yield相關alarm設定到小數點
        {
            TestIF_File.dIntervalLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit",          95.0);        //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dIntervalLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit RT",       95.0);
        }
        else                                                                    //JerryYang 20160615 Yield相關alarm設定到整數
        {
            TestIF_File.iIntervalLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Limit",   95);
            TestIF_File.iIntervalLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Limit RT",    95);
            TestIF_File.dIntervalLowYieldLimitByTotal           =double(TestIF_File.iIntervalLowYieldLimitByTotal);
            TestIF_File.dIntervalLowYieldLimitByTotal_RT        =double(TestIF_File.iIntervalLowYieldLimitByTotal_RT);
        }
    }

    if(CUSTOMER_CODE==CC_TSI)
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite=false;
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT=false;
    }
//jou 980716 end

//Steven 20090821 Start : Alarm Count
    TestIF_File.iACAlarmType                        =ReadIniData(szDir, "Alarm Count", "Alarm Type",    4);
    TestIF_File.iACGroupMethod                      =ReadIniData(szDir, "Alarm Count", "Group Method",  0);
    TestIF_File.iACPeriod                           =ReadIniData(szDir, "Alarm Count", "Period",        10);
    TestIF_File.iACCounts                           =ReadIniData(szDir, "Alarm Count", "Counts",        10);
//Steven 20090821 End

//jou 2010-11-17 Start
    TestIF_File.bContinuousLoader                   =ReadIniData(szDir, "Alarm", "Continuous Loader", false);
    TestIF_File.iContinuousLoaderCount              =ReadIniData(szDir, "Alarm", "Continuous Loader Count", 1000);

    TestIF_File.bContinuousLoader_RT                =ReadIniData(szDir, "Alarm", "Continuous Loader RT", false);
    TestIF_File.iContinuousLoaderCount_RT           =ReadIniData(szDir, "Alarm", "Continuous Loader Count RT", 1000);
//jou 2010-11-17 end

//Steven 20110426 Start
    TestIF_File.bContinuousContact                  =ReadIniData(szDir, "Alarm", "Continuous Contact", false);
    TestIF_File.iContinuousContactCount             =ReadIniData(szDir, "Alarm", "Continuous Contact Count", 1000);

    TestIF_File.bContinuousContact_RT               =ReadIniData(szDir, "Alarm", "Continuous Contact RT", false);
    TestIF_File.iContinuousContactCount_RT          =ReadIniData(szDir, "Alarm", "Continuous Contact Count RT", 1000);

    TestIF_File.bLoadCellMeasure                    =ReadIniData(szDir, "Alarm", "LoadCellMeasure", 0);                 //kevin 20190907 Arm 測區次數道量測 功能;
    TestIF_File.iLoadCellCount                      =ReadIniData(szDir, "Alarm", "LoadCellMeasureCount", 1000);         //kevin 20190907 Arm 測區次數道量測

    if(IniConfig.bD56YieldPiggyBackEnable)                                      //kevin 20131101 強致 Enable Yield 裡面piggyback 功能
    {
        TestIF_File.bContinuousContact           =true;
        TestIF_File.bContinuousContact_RT        =true;
        TestIF_File.bContinuousPassBySocket      =true;
        TestIF_File.bContinuousPassBySocket_RT   =true;
        TestIF_File.bContinuousLoader            =true;
        TestIF_File.bContinuousLoader_RT         =true;
        TestIF_File.bContinuousPass              =true;
        TestIF_File.bContinuousPass_RT           =true;
    }
//Steven 20110426 End

    if(CUSTOMER_CODE==CC_SCC)
    {
        TestIF_File.bFailRateMode               =false;
        TestIF_File.bFailRateMode_RT            =false;
//        TestIF_File.bContinuousPass             =false;                       //Steven 20210421 : Mark for JSCC request.
//        TestIF_File.bContinuousPass_RT          =false;
//        TestIF_File.bContinuousPassBySocket     =false;
//        TestIF_File.bContinuousPassBySocket_RT  =false;
//        TestIF_File.bContinuousLoader           =false;
//        TestIF_File.bContinuousLoader_RT        =false;
    }

    if(CosFunction.bPiggyBackForASE==true)                                      //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    {
        TestIF_File.bContinuousPass             =false;
        TestIF_File.bContinuousPass_RT          =false;
        TestIF_File.bContinuousLoader           =false;
        TestIF_File.bContinuousLoader_RT        =false;
    }

    YM_CheckSettingNo();                                                           //Steven 20110506
    if(IniConfig.bShowFunctionWindow) filerw::ELTodo("golden uYieldMonitoring.cpp:1337-1367 fTemperFrom SetShowYield/ShowYieldFuntion not ported (no fTemperFrom instance)");   // golden uYieldMonitoring.cpp:1337-1367 取代：fTemperFrom 的 function status 面板（SetShowYield／ShowYieldFuntion）：移植樹沒有 fTemperFrom 實體（forms/fTemperFrom.h:46；同 uYieldMonitoring.cpp GATE G-YM-Temper*）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:1337-1367 -- fTemperFrom 的 function status 面板（SetShowYield／ShowYieldFuntion）：移植樹沒有 fTemperFrom 實體（forms/fTemperFrom.h:46；同 uYieldMonitoring.cpp GATE G-YM-Temper*）

    if(IniConfig.bShowFunctionWindow)                                           //jou 2010-12-04 start
    {
        if((LastSet.iRunStartMode==rsmContinuStart ||
            LastSet.iRunStartMode==rsmInitialStart ||
            LastSet.iRunStartMode==rsmAutoSiteMap ||
            LastSet.iRunStartMode==rsmQAMode) ||                                //ChungHung 20120725 add QAMode 使用 Noraml
            (CosFunction.bUseSCKART &&
             (LastSet.iRunStartMode==rsmInitial_ART ||
              LastSet.iRunStartMode==rsmContinuStart_ART)))                     //Steven 20161214 (wei) : For SCK ART
        {
            //ChungHung add 20120730 若其中一個Item 都沒選 則秀OFF
            fTemperFrom->SetShowYield(fTemperFrom->esytDoubleDevice,    (TestIF_File.bContinuousPass   || TestIF_File.bContinuousPassBySocket ||
                                                                         TestIF_File.bContinuousLoader || TestIF_File.bContinuousContact));
            fTemperFrom->SetShowYield(fTemperFrom->esytCGoodBin,        TestIF_File.bContinuousPass==true || TestIF_File.bContinuousPassBySocket==true);        //Steven 20110920
            fTemperFrom->SetShowYield(fTemperFrom->esytYieldMonitor,    TestIF_File.bFailAlarmLowYield || TestIF_File.bFailAlarmSiteYieldCmp || TestIF_File.bFailAlarmSiteYieldDifferent);
            fTemperFrom->SetShowYield(fTemperFrom->esytConsAlarm,       TestIF_File.bContsFailBySocket==true || TestIF_File.bContsFailByHead==true);
        }
        else
        {
            //ChungHung add 20120730 若其中一個Item 都沒選 則秀OFF
            fTemperFrom->SetShowYield(fTemperFrom->esytDoubleDevice,    (TestIF_File.bContinuousPass_RT   || TestIF_File.bContinuousPassBySocket_RT ||
                                                                         TestIF_File.bContinuousLoader_RT || TestIF_File.bContinuousContact_RT));
            fTemperFrom->SetShowYield(fTemperFrom->esytCGoodBin,        TestIF_File.bContinuousPass_RT==true || TestIF_File.bContinuousPassBySocket_RT==true);  //Steven 20110920
            fTemperFrom->SetShowYield(fTemperFrom->esytYieldMonitor,    TestIF_File.bFailAlarmLowYield_RT || TestIF_File.bFailAlarmSiteYieldCmp_RT || TestIF_File.bFailAlarmSiteYieldDifferent_RT);
            fTemperFrom->SetShowYield(fTemperFrom->esytConsAlarm,       TestIF_File.bContsFailBySocket_RT==true || TestIF_File.bContsFailByHead_RT==true);
        }
        fTemperFrom->SetShowYield(fTemperFrom->esytTest2, IniConfig.bFTContinueON);
        fTemperFrom->strShowYield[fTemperFrom->esytTest2].bShow=IniConfig.bFTContinueON;                                //kevin 20121008 RT
        fTemperFrom->ShowYieldFuntion();
    }
#endif // GATE (S12-C save)

    //wei 20141201 Low Yield Auto Clean(%) start
    TestIF_File.bFailAlarmLowYield_AutoClean        =ReadIniData(szDir, "Low Yield Auto Clean", "Enable",   0);
    TestIF_File.iLowYieldLimit_AutoClean            =ReadIniData(szDir, "Low Yield Auto Clean", "Limit",   10);
    TestIF_File.iLowYieldCount_AutoClean            =ReadIniData(szDir, "Low Yield Auto Clean", "Count",  100);
    //wei 20141201 Low Yield Auto Clean(%) end

    if(CosFunction.bAutoCloseSiteWhenRT)                                        //Steven 20200225 : 切到RT的時候,要關閉Socket
    {
        TestIF_File.iAutoCloseSiteWhenRT            =CheckRange(ReadIniData(szDir, "Auto Close Site When RT", "iAutoCloseSiteYieldWhenRT",   0), 0, 1);
        TestIF_File.dAutoCloseSiteYieldWhenRT       =CheckRange(ReadIniData(szDir, "Auto Close Site When RT", "dAutoCloseSiteYieldWhenRT",   0.0), 50.00, 0.01);
        TestIF_File.iAllSiteOnAtInitialStart        =CheckRange(ReadIniData(szDir, "Site On at Initial Start", "iAllSiteOnAtInitialStart",   1), 0, 1);         //Steven 20230814 : Initial Start的時候要全開Site
    }
    TestIF_File.iAutoSiteOffByGPIB                  =CheckRange(ReadIniData(szDir, "AutoSiteOnoff", "iAutoSiteOffByGPIB",   0), 0, 1);
    if(IniConfig.bE53LowYieldAutoClean==false)                                  //kevin 20160802
        TestIF_File.bFailAlarmLowYield_AutoClean=false;

 //---AutoRetest setup  kevin 20150704 start
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        TestIF_File.bEnablePassYieldART             =ReadIniData(szDir, "AutoRetest", "bPassART", false);               //使用 PASS 比較
        MyYieldPanel[0]->bEnablePassYieldART        =bFirstYieldCmp_ART             =TestIF_File.bEnablePassYieldART;
        TestIF_File.fPassYieldART                   =ReadIniData(szDir, "AutoRetest", "fPassyieldART", 0.00);
        MyYieldPanel[0]->fPassYieldART              =fFirstYieldSet_ART             =TestIF_File.fPassYieldART;

        TestIF_File.bEnableOpenShortART             =ReadIniData(szDir, "AutoRetest", "bOpenshortART", false);
        MyYieldPanel[0]->bEnableOpenShortART        =bOpenShortYieldCmp_ART         =TestIF_File.bEnableOpenShortART;
        TestIF_File.fOpenShortYieldART              =ReadIniData(szDir, "AutoRetest", "fOpenShortYieldART", 0.0);
        MyYieldPanel[0]->fOpenShortYieldART         =fOpenShortYieldSet_ART         =TestIF_File.fOpenShortYieldART;

        TestIF_File.bEnableRecoverART               =ReadIniData(szDir, "AutoRetest", "bRecoverART", false);
        MyYieldPanel[0]->bEnableRecoverART          =bRecoverRateYieldCmp_ART       =TestIF_File.bEnableRecoverART;
        TestIF_File.fRecoverYieldART                =ReadIniData(szDir, "AutoRetest", "fRecoverYieldART", 0.0);
        MyYieldPanel[0]->fRecoverYieldART           =fRecoverRateYieldRT1Set_ART    =TestIF_File.fRecoverYieldART;

        iAutoRetestLimit                            =ReadIniData(szDir, "AutoRetest", "ArtAutoRetestLimit", 0);         //RT 次數
        if(iAutoRetestLimit<=0)
            iAutoRetestLimit=1;
        MyYieldPanel[0]->ReTestLimit->ItemIndex     =iAutoRetestLimit-1;

        AnsiString bBin="", bBin1="";
        AnsiString abuffer="", abuffer1="";                                     //kevin 20170825 (wei) add

        abuffer= CheckAndReadIniData(szDir, "AutoRetest", "bPassBin", AnsiString(""));
        abuffer1=CheckAndReadIniData(szDir, "AutoRetest", "bOpenShortBin", AnsiString(""));                             //kevin 20170825 add
        if(abuffer=="" || abuffer1=="")
        {
            for(int i=0; i<iTestBinCount; i++)                                  //Steven 20251104 : iBinCount --> iTestBinCount
            {
                abuffer.sprintf("%s,","0");
                bBin+= abuffer;
                abuffer1.sprintf("%s,","0");
                bBin1+= abuffer1;
            }
            WriteIniData(szDir, "AutoRetest", "bPassBin", bBin);
            WriteIniData(szDir, "AutoRetest", "bOpenShortBin", bBin1);

            abuffer= CheckAndReadIniData(szDir, "AutoRetest", "bPassBin", AnsiString(""));
            abuffer1=CheckAndReadIniData(szDir, "AutoRetest", "bOpenShortBin", AnsiString(""));                         //kevin 20170825 add
        }

        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            TestIF_File.bPass[i]                    =(abuffer.SubString(i*2+1,1).Trim()==1);
            MyYieldPanel[0]->bPass[i]               =TestIF_File.bPass[i];

            TestIF_File.bOpenShort[i]               =(abuffer1.SubString(i*2+1,1).Trim()==1);
            MyYieldPanel[0]->bOpenShort[i]          =TestIF_File.bOpenShort[i];
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150825 KYEC 修改成By File
    {
        iAutoRetestLimitFile                        =ReadIniData(szDir, "AutoRetest", "ArtAutoRetestLimitFile", 1);     //RT 次數;       //wei  20150825 做AUTO RETEST最大次數
        iFailYieldRate_ARTFile                      =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFile", 0.0);                                         //RT 次數;       //wei  20150825

        iAutoLeastRetestLimitFile                   =ReadIniData(szDir, "AutoRetest", "AutoLeastRetestLimitFile", 1);   //wei 20160203
        dFailYieldRate_ARTFTFile[0]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTFTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTFTFile[1]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFTFile", 0.0);                                       //wei 20160203
        dFailYieldRate_ARTFTFile[2]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTFTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTRTFile[0]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTRTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTRTFile[1]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTRTFile", 0.0);                                       //wei 20160203
        dFailYieldRate_ARTRTFile[2]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTRTFile", 0.0);                                    //wei 20160203
        bAutoLeastRetestFile                        =ReadIniData(szDir, "AutoRetest", "bAutoLeastRetestFile", false);   //wei 20160203
        bUseFailNoDistinction                       =ReadIniData(szDir, "AutoRetest", "bUseFailNoDistinction", false);  //wei 20160203

        iUseFTFailYield                             =ReadIniData(szDir, "AutoRetest", "ArtUseFTFailYield", 0);          //wei 20160203
        iUseRTFailYield                             =ReadIniData(szDir, "AutoRetest", "ArtUseRTFailYield", 0);          //wei 20160203
        iUseFTFailYieldModel                        =ReadIniData(szDir, "AutoRetest", "ArtUseFTFailYieldModel", 0);     //wei 20160203
        iUseRTFailYieldModel                        =ReadIniData(szDir, "AutoRetest", "ArtUseRTFailYieldModel", 0);     //wei 20160203

        EL<TEdit>("TfYieldMonitoring", "edAutoRetestLimitFile")->Text                 =iAutoRetestLimitFile;
        EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFile")->Text               =iFailYieldRate_ARTFile;

        EL<TEdit>("TfYieldMonitoring", "edAutoLeastRetestLimitFile")->Text            =iAutoLeastRetestLimitFile;                                         //wei 20160203
        EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTFTFile")->Text          =dFailYieldRate_ARTFTFile[0];                                       //wei 20160203
        EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFTFile")->Text             =dFailYieldRate_ARTFTFile[1];
        EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTFTFile")->Text          =dFailYieldRate_ARTFTFile[2];
        EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTRTFile")->Text          =dFailYieldRate_ARTRTFile[0];
        EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTRTFile")->Text             =dFailYieldRate_ARTRTFile[1];
        EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTRTFile")->Text          =dFailYieldRate_ARTRTFile[2];
        EL<TRadioGroup>("TfYieldMonitoring", "rgFT_ART")->ItemIndex                         =iUseFTFailYield;
        EL<TRadioGroup>("TfYieldMonitoring", "rgRT_ART")->ItemIndex                         =iUseRTFailYield;

        EL<TComboBox>("TfYieldMonitoring", "cbUseFTFailYieldModel")->ItemIndex            =iUseFTFailYieldModel;
        EL<TComboBox>("TfYieldMonitoring", "cbUseRTFailYieldModel")->ItemIndex            =iUseRTFailYieldModel;

        EL<TCheckBox>("TfYieldMonitoring", "ckUseLeastRetestTimes")->Checked              =bAutoLeastRetestFile;
        EL<TCheckBox>("TfYieldMonitoring", "ckUseFailNoDistinction")->Checked             =bUseFailNoDistinction;
        IniConfig.iAutoRetestLimit                  =iAutoRetestLimitFile;
        IniConfig.iFailYieldRate_ART                =iFailYieldRate_ARTFile;
        EL<TPanel>("TfYieldMonitoring", "pnlARTFailYiel")->Visible=false;
    }

    if(CosFunction.bYieldAlarm4)                                                //wei 20160314
    {
        TestIF_File.bAlarm4ContinueType_Enable          =ReadIniData(szDir, "Alarm4", "bAlarm4ContinueType_Enable"                  ,        false);
        TestIF_File.iAlarm4ContinueType_IntervalCount   =ReadIniData(szDir, "Alarm4", "iAlarm4ContinueType_IntervalCount"           ,        1);
        TestIF_File.iAlarm4ContinueType_ContinueCount   =ReadIniData(szDir, "Alarm4", "iAlarm4ContinueType_ContinueCount"           ,        1);

        TestIF_File.bAlarm4EnableIntervalYield          =ReadIniData(szDir, "Alarm4", "bAlarm4EnableIntervalYield"                  ,        false);
        TestIF_File.iAlarm4IntervalYieldIntervalCount   =ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldIntervalCount"           ,        1);
        TestIF_File.iAlarm4IntervalYieldContinueCount   =CheckRange(ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldContinueCount"           ,        1), iMinYield, iMaxYield);
        TestIF_File.iAlarm4IntervalYieldYield           =ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldYield"                   ,        1);

        TestIF_File.bSiteToSiteYieldCmp                 =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Enable",              false);
        TestIF_File.iSiteToSiteYieldCmp                 =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Yield",                   0);
        TestIF_File.iSiteToSiteYieldCmpCount            =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_IntervalCount",        1000);

        TestIF_File.bHeadToHeadYieldCmp                 =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Enable",              false);
        TestIF_File.iHeadToHeadYieldCmp                 =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Yield",                   0);
        TestIF_File.iHeadToHeadYieldCmpCount            =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_IntervalCount",        1000);

        TestIF_File.bSiteYieldOverAlert                 =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_Enable",              false);
        TestIF_File.iSiteYieldOverAlert                 =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_Yield",                   0);
        TestIF_File.iSiteYieldOverAlertCount            =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_IntervalCount",        1000);
    }
    else
    {
        TestIF_File.bAlarm4ContinueType_Enable          =false;
        TestIF_File.bAlarm4EnableIntervalYield          =false;
        TestIF_File.bSiteToSiteYieldCmp                 =false;
        TestIF_File.bHeadToHeadYieldCmp                 =false;
        TestIF_File.bSiteYieldOverAlert                 =false;
//        TestIF_File.bAllSiteFail                        =false;
//        TestIF_File.bAllSiteFail_RT                     =false;
    }

    //Steven 20231017 : change position
    TestIF_File.bAllSiteFail                            =ReadIniData(szDir, "Alarm4", "AllSiteFail",                            false);                         //kevin 20170825 (wei) 整支ARM Fail bin
    TestIF_File.iAllSiteFailCount                       =ReadIniData(szDir, "Alarm4", "AllSiteFailCount",                        0);                            //kevin 20180720 (wei) all site fail count
    TestIF_File.iAllSiteFailCountRT                     =ReadIniData(szDir, "Alarm4", "AllSiteFailCount_RT",                     0);                            //Steven 20230118 : All site fail RT
    TestIF_File.bAllSiteFail_RT                         =ReadIniData(szDir, "Alarm4", "AllSiteFail_RT",                         false);                         //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開

    if(CosFunction.bYieldAlarm5)                                                //Sam 20171213 (Steven) : 超豐良率監控 //Sam 20180423 (wei) : MOFile of Yeild Download
    {
        TestIF_File.iAlarm5_BySiteIntervalContactCnt    =ReadIniData(szDir, "Alarm5", "iAlarm5_BySiteIntervalContactCnt",         100);
        TestIF_File.bAlarm5_BySiteLowYieldEnable        =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteLowYieldEnable",           false);
        TestIF_File.dAlarm5_BySiteLowYield              =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYield",                  95.0);
        TestIF_File.dAlarm5_BySiteLowYieldRej           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYieldRej",               95.0);
        TestIF_File.bAlarm5_BySiteCmpYieldEnable        =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteCmpYieldEnable",           false);
        TestIF_File.dAlarm5_BySiteCmpYield              =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYield",                     5);
        TestIF_File.dAlarm5_BySiteCmpYieldRej           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYieldRej",                 10);
        TestIF_File.bAlarm5_BySiteAlarmYieldEnable      =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteAlarmYieldEnable",         false);
        TestIF_File.dAlarm5_BySiteAlarmYield            =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYield",                 0.5);
        TestIF_File.dAlarm5_BySiteAlarmYieldRej         =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYieldRej",                1);
        TestIF_File.iAlarm5_OSBin                       =ReadIniData(szDir, "Alarm5", "iAlarm5_OSBin",                              5);
        TestIF_File.bAlarm5_BySitePreCmpYieldEnable     =ReadIniData(szDir, "Alarm5", "bAlarm5_BySitePreCmpYieldEnable",        false);
        TestIF_File.dAlarm5_BySitePreCmpYield           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYield",                  5);
        TestIF_File.dAlarm5_BySitePreCmpYieldRej        =ReadIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYieldRej",              10);
    }
    else
    {
        TestIF_File.bAlarm5_BySiteLowYieldEnable    =false;
        TestIF_File.bAlarm5_BySiteCmpYieldEnable    =false;
        TestIF_File.bAlarm5_BySiteAlarmYieldEnable  =false;
        TestIF_File.bAlarm5_BySitePreCmpYieldEnable =false;
    }

    //---AutoRetest setup  kevin 20150704 end
    if(USE_AUTO_RETEST==eartUninstall)
    {
        bFirstYieldCmp_ART=false;
        bOpenShortYieldCmp_ART=false;
        bRecoverRateYieldCmp_ART=false;
    }

    if(bFirstYieldCmp_ART || bOpenShortYieldCmp_ART || bRecoverRateYieldCmp_ART)                                        //kevin 20150704
    {
        bAutoReTest_ART=true;                                                   // 起動AUTORETEST
    }
    else
    {
        bAutoReTest_ART=false;
    }
    //kevin 20150601 end
    if(CosFunction.bPiggybackFunctionByHandler==true)                           //Isaac 20170712 :Piggyback function By Handlder(save file to config.ini)
    {
        AnsiString S="";
        S="config.ini";
        AnsiString szDir="";
        szDir=AuthPath+S;

        //Alarm Action
        TestIF_File.iCountAlarmAction          =ReadIniData(szDir, "Alarm", "Count Action",                 0);
        TestIF_File.iCountAlarmAction_RT       =ReadIniData(szDir, "Alarm", "Count Action RT",              0);

        //Continuous Pass
        TestIF_File.bContinuousPass            =ReadIniData(szDir, "Alarm", "Continuous Pass",              false);
        TestIF_File.iContinuousPassBin         =ReadIniData(szDir, "Alarm", "Continuous Pass Bin",          1);
        TestIF_File.iContinuousPassBinCount    =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count",    1000);
        TestIF_File.bContinuousPass_RT         =ReadIniData(szDir, "Alarm", "Continuous Pass RT",           false);
        TestIF_File.iContinuousPassBin_RT      =ReadIniData(szDir, "Alarm", "Continuous Pass RT Bin",       1);
        TestIF_File.iContinuousPassBinCount_RT =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT", 1000);

        //Continuous Pass by Socket - Steven 20110915
        TestIF_File.bContinuousPassBySocket                 =ReadIniData(szDir, "Alarm", "Continuous Pass By Socket",               false);
        TestIF_File.iContinuousPassBinCountBySocket         =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count By Socket",     1000);
        TestIF_File.bContinuousPassBySocket_RT              =ReadIniData(szDir, "Alarm", "Continuous Pass RT By Socket",            false);
        TestIF_File.iContinuousPassBinCountBySocket_RT      =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT By Socket",  1000);

        //Continuous Loader
        TestIF_File.bContinuousLoader                       =ReadIniData(szDir, "Alarm", "Continuous Loader",           false);
        TestIF_File.iContinuousLoaderCount                  =ReadIniData(szDir, "Alarm", "Continuous Loader Count",     1000);
        TestIF_File.bContinuousLoader_RT                    =ReadIniData(szDir, "Alarm", "Continuous Loader RT",        false);
        TestIF_File.iContinuousLoaderCount_RT               =ReadIniData(szDir, "Alarm", "Continuous Loader Count RT",  100);

        //Contunous Contact Count
        TestIF_File.bContinuousContact                      =ReadIniData(szDir, "Alarm", "Continuous Contact",          false);
        TestIF_File.iContinuousContactCount                 =ReadIniData(szDir, "Alarm", "Continuous Contact Count",    1000);
        TestIF_File.bContinuousContact_RT                   =ReadIniData(szDir, "Alarm", "Continuous Contact RT",       false);
        TestIF_File.iContinuousContactCount_RT              =ReadIniData(szDir, "Alarm", "Continuous Contact Count RT", 1000);

        if(IniConfig.bD56YieldPiggyBackEnable)                                  //強制Enable Yield 裡面piggyback 功能
        {
            TestIF_File.bContinuousContact           =true;
            TestIF_File.bContinuousContact_RT        =true;
            TestIF_File.bContinuousPassBySocket      =true;
            TestIF_File.bContinuousPassBySocket_RT   =true;
            TestIF_File.bContinuousLoader            =true;
            TestIF_File.bContinuousLoader_RT         =true;
            TestIF_File.bContinuousPass              =true;
            TestIF_File.bContinuousPass_RT           =true;
        }

        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20250120 : ASECL SONG要求強制開啟
        {
            TestIF_File.iContinuousContactCount=400;
            TestIF_File.iContinuousContactCount_RT=400;
            TestIF_File.bContinuousContact=true;
            TestIF_File.bContinuousContact_RT=true;

//            TestIF_File.bFailAlarmLowYield_RT=false;
//            TestIF_File.bFailAlarmSiteYieldDifferent_RT=false;
//            TestIF_File.bFailAlarmSiteYieldCmp_RT=false;
//            TestIF_File.bContsFailBySocket_RT=false;
//            TestIF_File.bContsFailByHead_RT=false;
        }
    }

    if(CosFunction.bAdaptiveYield)                                              //Sam 20230914 : 自適應性良率監控
    {
        TestIF_File.bAdaptiveLowYield               =ReadIniData(szDir, "AdaptiveYield", "Low Yield Alarm",        false);
        TestIF_File.iAdaptiveContsLowerAlarmNor     =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm",     3);                                     //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin     =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm Min", 3);
        TestIF_File.iAdaptiveYieldMax               =ReadIniData(szDir, "AdaptiveYield", "Yield Max",                   3);
        TestIF_File.iAdaptiveYieldMin               =ReadIniData(szDir, "AdaptiveYield", "Yield Min",                   1);

        TestIF_File.bAdaptiveLowYield_RT            =ReadIniData(szDir, "AdaptiveYield", "Low Yield Alarm RT",        false);
        TestIF_File.iAdaptiveContsLowerAlarmNor_RT  =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm RT",  3);                                     //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin_RT  =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm Min RT",  3);
        TestIF_File.iAdaptiveYieldMax_RT            =ReadIniData(szDir, "AdaptiveYield", "Yield Max RT",                3);
        TestIF_File.iAdaptiveYieldMin_RT            =ReadIniData(szDir, "AdaptiveYield", "Yield Min RT",                1);
    }

    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
        AnsiString asSpecBinBySiteCompare;
        for(int i=0; i<iTestBinCount; i++)
        {
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_FT",i);
            TestIF_File.bSpecBinBySiteCompareEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_RT",i);
            TestIF_File.bSpecBinBySiteCompareEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_FT",i);
            TestIF_File.dSpecBinBySiteComparePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_RT",i);
            TestIF_File.dSpecBinBySiteComparePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_FT",i);
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_RT",i);
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_FT",i);
            TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_RT",i);
            TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_FT",i);
            TestIF_File.bByBinFailureEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_RT",i);
            TestIF_File.bByBinFailureEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_FT",i);
            TestIF_File.dByBinFailurePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_RT",i);
            TestIF_File.dByBinFailurePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_FT",i);         //JerryYang 20231206
            TestIF_File.bFailCountEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_RT",i);
            TestIF_File.bFailCountEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_FT",i);
            TestIF_File.iFailCountLimit[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0);
            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_RT",i);
            TestIF_File.iFailCountLimit[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0);
        }
        TestIF_File.iSpecBinBySiteCompareIgnore[FT]=ReadIniData(szDir, "Alarm", "iSpecBinBySiteCompareIgnore_FT",          200);
        TestIF_File.iSpecBinBySiteCompareIgnore[RT]=ReadIniData(szDir, "Alarm", "iSpecBinBySiteCompareIgnore_RT",          200);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[FT]=ReadIniData(szDir, "Alarm", "iSpecBinByArmPerSiteCompareIgnore_FT",200);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[RT]=ReadIniData(szDir, "Alarm", "iSpecBinByArmPerSiteCompareIgnore_RT",200);
        TestIF_File.iByBinFailureIgnore[FT]=ReadIniData(szDir, "Alarm", "iByBinFailureIgnore_FT",200);
        TestIF_File.iByBinFailureIgnore[RT]=ReadIniData(szDir, "Alarm", "iByBinFailureIgnore_RT",200);
        TestIF_File.iFailCountIgnore[FT]=ReadIniData(szDir, "Alarm", "iFailCountIgnore_FT",200);                        //JerryYang 20231206
        TestIF_File.iFailCountIgnore[RT]=ReadIniData(szDir, "Alarm", "iFailCountIgnore_RT",200);
    }

    if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;
    {
        TestIF_File.bCreateManualEOCAP=ReadIniData(szDir, "Alarm", "bCreateManualEOCAP",          false);
    }

    YM_DoIniDataToForm();
    ;   // golden uYieldMonitoring.cpp:1704-1705 取代：fSortCT->pnlYield／pnlYieldART：移植樹 forms/fSortCT.h 沒有這兩個面板（同 GATE G-YM-Tail (b)(c)），純畫面
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:1704-1705 -- fSortCT->pnlYield／pnlYieldART：移植樹 forms/fSortCT.h 沒有這兩個面板（同 GATE G-YM-Tail (b)(c)），純畫面
    fSortCT->pnlYield->Visible=TestIF_File.bLowYieldAlarmByBin;                 //Steven 20141125
    fSortCT->pnlYieldART->Visible=TestIF_File.bLowYieldAlarmByBin;              //kevin 20150615
#endif // GATE (S12-C save)

}

// golden uYieldMonitoring.cpp:531  TfYieldMonitoring::DoIniDataToForm()
static void YM_DoIniDataToForm()
{

//Auto Site Off Function
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOff")->Checked                 =TestIF_File.bLowYieldAutoSiteOff;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByContiFail")->Checked      =TestIF_File.bLowYieldAutoSiteOffByContiFail;                               //Steven 20200420 : Continue fail, auto site off
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmBySite")->Checked      =TestIF_File.bLowYieldAutoSiteOffByArmSite;                                 //Steven 20230223 : by arm by site, auto site off
    EL<TCheckBox>("TfYieldMonitoring", "chkchkAutoSiteOffByPicker")->Checked      =TestIF_File.bLowYieldAutoSiteOffByPicker;                                  //Steven 20230223 : 根據Index吸嘴比較良率
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmContiFail")->Checked   =TestIF_File.bLowYieldAutoSiteOffArmContiFail;                              //Steven 20220818 : By Arm Continue fail, auto site off
    EL<TEdit>("TfYieldMonitoring", "edtSiteYieldCmp1SiteCnt_FT")->Text        =TestIF_File.iAlarmWhenSiteOnCountLess;
    EL<TRadioGroup>("TfYieldMonitoring", "rgCloseSiteOnHPAction")->ItemIndex        =TestIF_File.iCloseSiteOnHPDontTest;                                        //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->ItemIndex             =TestIF_File.iCloseSiteBin;         //Steven 20240409 : 關site的位置有IC不測試送指定 bin
    EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffAlarm")->Checked            =TestIF_File.bLowYieldAutoSiteOffAlarm;                                     //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffAlarm")->Text               =TestIF_File.iLowYieldAutoSiteOffAlarm;                                     //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site

//Low Yield Alarm(%) [Socket Yield compare with Total Yield]
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_FT")->Checked          =TestIF_File.bFailAlarmLowYield;
    EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")->Text             =TestIF_File.dLowYieldLimit;                //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT")->Text           =TestIF_File.iLowYieldCount;
    EL<TCheckBox>("TfYieldMonitoring", "cbYieldAlarmByBin")->Checked      =TestIF_File.bLowYieldAlarmByBin;           //Steven 20140828 : By Bin Yield Monitor
    EL<TCheckBox>("TfYieldMonitoring", "cbSlidingWindowYield")->Checked   =TestIF_File.bSlidingWindowYield;           //Steven 20260331 : Sliding Window Yield
    EL<TLabeledEdit>("TfYieldMonitoring", "edSlidingWindowSize")->Text       =TestIF_File.iSlidingWindowSize;            //Steven 20260331

    EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_RT")->Checked          =TestIF_File.bFailAlarmLowYield_RT;
    EL<TEdit>("TfYieldMonitoring", "edLowYield_RT")->Text             =TestIF_File.dLowYieldLimit_RT;             //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_RT")->Text           =TestIF_File.iLowYieldCount_RT;

//Site Yield Alarm(%)   [同一個Socket, 不同Arm比較]
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_FT")->Checked=TestIF_File.bFailAlarmSiteYieldDifferent;
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT")->Text   =TestIF_File.dFailAlarmSiteYield;           //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_FT")->Text =TestIF_File.iFailAlarmSiteYieldDifferentCount;

    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_RT")->Checked=TestIF_File.bFailAlarmSiteYieldDifferent_RT;
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT")->Text   =TestIF_File.dFailAlarmSiteYield_RT;        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_RT")->Text =TestIF_File.iFailAlarmSiteYieldDifferentCount_RT;

//Site Yield Alarm(%)   [Socket Yield互比]
    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_FT")->Checked      =TestIF_File.bFailAlarmSiteYieldCmp;
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_FT")->Text         =TestIF_File.dFailAlarmSiteYieldCmp;        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_FT")->Text       =TestIF_File.iFailAlarmSiteYieldCmpCount;

    EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_RT")->Checked      =TestIF_File.bFailAlarmSiteYieldCmp_RT;
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_RT")->Text         =TestIF_File.dFailAlarmSiteYieldCmp_RT;     //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_RT")->Text       =TestIF_File.iFailAlarmSiteYieldCmpCount_RT;
    ;   // golden uYieldMonitoring.cpp:574-574 取代：iYieldAlarmCheckIntervalByCount 移植樹沒有（ReadFile :1033 已記 ELTodo）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:574-574 -- iYieldAlarmCheckIntervalByCount 移植樹沒有（ReadFile :1033 已記 ELTodo）
    EL<TEdit>("TfYieldMonitoring", "edtYieldAlarmCheckIntervalByCount")->Text =TestIF_File.iYieldAlarmCheckIntervalByCount;   //AI(rf360-yield-count) 20260814 (RogerYang)
#endif // GATE (S12-C save)

//Low Yield Alarm(By Total)(%)  [總良率]                                        //wei 20151116 Low Yield By Total
    EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_FT")->Checked   =TestIF_File.bFailAlarmLowYieldByTotal;
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_FT")->Text      =TestIF_File.dLowYieldLimitByTotal;         //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_FT")->Text    =TestIF_File.iLowYieldCountByTotal;

    EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_RT")->Checked   =TestIF_File.bFailAlarmLowYieldByTotal_RT;
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_RT")->Text      =TestIF_File.dLowYieldLimitByTotal_RT;      //JerryYang 20160530 LowYieldLimit要能設定到小數點
    EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_RT")->Text    =TestIF_File.iLowYieldCountByTotal_RT;

//By Picker Compare Yield   [每個吸嘴的良率互比]                                //Steven 20230223 : 根據Index吸嘴比較良率
    EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_FT")->Checked =TestIF_File.bLowYieldByPicker;
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_FT")->Text    =TestIF_File.dLowYieldByPicker;
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_FT")->Text  =TestIF_File.iLowYieldCountByPicker;

    EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_RT")->Checked =TestIF_File.bLowYieldByPicker_RT;
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_RT")->Text    =TestIF_File.dLowYieldByPicker_RT;
    EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_RT")->Text  =TestIF_File.iLowYieldCountByPicker_RT;

//Alarm Action
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")->ItemIndex       =TestIF_File.iCountAlarmAction;
    EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")->ItemIndex       =TestIF_File.iCountAlarmAction_RT;

//Continuous Pass
    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_FT")->Checked    =TestIF_File.bContinuousPass;
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT")->ItemIndex =TestIF_File.iContinuousPassBin-1;
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_FT")->Text       =TestIF_File.iContinuousPassBinCount;

    EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_RT")->Checked    =TestIF_File.bContinuousPass_RT;
    EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT")->ItemIndex =TestIF_File.iContinuousPassBin_RT-1;
    EL<TEdit>("TfYieldMonitoring", "edContinuousPass_RT")->Text       =TestIF_File.iContinuousPassBinCount_RT;

//Consecutive Failure Ignore
    EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_FT")->Checked   =TestIF_File.bContsFailIgnore;              //wei 20160115 銅鑼前幾顆不計算ContsFail
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_FT")->Text      =TestIF_File.iContsFailIgnore;              //wei 20160115 銅鑼前幾顆不計算ContsFail

    EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_RT")->Checked   =TestIF_File.bContsFailIgnore_RT;           //wei 20160115 銅鑼前幾顆不計算ContsFail
    EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_RT")->Text      =TestIF_File.iContsFailIgnore_RT;           //wei 20160115 銅鑼前幾顆不計算ContsFail

    if(IniConfig.bD56YieldPiggyBackEnable)                                      //kevin 20131009 強致 Enable Yield 裡面piggyback 功能
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Enabled=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Enabled   =false;
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Enabled=false;
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Enabled   =false;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Checked=true;
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")->Text   =TestIF_File.iContinuousPassBinCountBySocket;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Checked=true;
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")->Text   =TestIF_File.iContinuousPassBinCountBySocket_RT;

        //Continuous Loader
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_FT")->Checked=false;
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT")->Text   =TestIF_File.iContinuousLoaderCount;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_RT")->Checked=false;
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT")->Text   =TestIF_File.iContinuousLoaderCount_RT;

        //Continuous Contact Count
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Checked   =true;
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")->Text      =TestIF_File.iContinuousContactCount;

        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Checked   =true;
        EL<TEdit>("TfYieldMonitoring", "edContactCountRT")->Text      =TestIF_File.iContinuousContactCount_RT;
    }
    else
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Enabled=true;
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Enabled   =true;
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Enabled=true;
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Enabled   =true;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Checked=TestIF_File.bContinuousPassBySocket;
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")->Text   =TestIF_File.iContinuousPassBinCountBySocket;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Checked=TestIF_File.bContinuousPassBySocket_RT;
        EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")->Text   =TestIF_File.iContinuousPassBinCountBySocket_RT;

        //Continuous Loader
        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_FT")->Checked=TestIF_File.bContinuousLoader;
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT")->Text   =TestIF_File.iContinuousLoaderCount;

        EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_RT")->Checked=TestIF_File.bContinuousLoader_RT;
        EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT")->Text   =TestIF_File.iContinuousLoaderCount_RT;

        //Continuous Contact Count
        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Checked   =TestIF_File.bContinuousContact;
        EL<TEdit>("TfYieldMonitoring", "edContactCountFT")->Text      =TestIF_File.iContinuousContactCount;

        EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Checked   =TestIF_File.bContinuousContact_RT;
        EL<TEdit>("TfYieldMonitoring", "edContactCountRT")->Text      =TestIF_File.iContinuousContactCount_RT;
    }

    if(CUSTOMER_CODE==CC_PTI)
    {
        fLotInfo->RefreshOtherTool();
    }

//Consecutive Failure Alarm ( Socket )
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn") ->Enabled=true;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Enabled=true;
    if(IniConfig.bFTContinueON)                                                 //kevin 20121008 FT MODE 不能關連續FAIL
    {
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn") ->Checked=true;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Checked=false;
        if(TestIF_File.iContsFailSocketAlarmCT>(unsigned int)IniConfig.iFTMAXValue)
           TestIF_File.iContsFailSocketAlarmCT=(unsigned int)IniConfig.iFTMAXValue;

       EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn") ->Enabled=false;
       EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Enabled=false;
    }
    else
    {
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn") ->Checked=TestIF_File.bContsFailBySocket;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOff")->Checked=!TestIF_File.bContsFailBySocket;
    }
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT")->Text=TestIF_File.iContsFailSocketAlarmCT;
    EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT")->Text=TestIF_File.iContsFailSocketAlarmCT_RT;
    EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_FT")->Checked=TestIF_File.bCountSpcBinContinuously_FT;                                         //Steven 20230529 : Spc Bin Couont改成連續錯誤
    EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_RT")->Checked=TestIF_File.bCountSpcBinContinuously_RT;                                         //Steven 20230529 : Spc Bin Couont改成連續錯誤

    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOn") ->Checked=TestIF_File.bContsFailBySocket_RT;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOff")->Checked=!TestIF_File.bContsFailBySocket_RT;

//Consecutive Failure Alarm ( Head)
    //Consecutive Failure Alarm ( Socket )
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn") ->Enabled=true;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Enabled=true;
    if(IniConfig.bFTContinueON)                                                 //kevin 20121008 FT MODE 不能關連續FAIL
    {
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn") ->Checked=true;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Checked=false;
        if(TestIF_File.iContsFailHeadAlarmCT>(unsigned int)IniConfig.iFTMAXValue)
            TestIF_File.iContsFailHeadAlarmCT=(unsigned int)IniConfig.iFTMAXValue;

        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn") ->Enabled=false;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Enabled=false;
    }
    else
    {
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn") ->Checked=TestIF_File.bContsFailByHead;
        EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOff")->Checked=!TestIF_File.bContsFailByHead;
    }
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOn") ->Checked    =TestIF_File.bContsFailByHead_RT;
    EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOff")->Checked    =!TestIF_File.bContsFailByHead_RT;
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT")->Text     =TestIF_File.iContsFailHeadAlarmCT_RT;
    EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")->Text     =TestIF_File.iContsFailHeadAlarmCT;

    //wei 20141201 Low Yield Auto Clean(%) start
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoCleanYieldOnOff")->ItemIndex    =TestIF_File.bFailAlarmLowYield_AutoClean;
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldLimit")->Text      =TestIF_File.iLowYieldLimit_AutoClean;
    EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldCount")->Text      =TestIF_File.iLowYieldCount_AutoClean;
    EL<TEdit>("TfYieldMonitoring", "eCloseSite")->Text                    =IntToStr(TestIF_File.iLowYieldLimit_AutoClean*2);                              //kevin 20150306
    //wei 20141201 Low Yield Auto Clean(%) end

    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOffWhenRT")->ItemIndex      =TestIF_File.iAutoCloseSiteWhenRT;      //Steven 20200225 : 切到RT的時候,要關閉Socket
    EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffWhenRT")->Text          =TestIF_File.dAutoCloseSiteYieldWhenRT;                                         //Steven 20200225 : 切到RT的時候,要關閉Socket
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn")->ItemIndex             =TestIF_File.iAllSiteOnAtInitialStart;  //Steven 20230814 : Initial Start的時候要全開Site
    EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOnByGPIB")->ItemIndex       =TestIF_File.iAutoSiteOffByGPIB;        //JimmyChiu 20250715 : Auto site on/off by GPIB
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150529 AutoRetest
    {
        MyYieldPanel[0]->ReTestLimit->ItemIndex=iAutoRetestLimit-1;             //kevin 20150704
    ;   // golden uYieldMonitoring.cpp:738-739 取代：InitDataToEdit(0)／InitmtBinSelectData()：ASE ART 格子（TTMyTray256）上色與編號，純畫面
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:738-739 -- InitDataToEdit(0)／InitmtBinSelectData()：ASE ART 格子（TTMyTray256）上色與編號，純畫面
        InitDataToEdit(0);
        MyYieldPanel[0]->InitmtBinSelectData();
#endif // GATE (S12-C save)
    }

    //wei 20160314
    EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType")->Checked               =TestIF_File.bAlarm4ContinueType_Enable;
    EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount")->Text                 =TestIF_File.iAlarm4ContinueType_IntervalCount;
    EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount")->Text                 =TestIF_File.iAlarm4ContinueType_ContinueCount;

    EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield")->Checked       =TestIF_File.bAlarm4EnableIntervalYield;
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount")->Text   =TestIF_File.iAlarm4IntervalYieldIntervalCount;
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount")->Text   =TestIF_File.iAlarm4IntervalYieldContinueCount;
    EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")->Text           =TestIF_File.iAlarm4IntervalYieldYield;

    EL<TCheckBox>("TfYieldMonitoring", "cb_SiteToSiteYieldEnable")->Checked           =TestIF_File.bSiteToSiteYieldCmp;
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")->Text                    =TestIF_File.iSiteToSiteYieldCmp;
    EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount")->Text               =TestIF_File.iSiteToSiteYieldCmpCount;

    EL<TCheckBox>("TfYieldMonitoring", "cb_HeadToHeadYieldEnable")->Checked           =TestIF_File.bHeadToHeadYieldCmp;
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")->Text                    =TestIF_File.iHeadToHeadYieldCmp;
    EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount")->Text               =TestIF_File.iHeadToHeadYieldCmpCount;

    EL<TCheckBox>("TfYieldMonitoring", "cb_SiteYieldOverAlert")->Checked              =TestIF_File.bSiteYieldOverAlert;
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert")->Text                 =TestIF_File.iSiteYieldOverAlert;
    EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount")->Text            =TestIF_File.iSiteYieldOverAlertCount;
    EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail")->Checked                      =TestIF_File.bAllSiteFail;      //kevin 20170825 (wei) 整支ARM Fail bin
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount")->Text                    =TestIF_File.iAllSiteFailCount;                                         //kevin 20180720 (wei) add all site fail count
    EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT")->Text                  =TestIF_File.iAllSiteFailCountRT;                                       //Steven 20230118 : All site fail RT
    EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT")->Checked                   =TestIF_File.bAllSiteFail_RT;   //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20200603 ：矽格-湖口，Yield 頁面 AllSiteFail 不使用
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail")->Checked      =false;
        EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT")->Checked   =false;
    }

    if(CosFunction.bYieldAlarm5)                                                //Sam 20171213 (Steven) : 超豐良率監控 //Sam 20180423 (wei) : MOFile of Yeild Download
    {
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteIntervalContactCnt")->Text     =TestIF_File.iAlarm5_BySiteIntervalContactCnt;
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteLowYieldEnable")->Checked      =TestIF_File.bAlarm5_BySiteLowYieldEnable;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYield")->Text               =TestIF_File.dAlarm5_BySiteLowYield;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYieldRej")->Text            =TestIF_File.dAlarm5_BySiteLowYieldRej;
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteCmpYieldEnable")->Checked      =TestIF_File.bAlarm5_BySiteCmpYieldEnable;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYield")->Text               =TestIF_File.dAlarm5_BySiteCmpYield;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYieldRej")->Text            =TestIF_File.dAlarm5_BySiteCmpYieldRej;
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteAlarmYieldEnable")->Checked    =TestIF_File.bAlarm5_BySiteAlarmYieldEnable;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYield")->Text             =TestIF_File.dAlarm5_BySiteAlarmYield;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYieldRej")->Text          =TestIF_File.dAlarm5_BySiteAlarmYieldRej;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_OSBin")->Text                        =TestIF_File.iAlarm5_OSBin;
        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySitePreCmpYieldEnable")->Checked   =TestIF_File.bAlarm5_BySitePreCmpYieldEnable;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYield")->Text            =TestIF_File.dAlarm5_BySitePreCmpYield;
        EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYieldRej")->Text         =TestIF_File.dAlarm5_BySitePreCmpYieldRej;
    }

    EL<TCheckBox>("TfYieldMonitoring", "cbLoadCellMeasure")->Checked             = TestIF_File.bLoadCellMeasure;      //kevin 20190907 Arm 測區次數道量測 功能;
    EL<TEdit>("TfYieldMonitoring", "edLoadCellMeasure")->Text                = TestIF_File.iLoadCellCount;        //kevin 20190907 Arm 測區次數道量測

    if(CosFunction.bAdaptiveYield)                                              //Sam 20230914 : 自適應性良率監控
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_FT")->Checked  =TestIF_File.bAdaptiveLowYield;
        EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_FT")->Text  =TestIF_File.iAdaptiveContsLowerAlarmNor;                                       //Sam 20240726 : AI Clean
        EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_FT")->Text  =TestIF_File.iAdaptiveContsLowerAlarmMin;
        EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_FT")->Text     =TestIF_File.iAdaptiveYieldMax;
        EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_FT")->Text     =TestIF_File.iAdaptiveYieldMin;

        EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_RT")->Checked  =TestIF_File.bAdaptiveLowYield_RT;
        EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_RT")->Text  =TestIF_File.iAdaptiveContsLowerAlarmNor_RT;                                    //Sam 20240726 : AI Clean
        EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_RT")->Text  =TestIF_File.iAdaptiveContsLowerAlarmMin_RT;
        EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_RT")->Text     =TestIF_File.iAdaptiveYieldMax_RT;
        EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_RT")->Text     =TestIF_File.iAdaptiveYieldMin_RT;
    }
        TCheckBox *cbByBinFailCountCat_FT[16]={EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat0_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat1_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat2_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat3_FT"),                                 //JerryYang 20250120 : add
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat4_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat5_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat6_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat7_FT"),
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat8_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat9_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat10_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat11_FT"),
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat12_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat13_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat14_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat15_FT")};

        TCheckBox *cbByBinFailCountCat_RT[16]={EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat0_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat1_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat2_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat3_RT"),
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat4_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat5_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat6_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat7_RT"),
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat8_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat9_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat10_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat11_RT"),
                                             EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat12_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat13_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat14_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat15_RT")};

        TEdit *edByBinFailCountCat_FT[16]={EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_FT"),                 //JerryYang 20231206
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_FT"),
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_FT"),
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_FT")};

        TEdit *edByBinFailCountCat_RT[16]={EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_RT"),
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_RT"),
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_RT"),
                                         EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_RT")};

    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
        for(int i=0; i<TEST_MAX_BIN; i++)
        {
            cbByBinSiteGapCat_FT[i]->Checked=TestIF_File.bSpecBinBySiteCompareEnable[FT][i];
            cbByBinSiteGapCat_RT[i]->Checked=TestIF_File.bSpecBinBySiteCompareEnable[RT][i];
            edByBinSiteGapCat_FT[i]->Text   =TestIF_File.dSpecBinBySiteComparePercent[FT][i];
            edByBinSiteGapCat_RT[i]->Text   =TestIF_File.dSpecBinBySiteComparePercent[RT][i];
            cbByArmSiteGapCat_FT[i]->Checked=TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i];
            cbByArmSiteGapCat_RT[i]->Checked=TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i];
            edByArmSiteGapCat_FT[i]->Text   =TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i];
            edByArmSiteGapCat_RT[i]->Text   =TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i];
            cbByBinFailureCat_FT[i]->Checked=TestIF_File.bByBinFailureEnable[FT][i];
            cbByBinFailureCat_RT[i]->Checked=TestIF_File.bByBinFailureEnable[RT][i];
            edByBinFailureCat_FT[i]->Text   =TestIF_File.dByBinFailurePercent[FT][i];
            edByBinFailureCat_RT[i]->Text   =TestIF_File.dByBinFailurePercent[RT][i];
        }

        for(int i=0; i<16; i++)                                                 //RogerYang 20260409 : TEST_MAX_BIN會溢位，暫時放給它過
        {
            cbByBinFailCountCat_FT[i]->Checked=TestIF_File.bFailCountEnable[FT][i];      //JerryYang 20231207
            cbByBinFailCountCat_RT[i]->Checked=TestIF_File.bFailCountEnable[RT][i];
            edByBinFailCountCat_FT[i]->Text=TestIF_File.iFailCountLimit[FT][i];
            edByBinFailCountCat_RT[i]->Text=TestIF_File.iFailCountLimit[RT][i];
        }
        EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_FT")->Text        =TestIF_File.iSpecBinBySiteCompareIgnore[FT];
        EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_RT")->Text        =TestIF_File.iSpecBinBySiteCompareIgnore[RT];
        EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_FT")->Text        =TestIF_File.iSpecBinByArmPerSiteCompareIgnore[FT];
        EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_RT")->Text        =TestIF_File.iSpecBinByArmPerSiteCompareIgnore[RT];
        EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_FT")->Text        =TestIF_File.iByBinFailureIgnore[FT];
        EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_RT")->Text        =TestIF_File.iByBinFailureIgnore[RT];
        EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_FT")->Text=TestIF_File.iFailCountIgnore[FT];
        EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_RT")->Text=TestIF_File.iFailCountIgnore[RT];
    }

    if(CosFunction.bSpecailLowYeild)                                            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_Special")->Checked         =TestIF_File.bFailAlarmLowYieldSpecial;
        EL<TEdit>("TfYieldMonitoring", "edLowYield_Special")->Text            =TestIF_File.dLowYieldLimitSpecial;
        EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special1")->Text         =TestIF_File.iLowYieldCountSpecial1;
        EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special2")->Text         =TestIF_File.iLowYieldCountSpecial2;
    }

//Interval Low Yield Alarm(By Site)(%)                                          //wei 20180606 Interval Low Yield By Site
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_FT")->Checked    =TestIF_File.bFailAlarmIntervalLowYieldBySite;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT")->Text       =TestIF_File.iIntervalLowYieldLimitBySite;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT")->Text       =TestIF_File.dIntervalLowYieldLimitBySite;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_FT")->Text     =TestIF_File.iIntervalLowYieldCountBySite;

    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_RT")->Checked    =TestIF_File.bFailAlarmIntervalLowYieldBySite_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT")->Text       =TestIF_File.iIntervalLowYieldLimitBySite_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT")->Text       =TestIF_File.dIntervalLowYieldLimitBySite_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_RT")->Text     =TestIF_File.iIntervalLowYieldCountBySite_RT;

//Interval Low Yield Alarm(By Total)(%)                                         //wei 20180718 Interval Low Yield By Total
    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_FT")->Checked   =TestIF_File.bFailAlarmIntervalLowYieldByTotal;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT")->Text      =TestIF_File.iIntervalLowYieldLimitByTotal;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT")->Text      =TestIF_File.dIntervalLowYieldLimitByTotal;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_FT")->Text    =TestIF_File.iIntervalLowYieldCountByTotal;

    EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_RT")->Checked   =TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT")->Text      =TestIF_File.iIntervalLowYieldLimitByTotal_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT")->Text      =TestIF_File.dIntervalLowYieldLimitByTotal_RT;
    EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_RT")->Text    =TestIF_File.iIntervalLowYieldCountByTotal_RT;

    if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;
    {
        EL<TCheckBox>("TfYieldMonitoring", "chkCreateManualEOCAP")->Checked=TestIF_File.bCreateManualEOCAP;
    }

}

// golden uYieldMonitoring.cpp:6042  TfYieldMonitoring::SetClosedSiteBin()
static void YM_SetClosedSiteBin()
{

    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->Clear();
    for(int i=0; i<iTestBinCount; i++)
    {
        EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->Items->Add(i);                                        //Steven 20240409 : 關site的位置有IC不測試送指定 bin
    }
    EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->Items->Add("Error");

}

// golden uYieldMonitoring.cpp:3248  TfYieldMonitoring::CheckSettingNo()
static void YM_CheckSettingNo()
{
    filerw::ELMark("CheckSettingNo");   // FileRW：存檔流程 trace（ack.trace）

//    TestIF_File.iIgnoreIC                   =CheckRange(int(TestIF_File.iIgnoreIC),                    100000, 1);
    TestIF_File.iContinuousPassBin          =CheckRange(int(TestIF_File.iContinuousPassBin),           16, 1);
    TestIF_File.iContinuousPassBin_RT       =CheckRange(int(TestIF_File.iContinuousPassBin_RT),        16, 1);
    TestIF_File.iContinuousPassBinCount     =CheckRange(int(TestIF_File.iContinuousPassBinCount),      iMinCount, iMaxCount);
    TestIF_File.iContinuousPassBinCount_RT  =CheckRange(int(TestIF_File.iContinuousPassBinCount_RT),   iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount      =CheckRange(int(TestIF_File.iContinuousLoaderCount),       iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount_RT   =CheckRange(int(TestIF_File.iContinuousLoaderCount_RT),    iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount     =CheckRange(int(TestIF_File.iContinuousContactCount),      iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount_RT  =CheckRange(int(TestIF_File.iContinuousContactCount_RT),   iMinCount, iMaxCount);

}

// golden uYieldMonitoring.cpp:3266  TfYieldMonitoring::FormClose(TObject *Sender, TCloseAction &Action)
static void YM_FormClose()
{

    fShow=false;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden uYieldMonitoring.cpp:3092  TfYieldMonitoring::btnApplyClick(TObject *Sender)
static void YM_btnApplyClick()
{
    filerw::ELMark("btnApplyClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden uYieldMonitoring.cpp:3098-3098 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:3098-3098 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）
        Close();
#endif // GATE (S12-C save)
        return;
    }
    AnsiString S="";
    if(bGPIBChangeAlarm==false)                                                 //wei 20151127 GPIB Change Alarm
        YM_DoFormToData();
    YM_CheckSettingNo();                                                           //Steven 20110506

    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
       LastSet.iRunStartMode==rsmContinuRetest_ART &&
       bART_needRT2==false)                                                     //kevin 20150717    add
    {
        TestIF_File.bContsFailBySocket=false;                                   //kevin 20150812 ART_分tray不卡 連續icfail
        TestIF_File.bContsFailByHead=false;
    }

    YM_SaveSetupFile(GetRecipePath(), GetLastOpenFN());                            //kevin 20150105 Start   另存 jobfile
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(S, "JOBFILE");
    #endif

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150812  add
    {
        YM_ReadFile();                                                             //kevin 20150812  add
        YM_DoIniDataToForm();                                                      //kevin 20150812 add
    }

    if(CosFunction.bPiggybackFunctionByHandler==true) filerw::ELTodo("golden uYieldMonitoring.cpp:3127 SaveSetupFileToConfig(AuthPath, config.ini) [Alarm] Count Action/Continuous * NOT written -- config.ini is owned by FileRW/IniConfig");   // golden uYieldMonitoring.cpp:3125-3128 取代：SaveSetupFileToConfig(AuthPath,"config.ini")：config.ini 由 FileRW/IniConfig 擁有（C 路唯一寫者），這裡不寫
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:3125-3128 -- SaveSetupFileToConfig(AuthPath,"config.ini")：config.ini 由 FileRW/IniConfig 擁有（C 路唯一寫者），這裡不寫
    if(CosFunction.bPiggybackFunctionByHandler==true)                           //Isaac 20170712 :Piggyback function By Handlder(save file to config.ini)
    {
        SaveSetupFileToConfig(AuthPath, "config.ini");
    }
#endif // GATE (S12-C save)

    if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden uYieldMonitoring.cpp:3130-3149 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");   // golden uYieldMonitoring.cpp:3130-3149 取代：Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／FTestIF／SearchRecipeParameter 未移植）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:3130-3149 -- Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／FTestIF／SearchRecipeParameter 未移植）
    if(CosFunction.bRecipeParameterDefault)                                     //Sam 20201209 : Default Recipe ChangeLog
    {
        fRPDefault->RP_Name->Clear()    ;
        fRPDefault->RP_Default->Clear() ;
        fRPDefault->RP_Value->Clear()   ;

        if(CosFunction.bRecipeParameterDefaultLog)
        {
            fSpeed->SearchRecipeParameter(fSpeed->tsAllSpeed, -1);              //Isaac 20171227 (Steven) : AutoSpeed RPD一鍵復原
            fSpeed->SearchRecipeParameter(fSpeed->tsIndex, 0);
            fSpeed->SearchRecipeParameter(fSpeed->tsInArm, 1);
            fSpeed->SearchRecipeParameter(fSpeed->tsOutArm, 2);
            fSpeed->SearchRecipeParameter(fSpeed->tsTrayArm, 3);
            fSpeed->SearchRecipeParameter(fSpeed->tsShuttle, 4);
            fCleaning->SearchRecipeParameter(fCleaning, 0);
            FTestIF->SearchRecipeParameter(FTestIF, 0);
        }
        SearchRecipeParameter(fYieldMonitoring, 0);
        fRPDefault->CompareRPDefaultAndValue(fRPDefault->RP_Name, fRPDefault->RP_Default, fRPDefault->RP_Value);
    }
#endif // GATE (S12-C save)

    EL<TButton>("TfYieldMonitoring", "btnApply")->Enabled=false;
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

    if(IniConfig.bSIGURDFunction &&
       fLotInfo->pgLotinfo->ActivePage==fLotInfo->tsYieldMonitior)              //Sam 20210331 : 北興新增 Yield Monitor
    {
        fLotInfo->RefreshYieldMonitor();
    }

    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }
    fLotInfo->RefreshYieldMonitor();                                            //Sam 20210916 : 新增 Yiled Monitor 到畫面上

}

// golden uYieldMonitoring.cpp:1708  TfYieldMonitoring::DoFormToData()
static void YM_DoFormToData()
{
    filerw::ELMark("DoFormToData");   // FileRW：存檔流程 trace（ack.trace）

//Low Yield Alarm(%)
    TestIF_File.bFailAlarmLowYield                  =EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_FT")->Checked;
    TestIF_File.iLowYieldLimit                      =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dLowYieldLimit                      =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edLowYield_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iLowYieldCount                      =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_FT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.bLowYieldAlarmByBin                 =EL<TCheckBox>("TfYieldMonitoring", "cbYieldAlarmByBin")->Checked;                                        //Steven 20140828 : By Bin Yield Monitor
    TestIF_File.bSlidingWindowYield                 =EL<TCheckBox>("TfYieldMonitoring", "cbSlidingWindowYield")->Checked;                                     //Steven 20260331 : Sliding Window Yield
    TestIF_File.iSlidingWindowSize                  =CheckRange(atoi(EL<TLabeledEdit>("TfYieldMonitoring", "edSlidingWindowSize")->Text.c_str()), 10, 2000);     //Steven 20260331

    TestIF_File.bFailAlarmLowYield_RT               =EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_RT")->Checked;
    TestIF_File.iLowYieldLimit_RT                   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYield_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dLowYieldLimit_RT                   =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edLowYield_RT")->Text.c_str()), dMinYield, dMaxYield);                                       //JerryYang 20160530 LowYieldLimit要能設定到小數點
    TestIF_File.iLowYieldCount_RT                   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_RT")->Text.c_str()), iMinCount, iMaxCount);

    if(CosFunction.bLowYieldAutoSiteOff)                                        //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    {
        TestIF_File.bLowYieldAutoSiteOff            =EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOff")->Checked;
        TestIF_File.bLowYieldAutoSiteOffByArmSite   =EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmBySite")->Checked;                                //Steven 20230223 : by arm by site, auto site off
        TestIF_File.bLowYieldAutoSiteOffByContiFail =EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByContiFail")->Checked;                                //Steven 20200420 : Continue fail, auto site off
        TestIF_File.bLowYieldAutoSiteOffArmContiFail=EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffByArmContiFail")->Checked;                             //Steven 20220818 : By Arm Continue fail, auto site off
        TestIF_File.bLowYieldAutoSiteOffByPicker    =EL<TCheckBox>("TfYieldMonitoring", "chkchkAutoSiteOffByPicker")->Checked;                                //Steven 20230223 : 根據Index吸嘴比較良率
        TestIF_File.iAlarmWhenSiteOnCountLess       =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edtSiteYieldCmp1SiteCnt_FT")->Text.c_str()), 2, 32);
        TestIF_File.iCloseSiteOnHPDontTest          =EL<TRadioGroup>("TfYieldMonitoring", "rgCloseSiteOnHPAction")->ItemIndex;                                  //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
        TestIF_File.iCloseSiteBin                   =EL<TComboBox>("TfYieldMonitoring", "cbbClosedSiteBin")->ItemIndex;                                       //Steven 20240409 : 關site的位置有IC不測試送指定 bin
        TestIF_File.bLowYieldAutoSiteOffAlarm       =EL<TCheckBox>("TfYieldMonitoring", "chkAutoSiteOffAlarm")->Checked;                                      //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        TestIF_File.iLowYieldAutoSiteOffAlarm       =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffAlarm")->Text.c_str()), 2, 10);        //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
    }

//Site Yield Alarm(%)
    TestIF_File.bFailAlarmSiteYieldDifferent        =EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_FT")->Checked;
    TestIF_File.iFailAlarmSiteYield                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dFailAlarmSiteYield                 =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iFailAlarmSiteYieldDifferentCount   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_FT")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bFailAlarmSiteYieldDifferent_RT     =EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldDifferent_RT")->Checked;
    TestIF_File.iFailAlarmSiteYield_RT              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dFailAlarmSiteYield_RT              =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferent_RT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iFailAlarmSiteYieldDifferentCount_RT=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldDifferentIg_RT")->Text.c_str()), iMinCount, iMaxCount);

//Site Yield Alarm(%)
    TestIF_File.bFailAlarmSiteYieldCmp              =EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_FT")->Checked;

    TestIF_File.iFailAlarmSiteYieldCmp              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dFailAlarmSiteYieldCmp              =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iFailAlarmSiteYieldCmpCount         =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_FT")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bFailAlarmSiteYieldCmp_RT           =EL<TCheckBox>("TfYieldMonitoring", "cbSiteYieldCmp_RT")->Checked;
    TestIF_File.iFailAlarmSiteYieldCmp_RT           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dFailAlarmSiteYieldCmp_RT           =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmp_RT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iFailAlarmSiteYieldCmpCount_RT      =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edSiteYieldCmpIg_RT")->Text.c_str()), iMinCount, iMaxCount);
    ;   // golden uYieldMonitoring.cpp:1760-1760 取代：iYieldAlarmCheckIntervalByCount 移植樹沒有（SaveSetupFile :2139 記 ELTodo）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:1760-1760 -- iYieldAlarmCheckIntervalByCount 移植樹沒有（SaveSetupFile :2139 記 ELTodo）
    TestIF_File.iYieldAlarmCheckIntervalByCount     =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edtYieldAlarmCheckIntervalByCount")->Text.c_str()), iMinCount, iMaxCount);   //AI(rf360-yield-count) 20260814 (RogerYang)
#endif // GATE (S12-C save)

//Alarm Action
    TestIF_File.iCountAlarmAction                   =CheckRange(EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_FT")->ItemIndex, 0, 1);
    TestIF_File.iCountAlarmAction_RT                =CheckRange(EL<TRadioGroup>("TfYieldMonitoring", "rgPiggyBack_RT")->ItemIndex, 0, 1);

//Low Yield Alarm(%)                                                            //wei 20151116 Low Yield By Total
    TestIF_File.bFailAlarmLowYieldByTotal           =EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_FT")->Checked;
    TestIF_File.iLowYieldLimitByTotal               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dLowYieldLimitByTotal               =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iLowYieldCountByTotal               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_FT")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bFailAlarmLowYieldByTotal_RT        =EL<TCheckBox>("TfYieldMonitoring", "cbLowYieldByTotal_RT")->Checked;
    TestIF_File.iLowYieldLimitByTotal_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dLowYieldLimitByTotal_RT            =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotal_RT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iLowYieldCountByTotal_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldByTotalIg_RT")->Text.c_str()), iMinCount, iMaxCount);

//By Picker Compare Yield   [每個吸嘴的良率互比]                                //Steven 20230223 : 根據Index吸嘴比較良率
    TestIF_File.bLowYieldByPicker                   =EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_FT")->Checked;
    TestIF_File.bLowYieldByPicker_RT                =EL<TCheckBox>("TfYieldMonitoring", "chkLowYieldByPicker_RT")->Checked;
    TestIF_File.iLowYieldCountByPicker              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_FT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.dLowYieldByPicker                   =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iLowYieldCountByPicker_RT           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPickerIg_RT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.dLowYieldByPicker_RT                =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edtLowYieldByPicker_RT")->Text.c_str()), dMinYield, dMaxYield);

//Interval Low Yield Alarm(%)                                                   //wei 20180606 Interval Low Yield By Site
    if(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_FT")->Text.c_str())!=TestIF_File.iIntervalLowYieldCountBySite ||
       atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_RT")->Text.c_str())!=TestIF_File.iIntervalLowYieldCountBySite_RT )
    {
        for(int i=0; i<2; i++)
        {
            iYieldSiteCount[i]=0;
            bYieldSiteBin[i]=false;
        }
        memset(bIntervalYieldIsPass, 0, sizeof(bIntervalYieldIsPass));          // 20180629
    }

    if(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_FT")->Text.c_str())!=TestIF_File.iIntervalLowYieldCountByTotal ||
       atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_RT")->Text.c_str())!=TestIF_File.iIntervalLowYieldCountByTotal_RT )
    {
        iYieldTotalCount=0;                                                     //wei 20180718 Interval Low Yield By Total
        bYieldTotalBin=false;
        memset(bYieldTotalBinIsPass, 0, sizeof(bYieldTotalBinIsPass));          // 20180629
    }

    TestIF_File.bFailAlarmIntervalLowYieldBySite           =EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_FT")->Checked;
    TestIF_File.iIntervalLowYieldLimitBySite               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dIntervalLowYieldLimitBySite               =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iIntervalLowYieldCountBySite               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_FT")->Text.c_str()), 1, 300);

    TestIF_File.bFailAlarmIntervalLowYieldBySite_RT        =EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldBySite_RT")->Checked;
    TestIF_File.iIntervalLowYieldLimitBySite_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dIntervalLowYieldLimitBySite_RT            =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySite_RT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iIntervalLowYieldCountBySite_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldBySiteIg_RT")->Text.c_str()), 1, 300);

    TestIF_File.bFailAlarmIntervalLowYieldByTotal          =EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_FT")->Checked;
    TestIF_File.iIntervalLowYieldLimitByTotal              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dIntervalLowYieldLimitByTotal              =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_FT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iIntervalLowYieldCountByTotal              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_FT")->Text.c_str()), 1, 300);

    TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT       =EL<TCheckBox>("TfYieldMonitoring", "cbIntervalLowYieldByTotal_RT")->Checked;
    TestIF_File.iIntervalLowYieldLimitByTotal_RT           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.dIntervalLowYieldLimitByTotal_RT           =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotal_RT")->Text.c_str()), dMinYield, dMaxYield);
    TestIF_File.iIntervalLowYieldCountByTotal_RT           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edIntervalLowYieldByTotalIg_RT")->Text.c_str()), 1, 300);

//Continuous Pass
    if(IniConfig.bD56YieldPiggyBackEnable)
    {
        TestIF_File.bContinuousLoader               =true;
        TestIF_File.bContinuousLoader_RT            =true;
        TestIF_File.bContinuousPassBySocket         =true;
        TestIF_File.bContinuousPassBySocket_RT      =true;
        TestIF_File.bContinuousContact              =true;
        TestIF_File.bContinuousContact_RT           =true;
        TestIF_File.bContinuousPass_RT              =true;
        TestIF_File.bContinuousPass                 =true;
    }
    else
    {
        TestIF_File.bContinuousLoader               =EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_FT")->Checked;
        TestIF_File.bContinuousLoader_RT            =EL<TCheckBox>("TfYieldMonitoring", "cbContinuousLoad_RT")->Checked;
        TestIF_File.bContinuousPassBySocket         =EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_FT")->Checked;
        TestIF_File.bContinuousPassBySocket_RT      =EL<TCheckBox>("TfYieldMonitoring", "cbContinuPassSkt_RT")->Checked;
        TestIF_File.bContinuousContact              =EL<TCheckBox>("TfYieldMonitoring", "cbContactCountFT")->Checked;
        TestIF_File.bContinuousContact_RT           =EL<TCheckBox>("TfYieldMonitoring", "cbContactCountRT")->Checked;
        TestIF_File.bContinuousPass_RT              =EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_RT")->Checked;
        TestIF_File.bContinuousPass                 =EL<TCheckBox>("TfYieldMonitoring", "cbContinuousPass_FT")->Checked;
    }

    if(CosFunction.bPiggyBackForASE==true)                                      //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    {
        TestIF_File.bContinuousPass                 =false;
        TestIF_File.bContinuousPass_RT              =false;
        TestIF_File.bContinuousLoader               =false;
        TestIF_File.bContinuousLoader_RT            =false;
    }

    TestIF_File.iContinuousPassBin                  =CheckRange(EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_FT")->ItemIndex+1, 1, iTestBinCount);   //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
    TestIF_File.iContinuousPassBinCount             =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuousPass_FT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousPassBin_RT               =CheckRange(EL<TComboBox>("TfYieldMonitoring", "cobContinuousPass_RT")->ItemIndex+1, 1, iTestBinCount);   //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
    TestIF_File.iContinuousPassBinCount_RT          =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuousPass_RT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousPassBinCountBySocket     =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_FT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousPassBinCountBySocket_RT  =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuPassSkt_RT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount             =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContactCountFT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount_RT          =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContactCountRT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount              =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_FT")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount_RT           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContinuousLoad_RT")->Text.c_str()), iMinCount, iMaxCount);

//Consecutive Failure Alarm ( Socket )
    TestIF_File.bContsFailBySocket                  =EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_FTOn")->Checked;
    //TestIF_File.iContsFailSocketAlarmCT             =CheckRange(atoi(edContsFailSocketAlarmCT_FT->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iContsFailSocketAlarmCT             =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_FT")->Text.c_str()), iMinCount, iMaxCount);     //AI(ht9045-yield-flow) 20260713 (RogerYang) : ContsFail count 誤用 iMaxYield(100) 夾住，改用 iMaxCount

    TestIF_File.bCountSpcBinContinuously_FT         =EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_FT")->Checked;                                //Steven 20230529 : Spc Bin Couont改成連續錯誤
    TestIF_File.bCountSpcBinContinuously_RT         =EL<TCheckBox>("TfYieldMonitoring", "chkSpcBinCntAsContinue_RT")->Checked;                                //Steven 20230529 : Spc Bin Couont改成連續錯誤

    TestIF_File.bContsFailBySocket_RT               =EL<TRadioButton>("TfYieldMonitoring", "rbContsFailBySocket_RTOn")->Checked;
    //TestIF_File.iContsFailSocketAlarmCT_RT          =CheckRange(atoi(edContsFailSocketAlarmCT_RT->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iContsFailSocketAlarmCT_RT          =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailSocketAlarmCT_RT")->Text.c_str()), iMinCount, iMaxCount);     //AI(ht9045-yield-flow) 20260713 (RogerYang) : ContsFail count 誤用 iMaxYield(100) 夾住，改用 iMaxCount

//Consecutive Failure Alarm ( Head)
    TestIF_File.bContsFailByHead                    =EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_FTOn")->Checked;
//    TestIF_File.iContsFailHeadAlarmCT               =CheckRange(atoi(edContsFailHeadAlarmCT_FT->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iContsFailHeadAlarmCT               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_FT")->Text.c_str()), iMinCount, iMaxCount);       //AI(ht9045-yield-flow) 20260713 (RogerYang) : ContsFail count 誤用 iMaxYield(100) 夾住，改用 iMaxCount

    TestIF_File.bContsFailByHead_RT                 =EL<TRadioButton>("TfYieldMonitoring", "rbContsFailByHead_RTOn")->Checked;
//    TestIF_File.iContsFailHeadAlarmCT_RT            =CheckRange(atoi(edContsFailHeadAlarmCT_RT->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iContsFailHeadAlarmCT_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailHeadAlarmCT_RT")->Text.c_str()), iMinCount, iMaxCount);       //AI(ht9045-yield-flow) 20260713 (RogerYang) : ContsFail count 誤用 iMaxYield(100) 夾住，改用 iMaxCount

//Consecutive Failure Ignore                                                    //wei 20160115 銅鑼前幾顆不計算ContsFail
    TestIF_File.bContsFailIgnore                    =EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_FT")->Checked;
    TestIF_File.iContsFailIgnore                    =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_FT")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bContsFailIgnore_RT                 =EL<TCheckBox>("TfYieldMonitoring", "cbContsFailIgnore_RT")->Checked;
    TestIF_File.iContsFailIgnore_RT                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edContsFailIgnore_RT")->Text.c_str()), iMinCount, iMaxCount);

    //wei 20141201 Low Yield Auto Clean(%) start
    TestIF_File.bFailAlarmLowYield_AutoClean        =EL<TRadioGroup>("TfYieldMonitoring", "rgAutoCleanYieldOnOff")->ItemIndex;
    TestIF_File.iLowYieldLimit_AutoClean            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldLimit")->Text.c_str()), 1, 50);
    TestIF_File.iLowYieldCount_AutoClean            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAutoCleanLowYieldCount")->Text.c_str()), iMinCount, iMaxCount);
    //wei 20141201 Low Yield Auto Clean(%) end

    TestIF_File.iAutoCloseSiteWhenRT                =EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOffWhenRT")->ItemIndex;                                    //Steven 20200225 : 切到RT的時候,要關閉Socket
    TestIF_File.dAutoCloseSiteYieldWhenRT           =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edtAutoSiteOffWhenRT")->Text.c_str()), 50.00, 0.01);                                         //Steven 20200225 : 切到RT的時候,要關閉Socket
    TestIF_File.iAllSiteOnAtInitialStart            =EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOn")->ItemIndex;   //Steven 20230814 : Initial Start的時候要全開Site
    TestIF_File.iAutoSiteOffByGPIB                  =EL<TRadioGroup>("TfYieldMonitoring", "rgAutoSiteOnByGPIB")->ItemIndex;                                     //JimmyChiu 20250715 : Auto site on/off by GPIB

    if(IniConfig.bE53LowYieldAutoClean==false)                                  //kevin 20160802
        TestIF_File.bFailAlarmLowYield_AutoClean=false;

    //wei 20160314
    TestIF_File.bAlarm4ContinueType_Enable          =EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType")->Checked;
    TestIF_File.iAlarm4ContinueType_IntervalCount   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iAlarm4ContinueType_ContinueCount   =atoi(EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount")->Text.c_str());

    TestIF_File.bAlarm4EnableIntervalYield          =EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield")->Checked ;
    TestIF_File.iAlarm4IntervalYieldIntervalCount   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount")->Text.c_str()), iMinCount, iMaxCount);
    TestIF_File.iAlarm4IntervalYieldContinueCount   =atoi(EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount")->Text.c_str());
    TestIF_File.iAlarm4IntervalYieldYield           =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")->Text.c_str()), iMinYield, iMaxYield);

    TestIF_File.bSiteToSiteYieldCmp                 =EL<TCheckBox>("TfYieldMonitoring", "cb_SiteToSiteYieldEnable")->Checked;
    TestIF_File.iSiteToSiteYieldCmp                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iSiteToSiteYieldCmpCount            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bHeadToHeadYieldCmp                 =EL<TCheckBox>("TfYieldMonitoring", "cb_HeadToHeadYieldEnable")->Checked;
    TestIF_File.iHeadToHeadYieldCmp                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iHeadToHeadYieldCmpCount            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bSiteYieldOverAlert                 =EL<TCheckBox>("TfYieldMonitoring", "cb_SiteYieldOverAlert")->Checked;
    TestIF_File.iSiteYieldOverAlert                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert")->Text.c_str()), iMinYield, iMaxYield);
    TestIF_File.iSiteYieldOverAlertCount            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount")->Text.c_str()), iMinCount, iMaxCount);

    TestIF_File.bAllSiteFail                        =EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail")->Checked;    //kevin 20170825 (wei) 整支ARM Fail bin
    TestIF_File.iAllSiteFailCount                   =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount")->Text.c_str()), 0, 500);        //kevin 20180720 (wei) all site fail count
    TestIF_File.iAllSiteFailCountRT                 =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT")->Text.c_str()), 0, 500);
    TestIF_File.bAllSiteFail_RT                     =EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT")->Checked;                                         //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20200603 ：矽格-湖口，Yield 頁面 AllSiteFail 不使用
    {
        TestIF_File.bAllSiteFail        =false;
        TestIF_File.bAllSiteFail_RT     =false;
    }
    //kevin 20150529 AutoRetest Start
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        iAutoRetestLimit= MyYieldPanel[0]->ReTestLimit->ItemIndex+1;            //kevin 20150704  //kevin 20150601 做AUTO RETEST最大次數

    if(CosFunction.bAdaptiveYield)                                              //Sam 20230914 : 自適應性良率監控
    {
        TestIF_File.bAdaptiveLowYield           =EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_FT")->Checked;
        TestIF_File.iAdaptiveContsLowerAlarmNor     =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_FT")->Text.c_str()), iMinYield, iMaxYield);                            //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin     =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_FT")->Text.c_str()), iMinYield, iMaxYield);
        TestIF_File.iAdaptiveYieldMax               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_FT")->Text.c_str()), 1, 20);
        TestIF_File.iAdaptiveYieldMin               =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_FT")->Text.c_str()), 1, 20);

        TestIF_File.bAdaptiveLowYield_RT        =EL<TCheckBox>("TfYieldMonitoring", "cbAdaptiveLowYield_RT")->Checked ;
        TestIF_File.iAdaptiveContsLowerAlarmNor_RT  =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmNor_RT")->Text.c_str()), iMinYield, iMaxYield);                            //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin_RT  =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edlContsLowerAlarmMin_RT")->Text.c_str()), iMinYield, iMaxYield);
        TestIF_File.iAdaptiveYieldMax_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMax_RT")->Text.c_str()), 1, 20);
        TestIF_File.iAdaptiveYieldMin_RT            =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edAdaptiveYieldMin_RT")->Text.c_str()), 1, 20);
    }

    TCheckBox *cbByBinFailCountCat_FT[16]={EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat0_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat1_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat2_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat3_FT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat4_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat5_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat6_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat7_FT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat8_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat9_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat10_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat11_FT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat12_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat13_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat14_FT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat15_FT")};

    TCheckBox *cbByBinFailCountCat_RT[16]={EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat0_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat1_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat2_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat3_RT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat4_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat5_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat6_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat7_RT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat8_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat9_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat10_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat11_RT"),
                                         EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat12_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat13_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat14_RT"),EL<TCheckBox>("TfYieldMonitoring", "cbBybinLimitCat15_RT")};

    TEdit *edByBinFailCountCat_FT[16]={EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_FT"),                     //JerryYang 20231206
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_FT"),
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_FT"),
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_FT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_FT")};

    TEdit *edByBinFailCountCat_RT[16]={EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat0_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat1_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat2_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat3_RT"),
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat4_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat5_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat6_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat7_RT"),
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat8_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat9_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat10_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat11_RT"),
                                     EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat12_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat13_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat14_RT"),EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCat15_RT")};

    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
        for(int i=0; i<TEST_MAX_BIN; i++)
        {
            TestIF_File.bSpecBinBySiteCompareEnable[FT][i]          =(i<iTestBinCount)?cbByBinSiteGapCat_FT[i]->Checked:false;
            TestIF_File.bSpecBinBySiteCompareEnable[RT][i]          =(i<iTestBinCount)?cbByBinSiteGapCat_RT[i]->Checked:false;
            TestIF_File.dSpecBinBySiteComparePercent[FT][i]         =CheckRange(atof(edByBinSiteGapCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);
            TestIF_File.dSpecBinBySiteComparePercent[RT][i]         =CheckRange(atof(edByBinSiteGapCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]    =(i<iTestBinCount)?cbByArmSiteGapCat_FT[i]->Checked:false;
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]    =(i<iTestBinCount)?cbByArmSiteGapCat_RT[i]->Checked:false;
            TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]   =CheckRange(atof(edByArmSiteGapCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);
            TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]   =CheckRange(atof(edByArmSiteGapCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);
            TestIF_File.bByBinFailureEnable[FT][i]                  =(i<iTestBinCount)?cbByBinFailureCat_FT[i]->Checked:false;
            TestIF_File.bByBinFailureEnable[RT][i]                  =(i<iTestBinCount)?cbByBinFailureCat_RT[i]->Checked:false;
            TestIF_File.dByBinFailurePercent[FT][i]                 =CheckRange(atof(edByBinFailureCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);
            TestIF_File.dByBinFailurePercent[RT][i]                 =CheckRange(atof(edByBinFailureCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);
        }

        for(int i=0; i<16; i++)                                                 //RogerYang 20260409 : TEST_MAX_BIN會溢位，暫時放給它過
        {
            TestIF_File.bFailCountEnable[FT][i]=cbByBinFailCountCat_FT[i]->Checked;                                     //JerryYang 20231206
            TestIF_File.bFailCountEnable[RT][i]=cbByBinFailCountCat_RT[i]->Checked;
            TestIF_File.iFailCountLimit[FT][i]=CheckRange(atoi(edByBinFailCountCat_FT[i]->Text.c_str()), 0, 100000);
            TestIF_File.iFailCountLimit[RT][i]=CheckRange(atoi(edByBinFailCountCat_RT[i]->Text.c_str()), 0, 100000);
        }
        TestIF_File.iSpecBinBySiteCompareIgnore[FT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_FT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iSpecBinBySiteCompareIgnore[RT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinSiteGapCatCT_RT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[FT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_FT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[RT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByArmSiteGapCatCT_RT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iByBinFailureIgnore[FT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_FT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iByBinFailureIgnore[RT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinFailureCatCT_RT")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iFailCountIgnore[FT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_FT")->Text.c_str()), 20, 100000);         //JerryYang 20231206
        TestIF_File.iFailCountIgnore[RT]=CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edByBinLimitCountCatCT_RT")->Text.c_str()), 20, 100000);
    }

    if(CosFunction.bSpecailLowYeild)                                            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        TestIF_File.bFailAlarmLowYieldSpecial   =EL<TCheckBox>("TfYieldMonitoring", "cbLowYield_Special")->Checked;
        TestIF_File.iLowYieldLimitSpecial       =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYield_Special")->Text.c_str()), iMinYield, iMaxYield);
        TestIF_File.dLowYieldLimitSpecial       =CheckRange(atof(EL<TEdit>("TfYieldMonitoring", "edLowYield_Special")->Text.c_str()), dMinYield, dMaxYield);
        TestIF_File.iLowYieldCountSpecial1      =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special1")->Text.c_str()), iMinCount, iMaxCount);
        TestIF_File.iLowYieldCountSpecial2      =CheckRange(atoi(EL<TEdit>("TfYieldMonitoring", "edLowYieldIg_Special2")->Text.c_str()), iMinCount, iMaxCount);
    }

    if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;
    {
        TestIF_File.bCreateManualEOCAP=EL<TCheckBox>("TfYieldMonitoring", "chkCreateManualEOCAP")->Checked;
    }

}

// golden uYieldMonitoring.cpp:2033  TfYieldMonitoring::SaveSetupFile(AnsiString szDir, AnsiString S)
static void YM_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    MyForceDirectories(szDir, "TfYieldMonitoring::SaveSetupFile");
    szDir+="Tester.Data";

//Low Yield Alarm(%)
    WriteIniData(szDir, "Low Yield Alarm", "Enable",     int(TestIF_File.bFailAlarmLowYield));
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            WriteIniData(szDir, "Low Yield Alarm", "Limit",    TestIF_File.dLowYieldLimit);
            WriteIniData(szDir, "Low Yield Alarm", "Limit RT", TestIF_File.dLowYieldLimit_RT);
        }
        else
        {
            WriteIniData(szDir, "Low Yield Alarm", "Double Limit",    TestIF_File.dLowYieldLimit);
            WriteIniData(szDir, "Low Yield Alarm", "Double Limit RT", TestIF_File.dLowYieldLimit_RT);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        WriteIniData(szDir, "Low Yield Alarm", "Limit",      int(TestIF_File.iLowYieldLimit));
        WriteIniData(szDir, "Low Yield Alarm", "Limit RT",   int(TestIF_File.iLowYieldLimit_RT));
    }
    WriteIniData(szDir, "Low Yield Alarm", "Count",      int(TestIF_File.iLowYieldCount));
    WriteIniData(szDir, "Low Yield Alarm", "Enable By Bin Setting", int(TestIF_File.bLowYieldAlarmByBin));              //Steven 20140828 : By Bin Yield Monitor
    WriteIniData(szDir, "Low Yield Alarm", "Sliding Window Enable", int(TestIF_File.bSlidingWindowYield));              //Steven 20260331 : Sliding Window Yield
    WriteIniData(szDir, "Low Yield Alarm", "Sliding Window Size",   int(TestIF_File.iSlidingWindowSize));               //Steven 20260331
    WriteIniData(szDir, "Low Yield Alarm", "Enable RT",  int(TestIF_File.bFailAlarmLowYield_RT));

    WriteIniData(szDir, "Low Yield Alarm", "Count RT",   int(TestIF_File.iLowYieldCount_RT));

//    TestIF_File.iIgnoreIC       =CheckRange(int(TestIF_File.iIgnoreIC),    0, 100000);
//    TestIF_File.iIgnoreIC_RT    =CheckRange(int(TestIF_File.iIgnoreIC_RT), 0, 100000);

    if(CosFunction.bLowYieldAutoSiteOff)                                        //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    {
        WriteIniData(szDir, "Site Yield Alarm", "Alarm When Site On Count Less Than",   TestIF_File.iAlarmWhenSiteOnCountLess);
        WriteIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off",              TestIF_File.bLowYieldAutoSiteOff);
        WriteIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffByContiFail",      TestIF_File.bLowYieldAutoSiteOffByContiFail);                           //Steven 20200420 : Continue fail, auto site off
        WriteIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffArmContiFail",     TestIF_File.bLowYieldAutoSiteOffArmContiFail);                          //Steven 20220818 : By Arm Continue fail, auto site off
        WriteIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm",        TestIF_File.bLowYieldAutoSiteOffAlarm);                                 //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        WriteIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm Count",  TestIF_File.iLowYieldAutoSiteOffAlarm);                                 //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        WriteIniData(szDir, "Site Yield Alarm", "By Arm Site Low Yield Auto Site Off ", TestIF_File.bLowYieldAutoSiteOffByArmSite);                             //Steven 20230223 : by arm by site, auto site off
        WriteIniData(szDir, "Site Yield Alarm", "By Picker Low Yield Auto Site Off ",   TestIF_File.bLowYieldAutoSiteOffByPicker);                              //Steven 20230223 : 根據Index吸嘴比較良率

        if(IniConfig.bI28_OnOffSiteOnTheFly ||
           TestIF_File.bLowYieldAutoSiteOff ||
           TestIF_File.bLowYieldAutoSiteOffByContiFail)                         //Steven 20250604 : 關site的位置有IC不測試送指定 bin
        {
            WriteIniData(szDir, "Site Yield Alarm", "HP Close Site Do Not Test",        TestIF_File.iCloseSiteOnHPDontTest);                                    //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
            WriteIniData(szDir, "Site Yield Alarm", "Bin of Closed Site",               TestIF_File.iCloseSiteBin);     //Steven 20240409 : 關site的位置有IC不測試送指定 bin
        }
    }

//Site Yield Alarm(%)
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different",          int(TestIF_File.bFailAlarmSiteYieldDifferent));
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            WriteIniData(szDir, "Site Yield Alarm","Site Yield",    TestIF_File.dFailAlarmSiteYield);
            WriteIniData(szDir, "Site Yield Alarm","Site Yield RT", TestIF_File.dFailAlarmSiteYield_RT);
        }
        else
        {
            WriteIniData(szDir, "Site Yield Alarm","Double Site Yield",    TestIF_File.dFailAlarmSiteYield);
            WriteIniData(szDir, "Site Yield Alarm","Double Site Yield RT", TestIF_File.dFailAlarmSiteYield_RT);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        WriteIniData(szDir, "Site Yield Alarm","Site Yield",                int(TestIF_File.iFailAlarmSiteYield));
        WriteIniData(szDir, "Site Yield Alarm","Site Yield RT",             int(TestIF_File.iFailAlarmSiteYield_RT));
    }
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different Count",    int(TestIF_File.iFailAlarmSiteYieldDifferentCount));
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different RT",       int(TestIF_File.bFailAlarmSiteYieldDifferent_RT));
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Different Count RT", int(TestIF_File.iFailAlarmSiteYieldDifferentCount_RT));

//Site Yield Alarm(%)
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Compare",            int(TestIF_File.bFailAlarmSiteYieldCmp));
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            WriteIniData(szDir, "Site Yield Alarm","Site Yield Cmp",     TestIF_File.dFailAlarmSiteYieldCmp);
            WriteIniData(szDir, "Site Yield Alarm","Site Yield Cmp RT",  TestIF_File.dFailAlarmSiteYieldCmp_RT);
        }
        else
        {
            WriteIniData(szDir, "Site Yield Alarm","Double Site Yield Cmp",     TestIF_File.dFailAlarmSiteYieldCmp);
            WriteIniData(szDir, "Site Yield Alarm","Double Site Yield Cmp RT",  TestIF_File.dFailAlarmSiteYieldCmp_RT);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        WriteIniData(szDir, "Site Yield Alarm","Site Yield Cmp",            int(TestIF_File.iFailAlarmSiteYieldCmp));
        WriteIniData(szDir, "Site Yield Alarm","Site Yield Cmp RT",         int(TestIF_File.iFailAlarmSiteYieldCmp_RT));
    }
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Compare Count",      int(TestIF_File.iFailAlarmSiteYieldCmpCount));
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Compare RT",         int(TestIF_File.bFailAlarmSiteYieldCmp_RT));
    WriteIniData(szDir, "Site Yield Alarm","Site Yield Compare Count RT",   int(TestIF_File.iFailAlarmSiteYieldCmpCount_RT));
    filerw::ELTodo("golden uYieldMonitoring.cpp:2139 [Site Yield Alarm] Check Interval By Count NOT written -- TestIF_File.iYieldAlarmCheckIntervalByCount (912) missing from port SYSTEM_TEST_IF");   // golden uYieldMonitoring.cpp:2139-2139 取代：iYieldAlarmCheckIntervalByCount 移植樹沒有 → 這個鍵不寫（檔裡原值不動）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2139-2139 -- iYieldAlarmCheckIntervalByCount 移植樹沒有 → 這個鍵不寫（檔裡原值不動）
    WriteIniData(szDir, "Site Yield Alarm","Check Interval By Count",       int(TestIF_File.iYieldAlarmCheckIntervalByCount));  //AI(rf360-yield-count) 20260814 (RogerYang)
#endif // GATE (S12-C save)

//Low Yield Alarm(By Total)(%)                                                  //wei 20151116 Low Yield By Total
    WriteIniData(szDir, "Low Yield Alarm", "By Total Enable", int(TestIF_File.bFailAlarmLowYieldByTotal));
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            WriteIniData(szDir, "Low Yield Alarm", "By Total Limit",     TestIF_File.dLowYieldLimitByTotal);
            WriteIniData(szDir, "Low Yield Alarm", "By Total Limit RT",  TestIF_File.dLowYieldLimitByTotal_RT);
        }
        else
        {
            WriteIniData(szDir, "Low Yield Alarm", "Double By Total Limit",     TestIF_File.dLowYieldLimitByTotal);
            WriteIniData(szDir, "Low Yield Alarm", "Double By Total Limit RT",  TestIF_File.dLowYieldLimitByTotal_RT);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        WriteIniData(szDir, "Low Yield Alarm", "By Total Limit",            int(TestIF_File.iLowYieldLimitByTotal));
        WriteIniData(szDir, "Low Yield Alarm", "By Total Limit RT",         int(TestIF_File.iLowYieldLimitByTotal_RT));
    }
    WriteIniData(szDir, "Low Yield Alarm", "By Total Count",      int(TestIF_File.iLowYieldCountByTotal));
    WriteIniData(szDir, "Low Yield Alarm", "By Total Enable RT",  int(TestIF_File.bFailAlarmLowYieldByTotal_RT));
    WriteIniData(szDir, "Low Yield Alarm", "By Total Count RT",   int(TestIF_File.iLowYieldCountByTotal_RT));

//By Picker Compare Yield   [每個吸嘴的良率互比]                                //Steven 20230223 : 根據Index吸嘴比較良率
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Enable",      TestIF_File.bLowYieldByPicker        );
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Count",       TestIF_File.iLowYieldCountByPicker   );
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Limit",       TestIF_File.dLowYieldByPicker        );
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Enable RT",   TestIF_File.bLowYieldByPicker_RT     );
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Count RT",    TestIF_File.iLowYieldCountByPicker_RT);
    WriteIniData(szDir, "Low Yield Alarm", "By Picker Limit RT",    TestIF_File.dLowYieldByPicker_RT     );

//Low Yield Alarm(Special)(%)
    if(CosFunction.bSpecailLowYeild)                                            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        WriteIniData(szDir, "Low Yield Alarm", "Special Enable", int(TestIF_File.bFailAlarmLowYieldSpecial));
        if(CosFunction.bYieldAlarmUseDouble)                                    //JerryYang 20160530 LowYieldLimit要能設定到小數點
        {
            WriteIniData(szDir, "Low Yield Alarm", "Special Limit",     TestIF_File.dLowYieldLimitSpecial);
        }
        else
        {
            WriteIniData(szDir, "Low Yield Alarm", "Special Limit",      int(TestIF_File.iLowYieldLimitSpecial));
        }
        WriteIniData(szDir, "Low Yield Alarm", "Special Count1",      int(TestIF_File.iLowYieldCountSpecial1));
        WriteIniData(szDir, "Low Yield Alarm", "Special Count2",      int(TestIF_File.iLowYieldCountSpecial2));
    }

//Alarm Action
    WriteIniData(szDir, "Alarm",     "Count Action",    int(TestIF_File.iCountAlarmAction));                            //Steven 20101116
    WriteIniData(szDir, "Alarm",     "Count Action RT", int(TestIF_File.iCountAlarmAction_RT));

//Continuous Pass
    WriteIniData(szDir, "Alarm",     "Continuous Pass",              int(TestIF_File.bContinuousPass));
    WriteIniData(szDir, "Alarm",     "Continuous Pass Bin",          int(TestIF_File.iContinuousPassBin));
    WriteIniData(szDir, "Alarm",     "Continuous Pass Bin Count",    int(TestIF_File.iContinuousPassBinCount));
    WriteIniData(szDir, "Alarm",     "Continuous Pass RT",           int(TestIF_File.bContinuousPass_RT));
    WriteIniData(szDir, "Alarm",     "Continuous Pass RT Bin",       int(TestIF_File.iContinuousPassBin_RT));
    WriteIniData(szDir, "Alarm",     "Continuous Pass Bin Count RT", int(TestIF_File.iContinuousPassBinCount_RT));

//    WriteIniData(szDir, "Alarm",     "iBinAlarmByTotalOrBySite",     int(TestIF_File.iBinAlarmByTotalOrBySite));  //jou 2013-04-25 BIN Alarm By Total or By Site;
//    WriteIniData(szDir, "Alarm",     "iBinAlarmByTotalOrBySite_RT",  int(TestIF_File.iBinAlarmByTotalOrBySite_RT));  //jou 2013-04-25 BIN Alarm By Total or By Site;

//Continuous Pass by Socket - Steven 20110915
    WriteIniData(szDir, "Alarm",     "Continuous Pass By Socket",               int(TestIF_File.bContinuousPassBySocket));
    WriteIniData(szDir, "Alarm",     "Continuous Pass Bin Count By Socket",     int(TestIF_File.iContinuousPassBinCountBySocket));
    WriteIniData(szDir, "Alarm",     "Continuous Pass RT By Socket",            int(TestIF_File.bContinuousPassBySocket_RT));
    WriteIniData(szDir, "Alarm",     "Continuous Pass Bin Count RT By Socket",  int(TestIF_File.iContinuousPassBinCountBySocket_RT));

//Continuous Loader
    //jou 2010-11-17 Start
    WriteIniData(szDir, "Alarm",     "Continuous Loader",           int(TestIF_File.bContinuousLoader));
    WriteIniData(szDir, "Alarm",     "Continuous Loader Count",     int(TestIF_File.iContinuousLoaderCount));
    WriteIniData(szDir, "Alarm",     "Continuous Loader RT",        int(TestIF_File.bContinuousLoader_RT));
    WriteIniData(szDir, "Alarm",     "Continuous Loader Count RT",  int(TestIF_File.iContinuousLoaderCount_RT));
    //jou 2010-11-17 end

//Consecutive Failure Alarm ( Socket )
    WriteIniData(szDir, "Alarm",     "SocketEnable",    int(TestIF_File.bContsFailBySocket));
    WriteIniData(szDir, "Alarm",     "SocketCT",        int(TestIF_File.iContsFailSocketAlarmCT));
    WriteIniData(szDir, "Alarm",     "SocketEnable RT", int(TestIF_File.bContsFailBySocket_RT));
    WriteIniData(szDir, "Alarm",     "SocketCT RT",     int(TestIF_File.iContsFailSocketAlarmCT_RT));
    WriteIniData(szDir, "Alarm", "bCountSpcBinContinuously_FT", TestIF_File.bCountSpcBinContinuously_FT);               //Steven 20230529 : Spc Bin Couont改成連續錯誤
    WriteIniData(szDir, "Alarm", "bCountSpcBinContinuously_RT", TestIF_File.bCountSpcBinContinuously_RT);

//Consecutive Failure Alarm ( Head)
    WriteIniData(szDir, "Alarm",     "HeadEnable",      int(TestIF_File.bContsFailByHead));
    WriteIniData(szDir, "Alarm",     "HeadCT",          int(TestIF_File.iContsFailHeadAlarmCT));
    WriteIniData(szDir, "Alarm",     "HeadEnable RT",   int(TestIF_File.bContsFailByHead_RT));
    WriteIniData(szDir, "Alarm",     "HeadCT RT",       int(TestIF_File.iContsFailHeadAlarmCT_RT));

//Consecutive Failure Ignore                                                    //wei 20160115 銅鑼前幾顆不計算ContsFail
    WriteIniData(szDir, "Alarm",     "ContsFailIgnoreEnable",      int(TestIF_File.bContsFailIgnore));
    WriteIniData(szDir, "Alarm",     "ContsFailIgnoreCT",          int(TestIF_File.iContsFailIgnore));
    WriteIniData(szDir, "Alarm",     "ContsFailIgnoreEnable RT",   int(TestIF_File.bContsFailIgnore_RT));
    WriteIniData(szDir, "Alarm",     "ContsFailIgnoreCT RT",       int(TestIF_File.iContsFailIgnore_RT));

//Steven 20110426 Start
    WriteIniData(szDir, "Alarm",     "Continuous Contact",          int(TestIF_File.bContinuousContact));
    WriteIniData(szDir, "Alarm",     "Continuous Contact Count",    int(TestIF_File.iContinuousContactCount));
    WriteIniData(szDir, "Alarm",     "Continuous Contact RT",       int(TestIF_File.bContinuousContact_RT));
    WriteIniData(szDir, "Alarm",     "Continuous Contact Count RT", int(TestIF_File.iContinuousContactCount_RT));
//Steven 20110426 End

    //wei 20141201 Low Yield Auto Clean(%) start
    WriteIniData(szDir, "Low Yield Auto Clean", "Enable",     int(TestIF_File.bFailAlarmLowYield_AutoClean));
    WriteIniData(szDir, "Low Yield Auto Clean", "Count",      int(TestIF_File.iLowYieldCount_AutoClean));
    WriteIniData(szDir, "Low Yield Auto Clean", "Limit",      int(TestIF_File.iLowYieldLimit_AutoClean));

    if(CosFunction.bAutoCloseSiteWhenRT)                                        //Steven 20200225 : 切到RT的時候,要關閉Socket
    {
        WriteIniData(szDir, "Auto Close Site When RT", "iAutoCloseSiteYieldWhenRT",   TestIF_File.iAutoCloseSiteWhenRT);
        WriteIniData(szDir, "Auto Close Site When RT", "dAutoCloseSiteYieldWhenRT",   TestIF_File.dAutoCloseSiteYieldWhenRT);
        WriteIniData(szDir, "Site On at Initial Start", "iAllSiteOnAtInitialStart",   TestIF_File.iAllSiteOnAtInitialStart);                                    //Steven 20230814 : Initial Start的時候要全開Site
    }
    WriteIniData(szDir, "AutoSiteOnoff", "iAutoSiteOffByGPIB",   TestIF_File.iAutoSiteOffByGPIB);                       //JimmyChiu 20250715 : Auto site on/off by GPIB
    //---AutoRetest setup  kevin 20150704 start
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        WriteIniData(szDir, "AutoRetest", "ArtAutoRetestLimit", iAutoRetestLimit);                                      //RT 次數
        WriteIniData(szDir, "AutoRetest", "bPassART",           MyYieldPanel[0]->bEnablePassYieldART);
        WriteIniData(szDir, "AutoRetest", "fPassyieldART",      MyYieldPanel[0]->fPassYieldART);
        WriteIniData(szDir, "AutoRetest", "bOpenshortART",      MyYieldPanel[0]->bEnableOpenShortART);
        WriteIniData(szDir, "AutoRetest", "fOpenShortYieldART", MyYieldPanel[0]->fOpenShortYieldART);
        WriteIniData(szDir, "AutoRetest", "bRecoverART",        MyYieldPanel[0]->bEnableRecoverART);
        WriteIniData(szDir, "AutoRetest", "fRecoverYieldART",   MyYieldPanel[0]->fRecoverYieldART);

        AnsiString bBin="", bBin1="", abuffer="", abuffer1="";                  //kevin 20170825 (wei) add
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            if(MyYieldPanel[0]->bPass[i])
                abuffer.sprintf("%s,","1");
            else
                abuffer.sprintf("%s,","0");
            bBin+= abuffer;
            if(MyYieldPanel[0]->bOpenShort[i])
                abuffer1.sprintf("%s,","1");
            else
                abuffer1.sprintf("%s,","0");

            bBin1+= abuffer1;
        }
        WriteIniData(szDir, "AutoRetest", "bPassBin", bBin);
        WriteIniData(szDir, "AutoRetest", "bOpenShortBin", bBin1);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150825 KYEC 修改成By File
    {
        WriteIniData(szDir, "AutoRetest", "ArtAutoRetestLimitFile",             EL<TEdit>("TfYieldMonitoring", "edAutoRetestLimitFile")->Text);           //RT 次數
        WriteIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFile",           EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFile")->Text);         //wei 20160203

        WriteIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTFTFile",      EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTFTFile")->Text);    //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTFTFile",      EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTFTFile")->Text);    //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFTFile",         EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTFTFile")->Text);       //wei 20160203

        WriteIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTRTFile",         EL<TEdit>("TfYieldMonitoring", "edFailYieldRate_ARTRTFile")->Text);       //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTRTFile",      EL<TEdit>("TfYieldMonitoring", "edFailYieldMaxRate_ARTRTFile")->Text);    //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTRTFile",      EL<TEdit>("TfYieldMonitoring", "edFailYieldMinRate_ARTRTFile")->Text);    //wei 20160203

        WriteIniData(szDir, "AutoRetest", "AutoLeastRetestLimitFile",           EL<TEdit>("TfYieldMonitoring", "edAutoLeastRetestLimitFile")->Text);      //wei 20160203

        WriteIniData(szDir, "AutoRetest", "bAutoLeastRetestFile",               EL<TCheckBox>("TfYieldMonitoring", "ckUseLeastRetestTimes")->Checked);        //wei 20160203
        WriteIniData(szDir, "AutoRetest", "bUseFailNoDistinction",              EL<TCheckBox>("TfYieldMonitoring", "ckUseFailNoDistinction")->Checked);       //wei 20160203

        WriteIniData(szDir, "AutoRetest", "ArtUseFTFailYield",                  EL<TRadioGroup>("TfYieldMonitoring", "rgFT_ART")->ItemIndex);                   //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtUseRTFailYield",                  EL<TRadioGroup>("TfYieldMonitoring", "rgRT_ART")->ItemIndex);                   //wei 20160203

        WriteIniData(szDir, "AutoRetest", "ArtUseFTFailYieldModel",             EL<TComboBox>("TfYieldMonitoring", "cbUseFTFailYieldModel")->ItemIndex);      //wei 20160203
        WriteIniData(szDir, "AutoRetest", "ArtUseRTFailYieldModel",             EL<TComboBox>("TfYieldMonitoring", "cbUseRTFailYieldModel")->ItemIndex);      //wei 20160203
    }

    //wei 20160314
    if(CosFunction.bYieldAlarm4)                                                //Steven 20230223 : 隱藏沒用到的
    {
        WriteIniData(szDir, "Alarm4", "bAlarm4ContinueType_Enable",             EL<TCheckBox>("TfYieldMonitoring", "cbAlarm4ContinueType")->Checked);
        WriteIniData(szDir, "Alarm4", "iAlarm4ContinueType_IntervalCount",      EL<TEdit>("TfYieldMonitoring", "edAlarm4IntervalCount")->Text);
        WriteIniData(szDir, "Alarm4", "iAlarm4ContinueType_ContinueCount",      EL<TEdit>("TfYieldMonitoring", "edAlarm4ContinueCount")->Text);

        WriteIniData(szDir, "Alarm4", "bAlarm4EnableIntervalYield",             EL<TCheckBox>("TfYieldMonitoring", "cb_Alarm4EnableIntervalYield")->Checked);
        WriteIniData(szDir, "Alarm4", "iAlarm4IntervalYieldIntervalCount",      EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldIntervalCount")->Text);
        WriteIniData(szDir, "Alarm4", "iAlarm4IntervalYieldContinueCount",      EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldContinueCount")->Text);
        WriteIniData(szDir, "Alarm4", "iAlarm4IntervalYieldYield",              EL<TEdit>("TfYieldMonitoring", "ed_Alarm4IntervalYieldYield")->Text);

        WriteIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Enable",              EL<TCheckBox>("TfYieldMonitoring", "cb_SiteToSiteYieldEnable")->Checked);
        WriteIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Yield",               EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYield")->Text);
        WriteIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_IntervalCount",       EL<TEdit>("TfYieldMonitoring", "ed_SiteToSiteYieldCount")->Text);

        WriteIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Enable",              EL<TCheckBox>("TfYieldMonitoring", "cb_HeadToHeadYieldEnable")->Checked);
        WriteIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Yield",               EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYield")->Text);
        WriteIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_IntervalCount",       EL<TEdit>("TfYieldMonitoring", "ed_HeadToHeadYieldCount")->Text);

        WriteIniData(szDir, "Alarm4", "SiteYieldOverAlert_Enable",              EL<TCheckBox>("TfYieldMonitoring", "cb_SiteYieldOverAlert")->Checked);
        WriteIniData(szDir, "Alarm4", "SiteYieldOverAlert_Yield",               EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlert")->Text);
        WriteIniData(szDir, "Alarm4", "SiteYieldOverAlert_IntervalCount",       EL<TEdit>("TfYieldMonitoring", "ed_SiteYieldOverAlertCount")->Text);
    }

    //Steven 20231017 : change position
    WriteIniData(szDir, "Alarm4", "AllSiteFail",                                EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail")->Checked);                //kevin 20170825 (wei) 整支ARM Fail bin
    WriteIniData(szDir, "Alarm4", "AllSiteFailCount",                           EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCount")->Text);              //kevin 20180720 (wei) all site fail count
    WriteIniData(szDir, "Alarm4", "AllSiteFailCount_RT",                        EL<TEdit>("TfYieldMonitoring", "edAllSiteFailCountRT")->Text);            //Steven 20230118 : All site fail RT
    WriteIniData(szDir, "Alarm4", "AllSiteFail_RT",                             EL<TCheckBox>("TfYieldMonitoring", "cbAllSiteFail_RT")->Checked);             //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開

    //Sam 20171213 (Steven) : 超豐良率監控
    //==>
    if(CosFunction.bYieldAlarm5==true)                                          //Sam 20180423 (wei) : MOFile of Yeild Download
    {
        WriteIniData(szDir, "Alarm5", "iAlarm5_BySiteIntervalContactCnt",       EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteIntervalContactCnt")->Text);
        WriteIniData(szDir, "Alarm5", "bAlarm5_BySiteLowYieldEnable",           EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteLowYieldEnable")->Checked);                                        //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYield",                 EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYield")->Text);         //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYieldRej",              EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteLowYieldRej")->Text);      //Sam 20180423 (wei) : MOFile of Yeild Download
        WriteIniData(szDir, "Alarm5", "bAlarm5_BySiteCmpYieldEnable",           EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteCmpYieldEnable")->Checked);                                        //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYield",                 EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYield")->Text);         //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYieldRej",              EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteCmpYieldRej")->Text);      //Sam 20180423 (wei) : MOFile of Yeild Download
        WriteIniData(szDir, "Alarm5", "bAlarm5_BySiteAlarmYieldEnable",         EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySiteAlarmYieldEnable")->Checked);                                      //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYield",               EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYield")->Text);       //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYieldRej",            EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySiteAlarmYieldRej")->Text);    //Sam 20180423 (wei) : MOFile of Yeild Download
        WriteIniData(szDir, "Alarm5", "iAlarm5_OSBin",                          EL<TEdit>("TfYieldMonitoring", "edAlarm5_OSBin")->Text);                  //Sam 20171213 : 超豐良率監控
        WriteIniData(szDir, "Alarm5", "bAlarm5_BySitePreCmpYieldEnable",        EL<TCheckBox>("TfYieldMonitoring", "cbAlarm5_BySitePreCmpYieldEnable")->Checked);                                     //Sam 20180423 (wei) : MOFile of Yeild Download
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYield",              EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYield")->Text);      //Sam 20180423 (wei) : MOFile of Yeild Download
        WriteIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYieldRej",           EL<TEdit>("TfYieldMonitoring", "edAlarm5_BySitePreCmpYieldRej")->Text);   //Sam 20180423 (wei) : MOFile of Yeild Download
    }

    EL<TEdit>("TfYieldMonitoring", "eCloseSite")->Text = IntToStr(TestIF_File.iLowYieldLimit_AutoClean*2);        //kevin 20150306
    if(TestIF_File.bFailAlarmLowYield_AutoClean==true) filerw::ELTodo("golden uYieldMonitoring.cpp:2365-2379 bStandardYield reset (loop over TestSocket.iShtRow/iShtCol) not run -- TestSocket not visible in this TU, FileRW/_KitSuck.cpp adapter missing");   // golden uYieldMonitoring.cpp:2365-2379 取代：Auto Clean 低良率基準重置：迴圈上限 TestSocket.iShtRow／iShtCol —— TestSocket 在 mykitsuck.h／aHotPlateSubstrate.h（與 HTEditList.h 衝突），本 TU 看不到，FileRW/_KitSuck.cpp 目前沒有轉接函式（需要 FileRW_TestSocketShtRow/Col，見交付報告）
#if 0 // GATE (S12-C save) golden uYieldMonitoring.cpp:2365-2379 -- Auto Clean 低良率基準重置：迴圈上限 TestSocket.iShtRow／iShtCol —— TestSocket 在 mykitsuck.h／aHotPlateSubstrate.h（與 HTEditList.h 衝突），本 TU 看不到，FileRW/_KitSuck.cpp 目前沒有轉接函式（需要 FileRW_TestSocketShtRow/Col，見交付報告）
    bool doAutoYield=false;
    if(TestIF_File.bFailAlarmLowYield_AutoClean==true)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(bLowYieldCloseSite[0][i][j])
                    doAutoYield=true;                                           //kevin 20150306 儲存時尚未做過AUTO YIELD 將首次記錄更新
            }
        }

        if(doAutoYield==false)
            bStandardYield=false;                                               //kevin 20150306 儲存時將
    }
#endif // GATE (S12-C save)
    //Site Yield Alarm(%)

    if(CosFunction.bAdaptiveYield)                                              //Sam 20230914 : 自適應性良率監控
    {
        WriteIniData(szDir, "AdaptiveYield",     "Low Yield Alarm",           TestIF_File.bAdaptiveLowYield);
        WriteIniData(szDir, "AdaptiveYield",     "Consecutive Lower Alarm",     TestIF_File.iAdaptiveContsLowerAlarmNor);                                       //Sam 20240726 : AI Clean
        WriteIniData(szDir, "AdaptiveYield",     "Consecutive Lower Alarm Min", TestIF_File.iAdaptiveContsLowerAlarmMin);
        WriteIniData(szDir, "AdaptiveYield",     "Yield Max",                   TestIF_File.iAdaptiveYieldMax);
        WriteIniData(szDir, "AdaptiveYield",     "Yield Min",                   TestIF_File.iAdaptiveYieldMin);

        WriteIniData(szDir, "AdaptiveYield",     "Low Yield Alarm RT",        TestIF_File.bAdaptiveLowYield_RT);
        WriteIniData(szDir, "AdaptiveYield",     "Consecutive Lower Alarm RT",  TestIF_File.iAdaptiveContsLowerAlarmNor_RT);                                    //Sam 20240726 : AI Clean
        WriteIniData(szDir, "AdaptiveYield",     "Consecutive Lower Alarm Min RT",TestIF_File.iAdaptiveContsLowerAlarmMin_RT);
        WriteIniData(szDir, "AdaptiveYield",     "Yield Max RT",                TestIF_File.iAdaptiveYieldMax_RT);
        WriteIniData(szDir, "AdaptiveYield",     "Yield Min RT",                TestIF_File.iAdaptiveYieldMin_RT);
    }

    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
        AnsiString asSpecBinBySiteCompare;
        for(int i=0; i<iTestBinCount; i++)
        {
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bSpecBinBySiteCompareEnable[FT][i]);
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bSpecBinBySiteCompareEnable[RT][i]);

            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dSpecBinBySiteComparePercent[FT][i]));
            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dSpecBinBySiteComparePercent[RT][i]));

            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]);
            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]);

            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]));
            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]));

            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bByBinFailureEnable[FT][i]);
            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bByBinFailureEnable[RT][i]);

            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_FT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dByBinFailurePercent[FT][i]));
            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_RT", i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           double(TestIF_File.dByBinFailurePercent[RT][i]));

            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_FT",i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bFailCountEnable[FT][i]);
            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_RT",i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           TestIF_File.bFailCountEnable[RT][i]);

            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_FT",i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           int(TestIF_File.iFailCountLimit[FT][i]));
            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_RT",i);
            WriteIniData(szDir, "Alarm",     asSpecBinBySiteCompare,           int(TestIF_File.iFailCountLimit[RT][i]));
        }
        WriteIniData(szDir, "Alarm",     "iSpecBinBySiteCompareIgnore_FT",       int(TestIF_File.iSpecBinBySiteCompareIgnore[FT]));
        WriteIniData(szDir, "Alarm",     "iSpecBinBySiteCompareIgnore_RT",       int(TestIF_File.iSpecBinBySiteCompareIgnore[RT]));
        WriteIniData(szDir, "Alarm",     "iSpecBinByArmPerSiteCompareIgnore_FT", int(TestIF_File.iSpecBinByArmPerSiteCompareIgnore[FT]));
        WriteIniData(szDir, "Alarm",     "iSpecBinByArmPerSiteCompareIgnore_RT", int(TestIF_File.iSpecBinByArmPerSiteCompareIgnore[RT]));
        WriteIniData(szDir, "Alarm",     "iByBinFailureIgnore_FT", int(TestIF_File.iByBinFailureIgnore[FT]));
        WriteIniData(szDir, "Alarm",     "iByBinFailureIgnore_RT", int(TestIF_File.iByBinFailureIgnore[RT]));
        WriteIniData(szDir, "Alarm",     "iFailCountIgnore_FT",    int(TestIF_File.iFailCountIgnore[FT]));              //JerryYang 20231206
        WriteIniData(szDir, "Alarm",     "iFailCountIgnore_RT",    int(TestIF_File.iFailCountIgnore[RT]));
    }

    WriteIniData(szDir, "Alarm",     "LoadCellMeasure",       EL<TCheckBox>("TfYieldMonitoring", "cbLoadCellMeasure")->Checked);                              //kevin 20190907 Arm 測區次數道量測 功能;
    WriteIniData(szDir, "Alarm",     "LoadCellMeasureCount",  EL<TEdit>("TfYieldMonitoring", "edLoadCellMeasure")->Text);                                 //kevin 20190907 Arm 測區次數道量測

    if(CosFunction.IntervalYieldCount==true)                                    //Steven 20230223 : 隱藏沒用到的
    {
        //Interval Low Yield Alarm(By Site)(%)                                  //wei 20180606 Interval Low Yield By Site
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Site Enable",           int(TestIF_File.bFailAlarmIntervalLowYieldBySite));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Site Count",            int(TestIF_File.iIntervalLowYieldCountBySite));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Site Enable RT",        int(TestIF_File.bFailAlarmIntervalLowYieldBySite_RT));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Site Count RT",         int(TestIF_File.iIntervalLowYieldCountBySite_RT));
        if(CosFunction.bYieldAlarmUseDouble)
        {
            WriteIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit",     TestIF_File.dIntervalLowYieldLimitBySite);
            WriteIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit RT",  TestIF_File.dIntervalLowYieldLimitBySite_RT);
        }
        else
        {
            WriteIniData(szDir, "Low Yield Alarm", "Interval By Site Limit",            TestIF_File.iIntervalLowYieldLimitBySite);
            WriteIniData(szDir, "Low Yield Alarm", "Interval By Site Limit RT",         TestIF_File.iIntervalLowYieldLimitBySite_RT);
        }
        //Interval Low Yield Alarm(By Total)(%)                                 //wei 20180718 Interval Low Yield By Total
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Total Enable",          int(TestIF_File.bFailAlarmIntervalLowYieldByTotal));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Total Count",           int(TestIF_File.iIntervalLowYieldCountByTotal));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Total Enable RT",       int(TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT));
        WriteIniData(szDir, "Low Yield Alarm",     "Interval By Total Count RT",        int(TestIF_File.iIntervalLowYieldCountByTotal_RT));
        if(CosFunction.bYieldAlarmUseDouble)
        {
            WriteIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit",    TestIF_File.dIntervalLowYieldLimitByTotal);
            WriteIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit RT", TestIF_File.dIntervalLowYieldLimitByTotal_RT);
        }
        else
        {
            WriteIniData(szDir, "Low Yield Alarm", "Interval By Total Limit",           TestIF_File.iIntervalLowYieldLimitByTotal);
            WriteIniData(szDir, "Low Yield Alarm", "Interval By Total Limit RT",        TestIF_File.iIntervalLowYieldLimitByTotal_RT);
        }
    }

    if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;
    {
        WriteIniData(szDir, "Alarm",     "bCreateManualEOCAP",       int(TestIF_File.bCreateManualEOCAP));
    }

}

