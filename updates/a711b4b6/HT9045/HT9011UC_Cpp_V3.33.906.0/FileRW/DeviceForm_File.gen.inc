// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 DeviceForm_File 的讀寫檔（HTEditList：elContact → Contact.Data、Position Offset.Data、system/Contact.ini (CosFunction.bContactHeightSaveToContactIni)）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cContact.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 182 個（header 宣告 272 個）。
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
#include "forms/fContact.h"
#include "forms/fOffSet.h"
#include "forms/fProductionInfo.h"
#include "csystem.h"
#include "cpublic.h"
#include "cUnitConvert.h"
#include "ContactForce.h"
#include "CCLink/MyCCLinkSensor_predicates.h"
#include "atester_shims.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
extern int iCloseSiteModeFor2x8;       // aHotPlateSubstrate.h:903（golden ainarm9045_2x8_8.h:28）
void GetLimitAuth();                   // cAuthority.h:77（golden cAuthority.h）
extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）
struct DF_SlkView { SlkForceTable* t; size_t size() const { return t->size(); } SlkForceData* operator[](size_t i) const { return &t->items[i]; } };
struct DF_ContactForceView { DF_SlkView SLKClass; DF_SlkView DieForceSLKClass; };
static DF_ContactForceView* DF_ContactForce() { static DF_ContactForceView v; v.SLKClass.t=&ContactForceTables().SLKClass; v.DieForceSLKClass.t=&ContactForceTables().DieForceSLKClass; return &v; }
#define fContactForce (DF_ContactForce())   // golden ContactForce.h:361 extern TfContactForce *fContactForce

// golden cContact.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
#define CarlibrationTask (fContactForm->CarlibrationTask)   // golden cContact.h:557 —— 移植樹 fContactForm 那一份（public；存檔守衛讀它）
#define eTeachSHLtcNomal 0                   // golden cContact.h:633 enum eInSHLtcStatus
static bool fShow=false;                  // golden cContact.h:549（不接 fContact／fContactForm，見檔頭）
static bool bSetupStart=false;            // golden cContact.h:545
static bool bSetupStep=false;             // golden cContact.h:551
static bool bOneCycleFinish=false;        // golden cContact.h:552
static double dDutCount=2;                // golden cContact.h:553
static bool bSetHasIC=false;              // golden cContact.h:569
static bool bAutoHighFinish=false;        // golden cContact.h:400
static int iSpeedZ=30, iSpeed=30, TorqueData=0;   // golden cContact.h:401/403/404
static bool MotorStatus=false, bContinueContact=false, bOldRTCAutoTuning=false, brecordmsgLock=false;   // golden cContact.h
static int RELEASE_UP_BIG=0, RELEASE_UP_SMALL=0;   // golden cContact.h:409-410
static int ROILearningTimes=0, iDoArm1PlaceToShuttleTask=0, iDoArm2PlaceToShuttleTask=0, iAutoTeachInSHLtcStatus=0;   // golden cContact.h
static double dMinForce=0.0;              // golden cContact.h:627（GetMinForce 寫）
static TStringList *sOutDiameter=nullptr; // golden cContact.h:583
static AnsiString asContactHeight[2];     // golden cContact.cpp:103 檔案層全域
static const int CONTACT_NORMAL=0, CONTACT_AUTO_GET_HEIGHT=1, CONTACT_MANUAL_GET_HEIGHT=2, CONTACT_TEST=3, AUTO_CONTACT_TEST=4, STEP_CONTACT_TEST=5, CONTACT_LoadCell_AUTO_GET_HEIGHT=8, CONTACT_DEVICE_MAP_CHECK=9, CONTACT_DEVICE_LOOP_TEST=10, K_TEMP_INDEX_MOVE=11, VISUAL_DETECTION_TEST=12;   // golden cContact.cpp:74-86
static const double fIndexDownPos_for9045=-135.0, fIndexDownPos_forATC=-146.0, fIndexDownPos_for9046LS=-148.0;   // golden cContact.cpp:91-93

// golden cContact.cpp 檔案層級全域（本表單私用，照抄成 static）
static double dOldDieForceKitDiameter=0.0;   // golden cContact.cpp:102

using filerw::EL;

// 前置宣告
static void DF_TfContact();
static void DF_InitContactEdtList();
static void DF_FormShow();
static void DF_ReadFile();
static void DF_SetIndexDownPos();
static double DF_CalcDeviceForce(double dPinCount, double dForcePerPinN, double dForcePerPinGf, bool bReturnKgf);
static void DF_DoIniDataToForm();
static void DF_SetContactMode();
static void DF_ChangeContactMode(bool ret);
static void DF_ShowArmAndDeviceForce();
static void DF_DutCount();
static void DF_scrbSLKChange();
static void DF_edAirForceChange();
static void DF_InitCarlibrationTask();
static void DF_UpdateContactRelative();
static void DF_spbSaveClick();
static bool DF_CheckContactSettingChange();
static void DF_SaveSetupFile(AnsiString szDir, AnsiString S);
static double DF_CalculateTotalAirForce(double dBallCount, double dSingleGf);
static double DF_GetMaxIndexForceLimit();
static double DF_GetMinForce(double dKitDiameter, int iTag);
static void DF_CountDieForceKg(bool bByPinCount);
static void DF_edPinCountChange();
static void DF_edForcePerPinNChange();
static void DF_edDieForcePerPinGChange();
static void DF_rgKitDiameterClick();
static void DF_rgOutKitDiameterClick();
static void DF_rgDieForceKitDiameterClick();
static void DF_chkUseAddWeightClick();
static void DF_cbEnableUKClick();
static void DF_cbContactModeChange();
static void DF_coD41Change();
static void DF_edContactHeight1Change();
static void DF_edForcePerDeviceKGClick();

// golden cContact.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void DF_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfContact", "cbContactMode")->Items;
      s->Add("Direct Contact");
      s->Add("Drop Contact");
      s->Add("Direct & Slow Contact");
      s->Add("TMOVE Contact");
      s->Add("TMOVE Drop Contact");
      s->Add("Direct & Soft EP Contact");
      s->Add("Drop & Soft EP Contact");
      s->Add("Shift Contact for 8Site 1x4");
      s->Add("Drop & Slow Contact");
      s->Add("TMOVE Slow Contact");
      s->Add("TMOVE Drop & Slow Contact");
    }
    { TStringList* s = EL<TComboBox>("TfContact", "cbIndexArmJogMove_mm")->Items;
      s->Add("0.01");
      s->Add("0.05");
      s->Add("0.10");
      s->Add("0.20");
    }
    { TStringList* s = EL<TComboBox>("TfContact", "cbVacuumMode")->Items;
      s->Add("Vacuum ON Mode");
      s->Add("Vacuum OFF Mode");
    }
    { TStringList* s = EL<TComboBox>("TfContact", "coD41")->Items;
      s->Add("Inside socket");
      s->Add("Above Socket");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items;
      s->Add("30");
      s->Add("40");
      s->Add("60");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgIndexUpSpeed")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items;
      s->Add("30");
      s->Add("40");
      s->Add("60");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items;
      s->Add("40mm");
      s->Add("55mm");
      s->Add("75mm");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgPSDT")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgPurgeBeforePickShuttle")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgSidePushMode")->Items;
      s->Add("M1N");
      s->Add("New");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgSuckDevicesDuringTest")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
    { TStringList* s = EL<TRadioGroup>("TfContact", "rgTesterSidePush")->Items;
      s->Add("OFF");
      s->Add("ON");
    }
}

// golden cContact.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kDF_ParentOf[][2] = {
    {"AutoZTeachButton", "gbHeightStatus"},
    {"Memo1", "gbContactStatus"},
    {"SetMPaBtn", "gbContactForce"},
    {"btAt", "gbDynaTemp"},
    {"btBarcode", "pnlBottom"},
    {"btContactForce", "pnlSLKSetting"},
    {"btOffset", "pnlBottom"},
    {"btTCPIP", "gbContactForce"},
    {"btTempOffset", "gbDynaTemp"},
    {"btnPause", "pnlBottom"},
    {"btnStart", "pnlBottom"},
    {"btnTStart", "gbContactForce"},
    {"btnTStep", "gbContactForce"},
    {"btnTempSet", "gbContactForce"},
    {"btnTrayMap", "pnlFuncSelect"},
    {"cb2DMatrix", "pnlFuncSelect"},
    {"cbCalibrateAboveHeight", "pnlFuncSelect"},
    {"cbContactMode", "gbContactParameter"},
    {"cbEnableUK", "pnlFuncSelect"},
    {"cbOneTouchAutoContactHight", "pnlFuncSelect"},
    {"cbRTCAutoTuning", "pnlFuncSelect"},
    {"cbRTCModel", "pnlFuncSelect"},
    {"cbSFCAutoTune", "pnlFuncSelect"},
    {"cbTeachInSHSen", "pnlFuncSelect"},
    {"cbTestContactMode", "pnlFuncSelect"},
    {"cbVacuumMode", "gbContactParameter"},
    {"chk2DID", "pnlFuncSelect"},
    {"chkDailyCorrelation", "pnlFuncSelect"},
    {"chkShuttle", "pnlFuncSelect"},
    {"chkTeachInOutArmZ", "pnlFuncSelect"},
    {"chkUseAddWeight", "gbContactForce"},
    {"chk_K_Temperature", "pnlFuncSelect"},
    {"coD41", "palTestSocket"},
    {"edAirForce", "gbContactForce"},
    {"edAirForceN", "gbContactForce"},
    {"edAirKPA", "gbContactForce"},
    {"edAutoContactTest", "rgHandlerMode"},
    {"edAutoKSHTReleaseOfs", "gbContactHeight"},
    {"edContactBackUp1", "gbContactHeight"},
    {"edContactBackUp2", "gbContactHeight"},
    {"edContactHeight1", "gbContactHeight"},
    {"edContactHeight2", "gbContactHeight"},
    {"edContactOffsetArm1", "gbContactHeight"},
    {"edContactOffsetArm2", "gbContactHeight"},
    {"edContactRelativeZ1", "gbContactHeight"},
    {"edContactRelativeZ2", "gbContactHeight"},
    {"edD41", "palTestSocket"},
    {"edDeviceLoopTest", "rgHandlerMode"},
    {"edDieForcePerPinG", "gbDoubleEPSet"},
    {"edDieForcePerPinN", "gbDoubleEPSet"},
    {"edDoubleForce", "gbDoubleEPSet"},
    {"edDropByPassDetect", "gbContactParameter"},
    {"edDropOffset1", "gbContactHeight"},
    {"edDropOffset2", "gbContactHeight"},
    {"edDropSpeed", "gbContactParameter"},
    {"edDropWaitTime", "gbContactParameter"},
    {"edForcePerDeviceKG", "gbContactForce"},
    {"edForcePerDeviceN", "gbContactForce"},
    {"edForcePerPinG", "gbContactForce"},
    {"edForcePerPinN", "gbContactForce"},
    {"edLoadCellHeight1", "gbContactHeight"},
    {"edLoadCellHeight2", "gbContactHeight"},
    {"edOrgPick1", "gbContactHeight"},
    {"edOrgPick2", "gbContactHeight"},
    {"edOutDiameter", "pnlBottom"},
    {"edPickUp1", "gbContactHeight"},
    {"edPickUp2", "gbContactHeight"},
    {"edPinCount", "gbContactForce"},
    {"edReleaseHeight1", "gbContactHeight"},
    {"edReleaseHeight2", "gbContactHeight"},
    {"edSetKg", "gbContactForce"},
    {"edShtPickOffset1", "gbContactHeight"},
    {"edShtPickOffset2", "gbContactHeight"},
    {"edSidePushWaitTime", "pnlSidePush"},
    {"edTestContactCount", "pnlFuncSelect"},
    {"edTestSec", "pnlFuncSelect"},
    {"edTransfer", "gbContactForce"},
    {"edUpOffset1", "gbContactHeight"},
    {"edUpOffset2", "gbContactHeight"},
    {"edUpSpeed", "gbContactParameter"},
    {"edUpWaitTime", "gbContactParameter"},
    {"edXDimension", "gbContactForce"},
    {"edYDimension", "gbContactForce"},
    {"edtPinOfDie", "gbDoubleEPSet"},
    {"edtPurgeBdforePickShuttleOffSet", "pnlPurge"},
    {"edtPurgeBeforePickShuttleInterval", "pnlPurge"},
    {"edtPurgeBeforePickShuttleTime", "pnlPurge"},
    {"edtTorqueCmp", "pnlTorqueControl"},
    {"edtTorqueMax", "pnlTorqueControl"},
    {"gbContactForce", "pnlSLKSetting"},
    {"gbContactHeight", "pnlTop"},
    {"gbContactParameter", "pnlTop"},
    {"gbContactStatus", "panHeightMode"},
    {"gbDoubleEPSet", "pnlSLKSetting"},
    {"gbDynaTemp", "pnlBottom"},
    {"gbHeightStatus", "gbContactStatus"},
    {"imgSLK", "gbContactForce"},
    {"labContactOffset", "gbContactHeight"},
    {"labContactRelative", "gbContactHeight"},
    {"labD41", "palTestSocket"},
    {"labEPValue", "gbContactForce"},
    {"labEPValue_1032", "gbContactForce"},
    {"labSetKg", "gbContactForce"},
    {"labShtOffset", "gbContactHeight"},
    {"labTransfer", "gbContactForce"},
    {"labbUserDefMaxContactHeight", "gbContactHeight"},
    {"lbAutoContactTest", "rgHandlerMode"},
    {"lbHisiVer", "gbContactForce"},
    {"lblContactOfsMM", "gbContactHeight"},
    {"lblCount", "pnlFuncSelect"},
    {"lblDummyContact", "gbContactParameter"},
    {"lblICCheck", "palTestSocket"},
    {"lblIndexUpSpeed", "gbContactParameter"},
    {"lblLoadCellHeight", "gbContactHeight"},
    {"lblLoadCellHighMM", "gbContactHeight"},
    {"lblMaxForce", "gbContactForce"},
    {"lblMaxForcePerIC", "gbContactForce"},
    {"lblMinForce", "gbContactForce"},
    {"lblNowCount", "pnlFuncSelect"},
    {"lblOrgHeight", "gbContactHeight"},
    {"lblOrgHeightMM", "gbContactHeight"},
    {"lblOrgPick", "gbContactHeight"},
    {"lblOrgPickMM", "gbContactHeight"},
    {"lblPSDT", "gbContactParameter"},
    {"lblPerGf", "gbContactForce"},
    {"lblPerPinN", "gbContactForce"},
    {"lblPinCount", "gbContactForce"},
    {"lblReadEP", "gbContactForce"},
    {"lblReadEP2", "gbContactForce"},
    {"lblReleaseOfs", "gbContactHeight"},
    {"lblShtPickOfsMM", "gbContactHeight"},
    {"lblTsetSec", "pnlFuncSelect"},
    {"lblUpOffset", "gbContactHeight"},
    {"lblUpOfsMM", "gbContactHeight"},
    {"lblUpSpeed", "gbContactParameter"},
    {"lblUpWaitTime", "gbContactParameter"},
    {"ledOneCycle", "gbContactForce"},
    {"palHeightCalibration", "pnlTop"},
    {"palOTD_4", "pnlSLKSetting"},
    {"palOTD_6", "pnlSLKSetting"},
    {"palSoakTimeWating", "gbContactHeight"},
    {"palTorqueArm1", "gbContactHeight"},
    {"palTorqueArm2", "gbContactHeight"},
    {"panHeightMode", "palHeightCalibration"},
    {"pnlDummyContact", "gbContactParameter"},
    {"pnlFuncSelect", "palHeightCalibration"},
    {"pnlHandlerSatus", "gbHeightStatus"},
    {"pnlPurge", "gbContactParameter"},
    {"pnlSensorAdj", "pnlSLKSetting"},
    {"pnlSidePush", "gbContactParameter"},
    {"pnlTorqueControl", "gbContactHeight"},
    {"rbAutoContactTest", "rgHandlerMode"},
    {"rbAutoHeight", "rgHandlerMode"},
    {"rbContactTest", "rgHandlerMode"},
    {"rbDeviceLoopTest", "rgHandlerMode"},
    {"rbDeviceMapping", "rgHandlerMode"},
    {"rbDummyMode", "pnlDummyContact"},
    {"rbKTempIndexMove", "rgHandlerMode"},
    {"rbLoadCellAutoHigh", "rgHandlerMode"},
    {"rbManualHeight", "rgHandlerMode"},
    {"rbModeNormal", "rgHandlerMode"},
    {"rbNormal", "pnlDummyContact"},
    {"rbStepContactTest", "rgHandlerMode"},
    {"rbTemp", "pnlBottom"},
    {"rbVisualDetectionTest", "rgHandlerMode"},
    {"rgDieForceKitDiameter", "gbDoubleEPSet"},
    {"rgHandlerMode", "panHeightMode"},
    {"rgIndexUpSpeed", "gbContactParameter"},
    {"rgKitDiameter", "gbContactForce"},
    {"rgOutKitDiameter", "gbContactForce"},
    {"rgPSDT", "gbContactParameter"},
    {"rgPurgeBeforePickShuttle", "pnlPurge"},
    {"rgShuttleWaitingOutSiteChamber", "gbContactParameter"},
    {"rgSidePushMode", "pnlSidePush"},
    {"rgSuckShuttleDeviceAfterTested", "gbContactParameter"},
    {"rgSuckShuttleDeviceWaitOnShuttle", "gbContactParameter"},
    {"rgTesterSidePush", "pnlSidePush"},
    {"scrbSLK", "gbContactForce"},
    {"spbOneCycle", "gbContactForce"},
    {"spbSave", "pnlBottom"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void DF_CreateContainerProxies()
{
    EL<TGroupBox>("TfContact", "gbContactForce");
    EL<TGroupBox>("TfContact", "gbContactHeight");
    EL<TGroupBox>("TfContact", "gbContactParameter");
    EL<TGroupBox>("TfContact", "gbContactStatus");
    EL<TGroupBox>("TfContact", "gbDoubleEPSet");
    EL<TGroupBox>("TfContact", "gbDynaTemp");
    EL<TGroupBox>("TfContact", "gbHeightStatus");
    EL<TPanel>("TfContact", "palHeightCalibration");
    EL<TPanel>("TfContact", "palTestSocket");
    EL<TPanel>("TfContact", "panHeightMode");
    EL<TPanel>("TfContact", "pnlBottom");
    EL<TPanel>("TfContact", "pnlDummyContact");
    EL<TPanel>("TfContact", "pnlFuncSelect");
    EL<TPanel>("TfContact", "pnlPurge");
    EL<TPanel>("TfContact", "pnlSLKSetting");
    EL<TPanel>("TfContact", "pnlSidePush");
    EL<TPanel>("TfContact", "pnlTop");
    EL<TPanel>("TfContact", "pnlTorqueControl");
    EL<TGroupBox>("TfContact", "rgHandlerMode");
    filerw::ELSetParents("TfContact", kDF_ParentOf, (int)(sizeof(kDF_ParentOf) / sizeof(kDF_ParentOf[0])));
}

// golden cContact.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void DF_DfmState()
{
    EL<TControl>("TfContact", "OTDTimer")->Enabled = false;
    EL<TButton>("TfContact", "SetMPaBtn")->Visible = false;
    EL<TControl>("TfContact", "TimerEP")->Enabled = false;
    EL<TButton>("TfContact", "btOffset")->Visible = false;
    EL<TButton>("TfContact", "btTCPIP")->Visible = false;
    EL<TButton>("TfContact", "btnPause")->Visible = false;
    EL<TButton>("TfContact", "btnStart")->Visible = false;
    EL<TButton>("TfContact", "btnTStart")->Visible = false;
    EL<TButton>("TfContact", "btnTStep")->Visible = false;
    EL<TButton>("TfContact", "btnTrayMap")->Visible = false;
    EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Visible = false;
    EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Visible = false;
    EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Visible = false;
    EL<TCheckBox>("TfContact", "cbTestContactMode")->Visible = false;
    EL<TCheckBox>("TfContact", "chkTeachInOutArmZ")->Visible = false;
    EL<TCheckBox>("TfContact", "chkUseAddWeight")->Visible = false;
    EL<TCheckBox>("TfContact", "chk_K_Temperature")->Visible = false;
    EL<TEdit>("TfContact", "edAirForce")->Enabled = false;
    EL<TEdit>("TfContact", "edAirForceN")->Enabled = false;
    EL<TEdit>("TfContact", "edAirKPA")->Enabled = false;
    EL<TEdit>("TfContact", "edContactBackUp1")->Enabled = false;
    EL<TEdit>("TfContact", "edContactBackUp2")->Enabled = false;
    EL<TEdit>("TfContact", "edContactRelativeZ1")->Enabled = false;
    EL<TEdit>("TfContact", "edContactRelativeZ2")->Enabled = false;
    EL<TEdit>("TfContact", "edDieForcePerPinN")->Enabled = false;
    EL<TEdit>("TfContact", "edForcePerDeviceKG")->Enabled = false;
    EL<TEdit>("TfContact", "edForcePerDeviceN")->Enabled = false;
    EL<TEdit>("TfContact", "edOrgPick1")->Enabled = false;
    EL<TEdit>("TfContact", "edOrgPick2")->Enabled = false;
    EL<TEdit>("TfContact", "edOutDiameter")->Enabled = false;
    EL<TEdit>("TfContact", "edOutDiameter")->Visible = false;
    EL<TEdit>("TfContact", "edSetKg")->Enabled = false;
    EL<TEdit>("TfContact", "edSetKg")->Visible = false;
    EL<TEdit>("TfContact", "edTransfer")->Enabled = false;
    EL<TEdit>("TfContact", "edTransfer")->Visible = false;
    EL<TLabel>("TfContact", "labSetKg")->Visible = false;
    EL<TLabel>("TfContact", "labTransfer")->Visible = false;
    EL<TLabel>("TfContact", "lbHisiVer")->Visible = false;
    EL<TLabel>("TfContact", "lblMaxForcePerIC")->Visible = false;
    EL<TLabel>("TfContact", "lblPSDT")->Visible = false;
    EL<TLabel>("TfContact", "lblReadEP")->Visible = false;
    EL<TLabel>("TfContact", "lblReadEP2")->Visible = false;
    EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Visible = false;
    EL<TRadioGroup>("TfContact", "rgPSDT")->Visible = false;
    EL<TControl>("TfContact", "timerContact")->Enabled = false;
    EL<TComboBox>("TfContact", "cbContactMode")->Text = "Direct Contact";
    EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex = 0;
    EL<TCheckBox>("TfContact", "chkShuttle")->Checked = true;
    EL<TComboBox>("TfContact", "coD41")->Text = "Inside socket";
    EL<TComboBox>("TfContact", "coD41")->ItemIndex = 0;
    EL<TEdit>("TfContact", "edAirForce")->Text = "edAirForce";
    EL<TEdit>("TfContact", "edAutoContactTest")->Text = "4";
    EL<TEdit>("TfContact", "edD41")->Text = "1";
    EL<TEdit>("TfContact", "edDeviceLoopTest")->Text = "4";
    EL<TEdit>("TfContact", "edOutDiameter")->Text = "40,55,75,108";
    EL<TEdit>("TfContact", "edShtPickOffset1")->Text = "0.0";
    EL<TEdit>("TfContact", "edShtPickOffset2")->Text = "0.0";
    EL<TEdit>("TfContact", "edTestContactCount")->Text = "4";
    EL<TEdit>("TfContact", "edTestSec")->Text = "1";
    EL<TEdit>("TfContact", "edtPurgeBdforePickShuttleOffSet")->Text = "1";
    EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleInterval")->Text = "1";
    EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleTime")->Text = "1";
    EL<TRadioButton>("TfContact", "rbModeNormal")->Checked = true;
    EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgIndexUpSpeed")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgPSDT")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgPurgeBeforePickShuttle")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgSidePushMode")->ItemIndex = 0;
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested")->ItemIndex = 1;
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle")->ItemIndex = 1;
    EL<TRadioGroup>("TfContact", "rgTesterSidePush")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kDF_EnableAll[] = {
    "AutoZTeachButton", "SetMPaBtn", "btAt", "btBarcode", "btContactForce", "btOffset",
    "btTCPIP", "btTempOffset", "btnPause", "btnStart", "btnTStart", "btnTStep",
    "btnTempSet", "btnTrayMap", "cb2DMatrix", "cbCalibrateAboveHeight", "cbEnableUK", "cbOneTouchAutoContactHight",
    "cbRTCAutoTuning", "cbRTCModel", "cbSFCAutoTune", "cbTeachInSHSen", "cbTestContactMode", "chk2DID",
    "chkDailyCorrelation", "chkShuttle", "chkTeachInOutArmZ", "chkUseAddWeight", "chk_K_Temperature", "gbContactForce",
    "gbContactHeight", "gbContactParameter", "gbContactStatus", "gbDoubleEPSet", "gbDynaTemp", "gbHeightStatus",
    "labContactOffset", "labContactRelative", "labD41", "labEPValue", "labEPValue_1032", "labSetKg",
    "labShtOffset", "labTransfer", "labbUserDefMaxContactHeight", "lbAutoContactTest", "lbHisiVer", "lblContactOfsMM",
    "lblCount", "lblDummyContact", "lblICCheck", "lblIndexUpSpeed", "lblLoadCellHeight", "lblLoadCellHighMM",
    "lblMaxForce", "lblMaxForcePerIC", "lblMinForce", "lblNowCount", "lblOrgHeight", "lblOrgHeightMM",
    "lblOrgPick", "lblOrgPickMM", "lblPSDT", "lblPerGf", "lblPerPinN", "lblPinCount",
    "lblReadEP", "lblReadEP2", "lblReleaseOfs", "lblShtPickOfsMM", "lblTsetSec", "lblUpOffset",
    "lblUpOfsMM", "lblUpSpeed", "lblUpWaitTime", "palHeightCalibration", "palOTD_4", "palOTD_6",
    "palSoakTimeWating", "palTestSocket", "palTorqueArm1", "palTorqueArm2", "panHeightMode", "pnlBottom",
    "pnlDummyContact", "pnlFuncSelect", "pnlHandlerSatus", "pnlPurge", "pnlSLKSetting", "pnlSensorAdj",
    "pnlSidePush", "pnlTop", "pnlTorqueControl", "rbAutoContactTest", "rbAutoHeight", "rbContactTest",
    "rbDeviceLoopTest", "rbDeviceMapping", "rbDummyMode", "rbKTempIndexMove", "rbLoadCellAutoHigh", "rbManualHeight",
    "rbModeNormal", "rbNormal", "rbStepContactTest", "rbTemp", "rbVisualDetectionTest", "rgDieForceKitDiameter",
    "rgHandlerMode", "rgIndexUpSpeed", "rgKitDiameter", "rgOutKitDiameter", "rgPSDT", "rgPurgeBeforePickShuttle",
    "rgShuttleWaitingOutSiteChamber", "rgSidePushMode", "rgSuckShuttleDeviceAfterTested", "rgSuckShuttleDeviceWaitOnShuttle", "rgTesterSidePush", "spbOneCycle",
    "spbSave",
};

// 存檔流程（spbSaveClick、CheckContactSettingChange、SaveSetupFile）讀的替身 65 個 —— FileRW/DeviceForm_File.cpp 用它決定頁面必須送哪些值
static const char* const kDF_SaveReads[] = {
    "cbContactMode", "cbEnableUK", "cbVacuumMode", "chkUseAddWeight", "chk_K_Temperature", "coD41",
    "edAirForce", "edAutoContactTest", "edAutoKSHTReleaseOfs", "edContactBackUp1", "edContactBackUp2", "edContactHeight1",
    "edContactHeight2", "edContactOffsetArm1", "edContactOffsetArm2", "edD41", "edDieForcePerPinG", "edDieForcePerPinN",
    "edDoubleForce", "edDropByPassDetect", "edDropOffset1", "edDropOffset2", "edDropSpeed", "edDropWaitTime",
    "edForcePerPinG", "edForcePerPinN", "edLoadCellHeight1", "edLoadCellHeight2", "edOrgPick1", "edOrgPick2",
    "edPickUp1", "edPickUp2", "edPinCount", "edReleaseHeight1", "edReleaseHeight2", "edShtPickOffset1",
    "edShtPickOffset2", "edSidePushWaitTime", "edTestContactCount", "edTestSec", "edUpOffset1", "edUpOffset2",
    "edUpSpeed", "edUpWaitTime", "edXDimension", "edYDimension", "edtPinOfDie", "edtPurgeBdforePickShuttleOffSet",
    "edtPurgeBeforePickShuttleInterval", "edtPurgeBeforePickShuttleTime", "edtTorqueCmp", "edtTorqueMax", "palTorqueArm1", "palTorqueArm2",
    "rgDieForceKitDiameter", "rgIndexUpSpeed", "rgKitDiameter", "rgPSDT", "rgPurgeBeforePickShuttle", "rgShuttleWaitingOutSiteChamber",
    "rgSidePushMode", "rgSuckShuttleDeviceAfterTested", "rgSuckShuttleDeviceWaitOnShuttle", "rgTesterSidePush", "scrbSLK",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void DF_CreateSaveProxies()
{
    EL<TComboBox>("TfContact", "cbContactMode");
    EL<TCheckBox>("TfContact", "cbEnableUK");
    EL<TComboBox>("TfContact", "cbVacuumMode");
    EL<TCheckBox>("TfContact", "chkUseAddWeight");
    EL<TCheckBox>("TfContact", "chk_K_Temperature");
    EL<TComboBox>("TfContact", "coD41");
    EL<TEdit>("TfContact", "edAirForce");
    EL<TEdit>("TfContact", "edAutoContactTest");
    EL<TEdit>("TfContact", "edAutoKSHTReleaseOfs");
    EL<TEdit>("TfContact", "edContactBackUp1");
    EL<TEdit>("TfContact", "edContactBackUp2");
    EL<TEdit>("TfContact", "edContactHeight1");
    EL<TEdit>("TfContact", "edContactHeight2");
    EL<TEdit>("TfContact", "edContactOffsetArm1");
    EL<TEdit>("TfContact", "edContactOffsetArm2");
    EL<TEdit>("TfContact", "edD41");
    EL<TEdit>("TfContact", "edDieForcePerPinG");
    EL<TEdit>("TfContact", "edDieForcePerPinN");
    EL<TEdit>("TfContact", "edDoubleForce");
    EL<TEdit>("TfContact", "edDropByPassDetect");
    EL<TEdit>("TfContact", "edDropOffset1");
    EL<TEdit>("TfContact", "edDropOffset2");
    EL<TEdit>("TfContact", "edDropSpeed");
    EL<TEdit>("TfContact", "edDropWaitTime");
    EL<TEdit>("TfContact", "edForcePerPinG");
    EL<TEdit>("TfContact", "edForcePerPinN");
    EL<TEdit>("TfContact", "edLoadCellHeight1");
    EL<TEdit>("TfContact", "edLoadCellHeight2");
    EL<TEdit>("TfContact", "edOrgPick1");
    EL<TEdit>("TfContact", "edOrgPick2");
    EL<TEdit>("TfContact", "edPickUp1");
    EL<TEdit>("TfContact", "edPickUp2");
    EL<TEdit>("TfContact", "edPinCount");
    EL<TEdit>("TfContact", "edReleaseHeight1");
    EL<TEdit>("TfContact", "edReleaseHeight2");
    EL<TEdit>("TfContact", "edShtPickOffset1");
    EL<TEdit>("TfContact", "edShtPickOffset2");
    EL<TEdit>("TfContact", "edSidePushWaitTime");
    EL<TEdit>("TfContact", "edTestContactCount");
    EL<TEdit>("TfContact", "edTestSec");
    EL<TEdit>("TfContact", "edUpOffset1");
    EL<TEdit>("TfContact", "edUpOffset2");
    EL<TEdit>("TfContact", "edUpSpeed");
    EL<TEdit>("TfContact", "edUpWaitTime");
    EL<TEdit>("TfContact", "edXDimension");
    EL<TEdit>("TfContact", "edYDimension");
    EL<TEdit>("TfContact", "edtPinOfDie");
    EL<TEdit>("TfContact", "edtPurgeBdforePickShuttleOffSet");
    EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleInterval");
    EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleTime");
    EL<TEdit>("TfContact", "edtTorqueCmp");
    EL<TEdit>("TfContact", "edtTorqueMax");
    EL<TPanel>("TfContact", "palTorqueArm1");
    EL<TPanel>("TfContact", "palTorqueArm2");
    EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter");
    EL<TRadioGroup>("TfContact", "rgIndexUpSpeed");
    EL<TRadioGroup>("TfContact", "rgKitDiameter");
    EL<TRadioGroup>("TfContact", "rgPSDT");
    EL<TRadioGroup>("TfContact", "rgPurgeBeforePickShuttle");
    EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber");
    EL<TRadioGroup>("TfContact", "rgSidePushMode");
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested");
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle");
    EL<TRadioGroup>("TfContact", "rgTesterSidePush");
    EL<TControl>("TfContact", "scrbSLK");
}

// golden cContact.cpp:107  TfContact::TfContact(TComponent* Owner)
static void DF_TfContact()
{

    if(CUSTOMER_CODE==CC_JCET)                                                  //RogerYang 20260129 : 凌中心不想要人員在右上角關閉視窗時掉料 -> 把X收起來
    {
    ;   // golden cContact.cpp:112-112 取代：BorderIcons：視窗標題列按鈕（HTML 不用）
#if 0 // GATE (S12-C save) golden cContact.cpp:112-112 -- BorderIcons：視窗標題列按鈕（HTML 不用）
        this->BorderIcons=TBorderIcons();
#endif // GATE (S12-C save)
    }

    brecordmsgLock=false;
    AnsiString FileName="D:\\HT9045\\system\\ContactInfo.ini";
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20130724 高雄日月光 contact修改成NS顯示
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Text!="")
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Clear();
    ;   // golden cContact.cpp:121-121 取代：rgKitDiameter->Caption：群組標題字（vclcompat TRadioGroup 沒有 Caption；HTML 標籤）
#if 0 // GATE (S12-C save) golden cContact.cpp:121-121 -- rgKitDiameter->Caption：群組標題字（vclcompat TRadioGroup 沒有 Caption；HTML 標籤）
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Caption= "Kit Diameter (Square)";
#endif // GATE (S12-C save)
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("40 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("55 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("75 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("20 mm");
        bKitOutDiameter =true;
        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Add("40 mm");                                  //kevin 20170628 (Steven) add for ASE-KH
        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Add("55 mm");
        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Add("75 mm");
        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Add("20 mm");
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Text!="")
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Clear();
    ;   // golden cContact.cpp:137-137 取代：rgKitDiameter->Caption：同上
#if 0 // GATE (S12-C save) golden cContact.cpp:137-137 -- rgKitDiameter->Caption：同上
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Caption= "Kit Diameter";
#endif // GATE (S12-C save)
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("28 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("40 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("58 mm");
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add("56 mm");
    }

    if(CosFunction.bUseDynamicKitDiameter)                                      //Steven 20170605 (wei) : 可以自定義Kit直徑
    {
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Clear();

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            if(FileExists(FileName))
                EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Type"), AnsiString("28,40,58,56"));
            else
                EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=AnsiString("28,40,58,56");
        }
        else
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20210813 高雄日月光 contact修改顯示
            {
                EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Clear();                               //kevin  20210814 add
                if(FileExists(FileName))
                {
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Type"), AnsiString("30,40,60"));
                    EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->CommaText=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("OutDiameter"), AnsiString("40,55,75"));  //kevin  20210814 add
                    EL<TEdit>("TfContact", "edOutDiameter")->Text=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("OutDiameter"), AnsiString("40,55,75,108"));           //kevin  20210814 add OutDiameter
                }
                else
                {
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=AnsiString("30,40,60");
                    EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->CommaText=AnsiString("40,55,75");  //kevin  20210814 add  OutDiameter
                    EL<TEdit>("TfContact", "edOutDiameter")->Text=("40,55,75,20");
                }
    sOutDiameter = new TStringList(); sOutDiameter->Clear(); sOutDiameter->CommaText=EL<TEdit>("TfContact", "edOutDiameter")->Text; for(int i=0; i<sOutDiameter->Count; i++) { sKitOutDiameter[i]=""; if(sOutDiameter->Strings[i]!="" && atof(sOutDiameter->Strings[i].c_str())>15.0) sKitOutDiameter[i]=sOutDiameter->Strings[i]; } sOutDiameter->Clear(); delete sOutDiameter;   // golden cContact.cpp:172-183 取代：sOutDiameter：golden cContact.h:583 的 TStringList* 成員（產生器依 header 把它當元件名改寫，改回成員本身；程式照 golden）
#if 0 // GATE (S12-C save) golden cContact.cpp:172-183 -- sOutDiameter：golden cContact.h:583 的 TStringList* 成員（產生器依 header 把它當元件名改寫，改回成員本身；程式照 golden）
                EL<TControl>("TfContact", "sOutDiameter") = new TStringList();
                EL<TControl>("TfContact", "sOutDiameter")->Clear();
                EL<TControl>("TfContact", "sOutDiameter")->CommaText=EL<TEdit>("TfContact", "edOutDiameter")->Text;

                for(int i=0; i<EL<TControl>("TfContact", "sOutDiameter")->Count; i++)
                {
                    sKitOutDiameter[i]="";                                      //Ztex 2024.08.30 Add Clear sKitOutDiameter Array
                    if(EL<TControl>("TfContact", "sOutDiameter")->Strings[i]!="" && atof(EL<TControl>("TfContact", "sOutDiameter")->Strings[i].c_str())>15.0)
                       sKitOutDiameter[i]=EL<TControl>("TfContact", "sOutDiameter")->Strings[i];
                }
                EL<TControl>("TfContact", "sOutDiameter")->Clear();
                delete EL<TControl>("TfContact", "sOutDiameter");
#endif // GATE (S12-C save)
            }
            else
            {
                if(FileExists(FileName))
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("Type"), AnsiString("30,40,60,56"));
                else
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->CommaText=AnsiString("30,40,60,56");
            }
        }
    }
    DF_InitContactEdtList();

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                                                    //Ifor 20191003 : add Die Force 可以自定義Kit直徑   //AI(ht9045-v899) 20260430: extend Die Force kit selection to Multi EP half.
    {
        EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Clear();
        if(FileExists(FileName))
            EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->CommaText=CheckAndReadIniData(FileName, AnsiString("SLK Type"), AnsiString("DieForceType"), AnsiString("20,30,40,50"));
        else
            EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->CommaText=AnsiString("20,30,40,50");
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
    {
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Clear();
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct & Soft Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Drop Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct & Soft EP Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop & Soft EP Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Shift Contact for 8Site 1x4");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop & Slow Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Slow Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Drop & Slow Contact");
    }
    else
    {
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Clear();
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop Contact");
        if(CUSTOMER_CODE==CC_SPIL_CS)   //JerryYang 20260415 : 矽品中山要求Follow台積電顯示名稱
            EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct & Soft Contact");
        else
            EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct & Slow Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Drop Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Direct & Soft EP Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop & Soft EP Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Shift Contact for 8Site 1x4");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("Drop & Slow Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Slow Contact");
        EL<TComboBox>("TfContact", "cbContactMode")->Items->Add("TMOVE Drop & Slow Contact");
    }

    fShow=false;
    bSetupStart=false;
    bSetupStep=false;
    dDutCount=2;

    iSpeed=30;
    iSpeedZ=30;                                                                 //Steven 20110503 : 240KG 加快速度
    TorqueData=0;
    MotorStatus=false;
    bContinueContact=false;
    bOldRTCAutoTuning=false;                                                    //Sam 20240711 : RTC Auto Tuning
    ;   // golden cContact.cpp:250-250 取代：InitRTCAutoTuning()：RTC Auto Tuning 狀態機 task（執行期狀態，在 fContactForm；它的 Init() 已做）
#if 0 // GATE (S12-C save) golden cContact.cpp:250-250 -- InitRTCAutoTuning()：RTC Auto Tuning 狀態機 task（執行期狀態，在 fContactForm；它的 Init() 已做）
    InitRTCAutoTuning();                                                        //Sam 20230419 : 新增 RTC Auto Tuning 功能
#endif // GATE (S12-C save)
    if(MachineTypeChoice==Type_HT9046)                                          //9046
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Max=6;   // golden cContact.cpp:252-252 取代：scrbSLK->Max（TScrollBar → ELTrackBar，見檔頭）
#if 0 // GATE (S12-C save) golden cContact.cpp:252-252 -- scrbSLK->Max（TScrollBar → ELTrackBar，見檔頭）
        EL<TControl>("TfContact", "scrbSLK")->Max=6;
#endif // GATE (S12-C save)
    else if(MachineTypeChoice==Type_HT9045_12Site)                              //ChungHung 20130507 add HT9045 updata for 12site 517
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Max=6;   // golden cContact.cpp:254-254 取代：scrbSLK->Max
#if 0 // GATE (S12-C save) golden cContact.cpp:254-254 -- scrbSLK->Max
        EL<TControl>("TfContact", "scrbSLK")->Max=6;
#endif // GATE (S12-C save)
    else
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Max=5;   // golden cContact.cpp:256-256 取代：scrbSLK->Max
#if 0 // GATE (S12-C save) golden cContact.cpp:256-256 -- scrbSLK->Max
        EL<TControl>("TfContact", "scrbSLK")->Max=5;
#endif // GATE (S12-C save)

    if(MachineTypeChoice==Type_HT9046_LS)                                       //Steven 20140516 : for 1024x768解析度
    {
        if(IniConfig.bSIGURDFunction)                                           //Sam 20211230 : 矽格先卡這個高度
    ;   // golden cContact.cpp:261-261 取代：gbContactParameter->Height：版面高度（HTML 不用）
#if 0 // GATE (S12-C save) golden cContact.cpp:261-261 -- gbContactParameter->Height：版面高度（HTML 不用）
            EL<TGroupBox>("TfContact", "gbContactParameter")->Height=230;
#endif // GATE (S12-C save)
        else
    ;   // golden cContact.cpp:263-263 取代：gbContactParameter->Height：版面高度
#if 0 // GATE (S12-C save) golden cContact.cpp:263-263 -- gbContactParameter->Height：版面高度
            EL<TGroupBox>("TfContact", "gbContactParameter")->Height=265;                                     //kevin 20190913 //ChungHung 20150519 modify 169--->233  //wei 20170526 (jou) 233->265
#endif // GATE (S12-C save)
    }
    else
    {
    ;   // golden cContact.cpp:267-267 取代：gbContactParameter->Height：版面高度
#if 0 // GATE (S12-C save) golden cContact.cpp:267-267 -- gbContactParameter->Height：版面高度
        EL<TGroupBox>("TfContact", "gbContactParameter")->Height=230;                                         //ChungHung 20150519 modify 126--->177  //wei 20170526 (jou) 177->230
#endif // GATE (S12-C save)
    }

    ROILearningTimes=20;                                                        //Steven 20120222 : ROI的學習次數
    //OTDTimer->Enabled=(USE_OTD);                                              //Steven 20140312 : OTD
    iDoArm1PlaceToShuttleTask=1;                                                //JimmyChiu 20220708 : add Index Arm Axis
    iDoArm2PlaceToShuttleTask=1;                                                //JimmyChiu 20220708 : add Index Arm Axis

    iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;                                   //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
    #ifndef BOOST_FUNCTION_TEST
    EL<TButton>("TfContact", "btnTempSet")->Visible=false;
    #endif

}

// golden cContact.cpp:358  TfContact::InitContactEdtList()
static void DF_InitContactEdtList()
{

//    bool bShow=true, bNoShow=false;
//    bool bEnable=true, bDisable=false;
//    bool bReadFromFile=true, bFixedValue=false;

}

// golden cContact.cpp:1094  TfContact::FormShow(TObject *Sender)
static void DF_FormShow()
{

    AnsiString S, Str;
//    int Pos=0;
    EL<TGroupBox>("TfContact", "gbContactHeight")->Enabled=true;
    EL<TControl>("TfContact", "scrbSLK")->Enabled        =true;
    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Enabled  =true;
    EL<TEdit>("TfContact", "edD41")->Enabled          =true;
    bRTCAutoVerifyControlEP =false;                                             //jou 2014-06-24 RTC 自動進行Model驗證
    bTestEPaddKg            =false;                                             //jou 20171024
    ;   // golden cContact.cpp:1104-1104 取代：rbTemp->SetFocus()：焦點
#if 0 // GATE (S12-C save) golden cContact.cpp:1104-1104 -- rbTemp->SetFocus()：焦點
    EL<TRadioButton>("TfContact", "rbTemp")->SetFocus();                                                         //jou 20231011 : 改用rbTemp, 避免跳例外
#endif // GATE (S12-C save)
    GetLimitAuth();                                                             //jou 2014-08-08 修正 contact offset 上下限讀值為0 錯誤
    ;   // golden cContact.cpp:1106-1106 取代：LoadImage()：示意圖（HTML 自己有）
#if 0 // GATE (S12-C save) golden cContact.cpp:1106-1106 -- LoadImage()：示意圖（HTML 自己有）
    LoadImage();
#endif // GATE (S12-C save)
    EL<TPanel>("TfContact", "pnlHandlerSatus")->Visible=false;

    if(CosFunction.bUseDynamicKitDiameter)                                      //Steven 20170605 (wei) : 可以自定義Kit直徑
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Text!="")
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Clear();

        if(bKitOutDiameter && EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Text!="")                //kevin 20170628 (Steven) add for ASE-KH
        {
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Visible=true;
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Top =255;                                         //kevin 20191214 change 220->255;
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Left=12;
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Clear();
            EL<TRadioGroup>("TfContact", "rgKitDiameter") ->Visible=false;
        }

    ;   // golden cContact.cpp:1123-1123 取代：rgKitDiameter->Caption：群組標題字
#if 0 // GATE (S12-C save) golden cContact.cpp:1123-1123 -- rgKitDiameter->Caption：群組標題字
        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Caption="Kit Diameter";
#endif // GATE (S12-C save)
        for(unsigned int i=0; i<fContactForce->SLKClass.size(); i++)
        {
            if(fContactForce->SLKClass[i]->bShow)
            {
                if(EP_Install==5)
                {
                    Str=fContactForce->SLKClass[i]->sDiameter;
                    if(Str.Pos("Arm2_")==0)
                    {
                        Str=Str.SubString(6, Str.Length());
                        EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add(Str);
                    }
                }
                else if(fContactForce->SLKClass[i]->sDiameter!="80_Hi")         //Ifor 20200318 : add 鋼徑80mmn 分段補償
                {
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Add(fContactForce->SLKClass[i]->sDiameter);
                    if(bKitOutDiameter && i<=9)                                 //kevin 20170628 (Steven) add for ASE-KH
                        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->Items->Add(sKitOutDiameter[i]);
                }
            }
        }
    }

    if(INSTALL_DOUBLE_EP>0)                                                     //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    {
        if(EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Text!="")
            EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Clear();

    ;   // golden cContact.cpp:1152-1152 取代：rgDieForceKitDiameter->Caption：群組標題字
#if 0 // GATE (S12-C save) golden cContact.cpp:1152-1152 -- rgDieForceKitDiameter->Caption：群組標題字
        EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Caption="Die Force Kit Diameter";
#endif // GATE (S12-C save)
        for(unsigned int i=0; i<fContactForce->DieForceSLKClass.size(); i++)
        {
            if(fContactForce->DieForceSLKClass[i]->bShow)
            {
                EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Add(fContactForce->DieForceSLKClass[i]->sDiameter);
            }
        }
        dOldDieForceKitDiameter=DeviceForm_File.dDieForceKitDiameter;
    }

    if(CosFunction.bAutoHeightSHTReleaseByFile)                                 //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
    {
        EL<TEdit>("TfContact", "edAutoKSHTReleaseOfs")->Visible=true;
        EL<TLabel>("TfContact", "lblReleaseOfs")->Visible=true;
    }
    else
    {
        EL<TEdit>("TfContact", "edAutoKSHTReleaseOfs")->Visible=false;
        EL<TLabel>("TfContact", "lblReleaseOfs")->Visible=false;
    }

    DF_ReadFile();
    DF_DoIniDataToForm();
    S.sprintf("Contact Parameter  ''%s''  ",GetLastOpenFN());
    ;   // golden cContact.cpp:1177-1177 取代：Caption=S：視窗標題
#if 0 // GATE (S12-C save) golden cContact.cpp:1177-1177 -- Caption=S：視窗標題
    Caption=S;
#endif // GATE (S12-C save)
    EL<TRadioButton>("TfContact", "rbModeNormal")->Checked=true;
    DF_SetContactMode();                                                           //Steven 20170605 (wei) : 修改顯示
    bSetupStart=false;
    DF_ShowArmAndDeviceForce();

    fShow=true;                                                                 //Steven 20220209 : 要在ScrollBar1被修改前
    EL<TControl>("TfContact", "scrbSLK")->Visible=true;
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position=DeviceForm_File.iHeadDeviceCT; DF_scrbSLKChange();   // golden cContact.cpp:1185-1186 取代：scrbSLK->Position（ELTrackBar）＋scrbSLKChange(this)（事件處理器參數已拿掉）
#if 0 // GATE (S12-C save) golden cContact.cpp:1185-1186 -- scrbSLK->Position（ELTrackBar）＋scrbSLKChange(this)（事件處理器參數已拿掉）
    EL<TControl>("TfContact", "scrbSLK")->Position=DeviceForm_File.iHeadDeviceCT;
    DF_scrbSLKChange(this);
#endif // GATE (S12-C save)
    bSetupStep=false;
    DF_InitCarlibrationTask();

    if(EP_Install)
    {
        EL<TLabel>("TfContact", "lblReadEP")->Visible=true;
        EL<TLabel>("TfContact", "labSetKg")->Visible=true;                                                 //Ifor 20150909 Contact 最小公斤顯示
        EL<TEdit>("TfContact", "edSetKg")->Visible=true;

        if(CUSTOMER_CODE==CC_HONPREC_QC)                                        //Ifor 20150909 : QC 顯示
        {
            EL<TLabel>("TfContact", "labTransfer")->Visible=true;                                          //Ifor 20150909 Contact EP 轉換公斤顯示
            EL<TEdit>("TfContact", "edTransfer")->Visible=true;
        }
    }

    if(EP_Install==5)
    {
        EL<TLabel>("TfContact", "lblReadEP2")->Visible=true;
    }

    if(IndexHasIC() || FileRW_InOutArmSuckHasIC() || ShuttleHasIC())   // golden cContact.cpp:1208-1211 取代：InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
#if 0 // GATE (S12-C save) golden cContact.cpp:1208-1211 -- InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
    if(IndexHasIC() ||                                                          //jou 980615 one cycle finish after can auto high or contact test
       InArmSuck.HasIC() ||                                                     //JerryYang 20181212 (Steven) : shuttle及 in/out arm有IC不能切換模式
       OutArmSuck.HasIC() ||
       ShuttleHasIC())
#endif // GATE (S12-C save)
    {
        EL<TGroupBox>("TfContact", "rgHandlerMode")->Enabled=false;
        bOneCycleFinish=false;
        EL<TRadioButton>("TfContact", "rbModeNormal")->Enabled=false;                                            //kevin 20190227 has IC
    }
    else
    {
        EL<TGroupBox>("TfContact", "rgHandlerMode")->Enabled=true;
        bOneCycleFinish=true;
        EL<TRadioButton>("TfContact", "rbModeNormal")->Enabled=true;                                             //kevin 20190227 has IC
    }

    if(IniConfig.bContactAlwaysIncludeShuttle)                                  //Steven 20090926 for KYEC 29818 : Never Enable and Always True
    {
        EL<TCheckBox>("TfContact", "chkShuttle")->Checked=true;
        EL<TCheckBox>("TfContact", "chkShuttle")->Enabled=false;
    }

    ;   // golden cContact.cpp:1230-1231 取代：Left／Top：視窗位置
#if 0 // GATE (S12-C save) golden cContact.cpp:1230-1231 -- Left／Top：視窗位置
    Left=10;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    EL<TRadioButton>("TfContact", "rbDeviceMapping")->Visible=CosFunction.bDeviceMapTest;                        //Steven 20190910 : Qualcomm功能
    EL<TRadioButton>("TfContact", "rbVisualDetectionTest")->Visible=(IniConfig.bAMDFunction);                   //Ifor 20260204 add: TFAMD 要求新增檢測流程
    bContinueContact=false;
    EL<TButton>("TfContact", "SetMPaBtn")->Visible=false;                                                   //Steven 20100811 沒用到的元件
    //cbContactMode->Enabled=true;                                              //Steven 20100406 : 避免做到一半被更換
    EL<TComboBox>("TfContact", "cbVacuumMode")->Enabled=true;
    EL<TComboBox>("TfContact", "cbContactMode")->Enabled = EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropPlaceShiftContact?false:true;                                //ChungHung 20150528 add for 海思 _8Site1x4
    EL<TComboBox>("TfContact", "cbVacuumMode")->Enabled  = EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropPlaceShiftContact?false:true;                                //ChungHung 20150528 add for 海思 _8Site1x4
    EL<TEdit>("TfContact", "edContactHeight1")->Enabled=true;
    EL<TEdit>("TfContact", "edContactHeight2")->Enabled=true;

    if(CosFunction.bHiSiliconFunction==true ||                                  //Ifor 20190905 : add 海思 V02.30 版 不可修改ContactHeight高度
       IniConfig.bSPILFunction==true        ||                                  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
       CUSTOMER_CODE==CC_ASE_M              ||                                  //Ifor 20170117 add ASEM 不可修改ContactHeight高度
       CUSTOMER_CODE==CC_KYEC_LEE           ||                                  //wei 20150622 與ASE高雄區隔
       IniConfig.bVTESTFunction==true )
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnablePEModel==true)
        {
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled=true;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled=true;
        }
        else
        {
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled=false;
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                                       //wei 20150622 與ASE高雄區隔
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                     //Steven 20131101 : Add ASE-K12 //Steven 20100412 : Index有IC時不可以修改
    {
        if(IndexHasIC())
        {
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled=false;
            EL<TEdit>("TfContact", "edContactOffsetArm1")->Enabled=false;                                 //kevin 20150520
            EL<TEdit>("TfContact", "edContactOffsetArm2")->Enabled=false;                                 //kevin 20150520
        }
        else
        {
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled=true;                                     //kevin 20150520
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled=true;                                     //kevin 20150520
        }
        //chkUseAddWeight->Visible=true;                                        //kevin 20170628 (Steven) add for ASE-KH
        EL<TCheckBox>("TfContact", "chkTeachInOutArmZ")->Visible=true;                                        //kevin 20180129 (Steven) add auto teach in out arm Z
        if(bIndexCheckVacum==false)                                             //kevin 20200716 add ASE_KH 卡輸入
        {
            EL<TPanel>("TfContact", "palTestSocket")->Enabled=false;
            EL<TComboBox>("TfContact", "coD41")->Enabled=false;
            EL<TEdit>("TfContact", "edD41")->Enabled=false;
        }
        else
        {
            EL<TPanel>("TfContact", "palTestSocket")->Enabled=true;
            EL<TComboBox>("TfContact", "coD41")->Enabled=true;
            EL<TEdit>("TfContact", "edD41")->Enabled=true;
        }
        EL<TCheckBox>("TfContact", "chkShuttle")->Checked=true;                                               //kevin 20220630 add  強制 shuttle k,不需要再取消

        if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                 //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
        {
            EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Visible=true;
        }
        else
        {
            EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Visible=false;
            EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Checked=false;
        }
    }
    else if(CUSTOMER_CODE==CC_ASE_CL)                                           //JerryYang 20250120 : add
    {
        EL<TCheckBox>("TfContact", "chkTeachInOutArmZ")->Visible=true;                                        //kevin 20180129 (Steven) add auto teach in out arm Z
        EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;                                      //Chrischen 20260316 : ASECL Contact反灰需求
        EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
    }
    else
    {
        EL<TCheckBox>("TfContact", "chkTeachInOutArmZ")->Visible=false;                                       //kevin 20180129 (Steven) add auto teach in out arm Z
        EL<TCheckBox>("TfContact", "chkTeachInOutArmZ")->Checked=false;                                       //kevin 20180129 (Steven) add auto teach in out arm Z
    }

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
    {
        EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Visible=false;
        EL<TCheckBox>("TfContact", "cbTeachInSHSen")->Checked=false;
    }

    EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Checked=false;                                  //Ifor 20220803 add 一鍵完成Auto Contact Hight
    EL<TLabel>("TfContact", "lblDummyContact")->Visible=false;
    EL<TPanel>("TfContact", "pnlDummyContact")->Visible=false;

    EL<TPanel>("TfContact", "palOTD_4")->Visible=(USE_OTD==1?true:false);                                  //Steven 20140312 : OTD    //ChungHung 20140709 add by hardware control
    EL<TPanel>("TfContact", "palOTD_6")->Visible=(USE_OTD==1?true:false);                                  //Steven 20140312 : OTD    //ChungHung 20140709 add by hardware control

    EL<TPanel>("TfContact", "pnlSensorAdj")->Visible=(SHUTTLE_SENSOR_TYPE==eSensorCCLink  ||               //CC-Link      //Steven 20131008 : for HT9046AH
                           SHUTTLE_SENSOR_TYPE==eSensorCCLink3 ||
                           UseCanBusOrEtherCAT())?true:false;                   //Sam 20230707 : EtherCAT Shuttle sensor

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        RELEASE_UP_BIG=400;                                                     //jou 2010-11-09 矽品要求release改成4mm
        RELEASE_UP_SMALL=400;
        if(fSecurity->Insufficient(109,false)==false)                           //Ifor 20151103 : 矽品要求Contact 畫面中Sensor Adjustment新增功能顯示權限
            EL<TPanel>("TfContact", "pnlSensorAdj")->Visible=false;
    }
    else if(CUSTOMER_CODE==CC_AMKOR_China ||
            CUSTOMER_CODE==CC_QUALCOMM)                                         //JerryYang 20170412 (Steven) add QUALCOMM
    {
        RELEASE_UP_BIG=300;
        RELEASE_UP_SMALL=200;
    }
    else if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                   //KaiChen 20191111 ：矽格-北興 黃建榮 要求 Contact 頁面中，IC 大小任一邊小於3.5mm，Release Height 設為 1mm
    {
        RELEASE_UP_BIG=200;
        RELEASE_UP_SMALL=100;
    }
    else
    {
        RELEASE_UP_BIG=200;
        RELEASE_UP_SMALL=200;                                                   //jou 2010-11-23
    }

    iContactMode=CONTACT_NORMAL;
    bAutoHighFinish=false;

    if(INDEX_PRESS_TYPE==e240KG)                                                //Steven 20110314 : 240KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:240)";
    else if(INDEX_PRESS_TYPE==e120KG)                                           //Steven 20121124 : 120KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:120)";
    else if(INDEX_PRESS_TYPE==e500KG)                                           //Steven 20121124 : 500KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:500)";
    else if(INDEX_PRESS_TYPE==e400KG)                                           //Steven 20131007 : Index 1.5KW, 400KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:400)";
    else if(INDEX_PRESS_TYPE==e360KG)                                           //Steven 20140312 : For HT9045WA 360KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:360)";
    else if(INDEX_PRESS_TYPE==e260KG)
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:260)";
    else if(INDEX_PRESS_TYPE==e160KG)                                           //ifor 20170215 (wei) add 160KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:160)";
    else if(INDEX_PRESS_TYPE==e640KG)                                           //Sam 20210628 : add 640KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:640)";
    else if(INDEX_PRESS_TYPE==e800KG)                                           //Ifor 20230830 : add 800KG
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:800)";
    else
        EL<TLabel>("TfContact", "lblMaxForce")->Caption="Kgf(Max:85)";
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20171130 (Steven) SPIL 順信要求提高auto height速度
        iSpeedZ=(INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e400KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e360KG)?120:50;
    else
        iSpeedZ=(INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e400KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e360KG)?120:30;                          //Steven 20110503 : 240KG 加快速度  //Steven 20131007 : Index 1.5KW, 400KG

#if 0 // GATE (S12-C save) golden cContact.cpp:1388-1404 -- iFTestBackItem/iBTestBackItem backup of FTestSuck/BTestSuck.Item (aHotPlateSubstrate.h vs HTEditList.h TList clash; restore side is golden FormClose, not run by the web page)
    for(int i=0; i<FTestSuck.iMaxRow; i++)                                      //jou 2011-07-01 start : 解決未完成auto high中途退出會一直出現need one cycle error
    {
        for(int j=0; j<FTestSuck.iMaxCol; j++)
        {
            iFTestBackItem[i][j]=NULL_IC;
            iBTestBackItem[i][j]=NULL_IC;
        }
    }

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            iFTestBackItem[i][j]=FTestSuck.Item[i][j];
            iBTestBackItem[i][j]=BTestSuck.Item[i][j];
        }
    }
#endif // GATE (S12-C save)

    if(CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bHiSiliconFunction)            //Ifor 20190705 : KYEC 海思版Contact頁面不顯示EP調整選項
    {
        EL<TPanel>("TfContact", "btContactForce")->Visible=false;
    }
    else if(WEIGHT_CALIBRATION      ||                                          //Steven 20150224 : Contact畫面可以調整D25設定值
            CUSTOMER_CODE==CC_HONPREC_QC ||                                     //Steven 20111108
            CosFunction.bUseDynamicKitDiameter)                                 //Steven 20170605 (wei) : 可以自定義Kit直徑
    {
        EL<TPanel>("TfContact", "btContactForce")->Visible=fSecurity->Insufficient(152, false);            //Steven 20220208 : 壓力校正的權限
    }
    else
    {
        EL<TPanel>("TfContact", "btContactForce")->Visible=false;
    }

    if(REAL_TIME_CCD)                                                           //Steven 20111102
    {
        EL<TCheckBox>("TfContact", "cbRTCModel")->Visible=!COM2->bCCDDummyRum;                                //Steven 20110922
        EL<TCheckBox>("TfContact", "cbRTCModel")->Checked=false;                                              //ChungHung 20120106 預設不開啟 SCK要求
        EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Visible=!COM2->bCCDDummyRum;                           //Sam 20230419 : 新增 RTC Auto Tuning 功能
        EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Checked=false;
        EL<TCheckBox>("TfContact", "cbRTCModel")->Enabled=true;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbRTCModel")->Visible=false;                                              //Steven 20110922
        EL<TCheckBox>("TfContact", "cbRTCModel")->Checked=false;
        EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Visible=false;                                         //Sam 20230419 : 新增 RTC Auto Tuning 功能
        EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Checked=false;
    }

    if(IniConfig.bAMDFunction)
    {
         EL<TCheckBox>("TfContact", "cb2DMatrix")->Visible=false;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cb2DMatrix")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode);                 //Steven 20160118 : 自動學習2D code Matrix
        EL<TCheckBox>("TfContact", "cb2DMatrix")->Checked=false;
    }
    EL<TCheckBox>("TfContact", "chk2DID")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode);                        //Steven 20210310 : contact test可以跳過讀取2DID
    EL<TCheckBox>("TfContact", "chk2DID")->Checked=TestIF_File.bEnableBarCode;

    if(AccessLevel<LevelSet.AccessLevel[18])
    {
        EL<TGroupBox>("TfContact", "gbContactHeight")->Enabled        =false;
        EL<TGroupBox>("TfContact", "gbContactParameter")->Enabled     =false;
        EL<TGroupBox>("TfContact", "gbContactForce")->Enabled         =false;
        EL<TPanel>("TfContact", "palHeightCalibration")->Enabled   =false;
    }
    else
    {
        if(IniConfig.bEnableRms &&
           (CUSTOMER_CODE==CC_SCC ||
            CUSTOMER_CODE==CC_SCK))                                             //Steven 20101216
            EL<TGroupBox>("TfContact", "gbContactParameter")->Enabled=false;
        else
            EL<TGroupBox>("TfContact", "gbContactParameter")->Enabled =fSecurity->Insufficient( 92, false);   //jou 2014-03-26 SPIL 要求 Contact mode 權限分級
        EL<TGroupBox>("TfContact", "gbContactHeight")->Enabled        =fSecurity->Insufficient(101, false);   //Steven 20150416 : 新的權限設定
        EL<TPanel>("TfContact", "palHeightCalibration")->Enabled   =fSecurity->Insufficient( 93, false);   //jou 2014-03-26 SPIL 要求 Contact mode 權限分級
        EL<TGroupBox>("TfContact", "gbContactForce")->Enabled         =fSecurity->Insufficient( 38, false);   //Steven 20111116
        EL<TPanel>("TfContact", "palTestSocket")->Enabled          =fSecurity->Insufficient(103, false);   //wei 20150417 : 新的權限設定

        if(EL<TEdit>("TfContact", "edContactHeight1")->Enabled==true)                                     //Steven 20221024 : 修正Contact Height的Enable
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled   =fSecurity->Insufficient(171, false);   //Sam 20220117 : 新增一組只鎖 Contact height 的權限
        if(EL<TEdit>("TfContact", "edContactHeight2")->Enabled==true)
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled   =fSecurity->Insufficient(171, false);   //Sam 20220117 : 新增一組只鎖 Contact height 的權限

        if(EL<TEdit>("TfContact", "edReleaseHeight1")->Enabled==true)
            EL<TEdit>("TfContact", "edReleaseHeight1")->Enabled   =fSecurity->Insufficient(173, false);   //Sam 20221207 : 新增一組只鎖 Release height 的權限
        if(EL<TEdit>("TfContact", "edReleaseHeight2")->Enabled==true)
            EL<TEdit>("TfContact", "edReleaseHeight2")->Enabled   =fSecurity->Insufficient(173, false);   //Sam 20221207 : 新增一組只鎖 Release height 的權限
        if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }

    if(IndexHasIC())
    {
        EL<TGroupBox>("TfContact", "gbContactParameter")->Enabled=false;
    }

    if(bEnable_KLT_Function==true && HasICUnderMachine())                       //Steven 20220607 : KLT要求在有料時, 不可以修改Contact Mode
    {
        EL<TGroupBox>("TfContact", "gbContactParameter")->Enabled=false;
    }

    if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DirectContactMode || EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMove)
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=false;                                           //2012-01-03    Dell
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=false;                                           //2012-01-03    Dell
    }
    else
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=true;
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=true;
    }

    if(MachineTypeChoice==Type_HT9046_LS)
    {
        EL<TLabel>("TfContact", "lblPSDT")->Visible=true;
        EL<TRadioGroup>("TfContact", "rgPSDT")->Visible =true;
    }

    EL<TGroupBox>("TfContact", "gbDynaTemp")->Visible=IniConfig.bC04EnableTestTempIC;
    if(IniConfig.bG13ShowTempOffsetOnContact)                                   //JerryYang 20201227 Add bG13ShowTempOffsetOnContact
    {
    ;   // golden cContact.cpp:1518-1519 取代：btTempOffset／btAt->Parent：版面父容器（HTML 不用）
#if 0 // GATE (S12-C save) golden cContact.cpp:1518-1519 -- btTempOffset／btAt->Parent：版面父容器（HTML 不用）
        EL<TButton>("TfContact", "btTempOffset")->Parent=EL<TPanel>("TfContact", "pnlBottom");
        EL<TButton>("TfContact", "btAt")->Parent=EL<TPanel>("TfContact", "pnlBottom");
#endif // GATE (S12-C save)
    }
    else
    {
    ;   // golden cContact.cpp:1523-1524 取代：btTempOffset／btAt->Parent：版面父容器
#if 0 // GATE (S12-C save) golden cContact.cpp:1523-1524 -- btTempOffset／btAt->Parent：版面父容器
        EL<TButton>("TfContact", "btTempOffset")->Parent=EL<TGroupBox>("TfContact", "gbDynaTemp");
        EL<TButton>("TfContact", "btAt")->Parent=EL<TGroupBox>("TfContact", "gbDynaTemp");
#endif // GATE (S12-C save)
    }

    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2當作指紋測試
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                         //kevin 20150127 Arm1 下壓 arm2 測試
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
    {
        EL<TComboBox>("TfContact", "cbContactMode")->Enabled=false;
        EL<TComboBox>("TfContact", "cbVacuumMode")->Enabled=false;
    }

    if(CosFunction.bOnlyUseContactModeDiffSpeed)                                //Steven 20131101 : 強制使用兩段速Contact
    {
        EL<TComboBox>("TfContact", "cbContactMode")->Enabled=false;
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[9])
        {
            EL<TComboBox>("TfContact", "cbContactMode")->Enabled=false;
            EL<TComboBox>("TfContact", "cbVacuumMode")->Enabled=false;
        }

        if(bAuthCriticalPara[10])
        {
            EL<TEdit>("TfContact", "edDropWaitTime")->Enabled=false;
        }

        if(bAuthCriticalPara[12])
        {
            EL<TEdit>("TfContact", "edD41")->Enabled=false;
        }
    }

    if(CosFunction.bFixedDropSpeed)                                             //Steven 20131101 : 使用固定的Drop速度
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=false;                                           //Steven 20131128
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=false;                                           //Steven 20131128
        EL<TEdit>("TfContact", "edDropWaitTime")->Enabled=false;
    }

    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
       TestIF_File.bArm1PickPlaceArm2Test==true)
    {
        EL<TLabel>("TfContact", "lblUpWaitTime")->Visible=true;
        EL<TLabel>("TfContact", "lblUpSpeed")->Visible=true;
        EL<TEdit>("TfContact", "edUpWaitTime")->Visible=true;
        EL<TEdit>("TfContact", "edUpSpeed")->Visible=true;
        EL<TLabel>("TfContact", "lblUpOffset")->Visible=true;
        EL<TEdit>("TfContact", "edUpOffset1")->Visible=true;
        EL<TEdit>("TfContact", "edUpOffset2")->Visible=true;
        EL<TLabel>("TfContact", "lblUpOfsMM")->Visible=true;
    }
    else
    {
        EL<TLabel>("TfContact", "lblUpWaitTime")->Visible=false;
        EL<TLabel>("TfContact", "lblUpSpeed")->Visible=false;
        EL<TEdit>("TfContact", "edUpWaitTime")->Visible=false;
        EL<TEdit>("TfContact", "edUpSpeed")->Visible=false;
        EL<TLabel>("TfContact", "lblUpOffset")->Visible=false;
        EL<TEdit>("TfContact", "edUpOffset1")->Visible=false;
        EL<TEdit>("TfContact", "edUpOffset2")->Visible=false;
        EL<TLabel>("TfContact", "lblUpOfsMM")->Visible=false;
    }

    EL<TPanel>("TfContact", "pnlTorqueControl")->Visible=IniConfig.bControlTorque;                         //jou 2013-11-05 Index Control Torque
    if(IniConfig.bControlTorque)                                                //ChungHung 20140516 因新增Function 畫面上重疊 所以有開啟時再將位置移回來避免 修改時找不到畫面
    {
        EL<TPanel>("TfContact", "pnlTorqueControl")->Top=105;
        EL<TPanel>("TfContact", "pnlTorqueControl")->Left=9;
    }

    if(CUSTOMER_CODE==CC_SCS)                                                   //jou 2016-03-10 SCS客戶要求contact Force per pin N 反白不能修改
    {
        EL<TEdit>("TfContact", "edForcePerPinN")->Enabled=false;
    }

    if((EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContact &&                                //Richard 20220321 : 渠梁Side Push
        EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMoveDrop) &&
       CosFunction.bTesterSidePushFunction==true)
    {
        EL<TPanel>("TfContact", "pnlSidePush")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfContact", "pnlSidePush")->Visible=false;
    }

    if(IniConfig.bAMDFunction)
    {
        EL<TCheckBox>("TfContact", "cbSFCAutoTune")->Visible=false;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbSFCAutoTune")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode ||
                                SHT_FLOATING_CHK!=0);                           //jou 20170214 (Steven) : 修正無安裝CCD版本置偏需關閉function
    }

    EL<TRadioButton>("TfContact", "rbAutoContactTest")->Visible=IniConfig.bD15_AutoContactTest;                  //Steven 20150224 : Auto Contact Test
    EL<TEdit>("TfContact", "edAutoContactTest")->Visible=IniConfig.bD15_AutoContactTest;
    EL<TLabel>("TfContact", "lbAutoContactTest")->Visible=IniConfig.bD15_AutoContactTest;
    EL<TRadioButton>("TfContact", "rbStepContactTest")->Visible=IniConfig.bD16_StepContactTest;                  //Steven 20150811 Start: Step by Step Contact Test
    EL<TButton>("TfContact", "btnStart")->Visible=IniConfig.bD16_StepContactTest;
    EL<TButton>("TfContact", "btnPause")->Visible=IniConfig.bD16_StepContactTest;
    EL<TButton>("TfContact", "btnTStep")->Visible=IniConfig.bD16_StepContactTest;
    EL<TButton>("TfContact", "btnTStart")->Visible=IniConfig.bD16_StepContactTest;
    EL<TSpeedButton>("TfContact", "spbOneCycle")->Visible=IniConfig.bD16_StepContactTest;
    ;   // golden cContact.cpp:1633-1633 取代：ledOneCycle->Value：TMyLed 燈號（替身是 TControl，沒有 Value；燈號是 Step Contact Test 狀態）
#if 0 // GATE (S12-C save) golden cContact.cpp:1633-1633 -- ledOneCycle->Value：TMyLed 燈號（替身是 TControl，沒有 Value；燈號是 Step Contact Test 狀態）
    EL<TControl>("TfContact", "ledOneCycle")->Value=false;
#endif // GATE (S12-C save)
    EL<TControl>("TfContact", "ledOneCycle")->Visible=IniConfig.bD16_StepContactTest;
    EL<TButton>("TfContact", "btOffset")->Visible=IniConfig.bD16_StepContactTest;
    EL<TButton>("TfContact", "btBarcode")->Visible=(CUSTOMER_CODE==CC_KYEC_XILINX &&
                        BAR_CODE_INSTALL==ebctInShtIntel);                      //wei 20160317 Barcode
    if(IniConfig.bChangeKitNoHardStop==false)                                   //jou 2015-12-08 Xilinx 驗證用
    {
        EL<TLabel>("TfContact", "labShtOffset")->Visible=IniConfig.bD16_StepContactTest;
        EL<TEdit>("TfContact", "edShtPickOffset1")->Visible=IniConfig.bD16_StepContactTest;
        EL<TEdit>("TfContact", "edShtPickOffset2")->Visible=IniConfig.bD16_StepContactTest;
        EL<TLabel>("TfContact", "lblShtPickOfsMM")->Visible=IniConfig.bD16_StepContactTest;
    }

    EL<TControl>("TfContact", "OTDTimer")->Enabled=(USE_OTD==1?true:false);                                  //ChungHung 20140709 add by hardware control
    EL<TControl>("TfContact", "TimerEP")->Enabled=(EP_Install==3 || EP_Install==5);                          //Steven 20160630;
    EL<TControl>("TfContact", "timerContact")->Enabled=true;                                                 //JerryYang 20251118 : 機台有IC不能切換模式
    EL<TLabel>("TfContact", "labEPValue")->Visible=false;                                                  //Steven 20160630
    EL<TLabel>("TfContact", "labEPValue_1032")->Visible=false;
    if(CosFunction.bPurgeBeforePickShuttle)                                     //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
    {
        EL<TPanel>("TfContact", "pnlPurge")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfContact", "pnlPurge")->Visible=false;
    }
    EL<TPanel>("TfContact", "palSoakTimeWating")->Visible=false;

#ifdef SOFT_SIMULTE
    EL<TButton>("TfContact", "btnStart")->Visible=true;
    EL<TButton>("TfContact", "btnPause")->Visible=true;
    EL<TButton>("TfContact", "btnTStep")->Visible=true;
    EL<TButton>("TfContact", "btnTStart")->Visible=true;
    EL<TSpeedButton>("TfContact", "spbOneCycle")->Visible=true;                                                  //Steven 20150811 : Step by Step Contact Test
#endif

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160311 客戶要求顯示海思專用版本
    {
        EL<TLabel>("TfContact", "lbHisiVer")->Caption=ATCVersion;
    }
    else
    {
        EL<TLabel>("TfContact", "lbHisiVer")->Caption="";
    }
    EL<TButton>("TfContact", "btTCPIP")->Visible=(TestIF.iTestType==TCP_IP_MODE);                           //wei 20211027 open short TCP/IP
    bSetHasIC=false;                                                            //Steven 20190626 : Fixed 2DID error in contact test

    EL<TLabel>("TfContact", "lblIndexUpSpeed")->Visible=(CUSTOMER_CODE!=CC_ASE_KaohSiung);                 //Steven 20220524 : 開放上升兩段速
    EL<TRadioGroup>("TfContact", "rgIndexUpSpeed")->Visible =(CUSTOMER_CODE!=CC_ASE_KaohSiung);                 //Steven 20220524 : 開放上升兩段速
    EL<TGroupBox>("TfContact", "gbDoubleEPSet")->Visible  =(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI);                            //Ifor 20190104 : Add Double EP Control   //AI(ht9045-v899) 20260430: show Double EP set for Multi EP half.

    if(IniConfig.bD67LoadCellMeasure)                                           //kevin 20190907 Arm 測區次數道量測 功能;
    {
        EL<TRadioButton>("TfContact", "rbLoadCellAutoHigh")->Visible=true;
        EL<TEdit>("TfContact", "edLoadCellHeight1")->Visible=true;
        EL<TLabel>("TfContact", "lblLoadCellHeight")->Visible=true;
        EL<TEdit>("TfContact", "edLoadCellHeight2")->Visible=true;
        EL<TLabel>("TfContact", "lblLoadCellHighMM")->Visible=true;
    }
    else
    {
        EL<TRadioButton>("TfContact", "rbLoadCellAutoHigh")->Visible=false;
        EL<TEdit>("TfContact", "edLoadCellHeight1")->Visible=false;
        EL<TLabel>("TfContact", "lblLoadCellHeight")->Visible=false;
        EL<TEdit>("TfContact", "edLoadCellHeight2")->Visible=false;
        EL<TLabel>("TfContact", "lblLoadCellHighMM")->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        EL<TLabel>("TfContact", "labContactRelative")->Visible=true;
        EL<TEdit>("TfContact", "edContactRelativeZ1")->Visible=true;
        EL<TEdit>("TfContact", "edContactRelativeZ2")->Visible=true;
        EL<TLabel>("TfContact", "lblUpOfsMM")->Visible=true;

        EL<TCheckBox>("TfContact", "cbCalibrateAboveHeight")->Visible=true;
    }
    else
    {
        EL<TLabel>("TfContact", "labContactRelative")->Visible=false;
        EL<TEdit>("TfContact", "edContactRelativeZ1")->Visible=false;
        EL<TEdit>("TfContact", "edContactRelativeZ2")->Visible=false;
        EL<TLabel>("TfContact", "lblUpOfsMM")->Visible=false;

        EL<TCheckBox>("TfContact", "cbCalibrateAboveHeight")->Visible=false;
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[5])
        {
            EL<TControl>("TfContact", "scrbSLK")->Enabled=false;
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->Enabled=false;
        }

        if(bAuthCriticalPara[8])
        {
            EL<TEdit>("TfContact", "edPinCount")->Enabled=false;
            EL<TEdit>("TfContact", "edForcePerPinN")->Enabled=false;
            EL<TEdit>("TfContact", "edForcePerPinG")->Enabled=false;
        }
    }

    if(CosFunction.bHiSiliconFunction==true && IniConfig.bAMDFunction)
    {
        EL<TEdit>("TfContact", "edPinCount")->Enabled=false;
        EL<TEdit>("TfContact", "edForcePerPinN")->Enabled=false;
        EL<TEdit>("TfContact", "edForcePerPinG")->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_AMKOR_Korea &&
       TestIF_File.iGpibMode==InterfaceType_15BinQorvo)
    {
        EL<TLabel>("TfContact", "lblPinCount")->Visible=false;
        EL<TEdit>("TfContact", "edPinCount")->Visible=false;
        EL<TLabel>("TfContact", "lblPerPinN")->Visible=false;
        EL<TEdit>("TfContact", "edForcePerPinN")->Visible=false;
        EL<TEdit>("TfContact", "edForcePerPinG")->Visible=false;
        EL<TLabel>("TfContact", "lblPerGf")->Visible=false;
        EL<TEdit>("TfContact", "edForcePerDeviceKG")->Enabled=true;
    }
    else if(CUSTOMER_CODE==CC_JCET)
    {
        EL<TLabel>("TfContact", "lblPinCount")->Visible=true;
        EL<TEdit>("TfContact", "edPinCount")->Visible=true;
        EL<TLabel>("TfContact", "lblPerPinN")->Visible=true;
        EL<TEdit>("TfContact", "edForcePerPinN")->Visible=true;
        EL<TEdit>("TfContact", "edForcePerDeviceKG")->Enabled=false;
    }
    else if(CosFunction.bOEEFunction)                                           //JimmyChiu 20220120 add
    {
        EL<TEdit>("TfContact", "edPinCount")->Enabled=!fProductionInfo->CheckContactForceExist();
        EL<TEdit>("TfContact", "edForcePerPinN")->Enabled=!fProductionInfo->CheckContactForceExist();
        EL<TEdit>("TfContact", "edForcePerPinG")->Enabled=!fProductionInfo->CheckContactForceExist();
    }
    else
    {
        EL<TLabel>("TfContact", "lblPinCount")->Visible=true;
        EL<TEdit>("TfContact", "edPinCount")->Visible=true;
        EL<TLabel>("TfContact", "lblPerPinN")->Visible=true;
        EL<TEdit>("TfContact", "edForcePerPinN")->Visible=true;
        EL<TEdit>("TfContact", "edForcePerPinG")->Visible=true;
        EL<TLabel>("TfContact", "lblPerGf")->Visible=true;
        EL<TEdit>("TfContact", "edForcePerDeviceKG")->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20221229 : 晶兆成建勳說要增加顯示(±30%)
    {
        EL<TLabel>("TfContact", "lblPerGf")->Caption="gf(±30%)";
    ;   // golden cContact.cpp:1782-1782 取代：lblPerGf->Width：版面寬度
#if 0 // GATE (S12-C save) golden cContact.cpp:1782-1782 -- lblPerGf->Width：版面寬度
        EL<TLabel>("TfContact", "lblPerGf")->Width=65;
#endif // GATE (S12-C save)
    }

    EL<TSpeedButton>("TfContact", "AutoZTeachButton")->Visible=(CUSTOMER_CODE==CC_ASE_KaohSiung);                //kevin 20210504 add into autoteach form
    EL<TButton>("TfContact", "btnTrayMap")->Visible=(USE_TRAY_MAPPING!=etmUninstall && (TestIF_File.bEnableTrayMap || TestIF_File.bEnableDeviceRemain));
    if(IniConfig.bD36EnableRTCAutoModelVerify &&
       CosFunction.bRTCHalfViewAutoVerify)                                      //JerryYang 20220215 : RTC Auto Verify half view check
    {
        EL<TCheckBox>("TfContact", "cbRTCModel")->Caption="Calibration RTC Model+Auto Verify";
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbRTCModel")->Caption="Calibration RTC Model";
    }

    DF_UpdateContactRelative();                                                    //JerryYang 20250120 : add

    if(bUserDefMaxContactHeight==true)                                          //jou 20230111 : 增加 User define max contact height 修改提示
    {
        EL<TLabel>("TfContact", "labbUserDefMaxContactHeight")->Visible=true;
        S.sprintf("*User define max contact height : %3.2f mm", dUserDefMaxContactHeight);
        EL<TLabel>("TfContact", "labbUserDefMaxContactHeight")->Caption=S;
    }
    else
    {
        EL<TLabel>("TfContact", "labbUserDefMaxContactHeight")->Visible=false;
    }

    if(IniConfig.bG22NoticeTakeoutTray)                                         //JerryYang 20231218 : G22提醒人員取tray功能
    {
        bContactModeNeedOpenDoor=true;
    }

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Sam 20240118 : contact 介面下關閉 InitTempOffset 加熱功能
    {
        bUseInitTempOffset=false;
    }
    bDoRTCLearning=false;                                                       //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air
    EL<TCheckBox>("TfContact", "chk_K_Temperature")->Visible=(Tri_Temp_Machine==1)? 1:0;                      //Ztex 2024.03.26 Add Contact Mode K Temperature

    EL<TRadioButton>("TfContact", "rbKTempIndexMove")->Visible=(Tri_Temp_Machine==1)? 1:0;                       //JerryYang 20250711 : TriTemp才顯示
    EL<TRadioButton>("TfContact", "rbDeviceLoopTest")->Visible=(Tri_Temp_Machine==1)? 1:0;                       //JerryYang 20250711 : TriTemp才顯示
    EL<TEdit>("TfContact", "edDeviceLoopTest")->Visible=(Tri_Temp_Machine==1)? 1:0;                       //JerryYang 20250711 : TriTemp才顯示

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cContact.cpp:365  TfContact::ReadFile()
static void DF_ReadFile()
{

    bool bRet=false;
    static bool bFirst=false;                                                   //kevin 20180409 add
    AnsiString szDir=GetRecipeFileName("Contact.Data");
    AnsiString S=GetLastOpenFN();
    AnsiString szDir2="D:\\HT9045\\system\\Contact.ini";
    AnsiString szDir3=GetRecipePath();
    DF_SetIndexDownPos();

    elContact->ReadEditTextFromFile(szDir3, "Contact.Data");

    CONTECT_SHUTTLE_KG=CheckRange(CheckAndReadIniDataGeneral("IndexDriver", "CONTECT_SHUTTLE_KG", 10), 30, 10);         //jou 2011-03-15
    if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                     //Panasonic
        CONTECT_SHUTTLE_KG=CONTECT_SHUTTLE_KG;
    else
        CONTECT_SHUTTLE_KG=CONTECT_SHUTTLE_KG_MITSUBISHI;

//    if(INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG)                  //Steven 20240809 : mark
//        CONTECT_SHUTTLE_KG=CONTECT_SHUTTLE_240KG;                               //Steven 20110310 : 240KG

    if(CosFunction.bContactHeightSaveToContactIni)                              //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
    {
        if(FileExists(szDir2)==false ||
           CheckIniData(szDir2, S, "Test Arm1")==false)                         //檔案不存在或是沒有存檔過的就歸零
        {
            DeviceForm_File.IndexContact[0]             =0.0;
            DeviceForm_File.IndexArmPick[0]             =0.0;
            DeviceForm_File.IndexDrop[0]                =0.0;
            DeviceForm_File.IndexPlace[0]               =0.0;
            DeviceForm_File.IndexContact[1]             =0.0;
            DeviceForm_File.IndexArmPick[1]             =0.0;
            DeviceForm_File.IndexDrop[1]                =0.0;
            DeviceForm_File.IndexPlace[1]               =0.0;
            DeviceForm_File.IndexContactBackUp[0]       =0.0;
            DeviceForm_File.IndexContactBackUp[1]       =0.0;
            DeviceForm_File.IndexContactShuttlePickUp[0]=0.0;
            DeviceForm_File.IndexContactShuttlePickUp[1]=0.0;
        }
        else
        {
            DeviceForm_File.IndexContact[0]             =ReadWriteIni(szDir2, S, "Test Arm1",           1.0, 1.0, true, true, 1.0, fIndexDownPos);              //Steven 20211109 : -148 --> fIndexDownPos
            DeviceForm_File.IndexArmPick[0]             =ReadIniData(szDir2, S, "Pick Up1",             0.0);
            DeviceForm_File.IndexDrop[0]                =ReadIniData(szDir2, S, "Drop1",                0.0);
            DeviceForm_File.IndexPlace[0]               =ReadIniData(szDir2, S, "Place1",               0.0);
            DeviceForm_File.IndexContact[1]             =ReadWriteIni(szDir2, S, "Test Arm2",           1.0, 1.0, true, true, 1.0, fIndexDownPos);              //Steven 20211109 : -148 --> fIndexDownPos
            DeviceForm_File.IndexArmPick[1]             =ReadIniData(szDir2, S, "Pick Up2",             0.0);
            DeviceForm_File.IndexDrop[1]                =ReadIniData(szDir2, S, "Drop2",                0.0);
            DeviceForm_File.IndexPlace[1]               =ReadIniData(szDir2, S, "Place2",               0.0);
            DeviceForm_File.IndexContactBackUp[0]       =ReadIniData(szDir2, S, "ContactBackUp1",       0.0);
            DeviceForm_File.IndexContactBackUp[1]       =ReadIniData(szDir2, S, "ContactBackUp2",       0.0);
            DeviceForm_File.IndexContactShuttlePickUp[0]=ReadIniData(szDir2, S, "ShuttlePickBackUp1",   0.0);
            DeviceForm_File.IndexContactShuttlePickUp[1]=ReadIniData(szDir2, S, "ShuttlePickBackUp2",   0.0);
        }
    }
    else
    {
        DeviceForm_File.IndexContact[0]             =ReadWriteIni(szDir, "Test Arm1", "Contact", 1.0, 1.0, true, true, 1.0, fIndexDownPos);                     //kevin 20180402 0.0 會撞機
        DeviceForm_File.IndexContact[1]             =ReadWriteIni(szDir, "Test Arm2", "Contact", 1.0, 1.0, true, true, 1.0, fIndexDownPos);                     //kevin 20180402 0.0 會撞機
        DeviceForm_File.IndexArmPick[0]             =ReadIniData(szDir, "Test Arm1", "Pick Up", 0.0);
        DeviceForm_File.IndexPlace[0]               =ReadIniData(szDir, "Test Arm1", "Place",   0.0);
        DeviceForm_File.IndexArmPick[1]             =ReadIniData(szDir, "Test Arm2", "Pick Up", 0.0);
        DeviceForm_File.IndexPlace[1]               =ReadIniData(szDir, "Test Arm2", "Place",   0.0);
        //ChungHung 20130715 add ATK 顯示上次AutoHeight的值
        DeviceForm_File.IndexContactBackUp[0]       =ReadIniData(szDir, "Test Arm1", "ContactBackUp", 0.0);
        DeviceForm_File.IndexContactBackUp[1]       =ReadIniData(szDir, "Test Arm2", "ContactBackUp", 0.0);

        //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
        DeviceForm_File.IndexContactShuttlePickUp[0]=ReadIniData(szDir, "Test Arm1", "ShuttlePickBackUp", 0.0);
        DeviceForm_File.IndexContactShuttlePickUp[1]=ReadIniData(szDir, "Test Arm2", "ShuttlePickBackUp", 0.0);

        //Sam 20190225 : 力成 DirectContact & DropContact 切換後，DropContact 的 Drop 不要變成預設值
        //==>
        if(DeviceForm_File.ContactMode==DropContact && CUSTOMER_CODE==CC_PTI)
        {
            DeviceForm_File.IndexDrop[1]     =ReadIniData(szDir, "Test Arm2", "Drop_DropContactMode",    2.0);
            DeviceForm_File.IndexDrop[0]     =ReadIniData(szDir, "Test Arm1", "Drop_DropContactMode",    2.0);
        }
        else
        {
            DeviceForm_File.IndexDrop[1]     =ReadIniData(szDir, "Test Arm2", "Drop",    0.0);
            DeviceForm_File.IndexDrop[0]     =ReadIniData(szDir, "Test Arm1", "Drop",    0.0);
        }
        //<==
        //Sam 20190225 : 力成 DirectContact & DropContact 切換後，DropContact 的 Drop 不要變成預設值
    }

    DeviceForm_File.dLoadCellZ1Down  =ReadIniData(szDir, "Test Arm1", "LoadCellZ1", 0.0);                               //kevin 20190907 add loadcell Z pos
    DeviceForm_File.dLoadCellZ2Down  =ReadIniData(szDir, "Test Arm2", "LoadCellZ2", 0.0);                               //kevin 20190907 add loadcel2 Z pos

    DeviceForm_File.DropWait         =ReadIniData(szDir, "Wait Time", "Drop Wait",      0.0);
    DeviceForm_File.DropSpeed        =ReadIniData(szDir, "Wait Time", "Drop Speed",     30.0);                          //Eliot 2011_0318
    DeviceForm_File.ContactMode      =ReadIniData(szDir, "Mode",      "Contact",        0.0);

    DeviceForm_File.UpWait           =ReadIniData(szDir, "Wait Time", "Up Wait",      0.0);
    DeviceForm_File.UpSpeed          =ReadIniData(szDir, "Wait Time", "Up Speed",     0.0);
    DeviceForm_File.IndexUp[0]       =ReadIniData(szDir, "Test Arm1", "Up",    0.0);
    DeviceForm_File.IndexUp[1]       =ReadIniData(szDir, "Test Arm2", "Up",    0.0);

    DeviceForm_File.dSitePushWaitTime=ReadIniData(szDir, "Wait Time", "Side Push Wait Time",      0.0);                 //Richard 20220321 : 渠梁Side Push

    if(CosFunction.bAutoHeightSHTReleaseByFile)                                 //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
    {
        DeviceForm_File.iAutoHeightSHTReleaseOfs=ReadIniData(szDir, "Mode", "AutoKSHTOfs", 2.0);
    }

    if(CosFunction.bOnlyUseContactModeDiffSpeed)                                //Steven 20131101 : 強制使用兩段速Contact
    {
        DeviceForm_File.ContactMode=DirectContactModeDiffentSpeed;
    }

    if(CosFunction.bFixedDropSpeed)                                             //Steven 20131101 : 使用固定的Drop速度
    {
        if(DeviceForm_File.DropSpeed>CosFunction.iFixedDropSpeed)
            DeviceForm_File.DropSpeed=CosFunction.iFixedDropSpeed;

        if(DeviceForm_File.IndexDrop[0]<CosFunction.dLimitMinDropOffset)
            DeviceForm_File.IndexDrop[0]=CosFunction.dLimitMinDropOffset;

        if(DeviceForm_File.IndexDrop[1]<CosFunction.dLimitMinDropOffset)
            DeviceForm_File.IndexDrop[1]=CosFunction.dLimitMinDropOffset;
    }

    DeviceForm_File.VacuumMode       =ReadIniData(szDir, "Mode",      "Vacuum",         0.0);

    if(CUSTOMER_CODE==CC_KYEC_CHEN && IniConfig.bA16ContactTestDropContact)     //wei 20150831
    {
        DeviceForm_File.ContactMode=0;
        DeviceForm_File.VacuumMode=1;
        DeviceForm_File.DropWait=0.5;
    }

    if(TestIF_File.bUseSLKClamp && TestIF_File.iSeparabilityTest==1)            //JerryYang 20180518 (wei) 移到上面
    {
        DeviceForm_File.ContactMode=TMove;
        DeviceForm_File.VacuumMode=1;
    }

    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2當作指紋測試
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true))                      //kevin 20150127 Arm1 下壓 arm2 測試  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
    {
        DeviceForm_File.ContactMode=TMoveDrop;
        DeviceForm_File.VacuumMode=1;
    }

    //jou 2013-05-08 修正舊的工作檔選擇 Direct Contact Mode 但是Drop不等於0，會跑Drop mode
    if(DeviceForm_File.ContactMode==DirectContactMode || DeviceForm_File.ContactMode==TMove)                            //JerryYang 20180518 : 新增保護
    {
        DeviceForm_File.IndexDrop[0]=0;
        DeviceForm_File.IndexDrop[1]=0;
    }
    DeviceForm_File.DummyMode        =ReadIniData(szDir, "Mode",      "Dummy Contact",  0.0);
    DeviceForm_File.iHeadDeviceCT    =ReadIniData(szDir, "Mode",      "Head Device Mode", 0);

    if(bFirst==true && TestIF_File.iTestMode==SingleSite)                       //JerryYang 20171215 (Steven) Single site只支援浮動頭1對1或是2對1
    {
        if(DeviceForm_File.iHeadDeviceCT!=2 && DeviceForm_File.iHeadDeviceCT!=5)
        {
            DeviceForm.iHeadDeviceCT=2;
            DeviceForm_File.iHeadDeviceCT=2;
            WriteIniData(szDir, "Mode", "Head Device Mode", DeviceForm_File.iHeadDeviceCT);
        }
    }
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position=DeviceForm_File.iHeadDeviceCT;   // golden cContact.cpp:529-529 取代：scrbSLK->Position（ELTrackBar）
#if 0 // GATE (S12-C save) golden cContact.cpp:529-529 -- scrbSLK->Position（ELTrackBar）
    EL<TControl>("TfContact", "scrbSLK")->Position               =DeviceForm_File.iHeadDeviceCT;             //Steven 20140627 : 避免程式啟動時, EP會錯
#endif // GATE (S12-C save)
    DeviceForm_File.dKitDiameter    =ReadIniData(szDir, "Mode",      "Kit Diameter",     3.0);                          //Steven 20110502 : 240KG

    DeviceForm_File.iKitDiameterMode                = ReadIniData(szDir, "Mode",      "KitDiameterMode",   999);        //kevin 20210813 Kit 外徑 選擇  10 尚未有資料
    if(DeviceForm_File.iKitDiameterMode==999)                                   //kevin 20210827 add
    {
        if(DeviceForm_File.dKitDiameter==3)
            DeviceForm_File.iKitDiameterMode=0;
        else if(DeviceForm_File.dKitDiameter==4)
            DeviceForm_File.iKitDiameterMode=1;
        else if(DeviceForm_File.dKitDiameter==5.6)
            DeviceForm_File.iKitDiameterMode=2;
        else if(DeviceForm_File.dKitDiameter==8 || DeviceForm_File.dKitDiameter==40.2)
            DeviceForm_File.iKitDiameterMode=3;
        else
            DeviceForm_File.iKitDiameterMode=4;
    }

//    DeviceForm_File.dPress           =ReadIniData(szDir, "Torque Control", "Torque",        0.0); //jou 2014-09-06 修正開啟程式的時候EP異常
    DeviceForm_File.iPinCT           =ReadIniData(szDir, "Torque Control", "Pin Number",    0.0);
    DeviceForm_File.XDimension       =ReadIniData(szDir, "Torque Control", "X Dimension",   0.0);
    DeviceForm_File.YDimension       =ReadIniData(szDir, "Torque Control", "Y Dimension",   0.0);
    DeviceForm_File.dDieForceKitDiameter    =ReadIniData(szDir, "Mode",      "Die Force Kit Diameter",     2.0);        //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    //==> Eastsun 20260526 #026-1.21 Ifor 20220218 add:KYEC 要求新增Dual Force 開關
    DeviceForm_File.bUseDieForce            =CheckAndReadIniData(szDir, "Mode",      "bUseDieForce",              false);
    //<== Eastsun 20260526 #026-1.21

#if 0 // GATE (S12-C save) golden cContact.cpp:556-561 -- REAL_TIME_CCD ROI size serial send via COM2 (TCOM2Shim has no sRealTimeCom_Send/SendCommToVision) -- hardware write, same GATE as forms/fContact.cpp G-CT-RTCROI
    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                              //jou 20180724 : 新增RTC ROI size command
    {
        S.sprintf("@SIZE%sX%s+",FormatFloat("00.0",DeviceForm_File.XDimension),FormatFloat("00.0",DeviceForm_File.YDimension));
        COM2->sRealTimeCom_Send[COM2->rtROIsize] = S;
        COM2->SendCommToVision(COM2->rtROIsize, true);
    }
#endif // GATE (S12-C save)

    if(CosFunction.bFixNameOfForcePerPinG)                                      //Steven 20240821 : 凌中心說要修正存檔名稱
    {
        bRet=CheckKeyExist(szDir, "Torque Control", "Force Per Pin N");
        if(bRet==false)
        {
            DeviceForm_File.ForcePerPinN=ReadIniData(szDir, "Torque Control", "Force Per Pin", 0.0);
            WriteIniData(szDir, "Torque Control", "Force Per Pin N", DeviceForm_File.ForcePerPinN);
        }
        DeviceForm_File.ForcePerPinN=ReadIniData(szDir, "Torque Control", "Force Per Pin N", 0.0);

        bRet=CheckKeyExist(szDir, "Torque Control", "Force Per Pin G");
        if(bRet==false)
        {
            if(CUSTOMER_CODE==CC_JCET || CUSTOMER_CODE==CC_AMKOR_China)         //Steven 20230110 : Pin force 只卡JCET & ATC
                DeviceForm_File.ForcePerPinG=ReadWriteIni(szDir, "Torque Control", "Force Per Pin Kg", DeviceForm_File.ForcePerPinN*1000.0/9.8, DeviceForm_File.ForcePerPinN*1000.0/9.8, true, true, InputLimit.dForcePerpinHigh, InputLimit.dForcePerpinLow);  //Steven 20220216 : Pinforce加上上下限
            else
                DeviceForm_File.ForcePerPinG=ReadIniData(szDir, "Torque Control", "Force Per Pin Kg", 30.0);

            WriteIniData(szDir, "Torque Control", "Force Per Pin G",  DeviceForm_File.ForcePerPinG);
        }

        if(CUSTOMER_CODE==CC_JCET || CUSTOMER_CODE==CC_AMKOR_China)             //Steven 20230110 : Pin force 只卡JCET & ATC
            DeviceForm_File.ForcePerPinG=ReadWriteIni(szDir, "Torque Control", "Force Per Pin G", DeviceForm_File.ForcePerPinN*1000.0/9.8, DeviceForm_File.ForcePerPinN*1000.0/9.8, true, true, InputLimit.dForcePerpinHigh, InputLimit.dForcePerpinLow);  //Steven 20220216 : Pinforce加上上下限
        else
            DeviceForm_File.ForcePerPinG=ReadIniData(szDir, "Torque Control", "Force Per Pin G", 30.0);
    }
    else
    {
        DeviceForm_File.ForcePerPinN      =ReadIniData(szDir, "Torque Control", "Force Per Pin", 0.0);
        if(CUSTOMER_CODE==CC_JCET || CUSTOMER_CODE==CC_AMKOR_China)             //Steven 20230110 : Pin force 只卡JCET & ATC
            DeviceForm_File.ForcePerPinG=ReadWriteIni(szDir, "Torque Control", "Force Per Pin Kg", DeviceForm_File.ForcePerPinN*1000.0/9.8, DeviceForm_File.ForcePerPinN*1000.0/9.8, true, true, InputLimit.dForcePerpinHigh, InputLimit.dForcePerpinLow);  //Steven 20220216 : Pinforce加上上下限
        else
            DeviceForm_File.ForcePerPinG=ReadIniData(szDir, "Torque Control", "Force Per Pin Kg", 30.0);
    }
    DeviceForm_File.iIndexTorqueMax  =CheckAndReadIniData(szDir, "Torque Control", "iIndexTorqueMax", 120);
    DeviceForm_File.iIndexTorqueCmp  =CheckAndReadIniData(szDir, "Torque Control", "iIndexTorqueCmp", 30);
    DeviceForm_File.bUseAddWeight    =CheckAndReadIniData(szDir, "Torque Control", "bUseAddWeight", false);             //kevin 20170629 (Steven) direct heat add weight

    DeviceForm_File.DoubleForce      =ReadIniData(szDir, "Torque Control", "Double Force", 3.0);                        //Ifor 20190104 : Add Double EP Control
    DeviceForm_File.iPinOfDie        =ReadIniData(szDir, "Torque Control", "Pin of Die", 10);                           //Steven 20191106: Pin of Die

    if(IniConfig.bKoreaFunction==true ||                                        //ChungHung 20140716 解Mark
       CUSTOMER_CODE==CC_TSMC_TAINAN ||                                         //wei 20151117 傳送ContactForce設定值
       CUSTOMER_CODE== CC_ASE_KaohSiung ||                                      //kevin 20131216
       IniConfig.bSPILFunction==true)                                           //JerryYang 20230204 : Add SPIL support回傳GPIB force指令
    {
//        asArmForce1=DeviceForm_File.iPinCT * DeviceForm_File.ForcePerPinN/9.8;
//        asArmForce2=DeviceForm_File.iPinCT * DeviceForm_File.ForcePerPinN/9.8;
        double dGpibForce = DF_CalcDeviceForce(DeviceForm_File.iPinCT,             //AI(ht9045-contact-force) 20260624 (RogerYang) : ★這裡決定送Kgf(true)還是N(false)
                                            DeviceForm_File.ForcePerPinN,
                                            DeviceForm_File.ForcePerPinG,
                                            true);
        asArmForce1=FormatFloat("0.0000", dGpibForce);
        asArmForce2=FormatFloat("0.0000", dGpibForce);
    }

    DeviceForm_File.dZ1Torue        =ReadIniData(szDir, "Torque Control", "dZ1Torue",      0.0);                        //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    DeviceForm_File.dZ2Torue        =ReadIniData(szDir, "Torque Control", "dZ2Torue",      0.0);

    int iItem=0;                                                                //Frank 20171030 add Enable Universal Kit
    if(CosFunction.bUniversalKit)                                               //JerryYang 20171102 (wei) UniversalKit改成客戶功能
    {
        iItem=ReadIniData(szDir, "Torque Control", "Enable Use Universal Kit", 0);
        if(iItem==0)
            DeviceForm_File.bEnableUseUniversalShuttle=false;
        else
            DeviceForm_File.bEnableUseUniversalShuttle=true;
    }
    else
    {
        DeviceForm_File.bEnableUseUniversalShuttle=false;
    }

    iItem=ReadIniData(szDir, "Mode", "Suck Shuttle Device After Tested", 0);    //Ifor 20180116 (Steven) : 被刪除先補回去
    if(iItem==0)
        DeviceForm_File.bSuckShuttleDeviceAfterTested=false;
    else
        DeviceForm_File.bSuckShuttleDeviceAfterTested=true;

    iItem=ReadIniData(szDir, "Mode", "Suck Shuttle Device Wait On Shuttle", 0);                                         //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
    if(iItem==0)
        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle=false;
    else
        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle=true;

    DeviceForm_File.iSocketInitialICCheckPosition=ReadIniData(szDir, "Mode","iSocketInitialICCheckPosition",0);

    DeviceForm_File.dDropByPassDetect=CheckRange(ReadIniData(szDir, "Mode", "dDropByPassDetect", 0.00), 20.0, 0.0);

    double f=ReadIniData(szDir, "Mode", "fSocketInitialICCheckPositionOffset", 0.00);
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        DeviceForm_File.fSocketInitialICCheckPositionOffset=CheckRange(f, 5.0, 0.0);                                    //wei 20160309 Check Position Offset 最大值5
    }
    else
    {
        DeviceForm_File.fSocketInitialICCheckPositionOffset=CheckRange(f, 20.0, 0.0);                                   //Ifor 20160610 CC_KYEC_LEE 以外客戶 Check Position Offset 最大值20
    }
    WriteIniData(szDir, "Mode", "fSocketInitialICCheckPositionOffset", DeviceForm_File.fSocketInitialICCheckPositionOffset);

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170912 (Steven) 矽品世明要求強制關閉shuttle wait outside功能
    {
        iItem=0;
    }
    else
    {
        iItem=ReadIniData(szDir, "Mode", "Shuttle Waiting Out Site Chamber", 0);                                        //jou 980930 add : Shuttle Waiting Out Site Chamber
    }

    if(iItem==0)
        DeviceForm_File.bShuttleWaitingOutSiteChamber=false;
    else
        DeviceForm_File.bShuttleWaitingOutSiteChamber=true;

    if(CosFunction.bOEEFunction && fProductionInfo!=NULL)                       //Steven 20180417 (Jou) : OEE功能
    {
        fProductionInfo->dOriginal_Device_Pin_Force=ReadIniData(szDir, "Torque Control", "Original Force Per Pin Kg", 0);
        fProductionInfo->dOriginal_Device_Pin_Count=ReadIniData(szDir, "Torque Control", "Original Pin Number", 0);
    }

    //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue start
    if(CosFunction.bPurgeBeforePickShuttle)                                     //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
    {
        iItem=ReadIniData(szDir, "Mode", "Air Purge Before Pick From Shuttle", 0);
        if(iItem==0) DeviceForm_File.bPurgeBeforePickShuttle=false;
        else         DeviceForm_File.bPurgeBeforePickShuttle=true;

        DeviceForm_File.iPurgeBeforePickShuttleTime = ReadIniData(szDir, "Mode", "Air Purge Before Pick Shuttle Time", 1);
        DeviceForm_File.iPurgeBeforePickShuttleInterval = ReadIniData(szDir, "Mode", "Air Purge Before Pick Shuttle Interval", 1);
        DeviceForm_File.iPurgeBdforePickShuttleOffSet = ReadIniData(szDir, "Mode", "Air Purge Before Pick Shuttle OffSet", 1);
    }
    //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue end
    DeviceForm_File.bTesterSidePush = ReadIniData(szDir, "Mode", "Tester Side Push", 0);                                //JerryYang 20241118 : fix    //Richard 20220321
    DeviceForm_File.iSidePushMode   = ReadIniData(szDir, "Mode", "Tester Side Push Mode", 0);                           //Richard 20220321
    EL<TEdit>("TfContact", "edTestContactCount")->Text=ReadIniData(szDir, "Height Calibration", "Test Contact Count", AnsiString("1"));           //KevinCheng 20260115 : NV Contact Test
    EL<TEdit>("TfContact", "edTestSec")->Text=ReadIniData(szDir, "Height Calibration", "Test Sec", AnsiString("1"));                              //KevinCheng 20260115 : NV Contact Test
    EL<TEdit>("TfContact", "edAutoContactTest")->Text=ReadIniData(szDir, "Height Calibration", "Auto Contact Test Count", AnsiString("4"));       //Steven 20150224 : Auto Contact Test
    if(atoi(EL<TEdit>("TfContact", "edAutoContactTest")->Text.c_str())<1)                                 //Steven 20180619 (Jou)
        EL<TEdit>("TfContact", "edAutoContactTest")->Text="1";

    if(atoi(EL<TEdit>("TfContact", "edTestContactCount")->Text.c_str())<1)                                //KevinCheng 20260115 : NV Contact Test
        EL<TEdit>("TfContact", "edTestContactCount")->Text="1";
    if(atoi(EL<TEdit>("TfContact", "edTestSec")->Text.c_str())<1)                                         //KevinCheng 20260115 : NV Contact Test
        EL<TEdit>("TfContact", "edTestSec")->Text="1";
    if(atoi(EL<TEdit>("TfContact", "edTestSec")->Text.c_str())>60)                                        //KevinCheng 20260115 : NV Contact Test
        EL<TEdit>("TfContact", "edTestSec")->Text="60";

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Steven 20220524
    {
        DeviceForm_File.bIndexUpSpeed=false;
    }
    else
    {
        iItem=ReadIniData(szDir, "Mode", "IndexUpSpeed", 0);                    //kevin 20170524 (wei) add index up speed
        if(iItem==0)
            DeviceForm_File.bIndexUpSpeed=false;
        else
            DeviceForm_File.bIndexUpSpeed=true;
    }

    DeviceForm_File.DieForcePerPinN     =ReadIniData(szDir, "Torque Control", "Die Force Per Pin", 0.0);
    DeviceForm_File.DieForcePerPinG     =ReadIniData(szDir, "Torque Control", "Die Force Per Pin Kg", 30.0);
    bFirst=true;                                                                //kevin 20180409 add

}

// golden cContact.cpp:18919  TfContact::SetIndexDownPos()
static void DF_SetIndexDownPos()
{

    if(bUserDefMaxContactHeight)                                                //Steven 20180412 : For new HT-9045高度較高
    {
        fIndexDownPos=dUserDefMaxContactHeight;
    }
    else if(MachineTypeChoice==Type_HT9046_LS)
    {
        fIndexDownPos=fIndexDownPos_for9046LS;
        if(CUSTOMER_CODE==CC_QUALCOMM && ATC_MixMode==eMixATC60_ATC33)          //Ifor 20260121 add
        {
            fIndexDownPos=fIndexDownPos-20;
        }
    }
    else if(ATC_SYSTEM!=eATCUninstall)                                          //wei 20160303 HT-9045HA -146
    {
        fIndexDownPos=fIndexDownPos_forATC;
    }
    else
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE ||
           CUSTOMER_CODE==CC_KYEC_CHEN ||
           CUSTOMER_CODE==CC_KYEC_JCTHIU ||
           CUSTOMER_CODE==CC_DL_TEK   ||
           CUSTOMER_CODE==CC_KYEC_XILINX)                                       //Ifor 20161117 Kyec Z擋版往上拉5mm Z需多下降5mm  -135-5 =-140
        {
            fIndexDownPos=fIndexDownPos_for9045-5;
        }
        else
        {
            fIndexDownPos=fIndexDownPos_for9045;
        }
    }

}

// golden cContact.cpp:23055  TfContact::CalcDeviceForce(double dPinCount, double dForcePerPinN, double dForcePerPinGf, bool bReturnKgf)
static double DF_CalcDeviceForce(double dPinCount, double dForcePerPinN, double dForcePerPinGf, bool bReturnKgf)
{

    if(bReturnKgf)
        return dPinCount * dForcePerPinGf * 0.001;                              //gf -> Kgf，與edForcePerDeviceKG同公式
    else
        return dPinCount * dForcePerPinN;                                       //N，與edForcePerDeviceN同公式

}

// golden cContact.cpp:728  TfContact::DoIniDataToForm()
static void DF_DoIniDataToForm()
{

    AnsiString szDir="", sBuffer="";
    szDir=fOffSet->GetOffsetPath();                                             //Steven 20190109 : 整合Offset路徑
    MyForceDirectories(szDir);
    szDir+="\\Position Offset.Data";
    elContact->InitialDataToEdit();
    EL<TEdit>("TfContact", "edPickUp1")->Text             =FormatFloat("0.00", DeviceForm_File.IndexArmPick[0]);
    EL<TEdit>("TfContact", "edReleaseHeight1")->Text      =FormatFloat("0.00", DeviceForm_File.IndexPlace[0]);
    EL<TEdit>("TfContact", "edContactHeight1")->Text      =FormatFloat("0.00", DeviceForm_File.IndexContact[0]);
    EL<TEdit>("TfContact", "edPickUp2")->Text             =FormatFloat("0.00", DeviceForm_File.IndexArmPick[1]);
    EL<TEdit>("TfContact", "edReleaseHeight2")->Text      =FormatFloat("0.00", DeviceForm_File.IndexPlace[1]);
    EL<TEdit>("TfContact", "edContactHeight2")->Text      =FormatFloat("0.00", DeviceForm_File.IndexContact[1]);

    asContactHeight[0]=EL<TEdit>("TfContact", "edContactHeight1")->Text;
    asContactHeight[1]=EL<TEdit>("TfContact", "edContactHeight2")->Text;

    if(CosFunction.bAutoHeightSHTReleaseByFile)                                 //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
    {
        EL<TEdit>("TfContact", "edAutoKSHTReleaseOfs")->Text=FormatFloat("0.00", DeviceForm_File.iAutoHeightSHTReleaseOfs);
    }

    if(CosFunction.bFixedDropSpeed)                                             //Steven 20131101 : 使用固定的Drop速度
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Text="2.0";
        EL<TEdit>("TfContact", "edDropOffset2")->Text="2.0";
    }
    else
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Text=FormatFloat("0.00", DeviceForm_File.IndexDrop[0]);
        EL<TEdit>("TfContact", "edDropOffset2")->Text=FormatFloat("0.00", DeviceForm_File.IndexDrop[1]);
    }

    EL<TEdit>("TfContact", "edDropWaitTime")->Text        =FormatFloat("0.00", DeviceForm_File.DropWait);
    EL<TEdit>("TfContact", "edDropSpeed")->Text           =FormatFloat("0.00", DeviceForm_File.DropSpeed);                                        //Eliot 2011_0318

    //Ifor 20210111 add:
    EL<TEdit>("TfContact", "edUpWaitTime")->Text        =FormatFloat("0.00", DeviceForm_File.UpWait);

    EL<TEdit>("TfContact", "edSidePushWaitTime")->Text  =FormatFloat("0.00", DeviceForm_File.dSitePushWaitTime);                                  //Richard 20220321 : 渠梁Side Push
    EL<TEdit>("TfContact", "edUpSpeed")->Text           =DeviceForm_File.UpSpeed;
    EL<TEdit>("TfContact", "edUpOffset1")->Text         =FormatFloat("0.00", DeviceForm_File.IndexUp[0]);
    EL<TEdit>("TfContact", "edUpOffset2")->Text         =FormatFloat("0.00", DeviceForm_File.IndexUp[1]);

    if(IniConfig.bControlTorque)                                                //jou 2013-11-05 Index Control Torque
    {
        EL<TEdit>("TfContact", "edtTorqueMax")->Text=DeviceForm_File.iIndexTorqueMax;
        EL<TEdit>("TfContact", "edtTorqueCmp")->Text=DeviceForm_File.iIndexTorqueCmp;
    }

    if(CosFunction.bShowLastShuttleHigh)                                        //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    {
        EL<TLabel>("TfContact", "lblOrgPick")->Visible=true;
        EL<TEdit>("TfContact", "edOrgPick1")->Visible=true;
        EL<TEdit>("TfContact", "edOrgPick2")->Visible=true;
        EL<TLabel>("TfContact", "lblOrgPickMM")->Visible=true;
    }
    else
    {
        EL<TLabel>("TfContact", "lblOrgPick")->Visible=false;
        EL<TEdit>("TfContact", "edOrgPick1")->Visible=false;
        EL<TEdit>("TfContact", "edOrgPick2")->Visible=false;
        EL<TLabel>("TfContact", "lblOrgPickMM")->Visible=false;
    }

    if(CosFunction.bShowLastContactHigh)                                        //Steven 20140409 : 顯示之前的Contact高度
    {
        EL<TLabel>("TfContact", "lblOrgHeight")->Visible=true;
        EL<TEdit>("TfContact", "edContactBackUp1")->Visible=true;
        EL<TEdit>("TfContact", "edContactBackUp2")->Visible=true;
        EL<TLabel>("TfContact", "lblOrgHeightMM")->Visible=true;
    }
    else
    {
        EL<TLabel>("TfContact", "lblOrgHeight")->Visible=false;
        EL<TEdit>("TfContact", "edContactBackUp1")->Visible=false;
        EL<TEdit>("TfContact", "edContactBackUp2")->Visible=false;
        EL<TLabel>("TfContact", "lblOrgHeightMM")->Visible=false;
    }
    EL<TEdit>("TfContact", "edContactBackUp1")->Text  =FormatFloat("0.00", DeviceForm_File.IndexContactBackUp[0]);
    EL<TEdit>("TfContact", "edContactBackUp2")->Text  =FormatFloat("0.00", DeviceForm_File.IndexContactBackUp[1]);

    EL<TEdit>("TfContact", "edOrgPick1")->Text        =FormatFloat("0.00", DeviceForm_File.IndexContactShuttlePickUp[0]);                         //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    EL<TEdit>("TfContact", "edOrgPick2")->Text        =FormatFloat("0.00", DeviceForm_File.IndexContactShuttlePickUp[1]);

    if(TestIF_File.iTestMode!=_8Site1X4)                                        //ChungHung 20150528 add for 海思 _8Site1x4
    {
        if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropPlaceShiftContact)
            filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DirectContactMode);
    }
    else
    {
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DropPlaceShiftContact);
    }

    filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DeviceForm_File.ContactMode);
    filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbVacuumMode"), DeviceForm_File.VacuumMode);
    EL<TRadioButton>("TfContact", "rbNormal")->Checked           =!DeviceForm_File.DummyMode;
    EL<TRadioButton>("TfContact", "rbDummyMode")->Checked        =DeviceForm_File.DummyMode;
    EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position=DeviceForm_File.iHeadDeviceCT;   // golden cContact.cpp:827-827 取代：scrbSLK->Position（ELTrackBar）
#if 0 // GATE (S12-C save) golden cContact.cpp:827-827 -- scrbSLK->Position（ELTrackBar）
    EL<TControl>("TfContact", "scrbSLK")->Position           =DeviceForm_File.iHeadDeviceCT;
#endif // GATE (S12-C save)

    EL<TPanel>("TfContact", "palTorqueArm1")->Visible=false;
    EL<TPanel>("TfContact", "palTorqueArm2")->Visible=false;
    if(CosFunction.bUniversalKit)                                               //JerryYang 20171102 (wei) UniversalKit改成客戶功能
    {
        EL<TCheckBox>("TfContact", "cbEnableUK")->Checked=DeviceForm_File.bEnableUseUniversalShuttle;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbEnableUK")->Visible=false;
    }

    if(CosFunction.bUseDynamicKitDiameter==false)
    {
        if(DeviceForm_File.dKitDiameter==6.0)                                   //Steven 20110502 : 240KG
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=2;
        else if(DeviceForm_File.dKitDiameter==4.0)
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=1;
        else if(DeviceForm_File.dKitDiameter==5.6)                              //wei 20151005 add 56mm
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=3;
        else
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=0;
    }
    else                                                                        //Steven 20170605 (wei) : 可以自定義Kit直徑
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Count>=4)                                      //kevin 20200722 display 4 太窄
    ;   // golden cContact.cpp:854-854 取代：rgKitDiameter->Height：版面高度（HTML 不用）
#if 0 // GATE (S12-C save) golden cContact.cpp:854-854 -- rgKitDiameter->Height：版面高度（HTML 不用）
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->Height=45;
#endif // GATE (S12-C save)
        for(int i=0; i<EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Count; i++)
        {
            double d=atof(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[i].c_str())/10.0;
            sBuffer=EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[i].c_str();
            if(sBuffer=="40x2")
                d=40.2;

            if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //Ifor 20191106 : add KYEC 缸徑 28 40 58 56
            {
                if((d==2.8 && DeviceForm_File.dKitDiameter==3)  ||
                   (d==5.8 && DeviceForm_File.dKitDiameter==6)  ||
                   (d==DeviceForm_File.dKitDiameter)            )
                {
                    EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=i;
                }
            }
            else
            {
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20210827 change
                {
                    if((d==3.0 && DeviceForm_File.dKitDiameter==3)  ||          //40
                       (d==4.0 && DeviceForm_File.dKitDiameter==4)  ||          //55
                       (d==6.0 && DeviceForm_File.dKitDiameter==6))             //75
                    {
                        EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=i;
                        DeviceForm_File.iKitDiameterMode=EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex;                                      //kevin 20210813 Kit 外徑 選擇
                        EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex=i;
                        break;
                    }
                    else
                    {
                        if(i>=3)
                        {
                            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=i;
                            DeviceForm_File.iKitDiameterMode=EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex;                                  //kevin 20210813 Kit 外徑 選擇
                            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex=i;
                        }
                    }
                }
                else
                {
                    if(d==DeviceForm_File.dKitDiameter)
                    {
                        EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=i;
                        if(bKitOutDiameter)                                     //kevin 20170628 (Steven) add for ASE-KH
                        {
                            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex=i;
                        }
                    }
                }
            }
        }

        if((DeviceForm_File.dKitDiameter==8 ||
            DeviceForm_File.dKitDiameter==40.2) &&
            DeviceForm_File.dPress>240)
        {
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex=EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex+1;
        }

        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==-1)
        {
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=0;
            EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex=0;                                      //kevin 20170628 (Steven) add for ASE-KH
        }
    }

    if(INSTALL_DOUBLE_EP>0)                                                     //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    {
        for(int i=0; i<EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Count; i++)
        {
            double d=atof(EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Strings[i].c_str())/10.0;
            if(d==DeviceForm_File.dDieForceKitDiameter)
            {
                EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex=i;
            }
        }

        if(EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex==-1)
        {
            EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex=0;
        }
    }

    EL<TEdit>("TfContact", "edAirForce")->Text        =DeviceForm_File.dPress;
//    edLimitDistance->Text   =DeviceForm_File.Distance;
    EL<TEdit>("TfContact", "edPinCount")->Text        =DeviceForm_File.iPinCT;
    EL<TEdit>("TfContact", "edXDimension")->Text      =FormatFloat("0.00", DeviceForm_File.XDimension);
    EL<TEdit>("TfContact", "edYDimension")->Text      =FormatFloat("0.00", DeviceForm_File.YDimension);
//    edCenterOfBuffer->Text  =DeviceForm_File.Stroke;
    EL<TEdit>("TfContact", "edForcePerPinN")->Text    =FormatFloat("0.0000",DeviceForm_File.ForcePerPinN);
    if(CosFunction.bForecePerPinKGf)                                            //JerryYang 20180515 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    {
        EL<TEdit>("TfContact", "edForcePerPinN")->Enabled=false;
    }
    EL<TEdit>("TfContact", "edForcePerPinG")->Text=FormatFloat("0.0000", DeviceForm_File.ForcePerPinG);   //JimmyChiu 20220121 add
    EL<TEdit>("TfContact", "edDoubleForce")->Text =FormatFloat("0.0000", DeviceForm_File.DoubleForce);    //Ifor 20190104 : Add Double EP Control
    EL<TEdit>("TfContact", "edtPinOfDie")->Text   =DeviceForm_File.iPinOfDie;                             //Steven 20191106: Pin of Die

    //jou 980827
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested")->ItemIndex   =DeviceForm_File.bSuckShuttleDeviceAfterTested;
    EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle")->ItemIndex =DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle;                       //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170912 (Steven) 矽品世明要求強制關閉shuttle wait outside功能
    {
        EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->Enabled =false;                         //jou 980930 add : Shuttle Waiting Out Site Chamber
    }
    EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->ItemIndex   =DeviceForm_File.bShuttleWaitingOutSiteChamber;
    EL<TRadioGroup>("TfContact", "rgTesterSidePush")->ItemIndex                 =DeviceForm_File.bTesterSidePush;                                       //Richard 20220321 : 渠梁Side Push
    EL<TRadioGroup>("TfContact", "rgSidePushMode")->ItemIndex                   =DeviceForm_File.iSidePushMode;                                         //Richard 20220321 : 渠梁Side Push
    EL<TRadioGroup>("TfContact", "rgPSDT")->ItemIndex                           =DeviceForm_File.bPickShuttleDeviceTogether;
    filerw::ELComboIndex(EL<TComboBox>("TfContact", "coD41"), DeviceForm_File.iSocketInitialICCheckPosition);
    EL<TEdit>("TfContact", "edD41")->Text                                 =FormatFloat("0.00", DeviceForm_File.fSocketInitialICCheckPositionOffset);
    EL<TRadioGroup>("TfContact", "rgIndexUpSpeed")->ItemIndex                   =DeviceForm_File.bIndexUpSpeed;                                         //kevin 20170524 (wei) add index up speed
    EL<TEdit>("TfContact", "edDropByPassDetect")->Text                    =DeviceForm_File.dDropByPassDetect;

    if(CosFunction.bPurgeBeforePickShuttle)                                     //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
    {                                                                           //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue
        EL<TRadioGroup>("TfContact", "rgPurgeBeforePickShuttle")->ItemIndex     =DeviceForm_File.bPurgeBeforePickShuttle;
        EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleTime")->Text     =DeviceForm_File.iPurgeBeforePickShuttleTime;
        EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleInterval")->Text =DeviceForm_File.iPurgeBeforePickShuttleInterval;
        EL<TEdit>("TfContact", "edtPurgeBdforePickShuttleOffSet")->Text   =DeviceForm_File.iPurgeBdforePickShuttleOffSet;
    }

    if(CosFunction.bContactHaveOffset==false)                                   //ChungHung 20140327 add by Customer 強制Contact Offset為0
    {
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Text   =0;                                         //ChungHung 20140808 補上自動變為0
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Text   =0;                                         //ChungHung 20140808 補上自動變為0
        EL<TLabel>("TfContact", "labContactOffset")->Visible   =false;
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible=false;
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible=false;
        EL<TLabel>("TfContact", "lblContactOfsMM")->Visible    =false;
    }
    else if(IniConfig.bChangeKitNoHardStop)                                     //jou 2015-12-08 Xilinx 驗證用
    {
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Text   =ReadIniData(szDir, "Test Arm1", "Contact", 0.0);
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Text   =ReadIniData(szDir, "Test Arm2", "Contact", 0.0);
        EL<TEdit>("TfContact", "edShtPickOffset1")->Text      =ReadIniData(szDir, "Test Arm1", "Pick Up", 0.0);
        EL<TEdit>("TfContact", "edShtPickOffset2")->Text      =ReadIniData(szDir, "Test Arm2", "Pick Up", 0.0);

        EL<TPanel>("TfContact", "palTorqueArm1")->Caption      =DeviceForm_File.dZ1Torue;                  //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
        EL<TPanel>("TfContact", "palTorqueArm2")->Caption      =DeviceForm_File.dZ2Torue;
        EL<TPanel>("TfContact", "palTorqueArm1")->Visible      =true;
        EL<TPanel>("TfContact", "palTorqueArm2")->Visible      =true;

        EL<TLabel>("TfContact", "labContactOffset")->Visible   =true;
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible=true;
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible=true;
        EL<TLabel>("TfContact", "labShtOffset")->Visible       =true;
        EL<TEdit>("TfContact", "edShtPickOffset1")->Visible   =true;
        EL<TEdit>("TfContact", "edShtPickOffset2")->Visible   =true;
        EL<TLabel>("TfContact", "lblShtPickOfsMM")->Visible    =true;
        EL<TLabel>("TfContact", "lblContactOfsMM")->Visible    =true;
        EL<TEdit>("TfContact", "edContactHeight1")->Enabled   =false;
        EL<TEdit>("TfContact", "edContactHeight2")->Enabled   =false;
        EL<TEdit>("TfContact", "edPickUp1")->Enabled          =false;
        EL<TEdit>("TfContact", "edPickUp2")->Enabled          =false;
    }
    else if(CosFunction.bContactShowOffset)                                     //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    {                                                                           //ChungHung 20140530 fix 應該是 Test Arm1 and Test Arm2
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Text   =ReadIniData(szDir, "Test Arm1", "Contact", 0.0);                                   //Steven 20140308 : Contact畫面加入Offset
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Text   =ReadIniData(szDir, "Test Arm2", "Contact", 0.0);                                   //Steven 20140308 : Contact畫面加入Offset
        EL<TLabel>("TfContact", "labContactOffset")->Visible   =true;
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible=true;
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible=true;
        EL<TLabel>("TfContact", "lblContactOfsMM")->Visible    =true;
        if(CUSTOMER_CODE==CC_ASE_CL)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }
    else
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //wei 20151228 修正ShtPickOffset存檔讀取
        {
            EL<TEdit>("TfContact", "edShtPickOffset1")->Text  =ReadIniData(szDir, "Test Arm1", "Pick Up", 0.0);
            EL<TEdit>("TfContact", "edShtPickOffset2")->Text  =ReadIniData(szDir, "Test Arm2", "Pick Up", 0.0);
        }
        EL<TLabel>("TfContact", "labContactOffset")->Visible   =false;
        EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible=false;
        EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible=false;
        EL<TLabel>("TfContact", "lblContactOfsMM")->Visible    =false;
    }

    if(IniConfig.bSPILFunction && IniConfig.bD58UseArm1PickPlaceArm2Test)       //KevinCheng 20260115 : NV Contact Test
    {
        EL<TCheckBox>("TfContact", "cbTestContactMode")->Visible=true;
        EL<TEdit>("TfContact", "edTestContactCount")->Visible=true;
        EL<TLabel>("TfContact", "lblTsetSec")->Visible=true;
        EL<TEdit>("TfContact", "edTestSec")->Visible=true;
        EL<TLabel>("TfContact", "lblCount")->Visible=true;
        EL<TLabel>("TfContact", "lblNowCount")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbTestContactMode")->Visible=false;
        EL<TEdit>("TfContact", "edTestContactCount")->Visible=false;
        EL<TLabel>("TfContact", "lblTsetSec")->Visible=false;
        EL<TEdit>("TfContact", "edTestSec")->Visible=false;
        EL<TLabel>("TfContact", "lblCount")->Visible=false;
        EL<TLabel>("TfContact", "lblNowCount")->Visible=false;
    }

    if(IniConfig.bD67LoadCellMeasure)                                           //kevin 20190907 Arm 測區次數道量測 功能;
    {
        EL<TEdit>("TfContact", "edLoadCellHeight1")->Text     =FormatFloat("0.00", DeviceForm_File.dLoadCellZ1Down);
        EL<TEdit>("TfContact", "edLoadCellHeight2")->Text     =FormatFloat("0.00", DeviceForm_File.dLoadCellZ2Down);
    }

    if(IniConfig.bTestIcCheckInContact==false)                                  //ChungHung 20140408 放錯位置
    {
        EL<TLabel>("TfContact", "lblICCheck")->Visible         =false;
        EL<TComboBox>("TfContact", "coD41")->Visible              =false;
        EL<TEdit>("TfContact", "edD41")->Visible              =false;
        EL<TLabel>("TfContact", "labD41")->Visible             =false;                                     //ChungHung 20140408 add
    }
    else
    {
        if(DeviceForm_File.iSocketInitialICCheckPosition==0)
        {
            EL<TEdit>("TfContact", "edD41")->Visible          =false;
            EL<TLabel>("TfContact", "labD41")->Visible         =false;
        }
        else
        {
            EL<TEdit>("TfContact", "edD41")->Visible          =true;
            EL<TLabel>("TfContact", "labD41")->Visible         =true;
        }
    }
    EL<TCheckBox>("TfContact", "chkUseAddWeight")->Checked=DeviceForm_File.bUseAddWeight;                     //kevin 20170629 (Steven) direct heat add weight

    EL<TEdit>("TfContact", "edDieForcePerPinN")->Text =FormatFloat("0.0000", DeviceForm_File.DieForcePerPinN);
    EL<TEdit>("TfContact", "edDieForcePerPinG")->Text =FormatFloat("0.0000", DeviceForm_File.DieForcePerPinG);                                    //JimmyChiu 20220121 add

}

// golden cContact.cpp:15566  TfContact::SetContactMode()
static void DF_SetContactMode()
{

    if(EL<TRadioButton>("TfContact", "rbModeNormal")->Checked)
    {
        iContactMode=CONTACT_NORMAL;
        if(CUSTOMER_CODE==CC_KYEC_CHEN &&
           IniConfig.bA16ContactTestDropContact)                                //wei 20150831
        {
            DF_ChangeContactMode(false);
        }
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_NORMAL");
        if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }
    else if(EL<TRadioButton>("TfContact", "rbDeviceMapping")->Checked)                                           //Steven 20190910 : Qualcomm功能
    {
        iContactMode=CONTACT_DEVICE_MAP_CHECK;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_DEVICE_MAP_CHECK");
    }
    else if(EL<TRadioButton>("TfContact", "rbAutoHeight")->Checked)
    {
        iContactMode=CONTACT_AUTO_GET_HEIGHT;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_AUTO_GET_HEIGHT");
        if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }
    else if(EL<TRadioButton>("TfContact", "rbManualHeight")->Checked)
    {
        iContactMode=CONTACT_MANUAL_GET_HEIGHT;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_MANUAL_GET_HEIGHT");
        if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }
    else if(EL<TRadioButton>("TfContact", "rbContactTest")->Checked)
    {
        iContactMode=CONTACT_TEST;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_TEST");
        if(CUSTOMER_CODE==CC_KYEC_CHEN &&
           IniConfig.bA16ContactTestDropContact)                                //wei 20150831
        {
            DF_ChangeContactMode(true);
        }
        if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)          //Chrischen 20260316 : ASECL Contact反灰需求
        {
//            palTestSocket->Enabled=false;
//            coD41->Enabled=false;
//            edD41->Enabled=false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;
        }
    }
    else if(EL<TRadioButton>("TfContact", "rbAutoContactTest")->Checked)
    {
        iContactMode=AUTO_CONTACT_TEST;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("AUTO_CONTACT_TEST");
    }
    else if(EL<TRadioButton>("TfContact", "rbStepContactTest")->Checked)                                         //Steven 20150811 : Step by Step Contact Test
    {
        iContactMode=STEP_CONTACT_TEST;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("STEP_CONTACT_TEST");
    }
    else if(EL<TRadioButton>("TfContact", "rbLoadCellAutoHigh")->Checked)                                        // Step by Step Contact Test
    {
        iContactMode=CONTACT_LoadCell_AUTO_GET_HEIGHT;                          //kevin 20190909 add Load cell AutoHigh
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_LoadCell_AUTO_GET_HEIGHT");
    }
    else if(EL<TRadioButton>("TfContact", "rbDeviceLoopTest")->Checked)                                          //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST
    {
        iContactMode=CONTACT_DEVICE_LOOP_TEST;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("CONTACT_DEVICE_LOOP_TEST");
    }
    else if(EL<TRadioButton>("TfContact", "rbKTempIndexMove")->Checked)                                          //Ztex 2024.03.26 Add Contact Mode K Temperature
    {
        iContactMode=K_TEMP_INDEX_MOVE;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("K_Temp_Index_Move");
        EL<TCheckBox>("TfContact", "chk_K_Temperature")->Checked=true;
    }
    else if(EL<TRadioButton>("TfContact", "rbVisualDetectionTest")->Checked)                                     //Ifor 20260204 add: TFAMD 要求新增檢測流程
    {
        iContactMode=VISUAL_DETECTION_TEST;
        EL<TMemo>("TfContact", "Memo1")->Lines->Add("VISUAL_DETECTION_TEST");
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       CUSTOMER_CODE==CC_MAXIM_THAILAND)
    {
        if(iContactMode==CONTACT_AUTO_GET_HEIGHT)
        {
            EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Visible=true;
        }
        else
        {
            EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Visible=false;
            EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Checked=false;                          //Ifor 20220803 add 一鍵完成Auto Contact Hight
        }
    }
    else
    {
        EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Visible=false;
        EL<TCheckBox>("TfContact", "cbOneTouchAutoContactHight")->Checked=false;                              //Ifor 20220803 add 一鍵完成Auto Contact Hight
    }

    if(iContactMode==STEP_CONTACT_TEST)
    {
        EL<TCheckBox>("TfContact", "chkDailyCorrelation")->Enabled=true;
    }
    else
    {
        EL<TCheckBox>("TfContact", "chkDailyCorrelation")->Enabled=false;
        EL<TCheckBox>("TfContact", "chkDailyCorrelation")->Checked=false;
    }

}

// golden cContact.cpp:15698  TfContact::ChangeContactMode(bool ret)
static void DF_ChangeContactMode(bool ret)
{

    if(ret==true)
    {
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DropContact);
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbVacuumMode"), VacuumOFFMode);
        EL<TEdit>("TfContact", "edDropWaitTime")->Text="0.5";
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=true;                                            //2012-01-03    Dell
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=true;                                            //2012-01-03    Dell
    }
    else
    {
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DirectContactMode);
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbVacuumMode"), VacuumOFFMode);
        EL<TEdit>("TfContact", "edDropWaitTime")->Text="0.5";
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=false;                                           //2012-01-03    Dell
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=false;                                           //2012-01-03    Dell
    }

}

// golden cContact.cpp:1925  TfContact::ShowArmAndDeviceForce()
static void DF_ShowArmAndDeviceForce()
{

    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceKG")->Color=clWhite;
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceN")->Color=clWhite;
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirForce")->Color=clWhite;

    double dBallCount=atof(EL<TEdit>("TfContact", "edPinCount")->Text.c_str());                           //腳數
    double dSingleN=atof(EL<TEdit>("TfContact", "edForcePerPinN")->Text.c_str());                         //牛頓
    double dSingleGf=atof(EL<TEdit>("TfContact", "edForcePerPinG")->Text.c_str());                        //gf

    iTotalGf=DF_CalculateTotalAirForce(dBallCount, dSingleGf);                     //JimmyChiu 20220119 獨立計算空氣壓力
    DeviceForm.dPress=iTotalGf;

//    double dDeviceN=dBallCount*dSingleN;                                        // device要壓的牛頓
//    double dDeviceGf=dBallCount*dSingleGf*0.001;                                // device要壓的公斤     //JimmyChiu 20220119 /1000=>*0.001
    double dDeviceN  = DF_CalcDeviceForce(dBallCount, dSingleN, dSingleGf, false); //AI(ht9045-contact-force) 20260624 (RogerYang) : 統一公式(N)
    double dDeviceGf = DF_CalcDeviceForce(dBallCount, dSingleN, dSingleGf, true);  //AI(ht9045-contact-force) 20260624 (RogerYang) : 統一公式(Kgf)
    EL<TEdit>("TfContact", "edForcePerDeviceKG")->Text=FormatFloat("0.0000", dDeviceGf);                  //秀出一個ic要壓的公斤
    EL<TEdit>("TfContact", "edForcePerDeviceN")->Text =FormatFloat("0.0000", dDeviceN);                   //秀出一個ic要壓的牛頓

    EL<TEdit>("TfContact", "edAirForce")->Text =FormatFloat("0.0000", iTotalGf);
    EL<TEdit>("TfContact", "edAirForceN")->Text=FormatFloat("0.0000", iTotalGf*9.8);                      //JerryYang (Steven) 20161215 改由KG換算成牛頓,避免在下限時顯示的kgf跟N力量不同

}

// golden cContact.cpp:1988  TfContact::DutCount()
static void DF_DutCount()
{

    AnsiString asStr;
    switch(TestIF_File.iTestMode)                                               //jou 2014-09-06 修正開啟程式的時候EP異常
    {
        case SingleSite:
            dDutCount=1;
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                                                              //Frank 20200520 2X2NN Mode
//        case DualSiteBS: //1x1 busyshuttle
            if(IniConfig.bD27UseSingleSite85kg==true)
            {
                if((LastSet.bUseTestSocket[0][0][1]==false && LastSet.bUseTestSocket[1][0][1]==false)  ||               //ChungHung 20130910 alter for SCK can close site by Index
                   (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false))                  //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                    dDutCount=1;
                else
                    dDutCount=2;
            }
            else
            {
                dDutCount=2;
            }
            break;
        case DualSite2x1:
            if(IniConfig.bD27UseSingleSite85kg==true)
            {
                if((LastSet.bUseTestSocket[0][1][0]==false && LastSet.bUseTestSocket[1][1][0]==false) ||                //ChungHung 20130910 alter for SCK can close site by Index
                   (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false))                  //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                    dDutCount=1;
                else
                    dDutCount=2;
            }
            else
            {
                dDutCount=2;
            }
            break;
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
        case TriSite1X3:
            dDutCount=3;
            break;
        case QualSite1X4:                                                       //1x4
        case QualSite2X2:                                                       //2x2
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
//        case QualSite2X2BS: //2x2 busyshuttle
            dDutCount=4;
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            dDutCount=6;
            break;
        case _8Site2X4:                                                         //2x4
            if(TestIF_File.bOctal_12Kit)                                        //ChungHung 20140508 add for SCK
                dDutCount=12;
            else
                dDutCount=8;
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
            dDutCount=10;
            break;
        case _12Site2X6:
            dDutCount=12;
            break;
        case _16Site2X8:                                                                                                                                        //2x8
            if(TestIF_File.iSiteMap[0][0]<=0 && TestIF_File.iSiteMap[0][2]<=0 && TestIF_File.iSiteMap[0][4]<=0 && TestIF_File.iSiteMap[0][6]<=0 &&
               TestIF_File.iSiteMap[1][0]<=0 && TestIF_File.iSiteMap[1][2]<=0 && TestIF_File.iSiteMap[1][4]<=0 && TestIF_File.iSiteMap[1][6]<=0)                //Steven 20221122 : 16 site關site壓力顯示修正
            {
                dDutCount=8;
            }
            else if(TestIF_File.iSiteMap[0][1]<=0 && TestIF_File.iSiteMap[0][3]<=0 && TestIF_File.iSiteMap[0][5]<=0 && TestIF_File.iSiteMap[0][7]<=0 &&
                    TestIF_File.iSiteMap[1][1]<=0 && TestIF_File.iSiteMap[1][3]<=0 && TestIF_File.iSiteMap[1][5]<=0 && TestIF_File.iSiteMap[1][7]<=0)           //Steven 20221122 : 16 site關site壓力顯示修正
            {
                dDutCount=8;
            }
            else if(TestIF_File.iSiteMap[0][0]<=0 && TestIF_File.iSiteMap[0][2]<=0 && TestIF_File.iSiteMap[0][4]<=0 && TestIF_File.iSiteMap[0][6]<=0 &&
                    TestIF_File.iSiteMap[1][1]<=0 && TestIF_File.iSiteMap[1][3]<=0 && TestIF_File.iSiteMap[1][5]<=0 && TestIF_File.iSiteMap[1][7]<=0)
            {
                dDutCount=8;
            }
            else if(TestIF_File.iSiteMap[0][1]<=0 && TestIF_File.iSiteMap[0][3]<=0 && TestIF_File.iSiteMap[0][5]<=0 && TestIF_File.iSiteMap[0][7]<=0 &&
                    TestIF_File.iSiteMap[1][0]<=0 && TestIF_File.iSiteMap[1][2]<=0 && TestIF_File.iSiteMap[1][4]<=0 && TestIF_File.iSiteMap[1][6]<=0)
            {
                dDutCount=8;
            }
            else if(iCloseSiteModeFor2x8>e2x8Standard)                          //Steven 20260420 : != --> >
            {
                dDutCount=8;
            }
            else
            {
                dDutCount=16;
            }
            break;
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            dDutCount=8;                                                        //KenHsieh 20230313 : NN Mode Dutcount新增
            break;
        case _32Site4X8N:
            dDutCount=16;                                                       //KenHsieh 20230313 : NN Mode Dutcount 32 -> 16
            break;
    }

    if(TestIF_File.iTestMode==SingleSite) { if(EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position==3 || EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position==6) EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position=5; else if(EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position==4) EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position=2; } DeviceForm_File.iHeadDeviceCT=EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position; asStr.sprintf("%sContact%d.bmp", BmpPath.c_str(), (int)EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position);   // golden cContact.cpp:2090-2098 取代：scrbSLK->Position（ELTrackBar；設 Position 照 VCL 觸發 OnChange=scrbSLKChange）
#if 0 // GATE (S12-C save) golden cContact.cpp:2090-2098 -- scrbSLK->Position（ELTrackBar；設 Position 照 VCL 觸發 OnChange=scrbSLKChange）
    if(TestIF_File.iTestMode==SingleSite)                                       //JerryYang 20171215 (Steven) Single site只支援浮動頭1對1或是2對1
    {
        if(EL<TControl>("TfContact", "scrbSLK")->Position==3 || EL<TControl>("TfContact", "scrbSLK")->Position==6)
            EL<TControl>("TfContact", "scrbSLK")->Position=5;
        else if(EL<TControl>("TfContact", "scrbSLK")->Position==4)
            EL<TControl>("TfContact", "scrbSLK")->Position=2;
    }
    DeviceForm_File.iHeadDeviceCT=EL<TControl>("TfContact", "scrbSLK")->Position;                            //jou 2014-04-24 修正選擇head數時,force值不會馬上改變
    asStr.sprintf("%sContact%d.bmp", BmpPath, EL<TControl>("TfContact", "scrbSLK")->Position);
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfContact", "imgSLK")->Picture->LoadFromFile(asStr);                                       //JimmyChiu 20220121 str[64]=>AnsiString asStr
    //ShowArmAndDeviceForce();                                                  //JimmyChiu 20220121 mark 改變階層

}

// golden cContact.cpp:2103  TfContact::scrbSLKChange(TObject *Sender)
static void DF_scrbSLKChange()
{

    if(fShow==false)                                                            //JimmyChiu 20220121 add
        return;
    DF_DutCount();
    DF_ShowArmAndDeviceForce();                                                    //JimmyChiu 20220121 add
    DF_edAirForceChange();   // golden cContact.cpp:2109-2109 取代：edAirForceChange(this)：事件處理器參數已拿掉
#if 0 // GATE (S12-C save) golden cContact.cpp:2109-2109 -- edAirForceChange(this)：事件處理器參數已拿掉
    DF_edAirForceChange(this);                                                     //jou 2014-04-24 修正選擇head數時,force值不會馬上改變
#endif // GATE (S12-C save)

}

// golden cContact.cpp:2297  TfContact::edAirForceChange(TObject *Sender)
static void DF_edAirForceChange()
{

    DeviceForm.dPress=atof(EL<TEdit>("TfContact", "edAirForce")->Text.c_str());
    DeviceForm_File.dPress=DeviceForm.dPress;                                   //jou 2014-09-06 修正開啟程式的時候EP異常
    filerw::ELTodo("golden cContact.cpp:2301 ADAM_WriteVoltage(DeviceForm.dPress) not done (EP hardware output; ADAM_* are empty stubs in the port)");   // golden cContact.cpp:2301-2301 取代：ADAM_WriteVoltage：EP 電壓輸出（硬體；移植樹 atester_shims 是空殼）—— DeviceForm.dPress／DeviceForm_File.dPress 兩行照 golden
#if 0 // GATE (S12-C save) golden cContact.cpp:2301-2301 -- ADAM_WriteVoltage：EP 電壓輸出（硬體；移植樹 atester_shims 是空殼）—— DeviceForm.dPress／DeviceForm_File.dPress 兩行照 golden
    ADAM_WriteVoltage(DeviceForm.dPress);                                       //Steven 20091209
#endif // GATE (S12-C save)

}

// golden cContact.cpp:11777  TfContact::InitCarlibrationTask()
static void DF_InitCarlibrationTask()
{

    CarlibrationTask=1;

}

// golden cContact.cpp:22608  TfContact::UpdateContactRelative()
static void DF_UpdateContactRelative()
{

    int Pos=0;
    Pos=Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str()));   // golden cContact.cpp:22611-22611 取代：Get0_01MMType(char*) const
#if 0 // GATE (S12-C save) golden cContact.cpp:22611-22611 -- Get0_01MMType(char*) const
    Pos=Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str());
#endif // GATE (S12-C save)
    EL<TEdit>("TfContact", "edContactRelativeZ1")->Text=ConvertTouMType(Teach.iContactZ1Relative-Pos);

    Pos=Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str()));   // golden cContact.cpp:22614-22614 取代：Get0_01MMType(char*) const
#if 0 // GATE (S12-C save) golden cContact.cpp:22614-22614 -- Get0_01MMType(char*) const
    Pos=Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str());
#endif // GATE (S12-C save)
    EL<TEdit>("TfContact", "edContactRelativeZ2")->Text=ConvertTouMType(Teach.iContactZ2Relative-Pos);

}

// golden cContact.cpp:14179  TfContact::spbSaveClick(TObject *Sender)
static void DF_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        if(CarlibrationTask!=1)
        {
            filerw::ELMessage("""Auto Height""/""Contect Test"" Mode need Finish !!", "需要完成""Auto Height""/""Contect Test""!!");
            return;
        }
    filerw::ELMark("closed");   // golden cContact.cpp:14190-14190 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
#if 0 // GATE (S12-C save) golden cContact.cpp:14190-14190 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）
        Close();
#endif // GATE (S12-C save)
        return;
    }

    int ret=0;
    AnsiString str1="";
    double position1=0.0, position2=0.0, position3=0.0;
    if(EL<TCheckBox>("TfContact", "chk_K_Temperature")->Checked==true)                                        //Ztex 2024.03.26 Add Contact Mode K Temperature
        return;

    if(INSTALL_DOUBLE_EP!=0)
    {
        if(dOldDieForceKitDiameter!=DeviceForm_File.dDieForceKitDiameter)
        {
            filerw::ELMessage("The Diameter bore has changed.Please confirm the KG setting value again.!","直徑有變動，請重新確認KG設定值。!");
            dOldDieForceKitDiameter=DeviceForm_File.dDieForceKitDiameter;
        }
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //JerryYang 20170213 (Steven) 矽品騰清要求修改Run check觸發條件,此段要放在SaveSetupFile之前
    {
        if(DF_CheckContactSettingChange()==true)
        {
            bHasSaveSet=true;
        }
        EventReport(SECS_EVENT.SaveRecipe);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        str1="";
    position1=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str())))); position2=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(asContactHeight[0].c_str()))));   // golden cContact.cpp:14221-14222 取代：Get0_01MMType(char*) const
#if 0 // GATE (S12-C save) golden cContact.cpp:14221-14222 -- Get0_01MMType(char*) const
        position1=atof(ConvertTouMType(Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str())));
        position2=atof(ConvertTouMType(Get0_01MMType(asContactHeight[0].c_str())));
#endif // GATE (S12-C save)
        position3=position1-position2;
        if(position3<0)
        {
            position3*=-1;
        }

        if(position3>IniConfig.dD68DistanceRange)
        {
            str1.sprintf("Arm1 Before Auto Height Pos =%smm,After Auto Height Pos=%s", asContactHeight[0].c_str(), EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str());
            ret=filerw::ELAsk(str1, "Do you want to change?");
            if(ret!=1)
            {
                EL<TEdit>("TfContact", "edContactHeight1")->Text=asContactHeight[0];
            }
        }

        str1="";
    position1=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str())))); position2=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(asContactHeight[1].c_str()))));   // golden cContact.cpp:14240-14241 取代：Get0_01MMType(char*) const
#if 0 // GATE (S12-C save) golden cContact.cpp:14240-14241 -- Get0_01MMType(char*) const
        position1=atof(ConvertTouMType(Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str())));
        position2=atof(ConvertTouMType(Get0_01MMType(asContactHeight[1].c_str())));
#endif // GATE (S12-C save)
        position3=position1-position2;
        if(position3<0)
        {
            position3*=-1;
        }

        if(position3>IniConfig.dD68DistanceRange)
        {
            str1.sprintf("Arm2 Before Auto Height Pos =%smm,After Auto Height Pos=%s", asContactHeight[1].c_str(), EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str());
            ret=filerw::ELAsk(str1, "Do you want to change?");
            if(ret!=1)
            {
                EL<TEdit>("TfContact", "edContactHeight2")->Text=asContactHeight[1];
            }
        }
    }
    DF_SaveSetupFile(GetRecipePath(), GetLastOpenFN());                            //kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(GetLastOpenFN(), "JOBFILE");
    #endif
    DF_ReadFile();
    DF_DoIniDataToForm();                                                          //kevin 20180402 add refesh show data
    if(bAutoHighFinish==true)
    {
    filerw::ELTodo("golden cContact.cpp:14266 fOffSet->ClearIndexOffset() is GATE (O-6) in the port (no body) -- index offset not cleared after auto height");   // golden cContact.cpp:14266-14266 取代：fOffSet->ClearIndexOffset()：移植樹 GATE (O-6)，沒有本體（forms/fOffSet.h:390）
#if 0 // GATE (S12-C save) golden cContact.cpp:14266-14266 -- fOffSet->ClearIndexOffset()：移植樹 GATE (O-6)，沒有本體（forms/fOffSet.h:390）
        fOffSet->ClearIndexOffset();
#endif // GATE (S12-C save)
        bAutoHighFinish=false;
    }
    EL<TSpeedButton>("TfContact", "spbSave")->Down=false;
    bHeightCalibrationFinish=true;                                              //JerryYang 20151103 做完Height Calibration
    filerw::ELTodo("golden cContact.cpp:14271-14278 KYEC_LEE fMain->LabZ1_Test/LabZ2_Test caption (not in the port TfMain)");
#if 0 // GATE (S12-C save) golden cContact.cpp:14271-14278 -- KYEC_LEE fMain->LabZ1_Test/LabZ2_Test caption (not in the port TfMain)
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20181130 : add KYEC 要求顯示Contact高度
    {
        double position1,position2;
        position1=atof(ConvertTouMType(Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str())));
        position2=atof(ConvertTouMType(Get0_01MMType(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str())));
        fMain->LabZ1_Test->Caption=position1;
        fMain->LabZ2_Test->Caption=position2;
    }
#endif // GATE (S12-C save)

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    filerw::ELTodo("golden cContact.cpp:14281-14294 Double/Multi/Independent EP output after save (IsMultiEPPressureRouteActive/IsIndependentEPPressureRouteActive/APAX_WriteData not in the port; ADAM_* are empty stubs)");
#if 0 // GATE (S12-C save) golden cContact.cpp:14281-14294 -- Double/Multi/Independent EP output after save (IsMultiEPPressureRouteActive/IsIndependentEPPressureRouteActive/APAX_WriteData not in the port; ADAM_* are empty stubs)
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //AI(ht9045-v899) 20260430: extend DoubleForce write to Multi EP half.
    {
        int iInputValue=0;
        iInputValue=TransformFuntion(DeviceForm_File.DoubleForce, true);
        //AI(ht9045-v899) 20260526: Multi EP Die Force SAVE flushes the computed per-site iAPAXEPValue instead of one common direct value.
        if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && IsMultiEPPressureRouteActive()==true)
            APAX_WriteData(false, 0);
        else
            ADAM_DirectWriteData(iInputValue, 0, 0);
    }

    //AI(ht9045-v899) 20260526: TfContact SAVE must refresh independent/Multi EP site outputs after site trim changes.
    if(IsIndependentEPPressureRouteActive()==true)
        ADAM_WriteVoltage(DeviceForm.dPress);
#endif // GATE (S12-C save)

    if(TestIF_File.bEnableReadAndCheckTorque)                                   //kevin 20211211 change
    {
        bResetArm1Value=true;
        bResetArm2Value=true;
        bResetRecordSetArm1Value=false;
        bResetRecordSetArm2Value=false;
        iReadTorqueError=0;
    }

    filerw::ELTodo("golden cContact.cpp:14305-14308 USE_VibrationCommunication dmTrayMotor->bNeedSetVibrateMotSpeed (dmTrayMotor not in the port)");
#if 0 // GATE (S12-C save) golden cContact.cpp:14305-14308 -- USE_VibrationCommunication dmTrayMotor->bNeedSetVibrateMotSpeed (dmTrayMotor not in the port)
    if(USE_VibrationCommunication)                                              //JerryYang 20230814 : add震動馬達通訊調速版本
    {
        dmTrayMotor->bNeedSetVibrateMotSpeed=true;
    }
#endif // GATE (S12-C save)

}

// golden cContact.cpp:17345  TfContact::CheckContactSettingChange()
static bool DF_CheckContactSettingChange()
{
    filerw::ELMark("CheckContactSettingChange");   // FileRW：存檔流程 trace（ack.trace）

    //============Contact parameter==========
    if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex!=DeviceForm.ContactMode)
        return true;
    else if(EL<TComboBox>("TfContact", "cbVacuumMode")->ItemIndex!=DeviceForm.VacuumMode)
        return true;
    else if(EL<TEdit>("TfContact", "edDropWaitTime")->Text!=FormatFloat("0.00", DeviceForm.DropWait))
        return true;
    else if(EL<TEdit>("TfContact", "edDropSpeed")->Text!=FormatFloat("0.00", DeviceForm.DropSpeed))
        return true;
    else if(EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->ItemIndex!=DeviceForm.bShuttleWaitingOutSiteChamber)
        return true;
    else if(EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested")->ItemIndex!=DeviceForm.bSuckShuttleDeviceAfterTested)
        return true;
    else if(EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle")->ItemIndex!=DeviceForm.bSuckShuttleDeviceWaitOnShuttle)                    //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
        return true;
    //============Contact Force===============
    else if(atof(EL<TEdit>("TfContact", "edAirForce")->Text.c_str())!=DeviceForm.dPress)
        return true;
    else if((int)EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position!=DeviceForm.iHeadDeviceCT)   // golden cContact.cpp:17365-17365 取代：scrbSLK->Position（ELTrackBar）
#if 0 // GATE (S12-C save) golden cContact.cpp:17365-17365 -- scrbSLK->Position（ELTrackBar）
    else if(EL<TControl>("TfContact", "scrbSLK")->Position!=DeviceForm.iHeadDeviceCT)
#endif // GATE (S12-C save)
        return true;
    else if(DeviceForm.dKitDiameter==6.0 && EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex!=2)
        return true;
    else if(DeviceForm.dKitDiameter==4.0 && EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex!=1)
        return true;
    else if(DeviceForm.dKitDiameter==5.6 && EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex!=3)
        return true;
    else if(DeviceForm.dKitDiameter==3.0 && EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex!=0)
        return true;
    else if(atoi(EL<TEdit>("TfContact", "edPinCount")->Text.c_str())!=DeviceForm.iPinCT)
        return true;
    else if(EL<TEdit>("TfContact", "edForcePerPinN")->Text!=FormatFloat("0.0000", DeviceForm.ForcePerPinN))
        return true;
    else if(double(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edXDimension")->Text.c_str())))!=FormatFloat("0.00", DeviceForm.XDimension).ToDouble())   // golden cContact.cpp:17379-17379 取代：int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）
#if 0 // GATE (S12-C save) golden cContact.cpp:17379-17379 -- int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）
    else if(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edXDimension")->Text.c_str()))!=FormatFloat("0.00", DeviceForm.XDimension))
#endif // GATE (S12-C save)
        return true;
    else if(double(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edYDimension")->Text.c_str())))!=FormatFloat("0.00", DeviceForm.YDimension).ToDouble())   // golden cContact.cpp:17381-17381 取代：int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）
#if 0 // GATE (S12-C save) golden cContact.cpp:17381-17381 -- int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）
    else if(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edYDimension")->Text.c_str()))!=FormatFloat("0.00", DeviceForm.YDimension))
#endif // GATE (S12-C save)
        return true;
    else
        return false;

}

// golden cContact.cpp:14313  TfContact::SaveSetupFile(AnsiString szDir, AnsiString S)
static void DF_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    int iResult=0;                                                              //Steven 20100531 強制關閉
    AnsiString S1=GetLastOpenFN();
    AnsiString szDir2;
    AnsiString szDir3="D:\\HT9045\\system\\Contact.ini";
    AnsiString sBuffer="";
    elContact->SaveEditTextToFile(szDir, "Contact.Data");
    MyForceDirectories(szDir);
    szDir+="Contact.Data";

    if(CosFunction.bContactHeightSaveToContactIni)                              //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
    {
        WriteIniData(szDir3, S1, "Pick Up1",             EL<TEdit>("TfContact", "edPickUp1")->Text);
        WriteIniData(szDir3, S1, "Test Arm1",            EL<TEdit>("TfContact", "edContactHeight1")->Text);
        WriteIniData(szDir3, S1, "Drop1",                EL<TEdit>("TfContact", "edDropOffset1")->Text);
        WriteIniData(szDir3, S1, "Place1",               EL<TEdit>("TfContact", "edReleaseHeight1")->Text);
        WriteIniData(szDir3, S1, "Pick Up2",             EL<TEdit>("TfContact", "edPickUp2")->Text);
        WriteIniData(szDir3, S1, "Test Arm2",            EL<TEdit>("TfContact", "edContactHeight2")->Text);
        WriteIniData(szDir3, S1, "Drop2",                EL<TEdit>("TfContact", "edDropOffset2")->Text);
        WriteIniData(szDir3, S1, "Place2",               EL<TEdit>("TfContact", "edReleaseHeight2")->Text);
        WriteIniData(szDir3, S1, "ContactBackUp1",       EL<TEdit>("TfContact", "edContactBackUp1")->Text);                                       //Steven 20190822 : download cover file include ContactBackUp and ShuttlePickBackUp
        WriteIniData(szDir3, S1, "ContactBackUp2",       EL<TEdit>("TfContact", "edContactBackUp2")->Text);
        WriteIniData(szDir3, S1, "ShuttlePickBackUp1",   atof(EL<TEdit>("TfContact", "edOrgPick1")->Text.c_str()));
        WriteIniData(szDir3, S1, "ShuttlePickBackUp2",   atof(EL<TEdit>("TfContact", "edOrgPick2")->Text.c_str()));
    }
    else
    {
        WriteIniData(szDir, "Test Arm1", "Contact",             EL<TEdit>("TfContact", "edContactHeight1")->Text);
        WriteIniData(szDir, "Test Arm2", "Contact",             EL<TEdit>("TfContact", "edContactHeight2")->Text);
        WriteIniData(szDir, "Test Arm1", "Pick Up",             EL<TEdit>("TfContact", "edPickUp1")->Text);
        WriteIniData(szDir, "Test Arm1", "Place",               EL<TEdit>("TfContact", "edReleaseHeight1")->Text);
        WriteIniData(szDir, "Test Arm2", "Pick Up",             EL<TEdit>("TfContact", "edPickUp2")->Text);
        WriteIniData(szDir, "Test Arm2", "Place",               EL<TEdit>("TfContact", "edReleaseHeight2")->Text);

        WriteIniData(szDir, "Test Arm1", "ContactBackUp",       EL<TEdit>("TfContact", "edContactBackUp1")->Text);                                //ChungHung 20130715 add ATK 顯示上次AutoHeight的值
        WriteIniData(szDir, "Test Arm2", "ContactBackUp",       EL<TEdit>("TfContact", "edContactBackUp2")->Text);
        WriteIniData(szDir, "Test Arm1", "ShuttlePickBackUp",   atof(EL<TEdit>("TfContact", "edOrgPick1")->Text.c_str()));
        WriteIniData(szDir, "Test Arm2", "ShuttlePickBackUp",   atof(EL<TEdit>("TfContact", "edOrgPick2")->Text.c_str()));

        if(CosFunction.bFixedDropSpeed)                                         //Steven 20131101 : 使用固定的Drop速度
        {
            WriteIniData(szDir, "Test Arm1", "Drop",            CosFunction.dLimitMinDropOffset);
            WriteIniData(szDir, "Test Arm2", "Drop",            CosFunction.dLimitMinDropOffset);
        }
        else
        {
            if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContact &&
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190225 : 力成 DirectContact & DropContact 切換後，DropContact 的 Drop 不要變成預設值
            {
                WriteIniData(szDir, "Test Arm1", "Drop_DropContactMode",    EL<TEdit>("TfContact", "edDropOffset1")->Text);
                WriteIniData(szDir, "Test Arm2", "Drop_DropContactMode",    EL<TEdit>("TfContact", "edDropOffset2")->Text);
            }
            else
            {
                WriteIniData(szDir, "Test Arm1", "Drop",                    EL<TEdit>("TfContact", "edDropOffset1")->Text);
                WriteIniData(szDir, "Test Arm2", "Drop",                    EL<TEdit>("TfContact", "edDropOffset2")->Text);
            }
        }
    }

    WriteIniData(szDir, "Wait Time", "Drop Speed",  EL<TEdit>("TfContact", "edDropSpeed")->Text);         //Eliot 2011_0318
    WriteIniData(szDir, "Wait Time", "Side Push Wait Time",  EL<TEdit>("TfContact", "edSidePushWaitTime")->Text);                                 //Richard 20220321 : 渠梁Side Push

    if(TestIF_File.bUseSLKClamp &&
       TestIF_File.iSeparabilityTest==1)                                        //JerryYang 20160524
    {
        WriteIniData(szDir, "Mode", "Contact",          TMove);
        WriteIniData(szDir, "Mode", "Vacuum",           1);
        WriteIniData(szDir, "Wait Time", "Drop Wait",   EL<TEdit>("TfContact", "edDropWaitTime")->Text);  //JerryYang 20180519 (wei) : 修正drop wait存檔異常問題
        WriteIniData(szDir, "Wait Time", "Up Wait",     EL<TEdit>("TfContact", "edUpWaitTime")->Text);
    }
    else if(CUSTOMER_CODE==CC_KYEC_CHEN &&
            IniConfig.bA16ContactTestDropContact)                               //wei 20150831
    {
        WriteIniData(szDir, "Mode", "Contact",          0);
        WriteIniData(szDir, "Mode", "Vacuum",           1);
        WriteIniData(szDir, "Wait Time", "Drop Wait",   0.5);
    }
    else
    {
        WriteIniData(szDir, "Mode", "Contact",          EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex);
        WriteIniData(szDir, "Mode", "Vacuum",           EL<TComboBox>("TfContact", "cbVacuumMode")->ItemIndex);
        WriteIniData(szDir, "Wait Time", "Drop Wait",   EL<TEdit>("TfContact", "edDropWaitTime")->Text);
        WriteIniData(szDir, "Wait Time", "Up Wait",     EL<TEdit>("TfContact", "edUpWaitTime")->Text);
    }
    WriteIniData(szDir, "Wait Time", "Up Speed",        EL<TEdit>("TfContact", "edUpSpeed")->Text);
    WriteIniData(szDir, "Test Arm1", "Up",              EL<TEdit>("TfContact", "edUpOffset1")->Text);
    WriteIniData(szDir, "Test Arm2", "Up",              EL<TEdit>("TfContact", "edUpOffset2")->Text);

    WriteIniData(szDir, "Mode", "Dummy Contact",    iResult);
    WriteIniData(szDir, "Mode", "Head Device Mode", (int)EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position);   // golden cContact.cpp:14404-14404 取代：scrbSLK->Position（ELTrackBar）
#if 0 // GATE (S12-C save) golden cContact.cpp:14404-14404 -- scrbSLK->Position（ELTrackBar）
    WriteIniData(szDir, "Mode", "Head Device Mode", EL<TControl>("TfContact", "scrbSLK")->Position);
#endif // GATE (S12-C save)

    if(IniConfig.bD67LoadCellMeasure)                                           //kevin 20190907 Arm 測區次數道量測 功能;
    {
        WriteIniData(szDir, "Test Arm1", "LoadCellZ1", EL<TEdit>("TfContact", "edLoadCellHeight1")->Text);
        WriteIniData(szDir, "Test Arm2", "LoadCellZ2", EL<TEdit>("TfContact", "edLoadCellHeight2")->Text);
    }

    if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==2)                                             //Steven 20110502 : 240KG
    {
        WriteIniData(szDir, "Mode", "Kit Diameter", 6.0);
    }
    else if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==1)
    {
        WriteIniData(szDir, "Mode", "Kit Diameter", 4.0);
    }
//    else if(rgKitDiameter->ItemIndex==3)
//    {
//        WriteIniData(szDir, "Mode", "Kit Diameter", 5.6);                     //wei 20151005 add 56mm
//    }
    else if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==0)
    {
        WriteIniData(szDir, "Mode", "Kit Diameter", 3.0);
    }
    else
    {
        sBuffer=EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str();
        if(sBuffer=="40x2")
        {
            WriteIniData(szDir, "Mode", "Kit Diameter", 40.2);
        }
        else
        {
            WriteIniData(szDir, "Mode", "Kit Diameter", atof(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str())/10.0);
        }
    }

    WriteIniData(szDir,     "Mode", "KitDiameterMode",                          EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex);              //kevin 20210813 Kit 外徑 選擇  10 尚未有資料

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                                                 //Ifor 20191003 : add Die Force 可以自定義Kit直徑   //AI(ht9045-v899) 20260504: extend Die Force Kit Diameter save to Multi EP (was ==true).
    {
        WriteIniData(szDir, "Mode", "Die Force Kit Diameter",                   atof(EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex].c_str())/10.0);
    }

    WriteIniData(szDir,     "Mode", "Suck Shuttle Device After Tested",         EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceAfterTested")->ItemIndex);
    WriteIniData(szDir,     "Mode", "Suck Shuttle Device Wait On Shuttle",      EL<TRadioGroup>("TfContact", "rgSuckShuttleDeviceWaitOnShuttle")->ItemIndex);                                   //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
    WriteIniData(szDir,     "Mode", "Shuttle Waiting Out Site Chamber",         EL<TRadioGroup>("TfContact", "rgShuttleWaitingOutSiteChamber")->ItemIndex);                                     //jou 980930 add : Shuttle Waiting Out Site Chamber
    WriteIniData(szDir,     "Mode", "Tester Side Push",                         EL<TRadioGroup>("TfContact", "rgTesterSidePush")->ItemIndex);           //Richard 20220321 : 渠梁Side Push
    WriteIniData(szDir,     "Mode", "Tester Side Push Mode",                    EL<TRadioGroup>("TfContact", "rgSidePushMode")->ItemIndex);             //Richard 20220321 : 渠梁Side Push
    WriteIniData(szDir,     "Mode", "Pick Shuttle Device Together",             EL<TRadioGroup>("TfContact", "rgPSDT")->ItemIndex);
    WriteIniData(szDir,     "Mode", "iSocketInitialICCheckPosition",            EL<TComboBox>("TfContact", "coD41")->ItemIndex);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Steven 20220524 : 開放上升兩段速
        WriteIniData(szDir, "Mode", "IndexUpSpeed", 0);
    else
        WriteIniData(szDir, "Mode", "IndexUpSpeed",  EL<TRadioGroup>("TfContact", "rgIndexUpSpeed")->ItemIndex);                                        //kevin 20170524 (wei) add index up speed

    WriteIniData(szDir,     "Mode","dDropByPassDetect",  EL<TEdit>("TfContact", "edDropByPassDetect")->Text);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20160309 Check Pos ition Offset 最大值5
    {
        EL<TEdit>("TfContact", "edD41")->Text=CheckRange(atof(EL<TEdit>("TfContact", "edD41")->Text.c_str()), 5.0, 0.0);
    }
    else                                                                        //Ifor 20160610 add Check Pos ition Offset 最大值20
    {
        EL<TEdit>("TfContact", "edD41")->Text=CheckRange(atof(EL<TEdit>("TfContact", "edD41")->Text.c_str()), 20.0, 0.0);
    }
    WriteIniData(szDir,     "Mode", "fSocketInitialICCheckPositionOffset",      EL<TEdit>("TfContact", "edD41")->Text);

    if(CosFunction.bPurgeBeforePickShuttle)                                     //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
    {
        WriteIniData(szDir, "Mode", "Air Purge Before Pick From Shuttle",       EL<TRadioGroup>("TfContact", "rgPurgeBeforePickShuttle")->ItemIndex);
        WriteIniData(szDir, "Mode", "Air Purge Before Pick Shuttle Time",       EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleTime")->Text);
        WriteIniData(szDir, "Mode", "Air Purge Before Pick Shuttle Interval",   EL<TEdit>("TfContact", "edtPurgeBeforePickShuttleInterval")->Text);
        WriteIniData(szDir, "Mode", "Air Purge Before Pick Shuttle OffSet",     EL<TEdit>("TfContact", "edtPurgeBdforePickShuttleOffSet")->Text);
    }
    //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue end

    WriteIniData(szDir, "Torque Control", "Torque",                             EL<TEdit>("TfContact", "edAirForce")->Text);
    WriteIniData(szDir, "Torque Control", "Pin Number",                         EL<TEdit>("TfContact", "edPinCount")->Text);
    WriteIniData(szDir, "Torque Control", "X Dimension",                        EL<TEdit>("TfContact", "edXDimension")->Text);
    WriteIniData(szDir, "Torque Control", "Y Dimension",                        EL<TEdit>("TfContact", "edYDimension")->Text);
    if(CosFunction.bFixNameOfForcePerPinG)                                      //Steven 20240821 : 凌中心說要修正存檔名稱
    {
        WriteIniData(szDir, "Torque Control", "Force Per Pin N",                EL<TEdit>("TfContact", "edForcePerPinN")->Text);
        WriteIniData(szDir, "Torque Control", "Force Per Pin G",                EL<TEdit>("TfContact", "edForcePerPinG")->Text);
    }
    else
    {
        WriteIniData(szDir, "Torque Control", "Force Per Pin",                  EL<TEdit>("TfContact", "edForcePerPinN")->Text);
        WriteIniData(szDir, "Torque Control", "Force Per Pin Kg",               EL<TEdit>("TfContact", "edForcePerPinG")->Text);
    }

    WriteIniData(szDir, "Torque Control", "iIndexTorqueMax",                    EL<TEdit>("TfContact", "edtTorqueMax")->Text);
    WriteIniData(szDir, "Torque Control", "iIndexTorqueCmp",                    EL<TEdit>("TfContact", "edtTorqueCmp")->Text);
    WriteIniData(szDir, "Torque Control", "bUseAddWeight",                      EL<TCheckBox>("TfContact", "chkUseAddWeight")->Checked);              //kevin 20170629 (Steven) direct heat add weight
    WriteIniData(szDir, "Torque Control", "dZ1Torue",                           EL<TPanel>("TfContact", "palTorqueArm1")->Caption);                //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    WriteIniData(szDir, "Torque Control", "dZ2Torue",                           EL<TPanel>("TfContact", "palTorqueArm2")->Caption);
    WriteIniData(szDir, "Torque Control", "Enable Use Universal Kit",           EL<TCheckBox>("TfContact", "cbEnableUK")->Checked);                   //Frank 20171030 (Steven) add Enable Universal Kit

    WriteIniData(szDir, "Torque Control", "Double Force",                       EL<TEdit>("TfContact", "edDoubleForce")->Text);                   //Ifor 20190104 : Add Double EP Control
    WriteIniData(szDir, "Torque Control", "Pin of Die",                         EL<TEdit>("TfContact", "edtPinOfDie")->Text);                     //Steven 20191106: Pin of Die

    WriteIniData(szDir, "Height Calibration", "Auto Contact Test Count",        EL<TEdit>("TfContact", "edAutoContactTest")->Text);               //Steven 20150224 : Auto Contact Test   //Steven 20180619 (Jou)

    WriteIniData(szDir, "Torque Control", "Die Force Per Pin",                  EL<TEdit>("TfContact", "edDieForcePerPinN")->Text);
    WriteIniData(szDir, "Torque Control", "Die Force Per Pin Kg",               EL<TEdit>("TfContact", "edDieForcePerPinG")->Text);
    WriteIniData(szDir, "Height Calibration", "Test Contact Count",             EL<TEdit>("TfContact", "edTestContactCount")->Text);              //KevinCheng 20260115 : NV Contact Test
    WriteIniData(szDir, "Height Calibration", "Test Sec",                       EL<TEdit>("TfContact", "edTestSec")->Text);                       //KevinCheng 20260115 : NV Contact Test

    if(CosFunction.bAutoHeightSHTReleaseByFile)                                 //Sam 20200217
    {
         WriteIniData(szDir, "Mode", "AutoKSHTOfs",       EL<TEdit>("TfContact", "edAutoKSHTReleaseOfs")->Text);
    }
    AddSpace(szDir);

    szDir2=fOffSet->GetOffsetPath();                                            //Steven 20190109 : 整合Offset路徑
    MyForceDirectories(szDir2);
    szDir2+="\\Position Offset.Data";

    if(CosFunction.bContactHaveOffset==false)                                   //ChungHung 20140327 add by Customer 強制Contact Offset為0
    {
        WriteIniData(szDir2, "Test Arm1", "Contact", 0);                        //ChungHung 20140530 fix 應該是 Test Arm1 and Test Arm2
        WriteIniData(szDir2, "Test Arm2", "Contact", 0);
    }
    else if(CosFunction.bContactShowOffset)                                     //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    {
        double dArm1Offset=atof(EL<TEdit>("TfContact", "edContactOffsetArm1")->Text.c_str());             //Ifor 20170117 add Contact Offset 上下極限卡控，避免人員手Key超過極限值
        double dArm2Offset=atof(EL<TEdit>("TfContact", "edContactOffsetArm2")->Text.c_str());

        if(CUSTOMER_CODE!=CC_KYEC_LEE)
        {
            if(dArm1Offset<=InputLimit.dContactLow)
                EL<TEdit>("TfContact", "edContactOffsetArm1")->Text=InputLimit.dContactLow;
            else if(dArm1Offset>=InputLimit.dContactHigh)
                EL<TEdit>("TfContact", "edContactOffsetArm1")->Text=InputLimit.dContactHigh;

            if(dArm2Offset<=InputLimit.dContactLow)
                EL<TEdit>("TfContact", "edContactOffsetArm2")->Text=InputLimit.dContactLow;
            else if(dArm2Offset>=InputLimit.dContactHigh)
                EL<TEdit>("TfContact", "edContactOffsetArm2")->Text=InputLimit.dContactHigh;
        }
        else
        {
                                                                                //Ifor 20191004 : Add KYEC 舊有設定值未修改不卡上下限
        }

        WriteIniData(szDir2, "Test Arm1", "Contact", EL<TEdit>("TfContact", "edContactOffsetArm1")->Text);                                        //ChungHung 20140530 fix 應該是 Test Arm1 and Test Arm2
        WriteIniData(szDir2, "Test Arm2", "Contact", EL<TEdit>("TfContact", "edContactOffsetArm2")->Text);
        Offset_File.iIndexArmContact[0]=atof(EL<TEdit>("TfContact", "edContactOffsetArm1")->Text.c_str());                                        //Steven 20150512 : Offset --> Offset_File
        Offset_File.iIndexArmContact[1]=atof(EL<TEdit>("TfContact", "edContactOffsetArm2")->Text.c_str());
        Offset.iIndexArmContact[0]  = iUnitMultiply100(Offset_File.iIndexArmContact[0]);                                //Steven 20150512 : Offset --> Offset_File
        Offset.iIndexArmContact[1]  = iUnitMultiply100(Offset_File.iIndexArmContact[1]);
    }
    else if(IniConfig.bChangeKitNoHardStop)                                     //jou 2015-12-08 Xilinx 驗證用
    {
        WriteIniData(szDir2, "Test Arm1", "Contact", EL<TEdit>("TfContact", "edContactOffsetArm1")->Text);
        WriteIniData(szDir2, "Test Arm2", "Contact", EL<TEdit>("TfContact", "edContactOffsetArm2")->Text);
        WriteIniData(szDir2, "Test Arm1", "Pick Up", EL<TEdit>("TfContact", "edShtPickOffset1")->Text);
        WriteIniData(szDir2, "Test Arm2", "Pick Up", EL<TEdit>("TfContact", "edShtPickOffset2")->Text);
        Offset_File.iIndexArmContact[0]=atof(EL<TEdit>("TfContact", "edContactOffsetArm1")->Text.c_str());
        Offset_File.iIndexArmContact[1]=atof(EL<TEdit>("TfContact", "edContactOffsetArm2")->Text.c_str());
        Offset_File.iIndexArmPickUp[0]=atof(EL<TEdit>("TfContact", "edShtPickOffset1")->Text.c_str());
        Offset_File.iIndexArmPickUp[1]=atof(EL<TEdit>("TfContact", "edShtPickOffset2")->Text.c_str());
        Offset.iIndexArmContact[0]  = iUnitMultiply100(Offset_File.iIndexArmContact[0]);
        Offset.iIndexArmContact[1]  = iUnitMultiply100(Offset_File.iIndexArmContact[1]);
        Offset.iIndexArmPickUp[0]   = iUnitMultiply100(Offset_File.iIndexArmPickUp[0]);
        Offset.iIndexArmPickUp[1]   = iUnitMultiply100(Offset_File.iIndexArmPickUp[1]);
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20151228 修正ShtPickOffset存檔讀取
    {
        WriteIniData(szDir2, "Test Arm1", "Pick Up", EL<TEdit>("TfContact", "edShtPickOffset1")->Text);
        WriteIniData(szDir2, "Test Arm2", "Pick Up", EL<TEdit>("TfContact", "edShtPickOffset2")->Text);
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        WriteIniData(szDir, "Torque Control", "Original Force Per Pin Kg",  FloatToStr(fProductionInfo->dOriginal_Device_Pin_Force));                           //wei 20150727
        WriteIniData(szDir, "Torque Control", "Original Pin Number",        FloatToStr(fProductionInfo->dOriginal_Device_Pin_Count));
    }

}

// golden cContact.cpp:18966  TfContact::CalculateTotalAirForce(double dBallCount, double dSingleGf)
static double DF_CalculateTotalAirForce(double dBallCount, double dSingleGf)
{

    AnsiString Str;
    AnsiString sBuffer;
    double dTotalForce=0.0;
    double fComplianceUnit=1.0, dKitDiameter=30.0;
    double dMinKgPerHead=1.0;                                                   //Ifor 20191114 : add Heat 最小公斤數保護
    double dNowKgPerHead=0.0, dHeadMaxForce=0.0;
    int iTag=-1;
    double dDeviceGf=dBallCount*dSingleGf*0.001;                                // device要壓的公斤     //JimmyChiu 20220119 /1000=>*0.001

    if(TestIF.iTestMode==DualSite &&
       TestIF_File.bQualSite2X2Shift &&
       TestIF_File.bNS7000kit)                                                  //kevin 20170513 (wei) add
        dDutCount=4;

//    if(CosFunction.bUseDynamicKitDiameter)                                    //Steven 20170605 (wei) : 可以自定義Kit直徑  //Steven 20230901 : 統一只有這個版本
//    {
    switch((int)EL<filerw::ELTrackBar>("TfContact", "scrbSLK")->Position)   // golden cContact.cpp:18984-18984 取代：scrbSLK->Position（ELTrackBar）。golden iconv 行號 :18985；產生器 cp950 解碼在 :18744-18966 間少算一行，行號以產生器為準、內容已核對
#if 0 // GATE (S12-C save) golden cContact.cpp:18984-18984 -- scrbSLK->Position（ELTrackBar）。golden iconv 行號 :18985；產生器 cp950 解碼在 :18744-18966 間少算一行，行號以產生器為準、內容已核對
    switch(EL<TControl>("TfContact", "scrbSLK")->Position)
#endif // GATE (S12-C save)
    {
        case 2:                                                                 // 1 Device with 1 Compliance Unit
            fComplianceUnit=1.0;    break;
        case 3:                                                                 // 2 Device with 1 Compliance Unit
            fComplianceUnit=0.5;    break;
        case 4:                                                                 // 4 Device with 1 Compliance Unit
            fComplianceUnit=0.25;   break;
        case 5:                                                                 // 2 Device with 4 Compliance Unit
            fComplianceUnit=2.0;    break;
        case 6:                                                                 // 8 Device with 1 Compliance Unit
            fComplianceUnit=0.125;  break;
    }

    if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex>=0 &&                                           //Ifor 20191114 : Contact最小值保護往上移
       EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex<EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Count)
    {
        for(unsigned int i=0; i<fContactForce->SLKClass.size(); i++)
        {
            double d1=fContactForce->SLKClass[i]->dDiameter;
            double d2=atof(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str());
            sBuffer=EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str();
            if(sBuffer=="40x2")
            {
                d2=402;
            }

            if(d1==d2)
            {
                iTag=i;
                dKitDiameter=d2;
                if(dKitDiameter==402)
                    dKitDiameter=40;
            }
        }
    }

    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirForce")->Color           =clWhite;                                       //Steven 20230901 : 要先重置顏色
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirKPA")->Color             =clWhite;
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edSetKg")->Color              =clWhite;
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceKG")->Color   =clWhite;
    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceN")->Color    =clWhite;

    if(IniConfig.bD28MaxForceLimitByDiameter)                                   //Steven 20200813 : 用缸徑計算最大壓力
    {
        EL<TLabel>("TfContact", "lblMaxForcePerIC")->Visible=true;
        double _coefficient=(EP_MAXKPA<=500)?5.0:6.0;                           //Jimmychiu 20211021 : #P211018-ATK-H9-01 , V3.21.701.1 , It does not work [D28] Maxium Force by Kit Diameter.
        dHeadMaxForce=(((dKitDiameter*dKitDiameter*3.14)/4.0)*_coefficient*0.0101972)*fComplianceUnit;                  //4 = 2*2 ;  2=diameter to radius
        Str.sprintf("Max force per compliance: %0.2fkg", ChangeToFloatNonPcnt((double)(dHeadMaxForce), (double)(fComplianceUnit)));
        EL<TLabel>("TfContact", "lblMaxForcePerIC")->Caption=Str;
        if(dDeviceGf>dHeadMaxForce)
        {
            dDeviceGf=dHeadMaxForce;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirKPA")->Color             =clRed;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edSetKg")->Color              =clRed;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceKG")->Color   =clRed;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceN")->Color    =clRed;
            EL<TEdit>("TfContact", "edForcePerDeviceKG")->Text=FormatFloat("0.0000", dDeviceGf);
            EL<TEdit>("TfContact", "edForcePerDeviceN")->Text=FormatFloat("0.0000", dDeviceGf*9.8);       //Jimmychiu 20211021 : #P211018-ATK-H9-01 , V3.21.701.1 , It does not work [D28] Maxium Force by Kit Diameter.
        }
    }
    else
    {
        EL<TLabel>("TfContact", "lblMaxForcePerIC")->Visible=false;
    }

    dTotalForce=dDeviceGf*dDutCount;                                            //JimmyChiu 20220119 模組化計算AirForce

    if(iTag==-1 || iTag>(int)fContactForce->SLKClass.size())
        iTag=0;
    dMinForce=DF_GetMinForce(dKitDiameter, iTag);                                  //JimmyChiu 20220119 獨立檢查最小空壓

    switch(TestIF.iTestMode)
    {
        case SingleSite:
            dNowKgPerHead=dTotalForce/(1.0*fComplianceUnit);                    //Steven 20231110 : Fix for min force of 1x1
            dMinKgPerHead=dMinForce*(1.0*fComplianceUnit);
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                                                              //Frank 20200520 2X2NN Mode
            if(((LastSet.bUseTestSocket[0][0][1]==false && LastSet.bUseTestSocket[1][0][1]==false)  ||
                (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false)) &&                  //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                IniConfig.bD27UseSingleSite85kg)                                                                        //2012-01-03    Dell 在1X2模式下,關Site能達85kg
            {
                dNowKgPerHead=dTotalForce/(1.0*fComplianceUnit);                                                        //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(1.0*fComplianceUnit);                                                          //Ifor 20191114 : add Heat 最小公斤數保護
            }
            else
            {
                dNowKgPerHead=dTotalForce/(2.0*fComplianceUnit);                                                        //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(2.0*fComplianceUnit);                                                          //Ifor 20191114 : add Heat 最小公斤數保護
            }
            break;
        case DualSite2x1:
            if(((LastSet.bUseTestSocket[0][1][0]==false && LastSet.bUseTestSocket[1][1][0]==false)  ||
                (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false)) &&                  //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                IniConfig.bD27UseSingleSite85kg)                                                                        //2012-01-03    Dell 在1X2模式下,關Site能達85kg
            {
                dNowKgPerHead=dTotalForce/(1.0*fComplianceUnit);                                                        //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(1.0*fComplianceUnit);                                                          //Ifor 20191114 : add Heat 最小公斤數保護
            }
            else
            {
                dNowKgPerHead=dTotalForce/(2.0*fComplianceUnit);                                                        //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(2.0*fComplianceUnit);                                                          //Ifor 20191114 : add Heat 最小公斤數保護
            }
            break;
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
        case TriSite1X3:
            dNowKgPerHead=dTotalForce/(3.0*fComplianceUnit);                    //JimmyChiu 20220119 模組化計算AirForce
            dMinKgPerHead=dMinForce*(3.0*fComplianceUnit);                      //Ifor 20191114 : add Heat 最小公斤數保護
            break;
        case QualSite1X4:                                                       //1x4
        case QualSite2X2:                                                       //2x2
//                case QualSite2X2BS: //2x1 BusyShuttle
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            dNowKgPerHead=dTotalForce/(4.0*fComplianceUnit);                    //JimmyChiu 20220119 模組化計算AirForce
            dMinKgPerHead=dMinForce*(4.0*fComplianceUnit);                      //Ifor 20191114 : add Heat 最小公斤數保護
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            dNowKgPerHead=dTotalForce/(6.0*fComplianceUnit);                    //JimmyChiu 20220119 模組化計算AirForce
            dMinKgPerHead=dMinForce*(6.0*fComplianceUnit);                      //Ifor 20191114 : add Heat 最小公斤數保護
            break;
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
        case _8Site2X4:                                                         //2x4
            if(TestIF_File.bOctal_12Kit)                                        //ChungHung 20140508 add for SCK
            {
                dNowKgPerHead=dTotalForce/(12.0*fComplianceUnit);               //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(12.0*fComplianceUnit);                 //Ifor 20191114 : add Heat 最小公斤數保護
            }
            else
            {
                dNowKgPerHead=dTotalForce/(8.0*fComplianceUnit);                //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(8.0*fComplianceUnit);                  //Ifor 20191114 : add Heat 最小公斤數保護
            }
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
            dNowKgPerHead=dTotalForce/(10.0*fComplianceUnit);                   //JimmyChiu 20220119 模組化計算AirForce
            dMinKgPerHead=dMinForce*(10.0*fComplianceUnit);                     //Ifor 20191114 : add Heat 最小公斤數保護
            break;
        case _12Site2X6:
            dNowKgPerHead=dTotalForce/(12.0*fComplianceUnit);                   //JimmyChiu 20220119 模組化計算AirForce
            dMinKgPerHead=dMinForce*(12.0*fComplianceUnit);                     //Ifor 20191114 : add Heat 最小公斤數保護
            break;
        case _16Site2X8:                                                        //2x8
            if(iCloseSiteModeFor2x8>e2x8Standard)                               //Steven 20260420 : != --> >
            {
                dNowKgPerHead=dTotalForce/(8.0*fComplianceUnit);
                dMinKgPerHead=dMinForce*(8.0*fComplianceUnit);
            }
            else
            {
                dNowKgPerHead=dTotalForce/(16.0*fComplianceUnit);               //JimmyChiu 20220119 模組化計算AirForce
                dMinKgPerHead=dMinForce*(16.0*fComplianceUnit);                 //Ifor 20191114 : add Heat 最小公斤數保護
            }
            break;
        case _32Site4X8N:                                                       //Steven 20140619 : for 32Site
        case _32Site4X8M:
            dNowKgPerHead=dTotalForce/(16.0*fComplianceUnit);                   //JimmyChiu 20220119 模組化計算AirForce  //KenHsieh 20230313 : NN Mode Dutcount 32 -> 16
            dMinKgPerHead=dMinForce*(16.0*fComplianceUnit);                     //Ifor 20191114 : add Heat 最小公斤數保護      //KenHsieh 20230313 : NN Mode Dutcount 32 -> 16
            break;
    }

    IniConfig.iEP_Min_KG=dMinKgPerHead;
    Str.sprintf("Min force per compliance: %0.2fkg", dMinForce);

    if(dNowKgPerHead<=dMinForce)                                                //1個arm要壓的公斤
    {
        dTotalForce=dMinKgPerHead;                                              //Steven 20170705 (wei) : 修正最小Contact Force保護 for 新版contact校驗       //JimmyChiu 20220119 模組化計算AirForce
        if(CUSTOMER_CODE==CC_ASE_KaohSiung && TestIF.iTestMode==QualSite2X2)    //kevin 20191204 ASE KH  Telix   2x2
        {
             dTotalForce=dDeviceGf*dDutCount;                                   //1個arm要壓的公斤 //JimmyChiu 20220119 模組化計算AirForce
             if(dKitDiameter==30)
             {
                dMinForce=1;
             }
             Str.sprintf("Min force per ic: %0.2fkg", dMinForce);
        }
        else
        {
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirKPA")->Color             =clYellow;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edSetKg")->Color              =clYellow;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceKG")->Color   =clYellow;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edForcePerDeviceN")->Color    =clYellow;
            Str.sprintf("Min force per compliance: %0.2fkg", dMinForce);
        }
    }
    EL<TLabel>("TfContact", "lblMinForce")->Caption=Str;

    double dMaxLimit=DF_GetMaxIndexForceLimit();
    if(dMaxLimit>0 && dTotalForce>dMaxLimit)
    {
        ;   // UI-only（HTML 端處理）：EL<TEdit>("TfContact", "edAirForce")->Color=clRed;
//        if(IniConfig.bD28MaxForceLimitByDiameter==false)                      //Steven 20200813 : 用缸徑計算最大壓力
        dTotalForce=dMaxLimit;
    }
    return dTotalForce;

}

// golden cContact.cpp:19184  TfContact::GetMaxIndexForceLimit()
static double DF_GetMaxIndexForceLimit()
{

    double dMaxLimit=0.0;
    if(INDEX_PRESS_TYPE==e240KG)                                                //Steven 20110310 : 240KG
    {
        dMaxLimit=240;
    }
    else if(INDEX_PRESS_TYPE==e120KG)                                           //Steven 20121124 : 120KG
    {
        dMaxLimit=120;
    }
    else if(INDEX_PRESS_TYPE==e500KG)                                           //Steven 20121124 : 500KG
    {
        dMaxLimit=500;
    }
    else if(INDEX_PRESS_TYPE==e400KG)                                           //Steven 20131007 : Index 1.5KW, 400KG
    {
        dMaxLimit=400;
    }
    else if(INDEX_PRESS_TYPE==e360KG)                                           //Steven 20140312 : For HT9045WA 360KG
    {
        dMaxLimit=360;
    }
    else if(INDEX_PRESS_TYPE==e260KG)                                           //Steven 20131007 : Index 1.5KW, 400KG
    {
        dMaxLimit=260;
    }
    else if(INDEX_PRESS_TYPE==e160KG)                                           //ifor 20170215 (wei) add 160KG
    {
        dMaxLimit=160;
    }
    else if(INDEX_PRESS_TYPE==e640KG)                                           //Sam 20210628 : add 640KG
    {
        dMaxLimit=640;
    }
    else if(INDEX_PRESS_TYPE==e800KG)                                           //Ifor 20230830 : add 800KG
    {
        dMaxLimit=800;
    }
    else
    {
        if(dDutCount==1 &&
           (TestIF.iTestMode==DualSite   ||                                     //jou 2012-03-14 加入判斷TestIF.iTestMode==DualSite && IniConfig.bD27UseSingleSite85kg==true，不然dual site最大只會到達 55/2=27.5 KG
            TestIF.iTestMode==SingleSite ||
            TestIF.iTestMode==QualSite2X2N) &&                                  //Steven 20210908 : add 2x2 nn mode
           IniConfig.bD27UseSingleSite85kg==true)
        {
            if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==1)                                     //Steven 20110915 : 40mm浮動頭,最多只能55kgf  <2 ==> ==1
            {
                dMaxLimit=55;
            }
            else if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==2)                                //Steven 20110915: 60mm浮動頭
            {
                dMaxLimit=85;
            }
        }
        else
        {
            dMaxLimit=85;                                                       //Steven 20110914 : 將9045/46都改成85kgf
        }
    }
    return dMaxLimit;

}

// golden cContact.cpp:19248  TfContact::GetMinForce(double dKitDiameter, int iTag)
static double DF_GetMinForce(double dKitDiameter, int iTag)
{

    if(dKitDiameter==20)
    {
        dMinForce=0.5;
        if(IniConfig.bD04MinForceByFile==true)                                  //Steven 20220822 : Min force for different SLK
        {
            if(dMinForce<IniConfig.dD04MinForceByFile_20mm)
                dMinForce=IniConfig.dD04MinForceByFile_20mm;
        }
    }
    else if(dKitDiameter==30 || dKitDiameter==28)                               //Ifor 20191106 : add KYEC 缸徑 28 40 58 56
    {
        dMinForce=1.5;
        if(IniConfig.bD04MinForceByFile==true)                                  //Steven 20220822 : Min force for different SLK
        {
            if(dMinForce<IniConfig.dD04MinForceByFile_30mm)
                dMinForce=IniConfig.dD04MinForceByFile_30mm;
        }
    }
    else if(dKitDiameter==40)
    {
        dMinForce=4.0;
        if(IniConfig.bD04MinForceByFile==true)                                  //Steven 20220822 : Min force for different SLK
        {
            if(dMinForce<IniConfig.dD04MinForceByFile_40mm)
                dMinForce=IniConfig.dD04MinForceByFile_40mm;
        }
    }
    else if(dKitDiameter==60 || dKitDiameter==58)                               //Ifor 20191106 : add KYEC 缸徑 28 40 58 56
    {
        dMinForce=8.0;
        if(IniConfig.bD04MinForceByFile==true)                                  //Steven 20220822 : Min force for different SLK
        {
            if(dMinForce<IniConfig.dD04MinForceByFile_60mm)
                dMinForce=IniConfig.dD04MinForceByFile_60mm;
        }
    }
    else if(dKitDiameter==80)                                                   //Ifor 20240620 : add缸徑 80
    {
        dMinForce=15.0;
        if(IniConfig.bD04MinForceByFile==true)
        {
            if(dMinForce<IniConfig.dD04MinForceByFile_80mm)
                dMinForce=IniConfig.dD04MinForceByFile_80mm;
        }
    }
    else
    {
        if(TestIF_File.bNSKitPress)                                             //kevin 20170807 (Steven) add NS offset
        {
            dMinForce=fContactForce->SLKClass[iTag]->dContactOffset_NS;
        }
        else
        {
            dMinForce=fContactForce->SLKClass[iTag]->dContactOffset;
        }

        if(IniConfig.bD04MinForceByFile==true)                                  //Steven 20190314 : Min force is read from file
        {
            if(dMinForce<IniConfig.dD04MinForceByFile)
                dMinForce=IniConfig.dD04MinForceByFile;
        }
    }
    return dMinForce;

}

// golden cContact.cpp:18662  TfContact::CountDieForceKg(bool bByPinCount)
static void DF_CountDieForceKg(bool bByPinCount)
{

    double dForce, dPinCount, dPinForce;
    int iPinCount;
    AnsiString Str;
    if(bByPinCount)
    {
        dPinCount=atof(EL<TEdit>("TfContact", "edtPinOfDie")->Text.c_str());
        dPinForce=atof(EL<TEdit>("TfContact", "edDieForcePerPinG")->Text.c_str())/1000.0;
        dForce=dPinCount*dPinForce;
        Str.sprintf("%0.2f", dForce);
        EL<TEdit>("TfContact", "edDoubleForce")->Text=Str;
    }
    else
    {
        dForce=atof(EL<TEdit>("TfContact", "edDoubleForce")->Text.c_str())*1000;
        dPinForce=atof(EL<TEdit>("TfContact", "edForcePerPinG")->Text.c_str());
        if(dPinForce!=0)
            iPinCount=dForce/dPinForce;
        else
            iPinCount=dForce/1.0;
        EL<TEdit>("TfContact", "edtPinOfDie")->Text=iPinCount;
    }

}

// golden cContact.cpp:1949  TfContact::edPinCountChange(TObject *Sender)
static void DF_edPinCountChange()
{

    if(fShow==false)                                                            //JimmyChiu 20220121 add
        return;
    DF_ShowArmAndDeviceForce();

}

// golden cContact.cpp:1956  TfContact::edForcePerPinNChange(TObject *Sender)
static void DF_edForcePerPinNChange()
{

    if(fShow==false)                                                            //JimmyChiu 20220121 add
        return;
    double f;
    if(CosFunction.bForecePerPinKGf==false)                                     //JerryYang 20180515 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    {
        f=atof(EL<TEdit>("TfContact", "edForcePerPinN")->Text.c_str());
        f=f*1000.0/9.8;
        EL<TEdit>("TfContact", "edForcePerPinG")->Text=FormatFloat("0.0000", f);
        DF_ShowArmAndDeviceForce();
    }

}

// golden cContact.cpp:1970  TfContact::edDieForcePerPinGChange(TObject *Sender)
static void DF_edDieForcePerPinGChange()
{

    if(fShow==false)                                                            //JimmyChiu 20220121 add
        return;
    double f;
    f=atof(EL<TEdit>("TfContact", "edDieForcePerPinG")->Text.c_str());
    f=f*9.8/1000.0;
    EL<TEdit>("TfContact", "edDieForcePerPinN")->Text=FormatFloat("0.0000", f);

    f=atof(EL<TEdit>("TfContact", "edForcePerPinG")->Text.c_str());
    f=f*9.8/1000.0;
    EL<TEdit>("TfContact", "edForcePerPinN")->Text=FormatFloat("0.0000", f);

    DF_ShowArmAndDeviceForce();
    if(INSTALL_DOUBLE_EP!=0)
        DF_CountDieForceKg(true);                                                  //Steven 20210202 : Pin force被改變時, due force需要連動變更

}

// golden cContact.cpp:15509  TfContact::rgKitDiameterClick(TObject *Sender)
static void DF_rgKitDiameterClick()
{

    if(CosFunction.bUseDynamicKitDiameter==false)
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==2)                                         //jou 2014-04-24 修正選擇Kit Diameter數時,force值不會馬上改變
            DeviceForm_File.dKitDiameter=6.0;
        else if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==1)
            DeviceForm_File.dKitDiameter=4.0;
        else if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex==3)                                    //wei 20151005 add 56mm
            DeviceForm_File.dKitDiameter=5.6;
        else
            DeviceForm_File.dKitDiameter=3.0;
    }
    else                                                                        //Steven 20170605 (wei) : 可以自定義Kit直徑
    {
        if(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex]=="40x2")
        {
            DeviceForm_File.dKitDiameter=402;
        }
        else
        {
            DeviceForm_File.dKitDiameter=atof(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str())/10.0;
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20191106 : add KYEC 缸徑 28 40 58 56
        {
            if(DeviceForm_File.dKitDiameter==2.8)
                DeviceForm_File.dKitDiameter=3;
            else if(DeviceForm_File.dKitDiameter==5.8)
                DeviceForm_File.dKitDiameter=6;
        }
    }

    DF_ShowArmAndDeviceForce();                                                    //Steven 20230901 : 補上
    DF_edAirForceChange();   // golden cContact.cpp:15543-15543 取代：edAirForceChange(this)：事件處理器參數已拿掉
#if 0 // GATE (S12-C save) golden cContact.cpp:15543-15543 -- edAirForceChange(this)：事件處理器參數已拿掉
    DF_edAirForceChange(this);
#endif // GATE (S12-C save)

}

// golden cContact.cpp:17393  TfContact::rgOutKitDiameterClick(TObject *Sender)
static void DF_rgOutKitDiameterClick()
{

    if(CosFunction.bUseDynamicKitDiameter==false)                               //kevin 20170912
    {                                                                           //jou 2014-04-24 修正選擇Kit Diameter數時,force值不會馬上改變
        if(EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex==2)
        {
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=2;
            DeviceForm_File.dKitDiameter=6.0;
        }
        else if(EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex==1)
        {
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=1;
            DeviceForm_File.dKitDiameter=4.0;
        }
        else if(EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex==3)                                 //wei 20151005 add 56mm
        {
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=3;
            DeviceForm_File.dKitDiameter=5.6;
        }
        else
        {
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=0;
            DeviceForm_File.dKitDiameter=3.0;
        }
    }
    else                                                                        //Steven 20170605 : 可以自定義Kit直徑
    {
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20210813
            DeviceForm_File.iKitDiameterMode=EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex;       //kevin 20210813 Kit 外徑 選擇

        if(bKitOutDiameter)                                                     //Ifor 20230214 add:避免Contact 缸徑顯示異常問題
            EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex=EL<TRadioGroup>("TfContact", "rgOutKitDiameter")->ItemIndex;
        DeviceForm_File.dKitDiameter=atof(EL<TRadioGroup>("TfContact", "rgKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgKitDiameter")->ItemIndex].c_str())/10.0;
    }
    DF_ShowArmAndDeviceForce();                                                    //Steven 20230901 : 補上

}

// golden cContact.cpp:19796  TfContact::rgDieForceKitDiameterClick(TObject *Sender)
static void DF_rgDieForceKitDiameterClick()
{

    DeviceForm_File.dDieForceKitDiameter=atof(EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->Items->Strings[EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")->ItemIndex].c_str())/10.0;

}

// golden cContact.cpp:17387  TfContact::chkUseAddWeightClick(TObject *Sender)
static void DF_chkUseAddWeightClick()
{

    DeviceForm_File.bUseAddWeight=EL<TCheckBox>("TfContact", "chkUseAddWeight")->Checked;                     //kevin 20170802 (Steven) add  加重 site * 1.25
    DF_edAirForceChange();   // golden cContact.cpp:17390-17390 取代：edAirForceChange(this)：事件處理器參數已拿掉
#if 0 // GATE (S12-C save) golden cContact.cpp:17390-17390 -- edAirForceChange(this)：事件處理器參數已拿掉
    DF_edAirForceChange(this);
#endif // GATE (S12-C save)

}

// golden cContact.cpp:17430  TfContact::cbEnableUKClick(TObject *Sender)
static void DF_cbEnableUKClick()
{

    if(CosFunction.bUniversalKit)                                               //JerryYang 20171102 (wei) UniversalKit改成客戶功能
    {
        if(EL<TCheckBox>("TfContact", "cbEnableUK")->Checked==true)
        {
            EL<TPanel>("TfContact", "palTorqueArm1")->Visible          =true;
            EL<TPanel>("TfContact", "palTorqueArm2")->Visible          =true;
            EL<TLabel>("TfContact", "labContactOffset")->Visible       =true;
            EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible    =true;
            EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible    =true;
            EL<TLabel>("TfContact", "labShtOffset")->Visible           =true;
            EL<TEdit>("TfContact", "edShtPickOffset1")->Visible       =true;
            EL<TEdit>("TfContact", "edShtPickOffset2")->Visible       =true;
            EL<TLabel>("TfContact", "lblShtPickOfsMM")->Visible        =true;
            EL<TLabel>("TfContact", "lblContactOfsMM")->Visible        =true;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled       =false;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled       =false;
            EL<TEdit>("TfContact", "edPickUp1")->Enabled              =false;
            EL<TEdit>("TfContact", "edPickUp2")->Enabled              =false;
            IniConfig.bRemeberAutoHeight    =true;                              //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
            IniConfig.bChangeKitNoHardStop  =true;                              //jou 2015-12-08 Xilinx 驗證用
        }
        else
        {
            EL<TPanel>("TfContact", "palTorqueArm1")->Visible          =false;
            EL<TPanel>("TfContact", "palTorqueArm2")->Visible          =false;
            EL<TLabel>("TfContact", "labContactOffset")->Visible       =false;
            EL<TEdit>("TfContact", "edContactOffsetArm1")->Visible    =false;
            EL<TEdit>("TfContact", "edContactOffsetArm2")->Visible    =false;
            EL<TLabel>("TfContact", "labShtOffset")->Visible           =false;
            EL<TEdit>("TfContact", "edShtPickOffset1")->Visible       =false;
            EL<TEdit>("TfContact", "edShtPickOffset2")->Visible       =false;
            EL<TLabel>("TfContact", "lblShtPickOfsMM")->Visible        =false;
            EL<TLabel>("TfContact", "lblContactOfsMM")->Visible        =false;
            EL<TEdit>("TfContact", "edContactHeight1")->Enabled       =true;
            EL<TEdit>("TfContact", "edContactHeight2")->Enabled       =true;
            EL<TEdit>("TfContact", "edPickUp1")->Enabled              =true;
            EL<TEdit>("TfContact", "edPickUp2")->Enabled              =true;
            IniConfig.bRemeberAutoHeight    =false;                             //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
            IniConfig.bChangeKitNoHardStop  =false;                             //jou 2015-12-08 Xilinx 驗證用
        }
    }

}

// golden cContact.cpp:14084  TfContact::cbContactModeChange(TObject *Sender)
static void DF_cbContactModeChange()
{

    AnsiString szDir="";

    if(CosFunction.bFixedDropSpeed)                                             //Steven 20131101 : 使用固定的Drop速度
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=false;
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=false;
    }
    else
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=true;                                            //2012-01-03    Dell
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=true;                                            //2012-01-03    Dell
    }

    if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropPlaceShiftContact)                         //Steven 20220607 : DropPlaceShiftContact模式已不使用, 自動跳下一個
    {
        filerw::ELComboIndex(EL<TComboBox>("TfContact", "cbContactMode"), DropContactModeDiffentSpeed);
    }

    if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DirectContactMode ||                           //Steven 20100406 丟測模式會自動把Offset改成2mm
       EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMove)
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Text=0;
        EL<TEdit>("TfContact", "edDropOffset2")->Text=0;
        EL<TEdit>("TfContact", "edDropOffset1")->Enabled=false;                                           //2012-01-03    Dell
        EL<TEdit>("TfContact", "edDropOffset2")->Enabled=false;                                           //2012-01-03    Dell
    }
    else if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContact ||
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContactModeDiffentSpeed ||
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMoveDrop ||
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMoveDropSlowContact ||
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropPlaceShiftContact)                    //ChungHung 20150528 add for 海思 _8Site1x4
    {
        if(CUSTOMER_CODE==CC_KYEC_CHEN && IniConfig.bA16ContactTestDropContact)                                         //wei 20150831
        {
            EL<TEdit>("TfContact", "edDropOffset1")->Text=0;
            EL<TEdit>("TfContact", "edDropOffset2")->Text=0;
        }
        else
        {
            if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContact &&
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190225 : 力成 DirectContact & DropContact 切換後，DropContact 的 Drop 不要變成預設值
            {
                szDir=GetRecipeFileName("Contact.Data");
                EL<TEdit>("TfContact", "edDropOffset1")->Text=FormatFloat("0.00", ReadIniData(szDir, "Test Arm1", "Drop_DropContactMode",    2.0));
                EL<TEdit>("TfContact", "edDropOffset2")->Text=FormatFloat("0.00", ReadIniData(szDir, "Test Arm2", "Drop_DropContactMode",    2.0));
            }
            else
            {
                EL<TEdit>("TfContact", "edDropOffset1")->Text=2;
                EL<TEdit>("TfContact", "edDropOffset2")->Text=2;
            }
        }
    }
    else if(EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DirectContactModeDiffentSpeed ||
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMoveSlowContact ||                       //Steven 20160130 : TMove Soft contact
            EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContactSoftEP)                        //kevin 20220621 add drop soft mode
    {
        if(atoi(EL<TEdit>("TfContact", "edDropOffset1")->Text.c_str())<2)
        {
            EL<TEdit>("TfContact", "edDropOffset1")->Text="2";
        }

        if(atoi(EL<TEdit>("TfContact", "edDropOffset2")->Text.c_str())<2)
        {
            EL<TEdit>("TfContact", "edDropOffset2")->Text="2";
        }
    }
    else                                                                        //kevin 20220621 add other mode 清為 0
    {
        EL<TEdit>("TfContact", "edDropOffset1")->Text=0;
        EL<TEdit>("TfContact", "edDropOffset2")->Text=0;
    }

    if((EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==DropContact ||
        EL<TComboBox>("TfContact", "cbContactMode")->ItemIndex==TMoveDrop) &&                                 //JerryYang 20240111 : add
        CosFunction.bTesterSidePushFunction==true)                              //Richard 20220321 : 渠梁Side Push
    {
        EL<TPanel>("TfContact", "pnlSidePush")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfContact", "pnlSidePush")->Visible=false;
    }

}

// golden cContact.cpp:15274  TfContact::coD41Change(TObject *Sender)
static void DF_coD41Change()
{

    if(EL<TComboBox>("TfContact", "coD41")->ItemIndex==0)
    {
        EL<TEdit>("TfContact", "edD41")->Visible=false;
        EL<TLabel>("TfContact", "labD41")->Visible=false;
    }
    else
    {
        EL<TEdit>("TfContact", "edD41")->Visible=true;
        EL<TLabel>("TfContact", "labD41")->Visible=true;
    }

}

// golden cContact.cpp:2290  TfContact::edContactHeight1Change(TObject *Sender)
static void DF_edContactHeight1Change()
{

    if(fShow==false)                                                            //JimmyChiu 20220121 add
        return;
    filerw::ELTodo("golden cContact.cpp:2294 fOffSet->bEnterSpecialOffset=true not done (TfOffSet has no such member in the port)");   // golden cContact.cpp:2294-2294 取代：fOffSet->bEnterSpecialOffset：移植樹 TfOffSet 沒有這個成員（forms/fContact.h X-18；Offset 頁的 C 形狀用 J.M 自帶）
#if 0 // GATE (S12-C save) golden cContact.cpp:2294-2294 -- fOffSet->bEnterSpecialOffset：移植樹 TfOffSet 沒有這個成員（forms/fContact.h X-18；Offset 頁的 C 形狀用 J.M 自帶）
    fOffSet->bEnterSpecialOffset=true;
#endif // GATE (S12-C save)

}

// golden cContact.cpp:18699  TfContact::edForcePerDeviceKGClick(TObject *Sender)
static void DF_edForcePerDeviceKGClick()
{

    ;   // golden cContact.cpp:18701-18701 取代：fQwertyKey->ShowQwertyKey：螢幕小鍵盤（輸入在頁面）
#if 0 // GATE (S12-C save) golden cContact.cpp:18701-18701 -- fQwertyKey->ShowQwertyKey：螢幕小鍵盤（輸入在頁面）
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 1.0, 120.0);
#endif // GATE (S12-C save)
    double dForce, dPinCount, dPinForce;

    AnsiString Str;
    dPinCount=atoi(EL<TEdit>("TfContact", "edPinCount")->Text.c_str());

    dForce=atof(EL<TEdit>("TfContact", "edForcePerDeviceKG")->Text.c_str())*1000.0;
    if(dPinCount!=0)
        dPinForce=dForce/dPinCount;
    else
        dPinForce=dForce/1;

    EL<TEdit>("TfContact", "edForcePerPinG")->Text=dPinForce;

}

