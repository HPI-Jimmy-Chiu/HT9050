// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 AOAOffset 的讀寫檔（HTEditList： → D:\HT9045\system\Gerneral.ini [System] AOA_InArm_*／AOA_OutArm_* _X/_Y（寫：OffsetSaveClick，WriteIniDataGeneral 26 鍵，AUTO_EMPTY_COLOR>=3 時 38 鍵；讀：移植樹 database.cpp:1576 ReadGeneralIni，不在本檔））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden main.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 0 個（header 宣告 886 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"


// golden main.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static void AOA_FormShowAOA()   // golden TfMain::FormShow main.cpp:11681-11726 ＋ :11738（逐字；fMain->Ed_* 與 pnlAOAForHT9011 → 具名替身）
{
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&
       TestIF_File.bEnableAutoAlignment==true)                                  //KenHsieh 20210918 : CCD AUTO ALIGNMENT offset新增
    {
        filerw::EL<TEdit>("TfMain", "Ed_LoaderOffset_X")->Text  =iAOA_InArm_Loader_X;
        filerw::EL<TEdit>("TfMain", "Ed_LoaderOffset_Y")->Text  =iAOA_InArm_Loader_Y;
        filerw::EL<TEdit>("TfMain", "Ed_InSH1Offset_X")->Text   =iAOA_InArm_Shuttle1_X;
        filerw::EL<TEdit>("TfMain", "Ed_InSH1Offset_Y")->Text   =iAOA_InArm_Shuttle1_Y;
        filerw::EL<TEdit>("TfMain", "Ed_InSH2Offset_X")->Text   =iAOA_InArm_Shuttle2_X;
        filerw::EL<TEdit>("TfMain", "Ed_InSH2Offset_Y")->Text   =iAOA_InArm_Shuttle2_Y;
        filerw::EL<TEdit>("TfMain", "Ed_HP1Offset_X")->Text     =iAOA_InArm_Hotplate1_X;
        filerw::EL<TEdit>("TfMain", "Ed_HP1Offset_Y")->Text     =iAOA_InArm_Hotplate1_Y;
        filerw::EL<TEdit>("TfMain", "Ed_HP2Offset_X")->Text     =iAOA_InArm_Hotplate2_X;
        filerw::EL<TEdit>("TfMain", "Ed_HP2Offset_Y")->Text     =iAOA_InArm_Hotplate2_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Auto1Offset_X")->Text   =iAOA_OutArm_Auto1_X;
        filerw::EL<TEdit>("TfMain", "Ed_Auto1Offset_Y")->Text   =iAOA_OutArm_Auto1_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Auto2Offset_X")->Text   =iAOA_OutArm_Auto2_X;
        filerw::EL<TEdit>("TfMain", "Ed_Auto2Offset_Y")->Text   =iAOA_OutArm_Auto2_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Auto3Offset_X")->Text   =iAOA_OutArm_Auto3_X;
        filerw::EL<TEdit>("TfMain", "Ed_Auto3Offset_Y")->Text   =iAOA_OutArm_Auto3_Y;

        filerw::EL<TEdit>("TfMain", "Ed_Auto4Offset_X")->Text   =iAOA_OutArm_Auto4_X;                   //Steven 20240428 : Add for HT9011 AOA
        filerw::EL<TEdit>("TfMain", "Ed_Auto4Offset_Y")->Text   =iAOA_OutArm_Auto4_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Auto5Offset_X")->Text   =iAOA_OutArm_Auto5_X;
        filerw::EL<TEdit>("TfMain", "Ed_Auto5Offset_Y")->Text   =iAOA_OutArm_Auto5_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Auto6Offset_X")->Text   =iAOA_OutArm_Auto6_X;
        filerw::EL<TEdit>("TfMain", "Ed_Auto6Offset_Y")->Text   =iAOA_OutArm_Auto6_Y;

        filerw::EL<TEdit>("TfMain", "Ed_Fix1Offset_X")->Text    =iAOA_OutArm_Fix1_X;
        filerw::EL<TEdit>("TfMain", "Ed_Fix1Offset_Y")->Text    =iAOA_OutArm_Fix1_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Fix2Offset_X")->Text    =iAOA_OutArm_Fix2_X;
        filerw::EL<TEdit>("TfMain", "Ed_Fix2Offset_Y")->Text    =iAOA_OutArm_Fix2_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Fix3Offset_X")->Text    =iAOA_OutArm_Fix3_X;
        filerw::EL<TEdit>("TfMain", "Ed_Fix3Offset_Y")->Text    =iAOA_OutArm_Fix3_Y;

        filerw::EL<TEdit>("TfMain", "Ed_Fix4Offset_X")->Text    =iAOA_OutArm_Fix4_X;                    //Steven 20240428 : Add for HT9011 AOA
        filerw::EL<TEdit>("TfMain", "Ed_Fix4Offset_Y")->Text    =iAOA_OutArm_Fix4_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Fix5Offset_X")->Text    =iAOA_OutArm_Fix5_X;
        filerw::EL<TEdit>("TfMain", "Ed_Fix5Offset_Y")->Text    =iAOA_OutArm_Fix5_Y;
        filerw::EL<TEdit>("TfMain", "Ed_Fix6Offset_X")->Text    =iAOA_OutArm_Fix6_X;
        filerw::EL<TEdit>("TfMain", "Ed_Fix6Offset_Y")->Text    =iAOA_OutArm_Fix6_Y;

        filerw::EL<TEdit>("TfMain", "Ed_OutSH1Offset_X")->Text  =iAOA_OutArm_Shuttle1_X;
        filerw::EL<TEdit>("TfMain", "Ed_OutSH1Offset_Y")->Text  =iAOA_OutArm_Shuttle1_Y;
        filerw::EL<TEdit>("TfMain", "Ed_OutSH2Offset_X")->Text  =iAOA_OutArm_Shuttle2_X;
        filerw::EL<TEdit>("TfMain", "Ed_OutSH2Offset_Y")->Text  =iAOA_OutArm_Shuttle2_Y;
    }
    // golden :11727-11737（RENESAS_Server 瑞薩 FT-CT，與 AOA 無關）不在這裡
    filerw::EL<TPanel>("TfMain", "pnlAOAForHT9011")->Visible=(AUTO_EMPTY_COLOR>=3);                             //Steven 20240428 : Add for HT9011 AOA   // golden main.cpp:11738
}

using filerw::EL;

// 前置宣告
static void AOA_OffsetSaveClick();

// golden main.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void AOA_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfMain", "cbRunStartMode")->Items;
      s->Add("");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbSimuBinSite0")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbSimuBinSite1")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbSimuBinSite2")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbSimuBinSite3")->Items;
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbUnloaderinfo")->Items;
      s->Add("Auto1");
      s->Add("Auto2");
      s->Add("Auto3");
      s->Add("Auto4");
      s->Add("Auto5");
      s->Add("Auto6");
      s->Add("Fix1");
      s->Add("Fix2");
      s->Add("Fix3");
      s->Add("Fix4");
      s->Add("Fix5");
      s->Add("Fix6");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbUserSelect")->Items;
      s->Add("Operator");
      s->Add("Engineer");
      s->Add("Supervisor");
      s->Add("HonPrec");
    }
    { TStringList* s = EL<TComboBox>("TfMain", "cbbRunModeSel")->Items;
      s->Add("Continuous Start");
      s->Add("Initial Start");
    }
    { TStringList* s = EL<TRadioGroup>("TfMain", "rgHP")->Items;
      s->Add("HP 0");
      s->Add("HP 1");
    }
    { TStringList* s = EL<TRadioGroup>("TfMain", "rgHotplateShowMessage")->Items;
      s->Add("[1] Hot Time");
      s->Add("[2] Which Shuttle ");
      s->Add("[3] Which Kit");
      s->Add("[4] Count ");
      s->Add("[5] Which Row");
      s->Add("[6] Which Site");
      s->Add("[7] Table HP");
    }
    { TStringList* s = EL<TRadioGroup>("TfMain", "rgUnloaderInfo")->Items;
      s->Add("[1] Bin");
      s->Add("[2] Which Index ");
      s->Add("[3] Which Site");
    }
}

// golden main.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kAOA_ParentOf[][2] = {
    {"Ed_Auto1Offset_X", "Panel14"},
    {"Ed_Auto1Offset_Y", "Panel14"},
    {"Ed_Auto2Offset_X", "Panel14"},
    {"Ed_Auto2Offset_Y", "Panel14"},
    {"Ed_Auto3Offset_X", "Panel14"},
    {"Ed_Auto3Offset_Y", "Panel14"},
    {"Ed_Auto4Offset_X", "pnlAOAForHT9011"},
    {"Ed_Auto4Offset_Y", "pnlAOAForHT9011"},
    {"Ed_Auto5Offset_X", "pnlAOAForHT9011"},
    {"Ed_Auto5Offset_Y", "pnlAOAForHT9011"},
    {"Ed_Auto6Offset_X", "pnlAOAForHT9011"},
    {"Ed_Auto6Offset_Y", "pnlAOAForHT9011"},
    {"Ed_Fix1Offset_X", "Panel14"},
    {"Ed_Fix1Offset_Y", "Panel14"},
    {"Ed_Fix2Offset_X", "Panel14"},
    {"Ed_Fix2Offset_Y", "Panel14"},
    {"Ed_Fix3Offset_X", "Panel14"},
    {"Ed_Fix3Offset_Y", "Panel14"},
    {"Ed_Fix4Offset_X", "pnlAOAForHT9011"},
    {"Ed_Fix4Offset_Y", "pnlAOAForHT9011"},
    {"Ed_Fix5Offset_X", "pnlAOAForHT9011"},
    {"Ed_Fix5Offset_Y", "pnlAOAForHT9011"},
    {"Ed_Fix6Offset_X", "pnlAOAForHT9011"},
    {"Ed_Fix6Offset_Y", "pnlAOAForHT9011"},
    {"Ed_HP1Offset_X", "Panel14"},
    {"Ed_HP1Offset_Y", "Panel14"},
    {"Ed_HP2Offset_X", "Panel14"},
    {"Ed_HP2Offset_Y", "Panel14"},
    {"Ed_InSH1Offset_X", "Panel14"},
    {"Ed_InSH1Offset_Y", "Panel14"},
    {"Ed_InSH2Offset_X", "Panel14"},
    {"Ed_InSH2Offset_Y", "Panel14"},
    {"Ed_LoaderOffset_X", "Panel14"},
    {"Ed_LoaderOffset_Y", "Panel14"},
    {"Ed_OutSH1Offset_X", "Panel14"},
    {"Ed_OutSH1Offset_Y", "Panel14"},
    {"Ed_OutSH2Offset_X", "Panel14"},
    {"Ed_OutSH2Offset_Y", "Panel14"},
    {"Panel14", "ts1"},
    {"pgMotionView", "pnlBackGround"},
    {"pnlAOAForHT9011", "Panel14"},
    {"pnlBackGround", "tsMotionView"},
    {"ts1", "pgMotionView"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void AOA_CreateContainerProxies()
{
    EL<TPanel>("TfMain", "Panel14");
    EL<TPageControl>("TfMain", "pgMotionView");
    EL<TPanel>("TfMain", "pnlAOAForHT9011");
    EL<TPanel>("TfMain", "pnlBackGround");
    EL<TTabSheet>("TfMain", "ts1");
    EL<TTabSheet>("TfMain", "tsMotionView");
    filerw::ELSetParents("TfMain", kAOA_ParentOf, (int)(sizeof(kAOA_ParentOf) / sizeof(kAOA_ParentOf[0])));
}

// golden main.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void AOA_DfmState()
{
    EL<TEdit>("TfMain", "Ed_Auto1Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto1Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto2Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto2Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto3Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto3Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto4Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto4Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto5Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto5Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto6Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto6Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix1Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix1Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix2Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix2Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix3Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix3Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix4Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix4Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix5Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix5Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix6Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Fix6Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_HP1Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_HP1Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_HP2Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_HP2Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_InSH1Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_InSH1Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_InSH2Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_InSH2Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_LoaderOffset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_LoaderOffset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_X")->Visible = false;
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_Y")->Visible = false;
    EL<TEdit>("TfMain", "Ed_Auto1Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto1Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto2Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto2Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto3Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto3Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto4Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto4Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto5Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto5Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto6Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Auto6Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix1Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix1Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix2Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix2Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix3Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix3Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix4Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix4Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix5Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix5Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix6Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_Fix6Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_HP1Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_HP1Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_HP2Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_HP2Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_InSH1Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_InSH1Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_InSH2Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_InSH2Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_LoaderOffset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_LoaderOffset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_Y")->Text = "0";
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_X")->Text = "0";
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_Y")->Text = "0";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kAOA_EnableAll[] = {
    "Panel14", "pgMotionView", "pnlAOAForHT9011", "pnlBackGround", "ts1", "tsMotionView",
};

// 存檔流程（OffsetSaveClick）讀的替身 38 個 —— FileRW/AOAOffset.cpp 用它決定頁面必須送哪些值
static const char* const kAOA_SaveReads[] = {
    "Ed_Auto1Offset_X", "Ed_Auto1Offset_Y", "Ed_Auto2Offset_X", "Ed_Auto2Offset_Y", "Ed_Auto3Offset_X", "Ed_Auto3Offset_Y",
    "Ed_Auto4Offset_X", "Ed_Auto4Offset_Y", "Ed_Auto5Offset_X", "Ed_Auto5Offset_Y", "Ed_Auto6Offset_X", "Ed_Auto6Offset_Y",
    "Ed_Fix1Offset_X", "Ed_Fix1Offset_Y", "Ed_Fix2Offset_X", "Ed_Fix2Offset_Y", "Ed_Fix3Offset_X", "Ed_Fix3Offset_Y",
    "Ed_Fix4Offset_X", "Ed_Fix4Offset_Y", "Ed_Fix5Offset_X", "Ed_Fix5Offset_Y", "Ed_Fix6Offset_X", "Ed_Fix6Offset_Y",
    "Ed_HP1Offset_X", "Ed_HP1Offset_Y", "Ed_HP2Offset_X", "Ed_HP2Offset_Y", "Ed_InSH1Offset_X", "Ed_InSH1Offset_Y",
    "Ed_InSH2Offset_X", "Ed_InSH2Offset_Y", "Ed_LoaderOffset_X", "Ed_LoaderOffset_Y", "Ed_OutSH1Offset_X", "Ed_OutSH1Offset_Y",
    "Ed_OutSH2Offset_X", "Ed_OutSH2Offset_Y",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void AOA_CreateSaveProxies()
{
    EL<TEdit>("TfMain", "Ed_Auto1Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto1Offset_Y");
    EL<TEdit>("TfMain", "Ed_Auto2Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto2Offset_Y");
    EL<TEdit>("TfMain", "Ed_Auto3Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto3Offset_Y");
    EL<TEdit>("TfMain", "Ed_Auto4Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto4Offset_Y");
    EL<TEdit>("TfMain", "Ed_Auto5Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto5Offset_Y");
    EL<TEdit>("TfMain", "Ed_Auto6Offset_X");
    EL<TEdit>("TfMain", "Ed_Auto6Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix1Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix1Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix2Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix2Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix3Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix3Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix4Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix4Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix5Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix5Offset_Y");
    EL<TEdit>("TfMain", "Ed_Fix6Offset_X");
    EL<TEdit>("TfMain", "Ed_Fix6Offset_Y");
    EL<TEdit>("TfMain", "Ed_HP1Offset_X");
    EL<TEdit>("TfMain", "Ed_HP1Offset_Y");
    EL<TEdit>("TfMain", "Ed_HP2Offset_X");
    EL<TEdit>("TfMain", "Ed_HP2Offset_Y");
    EL<TEdit>("TfMain", "Ed_InSH1Offset_X");
    EL<TEdit>("TfMain", "Ed_InSH1Offset_Y");
    EL<TEdit>("TfMain", "Ed_InSH2Offset_X");
    EL<TEdit>("TfMain", "Ed_InSH2Offset_Y");
    EL<TEdit>("TfMain", "Ed_LoaderOffset_X");
    EL<TEdit>("TfMain", "Ed_LoaderOffset_Y");
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_X");
    EL<TEdit>("TfMain", "Ed_OutSH1Offset_Y");
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_X");
    EL<TEdit>("TfMain", "Ed_OutSH2Offset_Y");
}

// golden main.cpp:34988  TfMain::OffsetSaveClick(TObject *Sender)
static void AOA_OffsetSaveClick()
{
    filerw::ELMark("OffsetSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    iAOA_InArm_Loader_X     =atoi(EL<TEdit>("TfMain", "Ed_LoaderOffset_X")->Text.c_str());   // golden main.cpp:34990-34990 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34990-34990 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Loader_X     =atoi(fMain->Ed_LoaderOffset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Loader_Y     =atoi(EL<TEdit>("TfMain", "Ed_LoaderOffset_Y")->Text.c_str());   // golden main.cpp:34991-34991 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34991-34991 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Loader_Y     =atoi(fMain->Ed_LoaderOffset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Shuttle1_X   =atoi(EL<TEdit>("TfMain", "Ed_InSH1Offset_X")->Text.c_str());   // golden main.cpp:34992-34992 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34992-34992 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Shuttle1_X   =atoi(fMain->Ed_InSH1Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Shuttle1_Y   =atoi(EL<TEdit>("TfMain", "Ed_InSH1Offset_Y")->Text.c_str());   // golden main.cpp:34993-34993 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34993-34993 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Shuttle1_Y   =atoi(fMain->Ed_InSH1Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Shuttle2_X   =atoi(EL<TEdit>("TfMain", "Ed_InSH2Offset_X")->Text.c_str());   // golden main.cpp:34994-34994 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34994-34994 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Shuttle2_X   =atoi(fMain->Ed_InSH2Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Shuttle2_Y   =atoi(EL<TEdit>("TfMain", "Ed_InSH2Offset_Y")->Text.c_str());   // golden main.cpp:34995-34995 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34995-34995 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Shuttle2_Y   =atoi(fMain->Ed_InSH2Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Hotplate1_X  =atoi(EL<TEdit>("TfMain", "Ed_HP1Offset_X")->Text.c_str());   // golden main.cpp:34996-34996 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34996-34996 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Hotplate1_X  =atoi(fMain->Ed_HP1Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Hotplate1_Y  =atoi(EL<TEdit>("TfMain", "Ed_HP1Offset_Y")->Text.c_str());   // golden main.cpp:34997-34997 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34997-34997 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Hotplate1_Y  =atoi(fMain->Ed_HP1Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Hotplate2_X  =atoi(EL<TEdit>("TfMain", "Ed_HP2Offset_X")->Text.c_str());   // golden main.cpp:34998-34998 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34998-34998 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Hotplate2_X  =atoi(fMain->Ed_HP2Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_InArm_Hotplate2_Y  =atoi(EL<TEdit>("TfMain", "Ed_HP2Offset_Y")->Text.c_str());   // golden main.cpp:34999-34999 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:34999-34999 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_InArm_Hotplate2_Y  =atoi(fMain->Ed_HP2Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto1_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto1Offset_X")->Text.c_str());   // golden main.cpp:35000-35000 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35000-35000 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto1_X     =atoi(fMain->Ed_Auto1Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto1_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto1Offset_Y")->Text.c_str());   // golden main.cpp:35001-35001 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35001-35001 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto1_Y     =atoi(fMain->Ed_Auto1Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto2_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto2Offset_X")->Text.c_str());   // golden main.cpp:35002-35002 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35002-35002 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto2_X     =atoi(fMain->Ed_Auto2Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto2_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto2Offset_Y")->Text.c_str());   // golden main.cpp:35003-35003 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35003-35003 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto2_Y     =atoi(fMain->Ed_Auto2Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto3_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto3Offset_X")->Text.c_str());   // golden main.cpp:35004-35004 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35004-35004 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto3_X     =atoi(fMain->Ed_Auto3Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto3_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto3Offset_Y")->Text.c_str());   // golden main.cpp:35005-35005 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35005-35005 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto3_Y     =atoi(fMain->Ed_Auto3Offset_Y->Text.c_str());
#endif // GATE (S12-C save)

    iAOA_OutArm_Auto4_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto4Offset_X")->Text.c_str());   // golden main.cpp:35007-35007 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35007-35007 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto4_X     =atoi(fMain->Ed_Auto4Offset_X->Text.c_str());       //Steven 20240428 : Add for HT9011 AOA
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto4_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto4Offset_Y")->Text.c_str());   // golden main.cpp:35008-35008 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35008-35008 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto4_Y     =atoi(fMain->Ed_Auto4Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto5_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto5Offset_X")->Text.c_str());   // golden main.cpp:35009-35009 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35009-35009 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto5_X     =atoi(fMain->Ed_Auto5Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto5_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto5Offset_Y")->Text.c_str());   // golden main.cpp:35010-35010 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35010-35010 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto5_Y     =atoi(fMain->Ed_Auto5Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto6_X     =atoi(EL<TEdit>("TfMain", "Ed_Auto6Offset_X")->Text.c_str());   // golden main.cpp:35011-35011 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35011-35011 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto6_X     =atoi(fMain->Ed_Auto6Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Auto6_Y     =atoi(EL<TEdit>("TfMain", "Ed_Auto6Offset_Y")->Text.c_str());   // golden main.cpp:35012-35012 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35012-35012 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Auto6_Y     =atoi(fMain->Ed_Auto6Offset_Y->Text.c_str());
#endif // GATE (S12-C save)

    iAOA_OutArm_Fix1_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix1Offset_X")->Text.c_str());   // golden main.cpp:35014-35014 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35014-35014 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix1_X      =atoi(fMain->Ed_Fix1Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix1_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix1Offset_Y")->Text.c_str());   // golden main.cpp:35015-35015 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35015-35015 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix1_Y      =atoi(fMain->Ed_Fix1Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix2_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix2Offset_X")->Text.c_str());   // golden main.cpp:35016-35016 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35016-35016 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix2_X      =atoi(fMain->Ed_Fix2Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix2_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix2Offset_Y")->Text.c_str());   // golden main.cpp:35017-35017 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35017-35017 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix2_Y      =atoi(fMain->Ed_Fix2Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix3_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix3Offset_X")->Text.c_str());   // golden main.cpp:35018-35018 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35018-35018 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix3_X      =atoi(fMain->Ed_Fix3Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix3_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix3Offset_Y")->Text.c_str());   // golden main.cpp:35019-35019 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35019-35019 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix3_Y      =atoi(fMain->Ed_Fix3Offset_Y->Text.c_str());
#endif // GATE (S12-C save)

    iAOA_OutArm_Fix4_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix4Offset_X")->Text.c_str());   // golden main.cpp:35021-35021 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35021-35021 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix4_X      =atoi(fMain->Ed_Fix4Offset_X->Text.c_str());        //Steven 20240428 : Add for HT9011 AOA
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix4_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix4Offset_Y")->Text.c_str());   // golden main.cpp:35022-35022 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35022-35022 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix4_Y      =atoi(fMain->Ed_Fix4Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix5_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix5Offset_X")->Text.c_str());   // golden main.cpp:35023-35023 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35023-35023 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix5_X      =atoi(fMain->Ed_Fix5Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix5_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix5Offset_Y")->Text.c_str());   // golden main.cpp:35024-35024 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35024-35024 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix5_Y      =atoi(fMain->Ed_Fix5Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix6_X      =atoi(EL<TEdit>("TfMain", "Ed_Fix6Offset_X")->Text.c_str());   // golden main.cpp:35025-35025 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35025-35025 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix6_X      =atoi(fMain->Ed_Fix6Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Fix6_Y      =atoi(EL<TEdit>("TfMain", "Ed_Fix6Offset_Y")->Text.c_str());   // golden main.cpp:35026-35026 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35026-35026 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Fix6_Y      =atoi(fMain->Ed_Fix6Offset_Y->Text.c_str());
#endif // GATE (S12-C save)

    iAOA_OutArm_Shuttle1_X  =atoi(EL<TEdit>("TfMain", "Ed_OutSH1Offset_X")->Text.c_str());   // golden main.cpp:35028-35028 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35028-35028 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Shuttle1_X  =atoi(fMain->Ed_OutSH1Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Shuttle1_Y  =atoi(EL<TEdit>("TfMain", "Ed_OutSH1Offset_Y")->Text.c_str());   // golden main.cpp:35029-35029 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35029-35029 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Shuttle1_Y  =atoi(fMain->Ed_OutSH1Offset_Y->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Shuttle2_X  =atoi(EL<TEdit>("TfMain", "Ed_OutSH2Offset_X")->Text.c_str());   // golden main.cpp:35030-35030 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35030-35030 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Shuttle2_X  =atoi(fMain->Ed_OutSH2Offset_X->Text.c_str());
#endif // GATE (S12-C save)
    iAOA_OutArm_Shuttle2_Y  =atoi(EL<TEdit>("TfMain", "Ed_OutSH2Offset_Y")->Text.c_str());   // golden main.cpp:35031-35031 取代：golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
#if 0 // GATE (S12-C save) golden main.cpp:35031-35031 -- golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字
    iAOA_OutArm_Shuttle2_Y  =atoi(fMain->Ed_OutSH2Offset_Y->Text.c_str());
#endif // GATE (S12-C save)

    WriteIniDataGeneral("System", "AOA_InArm_Loader_X", iAOA_InArm_Loader_X);
    WriteIniDataGeneral("System", "AOA_InArm_Loader_Y", iAOA_InArm_Loader_Y);
    WriteIniDataGeneral("System", "AOA_InArm_Shuttle1_X", iAOA_InArm_Shuttle1_X);
    WriteIniDataGeneral("System", "AOA_InArm_Shuttle1_Y", iAOA_InArm_Shuttle1_Y);
    WriteIniDataGeneral("System", "AOA_InArm_Shuttle2_X", iAOA_InArm_Shuttle2_X);
    WriteIniDataGeneral("System", "AOA_InArm_Shuttle2_Y", iAOA_InArm_Shuttle2_Y);
    WriteIniDataGeneral("System", "AOA_InArm_Hotplate1_X", iAOA_InArm_Hotplate1_X);
    WriteIniDataGeneral("System", "AOA_InArm_Hotplate1_Y", iAOA_InArm_Hotplate1_Y);
    WriteIniDataGeneral("System", "AOA_InArm_Hotplate2_X", iAOA_InArm_Hotplate2_X);
    WriteIniDataGeneral("System", "AOA_InArm_Hotplate2_Y", iAOA_InArm_Hotplate2_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto1_X", iAOA_OutArm_Auto1_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto1_Y", iAOA_OutArm_Auto1_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto2_X", iAOA_OutArm_Auto2_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto2_Y", iAOA_OutArm_Auto2_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto3_X", iAOA_OutArm_Auto3_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Auto3_Y", iAOA_OutArm_Auto3_Y);

    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20240428 : Add for HT9011 AOA
    {
        WriteIniDataGeneral("System", "AOA_OutArm_Auto4_X", iAOA_OutArm_Auto4_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Auto4_Y", iAOA_OutArm_Auto4_Y);
        WriteIniDataGeneral("System", "AOA_OutArm_Auto5_X", iAOA_OutArm_Auto5_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Auto5_Y", iAOA_OutArm_Auto5_Y);
        WriteIniDataGeneral("System", "AOA_OutArm_Auto6_X", iAOA_OutArm_Auto6_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Auto6_Y", iAOA_OutArm_Auto6_Y);
    }

    WriteIniDataGeneral("System", "AOA_OutArm_Fix1_X", iAOA_OutArm_Fix1_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Fix1_Y", iAOA_OutArm_Fix1_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Fix2_X", iAOA_OutArm_Fix2_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Fix2_Y", iAOA_OutArm_Fix2_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Fix3_X", iAOA_OutArm_Fix3_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Fix3_Y", iAOA_OutArm_Fix3_Y);

    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20240428 : Add for HT9011 AOA
    {
        WriteIniDataGeneral("System", "AOA_OutArm_Fix4_X", iAOA_OutArm_Fix4_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Fix4_Y", iAOA_OutArm_Fix4_Y);
        WriteIniDataGeneral("System", "AOA_OutArm_Fix5_X", iAOA_OutArm_Fix5_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Fix5_Y", iAOA_OutArm_Fix5_Y);
        WriteIniDataGeneral("System", "AOA_OutArm_Fix6_X", iAOA_OutArm_Fix6_X);
        WriteIniDataGeneral("System", "AOA_OutArm_Fix6_Y", iAOA_OutArm_Fix6_Y);
    }

    WriteIniDataGeneral("System", "AOA_OutArm_Shuttle1_X", iAOA_OutArm_Shuttle1_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Shuttle1_Y", iAOA_OutArm_Shuttle1_Y);
    WriteIniDataGeneral("System", "AOA_OutArm_Shuttle2_X", iAOA_OutArm_Shuttle2_X);
    WriteIniDataGeneral("System", "AOA_OutArm_Shuttle2_Y", iAOA_OutArm_Shuttle2_Y);

}

