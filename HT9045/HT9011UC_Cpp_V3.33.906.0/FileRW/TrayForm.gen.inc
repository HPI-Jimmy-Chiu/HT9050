// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TrayForm 的讀寫檔（HTEditList： → Tray.Data）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden cTrayAssignment.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 173 個（header 宣告 247 個）。
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
#include "forms/fShowBinSelect.h"
#include "csystem.h"
#include "Motor/mymotor.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）
static int  TA_RunModePageIndex();                   // FileRW/TrayForm.cpp：pgRunMode->ActivePageIndex（不算存檔讀的替身）
static void TA_SetRadioIndex(TRadioGroup* g, int v);  // FileRW/TrayForm.cpp：VCL TCustomRadioGroup.SetItemIndex 夾值
extern void MyDBIProcess(AnsiString S1, AnsiString S2);   // aHotPlateSubstrate.h:979（本體 aHotPlateSubstrate.cpp:1244；aHotPlateSubstrate.h 與 HTEditList.h 衝突）

// golden cTrayAssignment.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static int iTrayDirect[eTrayCount];            // golden cTrayAssignment.h:285（private）方向圖索引 0=Loader 1..6=Auto 7..12=Fix
static bool fShow=false;                       // golden cTrayAssignment.h:302
static TCheckBox *chkICSort[MAX_AUTO_TRAY];    // golden cTrayAssignment.h:303（建構子 :47-48 填）
static TEdit *edtICSort[MAX_AUTO_TRAY][6];     // golden cTrayAssignment.h:304（建構子 :49-61 填）
static AnsiString Caption;                     // golden TForm::Caption（FormShow :750，HTML 不用）
static int Left=0;                             // golden TForm::Left（FormShow :756）
static int Top=0;                              // golden TForm::Top（FormShow :757）
static int Width=0;                            // golden TForm::Width（ShowCompnet :1089-1147）
static bool TA_rgLoaderTrayModeItems=true;     // FileRW：rgLoaderTrayMode->Controls[0..2]->Enabled（golden ReadFile :341-376，三顆一起設）

using filerw::EL;

// 前置宣告
static void TA_TfTrayAssignment();
static void TA_FixCanUse();
static void TA_ReadFile();
static void TA_DoIniDataToForm();
static void TA_FormShow();
static void TA_ShowCompnet();
static void TA_ShowTrayDirectIMG();
static void TA_GraphicToRadio();
static void TA_RadioToGraphic();
static void TA_GraphicToRadio_RT();
static void TA_RadioToGraphic_RT();
static void TA_sbNormalTestChange();
static void TA_sbNormalTest_RTChange();
static void TA_FormClose();
static void TA_spbSaveClick();
static void TA_SaveSetupFile(AnsiString szDir, AnsiString S);

// golden cTrayAssignment.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void TA_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTray1st_KYEC")->Items;
      s->Add("Empty");
      s->Add("Color");
      s->Add("No Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTray2nd_KYEC")->Items;
      s->Add("Empty");
      s->Add("Color");
      s->Add("No Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTray3rd_KYEC")->Items;
      s->Add("Empty");
      s->Add("Color");
      s->Add("No Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID1st_KYEC")->Items;
      s->Add("No Use");
      s->Add("Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID2nd_KYEC")->Items;
      s->Add("No Use");
      s->Add("Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID3rd_KYEC")->Items;
      s->Add("No Use");
      s->Add("Use");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbColor")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix1")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix2")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix3")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix4")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix5")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbFix6")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbInputRotateKIT")->Items;
      s->Add("0   Deg");
      s->Add("90  Deg");
      s->Add("-90 Deg");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbLoader")->Items;
      s->Add("Type1");
      s->Add("Type2");
      s->Add("Type3");
    }
    { TStringList* s = EL<TComboBox>("TfTrayAssignment", "cbOutputRotateKIT")->Items;
      s->Add("0   Deg");
      s->Add("90  Deg");
      s->Add("-90 Deg");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAMRReaderPos_KYEC")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->Items;
      s->Add("[1]  Full Bin");
      s->Add("[2] Up Down");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->Items;
      s->Add("[1] Empty");
      s->Add("[2] Color");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Items;
      s->Add("None");
      s->Add("When Loader had skip device, need remove loader tray manually.");
      s->Add("When Loader had skip device, need open door to check tray.");
    }
    { TStringList* s = EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->Items;
      s->Add("Same");
      s->Add("Different");
    }
}

// golden cTrayAssignment.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kTA_ParentOf[][2] = {
    {"GroupBox1", "palTrayAssign"},
    {"ImgNormalTest", "tsNormalTestGraph"},
    {"ImgReTest", "tsReTestGraph"},
    {"Label47", "palAMR"},
    {"Label49", "palAMR"},
    {"Panel5", "Panel3"},
    {"RGAuto1", "pnlFTTrayFrom"},
    {"RGAuto2", "pnlFTTrayFrom"},
    {"RGAuto3", "pnlFTTrayFrom"},
    {"RGAuto4", "pnlFTTrayFrom"},
    {"RGAuto5", "pnlFTTrayFrom"},
    {"RGAuto6", "pnlFTTrayFrom"},
    {"RGLoader", "pnlLoad_FT"},
    {"cbAMRTray1st_KYEC", "GroupBox2_KYEC"},
    {"cbAMRTray2nd_KYEC", "GroupBox2_KYEC"},
    {"cbAMRTray3rd_KYEC", "GroupBox2_KYEC"},
    {"cbAMRTrayID1st_KYEC", "GroupBox2_KYEC"},
    {"cbAMRTrayID2nd_KYEC", "GroupBox2_KYEC"},
    {"cbAMRTrayID3rd_KYEC", "GroupBox2_KYEC"},
    {"cbColor", "gbColorTray"},
    {"cbEmpty", "gbEmptyTray"},
    {"cbEnableAMRLoader_KYEC", "GroupBox2_KYEC"},
    {"cbEnableAMR_KYEC", "GroupBox2_KYEC"},
    {"cbFix1", "grpFix1"},
    {"cbFix2", "grpFix2"},
    {"cbFix3", "grpFix3"},
    {"cbFix4", "grpFix4"},
    {"cbFix5", "grpFix5"},
    {"cbFix6", "grpFix6"},
    {"cbInputRotateKIT", "gbRotateKit"},
    {"cbLoader", "GroupBox1"},
    {"cbManuTakeAutoFailTray_FT", "gbUnloadMode"},
    {"cbManuTakeAutoFailTray_RT", "gbUnloadMode"},
    {"cbMoveAfterTrayGoOut", "gbTrayArmMode"},
    {"cbOutputRotateKIT", "gbRotateKit"},
    {"cbSpecTrayCnt", "palAMR"},
    {"cbVTestNoRTBin", "grpVTestNoTBin"},
    {"chkAutoTrayFeed", "gbUnloadMode"},
    {"chkCheckLoadDirection", "grpCheckLoaderDirection"},
    {"chkColorTray", "gbColorTrayMode"},
    {"chkICSort1", "grpICSort1"},
    {"chkICSort2", "grpICSort2"},
    {"chkICSort3", "grpICSort3"},
    {"chkICSort4", "grpICSort4"},
    {"chkICSort5", "grpICSort5"},
    {"chkICSort6", "grpICSort6"},
    {"chkTrayEndRecvAuto1", "gbAuto1"},
    {"chkTrayEndRecvAuto2", "gbAuto2"},
    {"chkTrayEndRecvAuto3", "gbAuto3"},
    {"chkTrayEndRecvAuto4", "gbAuto4"},
    {"chkTrayEndRecvAuto5", "gbAuto5"},
    {"chkTrayEndRecvAuto6", "gbAuto6"},
    {"ckUseFix1", "grpFix1"},
    {"ckUseFix2", "grpFix2"},
    {"ckUseFix3", "grpFix3"},
    {"ckUseFix4", "grpFix4"},
    {"ckUseFix5", "grpFix5"},
    {"ckUseFix6", "grpFix6"},
    {"edAMRFullTrayCount", "palAMR"},
    {"edAMRInputTrayCount", "palAMR"},
    {"edAuto1Type", "gbAuto1"},
    {"edAuto2Type", "gbAuto2"},
    {"edAuto3Type", "gbAuto3"},
    {"edAuto4Type", "gbAuto4"},
    {"edAuto5Type", "gbAuto5"},
    {"edAuto6Type", "gbAuto6"},
    {"edNoRTBinFix1", "grpVTestNoTBin"},
    {"edNoRTBinFix2", "grpVTestNoTBin"},
    {"edNoRTBinFix3", "grpVTestNoTBin"},
    {"edtICSort1_1", "grpICSort1"},
    {"edtICSort1_2", "grpICSort1"},
    {"edtICSort1_3", "grpICSort1"},
    {"edtICSort1_4", "grpICSort1"},
    {"edtICSort1_5", "grpICSort1"},
    {"edtICSort1_6", "grpICSort1"},
    {"edtICSort2_1", "grpICSort2"},
    {"edtICSort2_2", "grpICSort2"},
    {"edtICSort2_3", "grpICSort2"},
    {"edtICSort2_4", "grpICSort2"},
    {"edtICSort2_5", "grpICSort2"},
    {"edtICSort2_6", "grpICSort2"},
    {"edtICSort3_1", "grpICSort3"},
    {"edtICSort3_2", "grpICSort3"},
    {"edtICSort3_3", "grpICSort3"},
    {"edtICSort3_4", "grpICSort3"},
    {"edtICSort3_5", "grpICSort3"},
    {"edtICSort3_6", "grpICSort3"},
    {"edtICSort4_1", "grpICSort4"},
    {"edtICSort4_2", "grpICSort4"},
    {"edtICSort4_3", "grpICSort4"},
    {"edtICSort4_4", "grpICSort4"},
    {"edtICSort4_5", "grpICSort4"},
    {"edtICSort4_6", "grpICSort4"},
    {"edtICSort5_1", "grpICSort5"},
    {"edtICSort5_2", "grpICSort5"},
    {"edtICSort5_3", "grpICSort5"},
    {"edtICSort5_4", "grpICSort5"},
    {"edtICSort5_5", "grpICSort5"},
    {"edtICSort5_6", "grpICSort5"},
    {"edtICSort6_1", "grpICSort6"},
    {"edtICSort6_2", "grpICSort6"},
    {"edtICSort6_3", "grpICSort6"},
    {"edtICSort6_4", "grpICSort6"},
    {"edtICSort6_5", "grpICSort6"},
    {"edtICSort6_6", "grpICSort6"},
    {"gbAuto1", "palTrayAssign"},
    {"gbAuto2", "palTrayAssign"},
    {"gbAuto3", "palTrayAssign"},
    {"gbAuto4", "palTrayAssign"},
    {"gbAuto5", "palTrayAssign"},
    {"gbAuto6", "palTrayAssign"},
    {"gbColorTray", "palTrayAssign"},
    {"gbColorTrayMode", "Panel5"},
    {"gbEmptyTray", "palTrayAssign"},
    {"gbRotateKit", "pnlTop"},
    {"gbTrayArmMode", "Panel5"},
    {"gbUnloadMode", "Panel5"},
    {"grpCheckLoaderDirection", "Panel5"},
    {"grpFix1", "pnlTop"},
    {"grpFix2", "pnlTop"},
    {"grpFix3", "pnlTop"},
    {"grpFix4", "pnlTop"},
    {"grpFix5", "pnlTop"},
    {"grpFix6", "pnlTop"},
    {"grpICSort1", "grpICSortFunc"},
    {"grpICSort2", "grpICSortFunc"},
    {"grpICSort3", "grpICSortFunc"},
    {"grpICSort4", "grpICSortFunc"},
    {"grpICSort5", "grpICSortFunc"},
    {"grpICSort6", "grpICSortFunc"},
    {"grpICSortFunc", "Panel3"},
    {"grpVTestNoTBin", "palAMR"},
    {"imgAuto1", "gbAuto1"},
    {"imgAuto2", "gbAuto2"},
    {"imgAuto3", "gbAuto3"},
    {"imgAuto4", "gbAuto4"},
    {"imgAuto5", "gbAuto5"},
    {"imgAuto6", "gbAuto6"},
    {"imgFix1", "grpFix1"},
    {"imgFix2", "grpFix2"},
    {"imgFix3", "grpFix3"},
    {"imgFix4", "grpFix4"},
    {"imgFix5", "grpFix5"},
    {"imgFix6", "grpFix6"},
    {"imgLoader", "GroupBox1"},
    {"labFix1", "grpFix1"},
    {"labFix2", "grpFix2"},
    {"labFix3", "grpFix3"},
    {"labFix4", "grpFix4"},
    {"labFix5", "grpFix5"},
    {"labFix6", "grpFix6"},
    {"lblHint", "pnlLoad_FT"},
    {"lblHint2", "pnlLoad_RT"},
    {"palAMR", "palTrayAssign"},
    {"pgRunMode", "palTrayAssign"},
    {"pnlFTTrayFrom", "tsNormalTestGroup"},
    {"pnlLoad_FT", "tsNormalTestGroup"},
    {"pnlLoad_RT", "tsReTestGroup"},
    {"pnlRTTrayFrom", "tsReTestGroup"},
    {"pnlTop", "palTrayAssign"},
    {"rbTemp", "Panel2"},
    {"rgAMRReaderPos_KYEC", "GroupBox2_KYEC"},
    {"rgAuto1_RT", "pnlRTTrayFrom"},
    {"rgAuto2_RT", "pnlRTTrayFrom"},
    {"rgAuto3_RT", "pnlRTTrayFrom"},
    {"rgAuto4_RT", "pnlRTTrayFrom"},
    {"rgAuto5_RT", "pnlRTTrayFrom"},
    {"rgAuto6_RT", "pnlRTTrayFrom"},
    {"rgFixTrayMode", "pnlTop"},
    {"rgLoad_RT", "pnlLoad_RT"},
    {"rgLoaderTrayMode", "Panel5"},
    {"rgLoaderType", "pnlTop"},
    {"sbNormalTest", "tsNormalTestGraph"},
    {"sbNormalTest_RT", "tsReTestGraph"},
    {"spbSave", "Panel2"},
    {"tsNormalTestGraph", "pgRunMode"},
    {"tsNormalTestGroup", "pgRunMode"},
    {"tsReTestGraph", "pgRunMode"},
    {"tsReTestGroup", "pgRunMode"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void TA_CreateContainerProxies()
{
    EL<TGroupBox>("TfTrayAssignment", "GroupBox1");
    EL<TGroupBox>("TfTrayAssignment", "GroupBox2_KYEC");
    EL<TPanel>("TfTrayAssignment", "Panel2");
    EL<TPanel>("TfTrayAssignment", "Panel3");
    EL<TPanel>("TfTrayAssignment", "Panel5");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto1");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto2");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto3");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto4");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto5");
    EL<TGroupBox>("TfTrayAssignment", "gbAuto6");
    EL<TGroupBox>("TfTrayAssignment", "gbColorTray");
    EL<TGroupBox>("TfTrayAssignment", "gbColorTrayMode");
    EL<TGroupBox>("TfTrayAssignment", "gbEmptyTray");
    EL<TGroupBox>("TfTrayAssignment", "gbRotateKit");
    EL<TGroupBox>("TfTrayAssignment", "gbTrayArmMode");
    EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode");
    EL<TGroupBox>("TfTrayAssignment", "grpCheckLoaderDirection");
    EL<TGroupBox>("TfTrayAssignment", "grpFix1");
    EL<TGroupBox>("TfTrayAssignment", "grpFix2");
    EL<TGroupBox>("TfTrayAssignment", "grpFix3");
    EL<TGroupBox>("TfTrayAssignment", "grpFix4");
    EL<TGroupBox>("TfTrayAssignment", "grpFix5");
    EL<TGroupBox>("TfTrayAssignment", "grpFix6");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort1");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort2");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort3");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort4");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort5");
    EL<TGroupBox>("TfTrayAssignment", "grpICSort6");
    EL<TGroupBox>("TfTrayAssignment", "grpICSortFunc");
    EL<TGroupBox>("TfTrayAssignment", "grpVTestNoTBin");
    EL<TPanel>("TfTrayAssignment", "palAMR");
    EL<TPanel>("TfTrayAssignment", "palTrayAssign");
    EL<TPageControl>("TfTrayAssignment", "pgRunMode");
    EL<TPanel>("TfTrayAssignment", "pnlFTTrayFrom");
    EL<TPanel>("TfTrayAssignment", "pnlLoad_FT");
    EL<TPanel>("TfTrayAssignment", "pnlLoad_RT");
    EL<TPanel>("TfTrayAssignment", "pnlRTTrayFrom");
    EL<TPanel>("TfTrayAssignment", "pnlTop");
    EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph");
    EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGroup");
    EL<TTabSheet>("TfTrayAssignment", "tsReTestGraph");
    EL<TTabSheet>("TfTrayAssignment", "tsReTestGroup");
    filerw::ELSetParents("TfTrayAssignment", kTA_ParentOf, (int)(sizeof(kTA_ParentOf) / sizeof(kTA_ParentOf[0])));
}

// golden cTrayAssignment.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void TA_DfmState()
{
    EL<TGroupBox>("TfTrayAssignment", "GroupBox2_KYEC")->Visible = false;
    EL<TEdit>("TfTrayAssignment", "edAuto1Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto1Type");
    EL<TEdit>("TfTrayAssignment", "edAuto2Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto2Type");
    EL<TEdit>("TfTrayAssignment", "edAuto3Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto3Type");
    EL<TEdit>("TfTrayAssignment", "edAuto4Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto4Type");
    EL<TEdit>("TfTrayAssignment", "edAuto5Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto5Type");
    EL<TEdit>("TfTrayAssignment", "edAuto6Type")->Enabled = false;
    filerw::ELSetReadOnly("TfTrayAssignment", "edAuto6Type");
    EL<TGroupBox>("TfTrayAssignment", "gbRotateKit")->Visible = false;
    EL<TGroupBox>("TfTrayAssignment", "grpICSortFunc")->Visible = false;
    EL<TGroupBox>("TfTrayAssignment", "grpVTestNoTBin")->Visible = false;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix1")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix1")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix2")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix2")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix3")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix3")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix4")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix4")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix5")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix5")->ItemIndex = 0;
    EL<TComboBox>("TfTrayAssignment", "cbFix6")->Text = "Type1";
    EL<TComboBox>("TfTrayAssignment", "cbFix6")->ItemIndex = 0;
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_FT")->Checked = true;
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_RT")->Checked = true;
    EL<TEdit>("TfTrayAssignment", "edAMRFullTrayCount")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edAMRInputTrayCount")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix1")->Text = "FIX1@NORTBIN";
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix2")->Text = "FIX2@NORTBIN";
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix3")->Text = "FIX3@NORTBIN";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort1_6")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort2_6")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort3_6")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort4_6")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort5_6")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_1")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_2")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_3")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_4")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_5")->Text = "0";
    EL<TEdit>("TfTrayAssignment", "edtICSort6_6")->Text = "0";
    EL<TRadioGroup>("TfTrayAssignment", "rgAMRReaderPos_KYEC")->ItemIndex = 1;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->ItemIndex = 0;
    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex = 1;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kTA_EnableAll[] = {
    "GroupBox1", "GroupBox2_KYEC", "Label47", "Label49", "Panel2", "Panel3",
    "Panel5", "RGAuto1", "RGAuto2", "RGAuto3", "RGAuto4", "RGAuto5",
    "RGAuto6", "RGLoader", "cbEnableAMRLoader_KYEC", "cbEnableAMR_KYEC", "cbManuTakeAutoFailTray_FT", "cbManuTakeAutoFailTray_RT",
    "cbMoveAfterTrayGoOut", "cbSpecTrayCnt", "cbVTestNoRTBin", "chkAutoTrayFeed", "chkCheckLoadDirection", "chkColorTray",
    "chkICSort1", "chkICSort2", "chkICSort3", "chkICSort4", "chkICSort5", "chkICSort6",
    "chkTrayEndRecvAuto1", "chkTrayEndRecvAuto2", "chkTrayEndRecvAuto3", "chkTrayEndRecvAuto4", "chkTrayEndRecvAuto5", "chkTrayEndRecvAuto6",
    "ckUseFix1", "ckUseFix2", "ckUseFix3", "ckUseFix4", "ckUseFix5", "ckUseFix6",
    "gbAuto1", "gbAuto2", "gbAuto3", "gbAuto4", "gbAuto5", "gbAuto6",
    "gbColorTray", "gbColorTrayMode", "gbEmptyTray", "gbRotateKit", "gbTrayArmMode", "gbUnloadMode",
    "grpCheckLoaderDirection", "grpFix1", "grpFix2", "grpFix3", "grpFix4", "grpFix5",
    "grpFix6", "grpICSort1", "grpICSort2", "grpICSort3", "grpICSort4", "grpICSort5",
    "grpICSort6", "grpICSortFunc", "grpVTestNoTBin", "labFix1", "labFix2", "labFix3",
    "labFix4", "labFix5", "labFix6", "lblHint", "lblHint2", "palAMR",
    "palTrayAssign", "pgRunMode", "pnlFTTrayFrom", "pnlLoad_FT", "pnlLoad_RT", "pnlRTTrayFrom",
    "pnlTop", "rbTemp", "rgAMRReaderPos_KYEC", "rgAuto1_RT", "rgAuto2_RT", "rgAuto3_RT",
    "rgAuto4_RT", "rgAuto5_RT", "rgAuto6_RT", "rgFixTrayMode", "rgLoad_RT", "rgLoaderTrayMode",
    "rgLoaderType", "spbSave", "tsNormalTestGraph", "tsNormalTestGroup", "tsReTestGraph", "tsReTestGroup",
};

// 存檔流程（spbSaveClick、SaveSetupFile、GraphicToRadio、GraphicToRadio_RT）讀的替身 64 個 —— FileRW/TrayForm.cpp 用它決定頁面必須送哪些值
static const char* const kTA_SaveReads[] = {
    "RGAuto1", "RGAuto2", "RGAuto3", "RGAuto4", "RGAuto5", "RGAuto6",
    "RGLoader", "cbAMRTray1st_KYEC", "cbAMRTray2nd_KYEC", "cbAMRTray3rd_KYEC", "cbAMRTrayID1st_KYEC", "cbAMRTrayID2nd_KYEC",
    "cbAMRTrayID3rd_KYEC", "cbColor", "cbEmpty", "cbEnableAMRLoader_KYEC", "cbEnableAMR_KYEC", "cbFix1",
    "cbFix2", "cbFix3", "cbFix4", "cbFix5", "cbFix6", "cbInputRotateKIT",
    "cbLoader", "cbManuTakeAutoFailTray_FT", "cbManuTakeAutoFailTray_RT", "cbMoveAfterTrayGoOut", "cbOutputRotateKIT", "cbSpecTrayCnt",
    "cbVTestNoRTBin", "chkAutoTrayFeed", "chkCheckLoadDirection", "chkColorTray", "chkTrayEndRecvAuto1", "chkTrayEndRecvAuto2",
    "chkTrayEndRecvAuto3", "chkTrayEndRecvAuto4", "chkTrayEndRecvAuto5", "chkTrayEndRecvAuto6", "ckUseFix1", "ckUseFix2",
    "ckUseFix3", "ckUseFix4", "ckUseFix5", "ckUseFix6", "edAMRFullTrayCount", "edAMRInputTrayCount",
    "edNoRTBinFix1", "edNoRTBinFix2", "edNoRTBinFix3", "rgAMRReaderPos_KYEC", "rgAuto1_RT", "rgAuto2_RT",
    "rgAuto3_RT", "rgAuto4_RT", "rgAuto5_RT", "rgAuto6_RT", "rgFixTrayMode", "rgLoad_RT",
    "rgLoaderTrayMode", "rgLoaderType", "sbNormalTest", "sbNormalTest_RT",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void TA_CreateSaveProxies()
{
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto1");
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto2");
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto3");
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto4");
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto5");
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto6");
    EL<TRadioGroup>("TfTrayAssignment", "RGLoader");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTray1st_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTray2nd_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTray3rd_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID1st_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID2nd_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID3rd_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbColor");
    EL<TComboBox>("TfTrayAssignment", "cbEmpty");
    EL<TCheckBox>("TfTrayAssignment", "cbEnableAMRLoader_KYEC");
    EL<TCheckBox>("TfTrayAssignment", "cbEnableAMR_KYEC");
    EL<TComboBox>("TfTrayAssignment", "cbFix1");
    EL<TComboBox>("TfTrayAssignment", "cbFix2");
    EL<TComboBox>("TfTrayAssignment", "cbFix3");
    EL<TComboBox>("TfTrayAssignment", "cbFix4");
    EL<TComboBox>("TfTrayAssignment", "cbFix5");
    EL<TComboBox>("TfTrayAssignment", "cbFix6");
    EL<TComboBox>("TfTrayAssignment", "cbInputRotateKIT");
    EL<TComboBox>("TfTrayAssignment", "cbLoader");
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_FT");
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_RT");
    EL<TCheckBox>("TfTrayAssignment", "cbMoveAfterTrayGoOut");
    EL<TComboBox>("TfTrayAssignment", "cbOutputRotateKIT");
    EL<TCheckBox>("TfTrayAssignment", "cbSpecTrayCnt");
    EL<TCheckBox>("TfTrayAssignment", "cbVTestNoRTBin");
    EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed");
    EL<TCheckBox>("TfTrayAssignment", "chkCheckLoadDirection");
    EL<TCheckBox>("TfTrayAssignment", "chkColorTray");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto1");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto2");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto3");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto4");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto5");
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto6");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix1");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix2");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix3");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix4");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix5");
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix6");
    EL<TEdit>("TfTrayAssignment", "edAMRFullTrayCount");
    EL<TEdit>("TfTrayAssignment", "edAMRInputTrayCount");
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix1");
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix2");
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix3");
    EL<TRadioGroup>("TfTrayAssignment", "rgAMRReaderPos_KYEC");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode");
    EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT");
    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode");
    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType");
    EL<TControl>("TfTrayAssignment", "sbNormalTest");
    EL<TControl>("TfTrayAssignment", "sbNormalTest_RT");
}

// golden cTrayAssignment.cpp:27  TfTrayAssignment::TfTrayAssignment(TComponent* Owner)
static void TA_TfTrayAssignment()
{

    for(int i=0; i<eTrayCount; i++)
        iTrayDirect[i]=0;

    if(bUseAuto2Empty)
    {
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Items->Add("[3]AUTO1");                                       //kevin 20120718新增auto2 氣缸 kyec mft
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Items->Add("[3]AUTO2");
        EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->Items->Add("[3]AUTO1");                                      //kevin 20120718新增auto2 氣缸 kyec mft
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->Items->Add("[3]AUTO2");
    }

    if(CosFunction.bLoaderTrayToAuto1)
    {
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Items->Add("[3]AUTO1");
        EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->Items->Add("[3]AUTO1");
    }

    chkICSort[0]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort1");     chkICSort[1]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort2");      chkICSort[2]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort3");
    chkICSort[3]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort4");     chkICSort[4]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort5");      chkICSort[5]   =EL<TCheckBox>("TfTrayAssignment", "chkICSort6");
    edtICSort[0][0]=EL<TEdit>("TfTrayAssignment", "edtICSort1_1");   edtICSort[0][1]=EL<TEdit>("TfTrayAssignment", "edtICSort1_2");    edtICSort[0][2]=EL<TEdit>("TfTrayAssignment", "edtICSort1_3");
    edtICSort[0][3]=EL<TEdit>("TfTrayAssignment", "edtICSort1_4");   edtICSort[0][4]=EL<TEdit>("TfTrayAssignment", "edtICSort1_5");    edtICSort[0][5]=EL<TEdit>("TfTrayAssignment", "edtICSort1_6");
    edtICSort[1][0]=EL<TEdit>("TfTrayAssignment", "edtICSort2_1");   edtICSort[1][1]=EL<TEdit>("TfTrayAssignment", "edtICSort2_2");    edtICSort[1][2]=EL<TEdit>("TfTrayAssignment", "edtICSort2_3");
    edtICSort[1][3]=EL<TEdit>("TfTrayAssignment", "edtICSort2_4");   edtICSort[1][4]=EL<TEdit>("TfTrayAssignment", "edtICSort2_5");    edtICSort[1][5]=EL<TEdit>("TfTrayAssignment", "edtICSort2_6");
    edtICSort[2][0]=EL<TEdit>("TfTrayAssignment", "edtICSort3_1");   edtICSort[2][1]=EL<TEdit>("TfTrayAssignment", "edtICSort3_2");    edtICSort[2][2]=EL<TEdit>("TfTrayAssignment", "edtICSort3_3");
    edtICSort[2][3]=EL<TEdit>("TfTrayAssignment", "edtICSort3_4");   edtICSort[2][4]=EL<TEdit>("TfTrayAssignment", "edtICSort3_5");    edtICSort[2][5]=EL<TEdit>("TfTrayAssignment", "edtICSort3_6");

    edtICSort[3][0]=EL<TEdit>("TfTrayAssignment", "edtICSort4_1");   edtICSort[3][1]=EL<TEdit>("TfTrayAssignment", "edtICSort4_2");    edtICSort[3][2]=EL<TEdit>("TfTrayAssignment", "edtICSort4_3");
    edtICSort[3][3]=EL<TEdit>("TfTrayAssignment", "edtICSort4_4");   edtICSort[3][4]=EL<TEdit>("TfTrayAssignment", "edtICSort4_5");    edtICSort[3][5]=EL<TEdit>("TfTrayAssignment", "edtICSort4_6");
    edtICSort[4][0]=EL<TEdit>("TfTrayAssignment", "edtICSort5_1");   edtICSort[4][1]=EL<TEdit>("TfTrayAssignment", "edtICSort5_2");    edtICSort[4][2]=EL<TEdit>("TfTrayAssignment", "edtICSort5_3");
    edtICSort[4][3]=EL<TEdit>("TfTrayAssignment", "edtICSort5_4");   edtICSort[4][4]=EL<TEdit>("TfTrayAssignment", "edtICSort5_5");    edtICSort[4][5]=EL<TEdit>("TfTrayAssignment", "edtICSort5_6");
    edtICSort[5][0]=EL<TEdit>("TfTrayAssignment", "edtICSort6_1");   edtICSort[5][1]=EL<TEdit>("TfTrayAssignment", "edtICSort6_2");    edtICSort[5][2]=EL<TEdit>("TfTrayAssignment", "edtICSort6_3");
    edtICSort[5][3]=EL<TEdit>("TfTrayAssignment", "edtICSort6_4");   edtICSort[5][4]=EL<TEdit>("TfTrayAssignment", "edtICSort6_5");    edtICSort[5][5]=EL<TEdit>("TfTrayAssignment", "edtICSort6_6");

}

// golden cTrayAssignment.cpp:64  TfTrayAssignment::FixCanUse()
static void TA_FixCanUse()
{

    if(IniConfig.bUseFix3==true)
    {
        Prod.iTrayType[eFix3]=tTrayFix;
    }
    else if(AUTO_EMPTY_COLOR>=3)
    {
        Prod.iTrayType[eFix3]=tTrayFix;
    }
    else
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            if(DeviceForm_File.XDimension>15)                                   //kevin 20110916
            {
                Prod.iTrayType[eFix3]=tNotUse;                                  //不使用FIX3TRAY
            }
            else
            {
                Prod.iTrayType[eFix3]=tTrayFix;                                 //使用   //kevin 20110916
            }
        }
        else
        {
            Prod.iTrayType[eFix3]=tNotUse;                                      //不使用FIX3TRAY
        }
    }

    if(CosFunction.bLoaderTrayToAuto1)
    {
        if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)
            Prod.iTrayType[eAuto1]=tNotUse;
        else
            Prod.iTrayType[eAuto1]=tTrayAuto;
    }

    if(USE_ROTATE==eCynRot)                                                     //kevin 20130812  氣缸版
    {
        if(iRotate_Out_Tray6>=0)
            Prod.iTrayType[iRotate_Out_Tray6]=tNotUse;
    }

    if(MachineTypeChoice==Type_HT9045 &&
       FIX3_FULL_PLACE==Fix3K_UseCylinder &&
       USE_ROTATE==eMotRot)                                                     //JerryYang 20230608 : HT-9045機型使用汽缸FIX板+Rotate時鎖定不得使用FIX3
    {
        Prod.iTrayType[iFixRight    ]=tNotUse;
        Prod.iTrayType[iFixRightHalf]=tNotUse;
    }

    if(MachineTypeChoice==Type_HT9046_LS && USE_ROTATE_KIT==1 && AUTO3_IS_MAGAZINE==0)  //JerryYang 20260716 : add
    {
        if(iAutoCnt==3)
        {
            Prod.iTrayType[eFix1]=tNotUse;
            Prod.iTrayType[eFix4]=tNotUse;
        }
    }

    fShowBinSelect->SetAutoVisible();

    if(iHWFix_BinBox==1)
    {
        Prod.iTrayType[eBulkBox     ]=tTrayBox;
    }
    else if(IniConfig.bI33ErrorBinBox)
    {
        Prod.iTrayType[eBulkBox     ]=tTrayBox;
    }
    else
    {
        Prod.iTrayType[eBulkBox     ]=tNotUse;
    }

    //==> Ifor 20260702 : Top&Bottom AOI installed occupies Fix2/Fix3 tray slots, force disable at Tray.Data read time
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
    {
        Prod.iTrayType[eFix2]=tNotUse;
        Prod.iTrayType[eFix3]=tNotUse;
        if(TrayForm.iFixTrayMode==1)                                            //Fix Up/Down mode : also disable Fix2/Fix3 lower split slots
        {
            TrayForm.bTrayUpDownSet[eFix2]=false;
            TrayForm.bTrayUpDownSet[eFix3]=false;
            if(AUTO_EMPTY_COLOR>=3)
            {
                Prod.iTrayType[eFix8]=tNotUse;
                Prod.iTrayType[eFix9]=tNotUse;
            }
            else
            {
                Prod.iTrayType[eFix5]=tNotUse;
                Prod.iTrayType[eFix6]=tNotUse;
            }
        }
    }
    //<== Ifor 20260702

}

// golden cTrayAssignment.cpp:163  TfTrayAssignment::ReadFile()
static void TA_ReadFile()
{

    AnsiString S="", Str1, Str2;
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\Tray.Data", DataPath, S);
    AnsiString szDir2="D:\\HT9045\\config\\config.ini";

    if(IniConfig.bP46_LoadTrayModeByHandler)                                    //Steven 20221117 : Loader Tray Mode設定跟著機台變
    {
        TrayForm.iManualRemoveLoader   =ReadIniData(szDir2, "Flag",    "Skip Manual Remove Tray",  0);
    }
    else
    {
        TrayForm.iManualRemoveLoader   =ReadIniData(szDir, "Flag",     "Skip Manual Remove Tray",  0);                  //Steven 20101019       //Steven 20130819 : 加入開門檢查的功能
    }

    TrayForm.iRotateKIT_InputType      =CheckRange(ReadIniData(szDir, "Rotation", "Input",  0), 0, 2);
    TrayForm.iRotateKIT_OutputType     =CheckRange(ReadIniData(szDir, "Rotation", "Output", 0), 0, 2);

    if(iRotate_Type!=eCynRotate)                                                //2013-04-12    Dell 太多地方卡這個參數,所以在馬達版先強用一下
        TrayForm.iRotateKIT_InputType  =1;

    TrayForm.LodareType                =ReadIniData(szDir, "Flag",     "Loader Type",   0);

    if(USE_TRAY_MAPPING==1 &&
       TestIF_File.bEnableTrayID2==true &&
       USE_KEYENCE_EMPTY==3)                                                    //JerryYang 20250113 : Color上方TrayID
    {
        TrayForm.LodareType=1;
    }

    if(CosFunction.bUseTrayUpDownSet==false)
    {
        TrayForm.iFixTrayMode=0;
    }
    else
    {
        if(TrayForm.LodareType==0)                                              //jou 2012-09-04 Loader type 選擇 same , Fix tray mode 強制設定為 Full tray
            TrayForm.iFixTrayMode=0;
        else
            TrayForm.iFixTrayMode      =ReadIniData(szDir, "Flag",     "Fix Tray Mode", 0);
    }

    if(TrayForm.iFixTrayMode==1)                                                //wei 20160224 TSMC FIX UPDOWN
    {
        TrayForm.bTrayUpDownSet[eFix1]=ReadIniData(szDir, "Flag",     "Use Fix1 Tray", 0);
        TrayForm.bTrayUpDownSet[eFix2]=ReadIniData(szDir, "Flag",     "Use Fix2 Tray", 0);
        if(Prod.iTrayType[eFix3]==tNotUse)
            TrayForm.bTrayUpDownSet[eFix3]=false;
        else
            TrayForm.bTrayUpDownSet[eFix3]=ReadIniData(szDir, "Flag", "Use Fix3 Tray", 0);

        if(AUTO_EMPTY_COLOR>=3)
        {
            TrayForm.bTrayUpDownSet[eFix4 ]=ReadIniData(szDir, "Flag", "Use Fix4 Tray", 0);
            TrayForm.bTrayUpDownSet[eFix5 ]=ReadIniData(szDir, "Flag", "Use Fix5 Tray", 0);
            TrayForm.bTrayUpDownSet[eFix6 ]=ReadIniData(szDir, "Flag", "Use Fix6 Tray", 0);

            TrayForm.bTrayUpDownSet[eFix7 ]=TrayForm.bTrayUpDownSet[eFix1];
            TrayForm.bTrayUpDownSet[eFix8 ]=TrayForm.bTrayUpDownSet[eFix2];
            TrayForm.bTrayUpDownSet[eFix9 ]=TrayForm.bTrayUpDownSet[eFix3];
            TrayForm.bTrayUpDownSet[eFix10]=TrayForm.bTrayUpDownSet[eFix4];
            TrayForm.bTrayUpDownSet[eFix11]=TrayForm.bTrayUpDownSet[eFix5];
            TrayForm.bTrayUpDownSet[eFix12]=TrayForm.bTrayUpDownSet[eFix6];
        }
        else
        {
            TrayForm.bTrayUpDownSet[eFix4 ]=TrayForm.bTrayUpDownSet[eFix1];
            TrayForm.bTrayUpDownSet[eFix5 ]=TrayForm.bTrayUpDownSet[eFix2];
            TrayForm.bTrayUpDownSet[eFix6 ]=TrayForm.bTrayUpDownSet[eFix3];
        }
    }
    else
    {
        TrayForm.bTrayUpDownSet[eFix1 ]=false;
        TrayForm.bTrayUpDownSet[eFix2 ]=false;
        TrayForm.bTrayUpDownSet[eFix3 ]=false;
        TrayForm.bTrayUpDownSet[eFix4 ]=false;
        TrayForm.bTrayUpDownSet[eFix5 ]=false;
        TrayForm.bTrayUpDownSet[eFix6 ]=false;
        TrayForm.bTrayUpDownSet[eFix7 ]=false;
        TrayForm.bTrayUpDownSet[eFix8 ]=false;
        TrayForm.bTrayUpDownSet[eFix9 ]=false;
        TrayForm.bTrayUpDownSet[eFix10]=false;
        TrayForm.bTrayUpDownSet[eFix11]=false;
        TrayForm.bTrayUpDownSet[eFix12]=false;
    }

    if(AUTO_EMPTY_COLOR>=3)
    {
        Prod.iTrayType[eFix7 ]=(TrayForm.bTrayUpDownSet[eFix1])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix8 ]=(TrayForm.bTrayUpDownSet[eFix2])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix9 ]=(TrayForm.bTrayUpDownSet[eFix3])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix10]=(TrayForm.bTrayUpDownSet[eFix4])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix11]=(TrayForm.bTrayUpDownSet[eFix5])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix12]=(TrayForm.bTrayUpDownSet[eFix6])?tTrayFix:tNotUse;
    }
    else
    {
        Prod.iTrayType[eFix4]=(TrayForm.bTrayUpDownSet[eFix1])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix5]=(TrayForm.bTrayUpDownSet[eFix2])?tTrayFix:tNotUse;
        Prod.iTrayType[eFix6]=(TrayForm.bTrayUpDownSet[eFix3])?tTrayFix:tNotUse;
    }

    if(CosFunction.bUnloadTrayModeByRecipe==false)                              //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    {
        TrayForm.bAutoFeed             =ReadIniData(szDir, "Flag", "Auto Feed",                false);
        TrayForm.bFailAutoTrayManual_FT=ReadIniData(szDir, "Flag", "bFailAutoTrayManual_FT",   false);                  //Steven 20150116 : 手動移除Auto Fail Bin Tray
        TrayForm.bFailAutoTrayManual_RT=ReadIniData(szDir, "Flag", "bFailAutoTrayManual_RT",   false);                  //Steven 20150116 : 手動移除Auto Fail Bin Tray
    }

    if(fMain->hanaART->IsHanaArtAvailable())                                    //Steven 20251007 : for Hana ART
        TrayForm.bAutoFeed          =true;

    //AI(ht9045-v899) 20260817: CC_CYUEAN 固定關閉 Auto Tray Feed, 不受工作檔 Tray.Data 與 hanaART 影響
    if(CosFunction.bDisableAutoTrayFeed)
        TrayForm.bAutoFeed          =false;

    TrayForm.bColorTray             =ReadIniData(szDir, "Flag", "Color Tray Sensor",    false);                         //20140903 wei colcr Tray
    TrayForm.bChkLoadDirection      =ReadIniData(szDir, "Flag", "Check loader tray direction", false);                  //Steven 20190815 : JCET不重測Tray偵測
    TrayForm.bMoveAfterTrayGoOut    =ReadIniData(szDir, "Flag", "MoveAfterTrayGoOut",    false);                        //JerryYang 20241002 : SPIL要求功能 Trray arm等AUTO TRAY退到外面才能移動

    TrayForm.bSpecTrayCnt           =ReadIniData(szDir, "Flag",     "bSpecTrayCnt",    false);                          //JerryYang 20250220 : AUTO IN OUT

    TrayForm.bVTestNoRTBin          =ReadIniData(szDir, "VTest",    "NoRTBin",         false);                          //RogerYang 20250814 : 偉測不可複測bin功能
    TrayForm.asNoRTBinFix[0]        =ReadIniData(szDir, "VTest",    "NoRTBinFix1",     AnsiString(""));
    TrayForm.asNoRTBinFix[1]        =ReadIniData(szDir, "VTest",    "NoRTBinFix2",     AnsiString(""));
    TrayForm.asNoRTBinFix[2]        =ReadIniData(szDir, "VTest",    "NoRTBinFix3",     AnsiString(""));

    TrayForm.iFullTrayCount         =ReadIniData(szDir, "AMR",      "Full Tray Count",    12);
    TrayForm.iInputTrayCount        =ReadIniData(szDir, "AMR",      "Input Tray Count",   12);
    //==> Eastsun 20260512 F009 整合: 補入 AMR section 缺失 wiring (D1.1.1)
    TrayForm.bEnableAMRLoader       =ReadIniData(szDir, "AMR",      "Enahle AMR Loader",  false);
    if(TrayForm.bEnableAMRLoader==true &&
       (fMain->cbRunStartMode->Text=="Re-Test Continuous" || fMain->cbRunStartMode->Text=="Re-Test Initial Start"))
        TrayForm.bEnableAMR=false;                                              //Eastsun 20260512 F011 Phase 6 (a-side L239-L243 Re-Test override)
    else
        TrayForm.bEnableAMR         =ReadIniData(szDir, "AMR",      "Enahle AMR",         false);
    TrayForm.iReaderPos             =ReadIniData(szDir, "AMR",      "Reader Pos",         0);
    TrayForm.iTrayOrder[0]          =ReadIniData(szDir, "AMR",      "1st Tray Type",      0);
    TrayForm.iTrayOrder[1]          =ReadIniData(szDir, "AMR",      "2nd Tray Type",      0);
    TrayForm.iTrayOrder[2]          =ReadIniData(szDir, "AMR",      "3rd Tray Type",      0);
    TrayForm.bIDTrayOrder[0]        =ReadIniData(szDir, "AMR",      "1st ID Use",         false);
    TrayForm.bIDTrayOrder[1]        =ReadIniData(szDir, "AMR",      "2nd ID Use",         false);
    TrayForm.bIDTrayOrder[2]        =ReadIniData(szDir, "AMR",      "3rd ID Use",         false);
    if(TrayForm.iTrayOrder[1]==eNoUse)                                          //Eastsun 20260512 F011 Phase 6 (a-side L249-L254 iAMRCoverTray)
        iAMRCoverTray=1;
    else if(TrayForm.iTrayOrder[2]==eNoUse)
        iAMRCoverTray=2;
    else
        iAMRCoverTray=3;
    //<== Eastsun 20260512 F009 整合

    if(TrayForm.bSpecTrayCnt)                                                   //JerryYang 20250220 : AUTO IN OUT
    {
        iBundleIn=TrayForm.iFullTrayCount;
        iBundleOut=TrayForm.iInputTrayCount;
    }

    if(CUSTOMER_CODE==CC_SCS)                                                   //2013-10-08    Dell Fix
    {
        TrayForm.iManualRemoveLoader=2;
    }
    else if(IniConfig.bSPILFunction==true)                                      //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction  //wei 20150317 新竹矽品 auto skip 強制手動拿Tray
    {
        if(IniConfig.bP44LockLoaderTrayToNone)                                  //JerryYang 20220331 : 矽品中山要求AUTO SKIP後可由TRAY ARM搬TRAY
        {
            TrayForm.iManualRemoveLoader=0;
        }
        else
        {
            TrayForm.iManualRemoveLoader=1;
        }
    }
    else if(CUSTOMER_CODE==CC_SCS || CUSTOMER_CODE==CC_KYEC_XILINX)             //jou 20170413 (Steven) : KYEC Xilinx 要求"When Loader had skip device, need open the door to check the tray."寫為固定選項
    {
        TrayForm.iManualRemoveLoader=2;
    TA_rgLoaderTrayModeItems=false;   // golden cTrayAssignment.cpp:341-343 取代：rgLoaderTrayMode->Controls[0..2]->Enabled=false：vclcompat TRadioGroup 沒有 Controls[]；golden 三顆一起設、DFM 只有三項 ⇒ 等同「這組不能改」，記旗標，FormShow 結尾併進 Enabled
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:341-343 -- rgLoaderTrayMode->Controls[0..2]->Enabled=false：vclcompat TRadioGroup 沒有 Controls[]；golden 三顆一起設、DFM 只有三項 ⇒ 等同「這組不能改」，記旗標，FormShow 結尾併進 Enabled
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[0]->Enabled=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[1]->Enabled=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[2]->Enabled=false;
#endif // GATE (S12-C save)
    }
    else if(bEnable_KLT_Function==true)
    {
        if(ArmSpeed_File[InArm].bAutoSKIP)                                      //Ifor 20180802 : add KLT Auto SKIP 功能需求
        {
            TrayForm.iManualRemoveLoader=1;
    TA_rgLoaderTrayModeItems=false;   // golden cTrayAssignment.cpp:350-352 取代：同 :341-343（KLT Auto SKIP）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:350-352 -- 同 :341-343（KLT Auto SKIP）
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[0]->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[1]->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[2]->Enabled=false;
#endif // GATE (S12-C save)
            if(iHasReadeLastData==1)
                iHasReadeLastData=2;
        }
        else
        {
    TA_rgLoaderTrayModeItems=true;   // golden cTrayAssignment.cpp:358-360 取代：同 :341-343（KLT 非 Auto SKIP：三顆打開）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:358-360 -- 同 :341-343（KLT 非 Auto SKIP：三顆打開）
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[0]->Enabled=true;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[1]->Enabled=true;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[2]->Enabled=true;
#endif // GATE (S12-C save)
        }
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                         //Ifor 20190703 : add KYEC 強制手動取TRAY
    {
        //==> Eastsun 20260512 F011 Phase 6 (a-side cTrayAssignment.cpp L182-L196 KYEC AMR iManualRemoveLoader + bAutoFeed)
        if(TrayForm.bEnableAMR)                                                 //Eastsun 20260423 (a-side L182)
        {
            TrayForm.iManualRemoveLoader=0;
        }
        else
        {
            TrayForm.iManualRemoveLoader=1;
        }
    TA_rgLoaderTrayModeItems=false;   // golden cTrayAssignment.cpp:374-376 取代：同 :341-343（CC_KYEC_LEE）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:374-376 -- 同 :341-343（CC_KYEC_LEE）
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[0]->Enabled=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[1]->Enabled=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Controls[2]->Enabled=false;
#endif // GATE (S12-C save)
        TrayForm.bAutoFeed=true;                                                //Eastsun 20260512 F011 Phase 6 (a-side L194 KYEC AMR Auto Feed)
        EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Enabled=false;
        //<== Eastsun 20260512 F011 Phase 6
    }

    TrayForm.AutoCoverInitial          =ReadIniData(szDir, "Flag",     "Auto Cover Initial", false);
    TrayForm.AutoCoverRetest           =ReadIniData(szDir, "Flag",     "Auto Cover Retest",  false);

    TrayForm.Loader.iTrayType          =ReadIniData(szDir, "Loader",   "Tray Type",     0);
    TrayForm.Loader.Direction          =ReadIniData(szDir, "Loader",   "Direction",     0);
    TrayForm.LoaderToEmptyColor[FT]    =ReadIniData(szDir, "Loader",   "ToBuffer",      0);
    TrayForm.LoaderToEmptyColor[RT]    =ReadIniData(szDir, "Loader",   "ToBuffer_RT",   0);

    if(USE_TRAY_MAPPING==1 &&
       TestIF_File.bEnableTrayID2==true &&
       USE_KEYENCE_EMPTY==3)                                                    //JerryYang 20250113 : Color上方TrayID
    {
        TrayForm.LoaderToEmptyColor[FT]    =0;
        TrayForm.LoaderToEmptyColor[RT]    =0;
    }

    TrayForm.Empty.iTrayType           =ReadIniData(szDir, "Empty",    "Tray Type",     0);
    TrayForm.Color.iTrayType           =ReadIniData(szDir, "Color",    "Tray Type",     0);

    TA_FixCanUse();
    if(USE_TRAY_MAPPING==1 &&
       TestIF_File.bEnableTrayID2==true &&
       USE_KEYENCE_EMPTY==3)                                                    //JerryYang 20250113 : Color上方TrayID
    {
        TrayForm.AutoFromEmptyColor[FT][0] =1;
        TrayForm.AutoFromEmptyColor[RT][0] =1;

        TrayForm.AutoFromEmptyColor[FT][1] =1;
        TrayForm.AutoFromEmptyColor[RT][1] =1;

        TrayForm.AutoFromEmptyColor[FT][2] =1;
        TrayForm.AutoFromEmptyColor[RT][2] =1;
    }

    for(int i=0; i<=iFixRight; i++)
    {
        if(Prod.iTrayType[i]==tTrayBox)
        {
            TrayForm.Auto[i].iTrayType =0;
            TrayForm.Auto[i].Direction =0;
        }
        else if(Prod.iTrayType[i]!=tNotUse)
        {
            TrayForm.Auto[i].iTrayType =ReadIniData(szDir, s6TrayName[i],  "Tray Type",        0);
            TrayForm.Auto[i].Direction =ReadIniData(szDir, s6TrayName[i],  "Direction",        0);
        }
        else if(i<=eAuto6)                                                      //避免沒資料
        {
            TrayForm.Auto[i].iTrayType =TrayForm.Auto[eAuto1].iTrayType;
            TrayForm.Auto[i].Direction =TrayForm.Auto[eAuto1].iTrayType;
        }
        else if(i<=eFix12)
        {
            TrayForm.Auto[i].iTrayType =TrayForm.Auto[eFix1].iTrayType;
            TrayForm.Auto[i].Direction =TrayForm.Auto[eFix1].iTrayType;
        }
        else if(i<=eMag14)
        {
            TrayForm.Auto[i].iTrayType =TrayForm.Auto[iMagAtAuto].iTrayType;
            TrayForm.Auto[i].Direction =TrayForm.Auto[iMagAtAuto].iTrayType;
        }

        if(Prod.iTrayType[i]==tTrayAuto)
        {
            TrayForm.AutoFromEmptyColor[FT][eAuto1+i] =ReadIniData(szDir, s6TrayName[i], "FromBuffer",       0);        //JerryYang 20231001
            TrayForm.AutoFromEmptyColor[RT][eAuto1+i] =ReadIniData(szDir, s6TrayName[i], "FromBuffer_RT",    0);
            if(IniConfig.bP09TrayEndCanSelectTray)                              //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
                TrayForm.Auto[eAuto1+i].TrayEndRecv   =ReadIniData(szDir, s6TrayName[i], "TrayEndRecv",      false);
        }
    }

    if(TrayForm.iFixTrayMode==1)                                               //AI(ht9045-v899) 20260611(CASE-20260611-001): 上面迴圈只讀到iFixRight為止,Fix下半盤(eFix4~6 / eFix7~12)的Direction/iTrayType從未設定,預設停在0導致下半盤(如Fix5)擺放方向與上半盤不一致;此處仿bTrayUpDownSet同步,使下半盤方向/盤型跟隨對應上半盤
    {
        if(AUTO_EMPTY_COLOR>=3)
        {
            TrayForm.Auto[eFix7 ].Direction=TrayForm.Auto[eFix1].Direction;
            TrayForm.Auto[eFix8 ].Direction=TrayForm.Auto[eFix2].Direction;
            TrayForm.Auto[eFix9 ].Direction=TrayForm.Auto[eFix3].Direction;
            TrayForm.Auto[eFix10].Direction=TrayForm.Auto[eFix4].Direction;
            TrayForm.Auto[eFix11].Direction=TrayForm.Auto[eFix5].Direction;
            TrayForm.Auto[eFix12].Direction=TrayForm.Auto[eFix6].Direction;

            TrayForm.Auto[eFix7 ].iTrayType=TrayForm.Auto[eFix1].iTrayType;
            TrayForm.Auto[eFix8 ].iTrayType=TrayForm.Auto[eFix2].iTrayType;
            TrayForm.Auto[eFix9 ].iTrayType=TrayForm.Auto[eFix3].iTrayType;
            TrayForm.Auto[eFix10].iTrayType=TrayForm.Auto[eFix4].iTrayType;
            TrayForm.Auto[eFix11].iTrayType=TrayForm.Auto[eFix5].iTrayType;
            TrayForm.Auto[eFix12].iTrayType=TrayForm.Auto[eFix6].iTrayType;
        }
        else
        {
            TrayForm.Auto[eFix4 ].Direction=TrayForm.Auto[eFix1].Direction;
            TrayForm.Auto[eFix5 ].Direction=TrayForm.Auto[eFix2].Direction;
            TrayForm.Auto[eFix6 ].Direction=TrayForm.Auto[eFix3].Direction;

            TrayForm.Auto[eFix4 ].iTrayType=TrayForm.Auto[eFix1].iTrayType;
            TrayForm.Auto[eFix5 ].iTrayType=TrayForm.Auto[eFix2].iTrayType;
            TrayForm.Auto[eFix6 ].iTrayType=TrayForm.Auto[eFix3].iTrayType;
        }
    }

    if(bUseAuto2Empty==true)                                                    //kevin 20120726 Auto2 送空tray auto3 強至用 auto2空tray
    {
        if(TrayForm.AutoFromEmptyColor[FT][eAuto2]==2)
            TrayForm.AutoFromEmptyColor[FT][eAuto3]=2;

        if(TrayForm.AutoFromEmptyColor[RT][eAuto2]==2)
            TrayForm.AutoFromEmptyColor[RT][eAuto3]=2;
    }
    else                                                                        //jou 2012-11-28 當沒有安裝 Auto 2 Empty 時，必須強制把 TrayForm.AutoFromEmptyColor[RT][2]=2 -> 1
    {
        if(TrayForm.AutoFromEmptyColor[FT][eAuto3]>=2)
            TrayForm.AutoFromEmptyColor[FT][eAuto3]=1;
        if(TrayForm.AutoFromEmptyColor[RT][eAuto3]>=2)
            TrayForm.AutoFromEmptyColor[RT][eAuto3]=1;
    }

    if(IniConfig.bE71_10TrayPitchLockTrayAssign==true)                          //Ifor 20221212 add:KYEC 要求 Tray Form X或Y 小於 10mm 強制開啟
    {
        if((UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100<=1000 &&     //Eastsun 20260515 F023: E19 skip zero pitch
            UserDefForm_File[TrayForm.Loader.iTrayType].XPitch>0) ||
           (UserDefForm_File[TrayForm.Loader.iTrayType].YPitch*100<=1000 &&
            UserDefForm_File[TrayForm.Loader.iTrayType].YPitch>0) )
        {
            for(int i=0; i<=iAutoRight; i++)
            {
                if(Prod.iTrayType[i]==tTrayAuto)
                {
                    TrayForm.AutoFromEmptyColor[FT][i]=1;
                    TrayForm.AutoFromEmptyColor[RT][i]=1;
                }
            }
            TrayForm.LoaderToEmptyColor[FT]=0;
            TrayForm.LoaderToEmptyColor[RT]=0;
            TrayForm.LodareType=1;                                              //Ifor 20221220 add:需變更為Different才能顯示
        }
    }

    iBackupLoadDir=TrayForm.Loader.Direction;                                   //JerryYang 20201111 iTrayType->Direction  //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看

    if(CosFunction.bTraySortCntFunc)                                            //Steven 20210608 : Tray指定數量退盤功能
    {
        for(int i=0; i<=iAutoRight; i++)
        {
            if(Prod.iTrayType[i]==tTrayAuto)
            {
                TrayForm.bTraySortCntFunc[i]=ReadIniData(szDir, s6TrayName[i],  "bTraySortCntFunc",  false);

                for(int j=0; j<6; j++)
                {
                    Str2.sprintf("TraySortCnt%d_%d", i+1, j+1);
                    TrayForm.iTraySortCntFunc[i][j]=ReadIniData(szDir, s6TrayName[i], Str2, 0);
                }
            }
        }
    }
    if(fLotInfo!=NULL) //Eastsun 20260710 Merge
    {
        if(TrayForm.bEnableAMR)
            fLotInfo->tsKYEC_AMR->TabVisible=true;
        else
            fLotInfo->tsKYEC_AMR->TabVisible=false;
    }


}

// golden cTrayAssignment.cpp:548  TfTrayAssignment::DoIniDataToForm()
static void TA_DoIniDataToForm()
{

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbInputRotateKIT"), TrayForm.iRotateKIT_InputType);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbOutputRotateKIT"), TrayForm.iRotateKIT_OutputType);

    EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Checked            =TrayForm.bAutoFeed;
    EL<TCheckBox>("TfTrayAssignment", "chkColorTray")->Checked               =TrayForm.bColorTray;                   //20140903 wei colcr Tray

    EL<TCheckBox>("TfTrayAssignment", "cbMoveAfterTrayGoOut")->Checked       =TrayForm.bMoveAfterTrayGoOut;          //JerryYang 20241002 : SPIL要求功能 Trray arm等AUTO TRAY退到外面才能移動

    EL<TCheckBox>("TfTrayAssignment", "chkCheckLoadDirection")->Checked      =TrayForm.bChkLoadDirection;            //Steven 20190815 : JCET不重測Tray偵測

    EL<TCheckBox>("TfTrayAssignment", "cbSpecTrayCnt")->Checked              =TrayForm.bSpecTrayCnt;                 //JerryYang 20250220 : AUTO IN OUT

    EL<TCheckBox>("TfTrayAssignment", "cbVTestNoRTBin")->Checked             =TrayForm.bVTestNoRTBin;                //RogerYang 20250814 : 偉測不可複測bin功能
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix1")->Text                 =TrayForm.asNoRTBinFix[0];
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix2")->Text                 =TrayForm.asNoRTBinFix[1];
    EL<TEdit>("TfTrayAssignment", "edNoRTBinFix3")->Text                 =TrayForm.asNoRTBinFix[2];

    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->ItemIndex         =TrayForm.iManualRemoveLoader;          //Steven 20101019 : Loader有Skip的話，就要手動拿走Loader Tray           //Steven 20130819 : 加入開門檢查的功能
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_FT")->Checked  =TrayForm.bFailAutoTrayManual_FT;       //Steven 20150116 : 手動移除Auto Fail Bin Tray
    EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_RT")->Checked  =TrayForm.bFailAutoTrayManual_RT;       //Steven 20150116 : 手動移除Auto Fail Bin Tray
    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex             =TrayForm.LodareType;
    EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex            =TrayForm.iFixTrayMode;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbLoader"), TrayForm.Loader.iTrayType);
    iTrayDirect[0]                      =TrayForm.Loader.Direction;
    EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex                 =TrayForm.LoaderToEmptyColor[FT];
    EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex                =TrayForm.LoaderToEmptyColor[RT];

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbEmpty"), TrayForm.Empty.iTrayType);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbColor"), TrayForm.Color.iTrayType);

    EL<TEdit>("TfTrayAssignment", "edAMRFullTrayCount")->Text            =TrayForm.iFullTrayCount;
    EL<TEdit>("TfTrayAssignment", "edAMRInputTrayCount")->Text           =TrayForm.iInputTrayCount;              //JerryYang 20250220 : AUTO IN OUT
    //==> Eastsun 20260512 F009 整合: AMR DFM UI binding (D1.1.4)
    EL<TCheckBox>("TfTrayAssignment", "cbEnableAMR_KYEC")->Checked           =TrayForm.bEnableAMR;
    EL<TCheckBox>("TfTrayAssignment", "cbEnableAMRLoader_KYEC")->Checked     =TrayForm.bEnableAMRLoader;
    EL<TRadioGroup>("TfTrayAssignment", "rgAMRReaderPos_KYEC")->ItemIndex      =TrayForm.iReaderPos;
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTray1st_KYEC"), TrayForm.iTrayOrder[0]);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTray2nd_KYEC"), TrayForm.iTrayOrder[1]);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTray3rd_KYEC"), TrayForm.iTrayOrder[2]);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID1st_KYEC"), TrayForm.bIDTrayOrder[0]);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID2nd_KYEC"), TrayForm.bIDTrayOrder[1]);
    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID3rd_KYEC"), TrayForm.bIDTrayOrder[2]);
    //<== Eastsun 20260512 F009 整合

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto1];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto1];
    iTrayDirect[eAuto1+1]               =TrayForm.Auto[0].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto1")->Checked    =TrayForm.Auto[eAuto1].TrayEndRecv;

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto2];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto2];
    iTrayDirect[eAuto2+1]               =TrayForm.Auto[eAuto2].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto2")->Checked    =TrayForm.Auto[eAuto2].TrayEndRecv;

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto3];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto3];
    iTrayDirect[eAuto3+1]               =TrayForm.Auto[eAuto3].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto3")->Checked    =TrayForm.Auto[eAuto3].TrayEndRecv;

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto4];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto4];
    iTrayDirect[eAuto4+1]               =TrayForm.Auto[eAuto4].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto4")->Checked    =TrayForm.Auto[eAuto4].TrayEndRecv;

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto5];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto5];
    iTrayDirect[eAuto5+1]               =TrayForm.Auto[eAuto5].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto5")->Checked    =TrayForm.Auto[eAuto5].TrayEndRecv;

    EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex                  =TrayForm.AutoFromEmptyColor[FT][eAuto6];
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex               =TrayForm.AutoFromEmptyColor[RT][eAuto6];
    iTrayDirect[eAuto6+1]               =TrayForm.Auto[eAuto6].Direction;
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto6")->Checked    =TrayForm.Auto[eAuto6].TrayEndRecv;

    if(IniConfig.bBinBox)                                                       //jou 2012-12-11 support Bin Box start
    {
        if(AUTO_EMPTY_COLOR>=3)                                                 //Steven 20230907 : For HT-9011UC
        {
            if(EL<TComboBox>("TfTrayAssignment", "cbFix6")->Items->Count<=iBinBoxType)
            {
                EL<TComboBox>("TfTrayAssignment", "cbFix6")->Items->Add("Bin Box");
            }
        }
        else
        {
            if(EL<TComboBox>("TfTrayAssignment", "cbFix3")->Items->Count<=iBinBoxType)
            {
                EL<TComboBox>("TfTrayAssignment", "cbFix3")->Items->Add("Bin Box");
            }
        }
    }

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix1"), TrayForm.Auto[eFix1].iTrayType);
    iTrayDirect[eFix1+1]                =TrayForm.Auto[eFix1].Direction;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix2"), TrayForm.Auto[eFix2].iTrayType);
    iTrayDirect[eFix2+1]                =TrayForm.Auto[eFix2].Direction;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix3"), TrayForm.Auto[eFix3].iTrayType);
    iTrayDirect[eFix3+1]                =TrayForm.Auto[eFix3].Direction;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix4"), TrayForm.Auto[eFix4].iTrayType);
    iTrayDirect[eFix4+1]                =TrayForm.Auto[eFix4].Direction;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix5"), TrayForm.Auto[eFix5].iTrayType);
    iTrayDirect[eFix5+1]                =TrayForm.Auto[eFix5].Direction;

    filerw::ELComboIndex(EL<TComboBox>("TfTrayAssignment", "cbFix6"), TrayForm.Auto[eFix6].iTrayType);
    iTrayDirect[eFix6+1]                =TrayForm.Auto[eFix6].Direction;

    if(IniConfig.bTrayAssignUseGraphic &&                                       //Steven 20111121 : 使用圖片去顯示Tray Assign
       AUTO_EMPTY_COLOR<3)                                                      //Steven 20230915 : HT9016UC暫時不支援圖像模式
    {
        TA_RadioToGraphic();
        TA_RadioToGraphic_RT();
    }

    if(CosFunction.bUseTrayUpDownSet)                                           //wei 20160224 TSMC FIX UPDOWN
    {
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix1")->Checked  =TrayForm.bTrayUpDownSet[eFix1];
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix2")->Checked  =TrayForm.bTrayUpDownSet[eFix2];
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix3")->Checked  =TrayForm.bTrayUpDownSet[eFix3];
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix4")->Checked  =TrayForm.bTrayUpDownSet[eFix4];
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix5")->Checked  =TrayForm.bTrayUpDownSet[eFix5];
        EL<TCheckBox>("TfTrayAssignment", "ckUseFix6")->Checked  =TrayForm.bTrayUpDownSet[eFix6];
    }

    if(CosFunction.bTraySortCntFunc)                                            //Steven 20210608 : Tray指定數量退盤功能
    {
        if(HasICUnderMachine() || HasAnyICInMachine())
        {
            EL<TGroupBox>("TfTrayAssignment", "grpICSort1")->Enabled=false;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort2")->Enabled=false;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort3")->Enabled=false;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort4")->Enabled=false;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort5")->Enabled=false;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort6")->Enabled=false;
        }
        else
        {
            EL<TGroupBox>("TfTrayAssignment", "grpICSort1")->Enabled=true;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort2")->Enabled=true;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort3")->Enabled=true;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort4")->Enabled=true;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort5")->Enabled=true;
            EL<TGroupBox>("TfTrayAssignment", "grpICSort6")->Enabled=true;
        }

        EL<TGroupBox>("TfTrayAssignment", "grpICSortFunc")->Visible=true;
        for(int i=0; i<=iAutoRight; i++)
        {
            if(Prod.iTrayType[i]==tTrayAuto)
            {
                chkICSort[i]->Checked=TrayForm.bTraySortCntFunc[i];

                for(int j=0; j<6; j++)
                    edtICSort[i][j]->Text=TrayForm.iTraySortCntFunc[i][j];
            }
        }
    }
    else
    {
        EL<TGroupBox>("TfTrayAssignment", "grpICSortFunc")->Visible=false;
    }

}

// golden cTrayAssignment.cpp:742  TfTrayAssignment::FormShow(TObject *Sender)
static void TA_FormShow()
{

    AnsiString S;
    ;   // golden cTrayAssignment.cpp:745-745 取代：LoadImage()：方向示意圖（HTML 自己有）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:745-745 -- LoadImage()：方向示意圖（HTML 自己有）
    LoadImage();
#endif // GATE (S12-C save)
    TA_ReadFile();
    TA_DoIniDataToForm();
    TA_ShowCompnet();
    S.sprintf("Tray Assignment  ''%s''  ",GetLastOpenFN());
    Caption=S;
    TA_ShowTrayDirectIMG();

    if(USE_ROTATE_KIT==1)
        EL<TGroupBox>("TfTrayAssignment", "gbRotateKit")->Visible=(iRotate_Type==eCynRotate);

    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
    EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex);
    EL<TGroupBox>("TfTrayAssignment", "gbColorTrayMode")->Visible=(USE_COLOR_TRAY_SENSOR==1);                        //20140903 wei colcr Tray

    if(CosFunction.bSpecialP24)                                                 //Sam 20230320 : 整合 P24 北興版為獨立功能碼
    {
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex=0;
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TLabel>("TfTrayAssignment", "lblHint")->Visible=IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
    }
    else
    {
        EL<TLabel>("TfTrayAssignment", "lblHint")->Visible=false;
    }

    if(IniConfig.bP18FailAutoTrayManual==true)                                  //ChungHung 20120329 FailBin AutoTray Manual
    {
        if(Prod.iIsFailT6[eAuto1]==1)                                           //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=false;
        else
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=true;

        if(Prod.iIsFailT6[eAuto2]==1)                                           //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Enabled=false;
        else
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Enabled=true;

        if(Prod.iIsFailT6[eAuto3]==1)                                           //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=false;
        else
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=true;
    }
    else
    {
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=true;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Enabled=true;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=true;
    }

    if((bUseAuto2Empty || CosFunction.bLoaderTrayToAuto1) && EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex==2)                                    //kevin 20120718 判別 auto1 是否有設 bin別
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=false;
    else
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=true;

    if(bUseAuto2Empty && EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex==2)                                 //kevin 20120718 判別 auto1 是否有設 bin別
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=false;
    else
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=true;

    if((bUseAuto2Empty || CosFunction.bLoaderTrayToAuto1) && EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex==2)                                   //kevin 20120718 判別 auto1 是否有設 bin別
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->Enabled=false;
    else
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->Enabled=true;

    if(bUseAuto2Empty && EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex==2)                              //kevin 20120718 判別 auto1 是否有設 bin別
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->Enabled=false;
    else
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->Enabled=true;

    if(USE_ROTATE_KIT==1)
        EL<TGroupBox>("TfTrayAssignment", "gbRotateKit")->Visible=(iRotate_Type==eCynRotate);

    if(CosFunction.bSpecialP24)                                                 //Sam 20230320 : 整合 P24 北興版為獨立功能碼
    {
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex=0;
        EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;   //Sam 20231129 : 力成也要矽格北興P24功能
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TLabel>("TfTrayAssignment", "lblHint")->Visible=IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;

        EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;                                         //Sam 20231129 : 力成也要矽格北興P24功能
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->Enabled=!IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
        EL<TLabel>("TfTrayAssignment", "lblHint2")->Visible=IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray;
    }
    else
    {
        EL<TLabel>("TfTrayAssignment", "lblHint")->Visible=false;
        EL<TLabel>("TfTrayAssignment", "lblHint2")->Visible=false;                                                //Sam 20231129 : 力成也要矽格北興P24功能
    }

    if(USE_TRAY_MAPPING==1 &&
       TestIF_File.bEnableTrayID2==true &&
       USE_KEYENCE_EMPTY==3)                                                    //JerryYang 20250113 : Color上方TrayID
    {
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->Enabled=false;

            EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->Enabled=false;

            EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->Enabled=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->Enabled=false;
    }

    if(fMain->hanaART->IsHanaArtAvailable())                                    //Steven 20251007 : for Hana ART
        EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Enabled        =false;

    //AI(ht9045-v899) 20260817: CC_CYUEAN 固定關閉 Auto Tray Feed, 取消勾選並鎖定(只鎖這一顆, 同群組其他兩項不動)
    if(CosFunction.bDisableAutoTrayFeed)
    {
        EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Checked        =false;
        EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Enabled        =false;
    }

    if(CUSTOMER_CODE==CC_SCC ||                                                 //Steven 20101221
       CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20130621 add SCK RMS
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =false;                                 //Steven 20130819 : 加入開門檢查的功能
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =false;
        }
        else if(AccessLevel<LevelSet.AccessLevel[16])
        {
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =false;
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =false;
        }
        else
        {
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =true;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =true;                                  //Steven 20130819 : 加入開門檢查的功能
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =true;
        }
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE ||                                       //20140320 wei   KYEC 低於權限顯示不能修改
            CUSTOMER_CODE==CC_KYEC_XILINX)
    {
        if(AccessLevel<LevelSet.AccessLevel[16])
        {
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =false;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =false;
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =false;
        }
        else
        {
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =true;
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =true;
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =true;
        }
    }
    else if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)         //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[20])
        {
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled   =false;
            EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled      =false;
            EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled       =false;
        }
        else
        {
            if(AccessLevel<LevelSet.AccessLevel[16])
                EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false;
            else
                EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=fSecurity->Insufficient(102, false);
        }
    }
    else
    {
        if(CUSTOMER_CODE==CC_SCS)                                               //Steven 20130819 : Chew要求要直接設定,不能改  //2013-09-28 Dell modify for CC_AMKOR_Korea
        {
            EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false;
        }
        else                                                                    //Steven 20150416 : 新的權限設定
        {
            if(AccessLevel<LevelSet.AccessLevel[16])
                EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false;
            else
                EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=fSecurity->Insufficient(102, false);
        }

        EL<TPanel>("TfTrayAssignment", "palTrayAssign")->Enabled=fSecurity->Insufficient(16, false);
        EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled =fSecurity->Insufficient(16, false);
    }

    if(CosFunction.bUnloadTrayModeByRecipe==true)                               //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    {
        EL<TGroupBox>("TfTrayAssignment", "gbUnloadMode")->Enabled=false;
    }

    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto1")->Visible=IniConfig.bP09TrayEndCanSelectTray;            //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto2")->Visible=IniConfig.bP09TrayEndCanSelectTray;            //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto3")->Visible=IniConfig.bP09TrayEndCanSelectTray;            //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto4")->Visible=IniConfig.bP09TrayEndCanSelectTray;
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto5")->Visible=IniConfig.bP09TrayEndCanSelectTray;
    EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto6")->Visible=IniConfig.bP09TrayEndCanSelectTray;

    EL<TRadioButton>("TfTrayAssignment", "rbTemp")->SetFocus();

    EL<TLabel>("TfTrayAssignment", "labFix1")->Visible=(MOT[MManualTray1].TrayFeedHasIC());                       //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TLabel>("TfTrayAssignment", "labFix2")->Visible=(MOT[MManualTray2].TrayFeedHasIC());                       //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TLabel>("TfTrayAssignment", "labFix3")->Visible=(MOT[MManualTray3].TrayFeedHasIC());                       //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TLabel>("TfTrayAssignment", "labFix4")->Visible=(MOT[MManualTray4].TrayFeedHasIC());
    EL<TLabel>("TfTrayAssignment", "labFix5")->Visible=(MOT[MManualTray5].TrayFeedHasIC());
    EL<TLabel>("TfTrayAssignment", "labFix6")->Visible=(MOT[MManualTray6].TrayFeedHasIC());
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix1")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray1].TrayFeedHasIC()==false);  //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix2")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray2].TrayFeedHasIC()==false);  //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix3")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray3].TrayFeedHasIC()==false);  //wei 20160420 Fix盤要沒有IC才能上下盤切換
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix4")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray4].TrayFeedHasIC()==false);
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix5")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray5].TrayFeedHasIC()==false);
    EL<TCheckBox>("TfTrayAssignment", "ckUseFix6")->Visible=(CosFunction.bUseTrayUpDownSet && EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==1 && EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex==1 && MOT[MManualTray6].TrayFeedHasIC()==false);

    { if(!TA_rgLoaderTrayModeItems) EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false; } fShow=true;   // golden cTrayAssignment.cpp:966-966 取代：golden 每顆 RadioButton 的 Enabled（ReadFile 記的旗標）與群組 Enabled 取 AND ＝ 頁面上這組可不可以改
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:966-966 -- golden 每顆 RadioButton 的 Enabled（ReadFile 記的旗標）與群組 Enabled 取 AND ＝ 頁面上這組可不可以改
    fShow=true;
#endif // GATE (S12-C save)
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cTrayAssignment.cpp:993  TfTrayAssignment::ShowCompnet()
static void TA_ShowCompnet()
{

    EL<TGroupBox>("TfTrayAssignment", "gbEmptyTray")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "gbColorTray")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;

    if(IniConfig.bP09TrayEndCanSelectTray==false)                               //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    {
        EL<TGroupBox>("TfTrayAssignment", "gbAuto1")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto2")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto3")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto4")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto5")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto6")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    }
    else
    {
        EL<TGroupBox>("TfTrayAssignment", "gbAuto1")->Visible=true;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto2")->Visible=true;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto3")->Visible=true;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto4")->Visible=true;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto5")->Visible=true;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto6")->Visible=true;
    }

    if(CosFunction.bUseTrayUpDownSet==true)
        EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;

    EL<TGroupBox>("TfTrayAssignment", "grpFix1")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "grpFix2")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "grpFix3")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "grpFix4")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "grpFix5")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TGroupBox>("TfTrayAssignment", "grpFix6")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;

    EL<TPanel>("TfTrayAssignment", "pnlFTTrayFrom")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;
    EL<TPanel>("TfTrayAssignment", "pnlRTTrayFrom")->Visible=EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex;

    if(IniConfig.bUseFix3==false)                                               //2013-07-16    Dell    9046LS第一台沒Fix3;第二台以上Fix3標配
        EL<TGroupBox>("TfTrayAssignment", "grpFix3")->Visible=false;                                                 //2013-01-15    Dell    9046LS 硬體沒有Fix3

    if(EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePageIndex==0 ||                                         //TabSheet1
       EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePageIndex==2)
    {
    EL<TEdit>("TfTrayAssignment", "edAuto1Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1036-1036 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1036-1036 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto1Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto2Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1037-1037 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1037-1037 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto2Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto3Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1038-1038 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1038-1038 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto3Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto4Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1039-1039 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1039-1039 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto4Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto5Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1040-1040 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1040-1040 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto5Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto6Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1041-1041 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1041-1041 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto6Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    }
    else
    {
    EL<TEdit>("TfTrayAssignment", "edAuto1Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1045-1045 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1045-1045 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto1Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto2Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1046-1046 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1046-1046 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto2Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto3Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1047-1047 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1047-1047 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto3Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto4Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1048-1048 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1048-1048 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto4Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto5Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1049-1049 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1049-1049 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto5Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    EL<TEdit>("TfTrayAssignment", "edAuto6Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:EL<TComboBox>("TfTrayAssignment", "cbColor")->Text;   // golden cTrayAssignment.cpp:1050-1050 取代：三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1050-1050 -- 三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代
        EL<TEdit>("TfTrayAssignment", "edAuto6Type")->Text=(EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex==0)?EL<TComboBox>("TfTrayAssignment", "cbEmpty")->Text:cbColor->Text;
#endif // GATE (S12-C save)
    }

    if(IniConfig.bTrayAssignUseGraphic &&                                       //Steven 20111121 : 使用圖片去顯示Tray Assign
       AUTO_EMPTY_COLOR<3)                                                      //Steven 20230915 : HT9016UC暫時不支援圖像模式
    {
        if(EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex==0)
        {
            EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGroup")->TabVisible   =true;
            EL<TTabSheet>("TfTrayAssignment", "tsReTestGroup")->TabVisible       =true;
            EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph")->TabVisible   =false;
            EL<TTabSheet>("TfTrayAssignment", "tsReTestGraph")->TabVisible       =false;
        }
        else
        {
            EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGroup")->TabVisible   =false;
            EL<TTabSheet>("TfTrayAssignment", "tsReTestGroup")->TabVisible       =false;
            EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph")->TabVisible   =true;
            EL<TTabSheet>("TfTrayAssignment", "tsReTestGraph")->TabVisible       =true;
        }
    }
    else
    {
        EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGroup")->TabVisible       =true;
        EL<TTabSheet>("TfTrayAssignment", "tsReTestGroup")->TabVisible           =true;
        EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph")->TabVisible       =false;
        EL<TTabSheet>("TfTrayAssignment", "tsReTestGraph")->TabVisible           =false;
    }

    //在最下面調整畫面--------------------------
    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20230907 : For HT-9011UC
    {
        if(AUTO_EMPTY_COLOR==3)
        {
            EL<TGroupBox>("TfTrayAssignment", "grpICSort6")->Visible=false;
            EL<TGroupBox>("TfTrayAssignment", "gbAuto6")   ->Visible=false;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")   ->Visible=false;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->Visible=false;
        }
        Width=1280;                                                             //RogerYang 20250826 : 修改顯示位置
        if(IniConfig.bA65_BundleIDList)                                         //JerryYang 20250220 : AUTO IN OUT
        {
            Width+=250;
            EL<TPanel>("TfTrayAssignment", "palAMR")->Left=1250;                                                  //RogerYang 20250826 : 修改顯示位置
        }
        else
        {
            EL<TPanel>("TfTrayAssignment", "palAMR")->Left=1262;                                                  //RogerYang 20250826 : 修改顯示位置
        }
    }
    else
    {
        EL<TGroupBox>("TfTrayAssignment", "grpICSort4")->Visible=false;
        EL<TGroupBox>("TfTrayAssignment", "grpICSort5")->Visible=false;
        EL<TGroupBox>("TfTrayAssignment", "grpICSort6")->Visible=false;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto4")->Visible   =false;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto5")->Visible   =false;
        EL<TGroupBox>("TfTrayAssignment", "gbAuto6")->Visible   =false;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->Visible   =false;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->Visible   =false;
        EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->Visible   =false;
        EL<TGroupBox>("TfTrayAssignment", "grpFix4")->Visible   =false;
        EL<TGroupBox>("TfTrayAssignment", "grpFix5")->Visible   =false;
        EL<TGroupBox>("TfTrayAssignment", "grpFix6")->Visible   =false;
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->Visible=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->Visible=false;
        EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->Visible=false;
        Width=861;
        if(IniConfig.bA65_BundleIDList)                                         //JerryYang 20250220 : AUTO IN OUT
        {
            Width+=250;
            EL<TPanel>("TfTrayAssignment", "palAMR")->Left=842;                                                   //RogerYang 20250826 : 修改顯示位置
        }
        else if(IniConfig.bVTESTFunction==true)                                 //RogerYang 20250604 偉測不可複測bin功能
        {
//            if(bVTestNoRTBin==true)                                             //Rogeryang 20250723 改成label
//            {
//                lblVtestNoRTBin->Caption="MES Enable No Retest Bin";
//                lblVtestNoRTBin->Font->Color=clBlue;
//            }
//            else
//            {
//                lblVtestNoRTBin->Caption="MES Disable No Retest Bin";
//                lblVtestNoRTBin->Font->Color=clRed;
//            }
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfTrayAssignment", "edNoRTBinFix1")->Color=(bNoRTBinFixFlag[0]==true)?clYellow:clWindow;  //Rogeryang 20250723 add 顯示顏色
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfTrayAssignment", "edNoRTBinFix2")->Color=(bNoRTBinFixFlag[1]==true)?clYellow:clWindow;
            ;   // UI-only（HTML 端處理）：EL<TEdit>("TfTrayAssignment", "edNoRTBinFix3")->Color=(bNoRTBinFixFlag[2]==true)?clYellow:clWindow;

            EL<TCheckBox>("TfTrayAssignment", "cbSpecTrayCnt")->Visible=false;
            EL<TLabel>("TfTrayAssignment", "Label47")->Visible=false;
            EL<TEdit>("TfTrayAssignment", "edAMRInputTrayCount")->Visible=false;
            EL<TLabel>("TfTrayAssignment", "Label49")->Visible=false;
            EL<TEdit>("TfTrayAssignment", "edAMRFullTrayCount")->Visible=false;

            EL<TGroupBox>("TfTrayAssignment", "grpVTestNoTBin")->Top=5;
            EL<TGroupBox>("TfTrayAssignment", "grpVTestNoTBin")->Visible=true;
            Width+=250;
            EL<TPanel>("TfTrayAssignment", "palAMR")->Left=842;                                                   //RogerYang 20250826 : 修改顯示位置
        }
    }

}

// golden cTrayAssignment.cpp:1210  TfTrayAssignment::ShowTrayDirectIMG()
static void TA_ShowTrayDirectIMG()
{

    ;   // golden MyImage[]={imgLoader, imgAuto1..6, imgFix1..6}   // golden cTrayAssignment.cpp:1212-1213 取代：TImage *MyImage[]＝13 張方向圖（TImage 在 vclcompat 沒有；圖片 HTML 端處理）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1212-1213 -- TImage *MyImage[]＝13 張方向圖（TImage 在 vclcompat 沒有；圖片 HTML 端處理）
    TImage *MyImage[]={EL<TControl>("TfTrayAssignment", "imgLoader"), EL<TControl>("TfTrayAssignment", "imgAuto1"), EL<TControl>("TfTrayAssignment", "imgAuto2"), EL<TControl>("TfTrayAssignment", "imgAuto3"), EL<TControl>("TfTrayAssignment", "imgAuto4"), EL<TControl>("TfTrayAssignment", "imgAuto5"), EL<TControl>("TfTrayAssignment", "imgAuto6"),
                                  EL<TControl>("TfTrayAssignment", "imgFix1"),  EL<TControl>("TfTrayAssignment", "imgFix2"),  EL<TControl>("TfTrayAssignment", "imgFix3"),  EL<TControl>("TfTrayAssignment", "imgFix4"),  EL<TControl>("TfTrayAssignment", "imgFix5"),  EL<TControl>("TfTrayAssignment", "imgFix6")};
#endif // GATE (S12-C save)

    AnsiString Dir;
    for(int i=0; i<13; i++)   // golden cTrayAssignment.cpp:1216-1216 取代：sizeof(MyImage)/4 ＝ 13（BCB6 32 位元指標）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1216-1216 -- sizeof(MyImage)/4 ＝ 13（BCB6 32 位元指標）
    for(int i=0; i<sizeof(MyImage)/4; i++)
#endif // GATE (S12-C save)
    {
        if(iTrayDirect[i]<0 || iTrayDirect[i]>8)
            iTrayDirect[i]=0;
        Dir.sprintf("%stype%d.bmp", BmpPath, iTrayDirect[i]);
        try
        {
            ;   // UI-only（HTML 端處理）：MyImage[i]->Picture->LoadFromFile(Dir);
        }
        catch(...)
        {
            MyDBIProcess("Exception", "ShowTrayDirectIMG");
            filerw::ELMessage("''Path :D:\\HT9045\\IMG\\BMP\\'' be delete");
        }
    }

}

// golden cTrayAssignment.cpp:1644  TfTrayAssignment::GraphicToRadio()
static void TA_GraphicToRadio()
{
    filerw::ELMark("GraphicToRadio");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString str;
    ;   // golden cTrayAssignment.cpp:1647-1648 取代：圖片檔名（純畫面，HTML 端處理）。自己換成空敘述：產生器的 UI-only 改寫會把 ImgNormalTest 留在同一行註解裡，被存檔讀替身的掃描當成「讀」→ mustSend 出現沒有值種類的 TImage，頁面永遠存不了
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1647-1648 -- 圖片檔名（純畫面，HTML 端處理）。自己換成空敘述：產生器的 UI-only 改寫會把 ImgNormalTest 留在同一行註解裡，被存檔讀替身的掃描當成「讀」→ mustSend 出現沒有值種類的 TImage，頁面永遠存不了
    str.sprintf("%sTray%02d.bmp", BmpPath, EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position);
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfTrayAssignment", "ImgNormalTest")->Picture->LoadFromFile(str);
#endif // GATE (S12-C save)

    TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "RGAuto3"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position&0x01); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "RGAuto2"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position&0x02); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "RGAuto1"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position&0x04); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "RGLoader"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position&0x08);   // golden cTrayAssignment.cpp:1650-1653 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1650-1653 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex =EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position&0x01;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex =EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position&0x02;
    EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex =EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position&0x04;
    EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex=EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position&0x08;
#endif // GATE (S12-C save)

}

// golden cTrayAssignment.cpp:1656  TfTrayAssignment::RadioToGraphic()
static void TA_RadioToGraphic()
{

    int index[4]={0, 0, 0, 0}, i, j=0;                                          //QQQ

    index[0]=EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex;
    index[1]=EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex;
    index[2]=EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex;
    index[3]=EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex;

    for(i=0; i<4; i++)
        j+=index[i]<<i;

    EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position=j;   // golden cTrayAssignment.cpp:1668-1668 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1668-1668 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
    EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position=j;
#endif // GATE (S12-C save)
    AnsiString str;
    ;   // golden cTrayAssignment.cpp:1670-1670 取代：圖片檔名（純畫面，HTML 端處理）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1670-1670 -- 圖片檔名（純畫面，HTML 端處理）
    str.sprintf("%sTray%02d.bmp", BmpPath, EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position);
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfTrayAssignment", "ImgNormalTest")->Picture->LoadFromFile(str);

}

// golden cTrayAssignment.cpp:1679  TfTrayAssignment::GraphicToRadio_RT()
static void TA_GraphicToRadio_RT()
{
    filerw::ELMark("GraphicToRadio_RT");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString str;
    ;   // golden cTrayAssignment.cpp:1682-1683 取代：圖片檔名（純畫面，HTML 端處理）（同 :1647-1648，ImgReTest）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1682-1683 -- 圖片檔名（純畫面，HTML 端處理）（同 :1647-1648，ImgReTest）
    str.sprintf("%sTray%02d.bmp", BmpPath, EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position);
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfTrayAssignment", "ImgReTest")->Picture->LoadFromFile(str);
#endif // GATE (S12-C save)

    if(IniConfig.bFTTrayAss2RTTrayAss==true)                                    //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    {
    if(TA_RunModePageIndex()==2)   // golden cTrayAssignment.cpp:1687-1687 取代：pgRunMode->ActivePage==tsNormalTestGraph：vclcompat TPageControl 只有 ActivePageIndex（tsNormalTestGraph＝第 2 頁，DFM 順序）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1687-1687 -- pgRunMode->ActivePage==tsNormalTestGraph：vclcompat TPageControl 只有 ActivePageIndex（tsNormalTestGraph＝第 2 頁，DFM 順序）
        if(EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePage==EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph"))
#endif // GATE (S12-C save)
        {
    EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position=EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position;   // golden cTrayAssignment.cpp:1689-1689 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1689-1689 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
            EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position=EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position;
#endif // GATE (S12-C save)
        }
    }

    TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x01); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x02); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x04); TA_SetRadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT"), EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x08);   // golden cTrayAssignment.cpp:1693-1696 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1693-1696 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex=EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x01;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex=EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x02;
    EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex=EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x04;
    EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex =EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position&0x08;
#endif // GATE (S12-C save)

}

// golden cTrayAssignment.cpp:1699  TfTrayAssignment::RadioToGraphic_RT()
static void TA_RadioToGraphic_RT()
{

    int index[4]={0, 0, 0, 0}, i, j=0;

    index[0]=EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex;
    index[1]=EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex;
    index[2]=EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex;
    index[3]=EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex;

    for(i=0; i<4; i++)
        j+=index[i]<<i;

    EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position=j;   // golden cTrayAssignment.cpp:1711-1711 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1711-1711 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
    EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position=j;
#endif // GATE (S12-C save)
    AnsiString str;
    ;   // golden cTrayAssignment.cpp:1713-1713 取代：圖片檔名（純畫面，HTML 端處理）
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1713-1713 -- 圖片檔名（純畫面，HTML 端處理）
    str.sprintf("%sTray%02d.bmp", BmpPath, EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position);
#endif // GATE (S12-C save)
    ;   // UI-only（HTML 端處理）：EL<TControl>("TfTrayAssignment", "ImgReTest")->Picture->LoadFromFile(str);

}

// golden cTrayAssignment.cpp:1639  TfTrayAssignment::sbNormalTestChange(TObject *Sender)
static void TA_sbNormalTestChange()
{

    TA_GraphicToRadio();

}

// golden cTrayAssignment.cpp:1674  TfTrayAssignment::sbNormalTest_RTChange(TObject *Sender)
static void TA_sbNormalTest_RTChange()
{

    TA_GraphicToRadio_RT();

}

// golden cTrayAssignment.cpp:1325  TfTrayAssignment::FormClose(TObject *Sender, TCloseAction &Action)
static void TA_FormClose()
{

    EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=true;
    TA_ReadFile();
    TA_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
    fShow=false;

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden cTrayAssignment.cpp:1337  TfTrayAssignment::spbSaveClick(TObject *Sender)
static void TA_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden cTrayAssignment.cpp:1343-1343 取代：Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1343-1343 -- Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S="";
    //TComboBox *PtrCombBox[]={cbEmpty, cbColor};

    S=GetLastOpenFN();
    AnsiString szDir="";

    if(IniConfig.bFTBin2RTBin==true)                                            //Steven 20120627 : 當FT存檔時,把RT設定跟FT一樣
    {
    filerw::ELTodo("golden cTrayAssignment.cpp:1355 bFTBin2RTBin copies FT->RT only when an FT tab is active; the page does not send the pgRunMode tab -- server keeps the DFM tab tsReTestGroup => no copy, RT saved as sent"); if(TA_RunModePageIndex()==0 || TA_RunModePageIndex()==2)   // golden cTrayAssignment.cpp:1355-1356 取代：pgRunMode->ActivePage==tsNormalTestGroup||tsNormalTestGraph：頁面不送目前分頁，伺服器保留 DFM 的 tsReTestGroup（第 1 頁）⇒ 不複製 FT→RT
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1355-1356 -- pgRunMode->ActivePage==tsNormalTestGroup||tsNormalTestGraph：頁面不送目前分頁，伺服器保留 DFM 的 tsReTestGroup（第 1 頁）⇒ 不複製 FT→RT
        if(EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePage==EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGroup") ||
           EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePage==EL<TTabSheet>("TfTrayAssignment", "tsNormalTestGraph"))
#endif // GATE (S12-C save)
        {
            if(IniConfig.bTrayAssignUseGraphic &&                               //Steven 20111121 : 使用圖片去顯示Tray Assign
               AUTO_EMPTY_COLOR<3)                                              //Steven 20230915 : HT9016UC暫時不支援圖像模式
            {
    EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT")->Position=EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest")->Position;   // golden cTrayAssignment.cpp:1361-1361 取代：圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1361-1361 -- 圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值
                EL<TControl>("TfTrayAssignment", "sbNormalTest_RT")->Position=EL<TControl>("TfTrayAssignment", "sbNormalTest")->Position;
#endif // GATE (S12-C save)
            }
            else
            {
                EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex =EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex;
                EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex=EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex;
            }
        }
    }

    if(IniConfig.bTrayAssignUseGraphic &&                                       //Steven 20111121 : 使用圖片去顯示Tray Assign
       AUTO_EMPTY_COLOR<3)                                                      //Steven 20230915 : HT9016UC暫時不支援圖像模式
    {
        TA_GraphicToRadio();
        TA_GraphicToRadio_RT();
    }

    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
        EventReport(SECS_EVENT.SaveRecipe);
    }

    if(CosFunction.bOLPFunction)                                                //Sam 20230921 : Bin 設定錯誤不能啟動
    {
        LastSet.OLPSetBinErr[2]=0;
    }

    szDir.sprintf("%s%s", DataPath, S);
    TA_SaveSetupFile(szDir, S);
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(S, "JOBFILE");                                 //kevin 20150105  Start 另存 jobfile
    #endif
    if(TrayForm.bEnableAMR) //Eastsun 20260710 Merge
        fLotInfo->tsKYEC_AMR->TabVisible=true;
    else
        fLotInfo->tsKYEC_AMR->TabVisible=false;

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

// golden cTrayAssignment.cpp:1409  TfTrayAssignment::SaveSetupFile(AnsiString szDir, AnsiString S)
static void TA_SaveSetupFile(AnsiString szDir, AnsiString S)
{
    filerw::ELMark("SaveSetupFile");   // FileRW：存檔流程 trace（ack.trace）

    int iLinkSum=0;
    int ibuffer=0;
    AnsiString Str1, Str2;
    TComboBox *PtrCombBox[]={EL<TComboBox>("TfTrayAssignment", "cbEmpty"), EL<TComboBox>("TfTrayAssignment", "cbColor")};
    MyForceDirectories(szDir);
    szDir+="\\Tray.Data";
    AnsiString szDir2="D:\\HT9045\\config\\config.ini";

    if(IniConfig.bP46_LoadTrayModeByHandler)                                    //Steven 20221117 : Loader Tray Mode設定跟著機台變
    {
        WriteIniData(szDir2, "Flag", "Skip Manual Remove Tray",  EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->ItemIndex);
    }
    else
    {
        WriteIniData(szDir, "Flag", "Skip Manual Remove Tray",  EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->ItemIndex);                           //Steven 20101019          //Steven 20130819 : 加入開門檢查的功能
    }

    WriteIniData(szDir, "Rotation", "Input",   EL<TComboBox>("TfTrayAssignment", "cbInputRotateKIT")->ItemIndex);
    WriteIniData(szDir, "Rotation", "Output",  EL<TComboBox>("TfTrayAssignment", "cbOutputRotateKIT")->ItemIndex);

    if(CosFunction.bUnloadTrayModeByRecipe==false)                              //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    {
        WriteIniData(szDir, "Flag", "Auto Feed",                EL<TCheckBox>("TfTrayAssignment", "chkAutoTrayFeed")->Checked);
        WriteIniData(szDir, "Flag", "bFailAutoTrayManual_FT",   EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_FT")->Checked);                    //Steven 20150116 : 手動移除Auto Fail Bin Tray
        WriteIniData(szDir, "Flag", "bFailAutoTrayManual_RT",   EL<TCheckBox>("TfTrayAssignment", "cbManuTakeAutoFailTray_RT")->Checked);                    //Steven 20150116 : 手動移除Auto Fail Bin Tray
    }
    WriteIniData(szDir, "Flag", "Color Tray Sensor",        EL<TCheckBox>("TfTrayAssignment", "chkColorTray")->Checked);                                     //20140903 wei colcr Tray
    WriteIniData(szDir, "Flag", "Check loader tray direction", EL<TCheckBox>("TfTrayAssignment", "chkCheckLoadDirection")->Checked);                         //Steven 20190815 : JCET不重測Tray偵測
    WriteIniData(szDir, "Flag", "Loader Type",              EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType")->ItemIndex);
    WriteIniData(szDir, "Flag", "MoveAfterTrayGoOut",        EL<TCheckBox>("TfTrayAssignment", "cbMoveAfterTrayGoOut")->Checked);                            //JerryYang 20241002 : SPIL要求功能 Trray arm等AUTO TRAY退到外面才能移動

    WriteIniData(szDir, "Flag", "bSpecTrayCnt",        EL<TCheckBox>("TfTrayAssignment", "cbSpecTrayCnt")->Checked);                                         //JerryYang 20250212 : add

    WriteIniData(szDir, "VTest", "NoRTBin",       EL<TCheckBox>("TfTrayAssignment", "cbVTestNoRTBin")->Checked);     //RogerYang 20250814 : 偉測不可複測bin功能
    WriteIniData(szDir, "VTest", "NoRTBinFix1",   EL<TEdit>("TfTrayAssignment", "edNoRTBinFix1")->Text);
    WriteIniData(szDir, "VTest", "NoRTBinFix2",   EL<TEdit>("TfTrayAssignment", "edNoRTBinFix2")->Text);
    WriteIniData(szDir, "VTest", "NoRTBinFix3",   EL<TEdit>("TfTrayAssignment", "edNoRTBinFix3")->Text);

    if(CosFunction.bUseTrayUpDownSet==false)
    {
        WriteIniData(szDir, "Flag", "Fix Tray Mode",    0);
    }
    else
    {
        WriteIniData(szDir, "Flag", "Fix Tray Mode",    EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode")->ItemIndex);
    }

    if(CosFunction.bUseTrayUpDownSet)                                           //wei 20160224 TSMC FIX UPDOWN
    {
        WriteIniData(szDir, "Flag", "Use Fix1 Tray",    EL<TCheckBox>("TfTrayAssignment", "ckUseFix1")->Checked);
        WriteIniData(szDir, "Flag", "Use Fix2 Tray",    EL<TCheckBox>("TfTrayAssignment", "ckUseFix2")->Checked);
        WriteIniData(szDir, "Flag", "Use Fix3 Tray",    EL<TCheckBox>("TfTrayAssignment", "ckUseFix3")->Checked);
        if(AUTO_EMPTY_COLOR>=3)
        {
            WriteIniData(szDir, "Flag", "Use Fix4 Tray", EL<TCheckBox>("TfTrayAssignment", "ckUseFix4")->Checked);
            WriteIniData(szDir, "Flag", "Use Fix5 Tray", EL<TCheckBox>("TfTrayAssignment", "ckUseFix5")->Checked);
            WriteIniData(szDir, "Flag", "Use Fix6 Tray", EL<TCheckBox>("TfTrayAssignment", "ckUseFix6")->Checked);
        }
    }
    WriteIniData(szDir, "Flag", "Fix Tray Link",    iLinkSum);

    WriteIniData(szDir, "Loader", "Tray Type",      EL<TComboBox>("TfTrayAssignment", "cbLoader")->ItemIndex);
    WriteIniData(szDir, "Loader", "Direction",      iTrayDirect[0]);
    WriteIniData(szDir, "Loader", "ToBuffer",       EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex);
    WriteIniData(szDir, "Loader", "ToBuffer_RT",    EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex);

    WriteIniData(szDir, "Empty", "Tray Type",       EL<TComboBox>("TfTrayAssignment", "cbEmpty")->ItemIndex);
    WriteIniData(szDir, "Color", "Tray Type",       EL<TComboBox>("TfTrayAssignment", "cbColor")->ItemIndex);

    WriteIniData(szDir, "AMR", "Full Tray Count",         EL<TEdit>("TfTrayAssignment", "edAMRFullTrayCount")->Text);                                    //JerryYang 20250220 : AUTO IN OUT
    WriteIniData(szDir, "AMR", "Input Tray Count",        EL<TEdit>("TfTrayAssignment", "edAMRInputTrayCount")->Text);
    //==> Eastsun 20260512 F009 整合: AMR WriteIni (D1.1.2)
    WriteIniData(szDir, "AMR", "Enahle AMR",              EL<TCheckBox>("TfTrayAssignment", "cbEnableAMR_KYEC")->Checked);
    WriteIniData(szDir, "AMR", "Enahle AMR Loader",       EL<TCheckBox>("TfTrayAssignment", "cbEnableAMRLoader_KYEC")->Checked);
    WriteIniData(szDir, "AMR", "Reader Pos",              EL<TRadioGroup>("TfTrayAssignment", "rgAMRReaderPos_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "1st Tray Type",           EL<TComboBox>("TfTrayAssignment", "cbAMRTray1st_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "2nd Tray Type",           EL<TComboBox>("TfTrayAssignment", "cbAMRTray2nd_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "3rd Tray Type",           EL<TComboBox>("TfTrayAssignment", "cbAMRTray3rd_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "1st ID Use",              EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID1st_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "2nd ID Use",              EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID2nd_KYEC")->ItemIndex);
    WriteIniData(szDir, "AMR", "3rd ID Use",              EL<TComboBox>("TfTrayAssignment", "cbAMRTrayID3rd_KYEC")->ItemIndex);
    //<== Eastsun 20260512 F009 整合

    if(IniConfig.bP36BufferTrayNoSame &&
       (EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex==0 ||                                               //empty tray
        EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex==0))                                               //kevin 20171117 (wei) Load && Unload 強制不能使用相同軌道
    {
        if(EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex==0)                                              //color tray
        {
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex=1;
        }

        if(EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex==0)
        {
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex=1;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex=1;
        }
    }

    if(IniConfig.bP36BufferTrayNoSame &&
       (EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex==1 ||                                               //color tray
        EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex==1))                                               //kevin 20171117 (wei) Load && Unload 強制不能使用相同軌道
    {
        if(EL<TRadioGroup>("TfTrayAssignment", "RGLoader")->ItemIndex==1)
        {
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex=0;                                               //empty tray
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex=0;
        }

        if(EL<TRadioGroup>("TfTrayAssignment", "rgLoad_RT")->ItemIndex==1)
        {
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex=0;
            EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex=0;
        }
    }

    { const int k_=EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:1544 PtrCombBox[RGAuto1->ItemIndex] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto1], "Tray Type", PtrCombBox[k_]->ItemIndex); }   // golden cTrayAssignment.cpp:1544-1544 取代：PtrCombBox[RGAuto1->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1544-1544 -- PtrCombBox[RGAuto1->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
    WriteIniData(szDir, s6TrayName[eAuto1], "Tray Type",       PtrCombBox[EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex]->ItemIndex);
#endif // GATE (S12-C save)
    WriteIniData(szDir, s6TrayName[eAuto1], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto1")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto1], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto1_RT")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto1], "Direction",       iTrayDirect[eAuto1+1]);
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        WriteIniData(szDir, s6TrayName[eAuto1], "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto1")->Checked);

    if(EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex>=2)                                                   //kevin 20120912
        ibuffer=EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex;
    else
        ibuffer=EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex;
    if(ibuffer<0 || ibuffer>1) filerw::ELTodo("golden cTrayAssignment.cpp:1555 PtrCombBox[ibuffer] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto2], "Tray Type", PtrCombBox[ibuffer]->ItemIndex);   // golden cTrayAssignment.cpp:1555-1555 取代：PtrCombBox[ibuffer] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1555-1555 -- PtrCombBox[ibuffer] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
    WriteIniData(szDir, s6TrayName[eAuto2], "Tray Type",       PtrCombBox[ibuffer]->ItemIndex);                         //kevin 20120912
#endif // GATE (S12-C save)
    WriteIniData(szDir, s6TrayName[eAuto2], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto2")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto2], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto2_RT")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto2], "Direction",       iTrayDirect[eAuto2+1]);
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        WriteIniData(szDir, s6TrayName[eAuto2], "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto2")->Checked);

    { const int k_=EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:1562 PtrCombBox[RGAuto3->ItemIndex] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto3], "Tray Type", PtrCombBox[k_]->ItemIndex); }   // golden cTrayAssignment.cpp:1562-1562 取代：PtrCombBox[RGAuto3->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1562-1562 -- PtrCombBox[RGAuto3->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
    WriteIniData(szDir, s6TrayName[eAuto3], "Tray Type",       PtrCombBox[EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex]->ItemIndex);
#endif // GATE (S12-C save)
    WriteIniData(szDir, s6TrayName[eAuto3], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto3")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto3], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto3_RT")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eAuto3], "Direction",       iTrayDirect[eAuto3+1]);
    if(IniConfig.bP09TrayEndCanSelectTray)                                      //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        WriteIniData(szDir, s6TrayName[eAuto3], "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto3")->Checked);

    WriteIniData(szDir, s6TrayName[eFix1], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix1")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eFix1], "Direction",        iTrayDirect[eFix1+1]);

    WriteIniData(szDir, s6TrayName[eFix2], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix2")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eFix2], "Direction",        iTrayDirect[eFix2+1]);

    WriteIniData(szDir, s6TrayName[eFix3], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix3")->ItemIndex);
    WriteIniData(szDir, s6TrayName[eFix3], "Direction",        iTrayDirect[eFix3+1]);

    if(AUTO_EMPTY_COLOR>=3)
    {
    { const int k_=EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:1580 PtrCombBox[RGAuto4->ItemIndex] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto4], "Tray Type", PtrCombBox[k_]->ItemIndex); }   // golden cTrayAssignment.cpp:1580-1580 取代：PtrCombBox[RGAuto4->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1580-1580 -- PtrCombBox[RGAuto4->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
        WriteIniData(szDir, s6TrayName[eAuto4], "Tray Type",       PtrCombBox[EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex]->ItemIndex);
#endif // GATE (S12-C save)
        WriteIniData(szDir, s6TrayName[eAuto4], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto4")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto4], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto4_RT")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto4], "Direction",       iTrayDirect[eAuto4+1]);
        if(IniConfig.bP09TrayEndCanSelectTray)                                  //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
            WriteIniData(szDir, eAuto4, "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto4")->Checked);

    { const int k_=EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:1587 PtrCombBox[RGAuto5->ItemIndex] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto5], "Tray Type", PtrCombBox[k_]->ItemIndex); }   // golden cTrayAssignment.cpp:1587-1587 取代：PtrCombBox[RGAuto5->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1587-1587 -- PtrCombBox[RGAuto5->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
        WriteIniData(szDir, s6TrayName[eAuto5], "Tray Type",       PtrCombBox[EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex]->ItemIndex);
#endif // GATE (S12-C save)
        WriteIniData(szDir, s6TrayName[eAuto5], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto5")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto5], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto5_RT")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto5], "Direction",       iTrayDirect[eAuto5+1]);
        if(IniConfig.bP09TrayEndCanSelectTray)                                  //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
            WriteIniData(szDir, eAuto5, "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto5")->Checked);

    { const int k_=EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:1594 PtrCombBox[RGAuto6->ItemIndex] out of range (golden UB) -- key not written"); else WriteIniData(szDir, s6TrayName[eAuto6], "Tray Type", PtrCombBox[k_]->ItemIndex); }   // golden cTrayAssignment.cpp:1594-1594 取代：PtrCombBox[RGAuto6->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
#if 0 // GATE (S12-C save) golden cTrayAssignment.cpp:1594-1594 -- PtrCombBox[RGAuto6->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo
        WriteIniData(szDir, s6TrayName[eAuto6], "Tray Type",       PtrCombBox[EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex]->ItemIndex);
#endif // GATE (S12-C save)
        WriteIniData(szDir, s6TrayName[eAuto6], "FromBuffer",      EL<TRadioGroup>("TfTrayAssignment", "RGAuto6")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto6], "FromBuffer_RT",   EL<TRadioGroup>("TfTrayAssignment", "rgAuto6_RT")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eAuto6], "Direction",       iTrayDirect[eAuto6+1]);
        if(IniConfig.bP09TrayEndCanSelectTray)                                  //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
            WriteIniData(szDir, eAuto6, "TrayEndRecv", EL<TCheckBox>("TfTrayAssignment", "chkTrayEndRecvAuto6")->Checked);

        WriteIniData(szDir, s6TrayName[eFix4], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix4")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eFix4], "Direction",        iTrayDirect[eFix4+1]);

        WriteIniData(szDir, s6TrayName[eFix5], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix5")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eFix5], "Direction",        iTrayDirect[eFix5+1]);

        WriteIniData(szDir, s6TrayName[eFix6], "Tray Type",        EL<TComboBox>("TfTrayAssignment", "cbFix6")->ItemIndex);
        WriteIniData(szDir, s6TrayName[eFix6], "Direction",        iTrayDirect[eFix6+1]);
    }

    if(CosFunction.bTraySortCntFunc)                                            //Steven 20210608 : Tray指定數量退盤功能
    {
        for(int i=0; i<=iFixRight; i++)
        {
            if(Prod.iTrayType[i]==tTrayAuto)
            {
                WriteIniData(szDir, s6TrayName[i],  "bTraySortCntFunc",  chkICSort[i]->Checked);

                for(int j=0; j<6; j++)
                {
                    Str2.sprintf("TraySortCnt%d_%d", i+1, j+1);
                    WriteIniData(szDir, s6TrayName[i], Str2, edtICSort[i][j]->Text);
                }
            }
        }
    }

    AddSpace(szDir);
    TA_ReadFile();
    EL<TSpeedButton>("TfTrayAssignment", "spbSave")->Down=false;

}

