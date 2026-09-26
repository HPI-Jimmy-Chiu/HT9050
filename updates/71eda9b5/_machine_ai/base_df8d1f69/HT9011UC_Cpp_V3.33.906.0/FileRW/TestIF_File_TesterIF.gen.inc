// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_TesterIF 的讀寫檔（HTEditList： → Tester.Data、D:\RS232Standard\System\Setup.ini（RS232 模式，CheckRs232StandardIni））。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cTesterIF.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 119 個（header 宣告 240 個）。
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
#include "MessageDef.h"
#include "forms/fMain.h"
#include "forms/fSecurity.h"
#include "forms/fLotInfo.h"
#include "forms/fSCKART.h"
#include "forms/fTesterTCP.h"
#include "forms/fShowBinSelect.h"
#include "forms/fTesterIF.h"
#include "csystem.h"
#include "cinitial.h"
#include "BarcodeReader.h"
#include "myswitch.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

#include <windows.h>   // FindFirstFile（golden InitcbDIOType :76、ReadTestIFFile :825）
#include <cstdio>      // sprintf（golden ShowTTLState）
#include <cstring>     // strcmp
extern bool bAuthCriticalPara[26];            // cAuthority.h:64（golden cAuthority.h；cAuthority.h 帶 language.h，與 HTEditList.h 衝突）
void FileRW_TTLCfg_TFTestIF_cbDIOTypeChange(); // FileRW/TTLCfg.cpp（golden cTesterIF.cpp:1301-1303 GetDIOFileName＋LoadData）
// golden ShowTTLState 讀的 fDIOFrom 六個元件（見 members TIF_DIOFrom）
struct TIF_DIOFromView { TRadioGroup *rgStartLogic, *rgStartChannel; TComboBox *cbSignalType; TRadioGroup *rgBinLogic, *rgBinBitLength, *rgBinDataType; };

// golden cTesterIF.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;                 // golden cTesterIF.h:302 —— 本 TU 自己的（見檔頭：不寫 FTestIF->fShow）
#define bIsResetRs232Standard (FTestIF->bIsResetRs232Standard)   // golden cTesterIF.h:303 —— golden main.cpp:18520 計時器讀 FTestIF 這一份
static int ioldTestType=-1;              // golden cTesterIF.h:304（建構子 :44 設 -1；FormShow :331 記）
static int ioldDIOType=-1;               // golden cTesterIF.h:305
static bool bflag=false;                 // golden cTesterIF.cpp:38（file-scope static；rgInterfaceTypeClick 用，golden FormClose :1219 清）
static bool bDioListLost=false;          // FileRW：golden InitcbDIOType 找不到 DIO 檔時 Application->Terminate() → 網頁版拒存（見 replace）
static void TIF_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }
static bool TIF_AOIFailBinUse() { filerw::ELTodo("golden cTesterIF.cpp:937 FrmAOI->bCheckAOIFailBinUse() (CC_KYEC_LEE AOI fail bin -> 33 bins) not ported (port TFrmAOI has none) -- treated as false"); return false; }
static TIF_DIOFromView* TIF_DIOFrom() { static TIF_DIOFromView v; v.rgStartLogic=filerw::EL<TRadioGroup>("TfDIOFrom", "rgStartLogic"); v.rgStartChannel=filerw::EL<TRadioGroup>("TfDIOFrom", "rgStartChannel"); v.cbSignalType=filerw::EL<TComboBox>("TfDIOFrom", "cbSignalType"); v.rgBinLogic=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinLogic"); v.rgBinBitLength=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength"); v.rgBinDataType=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinDataType"); return &v; }
#define fDIOFrom TIF_DIOFrom()   // golden ShowTTLState :1244-1277（FileRW/TestIF_File_TesterIF.cpp 用完 #undef）
static const char* const kTIF_ToDoubleTexts[] = {"edInitialMaxTest", "edMaxTestTime", "edDummyTestTime", "edStartDelayTime", "edtInitStartDelay", "edtInitWaitTime", "edtTestingWaitTime", "edtAfterTestedDelay", "edtInitialDelay_1", "edtInitialDelay_2", "edtInitialDelay_3", "edtInitialDelay_4", "edtInitialDelay_5", "edtInitialDelay_6", "iTestFinishToNextTestOver", "edtInitialDelay_8", "edtInitialDelay_9", "iTeststartToNextTestStart", "edtInitialDelay_10", "edtInitialDec1", "edtInitialDec2", "edtInitialDec3", "edtInitialDec4", "edtInitialDelay_1_RT", "edtInitialDelay_2_RT", "edtInitialDelay_3_RT", "edtInitialDelay_4_RT", "edtInitialDelay_5_RT", "edtInitialDelay_6_RT", "edtInitialDelay_7_RT", "edtInitialDelay_8_RT", "edtInitialDelay_9_RT", "edtInitialDelay_10_RT"};   // golden SaveSetupFile ->Text.ToDouble()

using filerw::EL;

// 前置宣告
static void TIF_TFTestIF();
static void TIF_InitcbDIOType(bool bAlarm);
static void TIF_FormShow();
static void TIF_SaveSetupFile(AnsiString szDir, AnsiString S);
static void TIF_CheckRs232StandardIni();
static void TIF_ReadTestIFFile();
static void TIF_DoIniDataToForm();
static void TIF_rgInterfaceTypeClick();
static void TIF_ShowPageControl2(int Index);
static void TIF_ShowTTLState();
static void TIF_cbDIOTypeChange();
static void TIF_AntiSignalCBoxClick();
static void TIF_cbRs232TypeChange();
static void TIF_spbSaveClick();

// golden cTesterIF.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void TIF_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TFTestIF", "cbGPIBType")->Items;
      s->Add("Advan type1");
      s->Add("255 Bin");
      s->Add("16 Bin");
      s->Add("32 Bin");
      s->Add("SPEA Type");
      s->Add("16 Bin GS");
      s->Add("32 Bin GS");
      s->Add("Advan T6577");
      s->Add("Qrovo Protocol");
      s->Add("Delta_Castle");
    }
    { TStringList* s = EL<TComboBox>("TFTestIF", "cbRs232Type")->Items;
      s->Add("Standard");
    }
    { TStringList* s = EL<TComboBox>("TFTestIF", "cbbBaudRate")->Items;
      s->Add("4800");
      s->Add("7200");
      s->Add("9600");
      s->Add("14400");
      s->Add("19200");
      s->Add("115200");
    }
    { TStringList* s = EL<TComboBox>("TFTestIF", "iStartDelayCount")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rg2DID_Format")->Items;
      s->Add("Standard");
      s->Add("AMD format");
      s->Add("Intel format");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rgBaudRate")->Items;
      s->Add("19200");
      s->Add("9600");
      s->Add("4800");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rgBitLength")->Items;
      s->Add("7 Bits");
      s->Add("8 Bits");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->Items;
      s->Add("DIO");
      s->Add("GP-IB");
      s->Add("RS232");
      s->Add("TCP/IP");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rgParity")->Items;
      s->Add("Even");
      s->Add("Odd");
      s->Add("None");
    }
    { TStringList* s = EL<TRadioGroup>("TFTestIF", "rgStopBit")->Items;
      s->Add("1 Bit");
      s->Add("2Bits");
    }
}

// golden cTesterIF.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kTIF_ParentOf[][2] = {
    {"AntiSignalCBox", "grpTTL"},
    {"Label1", "palTime"},
    {"Label12", "palTime"},
    {"Label2", "palTime"},
    {"Label49", "palTime"},
    {"Label52", "palTime"},
    {"Label8", "palTime"},
    {"PageControl1", "pnlTimeSetting"},
    {"PageControl2", "grpIFType"},
    {"Panel1", "tsRT"},
    {"Panel2", "tsEQC"},
    {"Panel39", "gbInitialDelay"},
    {"cbASEJPMode", "grpTTL"},
    {"cbAfterAutoCleanFunctionUseInitialDelay", "pnlAutoClean"},
    {"cbAfterOpenHeatDoorUseInitialDelay", "pnlHeatDoor"},
    {"cbAfterShowAlarmMessageUseInitialDelay", "pnlAlarmMsg"},
    {"cbAutoOnecycleHomStart", "tsGpib"},
    {"cbDIOType", "grpTTLSetting"},
    {"cbEveryFirstDeviceUseInitialDelay", "pnl1stDevice"},
    {"cbForEgistec", "grpRS232Type"},
    {"cbGPIBType", "grpGPIBType"},
    {"cbOTDUnlockDelay", "pnlOTD"},
    {"cbPurgeAir", "rgPurgeAir"},
    {"cbRs232Type", "grpRS232Type"},
    {"cbSpiroxTesterLotEnd", "grpGPIB"},
    {"cbTestFinishToNextTestOver", "pnlEOTTime"},
    {"cbTestStartToNextTestStart", "pnlSOTTime"},
    {"cbUseOtherArmToTestAfterInitialDelay", "Panel39"},
    {"cbUseSocketHeating", "gbInitialDelay"},
    {"cbWhenHappenTestedTimeBelowUseInitialDelay", "pnlLowTestTime"},
    {"cbWhenNoFullSiteUseInitialDelay", "pnlNoFullsite"},
    {"cbWhenPressStopOverUseInitialDelay", "pnlPause"},
    {"cbbBaudRate", "gbBaudRate"},
    {"edBelowSec", "pnlLowTestTime"},
    {"edDummyTestTime", "palTime"},
    {"edDummyTestTime_EQC", "tsEQC"},
    {"edDummyTestTime_RT", "tsRT"},
    {"edGPIBAddress", "grpGPIBType"},
    {"edInitialMaxTest", "palTime"},
    {"edInitialMaxTest_EQC", "tsEQC"},
    {"edInitialMaxTest_RT", "tsRT"},
    {"edMaxBinCount", "gbRs232BinCount"},
    {"edMaxTestTime", "palTime"},
    {"edMaxTestTime_EQC", "tsEQC"},
    {"edMaxTestTime_RT", "tsRT"},
    {"edOverSec", "pnlPause"},
    {"edPurgeAir", "rgPurgeAir"},
    {"edStartDelayTime", "palStartDelay"},
    {"edStartDelayTime_EQC", "Panel2"},
    {"edStartDelayTime_RT", "Panel1"},
    {"edTCPIP_Address", "tsTCPIP"},
    {"edTCPIP_Port", "tsTCPIP"},
    {"edtAfterTestedDelay", "palTime"},
    {"edtAutoOnecycleHomStartTime", "tsGpib"},
    {"edtInitStartDelay", "palTime"},
    {"edtInitStartDelayCT", "palTime"},
    {"edtInitStartDelayCT_EQC", "tsEQC"},
    {"edtInitStartDelayCT_RT", "tsRT"},
    {"edtInitStartDelay_EQC", "tsEQC"},
    {"edtInitStartDelay_RT", "tsRT"},
    {"edtInitWaitTime", "grpTestingStopTime"},
    {"edtInitialDec1", "rgTime2"},
    {"edtInitialDec2", "rgTime2"},
    {"edtInitialDec3", "rgTime2"},
    {"edtInitialDec4", "rgTime2"},
    {"edtInitialDecCount1", "rgTime2"},
    {"edtInitialDecCount2", "rgTime2"},
    {"edtInitialDecCount3", "rgTime2"},
    {"edtInitialDecCount4", "rgTime2"},
    {"edtInitialDelay_1", "pnl1stDevice"},
    {"edtInitialDelay_10", "pnlSOTTime"},
    {"edtInitialDelay_10_RT", "pnlSOTTime"},
    {"edtInitialDelay_1_RT", "pnl1stDevice"},
    {"edtInitialDelay_2", "pnlAlarmMsg"},
    {"edtInitialDelay_2_RT", "pnlAlarmMsg"},
    {"edtInitialDelay_3", "pnlAutoClean"},
    {"edtInitialDelay_3_RT", "pnlAutoClean"},
    {"edtInitialDelay_4", "pnlLowTestTime"},
    {"edtInitialDelay_4_RT", "pnlLowTestTime"},
    {"edtInitialDelay_5", "pnlHeatDoor"},
    {"edtInitialDelay_5_RT", "pnlHeatDoor"},
    {"edtInitialDelay_6", "pnlPause"},
    {"edtInitialDelay_6_RT", "pnlPause"},
    {"edtInitialDelay_7", "pnlNoFullsite"},
    {"edtInitialDelay_7_RT", "pnlNoFullsite"},
    {"edtInitialDelay_8", "pnlEOTTime"},
    {"edtInitialDelay_8_RT", "pnlEOTTime"},
    {"edtInitialDelay_9", "pnlOTD"},
    {"edtInitialDelay_9_RT", "pnlOTD"},
    {"edtTestingWaitTime", "grpTestingStopTime"},
    {"edt_UseSocketHeating", "gbInitialDelay"},
    {"gbBaudRate", "pnlRS232"},
    {"gbInitialDelay", "pnlTimeSetting"},
    {"gbRs232BinCount", "pnlTimeSetting"},
    {"grpGPIB", "tsGpib"},
    {"grpGPIBType", "grpGPIB"},
    {"grpRS232", "tsRS232Setting"},
    {"grpRS232Type", "grpRS232"},
    {"grpTTL", "tsDio"},
    {"grpTTLSetting", "grpTTL"},
    {"grpTestingStopTime", "pnlTimeSetting"},
    {"iStartDelayCount", "rgTime2"},
    {"iTestFinishToNextTestOver", "pnlEOTTime"},
    {"iTeststartToNextTestStart", "pnlSOTTime"},
    {"labcAutoOnecycleHomStart", "tsGpib"},
    {"lblDelayTime", "pnlTitle"},
    {"lblFT", "pnlTitle"},
    {"lblRT", "pnlTitle"},
    {"lstTTL", "grpTTL"},
    {"palStartDelay", "palTime"},
    {"palTime", "rgTime1"},
    {"pgcRS232", "tsRs232"},
    {"pnl1stDevice", "gbInitialDelay"},
    {"pnlAlarmMsg", "gbInitialDelay"},
    {"pnlAutoClean", "gbInitialDelay"},
    {"pnlEOTTime", "gbInitialDelay"},
    {"pnlHeatDoor", "gbInitialDelay"},
    {"pnlLowTestTime", "gbInitialDelay"},
    {"pnlNoFullsite", "gbInitialDelay"},
    {"pnlOTD", "gbInitialDelay"},
    {"pnlPause", "gbInitialDelay"},
    {"pnlRS232", "grpRS232"},
    {"pnlSOTTime", "gbInitialDelay"},
    {"pnlTitle", "gbInitialDelay"},
    {"rbTemp", "pnlBtm"},
    {"rg2DID_Format", "grpGPIB"},
    {"rgBitLength", "pnlRS232"},
    {"rgInterfaceType", "grpIFType"},
    {"rgParity", "pnlRS232"},
    {"rgPurgeAir", "PageControl1"},
    {"rgStopBit", "pnlRS232"},
    {"rgTime1", "PageControl1"},
    {"rgTime2", "PageControl1"},
    {"spbSave", "pnlBtm"},
    {"tsDio", "PageControl2"},
    {"tsEQC", "PageControl1"},
    {"tsGpib", "PageControl2"},
    {"tsRS232Setting", "pgcRS232"},
    {"tsRT", "PageControl1"},
    {"tsRs232", "PageControl2"},
    {"tsTCPIP", "PageControl2"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void TIF_CreateContainerProxies()
{
    EL<TPageControl>("TFTestIF", "PageControl1");
    EL<TPageControl>("TFTestIF", "PageControl2");
    EL<TPanel>("TFTestIF", "Panel1");
    EL<TPanel>("TFTestIF", "Panel2");
    EL<TPanel>("TFTestIF", "Panel39");
    EL<TGroupBox>("TFTestIF", "gbBaudRate");
    EL<TGroupBox>("TFTestIF", "gbInitialDelay");
    EL<TGroupBox>("TFTestIF", "gbRs232BinCount");
    EL<TGroupBox>("TFTestIF", "grpGPIB");
    EL<TGroupBox>("TFTestIF", "grpGPIBType");
    EL<TGroupBox>("TFTestIF", "grpIFType");
    EL<TGroupBox>("TFTestIF", "grpRS232");
    EL<TGroupBox>("TFTestIF", "grpRS232Type");
    EL<TGroupBox>("TFTestIF", "grpTTL");
    EL<TGroupBox>("TFTestIF", "grpTTLSetting");
    EL<TGroupBox>("TFTestIF", "grpTestingStopTime");
    EL<TPanel>("TFTestIF", "palStartDelay");
    EL<TPanel>("TFTestIF", "palTime");
    EL<TPageControl>("TFTestIF", "pgcRS232");
    EL<TPanel>("TFTestIF", "pnl1stDevice");
    EL<TPanel>("TFTestIF", "pnlAlarmMsg");
    EL<TPanel>("TFTestIF", "pnlAutoClean");
    EL<TPanel>("TFTestIF", "pnlBtm");
    EL<TPanel>("TFTestIF", "pnlEOTTime");
    EL<TPanel>("TFTestIF", "pnlHeatDoor");
    EL<TPanel>("TFTestIF", "pnlLowTestTime");
    EL<TPanel>("TFTestIF", "pnlNoFullsite");
    EL<TPanel>("TFTestIF", "pnlOTD");
    EL<TPanel>("TFTestIF", "pnlPause");
    EL<TPanel>("TFTestIF", "pnlRS232");
    EL<TPanel>("TFTestIF", "pnlSOTTime");
    EL<TPanel>("TFTestIF", "pnlTimeSetting");
    EL<TPanel>("TFTestIF", "pnlTitle");
    EL<TTabSheet>("TFTestIF", "rgPurgeAir");
    EL<TTabSheet>("TFTestIF", "rgTime1");
    EL<TTabSheet>("TFTestIF", "rgTime2");
    EL<TTabSheet>("TFTestIF", "tsDio");
    EL<TTabSheet>("TFTestIF", "tsEQC");
    EL<TTabSheet>("TFTestIF", "tsGpib");
    EL<TTabSheet>("TFTestIF", "tsRS232Setting");
    EL<TTabSheet>("TFTestIF", "tsRT");
    EL<TTabSheet>("TFTestIF", "tsRs232");
    EL<TTabSheet>("TFTestIF", "tsTCPIP");
    filerw::ELSetParents("TFTestIF", kTIF_ParentOf, (int)(sizeof(kTIF_ParentOf) / sizeof(kTIF_ParentOf[0])));
}

// golden cTesterIF.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void TIF_DfmState()
{
    EL<TCheckBox>("TFTestIF", "cbAutoOnecycleHomStart")->Visible = false;
    EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime")->Visible = false;
    EL<TLabel>("TFTestIF", "labcAutoOnecycleHomStart")->Visible = false;
    EL<TComboBox>("TFTestIF", "cbGPIBType")->Text = "255 Bin";
    EL<TComboBox>("TFTestIF", "cbGPIBType")->ItemIndex = 1;
    EL<TComboBox>("TFTestIF", "cbRs232Type")->Text = "Standard";
    EL<TComboBox>("TFTestIF", "cbRs232Type")->ItemIndex = 0;
    EL<TComboBox>("TFTestIF", "cbbBaudRate")->Text = "9600";
    EL<TComboBox>("TFTestIF", "cbbBaudRate")->ItemIndex = 2;
    EL<TEdit>("TFTestIF", "edBelowSec")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edDummyTestTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edDummyTestTime_EQC")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edDummyTestTime_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edInitialMaxTest")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edInitialMaxTest_EQC")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edInitialMaxTest_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edMaxBinCount")->Text = "32";
    EL<TEdit>("TFTestIF", "edMaxTestTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edMaxTestTime_EQC")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edMaxTestTime_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edOverSec")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edPurgeAir")->Text = "0";
    EL<TEdit>("TFTestIF", "edStartDelayTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edStartDelayTime_EQC")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edStartDelayTime_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edTCPIP_Address")->Text = "192.168.1.50";
    EL<TEdit>("TFTestIF", "edTCPIP_Port")->Text = "6000";
    EL<TEdit>("TFTestIF", "edtAfterTestedDelay")->Text = "0";
    EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitStartDelay")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_EQC")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_RT")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitStartDelay_EQC")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitStartDelay_RT")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitWaitTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDec1")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDec2")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDec3")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDec4")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDecCount1")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitialDecCount2")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitialDecCount3")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitialDecCount4")->Text = "0";
    EL<TEdit>("TFTestIF", "edtInitialDelay_1")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_10")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_10_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_1_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_2")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_2_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_3")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_3_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_4")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_4_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_5")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_5_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_6")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_6_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_7")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_7_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_8")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_8_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_9")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtInitialDelay_9_RT")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edtTestingWaitTime")->Text = "0.00";
    EL<TEdit>("TFTestIF", "edt_UseSocketHeating")->Text = "0.00";
    EL<TComboBox>("TFTestIF", "iStartDelayCount")->Text = "0";
    EL<TEdit>("TFTestIF", "iTestFinishToNextTestOver")->Text = "0.00";
    EL<TEdit>("TFTestIF", "iTeststartToNextTestStart")->Text = "0.00";
    EL<TRadioGroup>("TFTestIF", "rg2DID_Format")->ItemIndex = 0;
    EL<TRadioGroup>("TFTestIF", "rgBitLength")->ItemIndex = 0;
    EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex = 1;
    EL<TRadioGroup>("TFTestIF", "rgParity")->ItemIndex = 0;
    EL<TRadioGroup>("TFTestIF", "rgStopBit")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kTIF_EnableAll[] = {
    "AntiSignalCBox", "Label1", "Label12", "Label2", "Label49", "Label52",
    "Label8", "PageControl1", "PageControl2", "Panel1", "Panel2", "Panel39",
    "cbASEJPMode", "cbAfterAutoCleanFunctionUseInitialDelay", "cbAfterOpenHeatDoorUseInitialDelay", "cbAfterShowAlarmMessageUseInitialDelay", "cbAutoOnecycleHomStart", "cbEveryFirstDeviceUseInitialDelay",
    "cbForEgistec", "cbOTDUnlockDelay", "cbPurgeAir", "cbSpiroxTesterLotEnd", "cbTestFinishToNextTestOver", "cbTestStartToNextTestStart",
    "cbUseOtherArmToTestAfterInitialDelay", "cbUseSocketHeating", "cbWhenHappenTestedTimeBelowUseInitialDelay", "cbWhenNoFullSiteUseInitialDelay", "cbWhenPressStopOverUseInitialDelay", "gbBaudRate",
    "gbInitialDelay", "gbRs232BinCount", "grpGPIB", "grpGPIBType", "grpIFType", "grpRS232",
    "grpRS232Type", "grpTTL", "grpTTLSetting", "grpTestingStopTime", "labcAutoOnecycleHomStart", "lblDelayTime",
    "lblFT", "lblRT", "palStartDelay", "palTime", "pgcRS232", "pnl1stDevice",
    "pnlAlarmMsg", "pnlAutoClean", "pnlBtm", "pnlEOTTime", "pnlHeatDoor", "pnlLowTestTime",
    "pnlNoFullsite", "pnlOTD", "pnlPause", "pnlRS232", "pnlSOTTime", "pnlTimeSetting",
    "pnlTitle", "rbTemp", "rg2DID_Format", "rgBitLength", "rgInterfaceType", "rgParity",
    "rgPurgeAir", "rgStopBit", "rgTime1", "rgTime2", "spbSave", "tsDio",
    "tsEQC", "tsGpib", "tsRS232Setting", "tsRT", "tsRs232", "tsTCPIP",
};

// 存檔流程（spbSaveClick、SaveSetupFile）讀的替身 88 個 —— FileRW/TestIF_File_TesterIF.cpp 用它決定頁面必須送哪些值
static const char* const kTIF_SaveReads[] = {
    "AntiSignalCBox", "cbASEJPMode", "cbAfterAutoCleanFunctionUseInitialDelay", "cbAfterOpenHeatDoorUseInitialDelay", "cbAfterShowAlarmMessageUseInitialDelay", "cbAutoOnecycleHomStart",
    "cbDIOType", "cbEveryFirstDeviceUseInitialDelay", "cbForEgistec", "cbGPIBType", "cbOTDUnlockDelay", "cbPurgeAir",
    "cbRs232Type", "cbSpiroxTesterLotEnd", "cbTestFinishToNextTestOver", "cbTestStartToNextTestStart", "cbUseOtherArmToTestAfterInitialDelay", "cbUseSocketHeating",
    "cbWhenHappenTestedTimeBelowUseInitialDelay", "cbWhenNoFullSiteUseInitialDelay", "cbWhenPressStopOverUseInitialDelay", "cbbBaudRate", "edBelowSec", "edDummyTestTime",
    "edDummyTestTime_EQC", "edDummyTestTime_RT", "edGPIBAddress", "edInitialMaxTest", "edInitialMaxTest_EQC", "edInitialMaxTest_RT",
    "edMaxBinCount", "edMaxTestTime", "edMaxTestTime_EQC", "edMaxTestTime_RT", "edOverSec", "edPurgeAir",
    "edStartDelayTime", "edStartDelayTime_EQC", "edStartDelayTime_RT", "edTCPIP_Address", "edTCPIP_Port", "edtAfterTestedDelay",
    "edtAutoOnecycleHomStartTime", "edtInitStartDelay", "edtInitStartDelayCT", "edtInitStartDelayCT_EQC", "edtInitStartDelayCT_RT", "edtInitStartDelay_EQC",
    "edtInitStartDelay_RT", "edtInitWaitTime", "edtInitialDec1", "edtInitialDec2", "edtInitialDec3", "edtInitialDec4",
    "edtInitialDecCount1", "edtInitialDecCount2", "edtInitialDecCount3", "edtInitialDecCount4", "edtInitialDelay_1", "edtInitialDelay_10",
    "edtInitialDelay_10_RT", "edtInitialDelay_1_RT", "edtInitialDelay_2", "edtInitialDelay_2_RT", "edtInitialDelay_3", "edtInitialDelay_3_RT",
    "edtInitialDelay_4", "edtInitialDelay_4_RT", "edtInitialDelay_5", "edtInitialDelay_5_RT", "edtInitialDelay_6", "edtInitialDelay_6_RT",
    "edtInitialDelay_7", "edtInitialDelay_7_RT", "edtInitialDelay_8", "edtInitialDelay_8_RT", "edtInitialDelay_9", "edtInitialDelay_9_RT",
    "edtTestingWaitTime", "edt_UseSocketHeating", "iStartDelayCount", "iTestFinishToNextTestOver", "iTeststartToNextTestStart", "rg2DID_Format",
    "rgBitLength", "rgInterfaceType", "rgParity", "rgStopBit",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void TIF_CreateSaveProxies()
{
    EL<TCheckBox>("TFTestIF", "AntiSignalCBox");
    EL<TCheckBox>("TFTestIF", "cbASEJPMode");
    EL<TCheckBox>("TFTestIF", "cbAfterAutoCleanFunctionUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbAfterOpenHeatDoorUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbAfterShowAlarmMessageUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbAutoOnecycleHomStart");
    EL<TComboBox>("TFTestIF", "cbDIOType");
    EL<TCheckBox>("TFTestIF", "cbEveryFirstDeviceUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbForEgistec");
    EL<TComboBox>("TFTestIF", "cbGPIBType");
    EL<TCheckBox>("TFTestIF", "cbOTDUnlockDelay");
    EL<TCheckBox>("TFTestIF", "cbPurgeAir");
    EL<TComboBox>("TFTestIF", "cbRs232Type");
    EL<TCheckBox>("TFTestIF", "cbSpiroxTesterLotEnd");
    EL<TCheckBox>("TFTestIF", "cbTestFinishToNextTestOver");
    EL<TCheckBox>("TFTestIF", "cbTestStartToNextTestStart");
    EL<TCheckBox>("TFTestIF", "cbUseOtherArmToTestAfterInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbUseSocketHeating");
    EL<TCheckBox>("TFTestIF", "cbWhenHappenTestedTimeBelowUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbWhenNoFullSiteUseInitialDelay");
    EL<TCheckBox>("TFTestIF", "cbWhenPressStopOverUseInitialDelay");
    EL<TComboBox>("TFTestIF", "cbbBaudRate");
    EL<TEdit>("TFTestIF", "edBelowSec");
    EL<TEdit>("TFTestIF", "edDummyTestTime");
    EL<TEdit>("TFTestIF", "edDummyTestTime_EQC");
    EL<TEdit>("TFTestIF", "edDummyTestTime_RT");
    EL<TEdit>("TFTestIF", "edGPIBAddress");
    EL<TEdit>("TFTestIF", "edInitialMaxTest");
    EL<TEdit>("TFTestIF", "edInitialMaxTest_EQC");
    EL<TEdit>("TFTestIF", "edInitialMaxTest_RT");
    EL<TEdit>("TFTestIF", "edMaxBinCount");
    EL<TEdit>("TFTestIF", "edMaxTestTime");
    EL<TEdit>("TFTestIF", "edMaxTestTime_EQC");
    EL<TEdit>("TFTestIF", "edMaxTestTime_RT");
    EL<TEdit>("TFTestIF", "edOverSec");
    EL<TEdit>("TFTestIF", "edPurgeAir");
    EL<TEdit>("TFTestIF", "edStartDelayTime");
    EL<TEdit>("TFTestIF", "edStartDelayTime_EQC");
    EL<TEdit>("TFTestIF", "edStartDelayTime_RT");
    EL<TEdit>("TFTestIF", "edTCPIP_Address");
    EL<TEdit>("TFTestIF", "edTCPIP_Port");
    EL<TEdit>("TFTestIF", "edtAfterTestedDelay");
    EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime");
    EL<TEdit>("TFTestIF", "edtInitStartDelay");
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT");
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_EQC");
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_RT");
    EL<TEdit>("TFTestIF", "edtInitStartDelay_EQC");
    EL<TEdit>("TFTestIF", "edtInitStartDelay_RT");
    EL<TEdit>("TFTestIF", "edtInitWaitTime");
    EL<TEdit>("TFTestIF", "edtInitialDec1");
    EL<TEdit>("TFTestIF", "edtInitialDec2");
    EL<TEdit>("TFTestIF", "edtInitialDec3");
    EL<TEdit>("TFTestIF", "edtInitialDec4");
    EL<TEdit>("TFTestIF", "edtInitialDecCount1");
    EL<TEdit>("TFTestIF", "edtInitialDecCount2");
    EL<TEdit>("TFTestIF", "edtInitialDecCount3");
    EL<TEdit>("TFTestIF", "edtInitialDecCount4");
    EL<TEdit>("TFTestIF", "edtInitialDelay_1");
    EL<TEdit>("TFTestIF", "edtInitialDelay_10");
    EL<TEdit>("TFTestIF", "edtInitialDelay_10_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_1_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_2");
    EL<TEdit>("TFTestIF", "edtInitialDelay_2_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_3");
    EL<TEdit>("TFTestIF", "edtInitialDelay_3_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_4");
    EL<TEdit>("TFTestIF", "edtInitialDelay_4_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_5");
    EL<TEdit>("TFTestIF", "edtInitialDelay_5_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_6");
    EL<TEdit>("TFTestIF", "edtInitialDelay_6_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_7");
    EL<TEdit>("TFTestIF", "edtInitialDelay_7_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_8");
    EL<TEdit>("TFTestIF", "edtInitialDelay_8_RT");
    EL<TEdit>("TFTestIF", "edtInitialDelay_9");
    EL<TEdit>("TFTestIF", "edtInitialDelay_9_RT");
    EL<TEdit>("TFTestIF", "edtTestingWaitTime");
    EL<TEdit>("TFTestIF", "edt_UseSocketHeating");
    EL<TComboBox>("TFTestIF", "iStartDelayCount");
    EL<TEdit>("TFTestIF", "iTestFinishToNextTestOver");
    EL<TEdit>("TFTestIF", "iTeststartToNextTestStart");
    EL<TRadioGroup>("TFTestIF", "rg2DID_Format");
    EL<TRadioGroup>("TFTestIF", "rgBitLength");
    EL<TRadioGroup>("TFTestIF", "rgInterfaceType");
    EL<TRadioGroup>("TFTestIF", "rgParity");
    EL<TRadioGroup>("TFTestIF", "rgStopBit");
}

// golden cTesterIF.cpp:40  TFTestIF::TFTestIF(TComponent* Owner)
static void TIF_TFTestIF()
{

    TIF_InitcbDIOType(false);                                                       //Steven 20101008 : 連按三下會不見
    ioldTestType=-1;                                                            //0:TTL_MODE ; 1: GPIB_MODE ; 2: RS232_MODE     //Isaac 20200903 :TTL RS232通訊
    ioldDIOType=-1;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20210924 GPIB 顯示type 命名
    {
    TIF_CbClear(EL<TComboBox>("TFTestIF", "cbGPIBType"));   // golden cTesterIF.cpp:49-49 取代：CC_ASE_KaohSiung 的 GPIB 型態改名（客戶專屬，編得過照留）：cbGPIBType->Clear() 照 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:49-49 -- CC_ASE_KaohSiung 的 GPIB 型態改名（客戶專屬，編得過照留）：cbGPIBType->Clear() 照 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""）
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Clear();
#endif // GATE (S12-C save)
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("Advan type1");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("256 Bin");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("16 Bin");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("32 Bin");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("SPEA Type");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("16 Bin GS");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("32 Bin GS");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("Advan T6577");
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Items->Add("Qrovo Protocol");
    ;   // golden cTesterIF.cpp:59-59 取代：cbGPIBType->Refresh()：重畫（vclcompat TControl 沒有 Refresh；HTML 端自己畫）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:59-59 -- cbGPIBType->Refresh()：重畫（vclcompat TControl 沒有 Refresh；HTML 端自己畫）
        EL<TComboBox>("TFTestIF", "cbGPIBType")->Refresh();
#endif // GATE (S12-C save)
    }

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20201209 : Default Recipe ChangeLog
    {
    ;   // golden cTesterIF.cpp:64-66 取代：CC_Greatek 的 ->Hint（Default Recipe ChangeLog 用的鍵名；vclcompat 沒有 Hint，RPDefault 未移植）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:64-66 -- CC_Greatek 的 ->Hint（Default Recipe ChangeLog 用的鍵名；vclcompat 沒有 Hint，RPDefault 未移植）
        EL<TEdit>("TFTestIF", "edInitialMaxTest")->Hint  ="Tester_Initial Max Time";
        EL<TEdit>("TFTestIF", "edMaxTestTime")->Hint     ="Tester_Wait Max Time";
        EL<TEdit>("TFTestIF", "edStartDelayTime")->Hint  ="Tester_Start Delay Time";
#endif // GATE (S12-C save)
    }

}

// golden cTesterIF.cpp:70  TFTestIF::InitcbDIOType(bool bAlarm)
static void TIF_InitcbDIOType(bool bAlarm)
{

    TIF_CbClear(EL<TComboBox>("TFTestIF", "cbDIOType")); bDioListLost=false;   // golden cTesterIF.cpp:72-72 取代：cbDIOType->Clear()：VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）；同時重置 bDioListLost
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:72-72 -- cbDIOType->Clear()：VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）；同時重置 bDioListLost
    EL<TComboBox>("TFTestIF", "cbDIOType")->Clear();
#endif // GATE (S12-C save)
    WIN32_FIND_DATA filedata;                                                   // Structure for file data
    HANDLE filehandle;                                                          // Handle for searching
    AnsiString szFileName;
    filehandle=FindFirstFile((DIOCFGPath + "*.ini").c_str(), &filedata);        //Steven 20100706 Start: 改用讀取資料夾的方式

    if(filehandle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                strcmp(filedata.cFileName, ".")==0 ||                           /* 不處理隱藏檔及 . 跟 .. */
                strcmp(filedata.cFileName, "..")==0)
                continue;

            if(ExtractFileExt(filedata.cFileName).LowerCase()==".ini")          // 若找到的檔案的副檔名是 .ini
            {
                szFileName=ChangeFileExt(ExtractFileName(filedata.cFileName), "");  // 取出檔名，其實就是把副檔名設成""
                EL<TComboBox>("TFTestIF", "cbDIOType")->Items->Add(szFileName);                              // 將檔名加到 cbDIOType
            }
        } while(FindNextFile(filehandle, &filedata));
    }
    else
    {
        if(bAlarm==false)
        {
    filerw::ELMessage("DIO data has been lossed, please check!!", "DIO Data loss"); bDioListLost=true; std::printf("FileRW TesterIF: golden InitcbDIOType -- no *.ini in DIOCFGPath (%s); golden terminates here, saves are refused\n", DIOCFGPath.c_str());   // golden cTesterIF.cpp:98-99 取代：DIO 檔一個都沒有＋bAlarm==false：golden MessageBox＋Application->Terminate()（結束程式）。網頁版改 ELMessage、記 bDioListLost → 這次之後的存檔一律拒絕（FileRW/TestIF_File_TesterIF.cpp SaveFlow），不結束 wb_serve（同 FileRW/TTLCfg.cpp）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:98-99 -- DIO 檔一個都沒有＋bAlarm==false：golden MessageBox＋Application->Terminate()（結束程式）。網頁版改 ELMessage、記 bDioListLost → 這次之後的存檔一律拒絕（FileRW/TestIF_File_TesterIF.cpp SaveFlow），不結束 wb_serve（同 FileRW/TTLCfg.cpp）
            Application->MessageBox("DIO data has been lossed, please check!!", "DIO Data loss", MB_OK|MB_TOPMOST);
            Application->Terminate();
#endif // GATE (S12-C save)
        }
        else
        {
            filerw::ELMessage("DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");
        }
    }
    FindClose(filehandle);

}

// golden cTesterIF.cpp:109  TFTestIF::FormShow(TObject *Sender)
static void TIF_FormShow()
{

    AnsiString S;
    EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=true;                                                 //Steven 20220523 : 避免SetFocus失敗
    EL<TPanel>("TFTestIF", "palTime")->Enabled=true;
    EL<TGroupBox>("TFTestIF", "gbRs232BinCount")->Visible=false;
    EL<TPageControl>("TFTestIF", "PageControl1")->ActivePageIndex=0;                                            //Jimmychiu 20221205 直接SetFocus會異常
    EL<TRadioButton>("TFTestIF", "rbTemp")->SetFocus();

    TIF_InitcbDIOType(TestIF_File.iTestType==TTL_MODE);                             //Steven 20101008 : 連按三下會不見   //Steven 20210723 : 修正不是TTL模式會Alarm
    TIF_ReadTestIFFile();
    TIF_DoIniDataToForm();
    S.sprintf("Tester Interface  ''%s''  ",GetLastOpenFN());
    ;   // golden cTesterIF.cpp:122-122 取代：Caption：視窗標題（HTML 自己有）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:122-122 -- Caption：視窗標題（HTML 自己有）
    Caption=S;
#endif // GATE (S12-C save)

    TIF_ShowTTLState();
    EL<TCheckBox>("TFTestIF", "AntiSignalCBox")->Visible = LastSet.bUseNewTTLBoard;                          //Eliot 2008_02_01 start
    EL<TCheckBox>("TFTestIF", "AntiSignalCBox")->Checked = TestIF.bAntiSignal;                               //Eliot 2008_02_01 end

    ;   // golden cTesterIF.cpp:128-129 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:128-129 -- Left／Top：視窗位置
    Left=100;                                                                   //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    if(fMain->CheckCanChangeRealDummy()==false)                                 //jou 981125 start : 防止測試中被改變
    {
        EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=false;
    }
    else if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)         //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[21])
            EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=false;
    }
    else
    {
        EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=fSecurity->Insufficient(19,false);                   //half
    }

    if(EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex==0)                                           //Isaac 20210309 :TTL RS232兩塊板子，TTL mode
    {
        if((TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                              //Isaac 20210309 :TTL RS232兩塊板子
        {
            CheckTTLBoardBitMode();
        }
        else
        {
            if(TestIF.iTestMode> _8Site2X4 &&                                   //jou 2010-01-06 start : 4 Site 以上不支援 TTL
               CosFunction.bTTLCanUse8Site==false)                              //Alick 20161011 (Steven) : TTL支援8Site
            {
                filerw::ELMessage("TTL only support less then 4 site, please select GPIB.", "4 Site 以上不支援 TTL，請選擇GPIB");
            }
        }
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220927 : SPIL lot info
    {
        EL<TTabSheet>("TFTestIF", "tsRT")->TabVisible=true;
        EL<TTabSheet>("TFTestIF", "tsEQC")->TabVisible=true;
    }
    else
    {
        EL<TTabSheet>("TFTestIF", "tsRT")->TabVisible=false;
        EL<TTabSheet>("TFTestIF", "tsEQC")->TabVisible=false;
    }

//    if(CUSTOMER_CODE!=CC_AMD_M)                                               //Ifor 20190927 : add AMD 強制開啟AMD Function
//    {
//        chkAMDFunction->Visible=(CosFunction.bAMDFunction==true);             //Ifor 20191025 : AMD 開放2D Barcode資料格式選項切換
//    }
//    chkAMDFunction->Visible=(CosFunction.bAMDFunction==true);                 //JerryYang 20200422 2DID format選項改用下拉選單

    if(CosFunction.bFTRTDiffInitStartDelayTime)                                 //JerryYang 20220330 : FT RT可設定不同的initial delay time
    {
        EL<TTabSheet>("TFTestIF", "rgTime1")->Caption="FT";
    }
    else
    {
        EL<TTabSheet>("TFTestIF", "rgTime1")->Caption="Time1";
    }

    fShow=true;
#ifdef SOFT_SIMULTE
    EL<TCheckBox>("TFTestIF", "cbForEgistec")->Visible=(CosFunction.bArm2ForFingerPrintTest);                //Steven 20140922 : Arm2當作指紋測試
#else
    EL<TCheckBox>("TFTestIF", "cbForEgistec")->Visible=false;
#endif

    TIF_cbRs232TypeChange();   // golden cTesterIF.cpp:194-194 取代：事件以 (this) 呼叫（參數已拿掉）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:194-194 -- 事件以 (this) 呼叫（參數已拿掉）
    TIF_cbRs232TypeChange(this);                                                    //Steven 20100722
#endif // GATE (S12-C save)
    EL<TPageControl>("TFTestIF", "PageControl1")->ActivePageIndex=0;
    EL<TGroupBox>("TFTestIF", "gbInitialDelay")->Visible=(LastSet.iTemperature==Tempture_Hot ||              //Steven 20171219 : 只有加熱模式要跑Initial Delay
                             LastSet.iTemperature==Tempture_AmbientHot ||       //kevin 20180811 (Steven) : add 恆溫控制
                             CUSTOMER_CODE==CC_KYEC_LEE);                       //Ifor 20180109 (Steven) : add KYEC 常高溫都要跑Initial Delay

    if(CUSTOMER_CODE==CC_SCC ||                                                 //ChungHung 20130621 add SCK RMS
       CUSTOMER_CODE==CC_SCK)                                                   //Steven 20101221
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=false;                                        //kevin 20180307 change
            EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->Enabled=false;
            EL<TTabSheet>("TFTestIF", "tsDio")->Enabled=false;
            EL<TTabSheet>("TFTestIF", "tsGpib")->Enabled=false;
            EL<TTabSheet>("TFTestIF", "tsRs232")->Enabled=false;
        }
        else
        {
            EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=true;                                         //kevin 20180307 change
            EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->Enabled=true;
            EL<TTabSheet>("TFTestIF", "tsDio")->Enabled=true;
            EL<TTabSheet>("TFTestIF", "tsGpib")->Enabled=true;
            EL<TTabSheet>("TFTestIF", "tsRs232")->Enabled=true;
        }
    }
    else if(CUSTOMER_CODE==CC_ASE_CL)                                           //JerryYang 20170301 (wei) ASE_CL權限控制
    {
        EL<TPanel>("TFTestIF", "palTime")->Enabled=(AccessLevel>=LevelSet.AccessLevel[91]);
        EL<TPanel>("TFTestIF", "palStartDelay")->Enabled=(AccessLevel>=LevelSet.AccessLevel[131]);        //JerryYang 20170301 Start Delay權限設定
    }
    else
    {
        //grpTestTimeSetting->Enabled=(AccessLevel>=LevelSet.AccessLevel[91]);  //Steven 20140114 : 測試時間設定的權限
        EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=(AccessLevel>=LevelSet.AccessLevel[91]);          //kevin 20180307 change
    }

    EL<TLabel>("TFTestIF", "Label49")             ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TEdit>("TFTestIF", "edtInitStartDelay")   ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TLabel>("TFTestIF", "Label1")              ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TLabel>("TFTestIF", "Label2")              ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT") ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TLabel>("TFTestIF", "Label52")             ->Visible=IniConfig.bInitialStartDelayCount;            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

    EL<TGroupBox>("TFTestIF", "grpTestingStopTime")->Visible=IniConfig.bEnableTestingNeedStopAllMotor && IniConfig.bI24TestingNeedStopAllMotor;   //jou 2013-09-25 Testing Need Stop All Motor

    EL<TLabel>("TFTestIF", "Label8")              ->Visible=CosFunction.bEnableAfterTestedDelay;          //ChungHung 20140730 add for ATK function after tested delay time
    EL<TEdit>("TFTestIF", "edtAfterTestedDelay") ->Visible=CosFunction.bEnableAfterTestedDelay;          //ChungHung 20140730 add for ATK function after tested delay time
    EL<TLabel>("TFTestIF", "Label12")             ->Visible=CosFunction.bEnableAfterTestedDelay;          //ChungHung 20140730 add for ATK function after tested delay time

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //20140320 wei   KYEC 低於權限顯示不能修改
    {
        if(AccessLevel<LevelSet.AccessLevel[19])
        {
            EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=false;
            //grpTestTimeSetting->Enabled=false;
            EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=false;                                        //kevin 20180307 change

            EL<TGroupBox>("TFTestIF", "grpTestingStopTime")->Enabled=false;
        }
        else
        {
            EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=true;
            //grpTestTimeSetting->Enabled=true;
            EL<TPageControl>("TFTestIF", "PageControl1")->Enabled=true;                                         //kevin 20180307 change
            EL<TGroupBox>("TFTestIF", "grpTestingStopTime")->Enabled=true;
        }
    }

    if(IniConfig.bSPILFunction==true && iRunStartMode==RT)                      //JerryYang 20220330 : FT RT可設定不同的initial delay time   //Steven 20221129 : Add SPIL
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TFTestIF", "PageControl1")->ActivePage=EL<TTabSheet>("TFTestIF", "tsRT");
    }
    else if(IniConfig.bSPILFunction==true && fSCKART->sInfo_Stage.Pos("QC")==1) //Steven 20221129 : Add SPIL
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TFTestIF", "PageControl1")->ActivePage=EL<TTabSheet>("TFTestIF", "tsEQC");
    }
    else
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TFTestIF", "PageControl1")->ActivePage=EL<TTabSheet>("TFTestIF", "rgTime1");
    }

    EL<TTabSheet>("TFTestIF", "rgTime2")->TabVisible=(CUSTOMER_CODE==CC_ASE_KaohSiung || IniConfig.bAMDFunction);             //kevin 20180307 add Steven 20170215 (wei): 其他客戶不顯示
    EL<TTabSheet>("TFTestIF", "rgPurgeAir")->TabVisible=(CUSTOMER_CODE==CC_ASE_KaohSiung ||                  //kevin 20180928 add
                            IniConfig.bSPILFunction);                           //JerryYang 20181115 (Steven) : SPIL要求吹氣功能

    EL<TGroupBox>("TFTestIF", "gbInitialDelay")->Enabled=(AccessLevel>=LevelSet.AccessLevel[123]);           //jou 2016-05-12 新增 "Use initial start delay in socket" 權限管控

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20151228 No FullSite delay
        EL<TPanel>("TFTestIF", "pnlNoFullsite")->Visible=true;
    else
        EL<TPanel>("TFTestIF", "pnlNoFullsite")->Visible=false;

    if(CosFunction.bAfterInitialDelayUseOtherArm==true)                         //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        EL<TCheckBox>("TFTestIF", "cbUseOtherArmToTestAfterInitialDelay")->Visible=true;
    else
        EL<TCheckBox>("TFTestIF", "cbUseOtherArmToTestAfterInitialDelay")->Visible=false;

    if(IniConfig.bI39SpiroxTesterLotEnd)                                        //JerryYang 20170706 (Steven) JCET通知測試機lot end功能改為by工作檔
        EL<TCheckBox>("TFTestIF", "cbSpiroxTesterLotEnd")->Visible=true;
    else
        EL<TCheckBox>("TFTestIF", "cbSpiroxTesterLotEnd")->Visible=false;
//    gbInitialDelay->Visible=(CUSTOMER_CODE!=CC_AMKOR_China);                  //jou 2014-08-25 安靠要求不使用 Use initial start delay in socket     //Steven 20150416 : 客戶說要打開

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    EL<TLabel>("TFTestIF", "lblDelayTime")->Left=(IniConfig.bI13InitStartDelayHasFTandRT)?356:320;
    EL<TLabel>("TFTestIF", "lblRT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TLabel>("TFTestIF", "lblFT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_1_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_2_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_3_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_4_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_5_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_6_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_7_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_8_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_9_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_10_RT")->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    if(CUSTOMER_CODE==CC_Greatek)
    {
        EL<TGroupBox>("TFTestIF", "grpIFType")->Enabled=fSecurity->Insufficient(27, false);                  //JimmyChiu 20211228 : InterFace Type
    }
    else if(CUSTOMER_CODE==CC_GIGAS)                                            //Isaac 20210821 : 全智要求定時onecycle回home,start
    {
        EL<TCheckBox>("TFTestIF", "cbAutoOnecycleHomStart")->Visible=true;
        EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime")->Visible=true;
        EL<TLabel>("TFTestIF", "labcAutoOnecycleHomStart")->Visible=true;
    }

    if(SW[SwPurgeAir].Enable==false)
    {
        EL<TTabSheet>("TFTestIF", "rgPurgeAir")->TabVisible=false;
    }

    ioldTestType=TestIF_File.iTestType;
    ioldDIOType=TestIF_File.iDioMode;
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cTesterIF.cpp:337  TFTestIF::SaveSetupFile(AnsiString szDir, AnsiString S)
static void TIF_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    MyForceDirectories(szDir);
    szDir+="\\Tester.Data";

    WriteIniData(szDir, "Mode",     "Tester Type",                  EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex);

    WriteIniData(szDir, "Time",     "Initial MAX Time",             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edInitialMaxTest")->Text.ToDouble()));         //jou 2011-11-09 增加initial max time set
    WriteIniData(szDir, "Time",     "MAX Time",                     FormatFloat("0.00", EL<TEdit>("TFTestIF", "edMaxTestTime")->Text.ToDouble()));
    WriteIniData(szDir, "Time",     "Dummy Time",                   FormatFloat("0.00", EL<TEdit>("TFTestIF", "edDummyTestTime")->Text.ToDouble()));
    WriteIniData(szDir, "Time",     "Stary Delay",                  FormatFloat("0.00", EL<TEdit>("TFTestIF", "edStartDelayTime")->Text.ToDouble()));

    WriteIniData(szDir, "Time",     "Initial Stary Delay",          FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitStartDelay")->Text.ToDouble()));       //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    WriteIniData(szDir, "Time",     "Initial Stary Delay CT",       EL<TEdit>("TFTestIF", "edtInitStartDelayCT")->Text);                                     //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

    WriteIniData(szDir, "Time",      "RT Initial MAX Time",         EL<TEdit>("TFTestIF", "edInitialMaxTest_RT")->Text);  //JerryYang 20220330 : FT RT可設定不同的initial delay time
    WriteIniData(szDir, "Time",      "RT MAX Time",                 EL<TEdit>("TFTestIF", "edMaxTestTime_RT")->Text);
    WriteIniData(szDir, "Time",      "RT Dummy Time",               EL<TEdit>("TFTestIF", "edDummyTestTime_RT")->Text);
    WriteIniData(szDir, "Time",      "RT Stary Delay",              EL<TEdit>("TFTestIF", "edStartDelayTime_RT")->Text);
    WriteIniData(szDir, "Time",      "RT Initial Stary Delay",      EL<TEdit>("TFTestIF", "edtInitStartDelay_RT")->Text);
    WriteIniData(szDir, "Time",      "RT Initial Stary Delay CT",   EL<TEdit>("TFTestIF", "edtInitStartDelayCT_RT")->Text);

    WriteIniData(szDir, "Time",      "EQC Initial MAX Time",        EL<TEdit>("TFTestIF", "edInitialMaxTest_EQC")->Text);
    WriteIniData(szDir, "Time",      "EQC MAX Time",                EL<TEdit>("TFTestIF", "edMaxTestTime_EQC")->Text);
    WriteIniData(szDir, "Time",      "EQC Dummy Time",              EL<TEdit>("TFTestIF", "edDummyTestTime_EQC")->Text);
    WriteIniData(szDir, "Time",      "EQC Stary Delay",             EL<TEdit>("TFTestIF", "edStartDelayTime_EQC")->Text);
    WriteIniData(szDir, "Time",      "EQC Initial Stary Delay",     EL<TEdit>("TFTestIF", "edtInitStartDelay_EQC")->Text);
    WriteIniData(szDir, "Time",      "EQC Initial Stary Delay CT",  EL<TEdit>("TFTestIF", "edtInitStartDelayCT_EQC")->Text);

    WriteIniData(szDir, "Time",     "Init Wait Time",               FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitWaitTime")->Text.ToDouble()));         //jou 2013-09-25 Testing Need Stop All Motor
    WriteIniData(szDir, "Time",     "Testing Wait Time",            FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtTestingWaitTime")->Text.ToDouble()));      //jou 2013-09-25 Testing Need Stop All Moto

    //ChungHung 20140730 add for ATK function after tested delay time
    WriteIniData(szDir, "Time",     "After Tested Delay",           FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtAfterTestedDelay")->Text.ToDouble()));

    WriteIniData(szDir, "DIO",      "Type",         EL<TComboBox>("TFTestIF", "cbDIOType")->ItemIndex);
    WriteIniData(szDir, "DIO",      "TypeName",     EL<TComboBox>("TFTestIF", "cbDIOType")->Text);
    WriteIniData(szDir, "DIO",      "Anti Signal",  EL<TCheckBox>("TFTestIF", "AntiSignalCBox")->Checked);
    //Steven 20161208 : ART need to set to 32binGS for ATK
    //==>
    if(USE_AUTO_RETEST==eartInstall       &&
       CosFunction.bUseSCKART==true       &&
       IniConfig.bA10_AutoReTest==true    &&
       IniConfig.bA10TestModeForART==true)
    {
        //用Config的,而且不複寫原本工作檔
    }
    else
    {
        WriteIniData(szDir, "GP-IB",     "Type",         EL<TComboBox>("TFTestIF", "cbGPIBType")->ItemIndex);
    }
    //<==
    //Steven 20161208 : ART need to set to 32binGS for ATK

    WriteIniData(szDir, "GP-IB",     "Address",      EL<TEdit>("TFTestIF", "edGPIBAddress")->Text);
    WriteIniData(szDir, "GP-IB",     "2DIDFormat", EL<TRadioGroup>("TFTestIF", "rg2DID_Format")->ItemIndex);
//    WriteIniData(szDir, "GP-IB",     "Intel 2DID",   cbIntel2D->Checked);     //JerryYang 20200422 2DID format選項改用下拉選單

    WriteIniData(szDir, "RS-232C",   "Type",         EL<TComboBox>("TFTestIF", "cbRs232Type")->ItemIndex);
    WriteIniData(szDir, "RS-232C",   "Bin Count",    EL<TEdit>("TFTestIF", "edMaxBinCount")->Text);
//    WriteIniData(szDir, "RS-232C",   "Baud Rate",    rgBaudRate->ItemIndex);
    WriteIniData(szDir, "RS-232C",   "BaudRate",     EL<TComboBox>("TFTestIF", "cbbBaudRate")->Text);        //Steven 20211224 : baudrate改用數字
    WriteIniData(szDir, "RS-232C",   "Bit Length",   EL<TRadioGroup>("TFTestIF", "rgBitLength")->ItemIndex);
    WriteIniData(szDir, "RS-232C",   "Stop Bit",     EL<TRadioGroup>("TFTestIF", "rgStopBit")->ItemIndex);
    WriteIniData(szDir, "RS-232C",   "Parity",       EL<TRadioGroup>("TFTestIF", "rgParity")->ItemIndex);

//    WriteIniData(szDir, "RS-232C",   "NeedVSOT",     cb_NeedSendVSOT->Checked);
    if(CosFunction.bArm2ForFingerPrintTest)                                     //Steven 20140922 : Arm2當作指紋測試
        WriteIniData(szDir, "RS-232C",   "ForEgisTecTest",   EL<TCheckBox>("TFTestIF", "cbForEgistec")->Checked);
    else
        WriteIniData(szDir, "RS-232C",   "ForEgisTecTest",   false);

    //ChungHung 20140425 add for TSMC Device
    WriteIniData(szDir, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",            EL<TCheckBox>("TFTestIF", "cbEveryFirstDeviceUseInitialDelay")->Checked);

    if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        WriteIniData(szDir, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         EL<TCheckBox>("TFTestIF", "cbUseOtherArmToTestAfterInitialDelay")->Checked);
    else
        WriteIniData(szDir, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         false);

    WriteIniData(szDir, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",       EL<TCheckBox>("TFTestIF", "cbAfterShowAlarmMessageUseInitialDelay")->Checked);
    WriteIniData(szDir, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay",   EL<TCheckBox>("TFTestIF", "cbWhenHappenTestedTimeBelowUseInitialDelay")->Checked);
    WriteIniData(szDir, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",            EL<TEdit>("TFTestIF", "edBelowSec")->Text);
    WriteIniData(szDir, "InitialMode",       "iInitialDelay",                               FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_1")->Text.ToDouble()));    //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_2",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_2")->Text.ToDouble()));    //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_3",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_3")->Text.ToDouble()));    //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_4",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_4")->Text.ToDouble()));    //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_5",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_5")->Text.ToDouble()));    //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_6",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_6")->Text.ToDouble()));    //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           EL<TCheckBox>("TFTestIF", "cbWhenPressStopOverUseInitialDelay")->Checked);   //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    WriteIniData(szDir, "InitialMode",       "iWhenPressStopOver",    EL<TEdit>("TFTestIF", "edOverSec")->Text);                                                     //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              EL<TCheckBox>("TFTestIF", "cbWhenNoFullSiteUseInitialDelay")->Checked);      //wei 20151228 No FullSite delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_7",                             EL<TEdit>("TFTestIF", "edtInitialDelay_7")->Text);                       //wei 20151228 No FullSite delay

    //ChungHung 20141017 add for SCK Add for TSMC Device
    WriteIniData(szDir, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      EL<TCheckBox>("TFTestIF", "cbAfterAutoCleanFunctionUseInitialDelay")->Checked);
    //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    WriteIniData(szDir, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           EL<TCheckBox>("TFTestIF", "cbAfterOpenHeatDoorUseInitialDelay")->Checked);

    WriteIniData(szDir, "InitialMode",       "bTestFinishToNextTestOver",                   EL<TCheckBox>("TFTestIF", "cbTestFinishToNextTestOver")->Checked);                                       //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir, "InitialMode",       "iTestFinishToNextTestOver",                   FormatFloat("0.00", EL<TEdit>("TFTestIF", "iTestFinishToNextTestOver")->Text.ToDouble()));           //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_8",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_8")->Text.ToDouble()));                   //kevin 20160310 測試完成到下一次完成超過所設時間須啟動

    WriteIniData(szDir, "InitialMode",       "bOTDUnlockDelay",                             EL<TCheckBox>("TFTestIF", "cbOTDUnlockDelay")->Checked);                                                 //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir, "InitialMode",       "iInitialDelay_9",                             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_9")->Text.ToDouble()));                   //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir, "InitialMode",       "bTestStartToNextTestStart",                   EL<TCheckBox>("TFTestIF", "cbTestStartToNextTestStart")->Checked);                                       //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir, "InitialMode",       "dTeststartToNextTestStart",                   FormatFloat("0.00", EL<TEdit>("TFTestIF", "iTeststartToNextTestStart")->Text.ToDouble()));           //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_10",                            FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_10")->Text.ToDouble()));                  //kevin 20181031 : SOT 超過時間Delay

    WriteIniData(szDir, "InitialMode",       "iEnStartDelayCount",                          EL<TComboBox>("TFTestIF", "iStartDelayCount")->ItemIndex );                                              //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay
    WriteIniData(szDir, "InitialMode",       "dInitialStartDelayDec1",                      FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDec1")->Text.ToDouble()));                      //kevin 20180307 每次等待時間
    WriteIniData(szDir, "InitialMode",       "iStartDelayCount1",                           EL<TEdit>("TFTestIF", "edtInitialDecCount1")->Text);                                                 //kevin 20180307 第幾個 Count開始執行送訊號delay
    WriteIniData(szDir, "InitialMode",       "dInitialStartDelayDec2",                      FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDec2")->Text.ToDouble()));                      //kevin 20180307 每次等待時間
    WriteIniData(szDir, "InitialMode",       "iStartDelayCount2",                           EL<TEdit>("TFTestIF", "edtInitialDecCount2")->Text);                                                 //kevin 20180307 第幾個 Count開始執行送訊號delay
    WriteIniData(szDir, "InitialMode",       "dInitialStartDelayDec3",                      FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDec3")->Text.ToDouble()));                      //kevin 20180307 每次等待時間
    WriteIniData(szDir, "InitialMode",       "iStartDelayCount3",                           EL<TEdit>("TFTestIF", "edtInitialDecCount3")->Text);                                                 //kevin 20180307 第幾個 Count開始執行送訊號delay
    WriteIniData(szDir, "InitialMode",       "dInitialStartDelayDec4",                      FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDec4")->Text.ToDouble()));                      //kevin 20180307 每次等待時間
    WriteIniData(szDir, "InitialMode",       "iStartDelayCount4",                           EL<TEdit>("TFTestIF", "edtInitialDecCount4")->Text);                                                 //kevin 20180307 第幾個 Count開始執行送訊號delay

    if(IniConfig.bI39SpiroxTesterLotEnd)                                        //JerryYang 20170706 (Steven) JCET通知測試機lot end功能改為by工作檔
        WriteIniData(szDir, "InitialMode",   "bSpiroxTesterLotEnd",                         EL<TCheckBox>("TFTestIF", "cbSpiroxTesterLotEnd")->Checked);
    else
        WriteIniData(szDir, "InitialMode",   "bSpiroxTesterLotEnd",                         false);

    WriteIniData(szDir, "InitialMode",       "bPurgeAir",                                   EL<TCheckBox>("TFTestIF", "cbPurgeAir")->Checked);                           //kevin 20180928 Contrarct time blower air loadboard
    WriteIniData(szDir, "InitialMode",       "iPurgeAir",                                   EL<TEdit>("TFTestIF", "edPurgeAir")->Text);                              //kevin 20180928 Contrarct time blower air loadboard

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_1_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_1_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_2_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_2_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_3_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_3_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_4_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_4_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_5_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_5_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_6_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_6_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_7_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_7_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_8_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_8_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_9_RT",              FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_9_RT")->Text.ToDouble()));
    WriteIniData(szDir, "InitialMode",       "dInitialDelay_10_RT",             FormatFloat("0.00", EL<TEdit>("TFTestIF", "edtInitialDelay_10_RT")->Text.ToDouble()));
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    WriteIniData(szDir, "Tester TCPIP",       "Address",                                    EL<TEdit>("TFTestIF", "edTCPIP_Address")->Text);                         //wei 20211027 open short TCP/IP
    WriteIniData(szDir, "Tester TCPIP",       "Port",                                       EL<TEdit>("TFTestIF", "edTCPIP_Port")->Text);                            //wei 20211027 open short TCP/IP
    WriteIniData(szDir, "InitialMode",        "bTTLUseASEJPMode",                           EL<TCheckBox>("TFTestIF", "cbASEJPMode")->Checked); //Frank 20220408 Add TTL ASE_JP Mode
    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210821 : 全智要求定時onecycle回home,start
    {
        WriteIniData(szDir, "Time",    "bAutoOnecycleHomStart", EL<TCheckBox>("TFTestIF", "cbAutoOnecycleHomStart")->Checked);       //Isaac 20210821 : 全智要求定時onecycle回home,start
        WriteIniData(szDir, "Time",    "iAutoOnecycleHomStartTime", EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime")->Text); //Isaac 20210821 : 全智要求定時onecycle回home,start
    }

    WriteIniData(szDir, "Time",      "bUseSocketHeating",    EL<TCheckBox>("TFTestIF", "cbUseSocketHeating")->Checked);//Ztex 2024.09.07 Add Use Socket Heating
    WriteIniData(szDir, "Time",      "iUseSocketHeating",    EL<TEdit>("TFTestIF", "edt_UseSocketHeating")->Text); //Ztex 2024.09.07 Add Use Socket Heating
    AddSpace(szDir);

}

// golden cTesterIF.cpp:497  TFTestIF::CheckRs232StandardIni()
static void TIF_CheckRs232StandardIni()
{

    int iTemp1, iTemp2;
    AnsiString szDir;
    bool bTemp;

    bTemp=false;
    szDir="D:\\RS232Standard\\System\\Setup.ini";

    //BaudRate
    iTemp1=ReadIniData(szDir, "COMPort", "BaudRate", 9600);                     //4800,7200,9600,14400,19200      //Steven 20211224 : baudrate改用數字
    iTemp2=TestIF_File.Rs232_Data.Baud_Rate;

    if(iTemp1!=iTemp2)
    {
        bTemp=true;
        WriteIniData(szDir, "COMPort", "BaudRate", iTemp2);
    }

    //ByteSize
    iTemp1=ReadIniData(szDir, "COMPort", "ByteSize", 7);                        //0:5,1:6,2:7,3:8
    if(TestIF_File.Rs232_Data.Bit_Length==0)                                    //0:7,1:8
        iTemp2=2;
    else
        iTemp2=3;
    if(iTemp1!=iTemp2)
    {
        bTemp=true;
        WriteIniData(szDir, "COMPort", "ByteSize", iTemp2);
    }

    //StopBits
    iTemp1=ReadIniData(szDir, "COMPort", "StopBits", 1);                        //0:1,1:1.5,2:2
    if(TestIF_File.Rs232_Data.Stop_Bit==0)                                      //0:1,1:2
        iTemp2=0;
    else
        iTemp2=2;
    if(iTemp1!=iTemp2)
    {
        bTemp=true;
        WriteIniData(szDir, "COMPort", "StopBits", iTemp2);
    }

    //Parity
    iTemp1=ReadIniData(szDir, "COMPort", "Parity"  , 2);                        //0:None,1:Odd,2:Even,3:Mark,4:Space
    if(TestIF_File.Rs232_Data.Parity==0)                                        //0:Even,1:Odd,2:None
        iTemp2=2;
    else if(TestIF_File.Rs232_Data.Parity==1)
        iTemp2=1;
    else
        iTemp2=0;
    if(iTemp1!=iTemp2)
    {
        bTemp=true;
        WriteIniData(szDir,"COMPort", "Parity", iTemp2);
    }

    bIsResetRs232Standard=bTemp;
    if(bIsResetRs232Standard)                                                   //Steven 20220323 : 針對RS232測試的參數被改變時, 要通知小程式一起改
    {
    filerw::ELTodo("golden cTesterIF.cpp:557 fMain->CloseGpibProgram(__FUNC__) not in the TfMain facade -- D:\\RS232Standard\\System\\Setup.ini was rewritten but the RS232 helper program is NOT told to close/reload (FTestIF->bIsResetRs232Standard=true is set as golden)");   // golden cTesterIF.cpp:557-557 取代：fMain->CloseGpibProgram：TfMain 門面沒有（golden main.cpp:29132：WM_COPYDATA MSG_CMD_CloseGpib 叫 GPIB／RS232 小程式關掉重讀 Setup.ini）。bIsResetRs232Standard 仍照 golden 設在 FTestIF 上（golden main.cpp:18520 的計時器另外靠它重啟小程式）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:557-557 -- fMain->CloseGpibProgram：TfMain 門面沒有（golden main.cpp:29132：WM_COPYDATA MSG_CMD_CloseGpib 叫 GPIB／RS232 小程式關掉重讀 Setup.ini）。bIsResetRs232Standard 仍照 golden 設在 FTestIF 上（golden main.cpp:18520 的計時器另外靠它重啟小程式）
        fMain->CloseGpibProgram(__FUNC__);
#endif // GATE (S12-C save)
    }

}

// golden cTesterIF.cpp:563  TFTestIF::ReadTestIFFile()
static void TIF_ReadTestIFFile()
{

    int iTemp;
    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";//, szDir2;
    szDir=DataPath+S;
    szDir+="\\Tester.Data";
//    szDir2.sprintf("%s%s\\Temperature.Data", DataPath, S);

    TestIF_File.iTestType                =ReadIniData(szDir, "Mode", "Tester Type",     0);

    if(TestIF_File.iTestType>EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->Items->Count-1)   // golden cTesterIF.cpp:575-575 取代：產生器的元件改寫不接在 ">" 後面（為了避開 "->"），這一行的 rgInterfaceType 沒改到 → 等價改寫（同一個判斷，DFM 4 項）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:575-575 -- 產生器的元件改寫不接在 ">" 後面（為了避開 "->"），這一行的 rgInterfaceType 沒改到 → 等價改寫（同一個判斷，DFM 4 項）
    if(TestIF_File.iTestType>rgInterfaceType->Items->Count-1)
#endif // GATE (S12-C save)
    {
        WriteIniData(szDir, "Mode", "Tester Type",       1);
        TestIF_File.iTestType            =GPIB_MODE;
    }

    if(TestIF_File.iTestType==TCP_IP_MODE)                                      //wei 20211027 open short TCP/IP
    {
        if(LastSet.iTester==OFF_LINE)                                           //wei 20211027 open short TCP/IP
        {
    filerw::ELTodo("golden cTesterIF.cpp:585-587 TCP/IP tester OFF_LINE: fTesterTCP->TimerTCPIPConnect/TimerProcessTCPData->Enabled=false, fLotInfo->labTCPIPSimulate->Visible=true -- not in the port facades (no TTimer / labTCPIPSimulate)");   // golden cTesterIF.cpp:585-587 取代：TCP/IP 模式＋OFF_LINE：fTesterTCP 的兩個 TTimer 與 fLotInfo->labTCPIPSimulate —— 移植樹門面沒有（forms/fTesterTCP.h 無 TTimer，TfLotInfo 無 labTCPIPSimulate）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:585-587 -- TCP/IP 模式＋OFF_LINE：fTesterTCP 的兩個 TTimer 與 fLotInfo->labTCPIPSimulate —— 移植樹門面沒有（forms/fTesterTCP.h 無 TTimer，TfLotInfo 無 labTCPIPSimulate）
            fTesterTCP->TimerTCPIPConnect->Enabled=false;
            fTesterTCP->TimerProcessTCPData->Enabled=false;
            fLotInfo->labTCPIPSimulate->Visible=true;
#endif // GATE (S12-C save)
        }
        else
        {
    filerw::ELTodo("golden cTesterIF.cpp:591-592 TCP/IP tester: fTesterTCP->TimerTCPIPConnect/TimerProcessTCPData->Enabled=true -- not ported (forms/fTesterTCP.h has no TTimer): the TCP/IP tester link is NOT started by this recipe");   // golden cTesterIF.cpp:591-592 取代：TCP/IP 模式：golden 啟動 fTesterTCP 的連線／收資料計時器（TCP/IP 測試機連線）—— 移植樹門面沒有 TTimer（forms/fTesterTCP.h:298-306）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:591-592 -- TCP/IP 模式：golden 啟動 fTesterTCP 的連線／收資料計時器（TCP/IP 測試機連線）—— 移植樹門面沒有 TTimer（forms/fTesterTCP.h:298-306）
            fTesterTCP->TimerTCPIPConnect->Enabled=true;
            fTesterTCP->TimerProcessTCPData->Enabled=true;
#endif // GATE (S12-C save)
        }

        if(IniConfig.bSECS_GEM_OneCycle && IniConfig.bN07_EnableSecsLotCheck)   //Steven 20230710 : OS測試機的工作檔也要上傳
        {
            if(DirectoryExists(IniConfig.sN07_6OSRecipePath)==false)
            {
                S.sprintf("Net Drive [N07-6] %s does not exists. Please make the folder first.", IniConfig.sN07_6OSRecipePath);
                filerw::ELMessage(S, "請設定OS測試機的網路硬碟");
            }
        }
        else if(IniConfig.bN06_CopyTesterFile)                                  //Steven 20250327 : OS測試機的工作檔也要上傳
        {
            if(DirectoryExists(IniConfig.asN06_TesterPath)==false)
            {
                S.sprintf("Net Drive [I55] %s does not exists. Please make the folder first.", IniConfig.asN06_TesterPath);
                filerw::ELMessage(S, "請設定OS測試機的網路硬碟");
            }
        }
    }
    else
    {
    ;   // golden cTesterIF.cpp:614-617 取代：非 TCP/IP 模式：golden 停掉 TCP/IP 計時器、關 ClientSocket_TCPIP、藏 labTCPIPSimulate。移植樹門面沒有這三樣，也就沒有被這條路徑打開過的 TCP/IP 連線可停 → 空敘述（不記 todo：每個非 TCP/IP 配方每次讀檔都會走到）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:614-617 -- 非 TCP/IP 模式：golden 停掉 TCP/IP 計時器、關 ClientSocket_TCPIP、藏 labTCPIPSimulate。移植樹門面沒有這三樣，也就沒有被這條路徑打開過的 TCP/IP 連線可停 → 空敘述（不記 todo：每個非 TCP/IP 配方每次讀檔都會走到）
        fTesterTCP->TimerTCPIPConnect->Enabled=false;
        fTesterTCP->TimerProcessTCPData->Enabled=false;
        fTesterTCP->ClientSocket_TCPIP->Close();
        fLotInfo->labTCPIPSimulate->Visible=false;
#endif // GATE (S12-C save)
    }
    fLotInfo->SettsChipAdvVisible();                                            //Steven 20250515 : 整合Open Short測試報表

    TestIF_File.iMaxTime                 =ReadIniData(szDir, "Time", "MAX Time",      0.0);
    TestIF_File.iInitialMaxTime          =ReadIniData(szDir, "Time", "Initial MAX Time", TestIF_File.iMaxTime);    //jou 2011-11-09 增加initial max time set
    TestIF_File.iDummyTime               =ReadIniData(szDir, "Time", "Dummy Time",    0.0);
    TestIF_File.dStartDelayTime          =ReadIniData(szDir, "Time", "Stary Delay",   0.0);

    //==> Ifor 20260807 : AMR loader delay time can be set by user (Lot page)
    TestIF_File.iAMRAutoStartDelay       =ReadIniData(szDir, "AMR", "Auto Start Delay",    15);
    TestIF_File.iAMRLoaderDetectDelay    =ReadIniData(szDir, "AMR", "Loader Detect Delay",  3);
    TestIF_File.iAMRWaitSupplyDelay      =ReadIniData(szDir, "AMR", "Wait Supply Delay",  360);
    TestIF_File.bAMRICQtyAutoCleanOut    =ReadIniData(szDir, "AMR", "IC Qty Auto Clean Out", true);      //Ifor 20260814 : 982 AMR IC qty auto clean out switch

    if(TestIF_File.iAMRAutoStartDelay<1 || TestIF_File.iAMRAutoStartDelay>600)
    {
        TestIF_File.iAMRAutoStartDelay=15;
    }

    if(TestIF_File.iAMRLoaderDetectDelay<1 || TestIF_File.iAMRLoaderDetectDelay>600)
    {
        TestIF_File.iAMRLoaderDetectDelay=3;
    }

    if(TestIF_File.iAMRWaitSupplyDelay<1 || TestIF_File.iAMRWaitSupplyDelay>3600)
    {
        TestIF_File.iAMRWaitSupplyDelay=360;
    }
    //<== Ifor 20260807

    TestIF_File.dInitStartDelayTime      =ReadIniData(szDir, "Time", "Initial Stary Delay",   0.0); //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    TestIF_File.iInitStartDelayTimeCT    =ReadIniData(szDir, "Time", "Initial Stary Delay CT",  0); //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    if(CosFunction.bFTRTDiffInitStartDelayTime)                                 //JerryYang 20220330 : FT RT可設定不同的initial delay time
    {
        TestIF_File.iFTMaxTime                 =ReadIniData(szDir, "Time", "MAX Time",                        TestIF_File.iMaxTime              );
        TestIF_File.iFTInitialMaxTime          =ReadIniData(szDir, "Time", "Initial MAX Time",                TestIF_File.iInitialMaxTime       );
        TestIF_File.iFTDummyTime               =ReadIniData(szDir, "Time", "Dummy Time",                      TestIF_File.iDummyTime            );
        TestIF_File.dFTStartDelayTime          =ReadIniData(szDir, "Time", "Stary Delay",                     TestIF_File.dStartDelayTime       );
        TestIF_File.dFTInitStartDelayTime      =ReadIniData(szDir, "Time", "Initial Stary Delay",             TestIF_File.dInitStartDelayTime   );
        TestIF_File.iFTInitStartDelayTimeCT    =ReadIniData(szDir, "Time", "Initial Stary Delay CT",          TestIF_File.iInitStartDelayTimeCT );

        TestIF_File.iRTMaxTime                 =ReadIniData(szDir, "Time", "RT MAX Time",               TestIF_File.iMaxTime              );
        TestIF_File.iRTInitialMaxTime          =ReadIniData(szDir, "Time", "RT Initial MAX Time",       TestIF_File.iInitialMaxTime       );    //jou 2011-11-09 增加initial max time set
        TestIF_File.iRTDummyTime               =ReadIniData(szDir, "Time", "RT Dummy Time",             TestIF_File.iDummyTime            );
        TestIF_File.dRTStartDelayTime          =ReadIniData(szDir, "Time", "RT Stary Delay",            TestIF_File.dStartDelayTime       );
        TestIF_File.dRTInitStartDelayTime      =ReadIniData(szDir, "Time", "RT Initial Stary Delay",    TestIF_File.dInitStartDelayTime   ); //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
        TestIF_File.iRTInitStartDelayTimeCT    =ReadIniData(szDir, "Time", "RT Initial Stary Delay CT", TestIF_File.iInitStartDelayTimeCT ); //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

        TestIF_File.iEQCMaxTime                 =ReadIniData(szDir, "Time", "EQC MAX Time",                 TestIF_File.iMaxTime              );
        TestIF_File.iEQCInitialMaxTime          =ReadIniData(szDir, "Time", "EQC Initial MAX Time",         TestIF_File.iInitialMaxTime       );
        TestIF_File.iEQCDummyTime               =ReadIniData(szDir, "Time", "EQC Dummy Time",               TestIF_File.iDummyTime            );
        TestIF_File.dEQCStartDelayTime          =ReadIniData(szDir, "Time", "EQC Stary Delay",              TestIF_File.dStartDelayTime       );
        TestIF_File.dEQCInitStartDelayTime      =ReadIniData(szDir, "Time", "EQC Initial Stary Delay",      TestIF_File.dInitStartDelayTime   );
        TestIF_File.iEQCInitStartDelayTimeCT    =ReadIniData(szDir, "Time", "EQC Initial Stary Delay CT",   TestIF_File.iInitStartDelayTimeCT );
        if(iRunStartMode==RT)
        {
            TestIF_File.iMaxTime                   =TestIF_File.iRTMaxTime;
            TestIF_File.iInitialMaxTime            =TestIF_File.iRTInitialMaxTime;
            TestIF_File.iDummyTime                 =TestIF_File.iRTDummyTime;
            TestIF_File.dStartDelayTime            =TestIF_File.dRTStartDelayTime;
            TestIF_File.dInitStartDelayTime        =TestIF_File.dRTInitStartDelayTime;
            TestIF_File.iInitStartDelayTimeCT      =TestIF_File.iRTInitStartDelayTimeCT;
        }
        else if(fSCKART->sInfo_Stage.Pos("QC")==1)
        {
            TestIF_File.iMaxTime                   =TestIF_File.iEQCMaxTime;
            TestIF_File.iInitialMaxTime            =TestIF_File.iEQCInitialMaxTime;
            TestIF_File.iDummyTime                 =TestIF_File.iEQCDummyTime;
            TestIF_File.dStartDelayTime            =TestIF_File.dEQCStartDelayTime;
            TestIF_File.dInitStartDelayTime        =TestIF_File.dEQCInitStartDelayTime;
            TestIF_File.iInitStartDelayTimeCT      =TestIF_File.iEQCInitStartDelayTimeCT;
        }
        else
        {
            TestIF_File.iMaxTime                         =TestIF_File.iFTMaxTime;
            TestIF_File.iInitialMaxTime                  =TestIF_File.iFTInitialMaxTime;
            TestIF_File.iDummyTime                       =TestIF_File.iFTDummyTime;
            TestIF_File.dStartDelayTime                  =TestIF_File.dFTStartDelayTime;
            TestIF_File.dInitStartDelayTime              =TestIF_File.dFTInitStartDelayTime;
            TestIF_File.iInitStartDelayTimeCT            =TestIF_File.iFTInitStartDelayTimeCT;
        }
    }

    TestIF_File.dInitWaitTime            =ReadIniData(szDir, "Time", "Init Wait Time",   1.0);  //jou 2013-09-25 Testing Need Stop All Motor
    TestIF_File.dTestingWaitTime         =ReadIniData(szDir, "Time", "Testing Wait Time",3.0);  //jou 2013-09-25 Testing Need Stop All Motor

    //ChungHung 20140730 add for ATK function after tested delay time
    TestIF_File.dAfterTestedDelay        =ReadIniData(szDir, "Time", "After Tested Delay", 0.0);

    TestIF_File.iDioMode                 =ReadIniData(szDir, "DIO", "Type",             0);

    if(TestIF_File.iDioMode<0)
        TestIF_File.iDioMode=0;

    //ChungHung 20140425 add for TSMC Device
    TestIF_File.bEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",          false);

    if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        TestIF_File.bUseOtherArmToTestAfterInitialDelay     =ReadIniData(szDir, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",       false);
    else
        TestIF_File.bUseOtherArmToTestAfterInitialDelay     =false;

    TestIF_File.bAfterShowAlarmMessageUseInitialDelay       =ReadIniData(szDir, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",     false);
    TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay   =ReadIniData(szDir, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay", false);
    TestIF_File.iEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",          1.00);

    #ifdef SOFT_SIMULTE                                                         //Steven 20241108 : 軟體模擬有做五秒就好
    TestIF_File.iInitialDelay                               =5;
    TestIF_File.iInitialDelay_2                             =5;
    TestIF_File.iInitialDelay_3                             =5;
    TestIF_File.iInitialDelay_4                             =5;
    TestIF_File.iInitialDelay_5                             =5;
    TestIF_File.iInitialDelay_6                             =5;
    TestIF_File.iInitialDelay_7                             =5;
    TestIF_File.iInitialDelay_8                             =5;
    TestIF_File.iInitialDelay_9                             =5;
    TestIF_File.dInitialDelay_10                            =5;
    #else
    TestIF_File.iInitialDelay                               =ReadIniData(szDir, "InitialMode",       "iInitialDelay",            1.00);     //ChungHung 20141210 add for SCK want to every event have delay
    TestIF_File.iInitialDelay_2                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_2",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay
    TestIF_File.iInitialDelay_3                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_3",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay
    TestIF_File.iInitialDelay_4                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_4",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay
    TestIF_File.iInitialDelay_5                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_5",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay
    TestIF_File.iInitialDelay_6                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_6",          1.00);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    TestIF_File.iInitialDelay_7                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_7",          1.00);     //wei 20151228 No FullSite delay
    TestIF_File.iInitialDelay_8                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_8",          1.00);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    TestIF_File.iInitialDelay_9                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_9",          1.00);     //Steven 20160818 : OTD打開Delay
    TestIF_File.dInitialDelay_10                            =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10",         0.0);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41 wait time
    #endif

    if(CosFunction.bHiSiliconFunction && CUSTOMER_CODE==CC_ASE_KaohSiung)       //kevin 20191227 add  Initial Start Delay Time 需卡下限, 最小30秒.
    {
        if(TestIF_File.dInitStartDelayTime<30)                                  //kevin 20200110
        {
            TestIF_File.dInitStartDelayTime=30;
            TestIF_File.iInitialDelay=30;
        }
    }

    TestIF_File.bOTDUnlockDelay                             =ReadIniData(szDir, "InitialMode",       "bOTDUnlockDelay",                             false);     //Steven 20160818 : OTD打開Delay
    TestIF_File.bWhenPressStopOverUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           false);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    TestIF_File.iWhenPressStopOver                          =ReadIniData(szDir, "InitialMode",       "iWhenPressStopOver",                          1.00);      //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    TestIF_File.bWhenNoFullSiteUseInitialDelay              =ReadIniData(szDir, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              false);     //wei 20151228 No FullSite delay
    TestIF_File.bAfterAutoCleanFunctionUseInitialDelay      =ReadIniData(szDir, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      false);
    TestIF_File.bTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "bTestFinishToNextTestOver",                   false);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    TestIF_File.iTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "iTestFinishToNextTestOver",                   1.00);      //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    TestIF_File.bAfterOpenHeatDoorUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           false);     //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    TestIF_File.bTestStartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "bTestStartToNextTestStart",                   false);     //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41
    TestIF_File.dTeststartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "dTeststartToNextTestStart",                   1.00);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41  over time

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    TestIF_File.dInitialDelay_1_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_1_RT",              0.0);
    TestIF_File.dInitialDelay_2_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_2_RT",              0.0);
    TestIF_File.dInitialDelay_3_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_3_RT",              0.0);
    TestIF_File.dInitialDelay_4_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_4_RT",              0.0);
    TestIF_File.dInitialDelay_5_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_5_RT",              0.0);
    TestIF_File.dInitialDelay_6_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_6_RT",              0.0);
    TestIF_File.dInitialDelay_7_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_7_RT",              0.0);
    TestIF_File.dInitialDelay_8_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_8_RT",              0.0);
    TestIF_File.dInitialDelay_9_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_9_RT",              0.0);
    TestIF_File.dInitialDelay_10_RT                         =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10_RT",             0.0);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    TestIF_File.iEnStartDelayCount                          =ReadIniData(szDir, "InitialMode",       "iEnStartDelayCount",          0);             //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay
    TestIF_File.dInitialStartDelayDec[0]                    =ReadIniData(szDir, "InitialMode",       "dInitialStartDelayDec1",   0.00);             //kevin 20180307 每次等待時間
    TestIF_File.iStartDelayCount[0]                         =ReadIniData(szDir, "InitialMode",       "iStartDelayCount1",           0);             //kevin 20180307 第幾個 Count開始執行送訊號delay
    TestIF_File.dInitialStartDelayDec[1]                    =ReadIniData(szDir, "InitialMode",       "dInitialStartDelayDec2",   0.00);             //kevin 20180307 每次等待時間
    TestIF_File.iStartDelayCount[1]                         =ReadIniData(szDir, "InitialMode",       "iStartDelayCount2",           0);             //kevin 20180307 第幾個 Count開始執行送訊號delay
    TestIF_File.dInitialStartDelayDec[2]                    =ReadIniData(szDir, "InitialMode",       "dInitialStartDelayDec3",   0.00);             //kevin 20180307 每次等待時間
    TestIF_File.iStartDelayCount[2]                         =ReadIniData(szDir, "InitialMode",       "iStartDelayCount3",           0);             //kevin 20180307 第幾個 Count開始執行送訊號delay
    TestIF_File.dInitialStartDelayDec[3]                    =ReadIniData(szDir, "InitialMode",       "dInitialStartDelayDec4",   0.00);             //kevin 20180307 每次等待時間
    TestIF_File.iStartDelayCount[3]                         =ReadIniData(szDir, "InitialMode",       "iStartDelayCount4",           0);             //kevin 20180307 第幾個 Count開始執行送訊號delay

    if(IniConfig.bI39SpiroxTesterLotEnd==true)                                  //JerryYang 20170706 (Steven) JCET通知測試機lot end功能改為by工作檔
        TestIF_File.bSpiroxTesterLotEnd                     =ReadIniData(szDir, "InitialMode",       "bSpiroxTesterLotEnd",         false);
    else
        TestIF_File.bSpiroxTesterLotEnd=false;

    TestIF_File.bPurgeAirAfterContract                      =ReadIniData(szDir, "InitialMode",       "bPurgeAir",   false);                             //kevin 20180928 Contrarct time blower air loadboard
    TestIF_File.iPurgeAirContract                           =ReadIniData(szDir, "InitialMode",       "iPurgeAir",   0);   //kevin 20180928 Contrarct time blower air loadboard

    TestIF_File.asTester_Address                            =ReadIniData(szDir, "Tester TCPIP",      "Address",                  AnsiString("172.16.8.150"));           //wei 20211027 open short TCP/IP
    TestIF_File.iTester_Port                                =ReadIniData(szDir, "Tester TCPIP",      "Port",                     6000);                                 //wei 20211027 open short TCP/IP

    TestIF_File.bTTLUseASEJPMode                            =ReadIniData(szDir, "InitialMode",       "bTTLUseASEJPMode",          false);   //Frank 20220408 Add TTL ASE_JP Mode

    fTesterTCP->edTCPIP_Address->Text                       =TestIF_File.asTester_Address;          //wei 20211027 open short TCP/IP
    fTesterTCP->edTCPIP_Port   ->Text                       =TestIF_File.iTester_Port;              //wei 20211027 open short TCP/IP

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210821 : 全智要求定時onecycle回home,start
    {
        TestIF_File.bAutoOnecycleHomStart                       =ReadIniData(szDir, "Time",              "bAutoOnecycleHomStart",   false);     //Isaac 20210821 : 全智要求定時onecycle回home,start
        TestIF_File.iAutoOnecycleHomStartTime                   =ReadIniData(szDir, "Time",              "iAutoOnecycleHomStartTime",   0);     //Isaac 20210821 : 全智要求定時onecycle回home,start
    }
    else
    {
        TestIF_File.bAutoOnecycleHomStart=false;
        TestIF_File.iAutoOnecycleHomStartTime=0;
    }
    //Steven 20101008 : Start
    if(CheckIniData(szDir, "DIO", "TypeName")==false)
    {
        WIN32_FIND_DATA filedata1;                                              // Structure for file data
        HANDLE filehandle1;                                                     // Handle for searching
        AnsiString szFileName;
        filehandle1=FindFirstFile((DIOCFGPath + "*.ini").c_str(), &filedata1);
        TStringList *MyList=new TStringList();
        if(filehandle1!=INVALID_HANDLE_VALUE)
        {
            do
            {
                /* 不處理隱藏檔及 . 跟 .. */
                if((filedata1.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                    strcmp(filedata1.cFileName, ".")==0 ||
                    strcmp(filedata1.cFileName, "..")==0)
                    continue;

                if(ExtractFileExt(filedata1.cFileName).LowerCase()==".ini")     // 若找到的檔案的副檔名是 .ini
                {
                    szFileName=ChangeFileExt(ExtractFileName(filedata1.cFileName), "");  // 取出檔名，其實就是把副檔名設成""
                    MyList->Add(szFileName);
                }
            } while(FindNextFile(filehandle1, &filedata1));
            FindClose(filehandle1);
        }

    if(TestIF_File.iDioMode<MyList->Count) { TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode]; WriteIniData(szDir, "DIO",       "TypeName",     TestIF_File.sDioName); } else { AnsiString e; e.sprintf("golden cTesterIF.cpp:846 MyList->Strings[%d] out of range (%d DIO files in DIOCFGPath) -- golden throws EStringListError here and :847 never writes; TypeName NOT written, TestIF_File.sDioName left as \"%s\"", TestIF_File.iDioMode, MyList->Count, TestIF_File.sDioName); std::printf("FileRW TesterIF: %s\n", e.c_str()); filerw::ELTodo(e.c_str()); }   // golden cTesterIF.cpp:846-847 取代：golden :846 MyList->Strings[iDioMode] 沒檢查範圍：BCB TStringList 丟 EStringListError（:847 不會寫）；vclcompat TStringList 回 ""（TStringList.cpp:76）→ 照抄會把 TypeName="" 寫進 Tester.Data。範圍內照 golden 取值＋寫 TypeName；範圍外不取、不寫、回報（同 FileRW/TTLCfg.cpp FileRW_TTLCfg_ReadDIOSection 的決斷）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:846-847 -- golden :846 MyList->Strings[iDioMode] 沒檢查範圍：BCB TStringList 丟 EStringListError（:847 不會寫）；vclcompat TStringList 回 ""（TStringList.cpp:76）→ 照抄會把 TypeName="" 寫進 Tester.Data。範圍內照 golden 取值＋寫 TypeName；範圍外不取、不寫、回報（同 FileRW/TTLCfg.cpp FileRW_TTLCfg_ReadDIOSection 的決斷）
        TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode];
        WriteIniData(szDir, "DIO",       "TypeName",     TestIF_File.sDioName);
#endif // GATE (S12-C save)
        MyList->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete MyList;
    }
    else
    {
        TestIF_File.sDioName             =ReadIniData(szDir, "DIO", "TypeName", AnsiString(""));
    }
    //Steven 20101008 : End

    TestIF_File.bAntiSignal              =ReadIniData(szDir, "DIO", "Anti Signal",      0);
    TestIF_File.iGpibMode                =ReadIniData(szDir, "GP-IB", "Type",           0);
    //Steven 20161208 : ART need to set to 32binGS for ATK
    //==>
    if(USE_AUTO_RETEST==eartInstall       &&
       CosFunction.bUseSCKART==true       &&
       IniConfig.bA10_AutoReTest==true    &&
       IniConfig.bA10TestModeForART==true)
    {
        TestIF_File.iGpibMode=IniConfig.iA10TestModeForART;
    }
    //<==
    //Steven 20161208 : ART need to set to 32binGS for ATK

    //fSCKART->SetGPIBVersion(fSCKART->edtVersion->Text);                       //Steven 20230811 : 針對GPIB發送Version命令, 統一處理          //Jimmychiu 20231205 : Mark SetGPIBVersion function

    TestIF_File.iGpibAddress             =ReadIniData(szDir, "GP-IB", "Address",        0);

    if(IniConfig.bAMDFunction)                                                 //Ifor 20191025 : AMD 預設開啟AMD Function
    {
        TestIF_File.bAMDFunction        =ReadIniData(szDir, "GP-IB", "AMD Function",   true);
    }
    else
    {
        TestIF_File.bAMDFunction        =ReadIniData(szDir, "GP-IB", "AMD Function",   false);
    }

    if(IniConfig.bAMDFunction)                                                 //JerryYang 20200422 2DID format選項改用下拉選單
    {
        TestIF_File.i2DIDFormat         =ReadIniData(szDir, "GP-IB", "2DIDFormat",   1);
    }
    else
    {
        TestIF_File.i2DIDFormat         =ReadIniData(szDir, "GP-IB", "2DIDFormat",   int(TestIF_File.bAMDFunction));
    }

    TestIF_File.iRs232Mode              =(eRs232Mode)ReadIniData(szDir, "RS-232C", "Type",         0);

    TestIF_File.iRs232MaxBinCount       =ReadIniData(szDir, "RS-232C", "Bin Count",    32);    //Steven 20121116 : RS232回傳的最大Bin數

    if(CheckIniData(szDir, "RS-232C",   "BaudRate")==false)
    {
        iTemp                           =ReadIniData(szDir, "RS-232C", "Baud Rate",    0);
        if(iTemp==0)//0:19200,1:9600,2:4800
            TestIF_File.Rs232_Data.Baud_Rate=19200;
        else if(iTemp==1)
            TestIF_File.Rs232_Data.Baud_Rate=9600;
        else
            TestIF_File.Rs232_Data.Baud_Rate=4800;
        WriteIniData(szDir, "RS-232C",   "BaudRate", TestIF_File.Rs232_Data.Baud_Rate);
    }
    else
    {
        TestIF_File.Rs232_Data.Baud_Rate =ReadIniData(szDir, "RS-232C", "BaudRate",     0);
    }
    TestIF_File.Rs232_Data.Bit_Length    =ReadIniData(szDir, "RS-232C", "Bit Length",   0);
    TestIF_File.Rs232_Data.Stop_Bit      =ReadIniData(szDir, "RS-232C", "Stop Bit",     0);
    TestIF_File.Rs232_Data.Parity        =ReadIniData(szDir, "RS-232C", "Parity",       0);

//    TestIF_File.bRS232NeedSendVSOT       =ReadIniData(szDir, "RS-232C", "NeedVSOT",     0);
    if(CosFunction.bArm2ForFingerPrintTest)                                     //Steven 20140922 : Arm2當作指紋測試
        TestIF_File.bForEgisTecTest      =ReadIniData(szDir, "RS-232C", "ForEgisTecTest",   false);
    else
        TestIF_File.bForEgisTecTest      =false;

    if(TestIF_File.iTestType==RS232_MODE &&
       TestIF_File.iRs232Mode==eRs23232Bin)                                     //Steven 20121112 : RS232支援32Bin b8080 --> bSupport32Bin
        iTestBinCount=TestIF_File.iRs232MaxBinCount;
    //else
    //    iTestBinCount=15;
    else if(TestIF_File.iTestType==GPIB_MODE &&
            TestIF_File.iGpibMode==InterfaceType_256Bin)                        //kevin 20140308 add 256Bin
        iTestBinCount=255;                                                      //Steven 20200602 : 256 --> 255
    else if(TestIF_File.iTestType==GPIB_MODE &&
            (TestIF_File.iGpibMode==InterfaceType_16Bin ||                      //Steven 20140805 ADD 16Bin and 32Bin
             TestIF_File.iGpibMode==InterfaceType_16BinGS))                     //Steven 20161122 (jou) : Add 16 bin GS and 32 bin GS
        iTestBinCount=17;
    else if(TestIF_File.iTestType==GPIB_MODE &&
            (TestIF_File.iGpibMode==InterfaceType_32Bin ||
             TestIF_File.iGpibMode==InterfaceType_32BinGS ||                    //Steven 20161122 (jou) : Add 16 bin GS and 32 bin GS
    (CUSTOMER_CODE==CC_KYEC_LEE && TIF_AOIFailBinUse())))    //Eastsun 20260710 Merge   // golden cTesterIF.cpp:937-937 取代：FrmAOI->bCheckAOIFailBinUse()（CC_KYEC_LEE 的 AOI fail bin，Eastsun 20260710）：移植樹 TFrmAOI（forms/fAOI.h）沒有這支；golden 本體 fAOI.cpp:9876 是純查詢（無副作用）→ 客戶碼先判（&& 左右交換不改結果），KYEC_LEE 才記 todo、當 false
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:937-937 -- FrmAOI->bCheckAOIFailBinUse()（CC_KYEC_LEE 的 AOI fail bin，Eastsun 20260710）：移植樹 TFrmAOI（forms/fAOI.h）沒有這支；golden 本體 fAOI.cpp:9876 是純查詢（無副作用）→ 客戶碼先判（&& 左右交換不改結果），KYEC_LEE 才記 todo、當 false
             (FrmAOI->bCheckAOIFailBinUse() && CUSTOMER_CODE==CC_KYEC_LEE)))    //Eastsun 20260710 Merge
#endif // GATE (S12-C save)
        iTestBinCount=33;
    else
        iTestBinCount=16;

//    if(iTestBinCount>256)  //kevin 20140308
//        iTestBinCount=255;

    if(fShowBinSelect!=NULL)
        fShowBinSelect->ShowInitialString();

    if(IniConfig.bD58UseArm1PickPlaceArm2Test)
    {
        if(TestIF_File.bForEgisTecTest==true)                                   //Steven 20140922 : Arm2當作指紋測試
        {
            fMain->SendMSG_CMD(MSG_CMD_2ArmTestMode);
        }
        else
        {
            if(TestIF_File.bArm1PickPlaceArm2Test)
                fMain->SendMSG_CMD(MSG_CMD_1ArmTestMode);
            else
                fMain->SendMSG_CMD(MSG_CMD_2ArmTestMode);
        }
    }

    if(TestIF_File.iTestType==RS232_MODE)                                       //Sam 20181219 : Handler 與 Rs232Standard 設定同步
    {
        TIF_CheckRs232StandardIni();
    }

    if(Temperature.bBoostFuncttion)                                             //Steven 20180817 : Boost Function
    {
        TestIF_File.bEveryFirstDeviceUseInitialDelay=false;
        TestIF_File.bUseOtherArmToTestAfterInitialDelay=false;
        TestIF_File.bAfterShowAlarmMessageUseInitialDelay=false;
        TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay=false;
        TestIF_File.bAfterAutoCleanFunctionUseInitialDelay=false;
        TestIF_File.bAfterOpenHeatDoorUseInitialDelay=false;
        TestIF_File.bWhenPressStopOverUseInitialDelay=false;
        TestIF_File.bWhenNoFullSiteUseInitialDelay=false;
        TestIF_File.bTestFinishToNextTestOver=false;
        TestIF_File.bOTDUnlockDelay=false;
    }

    if(TestIF_File.iShuttleMode==1)                                             //JerryYang 20240710 : 單ARM模式不支援換ARM預熱
    {
        TestIF_File.bUseOtherArmToTestAfterInitialDelay=false;
    }

    if(TestIF_File.bTTLUseASEJPMode)                                            //Frank 20220408 Add TTL ASE_JP Mode
    {
        TTLRS232VerCheck=11040801;
    }
    if(ATKRecipeInfo) ATKRecipeInfo->SaveFile(); else if(CUSTOMER_CODE==CC_AMKOR_Korea || CUSTOMER_CODE==CC_AMKOR_China) filerw::ELTodo("golden cTesterIF.cpp:991 ATKRecipeInfo->SaveFile() (AMKOR Information.txt / D:\\eRMS) not run -- ATKRecipeInfo is not created in the port (database.cpp:149)");   // golden cTesterIF.cpp:991-991 取代：ATKRecipeInfo：移植樹沒有建立（database.cpp:149 註解掉）→ 空指標。golden ATK_RECIPE_INFO::SaveFile（移植樹 cprod.cpp:497）非 CC_AMKOR_Korea／China 一進去就 return → 指標在就照呼叫；不在時 AMKOR 才記 todo（其他客戶 golden 本來就什麼都不做）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:991-991 -- ATKRecipeInfo：移植樹沒有建立（database.cpp:149 註解掉）→ 空指標。golden ATK_RECIPE_INFO::SaveFile（移植樹 cprod.cpp:497）非 CC_AMKOR_Korea／China 一進去就 return → 指標在就照呼叫；不在時 AMKOR 才記 todo（其他客戶 golden 本來就什麼都不做）
    ATKRecipeInfo->SaveFile();                                                  //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
#endif // GATE (S12-C save)
    TestIF_File.bUseSocketHeating    =ReadIniData(szDir, "Time", "bUseSocketHeating",   TestIF_File.bUseSocketHeating );//Ztex 2024.09.07 Add Use Socket Heating
    TestIF_File.iUseSocketHeating    =ReadIniData(szDir, "Time", "iUseSocketHeating",   0 );                            //Ztex 2024.09.07 Add Use Socket Heating
    TIF_DoIniDataToForm();

}

// golden cTesterIF.cpp:997  TFTestIF::DoIniDataToForm()
static void TIF_DoIniDataToForm()
{

    EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex      =TestIF_File.iTestType;

    if(CosFunction.bFTRTDiffInitStartDelayTime)                                                     //JerryYang 20220330 : FT RT可設定不同的initial delay time
    {
        EL<TEdit>("TFTestIF", "edInitialMaxTest")->Text          =FormatFloat("0.00",TestIF_File.iFTInitialMaxTime);         //jou 2011-11-09 增加initial max time set
        EL<TEdit>("TFTestIF", "edMaxTestTime")->Text             =FormatFloat("0.00",TestIF_File.iFTMaxTime);
        EL<TEdit>("TFTestIF", "edDummyTestTime")->Text           =FormatFloat("0.00",TestIF_File.iFTDummyTime);
        EL<TEdit>("TFTestIF", "edStartDelayTime")->Text          =FormatFloat("0.00",TestIF_File.dFTStartDelayTime);
        EL<TEdit>("TFTestIF", "edtInitStartDelay")->Text         =FormatFloat("0.00",TestIF_File.dFTInitStartDelayTime);     //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
        EL<TEdit>("TFTestIF", "edtInitStartDelayCT")->Text       =TestIF_File.iFTInitStartDelayTimeCT;                       //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    }
    else
    {
        EL<TEdit>("TFTestIF", "edInitialMaxTest")->Text          =FormatFloat("0.00",TestIF_File.iInitialMaxTime);           //jou 2011-11-09 增加initial max time set
        EL<TEdit>("TFTestIF", "edMaxTestTime")->Text             =FormatFloat("0.00",TestIF_File.iMaxTime);
        EL<TEdit>("TFTestIF", "edDummyTestTime")->Text           =FormatFloat("0.00",TestIF_File.iDummyTime);
        EL<TEdit>("TFTestIF", "edStartDelayTime")->Text          =FormatFloat("0.00",TestIF_File.dStartDelayTime);

        EL<TEdit>("TFTestIF", "edtInitStartDelay")->Text         =FormatFloat("0.00",TestIF_File.dInitStartDelayTime);       //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
        EL<TEdit>("TFTestIF", "edtInitStartDelayCT")->Text       =TestIF_File.iInitStartDelayTimeCT;                         //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    }

    EL<TEdit>("TFTestIF", "edInitialMaxTest_RT")->Text          =FormatFloat("0.00",TestIF_File.iRTInitialMaxTime);          //jou 2011-11-09 增加initial max time set
    EL<TEdit>("TFTestIF", "edMaxTestTime_RT")->Text             =FormatFloat("0.00",TestIF_File.iRTMaxTime);
    EL<TEdit>("TFTestIF", "edDummyTestTime_RT")->Text           =FormatFloat("0.00",TestIF_File.iRTDummyTime);
    EL<TEdit>("TFTestIF", "edStartDelayTime_RT")->Text          =FormatFloat("0.00",TestIF_File.dRTStartDelayTime);
    EL<TEdit>("TFTestIF", "edtInitStartDelay_RT")->Text         =FormatFloat("0.00",TestIF_File.dRTInitStartDelayTime);      //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_RT")->Text       =TestIF_File.iRTInitStartDelayTimeCT;                        //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

    EL<TEdit>("TFTestIF", "edInitialMaxTest_EQC")->Text          =FormatFloat("0.00",TestIF_File.iEQCInitialMaxTime);        //jou 2011-11-09 增加initial max time set
    EL<TEdit>("TFTestIF", "edMaxTestTime_EQC")->Text             =FormatFloat("0.00",TestIF_File.iEQCMaxTime);
    EL<TEdit>("TFTestIF", "edDummyTestTime_EQC")->Text           =FormatFloat("0.00",TestIF_File.iEQCDummyTime);
    EL<TEdit>("TFTestIF", "edStartDelayTime_EQC")->Text          =FormatFloat("0.00",TestIF_File.dEQCStartDelayTime);
    EL<TEdit>("TFTestIF", "edtInitStartDelay_EQC")->Text         =FormatFloat("0.00",TestIF_File.dEQCInitStartDelayTime);    //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    EL<TEdit>("TFTestIF", "edtInitStartDelayCT_EQC")->Text       =TestIF_File.iEQCInitStartDelayTimeCT;                      //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

    EL<TEdit>("TFTestIF", "edtInitWaitTime")->Text           =FormatFloat("0.00", TestIF_File.dInitWaitTime);                //jou 2013-09-25 Testing Need Stop All Motor
    EL<TEdit>("TFTestIF", "edtTestingWaitTime")->Text        =FormatFloat("0.00", TestIF_File.dTestingWaitTime);             //jou 2013-09-25 Testing Need Stop All Motor

    //ChungHung 20140425 add for TSMC Device
    EL<TCheckBox>("TFTestIF", "cbEveryFirstDeviceUseInitialDelay")->Checked          =TestIF_File.bEveryFirstDeviceUseInitialDelay;
    EL<TCheckBox>("TFTestIF", "cbUseOtherArmToTestAfterInitialDelay")->Checked       =TestIF_File.bUseOtherArmToTestAfterInitialDelay;               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    EL<TCheckBox>("TFTestIF", "cbAfterShowAlarmMessageUseInitialDelay")->Checked     =TestIF_File.bAfterShowAlarmMessageUseInitialDelay;
    EL<TCheckBox>("TFTestIF", "cbWhenHappenTestedTimeBelowUseInitialDelay")->Checked =TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay;
    EL<TCheckBox>("TFTestIF", "cbAfterAutoCleanFunctionUseInitialDelay")->Checked    =TestIF_File.bAfterAutoCleanFunctionUseInitialDelay;            //ChungHung 20141017 add for SCK Add for TSMC Device
    EL<TCheckBox>("TFTestIF", "cbAfterOpenHeatDoorUseInitialDelay")->Checked         =TestIF_File.bAfterOpenHeatDoorUseInitialDelay;                 //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    EL<TEdit>("TFTestIF", "edBelowSec")->Text                                    =FormatFloat("0.00", TestIF_File.iEveryFirstDeviceUseInitialDelay);
    EL<TEdit>("TFTestIF", "edtInitialDelay_1")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay);                //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_2")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_2);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_3")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_3);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_4")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_4);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_5")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_5);              //ChungHung 20141210 add for SCK want to every event have delay
    EL<TEdit>("TFTestIF", "edtAfterTestedDelay")->Text                           =FormatFloat("0.00", TestIF_File.dAfterTestedDelay);            //ChungHung 20140730 add for ATK function after tested delay time

    EL<TCheckBox>("TFTestIF", "cbWhenPressStopOverUseInitialDelay")->Checked         =TestIF_File.bWhenPressStopOverUseInitialDelay;                 //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    EL<TEdit>("TFTestIF", "edOverSec")->Text                                     =FormatFloat("0.00", TestIF_File.iWhenPressStopOver);           //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_6")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_6);              //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    EL<TCheckBox>("TFTestIF", "cbWhenNoFullSiteUseInitialDelay")->Checked            =TestIF_File.bWhenNoFullSiteUseInitialDelay;                    //wei 20151228 No FullSite delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_7")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_7);              //wei 20151228 No FullSite delay

    EL<TCheckBox>("TFTestIF", "cbTestFinishToNextTestOver")->Checked                 =TestIF_File.bTestFinishToNextTestOver;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    EL<TEdit>("TFTestIF", "iTestFinishToNextTestOver")->Text                     =FormatFloat("0.00", TestIF_File.iTestFinishToNextTestOver);    //kevin 20160310
    EL<TEdit>("TFTestIF", "edtInitialDelay_8")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_8);              //kevin 20160310

    EL<TCheckBox>("TFTestIF", "cbOTDUnlockDelay")->Checked                           =TestIF_File.bOTDUnlockDelay;                                   //Steven 20160818 : OTD打開Delay
    EL<TEdit>("TFTestIF", "edtInitialDelay_9")->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_9);              //Steven 20160818 : OTD打開Delay
    EL<TCheckBox>("TFTestIF", "cbTestStartToNextTestStart")->Checked                 =TestIF_File.bTestStartToNextTestStart;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    EL<TEdit>("TFTestIF", "iTeststartToNextTestStart")->Text                     =FormatFloat("0.00", TestIF_File.dTeststartToNextTestStart);    //kevin 20160310
    EL<TEdit>("TFTestIF", "edtInitialDelay_10")->Text                            =FormatFloat("0.00", TestIF_File.dInitialDelay_10);             //kevin 20181031 : SOT 超過時間Delay

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    EL<TEdit>("TFTestIF", "edtInitialDelay_1_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_1_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_2_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_2_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_3_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_3_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_4_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_4_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_5_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_5_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_6_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_6_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_7_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_7_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_8_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_8_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_9_RT")->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_9_RT);
    EL<TEdit>("TFTestIF", "edtInitialDelay_10_RT")->Text                         =FormatFloat("0.00", TestIF_File.dInitialDelay_10_RT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    EL<TEdit>("TFTestIF", "edTCPIP_Address")->Text                               =TestIF_File.asTester_Address;                                  //wei 20211027 open short TCP/IP
    EL<TEdit>("TFTestIF", "edTCPIP_Port")->Text                                  =TestIF_File.iTester_Port;                                      //wei 20211027 open short TCP/IP
    //Steven 20101008 Start
    EL<TCheckBox>("TFTestIF", "cbASEJPMode")->Checked                                =TestIF_File.bTTLUseASEJPMode;                                  //Frank 20220408 Add TTL ASE_JP Mode
    if(TestIF_File.sDioName!="")
    {
        for(int i=0; i<EL<TComboBox>("TFTestIF", "cbDIOType")->Items->Count; i++)
        {
            if(TestIF_File.sDioName==EL<TComboBox>("TFTestIF", "cbDIOType")->Items->Strings[i])
            {
                filerw::ELComboIndex(EL<TComboBox>("TFTestIF", "cbDIOType"), i);
                break;
            }
        }
    }
    else
    {
        if(TestIF_File.iDioMode<0)  //Steven 20110105
            TestIF_File.iDioMode=0;
        filerw::ELComboIndex(EL<TComboBox>("TFTestIF", "cbDIOType"), TestIF_File.iDioMode);
    }
    //Steven 20101008 End

    filerw::ELComboIndex(EL<TComboBox>("TFTestIF", "cbGPIBType"), TestIF_File.iGpibMode);
    EL<TEdit>("TFTestIF", "edGPIBAddress")->Text             =TestIF_File.iGpibAddress;
    EL<TRadioGroup>("TFTestIF", "rg2DID_Format")->ItemIndex        =TestIF_File.i2DIDFormat;                   //JerryYang 20200422 2DID format選項改用下拉選單

    filerw::ELComboIndex(EL<TComboBox>("TFTestIF", "cbRs232Type"), TestIF_File.iRs232Mode);
    EL<TEdit>("TFTestIF", "edMaxBinCount")->Text             =TestIF_File.iRs232MaxBinCount;             //Steven 20121116 : RS232回傳的最大Bin數
//    rgBaudRate->ItemIndex           =TestIF_File.Rs232_Data.Baud_Rate;
    filerw::ELComboText(EL<TComboBox>("TFTestIF", "cbbBaudRate"), TestIF_File.Rs232_Data.Baud_Rate);          //Steven 20211224 : baudrate改用數字
    EL<TRadioGroup>("TFTestIF", "rgBitLength")->ItemIndex          =TestIF_File.Rs232_Data.Bit_Length;
    EL<TRadioGroup>("TFTestIF", "rgStopBit")->ItemIndex            =TestIF_File.Rs232_Data.Stop_Bit;
    EL<TRadioGroup>("TFTestIF", "rgParity")->ItemIndex             =TestIF_File.Rs232_Data.Parity;
//    cb_NeedSendVSOT->Checked        =(bool)TestIF_File.bRS232NeedSendVSOT;

    EL<TCheckBox>("TFTestIF", "cbForEgistec")->Checked           =TestIF_File.bForEgisTecTest;                               //Steven 20140922 : Arm2當作指紋測試
    EL<TCheckBox>("TFTestIF", "cbSpiroxTesterLotEnd")->Checked   =TestIF_File.bSpiroxTesterLotEnd;                           //JerryYang 20170706 (Steven) JCET通知測試機lot end功能改為by工作檔

    filerw::ELComboIndex(EL<TComboBox>("TFTestIF", "iStartDelayCount"), TestIF_File.iEnStartDelayCount);                            //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay
    EL<TEdit>("TFTestIF", "edtInitialDec1")->Text            =FormatFloat("0.00",TestIF_File.dInitialStartDelayDec[0]);  //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間
    EL<TEdit>("TFTestIF", "edtInitialDecCount1")->Text       =TestIF_File.iStartDelayCount[0];                           //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間 次數
    EL<TEdit>("TFTestIF", "edtInitialDec2")->Text            =FormatFloat("0.00",TestIF_File.dInitialStartDelayDec[1]);  //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間
    EL<TEdit>("TFTestIF", "edtInitialDecCount2")->Text       =TestIF_File.iStartDelayCount[1];                           //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間 次數
    EL<TEdit>("TFTestIF", "edtInitialDec3")->Text            =FormatFloat("0.00",TestIF_File.dInitialStartDelayDec[2]);  //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間
    EL<TEdit>("TFTestIF", "edtInitialDecCount3")->Text       =TestIF_File.iStartDelayCount[2];                           //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間 次數
    EL<TEdit>("TFTestIF", "edtInitialDec4")->Text            =FormatFloat("0.00",TestIF_File.dInitialStartDelayDec[3]);  //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間
    EL<TEdit>("TFTestIF", "edtInitialDecCount4")->Text       =TestIF_File.iStartDelayCount[3];                           //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間 次數

    EL<TCheckBox>("TFTestIF", "cbPurgeAir")->Checked             =TestIF_File.bPurgeAirAfterContract;                        //kevin 20180928 Contrarct time blower air loadboard
    EL<TEdit>("TFTestIF", "edPurgeAir")->Text                =TestIF_File.iPurgeAirContract;                              //kevin 20180928 Contrarct time blower air loadboard
    EL<TCheckBox>("TFTestIF", "cbAutoOnecycleHomStart")->Checked =TestIF_File.bAutoOnecycleHomStart;                         //Isaac 20210821 : 全智要求定時onecycle回home,start
    EL<TEdit>("TFTestIF", "edtAutoOnecycleHomStartTime")->Text=TestIF_File.iAutoOnecycleHomStartTime;                    //Isaac 20210821 : 全智要求定時onecycle回home,start

    EL<TCheckBox>("TFTestIF", "cbUseSocketHeating")->Checked     =TestIF_File.bUseSocketHeating;             //Ztex 2024.09.07 Add Use Socket Heating
    EL<TEdit>("TFTestIF", "edt_UseSocketHeating")->Text      =TestIF_File.iUseSocketHeating;             //Ztex 2024.09.07 Add Use Socket Heating
    TIF_rgInterfaceTypeClick();   // golden cTesterIF.cpp:1141-1141 取代：事件以 (this) 呼叫（參數已拿掉）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1141-1141 -- 事件以 (this) 呼叫（參數已拿掉）
    TIF_rgInterfaceTypeClick(this);
#endif // GATE (S12-C save)

}

// golden cTesterIF.cpp:1157  TFTestIF::rgInterfaceTypeClick(TObject *Sender)
static void TIF_rgInterfaceTypeClick()
{

    if(EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex!=TestIF_File.iTestType &&
       Barcode_Reader(bcTesterIF)==0)                                           // 20140103 wei KYEC Barcode Reader
    {
        EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex=TestIF_File.iTestType;
        return;
    }

    if(bflag==false && EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex!=TestIF_File.iTestType && CosFunction.bTTLCanUse8Site==true)
    {
        EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex=TestIF_File.iTestType;
        bflag=true;
    }

    if(EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex==0)                                           //Isaac 20210309 :TTL RS232兩塊板子，TTL mode
    {
        if((TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                              //Isaac 20210309 :TTL RS232兩塊板子
        {
//            CheckTTLBoardBitMode();                                           //Mark 換工作檔會有問題
        }
        else
        {
            if(TestIF.iTestMode>=_8Site2X4 &&                                   //jou 2010-01-06 start : 4 Site 以上不支援 TTL
               fShow==true &&
               CosFunction.bTTLCanUse8Site==false)                              //Alick 20161011 (Steven) : TTL支援8Site
            {
                filerw::ELMessage("TTL only support less then 4 site, please select GPIB or RS-232.", "4 Site 以上不支援 TTL，請選擇GPIB 或 RS-232");
            }
        }
    }

    TIF_ShowPageControl2(EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex);

}

// golden cTesterIF.cpp:1192  TFTestIF::ShowPageControl2(int Index)
static void TIF_ShowPageControl2(int Index)
{

    TTabSheet *tsTemp[]={EL<TTabSheet>("TFTestIF", "tsDio"), EL<TTabSheet>("TFTestIF", "tsGpib"), EL<TTabSheet>("TFTestIF", "tsRs232"), EL<TTabSheet>("TFTestIF", "tsTCPIP")};                      //Steven 20230221 : 修正TCP/IP Tester設定顯示問題
    for(int i=0; i<4; i++)
        tsTemp[i]->TabVisible=false;
    if(Index<0 || Index>3) { filerw::ELTodo("golden cTesterIF.cpp:1197 ShowPageControl2: Index out of 0..3 (golden indexes tsTemp[] unchecked)"); return; } tsTemp[Index]->TabVisible=true;   // golden cTesterIF.cpp:1197-1197 取代：tsTemp[Index] 沒檢查範圍（rgInterfaceType->ItemIndex＝TestIF_File.iTestType，檔案是負數時 golden 讀陣列外 → AV）→ 範圍外記 todo 並 return
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1197-1197 -- tsTemp[Index] 沒檢查範圍（rgInterfaceType->ItemIndex＝TestIF_File.iTestType，檔案是負數時 golden 讀陣列外 → AV）→ 範圍外記 todo 並 return
    tsTemp[Index]->TabVisible=true;
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TFTestIF", "PageControl2")->ActivePage=tsTemp[Index];
    EL<TPageControl>("TFTestIF", "PageControl2")->ActivePageIndex=Index;
    TIF_cbRs232TypeChange();   // golden cTesterIF.cpp:1200-1200 取代：事件以 (this) 呼叫（參數已拿掉）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1200-1200 -- 事件以 (this) 呼叫（參數已拿掉）
    TIF_cbRs232TypeChange(this);
#endif // GATE (S12-C save)

}

// golden cTesterIF.cpp:1238  TFTestIF::ShowTTLState()
static void TIF_ShowTTLState()
{

    EL<TListBox>("TFTestIF", "lstTTL")->Clear();
    AnsiString sString;
    char cStr[128]="";
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("[Start Signal]");
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Logic                      : "+fDIOFrom->rgStartLogic->Items->Strings[TTLCfg.iSTLogicMode]);
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Chanel                    : "+fDIOFrom->rgStartChannel->Items->Strings[TTLCfg.iStartType]);
    if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        sprintf(cStr,        "Pulse Width           : %d μs.\n", TTLCfg.iSTPluseWidth);
    else
        sprintf(cStr,        "Pulse Width           : %d msec.\n", TTLCfg.iSTPluseWidth);
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add(cStr);

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("");

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("[DUT Signal]");
    if(TTLCfg.iDutType<0)                                                       //Steven 20100105
        TTLCfg.iDutType=0;
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Type                       : "+fDIOFrom->cbSignalType->Items->Strings[TTLCfg.iDutType]);
    if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        sprintf(cStr,        "Before ON             : %d μs.", TTLCfg.iDutBfOnTime);
    else
        sprintf(cStr,        "Before ON             : %d msec.", TTLCfg.iDutBfOnTime);
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add(cStr);
    if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        sprintf(cStr,        "After OFF              : %d μs.", TTLCfg.iDutAfOffTime);
    else
        sprintf(cStr,        "After OFF              : %d msec.", TTLCfg.iDutAfOffTime);
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add(cStr);

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("");

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("[END Signal]");
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Logic                      : "+fDIOFrom->rgBinLogic->Items->Strings[TTLCfg.iCateLogicMode]);

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("");

    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Channel & Bit Length  : "+fDIOFrom->rgBinBitLength->Items->Strings[TTLCfg.iCateBitLength]);
    EL<TListBox>("TFTestIF", "lstTTL")->Items->Add("Data Type                    : "+fDIOFrom->rgBinDataType->Items->Strings[TTLCfg.iCateDataType]);

    if(EL<TRadioGroup>("TFTestIF", "rgInterfaceType")->ItemIndex==0)                                           //Steven 20100812 : 要在TTL模式下才檢查     //Isaac 20210309 :TTL RS232兩塊板子，TTL mode
    {                                                                           //jou 2010-01-06 start : TTL 8bit & 10bit 不能測試超過 2個Site
        if(TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3)                                //Isaac 20210309 :TTL RS232兩塊板子
        {
            CheckTTLBoardBitMode();
        }
        else
        {
            if(TTLCfg.iCateBitLength==_8Bit    || TTLCfg.iCateBitLength==_10Bit ||
               TTLCfg.iCateBitLength==_10BitPE || TTLCfg.iCateBitLength==_10BitPO )
            {
                if(TestIF.iTestMode >= QualSite1X4)
                {
                    filerw::ELMessage("TTL 8bit & 10bit only support less than 2 site!", "TTL 2 Site 以上不支援 8bit & 10bit 模式");
                }
            }
        }
    }

}

// golden cTesterIF.cpp:1299  TFTestIF::cbDIOTypeChange(TObject *Sender)
static void TIF_cbDIOTypeChange()
{

    FileRW_TTLCfg_TFTestIF_cbDIOTypeChange();   // golden cTesterIF.cpp:1301-1303 取代：fDIOFrom->GetDIOFileName()＋LoadData(S)：golden TfDIOFrom 的 C 形狀在 FileRW/TTLCfg.cpp（FileRW_TTLCfg_TFTestIF_cbDIOTypeChange，逐行同 golden :1301-1303，讀 cbDIOType 替身＝同一個 EL<TComboBox>("TFTestIF","cbDIOType")）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1301-1303 -- fDIOFrom->GetDIOFileName()＋LoadData(S)：golden TfDIOFrom 的 C 形狀在 FileRW/TTLCfg.cpp（FileRW_TTLCfg_TFTestIF_cbDIOTypeChange，逐行同 golden :1301-1303，讀 cbDIOType 替身＝同一個 EL<TComboBox>("TFTestIF","cbDIOType")）
    AnsiString S;
    S=fDIOFrom->GetDIOFileName();                                               //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    fDIOFrom->LoadData(S);
#endif // GATE (S12-C save)
    TIF_ShowTTLState();

}

// golden cTesterIF.cpp:1307  TFTestIF::AntiSignalCBoxClick(TObject *Sender)
static void TIF_AntiSignalCBoxClick()
{

    TestIF.bAntiSignal = EL<TCheckBox>("TFTestIF", "AntiSignalCBox")->Checked;

}

// golden cTesterIF.cpp:1312  TFTestIF::cbRs232TypeChange(TObject *Sender)
static void TIF_cbRs232TypeChange()
{

    if(EL<TComboBox>("TFTestIF", "cbRs232Type")->ItemIndex==0)
        EL<TGroupBox>("TFTestIF", "gbRs232BinCount")->Visible=false;
    else
        EL<TGroupBox>("TFTestIF", "gbRs232BinCount")->Visible=true;

}

// golden cTesterIF.cpp:1320  TFTestIF::spbSaveClick(TObject *Sender)
static void TIF_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cTesterIF.cpp:1326-1326 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關；沒寫檔 → PageDesc::reload（golden FormClose 的 ReadTestIFFile）還原替身
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1326-1326 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關；沒寫檔 → PageDesc::reload（golden FormClose 的 ReadTestIFFile）還原替身
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir=DataPath+S;

    TIF_SaveSetupFile(szDir, S);                                                    //kevin 20150105  Start 另存 jobfile
    #ifdef ASE_KaohSiung
       fBuilder->bSaveAsJobFile(S,"JOBFILE");
    #endif

    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }
    TIF_ReadTestIFFile();
    EL<TSpeedButton>("TFTestIF", "spbSave")->Down=false;

    if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden cTesterIF.cpp:1348-1371 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");   // golden cTesterIF.cpp:1348-1371 取代：Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／fYieldMonitoring->SearchRecipeParameter、本表單 SearchRecipeParameter 的 TWinControl 走訪）移植樹沒有（同 A 形狀 tools/formbridge/TFTestIF.py 的 block）
#if 0 // GATE (S12-C save) golden cTesterIF.cpp:1348-1371 -- Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／fYieldMonitoring->SearchRecipeParameter、本表單 SearchRecipeParameter 的 TWinControl 走訪）移植樹沒有（同 A 形狀 tools/formbridge/TFTestIF.py 的 block）
    if(CosFunction.bRecipeParameterDefault)                                     //Sam 20201209 : Default Recipe ChangeLog
    {
        fRPDefault->RP_Name->Clear()    ;
        fRPDefault->RP_Default->Clear() ;
        fRPDefault->RP_Value->Clear()   ;

        if(CosFunction.bRecipeParameterDefaultLog)
        {
            fSpeed->SearchRecipeParameter(fSpeed->tsAllSpeed,-1);               //Isaac 20171227 (Steven) : AutoSpeed RPD一鍵復原
            fSpeed->SearchRecipeParameter(fSpeed->tsIndex,0);
            fSpeed->SearchRecipeParameter(fSpeed->tsInArm,1);
            fSpeed->SearchRecipeParameter(fSpeed->tsOutArm,2);
            fSpeed->SearchRecipeParameter(fSpeed->tsTrayArm,3);
            fSpeed->SearchRecipeParameter(fSpeed->tsShuttle,4);
            fCleaning->SearchRecipeParameter(fCleaning,0);
        }
        SearchRecipeParameter(FTestIF,0);
        if(CosFunction.bRecipeParameterDefaultLog)
        {
            fYieldMonitoring->SearchRecipeParameter(fYieldMonitoring,0);
        }

        fRPDefault->CompareRPDefaultAndValue(fRPDefault->RP_Name, fRPDefault->RP_Default, fRPDefault->RP_Value);
    }
#endif // GATE (S12-C save)

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數    //Isaac 20210309 :TTL RS232兩塊板子，要更新TTL的資料

}

