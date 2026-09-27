// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TTLCfg 的讀寫檔（HTEditList： → <recipe>\<cbDIOType>.ini（IniConfig.bI16TTLSaveInSetupFile）或 DIOCFGPath\<cbDIOType>.ini）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\DIOInterFaceCFG.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden DIOInterFaceCFG.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 19 個（header 宣告 30 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"

#include <windows.h>   // CopyFile（golden GetDIOFileName :61）；FileRW/TTLCfg_C.cpp 的 FindFirstFile
#include <cstring>     // strncpy（golden LoadData :110）
#include <cstdlib>     // atoi（golden spbSaveClick :211）

// golden DIOInterFaceCFG.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static AnsiString DIOFileName;     // golden DIOInterFaceCFG.h:64（spbLoadClick 設，spbSaveClick 讀）
static bool fShow=false;          // golden DIOInterFaceCFG.h:65（本 TU 自己的；golden Command.cpp:7353 讀它，移植樹沒有讀者）
static bool W906_DeleteDialogExecuted=false;   // [W906] golden OpenDialog1->Execute() 的回傳（spbDeleteClick :252）：網頁對話框選了檔按下去
static AnsiString W906_DeleteDialogFileName;   // [W906] golden OpenDialog1->FileName（:254）：網頁選的檔的完整路徑（DIOCFGPath＋檔名）

using filerw::EL;

// 前置宣告
static void DI_TfDIOFrom();
static void DI_FormShow();
static AnsiString DI_GetDIOFileName();
static void DI_LoadData(AnsiString S);
static void DI_DoIniDataToForm();
static void DI_InitData();
static void DI_spbSaveClick();
static void DI_spbLoadClick();
static void DI_FormClose();
static void DI_spbDeleteClick();

// golden DIOInterFaceCFG.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void DI_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfDIOFrom", "cbOneSTChannel")->Items;
      s->Add("CH1");
      s->Add("CH2");
      s->Add("CH3");
      s->Add("CH4");
    }
    { TStringList* s = EL<TComboBox>("TfDIOFrom", "cbSignalType")->Items;
      s->Add("None");
      s->Add("Positive Level");
      s->Add("Positive Pulse");
      s->Add("Negative Level");
      s->Add("Negative Pulse");
    }
    { TStringList* s = EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->Items;
      s->Add("4Bits(4CH)");
      s->Add("8Bits(2CH)");
      s->Add("5BitsPE(4CH)");
      s->Add("10BitsPE(2CH)");
      s->Add("5Bits(4CH)");
      s->Add("10Bits(2CH)");
      s->Add("5BitPO(4CH)");
      s->Add("10BitsPO(2CH)");
    }
    { TStringList* s = EL<TRadioGroup>("TfDIOFrom", "rgBinDataType")->Items;
      s->Add("One by One");
      s->Add("Binary");
    }
    { TStringList* s = EL<TRadioGroup>("TfDIOFrom", "rgBinLogic")->Items;
      s->Add("Posittive");
      s->Add("Negative");
    }
    { TStringList* s = EL<TRadioGroup>("TfDIOFrom", "rgStartChannel")->Items;
      s->Add("Each");
      s->Add("Single");
    }
    { TStringList* s = EL<TRadioGroup>("TfDIOFrom", "rgStartLogic")->Items;
      s->Add("Posittive");
      s->Add("Negative");
    }
}

// golden DIOInterFaceCFG.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kDI_ParentOf[][2] = {
    {"GroupBox1", "Panel1"},
    {"GroupBox2", "Panel1"},
    {"Label2", "GroupBox2"},
    {"Label4", "GroupBox2"},
    {"Label55", "GroupBox1"},
    {"cbOneSTChannel", "GroupBox1"},
    {"cbSignalType", "GroupBox2"},
    {"edPulseWidth", "GroupBox1"},
    {"edSignalAfterOff", "GroupBox2"},
    {"edSignalBeforeOn", "GroupBox2"},
    {"edTTLModeName", "GroupBox4"},
    {"rgBinBitLength", "GroupBox3"},
    {"rgBinDataType", "GroupBox3"},
    {"rgBinLogic", "GroupBox3"},
    {"rgStartChannel", "GroupBox1"},
    {"rgStartLogic", "GroupBox1"},
    {"spbDelete", "GroupBox4"},
    {"spbLoad", "GroupBox4"},
    {"spbSave", "GroupBox4"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void DI_CreateContainerProxies()
{
    EL<TGroupBox>("TfDIOFrom", "GroupBox1");
    EL<TGroupBox>("TfDIOFrom", "GroupBox2");
    EL<TGroupBox>("TfDIOFrom", "GroupBox3");
    EL<TGroupBox>("TfDIOFrom", "GroupBox4");
    EL<TPanel>("TfDIOFrom", "Panel1");
    filerw::ELSetParents("TfDIOFrom", kDI_ParentOf, (int)(sizeof(kDI_ParentOf) / sizeof(kDI_ParentOf[0])));
}

// golden DIOInterFaceCFG.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void DI_DfmState()
{
    EL<TEdit>("TfDIOFrom", "edTTLModeName")->Enabled = false;
    EL<TComboBox>("TfDIOFrom", "cbSignalType")->Text = "None";
    EL<TComboBox>("TfDIOFrom", "cbSignalType")->ItemIndex = 0;
    EL<TEdit>("TfDIOFrom", "edPulseWidth")->Text = "10";
    EL<TEdit>("TfDIOFrom", "edSignalAfterOff")->Text = "0.00";
    EL<TEdit>("TfDIOFrom", "edSignalBeforeOn")->Text = "10";
    EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->ItemIndex = 5;
    EL<TRadioGroup>("TfDIOFrom", "rgBinDataType")->ItemIndex = 0;
    EL<TRadioGroup>("TfDIOFrom", "rgBinLogic")->ItemIndex = 0;
    EL<TRadioGroup>("TfDIOFrom", "rgStartChannel")->ItemIndex = 0;
    EL<TRadioGroup>("TfDIOFrom", "rgStartLogic")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kDI_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "GroupBox4", "Label2", "Label4",
    "Label55", "Panel1", "rgBinBitLength", "rgBinDataType", "rgBinLogic", "rgStartChannel",
    "rgStartLogic", "spbDelete", "spbLoad", "spbSave",
};

// 存檔流程（spbSaveClick）讀的替身 11 個 —— FileRW/TTLCfg.cpp 用它決定頁面必須送哪些值
static const char* const kDI_SaveReads[] = {
    "cbOneSTChannel", "cbSignalType", "edPulseWidth", "edSignalAfterOff", "edSignalBeforeOn", "edTTLModeName",
    "rgBinBitLength", "rgBinDataType", "rgBinLogic", "rgStartChannel", "rgStartLogic",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void DI_CreateSaveProxies()
{
    EL<TComboBox>("TfDIOFrom", "cbOneSTChannel");
    EL<TComboBox>("TfDIOFrom", "cbSignalType");
    EL<TEdit>("TfDIOFrom", "edPulseWidth");
    EL<TEdit>("TfDIOFrom", "edSignalAfterOff");
    EL<TEdit>("TfDIOFrom", "edSignalBeforeOn");
    EL<TEdit>("TfDIOFrom", "edTTLModeName");
    EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength");
    EL<TRadioGroup>("TfDIOFrom", "rgBinDataType");
    EL<TRadioGroup>("TfDIOFrom", "rgBinLogic");
    EL<TRadioGroup>("TfDIOFrom", "rgStartChannel");
    EL<TRadioGroup>("TfDIOFrom", "rgStartLogic");
}

// golden DIOInterFaceCFG.cpp:19  TfDIOFrom::TfDIOFrom(TComponent* Owner)
static void DI_TfDIOFrom()
{


}

// golden DIOInterFaceCFG.cpp:24  TfDIOFrom::FormShow(TObject *Sender)
static void DI_FormShow()
{

    ;   // golden DIOInterFaceCFG.cpp:26-26 取代：OpenDialog1->InitialDir：開檔對話框的起始資料夾（伺服器端沒有對話框；Load 改成選本配方目前的 DIO 檔，見 spbLoadClick）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:26-26 -- OpenDialog1->InitialDir：開檔對話框的起始資料夾（伺服器端沒有對話框；Load 改成選本配方目前的 DIO 檔，見 spbLoadClick）
    EL<TControl>("TfDIOFrom", "OpenDialog1")->InitialDir=DIOCFGPath;
#endif // GATE (S12-C save)
    DI_InitData();
    ;   // golden DIOInterFaceCFG.cpp:28-29 取代：Top／Left：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:28-29 -- Top／Left：視窗位置（HTML 不用）
    Top=30;
    Left=200;
#endif // GATE (S12-C save)

    if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    {
        EL<TLabel>("TfDIOFrom", "Label55")->Caption="μs";
        EL<TLabel>("TfDIOFrom", "Label4") ->Caption="μs";
        EL<TLabel>("TfDIOFrom", "Label2") ->Caption="μs";
    }

    if(JCET_FOR_EVAN==1)        //Steven 20220506 : 吳如春要求只用10 bit bit
    {
        EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->Enabled=false;
        EL<TRadioGroup>("TfDIOFrom", "rgBinDataType") ->Enabled=false;
    }

    fShow=true;

}

// golden DIOInterFaceCFG.cpp:47  TfDIOFrom::GetDIOFileName()
static AnsiString DI_GetDIOFileName()
{

    AnsiString S, S1;
    AnsiString szDir="";
    if(IniConfig.bI16TTLSaveInSetupFile)
    {
        S=GetLastOpenFN();

    S1.sprintf("%s%s\\%s.ini", DataPath, S, EL<TComboBox>("TFTestIF", "cbDIOType")->Text);   // golden DIOInterFaceCFG.cpp:55-55 取代：FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:55-55 -- FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
        S1.sprintf("%s%s\\%s.ini", DataPath, S, FTestIF->cbDIOType->Text);
#endif // GATE (S12-C save)
        if(FileExists(S1)==false)        //檔案不存在的話,就去複製一份過來
        {
    szDir.sprintf("%s%s.ini", DIOCFGPath, EL<TComboBox>("TFTestIF", "cbDIOType")->Text);   // golden DIOInterFaceCFG.cpp:58-58 取代：FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:58-58 -- FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
            szDir.sprintf("%s%s.ini", DIOCFGPath, FTestIF->cbDIOType->Text);
#endif // GATE (S12-C save)
            if(FileExists(szDir))
            {
                CopyFile(szDir.c_str(), S1.c_str(), false);
            }
        }
    }
    else
    {
    S1.sprintf("%s%s.ini", DIOCFGPath, EL<TComboBox>("TFTestIF", "cbDIOType")->Text);   // golden DIOInterFaceCFG.cpp:67-67 取代：FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:67-67 -- FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好
        S1.sprintf("%s%s.ini", DIOCFGPath, FTestIF->cbDIOType->Text);
#endif // GATE (S12-C save)
    }
    return S1;

}

// golden DIOInterFaceCFG.cpp:72  TfDIOFrom::LoadData(AnsiString S)
static void DI_LoadData(AnsiString S)
{

    AnsiString asString="";

    if(FileExists(S)==false)    //Steven 20180620 (Jou) : 加上DIO檔案遺失的保護判斷
    {
        if(TestIF_File.iTestType==TTL_MODE) //Steven 20210723 : 修正不是TTL模式會Alarm
        {
            filerw::ELMessage("Current DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");
            SystemStart=false;
        }
        return;
    }

    TTLCfg.iSTLogicMode         =ReadIniData(S, "Start Signal",  "Logic", 0);
    TTLCfg.iStartType           =ReadIniData(S, "Start Signal",  "Channel", 0);
    TTLCfg.iOneSTChannel        =ReadIniData(S, "Start Signal",  "SelSignalCH", 0);
    TTLCfg.iSTPluseWidth        =ReadIniData(S, "Start Signal",  "Pluse Width", 0);
    TTLCfg.iDutType             =ReadIniData(S, "DUT Signal",    "Type", 0);
    if(TTLCfg.iDutType<0)   //Steven 20110105
        TTLCfg.iDutType=0;

    TTLCfg.iDutBfOnTime         =ReadIniData(S, "DUT Signal",    "Before On", 0);
    TTLCfg.iDutAfOffTime        =ReadIniData(S, "DUT Signal",    "After Off", 0);

    TTLCfg.iCateLogicMode       =ReadIniData(S, "Cate Signal",   "Logic", 0);
    if(JCET_FOR_EVAN==1)        //Steven 20220506 : 吳如春要求只用10 bit bit
    {
        TTLCfg.iCateBitLength       =3;
        TTLCfg.iCateDataType        =0;
    }
    else
    {
        TTLCfg.iCateBitLength       =ReadIniData(S, "Cate Signal",   "Channel status", 0);
        TTLCfg.iCateDataType        =ReadIniData(S, "Cate Signal",   "Data Type", 0);
    }

    asString=ReadIniData(S, "Name",          "Data Type", AnsiString("AAA"));
    strncpy(TTLCfg.cModeName, asString.c_str(), sizeof(TTLCfg.cModeName));
    if(TTLCfg.iCateBitLength==_5BitPE || TTLCfg.iCateBitLength==_10BitPE)
        TTLCfg.iCateParity=2;//even
    else if(TTLCfg.iCateBitLength==_5BitPO || TTLCfg.iCateBitLength==_10BitPO)
        TTLCfg.iCateParity=1;//odd
    else
        TTLCfg.iCateParity=0;//not use
    if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    {
        TTLCfg.iSTPluseWidth    =CheckRange((int)TTLCfg.iSTPluseWidth, 1, 500000);    //Ifor 20171229 : UNISEM TTL SOT訊號寬度統一修正為1ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutBfOnTime     =CheckRange(TTLCfg.iDutBfOnTime, 1, 500000);
        TTLCfg.iDutAfOffTime    =CheckRange(TTLCfg.iDutAfOffTime, 1, 500000);
    }
    else
    {
        TTLCfg.iSTPluseWidth    =CheckRange((int)TTLCfg.iSTPluseWidth, 10, 500);   //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutBfOnTime     =CheckRange(TTLCfg.iDutBfOnTime, 10, 500);         //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutAfOffTime    =CheckRange(TTLCfg.iDutAfOffTime, 10, 500);        //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
    }

    DI_DoIniDataToForm();

}

// golden DIOInterFaceCFG.cpp:133  TfDIOFrom::DoIniDataToForm()
static void DI_DoIniDataToForm()
{

    EL<TRadioGroup>("TfDIOFrom", "rgStartLogic")->ItemIndex     =TTLCfg.iSTLogicMode;
    EL<TRadioGroup>("TfDIOFrom", "rgStartChannel")->ItemIndex   =TTLCfg.iStartType;
    EL<TEdit>("TfDIOFrom", "edPulseWidth")->Text          =TTLCfg.iSTPluseWidth;
    filerw::ELComboIndex(EL<TComboBox>("TfDIOFrom", "cbOneSTChannel"), TTLCfg.iOneSTChannel);
    filerw::ELComboIndex(EL<TComboBox>("TfDIOFrom", "cbSignalType"), TTLCfg.iDutType);
    EL<TEdit>("TfDIOFrom", "edSignalBeforeOn")->Text      =TTLCfg.iDutBfOnTime;
    EL<TEdit>("TfDIOFrom", "edSignalAfterOff")->Text      =TTLCfg.iDutAfOffTime;
    EL<TRadioGroup>("TfDIOFrom", "rgBinLogic")->ItemIndex       =TTLCfg.iCateLogicMode;
    EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->ItemIndex   =TTLCfg.iCateBitLength;
    EL<TRadioGroup>("TfDIOFrom", "rgBinDataType")->ItemIndex    =TTLCfg.iCateDataType;
    EL<TEdit>("TfDIOFrom", "edTTLModeName")->Text         =TTLCfg.cModeName;

}

// golden DIOInterFaceCFG.cpp:148  TfDIOFrom::InitData()
static void DI_InitData()
{

    EL<TRadioGroup>("TfDIOFrom", "rgStartLogic")->ItemIndex=0;
    EL<TRadioGroup>("TfDIOFrom", "rgStartChannel")->ItemIndex=0;
    EL<TEdit>("TfDIOFrom", "edPulseWidth")->Text="";

    filerw::ELComboIndex(EL<TComboBox>("TfDIOFrom", "cbSignalType"), 0);
    EL<TEdit>("TfDIOFrom", "edSignalBeforeOn")->Text="";
    EL<TEdit>("TfDIOFrom", "edSignalAfterOff")->Text="";

    EL<TRadioGroup>("TfDIOFrom", "rgBinLogic")->ItemIndex=0;
    EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->ItemIndex=0;
    EL<TRadioGroup>("TfDIOFrom", "rgBinDataType")->ItemIndex=0;

    EL<TEdit>("TfDIOFrom", "edTTLModeName")->Text="";
    DIOFileName="";

}

// golden DIOInterFaceCFG.cpp:191  TfDIOFrom::spbSaveClick(TObject *Sender)
static void DI_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden DIOInterFaceCFG.cpp:197-197 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:197-197 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關
        Close();
#endif // GATE (S12-C save)
        return;
    }

    if(DIOFileName=="")
        return;

    ;   // golden DIOInterFaceCFG.cpp:204-204 取代：SaveDialog1->InitialDir：存檔對話框的起始資料夾（伺服器端沒有對話框）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:204-204 -- SaveDialog1->InitialDir：存檔對話框的起始資料夾（伺服器端沒有對話框）
    EL<TControl>("TfDIOFrom", "SaveDialog1")->InitialDir=DIOCFGPath;
#endif // GATE (S12-C save)
    ;   // golden DIOInterFaceCFG.cpp:205-205 取代：SaveDialog1->FileName：存檔對話框的預設檔名＝DIOFileName（:222 再從對話框取回，見下）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:205-205 -- SaveDialog1->FileName：存檔對話框的預設檔名＝DIOFileName（:222 再從對話框取回，見下）
    EL<TControl>("TfDIOFrom", "SaveDialog1")->FileName=DIOFileName;
#endif // GATE (S12-C save)

    if(true)   // golden DIOInterFaceCFG.cpp:207-207 取代：SaveDialog1->Execute()：視同使用者接受預設檔名按「存檔」（另存別的檔名／位置沒有模擬 —— 一律寫開頁載入的那個 DIO 檔）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:207-207 -- SaveDialog1->Execute()：視同使用者接受預設檔名按「存檔」（另存別的檔名／位置沒有模擬 —— 一律寫開頁載入的那個 DIO 檔）
    if(EL<TControl>("TfDIOFrom", "SaveDialog1")->Execute())
#endif // GATE (S12-C save)
    {
        if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        {
            TTLCfg.iSTPluseWidth        =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edPulseWidth")->Text.c_str()), 1, 500000);      //Ifor 20171229 : UNISEM TTL SOT訊號寬度統一修正為1ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutBfOnTime         =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edSignalBeforeOn")->Text.c_str()), 1, 500000);
            TTLCfg.iDutAfOffTime        =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edSignalAfterOff")->Text.c_str()), 1, 500000);
        }
        else
        {
            TTLCfg.iSTPluseWidth        =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edPulseWidth")->Text.c_str()), 10, 500);        //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutBfOnTime         =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edSignalBeforeOn")->Text.c_str()), 10, 500);    //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutAfOffTime        =CheckRange(atoi(EL<TEdit>("TfDIOFrom", "edSignalAfterOff")->Text.c_str()), 10, 500);    //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        }

    filerw::ELMark("DIOFileWritten");   // golden DIOInterFaceCFG.cpp:222-222 取代：DIOFileName=SaveDialog1->FileName：對話框預設檔名就是 DIOFileName（:205），值不變；這裡記 savedMark（下面 11 個 WriteIniData 一定會跑）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:222-222 -- DIOFileName=SaveDialog1->FileName：對話框預設檔名就是 DIOFileName（:205），值不變；這裡記 savedMark（下面 11 個 WriteIniData 一定會跑）
        DIOFileName=EL<TControl>("TfDIOFrom", "SaveDialog1")->FileName;
#endif // GATE (S12-C save)
        WriteIniData(DIOFileName, "Start Signal",  "Logic",           EL<TRadioGroup>("TfDIOFrom", "rgStartLogic")->ItemIndex);
        WriteIniData(DIOFileName, "Start Signal",  "Channel",         EL<TRadioGroup>("TfDIOFrom", "rgStartChannel")->ItemIndex);
        WriteIniData(DIOFileName, "Start Signal",  "Pluse Width",     (int)TTLCfg.iSTPluseWidth);
        WriteIniData(DIOFileName, "Start Signal",  "SelSignalCH",     EL<TComboBox>("TfDIOFrom", "cbOneSTChannel")->ItemIndex);
        WriteIniData(DIOFileName, "DUT Signal",    "Type",            EL<TComboBox>("TfDIOFrom", "cbSignalType")->ItemIndex);
        WriteIniData(DIOFileName, "DUT Signal",    "Before On",       TTLCfg.iDutBfOnTime);
        WriteIniData(DIOFileName, "DUT Signal",    "After Off",       TTLCfg.iDutAfOffTime);
        WriteIniData(DIOFileName, "Cate Signal",   "Logic",           EL<TRadioGroup>("TfDIOFrom", "rgBinLogic")->ItemIndex);
        WriteIniData(DIOFileName, "Cate Signal",   "Channel status",  EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength")->ItemIndex);
        WriteIniData(DIOFileName, "Cate Signal",   "Data Type",       EL<TRadioGroup>("TfDIOFrom", "rgBinDataType")->ItemIndex);
        WriteIniData(DIOFileName, "Name",          "Data Type",       EL<TEdit>("TfDIOFrom", "edTTLModeName")->Text);
    }
    EL<TSpeedButton>("TfDIOFrom", "spbSave")->Down=false;

}

// golden DIOInterFaceCFG.cpp:238  TfDIOFrom::spbLoadClick(TObject *Sender)
static void DI_spbLoadClick()
{

    ;   // golden DIOInterFaceCFG.cpp:240-240 取代：OpenDialog1->Title：開檔對話框標題
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:240-240 -- OpenDialog1->Title：開檔對話框標題
    EL<TControl>("TfDIOFrom", "OpenDialog1")->Title="Open DIO File";
#endif // GATE (S12-C save)
    if(true)   // golden DIOInterFaceCFG.cpp:241-241 取代：OpenDialog1->Execute()：網頁開頁視同使用者按 Load 並在對話框選了檔（見下一條）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:241-241 -- OpenDialog1->Execute()：網頁開頁視同使用者按 Load 並在對話框選了檔（見下一條）
    if(EL<TControl>("TfDIOFrom", "OpenDialog1")->Execute())
#endif // GATE (S12-C save)
    {
    DIOFileName=DI_GetDIOFileName();   // golden DIOInterFaceCFG.cpp:243-243 取代：DIOFileName=OpenDialog1->FileName：選的檔＝本配方目前的 DIO 檔 GetDIOFileName()（golden main.cpp:9415／:28668、cTesterIF.cpp:1302 交給 LoadData 的同一個檔）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:243-243 -- DIOFileName=OpenDialog1->FileName：選的檔＝本配方目前的 DIO 檔 GetDIOFileName()（golden main.cpp:9415／:28668、cTesterIF.cpp:1302 交給 LoadData 的同一個檔）
        DIOFileName=EL<TControl>("TfDIOFrom", "OpenDialog1")->FileName;
#endif // GATE (S12-C save)
        DI_LoadData(DIOFileName);
    }
    EL<TSpeedButton>("TfDIOFrom", "spbLoad")->Down=false;

}

// golden DIOInterFaceCFG.cpp:264  TfDIOFrom::FormClose(TObject *Sender, TCloseAction &Action)
static void DI_FormClose()
{

    fShow=false;
    DI_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔

}

// golden DIOInterFaceCFG.cpp:249  TfDIOFrom::spbDeleteClick(TObject *Sender)
static void DI_spbDeleteClick()
{

    ;   // golden DIOInterFaceCFG.cpp:251-251 取代：OpenDialog1->Title：對話框標題（網頁的刪除清單用 golden 字樣 "Select file to delete"）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:251-251 -- OpenDialog1->Title：對話框標題（網頁的刪除清單用 golden 字樣 "Select file to delete"）
    EL<TControl>("TfDIOFrom", "OpenDialog1")->Title="Select file to delete";
#endif // GATE (S12-C save)
    if(W906_DeleteDialogExecuted)   // golden DIOInterFaceCFG.cpp:252-252 取代：OpenDialog1->Execute()：網頁的刪除對話框（FileRW_TTLCfg_DeleteOp，列 DIOCFGPath\*.ini）選了檔按下去
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:252-252 -- OpenDialog1->Execute()：網頁的刪除對話框（FileRW_TTLCfg_DeleteOp，列 DIOCFGPath\*.ini）選了檔按下去
    if(EL<TControl>("TfDIOFrom", "OpenDialog1")->Execute())
#endif // GATE (S12-C save)
    {
    DeleteFile(W906_DeleteDialogFileName);   // golden DIOInterFaceCFG.cpp:254-254 取代：OpenDialog1->FileName：網頁選的檔（入口只收 DIOCFGPath 底下存在的 *.ini —— golden 的對話框可以換資料夾，網頁版不開放，見 FileRW/TTLCfg.cpp）
#if 0 // GATE (S12-C save) golden DIOInterFaceCFG.cpp:254-254 -- OpenDialog1->FileName：網頁選的檔（入口只收 DIOCFGPath 底下存在的 *.ini —— golden 的對話框可以換資料夾，網頁版不開放，見 FileRW/TTLCfg.cpp）
        DeleteFile(EL<TControl>("TfDIOFrom", "OpenDialog1")->FileName);
#endif // GATE (S12-C save)
    }
    EL<TSpeedButton>("TfDIOFrom", "spbDelete")->Down=false;

}

