// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 IniConfig 的讀寫檔（HTEditList：elConfig、cbLastSet、elConfig_byRecipe → config.ini、LastSet.ini、configByRecipe.ini）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cConfiguration.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 1427 個（header 宣告 2235 個）。
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
#include "atester_shims.h"
#include "mycylin.h"
#include "PowerSavingMode.h"
#include "forms/fLotInfo.h"
#include "cinitial.h"
#include "forms/fSCKART.h"
#include "forms/fOffSet.h"
#include "csystem.h"
#include "forms/fSetup.h"
#include "canary_support.h"
#include "mymessbox_shim.h"
#include "forms/fTesterIF.h"
#include "forms/fPassword.h"
#include "forms/fCleaning.h"
#include "MessageDef.h"
#include "forms/fAOI.h"
#include "forms/fBinSel.h"
#include "forms/fShowBinSelect.h"
#include "FileRW/_EditPage.h"   // AI(W906-FRW-S157) 20260927 [W906]：filerw::PageEvent（檔尾 kIC_Events）

AnsiString CheckFile(AnsiString szDir, AnsiString str);   // cAuthority.h:83（golden cAuthority.h）
extern bool authConfig[5];    // cAuthority.cpp:122（golden cAuthority.h:11）
extern bool authConf[14];     // cAuthority.h:60（golden cAuthority.h:12）
void GetConfAuth();           // cAuthority.cpp:472（golden cAuthority.h:27）
void SetTestRunMode();        // RunStartMode.cpp:938（golden cConfiguration.cpp:89 extern；Jimmy 0dcc9c2c 已翻）  AI(W906-FRW-S66) 20260926
bool FileRW_ArmSuckHasIC(int which);   // FileRW/_KitSuck.cpp（0＝InArmSuck；golden cbA09Click :6460／:6472）  AI(W906-EVB4) 20260928
extern bool bLockByServer;          // golden cConfiguration.cpp:6256（定義 golden ckernel.cpp:52；移植樹 Automation/automation.cpp:55）  AI(W906-EVB4) 20260928
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h 空殼虛擬函式（只包下一行的 include）
#endif
#include "Motor/mymotor.h"   // MOT[]（golden cbA09Click :6470-6471）  AI(W906-EVB4) 20260928
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// golden cConfiguration.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden cConfiguration.h: TfConfiguration::fShow
static void IC_QwertyKey(TEdit* EditPtr, int iFunction, int iDP=0, bool bCheckRange=false, double min=0, double max=0) { (void)iDP; if(EditPtr==NULL) return; if(iFunction&N_PORT) { bCheckRange=true; if(min<0 || max<=0) { min=0; max=65535; } } if(iFunction&N_INTEGER || iFunction&N_DOUBLE) { double d=atof(EditPtr->Text.c_str()); if(bCheckRange) EditPtr->Text=AnsiString(CheckRange(d, min, max)); } }   // golden myQwertyKeyBoard.cpp:248-257／:285-301（AI(W906-FRW-Q19) 20260927）

// golden cConfiguration.cpp 檔案層級全域（本表單私用，照抄成 static）
static AnsiString aEP60OldData="";                                                     //Ifor 20170807 (wei) add EP Old Data   // golden cConfiguration.cpp:69
static AnsiString aEP40OldData="";   // golden cConfiguration.cpp:70
static AnsiString aEP30OldData="";   // golden cConfiguration.cpp:71
static AnsiString aEP60OldData_NS="";   // golden cConfiguration.cpp:72
static AnsiString aEP40OldData_NS="";   // golden cConfiguration.cpp:73
static AnsiString aEP30OldData_NS="";   // golden cConfiguration.cpp:74
static bool bEP60DataChange=false;   // golden cConfiguration.cpp:75
static bool bEP40DataChange=false;   // golden cConfiguration.cpp:76
static bool bEP30DataChange=false;   // golden cConfiguration.cpp:77
static bool bEP60DataChange_NS=false;                                                  //Ifor 20170807 (wei) add EP Data Change Flag   // golden cConfiguration.cpp:78
static bool bEP40DataChange_NS=false;   // golden cConfiguration.cpp:79
static bool bEP30DataChange_NS=false;   // golden cConfiguration.cpp:80
static int old60data=0;   // golden cConfiguration.cpp:81
static int old40data=0;   // golden cConfiguration.cpp:82
static int old30data=0;   // golden cConfiguration.cpp:83
static int old60data_NS=0;   // golden cConfiguration.cpp:84
static int old40data_NS=0;   // golden cConfiguration.cpp:85
static int old30data_NS=0;   // golden cConfiguration.cpp:86
static bool bSave=false;   // golden cConfiguration.cpp:88
static bool bM01Enter=false;                                                           //ChungHung 20131009 add for SCK   // golden cConfiguration.cpp:65
static bool bStopChange=false;   // golden cConfiguration.cpp:60

using filerw::EL;

// 前置宣告
static void IC_ReadLockByFile();
static void IC_InitConfigEdtList_ItemA();
static void IC_InitConfigEdtList_ItemB();
static void IC_InitConfigEdtList_ItemC();
static void IC_InitConfigEdtList_ItemD();
static void IC_InitConfigEdtList_ItemE();
static void IC_InitConfigEdtList_ItemF();
static void IC_InitConfigEdtList_ItemG();
static void IC_InitConfigEdtList_ItemI();
static void IC_InitConfigEdtList_ItemL();
static void IC_InitConfigEdtList_ItemM();
static void IC_InitConfigEdtList_ItemN();
static void IC_InitConfigEdtList_ItemO();
static void IC_InitConfigEdtList_ItemP();
static void IC_InitConfigEdtList();
static void IC_ChangeCBListProperty();
static void IC_FormClose();
static void IC_CheckConfigurationBeforeSave();
static void IC_SaveConfiguration();
static void IC_LoadConfiguration();
static void IC_SetA73();
static bool IC_EnableRMSFunc();
static void IC_WriteContactData();
static void IC_FormShow();
static void IC_tbD25_Index30mmChange();
static void IC_tbD25_Index30mm_NSChange();
static void IC_tbD25_Index40mmChange();
static void IC_tbD25_Index40mm_NSChange();
static void IC_tbD25_Index60mmChange();
static void IC_tbD25_Index60mm_NSChange();
static void IC_tbD60_Index56mmChange();
static void IC_tbD60_Index56mm_NSChange();
static void IC_edOCRTrayLotChange(TObject *Sender);
static void IC_udD46Click();
static void IC_cbE30Click();
static void IC_cbE39Click();
static void IC_cbD36Click();
static void IC_Timer1Timer();
static void IC_btD47Click();
static void IC_btnRecordJamRateByTimeClearClick();
static void IC_btResumeClick();
static void IC_cbA09Click();

// golden cConfiguration.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void IC_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode")->Items;
      s->Add("Advan type1");
      s->Add("256 Bin");
      s->Add("16 Bin");
      s->Add("32 Bin");
      s->Add("SPEA Type");
      s->Add("16 Bin GS");
      s->Add("32 Bin GS");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbA10_6_HANA_ARTMode")->Items;
      s->Add("General");
      s->Add("SMILL");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "cbN31_1")->Items;
      s->Add("Disable");
      s->Add("by FTP");
      s->Add("by Net Drive");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbO06TimePeriod")->Items;
      s->Add("10");
      s->Add("30");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbbD47")->Items;
      s->Add("Type 1 Socket");
      s->Add("Type 2 Socket & Shuttle");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbbI41")->Items;
      s->Add("0");
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
      s->Add("16");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbbN06_Mode")->Items;
      s->Add("ASCII - Sends data as ASCII text");
      s->Add("IMAGE - raw binary data in 8-bit bytes");
      s->Add("BYTE - raw binary data using variable-length bytes");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "cbbO15_1")->Items;
      s->Add("Per hour");
      s->Add("Every 2 hour");
      s->Add("Every 4 hour");
      s->Add("Every 6 hour");
      s->Add("Every 8 hour");
      s->Add("Every 12 hour");
      s->Add("Per day");
      s->Add("By Lot");
      s->Add("Per month");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coD03")->Items;
      s->Add("[1]   70");
      s->Add("[2]   80");
      s->Add("[3]   90");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coD22")->Items;
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
      s->Add("10");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coD22_3")->Items;
      s->Add("0x42");
      s->Add("0x43");
      s->Add("0xC1");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coD26")->Items;
      s->Add("Fix Value");
      s->Add("Percentage");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coD41")->Items;
      s->Add("Inside socket");
      s->Add("Above Socket");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coI20")->Items;
      s->Add("0");
      s->Add("16");
      s->Add("E");
      s->Add("Err");
      s->Add("Error");
      s->Add("Accroding to Tester");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coI25")->Items;
      s->Add("With _    (Ex. DUAL_1X2_1_2_)");
      s->Add("Without _ (Ex. DUAL1X2_1_2_)");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coN10DataType")->Items;
      s->Add("yyyy_年");
      s->Add("yyyymm_年月");
      s->Add("yyyymmdd_年月日");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coO19_3")->Items;
      s->Add("Sun.");
      s->Add("Mon.");
      s->Add("Tue.");
      s->Add("Wed.");
      s->Add("Thu.");
      s->Add("Fri.");
      s->Add("Sat.");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coO19_5")->Items;
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
      s->Add("16");
      s->Add("17");
      s->Add("18");
      s->Add("19");
      s->Add("20");
      s->Add("21");
      s->Add("22");
      s->Add("23");
      s->Add("24");
      s->Add("25");
      s->Add("26");
      s->Add("27");
      s->Add("28");
      s->Add("29");
      s->Add("30");
    }
    { TStringList* s = EL<TComboBox>("TfConfiguration", "coP22")->Items;
      s->Add("Auto1");
      s->Add("Auto2");
      s->Add("Auto3");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->Items;
      s->Add("Operator");
      s->Add("Engineer");
      s->Add("Supervisor");
      s->Add("HonPrec");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->Items;
      s->Add("Operator");
      s->Add("Engineer");
      s->Add("Supervisor");
      s->Add("HonPrec");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgC05")->Items;
      s->Add("No message");
      s->Add("Show comfirm message");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgD17")->Items;
      s->Add("Normal");
      s->Add("Get hardware height with Full EP in high calibration.");
      s->Add("Get contact height without EP in high calibration.");
      s->Add("Check EP have leakage or not");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgD69")->Items;
      s->Add("Enable all index check");
      s->Add("Disable index check after auto clean");
      s->Add("Disable both index check when auto clean");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgD71")->Items;
      s->Add("Lot Strart/Lot End");
      s->Add("After index alarm");
      s->Add("Disable all index check");
      s->Add("Normal(Enable all index check)");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgE50")->Items;
      s->Add("RETRY/SKIP");
      s->Add("RETRY");
      s->Add("SKIP");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgF07")->Items;
      s->Add("1. Detect have IC");
      s->Add("2. Detect double device");
      s->Add("3 .Use Y latch sensor check floating");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgF26")->Items;
      s->Add("Skip/ Retry");
      s->Add("Skip");
      s->Add("Retry");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgI22")->Items;
      s->Add("SKIP");
      s->Add("RETRY");
      s->Add("RETRY && SKIP");
      s->Add("HOME");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgI38")->Items;
      s->Add("Settemp +XX.X");
      s->Add("+XX.X");
      s->Add("XX");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgI46")->Items;
      s->Add("None");
      s->Add("Alarm with Skip then set IC in socket to error bin ");
      s->Add("Alarm with Retey to resend SOT.");
      s->Add("Alarm with Skip and Retry.");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgL10")->Items;
      s->Add("5  Sec");
      s->Add("15 Sec");
      s->Add("30 Sec");
      s->Add("1  Min");
      s->Add("5  Min");
      s->Add("10 Min");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN09_4")->Items;
      s->Add("FTP");
      s->Add("Net Drive");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN10_3_1")->Items;
      s->Add("00:00~24:00");
      s->Add("08:00~20:00");
      s->Add("Per Hour");
      s->Add("Specified Time");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN10_4")->Items;
      s->Add("FTP");
      s->Add("Net Drive");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN14_20_1")->Items;
      s->Add("30 min");
      s->Add("60 min");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN23_1_2DSorting")->Items;
      s->Add("FTP");
      s->Add("Net Drive");
      s->Add("SECS/GEM");
      s->Add("Manual Select");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN32")->Items;
      s->Add("FTP");
      s->Add("Net Drive");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgN35_1")->Items;
      s->Add("30 min");
      s->Add("60 min");
      s->Add("90 min");
      s->Add("120min");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgP58RunModeAfterFT")->Items;
      s->Add("FT");
      s->Add("RT");
    }
    { TStringList* s = EL<TRadioGroup>("TfConfiguration", "rgUpDlMethod")->Items;
      s->Add("Network");
      s->Add("FTP");
    }
}

// golden cConfiguration.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kIC_ParentOf[][2] = {
    {"EdF12", "pal_F10"},
    {"G18", "pal_G"},
    {"GroupBox1", "grpN25"},
    {"GroupBox10", "pnlE31_2"},
    {"GroupBox11", "pnlE31_2"},
    {"GroupBox12", "pnlE31_2"},
    {"GroupBox13", "pnlE31_2"},
    {"GroupBox14", "pnlE31_2"},
    {"GroupBox15", "pnlE31_2"},
    {"GroupBox18", "pnlE31_2"},
    {"GroupBox19", "pnlE31_2"},
    {"GroupBox2", "pal_A1"},
    {"GroupBox20", "pnlE31_2"},
    {"GroupBox21", "pnlE31_2"},
    {"GroupBox22", "palE32_Hot"},
    {"GroupBox23", "palE32_Hot"},
    {"GroupBox24", "palE32_Hot"},
    {"GroupBox25", "palE32_Hot"},
    {"GroupBox29", "pnlE32_3"},
    {"GroupBox3", "tsN35"},
    {"GroupBox30", "pnlE32_3"},
    {"GroupBox31", "pnlE32_3"},
    {"GroupBox32", "pnlE32_3"},
    {"GroupBox33", "pnlE32_3"},
    {"GroupBox34", "pnlE32_3"},
    {"GroupBox35", "pnlE32_3"},
    {"GroupBox36", "pnlE32_3"},
    {"GroupBox37", "pnlE32_3"},
    {"GroupBox38", "pnlE32_3"},
    {"GroupBox39", "pnlE32_3"},
    {"GroupBox40", "pnlE32_3"},
    {"GroupBox41", "palE32_Cold"},
    {"GroupBox42", "palE32_Cold"},
    {"GroupBox43", "palE32_Cold"},
    {"GroupBox44", "palE32_Cold"},
    {"GroupBox5", "grpN22"},
    {"GroupBox6", "tsN40"},
    {"GroupBox7", "tsN40"},
    {"GroupBox8", "pnlE31_2"},
    {"GroupBox9", "pnlE31_2"},
    {"PageControl2", "grpC21"},
    {"Panel1", "GroupBox3"},
    {"Panel2", "tsTempComm"},
    {"Panel3", "tsN14_5"},
    {"Panel4", "tsN14_5"},
    {"Panel5", "tsN14_23"},
    {"Panel6", "tsN14_23"},
    {"Panel7", "GroupBox5"},
    {"TabSheet2", "pcF00"},
    {"TabSheet3", "PageControl2"},
    {"TabSheet4", "PageControl2"},
    {"TabSheet5", "PageControl2"},
    {"bgC05_1", "gbC05"},
    {"btD47", "grpD47"},
    {"btN06_TesterList", "gbN06_FTP"},
    {"btN06_TesterMap", "gbN06_FTP"},
    {"btN06_UpdateTesterList", "gbN06_FTP"},
    {"btResume", "gbN04"},
    {"btnA77RMSSetting", "tsA06"},
    {"btnAdd1000", "pnlSoftSpeed"},
    {"btnAdd10000", "pnlSoftSpeed"},
    {"btnDec1000", "pnlSoftSpeed"},
    {"btnMesSystem", "gbN05"},
    {"btnOpenEP", "pal_D3"},
    {"btnSave", "pal_G"},
    {"btnSetTo1000", "pnlSoftSpeed"},
    {"btnSetToTech", "pnlSoftSpeed"},
    {"btnUploadAll", "gbN05"},
    {"cb14_23", "Panel5"},
    {"cb14_24", "Panel6"},
    {"cbA01", "GroupBox2"},
    {"cbA01_1", "GroupBox2"},
    {"cbA01_2", "GroupBox2"},
    {"cbA02", "pal_A1"},
    {"cbA03", "pal_A1"},
    {"cbA04", "pal_A1"},
    {"cbA05", "pal_A1"},
    {"cbA08", "pal_A1"},
    {"cbA09", "pal_A1"},
    {"cbA10", "pal_A1"},
    {"cbA10_3", "gbA10"},
    {"cbA10_3_ARTTestMode", "gbA10"},
    {"cbA10_5", "gbA10"},
    {"cbA10_6", "gbA10"},
    {"cbA10_6_HANA_ARTMode", "gbA10"},
    {"cbA10_7", "gbA10"},
    {"cbA11", "pal_A2"},
    {"cbA12", "pnlA12"},
    {"cbA14", "pal_A2"},
    {"cbA15", "pal_A2"},
    {"cbA15_1", "pal_A2"},
    {"cbA17_1", "grpA17"},
    {"cbA17_2", "grpA17"},
    {"cbA19", "pal_A2"},
    {"cbA20_1", "grpA20"},
    {"cbA20_2", "grpA20"},
    {"cbA20_3", "grpA20"},
    {"cbA20_4", "grpA20"},
    {"cbA20_5", "grpA20"},
    {"cbA21", "pal_A3"},
    {"cbA22_1", "gbA22"},
    {"cbA23", "pal_A3"},
    {"cbA24", "pal_A3"},
    {"cbA26", "pal_A3"},
    {"cbA27", "pal_A3"},
    {"cbA27_1", "pal_A3"},
    {"cbA29", "pal_A3"},
    {"cbA30", "pal_A3"},
    {"cbA31", "gbA31"},
    {"cbA31_1", "gbA31_CleanTiming"},
    {"cbA32", "grpA32"},
    {"cbA32_01", "grpA32_1"},
    {"cbA32_02", "grpA32_1"},
    {"cbA32_03", "grpA32_1"},
    {"cbA32_04", "grpA32_1"},
    {"cbA32_05", "grpA32_1"},
    {"cbA32_06", "grpA32_1"},
    {"cbA32_07", "grpA32_1"},
    {"cbA32_08", "grpA32_1"},
    {"cbA32_09", "grpA32_1"},
    {"cbA32_10", "grpA32_1"},
    {"cbA32_2", "grpA32"},
    {"cbA32_3", "grpA32"},
    {"cbA33", "pal_A4"},
    {"cbA35", "pal_A4"},
    {"cbA36", "pal_A4"},
    {"cbA37", "pal_A4"},
    {"cbA38", "pal_A4"},
    {"cbA39", "pal_A4"},
    {"cbA40", "pal_A4"},
    {"cbA51", "tsA_05"},
    {"cbA55", "grpA55"},
    {"cbA56_1", "gbA56"},
    {"cbA57_1", "gbA57"},
    {"cbA57_2", "gbA57"},
    {"cbA57_3", "gbA57"},
    {"cbA58", "tsA_05"},
    {"cbA60_1", "grpA60"},
    {"cbA61", "tsA_05"},
    {"cbA62", "tsA_05"},
    {"cbA65", "tsA_05"},
    {"cbA66", "tsA_05"},
    {"cbA67", "tsA_05"},
    {"cbA68", "tsA_05"},
    {"cbA69", "tsA_05"},
    {"cbA71", "gbA71"},
    {"cbA72", "tsA06"},
    {"cbA73", "tsA06"},
    {"cbA74", "tsA06"},
    {"cbA75", "tsA06"},
    {"cbA76", "tsA06"},
    {"cbA77", "tsA06"},
    {"cbA78", "tsA06"},
    {"cbA81WaitSECS", "tsA06"},
    {"cbB01", "grpB01"},
    {"cbB02", "grpB02"},
    {"cbB03", "grpB03"},
    {"cbB05", "grpB05"},
    {"cbB11Enable", "gbB11"},
    {"cbB12Enable", "gbB12"},
    {"cbC01", "pal_C1"},
    {"cbC02", "pal_C1"},
    {"cbC03", "pal_C1"},
    {"cbC04", "pal_C1"},
    {"cbC05_ATC", "bgC05_1"},
    {"cbC05_Motor", "bgC05_1"},
    {"cbC05_Temp", "bgC05_1"},
    {"cbC05_Vacuum", "bgC05_1"},
    {"cbC06_01", "gbC06"},
    {"cbC06_02", "gbC06"},
    {"cbC06_03", "gbC06"},
    {"cbC06_04", "gbC06"},
    {"cbC06_05", "gbC06"},
    {"cbC06_06", "gbC06"},
    {"cbC06_07", "gbC06"},
    {"cbC06_08", "gbC06"},
    {"cbC06_09", "gbC06"},
    {"cbC06_10", "gbC06"},
    {"cbC06_11", "gbC06"},
    {"cbC06_12", "gbC06"},
    {"cbC07", "pal_C1"},
    {"cbC08", "pal_C1"},
    {"cbC08_1", "pal_C1"},
    {"cbC09", "pal_C1"},
    {"cbC10", "pal_C1"},
    {"cbC11", "pal_C2"},
    {"cbC12", "pal_C2"},
    {"cbC13", "pal_C2"},
    {"cbC16", "pal_C2"},
    {"cbC17", "grpC17"},
    {"cbC17_1", "grpC17"},
    {"cbC24", "tsC21"},
    {"cbD01", "pal_D1"},
    {"cbD01_1", "pal_D1"},
    {"cbD02", "pal_D1"},
    {"cbD04", "pal_D1"},
    {"cbD05", "pal_D1"},
    {"cbD05_1", "pal_D1"},
    {"cbD06", "pal_D1"},
    {"cbD10", "pal_D2"},
    {"cbD11", "pal_D2"},
    {"cbD12", "pal_D2"},
    {"cbD14", "pal_D2"},
    {"cbD15", "pal_D2"},
    {"cbD16", "pal_D2"},
    {"cbD21", "pal_D3"},
    {"cbD22_1", "grpD22"},
    {"cbD22_2", "grpD22"},
    {"cbD22_3", "grpD22"},
    {"cbD22_3_", "grpD22"},
    {"cbD22_4", "grpD22"},
    {"cbD23", "pal_D3"},
    {"cbD24", "pal_D3"},
    {"cbD26", "pal_D3"},
    {"cbD26_1", "pal_D3"},
    {"cbD26_2", "pal_D3"},
    {"cbD26_3", "pal_D3"},
    {"cbD27", "pal_D3"},
    {"cbD28", "pal_D3"},
    {"cbD29", "pal_D3"},
    {"cbD30", "pal_D4"},
    {"cbD31", "pal_D4"},
    {"cbD32", "pal_D4"},
    {"cbD33", "pal_D4"},
    {"cbD34", "pal_D4"},
    {"cbD35", "pal_D4"},
    {"cbD36", "pal_D4"},
    {"cbD36_1", "pal_D4"},
    {"cbD36_2", "pal_D4"},
    {"cbD37", "pal_D4"},
    {"cbD38", "pal_D4"},
    {"cbD40", "pal_D5"},
    {"cbD41", "pnlD41"},
    {"cbD42", "pal_D5"},
    {"cbD43", "pal_D5"},
    {"cbD43_1", "pal_D5"},
    {"cbD43_2", "pal_D5"},
    {"cbD44", "pal_D5"},
    {"cbD45", "pal_D5"},
    {"cbD47", "grpD47"},
    {"cbD48", "pal_D5"},
    {"cbD49", "pal_D5"},
    {"cbD50", "pal_D6"},
    {"cbD51", "pal_D6"},
    {"cbD52", "pal_D6"},
    {"cbD53", "pal_D6"},
    {"cbD54", "pal_D6"},
    {"cbD55", "pal_D6"},
    {"cbD56", "pal_D6"},
    {"cbD57", "pal_D6"},
    {"cbD58", "pal_D6"},
    {"cbD59", "pal_D6"},
    {"cbD61", "pal_D7"},
    {"cbD62", "pal_D7"},
    {"cbD63", "pnlD63"},
    {"cbD63_1", "pal_D7"},
    {"cbD64", "pal_D7"},
    {"cbD65", "pal_D7"},
    {"cbD66", "pal_D7"},
    {"cbD67", "pal_D7"},
    {"cbD70", "pal_D8"},
    {"cbD72", "pal_D8"},
    {"cbD73", "pal_D8"},
    {"cbD74", "pal_D8"},
    {"cbD75", "pal_D8"},
    {"cbD78", "pal_D8"},
    {"cbD79", "pal_D8"},
    {"cbD80", "tsD_80"},
    {"cbD81", "tsD_80"},
    {"cbD82", "tsD_80"},
    {"cbD83", "tsD_80"},
    {"cbE30", "pnlE30"},
    {"cbE30_1", "pnlE30_Hot"},
    {"cbE30_2", "pnlE30_Cold"},
    {"cbE31", "pnlE31"},
    {"cbE31_1", "pnlE31_Hot"},
    {"cbE31_2", "pnlE31_Cold"},
    {"cbE32", "pnlE32"},
    {"cbE32_1", "pnlE32_Hot"},
    {"cbE32_2", "pnlE32_Cold"},
    {"cbE33", "pal_E2"},
    {"cbE34", "pal_E2"},
    {"cbE35", "pal_E2"},
    {"cbE38", "pal_E2"},
    {"cbE39", "pal_E2"},
    {"cbE39_1", "pal_E2"},
    {"cbE40", "pal_E2"},
    {"cbE41", "pal_E2"},
    {"cbE42", "pal_E2"},
    {"cbE43", "pal_E2"},
    {"cbE44", "pal_E2"},
    {"cbE45", "pal_E2"},
    {"cbE46", "pal_E2"},
    {"cbE47", "pal_E2"},
    {"cbE48", "pal_E2"},
    {"cbE49", "pal_E2"},
    {"cbE51", "pal_E5"},
    {"cbE53", "pal_E5"},
    {"cbE54", "pal_E5"},
    {"cbE55", "pal_E5"},
    {"cbE56", "pal_E5"},
    {"cbE57", "pal_E5"},
    {"cbE58", "pal_E5"},
    {"cbE59", "pal_E5"},
    {"cbE60", "pal_E5"},
    {"cbE61", "pal_E5"},
    {"cbE62", "pal_E5"},
    {"cbE63", "pal_E5"},
    {"cbE64", "pal_E5"},
    {"cbE68", "pal_E5"},
    {"cbE79", "pal_E80"},
    {"cbE85_Auto1", "gbE85"},
    {"cbE85_Auto3", "gbE85"},
    {"cbE85_Enable", "gbE85"},
    {"cbE86", "pal_E80"},
    {"cbE87", "pal_E80"},
    {"cbE88", "pal_E80"},
    {"cbE89", "pal_E80"},
    {"cbE90", "pal_E80"},
    {"cbF01", "pal_F00"},
    {"cbF03", "pal_F00"},
    {"cbF05", "pal_F00"},
    {"cbF06", "pal_F00"},
    {"cbF09", "pal_F00"},
    {"cbF11", "pal_F10"},
    {"cbF12", "pal_F10"},
    {"cbF14", "pnlF14"},
    {"cbF14_1", "pnlF14"},
    {"cbF15", "pal_F10"},
    {"cbF16", "pal_F10"},
    {"cbF17", "pal_F10"},
    {"cbF18", "pal_F10"},
    {"cbF19", "pal_F10"},
    {"cbF20", "pal_F10"},
    {"cbF20Label2", "pal_F10"},
    {"cbF21", "pal_F20"},
    {"cbF22", "pal_F20"},
    {"cbF23", "pal_F20"},
    {"cbF24", "pal_F20"},
    {"cbF25", "pal_F20"},
    {"cbF28", "pal_F20"},
    {"cbF29", "pal_F20"},
    {"cbF30", "pal_F20"},
    {"cbF31", "grpF31"},
    {"cbF32", "pal_F30"},
    {"cbF33_Check2DHardware", "pal_F30"},
    {"cbF34", "pal_F30"},
    {"cbF35", "pal_F30"},
    {"cbF36", "pal_F30"},
    {"cbF37", "pal_F30"},
    {"cbG01", "pal_G"},
    {"cbG04", "pal_G"},
    {"cbG05", "pal_G"},
    {"cbG06", "pal_G"},
    {"cbG07", "pal_G"},
    {"cbG08", "pal_G"},
    {"cbG09", "pal_G"},
    {"cbG10", "pal_G"},
    {"cbG11", "pal_G"},
    {"cbG12", "pal_G"},
    {"cbG13", "pal_G"},
    {"cbG14", "pal_G"},
    {"cbG15", "pal_G"},
    {"cbG17", "pal_G"},
    {"cbG17_1", "pal_G"},
    {"cbG18_1", "G18"},
    {"cbG18_2", "G18"},
    {"cbG19", "pal_G"},
    {"cbG20", "pal_G"},
    {"cbG21", "pal_G"},
    {"cbG22", "pal_G"},
    {"cbG23", "pal_G"},
    {"cbG24", "pal_G"},
    {"cbI01", "pal_I01"},
    {"cbI02", "pal_I01"},
    {"cbI03", "pal_I01"},
    {"cbI04", "pal_I01"},
    {"cbI05", "pal_I01"},
    {"cbI06", "pal_I01"},
    {"cbI07", "pal_I01"},
    {"cbI08", "pal_I01"},
    {"cbI09", "pal_I01"},
    {"cbI12", "pal_I01"},
    {"cbI13", "pal_I01"},
    {"cbI16", "pal_I01"},
    {"cbI17", "pnlI17"},
    {"cbI18", "pal_I01"},
    {"cbI19", "pal_I01"},
    {"cbI21", "gbI21"},
    {"cbI21_10", "gbI21"},
    {"cbI21_5", "gbI21"},
    {"cbI21_6", "gbI21"},
    {"cbI21_7", "gbI21"},
    {"cbI21_8", "gbI21"},
    {"cbI21_9", "gbI21"},
    {"cbI21_CheckOpen", "gbI21"},
    {"cbI21_SameSoakTime", "gbI21"},
    {"cbI21_SkipSoakTime", "gbI21"},
    {"cbI22", "pal_I20"},
    {"cbI23", "pal_I20"},
    {"cbI24", "pal_I20"},
    {"cbI26", "pal_I20"},
    {"cbI27", "pal_I20"},
    {"cbI28", "pal_I20"},
    {"cbI29", "pal_I20"},
    {"cbI29_3", "pal_I20"},
    {"cbI30", "pal_I20"},
    {"cbI31_1", "grpI31"},
    {"cbI31_2", "grpI31"},
    {"cbI31_3", "grpI31"},
    {"cbI32", "pal_I30"},
    {"cbI33", "pal_I30"},
    {"cbI34", "pal_I30"},
    {"cbI35", "pal_I30"},
    {"cbI36", "pal_I30"},
    {"cbI37_1", "grpI37"},
    {"cbI37_2", "grpI37"},
    {"cbI37_3", "grpI37"},
    {"cbI39", "pal_I30"},
    {"cbI40", "pal_I30"},
    {"cbI41", "grpI41"},
    {"cbI41_1", "grpI41"},
    {"cbI41_2", "grpI41"},
    {"cbI41_3", "grpI41"},
    {"cbI41_4", "grpI41"},
    {"cbI41_5", "grpI41"},
    {"cbI41_6", "grpI41"},
    {"cbI42", "pal_I40"},
    {"cbI43", "pal_I40"},
    {"cbI45", "pal_I40"},
    {"cbI49", "pal_I40"},
    {"cbI50_EnableASM_Trigger", "gbI50"},
    {"cbI50_InitialStart", "gbI50"},
    {"cbI50_OnyCycle", "gbI50"},
    {"cbI50_Pause", "gbI50"},
    {"cbI50_RT", "gbI50"},
    {"cbI50_StartLot", "gbI50"},
    {"cbI50_TrayFeed", "gbI50"},
    {"cbI51", "tsI_50"},
    {"cbI52", "tsI_50"},
    {"cbI53", "tsI_50"},
    {"cbI54_1", "gbI54"},
    {"cbI54_2", "gbI54"},
    {"cbI54_Enable", "gbI54"},
    {"cbL03", "pal_L1"},
    {"cbL07", "pal_L1"},
    {"cbL09", "gbL09"},
    {"cbL10", "pal_L1"},
    {"cbL11_1", "gbL11"},
    {"cbL11_2", "gbL11"},
    {"cbL11_6", "gbL11"},
    {"cbL11_7", "gbL11"},
    {"cbL11_8", "gbL11"},
    {"cbL12", "pal_L2"},
    {"cbL13", "pal_L2"},
    {"cbL15", "pal_L2"},
    {"cbL17", "pal_L2"},
    {"cbL18", "pal_L2"},
    {"cbL19", "pal_L2"},
    {"cbL20", "pal_L2"},
    {"cbL21", "pal_L3"},
    {"cbL22", "pal_L3"},
    {"cbL24", "pal_L3"},
    {"cbL25", "pal_L3"},
    {"cbL28", "pal_L3"},
    {"cbL29", "pal_L3"},
    {"cbL30", "gbL30"},
    {"cbL30_1", "gbL30"},
    {"cbL32_1", "grpL32_1"},
    {"cbL32_2", "grpL32_2"},
    {"cbL33_1", "grpL33"},
    {"cbL33_2", "grpL33"},
    {"cbL34_1", "grpL34"},
    {"cbL34_5", "grpL34"},
    {"cbL35_1", "grpL35"},
    {"cbL37", "tsL_30_2"},
    {"cbL38", "tsL_30_2"},
    {"cbL39_1", "grpL39"},
    {"cbL39_2", "grpL39"},
    {"cbL42", "tsL_30_2"},
    {"cbL43", "tsL_30_2"},
    {"cbL44", "tsL_30_2"},
    {"cbL45", "tsL_30_2"},
    {"cbL46", "tsL_30_2"},
    {"cbL49", "tsL_30_2"},
    {"cbM01", "pal_M"},
    {"cbM01_01", "gbM01"},
    {"cbM01_02", "gbM01"},
    {"cbM01_03", "gbM01"},
    {"cbM01_04", "gbM01"},
    {"cbM01_05", "gbM01"},
    {"cbM01_06", "gbM01"},
    {"cbM01_07", "gbM01"},
    {"cbM01_08", "gbM01"},
    {"cbM01_09", "gbM01"},
    {"cbM01_10", "gbM01"},
    {"cbM01_12", "gbM01"},
    {"cbM01_13", "gbM01"},
    {"cbM01_14", "gbM01"},
    {"cbManuTakeAutoFailTray_FT", "gbUnloadMode"},
    {"cbManuTakeAutoFailTray_RT", "gbUnloadMode"},
    {"cbN05_1", "gbN05_1"},
    {"cbN05_CheckFile", "gbN05"},
    {"cbN05_CheckSetupFile", "gbN05"},
    {"cbN05_EnableRMS", "gbN05"},
    {"cbN05_SCKWebService", "gbN05_WebService"},
    {"cbN06_1", "gbN06_FTP"},
    {"cbN06_EnableFTP", "gbN06_FTP"},
    {"cbN06_UseBarcode", "gbN06_FTP"},
    {"cbN06_UseSystemCall", "gbN06_FTP"},
    {"cbN07_6CompressedFile", "gbN07"},
    {"cbN07_EnableEmployeeCheak", "gbN07"},
    {"cbN07_EnableHostStart", "gbN07"},
    {"cbN07_EnableSecs", "gbN07"},
    {"cbN07_EnableSecsLotCheck", "gbN07"},
    {"cbN07_EnableSecsOneCycle", "gbN07"},
    {"cbN08_1", "gbN08"},
    {"cbN10_1", "grpLogUploadToFTP"},
    {"cbN10_11", "tsN10_11_20"},
    {"cbN10_12", "tsN10_11_20"},
    {"cbN10_2", "grpLogUploadToFTP"},
    {"cbN10_3", "grpLogUploadToFTP"},
    {"cbN11_1", "gbN11"},
    {"cbN13_EnableARMSFunction", "grpASEMNetworkDrive"},
    {"cbN14_1", "tsN14_1"},
    {"cbN14_10", "tsN14_2"},
    {"cbN14_11", "tsN14_2"},
    {"cbN14_12", "tsN14_2"},
    {"cbN14_13", "tsN14_2"},
    {"cbN14_14", "pnlN14_3"},
    {"cbN14_15", "pnlN14_15"},
    {"cbN14_16", "grpN14_16"},
    {"cbN14_18", "pnlN14_18"},
    {"cbN14_19", "pnlN14_19"},
    {"cbN14_2", "tsN14_1"},
    {"cbN14_20", "Panel3"},
    {"cbN14_20_1", "Panel3"},
    {"cbN14_21", "Panel4"},
    {"cbN14_22", "grpN14_22"},
    {"cbN14_3", "grpN14_3"},
    {"cbN14_4", "tsN14_1"},
    {"cbN14_7", "tsN14_2"},
    {"cbN14_8", "tsN14_2"},
    {"cbN14_9", "tsN14_2"},
    {"cbN15_1", "grpN15"},
    {"cbN15_2", "grpN15"},
    {"cbN17_1", "gbN17LotSummary"},
    {"cbN17_3", "gbN17ProdLogUL"},
    {"cbN22", "grpN22"},
    {"cbN22_EveltLog", "grpN22"},
    {"cbN23", "GroupBox5"},
    {"cbN27_1", "grpN27"},
    {"cbN30_1", "gbN30"},
    {"cbN31_1", "gbN31"},
    {"cbN32_1", "gbN32"},
    {"cbN32_2", "gbN32"},
    {"cbN32_3", "gbN32"},
    {"cbN32_4", "gbN32"},
    {"cbN33", "gbN33"},
    {"cbN33_1", "gbN33_1"},
    {"cbN34", "gbN34"},
    {"cbN35", "GroupBox3"},
    {"cbN35_1", "grpN35"},
    {"cbN40_1", "GroupBox6"},
    {"cbN41_1", "GroupBox7"},
    {"cbO01", "pal_O1"},
    {"cbO02", "pal_O1"},
    {"cbO05", "pal_O1"},
    {"cbO06", "gbO06"},
    {"cbO06TimePeriod", "gbO06"},
    {"cbO07", "pal_O1"},
    {"cbO08", "pal_O1"},
    {"cbO09", "pal_O1"},
    {"cbO10_EventLogSaver", "pal_O1"},
    {"cbO11", "pal_O2"},
    {"cbO12_LifeTimeCount", "paLifeTime"},
    {"cbO13_LifeTimeCount", "paLifeTime"},
    {"cbO14_LifeTimeCount", "paLifeTime"},
    {"cbO18", "pal_O2"},
    {"cbO19_1", "grpO16"},
    {"cbO19_2", "grpO16"},
    {"cbO19_4", "grpO16"},
    {"cbO20", "pal_O2"},
    {"cbO20_1", "pal_O2"},
    {"cbO21", "tsO21"},
    {"cbO21_1", "tsO21"},
    {"cbP04", "pal_P0"},
    {"cbP05", "pal_P0"},
    {"cbP05_1", "pal_P0"},
    {"cbP06", "pal_P0"},
    {"cbP07", "pal_P0"},
    {"cbP08", "pal_P0"},
    {"cbP09", "pal_P0"},
    {"cbP10", "pal_P0"},
    {"cbP11", "pal_P1"},
    {"cbP13", "pnlP13"},
    {"cbP14", "pnlP13"},
    {"cbP15", "pal_P1"},
    {"cbP16", "pnlP16"},
    {"cbP17", "pal_P1"},
    {"cbP18", "pal_P1"},
    {"cbP19", "pal_P1"},
    {"cbP20", "pal_P1"},
    {"cbP21", "pal_P2"},
    {"cbP22", "pal_P2"},
    {"cbP24", "pal_P2"},
    {"cbP24_2", "pal_P2"},
    {"cbP24_3", "pal_P2"},
    {"cbP25", "pal_P2"},
    {"cbP26", "gbP26_OCRCheck"},
    {"cbP27", "pal_P2"},
    {"cbP28", "pal_P2"},
    {"cbP29", "pal_P2"},
    {"cbP30", "pal_P2"},
    {"cbP31", "pal_P3"},
    {"cbP32", "pal_P3"},
    {"cbP33", "pal_P3"},
    {"cbP34", "pal_P3"},
    {"cbP35", "pal_P3"},
    {"cbP36", "pal_P3"},
    {"cbP37", "pal_P3"},
    {"cbP38", "pal_P3"},
    {"cbP39", "pal_P3"},
    {"cbP40", "pal_P3"},
    {"cbP41", "pal_P3"},
    {"cbP42", "pal_P3"},
    {"cbP44", "pal_P3"},
    {"cbP45", "pal_P3"},
    {"cbP46", "pal_P3"},
    {"cbP48", "grpP48"},
    {"cbP49", "pal_P3"},
    {"cbP50", "pal_P3"},
    {"cbP51", "tsP50"},
    {"cbP52", "tsP50"},
    {"cbP53", "tsP50"},
    {"cbP54", "tsP50"},
    {"cbP55", "tsP50"},
    {"cbP57", "tsP50"},
    {"cbP59", "tsP50"},
    {"cbP60", "tsP50"},
    {"cbP62", "plP62"},
    {"cbP62_1", "plP62"},
    {"cbP65", "plP65"},
    {"cbP66", "tsP50"},
    {"cbbD47", "grpD47"},
    {"cbbI41", "grpI41"},
    {"cbbN06_Mode", "gbN06_FTP"},
    {"cbbO15_1", "grpO15"},
    {"cbkE78", "pal_E80"},
    {"cdD13", "pal_D2"},
    {"cdE52", "pal_E5"},
    {"chA16", "pal_A2"},
    {"checkbN05_RTC", "groupbN05_RTC"},
    {"chk24", "grpN24"},
    {"chkA09_1", "pal_A1"},
    {"chkA50", "tsA_05"},
    {"chkAutoTrayFeed", "gbUnloadMode"},
    {"chkC14", "pal_C2"},
    {"chkD18", "pal_D2"},
    {"chkE43_1", "pal_E2"},
    {"chkE65", "pal_E5"},
    {"chkE66", "pal_E5"},
    {"chkE67", "pal_E5"},
    {"chkE69", "pal_E5"},
    {"chkE70", "pal_E80"},
    {"chkE71", "pal_E80"},
    {"chkE72", "pal_E80"},
    {"chkE73", "pal_E80"},
    {"chkE74", "pal_E80"},
    {"chkF27", "pal_F20"},
    {"chkG16", "pal_G"},
    {"chkHeater", "grpCommMsg"},
    {"chkI29_1", "pal_I20"},
    {"chkI44", "pal_I40"},
    {"chkN05_TrayFeedClear", "gbN05"},
    {"chkN06_Tester", "gbN06_FTP"},
    {"chkN07_3_2", "gbN07"},
    {"chkN07_6", "gbN07"},
    {"chkN07_7", "gbN07"},
    {"chkN09", "grpN09"},
    {"chkN09_2", "grpN09"},
    {"chkN10_9", "tsN10_11_20"},
    {"chkN10_Passive", "grpN10_FTP"},
    {"chkN12", "gbN12_SocketIdProductDataUpoadToFTP"},
    {"chkN16", "grpN16"},
    {"chkN20", "grpN20"},
    {"chkN21_1", "grpN21"},
    {"chkN23_1", "grpN23_1"},
    {"chkN23_3", "grpN23_3"},
    {"chkN23_4", "rgbN23_2DSorting"},
    {"chkN23_5", "rgbN23_2DSorting"},
    {"chkN25_1", "grpN25_1"},
    {"chkN25_2", "GroupBox1"},
    {"chkN25_3", "grpN25_3"},
    {"chkN25_4", "grpN25_4"},
    {"chkN25_5", "grpN25_5"},
    {"chkN26_1", "gbN26"},
    {"chkN26_2", "gbN26"},
    {"chkN28", "grpN28"},
    {"chkN29", "tsN29"},
    {"chkO06AlarmHistroy", "gbO06"},
    {"chkO06AlarmStatist", "gbO06"},
    {"chkO06Production", "gbO06"},
    {"chkO06TimePeriod", "gbO06"},
    {"chkO06UseNetDrive", "gbO06"},
    {"chkO15_2", "grpO15"},
    {"chkO15_3", "grpO15"},
    {"chkO16", "pal_O2"},
    {"chkO17", "pal_O2"},
    {"chkO22", "tsO21"},
    {"chkO23", "tsO21"},
    {"chkO24", "tsO21"},
    {"chkO25", "tsO21"},
    {"chkP20_1", "pal_P1"},
    {"chkP20_2", "pal_P1"},
    {"chkP21_1", "pal_P2"},
    {"chkP21_2", "pal_P2"},
    {"chkP56", "tsP50"},
    {"coD22", "grpD22"},
    {"coD22_3", "grpD22"},
    {"coD26", "pal_D3"},
    {"coD41", "pnlD41"},
    {"coI20", "pal_I20"},
    {"coI25", "pal_I20"},
    {"coN10DataType", "tsN10_11_20"},
    {"coO19_3", "grpO16"},
    {"coO19_5", "grpO16"},
    {"coP22", "pal_P2"},
    {"dtO06_LastDate", "gbO06"},
    {"dtpN10_3_1_SpecifiedTime", "grpLogUploadToFTP"},
    {"dtpO06NextTime", "gbO06"},
    {"ed14_24", "Panel6"},
    {"edA01", "GroupBox2"},
    {"edA10_2", "palA10_ART"},
    {"edA10_3", "palA10_ART"},
    {"edA10_4", "gbA10"},
    {"edA11", "pal_A2"},
    {"edA12", "pnlA12"},
    {"edA15_ESDReportTime", "pal_A2"},
    {"edA22_2", "gbA22"},
    {"edA22_3", "gbA22"},
    {"edA25_1", "grpA25"},
    {"edA25_2", "grpA25"},
    {"edA32_1", "grpA32"},
    {"edA39", "pal_A4"},
    {"edA56_2", "gbA56"},
    {"edA56_3", "gbA56"},
    {"edA60_1", "grpA60"},
    {"edA60_2", "grpA60"},
    {"edA60_3", "grpA60"},
    {"edA60_4", "grpA60"},
    {"edA60_5", "grpA60"},
    {"edA71", "gbA71"},
    {"edB01", "grpB01"},
    {"edB01_1", "grpB01"},
    {"edB02", "grpB02"},
    {"edB03_1", "grpB03"},
    {"edB03_2", "grpB03"},
    {"edB11PATServerPath", "gbB11"},
    {"edB12Path", "gbB12"},
    {"edB13DownloadPath", "gbB13"},
    {"edB13UploadPath", "gbB13"},
    {"edB14IntervalTime", "gbB14"},
    {"edB14ReportRealTime", "gbB14"},
    {"edC05_ATC", "bgC05_1"},
    {"edC05_Motor", "bgC05_1"},
    {"edC05_Temp", "bgC05_1"},
    {"edC05_Vacuum", "bgC05_1"},
    {"edC09", "pal_C1"},
    {"edD01", "pal_D1"},
    {"edD01DelayTime_Xilinx", "palD01"},
    {"edD01_Xilinx", "palD01"},
    {"edD05", "pal_D1"},
    {"edD06", "pal_D1"},
    {"edD14", "pal_D2"},
    {"edD21_Sec", "pal_D3"},
    {"edD21_mm", "pal_D3"},
    {"edD23", "pal_D3"},
    {"edD25_30mm", "gbD25"},
    {"edD25_40mm", "gbD25"},
    {"edD25_60mm", "gbD25"},
    {"edD26", "pal_D3"},
    {"edD26_3", "pal_D3"},
    {"edD41", "pnlD41"},
    {"edD41_2", "pnlD41"},
    {"edD44", "pal_D5"},
    {"edD46", "pal_D5"},
    {"edD47_3", "grpD47"},
    {"edD47_Count", "grpD47"},
    {"edD47_Time", "grpD47"},
    {"edD53", "pal_D6"},
    {"edD54", "pal_D6"},
    {"edD60_56mm", "gbD60"},
    {"edD62", "pal_D7"},
    {"edD63", "pnlD63"},
    {"edD68", "pal_D7"},
    {"edE30_1_HP1X", "gbE30_1_HP1"},
    {"edE30_1_HP1Y", "gbE30_1_HP1"},
    {"edE30_1_HP2X", "gbE30_1_HP2"},
    {"edE30_1_HP2Y", "gbE30_1_HP2"},
    {"edE30_1_LodX", "gbE30_1_loader"},
    {"edE30_1_LodY", "gbE30_1_loader"},
    {"edE30_2_HP1X", "gbE30_2_HP1"},
    {"edE30_2_HP1Y", "gbE30_2_HP1"},
    {"edE30_2_HP2X", "gbE30_2_HP2"},
    {"edE30_2_HP2Y", "gbE30_2_HP2"},
    {"edE30_2_LodX", "gbE30_2_loader"},
    {"edE30_2_LodY", "gbE30_2_loader"},
    {"edE30_HP1X", "gbE30_HP1"},
    {"edE30_HP1Y", "gbE30_HP1"},
    {"edE30_HP2X", "gbE30_HP2"},
    {"edE30_HP2Y", "gbE30_HP2"},
    {"edE30_LodX", "gbE30_loader"},
    {"edE30_LodY", "gbE30_loader"},
    {"edE31_1_Au1X", "GroupBox8"},
    {"edE31_1_Au1Y", "GroupBox8"},
    {"edE31_1_Au2X", "GroupBox9"},
    {"edE31_1_Au2Y", "GroupBox9"},
    {"edE31_1_Au3X", "GroupBox10"},
    {"edE31_1_Au3Y", "GroupBox10"},
    {"edE31_1_Au4X", "GroupBox14"},
    {"edE31_1_Au4Y", "GroupBox14"},
    {"edE31_1_Au5X", "GroupBox15"},
    {"edE31_1_Au5Y", "GroupBox15"},
    {"edE31_1_Au6X", "GroupBox18"},
    {"edE31_1_Au6Y", "GroupBox18"},
    {"edE31_1_Fi1X", "GroupBox13"},
    {"edE31_1_Fi1Y", "GroupBox13"},
    {"edE31_1_Fi2X", "GroupBox12"},
    {"edE31_1_Fi2Y", "GroupBox12"},
    {"edE31_1_Fi3X", "GroupBox11"},
    {"edE31_1_Fi3Y", "GroupBox11"},
    {"edE31_1_Fi4X", "GroupBox19"},
    {"edE31_1_Fi4Y", "GroupBox19"},
    {"edE31_1_Fi5X", "GroupBox20"},
    {"edE31_1_Fi5Y", "GroupBox20"},
    {"edE31_1_Fi6X", "GroupBox21"},
    {"edE31_1_Fi6Y", "GroupBox21"},
    {"edE31_2_Au1X", "GroupBox29"},
    {"edE31_2_Au1Y", "GroupBox29"},
    {"edE31_2_Au2X", "GroupBox30"},
    {"edE31_2_Au2Y", "GroupBox30"},
    {"edE31_2_Au3X", "GroupBox31"},
    {"edE31_2_Au3Y", "GroupBox31"},
    {"edE31_2_Au4X", "GroupBox35"},
    {"edE31_2_Au4Y", "GroupBox35"},
    {"edE31_2_Au5X", "GroupBox36"},
    {"edE31_2_Au5Y", "GroupBox36"},
    {"edE31_2_Au6X", "GroupBox37"},
    {"edE31_2_Au6Y", "GroupBox37"},
    {"edE31_2_Fi1X", "GroupBox34"},
    {"edE31_2_Fi1Y", "GroupBox34"},
    {"edE31_2_Fi2X", "GroupBox33"},
    {"edE31_2_Fi2Y", "GroupBox33"},
    {"edE31_2_Fi3X", "GroupBox32"},
    {"edE31_2_Fi3Y", "GroupBox32"},
    {"edE31_2_Fi4X", "GroupBox38"},
    {"edE31_2_Fi4Y", "GroupBox38"},
    {"edE31_2_Fi5X", "GroupBox39"},
    {"edE31_2_Fi5Y", "GroupBox39"},
    {"edE31_2_Fi6X", "GroupBox40"},
    {"edE31_2_Fi6Y", "GroupBox40"},
    {"edE31_Au1X", "gbE31_Auto1"},
    {"edE31_Au1Y", "gbE31_Auto1"},
    {"edE31_Au2X", "gbE31_Auto2"},
    {"edE31_Au2Y", "gbE31_Auto2"},
    {"edE31_Au3X", "gbE31_Auto3"},
    {"edE31_Au3Y", "gbE31_Auto3"},
    {"edE31_Au4X", "gbE31_Auto4"},
    {"edE31_Au4Y", "gbE31_Auto4"},
    {"edE31_Au5X", "gbE31_Auto5"},
    {"edE31_Au5Y", "gbE31_Auto5"},
    {"edE31_Au6X", "grpE31_Auto6"},
    {"edE31_Au6Y", "grpE31_Auto6"},
    {"edE31_Fi1X", "gbE31_Fix1"},
    {"edE31_Fi1Y", "gbE31_Fix1"},
    {"edE31_Fi2X", "gbE31_Fix2"},
    {"edE31_Fi2Y", "gbE31_Fix2"},
    {"edE31_Fi3X", "gbE31_Fix3"},
    {"edE31_Fi3Y", "gbE31_Fix3"},
    {"edE31_Fi4X", "grpE31_Fix4"},
    {"edE31_Fi4Y", "grpE31_Fix4"},
    {"edE31_Fi5X", "grpE31_Fix5"},
    {"edE31_Fi5Y", "grpE31_Fix5"},
    {"edE31_Fi6X", "grpE31_Fix6"},
    {"edE31_Fi6Y", "grpE31_Fix6"},
    {"edE32_1_IS1X", "GroupBox22"},
    {"edE32_1_IS1Y", "GroupBox22"},
    {"edE32_1_IS2X", "GroupBox23"},
    {"edE32_1_IS2Y", "GroupBox23"},
    {"edE32_1_OS1X", "GroupBox24"},
    {"edE32_1_OS1Y", "GroupBox24"},
    {"edE32_1_OS2X", "GroupBox25"},
    {"edE32_1_OS2Y", "GroupBox25"},
    {"edE32_2_IS1X", "GroupBox41"},
    {"edE32_2_IS1Y", "GroupBox41"},
    {"edE32_2_IS2X", "GroupBox42"},
    {"edE32_2_IS2Y", "GroupBox42"},
    {"edE32_2_OS1X", "GroupBox43"},
    {"edE32_2_OS1Y", "GroupBox43"},
    {"edE32_2_OS2X", "GroupBox44"},
    {"edE32_2_OS2Y", "GroupBox44"},
    {"edE32_IS1X", "gbE32_InSh1"},
    {"edE32_IS1Y", "gbE32_InSh1"},
    {"edE32_IS2X", "gbE32_InSh2"},
    {"edE32_IS2Y", "gbE32_InSh2"},
    {"edE32_OS1X", "gbE32_OutSh1"},
    {"edE32_OS1Y", "gbE32_OutSh1"},
    {"edE32_OS2X", "gbE32_OutSh2"},
    {"edE32_OS2Y", "gbE32_OutSh2"},
    {"edE36", "pal_E2"},
    {"edE37", "pal_E2"},
    {"edE51", "pal_E5"},
    {"edE52", "pal_E5"},
    {"edE68", "pal_E5"},
    {"edE73", "pal_E80"},
    {"edF01", "pal_F00"},
    {"edF05", "pal_F00"},
    {"edF13_ADC", "gbF13_RotateShtSpeed"},
    {"edF13_Ini", "gbF13_RotateShtSpeed"},
    {"edF13_Jog", "gbF13_RotateShtSpeed"},
    {"edF14", "pnlF14"},
    {"edF14_1", "pnlF14"},
    {"edF14_1_No", "pnlF14"},
    {"edF14_No", "pnlF14"},
    {"edF23", "pal_F20"},
    {"edF25", "pal_F20"},
    {"edF31_1", "grpF31"},
    {"edF31_2", "grpF31"},
    {"edF31_3", "grpF31"},
    {"edF31_4", "grpF31"},
    {"edF31_5", "grpF31"},
    {"edF31_6", "grpF31"},
    {"edF32", "pal_F30"},
    {"edF37_Delay", "pal_F30"},
    {"edG14", "pal_G"},
    {"edG18Auto1Count", "G18"},
    {"edG18Auto2Count", "G18"},
    {"edG18Auto3Count", "G18"},
    {"edHeadCondition1", "paLifeTime"},
    {"edHeadCondition2", "paLifeTime"},
    {"edHeadCondition3", "paLifeTime"},
    {"edI21", "gbI21"},
    {"edI21_1", "gbI21"},
    {"edI21_6", "gbI21"},
    {"edI21_9", "gbI21"},
    {"edI22_1", "pal_I20"},
    {"edI29", "pal_I20"},
    {"edI29_3", "pal_I20"},
    {"edI44", "pal_I40"},
    {"edI49", "pal_I40"},
    {"edL03", "pal_L1"},
    {"edL04", "pal_L1"},
    {"edL05", "pal_L1"},
    {"edL08_Over", "pal_L1"},
    {"edL08_Under", "pal_L1"},
    {"edL09_Sh1L", "gbL09"},
    {"edL09_Sh1R", "gbL09"},
    {"edL09_Sh2L", "gbL09"},
    {"edL09_Sh2R", "gbL09"},
    {"edL11_1", "gbL11"},
    {"edL11_1_2", "gbL11"},
    {"edL11_2", "gbL11"},
    {"edL11_4", "gbL11"},
    {"edL11_6_Continuous", "gbL11"},
    {"edL11_6_Outside", "gbL11"},
    {"edL11_7_MaxSurge", "gbL11"},
    {"edL24", "pal_L3"},
    {"edL32_4", "grpL32_3"},
    {"edL32_5", "grpL32_3"},
    {"edL32_6", "grpL32_3"},
    {"edL32_7_1", "grpL32_3"},
    {"edL32_7_2", "grpL32_3"},
    {"edL32_8", "grpL32_3"},
    {"edL33_3", "grpL33"},
    {"edL33_4", "grpL33"},
    {"edL33_5", "grpL33"},
    {"edL33_6", "grpL33"},
    {"edL34_2", "grpL34"},
    {"edL34_3", "grpL34"},
    {"edL34_4", "grpL34"},
    {"edL35_2", "grpL35"},
    {"edL40", "tsL_30_2"},
    {"edL41", "tsL_30_2"},
    {"edL49", "tsL_30_2"},
    {"edN04_ID", "gbN04"},
    {"edN04_Model", "gbN04"},
    {"edN05_1", "gbN05_1"},
    {"edN05_AmbTemp", "gbN05"},
    {"edN05_DownPath", "gbN05"},
    {"edN05_RmsPath", "gbN05"},
    {"edN06_1", "gbN06_FTP"},
    {"edN06_DownPath", "gbN06_FTP"},
    {"edN06_FileName", "gbN06_FTP"},
    {"edN06_HostName", "gbN06_FTP"},
    {"edN06_Password", "gbN06_FTP"},
    {"edN06_Port", "gbN06_FTP"},
    {"edN06_TestList", "gbN06_FTP"},
    {"edN06_TesterMap", "gbN06_FTP"},
    {"edN06_UpLdPath", "gbN06_FTP"},
    {"edN06_UserName", "gbN06_FTP"},
    {"edN07_2", "pnlN07_2"},
    {"edN07_5", "gbN07"},
    {"edN08_2", "gbN08"},
    {"edN08_3", "gbN08"},
    {"edN10Host", "grpN10_FTP"},
    {"edN10Password", "grpN10_FTP"},
    {"edN10UploadPath", "grpN10_FTP"},
    {"edN10UserName", "grpN10_FTP"},
    {"edN12_HostName", "gbN12_SocketIdProductDataUpoadToFTP"},
    {"edN12_Password", "gbN12_SocketIdProductDataUpoadToFTP"},
    {"edN12_UpLdPath", "gbN12_SocketIdProductDataUpoadToFTP"},
    {"edN12_UserName", "gbN12_SocketIdProductDataUpoadToFTP"},
    {"edN14_20", "Panel3"},
    {"edN14_21", "Panel4"},
    {"edN14_21_BC", "Panel4"},
    {"edN14_21_HP", "Panel4"},
    {"edN14_21_TF", "Panel4"},
    {"edN15_3", "grpN15"},
    {"edN15_Host", "grpN15"},
    {"edN15_Password", "grpN15"},
    {"edN15_UserName", "grpN15"},
    {"edN16_DownPath", "grpN16"},
    {"edN16_HostName", "grpN16"},
    {"edN16_Password", "grpN16"},
    {"edN16_UpLdPath", "grpN16"},
    {"edN16_UserName", "grpN16"},
    {"edN17_2", "gbN17LotSummary"},
    {"edN17_4", "gbN17ProdLogUL"},
    {"edN21_1", "grpN21"},
    {"edN21_2", "grpN21"},
    {"edN21_3", "grpN21"},
    {"edN21_4", "grpN21"},
    {"edN23_2FTPHost", "gbN23_FTPSetting"},
    {"edN23_2FTPPath", "gbN23_FTPSetting"},
    {"edN23_2Pwd", "gbN23_FTPSetting"},
    {"edN23_2UserName", "gbN23_FTPSetting"},
    {"edN23_3NetDrivePath", "rgbN23_2DSorting"},
    {"edN27_1", "grpN27"},
    {"edN27_2", "grpN27"},
    {"edN27_3", "grpN27"},
    {"edN27_4", "grpN27"},
    {"edN27_6", "grpN27"},
    {"edN30_1", "gbN30"},
    {"edN30_2", "gbN30"},
    {"edN30_3", "gbN30"},
    {"edN30_4", "gbN30"},
    {"edN30_5", "gbN30"},
    {"edN31_1", "gbN31"},
    {"edN31_2", "gbN31"},
    {"edN31_3", "gbN31"},
    {"edN31_4", "gbN31"},
    {"edN31_5", "gbN31"},
    {"edN31_MaxOffset", "gbN31"},
    {"edN31_MinOffset", "gbN31"},
    {"edN32_1", "gbN32"},
    {"edN32_2", "gbN32"},
    {"edN32_3", "gbN32"},
    {"edN32_4", "gbN32"},
    {"edN32_5", "gbN32"},
    {"edN32_6", "gbN32"},
    {"edN33", "gbN33"},
    {"edN34", "gbN34"},
    {"edN35_1", "grpN35"},
    {"edN35_2", "grpN35"},
    {"edN35_3", "grpN35"},
    {"edN35_4", "grpN35"},
    {"edN40_1", "GroupBox6"},
    {"edN40_2", "GroupBox6"},
    {"edN40_3", "GroupBox6"},
    {"edN40_4", "GroupBox6"},
    {"edN41_1", "GroupBox7"},
    {"edO06_FilePath", "gbO06"},
    {"edO07", "pal_O1"},
    {"edP13_1", "pnlP13"},
    {"edP13_2", "pnlP13"},
    {"edP14_1", "pnlP13"},
    {"edP14_2", "pnlP13"},
    {"edP16_1", "pnlP16"},
    {"edP16_2", "pnlP16"},
    {"edP23_1", "gbP23_OCR"},
    {"edP23_2", "gbP23_OCR"},
    {"edP29", "pal_P2"},
    {"edP30", "pal_P2"},
    {"edP48DelayTime", "grpP48"},
    {"edP48Times", "grpP48"},
    {"ed_N22_1_HostAddress", "Panel7"},
    {"ed_N22_1_Password", "Panel7"},
    {"ed_N22_1_UploadPath", "Panel7"},
    {"ed_N22_1_UserName", "Panel7"},
    {"ed_N22_DownloadPath", "pnlN22"},
    {"ed_N22_HostAddress", "pnlN22"},
    {"ed_N22_Password", "pnlN22"},
    {"ed_N22_UserName", "pnlN22"},
    {"ed_N35_1_HostAddress", "Panel1"},
    {"ed_N35_1_Password", "Panel1"},
    {"ed_N35_1_UploadPath", "Panel1"},
    {"ed_N35_1_UserName", "Panel1"},
    {"editN05_RTC", "groupbN05_RTC"},
    {"edtA12_1", "pnlA12"},
    {"edtA28_1", "grpA28"},
    {"edtB05", "grpB05"},
    {"edtC17", "grpC17"},
    {"edtC21_Auto1", "TabSheet3"},
    {"edtC21_Auto1_b", "TabSheet5"},
    {"edtC21_Auto1_m", "TabSheet4"},
    {"edtC21_Auto2", "TabSheet3"},
    {"edtC21_Auto2_b", "TabSheet5"},
    {"edtC21_Auto2_m", "TabSheet4"},
    {"edtC21_Auto3", "TabSheet3"},
    {"edtC21_Auto3_b", "TabSheet5"},
    {"edtC21_Auto3_m", "TabSheet4"},
    {"edtC21_HP1", "TabSheet3"},
    {"edtC21_HP1_b", "TabSheet5"},
    {"edtC21_HP1_m", "TabSheet4"},
    {"edtC21_HP2", "TabSheet3"},
    {"edtC21_HP2_b", "TabSheet5"},
    {"edtC21_HP2_m", "TabSheet4"},
    {"edtC21_SHT1", "TabSheet3"},
    {"edtC21_SHT1_b", "TabSheet5"},
    {"edtC21_SHT1_m", "TabSheet4"},
    {"edtC21_SHT2", "TabSheet3"},
    {"edtC21_SHT2_b", "TabSheet5"},
    {"edtC21_SHT2_m", "TabSheet4"},
    {"edtD04", "pal_D1"},
    {"edtD04_20mm", "pal_D1"},
    {"edtD04_30mm", "pal_D1"},
    {"edtD04_40mm", "pal_D1"},
    {"edtD04_60mm", "pal_D1"},
    {"edtD04_80mm", "pal_D1"},
    {"edtD17_3", "pal_D2"},
    {"edtD47_5", "grpD47"},
    {"edtD47_6", "grpD47"},
    {"edtF23_1", "pal_F20"},
    {"edtI41_5", "grpI41"},
    {"edtL06", "pal_L1"},
    {"edtL09_1", "gbL09"},
    {"edtL09_2", "gbL09"},
    {"edtL36_1", "grpL37"},
    {"edtL36_2", "grpL37"},
    {"edtL39_2", "grpL39"},
    {"edtL42", "tsL_30_2"},
    {"edtL44", "tsL_30_2"},
    {"edtL45", "tsL_30_2"},
    {"edtN04_TesterID", "gbN04"},
    {"edtN07_7", "gbN07"},
    {"edtN09_5Host", "grpN09_5"},
    {"edtN09_5Password", "grpN09_5"},
    {"edtN09_5Path", "grpN09_5"},
    {"edtN09_5User", "grpN09_5"},
    {"edtN09_7", "grpN09"},
    {"edtN09_Handler", "grpN09"},
    {"edtN09_SearchTime", "grpN09"},
    {"edtN09_TSV", "grpN09"},
    {"edtN10_6", "tsN10_11_20"},
    {"edtN10_8", "tsN10_11_20"},
    {"edtN10_Port", "grpN10_FTP"},
    {"edtN14_1", "tsN14_1"},
    {"edtN14_12", "tsN14_2"},
    {"edtN14_12_Path", "tsN14_2"},
    {"edtN14_13", "tsN14_2"},
    {"edtN14_13_Path", "tsN14_2"},
    {"edtN14_14_1", "pnlN14_3"},
    {"edtN14_14_2", "pnlN14_3"},
    {"edtN14_14_3", "pnlN14_3"},
    {"edtN14_15_1", "pnlN14_15"},
    {"edtN14_15_2", "pnlN14_15"},
    {"edtN14_16_1", "grpN14_16"},
    {"edtN14_16_2", "grpN14_16"},
    {"edtN14_16_3", "grpN14_16"},
    {"edtN14_16_4", "grpN14_16"},
    {"edtN14_17", "pnlN14_17"},
    {"edtN14_18", "pnlN14_18"},
    {"edtN14_19", "pnlN14_19"},
    {"edtN14_2", "tsN14_1"},
    {"edtN14_22Exp", "grpN14_22"},
    {"edtN14_22Imp", "grpN14_22"},
    {"edtN14_3Host", "grpN14_3"},
    {"edtN14_3Password", "grpN14_3"},
    {"edtN14_3Path", "grpN14_3"},
    {"edtN14_3UserName", "grpN14_3"},
    {"edtN14_4", "tsN14_1"},
    {"edtN14_5", "tsN14_1"},
    {"edtN14_6_1", "grpN14_6"},
    {"edtN14_6_2", "grpN14_6"},
    {"edtN14_6_3", "grpN14_6"},
    {"edtN14_6_4", "grpN14_6"},
    {"edtN14_7", "tsN14_2"},
    {"edtN14_8", "tsN14_2"},
    {"edtN14_9", "tsN14_2"},
    {"edtN15_1", "grpN15"},
    {"edtN15_2", "grpN15"},
    {"edtN15_4", "grpN15"},
    {"edtN23_2_LineID", "grpN23_2"},
    {"edtN23_2_ProcessName", "grpN23_2"},
    {"edtN23_2_Product", "grpN23_2"},
    {"edtN23_4", "rgbN23_2DSorting"},
    {"edtN25_1_Host", "grpN25_1"},
    {"edtN25_1_Name", "grpN25_1"},
    {"edtN25_1_Password", "grpN25_1"},
    {"edtN25_1_Path", "grpN25_1"},
    {"edtN25_2_Host", "GroupBox1"},
    {"edtN25_2_Interval", "GroupBox1"},
    {"edtN25_2_Name", "GroupBox1"},
    {"edtN25_2_Password", "GroupBox1"},
    {"edtN25_2_Path", "GroupBox1"},
    {"edtN25_3_LogJamPath", "grpN25_3"},
    {"edtN25_4_UploadPath", "grpN25_4"},
    {"edtN25_5_UploadPath", "grpN25_5"},
    {"edtN26_1", "gbN26"},
    {"edtN26_2", "gbN26"},
    {"edtN26_3", "gbN26"},
    {"edtN26_4", "gbN26"},
    {"edtN28_IP", "grpN28"},
    {"edtN28_Path", "grpN28"},
    {"edtN29", "tsN29"},
    {"edtO06AlarmHistroy", "gbO06"},
    {"edtO06AlarmStatist", "gbO06"},
    {"edtO06Production", "gbO06"},
    {"edtO06_Local", "gbO06"},
    {"edtO06_Remote", "gbO06"},
    {"edtO11", "pal_O2"},
    {"edtO16", "pal_O2"},
    {"edtO17_Count", "pal_O2"},
    {"edtO17_Time", "pal_O2"},
    {"edtO18", "pal_O2"},
    {"edtO19_6", "grpO16"},
    {"edtP65", "plP65"},
    {"edtSearchFunction", "pnlsearch"},
    {"gbA10", "pal_A1"},
    {"gbA22", "pal_A3"},
    {"gbA31", "pal_A4"},
    {"gbA31_CleanTiming", "gbA31"},
    {"gbA56", "tsA_05"},
    {"gbA57", "tsA_05"},
    {"gbA71", "tsA06"},
    {"gbB11", "pal_B11"},
    {"gbB12", "pal_B11"},
    {"gbB13", "pal_B11"},
    {"gbB14", "pal_B11"},
    {"gbC05", "pal_C1"},
    {"gbC06", "pal_C1"},
    {"gbD25", "pal_D3"},
    {"gbD60", "pal_D7"},
    {"gbE30_1_HP1", "pnlE30_Hot"},
    {"gbE30_1_HP2", "pnlE30_Hot"},
    {"gbE30_1_loader", "pnlE30_Hot"},
    {"gbE30_2_HP1", "pnlE30_Cold"},
    {"gbE30_2_HP2", "pnlE30_Cold"},
    {"gbE30_2_loader", "pnlE30_Cold"},
    {"gbE30_HP1", "palE30"},
    {"gbE30_HP2", "palE30"},
    {"gbE30_loader", "palE30"},
    {"gbE31_Auto1", "palE31"},
    {"gbE31_Auto2", "palE31"},
    {"gbE31_Auto3", "palE31"},
    {"gbE31_Auto4", "palE31"},
    {"gbE31_Auto5", "palE31"},
    {"gbE31_Fix1", "palE31"},
    {"gbE31_Fix2", "palE31"},
    {"gbE31_Fix3", "palE31"},
    {"gbE32_InSh1", "palE32"},
    {"gbE32_InSh2", "palE32"},
    {"gbE32_OutSh1", "palE32"},
    {"gbE32_OutSh2", "palE32"},
    {"gbE85", "pal_E80"},
    {"gbF13_RotateShtSpeed", "pal_F10"},
    {"gbI21", "pal_I20"},
    {"gbI50", "pal_I40"},
    {"gbI54", "tsI_50"},
    {"gbL09", "pal_L1"},
    {"gbL11", "pal_L2"},
    {"gbL30", "pal_L3"},
    {"gbM01", "pal_M"},
    {"gbN04", "tsN00"},
    {"gbN05", "tsN05"},
    {"gbN05_1", "gbN05"},
    {"gbN05_WebService", "gbN05"},
    {"gbN06_FTP", "tsN06"},
    {"gbN06_HddLevel", "gbN06_FTP"},
    {"gbN06_ServerLv", "gbN06_FTP"},
    {"gbN07", "tsN07"},
    {"gbN08", "tsN08"},
    {"gbN11", "tsN11"},
    {"gbN12_SocketIdProductDataUpoadToFTP", "tsN12"},
    {"gbN17LotSummary", "tsN17"},
    {"gbN17ProdLogUL", "tsN17"},
    {"gbN23_FTPSetting", "rgbN23_2DSorting"},
    {"gbN26", "tsN26"},
    {"gbN30", "tsN30"},
    {"gbN31", "tsN31"},
    {"gbN32", "tsN32"},
    {"gbN33", "tsN33"},
    {"gbN33_1", "tsN33"},
    {"gbN34", "tsN34"},
    {"gbO06", "pal_O1"},
    {"gbP23_OCR", "pal_P2"},
    {"gbP26_OCRCheck", "pal_P2"},
    {"gbUnloadMode", "pal_P3"},
    {"groupbN05_RTC", "gbN05"},
    {"grpA17", "pal_A2"},
    {"grpA20", "pal_A2"},
    {"grpA25", "pal_A3"},
    {"grpA28", "pal_A3"},
    {"grpA32", "pal_A4"},
    {"grpA32_1", "grpA32"},
    {"grpA55", "tsA_05"},
    {"grpA60", "tsA_05"},
    {"grpASEMNetworkDrive", "tsN13"},
    {"grpB01", "pal_B1"},
    {"grpB02", "pal_B1"},
    {"grpB03", "pal_B1"},
    {"grpB05", "pal_B1"},
    {"grpC17", "pal_C2"},
    {"grpC21", "tsC21"},
    {"grpCommMsg", "Panel2"},
    {"grpD22", "pal_D3"},
    {"grpD47", "pal_D5"},
    {"grpE31_Auto6", "palE31"},
    {"grpE31_Fix4", "palE31"},
    {"grpE31_Fix5", "palE31"},
    {"grpE31_Fix6", "palE31"},
    {"grpF31", "pal_F30"},
    {"grpI31", "pal_I30"},
    {"grpI37", "pal_I30"},
    {"grpI41", "pal_I40"},
    {"grpL31", "tsL_30"},
    {"grpL32", "tsL_30"},
    {"grpL32_1", "grpL32"},
    {"grpL32_2", "grpL32"},
    {"grpL32_3", "grpL32"},
    {"grpL33", "tsL_30_1"},
    {"grpL34", "tsL_30_1"},
    {"grpL35", "tsL_30_1"},
    {"grpL37", "tsL_30_2"},
    {"grpL39", "tsL_30_2"},
    {"grpLogUploadToFTP", "tsN10_1_10"},
    {"grpN09", "tsN09"},
    {"grpN09_5", "grpN09"},
    {"grpN10_FTP", "grpLogUploadToFTP"},
    {"grpN14", "tsN14"},
    {"grpN14_16", "tsN14_4"},
    {"grpN14_22", "tsN14_5"},
    {"grpN14_3", "tsN14_1"},
    {"grpN14_6", "tsN14_1"},
    {"grpN15", "tsN15"},
    {"grpN16", "tsN16"},
    {"grpN20", "pal_N"},
    {"grpN21", "tsN21"},
    {"grpN22", "tsN22"},
    {"grpN23_1", "tsN23_1"},
    {"grpN23_2", "tsN23_2"},
    {"grpN23_3", "tsN23_3"},
    {"grpN23_4", "tsN23_3"},
    {"grpN23_5", "tsN23_3"},
    {"grpN24", "tsN24"},
    {"grpN25", "tsN25_1_3"},
    {"grpN25_1", "grpN25"},
    {"grpN25_3", "grpN25"},
    {"grpN25_4", "tsN25_4"},
    {"grpN25_5", "tsN25_4"},
    {"grpN27", "tsN27"},
    {"grpN28", "tsN28"},
    {"grpN35", "tsN35"},
    {"grpO15", "pal_O2"},
    {"grpO16", "pal_O2"},
    {"grpP48", "pal_P3"},
    {"imgI37_3", "grpI37"},
    {"labC05_2", "bgC05_1"},
    {"labC05_4", "bgC05_1"},
    {"labC05_6", "bgC05_1"},
    {"labC05_8", "bgC05_1"},
    {"labD21_1", "pal_D3"},
    {"labD21_2", "pal_D3"},
    {"labD25_1", "gbD25"},
    {"labD25_1_NS", "gbD25"},
    {"labD25_2", "gbD25"},
    {"labD25_2_NS", "gbD25"},
    {"labD25_3", "gbD25"},
    {"labD25_3_NS", "gbD25"},
    {"labD25_4", "gbD25"},
    {"labD25_5", "gbD25"},
    {"labD25_6", "gbD25"},
    {"labD41", "pnlD41"},
    {"labD41_1", "pnlD41"},
    {"labD41_2", "pnlD41"},
    {"labD41_3", "pnlD41"},
    {"labD47_1", "grpD47"},
    {"labD47_2", "grpD47"},
    {"labD47_3", "grpD47"},
    {"labD53", "pal_D6"},
    {"labD60", "gbD60"},
    {"labD60_1", "gbD60"},
    {"labD60_NS", "gbD60"},
    {"labD62", "pal_D7"},
    {"labD68_1", "pal_D7"},
    {"labF05", "pal_F00"},
    {"labF12", "pal_F10"},
    {"labF37_Delay", "pal_F30"},
    {"labL40", "tsL_30_2"},
    {"labL41", "tsL_30_2"},
    {"labN05_1", "gbN05"},
    {"labN05_AmbTemp", "gbN05"},
    {"labN05_DownPath", "gbN05"},
    {"labN06_DownloadPath", "gbN06_FTP"},
    {"labN06_FileName", "gbN06_FTP"},
    {"labN06_TesterList", "gbN06_FTP"},
    {"labN06_TesterMap", "gbN06_FTP"},
    {"labN06_UploadPath", "gbN06_FTP"},
    {"labN07_5", "gbN07"},
    {"labN31_5", "gbN31"},
    {"lbI21_6", "gbI21"},
    {"lb_N22_DownloadPath", "pnlN22"},
    {"lblA16", "pal_A2"},
    {"lblA60_1", "grpA60"},
    {"lblA60_2", "grpA60"},
    {"lblA60_3", "grpA60"},
    {"lblA60_4", "grpA60"},
    {"lblA60_5", "grpA60"},
    {"lblIPSCClear_Qty", "grpN14_16"},
    {"lblIPSCIn_Qty", "grpN14_16"},
    {"lblIPSCOut_Qty", "grpN14_16"},
    {"lblIPSC_Qty", "grpN14_16"},
    {"lblN10DataType", "tsN10_11_20"},
    {"lblN10_6", "tsN10_11_20"},
    {"lblO17", "pal_O2"},
    {"lblP04", "pal_P0"},
    {"lblUploadRealPath", "grpN10_FTP"},
    {"lbledtN07_6", "gbN07"},
    {"lbledtN23_1_URL", "grpN23_1"},
    {"lbledtN23_2_URL", "grpN23_3"},
    {"lbledtN23_4_URL", "grpN23_4"},
    {"lbledtN23_5_UploadPath", "grpN23_5"},
    {"lbledtN24", "grpN24"},
    {"listHeaterMonitor", "grpCommMsg"},
    {"paLifeTime", "pal_O2"},
    {"palA10_ART", "gbA10"},
    {"palD01", "pal_D1"},
    {"palE30", "pnlE30"},
    {"palE31", "pnlE31"},
    {"palE32", "pnlE32"},
    {"palE32_Cold", "pnlE32_Cold"},
    {"palE32_Hot", "pnlE32_Hot"},
    {"pal_A1", "tsA_00"},
    {"pal_A2", "tsA_01"},
    {"pal_A3", "tsA_03"},
    {"pal_A4", "tsA03"},
    {"pal_B1", "tsB_00"},
    {"pal_B11", "tsB11_15"},
    {"pal_C1", "tsC_00"},
    {"pal_C2", "tsC_10"},
    {"pal_D1", "tsD_Preasure"},
    {"pal_D2", "tsD_HighCal"},
    {"pal_D3", "tsD_Contact"},
    {"pal_D4", "tsD_Mode"},
    {"pal_D5", "tsD_Other"},
    {"pal_D6", "tsD_50"},
    {"pal_D7", "tsD_60"},
    {"pal_D8", "tsD_70"},
    {"pal_E2", "tsE_Other"},
    {"pal_E5", "tsE50"},
    {"pal_E80", "tsE70"},
    {"pal_F00", "tsF01"},
    {"pal_F10", "TabSheet2"},
    {"pal_F20", "tsF21"},
    {"pal_F30", "tsF31"},
    {"pal_G", "tsG00"},
    {"pal_I", "tsI00"},
    {"pal_I01", "tsI_01"},
    {"pal_I20", "tsI_20"},
    {"pal_I30", "tsI_30"},
    {"pal_I40", "tsI_40"},
    {"pal_L1", "tsL_00"},
    {"pal_L2", "tsL_10"},
    {"pal_L3", "tsL_20"},
    {"pal_M", "tsM00"},
    {"pal_N", "tsN00"},
    {"pal_O1", "tsO_00"},
    {"pal_O2", "tsO_11"},
    {"pal_P0", "tsP_00"},
    {"pal_P1", "tsP_10"},
    {"pal_P2", "tsP20"},
    {"pal_P3", "tsP30"},
    {"pcA00", "tsA00"},
    {"pcB00", "tsb00"},
    {"pcConfig", "tsConfig"},
    {"pcD00", "tsD00"},
    {"pcE00", "tsE00"},
    {"pcF00", "tsF00"},
    {"pcL00", "tsL00"},
    {"pcN00", "pal_N"},
    {"pcN25", "tsN25"},
    {"pcO00", "tsO00"},
    {"pcP00", "tsP00"},
    {"pgC00", "tsC00"},
    {"pgI00", "pal_I"},
    {"pgcE31", "tsE31"},
    {"pgcN10_1", "tsN10"},
    {"pgcN14_1", "grpN14"},
    {"pgcN23", "tsN23"},
    {"pgcScale", "tsE_XYScale"},
    {"plP62", "tsP50"},
    {"plP65", "tsP50"},
    {"pnlA12", "pal_A2"},
    {"pnlD41", "pal_D5"},
    {"pnlD63", "pal_D7"},
    {"pnlE30", "tsE30"},
    {"pnlE30_Cold", "tsE30"},
    {"pnlE30_Hot", "tsE30"},
    {"pnlE31", "tsE31_1"},
    {"pnlE31_2", "pnlE31_Hot"},
    {"pnlE31_Cold", "tsE31_Cold"},
    {"pnlE31_Hot", "tsE31_Hot"},
    {"pnlE32", "tsE32"},
    {"pnlE32_3", "pnlE31_Cold"},
    {"pnlE32_Cold", "tsE32"},
    {"pnlE32_Hot", "tsE32"},
    {"pnlF14", "pal_F10"},
    {"pnlHP", "tsHPData"},
    {"pnlI17", "pal_I"},
    {"pnlN07_2", "gbN07"},
    {"pnlN14_15", "tsN14_3"},
    {"pnlN14_17", "tsN14_3"},
    {"pnlN14_18", "tsN14_3"},
    {"pnlN14_19", "tsN14_3"},
    {"pnlN14_3", "tsN14_3"},
    {"pnlN22", "grpN22"},
    {"pnlP13", "pal_P1"},
    {"pnlP16", "pal_P1"},
    {"pnlSoftSpeed", "tsSoftSimu"},
    {"pnlTray", "tsTrayData"},
    {"pnlsearch", "tsSearchFunction"},
    {"rgC05", "gbC05"},
    {"rgD17", "pal_D2"},
    {"rgD69", "pal_D7"},
    {"rgD71", "pal_D8"},
    {"rgE50", "pal_E5"},
    {"rgF07", "pal_F00"},
    {"rgF26", "pal_F20"},
    {"rgI22", "pal_I20"},
    {"rgI38", "pal_I30"},
    {"rgI46", "pal_I40"},
    {"rgL10", "pal_L1"},
    {"rgN09_4", "grpN09"},
    {"rgN10_3_1", "grpLogUploadToFTP"},
    {"rgN10_4", "grpLogUploadToFTP"},
    {"rgN14_20_1", "Panel3"},
    {"rgN23_1_2DSorting", "rgbN23_2DSorting"},
    {"rgN32", "gbN32"},
    {"rgN35_1", "grpN35"},
    {"rgP23_OCRByInitialStart", "gbP23_OCR"},
    {"rgP23_OCRByNewTray", "gbP23_OCR"},
    {"rgP58RunModeAfterFT", "tsP50"},
    {"rgUpDlMethod", "gbN05"},
    {"rgbN23_2DSorting", "tsN23_0"},
    {"sbtReloadHP", "pnlHP"},
    {"sbtReloadTray", "pnlTray"},
    {"spbA27", "pal_A3"},
    {"strngrdAutoSaveLog", "gbO06"},
    {"tbD25_Index30mm", "gbD25"},
    {"tbD25_Index30mm_NS", "gbD25"},
    {"tbD25_Index40mm", "gbD25"},
    {"tbD25_Index40mm_NS", "gbD25"},
    {"tbD25_Index60mm", "gbD25"},
    {"tbD25_Index60mm_NS", "gbD25"},
    {"tbD60_Index56mm", "gbD60"},
    {"tbD60_Index56mm_NS", "gbD60"},
    {"tsA00", "pcConfig"},
    {"tsA03", "pcA00"},
    {"tsA06", "pcA00"},
    {"tsA_00", "pcA00"},
    {"tsA_01", "pcA00"},
    {"tsA_03", "pcA00"},
    {"tsA_05", "pcA00"},
    {"tsB11_15", "pcB00"},
    {"tsB_00", "pcB00"},
    {"tsC00", "pcConfig"},
    {"tsC21", "pgC00"},
    {"tsC_00", "pgC00"},
    {"tsC_10", "pgC00"},
    {"tsConfig", "PageControl1"},
    {"tsD00", "pcConfig"},
    {"tsD_50", "pcD00"},
    {"tsD_60", "pcD00"},
    {"tsD_70", "pcD00"},
    {"tsD_80", "pcD00"},
    {"tsD_Contact", "pcD00"},
    {"tsD_HighCal", "pcD00"},
    {"tsD_Mode", "pcD00"},
    {"tsD_Other", "pcD00"},
    {"tsD_Preasure", "pcD00"},
    {"tsE00", "pcConfig"},
    {"tsE30", "pgcScale"},
    {"tsE31", "pgcScale"},
    {"tsE31_1", "pgcE31"},
    {"tsE31_Cold", "pgcE31"},
    {"tsE31_Hot", "pgcE31"},
    {"tsE32", "pgcScale"},
    {"tsE50", "pcE00"},
    {"tsE70", "pcE00"},
    {"tsE_Other", "pcE00"},
    {"tsE_XYScale", "pcE00"},
    {"tsF00", "pcConfig"},
    {"tsF01", "pcF00"},
    {"tsF21", "pcF00"},
    {"tsF31", "pcF00"},
    {"tsG00", "pcConfig"},
    {"tsHPData", "PageControl1"},
    {"tsI00", "pcConfig"},
    {"tsI_01", "pgI00"},
    {"tsI_20", "pgI00"},
    {"tsI_30", "pgI00"},
    {"tsI_40", "pgI00"},
    {"tsI_50", "pgI00"},
    {"tsL00", "pcConfig"},
    {"tsL_00", "pcL00"},
    {"tsL_10", "pcL00"},
    {"tsL_20", "pcL00"},
    {"tsL_30", "pcL00"},
    {"tsL_30_1", "pcL00"},
    {"tsL_30_2", "pcL00"},
    {"tsM00", "pcConfig"},
    {"tsN00", "pcConfig"},
    {"tsN05", "pcN00"},
    {"tsN06", "pcN00"},
    {"tsN07", "pcN00"},
    {"tsN08", "pcN00"},
    {"tsN09", "pcN00"},
    {"tsN10", "pcN00"},
    {"tsN10_11_20", "pgcN10_1"},
    {"tsN10_1_10", "pgcN10_1"},
    {"tsN11", "pcN00"},
    {"tsN12", "pcN00"},
    {"tsN13", "pcN00"},
    {"tsN14", "pcN00"},
    {"tsN14_1", "pgcN14_1"},
    {"tsN14_2", "pgcN14_1"},
    {"tsN14_23", "pgcN14_1"},
    {"tsN14_3", "pgcN14_1"},
    {"tsN14_4", "pgcN14_1"},
    {"tsN14_5", "pgcN14_1"},
    {"tsN15", "pcN00"},
    {"tsN16", "pcN00"},
    {"tsN17", "pcN00"},
    {"tsN21", "pcN00"},
    {"tsN22", "pcN00"},
    {"tsN23", "pcN00"},
    {"tsN23_0", "pgcN23"},
    {"tsN23_1", "pgcN23"},
    {"tsN23_2", "pgcN23"},
    {"tsN23_3", "pgcN23"},
    {"tsN24", "pcN00"},
    {"tsN25", "pcN00"},
    {"tsN25_1_3", "pcN25"},
    {"tsN25_4", "pcN25"},
    {"tsN26", "pcN00"},
    {"tsN27", "pcN00"},
    {"tsN28", "pcN00"},
    {"tsN29", "pcN00"},
    {"tsN30", "pcN00"},
    {"tsN31", "pcN00"},
    {"tsN32", "pcN00"},
    {"tsN33", "pcN00"},
    {"tsN34", "pcN00"},
    {"tsN35", "pcN00"},
    {"tsN40", "pcN00"},
    {"tsO00", "pcConfig"},
    {"tsO21", "pcO00"},
    {"tsO_00", "pcO00"},
    {"tsO_11", "pcO00"},
    {"tsP00", "pcConfig"},
    {"tsP20", "pcP00"},
    {"tsP30", "pcP00"},
    {"tsP50", "pcP00"},
    {"tsP_00", "pcP00"},
    {"tsP_10", "pcP00"},
    {"tsSearchFunction", "pcConfig"},
    {"tsSoftSimu", "PageControl1"},
    {"tsTempComm", "PageControl1"},
    {"tsTrayData", "PageControl1"},
    {"tsb00", "pcConfig"},
    {"udD46", "pal_D5"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void IC_CreateContainerProxies()
{
    EL<TGroupBox>("TfConfiguration", "G18");
    EL<TGroupBox>("TfConfiguration", "GroupBox1");
    EL<TGroupBox>("TfConfiguration", "GroupBox10");
    EL<TGroupBox>("TfConfiguration", "GroupBox11");
    EL<TGroupBox>("TfConfiguration", "GroupBox12");
    EL<TGroupBox>("TfConfiguration", "GroupBox13");
    EL<TGroupBox>("TfConfiguration", "GroupBox14");
    EL<TGroupBox>("TfConfiguration", "GroupBox15");
    EL<TGroupBox>("TfConfiguration", "GroupBox18");
    EL<TGroupBox>("TfConfiguration", "GroupBox19");
    EL<TGroupBox>("TfConfiguration", "GroupBox2");
    EL<TGroupBox>("TfConfiguration", "GroupBox20");
    EL<TGroupBox>("TfConfiguration", "GroupBox21");
    EL<TGroupBox>("TfConfiguration", "GroupBox22");
    EL<TGroupBox>("TfConfiguration", "GroupBox23");
    EL<TGroupBox>("TfConfiguration", "GroupBox24");
    EL<TGroupBox>("TfConfiguration", "GroupBox25");
    EL<TGroupBox>("TfConfiguration", "GroupBox29");
    EL<TGroupBox>("TfConfiguration", "GroupBox3");
    EL<TGroupBox>("TfConfiguration", "GroupBox30");
    EL<TGroupBox>("TfConfiguration", "GroupBox31");
    EL<TGroupBox>("TfConfiguration", "GroupBox32");
    EL<TGroupBox>("TfConfiguration", "GroupBox33");
    EL<TGroupBox>("TfConfiguration", "GroupBox34");
    EL<TGroupBox>("TfConfiguration", "GroupBox35");
    EL<TGroupBox>("TfConfiguration", "GroupBox36");
    EL<TGroupBox>("TfConfiguration", "GroupBox37");
    EL<TGroupBox>("TfConfiguration", "GroupBox38");
    EL<TGroupBox>("TfConfiguration", "GroupBox39");
    EL<TGroupBox>("TfConfiguration", "GroupBox40");
    EL<TGroupBox>("TfConfiguration", "GroupBox41");
    EL<TGroupBox>("TfConfiguration", "GroupBox42");
    EL<TGroupBox>("TfConfiguration", "GroupBox43");
    EL<TGroupBox>("TfConfiguration", "GroupBox44");
    EL<TGroupBox>("TfConfiguration", "GroupBox5");
    EL<TGroupBox>("TfConfiguration", "GroupBox6");
    EL<TGroupBox>("TfConfiguration", "GroupBox7");
    EL<TGroupBox>("TfConfiguration", "GroupBox8");
    EL<TGroupBox>("TfConfiguration", "GroupBox9");
    EL<TPageControl>("TfConfiguration", "PageControl1");
    EL<TPageControl>("TfConfiguration", "PageControl2");
    EL<TPanel>("TfConfiguration", "Panel1");
    EL<TPanel>("TfConfiguration", "Panel2");
    EL<TPanel>("TfConfiguration", "Panel3");
    EL<TPanel>("TfConfiguration", "Panel4");
    EL<TPanel>("TfConfiguration", "Panel5");
    EL<TPanel>("TfConfiguration", "Panel6");
    EL<TPanel>("TfConfiguration", "Panel7");
    EL<TTabSheet>("TfConfiguration", "TabSheet2");
    EL<TTabSheet>("TfConfiguration", "TabSheet3");
    EL<TTabSheet>("TfConfiguration", "TabSheet4");
    EL<TTabSheet>("TfConfiguration", "TabSheet5");
    EL<TGroupBox>("TfConfiguration", "bgC05_1");
    EL<TGroupBox>("TfConfiguration", "gbA10");
    EL<TGroupBox>("TfConfiguration", "gbA22");
    EL<TGroupBox>("TfConfiguration", "gbA31");
    EL<TGroupBox>("TfConfiguration", "gbA31_CleanTiming");
    EL<TGroupBox>("TfConfiguration", "gbA56");
    EL<TGroupBox>("TfConfiguration", "gbA57");
    EL<TGroupBox>("TfConfiguration", "gbA71");
    EL<TGroupBox>("TfConfiguration", "gbB11");
    EL<TGroupBox>("TfConfiguration", "gbB12");
    EL<TGroupBox>("TfConfiguration", "gbB13");
    EL<TGroupBox>("TfConfiguration", "gbB14");
    EL<TGroupBox>("TfConfiguration", "gbC05");
    EL<TGroupBox>("TfConfiguration", "gbC06");
    EL<TGroupBox>("TfConfiguration", "gbD25");
    EL<TGroupBox>("TfConfiguration", "gbD60");
    EL<TGroupBox>("TfConfiguration", "gbE30_1_HP1");
    EL<TGroupBox>("TfConfiguration", "gbE30_1_HP2");
    EL<TGroupBox>("TfConfiguration", "gbE30_1_loader");
    EL<TGroupBox>("TfConfiguration", "gbE30_2_HP1");
    EL<TGroupBox>("TfConfiguration", "gbE30_2_HP2");
    EL<TGroupBox>("TfConfiguration", "gbE30_2_loader");
    EL<TGroupBox>("TfConfiguration", "gbE30_HP1");
    EL<TGroupBox>("TfConfiguration", "gbE30_HP2");
    EL<TGroupBox>("TfConfiguration", "gbE30_loader");
    EL<TGroupBox>("TfConfiguration", "gbE31_Auto1");
    EL<TGroupBox>("TfConfiguration", "gbE31_Auto2");
    EL<TGroupBox>("TfConfiguration", "gbE31_Auto3");
    EL<TGroupBox>("TfConfiguration", "gbE31_Auto4");
    EL<TGroupBox>("TfConfiguration", "gbE31_Auto5");
    EL<TGroupBox>("TfConfiguration", "gbE31_Fix1");
    EL<TGroupBox>("TfConfiguration", "gbE31_Fix2");
    EL<TGroupBox>("TfConfiguration", "gbE31_Fix3");
    EL<TGroupBox>("TfConfiguration", "gbE32_InSh1");
    EL<TGroupBox>("TfConfiguration", "gbE32_InSh2");
    EL<TGroupBox>("TfConfiguration", "gbE32_OutSh1");
    EL<TGroupBox>("TfConfiguration", "gbE32_OutSh2");
    EL<TGroupBox>("TfConfiguration", "gbE85");
    EL<TGroupBox>("TfConfiguration", "gbF13_RotateShtSpeed");
    EL<TGroupBox>("TfConfiguration", "gbI21");
    EL<TGroupBox>("TfConfiguration", "gbI50");
    EL<TGroupBox>("TfConfiguration", "gbI54");
    EL<TGroupBox>("TfConfiguration", "gbL09");
    EL<TGroupBox>("TfConfiguration", "gbL11");
    EL<TGroupBox>("TfConfiguration", "gbL30");
    EL<TGroupBox>("TfConfiguration", "gbM01");
    EL<TGroupBox>("TfConfiguration", "gbN04");
    EL<TGroupBox>("TfConfiguration", "gbN05");
    EL<TGroupBox>("TfConfiguration", "gbN05_1");
    EL<TGroupBox>("TfConfiguration", "gbN05_WebService");
    EL<TGroupBox>("TfConfiguration", "gbN06_FTP");
    EL<TGroupBox>("TfConfiguration", "gbN07");
    EL<TGroupBox>("TfConfiguration", "gbN08");
    EL<TGroupBox>("TfConfiguration", "gbN11");
    EL<TGroupBox>("TfConfiguration", "gbN12_SocketIdProductDataUpoadToFTP");
    EL<TGroupBox>("TfConfiguration", "gbN17LotSummary");
    EL<TGroupBox>("TfConfiguration", "gbN17ProdLogUL");
    EL<TGroupBox>("TfConfiguration", "gbN23_FTPSetting");
    EL<TGroupBox>("TfConfiguration", "gbN26");
    EL<TGroupBox>("TfConfiguration", "gbN30");
    EL<TGroupBox>("TfConfiguration", "gbN31");
    EL<TGroupBox>("TfConfiguration", "gbN32");
    EL<TGroupBox>("TfConfiguration", "gbN33");
    EL<TGroupBox>("TfConfiguration", "gbN33_1");
    EL<TGroupBox>("TfConfiguration", "gbN34");
    EL<TGroupBox>("TfConfiguration", "gbO06");
    EL<TGroupBox>("TfConfiguration", "gbP23_OCR");
    EL<TGroupBox>("TfConfiguration", "gbP26_OCRCheck");
    EL<TGroupBox>("TfConfiguration", "gbUnloadMode");
    EL<TGroupBox>("TfConfiguration", "groupbN05_RTC");
    EL<TGroupBox>("TfConfiguration", "grpA17");
    EL<TGroupBox>("TfConfiguration", "grpA20");
    EL<TGroupBox>("TfConfiguration", "grpA25");
    EL<TGroupBox>("TfConfiguration", "grpA28");
    EL<TGroupBox>("TfConfiguration", "grpA32");
    EL<TGroupBox>("TfConfiguration", "grpA32_1");
    EL<TGroupBox>("TfConfiguration", "grpA55");
    EL<TGroupBox>("TfConfiguration", "grpA60");
    EL<TGroupBox>("TfConfiguration", "grpASEMNetworkDrive");
    EL<TGroupBox>("TfConfiguration", "grpB01");
    EL<TGroupBox>("TfConfiguration", "grpB02");
    EL<TGroupBox>("TfConfiguration", "grpB03");
    EL<TGroupBox>("TfConfiguration", "grpB05");
    EL<TGroupBox>("TfConfiguration", "grpC17");
    EL<TGroupBox>("TfConfiguration", "grpC21");
    EL<TGroupBox>("TfConfiguration", "grpCommMsg");
    EL<TGroupBox>("TfConfiguration", "grpD22");
    EL<TGroupBox>("TfConfiguration", "grpD47");
    EL<TGroupBox>("TfConfiguration", "grpE31_Auto6");
    EL<TGroupBox>("TfConfiguration", "grpE31_Fix4");
    EL<TGroupBox>("TfConfiguration", "grpE31_Fix5");
    EL<TGroupBox>("TfConfiguration", "grpE31_Fix6");
    EL<TGroupBox>("TfConfiguration", "grpF31");
    EL<TGroupBox>("TfConfiguration", "grpI31");
    EL<TGroupBox>("TfConfiguration", "grpI37");
    EL<TGroupBox>("TfConfiguration", "grpI41");
    EL<TGroupBox>("TfConfiguration", "grpL32");
    EL<TGroupBox>("TfConfiguration", "grpL32_1");
    EL<TGroupBox>("TfConfiguration", "grpL32_2");
    EL<TGroupBox>("TfConfiguration", "grpL32_3");
    EL<TGroupBox>("TfConfiguration", "grpL33");
    EL<TGroupBox>("TfConfiguration", "grpL34");
    EL<TGroupBox>("TfConfiguration", "grpL35");
    EL<TGroupBox>("TfConfiguration", "grpL37");
    EL<TGroupBox>("TfConfiguration", "grpL39");
    EL<TGroupBox>("TfConfiguration", "grpLogUploadToFTP");
    EL<TGroupBox>("TfConfiguration", "grpN09");
    EL<TGroupBox>("TfConfiguration", "grpN09_5");
    EL<TGroupBox>("TfConfiguration", "grpN10_FTP");
    EL<TGroupBox>("TfConfiguration", "grpN14");
    EL<TGroupBox>("TfConfiguration", "grpN14_16");
    EL<TGroupBox>("TfConfiguration", "grpN14_22");
    EL<TGroupBox>("TfConfiguration", "grpN14_3");
    EL<TGroupBox>("TfConfiguration", "grpN14_6");
    EL<TGroupBox>("TfConfiguration", "grpN15");
    EL<TGroupBox>("TfConfiguration", "grpN16");
    EL<TGroupBox>("TfConfiguration", "grpN20");
    EL<TGroupBox>("TfConfiguration", "grpN21");
    EL<TGroupBox>("TfConfiguration", "grpN22");
    EL<TGroupBox>("TfConfiguration", "grpN23_1");
    EL<TGroupBox>("TfConfiguration", "grpN23_2");
    EL<TGroupBox>("TfConfiguration", "grpN23_3");
    EL<TGroupBox>("TfConfiguration", "grpN23_4");
    EL<TGroupBox>("TfConfiguration", "grpN23_5");
    EL<TGroupBox>("TfConfiguration", "grpN24");
    EL<TGroupBox>("TfConfiguration", "grpN25");
    EL<TGroupBox>("TfConfiguration", "grpN25_1");
    EL<TGroupBox>("TfConfiguration", "grpN25_3");
    EL<TGroupBox>("TfConfiguration", "grpN25_4");
    EL<TGroupBox>("TfConfiguration", "grpN25_5");
    EL<TGroupBox>("TfConfiguration", "grpN27");
    EL<TGroupBox>("TfConfiguration", "grpN28");
    EL<TGroupBox>("TfConfiguration", "grpN35");
    EL<TGroupBox>("TfConfiguration", "grpO15");
    EL<TGroupBox>("TfConfiguration", "grpO16");
    EL<TGroupBox>("TfConfiguration", "grpP48");
    EL<TPanel>("TfConfiguration", "paLifeTime");
    EL<TPanel>("TfConfiguration", "palA10_ART");
    EL<TPanel>("TfConfiguration", "palD01");
    EL<TPanel>("TfConfiguration", "palE30");
    EL<TPanel>("TfConfiguration", "palE31");
    EL<TPanel>("TfConfiguration", "palE32");
    EL<TPanel>("TfConfiguration", "palE32_Cold");
    EL<TPanel>("TfConfiguration", "palE32_Hot");
    EL<TPanel>("TfConfiguration", "pal_A1");
    EL<TPanel>("TfConfiguration", "pal_A2");
    EL<TPanel>("TfConfiguration", "pal_A3");
    EL<TPanel>("TfConfiguration", "pal_A4");
    EL<TPanel>("TfConfiguration", "pal_B1");
    EL<TPanel>("TfConfiguration", "pal_B11");
    EL<TPanel>("TfConfiguration", "pal_C1");
    EL<TPanel>("TfConfiguration", "pal_C2");
    EL<TPanel>("TfConfiguration", "pal_D1");
    EL<TPanel>("TfConfiguration", "pal_D2");
    EL<TPanel>("TfConfiguration", "pal_D3");
    EL<TPanel>("TfConfiguration", "pal_D4");
    EL<TPanel>("TfConfiguration", "pal_D5");
    EL<TPanel>("TfConfiguration", "pal_D6");
    EL<TPanel>("TfConfiguration", "pal_D7");
    EL<TPanel>("TfConfiguration", "pal_D8");
    EL<TPanel>("TfConfiguration", "pal_E2");
    EL<TPanel>("TfConfiguration", "pal_E5");
    EL<TPanel>("TfConfiguration", "pal_E80");
    EL<TPanel>("TfConfiguration", "pal_F00");
    EL<TPanel>("TfConfiguration", "pal_F10");
    EL<TPanel>("TfConfiguration", "pal_F20");
    EL<TPanel>("TfConfiguration", "pal_F30");
    EL<TPanel>("TfConfiguration", "pal_G");
    EL<TPanel>("TfConfiguration", "pal_I");
    EL<TPanel>("TfConfiguration", "pal_I01");
    EL<TPanel>("TfConfiguration", "pal_I20");
    EL<TPanel>("TfConfiguration", "pal_I30");
    EL<TPanel>("TfConfiguration", "pal_I40");
    EL<TPanel>("TfConfiguration", "pal_L1");
    EL<TPanel>("TfConfiguration", "pal_L2");
    EL<TPanel>("TfConfiguration", "pal_L3");
    EL<TPanel>("TfConfiguration", "pal_M");
    EL<TPanel>("TfConfiguration", "pal_N");
    EL<TPanel>("TfConfiguration", "pal_O1");
    EL<TPanel>("TfConfiguration", "pal_O2");
    EL<TPanel>("TfConfiguration", "pal_P0");
    EL<TPanel>("TfConfiguration", "pal_P1");
    EL<TPanel>("TfConfiguration", "pal_P2");
    EL<TPanel>("TfConfiguration", "pal_P3");
    EL<TPageControl>("TfConfiguration", "pcA00");
    EL<TPageControl>("TfConfiguration", "pcB00");
    EL<TPageControl>("TfConfiguration", "pcConfig");
    EL<TPageControl>("TfConfiguration", "pcD00");
    EL<TPageControl>("TfConfiguration", "pcE00");
    EL<TPageControl>("TfConfiguration", "pcF00");
    EL<TPageControl>("TfConfiguration", "pcL00");
    EL<TPageControl>("TfConfiguration", "pcN00");
    EL<TPageControl>("TfConfiguration", "pcN25");
    EL<TPageControl>("TfConfiguration", "pcO00");
    EL<TPageControl>("TfConfiguration", "pcP00");
    EL<TPageControl>("TfConfiguration", "pgC00");
    EL<TPageControl>("TfConfiguration", "pgI00");
    EL<TPageControl>("TfConfiguration", "pgcE31");
    EL<TPageControl>("TfConfiguration", "pgcN10_1");
    EL<TPageControl>("TfConfiguration", "pgcN14_1");
    EL<TPageControl>("TfConfiguration", "pgcN23");
    EL<TPageControl>("TfConfiguration", "pgcScale");
    EL<TPanel>("TfConfiguration", "plP62");
    EL<TPanel>("TfConfiguration", "plP65");
    EL<TPanel>("TfConfiguration", "pnlA12");
    EL<TPanel>("TfConfiguration", "pnlD41");
    EL<TPanel>("TfConfiguration", "pnlD63");
    EL<TPanel>("TfConfiguration", "pnlE30");
    EL<TPanel>("TfConfiguration", "pnlE30_Cold");
    EL<TPanel>("TfConfiguration", "pnlE30_Hot");
    EL<TPanel>("TfConfiguration", "pnlE31");
    EL<TPanel>("TfConfiguration", "pnlE31_2");
    EL<TPanel>("TfConfiguration", "pnlE31_Cold");
    EL<TPanel>("TfConfiguration", "pnlE31_Hot");
    EL<TPanel>("TfConfiguration", "pnlE32");
    EL<TPanel>("TfConfiguration", "pnlE32_3");
    EL<TPanel>("TfConfiguration", "pnlE32_Cold");
    EL<TPanel>("TfConfiguration", "pnlE32_Hot");
    EL<TPanel>("TfConfiguration", "pnlF14");
    EL<TPanel>("TfConfiguration", "pnlHP");
    EL<TPanel>("TfConfiguration", "pnlI17");
    EL<TPanel>("TfConfiguration", "pnlN07_2");
    EL<TPanel>("TfConfiguration", "pnlN14_15");
    EL<TPanel>("TfConfiguration", "pnlN14_17");
    EL<TPanel>("TfConfiguration", "pnlN14_18");
    EL<TPanel>("TfConfiguration", "pnlN14_19");
    EL<TPanel>("TfConfiguration", "pnlN14_3");
    EL<TPanel>("TfConfiguration", "pnlN22");
    EL<TPanel>("TfConfiguration", "pnlP13");
    EL<TPanel>("TfConfiguration", "pnlP16");
    EL<TPanel>("TfConfiguration", "pnlSoftSpeed");
    EL<TPanel>("TfConfiguration", "pnlTray");
    EL<TPanel>("TfConfiguration", "pnlsearch");
    EL<TGroupBox>("TfConfiguration", "rgbN23_2DSorting");
    EL<TTabSheet>("TfConfiguration", "tsA00");
    EL<TTabSheet>("TfConfiguration", "tsA03");
    EL<TTabSheet>("TfConfiguration", "tsA06");
    EL<TTabSheet>("TfConfiguration", "tsA_00");
    EL<TTabSheet>("TfConfiguration", "tsA_01");
    EL<TTabSheet>("TfConfiguration", "tsA_03");
    EL<TTabSheet>("TfConfiguration", "tsA_05");
    EL<TTabSheet>("TfConfiguration", "tsB11_15");
    EL<TTabSheet>("TfConfiguration", "tsB_00");
    EL<TTabSheet>("TfConfiguration", "tsC00");
    EL<TTabSheet>("TfConfiguration", "tsC21");
    EL<TTabSheet>("TfConfiguration", "tsC_00");
    EL<TTabSheet>("TfConfiguration", "tsC_10");
    EL<TTabSheet>("TfConfiguration", "tsConfig");
    EL<TTabSheet>("TfConfiguration", "tsD00");
    EL<TTabSheet>("TfConfiguration", "tsD_50");
    EL<TTabSheet>("TfConfiguration", "tsD_60");
    EL<TTabSheet>("TfConfiguration", "tsD_70");
    EL<TTabSheet>("TfConfiguration", "tsD_80");
    EL<TTabSheet>("TfConfiguration", "tsD_Contact");
    EL<TTabSheet>("TfConfiguration", "tsD_HighCal");
    EL<TTabSheet>("TfConfiguration", "tsD_Mode");
    EL<TTabSheet>("TfConfiguration", "tsD_Other");
    EL<TTabSheet>("TfConfiguration", "tsD_Preasure");
    EL<TTabSheet>("TfConfiguration", "tsE00");
    EL<TTabSheet>("TfConfiguration", "tsE30");
    EL<TTabSheet>("TfConfiguration", "tsE31");
    EL<TTabSheet>("TfConfiguration", "tsE31_1");
    EL<TTabSheet>("TfConfiguration", "tsE31_Cold");
    EL<TTabSheet>("TfConfiguration", "tsE31_Hot");
    EL<TTabSheet>("TfConfiguration", "tsE32");
    EL<TTabSheet>("TfConfiguration", "tsE50");
    EL<TTabSheet>("TfConfiguration", "tsE70");
    EL<TTabSheet>("TfConfiguration", "tsE_Other");
    EL<TTabSheet>("TfConfiguration", "tsE_XYScale");
    EL<TTabSheet>("TfConfiguration", "tsF00");
    EL<TTabSheet>("TfConfiguration", "tsF01");
    EL<TTabSheet>("TfConfiguration", "tsF21");
    EL<TTabSheet>("TfConfiguration", "tsF31");
    EL<TTabSheet>("TfConfiguration", "tsG00");
    EL<TTabSheet>("TfConfiguration", "tsHPData");
    EL<TTabSheet>("TfConfiguration", "tsI00");
    EL<TTabSheet>("TfConfiguration", "tsI_01");
    EL<TTabSheet>("TfConfiguration", "tsI_20");
    EL<TTabSheet>("TfConfiguration", "tsI_30");
    EL<TTabSheet>("TfConfiguration", "tsI_40");
    EL<TTabSheet>("TfConfiguration", "tsI_50");
    EL<TTabSheet>("TfConfiguration", "tsL00");
    EL<TTabSheet>("TfConfiguration", "tsL_00");
    EL<TTabSheet>("TfConfiguration", "tsL_10");
    EL<TTabSheet>("TfConfiguration", "tsL_20");
    EL<TTabSheet>("TfConfiguration", "tsL_30");
    EL<TTabSheet>("TfConfiguration", "tsL_30_1");
    EL<TTabSheet>("TfConfiguration", "tsL_30_2");
    EL<TTabSheet>("TfConfiguration", "tsM00");
    EL<TTabSheet>("TfConfiguration", "tsN00");
    EL<TTabSheet>("TfConfiguration", "tsN05");
    EL<TTabSheet>("TfConfiguration", "tsN06");
    EL<TTabSheet>("TfConfiguration", "tsN07");
    EL<TTabSheet>("TfConfiguration", "tsN08");
    EL<TTabSheet>("TfConfiguration", "tsN09");
    EL<TTabSheet>("TfConfiguration", "tsN10");
    EL<TTabSheet>("TfConfiguration", "tsN10_11_20");
    EL<TTabSheet>("TfConfiguration", "tsN10_1_10");
    EL<TTabSheet>("TfConfiguration", "tsN11");
    EL<TTabSheet>("TfConfiguration", "tsN12");
    EL<TTabSheet>("TfConfiguration", "tsN13");
    EL<TTabSheet>("TfConfiguration", "tsN14");
    EL<TTabSheet>("TfConfiguration", "tsN14_1");
    EL<TTabSheet>("TfConfiguration", "tsN14_2");
    EL<TTabSheet>("TfConfiguration", "tsN14_23");
    EL<TTabSheet>("TfConfiguration", "tsN14_3");
    EL<TTabSheet>("TfConfiguration", "tsN14_4");
    EL<TTabSheet>("TfConfiguration", "tsN14_5");
    EL<TTabSheet>("TfConfiguration", "tsN15");
    EL<TTabSheet>("TfConfiguration", "tsN16");
    EL<TTabSheet>("TfConfiguration", "tsN17");
    EL<TTabSheet>("TfConfiguration", "tsN21");
    EL<TTabSheet>("TfConfiguration", "tsN22");
    EL<TTabSheet>("TfConfiguration", "tsN23");
    EL<TTabSheet>("TfConfiguration", "tsN23_0");
    EL<TTabSheet>("TfConfiguration", "tsN23_1");
    EL<TTabSheet>("TfConfiguration", "tsN23_2");
    EL<TTabSheet>("TfConfiguration", "tsN23_3");
    EL<TTabSheet>("TfConfiguration", "tsN24");
    EL<TTabSheet>("TfConfiguration", "tsN25");
    EL<TTabSheet>("TfConfiguration", "tsN25_1_3");
    EL<TTabSheet>("TfConfiguration", "tsN25_4");
    EL<TTabSheet>("TfConfiguration", "tsN26");
    EL<TTabSheet>("TfConfiguration", "tsN27");
    EL<TTabSheet>("TfConfiguration", "tsN28");
    EL<TTabSheet>("TfConfiguration", "tsN29");
    EL<TTabSheet>("TfConfiguration", "tsN30");
    EL<TTabSheet>("TfConfiguration", "tsN31");
    EL<TTabSheet>("TfConfiguration", "tsN32");
    EL<TTabSheet>("TfConfiguration", "tsN33");
    EL<TTabSheet>("TfConfiguration", "tsN34");
    EL<TTabSheet>("TfConfiguration", "tsN35");
    EL<TTabSheet>("TfConfiguration", "tsN40");
    EL<TTabSheet>("TfConfiguration", "tsO00");
    EL<TTabSheet>("TfConfiguration", "tsO21");
    EL<TTabSheet>("TfConfiguration", "tsO_00");
    EL<TTabSheet>("TfConfiguration", "tsO_11");
    EL<TTabSheet>("TfConfiguration", "tsP00");
    EL<TTabSheet>("TfConfiguration", "tsP20");
    EL<TTabSheet>("TfConfiguration", "tsP30");
    EL<TTabSheet>("TfConfiguration", "tsP50");
    EL<TTabSheet>("TfConfiguration", "tsP_00");
    EL<TTabSheet>("TfConfiguration", "tsP_10");
    EL<TTabSheet>("TfConfiguration", "tsSearchFunction");
    EL<TTabSheet>("TfConfiguration", "tsSoftSimu");
    EL<TTabSheet>("TfConfiguration", "tsTempComm");
    EL<TTabSheet>("TfConfiguration", "tsTrayData");
    EL<TTabSheet>("TfConfiguration", "tsb00");
    filerw::ELSetParents("TfConfiguration", kIC_ParentOf, (int)(sizeof(kIC_ParentOf) / sizeof(kIC_ParentOf[0])));
}

// golden cConfiguration.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void IC_DfmState()
{
    EL<TButton>("TfConfiguration", "btnA77RMSSetting")->Visible = false;
    EL<TButton>("TfConfiguration", "btnMesSystem")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA10_3")->Visible = false;
    EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode")->Visible = false;
    EL<TComboBox>("TfConfiguration", "cbA10_6_HANA_ARTMode")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA21")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA23")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA24")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA35")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA36")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA37")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA38")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA51")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA58")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA61")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA65")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA66")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA67")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA68")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA69")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA72")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA73")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA74")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA75")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA76")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA77")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbA78")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbC02")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbC07")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbC09")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbC16")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbD24")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbD67")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbD70")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbD72")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbD73")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbF09")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI28")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI31_1")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI31_2")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI31_3")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI32")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI43")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbI45")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL11_6")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbL11_7")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbL11_8")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbL34_1")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL35_1")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL37")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL38")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL42")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL44")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL45")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbL46")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbN07_EnableEmployeeCheak")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsLotCheck")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "cbO10_EventLogSaver")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP06")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP07")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP08")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP09")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP10")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "cbP11")->Enabled = false;
    EL<TCheckBox>("TfConfiguration", "chkA50")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "chkN07_6")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "chkN07_7")->Visible = false;
    EL<TCheckBox>("TfConfiguration", "chkO17")->Visible = false;
    EL<filerw::ELDateTimePicker>("TfConfiguration", "dtO06_LastDate")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edA25_1")->Visible = false;
    EL<TEdit>("TfConfiguration", "edA25_2")->Visible = false;
    EL<TEdit>("TfConfiguration", "edC09")->Visible = false;
    EL<TEdit>("TfConfiguration", "edD46")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edD47_3")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edD62")->Visible = false;
    EL<TEdit>("TfConfiguration", "edD68")->Visible = false;
    EL<TEdit>("TfConfiguration", "edL11_6_Continuous")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edL11_6_Outside")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edL11_7_MaxSurge")->Enabled = false;
    filerw::ELSetReadOnly("TfConfiguration", "edL32_6");
    filerw::ELSetReadOnly("TfConfiguration", "edL32_7_1");
    filerw::ELSetReadOnly("TfConfiguration", "edL32_7_2");
    filerw::ELSetReadOnly("TfConfiguration", "edL32_8");
    EL<TEdit>("TfConfiguration", "edL33_3")->Enabled = false;
    filerw::ELSetReadOnly("TfConfiguration", "edL33_3");
    EL<TEdit>("TfConfiguration", "edL33_4")->Enabled = false;
    filerw::ELSetReadOnly("TfConfiguration", "edL33_4");
    EL<TEdit>("TfConfiguration", "edL33_5")->Enabled = false;
    filerw::ELSetReadOnly("TfConfiguration", "edL33_5");
    EL<TEdit>("TfConfiguration", "edL33_6")->Enabled = false;
    filerw::ELSetReadOnly("TfConfiguration", "edL33_6");
    EL<TEdit>("TfConfiguration", "edL40")->Visible = false;
    EL<TEdit>("TfConfiguration", "edL41")->Visible = false;
    EL<TEdit>("TfConfiguration", "edN06_FileName")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtC17")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtL36_1")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edtL36_1")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtL36_2")->Enabled = false;
    EL<TEdit>("TfConfiguration", "edtL36_2")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtL42")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtN14_6_1")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtN14_6_2")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtN14_6_3")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtN14_6_4")->Visible = false;
    EL<TEdit>("TfConfiguration", "edtN29")->Enabled = false;
    EL<TGroupBox>("TfConfiguration", "gbC06")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "gbI21")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpA20")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpA32_1")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpF31")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpL31")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpL32")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpL33")->Visible = false;
    EL<TGroupBox>("TfConfiguration", "grpL39")->Visible = false;
    EL<TLabel>("TfConfiguration", "labD62")->Visible = false;
    EL<TLabel>("TfConfiguration", "labD68_1")->Visible = false;
    EL<TLabel>("TfConfiguration", "labN06_FileName")->Visible = false;
    EL<TLabel>("TfConfiguration", "lbI21_6")->Visible = false;
    EL<TPanel>("TfConfiguration", "paLifeTime")->Visible = false;
    EL<TPageControl>("TfConfiguration", "pcN00")->Visible = false;
    EL<TSpeedButton>("TfConfiguration", "spbA27")->Visible = false;
    EL<TEdit>("TfConfiguration", "EdF12")->Text = "0.01";
    EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode")->Text = "32 Bin GS";
    EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode")->ItemIndex = 6;
    EL<TComboBox>("TfConfiguration", "cbA10_6_HANA_ARTMode")->Text = "General";
    EL<TCheckBox>("TfConfiguration", "cbD63")->Checked = true;
    EL<TCheckBox>("TfConfiguration", "cbI12")->Checked = true;
    EL<TCheckBox>("TfConfiguration", "cbManuTakeAutoFailTray_FT")->Checked = true;
    EL<TCheckBox>("TfConfiguration", "cbManuTakeAutoFailTray_RT")->Checked = true;
    EL<TCheckBox>("TfConfiguration", "cbN10_1")->Checked = true;
    EL<TCheckBox>("TfConfiguration", "cbN10_3")->Checked = true;
    EL<TRadioGroup>("TfConfiguration", "cbN31_1")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "cbO06TimePeriod")->Text = "10";
    EL<TComboBox>("TfConfiguration", "cbO06TimePeriod")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "cbbD47")->Text = "Type 2 Socket & Shuttle";
    EL<TComboBox>("TfConfiguration", "cbbD47")->ItemIndex = 1;
    EL<TComboBox>("TfConfiguration", "cbbI41")->Text = "0";
    EL<TComboBox>("TfConfiguration", "cbbI41")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "cbbN06_Mode")->Text = "ASCII - Sends data as ASCII text";
    EL<TComboBox>("TfConfiguration", "cbbN06_Mode")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "cbbO15_1")->Text = "By Lot";
    EL<TComboBox>("TfConfiguration", "coD22")->Text = "2";
    EL<TComboBox>("TfConfiguration", "coD22_3")->Text = "0x42";
    EL<TComboBox>("TfConfiguration", "coD26")->Text = "Fix Value";
    EL<TComboBox>("TfConfiguration", "coD41")->Text = "Inside socket";
    EL<TComboBox>("TfConfiguration", "coI20")->Text = "E";
    EL<TComboBox>("TfConfiguration", "coI20")->ItemIndex = 2;
    EL<TComboBox>("TfConfiguration", "coI25")->Text = "With _    (Ex. DUAL_1X2_1_2_)";
    EL<TComboBox>("TfConfiguration", "coN10DataType")->Text = "yyyy_年";
    EL<TComboBox>("TfConfiguration", "coN10DataType")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "coO19_3")->Text = "Sun.";
    EL<TComboBox>("TfConfiguration", "coO19_5")->Text = "1";
    EL<TComboBox>("TfConfiguration", "coO19_5")->ItemIndex = 0;
    EL<TComboBox>("TfConfiguration", "coP22")->Text = "Auto1";
    EL<TEdit>("TfConfiguration", "ed14_24")->Text = "2";
    EL<TEdit>("TfConfiguration", "edA01")->Text = "60";
    EL<TEdit>("TfConfiguration", "edA11")->Text = "60";
    EL<TEdit>("TfConfiguration", "edA12")->Text = "2";
    EL<TEdit>("TfConfiguration", "edA15_ESDReportTime")->Text = "2";
    EL<TEdit>("TfConfiguration", "edA39")->Text = "60";
    EL<TEdit>("TfConfiguration", "edA56_2")->Text = "500";
    EL<TEdit>("TfConfiguration", "edA56_3")->Text = "500";
    EL<TEdit>("TfConfiguration", "edA60_1")->Text = "10";
    EL<TEdit>("TfConfiguration", "edA60_2")->Text = "10";
    EL<TEdit>("TfConfiguration", "edA60_3")->Text = "10";
    EL<TEdit>("TfConfiguration", "edA60_4")->Text = "10";
    EL<TEdit>("TfConfiguration", "edA60_5")->Text = "10";
    EL<TEdit>("TfConfiguration", "edA71")->Text = "D:\\\\Run";
    EL<TEdit>("TfConfiguration", "edC05_ATC")->Text = "0";
    EL<TEdit>("TfConfiguration", "edC05_Motor")->Text = "0";
    EL<TEdit>("TfConfiguration", "edC05_Temp")->Text = "0";
    EL<TEdit>("TfConfiguration", "edC05_Vacuum")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD21_Sec")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD21_mm")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD23")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD26")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD26_3")->Text = "0";
    EL<TEdit>("TfConfiguration", "edD41")->Text = "1";
    EL<TEdit>("TfConfiguration", "edD41_2")->Text = "1";
    EL<TEdit>("TfConfiguration", "edE36")->Text = "0";
    EL<TEdit>("TfConfiguration", "edE37")->Text = "0";
    EL<TEdit>("TfConfiguration", "edE51")->Text = "100";
    EL<TEdit>("TfConfiguration", "edE52")->Text = "100";
    EL<TEdit>("TfConfiguration", "edE68")->Text = "100";
    EL<TEdit>("TfConfiguration", "edE73")->Text = "5";
    EL<TEdit>("TfConfiguration", "edF01")->Text = "100";
    EL<TEdit>("TfConfiguration", "edF13_ADC")->Text = "5";
    EL<TEdit>("TfConfiguration", "edF13_Ini")->Text = "50";
    EL<TEdit>("TfConfiguration", "edF13_Jog")->Text = "1500";
    EL<TEdit>("TfConfiguration", "edF14")->Text = "0.01";
    EL<TEdit>("TfConfiguration", "edF14_1")->Text = "0.01";
    EL<TEdit>("TfConfiguration", "edF14_1_No")->Text = "3";
    EL<TEdit>("TfConfiguration", "edF14_No")->Text = "3";
    EL<TEdit>("TfConfiguration", "edF23")->Text = "20";
    EL<TEdit>("TfConfiguration", "edF25")->Text = "30";
    EL<TEdit>("TfConfiguration", "edF31_1")->Text = "1000000";
    EL<TEdit>("TfConfiguration", "edF31_2")->Text = "1000000";
    EL<TEdit>("TfConfiguration", "edF31_3")->Text = "30";
    EL<TEdit>("TfConfiguration", "edF31_4")->Text = "30";
    EL<TEdit>("TfConfiguration", "edF31_5")->Text = "30";
    EL<TEdit>("TfConfiguration", "edF31_6")->Text = "30";
    EL<TEdit>("TfConfiguration", "edF32")->Text = "10";
    EL<TEdit>("TfConfiguration", "edF37_Delay")->Text = "500";
    EL<TEdit>("TfConfiguration", "edG14")->Text = "1";
    EL<TEdit>("TfConfiguration", "edI21")->Text = "120";
    EL<TEdit>("TfConfiguration", "edI21_1")->Text = "3";
    EL<TEdit>("TfConfiguration", "edI21_6")->Text = "3";
    EL<TEdit>("TfConfiguration", "edI21_9")->Text = "0";
    EL<TEdit>("TfConfiguration", "edI22_1")->Text = "0";
    EL<TEdit>("TfConfiguration", "edI29")->Text = "120";
    EL<TEdit>("TfConfiguration", "edI29_3")->Text = "100";
    EL<TEdit>("TfConfiguration", "edI44")->Text = "60";
    EL<TEdit>("TfConfiguration", "edI49")->Text = "100";
    EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL11_1_2")->Text = "20.0";
    EL<TEdit>("TfConfiguration", "edL11_2")->Text = "20";
    EL<TEdit>("TfConfiguration", "edL11_4")->Text = "130";
    EL<TEdit>("TfConfiguration", "edL11_6_Continuous")->Text = "3";
    EL<TEdit>("TfConfiguration", "edL11_6_Outside")->Text = "2";
    EL<TEdit>("TfConfiguration", "edL11_7_MaxSurge")->Text = "7";
    EL<TEdit>("TfConfiguration", "edL32_4")->Text = "80";
    EL<TEdit>("TfConfiguration", "edL32_5")->Text = "20";
    EL<TEdit>("TfConfiguration", "edL32_6")->Text = "10";
    EL<TEdit>("TfConfiguration", "edL32_7_1")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL32_7_2")->Text = "1";
    EL<TEdit>("TfConfiguration", "edL32_8")->Text = "10";
    EL<TEdit>("TfConfiguration", "edL33_3")->Text = "5";
    EL<TEdit>("TfConfiguration", "edL33_4")->Text = "5";
    EL<TEdit>("TfConfiguration", "edL33_5")->Text = "25";
    EL<TEdit>("TfConfiguration", "edL33_6")->Text = "131";
    EL<TEdit>("TfConfiguration", "edL34_2")->Text = "30";
    EL<TEdit>("TfConfiguration", "edL34_3")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL34_4")->Text = "0";
    EL<TEdit>("TfConfiguration", "edL35_2")->Text = "130";
    EL<TEdit>("TfConfiguration", "edL40")->Text = "7";
    EL<TEdit>("TfConfiguration", "edL41")->Text = "3";
    EL<TEdit>("TfConfiguration", "edN04_ID")->Text = "HT10";
    EL<TEdit>("TfConfiguration", "edN04_Model")->Text = "HT9045W";
    EL<TEdit>("TfConfiguration", "edN05_1")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Text = "40";
    EL<TEdit>("TfConfiguration", "edN05_DownPath")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN06_1")->Text = "edN06_1";
    EL<TEdit>("TfConfiguration", "edN07_2")->Text = "60";
    EL<TEdit>("TfConfiguration", "edN07_5")->Text = "60";
    EL<TEdit>("TfConfiguration", "edN14_20")->Text = "\\DefaultRecipeChangeLog\\";
    EL<TEdit>("TfConfiguration", "edN14_21")->Text = "/handler_data/SETUP_FILE";
    EL<TEdit>("TfConfiguration", "edN14_21_BC")->Text = "/handler_data/SETUP_FILE";
    EL<TEdit>("TfConfiguration", "edN14_21_HP")->Text = "/handler_data/SETUP_FILE";
    EL<TEdit>("TfConfiguration", "edN14_21_TF")->Text = "/handler_data/SETUP_FILE";
    EL<TEdit>("TfConfiguration", "edN17_2")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN17_4")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN21_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN21_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN21_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN21_4")->Text = "\\\\Handler\\\\InstallForAll\\\\";
    EL<TEdit>("TfConfiguration", "edN23_3NetDrivePath")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edN27_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN27_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN27_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN27_4")->Text = "\\\\Handler\\\\EventLogXml\\\\";
    EL<TEdit>("TfConfiguration", "edN27_6")->Text = "TesterID";
    EL<TEdit>("TfConfiguration", "edN30_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN30_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN30_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN30_4")->Text = "\\\\Handler\\\\GroundESDLog\\\\";
    EL<TEdit>("TfConfiguration", "edN30_5")->Text = "\\\\Handler\\\\GroundESDLog2\\\\";
    EL<TEdit>("TfConfiguration", "edN31_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN31_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN31_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN31_4")->Text = "\\\\Handler\\\\AutoTempOfs\\\\";
    EL<TEdit>("TfConfiguration", "edN31_5")->Text = "10";
    EL<TEdit>("TfConfiguration", "edN31_MaxOffset")->Text = "10";
    EL<TEdit>("TfConfiguration", "edN31_MinOffset")->Text = "10";
    EL<TEdit>("TfConfiguration", "edN32_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN32_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN32_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN32_4")->Text = "\\\\Handler\\\\Install\\\\";
    EL<TEdit>("TfConfiguration", "edN32_5")->Text = "D:\\\\RMS\\\\";
    EL<TEdit>("TfConfiguration", "edN32_6")->Text = "\\\\Handler\\\\InstallForAll\\\\";
    EL<TEdit>("TfConfiguration", "edN35_1")->Text = "SGPHANDLER";
    EL<TEdit>("TfConfiguration", "edN35_2")->Text = "CHROMA";
    EL<TEdit>("TfConfiguration", "edN35_3")->Text = "192.168.119.243";
    EL<TEdit>("TfConfiguration", "edN35_4")->Text = "\\\\HT_JAM\\\\HT-9046HA-8H\\\\";
    EL<TEdit>("TfConfiguration", "edN40_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edN40_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edN40_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edN40_4")->Text = "\\\\Handler\\\\GroundESDLog\\\\";
    EL<TEdit>("TfConfiguration", "edN41_1")->Text = "\\\\Handler\\\\GroundESDLog\\\\";
    EL<TEdit>("TfConfiguration", "edO06_FilePath")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edO07")->Text = "0";
    EL<TEdit>("TfConfiguration", "edP13_1")->Text = "20";
    EL<TEdit>("TfConfiguration", "edP13_2")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP14_1")->Text = "5";
    EL<TEdit>("TfConfiguration", "edP14_2")->Text = "5";
    EL<TEdit>("TfConfiguration", "edP16_1")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP16_2")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP23_1")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP23_2")->Text = "10";
    EL<TEdit>("TfConfiguration", "edP29")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP30")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP48DelayTime")->Text = "1";
    EL<TEdit>("TfConfiguration", "edP48Times")->Text = "1";
    EL<TEdit>("TfConfiguration", "editN05_RTC")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edtD17_3")->Text = "1";
    EL<TEdit>("TfConfiguration", "edtL09_1")->Text = "15";
    EL<TEdit>("TfConfiguration", "edtL09_2")->Text = "-40";
    EL<TEdit>("TfConfiguration", "edtL36_1")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtL36_2")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtL39_2")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtL42")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtL44")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtL45")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtN07_7")->Text = "60";
    EL<TEdit>("TfConfiguration", "edtN09_7")->Text = "172,192";
    EL<TEdit>("TfConfiguration", "edtN10_6")->Text = "60";
    EL<TEdit>("TfConfiguration", "edtN10_8")->Text = "D:\\RMS";
    EL<TLabeledEdit>("TfConfiguration", "edtN10_Port")->Text = "21";
    EL<TEdit>("TfConfiguration", "edtN14_12")->Text = "1";
    EL<TEdit>("TfConfiguration", "edtN14_13")->Text = "1";
    EL<TEdit>("TfConfiguration", "edtN14_14_1")->Text = "C:\\GTK\\EMG.exe";
    EL<TEdit>("TfConfiguration", "edtN14_14_2")->Text = "C:\\GTK_Control\\Message\\Alarm\\";
    EL<TEdit>("TfConfiguration", "edtN14_14_3")->Text = "C:\\GTK_Control\\Flag\\";
    EL<TEdit>("TfConfiguration", "edtN14_15_1")->Text = "C:\\GTK\\Lifetime_count.exe";
    EL<TEdit>("TfConfiguration", "edtN14_15_2")->Text = "C:\\GTK_Control\\Message\\count\\";
    EL<TEdit>("TfConfiguration", "edtN14_16_1")->Text = "C:\\GTK\\IPSC.exe";
    EL<TEdit>("TfConfiguration", "edtN14_16_2")->Text = "C:\\GTK_Control\\Flag\\IPSC.ini";
    EL<TEdit>("TfConfiguration", "edtN14_16_3")->Text = "C:\\GTK_CONTROL\\MESSAGE\\";
    EL<TEdit>("TfConfiguration", "edtN14_16_4")->Text = "2";
    EL<TEdit>("TfConfiguration", "edtN14_17")->Text = "39";
    EL<TEdit>("TfConfiguration", "edtN14_18")->Text = "\\Handler_data\\Taj_offset_Tool\\";
    EL<TEdit>("TfConfiguration", "edtN14_19")->Text = "\\TrayMapping\\";
    EL<TEdit>("TfConfiguration", "edtN23_4")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edtN25_1_Host")->Text = "10.20.50.3";
    EL<TEdit>("TfConfiguration", "edtN25_1_Name")->Text = "handler";
    EL<TEdit>("TfConfiguration", "edtN25_1_Password")->Text = "handler";
    EL<TEdit>("TfConfiguration", "edtN25_1_Path")->Text = "/summary/Handler/TesterStatus/";
    EL<TEdit>("TfConfiguration", "edtN25_2_Host")->Text = "10.20.50.3";
    EL<TEdit>("TfConfiguration", "edtN25_2_Name")->Text = "handler";
    EL<TEdit>("TfConfiguration", "edtN25_2_Password")->Text = "handler";
    EL<TEdit>("TfConfiguration", "edtN25_2_Path")->Text = "/Summary/naslfs2/Handler/";
    EL<TEdit>("TfConfiguration", "edtN25_3_LogJamPath")->Text = "/Summary/naslfs2/Handler/";
    EL<TEdit>("TfConfiguration", "edtN25_4_UploadPath")->Text = "/Summary/naslfs2/Handler/";
    EL<TEdit>("TfConfiguration", "edtN25_5_UploadPath")->Text = "/Summary/naslfs2/Handler/";
    EL<TEdit>("TfConfiguration", "edtN26_1")->Text = "UnKnown";
    EL<TEdit>("TfConfiguration", "edtN26_2")->Text = "1234";
    EL<TEdit>("TfConfiguration", "edtN26_3")->Text = "127.0.0.1";
    EL<TEdit>("TfConfiguration", "edtN26_4")->Text = "\\\\Handler\\\\temperature\\\\";
    EL<TEdit>("TfConfiguration", "edtO06AlarmHistroy")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edtO06AlarmStatist")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edtO06Production")->Text = "D:\\RMS";
    EL<TEdit>("TfConfiguration", "edtO06_Local")->Text = "\\\\steven-pc\\自由軟體";
    EL<TEdit>("TfConfiguration", "edtO06_Remote")->Text = "Z:";
    EL<TEdit>("TfConfiguration", "edtO11")->Text = "0";
    EL<TEdit>("TfConfiguration", "edtO16")->Text = "3";
    EL<TEdit>("TfConfiguration", "edtO17_Count")->Text = "5";
    EL<TEdit>("TfConfiguration", "edtO17_Time")->Text = "30";
    EL<TEdit>("TfConfiguration", "edtO18")->Text = "168";
    EL<TEdit>("TfConfiguration", "edtO19_6")->Text = "D:\\HT9045_Log\\OneDayProduct";
    EL<TEdit>("TfConfiguration", "edtP65")->Text = "1";
    EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->ItemIndex = 0;
    EL<TLabeledEdit>("TfConfiguration", "lbledtN07_6")->Text = "Z:\\";
    EL<TLabeledEdit>("TfConfiguration", "lbledtN23_4_URL")->Text = "D:\\RMS";
    EL<TLabeledEdit>("TfConfiguration", "lbledtN24")->Text = "6000";
    EL<TRadioGroup>("TfConfiguration", "rgC05")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgD17")->ItemIndex = 2;
    EL<TRadioGroup>("TfConfiguration", "rgD69")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgD71")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgE50")->ItemIndex = 1;
    EL<TRadioGroup>("TfConfiguration", "rgF07")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgF26")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgI22")->ItemIndex = 1;
    EL<TRadioGroup>("TfConfiguration", "rgI38")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgI46")->ItemIndex = 1;
    EL<TRadioGroup>("TfConfiguration", "rgL10")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN09_4")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN10_3_1")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN10_4")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN14_20_1")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN23_1_2DSorting")->ItemIndex = 1;
    EL<TRadioGroup>("TfConfiguration", "rgN32")->ItemIndex = 0;
    EL<TRadioGroup>("TfConfiguration", "rgN35_1")->ItemIndex = 0;
    EL<TRadioButton>("TfConfiguration", "rgP23_OCRByNewTray")->Checked = true;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->OnChange = &IC_tbD25_Index30mmChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->OnChange = &IC_tbD25_Index30mm_NSChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->OnChange = &IC_tbD25_Index40mmChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->OnChange = &IC_tbD25_Index40mm_NSChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->OnChange = &IC_tbD25_Index60mmChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->DfmInit(80, 115, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->OnChange = &IC_tbD25_Index60mm_NSChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->DfmInit(80, 150, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->OnChange = &IC_tbD60_Index56mmChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->DfmInit(80, 150, 80);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->OnChange = &IC_tbD60_Index56mm_NSChange;
    EL<filerw::ELTrackBar>("TfConfiguration", "udD46")->DfmInit(5, 15, 5);
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kIC_EnableAll[] = {
    "G18", "GroupBox1", "GroupBox10", "GroupBox11", "GroupBox12", "GroupBox13",
    "GroupBox14", "GroupBox15", "GroupBox18", "GroupBox19", "GroupBox2", "GroupBox20",
    "GroupBox21", "GroupBox22", "GroupBox23", "GroupBox24", "GroupBox25", "GroupBox29",
    "GroupBox3", "GroupBox30", "GroupBox31", "GroupBox32", "GroupBox33", "GroupBox34",
    "GroupBox35", "GroupBox36", "GroupBox37", "GroupBox38", "GroupBox39", "GroupBox40",
    "GroupBox41", "GroupBox42", "GroupBox43", "GroupBox44", "GroupBox5", "GroupBox6",
    "GroupBox7", "GroupBox8", "GroupBox9", "PageControl2", "Panel1", "Panel3",
    "Panel4", "Panel5", "Panel6", "Panel7", "TabSheet2", "TabSheet3",
    "TabSheet4", "TabSheet5", "bgC05_1", "btD47", "btN06_TesterList", "btN06_TesterMap",
    "btN06_UpdateTesterList", "btResume", "btnA77RMSSetting", "btnMesSystem", "btnOpenEP", "btnSave",
    "btnUploadAll", "cb14_23", "cb14_24", "cbA01", "cbA01_1", "cbA01_2",
    "cbA02", "cbA03", "cbA04", "cbA05", "cbA08", "cbA09",
    "cbA10", "cbA10_3", "cbA10_5", "cbA10_6", "cbA10_7", "cbA11",
    "cbA12", "cbA14", "cbA15", "cbA15_1", "cbA17_1", "cbA17_2",
    "cbA19", "cbA20_1", "cbA20_2", "cbA20_3", "cbA20_4", "cbA20_5",
    "cbA21", "cbA22_1", "cbA23", "cbA24", "cbA26", "cbA27",
    "cbA27_1", "cbA29", "cbA30", "cbA31", "cbA31_1", "cbA32",
    "cbA32_01", "cbA32_02", "cbA32_03", "cbA32_04", "cbA32_05", "cbA32_06",
    "cbA32_07", "cbA32_08", "cbA32_09", "cbA32_10", "cbA32_2", "cbA32_3",
    "cbA33", "cbA35", "cbA36", "cbA37", "cbA38", "cbA39",
    "cbA40", "cbA51", "cbA55", "cbA56_1", "cbA57_1", "cbA57_2",
    "cbA57_3", "cbA58", "cbA60_1", "cbA61", "cbA62", "cbA65",
    "cbA66", "cbA67", "cbA68", "cbA69", "cbA71", "cbA72",
    "cbA73", "cbA74", "cbA75", "cbA76", "cbA77", "cbA78",
    "cbA81WaitSECS", "cbB01", "cbB02", "cbB03", "cbB05", "cbB11Enable",
    "cbB12Enable", "cbC01", "cbC02", "cbC03", "cbC04", "cbC05_ATC",
    "cbC05_Motor", "cbC05_Temp", "cbC05_Vacuum", "cbC06_01", "cbC06_02", "cbC06_03",
    "cbC06_04", "cbC06_05", "cbC06_06", "cbC06_07", "cbC06_08", "cbC06_09",
    "cbC06_10", "cbC06_11", "cbC06_12", "cbC07", "cbC08", "cbC08_1",
    "cbC09", "cbC10", "cbC11", "cbC12", "cbC13", "cbC16",
    "cbC17", "cbC17_1", "cbC24", "cbD01", "cbD01_1", "cbD02",
    "cbD04", "cbD05", "cbD05_1", "cbD06", "cbD10", "cbD11",
    "cbD12", "cbD14", "cbD15", "cbD16", "cbD21", "cbD22_1",
    "cbD22_2", "cbD22_3", "cbD22_3_", "cbD22_4", "cbD23", "cbD24",
    "cbD26", "cbD26_1", "cbD26_2", "cbD26_3", "cbD27", "cbD28",
    "cbD29", "cbD30", "cbD31", "cbD32", "cbD33", "cbD34",
    "cbD35", "cbD36", "cbD36_1", "cbD36_2", "cbD37", "cbD38",
    "cbD40", "cbD41", "cbD42", "cbD43", "cbD43_1", "cbD43_2",
    "cbD44", "cbD45", "cbD47", "cbD48", "cbD49", "cbD50",
    "cbD51", "cbD52", "cbD53", "cbD54", "cbD55", "cbD56",
    "cbD57", "cbD58", "cbD59", "cbD61", "cbD62", "cbD63",
    "cbD63_1", "cbD64", "cbD65", "cbD66", "cbD67", "cbD70",
    "cbD72", "cbD73", "cbD74", "cbD75", "cbD78", "cbD79",
    "cbD80", "cbD81", "cbD82", "cbD83", "cbE30", "cbE30_1",
    "cbE30_2", "cbE31", "cbE31_1", "cbE31_2", "cbE32", "cbE32_1",
    "cbE32_2", "cbE33", "cbE34", "cbE35", "cbE38", "cbE39",
    "cbE39_1", "cbE40", "cbE41", "cbE42", "cbE43", "cbE44",
    "cbE45", "cbE46", "cbE47", "cbE48", "cbE49", "cbE51",
    "cbE53", "cbE54", "cbE55", "cbE56", "cbE57", "cbE58",
    "cbE59", "cbE60", "cbE61", "cbE62", "cbE63", "cbE64",
    "cbE68", "cbE79", "cbE85_Auto1", "cbE85_Auto3", "cbE85_Enable", "cbE86",
    "cbE87", "cbE88", "cbE89", "cbE90", "cbF01", "cbF03",
    "cbF05", "cbF06", "cbF09", "cbF11", "cbF12", "cbF14",
    "cbF14_1", "cbF15", "cbF16", "cbF17", "cbF18", "cbF19",
    "cbF20", "cbF20Label2", "cbF21", "cbF22", "cbF23", "cbF24",
    "cbF25", "cbF28", "cbF29", "cbF30", "cbF31", "cbF32",
    "cbF33_Check2DHardware", "cbF34", "cbF35", "cbF36", "cbF37", "cbG01",
    "cbG04", "cbG05", "cbG06", "cbG07", "cbG08", "cbG09",
    "cbG10", "cbG11", "cbG12", "cbG13", "cbG14", "cbG15",
    "cbG17", "cbG17_1", "cbG18_1", "cbG18_2", "cbG19", "cbG20",
    "cbG21", "cbG22", "cbG23", "cbG24", "cbI01", "cbI02",
    "cbI03", "cbI04", "cbI05", "cbI06", "cbI07", "cbI08",
    "cbI09", "cbI12", "cbI13", "cbI16", "cbI17", "cbI18",
    "cbI19", "cbI21", "cbI21_10", "cbI21_5", "cbI21_6", "cbI21_7",
    "cbI21_8", "cbI21_9", "cbI21_CheckOpen", "cbI21_SameSoakTime", "cbI21_SkipSoakTime", "cbI22",
    "cbI23", "cbI24", "cbI26", "cbI27", "cbI28", "cbI29",
    "cbI29_3", "cbI30", "cbI31_1", "cbI31_2", "cbI31_3", "cbI32",
    "cbI33", "cbI34", "cbI35", "cbI36", "cbI37_1", "cbI37_2",
    "cbI37_3", "cbI39", "cbI40", "cbI41", "cbI41_1", "cbI41_2",
    "cbI41_3", "cbI41_4", "cbI41_5", "cbI41_6", "cbI42", "cbI43",
    "cbI45", "cbI49", "cbI50_EnableASM_Trigger", "cbI50_InitialStart", "cbI50_OnyCycle", "cbI50_Pause",
    "cbI50_RT", "cbI50_StartLot", "cbI50_TrayFeed", "cbI51", "cbI52", "cbI53",
    "cbI54_1", "cbI54_2", "cbI54_Enable", "cbL03", "cbL07", "cbL09",
    "cbL10", "cbL11_1", "cbL11_2", "cbL11_6", "cbL11_7", "cbL11_8",
    "cbL12", "cbL13", "cbL15", "cbL17", "cbL18", "cbL19",
    "cbL20", "cbL21", "cbL22", "cbL24", "cbL25", "cbL28",
    "cbL29", "cbL30", "cbL30_1", "cbL32_1", "cbL32_2", "cbL33_1",
    "cbL33_2", "cbL34_1", "cbL34_5", "cbL35_1", "cbL37", "cbL38",
    "cbL39_1", "cbL39_2", "cbL42", "cbL43", "cbL44", "cbL45",
    "cbL46", "cbL49", "cbM01", "cbM01_01", "cbM01_02", "cbM01_03",
    "cbM01_04", "cbM01_05", "cbM01_06", "cbM01_07", "cbM01_08", "cbM01_09",
    "cbM01_10", "cbM01_12", "cbM01_13", "cbM01_14", "cbManuTakeAutoFailTray_FT", "cbManuTakeAutoFailTray_RT",
    "cbN05_1", "cbN05_CheckFile", "cbN05_CheckSetupFile", "cbN05_EnableRMS", "cbN05_SCKWebService", "cbN06_1",
    "cbN06_EnableFTP", "cbN06_UseBarcode", "cbN06_UseSystemCall", "cbN07_6CompressedFile", "cbN07_EnableEmployeeCheak", "cbN07_EnableHostStart",
    "cbN07_EnableSecs", "cbN07_EnableSecsLotCheck", "cbN07_EnableSecsOneCycle", "cbN08_1", "cbN10_1", "cbN10_11",
    "cbN10_12", "cbN10_2", "cbN10_3", "cbN11_1", "cbN13_EnableARMSFunction", "cbN14_1",
    "cbN14_10", "cbN14_11", "cbN14_12", "cbN14_13", "cbN14_14", "cbN14_15",
    "cbN14_16", "cbN14_18", "cbN14_19", "cbN14_2", "cbN14_20", "cbN14_20_1",
    "cbN14_21", "cbN14_22", "cbN14_3", "cbN14_4", "cbN14_7", "cbN14_8",
    "cbN14_9", "cbN15_1", "cbN15_2", "cbN17_1", "cbN17_3", "cbN22",
    "cbN22_EveltLog", "cbN23", "cbN27_1", "cbN30_1", "cbN31_1", "cbN32_1",
    "cbN32_2", "cbN32_3", "cbN32_4", "cbN33", "cbN33_1", "cbN34",
    "cbN35", "cbN35_1", "cbN40_1", "cbN41_1", "cbO01", "cbO02",
    "cbO05", "cbO06", "cbO07", "cbO08", "cbO09", "cbO10_EventLogSaver",
    "cbO11", "cbO12_LifeTimeCount", "cbO13_LifeTimeCount", "cbO14_LifeTimeCount", "cbO18", "cbO19_1",
    "cbO19_2", "cbO19_4", "cbO20", "cbO20_1", "cbO21", "cbO21_1",
    "cbP04", "cbP05", "cbP05_1", "cbP06", "cbP07", "cbP08",
    "cbP09", "cbP10", "cbP11", "cbP13", "cbP14", "cbP15",
    "cbP16", "cbP17", "cbP18", "cbP19", "cbP20", "cbP21",
    "cbP22", "cbP24", "cbP24_2", "cbP24_3", "cbP25", "cbP26",
    "cbP27", "cbP28", "cbP29", "cbP30", "cbP31", "cbP32",
    "cbP33", "cbP34", "cbP35", "cbP36", "cbP37", "cbP38",
    "cbP39", "cbP40", "cbP41", "cbP42", "cbP44", "cbP45",
    "cbP46", "cbP48", "cbP49", "cbP50", "cbP51", "cbP52",
    "cbP53", "cbP54", "cbP55", "cbP57", "cbP59", "cbP60",
    "cbP62", "cbP62_1", "cbP65", "cbP66", "cbkE78", "cdD13",
    "cdE52", "chA16", "checkbN05_RTC", "chk24", "chkA09_1", "chkA50",
    "chkAutoTrayFeed", "chkC14", "chkD18", "chkE43_1", "chkE65", "chkE66",
    "chkE67", "chkE69", "chkE70", "chkE71", "chkE72", "chkE73",
    "chkE74", "chkF27", "chkG16", "chkI29_1", "chkI44", "chkN05_TrayFeedClear",
    "chkN06_Tester", "chkN07_3_2", "chkN07_6", "chkN07_7", "chkN09", "chkN09_2",
    "chkN10_9", "chkN10_Passive", "chkN12", "chkN16", "chkN20", "chkN21_1",
    "chkN23_1", "chkN23_3", "chkN23_4", "chkN23_5", "chkN25_1", "chkN25_2",
    "chkN25_3", "chkN25_4", "chkN25_5", "chkN26_1", "chkN26_2", "chkN28",
    "chkN29", "chkO06AlarmHistroy", "chkO06AlarmStatist", "chkO06Production", "chkO06TimePeriod", "chkO06UseNetDrive",
    "chkO15_2", "chkO15_3", "chkO16", "chkO17", "chkO22", "chkO23",
    "chkO24", "chkO25", "chkP20_1", "chkP20_2", "chkP21_1", "chkP21_2",
    "chkP56", "gbA10", "gbA22", "gbA31", "gbA31_CleanTiming", "gbA56",
    "gbA57", "gbA71", "gbB11", "gbB12", "gbB13", "gbB14",
    "gbC05", "gbC06", "gbD25", "gbD60", "gbE30_1_HP1", "gbE30_1_HP2",
    "gbE30_1_loader", "gbE30_2_HP1", "gbE30_2_HP2", "gbE30_2_loader", "gbE30_HP1", "gbE30_HP2",
    "gbE30_loader", "gbE31_Auto1", "gbE31_Auto2", "gbE31_Auto3", "gbE31_Auto4", "gbE31_Auto5",
    "gbE31_Fix1", "gbE31_Fix2", "gbE31_Fix3", "gbE32_InSh1", "gbE32_InSh2", "gbE32_OutSh1",
    "gbE32_OutSh2", "gbE85", "gbF13_RotateShtSpeed", "gbI21", "gbI50", "gbI54",
    "gbL09", "gbL11", "gbL30", "gbM01", "gbN04", "gbN05",
    "gbN05_1", "gbN05_WebService", "gbN06_FTP", "gbN06_HddLevel", "gbN06_ServerLv", "gbN07",
    "gbN08", "gbN11", "gbN12_SocketIdProductDataUpoadToFTP", "gbN17LotSummary", "gbN17ProdLogUL", "gbN23_FTPSetting",
    "gbN26", "gbN30", "gbN31", "gbN32", "gbN33", "gbN33_1",
    "gbN34", "gbO06", "gbP23_OCR", "gbP26_OCRCheck", "gbUnloadMode", "groupbN05_RTC",
    "grpA17", "grpA20", "grpA25", "grpA28", "grpA32", "grpA32_1",
    "grpA55", "grpA60", "grpASEMNetworkDrive", "grpB01", "grpB02", "grpB03",
    "grpB05", "grpC17", "grpC21", "grpD22", "grpD47", "grpE31_Auto6",
    "grpE31_Fix4", "grpE31_Fix5", "grpE31_Fix6", "grpF31", "grpI31", "grpI37",
    "grpI41", "grpL31", "grpL32", "grpL32_1", "grpL32_2", "grpL32_3",
    "grpL33", "grpL34", "grpL35", "grpL37", "grpL39", "grpLogUploadToFTP",
    "grpN09", "grpN09_5", "grpN10_FTP", "grpN14", "grpN14_16", "grpN14_22",
    "grpN14_3", "grpN14_6", "grpN15", "grpN16", "grpN20", "grpN21",
    "grpN22", "grpN23_1", "grpN23_2", "grpN23_3", "grpN23_4", "grpN23_5",
    "grpN24", "grpN25", "grpN25_1", "grpN25_3", "grpN25_4", "grpN25_5",
    "grpN27", "grpN28", "grpN35", "grpO15", "grpO16", "grpP48",
    "labC05_2", "labC05_4", "labC05_6", "labC05_8", "labD21_1", "labD21_2",
    "labD25_1", "labD25_1_NS", "labD25_2", "labD25_2_NS", "labD25_3", "labD25_3_NS",
    "labD25_4", "labD25_5", "labD25_6", "labD41", "labD41_1", "labD41_2",
    "labD41_3", "labD47_1", "labD47_2", "labD47_3", "labD53", "labD60",
    "labD60_1", "labD60_NS", "labD62", "labD68_1", "labF05", "labF12",
    "labF37_Delay", "labL40", "labL41", "labN05_1", "labN05_AmbTemp", "labN05_DownPath",
    "labN06_DownloadPath", "labN06_FileName", "labN06_TesterList", "labN06_TesterMap", "labN06_UploadPath", "labN07_5",
    "labN31_5", "lbI21_6", "lb_N22_DownloadPath", "lblA16", "lblA60_1", "lblA60_2",
    "lblA60_3", "lblA60_4", "lblA60_5", "lblIPSCClear_Qty", "lblIPSCIn_Qty", "lblIPSCOut_Qty",
    "lblIPSC_Qty", "lblN10DataType", "lblN10_6", "lblO17", "lblP04", "lblUploadRealPath",
    "paLifeTime", "palA10_ART", "palD01", "palE30", "palE31", "palE32",
    "palE32_Cold", "palE32_Hot", "pal_A1", "pal_A2", "pal_A3", "pal_A4",
    "pal_B1", "pal_B11", "pal_C1", "pal_C2", "pal_D1", "pal_D2",
    "pal_D3", "pal_D4", "pal_D5", "pal_D6", "pal_D7", "pal_D8",
    "pal_E2", "pal_E5", "pal_E80", "pal_F00", "pal_F10", "pal_F20",
    "pal_F30", "pal_G", "pal_I", "pal_I01", "pal_I20", "pal_I30",
    "pal_I40", "pal_L1", "pal_L2", "pal_L3", "pal_M", "pal_N",
    "pal_O1", "pal_O2", "pal_P0", "pal_P1", "pal_P2", "pal_P3",
    "pcA00", "pcB00", "pcD00", "pcE00", "pcF00", "pcL00",
    "pcN00", "pcN25", "pcO00", "pcP00", "pgC00", "pgI00",
    "pgcE31", "pgcN10_1", "pgcN14_1", "pgcN23", "pgcScale", "plP62",
    "plP65", "pnlA12", "pnlD41", "pnlD63", "pnlE30", "pnlE30_Cold",
    "pnlE30_Hot", "pnlE31", "pnlE31_2", "pnlE31_Cold", "pnlE31_Hot", "pnlE32",
    "pnlE32_3", "pnlE32_Cold", "pnlE32_Hot", "pnlF14", "pnlI17", "pnlN07_2",
    "pnlN14_15", "pnlN14_17", "pnlN14_18", "pnlN14_19", "pnlN14_3", "pnlN22",
    "pnlP13", "pnlP16", "rgC05", "rgD17", "rgD69", "rgD71",
    "rgE50", "rgF07", "rgF26", "rgI22", "rgI38", "rgI46",
    "rgL10", "rgN09_4", "rgN10_3_1", "rgN10_4", "rgN14_20_1", "rgN23_1_2DSorting",
    "rgN32", "rgN35_1", "rgP23_OCRByInitialStart", "rgP23_OCRByNewTray", "rgP58RunModeAfterFT", "rgUpDlMethod",
    "rgbN23_2DSorting", "spbA27", "tsA00", "tsA03", "tsA06", "tsA_00",
    "tsA_01", "tsA_03", "tsA_05", "tsB11_15", "tsB_00", "tsC00",
    "tsC21", "tsC_00", "tsC_10", "tsD00", "tsD_50", "tsD_60",
    "tsD_70", "tsD_80", "tsD_Contact", "tsD_HighCal", "tsD_Mode", "tsD_Other",
    "tsD_Preasure", "tsE00", "tsE30", "tsE31", "tsE31_1", "tsE31_Cold",
    "tsE31_Hot", "tsE32", "tsE50", "tsE70", "tsE_Other", "tsE_XYScale",
    "tsF00", "tsF01", "tsF21", "tsF31", "tsG00", "tsI00",
    "tsI_01", "tsI_20", "tsI_30", "tsI_40", "tsI_50", "tsL00",
    "tsL_00", "tsL_10", "tsL_20", "tsL_30", "tsL_30_1", "tsL_30_2",
    "tsM00", "tsN00", "tsN05", "tsN06", "tsN07", "tsN08",
    "tsN09", "tsN10", "tsN10_11_20", "tsN10_1_10", "tsN11", "tsN12",
    "tsN13", "tsN14", "tsN14_1", "tsN14_2", "tsN14_23", "tsN14_3",
    "tsN14_4", "tsN14_5", "tsN15", "tsN16", "tsN17", "tsN21",
    "tsN22", "tsN23", "tsN23_0", "tsN23_1", "tsN23_2", "tsN23_3",
    "tsN24", "tsN25", "tsN25_1_3", "tsN25_4", "tsN26", "tsN27",
    "tsN28", "tsN29", "tsN30", "tsN31", "tsN32", "tsN33",
    "tsN34", "tsN35", "tsN40", "tsO00", "tsO21", "tsO_00",
    "tsO_11", "tsP00", "tsP20", "tsP30", "tsP50", "tsP_00",
    "tsP_10", "tsb00",
};

// 存檔流程（FormClose、CheckConfigurationBeforeSave、SaveConfiguration、LoadConfiguration、SetA73、WriteContactData）讀的替身 102 個 —— FileRW/IniConfig.cpp 用它決定頁面必須送哪些值
static const char* const kIC_SaveReads[] = {
    "cbA10_3_ARTTestMode", "cbA15", "cbA31", "cbC02", "cbC07", "cbD41",
    "cbE45", "cbI37_1", "cbN05_1", "cbN05_CheckFile", "cbN05_CheckSetupFile", "cbN05_EnableRMS",
    "cbN06_1", "cbN06_EnableFTP", "cbN07_EnableSecs", "cbbN06_Mode", "cdE52", "checkbN05_RTC",
    "chkN05_TrayFeedClear", "dtO06_LastDate", "dtpO06NextTime", "edD25_30mm", "edD25_40mm", "edD25_60mm",
    "edD60_56mm", "edE30_1_HP1X", "edE30_1_HP1Y", "edE30_1_HP2X", "edE30_1_HP2Y", "edE30_1_LodX",
    "edE30_1_LodY", "edE30_2_HP1X", "edE30_2_HP1Y", "edE30_2_HP2X", "edE30_2_HP2Y", "edE30_2_LodX",
    "edE30_2_LodY", "edE30_HP1X", "edE30_HP1Y", "edE30_HP2X", "edE30_HP2Y", "edE30_LodX",
    "edE30_LodY", "edE32_1_IS1X", "edE32_1_IS1Y", "edE32_1_IS2X", "edE32_1_IS2Y", "edE32_1_OS1X",
    "edE32_1_OS1Y", "edE32_1_OS2X", "edE32_1_OS2Y", "edE32_2_IS1X", "edE32_2_IS1Y", "edE32_2_IS2X",
    "edE32_2_IS2Y", "edE32_2_OS1X", "edE32_2_OS1Y", "edE32_2_OS2X", "edE32_2_OS2Y", "edE32_IS1X",
    "edE32_IS1Y", "edE32_IS2X", "edE32_IS2Y", "edE32_OS1X", "edE32_OS1Y", "edE32_OS2X",
    "edE32_OS2Y", "edL09_Sh1L", "edL09_Sh1R", "edL09_Sh2L", "edL09_Sh2R", "edN04_ID",
    "edN04_Model", "edN05_1", "edN05_AmbTemp", "edN05_DownPath", "edN05_RmsPath", "edN06_1",
    "edN06_DownPath", "edN06_HostName", "edN06_Password", "edN06_Port", "edN06_TestList", "edN06_TesterMap",
    "edN06_UpLdPath", "edN06_UserName", "editN05_RTC", "edtN04_TesterID", "gbN06_HddLevel", "gbN06_ServerLv",
    "imgI37_3", "rgP23_OCRByInitialStart", "rgP23_OCRByNewTray", "strngrdAutoSaveLog", "tbD25_Index30mm", "tbD25_Index30mm_NS",
    "tbD25_Index40mm", "tbD25_Index40mm_NS", "tbD25_Index60mm", "tbD25_Index60mm_NS", "tbD60_Index56mm", "tbD60_Index56mm_NS",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void IC_CreateSaveProxies()
{
    EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode");
    EL<TCheckBox>("TfConfiguration", "cbA15");
    EL<TCheckBox>("TfConfiguration", "cbA31");
    EL<TCheckBox>("TfConfiguration", "cbC02");
    EL<TCheckBox>("TfConfiguration", "cbC07");
    EL<TCheckBox>("TfConfiguration", "cbD41");
    EL<TCheckBox>("TfConfiguration", "cbE45");
    EL<TCheckBox>("TfConfiguration", "cbI37_1");
    EL<TCheckBox>("TfConfiguration", "cbN05_1");
    EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile");
    EL<TCheckBox>("TfConfiguration", "cbN05_CheckSetupFile");
    EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS");
    EL<TCheckBox>("TfConfiguration", "cbN06_1");
    EL<TCheckBox>("TfConfiguration", "cbN06_EnableFTP");
    EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs");
    EL<TComboBox>("TfConfiguration", "cbbN06_Mode");
    EL<TCheckBox>("TfConfiguration", "cdE52");
    EL<TCheckBox>("TfConfiguration", "checkbN05_RTC");
    EL<TCheckBox>("TfConfiguration", "chkN05_TrayFeedClear");
    EL<filerw::ELDateTimePicker>("TfConfiguration", "dtO06_LastDate");
    EL<filerw::ELDateTimePicker>("TfConfiguration", "dtpO06NextTime");
    EL<TEdit>("TfConfiguration", "edD25_30mm");
    EL<TEdit>("TfConfiguration", "edD25_40mm");
    EL<TEdit>("TfConfiguration", "edD25_60mm");
    EL<TEdit>("TfConfiguration", "edD60_56mm");
    EL<TEdit>("TfConfiguration", "edE30_1_HP1X");
    EL<TEdit>("TfConfiguration", "edE30_1_HP1Y");
    EL<TEdit>("TfConfiguration", "edE30_1_HP2X");
    EL<TEdit>("TfConfiguration", "edE30_1_HP2Y");
    EL<TEdit>("TfConfiguration", "edE30_1_LodX");
    EL<TEdit>("TfConfiguration", "edE30_1_LodY");
    EL<TEdit>("TfConfiguration", "edE30_2_HP1X");
    EL<TEdit>("TfConfiguration", "edE30_2_HP1Y");
    EL<TEdit>("TfConfiguration", "edE30_2_HP2X");
    EL<TEdit>("TfConfiguration", "edE30_2_HP2Y");
    EL<TEdit>("TfConfiguration", "edE30_2_LodX");
    EL<TEdit>("TfConfiguration", "edE30_2_LodY");
    EL<TEdit>("TfConfiguration", "edE30_HP1X");
    EL<TEdit>("TfConfiguration", "edE30_HP1Y");
    EL<TEdit>("TfConfiguration", "edE30_HP2X");
    EL<TEdit>("TfConfiguration", "edE30_HP2Y");
    EL<TEdit>("TfConfiguration", "edE30_LodX");
    EL<TEdit>("TfConfiguration", "edE30_LodY");
    EL<TEdit>("TfConfiguration", "edE32_1_IS1X");
    EL<TEdit>("TfConfiguration", "edE32_1_IS1Y");
    EL<TEdit>("TfConfiguration", "edE32_1_IS2X");
    EL<TEdit>("TfConfiguration", "edE32_1_IS2Y");
    EL<TEdit>("TfConfiguration", "edE32_1_OS1X");
    EL<TEdit>("TfConfiguration", "edE32_1_OS1Y");
    EL<TEdit>("TfConfiguration", "edE32_1_OS2X");
    EL<TEdit>("TfConfiguration", "edE32_1_OS2Y");
    EL<TEdit>("TfConfiguration", "edE32_2_IS1X");
    EL<TEdit>("TfConfiguration", "edE32_2_IS1Y");
    EL<TEdit>("TfConfiguration", "edE32_2_IS2X");
    EL<TEdit>("TfConfiguration", "edE32_2_IS2Y");
    EL<TEdit>("TfConfiguration", "edE32_2_OS1X");
    EL<TEdit>("TfConfiguration", "edE32_2_OS1Y");
    EL<TEdit>("TfConfiguration", "edE32_2_OS2X");
    EL<TEdit>("TfConfiguration", "edE32_2_OS2Y");
    EL<TEdit>("TfConfiguration", "edE32_IS1X");
    EL<TEdit>("TfConfiguration", "edE32_IS1Y");
    EL<TEdit>("TfConfiguration", "edE32_IS2X");
    EL<TEdit>("TfConfiguration", "edE32_IS2Y");
    EL<TEdit>("TfConfiguration", "edE32_OS1X");
    EL<TEdit>("TfConfiguration", "edE32_OS1Y");
    EL<TEdit>("TfConfiguration", "edE32_OS2X");
    EL<TEdit>("TfConfiguration", "edE32_OS2Y");
    EL<TEdit>("TfConfiguration", "edL09_Sh1L");
    EL<TEdit>("TfConfiguration", "edL09_Sh1R");
    EL<TEdit>("TfConfiguration", "edL09_Sh2L");
    EL<TEdit>("TfConfiguration", "edL09_Sh2R");
    EL<TEdit>("TfConfiguration", "edN04_ID");
    EL<TEdit>("TfConfiguration", "edN04_Model");
    EL<TEdit>("TfConfiguration", "edN05_1");
    EL<TEdit>("TfConfiguration", "edN05_AmbTemp");
    EL<TEdit>("TfConfiguration", "edN05_DownPath");
    EL<TEdit>("TfConfiguration", "edN05_RmsPath");
    EL<TEdit>("TfConfiguration", "edN06_1");
    EL<TEdit>("TfConfiguration", "edN06_DownPath");
    EL<TEdit>("TfConfiguration", "edN06_HostName");
    EL<TEdit>("TfConfiguration", "edN06_Password");
    EL<TEdit>("TfConfiguration", "edN06_Port");
    EL<TEdit>("TfConfiguration", "edN06_TestList");
    EL<TEdit>("TfConfiguration", "edN06_TesterMap");
    EL<TEdit>("TfConfiguration", "edN06_UpLdPath");
    EL<TEdit>("TfConfiguration", "edN06_UserName");
    EL<TEdit>("TfConfiguration", "editN05_RTC");
    EL<TEdit>("TfConfiguration", "edtN04_TesterID");
    EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel");
    EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv");
    EL<TControl>("TfConfiguration", "imgI37_3");
    EL<TRadioButton>("TfConfiguration", "rgP23_OCRByInitialStart");
    EL<TRadioButton>("TfConfiguration", "rgP23_OCRByNewTray");
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm");
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS");
}

// golden cConfiguration.cpp:240  TfConfiguration::ReadLockByFile()
static void IC_ReadLockByFile()
{

    /*
    Enabled=0 , Active=0
    Checked鎖起來，不勾關閉
    Enabled=0 , Active=1
    Checked鎖起來，打勾啟動
    Enabled=1 , Active=0 or 1(Active無作用)
    由 Config 畫面直接設定開關
    */
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bF06_Active=ReadIniData(sPath, "Specific", "F06_Active",  true);  //Steven 20140627 : Add for ASE-CL -- F06 打勾
    IniConfig.bF06_Enable=ReadIniData(sPath, "Specific", "F06_Enabled", false); //Steven 20140627 : Add for ASE-CL -- F06 Enable

    IniConfig.bF11_Active       =ReadIniData(sPath, "Specific", "F11_Active",   true);      //Sam 20240202 : 新增 F11 Lock by file 功能
    IniConfig.bF11_Enable       =ReadIniData(sPath, "Specific", "F11_Enabled",  false);     //Sam 20240202 : 新增 F11 Lock by file 功能

    IniConfig.bI06_Active=ReadIniData(sPath, "Specific", "I06_Active",  true);  //Sam 20220527 : for 矽格北興 -- I06 打勾
    IniConfig.bI06_Enable=ReadIniData(sPath, "Specific", "I06_Enabled", false); //Sam 20220527 : for 矽格北興 -- I06 Enable

    IniConfig.bP24_Active=ReadIniData(sPath, "Specific", "P24_Active",  true);  //JerryYang 20160220 add for 矽格北興 -- P24 打勾
    IniConfig.bP24_Enable=ReadIniData(sPath, "Specific", "P24_Enabled", false); //JerryYang 20160220 add for 矽格北興 -- P24 Enable

    IniConfig.bRTC_Active       =ReadIniData(sPath, "Specific", "RTC_Active",  true);       //Sam 20240311 : 新增 RTC Lock by file 功能
    IniConfig.bRTC_Enable       =ReadIniData(sPath, "Specific", "RTC_Enabled",  false);     //Sam 20240311 : 新增 RTC Lock by file 功能
    IniConfig.bI02_Enable       =ReadIniData(sPath, "Specific", "I02_Enabled", true);     //JerryYang 20260504 : for SPIL -- I02 Enable
    IniConfig.bA03_Enable       =ReadIniData(sPath, "Specific", "A03_Enabled", true);

    if(!CheckKeyExist(sPath, "Specific", "I02_Enabled")) WriteIniData(sPath, "Specific", "I02_Enabled", IniConfig.bI02_Enable);
    if(!CheckKeyExist(sPath, "Specific", "A03_Enabled")) WriteIniData(sPath, "Specific", "A03_Enabled", IniConfig.bA03_Enable);

}

// golden cConfiguration.cpp:388  TfConfiguration::InitConfigEdtList_ItemA()
static void IC_InitConfigEdtList_ItemA()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

    if(IniConfig.bVTESTFunction==true   ||
       CUSTOMER_CODE==CC_AMKOR_China    ||
       CUSTOMER_CODE==CC_ASE_KaohSiung  ||
       CUSTOMER_CODE==CC_KYEC_LEE       ||
       CUSTOMER_CODE==CC_QUALCOMM       ||
       IniConfig.bSPILFunction==true    ||
       CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20220215 : ASE-CL松諭要求鎖定A01
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01"), &IniConfig.bA01AutoSwitchToOperatorMode,           ECBool,     "Function", "bAutoSwitchToOperatorMode",            bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01"), &IniConfig.bA01AutoSwitchToOperatorMode,           ECBool,     "Function", "bAutoSwitchToOperatorMode",            bShow, bEnable, bReadFromFile, (int)LastSet.bAutoSwitchToOperatorMode);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150206 超過時間變成op模式
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 60);
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_ASE_CL)                                         //wei 20150903 鎖定A01
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 600);
    }
    else if(IniConfig.bSPILFunction==true)                                      //JerryYang 20200311 矽品國龍說固定180秒
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 180);
    }
    else if(CUSTOMER_CODE==CC_AMKOR_China ||
            CUSTOMER_CODE==CC_QUALCOMM)
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bEnable, bReadFromFile, 60,     false,  10,     180);
    }
    else if(IniConfig.bVTESTFunction==true)
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 120);
    }
    else if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                   //Sam 20250220 : 北興俊堯要求時間要 Disable
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bReadFromFile, 60,     false,  10,     1000);
    }
    else
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA01"),                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bEnable, bReadFromFile, 60,     false,  10,     1000);
    }

    if(CUSTOMER_CODE==CC_JCET ||                                                //JerryYang 20170705 (Steven) add for JCET 按start要切為OP權限改為選項
       CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //RogerYang 20251224 張寧要求打開
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01_1"), &IniConfig.bA01PressStartAutoSwitchToOperatorMode, ECBool,   "Function",     "bPressStartAutoSwitchToOperatorMode",  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01_1"), &IniConfig.bA01PressStartAutoSwitchToOperatorMode, ECBool,   "Function",     "bPressStartAutoSwitchToOperatorMode",  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01_2"), &IniConfig.bA02DisableSaveParsWhenSwitchToOp,    ECBool,   "Function",     "b2DisableSaveParsWhenSwitchToOp",  bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA01_2"), &IniConfig.bA02DisableSaveParsWhenSwitchToOp,    ECBool,   "Function",     "b2DisableSaveParsWhenSwitchToOp",  bNoShow, bDisable, bFixedValue, 0);   //RogerYang 20260312 : Fix
    }

    if(IniConfig.bFTBin2RTBin)                                                  //ChungHung 20120912 add Bin Setting like Epson
    {
        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20220215 : ASE-CL松諭要求
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA02"),    &IniConfig.bA02BinModelPrime,               ECBool,     "Function", "BinModelPrime",                        bShow, bDisable, bFixedValue, 0);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA02"),    &IniConfig.bA02BinModelPrime,               ECBool,     "Function", "BinModelPrime",                        bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA02"),    &IniConfig.bA02BinModelPrime,                   ECBool,     "Function", "BinModelPrime",                        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bLockA03ByFile)                                              //JerryYang 20260504 : for SPIL -- A03 Enable
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA03"),    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, IniConfig.bA03_Enable, bReadFromFile, 0);
    }
    else if(CUSTOMER_CODE==CC_SCS ||
       CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190926 : add ASEM強制開啟[A03]
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制開啟[A03]
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA03"),    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA03"),    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA04"),        &IniConfig.bA04LoaderTraySplitFailCanSkip,      ECBool,     "Function", "bA04LoaderTraySplitFailCanSkip",       bShow, bEnable, bReadFromFile, LastSet.bLoaderTraySplitFailCanSkip?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA05"),        &IniConfig.bA05UseAutoDocking,                  ECBool,     "Function", "bUseAutoDocking",                      bShow, bEnable, bReadFromFile, 0);                            //ChungHung 20120718 add UseAutoDocking Check Sensor
    if(IniConfig.bKoreaFunction)                                                //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA08"),    &IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain, ECBool, "Function", "bLastLoaderAutoCleanOutAndCheckAgain", bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA08"),    &IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain, ECBool, "Function", "bLastLoaderAutoCleanOutAndCheckAgain", bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //ChungHung 20130910 alter for SCK can close site by Index
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA09"),    &IniConfig.bA09_ByArmCloseSite,                 ECBool,     "Function", "bCloseSiteByIndexArm",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkA09_1"), &IniConfig.bA09_1_AutoCloseArm,                 ECBool,     "Function", "bA09_1_AutoCloseArm",                  bNoShow, bDisable, bFixedValue, 0);     //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA09"),    &IniConfig.bA09_ByArmCloseSite,                 ECBool,     "Function", "bCloseSiteByIndexArm",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkA09_1"), &IniConfig.bA09_1_AutoCloseArm,                 ECBool,     "Function", "bA09_1_AutoCloseArm",                  bShow, bEnable, bReadFromFile, 0);      //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20170119 TSMC 命名ART==>ATR
    {
        EL<TGroupBox>("TfConfiguration", "gbA10")->Caption="[A10] Auto Tray retest parameter (ATR)";
        EL<TCheckBox>("TfConfiguration", "cbA10")->Caption="[A10-1] Enable ATR";
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbA10")->Caption="[A10] Auto retest parameter (ART)";
        EL<TCheckBox>("TfConfiguration", "cbA10")->Caption="[A10-1] Enable ART";
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE &&
       (fMain->cbRunStartMode->Text.Pos("ART") ||
        fMain->cbRunStartMode->Text==StartModeName[rsmAutoRetest]))             //Ifor 20220506 Add ART 模式不可關閉ART
    {
        EL<TCheckBox>("TfConfiguration", "cbA10")->Enabled=false;
    }
    else
    {
        EL<TCheckBox>("TfConfiguration", "cbA10")->Enabled=true;
    }

    EL<TGroupBox>("TfConfiguration", "gbA10")->Visible              =(USE_AUTO_RETEST==eartInstall &&               //ChungHung 20140317 add Auto Retest
                                  CUSTOMER_CODE!=CC_ASE_KaohSiung);
    if(USE_AUTO_RETEST==eartUninstall)                                          //ChungHung 20140317 add Auto Retest
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10"),    &IniConfig.bA10_AutoReTest,                     ECBool,     "Function", "bAutoReTest",                          bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10"),    &IniConfig.bA10_AutoReTest,                     ECBool,     "Function", "bAutoReTest",                          bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_SCK)                                                   //Steven 20171211 (Wei) : Auto correction for SCK ART
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_3"),  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_5"),  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bShow, bEnable, bReadFromFile, 1);
    }
    else if(USE_AUTO_RETEST==eartInstall && CosFunction.bUseSCKART)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_3"),  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_5"),  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_3"),  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_5"),  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_HANA_MICRON)                                           //JimmyChiu 20241023 HANA ART Function
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_6"),              &IniConfig.bA10_6_HANA_ART_TestMode_Enable,  ECBool,     "Function", "bA10_6_HANA_ART_TestMode_Enable",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TComboBox>("TfConfiguration", "cbA10_6_HANA_ARTMode"), &IniConfig.iA10_6_HANA_ART_TestMode,         ECInteger,  "Function", "iA10_6_HANA_ART_TestMode",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_6"),              &IniConfig.bA10_6_HANA_ART_TestMode_Enable,  ECBool,     "Function", "bA10_6_HANA_ART_TestMode_Enable",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TComboBox>("TfConfiguration", "cbA10_6_HANA_ARTMode"), &IniConfig.iA10_6_HANA_ART_TestMode,         ECInteger,  "Function", "iA10_6_HANA_ART_TestMode",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                           //RogerYang 20251108 : 瑞薩FTCT
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_7"),              &IniConfig.bA10_7_Renesas_FTCT,     ECBool,     "Function", "bA10_7_Renesas_FTCT",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA10_7"),              &IniConfig.bA10_7_Renesas_FTCT,     ECBool,     "Function", "bA10_7_Renesas_FTCT",   bNoShow, bDisable, bFixedValue, 0);
    }

    EL<TPanel>("TfConfiguration", "palA10_ART")->Visible         =false;
    elConfig->Add(EL<TEdit>("TfConfiguration", "edA10_2"),      &IniConfig.iAutoRetestLimit,                    ECInteger,  "Auto Retest Parameter", "iAutoRetestLimit",        bShow, bEnable, bReadFromFile, 3,      false,  1,      10);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edA10_3"),      &IniConfig.iFailYieldRate_ART,                  ECInteger,  "Auto Retest Parameter", "iFailYieldRate_ART",      bShow, bEnable, bReadFromFile, 10,     false,  1,      100);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edA10_4"),      &IniConfig.iARTTrayArmSpeed,                    ECInteger,  "Auto Retest Parameter", "A10_4_ARTTrayArmSpeed",   bShow, bEnable, bReadFromFile, 30,     false,  1,      100);
    if(USE_AUTO_RETEST==eartInstall && CosFunction.bUseSCKART)                  //Steven 20161208 : ART need to set to 32binGS for ATK
        elConfig->Add(EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode"),  &IniConfig.iA10TestModeForART,      ECInteger,  "Function",  "A10TestModeForART",                   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode"),  &IniConfig.iA10TestModeForART,      ECInteger,  "Function",  "A10TestModeForART",                   bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //2013-04-23    Dell     // 20140103 wei KYEC Barcode Reader
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA11"),    &IniConfig.bA11BarcodeTime,                     ECBool,     "Barcode Reader", "bBarcodeTime",                   bShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA11"),    &IniConfig.iA11BarcodeTime,                     ECInteger,  "Barcode Reader", "iBarcodeTime",                   bShow, bEnable, bReadFromFile, 60,     false,  10,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA11"),    &IniConfig.bA11BarcodeTime,                     ECBool,     "Barcode Reader", "bBarcodeTime",                   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA11"),    &IniConfig.iA11BarcodeTime,                     ECInteger,  "Barcode Reader", "iBarcodeTime",                   bNoShow, bDisable, bFixedValue, 60);
    }

    if(USE_AUTO_RETEST==eartInstall)                                            //ChungHung 20141001 add
    {
        EL<TPanel>("TfConfiguration", "pnlA12")->Visible=true;                                                   //wei 20150826 拍拍Tray X軸 Offset
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA12"),    &IniConfig.bA12ClearLoaderDevice,               ECBool,     "Function", "bClearLoaderDevice",                   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA12"),    &IniConfig.iClearLoaderCount,                   ECInteger,  "Function", "iClearLoaderCount",                    bShow, bEnable, bReadFromFile, 2,      false,  1,      10);            //wei 20150810 拍拍Tray次數設定
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtA12_1"), &IniConfig.iCleanLoaderOffset,                  ECInteger,  "Function", "iCleanLoaderOffset",                   bShow, bEnable, bReadFromFile, 0,      false,  0,      500);           //wei 20150826 拍拍Tray X軸 Offset
    }
    else
    {
        EL<TPanel>("TfConfiguration", "pnlA12")->Visible=false;                                                  //wei 20150826 拍拍Tray X軸 Offset
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA12"),    &IniConfig.bA12ClearLoaderDevice,               ECBool,     "Function", "bClearLoaderDevice",                   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bEnableBarcodeReader)                                        //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA14"),    &IniConfig.bA14UseBarCodeSetWorkFile,           ECBool,     "Function", "bUseBarCodeSetWorkFile",               bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA14"),    &IniConfig.bA14UseBarCodeSetWorkFile,           ECBool,     "Function", "bUseBarCodeSetWorkFile",               bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bStartESDAutoDecayFunction &&                                //Ifor 20150924 :Add Auto Decay Test
       USE_NOVX3360==true)                                                      //Ifor 20210302 add:有安裝SIMCO才顯示與使用
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA15"),    &IniConfig.bA15AutoDecayTest,               ECBool,     "Function", "bAutoDecayTest",                       bShow, bDisable, bFixedValue, 1);    //Ifor 20220823 add: KYEC 樹榮要求[A15]強制開啟
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA15"),    &IniConfig.bA15AutoDecayTest,               ECBool,     "Function", "bAutoDecayTest",                       bShow, bEnable, bReadFromFile, 0);
        }
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA15_ESDReportTime"),      &IniConfig.dESDDataReportTime,  ECDouble,   "Index",    "dESDDataReportTime",                   bShow, bEnable, bReadFromFile, 10.0,   false,  0.01,   600.0);         //2~600秒 //Ifor 20160321 : edESDReportTime
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA15_1"),    &IniConfig.bA15_1ESDGiveWayFunction,          ECBool,     "Function", "bA15_1ESDGiveWayFunction",             bShow, bEnable, bReadFromFile, 0);   //Eastsun 20260521 整合
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA15"),    &IniConfig.bA15AutoDecayTest,                   ECBool,     "Function", "bAutoDecayTest",                       bNoShow, bDisable, bFixedValue, 0);
        EL<TEdit>("TfConfiguration", "edA15_ESDReportTime")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA15_1"),    &IniConfig.bA15_1ESDGiveWayFunction,          ECBool,     "Function", "bA15_1ESDGiveWayFunction",             bNoShow, bDisable, bFixedValue, 0);  //Eastsun 20260521 整合
    }

    if(CUSTOMER_CODE==CC_KYEC_CHEN)                                             //wei 20150831
    {
        EL<TLabel>("TfConfiguration", "lblA16")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chA16"),    &IniConfig.bA16ContactTestDropContact,          ECBool,     "Function", "bContactTestDropContact",              bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TLabel>("TfConfiguration", "lblA16")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chA16"),    &IniConfig.bA16ContactTestDropContact,          ECBool,     "Function", "bContactTestDropContact",              bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190618 : add ASEM 強制 Disable RESET Button
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制 Disable RESET Button
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA17_1"),  &IniConfig.bA17RESETButtonDisable,              ECBool,     "Function", "bRESETButtonDisable",                  bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA17_1"),  &IniConfig.bA17RESETButtonDisable,              ECBool,     "Function", "bRESETButtonDisable",                  bShow, bEnable, bReadFromFile, 0);  //kevin 20151113 RESET 按鍵使用
    }

    if(CUSTOMER_CODE==CC_UMC)                                                   //JimmuyChiu 20211012   Add Reset->Clean out with no testing
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA17_2"),  &IniConfig.bA17_1RESETCleanOutWithoutTest,      ECBool,     "Function", "bA17_1RESETCleanOutWithoutTest",       bShow, bDisable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA17_2"),  &IniConfig.bA17_1RESETCleanOutWithoutTest,      ECBool,     "Function", "bA17_1RESETCleanOutWithoutTest",       bShow, bEnable, bReadFromFile, 0);

    if(CosFunction.bUsePMAlarmFunction)                                         //wei 20160225 PMAlarmFunction
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA19"),    &IniConfig.bA19UsePMAlarmFunction,              ECBool,     "Function", "bUsePMAlarmFunction",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA19"),    &IniConfig.bA19UsePMAlarmFunction,              ECBool,     "Function", "bUsePMAlarmFunction",                  bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA20_1"),      &IniConfig.bA20_1CheckRTCFunction,              ECBool,     "Function", "bA20_1CheckRTCFunction",               bNoShow, bEnable, bFixedValue, 0);  //wei 20171023 Disable Start比對
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA20_2"),      &IniConfig.bA20_2CheckTrayIDFunction,           ECBool,     "Function", "bA20_2CheckTrayIDFunction",            bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA20_3"),      &IniConfig.bA20_3CheckAutocleanFunction,        ECBool,     "Function", "bA20_3CheckAutocleanFunction",         bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA20_4"),      &IniConfig.bA20_4CheckContsFailFunction,        ECBool,     "Function", "bA20_4CheckContsFailFunction",         bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA20_5"),      &IniConfig.bA20_5CheckOCRFunction,              ECBool,     "Function", "bA20_5CheckOCRFunction",               bNoShow, bEnable, bFixedValue, 0);

    if(USE_ROTATE_KIT==1 && iRotate_Type!=eCynRotate)                           //JerryYang 20160825 Rotate sensor偵測異常要先試著旋轉三次再跳alarm
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA21"),    &IniConfig.bA21RotateDetectErrNeedShake,        ECBool,     "Function", "bRotateDetectErrNeedShake",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA21"),    &IniConfig.bA21RotateDetectErrNeedShake,        ECBool,     "Function", "bRotateDetectErrNeedShake",            bNoShow, bEnable, bFixedValue, 0);

    if(USE_MAGNETIC_SCALE)
    {
        EL<TGroupBox>("TfConfiguration", "gbA22")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA22_1"),  &IniConfig.bA22MagneticScale,                   ECBool,     "Function", "bA22MagneticScale",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA22_2"),  &IniConfig.dA22MagneticScaleKeepRunRange,       ECDouble,   "Function", "dMagneticScaleKeepRunRange",           bShow, bEnable, bReadFromFile, 0.01,   true,   0.01,   10.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA22_3"),  &IniConfig.dA22MagneticScaleStopRunRange,       ECDouble,   "Function", "dMagneticScaleStopRunRange",           bShow, bEnable, bReadFromFile, 0.01,   true,   0.01,   10.0);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbA22")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA22_1"),  &IniConfig.bA22MagneticScale,                   ECBool,     "Function", "bA22MagneticScale",                    bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  //JerryYang 20170421 (Steven) JCET吳如春要求必須輸入lot no才能start   //JerryYang 20170706 (Steven) 重新啟用A23
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA23"),    &IniConfig.bA23CheckLotNoInSLTReport,           ECBool,     "Function", "bA23CheckLotNoInSLTReport",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA23"),    &IniConfig.bA23CheckLotNoInSLTReport,           ECBool,     "Function", "bA23CheckLotNoInSLTReport",            bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bUseAutoBackUpSetupFile)                                     //Ifor 20170508 (wei) add Auto BackUp Setup File ,& Last Data
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA24"),    &IniConfig.bA24AutoBackupSetupFile,             ECBool,     "Function", "bA24AutoBackupSetupFile",              bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA24"),    &IniConfig.bA24AutoBackupSetupFile,             ECBool,     "Function", "bA24AutoBackupSetupFile",              bNoShow, bEnable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045
    {
        EL<TGroupBox>("TfConfiguration", "grpA25")->Visible=true;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA25_1"),  &IniConfig.asA25RunExecutFile,                  ECText,     "Function", "asA25RunExecutFile",                   bShow, bEnable, bReadFromFile, "C:\\GTK\\Switch.exe");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA25_2"),  &IniConfig.asA25RunExecutButtonName,            ECText,     "Function", "asA25RunExecutButtonName",             bShow, bEnable, bReadFromFile, "SWITCH");
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpA25")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //KaiChen 20171225 (Steven)：Add Speed Display
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA26"),    &IniConfig.bA26MotorSpeedSortDisplay,           ECBool,     "Function", "bA26MotorSpeedSortDisplay",            bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA26"),    &IniConfig.bA26MotorSpeedSortDisplay,           ECBool,     "Function", "bA26MotorSpeedSortDisplay",            bShow, bEnable, bReadFromFile, 0);

    EL<TSpeedButton>("TfConfiguration", "spbA27")->Visible             =(CosFunction.bConfigStandard);                 //KaiChen 20171113 ：超豐 將 config.ini 另外存成 config_Standard.ini      //JimmyChiu 20220114 CUSTOMER_CODE==CC_Greatek => CosFunction.bConfigStandard
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA27"),        &IniConfig.bA27EnableLightScale,                ECBool,     "Function", "bA27EnableLightScale",                 bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA27_1"),      &IniConfig.bA27_1LogEnableLightScaleData,       ECBool,     "Function", "bA27_1LogEnableLightScaleData",        bShow, bEnable, bReadFromFile, 0);  //KaiChen 20171228 ：Log Light Scale Data
    if(CUSTOMER_CODE==CC_Greatek)
    {
        EL<TGroupBox>("TfConfiguration", "grpA28")->Visible=true;                                                   //KaiChen 20171111 (Steven) ：超豐 開啟指定路徑的 HTML 檔案(PM SOP)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtA28_1"), &IniConfig.asA28_1ShowPmSopReadFilePath,        ECText,     "Function", "asA26_1ShowPmSopReadFilePath",         bShow, bEnable, bReadFromFile, "");     //Sam 20220926 : 整理
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpA28")->Visible=false;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtA28_1"), &IniConfig.asA28_1ShowPmSopReadFilePath,        ECText,     "Function", "asA26_1ShowPmSopReadFilePath",         bNoShow, bEnable, bFixedValue, "");
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA29"),        &IniConfig.bA29EnableAutoCleanFunction,         ECBool,     "Function", "bA29EnableAutoCleanFunction",          bNoShow, bEnable, bFixedValue, 0);  //wei 20171023 Auto clean開關

    if(CosFunction.bSetupTeach)                                                 //JerryYang 20180921 Setup Teach功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA30"),    &IniConfig.bA30SetupTeachFunction,              ECBool,     "Function", "bA30SetupTeachFunction",               bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA30"),    &IniConfig.bA30SetupTeachFunction,              ECBool,     "Function", "bA30SetupTeachFunction",               bNoShow, bEnable, bFixedValue, 0);
    }

    if(USE_AutoCleanIonFan==1 && CosFunction.bIOTriggerIonFanAutoClean)         //Isaac 20210609 : IO觸發IonFan清針
    {
        EL<TGroupBox>("TfConfiguration", "gbA31")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA31"),    &IniConfig.bA31EnableAutoCleanIonFanFunction,   ECBool, "Function", "bA33EnableAutoCleanIonFanFunction",          bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA31_1"),  &IniConfig.bA31AutoCleanIonFanInitialStart,     ECBool, "Function", "bA33AutoCleanIonFanFunctionInitialStart",    bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbA31")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA31"),    &IniConfig.bA31EnableAutoCleanIonFanFunction,   ECBool, "Function", "bA33EnableAutoCleanIonFanFunction",          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA31_1"),  &IniConfig.bA31AutoCleanIonFanInitialStart,     ECBool, "Function", "bA33AutoCleanIonFanFunctionInitialStart",    bNoShow, bDisable, bFixedValue, 0);
    }

    EL<TGroupBox>("TfConfiguration", "grpA32_1")->Visible=false;
    if(IniConfig.bSIGURDFunction)
    {
        EL<TGroupBox>("TfConfiguration", "grpA32")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32"),    &IniConfig.bA32EnableFTPAutomation,             ECBool,     "Function", "bA32EnableFTPAutomation",              bShow, bEnable, bReadFromFile, 1);   //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA32_1"),  &IniConfig.asA32_1_HandlerID,                   ECText,     "Function", "asA32_1_HandlerID",                    bShow, bEnable, bReadFromFile, "");    //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_2"),  &IniConfig.bA32_2ReturnHandlerID2OI,            ECBool,     "Function", "bA32_2ReturnHandlerID2OI",             bShow, bEnable, bReadFromFile, 1);   //KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI

        if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                  //Sam 20220620 : 中興廠新增 A32-3 功能 for 93K function
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_3"), &IniConfig.bA32_3For93KFunction,             ECBool,     "Function", "bA32_3For93KFunction",                 bShow,  bEnable, bReadFromFile, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_3"), &IniConfig.bA32_3For93KFunction,             ECBool,     "Function", "bA32_3For93KFunction",                 bNoShow,bEnable, bFixedValue, 0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_01"), &IniConfig.bA32EnableCheckList[eCL_Temperature],ECBool, "FTPAutomation", "CheckList_Enable_Temperature",                bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_02"), &IniConfig.bA32EnableCheckList[eCL_Alarm],      ECBool, "FTPAutomation", "CheckList_Enable_Alarm",                      bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_03"), &IniConfig.bA32EnableCheckList[eCL_FT_Yield],   ECBool, "FTPAutomation", "CheckList_Enable_FT_Yield",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_04"), &IniConfig.bA32EnableCheckList[eCL_SiteMapping],ECBool, "FTPAutomation", "CheckList_Enable_SiteMapping",                bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_05"), &IniConfig.bA32EnableCheckList[eCL_Speed],      ECBool, "FTPAutomation", "CheckList_Enable_Speed",                      bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_06"), &IniConfig.bA32EnableCheckList[eCL_Contact],    ECBool, "FTPAutomation", "CheckList_Enable_Contact",                    bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_07"), &IniConfig.bA32EnableCheckList[eCL_Category],   ECBool, "FTPAutomation", "CheckList_Enable_Category",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_08"), &IniConfig.bA32EnableCheckList[eCL_BinSetting], ECBool, "FTPAutomation", "CheckList_Enable_BinSetting",                 bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_09"), &IniConfig.bA32EnableCheckList[eCL_TrayForm],   ECBool, "FTPAutomation", "CheckList_Enable_TrayForm",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_10"), &IniConfig.bA32EnableCheckList[eCL_HotPlate],   ECBool, "FTPAutomation", "CheckList_Enable_HotPlate",                   bShow, bEnable, bReadFromFile, 1);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA33"),    &IniConfig.bA33SetICToErrBinAfterOutShtLossIC,  ECBool, "Function", "bA33SetMachineIC2ErrorBinAfterOutShuttleLossIC",   bShow, bEnable, bReadFromFile, 0);  //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpA32")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32"),    &IniConfig.bA32EnableFTPAutomation,             ECBool, "Function", "bA32EnableFTPAutomation",                          bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA32_2"),  &IniConfig.bA32_2ReturnHandlerID2OI,            ECBool, "Function", "bA32_2ReturnHandlerID2OI",                         bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA33"),    &IniConfig.bA33SetICToErrBinAfterOutShtLossIC,  ECBool, "Function", "bA33SetMachineIC2ErrorBinAfterOutShuttleLossIC",   bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA35"),    &IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr, ECBool, "Function", "bA35SetErrBinWhenOutShtLoseAndOutShtPickupErr", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA35"),    &IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr, ECBool, "Function", "bA35SetErrBinWhenOutShtLoseAndOutShtPickupErr", bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_Microchip_Phil)                                        //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA36"),    &IniConfig.bA36OpenDoorSetErrBin,               ECBool, "Function", "bA36OpenDoorSetErrBin",        bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA36"),    &IniConfig.bA36OpenDoorSetErrBin,               ECBool, "Function", "bA36OpenDoorSetErrBin",        bNoShow, bEnable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA37"),    &IniConfig.bA37LotStartLotEnd,                  ECBool, "Function", "bA32LotStartLotEnd",           bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA37"),    &IniConfig.bA37LotStartLotEnd,                  ECBool, "Function", "bA32LotStartLotEnd",           bNoShow, bEnable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : add for SLT lot summary
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA38"),    &IniConfig.bA38_SLT_Summary,                    ECBool, "Function", "bA35_SLT_Summary",             bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA38"),    &IniConfig.bA38_SLT_Summary,                    ECBool, "Function", "bA35_SLT_Summary",             bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_M)                                                 //Ifor 20230420 add:ASEM要求新增Run Status Log Function
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA39"),    &IniConfig.bA39RecordRunState,                  ECBool, "Function", "bA39RecordRunState",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA39"),    &IniConfig.iA39RecordTime,                      ECInteger, "Function", "iA39RecordTime",            bShow, bEnable, bReadFromFile, 5,     false,  5,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA39"),    &IniConfig.bA39RecordRunState,                  ECBool, "Function", "bA39RecordRunState",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA39"),    &IniConfig.iA39RecordTime,                      ECInteger, "Function", "iA39RecordTime",            bNoShow, bDisable, bReadFromFile, 60,     false,  5,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)               //Ifor 20201126 add:Contact 頁面開啟時不記錄2D 資料
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA40"),    &IniConfig.bA40DoNotRecord2DDataWhenContactShow,ECBool, "Function", "bA40DoNotRecord2DDataWhenContactShow", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA40"),    &IniConfig.bA40DoNotRecord2DDataWhenContactShow,ECBool, "Function", "bA40DoNotRecord2DDataWhenContactShow", bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bNonCenterModeCanUseShtOffset)                               //Steven 20200715 : for Tinton
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkA50"),        &IniConfig.bA50Enable1x4BiasYOffset,       ECBool, "Function", "bA32Enable1x4BiasYOffset",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkA50"),        &IniConfig.bA50Enable1x4BiasYOffset,       ECBool, "Function", "bA32Enable1x4BiasYOffset",     bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bCanDisableQAMode)                                           //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA51"),        &IniConfig.bA51EnableEQCMode,               ECBool, "Function", "bA31EnableEQCMode",            bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA51"),        &IniConfig.bA51EnableEQCMode,               ECBool, "Function", "bA31EnableEQCMode",            bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUsePMAlarmFunction && CosFunction.bOEEFunction)             //JimmyChiu 20230315 : PM Alarm Update From Server by FTP
    {
        EL<TGroupBox>("TfConfiguration", "grpA55")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA55"), &IniConfig.bA55EnablePMAlarmUpdateFromServerbyFTP, ECBool, "Function", "bA55EnablePMAlarmUpdateFromServerbyFTP", bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpA55")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA55"), &IniConfig.bA55EnablePMAlarmUpdateFromServerbyFTP, ECBool, "Function", "bA55EnablePMAlarmUpdateFromServerbyFTP", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bManualSteplAutoTeach)                                       //JimmyChiu 20211020 : Auto alignment mode
    {
        EL<TGroupBox>("TfConfiguration", "gbA56")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA56_1"),  &IniConfig.bA56EnableAutoTeachFunciton,                 ECBool,     "Function",    "bA56EnableAutoTeachFunciton",              bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA56_2"),  &IniConfig.iA56ShuttlePickUpOffsetWhenAutoTeach,        ECInteger,  "Function",    "iA56ShuttlePickUpOffsetWhenAutoTeach",     bShow, bEnable, bReadFromFile, 1,   true,   1,   2000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA56_3"),  &IniConfig.iA56SocketPickUpOffsetWhenAutoTeachOnly,     ECInteger,  "Function",    "iA56SocketPickUpOffsetWhenAutoTeachOnly",  bShow, bEnable, bReadFromFile, 1,   true,   1,   2000);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbA56")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA56_1"),  &IniConfig.bA56EnableAutoTeachFunciton,                 ECBool,     "Function",    "bA56EnableAutoTeachFunciton",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA56_2"),  &IniConfig.iA56ShuttlePickUpOffsetWhenAutoTeach,        ECInteger,  "Function",    "iA56ShuttlePickUpOffsetWhenAutoTeach",     bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA56_3"),  &IniConfig.iA56SocketPickUpOffsetWhenAutoTeachOnly,     ECInteger,  "Function",    "iA56SocketPickUpOffsetWhenAutoTeachOnly",  bNoShow, bDisable, bFixedValue, 1);
    }

    EL<TGroupBox>("TfConfiguration", "gbA57")->Visible=(CosFunction.bSaveArmSpeedByMachine ||
                    CosFunction.bSaveTemperatureByMachine);                     //JimmyChiu 20220618 : save by machine
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA57_1"),  &IniConfig.bA57_1SaveArmSpeedByMachine,             ECBool, "Function", "bA57_1SaveArmSpeedByMachine",      (CosFunction.bSaveArmSpeedByMachine)?bShow:bNoShow, bEnable,    (CosFunction.bSaveArmSpeedByMachine)?bReadFromFile:bFixedValue,     0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA57_2"),  &IniConfig.bA57_2SaveTemperatureByMachine,          ECBool, "Function", "bA57_2SaveTemperatureByMachine",   (CosFunction.bSaveTemperatureByMachine)?bShow:bNoShow, bEnable, (CosFunction.bSaveTemperatureByMachine)?bReadFromFile:bFixedValue,  0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA57_3"),  &IniConfig.bA57_3SaveOffsetByMachine,               ECBool, "Function", "bA57_3SaveOffsetByMachine",        (CosFunction.bSaveOffsetByMachine)?bShow:bNoShow, bEnable,      (CosFunction.bSaveOffsetByMachine)?bReadFromFile:bFixedValue,       0);

    if(IniConfig.bSPILFunction==true)                                           //Jimmychiu 20230925 : Show Close Sites Alarm When Start
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA58"),    &IniConfig.bShowCloseSiteAlarmWhenStart,        ECBool,    "PrecautionRecord", "bShowCloseSiteAlarmWhenStart", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA58"),    &IniConfig.bShowCloseSiteAlarmWhenStart,        ECBool,    "PrecautionRecord", "bShowCloseSiteAlarmWhenStart", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bEnableHandlerResultServer)                                  //Sam 20240304 : 新增 AMR 功能
    {
        EL<TGroupBox>("TfConfiguration", "grpA60")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA60_1"),  &IniConfig.bA60EnableAMR,                       ECBool,"Function", "bA60EnableAMR",             bShow, bEnable, bReadFromFile, 0);
        ;   // UI-only（HTML 端處理）：EL<TLabel>("TfConfiguration", "lblA60_1")->Font->Color=clBlue;
        ;   // UI-only（HTML 端處理）：EL<TLabel>("TfConfiguration", "lblA60_2")->Font->Color=clBlue;
        ;   // UI-only（HTML 端處理）：EL<TLabel>("TfConfiguration", "lblA60_3")->Font->Color=clBlue;
        ;   // UI-only（HTML 端處理）：EL<TLabel>("TfConfiguration", "lblA60_4")->Font->Color=clBlue;
        ;   // UI-only（HTML 端處理）：EL<TLabel>("TfConfiguration", "lblA60_5")->Font->Color=clBlue;
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edA60_1"),     &IniConfig.iA60NotifyQty[0],        ECInteger,  "Function",     "iA60NotifyQtyLoader",    bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edA60_2"),     &IniConfig.iA60NotifyQty[3],        ECInteger,  "Function",     "iA60NotifyQtyAuto1",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edA60_3"),     &IniConfig.iA60NotifyQty[4],        ECInteger,  "Function",     "iA60NotifyQtyAuto2",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edA60_4"),     &IniConfig.iA60NotifyQty[5],        ECInteger,  "Function",     "iA60NotifyQtyAuto3",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edA60_5"),     &IniConfig.iA60LoaderQtyAtOneTime,  ECInteger,  "Function",     "iA60QtyAtOneTime",       bShow, bEnable, bReadFromFile, 9      ,false, 1,       100);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpA60")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA60_1"),  &IniConfig.bA60EnableAMR,                       ECBool,"Function", "bA60EnableAMR",             bNoShow, bEnable, bFixedValue, 0);
    }

    if(USE_COVER_TRAYID!=tCIDNotUse)                                            //JerryYang 20250304 : A65有安裝硬體才Show出來
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA65"),    &IniConfig.bA65_BundleIDList,                   ECBool,    "Function",          "bA65_BundleIDList",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA68"),    &IniConfig.bA68_AutoLoadUnload,                 ECBool,    "Function",          "bA68_AutoLoadUnload",         bShow, bEnable, bReadFromFile, 0);   //JerryYang 20250220 : add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA69"),    &IniConfig.bA69_ScanBinLabelToBinCodeEdit,     ECBool,    "Function",          "bA69_ScanBinLabelToBinCodeEdit", bShow, bEnable, bReadFromFile, 0);   //JerryYang 20260409 : add
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA65"),    &IniConfig.bA65_BundleIDList,                   ECBool,    "Function",          "bA65_BundleIDList",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA68"),    &IniConfig.bA68_AutoLoadUnload,                 ECBool,    "Function",          "bA68_AutoLoadUnload",         bNoShow, bDisable, bFixedValue, 0);  //JerryYang 20250220 : add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA69"),    &IniConfig.bA69_ScanBinLabelToBinCodeEdit,     ECBool,    "Function",          "bA69_ScanBinLabelToBinCodeEdit", bNoShow, bDisable, bFixedValue, 0);  //JerryYang 20260409 : add
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA66"),        &IniConfig.bA66_2D_Sort,                        ECBool,    "Function",          "bA66_2D_Sort",                bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Jeff 20241001 add Continuous Mode disable  clean MUBA
       elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA61"),    &IniConfig.bA61DisableCleanMUBA,                 ECBool, "Function", "bA61DisableCleanMUBA",            bShow, bEnable, bReadFromFile, 0);
    else
       elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA61"),    &IniConfig.bA61DisableCleanMUBA,                 ECBool, "Function", "bA61DisableCleanMUBA",            bNoShow, bDisable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA62"),       &IniConfig.bA62bUseStopMachineArmHome,           ECBool, "Function", "bA62bUseStopMachineArmHome",      bShow, bEnable, bReadFromFile, 0);//Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20241028 : 矽品彰化要求 特定ALARM要觸發ONE CYCLE
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA67"),    &IniConfig.bA67TriggerOneCycleWhenAlarm,        ECBool,    "Function", "bA67TriggerOneCycleWhenAlarm", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA67"),    &IniConfig.bA67TriggerOneCycleWhenAlarm,        ECBool,    "Function", "bA67TriggerOneCycleWhenAlarm", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_GIGAS)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA71"),    &IniConfig.bA71UseBackupNowRecipe,              ECBool,    "Function",     "bA71UseBackupNowRecipe",            bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA71"),    &IniConfig.sA71FolderPath,                      ECText,    "Function",     "sA71FolderPath",                    bShow, bEnable, bReadFromFile, "D:\\Run");
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA71"),    &IniConfig.bA71UseBackupNowRecipe,              ECBool,    "Function",     "bA71UseBackupNowRecipe",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edA71"),    &IniConfig.sA71FolderPath,                      ECText,    "Function",     "sA71FolderPath",                    bNoShow, bDisable, bFixedValue, "D:\\Run");
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Eastsun 20260519 整合
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA72"),  &IniConfig.bA72_FixAOIButton,               ECBool, "Function", "bA72_FixAOIButton",        bShow, bEnable,      bReadFromFile,       1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA72"),  &IniConfig.bA72_FixAOIButton,               ECBool, "Function", "bA72_FixAOIButton",        bNoShow, bEnable,      bReadFromFile,       0);
    }

//    if(CUSTOMER_CODE==CC_KYEC_LEE)   //EastSun 20260508 : A41 KYEC user manage FT/RT Bin Share (skip LotStart override)    //Eastsun 20260710整合
    if(CUSTOMER_CODE==CC_KYEC_CHEN || CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_STM || CUSTOMER_CODE==CC_KYEC_XILINX)  //Eastsun 20260710整合
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA73"),    &IniConfig.bA73UserOverrideFTRTBin,            ECBool, "Function", "bA73UserOverrideFTRTBin",         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA73"),    &IniConfig.bA73UserOverrideFTRTBin,            ECBool, "Function", "bA73UserOverrideFTRTBin",         bNoShow, bDisable, bReadFromFile, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)   //EastSun 20260508 : A41 KYEC user manage FT/RT Bin Share (skip LotStart override)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA74"),    &IniConfig.bA74AOIFailCountLinkLotRunMode,            ECBool, "Function", "bA74AOIFailCountLinkLotRunMode",         bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA74"),    &IniConfig.bA74AOIFailCountLinkLotRunMode,            ECBool, "Function", "bA74AOIFailCountLinkLotRunMode",         bNoShow, bDisable, bReadFromFile, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA75"), &IniConfig.bA75DownloadItemByAccessLevel, ECBool, "Function", "bA75DownloadItemByAccessLevel", bShow, bEnable, bReadFromFile, 0);   //AI(safety-critical-change) 20260611 (RogerYang) : 下載複寫項目依權限控管(OP禁用)

    if(IniConfig.bSPILFunction==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA76"), &IniConfig.bA76AfterHomeNeedAlm, ECBool, "Function", "bA76AfterHomeNeedAlm",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA81WaitSECS"), &IniConfig.bA81WaitSECS, ECBool, "SECS GEM", "Wait SECS",                         bShow, bEnable, bReadFromFile, 0);       //KevinCheng 20250922 Wait SECS
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA76"), &IniConfig.bA76AfterHomeNeedAlm, ECBool, "Function", "bA76AfterHomeNeedAlm",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA81WaitSECS"), &IniConfig.bA81WaitSECS, ECBool, "SECS GEM", "Wait SECS",                         bNoShow, bDisable, bFixedValue, 0);
    }
    if(CUSTOMER_CODE==CC_HANA_MICRON)                                           //RogerYang 20260725 : HANA RMS Interlock enable
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA77"), &IniConfig.bA77_EnableHanaRMSInterlock, ECBool, "A77", "A77_EnableHanaRMSInterlock", bShow, bEnable, bReadFromFile, 0);
        EL<TButton>("TfConfiguration", "btnA77RMSSetting")->Visible=true;
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA77"), &IniConfig.bA77_EnableHanaRMSInterlock, ECBool, "A77", "A77_EnableHanaRMSInterlock", bNoShow, bDisable, bReadFromFile, 0);
    }

    //AI(ht9045-config) 20260806 (RogerYang) : #R260723-ATK-H9-01 QA Mode Enable/Disable checkbox (ATK only)
    if(CUSTOMER_CODE==CC_AMKOR_Korea)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA78"), &IniConfig.bA78EnableQAMode, ECBool, "Function", "bA78EnableQAMode", bShow,   bEnable,  bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA78"), &IniConfig.bA78EnableQAMode, ECBool, "Function", "bA78EnableQAMode", bNoShow, bDisable, bFixedValue,   0);

}

// golden cConfiguration.cpp:1007  TfConfiguration::InitConfigEdtList_ItemB()
static void IC_InitConfigEdtList_ItemB()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

//    tsb00->TabVisible=(CUSTOMER_CODE==CC_Greatek  ||
//                       CUSTOMER_CODE==CC_PTI);                                //Sam 20240804 : 新增 PTI
    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    {
        EL<TGroupBox>("TfConfiguration", "grpB01")->Visible=true;
        EL<TGroupBox>("TfConfiguration", "grpB02")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB01"),    &IniConfig.bB01_UsePrecautionRecordFunction,            ECBool,     "PrecautionRecord",     "bB01_UsePrecautionRecordFunction",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB02"),    &IniConfig.bB02_HanderMajorMaintenanceRecordFunction,   ECBool,     "PrecautionRecord",     "bB02_HanderMajorMaintenanceRecordFunction",    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB01"),    &IniConfig.iB01_AutoWakeupPrecautionRecordFormTime,     ECInteger,  "PrecautionRecord",     "iB01_AutoWakeupPrecautionRecordFormTime",      bShow, bEnable, bReadFromFile, 10,   false,  0.01,   600.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB01_1"),  &IniConfig.asB01_PrecautionRecordSavePath,              ECText,     "PrecautionRecord",     "asB01_PrecautionRecordSavePath",               bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB02"),    &IniConfig.asB02_HanderMajorMaintenanceRecordSavePath,  ECText,     "PrecautionRecord",     "asB02_HanderMajorMaintenanceRecordSavePath",   bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpB01")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpB02")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB01"),    &IniConfig.bB01_UsePrecautionRecordFunction,            ECBool,     "PrecautionRecord",     "bB01_UsePrecautionRecordFunction",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB02"),    &IniConfig.bB02_HanderMajorMaintenanceRecordFunction,   ECBool,     "PrecautionRecord",     "bB02_HanderMajorMaintenanceRecordFunction",    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20231115 : PTI 新增 Tester report
    {
        EL<TGroupBox>("TfConfiguration", "grpB03")->Visible=true;
        elConfig         ->Add(EL<TCheckBox>("TfConfiguration", "cbB03"),       &IniConfig.bB03_TesterReport,   ECBool,     "Report",   "bB03_TesterReport",    bShow, bEnable, bReadFromFile, 0);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edB03_1"),     &IniConfig.sB03_Customer,       ECText,     "Report",   "sB03_Customer",        bShow, bEnable, bReadFromFile, "Customer");     //Sam 20231115 : PTI 新增 Tester report
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edB03_2"),     &IniConfig.sB03_DeviceID,       ECText,     "Report",   "sB03_DeviceID",        bShow, bEnable, bReadFromFile, "DeviceID");     //Sam 20231115 : PTI 新增 Tester report
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpB03")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB03"),    &IniConfig.bB03_TesterReport,               ECBool,     "Report",   "bB03_TesterReport",    bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB05"),        &IniConfig.bB05_OSReport,                   ECBool,     "Report",   "bB05_OSReport",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtB05"),       &IniConfig.sB05_OSReportPath,               ECText,     "Report",   "sB05_OSReportPath",    bShow, bEnable, bReadFromFile, "");             //Steven 20250513 : OS Report
    if(CUSTOMER_CODE==CC_PANTHER)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB11Enable"),          &IniConfig.bB11UsePATServerFile,  ECBool,  "PrecautionRecord",     "bB11UsePATServerFile",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB11PATServerPath"),   &IniConfig.sB11PATServerPath,     ECText,  "PrecautionRecord",     "sB11PATServerPath",     bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbB12Enable"),          &IniConfig.bB12UsePATSetup,       ECBool,  "PrecautionRecord",     "bB12UsePATSetup",       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB12Path"),            &IniConfig.sB12PATSetupPath,      ECText,  "PrecautionRecord",     "sB12PATSetupPath",      bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB13UploadPath"),      &IniConfig.sB13PATJobUploadPath,  ECText,  "PrecautionRecord",     "sB13PATJobUploadPath",  bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB13DownloadPath"),    &IniConfig.sB13PATJobDownloadPath,ECText,  "PrecautionRecord",     "sB13PATJobDownloadPath",bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB14IntervalTime"),    &IniConfig.iB14IntervalTime,      ECInteger,"PrecautionRecord",     "iB14IntervalTime",      bShow, bEnable, bReadFromFile, 20,   false,  0,   600);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edB14ReportRealTime"),  &IniConfig.sB14RealTimePath,      ECText,  "PrecautionRecord",     "sB14RealTimePath",      bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
    }
    else
    {
        EL<TPanel>("TfConfiguration", "pal_B11")->Visible=false;
    }

}

// golden cConfiguration.cpp:1065  TfConfiguration::InitConfigEdtList_ItemC()
static void IC_InitConfigEdtList_ItemC()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_HONPREC_QC)                //wei 20160215大風扇方向
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC01"),    &IniConfig.bC01_FanDirection,                   ECBool, "Function", "bFanDirection",                    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC01"),    &IniConfig.bC01_FanDirection,                   ECBool, "Function", "bFanDirection",                    bNoShow, bEnable, bFixedValue, 0);

    if(IniConfig.bEnableCCDUSETCPIP==true)
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbC02"),   &IniConfig.bC02InstallCCD,                      ECBool, "Index",    "bC02InstallCCD",                   bShow, bEnable, bReadFromFile, LastSet.bC02InstallCCD?"1":"0");
    else
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbC02"),   &IniConfig.bC02InstallCCD,                      ECBool, "Index",    "bC02InstallCCD",                   bNoShow, bDisable, bFixedValue, 0);

    if(USE_AUTO_RETEST==eartInstall)
    {
        if((SubMachineType==Type_HT9046AU))                                     //RogerYang 20250508 9046AU No Catch Cylinder
        {
            cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbC03"),   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bDisable, bFixedValue, 0);
        }
        else
        {
            cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbC03"),   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bDisable, bFixedValue, 1);
        }
    }
    else
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbC03"),   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bEnable, bReadFromFile, LastSet.bC03UseCatchTray?"1":"0");

    if(CUSTOMER_CODE==CC_HONPREC_QC)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC04"),    &IniConfig.bC04EnableTestTempIC,                ECBool, "Index",    "bC04EnableTestTempIC",             bShow, bEnable, bReadFromFile, LastSet.bEnableTestTempIC?"1":"0");
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC04"),    &IniConfig.bC04EnableTestTempIC,                ECBool, "Index",    "bC04EnableTestTempIC",             bNoShow, bEnable, bFixedValue, 0);

    //AI(ht9045-v899) 20260804: PTI shows the temp module only. Motor/vacuum/ATC/mode keep the very same
    //bNoShow/bDisable/bFixedValue treatment they already get today, so nothing but the temp
    //module is newly enabled. Mode is forced to 0 on purpose: mode 1 pops a YES/NO box and
    //waits for an operator, which would silently defeat an unattended overnight save. (CASE-PTI-20260804-001)
    if(IniConfig.bPowerSaveFunction && CosFunction.bPowerSaveTempOnly)
    {
        AnsiString asC05Max;

        EL<TGroupBox>("TfConfiguration", "gbC05")->Visible=true;
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgC05"),        &IniConfig.iPowersaveMode,                  ECInteger,  "OutPowerSave", "PowersaveMode",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Motor"),  &IniConfig.bC05_PowerSaveMotor,             ECBool,     "OutPowerSave", "UsepowerSaveMotor",        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Temp"),   &IniConfig.bC05_PowerSaveTemp,              ECBool,     "OutPowerSave", "UsepowerSaveTemp",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Vacuum"), &IniConfig.bC05_PowerSaveVacuum,            ECBool,     "OutPowerSave", "UsepowerSaveVacuum",       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_ATC"),    &IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Motor"),  &IniConfig.iHaltTime_Motor,                 ECInteger,  "OutPowerSave", "HaltTime_Motor",           bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Temp"),   &IniConfig.iHaltTime_Temp,                  ECInteger,  "OutPowerSave", "HaltTime_Temp",            bShow, bEnable, bReadFromFile, 1,      false,  1,      GetPowerSaveMaxMinute());
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Vacuum"), &IniConfig.iC05HaltTime_Vacuum,             ECInteger,  "OutPowerSave", "HaltTime_Vacuum",          bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_ATC"),    &IniConfig.iHaltTime_ATC,                   ECInteger,  "OutPowerSave", "HaltTime_ATC",             bNoShow, bDisable, bFixedValue, 1);

        asC05Max.sprintf("Max:%d min / Unit", GetPowerSaveMaxMinute());
        EL<TLabel>("TfConfiguration", "labC05_4")->Caption=asC05Max;                                             //AI(ht9045-v899) 20260804: temp row label follows the per-customer limit
        EL<TLabel>("TfConfiguration", "labC05_4")->Visible=true;
        EL<TLabel>("TfConfiguration", "labC05_2")->Visible=false;                                                //AI(ht9045-v899) 20260804: hide the motor/vacuum/ATC row labels together with their controls
        EL<TLabel>("TfConfiguration", "labC05_6")->Visible=false;
        EL<TLabel>("TfConfiguration", "labC05_8")->Visible=false;
    }
    else if(IniConfig.bPowerSaveFunction)
    {
        EL<TGroupBox>("TfConfiguration", "gbC05")->Visible=true;
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgC05"),        &IniConfig.iPowersaveMode,                  ECInteger,  "OutPowerSave", "PowersaveMode",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Motor"),  &IniConfig.bC05_PowerSaveMotor,             ECBool,     "OutPowerSave", "UsepowerSaveMotor",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Temp"),   &IniConfig.bC05_PowerSaveTemp,              ECBool,     "OutPowerSave", "UsepowerSaveTemp",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Vacuum"), &IniConfig.bC05_PowerSaveVacuum,            ECBool,     "OutPowerSave", "UsepowerSaveVacuum",       bShow, bEnable, bReadFromFile, 0);      //Steven 20221215 : Power saving for vacuum pump
        if(ATC_SYSTEM!=eNewATCSystem)
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_ATC"),&IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bNoShow, bDisable, bFixedValue, 0);      //Ifor 20240401 : Power saving for ATC System
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_ATC"),&IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20240401 : Power saving for ATC System
        }
//        elConfig->Add(edC05_Motor,  &IniConfig.iHaltTime_Motor,                 ECInteger,  "OutPowerSave", "HaltTime_Motor",           bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //kevin 20110328設定省電模式時間    //Eastsun 20260710整合
//        elConfig->Add(edC05_Temp,   &IniConfig.iHaltTime_Temp,                  ECInteger,  "OutPowerSave", "HaltTime_Temp",            bShow, bEnable, bReadFromFile, 1,      false,  1,      200);    //Eastsun 20260710整合
//        elConfig->Add(edC05_Vacuum, &IniConfig.iC05HaltTime_Vacuum,             ECInteger,  "OutPowerSave", "HaltTime_Vacuum",          bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //Steven 20221215 : Power saving for vacuum pump    //Eastsun 20260710整合
//        elConfig->Add(edC05_ATC,    &IniConfig.iHaltTime_ATC,                   ECInteger,  "OutPowerSave", "HaltTime_ATC",             bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //Ifor 20240401 : Power saving for ATC System    //Eastsun 20260710整合
        if(USE_BU5_Function==false)                                             //Eastsun 20260710整合
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Motor"),  &IniConfig.iHaltTime_Motor,    ECInteger, "OutPowerSave", "HaltTime_Motor",  bShow, bEnable, bReadFromFile, 1, false, 1, 200);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Temp"),   &IniConfig.iHaltTime_Temp,     ECInteger, "OutPowerSave", "HaltTime_Temp",   bShow, bEnable, bReadFromFile, 1, false, 1, 200);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Vacuum"), &IniConfig.iC05HaltTime_Vacuum,ECInteger, "OutPowerSave", "HaltTime_Vacuum", bShow, bEnable, bReadFromFile, 1, false, 1, 200); //Steven 20221215 : Power saving for vacuum pump
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_ATC"),    &IniConfig.iHaltTime_ATC,      ECInteger, "OutPowerSave", "HaltTime_ATC",    bShow, bEnable, bReadFromFile, 1, false, 1, 200); //Ifor 20240401 : Power saving for ATC System
        }
        else                                                                        //Eastsun 20260710整合
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Motor"),  &IniConfig.iHaltTime_Motor,    ECInteger, "OutPowerSave", "HaltTime_Motor",  bShow, bEnable, bReadFromFile, 1, false, 1, 400);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Temp"),   &IniConfig.iHaltTime_Temp,     ECInteger, "OutPowerSave", "HaltTime_Temp",   bShow, bEnable, bReadFromFile, 1, false, 1, 400);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Vacuum"), &IniConfig.iC05HaltTime_Vacuum,ECInteger, "OutPowerSave", "HaltTime_Vacuum", bShow, bEnable, bReadFromFile, 1, false, 1, 400);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_ATC"),    &IniConfig.iHaltTime_ATC,      ECInteger, "OutPowerSave", "HaltTime_ATC",    bShow, bEnable, bReadFromFile, 1, false, 1, 400);
        }
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbC05")->Visible=false;
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgC05"),        &IniConfig.iPowersaveMode,                  ECInteger,  "OutPowerSave", "PowersaveMode",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Motor"),  &IniConfig.bC05_PowerSaveMotor,             ECBool,     "OutPowerSave", "UsepowerSaveMotor",        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Temp"),   &IniConfig.bC05_PowerSaveTemp,              ECBool,     "OutPowerSave", "UsepowerSaveTemp",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_Vacuum"), &IniConfig.bC05_PowerSaveVacuum,            ECBool,     "OutPowerSave", "UsepowerSaveVacuum",       bNoShow, bDisable, bFixedValue, 0);         //Steven 20221215 : Power saving for vacuum pump
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC05_ATC"),    &IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bNoShow, bDisable, bFixedValue, 0);         //Ifor 20240401 : Power saving for ATC System
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Motor"),  &IniConfig.iHaltTime_Motor,                 ECInteger,  "OutPowerSave", "HaltTime_Motor",           bNoShow, bDisable, bFixedValue, 1);         //kevin 20110328設定省電模式時間
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Temp"),   &IniConfig.iHaltTime_Temp,                  ECInteger,  "OutPowerSave", "HaltTime_Temp",            bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_Vacuum"), &IniConfig.iC05HaltTime_Vacuum,             ECInteger,  "OutPowerSave", "HaltTime_Vacuum",          bNoShow, bDisable, bFixedValue, 1);         //Steven 20221215 : Power saving for vacuum pump
        elConfig->Add(EL<TEdit>("TfConfiguration", "edC05_ATC"),    &IniConfig.iHaltTime_ATC,                   ECInteger,  "OutPowerSave", "HaltTime_ATC",             bNoShow, bDisable, bFixedValue, 1);         //Ifor 20240401 : Power saving for ATC System
    }

    if(IniConfig.bCanByPassIonFan)                                              //Steven 20111013
    {
        EL<TGroupBox>("TfConfiguration", "gbC06")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_01"), &IniConfig.bC06_ByPassIonFan[0],                ECBool,     "bCanByPassIonFan", "Fan01",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_02"), &IniConfig.bC06_ByPassIonFan[1],                ECBool,     "bCanByPassIonFan", "Fan02",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_03"), &IniConfig.bC06_ByPassIonFan[2],                ECBool,     "bCanByPassIonFan", "Fan03",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_04"), &IniConfig.bC06_ByPassIonFan[3],                ECBool,     "bCanByPassIonFan", "Fan04",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_05"), &IniConfig.bC06_ByPassIonFan[4],                ECBool,     "bCanByPassIonFan", "Fan05",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_06"), &IniConfig.bC06_ByPassIonFan[5],                ECBool,     "bCanByPassIonFan", "Fan06",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_07"), &IniConfig.bC06_ByPassIonFan[6],                ECBool,     "bCanByPassIonFan", "Fan07",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_08"), &IniConfig.bC06_ByPassIonFan[7],                ECBool,     "bCanByPassIonFan", "Fan08",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_09"), &IniConfig.bC06_ByPassIonFan[8],                ECBool,     "bCanByPassIonFan", "Fan09",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_10"), &IniConfig.bC06_ByPassIonFan[9],                ECBool,     "bCanByPassIonFan", "Fan10",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_11"), &IniConfig.bC06_ByPassIonFan[10],               ECBool,     "bCanByPassIonFan", "Fan11",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_12"), &IniConfig.bC06_ByPassIonFan[11],               ECBool,     "bCanByPassIonFan", "Fan12",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbC06")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_01"), &IniConfig.bC06_ByPassIonFan[0],                ECBool,     "bCanByPassIonFan", "Fan01",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_02"), &IniConfig.bC06_ByPassIonFan[1],                ECBool,     "bCanByPassIonFan", "Fan02",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_03"), &IniConfig.bC06_ByPassIonFan[2],                ECBool,     "bCanByPassIonFan", "Fan03",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_04"), &IniConfig.bC06_ByPassIonFan[3],                ECBool,     "bCanByPassIonFan", "Fan04",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_05"), &IniConfig.bC06_ByPassIonFan[4],                ECBool,     "bCanByPassIonFan", "Fan05",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_06"), &IniConfig.bC06_ByPassIonFan[5],                ECBool,     "bCanByPassIonFan", "Fan06",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_07"), &IniConfig.bC06_ByPassIonFan[6],                ECBool,     "bCanByPassIonFan", "Fan07",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_08"), &IniConfig.bC06_ByPassIonFan[7],                ECBool,     "bCanByPassIonFan", "Fan08",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_09"), &IniConfig.bC06_ByPassIonFan[8],                ECBool,     "bCanByPassIonFan", "Fan09",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_10"), &IniConfig.bC06_ByPassIonFan[9],                ECBool,     "bCanByPassIonFan", "Fan10",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_11"), &IniConfig.bC06_ByPassIonFan[10],               ECBool,     "bCanByPassIonFan", "Fan11",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC06_12"), &IniConfig.bC06_ByPassIonFan[11],               ECBool,     "bCanByPassIonFan", "Fan12",                bNoShow, bEnable, bFixedValue, 0);
    }

    EL<TCheckBox>("TfConfiguration", "cbC07")->Visible=(INSTALL_OCR);
    #ifdef MTK_Version
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC08"),        &IniConfig.bC08_SocketSensor,                   ECBool,     "Index", "bUseSocketSensor",                bShow, bDisable, bFixedValue, 1);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC08_1"),      &IniConfig.bC08_1_CheckSocketSensorDetectON,    ECBool,     "Index", "bCheckSocketSensorDetectON",      bShow, bDisable, bFixedValue, 1);       //Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能
    #else
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC08"),        &IniConfig.bC08_SocketSensor,                   ECBool,     "Index", "bUseSocketSensor",                bShow, bEnable, bReadFromFile, 0);      //kevin 20130504 使用socket sensor detect 功能 piggy back
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC08_1"),      &IniConfig.bC08_1_CheckSocketSensorDetectON,    ECBool,     "Index", "bCheckSocketSensorDetectON",      bShow, bEnable, bReadFromFile, 0);      //Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能
    #endif
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC09"),        &IniConfig.bC09_CarRecord,                      ECBool,     "Car Record", "bCarRecord",                 bNoShow, bEnable, bFixedValue, 0);      //wei 2013-12-09
    elConfig->Add(EL<TEdit>("TfConfiguration", "edC09"),        &IniConfig.iCarRecordDelayTime,                 ECInteger,  "Car Record", "iCarRecordDelayTime",        bShow, bEnable, bReadFromFile,  200,   false,  1,      200);            //wei 2013-12-09

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20170125 add   //Ifor 20150724 :Enable ESD COM ERR Report Function
    {
        if(ESD_Monitor==false && USE_NOVX3360==false)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC10"),&IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bNoShow, bEnable, bFixedValue, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC10"),&IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC10"),    &IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bShow, bEnable, bReadFromFile, 0);
    }

    #ifdef SOFT_SIMULTE
    IniConfig.bC11UseMonitorView=false;
    #else
    if(CosFunction.bUseMonitorView)                                             //JerryYang 20160621 錄影監視功能
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC11"),    &IniConfig.bC11UseMonitorView,                  ECBool,     "Function", "bC11UseMonitorView",           bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC11"),    &IniConfig.bC11UseMonitorView,                  ECBool,     "Function", "bC11UseMonitorView",           bNoShow, bEnable, bFixedValue, 0);
    #endif

    if(CosFunction.bUsePEModelFunction)                                         //Ifor 20170125 (Steven) add PE Mode Switch
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC12"),    &IniConfig.bC12UsePEMode,                       ECBool,     "Function", "bC12UsePEMode",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC12"),    &IniConfig.bC12UsePEMode,                       ECBool,     "Function", "bC12UsePEMode",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bNeedToRestartGroundWhenInitialStart)                        //Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC13"),    &IniConfig.bC13NeedToRestartGroundWhenInitialStart, ECBool, "Function", "bC13NeedToRestartGroundWhenInitialStart", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC13"),    &IniConfig.bC13NeedToRestartGroundWhenInitialStart, ECBool, "Function", "bC13NeedToRestartGroundWhenInitialStart", bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkC14"),       &IniConfig.bC14SaveBinDisplayLog,               ECBool,     "Function", "bC14SaveBinDisplayLog",        bShow, bEnable, bReadFromFile, 0);  //Steven 20220309 : BinDisplay Log

    if(CosFunction.bUseBarCoderChangeSetupFile)                                 //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC16"),    &IniConfig.bC16UseBarCoderChangeSetupFile,      ECBool,     "Function", "bC16UseBarCoderChangeSetupFile", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC16"),    &IniConfig.bC16UseBarCoderChangeSetupFile,      ECBool,     "Function", "bC16UseBarCoderChangeSetupFile", bNoShow, bEnable, bFixedValue, 0);

    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader)                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC17"),    &IniConfig.bC17UseLoaderColorSensor,            ECBool,     "Function", "bC20UseLoaderColorSensor", bShow,   bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC17"),   &IniConfig.iC17DelayTimes,                      ECInteger,  "Function", "iC20DelayTimes",           bShow,   bEnable, bReadFromFile, 3,      false,  1,      10);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC17_1"),  &IniConfig.bC17_1_SkipAlarm,                    ECBool,     "Function", "bC20_1_SkipAlarm",         bNoShow, bEnable, bReadFromFile, 0);
        EL<TGroupBox>("TfConfiguration", "grpC17")->Visible=true;
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC17"),    &IniConfig.bC17UseLoaderColorSensor,            ECBool,     "Function", "bC20UseLoaderColorSensor", bNoShow, bEnable, bFixedValue, 0);
        EL<TGroupBox>("TfConfiguration", "grpC17")->Visible=false;
    }

    if(USE_VibrationCommunication)                                              //JerryYang 20230814 : add震動馬達通訊調速版本
    {
        EL<TGroupBox>("TfConfiguration", "grpC21")->Visible=true;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP1"),       &IniConfig.iC21MotSp_HP1[0],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1",         bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP2"),       &IniConfig.iC21MotSp_HP2[0],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2",         bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT1"),      &IniConfig.iC21MotSp_SHT1[0],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1",        bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT2"),      &IniConfig.iC21MotSp_SHT2[0],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2",        bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto1"),     &IniConfig.iC21MotSp_AUTO1[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto2"),     &IniConfig.iC21MotSp_AUTO2[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto3"),     &IniConfig.iC21MotSp_AUTO3[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP1_m"),     &IniConfig.iC21MotSp_HP1[1],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1_M",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP2_m"),     &IniConfig.iC21MotSp_HP2[1],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2_M",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT1_m"),    &IniConfig.iC21MotSp_SHT1[1],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1_M",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT2_m"),    &IniConfig.iC21MotSp_SHT2[1],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2_M",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto1_m"),   &IniConfig.iC21MotSp_AUTO1[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto2_m"),   &IniConfig.iC21MotSp_AUTO2[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto3_m"),   &IniConfig.iC21MotSp_AUTO3[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP1_b"),     &IniConfig.iC21MotSp_HP1[2],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1_B",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_HP2_b"),     &IniConfig.iC21MotSp_HP2[2],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2_B",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT1_b"),    &IniConfig.iC21MotSp_SHT1[2],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1_B",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_SHT2_b"),    &IniConfig.iC21MotSp_SHT2[2],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2_B",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto1_b"),   &IniConfig.iC21MotSp_AUTO1[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto2_b"),   &IniConfig.iC21MotSp_AUTO2[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtC21_Auto3_b"),   &IniConfig.iC21MotSp_AUTO3[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpC21")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)            //JerryYang 20250120 : add
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC24"),    &IniConfig.bC24InitialStartCheckCylinder,           ECBool,     "Function", "bC24InitialStartCheckCylinder",       bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC24"),    &IniConfig.bC24InitialStartCheckCylinder,           ECBool,     "Function", "bC24InitialStartCheckCylinder",       bNoShow, bEnable, bFixedValue, 0);
    }

}

// golden cConfiguration.cpp:1312  TfConfiguration::InitConfigEdtList_ItemD()
static void IC_InitConfigEdtList_ItemD()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(USE_IO_CHANGE_TOQUE==true)                                               //jou 2013-07-12 兩個funtion不能同時開
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD01"),        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bDisable, bFixedValue, 0);
    else if(CUSTOMER_CODE==CC_KYEC_XILINX &&
            IniConfig.bChangeKitNoHardStop==true &&
            IniConfig.bRemeberAutoHeight==true)                                 //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD01"),        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD01"),        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bEnable, bReadFromFile, LastSet.bEnableReadTorque?"1":"0");

    #ifndef ASE_KaohSiung                                                       //KaiHuang 20201221 Add
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01"),        &IniConfig.iD01ReadTorqueTimeCount,         ECInteger,  "Index", "iD01ReadTorqueTimeCount",         bShow, bEnable, bReadFromFile,  200,   false,  0,      1000);           //0~1000次
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD01_1"),      &IniConfig.bD01_1EnableReadAndCheckTorque,  ECBool,     "Index", "bD01_1EnableReadAndCheckTorque",  bShow  , bEnable , bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01_Xilinx"), &IniConfig.dD01ReadTorque,                  ECDouble,   "Index", "dD01ReadTorque",                  bNoShow, bDisable, bFixedValue, 10.0,  false,  10.0,   100.0);          //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01DelayTime_Xilinx"),&IniConfig.dD01ReadTorqueDelayTime, ECDouble,   "Index", "dD01ReadTorqueDelayTime",         bNoShow, bDisable, bFixedValue, 1.0,   false,  0.01,   5.0);            //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    #else
        //借來用
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01"),        &IniConfig.iD01ReadTorqueTimeCount,         ECInteger,  "Index", "iD01ReadTorqueTimeCount",         bNoShow, bDisable, bReadFromFile,  200,   false,  0,      1000);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD01_1"),      &IniConfig.bD01_1EnableReadAndCheckTorque,  ECBool,     "Index", "bD01_1EnableReadAndCheckTorque",  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01_Xilinx"), &IniConfig.dD01ReadTorque,                  ECDouble,   "Index", "dD01ReadTorque",                  bShow,   bEnable,  bReadFromFile,  3.0,   false,  0.1,    100.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD01DelayTime_Xilinx"),&IniConfig.dD01ReadTorqueDelayTime, ECDouble,   "Index", "dD01ReadTorqueDelayTime",         bShow,   bEnable,  bReadFromFile,  0.1,   false,  0.1,   10.0);
    #endif
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD02"),        &IniConfig.bD02OffReadTorqueDuringTest,         ECBool,     "Index", "bD02OffReadTorqueDuringTest",     bShow, bEnable, bReadFromFile,  LastSet.bOffReadTorqueDuringTest?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD04"),        &IniConfig.bD04MinForceByFile,                  ECBool,     "Contact Force", "bD04MinForceByFile",      bShow, bEnable, bReadFromFile,  0);                                     //Steven 20190314 : Min force is read from file
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04"),       &IniConfig.dD04MinForceByFile,                  ECDouble,   "Contact Force", "dD04MinForceByFile",      bShow, bEnable, bReadFromFile,  1.0,   false,  1.0,    120.0);          //Steven 20190314 : Min force is read from file
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04_20mm"),  &IniConfig.dD04MinForceByFile_20mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_20mm", bShow, bEnable, bReadFromFile,  0.5,   false,  1.0,    120.0);          //Steven 20220822 : Min force for different SLK
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04_30mm"),  &IniConfig.dD04MinForceByFile_30mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_30mm", bShow, bEnable, bReadFromFile,  1.5,   false,  1.0,    120.0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04_40mm"),  &IniConfig.dD04MinForceByFile_40mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_40mm", bShow, bEnable, bReadFromFile,  4.0,   false,  1.0,    120.0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04_60mm"),  &IniConfig.dD04MinForceByFile_60mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_60mm", bShow, bEnable, bReadFromFile,  8.0,   false,  1.0,    120.0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD04_80mm"),  &IniConfig.dD04MinForceByFile_80mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_80mm", bShow, bEnable, bReadFromFile,  15.0,  false,  1.0,    180.0);          //Ifor 20240620 : add缸徑 80
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD05"),        &IniConfig.bD05ContactCountAlarm,               ECBool,     "Index", "bD05ContactCountAlarm",           bNoShow, bEnable, bFixedValue,  0);                                     //wei 20170327
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD05"),        &IniConfig.iD05_ContactCountAlarm,              ECInteger,  "Contact Force", "iD05_ContactCountAlarm",  bNoShow, bDisable, bFixedValue, 1,     false,  0,      5000);           //wei 20170327 add
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD05_1"),      &IniConfig.bD05_1SaveSocketCntByHandler,        ECBool,     "Index", "bD05_1SaveSocketCntByHandler",    bShow, bEnable, bReadFromFile,  0);                                     //Steven 20250807 : By handler save contact count
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD06"),        &IniConfig.bD06ContactOffsetDefaultValue,       ECBool,     "Index", "bD06ContactOffsetDefaultValue",   bShow, bEnable, bReadFromFile,  0);                                     //JimmyChiu 20220120 add ContactOffsetDefaultValue
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD06"),        &IniConfig.dD06_ContactOffsetDefaultValue,      ECDouble,   "Contact Force", "dD06_ContactOffsetDefaultValue",   bShow, bEnable, bReadFromFile,  3,     false,  0,      10);    //JimmyChiu 20220120 add ContactOffsetDefaultValue
    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD10"),    &IniConfig.bD10ManualHeightComptibleWithNS,     ECBool,     "Index", "bD10ManualHeightComptibleWithNS", bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD10"),    &IniConfig.bD10ManualHeightComptibleWithNS,     ECBool,     "Index", "bD10ManualHeightComptibleWithNS", bShow, bEnable, bReadFromFile, LastSet.bManualHeightComptibleWithNS?"1":"0");

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD11"),        &IniConfig.bD11NoIcSkipAutoHeight,              ECBool,     "Index", "bNoIcSkipAutoHeight",             bShow, bEnable, bReadFromFile, 0);      //Steven 20110726
//    if(IniConfig.bSPILFunction==true)                                         //JerryYang 20241011 : Mark掉 //Steven 20140220 : 用生產ep去做蝦頭auto high
//        elConfig->Add(cbD12,    &IniConfig.bD12UseDeviceFormPressDoShtHeight,   ECBool,     "Index", "bUseDeviceFormPressDoShtHeight",  bShow, bDisable, bFixedValue, 1);
//    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD12"),    &IniConfig.bD12UseDeviceFormPressDoShtHeight,   ECBool,     "Index", "bUseDeviceFormPressDoShtHeight",  bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cdD13"),        &IniConfig.bD13CheckIndexHomeSensor,            ECBool,     "Index", "bCheckIndexHomeSensor",           bShow, bEnable, bReadFromFile, 0);      //Steven 20140828 : 歸零後檢查Index位置
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD14"),        &IniConfig.bD14_AutoHeightUseSetTorque,         ECBool,     "Index", "bD14_AutoHeightUseSetTorque",     bShow, bEnable, bReadFromFile, 0);      //Steven 20141105 : 使用指定的扭力進行Auto Height
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD14"),        &IniConfig.iD14_AutoHeightUseSetTorque,         ECInteger,  "Index", "iD14_AutoHeightUseSetTorque",     bShow, bEnable, bReadFromFile,  15,    false,  5,      30);             //kevin 20210730 10->5
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD15"),        &IniConfig.bD15_AutoContactTest,                ECBool,     "Index", "bD15_AutoContactTest",            bShow, bEnable, bReadFromFile, 0);      //Steven 20150224 : Auto Contact Test
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD16"),        &IniConfig.bD16_StepContactTest,                ECBool,     "Index", "bD16_StepContactTest",            bShow, bEnable, bReadFromFile, 0);      //Steven 20150811 : Step by Step Contact Test
    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgD17"),        &IniConfig.iD17_UseHardwareHeightToContact,     ECInteger,  "Index", "bD17_UseHardwareHeightToContact", bShow, bEnable, bReadFromFile, 0);      //Steven 20170411 (wei) : SCK的SIP怕刮傷所以Contact Height使用硬體高度
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD17_3"),     &IniConfig.dD17_3CheckEPLeakage,                ECDouble,   "Index", "dD17_3CheckEPLeakage",            bShow, bEnable, bReadFromFile, 1.0, false,  1.0,    3.0);    //Steven 20231025 : 充氣跟不充氣都做一次auto height, 然後檢查有沒有漏氣
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkD18"),       &IniConfig.bD18_AutoHeightWhenChangeRecipe,     ECBool,     "Index", "bD18_AutoHeightWhenChangeRecipe", bShow, bEnable, bReadFromFile, 0);      //Steven 20221013 : ATK希望更換工作檔時, 需要提示有沒有做Auto Height.

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD21"),        &IniConfig.bD21EnableFinishTestUpWait,          ECBool,     "Tester", "bD21EnableFinishTestUpWait",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD21_mm"),     &IniConfig.iD21FinishTestUpWaitHeight,          ECInteger,  "Tester", "iD21FinishTestUpWaitHeight",     bShow, bEnable, bReadFromFile, LastSet.iFinishTestUpWaitHeight,       false,  0,      10, EUuMToMM);  //0~10mm
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD21_Sec"),    &IniConfig.iD21FinishTestUpWaitTime,            ECInteger,  "Tester", "iD21FinishTestUpWaitTime",       bShow, bEnable, bReadFromFile, (int)LastSet.iD21FinishTestUpWaitTime, false,  0.1, 100.0, EUMSToSec);  //0.1~100Sec

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_1"),      &IniConfig.bD22SupportMultiDoubleContact,       ECBool,     "Index", "bD22SupportMultiDoubleContact",   bShow, bEnable, bReadFromFile, LastSet.bSupportMultiDoubleContact?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_2"),      &IniConfig.bD22DoubleContactNoNeedReContact,    ECBool,     "Index", "bDoubleContactNoNeedReContact",   bShow, bEnable, bReadFromFile, 0);      //Steven 20131202 : Double Contact不需要Index Arm上下動
    elConfig->Add(EL<TComboBox>("TfConfiguration", "coD22"),        &IniConfig.iD22DoubleContactCount,              ECInteger,  "Index", "iD22DoubleContactCount",          bShow, bEnable, bReadFromFile, LastSet.iDoubleContactCount);

    if(CUSTOMER_CODE==CC_AnalogDevice_Phil)                                     //JerryYang 20230721 : Analog double contact鎖定送0X42
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_3_"), &IniConfig.bD22DoubleContactUseDiffSRQ,         ECBool,     "Index", "bD22DoubleContactUseDiffSRQ",     bShow, bDisable, bFixedValue, 1);       //Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD22_3"),  &IniConfig.iD23_DoubleContactSRQ,               ECInteger,  "Index", "iD23_DoubleContactSRQ",           bShow, bDisable, bFixedValue, 2);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_3_"), &IniConfig.bD22DoubleContactUseDiffSRQ,         ECBool,     "Index", "bD22DoubleContactUseDiffSRQ",     bShow, bEnable, bReadFromFile, 0);       //Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD22_3"),  &IniConfig.iD23_DoubleContactSRQ,               ECInteger,  "Index", "iD23_DoubleContactSRQ",           bShow, bEnable, bReadFromFile, 0);
    }

    /*  //Sam 20231117 : 整合到 QA 模式 Mark
    if(CosFunction.bVerifyMode)                                                 //Sam 20221012 : 新增 VerifyMode 功能
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_3"),  &IniConfig.bD22VerifyMode,                      ECBool,     "Index", "bD22VerifyMode",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_3"),  &IniConfig.bD22VerifyMode,                      ECBool,     "Index", "bD22VerifyMode",                  bNoShow, bDisable, bFixedValue, 0);
    */
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20230909 : pass bin也可以設定Double contact
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_4"), &IniConfig.bD22_4_PassBinCanDoubleContact,       ECBool,     "Index", "bD22_4_PassBinCanDoubleContact",  bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD22_4"), &IniConfig.bD22_4_PassBinCanDoubleContact,       ECBool,     "Index", "bD22_4_PassBinCanDoubleContact",  bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD23"),        &IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting, ECBool, "Index", "bEveryDeviceDoubleContactFirstNoTestting", bShow, bEnable, bReadFromFile, 0); //ChungHung 20140709 add for SPIL
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD23"),        &IniConfig.iD23_MultiContactCount,              ECInteger,  "Index", "iMultiContactCount",              bShow, bEnable, bReadFromFile,  2,     false,  2,      30);             //Steven 20151001 : Add for TSMC

    if(IniConfig.bIndexEveryTimeCheckEP==true)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD24"),    &IniConfig.bD24EnableEPCheckFuntion,            ECBool,     "EP Check", "EnableEPCheckFuntion",         bShow, bEnable, bReadFromFile, 0);
    else                                                                        ECBool,
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD24"),    &IniConfig.bD24EnableEPCheckFuntion,            ECBool,     "EP Check", "EnableEPCheckFuntion",         bNoShow, bEnable, bFixedValue, 0);

    if(EP_Install==3 || EP_Install==5)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26"),    &IniConfig.bD26EnableEPEncoderRange,            ECBool,     "Index", "bEnableEPEncoderRange",           bShow, bEnable, bReadFromFile, 0);      //ChungHung 20111217
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_1"),  &IniConfig.bD26EnableEPLog,                     ECBool,     "Index", "bEnableEPLog",                    bShow, bEnable, bReadFromFile, 0);      //Ifor 20150706 :新增EP LOG 開關
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_2"),  &IniConfig.bD26EnableEncodeShow,                ECBool,     "Index", "bEnableEncodeShow",               bShow, bEnable, bReadFromFile, 0);      //Ifor 20160304 :新增EP encoder value Show 開關
        if(CosFunction.bHiSiliconFunction && CUSTOMER_CODE==CC_ASE_KaohSiung)   //kevin 20200313 add eP range 5
            elConfig->Add(EL<TEdit>("TfConfiguration", "edD26"),    &IniConfig.iD26EPEncoderRange,              ECInteger,  "Index", "iEPEncoderRange",                 bShow, bDisable, bFixedValue,  5);     //kevin 20200313 add eP range 5
        else
            elConfig->Add(EL<TEdit>("TfConfiguration", "edD26"),    &IniConfig.iD26EPEncoderRange,              ECInteger,  "Index", "iEPEncoderRange",                 bShow, bEnable, bReadFromFile,  100,   false,  10,     100);            //ChungHung 20111217
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26"),    &IniConfig.bD26EnableEPEncoderRange,            ECBool,     "Index", "bEnableEPEncoderRange",           bNoShow, bEnable, bFixedValue, 0);      //ChungHung 20111217
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_1"),  &IniConfig.bD26EnableEPLog,                     ECBool,     "Index", "bEnableEPLog",                    bNoShow, bEnable, bFixedValue, 0);      //Ifor 20150706 :新增EP LOG 開關
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_2"),  &IniConfig.bD26EnableEncodeShow,                ECBool,     "Index", "bEnableEncodeShow",               bNoShow, bEnable, bFixedValue, 0);      //Ifor 20160304 :新增EP encoder value Show 開關
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD26"),    &IniConfig.iD26EPEncoderRange,                  ECInteger,  "Index", "iEPEncoderRange",                 bNoShow, bDisable, bFixedValue, 100);
    }

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                            //AI(ht9045-v899) 20260504: V899 wrote ==true (==1) which dropped Multi EP path; restored V874 behavior.
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_3"),  &IniConfig.bD26_3EnableDualEPEncoderRange,      ECBool,     "Index", "bD26_3EnableDualEPEncoderRange",  bShow, bEnable, bReadFromFile, 0);
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)
        {
            EL<TCheckBox>("TfConfiguration", "cbD26_3")->Caption="[D26_3] Enable Dual EP encoder range +,-                   Kg";
        }
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD26_3"),  &IniConfig.iD26_3DualEPEncoderRange,            ECInteger,  "Index",  "iD26_3DualEPEncoderRange",       bShow, bEnable, bReadFromFile,  100,   false,  1,      100);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD26_3"),  &IniConfig.bD26_3EnableDualEPEncoderRange,      ECBool,      "Index", "bD26_3EnableDualEPEncoderRange", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD26_3"),  &IniConfig.iD26_3DualEPEncoderRange,            ECInteger,   "Index", "iD26_3DualEPEncoderRange",       bNoShow, bDisable, bFixedValue, 100);
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  //Richard 20230426 : JECT ADD 固定值改為百分比
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD26"),    &IniConfig.iD26_3FixValueOrPercentage,          ECInteger,  "Index", "iD26_3FixValueOrPercentage",      bShow, bEnable, bReadFromFile, 0); //Richard 20230428 : EP固定值或百分比
    else
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD26"),    &IniConfig.iD26_3FixValueOrPercentage,          ECInteger,  "Index", "iD26_3FixValueOrPercentage",      bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ATMEL || CUSTOMER_CODE==CC_QUALCOMM)                   //Steven 20131203 : bD27UseSingleSite85kg ATK強制開啟
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD27"),    &IniConfig.bD27UseSingleSite85kg,               ECBool,     "Index", "UseSingleSite85kg",               bShow, bDisable, bFixedValue, 1);        //2012-01-03    Dell 在1X2模式下關Site能達85kg
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD27"),    &IniConfig.bD27UseSingleSite85kg,               ECBool,     "Index", "UseSingleSite85kg",               bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20251017 Add:ASE SG 要求強制關閉[D28]
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD28"),        &IniConfig.bD28MaxForceLimitByDiameter,         ECBool,     "Index", "bD28MaxForceLimitByDiameter",     bShow, bDisable, bFixedValue, 0);      //Steven 20200813 : 用缸徑計算最大壓力
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD28"),        &IniConfig.bD28MaxForceLimitByDiameter,         ECBool,     "Index", "bD28MaxForceLimitByDiameter",     bShow, bDisable, bFixedValue, 1);      //Steven 20200813 : 用缸徑計算最大壓力

    if(CosFunction.bHaveIndexContactDelay)                                      //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD29"),    &IniConfig.bD29EnableIndexContactDelay,         ECBool,      "Index", "bEnableIndexContactDelay",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD29"),    &IniConfig.bD29EnableIndexContactDelay,         ECBool,      "Index", "bEnableIndexContactDelay",       bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD30"),        &IniConfig.bD30EnableSiteModeSelect,            ECBool,     "Index", "bD30EnableSiteModeSelect",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD31"),    &IniConfig.bD31RTCChangeRecipeNeedreCreateModel,    ECBool,     "Configuration", "bRTCChangeRecipeNeedreCreateModel", bShow, bEnable, bReadFromFile, 0);//jou 2012-03-01 [D31] RTC Change Recipe Need reCreate RTC Model
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD32"),    &IniConfig.bD32_35TrayPitchIndexDOnMoreThen500MS,   ECBool,     "Index", "b35TrayPitchIndexDOnMoreThen500MSec", bShow, bEnable, bReadFromFile, 0);      //Steven 20120727 : Tray Pitch 大於35mm的話 Index吹氣至少要0.5秒
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD33"),        &IniConfig.bD33RTCInitStartVerify,              ECBool,     "Configuration", "bRTCInitStartVerify",     bShow, bEnable, bReadFromFile, 0);      //Handler Use Model Verify
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD34"),        &IniConfig.bD34GailDMCProtection,               ECBool,     "Index", "bGailDMCProtection",              bShow, bEnable, bReadFromFile, 0);      //ChungHung 20131230 add for ATK    //Steven 20140315 : 改成可以選擇[D34]

    if(IniConfig.bHaveRTCCheckSiteMap)                                          //Steven 20140513
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD35"),    &IniConfig.bD35RTCCheckSiteMap,                 ECBool,     "Index", "bRTCCheckSiteMap",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD35"),    &IniConfig.bD35RTCCheckSiteMap,                 ECBool,     "Index", "bRTCCheckSiteMap",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bRTCAutoModelVerify)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36"),    &IniConfig.bD36EnableRTCAutoModelVerify,        ECBool,     "Index", "bEnableRTCAutoModelVerify",           bShow, bEnable, bReadFromFile, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36_1"),  &IniConfig.bD36_1EnableRTCAutoModelVerifyLive,  ECBool,     "Index", "bD36_1EnableRTCAutoModelVerifyLive",  bShow, bEnable, bReadFromFile, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36_2"),  &IniConfig.bD36_2AfterOneCycleNeedAutoVerify,   ECBool,     "Index", "bD36_2AfterOneCycleNeedAutoVerify",   bShow, bEnable, bReadFromFile, 0);      //JerryYang 20201116 : One cycle後要執行RTC auto verify
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36"),    &IniConfig.bD36EnableRTCAutoModelVerify,        ECBool,     "Index", "bEnableRTCAutoModelVerify",           bNoShow, bEnable, bFixedValue, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36_1"),  &IniConfig.bD36_1EnableRTCAutoModelVerifyLive,  ECBool,     "Index", "bD36_1EnableRTCAutoModelVerifyLive",  bNoShow, bEnable, bFixedValue, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD36_2"),  &IniConfig.bD36_2AfterOneCycleNeedAutoVerify,   ECBool,     "Index", "bD36_2AfterOneCycleNeedAutoVerify",   bNoShow, bEnable, bFixedValue, 0);      //JerryYang 20201116 : One cycle後要執行RTC auto verify
    }

    if(CosFunction.bManualProcess)                                              //ChungHung 20150526 add for QualComm US
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD37"),    &IniConfig.bD37EnableManualProcess,             ECBool,     "Index", "bEnableManualProcess",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD37"),    &IniConfig.bD37EnableManualProcess,             ECBool,     "Index", "bEnableManualProcess",                bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD38"),        &IniConfig.bD38IndexPutICToShtNoWaitMotion,     ECBool,     "Index", "bD38IndexPutICToShtNoWaitMotion",     bShow, bEnable, bReadFromFile, 0);      //Steven 20181228 : Add Index Action   //ChungHung 20171116 modify for Index Action

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
       CUSTOMER_CODE==CC_AMKOR_China || IniConfig.bKoreaFunction ||
       CUSTOMER_CODE==CC_KYEC_LEE    ||                                         //wei 20150624 KYEC強制打開
       IniConfig.bMaximFunction ||                                              //JerryYang 20190522 Maxim統一軟體功能
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 (Steven) add QUALCOMM
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD40"),    &IniConfig.bD40IndexICFallDownMustPressFMotorDown, ECBool,  "Index", "bD40IndexICFallDownMustPressFMotorDown", bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD40"),    &IniConfig.bD40IndexICFallDownMustPressFMotorDown, ECBool,  "Index", "bD40IndexICFallDownMustPressFMotorDown", bShow, bEnable, bReadFromFile, LastSet.bIndexICFallDownMustPressFMotorDown?"1":"0");
    }

    if(IniConfig.bTestIcCheckInContact==true)                                   //ChungHung 20140327 add by Customer
    {
        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                                   //Jeff 20241001 add for enable and disable checkvacum in contact
           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        {
           EL<TPanel>("TfConfiguration", "pnlD41")->Visible=true;

           EL<TLabel>("TfConfiguration", "labD41_1")->Visible=false;
           EL<TLabel>("TfConfiguration", "labD41_3")->Visible=false;
           EL<TLabel>("TfConfiguration", "labD41_2")->Visible=false;
           EL<TLabel>("TfConfiguration", "labD41")->Visible=false;
        }
        else
        {
           EL<TPanel>("TfConfiguration", "pnlD41")->Visible=false;
        }
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD41"),    &IniConfig.dD41SocketInitialCheckOffset,        ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bNoShow, bDisable, bFixedValue, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 10.0);        //Steven 20100818 : [D41]
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD41_2"),  &IniConfig.fIndexCheckOffset,                   ECDouble,   "Index", "fIndexCheckOffset",                       bNoShow, bDisable, bFixedValue, 0.0, false, 0.0, 10.0);       //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD41"),    &IniConfig.iD41SocketInitialICCheckPosition,    ECInteger,  "Index", "iD41SocketInitialICCheckPosition",        bNoShow, bDisable, bFixedValue, LastSet.iD41SocketInitialICCheckPosition);
    }
    else
    {
        EL<TPanel>("TfConfiguration", "pnlD41")->Visible=true;
        if(CosFunction.bIndexZDownToAboveSocket)
        {
            if(IniConfig.bIndexArm2SupplyLight)
                elConfig->Add(EL<TEdit>("TfConfiguration", "edD41"),&IniConfig.dD41SocketInitialCheckOffset,    ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bShow, bEnable, bReadFromFile, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 20.0);        //Steven 20100818 : [D41]
            else
                elConfig->Add(EL<TEdit>("TfConfiguration", "edD41"),&IniConfig.dD41SocketInitialCheckOffset,    ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bShow, bEnable, bReadFromFile, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 10.0);        //Steven 20100818 : [D41]

            elConfig->Add(EL<TEdit>("TfConfiguration", "edD41_2"),  &IniConfig.fIndexCheckOffset,               ECDouble,   "Index", "fIndexCheckOffset",                       bNoShow, bDisable, bFixedValue, 0.0, false);
        }
        else
        {
            double dMaxOffset=10.0;
            AnsiString szDirSecurity=CheckFile(AuthPath, "Security_new.def");   //Jimmychiu 20231128 : Edit Max index check offset by file
            dMaxOffset=CheckRange(CheckAndReadIniData(szDirSecurity, "Input Limit", "D41IndexCheckOffset", 10.0), 5.0, 100.0);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edD41"),&IniConfig.dD41SocketInitialCheckOffset,        ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bNoShow, bDisable, bFixedValue, LastSet.dD41SocketInitialCheckOffset);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edD41_2"),  &IniConfig.fIndexCheckOffset,               ECDouble,   "Index", "fIndexCheckOffset",                       bShow, bEnable, bReadFromFile, 0.0, false, 0.0, dMaxOffset);       //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
        }

        elConfig->Add(EL<TComboBox>("TfConfiguration", "coD41"),    &IniConfig.iD41SocketInitialICCheckPosition,    ECInteger,  "Index", "iD41SocketInitialICCheckPosition",        bShow, bEnable, bReadFromFile, LastSet.iD41SocketInitialICCheckPosition);

        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                 //kevin 20200716 add ASE_KH 卡輸入
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD41"),&IniConfig.bD41CheckbySetup,                    ECBool,     "Index", "bD41CheckbySetup",                    bShow, bEnable, bReadFromFile, 0); //kevin 20200801 by setup 功能只能用一次
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD41"),&IniConfig.bD41CheckbySetup,                    ECBool,     "Index", "bD41CheckbySetup",                    bNoShow, bDisable, bFixedValue, 0);                         //kevin 20200801 by setup 功能只能用一次
        }
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD42"),        &IniConfig.bD42IndexPickICShuttlePause,         ECBool,     "Index", "bD42IndexPickICShuttlePause",         bShow, bEnable, bReadFromFile, LastSet.bIndexPickICShuttlePause?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD43"),        &IniConfig.bD43IndexDropErrorCanRetryandSkip,   ECBool,     "Index", "bIndexDropErrorCanRetryandSkip",      bShow, bEnable, bReadFromFile, 0);  //ChungHung 20120717 add Index Drop Error Can Retry and Start
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD43_1"),      &IniConfig.bD43AutoRetryWhenIndexPickErr,       ECBool,     "Index", "bD43AutoRetryWhenIndexPickErr",       bShow, bEnable, bReadFromFile, 0);  //Steven 20170105 : Index吸取異常要退出來用Shuttle Sensor檢查後 再進去吸一次
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD43_2"),      &IniConfig.bD43IndexPickErrCheckSocket,         ECBool,     "Index", "bD43IndexPickErrCheckSocket",         bShow, bEnable, bReadFromFile, 0);  //Steven 20190115 : SCC要求吸取異常要檢查Socket

    if(CUSTOMER_CODE==CC_ASE_CL         ||                                      //JerryYang 20210129 : ASE-CL松諭要求強制開啟
       CUSTOMER_CODE==CC_SIGURD_HUKOU   ||                                      //Sam 20220527 : 矽格湖口 Jonas 要求強制開啟
       CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Sam 20240606 : 北興廠也要強制開啟
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD44"),    &IniConfig.bD44CheckIndexICDestroy,             ECBool,     "Index", "bD44CheckIndexICDestroy",             bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD44"),    &IniConfig.bD44CheckIndexICDestroy,             ECBool,     "Index", "bD44CheckIndexICDestroy",             bShow, bEnable, bReadFromFile, LastSet.bCheckIndexICDestroy?"1":"0");
    }

    if(INDEX_SUCKER_TYPE==1)                                                    //kevin 20121023 設定範圍避免過久壓降掉料
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD44"),    &IniConfig.iD44TestHeadCheckVacuumTime,         ECInteger,  "Index", "iD44TestHeadCheckVacuumTime",         bShow, bEnable, bReadFromFile, (int)LastSet.iTestHeadCheckVacuumTime, false,  0.1, 0.15, EUMSToSec);  //0.1~100Sec
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD44"),    &IniConfig.iD44TestHeadCheckVacuumTime,         ECInteger,  "Index", "iD44TestHeadCheckVacuumTime",         bShow, bEnable, bReadFromFile, (int)LastSet.iTestHeadCheckVacuumTime, false,  0.1, 100.0, EUMSToSec);  //0.1~100Sec

//    elConfig->Add(edD44_Height, &IniConfig.iD44TestHeadCheckVacuumHeight,       ECInteger,  "Index", "TestHeadCheckVacuumHeight",           bShow, bEnable, bReadFromFile,  0,     false,  0,      2000);        //wei 20150609 延遲破壞高度
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD45"),        &IniConfig.bD45UseOutArmCheckIndex,             ECBool,     "Index", "bD45UseOutArmCheckIndex",             bShow, bEnable, bReadFromFile, "1");

    elConfig->Add(EL<TEdit>("TfConfiguration", "edD46"), &IniConfig.iD46WaitIndexDestroyTime,                   ECInteger,  "Index", "iD46WaitIndexDestroyTime",            bShow, bEnable, bReadFromFile,  (int)LastSet.iWaitIndexDestroyTime,     false,  5,      15);

    if(CUSTOMER_CODE==CC_ASE_CL)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD47"),    &IniConfig.bD47EnableSocketPurgeFunction,       ECBool,     "Index", "bD47EnableSocketPurgeFunction",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD47"),    &IniConfig.bD47EnableSocketPurgeFunction,       ECBool,     "Index", "bD47EnableSocketPurgeFunction",       bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TComboBox>("TfConfiguration", "cbbD47"),       &IniConfig.iD47SocketPurgeType,                 ECInteger,  "Index", "iD47SocketPurgeType",                 bShow, bEnable, bReadFromFile, 0);    //Steven 20190703 : Socket Purge include shuttle
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD47_Count"),  &IniConfig.iD47SocketPurgeCount,                ECInteger,  "Index", "iD47SocketPurgeCount",                bShow, bEnable, bReadFromFile,  (int)LastSet.iWaitIndexDestroyTime, false, 0,    10);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD47_Time"),   &IniConfig.iD47SocketPurgeTime,                 ECInteger,  "Index", "iD47SocketPurgeTime",                 bShow, bEnable, bReadFromFile,  (int)LastSet.iSocketPurgeTime,      false, 0,    10);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edD47_3"),     &LastSet.iD47SocketTestedCount,                 ECInteger,  "Index", "iD47SocketTestedCount",               bShow, bEnable, bReadFromFile,  10,    false,  0   , 1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD47_5"),     &IniConfig.dD47_5_ContactOffset,                ECDouble,   "Index", "dD47_5_ContactOffset",                bShow, bEnable, bReadFromFile,  0.0,   false,  5.0, -5.0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtD47_6"),     &IniConfig.dD47_6_ShuttleOffset,                ECDouble,   "Index", "dD47_6_ShuttleOffset",                bShow, bEnable, bReadFromFile,  0.0,   false,  5.0, -5.0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD48"),        &IniConfig.bD48PowerOffEmgCanNotUseZ1Z2,        ECBool, "Index", "bD48DisablePowerOffEmgStopZ1Z2KeyButton", bShow, bEnable, bReadFromFile, LastSet.bD48PowerOffEmgCanNotUseZ1Z2?"1":"0");
    if(CosFunction.bRTCAlarmSetIndexToErrBin)                                   //JerryYang 20160712 for 力成發生RTC Alarm時把Index上所有IC設為Errorbin
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD49"),    &IniConfig.bD49RTCAlarmSetIndexToErrBin,        ECBool,     "Index", "bD49RTCAlarmSetIndexToErrBin",        bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD49"),    &IniConfig.bD49RTCAlarmSetIndexToErrBin,        ECBool,     "Index", "bD49RTCAlarmSetIndexToErrBin",        bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexPickErrSkipNeedCheckVac)                               //jou 20171031 (Steven) : 增加開關JSCC要求index pick up error 需再慢速下降吸一次
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD50"),    &IniConfig.bD50IndexPickErrSkipNeedCheckVac,    ECBool,     "Index", "bD50IndexPickErrSkipNeedCheckVac",    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD50"),    &IniConfig.bD50IndexPickErrSkipNeedCheckVac,    ECBool,     "Index", "bD50IndexPickErrSkipNeedCheckVac",    bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD51"), &IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear, ECBool,     "Index", "bD51UseOnecycleCleanOutFinishTestArmAtRear", bShow, bEnable, bReadFromFile, LastSet.bUseOnecycleCleanOutFinishTestArmAtRear?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD52"),        &IniConfig.bD52InterFaceErrHeadNeedUp,          ECBool,     "Index", "bD52InterFaceErrHeadNeedUp",          bShow, bEnable, bReadFromFile, LastSet.bInterFaceErrHeadNeedUp?"1":"0");
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD53"),        &IniConfig.bD53CCDLightOn,                      ECBool,     "Index", "bD53CCDLightOn",                      bShow, bEnable, bReadFromFile, LastSet.bCCDLightOn?"1":"0");

    elConfig->Add(EL<TEdit>("TfConfiguration", "edD53"),        &IniConfig.iD53LightOnMin,                      ECInteger,  "Index", "iD53LightOnMin",                      bShow, bEnable, bReadFromFile,  (int)LastSet.iLightOnMin,    false,  1,      10000);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD54"),        &IniConfig.bD54SlowDown,                        ECBool,     "Index", "bSlowDown",                           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edD54"),        &IniConfig.iD54SlowDownScale,                   ECInteger,  "Index", "iSlowDownScale",                      bShow, bEnable, bReadFromFile,  50,    false,  1,      100);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD55"),    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bNoShow, bDisable, bFixedValue, 0);
    }
    else if(REAL_TIME_CCD==false ||
            (COM2!=NULL && COM2->bCCDDummyRum))                                 //RTC 沒有開不能Disable IndexCheck
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD55"),    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bDisable, bFixedValue, 0);
    }
    else if(IniConfig.bM0109RTCOffCheckYieldPiggyBack)                          //Steven 20160727 : Fixed for SCK
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD55"),    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bDisable, bFixedValue, 1);
    }
    else                                                                        //ChungHung 20120710 add SCK Disable IndexCheck
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD55"),    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD56"),        &IniConfig.bD56YieldPiggyBackEnable,            ECBool,     "Index", "bYieldPiggyBackEnable",               bShow, bEnable, bReadFromFile, 0);  //kevin 20131009 強致 Enable Yield 裡面piggyback 功能

    if(IniConfig.bKoreaFunction || CUSTOMER_CODE==CC_ASE_KaohSiung)             //kevin 20141203 關site顯示 site號碼
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD57"),    &IniConfig.bD57SiteMapCloseDisplay,             ECBool,     "Index", "bSiteMapCloseDisplay",                bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD57"),    &IniConfig.bD57SiteMapCloseDisplay,             ECBool,     "Index", "bSiteMapCloseDisplay",                bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD58"),        &IniConfig.bD58UseArm1PickPlaceArm2Test,        ECBool,     "Index", "bUseArm1PickPlaceArm2Test",           bShow, bEnable, bReadFromFile, 0);  //kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試
    if(MachineTypeChoice==Type_HT9046_LS)                                       //Steven 20150910 : 32Site 雙Arm一起吸放
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD59"),    &IniConfig.bD59_32SitePnpTogether,              ECBool,     "Index", "bD59_32SitePnpTogether",              bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD59"),    &IniConfig.bD59_32SitePnpTogether,              ECBool,     "Index", "bD59_32SitePnpTogether",              bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bIndexArmVacOffErrNeedPiggyBack)                             //JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD61"),    &IniConfig.bD61IndexArmVacOffErrNeedPiggyBack,  ECBool,     "Index", "bD61IndexArmVacOffErrNeedPiggyBack",  bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD61"),    &IniConfig.bD61IndexArmVacOffErrNeedPiggyBack,  ECBool,     "Index", "bD61IndexArmVacOffErrNeedPiggyBack",  bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD62"),        &IniConfig.bD62PickUpErrorNeedPurge,            ECBool,     "Index", "bD62PickUpErrorNeedPurge",            bShow, bEnable, bReadFromFile, 0);  //Steveb 20161024 : 吸取異常需要吹氣一次

    if(CosFunction.bIndexPickErrSkipBlowAirTime)                                //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD62"),    &IniConfig.iD62IndexBlowAirTime,                ECInteger,  "Index", "iD62IndexBlowAirTime",                bShow, bEnable, bReadFromFile,  0,     false,  1,      10);
        EL<TLabel>("TfConfiguration", "labD62")->Visible=true;
    }
    else
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD62"),    &IniConfig.iD62IndexBlowAirTime,                ECInteger,  "Index", "iD62IndexBlowAirTime",                bNoShow, bDisable, bFixedValue, 0);
        EL<TLabel>("TfConfiguration", "labD62")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=true;
        if(CosFunction.bHiSiliconFunction==true)
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),&IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 1);   //kevin 20170515 (wei) Zphase home
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 1);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),&IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 0);  //kevin 20170515 (wei) Zphase home
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bDisable, bFixedValue, 0);
        }
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD64"),    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CosFunction.bHiSiliconFunction==true)                               //Ifor 20170809 (wei) add 海思版本強制開啟 Z Phase 功能
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 1);      //kevin 20170515 (wei) Zphase home
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable,  bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD64"),    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bDisable, bFixedValue, 0);      //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                    //kevin 20180727 move
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);     //kevin 20170515 (wei) Zphase home
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD64"),    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CUSTOMER_CODE==CC_GIGAS || CUSTOMER_CODE==CC_HONPREC_QC)            //Isaac 20201110 : Index Y find motor phase
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);     //Isaac 20201110 : Index Y find motor phase
    }
    else if(IniConfig.bSPILFunction)                                            //Isaac 20201110 : Index Y find motor phase
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);     //Isaac 20201110 : Index Y find motor phase
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD64"),    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bDisable, bFixedValue, 0);      //kevin 20171103 (wei) index pick up error only skip
    }
    else
    {
        EL<TPanel>("TfConfiguration", "pnlD63")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63"),    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 0);      //kevin 20170515 (wei) Zphase home
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD63_1"),  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD64"),    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }

    if(IniConfig.bSPILFunction==true && CosFunction.bHiSiliconFunction==true)   //JerryYang 20170921 (Steven) SPIL守志要求range要鎖定不得修改
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD63"),    &IniConfig.iD63IndexZHomeToZPhaseRange,         ECInteger,  "Index", "iD63IndexZHomeToZPhaseRange",             bShow, bDisable, bFixedValue,   10,    false,  1,      10);       //Isaac 20201124 : default:50->10,Range:1~1000->1~10
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD63"),    &IniConfig.iD63IndexZHomeToZPhaseRange,         ECInteger,  "Index", "iD63IndexZHomeToZPhaseRange",             bShow, bEnable, bReadFromFile,  10,    false,  1,      10);           //kevin 20170515 (wei) add Z phase range   //Isaac 20201124 : default:100->10,Range:1~1000->1~10

    if(CUSTOMER_CODE==CC_JCET)                                                  //Ifor 20171123 add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD65"),    &IniConfig.bD65EnableCheckSocketsensorFunction, ECBool,     "Index", "bD65EnableCheckSocketsensorFunction",     bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD65"),    &IniConfig.bD65EnableCheckSocketsensorFunction, ECBool,     "Index", "bD65EnableCheckSocketsensorFunction",     bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD66"),        &IniConfig.bD66Initialstartautoheight,          ECBool,     "Index", "bD66Initialstartautoheight",              bNoShow, bEnable, bFixedValue, 0);          //wei 20180227 Initial start auto height

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20190907 Arm 測區次數道量測 功能;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD67"),    &IniConfig.bD67LoadCellMeasure,                 ECBool,     "Index", "bD67LoadCellMeasure",                     bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD67"),    &IniConfig.bD67LoadCellMeasure,                 ECBool,     "Index", "bD67LoadCellMeasure",                     bNoShow, bEnable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20190925 : add KYEC 要求contact Hight 高度變更超過設定需報警
    {
        EL<TLabel>("TfConfiguration", "labD68_1")->Visible=true;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD68"),    &IniConfig.dD68DistanceRange,                   ECDouble,   "Index", "dD68DistanceRange",                       bShow, bEnable, bReadFromFile,  10,   false,  0.01,      10.0);        //Ifor 20190925 : add KYEC 要求contact Hight 高度變更超過設定需報警
    }
    else
    {
        EL<TLabel>("TfConfiguration", "labD68_1")->Visible=false;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edD68"),    &IniConfig.dD68DistanceRange,                   ECDouble,   "Index", "dD68DistanceRange",                       bNoShow, bDisable, bFixedValue,  10,   false,  0.01,      10.0);
    }

    if(CosFunction.bAfterAutoCleanNoIndexCheck)                                 //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgD69"),    &IniConfig.iD69IndexCheckModeForAutoClean,      ECInteger,  "Index", "iD69IndexCheckModeForAutoClean",          bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgD69"),    &IniConfig.iD69IndexCheckModeForAutoClean,      ECInteger,  "Index", "iD69IndexCheckModeForAutoClean",          bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexCycleTimeRecord)                                       //Sam 20200916 : Add Index Cycle Time Record
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD70"),    &IniConfig.bD70IndexCycleTimeRecord,            ECBool,     "Index", "bD70IndexCycleTimeRecord",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD70"),    &IniConfig.bD70IndexCycleTimeRecord,            ECBool,     "Index", "bD70IndexCycleTimeRecord",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexCheckCanTurnOff)                                       //Isaac 20211019 : 可選擇做index check的時機
    {
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgD71"),    &IniConfig.iD71IndexCheckOnOffMode,             ECInteger,  "Index", "iD71IndexCheckOnOffMode",                 bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgD71"),    &IniConfig.iD71IndexCheckOnOffMode,             ECInteger,  "Index", "iD71IndexCheckOnOffMode",                 bNoShow, bEnable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD72"),        &IniConfig.bD72NNModeMoveShtAfterContact,       ECBool,     "Index", "bD72NNModeMoveShtAfterContact",           bShow, bEnable, bReadFromFile, 0);                  //Steven 20220531 : index下壓之後才能移動shuttle
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD73"),    &IniConfig.bD73ContactModeFast,                 ECBool,     "Index", "bD73ContactModeFast",                     bShow, bDisable, bFixedValue, 1);                   //kevin 20220827 : 高雄 Contact mode fast
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD73"),    &IniConfig.bD73ContactModeFast,                 ECBool,     "Index", "bD73ContactModeFast",                     bShow, bEnable, bReadFromFile, 0);                  //kevin 20220817 : Conttact mode 加速

    if(REAL_TIME_CCD)                         //Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD74"),        &IniConfig.bD74RTCAutoTuning, ECBool,  "Index", "bD74RTCAutoTuning", bShow, bEnable, bReadFromFile, 0);      //Sam 20240711 : RTC Auto Tuning
        if(CUSTOMER_CODE==CC_SIGURD_HUKOU)     //Sam 20240709 : 矽格湖口家葳要求要強制鎖定開啟
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD75"),    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool,"Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD75"),    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool,"Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD74"),    &IniConfig.bD74RTCAutoTuning                       , ECBool, "Index", "bD74RTCAutoTuning",                        bNoShow, bEnable, bFixedValue, 0);   //Sam 20240711 : RTC Auto Tuning
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD75"),    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool, "Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUseIndexCheckHasICNeedPurge==true)                          //Ifor 20200622 add:Index Check Has IC Need Purge
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)//Ifor 20230619 add:KYEC要求強制開啟D68功能
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD78"),&IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD78"),&IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD78"),    &IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true)                      //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)          //Ifor 20230619 add:KYEC要求強制開啟D69功能
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD79"),&IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD79"),&IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD79"),    &IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bNoShow, bEnable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD80"),        &IniConfig.bD80AfterOutSHToRightSiteIndexCheck, ECBool,     "Index", "bD80EnableIndexPickShuttleErrNeedPurge", bShow, bEnable, bReadFromFile, 1);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD81"),        &IniConfig.bD81IndexCheckVacuumOnShuttle,       ECBool,     "Index", "bD81IndexCheckVacuumOnShuttle",          bShow, bEnable, bReadFromFile, 0);
    if(CosFunction.bSocketSensorCheckICAtArmDown)                               //Jimmychiu 20250826 : 每次下壓確認有IC在socket
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD82"),    &IniConfig.bD82CheckIndexHasIC,                 ECBool,     "Index", "bD82CheckIndexHasIC",                    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD82"),    &IniConfig.bD82CheckIndexHasIC,                 ECBool,     "Index", "bD82CheckIndexHasIC",                    bNoShow, bDisable, bFixedValue, 0);
    }
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                     //AI(ht9045-inarm-flow) 20260820 (RogerYang) : 三軸機無 Y1/Y2 獨立軸,ACSim 不可能成立,不顯示
    {                                                                           //刻意不卡 bUseTwoArm32Site——該旗標依 recipe 在 runtime 設定,表單建構當下不可靠;NN 判斷由 RunAutoCleanTwoArmSimultaneous() 擋
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD83"),    &IniConfig.bD83NNAutoCleanTwoArmSimultaneous,   ECBool,     "Index", "bD83NNAutoCleanTwoArmSimultaneous",      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbD83"),    &IniConfig.bD83NNAutoCleanTwoArmSimultaneous,   ECBool,     "Index", "bD83NNAutoCleanTwoArmSimultaneous",      bNoShow, bDisable, bFixedValue, 0);
    }

}

// golden cConfiguration.cpp:1837  TfConfiguration::InitConfigEdtList_ItemE()
static void IC_InitConfigEdtList_ItemE()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

//    cbTest->Add(edE31_Au1Y,   &IniConfig.dTrayYScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[0], true, 0.95, 1.05);

    cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE30"),       &IniConfig.bE30InArmUseDifferentScale,          ECBool,     "In/Out Arm", "bE30InArmUseDifferentScale",         bShow, bEnable, bReadFromFile, LastSet.bE30InArmUseDifferentScale?"1":"0");         //Steven 20090710

    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE30_1"),&IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,     "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bEnable, bReadFromFile, 0);      //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE30_2"),&IniConfig.bE30_2InArmUseDifferentScale_Cold,    ECBool,     "In/Out Arm", "bE30_2InArmUseDifferentScale_Cold",    bShow, bEnable, bReadFromFile, 0);
    }

    cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE31"),       &IniConfig.bE31OutArmUseDifferentScale,         ECBool,     "In/Out Arm", "bE31OutArmUseDifferentScale",        bShow, bEnable, bReadFromFile, LastSet.bE31OutArmUseDifferentScale?"1":"0");        //Steven 20090710
    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE31_1"),        &IniConfig.bE31_1OutArmUseDifferentScale_Hot,     ECBool,     "In/Out Arm", "bE31_1OutArmUseDifferentScale_Hot",    bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au1X"),   &IniConfig.dTrayXScale_Hot[eAuto1],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto1_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[0], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au1Y"),   &IniConfig.dTrayYScale_Hot[eAuto1],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto1_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[0], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au2X"),   &IniConfig.dTrayXScale_Hot[eAuto2],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto2_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[1], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au2Y"),   &IniConfig.dTrayYScale_Hot[eAuto2],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto2_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[1], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au3X"),   &IniConfig.dTrayXScale_Hot[eAuto3],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto3_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[2], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au3Y"),   &IniConfig.dTrayYScale_Hot[eAuto3],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto3_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[2], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au4X"),   &IniConfig.dTrayXScale_Hot[eAuto4],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto4_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au4Y"),   &IniConfig.dTrayYScale_Hot[eAuto4],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto4_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au5X"),   &IniConfig.dTrayXScale_Hot[eAuto5],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto5_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au5Y"),   &IniConfig.dTrayYScale_Hot[eAuto5],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto5_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au6X"),   &IniConfig.dTrayXScale_Hot[eAuto6],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto6_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Au6Y"),   &IniConfig.dTrayYScale_Hot[eAuto6],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto6_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi1X"),   &IniConfig.dTrayXScale_Hot[eFix1],       ECDouble,   "In/Out Arm", "dTrayXScaleFix1_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[3], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi1Y"),   &IniConfig.dTrayYScale_Hot[eFix1],       ECDouble,   "In/Out Arm", "dTrayYScaleFix1_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[3], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi2X"),   &IniConfig.dTrayXScale_Hot[eFix2],       ECDouble,   "In/Out Arm", "dTrayXScaleFix2_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[4], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi2Y"),   &IniConfig.dTrayYScale_Hot[eFix2],       ECDouble,   "In/Out Arm", "dTrayYScaleFix2_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[4], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi3X"),   &IniConfig.dTrayXScale_Hot[eFix3],       ECDouble,   "In/Out Arm", "dTrayXScaleFix3_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[5], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi3Y"),   &IniConfig.dTrayYScale_Hot[eFix3],       ECDouble,   "In/Out Arm", "dTrayYScaleFix3_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[5], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi4X"),   &IniConfig.dTrayXScale_Hot[eFix4],       ECDouble,   "In/Out Arm", "dTrayXScaleFix4_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi4Y"),   &IniConfig.dTrayYScale_Hot[eFix4],       ECDouble,   "In/Out Arm", "dTrayYScaleFix4_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi5X"),   &IniConfig.dTrayXScale_Hot[eFix5],       ECDouble,   "In/Out Arm", "dTrayXScaleFix5_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi5Y"),   &IniConfig.dTrayYScale_Hot[eFix5],       ECDouble,   "In/Out Arm", "dTrayYScaleFix5_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi6X"),   &IniConfig.dTrayXScale_Hot[eFix6],       ECDouble,   "In/Out Arm", "dTrayXScaleFix6_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_1_Fi6Y"),   &IniConfig.dTrayYScale_Hot[eFix6],       ECDouble,   "In/Out Arm", "dTrayYScaleFix6_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE31_2"),        &IniConfig.bE31_2OutArmUseDifferentScale_Cold,   ECBool,     "In/Out Arm", "bE31_2OutArmUseDifferentScale_Cold",   bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au1X"),   &IniConfig.dTrayXScale_Cold[eAuto1],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto1_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[0], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au1Y"),   &IniConfig.dTrayYScale_Cold[eAuto1],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto1_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[0], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au2X"),   &IniConfig.dTrayXScale_Cold[eAuto2],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto2_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[1], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au2Y"),   &IniConfig.dTrayYScale_Cold[eAuto2],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto2_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[1], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au3X"),   &IniConfig.dTrayXScale_Cold[eAuto3],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto3_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[2], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au3Y"),   &IniConfig.dTrayYScale_Cold[eAuto3],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto3_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[2], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au4X"),   &IniConfig.dTrayXScale_Cold[eAuto4],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto4_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au4Y"),   &IniConfig.dTrayYScale_Cold[eAuto4],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto4_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au5X"),   &IniConfig.dTrayXScale_Cold[eAuto5],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto5_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au5Y"),   &IniConfig.dTrayYScale_Cold[eAuto5],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto5_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au6X"),   &IniConfig.dTrayXScale_Cold[eAuto6],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto6_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Au6Y"),   &IniConfig.dTrayYScale_Cold[eAuto6],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto6_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi1X"),   &IniConfig.dTrayXScale_Cold[eFix1],      ECDouble,   "In/Out Arm", "dTrayXScaleFix1_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[3], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi1Y"),   &IniConfig.dTrayYScale_Cold[eFix1],      ECDouble,   "In/Out Arm", "dTrayYScaleFix1_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[3], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi2X"),   &IniConfig.dTrayXScale_Cold[eFix2],      ECDouble,   "In/Out Arm", "dTrayXScaleFix2_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[4], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi2Y"),   &IniConfig.dTrayYScale_Cold[eFix2],      ECDouble,   "In/Out Arm", "dTrayYScaleFix2_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[4], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi3X"),   &IniConfig.dTrayXScale_Cold[eFix3],      ECDouble,   "In/Out Arm", "dTrayXScaleFix3_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[5], false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi3Y"),   &IniConfig.dTrayYScale_Cold[eFix3],      ECDouble,   "In/Out Arm", "dTrayYScaleFix3_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[5], false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi4X"),   &IniConfig.dTrayXScale_Cold[eFix4],      ECDouble,   "In/Out Arm", "dTrayXScaleFix4_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi4Y"),   &IniConfig.dTrayYScale_Cold[eFix4],      ECDouble,   "In/Out Arm", "dTrayYScaleFix4_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi5X"),   &IniConfig.dTrayXScale_Cold[eFix5],      ECDouble,   "In/Out Arm", "dTrayXScaleFix5_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi5Y"),   &IniConfig.dTrayYScale_Cold[eFix5],      ECDouble,   "In/Out Arm", "dTrayYScaleFix5_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi6X"),   &IniConfig.dTrayXScale_Cold[eFix6],      ECDouble,   "In/Out Arm", "dTrayXScaleFix6_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_2_Fi6Y"),   &IniConfig.dTrayYScale_Cold[eFix6],      ECDouble,   "In/Out Arm", "dTrayYScaleFix6_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
    }
    cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE32"),       &IniConfig.bE32ShuttleUseDifferentScale,        ECBool,     "In/Out Arm", "bE32ShuttleUseDifferentScale",       bShow, bEnable, bReadFromFile, LastSet.bE32ShuttleUseDifferentScale?"1":"0");       //Steven 20090717

    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE32_1"),&IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,     "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE32_2"),&IniConfig.bE32_2ShuttleUseDifferentScale_Cold,  ECBool,     "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Cold",  bShow, bEnable, bReadFromFile, 0);      //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
    }

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au1X"),   &IniConfig.dTrayXScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[0], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au1Y"),   &IniConfig.dTrayYScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[0], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au2X"),   &IniConfig.dTrayXScale[eAuto2],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto2",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[1], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au2Y"),   &IniConfig.dTrayYScale[eAuto2],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto2",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[1], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au3X"),   &IniConfig.dTrayXScale[eAuto3],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto3",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[2], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au3Y"),   &IniConfig.dTrayYScale[eAuto3],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto3",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[2], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au4X"),   &IniConfig.dTrayXScale[eAuto4],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto4",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au4Y"),   &IniConfig.dTrayYScale[eAuto4],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto4",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au5X"),   &IniConfig.dTrayXScale[eAuto5],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto5",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au5Y"),   &IniConfig.dTrayYScale[eAuto5],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto5",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au6X"),   &IniConfig.dTrayXScale[eAuto6],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto6",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Au6Y"),   &IniConfig.dTrayYScale[eAuto6],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto6",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi1X"),   &IniConfig.dTrayXScale[eFix1],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix1",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[3], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi1Y"),   &IniConfig.dTrayYScale[eFix1],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix1",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[3], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi2X"),   &IniConfig.dTrayXScale[eFix2],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix2",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[4], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi2Y"),   &IniConfig.dTrayYScale[eFix2],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix2",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[4], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi3X"),   &IniConfig.dTrayXScale[eFix3],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix3",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[5], true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi3Y"),   &IniConfig.dTrayYScale[eFix3],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix3",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[5], true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi4X"),   &IniConfig.dTrayXScale[eFix4],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix4",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi4Y"),   &IniConfig.dTrayYScale[eFix4],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix4",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi5X"),   &IniConfig.dTrayXScale[eFix5],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix5",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi5Y"),   &IniConfig.dTrayYScale[eFix5],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix5",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi6X"),   &IniConfig.dTrayXScale[eFix6],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix6",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(EL<TEdit>("TfConfiguration", "edE31_Fi6Y"),   &IniConfig.dTrayYScale[eFix6],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix6",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    if(CosFunction.bHotModeUseDiffScale)            //JerryYang 20250120 : add
    {
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE30_1"),  &IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,   "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE32_1"),  &IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,   "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE30_1"),  &IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,   "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bDisable, bFixedValue, 0);
        cbLastSet->Add(EL<TCheckBox>("TfConfiguration", "cbE32_1"),  &IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,   "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE33"),        &IniConfig.bE33InOutArmZOffsetSameOne,          ECBool,     "In/Out Arm", "bE33InOutArmZOffsetSameOne",         bShow, bEnable, bReadFromFile, LastSet.bInOutArmZOffsetSameOne?"1":"0");            //jou 2010-06-21
    if(CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE34"),    &IniConfig.bE34InOutArmPitchZOffsetSameOne,     ECBool,     "In/Out Arm", "bE34InOutArmPitchZOffsetSameOne",    bShow, bDisable, bFixedValue, 0);         //Ifor 20160920 KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E34   功能(pick/release offset 要獨立)
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE34"),    &IniConfig.bE34InOutArmPitchZOffsetSameOne,     ECBool,     "In/Out Arm", "bE34InOutArmPitchZOffsetSameOne",    bShow, bEnable, bReadFromFile, LastSet.bInOutArmPitchZOffsetSameOne?"1":"0"); //jou 2010-12-20 Pitch ,& Z 縮減為一個

    if(IniConfig.bEnableInOutArmPlaceSkipSuckDetect)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE35"),    &IniConfig.bInOutArmPlaceSkipSuckDetect,        ECBool,     "In/Out Arm", "bInOutArmPlaceSkipSuckDetect",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE35"),    &IniConfig.bInOutArmPlaceSkipSuckDetect,        ECBool,     "In/Out Arm", "bInOutArmPlaceSkipSuckDetect",       bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(EL<TEdit>("TfConfiguration", "edE36"),        &IniConfig.iInArm60mmOffset,                    ECInteger,  "In/Out Arm", "iInArm60mmOffset",                   bShow, bEnable, bReadFromFile,  0,     false,  100,    -100);           //jou 2011-08-05 In Arm Y Pitch 60mm Offset
    elConfig->Add(EL<TEdit>("TfConfiguration", "edE37"),        &IniConfig.iOutArm60mmOffset,                   ECInteger,  "In/Out Arm", "iOutArm60mmOffset",                  bShow, bEnable, bReadFromFile,  3,     false,  100,    -100);           //jou 2011-08-05 Out Arm Y Pitch 60mm Offset

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE38"),    &IniConfig.bE38CheckHotPlateWhileInitialStart,  ECBool,         "In/Out Arm", "CheckHotPlateWhileInitialStart", bShow, bDisable, bFixedValue, 0);                       //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E38   功能(Initial Start Check HotPlate)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE39"),&IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed, ECBool, "In/Out Arm", "CheckHotPlateAfterCleanOutAndBeforeTrayFeed", bShow, bDisable, bFixedValue, 0);          //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E39   功能(Clean Out Tray Feed之前Check HotPlate)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE39_1"),  &IniConfig.bE39_1PutTheDevicesToErrorBin,       ECBool,         "In/Out Arm", "PutTheDevicesToErrorBin",        bShow, bDisable, bFixedValue, 0);                       //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E39_1 功能(HotPlate Check 放至 Err bin)
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE38"),    &IniConfig.bE38CheckHotPlateWhileInitialStart,  ECBool,         "In/Out Arm", "CheckHotPlateWhileInitialStart", bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120206 Hotplate check
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE39"),&IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed, ECBool, "In/Out Arm", "CheckHotPlateAfterCleanOutAndBeforeTrayFeed", bShow, bEnable, bReadFromFile, 0);         //ChungHung 20120206 Hotplate check
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE39_1"),  &IniConfig.bE39_1PutTheDevicesToErrorBin,       ECBool,         "In/Out Arm", "PutTheDevicesToErrorBin",        bShow, bEnable, bReadFromFile, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE40"),        &IniConfig.bE40ClearAllHotICThenPickLoadIC,     ECBool,     "In/Out Arm", "bClearAllHotICThenPickLoadIC",       bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120514 Clear HotIC then Pick Load IC
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE41"),        &IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent, ECBool,"In/Out Arm", "b35TrayPitchInOutSpeedSmall80Percent", bShow, bEnable, bReadFromFile, 0);                    //Steven 20120727 : Tray Pitch 大於35mm的話 In Out Arm需要小於80%
    if(CUSTOMER_CODE==CC_SCC)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE42"),    &IniConfig.bE42OutShuttleAlarmInArmServoOff,    ECBool,     "In/Out Arm", "bOutShuttleAlarmInArmServoOff",      bShow, bDisable, bFixedValue, 1);                       //ChungHung 20120723 Output Shuttle Alarm InArm Servo off
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE42"),    &IniConfig.bE42OutShuttleAlarmInArmServoOff,    ECBool,     "In/Out Arm", "bOutShuttleAlarmInArmServoOff",      bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120723 Output Shuttle Alarm InArm Servo off

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {
        EL<TCheckBox>("TfConfiguration", "cbE43")->Visible=false;
    }
    else if(IniConfig.bEnableAutoCleanFunction)                                 //Steven 20231121 : 有Auto Clean就可以選擇使用加熱盤
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE43"),    &IniConfig.bE43AutoCleanUseHotplate,            ECBool,     "In/Out Arm", "bAutoCleanUseHotplate",              bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20131120 AutoClean use Hotplate1cbE38,&IniConfig.bE38CheckHotPlateWhileInitialStart}       //ChungHung 20120206 Hotplate check
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE43"),    &IniConfig.bE43AutoCleanUseHotplate,            ECBool,     "In/Out Arm", "bAutoCleanUseHotplate",              bNoShow, bDisable, bFixedValue, 0);                     //ChungHung 20140408 add only for SCK
    }

    if(CUSTOMER_CODE==CC_SJ_Semiconductor)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE43_1"), &IniConfig.bE43_1_AutoCleanCountSaveFolder,     ECBool,     "In/Out Arm", "bE43_1_AutoCleanCountSaveFolder",    bShow, bEnable, bReadFromFile, 1);                      //Steven 20250527 : Save auto clean count to folder
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE43_1"), &IniConfig.bE43_1_AutoCleanCountSaveFolder,     ECBool,     "In/Out Arm", "bE43_1_AutoCleanCountSaveFolder",    bShow, bEnable, bReadFromFile, 0);                      //Steven 20250527 : Save auto clean count to folder

    if(CosFunction.bLoseDeviceOutShuttleServoOff==false)                        //ChungHung 20140522 add OutShuttle lose devices can servo off
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE44"), &IniConfig.bE44EnableLoseDeviceOutShuttleServoOff, ECBool,     "In/Out Arm", "bEnableLoseDeviceOutShuttleServoOff", bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE44"), &IniConfig.bE44EnableLoseDeviceOutShuttleServoOff, ECBool,     "In/Out Arm", "bEnableLoseDeviceOutShuttleServoOff", bShow, bEnable, bReadFromFile, 0);

    if(Tri_Temp_Machine==1)                                                     //Wenqi 20240516 Modify offset by workfile
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE45"),    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bEnable, bReadFromFile, 0);
    else if(CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE45"),    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bDisable, bFixedValue, 1);                       //wei 20151104 複製機台的offset避免offset讀不到資料  //Ifor 20191001 : KYEC [E45] false => true
    else if(CUSTOMER_CODE==CC_ASE_CL)   //JerryYang 20250120 : ASECL SONG要求強制關閉
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE45"),    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bDisable, bFixedValue, 0);                       //wei 20151104 複製機台的offset避免offset讀不到資料  //Ifor 20191001 : KYEC [E45] false => true
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE45"),    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : 所有工作檔共用同一個Offset檔案

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE46"),        &IniConfig.bE46_LoaderUse2Offset,               ECBool,     "In/Out Arm", "bE46_LoaderUse2Offset",              bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : Loader可以分別調前後排的Offset
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE47"),        &IniConfig.bE47_ShuttleUse4Offset,              ECBool,     "In/Out Arm", "bE47_ShuttleUse4Offset",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : Shuttle可以分別調前後排的Offset
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE48"),        &IniConfig.bE48_ShuttleUse4Offset_Autoclean,    ECBool,     "In/Out Arm", "bE48_ShuttleUse4Offset_Autoclean",   bShow, bEnable, bReadFromFile, 0);                      //20140923 wei :  Auto clean Shuttle可以分別調前後排的Offset
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE49"),        &IniConfig.bE49_LoaderOnlyRetryAndCleanOut,     ECBool,     "In/Out Arm", "bE49_LoaderOnlyRetryAndCleanOut",    bShow, bEnable, bReadFromFile, 0);                      //Steven 20141105 : Loader吸取異常只能Retry與CleanOut

    if(CosFunction.bOutArmPickUpErrBySetup==true)                               //JerryYang 20220923 : E50改成選項
    {
        EL<TRadioGroup>("TfConfiguration", "rgE50")->Visible=false;
    }
    else
    {
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgE50"),    &IniConfig.iE50_OutArmPickUpErrorOption,        ECInteger,  "InOutArm", "iE50_OutArmPickUpErrorOption",         bShow, bEnable, bReadFromFile, 0);                      //Steven 20181226 : 測試Time out可以按的按鈕
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE51"),        &IniConfig.bE51_EnableInArmZADC,                ECBool,     "In/Out Arm", "bE51_EnableInArmZADC",               bShow, bEnable, bReadFromFile, 0);                      //Steven 20141212 : 使用固定的ADC
    elConfig->Add(EL<TEdit>("TfConfiguration", "edE51"),        &IniConfig.iE51_EnableInArmZADC,                ECInteger,  "In/Out Arm", "iE51_EnableInArmZADC",               bShow, bEnable, bReadFromFile,  100,   false,  1,      100);            //Steven 20141212 : 使用固定的ADC
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cdE52"),        &IniConfig.bE52_EnableOutArmZADC,               ECBool,     "In/Out Arm", "bE52_EnableOutArmZADC",              bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edE52"),        &IniConfig.iE52_EnableOutArmZADC,               ECInteger,  "In/Out Arm", "iE52_EnableOutArmZADC",              bShow, bEnable, bReadFromFile,  100,   false,  1,      100);            //Steven 20141212 : 使用固定的ADC

    if(CosFunction.bLowYieldAutoSiteOff)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE53"),    &IniConfig.bE53LowYieldAutoClean,               ECBool,     "In/Out Arm", "bE53LowYieldAutoClean",              bNoShow, bDisable, bFixedValue, 0);                     //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE53"),    &IniConfig.bE53LowYieldAutoClean,               ECBool,     "In/Out Arm", "bE53LowYieldAutoClean",              bShow, bEnable, bReadFromFile, 0);                      //kevin 20160802 : CosFunction.bLowYieldAutoClean --> IniConfig.bE53LowYieldAutoClean

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE54"),        &IniConfig.bE54CheckCloseSiteNoIC,              ECBool,     "In/Out Arm", "bE54CheckCloseSiteNoIC",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20160922 : 因為OneCycle永遠先跑蝦頭1 檢查加熱盤錯誤功能與ByArmCloseSite衝突

    if(CosFunction.bUseFix3FullTray && FIX3_FULL_PLACE==Fix3K_Uninstall)        //Ifor 20161209 如果有安裝 Fix3 Full Place 強制關閉 Fix3 Full Tray 功能
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE55"),    &IniConfig.bE55UseFix3FullTray,                 ECBool,     "In/Out Arm", "bE55UseFix3FullTray",                bShow, bEnable, bReadFromFile, 0);                      //Ifor 20161121 : add Use Fix3 Full Tray Function
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE55"),    &IniConfig.bE55UseFix3FullTray,                 ECBool,     "In/Out Arm", "bE55UseFix3FullTray",                bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bAutoSkipNoDropError)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE56"),    &IniConfig.bE56LoaderRetryAtSamePosition,       ECBool,     "In/Out Arm", "bE56LoaderRetryAtSamePosition",      bShow, bDisable, bFixedValue, 1);                       //Steven 20170828 (wei) : Loader吸取異常時要在同一個位置作Retry
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE56"),    &IniConfig.bE56LoaderRetryAtSamePosition,       ECBool,     "In/Out Arm", "bE56LoaderRetryAtSamePosition",      bShow, bEnable, bReadFromFile, 0);                      //Steven 20170828 (wei) : Loader吸取異常時要在同一個位置作Retry
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE57"),        &IniConfig.bE57HPCanUseAnotherVacuumDelay,      ECBool,     "In/Out Arm", "bE57HPCanUseAnotherVacuumDelay",     bShow, bEnable, bReadFromFile, 0);                      //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE58"),        &IniConfig.bE57YPitchHome,                      ECBool,     "In/Out Arm", "bE57YPitchHome",                     bShow, bEnable, bReadFromFile, 0);                      //kevin 20180822 (Steven) : in out arm Y pitch 放完IC 歸 Y PITCH HOME
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE59"),        &IniConfig.bE59GroupOffsetFile,                 ECBool,     "In/Out Arm", "bE59GroupOffsetFile",                bShow, bEnable, bReadFromFile, 0);                      //Steven 20190327 : Offset file使用中括號做群組
    if(CosFunction.bAutoSkipNoDropError)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE60"),    &IniConfig.bE60PickLoaderDropAutoSkip,          ECBool,     "In/Out Arm", "bE60PickLoaderDropAutoSkip",         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE60"),    &IniConfig.bE60PickLoaderDropAutoSkip,          ECBool,     "In/Out Arm", "bE60PickLoaderDropAutoSkip",         bNoShow, bDisable, bFixedValue, 0);
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE61"),        &IniConfig.bE61InArmStandbyPosOnLoader,         ECBool,     "In/Out Arm", "bE61InArmStandbyPosOnLoader",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE62"),        &IniConfig.bE62TryPickLastRow,                  ECBool,     "In/Out Arm", "bE62TryPickLastRow",                 bShow, bEnable, bReadFromFile, 0);                      //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE63"),        &IniConfig.bE63RetryPickLoader,                 ECBool,     "In/Out Arm", "bE63RetryPickLoader",                bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20210324 add:KYEC 暫不使用等待後續討論結束
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE64"),    &IniConfig.bE64_50TrayPitchInOutSpeedSmall50Percent, ECBool, "In/Out Arm", "bE64_50TrayPitchInOutSpeedSmall50Percent", bShow, bDisable, bFixedValue, 1);     //Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50%
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE65"),   &IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr,   ECBool, "In/Out Arm", "bE65_ClearTrayDataWhenOutArmDestoryErr", bShow, bDisable, bFixedValue, 0);                  //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE64"),    &IniConfig.bE64_50TrayPitchInOutSpeedSmall50Percent,    ECBool, "In/Out Arm",   "bE64_50TrayPitchInOutSpeedSmall50Percent", bShow, bEnable, bReadFromFile, 0);     //Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50%
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE65"),   &IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr,      ECBool, "In/Out Arm",   "bE65_ClearTrayDataWhenOutArmDestoryErr", bShow, bEnable, bReadFromFile, 0);                  //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE66"),       &IniConfig.bE66_LogHotPlateAction,              ECBool,     "In/Out Arm", "bE66_LogHotPlateAction",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20211104 : 紀錄加熱盤的動作
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE67"),       &IniConfig.bE67_LoadPickerrorMoveWaitpos,       ECBool,     "In/Out Arm", "bE67_LoadPickerrorMoveWaitpos",      bShow, bEnable, bReadFromFile, 0);                      //kevin 20220723  : Load Pick up error Move wait pos
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE68"),        &IniConfig.bE68RecheckInOutArmICFallDown,       ECBool,     "In/Out Arm", "bE68RecheckInOutArmICFallDown",      bShow, bEnable, bReadFromFile, 0);                      //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    elConfig->Add(EL<TEdit>("TfConfiguration", "edE68"),        &IniConfig.iE68RecheckInOutArmICFallDown,       ECInteger,  "In/Out Arm", "iE68RecheckInOutArmICFallDown",      bShow, bEnable, bReadFromFile,  3,     false,  1,      100);              //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE69"),       &IniConfig.bE69_PickupErrorPlacement,           ECBool,     "In/Out Arm", "bE69_PickupErrorPlacement",          (USE_InPlacement==eartUninstall)?bNoShow:bShow, (USE_InPlacement==eartUninstall)?bDisable:bEnable, bReadFromFile, 0);  //JimmyChiu 20220908 add Pickup Error Placement
    if(CosFunction.bUseTrayThickAdjustZHeight==true)                            //Ifor 20221214 add: 使用Tray 厚度自動校正吸嘴高度
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)          //Ifor 20221220 add:KYEC要求強制開啟並反灰
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE70"),&IniConfig.bE70_UseTrayThickAdjustZHeight,     ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bShow, bDisable, bFixedValue, 1);       //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE70"),&IniConfig.bE70_UseTrayThickAdjustZHeight,     ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE70"),&IniConfig.bE70_UseTrayThickAdjustZHeight,         ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bNoShow, bDisable, bFixedValue, 0);     //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)               //Ifor 20221220 add:KYEC要求強制開啟並反灰
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE71"),   &IniConfig.bE71_10TrayPitchLockTrayAssign,      ECBool,     "Tray",  "bE71_10TrayPitchLockTrayAssign",          bShow, bDisable, bFixedValue, 1);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE71"),   &IniConfig.bE71_10TrayPitchLockTrayAssign,      ECBool,     "Tray",  "bE71_10TrayPitchLockTrayAssign",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE79"),        &IniConfig.bE79_InArmWaitShtLeftBeforePick,       ECBool,     "In/Out Arm", "bE79_InArmWaitShtLeftBeforePick",     bShow, bEnable, bReadFromFile, 0);  //JerryYang 20260707 : add E79 InArm wait shuttle Left before pick
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE72"),       &IniConfig.bE72_InarmPickICNeedWaitSH,          ECBool,     "In/Out Arm", "bE72_InarmPickICNeedWaitSH",         bShow, bEnable, bReadFromFile, 0);      //KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE73"),       &IniConfig.bE73_InOutZStepMotorLossCheck,       ECBool,     "Tray",  "bE73_InOutZStepMotorLossCheck",           bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    elConfig->Add(EL<TEdit>("TfConfiguration", "edE73"),        &IniConfig.iE73StepMotorCheckCnt,               ECInteger,  "In/Out Arm",           "iE73StepMotorCheckCnt",            bShow, bEnable, bReadFromFile,  5,   false,  3,      10);               //JerryYang 20231113 : add

    #ifdef SOFT_SIMULTE
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE74"),       &IniConfig.bE74_InspectArmPosition,             ECBool,     "Tray",  "bE74_InspectArmPosition",                 bShow, bEnable, bReadFromFile, 0);      //Jimmychiu 20240408 : debug for inarm position
    #else
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkE74"),       &IniConfig.bE74_InspectArmPosition,             ECBool,     "Tray",  "bE74_InspectArmPosition",                 bNoShow, bDisable, bFixedValue, 0);      //Jimmychiu 20240408 : debug for inarm position
    #endif
    if(CosFunction.bOneByOneWhenPickErrAtLoader==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbkE78"),       &IniConfig.bE78OneByOneWhenPickErrAtLoader,     ECBool,     "Tray",  "bE78OneByOneWhenPickErrAtLoader",         bShow, bEnable, bReadFromFile, 1);  //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
    }

    if(CosFunction.bFillTheTrayAfterOutArmPlace==true)                          //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE85_Enable"),     &IniConfig.bE85_FillTray_Enable,        ECBool,     "Tray",  "bE85_FillTray_Enable",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE85_Auto1"),      &IniConfig.bE85_FillTray_Auto1,         ECBool,     "Tray",  "bE85_FillTray_Auto1",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE85_Auto3"),      &IniConfig.bE85_FillTray_Auto3,         ECBool,     "Tray",  "bE85_FillTray_Auto3",                     bShow, bEnable, bReadFromFile, 0);
    }
    EL<TGroupBox>("TfConfiguration", "gbE85")->Visible=CosFunction.bFillTheTrayAfterOutArmPlace;
    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE86"),       &IniConfig.bE86_InArmPickErrOnLoaderOnlyCanSKIP, ECBool,   "In/Out Arm",  "bE72_InArmPickErrOnLoaderOnlyCanSKIP", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE87"),       &IniConfig.bE87PickupErrorAtLoaderNeedOpenDoor,  ECBool,   "In/Out Arm",  "bE73PickupErrorAtLoaderNeedOpenDoor",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE88"),       &IniConfig.bE88_InArmHeightFollow7000,           ECBool,   "In/Out Arm",  "bE75_InArmHeightFollow7000",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE89"),       &IniConfig.bE89_InArmHotPlatePitchUseScale,      ECBool,   "In/Out Arm",  "bE76_InArmHotPlatePitchUseScale",      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE86"),       &IniConfig.bE86_InArmPickErrOnLoaderOnlyCanSKIP, ECBool,   "In/Out Arm",  "bE72_InArmPickErrOnLoaderOnlyCanSKIP", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE87"),       &IniConfig.bE87PickupErrorAtLoaderNeedOpenDoor,  ECBool,   "In/Out Arm",   "bE73PickupErrorAtLoaderNeedOpenDoor", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE88"),       &IniConfig.bE88_InArmHeightFollow7000,           ECBool,   "In/Out Arm",  "bE75_InArmHeightFollow7000",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE89"),       &IniConfig.bE89_InArmHotPlatePitchUseScale,      ECBool,   "In/Out Arm",  "bE76_InArmHotPlatePitchUseScale",      bNoShow, bDisable, bFixedValue, 0);
    }
    //Steven 20260428 [E90]: Fix Tray Full retreat OutArm Y by -15000 for back-door tray pickup; ASE_SG forced ON
    if(CUSTOMER_CODE==CC_ASE_SG)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE90"),   &IniConfig.bE90_OutArmFixFullExtraY, ECBool, "In/Out Arm", "bE90_OutArmFixFullExtraY", bNoShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbE90"),   &IniConfig.bE90_OutArmFixFullExtraY, ECBool, "In/Out Arm", "bE90_OutArmFixFullExtraY", bShow,   bEnable,  bReadFromFile, 0);

}

// golden cConfiguration.cpp:2190  TfConfiguration::InitConfigEdtList_ItemF()
static void IC_InitConfigEdtList_ItemF()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF01"),        &IniConfig.bF01ShakeShuttleWhenJam,             ECBool,     "Shuttle", "bF01ShakeShuttleWhenJam",               bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF01"),        &IniConfig.iF01ShuttleShakeSpeed,               ECInteger,  "Shuttle", "iF01ShuttleShakeSpeed",                 bShow, bEnable, bReadFromFile, LastSet.iShuttleShakeSpeed,    false,  1,      100);//Steven 20120802 : Shuttle搖搖速度   //Steven 20160531 (wei) : 50 --> 1
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF03"),        &IniConfig.bF03OutputShuttleSkipICMiss,         ECBool,     "Shuttle", "bF03OutputShuttleSkipICMiss",           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF05"),        &IniConfig.iF05ShuttlePurgeCount,               ECInteger,  "Shuttle", "iF05ShuttlePurgeCount",                 bShow, bEnable, bReadFromFile, LastSet.iShuttlePurgeCount,    false,  1,      100);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF05"),        &IniConfig.bF05EnableShtPurgeFunction,          ECBool,     "Shuttle", "bF05EnableShtPurgeFunction",            bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF06"),        &IniConfig.bF06InitialICCheck,                  ECBool,     "Shuttle", "bF06InitialICCheck",                    bShow, bEnable, bReadFromFile, 0);

//    if(ENABLE_OUT_SHUTTLEY_LATCH==false)                                        //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
//        elConfig->Add(rgF07,    &IniConfig.iF07OutShuttleSensorMode,            ECInteger,  "Shuttle", "iF07OutShuttleSensorMode",              bShow, bDisable, bFixedValue, 0);
//    else
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgF07"),    &IniConfig.iF07OutShuttleSensorMode,            ECInteger,  "Shuttle", "iF07OutShuttleSensorMode",              bShow, bEnable, bReadFromFile, LastSet.iOutShuttleSensorMode);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF09"),        &IniConfig.bF09CheckICWhichFirstTimeLoad,       ECBool,     "Shuttle", "bF09CheckICWhichFirstTimeLoad",         bShow, bEnable, bReadFromFile, 0);                  //Steven 20090907 [F09]
    if(ENABLE_OUT_SHUTTLE_SENEOR==false)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF11"),    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bNoShow, bDisable, bFixedValue, 0);                 //JerryYang 20190401 out shuttle前後對照，硬體選項沒有勾就不要顯示
    else if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF11"),    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bShow, bDisable, bFixedValue, 1);                   //jou 2013-04-09 KYEC 邱凱偉要求強制打開
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF11"),    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bShow, bEnable, bReadFromFile, 0);                  //jou 2010-09-09 [F11] Out Shuttle 使用前後對照Sensor檢查殘留IC

    if(IniConfig.bHaveRotateShuttle && CUSTOMER_CODE!=CC_ASE_KaohSiung)
    {
        EL<TLabel>("TfConfiguration", "labF12")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF12"),    &IniConfig.bRotateShNeedCheck,                  ECBool,     "Rotate Shuttle", "bRotateShNeedCheck",             bShow, bEnable, bReadFromFile, 0);                  //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭
        elConfig->Add(EL<TEdit>("TfConfiguration", "EdF12"),    &IniConfig.dRoShCheckDelayTime,                 ECDouble,   "Rotate Shuttle", "dRoShCheckDelayTime",            bShow, bEnable, bReadFromFile,  0.01,  false, 1.0,     0.01);            //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭的延遲時間
    }
    else
    {
        EL<TLabel>("TfConfiguration", "labF12")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF12"),    &IniConfig.bRotateShNeedCheck,                  ECBool,     "Rotate Shuttle", "bRotateShNeedCheck",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "EdF12"),    &IniConfig.dRoShCheckDelayTime,                 ECDouble,   "Rotate Shuttle", "dRoShCheckDelayTime",            bNoShow, bDisable, bFixedValue, 0.01,  false, 1.0,     0.01);
    }

    if(IniConfig.bHaveRotateShuttle)                                            //Steven 20110819--------
    {
        EL<TGroupBox>("TfConfiguration", "gbF13_RotateShtSpeed")->Visible=(CUSTOMER_CODE!=CC_ASE_KaohSiung);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_ADC"),&IniConfig.iRotateADC,                          ECInteger,  "Configuration", "Rotate ADC",                      bShow, bEnable, bReadFromFile,  10,    false, 1,       100);            //Steven 20101018 : 轉轉蝦頭的加減速
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_Ini"),&IniConfig.iInitSpeed,                          ECInteger,  "Configuration", "RotateInitSpeed",                 bShow, bEnable, bReadFromFile,  50,    false, 1,       100);            //kevin 20110531 旋轉SHUTTLE 鎖最高速度
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_Jog"),&IniConfig.iPJogHighSpeed,                      ECInteger,  "Configuration", "RotateJogHighSpeed",              bShow, bEnable, bReadFromFile,  1000,  false, 1,       5000);           //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbF13_RotateShtSpeed")->Visible=false;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_ADC"),&IniConfig.iRotateADC,                          ECInteger,  "Configuration", "Rotate ADC",                      bNoShow, bDisable, bFixedValue, 10,    false, 1,       100);            //Steven 20101018 : 轉轉蝦頭的加減速
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_Ini"),&IniConfig.iInitSpeed,                          ECInteger,  "Configuration", "RotateInitSpeed",                 bNoShow, bDisable, bFixedValue, 50,    false, 1,       100);            //kevin 20110531 旋轉SHUTTLE 鎖最高速度
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF13_Jog"),&IniConfig.iPJogHighSpeed,                      ECInteger,  "Configuration", "RotateJogHighSpeed",              bNoShow, bDisable, bFixedValue, 1000,  false, 1,       5000);           //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF14"),        &IniConfig.bF14KnockShuttle,                    ECBool,     "Shuttle", "bKnockShuttle",                         bShow, bEnable, bReadFromFile,  0);                                     //Steven 20120801 : Shuttle敲敲
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF14_1"),      &IniConfig.bF14_1KnockShuttleFirst,             ECBool,     "Shuttle", "bKnockShuttleFirst",                    bShow, bEnable, bReadFromFile, 0);                                      //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF14"),        &IniConfig.dF14KnockShuttleInterval,            ECDouble,   "Shuttle", "dKnockShuttleInterval",                 bShow, bEnable, bReadFromFile,  0.1,   false, 1.0,     0.01);           //Steven 20120801 : Shuttle敲敲
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF14_No"),     &IniConfig.iF14KnockShuttleNo,                  ECInteger,  "Shuttle", "iKnockShuttleNo",                       bShow, bEnable, bReadFromFile,  3,     false, 3,       100);            //wei 20121206
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF14_1"),      &IniConfig.dF14KnockShuttleIntervalFirst,       ECDouble,   "Shuttle", "dF14KnockShuttleIntervalFirst",         bShow, bEnable, bReadFromFile,  0.1,   false, 1.0,     0.01);           //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF14_1_No"),   &IniConfig.iF14KnockShuttleNoFirst,             ECInteger,  "Shuttle", "iF14KnockShuttleNoFirst",               bShow, bEnable, bReadFromFile,  3,     false, 1,       100);            //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF15"),        &IniConfig.bF15OutShuttleLoseICNeedPWD,         ECBool,     "Shuttle", "OutShuttleLoseICNeedPWD",               bShow, bEnable, bReadFromFile, 0);                  //ChungHung 20120912 Amkor 需求Shuttle lose ic need password
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF16"),        &IniConfig.bF16CheckShuttleSensorBroken,        ECBool,     "Shuttle", "CheckShuttleSensorBroken",              bShow, bEnable, bReadFromFile, 0);                  //2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF17"),        &IniConfig.bF17Sht1First,                       ECBool,     "Shuttle", "bF17Sht1First",                         bShow, bEnable, bReadFromFile, 0);                  //Steven 20160926 : One cycle後要先跑蝦頭一

    if(CosFunction.bF18F22InshuttleDetectSaveByRecipe)                          //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
        elConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbF18"),    &IniConfig.bF18InshuttleDetect,        ECBool,     "Shuttle", "bInshuttleDetect",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic
    else if(IniConfig.bSPILFunction==true)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF18"),    &IniConfig.bF18InshuttleDetect,                 ECBool,     "Shuttle", "bInshuttleDetect",                      bNoShow, bDisable, bFixedValue, 0);                 //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF18"),    &IniConfig.bF18InshuttleDetect,                 ECBool,     "Shuttle", "bInshuttleDetect",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic

    if(CosFunction.bLockF19AlwaysOn)                                            //Steven 20150709 : Out shuttle lose IC要做Piggyback
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF19"),    &IniConfig.bF19OutShuttleLoseICNeedPiggyback,   ECBool,     "Shuttle", "bOutShuttleLoseICNeedPiggyback",        bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF19"),    &IniConfig.bF19OutShuttleLoseICNeedPiggyback,   ECBool,     "Shuttle", "bOutShuttleLoseICNeedPiggyback",        bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_PTI ||                                                 //Alick 20160815 add for 力成 Input Shuttle 檢測IC是否放到外圍，避免壓壞
       CUSTOMER_CODE==CC_SCC)                                                   //JerryYang 20210107 : Add SCC
    {
        EL<TLabel>("TfConfiguration", "cbF20Label2")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF20"),    &IniConfig.bF20InShuttleProminentDetect,        ECBool,     "Shuttle", "bF20InShuttleProminentDetect",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TLabel>("TfConfiguration", "cbF20Label2")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF20"),    &IniConfig.bF20InShuttleProminentDetect,        ECBool,     "Shuttle", "bF20InShuttleProminentDetect",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF21"),    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bNoShow, bDisable, bFixedValue, 0);                 //kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor  //kevin 20180525 Ifor 20180228 (Steven) KYEC暫時關閉避免16 Site 放料異常(蝦頭2 Kit1 吸嘴上下跳動)
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //Eastsun 20260515 F022: D6 Z home sensor fix
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF21"),    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bShow, bDisable, bFixedValue, 1);                 //kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor  //kevin 20180525 Ifor 20180228 (Steven) KYEC暫時關閉避免16 Site 放料異常(蝦頭2 Kit1 吸嘴上下跳動)
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF21"),    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bShow, bEnable, bReadFromFile, 0);

    if(CosFunction.bF18F22InshuttleDetectSaveByRecipe)                          //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
    {
        elConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbF22"), &IniConfig.bF22InShuttleDetectOutNoIC,    ECBool,     "Shuttle",  "bF22InshuttleDetectOutNoIC",           bShow, bEnable, bReadFromFile, 0);                  //kevin 20161108 shuttle 出來撿測有無IC殘留
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF22"),    &IniConfig.bF22InShuttleDetectOutNoIC,          ECBool,     "Shuttle",  "bF22InshuttleDetectOutNoIC",           bShow, bEnable, bReadFromFile, 0);                  //kevin 20161108 shuttle 出來撿測有無IC殘留
    }

    if(CosFunction.bKnockerSetBySetupFile==false)                               //JerryYang 20200504 修正開啟程式時振動功能失效問題
    {
        if(ShuttleVibration_INSTALL)                                            //JerryYang 20171006 (wei) Shuttle 震動馬達
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : SPIL強制啟用shuttle震動馬達功能
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF23"),    &IniConfig.bF23ShuttleVibration,        ECBool,     "Shuttle", "bF23ShuttleVibration",                  bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF23"),    &IniConfig.bF23ShuttleVibration,        ECBool,     "Shuttle", "bF23ShuttleVibration",                  bShow, bEnable, bReadFromFile, 0);
            }

            if(CUSTOMER_CODE==CC_TERAPOWER ||                                   //Sam 20210520 : TPW 修改震動時間 Range
               CUSTOMER_CODE==CC_AMKOR_Korea)
                elConfig->Add(EL<TEdit>("TfConfiguration", "edF23"),    &IniConfig.iF23ShuttleVibrationTime,    ECInteger,  "Shuttle", "iF23ShuttleVibrationTime",              bShow, bEnable, bReadFromFile,  20,   false, 1,      100);
            else
                elConfig->Add(EL<TEdit>("TfConfiguration", "edF23"),    &IniConfig.iF23ShuttleVibrationTime,    ECInteger,  "Shuttle", "iF23ShuttleVibrationTime",              bShow, bEnable, bReadFromFile,  20,   false, 20,     100);
            elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtF23_1"),     &IniConfig.iF23ShuttleVibrationCount,   ECInteger,  "Shuttle", "iF23ShuttleVibrationCount",             bShow, bEnable, bReadFromFile,  1,    false, 1,      3);    //JerryYang 20191113 shuttle振動馬達可設定次數
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF23"),    &IniConfig.bF23ShuttleVibration,            ECBool,     "Shuttle", "bF23ShuttleVibration",                  bNoShow, bDisable, bFixedValue, 0);
            EL<TEdit>("TfConfiguration", "edF23")->Visible=false;
            EL<TLabeledEdit>("TfConfiguration", "edtF23_1")->Visible=false;
        }
    }
    else
    {
        EL<TCheckBox>("TfConfiguration", "cbF23")->Visible=false;
        EL<TEdit>("TfConfiguration", "edF23")->Visible=false;
        EL<TLabeledEdit>("TfConfiguration", "edtF23_1")->Visible=false;
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF24"),        &IniConfig.bF24OutShuttleLoseIcOpenIndexDoor,   ECBool,     "Shuttle", "OutShuttleLoseIcOpenIndexDoor",         bShow, bEnable, bReadFromFile, 0);                              //kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF25"),        &IniConfig.bF25VibrateForOutShuttle,            ECBool,     "Shuttle", "bF25VibrateForOutShuttle",              bShow, bEnable, bReadFromFile, 0);                              //kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF25"),        &IniConfig.iF25VibrateTime,                     ECInteger,  "Shuttle", "iF25VibrateTime",                       bShow, bEnable, bReadFromFile,  20,    false, 20,      100);    //JerryYang 20171205 (Steven) shuttle震動馬達功能
    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgF26"),        &IniConfig.iF26OutShuttleJamSelectSkipOrRetry,  ECInteger,  "Shuttle", "iF25OutShuttleJamSelectSkipOrRetry",    bShow, bEnable, bReadFromFile,  0,    false, 2,      0);        //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry;

    if(CUSTOMER_CODE==CC_JCET)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkF27"),   &IniConfig.bF27OutShtLoseICNeedToPick,          ECBool,     "Shuttle", "bF27OutShtLoseICNeedToPick",            bShow, bEnable, bReadFromFile, 0);                  //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkF27"),   &IniConfig.bF27OutShtLoseICNeedToPick,          ECBool,     "Shuttle", "bF27OutShtLoseICNeedToPick",            bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF28"),        &IniConfig.bF28IndexCheckShuttlePos,            ECBool,     "Shuttle", "bF26IndexCheckShuttlePos",              bShow, bEnable, bReadFromFile, 0);                  //kevin 20220512 add  Index check shuttle pos for Sensor

    if(ShuttleVibration_INSTALL)                                                //Sam 20210602 : 每次都要強制震動
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF29"),    &IniConfig.bF29AlwaysVibrateOnShuttle,          ECBool,     "Shuttle", "bF26AlwaysVibrateOnShuttle",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF29"),    &IniConfig.bF29AlwaysVibrateOnShuttle,          ECBool,     "Shuttle", "bF26AlwaysVibrateOnShuttle",            bNoShow, bDisable, bFixedValue, 0);

    if(MachineTypeChoice==Type_HT9046_LS && SubMachineType==Type_HT9016C)       //Jimmychiu 20230628 Add TempPosShift for HT9016C
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF30"),    &IniConfig.bF30InShuttleSensorFollow16site,     ECBool,     "Shuttle", "bF26InShuttleSensorFollow16site",       bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF30"),    &IniConfig.bF30InShuttleSensorFollow16site,     ECBool,     "Shuttle", "bF26InShuttleSensorFollow16site",       bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20210426 : 2x4, 2x6 follow 2x8 shuttle sensor位置

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function ==>
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF31"),    &IniConfig.bF31_CheckShtMoveCnt,    ECBool,         "F_Shuttle", "bF31_CheckShtMoveCnt",                  bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_1"),  &IniConfig.iShtMoveCntSet[0],       ECInteger,      "F_Shuttle", "F31_1_iShuttleMotorMoveCountSet",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_2"),  &IniConfig.iShtMoveCntSet[1],       ECInteger,      "F_Shuttle", "F31_2_iShuttleMotorMoveCountSet",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_3"),  &IniConfig.iShtMoveCntNow[0],       ECInteger,      "F_Shuttle", "F31_3_iShuttleMotorMoveCountNow",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_4"),  &IniConfig.iShtMoveCntNow[1],       ECInteger,      "F_Shuttle", "F31_4_iShuttleMotorMoveCountNow",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_5"),  &IniConfig.iShtMoveCntHis[0],       ECInteger,      "F_Shuttle", "F31_5_iShuttleMotorMoveCountHistroy",   bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edF31_6"),  &IniConfig.iShtMoveCntHis[1],       ECInteger,      "F_Shuttle", "F31_6_iShuttleMotorMoveCountHistroy",   bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    //Ztex 2023.04.19 Add HT-1032 TriTemp Function <==

    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
      (SHUTTLE_SENSOR_TYPE==eSensorCanBus || SHUTTLE_SENSOR_TYPE==eSensorCanBus))   //KenHsieh 20230829 : add Check In Shuttle Sensor By pass
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF32"),    &IniConfig.bF32CheckInSHSenBypass,              ECBool,     "Shuttle",  "bF30CheckInSHSenBypass",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF32"),    &IniConfig.iF32InSHSenBypassValue,              ECInteger,  "Shuttle",  "iF30InSHSenBypassValue",               bShow, bEnable, bReadFromFile,  10,    false, 10,      4095);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF32"),    &IniConfig.bF32CheckInSHSenBypass,              ECBool,     "Shuttle",  "bF30CheckInSHSenBypass",               bNoShow, bDisable, bFixedValue, 0);
        EL<TEdit>("TfConfiguration", "edF32")->Visible=false;
    }

    if(IniConfig.bSPILFunction==true &&
       (BAR_CODE_INSTALL==ebctUseCCDMode ||                                     //JerryYang 20250220 : 2DID硬體順序檢查功能
       BAR_CODE_INSTALL==ebctInShtIntel ||
       BAR_CODE_INSTALL==ebctEtherNetCCD))
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF33_Check2DHardware"),    &IniConfig.bF33_Check2DHardware,ECBool,     "Shuttle",  "bF33_Check2DHardware",                 bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF33_Check2DHardware"),    &IniConfig.bF33_Check2DHardware,ECBool,     "Shuttle",  "bF33_Check2DHardware",                 bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bMaximFunction || CUSTOMER_CODE==CC_AnalogDevice_Phil)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF34"),        &IniConfig.bF34OutShtPickErrSetErrBin,           ECBool,     "Shuttle",  "bF34OutShtPickErrSetErrBin",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF35"),        &IniConfig.bF35OutShtLoseNeedSetErrBin,          ECBool,     "Shuttle",  "bF35OutShtLoseNeedSetErrBin",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF34"),        &IniConfig.bF34OutShtPickErrSetErrBin,           ECBool,     "Shuttle",  "bF34OutShtPickErrSetErrBin",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF35"),        &IniConfig.bF35OutShtLoseNeedSetErrBin,          ECBool,     "Shuttle",  "bF35OutShtLoseNeedSetErrBin",     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF36"),   &IniConfig.bF36OutShtLoseICResetSetAllToErr,          ECBool,     "Shuttle",     "bF36OutShtLoseICResetSetAllToErr",    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF36"),   &IniConfig.bF36OutShtLoseICResetSetAllToErr,          ECBool,      "Shuttle",     "bF36OutShtLoseICResetSetAllToErr",    bNoShow, bDisable, bFixedValue, 0);
    }
    if(BAR_CODE_INSTALL!=ebctUninstall &&
       BAR_CODE_INSTALL!=ebctOutShtAMD)                                         //AI(ht9045-config) 20260812 (RogerYang) : F37 NN mode 2DID pre-scan, 僅in-shuttle掃碼機型顯示
    {                                                                           //顯示條件刻意只卡 BAR_CODE_INSTALL,不卡 bUseTwoArm32Site——後者是依 recipe 測試模式在 runtime 設定的,Configuration 表單建構當下不可靠;NN mode 的判斷 runtime gate(IsNNPreScan2DIDCanStart)本來就會擋,UI 端不用重複
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF37"),        &IniConfig.bF37NNShuttlePreScan2DID,        ECBool,     "Shuttle",  "bF37NNShuttlePreScan2DID",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF37_Delay"),  &IniConfig.iF37NNPreScanStableDelayMS,      ECInteger,  "Shuttle",  "iF37NNPreScanStableDelayMS",           bShow, bEnable, bReadFromFile,  500,   false, 100,    5000);
        EL<TEdit>("TfConfiguration", "edF37_Delay")->Visible=true;                                              //AI(ht9045-config) 20260815 (RogerYang) : 與 else 分支對稱,labF37_Delay 未註冊進 elConfig 不會自動復原
        EL<TLabel>("TfConfiguration", "labF37_Delay")->Visible=true;
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbF37"),        &IniConfig.bF37NNShuttlePreScan2DID,        ECBool,     "Shuttle",  "bF37NNShuttlePreScan2DID",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edF37_Delay"),  &IniConfig.iF37NNPreScanStableDelayMS,      ECInteger,  "Shuttle",  "iF37NNPreScanStableDelayMS",           bNoShow, bDisable, bFixedValue, 500,   false, 100,    5000);
        EL<TEdit>("TfConfiguration", "edF37_Delay")->Visible=false;
        EL<TLabel>("TfConfiguration", "labF37_Delay")->Visible=false;
    }

}

// golden cConfiguration.cpp:2422  TfConfiguration::InitConfigEdtList_ItemG()
static void IC_InitConfigEdtList_ItemG()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG01"),        &IniConfig.bG01Show_TestRate,                   ECBool,     "Visible", "bG01Show_TestRate",                     bShow, bEnable, bReadFromFile, LastSet.bShow_TestRate?"1":"0"); //Steven 20090715
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG04"),        &IniConfig.bG04ShowFailAlarmCount,              ECBool,     "Visible", "bG04ShowFailAlarmCount",                bShow, bEnable, bReadFromFile, LastSet.bShowFailAlarmCount?"1":"0");            //jou 980717 add from ht9040 show alarm enabled
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG05"),        &IniConfig.bG05ShowSpeedMessage,                ECBool,     "Visible", "bG05ShowSpeedMessage",                  bShow, bEnable, bReadFromFile, LastSet.bShowSpeedMessage?"1":"0");              //Steven 20090720 show speed at uShowMessage
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG06"),        &IniConfig.bG06HomeinitialCheckZ1,              ECBool,     "Visible", "bHomeinitialCheckZ1",                   bShow, bEnable, bReadFromFile, 0);                  //kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG07"),        &IniConfig.bG07MultiColorForFailBin,            ECBool,     "Visible", "bSupportMultiColorForFailBin",          bShow, bEnable, bReadFromFile, 0);                  //Steven 20160310 : 改成有顏色的fail bin

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160413 顯示Are you sure
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG08"),    &IniConfig.bG08VisibleAreyousure,               ECBool,     "Visible", "bVisibleAreyousure",                    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG08"),    &IniConfig.bG08VisibleAreyousure,               ECBool,     "Visible", "bVisibleAreyousure",                    bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bG09NeedPasswordWhenEditSiteMap)                             //JerryYang 20160425 修改Site map需要密碼
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG09"),    &IniConfig.bG09NeedPasswordWhenEditSiteMap,     ECBool,     "Function", "bNeedPasswordWhenEditSiteMap",         bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20160425 修改Site map需要密碼
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG09"),    &IniConfig.bG09NeedPasswordWhenEditSiteMap,     ECBool,     "Function", "bNeedPasswordWhenEditSiteMap",         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG10"),        &IniConfig.bG10ShowImmediateUPH,                ECBool,     "Visible", "bG10ShowImmediateUPH",                  bShow, bDisable, bFixedValue, 1);                  //Steven 20160727 : Show immediate UPH
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG10"),        &IniConfig.bG10ShowImmediateUPH,                ECBool,     "Visible", "bG10ShowImmediateUPH",                  bShow, bEnable, bReadFromFile, 0);                  //Steven 20160727 : Show immediate UPH
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG11"),    &IniConfig.bG11ASEReport,                       ECBool,     "Visible", "bG11ASEReport",                         bShow, bEnable, bReadFromFile, 0);                  //kevin 20170306
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG11"),    &IniConfig.bG11ASEReport,                       ECBool,     "Visible", "bG11ASEReport",                         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_OSE || CUSTOMER_CODE==CC_ASE_KaohSiung)                //kevin 20180221 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG12"),    &IniConfig.bG12ContractModeManualMessage,       ECBool,     "Visible", "bG12ContractModeManualMessage",         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG12"),    &IniConfig.bG12ContractModeManualMessage,       ECBool,     "Visible", "bG12ContractModeManualMessage",         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG13"),        &IniConfig.bG13ShowTempOffsetOnContact,         ECBool,     "Visible", "bG13ShowTempOffsetOnContact",           bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Steven 20231018 : Fixed for G14
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG14"),    &IniConfig.bG14UseStartSoundAlarm,              ECBool,     "Visible", "bG14UseStartSoundAlarm",                bShow, bEnable, bReadFromFile, 0);                  //kevin 20201116  Start 發出聲音 不動 5sec
        elConfig->Add(EL<TEdit>("TfConfiguration", "edG14"),    &IniConfig.iG14StartWarrTime,                   ECInteger,  "Visible", "iStartWarrTime",                        bShow, bEnable, bReadFromFile,  3,     false, 1,       10);     //kevin 20201124 run Start alarm
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG14"),    &IniConfig.bG14UseStartSoundAlarm,              ECBool,     "Visible", "bG14UseStartSoundAlarm",                bNoShow, bDisable, bFixedValue, 0);                 //kevin 20201116  Start 發出聲音 不動 5sec
        elConfig->Add(EL<TEdit>("TfConfiguration", "edG14"),    &IniConfig.iG14StartWarrTime,                   ECInteger,  "Visible", "iStartWarrTime",                        bNoShow, bDisable, bFixedValue, 3);                 //kevin 20201124 run Start alarm
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG15"),        &IniConfig.bG15LoadInputCount,                  ECBool,     "Visible", "bG15LoadInputCount",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20211106 輸入顆數達成就 Clean out        //JimmyChiu 20220124 fix name
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkG16"),       &IniConfig.bG16BinDispNeedAlarm,                ECBool,     "Visible", "bG16BinDispNeedAlarm",                  bShow, bEnable, bReadFromFile, 0);                  //Steven 20211130 : JSCC要求Bin顯示器異常要alarm

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG17"),    &IniConfig.b17bUseLoadCCDTrayMap,               ECBool,     "Visible", "b17bUseLoadCCDTrayMap",                 bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610  Load 到定位通知eKeep 拍照，ekeep 自己按Start
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG17_1"),  &IniConfig.b17bUseLoadCCDTrayMapTrayend,        ECBool,     "Visible", "b17bUseLoadCCDTrayMapTrayend",          bShow, bEnable, bReadFromFile, 0);                  //kevin 20230530 trayend 拍照
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG17"),    &IniConfig.b17bUseLoadCCDTrayMap,               ECBool,     "Visible", "b17bUseLoadCCDTrayMap",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG17_1"),  &IniConfig.b17bUseLoadCCDTrayMapTrayend,        ECBool,     "Visible", "b17bUseLoadCCDTrayMapTrayend",          bNoShow, bDisable, bFixedValue, 0);
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG18_1"),      &IniConfig.b18bUseAutoTrayMap,                  ECBool,     "Visible", "b18bUseAutoTrayMap",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610 add Auto 123 Tray Map
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG18_2"),      &IniConfig.b18bDoubleUnloadTray,                ECBool,     "Visible", "b18bDoubleUnloadTray",                  bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610 add Unload Tray 2  倍 的設定 放空盤
    elConfig->Add(EL<TEdit>("TfConfiguration", "edG18Auto1Count"), &IniConfig.iUnloaderTrayCount[0],            ECInteger,  "Visible", "iAuto1Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edG18Auto2Count"), &IniConfig.iUnloaderTrayCount[1],            ECInteger,  "Visible", "iAuto2Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edG18Auto3Count"), &IniConfig.iUnloaderTrayCount[2],            ECInteger,  "Visible", "iAuto3Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG19"),    &IniConfig.b19InSHAutoCleanPA,                  ECBool,     "Visible", "b19InSHAutoCleanPA",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20230530 add Autoclean shuttle define new 參數
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG20"),    &IniConfig.b20FixfullWaitpos,                   ECBool,     "Visible", "b20FixfullWaitpos",                     bShow, bEnable, bReadFromFile, 0);                  //kevin 20230816 out arm move Wait
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG21"),    &IniConfig.b21colorfulltray,                    ECBool,     "Visible", "b21colorfulltray",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20230918 colortray no Alarm
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG19"),    &IniConfig.b19InSHAutoCleanPA,                  ECBool,     "Visible", "b19InSHAutoCleanPA",                    bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230530 add Autoclean shutt
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG20"),    &IniConfig.b20FixfullWaitpos,                   ECBool,     "Visible", "b20FixfullWaitpos",                     bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230816 out arm move Wait
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG21"),    &IniConfig.b21colorfulltray,                    ECBool,     "Visible", "b21colorfulltray",                      bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230918 colortray no Alarm
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG22"),        &IniConfig.bG22NoticeTakeoutTray,               ECBool,     "Visible", "bG22NoticeTakeoutTray",                 bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20231218 : G22提醒人員取tray功能
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG23"),        &IniConfig.bG23DiasbleFuncStatusView,           ECBool,     "Visible", "bG23DiasbleFuncStatusView",             bShow, bEnable, bReadFromFile, IniConfig.bVTESTFunction==true?"1":"0");    //RogerYang 20250728 Vtest設開關決定要不要顯示View
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbG24"),        &IniConfig.bG24DisableSECSGEMStatus,            ECBool,     "Visible", "bG24DisableSECSGEMStatus",              bShow, bEnable, bReadFromFile, 0);                  //RogerYang 20251222 : 丁曉東要求新增選項(G24)是否顯示

}

// golden cConfiguration.cpp:2513  TfConfiguration::InitConfigEdtList_ItemI()
static void IC_InitConfigEdtList_ItemI()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(CUSTOMER_CODE==CC_AMKOR_Philippines || CUSTOMER_CODE==CC_ASE_KaohSiung)  //JerryYang 20160228 for Amkor Philippines,Output Shuttle lose IC會自動回Home,所以強制開啟此功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI01"),    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bDisable, bFixedValue, 1);
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                         //Ifor 20160922 KYEC 喬智 要求 I01 強制關閉不可修改
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI01"),    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bDisable, bFixedValue, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI01"),    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bEnable, bReadFromFile, 0);
    }

    if(CosFunction.bLockI02ByFile)                                              //JerryYang 20260504 : for SPIL -- I02 Enable
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI02"),    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, IniConfig.bI02_Enable, bReadFromFile, 0);                   //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
    }
    else if(CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190618 : add ASEM 強制 關閉[I02]功能 回HOME INDEX ARM 全部丟ERROR Bin
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制 關閉[I02]功能 回HOME INDEX ARM 全部丟ERROR Bin
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI02"),    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bDisable, bFixedValue, 0);
    }
    else if(CUSTOMER_CODE==CC_ASE_CL ||                                         //JerryYang 20210129 : ASE-CL松諭要求強制開啟
           (CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true))
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI02"),    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI02"),    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bEnable, bReadFromFile, 0);                   //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
    }

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20210219 : 晶兆成需要開關 TempControl 模式
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI03"),    &IniConfig.bI03AmbientTempControl,              ECBool,     "Tester", "bI03AmbientTempControl",                 bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI03"),    &IniConfig.bI03AmbientTempControl,              ECBool,     "Tester", "bI03AmbientTempControl",                 bShow, bDisable, bFixedValue, 1);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI04"),        &IniConfig.bI04EnableChangeBinDuringTesting,    ECBool,     "Tester", "bI04EnableChangeBinDuringTesting",       bShow, bEnable, bReadFromFile, LastSet.bEnableChangeBinDuringTesting?"1":"0");

    if(CUSTOMER_CODE==CC_JCET)                                                  //JerryYang 20190709 JCET黃剛要求low yield不要強制one cycle
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI05"),    &IniConfig.bI05LowYieldForcedOneCycle,          ECBool,     "Tester", "bI05LowYieldForcedOneCycle",             bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI05"),    &IniConfig.bI05LowYieldForcedOneCycle,          ECBool,     "Tester", "bI05LowYieldForcedOneCycle",             bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI06"),        &IniConfig.bI06TurnOnI01AfterHome,              ECBool,     "Tester", "bI06TurnOnI01AfterHome",                 bShow, bEnable, bReadFromFile, 0);       //Sam 20220216 : Home 完成後強制開啟 I01 Function。

    if(CUSTOMER_CODE==CC_KYEC_CHEN)                                             //jou 2015-05-27 KYEC BU6 要求 I07 ResetGPIBAfterOneCycleCleanOut 強制關閉
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI07"),    &IniConfig.bI07ResetGPIBAfterOneCycleCleanOut,  ECBool,     "Tester", "bI07ResetGPIBAfterOneCycleCleanOut",     bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI07"),    &IniConfig.bI07ResetGPIBAfterOneCycleCleanOut,  ECBool,     "Tester", "bI07ResetGPIBAfterOneCycleCleanOut",     bShow, bEnable, bReadFromFile, 0);

    if(BAR_CODE_INSTALL!=ebctUninstall)                                         //Steven 20190412 : Initial Start時檢查有沒有開啟2DID
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI08"),    &IniConfig.bI08Check2DIDEnableWhenInitialStart, ECBool,     "Tester", "bI08Check2DIDEnableWhenInitialStart",    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI08"),    &IniConfig.bI08Check2DIDEnableWhenInitialStart, ECBool,     "Tester", "bI08Check2DIDEnableWhenInitialStart",    bShow, bDisable, bFixedValue, 0);

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI09"),    &IniConfig.bI09LowYieldOneCycleDontCleanShuttle,ECBool,     "Tester", "bI09LowYieldOneCycleDontCleanShuttle",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI09"),    &IniConfig.bI09LowYieldOneCycleDontCleanShuttle,ECBool,     "Tester", "bI09LowYieldOneCycleDontCleanShuttle",   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_SCC ||                                                 //jou 2012-06-27 We meet summary issue in SCC line, so please help fix two function (no one can change the setting)
       CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150903 鎖定I12
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI12"),    &IniConfig.bI12TesterTimerOutNotNeedReTest,     ECBool,     "Tester", "bI12TesterTimerOutNotNeedReTest",        bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI12"),    &IniConfig.bI12TesterTimerOutNotNeedReTest,     ECBool,     "Tester", "bI12TesterTimerOutNotNeedReTest",        bShow, bEnable, bReadFromFile, LastSet.bTesterTimerOutNotNeedReTest?"1":"0");

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI13"),        &IniConfig.bI13InitStartDelayHasFTandRT,        ECBool,     "Tester", "bI13InitStartDelayHasFTandRT",           bShow, bEnable, bReadFromFile, 0);      //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI16"),        &IniConfig.bI16TTLSaveInSetupFile,              ECBool,     "Tester", "bI16TTLSaveInSetupFile",                 bShow, bEnable, bReadFromFile, 0);      //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI18"),        &IniConfig.bI18CanReceiveEchoStop,              ECBool,     "Tester", "bCanReceiveEchoStop",                    bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130326 add for ASE_KR
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI19"),        &IniConfig.bI19AuToSitMapPauseWaitBin,          ECBool,     "Tester", "bAuToSitMapPauseWaitBin",                bShow, bEnable, bReadFromFile, 0);      //kevin 20150122 AutoSitMap 使用gpib模擬器讓機台暫停 設定 bin別
    elConfig->Add(EL<TComboBox>("TfConfiguration", "coI20"),        &IniConfig.iI20ErrorBinAlphabet,                ECInteger,  "Tester", "iI20ErrorBinAlphabet",                   bShow, bEnable, bReadFromFile, LastSet.iErrorBinAlphabet);   //Steven 20100413

    if(IniConfig.bUseAutoSiteMapping)                                           //Steven 20110421 : Auto Site Mapping
    {
        EL<TLabel>("TfConfiguration", "lbI21_6")->Visible=false;
        EL<TEdit>("TfConfiguration", "edI21_6")->Visible=false;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
           EL<TEdit>("TfConfiguration", "edI21")->Visible=false;
        }
        else if(IniConfig.bVTESTFunction==true)                                 //jou 20200707 : VTEST auto site mapping
        {
            EL<TLabel>("TfConfiguration", "lbI21_6")->Visible=true;
            EL<TEdit>("TfConfiguration", "edI21_6")->Visible=true;
        }

        EL<TGroupBox>("TfConfiguration", "gbI21")->Visible=true;
        EL<TEdit>("TfConfiguration", "edI21_9")->Visible=(CosFunction.bAutoSiteMappingUseFailBinSetting &&      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
                          !CosFunction.bAutoSiteMappingSetOpenBIN);             //Steven 20230213 : [I21-9]的OS Bin跟著工作檔

        if(CosFunction.bI21EnableASMByRecipe==false)                            //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
        {
            if(IniConfig.bVTESTFunction==true)                                  //RogerYang 20250624 修正無法By工作檔問題
                elConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbI21"),   &IniConfig.bI21EnableASM,           ECBool,     "Auto Site Mapping", "Enable Auto Site Mapping",    bShow, bEnable, bReadFromFile, 1);
            else
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21"),            &IniConfig.bI21EnableASM,           ECBool,     "Auto Site Mapping", "Enable Auto Site Mapping",    bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            EL<TCheckBox>("TfConfiguration", "cbI21")->Visible=false;
        }

        if(CUSTOMER_CODE==CC_LEADYO)                                            //KenHsieh 20251003 : LEADYO 需要設定fail次數報Alarm
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_1"),              &IniConfig.iI21FailRetryCount,      ECInteger, "Auto Site Mapping", "iI21FailRetryCount",           bShow,  bEnable,   bReadFromFile, 0, false, 0, 20);
        }
        else
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_1"),              &IniConfig.iI21FailRetryCount,      ECInteger, "Auto Site Mapping", "iI21FailRetryCount",           bNoShow, bDisable, bFixedValue,   0);
        }

        #ifdef ASE_KaohSiung                                                    //kevin 20150115
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI21"),                &IniConfig.fI21UseSameSoakTime,           ECDouble, "Auto Site Mapping", "Use Same Soak Time Sec",              bNoShow, bDisable, bFixedValue, 0.0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SkipSoakTime"),   &IniConfig.bI21SkipSoakTime,                ECBool, "Auto Site Mapping", "Skip Soak Time",                      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SameSoakTime"),   &IniConfig.bI21UseSameSoakTime,             ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_CheckOpen"),      &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_5"),              &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),              &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_7"),              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_8"),              &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_9"),          &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        #else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI21"),                &IniConfig.fI21UseSameSoakTime,           ECDouble, "Auto Site Mapping", "Use Same Soak Time Sec",              bShow, bEnable, bReadFromFile,  10.0, false, 0.0,     360.0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SkipSoakTime"),   &IniConfig.bI21SkipSoakTime,                ECBool, "Auto Site Mapping", "Skip Soak Time",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SameSoakTime"),   &IniConfig.bI21UseSameSoakTime,             ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bShow, bEnable, bReadFromFile, 0);
        if(CUSTOMER_CODE==CC_SCS)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_CheckOpen"),  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bShow, bDisable, bFixedValue, 1);        //Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open
        else if(CosFunction.bUSEJCETSiteMapMode)                                //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_CheckOpen"),  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_CheckOpen"),  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bShow, bEnable, bReadFromFile, 0);

        if(CosFunction.bUSEJCETSiteMapMode)                                     //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_5"),          &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);     //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_5"),          &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bShow, bEnable, bReadFromFile, 0);      //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray

        if(CUSTOMER_CODE==CC_ASE_M)                                             //Ifor 20180420 : add ASEM 要求強制開啟不可修改
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bDisable, bFixedValue, 1);        //Steven 20140729 : AutoSiteMapping, 邊生產邊做
        else if(IniConfig.bVTESTFunction==true)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bDisable, bFixedValue, 0);        //Steven 20140729 : AutoSiteMapping, 邊生產邊做
        else if(CosFunction.bUSEJCETSiteMapMode)                                //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bEnable, bReadFromFile, 0);

        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_7"),              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);      //kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_7"),              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bShow, bEnable, bReadFromFile, 0);      //kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2

        if(CosFunction.bAutoSiteMappingUseHotPlate==true)
        {
            if(CosFunction.bUSEJCETSiteMapMode ||
               CUSTOMER_CODE==CC_ASE_M)                                         //Ifor 20190624 : add ASEM 要求強制開啟不可修改
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_8"),      &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_8"),      &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bShow, bEnable, bReadFromFile, 0);      //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
            }
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_8"),          &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        }

        if(CosFunction.bAutoSiteMappingUseFailBinSetting==true)                 //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_9"),      &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bShow, bEnable, bReadFromFile, 0);      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_9"),      &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        }

        if(IniConfig.bVTESTFunction==true)                                      //jou 20200707 : VTEST auto site mapping
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_6"),          &IniConfig.iI21AutoSiteMappingErrCT,ECInteger,  "Auto Site Mapping", "iI21AutoSiteMappingErrCT",                bShow, bEnable, bReadFromFile,  3, false, 1,     5);
        else
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_6"),          &IniConfig.iI21AutoSiteMappingErrCT,ECInteger,  "Auto Site Mapping", "iI21AutoSiteMappingErrCT",                bNoShow, bDisable, bFixedValue,  0);

        if(CosFunction.bAutoSiteMappingUseFailBinSetting &&                     //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
           CosFunction.bAutoSiteMappingSetOpenBIN==false)                       //Steven 20230213 : [I21-9]的OS Bin跟著工作檔
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_9"),          &IniConfig.iI21UseFailBinSetting,   ECInteger,  "Auto Site Mapping", "iI21UseFailBinSetting",                   bShow, bEnable, bReadFromFile,  15, false, 2, 15);
        else
            elConfig->Add(EL<TEdit>("TfConfiguration", "edI21_9"),          &IniConfig.iI21UseFailBinSetting,   ECInteger,  "Auto Site Mapping", "iI21UseFailBinSetting",                   bNoShow, bDisable, bFixedValue, 0);

        //AI(ht9045-v899) 20260810: 甬矽要求RT複測模式可選擇不跑Site Mapping, 解鎖既有[I21-10]選項
        if(CUSTOMER_CODE==CC_JCET || CUSTOMER_CODE==CC_FOREHOPE_NINGBO)         //Richard 20230427 : RT mode不跑sitemapping
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_10"),    &IniConfig.bI21RTmodeDonotRunSiteMapping,ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",  bShow, bEnable, bReadFromFile, 0);      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_10"),    &IniConfig.bI21RTmodeDonotRunSiteMapping,ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",  bNoShow, bDisable, bFixedValue, 0);
        }
        #endif
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbI21")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21"),                &IniConfig.bI21EnableASM,                       ECBool, "Auto Site Mapping", "Enable Auto Site Mapping",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SkipSoakTime"),   &IniConfig.bI21SkipSoakTime,                    ECBool, "Auto Site Mapping", "Skip Soak Time",                      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_SameSoakTime"),   &IniConfig.bI21UseSameSoakTime,                 ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_CheckOpen"),      &IniConfig.bI21ASMNeedCheckEachSiteOpen,        ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_5"),              &IniConfig.bI21ASMRemoveLTrayManually,          ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_6"),              &IniConfig.bI21ASMRunTimeCHeck,                 ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_7"),              &IniConfig.bASMAutoSiteMapBinComBine,           ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_8"),              &IniConfig.bI21AutoSiteMappingUseHotplate,      ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_9"),              &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI21_10"),             &IniConfig.bI21RTmodeDonotRunSiteMapping,       ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",       bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bTestTimeOutOnlyShowSkip ||                                  //QQ 20230303
       CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       (CosFunction.bIndexAreaOnlyCanUseSkip &&
        CosFunction.bTestTimeOutShowSkipAndHome==false))                        //Steven 20141105 : Index內的所有異常都只能用Skip
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI22"),    &IniConfig.bI22TimeOutCanSkip,                  ECBool,     "Tester", "TestTimeOutCanSkip",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgI22"),    &IniConfig.iI22TestTimeOutOption,               ECInteger,  "Tester", "iI22TestTimeOutOption",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI22_1"),  &IniConfig.fI22HomeDelay,                       ECDouble,   "Tester", "fI22HomeDelay",                          bNoShow, bDisable, bFixedValue, 0.0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI22"),    &IniConfig.bI22TimeOutCanSkip,                  ECBool,     "Tester", "TestTimeOutCanSkip",                     bShow, bEnable, bReadFromFile, 0);      //Steven 20111220 : 測試TimeOut可以Skip
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgI22"),    &IniConfig.iI22TestTimeOutOption,               ECInteger,  "Tester", "iI22TestTimeOutOption",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI22_1"),  &IniConfig.fI22HomeDelay,                       ECDouble,   "Tester", "fI22HomeDelay",                          bShow, bEnable, bReadFromFile, 30.0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI23"),        &IniConfig.bI23HotTestWaitingMode,              ECBool,     "Tester", "HotTestWaitingMode",                     bShow, bEnable, bReadFromFile, 0);      //ChungHung 20111230 Hot Test Waiting Mode

    if(IniConfig.bEnableTestingNeedStopAllMotor)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI24"),    &IniConfig.bI24TestingNeedStopAllMotor,         ECBool,     "Tester", "bTestingNeedStopAllMotor",               bShow, bEnable, bReadFromFile, 0);      //jou 2013-09-25 Testing Need Stop All Motor
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI24"),    &IniConfig.bI24TestingNeedStopAllMotor,         ECBool,     "Tester", "bTestingNeedStopAllMotor",               bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bHiSiliconFunction==true &&                                  //Steven 20160301 : For ATC     //kevin 20130705 台積電通訊規格  0:HT    1:NS   //Steven 20160301 : 改為海思格式
       CUSTOMER_CODE!=CC_SCC)                                                   //Steven 20241029 : SCC的KL版本通訊不一樣
    {
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coI25"),    &IniConfig.iI25UseGPIBFormat,                   ECInteger,  "Temperature", "iUseGPIBFormat",                    bShow, bDisable, bFixedValue, 1);       //Ifor 20160407 海思專用版本 GPIB 格式強制設定為1
    }
    else
    {
        elConfig->Add(EL<TComboBox>("TfConfiguration", "coI25"),    &IniConfig.iI25UseGPIBFormat,                   ECInteger,  "Temperature", "iUseGPIBFormat",                    bShow, bEnable, bReadFromFile, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI26"),    &IniConfig.bI26TestCloseSiteHaveBin,            ECBool,     "Tester", "bTestCloseSiteHaveBin",                  bShow, bEnable, bReadFromFile, 0);      //kevin 20150202 測試時沒有 ic 出現bin資料或bin別沒設定需取出ic
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI26"),    &IniConfig.bI26TestCloseSiteHaveBin,            ECBool,     "Tester", "bTestCloseSiteHaveBin",                  bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bManualSortMode)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI27"),    &IniConfig.bI27_ManualSortMode,                 ECBool,     "Tester", "bI27_ManualSortMode",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20150915 : For TSMC 手動整盤功能
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI27"),    &IniConfig.bI27_ManualSortMode,                 ECBool,     "Tester", "bI27_ManualSortMode",                    bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI28"),        &IniConfig.bI28_OnOffSiteOnTheFly,              ECBool,     "Tester", "bI28_OnOffSiteOnTheFly",                 bShow, bEnable, bReadFromFile, 0);      //Steven 20150924 : 隨時開關Site功能

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI29"),        &IniConfig.bI29EnableYieldRecord,               ECBool,     "Function", "bEnableYieldRecord",                   bShow, bEnable, bReadFromFile, 0);      //Ifor 20151221 :新增 Yield Record Function
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkI29_1"),     &IniConfig.bI29_1SaveYieldBySocketByBin,        ECBool,     "Function", "bI29_1SaveYieldBySocketByBin",         bShow, bEnable, bReadFromFile, 0);      //Steven 20171108 (wei) : By Socket By Bin存檔
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI29_3"),      &IniConfig.bI29YieldRecordIntervalIC,           ECBool,     "Function", "bI29YieldRecordIntervalIC",            bShow, bEnable, bReadFromFile, 0);      //Sam 20231106 : 紀錄 Total yield
    elConfig->Add(EL<TEdit>("TfConfiguration", "edI29_3"),      &IniConfig.iI29YieldRecordIntervalIC,           ECInteger,  "Function", "iI29YieldRecordIntervalIC",            bShow, bEnable, bReadFromFile, 100,   false, 1,       3000);          //Sam 20231106 : 紀錄 Total yield
    elConfig->Add(EL<TEdit>("TfConfiguration", "edI29"),        &IniConfig.fI29YieldRecordInterval,             ECDouble,   "Function", "fYieldRecordInterval",                 bShow, bEnable, bReadFromFile, 600.0, false, 1.0,     3000.0);         //Ifor 20151221 :新增 Yield Record Interval Time
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI30"),        &IniConfig.bI30ContFailBin,                     ECBool,     "Tester", "I30_bContFailBin",                       bShow, bEnable, bReadFromFile, 0);      //kevin 20160407 使用Count bin  Count number 設定bin fail 數量到達就ALARM 清除計數

    if(CUSTOMER_CODE==CC_TSMC_TAINAN ||
       CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        EL<TGroupBox>("TfConfiguration", "grpI31")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_1"),  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_2"),  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bShow, bEnable, bReadFromFile, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_3"),  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bShow, bEnable, bReadFromFile, 0);
    }
    else if(CosFunction.bGPIBLotEnd==true)
    {
        EL<TGroupBox>("TfConfiguration", "grpI31")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_1"),  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_2"),  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bNoShow, bDisable, bFixedValue, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_3"),  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bNoShow, bDisable, bFixedValue, 0);     //wei 20170918 Reset Command
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpI31")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_1"),  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_2"),  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bNoShow, bDisable, bFixedValue, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI31_3"),  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bNoShow, bDisable, bFixedValue, 0);     //wei 20170918 Reset Command
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI32"),        &IniConfig.bI32CanCelErrorBin,                  ECBool,     "Tester", "bI32CanCelErrorBin",                     bNoShow, bDisable, bFixedValue, 0);     //kevin 20160802 取消error bin 要手動在 outshuttle 取出

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20220902
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI33"),    &IniConfig.bI33ErrorBinBox,                     ECBool,     "Tester", "bI33ErrorBinBox",                        bShow, bDisable, bFixedValue, 1);       //kevin 20160819 error bin 要放到 Bin Box
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI33"),    &IniConfig.bI33ErrorBinBox,                     ECBool,     "Tester", "bI33ErrorBinBox",                        bShow, bEnable, bReadFromFile, 0);      //kevin 20160819 error bin 要放到 Bin Box

    if(IniConfig.bSPILFunction==true)                                           //JerryYang20170329 (wei) 矽品中科,順信要求強制開啟I34功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI34"),    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bShow, bDisable, bFixedValue, 1);       //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    }
    else if(CosFunction.bAllSiteSameFailBinShowAlarm)                           //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI34"),    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI34"),    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI35"),        &IniConfig.bI35UseThirdSiteControlByEngineer,   ECBool,     "Tester", "bI35UseThirdSiteControlByEngineer",      bShow, bEnable, bReadFromFile, 0);      //Alick 20160926 add for 第三組工程師用開關SITE
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI36"),        &IniConfig.bI36TestTimeOut,                     ECBool,     "Tester", "bI36TestTimeOut",                        bShow, bEnable, bReadFromFile, 0);      //kevin 20161105 沒有收到測試資料需手動取出ic

    if(CosFunction.bHaveFIFOMode)                                               //Steven 20170302 (wei) : FIFO MODE
    {
        EL<TGroupBox>("TfConfiguration", "grpI37")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_1"),  &IniConfig.bI37_EnableFIFOMode,                 ECBool,     "Tester", "bI37_EnableFIFOMode",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_2"),  &IniConfig.bI37_EnableFIFOSiteOrder,            ECBool,     "Tester", "bI37_EnableFIFOSiteOrder",               bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_3"),  &IniConfig.bI37_LockLoaderDirection,            ECBool,     "Tester", "bI37_LockLoaderDirection",               bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "grpI37")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_1"),  &IniConfig.bI37_EnableFIFOMode,                 ECBool,     "Tester", "bI37_EnableFIFOMode",                    bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_2"),  &IniConfig.bI37_EnableFIFOSiteOrder,            ECBool,     "Tester", "bI37_EnableFIFOSiteOrder",               bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI37_3"),  &IniConfig.bI37_LockLoaderDirection,            ECBool,     "Tester", "bI37_LockLoaderDirection",               bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
    }

    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgI38"),        &IniConfig.iI38SETTEMPRespondSetTemp,           ECInteger,  "Tester", "bI38SETTEMPRespondSetTemp",              bShow, bEnable, bReadFromFile, 0);      //kevin 20180308 Settemp? 回傳需要 Settemp +25.0.
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI39"),        &IniConfig.bI39SpiroxTesterLotEnd,              ECBool,     "Tester", "bI39SpiroxTesterLotEnd",                 bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170515 (wei) JCET通知tester lot end command
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI40"),        &IniConfig.bI40_bStartProductOnLine,            ECBool,     "Tester", "bI40_bStartProductOnLine",               bShow, bEnable, bReadFromFile, 0);      //kevin 20180517 on line for operator

    //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    //==>
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41"),        &IniConfig.bI41EnableEmptySocketCheck,          ECBool,     "Tester", "bI41EnableEmptySocketCheck",         bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TComboBox>("TfConfiguration", "cbbI41"),       &IniConfig.iI41_BinOfESC,                       ECInteger,  "Tester", "iI44_BinOfESC",                      bShow, bEnable, bReadFromFile, 0);  //Steven 20220817 : Bin of ESC function
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_1"),      &IniConfig.bI41_1_StartOfLot,                   ECBool,     "Tester", "bI41_1_StartOfLot",                  bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_2"),      &IniConfig.bI41_2_OpenChamberDoor,              ECBool,     "Tester", "bI41_2_OpenChamberDoor",             bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_3"),      &IniConfig.bI41_3_AfterContactorTeminated,      ECBool,     "Tester", "bI41_3_AfterContactorTeminated",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_4"),      &IniConfig.bI41_4_AfterContactorJam,            ECBool,     "Tester", "bI41_4_AfterContactorJam",           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_5"),      &IniConfig.bI41_5_RegularExecutionCycle,        ECBool,     "Tester", "bI41_5_RegularExecutionCycle",       bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtI41_5"),     &IniConfig.iI41_5_RegularExecutionCycleCount,   ECInteger,  "Tester", "iI41_5_RegularExecutionCycleCount",  bShow, bEnable, bReadFromFile,  0,     false, 0,       100000);         //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI41_6"),      &IniConfig.bI41_6_Manual,                       ECBool,     "Tester", "bI41_6_Manual",                      bShow, bEnable, bReadFromFile, 0);
    //<==
    //Steven 20201022 : For RFMD Empty Socket Check Funstion.

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI42"),    &IniConfig.bI42_bEnableBarcodeFlowErr,          ECBool,  "Tester", "bI42_bEnableBarcodeFlowErr",         bShow, bEnable, bReadFromFile, 0);          //Ifor 20211116 add: Use Barcode Flow Err Check
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI42"),    &IniConfig.bI42_bEnableBarcodeFlowErr,          ECBool,  "Tester", "bI42_bEnableBarcodeFlowErr",         bShow, bEnable, bReadFromFile, 1);          //Ifor 20211116 add: Use Barcode Flow Err Check
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI43"),        &IniConfig.bI43ResetGPIBAfterTrayFeedFinish,    ECBool,     "Tester", "bI06ResetGPIBAfterTrayFeedFinish",   bShow, bEnable, bReadFromFile, 0);          //Sam 20211107 : 新增 Tray Feed Finish Reset GPIB

    if(CosFunction.bLowYieldAlarmIntervalTimeBySetting)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkI44"),   &IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting, ECBool, "Tester", "bI44_LowYieldAlarmIntervalTimeBySetting", bShow, bEnable, bReadFromFile, 0);     //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI44"),    &IniConfig.iI44_LowYieldAlarmIntervalTime,      ECInteger,  "Tester", "iI44_LowYieldAlarmIntervalTime",     bShow, bEnable, bReadFromFile,  60,     false,  1,      300);            //Steven 20141212 : 使用固定的ADC
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkI44"),   &IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting, ECBool, "Tester", "bI44_LowYieldAlarmIntervalTimeBySetting", bNoShow, bDisable, bFixedValue, 0);    //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
        elConfig->Add(EL<TEdit>("TfConfiguration", "edI44"),    &IniConfig.iI44_LowYieldAlarmIntervalTime,      ECInteger,  "Tester", "iI44_LowYieldAlarmIntervalTime",     bNoShow, bDisable, bFixedValue, 60,     false,  1,      300);            //Steven 20141212 : 使用固定的ADC
    }

    if(BAR_CODE_INSTALL!=ebctUninstall && CUSTOMER_CODE==CC_ASE_KaohSiung)      //KenHsieh 20230607 : ASEKH add 2D SORT
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI45"),    &IniConfig.bI45_Use2DIDSort,                    ECBool,     "Tester",  "bI45_Use2DIDSort",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI45"),    &IniConfig.bI45_Use2DIDSort,                    ECBool,     "Tester",  "bI45_Use2DIDSort",                  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20241018 : 錢寶龍說要強制開啟
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgI46"),    &IniConfig.iI46_ActionWhenGpibFlowErr,          ECInteger,  "Tester", "iI46_ActionWhenGpibFlowErr",         bShow, bDisable, bFixedValue,  1);
    else
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgI46"),    &IniConfig.iI46_ActionWhenGpibFlowErr,          ECInteger,  "Tester", "iI46_ActionWhenGpibFlowErr",         bShow, bEnable, bReadFromFile,  1);         //Steven 20231017 : GPIB flow error need alarm

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI49"),        &IniConfig.bI49_TesterTimeOutResetAllIC,        ECBool,     "Tester", "bI49_TesterTimeOutResetAllIC",       bShow, bEnable, bReadFromFile, 0);                          //Sam 20240215 : Tester time out show reset all ic
    elConfig->Add(EL<TEdit>("TfConfiguration", "edI49"),        &IniConfig.fI49_ChangeAboveSocket,              ECDouble,   "Tester", "fI49_ChangeAboveSocket",             bShow, bEnable, bReadFromFile,  0.0, false, 0.0,     50.0);

    if(IniConfig.bUseAutoSiteMapping)                                           //Jimmychiu 20230707 : Auto Site Mapping Trigger Function
    {
        EL<TGroupBox>("TfConfiguration", "gbI50")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_EnableASM_Trigger"),&IniConfig.bI50_EnableAutoSiteMappingTrigger, ECBool, "Tester", "bI50_EnableAutoSiteMappingTrigger",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_StartLot"),       &IniConfig.bI50_StartLot,                       ECBool, "Tester", "bI50_StartLot",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_InitialStart"),   &IniConfig.bI50_InitialStart,                   ECBool, "Tester", "bI50_InitialStart",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_OnyCycle"),       &IniConfig.bI50_OnyCycle,                       ECBool, "Tester", "bI50_OnyCycle",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_Pause"),          &IniConfig.bI50_Pause,                          ECBool, "Tester", "bI50_Pause",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_RT"),             &IniConfig.bI50_RT,                             ECBool, "Tester", "bI50_RT",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_TrayFeed"),       &IniConfig.bI50_TrayFeed,                       ECBool, "Tester", "bI50_TrayFeed",      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI50_EnableASM_Trigger"), &IniConfig.bI50_EnableAutoSiteMappingTrigger, ECBool, "Tester", "bI50_EnableAutoSiteMappingTrigger",      bNoShow, bDisable, bFixedValue, 0);
        EL<TGroupBox>("TfConfiguration", "gbI50")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if(USE_STM_Function==true)
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI51"),&IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",          bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI51"),&IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",         bNoShow, bDisable, bFixedValue, 0);      //kevin 20180517 on line for operator
        }
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI52"),    &IniConfig. bI52_bAQLSortMode,                  ECBool,  "Tester", "bI52_bAQLSortMode",                  bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI51"),    &IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",         bNoShow, bDisable, bFixedValue, 0);      //kevin 20180517 on line for operator
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI52"),    &IniConfig. bI52_bAQLSortMode,                  ECBool,  "Tester", "bI52_bAQLSortMode",                  bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true && CosFunction.bHiSiliconFunction==true)    //Ifor 20230210 add:新增 KLT 要求Onecycle Finish未超過設定時間不執行KL initial delay
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI53"),        &IniConfig. bI53_bKLTInitial,               ECBool,  "Tester", "bI53_bKLTInitial",      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI53"),        &IniConfig. bI53_bKLTInitial,               ECBool,  "Tester", "bI53_bKLTInitial",      bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bCheckTempDuringIndexArmTesting)                             //Jimmychiu 20240916 : Check the temperature during index arm testing
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI54_Enable"),         &IniConfig.bI54_Enable,             ECBool,  "Tester", "bI54_Enable",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI54_1"),              &IniConfig.bI54_1_AllICErr,         ECBool,  "Tester", "bI54_1_AllICErr",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI54_2"),              &IniConfig.bI54_2_AbnormalICErr,    ECBool,  "Tester", "bI54_2_AbnormalICErr", bShow, bEnable, bReadFromFile, 0);
    }
    else                                                                        //Steven 20241108 : 加上保護
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbI54_Enable"),         &IniConfig.bI54_Enable,             ECBool,  "Tester", "bI54_Enable",          bNoShow, bDisable, bFixedValue, 0);
        EL<TGroupBox>("TfConfiguration", "gbI54")->Visible=false;
    }

}

// golden cConfiguration.cpp:2966  TfConfiguration::InitConfigEdtList_ItemL()
static void IC_InitConfigEdtList_ItemL()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL03"),        &IniConfig.bL03SocketAirCoolingCT,              ECBool,     "Tempture", "bL03SocketAirCoolingCT",           bShow, bEnable, bReadFromFile, 0);      //jou 2016-04-28 Socket Air Cooling contact count trun on
    elConfig->Add(EL<TEdit>("TfConfiguration", "edL03"),        &IniConfig.iL03SocketAirCoolingCT,              ECInteger,  "Tempture", "iL03SocketAirCoolingCT",           bShow, bEnable, bReadFromFile,  2,     false, 0,       50);     //jou 2016-04-28 Socket Air Cooling contact count trun on

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL04"),    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  3);     //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CosFunction.bHiSiliconFunction==true)                               //Ifor 20160406 海思 ATC 相關參數寫死
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL04"),    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bDisable, bReadFromFile, 2,     false, 1,       10);
    else if(CUSTOMER_CODE==CC_ASE_CL)                                           //JerryYang 20200127 : 松諭要求把溫度range下限改為正負1度
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL04"),    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  2,     false, 1,       10);
    else if(IniConfig.bVTESTFunction==true)
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "edL04"),       &IniConfig.iL04TemptureRange,       ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  3,     false, 2,       10);
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL04"),    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  2,     false, 2,       10);

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL05"),    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bEnable, bReadFromFile,   3); //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CosFunction.bHiSiliconFunction==true)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL05"),    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bDisable, bReadFromFile, 2,     false, 2,       30);
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL05"),    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bEnable,  bReadFromFile, 2,     false, 2,       30);

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL06"),   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bEnable, bReadFromFile,   3); //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                   //Steven 20230410 : 南茂鍾永生要求修改
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL06"),   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bDisable, bReadFromFile, 2,     false, 0,       5);
    else if(CosFunction.bHiSiliconFunction==true)
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL06"),   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bDisable, bReadFromFile, 2,     false, 0,       10);
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL06"),   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bEnable,  bReadFromFile, 2,     false, 0,       10);         //jou 2013-04-11 Ambient Temperture Range

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL07"),    &IniConfig.bL07UseSingleTenmpertureLimit,       ECBool,     "Tempture", "bL07UseSingleTenmpertureLimit",    bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL07"),    &IniConfig.bL07UseSingleTenmpertureLimit,       ECBool,     "Tempture", "bL07UseSingleTenmpertureLimit",    bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TEdit>("TfConfiguration", "edL08_Over"),   &IniConfig.iSocketTemptureRangeOver,            ECInteger,  "Tempture", "iSocketTemptureRange",             bShow, bEnable,  bReadFromFile, 3,     false, 2,       30);         //Steven 20140308 : DUT溫度限制改成上下限分開
    elConfig->Add(EL<TEdit>("TfConfiguration", "edL08_Under"),  &IniConfig.iSocketTemptureRangeUnder,           ECInteger,  "Tempture", "iSocketTemptureRangeUnder",        bShow, bEnable,  bReadFromFile, 3,     false, 2,       30);         //Steven 20140308 : DUT溫度限制改成上下限分開

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL09"),    &IniConfig.bL09HotTempShuttleNoAddPos,          ECBool,     "Tempture", "bL23HotTempShuttlenoAddPos",       bShow, bEnable, bFixedValue, 1);        //kevin 20200812 : ASEKH 高溫shuttle不補位置
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL09"),    &IniConfig.bL09HotTempShuttleNoAddPos,          ECBool,     "Tempture", "bL23HotTempShuttlenoAddPos",       bShow, bEnable, bReadFromFile, 0);      //kevin 20200718 : ASEKH 高溫shuttle不補位置

    if(CUSTOMER_CODE==CC_ASE_KaohSiung && CosFunction.bHiSiliconFunction==true)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL10"),    &IniConfig.bL10IndexTestlogTemp,                ECBool,     "Tempture", "bL10IndexTestlogTemp",             bShow, bEnable, bFixedValue, 1);        //kevin 20200227 add 下壓溫度記 錄
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL10"),    &IniConfig.bL10IndexTestlogTemp,                ECBool,     "Tempture", "bL10IndexTestlogTemp",             bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgL10"),  &IniConfig.iL10TempRecordInterval,                    ECInteger,  "Index", "iL10TempRecordInterval",                bShow, bEnable,  bReadFromFile, LastSet.iTempRecordInterval);   //Steven 20100225 //Steven 20231017 : fixed

    if(CosFunction.bHiSiliconFunction)                                          //Ifor 20160406 海思 ATC 相關參數寫死
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_1"),  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bDisable, bFixedValue,   3);                                 //Ifor 20160406 L11-1 強制設定3度
    }
    else
    {
        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_1"),  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bEnable,  bReadFromFile, 3,     false, 1,       3);          //jou 20240426 : VTEST 要求 ATC 溫度限制設定1~3度
        else
            elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_1"),  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bEnable,  bReadFromFile, 10,     false, 1,       30);        //Eliot 2015_0105 2~50度
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bHiSiliconFunction==true)      //Ifor 20160310  取消水冷機自動關閉功能
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_2"),  &IniConfig.bL11_2ATCChillerProtectedFunction,   ECBool,     "Index", "bATCChillerProtectedFunction",        bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_2"),  &IniConfig.bL11_2ATCChillerProtectedFunction,   ECBool,     "Index", "bATCChillerProtectedFunction",        bShow, bEnable, bReadFromFile, 0);      // 2012.05.07  Joye  Chiller

    elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_1_2"),    &IniConfig.dATCTemperatureCheckTime,            ECDouble,   "Index", "dATCTemperatureCheckTime",            bShow, bEnable,  bReadFromFile, 10.0,   false, 0.01,    180.0);     //Steven 20121222 : ATC Temperature Over Check Time
    elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_2"),      &IniConfig.iATCChillerCheckTime,                ECInteger,  "Index", "iATCChillerCheckTime",                bShow, bEnable,  bReadFromFile, 5,      false, 5,       300);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_4"),      &IniConfig.iATCTemperatureOverLimit,            ECInteger,  "Index", "iATCTemperatureOverLimit",            bShow, bEnable,  bReadFromFile, 130,    false, 2,       130);       //Steven 20140916 : [L11-4] ATC的最高上限溫度

    if(ATC_SYSTEM==eATCUninstall || ATC_SYSTEM==eNonChamber)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_1"),  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_6"),  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_7"),  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_8"),  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bNoShow, bDisable, bFixedValue, 0);
    }
    else if(CosFunction.bHiSiliconFunction==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_1"),  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_6"),  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_6_Outside"),    &IniConfig.iATCTemperatureOutside,    ECInteger,  "Tempture", "iATCTemperatureOutside",           bShow, bDisable, bFixedValue, 3);   //Ifor 20160406 海思版本ATC功能[L11-6]強制設定 3度
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_6_Continuous"), &IniConfig.iATCTemperatureContinuous, ECInteger,  "Tempture", "iATCTemperatureContinuous",        bShow, bDisable, bFixedValue, 3);   //Ifor 20160406 海思版本ATC功能[L11-6]強制設定 3秒
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_7"),  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bShow, bDisable, bFixedValue, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_7_MaxSurge"),   &IniConfig.iATCMaxSurgeAlarm,         ECInteger,  "Tempture", "iATCMaxSurgeAlarm",                bShow, bDisable, bFixedValue, 7);   //Ifor 20160406 海思版本ATC功能[L11-7]強制設定 7度
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_8"),  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_1"),  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bDisable, bFixedValue, 1);      // 2012.05.11  Joye  ATC Temperature Over Check
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_1"),  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bEnable, bReadFromFile, 0);      // 2012.05.11  Joye  ATC Temperature Over Check
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_6"),  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-6] ATC突波於設定範圍內且持續發生超過設定時間後發出Alarm
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_6_Outside"),    &IniConfig.iATCTemperatureOutside,    ECInteger,  "Tempture", "iATCTemperatureOutside",           bShow, bEnable,  bReadFromFile, 2,      false, 1,       30);    //Ifor 20150911 : [L11-6] ATC 突波超出設定溫度
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_6_Continuous"), &IniConfig.iATCTemperatureContinuous, ECInteger,  "Tempture", "iATCTemperatureContinuous",        bShow, bEnable,  bReadFromFile, 3,      false, 1,       30);    //Ifor 20150911 : [L11-6] ATC 允許突波持續發生時間(s)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_7"),  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-7] ATC突波超出最大設定值發出Alarmed
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL11_7_MaxSurge"),   &IniConfig.iATCMaxSurgeAlarm,         ECInteger,  "Tempture", "iATCMaxSurgeAlarm",                bShow, bEnable,  bReadFromFile, 7,      false, 1,       30);    //Ifor 20150911 : [L11-7] ATC 最高突波上限
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL11_8"),  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-8] ATC第1組sensor跟第2組sensor溫度差值超過設定值發出警報
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL12"),        &IniConfig.bL12TempErrNoCloseHeater,            ECBool,     "Tempture", "TempErrNoCloseHeater",                     bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL13"),     &IniConfig.bL13HotPlateAndShuttleUseOneTempOffset, ECBool,     "Tempture", "bHotPlateAndShuttleUseOneTempOffset",      bShow, bEnable, bReadFromFile, 0);        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    if(CosFunction.bChamberModeEvenBlowNeedWaitTime)                            //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL15"), &IniConfig.bL15EnableChamberModeEvenBlowNeedWaitTime, ECBool,  "Tempture", "bEnableChamberModeEvenBlowNeedWaitTime",   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL15"), &IniConfig.bL15EnableChamberModeEvenBlowNeedWaitTime, ECBool,  "Tempture", "bEnableChamberModeEvenBlowNeedWaitTime",   bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL17"),        &IniConfig.bL17HeadHeaterOnWhenCloseSite,       ECBool,     "Tempture", "bHeadHeaterOnWhenCloseSite",       bShow, bEnable, bReadFromFile, 0);      //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160107 No FullSite Add Offset
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL18"),    &IniConfig.bL18NofullsiteaddTemperatureoffset,  ECBool,     "Tempture", "NofullsiteaddTemperatureoffset",   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL18"),    &IniConfig.bL18NofullsiteaddTemperatureoffset,  ECBool,     "Tempture", "NofullsiteaddTemperatureoffset",   bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_JCET)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL19"),    &IniConfig.bL19OpenHeatDoorgiveupchamberhot,    ECBool,     "Tempture", "bL19OpenHeatDoorgiveupchamberhot", bShow, bEnable, bReadFromFile, 0);      //kevin 20170520 (wei) 開chamber門只有不加熱chamber
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL19"),    &IniConfig.bL19OpenHeatDoorgiveupchamberhot,    ECBool,     "Tempture", "bL19OpenHeatDoorgiveupchamberhot", bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL20"),        &IniConfig.bL20AbientGuardBand,                 ECBool,     "Tempture", "bL20AbientGuardBand",              bShow, bEnable, bReadFromFile, 0);      //kevin 20180115 (Steven) add Amient Guard Band
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL21"),        &IniConfig.bL21PowerOffTemperature,             ECBool,     "Tempture", "bL21PowerOffTemperature",          bShow, bEnable, bReadFromFile, 0);      //kevin 20181112 (Steven) : power off 開 Chamber door 斷所有加熱電

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL22"),    &IniConfig.bL22Enable3SigmaTempMonitor,         ECBool,     "Tempture", "bL22Enable3SigmaTempMonitor",      bShow, bEnable, bReadFromFile, 0);      //kevin 20200521 : ASEKH 3SIGMA  溫度統計
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL22"),    &IniConfig.bL22Enable3SigmaTempMonitor,         ECBool,     "Tempture", "bL22Enable3SigmaTempMonitor",      bNoShow, bDisable, bFixedValue, 0);      //kevin 20200521 : ASEKH 3SIGMA  溫度統計

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20210122 : ASE-CL新增待溫功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL24"),    &IniConfig.bL24HeaterStableTime,                ECBool,     "Tempture", "bL24HeaterStableTime",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL24"),    &IniConfig.iL24HeaterStableTime,                ECInteger,  "Index", "iL24HeaterStableTime",                bShow, bEnable, bReadFromFile,  30, false, 10, 1800);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL24"),    &IniConfig.bL24HeaterStableTime,                ECBool,     "Tempture", "bL24HeaterStableTime",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL24"),    &IniConfig.iL24HeaterStableTime,                ECInteger,  "Index", "iL24HeaterStableTime",                bNoShow, bDisable, bFixedValue,  0);
    }

    if(CosFunction.bHiSiliconFunction==true ||
       TestIF_File.i2DIDFormat==eAMD ||
       CosFunction.bATCUseHandlerRecipeName==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL25"),    &IniConfig.bL25_1ATCFileNameWithTemp,           ECBool,     "Tempture", "bL25_1ATCFileNameWithTemp",        bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL25"),    &IniConfig.bL25_1ATCFileNameWithTemp,           ECBool,     "Tempture", "bL25_1ATCFileNameWithTemp",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                               //Sam 20231214 : Temp offset use ready temp range
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL28"),    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bShow, bDisable, bFixedValue, 1);
    else if(CUSTOMER_CODE==CC_ASE_SG)                                           //Ifor 20251105 add: ASE SG 客戶要求[L28]強制關閉不顯示
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL28"),    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL28"),    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL29"),        &IniConfig.bL29AmbientNotShowTemp,              ECBool,     "Tempture", "bL29AmbientNotShowTemp",           bShow, bEnable, bReadFromFile, 0);      //Sam 20221101 : 常溫模式不顯示溫度

    if(CosFunction.bUse1CableLayoutKitByConfig)                                 //Sam 20210715 : Use 1CableLayoutKit By Config
    {
        EL<TGroupBox>("TfConfiguration", "gbL30")->Visible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL30"),    &IniConfig.bL30Use1CableLayoutKitByConfig,      ECBool,     "Tempture", "bL30Use1CableLayoutKitByConfig",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL30_1"),  &IniConfig.bL30Use1CableLayoutKit,              ECBool,     "Tempture", "bL30Use1CableLayoutKit",           bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbL30")->Visible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL30"),    &IniConfig.bL30Use1CableLayoutKitByConfig,      ECBool,     "Tempture", "bL30Use1CableLayoutKitByConfig",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL30_1"),  &IniConfig.bL30Use1CableLayoutKit,              ECBool,     "Tempture", "bL30Use1CableLayoutKit",           bNoShow, bDisable, bFixedValue, 0);
    }

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    if(Tri_Temp_Machine==1)
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL09_1"),   &IniConfig.iL09_1HightTemp_Sht_Shift,         ECInteger,  "Tempture", "iL09_1HightTemp_Sht_Shift",            bShow, bEnable, bReadFromFile,  15,  false, -100, 100.0);//Ztex 2024.02.21 Add Change Temp When Shuttle Auto Shift Pos
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL09_2"),   &IniConfig.iL09_2LowTemp_Sht_Shift,           ECInteger,  "Tempture", "iL09_2LowTemp_Sht_Shift",              bShow, bEnable, bReadFromFile,  -40, false, -100, 100.0);//Ztex 2024.02.21 Add Change Temp When Shuttle Auto Shift Pos

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL32_1"),    &IniConfig.bL32_1ManuDefrost,                 ECBool,     "Tempture", "bL32_1ManualDefrost",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL32_2"),    &IniConfig.bL32_2AutoDefrostFunction,         ECBool,     "Tempture", "bL32_2AutoDefrostFunction",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_4"),    &IniConfig.iL32_4SetDefrostTemp,              ECInteger,  "Tempture", "iL32_4SetDefrostTempature",            bShow, bEnable, bReadFromFile,  10, false, 80, 100.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_5"),    &IniConfig.iL32_5SetDefrostTime,              ECInteger,  "Tempture", "iL32_5SetDefrostTime",                 bShow, bEnable, bReadFromFile,  10, false, 10, 60.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_6"),    &IniConfig.iL32_6LowTempRunAlarmDegree,       ECInteger,  "Tempture", "iL32_6LowTempRunAlarmDegree",          bShow, bEnable, bReadFromFile,  30, false, 10, 40);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_7_1"),  &IniConfig.iL32_7LowTempRunAlarmHour,         ECInteger,  "Tempture", "iL32_7LowTempRunAlarmHour",            bShow, bEnable, bReadFromFile,  2, false, 2, 72);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_7_2"),  &IniConfig.iL32_7LowTempRunAlarmMin,          ECInteger,  "Tempture", "iL32_7LowTempRunAlarmMin",             bShow, bEnable, bReadFromFile,  30, false, 0, 59);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL32_8"),    &IniConfig.iL32_8SetAirStreamTemp,            ECInteger,  "Tempture", "iL32_8SetAirStreamTemp",               bShow, bEnable, bReadFromFile,  40, false, 40, 180);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL33_1"),    &IniConfig.bL33_1CheckDoorOpenForTriTemp,     ECBool,     "Tempture", "bL33_1CheckDoorOpenForTriTemp",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL33_2"),    &IniConfig.bL33_2DoorOpenRunDefrost,          ECBool,     "Tempture", "bL33_2DoorOpenRunDefrost",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL33_3"),    &IniConfig.iL33_3BDoorOpenTimeForLowTemp,     ECInteger,  "Tempture", "iL33_3BDoorOpenTimeForLowTemp",        bShow, bEnable, bReadFromFile,  10, false, 10, 200.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL33_4"),    &IniConfig.iL33_4SDoorOpenTimeForLowTemp,     ECInteger,  "Tempture", "iL33_4SDoorOpenTimeForLowTemp",        bShow, bEnable, bReadFromFile,  10, false, 10, 200.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL33_5"),    &IniConfig.iL33_5DoorOpenTempForLowTemp,      ECInteger,  "Tempture", "iL33_5DoorOpenTempForLowTemp",         bShow, bEnable, bReadFromFile,  10, false, -55, 40.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL33_6"),    &IniConfig.iL33_6DoorOpenTempForHotTemp,      ECInteger,  "Tempture", "iL33_6DoorOpenTempForHotTemp",         bShow, bEnable, bReadFromFile,  10, false, 40, 175.0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL34_1"),    &IniConfig.bL34_1DelayOfFixDoorOpen,          ECBool,     "Tempture", "bL34_1DelayOfFixDoorOpen",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL34_2"),    &IniConfig.iL34_2DelaySecOfFixDoorOpen,       ECInteger,  "Tempture", "iL34_2DelaySecOfFixDoorOpen",          bShow, bEnable, bReadFromFile,  10, false, 10, 100.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL34_3"),    &IniConfig.dL34_3DewPointOfFixDoorOpen,       ECDouble,   "Tempture", "dL34_3DewPointOfFixDoorOpen",          bShow, bEnable,  bReadFromFile, 10.0,   false, 0.00,    -60.0);     //Steven 20121222 : ATC Temperature Over Check Time
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL34_4"),    &IniConfig.iL34_4OpenAuto3TrackGateSec,       ECInteger,  "Tempture", "iL34_4OpenAuto3TrackGateSec",          bShow, bEnable, bReadFromFile,  30, false, 10, 1800);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL34_5"),    &IniConfig.bL34_5FixTrayDoorCynAutoOpen,      ECBool,     "Tempture", "bL34_5FixTrayDoorCynAutoOpen",         bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL35_1"),    &IniConfig.bL35_1OverSetTempOpenFan,          ECBool,     "Tempture", "bL35_1OverSetTempOpenFan",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edL35_2"),    &IniConfig.iL35_2OpenFanTemp,                 ECInteger,  "Tempture", "iL35_2OpenFanTemp",                    bShow, bEnable, bReadFromFile,  30, false, 40, 175);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL36_1"),   &IniConfig.iL36_1Tri_Temp_Rang_ATC,           ECInteger,  "Tempture", "iL36_1Tri_Temp_Rang_ATC",                            bShow, bEnable, bReadFromFile, 3, false, 1,    10.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL36_2"),   &IniConfig.iL36_2Tri_Temp_Rang_Heater,        ECInteger,  "Tempture", "iL36_2Tri_Temp_Rang_Heater",                         bShow, bEnable, bReadFromFile, 3, false, 1,    20.0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL37"),      &IniConfig.bL37UnDockTurnOffAir,              ECBool,     "Tempture", "bL37UnDockTurnOffAir",               bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL39_1"),    &IniConfig.bL39_1AutoRunWhenTempOk,           ECBool,     "Tempture", "bL39_1AutoRunWhenTempOk",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL39_2"),    &IniConfig.bL39_2WaitTempstabilize,           ECBool,     "Tempture", "bL39_2WaitTempstabilize",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL39_2"),   &IniConfig.iL39_2WaitTempstabilize,           ECInteger,  "Tempture", "iL39_2WaitTempstabilize",                bShow, bEnable, bReadFromFile,  3, false, 1, 180);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edL40"),      &IniConfig.iL40ImmediateTempExceedsAlarm,     ECInteger,  "Tempture", "iL40ImmediateTempExceedsAlarm",     bShow, bEnable, bReadFromFile,  7, false, 1, 10);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edL41"),      &IniConfig.iL41TemperatureAlarmSecond,        ECInteger,  "Tempture", "iL41TemperatureAlarmSecond",     bShow, bEnable, bReadFromFile,  3, false, 1, 10);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL42"),      &IniConfig.bL42_UseOutShuttleDesoakTime,      ECBool,     "Tempture", "bL42_UseOutShuttleDesoakTime",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL42"),     &IniConfig.iL42_UseOutShuttleDesoakTime,      ECInteger,  "Tempture", "iL42_UseOutShuttleDesoakTime",         bShow, bEnable, bReadFromFile,  30, false, 0, 120);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL44"),      &IniConfig.bL44_SetColdAirSwitchTemp,         ECBool,     "Tempture", "bL44_SetColdAirSwitchTemp",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL44"),     &IniConfig.iL44_SetColdAirSwitchTemp,         ECInteger,  "Tempture", "iL44_SetColdAirSwitchTemp",         bShow, bEnable, bReadFromFile,  30, false, 0, 25);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL45"),      &IniConfig.bL45_SetDewPointOffset,            ECBool,     "Tempture", "bL45_SetDewPointOffset",         bShow, bEnable, bReadFromFile, 0);                //Ztex 2025.04.01 Add Set Dew Point Offset
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL45"),     &IniConfig.iL45_SetDewPointOffset,            ECInteger,  "Tempture", "iL45_SetDewPointOffset",         bShow, bEnable, bReadFromFile,  30, false, -25, 25);//Ztex 2025.04.01 Add Set Dew Point Offset
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL46"),      &IniConfig.bL46_AStreamErrorCompressOnecycle, ECBool,     "Tempture", "bL43_AStreamErrorCompressOnecycle",    bShow, bEnable, bReadFromFile, 0);//Ztex 2024.10.01 Add AStream Error Compress Onecycle
        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL42"),     &IniConfig.bL42_UseOutShuttleDesoakTime,       ECBool,     "Tempture", "bL42_UseOutShuttleDesoakTime",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtL42"),    &IniConfig.iL42_UseOutShuttleDesoakTime,       ECInteger,  "Tempture", "iL42_UseOutShuttleDesoakTime",         bNoShow, bDisable, bFixedValue, 0);
    }

    if(ATC_SYSTEM==eNewATCSystem)                                               //KenHsieh 20240216 : add ATC Power Follow Function
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL43"),    &IniConfig.bL43EnableATCPowerFollow,            ECBool,     "Tempture", "bL43EnableATCPowerFollow",             bShow,   bEnable,  bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL43"),    &IniConfig.bL43EnableATCPowerFollow,            ECBool,     "Tempture", "bL43EnableATCPowerFollow",             bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbL49"),        &IniConfig.bL49UseBySetupOffsetRange,           ECBool,     "Tempture", "bL49UseBySetupOffsetRange",       bShow, bEnable, bReadFromFile, 0);                       //Ifor 20231219 add Use By Setup Offset Range
    elConfig->Add(EL<TEdit>("TfConfiguration", "edL49"),       &IniConfig.iL49UseBySetupOffsetRange,           ECInteger,  "Tempture", "iL49UseBySetupOffsetRange",       bShow, bEnable, bReadFromFile,  30, false, 1, 60);     //Ifor 20231219 add Use By Setup Offset Range

}

// golden cConfiguration.cpp:3216  TfConfiguration::InitConfigEdtList_ItemM()
static void IC_InitConfigEdtList_ItemM()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;
    EL<TTabSheet>("TfConfiguration", "tsM00")->TabVisible=(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL);                                  //ChungHung 20131009 add for SCK
    if(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01"),    &IniConfig.bM01EnableMonitorFunction,           ECBool,     "Monitor", "bEnableMonitorFunction",                        bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_01"), &IniConfig.bM0101ContactModeUseDifferentSpeed,  ECBool,     "Monitor", "bMonitorContactModeMustSelectDifferentSpeed",   bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_02"), &IniConfig.bM0102SiteYieldDifferentMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldDifferentMustOn",              bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_03"), &IniConfig.bM0103ContinueFailBySocketMustOn,    ECBool,     "Monitor", "bMonitorSiteYieldContinueFailBySocketMustOn",   bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_04"), &IniConfig.bM0104ContinueFailByHeadMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldContinueFailByHeadMustOn",     bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_05"), &IniConfig.bM0105InOutArmDeviceCheckMustOn,     ECBool,     "Monitor", "bMonitorInOutArmDeviceCheckMustOn",             bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_06"), &IniConfig.bM0106IndexDeviceCheckDestoryMustOn, ECBool,     "Monitor", "bMonitorIndexArmDeviceCheckDestoryMustOn",      bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_07"), &IniConfig.bM0107AutoSpeedMustOn,               ECBool,     "Monitor", "bMonitorAutoSpeedMustOn",                       bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_08"), &IniConfig.bM0108EveryFirstDeviceMustOn,        ECBool,     "Monitor", "bMonitorEveryFirstDeviceMustEnabled",           bShow, bEnable, bReadFromFile, 0);  //ChungHung 20141210 add for SCK want to add Monitor every first device have delay time
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_09"), &IniConfig.bM0109RTCOffCheckYieldPiggyBack,     ECBool,     "Monitor", "bM0109RTCOffCheckYieldPiggyBack",               bShow, bEnable, bReadFromFile, 0);  //ChungHung 20150613 add for SCK want to check and show message
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_10"), &IniConfig.bM1010Disable_I12,                   ECBool,     "Monitor", "bM1010Disable_I12",                             bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_12"), &IniConfig.bM1012EnableAutoClean,               ECBool,     "Monitor", "bM1012EnableAutoClean",                         bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_13"), &IniConfig.bM1013Enable2DID,                    ECBool,     "Monitor", "bM1013Enable2DID",                              bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_14"), &IniConfig.bM1014EnableATC,                     ECBool,     "Monitor", "bM1014EnableATC",                               bShow, bEnable, bReadFromFile, 0);  //Steven 20191129 : For SCK
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01"),    &IniConfig.bM01EnableMonitorFunction,           ECBool,     "Monitor", "bEnableMonitorFunction",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_01"), &IniConfig.bM0101ContactModeUseDifferentSpeed,  ECBool,     "Monitor", "bMonitorContactModeMustSelectDifferentSpeed",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_02"), &IniConfig.bM0102SiteYieldDifferentMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldDifferentMustOn",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_03"), &IniConfig.bM0103ContinueFailBySocketMustOn,    ECBool,     "Monitor", "bMonitorSiteYieldContinueFailBySocketMustOn",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_04"), &IniConfig.bM0104ContinueFailByHeadMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldContinueFailByHeadMustOn",     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_05"), &IniConfig.bM0105InOutArmDeviceCheckMustOn,     ECBool,     "Monitor", "bMonitorInOutArmDeviceCheckMustOn",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_06"), &IniConfig.bM0106IndexDeviceCheckDestoryMustOn, ECBool,     "Monitor", "bMonitorIndexArmDeviceCheckDestoryMustOn",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_07"), &IniConfig.bM0107AutoSpeedMustOn,               ECBool,     "Monitor", "bMonitorAutoSpeedMustOn",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_08"), &IniConfig.bM0108EveryFirstDeviceMustOn,        ECBool,     "Monitor", "bMonitorEveryFirstDeviceMustEnabled",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_09"), &IniConfig.bM0109RTCOffCheckYieldPiggyBack,     ECBool,     "Monitor", "bM0109RTCOffCheckYieldPiggyBack",               bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_10"), &IniConfig.bM1010Disable_I12,                   ECBool,     "Monitor", "bM1010Disable_I12",                             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_12"), &IniConfig.bM1012EnableAutoClean,               ECBool,     "Monitor", "bM1012EnableAutoClean",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_13"), &IniConfig.bM1013Enable2DID,                    ECBool,     "Monitor", "bM1013Enable2DID",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbM01_14"), &IniConfig.bM1014EnableATC,                     ECBool,     "Monitor", "bM1014EnableATC",                               bNoShow, bDisable, bFixedValue, 0); //Steven 20191129 : For SCK
    }

}

// golden cConfiguration.cpp:3259  TfConfiguration::InitConfigEdtList_ItemN()
static void IC_InitConfigEdtList_ItemN()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";
    #ifdef SOFT_SIMULTE
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN05_SCKWebService"),  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bNoShow, bDisable, bFixedValue, 0);
    #else
    if(CUSTOMER_CODE==CC_SCK)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN05_SCKWebService"),  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bShow, bEnable, bReadFromFile, 0);             //Steven 20161201 : For SCK Web Service
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN05_SCKWebService"),  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bNoShow, bDisable, bFixedValue, 0);
    #endif
    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgUpDlMethod"),             &IniConfig.iN05_UpDLMethod,         ECInteger,  str, "iN05_UpDLMethod",                                     bShow, bEnable, bReadFromFile,  eByNetwork);   //Jimmychiu 20250707 : add RMS connect method

    if(CosFunction.bFTPUseBarcodeReader==true)
    {
      #ifndef SOFT_SIMULTE
        if(CUSTOMER_CODE==CC_CYUEAN)                                            //Sam 20230608 : 楊建軍說不要開關功能
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN06_UseBarcode"), &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bNoShow, bEnable, bFixedValue, 1);
        else
      #endif
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN06_UseBarcode"), &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN06_UseBarcode"),     &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN06_Tester"),            &IniConfig.bN06_CopyTesterFile,     ECBool,     "FTP", "bN06_CopyTesterFile",                               bShow, bEnable, bReadFromFile, 0);   //Steven 20250327 : OS測試機的工作檔也要上傳
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN06_FileName"),           &IniConfig.asN06_FileName,          ECText,     "FTP", "FTP File Name",                                     bShow, bEnable, bReadFromFile, "UnKnown");
    if(CosFunction.bEnable_SECS_GEM)                                            //Steven 20141006 : SECS GEM使用Remote Start功能
    {
        #ifndef SOFT_SIMULTE
        if((IniConfig.bSPILFunction==true &&
            CUSTOMER_CODE!=CC_XINYUN) ||                                        //RogerYang 20260626 : 芯云要放開，但芯云用bSPILFunction
            CUSTOMER_CODE==CC_JCET)                                             //JerryYang20170329 (wei) 矽品世明要求強制開啟SECS GEM  //JerryYang 20210107 : Add JCET
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs"), &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bShow, bDisable, bFixedValue, 1);
        else if(CUSTOMER_CODE==CC_ASE_KaohSiung && MachineTypeChoice==Type_HT9046)      //kevin 20181225 add HT9046 no use SECS GEM for ASE-KH
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs"), &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bNoShow, bDisable, bFixedValue, 0);
        else
        #endif
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs"), &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edN07_2"),              &IniConfig.iN07RunCheckAlarmTime,   ECInteger,  "Function", "iRunCheckAlarmTime",                           bShow, bEnable, bReadFromFile,  LastSet.iRunCheckAlarmTime, false, 10, 300);    //wei 20150512  Run Check Alarm Time

        if(CosFunction.bRCMDStart)
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung && MachineTypeChoice==Type_HT9046)
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableHostStart"), &IniConfig.bRCMDStart,     ECBool,     "SECS GEM", "Enable RCMD START",                            bNoShow, bDisable, bFixedValue, 0);
            else
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableHostStart"), &IniConfig.bRCMDStart,     ECBool,     "SECS GEM", "Enable RCMD START",                            bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableHostStart"),    &IniConfig.bRCMDStart,      ECBool,     "SECS GEM", "Enable RCMD START",                            bNoShow, bDisable, bFixedValue, 0);
        }

        if(CosFunction.bSECS_GEM_OneCycle)                                      //wei 20150824 Secs_Gem 斷線Onecycle
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsOneCycle"), &IniConfig.bSECS_GEM_OneCycle, ECBool,  "SECS GEM", "SECS GEM OneCycle",                            bShow, bEnable, bReadFromFile, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsOneCycle"), &IniConfig.bSECS_GEM_OneCycle, ECBool,  "SECS GEM", "SECS GEM OneCycle",                            bNoShow, bDisable, bFixedValue, 0);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN07_3_2"), &IniConfig.bN07_Alarm, ECBool, "SECS GEM", "SECS GEM Alarm",                                                          bShow, bEnable, bReadFromFile, 0);      //Steven 20260603 : Secs_Gem disconnect alarm

        if(CosFunction.bUseN07_5==true)                                         //Steven 20200309 : [N07-5]改成by客戶開啟
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableEmployeeCheak"),&IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bShow, bEnable, bReadFromFile, 0);    //Ifor 20180911 (Steven) : Add 啟動 Employee ID Check
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableEmployeeCheak"),&IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bNoShow, bDisable, bFixedValue, 0);

        if(CUSTOMER_CODE==CC_TSMC_TAINAN || CUSTOMER_CODE==CC_ASE_KaohSiung)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsLotCheck"), &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bShow, bEnable, bReadFromFile, 0);       //wei 20160727 Secs Gem Lot Check
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsLotCheck"), &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bNoShow, bDisable, bFixedValue, 0);

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
            elConfig->Add(EL<TEdit>("TfConfiguration", "edN07_5"), &IniConfig.iN07_EmployeeIdCheakTime,         ECInteger,  "SECS GEM", "iEmployeeIdCheckTime",                         bShow, bEnable, bReadFromFile,  5, false, 10, 300);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN07_6"), &IniConfig.bN07_6EnableUploadOSRecipe,          ECBool, "Specific", "bN07_6EnableUploadOSRecipe",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : OS測試機的工作檔也要上傳
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN07_6"), &IniConfig.sN07_6OSRecipePath,               ECFileName, "SECS GEM", "sN07_6OSRecipePath",                           bShow, bEnable, bReadFromFile, "Z:\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_6CompressedFile"), &IniConfig.bN07_6CompressedFile,   ECBool, "Specific", "bN07_6CompressedFile",                             bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : OS測試機的工作檔也要上傳
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN07_7"), &IniConfig.bN07_7SendRecipeAsBinary,            ECBool, "Specific", "bN07_7SendRecipeAsBinary",                         bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : 工作檔使用二進制上傳下載
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN07_7"), &IniConfig.dN07_7_DelayTime,                    ECDouble,  "SECS GEM", "iN07_7_DelayTime",                              bShow, bEnable, bReadFromFile,  1.0, false, 0.1, 10.0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs"),             &IniConfig.bEnable_SECS_GEM,        ECBool, "SECS GEM", "Enable SECS GEM",                          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableHostStart"),        &IniConfig.bRCMDStart,              ECBool, "SECS GEM", "Enable RCMD START",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsOneCycle"),     &IniConfig.bSECS_GEM_OneCycle,      ECBool, "SECS GEM", "SECS GEM OneCycle",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN07_3_2"),                   &IniConfig.bN07_Alarm,              ECBool, "SECS GEM", "SECS GEM Alarm",                           bNoShow, bDisable, bFixedValue, 0);      //Steven 20260603 : Secs_Gem disconnect alarm
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableEmployeeCheak"),    &IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecsLotCheck"),     &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bOLPFunction)                                                //Steven 20141229 : OLP的Log要存檔
    {
        EL<TTabSheet>("TfConfiguration", "tsN08")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN08_1"),  &IniConfig.bN08_1SaveOLPLog,                    ECBool,     "Automation", "Save Log",                                   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN08_2"),  &IniConfig.sN08OlpIP,                           ECText,     "OLP",        "OLP_IP",                                     bShow, bEnable, bReadFromFile, "192.168.120.143");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN08_3"),  &IniConfig.sN08OlpPort,                         ECText,     "OLP",        "OLP_Port",                                   bShow, bEnable, bReadFromFile, "6670");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN08")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN08_1"),  &IniConfig.bN08_1SaveOLPLog,                    ECBool,     "Automation", "Save Log",                                   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {                                                                           //Steven 20190521 : ATK lot count
        EL<TTabSheet>("TfConfiguration", "tsN09")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN09"),           &IniConfig.bN09_LotCountAutoFunc,       ECBool,     "Automation", "bN09_LotCountAutoFunc",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_TSV"),       &IniConfig.iN09_TSV_Port,               ECPort,     "Automation", "iN09_TSV_Port",                              bShow, bEnable, bReadFromFile, 4000,   false);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN09_2"),         &IniConfig.bN09_Enable_TSV,             ECBool,     "Automation", "bN09_Enable_TSV",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20231017 : add for ATK
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_SearchTime"),&IniConfig.dN09_SearchTime,             ECDouble,   "Automation", "dN09_SearchTime",                            bShow, bEnable, bReadFromFile, 0.0,    false,  1,      3000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_Handler"),   &IniConfig.sN09_HandlerFolder,          ECText,     "Automation", "sN09_HandlerFolder",                         bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\TestSummary");
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN09_4"),          &IniConfig.iN09_4_UploadMethod,         ECInteger,  "Automation", "iN09_4_UploadMethod",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_5User"),     &IniConfig.sN09_5_User,                 ECText,     "Automation", "sN19_5_User",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_5Password"), &IniConfig.sN09_5_Password,             ECPassword, "Automation", "sN19_5_Password",                            bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_5Host"),     &IniConfig.sN09_5_Host,                 ECText,     "Automation", "sN19_5_Host",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_5Path"),     &IniConfig.sN09_5_Path,                 ECText,     "Automation", "sN19_5_Path",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN09_7"),         &IniConfig.sN09_7_SkipIP,               ECText,     "Automation", "sN19_7_SkipIP",                              bShow, bEnable, bReadFromFile, "172,192");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN09")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN09"),           &IniConfig.bN09_LotCountAutoFunc,       ECBool,     "Automation", "bN09_LotCountAutoFunc",                      bNoShow, bDisable, bFixedValue, 0);
    }

    EL<TTabSheet>("TfConfiguration", "tsN10")->TabVisible=(CosFunction.bHiSiliconFunction ||                        //JerryYang 20180508 (jou) : fix N10 顯示問題
                       CosFunction.bUseLogUploadToFTPFunction ||
                       CosFunction.bHisiLogUploadNetwork ||
                       CUSTOMER_CODE==CC_HANA_MICRON);                          //JimmyChiu 20211008 R211005-Hana-H9-01

    if(CosFunction.bHiSiliconFunction ||
       CosFunction.bUseLogUploadToFTPFunction ||
       CosFunction.bHisiLogUploadNetwork)
    {
        if(IniConfig.bVTESTFunction==true)
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_1"),      &IniConfig.bN10Enable_FTPUpLoadLog,     ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_2"),      &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_3"),      &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bNoShow, bDisable, bFixedValue, 1);
            elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_3_1"),    &IniConfig.iN10UploadProductMethod,     ECInteger,  "FTPUpLoad", "iN10UploadProductMethod",                     bNoShow, bDisable, bFixedValue, 1);     //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
            elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_4"),      &IniConfig.iN10UploadMethod,            ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bNoShow, bDisable, bFixedValue, 1);

            EL<TRadioGroup>("TfConfiguration", "rgN10_3_1")->Visible=false;
            EL<TRadioGroup>("TfConfiguration", "rgN10_4")->Visible=false;

            EL<TGroupBox>("TfConfiguration", "grpN10_FTP")->Visible=false;
            EL<TLabel>("TfConfiguration", "lblN10_6")->Visible=false;
            EL<TEdit>("TfConfiguration", "edtN10_6")->Visible=false;

            EL<TCheckBox>("TfConfiguration", "cbL11_1")->Caption="[L11-1] ATC temperature range(1..3)";             //jou 20240426 : VTEST 要求 ATC 溫度限制設定1~3度
            EL<TEdit>("TfConfiguration", "edL11_1")->Tag=3;
        }
        else
        {
            if(CosFunction.bHiSiliconFunction==true ||                          //JerryYang 20170925 (Steven) Hisi版本強制開啟N10
               (CUSTOMER_CODE==CC_KYEC_LEE &&
                bEnable_KLT_Function==false))
            {
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20200110 add 高雄不需要
                    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_1"),  &IniConfig.bN10Enable_FTPUpLoadLog, ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bEnable, bReadFromFile, 0);       //Ifor 20160302 : Add FTP Upload Log To Host
                else
                    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_1"),  &IniConfig.bN10Enable_FTPUpLoadLog, ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_1"),  &IniConfig.bN10Enable_FTPUpLoadLog,     ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bEnable, bReadFromFile, 0);       //Ifor 20160302 : Add FTP Upload Log To Host
            }

            if(CUSTOMER_CODE==CC_KYEC_LEE)
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_2"),  &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bNoShow, bDisable, bFixedValue, 0);
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_3"),  &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bShow, bDisable, bFixedValue, 1);       //Steven 20180514 : JCET吳如春要求每日上傳Event Log Jam統計表 MTBF MUBF資料
            }
            else
            {
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_2"),  &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
                elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_3"),  &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20180514 : JCET吳如春要求每日上傳Event Log Jam統計表 MTBF MUBF資料
            }

    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_3_1"), &IniConfig.iN10UploadProductMethod, ECInteger, "FTPUpLoad", "iN10UploadProductMethod", bShow, bEnable, bReadFromFile, 0);  elConfig->Add(EL<filerw::ELDateTimePicker>("TfConfiguration", "dtpN10_3_1_SpecifiedTime"), &IniConfig.dN10_3_1_SpecifiedTime, ECDouble, "FTPUpLoad", "dN10_3_1_SpecifiedTime", bShow, bEnable, bReadFromFile, 0);   // golden cConfiguration.cpp:3444-3444 取代：AI(W906-FRW-W44-1) 20260928 ★W44-1＝A（Steven 0928）：N10-3-1 指定時間接上——golden dtpN10_3_1_SpecifiedTime 不在任何 elConfig->Add（912 dfm:15549、h:2160），IniConfig.dN10_3_1_SpecifiedTime 從不讀存；照 golden HTEditList 的 TDateTimePicker 支以 ECDouble 綁，節＝FTPUpLoad、鍵＝欄位名（ELA ReadScheduleConfig 讀的就是這個）
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:3444-3444 -- AI(W906-FRW-W44-1) 20260928 ★W44-1＝A（Steven 0928）：N10-3-1 指定時間接上——golden dtpN10_3_1_SpecifiedTime 不在任何 elConfig->Add（912 dfm:15549、h:2160），IniConfig.dN10_3_1_SpecifiedTime 從不讀存；照 golden HTEditList 的 TDateTimePicker 支以 ECDouble 綁，節＝FTPUpLoad、鍵＝欄位名（ELA ReadScheduleConfig 讀的就是這個）
            elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_3_1"),    &IniConfig.iN10UploadProductMethod,     ECInteger,  "FTPUpLoad", "iN10UploadProductMethod",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
#endif // GATE (S12-C save)
            if(CosFunction.bHisiLogUploadNetwork==true)                         //Steven 20190119 : Log上傳方式改為可選擇的
                elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_4"),      &IniConfig.iN10UploadMethod,        ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bShow, bEnable, bReadFromFile, 1);
            else
                elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_4"),      &IniConfig.iN10UploadMethod,        ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bShow, bEnable, bReadFromFile, 0);
        }

        if(CUSTOMER_CODE==CC_SIGURD_ChungXing ||                                //Sam 20170823 (wei) : 矽格中興 FTP Log 上傳增加時間格式選擇
           CUSTOMER_CODE==CC_SIGURD_PeiXing ||
           CUSTOMER_CODE==CC_SIGURD_HUKOU)                                      //Ifor 20180213 : add 矽格北興使用FTP Log 上傳增加時間格式選擇
        {
            elConfig->Add(EL<TComboBox>("TfConfiguration", "coN10DataType"),&IniConfig.iN10DataType,                ECInteger,  "FTPUpLoad", "iN10DataType",                                bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            EL<TComboBox>("TfConfiguration", "coN10DataType")->Visible=false;
            EL<TLabel>("TfConfiguration", "lblN10DataType")->Visible=false;
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_1"),      &IniConfig.bN10Enable_FTPUpLoadLog,         ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_2"),      &IniConfig.bN10_UploadSummaryToFTP,         ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_3"),      &IniConfig.bN10_DailyUploadProdData,        ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bNoShow, bDisable, bFixedValue, 0);
        EL<TComboBox>("TfConfiguration", "coN10DataType")->Visible=false;
        EL<TLabel>("TfConfiguration", "lblN10DataType")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN10UserName"),    &IniConfig.cN10FtpUserName,             ECText,     "FTPUpLoad",       "cN10FtpUserName",                       bShow, bEnable, bReadFromFile, "eap_handler_log");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN10_Passive"),   &IniConfig.bN10FtpPassive,              ECBool,     "FTPUpLoad",       "bN10FtpPassive",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtN10_Port"),      &IniConfig.iN10FtpPort,                 ECPort,     "FTPUpLoad",       "iN10FtpPort",                           bShow, bEnable, bReadFromFile, 21,   false);
        if(bEnable_KLT_Function==true)                                          //Ifor 20180813 (Steven) : Add 京隆 FTP 路徑
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Password"),&IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "abc1234!");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Host"),    &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "sz1ftp03.sz.com.cn");
        }
        else
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Password"),&IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "!qaz2wsx");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Host"),    &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "ch1fabfs05-ftp.kyec.com.tw");
        }
    }
    else
    {
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN10UserName"),    &IniConfig.cN10FtpUserName,             ECText,     "FTPUpLoad",       "cN10FtpUserName",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN10_Passive"),   &IniConfig.bN10FtpPassive,              ECBool,     "FTPUpLoad",       "bN10FtpPassive",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Password"),    &IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN10Host"),        &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtN10_Port"),      &IniConfig.iN10FtpPort,                 ECPort,     "FTPUpLoad",       "iN10FtpPort",                           bShow, bEnable, bReadFromFile, 21,   false);
    }

    if(IniConfig.bVTESTFunction==true && CosFunction.bHiSiliconFunction==false)
        ;
    else
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN10_6"),         &IniConfig.iN10UploadToHostIntervalTime,ECInteger,  "FTPUpLoad",       "iUploadToHostIntervalTime",             bShow, bEnable, bReadFromFile, 5, false,  5,      3600);

    elConfig->Add(EL<TEdit>("TfConfiguration", "edtN10_8"),             &IniConfig.sN10UploadDrivePath,         ECText,     "FTPUpLoad",       "sN10UploadDrivePath",                   bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);

    if(CosFunction.bSaveProductionLogByUnloaderTray==true)                      //Steven 20200409 : production log by unloader tray存檔並上傳FTP
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN10_9"),         &IniConfig.bN10_9_UploadUnloadTrayToFTP,ECBool,     "FTPUpLoad",        "bN10_9_UploadUnloadTrayToFTP",         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN10_9"),         &IniConfig.bN10_9_UploadUnloadTrayToFTP,ECBool,     "FTPUpLoad",        "bN10_9_UploadUnloadTrayToFTP",         bNoShow, bDisable, bFixedValue, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN10UploadPath"),      &IniConfig.cN10FtpUplaodPath,           ECText,     "FTPUpLoad",       "cN10FtpUplaodPath",                     bShow, bEnable, bReadFromFile, "\\");

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_11"),             &IniConfig.bN10_11_Enable_UploadFTPEventLog,ECBool, "FTPUpLoad",        "bN10_11_Enable_UploadFTPEventLog",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN10_12"),             &IniConfig.bN10_12_Enable_UploadFTPGPIBLog, ECBool, "FTPUpLoad",        "bN10_12_Enable_UploadFTPGPIBLog",      bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20160802 Clean Out Low Yield Close Site
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN11_1"),      &IniConfig.bN11_1CleanOutCloseSite,         ECBool,     "NETWORK", "bN11_1CleanOutCloseSite",                       bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN11")->TabVisible=false;                                                //Steven 20160804 : modify [N11] for 高雄
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN11_1"),      &IniConfig.bN11_1CleanOutCloseSite,         ECBool,     "NETWORK", "bN11_1CleanOutCloseSite",                       bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20170525 (wei) 力成 add Socket ID Product Data Upload To FTP
    {
        EL<TTabSheet>("TfConfiguration", "tsN12")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN12"),  &IniConfig.bN12_EnableSocketIdProductDataFTP,    ECBool,     "FTP",     "Second Enable FTP",                             bShow, bEnable, bReadFromFile, 0);      //Sam 20170525 (wei) 力成 add Socket ID Product Data Upload To FTP
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN12_UserName"),   &IniConfig.asN12_FtpUserName,           ECText,     "Automation",       "Second FTP User Name",                 bShow, bEnable, bReadFromFile, "UnKnown");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN12_Password"),   &IniConfig.asN12_FtpPassword,           ECPassword, "Automation",       "Second FTP Password",                  bShow, bEnable, bReadFromFile, "1234");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN12_HostName"),   &IniConfig.asN12_FtpHost,               ECText,     "Automation",       "Second FTP Host",                      bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN12_UpLdPath"),   &IniConfig.asN12_FtpUplaodPath,         ECText,     "Automation",       "Second FTP Upload Path",               bShow, bEnable, bReadFromFile, "/");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN12")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN12"),  &IniConfig.bN12_EnableSocketIdProductDataFTP,    ECBool,     "FTP",     "Second Enable FTP",                             bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseARMSFunction)                                            //Ifor 20170621 (wei) add ARMS Function
    {
        EL<TTabSheet>("TfConfiguration", "tsN13")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN13_EnableARMSFunction"), &IniConfig.bN13_EnableARMSFunction, ECBool, "ARMS", "bN13_EnableARMSFunction",                          bShow, bEnable, bReadFromFile, 0);      //Ifor 20170621 (wei) add ARMS Function
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN13")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN13_EnableARMSFunction"), &IniConfig.bN13_EnableARMSFunction, ECBool, "ARMS", "bN13_EnableARMSFunction",                          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN14")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_1"),          &IniConfig.bN14_1_EnableOEEFunction,    ECBool,     "Handler_OEE", "N14_HandlerOEEUseFunction",                 bShow, bEnable, bReadFromFile, 0);      //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_1"), &IniConfig.iN14_1_OEERecordCycleTime,   ECInteger,  "Handler_OEE", "N14_HandlerOEERecordCycleTime",             bShow, bEnable, bReadFromFile, 1800,    false, 60,       3000);  //Sam 20220922 : 改用 elConfig 方式
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_2"),  &IniConfig.bN14_2_OEEUseSaveProdData,   ECBool,     "Handler_OEE", "N14_HandlerOEEUseSaveProductionDataToPath", bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_2"), &IniConfig.asN14_2_OEESaveProdPath,     ECText,     "Handler_OEE", "N14_HandlerOEESaveProductionDataToPath",    bShow,bEnable, bReadFromFile, "D:\\ProductionInfoBackup");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_3"),  &IniConfig.bN14_3_OEEFTPUpload,         ECBool,     "Handler_OEE", "N14_HandlerOEEFTPUpload",                   bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_3UserName"), &IniConfig.asN14_3_OEEFTPUserName,      ECText,     "Handler_OEE", "N14_HandlerOEEUserName",                    bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_3Password"), &IniConfig.asN14_3_OEEFTPPassword,      ECPassword, "Handler_OEE", "N14_HandlerOEEPassword",                    bShow, bEnable, bReadFromFile, " ");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_3Host"),     &IniConfig.asN14_3_OEEFTPHost,          ECText,     "Handler_OEE", "N14_HandlerOEEHost",                        bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_3Path"),     &IniConfig.asN14_3_OEEFTPUploadPath,    ECText,     "Handler_OEE", "N14_HandlerOEEUploadPath",                  bShow, bEnable, bReadFromFile, "\\OEEE\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_4"),          &IniConfig.bN14_4_OEEAutoLoadMOFile,    ECBool,     "Handler_OEE", "N14_HandlerOEEAutoLoadMOFile",              bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_4"),         &IniConfig.asN14_4_MODownloadPath,      ECText,     "Handler_OEE", "N14_HandlerMODownloadPath",                 bShow, bEnable, bReadFromFile, "\\SETUP_FILE\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_5"),         &IniConfig.iN14_5_PauseIntervalTime,    ECInteger,  "Handler_OEE", "N14_PauseIntervalTimeSec",                  bShow, bEnable, bReadFromFile, 600,     false, 60,      3000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_6_1"),       &IniConfig.iN14_6_BySiteContactCnt,     ECInteger,  "Handler_OEE", "iN14_6_BySiteContactCnt",                   bShow, bEnable, bReadFromFile, 1800,    false, 1,       3000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_6_2"),       &IniConfig.dN14_6_BySiteLowYieldRate,   ECDouble,   "Handler_OEE", "dN14_6_BySiteLowYieldRate",                 bShow, bEnable, bReadFromFile, 100.0,   false, 1.0,     100.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_6_3"),       &IniConfig.dN14_6_BySiteCmpYield,       ECDouble,   "Handler_OEE", "dN14_6_BySiteCmpYield",                     bShow, bEnable, bReadFromFile, 100.0,   false, 1.0,     100.0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_6_4"),       &IniConfig.iN14_6_BySiteAlarmYieldRate, ECInteger,  "Handler_OEE", "iN14_6_BySiteAlarmYieldRate",               bShow, bEnable, bReadFromFile, 100,     false, 1,       100);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_7"),          &IniConfig.bN14_7_AutoMotive,           ECBool,     "Handler_OEE", "bN14_7_AutoMotive",                         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_7"),         &IniConfig.asN14_7_AutoMotivePath,      ECText,     "Handler_OEE", "asN14_7_AutoMotivePath",                    bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_8"),          &IniConfig.bN14_8_ULSetup,              ECBool,     "Handler_OEE", "bN14_8_ULSetup",                            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_8"),         &IniConfig.asN14_8_ULSetupPath,         ECText,     "Handler_OEE", "asN14_8_ULSetupPath",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_9"),          &IniConfig.bN14_9_ULQtyReport,          ECBool,     "Handler_OEE", "bN14_9_ULQtyReport",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_9"),         &IniConfig.asN14_9_ULQtyReportPath,     ECText,     "Handler_OEE", "asN14_9_ULQtyReportPath",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_10"),         &IniConfig.bN14_10_DownFileByMO,        ECBool,     "Handler_OEE", "N14_AutoDownloadSetupFileByMO",             bShow, bEnable, bReadFromFile, 0);      //Sam 20190227 : Try Catch Mo Download SetupFile
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_11"),         &IniConfig.bN14_11_CheckSiteMapByMO,    ECBool,     "Handler_OEE", "bN14_11_CheckSiteMapByMO",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_12"),         &IniConfig.bN14_12_ULTempLogToFTP,      ECBool,     "Handler_OEE", "bN14_12_ULTempLogToFTP",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_12"),        &IniConfig.iN14_12_ULTempLogInterval,   ECInteger,  "Handler_OEE", "iN14_12_ULTempLogInterval",                 bShow, bEnable, bReadFromFile, 100,     false, 1,       3000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_12_Path"),   &IniConfig.asN14_12_ULTempLogPath,      ECText,     "Handler_OEE", "asN14_12_ULTempLogPath",                    bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_13"),         &IniConfig.bN14_13_ULBinQtyToFTP,       ECBool,     "Handler_OEE", "bN14_13_ULBinQtyToFTP",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_13"),        &IniConfig.iN14_13_ULBinQtyInterval,    ECInteger,  "Handler_OEE", "iN14_13_ULBinQtyInterval",                  bShow, bEnable, bReadFromFile, 100,     false, 1,       3000);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_13_Path"),   &IniConfig.asN14_13_ULBinQtyPath,       ECText,     "Handler_OEE", "asN14_13_ULBinQtyPath",                     bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_14"),         &IniConfig.bN14_14_AlarmCtrlMachine,    ECBool,     "Handler_OEE", "bN14_14_AlarmCtrlMachine",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_14_1"),      &IniConfig.asN14_14_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_14_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\EMG.exe");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_14_2"),      &IniConfig.asN14_14_MessageFilePath,    ECText,     "Handler_OEE", "asN14_14_MessageFilePath",                  bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Message\\Alarm\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_14_3"),      &IniConfig.asN14_14_FlagFilePath,       ECText,     "Handler_OEE", "asN14_14_FlagFilePath",                     bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Flag\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_15"),         &IniConfig.bN14_15_SocketLifeTime,      ECBool,     "Handler_OEE", "bN14_15_SocketLifeTime",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_15_1"),      &IniConfig.asN14_15_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_15_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\Lifetime_count.exe");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_15_2"),      &IniConfig.asN14_15_MessageFilePath,    ECText,     "Handler_OEE", "asN14_15_MessageFilePath",                  bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Message\\count\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_16"),         &IniConfig.bN14_16_EnableIPSC,          ECBool,     "Handler_OEE", "bN14_16_EnableIPSC",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_16_1"),      &IniConfig.asN14_16_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_16_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\IPSC.exe");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_16_2"),      &IniConfig.asN14_16_FlagFilePath,       ECText,     "Handler_OEE", "asN14_16_FlagFilePath",                     bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Flag\\IPSC.ini");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_16_3"),      &IniConfig.asN14_16_ProductionFilePath, ECText,     "Handler_OEE", "asN14_16_ProductionFilePath",               bShow, bEnable, bReadFromFile, "C:\\GTK_CONTROL\\MESSAGE\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_16_4"),      &IniConfig.iN14_16_IPSCInterval,        ECInteger,  "Handler_OEE", "iN14_16_IPSCInterval",                      bShow, bEnable, bReadFromFile, 2,     false, 1,       100);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_17"),        &IniConfig.iN14_17_AmbientULTemp,       ECInteger,  "Handler_OEE", "iN14_17_AmbientULTemp",                     bShow, bEnable, bReadFromFile, 39,     false, 1,       100);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_18"),         &IniConfig.bN14_18_EnableTempOffset,    ECBool,     "Handler_OEE", "bN14_18_EnableTempOffset",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20200806 : 溫度 By Servo
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_18"),        &IniConfig.asN14_18_TempOffsetPath,     ECText,     "Handler_OEE", "asN14_18_TempOffsetPath",                   bShow, bEnable, bReadFromFile, "\\Handler_data\\Taj_offset_Tool");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_19"),         &IniConfig.bN14_19_TrayMappingToFTP,    ECBool,     "Handler_OEE", "bN14_19_TrayMappingToFTP",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20201209 : 增加資料上傳
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_19"),        &IniConfig.asN14_19_TrayMappingPath,    ECText,     "Handler_OEE", "asN14_19_TrayMappingPath",                  bShow, bEnable, bReadFromFile, "\\TrayMapping\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_20_1"),&IniConfig.bN14_20_DefaultRecipeChangeLogCycleRecord,ECBool,"Handler_OEE", "bN14_20_DefaultRecipeChangeLogCycleRecord",bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_20"),  &IniConfig.bN14_20_DefaultRecipeChangeLog,     ECBool,     "Handler_OEE", "bN14_20_DefaultRecipeChangeLog",            bShow, bEnable, bReadFromFile, 0);  //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN14_20_1"),       &IniConfig.iN14_20_CycleTime,           ECInteger,  "Handler_OEE", "iN14_20_CycleTime",                         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN14_20"),         &IniConfig.asN14_20_ChangeLogPath,      ECText,     "Handler_OEE", "asN14_20_DefaultRecipeChangeLogPath",       bShow, bEnable, bReadFromFile, "\\RecipeChangeLog\\");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_21"),         &IniConfig.bN14_21_SetUpConfiguration,  ECBool,     "Handler_OEE", "bN14_21_SetUpConfiguration",                bShow, bEnable, bReadFromFile, 0);  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN14_21"),         &IniConfig.asN14_21_SetUpConfiguration, ECText,     "Handler_OEE", "asN14_21_SetUpConfiguration",               bShow, bDisable, bFixedValue, "//SetupFile_List//SiteMap//HT9046//");   //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN14_21_BC"),      &IniConfig.asN14_21_BinCategory,        ECText,     "Handler_OEE", "asN14_21_BinCategory",                      bShow, bDisable, bFixedValue, "//SetupFile_List//BinCategory//HT9046//");  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN14_21_HP"),      &IniConfig.asN14_21_HP_SetUpConfig,     ECText,     "Handler_OEE", "asN14_21_HP_SetUpConfiguration",            bShow, bDisable, bFixedValue, "//SetupFile_List//Other//HT90XX-HotPlateForm//");  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN14_21_TF"),      &IniConfig.asN14_21_TF_SetUpConfig,     ECText,     "Handler_OEE", "asN14_21_TF_SetUpConfiguration",            bShow, bDisable, bFixedValue, "//SetupFile_List//TrayForm//");               //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_22"),         &IniConfig.bN14_21_ConfigUpdateFromServerExport,    ECBool,   "Handler_OEE", "bN14_21_ConfigUpdateFromServerExport",   bShow, bEnable, bReadFromFile, 0);  //JimmyChiu 20230410 : Config update from server
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_22Exp"),     &IniConfig.asN14_22_ConfigUpdateFromServerExport,   ECText,   "Handler_OEE", "asN14_22_ConfigUpdateFromServerExport",  bShow, bEnable, bReadFromFile, "//handler_data//Config//export//");             //JimmyChiu 20230410 : Config update from server
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN14_22Imp"),     &IniConfig.asN14_22_ConfigUpdateFromServerImport,   ECText,   "Handler_OEE", "asN14_22_ConfigUpdateFromServerImport",  bShow, bEnable, bReadFromFile, "//handler_datas//Config//import//");             //JimmyChiu 20230410 : Config update from server
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cb14_23"),          &IniConfig.bN14_23_ReadTextFileforPassword,         ECBool,   "Handler_OEE",             "bN14_23_ReadTextFileforPassword",           bShow, bEnable, bReadFromFile, 0);                                              //JimmyChiu 20240115 : Read Text File for Password
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cb14_24"),          &IniConfig.bN14_24_DynaMultiContinuPassSocket,      ECBool,   "Handler_OEE",             "bN14_24_DynaMultiContinuPassSocket",        bShow, bEnable, bReadFromFile, 0);                                              //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed14_24"),          &IniConfig.iN14_24_DyMultiPassPower,                ECInteger,"Handler_OEE",             "iN14_24_DyMultiPassPower",                  bShow, bEnable, bReadFromFile, 5);                                              //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN14")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_1"),  &IniConfig.bN14_1_EnableOEEFunction,            ECBool,     "Handler_OEE", "N14_HandlerOEEUseFunction",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_2"),  &IniConfig.bN14_2_OEEUseSaveProdData,           ECBool,     "Handler_OEE", "N14_HandlerOEEUseSaveProductionDataToPath", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_3"),  &IniConfig.bN14_3_OEEFTPUpload,                 ECBool,     "Handler_OEE", "N14_HandlerOEEFTPUpload",                   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_4"),  &IniConfig.bN14_4_OEEAutoLoadMOFile,            ECBool,     "Handler_OEE", "N14_HandlerOEEAutoLoadMOFile",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_7"),  &IniConfig.bN14_7_AutoMotive,                   ECBool,     "Handler_OEE", "bN14_7_AutoMotive",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_8"),  &IniConfig.bN14_8_ULSetup,                      ECBool,     "Handler_OEE", "bN14_8_ULSetup",                            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_9"),  &IniConfig.bN14_9_ULQtyReport,                  ECBool,     "Handler_OEE", "bN14_9_ULQtyReport",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_10"), &IniConfig.bN14_10_DownFileByMO,                ECBool,     "Handler_OEE", "N14_AutoDownloadSetupFileByMO",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_11"), &IniConfig.bN14_11_CheckSiteMapByMO,            ECBool,     "Handler_OEE", "bN14_11_CheckSiteMapByMO",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_12"), &IniConfig.bN14_12_ULTempLogToFTP,              ECBool,     "Handler_OEE", "bN14_12_ULTempLogToFTP",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_13"), &IniConfig.bN14_13_ULBinQtyToFTP,               ECBool,     "Handler_OEE", "bN14_13_ULBinQtyToFTP",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_14"), &IniConfig.bN14_14_AlarmCtrlMachine,            ECBool,     "Handler_OEE", "bN14_14_AlarmCtrlMachine",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_15"), &IniConfig.bN14_15_SocketLifeTime,              ECBool,     "Handler_OEE", "bN14_15_SocketLifeTime",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_16"), &IniConfig.bN14_16_EnableIPSC,                  ECBool,     "Handler_OEE", "bN14_16_EnableIPSC",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_18"), &IniConfig.bN14_18_EnableTempOffset,            ECBool,     "Handler_OEE", "bN14_18_EnableTempOffset",                  bNoShow, bDisable, bFixedValue, 0); //Sam 20200806 : 溫度 By Servo
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_19"), &IniConfig.bN14_19_TrayMappingToFTP,            ECBool,     "Handler_OEE", "bN14_19_TrayMappingToFTP",                  bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : 增加資料上傳
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_20"), &IniConfig.bN14_20_DefaultRecipeChangeLog,      ECBool,     "Handler_OEE", "bN14_20_DefaultRecipeChangeLog",            bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_20_1"),&IniConfig.bN14_20_DefaultRecipeChangeLogCycleRecord,ECBool,"Handler_OEE", "bN14_20_DefaultRecipeChangeLogCycleRecord",bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN14_21"), &IniConfig.bN14_21_SetUpConfiguration,          ECBool,     "Handler_OEE", "bN14_21_SetUpConfiguration",                bNoShow, bDisable, bFixedValue, 0); //JimmyChiu 20220303 : Add class SetUpConfiguration
    }

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20220922 : 改用 elConfig 方式
    {
        EL<TTabSheet>("TfConfiguration", "tsN15")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN15_1"),          &IniConfig.bN15UserLevelByTxt,          ECBool,     "ESD_Control", "N15_ESDControlUserLevelByTxt",              bShow, bEnable, bReadFromFile, 0);  //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN15_2"),          &IniConfig.bN15UseESDControlMachine,    ECBool,     "ESD_Control", "N15_ESDControlUseMachine",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN15_1"),         &IniConfig.asN15UserLevelByTxtReadFilePath,         ECText,   "ESD_Control",        "N15_ESDControlUserLevelByTxtReadFilePath", bShow, bEnable, bReadFromFile, "\\PASSWORD\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN15_2"),         &IniConfig.asN15UseESDControlMachineReadFilePath,   ECText,   "ESD_Control",        "N15_ESDControlUseMachineReadFilePath",     bShow, bEnable, bReadFromFile, "\\FT_ESD\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN15_UserName"),   &IniConfig.asN15ESDFTP_UserName,                    ECText,   "ESD_Control",        "N15_ESDControlFTP_UserName",               bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN15_Password"),   &IniConfig.asN15ESDFTP_Password,                    ECPassword,"ESD_Control",       "N15_ESDControlFTP_Password",               bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN15_Host"),       &IniConfig.asN15ESDFTP_Host,                        ECText,   "ESD_Control",        "N15_ESDControlFTP_Host",                   bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN15_3"),          &IniConfig.asN15ESDControlMachineSaveRecordFilePath,ECText,   "ESD_Control",        "N15_ESDControlMachineSaveRecordFilePath",  bShow, bEnable, bReadFromFile, "D:\\ESDControlLog");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN15_4"),         &IniConfig.asN15HandlerAUTOMOTIVEDownloadPath,      ECText,   "ESD_Control",        "N15_HandlerAUTOMOTIVEDownloadPath",        bShow, bEnable, bReadFromFile, "\\AUTOMOTIVE_FILE\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN15")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN15_1"),  &IniConfig.bN15UserLevelByTxt,                  ECBool,     "ESD_Control", "N15_ESDControlUserLevelByTxt",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN15_2"),  &IniConfig.bN15UseESDControlMachine,            ECBool,     "ESD_Control", "N15_ESDControlUseMachine",                  bNoShow, bDisable, bFixedValue, 0);          //JimmyChiu 20220303 : Add class SetUpConfiguration
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20170821 offset ftp
    {
        EL<TTabSheet>("TfConfiguration", "tsN16")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN16"),  &IniConfig.bEnableOffsetFTP,                     ECBool,     "N16", "bEnableOffsetFTP",                                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN16_UserName"),   &IniConfig.cN16FtpUserName,             ECText,     "N16", "cN16FtpUserName",                                   bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN16_Password"),   &IniConfig.cN16FtpPassword,             ECPassword, "N16", "cN16FtpPassword",                                   bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN16_HostName"),   &IniConfig.cN16FtpHost,                 ECText,     "N16", "cN16FtpHost",                                       bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN16_DownPath"),   &IniConfig.cN16FtpDownloadPath,         ECText,     "N16", "cN16FtpDownloadPath",                               bShow, bEnable, bReadFromFile, "D:\\ESDControlLog");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN16_UpLdPath"),   &IniConfig.cN16FtpUplaodPath,           ECText,     "N16", "cN16FtpUplaodPath",                                 bShow, bEnable, bReadFromFile, "\\AUTOMOTIVE_FILE\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN16")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN16"),  &IniConfig.bEnableOffsetFTP,                     ECBool,     "N16", "bEnableOffsetFTP",                                  bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20220923 : [N17] upload lot summary
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20230131 : add Qualcomm
    {
        EL<TTabSheet>("TfConfiguration", "tsN17")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN17_1"),          &IniConfig.bN17UploadLotSummary,        ECBool,     "Lot_Summary",      "bN17UploadLotSummary",                 bShow, bEnable, bReadFromFile, 0);  //JerryYang 20220923 : [N17] upload lot summary
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN17_2"),          &IniConfig.asN17LotSummaryPath,         ECText,     "Lot_Summary",      "asN17LotSummaryPath",                  bShow, bEnable, bReadFromFile, "\\\\handler-hs\\HANDLER_TEMP\\RAWDATA\\JOBEND_LOG\\");  //JerryYang 20220923 : [N17] upload lot summary
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN17_3"),          &IniConfig.bN17UploadProdLog,           ECBool,     "Production_Log",   "bN17UploadProdLog",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN17_4"),          &IniConfig.asN17ProductionLogPath,      ECText,     "Production_Log",   "asN17ProductionLogPath",               bShow, bEnable, bReadFromFile, "D:\\RMS\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN17")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN17_1"),          &IniConfig.bN17UploadLotSummary,        ECBool,     "Lot_Summary",      "bN17UploadLotSummary",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN17_3"),          &IniConfig.bN17UploadProdLog,           ECBool,     "Production_Log",   "bN17UploadProdLog",                    bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN20"),               &IniConfig.bN20_CheckMD5,               ECBool,     "NETWORK", "bN20_CheckMD5",                                 bShow, bEnable, bReadFromFile, 0);      //Steven 20170927 (wei) : 比對工作檔的檢查碼是否正確

    if(CosFunction.bHandlerStateChangeUploadServer)                             //Sam 20230511 : 機台資料變更後須上傳 FTP
    {
        EL<TTabSheet>("TfConfiguration", "tsN21")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN21_1"), &IniConfig.bN21_HandlerChangeStateUploadServer, ECBool,     "bHandlerStateChangeUploadServer", "bN21_HandlerChangeStateUploadServer",bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN21_1"),  &IniConfig.sN21_FTPUserName,                    ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPUserName",     bShow, bEnable, bReadFromFile, "SGPHANDLER");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN21_2"),  &IniConfig.sN21_FTPPassword,                    ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPPassword",     bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN21_3"),  &IniConfig.sN21_FTPHost,                        ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPHost",         bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN21_4"),  &IniConfig.sN21_FTPUploadPath,                  ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPUploadPath", bShow, bEnable, bReadFromFile, "\\Handler\\HandlerStateChange\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN21")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN21_1"), &IniConfig.bN21_HandlerChangeStateUploadServer, ECBool,     "bHandlerStateChangeUploadServer", "bN21_HandlerChangeStateUploadServer",bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Steven 20181224 : For ASE-CL
    {
        EL<TTabSheet>("TfConfiguration", "tsN22")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN22_EveltLog"),       &IniConfig.bN22Enable_EventLog,     ECBool,     "ASECL_FTP", "bN22Enable_EventLog",                         bShow, bEnable, bReadFromFile, 0);      //Steven 20181224 : For ASE-CL
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN22"),                &IniConfig.bN22Enable_ASE_CL_FTP,   ECBool,     "ASECL_FTP", "bN22Enable_ASE_CL_FTP",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_UserName"),      &IniConfig.sN22ASE_CL_FTPUserName,  ECText,     "ASECL_FTP", "sN22ASE_CL_FTPUserName",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_Password"),      &IniConfig.sN22ASE_CL_FTPPassword,  ECPassword, "ASECL_FTP", "sN22ASE_CL_FTPPassword",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_HostAddress"),   &IniConfig.sN22ASE_CL_FTPHost,      ECText,     "ASECL_FTP", "sN22ASE_CL_FTPHostc",                         bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_DownloadPath"),  &IniConfig.sN22ASE_CL_FTPDownlaodPath, ECText,  "ASECL_FTP", "sN22ASE_CL_FTPDownlaodPath",                  bShow, bEnable, bReadFromFile, "");

        cbConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbN23"),                    &IniConfig.bN23UploadJHT_Log,           ECBool,  "ASECL_FTP", "bN23UploadJHT_Log",             bShow, bEnable, bReadFromFile, 1);
        cbConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbN35"),                    &IniConfig.bN35UploadJHT_Log,           ECBool,  "ASECL_FTP", "bN35UploadJHT_Log",             bShow, bEnable, bReadFromFile, 1);
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N22_1_UserName"),        &IniConfig.sN22_1ASE_CL_FTPUserName,    ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPUserName",      bShow, bEnable, bReadFromFile, AnsiString("cuauser"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N22_1_Password"),        &IniConfig.sN22_1ASE_CL_FTPPassword,    ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPPassword",      bShow, bEnable, bReadFromFile, AnsiString("cuaasecl"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N22_1_HostAddress"),     &IniConfig.sN22_1ASE_CL_FTPHost,        ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPHost",          bShow, bEnable, bReadFromFile, AnsiString("10.14.97.228"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N22_1_UploadPath"),      &IniConfig.sN22_1ASE_CL_FTPUplaodPath,  ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPUplaodPath",    bShow, bEnable, bReadFromFile, AnsiString("//TestData//DATA_TRANSFER//FT//UPLOADSPACE//CUA/TRAY_MAP//DATA_IN//"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N35_1_UserName"),        &IniConfig.sN35_1ASE_CL_FTPUserName,    ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPUserName",      bShow, bEnable, bReadFromFile, AnsiString("cuauser"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N35_1_Password"),        &IniConfig.sN35_1ASE_CL_FTPPassword,    ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPPassword",      bShow, bEnable, bReadFromFile, AnsiString("cuaasecl"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N35_1_HostAddress"),     &IniConfig.sN35_1ASE_CL_FTPHost,        ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPHost",          bShow, bEnable, bReadFromFile, AnsiString("10.14.97.228"));
        elConfig_byRecipe->Add(EL<TEdit>("TfConfiguration", "ed_N35_1_UploadPath"),      &IniConfig.sN35_1ASE_CL_FTPUplaodPath,  ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPUplaodPath",    bShow, bEnable, bReadFromFile, AnsiString("//TestData//DATA_TRANSFER//FT//UPLOADSPACE//SQA/TRAY_MAP//DATA_IN//"));
    }
    else if(CUSTOMER_CODE==CC_HANA_MICRON)                                      //Steven 20251007 : Hana Micron Tray Map
    {
        EL<TTabSheet>("TfConfiguration", "tsN22")->TabVisible               =true;
        EL<TCheckBox>("TfConfiguration", "cbN22_EveltLog")->Visible         =false;
        IniConfig.bN22Enable_EventLog   =false;
        IniConfig.bN22Enable_ASE_CL_FTP =false;
        EL<TLabel>("TfConfiguration", "lb_N22_DownloadPath")->Caption    ="Upload Path";

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN22"),                &IniConfig.bN22_1_HANA_TrayMapFTP,  ECBool,     "HANA_FTP", "bN22_1_HANA_TrayMapFTP",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20181224 : For ASE-CL
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_UserName"),      &IniConfig.sN22_1_FTPUserName,      ECText,     "HANA_FTP", "sN22_1_FTPUserName",                           bShow, bEnable, bReadFromFile, "traymap");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_Password"),      &IniConfig.sN22_1_FTPPassword,      ECPassword, "HANA_FTP", "sN22_1_FTPPassword",                           bShow, bEnable, bReadFromFile, "map0206");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_HostAddress"),   &IniConfig.sN22_1_FTPHost,          ECText,     "HANA_FTP", "sN22_1_FTPHost",                               bShow, bEnable, bReadFromFile, "12.230.55.65");
        elConfig->Add(EL<TEdit>("TfConfiguration", "ed_N22_DownloadPath"),  &IniConfig.sN22_1_FTPUploadPath,    ECText,     "HANA_FTP", "sN22_1_FTPUploadPath",                         bShow, bEnable, bReadFromFile, "\\handlerid\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN22")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN22_EveltLog"),       &IniConfig.bN22Enable_EventLog,     ECBool,     "ASECL_FTP", "bN22Enable_EventLog",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN22"),                &IniConfig.bN22Enable_ASE_CL_FTP,   ECBool,     "ASECL_FTP", "bN22Enable_ASE_CL_FTP",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN23"),                &IniConfig.bN23UploadJHT_Log,       ECBool,    "ASECL_FTP", "bN23UploadJHT_Log",                           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN35"),                &IniConfig.bN35UploadJHT_Log,       ECBool,    "ASECL_FTP", "bN35UploadJHT_Log",                           bNoShow, bDisable, bFixedValue, 0);
    }

    EL<TTabSheet>("TfConfiguration", "tsN23")->TabVisible  =(CUSTOMER_CODE==CC_Murata || CosFunction.bSortingBy2DList || IniConfig.bSPILFunction==true);
    EL<TTabSheet>("TfConfiguration", "tsN23_1")->TabVisible=(CUSTOMER_CODE==CC_Murata);
    EL<TTabSheet>("TfConfiguration", "tsN23_2")->TabVisible=(CUSTOMER_CODE==CC_Murata);
    EL<TTabSheet>("TfConfiguration", "tsN23_3")->TabVisible=(CUSTOMER_CODE==CC_Murata ||
                        IniConfig.bSPILFunction==true ||                        //JerryYang 20250521 : add
                        CUSTOMER_CODE==CC_SJ_Semiconductor);                    //RogerYang 2026024 : Add for SJSM白名單
    EL<TLabeledEdit>("TfConfiguration", "lbledtN23_4_URL")->Text=IniConfig.sN23_4_URL;                                 //JerryYang 20241104 : 支援2DID白名單功能
    EL<TLabeledEdit>("TfConfiguration", "lbledtN23_5_UploadPath")->Text=IniConfig.sN23_5_UploadPath;

    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200409 : Murata 2DID比對功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_1"),             &IniConfig.bN23_1_Enable2DIDCompare,ECBool,     "Murata Function", "bN23_1_Enable2DIDCompare",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_3"),             &IniConfig.bN23_3_UploadTestResult, ECBool,     "Murata Function", "bN23_3_UploadTestResult",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN23_1_URL"),      &IniConfig.sN23_1_URL,              ECText,     "Murata Function", "sN23_1_URL",                            bShow, bEnable, bReadFromFile, "http://172.16.83.190/sm5200/page/TrafficExecute.jsp?pptid=519149&argument=519149.args");
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtN23_2_LineID"),      &IniConfig.sN23_2_Line,             ECText,     "Murata Function", "sN23_2_Line",                           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtN23_2_ProcessName"), &IniConfig.sN23_2_Process,          ECText,     "Murata Function", "sN23_2_Process",                        bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "edtN23_2_Product"),     &IniConfig.sN23_2_Product,          ECText,     "Murata Function", "sN23_2_Product",                        bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN23_2_URL"),      &IniConfig.sN23_3_URL,              ECText,     "Murata Function", "sN23_3_URL",                            bShow, bEnable, bReadFromFile, "http://172.16.83.190/sm5200/page/TrafficExecute.jsp?pptid=519129&argument=519129.args");
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_1"),             &IniConfig.bN23_1_Enable2DIDCompare,ECBool,     "Murata Function", "bN23_1_Enable2DIDCompare",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_3"),             &IniConfig.bN23_3_UploadTestResult, ECBool,     "Murata Function", "bN23_3_UploadTestResult",               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bSortingBy2DList)
    {
        EL<TTabSheet>("TfConfiguration", "tsN23")->TabVisible=true;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2UserName"),      &IniConfig.cN23FtpUserName,         ECText,     "2DSotingFunction",     "cN23FtpUserName",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2Pwd"),           &IniConfig.cN23FtpPassword,         ECPassword, "2DSotingFunction",     "cN23FtpPassword",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2FTPHost"),       &IniConfig.cN23FtpHost,             ECText,     "2DSotingFunction",     "cN23FtpHost",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2FTPPath"),       &IniConfig.cN23FtpDownloadPath,     ECText,     "2DSotingFunction",     "cN23FtpDownloadPath",              bShow, bEnable, bReadFromFile, "\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_3NetDrivePath"),  &IniConfig.sN23DownloadDrivePath,   ECText,     "2DSotingFunction",     "sN23DownloadDrivePath",            bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN23_1_2DSorting"),    &IniConfig.iN23DownloadMethod,      ECInteger,  "2DSotingFunction",     "iN23DownloadMethod",               bShow, bEnable, bReadFromFile, 0);  //JerryYang 20190313 : 2D sorting

        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN23_4"),             &IniConfig.sN23LotInfoPath,         ECText,     "2DSotingFunction",     "sN23LotInfoPath",                  bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_4"),             &IniConfig.bN23UseLotInfoFile,      ECBool,     "2DSotingFunction",     "bN23UseLotInfoFile",               bShow, bEnable, bReadFromFile, 0);  //Steven 20240829 : Lot info從檔案讀取
        if(IniConfig.bSPILFunction)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_5"),         &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bShow, bEnable, bReadFromFile, 1);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_5"),         &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bShow, bEnable, bReadFromFile, 0);  //Steven 20240830 : Summary資料夾不要有年月
    }
    else if(CosFunction.bMakeWhite2DIDList==true)                               //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN23")->TabVisible=true;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2UserName"),      &IniConfig.cN23FtpUserName,         ECText,     "2DSotingFunction",     "cN23FtpUserName",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2Pwd"),           &IniConfig.cN23FtpPassword,         ECPassword, "2DSotingFunction",     "cN23FtpPassword",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2FTPHost"),       &IniConfig.cN23FtpHost,             ECText,     "2DSotingFunction",     "cN23FtpHost",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_2FTPPath"),       &IniConfig.cN23FtpDownloadPath,     ECText,     "2DSotingFunction",     "cN23FtpDownloadPath",              bShow, bEnable, bReadFromFile, "\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN23_3NetDrivePath"),  &IniConfig.sN23DownloadDrivePath,   ECText,     "2DSotingFunction",     "sN23DownloadDrivePath",            bNoShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN23_1_2DSorting"),    &IniConfig.iN23DownloadMethod,      ECInteger,  "2DSotingFunction",     "iN23DownloadMethod",               bShow, bDisable, bReadFromFile, 0);  //JerryYang 20190313 : 2D sorting
        IniConfig.iN23DownloadMethod=0;
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN23_4"),             &IniConfig.sN23LotInfoPath,         ECText,     "2DSotingFunction",     "sN23LotInfoPath",                  bNoShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_4"),             &IniConfig.bN23UseLotInfoFile,      ECBool,     "2DSotingFunction",     "bN23UseLotInfoFile",               bNoShow, bEnable, bReadFromFile, 0);  //Steven 20240829 : Lot info從檔案讀取
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN23_5"),             &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bNoShow, bEnable, bReadFromFile, 0);  //Steven 20240830 : Summary資料夾不要有年月
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN23")->TabVisible=false;
    }

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20200409 : JSCC RTM功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN24")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chk24"),               &IniConfig.bN24_EnableRTM,           ECBool,     "RTM Function", "bN24_EnableRTM",                           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN24"),           &IniConfig.iN24_RTMPort,             ECPort,     "Murata Function",      "iN24_RTMPort",                     bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN24")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chk24"),               &IniConfig.bN24_EnableRTM,           ECBool,     "RTM Function", "bN24_EnableRTM",                           bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN23_4_URL"),          &IniConfig.sN23_4_URL,              ECText,     "2DID Search Function", "sN23_4_URL",                       bShow, bEnable, bReadFromFile, "D:\\RMS\\");  //JerryYang 20250521 : add
    elConfig->Add(EL<TLabeledEdit>("TfConfiguration", "lbledtN23_5_UploadPath"),   &IniConfig.sN23_5_UploadPath,       ECText,     "2DID White list",      "sN23_5_UploadPath",                bShow, bEnable, bReadFromFile, "D:\\RMS\\");

    if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                        //Steven 20210413 : 南茂的自動Start功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN25")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_1"),         &IniConfig.bN25_1_EnableStartControl,   ECBool,     "ChipMos Function", "bN25_1_EnableStartControl",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_2"),         &IniConfig.bN25_2_EnableUploadLog,      ECBool,     "ChipMos Function", "bN25_2_EnableUploadLog",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_1_Name"),        &IniConfig.sN25_1_FTPUserName,      ECText,     "ChipMos Function",     "sN25_1_FTPUserName",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_1_Password"),    &IniConfig.sN25_1_FTPPassword,      ECPassword, "ChipMos Function",     "sN25_1_FTPPassword",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_1_Host"),        &IniConfig.sN25_1_FTPHost,          ECText,     "ChipMos Function",     "sN25_1_FTPHost",                   bShow, bEnable, bReadFromFile, "10.20.50.3");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_1_Path"),        &IniConfig.sN25_1_FTPPath,          ECText,     "ChipMos Function",     "sN25_1_FTPPath",                   bShow, bEnable, bReadFromFile, "/summary/Handler/TesterStatus/");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_2_Name"),        &IniConfig.sN25_2_FTPUserName,      ECText,     "ChipMos Function",     "sN25_2_FTPUserName",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_2_Password"),    &IniConfig.sN25_2_FTPPassword,      ECPassword, "ChipMos Function",     "sN25_2_FTPPassword",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_2_Host"),        &IniConfig.sN25_2_FTPHost,          ECText,     "ChipMos Function",     "sN25_2_FTPHost",                   bShow, bEnable, bReadFromFile, "10.20.50.3");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_2_Path"),        &IniConfig.sN25_2_FTPPath,          ECText,     "ChipMos Function",     "sN25_2_FTPPath",                   bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_2_Interval"),    &IniConfig.iN25_2_UploadInterval,   ECInteger,  "ChipMos Function",     "iN25_2_UploadInterval",            bShow, bEnable, bReadFromFile, 10,    false, 1,       100);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_3"),             &IniConfig.bN25_3_EnableULJamLog,   ECBool,     "ChipMos Function",     "bN25_3_EnableULJamLog",            bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_3_LogJamPath"),  &IniConfig.sN25_3_JamLogFTPPath,    ECText,     "ChipMos Function",     "sN25_3_JamLogFTPPath",             bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_4"),             &IniConfig.bN25_4_EnableUpload,     ECBool,     "ChipMos Function",     "bN25_4_EnableUpload",              bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_4_UploadPath"),  &IniConfig.sN25_4_UploadPath,       ECText,     "ChipMos Function",     "sN25_4_UploadPath",                bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_5"),             &IniConfig.bN25_5_EnableUpload,     ECBool,     "ChipMos Function",     "bN25_5_EnableUpload",              bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN25_5_UploadPath"),  &IniConfig.sN25_5_UploadPath,       ECText,     "ChipMos Function",     "sN25_5_UploadPath",                bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN25")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_1"),     &IniConfig.bN25_1_EnableStartControl,       ECBool,     "ChipMos Function", "bN25_1_EnableStartControl",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN25_2"),     &IniConfig.bN25_2_EnableUploadLog,          ECBool,     "ChipMos Function", "bN25_2_EnableUploadLog",               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseJamRawData)                                              //Sam 20220922 : 改用 elConfig 方式 //Sam 20210224 : Auto Upload FTP JAMRawData 功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN26")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN26_1"),     &IniConfig.bN26_UseJamRawDataUpdataToFTP,   ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataUpdataToFTP",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN26_2"),     &IniConfig.bN26_UseJamRawDataRecord,        ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataRecord",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN26_1"),     &IniConfig.sN26_FTPUserName,                ECText,     "JamRawDataUpdataToFTP","sN26_FTPUserName",                 bShow, bEnable, bReadFromFile,  "SGPHANDLER");          //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN26_2"),     &IniConfig.sN26_FTPPassword,                ECPassword, "JamRawDataUpdataToFTP","sN26_FTPPassword",                 bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN26_3"),     &IniConfig.sN26_FTPHost,                    ECText,     "JamRawDataUpdataToFTP","sN26_FTPHost",                     bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN26_4"),     &IniConfig.sN26_FTPUplaodPath,              ECText,     "JamRawDataUpdataToFTP","sN26_FTPUplaodPath",               bShow, bEnable, bReadFromFile,  "\\HT_JAM\\HT-9046HA-8H\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN26")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN26_1"),     &IniConfig.bN26_UseJamRawDataUpdataToFTP,   ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataUpdataToFTP",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN26_2"),     &IniConfig.bN26_UseJamRawDataRecord,        ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataRecord",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseAlarmLogXml)                                             //Sam 20220922 : 改用 elConfig 方式 Mark //Sam 20210621 : 新增 Event Log xml  Updata
    {
        EL<TTabSheet>("TfConfiguration", "tsN27")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN27_1"),      &IniConfig.bN27_UseAlarmLogXmlUpdataToFTP,  ECBool,     "bUseAlarmLogXml", "bN27_UseAlarmLogXmlUpdataToFTP",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN27_1"),      &IniConfig.sN27_FTPUserName,                ECText,     "bUseAlarmLogXml", "sN27_FTPUserName",                      bShow, bEnable, bReadFromFile,  "SGPHANDLER");          //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN27_2"),      &IniConfig.sN27_FTPPassword,                ECPassword, "bUseAlarmLogXml", "sN27_FTPPassword",                      bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN27_3"),      &IniConfig.sN27_FTPHost,                    ECText,     "bUseAlarmLogXml", "sN27_FTPHost",                          bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN27_4"),      &IniConfig.sN27_FTPUplaodPath,              ECText,     "bUseAlarmLogXml", "sN27_FTPUplaodPath",                    bShow, bEnable, bReadFromFile,  "\\HT_JAM\\HT-9046HA-8H\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN27_6"),      &IniConfig.sN27_TesterID,                   ECText,     "bUseAlarmLogXml", "sN27_TesterID",                         bShow, bEnable, bReadFromFile,  "");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN27")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN27_1"),      &IniConfig.bN27_UseAlarmLogXmlUpdataToFTP,  ECBool,     "bUseAlarmLogXml", "bN27_UseAlarmLogXmlUpdataToFTP",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_SCK)                                                   //Steven 20210608 : JSCK OEE Function.
    {
        EL<TTabSheet>("TfConfiguration", "tsN28")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN28"),       &IniConfig.bN28_SCK_OEE,                    ECBool,     "JSCK Function",        "bN26_SCK_OEE",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN28_Path"),  &IniConfig.sN28_Path,                       ECText,     "JSCK Function",        "sN26_Path",                        bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\TestLog\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN28_IP"),    &IniConfig.sN28_IP,                         ECText,     "JSCK Function",        "sN26_IP",                          bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN28")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN28"),       &IniConfig.bN28_SCK_OEE,                    ECBool,     "JSCK Function",        "bN26_SCK_OEE",                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_GM_TEST)                                               //Steven 20220311 : GM Test工作檔比對功能
    {
        EL<TTabSheet>("TfConfiguration", "tsN29")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN29"),       &IniConfig.bN29_ParameterCheckForGMTest,    ECBool,     "N29", "bN29_ParameterCheck",                               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtN29"),       &IniConfig.sN29_FilePath,                   ECText,     "N29", "sN29_FilePath",                                     bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN29")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN29"),       &IniConfig.bN29_ParameterCheckForGMTest,    ECBool,     "N29", "bN29_ParameterCheck",                               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bRecordGroundESDByTestIC)                                    //Sam 20220922 : 改用 elConfig 方式   //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    {
        EL<TTabSheet>("TfConfiguration", "tsN30")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN30_1"),  &IniConfig.bN30_UseGroundESDUpdataToFTP,        ECBool,     "bRecordGroundESDByTestIC", "bN30_UseGroundESDUpdataToFTP", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN30_1"),  &IniConfig.sN30_FTPUserName,                    ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUserName",             bShow, bEnable, bReadFromFile,  "SGPHANDLER");      //Sam 20250122 : 客戶要求預設
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN30_2"),  &IniConfig.sN30_FTPPassword,                    ECPassword, "bRecordGroundESDByTestIC", "sN30_FTPPassword",             bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN30_3"),  &IniConfig.sN30_FTPHost,                        ECText,     "bRecordGroundESDByTestIC", "sN30_FTPHost",                 bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN30_4"),  &IniConfig.sN30_FTPUploadPath,                  ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUplaodPath",           bShow, bEnable, bReadFromFile,  "\\Handler\\GroundESDLog\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN30_5"),  &IniConfig.sN30_FTPUploadPath2,                 ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUplaodPath2",          bShow, bEnable, bReadFromFile,  "\\Handler\\GroundESDLog2\\");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN30")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN30_1"),  &IniConfig.bN30_UseGroundESDUpdataToFTP,        ECBool,     "bRecordGroundESDByTestIC", "bN30_UseGroundESDUpdataToFTP", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bAutoTmpeOfsByFTP)                                           //Sam 20220922 : 改用 elConfig 方式   //Sam 20220406 : 溫度自動補償功能 By FTP
    {
        EL<TTabSheet>("TfConfiguration", "tsN31")->TabVisible=true;
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "cbN31_1"),  &IniConfig.iN31_UseAutoTempOfsByFTP,            ECInteger,  "bAutoTmpeOfsByFTP", "bN31_UseAutoTempOfsByFTP",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_1"),  &IniConfig.sN31_FTPUserName,                    ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPUserName",                    bShow, bEnable, bReadFromFile, "SGPHANDLER");           //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_2"),  &IniConfig.sN31_FTPPassword,                    ECPassword, "bAutoTmpeOfsByFTP", "sN31_FTPPassword",                    bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_3"),  &IniConfig.sN31_FTPHost,                        ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPHost",                        bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_4"),  &IniConfig.sN31_FTPDownloadPath,                ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPDownloadPath",                bShow, bEnable, bReadFromFile, "\\Handler\\AutoTempOfs\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_5"),  &IniConfig.iN31_ContactCnt,                     ECInteger,  "bAutoTmpeOfsByFTP", "iN31_ContactCnt",                     bShow, bEnable, bReadFromFile, 10,    false, 1,       100);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_MinOffset"),  &IniConfig.dN31_MinOffset,ECDouble, "bAutoTmpeOfsByFTP",            "dN31_MinOffset",       bShow, bEnable, bReadFromFile, -30.0,   false, -30.0,   30.0);                               //Jimmychiu 20241226 : add N31 temp offset limit
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN31_MaxOffset"),  &IniConfig.dN31_MaxOffset,ECDouble, "bAutoTmpeOfsByFTP",            "dN31_MaxOffset",       bShow, bEnable, bReadFromFile, 30.0,    false, -30.0,   30.0);
        if(CUSTOMER_CODE==CC_UTAC_TW)                                           //Sam 20230815 : 聯測改為 Interval time
            EL<TLabel>("TfConfiguration", "labN31_5")->Caption="Intetval Time";
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN31")->TabVisible=false;
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "cbN31_1"),  &IniConfig.iN31_UseAutoTempOfsByFTP,            ECInteger,  "bAutoTmpeOfsByFTP", "bN31_UseAutoTempOfsByFTP",         bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bDownloadUpdateAutomatically)                                //Sam 20220922 : 改用 elConfig 方式 //Sam 20220824 : FTP 自動下載安裝更新包
    {
        EL<TTabSheet>("TfConfiguration", "tsN32")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_1"),  &IniConfig.bN32_DownloadUpdatesAutomatically,   ECBool,     "bDownloadUpdateAutomatically", "bN32_DownloadUpdatesAutomatically",bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_2"),  &IniConfig.bN32_CheckForUpdatesOnceDay,         ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckForUpdatesOnceDay;",     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_3"),  &IniConfig.bN32_CheckAtInitailStart,            ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtInitailStart;",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_4"),  &IniConfig.bN32_CheckAtTrayFeedFinish,          ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtTrayFeedFinish;",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_1"),  &IniConfig.sN32_FTPUserName,                    ECText,     "bDownloadUpdateAutomatically", "sN32_FTPUserName",                 bShow, bEnable, bReadFromFile, "SGPHANDLER");           //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_2"),  &IniConfig.sN32_FTPPassword,                    ECPassword, "bDownloadUpdateAutomatically", "sN32_FTPPassword",                 bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_3"),  &IniConfig.sN32_FTPHost,                        ECText,     "bDownloadUpdateAutomatically", "sN32_FTPHost",                     bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_4"),  &IniConfig.sN32_FTPDownloadPath,                ECText,     "bDownloadUpdateAutomatically", "sN32_FTPDownloadPath",             bShow, bEnable, bReadFromFile, "\\Handler\\Installer\\");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_5"),  &IniConfig.sN32_NetDownloadPath,                ECText,     "bDownloadUpdateAutomatically", "sN32_NetDownloadPath",             bShow, bEnable, bReadFromFile, "D:\\RMS\\");            //Steven 20221216 : 使用網路硬碟下載安裝包
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN32_6"),  &IniConfig.sN32_FTPDownloadPath2,               ECText,     "bDownloadUpdateAutomatically", "sN32_FTPDownloadPath2",            bShow, bEnable, bReadFromFile, "\\Handler\\InstallerForAll\\"); //Sam 20230328 : 自動更新增加版本檢查
        elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN32"),    &IniConfig.iN32_DownloadMode,                   ECInteger,  "bDownloadUpdateAutomatically", "iN32_DownloadMode",                bShow, bEnable, bReadFromFile, 0);                      //Steven 20221216 : 使用網路硬碟下載安裝包
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN32")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_1"),  &IniConfig.bN32_DownloadUpdatesAutomatically,   ECBool,     "bDownloadUpdateAutomatically", "bN32_DownloadUpdatesAutomatically",bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_2"),  &IniConfig.bN32_CheckForUpdatesOnceDay,         ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckForUpdatesOnceDay",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_3"),  &IniConfig.bN32_CheckAtInitailStart,            ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtInitailStart",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN32_4"),  &IniConfig.bN32_CheckAtTrayFeedFinish,          ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtTrayFeedFinish",       bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN35_1"),      &IniConfig.bN35_Ground_ESD_Upload,              ECBool,     "Rround_ESD_Upload",       "bN35_Ground_ESD_Upload",    bShow, bEnable, bReadFromFile, 0);                          //Sam 20250609 : Record Ground and ESD at intervals and upload
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN35_1"),      &IniConfig.sN35_FTPUserName,                    ECText,     "Rround_ESD_Upload",       "sN35_FTPUserName",          bShow, bEnable, bReadFromFile,  "SGPHANDLER");
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN35_2"),      &IniConfig.sN35_FTPPassword,                    ECText,     "Rround_ESD_Upload",       "sN35_FTPPassword",          bShow, bEnable, bReadFromFile,  "CHROMA");
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN35_3"),      &IniConfig.sN35_FTPHost,                        ECText,     "Rround_ESD_Upload",       "sN35_FTPHost",              bShow, bEnable, bReadFromFile,  "192.168.119.243");
    elConfig->Add(EL<TEdit>("TfConfiguration", "edN35_4"),      &IniConfig.sN35_FTPUploadPath,                  ECText,     "Rround_ESD_Upload",       "sN35_FTPUploadPath",        bShow, bEnable, bReadFromFile,  "\\Handler\\GroundandESD");
    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN35_1"),      &IniConfig.iN35_Interval,                       ECInteger,  "Rround_ESD_Upload",       "iN35_Interval",             bShow, bEnable, bReadFromFile, 0);          //Sam 20250609 : Record Ground and ESD at intervals and upload

    if(CUSTOMER_CODE==CC_LEADYO)                                                //KenHsieh 20230502 : 利揚要求上傳OCR + BIN Log上傳至Host
    {
        EL<TTabSheet>("TfConfiguration", "tsN33")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN33"),    &IniConfig.bN33_UpLoadOCRBinLogByNet,           ECBool,     "LEADYO Function",   "bN33_UpLoadOCRBinLogByNet",     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN33"),    &IniConfig.asN33_UploadLogPath,                 ECText,     "LEADYO Function",   "asN33_UploadLogPath",           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN33_1"),  &IniConfig.bN33_1_NetChangeFileAndData,         ECBool,     "LEADYO Function",   "bN33_1_NetChangeFileAndData",   bShow, bEnable, bReadFromFile, 0);             //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN33")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN33"),    &IniConfig.bN33_UpLoadOCRBinLogByNet,           ECBool,     "LEADYO Function",   "bN33_UpLoadOCRBinLogByNet",     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN33_1"),  &IniConfig.bN33_1_NetChangeFileAndData,         ECBool,     "LEADYO Function",   "bN33_1_NetChangeFileAndData",   bNoShow, bDisable, bFixedValue, 0);            //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    }

    if(CUSTOMER_CODE==CC_CYUEAN)                                                //Jimmychiu 20250324 : CC_CYUEAN OEE report
    {
        EL<TTabSheet>("TfConfiguration", "tsN34")->TabVisible=true;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN34"),    &IniConfig.bN34_GenerateOEEAlarmRpt,            ECBool,     "N34 Function",   "bN34_GenerateOEEAlarmRpt",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN34"),    &IniConfig.sN34_OEEAlarmRptPath,                ECText,     "N34 Function",   "sN34_OEEAlarmRptPath",             bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\Product_Loader\\OEEAlarmRpt");
    }
    else
    {
        EL<TTabSheet>("TfConfiguration", "tsN34")->TabVisible=false;
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN34"),    &IniConfig.bN34_GenerateOEEAlarmRpt,            ECBool,     "N34 Function",   "bN34_GenerateOEEAlarmRpt",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN34"),    &IniConfig.sN34_OEEAlarmRptPath,                ECText,     "N34 Function",   "sN34_OEEAlarmRptPath",             bNoShow, bDisable, bFixedValue, "D:\\HT9045_Log\\OEEAlarmRpt");
    }
    //==> Eastsun 20260520 整合
    if(USE_BU5_Function==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN40_1"),    &IniConfig.bN40_1_HandlerDataBackUpUseFunction,       ECBool,   "FTPUpLoad",         "bN40_1_HandlerDataBackUpUseFunction",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbN41_1"),    &IniConfig.bN41_1_HandlerDataBackUpToDiskUseFunction, ECBool,   "FTPUpLoad",         "bN41_1_HandlerDataBackUpToDiskUseFunction",    bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edN40_1"),    &IniConfig.sN40_FTPUserName,                          ECText,   "LEADYO Function",   "sN40FtpUserName",                              bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN40_2"),    &IniConfig.sN40_FTPPassword,                          ECText,   "LEADYO Function",   "sN40_FTPPassword",                             bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN40_3"),    &IniConfig.sN40_FTPHost,                              ECText,   "LEADYO Function",   "sN40_FTPHost",                                 bShow, bEnable, bReadFromFile, "");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edN40_4"),    &IniConfig.sN40_FTPUploadPath,                        ECText,   "LEADYO Function",   "sN40_FTPUploadPath",                           bShow, bEnable, bReadFromFile, "");

        elConfig->Add(EL<TEdit>("TfConfiguration", "edN41_1"),    &IniConfig.sN41_DiskUploadPath,                       ECText,   "LEADYO Function",   "sN41_DiskUploadPath",                          bShow, bEnable, bReadFromFile, "");
    }
    //<== Eastsun 20260520 整合

}

// golden cConfiguration.cpp:4022  TfConfiguration::InitConfigEdtList_ItemO()
static void IC_InitConfigEdtList_ItemO()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;
    bool bDefaultO10=false;

    bDefaultO10=IniConfig.bSPILFunction;                    //JerryYang 20251013 : fix O10預設值錯誤

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO01"),        &IniConfig.bO01_ResetNeedClearAndCheckHP,       ECBool,     "Count", "bResetNeedClearAndCheckHotPlate",                 bShow, bEnable, bReadFromFile, 0);
    if(IniConfig.bSPILFunction || IniConfig.bResetClearArmIC==true)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO02"),    &IniConfig.bO02ResetNotClearPlate,              ECBool,     "Count", "bO02ResetNotClearPlate",                          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO05"),    &IniConfig.bO05ResetNeedRemoveAllTray,          ECBool,     "Count", "bO05ResetNeedRemoveAllTray",                      bShow, bEnable, bReadFromFile, 0);      //Eliot 2010_03_05
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO02"),    &IniConfig.bO02ResetNotClearPlate,              ECBool,     "Count", "bO02ResetNotClearPlate",                          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO05"),    &IniConfig.bO05ResetNeedRemoveAllTray,          ECBool,     "Count", "bO05ResetNeedRemoveAllTray",                      bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bEventLogAutoSaveFunction)                                     //Steven 20110221 Start : EventLogAutoSave
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO06"),                &IniConfig.bO06_EventLogAutoSave,           ECBool, "Event Log", "EnableAutoSaveEventLog",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06UseNetDrive"),    &IniConfig.bAlarmStatistAutoSaveNetDrive,   ECBool, "Event Log", "bAlarmStatistAutoSaveNetDrive",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06TimePeriod"),     &IniConfig.bO06SaveLogTimePeriod,           ECBool, "Event Log", "EnanleTimePeriodSaveLog",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06AlarmHistroy"),   &IniConfig.bEnableAlarmHistroyAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmHistroy",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06AlarmStatist"),   &IniConfig.bEnableAlarmStatistAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmStatist",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06Production"),     &IniConfig.bEnableProductionAutoSave,       ECBool, "Event Log", "EnableAutoSaveProductiont",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edO06_FilePath"),       &IniConfig.asEventLogAutoSavePath,          ECText, "Event Log", "AutoSaveEventLogPath",                    bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO06AlarmHistroy"),   &IniConfig.asAlarmHistroyAutoSavePath,      ECText, "Event Log", "AutoSaveAlarmHistroyPath",                bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO06AlarmStatist"),   &IniConfig.asAlarmStatistAutoSavePath,      ECText, "Event Log", "AutoSaveAlarmStatistPath",                bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO06Production"),     &IniConfig.asProductionAutoSavePath,        ECText, "Event Log", "AutoSaveProductionPath",                  bShow, bEnable, bReadFromFile, "D:\\RMS");          //Steven 20140816 : Production Data
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO06_Remote"),        &IniConfig.asAlarmRemoteDirectory,          ECText, "Event Log", "asAlarmLocalDirectory",                   bShow, bEnable, bReadFromFile, "Z:");               //Steven 20140306 : 使用網路硬碟
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO06_Local"),         &IniConfig.asAlarmLocalDirectory,           ECText, "Event Log", "asAlarmRemoteDirectory",                  bShow, bEnable, bReadFromFile, "\\\\NET_DRVE");     //Steven 20140306 : 使用網路硬碟
        elConfig->Add(EL<TComboBox>("TfConfiguration", "cbO06TimePeriod"),      &IniConfig.iO06SaveLogTimePeriod,           ECInteger, "Event Log", "TimePeriodSaveLog",                    bShow, bEnable, bReadFromFile, 0); //JerryYang 20151026 : ProductionData週期時間(0->10mins,1->30mins)
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO06"),                &IniConfig.bO06_EventLogAutoSave,           ECBool, "Event Log", "EnableAutoSaveEventLog",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06UseNetDrive"),    &IniConfig.bAlarmStatistAutoSaveNetDrive,   ECBool, "Event Log", "bAlarmStatistAutoSaveNetDrive",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06TimePeriod"),     &IniConfig.bO06SaveLogTimePeriod,           ECBool, "Event Log", "EnanleTimePeriodSaveLog",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06AlarmHistroy"),   &IniConfig.bEnableAlarmHistroyAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmHistroy",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06AlarmStatist"),   &IniConfig.bEnableAlarmStatistAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmStatist",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO06Production"),     &IniConfig.bEnableProductionAutoSave,       ECBool, "Event Log", "EnableAutoSaveProductiont",               bNoShow, bDisable, bFixedValue, 0);
    }

//    if(IniConfig.bSPILFunction==true)     //JerryYang 20250305 : mark掉, 暫時先用Event log Saver                                           //Steven 20240604 : SPIL格式的event log
//    {
//        elConfig->Add(cbO10_EventLogSaver,  &IniConfig.bO10UseEventLogSaver,            ECBool, "Event Log", "bO10UseEventLogSaver",                    bNoShow, bDisable, bFixedValue, 0);      //Steven 20140902 : 使用外掛小程式
//    }
//    else
    {
        if(IniConfig.bEventLogAutoSaveFunction ||
           CosFunction.bUseMDB)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO10_EventLogSaver"),  &IniConfig.bO10UseEventLogSaver,        ECBool, "Event Log", "bO10UseEventLogSaver",                    bShow, bEnable, bReadFromFile, bDefaultO10?"1":"0");      //JerryYang 20251013 : fix O10預設值錯誤  //Steven 20140902 : 使用外掛小程式
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO07"),                    &IniConfig.bFTContinueON,           ECBool,     "Count", "bFTContinueON",                                   bShow, bEnable, bReadFromFile, 0);      //kevin FT MODE 不能關連續FAIL

    int iMaxFTContinue=100;
    if(CosFunction.bOEEFunction==true)                                          //Jimmychiu 20231026 : no limited max consecutive faulure count in Greatek
    {
        iMaxFTContinue=99999;
    }

    elConfig->Add(EL<TEdit>("TfConfiguration", "edO07"),                &IniConfig.iFTMAXValue,                 ECInteger,  "Count", "iFTMAXValue",                                     bShow, bEnable, bReadFromFile, 5,  false,  2,      iMaxFTContinue);            //kevin 20121008 FT MODE 連續FAIL 最大數量
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO09"),                &IniConfig.bInitialStartNeedAsk,        ECBool,     "Count", "bInitialStartNeedAsk",                            bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO11"),                &IniConfig.bRecordJamRateByTime,        ECBool,     "Count", "Record Jam Rate By Time",                         bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtO11"),               &IniConfig.iRecordJamRateIntervalTime,  ECInteger,  "Count", "Record Jam Rate Interval Time",                   bShow, bEnable, bReadFromFile, 1,  false,  1,      999999);     // 2015.11.11 , Joye , Add Jam Rate Record

    if(CosFunction.bUseHeadContactCount==true && CUSTOMER_CODE!=CC_SCC)         //Ifor 20191128 : Fix 銦片 Life Time 功能無顯示
    {
        if((CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bHiSiliconFunction==true) || //Ifor 20180919 (Steven) : KYEC 要求海思版第一個Life Time Count [O12]功能強制開啟並反灰
           IniConfig.bAMDFunction)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO12_LifeTimeCount"),  &IniConfig.bLifeTimeCount[0],   ECBool,     "O_Count", "bLifeTimeCount 1",                              bShow, bDisable, bFixedValue, 1);        //wei 20160509 Life Time Count
        else if(IniConfig.bVTESTFunction)                                       //AI(ht9045-config) 20260508 (RogerYang) : VTEST O12跟隨Recipe(configByRecipe.ini)
            elConfig_byRecipe->Add(EL<TCheckBox>("TfConfiguration", "cbO12_LifeTimeCount"), &IniConfig.bLifeTimeCount[0], ECBool, "O_Count", "bLifeTimeCount 1",                            bShow, bEnable, bReadFromFile, 0);
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO12_LifeTimeCount"),  &IniConfig.bLifeTimeCount[0],   ECBool,     "O_Count", "bLifeTimeCount 1",                              bShow, bEnable, bReadFromFile, 0);

        if(IniConfig.bVTESTFunction)                                            //AI(ht9045-config) 20260508 (RogerYang) : VTEST只用O12，隱藏O13/O14及HeadCondition2/3
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO13_LifeTimeCount"),  &IniConfig.bLifeTimeCount[1],   ECBool,     "O_Count", "bLifeTimeCount 2",                              bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO14_LifeTimeCount"),  &IniConfig.bLifeTimeCount[2],   ECBool,     "O_Count", "bLifeTimeCount 3",                              bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition1"),     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bShow, bEnable, bReadFromFile, "HeadCondition1");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition2"),     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bNoShow, bDisable, bReadFromFile, "HeadCondition2");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition3"),     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bNoShow, bDisable, bReadFromFile, "HeadCondition3");
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO13_LifeTimeCount"),      &IniConfig.bLifeTimeCount[1],   ECBool,     "O_Count", "bLifeTimeCount 2",                              bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO14_LifeTimeCount"),      &IniConfig.bLifeTimeCount[2],   ECBool,     "O_Count", "bLifeTimeCount 3",                              bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition1"),     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bShow, bEnable, bReadFromFile, "HeadCondition1");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition2"),     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bShow, bEnable, bReadFromFile, "HeadCondition2");
            elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition3"),     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bShow, bEnable, bReadFromFile, "HeadCondition3");
        }
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO12_LifeTimeCount"),  &IniConfig.bLifeTimeCount[0],       ECBool,     "O_Count", "bLifeTimeCount 1",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO13_LifeTimeCount"),  &IniConfig.bLifeTimeCount[1],       ECBool,     "O_Count", "bLifeTimeCount 2",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO14_LifeTimeCount"),  &IniConfig.bLifeTimeCount[2],       ECBool,     "O_Count", "bLifeTimeCount 3",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition1"),     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bNoShow, bDisable, bFixedValue, "HeadCondition1");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition2"),     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bNoShow, bDisable, bFixedValue, "HeadCondition2");
        elConfig->Add(EL<TEdit>("TfConfiguration", "edHeadCondition3"),     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bNoShow, bDisable, bFixedValue, "HeadCondition3");
    }

    elConfig->Add(EL<TComboBox>("TfConfiguration", "cbbO15_1"),     &IniConfig.iO15_SaveFilePeriod,                 ECInteger,  "Event Log", "iO15_SaveFilePeriod",                         bShow, bEnable, bReadFromFile,  6);     //Steven 20170829 (wei) : Event Log檔案存檔設定
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO15_2"),     &IniConfig.bO15_EventLogFileNameWithMachineID,  ECBool,     "Event Log", "bO15_EventLogFileNameWithMachineID",          bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO15_3"),     &IniConfig.bO15_EventLogSaveSameFolder,         ECBool,     "Event Log", "bO15_EventLogSaveSameFolder",                 bShow, bEnable, bReadFromFile, 0);       //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾

    if(CosFunction.bConAlarmNeedKeyInPassword)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO16"),   &IniConfig.bO16ConAlarmNeedKeyInPasswordCT,     ECBool,     "Count", "bO16ConAlarmNeedKeyInPasswordCT",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO16"),   &IniConfig.iO16ConAlarmNeedKeyInPasswordCT,     ECInteger,  "Count", "Con Alarm Need KeyIn Password CT",                bShow, bEnable, bReadFromFile,  3,     false,  1,      100);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO16"),   &IniConfig.bO16ConAlarmNeedKeyInPasswordCT,     ECBool,     "Count", "bO16ConAlarmNeedKeyInPasswordCT",                 bNoShow, bDisable, bFixedValue, 0);
        EL<TEdit>("TfConfiguration", "edtO16")->Visible=false;
    }

    if(CosFunction.bConAlarmInTimeLevelUp)                                      //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO17"),       &IniConfig.bO17EnableLevelUpWhenContiAlarm, ECBool,     "Count", "bO17EnableLevelUpWhenContiAlarm",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO17_Count"), &IniConfig.iO17LevelUpWhenContiAlarmCount,  ECInteger,  "Count", "iO17LevelUpWhenContiAlarmCount",                  bShow, bEnable, bReadFromFile,  3,     false,  1,      100);            //Steven 20210127 : 連續alarm次數
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO17_Time"),  &IniConfig.iO17LevelUpWhenContiAlarmTime,   ECInteger,  "Count", "iO17LevelUpWhenContiAlarmTime",                   bShow, bEnable, bReadFromFile,  30,    false,  1,      100000);         //Steven 20210127 : 連續alarm時間
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO17"),       &IniConfig.bO17EnableLevelUpWhenContiAlarm, ECBool,     "Count", "bO17EnableLevelUpWhenContiAlarm",                 bNoShow, bDisable, bFixedValue, 0);
        EL<TEdit>("TfConfiguration", "edtO17_Count")->Visible=false;
        EL<TEdit>("TfConfiguration", "edtO17_Time")->Visible=false;
        EL<TLabel>("TfConfiguration", "lblO17")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO18"),        &IniConfig.bO18SafeDoorOnOffDurationDetect, ECBool,     "Count", "bO18SafeDoorOnOffDurationDetect",                 bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtO18"),       &IniConfig.iO18SafeDoorOnOffDurationHour,   ECInteger,  "Count", "iO18SafeDoorOnOffDurationHour",                   bShow, bEnable, bReadFromFile,  168,     false,  1,      168);           //JerryYang 20210112 : 安全門檢查機制,長時間未開啟要跳出alarm
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO18"),        &IniConfig.bO18SafeDoorOnOffDurationDetect, ECBool,     "Count", "bO18SafeDoorOnOffDurationDetect",                 bNoShow, bDisable, bFixedValue, 0);
        EL<TEdit>("TfConfiguration", "edtO18")->Visible=false;
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO19_1"),      &IniConfig.bO19_AutoRecordReportByEveryDay,     ECBool,     "Event Log", "bO19_AutoRecordReportByEveryDay",             !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO19_2"),      &IniConfig.bO19_AutoRecordReportByEveryWeek,    ECBool,     "Event Log", "bO19_AutoRecordReportByEveryWeek",            bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TComboBox>("TfConfiguration", "coO19_3"),      &IniConfig.iO19_WeekPeriod,                     ECInteger,  "Event Log", "iO19_WeekPeriod",                             bShow, bEnable, bReadFromFile, 0);      //Sam 20210107 : Summary Report fuction
    elConfig->Add(EL<TComboBox>("TfConfiguration", "coO19_5"),      &IniConfig.iO19_MonthPeriod,                    ECInteger,  "Event Log", "iO19_MonthPeriod",                            !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week //Sam 20210107 : Summary Report fuction
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO19_4"),      &IniConfig.bO19_AutoRecordReportByEveryMonth,   ECBool,     "Event Log", "bO19_AutoRecordReportByEveryMonth",           !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week
    elConfig->Add(EL<TEdit>("TfConfiguration", "edtO19_6"),     &IniConfig.asO19_SavePath,                      ECText,     "Event Log", "asO19_SavePath",                              bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\Product_Loader");       //Sam 20210107 : Summary Report fuction
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO20"),        &IniConfig.bO20InOutArmPickerLifeTimeCount,     ECBool,     "O_Count", "bO20InOutArmPickerLifeTimeCount",               bShow, bEnable, bReadFromFile, 0);      //wei 20160509 Life Time Count

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO20_1"),      &IniConfig.bO20_1ClearLifeTimeWhenInitialStart, ECBool,     "O_Count", "bO20_1ClearLifeTimeWhenInitialStart",           bShow, bEnable, bReadFromFile, 0);      //Steven 20240531 : initial start的時候要清除資料

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO21"),        &IniConfig.bO21FTAfterTrayEndClearFailBinCount, ECBool,     "O_Count", "bO21FTAfterTrayEndClearFailBinCount",           bShow, bEnable, bReadFromFile, 0);      //Frank 20241114 : Add
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbO21_1"),      &IniConfig.bO21_1RTContinueStartClearLoadingFailBin, ECBool, "O_Count", "bO21_1RTContinueStartClearLoadingFailBin",     bShow, bEnable, bReadFromFile, 0);      //AI(ht9045-clearcount-flow) 20260703 (RogerYang) : ATK RT Continue Start clear Loading & Fail Bin

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO22"),       &IniConfig.bO22_ClearSortCntByDoubleClick,      ECBool,     "O_Count", "bO22_ClearSortCntByDoubleClick",                bShow, bEnable, bReadFromFile, 1);      //Steven 20241206 : 點兩下可以清除數量

    if(CUSTOMER_CODE==CC_JCET ||
       CUSTOMER_CODE==CC_SCC)                                                   //Steven 20250428 : add SCC
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO23"),   &IniConfig.bO23_InputLotIDByBarcode,            ECBool,     "O_Count", "bO23_InputLotIDByBarcode",                      bShow, bEnable, bReadFromFile, 1);      //Steven 20241224 : LotID只能用Barcode
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO23"),   &IniConfig.bO23_InputLotIDByBarcode,            ECBool,     "O_Count", "bO23_InputLotIDByBarcode",                      bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_JCET ||                                                //Steven 20250519 : Production Log By Lot
       CUSTOMER_CODE==CC_SCC ||
       CUSTOMER_CODE==CC_JSI_HAOXING)                                           //Frank 20241114 : Add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO24"),   &IniConfig.bO24_ProductionLogByLot,             ECBool,     "O_Count", "bO24_ProductionLogByLot",                       bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO24"),   &IniConfig.bO24_ProductionLogByLot,             ECBool,     "O_Count", "bO24_ProductionLogByLot",                       bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkO25"),        &IniConfig.bO25_RTResetCountBinWhenEnterRT,    ECBool,     "O_Count", "bO25_RTResetCountBinWhenEnterRT",               bShow, bEnable, bReadFromFile, 0);      //AI(ht9045-clearcount-flow) 20260713 (RogerYang) : SCK 進RT時重置Count Bin計數基準

}

// golden cConfiguration.cpp:4202  TfConfiguration::InitConfigEdtList_ItemP()
static void IC_InitConfigEdtList_ItemP()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    EL<TLabel>("TfConfiguration", "lblP04")->Visible=(CUSTOMER_CODE==CC_SIGURD_PeiXing);                         //Sam 20211124 : 矽格北興 P04 功能開啟，必須也要開啟 P24 不然會 Hang up，北興俊堯要求顯示提示
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20230529 add:KYEC 要求 P04功能強制關閉
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP04"),    &IniConfig.bP04ColorIsEmptyUnloader,            ECBool,     "Tray", "bP04ColorIsEmptyUnloader",                         bShow, bDisable, bFixedValue, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP04"),    &IniConfig.bP04ColorIsEmptyUnloader,            ECBool,     "Tray", "bP04ColorIsEmptyUnloader",                         bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP05"),        &IniConfig.bP05_LoaderCylinderPreOn,            ECBool,     "Tray", "P05_LoaderCylinderPreOn",                          bShow, bEnable, bReadFromFile, 0);      //Steven 20150429 : 預先打兩下Loader汽缸
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP05_1"),      &IniConfig.bP05_1_UnloadCylinderPreOn,          ECBool,     "Tray", "bP05_1_UnloadCylinderPreOn",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20240215 : 退tray前預先打兩下分離汽缸
    if(IniConfig.bUseTrayBlockMode)                                             //Ifor 20181016 (Steven) : add 避免無使用Tray Block模式的客戶跑Tray Block模式
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP06"),    &IniConfig.bP06_LoaderUseCarrierTray,           ECBool,     "Tray", "P06_LoaderUseCarrierTray",                         bShow, bEnable, bReadFromFile, 0);      //Frank 20160921 add
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP06"),    &IniConfig.bP06_LoaderUseCarrierTray,           ECBool,     "Tray", "P06_LoaderUseCarrierTray",                         bNoShow, bDisable, bFixedValue, 0);     //wei 20161125 不使用Tray Block Mode不顯示，避免被誤選

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP07"),    &IniConfig.bP07NoTrayaAutoTrayFeed,             ECBool,     "Tray", "bP07NoTrayaAutoTrayFeed",                          bShow, bEnable, bReadFromFile, 0);      //wei 20161125 //wei 20161121 TSMC 新增[P07]Loader No Traya Auto TrayFeed
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP07"),    &IniConfig.bP07NoTrayaAutoTrayFeed,             ECBool,     "Tray", "bP07NoTrayaAutoTrayFeed",                          bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP08"),        &IniConfig.bP08TrayFeedCleanLotID,              ECBool,     "Tray", "bP08TrayFeedCleanLotID",                           bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bCleanOutCanTrayEndBySetupFile)                              //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP09"),    &IniConfig.bP09TrayEndCanSelectTray,            ECBool,     "Tray", "bP09TrayEndCanSelectTray",                         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP09"),    &IniConfig.bP09TrayEndCanSelectTray,            ECBool,     "Tray", "bP09TrayEndCanSelectTray",                         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP10"),    &IniConfig.bP10FixedTrayProposeTheInitialQuestion,  ECBool,     "Tray", "bP10FixedTrayProposeTheInitialQuestion",           bShow, bEnable, bReadFromFile, 0);      //Steven 20100205 Add from HT9080A

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP11"),    &IniConfig.bP11RecordUPH,                       ECBool,     "Count", "RecordUPH",                                       bShow, bEnable, bReadFromFile, 0);      //RogerYang 20260731 : 開放 //Frank 20150515

    if(CosFunction.bKnockerSetBySetupFile==false)                               //Steven 20120510 : 全部改用汽缸的Enable判斷
    {
        if(CosFunction.bHiSiliconFunction)                                      //wei 20160309 強制開啟不能更改  //JerryYang 20191029 STM P14要能選擇
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP13"),    &IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop,  ECBool,     "Tray",     "bEnableAutoTrayEdgePushCylinderLoop",  bShow, bDisable, bFixedValue, 1);
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP14"),    &IniConfig.bP14EnableAutoTrayRecevieDelayCount,     ECBool,     "Tray",     "bEnableAutoTrayRecevieDelayCount",     bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP13"),    &IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop,  ECBool,     "Tray",     "bEnableAutoTrayEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP14"),    &IniConfig.bP14EnableAutoTrayRecevieDelayCount,     ECBool,     "Tray",     "bEnableAutoTrayRecevieDelayCount",     bShow, bEnable, bReadFromFile, 0);
        }

        elConfig->Add(EL<TEdit>("TfConfiguration", "edP13_1"),      &IniConfig.iP13EdgePushCylinderLoopDelay,           ECInteger,  "Tray",     "iP13EdgePushCylinderLoopDelay",        bShow, bEnable, bReadFromFile, LastSet.iEdgePushCylinderLoopDelay, false, 3000, 2);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edP13_2"),      &IniConfig.iP13EdgePushCylinderOnDelay,             ECInteger,  "Tray",     "iP13EdgePushCylinderOnDelay",          bShow, bEnable, bReadFromFile, 10, false,  10000, 1);

        elConfig->Add(EL<TEdit>("TfConfiguration", "edP14_1"),      &IniConfig.iP14AutoTrayRecevieDelayCount,           ECInteger,  "Tray",     "iP14AutoTrayRecevieDelayCount",        bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieDelayCount, false, 10000, 1);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edP14_2"),      &IniConfig.iP14AutoTrayRecevieLoopDelayTime,        ECInteger,  "Tray",     "iP14AutoTrayRecevieLoopDelayTime",     bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieLoopDelayTime, false, 3000, 2);

        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP16"),        &IniConfig.bP16EnableHotplateEdgePushCylinderLoop,  ECBool,     "Hotplate", "bEnableHotplateEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, 0);      //kevin 20191219 開啟

        if(TRAY_VIBRATION==VibrationMotor)
        {
            elConfig->Add(EL<TEdit>("TfConfiguration", "edP16_1"),  &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger,  "Hotplate", "iHotplateEdgePushCylinderLoopDelay",   bShow, bEnable, bReadFromFile, 5, false, 600, 200);
            elConfig->Add(EL<TEdit>("TfConfiguration", "edP16_2"),  &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger,  "Hotplate", "iHotplateEdgePushCylinderOnDelay",     bShow, bEnable, bReadFromFile, 5, false, 50, 2);
        }
        else
        {
            elUdUld->Add(EL<TEdit>("TfConfiguration", "edP16_1"),   &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger,  "Tray",    "iHotplateEdgePushCylinderLoopDelay",    bShow, bEnable, bReadFromFile, 10,                                          true,  2,   600);
            elUdUld->Add(EL<TEdit>("TfConfiguration", "edP16_2"),   &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger,  "Tray",    "iHotplateEdgePushCylinderOnDelay",      bShow, bEnable, bReadFromFile,10,                                          true,  2,   50);
        }
    }

    if(IniConfig.bEnableUnloadTrayFree)                                         //jou 2011-05-27
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP15"),    &IniConfig.bUnloadTrayFree,                     ECBool,     "Tray", "bUnloadTrayFree",                                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP15"),    &IniConfig.bUnloadTrayFree,                     ECBool,     "Tray", "bUnloadTrayFree",                                  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)  //kevin 20190923 add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP17"),    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else if(CUSTOMER_CODE==CC_KYEC_LEE && USE_STM_Function==true)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP17"),    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else if(CUSTOMER_CODE==CC_AMKOR_Japan)                                      //RogerYang 2051110 : 瑞薩FTCT需求，先放進Shuttle後再回來做一格一格檢查
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP17"),    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP17"),        &IniConfig.bP17InArmFullPickFromLoader,     ECBool,     "Loader", "DisableLoaderFull",                              bShow, bDisable, bFixedValue, 1);       //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP18"),        &IniConfig.bP18FailAutoTrayManual,              ECBool,     "Tray", "FailAutoTrayManual",                               bShow, bEnable, bReadFromFile, 0);      //ChungHung 20120329 FailBin AutoTray Manual
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP19"),        &IniConfig.bP19CatchTrayUpThenCheck,            ECBool,     "Tray", "bCatchTrayUpThenCheck",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20120727 : 夾Tray發生異常時要先把Arm上升再檢查一次如果還是沒夾到才Alarm
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP20"), &IniConfig.bP20ManualClearFixTrayDataAfterInitialStart,ECBool,     "Tray", "bManualClearFixTrayDataAfterInitialStart",         bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130305 add for Amkor Initial Strat後不清Tray盤資料需手動清除後才可Run
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP20_1"), &IniConfig.bP20_1_ManualClrAutoTrayWhenInitialStart,ECBool,     "Tray", "bP20_1_ManualClrAutoTrayWhenInitialStart",         bShow, bEnable, bReadFromFile, 0);      //Steven 20210420 : add for TFME InitialStrat後不清Tray盤資料需手動清除後才可Run
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP20_2"), &IniConfig.bP20_2_ManualClrLoadTrayWhenInitialStart,ECBool,     "Tray", "bP20_2_ManualClrLoadTrayWhenInitialStart",         bShow, bEnable, bReadFromFile, 0);      //Steven 20230117 : add for JSCK InitialStrat後需手動清除Loader Tray後才可Run

    if(CUSTOMER_CODE==CC_ASE_KaohSiung  ||                                      //kevin 20130708 高雄日月光強制TRUE
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12 ||                                   //Steven 20131101 : Add ASE-K12
       CosFunction.bHiSiliconFunction ||
       CUSTOMER_CODE==CC_KYEC_LEE ||                                            //Ifor 20180625 (wei) : Fix KYEC P21ㄧ般版本未強制開啟問題
       CUSTOMER_CODE==CC_Murata ||                                              //Steven 20200706 : Add Murata
       IniConfig.bSPILFunction==true)                                           //JerryYang 20250220 : add
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP21"),    &IniConfig.bP21CheckFixTray,                    ECBool,     "Tray", "bCheckFixTray",                                    bShow, bDisable, bFixedValue, 1);       //kevin 20130430 tray feed 需先取出FIX TRAY
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP21"),    &IniConfig.bP21CheckFixTray,                    ECBool,     "Tray", "bCheckFixTray",                                    bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP21_1"),     &IniConfig.bP21_1_CheckFixTray,                 ECBool,     "Tray", "bP21_1_CheckFixTray",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20250321 initial start時偵測fix tray需放入
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP21_2"),     &IniConfig.bP21_2_LoaderTrayFeed,               ECBool,     "Tray", "bP21_2_LoaderTrayFeed",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20250606 : Tray Feed 包含 loader tray

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP22"),        &IniConfig.bP22EnableFirstTrayNeedAlarm,        ECBool,     "Tray", "bEnableFirstTrayNeedAlarm",                        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(EL<TComboBox>("TfConfiguration", "coP22"),        &IniConfig.iEnableFirstTrayNeedAlarmNum,        ECInteger,  "Tray", "iEnableFirstTrayNeedAlarmNum",                     bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(EL<TEdit>("TfConfiguration", "edP23_1"),      &IniConfig.iOCRByNewTrayIntrvalTray,            ECInteger,  "P23", "iOCRByNewTrayIntrvalTray",                          bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edP23_2"),      &IniConfig.iOCRMaxInspDevices,                  ECInteger,  "P23", "iOCRMaxInspDevices",                                bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU || IniConfig.bSPILFunction)               //JerryYang 20241122 : SPIL Disable P24                           //Sam 20230317 : 矽格湖口廠不能用會與 P39 功能起衝突
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24"),    &IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray, ECBool,    "Tray", "bSkipEventNeedRemoveEmptyAndColorTray",        bNoShow, bEnable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24"),    &IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray, ECBool,    "Tray", "bSkipEventNeedRemoveEmptyAndColorTray",        bShow, bEnable, bReadFromFile, 0);      //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查

    if(CosFunction.bManuallyRemoveForceInColor)                                 //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24_2"),  &IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT,   ECBool, "Tray", "bP24SkipEventNeedRemoveColorTrayForIDT",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24_3"),  &IniConfig.bP24TwoTrayMustManuallyRemovedForGenernal,ECBool, "Tray", "bP24TwoTrayMustManuallyRemovedForGenernal",       bShow, bEnable, bReadFromFile, 0);      //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24_2"),  &IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT,   ECBool, "Tray", "bP24SkipEventNeedRemoveColorTrayForIDT",          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP24_3"),  &IniConfig.bP24TwoTrayMustManuallyRemovedForGenernal,ECBool, "Tray", "bP24TwoTrayMustManuallyRemovedForGenernal",       bNoShow, bDisable, bFixedValue, 0);     //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP25"),  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bShow, bDisable, bFixedValue, 1);       //kevin 20220810 ASE_KH
    }
    else if(AUTO_EMPTY_COLOR!=0)                                                //Alick 20160803 add Empty/Color 為四軸時，[P25]不顯示
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP25"),  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bShow, bEnable, bReadFromFile, 0);      //kevin 20151102  empty or color TRAY 不補AUTO 123 TRAY就不須載入一個空TRAY。
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP25"),  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bNoShow, bDisable, bFixedValue, 0);

    if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP26"),    &IniConfig.bP26_OCRCheckLot,                    ECBool,     "Tray", "bP26_OCRCheckLot",                                 bShow, bEnable, bReadFromFile, 0);      //wei 20151117 OCR Lot check
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP26"),    &IniConfig.bP26_OCRCheckLot,                    ECBool,     "Tray", "bP26_OCRCheckLot",                                 bNoShow, bDisable, bFixedValue, 0);

    if(AUTO3_IS_MAGAZINE==1)                                                    //JerryYang 20230515 : P27跟Magazine衝突，先不使用
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP27"),    &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool,  "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bNoShow, bDisable, bFixedValue, 0);
    }
    else
    {
        if(CosFunction.bSortingBinTraywhenCleanOut ||
           CosFunction.bSortingBinTrayWhenTrayFeed)                             //JerryYang 20170911 (Steven) 整盤功能,執行時機由clean out改至tray feed前
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP27"), &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool, "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bShow, bEnable, bReadFromFile, 0);      //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP27"), &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool, "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_UNISEM_M)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP28"),    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bShow, bDisable, bFixedValue, 1);
    else if(CUSTOMER_CODE==CC_SCC || CosFunction.bUseAuto1OnlyBin1)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP28"),    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bShow, bEnable, bReadFromFile, 0);      //Alick 20160801 SCC可設定AUTO1 ONLY BIN1
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP28"),    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP29"),        &IniConfig.bP29LoaderCheckIsFull,               ECBool,     "Function", "bP29LoaderCheckIsFull",                        bShow, bEnable, bReadFromFile, 0);      //Steven 20160818 : CheckLoader滿盤
    elConfig->Add(EL<TEdit>("TfConfiguration", "edP29"),        &IniConfig.dP29LoaderCheckIsFullInterval,       ECDouble,   "Function", "dP29LoaderCheckIsFullInterval",                bShow, bEnable, bReadFromFile,  10,    false,  1.0,      1000.0);   //Steven 20160818 : CheckLoader滿盤

    if(CosFunction.bUseFixTryCheckRemainingAmount)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP30"),    &IniConfig.bP30FixTryCheckRemainingAmount,      ECBool,     "Function", "bP30FixTryCheckRemainingAmount",               bShow, bEnable, bReadFromFile, 0);      //Ifor 20160829 : Check Fix Try 到達設定剩餘IC數量Alarm不停機
        elConfig->Add(EL<TEdit>("TfConfiguration", "edP30"),    &IniConfig.iP30FixCheckRemainingAmountInterval, ECInteger,  "Function", "iP30FixCheckRemainingAmountInterval",          bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);   //Ifor 20160829 : Fix Try 剩餘可放IC數量
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP30"),    &IniConfig.bP30FixTryCheckRemainingAmount,      ECBool,     "Function", "bP30FixTryCheckRemainingAmount",               bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EL<TEdit>("TfConfiguration", "edP30"),    &IniConfig.iP30FixCheckRemainingAmountInterval, ECInteger,  "Function", "iP30FixCheckRemainingAmountInterval",          bNoShow, bDisable, bFixedValue,  10,    false,  1,      1000);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP31"),        &IniConfig.bP31LoaderTryLastOneFeedContinueRun, ECBool,     "Function", "bP31LoaderTryLastOneFeedContinueRun",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20160829 : Loader Try 最後一盤入料Alarm不停機

    if(CosFunction.bUseEmptyColorTrayPreAlarm)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP32"),    &IniConfig.bP32EmptyColorTrayPreAlarm,          ECBool,     "Function", "bP32EmptyColorTrayPreAlarm",                   bShow, bEnable, bReadFromFile, 0);      //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP32"),    &IniConfig.bP32EmptyColorTrayPreAlarm,          ECBool,     "Function", "bP32EmptyColorTrayPreAlarm",                   bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bUseAutoTrayPreAlarm)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP33"),    &IniConfig.bP33AutoTrayPreAlarm,                ECBool,     "Function", "bP33AutoTrayPreAlarm",                         bShow, bEnable, bReadFromFile, 0);      //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP33"),    &IniConfig.bP33AutoTrayPreAlarm,                ECBool,     "Function", "bP33AutoTrayPreAlarm",                         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_TERAPOWER)
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP34"),    &IniConfig.bP34CleanOutChangeInitialMode,       ECBool,     "Function", "bP34CleanOutChangeInitialMode",                bShow, bEnable, bReadFromFile, 0);      //kevin 20170417 (wei) : CleanOut change initial Mode
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP34"),    &IniConfig.bP34CleanOutChangeInitialMode,       ECBool,     "Function", "bP34CleanOutChangeInitialMode",                bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20230608 add:KYEC 要求[P35] Tray Arm home safe pos強制反灰由DB選擇開/關
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP35"),    &IniConfig.bP35TrayArm,                         ECBool,     "Function", "bP35TrayArm",                                  bShow, bDisable, bReadFromFile, 0);      //kevin 20171006 (wei) tray arm home 需遮住sensor
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP35"),    &IniConfig.bP35TrayArm,                         ECBool,     "Function", "bP35TrayArm",                                  bShow, bEnable, bReadFromFile, 0);      //kevin 20171006 (wei) tray arm home 需遮住sensor

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP36"),    &IniConfig.bP36BufferTrayNoSame,                ECBool,     "Function", "bP36BufferTrayNoSame",                         bShow, bEnable, bReadFromFile, 0);      //kevin 20171117 (wei) Load ,&,& Unload 強制不能使用相同軌道取放盤
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP36"),    &IniConfig.bP36BufferTrayNoSame,                ECBool,     "Function", "bP36BufferTrayNoSame",                         bNoShow, bDisable, bFixedValue, 0);
    }

    if(USE_AUTO_RETEST==eartInstall)                                            //JerryYang 20250826 : 有ART才啟用P37
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP37"),    &IniConfig.bP37bAutoCylinderUP,                 ECBool,     "Function", "bP37bAutoCylinderUP" ,                         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP37"),    &IniConfig.bP37bAutoCylinderUP,                 ECBool,     "Function", "bP37bAutoCylinderUP" ,                         bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP38"),        &IniConfig.bP38UseEmptyFullPutColor,            ECBool,     "Function", "bP38UseEmptyFullPutColor",                     bNoShow, bDisable, bFixedValue, 0);     //wei 20170504 Use Empty Full Put Color

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP39"),    &IniConfig.bP39LoaderHasSkipPlaceToEmpty,       ECBool,     "Function", "bP39LoaderHasSkipPlaceToEmpty",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP39"),    &IniConfig.bP39LoaderHasSkipPlaceToEmpty,       ECBool,     "Function", "bP39LoaderHasSkipPlaceToEmpty",                bNoShow, bDisable, bFixedValue, 0);
    }

    if(LoaderUnload_StepMotor)                                                  //Sam 20201221 : Tray y step motor by machine
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP40"),    &IniConfig.bP40TrayYSpeedByMachine,             ECBool,     "Function", "bP40TrayYSpeedByMachine",                      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP40"),    &IniConfig.bP40TrayYSpeedByMachine,             ECBool,     "Function", "bP40TrayYSpeedByMachine" ,                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20240403 : Unload Tray Disable Edit
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP41"),        &IniConfig.bP41UnloadTrayDisableEdit,       ECBool,     "Function", "bP41UnloadTrayDisableEdit",                    bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP41"),        &IniConfig.bP41UnloadTrayDisableEdit,       ECBool,     "Function", "bP41UnloadTrayDisableEdit",                    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_FMSH)                                                  //Ifor 20210428 add:Auto 退Tray 完成時報警停機
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP42"),    &IniConfig.bP42AlarmWhenExitTrayComplete,       ECBool,     "Function", "bP42AlarmWhenExitTrayComplete",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP42"),    &IniConfig.bP42AlarmWhenExitTrayComplete,       ECBool,     "Function", "bP42AlarmWhenExitTrayComplete" ,               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUnloadTrayModeByRecipe)                                     //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkAutoTrayFeed"),           &TrayForm.bAutoFeed,              ECBool,  "Flag", "Auto Feed",                                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbManuTakeAutoFailTray_FT"), &TrayForm.bFailAutoTrayManual_FT, ECBool,  "Flag", "bFailAutoTrayManual_FT",                           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbManuTakeAutoFailTray_RT"), &TrayForm.bFailAutoTrayManual_RT, ECBool,  "Flag", "bFailAutoTrayManual_RT",                           bShow, bEnable, bReadFromFile, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220331 : 矽品中山要求AUTO SKIP後可由TRAY ARM搬TRAY
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP44"),    &IniConfig.bP44LockLoaderTrayToNone,            ECBool,     "Function", "bP44LockLoaderTrayToNone" ,                    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP44"),    &IniConfig.bP44LockLoaderTrayToNone,            ECBool,     "Function", "bP44LockLoaderTrayToNone" ,                    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_MAXIM_THAILAND)     //Ifor 20260831 add MAXIM_THAILAND(default 1) //kevin 20221008 load 空盤 不TRAY
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP45"),    &IniConfig.bP45LastLoaderNoInSide,              ECBool,     "Function", "bP45LastLoaderNoInSide",                       bShow, bEnable, bReadFromFile, 1); //kevin 20221008
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP45"),    &IniConfig.bP45LastLoaderNoInSide,              ECBool,     "Function", "bP45LastLoaderNoInSide" ,                      bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP46"),        &IniConfig.bP46_LoadTrayModeByHandler,          ECBool,     "Tray", "bP46_LoadTrayModeByHandler" ,                      bShow, bEnable, bReadFromFile, 0);        //Steven 20221117 : Loader Tray Mode設定跟著機台變

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP48"),        &IniConfig.bP48UnloaderCylinderLoop,            ECBool,     "Tray", "bP48UnloaderCylinderLoop" ,                        bShow, bEnable, bReadFromFile, 0);        //JimmyChiu 20230512 Unloader cylinder loop
    elConfig->Add(EL<TEdit>("TfConfiguration", "edP48Times"),     &IniConfig.iP48UnloaderCylinderLoopTimes,     ECInteger,  "Tray", "iP48UnloaderCylinderLoopTimes",                    bShow, bEnable, bReadFromFile, 0,    false, 0,       3);
    elConfig->Add(EL<TEdit>("TfConfiguration", "edP48DelayTime"), &IniConfig.iP48UnloaderCylinderLoopDelay,     ECInteger,  "Tray", "iP48UnloaderCylinderLoopDelay",                    bShow, bEnable, bReadFromFile, 0,    false, 0,       2);

    if(CosFunction.bUseLocalTraySpeed==true)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP49"),&IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bShow, bDisable, bFixedValue, 1);        //Ifor 20200825 add: Use Local Tray Speed   //Ifor 20200827 add: KYEC 強制開啟功能並反灰不可修改
        else
            elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP49"),&IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bShow, bEnable, bReadFromFile, 0);     //Ifor 20200825 add: Use Local Tray Speed
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP49"),    &IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bNoShow, bDisable, bFixedValue, 0);    //Ifor 20200825 add: Use Local Tray Speed
    }

    if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                      //Sam 20230221 : 矽格中興國桂要求要能關閉
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP50"),        &IniConfig.bP50DisabledAutoTrackSensorDetect, ECBool,          "Function", "bP50DisabledAutoTrackSensorDetect",          bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP50"),        &IniConfig.bP50DisabledAutoTrackSensorDetect, ECBool,          "Function", "bP50DisabledAutoTrackSensorDetect" ,         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP51"),        &IniConfig.bP51TrayArmPutwaitUnloadOK,          ECBool,     "Function", "bP51TrayArmPutwaitUnloadOK",                   (CUSTOMER_CODE==CC_ASE_KaohSiung), bEnable, bReadFromFile, 1);      //kevin 20230331 add TRAY arm 放 auto 123 等待tray上升避免輸 送帶滑進去 夾tray.
    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                                                                                                                                                                  //KenHsieh 20230919 : Empty & Color Last Tray Check
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP52"),    &IniConfig.bP52EmptyColorLastTrayCheck,         ECBool,     "Function", "bP51EmptyColorLastTrayCheck",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP52"),    &IniConfig.bP52EmptyColorLastTrayCheck,         ECBool,     "Function", "bP51EmptyColorLastTrayCheck",                  bNoShow, bDisable, bFixedValue, 0);

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20231218 : P53防混功能
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP53"),    &IniConfig.bP53_ForcedScanBinCodeOfUnloader,    ECBool,     "Tray", "bP53_ForcedScanBinCodeOfUnloader" ,   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP53"),    &IniConfig.bP53_ForcedScanBinCodeOfUnloader,    ECBool,     "Tray", "bP53_ForcedScanBinCodeOfUnloader" ,   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20251105 add: ASE SG 客戶要求[P54]強制關閉不顯示
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP54"),        &IniConfig.P54UnloaderTaryCheckHasErrorBinIC,   ECBool,      "Function", "P54UnloaderTaryCheckHasErrorBinIC",  bNoShow, bDisable, bFixedValue, 0);   //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP54"),        &IniConfig.P54UnloaderTaryCheckHasErrorBinIC,   ECBool,      "Function", "P54UnloaderTaryCheckHasErrorBinIC",  bShow, bEnable, bReadFromFile, 0);    //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin

    if(CUSTOMER_CODE==CC_KYEC_LEE &&                                            //Ifor 20211014 add: KYEC 要求暫時強制關閉P40功能，反灰不可修改
            AUTO_EMPTY_COLOR==false)                                            //Ifor 20201225 add:六軌模式不顯示並強制關閉
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP55"),        &IniConfig.bP55LdUldUseEmptyAndColorTray,   ECBool,     "Function", "bP40LdUldUseEmptyAndColorTray",                bShow, bDisable, bFixedValue, 0);       //Ifor 20201218 add: Load & Unload Use Empty And Color Tray
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP55"),        &IniConfig.bP55LdUldUseEmptyAndColorTray,   ECBool,     "Function", "bP40LdUldUseEmptyAndColorTray",                bNoShow, bDisable, bFixedValue, 0);         //Ifor 20201218 add: Load & Unload Use Empty And Color Tray
    }

    if(USE_TRAY_ARM_TYPE==eTrayArmNewType)                                     //Ifor 20260824 : New Tray Arm standby at middle of Empty and Color, P56 no effect, hide it
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP56"),       &IniConfig.bP56TrayArmWaitAtColorTrack,     ECBool,     "Function", "bP56TrayArmWaitAtColorTrack",                bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkP56"),           &IniConfig.bP56TrayArmWaitAtColorTrack,     ECBool,     "Function", "bP56TrayArmWaitAtColorTrack",                bShow, bEnable, bReadFromFile, 0);            //Steven 20240516 : Tray Arm等待位置改到Color
    elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP57"),            &IniConfig.bP57LoaderAutoCleanOutByInputCT, ECBool,     "Function", "bP57LoaderAutoCleanOutByInputCT",            bShow, bEnable, bReadFromFile, 0);            //Sam 20250605 : Loader Count AutoCleanOut
    elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgP58RunModeAfterFT"),&IniConfig.iP58RunModeAfterFT,            ECInteger,  "Function", "iP58RunModeAfterFT",                         bShow, bEnable, bReadFromFile,  0);           //JimmyChiu 20250905 : FT模式結束後切換模式

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP59"), &IniConfig.bP59UnloaderICFloattingAlarmAfterExit,  ECBool,     "Function", "bP57UnloaderICFloattingAlarmAfterExit",                      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP59"), &IniConfig.bP59UnloaderICFloattingAlarmAfterExit,  ECBool,     "Function", "bP57UnloaderICFloattingAlarmAfterExit" ,                     bShow, bEnable, bReadFromFile, 0);
    }

    if(CosFunction.bReadClipCodeFromUnloader)                                   //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP60"), &IniConfig.bP60ReadClipCodeFromUnloader,           ECBool,     "Function", "bP60ReadClipCodeFromUnloader",                      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP60"), &IniConfig.bP60ReadClipCodeFromUnloader,           ECBool,     "Function", "bP60ReadClipCodeFromUnloader" ,                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bFirstTrayCheckOnUnloader)                                   //Jimmychiu 20251205 : First Tray Check On Unloader
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP62"),       &IniConfig.bP62FirstTrayCheckOnUnloader,     ECBool,     "Function", "bP62FirstTrayCheckOnUnloader",   bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto1, &IniConfig.bP62Auto1,                        ECBool,     "Function", "bP62Auto1",                      bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto2, &IniConfig.bP62Auto2,                        ECBool,     "Function", "bP62Auto2",                      bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto3, &IniConfig.bP62Auto3,                        ECBool,     "Function", "bP62Auto3",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP62_1"),     &IniConfig.bP62AlwaysEnabledAtLotStart,      ECBool,     "Function", "bP62AlwaysEnabledAtLotStart",    bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP63,       &IniConfig.bP63MachineStopAtIntervalTime,    ECBool,     "Function", "bP63MachineStopAtIntervalTime",  bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(edP63,       &IniConfig.iP63IntervalTime,                 ECInteger,  "Function", "iP63IntervalTime",               bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        EL<TPanel>("TfConfiguration", "plP62")->Visible=false;
//        cbP63->Visible=false;
//        edP63->Visible=false;
    }

    if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                                        //Ifor 20260407 : [P65] Enable ARM QA Mode
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP65"),    &IniConfig.bP65EnableArmQAMode,                 ECBool,     "Function", "bP65EnableArmQAMode",          bShow, bEnable, bReadFromFile, 0);    //Ifor 20260407 : [P65] Enable ARM QA Mode
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtP65"),   &IniConfig.iP65ArmQAModeValue,                  ECInteger,  "Function", "iP65ArmQAModeValue",           bShow, bEnable, bReadFromFile,  0,     false,  0,      30);  //Ifor 20260407 : [P65] ARM QA Mode Value
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP65"),    &IniConfig.bP65EnableArmQAMode,                 ECBool,     "Function", "bP65EnableArmQAMode",          bNoShow, bDisable, bFixedValue, 0);  //Ifor 20260407 : [P65] Enable ARM QA Mode
        elConfig->Add(EL<TEdit>("TfConfiguration", "edtP65"),   &IniConfig.iP65ArmQAModeValue,                  ECInteger,  "Function", "iP65ArmQAModeValue",           bNoShow, bDisable, bFixedValue, 0);  //Ifor 20260407 : [P65] ARM QA Mode Value
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP66"),    &IniConfig.bP66AutoChangingFlashWarn,         ECBool,     "Function", "bP66AutoChangingFlashWarn",                  bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbP66"),    &IniConfig.bP66AutoChangingFlashWarn,         ECBool,     "Function", "bP66AutoChangingFlashWarn",                  bShow, bEnable, bReadFromFile, 0);
    }

}

// golden cConfiguration.cpp:4595  TfConfiguration::InitConfigEdtList()
static void IC_InitConfigEdtList()
{

    IC_InitConfigEdtList_ItemA();
    IC_InitConfigEdtList_ItemB();
    IC_InitConfigEdtList_ItemC();
    IC_InitConfigEdtList_ItemD();
    IC_InitConfigEdtList_ItemE();
    IC_InitConfigEdtList_ItemF();
    IC_InitConfigEdtList_ItemG();
    IC_InitConfigEdtList_ItemI();
    IC_InitConfigEdtList_ItemL();
    IC_InitConfigEdtList_ItemM();
    IC_InitConfigEdtList_ItemN();
    IC_InitConfigEdtList_ItemO();
    IC_InitConfigEdtList_ItemP();

    IC_ChangeCBListProperty();

}

// golden cConfiguration.cpp:276  TfConfiguration::ChangeCBListProperty()
static void IC_ChangeCBListProperty()
{

    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

    IC_ReadLockByFile();                                                           //Sam 20221018 : 修正 LockByFile 問題

    if(CUSTOMER_CODE==CC_KYEC_LEE && Tri_Temp_Machine!=1)                       //Wenqi 20240516 Use by Arm Close Site Function
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbA09")->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    else if(CosFunction.bHaveFIFOMode &&
            IniConfig.bI37_EnableFIFOMode &&
            IniConfig.bI37_EnableFIFOSiteOrder)                                 //Steven 20170302 (wei) : FIFO MODE 跟 By Arm Close Site功能衝突
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbA09")->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbA09")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockD42ByFile)                                              //JerryYang 20160220 add for Amkor-Philipine
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD42")->Tag])->ChangeProperty(bShow, IniConfig.bD42_Enable, !IniConfig.bD42_Enable, IniConfig.bD42_Active);
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
            CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD42")->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, true);
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD42")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockD44ByFile)                                              //JerryYang 20160220 add for Amkor-Philipine
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD44")->Tag])->ChangeProperty(bShow, IniConfig.bD44_Enable, !IniConfig.bD44_Enable, IniConfig.bD44_Active);
    else if(CUSTOMER_CODE==CC_ASE_CL         ||                                 //JerryYang 20210129 : ASE-CL松諭要求強制開啟
            CUSTOMER_CODE==CC_SIGURD_HUKOU   ||                                 //Sam 20220527 : 矽格湖口 Jonas 要求強制開啟
            CUSTOMER_CODE==CC_SIGURD_PeiXing ||                                 //Sam 20240606 : 北興廠也要強制開啟
            CUSTOMER_CODE==CC_KYEC_LEE && Tri_Temp_Machine==1)                  //Wenqi 20240516 KYEC要求強制反灰開啟
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD44")->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, true);
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbD44")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockF06ByFile)
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF06")->Tag])->ChangeProperty(bShow, IniConfig.bF06_Enable, IniConfig.bF06_Enable, IniConfig.bF06_Active);    //Sam 20221018 : 修正 LockByFile 問題
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF06")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockF11ByFile)  //Sam 20240202 : 新增 F11 Lock by file 功能
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF11")->Tag])->ChangeProperty(bShow, IniConfig.bF11_Enable, IniConfig.bF11_Enable, IniConfig.bF11_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF11")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(Cylinder[C_Shuttle_Knocker_1].Enable==false && Cylinder[C_Shuttle_Knocker_2].Enable==false)  //Steven 20160509 : 氣缸沒裝就不要啟用功能
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF14")->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF14_1")->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF14")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF14_1")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if((IniConfig.bA09_ByArmCloseSite ||
        bCanAutoCloseSite==true) &&                                             //Steven 20200420 : 整合Auto Site Off
       IniConfig.bL17HeadHeaterOnWhenCloseSite==false)                          //Steven 20160926 : 避免這幾個功能同時開,會導致IC換邊擺,然後可能發生沒加熱就生產
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF17")->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbF17")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if(CosFunction.bLockF26ByFile)                                              //Sam 20220527 : for 矽格湖口 -- F26 Enable
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TRadioGroup>("TfConfiguration", "rgF26")->Tag])->ChangeProperty(bShow, IniConfig.bF26_Enable, bReadFromFile, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[EL<TRadioGroup>("TfConfiguration", "rgF26")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if(IniConfig.bShowLotInfo)
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbI04")->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbI04")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockI06ByFile)                                              //Sam 20220527 : for 矽格北興
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbI06")->Tag])->ChangeProperty(bShow, IniConfig.bI06_Enable, IniConfig.bI06_Enable, IniConfig.bI06_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbI06")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    #ifndef SOFT_SIMULTE
    if(CosFunction.bKnockerSetBySetupFile==false && Cylinder[C_HotplateVibration].Enable==true)
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbP16")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbP16")->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    #endif

    if(CosFunction.bLockP24ByFile)                                              //JerryYang 20160425 lock P24
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbP24")->Tag])->ChangeProperty(bShow, IniConfig.bP24_Enable, IniConfig.bP24_Enable, IniConfig.bP24_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[EL<TCheckBox>("TfConfiguration", "cbP24")->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

}

// golden cConfiguration.cpp:5816  TfConfiguration::FormClose(TObject *Sender, TCloseAction &Action)
static void IC_FormClose()
{
    filerw::ELMark("FormClose");   // FileRW：存檔流程 trace（ack.trace）

    int ret;
    bool bCloseE45=false;
    AnsiString str1,sNewDir,sOldDir;
    fShow=false;
    EL<TCheckBox>("TfConfiguration", "cbI17")->Checked=false;                                                       //2007/07/29
    EL<TCheckBox>("TfConfiguration", "chkHeater")->Checked=false;
    PauseUT150Polling=false;
    EL<TLabeledEdit>("TfConfiguration", "edtSearchFunction")->Text="";                                                 //Steven 20210730 : 快速搜尋Config
    iMagazineCheckZPos=CheckRange(CheckAndReadIniDataGeneral("System",   "iMagazineCheckZPos",    400), 100, 1000);       //Ifor 20230419 add:往上移動避免客戶功能異常

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    }
    else
    {
        if(bUpdateAutomatically)                                                    //Sam 20220824 : FTP 自動下載安裝更新包
            ret=1;
        else if(CosFunction.bUseConfigSaveButton && bSave==false)    //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
        {
            ret=2;
        }
        else                                                                            //jou 2013-01-24 修正 MessageDlg 訊息被蓋在 Form 下方,像 Hangup。
            ret=filerw::ELAsk("Config data save to define?", "確定要寫入資料？");
        if(ret==2)
        {
            elConfig->InitialDataToEdit();                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
            bEP60DataChange=false;                                                  //JerryYang 20170923 重置EP log旗標
            bEP40DataChange=false;
            bEP30DataChange=false;
            bEP60DataChange_NS=false;
            bEP40DataChange_NS=false;
            bEP30DataChange_NS=false;
            return;
        }

        if(EL<TCheckBox>("TfConfiguration", "cbE45")->Checked!=IniConfig.bE45_AllSetupFileUseOneFile)                   //JerryYang 20220923 : 世明要求關閉E45功能時可選擇是否將Offset覆蓋到所有by recipe offset
        {
            if(EL<TCheckBox>("TfConfiguration", "cbE45")->Checked==false)                                               //關閉E45
            {
                bCloseE45=true;
            }
        }

    //    if(cbN07_EnableSecs->Checked==true && cbA37->Checked==true)               //JerryYang 20250529 : Mark掉
    //    {
    //        cbN07_EnableSecs->Checked=false;
    //        IniConfig.bEnable_SECS_GEM=cbN07_EnableSecs->Checked;
    //    }

        //JimmyChiu 20220114 優化Configuration 儲存方式
        //<==
        IC_CheckConfigurationBeforeSave();
        IC_SaveConfiguration();
        IC_LoadConfiguration();
        //<==
        //JimmyChiu 20220114 優化Configuration 儲存方式
        //Ifor 20220119 add:避免功能關掉又開啟導致異常
        //==>
    //    if(IniConfig.bA15AutoDecayTest==false)
        if(EL<TCheckBox>("TfConfiguration", "cbA15")->Checked==false)
        {
            bRunDecayTest=false;                                                    //Ifor 20220114 add:關閉功能需要清除旗標
            iESD_DecayTask=1;
        }

        if(EL<TCheckBox>("TfConfiguration", "cbA31")->Checked==false)
        {
            bDoIniStartAutoIonFanClean=false;                                       //Ifor 20220114 add:關閉功能需要清除旗標
            bStartAutoIonFanClean=false;                                            //Ifor 20220114 add:關閉功能需要清除旗標
        }
        //<==
        //Ifor 20220119 add:避免功能關掉又開啟導致異常

        SaveEventLogAutoSaveInfo();
        InitShuttleThreadParameter();                                               //Steven 20230109 : 避免資料錯誤
        fLotInfo->RefreshAMR();                                                     //Sam 20240304 : 新增 AMR 功能
        fLotInfo->SetSelectionVisible();                                            //AI(safety-critical-change) 20260611 (RogerYang) : Config存檔後即時刷新下載項目權限
    filerw::ELTodo("golden cConfiguration.cpp:5897-5918 TTL ASE_JP：TestSocket（mykitsuck.h）本 TU 看不到（TList 重複定義），fMain->Send_Command_TTL");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5897-5918 -- TTL ASE_JP：TestSocket（mykitsuck.h）本 TU 看不到（TList 重複定義），fMain->Send_Command_TTL
        if(TestIF_File.bTTLUseASEJPMode)                                            //Frank 20220408 Add TTL ASE_JP Mode
        {
            int i, j, k=0;
            for(i=0; i<MAX_SOCKET_ROW; i++)
            {
                for(j=0; j<MAX_SOCKET_COL; j++)
                {
                    if(!(TestSocket.Item[i][j]<TEST_PASS &&
                         TestSocket.Item[i][j]!=HAS_NULL_IC))
                        k++;
                }
            }

            if(k==0)
            {
                fMain->GetTTLState();
                if(IniConfig.bD22SupportMultiDoubleContact)
                    fMain->Send_Command_TTL("@00WDUTS00000000");
                else
                    fMain->Send_Command_TTL("@00WDUTS00100000");
            }
        }
#endif // GATE (S12-C save)

        if(IniConfig.bEnableCCDUSETCPIP)
        {
            if(IniConfig.bC02InstallCCD==true)
            {
                CCDInterfaceForm->CCDTimerOnOff(true);
            }
            else
            {
                CCDInterfaceForm->CCDTimerOnOff(false);
            }
        }
        EL<TControl>("TfConfiguration", "Timer1")->Enabled=false;

    filerw::ELTodo("golden cConfiguration.cpp:5933-5937 主畫面 UI：fMain->edSetupFileName／sbEngSite 的 Visible（HTML 端處理）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5933-5937 -- 主畫面 UI：fMain->edSetupFileName／sbEngSite 的 Visible（HTML 端處理）
        fMain->edSetupFileName->Visible=IniConfig.bA14UseBarCodeSetWorkFile;
        if(IniConfig.bI35UseThirdSiteControlByEngineer==true)                       //Alick 20160926 add for 第三組工程師用開關SITE
            fMain->sbEngSite->Visible=true;
        else
            fMain->sbEngSite->Visible=false;
#endif // GATE (S12-C save)
        fMain->ShowTestHeadComp(false);

    filerw::ELTodo("golden cConfiguration.cpp:5940-5943 fSCKART->Show()/Close()：SCK ART 視窗（移植樹門面沒有 Show/Close）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5940-5943 -- fSCKART->Show()/Close()：SCK ART 視窗（移植樹門面沒有 Show/Close）
        if(CosFunction.bUseSCKART && USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest && TestIF_File.bSCKART_EnableART)  //Steven 20161201 (wei) : For SCK 93K ART
            fSCKART->Show();
        else
            fSCKART->Close();
#endif // GATE (S12-C save)

    filerw::ELTodo("golden cConfiguration.cpp:5945-5946 fMain->tPSM.Restart()：省電計時器重啟（移植樹 fMain 沒有 tPSM）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5945-5946 -- fMain->tPSM.Restart()：省電計時器重啟（移植樹 fMain 沒有 tPSM）
        if(IniConfig.bPowerSaveFunction)                                            //Frank 20211214 : Add for power saving
            fMain->tPSM.Restart();
#endif // GATE (S12-C save)

        if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200629 : Murata要求可以按按鈕後停止Server功能
        {
            fLotInfo->iXMLOnLineStatus=0;
            fLotInfo->ShowXMLOnLine();
        }

        if(bCloseE45 && IniConfig.bE45_AllSetupFileUseOneFile==false)               //JerryYang 20220923 : 世明要求關閉E45功能時可選擇是否將Offset覆蓋到所有by recipe offset
        {
            ret=filerw::ELAsk("Save by machince offset to by all recipe offset?", "確認是否將By機台offset覆蓋到所有recipe offset?");
            if(ret==1)
            {
                for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
                {
                    str1=fMain->cbSetupFileName->Items->Strings[i];
                    sNewDir=fOffSet->GetOffsetPath(str1);
                    sOldDir.sprintf("%sDefineOffset", DefaultPath);
                    MyForceDirectories(sNewDir);
                    CopyFile(AnsiString(sOldDir+"\\Position Offset.Data").c_str(), AnsiString(sNewDir+"\\Position Offset.Data").c_str(), false);
                    CopyFile(AnsiString(sOldDir+"\\Position Offset Hot.Data").c_str(), AnsiString(sNewDir+"\\Position Offset Hot.Data").c_str(), false);
                }
            }
        }

    filerw::ELTodo("golden cConfiguration.cpp:5971-5984 I21 Auto Site Map：mykitsuck.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 InArmSuck 等");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5971-5984 -- I21 Auto Site Map：mykitsuck.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 InArmSuck 等
        if(IniConfig.bI21EnableASM)                                                 //Steven 20230117 : 開啟Auto Site map時, 要有選項可以選
        {
            if(InArmSuck.HasIC()==false &&
               IndexHasIC()==false &&
               OutArmSuck.HasIC()==false &&
               FRCarryKit.UseSiteNoIC() &&
               BRCarryKit.UseSiteNoIC() &&
               FLCarryKit.HasRealIC()==false &&
               BLCarryKit.HasRealIC()==false &&
               AllArmZIsSafe())
            {
                fMain->SetStartModeData();
            }
        }
#endif // GATE (S12-C save)

        if(IniConfig.bSIGURDFunction &&                                             //Sam 20210331 : 北興新增 Yield Monitor
           fLotInfo->pgLotinfo->ActivePage==fLotInfo->tsYieldMonitior)
        {
            fLotInfo->RefreshYieldMonitor();
        }
        bIndexCheckVacum=EL<TCheckBox>("TfConfiguration", "cbD41")->Checked;                                            //kevin 20200716 add ASE_KH 卡輸入
        if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20201130 : Socket Sensor是否有使用，要在主畫面顯示，要再讀一次檔
        {
            fSetup->ReadFile();
        }

    filerw::ELTodo("golden cConfiguration.cpp:5997-6000 dmTrayMotor->bNeedSetVibrateMotSpeed：震動馬達通訊（移植樹沒有 dmTrayMotor）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5997-6000 -- dmTrayMotor->bNeedSetVibrateMotSpeed：震動馬達通訊（移植樹沒有 dmTrayMotor）
        if(USE_VibrationCommunication)                                              //JerryYang 20230814 : add震動馬達通訊調速版本
        {
            dmTrayMotor->bNeedSetVibrateMotSpeed=true;
        }
#endif // GATE (S12-C save)

        if(IniConfig.bA65_BundleIDList &&                                           //JerryYang 20250704 : add
           IniConfig.bSPILFunction)
        {
            AnsiString sPath=AuthPath+"config.ini";
            IniConfig.bA37LotStartLotEnd=true;
            IniConfig.bA37LotStartLotEnd=ReadWriteIni(sPath, "Function", "bA32LotStartLotEnd", IniConfig.bA37LotStartLotEnd, false, false);
        }

        if(CosFunction.bFirstTrayCheckOnUnloader==true &&                           //Jimmychiu 20251205 : First Tray Check On Unloader
           IniConfig.bP62FirstTrayCheckOnUnloader==false)
        {
            fLotInfo->cbFirstTrayCheckOnUnloader->Checked=false;
        }

        if(CosFunction.bUseConfigSaveButton)    //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
        {
            bSave=false;
        }
    }
    //這一行請保持在最下面!!-----------------
    filerw::ELTodo("golden cConfiguration.cpp:6022-6022 fMain->ToolLoadICO()：工具列圖示（HTML 端處理）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:6022-6022 -- fMain->ToolLoadICO()：工具列圖示（HTML 端處理）
    fMain->ToolLoadICO();                                                       //Ifor 20150728 :更新Toolbar 顯示
#endif // GATE (S12-C save)

}

// golden cConfiguration.cpp:7313  TfConfiguration::CheckConfigurationBeforeSave()
static void IC_CheckConfigurationBeforeSave()
{
    filerw::ELMark("CheckConfigurationBeforeSave");   // FileRW：存檔流程 trace（ack.trace）

    if(bEP60DataChange==true)                                                   //JerryYang 20170922 (Steven) add for EP change log
    {
        bEP60DataChange=false;
        RecordProcess(aEP60OldData);
    }

    if(bEP40DataChange==true)
    {
        bEP40DataChange=false;
        RecordProcess(aEP40OldData);
    }

    if(bEP30DataChange==true)
    {
        bEP30DataChange=false;
        RecordProcess(aEP30OldData);
    }

    if(bEP60DataChange_NS==true)
    {
        bEP60DataChange_NS=false;
        RecordProcess(aEP60OldData_NS);
    }

    if(bEP40DataChange_NS==true)
    {
        bEP40DataChange_NS=false;
        RecordProcess(aEP40OldData_NS);
    }

    if(bEP30DataChange_NS==true)
    {
        bEP30DataChange_NS=false;
        RecordProcess(aEP30OldData_NS);
    }

    bool bFIFOChange=false;
    if(EL<TCheckBox>("TfConfiguration", "cbI37_1")->Checked != IniConfig.bI37_EnableFIFOMode)                       //Ifor 20181107 : add 開啟/關閉FIFO功能顯示需重新啟動程式訊息
    {
        if(EL<TCheckBox>("TfConfiguration", "cbI37_1")->Checked==true)
        {
            if(filerw::ELPasswordRefused("DoPassword_MBox")==false)
            {
                EL<TCheckBox>("TfConfiguration", "cbI37_1")->Checked=false;
            }
            else
            {
                bFIFOChange=true;
            }
        }
        else
        {
            bFIFOChange=true;
        }
    }

    if(IniConfig.bEnableCCDUSETCPIP)
    {
        if(EL<TCheckBox>("TfConfiguration", "cbC02")->Checked!=IniConfig.bC02InstallCCD)
        {
            if(EL<TCheckBox>("TfConfiguration", "cbC02")->Checked)
                RecordProcess("CCD Enabled.");
            else
                RecordProcess("CCD Disabled.");
        }
    }

    LastSet.bSystemInstallScanner=false;

    if(strcmp(LastSet.szSupervisor, "")!=0)
        sSuperVisorString=LastSet.szSupervisor;

//<<[A]-------------------------
    //Steven 20161208 : ART need to set to 32binGS for ATK
    //==>
    IniConfig.iA10TestModeForART=EL<TComboBox>("TfConfiguration", "cbA10_3_ARTTestMode")->ItemIndex;
    if(CosFunction.bUseSCKART)
    {
        if(USE_AUTO_RETEST==eartInstall       &&
           CosFunction.bUseSCKART==true       &&
           IniConfig.bA10_AutoReTest==true    &&
           IniConfig.bA10TestModeForART==true)
        {
            TestIF_File.iGpibMode=IniConfig.iA10TestModeForART;
        }
    filerw::ELTodo("golden cConfiguration.cpp:7400-7400 FTestIF->ReadTestIFFile()：移植樹 GATE F-5（porting-gaps 十四）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7400-7400 -- FTestIF->ReadTestIFFile()：移植樹 GATE F-5（porting-gaps 十四）
        FTestIF->ReadTestIFFile();
#endif // GATE (S12-C save)
        fMain->LoadTestModePicture();
    }
    //<==
    //Steven 20161208 : ART need to set to 32binGS for ATK

//<<[C]-------------------------
    bGetLotIDFormTester                                     =!EL<TCheckBox>("TfConfiguration", "cbC07")->Checked;   //ChungHung 20150615 反向   //ChungHung 20121123 add

//<<[D]-------------------------
    LastSet.dIndexLoadRate[1][0]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position/100.0;                //wei 20150303   京元NS浮動頭
    LastSet.dIndexLoadRate[1][1]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position/100.0;                //wei 20150303   京元NS浮動頭    //wei 20151005 add 56mm
    LastSet.dIndexLoadRate[1][2]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position/100.0;                //wei 20150303   京元NS浮動頭
    LastSet.dIndexLoadRate[1][3]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position/100.0;                //wei 20150303   京元NS浮動頭

    LastSet.dIndexLoadRate[0][0]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position/100.0;                   //jou 2011-06-10
    LastSet.dIndexLoadRate[0][1]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position/100.0;                   //wei 20151005 add 56mm
    LastSet.dIndexLoadRate[0][2]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position/100.0;                   //Steven 20110704
    LastSet.dIndexLoadRate[0][3]                            =EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position/100.0;                   //jou 2011-06-10

    LastSet.dIndexLoadRate[2][0]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edD25_60mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset
    LastSet.dIndexLoadRate[2][1]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edD60_56mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset     //wei 20151005 add 56mm
    LastSet.dIndexLoadRate[2][2]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edD25_40mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset
    LastSet.dIndexLoadRate[2][3]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edD25_30mm")->Text.c_str()), 0.5,   -0.5);//2014-06-26    Dell    for TSMC 高溫Load cell offset

    if(CosFunction.bLockD41ByFile)                                              //Steven 20140627 : Add for ASE-CL
    {
        IniConfig.iD41_Position                             =IniConfig.iD41SocketInitialICCheckPosition;        //Steven 20140627 : Add for ASE-CL -- D41 Inside/Above
        IniConfig.dD41_Offset                               =IniConfig.dD41SocketInitialCheckOffset;            //Steven 20140627 : Add for ASE-CL -- D41 高度
    }

    if(CosFunction.bLockD42ByFile)                                              //JerryYang 20160220 add for Amkor-Philippine
    {
        IniConfig.bD42_Active                               =IniConfig.bD42IndexPickICShuttlePause;             //JerryYang 20160220 : Add for ASE-CL -- D41 打勾
    }

    if(CosFunction.bLockD44ByFile)                                              //JerryYang 20160220 add for Amkor-Philippine
    {
        IniConfig.bD44_Active                               =IniConfig.bD44CheckIndexICDestroy;                 //JerryYang 20160220 : Add for ASE-CL -- D44 打勾
    }

    if(CUSTOMER_CODE==CC_ASE_CL)    //JerryYang 20220112 : 松諭要求回吸時間鎖定為0.2秒
    {
        IniConfig.iD44TestHeadCheckVacuumTime=200;
    }

//<<[E]-------------------------
    //Steven 20090710 Start
    LastSet.fLoaderTrayXScale                               =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_LodX")->Text.c_str()), 0.95, 1.05);
    LastSet.fHotPlateXScale[0]                              =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_HP1X")->Text.c_str()), 0.95, 1.05);
    LastSet.fHotPlateXScale[1]                              =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_HP2X")->Text.c_str()), 0.95, 1.05);
    LastSet.fLoaderTrayYScale                               =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_LodY")->Text.c_str()), 0.95, 1.05);
    LastSet.fHotPlateYScale[0]                              =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_HP1Y")->Text.c_str()), 0.95, 1.05);
    LastSet.fHotPlateYScale[1]                              =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_HP2Y")->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[0]                                  =CheckRange(atof(edE31_Au1X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[1]                                  =CheckRange(atof(edE31_Au2X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[2]                                  =CheckRange(atof(edE31_Au3X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[3]                                  =CheckRange(atof(edE31_Fi1X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[4]                                  =CheckRange(atof(edE31_Fi2X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayXScale[5]                                  =CheckRange(atof(edE31_Fi3X->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[0]                                  =CheckRange(atof(edE31_Au1Y->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[1]                                  =CheckRange(atof(edE31_Au2Y->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[2]                                  =CheckRange(atof(edE31_Au3Y->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[3]                                  =CheckRange(atof(edE31_Fi1Y->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[4]                                  =CheckRange(atof(edE31_Fi2Y->Text.c_str()), 0.95, 1.05);
//    LastSet.fTrayYScale[5]                                  =CheckRange(atof(edE31_Fi3Y->Text.c_str()), 0.95, 1.05);
    LastSet.fInShuttleXScale[0]                             =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_IS1X")->Text.c_str()), 0.95, 1.05);
    LastSet.fInShuttleXScale[1]                             =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_IS2X")->Text.c_str()), 0.95, 1.05);
    LastSet.fInShuttleYScale[0]                             =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_IS1Y")->Text.c_str()), 0.95, 1.05);
    LastSet.fInShuttleYScale[1]                             =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_IS2Y")->Text.c_str()), 0.95, 1.05);
    LastSet.fOutShuttleXScale[0]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_OS1X")->Text.c_str()), 0.95, 1.05);
    LastSet.fOutShuttleXScale[1]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_OS2X")->Text.c_str()), 0.95, 1.05);
    LastSet.fOutShuttleYScale[0]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_OS1Y")->Text.c_str()), 0.95, 1.05);
    LastSet.fOutShuttleYScale[1]                            =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_OS2Y")->Text.c_str()), 0.95, 1.05);
    //Steven 20090717 End

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
    {
        LastSet.fLoaderTrayXScale_Hot                       =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_LodX")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateXScale_Hot[0]                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_HP1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateXScale_Hot[1]                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_HP2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fLoaderTrayYScale_Hot                       =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_LodY")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateYScale_Hot[0]                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_HP1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateYScale_Hot[1]                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_1_HP2Y")->Text.c_str()), 0.95, 1.05);

        LastSet.fInShuttleXScale_Hot[0]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_IS1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleXScale_Hot[1]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_IS2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleYScale_Hot[0]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_IS1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleYScale_Hot[1]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_IS2Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleXScale_Hot[0]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_OS1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleXScale_Hot[1]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_OS2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleYScale_Hot[0]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_OS1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleYScale_Hot[1]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_1_OS2Y")->Text.c_str()), 0.95, 1.05);

        LastSet.fLoaderTrayXScale_Cold                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_LodX")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateXScale_Cold[0]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_HP1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateXScale_Cold[1]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_HP2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fLoaderTrayYScale_Cold                      =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_LodY")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateYScale_Cold[0]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_HP1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fHotPlateYScale_Cold[1]                     =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE30_2_HP2Y")->Text.c_str()), 0.95, 1.05);

        LastSet.fInShuttleXScale_Cold[0]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_IS1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleXScale_Cold[1]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_IS2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleYScale_Cold[0]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_IS1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fInShuttleYScale_Cold[1]                    =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_IS2Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleXScale_Cold[0]                   =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_OS1X")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleXScale_Cold[1]                   =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_OS2X")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleYScale_Cold[0]                   =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_OS1Y")->Text.c_str()), 0.95, 1.05);
        LastSet.fOutShuttleYScale_Cold[1]                   =CheckRange(atof(EL<TEdit>("TfConfiguration", "edE32_2_OS2Y")->Text.c_str()), 0.95, 1.05);
    }                                                                           //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--

    IniConfig.bE52_EnableOutArmZADC                         =EL<TCheckBox>("TfConfiguration", "cdE52")->Checked;

//<<[F]-------------------------

//<<[G]-------------------------

//<<[I]-------------------------
    if(CosFunction.bHaveFIFOMode)                                               //Steven 20170302 (wei) : FIFO MODE
    {
        IniConfig.iI37_LockLoaderDirection=EL<TControl>("TfConfiguration", "imgI37_3")->Tag;
    }
//<<[L]-------------------------
    //jou 980928 start : High tempture 130 deg Position shift
    LastSet.TempPosShift_Shuttle[0][0]                      =CheckRange(atoi(EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text.c_str()), 1000, -1000);      //正負1CM
    LastSet.TempPosShift_Shuttle[0][1]                      =CheckRange(atoi(EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text.c_str()), 1000, -1000);      //正負1CM
    LastSet.TempPosShift_Shuttle[1][0]                      =CheckRange(atoi(EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text.c_str()), 1000, -1000);      //正負1CM
    LastSet.TempPosShift_Shuttle[1][1]                      =CheckRange(atoi(EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text.c_str()), 1000, -1000);      //正負1CM
    //jou 980928 end

//<<[M]-------------------------

//<<[N]-------------------------
    if(IniConfig.bEnable_SECS_GEM==false)                                       //Steven 20140528 : Secs Gem
        IniConfig.sGPIBMachineID                            =EL<TEdit>("TfConfiguration", "edN04_ID")->Text;    //kevin 20130425 機台名稱    //ChungHung alter 20130510  //Steven 20140606 : For Secs Gem
    IniConfig.sMachineType                                  =EL<TEdit>("TfConfiguration", "edN04_Model")->Text; //Steven 20101019 Start
//    IniConfig.SocketHandlerID                               =edN04_ID->Text;  //Steven 20101019 Start
    SetSocketHandlerID(EL<TEdit>("TfConfiguration", "edN04_ID")->Text);                                         //Jimmychiu 20220805 SocketHandlerID can not be NULL
    IniConfig.RMSTesterID                                   =EL<TEdit>("TfConfiguration", "edtN04_TesterID")->Text;   //Ifor 20231219 add Tester ID
    if(IC_EnableRMSFunc())
    {
        if(CUSTOMER_CODE==CC_SCK)                                               //Steven 20210616 : Add special password during RMS "OFF" (password is same as RMS up load password)
        {
            if(IniConfig.bEnableRms!=EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked)                  //Steven 20210730 : Add special password during RMS "ON/OFF"
            {
                AnsiString sName    =CheckAndReadIniDataGeneral("RMS_Password", "Name", AnsiString("RMSUPLOAD"));
                AnsiString sPassword=CheckAndReadIniDataGeneral("RMS_Password", "Password", AnsiString("RMSUPLOAD"));
                fPassword->edUserName->Text="";                                 // name
                fPassword->edPassword->Text="";                                 // password
                fPassword->ShowModal();
                if(fPassword->edUserName->Text.UpperCase()!=sName.UpperCase() ||
                   fPassword->edPassword->Text.UpperCase()!=sPassword.UpperCase())
                {
                    EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked=true;
                }
            }
        }

        IniConfig.bEnableRms                                =EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked;
        IniConfig.sRmsPath                                  =EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Text;
        IniConfig.sRmsDownPath                              =EL<TEdit>("TfConfiguration", "edN05_DownPath")->Text;
        IniConfig.bCheckFile                                =EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Checked;
        IniConfig.bClearLotInfoWhenTrayFeed                 =EL<TCheckBox>("TfConfiguration", "chkN05_TrayFeedClear")->Checked; //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
        IniConfig.bEnableRmsCheckSetupFile                  =EL<TCheckBox>("TfConfiguration", "cbN05_CheckSetupFile")->Checked;  //Ifor 20230516 add: TFAMD 要求工作檔驗證
    }
    else if(IniConfig.bShowLotInfo)
    {
        IniConfig.bEnableRms                                =EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked;
        IniConfig.sRmsPath                                  =EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Text;
        IniConfig.fAmbientTemp                              =CheckRange(atof(EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Text.c_str()), 15.0, 50.0);        //15~50度
        IniConfig.bCheckFile                                =EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Checked;
        IniConfig.bEnableErms                               =EL<TCheckBox>("TfConfiguration", "cbN05_1")->Checked;  //Steven 20160711 : 使用進階版RMS

        IniConfig.bN05_RTCalarmUnload                       =EL<TCheckBox>("TfConfiguration", "checkbN05_RTC")->Checked;    //jou 20170210 (Steven) : RTC alarm image unload
        IniConfig.asN05_RTCalarmUnload                      =EL<TEdit>("TfConfiguration", "editN05_RTC")->Text;         //jou 20170210 (Steven) : RTC alarm image unload
        IniConfig.bClearLotInfoWhenTrayFeed                 =EL<TCheckBox>("TfConfiguration", "chkN05_TrayFeedClear")->Checked; //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
        IniConfig.bEnableRmsCheckSetupFile                  =EL<TCheckBox>("TfConfiguration", "cbN05_CheckSetupFile")->Checked;  //Ifor 20230516 add: TFAMD 要求工作檔驗證

        if(EL<TEdit>("TfConfiguration", "edN05_1")->Text=="D:\\HT9045" ||
           EL<TEdit>("TfConfiguration", "edN05_1")->Text=="D:\\HT9045\\system" ||
           EL<TEdit>("TfConfiguration", "edN05_1")->Text=="D:\\HT9045\\config" ||
           EL<TEdit>("TfConfiguration", "edN05_1")->Text=="D:\\HT9045\\IniData" ||
           EL<TEdit>("TfConfiguration", "edN05_1")->Text=="D:\\HT9045\\IniData\\Data")
            EL<TEdit>("TfConfiguration", "edN05_1")->Text="D:\\RMS";

        IniConfig.sErmsPath                                 =EL<TEdit>("TfConfiguration", "edN05_1")->Text;

        if(CosFunction.bDownloadRecipeLevelMode)                                //jou 2016-01-06 download recipe 增加權限模式選擇
            IniConfig.sRmsDownPath                          =EL<TEdit>("TfConfiguration", "edN05_DownPath")->Text;
    }
    else
    {
        IniConfig.bEnableRms=false;                                             //Steven 20110304 : 防呆
        IniConfig.bEnableErms=false;
    }

    if(CosFunction.bFTPFunction || IniConfig.bFTPJamCodeUpload)                 //ChungHung 20140108 add FTP unload jam code
    {
        IniConfig.bEnableFTP                                =EL<TCheckBox>("TfConfiguration", "cbN06_EnableFTP")->Checked;
        IniConfig.FtpUserName                               =EL<TEdit>("TfConfiguration", "edN06_UserName")->Text;
        IniConfig.FtpPassword                               =EL<TEdit>("TfConfiguration", "edN06_Password")->Text;
        IniConfig.FtpHost                                   =EL<TEdit>("TfConfiguration", "edN06_HostName")->Text;
        IniConfig.FtpDownloadPath                           =EL<TEdit>("TfConfiguration", "edN06_DownPath")->Text;
        IniConfig.FtpUplaodPath                             =EL<TEdit>("TfConfiguration", "edN06_UpLdPath")->Text;
        IniConfig.iHDEnable                                 =CheckRange(EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->ItemIndex, 0, 3);
        IniConfig.iServerEnable                             =CheckRange(EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->ItemIndex, 0, 3);
        IniConfig.N06_FtpPort                               =EL<TEdit>("TfConfiguration", "edN06_Port")->Text;  //Ifor 20201015 add:使用者自定義 FTP Port
        IniConfig.FtpTransMode                              =EL<TComboBox>("TfConfiguration", "cbbN06_Mode")->ItemIndex;                    //Steven 20230719 : 加入FTP傳輸模式

        if(CosFunction.PassworDownloadByFTP)                                    //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
        {
            IniConfig.FtpPasswordDownloadPath               =EL<TEdit>("TfConfiguration", "edN06_1")->Text;
            IniConfig.bFtpPasswordDownload                  =EL<TCheckBox>("TfConfiguration", "cbN06_1")->Checked;
        }

        if(CUSTOMER_CODE!=CC_TSMC_TAINAN)
        {
            if(CUSTOMER_CODE==CC_JSCC_OS)                                           //長電微電子 (JSCC OS部門)
            {
                IniConfig.asN06_TesterPath                  =EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Text;
                IniConfig.N06_TasterListFile                ="";
                IniConfig.N06_TasterListMap                 ="";
            }
            else
            {
                IniConfig.N06_TasterListFile                =EL<TEdit>("TfConfiguration", "edN06_TestList")->Text;
                IniConfig.N06_TasterListMap                 =EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Text;
            }
        }
    }
    else
    {
        IniConfig.bEnableFTP=false;
    }

    if(bSecsGemStatus!=EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs")->Checked)                               //Steven 20200818 : 修正Secs Gem的initial問題
    {
    filerw::ELTodo("golden cConfiguration.cpp:7637-7642 FSECS->GemInitial：SECS/GEM 初始化（移植樹沒有 FSECS）");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7637-7642 -- FSECS->GemInitial：SECS/GEM 初始化（移植樹沒有 FSECS）
        if(EL<TCheckBox>("TfConfiguration", "cbN07_EnableSecs")->Checked==true && bReadSecsGemData==false)
        {
            IniConfig.bEnable_SECS_GEM=true;                                    //JerryYang 20210222 : 修正啟用SECS/GEM沒有重新Initial的問題
            FSECS->GemInitial(IniConfig.sMachineType, RunInfo.SoftwareVersion); //Steven 20140606 : For Secs Gem
            bReadSecsGemData=true;                                              //wei 20160308 是否讀取SECSGEM資料
        }
#endif // GATE (S12-C save)
    }

    filerw::ELTodo("golden cConfiguration.cpp:7645-7648 fCleaning->LoadAutoCleanData()：移植樹仍是 GATE");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7645-7648 -- fCleaning->LoadAutoCleanData()：移植樹仍是 GATE
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20170821 offset ftp
    {
        fCleaning->LoadAutoCleanData();                                         //wei 20171023
    }
#endif // GATE (S12-C save)

//<<[O]-------------------------
    if(IniConfig.bEventLogAutoSaveFunction)                                     //Steven 20110221 Start : EventLogAutoSave
    {
        //AI(ht9045-v899) 20260623: [O]EventLog存檔包try/catch，避免單一picker例外中斷整個SaveConfiguration(原會使Enable FTP等設定全寫不進)
        try
        {
            //AI(ht9045-v899) 20260623: dtpO06NextTime為dtkTime picker,指派->Date在界外日期(1899-12-30)拋EDateTimeError;改用日期picker的Date加時間picker的Time組合,移除非法指派
            IniConfig.dtEventLogLastRecordDate                  =EL<filerw::ELDateTimePicker>("TfConfiguration", "dtO06_LastDate")->Date + EL<filerw::ELDateTimePicker>("TfConfiguration", "dtpO06NextTime")->Time;
            IniConfig.sEventLogLastRecordDate                   =IniConfig.dtEventLogLastRecordDate.FormatString("yyyy/mm/dd hh:mm:ss");    //Ifor 20160621 修正config.ini 時間異常問題
            IniConfig.iNextEventLogRecordSpace                  =1;
            for(int i=0; i<7; i++)
                IniConfig.bAutoSaveLogWeek[i]=EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[i][1]!=""?true:false;
        }
        catch(...) { /*AI(ht9045-v899) 20260623: 吞例外,不中斷後續存檔*/ }
    }

//<<[P]-------------------------
    if(EL<TRadioButton>("TfConfiguration", "rgP23_OCRByNewTray")->Checked==true)                                       //ChungHung 20121002 add OCR Function
        IniConfig.iOCRConditions=0;
    else if(EL<TRadioButton>("TfConfiguration", "rgP23_OCRByInitialStart")->Checked==true)
        IniConfig.iOCRConditions=1;
    else
        IniConfig.iOCRConditions=0;

//<<-------------以下請保持在最下面!!------------------------------
    IC_WriteContactData();                                                         //kevin 20170329 (Steven) 將CONTRACT 資料寫到INI 給 客戶檢查

    if(IniConfig.bShowLotInfo && (CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK ))
    {
        fMain->labAutomation->Caption    =(IniConfig.bEnableRms)?"RMS Enabled":"RMS Disabled";
        ;   // UI-only（HTML 端處理）：fMain->labAutomation->Font->Color=(IniConfig.bEnableRms)?clBlue:clTeal;
    }
    else if(IniConfig.bShowLotInfo && IniConfig.bVTESTFunction==true)           //jou 20230407 : config N05 & check file 同時開啟時才顯示 Network Enabled
    {
        fMain->labAutomation->Caption=((IniConfig.bEnableRms || IniConfig.bEnableFTP) && IniConfig.bCheckFile)?"Network Enabled":"Network Disabled";
        ;   // UI-only（HTML 端處理）：fMain->labAutomation->Font->Color=((IniConfig.bEnableRms || IniConfig.bEnableFTP) && IniConfig.bCheckFile)?clBlue:clTeal;
    }
    else if(IniConfig.bShowLotInfo)
    {
        fMain->labAutomation->Caption    =(IniConfig.bEnableRms || IniConfig.bEnableErms)?"Network Enabled":"Network Disabled";
        ;   // UI-only（HTML 端處理）：fMain->labAutomation->Font->Color=(IniConfig.bEnableRms || IniConfig.bEnableErms)?clBlue:clTeal;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20190925 : Add 京元換FTP Server 需可修改路徑
    {
        if(bFIFOChange==true)                                                   //Ifor 20181107 : add 開啟/關閉FIFO功能顯示需重新啟動程式訊息
        {
            filerw::ELMessage("Open/Close FIFO Function need restarting the HMI!", "打開/關閉FIFO功能需要重新啟動HMI。");
        }
    }

    if(CosFunction.bFTPFunction)                                                //Ifor 20231101 add:FTP Function
    {
        if(IniConfig.bEnableFTP==true)
        {
            fMain->SendMSG_CMD(MSG_CMD_EnableFTPFunction);
        }
        else
        {
            fMain->SendMSG_CMD(MSG_CMD_DisableFTPFunction);
        }
    }

}

// golden cConfiguration.cpp:7291  TfConfiguration::SaveConfiguration()
static void IC_SaveConfiguration()
{
    filerw::ELMark("SaveConfiguration");   // FileRW：存檔流程 trace（ack.trace）

    bool bO12Before = (ReadIniData(GetRecipePath()+"configByRecipe.ini", "O_Count", "bLifeTimeCount 1", AnsiString("0")).ToIntDef(0)!=0);  //AI(ht9045-config) 20260728 (RogerYang) : 存檔前讀O12舊值(SaveLastSetIni尚未寫,=舊)

    filerw::ELTodo("golden cConfiguration.cpp:7295-7295 fMain->DoShowUserDefFrom()：主畫面 UI");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7295-7295 -- fMain->DoShowUserDefFrom()：主畫面 UI
    fMain->DoShowUserDefFrom();                                                 //Steven 20110224
#endif // GATE (S12-C save)
    WriteLastDataFile();
    SaveLastSetIni();
    SaveTasterInfo();                                                           //Steven 20121019 : 補上
    filerw::ELTodo("golden cConfiguration.cpp:7299-7299 fMain->ChangeMainFormWitdh()：主畫面 UI");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7299-7299 -- fMain->ChangeMainFormWitdh()：主畫面 UI
    fMain->ChangeMainFormWitdh();                                               //RogerYang 20250728 Vtest設開關決定要不要顯示View
#endif // GATE (S12-C save)
    IC_SetA73();                                                                   //Eastsun 20260515 RT FT Bin分開設定
    filerw::ELTodo("golden cConfiguration.cpp:7301-7302 FrmAOI->AOIFailCountRefresh()：AOI 未移植");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7301-7302 -- FrmAOI->AOIFailCountRefresh()：AOI 未移植
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && IniConfig.bA74AOIFailCountLinkLotRunMode)
        FrmAOI->AOIFailCountRefresh();                 //Eastsun 20260408 :
#endif // GATE (S12-C save)

    bool bO12After = (ReadIniData(GetRecipePath()+"configByRecipe.ini", "O_Count", "bLifeTimeCount 1", AnsiString("0")).ToIntDef(0)!=0);   //AI(ht9045-config) 20260728 (RogerYang) : 存檔後讀O12新值
    if(IniConfig.bVTESTFunction && CosFunction.bUseHeadContactCount &&                                  //AI(ht9045-config) 20260728 (RogerYang) : 僅VTEST銦片
       bO12Before==false && bO12After==true)                                                           //AI(ht9045-config) 20260728 (RogerYang) : O12由關→開的上升緣才補arm(開批時O12=0沒arm,操作員此刻才開)
    {
    filerw::ELTodo("golden cConfiguration.cpp:7308-7308 fLotInfo->bVTESTKitCheckPending：移植樹 TfLotInfo 沒有這個欄位");
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:7308-7308 -- fLotInfo->bVTESTKitCheckPending：移植樹 TfLotInfo 沒有這個欄位
        fLotInfo->bVTESTKitCheckPending = true;                                 //AI(ht9045-config) 20260728 (RogerYang) : 補arm,下次Start消化→跳Change KIT
#endif // GATE (S12-C save)
        RecordProcess("VTEST Indium: O12 OFF->ON in Config, arm KIT check");    //AI(ht9045-config) 20260728 (RogerYang) : 診斷,確認OK後可留
    }

}

// golden cConfiguration.cpp:7714  TfConfiguration::LoadConfiguration()
static void IC_LoadConfiguration()
{
    filerw::ELMark("LoadConfiguration");   // FileRW：存檔流程 trace（ack.trace）

    ReadLastDataFile();
    ReadLastSetIni();

}

// golden cConfiguration.cpp:7905  TfConfiguration::SetA73()
static void IC_SetA73()
{
    filerw::ELMark("SetA73");   // FileRW：存檔流程 trace（ack.trace）

    // EastSun 20260508 : A73 apply runtime to memory vars (KYEC only) ; if state changed, reload Bin file so memory matches the chosen FT/RT scheme
//    if(CUSTOMER_CODE==CC_KYEC_LEE) //Eastsun 此方案只適用 bFTBin2RTBin bDisableRTBinSet都為true 的客戶    //Eastsun 20260710整合
    if(CUSTOMER_CODE==CC_KYEC_CHEN || CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_STM || CUSTOMER_CODE==CC_KYEC_XILINX)  //Eastsun 20260710整合
    {
        bool bWantFT2RT = (IniConfig.bA73UserOverrideFTRTBin==false);   // A73 OFF => keep legacy (FT==RT) ; A73 ON => separate
        IniConfig.bFTBin2RTBin       = bWantFT2RT;
        CosFunction.bDisableRTBinSet = bWantFT2RT;
        //EastSun 20260508 : only reload when user actually toggled the A73 checkbox (avoid loading on every save)
        static int sLastA73 = -1;   // -1 = first call, force mismatch to sync once at startup
        bool bA73Changed = (sLastA73 != (int)IniConfig.bA73UserOverrideFTRTBin);
        sLastA73 = (int)IniConfig.bA73UserOverrideFTRTBin;
        if(bA73Changed && fBinSel!=NULL)
        {
            fBinSel->ReadParam();                  //EastSun 20260508 : reload bin settings
            fBinSel->ReadFile(false, false, "");   //EastSun 20260508 : reload bin file (FT/RT path picked by bFTBin2RTBin)
            SetTestRunMode();                      //EastSun 20260508 : refresh iTestRunMode FIRST so ShowBinSel picks the right BinSelect[]
            if(fShowBinSelect!=NULL)
            {
                fShowBinSelect->ShowBinSel();           //EastSun 20260508 : repaint main view BinSel label/panel
                fShowBinSelect->InitShowBinDigital();   //EastSun 20260508 : refresh hardware BinDisplay (7-seg / TFT) after FT/RT scheme change
            }
        }
    }

}

// golden cConfiguration.cpp:7849  TfConfiguration::EnableRMSFunc()
static bool IC_EnableRMSFunc()
{

    bool bret=false;
    bret=CosFunction.bEnableRMS ||
         (IniConfig.bShowLotInfo && (CUSTOMER_CODE==CC_SCC    ||                //Steven 20101102 : RMS
                                     CUSTOMER_CODE==CC_SCK     ||
                                     IniConfig.bAMDFunction   ||               //Ifor 20200812 add: TF-AMD USE RMS
                                     CUSTOMER_CODE==CC_ETRENDTECH));            //Steven 20230302 : add for 逸昌
    return bret;

}

// golden cConfiguration.cpp:6733  TfConfiguration::WriteContactData()
static void IC_WriteContactData()
{
    filerw::ELMark("WriteContactData");   // FileRW：存檔流程 trace（ack.trace）

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        AnsiString sPath = "D:\\HT9045\\system\\ep.txt";
        WriteIniData(sPath, "Config", "D25-1", LastSet.dIndexLoadRate[0][0]);   //kevin 20170329 add  tbD25_Index60mm
        WriteIniData(sPath, "Config", "D25-2", LastSet.dIndexLoadRate[0][2]);   //kevin 20170329 add  tbD25_Index40mm
        WriteIniData(sPath, "Config", "D25-3", LastSet.dIndexLoadRate[0][3]);   //kevin 20170329 add  tbD25_Index30mm
    }

}

// golden cConfiguration.cpp:4614  TfConfiguration::FormShow(TObject *Sender)
static void IC_FormShow()
{

    AnsiString str;
    int iATCVerBuf=0;                                                           //KenHsieh 20240216 : add ATC Power Follow Function

    filerw::ELEnableNames("TfConfiguration", kIC_EnableAll, (int)(sizeof(kIC_EnableAll) / sizeof(kIC_EnableAll[0])));   // golden：頁與底下的容器／TLabel／TSpeedButton／TButton／TCheckBox／TRadioButton Enabled=true（審查 #2）   // golden cConfiguration.cpp:4619-4623 取代：pcConfig->Pages[i] 逐頁 ChangeCompomentEnabled(頁, true, true)：替身沒有頁的清單
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:4619-4623 -- pcConfig->Pages[i] 逐頁 ChangeCompomentEnabled(頁, true, true)：替身沒有頁的清單
    for(int i=0; i<EL<TPageControl>("TfConfiguration", "pcConfig")->PageCount; i++)                                    //Steven 20210810 : 快速搜尋Config加上權限控制
    {
        if(EL<TPageControl>("TfConfiguration", "pcConfig")->Pages[i]!=EL<TTabSheet>("TfConfiguration", "tsSearchFunction"))
            filerw::ELChangeCompomentEnabled(EL<TPageControl>("TfConfiguration", "pcConfig")->Pages[i], true, true);
    }                                                                           //Steven 20120807 End: 先在最上面Enable全部畫面
#endif // GATE (S12-C save)

    ReadLastSetIni();
    fShow=true;
    EL<TTabSheet>("TfConfiguration", "tsTempComm")->TabVisible=(CUSTOMER_CODE==CC_HONPREC_QC)?true:false;
    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //Steven 20110818
    {
        EL<TTabSheet>("TfConfiguration", "tsTempComm")->TabVisible=true;
    }

    EL<TListBox>("TfConfiguration", "listHeaterMonitor")->Items->Clear();
    EL<TCheckBox>("TfConfiguration", "chkHeater")->Checked=false;
    bStopChange=true;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(EL<TPageControl>("TfConfiguration", "PageControl1")->ActivePageIndex==2)                                    //kevin 20180411 key in password
        {
            EL<TPageControl>("TfConfiguration", "PageControl1")->ActivePageIndex=0;
        }
    }
    else
    {
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfConfiguration", "PageControl1")->ActivePage=EL<TTabSheet>("TfConfiguration", "tsConfig");
    }

#if 0 // GATE (S12-C save) golden cConfiguration.cpp:4648-4672 -- edSoftSpeed[]／labSoftSpeed[]：動態產生的軟體速度欄（存檔走 edSoftSpeed0Change，另一條觸發）——Steven 20260927 Q19：不需要（不做）
    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        edSoftSpeed[i]->Text=(LastSet.SoftSpeed[i]==0)?10000:LastSet.SoftSpeed[i];   //JerryYang 20161129 軟體模擬speed 1000->10000 //Steven 20160816 : provide initial soft speed as 1000
        edSoftSpeed[i]->Tag=i;
        labSoftSpeed[i]->Caption=MOT[i].Alias;
        if(MOT[i].Alias=="")                                                    //Steven 20150910 : 改成動態產生
        {
            edSoftSpeed[i]->Visible=false;
            labSoftSpeed[i]->Visible=false;
        }
        edSoftSpeed[i]->Enabled=authConfig[0];                                  //Steven 20090731
    }

    EL<TButton>("TfConfiguration", "btnAdd1000")  ->Enabled=authConfig[0];
    EL<TButton>("TfConfiguration", "btnDec1000")  ->Enabled=authConfig[0];
    EL<TButton>("TfConfiguration", "btnSetTo1000")->Enabled=authConfig[0];
    EL<TButton>("TfConfiguration", "btnAdd10000") ->Enabled=authConfig[0];
    EL<TButton>("TfConfiguration", "btnSetToTech")->Enabled=authConfig[0];

//#ifndef Carry4
    edSoftSpeed[17] ->Visible=false;
    labSoftSpeed[17]->Visible=false;
    edSoftSpeed[18] ->Visible=false;
    labSoftSpeed[18]->Visible=false;
//#endif
#endif // GATE (S12-C save)

    bStopChange=false;

    if(bHasTrayCSV)                                                             //Steven 20210629 : Tray Form改成CSV
    {
        EL<TSpeedButton>("TfConfiguration", "sbtReloadTray")->Click();
    }

    if(bHasPlateCSV)                                                            //Steven 20210629 : Plate Form改成CSV
    {
        EL<TSpeedButton>("TfConfiguration", "sbtReloadHP")->Click();
    }

    GetConfAuth();                                                              //Steven 20090731

//[A]-------------------------
    if(IniConfig.bA09_ByArmCloseSite)                                           //ChungHung 20140505 add 加強保護
        CosFunction.bOneCycleCanChangeArm=false;

//[C]-------------------------
    EL<TCheckBox>("TfConfiguration", "cbC07")->Checked=!bGetLotIDFormTester;                                        //ChungHung 20150615 反向

//[D]-------------------------
    double temp;
    temp=LastSet.dIndexLoadRate[1][0]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position=int(temp);                                     //wei 20150303   京元NS浮動頭

    temp=LastSet.dIndexLoadRate[1][2]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position=int(temp);

    temp=LastSet.dIndexLoadRate[1][3]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position=int(temp);

    temp=LastSet.dIndexLoadRate[0][0]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position   =int(temp);

    temp=LastSet.dIndexLoadRate[0][2]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position   =int(temp);

    temp=LastSet.dIndexLoadRate[0][3]*100;
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position   =int(temp);

    EL<TEdit>("TfConfiguration", "edD25_60mm")->Text            =LastSet.dIndexLoadRate[2][0];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset
    EL<TEdit>("TfConfiguration", "edD25_40mm")->Text            =LastSet.dIndexLoadRate[2][2];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset
    EL<TEdit>("TfConfiguration", "edD25_30mm")->Text            =LastSet.dIndexLoadRate[2][3];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset

    EL<TEdit>("TfConfiguration", "edD25_60mm")->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);      //2014-06-26    Dell    for TSMC 高溫Load cell offset
    EL<TEdit>("TfConfiguration", "edD25_40mm")->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
    EL<TEdit>("TfConfiguration", "edD25_30mm")->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
    EL<TLabel>("TfConfiguration", "labD25_4")->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    EL<TLabel>("TfConfiguration", "labD25_5")->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    EL<TLabel>("TfConfiguration", "labD25_6")->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Visible =(CosFunction.bEPUseNSSLK);                     //wei 20150303   京元NS浮動頭
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Visible =(CosFunction.bEPUseNSSLK);
    EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Visible =(CosFunction.bEPUseNSSLK);
    EL<TLabel>("TfConfiguration", "labD25_1_NS")->Visible        =(CosFunction.bEPUseNSSLK);
    EL<TLabel>("TfConfiguration", "labD25_2_NS")->Visible        =(CosFunction.bEPUseNSSLK);
    EL<TLabel>("TfConfiguration", "labD25_3_NS")->Visible        =(CosFunction.bEPUseNSSLK);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CosFunction.bHiSiliconFunction==true) //kevin 20170328 (Steven) EP 密碼輸入   //Ifor 20170803 (wei) add KYEC EP 密碼  //JerryYang 20170921 (Steven) Hisi版本 EP offset鎖定
    {
        EL<TGroupBox>("TfConfiguration", "gbD25")->Enabled=false;
        ;   // UI-only（HTML 端處理）：EL<TGroupBox>("TfConfiguration", "gbD25")->Color =clRed;
    }

    EL<TGroupBox>("TfConfiguration", "gbD25")->Visible              =(CosFunction.bUseDynamicKitDiameter==false);   //Steven 20170605 : 可以自定義Kit直徑
    EL<TButton>("TfConfiguration", "btnOpenEP")->Visible          =(CosFunction.bUseDynamicKitDiameter==false);   //Ifor 20170911 (Steven) : 無自訂Kit不需顯示Open EP 開關

    if(CosFunction.bLockD41ByFile)                                              //Steven 20140627 : Add for ASE-CL
    {
        EL<TEdit>("TfConfiguration", "edD41")->Enabled  =IniConfig.bD41_Enable;
        EL<TComboBox>("TfConfiguration", "coD41")->Enabled  =IniConfig.bD41_Enable;
    }

    EL<filerw::ELTrackBar>("TfConfiguration", "udD46")->Position     =IniConfig.iD46WaitIndexDestroyTime;

    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                              //Steven 20110907 : Real Time CCD - 燈光Start後不是On就是Off
    {
        EL<TLabel>("TfConfiguration", "labD53")->Visible=false;
        EL<TEdit>("TfConfiguration", "edD53")->Visible=false;
        EL<TEdit>("TfConfiguration", "edD53")->Text=0;
    }
    else
    {
        EL<TLabel>("TfConfiguration", "labD53")->Visible=true;
        EL<TEdit>("TfConfiguration", "edD53")->Visible=true;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
    {
        EL<TGroupBox>("TfConfiguration", "gbD60")->Visible=true;
        temp=LastSet.dIndexLoadRate[1][1]*100;
        EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position=int(temp);
        temp=LastSet.dIndexLoadRate[0][1]*100;
        EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position   =int(temp);

        EL<TEdit>("TfConfiguration", "edD60_56mm")->Text            =LastSet.dIndexLoadRate[2][1];
        EL<TEdit>("TfConfiguration", "edD60_56mm")->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
        EL<TLabel>("TfConfiguration", "labD60_1")->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
        EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Visible =(CosFunction.bEPUseNSSLK);
        EL<TLabel>("TfConfiguration", "labD60_NS")->Visible          =(CosFunction.bEPUseNSSLK);
        EL<TGroupBox>("TfConfiguration", "gbD60")->Visible              =(CosFunction.bUseDynamicKitDiameter==false);   //Steven 20170605 : 可以自定義Kit直徑
    }
    else
    {
        EL<TGroupBox>("TfConfiguration", "gbD60")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20170328 (Steven) EP 密碼輸入
    {
        EL<TGroupBox>("TfConfiguration", "gbD60")->Enabled=false;
        ;   // UI-only（HTML 端處理）：EL<TGroupBox>("TfConfiguration", "gbD60")->Color  =clRed;
    }

//[E]-------------------------
    //Steven 20090710 Start
    EL<TPanel>("TfConfiguration", "palE30")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE30")->Checked)?true:false;  //Steven 20090904
    EL<TPanel>("TfConfiguration", "palE31")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE31")->Checked)?true:false;  //Steven 20090904
    EL<TPanel>("TfConfiguration", "palE32")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE32")->Checked)?true:false;  //Steven 20090904
    EL<TEdit>("TfConfiguration", "edE30_LodX")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale);
    EL<TEdit>("TfConfiguration", "edE30_HP1X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale[0]);
    EL<TEdit>("TfConfiguration", "edE30_HP2X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale[1]);
    EL<TEdit>("TfConfiguration", "edE30_LodY")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale);
    EL<TEdit>("TfConfiguration", "edE30_HP1Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale[0]);
    EL<TEdit>("TfConfiguration", "edE30_HP2Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale[1]);
//    edE31_Au1X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[0]);
//    edE31_Au2X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[1]);
//    edE31_Au3X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[2]);
//    edE31_Fi1X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[3]);
//    edE31_Fi2X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[4]);
//    edE31_Fi3X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[5]);
//    edE31_Au1Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[0]);
//    edE31_Au2Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[1]);
//    edE31_Au3Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[2]);
//    edE31_Fi1Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[3]);
//    edE31_Fi2Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[4]);
//    edE31_Fi3Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[5]);
    EL<TEdit>("TfConfiguration", "edE32_IS1X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale[0]);
    EL<TEdit>("TfConfiguration", "edE32_IS2X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale[1]);
    EL<TEdit>("TfConfiguration", "edE32_IS1Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale[0]);
    EL<TEdit>("TfConfiguration", "edE32_IS2Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale[1]);
    EL<TEdit>("TfConfiguration", "edE32_OS1X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale[0]);
    EL<TEdit>("TfConfiguration", "edE32_OS2X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale[1]);
    EL<TEdit>("TfConfiguration", "edE32_OS1Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale[0]);
    EL<TEdit>("TfConfiguration", "edE32_OS2Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale[1]);
    //Steven 20090717 End

    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfConfiguration", "pgcScale")->ActivePage=EL<TTabSheet>("TfConfiguration", "tsE30");
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfConfiguration", "pgcE31")->ActivePage=EL<TTabSheet>("TfConfiguration", "tsE31_1");

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
    {
        EL<TEdit>("TfConfiguration", "edE30_1_LodX")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale_Hot) ;
        EL<TEdit>("TfConfiguration", "edE30_1_HP1X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Hot[0]);
        EL<TEdit>("TfConfiguration", "edE30_1_HP2X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Hot[1]);
        EL<TEdit>("TfConfiguration", "edE30_1_LodY")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale_Hot) ;
        EL<TEdit>("TfConfiguration", "edE30_1_HP1Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Hot[0]);
        EL<TEdit>("TfConfiguration", "edE30_1_HP2Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Hot[1]);

        EL<TEdit>("TfConfiguration", "edE32_1_IS1X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Hot[0]) ;
        EL<TEdit>("TfConfiguration", "edE32_1_IS2X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Hot[1]) ;
        EL<TEdit>("TfConfiguration", "edE32_1_IS1Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Hot[0]) ;
        EL<TEdit>("TfConfiguration", "edE32_1_IS2Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Hot[1]) ;
        EL<TEdit>("TfConfiguration", "edE32_1_OS1X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Hot[0]);
        EL<TEdit>("TfConfiguration", "edE32_1_OS2X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Hot[1]);
        EL<TEdit>("TfConfiguration", "edE32_1_OS1Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Hot[0]);
        EL<TEdit>("TfConfiguration", "edE32_1_OS2Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Hot[1]);

        EL<TEdit>("TfConfiguration", "edE30_2_LodX")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale_Cold) ;
        EL<TEdit>("TfConfiguration", "edE30_2_HP1X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Cold[0]);
        EL<TEdit>("TfConfiguration", "edE30_2_HP2X")->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Cold[1]);
        EL<TEdit>("TfConfiguration", "edE30_2_LodY")->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale_Cold) ;
        EL<TEdit>("TfConfiguration", "edE30_2_HP1Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Cold[0]);
        EL<TEdit>("TfConfiguration", "edE30_2_HP2Y")->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Cold[1]);

        EL<TEdit>("TfConfiguration", "edE32_2_IS1X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Cold[0]) ;
        EL<TEdit>("TfConfiguration", "edE32_2_IS2X")->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Cold[1]) ;
        EL<TEdit>("TfConfiguration", "edE32_2_IS1Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Cold[0]) ;
        EL<TEdit>("TfConfiguration", "edE32_2_IS2Y")->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Cold[1]) ;
        EL<TEdit>("TfConfiguration", "edE32_2_OS1X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Cold[0]);
        EL<TEdit>("TfConfiguration", "edE32_2_OS2X")->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Cold[1]);
        EL<TEdit>("TfConfiguration", "edE32_2_OS1Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Cold[0]);
        EL<TEdit>("TfConfiguration", "edE32_2_OS2Y")->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Cold[1]);
    }
    else
    {
        EL<TPanel>("TfConfiguration", "pnlE30_Hot")->Visible =false;
        EL<TPanel>("TfConfiguration", "pnlE30_Cold")->Visible=false;
        EL<TTabSheet>("TfConfiguration", "tsE31_Hot")->TabVisible=false;
        EL<TTabSheet>("TfConfiguration", "tsE31_Cold")->TabVisible=false;
        EL<TPanel>("TfConfiguration", "pnlE32_Hot")->Visible=false;
        EL<TPanel>("TfConfiguration", "pnlE32_Cold")->Visible=false;
    }

//[F]-------------------------
    if(Cylinder[C_Shuttle_Knocker_1].Enable==false &&
       Cylinder[C_Shuttle_Knocker_2].Enable==false)                             //Steven 20200522 : 氣缸沒裝就不要啟用功能
    {
        EL<TPanel>("TfConfiguration", "pnlF14")->Visible=false;
    }

//[G]-------------------------

//[H]-------------------------

//[I]-------------------------
    if(CosFunction.bHaveFIFOMode)
    {
        AnsiString Dir;
        EL<TControl>("TfConfiguration", "imgI37_3")->Tag=IniConfig.iI37_LockLoaderDirection;
        Dir.sprintf("%stype%d.bmp", BmpPath, IniConfig.iI37_LockLoaderDirection);
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:4884-4892 -- imgI37_3->Picture->LoadFromFile：圖片（HTML 端依 tag 顯示 type%d.bmp）
        try
        {
            ;   // UI-only（HTML 端處理）：EL<TControl>("TfConfiguration", "imgI37_3")->Picture->LoadFromFile(Dir);
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfConfiguration::FormShow");
            filerw::ELMessage("''Path :D:\\HT9045\\IMG\\BMP\\'' be delete");
        }
#endif // GATE (S12-C save)
    }

#if 0 // GATE (S12-C save) golden cConfiguration.cpp:4895-4899 -- rgI22->Controls[i]->Enabled：單一選項停用（替身沒有子選項）
    if(CosFunction.bTestTimeOutShowSkipAndHome)
    {
        EL<TRadioGroup>("TfConfiguration", "rgI22")->Controls[1]->Enabled=false;
        EL<TRadioGroup>("TfConfiguration", "rgI22")->Controls[2]->Enabled=false;
    }
#endif // GATE (S12-C save)
//[J]-------------------------

//[K]-------------------------

//[L]-------------------------
    //jou 980928 start : High tempture 130 deg Position shift
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            if(abs(LastSet.TempPosShift_Shuttle[i][j])>1001)
                LastSet.TempPosShift_Shuttle[i][j]=0;
        }
    }
    EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text=LastSet.TempPosShift_Shuttle[0][0];
    EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text=LastSet.TempPosShift_Shuttle[0][1];
    EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text=LastSet.TempPosShift_Shuttle[1][0];
    EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text=LastSet.TempPosShift_Shuttle[1][1];

    //Steven 20110214 Start
    if(FIX3_FULL_PLACE==Fix3K_ShortShuttle)
    {
        EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text=15;
        EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text=90;
        EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text=15;
        EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text=90;
    }
    else if(ATC_SYSTEM>eATCUninstall)                                           //wei 20160329 ATC模式 Shuttle偏移
    {
        EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text=15;
        EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text=60;
        EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text=15;
        EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text=60;
    }
    else
    {
        EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Text=40;
        EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Text=130;
        EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Text=40;
        EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Text=130;
    }
    //Steven 20110214 End
    //jou 980928 end
    EL<TGroupBox>("TfConfiguration", "gbL11")->Visible=(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber);              //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    if(Temperature.bUseTC2Offset)                                               //KenHsieh 20240311 : add Tc2 Offset
    {
        EL<TCheckBox>("TfConfiguration", "cbL11_6")->Checked=false;
        EL<TCheckBox>("TfConfiguration", "cbL11_7")->Checked=false;
        EL<TCheckBox>("TfConfiguration", "cbL11_8")->Checked=false;
        IniConfig.bL11_6ATCUseTemperatureOutsideAlarm=false;
        IniConfig.bL11_7ATCUseMaxSurgeAlarm=false;
        IniConfig.bL11_8ATCUseTemperatureCompare=false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung) { IniConfig.bL43EnableATCPowerFollow=false; EL<TCheckBox>("TfConfiguration", "cbL43")->Checked=IniConfig.bL43EnableATCPowerFollow; EL<TCheckBox>("TfConfiguration", "cbL43")->Visible=false; }   // golden cConfiguration.cpp:4954-4978 取代：ATC_InterfaceForm->asATC_SW_Ver：移植樹 ATC 門面沒有版本字串 → 等同 golden 字串為空，走 :4972-4977 else 支（審查 M-A）
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:4954-4978 -- ATC_InterfaceForm->asATC_SW_Ver：移植樹 ATC 門面沒有版本字串 → 等同 golden 字串為空，走 :4972-4977 else 支（審查 M-A）
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(ATC_SYSTEM==eNewATCSystem &&
           ATC_InterfaceForm->asATC_SW_Ver!="")                                 //KenHsieh 20240216 : add ATC Power Follow Function
        {
            iATCVerBuf=ATC_InterfaceForm->asATC_SW_Ver.Length()-5;
            str=ATC_InterfaceForm->asATC_SW_Ver.SubString(1, 3+iATCVerBuf);
            if(atof(str.c_str())>=2.5)
            {
                EL<TCheckBox>("TfConfiguration", "cbL43")->Visible=true;
            }
            else
            {
                IniConfig.bL43EnableATCPowerFollow=false;
                EL<TCheckBox>("TfConfiguration", "cbL43")->Checked=IniConfig.bL43EnableATCPowerFollow;
                EL<TCheckBox>("TfConfiguration", "cbL43")->Visible=false;
            }
        }
        else
        {
            IniConfig.bL43EnableATCPowerFollow=false;
            EL<TCheckBox>("TfConfiguration", "cbL43")->Checked=IniConfig.bL43EnableATCPowerFollow;
            EL<TCheckBox>("TfConfiguration", "cbL43")->Visible=false;
        }
    }
#endif // GATE (S12-C save)

    if(Tri_Temp_Machine!=1)
    {
        EL<TGroupBox>("TfConfiguration", "grpL31")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL32")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL33")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL34")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL35")->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL37")->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL37") ->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL38") ->Visible=false;
        EL<TGroupBox>("TfConfiguration", "grpL39")->Visible=false;
        EL<TLabel>("TfConfiguration", "labL40")->Visible=false;
        EL<TEdit>("TfConfiguration", "edL40") ->Visible=false;
        EL<TLabel>("TfConfiguration", "labL41")->Visible=false;
        EL<TEdit>("TfConfiguration", "edL41") ->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL42") ->Visible=false;
        EL<TEdit>("TfConfiguration", "edtL42")->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL46") ->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL44") ->Visible=false;
        EL<TEdit>("TfConfiguration", "edtL44")->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbL45") ->Visible=false;
        EL<TEdit>("TfConfiguration", "edtL45")->Visible=false;
        EL<TEdit>("TfConfiguration", "edtL09_1")->Visible=false;
        EL<TEdit>("TfConfiguration", "edtL09_2")->Visible=false;
    }
//[M]-------------------------
    bM01Enter=true;
    EL<TGroupBox>("TfConfiguration", "gbM01")->Visible=EL<TCheckBox>("TfConfiguration", "cbM01")->Checked;
    bM01Enter=false;

//[N]-------------------------
    EL<TPageControl>("TfConfiguration", "pcN00")->Visible                  =false;                                     //Steven 20110421

    if(CosFunction.bHiSiliconFunction  ||
       CosFunction.bUseLogUploadToFTPFunction ||
       CosFunction.bHisiLogUploadNetwork ||
       CUSTOMER_CODE==CC_ASE_KaohSiung ||                                       //kevin 20131009 高雄日月光強制TRUE
       CUSTOMER_CODE==CC_GM_TEST       ||                                       //Steven 20220520 : Add GM Test
       CUSTOMER_CODE==CC_ChipMos_ZHUBEI)
    {
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible              =true;
    }

    EL<TTabSheet>("TfConfiguration", "tsN05")->TabVisible               =false;                                     //wei 20150511
    EL<TGroupBox>("TfConfiguration", "gbN05_WebService")->Visible       =(CUSTOMER_CODE==CC_SCK);                   //Steven 20161201 : For SCK Web Service
    EL<TTabSheet>("TfConfiguration", "tsN06")->TabVisible               =false;                                     //wei 20150511
    EL<TTabSheet>("TfConfiguration", "tsN07")->TabVisible               =false;                                     //wei 20150511

    EL<TEdit>("TfConfiguration", "edN04_Model")->Text=IniConfig.sMachineType;
    EL<TEdit>("TfConfiguration", "edN04_ID")->Text   =IniConfig.SocketHandlerID;
    EL<TEdit>("TfConfiguration", "edtN04_TesterID")->Text           =IniConfig.RMSTesterID;                     //Ifor 20231219 add Tester ID

    if(CUSTOMER_CODE==CC_ASE_CL || IniConfig.bFTPJamCodeUpload)                 //Steven 20110210
    {
        if(MachineTypeChoice==Type_HT9045)                                      //Steven 20110708
            EL<TEdit>("TfConfiguration", "edN04_Model")->Text="HT9045";
        else if(MachineTypeChoice==Type_HT9045_12Site)                          //ChungHung 20130507 add HT9045 updata for 12site 517
            EL<TEdit>("TfConfiguration", "edN04_Model")->Text="HT9045_12Site";
        else
            EL<TEdit>("TfConfiguration", "edN04_Model")->Text="HT9046";

        EL<TEdit>("TfConfiguration", "edN04_Model")->Enabled=false;
        EL<TEdit>("TfConfiguration", "edN04_ID")->Text=AnsiString(PC_NAME);
        EL<TEdit>("TfConfiguration", "edN04_ID")->Enabled=false;
    }

    EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked=false;                                             //Steven 20110304 : 防呆
    EL<TGroupBox>("TfConfiguration", "gbN05_1")->Visible        =(CosFunction.bUseERMS);                            //Steven 20160711 : 使用進階版RMS

    EL<TGroupBox>("TfConfiguration", "groupbN05_RTC")->Visible=CosFunction.bRTCalarmUnload;                         //jou 20170210 (Steven) : RTC alarm image unload

    EL<TCheckBox>("TfConfiguration", "cbN06_EnableFTP")->Checked=false;                                             //Steven 20110304 : 防呆

    if(IC_EnableRMSFunc())
    {
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible          =true;                                          //Steven 20110421
        EL<TTabSheet>("TfConfiguration", "tsN05")->TabVisible       =true;
        EL<TGroupBox>("TfConfiguration", "gbN05")->Caption          ="[N05] RMS Setting for Recipe File";
        EL<TLabel>("TfConfiguration", "labN05_1")->Caption       ="Upload Path:";
        EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Caption="Enable RMS Connection";
        EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked=IniConfig.bEnableRms;
        EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Text     =IniConfig.sRmsPath;
        EL<TEdit>("TfConfiguration", "edN05_DownPath")->Text    =IniConfig.sRmsDownPath;

        EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Text     =IniConfig.fAmbientTemp;                        //Steven 20110421
        EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Checked=IniConfig.bCheckFile;                          //Steven 20110421
        EL<TCheckBox>("TfConfiguration", "chkN05_TrayFeedClear")->Checked=IniConfig.bClearLotInfoWhenTrayFeed;      //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
        EL<TButton>("TfConfiguration", "btnUploadAll")->Caption="Upload All";
        EL<TCheckBox>("TfConfiguration", "cbN05_CheckSetupFile")->Checked=IniConfig.bEnableRmsCheckSetupFile;       //Ifor 20230516 add: TFAMD 要求工作檔驗證
    }
    else if(IniConfig.bShowLotInfo)
    {
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible          =true;                                          //Steven 20110421
        if(CUSTOMER_CODE!=CC_TSMC_TAINAN &&                                     //wei 20161102 TSMC N05不顯示
           CUSTOMER_CODE!=CC_ASE_M &&                                           //Ifor 20170703 (wei) add ASEM 不顯示N05
           CUSTOMER_CODE!=CC_GIGAS)                                             //Isaac 20210128 : add 全智不顯示N05
            EL<TTabSheet>("TfConfiguration", "tsN05")->TabVisible       =true;

        if(CosFunction.bDownloadRecipeLevelMode==true)                          //jou 2016-01-06 download recipe 增加權限模式選擇
        {
            EL<TLabel>("TfConfiguration", "labN05_DownPath")->Caption="Engineer Path:";
        }
        else
        {
            EL<TEdit>("TfConfiguration", "edN05_DownPath")->Visible =false;
            EL<TLabel>("TfConfiguration", "labN05_DownPath")->Visible=false;
        }

        EL<TGroupBox>("TfConfiguration", "gbN05")->Caption          ="[N05] Server Setting for Recipe File";
        EL<TLabel>("TfConfiguration", "labN05_1")->Caption       ="Server Path:";
        EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Caption="Enable Network Connection";
        EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Checked=IniConfig.bEnableRms;
        EL<TCheckBox>("TfConfiguration", "cbN05_CheckSetupFile")->Checked=IniConfig.bEnableRmsCheckSetupFile;       //Ifor 20230516 add: TFAMD 要求工作檔驗證

        EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Text     =IniConfig.sRmsPath;
        EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Text     =IniConfig.fAmbientTemp;
        EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Checked=IniConfig.bCheckFile;

        if(IniConfig.bVTESTFunction==true)
        {
            EL<TLabel>("TfConfiguration", "labN05_AmbTemp")->Visible=false;
            EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Visible=false;
            EL<TButton>("TfConfiguration", "btnMesSystem")->Visible=true;
            #ifndef SOFT_SIMULTE                                                //RogerYang 20250218 軟體模擬可以修改
            EL<TCheckBox>("TfConfiguration", "cbN05_EnableRMS")->Enabled=false;
            EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Enabled=false;
            #endif
        }

        EL<TCheckBox>("TfConfiguration", "cbN05_1")->Checked        =IniConfig.bEnableErms;
        EL<TEdit>("TfConfiguration", "edN05_1")->Text           =IniConfig.sErmsPath;

        EL<TCheckBox>("TfConfiguration", "checkbN05_RTC")->Checked  =IniConfig.bN05_RTCalarmUnload;                 //jou 20170210 (Steven) : RTC alarm image unload
        EL<TEdit>("TfConfiguration", "editN05_RTC")->Text       =IniConfig.asN05_RTCalarmUnload;                //jou 20170210 (Steven) : RTC alarm image unload

        if(CosFunction.bDownloadRecipeLevelMode)                                //jou 2016-01-06 download recipe 增加權限模式選擇
            EL<TEdit>("TfConfiguration", "edN05_DownPath")->Text=IniConfig.sRmsDownPath;

        EL<TCheckBox>("TfConfiguration", "chkN05_TrayFeedClear")->Checked=IniConfig.bClearLotInfoWhenTrayFeed;      //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        EL<TTabSheet>("TfConfiguration", "tsN05")->TabVisible     =false;                                           //Ifor 20161229 (Steven) KYEC 喬智要求關閉N05 頁面及功能
        EL<TLabel>("TfConfiguration", "labN07_5")->Visible     =true;
        EL<TEdit>("TfConfiguration", "edN07_5")->Visible      =true;
    }
    else
    {
        EL<TLabel>("TfConfiguration", "labN07_5")->Visible     =false;
        EL<TEdit>("TfConfiguration", "edN07_5")->Visible      =false;
    }

    if(CosFunction.bFTPFunction)                                                //Steven 20091004 for KYEC 29818 : [I16] Never Enable and Always True    //Steven 20140609 : else if --> if
    {
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible              =true;                                      //Steven 20110421
        EL<TTabSheet>("TfConfiguration", "tsN06")->TabVisible           =true;
        EL<TCheckBox>("TfConfiguration", "cbN06_EnableFTP")->Checked    =IniConfig.bEnableFTP;
        EL<TEdit>("TfConfiguration", "edN06_UserName")->Text        =IniConfig.FtpUserName;
        EL<TEdit>("TfConfiguration", "edN06_Password")->Text        =IniConfig.FtpPassword;
        EL<TEdit>("TfConfiguration", "edN06_HostName")->Text        =IniConfig.FtpHost;
        EL<TEdit>("TfConfiguration", "edN06_DownPath")->Text        =IniConfig.FtpDownloadPath;
        EL<TEdit>("TfConfiguration", "edN06_UpLdPath")->Text        =IniConfig.FtpUplaodPath;
        EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->ItemIndex   =IniConfig.iHDEnable;
        EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->ItemIndex   =IniConfig.iServerEnable;
        if(CUSTOMER_CODE==CC_JSCC_OS)                                           //長電微電子 (JSCC OS部門)
        {
            EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Text   =IniConfig.asN06_TesterPath;
        }
        else
        {
            EL<TEdit>("TfConfiguration", "edN06_TestList")->Text    =IniConfig.N06_TasterListFile;
            EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Text   =IniConfig.N06_TasterListMap;
        }
        EL<TEdit>("TfConfiguration", "edN06_Port")->Text            =IniConfig.N06_FtpPort;                     //Ifor 20201015 add:使用者自定義 FTP Port
        filerw::ELComboIndex(EL<TComboBox>("TfConfiguration", "cbbN06_Mode"), IniConfig.FtpTransMode);                    //Steven 20230719 : 加入FTP傳輸模式

        if(CosFunction.PassworDownloadByFTP)                                    //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
        {
            EL<TCheckBox>("TfConfiguration", "cbN06_1")->Visible=true;
            EL<TCheckBox>("TfConfiguration", "cbN06_1")->Checked=IniConfig.bFtpPasswordDownload;
            EL<TEdit>("TfConfiguration", "edN06_1")->Visible=true;
            EL<TEdit>("TfConfiguration", "edN06_1")->Text=IniConfig.FtpPasswordDownloadPath;
        }
        else
        {
            EL<TCheckBox>("TfConfiguration", "cbN06_1")->Visible=false;
            EL<TCheckBox>("TfConfiguration", "cbN06_1")->Checked=false;
            EL<TEdit>("TfConfiguration", "edN06_1")->Visible=false;
        }

        EL<TLabel>("TfConfiguration", "labN06_TesterMap")->Visible           =false;
        EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Visible            =false;
        EL<TButton>("TfConfiguration", "btN06_TesterMap")->Visible            =false;
        EL<TLabel>("TfConfiguration", "labN06_TesterList")->Visible          =false;
        EL<TEdit>("TfConfiguration", "edN06_TestList")->Visible             =false;
        EL<TButton>("TfConfiguration", "btN06_TesterList")->Visible           =false;
        EL<TButton>("TfConfiguration", "btN06_UpdateTesterList")->Visible     =false;

        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            EL<TLabel>("TfConfiguration", "labN05_1")->Visible               =false;
            EL<TEdit>("TfConfiguration", "edN05_RmsPath")->Visible          =false;
            EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Visible        =false;
            EL<TCheckBox>("TfConfiguration", "cbN05_CheckFile")->Enabled        =false;
            EL<TLabel>("TfConfiguration", "labN05_AmbTemp")->Visible         =false;
            EL<TEdit>("TfConfiguration", "edN05_AmbTemp")->Visible          =false;
        }
        else if(CUSTOMER_CODE==CC_JSCC_OS)                                      //長電微電子 (JSCC OS部門)
        {
            EL<TLabel>("TfConfiguration", "labN06_TesterMap")->Visible       =true;
            EL<TLabel>("TfConfiguration", "labN06_TesterMap")->Caption       ="OS Tester Path:";
            EL<TEdit>("TfConfiguration", "edN06_TesterMap")->Visible        =true;
        }
        else if(CUSTOMER_CODE==CC_PTI)                                          //RogerYang 20170406 (Steven) 力成 add FTP upload for Jam Alarm Data
        {
            EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->Visible         =false;
            EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->Visible         =false;
            EL<TLabel>("TfConfiguration", "labN06_DownloadPath")->Visible    =false;
            EL<TEdit>("TfConfiguration", "edN06_DownPath")->Visible         =false;
            EL<TLabel>("TfConfiguration", "labN06_FileName")->Visible        =true;
            EL<TEdit>("TfConfiguration", "edN06_FileName")->Visible         =true;
            EL<TEdit>("TfConfiguration", "edN06_FileName")->Text            =IniConfig.asN06_FileName;
        }
        else if(CUSTOMER_CODE==CC_Greatek)                                      //Sam 20170815 (Steven) add 超豐不顯示TesterMap
        {
            EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->Visible         =false;
            EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->Visible         =false;
        }
    }

    EL<TCheckBox>("TfConfiguration", "cbN06_UseSystemCall")->Visible=false;

    if(IniConfig.bFTPJamCodeUpload)                                             //ChungHung 20140108 add FTP unload jam code
    {
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible              =true;                                      //Steven 20110421
        EL<TTabSheet>("TfConfiguration", "tsN06")->TabVisible           =true;
        EL<TCheckBox>("TfConfiguration", "cbN06_EnableFTP")->Checked    =IniConfig.bEnableFTP;
        EL<TEdit>("TfConfiguration", "edN06_UserName")->Text        =IniConfig.FtpUserName;
        EL<TEdit>("TfConfiguration", "edN06_Password")->Text        =IniConfig.FtpPassword;
        EL<TEdit>("TfConfiguration", "edN06_HostName")->Text        =IniConfig.FtpHost;
        EL<TEdit>("TfConfiguration", "edN06_DownPath")->Text        =IniConfig.FtpDownloadPath;
        EL<TEdit>("TfConfiguration", "edN06_UpLdPath")->Text        =IniConfig.FtpUplaodPath;
        EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->ItemIndex   =IniConfig.iHDEnable;
        EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->ItemIndex   =IniConfig.iServerEnable;
        EL<TEdit>("TfConfiguration", "edN06_Port")->Text            =IniConfig.N06_FtpPort;                     //Ifor 20201015 add:使用者自定義 FTP Port

        EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        EL<TButton>("TfConfiguration", "btN06_UpdateTesterList")->Visible =false;                                 //ChungHung 20140108 add FTP unload jam code
        EL<TLabel>("TfConfiguration", "labN06_DownloadPath")->Visible    =false;                                 //ChungHung 20140108 add FTP unload jam code
        EL<TEdit>("TfConfiguration", "edN06_DownPath")->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        EL<TLabel>("TfConfiguration", "labN06_UploadPath")->Caption      = "Server Path:";                       //ChungHung 20140108 add FTP unload jam code
    }

    if(CosFunction.bEnable_SECS_GEM)                                            //ChungHung 20150515 add for SECS GEM 顯示
    {
        bSecsGemStatus=IniConfig.bEnable_SECS_GEM;                              //wei 20160308 開啟SECSGEM功能
        EL<TPageControl>("TfConfiguration", "pcN00")->Visible                  =true;                                  //wei 20150511
        EL<TTabSheet>("TfConfiguration", "tsN07")->TabVisible               =(CosFunction.bEnable_SECS_GEM);        //Steven 20141006 : SECS GEM使用Remote Start功能
        EL<TPanel>("TfConfiguration", "pnlN07_2")->Visible               =(CosFunction.bRCMDStart);
    }
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5242-5242 -- FormHS->GetUploadServerByFTPPath：顯示用路徑（移植樹沒有 FormHS）
    EL<TLabel>("TfConfiguration", "lblUploadRealPath")->Caption=FormHS->GetUploadServerByFTPPath("EventLog", true);
#endif // GATE (S12-C save)

//[O]-------------------------
    EL<TGroupBox>("TfConfiguration", "gbO06")->Visible=false;
    if(IniConfig.bEventLogAutoSaveFunction)                                     //Steven 20110221 Start : EventLogAutoSave
    {
        EL<TGroupBox>("TfConfiguration", "gbO06")->Visible              =true;
        //AI(ht9045-v899) 20260623: 界外/空日期(1899-12-30)防呆,無效時回填Now()避免picker指派拋例外
        TDateTime dtO06Load = ((double)IniConfig.dtEventLogLastRecordDate < 2.0) ? Now() : IniConfig.dtEventLogLastRecordDate;
        EL<filerw::ELDateTimePicker>("TfConfiguration", "dtO06_LastDate")->Date        =dtO06Load;
        EL<filerw::ELDateTimePicker>("TfConfiguration", "dtpO06NextTime")->Time        =dtO06Load;
        for(int i=0; i<7; i++)
        {
            if(IniConfig.bAutoSaveLogWeek[i]==true)
                EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[i][1]="On";
            else
                EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[i][1]="";
        }
    }
    EL<TCheckBox>("TfConfiguration", "cbO08")->Visible=false;                                                       //Steven 20140221 : Mark

    if(CosFunction.bUseHeadContactCount==true)                                  //Ifor 20191128 : Fix 銦片 Life Time 功能無顯示
    {
        EL<TPanel>("TfConfiguration", "paLifeTime")->Visible=!(CUSTOMER_CODE==CC_SCC);
    }

//[P]-------------------------
    #ifndef SOFT_SIMULTE
    EL<TPanel>("TfConfiguration", "pnlP13")->Visible=(CosFunction.bKnockerSetBySetupFile==false);                //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    EL<TPanel>("TfConfiguration", "pnlP16")->Visible=(CosFunction.bKnockerSetBySetupFile==false &&               //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
                     Cylinder[C_HotplateVibration].Enable==true);               //Steven 20120510 : 全部改用汽缸的Enable判斷
    #endif

    IC_WriteContactData();                                                         //kevin 20170329 (Steven) 將CONTRACT 資料寫到INI 給 客戶檢查

    if(INSTALL_OCR!=eocrUninstal)                                               //ChungHung 20121002 add OCR Function
        EL<TGroupBox>("TfConfiguration", "gbP23_OCR")->Visible=true;
    else
        EL<TGroupBox>("TfConfiguration", "gbP23_OCR")->Visible=false;

    if(IniConfig.iOCRConditions==0)
        EL<TRadioButton>("TfConfiguration", "rgP23_OCRByNewTray")->Checked=true;
    else if(IniConfig.iOCRConditions==1)
        EL<TRadioButton>("TfConfiguration", "rgP23_OCRByInitialStart")->Checked=true;
    else
        EL<TRadioButton>("TfConfiguration", "rgP23_OCRByNewTray")->Checked=true;

    EL<TGroupBox>("TfConfiguration", "gbP26_OCRCheck")->Visible=CosFunction.bTrayOCR;                               //wei 20161125 因修改不使用IniConfig.iOCRConditions判斷OCR模式，所以移除
    for(int i=0; i<10; i++) { TEdit* p_=EL<TEdit>("TfConfiguration", (AnsiString("OCRTrayLot")+AnsiString(i)).c_str()); p_->Text=AnsiString(LastSet.TrayCount[i]); p_->Tag=i; }   // golden cConfiguration.cpp:5290-5294 取代：//AI(W906-FRW-Q19) 20260927: edOCRTrayLot[i]（golden 建構子 :208 new，Name＝"OCRTrayLot"+i）→ 具名替身 OCRTrayLot<i>；Text=LastSet.TrayCount[i]（int → AnsiString，同 golden 隱含轉換）、Tag=i
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5290-5294 -- //AI(W906-FRW-Q19) 20260927: edOCRTrayLot[i]（golden 建構子 :208 new，Name＝"OCRTrayLot"+i）→ 具名替身 OCRTrayLot<i>；Text=LastSet.TrayCount[i]（int → AnsiString，同 golden 隱含轉換）、Tag=i
    for(int i=0; i<10; i++)                                                     //wei 20151117 OCR Lot check
    {
        edOCRTrayLot[i]->Text=LastSet.TrayCount[i];
        edOCRTrayLot[i]->Tag=i;
    }
#endif // GATE (S12-C save)

    EL<TGroupBox>("TfConfiguration", "gbUnloadMode")->Visible=(CosFunction.bUnloadTrayModeByRecipe);                //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
//------------------------------------------------------------------------------

#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5299-5300 -- 表單 Left／Top：視窗位置
    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)
    bEnterOffset=true;

    if(CUSTOMER_CODE==CC_MTI)                                                   //Steven 20110131
    {
        EL<TButton>("TfConfiguration", "btResume")->Visible=true;
    }
    else
    {
        EL<TButton>("TfConfiguration", "btResume")->Visible=false;
    }

    EL<TControl>("TfConfiguration", "Timer1")->Enabled=true;

//請保持在最下面--------------------------------------------------------------
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5315-5315 -- ShowMemo()：說明欄
    ShowMemo();                                                                 //Steven 20120218 : 這行得在所有的Visible完成後才能執行
#endif // GATE (S12-C save)
    //Steven 20120807 Start: 在最下面決定權限能不能Enable                       //Steven 20140805 : 低權限可以看不能改
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_A1"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);  //JerryYang 20190928 為了讓[A30] Setup function不被鎖住
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_A2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_A3"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_A4"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_C1"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[68])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_C2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[68])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D3"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D4"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D5"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D6"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D7"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_D8"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPageControl>("TfConfiguration", "pgcScale"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[3]==false)?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_E2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[11]==false)?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_E5"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[11]==false)?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_F00"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_F10"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_F20"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_G"),  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[72])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_I01"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_I20"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_I30"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_I40"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_L1"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_L2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_L3"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_O1"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[75])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_O2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[75])?false:true);
    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210128 : 為了讓Enable FTP不被鎖住
    {
        filerw::ELChangeCompomentEnabled(EL<TGroupBox>("TfConfiguration", "gbN04")  , (AccessLevel<LevelSet.AccessLevel[76])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN05")  , (AccessLevel<LevelSet.AccessLevel[76])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_UserName")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_Password")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_HostName")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_DownPath")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_UpLdPath")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_Port")      , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_TesterMap") , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TEdit>("TfConfiguration", "edN06_TestList")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TRadioGroup>("TfConfiguration", "gbN06_HddLevel")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TRadioGroup>("TfConfiguration", "gbN06_ServerLv")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);

        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN07")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN08")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN09")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN10")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN11")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN12")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN13")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN14")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN15")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN16")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TGroupBox>("TfConfiguration", "grpN20") , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN22")  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
    }
    else
    {
        filerw::ELChangeCompomentEnabled(EL<TTabSheet>("TfConfiguration", "tsN00"),  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[76])?false:true);
    }
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_P0"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_P1"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_P2"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_P3"), (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pnlTray"),(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[78])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pnlHP"),  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[79])?false:true);
    filerw::ELChangeCompomentEnabled(EL<TPanel>("TfConfiguration", "pal_M"),  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[96])?false:true);
    //Steven 20120807 End: 在最下面決定權限能不能Enable
//    CheckEnabled(); //jou 2012-06-27 必須在ChangeCompomentEnabled之後         //Steven 20120912 : 會發生被Disable後,無法Enable的狀況

    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       (CUSTOMER_CODE==CC_KYEC_XILINX && IniConfig.bChangeKitNoHardStop==true && IniConfig.bRemeberAutoHeight==true))   //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    {
        EL<TPanel>("TfConfiguration", "palD01")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfConfiguration", "palD01")->Visible=false;
    }
//    gbL09->Enabled=false;                                                     //JerryYang 20230131 : L09功能可以開關
    EL<TEdit>("TfConfiguration", "edL09_Sh1L")->Enabled=false;
    EL<TEdit>("TfConfiguration", "edL09_Sh1R")->Enabled=false;
    EL<TEdit>("TfConfiguration", "edL09_Sh2L")->Enabled=false;
    EL<TEdit>("TfConfiguration", "edL09_Sh2R")->Enabled=false;

//    pal_A1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_A2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;       //wei 20160225 分頁
//    pal_C->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D3->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D4->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D5->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D6->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_E1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_E2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_F00->Enabled=(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_F10->Enabled=(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_G->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_I->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_L1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_L2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_O1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_O2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_N->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_P->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_M->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;

    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[0][0]="Sun";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[1][0]="Mon";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[2][0]="Tue";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[3][0]="Wed";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[4][0]="Thu";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[5][0]="Fri";
    EL<filerw::ELStringGrid>("TfConfiguration", "strngrdAutoSaveLog")->Cells[6][0]="Sat";

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160101 海思專用版本 顯示不可修改溫度
    {
        EL<TPanel>("TfConfiguration", "pal_L1")->Enabled=false;
        EL<TPanel>("TfConfiguration", "pal_L2")->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_JCET)
    {
        EL<TButton>("TfConfiguration", "btN06_UpdateTesterList")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //KaiChen 20200623 ：矽格北興，不給修改
    {
        //Sam 20220706 : 北興俊堯說可以再打開修改 Mark
        //==>
        //cbP13       ->Enabled=false;
        //edP13_1     ->Enabled=false;
        //edP13_2     ->Enabled=false;
        //<==
        //Sam 20220706 : 北興俊堯說可以再打開修改 Mark
        EL<TCheckBox>("TfConfiguration", "cbP14")       ->Enabled=false;
        EL<TEdit>("TfConfiguration", "edP14_1")     ->Enabled=false;
        EL<TEdit>("TfConfiguration", "edP14_2")     ->Enabled=false;
    }

    if(CosFunction.bUseConfigSaveButton)    //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    {
        EL<TButton>("TfConfiguration", "btnSave")->Visible=true;
        bSave=false;
    }
    else
    {
        EL<TButton>("TfConfiguration", "btnSave")->Visible=false;
    }
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, "fConfiguration", asLogPath);                        //kevin 20181025 change//Steven 20100629

}

// golden cConfiguration.cpp:6336  TfConfiguration::tbD25_Index30mmChange(TObject *Sender)
static void IC_tbD25_Index30mmChange()
{

    AnsiString buffer="30mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="28mm : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="40mm : ";
    else
        buffer="30mm : ";

   EL<TLabel>("TfConfiguration", "labD25_3")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position/100.0);
    if(old30data==0.0)
        old30data=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position;

    if(old30data!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position)
    {
        aEP30OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old30data, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position);   //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP30DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag

        if(old30data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old30data=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position=115;                                      //kevin 20170609 (wei) add EP 保護
        }

        if(old30data<85)
        {
            old30data=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm")->Position=85;                                       //kevin 20170609 (wei) add EP 保護
        }
    }

}

// golden cConfiguration.cpp:6622  TfConfiguration::tbD25_Index30mm_NSChange(TObject *Sender)
static void IC_tbD25_Index30mm_NSChange()
{

    AnsiString buffer="28mm for NS:";                                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="28mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="40mm for NS:";

    EL<TLabel>("TfConfiguration", "labD25_3_NS")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position/100.0);
    if(old30data_NS==0.0)
        old30data_NS=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position;

    if(old30data_NS!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position)
    {
        aEP30OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old30data_NS, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position);  //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP30DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old30data_NS>=115)
        {
            old30data_NS=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old30data_NS<=85)
        {
            old30data_NS=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index30mm_NS")->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }

}

// golden cConfiguration.cpp:6303  TfConfiguration::tbD25_Index40mmChange(TObject *Sender)
static void IC_tbD25_Index40mmChange()
{

    AnsiString buffer="40mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="40mm : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="55mm : ";
    else
        buffer="40mm : ";
    EL<TLabel>("TfConfiguration", "labD25_2")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position/100.0);

    if(old40data==0.0)
        old40data=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position;

    if(old40data!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position)
    {
        aEP40OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old40data, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position); //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP40DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag

        if(old40data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old40data=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position=115;                                      //kevin 20170609 (wei) add EP 保護
        }

        if(old40data<85)
        {
            old40data=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm")->Position=85;                                       //kevin 20170609 (wei) add EP 保護
        }
    }

}

// golden cConfiguration.cpp:6591  TfConfiguration::tbD25_Index40mm_NSChange(TObject *Sender)
static void IC_tbD25_Index40mm_NSChange()
{

    AnsiString buffer="40mm for NS: ";                                          //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="40mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="55mm for NS:";

    EL<TLabel>("TfConfiguration", "labD25_2_NS")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position/100.0);

    if(old40data_NS==0.0)
        old40data_NS=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position;

    if(old40data_NS!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position)
    {
        aEP40OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old40data_NS, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position);  //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP40DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old40data_NS>=115)
        {
            old40data_NS=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old40data_NS<=85)
        {
            old40data_NS=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index40mm_NS")->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }

}

// golden cConfiguration.cpp:6271  TfConfiguration::tbD25_Index60mmChange(TObject *Sender)
static void IC_tbD25_Index60mmChange()
{

    AnsiString buffer="60mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="58 : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="75mm : ";
    else
        buffer="60mm : ";
    EL<TLabel>("TfConfiguration", "labD25_1")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position/100.0);

    if(old60data==0.0)
        old60data=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position;

    if(old60data!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position)
    {
        aEP60OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old60data, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position); //Ifor 20170807 (wei) Sb1 =>aEPOldData  //JerryYang 20170922 (Steven) add for EP change log
        bEP60DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag
        if(old60data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old60data=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position=115;                                      //Ifor 20170807 (wei) tbD25_Index30mm => tbD25_Index60mm
        }

        if(old60data<85)
        {
            old60data=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm")->Position=85;                                       //Ifor 20170807 (wei) tbD25_Index30mm => tbD25_Index60mm
        }
    }

}

// golden cConfiguration.cpp:6560  TfConfiguration::tbD25_Index60mm_NSChange(TObject *Sender)
static void IC_tbD25_Index60mm_NSChange()
{

    AnsiString buffer="60mm for NS:";                                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="58mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="75mm for NS:";

    EL<TLabel>("TfConfiguration", "labD25_1_NS")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position/100.0);

    if(old60data_NS==0.0)
        old60data_NS=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position;

    if(old60data_NS!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position)
    {
        aEP60OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old60data_NS, EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position);   //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP60DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old60data_NS>=115)
        {
            old60data_NS=115;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old60data_NS<=85)
        {
            old60data_NS=85;
            EL<filerw::ELTrackBar>("TfConfiguration", "tbD25_Index60mm_NS")->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }

}

// golden cConfiguration.cpp:6652  TfConfiguration::tbD60_Index56mmChange(TObject *Sender)
static void IC_tbD60_Index56mmChange()
{

    AnsiString buffer="56mm : ",Sb1="",Sb2="",Sb3="";                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    static int olddata=0.0;

    EL<TLabel>("TfConfiguration", "labD60")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position/100.0);
    if(olddata==0.0)
        olddata=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position;

    if(olddata!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position)
    {
        Sb1.sprintf("EP %s mm : %4d -> %4d", buffer, olddata, EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position/100.0);
        RecordProcess(Sb1);                                                     //kevin 20170328 (Steven) add EP修改紀錄
        olddata=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm")->Position;
    }

}

// golden cConfiguration.cpp:6669  TfConfiguration::tbD60_Index56mm_NSChange(TObject *Sender)
static void IC_tbD60_Index56mm_NSChange()
{

    AnsiString buffer="56mm for NS : ",Sb1="",Sb2="",Sb3="";                    //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    static int olddata=0.0;

    EL<TLabel>("TfConfiguration", "labD60_NS")->Caption=buffer+AnsiString(EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position/100.0);
    if(olddata==0.0)
        olddata=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position;

    if(olddata!=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position)
    {
        Sb1.sprintf("EP %s mm : %4d -> %4d", buffer, olddata, EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position/100.0);
        RecordProcess(Sb1);                                                     //kevin 20170328 (Steven) add EP修改紀錄
        olddata=EL<filerw::ELTrackBar>("TfConfiguration", "tbD60_Index56mm_NS")->Position;
    }

}

// golden cConfiguration.cpp:5522  TfConfiguration::edOCRTrayLotChange(TObject *Sender)
static void IC_edOCRTrayLotChange(TObject *Sender)
{

    if(bStopChange) return;

    IC_QwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 20);   // golden cConfiguration.cpp:5526-5526 取代：//AI(W906-FRW-Q19) 20260927: fQwertyKey->ShowQwertyKey：螢幕小鍵盤在頁面（N_INTEGER 0～20，ht9045_iniconfig_p26_c.js）；這裡只做 golden 關鍵盤之後那一半（myQwertyKeyBoard.cpp:285-301：CheckRange 夾值後寫回 Text），見 IC_QwertyKey
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:5526-5526 -- //AI(W906-FRW-Q19) 20260927: fQwertyKey->ShowQwertyKey：螢幕小鍵盤在頁面（N_INTEGER 0～20，ht9045_iniconfig_p26_c.js）；這裡只做 golden 關鍵盤之後那一半（myQwertyKeyBoard.cpp:285-301：CheckRange 夾值後寫回 Text），見 IC_QwertyKey
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 20);
#endif // GATE (S12-C save)
    TEdit *Ptr;
    Ptr=(TEdit *)Sender;
    LastSet.TrayCount[Ptr->Tag]=atoi(Ptr->Text.c_str());
    WriteLastDataFile();

}

// golden cConfiguration.cpp:6094  TfConfiguration::udD46Click(TObject *Sender, TUDBtnType Button)
static void IC_udD46Click()
{

    EL<TEdit>("TfConfiguration", "edD46")->Text=EL<filerw::ELTrackBar>("TfConfiguration", "udD46")->Position;
    IniConfig.iD46WaitIndexDestroyTime=EL<filerw::ELTrackBar>("TfConfiguration", "udD46")->Position;

}

// golden cConfiguration.cpp:6125  TfConfiguration::cbE30Click(TObject *Sender)
static void IC_cbE30Click()
{

    EL<TPanel>("TfConfiguration", "palE30")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE30")->Checked)?true:false;
    EL<TPanel>("TfConfiguration", "palE31")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE31")->Checked)?true:false;
    EL<TPanel>("TfConfiguration", "palE32")->Visible=(EL<TCheckBox>("TfConfiguration", "cbE32")->Checked)?true:false;

}

// golden cConfiguration.cpp:6369  TfConfiguration::cbE39Click(TObject *Sender)
static void IC_cbE39Click()
{

    if(EL<TCheckBox>("TfConfiguration", "cbE39")->Checked)                                                          //ChungHung 20120206 Hotplate check
    {
        EL<TCheckBox>("TfConfiguration", "cbE39_1")->Visible=true;
        EL<TCheckBox>("TfConfiguration", "cbE39_1")->Checked=IniConfig.bE39_1PutTheDevicesToErrorBin;
    }
    else
    {
        EL<TCheckBox>("TfConfiguration", "cbE39_1")->Visible=false;
        EL<TCheckBox>("TfConfiguration", "cbE39_1")->Checked=false;
    }

}

// golden cConfiguration.cpp:6551  TfConfiguration::cbD36Click(TObject *Sender)
static void IC_cbD36Click()
{

    if(EL<TCheckBox>("TfConfiguration", "cbD36")->Checked==true)                                                    //jou 2014-06-24 RTC 自動進行Model驗證
    {
        EL<TCheckBox>("TfConfiguration", "cbD33")->Checked=false;
        EL<TCheckBox>("TfConfiguration", "cbD35")->Checked=true;
    }

}

// golden cConfiguration.cpp:6025  TfConfiguration::Timer1Timer(TObject *Sender)
static void IC_Timer1Timer()
{

    if(fShow==false)
        return;

    ;   // golden cConfiguration.cpp:6030-6030 取代：UpdateUT150Comm()（:5591）：Heater 分頁的溫控器輪詢／送溫度（TMC401WriteTemp、UT100WordWriteNoSucm，走 COM）＝Q41 盤點 CC-E11「加熱器測試」（歸 Jimmy）；網頁事件裡不送硬體指令
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:6030-6030 -- UpdateUT150Comm()（:5591）：Heater 分頁的溫控器輪詢／送溫度（TMC401WriteTemp、UT100WordWriteNoSucm，走 COM）＝Q41 盤點 CC-E11「加熱器測試」（歸 Jimmy）；網頁事件裡不送硬體指令
    UpdateUT150Comm();
#endif // GATE (S12-C save)

    EL<TEdit>("TfConfiguration", "edD21_mm")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD21")->Checked;
    EL<TEdit>("TfConfiguration", "edD21_Sec")->Visible  =EL<TCheckBox>("TfConfiguration", "cbD21")->Checked;
    EL<TLabel>("TfConfiguration", "labD47_1")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TLabel>("TfConfiguration", "labD47_2")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TLabel>("TfConfiguration", "labD47_3")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TEdit>("TfConfiguration", "edD47_3")->Visible    =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TButton>("TfConfiguration", "btD47")->Visible      =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TEdit>("TfConfiguration", "edD47_Count")->Visible=EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TEdit>("TfConfiguration", "edD47_Time")->Visible =EL<TCheckBox>("TfConfiguration", "cbD47")->Checked;
    EL<TEdit>("TfConfiguration", "edF05")->Visible      =EL<TCheckBox>("TfConfiguration", "cbF05")->Checked;
    EL<TLabel>("TfConfiguration", "labF05")->Visible     =EL<TCheckBox>("TfConfiguration", "cbF05")->Checked;
    EL<TLabel>("TfConfiguration", "labD21_1")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD21")->Checked;
    EL<TLabel>("TfConfiguration", "labD21_2")->Visible   =EL<TCheckBox>("TfConfiguration", "cbD21")->Checked;

    ;   // golden cConfiguration.cpp:6046-6049 取代：IPSC 數量標籤（fProductionInfo 的計數；Q41 盤點 CC-E13 的 IPSC 那一項，不是勾選連動顯示）：本 TU 不接 fProductionInfo
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:6046-6049 -- IPSC 數量標籤（fProductionInfo 的計數；Q41 盤點 CC-E13 的 IPSC 那一項，不是勾選連動顯示）：本 TU 不接 fProductionInfo
    EL<TLabel>("TfConfiguration", "lblIPSCClear_Qty")->Caption="Clear Qty    = "+ IntToStr(fProductionInfo->iIPSCFlag_ClearQty);
    EL<TLabel>("TfConfiguration", "lblIPSC_Qty")->Caption     ="IPSC Qty     = "+ IntToStr(fProductionInfo->iIPSCFlag_CountdownQty);
    EL<TLabel>("TfConfiguration", "lblIPSCIn_Qty")->Caption   ="Tester Qty   = "+ IntToStr(fProductionInfo->CalculateNowTotalICQty());
    EL<TLabel>("TfConfiguration", "lblIPSCOut_Qty")->Caption  ="Unloader Qty = "+ IntToStr(fProductionInfo->CalculateUnloadTotalICQty());
#endif // GATE (S12-C save)

}

// golden cConfiguration.cpp:6062  TfConfiguration::btD47Click(TObject *Sender)
static void IC_btD47Click()
{

    LastSet.iD47SocketTestedCount=0;
    EL<TEdit>("TfConfiguration", "edD47_3")->Text=LastSet.iD47SocketTestedCount;

}

// golden cConfiguration.cpp:6691  TfConfiguration::btnRecordJamRateByTimeClearClick(TObject *Sender)
static void IC_btnRecordJamRateByTimeClearClick()
{

    iRecordJamRateByTime_LoaderCount    = 0;
    iRecordJamRateByTime_JamCount       = 0;

    bRecordJamRateByTime_Clear = true;

}

// golden cConfiguration.cpp:6257  TfConfiguration::btResumeClick(TObject *Sender)
static void IC_btResumeClick()
{

    bLockByServer=false;
    filerw::ELMessage("Resume handler manually!", "手動恢復機台動作。");

}

// golden cConfiguration.cpp:6453  TfConfiguration::cbA09Click(TObject *Sender)
static void IC_cbA09Click()
{

    if(EL<TCheckBox>("TfConfiguration", "cbA09")->Checked==IniConfig.bA09_ByArmCloseSite)                           //wei 20160505 修改有改變時，才秀Alarm
        return;

    if(LastSet.iTemperature==Tempture_Ambient)
    {
    if(FileRW_ArmSuckHasIC(0) ||   // golden cConfiguration.cpp:6460-6460 取代：AI(W906-EVB4) 20260928 InArmSuck.HasIC()：TMyKitSuck 本 TU 看不到 → FileRW/_KitSuck.cpp 轉接
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:6460-6460 -- AI(W906-EVB4) 20260928 InArmSuck.HasIC()：TMyKitSuck 本 TU 看不到 → FileRW/_KitSuck.cpp 轉接
        if(InArmSuck.HasIC()     ||
#endif // GATE (S12-C save)
           InputShuttleHasIC()   ||
           IndexHasIC())
        {
            EL<TCheckBox>("TfConfiguration", "cbA09")->Checked=!EL<TCheckBox>("TfConfiguration", "cbA09")->Checked;
            ShowErrorMessage("MES1645", 0, MMSystem);                           //Must finish [One Cycle]
        }
    }
    else
    {
        if(MOT[MMPlate1].HasIC() ||
           MOT[MMPlate2].HasIC() ||
    FileRW_ArmSuckHasIC(0) ||   // golden cConfiguration.cpp:6472-6472 取代：AI(W906-EVB4) 20260928 InArmSuck.HasIC()：同上
#if 0 // GATE (S12-C save) golden cConfiguration.cpp:6472-6472 -- AI(W906-EVB4) 20260928 InArmSuck.HasIC()：同上
           InArmSuck.HasIC()     ||
#endif // GATE (S12-C save)
           InputShuttleHasIC()   ||
           IndexHasIC())
        {
            EL<TCheckBox>("TfConfiguration", "cbA09")->Checked=!EL<TCheckBox>("TfConfiguration", "cbA09")->Checked;
            ShowErrorMessage("MES1646", 0, MMSystem);                           //Must finish [Clean out]!!
        }
    }

}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/editlist/IniConfig.py 的 events；
//   FileRW/IniConfig.cpp 用 filerw::PageEventsRegistrar 註冊；本體 FileRW/_EditPage.cpp RunPageEvent）
static void IC_Ev_udD46Click(TControl* Sender) { (void)Sender; IC_udD46Click(); }
static void IC_Ev_cbE30Click(TControl* Sender) { (void)Sender; IC_cbE30Click(); }
static void IC_Ev_cbE39Click(TControl* Sender) { (void)Sender; IC_cbE39Click(); }
static void IC_Ev_cbD36Click(TControl* Sender) { (void)Sender; IC_cbD36Click(); }
static void IC_Ev_Timer1Timer(TControl* Sender) { (void)Sender; IC_Timer1Timer(); }
static void IC_Ev_btD47Click(TControl* Sender) { (void)Sender; IC_btD47Click(); }
static void IC_Ev_btnRecordJamRateByTimeClearClick(TControl* Sender) { (void)Sender; IC_btnRecordJamRateByTimeClearClick(); }
static void IC_Ev_btResumeClick(TControl* Sender) { (void)Sender; IC_btResumeClick(); }
static void IC_Ev_cbA09Click(TControl* Sender) { (void)Sender; IC_cbA09Click(); }
static void IC_Ev_tbD25_Index60mmChange(TControl* Sender) { (void)Sender; IC_tbD25_Index60mmChange(); }
static void IC_Ev_tbD25_Index30mmChange(TControl* Sender) { (void)Sender; IC_tbD25_Index30mmChange(); }
static void IC_Ev_tbD25_Index40mmChange(TControl* Sender) { (void)Sender; IC_tbD25_Index40mmChange(); }
static void IC_Ev_tbD25_Index60mm_NSChange(TControl* Sender) { (void)Sender; IC_tbD25_Index60mm_NSChange(); }
static void IC_Ev_tbD25_Index40mm_NSChange(TControl* Sender) { (void)Sender; IC_tbD25_Index40mm_NSChange(); }
static void IC_Ev_tbD25_Index30mm_NSChange(TControl* Sender) { (void)Sender; IC_tbD25_Index30mm_NSChange(); }
static void IC_Ev_tbD60_Index56mmChange(TControl* Sender) { (void)Sender; IC_tbD60_Index56mmChange(); }
static void IC_Ev_tbD60_Index56mm_NSChange(TControl* Sender) { (void)Sender; IC_tbD60_Index56mm_NSChange(); }
static const filerw::PageEvent kIC_Events[] = {
    {"udD46", "click", "cConfiguration.cpp:6094 TfConfiguration::udD46Click", &IC_Ev_udD46Click},
    {"cbE30", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE31", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE31_1", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE31_2", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE32", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE32_1", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE32_2", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE33", "click", "cConfiguration.cpp:6125 TfConfiguration::cbE30Click", &IC_Ev_cbE30Click},
    {"cbE39", "click", "cConfiguration.cpp:6369 TfConfiguration::cbE39Click", &IC_Ev_cbE39Click},
    {"cbD36", "click", "cConfiguration.cpp:6551 TfConfiguration::cbD36Click", &IC_Ev_cbD36Click},
    {"cbD37", "click", "cConfiguration.cpp:6551 TfConfiguration::cbD36Click", &IC_Ev_cbD36Click},
    {"cbD38", "click", "cConfiguration.cpp:6551 TfConfiguration::cbD36Click", &IC_Ev_cbD36Click},
    {"cbD36_1", "click", "cConfiguration.cpp:6551 TfConfiguration::cbD36Click", &IC_Ev_cbD36Click},
    {"cbD36_2", "click", "cConfiguration.cpp:6551 TfConfiguration::cbD36Click", &IC_Ev_cbD36Click},
    {"cbD21", "click", "cConfiguration.cpp:6025 TfConfiguration::Timer1Timer", &IC_Ev_Timer1Timer},
    {"cbD47", "click", "cConfiguration.cpp:6025 TfConfiguration::Timer1Timer", &IC_Ev_Timer1Timer},
    {"cbF05", "click", "cConfiguration.cpp:6025 TfConfiguration::Timer1Timer", &IC_Ev_Timer1Timer},
    {"btD47", "click", "cConfiguration.cpp:6062 TfConfiguration::btD47Click", &IC_Ev_btD47Click},
    {"btnRecordJamRateByTimeClear", "click", "cConfiguration.cpp:6691 TfConfiguration::btnRecordJamRateByTimeClearClick", &IC_Ev_btnRecordJamRateByTimeClearClick},
    {"btResume", "click", "cConfiguration.cpp:6257 TfConfiguration::btResumeClick", &IC_Ev_btResumeClick},
    {"cbA09", "click", "cConfiguration.cpp:6453 TfConfiguration::cbA09Click", &IC_Ev_cbA09Click},
    {"chkA09_1", "click", "cConfiguration.cpp:6453 TfConfiguration::cbA09Click", &IC_Ev_cbA09Click},
    {"cbA14", "click", "cConfiguration.cpp:6453 TfConfiguration::cbA09Click", &IC_Ev_cbA09Click},
    {"tbD25_Index60mm", "change", "cConfiguration.cpp:6271 TfConfiguration::tbD25_Index60mmChange", &IC_Ev_tbD25_Index60mmChange},
    {"tbD25_Index30mm", "change", "cConfiguration.cpp:6336 TfConfiguration::tbD25_Index30mmChange", &IC_Ev_tbD25_Index30mmChange},
    {"tbD25_Index40mm", "change", "cConfiguration.cpp:6303 TfConfiguration::tbD25_Index40mmChange", &IC_Ev_tbD25_Index40mmChange},
    {"tbD25_Index60mm_NS", "change", "cConfiguration.cpp:6560 TfConfiguration::tbD25_Index60mm_NSChange", &IC_Ev_tbD25_Index60mm_NSChange},
    {"tbD25_Index40mm_NS", "change", "cConfiguration.cpp:6591 TfConfiguration::tbD25_Index40mm_NSChange", &IC_Ev_tbD25_Index40mm_NSChange},
    {"tbD25_Index30mm_NS", "change", "cConfiguration.cpp:6622 TfConfiguration::tbD25_Index30mm_NSChange", &IC_Ev_tbD25_Index30mm_NSChange},
    {"tbD60_Index56mm", "change", "cConfiguration.cpp:6652 TfConfiguration::tbD60_Index56mmChange", &IC_Ev_tbD60_Index56mmChange},
    {"tbD60_Index56mm_NS", "change", "cConfiguration.cpp:6669 TfConfiguration::tbD60_Index56mm_NSChange", &IC_Ev_tbD60_Index56mm_NSChange},
};

