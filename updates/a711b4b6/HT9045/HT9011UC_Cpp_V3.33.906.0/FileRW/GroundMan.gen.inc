// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 GroundMan 的讀寫檔（HTEditList： → D:\HT9045\system\GroundMan.ini [System] Alarm_Continuous_Time／Alarm_Occurrences（寫，spbSaveClick）、同檔 [System] UseOffset／UseResetByStart、[Board_<n>_Offset]（讀，ReadGroundOffset；CheckAndReadIniData 缺鍵補寫））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\GroundMan\GroundMan.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden GroundMan\GroundMan.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 57 個（header 宣告 164 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include "MachineType.h"
#include "database.h"
#include "forms/fGroundMan.h"


// golden GroundMan\GroundMan.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;                     // golden GroundMan.h:231 —— 本 TU 自己的（同 ShuttleMove：網頁關頁沒有回報，寫門面的 fGroundMan->fShow 會永遠是 true）
#define iGroundMasterTask (fGroundMan->iGroundMasterTask)   // golden GroundMan.h:230：門面 forms/fGroundMan.h 的那一個（RS232 監測狀態；golden Timer1Timer :572 顯示在 labStatus）
#define bRs232Ok (fGroundMan->bRs232Ok)                     // golden GroundMan.h:208：門面的那一個（唯一設 true 的 Init_GM_RS232 屬 S48，未移植 → 一直是 false）
static bool bReaderOK[4][6];                 // golden GroundMan.h:201（以下到 asGroundVaule 都是 golden 私有成員；本 TU 只做建構子的初值，
static AnsiString sCMD[6];                   // golden GroundMan.h:202  RS232 監測（S48）才讀 —— 移植時與監測放在同一個 TU，見 FileRW/GroundMan.cpp 檔頭）
static AnsiString asRecordValueData[4][8];   // golden GroundMan.h:203
static double dOffset[4][8];                 // golden GroundMan.h:206（ReadGroundOffset 讀 [Board_<n>_Offset]）
static bool bRecordAlarmData[4][8];          // golden GroundMan.h:207
static bool bOpenClose[4][8];                // golden GroundMan.h:209
static AnsiString asShowName[4][8];          // golden GroundMan.h:210（ini 鍵名＋labCH_* 名稱；建構子 :86-124）
static bool bGroundManReset;                 // golden GroundMan.h:213
static bool bGroundManResetByStart;          // golden GroundMan.h:214（[System] UseResetByStart）
static bool bUseOffset;                      // golden GroundMan.h:215（[System] UseOffset）
static int  iUseGndBoard;                    // golden GroundMan.h:217（建構子依 HSys.iGroundManScanPoint：1／3／4）
static bool bGroundManAlarm_FirstTime[4][8]; // golden GroundMan.h:220
static int  iGroundManAlarm_HappenCount[4][8];   // golden GroundMan.h:222
static bool bInternalOhm;                    // golden GroundMan.h:223
static bool bVersion;                        // golden GroundMan.h:224
static int  iMachineOhmRetry;                // golden GroundMan.h:225
static AnsiString asGroundVaule;             // golden GroundMan.h:233（public；golden cObserver.cpp:2036／uLotInfo.cpp:15604 讀，移植樹沒有讀者）

// golden GroundMan\GroundMan.cpp 檔案層級全域（本表單私用，照抄成 static）
static int Alarm_Continuous_Time;   // golden GroundMan\GroundMan.cpp:21
static int Alarm_Occurrences;   // golden GroundMan\GroundMan.cpp:22

using filerw::EL;

// 前置宣告
static void GM_TfGroundMan();
static void GM_FormShow();
static void GM_spbSaveClick();
static void GM_ReadGroundOffset();

// golden GroundMan\GroundMan.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void GM_DfmItems()
{
}

// golden GroundMan\GroundMan.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kGM_ParentOf[][2] = {
    {"btnMaintenanceMode", "Panel1"},
    {"edContinuous_Time", "tsMain"},
    {"edOccurrences", "tsMain"},
    {"gbBoard0", "tsMain"},
    {"gbBoard1", "tsMain"},
    {"gbBoard2", "tsMain"},
    {"gbBoard3", "tsMain"},
    {"labBoardOhrm", "tsMain"},
    {"labBoardOhrm2", "tsMain"},
    {"labBoardOhrm3", "tsMain"},
    {"labBoardOhrm4", "tsMain"},
    {"labBoardVersion", "tsMain"},
    {"labBoardVersion2", "tsMain"},
    {"labBoardVersion3", "tsMain"},
    {"labBoardVersion4", "tsMain"},
    {"labCH_0_0", "gbBoard0"},
    {"labCH_0_1", "gbBoard0"},
    {"labCH_0_2", "gbBoard0"},
    {"labCH_0_3", "gbBoard0"},
    {"labCH_0_4", "gbBoard0"},
    {"labCH_0_5", "gbBoard0"},
    {"labCH_0_6", "gbBoard0"},
    {"labCH_0_7", "gbBoard0"},
    {"labCH_1_0", "gbBoard1"},
    {"labCH_1_1", "gbBoard1"},
    {"labCH_1_2", "gbBoard1"},
    {"labCH_1_3", "gbBoard1"},
    {"labCH_1_4", "gbBoard1"},
    {"labCH_1_5", "gbBoard1"},
    {"labCH_1_6", "gbBoard1"},
    {"labCH_1_7", "gbBoard1"},
    {"labCH_2_0", "gbBoard2"},
    {"labCH_2_1", "gbBoard2"},
    {"labCH_2_2", "gbBoard2"},
    {"labCH_2_3", "gbBoard2"},
    {"labCH_2_4", "gbBoard2"},
    {"labCH_2_5", "gbBoard2"},
    {"labCH_2_6", "gbBoard2"},
    {"labCH_2_7", "gbBoard2"},
    {"labCH_3_0", "gbBoard3"},
    {"labCH_3_1", "gbBoard3"},
    {"labCH_3_2", "gbBoard3"},
    {"labCH_3_3", "gbBoard3"},
    {"labCH_3_4", "gbBoard3"},
    {"labCH_3_5", "gbBoard3"},
    {"labCH_3_6", "gbBoard3"},
    {"labCH_3_7", "gbBoard3"},
    {"labCount_0_0", "gbBoard0"},
    {"labCount_0_1", "gbBoard0"},
    {"labResetByStart", "tsMain"},
    {"labUseOffset", "tsMain"},
    {"labValue_0_0", "gbBoard0"},
    {"labValue_0_1", "gbBoard0"},
    {"led_0_0", "gbBoard0"},
    {"led_0_1", "gbBoard0"},
    {"tsMain", "PageControl1"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void GM_CreateContainerProxies()
{
    EL<TPageControl>("TfGroundMan", "PageControl1");
    EL<TPanel>("TfGroundMan", "Panel1");
    EL<TGroupBox>("TfGroundMan", "gbBoard0");
    EL<TGroupBox>("TfGroundMan", "gbBoard1");
    EL<TGroupBox>("TfGroundMan", "gbBoard2");
    EL<TGroupBox>("TfGroundMan", "gbBoard3");
    EL<TTabSheet>("TfGroundMan", "tsMain");
    filerw::ELSetParents("TfGroundMan", kGM_ParentOf, (int)(sizeof(kGM_ParentOf) / sizeof(kGM_ParentOf[0])));
}

// golden GroundMan\GroundMan.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void GM_DfmState()
{
    EL<TLabel>("TfGroundMan", "labBoardOhrm")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardOhrm2")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardOhrm3")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardOhrm4")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardVersion")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardVersion2")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardVersion3")->Visible = false;
    EL<TLabel>("TfGroundMan", "labBoardVersion4")->Visible = false;
    EL<TEdit>("TfGroundMan", "edContinuous_Time")->Text = "2";
    EL<TEdit>("TfGroundMan", "edOccurrences")->Text = "6";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kGM_EnableAll[] = {
    "PageControl1", "Panel1", "btnMaintenanceMode", "gbBoard0", "gbBoard1", "gbBoard2",
    "gbBoard3", "labBoardOhrm", "labBoardOhrm2", "labBoardOhrm3", "labBoardOhrm4", "labBoardVersion",
    "labBoardVersion2", "labBoardVersion3", "labBoardVersion4", "labCH_0_0", "labCH_0_1", "labCH_0_2",
    "labCH_0_3", "labCH_0_4", "labCH_0_5", "labCH_0_6", "labCH_0_7", "labCH_1_0",
    "labCH_1_1", "labCH_1_2", "labCH_1_3", "labCH_1_4", "labCH_1_5", "labCH_1_6",
    "labCH_1_7", "labCH_2_0", "labCH_2_1", "labCH_2_2", "labCH_2_3", "labCH_2_4",
    "labCH_2_5", "labCH_2_6", "labCH_2_7", "labCH_3_0", "labCH_3_1", "labCH_3_2",
    "labCH_3_3", "labCH_3_4", "labCH_3_5", "labCH_3_6", "labCH_3_7", "labCount_0_0",
    "labCount_0_1", "labResetByStart", "labUseOffset", "labValue_0_0", "labValue_0_1", "tsMain",
};

// 存檔流程（spbSaveClick）讀的替身 2 個 —— FileRW/GroundMan.cpp 用它決定頁面必須送哪些值
static const char* const kGM_SaveReads[] = {
    "edContinuous_Time", "edOccurrences",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void GM_CreateSaveProxies()
{
    EL<TEdit>("TfGroundMan", "edContinuous_Time");
    EL<TEdit>("TfGroundMan", "edOccurrences");
}

// golden GroundMan\GroundMan.cpp:24  TfGroundMan::TfGroundMan(TComponent* Owner)
static void GM_TfGroundMan()
{

    ;   // golden GroundMan\GroundMan.cpp:27-27 取代：ZeroMemory 對 AnsiString 陣列：標準 C++ 是未定義行為（會把字串物件內部清成 0）；本 TU 的 static 陣列本來就是空字串（同 forms/fGroundMan.h DEVIATION D-5）。bool 陣列那三行照 golden
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:27-27 -- ZeroMemory 對 AnsiString 陣列：標準 C++ 是未定義行為（會把字串物件內部清成 0）；本 TU 的 static 陣列本來就是空字串（同 forms/fGroundMan.h DEVIATION D-5）。bool 陣列那三行照 golden
    ZeroMemory(asRecordValueData, sizeof(asRecordValueData));
#endif // GATE (S12-C save)
    ZeroMemory(bRecordAlarmData, sizeof(bRecordAlarmData));
    ZeroMemory(bReaderOK, sizeof(bReaderOK));
    ZeroMemory(bOpenClose, sizeof(bOpenClose));
    ;   // golden GroundMan\GroundMan.cpp:31-31 取代：同上（asShowName 在下面 :86-124 逐格照 golden 設定）
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:31-31 -- 同上（asShowName 在下面 :86-124 逐格照 golden 設定）
    ZeroMemory(asShowName, sizeof(asShowName));
#endif // GATE (S12-C save)

    iGroundMasterTask=1;
    bRs232Ok=false;
    fShow=false;
    bGroundManReset=false;
    bGroundManResetByStart=true;
    bUseOffset=true;

    if(HSys.iGroundManScanPoint==0)  //JerryYang 20210428 : Add 28點接地偵側
    {
        iUseGndBoard=1;
    }
    else if(HSys.iGroundManScanPoint==1)
    {
        iUseGndBoard=3;
    }
    else
    {
        iUseGndBoard=4;
    }

    for(int i=0; i<4; i++)  //JerryYang 20210428 : Add 28點接地偵側
    {
        for(int j=0; j<8; j++)
        {
            bOpenClose[i][j]=true;
        }
    }

    if(HSys.iGroundManScanPoint==0) // 8點偵測
    {
        for(int i=0; i<3; i++)
        {
            for(int j=0; j<8; j++)
            {
                if(i==1 || i==2)
                {
                    bOpenClose[i][j]=false;
                }
            }
        }
    }
    else if(HSys.iGroundManScanPoint==1) // 22點偵測
    {
        bOpenClose[0][0]=false;
        bOpenClose[0][1]=false;
    }
    else if(HSys.iGroundManScanPoint==2) //JerryYang 20210428 : Add 28點接地偵側
    {
        bOpenClose[0][0]=false;
        bOpenClose[0][1]=false;
        bOpenClose[3][6]=false;
        bOpenClose[3][7]=false;
    }
    asShowName[0][0]="InArm";
    asShowName[0][1]="OutArm";
    asShowName[0][2]="Arm1";
    asShowName[0][3]="Arm2";
    asShowName[0][4]="HP1";
    asShowName[0][5]="HP2";
    asShowName[0][6]="SH1";
    asShowName[0][7]="SH2";

    //KaiChen 20200707 ：修改 GroundMan 接點對應位置
    //==>
    asShowName[1][0]="InArm_A";
    asShowName[1][1]="InArm_C";
    asShowName[1][2]="InArm_E";
    asShowName[1][3]="InArm_G";
    asShowName[1][4]="InArm_B";
    asShowName[1][5]="InArm_D";
    asShowName[1][6]="InArm_F";
    asShowName[1][7]="InArm_H";

    asShowName[2][0]="OutArm_A";
    asShowName[2][1]="OutArm_C";
    asShowName[2][2]="OutArm_E";
    asShowName[2][3]="OutArm_G";
    asShowName[2][4]="OutArm_B";
    asShowName[2][5]="OutArm_D";
    asShowName[2][6]="OutArm_F";
    asShowName[2][7]="OutArm_H";
    //<==
    //KaiChen 20200707 ：修改 GroundMan 接點對應位置

    asShowName[3][0]="Loader";  //JerryYang 20210428 : Add 28點接地偵側
    asShowName[3][1]="Empty";
    asShowName[3][2]="Color";
    asShowName[3][3]="Auto1";
    asShowName[3][4]="Auto2";
    asShowName[3][5]="Auto3";
    asShowName[3][6]="Ch6";
    asShowName[3][7]="Ch7";
    asGroundVaule="NA"; //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    sCMD[0]="AlarmSet";     //Sam 20230906 : 新增 GroundMan Msg Log
    sCMD[1]="MachineOhm";
    sCMD[2]="AlarmGet";
    sCMD[3]="BoardOhm";
    sCMD[4]="BoardReset";
    sCMD[5]="BoardVersion";
    bInternalOhm=false;        //KenHsieh 20220803 : GroundMan流程及功能修改
    bVersion=false;            //KenHsieh 20220803 : GroundMan流程及功能修改
    ;   // golden GroundMan\GroundMan.cpp:134-134 取代：Timer1：RS232 監測的計時器（Timer1Timer :551，S48）；替身沒有 Interval。移植樹門面 forms/fGroundMan.cpp 建構子已設 fGroundMan->Timer1->Interval=30
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:134-134 -- Timer1：RS232 監測的計時器（Timer1Timer :551，S48）；替身沒有 Interval。移植樹門面 forms/fGroundMan.cpp 建構子已設 fGroundMan->Timer1->Interval=30
    EL<TControl>("TfGroundMan", "Timer1")->Interval=30;            //KenHsieh 20220803 : GroundMan流程及功能修改
#endif // GATE (S12-C save)
    iMachineOhmRetry=0;
    GM_ReadGroundOffset();

}

// golden GroundMan\GroundMan.cpp:139  TfGroundMan::FormShow(TObject *Sender)
static void GM_FormShow()
{

    ;   // golden GroundMan\GroundMan.cpp:141-142 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:141-142 -- Left／Top：視窗位置（HTML 不用）
    Left=100;   //Steven 20220407 : 避免跑到畫面外
    Top=100;
#endif // GATE (S12-C save)
    TLabel *LabelShowPtr[4][8]={
                                {EL<TLabel>("TfGroundMan", "labCH_0_0"), EL<TLabel>("TfGroundMan", "labCH_0_1"), EL<TLabel>("TfGroundMan", "labCH_0_2"), EL<TLabel>("TfGroundMan", "labCH_0_3"),
                                EL<TLabel>("TfGroundMan", "labCH_0_4"), EL<TLabel>("TfGroundMan", "labCH_0_5"), EL<TLabel>("TfGroundMan", "labCH_0_6"), EL<TLabel>("TfGroundMan", "labCH_0_7")},
                                {EL<TLabel>("TfGroundMan", "labCH_1_0"), EL<TLabel>("TfGroundMan", "labCH_1_1"), EL<TLabel>("TfGroundMan", "labCH_1_2"), EL<TLabel>("TfGroundMan", "labCH_1_3"),
                                EL<TLabel>("TfGroundMan", "labCH_1_4"), EL<TLabel>("TfGroundMan", "labCH_1_5"), EL<TLabel>("TfGroundMan", "labCH_1_6"), EL<TLabel>("TfGroundMan", "labCH_1_7")},
                                {EL<TLabel>("TfGroundMan", "labCH_2_0"), EL<TLabel>("TfGroundMan", "labCH_2_1"), EL<TLabel>("TfGroundMan", "labCH_2_2"), EL<TLabel>("TfGroundMan", "labCH_2_3"),
                                EL<TLabel>("TfGroundMan", "labCH_2_4"), EL<TLabel>("TfGroundMan", "labCH_2_5"), EL<TLabel>("TfGroundMan", "labCH_2_6"), EL<TLabel>("TfGroundMan", "labCH_2_7")},  //JerryYang 20210428 : Add 28點接地偵側
                                {EL<TLabel>("TfGroundMan", "labCH_3_0"), EL<TLabel>("TfGroundMan", "labCH_3_1"), EL<TLabel>("TfGroundMan", "labCH_3_2"), EL<TLabel>("TfGroundMan", "labCH_3_3"),
                                EL<TLabel>("TfGroundMan", "labCH_3_4"), EL<TLabel>("TfGroundMan", "labCH_3_5"), EL<TLabel>("TfGroundMan", "labCH_3_6"), EL<TLabel>("TfGroundMan", "labCH_3_7")}
                                };

    if(HSys.iGroundManScanPoint==0) // 8點偵測
    {
        EL<TGroupBox>("TfGroundMan", "gbBoard0")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard1")->Visible=false;
        EL<TGroupBox>("TfGroundMan", "gbBoard2")->Visible=false;
        EL<TGroupBox>("TfGroundMan", "gbBoard3")->Visible=false;
        EL<TLabel>("TfGroundMan", "labBoardOhrm")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion")->Visible=true;
    }
    else if(HSys.iGroundManScanPoint==1) // 22點偵測
    {
        EL<TGroupBox>("TfGroundMan", "gbBoard0")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard1")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard2")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard3")->Visible=false;
        EL<TLabel>("TfGroundMan", "labCH_0_0")->Visible=false;
        EL<TLabel>("TfGroundMan", "labCH_0_1")->Visible=false;
        EL<TControl>("TfGroundMan", "led_0_0")->Visible=false;
        EL<TControl>("TfGroundMan", "led_0_1")->Visible=false;
        EL<TLabel>("TfGroundMan", "labValue_0_0")->Visible=false;
        EL<TLabel>("TfGroundMan", "labValue_0_1")->Visible=false;
        EL<TLabel>("TfGroundMan", "labCount_0_0")->Visible=false;
        EL<TLabel>("TfGroundMan", "labCount_0_1")->Visible=false;

        EL<TLabel>("TfGroundMan", "labBoardOhrm")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardOhrm2")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardOhrm3")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion2")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion3")->Visible=true;
    }
    else if(HSys.iGroundManScanPoint==2) //JerryYang 20210428 : Add 28點接地偵側
    {
        EL<TGroupBox>("TfGroundMan", "gbBoard0")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard1")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard2")->Visible=true;
        EL<TGroupBox>("TfGroundMan", "gbBoard3")->Visible=true;
//        labCH_0_0->Visible=false;
//        labCH_0_1->Visible=false;
//        led_0_0->Visible=false;
//        led_0_1->Visible=false;
//        labValue_0_0->Visible=false;
//        labValue_0_1->Visible=false;

        EL<TLabel>("TfGroundMan", "labBoardOhrm")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardOhrm2")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardOhrm3")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardOhrm4")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion2")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion3")->Visible=true;
        EL<TLabel>("TfGroundMan", "labBoardVersion4")->Visible=true;
    }

    for(int i=0; i<iUseGndBoard; i++)
    {
        for(int j=0; j<8; j++)
        {
            LabelShowPtr[i][j]->Caption=asShowName[i][j];
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        EL<TEdit>("TfGroundMan", "edContinuous_Time")->Enabled=false;
        EL<TEdit>("TfGroundMan", "edOccurrences")->Enabled=false;
    }

    EL<TSpeedButton>("TfGroundMan", "btnMaintenanceMode")->Visible=(CUSTOMER_CODE==CC_SIGURD_PeiXing && AccessLevel==iDefHonPrecLevel);     //Sam 20250122 : 北興廠新增維修模式，不需要報警要一直顯示阻值

    EL<TPageControl>("TfGroundMan", "PageControl1")->ActivePageIndex=0;
    fShow=true;

}

// golden GroundMan\GroundMan.cpp:1418  TfGroundMan::spbSaveClick(TObject *Sender)
static void GM_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden GroundMan\GroundMan.cpp:1424-1424 取代：Close()：golden A02（Operator 權限）時關表單 → 伺服器端記 closed，頁面要重新開頁才能再存；沒寫檔。golden Close() 還會觸發 FormClose :228 → ReStart()（RS232 重開，S48），網頁不做
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:1424-1424 -- Close()：golden A02（Operator 權限）時關表單 → 伺服器端記 closed，頁面要重新開頁才能再存；沒寫檔。golden Close() 還會觸發 FormClose :228 → ReStart()（RS232 重開，S48），網頁不做
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString sPathName, sFileName;
    AnsiString asOffset="", asTTT="";
    AnsiString aEn="",aCh="";                                                   //Sam 20220105 : GroundMan Alarm Settring 增加保護，避免檢查時間太短無法報警
    sPathName.sprintf("D:\\HT9045\\system");

    sFileName.sprintf("%s\\GroundMan.ini", sPathName);
    MyForceDirectories(sPathName);

    int iAlarm_Continuous_Time=atoi(EL<TEdit>("TfGroundMan", "edContinuous_Time")->Text.c_str());
    int iAlarm_Occurrences=atoi(EL<TEdit>("TfGroundMan", "edOccurrences")->Text.c_str());

    //Sam 20220105 : GroundMan Alarm Settring 增加保護，避免檢查時間太短無法報警
    //==>
    if(iAlarm_Occurrences<=0)
        iAlarm_Occurrences=1;
    int iT=iAlarm_Continuous_Time/iAlarm_Occurrences;
    if(iT<5)
    {
        aEn.sprintf("ContinuousTime/Occurrences Must be more than 5");
        aCh.sprintf("ContinuousTime/Occurrences 必須大於 5");
        filerw::ELMessage(aEn, aCh);
        return;
    }
    //<==
    //Sam 20220105 : GroundMan Alarm Settring 增加保護，避免檢查時間太短無法報警

    /*   //Sam 20220105 : GroundMan Alarm Settring 增加保護，避免檢查時間太短無法報警 Mark
    int iMax_Occurrences=iAlarm_Continuous_Time*3;

    if(iAlarm_Occurrences>iMax_Occurrences)
        iAlarm_Occurrences=iMax_Occurrences;
    */
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        iAlarm_Continuous_Time=15;
        iAlarm_Occurrences=2;
    }

    filerw::ELMark("GM_WriteIniData"); WriteIniData(sFileName, "System",   "Alarm_Continuous_Time",  iAlarm_Continuous_Time);     //Alarm 允許時間   // golden GroundMan\GroundMan.cpp:1466-1466 取代：存檔標記：golden 第一個 WriteIniData（:1466）就落地 —— 之前的 A02（:1420）與「ContinuousTime/Occurrences 必須大於 5」（:1444）都在它前面 return，沒寫檔（savedMark="GM_WriteIniData"）
#if 0 // GATE (S12-C save) golden GroundMan\GroundMan.cpp:1466-1466 -- 存檔標記：golden 第一個 WriteIniData（:1466）就落地 —— 之前的 A02（:1420）與「ContinuousTime/Occurrences 必須大於 5」（:1444）都在它前面 return，沒寫檔（savedMark="GM_WriteIniData"）
    WriteIniData(sFileName, "System",   "Alarm_Continuous_Time",  iAlarm_Continuous_Time);     //Alarm 允許時間
#endif // GATE (S12-C save)
    WriteIniData(sFileName, "System",   "Alarm_Occurrences",      iAlarm_Occurrences);         //Alarm 允許次數
    GM_ReadGroundOffset();

}

// golden GroundMan\GroundMan.cpp:1573  TfGroundMan::ReadGroundOffset()
static void GM_ReadGroundOffset()
{

    AnsiString sPathName, sFileName;
    AnsiString asOffset="", asTTT="";

    sPathName.sprintf("D:\\HT9045\\system");
    MyForceDirectories(sPathName);
    sFileName.sprintf("%s\\GroundMan.ini", sPathName);

    bUseOffset=CheckAndReadIniData(sFileName, "System",    "UseOffset",    true);

    if(bUseOffset==true)
        EL<TLabel>("TfGroundMan", "labUseOffset")->Caption="Use Offset：ON";
    else
        EL<TLabel>("TfGroundMan", "labUseOffset")->Caption="Use Offset：OFF";

    bGroundManResetByStart=CheckAndReadIniData(sFileName, "System",    "UseResetByStart",    true);

    if(bGroundManResetByStart==true)
        EL<TLabel>("TfGroundMan", "labResetByStart")->Caption="Use Reset：ON";
    else
        EL<TLabel>("TfGroundMan", "labResetByStart")->Caption="Use Reset：OFF";

    //Ifor 20201109 add: Ground Man Alarm 計數
    //==>
    Alarm_Continuous_Time   =CheckAndReadIniData(sFileName, "System",    "Alarm_Continuous_Time",   15);  //Sam 20220104 :  2S > 15S    //Alarm 允許時間
    Alarm_Occurrences       =CheckAndReadIniData(sFileName, "System",    "Alarm_Occurrences",   2);   //Sam 20220104 :  6 > 2           //Alarm 允許時間
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        Alarm_Continuous_Time=15;
        Alarm_Occurrences=2;
    }
    EL<TEdit>("TfGroundMan", "edContinuous_Time")->Text=Alarm_Continuous_Time;
    EL<TEdit>("TfGroundMan", "edOccurrences")->Text=Alarm_Occurrences;
    //<==
    //Ifor 20201109 add: Ground Man Alarm 計數

    for(int i=0; i<iUseGndBoard; i++)
    {
        for(int j=0; j<8; j++)
        {
            asTTT.sprintf("Board_%d_Offset", i+1);
//            asOffset.sprintf("%d_%d", i+1, j+1);

            dOffset[i][j]=CheckAndReadIniData(sFileName, asTTT,    asShowName[i][j],    0.0);
            bGroundManAlarm_FirstTime[i][j]=true; //初始化
            iGroundManAlarm_HappenCount[i][j]=0;  //初始化
        }
    }

}

