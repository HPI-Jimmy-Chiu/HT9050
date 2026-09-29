// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 ArmSpeed_File 的讀寫檔（HTEditList： → ArmCondition.Data）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cSpeed.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 277 個（header 宣告 442 個）。
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
#include "forms/fObserver.h"
#include "csystem.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "FileRW/_EditPage.h"   // AI(W906-FRW-S157) 20260927 [W906]：filerw::PageEvent（檔尾 kSP_Events）

bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
extern bool authMainForm[12];        // cAuthority.h:56（golden cAuthority.h）
extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）

// golden cSpeed.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static AnsiString LastFileName;   // golden cSpeed.h: TfSpeed::LastFileName
static bool fShow=false;          // golden cSpeed.h: TfSpeed::fShow

using filerw::EL;

// 前置宣告
static void SP_TfSpeed();
static void SP_FormShow();
static void SP_ReadFile();
static void SP_ReadWriteFile(bool bRead, AnsiString szDir);
static void SP_DoIniDataToForm();
static void SP_tbAllSpeedChange();
static void SP_tbAccSpeedChange();
static void SP_tbEPControlChange();
static void SP_FormClose();
static void SP_spbSaveClick();
static void SP_spbSpeedAddClick();
static void SP_spbSpeedDecClick();
static void SP_cbIndexArmClick();
static void SP_spbSelectAllClick();
static void SP_spbSetToDefClick();

// golden cSpeed.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void SP_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfSpeed", "cobOutPickErrAct")->Items;
      s->Add("Auto 1");
      s->Add("Auto 2");
      s->Add("Auto 3");
      s->Add("Fix 1");
      s->Add("Fix 2");
      s->Add("Fix 3");
      s->Add("Error Bin");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgInArmDieClean")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->Items;
      s->Add("Open/Close");
      s->Add("Fixed");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->Items;
      s->Add("Together");
      s->Add("X Step by Step");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgIndexCycleTime")->Items;
      s->Add("Alarm and Running");
      s->Add("Pause and Alarm");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgOutArmDieClean")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->Items;
      s->Add("RETRY/SKIP");
      s->Add("RETRY");
      s->Add("SKIP");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->Items;
      s->Add("Open/Close");
      s->Add("Fixed");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgReleaseDelayTime")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->Items;
      s->Add("OFF");
      s->Add("Shake");
      s->Add("Vibration");
      s->Add("Shake and Vibration");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgTT_SetSpeed")->Items;
      s->Add("ON");
      s->Add("OFF");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_In")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_Out")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
}

// golden cSpeed.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kSP_ParentOf[][2] = {
    {"Edt_CheckTime_In", "gbInDestoryCheck"},
    {"Edt_CheckTime_Out", "gbOutDestoryCheck"},
    {"Edt_HeightCheck_In", "gbInDestoryCheck"},
    {"GroupBox1", "gbOutArmWaitTime"},
    {"GroupBox2", "tsMagazine"},
    {"GroupBox3", "tsMagazine"},
    {"GroupBox4", "palInArmLeft"},
    {"Panel2", "gbInArmWaitTime"},
    {"Panel3", "tsLoader"},
    {"Panel4", "tsAllSpeed"},
    {"Panel5", "tsLoader"},
    {"Panel6", "GroupBox4"},
    {"cbEnableDelayTimeZero", "gbEnableDelayTimeZero"},
    {"cbInArm", "gbUnitForChange"},
    {"cbInArmZ", "gbUnitForChange"},
    {"cbInRotate", "gbUnitForChange"},
    {"cbInVacuumCheckOptimize", "gbInArmVacuumCheck"},
    {"cbIndexArm", "gbUnitForChange"},
    {"cbIndexVacuumCheckOptimize", "gbIndexVacuumCheck"},
    {"cbOutArm", "gbUnitForChange"},
    {"cbOutArmZ", "gbUnitForChange"},
    {"cbOutRotate", "gbUnitForChange"},
    {"cbOutVacuumCheckOptimize", "gbOutArmVacuumCheck"},
    {"cbShuttle", "gbUnitForChange"},
    {"cbTrayArm", "gbUnitForChange"},
    {"chkEnableIndexCycleTimeMonitoring", "grpIndexCycleTimeMonitoring"},
    {"chkInArmPreSuck", "gbInVacuumOnFunc"},
    {"chkIndexPreSuck", "grpIndexPreSuck"},
    {"chkOutArmPreSuck", "gbOutVacuumOnFunc"},
    {"chkPickAfterOutArm", "gbIndexWaitTime"},
    {"chkTwoSpeedOnlyLoader", "gbTwoSpeed_In"},
    {"chkUseHPVacuumDelay", "grpHPVacuumDelay"},
    {"ckb_HeightCheck_In", "gbInDestoryCheck"},
    {"ckb_PauseCheck_In", "gbInDestoryCheck"},
    {"ckb_PauseCheck_Out", "gbOutDestoryCheck"},
    {"edAllAccSpeed", "gbAcc"},
    {"edAllSpeed", "gpSpeed"},
    {"edAutoSpeedLow", "Panel4"},
    {"edEPControl", "gbEPControl"},
    {"edInArmAirOnTime", "gbInArmAirOnTime"},
    {"edInArmCylinderDelay", "gbInArmCylinderDelay"},
    {"edInArmDestroyAgainCount", "gbInArmDestoryTime"},
    {"edInArmDestroyAgainTime", "gbInArmDestoryTime"},
    {"edInArmDieCleanDelay", "gbInArmDieClean"},
    {"edInArmDieCleanHeight", "gbInArmDieClean"},
    {"edInArmRetryCount", "gbInArmRetry"},
    {"edInArmRetryMM", "gbInArmRetry"},
    {"edInArmShtWaitTime", "gbInArmWaitOnSht"},
    {"edInPitchAcc", "gbInArmSpeed"},
    {"edInPitchSpd", "gbInArmSpeed"},
    {"edInRotAcc", "palInRotate"},
    {"edInRotSpd", "palInRotate"},
    {"edInVacumCheckTime", "gbInArmVacuumCheck"},
    {"edInXYAcc", "gbInArmSpeed"},
    {"edInXYSpd", "gbInArmSpeed"},
    {"edInZAcc", "gbInArmSpeed"},
    {"edInZSpd", "gbInArmSpeed"},
    {"edIndexAccDec", "gbIndexArmSpeed"},
    {"edIndexAirOnTime", "gbIndexAirOnTime"},
    {"edIndexArmRetryMM", "gbIndexRetry"},
    {"edIndexDestroyAgainCount", "gbIndexDestoryTime"},
    {"edIndexDestroyAgainTime", "gbIndexDestoryTime"},
    {"edIndexRetryCount", "gbIndexRetry"},
    {"edIndexSpeed", "gbIndexArmSpeed"},
    {"edIndexVacumCheckTime", "gbIndexVacuumCheck"},
    {"edMagCatchYAcc", "GroupBox2"},
    {"edMagCatchYSpd", "GroupBox2"},
    {"edMagZAcc", "GroupBox2"},
    {"edMagZSpd", "GroupBox2"},
    {"edOutArmAirOnTime", "gbOutArmAirOnTime"},
    {"edOutArmCylinderDelay", "gbOutArmCylinderDelay"},
    {"edOutArmDestroyAgainCount", "gbOutArmDestoryTime"},
    {"edOutArmDestroyAgainTime", "gbOutArmDestoryTime"},
    {"edOutArmDieCleanDelay", "gbOutArmDieClean"},
    {"edOutArmRetryCount", "gbOutArmRetry"},
    {"edOutArmRetryMM", "gbOutArmRetry"},
    {"edOutArmShtWaitTime", "GroupBox1"},
    {"edOutPitchAcc", "gbOutArmSpeed"},
    {"edOutPitchSpd", "gbOutArmSpeed"},
    {"edOutRotAcc", "palOutRotate"},
    {"edOutRotSpd", "palOutRotate"},
    {"edOutVacumCheckTime", "gbOutArmVacuumCheck"},
    {"edOutXAcc", "gbOutArmSpeed"},
    {"edOutXSpd", "gbOutArmSpeed"},
    {"edOutZAcc", "gbOutArmSpeed"},
    {"edOutZSpd", "gbOutArmSpeed"},
    {"edPrecisorCloseSp", "Panel6"},
    {"edPrecisorOpenSp", "Panel6"},
    {"edRelaseDelay", "gbInArmReleaseDelay"},
    {"edSecondADCCatchY", "GroupBox3"},
    {"edSecondADCIn", "gbTwoSpeed_In"},
    {"edSecondADCOut", "gbTwoSpeed_Out"},
    {"edSecondSpeedCatchY", "GroupBox3"},
    {"edSecondSpeedIn", "gbTwoSpeed_In"},
    {"edSecondSpeedOut", "gbTwoSpeed_Out"},
    {"edShakeAccDec", "grpShake"},
    {"edShakeCycles", "grpShake"},
    {"edShakeDelay", "grpShake"},
    {"edShakeDistance", "grpShake"},
    {"edSht1Acc", "gbShuttleSpeed"},
    {"edSht1Spd", "gbShuttleSpeed"},
    {"edSht2Acc", "gbShuttleSpeed"},
    {"edSht2Spd", "gbShuttleSpeed"},
    {"edShtDeviceCheckTime", "gbShtDeviceCheckTime"},
    {"edTrayArmAirOnTime", "gbTrayArmAirOnTime"},
    {"edTrayArmHandDown", "gbTrayArmHandDown"},
    {"edTrayArmRetryCount", "gbTrayArmRetryCount"},
    {"edTrayVacumCheckTime", "gbTrayArmVacuumCheck"},
    {"edTrayXAcc", "gbTrayArmSpeed"},
    {"edTrayXSpd", "gbTrayArmSpeed"},
    {"edTwoSpeedDistanceCatchY", "GroupBox3"},
    {"edTwoSpeedDistance_In", "gbTwoSpeed_In"},
    {"edTwoSpeedDistance_Out", "gbTwoSpeed_Out"},
    {"edtAuto1Speed1", "pnlAuto1Y"},
    {"edtAuto1SpeedZ", "pnlAuto1Z"},
    {"edtAuto2Speed1", "pnlAuto2Y"},
    {"edtAuto2SpeedZ", "pnlAuto2Z"},
    {"edtAuto3Speed1", "pnlAuto3Y"},
    {"edtAuto3SpeedZ", "pnlAuto3Z"},
    {"edtAuto4Speed1", "pnlAuto4Y"},
    {"edtAuto4SpeedZ", "pnlAuto4Z"},
    {"edtAuto5Speed1", "pnlAuto5Y"},
    {"edtAuto5SpeedZ", "pnlAuto5Z"},
    {"edtAuto6Speed1", "pnlAuto6Y"},
    {"edtAuto6SpeedZ", "pnlAuto6Z"},
    {"edtAutoSkipCT", "gbAutoSkip"},
    {"edtColorSpeed1", "pnlColorY"},
    {"edtColorSpeedZ", "pnlColorZ"},
    {"edtEmptySpeed1", "pnEmptyY"},
    {"edtEmptySpeedZ", "pnEmptyZ"},
    {"edtHPVacuumDelay", "grpHPVacuumDelay"},
    {"edtIndexCycleTimetolerance", "grpIndexCycleTimeMonitoring"},
    {"edtLoaderSpeed1", "pnlLoaderY"},
    {"edtLoaderSpeed2", "Panel5"},
    {"edtLoaderSpeedZ", "pnlpnlLoaderZ"},
    {"edtMonitoringIndexCycletime", "grpIndexCycleTimeMonitoring"},
    {"edtMonitoringOutlier", "grpIndexCycleTimeMonitoring"},
    {"edtMonitoringWindow", "grpIndexCycleTimeMonitoring"},
    {"edtOutArmPreSuck", "gbOutVacuumOnFunc"},
    {"edtTryAcc", "grpIndexUPHTryRun"},
    {"edtTrySpeed", "grpIndexUPHTryRun"},
    {"gbAcc", "tsAllSpeed"},
    {"gbAutoSkip", "palInArmLeft"},
    {"gbEPControl", "tsAllSpeed"},
    {"gbEnableDelayTimeZero", "palIndexRight"},
    {"gbInArmAirOnTime", "Panel2"},
    {"gbInArmCylinderDelay", "palInArmLeft"},
    {"gbInArmDestoryTime", "palInArmRight"},
    {"gbInArmDieClean", "palInArmLeft"},
    {"gbInArmReleaseDelay", "palInArmLeft"},
    {"gbInArmRetry", "palInArmRight"},
    {"gbInArmSpeed", "palInArmLeft"},
    {"gbInArmVacuumCheck", "gbInArmWaitTime"},
    {"gbInArmWaitOnSht", "Panel2"},
    {"gbInArmWaitTime", "palInArmRight"},
    {"gbInDestoryCheck", "palInArmRight"},
    {"gbInVacuumOnFunc", "palInArmLeft"},
    {"gbInarmDeviveConfirm", "palInArmLeft"},
    {"gbIndexAirOnTime", "gbIndexWaitTime"},
    {"gbIndexArmSpeed", "palIndexLeft"},
    {"gbIndexDestoryTime", "palIndexRight"},
    {"gbIndexRetry", "palIndexRight"},
    {"gbIndexVacuumCheck", "gbIndexWaitTime"},
    {"gbIndexWaitTime", "palIndexRight"},
    {"gbIndexeviveConfirm", "palIndexRight"},
    {"gbOutArmAirOnTime", "gbOutArmWaitTime"},
    {"gbOutArmCylinderDelay", "palOutLeft"},
    {"gbOutArmDestoryTime", "palOutRight"},
    {"gbOutArmDieClean", "palOutLeft"},
    {"gbOutArmRetry", "palOutRight"},
    {"gbOutArmSpeed", "palOutLeft"},
    {"gbOutArmVacuumCheck", "gbOutArmWaitTime"},
    {"gbOutArmWaitTime", "palOutRight"},
    {"gbOutDestoryCheck", "palOutRight"},
    {"gbOutPickErrAct", "palOutLeft"},
    {"gbOutVacuumOnFunc", "palOutLeft"},
    {"gbOutarmDeviveConfirm", "palOutLeft"},
    {"gbShtDeviceCheckTime", "palShuttleRight"},
    {"gbShuttleSpeed", "palShuttleLeft"},
    {"gbTrayArmAirOnTime", "gbTrayArmWaitTime"},
    {"gbTrayArmHandDown", "gbTrayArmWaitTime"},
    {"gbTrayArmRetryCount", "palTrayArmRight"},
    {"gbTrayArmSpeed", "palTrayArmLeft"},
    {"gbTrayArmVacuumCheck", "gbTrayArmWaitTime"},
    {"gbTrayArmWaitTime", "palTrayArmRight"},
    {"gbTwoSpeed_In", "palInArmRight"},
    {"gbTwoSpeed_Out", "palOutRight"},
    {"gbUnitForChange", "tsAllSpeed"},
    {"gpSpeed", "tsAllSpeed"},
    {"grpAuto1", "Panel3"},
    {"grpAuto2", "Panel3"},
    {"grpAuto3", "Panel3"},
    {"grpAuto4", "Panel5"},
    {"grpAuto5", "Panel5"},
    {"grpAuto6", "Panel5"},
    {"grpColor", "Panel3"},
    {"grpEmpty", "Panel3"},
    {"grpHPVacuumDelay", "gbInArmWaitTime"},
    {"grpIndexCycleTimeMonitoring", "palIndexLeft"},
    {"grpIndexPreSuck", "palIndexLeft"},
    {"grpIndexUPHTryRun", "palIndexLeft"},
    {"grpLoader", "Panel3"},
    {"grpShake", "palShuttleLeft"},
    {"lblTrayStepSpeedByMachine", "Panel5"},
    {"palInArmLeft", "tsInArm"},
    {"palInArmRight", "tsInArm"},
    {"palInRotate", "gbInArmSpeed"},
    {"palIndexLeft", "tsIndex"},
    {"palIndexRight", "tsIndex"},
    {"palOutLeft", "tsOutArm"},
    {"palOutRight", "tsOutArm"},
    {"palOutRotate", "gbOutArmSpeed"},
    {"palShuttleLeft", "tsShuttle"},
    {"palShuttleRight", "tsShuttle"},
    {"palTrayArmLeft", "tsTrayArm"},
    {"palTrayArmRight", "tsTrayArm"},
    {"pnEmptyY", "grpEmpty"},
    {"pnEmptyZ", "grpEmpty"},
    {"pnlAuto1Y", "grpAuto1"},
    {"pnlAuto1Z", "grpAuto1"},
    {"pnlAuto2Y", "grpAuto2"},
    {"pnlAuto2Z", "grpAuto2"},
    {"pnlAuto3Y", "grpAuto3"},
    {"pnlAuto3Z", "grpAuto3"},
    {"pnlAuto4Y", "grpAuto4"},
    {"pnlAuto4Z", "grpAuto4"},
    {"pnlAuto5Y", "grpAuto5"},
    {"pnlAuto5Z", "grpAuto5"},
    {"pnlAuto6Y", "grpAuto6"},
    {"pnlAuto6Z", "grpAuto6"},
    {"pnlColorY", "grpColor"},
    {"pnlColorZ", "grpColor"},
    {"pnlLoaderY", "grpLoader"},
    {"pnlpnlLoaderZ", "grpLoader"},
    {"rbTemp", "Panel1"},
    {"rgAutoSpeed", "tsAllSpeed"},
    {"rgInArmAutoSpeed", "gbAutoSkip"},
    {"rgInArmDieClean", "gbInArmDieClean"},
    {"rgInArmPitch", "palInArmLeft"},
    {"rgInShtOrder", "palShuttleLeft"},
    {"rgIndexCycleTime", "grpIndexCycleTimeMonitoring"},
    {"rgOutArmDieClean", "gbOutArmDieClean"},
    {"rgOutArmPickupErrMode", "palOutLeft"},
    {"rgOutArmPitch", "palOutLeft"},
    {"rgReleaseDelayTime", "gbInArmReleaseDelay"},
    {"rgSearchInarmLastModeForYPitch", "palInArmRight"},
    {"rgSearchLastModeForYPitch", "palOutRight"},
    {"rgShakeShuttle", "palShuttleLeft"},
    {"rgSocketCheck", "palIndexRight"},
    {"rgStepShuttle", "palShuttleLeft"},
    {"rgTT_SetSpeed", "tsAllSpeed"},
    {"rgTwoSpeedOnOff_In", "gbTwoSpeed_In"},
    {"rgTwoSpeedOnOff_Out", "gbTwoSpeed_Out"},
    {"spbSave", "Panel1"},
    {"spbSelectAll", "gbUnitForChange"},
    {"spbSpeedAdd", "gbUnitForChange"},
    {"spbSpeedDec", "gbUnitForChange"},
    {"tbAccSpeed", "gbAcc"},
    {"tbAllSpeed", "gpSpeed"},
    {"tbEPControl", "gbEPControl"},
    {"tsAllSpeed", "PageControl1"},
    {"tsInArm", "PageControl1"},
    {"tsIndex", "PageControl1"},
    {"tsLoader", "PageControl1"},
    {"tsMagazine", "PageControl1"},
    {"tsOutArm", "PageControl1"},
    {"tsShuttle", "PageControl1"},
    {"tsTrayArm", "PageControl1"},
    {"udAuto1Speed1", "pnlAuto1Y"},
    {"udAuto1SpeedZ", "pnlAuto1Z"},
    {"udAuto2Speed1", "pnlAuto2Y"},
    {"udAuto2SpeedZ", "pnlAuto2Z"},
    {"udAuto3Speed1", "pnlAuto3Y"},
    {"udAuto3SpeedZ", "pnlAuto3Z"},
    {"udAuto4Speed1", "pnlAuto4Y"},
    {"udAuto4SpeedZ", "pnlAuto4Z"},
    {"udAuto5Speed1", "pnlAuto5Y"},
    {"udAuto5SpeedZ", "pnlAuto5Z"},
    {"udAuto6Speed1", "pnlAuto6Y"},
    {"udAuto6SpeedZ", "pnlAuto6Z"},
    {"udColorSpeed1", "pnlColorY"},
    {"udColorSpeedZ", "pnlColorZ"},
    {"udEmptySpeed1", "pnEmptyY"},
    {"udEmptySpeedZ", "pnEmptyZ"},
    {"udInPitchAcc", "gbInArmSpeed"},
    {"udInPitchSpd", "gbInArmSpeed"},
    {"udInRotAcc", "palInRotate"},
    {"udInRotSpd", "palInRotate"},
    {"udInXAcc", "gbInArmSpeed"},
    {"udInXSpd", "gbInArmSpeed"},
    {"udInZAcc", "gbInArmSpeed"},
    {"udInZSpd", "gbInArmSpeed"},
    {"udIndexAcc", "gbIndexArmSpeed"},
    {"udIndexSpd", "gbIndexArmSpeed"},
    {"udLoaderSpeed1", "pnlLoaderY"},
    {"udLoaderSpeedZ", "pnlpnlLoaderZ"},
    {"udMagCatchYAcc", "GroupBox2"},
    {"udMagCatchYSpd", "GroupBox2"},
    {"udMagZAcc", "GroupBox2"},
    {"udMagZSpd", "GroupBox2"},
    {"udOutPitchAcc", "gbOutArmSpeed"},
    {"udOutPitchSpd", "gbOutArmSpeed"},
    {"udOutRotAcc", "palOutRotate"},
    {"udOutRotSpd", "palOutRotate"},
    {"udOutXAcc", "gbOutArmSpeed"},
    {"udOutXSpd", "gbOutArmSpeed"},
    {"udOutZAcc", "gbOutArmSpeed"},
    {"udOutZSpd", "gbOutArmSpeed"},
    {"udPrecisorCloseSp", "Panel6"},
    {"udPrecisorOpenSp", "Panel6"},
    {"udSht1Acc", "gbShuttleSpeed"},
    {"udSht1Spd", "gbShuttleSpeed"},
    {"udSht2Acc", "gbShuttleSpeed"},
    {"udSht2Spd", "gbShuttleSpeed"},
    {"udTrayXAcc", "gbTrayArmSpeed"},
    {"udTrayXSpd", "gbTrayArmSpeed"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void SP_CreateContainerProxies()
{
    EL<TGroupBox>("TfSpeed", "GroupBox1");
    EL<TGroupBox>("TfSpeed", "GroupBox2");
    EL<TGroupBox>("TfSpeed", "GroupBox3");
    EL<TGroupBox>("TfSpeed", "GroupBox4");
    EL<TPageControl>("TfSpeed", "PageControl1");
    EL<TPanel>("TfSpeed", "Panel1");
    EL<TPanel>("TfSpeed", "Panel2");
    EL<TPanel>("TfSpeed", "Panel3");
    EL<TPanel>("TfSpeed", "Panel4");
    EL<TPanel>("TfSpeed", "Panel5");
    EL<TPanel>("TfSpeed", "Panel6");
    EL<TGroupBox>("TfSpeed", "gbAcc");
    EL<TGroupBox>("TfSpeed", "gbAutoSkip");
    EL<TGroupBox>("TfSpeed", "gbEPControl");
    EL<TGroupBox>("TfSpeed", "gbEnableDelayTimeZero");
    EL<TGroupBox>("TfSpeed", "gbInArmAirOnTime");
    EL<TGroupBox>("TfSpeed", "gbInArmCylinderDelay");
    EL<TGroupBox>("TfSpeed", "gbInArmDestoryTime");
    EL<TGroupBox>("TfSpeed", "gbInArmDieClean");
    EL<TGroupBox>("TfSpeed", "gbInArmReleaseDelay");
    EL<TGroupBox>("TfSpeed", "gbInArmRetry");
    EL<TGroupBox>("TfSpeed", "gbInArmSpeed");
    EL<TGroupBox>("TfSpeed", "gbInArmVacuumCheck");
    EL<TGroupBox>("TfSpeed", "gbInArmWaitOnSht");
    EL<TGroupBox>("TfSpeed", "gbInArmWaitTime");
    EL<TGroupBox>("TfSpeed", "gbInDestoryCheck");
    EL<TGroupBox>("TfSpeed", "gbInVacuumOnFunc");
    EL<TGroupBox>("TfSpeed", "gbIndexAirOnTime");
    EL<TGroupBox>("TfSpeed", "gbIndexArmSpeed");
    EL<TGroupBox>("TfSpeed", "gbIndexDestoryTime");
    EL<TGroupBox>("TfSpeed", "gbIndexRetry");
    EL<TGroupBox>("TfSpeed", "gbIndexVacuumCheck");
    EL<TGroupBox>("TfSpeed", "gbIndexWaitTime");
    EL<TGroupBox>("TfSpeed", "gbOutArmAirOnTime");
    EL<TGroupBox>("TfSpeed", "gbOutArmCylinderDelay");
    EL<TGroupBox>("TfSpeed", "gbOutArmDestoryTime");
    EL<TGroupBox>("TfSpeed", "gbOutArmDieClean");
    EL<TGroupBox>("TfSpeed", "gbOutArmRetry");
    EL<TGroupBox>("TfSpeed", "gbOutArmSpeed");
    EL<TGroupBox>("TfSpeed", "gbOutArmVacuumCheck");
    EL<TGroupBox>("TfSpeed", "gbOutArmWaitTime");
    EL<TGroupBox>("TfSpeed", "gbOutDestoryCheck");
    EL<TGroupBox>("TfSpeed", "gbOutVacuumOnFunc");
    EL<TGroupBox>("TfSpeed", "gbShtDeviceCheckTime");
    EL<TGroupBox>("TfSpeed", "gbShuttleSpeed");
    EL<TGroupBox>("TfSpeed", "gbTrayArmAirOnTime");
    EL<TGroupBox>("TfSpeed", "gbTrayArmHandDown");
    EL<TGroupBox>("TfSpeed", "gbTrayArmRetryCount");
    EL<TGroupBox>("TfSpeed", "gbTrayArmSpeed");
    EL<TGroupBox>("TfSpeed", "gbTrayArmVacuumCheck");
    EL<TGroupBox>("TfSpeed", "gbTrayArmWaitTime");
    EL<TGroupBox>("TfSpeed", "gbTwoSpeed_In");
    EL<TGroupBox>("TfSpeed", "gbTwoSpeed_Out");
    EL<TGroupBox>("TfSpeed", "gbUnitForChange");
    EL<TGroupBox>("TfSpeed", "gpSpeed");
    EL<TGroupBox>("TfSpeed", "grpAuto1");
    EL<TGroupBox>("TfSpeed", "grpAuto2");
    EL<TGroupBox>("TfSpeed", "grpAuto3");
    EL<TGroupBox>("TfSpeed", "grpAuto4");
    EL<TGroupBox>("TfSpeed", "grpAuto5");
    EL<TGroupBox>("TfSpeed", "grpAuto6");
    EL<TGroupBox>("TfSpeed", "grpColor");
    EL<TGroupBox>("TfSpeed", "grpEmpty");
    EL<TGroupBox>("TfSpeed", "grpHPVacuumDelay");
    EL<TGroupBox>("TfSpeed", "grpIndexCycleTimeMonitoring");
    EL<TGroupBox>("TfSpeed", "grpIndexPreSuck");
    EL<TGroupBox>("TfSpeed", "grpIndexUPHTryRun");
    EL<TGroupBox>("TfSpeed", "grpLoader");
    EL<TGroupBox>("TfSpeed", "grpShake");
    EL<TPanel>("TfSpeed", "palInArmLeft");
    EL<TPanel>("TfSpeed", "palInArmRight");
    EL<TPanel>("TfSpeed", "palInRotate");
    EL<TPanel>("TfSpeed", "palIndexLeft");
    EL<TPanel>("TfSpeed", "palIndexRight");
    EL<TPanel>("TfSpeed", "palOutLeft");
    EL<TPanel>("TfSpeed", "palOutRight");
    EL<TPanel>("TfSpeed", "palOutRotate");
    EL<TPanel>("TfSpeed", "palShuttleLeft");
    EL<TPanel>("TfSpeed", "palShuttleRight");
    EL<TPanel>("TfSpeed", "palTrayArmLeft");
    EL<TPanel>("TfSpeed", "palTrayArmRight");
    EL<TPanel>("TfSpeed", "pnEmptyY");
    EL<TPanel>("TfSpeed", "pnEmptyZ");
    EL<TPanel>("TfSpeed", "pnlAuto1Y");
    EL<TPanel>("TfSpeed", "pnlAuto1Z");
    EL<TPanel>("TfSpeed", "pnlAuto2Y");
    EL<TPanel>("TfSpeed", "pnlAuto2Z");
    EL<TPanel>("TfSpeed", "pnlAuto3Y");
    EL<TPanel>("TfSpeed", "pnlAuto3Z");
    EL<TPanel>("TfSpeed", "pnlAuto4Y");
    EL<TPanel>("TfSpeed", "pnlAuto4Z");
    EL<TPanel>("TfSpeed", "pnlAuto5Y");
    EL<TPanel>("TfSpeed", "pnlAuto5Z");
    EL<TPanel>("TfSpeed", "pnlAuto6Y");
    EL<TPanel>("TfSpeed", "pnlAuto6Z");
    EL<TPanel>("TfSpeed", "pnlColorY");
    EL<TPanel>("TfSpeed", "pnlColorZ");
    EL<TPanel>("TfSpeed", "pnlLoaderY");
    EL<TPanel>("TfSpeed", "pnlpnlLoaderZ");
    EL<TTabSheet>("TfSpeed", "tsAllSpeed");
    EL<TTabSheet>("TfSpeed", "tsInArm");
    EL<TTabSheet>("TfSpeed", "tsIndex");
    EL<TTabSheet>("TfSpeed", "tsLoader");
    EL<TTabSheet>("TfSpeed", "tsMagazine");
    EL<TTabSheet>("TfSpeed", "tsOutArm");
    EL<TTabSheet>("TfSpeed", "tsShuttle");
    EL<TTabSheet>("TfSpeed", "tsTrayArm");
    filerw::ELSetParents("TfSpeed", kSP_ParentOf, (int)(sizeof(kSP_ParentOf) / sizeof(kSP_ParentOf[0])));
}

// golden cSpeed.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void SP_DfmState()
{
    EL<TCheckBox>("TfSpeed", "cbInArmZ")->Visible = false;
    EL<TCheckBox>("TfSpeed", "cbInVacuumCheckOptimize")->Visible = false;
    EL<TCheckBox>("TfSpeed", "cbIndexVacuumCheckOptimize")->Visible = false;
    EL<TCheckBox>("TfSpeed", "cbOutArmZ")->Visible = false;
    EL<TCheckBox>("TfSpeed", "cbOutVacuumCheckOptimize")->Visible = false;
    EL<TEdit>("TfSpeed", "edAllAccSpeed")->Enabled = false;
    filerw::ELSetReadOnly("TfSpeed", "edAllAccSpeed");
    EL<TEdit>("TfSpeed", "edAllSpeed")->Enabled = false;
    filerw::ELSetReadOnly("TfSpeed", "edAllSpeed");
    EL<TEdit>("TfSpeed", "edEPControl")->Enabled = false;
    filerw::ELSetReadOnly("TfSpeed", "edEPControl");
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Enabled = false;
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Visible = false;
    EL<TEdit>("TfSpeed", "edIndexArmRetryMM")->Visible = false;
    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed2")->Visible = false;
    EL<TEdit>("TfSpeed", "edtTryAcc")->Enabled = false;
    EL<TEdit>("TfSpeed", "edtTryAcc")->Visible = false;
    EL<TGroupBox>("TfSpeed", "gbIndexRetry")->Visible = false;
    EL<TGroupBox>("TfSpeed", "gbOutPickErrAct")->Visible = false;
    EL<TGroupBox>("TfSpeed", "gbShtDeviceCheckTime")->Visible = false;
    EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->Visible = false;
    EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->Visible = false;
    EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->Visible = false;
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->Visible = false;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Enabled = false;
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Enabled = false;
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Visible = false;
    EL<TEdit>("TfSpeed", "edAutoSpeedLow")->Text = "100";
    EL<TEdit>("TfSpeed", "edInPitchAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edInPitchSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edInRotAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edInRotSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edInXYAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edInXYSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edInZAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edInZSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Text = "100";
    EL<TEdit>("TfSpeed", "edIndexSpeed")->Text = "1";
    EL<TEdit>("TfSpeed", "edMagCatchYAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edMagCatchYSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edMagZAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edMagZSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutArmAirOnTime")->Text = "0.3";
    EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutRotAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutRotSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutXAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutXSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutZAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edOutZSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edPrecisorCloseSp")->Text = "1";
    EL<TEdit>("TfSpeed", "edPrecisorOpenSp")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondADCCatchY")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondADCIn")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondADCOut")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondSpeedCatchY")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondSpeedIn")->Text = "1";
    EL<TEdit>("TfSpeed", "edSecondSpeedOut")->Text = "1";
    EL<TEdit>("TfSpeed", "edShakeAccDec")->Text = "1";
    EL<TEdit>("TfSpeed", "edSht1Acc")->Text = "1";
    EL<TEdit>("TfSpeed", "edSht1Spd")->Text = "1";
    EL<TEdit>("TfSpeed", "edSht2Acc")->Text = "1";
    EL<TEdit>("TfSpeed", "edSht2Spd")->Text = "1";
    EL<TEdit>("TfSpeed", "edTrayArmAirOnTime")->Text = "0.5";
    EL<TEdit>("TfSpeed", "edTrayArmHandDown")->Text = "0.5";
    EL<TEdit>("TfSpeed", "edTrayVacumCheckTime")->Text = "0.5";
    EL<TEdit>("TfSpeed", "edTrayXAcc")->Text = "1";
    EL<TEdit>("TfSpeed", "edTrayXSpd")->Text = "1";
    EL<TEdit>("TfSpeed", "edTwoSpeedDistanceCatchY")->Text = "3.0";
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_In")->Text = "3.0";
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_Out")->Text = "3";
    EL<TLabeledEdit>("TfSpeed", "edtAuto1Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto1SpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto2Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto2SpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto3Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto3SpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto4Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto4SpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto5Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto5SpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto6Speed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtAuto6SpeedZ")->Text = "1";
    EL<TEdit>("TfSpeed", "edtAutoSkipCT")->Text = "10";
    EL<TLabeledEdit>("TfSpeed", "edtColorSpeed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtColorSpeedZ")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtEmptySpeed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtEmptySpeedZ")->Text = "1";
    EL<TEdit>("TfSpeed", "edtIndexCycleTimetolerance")->Text = "0.00";
    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed1")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed2")->Text = "1";
    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeedZ")->Text = "1";
    EL<TEdit>("TfSpeed", "edtMonitoringIndexCycletime")->Text = "0.00";
    EL<TEdit>("TfSpeed", "edtMonitoringOutlier")->Text = "0.00";
    EL<TEdit>("TfSpeed", "edtMonitoringWindow")->Text = "0";
    EL<TEdit>("TfSpeed", "edtTryAcc")->Text = "5500000";
    EL<TEdit>("TfSpeed", "edtTrySpeed")->Text = "900000";
    EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "rgInArmDieClean")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgIndexCycleTime")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "rgOutArmDieClean")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->ItemIndex = 0;
    EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgReleaseDelayTime")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgTT_SetSpeed")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_In")->ItemIndex = 1;
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_Out")->ItemIndex = 1;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->OnChange = &SP_tbAccSpeedChange;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->OnChange = &SP_tbAllSpeedChange;
    EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->OnChange = &SP_tbEPControlChange;
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto1Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto1Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto1Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto1SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto1SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto1SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto2Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto2Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto2Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto2SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto2SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto2SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto3Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto3Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto3Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto3SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto3SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto3SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto4Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto4Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto4Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto4SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto4SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto4SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto5Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto5Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto5Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto5SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto5SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto5SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto6Speed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto6Speed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto6Speed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto6SpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udAuto6SpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtAuto6SpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtColorSpeed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtColorSpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtEmptySpeed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtEmptySpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchAcc")->Associate = EL<TEdit>("TfSpeed", "edInPitchAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchSpd")->Associate = EL<TEdit>("TfSpeed", "edInPitchSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->Associate = EL<TEdit>("TfSpeed", "edInRotAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->Associate = EL<TEdit>("TfSpeed", "edInRotSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->Associate = EL<TEdit>("TfSpeed", "edInXYAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->Associate = EL<TEdit>("TfSpeed", "edInXYSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udInZAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInZAcc")->Associate = EL<TEdit>("TfSpeed", "edInZAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Associate = EL<TEdit>("TfSpeed", "edInZSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->DfmInit(1, 100, 100);
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Associate = EL<TEdit>("TfSpeed", "edIndexAccDec");
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->Associate = EL<TEdit>("TfSpeed", "edIndexSpeed");
    EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeed1")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeed1")->Associate = EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed1");
    EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeedZ")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeedZ")->Associate = EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeedZ");
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYAcc")->Associate = EL<TEdit>("TfSpeed", "edMagCatchYAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYSpd")->Associate = EL<TEdit>("TfSpeed", "edMagCatchYSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udMagZAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udMagZAcc")->Associate = EL<TEdit>("TfSpeed", "edMagZAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udMagZSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udMagZSpd")->Associate = EL<TEdit>("TfSpeed", "edMagZSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchAcc")->Associate = EL<TEdit>("TfSpeed", "edOutPitchAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchSpd")->Associate = EL<TEdit>("TfSpeed", "edOutPitchSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->Associate = EL<TEdit>("TfSpeed", "edOutRotAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->Associate = EL<TEdit>("TfSpeed", "edOutRotSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXAcc")->Associate = EL<TEdit>("TfSpeed", "edOutXAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Associate = EL<TEdit>("TfSpeed", "edOutXSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->Associate = EL<TEdit>("TfSpeed", "edOutZAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->Associate = EL<TEdit>("TfSpeed", "edOutZSpd");
    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorCloseSp")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorCloseSp")->Associate = EL<TEdit>("TfSpeed", "edPrecisorCloseSp");
    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorOpenSp")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorOpenSp")->Associate = EL<TEdit>("TfSpeed", "edPrecisorOpenSp");
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->Associate = EL<TEdit>("TfSpeed", "edSht1Acc");
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->Associate = EL<TEdit>("TfSpeed", "edSht1Spd");
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->Associate = EL<TEdit>("TfSpeed", "edSht2Acc");
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->Associate = EL<TEdit>("TfSpeed", "edSht2Spd");
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->Associate = EL<TEdit>("TfSpeed", "edTrayXAcc");
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->DfmInit(1, 100, 1);
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->Associate = EL<TEdit>("TfSpeed", "edTrayXSpd");
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kSP_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "GroupBox4", "PageControl1", "Panel1",
    "Panel2", "Panel3", "Panel4", "Panel5", "Panel6", "cbEnableDelayTimeZero",
    "cbInArm", "cbInArmZ", "cbInRotate", "cbInVacuumCheckOptimize", "cbIndexArm", "cbIndexVacuumCheckOptimize",
    "cbOutArm", "cbOutArmZ", "cbOutRotate", "cbOutVacuumCheckOptimize", "cbShuttle", "cbTrayArm",
    "chkEnableIndexCycleTimeMonitoring", "chkInArmPreSuck", "chkIndexPreSuck", "chkOutArmPreSuck", "chkPickAfterOutArm", "chkTwoSpeedOnlyLoader",
    "chkUseHPVacuumDelay", "ckb_HeightCheck_In", "ckb_PauseCheck_In", "ckb_PauseCheck_Out", "gbAcc", "gbAutoSkip",
    "gbEPControl", "gbEnableDelayTimeZero", "gbInArmAirOnTime", "gbInArmCylinderDelay", "gbInArmDestoryTime", "gbInArmDieClean",
    "gbInArmReleaseDelay", "gbInArmRetry", "gbInArmSpeed", "gbInArmVacuumCheck", "gbInArmWaitOnSht", "gbInArmWaitTime",
    "gbInDestoryCheck", "gbInVacuumOnFunc", "gbInarmDeviveConfirm", "gbIndexAirOnTime", "gbIndexArmSpeed", "gbIndexDestoryTime",
    "gbIndexRetry", "gbIndexVacuumCheck", "gbIndexWaitTime", "gbIndexeviveConfirm", "gbOutArmAirOnTime", "gbOutArmCylinderDelay",
    "gbOutArmDestoryTime", "gbOutArmDieClean", "gbOutArmRetry", "gbOutArmSpeed", "gbOutArmVacuumCheck", "gbOutArmWaitTime",
    "gbOutDestoryCheck", "gbOutPickErrAct", "gbOutVacuumOnFunc", "gbOutarmDeviveConfirm", "gbShtDeviceCheckTime", "gbShuttleSpeed",
    "gbTrayArmAirOnTime", "gbTrayArmHandDown", "gbTrayArmRetryCount", "gbTrayArmSpeed", "gbTrayArmVacuumCheck", "gbTrayArmWaitTime",
    "gbTwoSpeed_In", "gbTwoSpeed_Out", "gbUnitForChange", "gpSpeed", "grpAuto1", "grpAuto2",
    "grpAuto3", "grpAuto4", "grpAuto5", "grpAuto6", "grpColor", "grpEmpty",
    "grpHPVacuumDelay", "grpIndexCycleTimeMonitoring", "grpIndexPreSuck", "grpIndexUPHTryRun", "grpLoader", "grpShake",
    "lblTrayStepSpeedByMachine", "palInArmLeft", "palInArmRight", "palInRotate", "palIndexLeft", "palIndexRight",
    "palOutLeft", "palOutRight", "palOutRotate", "palShuttleLeft", "palShuttleRight", "palTrayArmLeft",
    "palTrayArmRight", "pnEmptyY", "pnEmptyZ", "pnlAuto1Y", "pnlAuto1Z", "pnlAuto2Y",
    "pnlAuto2Z", "pnlAuto3Y", "pnlAuto3Z", "pnlAuto4Y", "pnlAuto4Z", "pnlAuto5Y",
    "pnlAuto5Z", "pnlAuto6Y", "pnlAuto6Z", "pnlColorY", "pnlColorZ", "pnlLoaderY",
    "pnlpnlLoaderZ", "rbTemp", "rgAutoSpeed", "rgInArmAutoSpeed", "rgInArmDieClean", "rgInArmPitch",
    "rgInShtOrder", "rgIndexCycleTime", "rgOutArmDieClean", "rgOutArmPickupErrMode", "rgOutArmPitch", "rgReleaseDelayTime",
    "rgSearchInarmLastModeForYPitch", "rgSearchLastModeForYPitch", "rgShakeShuttle", "rgSocketCheck", "rgStepShuttle", "rgTT_SetSpeed",
    "rgTwoSpeedOnOff_In", "rgTwoSpeedOnOff_Out", "spbSave", "spbSelectAll", "spbSpeedAdd", "spbSpeedDec",
    "tsAllSpeed", "tsInArm", "tsIndex", "tsLoader", "tsMagazine", "tsOutArm",
    "tsShuttle", "tsTrayArm",
};

// 存檔流程（spbSaveClick、ReadWriteFile）讀的替身 130 個 —— FileRW/ArmSpeed_File.cpp 用它決定頁面必須送哪些值
static const char* const kSP_SaveReads[] = {
    "Edt_CheckTime_In", "Edt_CheckTime_Out", "Edt_HeightCheck_In", "cbEnableDelayTimeZero", "cbInVacuumCheckOptimize", "cbIndexVacuumCheckOptimize",
    "cbOutVacuumCheckOptimize", "chkEnableIndexCycleTimeMonitoring", "chkInArmPreSuck", "chkIndexPreSuck", "chkOutArmPreSuck", "chkPickAfterOutArm",
    "chkTwoSpeedOnlyLoader", "chkUseHPVacuumDelay", "ckb_HeightCheck_In", "ckb_PauseCheck_In", "ckb_PauseCheck_Out", "edAllAccSpeed",
    "edAllSpeed", "edAutoSpeedLow", "edEPControl", "edInArmAirOnTime", "edInArmCylinderDelay", "edInArmDestroyAgainCount",
    "edInArmDestroyAgainTime", "edInArmDieCleanDelay", "edInArmDieCleanHeight", "edInArmRetryCount", "edInArmRetryMM", "edInArmShtWaitTime",
    "edInPitchAcc", "edInPitchSpd", "edInRotAcc", "edInRotSpd", "edInVacumCheckTime", "edInXYAcc",
    "edInXYSpd", "edInZAcc", "edInZSpd", "edIndexAccDec", "edIndexAirOnTime", "edIndexArmRetryMM",
    "edIndexDestroyAgainCount", "edIndexDestroyAgainTime", "edIndexRetryCount", "edIndexSpeed", "edIndexVacumCheckTime", "edMagCatchYAcc",
    "edMagCatchYSpd", "edMagZAcc", "edMagZSpd", "edOutArmAirOnTime", "edOutArmCylinderDelay", "edOutArmDestroyAgainCount",
    "edOutArmDestroyAgainTime", "edOutArmRetryCount", "edOutArmRetryMM", "edOutArmShtWaitTime", "edOutPitchAcc", "edOutPitchSpd",
    "edOutRotAcc", "edOutRotSpd", "edOutVacumCheckTime", "edOutXAcc", "edOutXSpd", "edOutZAcc",
    "edOutZSpd", "edPrecisorCloseSp", "edPrecisorOpenSp", "edRelaseDelay", "edSecondADCCatchY", "edSecondADCIn",
    "edSecondADCOut", "edSecondSpeedCatchY", "edSecondSpeedIn", "edSecondSpeedOut", "edShakeAccDec", "edShakeCycles",
    "edShakeDelay", "edShakeDistance", "edSht1Acc", "edSht1Spd", "edSht2Acc", "edSht2Spd",
    "edShtDeviceCheckTime", "edTrayArmAirOnTime", "edTrayArmHandDown", "edTrayArmRetryCount", "edTrayVacumCheckTime", "edTrayXAcc",
    "edTrayXSpd", "edTwoSpeedDistanceCatchY", "edTwoSpeedDistance_In", "edTwoSpeedDistance_Out", "edtAuto1SpeedZ", "edtAuto2SpeedZ",
    "edtAuto3SpeedZ", "edtAuto4SpeedZ", "edtAuto5SpeedZ", "edtAuto6SpeedZ", "edtAutoSkipCT", "edtColorSpeedZ",
    "edtEmptySpeedZ", "edtHPVacuumDelay", "edtIndexCycleTimetolerance", "edtLoaderSpeedZ", "edtMonitoringIndexCycletime", "edtMonitoringOutlier",
    "edtMonitoringWindow", "edtOutArmPreSuck", "gbInarmDeviveConfirm", "gbIndexeviveConfirm", "gbOutarmDeviveConfirm", "rgAutoSpeed",
    "rgInArmAutoSpeed", "rgInArmDieClean", "rgInArmPitch", "rgInShtOrder", "rgIndexCycleTime", "rgOutArmPickupErrMode",
    "rgOutArmPitch", "rgReleaseDelayTime", "rgSearchInarmLastModeForYPitch", "rgSearchLastModeForYPitch", "rgShakeShuttle", "rgSocketCheck",
    "rgStepShuttle", "rgTT_SetSpeed", "rgTwoSpeedOnOff_In", "rgTwoSpeedOnOff_Out",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void SP_CreateSaveProxies()
{
    EL<TEdit>("TfSpeed", "Edt_CheckTime_In");
    EL<TEdit>("TfSpeed", "Edt_CheckTime_Out");
    EL<TEdit>("TfSpeed", "Edt_HeightCheck_In");
    EL<TCheckBox>("TfSpeed", "cbEnableDelayTimeZero");
    EL<TCheckBox>("TfSpeed", "cbInVacuumCheckOptimize");
    EL<TCheckBox>("TfSpeed", "cbIndexVacuumCheckOptimize");
    EL<TCheckBox>("TfSpeed", "cbOutVacuumCheckOptimize");
    EL<TCheckBox>("TfSpeed", "chkEnableIndexCycleTimeMonitoring");
    EL<TCheckBox>("TfSpeed", "chkInArmPreSuck");
    EL<TCheckBox>("TfSpeed", "chkIndexPreSuck");
    EL<TCheckBox>("TfSpeed", "chkOutArmPreSuck");
    EL<TCheckBox>("TfSpeed", "chkPickAfterOutArm");
    EL<TCheckBox>("TfSpeed", "chkTwoSpeedOnlyLoader");
    EL<TCheckBox>("TfSpeed", "chkUseHPVacuumDelay");
    EL<TCheckBox>("TfSpeed", "ckb_HeightCheck_In");
    EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_In");
    EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_Out");
    EL<TEdit>("TfSpeed", "edAllAccSpeed");
    EL<TEdit>("TfSpeed", "edAllSpeed");
    EL<TEdit>("TfSpeed", "edAutoSpeedLow");
    EL<TEdit>("TfSpeed", "edEPControl");
    EL<TEdit>("TfSpeed", "edInArmAirOnTime");
    EL<TEdit>("TfSpeed", "edInArmCylinderDelay");
    EL<TEdit>("TfSpeed", "edInArmDestroyAgainCount");
    EL<TEdit>("TfSpeed", "edInArmDestroyAgainTime");
    EL<TEdit>("TfSpeed", "edInArmDieCleanDelay");
    EL<TEdit>("TfSpeed", "edInArmDieCleanHeight");
    EL<TEdit>("TfSpeed", "edInArmRetryCount");
    EL<TEdit>("TfSpeed", "edInArmRetryMM");
    EL<TEdit>("TfSpeed", "edInArmShtWaitTime");
    EL<TEdit>("TfSpeed", "edInPitchAcc");
    EL<TEdit>("TfSpeed", "edInPitchSpd");
    EL<TEdit>("TfSpeed", "edInRotAcc");
    EL<TEdit>("TfSpeed", "edInRotSpd");
    EL<TEdit>("TfSpeed", "edInVacumCheckTime");
    EL<TEdit>("TfSpeed", "edInXYAcc");
    EL<TEdit>("TfSpeed", "edInXYSpd");
    EL<TEdit>("TfSpeed", "edInZAcc");
    EL<TEdit>("TfSpeed", "edInZSpd");
    EL<TEdit>("TfSpeed", "edIndexAccDec");
    EL<TEdit>("TfSpeed", "edIndexAirOnTime");
    EL<TEdit>("TfSpeed", "edIndexArmRetryMM");
    EL<TEdit>("TfSpeed", "edIndexDestroyAgainCount");
    EL<TEdit>("TfSpeed", "edIndexDestroyAgainTime");
    EL<TEdit>("TfSpeed", "edIndexRetryCount");
    EL<TEdit>("TfSpeed", "edIndexSpeed");
    EL<TEdit>("TfSpeed", "edIndexVacumCheckTime");
    EL<TEdit>("TfSpeed", "edMagCatchYAcc");
    EL<TEdit>("TfSpeed", "edMagCatchYSpd");
    EL<TEdit>("TfSpeed", "edMagZAcc");
    EL<TEdit>("TfSpeed", "edMagZSpd");
    EL<TEdit>("TfSpeed", "edOutArmAirOnTime");
    EL<TEdit>("TfSpeed", "edOutArmCylinderDelay");
    EL<TEdit>("TfSpeed", "edOutArmDestroyAgainCount");
    EL<TEdit>("TfSpeed", "edOutArmDestroyAgainTime");
    EL<TEdit>("TfSpeed", "edOutArmRetryCount");
    EL<TEdit>("TfSpeed", "edOutArmRetryMM");
    EL<TEdit>("TfSpeed", "edOutArmShtWaitTime");
    EL<TEdit>("TfSpeed", "edOutPitchAcc");
    EL<TEdit>("TfSpeed", "edOutPitchSpd");
    EL<TEdit>("TfSpeed", "edOutRotAcc");
    EL<TEdit>("TfSpeed", "edOutRotSpd");
    EL<TEdit>("TfSpeed", "edOutVacumCheckTime");
    EL<TEdit>("TfSpeed", "edOutXAcc");
    EL<TEdit>("TfSpeed", "edOutXSpd");
    EL<TEdit>("TfSpeed", "edOutZAcc");
    EL<TEdit>("TfSpeed", "edOutZSpd");
    EL<TEdit>("TfSpeed", "edPrecisorCloseSp");
    EL<TEdit>("TfSpeed", "edPrecisorOpenSp");
    EL<TEdit>("TfSpeed", "edRelaseDelay");
    EL<TEdit>("TfSpeed", "edSecondADCCatchY");
    EL<TEdit>("TfSpeed", "edSecondADCIn");
    EL<TEdit>("TfSpeed", "edSecondADCOut");
    EL<TEdit>("TfSpeed", "edSecondSpeedCatchY");
    EL<TEdit>("TfSpeed", "edSecondSpeedIn");
    EL<TEdit>("TfSpeed", "edSecondSpeedOut");
    EL<TEdit>("TfSpeed", "edShakeAccDec");
    EL<TEdit>("TfSpeed", "edShakeCycles");
    EL<TEdit>("TfSpeed", "edShakeDelay");
    EL<TEdit>("TfSpeed", "edShakeDistance");
    EL<TEdit>("TfSpeed", "edSht1Acc");
    EL<TEdit>("TfSpeed", "edSht1Spd");
    EL<TEdit>("TfSpeed", "edSht2Acc");
    EL<TEdit>("TfSpeed", "edSht2Spd");
    EL<TEdit>("TfSpeed", "edShtDeviceCheckTime");
    EL<TEdit>("TfSpeed", "edTrayArmAirOnTime");
    EL<TEdit>("TfSpeed", "edTrayArmHandDown");
    EL<TEdit>("TfSpeed", "edTrayArmRetryCount");
    EL<TEdit>("TfSpeed", "edTrayVacumCheckTime");
    EL<TEdit>("TfSpeed", "edTrayXAcc");
    EL<TEdit>("TfSpeed", "edTrayXSpd");
    EL<TEdit>("TfSpeed", "edTwoSpeedDistanceCatchY");
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_In");
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_Out");
    EL<TLabeledEdit>("TfSpeed", "edtAuto1SpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtAuto2SpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtAuto3SpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtAuto4SpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtAuto5SpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtAuto6SpeedZ");
    EL<TEdit>("TfSpeed", "edtAutoSkipCT");
    EL<TLabeledEdit>("TfSpeed", "edtColorSpeedZ");
    EL<TLabeledEdit>("TfSpeed", "edtEmptySpeedZ");
    EL<TEdit>("TfSpeed", "edtHPVacuumDelay");
    EL<TEdit>("TfSpeed", "edtIndexCycleTimetolerance");
    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeedZ");
    EL<TEdit>("TfSpeed", "edtMonitoringIndexCycletime");
    EL<TEdit>("TfSpeed", "edtMonitoringOutlier");
    EL<TEdit>("TfSpeed", "edtMonitoringWindow");
    EL<TEdit>("TfSpeed", "edtOutArmPreSuck");
    EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm");
    EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm");
    EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm");
    EL<TRadioGroup>("TfSpeed", "rgAutoSpeed");
    EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed");
    EL<TRadioGroup>("TfSpeed", "rgInArmDieClean");
    EL<TRadioGroup>("TfSpeed", "rgInArmPitch");
    EL<TRadioGroup>("TfSpeed", "rgInShtOrder");
    EL<TRadioGroup>("TfSpeed", "rgIndexCycleTime");
    EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode");
    EL<TRadioGroup>("TfSpeed", "rgOutArmPitch");
    EL<TRadioGroup>("TfSpeed", "rgReleaseDelayTime");
    EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch");
    EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch");
    EL<TRadioGroup>("TfSpeed", "rgShakeShuttle");
    EL<TRadioGroup>("TfSpeed", "rgSocketCheck");
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle");
    EL<TRadioGroup>("TfSpeed", "rgTT_SetSpeed");
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_In");
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_Out");
}

// golden cSpeed.cpp:31  TfSpeed::TfSpeed(TComponent* Owner)
static void SP_TfSpeed()
{

    LastFileName="";
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Enabled=false;
    EL<TSpeedButton>("TfSpeed", "spbSpeedAdd")->Enabled=false;
    EL<TSpeedButton>("TfSpeed", "spbSpeedDec")->Enabled=false;
    ;   // golden cSpeed.cpp:38-38 取代：edtHPVacuumDelay->Hint：Default Recipe ChangeLog 用的提示字（HTML 不用）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:38-38 -- edtHPVacuumDelay->Hint：Default Recipe ChangeLog 用的提示字（HTML 不用）
    EL<TEdit>("TfSpeed", "edtHPVacuumDelay")->Hint="edtHPVacuumDelay";                                  //Sam 20201209 : Default Recipe ChangeLog
#endif // GATE (S12-C save)

}

// golden cSpeed.cpp:41  TfSpeed::FormShow(TObject *Sender)
static void SP_FormShow()
{

    AnsiString S;
    LastFileName=GetLastOpenFN();
    SP_ReadFile();
    SP_DoIniDataToForm();
    S.sprintf("Speed Condition  ''%s''  ",LastFileName);
    ;   // golden cSpeed.cpp:48-48 取代：Caption：視窗標題
#if 0 // GATE (S12-C save) golden cSpeed.cpp:48-48 -- Caption：視窗標題
    Caption=S;
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfSpeed", "PageControl1")->ActivePage=EL<TTabSheet>("TfSpeed", "tsAllSpeed");
    fShow=true;
    ;   // golden cSpeed.cpp:51-52 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden cSpeed.cpp:51-52 -- Left／Top：視窗位置
    Left=125;                                                                   //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    EL<TGroupBox>("TfSpeed", "gbIndexRetry")->Visible   =false;
    EL<TGroupBox>("TfSpeed", "gbAutoSkip")->Visible     =IniConfig.bRecordSkipPosition;                     //Steven 20140816 : rgInArmAutoSpeed --> gbAutoSkip
    EL<TEdit>("TfSpeed", "edIndexVacumCheckTime")->Enabled=true;                                        //JerryYang 20220311 : ATP鎖定Critical parameter
    EL<TEdit>("TfSpeed", "edIndexAirOnTime")->Enabled=true;
    EL<TEdit>("TfSpeed", "edIndexSpeed")->Enabled   =true;
    EL<TGroupBox>("TfSpeed", "gbInArmCylinderDelay")->Visible =(InOutArmPickerUseMotor!=eptUseMot);
    EL<TGroupBox>("TfSpeed", "gbOutArmCylinderDelay")->Visible=(InOutArmPickerUseMotor!=eptUseMot);

    EL<TEdit>("TfSpeed", "edIndexRetryCount")->Text =0;
    EL<TEdit>("TfSpeed", "edIndexArmRetryMM")->Text =0;
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Text     =100;                                               //Steven 20091214 Add for Index ACC & DEC must be 100%

    EL<TPanel>("TfSpeed", "palInRotate")->Visible    =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);      //Steven 20170425 (wei) : Add rotate motor
    EL<TPanel>("TfSpeed", "palOutRotate")->Visible   =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    EL<TCheckBox>("TfSpeed", "cbInRotate")->Visible     =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    EL<TCheckBox>("TfSpeed", "cbOutRotate")->Visible    =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    EL<TTabSheet>("TfSpeed", "tsMagazine")->Visible     =(AUTO3_IS_MAGAZINE==1);                            //JerryYang 20220909 : add magazine
    //jou 2010-05-19 start : 負壓
    if(INDEX_SUCKER_TYPE==0)
    {
        EL<TGroupBox>("TfSpeed", "grpIndexPreSuck")->Visible=true;
    }
    else
    {
        EL<TGroupBox>("TfSpeed", "grpIndexPreSuck")->Visible=false;
        EL<TCheckBox>("TfSpeed", "chkIndexPreSuck")->Checked=true;
    }
    //jou 2010-05-19 end

    EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->Visible    =CosFunction.bEnableAutoSpeed;                      //Steven 20110526
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->Visible  =IniConfig.bEnableStepShuttle;                      //jou 2013-07-16 Step Shuttle check Index -> Input
    EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->Visible =CosFunction.bShakeShuttleEveryTime;                //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
    EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->Visible   =(TestIF_File.iTestMode==QualSite1X4 ||             //Steven 20250630 : 力旺要照順序擺
                              TestIF_File.iTestMode>=_8Site2X4);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->Max       =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Max       =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInPitchSpd")->Max   =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchSpd")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->Max    =CosFunction.iLimitMaxSpeed;

        EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->Max       =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Max       =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInPitchAcc")->Max   =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchAcc")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->Max      =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->Max     =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->Max    =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->Max    =CosFunction.iLimitMaxSpeed;                        //Ifor 20150710 :限制最高速度
        EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeed1")->Max =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto1Speed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto2Speed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto3Speed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto4Speed1")->Max  =CosFunction.iLimitMaxSpeed;                        //Steven 20230907 : For HT-9011UC
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto5Speed1")->Max  =CosFunction.iLimitMaxSpeed;
        EL<filerw::ELTrackBar>("TfSpeed", "udAuto6Speed1")->Max  =CosFunction.iLimitMaxSpeed;
    }

    EL<TGroupBox>("TfSpeed", "grpHPVacuumDelay")->Visible=IniConfig.bE57HPCanUseAnotherVacuumDelay;         //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->Enabled   =true;
    EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->Enabled  =true;
    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->Enabled =false;
            EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->Enabled=false;
        }
    }

    if(IniConfig.bRecordSkipPosition)                                           //wei 20150320  Onecycle 後才能開關Auto Skip 避免吸嘴錯誤hang up
    {
    if(FileRW_InOutArmSuckHasIC() || ShuttleHasIC() || IndexHasIC())   // golden cSpeed.cpp:146-147 取代：InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:146-147 -- InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
        if(InArmSuck.HasIC() || OutArmSuck.HasIC() ||
           ShuttleHasIC()    || IndexHasIC())
#endif // GATE (S12-C save)
        {
            EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->Enabled=false;
        }
        else
        {
            EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->Enabled=true;
        }
    }
    EL<TGroupBox>("TfSpeed", "gbOutPickErrAct")->Visible=(CUSTOMER_CODE==CC_AMKOR_Japan);                   //Steven 20170202 : For ATJ Out Arm pickup error put to define tray

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20170303 (wei) ASE_CL 權限控制
    {
        EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->Enabled          =fSecurity->Insufficient(133, false);
        EL<TGroupBox>("TfSpeed", "gbIndexWaitTime")->Enabled      =fSecurity->Insufficient(134, false);
        EL<TGroupBox>("TfSpeed", "gbIndexDestoryTime")->Enabled   =fSecurity->Insufficient(135, false);
        //Input Arm
        EL<TGroupBox>("TfSpeed", "gbInVacuumOnFunc")->Enabled     =fSecurity->Insufficient(136, false);
        EL<TGroupBox>("TfSpeed", "gbInArmReleaseDelay")->Enabled  =fSecurity->Insufficient(137, false);
        EL<TGroupBox>("TfSpeed", "gbInArmRetry")->Enabled         =fSecurity->Insufficient(138, false);
        EL<TGroupBox>("TfSpeed", "gbInArmWaitTime")->Enabled      =fSecurity->Insufficient(138, false);
        EL<TGroupBox>("TfSpeed", "gbInArmDestoryTime")->Enabled   =fSecurity->Insufficient(139, false);
        EL<TGroupBox>("TfSpeed", "gbInDestoryCheck")->Enabled     =fSecurity->Insufficient(139, false);
        EL<TGroupBox>("TfSpeed", "gbTwoSpeed_In")->Enabled        =fSecurity->Insufficient(140, false);
        //Output Arm
        EL<TGroupBox>("TfSpeed", "gbOutVacuumOnFunc")->Enabled    =fSecurity->Insufficient(141, false);
        EL<TGroupBox>("TfSpeed", "gbOutArmRetry")->Enabled        =fSecurity->Insufficient(142, false);
        EL<TGroupBox>("TfSpeed", "gbOutArmWaitTime") ->Enabled    =fSecurity->Insufficient(142, false);
        EL<TGroupBox>("TfSpeed", "gbOutArmDestoryTime")->Enabled  =fSecurity->Insufficient(143, false);
        EL<TGroupBox>("TfSpeed", "gbOutDestoryCheck")->Enabled    =fSecurity->Insufficient(144, false);
        EL<TGroupBox>("TfSpeed", "gbTwoSpeed_Out")->Enabled       =fSecurity->Insufficient(145, false);
        EL<TGroupBox>("TfSpeed", "gbTrayArmWaitTime")->Enabled    =fSecurity->Insufficient(146, false);
    }
    else                                                                        //jou 981207 權限控制
    {
        EL<TTabSheet>("TfSpeed", "tsAllSpeed")->Enabled =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsInArm")->Enabled    =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsShuttle")->Enabled  =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsIndex")->Enabled    =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsOutArm")->Enabled   =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsTrayArm")->Enabled  =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        EL<TTabSheet>("TfSpeed", "tsLoader")->Enabled   =(fSecurity->Insufficient(3, false) && authMainForm[3]);
    }
    EL<TCheckBox>("TfSpeed", "ckb_HeightCheck_In")->Visible         =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //wei 20160825 回吸偵測
    EL<TEdit>("TfSpeed", "Edt_HeightCheck_In")->Visible         =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //wei 20160825 回吸偵測
    EL<TGroupBox>("TfSpeed", "gbInArmReleaseDelay")->Visible        =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //JerryYang 20170406 (Steven) 只有TSMC才使用input arm release IC前delay
    EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm")->Visible       =(CUSTOMER_CODE==CC_ASE_KaohSiung);
    EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm")->Visible      =(CUSTOMER_CODE==CC_ASE_KaohSiung);
    EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm")->Visible        =(CUSTOMER_CODE==CC_ASE_KaohSiung);     //kevin 20190629 add index 4 site conform
    EL<TGroupBox>("TfSpeed", "grpIndexCycleTimeMonitoring")->Visible=(CUSTOMER_CODE==CC_SCK ||              //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
                                          IniConfig.bSPILFunction==true);       //JerryYang 20220824 : SPIL index cycle time monitor

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : Index cycle time監控
    {
        EL<TCheckBox>("TfSpeed", "chkEnableIndexCycleTimeMonitoring")->Enabled=(AccessLevel>=iDefHonPrecLevel);
        EL<TEdit>("TfSpeed", "edtIndexCycleTimetolerance")->Enabled=(AccessLevel>=iDefHonPrecLevel);
    }
//    grpIndexCycleTimeMonitoring->Visible=(!IniConfig.bSPILFunction);          //JerryYang 20220824 : SPIL index cycle time monitor

    EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch")->Visible=(CosFunction.bYPitchNotUseSearchLastMode==true && USE_OUT_Y_IS_AUTO_PITCH==true);                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch")->Visible=(CosFunction.bYPitchNotUseSearchLastMode==true && USE_IN_Y_IS_AUTO_PITCH==true);                                    //kevin 20211130 for in arm Ypitch  //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
    ;   // golden cSpeed.cpp:208-208 取代：rbTemp->SetFocus()：焦點
#if 0 // GATE (S12-C save) golden cSpeed.cpp:208-208 -- rbTemp->SetFocus()：焦點
    EL<TRadioButton>("TfSpeed", "rbTemp")->SetFocus();
#endif // GATE (S12-C save)

    //Steven 20200529 : Loader入Tray改步進
    //==>
    EL<TTabSheet>("TfSpeed", "tsLoader")->TabVisible=(LoaderUnload_StepMotor ||
                          LOAD_Z_USE_MOTOR[0] || LOAD_Z_USE_MOTOR[1] || LOAD_Z_USE_MOTOR[2] ||
                          LOAD_Z_USE_MOTOR[3] || LOAD_Z_USE_MOTOR[4] || LOAD_Z_USE_MOTOR[5] ||
                          LOAD_Z_USE_MOTOR[6] || LOAD_Z_USE_MOTOR[7] || LOAD_Z_USE_MOTOR[8]);

    if(EL<TTabSheet>("TfSpeed", "tsLoader")->TabVisible)                                                    //Steven 20200724 : 修改Tray Z馬達速度設定
    {
        EL<TPanel>("TfSpeed", "pnlLoaderY")->Visible=LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnlpnlLoaderZ")->Visible=LOAD_Z_USE_MOTOR[0];
        EL<TPanel>("TfSpeed", "pnEmptyY")->Visible =LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnEmptyZ")->Visible =(LOAD_Z_USE_MOTOR[1] && AUTO_EMPTY_COLOR!=0);
        EL<TPanel>("TfSpeed", "pnlColorY")->Visible=LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnlColorZ")->Visible=(LOAD_Z_USE_MOTOR[2] && AUTO_EMPTY_COLOR!=0);
        EL<TPanel>("TfSpeed", "pnlAuto1Y")->Visible=LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnlAuto1Z")->Visible=LOAD_Z_USE_MOTOR[3];
        EL<TPanel>("TfSpeed", "pnlAuto2Y")->Visible=LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnlAuto2Z")->Visible=LOAD_Z_USE_MOTOR[4];
        EL<TPanel>("TfSpeed", "pnlAuto3Y")->Visible=LoaderUnload_StepMotor;
        EL<TPanel>("TfSpeed", "pnlAuto3Z")->Visible=LOAD_Z_USE_MOTOR[5];

        EL<TGroupBox>("TfSpeed", "grpAuto4")->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3) || LOAD_Z_USE_MOTOR[6]);
        EL<TPanel>("TfSpeed", "pnlAuto4Y")->Visible= (LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3);    //Steven 20230907 : For HT-9011UC
        EL<TPanel>("TfSpeed", "pnlAuto4Z")->Visible=LOAD_Z_USE_MOTOR[6];
        EL<TGroupBox>("TfSpeed", "grpAuto5")->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3) || LOAD_Z_USE_MOTOR[7]);
        EL<TPanel>("TfSpeed", "pnlAuto5Y")->Visible=(LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3);
        EL<TPanel>("TfSpeed", "pnlAuto5Z")->Visible=LOAD_Z_USE_MOTOR[7];
        EL<TGroupBox>("TfSpeed", "grpAuto6")->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=4) || LOAD_Z_USE_MOTOR[8]);
        EL<TPanel>("TfSpeed", "pnlAuto6Y")->Visible=(LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=4);
        EL<TPanel>("TfSpeed", "pnlAuto6Z")->Visible=LOAD_Z_USE_MOTOR[8];

        if(IniConfig.bSPILFunction)                                             //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
        {
            EL<TPanel>("TfSpeed", "pnlLoaderY")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnEmptyY")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlColorY")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto1Y")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto2Y")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto3Y")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto4Y")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto5Y")->Enabled=false;
            EL<TPanel>("TfSpeed", "pnlAuto6Y")->Enabled=false;
        }
    }

    EL<TGroupBox>("TfSpeed", "grpEmpty")->Visible=(AUTO_EMPTY_COLOR!=0);
    EL<TGroupBox>("TfSpeed", "grpColor")->Visible=(AUTO_EMPTY_COLOR!=0);
    //<==
    //Steven 20200529 : Loader入Tray改步進

    EL<TLabel>("TfSpeed", "lblTrayStepSpeedByMachine")->Visible=(LoaderUnload_StepMotor &&
                                        IniConfig.bP40TrayYSpeedByMachine);     //Sam 20201221 : Tray y step motor by machine
    #ifdef DEBUG_INDEX_UPH                                                      //Steven 20170818 : 測試UPH用
        EL<TGroupBox>("TfSpeed", "grpIndexUPHTryRun")->Visible=true;
    #else
        EL<TGroupBox>("TfSpeed", "grpIndexUPHTryRun")->Visible=false;
    #endif

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        if(AccessLevel<iDefHonPrecLevel)
        {
            EL<TGroupBox>("TfSpeed", "grpLoader")->Visible=false;
            EL<TGroupBox>("TfSpeed", "grpEmpty")->Visible=false;
            EL<TGroupBox>("TfSpeed", "grpColor")->Visible=false;
            EL<TGroupBox>("TfSpeed", "grpAuto1")->Visible=false;
            EL<TGroupBox>("TfSpeed", "grpAuto2")->Visible=false;
            EL<TGroupBox>("TfSpeed", "grpAuto3")->Visible=false;
        }
        else
        {
            EL<TGroupBox>("TfSpeed", "grpLoader")->Visible=true;
            EL<TGroupBox>("TfSpeed", "grpEmpty")->Visible=true;
            EL<TGroupBox>("TfSpeed", "grpColor")->Visible=true;
            EL<TGroupBox>("TfSpeed", "grpAuto1")->Visible=true;
            EL<TGroupBox>("TfSpeed", "grpAuto2")->Visible=true;
            EL<TGroupBox>("TfSpeed", "grpAuto3")->Visible=true;
        }
        EL<TGroupBox>("TfSpeed", "grpLoader")->Enabled=false;
        EL<TGroupBox>("TfSpeed", "grpEmpty")->Enabled=false;
        EL<TGroupBox>("TfSpeed", "grpColor")->Enabled=false;
        EL<TGroupBox>("TfSpeed", "grpAuto1")->Enabled=false;
        EL<TGroupBox>("TfSpeed", "grpAuto2")->Enabled=false;
        EL<TGroupBox>("TfSpeed", "grpAuto3")->Enabled=false;
    }

    EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed2")->Visible=(CUSTOMER_CODE==CC_ASE_KaohSiung);                 //kevin 20201225 2段速設定
    EL<TGroupBox>("TfSpeed", "gbInArmDieClean")->Visible=(USE_DIE_CLEAN==1);                                //wei 20170418
    EL<TGroupBox>("TfSpeed", "gbOutArmDieClean")->Visible=false;                                            //wei 20170418
    if(CosFunction.bOutArmPickUpErrBySetup)                                     //JerryYang 20220923 : E50改成選項
    {
        EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->Visible=true;
        if(AccessLevel==iDefHonPrecLevel)
        {
            EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->Enabled=true;
        }
        else
        {
            EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->Enabled=false;
        }
    }
    else
    {
        EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->Visible=false;
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[6])
        {
            EL<TEdit>("TfSpeed", "edIndexVacumCheckTime")->Enabled=false;
            EL<TEdit>("TfSpeed", "edIndexAirOnTime")->Enabled=false;
        }

        if(bAuthCriticalPara[11])
        {
            EL<TEdit>("TfSpeed", "edIndexSpeed")->Enabled=false;
        }
    }

    if(CUSTOMER_CODE==CC_QUALCOMM || IniConfig.bSPILFunction)                   //JerryYang 20260415 : add
    {
        EL<TGroupBox>("TfSpeed", "gbEnableDelayTimeZero")->Visible=false;
    }

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cSpeed.cpp:340  TfSpeed::ReadFile()
static void SP_ReadFile()
{

    AnsiString szDir="";
    LastFileName=GetLastOpenFN();
    if(CosFunction.bSaveArmSpeedByMachine && IniConfig.bA57_1SaveArmSpeedByMachine)                                     //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s", DataPath, LastFileName);
    }
    MyForceDirectories(szDir);
    szDir+="\\ArmCondition.Data";
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position                    =ReadIniData(szDir, "All", "Speed", 1);
    EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position                    =ReadIniData(szDir, "All", "Accel", 1);
    EL<TEdit>("TfSpeed", "edAllSpeed")->Text                        =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    EL<TEdit>("TfSpeed", "edAllAccSpeed")->Text                     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;

    if(USE_CKD_FCM_CleanAir)                                                    //Ifor 20150710 : EP Speed Control
    {
        EL<TGroupBox>("TfSpeed", "gbEPControl")->Visible                =true;                              //Ifor 20150710 :安裝CKD 才顯示
        EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->Position               =ReadIniData(szDir, "All", "EPControl", 1);
        EL<TEdit>("TfSpeed", "edEPControl")->Text                   =EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->Position;
        iEPControlValue                     =atoi(EL<TEdit>("TfSpeed", "edEPControl")->Text.c_str());
    }
    else
    {
        EL<TGroupBox>("TfSpeed", "gbEPControl")->Visible                =false;                             //Ifor 20150710 :安裝CKD 才顯示
    }
    //--------------------------------------------------------------------------
    //Index Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[IndexArm].iBodySP         =ReadIniData(szDir, "Index Arm", "X Speed",                  1);
    ArmSpeed_File[IndexArm].iACDCBodySP     =100;                               //ReadIniData(szDir, "Index Arm", "X Accel",                100);
    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[IndexArm].iBodySP     =CheckRange(ArmSpeed_File[IndexArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20240913 ASE 新加坡客戶要求 X => Z
    {
        ArmSpeed_File[IndexArm].iBodySP     =CheckAndReadIniData(szDir, "Index Arm", "Y Speed",   ArmSpeed_File[IndexArm].iBodySP);
        ArmSpeed_File[IndexArm].iACDCBodySP =CheckAndReadIniData(szDir, "Index Arm", "Y Accel",   ArmSpeed_File[IndexArm].iACDCBodySP);
    }
    ArmSpeed_File[IndexArm].iRetryCT        =ReadIniData(szDir, "Index Arm", "Retry Count",              0);
    ArmSpeed_File[IndexArm].iRetryCT        =0;
    ArmSpeed_File[IndexArm].dRetryDown      =ReadIniData(szDir, "Index Arm", "Retry Down",             0.0);
    ArmSpeed_File[IndexArm].dVacuumTI       =ReadIniData(szDir, "Index Arm", "Vacuum Check Time",      1.0);
    ArmSpeed_File[IndexArm].dCTAirOn        =ReadIniData(szDir, "Index Arm", "Counter Air ON Time",    1.0);
    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[IndexArm].dDestroyAgainTime=ReadIniData(szDir, "Index Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[IndexArm].iDestroyAgainCount=ReadIniData(szDir, "Index Arm", "Destroy Again Count", 1);

    if(IniConfig.bD32_35TrayPitchIndexDOnMoreThen500MS)                         //Steven 20120727 : Tray Pitch 大於35mm的話, Index吹氣至少要0.5秒
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效   //Steven 20140221
        {
            ArmSpeed_File[IndexArm].dCTAirOn=CheckRange(ArmSpeed_File[IndexArm].dCTAirOn, 0.5, 10.0);
        }
    }

    ArmSpeed_File[IndexArm].bIndexFloatCHK  =ReadIniData(szDir, "Index Arm", "Socket Check",             0);
    ArmSpeed_File[IndexArm].bSuckOnDown     =ReadIniData(szDir, "Index Arm", "Vacuum Timing",            0);

    if(CUSTOMER_CODE==CC_SINOICTECH || CUSTOMER_CODE==CC_FMSH)                  //Steven 20221207 : 華嶺預設是打開
        TestIF_File.bIndexPickICWhenOutShtNoIC  =ReadIniData(szDir, "Index Arm", "Pick IC when out shuttle no IC", 1);  //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    else
        TestIF_File.bIndexPickICWhenOutShtNoIC  =ReadIniData(szDir, "Index Arm", "Pick IC when out shuttle no IC", 0);

    if(CUSTOMER_CODE==CC_QUALCOMM || IniConfig.bSPILFunction)                                                          //JerryYang 20250826 : QCT不使用 //JerryYang 20260415 : add
    {
        TestIF_File.bEnableDelayTimeZero=false;
    }
    else
    {
        TestIF_File.bEnableDelayTimeZero=(ReadIniData(szDir, "Index Arm", "Enable Delay Time Zero", 0)==1)?true:false;  //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add “Air purge during place device on out-shuttle” function.
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[IndexArm].bDevicConfirm=ReadIniData(szDir, "Index Arm", "DevicConfirm", true);                    //kevin 20190629 add index 4 site conform
    else
        ArmSpeed_File[IndexArm].bDevicConfirm=false;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bD44CheckIndexICDestroy && ArmSpeed_File[IndexArm].bDevicConfirm)   //kevin 20190629 add 高雄強制4個 SITE INDEX check
    {
        IniConfig.bD44CheckIndexICDestroy=false;
    }
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
    //==>
    if(CUSTOMER_CODE==CC_SCK || IniConfig.bSPILFunction)                        //JerryYang 20220824 : SPIL index cycle time monitor
    {
        if(IniConfig.bSPILFunction)
        {
            TestIF_File.bIndexCycleTimeMonitor  =ReadIniData(szDir, "Index Arm", "EnableNewIndexCycleTimeMonitoring", false);                                   //JerryYang 20230131 : 預設先不開啟
        }
        else
        {
            TestIF_File.bIndexCycleTimeMonitor  =ReadIniData(szDir, "Index Arm", "EnableIndexCycleTimeMonitoring", true);
        }

        TestIF_File.dIndexCycletimeMonitor      =ReadIniData(szDir, "Index Arm", "Monitoring_IndexCycletime", 0.0);
        TestIF_File.dMonitorOutlier             =ReadIniData(szDir, "Index Arm", "Monitoring_Outlier", 0.0);
        TestIF_File.iMonitorWindow              =ReadIniData(szDir, "Index Arm", "Monitoring_Window", 0);
        TestIF_File.dICTTolerance               =ReadIniData(szDir, "Index Arm", "IndexCycleTimetolerance", 15);
        TestIF_File.iICTAction                  =ReadIniData(szDir, "Index Arm", "IndexCycleTimeAction", 0);            //Steven 20250521 : 可以選擇要不要停機
    }
    else
    {
        TestIF_File.bIndexCycleTimeMonitor      =false;
    }
    //<==
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
    if(CosFunction.bOutArmPickUpErrBySetup==true)                               //JerryYang 20220923 : E50改成選項
    {
        IniConfig.iE50_OutArmPickUpErrorOption=ReadIniData(szDir, "Output Arm", "iE50_OutArmPickUpErrorOption", 0);
    }

    //--------------------------------------------------------------------------
    //InPut Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[InArm].iBodySP            =ReadIniData(szDir, "Input Arm", "XY Speed",                 1);
    if(IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent)                      //Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80%
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效    //Steven 20140221
        {
            ArmSpeed_File[InArm].iBodySP    =CheckRange(ArmSpeed_File[InArm].iBodySP, 80, 1);
        }
    }

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[InArm].iBodySP        =CheckRange(ArmSpeed_File[InArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    AutoArmSpeed[InArm].iBodySP             =ArmSpeed_File[InArm].iBodySP;      //jou 2012-05-16 Auto Speed
    ArmSpeed_File[InArm].iACDCBodySP        =ReadIniData(szDir, "Input Arm", "XY Accel",                 1);
    AutoArmSpeed[InArm].iACDCBodySP         =ArmSpeed_File[InArm].iACDCBodySP;  //jou 2012-05-16 Auto Speed
    ArmSpeed_File[InArm].iZSP               =ReadIniData(szDir, "Input Arm", "Z Up Speed",               1);
    ArmSpeed_File[InArm].iACDCZSP           =ReadIniData(szDir, "Input Arm", "Z Up Accel",               1);
    ArmSpeed_File[InArm].bAutoSpeed         =ReadIniData(szDir, "Input Arm", "Auto Speed Down",          0);
    ArmSpeed_File[InArm].iAutoSpeedLow      =ReadIniData(szDir, "Input Arm", "AutoSpeedLow",            50);            //kevin 20210913 AutoSpeed 最低速
    if(CosFunction.bEnableAutoSpeed==false)                                     //Steven 20110526
        ArmSpeed_File[InArm].bAutoSpeed     =false;
    ArmSpeed_File[InArm].iRetryCT           =ReadIniData(szDir, "Input Arm", "Retry Count",              1);
    ArmSpeed_File[InArm].dRetryDown         =ReadIniData(szDir, "Input Arm", "Retry Down",             1.0);
    ArmSpeed_File[InArm].dVacuumTI          =ReadIniData(szDir, "Input Arm", "Vacuum Check Time",      1.0);
    ArmSpeed_File[InArm].dHPVacuumTI        =ReadIniData(szDir, "Input Arm", "HP Vacuum Check Time",   1.0);            //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    ArmSpeed_File[InArm].bUseHPVacuum       =ReadIniData(szDir, "Input Arm", "Use HP Vacuum Check Time",  false);       //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    if(IniConfig.bE57HPCanUseAnotherVacuumDelay==false)
        ArmSpeed_File[InArm].bUseHPVacuum=false;
    ArmSpeed_File[InArm].dCTAirOn           =ReadIniData(szDir, "Input Arm", "Counter Air ON Time",    1.0);
    ArmSpeed_File[InArm].dWaitOnSH          =ReadIniData(szDir, "Input Arm", "Shuttle Wait Time",      1.0);
    ArmSpeed_File[InArm].bAutoSKIP          =ReadIniData(szDir, "Input Arm", "Auto Skip",                0);
    ArmSpeed_File[InArm].iAutoSkipCT        =ReadIniData(szDir, "Input Arm", "Auto Skip CT",            20);
    ArmSpeed_File[InArm].bVariModeFIX       =ReadIniData(szDir, "Input Arm", "One by one",               0);

    if(IniConfig.bSPILFunction==true && ArmSpeed_File[InArm].bAutoSKIP)         //wei 20150317 新竹矽品 auto skip 強制手動拿Tray  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        TrayForm.iManualRemoveLoader=1;
    }

    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            ArmSpeed_File[InArm].bVariModeFIX=true;
        }

        TestIF_File.iInArmToShtReleaseMode  =ReadIniData(szDir, "Input Arm", "iInArmToShtReleaseMode", 0);              //Steven 20250630 : 力旺要照順序擺
    }
    else
    {
        TestIF_File.iInArmToShtReleaseMode  =0;                                 //Steven 20250630 : 力旺要照順序擺
    }

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ArmSpeed_File[InArm].bVariModeFIX=true;
    }

    ArmSpeed_File[InArm].iTwoSpeedMove     =ReadIniData(szDir, "Input Arm", "Two Speed On Off",    0);                  //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].dTwoSpeedDistance =ReadIniData(szDir, "Input Arm", "Two Speed Distance",  3.0);                //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].iTwoSpeed         =ReadIniData(szDir, "Input Arm", "Two Speed Precent",   10);                 //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].iTwoADC           =ReadIniData(szDir, "Input Arm", "Two Speed ADC",       10);                 //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].bTwoSpeedOnlyLoader=ReadIniData(szDir, "Input Arm", "Two Speed Only Loader", false);           //JerryYang 20190729 二段速功能可選擇only at loader

    ArmSpeed_File[InArm].bSuckOnDown       =ReadIniData(szDir, "Input Arm", "Vacuum Timing",            0);
    ArmSpeed_File[InArm].iVariSP           =ReadIniData(szDir, "Input Arm", "Open/Close Speed",         1);
    ArmSpeed_File[InArm].iACDCVariSP       =ReadIniData(szDir, "Input Arm", "Open/Close Accel",         1);

    //ChungHung 20131231 alter AutoYPitch start
    ArmSpeed_File[InArm].iVariSPY          =ArmSpeed_File[InArm].iVariSP;       //Steven 20140930 : For XY-Pitch
    ArmSpeed_File[InArm].iACDCVariSPY      =ArmSpeed_File[InArm].iACDCVariSP;
    //ChungHung 20131231 alter AutoYPitch end

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[InArm].dDestroyAgainTime  =ReadIniData(szDir, "Input Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[InArm].iDestroyAgainCount =ReadIniData(szDir, "Input Arm", "Destroy Again Count", 1);

    //2013-08-01    Dell    modify  公司統一回吸功能
    ArmSpeed_File[InArm].dDestroyCheckTime  =ReadIniData(szDir, "Input Arm", "Destroy Check Time", 0.1);
    ArmSpeed_File[InArm].bDestroyPauseCheck =ReadIniData(szDir, "Input Arm", "Destroy Pause Check", 0);

    ArmSpeed_File[InRotate].iBodySP         =ReadIniData(szDir, "Input Arm", "Rotate Input Speed", 0.1);                //kevin 20130517 rotate add
    ArmSpeed_File[InRotate].iACDCBodySP     =ReadIniData(szDir, "Input Arm", "Rotate Input Accel", 0);

    ArmSpeed_File[InArm].dCylinderDelay     =ReadIniData(szDir, "Input Arm", "Cylinder Delay", 0.1);
    ArmSpeed_File[InArm].PrecisorOpenSp      =ReadIniData(szDir, "Input Arm", "PrecisorOpenSpeed", 1);                  //JerryYang 20250120 : add
    ArmSpeed_File[InArm].PrecisorCloseSp     =ReadIniData(szDir, "Input Arm", "PrecisorCloseSpeed", 1);
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //JerryYang 20170406 (Steven) 只有TSMC才使用input arm release IC前delay
    {
        ArmSpeed_File[InArm].dReleaseDelayTime  =ReadIniData(szDir, "Input Arm", "Release Delay Time", 0.0);            //JerryYang 20160127 for TSMC inarm release device前delay
        ArmSpeed_File[InArm].iEnableReleaseDelay=ReadIniData(szDir, "Input Arm", "Enable Relase Delay", 1);             //JerryYang 20160127 for TSMC inarm release device前delay
    }
    else
    {
        ArmSpeed_File[InArm].dReleaseDelayTime=0.0;                             //JerryYang 20160127 for TSMC inarm release device前delay
        ArmSpeed_File[InArm].iEnableReleaseDelay=1;                             //JerryYang 20160127 for TSMC inarm release device前delay
    }

    if(USE_DIE_CLEAN)
    {
        ArmSpeed_File[InArm].iEnableDieClean    =ReadIniData(szDir, "Input Arm", "Enable Die Clean",  0);               //wei 20170418
        ArmSpeed_File[InArm].dDieCleanDelay     =ReadIniData(szDir, "Input Arm", "Die Clean Delay",   0.0);             //wei 20170418
        ArmSpeed_File[InArm].dDieCleanHeight    =ReadIniData(szDir, "Input Arm", "Die Clean Height",  0.1);             //wei 20170418
    }
    else
    {
        ArmSpeed_File[InArm].iEnableDieClean    =0;
    }

    ArmSpeed_File[InArm].bHeightCheck  =ReadIniData(szDir, "Input Arm", "Enable Height Check", false);                  //wei 20160825 回吸偵測
    ArmSpeed_File[InArm].iHeightCheck  =ReadIniData(szDir, "Input Arm", "Height Check",         1000);                  //wei 20160825 回吸偵測
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[InArm].bDevicConfirm=ReadIniData(szDir, "Input Arm", "DevicConfirm", true);                       //kevin 20170510 (wei) 回吸偵測開關
    else
        ArmSpeed_File[InArm].bDevicConfirm=false;

    if(CosFunction.bYPitchNotUseSearchLastMode && USE_OUT_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20200324 中科志強說預設要false
        {
            ArmSpeed_File[InArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Input Arm", "bYPitchNotUseSearchLastMode", false);
        }
        else
        {
            ArmSpeed_File[InArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Input Arm", "bYPitchNotUseSearchLastMode", true);
        }
    }
    else
    {
        ArmSpeed_File[InArm].bYPitchUseSearchLastMode=true;
    }

    ArmSpeed_File[InArm].bTT_SetSpeed         =ReadIniData(szDir, "Input Arm", "Test Time Set Speed",          0);      //Ztex 2024.08.11 Add Test Time Set Speed
    if(ArmSpeed_File[InArm].bTT_SetSpeed==1)                                    //Ztex 2024.08.11 Add Test Time Set Speed
        ArmSpeed_File[InArm].bAutoSpeed=0;
    //--------------------------------------------------------------------------
    //Output Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[OutArm].iBodySP           =ReadIniData(szDir, "Output Arm", "XY Speed",                1);
    if(IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent)                      //Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80%
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效
        {
            ArmSpeed_File[OutArm].iBodySP   =CheckRange(ArmSpeed_File[OutArm].iBodySP, 80, 1);
        }
    }

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[OutArm].iBodySP       =CheckRange(ArmSpeed_File[OutArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    AutoArmSpeed[OutArm].iBodySP            =ArmSpeed_File[OutArm].iBodySP;     //jou 2012-05-16 Auto Speed
    ArmSpeed_File[OutArm].iACDCBodySP       =ReadIniData(szDir, "Output Arm", "XY Accel",                1);
    AutoArmSpeed[OutArm].iACDCBodySP        =ArmSpeed_File[OutArm].iACDCBodySP;                                         //jou 2012-05-16 Auto Speed
    ArmSpeed_File[OutArm].iZSP              =ReadIniData(szDir, "Output Arm", "Z Up Speed",              1);
    ArmSpeed_File[OutArm].iACDCZSP          =ReadIniData(szDir, "Output Arm", "Z Up Accel",              1);
    ArmSpeed_File[OutArm].iRetryCT          =ReadIniData(szDir, "Output Arm", "Retry Count",             1);
    ArmSpeed_File[OutArm].dRetryDown        =ReadIniData(szDir, "Output Arm", "Retry Down",              1.0);
    ArmSpeed_File[OutArm].dVacuumTI         =ReadIniData(szDir, "Output Arm", "Vacuum Check Time",       1.0);
    ArmSpeed_File[OutArm].dCTAirOn          =ReadIniData(szDir, "Output Arm", "Counter Air ON Time",     1.0);
    ArmSpeed_File[OutArm].dWaitOnSH         =ReadIniData(szDir, "Output Arm", "Shuttle Wait Time",       0.0);          //kevin 20210525 out arm 在 shuttlr 上等待時間到再往下吸取
    ArmSpeed_File[OutArm].bVariModeFIX      =ReadIniData(szDir, "Output Arm", "One by one",              0);

    ArmSpeed_File[OutArm].iAutoSpeedLow     =ReadIniData(szDir, "Input Arm", "AutoSpeedLow",            50);            //kevin 20210913 AutoSpeed 最低速

    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            ArmSpeed_File[OutArm].bVariModeFIX=true;
        }
    }

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ArmSpeed_File[OutArm].bVariModeFIX=true;
    }

    ArmSpeed_File[OutArm].bSuckOnDown       =ReadIniData(szDir, "Output Arm", "Vacuum Timing",           0);
    ArmSpeed_File[OutArm].iVariSP           =ReadIniData(szDir, "Output Arm", "Open/Close Speed",        1);
    ArmSpeed_File[OutArm].iACDCVariSP       =ReadIniData(szDir, "Output Arm", "Open/Close Accel",        1);
    ArmSpeed_File[OutArm].dDestroyPauseTime =ReadIniData(szDir, "Output Arm", "Wait before Air On",      0.01);
    ArmSpeed_File[OutArm].iTwoSpeedMove     =ReadIniData(szDir, "Output Arm", "Two Speed On Off",        0);            //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].dTwoSpeedDistance =ReadIniData(szDir, "Output Arm", "Two Speed Distance",      3.0);          //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].iTwoSpeed         =ReadIniData(szDir, "Output Arm", "Two Speed Precent",       10);           //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].iTwoADC           =ReadIniData(szDir, "Output Arm", "Two Speed ADC",           10);           //Steven 20140217 : 兩段速移動

    //ChungHung 20131231 alter AutoYPitch start
    ArmSpeed_File[OutArm].iVariSPY          =ArmSpeed_File[OutArm].iVariSP;     //Steven 20140930 : For XY-Pitch
    ArmSpeed_File[OutArm].iACDCVariSPY      =ArmSpeed_File[OutArm].iACDCVariSP;
    //ChungHung 20131231 alter AutoYPitch end

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[OutArm].dDestroyAgainTime =ReadIniData(szDir, "Output Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[OutArm].iDestroyAgainCount=ReadIniData(szDir, "Output Arm", "Destroy Again Count", 1);

    //2013-08-01    Dell    modify  公司統一回吸功能
    ArmSpeed_File[OutArm].dDestroyCheckTime =ReadIniData(szDir, "Output Arm", "Destroy Check Time", 0.1);
    ArmSpeed_File[OutArm].bDestroyPauseCheck=ReadIniData(szDir, "Output Arm", "Destroy Pause Check", 0);

    ArmSpeed_File[OutArm].dCylinderDelay    =ReadIniData(szDir, "Output Arm", "Cylinder Delay", 0.1);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[OutArm].bDevicConfirm =ReadIniData(szDir, "Output Arm", "DevicConfirm", true);                    //kevin 20170510 (wei) 回吸偵測開關
    else
        ArmSpeed_File[OutArm].bDevicConfirm =false;

    if(CosFunction.bYPitchNotUseSearchLastMode && USE_OUT_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20200324 中科志強說預設要false
        {
            ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Output Arm", "bYPitchNotUseSearchLastMode", false);
        }
        else
        {
            ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Output Arm", "bYPitchNotUseSearchLastMode", true);
        }
    }
    else
    {
        ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=true;
    }

    if(USE_DIE_CLEAN)
    {
        ArmSpeed_File[OutArm].iEnableDieClean   =ReadIniData(szDir, "Output Arm", "Enable Die Clean",  0);              //wei 20170418
        ArmSpeed_File[OutArm].dDieCleanDelay    =ReadIniData(szDir, "Output Arm", "Die Clean Delay",   0.0);            //wei 20170418
    }
    else
    {
        ArmSpeed_File[OutArm].iEnableDieClean   =0;
    }

    ArmSpeed_File[OutRotate].iBodySP        =ReadIniData(szDir, "Output Arm", "Rotate Output Speed", 0.1);              //Steven 20140520 : Input --> Output
    ArmSpeed_File[OutRotate].iACDCBodySP    =ReadIniData(szDir, "Output Arm", "Rotate Output Accel", 0);                //Steven 20140520 : Input --> Output
    //--------------------------------------------------------------------------
    //Empty Tray Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[TrayArm].iBodySP          =ReadIniData(szDir, "Empty Tray Arm", "Speed",               1);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[TrayArm].iBodySP      =CheckRange(ArmSpeed_File[TrayArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    ArmSpeed_File[TrayArm].iACDCBodySP      =ReadIniData(szDir, "Empty Tray Arm", "Accel",               1);
    ArmSpeed_File[TrayArm].iRetryCT         =ReadIniData(szDir, "Empty Tray Arm", "Retry Count",         1);
    ArmSpeed_File[TrayArm].dVacuumTI        =ReadIniData(szDir, "Empty Tray Arm", "Vacuum Check Time",   1.0);
    ArmSpeed_File[TrayArm].dCTAirOn         =ReadIniData(szDir, "Empty Tray Arm", "Counter Air ON Time", 1.0);
    ArmSpeed_File[TrayArm].dTAHeadDownIT    =ReadIniData(szDir, "Empty Tray Arm", "Hand Down Time",      1.0);
    if(IniConfig.bC03UseCatchTray)                                              //use catch Tray Fix
    {
        ArmSpeed_File[TrayArm].dTAHeadDownIT=CheckRange(ArmSpeed_File[TrayArm].dTAHeadDownIT, 0.3, 10.0);
    }
    //--------------------------------------------------------------------------
    //Shuttle
    //--------------------------------------------------------------------------
    SHSpeed_File.iSH1Sp                    =ReadIniData(szDir, "Shuttle", "Shuttle 1 Speed",    1);
    SHSpeed_File.iShakeCycles              =ReadIniData(szDir, "Shuttle", "Shake Cycles",           1);                 //Sam 20250326 : 新增 Shake 條件設定
    SHSpeed_File.iShakeDistance            =ReadIniData(szDir, "Shuttle", "Shake distance",         5);
    SHSpeed_File.dShakeDelay               =ReadIniData(szDir, "Shuttle", "Delay between shakes",   0.0);
    SHSpeed_File.iShakeAccDec              =ReadIniData(szDir, "Shuttle", "Shake Accel",            100);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        SHSpeed_File.iSH1Sp                 =CheckRange(SHSpeed_File.iSH1Sp, CosFunction.iLimitMaxSpeed, 1);
    }

    SHSpeed_File.iSH1ACDCSp                =ReadIniData(szDir, "Shuttle", "Shuttle 1 Accel",    1);
    SHSpeed_File.iSH2Sp                    =ReadIniData(szDir, "Shuttle", "Shuttle 2 Speed",    1);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        SHSpeed_File.iSH2Sp                 =CheckRange(SHSpeed_File.iSH2Sp, CosFunction.iLimitMaxSpeed, 1);
    }

    SHSpeed_File.iSH2ACDCSp                 =ReadIniData(szDir, "Shuttle", "Shuttle 2 Accel",    1);
    SHSpeed_File.iStepSH                    =ReadIniData(szDir, "Shuttle", "Step Shuttle",       0);
    TestIF_File.iShakeShuttleWhenPlaceIC    =ReadIniData(szDir, "Shuttle", "Shake Shuttle",      0);                    //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭

    //Steven 20170317 (wei) mark
    //==>
//    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                         //Steven 20170202 : For ATJ Out Arm pickup error put to define tray
//    {
//        TestIF_File.bOutPickErrAct          =ReadIniData(szDir, "Output Arm", "Out Pick Err Action", false);
//        TestIF_File.iOutPickErrAuto         =ReadIniData(szDir, "Output Arm", "Out Pick Err Tray", 6);
//    }
//    else
    //<==
    //Steven 20170317 (wei) mark
    {
        TestIF_File.bOutPickErrAct          =false;
        TestIF_File.iOutPickErrAuto         =6;
    }

    iIndexSpeed=atoi(EL<TEdit>("TfSpeed", "edtTrySpeed")->Text.c_str());                                //Steven 20170818 : 測試UPH用
    iIndexAcc=atoi(EL<TEdit>("TfSpeed", "edtTryAcc")->Text.c_str());

    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    MGSpeed_File.iCYSp                  =ReadIniData(szDir, "Magazine", "Magazine CatchY Speed",    1);
    MGSpeed_File.iCYACDCSp              =ReadIniData(szDir, "Magazine", "Magazine CatchY Accel",    1);

    MGSpeed_File.iMZSp                  =ReadIniData(szDir, "Magazine", "MagazineZ Speed",    1);
    MGSpeed_File.iMZACDCSp              =ReadIniData(szDir, "Magazine", "MagazineZ Accel",    1);

    MGSpeed_File.dCYDecRange            =ReadIniData(szDir, "Magazine", "Two Speed Distance",  3.0);
    MGSpeed_File.iCY2Sp                 =ReadIniData(szDir, "Magazine", "Two Speed Precent",   10);
    MGSpeed_File.iCY2ACDCSp             =ReadIniData(szDir, "Magazine", "Two Speed ADC",       10);

    SP_ReadWriteFile(true, szDir);

}

// golden cSpeed.cpp:784  TfSpeed::ReadWriteFile(bool bRead, AnsiString szDir)
static void SP_ReadWriteFile(bool bRead, AnsiString szDir)
{
    filerw::ELMark("ReadWriteFile");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szTrayDir="";
    szTrayDir=szDir;
    int idefaultSpeed=0;
    if(IniConfig.bP40TrayYSpeedByMachine)                                       //Sam 20201221 : Tray y step motor by machine
       szTrayDir=asTrayStepSpeedByMachinePatch;

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20210325 : 晶兆成要求預設值為70
        idefaultSpeed=70;
    else
        idefaultSpeed=100;

    if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:797-879 tray-Y step motor speeds via dmTrayMotor not ported (GATE S5) -- not shown, not saved");   // golden cSpeed.cpp:797-879 取代：LoaderUnload_StepMotor：Tray Y 步進速度全經 dmTrayMotor（移植樹沒有 dmTrayMotor，GATE S5）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:797-879 -- LoaderUnload_StepMotor：Tray Y 步進速度全經 dmTrayMotor（移植樹沒有 dmTrayMotor，GATE S5）
    if(LoaderUnload_StepMotor)                                                  //Steven 20200529 : Loader入Tray改步進
    {
        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20250120 : add
        {
            dmTrayMotor->iStepMotorSpeed[0]=60;
            dmTrayMotor->iStepMotorSpeed[1]=60;
            dmTrayMotor->iStepMotorSpeed[2]=60;
            dmTrayMotor->iStepMotorSpeed[3]=60;
        }
        else if(IniConfig.bSPILFunction)                                        //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
        {
            dmTrayMotor->iStepMotorSpeed[0]=75;
            dmTrayMotor->iStepMotorSpeed[1]=75;
            dmTrayMotor->iStepMotorSpeed[2]=75;
            dmTrayMotor->iStepMotorSpeed[3]=75;
            dmTrayMotor->iStepMotorSpeed[0]=75;
            dmTrayMotor->iStepMotorSpeed[6]=75;
            dmTrayMotor->iStepMotorSpeed[7]=75;
            dmTrayMotor->iStepMotorSpeed[8]=75;
        }
        else
        {
            dmTrayMotor->iStepMotorSpeed[0]=ReadWriteIni(szTrayDir, "Tray", "Loader Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed1")->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);  //Sam 20201221 : Tray y step motor by machine
            dmTrayMotor->iStepMotorSpeed[1]=ReadWriteIni(szTrayDir, "Tray", "Auto 1 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto1Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            dmTrayMotor->iStepMotorSpeed[2]=ReadWriteIni(szTrayDir, "Tray", "Auto 2 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto2Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            dmTrayMotor->iStepMotorSpeed[3]=ReadWriteIni(szTrayDir, "Tray", "Auto 3 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto3Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            if(AUTO_EMPTY_COLOR>=3)                                             //Steven 20230907 : For HT-9011UC
            {
                dmTrayMotor->iStepMotorSpeed[6]=ReadWriteIni(szTrayDir, "Tray", "Auto 4 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto4Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
                dmTrayMotor->iStepMotorSpeed[7]=ReadWriteIni(szTrayDir, "Tray", "Auto 5 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto5Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            }

            if(AUTO_EMPTY_COLOR>=4)                                             //Steven 20230907 : For HT-9011UC
                dmTrayMotor->iStepMotorSpeed[8]=ReadWriteIni(szTrayDir, "Tray", "Auto 6 Tray Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto6Speed1")->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
        }

        if(AUTO_EMPTY_COLOR!=0)
        {
            if(CUSTOMER_CODE==CC_ASE_CL)
            {
                dmTrayMotor->iStepMotorSpeed[4]=60;
                dmTrayMotor->iStepMotorSpeed[5]=60;                             //JerryYang 20240930 : 35 -> 60
            }
            else if(IniConfig.bSPILFunction)                                    //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
            {
                dmTrayMotor->iStepMotorSpeed[4]=75;
                dmTrayMotor->iStepMotorSpeed[5]=75;
            }
            else
            {
                dmTrayMotor->iStepMotorSpeed[4]=ReadWriteIni(szTrayDir, "Tray", "Empty Tray Speed",      atoi(EL<TLabeledEdit>("TfSpeed", "edtEmptySpeed1")->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);
                dmTrayMotor->iStepMotorSpeed[5]=ReadWriteIni(szTrayDir, "Tray", "Color Tray Speed",      atoi(EL<TLabeledEdit>("TfSpeed", "edtColorSpeed1")->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);
            }
            dmTrayMotor->iStepMotorSpeed[9]    =ReadWriteIni(szTrayDir, "Tray", "Loader Tray 2nd Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed2")->Text.c_str()), 20, bRead, true, 1, 100);
        }

        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeed1")->Position=dmTrayMotor->iStepMotorSpeed[0];           //Steven 20210625 : 修正軌道速度初始化設定問題
            EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed1")->Text   =dmTrayMotor->iStepMotorSpeed[0];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto1Speed1")->Position =dmTrayMotor->iStepMotorSpeed[1];
            EL<TLabeledEdit>("TfSpeed", "edtAuto1Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[1];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto2Speed1")->Position =dmTrayMotor->iStepMotorSpeed[2];
            EL<TLabeledEdit>("TfSpeed", "edtAuto2Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[2];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto3Speed1")->Position =dmTrayMotor->iStepMotorSpeed[3];
            EL<TLabeledEdit>("TfSpeed", "edtAuto3Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[3];
            EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeed1")->Position =dmTrayMotor->iStepMotorSpeed[4];
            EL<TLabeledEdit>("TfSpeed", "edtEmptySpeed1")->Text    =dmTrayMotor->iStepMotorSpeed[4];
            EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeed1")->Position =dmTrayMotor->iStepMotorSpeed[5];
            EL<TLabeledEdit>("TfSpeed", "edtColorSpeed1")->Text    =dmTrayMotor->iStepMotorSpeed[5];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto4Speed1")->Position =dmTrayMotor->iStepMotorSpeed[6];
            EL<TLabeledEdit>("TfSpeed", "edtAuto4Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[6];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto5Speed1")->Position =dmTrayMotor->iStepMotorSpeed[7];
            EL<TLabeledEdit>("TfSpeed", "edtAuto5Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[7];
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto6Speed1")->Position =dmTrayMotor->iStepMotorSpeed[8];
            EL<TLabeledEdit>("TfSpeed", "edtAuto6Speed1")->Text    =dmTrayMotor->iStepMotorSpeed[8];

            EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeed2")->Text   =dmTrayMotor->iStepMotorSpeed[9];           //SECOND SPEED   kevin 20201211
        }

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            dmTrayMotor->bNeedSetStepMotorSpeed=true;                           //kevin 20210602
    }
#endif // GATE (S12-C save)

    if(LOAD_Z_USE_MOTOR[0])                                                     //Steven 20200724 : 修改Tray Z馬達速度設定
    {
        TestIF_File.iTrayZStepSpeed[0]=ReadWriteIni(szDir, "Tray", "Loader Tray Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udLoaderSpeedZ")->Position=TestIF_File.iTrayZStepSpeed[0];
            EL<TLabeledEdit>("TfSpeed", "edtLoaderSpeedZ")->Text=TestIF_File.iTrayZStepSpeed[0];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[0]=50;
    }

    if(AUTO_EMPTY_COLOR!=0 && LOAD_Z_USE_MOTOR[1])
    {
        TestIF_File.iTrayZStepSpeed[1]=ReadWriteIni(szDir, "Tray", "Empty Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtEmptySpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udEmptySpeedZ")->Position=TestIF_File.iTrayZStepSpeed[1];
            EL<TLabeledEdit>("TfSpeed", "edtEmptySpeedZ")->Text=TestIF_File.iTrayZStepSpeed[1];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[1]=50;
    }

    if(AUTO_EMPTY_COLOR!=0 && LOAD_Z_USE_MOTOR[2])
    {
        TestIF_File.iTrayZStepSpeed[2]=ReadWriteIni(szDir, "Tray", "Color Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtColorSpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udColorSpeedZ")->Position=TestIF_File.iTrayZStepSpeed[2];
            EL<TLabeledEdit>("TfSpeed", "edtColorSpeedZ")->Text=TestIF_File.iTrayZStepSpeed[2];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[2]=50;
    }

    if(LOAD_Z_USE_MOTOR[3])
    {
        TestIF_File.iTrayZStepSpeed[3]=ReadWriteIni(szDir, "Tray", "Auto 1 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto1SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto1SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[3];
            EL<TLabeledEdit>("TfSpeed", "edtAuto1SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[3];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[3]=50;
    }

    if(LOAD_Z_USE_MOTOR[4])
    {
        TestIF_File.iTrayZStepSpeed[4]=ReadWriteIni(szDir, "Tray", "Auto 2 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto2SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto2SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[4];
            EL<TLabeledEdit>("TfSpeed", "edtAuto2SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[4];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[4]=50;
    }

    if(LOAD_Z_USE_MOTOR[5])
    {
        TestIF_File.iTrayZStepSpeed[5]=ReadWriteIni(szDir, "Tray", "Auto 3 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto3SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto3SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[5];
            EL<TLabeledEdit>("TfSpeed", "edtAuto3SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[5];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[5]=50;
    }

    if(LOAD_Z_USE_MOTOR[6])                                                     //Steven 20230907 : For HT-9011UC
    {
        TestIF_File.iTrayZStepSpeed[6]=ReadWriteIni(szDir, "Tray", "Auto 4 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto4SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto4SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[6];
            EL<TLabeledEdit>("TfSpeed", "edtAuto4SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[6];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[6]=50;
    }

    if(LOAD_Z_USE_MOTOR[7])
    {
        TestIF_File.iTrayZStepSpeed[7]=ReadWriteIni(szDir, "Tray", "Auto 5 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto5SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto5SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[7];
            EL<TLabeledEdit>("TfSpeed", "edtAuto5SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[7];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[7]=50;
    }

    if(LOAD_Z_USE_MOTOR[8])
    {
        TestIF_File.iTrayZStepSpeed[8]=ReadWriteIni(szDir, "Tray", "Auto 6 Z Speed", atoi(EL<TLabeledEdit>("TfSpeed", "edtAuto6SpeedZ")->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            EL<filerw::ELTrackBar>("TfSpeed", "udAuto6SpeedZ")->Position=TestIF_File.iTrayZStepSpeed[8];
            EL<TLabeledEdit>("TfSpeed", "edtAuto6SpeedZ")->Text=TestIF_File.iTrayZStepSpeed[8];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[8]=50;
    }

}

// golden cSpeed.cpp:1008  TfSpeed::DoIniDataToForm()
static void SP_DoIniDataToForm()
{

    //--------------------------------------------------------------------------
    //Index Arm
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->Position            =ArmSpeed_File[IndexArm].iBodySP;
    EL<TEdit>("TfSpeed", "edIndexSpeed")->Text              =EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Position            =ArmSpeed_File[IndexArm].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Text             =EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Position;
    EL<TEdit>("TfSpeed", "edIndexRetryCount")->Text         =ArmSpeed_File[IndexArm].iRetryCT;
    EL<TEdit>("TfSpeed", "edIndexArmRetryMM")->Text         =FormatFloat("0.00", ArmSpeed_File[IndexArm].dRetryDown);
    EL<TEdit>("TfSpeed", "edIndexVacumCheckTime")->Text     =FormatFloat("0.00", ArmSpeed_File[IndexArm].dVacuumTI);
    EL<TEdit>("TfSpeed", "edIndexAirOnTime")->Text          =FormatFloat("0.00", ArmSpeed_File[IndexArm].dCTAirOn);
    EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->ItemIndex        =ArmSpeed_File[IndexArm].bIndexFloatCHK;
    EL<TCheckBox>("TfSpeed", "chkIndexPreSuck")->Checked        =!ArmSpeed_File[IndexArm].bSuckOnDown;
    EL<TCheckBox>("TfSpeed", "chkPickAfterOutArm")->Checked     =TestIF_File.bIndexPickICWhenOutShtNoIC;    //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    EL<TCheckBox>("TfSpeed", "cbEnableDelayTimeZero")->Checked  =TestIF_File.bEnableDelayTimeZero;          //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add “Air purge during place device on out-shuttle” function.
    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    EL<TEdit>("TfSpeed", "edIndexDestroyAgainTime")->Text   =FormatFloat("0.00", ArmSpeed_File[IndexArm].dDestroyAgainTime);
    EL<TEdit>("TfSpeed", "edIndexDestroyAgainCount")->Text  =ArmSpeed_File[IndexArm].iDestroyAgainCount;

    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    //==>
    EL<TCheckBox>("TfSpeed", "chkEnableIndexCycleTimeMonitoring")->Checked      =TestIF_File.bIndexCycleTimeMonitor;
    EL<TEdit>("TfSpeed", "edtMonitoringIndexCycletime")->Text               =TestIF_File.dIndexCycletimeMonitor;
    EL<TEdit>("TfSpeed", "edtMonitoringOutlier")->Text                      =TestIF_File.dMonitorOutlier;
    EL<TEdit>("TfSpeed", "edtMonitoringWindow")->Text                       =TestIF_File.iMonitorWindow;

    EL<TEdit>("TfSpeed", "edtIndexCycleTimetolerance")->Text                =TestIF_File.dICTTolerance;                                         //JerryYang 20220824 : SPIL index cycle time monitor
    EL<TRadioGroup>("TfSpeed", "rgIndexCycleTime")->ItemIndex                     =TestIF_File.iICTAction;    //Steven 20250521 : 可以選擇要不要停機

    EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm")->ItemIndex =(ArmSpeed_File[IndexArm].bDevicConfirm==1)?0:1;                                     //kevin 20190629 add index 4 site conform
    //<==
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    if(CosFunction.bOutArmPickUpErrBySetup)                                     //JerryYang 20220923 : E50改成選項
    {
        EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->ItemIndex=IniConfig.iE50_OutArmPickUpErrorOption;
    }

    //--------------------------------------------------------------------------
    //Input Arm
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->Position              =ArmSpeed_File[InArm].iBodySP;
    EL<TEdit>("TfSpeed", "edInXYSpd")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->Position              =ArmSpeed_File[InArm].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edInXYAcc")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Position              =ArmSpeed_File[InArm].iZSP;
    EL<TEdit>("TfSpeed", "edInZSpd")->Text                  =EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udInZAcc")->Position              =ArmSpeed_File[InArm].iACDCZSP;
    EL<TEdit>("TfSpeed", "edInZAcc")->Text                  =EL<filerw::ELTrackBar>("TfSpeed", "udInZAcc")->Position;

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    EL<TEdit>("TfSpeed", "edInArmDestroyAgainTime")->Text   =FormatFloat("0.00", ArmSpeed_File[InArm].dDestroyAgainTime);
    EL<TEdit>("TfSpeed", "edInArmDestroyAgainCount")->Text  =ArmSpeed_File[InArm].iDestroyAgainCount;
    EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->ItemIndex          =(ArmSpeed_File[InArm].bAutoSpeed==false)?1:0;
    EL<TEdit>("TfSpeed", "edAutoSpeedLow")->Text            = ArmSpeed_File[InArm].iAutoSpeedLow;       //kevin 20210913 AutoSpeed 最低速

    EL<TEdit>("TfSpeed", "edInArmRetryCount")->Text         =ArmSpeed_File[InArm].iRetryCT;
    EL<TEdit>("TfSpeed", "edInArmRetryMM")->Text            =FormatFloat("0.00", ArmSpeed_File[InArm].dRetryDown);
    EL<TEdit>("TfSpeed", "edInVacumCheckTime")->Text        =FormatFloat("0.00", ArmSpeed_File[InArm].dVacuumTI);
    EL<TEdit>("TfSpeed", "edtHPVacuumDelay")->Text          =FormatFloat("0.00", ArmSpeed_File[InArm].dHPVacuumTI);                             //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    EL<TCheckBox>("TfSpeed", "chkUseHPVacuumDelay")->Checked    =ArmSpeed_File[InArm].bUseHPVacuum;         //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    EL<TEdit>("TfSpeed", "edInArmAirOnTime")->Text          =FormatFloat("0.00", ArmSpeed_File[InArm].dCTAirOn);
    EL<TEdit>("TfSpeed", "edInArmShtWaitTime")->Text        =FormatFloat("0.00", ArmSpeed_File[InArm].dWaitOnSH);
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_In")->ItemIndex   =ArmSpeed_File[InArm].iTwoSpeedMove;        //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_In")->Text     =FormatFloat("0.00", ArmSpeed_File[InArm].dTwoSpeedDistance);                       //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edSecondSpeedIn")->Text           =AnsiString(ArmSpeed_File[InArm].iTwoSpeed);                                        //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edSecondADCIn")->Text             =AnsiString(ArmSpeed_File[InArm].iTwoADC);  //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edtAutoSkipCT")->Text             =ArmSpeed_File[InArm].iAutoSkipCT;
    EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->ItemIndex     =(ArmSpeed_File[InArm].bAutoSKIP==1)?0:1;
    EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->ItemIndex         =(ArmSpeed_File[InArm].bVariModeFIX==true)?1:0;                                     //Jimmychiu 20251924 : bVariModeFIX==1 to bVariModeFIX==true
    EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->ItemIndex         =TestIF_File.iInArmToShtReleaseMode;        //Steven 20250630 : 力旺要照順序擺
    EL<TCheckBox>("TfSpeed", "chkTwoSpeedOnlyLoader")->Checked  =ArmSpeed_File[InArm].bTwoSpeedOnlyLoader;  //JerryYang 20190729 二段速功能可選擇only at loader

    EL<TCheckBox>("TfSpeed", "chkInArmPreSuck")->Checked        =!ArmSpeed_File[InArm].bSuckOnDown;
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchSpd")->Position          =ArmSpeed_File[InArm].iVariSP;
    EL<TEdit>("TfSpeed", "edInPitchSpd")->Text              =EL<filerw::ELTrackBar>("TfSpeed", "udInPitchSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udInPitchAcc")->Position          =ArmSpeed_File[InArm].iACDCVariSP;
    EL<TEdit>("TfSpeed", "edInPitchAcc")->Text              =EL<filerw::ELTrackBar>("TfSpeed", "udInPitchAcc")->Position;

    //Steven 20140428 : For Rotate
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->Position            =ArmSpeed_File[InRotate].iBodySP;
    EL<TEdit>("TfSpeed", "edInRotSpd")->Text                =EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->Position            =ArmSpeed_File[InRotate].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edInRotAcc")->Text                =EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->Position;

    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorOpenSp")->Position      =ArmSpeed_File[InArm].PrecisorOpenSp;       //JerryYang 20250120 : add
    EL<TEdit>("TfSpeed", "edPrecisorOpenSp")->Text          =EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorOpenSp")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorCloseSp")->Position     =ArmSpeed_File[InArm].PrecisorCloseSp;
    EL<TEdit>("TfSpeed", "edPrecisorCloseSp")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "udPrecisorCloseSp")->Position;

    //2013-08-01    Dell    modify  公司統一回吸功能
    EL<TEdit>("TfSpeed", "Edt_CheckTime_In")->Text          =FormatFloat("0.0", ArmSpeed_File[InArm].dDestroyCheckTime);
    EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_In")->Checked      =ArmSpeed_File[InArm].bDestroyPauseCheck;
    EL<TEdit>("TfSpeed", "edRelaseDelay")->Text             =FormatFloat("0.00", ArmSpeed_File[InArm].dReleaseDelayTime);                       //JerryYang 20160127 for TSMC inarm release device前delay
    EL<TRadioGroup>("TfSpeed", "rgReleaseDelayTime")->ItemIndex   =ArmSpeed_File[InArm].iEnableReleaseDelay;  //JerryYang 20160127 for TSMC inarm release device前delay

    EL<TEdit>("TfSpeed", "edInArmDieCleanDelay")->Text      =FormatFloat("0.00", ArmSpeed_File[InArm].dDieCleanDelay);                          //wei 20170418
    EL<TRadioGroup>("TfSpeed", "rgInArmDieClean")->ItemIndex      =(ArmSpeed_File[InArm].iEnableDieClean==1)?0:1;                                     //wei 20170418
    EL<TEdit>("TfSpeed", "edInArmDieCleanHeight")->Text     =FormatFloat("0.00", ArmSpeed_File[InArm].dDieCleanHeight);                         //wei 20170418
    EL<TCheckBox>("TfSpeed", "ckb_HeightCheck_In")->Checked     =ArmSpeed_File[InArm].bHeightCheck;         //wei 20160825 回吸偵測
    EL<TEdit>("TfSpeed", "Edt_HeightCheck_In")->Text        =ArmSpeed_File[InArm].iHeightCheck;         //wei 20160825 回吸偵測
    EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm")->ItemIndex =(ArmSpeed_File[InArm].bDevicConfirm==1)?0:1;                                       //kevin 20170510 (wei) 回吸功能開關內定開啟
    EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch")->ItemIndex=(ArmSpeed_File[InArm].bYPitchUseSearchLastMode==0)?1:0;

    EL<TEdit>("TfSpeed", "edInArmCylinderDelay")->Text      =FormatFloat("0.00", ArmSpeed_File[InArm].dCylinderDelay);
    EL<TRadioGroup>("TfSpeed", "rgTT_SetSpeed")->ItemIndex        =(ArmSpeed_File[InArm].bTT_SetSpeed==false)?1:0;                                    //Ztex 2024.08.11 Add Test Time Set Speed
    //--------------------------------------------------------------------------
    //Output Arm
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Position             =ArmSpeed_File[OutArm].iBodySP;
    EL<TEdit>("TfSpeed", "edOutXSpd")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutXAcc")->Position             =ArmSpeed_File[OutArm].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edOutXAcc")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udOutXAcc")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->Position             =ArmSpeed_File[OutArm].iZSP;
    EL<TEdit>("TfSpeed", "edOutZSpd")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->Position             =ArmSpeed_File[OutArm].iACDCZSP;
    EL<TEdit>("TfSpeed", "edOutZAcc")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->Position;

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    EL<TEdit>("TfSpeed", "edOutArmDestroyAgainTime")->Text  =FormatFloat("0.00", ArmSpeed_File[OutArm].dDestroyAgainTime);
    EL<TEdit>("TfSpeed", "edOutArmDestroyAgainCount")->Text =FormatFloat("0", ArmSpeed_File[OutArm].iDestroyAgainCount);
    EL<TEdit>("TfSpeed", "edOutArmRetryCount")->Text        =FormatFloat("0", ArmSpeed_File[OutArm].iRetryCT) ;
    EL<TEdit>("TfSpeed", "edOutArmRetryMM")->Text           =FormatFloat("0.00", ArmSpeed_File[OutArm].dRetryDown);
    EL<TEdit>("TfSpeed", "edOutVacumCheckTime")->Text       =FormatFloat("0.00", ArmSpeed_File[OutArm].dVacuumTI);
    EL<TEdit>("TfSpeed", "edOutArmShtWaitTime")->Text       =FormatFloat("0.00", ArmSpeed_File[OutArm].dWaitOnSH);                              //kevin 20210525 add out arm 等待時間到再往下 Shuttle 吸取 IC
    EL<TEdit>("TfSpeed", "edOutArmAirOnTime")->Text         =FormatFloat("0.00", ArmSpeed_File[OutArm].dCTAirOn);
    EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->ItemIndex        =(ArmSpeed_File[OutArm].bVariModeFIX==0)?0:1;
    EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_Out")->ItemIndex  =ArmSpeed_File[OutArm].iTwoSpeedMove;       //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edTwoSpeedDistance_Out")->Text    =FormatFloat("0.00", ArmSpeed_File[OutArm].dTwoSpeedDistance);                      //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edSecondSpeedOut")->Text          =AnsiString(ArmSpeed_File[OutArm].iTwoSpeed);                                       //Steven 20140217 : 兩段速移動
    EL<TEdit>("TfSpeed", "edSecondADCOut")->Text            =AnsiString(ArmSpeed_File[OutArm].iTwoADC);                                         //Steven 20140217 : 兩段速移動

    EL<TCheckBox>("TfSpeed", "chkOutArmPreSuck")->Checked       =!ArmSpeed_File[OutArm].bSuckOnDown;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchSpd")->Position         =ArmSpeed_File[OutArm].iVariSP;
    EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text             =EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchAcc")->Position         =ArmSpeed_File[OutArm].iACDCVariSP;
    EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text             =EL<filerw::ELTrackBar>("TfSpeed", "udOutPitchAcc")->Position;
    EL<TEdit>("TfSpeed", "edtOutArmPreSuck")->Text          =FormatFloat("0.00", ArmSpeed_File[OutArm].dDestroyPauseTime);
    EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm")->ItemIndex=(ArmSpeed_File[OutArm].bDevicConfirm==1)?0:1;                                      //kevin 20170510 (wei) 回吸功能開關
    EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch")->ItemIndex=(ArmSpeed_File[OutArm].bYPitchUseSearchLastMode==0)?1:0;                       //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格

    //Steven 20140428 : For Rotate
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->Position           =ArmSpeed_File[OutRotate].iBodySP;
    EL<TEdit>("TfSpeed", "edOutRotSpd")->Text               =EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->Position           =ArmSpeed_File[OutRotate].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edOutRotAcc")->Text               =EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->Position;

    //2013-08-01    Dell    modify  公司統一回吸功能
    EL<TEdit>("TfSpeed", "Edt_CheckTime_Out")->Text         =FormatFloat("0.0", ArmSpeed_File[OutArm].dDestroyCheckTime);
    EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_Out")->Checked     =ArmSpeed_File[OutArm].bDestroyPauseCheck;

    EL<TEdit>("TfSpeed", "edOutArmDieCleanDelay")->Text      =FormatFloat("0.00", ArmSpeed_File[OutArm].dDieCleanDelay);                        //wei 20170418
    EL<TRadioGroup>("TfSpeed", "rgOutArmDieClean")->ItemIndex      =(ArmSpeed_File[OutArm].iEnableDieClean==0)?1:0;                                   //wei 20170418

    EL<TEdit>("TfSpeed", "edOutArmCylinderDelay")->Text      =FormatFloat("0.00", ArmSpeed_File[OutArm].dCylinderDelay);
    //--------------------------------------------------------------------------
    //Empty Tray Arm
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->Position            =ArmSpeed_File[TrayArm].iBodySP;
    EL<TEdit>("TfSpeed", "edTrayXSpd")->Text                =EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->Position            =ArmSpeed_File[TrayArm].iACDCBodySP;
    EL<TEdit>("TfSpeed", "edTrayXAcc")->Text                =EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->Position;
    EL<TEdit>("TfSpeed", "edTrayArmRetryCount")->Text       =ArmSpeed_File[TrayArm].iRetryCT;
    EL<TEdit>("TfSpeed", "edTrayVacumCheckTime")->Text      =FormatFloat("0.00", ArmSpeed_File[TrayArm].dVacuumTI);
    EL<TEdit>("TfSpeed", "edTrayArmAirOnTime")->Text        =FormatFloat("0.00", ArmSpeed_File[TrayArm].dCTAirOn);
    EL<TEdit>("TfSpeed", "edTrayArmHandDown")->Text         =FormatFloat("0.00", ArmSpeed_File[TrayArm].dTAHeadDownIT);
    //--------------------------------------------------------------------------
    //Shuttle
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->Position             =SHSpeed_File.iSH1Sp;
    EL<TEdit>("TfSpeed", "edSht1Spd")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->Position             =SHSpeed_File.iSH1ACDCSp;
    EL<TEdit>("TfSpeed", "edSht1Acc")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->Position             =SHSpeed_File.iSH2Sp;
    EL<TEdit>("TfSpeed", "edSht2Spd")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->Position             =SHSpeed_File.iSH2ACDCSp;
    EL<TEdit>("TfSpeed", "edSht2Acc")->Text                 =EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->Position;
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->ItemIndex        =(SHSpeed_File.iStepSH==1)?0:1;
    EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->ItemIndex       =TestIF_File.iShakeShuttleWhenPlaceIC;      //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭  //Steven 20220427 : 每次放料都要抖抖馬達
    EL<TEdit>("TfSpeed", "edShakeCycles")->Text             =SHSpeed_File.iShakeCycles;                 //Sam 20250326 : 新增 Shake 條件設定
    EL<TEdit>("TfSpeed", "edShakeDistance")->Text           =SHSpeed_File.iShakeDistance;
    EL<TEdit>("TfSpeed", "edShakeDelay")->Text              =SHSpeed_File.dShakeDelay;
    EL<TEdit>("TfSpeed", "edShakeAccDec")->Text             =SHSpeed_File.iShakeAccDec;
    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYSpd")->Position        =MGSpeed_File.iCYSp;
    EL<TEdit>("TfSpeed", "edMagCatchYSpd")->Text            =EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYAcc")->Position        =MGSpeed_File.iCYACDCSp;
    EL<TEdit>("TfSpeed", "edMagCatchYAcc")->Text            =EL<filerw::ELTrackBar>("TfSpeed", "udMagCatchYAcc")->Position;

    EL<filerw::ELTrackBar>("TfSpeed", "udMagZSpd")->Position              =MGSpeed_File.iMZSp;
    EL<TEdit>("TfSpeed", "edMagZSpd")->Text                  =EL<filerw::ELTrackBar>("TfSpeed", "udMagZSpd")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "udMagZAcc")->Position              =MGSpeed_File.iMZACDCSp;
    EL<TEdit>("TfSpeed", "edMagZAcc")->Text                  =EL<filerw::ELTrackBar>("TfSpeed", "udMagZAcc")->Position;

    EL<TEdit>("TfSpeed", "edTwoSpeedDistanceCatchY")->Text =FormatFloat("0.00", MGSpeed_File.dCYDecRange);
    EL<TEdit>("TfSpeed", "edSecondSpeedCatchY")->Text        =AnsiString(MGSpeed_File.iCY2Sp);
    EL<TEdit>("TfSpeed", "edSecondADCCatchY")->Text          =AnsiString(MGSpeed_File.iCY2ACDCSp);

    //Steven 20170317 (wei) mark
    //==>
//    cbOutPickErrAct->Checked        =TestIF_File.bOutPickErrAct;              //Steven 20170202 : For ATJ Out Arm pickup error put to define tray
//    cobOutPickErrAct->ItemIndex     =TestIF_File.iOutPickErrAuto;
    //<==
    //Steven 20170317 (wei) mark

}

// golden cSpeed.cpp:1282  TfSpeed::tbAllSpeedChange(TObject *Sender)
static void SP_tbAllSpeedChange()
{

    EL<TEdit>("TfSpeed", "edAllSpeed")->Text            =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->SelEnd          =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    if(EL<TCheckBox>("TfSpeed", "cbIndexArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edIndexSpeed")->Text      =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udIndexSpd")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edInXYSpd")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<TEdit>("TfSpeed", "edInPitchSpd")->Text      =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;                          //Steven 20210608 : 100 --> tbAllSpeed->Position
        EL<filerw::ELTrackBar>("TfSpeed", "udInXSpd")->Position      =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edOutXSpd")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text     =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;                          //Steven 20210608 : 100 --> tbAllSpeed->Position
        EL<filerw::ELTrackBar>("TfSpeed", "udOutXSpd")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbShuttle")->Checked)
    {
        EL<TEdit>("TfSpeed", "edSht1Spd")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<TEdit>("TfSpeed", "edSht2Spd")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;                          //jou 2016-09-01 edSht1Spd -> edSht2Spd fix all speed shuttle2 error
        EL<filerw::ELTrackBar>("TfSpeed", "udSht1Spd")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht2Spd")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbTrayArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edTrayXSpd")->Text        =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udTrayXSpd")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInArmZ")->Checked)
    {
        EL<TEdit>("TfSpeed", "edInZSpd")->Text          =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udInZSpd")->Position      =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutArmZ")->Checked)
    {
        EL<TEdit>("TfSpeed", "edOutZSpd")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutZSpd")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInRotate")->Checked)                                                     //kevin 20130517 rotate
    {
        EL<TEdit>("TfSpeed", "edInRotSpd")->Text        =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udInRotSpd")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutRotate")->Checked)                                                    //kevin 20130517 rotate
    {
        EL<TEdit>("TfSpeed", "edOutRotSpd")->Text       =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutRotSpd")->Position   =EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position;
    }

}

// golden cSpeed.cpp:1345  TfSpeed::tbAccSpeedChange(TObject *Sender)
static void SP_tbAccSpeedChange()
{

    EL<TEdit>("TfSpeed", "edAllAccSpeed")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->SelEnd          =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    if(EL<TCheckBox>("TfSpeed", "cbIndexArm")->Checked)
    {
        //edIndexAccDec->Text   =tbAccSpeed->Position;                          //Steven 20091214 Mark for Index ACC & DEC must be 100%
        EL<filerw::ELTrackBar>("TfSpeed", "udIndexAcc")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edInXYAcc")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<TEdit>("TfSpeed", "edInPitchAcc")->Text      =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;                          //Steven 20210608 : 100 --> tbAllSpeed->Position
        EL<filerw::ELTrackBar>("TfSpeed", "udInXAcc")->Position      =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edOutXAcc")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;                          //Steven 20210608 : 100 --> tbAllSpeed->Position
        EL<filerw::ELTrackBar>("TfSpeed", "udOutXAcc")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbShuttle")->Checked)
    {
        EL<TEdit>("TfSpeed", "edSht1Acc")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<TEdit>("TfSpeed", "edSht2Acc")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht1Acc")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udSht2Acc")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbTrayArm")->Checked)
    {
        EL<TEdit>("TfSpeed", "edTrayXAcc")->Text        =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udTrayXAcc")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInArmZ")->Checked)
    {
        EL<TEdit>("TfSpeed", "edInZAcc")->Text          =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udInZAcc")->Position      =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutArmZ")->Checked)
    {
        EL<TEdit>("TfSpeed", "edOutZAcc")->Text         =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutZAcc")->Position     =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbInRotate")->Checked)                                                     //kevin 20130517 rotate
    {
        EL<TEdit>("TfSpeed", "edInRotAcc")->Text        =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udInRotAcc")->Position    =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

    if(EL<TCheckBox>("TfSpeed", "cbOutRotate")->Checked)                                                    //kevin 20130517 rotate
    {
        EL<TEdit>("TfSpeed", "edOutRotAcc")->Text       =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
        EL<filerw::ELTrackBar>("TfSpeed", "udOutRotAcc")->Position   =EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position;
    }

}

// golden cSpeed.cpp:1408  TfSpeed::tbEPControlChange(TObject *Sender)
static void SP_tbEPControlChange()
{

    EL<TEdit>("TfSpeed", "edEPControl")->Text  =EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->Position;
    EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->SelEnd=EL<filerw::ELTrackBar>("TfSpeed", "tbEPControl")->Position;

}

// golden cSpeed.cpp:1272  TfSpeed::FormClose(TObject *Sender, TCloseAction &Action)
static void SP_FormClose()
{

    fShow=false;
    SP_ReadFile();
    SP_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cSpeed.cpp:1433  TfSpeed::spbSaveClick(TObject *Sender)
static void SP_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szDir="";
    if(CosFunction.bSaveArmSpeedByMachine && IniConfig.bA57_1SaveArmSpeedByMachine)                                     //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s", DataPath, LastFileName);
    }
    MyForceDirectories(szDir);
    szDir+="\\ArmCondition.Data";
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Text=100;                                                    //Steven 20091214 Add for Index ACC & DEC must be 100%

    //jou 2010-09-10 start : 增加保護speed In/Out Arm 吸嘴Retry高度不應該是正的值
    EL<TEdit>("TfSpeed", "edInArmRetryMM")->Text =AnsiString(CheckRange(atof(EL<TEdit>("TfSpeed", "edInArmRetryMM")->Text.c_str()),  -0.01, -0.3));
    EL<TEdit>("TfSpeed", "edOutArmRetryMM")->Text=AnsiString(CheckRange(atof(EL<TEdit>("TfSpeed", "edOutArmRetryMM")->Text.c_str()), -0.01, -0.3));
    //jou 2010-09-10 end

    WriteIniData(szDir, "All", "Speed",     EL<TEdit>("TfSpeed", "edAllSpeed")->Text);
    WriteIniData(szDir, "All", "Accel",     EL<TEdit>("TfSpeed", "edAllAccSpeed")->Text);
    WriteIniData(szDir, "All", "EPControl", EL<TEdit>("TfSpeed", "edEPControl")->Text);                 //Ifor 20150710 :EP Control

    //--------------------------------------------------------------------------
    //Index Arm
    //--------------------------------------------------------------------------
    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20240913 ASE 新加坡客戶要求 X => Z
    {
        WriteIniData(szDir, "Index Arm", "Y Speed",             EL<TEdit>("TfSpeed", "edIndexSpeed")->Text);
        WriteIniData(szDir, "Index Arm", "Y Accel",             EL<TEdit>("TfSpeed", "edIndexAccDec")->Text);
    }
    else
    {
        WriteIniData(szDir, "Index Arm", "X Speed",             EL<TEdit>("TfSpeed", "edIndexSpeed")->Text);
        WriteIniData(szDir, "Index Arm", "X Accel",             EL<TEdit>("TfSpeed", "edIndexAccDec")->Text);
    }
    WriteIniData(szDir, "Index Arm", "Z Up Speed",              EL<TEdit>("TfSpeed", "edIndexSpeed")->Text);
    WriteIniData(szDir, "Index Arm", "Z Up Accel",              EL<TEdit>("TfSpeed", "edIndexAccDec")->Text);
    WriteIniData(szDir, "Index Arm", "Z Down Speed",            EL<TEdit>("TfSpeed", "edIndexSpeed")->Text);
    WriteIniData(szDir, "Index Arm", "Z Down Accel",            EL<TEdit>("TfSpeed", "edIndexAccDec")->Text);
    WriteIniData(szDir, "Index Arm", "Retry Count",             EL<TEdit>("TfSpeed", "edIndexRetryCount")->Text);
    WriteIniData(szDir, "Index Arm", "Retry Down",              EL<TEdit>("TfSpeed", "edIndexArmRetryMM")->Text);
    WriteIniData(szDir, "Index Arm", "Vacuum Check Time",       EL<TEdit>("TfSpeed", "edIndexVacumCheckTime")->Text);
    WriteIniData(szDir, "Index Arm", "Counter Air ON Time",     EL<TEdit>("TfSpeed", "edIndexAirOnTime")->Text);
    WriteIniData(szDir, "Output Arm", "Shuttle Wait Time",      EL<TEdit>("TfSpeed", "edOutArmShtWaitTime")->Text);                             //kevin 20210525 out arm 在SHUTTLE 上等待 時間再往下吸IC

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    WriteIniData(szDir, "Index Arm", "Destroy Again Time",      EL<TEdit>("TfSpeed", "edIndexDestroyAgainTime")->Text);
    WriteIniData(szDir, "Index Arm", "Destroy Again Count",     EL<TEdit>("TfSpeed", "edIndexDestroyAgainCount")->Text);

    WriteIniData(szDir, "Index Arm", "Vacuum Check Optimize",   (EL<TCheckBox>("TfSpeed", "cbIndexVacuumCheckOptimize")->Checked)?1:0);
    WriteIniData(szDir, "Index Arm", "Socket Check",            EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->ItemIndex);
    WriteIniData(szDir, "Index Arm", "Vacuum Timing",           (EL<TCheckBox>("TfSpeed", "chkIndexPreSuck")->Checked)?0:1);
    WriteIniData(szDir, "Index Arm", "Pick IC when out shuttle no IC", (EL<TCheckBox>("TfSpeed", "chkPickAfterOutArm")->Checked)?1:0);              //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    WriteIniData(szDir, "Index Arm", "Enable Delay Time Zero",  (EL<TCheckBox>("TfSpeed", "cbEnableDelayTimeZero")->Checked)?1:0);                  //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add “Air purge during place device on out-shuttle” function.

    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    //==>
    if(IniConfig.bSPILFunction)                                                 //JerryYang 20220824 : SPIL index cycle time monitor
    {
        WriteIniData(szDir, "Index Arm", "EnableNewIndexCycleTimeMonitoring",  (EL<TCheckBox>("TfSpeed", "chkEnableIndexCycleTimeMonitoring")->Checked)?1:0);                               //Isaac 20180301
    }
    else
    {
        WriteIniData(szDir, "Index Arm", "EnableIndexCycleTimeMonitoring",  (EL<TCheckBox>("TfSpeed", "chkEnableIndexCycleTimeMonitoring")->Checked)?1:0);                                  //Isaac 20180301
    }

    WriteIniData(szDir, "Index Arm", "Monitoring_IndexCycletime",       EL<TEdit>("TfSpeed", "edtMonitoringIndexCycletime")->Text);
    WriteIniData(szDir, "Index Arm", "Monitoring_Outlier",              EL<TEdit>("TfSpeed", "edtMonitoringOutlier")->Text);
    WriteIniData(szDir, "Index Arm", "Monitoring_Window",               EL<TEdit>("TfSpeed", "edtMonitoringWindow")->Text);
    WriteIniData(szDir, "Index Arm", "IndexCycleTimetolerance",         EL<TEdit>("TfSpeed", "edtIndexCycleTimetolerance")->Text);              //JerryYang 20220824 : SPIL index cycle time monitor
    WriteIniData(szDir, "Index Arm", "DevicConfirm",           (EL<TRadioGroup>("TfSpeed", "gbIndexeviveConfirm")->ItemIndex==0)?1:0);                //kevin 20190629 add index 4 site conform
    WriteIniData(szDir, "Index Arm", "IndexCycleTimeAction",   EL<TRadioGroup>("TfSpeed", "rgIndexCycleTime")->ItemIndex);                            //Steven 20250521 : 可以選擇要不要停機

    if(CosFunction.bOutArmPickUpErrBySetup==true)                               //JerryYang 20220923 : E50改成選項
    {
        WriteIniData(szDir, "Output Arm", "iE50_OutArmPickUpErrorOption",           EL<TRadioGroup>("TfSpeed", "rgOutArmPickupErrMode")->ItemIndex);  //kevin 20190629 add index 4 site conform
    }

    if(TestIF_File.bIndexCycleTimeMonitor)
    {
        if(IniConfig.bSPILFunction)                                             //JerryYang 20220824 : SPIL index cycle time monitor
        {
        }
        else
        {
            IniRecordMonitoringIndexCycleTime();
        }
        //        IniRecordMonitoringIndexCycleTime();
    ;   // golden cSpeed.cpp:1523-1526 取代：MyMessageBox->Close()：訊息框（伺服器端沒有）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1523-1526 -- MyMessageBox->Close()：訊息框（伺服器端沒有）
        if(MyMessageBox->Visible==true)                                         //Alarm 前若form有開啟先關閉
        {
            MyMessageBox->Close();
        }
#endif // GATE (S12-C save)
        bResetflag=true;
    }
    //<==
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function

    //--------------------------------------------------------------------------
    //Input Arm
    //--------------------------------------------------------------------------
    WriteIniData(szDir, "Input Arm", "XY Speed",                EL<TEdit>("TfSpeed", "edInXYSpd")->Text);
    WriteIniData(szDir, "Input Arm", "XY Accel",                EL<TEdit>("TfSpeed", "edInXYAcc")->Text);

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    WriteIniData(szDir, "Input Arm", "Destroy Again Time",      EL<TEdit>("TfSpeed", "edInArmDestroyAgainTime")->Text);
    WriteIniData(szDir, "Input Arm", "Destroy Again Count",     EL<TEdit>("TfSpeed", "edInArmDestroyAgainCount")->Text);

    //ChungHung 20110408 Z Speed can change start
//    if(atoi(edInZSpd->Text.c_str())>atoi(edInXYSpd->Text.c_str()) || atoi(edInZSpd->Text.c_str())>atoi(edInPitchSpd->Text.c_str()))
//    {
//        edInZSpd->Text=min(atoi(edInXYSpd->Text.c_str()), atoi(edInPitchSpd->Text.c_str()));
//    }
//    if(atoi(edInZAcc->Text.c_str())>atoi(edInXYAcc->Text.c_str()) || atoi(edInZAcc->Text.c_str())>atoi(edInPitchAcc->Text.c_str()))
//    {
//        edInZAcc->Text=min(atoi(edInXYAcc->Text.c_str()), atoi(edInPitchAcc->Text.c_str()));
//    }
    //ChungHung 20110408 Z Speed can change end
    WriteIniData(szDir, "Input Arm", "Z Up Speed",              EL<TEdit>("TfSpeed", "edInZSpd")->Text);
    WriteIniData(szDir, "Input Arm", "Z Up Accel",              EL<TEdit>("TfSpeed", "edInZAcc")->Text);
    WriteIniData(szDir, "Input Arm", "Z Down Speed",            EL<TEdit>("TfSpeed", "edInZSpd")->Text);
    WriteIniData(szDir, "Input Arm", "Z Down Accel",            EL<TEdit>("TfSpeed", "edInZAcc")->Text);
    WriteIniData(szDir, "Input Arm", "Auto Speed Down",         (EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->ItemIndex==0)?1:0);
    WriteIniData(szDir, "Input Arm", "AutoSpeedLow",            EL<TEdit>("TfSpeed", "edAutoSpeedLow")->Text);                                  //kevin 20210913 add low speed
    WriteIniData(szDir, "Input Arm", "Retry Count",             EL<TEdit>("TfSpeed", "edInArmRetryCount")->Text);
    WriteIniData(szDir, "Input Arm", "Retry Down",              EL<TEdit>("TfSpeed", "edInArmRetryMM")->Text);
    WriteIniData(szDir, "Input Arm", "Vacuum Check Time",       EL<TEdit>("TfSpeed", "edInVacumCheckTime")->Text);
    WriteIniData(szDir, "Input Arm", "HP Vacuum Check Time",    EL<TEdit>("TfSpeed", "edtHPVacuumDelay")->Text);                                //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    WriteIniData(szDir, "Input Arm", "Use HP Vacuum Check Time",  EL<TCheckBox>("TfSpeed", "chkUseHPVacuumDelay")->Checked);                        //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    WriteIniData(szDir, "Input Arm", "Counter Air ON Time",     EL<TEdit>("TfSpeed", "edInArmAirOnTime")->Text);
    WriteIniData(szDir, "Input Arm", "Shuttle Wait Time",       EL<TEdit>("TfSpeed", "edInArmShtWaitTime")->Text);
    WriteIniData(szDir, "Input Arm", "Two Speed On Off",        EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_In")->ItemIndex);                         //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Input Arm", "Two Speed Distance",      EL<TEdit>("TfSpeed", "edTwoSpeedDistance_In")->Text);                           //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Input Arm", "Two Speed Precent",       EL<TEdit>("TfSpeed", "edSecondSpeedIn")->Text);                                 //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Input Arm", "Two Speed ADC",           EL<TEdit>("TfSpeed", "edSecondADCIn")->Text);                                   //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Input Arm", "Two Speed Only Loader",   EL<TCheckBox>("TfSpeed", "chkTwoSpeedOnlyLoader")->Checked);                        //JerryYang 20190729 二段速功能可選擇only at loader

    //auto Skip enable
    WriteIniData(szDir, "Input Arm", "Auto Skip",               (EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->ItemIndex==0)?1:0);
    WriteIniData(szDir, "Input Arm", "Auto Skip CT",            EL<TEdit>("TfSpeed", "edtAutoSkipCT")->Text);

    //One by One Enable
    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ;
    }
    else
    {
        WriteIniData(szDir, "Input Arm", "One by one",          (EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->ItemIndex==0)?0:1);
    }

    WriteIniData(szDir, "Input Arm", "iInArmToShtReleaseMode",  EL<TRadioGroup>("TfSpeed", "rgInShtOrder")->ItemIndex);                               //Steven 20250630 : 力旺要照順序擺
    WriteIniData(szDir, "Input Arm", "Vacuum Timing",           (EL<TCheckBox>("TfSpeed", "chkInArmPreSuck")->Checked)?0:1);
    WriteIniData(szDir, "Input Arm", "Vacuum Check Optimize",   (EL<TCheckBox>("TfSpeed", "cbInVacuumCheckOptimize")->Checked)?1:0);
    WriteIniData(szDir, "Input Arm", "Open/Close Speed",        EL<TEdit>("TfSpeed", "edInPitchSpd")->Text);
    WriteIniData(szDir, "Input Arm", "Open/Close Accel",        EL<TEdit>("TfSpeed", "edInPitchAcc")->Text);

    WriteIniData(szDir, "Input Arm", "Rotate Input Speed",      EL<TEdit>("TfSpeed", "edInRotSpd")->Text);                                      //kevin 20130517 rotate add
    WriteIniData(szDir, "Input Arm", "Rotate Input Accel",      EL<TEdit>("TfSpeed", "edInRotAcc")->Text);                                      //kevin 20130517 rotate add

    WriteIniData(szDir, "Input Arm", "PrecisorOpenSpeed",      EL<TEdit>("TfSpeed", "edPrecisorOpenSp")->Text);                                 //JerryYang 20250120 : add
    WriteIniData(szDir, "Input Arm", "PrecisorCloseSpeed",      EL<TEdit>("TfSpeed", "edPrecisorCloseSp")->Text);

    //2013-08-01    Dell    modify  公司統一回吸功能
    WriteIniData(szDir, "Input Arm", "Destroy Check Time",      EL<TEdit>("TfSpeed", "Edt_CheckTime_In")->Text);
    WriteIniData(szDir, "Input Arm", "Destroy Pause Check",     (EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_In")->Checked)?1:0);
    WriteIniData(szDir, "Input Arm", "Release Delay Time",      EL<TEdit>("TfSpeed", "edRelaseDelay")->Text);                                   //JerryYang 20160127 for TSMC inarm release device前delay
    WriteIniData(szDir, "Input Arm", "Enable Relase Delay",     EL<TRadioGroup>("TfSpeed", "rgReleaseDelayTime")->ItemIndex);                         //JerryYang 20160127 for TSMC inarm release device前delay
    WriteIniData(szDir, "Input Arm", "Enable Height Check",    (EL<TCheckBox>("TfSpeed", "ckb_HeightCheck_In")->Checked)?1:0);                      //wei 20160825 回吸偵測
    WriteIniData(szDir, "Input Arm", "Height Check",            EL<TEdit>("TfSpeed", "Edt_HeightCheck_In")->Text);                              //wei 20160825 回吸偵測
    WriteIniData(szDir, "Input Arm", "DevicConfirm",           (EL<TRadioGroup>("TfSpeed", "gbInarmDeviveConfirm")->ItemIndex==0)?1:0);               //kevin 20170510 (wei) 回吸偵測開關
    WriteIniData(szDir, "Input Arm", "bYPitchNotUseSearchLastMode", (EL<TRadioGroup>("TfSpeed", "rgSearchInarmLastModeForYPitch")->ItemIndex==0)?1:0);                                        //kevin 20211130 add Ypitch 要求修改unloader擺放方式，盡量不要有空格

    WriteIniData(szDir, "Input Arm", "Cylinder Delay",          EL<TEdit>("TfSpeed", "edInArmCylinderDelay")->Text);
    if(USE_DIE_CLEAN)
    {
        WriteIniData(szDir, "Input Arm", "Enable Die Clean",    (EL<TRadioGroup>("TfSpeed", "rgInArmDieClean")->ItemIndex==0)?1:0);                   //wei 20170418
        WriteIniData(szDir, "Input Arm", "Die Clean Delay",     EL<TEdit>("TfSpeed", "edInArmDieCleanDelay")->Text);                            //wei 20170418
        WriteIniData(szDir, "Input Arm", "Die Clean Height",    EL<TEdit>("TfSpeed", "edInArmDieCleanHeight")->Text);                           //wei 20170418
    }
    WriteIniData(szDir, "Input Arm", "Test Time Set Speed",     (EL<TRadioGroup>("TfSpeed", "rgTT_SetSpeed")->ItemIndex==0)?1:0);                     //Ztex 2024.08.11 Add Test Time Set Speed
    //--------------------------------------------------------------------------
    //Output Arm
    //--------------------------------------------------------------------------
    WriteIniData(szDir, "Output Arm", "XY Speed",               EL<TEdit>("TfSpeed", "edOutXSpd")->Text);
    WriteIniData(szDir, "Output Arm", "XY Accel",               EL<TEdit>("TfSpeed", "edOutXAcc")->Text);

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    WriteIniData(szDir, "Output Arm", "Destroy Again Time",     EL<TEdit>("TfSpeed", "edOutArmDestroyAgainTime")->Text);
    WriteIniData(szDir, "Output Arm", "Destroy Again Count",    EL<TEdit>("TfSpeed", "edOutArmDestroyAgainCount")->Text);

    //ChungHung 20110408 Z Speed can change start
    /*
    if(atoi(EL<TEdit>("TfSpeed", "edOutZSpd")->Text.c_str())>atoi(EL<TEdit>("TfSpeed", "edOutXSpd")->Text.c_str()) || atoi(EL<TEdit>("TfSpeed", "edOutZSpd")->Text.c_str())>atoi(EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text.c_str()))
    {
        EL<TEdit>("TfSpeed", "edOutZSpd")->Text = min( atoi(EL<TEdit>("TfSpeed", "edOutXSpd")->Text.c_str()), atoi(EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text.c_str()));
    }

    if(atoi(EL<TEdit>("TfSpeed", "edOutZAcc")->Text.c_str())>atoi(EL<TEdit>("TfSpeed", "edOutXAcc")->Text.c_str()) || atoi(EL<TEdit>("TfSpeed", "edOutZAcc")->Text.c_str())>atoi(EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text.c_str()))
    {
        EL<TEdit>("TfSpeed", "edOutZAcc")->Text = min(atoi(EL<TEdit>("TfSpeed", "edOutXAcc")->Text.c_str()), atoi(EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text.c_str()));
    } */                                                                         //kevin 20170822 (Steven) 不卡 最大值
    //ChungHung 20110408 Z Speed can change start
    WriteIniData(szDir, "Output Arm", "Z Up Speed",             EL<TEdit>("TfSpeed", "edOutZSpd")->Text);
    WriteIniData(szDir, "Output Arm", "Z Up Accel",             EL<TEdit>("TfSpeed", "edOutZAcc")->Text);
    WriteIniData(szDir, "Output Arm", "Z Down Speed",           EL<TEdit>("TfSpeed", "edOutZSpd")->Text);
    WriteIniData(szDir, "Output Arm", "Z Down Accel",           EL<TEdit>("TfSpeed", "edOutZAcc")->Text);
    WriteIniData(szDir, "Output Arm", "Auto Speed Down",        (EL<TRadioGroup>("TfSpeed", "rgAutoSpeed")->ItemIndex==0)?1:0);
    WriteIniData(szDir, "Output Arm", "AutoSpeedLow",            EL<TEdit>("TfSpeed", "edAutoSpeedLow")->Text);                                 //kevin 20210913 add low speed
    WriteIniData(szDir, "Output Arm", "Retry Count",            EL<TEdit>("TfSpeed", "edOutArmRetryCount")->Text);
    WriteIniData(szDir, "Output Arm", "Retry Down",             EL<TEdit>("TfSpeed", "edOutArmRetryMM")->Text);
    WriteIniData(szDir, "Output Arm", "Vacuum Check Time",      EL<TEdit>("TfSpeed", "edOutVacumCheckTime")->Text);
    WriteIniData(szDir, "Output Arm", "Counter Air ON Time",    EL<TEdit>("TfSpeed", "edOutArmAirOnTime")->Text);
    WriteIniData(szDir, "Output Arm", "Shuttle Wait Time",      EL<TEdit>("TfSpeed", "edOutArmShtWaitTime")->Text);                             //kevin 20210525 out arm 在SHUTTLE 上等待 時間再往下吸IC
    WriteIniData(szDir, "Output Arm", "Two Speed On Off",       EL<TRadioGroup>("TfSpeed", "rgTwoSpeedOnOff_Out")->ItemIndex);                        //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Output Arm", "Two Speed Distance",     EL<TEdit>("TfSpeed", "edTwoSpeedDistance_Out")->Text);                          //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Output Arm", "Two Speed Precent",      EL<TEdit>("TfSpeed", "edSecondSpeedOut")->Text);                                //Steven 20140217 : 兩段速移動
    WriteIniData(szDir, "Output Arm", "Two Speed ADC",          EL<TEdit>("TfSpeed", "edSecondADCOut")->Text);                                  //Steven 20140217 : 兩段速移動

    //One by One Enable
    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ;
    }
    else
    {
        WriteIniData(szDir, "Output Arm", "One by one",         (EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->ItemIndex==0)?0:1);
    }
    WriteIniData(szDir, "Output Arm", "Vacuum Timing",          (EL<TCheckBox>("TfSpeed", "chkOutArmPreSuck")->Checked)?0:1);
    WriteIniData(szDir, "Output Arm", "Vacuum Check Optimize",  (EL<TCheckBox>("TfSpeed", "cbOutVacuumCheckOptimize")->Checked)?1:0);
    WriteIniData(szDir, "Output Arm", "Open/Close Speed",       EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text);
    WriteIniData(szDir, "Output Arm", "Open/Close Accel",       EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text);
    WriteIniData(szDir, "Output Arm", "Wait before Air On",     EL<TEdit>("TfSpeed", "edtOutArmPreSuck")->Text);

    WriteIniData(szDir, "Output Arm", "Rotate Output Speed",    EL<TEdit>("TfSpeed", "edOutRotSpd")->Text);                                     //kevin 20130517 rotate add
    WriteIniData(szDir, "Output Arm", "Rotate Output Accel",    EL<TEdit>("TfSpeed", "edOutRotAcc")->Text);                                     //kevin 20130517 rotate add
    //2013-08-01    Dell    modify  公司統一回吸功能
    WriteIniData(szDir, "Output Arm", "Destroy Check Time",     EL<TEdit>("TfSpeed", "Edt_CheckTime_Out")->Text);
    WriteIniData(szDir, "Output Arm", "Destroy Pause Check",    (EL<TCheckBox>("TfSpeed", "ckb_PauseCheck_Out")->Checked)?1:0);
    WriteIniData(szDir, "Output Arm", "DevicConfirm",           (EL<TRadioGroup>("TfSpeed", "gbOutarmDeviveConfirm")->ItemIndex==0)?1:0);             //kevin 20170510 (wei) 回吸偵測開關
    WriteIniData(szDir, "Output Arm", "bYPitchNotUseSearchLastMode", (EL<TRadioGroup>("TfSpeed", "rgSearchLastModeForYPitch")->ItemIndex==0)?1:0);    //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格

    WriteIniData(szDir, "Output Arm", "Cylinder Delay",          EL<TEdit>("TfSpeed", "edOutArmCylinderDelay")->Text);
    //--------------------------------------------------------------------------
    //Empty Tray Arm
    //--------------------------------------------------------------------------
    WriteIniData(szDir, "Empty Tray Arm", "Speed",              EL<TEdit>("TfSpeed", "edTrayXSpd")->Text);
    WriteIniData(szDir, "Empty Tray Arm", "Accel",              EL<TEdit>("TfSpeed", "edTrayXAcc")->Text);
    WriteIniData(szDir, "Empty Tray Arm", "Retry Count",        EL<TEdit>("TfSpeed", "edTrayArmRetryCount")->Text);
    WriteIniData(szDir, "Empty Tray Arm", "Vacuum Check Time",  EL<TEdit>("TfSpeed", "edTrayVacumCheckTime")->Text);
    WriteIniData(szDir, "Empty Tray Arm", "Counter Air ON Time",EL<TEdit>("TfSpeed", "edTrayArmAirOnTime")->Text);
    WriteIniData(szDir, "Empty Tray Arm", "Hand Down Time",     EL<TEdit>("TfSpeed", "edTrayArmHandDown")->Text);
    //--------------------------------------------------------------------------
    //Shuttle
    //--------------------------------------------------------------------------
    WriteIniData(szDir, "Shuttle", "Shuttle 1 Speed",           EL<TEdit>("TfSpeed", "edSht1Spd")->Text);
    WriteIniData(szDir, "Shuttle", "Shuttle 1 Accel",           EL<TEdit>("TfSpeed", "edSht1Acc")->Text);
    WriteIniData(szDir, "Shuttle", "Shuttle 2 Speed",           EL<TEdit>("TfSpeed", "edSht2Spd")->Text);
    WriteIniData(szDir, "Shuttle", "Shuttle 2 Accel",           EL<TEdit>("TfSpeed", "edSht2Acc")->Text);
    WriteIniData(szDir, "Shuttle", "Device Check Time",         EL<TEdit>("TfSpeed", "edShtDeviceCheckTime")->Text);
    WriteIniData(szDir, "Shuttle", "Shake Cycles",              EL<TEdit>("TfSpeed", "edShakeCycles")->Text);                                   //Sam 20250326 : 新增 Shake 條件設定
    WriteIniData(szDir, "Shuttle", "Shake distance",            EL<TEdit>("TfSpeed", "edShakeDistance")->Text);
    WriteIniData(szDir, "Shuttle", "Delay between shakes",      EL<TEdit>("TfSpeed", "edShakeDelay")->Text);
    WriteIniData(szDir, "Shuttle", "Shake Accel",               EL<TEdit>("TfSpeed", "edShakeAccDec")->Text);
    //step shuttle
    WriteIniData(szDir, "Shuttle", "Step Shuttle",              (EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->ItemIndex==0)?1:0);
    WriteIniData(szDir, "Shuttle", "Shake Shuttle",             (EL<TRadioGroup>("TfSpeed", "rgShakeShuttle")->ItemIndex));                           //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭   //Steven 20220427 : 每次放料都要抖抖馬達
    WriteIniData(szDir, "Shuttle", "Shuttle Sort Speed",        EL<TEdit>("TfSpeed", "edSht2Spd")->Text);                                       //RogerYang 20250514 Add for 9046AU 暫時用Shuttl2的速度
    WriteIniData(szDir, "Shuttle", "Shuttle Sort Accel",        EL<TEdit>("TfSpeed", "edSht2Acc")->Text);

    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    WriteIniData(szDir, "Magazine", "Magazine CatchY Speed",     EL<TEdit>("TfSpeed", "edMagCatchYSpd")->Text);
    WriteIniData(szDir, "Magazine", "Magazine CatchY Accel",     EL<TEdit>("TfSpeed", "edMagCatchYAcc")->Text);
    WriteIniData(szDir, "Magazine", "MagazineZ Speed",           EL<TEdit>("TfSpeed", "edMagZSpd")->Text);
    WriteIniData(szDir, "Magazine", "MagazineZ Accel",           EL<TEdit>("TfSpeed", "edMagZAcc")->Text);

    WriteIniData(szDir, "Magazine", "Two Speed Distance",      EL<TEdit>("TfSpeed", "edTwoSpeedDistanceCatchY")->Text);
    WriteIniData(szDir, "Magazine", "Two Speed Precent",       EL<TEdit>("TfSpeed", "edSecondSpeedCatchY")->Text);
    WriteIniData(szDir, "Magazine", "Two Speed ADC",           EL<TEdit>("TfSpeed", "edSecondADCCatchY")->Text);
    SP_ReadWriteFile(false, szDir);

    //Steven 20170317 (wei) mark
    //==>
//    if(CUSTOMER_CODE==CC_AMKOR_Japan)       //Steven 20170202 : For ATJ Out Arm pickup error put to define tray
//    {
//        WriteIniData(szDir, "Output Arm", "Out Pick Err Action", cbOutPickErrAct->Checked);
//        WriteIniData(szDir, "Output Arm", "Out Pick Err Tray",   cobOutPickErrAct->ItemIndex);
//    }
    //<==
    //Steven 20170317 (wei) mark

    AddSpace(szDir);
    SP_ReadFile();
    SP_DoIniDataToForm();
    ;   // golden cSpeed.cpp:1730-1730 取代：fShowMessage->sgdSpeedView->Refresh()：主畫面速度表重畫（HTML 端）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1730-1730 -- fShowMessage->sgdSpeedView->Refresh()：主畫面速度表重畫（HTML 端）
    fShowMessage->sgdSpeedView->Refresh();
#endif // GATE (S12-C save)
    EL<TSpeedButton>("TfSpeed", "spbSave")->Down=false;

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SwitchSpeed);                                    //47     Change Handler Speed
    }

    if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden cSpeed.cpp:1739-1761 Recipe Parameter Default compare not ported");   // golden cSpeed.cpp:1739-1761 取代：Recipe Parameter Default 比對（fRPDefault／SearchRecipeParameter 未移植）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1739-1761 -- Recipe Parameter Default 比對（fRPDefault／SearchRecipeParameter 未移植）
    if(CosFunction.bRecipeParameterDefault)                                     //Isaac 20170527 (Steven) defalut值比較功能
    {
        fRPDefault->RP_Name->Clear()    ;
        fRPDefault->RP_Default->Clear() ;
        fRPDefault->RP_Value->Clear()   ;
        SearchRecipeParameter(fSpeed->tsAllSpeed,-1);                           //Isaac 20171227 (Steven) : AutoSpeed RPD一鍵復原
        SearchRecipeParameter(fSpeed->tsIndex,0);
        SearchRecipeParameter(fSpeed->tsInArm,1);
        SearchRecipeParameter(fSpeed->tsOutArm,2);
        SearchRecipeParameter(fSpeed->tsTrayArm,3);
        SearchRecipeParameter(fSpeed->tsShuttle,4);
         //Sam 20201209 : Default Recipe ChangeLog
        //==>
        if(CosFunction.bRecipeParameterDefaultLog)
        {
           fCleaning->SearchRecipeParameter(fCleaning, 0);
           FTestIF->SearchRecipeParameter(FTestIF, 0);
           fYieldMonitoring->SearchRecipeParameter(fYieldMonitoring, 0);
        }
        //<==
        //Sam 20201209 : Default Recipe ChangeLog
        fRPDefault->CompareRPDefaultAndValue(fRPDefault->RP_Name,fRPDefault->RP_Default,fRPDefault->RP_Value);
    }
#endif // GATE (S12-C save)

    if(IniConfig.bSPILFunction || bEnable_KLT_Function)
    {
        //Ifor 20180802 (Steven) : add SPIL && KLT 判斷Auto Skip功能是否開啟並切換模式
        //==>
        AnsiString S="";
        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\Tray.Data", DataPath, S);
        if(ArmSpeed_File[InArm].bAutoSKIP)
            TrayForm.iManualRemoveLoader=1;
        else
            TrayForm.iManualRemoveLoader       =ReadIniData(szDir, "Flag",     "Skip Manual Remove Tray",  0);
        //<==
        //Ifor 20180802 (Steven) : add SPIL && KLT 判斷Auto Skip功能是否開啟並切換模式
    }

    if(TestIF_File.bEnableReadAndCheckTorque)                                   //kevin 20210804 change
    {
        bResetArm1Value=true;
        bResetArm2Value=true;
    }
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:1784 dmTrayMotor->StartSetSpeed not ported");   // golden cSpeed.cpp:1784-1784 取代：dmTrayMotor->StartSetSpeed()：步進速度推到驅動器（移植樹沒有 dmTrayMotor）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1784-1784 -- dmTrayMotor->StartSetSpeed()：步進速度推到驅動器（移植樹沒有 dmTrayMotor）
    dmTrayMotor->StartSetSpeed();                                               //Steven 20210318 : Loader入Tray改步進, 換位置寫入速度, 避免動作異常
#endif // GATE (S12-C save)
    bTT_SetSpeed_Check=false;                                                   //Ztex 2024.08.11 Add Test Time Set Speed

}

// golden cSpeed.cpp:1414  TfSpeed::spbSpeedAddClick(TObject *Sender)
static void SP_spbSpeedAddClick()
{

    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position + 10;   // golden cSpeed.cpp:1416-1416 取代：Position+=10：屬性複合指派 → 讀、加、寫回（ELTrackBar::Pos 沒有 +=）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1416-1416 -- Position+=10：屬性複合指派 → 讀、加、寫回（ELTrackBar::Pos 沒有 +=）
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position+=10;
#endif // GATE (S12-C save)
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position / 10;   // golden cSpeed.cpp:1417-1417 取代：Position/=10：同上（int 除法）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1417-1417 -- Position/=10：同上（int 除法）
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position/=10;
#endif // GATE (S12-C save)
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position * 10;   // golden cSpeed.cpp:1418-1418 取代：Position*=10：同上
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1418-1418 -- Position*=10：同上
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position*=10;
#endif // GATE (S12-C save)

}

// golden cSpeed.cpp:1421  TfSpeed::spbSpeedDecClick(TObject *Sender)
static void SP_spbSpeedDecClick()
{

    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position - 10;   // golden cSpeed.cpp:1423-1423 取代：Position-=10：同上
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1423-1423 -- Position-=10：同上
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position-=10;
#endif // GATE (S12-C save)

}

// golden cSpeed.cpp:1426  TfSpeed::cbIndexArmClick(TObject *Sender)
static void SP_cbIndexArmClick()
{

    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Enabled =true;
    EL<TSpeedButton>("TfSpeed", "spbSpeedAdd")->Enabled=true;
    EL<TSpeedButton>("TfSpeed", "spbSpeedDec")->Enabled=true;

}

// golden cSpeed.cpp:1795  TfSpeed::spbSelectAllClick(TObject *Sender)
static void SP_spbSelectAllClick()
{

    EL<TCheckBox>("TfSpeed", "cbIndexArm")->Checked =true;
    EL<TCheckBox>("TfSpeed", "cbInArm")->Checked    =true;
    EL<TCheckBox>("TfSpeed", "cbOutArm")->Checked   =true;
    EL<TCheckBox>("TfSpeed", "cbShuttle")->Checked  =true;
    EL<TCheckBox>("TfSpeed", "cbTrayArm")->Checked  =true;
    EL<TCheckBox>("TfSpeed", "cbInArmZ")->Checked   =true;
    EL<TCheckBox>("TfSpeed", "cbOutArmZ")->Checked  =true;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Enabled =true;
    EL<TSpeedButton>("TfSpeed", "spbSpeedAdd")->Enabled=true;
    EL<TSpeedButton>("TfSpeed", "spbSpeedDec")->Enabled=true;
    EL<TSpeedButton>("TfSpeed", "spbSelectAll")->Down  =false;
    EL<TCheckBox>("TfSpeed", "cbInRotate")->Checked =true;
    EL<TCheckBox>("TfSpeed", "cbOutRotate")->Checked=true;

}

// golden cSpeed.cpp:1812  TfSpeed::spbSetToDefClick(TObject *Sender)
static void SP_spbSetToDefClick()
{

    EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position=100;
    EL<filerw::ELTrackBar>("TfSpeed", "tbAccSpeed")->Position=100;
    EL<TEdit>("TfSpeed", "edAllSpeed")->Text    =100;
    EL<TEdit>("TfSpeed", "edAllAccSpeed")->Text =100;
    SP_spbSelectAllClick(); SP_spbSpeedAddClick(); SP_tbAccSpeedChange();   // golden cSpeed.cpp:1818-1820 取代：spbSelectAllClick(this)／spbSpeedAddClick(this)／tbAccSpeedChange(this)：直接呼叫處理器（Sender 不用）
#if 0 // GATE (S12-C save) golden cSpeed.cpp:1818-1820 -- spbSelectAllClick(this)／spbSpeedAddClick(this)／tbAccSpeedChange(this)：直接呼叫處理器（Sender 不用）
    SP_spbSelectAllClick(this);
    SP_spbSpeedAddClick(this);
    SP_tbAccSpeedChange(this);
#endif // GATE (S12-C save)

    //Index unit Condition
    //Speed
    EL<TEdit>("TfSpeed", "edIndexSpeed")->Text=100;                                                     // %
    //Accel./Decel.
    EL<TEdit>("TfSpeed", "edIndexAccDec")->Text=100;                                                    // %
    //Retry Count
    EL<TEdit>("TfSpeed", "edIndexRetryCount")->Text=0;                                                  // Times
    EL<TEdit>("TfSpeed", "edIndexArmRetryMM")->Text=0;                                                  // Down mm
    //Function for small package
    EL<TCheckBox>("TfSpeed", "chkIndexPreSuck")->Checked=true;
    //Wait Time
    //Vacuum Check Wait
    EL<TEdit>("TfSpeed", "edIndexVacumCheckTime")->Text=0.1;                                            // Sec
    //Counter Air On Time
    EL<TEdit>("TfSpeed", "edIndexAirOnTime")->Text=0.1;                                                 //Sec
    //Socket Device Floating Check
    EL<TRadioGroup>("TfSpeed", "rgSocketCheck")->ItemIndex=1;                                                 //0:ON,1:OFF

    //InputArm Condition
    //Speed & Accel./Dece.
    //X-Y Axis
    EL<TEdit>("TfSpeed", "edInXYSpd")->Text=100;                                                        //Speed(%)
    EL<TEdit>("TfSpeed", "edInXYAcc")->Text=100;                                                        //Accel/Decel.(%)
    //Z-Axis
    EL<TEdit>("TfSpeed", "edInZSpd")->Text=100;                                                         //Speed(%)
    EL<TEdit>("TfSpeed", "edInZAcc")->Text=100;                                                         //Accel/Decel.(%)
    //Open/Xlose
    EL<TEdit>("TfSpeed", "edInPitchSpd")->Text=100;                                                     //Speed(%)
    EL<TEdit>("TfSpeed", "edInPitchAcc")->Text=100;                                                     //Accel/Decel.(%)
    //Retry Count
    EL<TEdit>("TfSpeed", "edInArmRetryCount")->Text=2;                                                  // Times
    EL<TEdit>("TfSpeed", "edInArmRetryMM")->Text=-0.01;                                                 // Down mm
    //Function for small package
    EL<TCheckBox>("TfSpeed", "chkInArmPreSuck")->Checked=true;
    //Wait Time
    //Vacuum Check Wait
    EL<TEdit>("TfSpeed", "edInVacumCheckTime")->Text=0.1;                                               // Sec
    EL<TEdit>("TfSpeed", "edtHPVacuumDelay")->Text=0.1;                                                 // Sec
    //Counter Air On Time
    EL<TEdit>("TfSpeed", "edInArmAirOnTime")->Text=0.1;                                                 //Sec
    //Wait Time On Shuttle
    EL<TEdit>("TfSpeed", "edInArmShtWaitTime")->Text=0.01;                                              //Sec
    //Auto Skip
    EL<TRadioGroup>("TfSpeed", "rgInArmAutoSpeed")->ItemIndex=1;                                              //0:ON,1:OFF
    //Variable Pitch Function
    EL<TRadioGroup>("TfSpeed", "rgInArmPitch")->ItemIndex=0;                                                  //0:Open/Close,1:Fixed

    //OutputArm Condition
    //Speed & Accel./Dece.
    //X-Y Axis
    EL<TEdit>("TfSpeed", "edOutXSpd")->Text=100;                                                        //Speed(%)
    EL<TEdit>("TfSpeed", "edOutXAcc")->Text=100;                                                        //Accel/Decel.(%)
    //Z-Axis
    EL<TEdit>("TfSpeed", "edOutZSpd")->Text=100;                                                        //Speed(%)
    EL<TEdit>("TfSpeed", "edOutZAcc")->Text=100;                                                        //Accel/Decel.(%)
    //Open/Xlose
    EL<TEdit>("TfSpeed", "edOutPitchSpd")->Text=100;                                                    //Speed(%)
    EL<TEdit>("TfSpeed", "edOutPitchAcc")->Text=100;                                                    //Accel/Decel.(%)
    //Retry Count
    EL<TEdit>("TfSpeed", "edOutArmRetryCount")->Text=2;                                                 // Times
    EL<TEdit>("TfSpeed", "edOutArmRetryMM")->Text=-0.01;                                                // Down mm
    //Function for small package
    EL<TCheckBox>("TfSpeed", "chkOutArmPreSuck")->Checked=true;
    EL<TEdit>("TfSpeed", "edtOutArmPreSuck")->Text=0.01;                                                //Wait before Air-On Sec.
    //Wait Time
    //Vacuum Check Wait
    EL<TEdit>("TfSpeed", "edOutVacumCheckTime")->Text=0.1;                                              // Sec
    //Counter Air On Time
    EL<TEdit>("TfSpeed", "edOutArmAirOnTime")->Text=0.1;                                                //Sec
    //Variable Pitch Function
    EL<TRadioGroup>("TfSpeed", "rgOutArmPitch")->ItemIndex=0;                                                 //0:Open/Close,1:Fixed

    //Shuttle Condition
    //Speed & Accel/Dece.
    //Shuttle1
    EL<TEdit>("TfSpeed", "edSht1Spd")->Text=100;                                                        //Speed(%)
    EL<TEdit>("TfSpeed", "edSht1Acc")->Text=100;                                                        //Accel/Decel.(%)
    //Shuttle2
    EL<TEdit>("TfSpeed", "edSht2Spd")->Text=100;                                                        //Speed(%)
    EL<TEdit>("TfSpeed", "edSht2Acc")->Text=100;                                                        //Accel/Decel.(%)
    //Wait Time
    //Device Check(Input)
    EL<TEdit>("TfSpeed", "edShtDeviceCheckTime")->Text=0.01;                                            //Sec.
    //Step Shuttle
    EL<TRadioGroup>("TfSpeed", "rgStepShuttle")->ItemIndex=1;                                                 //0:ON,1:Off

    //Empty Tray Arm Condition
    //Speed & Accel/Dece.
    EL<TEdit>("TfSpeed", "edTrayXSpd")->Text=100;                                                       //Speed(%)
    EL<TEdit>("TfSpeed", "edTrayXAcc")->Text=100;                                                       //Accel./Decel.(%)
    //Wait Time
    //Vacuum Check Wait
    EL<TEdit>("TfSpeed", "edTrayVacumCheckTime")->Text=0.1;                                             //Sec.
    //Counter Air On Time
    EL<TEdit>("TfSpeed", "edTrayArmAirOnTime")->Text=0.1;                                               //Sec.
    //Hand Down
    EL<TEdit>("TfSpeed", "edTrayArmHandDown")->Text=0.5;                                                //Sec.
    //Retry Count
    EL<TEdit>("TfSpeed", "edTrayArmRetryCount")->Text=2;                                                //Times
    EL<TEdit>("TfSpeed", "edShakeCycles")->Text=1;                                                      //Sam 20250326 : 新增 Shake 條件設定
    EL<TEdit>("TfSpeed", "edShakeDistance")->Text=5;
    EL<TEdit>("TfSpeed", "edShakeDelay")->Text=0.0;
    EL<TEdit>("TfSpeed", "edShakeAccDec")->Text=100;
    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    EL<TEdit>("TfSpeed", "edMagCatchYSpd")->Text=50;                                                    //Speed(%)
    EL<TEdit>("TfSpeed", "edMagCatchYAcc")->Text=50;                                                    //Accel/Decel.(%)

    EL<TEdit>("TfSpeed", "edMagZSpd")->Text=50;                                                         //Speed(%)
    EL<TEdit>("TfSpeed", "edMagZAcc")->Text=50;                                                         //Accel/Decel.(%)

}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/editlist/ArmSpeed_File.py 的 events；
//   FileRW/ArmSpeed_File.cpp 用 filerw::PageEventsRegistrar 註冊；本體 FileRW/_EditPage.cpp RunPageEvent）
static void SP_Ev_cbIndexArmClick(TControl* Sender) { (void)Sender; SP_cbIndexArmClick(); }
static void SP_Ev_spbSelectAllClick(TControl* Sender) { (void)Sender; SP_spbSelectAllClick(); }
static void SP_Ev_spbSetToDefClick(TControl* Sender) { (void)Sender; SP_spbSetToDefClick(); }
static void SP_Ev_spbSpeedAddClick(TControl* Sender) { (void)Sender; SP_spbSpeedAddClick(); }
static void SP_Ev_spbSpeedDecClick(TControl* Sender) { (void)Sender; SP_spbSpeedDecClick(); }
static const filerw::PageEvent kSP_Events[] = {
    {"cbIndexArm", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbInArm", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbOutArm", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbShuttle", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbTrayArm", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbInArmZ", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbOutArmZ", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbInRotate", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"cbOutRotate", "click", "cSpeed.cpp:1426 TfSpeed::cbIndexArmClick", &SP_Ev_cbIndexArmClick},
    {"spbSelectAll", "click", "cSpeed.cpp:1795 TfSpeed::spbSelectAllClick", &SP_Ev_spbSelectAllClick},
    {"spbSetToDef", "click", "cSpeed.cpp:1812 TfSpeed::spbSetToDefClick", &SP_Ev_spbSetToDefClick},
    {"spbSpeedAdd", "click", "cSpeed.cpp:1414 TfSpeed::spbSpeedAddClick", &SP_Ev_spbSpeedAddClick},
    {"spbSpeedDec", "click", "cSpeed.cpp:1421 TfSpeed::spbSpeedDecClick", &SP_Ev_spbSpeedDecClick},
};

