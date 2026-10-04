// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Monitor 的讀寫檔（HTEditList： → D:\HT9045\system\MVData.ini [Setup] IP／Port、[Specific] HD_Space／HD_Space_Low／HD_Space_Low_Prompt／HD_Closed_Wait／HD_Closed_Wait_Time（讀：LoadTCPIPParament，開機＋開頁；寫：SaveTCPIPParament，Update 鈕））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Monitor/MonitorInterface.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden Monitor/MonitorInterface.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 7 個（header 宣告 40 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "forms/fMonitor.h"
#include "vclcompat/IniFiles.h"
#include "cmydef.h"


// golden Monitor/MonitorInterface.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bShow=false;                        // golden MonitorInterface.h:82（private）—— 本 TU 自己的：網頁沒有關頁事件（golden FormClose :74 設回 false），寫門面的 fMonitor->bShow 會讓 golden MonitorTimerTimer :230「bShow==false 才自動重連」永遠不成立（同 GroundMan 的 fShow）
#define sADDRESS (fMonitor->sADDRESS)   // golden MonitorInterface.h:89 —— 門面 forms/fMonitor.h:260 那一份（MVCtrl 連線用，交 Jimmy 接）
#define iPORT (fMonitor->iPORT)         // golden MonitorInterface.h:90 —— 門面 forms/fMonitor.h:261

using filerw::EL;

// 前置宣告
static void MN_FormShow();
static void MN_LoadTCPIPParament();
static void MN_SaveTCPIPParament();
static void MN_sbMVUpdateClick();

// golden Monitor/MonitorInterface.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void MN_DfmItems()
{
}

// golden Monitor/MonitorInterface.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kMN_ParentOf[][2] = {
    {"cbAfterHandlerTrayFeedMonitor1ClosedVideo", "tsMonitorSpecific"},
    {"cbWhenHDFullAlarm", "tsMonitorSpecific"},
    {"edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime", "tsMonitorSpecific"},
    {"edLowHDSpace", "tsMonitorSpecific"},
    {"edMVAddress", "pnMVSetupPage"},
    {"edMVPort", "pnMVSetupPage"},
    {"edWhenHDFullPrompt", "tsMonitorSpecific"},
    {"pnMVSetupPage", "tsMVSettingPage"},
    {"tsMVSettingPage", "MVPageControl"},
    {"tsMonitorSpecific", "MVPageControl"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void MN_CreateContainerProxies()
{
    EL<TPageControl>("TfMonitor", "MVPageControl");
    EL<TPanel>("TfMonitor", "pnMVSetupPage");
    EL<TTabSheet>("TfMonitor", "tsMVSettingPage");
    EL<TTabSheet>("TfMonitor", "tsMonitorSpecific");
    filerw::ELSetParents("TfMonitor", kMN_ParentOf, (int)(sizeof(kMN_ParentOf) / sizeof(kMN_ParentOf[0])));
}

// golden Monitor/MonitorInterface.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void MN_DfmState()
{
    EL<TEdit>("TfMonitor", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime")->Text = "20";
    EL<TEdit>("TfMonitor", "edLowHDSpace")->Text = "20";
    EL<TEdit>("TfMonitor", "edMVAddress")->Text = "127.0.0.1";
    EL<TEdit>("TfMonitor", "edMVPort")->Text = "1000";
    EL<TEdit>("TfMonitor", "edWhenHDFullPrompt")->Text = "40";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kMN_EnableAll[] = {
    "MVPageControl", "cbAfterHandlerTrayFeedMonitor1ClosedVideo", "cbWhenHDFullAlarm", "pnMVSetupPage", "tsMVSettingPage", "tsMonitorSpecific",
};

// 存檔流程（sbMVUpdateClick、SaveTCPIPParament）讀的替身 7 個 —— FileRW/Monitor.cpp 用它決定頁面必須送哪些值
static const char* const kMN_SaveReads[] = {
    "cbAfterHandlerTrayFeedMonitor1ClosedVideo", "cbWhenHDFullAlarm", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime", "edLowHDSpace", "edMVAddress", "edMVPort",
    "edWhenHDFullPrompt",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void MN_CreateSaveProxies()
{
    EL<TCheckBox>("TfMonitor", "cbAfterHandlerTrayFeedMonitor1ClosedVideo");
    EL<TCheckBox>("TfMonitor", "cbWhenHDFullAlarm");
    EL<TEdit>("TfMonitor", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime");
    EL<TEdit>("TfMonitor", "edLowHDSpace");
    EL<TEdit>("TfMonitor", "edMVAddress");
    EL<TEdit>("TfMonitor", "edMVPort");
    EL<TEdit>("TfMonitor", "edWhenHDFullPrompt");
}

// golden Monitor/MonitorInterface.cpp:47  TfMonitor::FormShow(TObject *Sender)
static void MN_FormShow()
{

    ;   // golden Monitor/MonitorInterface.cpp:49-52 取代：Width／Height／Left／Top：視窗大小與位置（HTML 不用）
#if 0 // GATE (S12-C save) golden Monitor/MonitorInterface.cpp:49-52 -- Width／Height／Left／Top：視窗大小與位置（HTML 不用）
    Width   =610;
    Height  =515;
    Left    =(1024-Width)/2;
    Top     =(768-Height)/2;
#endif // GATE (S12-C save)
    bShow   =true;
    MN_LoadTCPIPParament();

}

// golden Monitor/MonitorInterface.cpp:79  TfMonitor::LoadTCPIPParament()
static void MN_LoadTCPIPParament()
{

    TIniFile *pFile = new TIniFile("D:\\HT9045\\system\\MVData.ini");

    sADDRESS=pFile->ReadString("Setup", "IP", "127.0.0.1");
    iPORT   =pFile->ReadInteger("Setup", "Port", 7000);

    EL<TEdit>("TfMonitor", "edMVAddress")->Text           =sADDRESS;
    EL<TEdit>("TfMonitor", "edMVPort")->Text              =AnsiString(iPORT);
    EL<TCheckBox>("TfMonitor", "cbWhenHDFullAlarm")->Checked  =pFile->ReadBool( "Specific", "HD_Space", false);
    EL<TEdit>("TfMonitor", "edLowHDSpace")->Text          =AnsiString(pFile->ReadInteger("Specific" , "HD_Space_Low" , 0 ) );
    EL<TEdit>("TfMonitor", "edWhenHDFullPrompt")->Text    =AnsiString(pFile->ReadInteger("Specific" , "HD_Space_Low_Prompt" , 0 ) );
    EL<TCheckBox>("TfMonitor", "cbAfterHandlerTrayFeedMonitor1ClosedVideo")->Checked=pFile->ReadBool("Specific", "HD_Closed_Wait", false);
    EL<TEdit>("TfMonitor", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime")->Text=AnsiString(pFile->ReadInteger("Specific", "HD_Closed_Wait_Time", 0));

    ;   // golden Monitor/MonitorInterface.cpp:94-94 取代：MVCtrl->InitialSocket(sADDRESS, iPORT)：MonitorTCPIP 的連線位址（TCP，golden 建構子 :30 new 的 MVCtrl）—— 通訊那一半，交 Jimmy；移植樹門面的 MVCtrl 一直是 NULL（forms/fMonitor.h GATE M-1）。讀進來的 sADDRESS／iPORT 已在門面 fMonitor->sADDRESS／iPORT，接上連線時直接用
#if 0 // GATE (S12-C save) golden Monitor/MonitorInterface.cpp:94-94 -- MVCtrl->InitialSocket(sADDRESS, iPORT)：MonitorTCPIP 的連線位址（TCP，golden 建構子 :30 new 的 MVCtrl）—— 通訊那一半，交 Jimmy；移植樹門面的 MVCtrl 一直是 NULL（forms/fMonitor.h GATE M-1）。讀進來的 sADDRESS／iPORT 已在門面 fMonitor->sADDRESS／iPORT，接上連線時直接用
    MVCtrl->InitialSocket(sADDRESS, iPORT);
#endif // GATE (S12-C save)
    delete pFile;

}

// golden Monitor/MonitorInterface.cpp:98  TfMonitor::SaveTCPIPParament()
static void MN_SaveTCPIPParament()
{
    filerw::ELMark("SaveTCPIPParament");   // FileRW：存檔流程 trace（ack.trace）

    TIniFile *pFile = new TIniFile("D:\\HT9045\\system\\MVData.ini");
    bool bHDCheck=false;
    int iHDCheckLow=0, iHDCheckLowPrompt=0, iHDClosedWaitTime=0;

    sADDRESS    =EL<TEdit>("TfMonitor", "edMVAddress")->Text;
    iPORT       =atoi(EL<TEdit>("TfMonitor", "edMVPort")->Text.c_str());
    bHDCheck    =EL<TCheckBox>("TfMonitor", "cbWhenHDFullAlarm")->Checked;
    iHDCheckLow =atoi(EL<TEdit>("TfMonitor", "edLowHDSpace")->Text.c_str());
    iHDCheckLowPrompt=atoi(EL<TEdit>("TfMonitor", "edWhenHDFullPrompt")->Text.c_str());
    iHDClosedWaitTime=atoi(EL<TEdit>("TfMonitor", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime")->Text.c_str());

    pFile->WriteString("Setup", "IP", sADDRESS);
    pFile->WriteInteger("Setup", "Port", iPORT);
    pFile->WriteBool("Specific", "HD_Space", bHDCheck);
    pFile->WriteInteger("Specific", "HD_Space_Low", iHDCheckLow);
    pFile->WriteInteger("Specific", "HD_Space_Low_Prompt", iHDCheckLowPrompt);
    pFile->WriteBool("Specific", "HD_Closed_Wait", EL<TCheckBox>("TfMonitor", "cbAfterHandlerTrayFeedMonitor1ClosedVideo")->Checked);
    pFile->WriteInteger("Specific", "HD_Closed_Wait_Time", iHDClosedWaitTime);

    ;   // golden Monitor/MonitorInterface.cpp:119-119 取代：MVCtrl->InitialSocket(sADDRESS, iPORT)：MonitorTCPIP 的連線位址（TCP，golden 建構子 :30 new 的 MVCtrl）—— 通訊那一半，交 Jimmy；移植樹門面的 MVCtrl 一直是 NULL（forms/fMonitor.h GATE M-1）。讀進來的 sADDRESS／iPORT 已在門面 fMonitor->sADDRESS／iPORT，接上連線時直接用
#if 0 // GATE (S12-C save) golden Monitor/MonitorInterface.cpp:119-119 -- MVCtrl->InitialSocket(sADDRESS, iPORT)：MonitorTCPIP 的連線位址（TCP，golden 建構子 :30 new 的 MVCtrl）—— 通訊那一半，交 Jimmy；移植樹門面的 MVCtrl 一直是 NULL（forms/fMonitor.h GATE M-1）。讀進來的 sADDRESS／iPORT 已在門面 fMonitor->sADDRESS／iPORT，接上連線時直接用
    MVCtrl->InitialSocket(sADDRESS, iPORT);
#endif // GATE (S12-C save)
    delete pFile;

}

// golden Monitor/MonitorInterface.cpp:123  TfMonitor::sbMVUpdateClick(TObject *Sender)
static void MN_sbMVUpdateClick()
{
    filerw::ELMark("sbMVUpdateClick");   // FileRW：存檔流程 trace（ack.trace）

    MN_SaveTCPIPParament();

}

