// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 IniConfig_OCR 的讀寫檔（HTEditList： → <DataPath>\<配方>\AOI.Data [OCR SETTING] 19 鍵（讀 fOCR_ReadFile：ReadIniData；寫 spbSaveClick：WriteIniData）、system\Gerneral.ini [OCR SETTING] "OCR Port"（fOCR_ReadFile：CheckAndReadIniData，缺鍵補寫 24；存檔不寫））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\OCR.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden OCR.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 25 個（header 宣告 101 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "MessageDef.h"
#include "forms/fMain.h"

static const int OCR_TRIGGER_MODE_SWITCH=0;   // golden OCR.cpp:63（逐字；forms/fOCR.cpp 另有一份給 IsOCRCommandTrigger，const 是內部連結，不衝突）
static const int OCR_TRIGGER_MODE_COMMAND=1;   // golden OCR.cpp:64（逐字；forms/fOCR.cpp 另有一份給 IsOCRCommandTrigger，const 是內部連結，不衝突）

// golden OCR.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bShow=false;   // golden OCR.h:292（建構子 :208 設 false、FormShow :213 設 true、FormClose :228 設 false）；golden 全樹沒有表單外的讀者

using filerw::EL;

// 前置宣告
static void OC_fOCR_ReadFile();
static void OC_fOCR_DoIniDataToForm();
static void OC_FormShow();
static void OC_spbSaveClick();

// golden OCR.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void OC_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TfOCR", "RadioGroup1")->Items;
      s->Add("User	          admin + /r/n ");
      s->Add("Password           /r/n");
      s->Add("Off-line	          SO0 + /r/n");
      s->Add("Load Job File     LF + JobFileName + /r/n");
      s->Add("On-line	          SO1 +/r/n");
    }
    { TStringList* s = EL<TComboBox>("TfOCR", "cbCommReader1")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TRadioGroup>("TfOCR", "rgCommadn")->Items;
      s->Add("#VISIONON+ //詢問系統上線");
      s->Add("#START+    //Handler通知啟動檢測");
      s->Add("#END+      //Handler通知關閉檢測");
      s->Add("#FILE+     //Handler通知換料檔名");
      s->Add("#CLEAR+    //Handler通知結批完成");
      s->Add("#INSP+     //Handler送出比對命令與字串");
    }
}

// golden OCR.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kOC_ParentOf[][2] = {
    {"GroupBox1", "tsOCR_Cognex_Setting"},
    {"GroupBox2", "tsOCR_Cognex_Setting"},
    {"GroupBox3", "GroupBox1"},
    {"IPPort2", "Panel2"},
    {"Panel2", "tsOCR_Cognex"},
    {"TabSheet1", "PageControl1"},
    {"btSend", "palLeft"},
    {"cbCheckBarCodeMap", "GroupBox1"},
    {"ckCompareOCRData", "GroupBox1"},
    {"ckDisabledKeyin", "GroupBox1"},
    {"ckOCRAndBinLog", "GroupBox3"},
    {"ckOCRBinLogAddMark", "GroupBox3"},
    {"ckOCRCheckHasIC", "GroupBox1"},
    {"ckOCRLightChange", "GroupBox1"},
    {"ckOCRLightNoDown", "GroupBox1"},
    {"ckOCRMoveSRead", "GroupBox1"},
    {"ckStartposshift", "GroupBox1"},
    {"edOCRRetry", "GroupBox1"},
    {"edOCRSkip", "GroupBox1"},
    {"edOCRWordCount", "GroupBox1"},
    {"edOCRWordType", "GroupBox1"},
    {"edSetBlueLight", "GroupBox2"},
    {"edSetRedLight", "GroupBox2"},
    {"edStartposshift", "GroupBox1"},
    {"palLeft", "TabSheet1"},
    {"tsOCR_Cognex", "PageControl1"},
    {"tsOCR_Cognex_Setting", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void OC_CreateContainerProxies()
{
    EL<TGroupBox>("TfOCR", "GroupBox1");
    EL<TGroupBox>("TfOCR", "GroupBox2");
    EL<TGroupBox>("TfOCR", "GroupBox3");
    EL<TPageControl>("TfOCR", "PageControl1");
    EL<TPanel>("TfOCR", "Panel2");
    EL<TTabSheet>("TfOCR", "TabSheet1");
    EL<TPanel>("TfOCR", "palLeft");
    EL<TTabSheet>("TfOCR", "tsOCR_Cognex");
    EL<TTabSheet>("TfOCR", "tsOCR_Cognex_Setting");
    filerw::ELSetParents("TfOCR", kOC_ParentOf, (int)(sizeof(kOC_ParentOf) / sizeof(kOC_ParentOf[0])));
}

// golden OCR.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void OC_DfmState()
{
    EL<TCheckBox>("TfOCR", "ckOCRCheckHasIC")->Visible = false;
    EL<TCheckBox>("TfOCR", "ckOCRMoveSRead")->Visible = false;
    EL<TEdit>("TfOCR", "IPPort2")->Text = "23";
    EL<TEdit>("TfOCR", "edOCRRetry")->Text = "2";
    EL<TEdit>("TfOCR", "edOCRSkip")->Text = "2";
    EL<TEdit>("TfOCR", "edOCRWordCount")->Text = "2";
    EL<TEdit>("TfOCR", "edOCRWordType")->Text = "AANN**";
    EL<TEdit>("TfOCR", "edSetBlueLight")->Text = "0";
    EL<TEdit>("TfOCR", "edSetRedLight")->Text = "0";
    EL<TEdit>("TfOCR", "edStartposshift")->Text = "0";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kOC_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "PageControl1", "Panel2", "TabSheet1",
    "btSend", "cbCheckBarCodeMap", "ckCompareOCRData", "ckDisabledKeyin", "ckOCRAndBinLog", "ckOCRBinLogAddMark",
    "ckOCRCheckHasIC", "ckOCRLightChange", "ckOCRLightNoDown", "ckOCRMoveSRead", "ckStartposshift", "ckWordCount",
    "palLeft", "tsOCR_Cognex", "tsOCR_Cognex_Setting",
};

// 存檔流程（spbSaveClick）讀的替身 19 個 —— FileRW/IniConfig_OCR.cpp 用它決定頁面必須送哪些值
static const char* const kOC_SaveReads[] = {
    "cbCheckBarCodeMap", "ckCompareOCRData", "ckDisabledKeyin", "ckOCRAndBinLog", "ckOCRBinLogAddMark", "ckOCRCheckHasIC",
    "ckOCRLightChange", "ckOCRLightNoDown", "ckOCRMoveSRead", "ckStartposshift", "ckWordCount", "edOCRRetry",
    "edOCRSkip", "edOCRWordCount", "edOCRWordType", "edSetBlueLight", "edSetRedLight", "edStartposshift",
    "rgOCRTriggerMode",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void OC_CreateSaveProxies()
{
    EL<TCheckBox>("TfOCR", "cbCheckBarCodeMap");
    EL<TCheckBox>("TfOCR", "ckCompareOCRData");
    EL<TCheckBox>("TfOCR", "ckDisabledKeyin");
    EL<TCheckBox>("TfOCR", "ckOCRAndBinLog");
    EL<TCheckBox>("TfOCR", "ckOCRBinLogAddMark");
    EL<TCheckBox>("TfOCR", "ckOCRCheckHasIC");
    EL<TCheckBox>("TfOCR", "ckOCRLightChange");
    EL<TCheckBox>("TfOCR", "ckOCRLightNoDown");
    EL<TCheckBox>("TfOCR", "ckOCRMoveSRead");
    EL<TCheckBox>("TfOCR", "ckStartposshift");
    EL<TCheckBox>("TfOCR", "ckWordCount");
    EL<TEdit>("TfOCR", "edOCRRetry");
    EL<TEdit>("TfOCR", "edOCRSkip");
    EL<TEdit>("TfOCR", "edOCRWordCount");
    EL<TEdit>("TfOCR", "edOCRWordType");
    EL<TEdit>("TfOCR", "edSetBlueLight");
    EL<TEdit>("TfOCR", "edSetRedLight");
    EL<TEdit>("TfOCR", "edStartposshift");
    EL<TRadioGroup>("TfOCR", "rgOCRTriggerMode");
}

// golden OCR.cpp:2197  TfOCR::fOCR_ReadFile()
static void OC_fOCR_ReadFile()
{

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";

    AnsiString tmpS ="";
    szDir.sprintf("%s%s\\AOI.Data", DataPath, S);

    IniConfig.bDisabledKeyin    = ReadIniData(szDir, "OCR SETTING", "bDisabledKeyin",    false);        //Frank 20161012 add
    IniConfig.bCheckBarCodeMap  = ReadIniData(szDir, "OCR SETTING", "bCheckBarCodeMap",  false);        //Frank 20161025 確認四個角落的OCR Code
    IniConfig.bEnabledOCRCheckIC= ReadIniData(szDir, "OCR SETTING", "bEnabledOCRCheckIC",false);        //wei 20161228 確認Tray是否有IC
    IniConfig.bEnableOCRMoveSRead= ReadIniData(szDir, "OCR SETTING", "bEnabledOCRMoveSRead",false);     //wei 20161118 OCR S型讀取
    IniConfig.bEnableStartposshift= ReadIniData(szDir, "OCR SETTING", "bEnabledStartposshift",false);   //wei 20181225 初始點位
    IniConfig.iStartposshift    = ReadIniData(szDir, "OCR SETTING", "iStartposshift",0);                //wei 20181225 初始點位
    IniConfig.iOCRPort          = CheckAndReadIniData(asGeneralPath, "OCR SETTING", "OCR Port", 24);    //wei 20181225 ocr port
    IniConfig.iOCRTriggerMode   = ReadIniData(szDir, "OCR SETTING", "iOCRTriggerMode", OCR_TRIGGER_MODE_SWITCH); //AI(ht9045-v899) 20260522: default missing OCR trigger mode to switch for old machine compatibility
    if(IniConfig.iOCRTriggerMode<OCR_TRIGGER_MODE_SWITCH ||
       IniConfig.iOCRTriggerMode>OCR_TRIGGER_MODE_COMMAND)
        IniConfig.iOCRTriggerMode=OCR_TRIGGER_MODE_SWITCH;

    IniConfig.iOCRSkip        = ReadIniData(szDir, "OCR SETTING", "iOCRSkip",         3);
    IniConfig.iOCRRetry       = ReadIniData(szDir, "OCR SETTING", "iOCRRetry",       10);
    IniConfig.iOCRWordCount   = ReadIniData(szDir, "OCR SETTING", "iWordCount",      10);
    IniConfig.asOCRWordType   = ReadIniData(szDir, "OCR SETTING", "asOCRWordType",   AnsiString(""));   //wei 20161128 確認字串各自Type是否正確

    IniConfig.iBlueLight      = ReadIniData(szDir, "OCR SETTING", "iBlueLight",       0);
    IniConfig.iRedLight       = ReadIniData(szDir, "OCR SETTING", "iRedLight",        0);
    IniConfig.OCRLightChange  = ReadIniData(szDir, "OCR SETTING", "OCRLightChange",false);              //wei 20181225 光源auto change
    IniConfig.OCRLightNoDown  = ReadIniData(szDir, "OCR SETTING", "OCRLightNoDown",false);              //wei 20181225 光源不下降
    IniConfig.bCompareOCRData = ReadIniData(szDir, "OCR SETTING", "CompareOCRData",false);              //KenHsieh 20220825 : 新增OCR比對功能
    IniConfig.bOCRAndBinLog   = ReadIniData(szDir, "OCR SETTING", "OCRAndBinLog"  ,false);              //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
    IniConfig.bOCRBinLogAddMark=ReadIniData(szDir, "OCR SETTING", "OCRBinLogAddMark" ,false);           //KenHsieh 20230412 : 新增OCR Data + Bin Log功能 + Mark

    IniConfig.bEnabledOCRCheckWordCount= ReadIniData(szDir, "OCR SETTING", "bEnabledOCRCheckWordCount",false);//wei 20220413 開關字數判斷功能

    if(INSTALL_OCR!=eocrUninstal)   //jou 20211029 : 需判斷是否有安裝OCR
    {
        if(TestIF_File.bOcrFunction)            //Steven 20150713 : GPIB update to V2.01 for 2D Code   //wei 20151120
            fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);
        else
            fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);

        if(TestIF_File.b2DUsePinInspection)                                     //Ifor 20240528 add:Pin1 Function
            fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);
        else
            fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);
    }

}

// golden OCR.cpp:2247  TfOCR::fOCR_DoIniDataToForm()
static void OC_fOCR_DoIniDataToForm()
{

    EL<TEdit>("TfOCR", "edOCRSkip")->Text        = IniConfig.iOCRSkip;
    EL<TEdit>("TfOCR", "edOCRRetry")->Text       = IniConfig.iOCRRetry;
    EL<TEdit>("TfOCR", "edOCRWordCount")->Text   = IniConfig.iOCRWordCount;
    EL<TEdit>("TfOCR", "edOCRWordType")->Text    = IniConfig.asOCRWordType;                           //wei 20161128 確認字串各自Type是否正確

    if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
    {
        EL<TTabSheet>("TfOCR", "TabSheet1")->TabVisible=false;
        EL<TTabSheet>("TfOCR", "tsOCR_Cognex")->TabVisible=true;
        EL<TTabSheet>("TfOCR", "tsOCR_Cognex_Setting")->TabVisible=true;
    }
    else
    {
        EL<TTabSheet>("TfOCR", "TabSheet1")->TabVisible=true;
        EL<TTabSheet>("TfOCR", "tsOCR_Cognex")->TabVisible=false;
        EL<TTabSheet>("TfOCR", "tsOCR_Cognex_Setting")->TabVisible=false;
    }

    EL<TEdit>("TfOCR", "edSetBlueLight")->Text= IniConfig.iBlueLight;
    EL<TEdit>("TfOCR", "edSetRedLight")->Text = IniConfig.iRedLight;
    EL<TCheckBox>("TfOCR", "ckDisabledKeyin")  ->Checked =IniConfig.bDisabledKeyin;      //wei 20161004 No IC 不能Keyin
    EL<TCheckBox>("TfOCR", "cbCheckBarCodeMap")->Checked =IniConfig.bCheckBarCodeMap;    //Frank 20161025 確認四個角落的OCR Code
    EL<TCheckBox>("TfOCR", "ckOCRCheckHasIC")  ->Checked =IniConfig.bEnabledOCRCheckIC;  //wei 20161228 確認Tray是否有IC
    EL<TCheckBox>("TfOCR", "ckOCRMoveSRead")   ->Checked =IniConfig.bEnableOCRMoveSRead; //wei 20161118 OCR S型讀取
    EL<TCheckBox>("TfOCR", "ckStartposshift")  ->Checked =IniConfig.bEnableStartposshift;//wei 20181225 初始點位
    EL<TEdit>("TfOCR", "edStartposshift")->Text       = IniConfig.iStartposshift;    //wei 20181225 初始點位
    EL<TCheckBox>("TfOCR", "ckOCRLightChange")->Checked   = IniConfig.OCRLightChange;    //wei 20181225 光源auto change
    EL<TCheckBox>("TfOCR", "ckOCRLightNoDown")->Checked   = IniConfig.OCRLightNoDown;    //wei 20181225 光源不下降
    EL<TEdit>("TfOCR", "IPPort2")->Text               = IniConfig.iOCRPort;          //wei 20181225 ocr port
    EL<TRadioGroup>("TfOCR", "rgOCRTriggerMode")->ItemIndex  = IniConfig.iOCRTriggerMode;   //AI(ht9045-v899) 20260522: show saved switch/SE8 OCR trigger mode, defaulting to switch
    EL<TCheckBox>("TfOCR", "ckCompareOCRData")->Checked   = IniConfig.bCompareOCRData;   //KenHsieh 20220825 : 新增OCR比對功能
    EL<TCheckBox>("TfOCR", "ckOCRAndBinLog")->Checked     = IniConfig.bOCRAndBinLog;     //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
    EL<TCheckBox>("TfOCR", "ckOCRBinLogAddMark")->Checked = IniConfig.bOCRBinLogAddMark; //KenHsieh 20230412 : 新增OCR Data + Bin Log功能 + Mark
    EL<TCheckBox>("TfOCR", "ckWordCount")->Checked        =IniConfig.bEnabledOCRCheckWordCount;  //wei 20220413 開關字數判斷功能

}

// golden OCR.cpp:211  TfOCR::FormShow(TObject *Sender)
static void OC_FormShow()
{

    bShow=true;
    ;   // golden OCR.cpp:214-214 取代：CreateOCRSimulationControls()（AI(ht9045-v899) 20260505）：執行期 new 一個「OCR Simulation」分頁與十幾個元件（TTabSheet／TButton／TTimer…）＋ ckWordCount 改掛 tsOCR_Cognex_Setting —— 純畫面＋OCR 模擬流程（Jimmy），沒有讀寫檔
#if 0 // GATE (S12-C save) golden OCR.cpp:214-214 -- CreateOCRSimulationControls()（AI(ht9045-v899) 20260505）：執行期 new 一個「OCR Simulation」分頁與十幾個元件（TTabSheet／TButton／TTimer…）＋ ckWordCount 改掛 tsOCR_Cognex_Setting —— 純畫面＋OCR 模擬流程（Jimmy），沒有讀寫檔
    CreateOCRSimulationControls();                                             //AI(ht9045-v899) 20260505: add OCR simulation UI after dfm controls are loaded
#endif // GATE (S12-C save)
    EL<TPageControl>("TfOCR", "PageControl1")->ActivePageIndex=2;                                            //tsOCR_Cognex_Setting
    OC_fOCR_ReadFile();
    OC_fOCR_DoIniDataToForm();
    ;   // golden OCR.cpp:218-218 取代：ShowOCRImg()（:885）：imgOCR 載入 _ImgPath+"OCR.bmp"（TImage，純畫面）
#if 0 // GATE (S12-C save) golden OCR.cpp:218-218 -- ShowOCRImg()（:885）：imgOCR 載入 _ImgPath+"OCR.bmp"（TImage，純畫面）
    ShowOCRImg();
#endif // GATE (S12-C save)
    ;   // golden OCR.cpp:219-219 取代：ShowMatchImg()（:900）：imgMatch 載入 _ImgPath+"Match.bmp"（TImage，純畫面）
#if 0 // GATE (S12-C save) golden OCR.cpp:219-219 -- ShowMatchImg()（:900）：imgMatch 載入 _ImgPath+"Match.bmp"（TImage，純畫面）
    ShowMatchImg();
#endif // GATE (S12-C save)
    EL<TCheckBox>("TfOCR", "ckDisabledKeyin")->Visible  =(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bSPILFunction);
    EL<TCheckBox>("TfOCR", "cbCheckBarCodeMap")->Visible=(CUSTOMER_CODE==CC_KYEC_LEE);
    ;   // golden OCR.cpp:222-222 取代：btSend（TPanel）的 BevelInner 外觀：純畫面，vclcompat TPanel 沒有 BevelInner
#if 0 // GATE (S12-C save) golden OCR.cpp:222-222 -- btSend（TPanel）的 BevelInner 外觀：純畫面，vclcompat TPanel 沒有 BevelInner
    EL<TPanel>("TfOCR", "btSend")->BevelInner=bvRaised;
#endif // GATE (S12-C save)

}

// golden OCR.cpp:2151  TfOCR::spbSaveClick(TObject *Sender)
static void OC_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden OCR.cpp:2157-2157 取代：Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔
#if 0 // GATE (S12-C save) golden OCR.cpp:2157-2157 -- Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S="";

    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\AOI.Data", DataPath, S);

    filerw::ELMark("OC_WriteIniData"); WriteIniData(szDir, "OCR SETTING", "iOCRSkip",        EL<TEdit>("TfOCR", "edOCRSkip")->Text);   // golden OCR.cpp:2167-2167 取代：存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 A02 會 return（沒寫檔）
#if 0 // GATE (S12-C save) golden OCR.cpp:2167-2167 -- 存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 A02 會 return（沒寫檔）
    WriteIniData(szDir, "OCR SETTING", "iOCRSkip",        EL<TEdit>("TfOCR", "edOCRSkip")->Text);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "OCR SETTING", "iOCRRetry",       EL<TEdit>("TfOCR", "edOCRRetry")->Text);
    WriteIniData(szDir, "OCR SETTING", "iWordCount",      EL<TEdit>("TfOCR", "edOCRWordCount")->Text);
    WriteIniData(szDir, "OCR SETTING", "asOCRWordType",   EL<TEdit>("TfOCR", "edOCRWordType")->Text);          //wei 20161128 確認字串各自Type是否正確
    WriteIniData(szDir, "OCR SETTING", "iBlueLight",        EL<TEdit>("TfOCR", "edSetBlueLight")->Text);
    WriteIniData(szDir, "OCR SETTING", "iRedLight",         EL<TEdit>("TfOCR", "edSetRedLight")->Text);
    WriteIniData(szDir, "OCR SETTING", "bDisabledKeyin",    EL<TCheckBox>("TfOCR", "ckDisabledKeyin")->Checked);      //wei 20161004 No IC 不能Keyin
    WriteIniData(szDir, "OCR SETTING", "bCheckBarCodeMap",  EL<TCheckBox>("TfOCR", "cbCheckBarCodeMap")->Checked);    //Frank 20161025 確認四個角落的OCR Code
    WriteIniData(szDir, "OCR SETTING", "bEnabledOCRCheckIC",EL<TCheckBox>("TfOCR", "ckOCRCheckHasIC")->Checked);      //wei 20161228 確認Tray是否有IC
    WriteIniData(szDir, "OCR SETTING", "bEnabledOCRMoveSRead",EL<TCheckBox>("TfOCR", "ckOCRMoveSRead")->Checked);     //wei 20161118 OCR S型讀取
    WriteIniData(szDir, "OCR SETTING", "bEnabledStartposshift",EL<TCheckBox>("TfOCR", "ckStartposshift")->Checked);   //wei 20181225 初始點位
    WriteIniData(szDir, "OCR SETTING", "iStartposshift",    EL<TEdit>("TfOCR", "edStartposshift")->Text);         //wei 20181225 初始點位
    WriteIniData(szDir, "OCR SETTING", "OCRLightChange",    EL<TCheckBox>("TfOCR", "ckOCRLightChange")->Checked);     //wei 20181225 光源auto change
    WriteIniData(szDir, "OCR SETTING", "OCRLightNoDown",    EL<TCheckBox>("TfOCR", "ckOCRLightNoDown")->Checked);     //wei 20181225 光源不下降
    WriteIniData(szDir, "OCR SETTING", "iOCRTriggerMode",    EL<TRadioGroup>("TfOCR", "rgOCRTriggerMode")->ItemIndex);   //AI(ht9045-v899) 20260522: persist selected switch/SE8 OCR trigger mode per setup file
    WriteIniData(szDir, "OCR SETTING", "CompareOCRData",    EL<TCheckBox>("TfOCR", "ckCompareOCRData")->Checked);     //KenHsieh 20220825 : 新增OCR比對功能
    WriteIniData(szDir, "OCR SETTING", "bEnabledOCRCheckWordCount",  EL<TCheckBox>("TfOCR", "ckWordCount")->Checked); //wei 20220413 開關字數判斷功能
    WriteIniData(szDir, "OCR SETTING", "OCRAndBinLog",      EL<TCheckBox>("TfOCR", "ckOCRAndBinLog")->Checked);       //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
    WriteIniData(szDir, "OCR SETTING", "OCRBinLogAddMark",  EL<TCheckBox>("TfOCR", "ckOCRBinLogAddMark")->Checked);   //KenHsieh 20230412 : 新增OCR Data + Bin Log功能 + Mark

    OC_fOCR_ReadFile();
    fMain->BackupSetupFile();    //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

