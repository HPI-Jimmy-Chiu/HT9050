// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 AOISetup 的讀寫檔（HTEditList： → <DataPath>\<配方>\AOI.Data [SETTING]／[DutOnOff_BGAView]／[DutOnOff_PADView]／[RS232]／[AOITRAY]（讀 fAOI_ReadFile：只有 ReadIniData；寫 spbSaveClick：WriteIniData））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden fAOI.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 91 個（header 宣告 356 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "common.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "forms/fAOI.h"
#include "forms/fMain.h"

#include <cstdio>   // std::printf（fAOI_ReadFile 的 Top&Bottom 提醒）

// golden fAOI.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static int iQuotient;    // golden fAOI.h:428（private；建構子 :180 設 0 —— 本 TU static 零初值相同；golden 只有讀寫檔兩支用）
static int iRemainder;   // golden fAOI.h:429（同上，:181）

using filerw::EL;

// 前置宣告
static void AO_fAOI_ReadFile();
static void AO_fAOI_DoIniDataToForm();
static void AO_UpdateAOIFailBinTypetoForm();
static void AO_UpDataTrayData();
static bool AO_CheckFailBin();
static void AO_spbSaveClick();

// golden fAOI.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void AO_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TFrmAOI", "ComboBox1")->Items;
      s->Add("Auto1");
      s->Add("Auto2");
      s->Add("Auto3");
      s->Add("Fix1");
      s->Add("Fix3");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbAOIFial")->Items;
      s->Add("Fix2");
      s->Add("Fix3");
      s->Add("Fix5");
      s->Add("Fix6");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->Items;
      s->Add("Fix2");
      s->Add("Fix3");
      s->Add("Fix5");
      s->Add("Fix6");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbBaudRate")->Items;
      s->Add("4800");
      s->Add("7200");
      s->Add("9600");
      s->Add("14400");
      s->Add("19200");
      s->Add("38400");
      s->Add("57600");
      s->Add("115200");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbByteSize")->Items;
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbDevice")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbParity")->Items;
      s->Add("None");
      s->Add("Odd");
      s->Add("Even");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbStopBit")->Items;
      s->Add("1");
      s->Add("1.5");
      s->Add("2");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopAOIFial")->Items;
      s->Add("Fix2");
      s->Add("Fix3");
      s->Add("Fix5");
      s->Add("Fix6");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopBaudRate")->Items;
      s->Add("4800");
      s->Add("7200");
      s->Add("9600");
      s->Add("14400");
      s->Add("19200");
      s->Add("38400");
      s->Add("57600");
      s->Add("115200");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopByteSize")->Items;
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopDevice")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopParity")->Items;
      s->Add("None");
      s->Add("Odd");
      s->Add("Even");
    }
    { TStringList* s = EL<TComboBox>("TFrmAOI", "cbTopStopBit")->Items;
      s->Add("1");
      s->Add("1.5");
      s->Add("2");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType")->Items;
      s->Add("AOI Only");
      s->Add("AOI+Tester");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgAOISelectZ")->Items;
      s->Add("Z1");
      s->Add("Z2");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgAOISiteMode")->Items;
      s->Add("Site1");
      s->Add("Site2");
      s->Add("Site1&&Site2");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgActionMode")->Items;
      s->Add("Scan Top and Bottom");
      s->Add("Scan Bottom");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgBallDamageType")->Items;
      s->Add("SINGLE");
      s->Add("DUAL");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgScannerMode")->Items;
      s->Add("Not Use");
      s->Add("Use Scanner AOI");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgTopAOISiteMode")->Items;
      s->Add("Site1");
      s->Add("Site2");
      s->Add("Site1&&Site2");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgTopBtmScannEnable")->Items;
      s->Add("Not Use");
      s->Add("Use Scanner AOI");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmAOI", "rgTopScannerMode")->Items;
      s->Add("Not Use");
      s->Add("Use TOP Scanner AOI");
    }
}

// golden fAOI.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kAO_ParentOf[][2] = {
    {"GroupBox1", "TabSheet1"},
    {"GroupBox11", "TabSheet2"},
    {"GroupBox12", "TabSheet4"},
    {"GroupBox13", "TabSheet3"},
    {"GroupBox14", "TabSheet5"},
    {"GroupBox15", "TabSheet6"},
    {"GroupBox17", "Panel21"},
    {"GroupBox18", "Panel21"},
    {"GroupBox2", "TabSheet1"},
    {"GroupBox3", "TabSheet1"},
    {"GroupBox4", "TabSheet1"},
    {"GroupBox5", "GroupBox7"},
    {"GroupBox7", "TabSheet2"},
    {"GroupBox8", "TabSheet2"},
    {"GroupBox9", "GroupBox8"},
    {"PageControl4", "tsTopBtnModSet"},
    {"Panel15", "Panel17"},
    {"Panel17", "tsTopBtnModSet"},
    {"Panel21", "TabSheet11"},
    {"Panel3", "GroupBox4"},
    {"Panel5", "TabSheet5"},
    {"TabSheet1", "PageControl1"},
    {"TabSheet11", "PageControl4"},
    {"TabSheet2", "PageControl1"},
    {"TabSheet3", "PageControl1"},
    {"TabSheet4", "PageControl1"},
    {"TabSheet5", "PageControl1"},
    {"TabSheet6", "PageControl1"},
    {"cbAOIFial", "pnlAOISet"},
    {"cbAOIFialAndTestPass", "pnlAOISet"},
    {"cbAOINoSort", "GroupBox4"},
    {"cbAutoSkipBGA", "GroupBox4"},
    {"cbAutoSkipPadPkg", "GroupBox4"},
    {"cbBDBySite", "pnlAOIBDAlarm"},
    {"cbBDBySiteConti", "pnlAOIBDAlarm"},
    {"cbBDByTotal", "pnlAOIBDAlarm"},
    {"cbBDByTotalConti", "pnlAOIBDAlarm"},
    {"cbBGAFailRun", "GroupBox4"},
    {"cbBaudRate", "GroupBox12"},
    {"cbByteSize", "GroupBox12"},
    {"cbDBAlarmAutoReset", "pnlBottom"},
    {"cbDevice", "GroupBox12"},
    {"cbEnabledBGAView", "GroupBox3"},
    {"cbEnabledPADView", "GroupBox2"},
    {"cbEnabledTopView", "GroupBox1"},
    {"cbParity", "GroupBox12"},
    {"cbStopBit", "GroupBox12"},
    {"cbTesterFailBin", "GroupBox4"},
    {"cbTopAOIFial", "Panel5"},
    {"cbTopBaudRate", "GroupBox15"},
    {"cbTopByteSize", "GroupBox15"},
    {"cbTopDevice", "GroupBox15"},
    {"cbTopParity", "GroupBox15"},
    {"cbTopStopBit", "GroupBox15"},
    {"cb_AlarmByArm", "GroupBox5"},
    {"cb_AlarmByArm1", "GroupBox9"},
    {"cb_AlarmBySite", "GroupBox5"},
    {"cb_AlarmBySite1", "GroupBox9"},
    {"cb_EnabledPositionByAOI", "TabSheet3"},
    {"cb_LGAModeDirection", "TabSheet3"},
    {"cb_ScanAOIAlarmByArm", "GroupBox13"},
    {"cb_ScanAOIAlarmBySite", "GroupBox13"},
    {"cb_ScanAOIUnUseFailBin", "TabSheet3"},
    {"cb_ScanAOIUseLGAMode", "TabSheet3"},
    {"cb_TopScanAOIAlarmByArm", "GroupBox14"},
    {"cb_TopScanAOIAlarmBySite", "GroupBox14"},
    {"cb_TopScanAOIUnUseFailBin", "TabSheet5"},
    {"cb_VitContiuneAlarm", "TabSheet2"},
    {"edtAOIBDCounterSite", "pnlAOIBDAlarm"},
    {"edtAOIBDCounterSiteConti", "pnlAOIBDAlarm"},
    {"edtAOIBDCounterTotal", "pnlAOIBDAlarm"},
    {"edtAOIBDCounterTotalConti", "pnlAOIBDAlarm"},
    {"edtAOIIntervalCounter", "pnlAOISet"},
    {"edtAOIRetryCounter", "pnlAOISet"},
    {"edtScannerICGain", "TabSheet3"},
    {"edtScannerLGAWaitTime", "TabSheet3"},
    {"edtScannerReadTimeout", "TabSheet3"},
    {"edtTopAOIIntervalCounter", "Panel5"},
    {"edtTopAOIRetryCounter", "Panel5"},
    {"edtTopScannerReadTimeout", "TabSheet5"},
    {"edt_AlarmCount", "GroupBox4"},
    {"edt_ConAlarmConut", "GroupBox11"},
    {"edt_ConAlarmConut1", "GroupBox11"},
    {"edt_FailBGAView", "GroupBox3"},
    {"edt_FailFailBGAPADView", "Panel3"},
    {"edt_FailPADView", "GroupBox2"},
    {"edt_FailTopView", "GroupBox1"},
    {"edt_PassFailBGAPADView", "Panel3"},
    {"edt_SDelayBGAView", "GroupBox3"},
    {"edt_SDelayPADView", "GroupBox2"},
    {"edt_SDelayScanAOI", "TabSheet3"},
    {"edt_SDelayTopScanAOI", "TabSheet5"},
    {"edt_SDelayTopView", "GroupBox1"},
    {"edt_ScanAOIAlarmCountByArm", "GroupBox13"},
    {"edt_ScanAOIAlarmCountBySite", "GroupBox13"},
    {"edt_TimeoutBGAView", "GroupBox3"},
    {"edt_TimeoutPADView", "GroupBox2"},
    {"edt_TimeoutScanAOI", "TabSheet3"},
    {"edt_TimeoutTopScanAOI", "TabSheet5"},
    {"edt_TimeoutTopView", "GroupBox1"},
    {"edt_TopScanAOIAlarmCountByArm", "GroupBox14"},
    {"edt_TopScanAOIAlarmCountBySite", "GroupBox14"},
    {"lblAOIBinSel1", "pnlAOISet"},
    {"lblAOIBinSel2", "pnlAOISet"},
    {"mtAOIBuffer", "TabSheet3"},
    {"pnlAOIBDAlarm", "TabSheet3"},
    {"pnlAOIBDSite1ContiLast", "pnlAOIBDAlarm"},
    {"pnlAOIBDSite1Last", "pnlAOIBDAlarm"},
    {"pnlAOIBDSite2ContiLast", "pnlAOIBDAlarm"},
    {"pnlAOIBDSite2Last", "pnlAOIBDAlarm"},
    {"pnlAOIBDTotalContiLast", "pnlAOIBDAlarm"},
    {"pnlAOIBDTotalLast", "pnlAOIBDAlarm"},
    {"pnlAOISet", "TabSheet3"},
    {"pnlBottom", "pnlAOIBDAlarm"},
    {"rgAOIFailBinType", "pnlAOISet"},
    {"rgActionMode", "Panel15"},
    {"rgBallDamageType", "pnlAOISet"},
    {"rgScannerMode", "TabSheet3"},
    {"rgTopScannerMode", "TabSheet5"},
    {"sgICLarge", "GroupBox18"},
    {"sgICSmall", "GroupBox17"},
    {"tsTopBtnModSet", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void AO_CreateContainerProxies()
{
    EL<TGroupBox>("TFrmAOI", "GroupBox1");
    EL<TGroupBox>("TFrmAOI", "GroupBox11");
    EL<TGroupBox>("TFrmAOI", "GroupBox12");
    EL<TGroupBox>("TFrmAOI", "GroupBox13");
    EL<TGroupBox>("TFrmAOI", "GroupBox14");
    EL<TGroupBox>("TFrmAOI", "GroupBox15");
    EL<TGroupBox>("TFrmAOI", "GroupBox17");
    EL<TGroupBox>("TFrmAOI", "GroupBox18");
    EL<TGroupBox>("TFrmAOI", "GroupBox2");
    EL<TGroupBox>("TFrmAOI", "GroupBox3");
    EL<TGroupBox>("TFrmAOI", "GroupBox4");
    EL<TGroupBox>("TFrmAOI", "GroupBox5");
    EL<TGroupBox>("TFrmAOI", "GroupBox7");
    EL<TGroupBox>("TFrmAOI", "GroupBox8");
    EL<TGroupBox>("TFrmAOI", "GroupBox9");
    EL<TPageControl>("TFrmAOI", "PageControl1");
    EL<TPageControl>("TFrmAOI", "PageControl4");
    EL<TPanel>("TFrmAOI", "Panel15");
    EL<TPanel>("TFrmAOI", "Panel17");
    EL<TPanel>("TFrmAOI", "Panel21");
    EL<TPanel>("TFrmAOI", "Panel3");
    EL<TPanel>("TFrmAOI", "Panel5");
    EL<TTabSheet>("TFrmAOI", "TabSheet1");
    EL<TTabSheet>("TFrmAOI", "TabSheet11");
    EL<TTabSheet>("TFrmAOI", "TabSheet2");
    EL<TTabSheet>("TFrmAOI", "TabSheet3");
    EL<TTabSheet>("TFrmAOI", "TabSheet4");
    EL<TTabSheet>("TFrmAOI", "TabSheet5");
    EL<TTabSheet>("TFrmAOI", "TabSheet6");
    EL<TPanel>("TFrmAOI", "pnlAOIBDAlarm");
    EL<TPanel>("TFrmAOI", "pnlAOISet");
    EL<TPanel>("TFrmAOI", "pnlBottom");
    EL<TTabSheet>("TFrmAOI", "tsTopBtnModSet");
    filerw::ELSetParents("TFrmAOI", kAO_ParentOf, (int)(sizeof(kAO_ParentOf) / sizeof(kAO_ParentOf[0])));
}

// golden fAOI.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void AO_DfmState()
{
    EL<TGroupBox>("TFrmAOI", "GroupBox1")->Visible = false;
    EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->Visible = false;
    EL<TEdit>("TFrmAOI", "edtAOIRetryCounter")->Visible = false;
    EL<TEdit>("TFrmAOI", "edtScannerICGain")->Visible = false;
    EL<TEdit>("TFrmAOI", "edtScannerReadTimeout")->Visible = false;
    EL<TEdit>("TFrmAOI", "edtTopAOIRetryCounter")->Visible = false;
    EL<TEdit>("TFrmAOI", "edtTopScannerReadTimeout")->Visible = false;
    EL<TLabel>("TFrmAOI", "lblAOIBinSel2")->Visible = false;
    EL<TPanel>("TFrmAOI", "pnlAOIBDAlarm")->Visible = false;
    EL<TRadioGroup>("TFrmAOI", "rgBallDamageType")->Visible = false;
    EL<TComboBox>("TFrmAOI", "cbAOIFial")->Text = "Fix3";
    EL<TComboBox>("TFrmAOI", "cbAOIFial")->ItemIndex = 1;
    EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->Text = "Fix3";
    EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->ItemIndex = 1;
    EL<TComboBox>("TFrmAOI", "cbBaudRate")->Text = "cbBaudRate";
    EL<TComboBox>("TFrmAOI", "cbByteSize")->Text = "cbByteSize";
    EL<TComboBox>("TFrmAOI", "cbDevice")->Text = "cbCOM";
    EL<TComboBox>("TFrmAOI", "cbParity")->Text = "cbParity";
    EL<TComboBox>("TFrmAOI", "cbStopBit")->Text = "cbStopBit";
    EL<TComboBox>("TFrmAOI", "cbTopBaudRate")->Text = "cbBaudRate";
    EL<TComboBox>("TFrmAOI", "cbTopByteSize")->Text = "cbByteSize";
    EL<TComboBox>("TFrmAOI", "cbTopDevice")->Text = "cbCOM";
    EL<TComboBox>("TFrmAOI", "cbTopParity")->Text = "cbParity";
    EL<TComboBox>("TFrmAOI", "cbTopStopBit")->Text = "cbStopBit";
    EL<TEdit>("TFrmAOI", "edtScannerICGain")->Text = "1";
    EL<TEdit>("TFrmAOI", "edt_AlarmCount")->Text = "1";
    EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType")->ItemIndex = 0;
    EL<TRadioGroup>("TFrmAOI", "rgActionMode")->ItemIndex = 0;
    EL<TRadioGroup>("TFrmAOI", "rgBallDamageType")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kAO_EnableAll[] = {
    "GroupBox1", "GroupBox11", "GroupBox12", "GroupBox13", "GroupBox14", "GroupBox15",
    "GroupBox17", "GroupBox18", "GroupBox2", "GroupBox3", "GroupBox4", "GroupBox5",
    "GroupBox7", "GroupBox8", "GroupBox9", "PageControl1", "PageControl4", "Panel15",
    "Panel17", "Panel21", "Panel3", "Panel5", "TabSheet1", "TabSheet11",
    "TabSheet2", "TabSheet3", "TabSheet4", "TabSheet5", "TabSheet6", "cbAOINoSort",
    "cbAutoSkipBGA", "cbAutoSkipPadPkg", "cbBDBySite", "cbBDBySiteConti", "cbBDByTotal", "cbBDByTotalConti",
    "cbBGAFailRun", "cbDBAlarmAutoReset", "cbEnabledBGAView", "cbEnabledPADView", "cbEnabledTopView", "cbTesterFailBin",
    "cb_AlarmByArm", "cb_AlarmByArm1", "cb_AlarmBySite", "cb_AlarmBySite1", "cb_EnabledPositionByAOI", "cb_LGAModeDirection",
    "cb_ScanAOIAlarmByArm", "cb_ScanAOIAlarmBySite", "cb_ScanAOIUnUseFailBin", "cb_ScanAOIUseLGAMode", "cb_TopScanAOIAlarmByArm", "cb_TopScanAOIAlarmBySite",
    "cb_TopScanAOIUnUseFailBin", "cb_VitContiuneAlarm", "lblAOIBinSel1", "lblAOIBinSel2", "pnlAOIBDAlarm", "pnlAOIBDSite1ContiLast",
    "pnlAOIBDSite1Last", "pnlAOIBDSite2ContiLast", "pnlAOIBDSite2Last", "pnlAOIBDTotalContiLast", "pnlAOIBDTotalLast", "pnlAOISet",
    "pnlBottom", "rgAOIFailBinType", "rgActionMode", "rgBallDamageType", "rgScannerMode", "rgTopScannerMode",
    "tsTopBtnModSet",
};

// 存檔流程（spbSaveClick）讀的替身 83 個 —— FileRW/AOISetup.cpp 用它決定頁面必須送哪些值
static const char* const kAO_SaveReads[] = {
    "cbAOIFial", "cbAOIFialAndTestPass", "cbAOINoSort", "cbAutoSkipBGA", "cbAutoSkipPadPkg", "cbBDBySite",
    "cbBDBySiteConti", "cbBDByTotal", "cbBDByTotalConti", "cbBGAFailRun", "cbBaudRate", "cbByteSize",
    "cbDBAlarmAutoReset", "cbDevice", "cbEnabledBGAView", "cbEnabledPADView", "cbEnabledTopView", "cbParity",
    "cbStopBit", "cbTesterFailBin", "cbTopAOIFial", "cbTopBaudRate", "cbTopByteSize", "cbTopDevice",
    "cbTopParity", "cbTopStopBit", "cb_AlarmByArm", "cb_AlarmByArm1", "cb_AlarmBySite", "cb_AlarmBySite1",
    "cb_EnabledPositionByAOI", "cb_LGAModeDirection", "cb_ScanAOIAlarmByArm", "cb_ScanAOIAlarmBySite", "cb_ScanAOIUnUseFailBin", "cb_ScanAOIUseLGAMode",
    "cb_TopScanAOIAlarmByArm", "cb_TopScanAOIAlarmBySite", "cb_TopScanAOIUnUseFailBin", "cb_VitContiuneAlarm", "edtAOIBDCounterSite", "edtAOIBDCounterSiteConti",
    "edtAOIBDCounterTotal", "edtAOIBDCounterTotalConti", "edtAOIIntervalCounter", "edtAOIRetryCounter", "edtScannerICGain", "edtScannerLGAWaitTime",
    "edtScannerReadTimeout", "edtTopAOIIntervalCounter", "edtTopAOIRetryCounter", "edtTopScannerReadTimeout", "edt_AlarmCount", "edt_ConAlarmConut",
    "edt_ConAlarmConut1", "edt_FailBGAView", "edt_FailFailBGAPADView", "edt_FailPADView", "edt_FailTopView", "edt_PassFailBGAPADView",
    "edt_SDelayBGAView", "edt_SDelayPADView", "edt_SDelayScanAOI", "edt_SDelayTopScanAOI", "edt_SDelayTopView", "edt_ScanAOIAlarmCountByArm",
    "edt_ScanAOIAlarmCountBySite", "edt_TimeoutBGAView", "edt_TimeoutPADView", "edt_TimeoutScanAOI", "edt_TimeoutTopScanAOI", "edt_TimeoutTopView",
    "edt_TopScanAOIAlarmCountByArm", "edt_TopScanAOIAlarmCountBySite", "pnlAOIBDSite1ContiLast", "pnlAOIBDSite1Last", "pnlAOIBDSite2ContiLast", "pnlAOIBDSite2Last",
    "pnlAOIBDTotalContiLast", "pnlAOIBDTotalLast", "rgAOIFailBinType", "rgScannerMode", "rgTopScannerMode",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void AO_CreateSaveProxies()
{
    EL<TComboBox>("TFrmAOI", "cbAOIFial");
    EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass");
    EL<TCheckBox>("TFrmAOI", "cbAOINoSort");
    EL<TCheckBox>("TFrmAOI", "cbAutoSkipBGA");
    EL<TCheckBox>("TFrmAOI", "cbAutoSkipPadPkg");
    EL<TCheckBox>("TFrmAOI", "cbBDBySite");
    EL<TCheckBox>("TFrmAOI", "cbBDBySiteConti");
    EL<TCheckBox>("TFrmAOI", "cbBDByTotal");
    EL<TCheckBox>("TFrmAOI", "cbBDByTotalConti");
    EL<TCheckBox>("TFrmAOI", "cbBGAFailRun");
    EL<TComboBox>("TFrmAOI", "cbBaudRate");
    EL<TComboBox>("TFrmAOI", "cbByteSize");
    EL<TCheckBox>("TFrmAOI", "cbDBAlarmAutoReset");
    EL<TComboBox>("TFrmAOI", "cbDevice");
    EL<TCheckBox>("TFrmAOI", "cbEnabledBGAView");
    EL<TCheckBox>("TFrmAOI", "cbEnabledPADView");
    EL<TCheckBox>("TFrmAOI", "cbEnabledTopView");
    EL<TComboBox>("TFrmAOI", "cbParity");
    EL<TComboBox>("TFrmAOI", "cbStopBit");
    EL<TCheckBox>("TFrmAOI", "cbTesterFailBin");
    EL<TComboBox>("TFrmAOI", "cbTopAOIFial");
    EL<TComboBox>("TFrmAOI", "cbTopBaudRate");
    EL<TComboBox>("TFrmAOI", "cbTopByteSize");
    EL<TComboBox>("TFrmAOI", "cbTopDevice");
    EL<TComboBox>("TFrmAOI", "cbTopParity");
    EL<TComboBox>("TFrmAOI", "cbTopStopBit");
    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm");
    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm1");
    EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite");
    EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite1");
    EL<TCheckBox>("TFrmAOI", "cb_EnabledPositionByAOI");
    EL<TCheckBox>("TFrmAOI", "cb_LGAModeDirection");
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmByArm");
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmBySite");
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUnUseFailBin");
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUseLGAMode");
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmByArm");
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmBySite");
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIUnUseFailBin");
    EL<TCheckBox>("TFrmAOI", "cb_VitContiuneAlarm");
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterSite");
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterSiteConti");
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotal");
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotalConti");
    EL<TEdit>("TFrmAOI", "edtAOIIntervalCounter");
    EL<TEdit>("TFrmAOI", "edtAOIRetryCounter");
    EL<TEdit>("TFrmAOI", "edtScannerICGain");
    EL<TEdit>("TFrmAOI", "edtScannerLGAWaitTime");
    EL<TEdit>("TFrmAOI", "edtScannerReadTimeout");
    EL<TEdit>("TFrmAOI", "edtTopAOIIntervalCounter");
    EL<TEdit>("TFrmAOI", "edtTopAOIRetryCounter");
    EL<TEdit>("TFrmAOI", "edtTopScannerReadTimeout");
    EL<TEdit>("TFrmAOI", "edt_AlarmCount");
    EL<TEdit>("TFrmAOI", "edt_ConAlarmConut");
    EL<TEdit>("TFrmAOI", "edt_ConAlarmConut1");
    EL<TEdit>("TFrmAOI", "edt_FailBGAView");
    EL<TEdit>("TFrmAOI", "edt_FailFailBGAPADView");
    EL<TEdit>("TFrmAOI", "edt_FailPADView");
    EL<TEdit>("TFrmAOI", "edt_FailTopView");
    EL<TEdit>("TFrmAOI", "edt_PassFailBGAPADView");
    EL<TEdit>("TFrmAOI", "edt_SDelayBGAView");
    EL<TEdit>("TFrmAOI", "edt_SDelayPADView");
    EL<TEdit>("TFrmAOI", "edt_SDelayScanAOI");
    EL<TEdit>("TFrmAOI", "edt_SDelayTopScanAOI");
    EL<TEdit>("TFrmAOI", "edt_SDelayTopView");
    EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountByArm");
    EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountBySite");
    EL<TEdit>("TFrmAOI", "edt_TimeoutBGAView");
    EL<TEdit>("TFrmAOI", "edt_TimeoutPADView");
    EL<TEdit>("TFrmAOI", "edt_TimeoutScanAOI");
    EL<TEdit>("TFrmAOI", "edt_TimeoutTopScanAOI");
    EL<TEdit>("TFrmAOI", "edt_TimeoutTopView");
    EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountByArm");
    EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountBySite");
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite1ContiLast");
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite1Last");
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite2ContiLast");
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite2Last");
    EL<TPanel>("TFrmAOI", "pnlAOIBDTotalContiLast");
    EL<TPanel>("TFrmAOI", "pnlAOIBDTotalLast");
    EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType");
    EL<TRadioGroup>("TFrmAOI", "rgScannerMode");
    EL<TRadioGroup>("TFrmAOI", "rgTopScannerMode");
}

// golden fAOI.cpp:3369  TFrmAOI::fAOI_ReadFile()
static void AO_fAOI_ReadFile()
{

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";

    AnsiString tmpS="";
    szDir.sprintf("%s%s\\AOI.Data", DataPath, S);

    tAOISetup.tTopView.bEnabled         =ReadIniData(szDir, "SETTING", "EnabledTopView", false);
    tAOISetup.tTopView.iFailBin         =ReadIniData(szDir, "SETTING", "FailBinTopView", 15);
    tAOISetup.tTopView.iTimeOut         =ReadIniData(szDir, "SETTING", "TimeOutTopView", 3);
    tAOISetup.tTopView.iStartDelayTime  =ReadIniData(szDir, "SETTING", "StartDelayTimeTopView", 10);

    tAOISetup.tPADView.bEnabled         =ReadIniData(szDir, "SETTING", "EnabledPADView", false);
    tAOISetup.tPADView.iFailBin         =ReadIniData(szDir, "SETTING", "FailBinPADView", 15);
    tAOISetup.tPADView.iTimeOut         =ReadIniData(szDir, "SETTING", "TimeOutPADView", 3);
    tAOISetup.tPADView.iStartDelayTime  =ReadIniData(szDir, "SETTING", "StartDelayTimePADView", 10);

    tAOISetup.tBGAView.bEnabled         =ReadIniData(szDir, "SETTING", "EnabledBGAView", false);
    tAOISetup.tBGAView.iFailBin         =ReadIniData(szDir, "SETTING", "FailBinBGAView", 15);
    tAOISetup.tBGAView.iTimeOut         =ReadIniData(szDir, "SETTING", "TimeOutBGAView", 3);
    tAOISetup.tBGAView.iStartDelayTime  =ReadIniData(szDir, "SETTING", "StartDelayTimeBGAView", 10);

    tAOISetup.tBGAPADView.iTestPassFailBin =ReadIniData(szDir, "SETTING", "TestPassFailBinBGAPADView", 15);         //wei  20160706
    tAOISetup.tBGAPADView.iTestFailFailBin =ReadIniData(szDir, "SETTING", "TestFailFailBinBGAPADView", 15);         //wei  20160706
    tAOISetup.tTesterFailBin.bEnabled   =ReadIniData(szDir, "SETTING", "EnabledTesterFailBin",   false);            //wei  20160706

    tAOISetup.iAlarmCount               =ReadIniData(szDir, "SETTING", "AlarmCount", 1);
    tAOISetup.bEnabledContinueAlarm     =ReadIniData(szDir, "SETTING", "EnabledContinueAlarm", false);              //Alick 20160712 是否使用連續Fail
    tAOISetup.bEnabledBySiteBGAView     =ReadIniData(szDir, "SETTING", "EnabledBySiteBGAView", false);              //Alick 20160712 BGA Fail by Site
    tAOISetup.bEnabledByArmBGAView      =ReadIniData(szDir, "SETTING", "EnabledByArmBGAView",  false);              //Alick 20160712 BGA Fail by Arm
    tAOISetup.iContAlarmCountBySite     =ReadIniData(szDir, "SETTING", "ContAlarmCountBySite", 3);                  //Alick 20160712 Fail by Site Count
    tAOISetup.iContAlarmCountByArm      =ReadIniData(szDir, "SETTING", "ContAlarmCountByArm",  3);                  //Alick 20160718 Fail by Arm Count
    tAOISetup.bEnabledBySitePADView     =ReadIniData(szDir, "SETTING", "EnabledBySitePADView", false);              //Alick 20160712 PAD Fail by Site
    tAOISetup.bEnabledByArmPADView      =ReadIniData(szDir, "SETTING", "EnabledByArmPADView",  false);              //Alick 20160712 PAD Fail bu Arm

    //Ifor 20191225 : add Scan AOI Alarm
    tAOISetup.bEnabledScanAOIBySiteAlarm    =ReadIniData(szDir, "SETTING", "bEnabledScanAOIBySiteAlarm", false);
    tAOISetup.bEnabledScanAOIByArmAlarm     =ReadIniData(szDir, "SETTING", "bEnabledScanAOIByArmAlarm",  false);
    tAOISetup.iScanAOIAlarmCountBySite      =ReadIniData(szDir, "SETTING", "iScanAOIAlarmCountBySite", 3);
    tAOISetup.iScanAOIAlarmCountByArm       =ReadIniData(szDir, "SETTING", "iScanAOIAlarmCountByArm",  3);

    tAOISetup.bEnabledScanAOIUnUseFailBin   =ReadIniData(szDir, "SETTING", "bEnabledScanAOIUnUseFailBin", false);   //Ifor 20200825 add:TF-AMD AOI Error 不丟至Fail Bin
    //Ifor 20191225 : add Scan AOI Alarm

    for(int i=0; i<MAX_ARM_Row; i++)                                                                                //Alick 20160713
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            S.sprintf("Dut %s", IndexSuckName[i][j]);
            bVitroxBGAViewUse[i][j]=ReadIniData(szDir, "DutOnOff_BGAView", S, bVitroxBGAViewMap[i][j]);
            bVitroxBGAViewMap[i][j]=bVitroxBGAViewUse[i][j];
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)                                                                            //Alick 20160714
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            S.sprintf("Dut %s", IndexSuckName[i][j]);
            bVitroxPADViewUse[i][j]=ReadIniData(szDir, "DutOnOff_PADView", S, bVitroxPADViewMap[i][j]);
            bVitroxPADViewMap[i][j]=bVitroxPADViewUse[i][j];
        }
    }

    if(USE_AOI_Inspection)
        tAOISetup.bEnabledAOI = tAOISetup.tTopView.bEnabled || tAOISetup.tPADView.bEnabled || tAOISetup.tBGAView.bEnabled;
    else
        tAOISetup.bEnabledAOI = false;

    tAOISetup.tFailRunFunction.bEnabled = ReadIniData(szDir, "SETTING", "EnabledailRunFunction", false);
    tAOISetup.tAutoSkipBGA.bEnabled     = ReadIniData(szDir, "SETTING", "EnabledAutoSkipBGA",    false);
    tAOISetup.tAutoSkipPadPkg.bEnabled  = ReadIniData(szDir, "SETTING", "EnabledAutoSkipPadPkg", false);

    tAOISetup.tAOINoSort.bEnabled       = ReadIniData(szDir, "SETTING", "EnabledAOINoSort", false);   //wei 20161207

    ScannerAOIIF.iEnableScannerMode      =ReadIniData(szDir, "SETTING", "iEnableScannerMode", 0);                               //Ifor 20190724 add AMD AOI
    ScannerAOIIF.fScannerReadTimeout     =ReadIniData(szDir, "SETTING", "ScannerReadTimeout", 100);
    ScannerAOIIF.iIntervalCounter        =ReadIniData(szDir, "SETTING", "iIntervalCounter", 0);
    ScannerAOIIF.iRetryCounter           =ReadIniData(szDir, "SETTING", "iRetryCounter", 0);
    ScannerAOIIF.iAOIFailBinType         =ReadIniData(szDir, "SETTING", "iAOIFailBinType", 0);                              //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    ScannerAOIIF.ScannerIfError          =ReadIniData(szDir, "SETTING", "iScanAOIFialBin", 0);
    ScannerAOIIF.ScannerIfErrorAndTestPass=ReadIniData(szDir,"SETTING", "iAOIFialAndTestPass", 0);                          //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    ScannerAOIIF.iBallDamageType         =ReadIniData(szDir, "SETTING", "iBallDamageType", 0);
    ScannerAOIIF.bBDTotalFunction        =ReadIniData(szDir, "SETTING", "bBDTotalFunction", false);
    ScannerAOIIF.bBDTotalContiFunction   =ReadIniData(szDir, "SETTING", "bBDTotalContiFunction", false);
    ScannerAOIIF.bBDSiteFunction         =ReadIniData(szDir, "SETTING", "bBDSiteFunction", false);
    ScannerAOIIF.bBDSiteContiFunction    =ReadIniData(szDir, "SETTING", "bBDSiteContiFunction", false);
    ScannerAOIIF.bDBAlramAutoResetCount  =ReadIniData(szDir, "SETTING", "bDBAlramAutoResetCount", false);

    ScannerAOIIF.iBDTotalCounter         =ReadIniData(szDir, "SETTING", "iBDTotalCounter", 0);
    ScannerAOIIF.iBDTotalContiCounter    =ReadIniData(szDir, "SETTING", "iBDTotalContiCounter", 0);
    ScannerAOIIF.iBDSiteCounter          =ReadIniData(szDir, "SETTING", "iBDSiteCounter", 0);
    ScannerAOIIF.iBDSiteContiCounter     =ReadIniData(szDir, "SETTING", "iBDSiteContiCounter", 0);
    ScannerAOIIF.iBDTotalCounterLast     =ReadIniData(szDir, "SETTING", "iBDTotalCounterLast", 0);
    ScannerAOIIF.iBDTotalContiCounterLast=ReadIniData(szDir, "SETTING", "iBDTotalContiCounterLast", 0);
    ScannerAOIIF.iBDSite1CounterLast     =ReadIniData(szDir, "SETTING", "iBDSite1CounterLast", 0);
    ScannerAOIIF.iBDSite2CounterLast     =ReadIniData(szDir, "SETTING", "iBDSite2CounterLast", 0);
    ScannerAOIIF.iBDSite1ContiCounterLast=ReadIniData(szDir, "SETTING", "iBDSite1ContiCounterLast", 0);
    ScannerAOIIF.iBDSite2ContiCounterLast=ReadIniData(szDir, "SETTING", "iBDSite2ContiCounterLast", 0);

    ScannerAOIIF.iStartDelayTime        =ReadIniData(szDir, "SETTING", "StartDelayTimeScanAOIView", 10);
    ScannerAOIIF.iTimeOut               =ReadIniData(szDir, "SETTING", "TimeOutScanAOIView", 3);

    ScannerAOIIF.sSCANNER_ComPort       =ReadIniData(szDir, "RS232", "Device", AnsiString(""));
    ScannerAOIIF.sSCANNER_BaudRate      =ReadIniData(szDir, "RS232", "Baud Rate", AnsiString("57600"));
    ScannerAOIIF.sSCANNER_ByteSize      =ReadIniData(szDir, "RS232", "Byte Size", AnsiString("8"));
    ScannerAOIIF.sSCANNER_StopBit       =ReadIniData(szDir, "RS232", "Stop Bit", AnsiString("1"));
    ScannerAOIIF.sSCANNER_Parity        =ReadIniData(szDir, "RS232", "Parity", AnsiString("None"));

    ScannerAOIIF.fScannerICGain         =CheckRange(ReadIniData(szDir, "SETTING", "ScannerICGain", 1.0),1.5 ,0.5);          //Ifor 20200525 add:依據Tray Y Pitch Gain 決定不使用範圍
    EL<TEdit>("TFrmAOI", "edtScannerICGain")->Text              =FormatFloat("0.00", ScannerAOIIF.fScannerICGain);

    ScannerAOIIF.bScanAOIUseLGAMode     =ReadIniData(szDir, "SETTING", "bEnabledScanAOIUseLGAMode", false);                 //Ifor 20211022 add: AOI LGA Mode
    ScannerAOIIF.fLGAWaitTime           =CheckRange(ReadIniData(szDir, "SETTING", "ScannerLGAWaitTime", 2.0),20.0 ,1.0);    //Ifor 20211022 add: AOI LGA Mode
    ScannerAOIIF.bLGAModeDirection      =ReadIniData(szDir, "SETTING", "bLGAModeDirection", true);                          //Ifor 20230202 add: AOI LGA Mode Direction
    ScannerAOIIF.bEnabledPositionByAOI  =ReadIniData(szDir, "SETTING", "bEnabledPositionByAOI", false);                     //Ifor 20251031 add: Move Position Provided By AOI

    ScannerAOIIF.iEnableTopScannerMode      =ReadIniData(szDir, "SETTING", "iEnableTopScannerMode", 0);                     //Ifor 20200902 add: TFAMD Top AOI
    ScannerAOIIF.fTopScannerReadTimeout     =ReadIniData(szDir, "SETTING", "TopScannerReadTimeout", 100);
    ScannerAOIIF.iTopIntervalCounter        =ReadIniData(szDir, "SETTING", "iTopIntervalCounter", 0);
    ScannerAOIIF.iTopRetryCounter           =ReadIniData(szDir, "SETTING", "iTopRetryCounter", 0);
    ScannerAOIIF.TopScannerIfError          =ReadIniData(szDir, "SETTING", "iTopScanAOIFialBin", 0);

    ScannerAOIIF.iTopStartDelayTime        =ReadIniData(szDir, "SETTING", "StartDelayTimeTopScanAOIView", 10);
    ScannerAOIIF.iTopTimeOut               =ReadIniData(szDir, "SETTING", "TimeOutScanTopAOIView", 3);

    ScannerAOIIF.sTopSCANNER_ComPort       =ReadIniData(szDir, "RS232", "Top Device", AnsiString(""));
    ScannerAOIIF.sTopSCANNER_BaudRate      =ReadIniData(szDir, "RS232", "Top Baud Rate", AnsiString("57600"));
    ScannerAOIIF.sTopSCANNER_ByteSize      =ReadIniData(szDir, "RS232", "Top Byte Size", AnsiString("8"));
    ScannerAOIIF.sTopSCANNER_StopBit       =ReadIniData(szDir, "RS232", "Top Stop Bit", AnsiString("1"));
    ScannerAOIIF.sTopSCANNER_Parity        =ReadIniData(szDir, "RS232", "Top Parity", AnsiString("None"));

    ScannerAOIIF.fScannerICGain         =CheckRange(ReadIniData(szDir, "SETTING", "ScannerICGain", 1.0), 1.5 ,0.5);   //Ifor 20200525 add:依據Tray Y Pitch Gain 決定不使用範圍
    EL<TEdit>("TFrmAOI", "edtScannerICGain")->Text              =FormatFloat("0.00", ScannerAOIIF.fScannerICGain);

    AnsiString AsTrayData="";
    AnsiString asBuffer="";
    bool bHasFile=true;
    if(iQuotient==0)                                                            //KenHsieh 20220830 : 修改add:AOI Tray Edit
    {
        iQuotient=_MAX_COL_ITEM/10;
        iRemainder=_MAX_COL_ITEM%10;
        if(iRemainder!=0)
            iQuotient+=1;
    }

    for(int k=0; k<iQuotient; k++)                                              //KenHsieh 20220830 : 修改add:AOI Tray Edit
    {
        S.sprintf("TrayDisable_%d", k+1);
        AsTrayData                      =ReadIniData(szDir, "AOITRAY", S, AnsiString("NA"));
        asBuffer=asBuffer+AsTrayData;
        if(AsTrayData=="NA")
            bHasFile=false;
    }
    TStringList *sList;
    sList=new TStringList();
    sList->CommaText=asBuffer;
    if(asBuffer.Length()<_MAX_COL_ITEM*_MAX_ROW_ITEM*2)                         //KenHsieh 20220830 : 修改add:AOI Tray Edit
        bHasFile=false;

    if(bHasFile==true)
    {
        for(int i=0; i<_MAX_COL_ITEM; i++)
        {
            for(int j=0; j<_MAX_ROW_ITEM; j++)
            {
                MOT[MMScanAOI].Tray.Data[i][j]=atoi(sList->Strings[i*_MAX_ROW_ITEM+j].c_str());   //KenHsieh 20220830 : 修改add:AOI Tray Edit
            }
        }
    }
    sList->Clear();
    delete sList;
    AO_fAOI_DoIniDataToForm();                                                     //Ifor 20200813 fix:移至檔案讀取後
    AO_UpDataTrayData();

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) std::printf("AOISetup: USE_Scanner_AOI_Inspection=Top&Bottom but TTopBottomInspect is not ported -- golden fAOI.cpp:3546-3574 (elParameter->ReadEditTextFromFile AOI.Data, ttbInsp) skipped (Jimmy)\n");   // golden fAOI.cpp:3546-3574 取代：elParameter->ReadEditTextFromFile（Top&Bottom 才有 elParameter）＋ ttbInsp->SetCommParameter／UpdateAOIRecipeUI／sgICSmall／sgICLarge：TTopBottomInspect 不在移植樹（ttbInsp 恆無）→ 其餘機台與 golden 相同（elParameter NULL、ttbInsp NULL 都不跑）；Top&Bottom 機台開機印一行提醒：[TopBottomInspect] 等鍵沒有讀進任何結構
#if 0 // GATE (S12-C save) golden fAOI.cpp:3546-3574 -- elParameter->ReadEditTextFromFile（Top&Bottom 才有 elParameter）＋ ttbInsp->SetCommParameter／UpdateAOIRecipeUI／sgICSmall／sgICLarge：TTopBottomInspect 不在移植樹（ttbInsp 恆無）→ 其餘機台與 golden 相同（elParameter NULL、ttbInsp NULL 都不跑）；Top&Bottom 機台開機印一行提醒：[TopBottomInspect] 等鍵沒有讀進任何結構
    if(elParameter)                                                             //AI(general) 20260411 (Rogeryang) : fix path缺Recipe子資料夾 + NULL guard
    {
        tmpS.sprintf("%s%s\\", DataPath, GetLastOpenFN());
        elParameter->ReadEditTextFromFile(tmpS, "AOI.Data");
    }

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && EL<TControl>("TFrmAOI", "ttbInsp"))
    {
        EL<TControl>("TFrmAOI", "ttbInsp")->SetCommParameter(EL<TControl>("TFrmAOI", "ttbInsp")->sSocketAddress,EL<TControl>("TFrmAOI", "ttbInsp")->sSocketPort);
        UpdateAOIRecipeUI();                                                    //Ifor 20260827 : recipe combo / label from AOI.Data
        for(int i=0;i<2;i++)
        {
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[1][i+1]=IntToStr(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].ppPosition.X);
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[2][i+1]=IntToStr(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].ppPosition.Y);
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[3][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].bClampLT)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[4][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].bClampLB)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[5][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].bClampRT)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[6][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiLessThanOrEqual65mm[i].bClampRB)?"1":"0";
        }
        for(int i=0;i<4;i++)
        {
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[1][i+1]=IntToStr(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].ppPosition.X);
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[2][i+1]=IntToStr(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].ppPosition.Y);
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[3][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].bClampLT)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[4][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].bClampLB)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[5][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].bClampRT)?"1":"0";
            EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[6][i+1]=(EL<TControl>("TFrmAOI", "ttbInsp")->tbpiMoreThan65mm[i].bClampRB)?"1":"0";
        }
    }
#endif // GATE (S12-C save)
    //Update Main Form UI
    ;   // golden fAOI.cpp:3576-3589 取代：純畫面（主畫面 fMain->labScanAOI「Scan AOI ON/OFF」字樣與字色；移植樹門面 forms/fMain.h 沒有這個元件、網頁也沒有對應 id）。判斷式裡的 FrmAOI->RunTopBottomInspect() 在移植樹恆 false（ttbInsp 不存在，golden :4782-4785 同樣回 false）
#if 0 // GATE (S12-C save) golden fAOI.cpp:3576-3589 -- 純畫面（主畫面 fMain->labScanAOI「Scan AOI ON/OFF」字樣與字色；移植樹門面 forms/fMain.h 沒有這個元件、網頁也沒有對應 id）。判斷式裡的 FrmAOI->RunTopBottomInspect() 在移植樹恆 false（ttbInsp 不存在，golden :4782-4785 同樣回 false）
    if(USE_Scanner_AOI_Inspection>(int)eBtnAOI_Uninstall && fMain)              //Jimmychiu 20240322 : Top & Bottom Inspect
    {
        if((USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode==0) ||
           (FrmAOI->RunTopBottomInspect()==false))
        {
            fMain->labScanAOI->Caption="Scan AOI OFF";
            ;   // UI-only（HTML 端處理）：fMain->labScanAOI->Font->Color=clRed;
        }
        else
        {
            fMain->labScanAOI->Caption="Scan AOI ON";
            ;   // UI-only（HTML 端處理）：fMain->labScanAOI->Font->Color=clBlue;
        }
    }
#endif // GATE (S12-C save)

    ;   // golden fAOI.cpp:3591-3608 取代：CC_KYEC_LEE 且 ttbInsp!=NULL 才跑（Eastsun 20260519 KYEC 鎖 rgActionMode、ttbInsp->iAction=1）：客戶專屬（S25）而且要 TTopBottomInspect —— 移植樹 ttbInsp 恆無，golden 在同條件下也不跑
#if 0 // GATE (S12-C save) golden fAOI.cpp:3591-3608 -- CC_KYEC_LEE 且 ttbInsp!=NULL 才跑（Eastsun 20260519 KYEC 鎖 rgActionMode、ttbInsp->iAction=1）：客戶專屬（S25）而且要 TTopBottomInspect —— 移植樹 ttbInsp 恆無，golden 在同條件下也不跑
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)               //Eastsun 20260519 KYEC 要求鎖定
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && EL<TControl>("TFrmAOI", "ttbInsp") != NULL)
        {
            if(IniConfig.bA72_FixAOIButton==1)
            {
                EL<TRadioGroup>("TFrmAOI", "rgActionMode")->Controls[0]->Enabled=false;
                EL<TRadioGroup>("TFrmAOI", "rgActionMode")->Controls[1]->Enabled=false;
                EL<TControl>("TFrmAOI", "ttbInsp")->iAction=1;
                EL<TRadioGroup>("TFrmAOI", "rgActionMode")->ItemIndex=1;
            }
            else
            {
                EL<TRadioGroup>("TFrmAOI", "rgActionMode")->Controls[0]->Enabled=true;
                EL<TRadioGroup>("TFrmAOI", "rgActionMode")->Controls[1]->Enabled=true;
            }
        }
    }
#endif // GATE (S12-C save)

}

// golden fAOI.cpp:3611  TFrmAOI::fAOI_DoIniDataToForm()
static void AO_fAOI_DoIniDataToForm()
{

    EL<TCheckBox>("TFrmAOI", "cbEnabledTopView")->Checked = tAOISetup.tTopView.bEnabled;
    EL<TEdit>("TFrmAOI", "edt_FailTopView")->Text     = tAOISetup.tTopView.iFailBin;
    EL<TEdit>("TFrmAOI", "edt_TimeoutTopView")->Text  = tAOISetup.tTopView.iTimeOut;
    EL<TEdit>("TFrmAOI", "edt_SDelayTopView")->Text   = tAOISetup.tTopView.iStartDelayTime;

    EL<TCheckBox>("TFrmAOI", "cbEnabledPADView")->Checked = tAOISetup.tPADView.bEnabled;
    EL<TEdit>("TFrmAOI", "edt_FailPADView")->Text     = tAOISetup.tPADView.iFailBin;
    EL<TEdit>("TFrmAOI", "edt_TimeoutPADView")->Text  = tAOISetup.tPADView.iTimeOut;
    EL<TEdit>("TFrmAOI", "edt_SDelayPADView")->Text   = tAOISetup.tPADView.iStartDelayTime;

    EL<TCheckBox>("TFrmAOI", "cbEnabledBGAView")->Checked = tAOISetup.tBGAView.bEnabled;
    EL<TEdit>("TFrmAOI", "edt_FailBGAView")->Text     = tAOISetup.tBGAView.iFailBin;
    EL<TEdit>("TFrmAOI", "edt_TimeoutBGAView")->Text  = tAOISetup.tBGAView.iTimeOut;
    EL<TEdit>("TFrmAOI", "edt_SDelayBGAView")->Text   = tAOISetup.tBGAView.iStartDelayTime;

    EL<TEdit>("TFrmAOI", "edt_AlarmCount")->Text         = tAOISetup.iAlarmCount;
    EL<TCheckBox>("TFrmAOI", "cbBGAFailRun")->Checked        = tAOISetup.tFailRunFunction.bEnabled;   //wei 20160617 Vitrox

    EL<TCheckBox>("TFrmAOI", "cbAutoSkipBGA")->Checked       = tAOISetup.tAutoSkipBGA.bEnabled;       //wei 20160617 Vitrox
    EL<TCheckBox>("TFrmAOI", "cbAutoSkipPadPkg")->Checked    = tAOISetup.tAutoSkipPadPkg.bEnabled;    //wei 20160617 Vitrox
    EL<TCheckBox>("TFrmAOI", "cbAOINoSort")->Checked         = tAOISetup.tAOINoSort.bEnabled;         //wei 20161207

    EL<TEdit>("TFrmAOI", "edt_PassFailBGAPADView")->Text = tAOISetup.tBGAPADView.iTestPassFailBin;   //wei  20160706
    EL<TEdit>("TFrmAOI", "edt_FailFailBGAPADView")->Text = tAOISetup.tBGAPADView.iTestFailFailBin;   //wei  20160706
    EL<TCheckBox>("TFrmAOI", "cbTesterFailBin")->Checked     = tAOISetup.tTesterFailBin.bEnabled;     //wei  20160706
    EL<TCheckBox>("TFrmAOI", "cb_VitContiuneAlarm")->Checked = tAOISetup.bEnabledContinueAlarm;       //Alick 20160712
    EL<TEdit>("TFrmAOI", "edt_ConAlarmConut")->Text      = tAOISetup.iContAlarmCountBySite;       //Alick 20160712
    EL<TEdit>("TFrmAOI", "edt_ConAlarmConut1")->Text     = tAOISetup.iContAlarmCountByArm;        //Alick 20160718
    EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite")->Checked      = tAOISetup.bEnabledBySiteBGAView;       //Alick 20160712
    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm")->Checked       = tAOISetup.bEnabledByArmBGAView;        //Alick 20160712
    EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite1")->Checked     = tAOISetup.bEnabledBySitePADView;       //Alick 20160712
    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm1")->Checked      = tAOISetup.bEnabledByArmPADView;        //Alick 20160712

    //Ifor 20191225 : add Scan AOI Alarm
    //==>
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmBySite")->Checked      = tAOISetup.bEnabledScanAOIBySiteAlarm;
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmByArm")->Checked       = tAOISetup.bEnabledScanAOIByArmAlarm;
    EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountBySite")->Text   = tAOISetup.iScanAOIAlarmCountBySite;
    EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountByArm")->Text    = tAOISetup.iScanAOIAlarmCountByArm;
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUnUseFailBin")->Checked     = tAOISetup.bEnabledScanAOIUnUseFailBin;    //Ifor 20200825 add:TF-AMD AOI Error 不丟至Fail Bin
    //<==
    //Ifor 20191225 : add Scan AOI Alarm

    //Ifor 20190724 add AMD AOI
    //==>
    EL<TRadioGroup>("TFrmAOI", "rgScannerMode")->ItemIndex        = ScannerAOIIF.iEnableScannerMode;
    EL<TEdit>("TFrmAOI", "edtScannerReadTimeout")->Text     = ((int)(ScannerAOIIF.fScannerReadTimeout*100))/100;

    EL<TEdit>("TFrmAOI", "edtAOIIntervalCounter")->Text     = ScannerAOIIF.iIntervalCounter;
    EL<TEdit>("TFrmAOI", "edtAOIRetryCounter")->Text        = ScannerAOIIF.iRetryCounter;

    EL<TRadioGroup>("TFrmAOI", "rgBallDamageType")->ItemIndex     = ScannerAOIIF.iBallDamageType;

    EL<TCheckBox>("TFrmAOI", "cbBDByTotal")->Checked            = ScannerAOIIF.bBDTotalFunction;
    EL<TCheckBox>("TFrmAOI", "cbBDByTotalConti")->Checked       = ScannerAOIIF.bBDTotalContiFunction;
    EL<TCheckBox>("TFrmAOI", "cbBDBySite")->Checked             = ScannerAOIIF.bBDSiteFunction;
    EL<TCheckBox>("TFrmAOI", "cbBDBySiteConti")->Checked        = ScannerAOIIF.bBDSiteContiFunction;
    EL<TCheckBox>("TFrmAOI", "cbDBAlarmAutoReset")->Checked     = ScannerAOIIF.bDBAlramAutoResetCount;

    EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotal")->Text      = ScannerAOIIF.iBDTotalCounter;
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotalConti")->Text = ScannerAOIIF.iBDTotalContiCounter;
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterSite")->Text       = ScannerAOIIF.iBDSiteCounter;
    EL<TEdit>("TFrmAOI", "edtAOIBDCounterSiteConti")->Text  = ScannerAOIIF.iBDSiteContiCounter;

    EL<TPanel>("TFrmAOI", "pnlAOIBDTotalLast")->Caption      = ScannerAOIIF.iBDTotalCounterLast;
    EL<TPanel>("TFrmAOI", "pnlAOIBDTotalContiLast")->Caption = ScannerAOIIF.iBDTotalContiCounterLast;
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite1Last")->Caption      = ScannerAOIIF.iBDSite1CounterLast;
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite2Last")->Caption      = ScannerAOIIF.iBDSite2CounterLast;
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite1ContiLast")->Caption = ScannerAOIIF.iBDSite1ContiCounterLast;
    EL<TPanel>("TFrmAOI", "pnlAOIBDSite2ContiLast")->Caption = ScannerAOIIF.iBDSite2ContiCounterLast;

    EL<TEdit>("TFrmAOI", "edt_SDelayScanAOI")->Text         = ScannerAOIIF.iStartDelayTime;
    EL<TEdit>("TFrmAOI", "edt_TimeoutScanAOI")->Text        = ScannerAOIIF.iTimeOut;

    filerw::ELComboText(EL<TComboBox>("TFrmAOI", "cbDevice"), ScannerAOIIF.sSCANNER_ComPort);
    filerw::ELComboText(EL<TComboBox>("TFrmAOI", "cbBaudRate"), ScannerAOIIF.sSCANNER_BaudRate);

    EL<TEdit>("TFrmAOI", "edtScannerICGain")->Text          =FormatFloat("0.00", ScannerAOIIF.fScannerICGain);  //Ifor 20200525 add:依據Tray Y Pitch Gain 決定不使用範圍

    if(ScannerAOIIF.sSCANNER_ByteSize == "5")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbByteSize"), 0);
    else if(ScannerAOIIF.sSCANNER_ByteSize == "6")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbByteSize"), 1);
    else if(ScannerAOIIF.sSCANNER_ByteSize == "7")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbByteSize"), 2);
    else if(ScannerAOIIF.sSCANNER_ByteSize == "8")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbByteSize"), 3);

    if(ScannerAOIIF.sSCANNER_StopBit == "1" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbStopBit"), 0);
    else if(ScannerAOIIF.sSCANNER_StopBit == "1.5" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbStopBit"), 1);
    else if(ScannerAOIIF.sSCANNER_StopBit == "2" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbStopBit"), 2);

    if(ScannerAOIIF.sSCANNER_Parity == "None" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbParity"), 0);
    else if(ScannerAOIIF.sSCANNER_Parity == "Odd" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbParity"), 1);
    else if(ScannerAOIIF.sSCANNER_Parity == "Even" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbParity"), 2);

    EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType")->ItemIndex     =ScannerAOIIF.iAOIFailBinType;                  //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    AO_UpdateAOIFailBinTypetoForm();                                                   //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbAOIFial"), ScannerAOIIF.ScannerIfError);
    filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass"), ScannerAOIIF.ScannerIfErrorAndTestPass);        //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    //<==
    //Ifor 20190724 add AMD AOI
    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUseLGAMode")->Checked   = ScannerAOIIF.bScanAOIUseLGAMode;              //Ifor 20211022 add: AOI LGA Mode
    EL<TCheckBox>("TFrmAOI", "cb_LGAModeDirection")->Checked    = ScannerAOIIF.bLGAModeDirection;               //Ifor 20230202 add: AOI LGA Mode Direction
    EL<TCheckBox>("TFrmAOI", "cb_EnabledPositionByAOI")->Checked= ScannerAOIIF.bEnabledPositionByAOI;           //Ifor 20251031 add: Move Position Provided By AOI
    EL<TEdit>("TFrmAOI", "edtScannerLGAWaitTime")->Text     = ((int)(ScannerAOIIF.fLGAWaitTime*100))/100;   //Ifor 20211022 add: AOI LGA Mode
    //Ifor 20200902 add: TFAMD Top AOI
    //==>
    EL<TRadioGroup>("TFrmAOI", "rgTopScannerMode")->ItemIndex         = ScannerAOIIF.iEnableTopScannerMode;
    EL<TEdit>("TFrmAOI", "edtTopAOIIntervalCounter")->Text      = ScannerAOIIF.iTopIntervalCounter;
    EL<TEdit>("TFrmAOI", "edtTopAOIRetryCounter")->Text         = ScannerAOIIF.iTopRetryCounter;
    filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopAOIFial"), ScannerAOIIF.TopScannerIfError);
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmBySite")->Checked   = tAOISetup.bEnabledTopScanAOIBySiteAlarm;
    EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountBySite")->Text= tAOISetup.iTopScanAOIAlarmCountBySite;
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmByArm")->Checked    = tAOISetup.bEnabledTopScanAOIByArmAlarm;
    EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountByArm")->Text = tAOISetup.iTopScanAOIAlarmCountByArm;
    EL<TEdit>("TFrmAOI", "edt_SDelayTopScanAOI")->Text          = ScannerAOIIF.iTopStartDelayTime;
    EL<TEdit>("TFrmAOI", "edt_TimeoutTopScanAOI")->Text         = ScannerAOIIF.iTopTimeOut;
    EL<TEdit>("TFrmAOI", "edtTopScannerReadTimeout")->Text      = ((int)(ScannerAOIIF.fTopScannerReadTimeout*100))/100;
    EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIUnUseFailBin")->Checked  = tAOISetup.bEnabledTopScanAOIUnUseFailBin;

    filerw::ELComboText(EL<TComboBox>("TFrmAOI", "cbTopDevice"), ScannerAOIIF.sTopSCANNER_ComPort);
    filerw::ELComboText(EL<TComboBox>("TFrmAOI", "cbTopBaudRate"), ScannerAOIIF.sTopSCANNER_BaudRate);

    if(ScannerAOIIF.sTopSCANNER_ByteSize == "5")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopByteSize"), 0);
    else if(ScannerAOIIF.sTopSCANNER_ByteSize == "6")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopByteSize"), 1);
    else if(ScannerAOIIF.sTopSCANNER_ByteSize == "7")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopByteSize"), 2);
    else if(ScannerAOIIF.sTopSCANNER_ByteSize == "8")
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopByteSize"), 3);

    if(ScannerAOIIF.sTopSCANNER_StopBit == "1" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopStopBit"), 0);
    else if(ScannerAOIIF.sTopSCANNER_StopBit == "1.5" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopStopBit"), 1);
    else if(ScannerAOIIF.sTopSCANNER_StopBit == "2" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopStopBit"), 2);

    if(ScannerAOIIF.sTopSCANNER_Parity == "None" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopParity"), 0);
    else if(ScannerAOIIF.sTopSCANNER_Parity == "Odd" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopParity"), 1);
    else if(ScannerAOIIF.sTopSCANNER_Parity == "Even" )
        filerw::ELComboIndex(EL<TComboBox>("TFrmAOI", "cbTopParity"), 2);
    //<==
    //Ifor 20200902 add: TFAMD Top AOI

}

// golden fAOI.cpp:4653  TFrmAOI::UpdateAOIFailBinTypetoForm()
static void AO_UpdateAOIFailBinTypetoForm()
{

    if(EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType")->ItemIndex==0)
    {
        EL<TLabel>("TFrmAOI", "lblAOIBinSel1")->Caption          ="AOI Fail :";
        EL<TLabel>("TFrmAOI", "lblAOIBinSel2")->Visible          =false;
        EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->Visible   =false;
    }
    else
    {
        EL<TLabel>("TFrmAOI", "lblAOIBinSel1")->Caption          ="AOI Fail + Test Fail :";
        EL<TLabel>("TFrmAOI", "lblAOIBinSel2")->Visible          =true;
        EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->Visible   =true;
    }

}

// golden fAOI.cpp:4630  TFrmAOI::UpDataTrayData()
static void AO_UpDataTrayData()
{

    if(InitialOK==false)
        return;

    ;   // golden fAOI.cpp:4635-4636 取代：純畫面：mtAOIBuffer（TTMyTray 盤面元件）的格數；下面兩行 MOT[MMScanAOI].Tray.X/YItem 是資料，照 golden 保留
#if 0 // GATE (S12-C save) golden fAOI.cpp:4635-4636 -- 純畫面：mtAOIBuffer（TTMyTray 盤面元件）的格數；下面兩行 MOT[MMScanAOI].Tray.X/YItem 是資料，照 golden 保留
    EL<TControl>("TFrmAOI", "mtAOIBuffer")->XItem=MOT[MManualTray2].Tray.XItem;
    EL<TControl>("TFrmAOI", "mtAOIBuffer")->YItem=MOT[MManualTray2].Tray.YItem;
#endif // GATE (S12-C save)
    MOT[MMScanAOI].Tray.XItem=MOT[MManualTray2].Tray.XItem;
    MOT[MMScanAOI].Tray.YItem=MOT[MManualTray2].Tray.YItem;
    for(int i=0; i<MOT[MMScanAOI].Tray.XItem; i++)
    {
        for(int j=0; j<MOT[MMScanAOI].Tray.YItem; j++)
        {
    ;   // golden fAOI.cpp:4643-4643 取代：純畫面：mtAOIBuffer 逐格上色（外層兩個 for 照 golden 留著，只剩空敘述）
#if 0 // GATE (S12-C save) golden fAOI.cpp:4643-4643 -- 純畫面：mtAOIBuffer 逐格上色（外層兩個 for 照 golden 留著，只剩空敘述）
            EL<TControl>("TFrmAOI", "mtAOIBuffer")->SetCellColorIndex(i, j, MOT[MMScanAOI].Tray.Data[i][j]);
#endif // GATE (S12-C save)
        }
    }

}

// golden fAOI.cpp:3899  TFrmAOI::CheckFailBin()
static bool AO_CheckFailBin()
{

    int iBin[5];

    iBin[1]=atoi(EL<TEdit>("TFrmAOI", "edt_FailPADView")->Text.c_str());
    iBin[2]=atoi(EL<TEdit>("TFrmAOI", "edt_FailBGAView")->Text.c_str());

    iBin[3]=atoi(EL<TEdit>("TFrmAOI", "edt_PassFailBGAPADView")->Text.c_str());
    iBin[4]=atoi(EL<TEdit>("TFrmAOI", "edt_FailFailBGAPADView")->Text.c_str());

    if(EL<TCheckBox>("TFrmAOI", "cbEnabledBGAView")->Checked || EL<TCheckBox>("TFrmAOI", "cbEnabledPADView")->Checked || EL<TCheckBox>("TFrmAOI", "cbEnabledTopView")->Checked)
    {
        for(int i=1; i<sizeof(iBin)/sizeof(int); i++)
        {
            if(Prod.bIsPassBin[iBin[i]])
            {
                return false;
            }
        }
    }
    return true;

}

// golden fAOI.cpp:3120  TFrmAOI::spbSaveClick(TObject *Sender)
static void AO_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden fAOI.cpp:3126-3126 取代：Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔
#if 0 // GATE (S12-C save) golden fAOI.cpp:3126-3126 -- Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔
        Close();
#endif // GATE (S12-C save)
    return;   // golden fAOI.cpp:3127-3127 取代：AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 fAOI.cpp:2950-2955：A02 分支只有 Close()（:2954），後面沒有 return，存檔照跑；主選單 main.cpp:28585 用 Show 開（非對話框），fAOI.dfm 沒綁 OnClose ⇒ Close() 只把表單藏起來，操作員改的值照樣寫進檔。V912 fAOI.cpp:3126-3127：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。Kept: #20 exception (Steven 1003 Q78, keep V912)
#if 0 // GATE (S12-C save) golden fAOI.cpp:3127-3127 -- AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 fAOI.cpp:2950-2955：A02 分支只有 Close()（:2954），後面沒有 return，存檔照跑；主選單 main.cpp:28585 用 Show 開（非對話框），fAOI.dfm 沒綁 OnClose ⇒ Close() 只把表單藏起來，操作員改的值照樣寫進檔。V912 fAOI.cpp:3126-3127：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。Kept: #20 exception (Steven 1003 Q78, keep V912)
        return;
#endif // GATE (S12-C save)
    }

    AnsiString S="";

    if(AO_CheckFailBin()==false)
    {
        filerw::ELMessage("AOI Fail Bin設定與Bin Sel Pass Bin別相同");
    }

    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\AOI.Data", DataPath, S);

    filerw::ELMark("AO_WriteIniData"); WriteIniData(szDir, "SETTING", "EnabledTopView", EL<TCheckBox>("TFrmAOI", "cbEnabledTopView")->Checked);   // golden fAOI.cpp:3141-3141 取代：存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02 會 return（沒寫檔），CheckFailBin 只提示、不 return
#if 0 // GATE (S12-C save) golden fAOI.cpp:3141-3141 -- 存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02 會 return（沒寫檔），CheckFailBin 只提示、不 return
    WriteIniData(szDir, "SETTING", "EnabledTopView", EL<TCheckBox>("TFrmAOI", "cbEnabledTopView")->Checked);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "SETTING", "FailBinTopView", EL<TEdit>("TFrmAOI", "edt_FailTopView")->Text);
    WriteIniData(szDir, "SETTING", "TimeOutTopView", EL<TEdit>("TFrmAOI", "edt_TimeoutTopView")->Text);
    WriteIniData(szDir, "SETTING", "StartDelayTimeTopView", EL<TEdit>("TFrmAOI", "edt_SDelayTopView")->Text);

    WriteIniData(szDir, "SETTING", "EnabledPADView", EL<TCheckBox>("TFrmAOI", "cbEnabledPADView")->Checked);
    WriteIniData(szDir, "SETTING", "FailBinPADView", EL<TEdit>("TFrmAOI", "edt_FailPADView")->Text);
    WriteIniData(szDir, "SETTING", "TimeOutPADView", EL<TEdit>("TFrmAOI", "edt_TimeoutPADView")->Text);
    WriteIniData(szDir, "SETTING", "StartDelayTimePADView", EL<TEdit>("TFrmAOI", "edt_SDelayPADView")->Text);

    WriteIniData(szDir, "SETTING", "EnabledBGAView", EL<TCheckBox>("TFrmAOI", "cbEnabledBGAView")->Checked);
    WriteIniData(szDir, "SETTING", "FailBinBGAView", EL<TEdit>("TFrmAOI", "edt_FailBGAView")->Text);
    WriteIniData(szDir, "SETTING", "TimeOutBGAView", EL<TEdit>("TFrmAOI", "edt_TimeoutBGAView")->Text);
    WriteIniData(szDir, "SETTING", "StartDelayTimeBGAView", EL<TEdit>("TFrmAOI", "edt_SDelayBGAView")->Text);

    WriteIniData(szDir, "SETTING", "AlarmCount", EL<TEdit>("TFrmAOI", "edt_AlarmCount")->Text);

    WriteIniData(szDir, "SETTING", "EnabledailRunFunction", EL<TCheckBox>("TFrmAOI", "cbBGAFailRun")->Checked);
    WriteIniData(szDir, "SETTING", "EnabledAutoSkipBGA",    EL<TCheckBox>("TFrmAOI", "cbAutoSkipBGA")->Checked);
    WriteIniData(szDir, "SETTING", "EnabledAutoSkipPadPkg", EL<TCheckBox>("TFrmAOI", "cbAutoSkipPadPkg")->Checked);

    WriteIniData(szDir, "SETTING", "EnabledAOINoSort",      EL<TCheckBox>("TFrmAOI", "cbAOINoSort")->Checked);   //wei 20161207

    WriteIniData(szDir, "SETTING", "TestPassFailBinBGAPADView", EL<TEdit>("TFrmAOI", "edt_PassFailBGAPADView")->Text);    //wei  20160706
    WriteIniData(szDir, "SETTING", "TestFailFailBinBGAPADView", EL<TEdit>("TFrmAOI", "edt_FailFailBGAPADView")->Text);    //wei  20160706

    WriteIniData(szDir, "SETTING", "EnabledTesterFailBin",   EL<TCheckBox>("TFrmAOI", "cbTesterFailBin")->Checked);     //wei  20160706
    WriteIniData(szDir, "SETTING", "EnabledContinueAlarm",   EL<TCheckBox>("TFrmAOI", "cb_VitContiuneAlarm")->Checked); //Alick 20160712
    WriteIniData(szDir, "SETTING", "EnabledBySiteBGAView",   EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite")->Checked);      //Alick 20160712
    WriteIniData(szDir, "SETTING", "EnabledByArmBGAView",    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm")->Checked);       //Alick 20160712
    WriteIniData(szDir, "SETTING", "ContAlarmCountBySite",   EL<TEdit>("TFrmAOI", "edt_ConAlarmConut")->Text);      //Alick 20160712
    WriteIniData(szDir, "SETTING", "ContAlarmCountByArm",    EL<TEdit>("TFrmAOI", "edt_ConAlarmConut1")->Text);     //Alick 20160718
    WriteIniData(szDir, "SETTING", "EnabledBySitePADView",   EL<TCheckBox>("TFrmAOI", "cb_AlarmBySite1")->Checked);     //Alick 20160712
    WriteIniData(szDir, "SETTING", "EnabledByArmPADView",    EL<TCheckBox>("TFrmAOI", "cb_AlarmByArm1")->Checked);      //Alick 20160712

    //Ifor 20191225 : add Scan AOI Alarm
    //==>
    WriteIniData(szDir, "SETTING", "bEnabledScanAOIBySiteAlarm",    EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmBySite")->Checked);
    WriteIniData(szDir, "SETTING", "bEnabledScanAOIByArmAlarm",     EL<TCheckBox>("TFrmAOI", "cb_ScanAOIAlarmByArm")->Checked);
    WriteIniData(szDir, "SETTING", "iScanAOIAlarmCountBySite",      EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountBySite")->Text);
    WriteIniData(szDir, "SETTING", "iScanAOIAlarmCountByArm",       EL<TEdit>("TFrmAOI", "edt_ScanAOIAlarmCountByArm")->Text);

    WriteIniData(szDir, "SETTING", "bEnabledScanAOIUnUseFailBin",   EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUnUseFailBin")->Checked);   //Ifor 20200825 add:TF-AMD AOI Error 不丟至Fail Bin
    //<==
    //Ifor 20191225 : add Scan AOI Alarm

    for(int i=0; i<MAX_ARM_Row; i++)                                                              //Alick 20160713
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            S.sprintf("Dut %s", IndexSuckName[i][j]);
            WriteIniData(szDir, "DutOnOff_BGAView", S, bVitroxBGAViewUse[i][j]);
            bVitroxBGAViewMap[i][j]=bVitroxBGAViewUse[i][j];
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)                                                              //Alick 20160714
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            S.sprintf("Dut %s", IndexSuckName[i][j]);
            WriteIniData(szDir, "DutOnOff_PADView", S, bVitroxPADViewUse[i][j]);
            bVitroxPADViewMap[i][j]=bVitroxPADViewUse[i][j];
        }
    }

    //Ifor 20190724 add AMD AOI
    //==>
    WriteIniData(szDir, "SETTING", "iEnableScannerMode",        EL<TRadioGroup>("TFrmAOI", "rgScannerMode")->ItemIndex);
    WriteIniData(szDir, "SETTING", "ScannerReadTimeout",        EL<TEdit>("TFrmAOI", "edtScannerReadTimeout")->Text);

    WriteIniData(szDir, "SETTING", "iIntervalCounter",          EL<TEdit>("TFrmAOI", "edtAOIIntervalCounter")->Text);
    WriteIniData(szDir, "SETTING", "iRetryCounter",             EL<TEdit>("TFrmAOI", "edtAOIRetryCounter")->Text);
    WriteIniData(szDir, "SETTING", "iAOIFailBinType",           EL<TRadioGroup>("TFrmAOI", "rgAOIFailBinType")->ItemIndex);       //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
    WriteIniData(szDir, "SETTING", "iScanAOIFialBin",           EL<TComboBox>("TFrmAOI", "cbAOIFial")->ItemIndex);
    WriteIniData(szDir, "SETTING", "iAOIFialAndTestPass",       EL<TComboBox>("TFrmAOI", "cbAOIFialAndTestPass")->ItemIndex);   //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項

    WriteIniData(szDir, "SETTING", "bBDTotalFunction",          EL<TCheckBox>("TFrmAOI", "cbBDByTotal")->Checked);
    WriteIniData(szDir, "SETTING", "bBDTotalContiFunction",     EL<TCheckBox>("TFrmAOI", "cbBDByTotalConti")->Checked);
    WriteIniData(szDir, "SETTING", "bBDSiteFunction",           EL<TCheckBox>("TFrmAOI", "cbBDBySite")->Checked);
    WriteIniData(szDir, "SETTING", "bBDSiteContiFunction",      EL<TCheckBox>("TFrmAOI", "cbBDBySiteConti")->Checked);
    WriteIniData(szDir, "SETTING", "bDBAlramAutoResetCount",    EL<TCheckBox>("TFrmAOI", "cbDBAlarmAutoReset")->Checked);

    WriteIniData(szDir, "SETTING", "iBDTotalCounter",           EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotal")->Text);
    WriteIniData(szDir, "SETTING", "iBDTotalContiCounter",      EL<TEdit>("TFrmAOI", "edtAOIBDCounterTotalConti")->Text);
    WriteIniData(szDir, "SETTING", "iBDSiteCounter",            EL<TEdit>("TFrmAOI", "edtAOIBDCounterSite")->Text);
    WriteIniData(szDir, "SETTING", "iBDSiteContiCounter",       EL<TEdit>("TFrmAOI", "edtAOIBDCounterSiteConti")->Text);

    WriteIniData(szDir, "SETTING", "iBDTotalCounterLast",       EL<TPanel>("TFrmAOI", "pnlAOIBDTotalLast")->Caption);
    WriteIniData(szDir, "SETTING", "iBDTotalContiCounterLast",  EL<TPanel>("TFrmAOI", "pnlAOIBDTotalContiLast")->Caption);
    WriteIniData(szDir, "SETTING", "iBDSite1CounterLast",       EL<TPanel>("TFrmAOI", "pnlAOIBDSite1Last")->Caption);
    WriteIniData(szDir, "SETTING", "iBDSite2CounterLast",       EL<TPanel>("TFrmAOI", "pnlAOIBDSite2Last")->Caption);
    WriteIniData(szDir, "SETTING", "iBDSite1ContiCounterLast",  EL<TPanel>("TFrmAOI", "pnlAOIBDSite1ContiLast")->Caption);
    WriteIniData(szDir, "SETTING", "iBDSite2ContiCounterLast",  EL<TPanel>("TFrmAOI", "pnlAOIBDSite2ContiLast")->Caption);

    WriteIniData(szDir, "SETTING", "StartDelayTimeScanAOIView", EL<TEdit>("TFrmAOI", "edt_SDelayScanAOI")->Text);
    WriteIniData(szDir, "SETTING", "TimeOutScanAOIView", EL<TEdit>("TFrmAOI", "edt_TimeoutScanAOI")->Text);

    WriteIniData(szDir, "RS232",   "Device",                    EL<TComboBox>("TFrmAOI", "cbDevice")->Text);
    WriteIniData(szDir, "RS232",   "Baud Rate",                 EL<TComboBox>("TFrmAOI", "cbBaudRate")->Text);
    WriteIniData(szDir, "RS232",   "Byte Size",                 EL<TComboBox>("TFrmAOI", "cbByteSize")->Text);
    WriteIniData(szDir, "RS232",   "Stop Bit",                  EL<TComboBox>("TFrmAOI", "cbStopBit")->Text);
    WriteIniData(szDir, "RS232",   "Parity",                    EL<TComboBox>("TFrmAOI", "cbParity")->Text);

    WriteIniData(szDir, "SETTING", "ScannerICGain",             EL<TEdit>("TFrmAOI", "edtScannerICGain")->Text);        //Ifor 20200525 add:依據Tray Y Pitch Gain 決定不使用範圍

    WriteIniData(szDir, "SETTING", "bEnabledScanAOIUseLGAMode", EL<TCheckBox>("TFrmAOI", "cb_ScanAOIUseLGAMode")->Checked); //Ifor 20211022 add: AOI LGA Mode
    WriteIniData(szDir, "SETTING", "ScannerLGAWaitTime",        EL<TEdit>("TFrmAOI", "edtScannerLGAWaitTime")->Text);   //Ifor 20211022 add: AOI LGA Mode

    WriteIniData(szDir, "SETTING", "bLGAModeDirection", EL<TCheckBox>("TFrmAOI", "cb_LGAModeDirection")->Checked);          //Ifor 20230202 add: AOI LGA Mode Direction
    WriteIniData(szDir, "SETTING", "bEnabledPositionByAOI", EL<TCheckBox>("TFrmAOI", "cb_EnabledPositionByAOI")->Checked);              //Ifor 20251031 add: Move Position Provided By AOI
    //<==
    //Ifor 20190724 add AMD AOI

    //Ifor 20211027 add:AOI Tray Edit
    //==>
    AnsiString AsTrayData="";

    for(int k=0; k<iQuotient; k++)                                              //KenHsieh 20220830 : 修改add:AOI Tray Edit
    {
        AsTrayData="";
        if(iRemainder==0 || k!=iQuotient-1)
        {
            for(int i=0; i<10; i++)
            {
                for(int j=0; j<_MAX_ROW_ITEM; j++)
                {
                    AsTrayData=AsTrayData+MOT[MMScanAOI].Tray.Data[i+(k*10)][j]+",";
                }
            }
        }
        else
        {
            for(int i=0; i<iRemainder; i++)
            {
                for(int j=0; j<_MAX_ROW_ITEM; j++)
                {
                    AsTrayData=AsTrayData+MOT[MMScanAOI].Tray.Data[i+(k*10)][j]+",";
                }
            }
        }

        S.sprintf("TrayDisable_%d", k+1);
        WriteIniData(szDir, "AOITRAY", S, AsTrayData);                          //Ifor 20211022 add: AOI LGA Mode
    }
    //<==
    //Ifor 20211027 add:AOI Tray Edit

    //Ifor 20200902 add: TFAMD Top AOI
    //==>
    WriteIniData(szDir, "SETTING", "iEnableTopScannerMode",     EL<TRadioGroup>("TFrmAOI", "rgTopScannerMode")->ItemIndex);
    WriteIniData(szDir, "SETTING", "iTopIntervalCounter",       EL<TEdit>("TFrmAOI", "edtTopAOIIntervalCounter")->Text);
    WriteIniData(szDir, "SETTING", "iTopRetryCounter",          EL<TEdit>("TFrmAOI", "edtTopAOIRetryCounter")->Text);
    WriteIniData(szDir, "SETTING", "iTopScanAOIFialBin",        EL<TComboBox>("TFrmAOI", "cbTopAOIFial")->ItemIndex);

    WriteIniData(szDir, "SETTING", "bEnabledTopScanAOIBySiteAlarm", EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmBySite")->Checked);
    WriteIniData(szDir, "SETTING", "iTopScanAOIAlarmCountBySite",   EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountBySite")->Text);
    WriteIniData(szDir, "SETTING", "bEnabledTopScanAOIByArmAlarm",  EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIAlarmByArm")->Checked);
    WriteIniData(szDir, "SETTING", "iTopScanAOIAlarmCountByArm",    EL<TEdit>("TFrmAOI", "edt_TopScanAOIAlarmCountByArm")->Text);
    WriteIniData(szDir, "SETTING", "StartDelayTimeTopScanAOIView",  EL<TEdit>("TFrmAOI", "edt_SDelayTopScanAOI")->Text);   // golden fAOI.cpp:3300-3300 取代：AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 把 Top 的延遲 edt_SDelayTopScanAOI 寫進 Bottom 的鍵 "StartDelayTimeScanAOIView"（蓋掉 :3236 剛寫的 Bottom 值），讀檔 :3494 讀 Top 用 "StartDelayTimeTopScanAOIView" （golden 沒有人寫）→ 改寫讀檔用的鍵；讀檔照 golden。AOI.Data 格式從此跟 BCB6 機台不一致（BCB6 的舊檔讀起來跟 BCB6 相同），V912 不改
#if 0 // GATE (S12-C save) golden fAOI.cpp:3300-3300 -- AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 把 Top 的延遲 edt_SDelayTopScanAOI 寫進 Bottom 的鍵 "StartDelayTimeScanAOIView"（蓋掉 :3236 剛寫的 Bottom 值），讀檔 :3494 讀 Top 用 "StartDelayTimeTopScanAOIView" （golden 沒有人寫）→ 改寫讀檔用的鍵；讀檔照 golden。AOI.Data 格式從此跟 BCB6 機台不一致（BCB6 的舊檔讀起來跟 BCB6 相同），V912 不改
    WriteIniData(szDir, "SETTING", "StartDelayTimeScanAOIView",     EL<TEdit>("TFrmAOI", "edt_SDelayTopScanAOI")->Text);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "SETTING", "TimeOutScanTopAOIView",         EL<TEdit>("TFrmAOI", "edt_TimeoutTopScanAOI")->Text);   // golden fAOI.cpp:3301-3301 取代：AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 寫 "TimeOutTopScanAOIView"、讀檔 :3495 讀 "TimeOutScanTopAOIView" → Top Timeout 存了讀不回來。鍵名統一成讀檔的 "TimeOutScanTopAOIView"（改寫這一行；讀檔照 golden）：BCB6 舊檔讀起來跟 BCB6 相同（預設 3），906 存的檔 BCB6 也讀得到；BCB6 寫的 "TimeOutTopScanAOIView" 從此沒有人讀也沒有人寫。選哪個鍵名是 R 題（Steven 可推翻，tools/editlist/AOISetup.py 檔頭），V912 不改
#if 0 // GATE (S12-C save) golden fAOI.cpp:3301-3301 -- AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 寫 "TimeOutTopScanAOIView"、讀檔 :3495 讀 "TimeOutScanTopAOIView" → Top Timeout 存了讀不回來。鍵名統一成讀檔的 "TimeOutScanTopAOIView"（改寫這一行；讀檔照 golden）：BCB6 舊檔讀起來跟 BCB6 相同（預設 3），906 存的檔 BCB6 也讀得到；BCB6 寫的 "TimeOutTopScanAOIView" 從此沒有人讀也沒有人寫。選哪個鍵名是 R 題（Steven 可推翻，tools/editlist/AOISetup.py 檔頭），V912 不改
    WriteIniData(szDir, "SETTING", "TimeOutTopScanAOIView",         EL<TEdit>("TFrmAOI", "edt_TimeoutTopScanAOI")->Text);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "SETTING", "TopScannerReadTimeout",         EL<TEdit>("TFrmAOI", "edtTopScannerReadTimeout")->Text);
    WriteIniData(szDir, "SETTING", "bEnabledTopScanAOIUnUseFailBin",EL<TCheckBox>("TFrmAOI", "cb_TopScanAOIUnUseFailBin")->Checked);

    WriteIniData(szDir, "RS232",   "Top Device",                    EL<TComboBox>("TFrmAOI", "cbTopDevice")->Text);
    WriteIniData(szDir, "RS232",   "Top Baud Rate",                 EL<TComboBox>("TFrmAOI", "cbTopBaudRate")->Text);
    WriteIniData(szDir, "RS232",   "Top Byte Size",                 EL<TComboBox>("TFrmAOI", "cbTopByteSize")->Text);
    WriteIniData(szDir, "RS232",   "Top Stop Bit",                  EL<TComboBox>("TFrmAOI", "cbTopStopBit")->Text);
    WriteIniData(szDir, "RS232",   "Top Parity",                    EL<TComboBox>("TFrmAOI", "cbTopParity")->Text);
    //<==
    //Ifor 20200902 add: TFAMD Top AOI

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) filerw::ELTodo("golden fAOI.cpp:3313-3329 Top&Bottom AOI grid -> edlsSamll/edlsLarge not ported (TTopBottomInspect / elParameter, Jimmy)");   // golden fAOI.cpp:3313-3329 取代：Top&Bottom：sgICSmall／sgICLarge 格子 → edlsSamll／edlsLarge（建構子 InitialEdList 建的 TEdit* 陣列，elParameter 註冊的元件）。TTopBottomInspect 與 elParameter 都不在移植樹 → Top&Bottom 機台回報 todo，其餘機台 golden 本來就不跑
#if 0 // GATE (S12-C save) golden fAOI.cpp:3313-3329 -- Top&Bottom：sgICSmall／sgICLarge 格子 → edlsSamll／edlsLarge（建構子 InitialEdList 建的 TEdit* 陣列，elParameter 註冊的元件）。TTopBottomInspect 與 elParameter 都不在移植樹 → Top&Bottom 機台回報 todo，其餘機台 golden 本來就不跑
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)                    //Jimmychiu 20240322 : Top & Bottom Inspect
    {
        for(int i=0;i<2;i++)
        {
            for(int j=0;j<6;j++)
            {
                edlsSamll[i][j]->Text=EL<filerw::ELStringGrid>("TFrmAOI", "sgICSmall")->Cells[j+1][i+1];
            }
        }
        for(int i=0;i<4;i++)
        {
            for(int j=0;j<6;j++)
            {
                edlsLarge[i][j]->Text=EL<filerw::ELStringGrid>("TFrmAOI", "sgICLarge")->Cells[j+1][i+1];
            }
        }
    }
#endif // GATE (S12-C save)

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) filerw::ELTodo("golden fAOI.cpp:3331-3336 elParameter->SaveEditTextToFile(AOI.Data) not ported: Top&Bottom keys ([TopBottomInspect]/[ZPickOffset]/[Function Setting]) kept as on disk (TTopBottomInspect, Jimmy)");   // golden fAOI.cpp:3331-3336 取代：elParameter->SaveEditTextToFile(<配方>\, "AOI.Data")：elParameter 只有 Top&Bottom 建構子才 new（建構子 :182 設 NULL）→ 其餘機台 golden 跳過。Top&Bottom 的 ttbInsp 欄位（[TopBottomInspect]／[ZPickOffset]／[Function Setting] 共 70 多鍵）不在移植樹 → 回報 todo，那些鍵保持檔案原值
#if 0 // GATE (S12-C save) golden fAOI.cpp:3331-3336 -- elParameter->SaveEditTextToFile(<配方>\, "AOI.Data")：elParameter 只有 Top&Bottom 建構子才 new（建構子 :182 設 NULL）→ 其餘機台 golden 跳過。Top&Bottom 的 ttbInsp 欄位（[TopBottomInspect]／[ZPickOffset]／[Function Setting] 共 70 多鍵）不在移植樹 → 回報 todo，那些鍵保持檔案原值
    if(elParameter)                                                             //AI(general) 20260411 (Rogeryang) : fix path缺Recipe子資料夾 + NULL guard
    {
        AnsiString sSavePath;
        sSavePath.sprintf("%s%s\\", DataPath, GetLastOpenFN());
        elParameter->SaveEditTextToFile(sSavePath, "AOI.Data");
    }
#endif // GATE (S12-C save)

    AO_fAOI_ReadFile();
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

    if(USE_Scanner_AOI_Inspection==true) filerw::ELTodo("golden fAOI.cpp:3341-3351 Scanner AOI RS232 stop/start after save (spbStopCom/spbStartCom) not ported (device comm, Jimmy)");   // golden fAOI.cpp:3341-3351 取代：Scanner AOI 的 RS232 起停（spbStopCom／spbStartCom → AOIComm->StopComm／RS232Init）：外部設備通訊，S48 歸 Jimmy。條件照 golden
#if 0 // GATE (S12-C save) golden fAOI.cpp:3341-3351 -- Scanner AOI 的 RS232 起停（spbStopCom／spbStartCom → AOIComm->StopComm／RS232Init）：外部設備通訊，S48 歸 Jimmy。條件照 golden
    if(USE_Scanner_AOI_Inspection==true)
    {
        if(ScannerAOIIF.iEnableScannerMode==0)
        {
            FrmAOI->spbStopCom->Click();
        }
        else
        {
            FrmAOI->spbStartCom->Click();
        }
    }
#endif // GATE (S12-C save)

    if(USE_Top_Scanner_AOI_Inspection==true) filerw::ELTodo("golden fAOI.cpp:3353-3363 Top Scanner AOI RS232 stop/start after save not ported (device comm, Jimmy)");   // golden fAOI.cpp:3353-3363 取代：Top Scanner AOI 的 RS232 起停（同上；golden 按的也是 spbStopCom／spbStartCom）：S48 歸 Jimmy。條件照 golden
#if 0 // GATE (S12-C save) golden fAOI.cpp:3353-3363 -- Top Scanner AOI 的 RS232 起停（同上；golden 按的也是 spbStopCom／spbStartCom）：S48 歸 Jimmy。條件照 golden
    if(USE_Top_Scanner_AOI_Inspection==true)                                    //Ifor 20200902 add: TFAMD Top AOI
    {
        if(ScannerAOIIF.iEnableTopScannerMode==0)
        {
            FrmAOI->spbStopCom->Click();
        }
        else
        {
            FrmAOI->spbStartCom->Click();
        }
    }
#endif // GATE (S12-C save)

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) filerw::ELTodo("golden fAOI.cpp:3365-3366 ttbInsp->DoCCDLightDown after save not ported (TTopBottomInspect, Jimmy)");   // golden fAOI.cpp:3365-3366 取代：RunTopBottomInspect()（ttbInsp && Top&Bottom && ttbInsp->iEnable==1）→ ttbInsp->DoCCDLightDown：CCD 燈（外部設備），S48 歸 Jimmy；ttbInsp 不在移植樹 → Top&Bottom 機台回報 todo
#if 0 // GATE (S12-C save) golden fAOI.cpp:3365-3366 -- RunTopBottomInspect()（ttbInsp && Top&Bottom && ttbInsp->iEnable==1）→ ttbInsp->DoCCDLightDown：CCD 燈（外部設備），S48 歸 Jimmy；ttbInsp 不在移植樹 → Top&Bottom 機台回報 todo
    if(FrmAOI->RunTopBottomInspect())
        FrmAOI->ttbInsp->DoCCDLightDown(false, true);
#endif // GATE (S12-C save)

}

