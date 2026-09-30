// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_AGV 的讀寫檔（HTEditList： → D:\HT9045\config\AGV.ini [Configuration]（W906_AGVINI_PATH 測試縫））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation/AGV.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden Automation/AGV.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 27 個（header 宣告 142 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "cmydef.h"
#include "common.h"
#include "vclcompat/SysUtils.h"
#include "Automation/AGV_E84.h"
#include "FileRW/_EditPage.h"   // AI(W906-FRW-S157) 20260927 [W906]：filerw::PageEvent（檔尾 kAG_Events）

AnsiString W906_AgvIniPath();   // FileRW/TestIF_File_AGV.cpp（AI(W906-B8-AG1) 20260930：golden AGV.cpp:1187／:1230 的路徑，測試縫 W906_AGVINI_PATH）

// golden Automation/AGV.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden Automation/AGV.h:176（本 TU 自己的；移植樹 forms/fAGV.h 門面沒有這個成員，golden 也沒有別的檔讀 fAGV->fShow）

using filerw::EL;

// 前置宣告
static void AG_FormShow();
static void AG_FormClose();
static void AG_ReadFile();
static void AG_DoIniDataToForm();
static void AG_spbSaveClick();
static void AG_btInitalLoadClick();
static void AG_btInitalUnLoadClick();

// golden Automation/AGV.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void AG_DfmItems()
{
}

// golden Automation/AGV.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kAG_ParentOf[][2] = {
    {"GroupBox1", "Panel3"},
    {"GroupBox4", "Panel3"},
    {"Panel2", "tsAGV_Setting"},
    {"Panel3", "tsAGV_Setting"},
    {"cbEnableAGVFunction", "Panel3"},
    {"edAuto1Count", "GroupBox4"},
    {"edAuto2Count", "GroupBox4"},
    {"edAuto3Count", "GroupBox4"},
    {"edE84_1_TA1", "gbE84TimeOut"},
    {"edE84_1_TA2", "gbE84TimeOut"},
    {"edE84_1_TA3", "gbE84TimeOut"},
    {"edE84_1_TD0", "gbE84TimeOut"},
    {"edE84_1_TD1", "gbE84TimeOut"},
    {"edE84_1_TP1", "gbE84TimeOut"},
    {"edE84_1_TP2", "gbE84TimeOut"},
    {"edE84_1_TP3", "gbE84TimeOut"},
    {"edE84_1_TP4", "gbE84TimeOut"},
    {"edE84_1_TP5", "gbE84TimeOut"},
    {"edE84_1_TP6", "gbE84TimeOut"},
    {"edE84_2_TA1", "GroupBox1"},
    {"edE84_2_TA2", "GroupBox1"},
    {"edE84_2_TA3", "GroupBox1"},
    {"edE84_2_TD0", "GroupBox1"},
    {"edE84_2_TD1", "GroupBox1"},
    {"edE84_2_TP1", "GroupBox1"},
    {"edE84_2_TP2", "GroupBox1"},
    {"edE84_2_TP3", "GroupBox1"},
    {"edE84_2_TP4", "GroupBox1"},
    {"edE84_2_TP5", "GroupBox1"},
    {"edE84_2_TP6", "GroupBox1"},
    {"gbE84TimeOut", "Panel3"},
    {"spbSave", "Panel2"},
    {"tsAGV_Setting", "AGV"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void AG_CreateContainerProxies()
{
    EL<TPageControl>("TfAGV", "AGV");
    EL<TGroupBox>("TfAGV", "GroupBox1");
    EL<TGroupBox>("TfAGV", "GroupBox4");
    EL<TPanel>("TfAGV", "Panel2");
    EL<TPanel>("TfAGV", "Panel3");
    EL<TGroupBox>("TfAGV", "gbE84TimeOut");
    EL<TTabSheet>("TfAGV", "tsAGV_Setting");
    filerw::ELSetParents("TfAGV", kAG_ParentOf, (int)(sizeof(kAG_ParentOf) / sizeof(kAG_ParentOf[0])));
}

// golden Automation/AGV.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void AG_DfmState()
{
    EL<TEdit>("TfAGV", "edE84_1_TA1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TA2")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TA3")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TD0")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TD1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TP1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TP2")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TP3")->Text = "60";
    EL<TEdit>("TfAGV", "edE84_1_TP4")->Text = "60";
    EL<TEdit>("TfAGV", "edE84_1_TP5")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_1_TP6")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TA1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TA2")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TA3")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TD0")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TD1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TP1")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TP2")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TP3")->Text = "60";
    EL<TEdit>("TfAGV", "edE84_2_TP4")->Text = "60";
    EL<TEdit>("TfAGV", "edE84_2_TP5")->Text = "2";
    EL<TEdit>("TfAGV", "edE84_2_TP6")->Text = "2";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kAG_EnableAll[] = {
    "AGV", "GroupBox1", "GroupBox4", "Panel2", "Panel3", "cbEnableAGVFunction",
    "gbE84TimeOut", "spbSave", "tsAGV_Setting",
};

// 存檔流程（spbSaveClick）讀的替身 26 個 —— FileRW/TestIF_File_AGV.cpp 用它決定頁面必須送哪些值
static const char* const kAG_SaveReads[] = {
    "cbEnableAGVFunction", "edAuto1Count", "edAuto2Count", "edAuto3Count", "edE84_1_TA1", "edE84_1_TA2",
    "edE84_1_TA3", "edE84_1_TD0", "edE84_1_TD1", "edE84_1_TP1", "edE84_1_TP2", "edE84_1_TP3",
    "edE84_1_TP4", "edE84_1_TP5", "edE84_1_TP6", "edE84_2_TA1", "edE84_2_TA2", "edE84_2_TA3",
    "edE84_2_TD0", "edE84_2_TD1", "edE84_2_TP1", "edE84_2_TP2", "edE84_2_TP3", "edE84_2_TP4",
    "edE84_2_TP5", "edE84_2_TP6",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void AG_CreateSaveProxies()
{
    EL<TCheckBox>("TfAGV", "cbEnableAGVFunction");
    EL<TEdit>("TfAGV", "edAuto1Count");
    EL<TEdit>("TfAGV", "edAuto2Count");
    EL<TEdit>("TfAGV", "edAuto3Count");
    EL<TEdit>("TfAGV", "edE84_1_TA1");
    EL<TEdit>("TfAGV", "edE84_1_TA2");
    EL<TEdit>("TfAGV", "edE84_1_TA3");
    EL<TEdit>("TfAGV", "edE84_1_TD0");
    EL<TEdit>("TfAGV", "edE84_1_TD1");
    EL<TEdit>("TfAGV", "edE84_1_TP1");
    EL<TEdit>("TfAGV", "edE84_1_TP2");
    EL<TEdit>("TfAGV", "edE84_1_TP3");
    EL<TEdit>("TfAGV", "edE84_1_TP4");
    EL<TEdit>("TfAGV", "edE84_1_TP5");
    EL<TEdit>("TfAGV", "edE84_1_TP6");
    EL<TEdit>("TfAGV", "edE84_2_TA1");
    EL<TEdit>("TfAGV", "edE84_2_TA2");
    EL<TEdit>("TfAGV", "edE84_2_TA3");
    EL<TEdit>("TfAGV", "edE84_2_TD0");
    EL<TEdit>("TfAGV", "edE84_2_TD1");
    EL<TEdit>("TfAGV", "edE84_2_TP1");
    EL<TEdit>("TfAGV", "edE84_2_TP2");
    EL<TEdit>("TfAGV", "edE84_2_TP3");
    EL<TEdit>("TfAGV", "edE84_2_TP4");
    EL<TEdit>("TfAGV", "edE84_2_TP5");
    EL<TEdit>("TfAGV", "edE84_2_TP6");
}

// golden Automation/AGV.cpp:1304  TfAGV::FormShow(TObject *Sender)
static void AG_FormShow()
{

    AG_ReadFile();
    AG_DoIniDataToForm();
    fShow=true;

}

// golden Automation/AGV.cpp:1311  TfAGV::FormClose(TObject *Sender, TCloseAction &Action)
static void AG_FormClose()
{

    fShow=false;

}

// golden Automation/AGV.cpp:1227  TfAGV::ReadFile()
static void AG_ReadFile()
{

    AnsiString szDir="";
    szDir=W906_AgvIniPath();   // golden Automation/AGV.cpp:1230-1230 取代：測試縫：同 :1187
#if 0 // GATE (S12-C save) golden Automation/AGV.cpp:1230-1230 -- 測試縫：同 :1187
    szDir="D:\\HT9045\\config\\AGV.ini";
#endif // GATE (S12-C save)

    TestIF_File.iE84TimeOut_K12[0][0]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 1", 2);
    TestIF_File.iE84TimeOut_K12[0][1]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 2", 2);
    TestIF_File.iE84TimeOut_K12[0][2]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 3", 60);
    TestIF_File.iE84TimeOut_K12[0][3]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 4", 60);
    TestIF_File.iE84TimeOut_K12[0][4]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 5", 2);
    TestIF_File.iE84TimeOut_K12[0][5]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 6", 2);
    TestIF_File.iE84TimeOut_K12[0][6]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 7", 2);
    TestIF_File.iE84TimeOut_K12[0][7]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 8", 120);
    TestIF_File.iE84TimeOut_K12[0][8]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 9", 60);
    TestIF_File.iE84TimeOut_K12[0][9]       =ReadIniData(szDir, "Configuration", "E84_1 Time Out 10",60);
    TestIF_File.iE84TimeOut_K12[0][10]      =ReadIniData(szDir, "Configuration", "E84_1 Time Out 11",60);

    TestIF_File.iE84TimeOut_K12[1][0]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 1", 2);
    TestIF_File.iE84TimeOut_K12[1][1]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 2", 2);
    TestIF_File.iE84TimeOut_K12[1][2]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 3", 60);
    TestIF_File.iE84TimeOut_K12[1][3]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 4", 60);
    TestIF_File.iE84TimeOut_K12[1][4]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 5", 2);
    TestIF_File.iE84TimeOut_K12[1][5]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 6", 2);
    TestIF_File.iE84TimeOut_K12[1][6]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 7", 2);
    TestIF_File.iE84TimeOut_K12[1][7]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 8", 120);
    TestIF_File.iE84TimeOut_K12[1][8]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 9", 60);
    TestIF_File.iE84TimeOut_K12[1][9]       =ReadIniData(szDir, "Configuration", "E84_2 Time Out 10",60);
    TestIF_File.iE84TimeOut_K12[1][10]      =ReadIniData(szDir, "Configuration", "E84_2 Time Out 11",60);

    //AI(ht9045-e84-flow) 20260825 (RogerYang) : Auto1~3 Tray Count原本只寫不讀, iLoaderUnloaderTrayCount恆0使滿盤判斷恆真
    TestIF_File.iLoaderUnloaderTrayCount[0] =ReadIniData(szDir, "Configuration", "Auto1 Tray Count", 0);
    TestIF_File.iLoaderUnloaderTrayCount[1] =ReadIniData(szDir, "Configuration", "Auto2 Tray Count", 0);
    TestIF_File.iLoaderUnloaderTrayCount[2] =ReadIniData(szDir, "Configuration", "Auto3 Tray Count", 0);

    TestIF_File.bEnableE84                  =ReadIniData(szDir, "Configuration", "E84 Enable"   , false);

}

// golden Automation/AGV.cpp:1264  TfAGV::DoIniDataToForm()
static void AG_DoIniDataToForm()
{

    EL<TCheckBox>("TfAGV", "cbEnableAGVFunction")->Checked      =TestIF_File.bEnableE84;

    EL<TEdit>("TfAGV", "edE84_1_TP1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][0]);
    EL<TEdit>("TfAGV", "edE84_1_TP2")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][1]);
    EL<TEdit>("TfAGV", "edE84_1_TP3")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][2]);
    EL<TEdit>("TfAGV", "edE84_1_TP4")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][3]);
    EL<TEdit>("TfAGV", "edE84_1_TP5")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][4]);
    EL<TEdit>("TfAGV", "edE84_1_TP6")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][5]);
    EL<TEdit>("TfAGV", "edE84_1_TA1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][6]);
    EL<TEdit>("TfAGV", "edE84_1_TA2")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][7]);
    EL<TEdit>("TfAGV", "edE84_1_TA3")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][8]);
    EL<TEdit>("TfAGV", "edE84_1_TD0")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][9]);
    EL<TEdit>("TfAGV", "edE84_1_TD1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[0][10]);

    EL<TEdit>("TfAGV", "edE84_2_TP1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][0]);
    EL<TEdit>("TfAGV", "edE84_2_TP2")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][1]);
    EL<TEdit>("TfAGV", "edE84_2_TP3")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][2]);
    EL<TEdit>("TfAGV", "edE84_2_TP4")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][3]);
    EL<TEdit>("TfAGV", "edE84_2_TP5")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][4]);
    EL<TEdit>("TfAGV", "edE84_2_TP6")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][5]);
    EL<TEdit>("TfAGV", "edE84_2_TA1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][6]);
    EL<TEdit>("TfAGV", "edE84_2_TA2")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][7]);
    EL<TEdit>("TfAGV", "edE84_2_TA3")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][8]);
    EL<TEdit>("TfAGV", "edE84_2_TD0")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][9]);
    EL<TEdit>("TfAGV", "edE84_2_TD1")->Text                 =AnsiString(TestIF_File.iE84TimeOut_K12[1][10]);

    EL<TEdit>("TfAGV", "edAuto1Count")->Text                =AnsiString(TestIF_File.iLoaderUnloaderTrayCount[0]);
    EL<TEdit>("TfAGV", "edAuto2Count")->Text                =AnsiString(TestIF_File.iLoaderUnloaderTrayCount[1]);
    EL<TEdit>("TfAGV", "edAuto3Count")->Text                =AnsiString(TestIF_File.iLoaderUnloaderTrayCount[2]);

}

// golden Automation/AGV.cpp:1184  TfAGV::spbSaveClick(TObject *Sender)
static void AG_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szDir="";
    szDir=W906_AgvIniPath();   // golden Automation/AGV.cpp:1187-1187 取代：測試縫：golden 寫死的 D:\HT9045\config\AGV.ini → W906_AgvIniPath()（沒設 W906_AGVINI_PATH＝golden 字面值）
#if 0 // GATE (S12-C save) golden Automation/AGV.cpp:1187-1187 -- 測試縫：golden 寫死的 D:\HT9045\config\AGV.ini → W906_AgvIniPath()（沒設 W906_AGVINI_PATH＝golden 字面值）
    szDir="D:\\HT9045\\config\\AGV.ini";
#endif // GATE (S12-C save)

    WriteIniData(szDir, "Configuration", "E84_1 Time Out 1",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP1")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 2",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP2")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 3",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP3")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 4",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP4")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 5",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP5")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 6",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TP6")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84_1 Time Out 7",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TA1")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 8",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TA2")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 9",                      atoi(EL<TEdit>("TfAGV", "edE84_1_TA3")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84_1 Time Out 10",                     atoi(EL<TEdit>("TfAGV", "edE84_1_TD0")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_1 Time Out 11",                     atoi(EL<TEdit>("TfAGV", "edE84_1_TD1")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84_2 Time Out 1",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP1")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 2",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP2")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 3",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP3")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 4",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP4")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 5",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP5")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 6",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TP6")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84_2 Time Out 7",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TA1")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 8",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TA2")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 9",                      atoi(EL<TEdit>("TfAGV", "edE84_2_TA3")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84_2 Time Out 10",                     atoi(EL<TEdit>("TfAGV", "edE84_2_TD0")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "E84_2 Time Out 11",                     atoi(EL<TEdit>("TfAGV", "edE84_2_TD1")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "Auto1 Tray Count",                      atoi(EL<TEdit>("TfAGV", "edAuto1Count")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "Auto2 Tray Count",                      atoi(EL<TEdit>("TfAGV", "edAuto2Count")->Text.c_str()));
    WriteIniData(szDir, "Configuration", "Auto3 Tray Count",                      atoi(EL<TEdit>("TfAGV", "edAuto3Count")->Text.c_str()));

    WriteIniData(szDir, "Configuration", "E84 Enable",                            (EL<TCheckBox>("TfAGV", "cbEnableAGVFunction")->Checked)?1:0);

    AG_ReadFile();
    EL<TSpeedButton>("TfAGV", "spbSave")->Down=false;

}

// golden Automation/AGV.cpp:1316  TfAGV::btInitalLoadClick(TObject *Sender)
static void AG_btInitalLoadClick()
{

    InitialE84LoadTask();
    InitialE84LoadSensor();
    for(int i=0; i<3; i++)                                                      //AI(ht9045-e84-flow) 20260814 (RogerYang) : 逾時路徑沒清, 這裡釋放軌道互鎖
        bE84LoaderActionflag[i]=false;

}

// golden Automation/AGV.cpp:1324  TfAGV::btInitalUnLoadClick(TObject *Sender)
static void AG_btInitalUnLoadClick()
{

    InitialE84UnLoaderTask();
    InitialE84UnloadSensor();
    for(int i=0; i<3; i++)                                                      //AI(ht9045-e84-flow) 20260814 (RogerYang) : 逾時路徑沒清, 這裡釋放軌道互鎖
        bE84UnloaderActionflag[i]=false;

}

// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/editlist/TestIF_File_AGV.py 的 events；
//   FileRW/TestIF_File_AGV.cpp 用 filerw::PageEventsRegistrar 註冊；本體 FileRW/_EditPage.cpp RunPageEvent）
static void AG_Ev_btInitalLoadClick(TControl* Sender) { (void)Sender; AG_btInitalLoadClick(); }
static void AG_Ev_btInitalUnLoadClick(TControl* Sender) { (void)Sender; AG_btInitalUnLoadClick(); }
static const filerw::PageEvent kAG_Events[] = {
    {"btInitalLoad", "click", "Automation/AGV.cpp:1316 TfAGV::btInitalLoadClick", &AG_Ev_btInitalLoadClick},
    {"btInitalUnLoad", "click", "Automation/AGV.cpp:1324 TfAGV::btInitalUnLoadClick", &AG_Ev_btInitalUnLoadClick},
};

