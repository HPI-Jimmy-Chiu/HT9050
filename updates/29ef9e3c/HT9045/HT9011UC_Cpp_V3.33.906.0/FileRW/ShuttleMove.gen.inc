// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 ShuttleMove 的讀寫檔（HTEditList： → system\teach.ini（fTeach->SaveFile(true)：TECH_PARA 十個 Shuttle 教導點）、<recipe>\HandlerCondition.Data [Shuttle] InSH1SenICAddPos／InSH2SenICAddPos（寫）＋InShtLtcPercent（只讀））。
//  來源：golden D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\ShuttleMove.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden ShuttleMove.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 30 個（header 宣告 56 個）。
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
#include "forms/fShuttleMove.h"
#include "Motor/mymotor.h"

void SetTechDataToProd();              // cinitial.h:183（golden cinitial.h:32；cinitial.h 其餘宣告本 TU 不用）
void InitShuttleThreadParameter();     // cinitial.h:143（golden sbUpdateClick :1992「Teach做完要重新Init一次」）
int  GetSHCHKPos(int iSite, int iCenterBase);   // cinitial.h:34（ShowShuttleSensorPosition 的格子數字）
void FileRW_Teach_SaveFile(bool bSaveByTeach);  // FileRW/Teach.cpp：golden TfTeach::SaveFile（uteach.cpp:4924，V912 :4939）
int  FileRW_InArmSuckShtCol();           // FileRW/_KitSuck.cpp：golden InArmSuck.iShtCol（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 衝突）
// golden BarCode.h:83-91 enum eMulti2DType 的三個值（ShowShuttleSensorPosition :2060-2062 用；移植樹只在 BarCode/BarCode_Shuttle2_CCDScan.h:197，那支 header 帶進 TfBarCode 狀態，本 TU 不 include）
static const int e1x2In1CCD=0, e2x2In1CCD=3, e2x2In2CCD=4;
// golden TTMyTray（HTray.h）的 C 路替身：只存 XItem 與 SetCellNumber 的數字（x＝欄、y＝列；DFM 四個格子 YItem 2 或 4、XItem ≤ 16）
struct SM_TTMyTray {
    int  XItem=0;
    int  Cell[16][4]={{0}};
    bool Has[16][4]={{false}};
    bool bPortGap=false;              // golden 填格子的來源沒有移植（mtInSHBarCodePos：fBarCode->GetMovePos）
    void SetCellNumber(int x, int y, int v) { if(x>=0 && x<16 && y>=0 && y<4) { Cell[x][y]=v; Has[x][y]=true; } }
    void Reset() { XItem=0; bPortGap=false; for(int x=0; x<16; x++) for(int y=0; y<4; y++) { Cell[x][y]=0; Has[x][y]=false; } }
};

// golden ShuttleMove.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool fShow=false;   // golden ShuttleMove.h:96 —— 本 TU 自己的。不寫門面 fShuttleMove->fShow：網頁關頁沒有回報（golden FormClose :2135 才清），寫了就永遠是 true，會讓移植樹讀它的地方（forms/fShuttleMove.h 的說明）改走「操作員開著這張表單」的分支
static int iMoveToTarget=-1;      // golden ShuttleMove.h:91（建構子 :64 設 -1）
static DWORD LastClickTime=0;     // golden ShuttleMove.h:92
static int LastClickX=-1;         // golden ShuttleMove.h:93
static int LastClickY=-1;         // golden ShuttleMove.h:93
#define bShuttleRetry (fShuttleMove->bShuttleRetry)   // golden ShuttleMove.h:110：門面的那一個（golden ckernel.cpp:142 讀；FormShow :87 清成 false＝它的常態值）
static SM_TTMyTray SM_mtedInSHSen7DetectPos;    // golden ShuttleMove.h:49 TTMyTray（見 decls）
static SM_TTMyTray SM_mtInSHBarCodePos;         // golden ShuttleMove.h:50
static SM_TTMyTray SM_mtOutSHDetectPos;         // golden ShuttleMove.h:51
static SM_TTMyTray SM_mtedInSHSenICDetectPos;   // golden ShuttleMove.h:68

using filerw::EL;

// 前置宣告
static void SM_TfShuttleMove();
static void SM_FormShow();
static void SM_sbUpdateClick();
static void SM_ShowShuttleSensorPosition();

// golden ShuttleMove.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void SM_DfmItems()
{
}

// golden ShuttleMove.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kSM_ParentOf[][2] = {
    {"Panel2", "Panel1"},
    {"Panel3", "Panel1"},
    {"btRetry", "gbOutFiberCheck"},
    {"btShu1Left", "gbShuttleMove"},
    {"btShu1Right", "gbShuttleMove"},
    {"btShu2Left", "gbShuttleMove"},
    {"btShu2Right", "gbShuttleMove"},
    {"btnTStep", "gbBarCode"},
    {"edInSH1BarCodePos", "gbBarCode"},
    {"edInSH1Sen7DetectPos", "gbInFiberCheck"},
    {"edInSH1SenICDetectPos", "gbInFiberCheckShtSnLct"},
    {"edInSH1SenICDetectZPos", "gbInFiberCheckShtSnLct"},
    {"edInSH2BarCodePos", "gbBarCode"},
    {"edInSH2Sen7DetectPos", "gbInFiberCheck"},
    {"edInSH2SenICDetectPos", "gbInFiberCheckShtSnLct"},
    {"edInSH2SenICDetectZPos", "gbInFiberCheckShtSnLct"},
    {"edOutSH1OneRowDetectPos", "gbOutFiberCheck"},
    {"edOutSH1ZDetectPos", "gbOutFiberCheck"},
    {"edOutSH2OneRowDetectPos", "gbOutFiberCheck"},
    {"edOutSH2ZDetectPos", "gbOutFiberCheck"},
    {"gbBarCode", "Panel3"},
    {"gbInFiberCheck", "Panel3"},
    {"gbInFiberCheckShtSnLct", "Panel3"},
    {"gbOutFiberCheck", "Panel3"},
    {"gbScanOutShuttle", "Panel2"},
    {"gbShuttleMove", "Panel2"},
    {"meShuttleMaintain", "Panel1"},
    {"mtInSHBarCodePos", "gbBarCode"},
    {"mtOutSHDetectPos", "gbOutFiberCheck"},
    {"mtedInSHSen7DetectPos", "gbInFiberCheck"},
    {"mtedInSHSenICDetectPos", "gbInFiberCheckShtSnLct"},
    {"palSh1Encoder", "gbShuttleMove"},
    {"palSh2Encoder", "gbShuttleMove"},
    {"sbSensorLatch", "Panel2"},
    {"sbShuttleSensor", "Panel2"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void SM_CreateContainerProxies()
{
    EL<TPanel>("TfShuttleMove", "Panel1");
    EL<TPanel>("TfShuttleMove", "Panel2");
    EL<TPanel>("TfShuttleMove", "Panel3");
    EL<TGroupBox>("TfShuttleMove", "gbBarCode");
    EL<TGroupBox>("TfShuttleMove", "gbInFiberCheck");
    EL<TGroupBox>("TfShuttleMove", "gbInFiberCheckShtSnLct");
    EL<TGroupBox>("TfShuttleMove", "gbOutFiberCheck");
    EL<TGroupBox>("TfShuttleMove", "gbShuttleMove");
    filerw::ELSetParents("TfShuttleMove", kSM_ParentOf, (int)(sizeof(kSM_ParentOf) / sizeof(kSM_ParentOf[0])));
}

// golden ShuttleMove.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void SM_DfmState()
{
    EL<TButton>("TfShuttleMove", "btRetry")->Visible = false;
    EL<TButton>("TfShuttleMove", "btnTStep")->Visible = false;
    EL<TSpeedButton>("TfShuttleMove", "sbSensorLatch")->Visible = false;
    EL<TSpeedButton>("TfShuttleMove", "sbShuttleSensor")->Visible = false;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kSM_EnableAll[] = {
    "Panel1", "Panel2", "Panel3", "btRetry", "btShu1Left", "btShu1Right",
    "btShu2Left", "btShu2Right", "btnTStep", "gbBarCode", "gbInFiberCheck", "gbInFiberCheckShtSnLct",
    "gbOutFiberCheck", "gbScanOutShuttle", "gbShuttleMove", "palSh1Encoder", "palSh2Encoder", "sbSensorLatch",
    "sbShuttleSensor",
};

// 存檔流程（sbUpdateClick）讀的替身 12 個 —— FileRW/ShuttleMove.cpp 用它決定頁面必須送哪些值
static const char* const kSM_SaveReads[] = {
    "edInSH1BarCodePos", "edInSH1Sen7DetectPos", "edInSH1SenICDetectPos", "edInSH1SenICDetectZPos", "edInSH2BarCodePos", "edInSH2Sen7DetectPos",
    "edInSH2SenICDetectPos", "edInSH2SenICDetectZPos", "edOutSH1OneRowDetectPos", "edOutSH1ZDetectPos", "edOutSH2OneRowDetectPos", "edOutSH2ZDetectPos",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void SM_CreateSaveProxies()
{
    EL<TEdit>("TfShuttleMove", "edInSH1BarCodePos");
    EL<TEdit>("TfShuttleMove", "edInSH1Sen7DetectPos");
    EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectPos");
    EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectZPos");
    EL<TEdit>("TfShuttleMove", "edInSH2BarCodePos");
    EL<TEdit>("TfShuttleMove", "edInSH2Sen7DetectPos");
    EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectPos");
    EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectZPos");
    EL<TEdit>("TfShuttleMove", "edOutSH1OneRowDetectPos");
    EL<TEdit>("TfShuttleMove", "edOutSH1ZDetectPos");
    EL<TEdit>("TfShuttleMove", "edOutSH2OneRowDetectPos");
    EL<TEdit>("TfShuttleMove", "edOutSH2ZDetectPos");
}

// golden ShuttleMove.cpp:59  TfShuttleMove::TfShuttleMove(TComponent* Owner)
static void SM_TfShuttleMove()
{

    fShow=false;
    bShuttleRetry=false;
    iMoveToTarget=-1;

}

// golden ShuttleMove.cpp:67  TfShuttleMove::FormShow(TObject *Sender)
static void SM_FormShow()
{

    fShow=true;
    ;   // golden ShuttleMove.cpp:70-71 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:70-71 -- Left／Top：視窗位置（HTML 不用）
    Left=100;
    Top =20;
#endif // GATE (S12-C save)
    EL<TButton>("TfShuttleMove", "btShu1Left")       ->Enabled=true;
    EL<TButton>("TfShuttleMove", "btShu1Right")      ->Enabled=true;
    EL<TButton>("TfShuttleMove", "btShu2Left")       ->Enabled=true;
    EL<TButton>("TfShuttleMove", "btShu2Right")      ->Enabled=true;

    if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //JerryYang 20160901 shuttle使用one side時,shuttle maintain不給動
    {
        EL<TButton>("TfShuttleMove", "btShu2Left")       ->Enabled=false;
        EL<TButton>("TfShuttleMove", "btShu2Right")      ->Enabled=false;
    }
    else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)
    {
        EL<TButton>("TfShuttleMove", "btShu1Left")       ->Enabled=false;
        EL<TButton>("TfShuttleMove", "btShu1Right")      ->Enabled=false;
    }
    bShuttleRetry=false;
    EL<TGroupBox>("TfShuttleMove", "gbBarCode")->Visible=(BAR_CODE_INSTALL!=ebctUninstall);
    EL<TSpeedButton>("TfShuttleMove", "sbShuttleSensor")->Visible=(SHUTTLE_SENSOR_TYPE==eSensorCCLink || SHUTTLE_SENSOR_TYPE==eSensorCCLink3 ||
                              SHUTTLE_SENSOR_TYPE==eSensorCanBus || SHUTTLE_SENSOR_TYPE==eSensorCanBus3 ||
                              SHUTTLE_SENSOR_TYPE==eSensorEtherCAT || SHUTTLE_SENSOR_TYPE==eSensorEtherCAT3);   //Steven 20191126 : 補上Canbus //Ifor 20250926 add:EtherCAT顯示

    EL<TMemo>("TfShuttleMove", "meShuttleMaintain")->Clear();
    #ifdef SOFT_SIMULTE
    EL<TButton>("TfShuttleMove", "btRetry")->Visible=true;
    #else
    EL<TButton>("TfShuttleMove", "btRetry")->Visible=false;
    #endif

    EL<TEdit>("TfShuttleMove", "edInSH1Sen7DetectPos")->Text      =Tech.iInSH1Sen7DetectPos;
    EL<TEdit>("TfShuttleMove", "edInSH2Sen7DetectPos")->Text      =Tech.iInSH2Sen7DetectPos;
    EL<TEdit>("TfShuttleMove", "edInSH1BarCodePos")->Text         =Tech.iInSH1BarCodePos;
    EL<TEdit>("TfShuttleMove", "edInSH2BarCodePos")->Text         =Tech.iInSH2BarCodePos;
    EL<TEdit>("TfShuttleMove", "edOutSH1OneRowDetectPos")->Text   =Tech.OutSH1ZOneRowDetectPos;
    EL<TEdit>("TfShuttleMove", "edOutSH2OneRowDetectPos")->Text   =Tech.OutSH2ZOneRowDetectPos;
    EL<TEdit>("TfShuttleMove", "edOutSH1ZDetectPos")->Text        =Tech.OutSH1ZDetectPos;
    EL<TEdit>("TfShuttleMove", "edOutSH2ZDetectPos")->Text        =Tech.OutSH2ZDetectPos;

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    {
    fShuttleMove->ReadData();   // golden ShuttleMove.cpp:111-111 取代：ReadData()：golden 表單方法 → 門面 fShuttleMove->ReadData()（forms/fShuttleMove.cpp，golden :2252-2264 逐句；main.cpp／cinitial.cpp 呼叫的同一支）
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:111-111 -- ReadData()：golden 表單方法 → 門面 fShuttleMove->ReadData()（forms/fShuttleMove.cpp，golden :2252-2264 逐句；main.cpp／cinitial.cpp 呼叫的同一支）
        ReadData();
#endif // GATE (S12-C save)
        EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectPos")->Text =Tech.iInSH1SenICDetectPos;
        EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectPos")->Text =Tech.iInSH2SenICDetectPos;
        EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectZPos")->Text=Prod.iInSH1SenICAddPos;
        EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectZPos")->Text=Prod.iInSH2SenICAddPos;
    }

    if(IniConfig.bSPILFunction==true)  //JerryYang 20170921 (Steven) SPIL守志要求鎖定
    {
        EL<TEdit>("TfShuttleMove", "edInSH1Sen7DetectPos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edInSH2Sen7DetectPos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edInSH1BarCodePos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edInSH2BarCodePos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edOutSH1OneRowDetectPos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edOutSH2OneRowDetectPos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edOutSH1ZDetectPos")->Enabled=false;
        EL<TEdit>("TfShuttleMove", "edOutSH2ZDetectPos")->Enabled=false;
        if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                 //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
        {
            EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectPos")->Enabled=false;
            EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectPos")->Enabled=false;
            EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectZPos")->Enabled=false;
            EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectZPos")->Enabled=false;
        }
    }
    SM_ShowShuttleSensorPosition();
    EL<TPanel>("TfShuttleMove", "palSh1Encoder")->Caption=MOT[MInShuttle1].ReadEncoderPos();
    EL<TPanel>("TfShuttleMove", "palSh2Encoder")->Caption=MOT[MInShuttle2].ReadEncoderPos();

    EL<TSpeedButton>("TfShuttleMove", "sbSensorLatch")->Visible= ENABLE_OUT_SHUTTLEY_LATCH; //kevin 20180129 add

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    {
        EL<TGroupBox>("TfShuttleMove", "gbScanOutShuttle")->Caption="Detect In/Out Shuttle device";
        EL<TGroupBox>("TfShuttleMove", "gbInFiberCheckShtSnLct")->Visible=true;
    }
    else
    {
        EL<TGroupBox>("TfShuttleMove", "gbInFiberCheckShtSnLct")->Visible=false;
    }

    if(CosFunction.bAutoTeachOutShuttleLantch)
    {
        iMoveToTarget=-1;
        LastClickTime=0;
        LastClickX=-1;
        LastClickY=-1;
    }

}

// golden ShuttleMove.cpp:1965  TfShuttleMove::sbUpdateClick(TObject *Sender)
static void SM_sbUpdateClick()
{
    filerw::ELMark("sbUpdateClick");   // FileRW：存檔流程 trace（ack.trace）

    Tech.iInSH1Sen7DetectPos    =atoi(EL<TEdit>("TfShuttleMove", "edInSH1Sen7DetectPos")->Text.c_str());
    Tech.iInSH2Sen7DetectPos    =atoi(EL<TEdit>("TfShuttleMove", "edInSH2Sen7DetectPos")->Text.c_str());
    Tech.iInSH1BarCodePos       =atoi(EL<TEdit>("TfShuttleMove", "edInSH1BarCodePos")->Text.c_str());
    Tech.iInSH2BarCodePos       =atoi(EL<TEdit>("TfShuttleMove", "edInSH2BarCodePos")->Text.c_str());
    Tech.OutSH1ZOneRowDetectPos =atoi(EL<TEdit>("TfShuttleMove", "edOutSH1OneRowDetectPos")->Text.c_str());
    Tech.OutSH2ZOneRowDetectPos =atoi(EL<TEdit>("TfShuttleMove", "edOutSH2OneRowDetectPos")->Text.c_str());
    Tech.OutSH1ZDetectPos       =atoi(EL<TEdit>("TfShuttleMove", "edOutSH1ZDetectPos")->Text.c_str());
    Tech.OutSH2ZDetectPos       =atoi(EL<TEdit>("TfShuttleMove", "edOutSH2ZDetectPos")->Text.c_str());

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    {
        AnsiString S, szDir="";

        Tech.iInSH1SenICDetectPos=atoi(EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectPos")->Text.c_str());
        Tech.iInSH2SenICDetectPos=atoi(EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectPos")->Text.c_str());

        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
        WriteIniData(szDir, "Shuttle", "InSH1SenICAddPos", EL<TEdit>("TfShuttleMove", "edInSH1SenICDetectZPos")->Text);
        WriteIniData(szDir, "Shuttle", "InSH2SenICAddPos", EL<TEdit>("TfShuttleMove", "edInSH2SenICDetectZPos")->Text);

    fShuttleMove->ReadData();   // golden ShuttleMove.cpp:1988-1988 取代：ReadData()：同 FormShow :111 → 門面 fShuttleMove->ReadData()（寫完重讀，Prod.iInSH?SenICAddPos 變新值）
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:1988-1988 -- ReadData()：同 FormShow :111 → 門面 fShuttleMove->ReadData()（寫完重讀，Prod.iInSH?SenICAddPos 變新值）
        ReadData();
#endif // GATE (S12-C save)
    }

    FileRW_Teach_SaveFile(true);   // golden ShuttleMove.cpp:1991-1991 取代：fTeach->SaveFile(true)：golden uteach.cpp:4924（V912 :4939；bSaveByTeach=true：TECH_PARA 的 SetEdit->Text=*Parameter 後寫 teach.ini）→ FileRW/Teach.cpp 的 FileRW_Teach_SaveFile(true)（同一份 IC_SaveFile；tech.dat 照裁決 14 不寫、回報 todo）
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:1991-1991 -- fTeach->SaveFile(true)：golden uteach.cpp:4924（V912 :4939；bSaveByTeach=true：TECH_PARA 的 SetEdit->Text=*Parameter 後寫 teach.ini）→ FileRW/Teach.cpp 的 FileRW_Teach_SaveFile(true)（同一份 IC_SaveFile；tech.dat 照裁決 14 不寫、回報 todo）
    fTeach->SaveFile(true);
#endif // GATE (S12-C save)
    InitShuttleThreadParameter();                                               //Steven 20120921 : Teach做完要重新Init一次
    SM_ShowShuttleSensorPosition();

}

// golden ShuttleMove.cpp:1998  TfShuttleMove::ShowShuttleSensorPosition()
static void SM_ShowShuttleSensorPosition()
{

    SetTechDataToProd();                                                        //jimmychiu 20251204 : update sht position array before show info
    SM_mtedInSHSen7DetectPos.XItem=FileRW_InArmSuckShtCol();   // golden ShuttleMove.cpp:2001-2001 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2001-2001 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
    EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->XItem=InArmSuck.iShtCol;
#endif // GATE (S12-C save)
    SM_mtOutSHDetectPos.XItem=FileRW_InArmSuckShtCol();   // golden ShuttleMove.cpp:2002-2002 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2002-2002 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
    EL<TControl>("TfShuttleMove", "mtOutSHDetectPos")->XItem=InArmSuck.iShtCol;
#endif // GATE (S12-C save)
    SM_mtedInSHSenICDetectPos.XItem=FileRW_InArmSuckShtCol();   // golden ShuttleMove.cpp:2003-2003 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2003-2003 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
    EL<TControl>("TfShuttleMove", "mtedInSHSenICDetectPos")->XItem=InArmSuck.iShtCol;                            //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
#endif // GATE (S12-C save)

    for(int i=0; i<FileRW_InArmSuckShtCol(); i++)   // golden ShuttleMove.cpp:2005-2005 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2005-2005 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
    for(int i=0; i<InArmSuck.iShtCol; i++)
#endif // GATE (S12-C save)
    {
    SM_mtOutSHDetectPos.SetCellNumber(i, 1, GetSHCHKPos(i, SThreadPara.base_pos[0]));   // golden ShuttleMove.cpp:2007-2007 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2007-2007 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtOutSHDetectPos")->SetCellNumber(i, 1, GetSHCHKPos(i, SThreadPara.base_pos[0]));
#endif // GATE (S12-C save)
    SM_mtOutSHDetectPos.SetCellNumber(i, 0, GetSHCHKPos(i, SThreadPara.base_pos[1]));   // golden ShuttleMove.cpp:2008-2008 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2008-2008 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtOutSHDetectPos")->SetCellNumber(i, 0, GetSHCHKPos(i, SThreadPara.base_pos[1]));
#endif // GATE (S12-C save)
    SM_mtOutSHDetectPos.SetCellNumber(i, 3, GetSHCHKPos(i, SThreadPara.base_posY[0]));   // golden ShuttleMove.cpp:2009-2009 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2009-2009 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtOutSHDetectPos")->SetCellNumber(i, 3, GetSHCHKPos(i, SThreadPara.base_posY[0]));
#endif // GATE (S12-C save)
    SM_mtOutSHDetectPos.SetCellNumber(i, 2, GetSHCHKPos(i, SThreadPara.base_posY[1]));   // golden ShuttleMove.cpp:2010-2010 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2010-2010 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtOutSHDetectPos")->SetCellNumber(i, 2, GetSHCHKPos(i, SThreadPara.base_posY[1]));
#endif // GATE (S12-C save)

        if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                 //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
        {
    SM_mtedInSHSenICDetectPos.SetCellNumber(i, 1, GetSHCHKPos(i, SThreadPara.base_InposY_SHSn[0]));   // golden ShuttleMove.cpp:2014-2014 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2014-2014 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSenICDetectPos")->SetCellNumber(i, 1, GetSHCHKPos(i, SThreadPara.base_InposY_SHSn[0]));
#endif // GATE (S12-C save)
    SM_mtedInSHSenICDetectPos.SetCellNumber(i, 0, GetSHCHKPos(i, SThreadPara.base_InposY_SHSn[1]));   // golden ShuttleMove.cpp:2015-2015 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2015-2015 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSenICDetectPos")->SetCellNumber(i, 0, GetSHCHKPos(i, SThreadPara.base_InposY_SHSn[1]));
#endif // GATE (S12-C save)
        }

    if(FileRW_InArmSuckShtCol()==1)   // golden ShuttleMove.cpp:2018-2018 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2018-2018 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        if(InArmSuck.iShtCol==1)
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x1[0]);   // golden ShuttleMove.cpp:2020-2020 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2020-2020 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x1[0]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x1[1]);   // golden ShuttleMove.cpp:2021-2021 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2021-2021 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x1[1]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==2)   // golden ShuttleMove.cpp:2023-2023 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2023-2023 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==2)
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x2[0][i]);   // golden ShuttleMove.cpp:2025-2025 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2025-2025 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x2[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x2[1][i]);   // golden ShuttleMove.cpp:2026-2026 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2026-2026 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x2[1][i]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==3)   // golden ShuttleMove.cpp:2028-2028 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2028-2028 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==3)                                           //ChungHung 20140115 add for 2x3_6
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen7DetectPos2x3[0][i]);   // golden ShuttleMove.cpp:2030-2030 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2030-2030 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen7DetectPos2x3[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen7DetectPos2x3[1][i]);   // golden ShuttleMove.cpp:2031-2031 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2031-2031 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen7DetectPos2x3[1][i]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==4)   // golden ShuttleMove.cpp:2033-2033 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2033-2033 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==4)
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x4[0][i]);   // golden ShuttleMove.cpp:2035-2035 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2035-2035 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen7DetectPos1x4[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x4[1][i]);   // golden ShuttleMove.cpp:2036-2036 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2036-2036 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen7DetectPos1x4[1][i]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==5)   // golden ShuttleMove.cpp:2038-2038 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2038-2038 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==5)                                           //Steven 20221027 : Add for 2x5
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x5[0][i]);   // golden ShuttleMove.cpp:2040-2040 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2040-2040 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x5[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x5[1][i]);   // golden ShuttleMove.cpp:2041-2041 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2041-2041 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x5[1][i]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==6)   // golden ShuttleMove.cpp:2043-2043 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2043-2043 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==6)
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x6[0][i]);   // golden ShuttleMove.cpp:2045-2045 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2045-2045 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x6[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x6[1][i]);   // golden ShuttleMove.cpp:2046-2046 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2046-2046 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x6[1][i]);
#endif // GATE (S12-C save)
        }
    else if(FileRW_InArmSuckShtCol()==8)   // golden ShuttleMove.cpp:2048-2048 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2048-2048 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        else if(InArmSuck.iShtCol==8)
#endif // GATE (S12-C save)
        {
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x8[0][i]);   // golden ShuttleMove.cpp:2050-2050 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2050-2050 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 1, Prod.iInSHSen9DetectPos2x8[0][i]);
#endif // GATE (S12-C save)
    SM_mtedInSHSen7DetectPos.SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x8[1][i]);   // golden ShuttleMove.cpp:2051-2051 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2051-2051 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtedInSHSen7DetectPos")->SetCellNumber(i, 0, Prod.iInSHSen9DetectPos2x8[1][i]);
#endif // GATE (S12-C save)
        }
        else
        {
            filerw::ELMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "TfShuttleMove::ShowShuttleSensorPosition");
        }
    }

    if(TestIF_File.bEnableMulti2D &&
       (TestIF_File.iMulti2DType==e1x2In1CCD ||
        TestIF_File.iMulti2DType==e2x2In1CCD ||
        TestIF_File.iMulti2DType==e2x2In2CCD))                                  //Steven 20240612 : for 對角2D
    {
    SM_mtInSHBarCodePos.XItem=FileRW_InArmSuckShtCol()*2;   // golden ShuttleMove.cpp:2064-2064 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2064-2064 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->XItem=InArmSuck.iShtCol*2;
#endif // GATE (S12-C save)

    for(int i=0; i<FileRW_InArmSuckShtCol(); i++)   // golden ShuttleMove.cpp:2066-2066 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2066-2066 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        for(int i=0; i<InArmSuck.iShtCol; i++)
#endif // GATE (S12-C save)
        {
    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2068-2068 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2068-2068 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i*2,   1, fBarCode->GetMovePos(0, i, 0, false));    //Sht1
#endif // GATE (S12-C save)
    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2069-2069 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2069-2069 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i*2+1, 1, fBarCode->GetMovePos(0, i, 1, false));
#endif // GATE (S12-C save)

    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2071-2071 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2071-2071 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i*2,   0, fBarCode->GetMovePos(1, i, 0, false));    //Sht2
#endif // GATE (S12-C save)
    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2072-2072 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2072-2072 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i*2+1, 0, fBarCode->GetMovePos(1, i, 1, false));
#endif // GATE (S12-C save)
        }
        #ifdef SOFT_SIMULTE
        EL<TButton>("TfShuttleMove", "btnTStep")->Visible=true;
        #else
        EL<TButton>("TfShuttleMove", "btnTStep")->Visible=false;
        #endif
    }
    else
    {
    SM_mtInSHBarCodePos.XItem=FileRW_InArmSuckShtCol();   // golden ShuttleMove.cpp:2082-2082 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2082-2082 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->XItem=InArmSuck.iShtCol;
#endif // GATE (S12-C save)

    for(int i=0; i<FileRW_InArmSuckShtCol(); i++)   // golden ShuttleMove.cpp:2084-2084 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2084-2084 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
        for(int i=0; i<InArmSuck.iShtCol; i++)
#endif // GATE (S12-C save)
        {
    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2086-2086 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2086-2086 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i, 1, fBarCode->GetMovePos(0, i, 0, false));
#endif // GATE (S12-C save)
    SM_mtInSHBarCodePos.bPortGap=true;   // golden ShuttleMove.cpp:2087-2087 取代：自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
#if 0 // GATE (S12-C save) golden ShuttleMove.cpp:2087-2087 -- 自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；fBarCode->GetMovePos 未移植 → bPortGap
            EL<TControl>("TfShuttleMove", "mtInSHBarCodePos")->SetCellNumber(i, 0, fBarCode->GetMovePos(1, i, 0, false));
#endif // GATE (S12-C save)
        }

        EL<TButton>("TfShuttleMove", "btnTStep")->Visible=false;
    }

}

