// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_SetUp 的讀寫檔（HTEditList： → HandlerCondition.Data、Contact.Data（Test Arm1/Arm2 Contact 等）、Temperature.Data（[ATC]））。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cSetUp.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 172 個（header 宣告 196 個）。
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
#include "forms/fSetup.h"
#include "csystem.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "Motor/mymotor.h"
#include "mysensor.h"
#include "atester_shims.h"
#include "BarcodeReader.h"
#include "cMyDB.h"
#include "cinitial.h"
#include "ainarm9045.h"
#include "ATC/ATCInterface.h"

void GetSetupAuth();                      // cAuthority.h:80（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）
extern int  iSetupSiteMapping;           // cAuthority.h:68
extern void SetRunStartMode(eRunStartMode Mode = rsmNull, AnsiString ModeText = "");   // aHotPlateSubstrate.h:943
bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
void FileRW_Setup_ContactDutCount();     // FileRW/TestIF_File_SetUp.cpp 下半（fContactForm->DutCount）
void FileRW_Setup_ContactReadFile();     // FileRW/TestIF_File_SetUp.cpp 下半（fContactForm->ReadFile）
void FileRW_Cleaning_LoadAutoCleanData(); // FileRW/TestIF_File_Cleaning.cpp（golden fCleaning->LoadAutoCleanData，cSetUp.cpp:4043）
extern bool authSetup[8];                // cAuthority.h:66（golden cAuthority.h）
extern bool bAuthCriticalPara[26];       // cAuthority.h:64（golden cAuthority.h）

// golden cSetUp.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;             // golden cSetUp.h:261 —— 本 TU 自己的（見檔頭：不寫 fSetup->fShow）
static bool bNeedPassword=false;     // golden cSetUp.h:265（開頁 DoIniDataToForm 設，存檔 sbUpdateClick 讀）
static bool bOCRNeedPassword=false;  // golden cSetUp.h:253
static bool bShuttleMode=false;      // golden cSetUp.h:255
#define bSavePressed (fSetup->bSavePressed)            // golden cSetUp.h:251 —— 移植樹 ReadFile 讀它
#define bFirstTime (fSetup->bFirstTime)                // golden cSetUp.h:264 —— SECS uHGemHT9045.cpp 寫它
static int iTestMode=TotalTestMode;  // golden cSetUp.h:290（建構子 :127）—— 名稱與 TestIF_File.iTestMode 相同，不能 #define；讀檔器前後與 fSetup->iTestMode 同步
#define iTestModeOcr (fSetup->iTestModeOcr)            // golden cSetUp.h:252
#define bSiteMapHasChange (fSetup->bSiteMapHasChange)  // golden cSetUp.h:289
#define shtMode (fSetup->shtMode)                      // golden cSetUp.h:285 —— 移植樹 SetShtMode／VertifyShtModeisDiff 同一份
#define SetShtMode fSetup->SetShtMode                  // golden :4741 —— 移植樹 forms/fSetup.cpp（純成員寫入，逐行相同）
#define VertifyShtModeisDiff fSetup->VertifyShtModeisDiff   // golden :4750（同上）
#define GetTestMode fSetup->GetTestMode                // golden :2187（純查表）
#define CheckSTMMode fSetup->CheckSTMMode              // golden :2156（移植樹 cSetUp.cpp:358）
#define ReadUseSuckModeFile fSetup->ReadUseSuckModeFile   // golden :2141（移植樹 cSetUp.cpp:336）
static TComboBox   *TestSiteCH[MAX_SOCKET_ROW][MAX_SOCKET_COL];   // golden cSetUp.cpp:49
static TLabel      *TestLabCol[MAX_SOCKET_COL];                   // golden cSetUp.cpp:50
static TLabel      *TestLabRow[MAX_SOCKET_ROW];                   // golden cSetUp.cpp:51
static int  iTestSiteCh[MAX_SOCKET_ROW][MAX_SOCKET_COL];          // golden cSetUp.cpp:52
static int iASMTestMode;                                          // golden cSetUp.cpp:53
static int iASMSiteMap[MAX_SOCKET_ROW][MAX_SOCKET_COL];           // golden cSetUp.cpp:54
static bool bNeedEnterPassword=false;                             // golden cSetUp.cpp:55（移植樹 cSetUp.cpp:2046 另有 static 一份）
static AnsiString sSigPassword;                                   // golden cSetUp.cpp:56
static TRadioGroup *MyTempRGBox[iSnSocketCnt];                    // golden cSetUp.cpp:57
static int OrgTestMode=0;                                         // golden cSetUp.cpp:121
static void SU_PortReadFile() { bool f=fSetup->fShow; fSetup->fShow=fShow; fSetup->ScrollBar1->Position=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position; fSetup->rgInOutArmYPitch->ItemIndex=filerw::EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex; fSetup->iTestMode=iTestMode; fSetup->ReadFile(); iTestMode=fSetup->iTestMode; fSetup->fShow=f; }   // golden ReadFile() → 移植樹 fSetup->ReadFile()（cSetUp.cpp:416）
#define ReadFile SU_PortReadFile   // FileRW/TestIF_File_SetUp.cpp 在 include forms/fContact.h 之前 #undef
static void SU_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }   // VCL Clear()：CB_RESETCONTENT
static void SU_CbSetIndex(TComboBox* cb, int i) { if(i>=0 && i<cb->Items->Count) { cb->ItemIndex=i; cb->Text=cb->Items->Strings[i]; } else { cb->ItemIndex=-1; cb->Text=""; } }   // VCL SetItemIndex：CB_SETCURSEL（超出範圍 → 沒有選取）
#define IncludeTrailingPathDelimiter IncludeTrailingBackslash   // vclcompat 只有後者（同義；Public/HTEditList.cpp:643 同做法）

using filerw::EL;

// 前置宣告
static void SU_TfSetup();
static void SU_FormShow();
static void SU_DoIniDataToForm();
static void SU_ScrollBar1Change();
static void SU_CompChange(int iMode);
static void SU_chkOffCenterkitClick();
static void SU_rgYPitchOffsetModeClick();
static void SU_FormClose();
static void SU_sbUpdateClick();
static bool SU_CHSetError();
static bool SU_CheckShuttlePitch();
static void SU_SaveSetupFile(AnsiString sPath);
static void SU_CoSocketComboChange();
static void SU_btnLUpToRDownNClick(TObject *Sender);

// golden cSetUp.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void SU_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfSetup", "CoSocketCombo")->Items;
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
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAa")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAb")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAc")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAd")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAe")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAf")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAg")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbAh")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBa")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBb")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBc")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBd")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBe")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBf")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBg")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TComboBox>("TfSetup", "cbBh")->Items;
      s->Add("---");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->Items;
      s->Add("60.0 mm");
      s->Add("63.5 mm");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Items;
      s->Add("Normal Mode");
      s->Add("Search Last Mode");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items;
      s->Add("Use 4 pick unit");
      s->Add("Use 8 pick unit");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgYOffset")->Items;
      s->Add("30mm");
      s->Add("15mm");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->Items;
      s->Add("For input arm");
      s->Add("For index arm");
      s->Add("Y Center Pitch");
    }
    { TStringList* s = EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->Items;
      s->Add("Normal");
      s->Add("Separate");
    }
}

// golden cSetUp.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kSU_ParentOf[][2] = {
    {"Arm1PickArm2Test", "gbArm1PickArm2Test"},
    {"CoSocketCombo", "Panel4"},
    {"Image1", "grpHandlingMode"},
    {"Panel1", "grpHandlingMode"},
    {"Panel2", "palLeft"},
    {"Panel3", "palLeft"},
    {"Panel4", "grpSocketSensor"},
    {"Panel5", "palLeft"},
    {"Panel6", "grpHandlingMode"},
    {"Panel7", "Panel6"},
    {"Panel8", "Panel6"},
    {"ScrollBar1", "grpHandlingMode"},
    {"XPitch", "grpHandlingMode"},
    {"XShiftPitch", "paQualSite2X2Shift"},
    {"YPitch", "grpHandlingMode"},
    {"bDoubleUnloadTray", "tsTrayMap"},
    {"bUseLoadCCD", "tsTrayMap"},
    {"bUseTrayMap", "tsTrayMap"},
    {"btAutoShuttlePitch", "grpHandlingMode"},
    {"btnLDownToRUpZ", "grpSiteMap"},
    {"btnLUpToRDownN", "grpSiteMap"},
    {"btnLUpToRDownZ", "grpSiteMap"},
    {"btnRDownToLUpZ", "grpSiteMap"},
    {"btnRUpToLDownN", "grpSiteMap"},
    {"btnRUpToLDownZ", "grpSiteMap"},
    {"cb12Site10DirectHeater", "grpIndexOption"},
    {"cb16DirectHeater", "grpIndexOption"},
    {"cb16change12DirectHeater", "grpIndexOption"},
    {"cb16change8DirectHeater", "grpIndexOption"},
    {"cb1CableLayoutKit", "grpIndexOption"},
    {"cb1x2Use1x4siteSLK", "grpIndexOption"},
    {"cb2CableLayoutKit", "grpIndexOption"},
    {"cb6CableLayoutKit", "grpIndexOption"},
    {"cbAa", "grpSiteMap"},
    {"cbAb", "grpSiteMap"},
    {"cbAc", "grpSiteMap"},
    {"cbAd", "grpSiteMap"},
    {"cbAe", "grpSiteMap"},
    {"cbAf", "grpSiteMap"},
    {"cbAg", "grpSiteMap"},
    {"cbAh", "grpSiteMap"},
    {"cbArm1OnlyPlaceArm2TestAndSuck", "gbArm1PickArm2Test"},
    {"cbArm1UseHeat", "gbArm1PickArm2Test"},
    {"cbAutoCleanAlso", "gbArm1PickArm2Test"},
    {"cbBa", "grpSiteMap"},
    {"cbBb", "grpSiteMap"},
    {"cbBc", "grpSiteMap"},
    {"cbBd", "grpSiteMap"},
    {"cbBe", "grpSiteMap"},
    {"cbBf", "grpSiteMap"},
    {"cbBg", "grpSiteMap"},
    {"cbBh", "grpSiteMap"},
    {"cbCa", "grpSiteMap"},
    {"cbCb", "grpSiteMap"},
    {"cbCc", "grpSiteMap"},
    {"cbCd", "grpSiteMap"},
    {"cbCe", "grpSiteMap"},
    {"cbCf", "grpSiteMap"},
    {"cbCg", "grpSiteMap"},
    {"cbCh", "grpSiteMap"},
    {"cbCheckArm2Vacuum", "gbArm1PickArm2Test"},
    {"cbDa", "grpSiteMap"},
    {"cbDb", "grpSiteMap"},
    {"cbDc", "grpSiteMap"},
    {"cbDd", "grpSiteMap"},
    {"cbDe", "grpSiteMap"},
    {"cbDf", "grpSiteMap"},
    {"cbDg", "grpSiteMap"},
    {"cbDh", "grpSiteMap"},
    {"cbEnablePreciser", "gbPreciser"},
    {"cbEnablePreciserHotPlate", "gbPreciser"},
    {"cbEnableRealTimeCCD", "gbRTC"},
    {"cbEnableSocketFloat", "gbRTC"},
    {"cbEnableStepAsideMode", "gbRTC"},
    {"cbEnabledPreciserRT", "gbPreciser"},
    {"cbF18", "Panel7"},
    {"cbHotechLayoutKit2x2", "grpIndexOption"},
    {"cbI21", "Panel2"},
    {"cbInUseBackRow", "gbInUseBackRow"},
    {"cbIndSLK", "grpIndexOption"},
    {"cbNS8000H", "grpIndexOption"},
    {"cbNSKitPress", "grpIndexOption"},
    {"cbO12_LifeTimeCount", "grpHeadCondition"},
    {"cbO13_LifeTimeCount", "grpHeadCondition"},
    {"cbO14_LifeTimeCount", "grpHeadCondition"},
    {"cbOcrFunction", "gbOcr"},
    {"cbOctal12Site", "grpIndexOption"},
    {"cbOctal16Site", "grpIndexOption"},
    {"cbOutUseBackRow", "gbOutUseBackRow"},
    {"cbPreventDropfunction", "grpIndexOption"},
    {"cbQualSite2X2Shift", "grpIndexOption"},
    {"cbRTC20CheckFunction", "gbArm1PickArm2Test"},
    {"cbRTC20GiveWayCheck", "gbArm1PickArm2Test"},
    {"cbRTC20OverFlowCheck", "gbArm1PickArm2Test"},
    {"cbRTCICResidueCheck", "gbArm1PickArm2Test"},
    {"cbSingleInArmUseOtherSuck", "grpIndexOption"},
    {"cbSingleSiteSingleHeater", "grpIndexOption"},
    {"cbSingleUseOtherSuck", "grpIndexOption"},
    {"cbSocketSensor", "Panel4"},
    {"cbSquareOctalLayout", "grpIndexOption"},
    {"cbUse1x3siteSLK", "grpIndexOption"},
    {"cbUseRotateForHT7000HPKit", "grpIndexOption"},
    {"cbUseSLKClamp", "gbSocketClamp"},
    {"cbUseTesterDry", "Panel7"},
    {"cbbDualSiteUseOneSuck", "grpIndexOption"},
    {"cbdisibleinitialcheck", "Panel4"},
    {"chk12SiteUse2x8SLK", "grpIndexOption"},
    {"chk2x2Use16siteSLK", "grpIndexOption"},
    {"chkNS7000CS", "grpIndexOption"},
    {"chkOctal80", "grpIndexOption"},
    {"chkOffCenterkit", "grpIndexOption"},
    {"chkReadTorque", "tsCheckTorque"},
    {"chkRotateShuttle", "grpIndexOption"},
    {"chkSiteMapOneCycle", "grpAutoSiteMap"},
    {"chkSiteMapOpenSite", "grpAutoSiteMap"},
    {"chkUseXCenterPitch", "grpUseXCenterPitch"},
    {"edAuto1Count", "tsTrayMap"},
    {"edAuto2Count", "tsTrayMap"},
    {"edAuto3Count", "tsTrayMap"},
    {"edDelayTime", "tsCheckTorque"},
    {"edOverRange", "tsCheckTorque"},
    {"edPreciserXPitch", "gbPreciser"},
    {"edPreciserYPitch", "gbPreciser"},
    {"edYOffset", "grpHandlingMode"},
    {"edtGetValueDelayTime", "tsTrayMap"},
    {"edtRTCFileName", "gbRTC"},
    {"edtXCenterPitch", "grpUseXCenterPitch"},
    {"gbArm1PickArm2Test", "grpFunction"},
    {"gbInUseBackRow", "Panel8"},
    {"gbOcr", "grpFunction"},
    {"gbOutUseBackRow", "Panel8"},
    {"gbPreciser", "Panel7"},
    {"gbRTC", "Panel8"},
    {"gbShuttleMode", "Panel3"},
    {"gbSocketClamp", "grpFunction"},
    {"grpAutoSiteMap", "Panel8"},
    {"grpFunction", "palLeft"},
    {"grpHandlingMode", "palLeft"},
    {"grpHeadCondition", "Panel7"},
    {"grpIndexOption", "Panel3"},
    {"grpShuttleSelect", "gbShuttleMode"},
    {"grpSiteMap", "Panel5"},
    {"grpSocketSensor", "grpFunction"},
    {"grpUseXCenterPitch", "Panel7"},
    {"labColA", "grpSiteMap"},
    {"labColB", "grpSiteMap"},
    {"labColC", "grpSiteMap"},
    {"labColD", "grpSiteMap"},
    {"labColE", "grpSiteMap"},
    {"labColF", "grpSiteMap"},
    {"labColG", "grpSiteMap"},
    {"labColH", "grpSiteMap"},
    {"labRowA", "grpSiteMap"},
    {"labRowB", "grpSiteMap"},
    {"labRowC", "grpSiteMap"},
    {"labRowD", "grpSiteMap"},
    {"labYOffset", "grpHandlingMode"},
    {"lbShowMessage", "gbRTC"},
    {"lblXPMM", "grpHandlingMode"},
    {"lblXPitch", "grpHandlingMode"},
    {"lblYPitch", "grpHandlingMode"},
    {"paQualSite2X2Shift", "grpHandlingMode"},
    {"palVisibleIndex", "grpHandlingMode"},
    {"pgcASE", "Panel5"},
    {"rbTemp", "Panel2"},
    {"rgInOutArmYPitch", "grpHandlingMode"},
    {"rgSelectSearchLast", "Panel3"},
    {"rgShtModeNormal", "gbShuttleMode"},
    {"rgShtModeOneSide", "gbShuttleMode"},
    {"rgUseSht1", "grpShuttleSelect"},
    {"rgUseSht2", "grpShuttleSelect"},
    {"rgUseSuckMode", "Panel3"},
    {"rgYOffset", "grpHandlingMode"},
    {"rgYPitchOffsetMode", "grpHandlingMode"},
    {"rgseparabilityTest", "gbSocketClamp"},
    {"sbUpdate", "Panel2"},
    {"scrlbxSocketSensor", "grpSocketSensor"},
    {"tsCheckTorque", "pgcASE"},
    {"tsTrayMap", "pgcASE"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void SU_CreateContainerProxies()
{
    EL<TPanel>("TfSetup", "Panel2");
    EL<TPanel>("TfSetup", "Panel3");
    EL<TPanel>("TfSetup", "Panel4");
    EL<TPanel>("TfSetup", "Panel5");
    EL<TPanel>("TfSetup", "Panel6");
    EL<TPanel>("TfSetup", "Panel7");
    EL<TPanel>("TfSetup", "Panel8");
    EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test");
    EL<TGroupBox>("TfSetup", "gbInUseBackRow");
    EL<TGroupBox>("TfSetup", "gbOcr");
    EL<TGroupBox>("TfSetup", "gbOutUseBackRow");
    EL<TGroupBox>("TfSetup", "gbPreciser");
    EL<TGroupBox>("TfSetup", "gbRTC");
    EL<TGroupBox>("TfSetup", "gbShuttleMode");
    EL<TGroupBox>("TfSetup", "gbSocketClamp");
    EL<TGroupBox>("TfSetup", "grpAutoSiteMap");
    EL<TGroupBox>("TfSetup", "grpFunction");
    EL<TGroupBox>("TfSetup", "grpHandlingMode");
    EL<TGroupBox>("TfSetup", "grpHeadCondition");
    EL<TGroupBox>("TfSetup", "grpIndexOption");
    EL<TGroupBox>("TfSetup", "grpShuttleSelect");
    EL<TGroupBox>("TfSetup", "grpSiteMap");
    EL<TGroupBox>("TfSetup", "grpSocketSensor");
    EL<TGroupBox>("TfSetup", "grpUseXCenterPitch");
    EL<TPanel>("TfSetup", "paQualSite2X2Shift");
    EL<TPanel>("TfSetup", "palLeft");
    EL<TPageControl>("TfSetup", "pgcASE");
    EL<TTabSheet>("TfSetup", "tsCheckTorque");
    EL<TTabSheet>("TfSetup", "tsTrayMap");
    filerw::ELSetParents("TfSetup", kSU_ParentOf, (int)(sizeof(kSU_ParentOf) / sizeof(kSU_ParentOf[0])));
}

// golden cSetUp.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void SU_DfmState()
{
    EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Visible = false;
    EL<TCheckBox>("TfSetup", "cb16change8DirectHeater")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbF18")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbNSKitPress")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbRTC20OverFlowCheck")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit")->Visible = false;
    EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck")->Visible = false;
    EL<TCheckBox>("TfSetup", "chkRotateShuttle")->Visible = false;
    EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible = false;
    EL<TEdit>("TfSetup", "XPitch")->Text = "XPitch";
    EL<TEdit>("TfSetup", "XShiftPitch")->Text = "XShiftPitch";
    EL<TComboBox>("TfSetup", "cbAa")->Text = "---";
    EL<TComboBox>("TfSetup", "cbAa")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbAb")->Text = "---";
    EL<TComboBox>("TfSetup", "cbAb")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbAc")->Text = "1";
    EL<TComboBox>("TfSetup", "cbAc")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbAd")->Text = "3";
    EL<TComboBox>("TfSetup", "cbAd")->ItemIndex = 2;
    EL<TComboBox>("TfSetup", "cbAe")->Text = "5";
    EL<TComboBox>("TfSetup", "cbAe")->ItemIndex = 4;
    EL<TComboBox>("TfSetup", "cbAf")->Text = "7";
    EL<TComboBox>("TfSetup", "cbAf")->ItemIndex = 6;
    EL<TComboBox>("TfSetup", "cbAg")->Text = "---";
    EL<TComboBox>("TfSetup", "cbAg")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbAh")->Text = "---";
    EL<TComboBox>("TfSetup", "cbAh")->ItemIndex = 0;
    EL<TCheckBox>("TfSetup", "cbAutoCleanAlso")->Checked = true;
    EL<TComboBox>("TfSetup", "cbBa")->Text = "---";
    EL<TComboBox>("TfSetup", "cbBa")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbBb")->Text = "---";
    EL<TComboBox>("TfSetup", "cbBb")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbBc")->Text = "2";
    EL<TComboBox>("TfSetup", "cbBc")->ItemIndex = 1;
    EL<TComboBox>("TfSetup", "cbBd")->Text = "4";
    EL<TComboBox>("TfSetup", "cbBd")->ItemIndex = 3;
    EL<TComboBox>("TfSetup", "cbBe")->Text = "6";
    EL<TComboBox>("TfSetup", "cbBe")->ItemIndex = 5;
    EL<TComboBox>("TfSetup", "cbBf")->Text = "8";
    EL<TComboBox>("TfSetup", "cbBf")->ItemIndex = 7;
    EL<TComboBox>("TfSetup", "cbBg")->Text = "---";
    EL<TComboBox>("TfSetup", "cbBg")->ItemIndex = 0;
    EL<TComboBox>("TfSetup", "cbBh")->Text = "---";
    EL<TComboBox>("TfSetup", "cbBh")->ItemIndex = 0;
    EL<TCheckBox>("TfSetup", "cbCheckArm2Vacuum")->Checked = true;
    EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked = true;
    EL<TCheckBox>("TfSetup", "cbOctal12Site")->Checked = true;
    EL<TCheckBox>("TfSetup", "cbRTC20CheckFunction")->Checked = true;
    EL<TCheckBox>("TfSetup", "cbRTC20GiveWayCheck")->Checked = true;
    EL<TCheckBox>("TfSetup", "cbRTC20OverFlowCheck")->Checked = true;
    EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK")->Checked = true;
    EL<TEdit>("TfSetup", "edDelayTime")->Text = "0";
    EL<TEdit>("TfSetup", "edOverRange")->Text = "0";
    EL<TEdit>("TfSetup", "edtGetValueDelayTime")->Text = "0";
    EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex = 0;
    EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex = 1;
    EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex = 0;
    EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex = 0;
    EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kSU_EnableAll[] = {
    "Arm1PickArm2Test", "Panel1", "Panel2", "Panel3", "Panel4", "Panel5",
    "Panel6", "Panel7", "Panel8", "bDoubleUnloadTray", "bUseLoadCCD", "bUseTrayMap",
    "btAutoShuttlePitch", "btnLDownToRUpZ", "btnLUpToRDownN", "btnLUpToRDownZ", "btnRDownToLUpZ", "btnRUpToLDownN",
    "btnRUpToLDownZ", "cb12Site10DirectHeater", "cb16DirectHeater", "cb16change12DirectHeater", "cb16change8DirectHeater", "cb1CableLayoutKit",
    "cb1x2Use1x4siteSLK", "cb2CableLayoutKit", "cb6CableLayoutKit", "cbArm1OnlyPlaceArm2TestAndSuck", "cbArm1UseHeat", "cbAutoCleanAlso",
    "cbCheckArm2Vacuum", "cbEnablePreciser", "cbEnablePreciserHotPlate", "cbEnableRealTimeCCD", "cbEnableSocketFloat", "cbEnableStepAsideMode",
    "cbEnabledPreciserRT", "cbF18", "cbHotechLayoutKit2x2", "cbI21", "cbInUseBackRow", "cbIndSLK",
    "cbNS8000H", "cbNSKitPress", "cbO12_LifeTimeCount", "cbO13_LifeTimeCount", "cbO14_LifeTimeCount", "cbOcrFunction",
    "cbOctal12Site", "cbOctal16Site", "cbOutUseBackRow", "cbPreventDropfunction", "cbQualSite2X2Shift", "cbRTC20CheckFunction",
    "cbRTC20GiveWayCheck", "cbRTC20OverFlowCheck", "cbRTCICResidueCheck", "cbSingleInArmUseOtherSuck", "cbSingleSiteSingleHeater", "cbSingleUseOtherSuck",
    "cbSocketSensor", "cbSquareOctalLayout", "cbUse1x3siteSLK", "cbUseRotateForHT7000HPKit", "cbUseSLKClamp", "cbUseTesterDry",
    "cbbDualSiteUseOneSuck", "cbdisibleinitialcheck", "chk12SiteUse2x8SLK", "chk2x2Use16siteSLK", "chkNS7000CS", "chkOctal80",
    "chkOffCenterkit", "chkReadTorque", "chkRotateShuttle", "chkSiteMapOneCycle", "chkSiteMapOpenSite", "chkUseXCenterPitch",
    "gbArm1PickArm2Test", "gbInUseBackRow", "gbOcr", "gbOutUseBackRow", "gbPreciser", "gbRTC",
    "gbShuttleMode", "gbSocketClamp", "grpAutoSiteMap", "grpFunction", "grpHandlingMode", "grpHeadCondition",
    "grpIndexOption", "grpShuttleSelect", "grpSiteMap", "grpSocketSensor", "grpUseXCenterPitch", "labColA",
    "labColB", "labColC", "labColD", "labColE", "labColF", "labColG",
    "labColH", "labRowA", "labRowB", "labRowC", "labRowD", "labYOffset",
    "lbShowMessage", "lblXPMM", "lblXPitch", "lblYPitch", "paQualSite2X2Shift", "palLeft",
    "palVisibleIndex", "pgcASE", "rbTemp", "rgInOutArmYPitch", "rgSelectSearchLast", "rgShtModeNormal",
    "rgShtModeOneSide", "rgUseSht1", "rgUseSht2", "rgUseSuckMode", "rgYOffset", "rgYPitchOffsetMode",
    "rgseparabilityTest", "sbUpdate", "scrlbxSocketSensor", "tsCheckTorque", "tsTrayMap",
};

// 存檔流程（sbUpdateClick、CHSetError、CheckShuttlePitch、SaveSetupFile）讀的替身 91 個 —— FileRW/TestIF_File_SetUp.cpp 用它決定頁面必須送哪些值
static const char* const kSU_SaveReads[] = {
    "Arm1PickArm2Test", "CoSocketCombo", "ScrollBar1", "XPitch", "XShiftPitch", "YPitch",
    "bDoubleUnloadTray", "bUseLoadCCD", "bUseTrayMap", "cb12Site10DirectHeater", "cb16DirectHeater", "cb16change12DirectHeater",
    "cb16change8DirectHeater", "cb1CableLayoutKit", "cb1x2Use1x4siteSLK", "cb2CableLayoutKit", "cb6CableLayoutKit", "cbArm1OnlyPlaceArm2TestAndSuck",
    "cbArm1UseHeat", "cbAutoCleanAlso", "cbCheckArm2Vacuum", "cbEnablePreciser", "cbEnablePreciserHotPlate", "cbEnableRealTimeCCD",
    "cbEnableSocketFloat", "cbEnableStepAsideMode", "cbEnabledPreciserRT", "cbF18", "cbHotechLayoutKit2x2", "cbI21",
    "cbInUseBackRow", "cbIndSLK", "cbNS8000H", "cbNSKitPress", "cbO12_LifeTimeCount", "cbO13_LifeTimeCount",
    "cbO14_LifeTimeCount", "cbOcrFunction", "cbOctal12Site", "cbOctal16Site", "cbOutUseBackRow", "cbPreventDropfunction",
    "cbQualSite2X2Shift", "cbRTC20CheckFunction", "cbRTC20GiveWayCheck", "cbRTC20OverFlowCheck", "cbRTCICResidueCheck", "cbSingleInArmUseOtherSuck",
    "cbSingleSiteSingleHeater", "cbSingleUseOtherSuck", "cbSocketSensor", "cbSquareOctalLayout", "cbUse1x3siteSLK", "cbUseRotateForHT7000HPKit",
    "cbUseSLKClamp", "cbUseTesterDry", "cbbDualSiteUseOneSuck", "cbdisibleinitialcheck", "chk12SiteUse2x8SLK", "chk2x2Use16siteSLK",
    "chkNS7000CS", "chkOctal80", "chkOffCenterkit", "chkReadTorque", "chkRotateShuttle", "chkSiteMapOneCycle",
    "chkSiteMapOpenSite", "chkUseXCenterPitch", "edAuto1Count", "edAuto2Count", "edAuto3Count", "edDelayTime",
    "edOverRange", "edPreciserXPitch", "edPreciserYPitch", "edYOffset", "edtGetValueDelayTime", "edtRTCFileName",
    "edtXCenterPitch", "gbOcr", "gbRTC", "rgInOutArmYPitch", "rgSelectSearchLast", "rgShtModeNormal",
    "rgShtModeOneSide", "rgUseSht1", "rgUseSht2", "rgUseSuckMode", "rgYOffset", "rgYPitchOffsetMode",
    "rgseparabilityTest",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void SU_CreateSaveProxies()
{
    EL<TCheckBox>("TfSetup", "Arm1PickArm2Test");
    EL<TComboBox>("TfSetup", "CoSocketCombo");
    EL<TControl>("TfSetup", "ScrollBar1");
    EL<TEdit>("TfSetup", "XPitch");
    EL<TEdit>("TfSetup", "XShiftPitch");
    EL<TEdit>("TfSetup", "YPitch");
    EL<TCheckBox>("TfSetup", "bDoubleUnloadTray");
    EL<TCheckBox>("TfSetup", "bUseLoadCCD");
    EL<TCheckBox>("TfSetup", "bUseTrayMap");
    EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater");
    EL<TCheckBox>("TfSetup", "cb16DirectHeater");
    EL<TCheckBox>("TfSetup", "cb16change12DirectHeater");
    EL<TCheckBox>("TfSetup", "cb16change8DirectHeater");
    EL<TCheckBox>("TfSetup", "cb1CableLayoutKit");
    EL<TCheckBox>("TfSetup", "cb1x2Use1x4siteSLK");
    EL<TCheckBox>("TfSetup", "cb2CableLayoutKit");
    EL<TCheckBox>("TfSetup", "cb6CableLayoutKit");
    EL<TCheckBox>("TfSetup", "cbArm1OnlyPlaceArm2TestAndSuck");
    EL<TCheckBox>("TfSetup", "cbArm1UseHeat");
    EL<TCheckBox>("TfSetup", "cbAutoCleanAlso");
    EL<TCheckBox>("TfSetup", "cbCheckArm2Vacuum");
    EL<TCheckBox>("TfSetup", "cbEnablePreciser");
    EL<TCheckBox>("TfSetup", "cbEnablePreciserHotPlate");
    EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD");
    EL<TCheckBox>("TfSetup", "cbEnableSocketFloat");
    EL<TCheckBox>("TfSetup", "cbEnableStepAsideMode");
    EL<TCheckBox>("TfSetup", "cbEnabledPreciserRT");
    EL<TCheckBox>("TfSetup", "cbF18");
    EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2");
    EL<TCheckBox>("TfSetup", "cbI21");
    EL<TCheckBox>("TfSetup", "cbInUseBackRow");
    EL<TCheckBox>("TfSetup", "cbIndSLK");
    EL<TCheckBox>("TfSetup", "cbNS8000H");
    EL<TCheckBox>("TfSetup", "cbNSKitPress");
    EL<TCheckBox>("TfSetup", "cbO12_LifeTimeCount");
    EL<TCheckBox>("TfSetup", "cbO13_LifeTimeCount");
    EL<TCheckBox>("TfSetup", "cbO14_LifeTimeCount");
    EL<TCheckBox>("TfSetup", "cbOcrFunction");
    EL<TCheckBox>("TfSetup", "cbOctal12Site");
    EL<TCheckBox>("TfSetup", "cbOctal16Site");
    EL<TCheckBox>("TfSetup", "cbOutUseBackRow");
    EL<TCheckBox>("TfSetup", "cbPreventDropfunction");
    EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift");
    EL<TCheckBox>("TfSetup", "cbRTC20CheckFunction");
    EL<TCheckBox>("TfSetup", "cbRTC20GiveWayCheck");
    EL<TCheckBox>("TfSetup", "cbRTC20OverFlowCheck");
    EL<TCheckBox>("TfSetup", "cbRTCICResidueCheck");
    EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck");
    EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater");
    EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck");
    EL<TCheckBox>("TfSetup", "cbSocketSensor");
    EL<TCheckBox>("TfSetup", "cbSquareOctalLayout");
    EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK");
    EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit");
    EL<TCheckBox>("TfSetup", "cbUseSLKClamp");
    EL<TCheckBox>("TfSetup", "cbUseTesterDry");
    EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck");
    EL<TCheckBox>("TfSetup", "cbdisibleinitialcheck");
    EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK");
    EL<TCheckBox>("TfSetup", "chk2x2Use16siteSLK");
    EL<TCheckBox>("TfSetup", "chkNS7000CS");
    EL<TCheckBox>("TfSetup", "chkOctal80");
    EL<TCheckBox>("TfSetup", "chkOffCenterkit");
    EL<TCheckBox>("TfSetup", "chkReadTorque");
    EL<TCheckBox>("TfSetup", "chkRotateShuttle");
    EL<TCheckBox>("TfSetup", "chkSiteMapOneCycle");
    EL<TCheckBox>("TfSetup", "chkSiteMapOpenSite");
    EL<TCheckBox>("TfSetup", "chkUseXCenterPitch");
    EL<TEdit>("TfSetup", "edAuto1Count");
    EL<TEdit>("TfSetup", "edAuto2Count");
    EL<TEdit>("TfSetup", "edAuto3Count");
    EL<TEdit>("TfSetup", "edDelayTime");
    EL<TEdit>("TfSetup", "edOverRange");
    EL<TEdit>("TfSetup", "edPreciserXPitch");
    EL<TEdit>("TfSetup", "edPreciserYPitch");
    EL<TEdit>("TfSetup", "edYOffset");
    EL<TEdit>("TfSetup", "edtGetValueDelayTime");
    EL<TEdit>("TfSetup", "edtRTCFileName");
    EL<TEdit>("TfSetup", "edtXCenterPitch");
    EL<TGroupBox>("TfSetup", "gbOcr");
    EL<TGroupBox>("TfSetup", "gbRTC");
    EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch");
    EL<TRadioGroup>("TfSetup", "rgSelectSearchLast");
    EL<TRadioButton>("TfSetup", "rgShtModeNormal");
    EL<TRadioButton>("TfSetup", "rgShtModeOneSide");
    EL<TRadioButton>("TfSetup", "rgUseSht1");
    EL<TRadioButton>("TfSetup", "rgUseSht2");
    EL<TRadioGroup>("TfSetup", "rgUseSuckMode");
    EL<TRadioGroup>("TfSetup", "rgYOffset");
    EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode");
    EL<TRadioGroup>("TfSetup", "rgseparabilityTest");
}

// golden cSetUp.cpp:123  TfSetup::TfSetup(TComponent* Owner)
static void SU_TfSetup()
{

    bSiteMapHasChange=true;                                                     //Steven 20250102 : 紀錄test mode與site map是否有變更
    iTestMode=TotalTestMode;
    AnsiString Str;
    TComboBox *tempTestSiteCBox[MAX_SOCKET_ROW][MAX_SOCKET_COL]=
    {
        {EL<TComboBox>("TfSetup", "cbAa"), EL<TComboBox>("TfSetup", "cbAb"), EL<TComboBox>("TfSetup", "cbAc"), EL<TComboBox>("TfSetup", "cbAd"), EL<TComboBox>("TfSetup", "cbAe"), EL<TComboBox>("TfSetup", "cbAf"), EL<TComboBox>("TfSetup", "cbAg"), EL<TComboBox>("TfSetup", "cbAh")},
        {EL<TComboBox>("TfSetup", "cbBa"), EL<TComboBox>("TfSetup", "cbBb"), EL<TComboBox>("TfSetup", "cbBc"), EL<TComboBox>("TfSetup", "cbBd"), EL<TComboBox>("TfSetup", "cbBe"), EL<TComboBox>("TfSetup", "cbBf"), EL<TComboBox>("TfSetup", "cbBg"), EL<TComboBox>("TfSetup", "cbBh")},
        {EL<TComboBox>("TfSetup", "cbCa"), EL<TComboBox>("TfSetup", "cbCb"), EL<TComboBox>("TfSetup", "cbCc"), EL<TComboBox>("TfSetup", "cbCd"), EL<TComboBox>("TfSetup", "cbCe"), EL<TComboBox>("TfSetup", "cbCf"), EL<TComboBox>("TfSetup", "cbCg"), EL<TComboBox>("TfSetup", "cbCh")},
        {EL<TComboBox>("TfSetup", "cbDa"), EL<TComboBox>("TfSetup", "cbDb"), EL<TComboBox>("TfSetup", "cbDc"), EL<TComboBox>("TfSetup", "cbDd"), EL<TComboBox>("TfSetup", "cbDe"), EL<TComboBox>("TfSetup", "cbDf"), EL<TComboBox>("TfSetup", "cbDg"), EL<TComboBox>("TfSetup", "cbDh")}
    };
    TLabel *tempTestLabCol[MAX_SOCKET_COL]=
    {
        EL<TLabel>("TfSetup", "labColA"), EL<TLabel>("TfSetup", "labColB"), EL<TLabel>("TfSetup", "labColC"), EL<TLabel>("TfSetup", "labColD"), EL<TLabel>("TfSetup", "labColE"), EL<TLabel>("TfSetup", "labColF"), EL<TLabel>("TfSetup", "labColG"), EL<TLabel>("TfSetup", "labColH")
    };
    TLabel *tempTestLabRow[MAX_SOCKET_ROW]={EL<TLabel>("TfSetup", "labRowA"), EL<TLabel>("TfSetup", "labRowB"), EL<TLabel>("TfSetup", "labRowC"), EL<TLabel>("TfSetup", "labRowD")};

    int iCol, iRow;
    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        iRow=i/MAX_SOCKET_COL;
        iCol=i%MAX_SOCKET_COL;

        if(i<MAX_SOCKET_ROW)
        {
            TestLabRow[i]=tempTestLabRow[i];
            TestLabRow[i]->Visible=false;
        }

        if(i<MAX_SOCKET_COL)
        {
            TestLabCol[i]=tempTestLabCol[i];
            TestLabCol[i]->Visible=false;
        }
        iTestSiteCh[iRow][iCol]=-1;
        TestSiteCH[iRow][iCol]=tempTestSiteCBox[iRow][iCol];
        TestSiteCH[iRow][iCol]->Visible=false;
    }
    fShow=false;

    if(MachineTypeChoice==Type_HT9045) filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Max=_8Site2X4; else if(MachineTypeChoice==Type_HT9046_LS || MachineTypeChoice==Type_HT1032) filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Max=_32Site4X8N; else if(MachineTypeChoice==Type_HT9045_12Site) filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Max=_12Site2X6; else filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Max=_16Site2X8;   // golden cSetUp.cpp:165-181 取代：ScrollBar1->Max（依 MachineTypeChoice）：golden TScrollBar → filerw::ELTrackBar（Max 變了會重新夾 Position，同 VCL）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:165-181 -- ScrollBar1->Max（依 MachineTypeChoice）：golden TScrollBar → filerw::ELTrackBar（Max 變了會重新夾 Position，同 VCL）
    if(MachineTypeChoice==Type_HT9045)
    {
        EL<TControl>("TfSetup", "ScrollBar1")->Max=_8Site2X4;
    }
    else if(MachineTypeChoice==Type_HT9046_LS ||
            MachineTypeChoice==Type_HT1032)
    {
        EL<TControl>("TfSetup", "ScrollBar1")->Max=_32Site4X8N;
    }
    else if(MachineTypeChoice==Type_HT9045_12Site)                              //ChungHung 20130507 add HT9045 updata for 12site 517
    {
        EL<TControl>("TfSetup", "ScrollBar1")->Max=_12Site2X6;
    }
    else
    {
        EL<TControl>("TfSetup", "ScrollBar1")->Max=_16Site2X8;
    }
#endif // GATE (S12-C save)

    iTestModeOcr=-1;                                                            //Steven 20120716 : OCR

    if(!fSetup->tSiteMap) filerw::ELTodo("TfSetup ctor: fSetup->tSiteMap is null -- fSetup->Init() must run before FileRW_Setup_Boot()");   // golden cSetUp.cpp:185-192 取代：tSiteMap=new TStringList＋補 32 個 "0"：移植樹 fSetup->Init()（cSetUp.cpp:1348，同一段 golden 建構子）已建好；SECS EC 3540（uHGemHT9045_EC.cpp:761）拿的是 fSetup->tSiteMap 指標 —— 再 new 一份會讓 SECS 指到舊的
#if 0 // GATE (S12-C save) golden cSetUp.cpp:185-192 -- tSiteMap=new TStringList＋補 32 個 "0"：移植樹 fSetup->Init()（cSetUp.cpp:1348，同一段 golden 建構子）已建好；SECS EC 3540（uHGemHT9045_EC.cpp:761）拿的是 fSetup->tSiteMap 指標 —— 再 new 一份會讓 SECS 指到舊的
    EL<TControl>("TfSetup", "tSiteMap")=new TStringList();                                                 //Steven 20140510 : Secs Gem
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            EL<TControl>("TfSetup", "tSiteMap")->Add("0");
        }
    }
#endif // GATE (S12-C save)

    if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)                                //Steven 20140819 : Y-Pitch 36mm
    {
        EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->Items->Clear();
        EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->Items->Add("60.0 mm");
        EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->Items->Add("36.0 mm");
    }

    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200610 : Socket sensor 改成16顆
    {
    MyTempRGBox[i]=EL<TRadioGroup>("TfSetup", (AnsiString("rgSensor")+AnsiString(i+1)).c_str()); Str.sprintf("Sensor %d usage", i+1); MyTempRGBox[i]->Items->Add("No use"); MyTempRGBox[i]->Items->Add("Has IC"); MyTempRGBox[i]->Items->Add("Floating");   // golden cSetUp.cpp:203-214 取代：MyTempRGBox[i]=new TRadioGroup(this)，Name="rgSensor"+(i+1)：改成具名替身（名稱＝頁面元件 id）。Parent／Height／Caption／Columns／Align／OnClick 是畫面（頁面自己有）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:203-214 -- MyTempRGBox[i]=new TRadioGroup(this)，Name="rgSensor"+(i+1)：改成具名替身（名稱＝頁面元件 id）。Parent／Height／Caption／Columns／Align／OnClick 是畫面（頁面自己有）
        MyTempRGBox[i]=new TRadioGroup(this);
        MyTempRGBox[i]->Parent=EL<TControl>("TfSetup", "scrlbxSocketSensor");
        MyTempRGBox[i]->Name=AnsiString("rgSensor")+AnsiString(i+1);
        MyTempRGBox[i]->Height=36;
        Str.sprintf("Sensor %d usage", i+1);
        MyTempRGBox[i]->Caption=Str;
        MyTempRGBox[i]->Columns=3;
        MyTempRGBox[i]->Items->Add("No use");
        MyTempRGBox[i]->Items->Add("Has IC");
        MyTempRGBox[i]->Items->Add("Floating");
        MyTempRGBox[i]->Align=alTop;
        MyTempRGBox[i]->OnClick=rgSensor1Click;
#endif // GATE (S12-C save)
    }

}

// golden cSetUp.cpp:1615  TfSetup::FormShow(TObject *Sender)
static void SU_FormShow()
{

    GetSetupAuth();
    EL<TGroupBox>("TfSetup", "gbSocketClamp")->Visible=INSTALL_SOCKET_CLAMP;                                //JerryYang 20160602 有改機為分離式才顯示
    EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->Visible=INSTALL_SOCKET_CLAMP;                           //JerryYang 20160602 有改機為分離式才顯示
    EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=true;
    EL<TCheckBox>("TfSetup", "cbI21")->Visible=(CosFunction.bI21EnableASMByRecipe);                         //Steven 20210518 : 吳如春希望Auto site map從工作檔開關

    if(FileRW_InOutArmSuckHasIC() || ShuttleHasIC()==true || IndexHasIC()==true)   // golden cSetUp.cpp:1623-1624 取代：InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1623-1624 -- InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）
    if(InArmSuck.HasIC()==true || OutArmSuck.HasIC()==true ||                   //Isaac 20211019 : fixed for auto site map
        ShuttleHasIC()==true   || IndexHasIC()==true)
#endif // GATE (S12-C save)
        EL<TCheckBox>("TfSetup", "cbI21")->Enabled=false;
    else
        EL<TCheckBox>("TfSetup", "cbI21")->Enabled=(AccessLevel<LevelSet.AccessLevel[167])?false:true;      //Isaac 20210714 : JECT要求獨立權限設定

    if(CUSTOMER_CODE==CC_JCET)                                                  //JerryYang 20170426 (Steven) 江蘇長電吳如春要求不顯示
    {
        EL<TCheckBox>("TfSetup", "cbdisibleinitialcheck")->Visible=false;
    }

    if(fMain->CheckCanChangeRealDummy()==false)                                 //機台內有IC
    {
        EL<TControl>("TfSetup", "ScrollBar1")          ->Visible=false;
        EL<TEdit>("TfSetup", "XPitch")              ->Enabled=false;
        EL<TEdit>("TfSetup", "YPitch")              ->Enabled=false;
        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")  ->Enabled=false;
        EL<TRadioGroup>("TfSetup", "rgUseSuckMode")       ->Enabled=false;
        EL<TGroupBox>("TfSetup", "grpSiteMap")           ->Enabled=false;
        EL<TCheckBox>("TfSetup", "chkOffCenterkit")     ->Enabled=false;
        EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")    ->Enabled=false;
        EL<TGroupBox>("TfSetup", "grpIndexOption")      ->Enabled=false;                                    //ChungHung 20150528 add
        EL<TGroupBox>("TfSetup", "gbSocketClamp")->Enabled      =false;                                     //JerryYang 20160602
        EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->Enabled =false;
        EL<TGroupBox>("TfSetup", "grpUseXCenterPitch")->Enabled =false;                                     //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
        EL<TEdit>("TfSetup", "edtRTCFileName")->Enabled=false;                                          //kevin 20200804

        if(CosFunction.bOneCycleCanChangeArm==false)
        {
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
            EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=false;
            EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=false;
        }
        else
        {
    if(ShuttleHasIC() || IndexHasIC() || FileRW_InOutArmSuckHasIC())   // golden cSetUp.cpp:1658-1658 取代：InArmSuck／OutArmSuck：同上（FileRW/_KitSuck.cpp 轉接）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1658-1658 -- InArmSuck／OutArmSuck：同上（FileRW/_KitSuck.cpp 轉接）
            if(ShuttleHasIC() || IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC())
#endif // GATE (S12-C save)
            {
                EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=false;
            }
            else if(LastSet.iTemperature==Tempture_Hot &&
                    TestIF_File.iShuttleMode==1)                                //JerryYang 20161212 加熱單ARM不能切為雙ARM,避免hang up
            {
                EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=false;
            }
            else
            {
                if(IniConfig.bShuttleModeAccseeLevel==true)                     //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com]
                {                                                               //請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
                    if(AccessLevel<LevelSet.AccessLevel[40])
                    {
                        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
                        EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=false;
                        EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=false;
                    }
                    else
                    {
                        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=true;
                        EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=true;
                        EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=true;
                    }
                }
                else
                {
                    EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=true;
                    EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=true;
                    EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=true;
                }
            }
        }
    }
    else
    {
        EL<TGroupBox>("TfSetup", "grpIndexOption")    ->Enabled=true;                                       //ChungHung 20150528 add
        EL<TGroupBox>("TfSetup", "grpUseXCenterPitch")->Enabled=true;                                       //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
        EL<TEdit>("TfSetup", "edtRTCFileName")    ->Enabled=true;                                       //機台內沒有IC //kevin 20200804
        EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")  ->Enabled=true;

        if((CUSTOMER_CODE==CC_AMKOR_China ||                                    //jou 2012-02-02 安靠技術員只能改shuttle mode其他的不能改
            CUSTOMER_CODE==CC_QUALCOMM) &&                                      //JerryYang 20170412 (Steven) add QUALCOMM
           AccessLevel<iDefSupervisorLevel)                                     //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
        {
            EL<TControl>("TfSetup", "ScrollBar1")          ->Visible=false;
            EL<TEdit>("TfSetup", "XPitch")              ->Enabled=false;
            EL<TEdit>("TfSetup", "YPitch")              ->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")  ->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")       ->Enabled=false;
            EL<TCheckBox>("TfSetup", "chkOffCenterkit")     ->Enabled=false;
        }
        else
        {
            EL<TControl>("TfSetup", "ScrollBar1")          ->Visible=fSecurity->Insufficient(21,false);
            EL<TEdit>("TfSetup", "XPitch")              ->Enabled=fSecurity->Insufficient(21,false);
            EL<TEdit>("TfSetup", "YPitch")              ->Enabled=fSecurity->Insufficient(21,false);
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")  ->Enabled=fSecurity->Insufficient(21,false);
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")       ->Enabled=fSecurity->Insufficient(21,false);
            if(CosFunction.bCanUseBias)                                         //Steven 20190801 : SCC李國旗說要取消BIAS選項
                EL<TCheckBox>("TfSetup", "chkOffCenterkit") ->Enabled=fSecurity->Insufficient(21,false);
            else
                EL<TCheckBox>("TfSetup", "chkOffCenterkit") ->Enabled=false;
        }

        if(IniConfig.bShuttleModeAccseeLevel==true)                             //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com]
        {                                                                       //請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
            if(AccessLevel<LevelSet.AccessLevel[40])
            {
                EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=false;
                EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=false;
            }
            else
            {
                EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=true;
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=true;
                EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=true;
            }
        }
        else
        {
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=fSecurity->Insufficient(21, false);
            EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Enabled=fSecurity->Insufficient(21, false);
            EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Enabled=fSecurity->Insufficient(21, false);
        }

        if(TestIF_File.iShuttleMode==1)                                         //Steven 20101013 單Arm使用多吸嘴會Hang Up
        {
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled=false;
        }
        else
        {
            if((CUSTOMER_CODE==CC_SCC ||                                        //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
               (CUSTOMER_CODE==CC_SCK && AccessLevel<=iDefSupervisorLevel)) &&
                IniConfig.bEnableRms)                                           //ChungHung 20130621 add SCK RMS
            {
                EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=false;
                EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Enabled=false;
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled=false;
            }
            else
            {
                EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=fSecurity->Insufficient(21, false);
                EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled=fSecurity->Insufficient(21, false);
            }
        }

        if((CUSTOMER_CODE==CC_SCC ||                                            //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
            CUSTOMER_CODE==CC_SCK) &&
            IniConfig.bEnableRms)                                               //ChungHung 20130621 add SCK RMS
        {
            EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;                                          //Steven 20111109 : grpSiteMap拉到外面來
        }
        else
        {
            if(IniConfig.bSiteMappingDisable==true)
            {
                if(iSetupSiteMapping==4)
                {
                    EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
                }
                else
                {
                    EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=fSecurity->Insufficient(21, false);
                }
            }
            else
            {
                if((CUSTOMER_CODE==CC_AMKOR_China ||                            //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
                    CUSTOMER_CODE==CC_SIGURD_PeiXing ||                         //JerryYang 20160330 矽格北興也要鎖住site map
                    CUSTOMER_CODE==CC_QUALCOMM) &&                              //JerryYang 20170412 (Steven) add QUALCOMM
                    AccessLevel<iDefSupervisorLevel)
                {
                    EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;                                  //jou 2012-02-02 安靠技術員只能改shuttle mode其他的不能改
                }
                else if(IniConfig.bVTESTFunction==true &&
                        (MOT[MMTrayZ].fHasTray ||
                         MOT[MMTrayY].fHasTray ||
                         HasAnyICInMachine()))
                {
                    EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
                    EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=false;
                }
                else
                {
                    EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=fSecurity->Insufficient(94, false);     //Steven 20140428 : 21 --> 94
                }
            }
        }

        if(CUSTOMER_CODE==CC_SCC && IniConfig.bEnableFTP)                       //Ifor 20181023 add SCC 開啟FTP功能不可修改Setup相關資料僅可開關ARM
        {
            EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=false;
            EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
            EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled=false;
        }

        if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)          //JerryYang 20220311 : ATP鎖定Critical parameter
        {
            if(bAuthCriticalPara[4])
            {
                EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=false;
                EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
            }
        }
    }

    if(IniConfig.bHaveRotateShuttle)
    {
        EL<TCheckBox>("TfSetup", "chkRotateShuttle")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfSetup", "chkRotateShuttle")->Visible=false;
        EL<TCheckBox>("TfSetup", "chkRotateShuttle")->Checked=false;
    }

    EL<TGroupBox>("TfSetup", "grpHeadCondition")->Visible=(CUSTOMER_CODE==CC_SCC);                          //Steven 20220624 : JSCC楊建軍要求跟著工作檔跑

    EL<TGroupBox>("TfSetup", "grpUseXCenterPitch")->Visible=(CosFunction.b2x4SupportCenterPitch==true &&
                                 (TestIF_File.iTestMode==_8Site2X4 ||           //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
                                  TestIF_File.iTestMode==_16Site4X4));          //Sam 20190226 : 16Site4X4

    EL<TGroupBox>("TfSetup", "gbPreciser")->Visible=USE_PRECISER;                                           //Frank 20180410 (Steven) : InArm Preciser Station

    AnsiString S;
    OrgTestMode=TestIF.iTestMode;
    ;   // golden cSetUp.cpp:1855-1855 取代：示意圖（HTML 自己有）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1855-1855 -- 示意圖（HTML 自己有）
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+"2site.bmp");
#endif // GATE (S12-C save)

    EL<TGroupBox>("TfSetup", "grpAutoSiteMap")->Visible=(IniConfig.bVTESTFunction==true);
    fShow=true;                                                                 //Steven 20110907 : 一定要在ReadFile();前面
    ReadFile();
    bSavePressed=false;                                                         //Steven 20110907 : 一定要在ReadFile();後面
    SU_DoIniDataToForm();
        SU_ScrollBar1Change();   // golden cSetUp.cpp:1862-1862 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1862-1862 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    SU_ScrollBar1Change(this);                                                     //Steven 20120514 : 必須在DoIniDataToForm();後面
#endif // GATE (S12-C save)
    SU_DoIniDataToForm();                                                          //Steven 20120705 : 得再做一次,不然Pitch的數字會被改掉

    for(int i=0; i<MAX_SOCKET_ROW; i++)                                         //Steven 20120523 : 必須在ScrollBar1Change();後面
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
                SU_CbSetIndex(TestSiteCH[i][j], TestIF_File.iSiteMap[i][j]);   // golden cSetUp.cpp:1869-1869 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1869-1869 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            TestSiteCH[i][j]->ItemIndex =TestIF_File.iSiteMap[i][j];
#endif // GATE (S12-C save)
            iRecordSiteMapOrder[i][j]   =TestIF_File.iSiteMap[i][j];            //kevin 20161003 Site 順序
        }
    }

    S.sprintf("SetUp  ''%s''  ", GetLastOpenFN());
    ;   // golden cSetUp.cpp:1875-1878 取代：Caption／Left／Top：視窗標題與位置
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1875-1878 -- Caption／Left／Top：視窗標題與位置
    Caption=S;

    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (S12-C save)

    if(IniConfig.bSiteMappingFastSetDisable)                                    //jou 2010-11-23 start : ASE韓國不要快速設定Bin
    {
        EL<TSpeedButton>("TfSetup", "btnLUpToRDownN")->Visible=false;
        EL<TSpeedButton>("TfSetup", "btnLUpToRDownZ")->Visible=false;
        EL<TSpeedButton>("TfSetup", "btnLDownToRUpZ")->Visible=false;
        EL<TSpeedButton>("TfSetup", "btnRUpToLDownZ")->Visible=false;
        EL<TSpeedButton>("TfSetup", "btnRDownToLUpZ")->Visible=false;
        EL<TSpeedButton>("TfSetup", "btnRUpToLDownN")->Visible=false;
#ifdef SOFT_SIMULTE
        EL<TSpeedButton>("TfSetup", "btnLUpToRDownN")->Visible=true;
#endif
    }

    EL<TGroupBox>("TfSetup", "gbRTC")->Visible=REAL_TIME_CCD;                                               //Steven 20110824 : Real time CCD - Set Up可以開關
    EL<TGroupBox>("TfSetup", "gbRTC")->Enabled=true;
    EL<TGroupBox>("TfSetup", "gbOcr")->Visible=INSTALL_OCR;
    EL<TGroupBox>("TfSetup", "gbOcr")->Enabled=true;

        iTestModeOcr    =filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position;   // golden cSetUp.cpp:1898-1898 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1898-1898 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    iTestModeOcr    =EL<TControl>("TfSetup", "ScrollBar1")->Position;                                      //Steven 20120716 : OCR
#endif // GATE (S12-C save)
    EL<TLabel>("TfSetup", "lbShowMessage")->Visible=false;
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //Sam 20240711 : RTC Auto Tuning
        EL<TCheckBox>("TfSetup", "cbEnableStepAsideMode")->Enabled=false;

    EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->Visible=(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635 ||
                               USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360 ||      //JerryYang 20251218 : IN/OUT ARM支援不同模組
                               USE_OUT_ARM_Y_PITCH==iXPitchManual635 ||
                               USE_OUT_ARM_Y_PITCH==iXPitchManual360);

    if(EL<TControl>("TfSetup", "ScrollBar1")->Enabled==true && authSetup[0]==false)                        //Steven 20120405 : Set Up加入Security_new權限
    {
        EL<TControl>("TfSetup", "ScrollBar1")->Enabled=false;
    }

    if(EL<TEdit>("TfSetup", "YPitch")->Enabled==true && authSetup[0]==false)
    {
        EL<TEdit>("TfSetup", "YPitch")->Enabled=false;
    }

    if(EL<TEdit>("TfSetup", "XPitch")->Enabled==true && authSetup[0]==false)
    {
        EL<TEdit>("TfSetup", "XPitch")->Enabled=false;
    }

    if(EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Enabled==true && authSetup[1]==false)
    {
        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Enabled=false;
    }

    if(EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled==true && authSetup[2]==false)
    {
        EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled=false;
    }

    if(EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled==true && authSetup[3]==false)
    {
        EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled=false;
    }

    if(EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled==true && authSetup[4]==false)
    {
        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Enabled=false;
    }

    if(EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled==true && authSetup[5]==false)
    {
        EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
    }

    if(EL<TGroupBox>("TfSetup", "gbRTC")->Enabled==true && authSetup[6]==false)
    {
        EL<TGroupBox>("TfSetup", "gbRTC")->Enabled=false;
    }

    if(IniConfig.bD58UseArm1PickPlaceArm2Test &&
       bUseTwoArm32Site==false)                                                 //Steven 20220819 : 加上保護 for D58
    {
        EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=true;                                       //kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試
        EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Enabled=(AccessLevel>=LevelSet.AccessLevel[165]);
    }
    else
    {
        EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=false;                                      //kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試
        TestIF_File.bArm1PickPlaceArm2Test=false;
    }

    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true ||                                     //Steven 20140922 : Arm2當作指紋測試
       (TestIF_File.bArm1PickPlaceArm2Test &&                                   //kevin 20150127 Arm1 下壓 arm2 測試
        IniConfig.bD58UseArm1PickPlaceArm2Test==true))                          //Ifor 20191122 : 避免D58功能關閉無法開ARM問題
    {
        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;                                           //only use shuttle 1
    }
    else
    {
        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=IniConfig.bD30EnableSiteModeSelect;
    }

    if(EL<TGroupBox>("TfSetup", "gbOcr")->Enabled==true && authSetup[7]==false)                             //ChungHung 20130118 add OCR - 關閉要密碼
    {
        EL<TGroupBox>("TfSetup", "gbOcr")->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
        EL<TCheckBox>("TfSetup", "cbOcrFunction")->Enabled=(AccessLevel>=iDefSupervisorLevel)?true:false;   //Alick 20170106 add OCR權限設定在SUPERVISOR以上

    //這兩個要保持在最下面---------------------------------
    bSiteMapHasChange=false;                                                    //Steven 20250102 : 紀錄test mode與site map是否有變更
    iTestMode=TestIF_File.iTestMode;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            iTestSiteCh[i][j]=TestIF_File.iSiteMap[i][j];
        }
    }

        iASMTestMode=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position;   // golden cSetUp.cpp:1996-1996 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1996-1996 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    iASMTestMode=EL<TControl>("TfSetup", "ScrollBar1")->Position;                                          //Steven 20120927 : for auto site mapping
#endif // GATE (S12-C save)
    for(int i=0; i<MAX_SOCKET_ROW; i++)
        for(int j=0; j<MAX_SOCKET_COL; j++)
            iASMSiteMap[i][j]=TestSiteCH[i][j]->ItemIndex;                      //Steven 20120927 : for auto site mapping

     if(IniConfig.bC08_SocketSensor)                                            //kevin 20130504 socket sensor enanle
     {
         EL<TGroupBox>("TfSetup", "grpSocketSensor")->Visible=true;
         EL<TGroupBox>("TfSetup", "grpSocketSensor")->Enabled=(AccessLevel>=LevelSet.AccessLevel[159]);
     }
     else
     {
         EL<TGroupBox>("TfSetup", "grpSocketSensor")->Visible=false;
     }

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //20140320 wei   KYEC 低於權限顯示不能修改
    {
        if(AccessLevel<LevelSet.AccessLevel[21])
        {
            EL<TGroupBox>("TfSetup", "grpHandlingMode")->Enabled=false;
            EL<TGroupBox>("TfSetup", "grpSiteMap")->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Enabled=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Enabled=false;
            EL<TGroupBox>("TfSetup", "grpIndexOption")->Enabled=false;
        }                                                                       //Frank 20161212 (Jou) Mark Code 機台內有IC就不能更改Setup裡的Site
//        else
//        {
//            grpHandlingMode->Enabled=true;
//            grpSiteMap->Enabled=true;
//            rgSelectSearchLast->Enabled=true;
//            rgUseSuckMode->Enabled=true;
//            grpIndexOption->Enabled=true;
//        }
    }

    if(CUSTOMER_CODE==CC_SCS)
    {
        if(TestIF_File.bNS7000kit==true ||
           TestIF_File.bNS7000CS==true ||
           TestIF_File.bNS8000CS==true)
        {
            filerw::ELMessage("You are using Epson layout kit");
        }
        else
        {
            filerw::ELMessage("You are using HonPrec layout kit");
        }
        EL<TCheckBox>("TfSetup", "chkNS7000CS")->Caption="NS7000 layout kit";
    }

    if(CosFunction.bLockRTC && LastSet.iTester==ON_LINE)                        //JerryYang 20160220 add for Amkor-Philippine,避免人員開關RTC功能
    {
        EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Enabled=false;
    }
    else if(CosFunction.bLockRTC && LastSet.iTester==OFF_LINE)
    {
        EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Enabled=true;
    }

    EL<TPanel>("TfSetup", "paQualSite2X2Shift")->Visible=((TestIF_File.iTestMode==QualSite2X2 ||
                                  TestIF_File.iTestMode==DualSite) &&           //kevin 20170415 (wei) add DualSite
                                 TestIF_File.bQualSite2X2Shift);                //wei 20160226 TSMC X Shift

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20160316
    {
        EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Caption="OnebyOne Direct Heater";                     //kevin 20210923 add 20200417 改名稱
        EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Caption="NS7000 bias kit(偏心Kit)";
        EL<TCheckBox>("TfSetup", "chkNS7000CS")->Caption="NS7000 change socket(真空位置不同)";
        if(TestIF_File.bNSKitPress)
        {
            filerw::ELMessage("You are using Epson layout kit");
        }

        if(bUseNSKitKey)                                                        //kevin 20170814 (Steven) 低FORCE 需軟體鎖住
            EL<TCheckBox>("TfSetup", "cbNSKitPress")->Visible=true;                                         //kevin 20170804

        EL<TTabSheet>("TfSetup", "tsCheckTorque")->TabVisible=true;                                         //kevin 20210806 add 扭力比對功能
        EL<TTabSheet>("TfSetup", "tsTrayMap")->TabVisible=true;
        EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Caption="12/16 Site One by One Direct heater layout";         //kevin 20220503 change name
        EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Caption="12 Site 10 One by One Direct heater layout";   //kevin 20220503 change name
    }
    else
    {
        EL<TTabSheet>("TfSetup", "tsCheckTorque")->TabVisible=false;
        EL<TTabSheet>("TfSetup", "tsTrayMap")->TabVisible=false;
        EL<TPageControl>("TfSetup", "pgcASE")->Visible=false;
    }

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                            //AI(ht9045-v899) 20260430: extend cbIndSLK visibility to Multi EP half.
    {
        EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=true;
    }
    else
    {
        EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=false;
    }

#if 0 // GATE (S12-C save) golden cSetUp.cpp:2093-2112 -- CC_SIGURD_PeiXing：在 C:\Windows 建立／讀 SitMap.ini（site map 密碼）—— 開頁不可以寫系統資料夾，不做
    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Alick 20160602 新增矽格Site map Password用ini檔
    {
        bNeedEnterPassword=true;                                                //Alick 20160603 修改Site map需要密碼
        AnsiString sSigPath="C:\\Windows\\SitMap.ini";                          //Alick 20160603 密碼ini檔存位置
        FILE *fp;
        if(FileExists(sSigPath)==false)
        {
            fp=fopen("C:\\Windows\\SitMap.ini", "w+");
            fprintf(fp, "%s\n", "9045");
            fclose(fp);
        }
        else
        {
            TStringList *sTmp = new TStringList;
            sTmp->LoadFromFile(sSigPath);
            sSigPassword=sTmp->Strings[0];
            sTmp->Clear();                                                      //Ifor 20170603 (wei) TStringList 刪除前先 Clean
            delete sTmp;
        }
    }
#endif // GATE (S12-C save)

    if(IniConfig.bSPILFunction==true)                                           //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
        EL<TCheckBox>("TfSetup", "cbF18")->Visible=true;
    EL<TButton>("TfSetup", "btAutoShuttlePitch")->Visible=(AUTO_SENSOR_INSTALL==1);                       //wei 20160914 Auto Shuttle Sensor

    EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Enabled=fSecurity->Insufficient(155, false);              //JerryYang 20180416 (Steven) : MicroChip Philippine要求加入權限控管

    EL<TCheckBox>("TfSetup", "cbUseTesterDry")->Visible=Sen[SnTesterDryAir].Enable;                         //Ifor 20200115 : add Tester Dry Air Control
    if(CosFunction.bLockRTCByFile)                                              //Sam 20240311 : 新增 RTC Lock by file 功能
    {
    filerw::ELTodo("golden cSetUp.cpp FormShow: fConfiguration->ReadLockByFile() not ported (GATE G-SU-LockByFile) -- IniConfig.bRTC_Enable/bRTC_Active as last read");   // golden cSetUp.cpp:2123-2123 取代：fConfiguration->ReadLockByFile()：移植樹唯一的 fConfiguration 是 W5SckArtRem_ConfigStub，沒有 ReadLockByFile（同移植樹 GATE G-SU-LockByFile）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:2123-2123 -- fConfiguration->ReadLockByFile()：移植樹唯一的 fConfiguration 是 W5SckArtRem_ConfigStub，沒有 ReadLockByFile（同移植樹 GATE G-SU-LockByFile）
        fConfiguration->ReadLockByFile();
#endif // GATE (S12-C save)
        EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Enabled=IniConfig.bRTC_Enable;
        if(IniConfig.bRTC_Enable==false)
            EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked=IniConfig.bRTC_Active;
    }

    if(CosFunction.bDisableOpenAllSiteWhenChangeShtMod)
    {
        SetShtMode(EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked,
                   EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked,
                   EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked,
                   EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked);
    }

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cSetUp.cpp:3044  TfSetup::DoIniDataToForm()
static void SU_DoIniDataToForm()
{

        filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position = TestIF_File.iTestMode;   // golden cSetUp.cpp:3046-3046 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3046-3046 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    EL<TControl>("TfSetup", "ScrollBar1")->Position = TestIF_File.iTestMode;
#endif // GATE (S12-C save)

    for(int i=0; i<MAX_SOCKET_ROW; i++)
        for(int j=0; j<MAX_SOCKET_COL; j++)
                SU_CbSetIndex(TestSiteCH[i][j], TestIF_File.iSiteMap[i][j]);   // golden cSetUp.cpp:3050-3050 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3050-3050 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            TestSiteCH[i][j]->ItemIndex=TestIF_File.iSiteMap[i][j];
#endif // GATE (S12-C save)

    if(TestIF_File.iShuttleMode==0)
    {
        EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked=true;
        iOldShuttleMode=0;
        iWhichShuttle0000=0;                                                    //KEVIN 20170504關Arm2 load tray 不會吸IC  Hang up

        if(TestIF_File.iUseSuckMode==4)                                         //Jimmychiu 20240219 : add for 8site2x4N
        {
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;
        }
        else if(TestIF_File.iUseSuckMode==8)
        {
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=1;
        }
    }
    else
    {
        EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked=true;
        iOldShuttleMode=1;
    }

    if(TestIF_File.iShuttle_Sel==0)
    {
        EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked=true;
        iOldSelShuttle=0;
        iWhichShuttle0000=0;                                                    //KEVIN 20170504關Arm2 (wei) load tray 不會吸IC  Hang up
    }
    else
    {
        EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked=true;
        iOldSelShuttle=1;
        if(TestIF_File.iShuttleMode==1)
            iWhichShuttle0000=1;                                                //KEVIN 20170504 (wei) 關Arm2 load tray 不會吸IC  Hang up
    }

    EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex     = TestIF_File.iYPitchOffsetMode;          //Steven 20201013 : NN mode add Y-Pitch offset selection

    if(IniConfig.bSPILFunction==true)                                           //Ifor 20160614 矽品要求Hotplate 僅可到小數後兩位 (矽品 IT 系統限制)
    {
        EL<TEdit>("TfSetup", "XPitch")->Text   = FormatFloat("0.00", TestIF_File.dSiteXPitch);          //Ifor 20160614 Setup pitch "0.000" -> "0.00"
        EL<TEdit>("TfSetup", "YPitch")->Text   = FormatFloat("0.00", TestIF_File.dSiteYPitch);          //Ifor 20160614 Setup pitch "0.000" -> "0.00"
        if(TestIF_File.iYPitchOffsetMode!=1)                                    //KenHsieh 20231227 : add Y Center Pitch mode   //Steven 20201013 : NN mode add Y-Pitch offset selection
        {
            EL<TEdit>("TfSetup", "edYOffset")->Text= FormatFloat("0.00", TestIF_File.dSiteYOffset);     //Ifor 20160614 Setup pitch "0.000" -> "0.00"
        }
        else
        {
            EL<TEdit>("TfSetup", "edYOffset")->Text=10.00;
        }
    }
    else
    {
        EL<TEdit>("TfSetup", "XPitch")->Text   = FormatFloat("0.000", TestIF_File.dSiteXPitch);         //jou 2016-04-22 Setup pitch "0.00" -> "0.000"
        EL<TEdit>("TfSetup", "YPitch")->Text   = FormatFloat("0.000", TestIF_File.dSiteYPitch);         //jou 2016-04-22 Setup pitch "0.00" -> "0.000"
        if(TestIF_File.iYPitchOffsetMode!=1)                                    //KenHsieh 20231227 : add Y Center Pitch mode   //Steven 20201013 : NN mode add Y-Pitch offset selection
        {
            EL<TEdit>("TfSetup", "edYOffset")->Text= FormatFloat("0.000", TestIF_File.dSiteYOffset);    //Steven 20140512 : For HT-9047 //jou 2016-04-22 Setup pitch "0.00" -> "0.000"
        }
        else
        {
            EL<TEdit>("TfSetup", "edYOffset")->Text=10.000;
        }
    }

    EL<TEdit>("TfSetup", "edPreciserXPitch")->Text=FormatFloat("0.000", TestIF_File.dPreciserXPitch);   //Frank 20180410 (Steven) : InArm Preciser Station
    EL<TEdit>("TfSetup", "edPreciserYPitch")->Text=FormatFloat("0.000", TestIF_File.dPreciserYPitch);
    EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex  =TestIF_File.i1x4SiteYOffset;

    EL<TEdit>("TfSetup", "XShiftPitch")->Text= FormatFloat("0.00", TestIF_File.dShiftXPitch);           //wei 20160226 TSMC X Shift

    if(TestIF_File.bSearchLastMode==false)
        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
    else
        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=1;

    switch(TestIF_File.iTestMode)
    {
        case DualSite:                                                          //Steven 2012.09.21 : 反向
            if(TestIF_File.iUseSuckMode==4)
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=1;                                     // 4 pick unit
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;                                     // 2 pick unit
            }
            break;
        case QualSite1X4:
        case QualSite2X2:
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
            if(USE_PICKER_COUNT!=0)                                             //Steven 20161117 : for HT-9045S
            {
                if(TestIF_File.iUseSuckMode==8)
                    EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=1;                                 // 8 pick unit
                else
                    EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;                                 // 4 pick unit
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;
            }
            break;
        case _8Site2X4:
        case _16Site4X4:
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //Steven 20231207 : 2x4 16 picker
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
            {
                if(TestIF_File.iUseSuckMode==16)
                    EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=1;                                 // 16 pick unit
                else
                    EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;                                 // 4 pick unit
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;
            }
            break;
    }

    EL<TCheckBox>("TfSetup", "chkOffCenterkit")     ->Checked   = TestIF_File.bNS7000kit;                   //jou 981208 : NS7000 bias kit
    EL<TCheckBox>("TfSetup", "chkNS7000CS")         ->Checked   = TestIF_File.bNS7000CS;                    //Steven 20100119
    EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")->Checked     = TestIF_File.bQualSite2X2Shift;            //wei 20160226 TSMC X Shift
    EL<TCheckBox>("TfSetup", "chkRotateShuttle")    ->Checked   = TestIF_File.bRotateShuttle;               //Steven 20100713 : 轉轉蝦頭
    EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD") ->Checked   = !COM2->bCCDDummyRum;                      //Steven 20110824 : Real time CCD - Set Up可以開關
    bNeedPassword                   = EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked;             //Steven 20110907 : Real Time CCD - 關掉要密碼
    EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Checked=TestIF_File.b16Direct12Shuttle;           //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    EL<TCheckBox>("TfSetup", "cb16change8DirectHeater") ->Checked=TestIF_File.b16Direct8Shuttle;            //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle

    EL<TCheckBox>("TfSetup", "cbEnableSocketFloat") ->Checked   =TestIF_File.bUseSocketFloat;               //Frank QQ
    EL<TCheckBox>("TfSetup", "cbEnableSocketFloat") ->Visible   =false;                                     //Ifor 20180301 :未驗證完成暫時不用強制關閉

    EL<TCheckBox>("TfSetup", "cbEnableStepAsideMode")->Checked  =TestIF_File.bUseRTCStepAsideMode;          //Ifor 20230908 add:RTC 讓位功能 By Setup File

    if(IniConfig.bKoreaFunction==false)                                         //ChungHung 20121031 add
        EL<TCheckBox>("TfSetup", "cbNS8000H")       ->Checked   =TestIF_File.bNS8000CS;                     //Steven 20120606 : 16溫控器 8Site使用Hontech頭
    EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Checked      =TestIF_File.b2CableLayoutKit;              //Steven 20150724 : 16溫控器 2條線版本
    EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")   ->Checked   =TestIF_File.b1CableLayoutKit;              //jou 2015-10-15  : 16溫控器 1條線版本
    EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")   ->Checked   = TestIF_File.b6CableLayoutKit;             //Steven 20230613 : ATC6.1 16Site SLK
    EL<TCheckBox>("TfSetup", "cbOcrFunction")       ->Checked   =TestIF_File.bOcrFunction;                  //Steven 20120716 : OCR
    bOCRNeedPassword                =EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked;                    //ChungHung 20130118 add OCR - 關閉要密碼
    EL<TCheckBox>("TfSetup", "chkOctal80")          ->Checked   = TestIF_File.bOctal_80Kit;                 //2013-03-05    Dell    for ATK pitch 80mm kit

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20220624 : JSCC楊建軍要求跟著工作檔跑
    {
        EL<TCheckBox>("TfSetup", "cbO12_LifeTimeCount")->Checked=IniConfig.bLifeTimeCount[0];
        EL<TCheckBox>("TfSetup", "cbO13_LifeTimeCount")->Checked=IniConfig.bLifeTimeCount[1];
        EL<TCheckBox>("TfSetup", "cbO14_LifeTimeCount")->Checked=IniConfig.bLifeTimeCount[2];
    }

    if(CUSTOMER_CODE==CC_JCET &&
       bFirstTimeEnableSocketSensor)                                            //Ifor 20171121 (Steven) : add JCET 如春要求第一次啟動時須強制開啟Socket Sensor 功能
    {
        AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");
        bFirstTimeEnableSocketSensor=false;

        if(JCET_FOR_EVAN)                                                       //Steven 20210806 : 嚴海軍要求切換工作檔時, Socket Sensor與Site Map要默認啟用
        {
            IniConfig.bI21EnableASM=true;
            WriteIniData(szDir, "Auto Site Mapping", "Enable Auto Site Mapping", 1);
            if(CosFunction.bUseOpenCloseSiteMapAtAnyTime==true)                 //Ifor 20190620 : add Auto Site Maping 隨時關閉開關顯示
            {
    filerw::ELTodo("golden DoIniDataToForm: fMain->cbDisableSiteMappingCheck->Visible -- TfMain facade has none (GATE G-SU-SiteMapChk)");   // golden cSetUp.cpp:3213-3213 取代：fMain->cbDisableSiteMappingCheck：TfMain 門面沒有（同移植樹 GATE G-SU-SiteMapChk，主畫面勾選框的顯示）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3213-3213 -- fMain->cbDisableSiteMappingCheck：TfMain 門面沒有（同移植樹 GATE G-SU-SiteMapChk，主畫面勾選框的顯示）
                fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;
#endif // GATE (S12-C save)
            }
            else
            {
    filerw::ELTodo("golden DoIniDataToForm: fMain->cbDisableSiteMappingCheck->Visible=false -- TfMain facade has none (GATE G-SU-SiteMapChk)");   // golden cSetUp.cpp:3217-3217 取代：同上
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3217-3217 -- 同上
                fMain->cbDisableSiteMappingCheck->Visible=false;
#endif // GATE (S12-C save)
            }
        }

        if(IniConfig.bC08_SocketSensor && IniConfig.bD65EnableCheckSocketsensorFunction)
        {
            TestIF_File.bEnSocketSensor=true;
            WriteIniData(szDir, "Configuration", "SocketSensor", 1);
        }
    }

    #ifdef MTK_Version
    EL<TCheckBox>("TfSetup", "cbSocketSensor")      ->Checked   =true;
    EL<TCheckBox>("TfSetup", "cbSocketSensor")      ->Enabled   =false;
    #else
    EL<TCheckBox>("TfSetup", "cbSocketSensor")      ->Checked   = TestIF_File.bEnSocketSensor;              //kevin 20130504  使用SOCKET SENSOR
    EL<TCheckBox>("TfSetup", "cbSocketSensor")      ->Enabled   = (AccessLevel>=LevelSet.AccessLevel[159]); //Steven 20191129 : Socket sensor加入權限控制
    #endif
    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[15])
        {
            EL<TCheckBox>("TfSetup", "cbSocketSensor")->Enabled=false;
        }
    }

    EL<TCheckBox>("TfSetup", "cbdisibleinitialcheck")->Checked  = TestIF_File.bSocketDisibleinitialcheck;   //kevin 20170209 (wei) 不使用SOCKET SENSOR initial check
    EL<TCheckBox>("TfSetup", "cbNSKitPress")->Checked           = TestIF_File.bNSKitPress;                  //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值
        SU_CbSetIndex(EL<TComboBox>("TfSetup", "CoSocketCombo"), TestIF_File.iSocketCount-1);   // golden cSetUp.cpp:3245-3245 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3245-3245 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    EL<TComboBox>("TfSetup", "CoSocketCombo")       ->ItemIndex = TestIF_File.iSocketCount-1;               //kevin 20130504 使用多少顆SENSOR 最多8個
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfSetup", "cbIndSLK")->Checked               = TestIF_File.bIndEPSLK;                    //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值

    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200420 : Socket Sensor功能可以選
    {
        MyTempRGBox[i]->ItemIndex=TestIF_File.iSensorCheckType[i];
        if(i<TestIF_File.iSocketCount)
        {
            MyTempRGBox[i]->Visible=true;
            if(MyTempRGBox[i]->ItemIndex==0)
            {
                if(TestIF_File.bSocketSensorCheckFloating)
                    MyTempRGBox[i]->ItemIndex=2;
                else
                    MyTempRGBox[i]->ItemIndex=1;
            }
        }
        else
        {
            MyTempRGBox[i]->Visible=false;
            MyTempRGBox[i]->ItemIndex=0;
        }
    }

    if(CosFunction.bI21EnableASMByRecipe)                                       //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
        EL<TCheckBox>("TfSetup", "cbI21")->Checked=IniConfig.bI21EnableASM;

    EL<TCheckBox>("TfSetup", "cbOctal16Site")       ->Checked   =TestIF_File.bOctal_16Kit;                  //Steven 20140312 : 8Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "cbSquareOctalLayout") ->Checked   = TestIF_File.bSquare_OctalKit;             //Steven 20141224 : 2x2Site使用8Site Kit
    EL<TCheckBox>("TfSetup", "chk2x2Use16siteSLK")  ->Checked   =TestIF_File.b2x2Use16SiteKit;              //Steven 20191113 : 2x2Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "cb1x2Use1x4siteSLK")  ->Checked   = TestIF_File.b1x2Use1x4SiteKit;            //Isaac 20200311 : 1x2Site使用1x4Site Kit
    EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")     ->Checked   = TestIF_File.bUse1x3SiteKit;               //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK")  ->Checked   = TestIF_File.b2x6Use2x8SitSLK;             //Steven 20240807 : 12Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "cb16DirectHeater")    ->Checked   = TestIF_File.bUse32Heater;                 //Steven 20140923 : Index使用EJ1N版32組加熱器
    EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Checked = TestIF_File.b12SiteUse10Heater;           //Steven 20140923 : Index使用EJ1N版32組加熱器
    EL<TCheckBox>("TfSetup", "Arm1PickArm2Test")    ->Checked   = TestIF_File.bArm1PickPlaceArm2Test;       //kevin 20150127 Arm1 下壓 arm2 測試
    EL<TCheckBox>("TfSetup", "cbCheckArm2Vacuum")   ->Checked   = TestIF_File.bCheckArm2Vacuum;             //Steven 20150129 : 需要確認Arm2有沒有粘料
    EL<TCheckBox>("TfSetup", "cbUseSLKClamp")       ->Checked   = TestIF_File.bUseSLKClamp;                 //JerryYang 20160523
    EL<TRadioGroup>("TfSetup", "rgseparabilityTest")  ->ItemIndex = TestIF_File.iSeparabilityTest;            //JerryYang 20160602
    if(TestIF_File.bUseSLKClamp)                                                //JerryYang 20160714 bUseSLKClamp
        EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->Enabled=true;
    else
        EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->Enabled=false;
    EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater")->Checked=TestIF_File.bSingleHeater;                //JerryYang 20161013
    EL<TCheckBox>("TfSetup", "cbOctal12Site")->Checked= TestIF_File.bOctal_12Kit;                           //ChungHung 20140508 add for SCK

    EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Checked=TestIF_File.bHontechLayoutKit2x2;             //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.

    EL<TCheckBox>("TfSetup", "cbEnablePreciser")            ->Checked   = TestIF_File.bEnableUsePreciser;   //Frank 20180410 (Steven) : InArm Preciser Station
    EL<TCheckBox>("TfSetup", "cbEnabledPreciserRT")         ->Checked   = TestIF_File.bEnableRTPreciser;    //Frank 20180410 (Steven) : InArm Preciser Station
    if(SPIL_FOR_QLE==1)                                                         //Kevincheng 20260525 : 渠梁常溫模式使用hotplate
    {
        EL<TCheckBox>("TfSetup", "cbEnablePreciserHotPlate")    ->Checked   = TestIF_File.bEnablePreciserHotPlate;
    }
    else
    {
        EL<TCheckBox>("TfSetup", "cbEnablePreciserHotPlate")    ->Visible   =false;
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20200701 : VTEST for auto site mapping cable mount
    {
        EL<TCheckBox>("TfSetup", "chkSiteMapOpenSite")->Checked = TestIF_File.bAutoSiteMappingOpenSite;
        EL<TCheckBox>("TfSetup", "chkSiteMapOneCycle")->Checked = TestIF_File.bAutoSiteMappingOneCycle;
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635 ||                              //jou 2012-05-15 : 選擇 Y Pitch 機構模式
       USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)                                //Steven 20140819 : Y-Pitch 36mm
    {
        if(TestIF_File.iARM_Y_PITCH==6000)
            EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex=0;
        else
            EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex=iXPitchManual635;
    }

    EL<TCheckBox>("TfSetup", "cbF18")->Checked=TestIF_File.bF18InshuttleDetect;                             //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
    EL<TCheckBox>("TfSetup", "cbRTC20CheckFunction")->Checked=TestIF_File.bRTC20CheckFunction;              //Ifor 20190712 : add RTC2.0 Check
    EL<TCheckBox>("TfSetup", "cbRTC20GiveWayCheck")->Checked=TestIF_File.bRTC20GiveWayCheck;                //Ifor 20190712 : add RTC2.0 Check

    EL<TCheckBox>("TfSetup", "cbRTCICResidueCheck")->Checked=TestIF_File.bRTCICResidueCheck;                //Ifor 20190712 : add RTC2.0 Check
    EL<TCheckBox>("TfSetup", "cbRTC20OverFlowCheck")->Checked=TestIF_File.bRTC20OverFlowCheck;              //Ifor 20190712 : add RTC2.0 Check

    EL<TCheckBox>("TfSetup", "cbArm1UseHeat")->Checked=TestIF_File.bArm1UseHeat;                            //Ifor 20190815 : add Arm1是否加熱開關
    EL<TCheckBox>("TfSetup", "cbAutoCleanAlso")->Checked=TestIF_File.bArm1PickPlaceArm2Test_RunAutoClean;   //Jimmychiu 20230710 : Auto Clean 跑 Arm1 下壓 arm2 測試
    EL<TCheckBox>("TfSetup", "cbArm1OnlyPlaceArm2TestAndSuck")->Checked=TestIF_File.bArm1OnlyPlaceArm2TestAndSuck;//Jimmychiu 20250722 : Arm1 Only Place Arm2 Test And Suck

    if(CosFunction.b2x4SupportCenterPitch==false ||                             //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
       TestIF_File.iTestMode!=_8Site2X4)
    {
        EL<TCheckBox>("TfSetup", "chkUseXCenterPitch")->Checked=false;
    }
    else
    {
        EL<TCheckBox>("TfSetup", "chkUseXCenterPitch")->Checked=TestIF_File.bEnableUseXCenterPitch;
        EL<TEdit>("TfSetup", "edtXCenterPitch")->Text      =TestIF_File.dSiteXCenterPitch;
    }

    EL<TCheckBox>("TfSetup", "cbInUseBackRow")->Checked =TestIF_File.bInArmUseBackRowSuck;                  //jou 20161122 (Steven) In & Out arm use back row suck
    EL<TCheckBox>("TfSetup", "cbOutUseBackRow")->Checked=TestIF_File.bOutArmUseBackRowSuck;                 //jou 20161122 (Steven) In & Out arm use back row suck
    EL<TEdit>("TfSetup", "edtRTCFileName")->Text    =TestIF_File.sRtcFileName;                          //kevin 20200731 RTC FILENAME

    EL<TCheckBox>("TfSetup", "chkReadTorque")->Checked  =TestIF_File.bEnableReadAndCheckTorque;             //kevin 20210804
    EL<TEdit>("TfSetup", "edOverRange")->Text       =TestIF_File.dReadTorque;                           //kevin 20210804 扭力誤差值 change by setup
    EL<TEdit>("TfSetup", "edDelayTime")->Text       =TestIF_File.dReadTorqueDelayTime;                  //kevin 20210804 等待時間去讀取扭力 change by setup

    EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit")->Checked=TestIF_File.UseRotateForHT7000HPKit;     //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit

    EL<TCheckBox>("TfSetup", "bDoubleUnloadTray")->Checked  = TestIF_File.bDoubleUnloadTray;                //kevin 20220506 add Unload Tray 2  倍的設定

    EL<TEdit>("TfSetup", "edAuto1Count")->Text       =TestIF_File.iUnloaderTrayCount[0];                //kevin 20220506 add Unload Tray Total
    EL<TEdit>("TfSetup", "edAuto2Count")->Text       =TestIF_File.iUnloaderTrayCount[1];                //kevin 20220506 add Unload Tray Total
    EL<TEdit>("TfSetup", "edAuto3Count")->Text       =TestIF_File.iUnloaderTrayCount[2];                //kevin 20220506 add Unload Tray Total
    EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck") ->Checked =TestIF_File.bSingleUseOtherSuck;            //wei 20220905 Single Use Other Suck(IN C OUT E)
    EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck") ->Checked      =TestIF_File.bDualSiteUseOneSuck;      //wei 20251218 DualSite Use One Suck
    EL<TEdit>("TfSetup", "edtGetValueDelayTime")->Text=TestIF_File.dSendGetValueDelayTime;              //KenHsieh 20220317 : 延遲取值時間
    EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck") ->Checked =TestIF_File.bSingleInArmUseOtherSuck;  //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    EL<TCheckBox>("TfSetup", "cbPreventDropfunction")->Checked =TestIF_File.bPreventDropfunction;           //Frank 20260710 ADD//Eastsun 20260710整合

}

// golden cSetUp.cpp:232  TfSetup::ScrollBar1Change(TObject *Sender)
static void SU_ScrollBar1Change()
{

    static bool bFirstRead=true;
    EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=true;                                                //kevin 20210813 add close arm display
    if(CosFunction.bDisableOpenAllSiteWhenChangeShtMod)                         //Jimmychiu 20241203 : Disable opening all sites when changing shuttle mode.
    {
        shtMode.Clear();
    }

        if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position!=TestIF_File.iTestMode &&   // golden cSetUp.cpp:241-241 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:241-241 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    if(EL<TControl>("TfSetup", "ScrollBar1")->Position!=TestIF_File.iTestMode &&
#endif // GATE (S12-C save)
       Barcode_Reader(bcSetup)==0)                                              // 20140103 wei KYEC Barcode Reader 20140314
    {
            filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position=TestIF_File.iTestMode;   // golden cSetUp.cpp:244-244 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:244-244 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        EL<TControl>("TfSetup", "ScrollBar1")->Position=TestIF_File.iTestMode;
#endif // GATE (S12-C save)
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                    SU_CbSetIndex(TestSiteCH[i][j], TestIF_File.iSiteMap[i][j]);   // golden cSetUp.cpp:249-249 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:249-249 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                TestSiteCH[i][j]->ItemIndex=TestIF_File.iSiteMap[i][j];
#endif // GATE (S12-C save)
            }
        }
        return;
    }

    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true ||                                     //Steven 20140922 : Arm2當作指紋測試
       (TestIF_File.bArm1PickPlaceArm2Test &&                                   //kevin 20150127 Arm1 下壓 arm2 測試
        IniConfig.bD58UseArm1PickPlaceArm2Test==true))                          //Ifor 20191122 : 避免D58功能關閉無法開ARM問題
    {
        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;                                           //only use shuttle 1
    }
    else
    {
        EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=IniConfig.bD30EnableSiteModeSelect;
    }

    if(bGPIBChangeSiteMap)                                                      //wei 20151127 GPIB Change Site Map
    {
            filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position=TestIF_File.iTestMode;   // golden cSetUp.cpp:269-269 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:269-269 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        EL<TControl>("TfSetup", "ScrollBar1")->Position=TestIF_File.iTestMode;
#endif // GATE (S12-C save)
        for(int i=0; i<MAX_SOCKET_ROW; i++)
            for(int j=0; j<MAX_SOCKET_COL; j++)
                    SU_CbSetIndex(TestSiteCH[i][j], TestIF.iSiteMap[i][j]);   // golden cSetUp.cpp:272-272 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:272-272 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                TestSiteCH[i][j]->ItemIndex=TestIF.iSiteMap[i][j];
#endif // GATE (S12-C save)
    }
        int iPos=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position;   // golden cSetUp.cpp:274-274 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:274-274 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    int iPos=EL<TControl>("TfSetup", "ScrollBar1")->Position;
#endif // GATE (S12-C save)
    int iCheckPos;                                                              //wei 20150314 ScrollBar1->Position改變時，會有問題。  iPos-->iCheckPos

    int iMax;

    bool bShow32Site=(Sen[SnNegativePressureAir2].Enable==true);
#ifdef SOFT_SIMULTE
    bShow32Site=true;
#endif
//    if(CUSTOMER_CODE!=CC_HONPREC_QC  &&
//       USE_ROTATE_KIT==1             &&
//       iRotate_Type==eInOutArm1Motor &&
//       USE_PICKER_COUNT==ep1Picker)                                             //Frank 20250812 : modify InOutArm1Motor
//    {
//        iMax=SingleSite;
//    }
//    else
    if(MachineTypeChoice==Type_HT9045)
    {
        iMax=_8Site2X4;
    }
    else if((MachineTypeChoice==Type_HT9046_LS ||
             MachineTypeChoice==Type_HT1032) &&
             bShow32Site==true)                                                 //Ifor 20171226 (Steven) add 需安裝兩顆才可以跑32Site
    {
        iMax=_32Site4X8N;
    }
    else if(MachineTypeChoice==Type_HT9045_12Site)                              //ChungHung 20130507 add HT9045 updata for 12site 517
    {
        iMax=_12Site2X6;
    }
    else
    {
        iMax=_16Site2X8;
    }

    if(USE_PICKER_COUNT==0 && bHT9045S_USE2x4==false)                           //Ifor 20170308 (wei) add 鎖住 HT9045S 僅可跑2*2
    {
        iMax=QualSite2X2;
    }

    if(iPos>=iMax)
    {
            filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position=iMax;   // golden cSetUp.cpp:317-317 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:317-317 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        EL<TControl>("TfSetup", "ScrollBar1")->Position=iMax;
#endif // GATE (S12-C save)
        OrgTestMode=iPos;
    }

    if(((!(MachineTypeChoice==Type_HT9046_LS ||
           MachineTypeChoice==Type_HT1032) ||                                   //ChungHung 20150528 add
        (MachineTypeChoice==Type_HT9046_LS && SubMachineType==Type_HT9016C) ||  //Jimmychiu 20230207 : HT9016C不可以跑NN Mode
        (MachineTypeChoice==Type_HT9046_LS && SubMachineType==Type_HT9046LA)) &&//Steven 20220425 : 9046LA不可以跑NN Mode
        (iPos==_32Site4X8N  ||
         iPos==_32Site4X8M  ||
         iPos==_16Site4X4   ||
         iPos==QualSite2X2N ||
         iPos==_6Site2X3N   ||
         iPos==_8Site2X4N)) ||                                                  //Steven 20220425 : 2x3 NN mode
       (MachineTypeChoice==Type_HT9045 && iPos>=_10Site2X5)         ||
       (MachineTypeChoice==Type_HT9045_12Site && iPos>=_16Site2X8)  ||
       (iPos==DualSite2x1  && CosFunction.bEnable2x1Site==false)    ||
       (iPos==_12Site2X6   && CosFunction.bEnable12Site==false)     ||
       (iPos==_6Site2X3    && CosFunction.bEnable6Site==false)      ||          //ChungHung 20140115 add for 2x3_6
       (iPos==_6Site2X3    && USE_PICKER_COUNT==0)                  ||          //Steven 20161117 : for HT-9045S
       (iPos==TriSite1X3   && USE_PICKER_COUNT==0)                  ||          //Steven 20161117 : for HT-9045S
       (iPos==QualSite2X2N && CosFunction.bCanUse2x2NNMode==false)  ||          //Steven 20200722 : 2x2 NN mode
       (iPos==_6Site2X3N   && CosFunction.bCanUse2x3NNMode==false)  ||          //Steven 20220425 : 2x3 NN mode
       (iPos==_8Site2X4N   && CosFunction.bCanUse2x4NNMode==false))             //Wei 20231211 : 2X4NN Mode
    {
        if(OrgTestMode<iPos)
            iPos++;
        else
            iPos--;

        if(iPos<0)                                                              //Steven 20120816 : 修正小於1的Error
        {
            iPos=1;
        }
            filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position=iPos;   // golden cSetUp.cpp:351-351 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:351-351 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        EL<TControl>("TfSetup", "ScrollBar1")->Position=iPos;
#endif // GATE (S12-C save)
        OrgTestMode=iPos;
        bGPIBChangeSiteMap=false;                                               //JerryYang 20150105 GPIB Change Site Map
        return;                                                                 //ChungHung 20150528 modify ScrollBar1->Position改變時，會在觸發一次事件。
    }

        iCheckPos=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position;   // golden cSetUp.cpp:357-357 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:357-357 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    iCheckPos=EL<TControl>("TfSetup", "ScrollBar1")->Position;                                             //避免ScrollBar1->Position改變時，會有問題。
#endif // GATE (S12-C save)
    OrgTestMode=iCheckPos;                                                      //ChungHung 20150528 modify ScrollBar1->Position改變時，會在觸發一次事件。

        SU_chkOffCenterkitClick();   // golden cSetUp.cpp:360-360 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:360-360 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    SU_chkOffCenterkitClick(this);
#endif // GATE (S12-C save)
    EL<TPanel>("TfSetup", "Panel1")->Caption=TestSiteFileName[0][iCheckPos];                             //wei 20150314 ScrollBar1->Position改變時，會有問題。  iPos-->iCheckPos
    SU_CompChange(iCheckPos);                                                      //wei 20150314 ScrollBar1->Position改變時，會有問題。  iPos-->iCheckPos

    if(bFirstRead==true)                                                        //Ifor 20161117 add 取得程式開啟時 Handler Mode
    {
        asHandlingMode=EL<TPanel>("TfSetup", "Panel1")->Caption;                                         //Ifor 20161117 add Handler Mode Backup
        bFirstRead=false;
    }

    EL<TComboBox>("TfSetup", "cbAa")->Enabled=true;
    EL<TEdit>("TfSetup", "XPitch")                  ->Visible=true;                                     //Steven 20120604 : XPitch 顯示異常
    EL<TLabel>("TfSetup", "lblXPitch")               ->Visible=true;
    EL<TLabel>("TfSetup", "lblXPMM")                 ->Visible=true;
    EL<TCheckBox>("TfSetup", "cbNS8000H")               ->Visible=false;
    EL<TCheckBox>("TfSetup", "chkOctal80")              ->Visible=false;
    EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")       ->Visible=false;                                    //Steven 20150724 : 16溫控器 2條線版本
    EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")       ->Visible=false;                                    //jou 2015-10-15  : 16溫控器 1條線版本
    EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")       ->Visible=false;                                    //Steven 20230613 : ATC6.1 16Site SLK
    EL<TCheckBox>("TfSetup", "cbOctal16Site")           ->Visible=false;                                    //Steven 20140312 : 8Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK")      ->Visible=false;                                    //Steven 20240807 : 12Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "cbSquareOctalLayout")     ->Visible=false;                                    //Steven 20141224 : 2x2Site使用8Site Kit
    EL<TCheckBox>("TfSetup", "chk2x2Use16siteSLK")      ->Visible=false;                                    //Steven 20191113 : 2x2Site使用16Site Kit
    EL<TCheckBox>("TfSetup", "cb1x2Use1x4siteSLK")      ->Visible=false;                                    //Isaac 20200311 : 1x2Site使用1x4Site Kit
    EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")         ->Visible=false;                                    //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    EL<TCheckBox>("TfSetup", "cbOctal12Site")           ->Visible=false;                                    //ChungHung 20140508 add for SCK
    EL<TLabel>("TfSetup", "labYOffset")              ->Visible=false;                                    //Steven 20140512 : For HT-9047
    EL<TEdit>("TfSetup", "edYOffset")               ->Visible=false;                                    //Steven 20140512 : For HT-9047
    EL<TRadioGroup>("TfSetup", "rgYOffset")               ->Visible=false;                                    //Steven 20200715 : for Tinton
    EL<TCheckBox>("TfSetup", "cb16DirectHeater")        ->Visible=false;                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
    EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")  ->Visible=false;                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
    EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")    ->Visible=false;                                    //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
    EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")      ->Visible=false;                                    //wei 20160226 TSMC X Shift
    EL<TPanel>("TfSetup", "paQualSite2X2Shift")      ->Visible=false;                                    //wei 20160226 TSMC X Shift
    EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater")->Visible=false;                                    //JerryYang 20161013 新增Single site一支加熱棒模式
    EL<TGroupBox>("TfSetup", "gbInUseBackRow")          ->Visible=false;                                    //jou 20161122 (Steven) In & Out arm use back row suck
    EL<TGroupBox>("TfSetup", "gbOutUseBackRow")         ->Visible=false;                                    //jou 20161122 (Steven) In & Out arm use back row suck
    EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")      ->Checked=false;                                    //kevin 20170513 (wei) add
    EL<TGroupBox>("TfSetup", "grpUseXCenterPitch")->Visible=(CosFunction.b2x4SupportCenterPitch==true &&
                                 iCheckPos==_8Site2X4);                         //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    bUseTwoArm32Site=false;                                                     //kevin 20190322 add two arm tester 32SITE 16SITE(4X4)
    EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Visible=false;                                    //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Checked=false;                                    //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    EL<TCheckBox>("TfSetup", "cb16change8DirectHeater") ->Visible=false;                                    //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
    EL<TCheckBox>("TfSetup", "cb16change8DirectHeater") ->Checked=false;                                    //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
    EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")      ->Visible=false;                                    //Steven 20201013 : NN mode add Y-Pitch offset selection
    EL<TCheckBox>("TfSetup", "cbIndSLK")                ->Visible=false;
    EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck")    ->Visible=false;                                    //wei 20220905 Single Use Other Suck(IN C OUT E)
    EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck")   ->Visible=false;                                    //wei 20251218 DualSite Use One Suck
    EL<TCheckBox>("TfSetup", "chkOffCenterkit")         ->Checked=false;                                    //kevin 20201211 add for ASE KH
    EL<TCheckBox>("TfSetup", "chkOffCenterkit")         ->Visible=false;
    EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit")->Visible=false;                                   //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
    EL<TPanel>("TfSetup", "palVisibleIndex")         ->Visible=false;                                    //KenHsieh 20231227 : add Y Center Pitch mode
    EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck")->Visible=false;                                   //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    EL<TCheckBox>("TfSetup", "cbPreventDropfunction")   ->Visible=false;                                    //Frank 20260710 ADD//Eastsun 20260710整合
    switch(iCheckPos)                                                           //wei 20150314 ScrollBar1->Position改變時，會有問題。  iPos-->iCheckPos
    {
        case SingleSite:                                                        //1x1
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TComboBox>("TfSetup", "cbAa")->Enabled=false;
            filerw::ELComboIndex(EL<TComboBox>("TfSetup", "cbAa"), 1);
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;
            EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=CosFunction.bCanUseBias;                   //ChungHung 20140519 add Singe Site use NS7000 kit  //Steven 20190801 : SCC李國旗說要取消BIAS選項

            if(USE_16_HEATER==eht16Heater       ||                              //Steven 20120606 : 16溫控器 8Site使用Hontech頭
               USE_16_HEATER==eht16HeaterEJ1N   ||
               USE_16_HEATER==eht32HeaterEJ1N   ||                              //Steven 20140923 : Index使用EJ1N版32組加熱器
               USE_16_HEATER==eht32HeaterKT4H   ||                              //Steven 20150211 : Index使用KT4H版32組加熱器
               USE_16_HEATER==eht16HeaterDTME08 ||                              //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
               USE_16_HEATER==eht32HeaterDTME08 )                               //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            {
                if(CUSTOMER_CODE!=CC_SCS)                                       //jou 20170518 (Steven) : SCS 要求沒用到不顯示
                    EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Visible=true;                            //JerryYang 20160826 Single site兩條線版本
            }
            else if(USE_16_HEATER==eht4Heater)
            {
                EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater")->Visible=true;                         //JerryYang 20161013 新增Single site一支加熱棒模式
            }
            EL<TLabel>("TfSetup", "lblXPitch")->Visible=false;
            EL<TLabel>("TfSetup", "lblXPMM")->Visible=false;
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch") ->Visible=false;
            EL<TEdit>("TfSetup", "XPitch") ->Visible=false;
            EL<TEdit>("TfSetup", "XPitch") ->Text   ="1";
            EL<TEdit>("TfSetup", "YPitch") ->Text   ="0";
            iSiteTotal=1;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck")    ->Visible=true;                             //wei 20220905 Single Use Other Suck(IN C OUT E)
            EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck")->Visible=true;                            //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
            EL<TCheckBox>("TfSetup", "cbPreventDropfunction")   ->Visible=true;                             //Frank 20260710 ADD//Eastsun 20260710整合
            break;
        case DualSite:                                                          //1x2
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 2 pick unit";
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 4 pick unit";
            if(EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==1)
            {
                if(IniConfig.bDisableSelectSearchLast==true)                    //jou 2012-01-10 取消Setup，Search Last Mode功能。
                {
                    EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
                }
                else
                {
                    EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
                }
            }

            if(USE_16_HEATER==eht16Heater       ||                              //Steven 20120606 : 16溫控器 8Site使用Hontech頭
               USE_16_HEATER==eht16HeaterEJ1N   ||
               USE_16_HEATER==eht32HeaterEJ1N   ||                              //Steven 20140923 : Index使用EJ1N版32組加熱器
               USE_16_HEATER==eht32HeaterKT4H   ||                              //Steven 20150211 : Index使用KT4H版32組加熱器
               USE_16_HEATER==eht16HeaterDTME08 ||                              //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
               USE_16_HEATER==eht32HeaterDTME08 )                               //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            {
                EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Visible=true;                                //JerryYang 20160826 Dual site兩條線版本
                if(IniConfig.bL30Use1CableLayoutKitByConfig==false)
                    EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")->Visible=true;
            }
            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                         //Steven 20100119
            EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=CosFunction.bCanUseBias;                   //Steven 20190801 : SCC李國旗說要取消BIAS選項

            if(CosFunction.bEnable_1x3Kit)                                      //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
            {
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Visible=true;
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Caption="Dual Site use 1x3 SLK";
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Visible=CosFunction.bEnable_1x3Kit;
            }

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20170415 (wei) KaohSiung add 2x2 Shift
            {
                EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")->Visible=true;
                EL<TPanel>("TfSetup", "paQualSite2X2Shift")->Visible=EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")->Checked;        //kevin 20170513 X Shift
            }

            if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                    //AI(ht9045-v899) 20260430: extend cbIndSLK visibility to Multi EP half.
            {
                EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=true;
            }
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;
            EL<TEdit>("TfSetup", "XPitch")->Text=80;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text="0";                                                   //Steven 20100827
            iSiteTotal=2;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            EL<TCheckBox>("TfSetup", "cb1x2Use1x4siteSLK")->Visible=CosFunction.bEnableDual_1x4Kit;         //Isaac 20200311 : 1x2Site使用1x4Site Kit
            EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit")->Visible=(CosFunction.bRotateUseHT7000HPKit &&
                                                USE_ROTATE_KIT);                //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
            if(DeviceForm.XDimension>=7000 && DeviceForm.YDimension>=7000)      //wei 20251218 DualSite Use One Suck
                EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck")->Visible=true;
            break;
        case DualSite2x1:                                                       //2x1
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;                                          //Steven 20140312 : For HT9045WA

            EL<TLabel>("TfSetup", "lblXPitch")->Visible=false;
            EL<TLabel>("TfSetup", "lblXPMM")->Visible=false;
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch") ->Visible=true;
            EL<TEdit>("TfSetup", "XPitch") ->Visible=false;
            EL<TEdit>("TfSetup", "XPitch") ->Text   ="0";
            EL<TEdit>("TfSetup", "YPitch") ->Text   ="";
            iSiteTotal=2;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            break;
        case TriSite1X3:                                                        //Frank 20160329 add for 1x3_4
            EL<TCheckBox>("TfSetup", "Arm1PickArm2Test")->Checked=false;                                    //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=false;

            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            if(ATC_SYSTEM>eATC30)                                               //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
            {
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;
            }
            else
            {
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                     //Steven 20100119
            }

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;
            EL<TEdit>("TfSetup", "XPitch")->Text="40";                                                  //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text="0";                                                   //Steven 20100827
            iSiteTotal=3;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            break;
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TCheckBox>("TfSetup", "Arm1PickArm2Test")->Checked=false;                                    //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=false;

            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            if(ATC_SYSTEM>eATC30)                                               //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
            {
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;
            }
            else
            {
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                     //Steven 20100119
            }

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text=60;                                                    //Steven 20100827
            iSiteTotal=4;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            break;
        case QualSite1X4:                                                       //1x4
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4

            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            if(USE_PICKER_COUNT!=0)                                             //Steven 20161117 : for HT-9045S
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 4 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 8 pick unit";
                if(EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==1)
                {
                    if(IniConfig.bDisableSelectSearchLast==true)                //jou 2012-01-10 取消Setup，Search Last Mode功能。
                    {
                        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
                    }
                    else
                    {
                        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
                    }
                }
            }

//            if(IniConfig.bKoreaFunction==false &&                             //Steven 20240401 : Mark 1x4 SLK 2 Connector for Korea
            if(CUSTOMER_CODE!=CC_SCS)                                           //jou 2015-08-23 SCS remove Use 2 cable layout kit in Tool>setup
            {
                if(USE_16_HEATER==eht16Heater       ||                          //Steven 20120606 : 16溫控器 8Site使用Hontech頭
                   USE_16_HEATER==eht16HeaterEJ1N   ||
                   USE_16_HEATER==eht32HeaterEJ1N   ||                          //Steven 20140923 : Index使用EJ1N版32組加熱器
                   USE_16_HEATER==eht32HeaterKT4H   ||                          //Steven 20150211 : Index使用KT4H版32組加熱器
                   USE_16_HEATER==eht16HeaterDTME08 ||                          //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                   USE_16_HEATER==eht32HeaterDTME08 )                           //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
                {
                    EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Visible=true;                            //Steven 20150724 : 16溫控器 2條線版本
                    if(IniConfig.bL30Use1CableLayoutKitByConfig==false)         //Steven 20141022 : 1x4 Layout Kit with 2 Connector
                        EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")->Visible=true;                        //jou 2015-10-15  : 16溫控器 1條線版本
                }
            }

            if(CosFunction.bInOutArmUseBackRowSuck==true)                       //jou 20161122 (Steven) In & Out arm use back row suck
            {
                EL<TGroupBox>("TfSetup", "gbInUseBackRow")->Visible=true;
                EL<TGroupBox>("TfSetup", "gbOutUseBackRow")->Visible=true;
            }

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;                                          //kevin 20140829 使用偏心氣孔

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            {
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=true;
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked=true;                                  //kevin 20200924 add for ASE KH
            }
            else
            {
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=CosFunction.bCanUseBias;               //Steven 20190801 : SCC李國旗說要取消BIAS選項
            }

            if(CosFunction.bNonCenterModeCanUseShtOffset &&
               IniConfig.bA50Enable1x4BiasYOffset &&
               EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked)                                        //Steven 20200715 : for Tinton
            {
                EL<TRadioGroup>("TfSetup", "rgYOffset")->Visible=true;
                EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex=0;
            }

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text="0";                                                   //Steven 20100827
            iSiteTotal=4;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
            {
                EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=true;
            }
            //<== //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end
            break;
        case QualSite2X2N:                                                      //Frank 20200520 2X2NN Mode
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }

            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            if(CUSTOMER_CODE==CC_SCS)
            {
                EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Visible=true;                             //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
            }
            else
            {
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=CosFunction.bCanUseBias;               //Steven 20190801 : SCC李國旗說要取消BIAS選項
            }

            if(CosFunction.bEnable_1x3Kit)                                      //KevinC 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
            {
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Visible=true;
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Caption="2x2 NN Mode use 1x3 SLK";
                EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Visible=CosFunction.bEnable_1x3Kit;
            }

            EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex=0;                                    //KenHsieh 20231227 : add Y Center Pitch mode
            EL<TLabel>("TfSetup", "labYOffset")   ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")    ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")->Text=80;                                                 //Steven 20140512 : For HT-9047 //jou 2016-05-06 32 Sites Y Offset 統一填入LoaderBoard尺寸 10 -> 80
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;

            EL<TEdit>("TfSetup", "XPitch")->Text=80;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text="0";                                                   //Steven 20100827
            iSiteTotal=4;                                                       //kevin 20160125 SITE 數設定
            //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
            {
                EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=true;
            }
            //<== //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end
            break;
        case QualSite2X2: //2x2
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }

            if(CUSTOMER_CODE!=CC_SCS)                                           //jou 20170518 (Steven) : SCS 要求沒用到不顯示
            {
                EL<TCheckBox>("TfSetup", "cbSquareOctalLayout")->Visible=true;                              //Steven 20141224 : 2x2Site使用8Site Kit
                if(CUSTOMER_CODE==CC_HANA_MICRON)
                    EL<TCheckBox>("TfSetup", "chk2x2Use16siteSLK")->Visible=true;                           //Steven 20191113 : 2x2Site使用16Site Kit
            }

            if(USE_PICKER_COUNT!=0)                                             //Steven 20161117 : for HT-9045S
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 4 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 8 pick unit";
            }

            if(IniConfig.bL30Use1CableLayoutKitByConfig==false)
                EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")->Visible=true;                                //JerryYang 20181207 2x2新增一條線版本

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;                                          //Steven 20100119

            if(CUSTOMER_CODE==CC_TSMC_TAINAN ||
               CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20170415 (wei) KaohSiung add 2x2 Shift
            {
                EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")->Visible=true;                               //wei 20160226 TSMC X Shift
                EL<TPanel>("TfSetup", "paQualSite2X2Shift")->Visible=(TestIF_File.bQualSite2X2Shift);    //wei 20160226 TSMC X Shift
            }

            if(CosFunction.bCanUse2x2Bias)                                      //Steven 20190724 : 2x2偏心開關
            {
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=true;                                  //Steven 20190801 : SCC李國旗說要取消BIAS選項
            }
            else                                                                //jou 2015-08-23 SCS remove 2x2 NS7000 bias kit in Tool>setup
            {
                EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Visible=false;
            }

            if(CUSTOMER_CODE==CC_SCS)
            {
                EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Visible=true;                             //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
            }

            if(CUSTOMER_CODE==CC_ATEC && DeviceForm_File.iHeadDeviceCT==3)      //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
            {
                EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Visible=true;
            }
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;

            EL<TEdit>("TfSetup", "XPitch")->Text=80;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text=60;                                                    //Steven 20100827
            iSiteTotal=4;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
            {
                EL<TCheckBox>("TfSetup", "cbIndSLK")->Visible=true;
            }
            //<== //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                         //Steven 20100119
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text=60;                                                    //Steven 20100827
            iSiteTotal=6;                                                       //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            break;
        case _6Site2X3N:                                                        //Steven 20220425 : 2x3 NN mode
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=false;                                  //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;

            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;
            EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Visible=false;

            EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex=0;                                    //KenHsieh 20231227 : add Y Center Pitch mode
            EL<TLabel>("TfSetup", "labYOffset")   ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")    ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")->Text=80;                                                 //Steven 20140512 : For HT-9047 //jou 2016-05-06 32 Sites Y Offset 統一填入LoaderBoard尺寸 10 -> 80
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;

            EL<TEdit>("TfSetup", "XPitch")->Text=120;
            EL<TEdit>("TfSetup", "YPitch")->Text="0";
            iSiteTotal=6;                                                       //kevin 20160125 SITE 數設定
            break;
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=false;                                  //ChungHung 20150528 add for 海思 _8Site1x4
            EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
            if(USE_PICKER_COUNT!=0)                                             //Steven 20161117 : for HT-9045S
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 4 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 8 pick unit";
                if(EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==1)
                {
                    if(IniConfig.bDisableSelectSearchLast==true)                //jou 2012-01-10 取消Setup，Search Last Mode功能。
                    {
                        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
                    }
                    else
                    {
                        EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
                    }
                }
            }

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;
            EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Visible=false;

            EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex=0;                                    //KenHsieh 20231227 : add Y Center Pitch mode
            EL<TLabel>("TfSetup", "labYOffset")   ->Visible=false;                                       //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")    ->Visible=false;                                       //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")->Text=80;                                                 //Steven 20140512 : For HT-9047 //jou 2016-05-06 32 Sites Y Offset 統一填入LoaderBoard尺寸 10 -> 80
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=false;
            EL<TEdit>("TfSetup", "YPitch")->Visible=false;

            EL<TEdit>("TfSetup", "XPitch")->Text=80;
            EL<TEdit>("TfSetup", "YPitch")->Text="0";
            iSiteTotal=8;                                                       //kevin 20160125 SITE 數設定
            break;
        case _8Site2X4:                                                         //2x4
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //Steven 20231207 : 2x4 16 picker
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 8 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 16 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;
            }

            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;
            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                         //Steven 20100119

            if(CUSTOMER_CODE==CC_ATEC && DeviceForm_File.iHeadDeviceCT==4)      //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
            {
                EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Visible=true;
            }

            if(IniConfig.bL30Use1CableLayoutKitByConfig==false)                 //Sam 20210715 : Use 1CableLayoutKit By Config //Sam 20210524 2x4 新增一條線版本
                EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")->Visible=true;

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text=60;                                                    //Steven 20100827
            iSiteTotal=8;                                                       //kevin 20160125 SITE 數設定

            if(CUSTOMER_CODE!=CC_AMKOR_China &&
               CUSTOMER_CODE!=CC_QUALCOMM &&                                    //JerryYang 20170412 (Steven) add QUALCOMM
               IniConfig.bKoreaFunction==false)                                 //ChungHung 20121031 add
            {
                if(USE_16_HEATER==eht16Heater       ||                          //Steven 20120606 : 16溫控器 8Site使用Hontech頭
                   USE_16_HEATER==eht16HeaterEJ1N   ||
                   USE_16_HEATER==eht32HeaterEJ1N   ||                          //Steven 20140923 : Index使用EJ1N版32組加熱器
                   USE_16_HEATER==eht32HeaterKT4H   ||                          //Steven 20150211 : Index使用KT4H版32組加熱器
                   USE_16_HEATER==eht16HeaterDTME08 ||                          //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                   USE_16_HEATER==eht32HeaterDTME08 )                           //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
                {
                    EL<TCheckBox>("TfSetup", "cbNS8000H")->Visible=true;
                }
            }

            if(MachineTypeChoice==Type_HT9046 ||
               MachineTypeChoice==Type_HT9046_LS)                               //2013-03-05    Dell    for ATK pitch 80mm kit
            {
                if(CUSTOMER_CODE==CC_SCK)                                       //2013-03-05    Dell    for ATK pitch 80mm kit
                {
                    EL<TCheckBox>("TfSetup", "chkOctal80")->Visible=true;
                }
                EL<TCheckBox>("TfSetup", "cbOctal12Site")->Visible=CosFunction.bEnableOctal_12Kit;          //ChungHung 20140508 add for SCK
                EL<TCheckBox>("TfSetup", "cbOctal16Site")->Caption="Octal site use 16 Site Layout Kit";
                EL<TCheckBox>("TfSetup", "cbOctal16Site")->Visible=CosFunction.bEnableOctal_16Kit;          //Steven 20140312 : 8Site使用16Site Kit
            }

            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            break;
        case _16Site2X8:                                                        //2x8
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                         //Steven 20100119
            EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Visible=true;                             //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
            EL<TCheckBox>("TfSetup", "cb16change8DirectHeater") ->Visible=true;                             //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=30;                                                    //Steven 20100827
            EL<TEdit>("TfSetup", "YPitch")->Text=60;                                                    //Steven 20100827
            iSiteTotal=16;                                                      //kevin 20160125 SITE 數設定
            EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")->Visible=(MachineTypeChoice==Type_HT1032);        //Steven 20230613 : ATC6.1 16Site SLK
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            break;
         case _12Site2X6:                                                       //2x6  //ChungHung 20130507 add HT9045 updata for 12site 517
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                //Ifor 20170420 (wei) add SIGURD PeiXing Hisi 2x6 常溫 無感溫線
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;                                      //Steven 20100119
            else
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                     //Steven 20100119

            EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK")->Visible=(CosFunction.bEnable12SiteUse16SLK);    //Steven 20240807 : 12Site使用16Site Kit
            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;
            EL<TEdit>("TfSetup", "YPitch")->Text=63.5;
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            {
                EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Visible=false;                          //kevin 20161102
            }
            else
            {
                EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N ||    //Steven 20140923 : Index使用EJ1N版32組加熱器
                                                 USE_16_HEATER==eht32HeaterKT4H ||    //Steven 20150211 : Index使用KT4H版32組加熱器
                                                 USE_16_HEATER==eht32HeaterDTME08);   //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
            }
            iSiteTotal=12;                                                      //kevin 20160125 SITE 數設定
            break;
         case _10Site2X5:                                                       //2x5   //wei 20190614 10 site
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;

            if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                //Ifor 20170420 (wei) add SIGURD PeiXing Hisi 2x6 常溫 無感溫線
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=true;                                      //Steven 20100119
            else
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                     //Steven 20100119

            EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")->Visible=(MachineTypeChoice==Type_HT1032);        //Steven 20230613 : ATC6.1 16Site SLK

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=60;
            EL<TEdit>("TfSetup", "YPitch")->Text=60;

            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N ||        //Steven 20140923 : Index use EJ1N 32 heater
                                       USE_16_HEATER==eht32HeaterKT4H ||        //Steven 20150211 : Index use KT4H 32 heater
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater
                                       USE_16_HEATER==eht32HeaterDTME08);       //AI(ht9045-heater-control) 20260812 (RogerYang) : 10-Site missed in JimmyChiu 20210923 DTME08 sweep

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            {
                EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Visible=false;                          //kevin 20161102
            }
            else
            {
                EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N ||    //Steven 20140923 : Index use EJ1N 32 heater
                                                 USE_16_HEATER==eht32HeaterKT4H ||    //Steven 20150211 : Index use KT4H 32 heater
                                                 USE_16_HEATER==eht32HeaterDTME08);   //AI(ht9045-heater-control) 20260812 (RogerYang) : 10-Site add DTME08 32 heater, align 12-Site
            }
            iSiteTotal=10;                                                      //kevin 20160125 SITE 數設定
            break;
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //Steven 20231207 : 2x4 16 picker
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[0]="Use 8 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Items->Strings[1]="Use 16 pick unit";
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=true;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;
            }
            EL<TCheckBox>("TfSetup", "cbOctal16Site")->Caption="4x4 site use 4x8 site layout kit";
            EL<TCheckBox>("TfSetup", "cbOctal16Site")->Visible=CosFunction.bEnableOctal_16Kit;              //Steven 20140312 : 8Site使用16Site Kit

        case _32Site4X8N:                                                       //4x8
            EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible=IniConfig.bD58UseArm1PickPlaceArm2Test; //ChungHung 20150528 add for 海思 _8Site1x4
            if(IniConfig.bDisableSelectSearchLast==true)                        //jou 2012-01-10 取消Setup，Search Last Mode功能。
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=false;
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex=0;
            }
            else
            {
                EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->Visible=true;
            }

            if(iCheckPos==_32Site4X8N)                                          //Steven 20231207 : 2x4 16 picker
            {
                EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;
            }

            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Visible=false;                                         //Steven 20100119
            EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")->Visible=(MachineTypeChoice==Type_HT1032);        //Steven 20230613 : ATC6.1 16Site SLK

            EL<TLabel>("TfSetup", "lblYPitch")->Visible=true;
            EL<TEdit>("TfSetup", "YPitch")->Visible=true;
            EL<TEdit>("TfSetup", "XPitch")->Text=40;
            EL<TEdit>("TfSetup", "YPitch")->Text=60;
            EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->Visible=true;                                   //KenHsieh 20231227 : add Y Center Pitch mode
                SU_rgYPitchOffsetModeClick();   // golden cSetUp.cpp:1129-1129 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1129-1129 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->OnClick(this);
#endif // GATE (S12-C save)
            EL<TLabel>("TfSetup", "labYOffset")   ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")    ->Visible=true;                                        //Steven 20140512 : For HT-9047
            EL<TEdit>("TfSetup", "edYOffset")->Text=80;                                                 //Steven 20140512 : For HT-9047 //jou 2016-05-06 32 Sites Y Offset 統一填入LoaderBoard尺寸 10 -> 80
            EL<TGroupBox>("TfSetup", "gbShuttleMode")->Visible=false;                                       //Steven 20140512 : For HT-9047
            EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Visible=(USE_16_HEATER==eht32HeaterEJ1N  ||       //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器
                                       USE_16_HEATER==eht32HeaterKT4H  ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      (ATC_SYSTEM==eNewATCSystem &&
                                       iATC_Use_Heat_Count>=16) ||              //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
                                       USE_16_HEATER==eht32HeaterDTME08);       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

            if(iCheckPos==_16Site4X4)                                           //Sam 20190226 : 16Site4X4
            {
                iSiteTotal=16;
            }
            else
            {
                iSiteTotal=32;                                                  //kevin 20160125 SITE 數設定
                if(CosFunction.b32SiteYOffsetMode)                              //Steven 20201013 : NN mode add Y-Pitch offset selection
                    EL<TPanel>("TfSetup", "palVisibleIndex")->Visible=true;                              //KenHsieh 20231227 : add Y Center Pitch mode
            }
            break;
    }

    if(iCheckPos==QualSite2X2N ||
       iCheckPos==_6Site2X3N   ||
       iCheckPos==_8Site2X4N   ||                                               //Wei 20231211 : 2X4NN Mode
       iCheckPos==_16Site4X4   ||
       iCheckPos==_32Site4X8N)
    {
        bUseTwoArm32Site=true;                                                  //kevin 20190322 add two arm tester 32SITE 16SITE(4X4)
    }
    else
    {
        bUseTwoArm32Site=false;
    }

    if(bGPIBChangeSiteMap)                                                      //JerryYang 20160113 由GPIB改setup 若YPitch為0則使用預設值
    {
        if(iCheckPos==_8Site1X4   ||
           iCheckPos==QualSite2X2 ||
           iCheckPos==_6Site2X3   ||
           iCheckPos==_8Site2X4   ||
           iCheckPos==_16Site2X8  ||
           iCheckPos==_16Site4X4  ||                                            //Sam 20190226 : 16Site4X4
           iCheckPos==_32Site4X8N)
        {
            if(TestIF_File.dSiteYPitch==0)
                TestIF_File.dSiteYPitch=60;
        }
        else if(iCheckPos==_10Site2X5 ||                                        //wei 20190614 10 site
                iCheckPos==_12Site2X6)
        {
            if(TestIF_File.dSiteYPitch==0)
                TestIF_File.dSiteYPitch=63.5;
        }
        bGPIBChangeSiteMap=false;                                               //JerryYang 20160105
    }

    if(EL<TGroupBox>("TfSetup", "gbArm1PickArm2Test")->Visible==false &&
       EL<TGroupBox>("TfSetup", "gbOcr")->Visible==false &&
       EL<TGroupBox>("TfSetup", "gbSocketClamp")->Visible==false &&
       EL<TGroupBox>("TfSetup", "grpSocketSensor")->Visible==false)
    {
    ;   // golden cSetUp.cpp:1193-1193 取代：表單寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1193-1193 -- 表單寬度（HTML 不用）
        fSetup->Width=818;
#endif // GATE (S12-C save)
        EL<TGroupBox>("TfSetup", "grpFunction")->Visible=false;
    }
    else
    {
    ;   // golden cSetUp.cpp:1198-1198 取代：表單寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1198-1198 -- 表單寬度（HTML 不用）
        fSetup->Width=1090;
#endif // GATE (S12-C save)
        EL<TGroupBox>("TfSetup", "grpFunction")->Visible=true;
    }

}

// golden cSetUp.cpp:1203  TfSetup::CompChange(int iMode)
static void SU_CompChange(int iMode)
{

    int iTestCHCT=SiteData[iMode].Cnt;

    if(IniConfig.bDualSiteSupply4CH==true)                                      //jou 2012-11-20 Dual Site supply 4's Channel
    {
        if(iMode==DualSite)
        {
            iTestCHCT+=2;
        }
    }

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
       CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(iMode==_8Site2X4 &&
           CosFunction.bEnableOctal_12Kit==true &&
           EL<TCheckBox>("TfSetup", "cbOctal12Site")->Checked==true)
        {
            iTestCHCT+=4;
        }
        else if(iMode==_8Site2X4 &&
                CosFunction.bEnableOctal_12Kit==true &&
                EL<TCheckBox>("TfSetup", "cbOctal12Site")->Checked==false)
        {
            iTestCHCT=SiteData[iMode].Cnt;
        }
    }

    if(CosFunction.bUse32ChanelSiteMap)                                         //Steven 20170530 (wei) : Use 32CH site map
    {
        iTestCHCT=32;

        if(IniConfig.bVTESTFunction)                                            //RogerYang 20260529 : 偉測趙坤鵬說開放到最多16site
        {
            iTestCHCT=16;
        }
    }

    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        TestLabRow[i]->Visible=false;
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
                SU_CbClear(TestSiteCH[i][j]);   // golden cSetUp.cpp:1247-1247 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1247-1247 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            TestSiteCH[i][j]->Clear();
#endif // GATE (S12-C save)
            TestSiteCH[i][j]->Visible=false;
            TestLabCol[j]->Visible=false;
        }
    }

    for(int i=0; i<SiteData[iMode].XItem; i++)
    {
        TestLabCol[i]->Visible=true;
        for(int j=0; j<SiteData[iMode].YItem; j++)
        {
            TestSiteCH[j][i]->Visible=true;
            TestSiteCH[j][i]->Enabled=true;
            for(int k=0; k<=iTestCHCT; k++)
            {
                if(k==0)
                    TestSiteCH[j][i]->Items->Add("- - -");
                else
                    TestSiteCH[j][i]->Items->Add("CH "+AnsiString (k));
            }
            TestLabRow[j]->Visible=true;
        }
    }

}

// golden cSetUp.cpp:4523  TfSetup::chkOffCenterkitClick(TObject *Sender)
static void SU_chkOffCenterkitClick()
{

        int iCheckPos=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position;   // golden cSetUp.cpp:4525-4525 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4525-4525 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    int iCheckPos=EL<TControl>("TfSetup", "ScrollBar1")->Position;                                         //避免ScrollBar1->Position改變時，會有問題。
#endif // GATE (S12-C save)

    if(CosFunction.b2x4SupportCenterPitch==true &&                              //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
       iCheckPos==_8Site2X4)
    {
        if(EL<TCheckBox>("TfSetup", "chkUseXCenterPitch")->Checked==true &&
           FileExists(BmpPath+"8siteCenterX.bmp"))
        {
            ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+"8siteCenterX.bmp");
        }
        else
        {
            ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+TestSiteFileName[1][iCheckPos]);
        }
    }
    else if(EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked==false)
    {
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+TestSiteFileName[1][iCheckPos]);
        EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex=0;
        EL<TRadioGroup>("TfSetup", "rgYOffset")->Visible=false;
    }
    else
    {
        EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex=0;
        EL<TRadioGroup>("TfSetup", "rgYOffset")->Visible=false;
        switch(iCheckPos)
        {
            case SingleSite:                                                    //1x1
                ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+"1siteOffCentre.bmp");
                break;
            case DualSite:                                                      //1x2
                ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+"2siteOffCentre.bmp");
                break;
            case QualSite1X4:                                                   //1x4
                if(CosFunction.bNonCenterModeCanUseShtOffset &&
                   IniConfig.bA50Enable1x4BiasYOffset &&
                   EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked)                                    //Steven 20200715 : for Tinton
                {
                    EL<TRadioGroup>("TfSetup", "rgYOffset")->Visible=true;
                }
                ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+"4siteOffCentre.bmp");
                break;
            default:
                ;   // UI-only（HTML 端處理）：EL<TControl>("TfSetup", "Image1")->Picture->LoadFromFile(BmpPath+TestSiteFileName[1][iCheckPos]);
        }
    }

}

// golden cSetUp.cpp:4812  TfSetup::rgYPitchOffsetModeClick(TObject *Sender)
static void SU_rgYPitchOffsetModeClick()
{

    if(EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex==1)                                        //Steven 20201013 : NN mode add Y-Pitch offset selection
    {
        EL<TLabel>("TfSetup", "labYOffset")->Caption="Y Offset (mm)";                                    //KenHsieh 20231227 : add Y Center Pitch mode
    ;   // golden cSetUp.cpp:4817-4817 取代：標籤寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4817-4817 -- 標籤寬度（HTML 不用）
        EL<TLabel>("TfSetup", "labYOffset")->Width=93;
#endif // GATE (S12-C save)
        EL<TEdit>("TfSetup", "edYOffset")->Enabled=false;
        EL<TEdit>("TfSetup", "edYOffset")->Text=10;
    }
    else if(EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex==0)                                   //KenHsieh 20231227 : add Y Center Pitch mode
    {
        EL<TLabel>("TfSetup", "labYOffset")->Caption="Y Offset (mm)";
    ;   // golden cSetUp.cpp:4824-4824 取代：標籤寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4824-4824 -- 標籤寬度（HTML 不用）
        EL<TLabel>("TfSetup", "labYOffset")->Width=93;
#endif // GATE (S12-C save)
        EL<TEdit>("TfSetup", "edYOffset")->Text= FormatFloat("0.000", TestIF_File.dSiteYOffset);        //Steven 20140512 : For HT-9047 //jou 2016-04-22 Setup pitch "0.00" -> "0.000"
        EL<TEdit>("TfSetup", "edYOffset")->Enabled=true;
    }
    else
    {
        EL<TLabel>("TfSetup", "labYOffset")->Caption="Y Center Pitch (mm)";                              //KenHsieh 20231227 : add Y Center Pitch mode
    ;   // golden cSetUp.cpp:4831-4831 取代：標籤寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4831-4831 -- 標籤寬度（HTML 不用）
        EL<TLabel>("TfSetup", "labYOffset")->Width=134;
#endif // GATE (S12-C save)
        EL<TEdit>("TfSetup", "edYOffset")->Text= FormatFloat("0.000", TestIF_File.dSiteYOffset);        //Steven 20140512 : For HT-9047 //jou 2016-04-22 Setup pitch "0.00" -> "0.000"
        EL<TEdit>("TfSetup", "edYOffset")->Enabled=true;
    }

}

// golden cSetUp.cpp:3363  TfSetup::FormClose(TObject *Sender, TCloseAction &Action)
static void SU_FormClose()
{

    AnsiString str;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                     //Steven 20131101 : Add ASE-K12
    {
        if(SU_CHSetError())
        {
    filerw::ELMark("closeRefused");   // golden cSetUp.cpp:3371-3371 取代：Action=caNone：VCL 關窗參數（參數已拿掉）→ 記 trace
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3371-3371 -- Action=caNone：VCL 關窗參數（參數已拿掉）→ 記 trace
            Action=caNone;
#endif // GATE (S12-C save)
            return;
        }
    }

    if(AUTO_SENSOR_INSTALL && bSaveNeedHome)
    {
        filerw::ELMessage("Auto Shuttle Sensor Need Reset!!");
        return;
    }
    ReadFile();
    fShow=false;                                                                //Steven 20110907 : 一定要在ReadFile下面

    if(CosFunction.bI21EnableASMByRecipe)                                       //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    {
        if(CosFunction.bAutoSiteMappingUseHotPlate==true    &&                  //Ifor 20210913 add:關閉Site Mapping
           IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
           LastSet.iRunStartMode==rsmAutoSiteMap            )
        {
    if(FileRW_InOutArmSuckHasIC()==false &&   // golden cSetUp.cpp:3390-3390 取代：InArmSuck／OutArmSuck：FileRW/_KitSuck.cpp 轉接（!(A||B) ≡ A==false && B==false）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3390-3390 -- InArmSuck／OutArmSuck：FileRW/_KitSuck.cpp 轉接（!(A||B) ≡ A==false && B==false）
            if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false &&
#endif // GATE (S12-C save)
               ShuttleHasIC()==false    && IndexHasIC()==false        )
            {
                if(IniConfig.bI21EnableASM==false)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<HotPlateForm.XDivision; j++)
                        {
                            for(int k=0; k<HotPlateForm.YDivision; k++)
                            {
                                MOT[MMPlate1+i].Tray.SiteMapData[j][k]=0;       //Ifor 20210913 add: 清除Auto Site Mapping 補回Has Null IC資料
                            }
                        }
                    }
                    SetRunStartMode(rsmContinuStart);
                    bAutoSiteMapHotplateSave=false;
                    bSiteMappingCHKOK=true;
                }
            }
            else
            {
                IniConfig.bI21EnableASM=true;
            }
        }
    }

    bNeedPassword=false;
    bOCRNeedPassword=false;                                                     //ChungHung 20130118 add OCR - 關閉要密碼
    EL<TLabel>("TfSetup", "lbShowMessage")->Visible=false;

    ;   // golden cSetUp.cpp:3421-3421 取代：焦點（HTML 端）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3421-3421 -- 焦點（HTML 端）
    EL<TRadioButton>("TfSetup", "rbTemp")->SetFocus();                                                         //ChungHung 20130718 add 解決修改shuttle mode後按下離開  無法更新問題
#endif // GATE (S12-C save)

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);    //Steven 20100629

}

// golden cSetUp.cpp:3492  TfSetup::sbUpdateClick(TObject *Sender)
static void SU_sbUpdateClick()
{
    filerw::ELMark("sbUpdateClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cSetUp.cpp:3498-3498 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed，頁面自己關
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3498-3498 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed，頁面自己關
        Close();
#endif // GATE (S12-C save)
        return;
    }

    if(IniConfig.bF33_Check2DHardware &&                                        //JerryYang 20250220 : 2DID硬體順序檢查功能
        (BAR_CODE_INSTALL==ebctUseCCDMode ||
         BAR_CODE_INSTALL==ebctInShtIntel ||
         BAR_CODE_INSTALL==ebctEtherNetCCD) &&
        TestIF_File.bEnableBarCode)// &&
//        TestIF_File.bEnableBottom2D==false)                                   //JerryYang 20250220 : 2DID硬體順序檢查功能
    {
        if(HasICUnderMachine() || HasAnyICInMachine())
        {
            if(TestIF_File.iShuttleMode==1 &&
               EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked==true &&
               EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked==false)
            {
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked=false;
                EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked=true;
                filerw::ELMessage("因啟用2D CHECK功能,請先Clean out後再開ARM!");
                return;
            }
            else if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0 &&  //Arm 1
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked==false && EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked==true &&
                EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked==false && EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked==true)
            {
                filerw::ELMessage("因啟用2D CHECK功能,請先Clean out後再開ARM!");
                return;
            }
            else if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1 &&  //Arm 2
                EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked==false && EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked==true &&
                EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked==true && EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked==false)
            {
                filerw::ELMessage("因啟用2D CHECK功能,請先Clean out後再開ARM!");
                return;
            }
        }
    }

    if(SU_CHSetError())   // golden cSetUp.cpp:3537-3537 取代：golden bool 方法
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3537-3537 -- golden bool 方法
    if(SU_CHSetError())                                                            //jou 2010-09-02 start : Site map的問題,如出現相同或沒填的site,將所有的map全部清除,讓工程師重填.
#endif // GATE (S12-C save)
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                    SU_CbSetIndex(TestSiteCH[i][j], 0);   // golden cSetUp.cpp:3543-3543 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3543-3543 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                TestSiteCH[i][j]->ItemIndex=0;
#endif // GATE (S12-C save)
            }
        }
        return;
    }

    if(EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck")->Checked && EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck")->Checked)
    {
        filerw::ELMessage("Two 1x1 nozzle modes selected!","已選擇兩種1x1吸嘴模式");
        return;
    }

    if(SU_CheckShuttlePitch()==false)                                              //wei 20160914 Auto Shuttle Sensor
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)                                     //jou 2010-09-02 start : Site map的問題,如出現相同或沒填的site,將所有的map全部清除,讓工程師重填.
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                    SU_CbSetIndex(TestSiteCH[i][j], 0);   // golden cSetUp.cpp:3561-3561 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3561-3561 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                TestSiteCH[i][j]->ItemIndex=0;
#endif // GATE (S12-C save)
            }
        }
        return;
    }

    if(CosFunction.bLockRTCByFile &&                                            //ChungHung 20130118 add OCR - 關閉要密碼
       IniConfig.bRTC_Enable==false)                                            //Sam 20240311 : 新增 RTC Lock by file 功能
    {
    }
    else
    {
        if((bNeedPassword==true && EL<TGroupBox>("TfSetup", "gbRTC")->Enabled==true && EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked==false) ||   //Steven 20110907 : 關RTC要密碼
           (bOCRNeedPassword==true && EL<TGroupBox>("TfSetup", "gbOcr")->Enabled==true && EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked==false))
        {
                if(filerw::ELPasswordRefused("TfSetup::DoPassword (golden password form)")==false)   // golden cSetUp.cpp:3576-3576 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3576-3576 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            if(DoPassword()==false)
#endif // GATE (S12-C save)
            {
                if(bNeedPassword && EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked==false)
                    EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked=true;
                if(bOCRNeedPassword && EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked==false)
                    EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked=true;
            }
            else
            {
                if(bNeedPassword)
                {
                    bNeedPassword=false;
                    RecordProcess("RealTimeCCD Disabled");
                }

                if(bOCRNeedPassword)
                {
                    bOCRNeedPassword=false;
                    RecordProcess("OCR Disabled");
                }
            }
        }
        else if(bNeedPassword==true && EL<TGroupBox>("TfSetup", "gbRTC")->Enabled==true && EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked==false)    //Steven 20110907 : 關RTC要密碼
        {
                if(filerw::ELPasswordRefused("TfSetup::DoPassword (golden password form)")==false)   // golden cSetUp.cpp:3600-3600 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3600-3600 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
            if(DoPassword()==false)
#endif // GATE (S12-C save)
            {
                EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked=true;
            }
            else
            {
                bNeedPassword=false;
                RecordProcess("RealTimeCCD Disabled");
            }
        }
        else if(bNeedPassword==false && EL<TGroupBox>("TfSetup", "gbRTC")->Enabled==true && EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked==true)
        {
            filerw::ELMessage("Program need to restart to active RTC fuinction!", "The program will automatically be closed after exit setup form.");
    filerw::ELTodo("golden cSetUp.cpp sbUpdateClick: fMain->bNeedRestartProgram=true -- TfMain facade has none (GATE G-SU-Restart); restart request dropped");   // golden cSetUp.cpp:3613-3613 取代：fMain->bNeedRestartProgram：TfMain 門面沒有（同移植樹 GATE G-SU-Restart）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3613-3613 -- fMain->bNeedRestartProgram：TfMain 門面沒有（同移植樹 GATE G-SU-Restart）
            fMain->bNeedRestartProgram=true;
#endif // GATE (S12-C save)
        }
    }

    if(EL<TGroupBox>("TfSetup", "gbRTC")->Enabled==true && EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked==true)              //Steven 20110907 : RTC開了又關,也要輸密碼
    {
        bNeedPassword=true;
    }

    if(EL<TGroupBox>("TfSetup", "gbOcr")->Enabled==true && EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked==true)                    //ChungHung 20130118 add OCR - 開了又關,也要輸密碼
    {
        bOCRNeedPassword=true;
    }

    SU_SaveSetupFile(GetRecipePath());                                             //kevin 20150105
    #ifdef ASE_KaohSiung
    ;   // golden cSetUp.cpp:3629-3629 取代：#ifdef ASE_KaohSiung：移植樹沒有定義這個巨集，原樣留在 #if 0
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3629-3629 -- #ifdef ASE_KaohSiung：移植樹沒有定義這個巨集，原樣留在 #if 0
        fBuilder->bSaveAsJobFile(GetLastOpenFN(), "JOBFILE");
#endif // GATE (S12-C save)
    #endif

    filerw::ELTodo("golden cSetUp.cpp sbUpdateClick: fMain->bEnableAutoclean() -- TfMain facade has none (GATE G-SU-AcBtn)");   // golden cSetUp.cpp:3632-3632 取代：fMain->bEnableAutoclean()：TfMain 門面沒有（GATE G-SU-AcBtn，只是顯示 AutoClean 鈕）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3632-3632 -- fMain->bEnableAutoclean()：TfMain 門面沒有（GATE G-SU-AcBtn，只是顯示 AutoClean 鈕）
    fMain->bEnableAutoclean();                                                  //kevin 20120518 autoclean
#endif // GATE (S12-C save)
    FileRW_Setup_ContactDutCount();   // golden cSetUp.cpp:3633-3633 取代：移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單是 fContactForm（forms/fContact.h:1634，DutCount 在 :1522）→ FileRW 轉接
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3633-3633 -- 移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單是 fContactForm（forms/fContact.h:1634，DutCount 在 :1522）→ FileRW 轉接
    fContact->DutCount();                                                       //jou 2014-09-06 修正開啟程式的時候EP異常
#endif // GATE (S12-C save)

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.ArmOnOff);                                       //45     開關Arm
        EventReport(SECS_EVENT.SaveRecipe);
    }
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    FileRW_Setup_ContactReadFile();   // golden cSetUp.cpp:3642-3642 取代：同上：fContactForm->ReadFile()（forms/fContact.h:1495）→ FileRW 轉接
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3642-3642 -- 同上：fContactForm->ReadFile()（forms/fContact.h:1495）→ FileRW 轉接
    fContact->ReadFile();                                                       //JerryYang 20180518 : add
#endif // GATE (S12-C save)
    SetWorkParameter();
    if(AUTO_SENSOR_INSTALL)                                                     //wei 20170213
        bSaveNeedHome=true;

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 修改模式需切換是否使用NS KIT
    {
        ADAM_WriteVoltage(DeviceForm.dPress);
    }

}

// golden cSetUp.cpp:1292  TfSetup::CHSetError()
static bool SU_CHSetError()
{
    filerw::ELMark("CHSetError");   // FileRW：存檔流程 trace（ack.trace）

    bool bSelOne=false;
    int iXpitch, iYpitch;
    int iRow1, iCol1, iRow2, iCol2;

    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        for(int j=i+1; j<MAX_SOCKET_TOTAL; j++)
        {
            iRow1=i/MAX_SOCKET_COL;
            iCol1=i%MAX_SOCKET_COL;
            iRow2=j/MAX_SOCKET_COL;
            iCol2=j%MAX_SOCKET_COL;
            if(TestSiteCH[iRow1][iCol1]->ItemIndex==-1 &&
               TestSiteCH[iRow1][iCol1]->Visible)
            {
                filerw::ELMessage("Test Site Not Select");
                return true;
            }

            if(TestSiteCH[iRow1][iCol1]->Visible &&
               TestSiteCH[iRow2][iCol2]->Visible)
            {
                if(TestSiteCH[iRow1][iCol1]->ItemIndex==TestSiteCH[iRow2][iCol2]->ItemIndex &&
                   TestSiteCH[iRow1][iCol1]->ItemIndex!=0)
                {
                    filerw::ELMessage("Test Site Set Same CH");
                    return true;
                }

                if(TestSiteCH[iRow1][iCol1]->ItemIndex>=1 ||
                   TestSiteCH[iRow2][iCol2]->ItemIndex>=1)
                {
                    bSelOne=true;
                }
            }
        }
    }

        if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==SingleSite)   // golden cSetUp.cpp:1332-1332 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1332-1332 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    if(EL<TControl>("TfSetup", "ScrollBar1")->Position==SingleSite)
#endif // GATE (S12-C save)
    {
            SU_CbSetIndex(TestSiteCH[0][0], 1);   // golden cSetUp.cpp:1334-1334 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1334-1334 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        TestSiteCH[0][0]->ItemIndex=1;
#endif // GATE (S12-C save)
        bSelOne=true;
    }

    if(bSelOne==false)
    {
        filerw::ELMessage("Test Site Not Select");
        return true;
    }

    iXpitch=atoi(EL<TEdit>("TfSetup", "XPitch")->Text.c_str());                                         //jou 980725 start : set over limit
    iYpitch=atoi(EL<TEdit>("TfSetup", "YPitch")->Text.c_str());

        if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position!=SingleSite &&   // golden cSetUp.cpp:1347-1347 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1347-1347 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    if(EL<TControl>("TfSetup", "ScrollBar1")->Position!=SingleSite &&
#endif // GATE (S12-C save)
           filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position!=DualSite2x1)   // golden cSetUp.cpp:1348-1348 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1348-1348 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
       EL<TControl>("TfSetup", "ScrollBar1")->Position!=DualSite2x1)
#endif // GATE (S12-C save)
    {
        if(iXpitch<20)
        {
            filerw::ELMessage("X Pitch can not < 20mm!!");
            return true;
        }
    }

        switch(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position)   // golden cSetUp.cpp:1357-1357 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:1357-1357 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    switch(EL<TControl>("TfSetup", "ScrollBar1")->Position)
#endif // GATE (S12-C save)
    {
        case SingleSite:
            break;
        case DualSite2x1:
            if(iYpitch>80)
            {
                filerw::ELMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                filerw::ELMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case DualSite:                                                          //1x2
            if(MachineTypeChoice==Type_HT9046_LS)                               //Steven 20200114 : SCK新的SLK
            {
                if(iXpitch<40 || iXpitch>240)
                {
                    filerw::ELMessage("X Pitch Must Between 40~240mm");
                    return true;
                }
            }
            else if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                        //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch<40 || iXpitch>120)                                   //Steven 20150505 : 1x2加大支援X-Pitch 120mm
                {
                    filerw::ELMessage("X Pitch Must Between 40~120mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch<40 || iXpitch>150)
                {
                    filerw::ELMessage("X Pitch Must Between 40~150mm");
                    return true;
                }
            }
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)                                                  //Steven 20100617 : 60 -> 80
                {
                    filerw::ELMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    filerw::ELMessage("X Pitch can not over 100mm");
                    return true;
                }
            }
            break;
        case QualSite2X2:                                                       //2x2
            if(iXpitch>200)                                                     //Steven 20150224 : 韓國Danny說要180的Pitch
            {
                filerw::ELMessage("X Pitch can not over 200mm");
                return true;
            }

            if(iYpitch>80)
            {
                filerw::ELMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                filerw::ELMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>120)                                                 //Steven 20200211 : 80 --> 120 for SCK
                {
                    filerw::ELMessage("X Pitch can not over 120mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    filerw::ELMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)
            {
                filerw::ELMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                filerw::ELMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _8Site2X4:                                                         //2x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)                                                  //Steven 20100825 : 60 -> 80
                {
                    filerw::ELMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    filerw::ELMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)                                                      //Steven 20100617 : 60 -> 80
            {
                filerw::ELMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                filerw::ELMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
        case _12Site2X6:
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)
                {
                    filerw::ELMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    filerw::ELMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)
            {
                filerw::ELMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<30)                                                      //Steven 20170530 (wei) : 35 --> 30
            {
                filerw::ELMessage("Y Pitch can not < 30mm");
                return true;
            }
            break;
        case _16Site2X8:                                                        //2x8
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch50mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>50)
                {
                    filerw::ELMessage("X Pitch can not over 50mm");
                    return true;
                }
            }
            else
            {
                if(MachineTypeChoice==Type_HT9046_LS ||
                   SubMachineType==Type_HT9046LA)                               //Steven 20190307 : Add for HT-9046LA
                {
                    if(iXpitch>45)                                              //JerryYang 20171101 (wei) 16 site X-pitch最大值40->45
                    {
                        filerw::ELMessage("X Pitch can not over 45mm");
                        return true;
                    }
                }
                else if(MachineTypeChoice==Type_HT1032)
                {
                    if(iXpitch>40)
                    {
                        filerw::ELMessage("X Pitch can not over 40mm");
                        return true;
                    }
                }
                else
                {
                    if(iXpitch>30)
                    {
                        filerw::ELMessage("X Pitch can not over 30mm");
                        return true;
                    }
                }
            }

            if(iYpitch>63.5)
            {
                filerw::ELMessage("Y Pitch can not over 63.5mm");
                return true;
            }

            if(iYpitch<20)
            {
                filerw::ELMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _32Site4X8M:
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm ||                           //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
               MachineTypeChoice==Type_HT1032)
            {
                if(iXpitch>40)
                {
                    filerw::ELMessage("X Pitch can not over 40mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>50)
                {
                    filerw::ELMessage("X Pitch can not over 50mm");
                    return true;
                }
            }

            if(iYpitch>63.5)
            {
                filerw::ELMessage("Y Pitch can not over 63.5mm");
                return true;
            }

            if(iYpitch<35)
            {
                filerw::ELMessage("Y Pitch can not < 35mm");
                return true;
            }
            break;
    }
    return false;

}

// golden cSetUp.cpp:4678  TfSetup::CheckShuttlePitch()
static bool SU_CheckShuttlePitch()
{
    filerw::ELMark("CheckShuttlePitch");   // FileRW：存檔流程 trace（ack.trace）

    bool bShuttleSensorCanMove=false;
    double iShuttlePitch=0.0;
    double iXpitch=atof(EL<TEdit>("TfSetup", "XPitch")->Text.c_str());

    if(AUTO_SENSOR_INSTALL)
    {
            if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==DualSite     ||   // golden cSetUp.cpp:4686-4686 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4686-4686 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        if(EL<TControl>("TfSetup", "ScrollBar1")->Position==DualSite     ||                                //1x2 //sensor 2 4
#endif // GATE (S12-C save)
               filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==QualSite2X2  ||   // golden cSetUp.cpp:4687-4687 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4687-4687 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
           EL<TControl>("TfSetup", "ScrollBar1")->Position==QualSite2X2  ||
#endif // GATE (S12-C save)
               filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==QualSite2X2N)   // golden cSetUp.cpp:4688-4688 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4688-4688 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
           EL<TControl>("TfSetup", "ScrollBar1")->Position==QualSite2X2N)                                  //2x2
#endif // GATE (S12-C save)
        {
            iShuttlePitch=iXpitch*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
            }
            else if(iShuttlePitch>=9000 && iShuttlePitch<=18000)
            {
                bShuttleSensorCanMove=true;
            }
        }
            else if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==TriSite1X3 ||   // golden cSetUp.cpp:4700-4700 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4700-4700 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        else if(EL<TControl>("TfSetup", "ScrollBar1")->Position==TriSite1X3 ||                             //1x3  //Frank 20160329 add for 1x3_4    //sensor 1 3 5
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_6Site2X3  ||   // golden cSetUp.cpp:4701-4701 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4701-4701 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_6Site2X3  ||                             //ChungHung 20140115 add for 2x3_6
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_6Site2X3N )   // golden cSetUp.cpp:4702-4702 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4702-4702 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_6Site2X3N )                              //Steven 20220425 : 2x3 NN mode
#endif // GATE (S12-C save)
        {
            iShuttlePitch=iXpitch*2*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
            }
            else if(iShuttlePitch>=8759 && iShuttlePitch<=24000)                //20170213
            {
                bShuttleSensorCanMove=true;
            }
        }
            else if(filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==QualSite1X4 ||   // golden cSetUp.cpp:4714-4714 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4714-4714 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
        else if(EL<TControl>("TfSetup", "ScrollBar1")->Position==QualSite1X4 ||                            //1x4                       //sensor 1 2 4 5
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_8Site1X4   ||   // golden cSetUp.cpp:4715-4715 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4715-4715 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_8Site1X4   ||                            //ChungHung 20150528 add for 海思 _8Site1x4
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_8Site2X4   ||   // golden cSetUp.cpp:4716-4716 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4716-4716 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_8Site2X4   ||
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_16Site4X4  ||   // golden cSetUp.cpp:4717-4717 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4717-4717 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_16Site4X4  ||                            //Sam 20190226 : 16Site4X4 //2x4
#endif // GATE (S12-C save)
                    filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position==_8Site2X4N  )   // golden cSetUp.cpp:4718-4718 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4718-4718 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                EL<TControl>("TfSetup", "ScrollBar1")->Position==_8Site2X4N  )                             //Wei 20231211 : 2X4NN Mode
#endif // GATE (S12-C save)
        {
            iShuttlePitch=iXpitch*3*100;
            if(iShuttlePitch>=12000 && iShuttlePitch<=24000)
            {
                bShuttleSensorCanMove=true;
            }
        }

        if(bShuttleSensorCanMove==false)
        {
            filerw::ELMessage("Auto Shuttle Sensor 不支援此Pitch!!");
            return false;
        }
    }
    else
    {
        return true;
    }

    return bShuttleSensorCanMove;

}

// golden cSetUp.cpp:3655  TfSetup::SaveSetupFile(AnsiString sPath)
static void SU_SaveSetupFile(AnsiString sPath)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString str;
    int iSite=0;                                                                //kevin 20160818
    MyForceDirectories(sPath);
    AnsiString sTemp    =IncludeTrailingPathDelimiter(sPath)+"Temperature.Data";
    AnsiString sContact =IncludeTrailingPathDelimiter(sPath)+"Contact.Data";
    AnsiString szDir    =IncludeTrailingPathDelimiter(sPath)+"HandlerCondition.Data";

    WriteIniData(szDir, "Configuration", "12_16SiteDirectHeaterLayout", EL<TCheckBox>("TfSetup", "cb16DirectHeater")->Checked);                 //Steven 20140923 : Index使用EJ1N版32組加熱器
    WriteIniData(szDir, "Configuration", "12Site10DirectHeaterLayout",  EL<TCheckBox>("TfSetup", "cb12Site10DirectHeater")->Checked);           //Steven 20140923 : Index使用EJ1N版32組加熱器

        WriteIniData(szDir, "Configuration", "Test Mode", TestSiteFileName[0][filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position]);   // golden cSetUp.cpp:3667-3667 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3667-3667 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    WriteIniData(szDir, "Configuration", "Test Mode", TestSiteFileName[0][EL<TControl>("TfSetup", "ScrollBar1")->Position]);                   //Steven 20120814 : 改用文字存模式,方便新增新的模式
#endif // GATE (S12-C save)

        TestIF_File.iTestMode=GetTestMode(TestSiteFileName[0][filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position]);   // golden cSetUp.cpp:3669-3669 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3669-3669 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
    TestIF_File.iTestMode=GetTestMode(TestSiteFileName[0][EL<TControl>("TfSetup", "ScrollBar1")->Position]);                   //Steven 20250321 : 整合TestMode轉換
#endif // GATE (S12-C save)
    bSiteMapHasChange=false;
    if(iTestMode!=TestIF_File.iTestMode)                                        //Steven 20250102 : 紀錄test mode與site map是否有變更
    {
        bSiteMapHasChange=true;
        DoInArm_9045_Type();                                                    //Steven 20250321 : 要先更新一次
        iTestMode=TestIF_File.iTestMode;
    }

    if(CosFunction.bI21EnableASMByRecipe)                                       //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    {
        WriteIniData(szDir, "Auto Site Mapping", "Enable Auto Site Mapping",    EL<TCheckBox>("TfSetup", "cbI21")->Checked?1:0);
    }

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20220624 : JSCC楊建軍要求跟著工作檔跑
    {
        WriteIniData(szDir, "O_Count", "bLifeTimeCount 1",          (EL<TCheckBox>("TfSetup", "cbO12_LifeTimeCount")->Checked)?1:0);
        WriteIniData(szDir, "O_Count", "bLifeTimeCount 2",          (EL<TCheckBox>("TfSetup", "cbO13_LifeTimeCount")->Checked)?1:0);
        WriteIniData(szDir, "O_Count", "bLifeTimeCount 3",          (EL<TCheckBox>("TfSetup", "cbO14_LifeTimeCount")->Checked)?1:0);
    }

    WriteIniData(szDir, "Configuration", "Enable Single Use Other Suck",   (EL<TCheckBox>("TfSetup", "cbSingleUseOtherSuck")->Checked)   ?1:0);         //wei 20220905 Single Use Other Suck(IN C OUT E)

    if(DeviceForm.XDimension<7000)                                              //wei 20251218 DualSite Use One Suck
        EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck")->Checked=false;
    WriteIniData(szDir, "Configuration", "Enable DualSite Use One Suck",   (EL<TCheckBox>("TfSetup", "cbbDualSiteUseOneSuck")->Checked)  ?1:0);         //wei 20251218 DualSite Use One Suck
    WriteIniData(szDir, "Configuration", "Enable Single In Use Other Suck",(EL<TCheckBox>("TfSetup", "cbSingleInArmUseOtherSuck")->Checked)   ?1:0);    //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    WriteIniData(szDir, "Configuration", "Enable Prevent Drop function",   (EL<TCheckBox>("TfSetup", "cbPreventDropfunction")->Checked)   ?1:0);        //Frank 20260710 ADD//Eastsun 20260710整合

    if(EL<TCheckBox>("TfSetup", "cbPreventDropfunction")->Checked==true)                                                                                //Frank 20260710 ADD//Eastsun 20260710整合
        WriteIniData(sContact, "Mode", "Suck Shuttle Device After Tested",  1);
    WriteIniData(szDir, "Configuration", "Shuttle1 Cancel",         (EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked)            ?0:1);
    WriteIniData(szDir, "Configuration", "Shuttle2 Cancel",         (EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked)            ?0:1);
    WriteIniData(szDir, "Configuration", "Shuttle Mode",            (EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked)      ?0:1);
    WriteIniData(szDir, "Configuration", "QualSite2X2 Shift",       (EL<TCheckBox>("TfSetup", "cbQualSite2X2Shift")->Checked)   ?1:0);          //wei 20160226 TSMC X Shift
    WriteIniData(szDir, "Configuration", "NS7000 bias kit",         (EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked)      ?1:0);          //jou 981208 : NS7000 bias kit
    if(IniConfig.bKoreaFunction==false)                                         //ChungHung 20121031 add
        WriteIniData(szDir, "Configuration", "NS8000 Change Socket", (EL<TCheckBox>("TfSetup", "cbNS8000H")->Checked)        ?1:0);

    if(MachineTypeChoice==Type_HT1032)
        WriteIniData(szDir, "Configuration", "6 Cable Layout Kit",  (EL<TCheckBox>("TfSetup", "cb6CableLayoutKit")->Checked)    ?1:0);          //Steven 20230613 : ATC6.1 16Site SLK
    WriteIniData(szDir, "Configuration", "2 Cable Layout Kit",      (EL<TCheckBox>("TfSetup", "cb2CableLayoutKit")->Checked)    ?1:0);          //Steven 20150724 : 16溫控器 2條線版本
    WriteIniData(szDir, "Configuration", "1 Cable Layout Kit",      (EL<TCheckBox>("TfSetup", "cb1CableLayoutKit")->Checked)    ?1:0);          //jou 2015-10-15  : 16溫控器 1條線版本
    WriteIniData(szDir, "Configuration", "Rotate Shuttle",          (EL<TCheckBox>("TfSetup", "chkRotateShuttle")->Checked)     ?1:0);
    if(USE_16_HEATER==eht4Heater)
        WriteIniData(szDir, "Configuration", "Single site Use single heater", (EL<TCheckBox>("TfSetup", "cbSingleSiteSingleHeater")->Checked)     ?1:0); //JerryYang 20161013 新增Single site一支加熱棒模式
    else
        WriteIniData(szDir, "Configuration", "Single site Use single heater", false); //JerryYang 20161013 新增Single site一支加熱棒模式
    if(INSTALL_OCR==eocrUninstal)                                               //Steven 20160122
        WriteIniData(szDir, "Configuration", "OCR Function",        false);     //Steven 20120716 : OCR
    else
        WriteIniData(szDir, "Configuration", "OCR Function",        (EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked)        ?1:0);          //Steven 20120716 : OCR

    if(INSTALL_SOCKET_CLAMP==false)                                             //JerryYang 20160523
        WriteIniData(szDir, "Configuration", "Socket Clamp Function",        false);
    else
        WriteIniData(szDir, "Configuration", "Socket Clamp Function",        (EL<TCheckBox>("TfSetup", "cbUseSLKClamp")->Checked)        ?1:0);

    if(EL<TCheckBox>("TfSetup", "cbUseSLKClamp")->Checked==false)
        WriteIniData(szDir, "Configuration", "Enable Separability Test",            0);
    else
        WriteIniData(szDir, "Configuration", "Enable Separability Test",            EL<TRadioGroup>("TfSetup", "rgseparabilityTest")->ItemIndex);

    WriteIniData(szDir, "Configuration", "SocketSensor",            (EL<TCheckBox>("TfSetup", "cbSocketSensor")->Checked)        ?1:0);         //kevin 20130504 socket sensor
    WriteIniData(szDir, "Configuration", "SocketCountt",            EL<TComboBox>("TfSetup", "CoSocketCombo")->Text);                           //kevin 20130504 socket sensor 多少個
    WriteIniData(szDir, "Configuration", "bArm1PickPlaceArm2Test",  (EL<TCheckBox>("TfSetup", "Arm1PickArm2Test")->Checked)?1:0);               //kevin 20150127 Arm1 下壓 arm2 測試
    WriteIniData(szDir, "Configuration", "bCheckArm2Vacuum",        (EL<TCheckBox>("TfSetup", "cbCheckArm2Vacuum")->Checked)?1:0);              //Steven 20150129 : 需要確認Arm2有沒有粘料

    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200420 : Socket Sensor功能可以選
    {
        str.sprintf("iSensorCheckType%d", i+1);
        WriteIniData(szDir, "Configuration", str, MyTempRGBox[i]->ItemIndex);
    }

    WriteIniData(szDir, "Configuration", "SocketDisibleinitialcheck", (EL<TCheckBox>("TfSetup", "cbdisibleinitialcheck")->Checked)        ?1:0); //kevin 20170209 (wei) disible socket sensor index check
    WriteIniData(szDir, "Configuration", "bUseTwoArmwork", bUseTwoArm32Site);   //kevin 20190322 add use two arm

    WriteIniData(szDir, "Configuration", "Enable Preciser",             (EL<TCheckBox>("TfSetup", "cbEnablePreciser")->Checked)           ?1:0);      //Frank 20180410 (Steven) : InArm Preciser Station
    WriteIniData(szDir, "Configuration", "Enable RT Preciser",          (EL<TCheckBox>("TfSetup", "cbEnabledPreciserRT")->Checked)        ?1:0);      //Frank 20180410 (Steven) : InArm Preciser Station
    WriteIniData(szDir, "Configuration", "Enable Preciser HotPlate",  (EL<TCheckBox>("TfSetup", "cbEnablePreciserHotPlate")->Checked)   ?1:0);      //Kevincheng 20260525 : 渠梁常溫模式使用hotplate

    if(IniConfig.bVTESTFunction==true)                                          //jou 20200701 : VTEST for auto site mapping cable mount
    {
        WriteIniData(szDir, "Configuration", "Auto site mapping open site",  (EL<TCheckBox>("TfSetup", "chkSiteMapOpenSite")->Checked)           ?1:0);
        WriteIniData(szDir, "Configuration", "Auto site mapping one cycle",  (EL<TCheckBox>("TfSetup", "chkSiteMapOneCycle")->Checked)           ?1:0);
    }

    if(IniConfig.bRTCbySystem)                                                  //ChungHung 20120716 RTC by System
    filerw::ELTodo("golden cSetUp.cpp:3757-3757 IniConfig.bRTCbySystem：直寫 config.ini [RTC] Enable —— config.ini 由 FileRW/IniConfig 擁有，這裡不寫");
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3757-3757 -- IniConfig.bRTCbySystem：直寫 config.ini [RTC] Enable —— config.ini 由 FileRW/IniConfig 擁有，這裡不寫
        WriteIniData("D:\\HT9045\\config\\config.ini", "RTC", "Enable",         (EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked)  ?0:1);
#endif // GATE (S12-C save)
    else
        WriteIniData(szDir, "Configuration", "Real Time CCD",       (EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked)  ?0:1);      //Steven 20110824 : Real time CCD - Set Up可以開關

    WriteIniData(szDir, "Configuration", "bUseSocketFloat",         (EL<TCheckBox>("TfSetup", "cbEnableSocketFloat")->Checked)  ?1:0);      //Frank QQ
    WriteIniData(szDir, "Configuration", "bUseRTCStepAsideMode",    (EL<TCheckBox>("TfSetup", "cbEnableStepAsideMode")->Checked)  ?1:0);    //Ifor 20230908 add:RTC 讓位功能 By Setup File

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210422 : 1x4,xpitch>40,not support inarm 8 picker
    {
        if(TestIF_File.iTestMode==QualSite1X4 &&
           atoi(EL<TEdit>("TfSetup", "XPitch")->Text.c_str())>40 &&
           EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==1)
        {
            filerw::ELMessage("1x4, X pitch>40mm, not support in arm use 8 pick unit!!");
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex=0;
            EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->Visible=false;
            EL<TEdit>("TfSetup", "XPitch")->Text="";
            return;
        }
    }

    //==> Eastsun 20260526 #026-4.3 Ifor 20240618 :KYEC 特殊KIT 至小數點第三位
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        WriteIniData(szDir, "Configuration", "X Pitch",                 FormatFloat("0.000", EL<TEdit>("TfSetup", "XPitch")->Text.ToDouble()));
        WriteIniData(szDir, "Configuration", "Y Pitch",                 FormatFloat("0.000", EL<TEdit>("TfSetup", "YPitch")->Text.ToDouble()));
    }
    else
    {
        WriteIniData(szDir, "Configuration", "X Pitch", FormatFloat("0.00", EL<TEdit>("TfSetup", "XPitch")->Text.ToDouble()));  //JerryYang 20200214 統一格式小數點第二位
        WriteIniData(szDir, "Configuration", "Y Pitch", FormatFloat("0.00", EL<TEdit>("TfSetup", "YPitch")->Text.ToDouble()));
    }
    //<== Eastsun 20260526 #026-4.3
    if(CosFunction.bNonCenterModeCanUseShtOffset)
    {
        if(IniConfig.bA50Enable1x4BiasYOffset &&                                //Steven 20200715 : for Tinton
           TestIF_File.iTestMode==QualSite1X4 &&
           EL<TCheckBox>("TfSetup", "chkOffCenterkit")->Checked)
        {
            WriteIniData(szDir, "Configuration", "i1x4SiteYOffset", EL<TRadioGroup>("TfSetup", "rgYOffset")->ItemIndex);
        }
        else
        {
            WriteIniData(szDir, "Configuration", "i1x4SiteYOffset", 0);
        }
    }

    WriteIniData(szDir, "Configuration", "iYPitchOffsetMode",       EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex);
    if(EL<TRadioGroup>("TfSetup", "rgYPitchOffsetMode")->ItemIndex!=1)                                        //KenHsieh 20231227 : add Y Center Pitch mode
        WriteIniData(szDir, "Configuration", "Y Offset",            EL<TEdit>("TfSetup", "edYOffset")->Text                    );               //Steven 20140512 : For HT-9047
    WriteIniData(szDir, "Configuration", "Search Last Mode",         EL<TRadioGroup>("TfSetup", "rgSelectSearchLast")->ItemIndex      );
    WriteIniData(szDir, "Configuration", "Octal Pitch 80",          (EL<TCheckBox>("TfSetup", "chkOctal80")->Checked)           ?1:0);              //2013-03-05    Dell    for ATK pitch 80mm kit
    WriteIniData(szDir, "Configuration", "Octal 16 Kit",            (EL<TCheckBox>("TfSetup", "cbOctal16Site")->Checked)        ?1:0);              //Steven 20140312 : 8Site使用16Site Kit
        WriteIniData(szDir, "Configuration", "Square Octal Kit",    (EL<TCheckBox>("TfSetup", "cbSquareOctalLayout")->Checked)  ?1:0);              //Steven 20141224 : 2x2Site使用8Site Kit
    WriteIniData(szDir, "Configuration", "2x2 Use 16 Site Kit",     (EL<TCheckBox>("TfSetup", "chk2x2Use16siteSLK")->Checked)   ?1:0);              //Steven 20191113 : 2x2Site使用16Site Kit
    WriteIniData(szDir, "Configuration", "Octal 12Kit",             (EL<TCheckBox>("TfSetup", "cbOctal12Site")->Checked)        ?1:0);              //ChungHung 20140508 add for SCK
    WriteIniData(szDir, "Configuration", "1x2_Use_1x4_Kit",         (EL<TCheckBox>("TfSetup", "cb1x2Use1x4siteSLK")->Checked)  ?1:0);               //Isaac 20200311 : 1x2Site使用1x4Site Kit
    WriteIniData(szDir, "Configuration", "Use_1x3_Kit",             (EL<TCheckBox>("TfSetup", "cbUse1x3siteSLK")->Checked)  ?1:0);                  //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    WriteIniData(szDir, "Configuration", "b2x6Use2x8SitSLK",        (EL<TCheckBox>("TfSetup", "chk12SiteUse2x8SLK")->Checked)  ?1:0);               //Steven 20240807 : 12Site使用16Site Kit

    WriteIniData(szDir, "Configuration", "bHontechLayoutKit2x2",    (EL<TCheckBox>("TfSetup", "cbHotechLayoutKit2x2")->Checked)        ?1:0);       //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
    WriteIniData(szDir, "Configuration", "X Shift Pitch",            EL<TEdit>("TfSetup", "XShiftPitch")->Text                   );             //wei 20160226 TSMC X Shift
    WriteIniData(szDir, "Configuration", "bNSKitPress",             (EL<TCheckBox>("TfSetup", "cbNSKitPress")->Checked)         ?1:0);              //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值

    WriteIniData(szDir, "Configuration", "bIndEPSLK",               (EL<TCheckBox>("TfSetup", "cbIndSLK")->Checked)             ?1:0);              //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值

    WriteIniData(szDir, "Configuration", "b16Direct12Shuttle",      (EL<TCheckBox>("TfSetup", "cb16change12DirectHeater")->Checked) ?1:0);          //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    WriteIniData(szDir, "Configuration", "b16Direct8Shuttle",       (EL<TCheckBox>("TfSetup", "cb16change8DirectHeater")->Checked) ?1:0);           //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
    WriteIniData(szDir, "Configuration", "bRTC20CheckFunction",     (EL<TCheckBox>("TfSetup", "cbRTC20CheckFunction")->Checked)?1:0);               //Ifor 20190712 : add RTC2.0 Check
    WriteIniData(szDir, "Configuration", "bRTC20GiveWayCheck",      (EL<TCheckBox>("TfSetup", "cbRTC20GiveWayCheck")->Checked)?1:0);                //Ifor 20190712 : add RTC2.0 Check

    WriteIniData(szDir, "Configuration", "bRTCICResidueCheck",      (EL<TCheckBox>("TfSetup", "cbRTCICResidueCheck")->Checked)?1:0);                //Ifor 20190712 : add RTC2.0 Check
    WriteIniData(szDir, "Configuration", "bRTC20OverFlowCheck",     (EL<TCheckBox>("TfSetup", "cbRTC20OverFlowCheck")->Checked)?1:0);               //Ifor 20190712 : add RTC2.0 Check

    WriteIniData(szDir, "Configuration", "bArm1UseHeat",            (EL<TCheckBox>("TfSetup", "cbArm1UseHeat")->Checked)?1:0);                      //Ifor 20190815 : add Arm1是否加熱開關
    WriteIniData(szDir, "Configuration", "bArm1PickPlaceArm2Test_RunAutoClean",            (EL<TCheckBox>("TfSetup", "cbAutoCleanAlso")->Checked)?1:0);//Jimmychiu 20230710 : Auto Clean 跑 Arm1 下壓 arm2 測試
    WriteIniData(szDir, "Configuration", "bArm1OnlyPlaceArm2TestAndSuck", (EL<TCheckBox>("TfSetup", "cbArm1OnlyPlaceArm2TestAndSuck")->Checked)?1:0);//Jimmychiu 20250722 : Arm1 Only Place Arm2 Test And Suck

    WriteIniData(szDir, "Configuration", "Preciser X Pitch",        EL<TEdit>("TfSetup", "edPreciserXPitch")->Text    );                        //Frank 20180410 (Steven) : InArm Preciser Station
    WriteIniData(szDir, "Configuration", "Preciser Y Pitch",        EL<TEdit>("TfSetup", "edPreciserYPitch")->Text    );                        //Frank 20180410 (Steven) : InArm Preciser Station
    WriteIniData(szDir, "Configuration", "Use Tester Dry Air",      (EL<TCheckBox>("TfSetup", "cbUseTesterDry")->Checked)           ?1:0);          //Ifor 20200227 : Fix Purge air無存檔問題

    if(CosFunction.bRotateUseHT7000HPKit && USE_ROTATE_KIT)                     //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
        WriteIniData(szDir, "Configuration", "UseRotateForHT7000HPKit",(EL<TCheckBox>("TfSetup", "cbUseRotateForHT7000HPKit")->Checked)?1:0);
    else
        WriteIniData(szDir, "Configuration", "UseRotateForHT7000HPKit",0);

    WriteIniData(szDir, "Configuration", "UseAutoTrayMap",                EL<TCheckBox>("TfSetup", "bUseTrayMap")->Checked);            //kevin 20220401 add Auto 123 Tray Map
    WriteIniData(szDir, "Configuration", "UseLoadCCDTrayMap",             EL<TCheckBox>("TfSetup", "bUseLoadCCD")->Checked);            //kevin 20220330 add Load 載入CCD 判斷有無IC 存在
    WriteIniData(szDir, "Configuration", "DoubleUnloadTray",              EL<TCheckBox>("TfSetup", "bDoubleUnloadTray")->Checked );     //kevin 20220506 add Unload Tray 2  倍的設定

    WriteIniData(szDir, "Configuration", "Auto1 Tray Count",              EL<TEdit>("TfSetup", "edAuto1Count")->Text);              //kevin 20220401
    WriteIniData(szDir, "Configuration", "Auto2 Tray Count",              EL<TEdit>("TfSetup", "edAuto2Count")->Text);              //kevin 20220401
    WriteIniData(szDir, "Configuration", "Auto3 Tray Count",              EL<TEdit>("TfSetup", "edAuto3Count")->Text);              //kevin 20220401
    WriteIniData(szDir, "Configuration", "SiteMapDirection",              IniConfig.iSiteMapDirection);     //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
    //kevin 20211130
    int K=1;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                      //kevin 20211130 K5 強制 SITE 順序  林建續
    { filerw::ELTodo("golden cSetUp.cpp SaveSetupFile CC_ASE_KaohSiung_K3 forced site order (TestSocket.iShtRow/iShtCol, needs FileRW/_KitSuck.cpp adapter) not ported -- Site Aa..Dh keys NOT written for K3"); }   // golden cSetUp.cpp:3855-3877 取代：CC_ASE_KaohSiung_K3 強制 site 順序：迴圈上限 TestSocket.iShtRow／iShtCol（TMyKitSuck，aHotPlateSubstrate.h 與 HTEditList.h 衝突，本 TU 看不到）—— 需要 FileRW/_KitSuck.cpp 加轉接 FileRW_TestSocketShtRowCol()（整合者）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:3855-3877 -- CC_ASE_KaohSiung_K3 強制 site 順序：迴圈上限 TestSocket.iShtRow／iShtCol（TMyKitSuck，aHotPlateSubstrate.h 與 HTEditList.h 衝突，本 TU 看不到）—— 需要 FileRW/_KitSuck.cpp 加轉接 FileRW_TestSocketShtRowCol()（整合者）
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                str.sprintf("Site %c%c", i+'A', j+'a');                         //kevin 20180312 j+'A' ==> j+'a'
                if(TestSiteCH[i][j]->ItemIndex==0)
                {
                    iSite=0;
                }
                else
                {
                    iSite=K;
                    TestSiteCH[i][j]->ItemIndex=K;
                }

                if(iSite<0)                                                     //kevin 20160818 沒有使用 Site 0
                    iSite=0;
                WriteIniData(szDir, "Configuration", str, iSite);               //kevin 20160818 沒有使用 Site 0
                K++;
            }
        }
    }
#endif // GATE (S12-C save)
    else
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                str.sprintf("Site %c%c", i+'A', j+'a');                         //kevin 20180312 j+'A' ==> j+'a'
                if(IniConfig.bSPILFunction==true)                               //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction  //JerryYang 20161019 影響到run check比對,矽品要求恢復成原本的存檔內容
                {
                    WriteIniData(szDir, "Configuration", str, TestSiteCH[i][j]->ItemIndex);
                }
                else
                {
                    iSite = TestSiteCH[i][j]->ItemIndex;
                    if(iSite<0)                                                 //kevin 20160818 沒有使用 Site 0
                        iSite=0;
                    WriteIniData(szDir, "Configuration", str, iSite);           //kevin 20160818 沒有使用 Site 0
                }
            }
        }
    }

    if(EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked)                                                //Steven 20101013 : 關單Arm不可使用多吸嘴
    {
        switch(TestIF_File.iTestMode)
        {
            case DualSite:                                                      //1x2
                WriteIniData(szDir, "Configuration", "Use Suck Mode", (EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==0)?2:4);
                break;
            case QualSite1X4:                                                   //1x4
            case QualSite2X2:                                                   //2x2
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
            case _8Site2X4N:                                                    //Jimmychiu 20240219 : add for 8site2x4N
                WriteIniData(szDir, "Configuration", "Use Suck Mode", (EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==0)?4:8);
                break;
            case _8Site2X4:
            case _16Site4X4:
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                  //Steven 20231207 : 2x4 16 picker
                   USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
                {
                    WriteIniData(szDir, "Configuration", "Use Suck Mode", (EL<TRadioGroup>("TfSetup", "rgUseSuckMode")->ItemIndex==0)?8:16);
                }
                break;
        }
    }
    else
    {
        switch(TestIF_File.iTestMode)
        {
            case DualSite:                                                      //1x2
                WriteIniData(szDir, "Configuration", "Use Suck Mode", 2);
                break;
            case TriSite1X3:                                                    //1x3  //Frank 20160329 add for 1x3_4
            case QualSite1X4:                                                   //1x4
            case QualSite2X2:                                                   //2x2
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
                WriteIniData(szDir, "Configuration", "Use Suck Mode", 4);
                break;
            case _8Site2X4:
            case _16Site4X4:
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                  //Steven 20231207 : 2x4 16 picker
                   USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
                {
                    WriteIniData(szDir, "Configuration", "Use Suck Mode", 8);
                }
                break;
        }
    }

    if(ATC_SYSTEM>eATC30 &&                                                     //20141204 ChungHung add for ATC3.0
       ATC_SYSTEM!=eNonChamber)                                                 //2014-05-30    Dell    for ATC6.0
    {                                                                           //Steven 20100119 Start: Use NS 7000 Change Socket
        if((TestIF_File.iTestMode==QualSite2X2  ||
            TestIF_File.iTestMode==_8Site2X4    ||
            TestIF_File.iTestMode==QualSite1X4  ||                              //kevin 20150121
            TestIF_File.iTestMode==_8Site1X4    ||                              //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==DualSite2x1  ||
            TestIF_File.iTestMode==QualSite2X2N ||                              //Frank 20200520 2X2NN Mode
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4 //Steven 20140312 : For HT9045WA
            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Checked)
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 1);
        }
        else if(CUSTOMER_CODE==CC_SIGURD_PeiXing &&
                TestIF_File.iTestMode==_12Site2X6 &&
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Checked)                                           //Ifor 20170420 (wei) add for 海思 _12Site2X6
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 1);
        }
        else
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 0);
        }
    }
    else
    {
        if((TestIF_File.iTestMode==QualSite2X2  ||
            TestIF_File.iTestMode==_8Site2X4    ||
            TestIF_File.iTestMode==QualSite1X4  ||                              //kevin 20150121
            TestIF_File.iTestMode==_8Site1X4    ||                              //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==DualSite2x1  ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4 //Steven 20140312 : For HT9045WA
            EL<TCheckBox>("TfSetup", "chkNS7000CS")->Checked)
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 1);
        }
        else if(CUSTOMER_CODE==CC_SIGURD_PeiXing &&
                TestIF_File.iTestMode==_12Site2X6 &&
                EL<TCheckBox>("TfSetup", "chkNS7000CS")->Checked)                                           //Ifor 20170420 (wei) add for 海思 _12Site2X6
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 1);
        }
        else
        {
            WriteIniData(szDir, "Configuration", "NS7000 Change Socket", 0);
        }
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60)                                       //jou 2012-05-15 : 選擇 Y Pitch 機構模式
    {
        WriteIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6000);
    }
    else if(USE_IN_Y_IS_AUTO_PITCH==true || USE_OUT_Y_IS_AUTO_PITCH==true)                                             //Steven 20131002 : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        WriteIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6000);
    }
    else
    {
        if(EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex==iXPitch60)
        {
            WriteIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6000);
        }
        else
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635)
                WriteIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6350);
            else if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)                   //Steven 20140819 : Y-Pitch 36mm
                WriteIniData(szDir, "Configuration", "In Out Arm Y Pitch", 3600);
        }
    }
    WriteIniData(szDir, "Configuration", "bF18InshuttleDetect",        EL<TCheckBox>("TfSetup", "cbF18")->Checked);         //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
    WriteIniData(szDir, "Configuration", "In Arm Use Back Row", EL<TCheckBox>("TfSetup", "cbInUseBackRow")->Checked);       //jou 20161122 (Steven) In & Out arm use back row suck
    WriteIniData(szDir, "Configuration", "Out Arm Use Back Row", EL<TCheckBox>("TfSetup", "cbOutUseBackRow")->Checked);     //jou 20161122 (Steven) In & Out arm use back row suck
    WriteIniData(szDir, "Configuration", "RTCFileName", EL<TEdit>("TfSetup", "edtRTCFileName")->Text);                  //kevin 20200731 RTC FILENAME

    if(CosFunction.b2x4SupportCenterPitch==true &&                              //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
       TestIF_File.iTestMode==_8Site2X4)
    {
        WriteIniData(szDir, "Configuration", "Enable X Center Pitch", EL<TCheckBox>("TfSetup", "chkUseXCenterPitch")->Checked);
        WriteIniData(szDir, "Configuration", "X Center Pitch", EL<TEdit>("TfSetup", "edtXCenterPitch")->Text);
    }
    else
    {
        WriteIniData(szDir, "Configuration", "Enable X Center Pitch", false);
    }

    WriteIniData(szDir, "Configuration", "EnableReadAndCheckTorque",    EL<TCheckBox>("TfSetup", "chkReadTorque")->Checked);    //kevin 20210804 扭力比對by set up file
    WriteIniData(szDir, "Configuration", "dReadTorque",                 EL<TEdit>("TfSetup", "edOverRange")->Text);
    WriteIniData(szDir, "Configuration", "dReadTorqueDelayTime",        EL<TEdit>("TfSetup", "edDelayTime")->Text);
    WriteIniData(szDir, "Configuration", "SendGetValueDelayTime",       EL<TEdit>("TfSetup", "edtGetValueDelayTime")->Text);//KenHsieh 20220317 : 延遲取值時間

//    AddSpace(szDir);
    bSavePressed=true;                                                          //Steven 20110907 : 一定要在ReadFile();之前

    ReadFile();
    FileRW_Cleaning_LoadAutoCleanData();   // golden cSetUp.cpp:4043-4043 取代：golden TfCleaning::LoadAutoCleanData（uCleaning.cpp:65）→ C 路 FileRW/TestIF_File_Cleaning.cpp（替身沒開機就印出來、不跑）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4043-4043 -- golden TfCleaning::LoadAutoCleanData（uCleaning.cpp:65）→ C 路 FileRW/TestIF_File_Cleaning.cpp（替身沒開機就印出來、不跑）
    fCleaning->LoadAutoCleanData();
#endif // GATE (S12-C save)
    ChangeSite();
    EL<TSpeedButton>("TfSetup", "sbUpdate")->Down=false;

    bool bHasChange=false;
    if(IniConfig.bUseAutoSiteMapping)                                           //jou 2011-03-24 start : Auto Site Mapping
    {
        if(IniConfig.bI21EnableASM)                                             //Steven 20110502
        {
            if(iASMTestMode!=TestIF_File.iTestMode)                             //Steven 20120927 : for auto site mapping
                bHasChange=true;

            for(int i=0; i<MAX_SOCKET_ROW; i++)                                 //jou 2014-04-18 Auto site mapping 修正記憶體破壞 MAX_SOCKET_ROW <-> MAX_SOCKET_COL
            {
                for(int j=0; j<MAX_SOCKET_COL; j++)
                {
                    #ifndef ASE_KaohSiung                                       //kevin 20150114 高雄不使用
                        if(iASMSiteMap[i][j]!=TestSiteCH[i][j]->ItemIndex)
                            bHasChange=true;
                    #else
                        if(iASMSiteMap[i][j]==0)                                //kevin 20150114原本是 關site
                            if(TestSiteCH[i][j]->ItemIndex!=0)                  //現在開site
                                bHasChange=true;                                //要做autosite map
                    #endif
                }
            }

            if(CosFunction.bI21EnableASMByRecipe && LastSet.iRunStartMode!=rsmAutoSiteMap)
                bHasChange=true;

            if(bHasChange && HasICUnderMachine()==false)                        //Steven 20210518 : 有IC的狀態下, 不可以開關
            {
                if(TestIF_File.iTestMode==SingleSite)                           //Steven 20130610 : Single Site不做Auto Site Mapping
                    fMain->SetMainRunStartMode(rsmContinuStart);
                else
                    fMain->SetMainRunStartMode(rsmAutoSiteMap);
            }
        }
        else
        {
            if(CosFunction.bI21EnableASMByRecipe && HasICUnderMachine()==false) //Steven 20210518 : 有IC的狀態下, 不可以開關
            {
                fMain->cbRunStartMode->Enabled=true;
            }
        }
    }

    if(HasICUnderMachine()==false)                                              //有開SITE 才恢復畫面關SITE -> 開SITE
    {
        if(CosFunction.bDisableOpenAllSiteWhenChangeShtMod)                     //Jimmychiu 20241203 : Disable opening all sites when changing shuttle mode.
        {
            bool bdiff=VertifyShtModeisDiff(EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked,
                                            EL<TRadioButton>("TfSetup", "rgShtModeOneSide")->Checked,
                                            EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked,
                                            EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked
                                            );
            fMain->ShowTestHeadComp(bdiff);
        }
        else
        {
//            bSiteMapHasChange=false;                                            //Steven 20250102 : 紀錄test mode與site map是否有變更
//            if(iTestMode!=TestIF_File.iTestMode)
//            {
//                bSiteMapHasChange=true;
//                iTestMode=TestIF_File.iTestMode;
//            }

            for(int i=0; i<MAX_SOCKET_ROW; i++)
            {
                for(int j=0; j<MAX_SOCKET_COL; j++)
                {
                    if(iTestSiteCh[i][j]!=TestIF_File.iSiteMap[i][j])
                    {
                        bSiteMapHasChange=true;
                        iTestSiteCh[i][j]=TestIF_File.iSiteMap[i][j];
                    }
                }
            }

            fMain->ShowTestHeadComp(bSiteMapHasChange);                         //kevin 20150401 判斷開關SITE
        }
    }
    else
    {
        fMain->ShowTestHeadComp(false);
    }
    filerw::ELTodo("golden cSetUp.cpp:4129-4129 myLog.Save_SiteStatusLog()：移植樹 handlerlog 沒有這支（handlerlog.h:16-20），且 handlerlog.h 與 language.h 衝突");
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4129-4129 -- myLog.Save_SiteStatusLog()：移植樹 handlerlog 沒有這支（handlerlog.h:16-20），且 handlerlog.h 與 language.h 衝突
    myLog.Save_SiteStatusLog();                                                 //JerryYang 20160622 開關site記錄log
#endif // GATE (S12-C save)

    if(IniConfig.bShuttleMode50)
    {
        if(EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked==false)                                     //Use Signal shuttle
        {
            if(EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked)                                              //shuttle 1
            {
                if(IniConfig.bRemeberAutoHeight==false)                         //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
                {
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&          //Ifor 20190718 Fix Arm1PickPlace Arm2Test Contact Error
                       TestIF_File.bArm1PickPlaceArm2Test==true     )
                    {
                    }
                    else
                    {
                        WriteIniData(sContact, "Test Arm2", "Contact", AnsiString("-50"));  //強制設成-50mm
                    }
                }
    FileRW_Setup_ContactReadFile();   // golden cSetUp.cpp:4148-4148 取代：移植樹 fContact 是 TfContactShim（沒有 ReadFile）→ fContactForm->ReadFile() 轉接（Test Arm Contact 強制 -50 之後重讀）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4148-4148 -- 移植樹 fContact 是 TfContactShim（沒有 ReadFile）→ fContactForm->ReadFile() 轉接（Test Arm Contact 強制 -50 之後重讀）
                fContact->ReadFile();
#endif // GATE (S12-C save)
            }
            else                                                                //shuttle 2
            {
                if(IniConfig.bRemeberAutoHeight==false)                         //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
                    WriteIniData(sContact, "Test Arm1", "Contact", AnsiString("-50")); //JerryYang 20190429 fix contact height被存成"1"的問題
    FileRW_Setup_ContactReadFile();   // golden cSetUp.cpp:4154-4154 取代：同上
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4154-4154 -- 同上
                fContact->ReadFile();
#endif // GATE (S12-C save)
            }
        }
    }

    if(EL<TRadioButton>("TfSetup", "rgShtModeNormal")->Checked==true)
    {
        WriteIniData(sTemp, "ATC", "ATC7CH1Enabled", true);
        WriteIniData(sTemp, "ATC", "ATC7CH2Enabled", true);
        WriteIniData(sTemp, "ATC", "ATC7CH3Enabled", true);
        WriteIniData(sTemp, "ATC", "ATC7CH4Enabled", true);
    }
    else
    {
        if(EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked==true)
        {
            WriteIniData(sTemp, "ATC", "ATC7CH1Enabled", bTestSiteUse[0][0][0]);
            WriteIniData(sTemp, "ATC", "ATC7CH2Enabled", bTestSiteUse[0][0][1]);
        }
        else
        {
            WriteIniData(sTemp, "ATC", "ATC7CH1Enabled", EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked);
            WriteIniData(sTemp, "ATC", "ATC7CH2Enabled", EL<TRadioButton>("TfSetup", "rgUseSht1")->Checked);
        }

        if(EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked==true)
        {
            WriteIniData(sTemp, "ATC", "ATC7CH3Enabled", bTestSiteUse[1][0][0]);
            WriteIniData(sTemp, "ATC", "ATC7CH4Enabled", bTestSiteUse[1][0][1]);
        }
        else
        {
            WriteIniData(sTemp, "ATC", "ATC7CH3Enabled", EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked);
            WriteIniData(sTemp, "ATC", "ATC7CH4Enabled", EL<TRadioButton>("TfSetup", "rgUseSht2")->Checked);
        }
    }
    Temperature.bATC7ChannelEnabled[0] = ReadIniData(sTemp, "ATC", "ATC7CH1Enabled", false);
    Temperature.bATC7ChannelEnabled[1] = ReadIniData(sTemp, "ATC", "ATC7CH2Enabled", false);
    Temperature.bATC7ChannelEnabled[2] = ReadIniData(sTemp, "ATC", "ATC7CH3Enabled", false);
    Temperature.bATC7ChannelEnabled[3] = ReadIniData(sTemp, "ATC", "ATC7CH4Enabled", false);
    AnsiString asData1="", asData2="";
    asData1.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[0], Temperature.bATC7ChannelEnabled[1]);
    asData2.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[2], Temperature.bATC7ChannelEnabled[3]);
    ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, asData1, asData2);

}

// golden cSetUp.cpp:4785  TfSetup::CoSocketComboChange(TObject *Sender)
static void SU_CoSocketComboChange()
{

    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200420 : Socket Sensor功能可以選
    {
        if(i<atoi(EL<TComboBox>("TfSetup", "CoSocketCombo")->Text.c_str()))
        {
            MyTempRGBox[i]->Visible=true;
            if(TestIF_File.bSocketSensorCheckFloating)
                MyTempRGBox[i]->ItemIndex=2;
            else
                MyTempRGBox[i]->ItemIndex=1;
        }
        else
        {
            MyTempRGBox[i]->Visible=false;
            MyTempRGBox[i]->ItemIndex=0;
        }
    }

}

// golden cSetUp.cpp:4200  TfSetup::btnLUpToRDownNClick(TObject *Sender)
static void SU_btnLUpToRDownNClick(TObject *Sender)
{

    if(Barcode_Reader(bcSetup)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        return;
    }

    if(IniConfig.bG09NeedPasswordWhenEditSiteMap==true)                         //Alick 20160603 Site Map 需要密碼
    {
        if(bNeedEnterPassword==true)
        {
    if(filerw::ELPasswordRefused("TfSetup::btnLUpToRDownNClick G09 site-map password (golden fPassword/fQwertyKey)")) bNeedEnterPassword=false;   // golden cSetUp.cpp:4211-4216 取代：G09 Site Map 密碼框（fPassword／fQwertyKey 是表單，伺服器端沒有）→ 視同密碼錯誤（golden 輸錯：bNeedEnterPassword 維持 true）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4211-4216 -- G09 Site Map 密碼框（fPassword／fQwertyKey 是表單，伺服器端沒有）→ 視同密碼錯誤（golden 輸錯：bNeedEnterPassword 維持 true）
            fPassword->edPassword->Text="";
            fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
            if(sSigPassword==fPassword->edPassword->Text)
            {
                bNeedEnterPassword=false;
            }
#endif // GATE (S12-C save)
        }
    }
    else
        bNeedEnterPassword=false;

    if(bNeedEnterPassword==true)
        return;                                                                 //Alick 20161004 add for 矽格SiteMap密碼功能修正

    int iSX, iSY, iCT=1, iDirection;

    TSpeedButton *Ptr=(TSpeedButton *)Sender;
    iDirection=Ptr->Tag;
    if(iDirection>=0 && iDirection<=3)
    {
        iSX=MAX_SOCKET_ROW;
        iSY=MAX_SOCKET_COL;
    }
    else
    {
        iSX=MAX_SOCKET_COL;
        iSY=MAX_SOCKET_ROW;
    }
    for(int i=0; i<iSX; i++)
    {
        for(int j=0; j<iSY; j++)
        {
            if(iDirection==0)                                                   //Left_Top To Right
            {
                if(TestSiteCH[i][j]->Visible && TestSiteCH[i][j]->Enabled)      //Steven 20120813 : 加入'&& TestSiteCH[i][j]->Enabled' for 12Site
                {
                        SU_CbSetIndex(TestSiteCH[i][j], iCT);   // golden cSetUp.cpp:4247-4247 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4247-4247 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[i][j]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
            else if(iDirection==1)                                              //Left_Bottom To Right
            {
                if(TestSiteCH[3-i][j]->Visible && TestSiteCH[i][j]->Enabled)
                {
                        SU_CbSetIndex(TestSiteCH[3-i][j], iCT);   // golden cSetUp.cpp:4255-4255 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4255-4255 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[3-i][j]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
            else if(iDirection==2)                                              //Right_Top To Left
            {
                if(TestSiteCH[i][7-j]->Visible && TestSiteCH[i][j]->Enabled)
                {
                        SU_CbSetIndex(TestSiteCH[i][7-j], iCT);   // golden cSetUp.cpp:4263-4263 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4263-4263 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[i][7-j]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
            else if(iDirection==3)                                              //Right_Bottom To Left
            {
                if(TestSiteCH[3-i][7-j]->Visible && TestSiteCH[i][j]->Enabled)
                {
                        SU_CbSetIndex(TestSiteCH[3-i][7-j], iCT);   // golden cSetUp.cpp:4271-4271 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4271-4271 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[3-i][7-j]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
            else if(iDirection==4)                                              //Left_Top To Bottom
            {
                if(TestSiteCH[j][i]->Visible && TestSiteCH[j][i]->Enabled)
                {
                        SU_CbSetIndex(TestSiteCH[j][i], iCT);   // golden cSetUp.cpp:4279-4279 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4279-4279 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[j][i]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
            else                                                                //Right_Top To Bottom
            {
                if(TestSiteCH[j][7-i]->Visible && TestSiteCH[j][i]->Enabled)
                {
                        SU_CbSetIndex(TestSiteCH[j][7-i], iCT);   // golden cSetUp.cpp:4287-4287 取代：自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
#if 0 // GATE (S12-C save) golden cSetUp.cpp:4287-4287 -- 自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）
                    TestSiteCH[j][7-i]->ItemIndex=iCT;
#endif // GATE (S12-C save)
                    iCT++;
                }
            }
        }
    }

    if(iDirection==0)                                                           //Left_Top To Right
    {                                                                           //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
        IniConfig.iSiteMapDirection=5;
    }
    else if(iDirection==1)                                                      //Left_Bottom To Right
    {
        IniConfig.iSiteMapDirection=10;
    }
    else if(iDirection==2)                                                      //Right_Top To Left
    {
        IniConfig.iSiteMapDirection=9;
    }
    else if(iDirection==3)                                                      //Right_Bottom To Left
    {
        IniConfig.iSiteMapDirection=6;
    }
    else if(iDirection==4)                                                      //Left_Top To Bottom
    {
        IniConfig.iSiteMapDirection=7;
    }
    else if(iDirection==5)                                                      //Right_Top To Bottom
    {
        IniConfig.iSiteMapDirection=11;
    }
    else                                                                        //none
    {
        IniConfig.iSiteMapDirection=0;
    }
    Ptr->Down=false;

}

