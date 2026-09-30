// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Offset_File 的讀寫檔（HTEditList： → Position Offset.Data、Position Offset Hot.Data（shuttle Hand X/Y、Test Arm shuttle 四鍵，LastSet.iTemperature==Hot）、Tri_Position_Offset()（Tri_Temp_Machine：Position Offset cool／Hot／一般）、D:\HT9045\data\<recipe>.ini（CC_ASE_CL scale））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cOffSet.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 300 個（header 宣告 379 個）。
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
#include "forms/fOffSet.h"
#include "csystem.h"
#include "cinitial.h"
#include "BarcodeReader.h"
#include "canary_support.h"
#include "FileRW/_EditPage.h"   // AI(W906-FRW-S157) 20260927 [W906]：filerw::PageEvent（檔尾 kOS_Events）

extern bool authMainForm[12];            // cAuthority.h:56（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）
void GetLimitAuth();                     // cAuthority.h:77
bool FileRW_Offset_IsRun2DCheck();       // FileRW/Offset_File_C.cpp 下半（fContactForm->IsRun2DCheck()）
void FileRW_ArmSuckDims(int which, int* row, int* col);   // FileRW/_KitSuck.cpp（0＝InArmSuck，1＝OutArmSuck）
bool FileRW_ArmSuckHasIC(int which);                      // FileRW/_KitSuck.cpp
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129（不 include cMyDB.h，同 FileRW/_EditPage.cpp:968）  //AI(W906-EVB10A) 20260929 [W906]：OS-6 FormClose 用
// golden TMyKitSuck 用到的三樣（iMotRow／iMotCol／HasIC()）的轉接物件：InArmSuck／OutArmSuck 巨集展開成它
struct OS_Suck { int which, iMotRow, iMotCol; bool HasIC() const { return FileRW_ArmSuckHasIC(which); } };
static inline OS_Suck OS_SuckOf(int w) { OS_Suck s; s.which=w; FileRW_ArmSuckDims(w, &s.iMotRow, &s.iMotCol); return s; }

// golden cOffSet.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static const int iSaveStander=1;     // golden cOffSet.cpp:172
static const int iSaveSpecial=2;     // golden cOffSet.cpp:173
static const int iSaveAll=3;         // golden cOffSet.cpp:174
static TSpeedButton *OffSetSelBot[OfsTotal];          // golden cOffSet.h:452
static TSpeedButton *OfffSetSpecSelBot[trayOfsTotal]; // golden cOffSet.h:453
static TEdit *MyPickEdit[2][8];                       // golden cOffSet.h:454
static TEdit *MyRelsEdit[2][8];                       // golden cOffSet.h:455
static TEdit *MySingleOffsetTEditX[2][8];             // golden cOffSet.h:458
static TEdit *MySingleOffsetTEditY[2][8];             // golden cOffSet.h:459
static TLabel *MyPickLab[2][8];                       // golden cOffSet.h:460
static TLabel *MyRelsLab[2][8];                       // golden cOffSet.h:461
static TLabel *MySingleOffsetLabX[2][8];              // golden cOffSet.h:462
static TLabel *MySingleOffsetLabY[2][8];              // golden cOffSet.h:463
static bool bUseUpdate=false;                         // golden cOffSet.h:465
static double dTemp=0;                                // golden cOffSet.h:470
static bool fShow=false;                              // golden cOffSet.h:475（本 TU 自己的：頁面沒有關表單事件）
static int iNowOffsetSel=-1;                          // golden cOffSet.h:476（建構子 :371 設 -1）
static bool bEnterSpecialOffset=false;                // golden cOffSet.h:477
#define LastFileName (fOffSet->LastFileName)   // golden cOffSet.h:478 —— 移植樹 GetOffsetPath／ReadFile 用 fOffSet 那一份
static int iSpecialOffSetSel=-1;                      // golden cOffSet.h:479（建構子 :372 設 -1）
static int iIndexChange=0;                            // golden cOffSet.h:480
static int iOffsetMap[OfsTotal];                      // golden cOffSet.h:481
#define GetOffsetPath fOffSet->GetOffsetPath   // golden :1430 —— 移植樹 cOffSet.cpp:188（live）
#define Tri_Position_Offset fOffSet->Tri_Position_Offset   // golden :3530 —— 移植樹 forms/fOffSet.cpp:381（live）
static bool OS_bSkipReadFile=false;   // 開頁逐一切換選取時略過按鈕事件裡的 ReadFile（檔案沒變；見檔頭）
static bool OS_bSelRefused=false;     // SpBotSelClick 被 KYEC barcode 拒（golden return）
static int  OS_iActivePage=0;         // spbSaveClick 的 PageControl1->ActivePageIndex（0＝In Out Arm，1＝Index／Tray）
static bool OS_bPartOnly=false;       // spbSaveClick 只跑到 SaveFile（多組存檔的前面幾組）
static bool OS_bTailOnly=false;       // spbSaveClick 跳過 SaveFile、只跑尾端（ReadFile 起，全部組存完後一次）
static bool OS_bSavePrev=false;       // SpBotSelClick 照 golden :2487 先存上一組（只有 form.event 的部位鈕設；R127 照 BCB，//AI(W906-D013) 20260929）
static void OS_PortReadFile() { if(!OS_bSkipReadFile) fOffSet->ReadFile(); }   // golden ReadFile() → 移植樹 fOffSet->ReadFile()（cOffSet.cpp:367）
#define ReadFile OS_PortReadFile   // FileRW/Offset_File_C.cpp 在 include 其他 header 之前 #undef
#define InArmSuck OS_SuckOf(0)    // golden InArmSuck.iMotRow／iMotCol／HasIC() → FileRW/_KitSuck.cpp
#define OutArmSuck OS_SuckOf(1)   // golden OutArmSuck.…（同上）

using filerw::EL;

// 前置宣告
static void OS_TfOffSet();
static void OS_LoadImage(bool bInArm);
static void OS_FormShow();
static void OS_SetXYPitchVCLVisible(bool bVisible);
static void OS_ShowOneByOneOffSet();
static bool OS_SaveFile(int iSelPartData, int SpecialMode, bool bReset);
static bool OS_SaveSetupFile(AnsiString szDir, AnsiString szFilePath, int iSelPartData, int SpecialMode, bool bReset);
static void OS_DoIniDataToForm(int iNowOffsetSel, int SpecialMode);
static void OS_SpBotSelClick(int iPtrTag);
static void OS_IndexOffSetBT2Click(int iPtrTag);
static void OS_spbSaveClick();
static void OS_Timer1Timer();
static void OS_ShowFinalAirForce();
static void OS_btnBackClick();
static void OS_ShowOffSetList();
static void OS_sb_AutoOffsetUpClick();
static void OS_sb_AutoOffsetDownClick();
static void OS_sb_AutoOffsetRightClick();
static void OS_sb_AutoOffsetLeftClick();
static void OS_FormClose();

// golden cOffSet.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void OS_DfmItems()
{
}

// golden cOffSet.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kOS_ParentOf[][2] = {
    {"CheckBox1", "Panel6"},
    {"EditPickA", "pnlPicker"},
    {"EditPickB", "pnlPicker"},
    {"EditPickC", "pnlPicker"},
    {"EditPickD", "pnlPicker"},
    {"EditPickE", "pnlPicker"},
    {"EditPickF", "pnlPicker"},
    {"EditPickG", "pnlPicker"},
    {"EditPickH", "pnlPicker"},
    {"EditPickI", "pnlPicker"},
    {"EditPickJ", "pnlPicker"},
    {"EditPickK", "pnlPicker"},
    {"EditPickL", "pnlPicker"},
    {"EditPickM", "pnlPicker"},
    {"EditPickN", "pnlPicker"},
    {"EditPickO", "pnlPicker"},
    {"EditPickP", "pnlPicker"},
    {"EdtOffsetAX", "pnlPicker"},
    {"EdtOffsetAY", "pnlPicker"},
    {"EdtOffsetBX", "pnlPicker"},
    {"EdtOffsetBY", "pnlPicker"},
    {"EdtOffsetCX", "pnlPicker"},
    {"EdtOffsetCY", "pnlPicker"},
    {"EdtOffsetDX", "pnlPicker"},
    {"EdtOffsetDY", "pnlPicker"},
    {"EdtOffsetEX", "pnlPicker"},
    {"EdtOffsetEY", "pnlPicker"},
    {"EdtOffsetFX", "pnlPicker"},
    {"EdtOffsetFY", "pnlPicker"},
    {"EdtOffsetGX", "pnlPicker"},
    {"EdtOffsetGY", "pnlPicker"},
    {"EdtOffsetHX", "pnlPicker"},
    {"EdtOffsetHY", "pnlPicker"},
    {"EdtOffsetIX", "pnlPicker"},
    {"EdtOffsetIY", "pnlPicker"},
    {"EdtOffsetJX", "pnlPicker"},
    {"EdtOffsetJY", "pnlPicker"},
    {"EdtOffsetKX", "pnlPicker"},
    {"EdtOffsetKY", "pnlPicker"},
    {"EdtOffsetLX", "pnlPicker"},
    {"EdtOffsetLY", "pnlPicker"},
    {"EdtOffsetMX", "pnlPicker"},
    {"EdtOffsetMY", "pnlPicker"},
    {"EdtOffsetNX", "pnlPicker"},
    {"EdtOffsetNY", "pnlPicker"},
    {"EdtOffsetOX", "pnlPicker"},
    {"EdtOffsetOY", "pnlPicker"},
    {"EdtOffsetPX", "pnlPicker"},
    {"EdtOffsetPY", "pnlPicker"},
    {"EdtRelsA", "pnlPicker"},
    {"EdtRelsB", "pnlPicker"},
    {"EdtRelsC", "pnlPicker"},
    {"EdtRelsD", "pnlPicker"},
    {"EdtRelsE", "pnlPicker"},
    {"EdtRelsF", "pnlPicker"},
    {"EdtRelsG", "pnlPicker"},
    {"EdtRelsH", "pnlPicker"},
    {"EdtRelsI", "pnlPicker"},
    {"EdtRelsJ", "pnlPicker"},
    {"EdtRelsK", "pnlPicker"},
    {"EdtRelsL", "pnlPicker"},
    {"EdtRelsM", "pnlPicker"},
    {"EdtRelsN", "pnlPicker"},
    {"EdtRelsO", "pnlPicker"},
    {"EdtRelsP", "pnlPicker"},
    {"GroupBox2", "grpAutoClean"},
    {"GroupBox4", "grpAutoClean"},
    {"GroupBox5", "tsScale"},
    {"GroupBox6", "GroupBox5"},
    {"GroupBox7", "GroupBox5"},
    {"GroupBox8", "GroupBox5"},
    {"Image1", "pnlMain"},
    {"Image2", "pnlPicker"},
    {"Image3", "pnlIndex"},
    {"IndexArmOffSet1", "Panel3"},
    {"IndexArmOffSet2", "Panel3"},
    {"IndexArmOffSet3", "Panel3"},
    {"IndexArmOffSet4", "Panel3"},
    {"IndexArmOffSet5", "Panel3"},
    {"IndexArmOffSet6", "Panel3"},
    {"IndexOffSetBT1", "pnlIndex"},
    {"IndexOffSetBT2", "pnlIndex"},
    {"LabOffsetAX", "pnlPicker"},
    {"LabOffsetAY", "pnlPicker"},
    {"LabOffsetBX", "pnlPicker"},
    {"LabOffsetBY", "pnlPicker"},
    {"LabOffsetCX", "pnlPicker"},
    {"LabOffsetCY", "pnlPicker"},
    {"LabOffsetDX", "pnlPicker"},
    {"LabOffsetDY", "pnlPicker"},
    {"LabOffsetEX", "pnlPicker"},
    {"LabOffsetEY", "pnlPicker"},
    {"LabOffsetFX", "pnlPicker"},
    {"LabOffsetFY", "pnlPicker"},
    {"LabOffsetGX", "pnlPicker"},
    {"LabOffsetGY", "pnlPicker"},
    {"LabOffsetHX", "pnlPicker"},
    {"LabOffsetHY", "pnlPicker"},
    {"LabOffsetIX", "pnlPicker"},
    {"LabOffsetIY", "pnlPicker"},
    {"LabOffsetJX", "pnlPicker"},
    {"LabOffsetJY", "pnlPicker"},
    {"LabOffsetKX", "pnlPicker"},
    {"LabOffsetKY", "pnlPicker"},
    {"LabOffsetLX", "pnlPicker"},
    {"LabOffsetLY", "pnlPicker"},
    {"LabOffsetMX", "pnlPicker"},
    {"LabOffsetMY", "pnlPicker"},
    {"LabOffsetNX", "pnlPicker"},
    {"LabOffsetNY", "pnlPicker"},
    {"LabOffsetOX", "pnlPicker"},
    {"LabOffsetOY", "pnlPicker"},
    {"LabOffsetPX", "pnlPicker"},
    {"LabOffsetPY", "pnlPicker"},
    {"LabPickA", "pnlPicker"},
    {"LabPickB", "pnlPicker"},
    {"LabPickC", "pnlPicker"},
    {"LabPickD", "pnlPicker"},
    {"LabPickE", "pnlPicker"},
    {"LabPickF", "pnlPicker"},
    {"LabPickG", "pnlPicker"},
    {"LabPickH", "pnlPicker"},
    {"LabPickI", "pnlPicker"},
    {"LabPickJ", "pnlPicker"},
    {"LabPickK", "pnlPicker"},
    {"LabPickL", "pnlPicker"},
    {"LabPickM", "pnlPicker"},
    {"LabPickN", "pnlPicker"},
    {"LabPickO", "pnlPicker"},
    {"LabPickP", "pnlPicker"},
    {"LabRelsA", "pnlPicker"},
    {"LabRelsB", "pnlPicker"},
    {"LabRelsC", "pnlPicker"},
    {"LabRelsD", "pnlPicker"},
    {"LabRelsE", "pnlPicker"},
    {"LabRelsF", "pnlPicker"},
    {"LabRelsG", "pnlPicker"},
    {"LabRelsH", "pnlPicker"},
    {"LabRelsI", "pnlPicker"},
    {"LabRelsJ", "pnlPicker"},
    {"LabRelsK", "pnlPicker"},
    {"LabRelsL", "pnlPicker"},
    {"LabRelsM", "pnlPicker"},
    {"LabRelsN", "pnlPicker"},
    {"LabRelsO", "pnlPicker"},
    {"LabRelsP", "pnlPicker"},
    {"Panel2", "pnlIndexOfs"},
    {"Panel3", "pnlIndexOfs"},
    {"Panel6", "pnlPicker"},
    {"Pnl_AOI", "tsInOutArmOffset"},
    {"Pnl_ScanAOI", "pnlMain"},
    {"btnBack", "tsArmOffset"},
    {"btnBottom2D", "pnlMain"},
    {"btnOffsetList", "pnlMain"},
    {"btnSortAuto1", "grpOutArm"},
    {"btnSortAuto2", "grpOutArm"},
    {"btnSortAuto3", "grpOutArm"},
    {"btnSortAuto6", "grpOutArm456"},
    {"btnSortFix1", "grpOutArm"},
    {"btnSortFix2", "grpOutArm"},
    {"btnSortFix3", "grpOutArm"},
    {"btnTrayOfsAuto1", "pnlIndex"},
    {"btnTrayOfsAuto2", "pnlIndex"},
    {"btnTrayOfsAuto3", "pnlIndex"},
    {"btnTrayOfsAuto4", "pnlIndex"},
    {"btnTrayOfsAuto5", "pnlIndex"},
    {"btnTrayOfsAuto6", "pnlIndex"},
    {"btnTrayOfsColor", "pnlIndex"},
    {"btnTrayOfsEmpty", "pnlIndex"},
    {"btnTrayOfsLoader", "pnlIndex"},
    {"cb_AutoOffsetPositionCheck", "pan_AutoOffsetMove"},
    {"chkInArmScakeEnable", "tsScale"},
    {"chkOutArmScakeEnable", "palE32"},
    {"ck_AOI", "Pnl_AOI"},
    {"edArmX", "Panel6"},
    {"edArmY", "Panel6"},
    {"edLodX", "GroupBox6"},
    {"edLodY", "GroupBox6"},
    {"edOffsetContactForce", "pnlOEE_Contact"},
    {"edPickUp", "Panel6"},
    {"edPitchX1", "Panel6"},
    {"edPitchX2", "Panel6"},
    {"edPitchX3", "Panel6"},
    {"edPitchX4", "Panel6"},
    {"edPitchY", "Panel6"},
    {"edPreciserClose", "palPreciser"},
    {"edPreciserOpen", "palPreciser"},
    {"edRelease", "Panel6"},
    {"edShtFor2D", "Panel3"},
    {"ed_IS1X", "gbE32_InSh1"},
    {"ed_IS1Y", "gbE32_InSh1"},
    {"ed_IS2X", "gbE32_InSh2"},
    {"ed_IS2Y", "gbE32_InSh2"},
    {"ed_OS1X", "gbE32_OutSh1"},
    {"ed_OS1Y", "gbE32_OutSh1"},
    {"ed_OS2X", "gbE32_OutSh2"},
    {"ed_OS2Y", "gbE32_OutSh2"},
    {"edlLoadZ", "Panel2"},
    {"edtARTPlace", "pnlTrayXART"},
    {"edtTrayArmX", "Panel2"},
    {"edt_HP1X", "GroupBox7"},
    {"edt_HP1Y", "GroupBox7"},
    {"edt_HP2X", "GroupBox8"},
    {"edt_HP2Y", "GroupBox8"},
    {"gbE32_InSh1", "tsScale"},
    {"gbE32_InSh2", "tsScale"},
    {"gbE32_OutSh1", "palE32"},
    {"gbE32_OutSh2", "palE32"},
    {"grpAutoClean", "pnlMain"},
    {"grpOutArm", "grpSetupTeach"},
    {"grpOutArm456", "grpOutArm"},
    {"grpSetupTeach", "tsSetupTeach"},
    {"labWarningForStop", "Panel5"},
    {"labWarningForZ", "pnlPicker"},
    {"lbFinalAirForce", "pnlOEE_Contact"},
    {"lblArmXRange", "Panel6"},
    {"lblArmYRange", "Panel6"},
    {"lblLoadZ", "Panel2"},
    {"lblOutArm", "pnlPicker"},
    {"lblPickUp", "Panel6"},
    {"lblPickUpRange", "Panel6"},
    {"lblPitchX1", "Panel6"},
    {"lblPitchX1Range", "Panel6"},
    {"lblPitchX2", "Panel6"},
    {"lblPitchX2Range", "Panel6"},
    {"lblPitchX3", "Panel6"},
    {"lblPitchX4", "Panel6"},
    {"lblPitchY", "Panel6"},
    {"lblPitchYRange", "Panel6"},
    {"lblRelease", "Panel6"},
    {"lblReleaseRange", "Panel6"},
    {"lblShowMessage", "pnlIndexOfs"},
    {"lblShtLeft", "Panel3"},
    {"lblShtRight", "Panel3"},
    {"palAuto6", "grpOutArm456"},
    {"palE32", "tsScale"},
    {"palOffsetParts", "pnlPicker"},
    {"palPreciser", "Panel6"},
    {"pan_AutoOffsetMove", "Panel6"},
    {"pnlIndex", "tsIndexOffset"},
    {"pnlIndexOffset", "pnlIndexOfs"},
    {"pnlIndexOfs", "tsIndexOffset"},
    {"pnlMain", "tsInOutArmOffset"},
    {"pnlOEE_Contact", "pnlIndexOfs"},
    {"pnlPicker", "tsInOutArmOffset"},
    {"pnlTrayXART", "pnlIndexOfs"},
    {"sbAuto1", "pnlMain"},
    {"sbAuto2", "pnlMain"},
    {"sbAuto3", "pnlMain"},
    {"sbAuto4", "pnlMain"},
    {"sbAuto5", "pnlMain"},
    {"sbAuto6", "pnlMain"},
    {"sbAutoClean", "pnlMain"},
    {"sbAutoSh1", "pnlMain"},
    {"sbAutoSh2", "pnlMain"},
    {"sbBGAView", "Pnl_AOI"},
    {"sbFix1", "pnlMain"},
    {"sbFix2", "pnlMain"},
    {"sbFix3", "pnlMain"},
    {"sbFix4", "pnlMain"},
    {"sbFix5", "pnlMain"},
    {"sbFix6", "pnlMain"},
    {"sbHp1", "pnlMain"},
    {"sbHp2", "pnlMain"},
    {"sbInArmZCalibration", "tsZCalibration"},
    {"sbInPlacement", "pnlMain"},
    {"sbInRotate", "pnlMain"},
    {"sbInSh1", "pnlMain"},
    {"sbInSh1LB", "pnlMain"},
    {"sbInSh1LB_AutoClean", "GroupBox2"},
    {"sbInSh1RA", "pnlMain"},
    {"sbInSh1RA_AutoClean", "GroupBox2"},
    {"sbInSh1RB", "pnlMain"},
    {"sbInSh1RB_AutoClean", "GroupBox2"},
    {"sbInSh1_AutoClean", "GroupBox2"},
    {"sbInSh2", "pnlMain"},
    {"sbInSh2LB", "pnlMain"},
    {"sbInSh2LB_AutoClean", "GroupBox4"},
    {"sbInSh2RA", "pnlMain"},
    {"sbInSh2RA_AutoClean", "GroupBox4"},
    {"sbInSh2RB", "pnlMain"},
    {"sbInSh2RB_AutoClean", "GroupBox4"},
    {"sbInSh2_AutoClean", "GroupBox4"},
    {"sbLoader", "pnlMain"},
    {"sbLoaderB", "pnlMain"},
    {"sbOCR", "pnlMain"},
    {"sbOutArmZCalibration", "tsZCalibration"},
    {"sbOutRotate", "pnlMain"},
    {"sbOutSh1", "pnlMain"},
    {"sbOutSh1LB", "pnlMain"},
    {"sbOutSh1RA", "pnlMain"},
    {"sbOutSh1RB", "pnlMain"},
    {"sbOutSh2", "pnlMain"},
    {"sbOutSh2LB", "pnlMain"},
    {"sbOutSh2RA", "pnlMain"},
    {"sbOutSh2RB", "pnlMain"},
    {"sbPADView", "Pnl_AOI"},
    {"sbPreciser", "pnlMain"},
    {"sbScanAOI", "Pnl_ScanAOI"},
    {"sbTopView", "pnlMain"},
    {"sbZcalibration", "pnlMain"},
    {"sgOffsetList", "tsArmOffset"},
    {"sg_Offset_HotTempShiftOffset", "ts_Offset_HotTempShiftOffset"},
    {"spbSave", "Panel5"},
    {"tsArmOffset", "PageControl1"},
    {"tsInOutArmOffset", "PageControl1"},
    {"tsIndexOffset", "PageControl1"},
    {"tsScale", "PageControl1"},
    {"tsSetupTeach", "PageControl1"},
    {"tsZCalibration", "PageControl1"},
    {"ts_Offset_HotTempShiftOffset", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void OS_CreateContainerProxies()
{
    EL<TGroupBox>("TfOffSet", "GroupBox2");
    EL<TGroupBox>("TfOffSet", "GroupBox4");
    EL<TGroupBox>("TfOffSet", "GroupBox5");
    EL<TGroupBox>("TfOffSet", "GroupBox6");
    EL<TGroupBox>("TfOffSet", "GroupBox7");
    EL<TGroupBox>("TfOffSet", "GroupBox8");
    EL<TPageControl>("TfOffSet", "PageControl1");
    EL<TPanel>("TfOffSet", "Panel2");
    EL<TPanel>("TfOffSet", "Panel3");
    EL<TPanel>("TfOffSet", "Panel5");
    EL<TPanel>("TfOffSet", "Panel6");
    EL<TPanel>("TfOffSet", "Pnl_AOI");
    EL<TPanel>("TfOffSet", "Pnl_ScanAOI");
    EL<TGroupBox>("TfOffSet", "gbE32_InSh1");
    EL<TGroupBox>("TfOffSet", "gbE32_InSh2");
    EL<TGroupBox>("TfOffSet", "gbE32_OutSh1");
    EL<TGroupBox>("TfOffSet", "gbE32_OutSh2");
    EL<TGroupBox>("TfOffSet", "grpAutoClean");
    EL<TGroupBox>("TfOffSet", "grpOutArm");
    EL<TPanel>("TfOffSet", "grpOutArm456");
    EL<TGroupBox>("TfOffSet", "grpSetupTeach");
    EL<TPanel>("TfOffSet", "palE32");
    EL<TPanel>("TfOffSet", "palPreciser");
    EL<TPanel>("TfOffSet", "pan_AutoOffsetMove");
    EL<TPanel>("TfOffSet", "pnlIndex");
    EL<TPanel>("TfOffSet", "pnlIndexOfs");
    EL<TPanel>("TfOffSet", "pnlMain");
    EL<TPanel>("TfOffSet", "pnlOEE_Contact");
    EL<TPanel>("TfOffSet", "pnlPicker");
    EL<TPanel>("TfOffSet", "pnlTrayXART");
    EL<TTabSheet>("TfOffSet", "tsArmOffset");
    EL<TTabSheet>("TfOffSet", "tsInOutArmOffset");
    EL<TTabSheet>("TfOffSet", "tsIndexOffset");
    EL<TTabSheet>("TfOffSet", "tsScale");
    EL<TTabSheet>("TfOffSet", "tsSetupTeach");
    EL<TTabSheet>("TfOffSet", "tsZCalibration");
    EL<TTabSheet>("TfOffSet", "ts_Offset_HotTempShiftOffset");
    filerw::ELSetParents("TfOffSet", kOS_ParentOf, (int)(sizeof(kOS_ParentOf) / sizeof(kOS_ParentOf[0])));
}

// golden cOffSet.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void OS_DfmState()
{
    EL<TCheckBox>("TfOffSet", "CheckBox1")->Visible = false;
    EL<TControl>("TfOffSet", "Timer1")->Enabled = false;
    EL<TControl>("TfOffSet", "TimerSetupTeach")->Enabled = false;
    EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible = false;
    EL<TLabel>("TfOffSet", "labWarningForStop")->Visible = false;
    EL<TLabel>("TfOffSet", "labWarningForZ")->Visible = false;
    EL<TLabel>("TfOffSet", "lblOutArm")->Visible = false;
    EL<TPanel>("TfOffSet", "pan_AutoOffsetMove")->Visible = false;
    EL<TSpeedButton>("TfOffSet", "sbInArmZCalibration")->Visible = false;
    EL<TSpeedButton>("TfOffSet", "sbOutArmZCalibration")->Visible = false;
    EL<TSpeedButton>("TfOffSet", "sbTopView")->Visible = false;
    EL<TEdit>("TfOffSet", "edLodX")->Text = "edE30_LodX";
    EL<TEdit>("TfOffSet", "edLodY")->Text = "edE30_LodY";
    EL<TEdit>("TfOffSet", "ed_IS1X")->Text = "ed_IS1X";
    EL<TEdit>("TfOffSet", "ed_IS1Y")->Text = "ed_IS1Y";
    EL<TEdit>("TfOffSet", "ed_IS2X")->Text = "ed_IS2X";
    EL<TEdit>("TfOffSet", "ed_IS2Y")->Text = "ed_IS2Y";
    EL<TEdit>("TfOffSet", "ed_OS1X")->Text = "ed_OS1X";
    EL<TEdit>("TfOffSet", "ed_OS1Y")->Text = "ed_OS1Y";
    EL<TEdit>("TfOffSet", "ed_OS2X")->Text = "ed_OS2X";
    EL<TEdit>("TfOffSet", "ed_OS2Y")->Text = "ed_OS2Y";
    EL<TEdit>("TfOffSet", "edt_HP1X")->Text = "edE30_HP1X";
    EL<TEdit>("TfOffSet", "edt_HP1Y")->Text = "edE30_HP1Y";
    EL<TEdit>("TfOffSet", "edt_HP2X")->Text = "edE30_HP2X";
    EL<TEdit>("TfOffSet", "edt_HP2Y")->Text = "edE30_HP2Y";
    EL<TSpeedButton>("TfOffSet", "sb_AutoOffsetDown")->Tag = 2;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
    EL<TSpeedButton>("TfOffSet", "sb_AutoOffsetRight")->Tag = 1;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
    EL<TSpeedButton>("TfOffSet", "sb_AutoOffsetUp")->Tag = 3;   // AI(W906-FRW-S157)：golden DFM 設計期 Tag（form.event 處理器讀 Sender->Tag）
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kOS_EnableAll[] = {
    "CheckBox1", "GroupBox2", "GroupBox4", "GroupBox5", "GroupBox6", "GroupBox7",
    "GroupBox8", "IndexOffSetBT1", "IndexOffSetBT2", "LabOffsetAX", "LabOffsetAY", "LabOffsetBX",
    "LabOffsetBY", "LabOffsetCX", "LabOffsetCY", "LabOffsetDX", "LabOffsetDY", "LabOffsetEX",
    "LabOffsetEY", "LabOffsetFX", "LabOffsetFY", "LabOffsetGX", "LabOffsetGY", "LabOffsetHX",
    "LabOffsetHY", "LabOffsetIX", "LabOffsetIY", "LabOffsetJX", "LabOffsetJY", "LabOffsetKX",
    "LabOffsetKY", "LabOffsetLX", "LabOffsetLY", "LabOffsetMX", "LabOffsetMY", "LabOffsetNX",
    "LabOffsetNY", "LabOffsetOX", "LabOffsetOY", "LabOffsetPX", "LabOffsetPY", "LabPickA",
    "LabPickB", "LabPickC", "LabPickD", "LabPickE", "LabPickF", "LabPickG",
    "LabPickH", "LabPickI", "LabPickJ", "LabPickK", "LabPickL", "LabPickM",
    "LabPickN", "LabPickO", "LabPickP", "LabRelsA", "LabRelsB", "LabRelsC",
    "LabRelsD", "LabRelsE", "LabRelsF", "LabRelsG", "LabRelsH", "LabRelsI",
    "LabRelsJ", "LabRelsK", "LabRelsL", "LabRelsM", "LabRelsN", "LabRelsO",
    "LabRelsP", "PageControl1", "Panel2", "Panel3", "Panel5", "Panel6",
    "Pnl_AOI", "Pnl_ScanAOI", "btnBack", "btnBottom2D", "btnOffsetList", "btnSortAuto1",
    "btnSortAuto2", "btnSortAuto3", "btnSortAuto6", "btnSortFix1", "btnSortFix2", "btnSortFix3",
    "btnTrayOfsAuto1", "btnTrayOfsAuto2", "btnTrayOfsAuto3", "btnTrayOfsAuto4", "btnTrayOfsAuto5", "btnTrayOfsAuto6",
    "btnTrayOfsColor", "btnTrayOfsEmpty", "btnTrayOfsLoader", "cb_AutoOffsetPositionCheck", "chkInArmScakeEnable", "chkOutArmScakeEnable",
    "ck_AOI", "gbE32_InSh1", "gbE32_InSh2", "gbE32_OutSh1", "gbE32_OutSh2", "grpAutoClean",
    "grpOutArm", "grpOutArm456", "grpSetupTeach", "labWarningForStop", "labWarningForZ", "lbFinalAirForce",
    "lblArmXRange", "lblArmYRange", "lblLoadZ", "lblOutArm", "lblPickUp", "lblPickUpRange",
    "lblPitchX1", "lblPitchX1Range", "lblPitchX2", "lblPitchX2Range", "lblPitchX3", "lblPitchX4",
    "lblPitchY", "lblPitchYRange", "lblRelease", "lblReleaseRange", "lblShowMessage", "lblShtLeft",
    "lblShtRight", "palAuto6", "palE32", "palOffsetParts", "palPreciser", "pan_AutoOffsetMove",
    "pnlIndex", "pnlIndexOffset", "pnlIndexOfs", "pnlMain", "pnlOEE_Contact", "pnlPicker",
    "pnlTrayXART", "sbAuto1", "sbAuto2", "sbAuto3", "sbAuto4", "sbAuto5",
    "sbAuto6", "sbAutoClean", "sbAutoSh1", "sbAutoSh2", "sbBGAView", "sbFix1",
    "sbFix2", "sbFix3", "sbFix4", "sbFix5", "sbFix6", "sbHp1",
    "sbHp2", "sbInArmZCalibration", "sbInPlacement", "sbInRotate", "sbInSh1", "sbInSh1LB",
    "sbInSh1LB_AutoClean", "sbInSh1RA", "sbInSh1RA_AutoClean", "sbInSh1RB", "sbInSh1RB_AutoClean", "sbInSh1_AutoClean",
    "sbInSh2", "sbInSh2LB", "sbInSh2LB_AutoClean", "sbInSh2RA", "sbInSh2RA_AutoClean", "sbInSh2RB",
    "sbInSh2RB_AutoClean", "sbInSh2_AutoClean", "sbLoader", "sbLoaderB", "sbOCR", "sbOutArmZCalibration",
    "sbOutRotate", "sbOutSh1", "sbOutSh1LB", "sbOutSh1RA", "sbOutSh1RB", "sbOutSh2",
    "sbOutSh2LB", "sbOutSh2RA", "sbOutSh2RB", "sbPADView", "sbPreciser", "sbScanAOI",
    "sbTopView", "sbZcalibration", "spbSave", "tsArmOffset", "tsInOutArmOffset", "tsIndexOffset",
    "tsScale", "tsSetupTeach", "tsZCalibration", "ts_Offset_HotTempShiftOffset",
};

// 存檔流程（spbSaveClick、SaveFile、SaveSetupFile）讀的替身 31 個 —— FileRW/Offset_File.cpp 用它決定頁面必須送哪些值
static const char* const kOS_SaveReads[] = {
    "IndexArmOffSet1", "IndexArmOffSet2", "IndexArmOffSet3", "IndexArmOffSet4", "IndexArmOffSet5", "IndexArmOffSet6",
    "chkInArmScakeEnable", "chkOutArmScakeEnable", "edArmX", "edArmY", "edPickUp", "edPitchX1",
    "edPitchX2", "edPitchX3", "edPitchX4", "edPitchY", "edPreciserClose", "edPreciserOpen",
    "edRelease", "edShtFor2D", "ed_IS1X", "ed_IS1Y", "ed_IS2X", "ed_IS2Y",
    "ed_OS1X", "ed_OS1Y", "ed_OS2X", "ed_OS2Y", "edlLoadZ", "edtARTPlace",
    "edtTrayArmX",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void OS_CreateSaveProxies()
{
    EL<TEdit>("TfOffSet", "IndexArmOffSet1");
    EL<TEdit>("TfOffSet", "IndexArmOffSet2");
    EL<TEdit>("TfOffSet", "IndexArmOffSet3");
    EL<TEdit>("TfOffSet", "IndexArmOffSet4");
    EL<TEdit>("TfOffSet", "IndexArmOffSet5");
    EL<TEdit>("TfOffSet", "IndexArmOffSet6");
    EL<TCheckBox>("TfOffSet", "chkInArmScakeEnable");
    EL<TCheckBox>("TfOffSet", "chkOutArmScakeEnable");
    EL<TEdit>("TfOffSet", "edArmX");
    EL<TEdit>("TfOffSet", "edArmY");
    EL<TEdit>("TfOffSet", "edPickUp");
    EL<TEdit>("TfOffSet", "edPitchX1");
    EL<TEdit>("TfOffSet", "edPitchX2");
    EL<TEdit>("TfOffSet", "edPitchX3");
    EL<TEdit>("TfOffSet", "edPitchX4");
    EL<TEdit>("TfOffSet", "edPitchY");
    EL<TEdit>("TfOffSet", "edPreciserClose");
    EL<TEdit>("TfOffSet", "edPreciserOpen");
    EL<TEdit>("TfOffSet", "edRelease");
    EL<TEdit>("TfOffSet", "edShtFor2D");
    EL<TEdit>("TfOffSet", "ed_IS1X");
    EL<TEdit>("TfOffSet", "ed_IS1Y");
    EL<TEdit>("TfOffSet", "ed_IS2X");
    EL<TEdit>("TfOffSet", "ed_IS2Y");
    EL<TEdit>("TfOffSet", "ed_OS1X");
    EL<TEdit>("TfOffSet", "ed_OS1Y");
    EL<TEdit>("TfOffSet", "ed_OS2X");
    EL<TEdit>("TfOffSet", "ed_OS2Y");
    EL<TEdit>("TfOffSet", "edlLoadZ");
    EL<TEdit>("TfOffSet", "edtARTPlace");
    EL<TEdit>("TfOffSet", "edtTrayArmX");
}

// golden cOffSet.cpp:186  TfOffSet::TfOffSet(TComponent* Owner)
static void OS_TfOffSet()
{

    EL<TPageControl>("TfOffSet", "PageControl1")->ActivePageIndex   =1;                                         //Steven 20120706 : 調整畫面
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfOffSet", "PageControl1")->ActivePage        =EL<TTabSheet>("TfOffSet", "tsIndexOffset");
//    tsInOutArmOffset->TabVisible    =false;                                     //Steven 20120706 : 調整畫面
    EL<TTabSheet>("TfOffSet", "tsIndexOffset")->TabVisible       =false;                                     //Steven 20120706 : 調整畫面
    EL<TTabSheet>("TfOffSet", "tsArmOffset")->TabVisible         =false;
//    tsPickerList->TabVisible        =false;
    EL<TPageControl>("TfOffSet", "PageControl1")->ActivePageIndex   =0;
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfOffSet", "PageControl1")->ActivePage        =EL<TTabSheet>("TfOffSet", "tsInOutArmOffset");

    iOffsetMap[OfsLoader                ]=InOfsLoader;
    iOffsetMap[OfsHP1                   ]=InOfsHP1;
    iOffsetMap[OfsHP2                   ]=InOfsHP2;
    iOffsetMap[OfsInSh1                 ]=InOfsInSh1;
    iOffsetMap[OfsInSh2                 ]=InOfsInSh2;
    iOffsetMap[OfsOutSh1                ]=OutOfsOutSh1;
    iOffsetMap[OfsOutSh2                ]=OutOfsOutSh2;
    iOffsetMap[OfsAuto1                 ]=OutOfsAuto1;
    iOffsetMap[OfsAuto2                 ]=OutOfsAuto2;
    iOffsetMap[OfsAuto3                 ]=OutOfsAuto3;
    iOffsetMap[OfsAuto4                 ]=OutOfsAuto4;
    iOffsetMap[OfsAuto5                 ]=OutOfsAuto5;
    iOffsetMap[OfsAuto6                 ]=OutOfsAuto6;
    iOffsetMap[OfsFix1                  ]=OutOfsFix1;
    iOffsetMap[OfsFix2                  ]=OutOfsFix2;
    iOffsetMap[OfsFix3                  ]=OutOfsFix3;
    iOffsetMap[OfsFix4                  ]=OutOfsFix4;
    iOffsetMap[OfsFix5                  ]=OutOfsFix5;
    iOffsetMap[OfsFix6                  ]=OutOfsFix6;
    iOffsetMap[OfsAutoClean             ]=InOfsAutoClean;
    iOffsetMap[OfsOCR                   ]=InOfsOCR;
    iOffsetMap[OfsRotate_In             ]=InOfsRotate_In;
    iOffsetMap[OfsRotate_Out            ]=OutOfsRotate_Out;
    iOffsetMap[OfsTopView               ]=OutOfsTopView_Out;
    iOffsetMap[OfsPADView               ]=OutOfsPADView_Out;
    iOffsetMap[OfsBGAView               ]=OutOfsBGA_Out;
    iOffsetMap[OfsLoaderB               ]=InOfsLoaderB;
    iOffsetMap[OfsInSh1LB               ]=InOfsInSh1LB;
    iOffsetMap[OfsInSh1RA               ]=InOfsInSh1RA;
    iOffsetMap[OfsInSh1RB               ]=InOfsInSh1RB;
    iOffsetMap[OfsInSh2LB               ]=InOfsInSh2LB;
    iOffsetMap[OfsInSh2RA               ]=InOfsInSh2RA;
    iOffsetMap[OfsInSh2RB               ]=InOfsInSh2RB;
    iOffsetMap[OfsInSh1_AutoClean       ]=InOfsInSh1_AutoClean;
    iOffsetMap[OfsInSh1LB_AutoClean     ]=InOfsInSh1LB_AutoClean;
    iOffsetMap[OfsInSh1RA_AutoClean     ]=InOfsInSh1RA_AutoClean;
    iOffsetMap[OfsInSh1RB_AutoClean     ]=InOfsInSh1RB_AutoClean;
    iOffsetMap[OfsInSh2_AutoClean       ]=InOfsInSh2_AutoClean;
    iOffsetMap[OfsInSh2LB_AutoClean     ]=InOfsInSh2LB_AutoClean;
    iOffsetMap[OfsInSh2RA_AutoClean     ]=InOfsInSh2RA_AutoClean;
    iOffsetMap[OfsInSh2RB_AutoClean     ]=InOfsInSh2RB_AutoClean;
    iOffsetMap[OfsAutoSh1               ]=InOfsAutoSh1;
    iOffsetMap[OfsAutoSh2               ]=InOfsAutoSh2;
    iOffsetMap[OfsPreciser              ]=InOfsPreciser;
    iOffsetMap[OfsOutSh1LB              ]=OutOfsOutSh1LB;
    iOffsetMap[OfsOutSh1RA              ]=OutOfsOutSh1RA;
    iOffsetMap[OfsOutSh1RB              ]=OutOfsOutSh1RB;
    iOffsetMap[OfsOutSh2LB              ]=OutOfsOutSh2LB;
    iOffsetMap[OfsOutSh2RA              ]=OutOfsOutSh2RA;
    iOffsetMap[OfsOutSh2RB              ]=OutOfsOutSh2RB;
    iOffsetMap[OfsScanAOI               ]=OutOfsScannerAOI;
    iOffsetMap[OfsInPlacement           ]=InOfsPlacement;
    iOffsetMap[OfsBottom2DID            ]=InOfsBottom2DID;

    TSpeedButton *tempSpPtr[]=
    {
        EL<TSpeedButton>("TfOffSet", "sbLoader"),  EL<TSpeedButton>("TfOffSet", "sbHp1"),  EL<TSpeedButton>("TfOffSet", "sbHp2"), EL<TSpeedButton>("TfOffSet", "sbInSh1"), EL<TSpeedButton>("TfOffSet", "sbInSh2"),
        EL<TSpeedButton>("TfOffSet", "sbOutSh1"),  EL<TSpeedButton>("TfOffSet", "sbOutSh2"),
        EL<TSpeedButton>("TfOffSet", "sbAuto1"), EL<TSpeedButton>("TfOffSet", "sbAuto2"), EL<TSpeedButton>("TfOffSet", "sbAuto3"), EL<TSpeedButton>("TfOffSet", "sbAuto4"), EL<TSpeedButton>("TfOffSet", "sbAuto5"), EL<TSpeedButton>("TfOffSet", "sbAuto6"),                   //Steven 20230907 : For HT-9011UC
        EL<TSpeedButton>("TfOffSet", "sbFix1"),  EL<TSpeedButton>("TfOffSet", "sbFix2"),  EL<TSpeedButton>("TfOffSet", "sbFix3"),  EL<TSpeedButton>("TfOffSet", "sbFix4"),  EL<TSpeedButton>("TfOffSet", "sbFix5"),  EL<TSpeedButton>("TfOffSet", "sbFix6"),                    //Steven 20230907 : For HT-9011UC
        EL<TSpeedButton>("TfOffSet", "sbAutoClean"), EL<TSpeedButton>("TfOffSet", "sbOCR"),
        EL<TSpeedButton>("TfOffSet", "sbInRotate"), EL<TSpeedButton>("TfOffSet", "sbOutRotate"),
        EL<TSpeedButton>("TfOffSet", "sbTopView"), EL<TSpeedButton>("TfOffSet", "sbPADView"), EL<TSpeedButton>("TfOffSet", "sbBGAView"),                                        //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
        EL<TSpeedButton>("TfOffSet", "sbLoaderB"),                                                              //Steven 20140811 : For 32 Site Loader and Shuttle Offset
        EL<TSpeedButton>("TfOffSet", "sbInSh1LB"), EL<TSpeedButton>("TfOffSet", "sbInSh1RA"), EL<TSpeedButton>("TfOffSet", "sbInSh1RB"),                                        //Steven 20140811 : For 32 Site Loader and Shuttle Offset
        EL<TSpeedButton>("TfOffSet", "sbInSh2LB"), EL<TSpeedButton>("TfOffSet", "sbInSh2RA"), EL<TSpeedButton>("TfOffSet", "sbInSh2RB"),                                        //Steven 20140811 : For 32 Site Loader and Shuttle Offset
        EL<TSpeedButton>("TfOffSet", "sbInSh1_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh1LB_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh1RA_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh1RB_AutoClean"),         //20140923 wei : For Shuttle Auto Clean
        EL<TSpeedButton>("TfOffSet", "sbInSh2_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh2LB_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh2RA_AutoClean"), EL<TSpeedButton>("TfOffSet", "sbInSh2RB_AutoClean"),         //20140923 wei : For Shuttle Auto Clean
        EL<TSpeedButton>("TfOffSet", "sbAutoSh1"),EL<TSpeedButton>("TfOffSet", "sbAutoSh2"),                                                                      //wei 20160914 Auto Shuttle Sensor
        EL<TSpeedButton>("TfOffSet", "sbPreciser"),                                                             //Frank 20180410 (Steven) : InArm Preciser Station
        EL<TSpeedButton>("TfOffSet", "sbOutSh1LB"), EL<TSpeedButton>("TfOffSet", "sbOutSh1RA"), EL<TSpeedButton>("TfOffSet", "sbOutSh1RB"),                                     //Steven 20190802 : For 32 Site Out Shuttle Offset
        EL<TSpeedButton>("TfOffSet", "sbOutSh2LB"), EL<TSpeedButton>("TfOffSet", "sbOutSh2RA"), EL<TSpeedButton>("TfOffSet", "sbOutSh2RB"),
        EL<TSpeedButton>("TfOffSet", "sbScanAOI"),                                                              //Ifor 20191229 : add Scan AOI
        EL<TSpeedButton>("TfOffSet", "sbInPlacement"),                                                          //JimmyChiu 20220908 add Pickup Error Placement
        EL<TSpeedButton>("TfOffSet", "btnBottom2D"),
    };
    TSpeedButton *SpecOffSetPtr[]=
    {
        EL<TSpeedButton>("TfOffSet", "btnTrayOfsLoader"), EL<TSpeedButton>("TfOffSet", "btnTrayOfsEmpty"), EL<TSpeedButton>("TfOffSet", "btnTrayOfsColor"),
        EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto1"),  EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto2"), EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto3"),
        EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto4"),  EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto5"), EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto6"),
        EL<TSpeedButton>("TfOffSet", "IndexOffSetBT1"),   EL<TSpeedButton>("TfOffSet", "IndexOffSetBT2")
    };
    TEdit *tempPickEdit[2][8]=
    {
        {EL<TEdit>("TfOffSet", "EditPickA"), EL<TEdit>("TfOffSet", "EditPickC"), EL<TEdit>("TfOffSet", "EditPickE"), EL<TEdit>("TfOffSet", "EditPickG"), EL<TEdit>("TfOffSet", "EditPickI"), EL<TEdit>("TfOffSet", "EditPickK"), EL<TEdit>("TfOffSet", "EditPickM"), EL<TEdit>("TfOffSet", "EditPickO")},
        {EL<TEdit>("TfOffSet", "EditPickB"), EL<TEdit>("TfOffSet", "EditPickD"), EL<TEdit>("TfOffSet", "EditPickF"), EL<TEdit>("TfOffSet", "EditPickH"), EL<TEdit>("TfOffSet", "EditPickJ"), EL<TEdit>("TfOffSet", "EditPickL"), EL<TEdit>("TfOffSet", "EditPickN"), EL<TEdit>("TfOffSet", "EditPickP")}
    };
    TEdit *tempRelsEdit[2][8]=
    {
        {EL<TEdit>("TfOffSet", "EdtRelsA"), EL<TEdit>("TfOffSet", "EdtRelsC"), EL<TEdit>("TfOffSet", "EdtRelsE"), EL<TEdit>("TfOffSet", "EdtRelsG"), EL<TEdit>("TfOffSet", "EdtRelsI"), EL<TEdit>("TfOffSet", "EdtRelsK"), EL<TEdit>("TfOffSet", "EdtRelsM"), EL<TEdit>("TfOffSet", "EdtRelsO")},
        {EL<TEdit>("TfOffSet", "EdtRelsB"), EL<TEdit>("TfOffSet", "EdtRelsD"), EL<TEdit>("TfOffSet", "EdtRelsF"), EL<TEdit>("TfOffSet", "EdtRelsH"), EL<TEdit>("TfOffSet", "EdtRelsJ"), EL<TEdit>("TfOffSet", "EdtRelsL"), EL<TEdit>("TfOffSet", "EdtRelsN"), EL<TEdit>("TfOffSet", "EdtRelsP")}
    };
    TLabel *tempPickLab[2][8]=
    {
        {EL<TLabel>("TfOffSet", "LabPickA"), EL<TLabel>("TfOffSet", "LabPickC"), EL<TLabel>("TfOffSet", "LabPickE"), EL<TLabel>("TfOffSet", "LabPickG"), EL<TLabel>("TfOffSet", "LabPickI"), EL<TLabel>("TfOffSet", "LabPickK"), EL<TLabel>("TfOffSet", "LabPickM"), EL<TLabel>("TfOffSet", "LabPickO")},
        {EL<TLabel>("TfOffSet", "LabPickB"), EL<TLabel>("TfOffSet", "LabPickD"), EL<TLabel>("TfOffSet", "LabPickF"), EL<TLabel>("TfOffSet", "LabPickH"), EL<TLabel>("TfOffSet", "LabPickJ"), EL<TLabel>("TfOffSet", "LabPickL"), EL<TLabel>("TfOffSet", "LabPickN"), EL<TLabel>("TfOffSet", "LabPickP")}
    };
    TLabel *tempRelsLab[2][8]=
    {
        {EL<TLabel>("TfOffSet", "LabRelsA"), EL<TLabel>("TfOffSet", "LabRelsC"), EL<TLabel>("TfOffSet", "LabRelsE"), EL<TLabel>("TfOffSet", "LabRelsG"), EL<TLabel>("TfOffSet", "LabRelsI"), EL<TLabel>("TfOffSet", "LabRelsK"), EL<TLabel>("TfOffSet", "LabRelsM"), EL<TLabel>("TfOffSet", "LabRelsO")},
        {EL<TLabel>("TfOffSet", "LabRelsB"), EL<TLabel>("TfOffSet", "LabRelsD"), EL<TLabel>("TfOffSet", "LabRelsF"), EL<TLabel>("TfOffSet", "LabRelsH"), EL<TLabel>("TfOffSet", "LabRelsJ"), EL<TLabel>("TfOffSet", "LabRelsL"), EL<TLabel>("TfOffSet", "LabRelsN"), EL<TLabel>("TfOffSet", "LabRelsP")}
    };

    TEdit *SingleOffsetTEditX[2][8]=                                            //Eastsun 20251224 : 吸嘴單獨offset功能UI陣列
    {                                                                           //Eastsun 20251224 : X編輯元件edit
        {EL<TEdit>("TfOffSet", "EdtOffsetAX"), EL<TEdit>("TfOffSet", "EdtOffsetCX"), EL<TEdit>("TfOffSet", "EdtOffsetEX"), EL<TEdit>("TfOffSet", "EdtOffsetGX"), EL<TEdit>("TfOffSet", "EdtOffsetIX"), EL<TEdit>("TfOffSet", "EdtOffsetKX"), EL<TEdit>("TfOffSet", "EdtOffsetMX"), EL<TEdit>("TfOffSet", "EdtOffsetOX")},
        {EL<TEdit>("TfOffSet", "EdtOffsetBX"), EL<TEdit>("TfOffSet", "EdtOffsetDX"), EL<TEdit>("TfOffSet", "EdtOffsetFX"), EL<TEdit>("TfOffSet", "EdtOffsetHX"), EL<TEdit>("TfOffSet", "EdtOffsetJX"), EL<TEdit>("TfOffSet", "EdtOffsetLX"), EL<TEdit>("TfOffSet", "EdtOffsetNX"), EL<TEdit>("TfOffSet", "EdtOffsetPX")}
    };
    TEdit *SingleOffsetTEditY[2][8]=                                            //Eastsun 20251224 : 吸嘴單獨offset功能UI陣列
    {                                                                           //Eastsun 20251224 : Y編輯元件edit
        {EL<TEdit>("TfOffSet", "EdtOffsetAY"), EL<TEdit>("TfOffSet", "EdtOffsetCY"), EL<TEdit>("TfOffSet", "EdtOffsetEY"), EL<TEdit>("TfOffSet", "EdtOffsetGY"), EL<TEdit>("TfOffSet", "EdtOffsetIY"), EL<TEdit>("TfOffSet", "EdtOffsetKY"), EL<TEdit>("TfOffSet", "EdtOffsetMY"), EL<TEdit>("TfOffSet", "EdtOffsetOY")},
        {EL<TEdit>("TfOffSet", "EdtOffsetBY"), EL<TEdit>("TfOffSet", "EdtOffsetDY"), EL<TEdit>("TfOffSet", "EdtOffsetFY"), EL<TEdit>("TfOffSet", "EdtOffsetHY"), EL<TEdit>("TfOffSet", "EdtOffsetJY"), EL<TEdit>("TfOffSet", "EdtOffsetLY"), EL<TEdit>("TfOffSet", "EdtOffsetNY"), EL<TEdit>("TfOffSet", "EdtOffsetPY")}
    };
    TLabel *SingleOffsetLabX[2][8]=                                                 //Eastsun 20251224 : 吸嘴單獨offset功能UI陣列
    {                                                                           //Eastsun 20251224 : X顯示元件label
        {EL<TLabel>("TfOffSet", "LabOffsetAX"), EL<TLabel>("TfOffSet", "LabOffsetCX"), EL<TLabel>("TfOffSet", "LabOffsetEX"), EL<TLabel>("TfOffSet", "LabOffsetGX"), EL<TLabel>("TfOffSet", "LabOffsetIX"), EL<TLabel>("TfOffSet", "LabOffsetKX"), EL<TLabel>("TfOffSet", "LabOffsetMX"), EL<TLabel>("TfOffSet", "LabOffsetOX")},
        {EL<TLabel>("TfOffSet", "LabOffsetBX"), EL<TLabel>("TfOffSet", "LabOffsetDX"), EL<TLabel>("TfOffSet", "LabOffsetFX"), EL<TLabel>("TfOffSet", "LabOffsetHX"), EL<TLabel>("TfOffSet", "LabOffsetJX"), EL<TLabel>("TfOffSet", "LabOffsetLX"), EL<TLabel>("TfOffSet", "LabOffsetNX"), EL<TLabel>("TfOffSet", "LabOffsetPX")}
    };
    TLabel *SingleOffsetLabY[2][8]=                                                 //Eastsun 20251224 : 吸嘴單獨offset功能UI陣列
    {                                                                           //Eastsun 20251224 : Y顯示元件label
        {EL<TLabel>("TfOffSet", "LabOffsetAY"), EL<TLabel>("TfOffSet", "LabOffsetCY"), EL<TLabel>("TfOffSet", "LabOffsetEY"), EL<TLabel>("TfOffSet", "LabOffsetGY"), EL<TLabel>("TfOffSet", "LabOffsetIY"), EL<TLabel>("TfOffSet", "LabOffsetKY"), EL<TLabel>("TfOffSet", "LabOffsetMY"), EL<TLabel>("TfOffSet", "LabOffsetOY")},
        {EL<TLabel>("TfOffSet", "LabOffsetBY"), EL<TLabel>("TfOffSet", "LabOffsetDY"), EL<TLabel>("TfOffSet", "LabOffsetFY"), EL<TLabel>("TfOffSet", "LabOffsetHY"), EL<TLabel>("TfOffSet", "LabOffsetJY"), EL<TLabel>("TfOffSet", "LabOffsetLY"), EL<TLabel>("TfOffSet", "LabOffsetNY"), EL<TLabel>("TfOffSet", "LabOffsetPY")}
    };

    for(int i=0; i<OfsTotal; i++)
    {
        OffSetSelBot[i]=tempSpPtr[i];
        OffSetSelBot[i]->Tag=i;
    ;   // golden cOffSet.cpp:327-327 取代：OnClick 事件指派：頁面按鈕由入口 FileRW/Offset_File_C.cpp 以 Tag 呼叫 OS_SpBotSelClick(i)（同一支 golden 事件）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:327-327 -- OnClick 事件指派：頁面按鈕由入口 FileRW/Offset_File_C.cpp 以 Tag 呼叫 OS_SpBotSelClick(i)（同一支 golden 事件）
        OffSetSelBot[i]->OnClick=SpBotSelClick;
#endif // GATE (S12-C save)
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            MyPickEdit[i][j]=tempPickEdit[i][j];
            MyPickEdit[i][j]->Visible=false;
            MyPickEdit[i][j]->Text=0.0;

            MyRelsEdit[i][j]=tempRelsEdit[i][j];
            MyRelsEdit[i][j]->Visible=false;
            MyRelsEdit[i][j]->Text=0.0;

            MyPickLab[i][j]=tempPickLab[i][j];
            MyPickLab[i][j]->Visible=false;

            MyRelsLab[i][j]=tempRelsLab[i][j];
            MyRelsLab[i][j]->Visible=false;

            MySingleOffsetTEditX[i][j]=SingleOffsetTEditX[i][j];
            MySingleOffsetTEditX[i][j]->Visible=false;
            MySingleOffsetTEditX[i][j]->Text=0.0;

            MySingleOffsetTEditY[i][j]=SingleOffsetTEditY[i][j];
            MySingleOffsetTEditY[i][j]->Visible=false;
            MySingleOffsetTEditY[i][j]->Text=0.0;

            MySingleOffsetLabX[i][j]=SingleOffsetLabX[i][j];
            MySingleOffsetLabX[i][j]->Visible=false;

            MySingleOffsetLabY[i][j]=SingleOffsetLabY[i][j];
            MySingleOffsetLabY[i][j]->Visible=false;
        }
    }

    for(int i=0; i<trayOfsTotal; i++)
    {
        OfffSetSpecSelBot[i]=SpecOffSetPtr[i];
        OfffSetSpecSelBot[i]->Tag=i;
    ;   // golden cOffSet.cpp:368-368 取代：OnClick 事件指派：同上（OS_IndexOffSetBT2Click(i)）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:368-368 -- OnClick 事件指派：同上（OS_IndexOffSetBT2Click(i)）
        OfffSetSpecSelBot[i]->OnClick=IndexOffSetBT2Click;
#endif // GATE (S12-C save)
    }

    iNowOffsetSel       =-1;
    iSpecialOffSetSel   =-1;
    LastFileName        ="";
    EL<TEdit>("TfOffSet", "edArmX")->Text        =0.0;
    EL<TEdit>("TfOffSet", "edArmY")->Text        =0.0;
    EL<TEdit>("TfOffSet", "edPickUp")->Text      =0.0;
    EL<TEdit>("TfOffSet", "edRelease")->Text     =0.0;
    EL<TEdit>("TfOffSet", "edPitchX1")->Text     =0.0;

    EL<TEdit>("TfOffSet", "edPitchY")->Text      =0.0;                                                   //ChungHung 20131231 alter AutoYPitch
    EL<TEdit>("TfOffSet", "edPitchX2")->Text     =0.0;                                                   //ChungHung 20131231 alter AutoYPitch
    bEnterSpecialOffset =false;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                                       //jou 2010-08-03 offset tray arm 調整 offset,loader處隱藏,只保留Tray set.
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                     //Steven 20131101 : Add ASE-K12
        EL<TSpeedButton>("TfOffSet", "btnTrayOfsLoader")->Visible=false;

    ;   // UI-only（HTML 端處理）：EL<TControl>("TfOffSet", "Image2")->Picture->Bitmap->TransparentColor = EL<TControl>("TfOffSet", "Image2")->Canvas->Pixels[0][0];

    ;   // golden cOffSet.cpp:390-423 取代：sgOffsetList（TStringGrid）表頭／欄寬／列數：總表分頁 tsArmOffset 的畫面，替身 ELStringGrid 沒有 ColCount／ColWidths；總表內容頁面可由各組 offsets 自己組（ShowOffSetList 同樣不轉）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:390-423 -- sgOffsetList（TStringGrid）表頭／欄寬／列數：總表分頁 tsArmOffset 的畫面，替身 ELStringGrid 沒有 ColCount／ColWidths；總表內容頁面可由各組 offsets 自己組（ShowOffSetList 同樣不轉）
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColCount=oiArmTotal;
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->RowCount=OfsTotal+1;
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->DefaultColWidth=80;
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[0]=200;
    for(int i=0; i<OfsTotal; i++)
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[0][i+1]=AnsiString(" ")+CapStr[i];
    }
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmX     ][0]=" X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmY     ][0]=" Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch1][0]=" X Pitch 1";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch2][0]=" X Pitch 2";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch3][0]=" X Pitch 3";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch4][0]=" X Pitch 4";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitchY][0]=" Y Pitch";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPick  ][0]=" Pick";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPlace ][0]=" Place";

    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_X ][0]=" A_X";                               //Eastsun 20251218 : 每支吸嘴獨立偏移XY
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_Y ][0]=" A_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_X ][0]=" B_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_Y ][0]=" B_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_X ][0]=" C_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_Y ][0]=" C_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_X ][0]=" D_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_Y ][0]=" D_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_X ][0]=" E_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_Y ][0]=" E_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_X ][0]=" F_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_Y ][0]=" F_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_X ][0]=" G_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_Y ][0]=" G_Y";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_X ][0]=" H_X";
    EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_Y ][0]=" H_Y";
#endif // GATE (S12-C save)

    EL<TButton>("TfOffSet", "btnSortAuto1")->Tag=eAuto1;
    EL<TButton>("TfOffSet", "btnSortAuto2")->Tag=eAuto2;
    EL<TButton>("TfOffSet", "btnSortAuto3")->Tag=eAuto3;
    EL<TButton>("TfOffSet", "btnSortFix1")->Tag =eFix1;
    EL<TButton>("TfOffSet", "btnSortFix2")->Tag =eFix2;
    EL<TButton>("TfOffSet", "btnSortFix3")->Tag =eFix3;

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)                    //Jimmychiu 20240322 : Top & Bottom Inspect
    {
        EL<TSpeedButton>("TfOffSet", "sbScanAOI")->Visible=true;
        CapStr[OfsScanAOI]="Top Bottom AOI";
        CapStrOutput[OutOfsScannerAOI]="Top Bottom AOI";
    }

    OS_LoadImage(true);   // golden cOffSet.cpp:439-439 取代：golden LoadImage() 用 cOffSet.h 的預設參數 bInArm=true
#if 0 // GATE (S12-C save) golden cOffSet.cpp:439-439 -- golden LoadImage() 用 cOffSet.h 的預設參數 bInArm=true
    OS_LoadImage();
#endif // GATE (S12-C save)

}

// golden cOffSet.cpp:442  TfOffSet::LoadImage(bool bInArm)
static void OS_LoadImage(bool bInArm)
{

    AnsiString asString;                                                        //jou 2012-08-22 將大型圖片用LoadFormFile節省compiler時間 & 縮小執行檔大小

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //Steven 20240822 : For HT-9046AU
        asString=BmpPath+"InOutArmOffset_AU.bmp";
    else if(AUTO_EMPTY_COLOR>=3)                                                //Steven 20230907 : For HT-9011UC
        asString=BmpPath+"InOutArmOffset_6.bmp";
    else
        asString=BmpPath+"InOutArmOffset.BMP";

    if(FileExists(asString))
    {
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfOffSet", "Image1")->Picture->LoadFromFile(asString);
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfOffSet", "Image3")->Picture->LoadFromFile(asString);
    }

    OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);                               //kevin 20151105  : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //Steven 20141029 : XY-Pitch
    {
        if(bInArm)
            asString=BmpPath+"SuckBaseF.bmp";
        else
            asString=BmpPath+"SuckBaseD.bmp";
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)
    {
        asString=BmpPath+"SuckBase1032_Bd.bmp";
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        if(bInArm)
            asString=BmpPath+"SuckBase1032_Bd.bmp";
        else
            asString=BmpPath+"SuckBase1032_Be.bmp";
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                       //Ztex 2024.02.24 Add HT-1132
    {
        if(bInArm)
            asString=BmpPath+"SuckBaseD.bmp";
        else
            asString=BmpPath+"SuckBaseF.bmp";
    }
    else
    {
        asString=BmpPath+"SuckUnit1.BMP";
    }

    if(FileExists(asString))
    { }   // golden cOffSet.cpp:491-491 取代：示意圖（HTML 自己有）；無大括號 if 的本體 → 空區塊（免 -Wempty-body）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:491-491 -- 示意圖（HTML 自己有）；無大括號 if 的本體 → 空區塊（免 -Wempty-body）
        ;   // UI-only（HTML 端處理）：EL<TControl>("TfOffSet", "Image2")->Picture->LoadFromFile(asString);
#endif // GATE (S12-C save)

    OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);                               //kevin 20151105 add  Steven 20140510 : XY變距

}

// golden cOffSet.cpp:496  TfOffSet::FormShow(TObject *Sender)
static void OS_FormShow()
{

    fShow=true;
    AnsiString S;
    GetLimitAuth();
    LastFileName=GetLastOpenFN();
    ReadFile();
    ;   // golden cOffSet.cpp:503-504 取代：Caption＝視窗標題（含 LastFileName；AnsiString 丟進 sprintf 的 ... g++ 不收）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:503-504 -- Caption＝視窗標題（含 LastFileName；AnsiString 丟進 sprintf 的 ... g++ 不收）
    S.sprintf("Position Offset  ''%s''  ",LastFileName);
    Caption=S;
#endif // GATE (S12-C save)

    bEnterOffset=true;
    bUseUpdate=false;
    ;   // golden cOffSet.cpp:508-508 取代：視窗位置
#if 0 // GATE (S12-C save) golden cOffSet.cpp:508-508 -- 視窗位置
    Left=10;
#endif // GATE (S12-C save)
    ;   // golden cOffSet.cpp:509-509 取代：視窗位置
#if 0 // GATE (S12-C save) golden cOffSet.cpp:509-509 -- 視窗位置
    Top=10;
#endif // GATE (S12-C save)

    if(iNowOffsetSel!=-1)
        OS_DoIniDataToForm(iNowOffsetSel, iSaveStander);
    if(iSpecialOffSetSel!=-1)
        OS_DoIniDataToForm(iSpecialOffSetSel, iSaveSpecial);

    S.sprintf("%d~%dmm", InputLimit.iOffsetXYHigh, InputLimit.iOffsetXYLow);    //Steven 20140909 : 修正Offset限制的顯示
    EL<TLabel>("TfOffSet", "lblArmXRange")    ->Caption=S;
    EL<TLabel>("TfOffSet", "lblArmYRange")    ->Caption=S;
    EL<TLabel>("TfOffSet", "lblPitchX1Range") ->Caption=S;
    EL<TLabel>("TfOffSet", "lblPitchX2Range") ->Caption=S;
    EL<TLabel>("TfOffSet", "lblPitchYRange")  ->Caption=S;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20211111 add 1 ~ -1 abel for K7
    {
        S.sprintf("%d~%dmm", 1, -1);
        EL<TLabel>("TfOffSet", "lblReleaseRange")->Caption=S;
        S.sprintf("%d~%dmm", InputLimit.iOffsetZHigh, InputLimit.iOffsetZLow);  //Steven 20140909 : 修正Offset限制的顯示
        EL<TLabel>("TfOffSet", "lblPickUpRange")->Caption=S;
    }
    else
    {
        S.sprintf("%d~%dmm", InputLimit.iOffsetZHigh, InputLimit.iOffsetZLow);  //Steven 20140909 : 修正Offset限制的顯示
        EL<TLabel>("TfOffSet", "lblReleaseRange")->Caption=S;
        EL<TLabel>("TfOffSet", "lblPickUpRange")->Caption=S;
    }

    S.sprintf("Shuttle Right Offset (%2.f<->%2.f)", InputLimit.dShuttleHigh, InputLimit.dShuttleLow);       //ChungHung 20150115 add for ATK +/-2 mm
    EL<TLabel>("TfOffSet", "lblShtRight")->Caption=S;
    S.sprintf("Shuttle Left Offset (%2.f<->%2.f)", InputLimit.dShuttleHigh, InputLimit.dShuttleLow);
    EL<TLabel>("TfOffSet", "lblShtLeft")->Caption=S;

    EL<TSpeedButton>("TfOffSet", "sbAutoClean")->Visible=(IniConfig.bEnableAutoCleanFunction && IniConfig.bE43AutoCleanUseHotplate==false); //Steven 20230417 : 使用E43加熱盤不顯示AutoClean
    EL<TSpeedButton>("TfOffSet", "sbInRotate")->Visible =((USE_ROTATE_KIT || USE_DIE_CLEAN==1)&& iRotate_Type!=eInOutArm1Motor);                  //jou 2014-05-29 修正offset畫面顯示錯誤
    EL<TSpeedButton>("TfOffSet", "sbOutRotate")->Visible=(USE_ROTATE_KIT && iRotate_Type!=eInOutArm1Motor);                                        //jou 2014-05-29 修正offset畫面顯示錯誤
    EL<TSpeedButton>("TfOffSet", "sbOCR")->Visible      =TestIF.bOcrFunction;

    EL<TCheckBox>("TfOffSet", "chkInArmScakeEnable")->Checked=TestIF_File.bInArmUseDifferentScaleBySetupFile;

    EL<TEdit>("TfOffSet", "edLodX")->Text=TestIF_File.fLoaderTrayXScaleBySetupFile;
    EL<TEdit>("TfOffSet", "edLodY")->Text=TestIF_File.fLoaderTrayYScaleBySetupFile;
    EL<TEdit>("TfOffSet", "edt_HP1X")->Text=TestIF_File.fHotPlateXScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "edt_HP1Y")->Text=TestIF_File.fHotPlateYScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "edt_HP2X")->Text=TestIF_File.fHotPlateXScaleBySetupFile[1];
    EL<TEdit>("TfOffSet", "edt_HP2Y")->Text=TestIF_File.fHotPlateYScaleBySetupFile[1];

    EL<TEdit>("TfOffSet", "ed_IS1X")->Text=TestIF_File.fInShuttleXScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "ed_IS1Y")->Text=TestIF_File.fInShuttleYScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "ed_IS2X")->Text=TestIF_File.fInShuttleXScaleBySetupFile[1];
    EL<TEdit>("TfOffSet", "ed_IS2Y")->Text=TestIF_File.fInShuttleYScaleBySetupFile[1];

    EL<TCheckBox>("TfOffSet", "chkOutArmScakeEnable")->Checked=TestIF_File.bOutArmUseDifferentScaleBySetupFile;

    EL<TEdit>("TfOffSet", "ed_OS1X")->Text=TestIF_File.fOutShuttleXScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "ed_OS1Y")->Text=TestIF_File.fOutShuttleYScaleBySetupFile[0];
    EL<TEdit>("TfOffSet", "ed_OS2X")->Text=TestIF_File.fOutShuttleXScaleBySetupFile[1];
    EL<TEdit>("TfOffSet", "ed_OS2Y")->Text=TestIF_File.fOutShuttleYScaleBySetupFile[1];

    EL<TLabel>("TfOffSet", "lblShowMessage")->Caption         ="";
    EL<TLabel>("TfOffSet", "labWarningForStop")->Visible      =false;                                     //jou 2011-02-18
    OS_btnBackClick();   // golden cOffSet.cpp:570-570 取代：事件以 (this) 呼叫（參數已拿掉）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:570-570 -- 事件以 (this) 呼叫（參數已拿掉）
    OS_btnBackClick(this);
#endif // GATE (S12-C save)

    EL<TPanel>("TfOffSet", "pan_AutoOffsetMove")->Visible     =IniConfig.bUseAutoOffsetFunction;          //jou 2013-08-29 Use Auto Offset Funtion
    EL<TPanel>("TfOffSet", "palPreciser")->Visible            =(TestIF_File.bEnableUsePreciser ||         //JerryYang 20180820 add
                                      TestIF_File.bEnableRTPreciser);
    if(AccessLevel<iDefHonPrecLevel)
    {
        EL<TTabSheet>("TfOffSet", "tsScale")->Enabled=false;
    }
    else
    {
        EL<TTabSheet>("TfOffSet", "tsScale")->Enabled=true;
    }

    EL<TPanel>("TfOffSet", "pnlPicker")->Enabled      =fSecurity->Insufficient(2, false) && authMainForm[2];  //jou 981207 權限控制
    EL<TSpeedButton>("TfOffSet", "btnOffsetList")->Enabled  =fSecurity->Insufficient(2, false) && authMainForm[2];
//    btnPickerList->Enabled  =fSecurity->Insufficient(2, false) && authMainForm[2];
    { if(CUSTOMER_CODE==CC_KYEC_XILINX) filerw::ELTodo("golden cOffSet.cpp:587 fBarCode->bShow taken as false (TfBarCode has no bShow in the port)"); } if(false && CUSTOMER_CODE==CC_KYEC_XILINX)   // golden cOffSet.cpp:587-587 取代：fBarCode->bShow（BarCode 表單開著）：TfBarCode 在 aHotPlateSubstrate.h:984（與 HTEditList.h 衝突）且沒有 bShow；web 開這頁時 BarCode 表單不會開著 → false
#if 0 // GATE (S12-C save) golden cOffSet.cpp:587-587 -- fBarCode->bShow（BarCode 表單開著）：TfBarCode 在 aHotPlateSubstrate.h:984（與 HTEditList.h 衝突）且沒有 bShow；web 開這頁時 BarCode 表單不會開著 → false
    if(fBarCode->bShow && CUSTOMER_CODE==CC_KYEC_XILINX)                            //Frank 20171030 (Steven) add BarCode調整Offset
#endif // GATE (S12-C save)
        EL<TPanel>("TfOffSet", "pnlIndexOfs")->Enabled=true;
    else
        EL<TPanel>("TfOffSet", "pnlIndexOfs")->Enabled=fSecurity->Insufficient(2,false) && authMainForm[2];   //jou 2014-06-12

    EL<TPanel>("TfOffSet", "grpOutArm456")->Visible=(AUTO_EMPTY_COLOR>=3);
    EL<TPanel>("TfOffSet", "palAuto6")    ->Visible=(AUTO_EMPTY_COLOR>=4);
    EL<TButton>("TfOffSet", "btnSortAuto6")->Visible=(AUTO_EMPTY_COLOR>=4);

    //Steven 20160630 : add USE_IN_OUT_ARM_Y_PITCH!=iXYPitchVariable
    EL<TSpeedButton>("TfOffSet", "sbLoaderB")->Visible=TestIF_File.bEnableAutoAlignment==false && (IniConfig.bE46_LoaderUse2Offset && (USE_IN_Y_IS_AUTO_PITCH==false || TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH) && (TestIF.iTestMode==_32Site4X8N || TestIF_File.iTestMode>=QualSite2X2));  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    EL<TSpeedButton>("TfOffSet", "sbInSh1LB")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=QualSite2X2))));
    EL<TSpeedButton>("TfOffSet", "sbInSh1RA")->Visible=TestIF_File.bEnableAutoAlignment==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5));
    EL<TSpeedButton>("TfOffSet", "sbInSh1RB")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5))));
    EL<TSpeedButton>("TfOffSet", "sbInSh2LB")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=QualSite2X2))));
    EL<TSpeedButton>("TfOffSet", "sbInSh2RA")->Visible=TestIF_File.bEnableAutoAlignment==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5));  //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    EL<TSpeedButton>("TfOffSet", "sbInSh2RB")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5))));

    //Steven 20190802 : For 32 Site Out Shuttle Offset
    EL<TSpeedButton>("TfOffSet", "sbOutSh1LB")->Visible=TestIF_File.bEnableAutoAlignment==false &&(USE_OUT_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=QualSite2X2))));
    EL<TSpeedButton>("TfOffSet", "sbOutSh1RA")->Visible=TestIF_File.bEnableAutoAlignment==false &&(TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_12Site2X6));
    EL<TSpeedButton>("TfOffSet", "sbOutSh1RB")->Visible=TestIF_File.bEnableAutoAlignment==false &&(USE_OUT_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5))));
    EL<TSpeedButton>("TfOffSet", "sbOutSh2LB")->Visible=TestIF_File.bEnableAutoAlignment==false &&(USE_OUT_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=QualSite2X2))));
    EL<TSpeedButton>("TfOffSet", "sbOutSh2RA")->Visible=TestIF_File.bEnableAutoAlignment==false &&(TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_12Site2X6));
    EL<TSpeedButton>("TfOffSet", "sbOutSh2RB")->Visible=TestIF_File.bEnableAutoAlignment==false &&(USE_OUT_Y_IS_AUTO_PITCH==false && (TestIF.dSiteYPitch!=TestIF_File.iARM_Y_PITCH && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE47_ShuttleUse4Offset && TestIF_File.iTestMode>=_10Site2X5))));

    EL<TSpeedButton>("TfOffSet", "sbInSh1LB_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=QualSite2X2)));     //20140923 wei : For Shuttle Auto Clean
    EL<TSpeedButton>("TfOffSet", "sbInSh1RA_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=_10Site2X5));                                                    //20140923 wei : For Shuttle Auto Clean
    EL<TSpeedButton>("TfOffSet", "sbInSh1RB_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=_10Site2X5)));      //20140923 wei : For Shuttle Auto Clean
    EL<TSpeedButton>("TfOffSet", "sbInSh2LB_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=QualSite2X2)));     //20140923 wei : For Shuttle Auto Clean
    EL<TSpeedButton>("TfOffSet", "sbInSh2RA_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=_10Site2X5));                                                    //20140923 wei : For Shuttle Auto Clean
    EL<TSpeedButton>("TfOffSet", "sbInSh2RB_AutoClean")->Visible=TestIF_File.bEnableAutoAlignment==false && (USE_IN_Y_IS_AUTO_PITCH==false && (TestIF.iTestMode==_32Site4X8N || (IniConfig.bE48_ShuttleUse4Offset_Autoclean && TestIF_File.iTestMode>=_10Site2X5)));      //20140923 wei : For Shuttle Auto Clean

    EL<TSpeedButton>("TfOffSet", "sbAutoSh1")->Visible    =(AUTO_SENSOR_INSTALL==1);                            //wei 20160914 Auto Shuttle Sensor
    EL<TSpeedButton>("TfOffSet", "sbAutoSh2")->Visible    =(AUTO_SENSOR_INSTALL==1);                            //wei 20160914 Auto Shuttle Sensor

    EL<TPanel>("TfOffSet", "Pnl_AOI")->Visible      =(USE_AOI_Inspection==1 || USE_Fix_AI_CCD==1 || (USE_Top_Scanner_AOI_Inspection==1 && ScannerAOIIF.iEnableTopScannerMode!=0));   //RogerYang 20180901 add 矽格湖口Demo AI CCD Function     //Steven 20140909 : AOI沒用到不要出現//Ifor 20200902 add: TFAMD Top AOI
    EL<TSpeedButton>("TfOffSet", "sbBGAView")->Visible    =EL<TPanel>("TfOffSet", "Pnl_AOI")->Visible;
    { if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) filerw::ELTodo("golden cOffSet.cpp:625 FrmAOI->RunTopBottomInspect() not in the port -- taken as false (Scan AOI button hidden)"); } EL<TPanel>("TfOffSet", "Pnl_ScanAOI")->Visible =(USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) || (false);   // golden cOffSet.cpp:625-625 取代：FrmAOI->RunTopBottomInspect()：移植樹 TFrmAOI 沒有這支 → false（只影響 Pnl_ScanAOI／sbScanAOI 顯示），Top&Bottom AOI 機種回報 todo
#if 0 // GATE (S12-C save) golden cOffSet.cpp:625-625 -- FrmAOI->RunTopBottomInspect()：移植樹 TFrmAOI 沒有這支 → false（只影響 Pnl_ScanAOI／sbScanAOI 顯示），Top&Bottom AOI 機種回報 todo
    EL<TPanel>("TfOffSet", "Pnl_ScanAOI")->Visible  =(USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) || (FrmAOI->RunTopBottomInspect()==true); //Ifor 20191229   //Jimmychiu 20240322 : Top & Bottom Inspect
#endif // GATE (S12-C save)

    EL<TSpeedButton>("TfOffSet", "sbScanAOI")->Visible    =EL<TPanel>("TfOffSet", "Pnl_ScanAOI")->Visible;
    EL<TSpeedButton>("TfOffSet", "sbPreciser")->Visible   =(USE_PRECISER);                                      //Frank 20180410 (Steven) : InArm Preciser Station
    EL<TGroupBox>("TfOffSet", "grpAutoClean")->Visible =(TestIF_File.iAutoClean_Function && IniConfig.bE48_ShuttleUse4Offset_Autoclean) ;        //20140923 wei : For Shuttle Auto Clean
    if(EL<TGroupBox>("TfOffSet", "grpAutoClean")->Visible==false)
    {
        EL<TSpeedButton>("TfOffSet", "sbInSh1_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh2_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh1LB_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh1RA_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh1RB_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh2LB_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh2RA_AutoClean")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbInSh2RB_AutoClean")->Visible=false;
    }
    else
    {
        EL<TSpeedButton>("TfOffSet", "sbInSh1_AutoClean")->Visible=true;
        EL<TSpeedButton>("TfOffSet", "sbInSh2_AutoClean")->Visible=true;
    }

    EL<TSpeedButton>("TfOffSet", "sbInPlacement")->Visible=(fMain->cInplace->InArmPlacementEnable());           //JimmyChiu 20220908 add Pickup Error Placement
    EL<TSpeedButton>("TfOffSet", "sbInPlacement")->Visible=(BOTTOM_2DID>0);                                     //jerryYang 20230407 add 顯示

    if(USE_AUTO_RETEST==eartInstall &&
       (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&                        //kevin 20170831 (Steven) add ART tray arm offset
       (LastSet.iRunStartMode==rsmInitial_ART       ||
        LastSet.iRunStartMode==rsmContinuStart_ART  ||
        LastSet.iRunStartMode==rsmContinuRetest_ART ||
        LastSet.iRunStartMode==rsmAutoRetest))
    {
        EL<TSpeedButton>("TfOffSet", "btnTrayOfsLoader")->Visible       =true;
//        pnlTrayXART->Visible            =true;
    }
    else
    {
//        pnlTrayXART->Visible            =false;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                 //Steven 20131101 : Add ASE-K12
        {
            EL<TSpeedButton>("TfOffSet", "btnTrayOfsLoader")->Visible   =false;
        }
    }

//    fOffSet->Width          =(AUTO_EMPTY_COLOR>=3)?1300:1100;                   //Steven 20230907 : For HT-9011UC
    ;   // golden cOffSet.cpp:671-671 取代：版面寬度（HTML 不用；vclcompat TControl 沒有 Width）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:671-671 -- 版面寬度（HTML 不用；vclcompat TControl 沒有 Width）
    EL<TPanel>("TfOffSet", "pnlMain")->Width          =(AUTO_EMPTY_COLOR>=3)?751:571;
#endif // GATE (S12-C save)
    ;   // golden cOffSet.cpp:672-673 取代：版面寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:672-673 -- 版面寬度（HTML 不用）
    EL<TPanel>("TfOffSet", "pnlPicker")->Width        =(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
                              USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)?976:487; //Ztex 2023.12.06 Add HT-1032
#endif // GATE (S12-C save)
    EL<TSpeedButton>("TfOffSet", "sbAuto4")->Visible        =(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "sbAuto5")->Visible        =(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "sbAuto6")->Visible        =(AUTO_EMPTY_COLOR>=4);
    EL<TSpeedButton>("TfOffSet", "sbFix4")->Visible         =(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "sbFix5")->Visible         =(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "sbFix6")->Visible         =(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto4")->Visible=(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto5")->Visible=(AUTO_EMPTY_COLOR>=3);
    EL<TSpeedButton>("TfOffSet", "btnTrayOfsAuto6")->Visible=(AUTO_EMPTY_COLOR>=4);
    ;   // golden cOffSet.cpp:683-683 取代：視窗寬度（HTML 不用）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:683-683 -- 視窗寬度（HTML 不用）
    fOffSet->Width          =EL<TPanel>("TfOffSet", "pnlMain")->Width+EL<TPanel>("TfOffSet", "pnlPicker")->Width+5;
#endif // GATE (S12-C save)

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //Steven 20240822 : For HT-9046AU
    {
        EL<TSpeedButton>("TfOffSet", "sbAuto1")->Left=548;
        EL<TSpeedButton>("TfOffSet", "sbAuto2")->Left=608;
        EL<TSpeedButton>("TfOffSet", "sbAuto3")->Left=668;
        EL<TSpeedButton>("TfOffSet", "sbAuto4")->Left=308;                                                      //RogerYang 20250505 Fix Position
        EL<TSpeedButton>("TfOffSet", "sbAuto5")->Left=368;
        EL<TSpeedButton>("TfOffSet", "sbAuto6")->Left=428;
        EL<TSpeedButton>("TfOffSet", "sbLoader")->Left=188;
    }

    OS_LoadImage(true);   // golden cOffSet.cpp:696-696 取代：golden LoadImage() 用預設參數 bInArm=true
#if 0 // GATE (S12-C save) golden cOffSet.cpp:696-696 -- golden LoadImage() 用預設參數 bInArm=true
    OS_LoadImage();                                                                //Steven 20141029 : XY-Pitch
#endif // GATE (S12-C save)
    EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible=false;                                               //JerryYang 20180921 Setup Teach功能
    EL<TTabSheet>("TfOffSet", "tsSetupTeach")->TabVisible=false;
    EL<TTabSheet>("TfOffSet", "tsZCalibration")->TabVisible=false;
    EL<TTabSheet>("TfOffSet", "tsScale")->TabVisible=false;
    EL<TTabSheet>("TfOffSet", "ts_Offset_HotTempShiftOffset")->TabVisible=false;
    EL<TSpeedButton>("TfOffSet", "sbZcalibration")->Visible=false;

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        EL<TTabSheet>("TfOffSet", "tsZCalibration")->TabVisible=true;
        EL<TTabSheet>("TfOffSet", "tsScale")->TabVisible=true;
        EL<TTabSheet>("TfOffSet", "ts_Offset_HotTempShiftOffset")->TabVisible=true;
        EL<TSpeedButton>("TfOffSet", "sbZcalibration")->Visible=true;
    }

    if(IniConfig.bA30SetupTeachFunction)
    {
        if(LastSet.iTester==OFF_LINE)
        {
            EL<TControl>("TfOffSet", "TimerSetupTeach")->Enabled=true;
            EL<TGroupBox>("TfOffSet", "grpSetupTeach")->Visible  =true;
            EL<TTabSheet>("TfOffSet", "tsSetupTeach")->TabVisible=true;
//            fOffSet->Width          =(AUTO_EMPTY_COLOR>=3)?1450:1262;
        }
    }

    { if(CosFunction.bOEEFunction) filerw::ELTodo("golden cOffSet.cpp:723-732 OEE contact force offset (fProductionInfo->CheckContactForceExist/GetOffsetContactForce) not in the port (GATE O-7) -- pnlOEE_Contact hidden"); } EL<TPanel>("TfOffSet", "pnlOEE_Contact")->Visible=false;   // golden cOffSet.cpp:723-732 取代：OEE Contact Force offset：fProductionInfo->CheckContactForceExist／GetOffsetContactForce 全樹沒有（同移植樹 GATE O-7）→ pnlOEE_Contact 不顯示（golden else 分支），OEE 機台回報 todo
#if 0 // GATE (S12-C save) golden cOffSet.cpp:723-732 -- OEE Contact Force offset：fProductionInfo->CheckContactForceExist／GetOffsetContactForce 全樹沒有（同移植樹 GATE O-7）→ pnlOEE_Contact 不顯示（golden else 分支），OEE 機台回報 todo
    if(CosFunction.bOEEFunction && fProductionInfo!=NULL)
    {
        EL<TPanel>("TfOffSet", "pnlOEE_Contact")->Visible     =CosFunction.bOffsetContactForce && IniConfig.bD06ContactOffsetDefaultValue && fProductionInfo->CheckContactForceExist();      //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
        EL<TEdit>("TfOffSet", "edOffsetContactForce")->Text  =FloatToStr(fProductionInfo->GetOffsetContactForce());
        OS_ShowFinalAirForce();                                                    //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
    }
    else
    {
        EL<TPanel>("TfOffSet", "pnlOEE_Contact")->Visible     =false;
    }
#endif // GATE (S12-C save)

    iIndexChange            =0;                                                 //kevin 20211211 index socket value change
    if(USE_Top_Scanner_AOI_Inspection==1)                                       //Ifor 20200902 add: TFAMD Top AOI
    {
        EL<TSpeedButton>("TfOffSet", "sbBGAView")->Visible  =false;
        EL<TSpeedButton>("TfOffSet", "sbPADView")->Visible  =false;
        EL<TSpeedButton>("TfOffSet", "sbTopView")->Visible  =true;
        EL<TCheckBox>("TfOffSet", "ck_AOI")->Caption     ="Top AOI";
        EL<TSpeedButton>("TfOffSet", "sbTopView")->Top      =32;
    }
    else
    {
        EL<TSpeedButton>("TfOffSet", "sbBGAView")->Visible  =false;
        EL<TSpeedButton>("TfOffSet", "sbPADView")->Visible  =false;
        EL<TSpeedButton>("TfOffSet", "sbTopView")->Visible  =false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        if(InArmSuck.HasIC()==true || OutArmSuck.HasIC()==true ||
           ShuttleHasIC()==true || IndexHasIC()==true || LastSet.iRealDummy!=REALLY)
        {
            EL<TSpeedButton>("TfOffSet", "sbInArmZCalibration")->Visible=false;
            EL<TSpeedButton>("TfOffSet", "sbOutArmZCalibration")->Visible=false;
        }
        else
        {
            EL<TSpeedButton>("TfOffSet", "sbInArmZCalibration")->Visible=true;
            EL<TSpeedButton>("TfOffSet", "sbOutArmZCalibration")->Visible=true;
        }
        EL<TSpeedButton>("TfOffSet", "sbInArmZCalibration")->Enabled=true;
        EL<TSpeedButton>("TfOffSet", "sbOutArmZCalibration")->Enabled=true;
    }
    else
    {
        EL<TSpeedButton>("TfOffSet", "sbInArmZCalibration")->Visible=false;
        EL<TSpeedButton>("TfOffSet", "sbOutArmZCalibration")->Visible=false;
    }

    bDoInZTeach=false;
    bDoOutZTeach=false;

    //Eastsun 20260710 Merge - B03 add: KYEC LEE offset enter barcode
    if(CUSTOMER_CODE==CC_KYEC_LEE && USE_BU5_Function==true)                    //Frank 20260710 ADD
        bOffsetEnterBarcode=true;

    EL<TControl>("TfOffSet", "Timer1")->Enabled=true;                                                       //Steven 20220120 : 機台在動的時候,不可以調整Shuttle Offset,避免Hang up

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
    filerw::ELTodo("golden cOffSet.cpp:783-828 ASE_CL hot temp shift offset grid + ReadHotTempShiftOffsetData() not in the port");   // golden cOffSet.cpp:783-828 取代：CC_ASE_CL：sg_Offset_HotTempShiftOffset 表頭（ELStringGrid 沒有 ColWidths）＋fOffSet->ReadHotTempShiftOffsetData()（移植樹 TfOffSet 沒有）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:783-828 -- CC_ASE_CL：sg_Offset_HotTempShiftOffset 表頭（ELStringGrid 沒有 ColWidths）＋fOffSet->ReadHotTempShiftOffsetData()（移植樹 TfOffSet 沒有）
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->ColWidths[0]  = 100;
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][0]   = "Position";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][1]   = "LoaderX";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][2]   = "LoaderY";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][3]   = "HotPlate1X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][4]   = "HotPlate1Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][5]   = "HotPlate2X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][6]   = "HotPlate2Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][7]   = "InShuttle1X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][8]   = "InShuttle1Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][9]   = "InShuttle2X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][10]  = "InShuttle2Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][11]  = "OutShuttle1X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][12]  = "OutShuttle1Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][13]  = "OutShuttle2X";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][14]  = "OutShuttle2Y";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][15]  = "Index1ToShuttleRight";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][16]  = "Index1ToShuttleLeft";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][17]  = "Index2ToShuttleRight";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][18]  = "Index2ToShuttleLeft";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][20]  = "LoaderXScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][21]  = "LoaderYScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][22]  = "HotPlate1XScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][23]  = "HotPlate1YScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][24]  = "HotPlate2XScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][25]  = "HotPlate2YScale";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][27]  = "Auto1XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][28]  = "Auto1YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][29]  = "Auto2XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][30]  = "Auto2YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][31]  = "Auto3XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][32]  = "Auto3YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][33]  = "Fix1XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][34]  = "Fix1YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][35]  = "Fix2XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][36]  = "Fix2YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][37]  = "Fix3XOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[0][38]  = "Fix3YOffset";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[1][0]   = "Low Temp";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[2][0]   = "Mid Temp";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[3][0]   = "High Temp";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[4][0]   = "LM Ratio";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[5][0]   = "MH Ratio";
        EL<filerw::ELStringGrid>("TfOffSet", "sg_Offset_HotTempShiftOffset")->Cells[6][0]   = "Current Temp";

        fOffSet->ReadHotTempShiftOffsetData();
#endif // GATE (S12-C save)
    }

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cOffSet.cpp:860  TfOffSet::SetXYPitchVCLVisible(bool bVisible)
static void OS_SetXYPitchVCLVisible(bool bVisible)
{

    if(bVisible)                                                                //Steven 20151103 : Fixed for 32Site YPitch
    {
        EL<TEdit>("TfOffSet", "edPitchX1")->Visible  =bVisible;
    }
    EL<TLabel>("TfOffSet", "lblPitchX2")->Visible     =bVisible;
    EL<TLabel>("TfOffSet", "lblPitchY")->Visible      =bVisible;
    EL<TEdit>("TfOffSet", "edPitchX2")->Visible      =bVisible;
    EL<TEdit>("TfOffSet", "edPitchY")->Visible       =bVisible;
    EL<TLabel>("TfOffSet", "lblPitchX2Range")->Visible=bVisible;
    EL<TLabel>("TfOffSet", "lblPitchYRange")->Visible =bVisible;
    if(USE_PICKER_COUNT==ep16Picker)
    {
        EL<TLabel>("TfOffSet", "lblPitchX3")->Visible     =bVisible;
        EL<TLabel>("TfOffSet", "lblPitchX4")->Visible     =bVisible;
        EL<TEdit>("TfOffSet", "edPitchX3")->Visible      =bVisible;
        EL<TEdit>("TfOffSet", "edPitchX4")->Visible      =bVisible;
    }
    else
    {
        EL<TLabel>("TfOffSet", "lblPitchX3")->Visible     =false;
        EL<TLabel>("TfOffSet", "lblPitchX4")->Visible     =false;
        EL<TEdit>("TfOffSet", "edPitchX3")->Visible      =false;
        EL<TEdit>("TfOffSet", "edPitchX4")->Visible      =false;
    }

}

// golden cOffSet.cpp:888  TfOffSet::ShowOneByOneOffSet()
static void OS_ShowOneByOneOffSet()
{

    if(iNowOffsetSel<OfsLoader || iNowOffsetSel>=OfsTotal)
        return;

    if(LastSet.iLanguageCountry==0)
    {
        EL<TLabel>("TfOffSet", "labWarningForZ")->Caption="*. In arm Z axis offset are same as Loader";
        EL<TLabel>("TfOffSet", "lblOutArm")->Caption="*. Out arm Z axis offset are same as Auto1";
    }
    else
    {
        EL<TLabel>("TfOffSet", "labWarningForZ")->Caption="*. In Arm Z軸補償在Loader";
        EL<TLabel>("TfOffSet", "lblOutArm")->Caption="*. Out Arm Z軸補償在Auto 1";
    }

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                      //Steven 20141029 : XY-Pitch
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //QQ
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be  ||                      //Ztex 2023.12.06 Add HT-1032
               USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                    //Ztex 2024.02.24 Add HT-1132
            {
                if((iNowOffsetSel>=OfsOutSh1 &&
                    iNowOffsetSel<=OfsFix6) ||                                  //Steven 20230907 : For HT-9011UC Fix3 --> Fix6
                    iNowOffsetSel==OfsRotate_Out    ||
                    iNowOffsetSel==OfsTopView       ||
                    iNowOffsetSel==OfsPADView       ||                          //wei 20160617 Vitrox
                    iNowOffsetSel==OfsBGAView       ||                          //wei 20160617 Vitrox
                    iNowOffsetSel==OfsScanAOI       )                           //Ifor 20191229 : add
                {
                    if(i==iOutArmYBase && j==iOutArmXBase)
                    {
                        MyPickLab[i][j]    ->Visible=false;
                        MyPickEdit[i][j]   ->Visible=false;
                        MyPickEdit[i][j]   ->Text   =0;
                        MyRelsLab[i][j]    ->Visible=false;
                        MyRelsEdit[i][j]   ->Visible=false;
                        MyRelsEdit[i][j]   ->Text   =0;
                        continue;
                    }
                }
                else
                {
                    if(i==iInArmYBase && j==iInArmXBase)
                    {
                        MyPickLab[i][j]    ->Visible=false;
                        MyPickEdit[i][j]   ->Visible=false;
                        MyPickEdit[i][j]   ->Text   =0;
                        MyRelsLab[i][j]    ->Visible=false;
                        MyRelsEdit[i][j]   ->Visible=false;
                        MyRelsEdit[i][j]   ->Text   =0;
                        continue;
                    }
                }
            }
            else
            {
                if(i==iInArmYBase && j==iInArmXBase)
                {
                    MyPickLab[i][j]    ->Visible=false;
                    MyPickEdit[i][j]   ->Visible=false;
                    MyPickEdit[i][j]   ->Text   =0;
                    MyRelsLab[i][j]    ->Visible=false;
                    MyRelsEdit[i][j]   ->Visible=false;
                    MyRelsEdit[i][j]   ->Text   =0;
                    continue;
                }
            }

            if(IniConfig.bE33InOutArmZOffsetSameOne==true)
            {
                if((iNowOffsetSel>=OfsLoader && iNowOffsetSel<=OfsInSh2) ||     //jou 2014-03-11 add AutoClean & In Rotate
                    iNowOffsetSel==OfsAutoClean ||
                    iNowOffsetSel==OfsOCR ||
                    iNowOffsetSel==OfsRotate_In ||
                   (iNowOffsetSel>=OfsLoaderB && iNowOffsetSel<=OfsInSh2RB) ||
                    iNowOffsetSel==OfsAutoSh1 ||
                    iNowOffsetSel==OfsAutoSh2 ||                                //Steven 20140811 : For 32 Site Loader and Shuttle Offset    //wei 20160914 Auto Shuttle Sensor
                    iNowOffsetSel==OfsPreciser ||                               //Frank 20180512 : modify 1 Dut 1 Motor
                    iNowOffsetSel==OfsBottom2DID)                               //JerryYang 20260223 : fix bottom 2DID offset
                {
                    EL<TLabel>("TfOffSet", "labWarningForZ")  ->Visible=true;
                    EL<TLabel>("TfOffSet", "lblOutArm")       ->Visible=false;
                }
                else
                {
                    EL<TLabel>("TfOffSet", "labWarningForZ")  ->Visible=false;
                    EL<TLabel>("TfOffSet", "lblOutArm")       ->Visible=true;
                }

                if(iNowOffsetSel>=OfsTopView && iNowOffsetSel<=OfsBGAView)      //wei 20160617 Vitrox
                {
                    MyRelsLab[i][j] ->Visible=true;
                    MyRelsEdit[i][j]->Visible=true;
                    MyPickLab[i][j] ->Visible=false;
                    MyPickEdit[i][j]->Visible=false;
                    EL<TLabel>("TfOffSet", "labWarningForZ")  ->Visible=false;
                    EL<TLabel>("TfOffSet", "lblOutArm")       ->Visible=false;
                }
                else if(iNowOffsetSel==OfsLoader ||                             //kevin 20130604 add rotate
                        iNowOffsetSel==OfsAuto1)                                //ChungHung 20140506 alter 參數放錯
                {
                    MyPickLab[i][j] ->Visible=true;
                    MyPickEdit[i][j]->Visible=true;
                    MyRelsLab[i][j] ->Visible=true;
                    MyRelsEdit[i][j]->Visible=true;
                }
                else if(iNowOffsetSel==OfsScanAOI)                              //Ifor 20191229 : add
                {
                    MyRelsLab[i][j] ->Visible=true;
                    MyRelsEdit[i][j]->Visible=true;
                    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)            //Jimmychiu 20240322 : Top & Bottom Inspect
                    {
                        MyPickLab[i][j]->Visible=true;
                        MyPickEdit[i][j]->Visible=true;
                    }
                    else
                    {
                        MyPickLab[i][j] ->Visible=false;
                        MyPickEdit[i][j]->Visible=false;
                    }
                    EL<TLabel>("TfOffSet", "labWarningForZ")  ->Visible=false;
                    EL<TLabel>("TfOffSet", "lblOutArm")       ->Visible=false;
                }
                else
                {
                    MyPickLab[i][j] ->Visible=false;
                    MyPickEdit[i][j]->Visible=false;
                    MyRelsLab[i][j] ->Visible=false;
                    MyRelsEdit[i][j]->Visible=false;
                }
            }
            else
            {
                EL<TLabel>("TfOffSet", "labWarningForZ")      ->Visible=false;
                EL<TLabel>("TfOffSet", "lblOutArm")           ->Visible=false;
                //------------------------------------------
                //pick Group
                //------------------------------------------
                if(iNowOffsetSel==OfsAutoClean  ||
                   iNowOffsetSel==OfsInSh1      ||
                   iNowOffsetSel==OfsInSh2      ||
                   (iNowOffsetSel>=OfsAuto1 &&                                  //InSH1 InSH2 Auto1~Fix3
                    iNowOffsetSel<=OfsFix6) ||                                  //Steven 20230907 : For HT-9011UC Fix3 --> Fix6
                   (iNowOffsetSel>=OfsInSh1LB && iNowOffsetSel<=OfsInSh2RB) ||  //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                   iNowOffsetSel==OfsAutoSh1 || iNowOffsetSel==OfsAutoSh2 ||    //wei 20160914 Auto Shuttle Sensor
                   iNowOffsetSel==OfsPreciser)
                {
                    if(iNowOffsetSel==OfsFix3 &&
                       CosFunction.bUseFix3FullTray &&                          //Ifor 20161222 (jou) add 若有開啟Fix3 Full Tray 客戶功能 顯示Fix3 Pickup Offset
                       FIX3_FULL_PLACE==Fix3K_Uninstall)                        //Steven 20250911 : for 硬體 fix3 full 可以關閉
                    {
                        MyPickLab[i][j] ->Visible=true;
                        MyPickEdit[i][j]->Visible=true;
                    }
                    else
                    {
                        MyPickLab[i][j] ->Visible=false;
                        MyPickEdit[i][j]->Visible=false;
                        MyPickEdit[i][j]->Text=0;
                    }
                }
                else if(iNowOffsetSel>=OfsTopView && iNowOffsetSel<=OfsBGAView) //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
                {
                    MyPickLab[i][j] ->Visible=false;
                    MyPickEdit[i][j]->Visible=false;
                    MyPickEdit[i][j]->Text=0;
                }
                else if(iNowOffsetSel==OfsScanAOI)                              //Ifor 20191229 : add
                {
                    MyPickLab[i][j] ->Visible=false;
                    MyPickEdit[i][j]->Visible=false;
                    MyPickEdit[i][j]->Text=0;
                }
                else
                {
                    MyPickLab[i][j]->Visible=true;
                    MyPickEdit[i][j]->Visible=true;
                }
                //------------------------------------------
                //Release Group
                //------------------------------------------
                if(iNowOffsetSel==OfsAutoClean ||
                   iNowOffsetSel==OfsLoader    ||
                   iNowOffsetSel==OfsOutSh1    ||
                   iNowOffsetSel==OfsOutSh2    ||                               //Loader and OutSH1 OutSH2
                   iNowOffsetSel==OfsLoaderB   ||                               //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                   iNowOffsetSel==OfsAutoSh1   ||
                   iNowOffsetSel==OfsAutoSh2     )                              //wei 20160914 Auto Shuttle Sensor
                {
                    MyRelsLab[i][j] ->Visible=false;
                    MyRelsEdit[i][j]->Visible=false;
                    MyRelsEdit[i][j]->Text=0;
                }
                else
                {
                    MyRelsEdit[i][j]->Visible=true;
                    MyRelsLab[i][j] ->Visible=true;
                }
            }

            bool bShowSingleOffsetSet = false;
            if(ArmSpeed[OutArm].bVariModeFIX==true ||                           //Eastsun 20251224 : 吸嘴獨立offset功能
               ArmSpeed[InArm].bVariModeFIX ==true)                             //Eastsun 20251224 : 只有在IN OUT ARM
            {                                                                   //Eastsun 20251224 : 開啟Fix才有效
                if((iNowOffsetSel>=OfsAuto1     &&                              //Eastsun 20251224 : 選擇在loader/unloader區才可顯示
                   iNowOffsetSel<=OfsFix6)      ||
                   iNowOffsetSel==OfsLoader     )
                {
                    MySingleOffsetTEditX[i][j]->Visible=true;
                    MySingleOffsetTEditY[i][j]->Visible=true;
                    MySingleOffsetLabX[i][j]->Visible=true;
                    MySingleOffsetLabY[i][j]->Visible=true;
                    bShowSingleOffsetSet = true;
                }
            }

            if(!bShowSingleOffsetSet)                                           //Eastsun 20251224 : 其他區域沒有寫offset所以不寫
            {
                MySingleOffsetTEditX[i][j]->Visible=false;
                MySingleOffsetTEditY[i][j]->Visible=false;
                MySingleOffsetLabX[i][j]->Visible=false;
                MySingleOffsetLabY[i][j]->Visible=false;
            }
        }
    }

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==false)
    {
        switch(iNowOffsetSel)                                                   //Sam 20230508 : 整理 Offset 顯示
        {
            case OfsAutoClean:
            case OfsHP1:
            case OfsHP2:
            case OfsPreciser:                                                   //Sam 20230508 : 修正 Preciser 顯示
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                             //kevin 20190212
                    OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);         //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsInSh1:
            case OfsInSh2:
                if(IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==false) //RogerYang 20251225 : IN/OUT ARM支援不同模組
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=false;
                }
                else
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=true;
                }
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);
                break;
            case OfsAuto1:
            case OfsAuto2:
            case OfsAuto3:
            case OfsAuto4:                                                      //Steven 20230907 : For HT-9011UC
            case OfsAuto5:
            case OfsAuto6:
            case OfsFix1:
            case OfsFix2:
            case OfsFix3:
            case OfsFix4:                                                       //Steven 20230907 : For HT-9011UC
            case OfsFix5:
            case OfsFix6:
                if(IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==false &&
                   CosFunction.bUseFix3FullTray==false &&                       //JerryYang 20150910 整盤功能需要Tray Pickup IC Offset   //Ifor 20161222 (jou) add Fix3 Full Tray Offset
                   AUTO3_IS_MAGAZINE==0)                                        //JerryYang 20220909 : add magazine
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=false;
                }
                else                                                            //RogerYang 20170731 (wei) 修正功能重複開啟後 PickUp 設定就不見問題
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=true;
                }
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_OUT_Y_IS_AUTO_PITCH==true);            //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsLoader:
    if(FileRW_Offset_IsRun2DCheck()==true)   // golden cOffSet.cpp:1173-1173 取代：移植樹 fContact 是 TfContactShim（atester_shims.h，IsRun2DCheck 離線回 false）；真表單 fContactForm（forms/fContact.h:1573 ACTIVE）→ 入口檔轉接
#if 0 // GATE (S12-C save) golden cOffSet.cpp:1173-1173 -- 移植樹 fContact 是 TfContactShim（atester_shims.h，IsRun2DCheck 離線回 false）；真表單 fContactForm（forms/fContact.h:1573 ACTIVE）→ 入口檔轉接
                if(fContact->IsRun2DCheck()==true)          //JerryYang 20250220 : 2DID硬體順序檢查功能
#endif // GATE (S12-C save)
                    EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                else
                    EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);             //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsOutSh1:
            case OfsOutSh2:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_OUT_Y_IS_AUTO_PITCH==true);            //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsOCR:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);             //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsInSh1LB:
            case OfsInSh1RA:
            case OfsInSh1RB:
            case OfsInSh2LB:
            case OfsInSh2RA:
            case OfsInSh2RB:
            case OfsOutSh1LB:
            case OfsOutSh1RA:
            case OfsOutSh1RB:
            case OfsOutSh2LB:
            case OfsOutSh2RA:
            case OfsOutSh2RB:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_OUT_Y_IS_AUTO_PITCH==true);
                break;
            case OfsLoaderB:
            case OfsAutoSh1:
            case OfsAutoSh2:
            case OfsInSh1LB_AutoClean:
            case OfsInSh1RA_AutoClean:
            case OfsInSh1RB_AutoClean:
            case OfsInSh2LB_AutoClean:
            case OfsInSh2RA_AutoClean:
            case OfsInSh2RB_AutoClean:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);
                break;
            case OfsRotate_In:
            case OfsRotate_Out:
            case OfsInSh1_AutoClean:
            case OfsInSh2_AutoClean:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);
                break;
            case OfsScanAOI:
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)            //Jimmychiu 20240322 : Top & Bottom Inspect
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=true;
                }
                else
                {
                    EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                }

                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(false);
                break;
            case OfsBottom2DID:                                                 //JerryYang 20260223 : fix bottom 2DID offset
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true); //Steven 20140510 : XY變距
                break;
        }
    }
    else
    {
        switch(iNowOffsetSel)                                                   //jou 2010-12-20 Pitch & Z 縮減為一個
        {
            case OfsLoader:                                                     //Loader
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
    if(FileRW_Offset_IsRun2DCheck()==true)   // golden cOffSet.cpp:1261-1261 取代：同上
#if 0 // GATE (S12-C save) golden cOffSet.cpp:1261-1261 -- 同上
                if(fContact->IsRun2DCheck()==true)                              //JerryYang 20250220 : 2DID硬體順序檢查功能
#endif // GATE (S12-C save)
                    EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                else
                    EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true); //Steven 20140510 : XY變距    //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsOutSh1:                                                     //Out Shuttle 1
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_OUT_Y_IS_AUTO_PITCH==true); //Steven 20140510 : XY變距   //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsHP1:                                                        //Hotplate 1
            case OfsPreciser:                                                   //Sam 20230508 : 修正 Preciser 顯示
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsHP2:                                                        //Hotplate 2
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsOutSh2:                                                     //Out Shuttle 2
            case OfsLoaderB:                                                    //20 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsInSh2:                                                      //In Shuttle 2
            case OfsAuto2:                                                      //Auto 2
            case OfsAuto3:                                                      //Auto 3
            case OfsAuto4:                                                      //Auto 4    //Steven 20230907 : For HT-9011UC
            case OfsAuto5:                                                      //Auto 5
            case OfsAuto6:                                                      //Auto 6
            case OfsFix2:                                                       //Fix 2
                if(FIX3_FULL_PLACE==Fix3K_UseCylinder46LA)                      //ChungHung 20140722 add for HT9046LA
                {
                    EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                    EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                    EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                    OS_SetXYPitchVCLVisible(false);                                //Steven 20140510 : XY變距
                    break;
                }
            case OfsFix3:                                                       //Fix 3
            case OfsFix4:                                                       //Fix 4     //Steven 20230907 : For HT-9011UC
            case OfsFix5:                                                       //Fix 5
            case OfsFix6:                                                       //Fix 6
            case OfsInSh1LB:                                                    //21 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
            case OfsInSh1RA:                                                    //22 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
            case OfsInSh1RB:                                                    //23 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
            case OfsInSh2LB:                                                    //24 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
            case OfsInSh2RA:                                                    //25 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
            case OfsInSh2RB:                                                    //26 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=false;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsInSh1:                                                      //In Shuttle 1
            case OfsAuto1:                                                      //Auto 1
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                OS_SetXYPitchVCLVisible(USE_OUT_Y_IS_AUTO_PITCH==true);                   //Steven 20140510 : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
                break;
            case OfsFix1:                                                       //Fix 1
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsTopView:                                                    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=true;
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsPADView:                                                    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
            case OfsBGAView:                                                    //wei 20160617 Vitrox
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=false;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
            case OfsScanAOI:                                                    //Ifor 20191229 : add
                EL<TEdit>("TfOffSet", "edPitchX1")->Visible=true;
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)            //Jimmychiu 20240322 : Top & Bottom Inspect
                {
                    EL<TEdit>("TfOffSet", "edPickUp")->Visible=true;
                }
                else
                {
                    EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                }
                break;
            case OfsBottom2DID:                                                 //JerryYang 20260223 : fix bottom 2DID offset
                EL<TEdit>("TfOffSet", "edRelease")->Visible=true;
                EL<TEdit>("TfOffSet", "edPickUp") ->Visible=false;
                OS_SetXYPitchVCLVisible(USE_IN_Y_IS_AUTO_PITCH==true);
                OS_SetXYPitchVCLVisible(false);                                    //Steven 20140510 : XY變距
                break;
        }
    }

    if(USE_PICKER_COUNT==0)                                                     //Steven 20161117 : for HT-9045S
    {
        EL<TEdit>("TfOffSet", "EdtRelsC") ->Visible=false;
        EL<TEdit>("TfOffSet", "EdtRelsD") ->Visible=false;
        EL<TEdit>("TfOffSet", "EdtRelsG") ->Visible=false;
        EL<TEdit>("TfOffSet", "EdtRelsH") ->Visible=false;
        EL<TEdit>("TfOffSet", "EditPickC")->Visible=false;
        EL<TEdit>("TfOffSet", "EditPickD")->Visible=false;
        EL<TEdit>("TfOffSet", "EditPickG")->Visible=false;
        EL<TEdit>("TfOffSet", "EditPickH")->Visible=false;
    }

    if(iNowOffsetSel==OfsPreciser)                                              //Frank 20180410 (Steven) : InArm Preciser Station
    {
        EL<TPanel>("TfOffSet", "palPreciser")->Visible=true;
    }
    else
    {
        EL<TPanel>("TfOffSet", "palPreciser")->Visible=false;
    }

    EL<TLabel>("TfOffSet", "lblRelease")      ->Visible=EL<TEdit>("TfOffSet", "edRelease")->Visible;
    EL<TLabel>("TfOffSet", "lblReleaseRange") ->Visible=EL<TEdit>("TfOffSet", "edRelease")->Visible;                               //kevin 20140320 秀極限範圍
    EL<TLabel>("TfOffSet", "lblPickUp")       ->Visible=EL<TEdit>("TfOffSet", "edPickUp") ->Visible;
    EL<TLabel>("TfOffSet", "lblPickUpRange")  ->Visible=EL<TEdit>("TfOffSet", "edPickUp") ->Visible;                               //kevin 20140320 秀極限範圍
    EL<TLabel>("TfOffSet", "lblPitchX1")      ->Visible=EL<TEdit>("TfOffSet", "edPitchX1")->Visible;
    EL<TLabel>("TfOffSet", "lblPitchX1Range") ->Visible=EL<TEdit>("TfOffSet", "edPitchX1")->Visible;                               //kevin 20140320 秀極限範圍
    if(CUSTOMER_CODE==CC_ASE_CL && AccessLevel>=iDefEngineerLevel)              //JerryYang 20250120 : modify
    {
        EL<TSpeedButton>("TfOffSet", "sbZcalibration")->Visible=true;
    }
    else
    {
        EL<TSpeedButton>("TfOffSet", "sbZcalibration")->Visible=false;
    }

}

// golden cOffSet.cpp:1406  TfOffSet::SaveFile(int iSelPartData, int SpecialMode, bool bReset)
static bool OS_SaveFile(int iSelPartData, int SpecialMode, bool bReset)
{
    filerw::ELMark("SaveFile");   // FileRW：存檔流程 trace（ack.trace）

    if(iSelPartData<0 || iSelPartData>OfsTotal)
        return false;
    AnsiString szDir="", szFilePath="";

    if(CosFunction.bSaveOffsetByMachine &&
       IniConfig.bA57_3SaveOffsetByMachine)                                     //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir=GetOffsetPath();
    }
    szFilePath=szDir;

    bool bResult=OS_SaveSetupFile(szDir, szFilePath, iSelPartData, SpecialMode, bReset);//kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(LastFileName, "JOBFILE");                      //kevin 20150105  Start 另存 jobfile
    #endif
    return bResult;

}

// golden cOffSet.cpp:1470  TfOffSet::SaveSetupFile(AnsiString szDir, AnsiString szFilePath, int iSelPartData, int SpecialMode, bool bReset)
static bool OS_SaveSetupFile(AnsiString szDir, AnsiString szFilePath, int iSelPartData, int SpecialMode, bool bReset)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    bool FilePathErr=false;
    bool bOverLimitation=false;
    bool bUnderLimitation=false;
    double dPickup, dRelease;
    int  iNum;
    AnsiString str;

    int iZLimitHigh;
    int iZLimitLow;
    if(iSelPartData==OfsAuto1 ||                                                //Steven 20210317 : 通富微要求unloader獨立設置
       iSelPartData==OfsAuto2 ||
       iSelPartData==OfsAuto3 ||
       iSelPartData==OfsAuto4 ||                                                //Steven 20230907 : For HT-9011UC
       iSelPartData==OfsAuto5 ||
       iSelPartData==OfsAuto6 ||
       iSelPartData==OfsFix1  ||
       iSelPartData==OfsFix2  ||
       iSelPartData==OfsFix3  ||
       iSelPartData==OfsFix4  ||                                                //Steven 20230907 : For HT-9011UC
       iSelPartData==OfsFix5  ||
       iSelPartData==OfsFix6)
    {
        iZLimitHigh=InputLimit.iOffsetUnloaderZHigh;
        iZLimitLow =InputLimit.iOffsetUnloaderZLow;
    }
    else
    {
        iZLimitHigh=InputLimit.iOffsetZHigh;
        iZLimitLow =InputLimit.iOffsetZLow;
    }

    MyForceDirectories(szDir);

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.03.13 Add Low Temp Offset
        szDir+=Tri_Position_Offset();
    else
        szDir+="\\Position Offset.Data";

    if(!FileExists(szDir))
        FilePathErr=true;

    if(bReset || FilePathErr)                                                   //save mode at first step
    {
        for(int k=0; k<OfsTotal; k++)                                           //only make ini mode
        {
            if(AUTO_EMPTY_COLOR<3)                                              //Steven 20230907 : For HT-9011UC
            {
                if(k==OfsAuto4 || k==OfsAuto5 || k==OfsAuto6 ||
                   k==OfsFix4  || k==OfsFix5  || k==OfsFix6)
                {
                    continue;                                                   //沒用到的不存檔
                }
            }

            WriteIniData(szDir, CapStr[k], "One By One", 1);
            WriteIniData(szDir, CapStr[k], "VariableY",  0.0);                  //ChungHung 20131231 alter AutoYPitch
            WriteIniData(szDir, CapStr[k], "Variable",   0.0);
            WriteIniData(szDir, CapStr[k], "Variable2",  0.0);                  //Steven 20131002 : XY變距
            WriteIniData(szDir, CapStr[k], "Hand X",     0.0);
            WriteIniData(szDir, CapStr[k], "Hand Y",     0.0);
            WriteIniData(szDir, CapStr[k], "PickUp",     0.0);
            WriteIniData(szDir, CapStr[k], "Place",      0.0);
            for(int i=0; i<InArmSuck.iMotCol; i++)                              //8=In/out arm suction
            {
                for(int j=0; j<InArmSuck.iMotRow; j++)
                {
                    WriteIniData(szDir, CapStr[k], str.sprintf("Hand %cX",  'A'+(i*2)+j), 0.0);
                    WriteIniData(szDir, CapStr[k], str.sprintf("Hand %cY",  'A'+(i*2)+j), 0.0);
                    WriteIniData(szDir, CapStr[k], str.sprintf("PickUp %c", 'A'+(i*2)+j), 0.0);
                    WriteIniData(szDir, CapStr[k], str.sprintf("Place %c",  'A'+(i*2)+j), 0.0);
                }
            }
        }

        if(bReset)
            return false;

        for(int i=0; i<2; i++)
        {
            str="";
            str.sprintf("Test Arm%d", i+1);
            WriteIniData(szDir, str, "Pick Up",       0.0);
            WriteIniData(szDir, str, "Place",         0.0);
            WriteIniData(szDir, str, "Contact",       0.0);
            WriteIniData(szDir, str, "Shuttle Right", 0.0);
            WriteIniData(szDir, str, "Busy SH Halt",  0.0);
            WriteIniData(szDir, str, "ART-Place",     0.0);                     //kevin 20170831 (Steven) ART offset
        }

        for(int i=0; i<=tOfsAuto6; i++)
        {
            WriteIniData(szDir, SpecialOffSetName[i], "Place", 0.0);
            WriteIniData(szDir, SpecialOffSetName[i], "ART-Place", 0.0);        //kevin 20170831 (Steven) ART offset
            WriteIniData(szDir, SpecialOffSetName[i], "Loader Z", 0.0);         //Steven 20190813 : 入Tray改用步進馬達
        }
        FilePathErr=false;
    }

    if(SpecialMode & iSaveSpecial)
    {
        if(iSelPartData<=tOfsAuto6)
        {
            if(AUTO_EMPTY_COLOR<3 &&                                            //Steven 20230907 : For HT-9011UC
               (SpecialMode==tOfsAuto4 ||
                SpecialMode==tOfsAuto5 ||
                SpecialMode==tOfsAuto6))
            {
            }
            else
            {
                WriteIniData(szDir, SpecialOffSetName[iSelPartData], "Place",       EL<TEdit>("TfOffSet", "edtTrayArmX")->Text);
                WriteIniData(szDir, SpecialOffSetName[iSelPartData], "ART-Place",   EL<TEdit>("TfOffSet", "edtARTPlace")->Text);     //kevin 20170831 (Steven) ART offset
                WriteIniData(szDir, SpecialOffSetName[iSelPartData], "Loader Z",    EL<TEdit>("TfOffSet", "edlLoadZ")->Text);        //Steven 20190813 : 入Tray改用步進馬達
            }
        }
        else
        {
            if(iSelPartData==tOfsIndex1)
            {
                iNum=1;
            }
            else if(iSelPartData==tOfsIndex2)
            {
                iNum=2;
            }
            else
            {
                ShowErrorMessage("WAR1682", 0, MMSystem, 0, szDir);
                return true;
            }

            str="";
            str.sprintf("Test Arm%d", iNum);                                    //Steven 20140825 : iNum --> iNum+1
            WriteIniData(szDir, str, "Pick Up",       EL<TEdit>("TfOffSet", "IndexArmOffSet1")->Text);
            WriteIniData(szDir, str, "Place",         EL<TEdit>("TfOffSet", "IndexArmOffSet2")->Text);
            if(CosFunction.bContactHaveOffset==false)                           //ChungHung 20140327 add by Customer 強制Contact Offset為0
            {
                WriteIniData(szDir, str, "Contact",   0);
            }
            else
            {
                WriteIniData(szDir, str, "Contact",   EL<TEdit>("TfOffSet", "IndexArmOffSet3")->Text);
            }

            szDir=szFilePath;                                                   //kevin 20150105
            if(Tri_Temp_Machine==1)                                             //Ztex 2024.03.13 Add Low Temp Offset
                szDir+=Tri_Position_Offset();
            else if(LastSet.iTemperature==Tempture_Hot)
                szDir+="\\Position Offset Hot.Data";
            else
                szDir+="\\Position Offset.Data";

            WriteIniData(szDir, str, "Busy SH Halt",  EL<TEdit>("TfOffSet", "IndexArmOffSet4")->Text);
            WriteIniData(szDir, str, "Shuttle Right", EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Text);
            WriteIniData(szDir, str, "Shuttle Left",  EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Text);
            WriteIniData(szDir, str, "Shuttle for 2D",  EL<TEdit>("TfOffSet", "edShtFor2D")->Text);      //Steven 20151218 : Offset for 2d reader
        }
    }

    bOverLimitation=false;
    bUnderLimitation=false;
    if(SpecialMode & iSaveStander)
    {
        if(AUTO_EMPTY_COLOR<3 &&                                                //Steven 20230907 : For HT-9011UC
           (iSelPartData==OfsAuto4 || iSelPartData==OfsAuto5 || iSelPartData==OfsAuto6 ||
            iSelPartData==OfsFix4  || iSelPartData==OfsFix5  || iSelPartData==OfsFix6))
        {
            //沒用到的不存檔
        }
        else
        {
            szDir=szFilePath;
            if(Tri_Temp_Machine==1)                                             //Ztex 2024.03.13 Add Low Temp Offset
                szDir+=Tri_Position_Offset();
            else if(LastSet.iTemperature==Tempture_Hot && ((iSelPartData==OfsInSh1    || iSelPartData==OfsInSh2 ||
                                                       iSelPartData==OfsOutSh1   || iSelPartData==OfsOutSh2) ||
                                                      (iSelPartData>=OfsInSh1LB  && iSelPartData<=OfsInSh2RB) ||    //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                                                      (iSelPartData>=OfsOutSh1LB && iSelPartData<=OfsOutSh2RB)))    //Steven 20190802 : For 32 Site Out Shuttle Offset
                szDir+="\\Position Offset Hot.Data";
            else
                szDir+="\\Position Offset.Data";

            WriteIniData(szDir, CapStr[iSelPartData], "Hand X",   EL<TEdit>("TfOffSet", "edArmX")->Text);
            WriteIniData(szDir, CapStr[iSelPartData], "Hand Y",   EL<TEdit>("TfOffSet", "edArmY")->Text);

            szDir=szFilePath;

            if(Tri_Temp_Machine==1)                                             //Ztex 2024.03.13 Add Low Temp Offset
                szDir+=Tri_Position_Offset();
            else
                szDir+="\\Position Offset.Data";

            //Steven 20210324 : 超出上下限要提示
            //==>
            dPickup =atof(EL<TEdit>("TfOffSet", "edPickUp")->Text.c_str());
            dRelease=atof(EL<TEdit>("TfOffSet", "edRelease")->Text.c_str());

            if(EL<TEdit>("TfOffSet", "edPickUp")->Visible==true)
            {
                if(dPickup<double(InputLimit.iOffsetZLow))
                {
                    bUnderLimitation=true;
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edPickUp")->Color=clYellow;
                }
                else if(dPickup>double(InputLimit.iOffsetZHigh))
                {
                    bOverLimitation=true;
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edPickUp")->Color=clRed;
                }
                else
                {
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edPickUp")->Color=clWhite;
                }
            }
            else
            {
                ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edPickUp")->Color=clWhite;
            }

            if(EL<TEdit>("TfOffSet", "edRelease")->Visible==true)
            {
                if(dRelease<double(iZLimitLow))
                {
                    bUnderLimitation=true;
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edRelease")->Color=clYellow;
                }
                else if(dRelease>double(iZLimitHigh))
                {
                    bOverLimitation=true;
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edRelease")->Color=clRed;
                }
                else
                {
                    ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edRelease")->Color=clWhite;
                }
            }
            else
            {
                ;   // UI-only（HTML 端處理）：EL<TEdit>("TfOffSet", "edRelease")->Color=clWhite;
            }
            //<==
            //Steven 20210324 : 超出上下限要提示

            EL<TEdit>("TfOffSet", "edPickUp")->Text =CheckRange(dPickup, double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));  //JerryYang 20190328 Offset上下限保護
            EL<TEdit>("TfOffSet", "edRelease")->Text=CheckRange(dRelease, double(iZLimitHigh), double(iZLimitLow));
            WriteIniData(szDir, CapStr[iSelPartData], "One By One", 1);
            WriteIniData(szDir, CapStr[iSelPartData], "VariableY",EL<TEdit>("TfOffSet", "edPitchY")->Text);  //ChungHung 20131231 alter AutoYPitch
            WriteIniData(szDir, CapStr[iSelPartData], "Variable4",EL<TEdit>("TfOffSet", "edPitchX4")->Text); //Steven 20131002 : XY變距
            WriteIniData(szDir, CapStr[iSelPartData], "Variable3",EL<TEdit>("TfOffSet", "edPitchX3")->Text); //Steven 20131002 : XY變距
            WriteIniData(szDir, CapStr[iSelPartData], "Variable2",EL<TEdit>("TfOffSet", "edPitchX2")->Text); //Steven 20131002 : XY變距
            WriteIniData(szDir, CapStr[iSelPartData], "Variable", EL<TEdit>("TfOffSet", "edPitchX1")->Text);
            WriteIniData(szDir, CapStr[iSelPartData], "PickUp",   EL<TEdit>("TfOffSet", "edPickUp")->Text);
            WriteIniData(szDir, CapStr[iSelPartData], "Place",    EL<TEdit>("TfOffSet", "edRelease")->Text);

            if(iSelPartData==OfsPreciser)                                                       //wei 20240507 避免存檔錯誤
            {
                WriteIniData(szDir, "Preciser", "Preciser Open",      EL<TEdit>("TfOffSet", "edPreciserOpen")->Text);    //JerryYang 20180820 add
                WriteIniData(szDir, "Preciser", "Preciser Close",     EL<TEdit>("TfOffSet", "edPreciserClose")->Text);
            }

            for(int i=0; i<InArmSuck.iMotCol; i++)                              //8=In/out arm suction
            {
                for(int j=0; j<InArmSuck.iMotRow; j++)
                {
                    if(IniConfig.bE33InOutArmZOffsetSameOne==false ||
                       ((IniConfig.bE33InOutArmZOffsetSameOne==true) &&
                        (iSelPartData==OfsLoader  || iSelPartData==OfsAuto1   ||
                         iSelPartData==OfsPADView || iSelPartData==OfsBGAView ||
                         iSelPartData==OfsScanAOI || iSelPartData==OfsBottom2DID)))  //KaiChen 20200414 ：新增Bottom2DID OffSet  //Steven 20260316 : Fix condition
                    {
                        //Steven 20210324 : 超出上下限要提示
                        //==>
                        dPickup =atof(MyPickEdit[j][i]->Text.c_str());
                        dRelease=atof(MyRelsEdit[j][i]->Text.c_str());
                        if(MyPickEdit[j][i]->Visible==true)
                        {
                            if(dPickup<double(InputLimit.iOffsetZLow))
                            {
                                bUnderLimitation=true;
                                ;   // UI-only（HTML 端處理）：MyPickEdit[j][i]->Color=clYellow;
                            }
                            else if(dPickup>double(InputLimit.iOffsetZHigh))
                            {
                                bOverLimitation=true;
                                ;   // UI-only（HTML 端處理）：MyPickEdit[j][i]->Color=clRed;
                            }
                            else
                            {
                                ;   // UI-only（HTML 端處理）：MyPickEdit[j][i]->Color=clWhite;
                            }
                        }
                        else
                        {
                            ;   // UI-only（HTML 端處理）：MyPickEdit[j][i]->Color=clWhite;
                        }

                        if(MyPickEdit[j][i]->Visible==true)
                        {
                            if(dRelease<double(iZLimitLow))
                            {
                                bUnderLimitation=true;
                                ;   // UI-only（HTML 端處理）：MyRelsEdit[j][i]->Color=clYellow;
                            }
                            else if(dRelease>double(iZLimitHigh))
                            {
                                bOverLimitation=true;
                                ;   // UI-only（HTML 端處理）：MyRelsEdit[j][i]->Color=clRed;
                            }
                            else
                            {
                                ;   // UI-only（HTML 端處理）：MyRelsEdit[j][i]->Color=clWhite;
                            }
                        }
                        else
                        {
                            ;   // UI-only（HTML 端處理）：MyRelsEdit[j][i]->Color=clWhite;
                        }
                        //<==
                        //Steven 20210324 : 超出上下限要提示

                        MyPickEdit[j][i]->Text=CheckRange(dPickup, double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));   //JerryYang 20190328 Offset上下限保護
                        MyRelsEdit[j][i]->Text=CheckRange(dRelease, double(iZLimitHigh), double(iZLimitLow));

                        //WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Hand %cX",  'A'+(i*2)+j), MyXOffsetEdit[j][i]->Text);
                        //WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Hand %cY",  'A'+(i*2)+j), MyYOffsetEdit[j][i]->Text);
                        //==>
                        WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Hand %cX",  'A'+(i*2)+j), MySingleOffsetTEditX[j][i]->Text);//Eastsun 20251230 : 獨立offset功能 修復
                        WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Hand %cY",  'A'+(i*2)+j), MySingleOffsetTEditY[j][i]->Text);

                        WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("PickUp %c", 'A'+(i*2)+j), MyPickEdit[j][i]->Text);
                        WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Place %c",  'A'+(i*2)+j), MyRelsEdit[j][i]->Text);
                    }
                }
            }
        }
    }

    AddSpace(szDir);
    if(CUSTOMER_CODE==CC_TFME_CHINA)                                            //Steven 20210324 : 超出上下限要提示
    {
        if(bOverLimitation || bUnderLimitation)
        {
            filerw::ELMessage("There are some offset is not inside the limitation!\r And already changed automatically.", "部分補償值超出限制, 已被重置數值!");
            return true;
        }
    }
    return false;

}

// golden cOffSet.cpp:2316  TfOffSet::DoIniDataToForm(int iNowOffsetSel, int SpecialMode)
static void OS_DoIniDataToForm(int iNowOffsetSel, int SpecialMode)
{

    int iNum;

    if(iNowOffsetSel<0)                                                         //jou 2011-11-22 防止讀取到unknow記憶體,iNowOffsetSel=-1
    {
        return;
    }

    OS_ShowOffSetList();
    if(SpecialMode==iSaveStander)
    {
        if(iNowOffsetSel==OfsRotate_In          || iNowOffsetSel==OfsHP1                || iNowOffsetSel==OfsHP2                ||
           iNowOffsetSel==OfsAutoClean          || iNowOffsetSel==OfsOCR                || iNowOffsetSel==OfsInPlacement        ||
           iNowOffsetSel==OfsLoader             || iNowOffsetSel==OfsLoaderB            ||
           iNowOffsetSel==OfsAutoSh1            || iNowOffsetSel==OfsAutoSh2            || iNowOffsetSel==OfsPreciser           || iNowOffsetSel==OfsBottom2DID         ||
           iNowOffsetSel==OfsInSh1              || iNowOffsetSel==OfsInSh1LB            || iNowOffsetSel==OfsInSh1RA            || iNowOffsetSel==OfsInSh1RB            ||
           iNowOffsetSel==OfsInSh2              || iNowOffsetSel==OfsInSh2LB            || iNowOffsetSel==OfsInSh2RA            || iNowOffsetSel==OfsInSh2RB            ||
           iNowOffsetSel==OfsInSh1_AutoClean    || iNowOffsetSel==OfsInSh1LB_AutoClean  || iNowOffsetSel==OfsInSh1RA_AutoClean  || iNowOffsetSel==OfsInSh1RB_AutoClean  ||
           iNowOffsetSel==OfsInSh2_AutoClean    || iNowOffsetSel==OfsInSh2LB_AutoClean  || iNowOffsetSel==OfsInSh2RA_AutoClean  || iNowOffsetSel==OfsInSh2RB_AutoClean)
        {
            if(iNowOffsetSel==OfsOCR)                                           //OCR
                EL<TCheckBox>("TfOffSet", "CheckBox1")->Checked      =true;

            EL<TEdit>("TfOffSet", "edArmX")->Text                =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetX());
            EL<TEdit>("TfOffSet", "edArmY")->Text                =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetY());
            EL<TEdit>("TfOffSet", "edPickUp")->Text              =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetPickUp());
            EL<TEdit>("TfOffSet", "edRelease")->Text             =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetPlace());
            EL<TEdit>("TfOffSet", "edPitchX1")->Text             =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable());

            if(USE_IN_Y_IS_AUTO_PITCH==true ||              //JerryYang 20251218 : IN/OUT ARM支援不同模組
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)
            {
                EL<TEdit>("TfOffSet", "edPitchX2")->Text         =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable2());
            }

            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.25 for HT-1032 AT
            {
                EL<TEdit>("TfOffSet", "edPitchX3")->Text         =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable3());
                EL<TEdit>("TfOffSet", "edPitchX4")->Text         =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable4());
            }

            if(USE_IN_Y_IS_AUTO_PITCH==true)                //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<TEdit>("TfOffSet", "edPitchY")->Text          =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariableY());
            }

            for(int i=0; i<InArmSuck.iMotRow; i++)
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    //MyXOffsetEdit[i][j]->Text  =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetX[i][j]);
                    //MyYOffsetEdit[i][j]->Text  =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetY[i][j]);
                    //==>
                    MySingleOffsetTEditX[i][j]->Text  =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetX[i][j]);  //Eastsun 20251230 : 修復獨立offset功能
                    MySingleOffsetTEditY[i][j]->Text  =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetY[i][j]);

                    MyPickEdit[i][j]->Text     =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPickUpOffSet[i][j]);
                    MyRelsEdit[i][j]->Text     =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPlaceOffSet[i][j]);
                }
            }

            if(iNowOffsetSel==OfsPreciser)                                                  //wei 20240507 避免存檔顯示錯誤
            {
                EL<TEdit>("TfOffSet", "edPreciserOpen")->Text    =FormatFloat("0.00", Offset_File.iPreciserOpen);    //JerryYang 20180820 add
                EL<TEdit>("TfOffSet", "edPreciserClose")->Text   =FormatFloat("0.00", Offset_File.iPreciserClose);
            }
        }
        else if(iNowOffsetSel==OfsOutSh1        || iNowOffsetSel==OfsOutSh1LB || iNowOffsetSel==OfsOutSh1RA || iNowOffsetSel==OfsOutSh1RB   ||
                iNowOffsetSel==OfsOutSh2        || iNowOffsetSel==OfsOutSh2LB || iNowOffsetSel==OfsOutSh2RA || iNowOffsetSel==OfsOutSh2RB   ||
                iNowOffsetSel==OfsAuto1         || iNowOffsetSel==OfsAuto2    || iNowOffsetSel==OfsAuto3    || iNowOffsetSel==OfsAuto4      || iNowOffsetSel==OfsAuto5  || iNowOffsetSel==OfsAuto6 ||
                iNowOffsetSel==OfsFix1          || iNowOffsetSel==OfsFix2     || iNowOffsetSel==OfsFix3     || iNowOffsetSel==OfsFix4       || iNowOffsetSel==OfsFix5   || iNowOffsetSel==OfsFix6  ||
                iNowOffsetSel==OfsRotate_Out    || iNowOffsetSel==OfsTopView  || iNowOffsetSel==OfsPADView  || iNowOffsetSel==OfsBGAView    || iNowOffsetSel==OfsScanAOI)
        {
            EL<TEdit>("TfOffSet", "edArmX")->Text                =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetX());
            EL<TEdit>("TfOffSet", "edArmY")->Text                =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetY());
            EL<TEdit>("TfOffSet", "edPickUp")->Text              =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetPickUp());
            EL<TEdit>("TfOffSet", "edRelease")->Text             =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetPlace());
            EL<TEdit>("TfOffSet", "edPitchX1")->Text             =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable());

            if(USE_OUT_Y_IS_AUTO_PITCH==true ||     //JerryYang 20251218 : IN/OUT ARM支援不同模組
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)
            {
                EL<TEdit>("TfOffSet", "edPitchX2")->Text         =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable2());
            }

            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.25 For HT-1032 AT
            {
                EL<TEdit>("TfOffSet", "edPitchX3")->Text         =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable3());
                EL<TEdit>("TfOffSet", "edPitchX4")->Text         =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariable4());
            }

            if(USE_OUT_Y_IS_AUTO_PITCH==true)           //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<TEdit>("TfOffSet", "edPitchY")->Text          =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->GetVariableY());   //ChungHung 20131231 alter AutoYPitch
            }

            for(int i=0; i<OutArmSuck.iMotRow; i++)
            {
                for(int j=0; j<OutArmSuck.iMotCol; j++)
                {
                    //MyXOffsetEdit[i][j]->Text  =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetX[i][j]);
                    //MyYOffsetEdit[i][j]->Text  =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetY[i][j]);
                    //==>
                    MySingleOffsetTEditX[i][j]->Text  =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetX[i][j]);  //Eastsun 20251230 : 修復獨立offset功能
                    MySingleOffsetTEditY[i][j]->Text  =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPosOffSetY[i][j]);

                    MyPickEdit[i][j]->Text     =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPickUpOffSet[i][j]);
                    MyRelsEdit[i][j]->Text     =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[iNowOffsetSel]]->SingleOffSet->dPlaceOffSet[i][j]);
                }
            }
        }
    }
    else  //Tray Arm Offset
    {
        if(iNowOffsetSel<=tOfsAuto6)
        {
            EL<TEdit>("TfOffSet", "edtTrayArmX")->Text=FormatFloat("0.00", Offset_File.iTrayArmX[iNowOffsetSel]);
            EL<TPanel>("TfOffSet", "Panel2")->Visible=true;
            EL<TPanel>("TfOffSet", "Panel3")->Visible=false;
            EL<TEdit>("TfOffSet", "edtARTPlace")->Text=FormatFloat("0.00", Offset_File.iTrayArmX_ART[iNowOffsetSel]); //kevin 20170831 (Steven) ART offset
            if(USE_AUTO_RETEST==eartInstall &&
               (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&                //kevin 20170831 (Steven) add ART tray arm offset
               (LastSet.iRunStartMode==rsmInitial_ART       ||
                LastSet.iRunStartMode==rsmContinuStart_ART  ||
                LastSet.iRunStartMode==rsmContinuRetest_ART ||
                LastSet.iRunStartMode==rsmAutoRetest))
            {
                EL<TPanel>("TfOffSet", "pnlTrayXART")->Visible=true;
            }
            else
            {
                EL<TPanel>("TfOffSet", "pnlTrayXART")->Visible=false;                                     //Ztex 2024.03.19 Add HT-1032AT
            }
            EL<TEdit>("TfOffSet", "edlLoadZ")->Text=FormatFloat("0.00", Offset_File.dTrayZseparate[iNowOffsetSel]);   //Steven 20190813 : 入Tray改用步進馬達
            EL<TEdit>("TfOffSet", "edlLoadZ")->Visible=(LOAD_Z_USE_MOTOR[iNowOffsetSel] || TRAY_ARM_MODE==eUnderCoveyor);//Ztex 2024.03.19 Add HT-1032AT
            EL<TLabel>("TfOffSet", "lblLoadZ")->Visible=(LOAD_Z_USE_MOTOR[iNowOffsetSel] || TRAY_ARM_MODE==eUnderCoveyor);//Ztex 2024.03.19 Add HT-1032AT
        }
        else
        {
            EL<TPanel>("TfOffSet", "pnlTrayXART")->Visible=false;
            iNum=iNowOffsetSel-tOfsIndex1;
            if(iNum>1)
                ShowErrorMessage("WAR1681", 0, MMSystem, 0, "ReadOffset");

            EL<TEdit>("TfOffSet", "IndexArmOffSet1")->Text=FormatFloat("0.00", Offset_File.iIndexArmPickUp[iNum]);
            EL<TEdit>("TfOffSet", "IndexArmOffSet2")->Text=FormatFloat("0.00", Offset_File.iIndexArmPlace[iNum]);
            EL<TEdit>("TfOffSet", "IndexArmOffSet3")->Text=FormatFloat("0.00", Offset_File.iIndexArmContact[iNum]);
            EL<TEdit>("TfOffSet", "IndexArmOffSet4")->Text=FormatFloat("0.00", Offset_File.iSHHalft[iNum]);
            EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Text=FormatFloat("0.00", Offset_File.iSHRightPod[iNum]);
            EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Text=FormatFloat("0.00", Offset_File.iSHLeftPod[iNum]);
            EL<TEdit>("TfOffSet", "edShtFor2D")->Text     =FormatFloat("0.00", Offset_File.iSHLeft2D[iNum]);  //Steven 20151218 : Offset for 2d reader
            EL<TPanel>("TfOffSet", "Panel2")->Visible=false;
            EL<TPanel>("TfOffSet", "Panel3")->Visible=true;
        }
    }

}

// golden cOffSet.cpp:2476  TfOffSet::SpBotSelClick(TObject *Sender)
static void OS_SpBotSelClick(int iPtrTag)
{

    AnsiString S;
    if(bOffsetEnterBarcode==false)
    {
        if(Barcode_Reader(bcOffset)==0)                                         // 20140103 wei KYEC Barcode Reader
        {
    { OS_bSelRefused=true; filerw::ELMark("barcodeRefused"); return; }   // golden cOffSet.cpp:2483-2483 取代：KYEC barcode 登入被拒（golden return）：記下讓入口知道這個選取沒生效
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2483-2483 -- KYEC barcode 登入被拒（golden return）：記下讓入口知道這個選取沒生效
            return;
#endif // GATE (S12-C save)
        }
    }
    TSpeedButton *Ptr=OffSetSelBot[iPtrTag];   // golden cOffSet.cpp:2486-2486 取代：Sender＝被按的部位鈕：入口傳 Tag（＝golden 建構子 :326 OffSetSelBot[i]->Tag=i）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2486-2486 -- Sender＝被按的部位鈕：入口傳 Tag（＝golden 建構子 :326 OffSetSelBot[i]->Tag=i）
    TSpeedButton *Ptr=(TSpeedButton *)Sender;
#endif // GATE (S12-C save)
    bool bResult=OS_bSavePrev ? OS_SaveFile(iNowOffsetSel, iSaveStander, false) : false;   // golden cOffSet.cpp:2487-2487 取代：golden 換部位前先存「上一個部位」。C 路的顯示是開頁時逐一切換、不是使用者操作 → 不存；使用者改過的每一組都由存檔請求各自跑 spbSaveClick 存（最後檔案內容相同）。//AI(W906-D013) 20260929：R127 照 BCB —— 只有 WS form.event 的部位鈕（FileRW/Offset_File.cpp OS_EvSelect，上一組＝頁面這一次真的在看的組）才照 golden 先存上一組（OS_bSavePrev）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2487-2487 -- golden 換部位前先存「上一個部位」。C 路的顯示是開頁時逐一切換、不是使用者操作 → 不存；使用者改過的每一組都由存檔請求各自跑 spbSaveClick 存（最後檔案內容相同）。//AI(W906-D013) 20260929：R127 照 BCB —— 只有 WS form.event 的部位鈕（FileRW/Offset_File.cpp OS_EvSelect，上一組＝頁面這一次真的在看的組）才照 golden 先存上一組（OS_bSavePrev）
    bool bResult=OS_SaveFile(iNowOffsetSel, iSaveStander, false);
#endif // GATE (S12-C save)
    if(bResult)
    {
        Ptr->Down=false;
        if(iNowOffsetSel!=-1)
            OffSetSelBot[iNowOffsetSel]->Down=true;
        iNowOffsetSel=iNowOffsetSel;
    }
    else
    {
        iNowOffsetSel=Ptr->Tag;
    }

    if(Ptr->Tag==OfsRotate_In && USE_DIE_CLEAN!=0)
        EL<TPanel>("TfOffSet", "palOffsetParts")->Caption="Die Clean";
    else
        EL<TPanel>("TfOffSet", "palOffsetParts")->Caption=CapStr[iNowOffsetSel];

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                              //Steven 20141029 : XY-Pitch
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        if((iNowOffsetSel>=OfsOutSh1 && iNowOffsetSel<=OfsFix3) ||
            iNowOffsetSel==OfsRotate_Out    ||
            iNowOffsetSel==OfsTopView       ||
            iNowOffsetSel==OfsPADView       ||                                  //wei 20160617 Vitrox
            iNowOffsetSel==OfsBGAView       ||                                  //wei 20160617 Vitrox
            iNowOffsetSel==OfsScanAOI       )
        {
            OS_LoadImage(false);                                                   //Steven 20141029 : XY-Pitch
        }
        else
        {
            OS_LoadImage(true);
        }
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        if(iNowOffsetSel==OfsAuto1  ||
           iNowOffsetSel==OfsAuto2  ||
           iNowOffsetSel==OfsAuto3  ||
           iNowOffsetSel==OfsAuto4  ||
           iNowOffsetSel==OfsAuto5  ||
           iNowOffsetSel==OfsAuto6  ||
           iNowOffsetSel==OfsFix1   ||
           iNowOffsetSel==OfsFix2   ||
           iNowOffsetSel==OfsFix3   ||
           iNowOffsetSel==OfsFix4   ||
           iNowOffsetSel==OfsFix5   ||
           iNowOffsetSel==OfsFix6   ||
           iNowOffsetSel==OfsOutSh1 ||
           iNowOffsetSel==OfsOutSh2 ||
           iNowOffsetSel==OfsOutSh1RA ||
           iNowOffsetSel==OfsOutSh2RA)
            OS_LoadImage(false);
        else
            OS_LoadImage(true);
    }

    int iZLimitHigh;                                                            //Steven 20210317 : 通富微要求unloader獨立設置
    int iZLimitLow;
    if(iNowOffsetSel==OfsAuto1 ||
       iNowOffsetSel==OfsAuto2 ||
       iNowOffsetSel==OfsAuto3 ||
       iNowOffsetSel==OfsAuto4 ||                                               //Steven 20230907 : For HT-9011UC
       iNowOffsetSel==OfsAuto5 ||
       iNowOffsetSel==OfsAuto6 ||
       iNowOffsetSel==OfsFix1  ||
       iNowOffsetSel==OfsFix2  ||
       iNowOffsetSel==OfsFix3  ||
       iNowOffsetSel==OfsFix4  ||                                               //Steven 20230907 : For HT-9011UC
       iNowOffsetSel==OfsFix5  ||
       iNowOffsetSel==OfsFix6)
    {
        iZLimitHigh=InputLimit.iOffsetUnloaderZHigh;
        iZLimitLow =InputLimit.iOffsetUnloaderZLow;
    }
    else
    {
        iZLimitHigh=InputLimit.iOffsetZHigh;
        iZLimitLow =InputLimit.iOffsetZLow;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20211111 add 1 ~ -1
    {
        if(iNowOffsetSel==OfsInSh1 || iNowOffsetSel==OfsInSh2)
        {
            S.sprintf("%d~%dmm", 1, -1);
        }
        else
        {
            S.sprintf("%d~%dmm", iZLimitHigh, iZLimitLow);
        }
    }
    else
    {
        S.sprintf("%d~%dmm", iZLimitHigh, iZLimitLow);
    }
    EL<TLabel>("TfOffSet", "lblReleaseRange")->Caption=S;
    EL<TLabel>("TfOffSet", "lblPickUpRange")->Caption=S;

    OS_ShowOneByOneOffSet();
    ReadFile();
    OS_DoIniDataToForm(iNowOffsetSel, iSaveStander);
    Ptr->Down=false;

}

// golden cOffSet.cpp:2594  TfOffSet::IndexOffSetBT2Click(TObject *Sender)
static void OS_IndexOffSetBT2Click(int iPtrTag)
{

    TSpeedButton *Ptr=OfffSetSpecSelBot[iPtrTag];   // golden cOffSet.cpp:2596-2596 取代：Sender：入口傳 Tag（golden 建構子 :367 OfffSetSpecSelBot[i]->Tag=i）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2596-2596 -- Sender：入口傳 Tag（golden 建構子 :367 OfffSetSpecSelBot[i]->Tag=i）
    TSpeedButton *Ptr=(TSpeedButton *)Sender;
#endif // GATE (S12-C save)
    EL<TPanel>("TfOffSet", "pnlIndexOffset")->Caption=SpecialOffSetName[Ptr->Tag];
    ;   // golden cOffSet.cpp:2598-2598 取代：同 SpBotSelClick：換選取前先存上一個 → C 路顯示不存（存檔請求各組自己存）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2598-2598 -- 同 SpBotSelClick：換選取前先存上一個 → C 路顯示不存（存檔請求各組自己存）
    OS_SaveFile(iSpecialOffSetSel, iSaveSpecial, false);
#endif // GATE (S12-C save)
    iSpecialOffSetSel=Ptr->Tag;
    ReadFile();
    OS_DoIniDataToForm(iSpecialOffSetSel, iSaveSpecial);
    Ptr->Down=false;

}

// golden cOffSet.cpp:2807  TfOffSet::spbSaveClick(TObject *Sender)
static void OS_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString S="";
    EL<TSpeedButton>("TfOffSet", "spbSave")->Down=false;
    filerw::ELTodo("golden cOffSet.cpp:2811 fMain->iOperatorModeCount=0 not done (member not in the port TfMain)");   // golden cOffSet.cpp:2811-2811 取代：fMain->iOperatorModeCount（時間到登出的計數）：移植樹 TfMain 沒有這個成員
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2811-2811 -- fMain->iOperatorModeCount（時間到登出的計數）：移植樹 TfMain 沒有這個成員
    fMain->iOperatorModeCount=0;                                                //Ifor 20151027 時間到登出
#endif // GATE (S12-C save)
    if(OS_iActivePage==0)   // golden cOffSet.cpp:2812-2812 取代：ActivePageIndex==0（使用者在 In Out Arm 分頁）：入口依這一組是 stander 還是 special 設 OS_iActivePage
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2812-2812 -- ActivePageIndex==0（使用者在 In Out Arm 分頁）：入口依這一組是 stander 還是 special 設 OS_iActivePage
    if(EL<TPageControl>("TfOffSet", "PageControl1")->ActivePageIndex==0)
#endif // GATE (S12-C save)
    {
    if(!OS_bTailOnly) OS_SaveFile(iNowOffsetSel, iSaveStander, false);   // golden cOffSet.cpp:2814-2814 取代：多組存檔：尾端（ReadFile 起）只在全部組存完後跑一次 → 尾端那次跳過 SaveFile
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2814-2814 -- 多組存檔：尾端（ReadFile 起）只在全部組存完後跑一次 → 尾端那次跳過 SaveFile
        OS_SaveFile(iNowOffsetSel, iSaveStander, false);
#endif // GATE (S12-C save)
        bEnterSpecialOffset=false;
    }
    else
    {
    if(!OS_bTailOnly) OS_SaveFile(iSpecialOffSetSel, iSaveSpecial, false);   // golden cOffSet.cpp:2819-2819 取代：同上
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2819-2819 -- 同上
        OS_SaveFile(iSpecialOffSetSel, iSaveSpecial, false);
#endif // GATE (S12-C save)
        bEnterSpecialOffset=true;
    }

    if(OS_bPartOnly) return;   ReadFile();   // golden cOffSet.cpp:2823-2823 取代：多組存檔：前面各組只跑到 SaveFile（OS_bPartOnly），ReadFile 以後的全域副作用（重讀、ASE_CL、Pause、SetWorkParameter）最後跑一次
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2823-2823 -- 多組存檔：前面各組只跑到 SaveFile（OS_bPartOnly），ReadFile 以後的全域副作用（重讀、ASE_CL、Pause、SetWorkParameter）最後跑一次
    ReadFile();
#endif // GATE (S12-C save)
    bEnterOffset=true;

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        if(LastSet.iTemperature==Tempture_Ambient)
        {
    filerw::ELTodo("golden cOffSet.cpp:2830 SetTempShiftOffsetPeriod(25, recipe) not in the port");   // golden cOffSet.cpp:2830-2830 取代：SetTempShiftOffsetPeriod（pig 20240215）：移植樹沒有
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2830-2830 -- SetTempShiftOffsetPeriod（pig 20240215）：移植樹沒有
            SetTempShiftOffsetPeriod(25,fMain->cbSetupFileName->Text);
#endif // GATE (S12-C save)
        }
        else
        {
    filerw::ELTodo("golden cOffSet.cpp:2834 SetTempShiftOffsetPeriod(fWorkTemperBase, recipe) not in the port");   // golden cOffSet.cpp:2834-2834 取代：同上
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2834-2834 -- 同上
            SetTempShiftOffsetPeriod(Temperature.fWorkTemperBase, fMain->cbSetupFileName->Text);
#endif // GATE (S12-C save)
        }

    S.sprintf("D:\\HT9045\\data\\%s.ini", fMain->cbSetupFileName->Text.c_str());   // golden cOffSet.cpp:2837-2837 取代：AnsiString 丟進 sprintf 的 ...（g++ 不收）→ .c_str()，路徑照 golden
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2837-2837 -- AnsiString 丟進 sprintf 的 ...（g++ 不收）→ .c_str()，路徑照 golden
        S.sprintf("D:\\HT9045\\data\\%s.ini", fMain->cbSetupFileName->Text);
#endif // GATE (S12-C save)
//
    WriteIniData(S,"LoaderScale","ScanleEnable",EL<TCheckBox>("TfOffSet", "chkInArmScakeEnable")->Checked);   // golden cOffSet.cpp:2839-2839 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2839-2839 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
    WriteIniData(S,"LoaderScale","ScanleEnable",fOffSet->chkInArmScakeEnable->Checked);
#endif // GATE (S12-C save)
//    WriteIniData(S,"LoaderScale","LoaderX",fOffSet->edLodX->Text);
//    WriteIniData(S,"LoaderScale","LoaderY",fOffSet->edLodY->Text);
//    WriteIniData(S,"LoaderScale","HotPlate1X",fOffSet->edt_HP1X->Text);
//    WriteIniData(S,"LoaderScale","HotPlate1Y",fOffSet->edt_HP1Y->Text);
//    WriteIniData(S,"LoaderScale","HotPlate2X",fOffSet->edt_HP2X->Text);
//    WriteIniData(S,"LoaderScale","HotPlate2Y",fOffSet->edt_HP2Y->Text);
    WriteIniData(S,"LoaderScale","Shuttle1X",EL<TEdit>("TfOffSet", "ed_IS1X")->Text);   // golden cOffSet.cpp:2846-2846 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2846-2846 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"LoaderScale","Shuttle1X",fOffSet->ed_IS1X->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"LoaderScale","Shuttle1Y",EL<TEdit>("TfOffSet", "ed_IS1Y")->Text);   // golden cOffSet.cpp:2847-2847 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2847-2847 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"LoaderScale","Shuttle1Y",fOffSet->ed_IS1Y->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"LoaderScale","Shuttle2X",EL<TEdit>("TfOffSet", "ed_IS2X")->Text);   // golden cOffSet.cpp:2848-2848 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2848-2848 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"LoaderScale","Shuttle2X",fOffSet->ed_IS2X->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"LoaderScale","Shuttle2Y",EL<TEdit>("TfOffSet", "ed_IS2Y")->Text);   // golden cOffSet.cpp:2849-2849 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2849-2849 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"LoaderScale","Shuttle2Y",fOffSet->ed_IS2Y->Text);
#endif // GATE (S12-C save)
//
    WriteIniData(S,"UnLoaderScale","ScanleEnable",EL<TCheckBox>("TfOffSet", "chkOutArmScakeEnable")->Checked);   // golden cOffSet.cpp:2851-2851 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2851-2851 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"UnLoaderScale","ScanleEnable",fOffSet->chkOutArmScakeEnable->Checked);
#endif // GATE (S12-C save)
    WriteIniData(S,"UnLoaderScale","Shuttle1X",EL<TEdit>("TfOffSet", "ed_OS1X")->Text);   // golden cOffSet.cpp:2852-2852 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2852-2852 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"UnLoaderScale","Shuttle1X",fOffSet->ed_OS1X->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"UnLoaderScale","Shuttle1Y",EL<TEdit>("TfOffSet", "ed_OS1Y")->Text);   // golden cOffSet.cpp:2853-2853 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2853-2853 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"UnLoaderScale","Shuttle1Y",fOffSet->ed_OS1Y->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"UnLoaderScale","Shuttle2X",EL<TEdit>("TfOffSet", "ed_OS2X")->Text);   // golden cOffSet.cpp:2854-2854 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2854-2854 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"UnLoaderScale","Shuttle2X",fOffSet->ed_OS2X->Text);
#endif // GATE (S12-C save)
    WriteIniData(S,"UnLoaderScale","Shuttle2Y",EL<TEdit>("TfOffSet", "ed_OS2Y")->Text);   // golden cOffSet.cpp:2855-2855 取代：golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2855-2855 -- golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身
        WriteIniData(S,"UnLoaderScale","Shuttle2Y",fOffSet->ed_OS2Y->Text);
#endif // GATE (S12-C save)

        EL<TCheckBox>("TfOffSet", "chkInArmScakeEnable")->Checked=TestIF_File.bInArmUseDifferentScaleBySetupFile;

        EL<TEdit>("TfOffSet", "edLodX")->Text=TestIF_File.fLoaderTrayXScaleBySetupFile;
        EL<TEdit>("TfOffSet", "edLodY")->Text=TestIF_File.fLoaderTrayYScaleBySetupFile;
        EL<TEdit>("TfOffSet", "edt_HP1X")->Text=TestIF_File.fHotPlateXScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "edt_HP1Y")->Text=TestIF_File.fHotPlateYScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "edt_HP2X")->Text=TestIF_File.fHotPlateXScaleBySetupFile[1];
        EL<TEdit>("TfOffSet", "edt_HP2Y")->Text=TestIF_File.fHotPlateYScaleBySetupFile[1];

        EL<TEdit>("TfOffSet", "ed_IS1X")->Text=TestIF_File.fInShuttleXScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "ed_IS1Y")->Text=TestIF_File.fInShuttleYScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "ed_IS2X")->Text=TestIF_File.fInShuttleXScaleBySetupFile[1];
        EL<TEdit>("TfOffSet", "ed_IS2Y")->Text=TestIF_File.fInShuttleYScaleBySetupFile[1];

        EL<TCheckBox>("TfOffSet", "chkOutArmScakeEnable")->Checked=TestIF_File.bOutArmUseDifferentScaleBySetupFile;

        EL<TEdit>("TfOffSet", "ed_OS1X")->Text=TestIF_File.fOutShuttleXScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "ed_OS1Y")->Text=TestIF_File.fOutShuttleYScaleBySetupFile[0];
        EL<TEdit>("TfOffSet", "ed_OS2X")->Text=TestIF_File.fOutShuttleXScaleBySetupFile[1];
        EL<TEdit>("TfOffSet", "ed_OS2Y")->Text=TestIF_File.fOutShuttleYScaleBySetupFile[1];
    }
    bUseUpdate=true;

    fMain->Pause("Save Offset"); filerw::ELTodo("golden cOffSet.cpp:2880 fMain->Pause(\"Save Offset\") is an offline counter in the port -- machine not paused");   // golden cOffSet.cpp:2880-2880 取代：移植樹 TfMain::Pause 是離線計數器（forms/fMain.h:167，不停機）→ 照叫並回報
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2880-2880 -- 移植樹 TfMain::Pause 是離線計數器（forms/fMain.h:167，不停機）→ 照叫並回報
    fMain->Pause("Save Offset");                                                //jou 2011-02-18
#endif // GATE (S12-C save)
    EL<TLabel>("TfOffSet", "labWarningForStop")->Visible=true;                                            //jou 2011-02-18
    SetWorkParameter();                                                         //Steven 20221122 : 存檔後要重新load參數

}

// golden cOffSet.cpp:3284  TfOffSet::Timer1Timer(TObject *Sender)
static void OS_Timer1Timer()
{

    static bool bRun=false;
    if(bRun)
        return;

    bRun=true;

    if(SystemStart)                                                             //Steven 20220120 : 機台在動的時候,不可以調整Shuttle Offset,避免Hang up
    {
        EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=false;
        EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=false;
    }
    else
    {
        if(iSpecialOffSetSel==tOfsIndex1)
        {
            if(InSHT1InRT())
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=false;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=true;
            }
            else if(InSHT1InLF())
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=true;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=false;
            }
            else
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=true;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=true;
            }
        }
        else if(iSpecialOffSetSel==tOfsIndex2)
        {
            if(InSHT2InRT())
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=false;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=true;
            }
            else if(InSHT2InLF())
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=true;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=false;
            }
            else
            {
                EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=true;
                EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=true;
            }
        }
        else
        {
            EL<TEdit>("TfOffSet", "IndexArmOffSet5")->Enabled=false;
            EL<TEdit>("TfOffSet", "IndexArmOffSet6")->Enabled=false;
        }
    }

    bRun=false;

}

// golden cOffSet.cpp:3351  TfOffSet::ShowFinalAirForce()
static void OS_ShowFinalAirForce()
{

    EL<TLabel>("TfOffSet", "lbFinalAirForce")->Caption=FloatToStr(DeviceForm.fAireForce);

}

// golden cOffSet.cpp:3367  TfOffSet::btnBackClick(TObject *Sender)
static void OS_btnBackClick()
{

    EL<TSpeedButton>("TfOffSet", "btnBack")->Down=false;
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfOffSet", "PageControl1")->ActivePage=EL<TTabSheet>("TfOffSet", "tsInOutArmOffset");

}

// golden cOffSet.cpp:3392  TfOffSet::ShowOffSetList()
static void OS_ShowOffSetList()
{

    ;   // golden cOffSet.cpp:3394-3527 取代：sgOffsetList（TStringGrid）的 ColWidths／RowHeights／Cells：總表分頁的畫面，替身沒有 ColWidths；資料就是各組 offsets（頁面自己組表）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:3394-3527 -- sgOffsetList（TStringGrid）的 ColWidths／RowHeights／Cells：總表分頁的畫面，替身沒有 ColWidths；資料就是各組 offsets（頁面自己組表）
    int iCol;

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                    //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch2]=80;
    }
    else
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch2]=-1;
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                              //QQ
       USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                                 //Ztex 2023.12.06 Add HT-1032
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch3]=80;
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch4]=80;
    }
    else
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch3]=-1;
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitch4]=-1;
    }

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                    //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitchY]=80;
    }
    else
    {
        EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->ColWidths[oiArmPitchY]=-1;
    }

    for(int i=0; i<OfsTotal; i++)
    {
        iCol=i+1;
        if(OffSetSelBot[i]->Visible==true)
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->RowHeights[i+1]=30;
        else
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->RowHeights[i+1]=-1;

        if(i==OfsRotate_In          || i==OfsHP1                || i==OfsHP2                ||
           i==OfsAutoClean          || i==OfsOCR                || i==OfsInPlacement        ||
           i==OfsLoader             || i==OfsLoaderB            ||
           i==OfsAutoSh1            || i==OfsAutoSh2            || i==OfsPreciser           || i==OfsBottom2DID         ||
           i==OfsInSh1              || i==OfsInSh1LB            || i==OfsInSh1RA            || i==OfsInSh1RB            ||
           i==OfsInSh2              || i==OfsInSh2LB            || i==OfsInSh2RA            || i==OfsInSh2RB            ||
           i==OfsInSh1_AutoClean    || i==OfsInSh1LB_AutoClean  || i==OfsInSh1RA_AutoClean  || i==OfsInSh1RB_AutoClean  ||
           i==OfsInSh2_AutoClean    || i==OfsInSh2LB_AutoClean  || i==OfsInSh2RA_AutoClean  || i==OfsInSh2RB_AutoClean)
        {
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmX      ][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetX());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmY      ][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetY());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPick   ][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetPickUp());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPlace  ][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetPlace());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch1 ][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetVariable());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][0]);     //Eastsun 20251219 : in/out Arm Fix模式 獨立吸嘴偏移X功能  [A C E G] [B D F H]
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_X][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_Y][iCol]    =FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][3]);

            if(USE_IN_Y_IS_AUTO_PITCH==true)                                    //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch2][iCol]=FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetVariable2());
            }

            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //QQ
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.06 Add HT-1032
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch3][iCol]=FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetVariable3());
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch4][iCol]=FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetVariable4());
            }

            if(USE_IN_Y_IS_AUTO_PITCH==true)                                    //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitchY][iCol]=FormatFloat("0.00", InArmOffSet_File[iOffsetMap[i]]->GetVariableY());
            }
        }
        else if(i==OfsOutSh1        || i==OfsOutSh1LB || i==OfsOutSh1RA || i==OfsOutSh1RB   ||
                i==OfsOutSh2        || i==OfsOutSh2LB || i==OfsOutSh2RA || i==OfsOutSh2RB   ||
                i==OfsAuto1         || i==OfsAuto2    || i==OfsAuto3    || i==OfsAuto4      || i==OfsAuto5  || i==OfsAuto6 ||
                i==OfsFix1          || i==OfsFix2     || i==OfsFix3     || i==OfsFix4       || i==OfsFix5   || i==OfsFix6  ||
                i==OfsRotate_Out    || i==OfsTopView  || i==OfsPADView  || i==OfsBGAView    || i==OfsScanAOI)
        {
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmX      ][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetX());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmY      ][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetY());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPick   ][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetPickUp());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPlace  ][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetPlace());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch1 ][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetVariable());
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][0]);     //Eastsun 20251219 : in/out Arm Fix模式 獨立吸嘴偏移X功能  [A C E G] [B D F H]
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickA_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickB_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][0]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickC_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickD_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][1]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickE_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickF_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][2]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[0][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickG_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[0][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_X][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetX[1][3]);
            EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPickH_Y][iCol]    =FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->SingleOffSet->dPosOffSetY[1][3]);

            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch2][iCol]=FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetVariable2());
            }

            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //QQ
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.06 Add HT-1032
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch3][iCol]=FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetVariable3());
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitch4][iCol]=FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetVariable4());
            }

            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                EL<filerw::ELStringGrid>("TfOffSet", "sgOffsetList")->Cells[oiArmPitchY][iCol]=FormatFloat("0.00", OutArmOffSet_File[iOffsetMap[i]]->GetVariableY());
            }
        }
    }
#endif // GATE (S12-C save)

}

// golden cOffSet.cpp:2892  TfOffSet::sb_AutoOffsetUpClick(TObject *Sender)
static void OS_sb_AutoOffsetUpClick()
{

    if(SystemStart)
        return;

    EL<TEdit>("TfOffSet", "edArmY")->Text=FormatFloat("0.00", atof(EL<TEdit>("TfOffSet", "edArmY")->Text.c_str())+0.1);
    OS_bSkipReadFile=false; OS_bPartOnly=false; OS_bTailOnly=false; OS_iActivePage=0; OS_spbSaveClick();   // golden cOffSet.cpp:2898-2898 取代：spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2898-2898 -- spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
    OS_spbSaveClick(this);
#endif // GATE (S12-C save)
    if(EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked==true)
    {
    filerw::ELTodo("golden cOffSet.cpp:2901 fMain->Start(\"sb_AutoOffsetUpClick\") (OS-1b: auto offset position check runs the machine once) not ported -- B8 / Jimmy (WebStart.cpp); the offset was saved, the machine did not move");   // golden cOffSet.cpp:2901-2901 取代：OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2901-2901 -- OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
        fMain->Start("sb_AutoOffsetUpClick");
#endif // GATE (S12-C save)
    }

}

// golden cOffSet.cpp:2905  TfOffSet::sb_AutoOffsetDownClick(TObject *Sender)
static void OS_sb_AutoOffsetDownClick()
{

    if(SystemStart)
        return;

    EL<TEdit>("TfOffSet", "edArmY")->Text=FormatFloat("0.00", atof(EL<TEdit>("TfOffSet", "edArmY")->Text.c_str())-0.1);
    OS_bSkipReadFile=false; OS_bPartOnly=false; OS_bTailOnly=false; OS_iActivePage=0; OS_spbSaveClick();   // golden cOffSet.cpp:2911-2911 取代：spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2911-2911 -- spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
    OS_spbSaveClick(this);
#endif // GATE (S12-C save)
    if(EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked==true)
    {
    filerw::ELTodo("golden cOffSet.cpp:2914 fMain->Start(\"sb_AutoOffsetDownClick\") (OS-1b: auto offset position check runs the machine once) not ported -- B8 / Jimmy (WebStart.cpp); the offset was saved, the machine did not move");   // golden cOffSet.cpp:2914-2914 取代：OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2914-2914 -- OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
        fMain->Start("sb_AutoOffsetDownClick");
#endif // GATE (S12-C save)
    }

}

// golden cOffSet.cpp:2918  TfOffSet::sb_AutoOffsetRightClick(TObject *Sender)
static void OS_sb_AutoOffsetRightClick()
{

    if(SystemStart)
        return;

    EL<TEdit>("TfOffSet", "edArmX")->Text=FormatFloat("0.00", atof(EL<TEdit>("TfOffSet", "edArmX")->Text.c_str())+0.1);
    OS_bSkipReadFile=false; OS_bPartOnly=false; OS_bTailOnly=false; OS_iActivePage=0; OS_spbSaveClick();   // golden cOffSet.cpp:2924-2924 取代：spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2924-2924 -- spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
    OS_spbSaveClick(this);
#endif // GATE (S12-C save)
    if(EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked==true)
    {
    filerw::ELTodo("golden cOffSet.cpp:2927 fMain->Start(\"sb_AutoOffsetRightClick\") (OS-1b: auto offset position check runs the machine once) not ported -- B8 / Jimmy (WebStart.cpp); the offset was saved, the machine did not move");   // golden cOffSet.cpp:2927-2927 取代：OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2927-2927 -- OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
        fMain->Start("sb_AutoOffsetRightClick");
#endif // GATE (S12-C save)
    }

}

// golden cOffSet.cpp:2931  TfOffSet::sb_AutoOffsetLeftClick(TObject *Sender)
static void OS_sb_AutoOffsetLeftClick()
{

    if(SystemStart)
        return;

    EL<TEdit>("TfOffSet", "edArmX")->Text=FormatFloat("0.00", atof(EL<TEdit>("TfOffSet", "edArmX")->Text.c_str())-0.1);
    OS_bSkipReadFile=false; OS_bPartOnly=false; OS_bTailOnly=false; OS_iActivePage=0; OS_spbSaveClick();   // golden cOffSet.cpp:2937-2937 取代：spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2937-2937 -- spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）
    OS_spbSaveClick(this);
#endif // GATE (S12-C save)
    if(EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked==true)
    {
    filerw::ELTodo("golden cOffSet.cpp:2940 fMain->Start(\"sb_AutoOffsetLeftClick\") (OS-1b: auto offset position check runs the machine once) not ported -- B8 / Jimmy (WebStart.cpp); the offset was saved, the machine did not move");   // golden cOffSet.cpp:2940-2940 取代：OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
#if 0 // GATE (S12-C save) golden cOffSet.cpp:2940-2940 -- OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；這裡不跑，記 todo 讓頁面看得到
        fMain->Start("sb_AutoOffsetLeftClick");
#endif // GATE (S12-C save)
    }

}

// golden cOffSet.cpp:835  TfOffSet::FormClose(TObject *Sender, TCloseAction &Action)
static void OS_FormClose()
{

    fShow=false;
    EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked=false;
    bOffsetEnterBarcode=false;
    ReadFile();

    EL<TControl>("TfOffSet", "Timer1")->Enabled=false;                                                      //Steven 20220120 : 機台在動的時候,不可以調整Shuttle Offset,避免Hang up
    EL<TControl>("TfOffSet", "TimerSetupTeach")->Enabled=false;                                             //JerryYang 20180921 Setup Teach功能

    if(bDoInZTeach==true || bDoOutZTeach==true)                                 //JerryYang 20250120 : modify
    {
        bDoInZTeach=false;
        bDoOutZTeach=false;

        fAllMotorHome=false;
        iHome=1;

        NewRecordProcess("", "ZCalibration not finish, do homing!", "OffSet->FormClose");
    }

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/editlist/Offset_File.py 的 events；
//   FileRW/Offset_File.cpp 用 filerw::PageEventsRegistrar 註冊；本體 FileRW/_EditPage.cpp RunPageEvent）
static void OS_Ev_sb_AutoOffsetUpClick(TControl* Sender) { (void)Sender; OS_sb_AutoOffsetUpClick(); }
static void OS_Ev_sb_AutoOffsetDownClick(TControl* Sender) { (void)Sender; OS_sb_AutoOffsetDownClick(); }
static void OS_Ev_sb_AutoOffsetRightClick(TControl* Sender) { (void)Sender; OS_sb_AutoOffsetRightClick(); }
static void OS_Ev_sb_AutoOffsetLeftClick(TControl* Sender) { (void)Sender; OS_sb_AutoOffsetLeftClick(); }
static const filerw::PageEvent kOS_Events[] = {
    {"sb_AutoOffsetUp", "click", "cOffSet.cpp:2892 TfOffSet::sb_AutoOffsetUpClick", &OS_Ev_sb_AutoOffsetUpClick},
    {"sb_AutoOffsetDown", "click", "cOffSet.cpp:2905 TfOffSet::sb_AutoOffsetDownClick", &OS_Ev_sb_AutoOffsetDownClick},
    {"sb_AutoOffsetRight", "click", "cOffSet.cpp:2918 TfOffSet::sb_AutoOffsetRightClick", &OS_Ev_sb_AutoOffsetRightClick},
    {"sb_AutoOffsetLeft", "click", "cOffSet.cpp:2931 TfOffSet::sb_AutoOffsetLeftClick", &OS_Ev_sb_AutoOffsetLeftClick},
};

