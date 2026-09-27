// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_Magazine 的讀寫檔（HTEditList： → <DataPath>\<配方>\HandlerCondition.Data [Configuration] Tray Magazine Source／Mag Fix tray Type／Mag display order（寫：spbSaveClick；讀：移植樹 Magazine.cpp ReadFile）、AuthPath+config.ini（D:\HT9045\config\config.ini）[Magazine Z Offset] In Magazine Offset 0..15／Out Auto3 Offset 0..15（寫：spbSaveClick；讀：移植樹 Magazine.cpp ReadFile））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Magazine.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden Magazine.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 36 個（header 宣告 108 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "MachineType.h"
#include "Magazine.h"
#include "forms/fShowBinSelect.h"
#include "vclcompat/SysUtils.h"


// golden Magazine.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow;                                   // golden Magazine.h:137（建構子設 false；golden 表單外沒有讀者）
static TEdit *EditInMag[16];                         // golden Magazine.h:140（建構子指向 edInMagOfs01..16 替身）
static TEdit *EditOutAuto[16];                       // golden Magazine.h:141（建構子指向 edOutAuto3Ofs01..16 替身）
#define iInMagOfs (fMagazine->iInMagOfs)      // golden Magazine.h:142 → 移植樹門面 Magazine.h（ReadFile 填；golden Magazine.cpp:3044 讀）
#define iOutAutoOfs (fMagazine->iOutAutoOfs)  // golden Magazine.h:143 → 移植樹門面 Magazine.h（ReadFile 填；golden Magazine.cpp:3042 讀）

using filerw::EL;

// 前置宣告
static void MG_TfMagazine();
static void MG_DoIniDataToForm();
static void MG_spbSaveClick();

// golden Magazine.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void MG_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder")->Items;
      s->Add("[1] From top to bottom in sequence14 to 1.");
      s->Add("[2] From top to bottom in sequence1 to 14.");
    }
    { TStringList* s = EL<TRadioGroup>("TfMagazine", "rgMagFixType")->Items;
      s->Add("[1] Fix 1,2,3 used to be unloader tray.");
      s->Add("[2] Fix 1,2,3 used to be buffer area.");
    }
    { TStringList* s = EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->Items;
      s->Add("[1] From handler Empty lane");
      s->Add("[2] Manual Mag. slot");
    }
}

// golden Magazine.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kMG_ParentOf[][2] = {
    {"edInMagOfs01", "GroupBox1"},
    {"edInMagOfs02", "GroupBox1"},
    {"edInMagOfs03", "GroupBox1"},
    {"edInMagOfs04", "GroupBox1"},
    {"edInMagOfs05", "GroupBox1"},
    {"edInMagOfs06", "GroupBox1"},
    {"edInMagOfs07", "GroupBox1"},
    {"edInMagOfs08", "GroupBox1"},
    {"edInMagOfs09", "GroupBox1"},
    {"edInMagOfs10", "GroupBox1"},
    {"edInMagOfs11", "GroupBox1"},
    {"edInMagOfs12", "GroupBox1"},
    {"edInMagOfs13", "GroupBox1"},
    {"edInMagOfs14", "GroupBox1"},
    {"edInMagOfs15", "GroupBox1"},
    {"edInMagOfs16", "GroupBox1"},
    {"edOutAuto3Ofs01", "GroupBox1"},
    {"edOutAuto3Ofs02", "GroupBox1"},
    {"edOutAuto3Ofs03", "GroupBox1"},
    {"edOutAuto3Ofs04", "GroupBox1"},
    {"edOutAuto3Ofs05", "GroupBox1"},
    {"edOutAuto3Ofs06", "GroupBox1"},
    {"edOutAuto3Ofs07", "GroupBox1"},
    {"edOutAuto3Ofs08", "GroupBox1"},
    {"edOutAuto3Ofs09", "GroupBox1"},
    {"edOutAuto3Ofs10", "GroupBox1"},
    {"edOutAuto3Ofs11", "GroupBox1"},
    {"edOutAuto3Ofs12", "GroupBox1"},
    {"edOutAuto3Ofs13", "GroupBox1"},
    {"edOutAuto3Ofs14", "GroupBox1"},
    {"edOutAuto3Ofs15", "GroupBox1"},
    {"edOutAuto3Ofs16", "GroupBox1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void MG_CreateContainerProxies()
{
    EL<TGroupBox>("TfMagazine", "GroupBox1");
    filerw::ELSetParents("TfMagazine", kMG_ParentOf, (int)(sizeof(kMG_ParentOf) / sizeof(kMG_ParentOf[0])));
}

// golden Magazine.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void MG_DfmState()
{
    EL<TEdit>("TfMagazine", "edInMagOfs01")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs02")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs03")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs04")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs05")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs06")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs07")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs08")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs09")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs10")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs11")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs12")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs13")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs14")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs15")->Text = "0";
    EL<TEdit>("TfMagazine", "edInMagOfs16")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs01")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs02")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs03")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs04")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs05")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs06")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs07")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs08")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs09")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs10")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs11")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs12")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs13")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs14")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs15")->Text = "0";
    EL<TEdit>("TfMagazine", "edOutAuto3Ofs16")->Text = "0";
    EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder")->ItemIndex = 0;
    EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex = 0;
    EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kMG_EnableAll[] = {
    "GroupBox1", "rgMagDisplayOrder", "rgMagFixType", "rgMagTraySource", "spbSave",
};

// 存檔流程（spbSaveClick）讀的替身 3 個 —— FileRW/TestIF_File_Magazine.cpp 用它決定頁面必須送哪些值
static const char* const kMG_SaveReads[] = {
    "rgMagDisplayOrder", "rgMagFixType", "rgMagTraySource",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void MG_CreateSaveProxies()
{
    EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder");
    EL<TRadioGroup>("TfMagazine", "rgMagFixType");
    EL<TRadioGroup>("TfMagazine", "rgMagTraySource");
}

// golden Magazine.cpp:44  TfMagazine::TfMagazine(TComponent* Owner)
static void MG_TfMagazine()
{

    fShow=false;
    EditInMag[0]=EL<TEdit>("TfMagazine", "edInMagOfs01");
    EditInMag[1]=EL<TEdit>("TfMagazine", "edInMagOfs02");
    EditInMag[2]=EL<TEdit>("TfMagazine", "edInMagOfs03");
    EditInMag[3]=EL<TEdit>("TfMagazine", "edInMagOfs04");
    EditInMag[4]=EL<TEdit>("TfMagazine", "edInMagOfs05");
    EditInMag[5]=EL<TEdit>("TfMagazine", "edInMagOfs06");
    EditInMag[6]=EL<TEdit>("TfMagazine", "edInMagOfs07");
    EditInMag[7]=EL<TEdit>("TfMagazine", "edInMagOfs08");
    EditInMag[8]=EL<TEdit>("TfMagazine", "edInMagOfs09");
    EditInMag[9]=EL<TEdit>("TfMagazine", "edInMagOfs10");
    EditInMag[10]=EL<TEdit>("TfMagazine", "edInMagOfs11");
    EditInMag[11]=EL<TEdit>("TfMagazine", "edInMagOfs12");
    EditInMag[12]=EL<TEdit>("TfMagazine", "edInMagOfs13");
    EditInMag[13]=EL<TEdit>("TfMagazine", "edInMagOfs14");
    EditInMag[14]=EL<TEdit>("TfMagazine", "edInMagOfs15");
    EditInMag[15]=EL<TEdit>("TfMagazine", "edInMagOfs16");
    EditOutAuto[0]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs01");
    EditOutAuto[1]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs02");
    EditOutAuto[2]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs03");
    EditOutAuto[3]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs04");
    EditOutAuto[4]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs05");
    EditOutAuto[5]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs06");
    EditOutAuto[6]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs07");
    EditOutAuto[7]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs08");
    EditOutAuto[8]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs09");
    EditOutAuto[9]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs10");
    EditOutAuto[10]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs11");
    EditOutAuto[11]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs12");
    EditOutAuto[12]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs13");
    EditOutAuto[13]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs14");
    EditOutAuto[14]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs15");
    EditOutAuto[15]=EL<TEdit>("TfMagazine", "edOutAuto3Ofs16");

}

// golden Magazine.cpp:3654  TfMagazine::DoIniDataToForm()
static void MG_DoIniDataToForm()
{

    EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->ItemIndex=TestIF_File.iMagTraySource;
    EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex=TestIF_File.iMagFixTrayType;
    EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder")->ItemIndex=TestIF_File.iMagDisplayOrder;

    for(int i=0;i<16;i++)
    {
        EditInMag[i]->Text  =iInMagOfs[i];
        EditOutAuto[i]->Text=iOutAutoOfs[i];
    }

}

// golden Magazine.cpp:3547  TfMagazine::spbSaveClick(TObject *Sender)
static void MG_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString sPath=AuthPath+"config.ini";
    AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");
    int temp=0;
    bool bFixHasBin=false, bCanUseBuffer=true;

    filerw::ELMark("MG_WriteIniData"); WriteIniData(szDir, "Configuration", "Tray Magazine Source", (EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->ItemIndex)?1:0);   // golden Magazine.cpp:3554-3554 取代：存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）
#if 0 // GATE (S12-C save) golden Magazine.cpp:3554-3554 -- 存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）
    WriteIniData(szDir, "Configuration", "Tray Magazine Source",                 (EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->ItemIndex)?1:0);
#endif // GATE (S12-C save)

    for(int j=0; j<iTestBinCount; j++)
    {
        temp=Prod.iT6PosCate[j];
        if((temp>=ePosFix1 && temp<=9) ||           //QQQ
           (Prod.iIfErrorT6>=iFixRight && Prod.iIfErrorT6<=iFixRightHalf))
        {
            bFixHasBin=true;
        }
    }

    if(EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex==1)
    {
        if(MachineTypeChoice==Type_HT9046_LS && FIX3_INSTALL==1 && FIX3_FULL_PLACE==Fix3K_Uninstall)
        {
            if(LoadForm->YDivision<10 || LoadForm->XPitch>4000 || DeviceForm_File.XDimension>=40 || DeviceForm_File.YDimension>=40)
            {
                bCanUseBuffer=false;
            }

            if(bCanUseBuffer==false && EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex==1)
            {
                filerw::ELMessage("Device dimension is bigger than 40mm, not supported Magazine buffer tray.");
            }
            else if(bFixHasBin && EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex==1)
            {
                filerw::ELMessage("Fix tray has set bin, can not use to be Magazine buffer tray.");
            }
            else
            {
                WriteIniData(szDir, "Configuration", "Mag Fix tray Type",                    (EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex)?1:0);
            }
        }
        else
        {
            filerw::ELMessage("Machince type is not support Magazine buffer tray");
        }
    }
    else
    {
        WriteIniData(szDir, "Configuration", "Mag Fix tray Type",                    (EL<TRadioGroup>("TfMagazine", "rgMagFixType")->ItemIndex)?1:0);
    }

    for(int i=0; i<16; i++)
    {
        WriteIniData(sPath, "Magazine Z Offset", "In Magazine Offset "+IntToStr(i), atoi(EditInMag[i]->Text.c_str()));
        WriteIniData(sPath, "Magazine Z Offset", "Out Auto3 Offset "  +IntToStr(i), atoi(EditOutAuto[i]->Text.c_str()));
    }
    WriteIniData(szDir, "Configuration", "Mag display order",   (EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder")->ItemIndex)?1:0);    //Ifor 20240227 add:Magazine 顯示順序變更

    if(TestIF_File.iMagDisplayOrder != EL<TRadioGroup>("TfMagazine", "rgMagDisplayOrder")->ItemIndex)
        fShowBinSelect->InitShowBinDigital();

    fMagazine->ReadFile();   // golden Magazine.cpp:3608-3608 取代：golden ReadFile（:3613）：移植樹 Magazine.cpp 已逐字翻好（開機鏈 golden :9351／:9357 也叫它）→ 叫同一支，全樹一份讀檔器
#if 0 // GATE (S12-C save) golden Magazine.cpp:3608-3608 -- golden ReadFile（:3613）：移植樹 Magazine.cpp 已逐字翻好（開機鏈 golden :9351／:9357 也叫它）→ 叫同一支，全樹一份讀檔器
    ReadFile();
#endif // GATE (S12-C save)
    MG_DoIniDataToForm();
    EL<TSpeedButton>("TfMagazine", "spbSave")->Down=false;

}

