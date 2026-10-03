// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_QAMode 的讀寫檔（HTEditList： → <recipe>\Tester.Data [QA Mode]／[QA Sampling]）。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\QAMode.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden QAMode.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 16 個（header 宣告 26 個）。
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
#include "forms/fLotInfo.h"
#include "forms/fBinSel.h"

void MyDBIProcess(AnsiString S1, AnsiString S2);   // golden aHotPlateSubstrate.h:924（aHotPlateSubstrate.h 與 HTEditList.h 衝突，只宣告這一支；ShowTrayDirectIMG :274）

// golden QAMode.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden QAMode.h:58（本 TU 自己的；移植樹 fQAMode->fShow 是 forms/fQAMode.h 的另一個，Command.cpp 的讀者仍是 #if 0）

using filerw::EL;

// 前置宣告
static void QA_TfQAMode();
static void QA_FormShow();
static void QA_FormClose();
static void QA_ReadFile();
static void QA_DoIniDataToForm();
static void QA_DoFormToData();
static void QA_btnApplyClick();
static void QA_ShowTrayDirectIMG();

// golden QAMode.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void QA_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfQAMode", "cbQAModeBin")->Items;
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
    { TStringList* s = EL<TComboBox>("TfQAMode", "cbbQAD22Double")->Items;
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
    { TStringList* s = EL<TComboBox>("TfQAMode", "cbbQASampleBin")->Items;
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
    { TStringList* s = EL<TComboBox>("TfQAMode", "cbbQASampleTray")->Items;
      s->Add("Auto 1");
      s->Add("Auto 2");
      s->Add("Auto 3");
      s->Add("Fix 1");
      s->Add("Fix 2");
      s->Add("Fix 3");
    }
    { TStringList* s = EL<TRadioGroup>("TfQAMode", "rgQARunMode")->Items;
      s->Add("Clean out every devices without test automatically.");
      s->Add("One cycle and alarm, then normal production.");
      s->Add("Clean out every devices as setted bin automatically.");
      s->Add("Not counting Device.Need to end manually.");
    }
}

// golden QAMode.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kQA_ParentOf[][2] = {
    {"Image1", "gbLoaderDirection"},
    {"cbQAD22Double", "gbQAMode"},
    {"cbQAModeBin", "gbQAMode"},
    {"cbTempOffset", "pnlQASample"},
    {"cbTrayEndCloseYield100Site", "gbQAMode"},
    {"cbTrayEndDoQAModeAgain", "gbQAMode"},
    {"cbbQAD22Double", "gbQAMode"},
    {"cbbQASampleBin", "pnlQASample"},
    {"cbbQASampleTray", "pnlQASample"},
    {"edQAMode", "gbQAMode"},
    {"edtQASampleCnt", "pnlQASample"},
    {"gbLoaderDirection", "gbQAMode"},
    {"gbQAMode", "tsQAMode"},
    {"labQAModeBin", "gbQAMode"},
    {"pnlQASample", "tsQASampling"},
    {"rgQARunMode", "gbQAMode"},
    {"tsQAMode", "pgcQAMode"},
    {"tsQASampling", "pgcQAMode"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void QA_CreateContainerProxies()
{
    EL<TGroupBox>("TfQAMode", "gbLoaderDirection");
    EL<TGroupBox>("TfQAMode", "gbQAMode");
    EL<TPageControl>("TfQAMode", "pgcQAMode");
    EL<TPanel>("TfQAMode", "pnlQASample");
    EL<TTabSheet>("TfQAMode", "tsQAMode");
    EL<TTabSheet>("TfQAMode", "tsQASampling");
    filerw::ELSetParents("TfQAMode", kQA_ParentOf, (int)(sizeof(kQA_ParentOf) / sizeof(kQA_ParentOf[0])));
}

// golden QAMode.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void QA_DfmState()
{
    EL<TComboBox>("TfQAMode", "cbQAModeBin")->Text = "1";
    EL<TCheckBox>("TfQAMode", "cbTempOffset")->Checked = true;
    EL<TComboBox>("TfQAMode", "cbbQAD22Double")->Text = "2";
    EL<TComboBox>("TfQAMode", "cbbQASampleBin")->Text = "1";
    EL<TComboBox>("TfQAMode", "cbbQASampleBin")->ItemIndex = 1;
    EL<TComboBox>("TfQAMode", "cbbQASampleTray")->Text = "Fix 1";
    EL<TComboBox>("TfQAMode", "cbbQASampleTray")->ItemIndex = 3;
    EL<TEdit>("TfQAMode", "edQAMode")->Text = "50";
    EL<TEdit>("TfQAMode", "edtQASampleCnt")->Text = "100";
    EL<TRadioGroup>("TfQAMode", "rgQARunMode")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kQA_EnableAll[] = {
    "cbQAD22Double", "cbTempOffset", "cbTrayEndCloseYield100Site", "cbTrayEndDoQAModeAgain", "gbLoaderDirection", "gbQAMode",
    "labQAModeBin", "pgcQAMode", "pnlQASample", "rgQARunMode", "tsQAMode", "tsQASampling",
};

// 存檔流程（btnApplyClick、DoFormToData）讀的替身 11 個 —— FileRW/TestIF_File_QAMode.cpp 用它決定頁面必須送哪些值
static const char* const kQA_SaveReads[] = {
    "cbQAD22Double", "cbQAModeBin", "cbTempOffset", "cbTrayEndCloseYield100Site", "cbTrayEndDoQAModeAgain", "cbbQAD22Double",
    "cbbQASampleBin", "cbbQASampleTray", "edQAMode", "edtQASampleCnt", "rgQARunMode",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void QA_CreateSaveProxies()
{
    EL<TCheckBox>("TfQAMode", "cbQAD22Double");
    EL<TComboBox>("TfQAMode", "cbQAModeBin");
    EL<TCheckBox>("TfQAMode", "cbTempOffset");
    EL<TCheckBox>("TfQAMode", "cbTrayEndCloseYield100Site");
    EL<TCheckBox>("TfQAMode", "cbTrayEndDoQAModeAgain");
    EL<TComboBox>("TfQAMode", "cbbQAD22Double");
    EL<TComboBox>("TfQAMode", "cbbQASampleBin");
    EL<TComboBox>("TfQAMode", "cbbQASampleTray");
    EL<TEdit>("TfQAMode", "edQAMode");
    EL<TEdit>("TfQAMode", "edtQASampleCnt");
    EL<TRadioGroup>("TfQAMode", "rgQARunMode");
}

// golden QAMode.cpp:22  TfQAMode::TfQAMode(TComponent* Owner)
static void QA_TfQAMode()
{


}

// golden QAMode.cpp:113  TfQAMode::FormShow(TObject *Sender)
static void QA_FormShow()
{

    EL<TComboBox>("TfQAMode", "cbQAModeBin")->Clear();                                                       //Steven 20150122 : Fixed for QA Mode
    for(int i=0; i<iTestBinCount; i++)
        EL<TComboBox>("TfQAMode", "cbQAModeBin")->Items->Add(i);

    EL<TComboBox>("TfQAMode", "cbbQASampleTray")->Clear();
    for(int i=0; i<iFixRight; i++)
    {
        if(Prod.iTrayType[i]==tTrayAuto)
            EL<TComboBox>("TfQAMode", "cbbQASampleTray")->Items->Add(s6TrayName[i]);
    }

    ;   // golden QAMode.cpp:126-127 取代：Left／Top：視窗位置（HTML 不用；vclcompat 沒有 Width／Height）
#if 0 // GATE (S12-C save) golden QAMode.cpp:126-127 -- Left／Top：視窗位置（HTML 不用；vclcompat 沒有 Width／Height）
    Left=(1280-Width)/2;
    Top =(1024-Height)/2;
#endif // GATE (S12-C save)

    QA_ReadFile();
    QA_DoIniDataToForm();
    QA_ShowTrayDirectIMG();
    EL<TComboBox>("TfQAMode", "cbQAModeBin")->Enabled=(AccessLevel<LevelSet.AccessLevel[85])?false:true;
    if(CosFunction.bQAModeUseUnloadCnt)                                         //JerryYang 20221004 : Maxim版本QA mode
    {
        EL<TRadioGroup>("TfQAMode", "rgQARunMode")->Enabled=false;
    }
    else
    {
        EL<TRadioGroup>("TfQAMode", "rgQARunMode")->Enabled=(AccessLevel<LevelSet.AccessLevel[85])?false:true;
    }
    EL<TGroupBox>("TfQAMode", "gbLoaderDirection")->Enabled=(AccessLevel<LevelSet.AccessLevel[85])?false:true;
    EL<TEdit>("TfQAMode", "edQAMode")->Enabled=(AccessLevel<LevelSet.AccessLevel[98])?false:true;
    EL<TPanel>("TfQAMode", "pnlQASample")->Enabled=(AccessLevel<LevelSet.AccessLevel[85])?false:true;     //Steven 20190326 : QA Sampling

    if(CUSTOMER_CODE==CC_SCS)
        EL<TRadioGroup>("TfQAMode", "rgQARunMode")->Enabled=false;                                             // wei  20130103

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Sam 20240122 : 矽格北興俊堯要求鎖死
    {
        EL<TEdit>("TfQAMode", "edQAMode")->Enabled=false;
        EL<TRadioGroup>("TfQAMode", "rgQARunMode")->Enabled=false;
        EL<TComboBox>("TfQAMode", "cbQAModeBin")->Enabled=false;
        EL<TCheckBox>("TfQAMode", "cbTrayEndDoQAModeAgain")->Enabled=false;
    }

    if(IniConfig.bMaximFunction==true ||                                        //JerryYang 20221004 : Maxim版本QA mode
       CUSTOMER_CODE==CC_AnalogDevice_Phil)
    {
        EL<TLabel>("TfQAMode", "labQAModeBin")->Visible=false;
        EL<TComboBox>("TfQAMode", "cbQAModeBin")->Visible=false;
        EL<TCheckBox>("TfQAMode", "cbTrayEndDoQAModeAgain")->Visible=false;
    }

    fShow=true;
    EL<TTabSheet>("TfQAMode", "tsQASampling")->TabVisible=(CUSTOMER_CODE==CC_AMKOR_Korea &&
                              TestIF_File.bSCKART_EnableART);                   //Steven 20190326 : QA Sampling
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden QAMode.cpp:171  TfQAMode::FormClose(TObject *Sender, TCloseAction &Action)
static void QA_FormClose()
{

    fShow=false;
    QA_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden QAMode.cpp:179  TfQAMode::ReadFile()
static void QA_ReadFile()
{

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);
    MyForceDirectories(szDir);
    szDir+="\\Tester.Data";

    TestIF_File.iQAModeCount    =CheckRange(ReadIniData(szDir, "QA Mode", "Count", 200), 5, 10000);  //Steven 20111026

    if(CUSTOMER_CODE==CC_SCS || CosFunction.bQAModeUseUnloadCnt)                //JerryYang 20221004 : Maxim版本QA mode
        TestIF_File.iQAModeRunType=1;                                           // wei  20130103
    else if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                   //Sam 20240122 : 矽格北興俊堯要求鎖死
        TestIF_File.iQAModeRunType=3;
    else
        TestIF_File.iQAModeRunType=ReadIniData(szDir, "QA Mode", "Run Type", 0);//Steven 20120612 : QA做完後的動作

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //ChungHung 20141204 add for ATK
        TestIF_File.iQAModeBin=ReadIniData(szDir, "QA Mode", "Untest Bin", 1);
    else
        TestIF_File.iQAModeBin=ReadIniData(szDir, "QA Mode", "Untest Bin", iTestBinCount-1);                    //Steven 20141023 : QA做完後的Bin
    TestIF_File.iQAModeBin=CheckRange(TestIF_File.iQAModeBin, 0, iTestBinCount-1);                              //Steven 20150122 : Fixed for QA Mode

    TestIF_File.iQATrayDirect      =CheckAndReadIniData(szDir, "QA Mode", "Tray Direct", TrayForm.Loader.Direction);
    TestIF_File.bQAModeAfterTrayEnd=CheckAndReadIniData(szDir, "QA Mode", "bQAModeAfterTrayEnd", false);        //Steven 20151125 : QA做完後的TrayEnd要重做QA
    TestIF_File.bQAD22DoubleContact=CheckAndReadIniData(szDir, "QA Mode", "bQAD22DoubleContact", false);        //Sam 20231117 : 整合到 QA 模式
    TestIF_File.iQAD22DoubleContactCount=CheckAndReadIniData(szDir, "QA Mode", "iQAD22DoubleContactCount", 1);              //Sam 20231117 : 整合到 QA 模式
    TestIF_File.bQATrayEndCloseYield100Site=CheckAndReadIniData(szDir, "QA Mode", "bQATrayEndCloseYield100Site", false);        //Sam 20231117 : 整合到 QA 模式

    TestIF_File.bEnableQASampling  =CheckAndReadIniData(szDir, "QA Sampling", "Enable",    false);             //Steven 20190326 : QA Sampling
    TestIF_File.iQASamplingBin     =CheckAndReadIniData(szDir, "QA Sampling", "Bin",       1);

    if(CheckKeyExist(szDir, "QA Sampling", "Tray Name")==false)
    {
        TestIF_File.iQASamplingT3Pos=CheckAndReadIniData(szDir, "QA Sampling", "Tray",      3);

        int iTray=TestIF_File.iQASamplingT3Pos-1;
        if(iTray<0)
            TestIF_File.sQASamplingTray=s3TrayName[0];
        else
            TestIF_File.sQASamplingTray=s3TrayName[iTray];
    }
    else
    {
        TestIF_File.sQASamplingTray=CheckAndReadIniData(szDir, "QA Sampling", "Tray Name", AnsiString("Fix1"));
        TestIF_File.iQASamplingT3Pos=0;                                         //因為0是no-use
        for(int i3=0; i3<e3TrayCount; i3++)
        {
            if(s3TrayName[i3]==TestIF_File.sQASamplingTray)
                TestIF_File.iQASamplingT3Pos=i3+1;
        }
    }
    TestIF_File.iQASamplingCnt     =CheckAndReadIniData(szDir, "QA Sampling", "Count",     100);

}

// golden QAMode.cpp:38  TfQAMode::DoIniDataToForm()
static void QA_DoIniDataToForm()
{

    fLotInfo->edQAMode->Text         =TestIF_File.iQAModeCount;                 //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
    EL<TEdit>("TfQAMode", "edQAMode")->Text                  =TestIF_File.iQAModeCount;                  //Steven 20111026
    EL<TRadioGroup>("TfQAMode", "rgQARunMode")->ItemIndex          =TestIF_File.iQAModeRunType;                //Steven 20120612 : QA做完後的動作
    filerw::ELComboIndex(EL<TComboBox>("TfQAMode", "cbQAModeBin"), TestIF_File.iQAModeBin);                    //Steven 20141023 : QA做完後的Bin
    EL<TCheckBox>("TfQAMode", "cbTrayEndDoQAModeAgain")->Checked =TestIF_File.bQAModeAfterTrayEnd;           //Steven 20151125 : QA做完後的TrayEnd要重做QA
    EL<TCheckBox>("TfQAMode", "cbQAD22Double")->Checked          =TestIF_File.bQAD22DoubleContact;           //Sam 20231117 : 整合到 QA 模式
    filerw::ELComboIndex(EL<TComboBox>("TfQAMode", "cbbQAD22Double"), TestIF_File.iQAD22DoubleContactCount);      //Sam 20231117 : 整合到 QA 模式
    EL<TCheckBox>("TfQAMode", "cbTrayEndCloseYield100Site")->Checked=TestIF_File.bQATrayEndCloseYield100Site;//Sam 20231117 : 整合到 QA 模式

    EL<TCheckBox>("TfQAMode", "cbTempOffset")->Checked           =TestIF_File.bEnableQASampling;             //Steven 20190326 : QA Sampling
    filerw::ELComboIndex(EL<TComboBox>("TfQAMode", "cbbQASampleBin"), TestIF_File.iQASamplingBin);
//    cbbQASampleTray->ItemIndex      =TestIF_File.iQASamplingT3Pos-1;          //因為0是no-use
    filerw::ELComboText(EL<TComboBox>("TfQAMode", "cbbQASampleTray"), TestIF_File.sQASamplingTray);
    EL<TEdit>("TfQAMode", "edtQASampleCnt")->Text            =TestIF_File.iQASamplingCnt;

}

// golden QAMode.cpp:56  TfQAMode::DoFormToData()
static void QA_DoFormToData()
{
    filerw::ELMark("DoFormToData");   // FileRW：存檔流程 trace（ack.trace）

    TestIF_File.iQAModeCount        =CheckRange(atoi(EL<TEdit>("TfQAMode", "edQAMode")->Text.c_str()), 5, 10000);    //Steven 20111026
    TestIF_File.iQAModeRunType      =EL<TRadioGroup>("TfQAMode", "rgQARunMode")->ItemIndex;                                //Steven 20120612 : QA做完後的動作
    TestIF_File.iQAModeBin          =EL<TComboBox>("TfQAMode", "cbQAModeBin")->ItemIndex;                                //Steven 20141023 : QA做完後的Bin
    TestIF_File.bQAModeAfterTrayEnd =EL<TCheckBox>("TfQAMode", "cbTrayEndDoQAModeAgain")->Checked;                       //Steven 20151125 : QA做完後的TrayEnd要重做QA
    TestIF_File.bQAD22DoubleContact =EL<TCheckBox>("TfQAMode", "cbQAD22Double")->Checked;                                //Sam 20231117 : 整合到 QA 模式
    TestIF_File.iQAD22DoubleContactCount=EL<TComboBox>("TfQAMode", "cbbQAD22Double")->ItemIndex;                         //Sam 20231117 : 整合到 QA 模式
    TestIF_File.bQATrayEndCloseYield100Site=EL<TCheckBox>("TfQAMode", "cbTrayEndCloseYield100Site")->Checked;            //Sam 20231117 : 整合到 QA 模式

    TestIF_File.bEnableQASampling   =EL<TCheckBox>("TfQAMode", "cbTempOffset")->Checked;                     //Steven 20190326 : QA Sampling
    TestIF_File.iQASamplingBin      =EL<TComboBox>("TfQAMode", "cbbQASampleBin")->ItemIndex;
//    TestIF_File.iQASamplingT3Pos     =cbbQASampleTray->ItemIndex+1;           //因為0是no-use
    TestIF_File.sQASamplingTray     =EL<TComboBox>("TfQAMode", "cbbQASampleTray")->Text;
    TestIF_File.iQASamplingCnt      =atoi(EL<TEdit>("TfQAMode", "edtQASampleCnt")->Text.c_str());

}

// golden QAMode.cpp:73  TfQAMode::btnApplyClick(TObject *Sender)
static void QA_btnApplyClick()
{
    filerw::ELMark("btnApplyClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden QAMode.cpp:79-79 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（PageSave 沒寫檔就照 golden FormClose 還原替身）
#if 0 // GATE (S12-C save) golden QAMode.cpp:79-79 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（PageSave 沒寫檔就照 golden FormClose 還原替身）
        Close();
#endif // GATE (S12-C save)
    return;   // golden QAMode.cpp:80-80 取代：AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 QAMode.cpp:75-80：A02 分支只有 Close()（:79），後面沒有 return，存檔照跑；主選單 main.cpp:28881 用 Show 開（非對話框），Close() 當場跑 FormClose（:170-176），其中 DoIniDataToForm()（:173，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。V912 QAMode.cpp:79-80：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。Kept: #20 exception (Steven 1003 Q78, keep V912)
#if 0 // GATE (S12-C save) golden QAMode.cpp:80-80 -- AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 QAMode.cpp:75-80：A02 分支只有 Close()（:79），後面沒有 return，存檔照跑；主選單 main.cpp:28881 用 Show 開（非對話框），Close() 當場跑 FormClose（:170-176），其中 DoIniDataToForm()（:173，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。V912 QAMode.cpp:79-80：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。Kept: #20 exception (Steven 1003 Q78, keep V912)
        return;
#endif // GATE (S12-C save)
    }

    QA_DoFormToData();

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s", DataPath, S);
    MyForceDirectories(szDir);
    szDir+="\\Tester.Data";

    WriteIniData(szDir, "QA Mode", "Count",                 int(TestIF_File.iQAModeCount));     //Steven 20111026
    WriteIniData(szDir, "QA Mode", "Run Type",              TestIF_File.iQAModeRunType);        //Steven 20120612 : QA做完後的動作
    WriteIniData(szDir, "QA Mode", "Tray Direct",           TestIF_File.iQATrayDirect);
    WriteIniData(szDir, "QA Mode", "Untest Bin",            TestIF_File.iQAModeBin);            //Steven 20141023 : QA做完後的Bin
    WriteIniData(szDir, "QA Mode", "bQAModeAfterTrayEnd",   TestIF_File.bQAModeAfterTrayEnd);   //Steven 20151125 : QA做完後的TrayEnd要重做QA
    WriteIniData(szDir, "QA Mode", "bQAD22DoubleContact",   TestIF_File.bQAD22DoubleContact);   //Sam 20231117 : 整合到 QA 模式
    WriteIniData(szDir, "QA Mode", "iQAD22DoubleContactCount", TestIF_File.iQAD22DoubleContactCount);       //Sam 20231117 : 整合到 QA 模式
    WriteIniData(szDir, "QA Mode", "bQATrayEndCloseYield100Site",   TestIF_File.bQATrayEndCloseYield100Site);   //Sam 20231117 : 整合到 QA 模式

    WriteIniData(szDir, "QA Sampling", "Enable",    TestIF_File.bEnableQASampling);             //Steven 20190326 : QA Sampling
    WriteIniData(szDir, "QA Sampling", "Bin",       TestIF_File.iQASamplingBin);
//    WriteIniData(szDir, "QA Sampling", "Tray",      TestIF_File.iQASamplingTray);
    WriteIniData(szDir, "QA Sampling", "Tray Name", TestIF_File.sQASamplingTray);               //Steven 20230930 : 改用Tray名稱存檔, 避免異常
    WriteIniData(szDir, "QA Sampling", "Count",     TestIF_File.iQASamplingCnt);
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    QA_ReadFile();                                                                 //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
    QA_DoIniDataToForm();

    fBinSel->ReadFile(false, false, "");                                                        //Steven 20190326 : QA Sampling

}

// golden QAMode.cpp:256  TfQAMode::ShowTrayDirectIMG()
static void QA_ShowTrayDirectIMG()
{

    AnsiString sDir;
    if(CosFunction.bQAmodeSupplyTrayDir==false)
    {
        EL<TGroupBox>("TfQAMode", "gbLoaderDirection")->Visible=false;
    }
    else
    {
        if(TestIF_File.iQATrayDirect<0 || TestIF_File.iQATrayDirect>8)
            TestIF_File.iQATrayDirect=0;
        sDir.sprintf("%stype%d.bmp", BmpPath, TestIF_File.iQATrayDirect);
        try
        {
            ;   // UI-only（HTML 端處理）：EL<TControl>("TfQAMode", "Image1")->Picture->LoadFromFile(sDir);
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfQAMode::ShowTrayDirectIMG");
            filerw::ELMessage("''Path :D:\\HT9045\\IMG\\BMP\\'' be delete");
        }
    }

}

