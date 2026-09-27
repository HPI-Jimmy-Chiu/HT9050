// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 AutoCalSuckZ 的讀寫檔（HTEditList：elData → D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data [ArmSuckZAuto] 22 鍵（讀 LoadAutoCalSuckZ＝ReadEditTextFromFile；寫 SaveAutoCalSuckZ＝SaveEditTextToFile））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ProductionInfo/ProductionInfo.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden ProductionInfo/ProductionInfo.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 27 個（header 宣告 154 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "common.h"


// golden ProductionInfo/ProductionInfo.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static HTEditList *elData=nullptr;   // golden ProductionInfo.h:552（public 成員）。本 TU 自己的：golden 表單外沒有人碰 elData
static AnsiString GetProInfoFilePath(){return "D:\\HT9045\\system\\ProductionInfo";}   // golden ProductionInfo.h:487（inline 成員，逐字）—— golden 寫死的系統路徑，不跟配方、不經 DataPath

using filerw::EL;

// 前置宣告
static void ACZ_TfProductionInfo();
static void ACZ_DoIniDataToForm();
static void ACZ_InitAutoCalSuckZ();
static void ACZ_SaveAutoCalSuckZ();
static void ACZ_LoadAutoCalSuckZ();
static void ACZ_btnAutoCalSuckZSaveClick();

// golden ProductionInfo/ProductionInfo.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void ACZ_DfmItems()
{
}

// golden ProductionInfo/ProductionInfo.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kACZ_ParentOf[][2] = {
    {"GroupBox12", "Panel13"},
    {"GroupBox13", "Panel10"},
    {"Panel10", "tsOutArmSuckZ"},
    {"Panel11", "tsOutArmSuckZ"},
    {"Panel12", "tsInArmSuckZ"},
    {"Panel13", "tsInArmSuckZ"},
    {"cbInarmSuckZAutoEnable", "Panel12"},
    {"cbOutarmSuckZAutoEnable", "Panel11"},
    {"edInarmSearchStartZ", "gbInarmSearchStartZ"},
    {"edInarmTestAreaX", "gbTestAreaInarm"},
    {"edInarmTestAreaY", "gbTestAreaInarm"},
    {"edOutarmSearchStartZ", "gbOutarmSearchStartZ"},
    {"edOutarmTestAreaX", "gbTestAreaOutarm"},
    {"edOutarmTestAreaY", "gbTestAreaOutarm"},
    {"gbInarmSearchStartZ", "Panel13"},
    {"gbOutarmSearchStartZ", "Panel10"},
    {"gbTestAreaInarm", "Panel13"},
    {"gbTestAreaOutarm", "Panel10"},
    {"pcInOutArmSuckZ", "tsCalibrateSuckZHeight"},
    {"setEditZ1A", "GroupBox12"},
    {"setEditZ1B", "GroupBox12"},
    {"setEditZ1C", "GroupBox12"},
    {"setEditZ1D", "GroupBox12"},
    {"setEditZ1E", "GroupBox12"},
    {"setEditZ1F", "GroupBox12"},
    {"setEditZ1G", "GroupBox12"},
    {"setEditZ1H", "GroupBox12"},
    {"setEditZ2A", "GroupBox13"},
    {"setEditZ2B", "GroupBox13"},
    {"setEditZ2C", "GroupBox13"},
    {"setEditZ2D", "GroupBox13"},
    {"setEditZ2E", "GroupBox13"},
    {"setEditZ2F", "GroupBox13"},
    {"setEditZ2G", "GroupBox13"},
    {"setEditZ2H", "GroupBox13"},
    {"tsCalibrateSuckZHeight", "PageControl1"},
    {"tsInArmSuckZ", "pcInOutArmSuckZ"},
    {"tsOutArmSuckZ", "pcInOutArmSuckZ"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void ACZ_CreateContainerProxies()
{
    EL<TGroupBox>("TfProductionInfo", "GroupBox12");
    EL<TGroupBox>("TfProductionInfo", "GroupBox13");
    EL<TPageControl>("TfProductionInfo", "PageControl1");
    EL<TPanel>("TfProductionInfo", "Panel10");
    EL<TPanel>("TfProductionInfo", "Panel11");
    EL<TPanel>("TfProductionInfo", "Panel12");
    EL<TPanel>("TfProductionInfo", "Panel13");
    EL<TGroupBox>("TfProductionInfo", "gbInarmSearchStartZ");
    EL<TGroupBox>("TfProductionInfo", "gbOutarmSearchStartZ");
    EL<TGroupBox>("TfProductionInfo", "gbTestAreaInarm");
    EL<TGroupBox>("TfProductionInfo", "gbTestAreaOutarm");
    EL<TPageControl>("TfProductionInfo", "pcInOutArmSuckZ");
    EL<TTabSheet>("TfProductionInfo", "tsCalibrateSuckZHeight");
    EL<TTabSheet>("TfProductionInfo", "tsInArmSuckZ");
    EL<TTabSheet>("TfProductionInfo", "tsOutArmSuckZ");
    filerw::ELSetParents("TfProductionInfo", kACZ_ParentOf, (int)(sizeof(kACZ_ParentOf) / sizeof(kACZ_ParentOf[0])));
}

// golden ProductionInfo/ProductionInfo.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void ACZ_DfmState()
{
    EL<TControl>("TfProductionInfo", "tm_IPSCControl")->Enabled = false;
    EL<TControl>("TfProductionInfo", "tm_PI")->Enabled = false;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kACZ_EnableAll[] = {
    "GroupBox12", "GroupBox13", "PageControl1", "Panel10", "Panel11", "Panel12",
    "Panel13", "cbInarmSuckZAutoEnable", "cbOutarmSuckZAutoEnable", "gbInarmSearchStartZ", "gbOutarmSearchStartZ", "gbTestAreaInarm",
    "gbTestAreaOutarm", "pcInOutArmSuckZ", "tsCalibrateSuckZHeight", "tsInArmSuckZ", "tsOutArmSuckZ",
};

// 存檔流程（btnAutoCalSuckZSaveClick、SaveAutoCalSuckZ）讀的替身 0 個 —— FileRW/AutoCalSuckZ.cpp 用它決定頁面必須送哪些值
static const char* const kACZ_SaveReads[] = {
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void ACZ_CreateSaveProxies()
{
}

// golden ProductionInfo/ProductionInfo.cpp:52  TfProductionInfo::TfProductionInfo(TComponent* Owner)
static void ACZ_TfProductionInfo()
{

#if 0 // GATE (S12-C save) golden ProductionInfo/ProductionInfo.cpp:55-65 -- OEE／IPSC 成員（bShow、bRecFromErrorNote、CompareDataList、bAgreeExportConfig、sIPSCFlag_*、bIPSC*、asTempstring、cDynaThres）：TfProductionInfo 的 N14 OEE（超豐）功能，不是 AutoCalSuckZ.Data 讀寫段；成員在門面 forms/fProductionInfo.h（bShow 的初值 false 已由門面的預設成員初始化給），其餘 OEE 物件要額外移植（RULINGS 第 25 條擋並註記）
    bShow=false;                                                                //Sam 20200225 : OEE 功能防呆保護提示顯示在畫面，不要用 MessageBox。
    bRecFromErrorNote=false;                                                    //Jimmychiu 20220901 加入初始值
    EL<TControl>("TfProductionInfo", "CompareDataList")=new TList;
    bAgreeExportConfig=false;
    sIPSCFlag_ShowMessageAndStop="";
    bIPSCFlag_ShowMessageAndStop=false;
    bIPSCFlag_NeedPause=false;
    bIPSCFlag_NeedStart=false;
    asTempstring="";
    bIPSCPause=false;
    cDynaThres=new cDynamicMultiContinualPassBinBySocket;
#endif // GATE (S12-C save)
    ZeroMemory(iInArmZHeightDiff,       sizeof(iInArmZHeightDiff));
    ZeroMemory(iOutArmZHeightDiff,      sizeof(iOutArmZHeightDiff));
    bEnableInarmSuckZAuto=false;
    bEnableOutarmSuckZAuto=false;
    elData     =new HTEditList;
    ACZ_InitAutoCalSuckZ();
    iInArmSearchStartZ=0;
    iOutArmSearchStartZ=0;
#if 0 // GATE (S12-C save) golden ProductionInfo/ProductionInfo.cpp:74-80 -- bN14_21_UsedGDK04／InitialOEECount()×2（golden :842，清 OEE 計時與 jam/service 計數）／iAutoClean_IntervalContact_Server／bN14_21_SetUpConfig_Success：同上，N14 OEE 段，門面沒有 InitialOEECount（要額外移植）
    bN14_21_UsedGDK04=false;
    InitialOEECount();

    iAutoClean_IntervalContact_Server=0;
    InitialOEECount();

    bN14_21_SetUpConfig_Success=false;
#endif // GATE (S12-C save)

}

// golden ProductionInfo/ProductionInfo.cpp:83  TfProductionInfo::DoIniDataToForm()
static void ACZ_DoIniDataToForm()
{

    if(CosFunction.bOEEFunction==false)
        return;
#if 0 // GATE (S12-C save) golden ProductionInfo/ProductionInfo.cpp:87-144 -- OEE 本體（CopyIPSCback2IPSC 讀 IPSC ini、LoadSetting 讀 OEE 設定、複製 7z.exe 到 D:\HT9045、LoadMOInformation、ESDForm、tm_PI／tm_IPSCControl 計時器、ACM 訊息、LoadConfigFromServer FTP）：N14 超豐客戶功能，要額外移植才編得過（RULINGS 第 25 條）。上面第一行 `if(CosFunction.bOEEFunction==false) return;` 照 golden 留著 —— 它同時決定下面 AutoCalSuckZ.Data 讀不讀
    CopyIPSCback2IPSC();                                                        //JimmyChiu 20211227 : 讀取IPSC ini file(第一次執行讀取最後一次ini檔)
    ACM_WriteMsgAndCallExe_sMessage=AnsiString("");                             //Jimmychiu 20220901 降低ACM交握影響主程序效能
    iControlBinTimerCnt                     =0;                                 //Sam 20200525 : Control Bin
    bIsControlBinCounting                   =false;                             //Sam 20200525 : Control Bin
    bNeedLotEndAfterCleanOut=false;
    //AI(ht9045-v912) 20260915: CASE-20260915-002 依 V899 註解 —— FormClose 每次關閉本表單都會呼叫 DoIniDataToForm()，這行會把 ClickPause() 剛清掉的旗標重新武裝，使 N14_PauseIntervalTimeSec 間隔失效
//  bPauseTimeNeedSaveAndUpdateOEEFiles=true;                                   //Mylin 20170705 Add OEE Report Field
    dOffsetContactForce=0.0;
    sPI_STime="";                                                               //Jimmychiu 20240119 : ScheduleName = Mo+Flow
    bStartNeedSaveAndUpdateOEEFiles=true;                                       //Sam 20171015 : Bug Fix
    OEE_SetHandlerID(IniConfig.SocketHandlerID);                                //Sam 20170801 HT-7045 變數改為 HT-9045 變數
    //AI(ht9045-v912) 20260915: CASE-20260915-002 依 V899 註解 —— InitialOEECount() 內部也會武裝同一旗標（只註解上面那行無效），且會清掉該批 OEE 計時與 jam/service 計數；建構子已呼叫過
//  InitialOEECount();
    LoadSetting();
    sLocalMOFullFileName="";                                                    //Sam 20190227 : Try Catch Mo Download SetupFile
    if(FileExists("d:\\HT9045\\7z.exe")==false)
    {
        CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false);
    }
    bFTPError=false;
    bCycleTimeUp=false;
    InitialIPSCControlCount();
    SetNextRecordDateTime();
    if(_bOEEStartLotSuccess==true)
    {
        #ifndef SOFT_SIMULTE
        if(LoadMOInformation(true)==false)
        {
            AnsiString sMsg="Production Start Lot Fail!#Please Check Load MO File";
            filerw::ELMessage(sMsg);
        }
        #endif
        //Mylin 20170331 (Joye,Hungyeh) Modify ESD Control Save To File and Show
        //==>
        AnsiString sSaveESDControlMsgFilePath=sProductionInfoFilePath + "\\" + sLoadMO_MO + "\\" + _sOEE_DirectoryName + "\\"; //Mylin 20170517 Add Load and Check MO Information
        AnsiString sSaveESDControlMsgFileName=_sOEE_MO + "_ESDControlMsg.csv";
        ESDForm->SetSaveESDControlMsgToFile(sSaveESDControlMsgFilePath, sSaveESDControlMsgFileName);
        //<==
        //Mylin 20170331 (Joye,Hungyeh) Modify ESD Control Save To File and Show
    }
    else
    {
        ESDForm->SetSaveESDControlMsgToFile("", "");
    }
    EL<TControl>("TfProductionInfo", "tm_PI")->Enabled=true;
    EL<TControl>("TfProductionInfo", "tm_IPSCControl")->Enabled=true;
    tIPSCTimer.LatchCycleTimeSec(true);
    tIPSCCycleTimer.LatchCycleTimeSec(true);
    tCheckFTPFileTimer.LatchCycleTimeSec(true);                                 //Sam 20200305 : 增加 FTP 檔案檢查，檢查到 VIE_STOP.txt 就執行 OneCycle
    tControlBinTimer.LatchCycleTimeSec(true);                                   //Sam 20200525 : Control Bin
    #ifndef SOFT_SIMULTE
    if(IniConfig.bN14_14_AlarmCtrlMachine)                                      //Sam 20210412 : 超豐新增軟體開啟時要丟訊息
        fProductionInfo->SetACM_WriteMsgAndCallExe_sMessage("Start");           //Jimmychiu 20220901 降低ACM交握影響主程序效能
    if(IniConfig.bN14_21_ConfigUpdateFromServerExport)
    {
        LoadConfigFromServer();
    }
    #endif
#endif // GATE (S12-C save)
    elData->InitialDataToEdit();
    ACZ_LoadAutoCalSuckZ();

}

// golden ProductionInfo/ProductionInfo.cpp:6019  TfProductionInfo::InitAutoCalSuckZ()
static void ACZ_InitAutoCalSuckZ()
{

    bool bShow=true, bEnable=true, bReadFromFile=true;//, bFixedValue=false, bDisable=false;
    InArmAutoCalSuckZPoint.Clear();
    OutArmAutoCalSuckZPoint.Clear();
    //in arm
    elData->Add(EL<TCheckBox>("TfProductionInfo", "cbInarmSuckZAutoEnable"),     &bEnableInarmSuckZAuto,       ECBool,     "ArmSuckZAuto", "bEnableInarmSuckZAuto",  bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1A"),                 &iInArmZHeightDiff[0][0],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff0_0",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1B"),                 &iInArmZHeightDiff[1][0],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff1_0",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1C"),                 &iInArmZHeightDiff[0][1],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff0_1",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1D"),                 &iInArmZHeightDiff[1][1],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff1_1",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1E"),                 &iInArmZHeightDiff[0][2],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff0_2",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1F"),                 &iInArmZHeightDiff[1][2],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff1_2",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1G"),                 &iInArmZHeightDiff[0][3],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff0_3",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ1H"),                 &iInArmZHeightDiff[1][3],     ECInteger,  "ArmSuckZAuto", "iInArmZHeightDiff1_3",   bShow, bEnable, bReadFromFile, 0,     false, -500, 500);

    elData->Add(EL<TEdit>("TfProductionInfo", "edInarmTestAreaX"),           &InArmAutoCalSuckZPoint.X,    ECInteger,  "ArmSuckZAuto", "InArmAutoCalSuckZPointX",  bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "edInarmTestAreaY"),           &InArmAutoCalSuckZPoint.Y,    ECInteger,  "ArmSuckZAuto", "InArmAutoCalSuckZPointY",  bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "edInarmSearchStartZ"),        &iInArmSearchStartZ,          ECInteger,  "ArmSuckZAuto", "iInArmSearchStartZ",     bShow, bEnable, bReadFromFile, 0);

    //out arm
    elData->Add(EL<TCheckBox>("TfProductionInfo", "cbOutarmSuckZAutoEnable"),    &bEnableOutarmSuckZAuto,      ECBool,     "ArmSuckZAuto", "bEnableOutarmSuckZAuto", bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2A"),                 &iOutArmZHeightDiff[0][0],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff0_0",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2B"),                 &iOutArmZHeightDiff[1][0],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff1_0",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2C"),                 &iOutArmZHeightDiff[0][1],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff0_1",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2D"),                 &iOutArmZHeightDiff[1][1],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff1_1",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2E"),                 &iOutArmZHeightDiff[0][2],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff0_2",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2F"),                 &iOutArmZHeightDiff[1][2],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff1_2",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2G"),                 &iOutArmZHeightDiff[0][3],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff0_3",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);
    elData->Add(EL<TEdit>("TfProductionInfo", "setEditZ2H"),                 &iOutArmZHeightDiff[1][3],    ECInteger,  "ArmSuckZAuto", "iOutArmZHeightDiff1_3",  bShow, bEnable, bReadFromFile, 0,     false, -500, 500);

    elData->Add(EL<TEdit>("TfProductionInfo", "edOutarmTestAreaX"),          &OutArmAutoCalSuckZPoint.X,   ECInteger,  "ArmSuckZAuto", "OutArmAutoCalSuckZPointX",  bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "edOutarmTestAreaY"),          &OutArmAutoCalSuckZPoint.Y,   ECInteger,  "ArmSuckZAuto", "OutArmAutoCalSuckZPointY",  bShow, bEnable, bReadFromFile, 0);
    elData->Add(EL<TEdit>("TfProductionInfo", "edOutarmSearchStartZ"),       &iOutArmSearchStartZ,         ECInteger,  "ArmSuckZAuto", "iOutArmSearchStartZ",    bShow, bEnable, bReadFromFile, 0);

}

// golden ProductionInfo/ProductionInfo.cpp:6055  TfProductionInfo::SaveAutoCalSuckZ()
static void ACZ_SaveAutoCalSuckZ()
{
    filerw::ELMark("SaveAutoCalSuckZ");   // FileRW：存檔流程 trace（ack.trace）

    AnsiString szDir=GetProInfoFilePath();
    MyForceDirectories(szDir);
    szDir+="\\";
    filerw::ELMark("ACZ_SaveEditTextToFile"); elData->SaveEditTextToFile(szDir, "AutoCalSuckZ.Data");   // golden ProductionInfo/ProductionInfo.cpp:6060-6060 取代：存檔標記：golden 唯一的寫檔點（之前的 MyForceDirectories 只建資料夾）。之後有頁面時當 PageDesc.savedMark 用
#if 0 // GATE (S12-C save) golden ProductionInfo/ProductionInfo.cpp:6060-6060 -- 存檔標記：golden 唯一的寫檔點（之前的 MyForceDirectories 只建資料夾）。之後有頁面時當 PageDesc.savedMark 用
    elData->SaveEditTextToFile(szDir, "AutoCalSuckZ.Data");
#endif // GATE (S12-C save)

}

// golden ProductionInfo/ProductionInfo.cpp:6063  TfProductionInfo::LoadAutoCalSuckZ()
static void ACZ_LoadAutoCalSuckZ()
{

    AnsiString szDir=GetProInfoFilePath();
    MyForceDirectories(szDir);
    szDir+="\\";
    elData->ReadEditTextFromFile(szDir, "AutoCalSuckZ.Data");

}

// golden ProductionInfo/ProductionInfo.cpp:6135  TfProductionInfo::btnAutoCalSuckZSaveClick(TObject *Sender)
static void ACZ_btnAutoCalSuckZSaveClick()
{
    filerw::ELMark("btnAutoCalSuckZSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    ACZ_SaveAutoCalSuckZ();
    ACZ_LoadAutoCalSuckZ();
    filerw::ELMark("closed");   // golden ProductionInfo/ProductionInfo.cpp:6139-6139 取代：Close()：golden 存完、重讀之後關表單（沒有條件）；網頁端記 closed。golden Close() 觸發 FormClose :4684（bShow=false; DoIniDataToForm();「離開頁面要刷新一次, 避免誤存檔」）—— 上一行剛 LoadAutoCalSuckZ 過，DoIniDataToForm 在 OEE 機台只會再讀同一個檔一次（其餘是擋掉的 OEE 本體），非 OEE 機台第一行就 return；資料上等於沒事，不重跑
#if 0 // GATE (S12-C save) golden ProductionInfo/ProductionInfo.cpp:6139-6139 -- Close()：golden 存完、重讀之後關表單（沒有條件）；網頁端記 closed。golden Close() 觸發 FormClose :4684（bShow=false; DoIniDataToForm();「離開頁面要刷新一次, 避免誤存檔」）—— 上一行剛 LoadAutoCalSuckZ 過，DoIniDataToForm 在 OEE 機台只會再讀同一個檔一次（其餘是擋掉的 OEE 本體），非 OEE 機台第一行就 return；資料上等於沒事，不重跑
    Close();
#endif // GATE (S12-C save)

}

