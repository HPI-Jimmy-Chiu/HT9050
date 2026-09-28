// =============================================================================
//  cStateRecord.cpp  --  State Record（TfMain::DoStateRecord 與它的四個幫手）
//
//  AI(W906-STATEREC) 20260924: 新檔。golden 對照：HT9011UC_Code_V3.33.906.0_20260618/main.cpp（cp950）。
//
//  翻譯範圍（golden 行號）：
//    TfMain::SaveTaskList            main.cpp:6402-6751   Task_ListWithTime.csv／Task_ListWithTime2.csv
//    TfMain::SaveDecisionVariables   main.cpp:6753-6963   DecisionVariables.csv
//    TfMain::ProcessTimeUpdate       main.cpp:7806-7818   sFileNameTime（資料夾名稱從它來）
//    TfMain::StateRecordImage        main.cpp:26109-26208 抓圖狀態機（抓圖閘住，見本體）
//    TfMain::sbStateRecordClick      main.cpp:26209-26212 在 forms/fMain.cpp 檔尾（它只呼叫 virtual 的 DoStateRecord）
//    DumpMainFormSnapshot            main.cpp:26214-26339 MainFormSnapshot.txt
//    TfMain::DoStateRecord           main.cpp:26340-26678 本體 = W906_DoStateRecordBody
//    Timer2Timer 的 State Record 段  main.cpp:21142-21153 = W906_StateRecordTimer2Pump
//
//  ---------------------------------------------------------------------------
//  為什麼不放 forms/fMain.cpp
//  ---------------------------------------------------------------------------
//  同 cMainStatus.cpp／cTrayForm.cpp 檔頭：forms/fMain.cpp 編在 ht9045_forms，只准連
//  vclcompat＋ht9045_globals＋ht9045_core；本體要 SaveMachineRecord（cinitial.cpp）、Del_Tree／
//  IsMainProcAlive（csystem.cpp）、MOT[]、InArmSuck 等 ht9045_sm 的符號。
//
//  DoStateRecord 在 forms/fMain.h:194 是 virtual，移植樹有 11 行 fMain->DoStateRecord( 呼叫（Grep 20260924）：
//    SOFT_SIMULTE 組態下活的 3 行：acatchtray.cpp:3927、ainarm2.cpp:5655、atester_32Site.cpp:2367；
//    出貨組態（-DW906_NO_SOFT_SIMULTE）再多 4 行：csystem.cpp:1265、aTester_Front.cpp:9905、aTester_Rear.cpp:9843、uhome.cpp:942；
//    永遠不編的 4 行：atester.cpp:4446／:4448 與 csystem_shims.cpp:59（#if 0）、asendic.cpp:918（DEBUG_AutoZMotor 未定義）。
//  所以**沒有**改成非 virtual（cMainStatus.cpp 的作法），而是走 Clarn_Data 的安裝座作法
//  （forms/fMain.h:1246-1321）：forms/fMain.cpp:400 呼叫 W906_StateRecordBody 指標，
//  本檔的 W906_InstallStateRecordBody() 由 wb_serve 開機時明確呼叫。
//  理由：改成非 virtual 會讓那些 sm 呼叫點所在的每一支測試執行檔都把本檔（連同 7z／robocopy／
//  背景執行緒）連進去；走指標則只有 wb_serve 會連到本檔 —— 測試裡的 DoStateRecord 維持 no-op。
//
//  ---------------------------------------------------------------------------
//  執行模型（使用者 20260924 裁決）
//  ---------------------------------------------------------------------------
//  golden 在 UI 執行緒跑 DoStateRecord、Timer2 每秒推一次抓圖狀態機，第 6 格才用 system() 跑 7z。
//  移植樹的 wb_serve 只有一條 tick 執行緒同時跑機台控制與網頁命令，所以：
//    * 同步（tick 執行緒，呼叫當下）：所有「變數快照」—— Task_ListWithTime*.csv、DecisionVariables.csv、
//      MainFormSnapshot.txt、Ver.txt，以及 golden 的 SaveMachineRecord／建資料夾／組批次檔內容。
//    * 背景（一條 detached Win32 執行緒）：所有「複製檔案」與 7z 壓縮 —— CopyFile、SHFileOperation、
//      寫 1.bat 並執行（**等它結束**）、7z、Del_Tree、手動時開資料夾。
//      同時只允許一個背景工作（W906_StateRecordWorkerBusy，forms/fMain.cpp 檔尾定義）。
//  背景執行緒**不碰任何機台全域**：它拿到的是同步段組好的一份 StateRecordJob（全是字串）。
//
//  ---------------------------------------------------------------------------
//  ⚠ SOFT_SIMULTE：golden 自己把整段錄製包在 `#ifdef SOFT_SIMULTE #else … #endif`（main.cpp:26343-26674）
//  ---------------------------------------------------------------------------
//  本檔照翻。⇒ SOFT_SIMULTE 組態（V906 預設）下 DoStateRecord 只做三件事：
//  RecordProcess("State Record.")、LogIndexMaxMinPos("StateRecord")、DumpMainFormSnapshot(NewPath)。
//  NewPath 在那個組態下從來沒被設過（golden 同），是空字串 ⇒ DumpMainFormSnapshot 寫的是
//  "\\MainFormSnapshot.txt"，也就是**目前磁碟機的根目錄**。這是 golden 的形狀
//  （AI(ht9045-v899) 20260604 把呼叫放在 #endif 之後），照翻不修；要不要讓模擬組態也錄製是使用者的決定。
// =============================================================================
#include "forms/fMain.h"
#include "forms/fObserver.h"       // fObserver->Memo1（Ver.txt）
#include "forms/fAGV.h"            // fAGV->IsATK_AMR()
#include "vclcompat/vcl_compat.h"  // AnsiString、TStringList、FileExists、FindFirst、FormatDateTime…
#include "MachineType.h"           // SOFT_SIMULTE、TOTAL_MOTOR
#include "cmydef.h"                // bNoUseAutoRecord、sFileNameTime、sAlarmTime、QueueTaskList／qTaskCount、slEventLog、
                                   // QueueGalilCmd、CUSTOMER_CODE、bRunAutoClean、XResolution… 以及 SaveTaskList 的旗標群
#include "cprod.h"                 // Prod、TestIF_File、TrayForm
#include "cpublic.h"               // GetTimeInfo、ExecZipCommand（對照用）、LogIndexMaxMinPos、MAX_Q_10、TMyQueue10
#include "common.h"                // MyForceDirectories、GetLastOpenFN、DataPath、asGalilCmdPath、asTestTCPIPLogPath
#include "LastSet.h"               // LastSet.*
#include "Config.h"                // IniConfig.*
#include "CosFunction.h"           // CosFunction.bOLPFunction
#include "cinitial.h"              // SaveMachineRecord
#include "csystem.h"               // Del_Tree、IsMainProcAlive／GetMainProcSilentSeconds／GetMainProcLastEnterTimeString／GetMainProcCallCount
#include "MessageDef.h"            // MSG_CMD_State_Record
#include "SgdToXLS.h"              // SGDToXLS（移植樹本體是 no-op，SgdToXLS.cpp:88-94）
#include "Motor/mymotor.h"         // MOT[]、RecordIndexPosition
#include "Motor/myMN200motor.h"    // MNetLog
#include "aHotPlateSubstrate.h"    // InArmSuck／OutArmSuck／FRCarryKit… —— 與 cMainStatus.cpp:29 同一個標頭（兩個 TMyKitSuck 的陷阱）
#include "acarry.h"                // AutoSHT1Task／AutoSHT2Task、b1ShuttleMoveToLeft／b2ShuttleMoveToLeft
#include "acatchtray.h"            // CatchTrayTask、iCatchFromLoaderTask
#include "aoutarm.h"               // OutArmTask、iPickFromShuttle1Task／iPickFromShuttle2Task
#include "asendic.h"               // iTrayZLoadTrayToWaitTask
#include "asendic_Loader.h"        // LoadTask
#include "atester.h"               // iTestTask、iTestHeadMotorTask、iTestYTask
#include "acarry_shims.h"          // ATC_InterfaceForm（golden ATC_Handler_Side.h 的全域名；移植樹由這個 shim 持有）
#include "Automation/automation.h" // fAutomationEngine（golden 的 fAutomation；名稱改掉的原因見 automation.h:81-99）
#include "Public/MyStringList.h"   // TMyStringList::GetFileName（slEventLog）
#include "Public/MyProductionRecord.h" // TMyProductionRecord::SaveRecordCleanPad
#include "canary_support.h"        // RecordProcess、ShowErrorMessage、ShowMyMessage

#include <windows.h>
#include <shellapi.h>              // SHFileOperation／SHFILEOPSTRUCT／ShellExecute（handlerlog.cpp:182 已連 shell32）
#include <atomic>
#include <cstdio>
#include <cstdlib>                 // system()
#include <string>
#include <utility>
#include <vector>

// golden ainarm2.h:219 的全域；移植樹唯一定義在 acatchtray.cpp:136，沒有任何標頭 extern 它
// （ainarm2.cpp:5187-5196 記載），所以同 ainarm2.cpp／asendic_Loader.cpp:233 的作法就地宣告。
extern int iInArmWaitPosition;
// golden ainarm2.h:138；本體 ainarm2.cpp:4977（acatchtray_shims.h:415 也宣告了同一個簽章）。
bool IsMoveInArm2XYToWait();

// -----------------------------------------------------------------------------
//  背景工作的輸入：同步段組好、背景執行緒只讀。全部是 std::string，**不含任何機台全域的參照**。
// -----------------------------------------------------------------------------
namespace {

struct StateRecordJob
{
    std::string newPath;                                        // golden NewPath（當下的值）
    std::string sDataPath;                                      // golden SDataPath（手動時開資料夾用）
    bool        bManual = false;                                // golden bManualStateRecord
    std::vector<std::pair<std::string, std::string> > copies;   // golden 的 CopyFile(src, dst, false)，依 golden 順序
    std::string shFrom, shTo;                                   // golden SHFileOperation(FO_COPY) 的來源／目的（空 = 不做）
    std::string batFile;                                        // golden BatFile
    std::vector<std::string> batLines;                          // golden TestList 的內容
};

void RunStateRecordJob(StateRecordJob job);

DWORD WINAPI StateRecordWorkerProc(LPVOID p)
{
    StateRecordJob *pJob=static_cast<StateRecordJob*>(p);
    try { RunStateRecordJob(*pJob); } catch(...) { W906_StateRecordWorkerBusy.store(false); }
    delete pJob;
    return 0;
}

}  // namespace

// W906_StateRecordWorkerBusy：forms/fMain.cpp 檔尾定義、fMain.h 檔尾宣告。
//   true = 背景工作還在跑。act.main.stateRecord 用它回 busy，本檔用它擋重疊錄製。

// =============================================================================
//  TfMain::SaveTaskList  --  golden main.cpp:6402-6751，逐字翻譯
//
//  ⚠ 輸出的前半段（每一列一個 QueueTaskList[i]）在移植樹目前會是空列：golden 在 FormShow
//    （main.cpp:9740-10028）用 SetAliasAndTask 登錄 281 個 task 游標，移植樹沒有那段
//    （全樹 SetAliasAndTask( 呼叫 0 個，只有 cpublic.cpp:1280 的定義）。那是 FormShow 的缺口，
//    不是本函式的；後半段（旗標、MOT[]、吸嘴 Item、CleanKitTime、MainProcMonitor）是活的。
// =============================================================================
void TfMain::SaveTaskList(AnsiString NewPath)                                   //Steven 20200304 : 儲存紀錄Task+time的方式
{
    int flag;
    TStringList *sList=new TStringList();
    TStringList *sList2=new TStringList();
    AnsiString Str="", Str1="";
    AnsiString Str2="";
    bool bTemp=false;                                                           //JerryYang 20250505 : add log
    int iTemp=0;
    bool bMainProcAlive=false;                                                  //JerryYang 20260414 : Add 執行緒最後進入時間log
    double dMainProcSilentSec=-1.0;
    AnsiString sMainProcLastTime="N/A";
    AnsiString sSaveTime=FormatDateTime("yyyy/mm/dd hh:nn:ss.zzz", Now());

    for(int i=1; i<qTaskCount; i++)
    {
        Str=QueueTaskList[i].Alias+",";
        for(int j=0; j<MAX_Q_10; j++)
        {
            if(QueueTaskList[i].iCount<MAX_Q_10)
            {
                flag=QueueTaskList[i].iCount-j;                                 //Steven 20200730 : 修正Task List紀錄順序
                if(flag>=0)                                                     //Steven 20200811 : 修正Task List紀錄內容
                {
                    if(QueueTaskList[i].iData[j]>0)                             //Steven 20211030 : Task=0的不紀錄
                    {
                        Str+=" "+AnsiString(QueueTaskList[i].GetDateTime(flag))+AnsiString(",");
                        Str2.sprintf("%s, %s", QueueTaskList[i].GetDateTime(flag), QueueTaskList[i].Alias);
                        sList2->Add(Str2);
                    }
                }
                else
                {
                    Str+=AnsiString(",");
                }
            }
            else
            {
                if(QueueTaskList[i].iData[MAX_Q_10-j]>0)                        //Steven 20211030 : Task=0的不紀錄
                {
                    Str+=" "+AnsiString(QueueTaskList[i].GetDateTime(MAX_Q_10-j))+AnsiString(",");                      //Steven 20210607 : MAX_Q_10-j-1 --> MAX_Q_10-j
                    Str2.sprintf("%s, %s", QueueTaskList[i].GetDateTime(MAX_Q_10-j), QueueTaskList[i].Alias);
                    sList2->Add(Str2);
                }
            }
        }
        sList->Add(Str);
    }

    Str.sprintf("bPickFromLoader, %s", (bPickFromLoader)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPlaceToHotplate, %s", (bPlaceToHotplate)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPickFromHotplate, %s", (bPickFromHotplate)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bDestoryOnSht, %s", (bDestoryOnSht)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPlaceToShuttle2Step, %s", (bPlaceToShuttle2Step)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bWaitPreciserFinish, %s", (bWaitPreciserFinish)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bRecIndexDropAlarm1, %s", (bRecIndexDropAlarm1)?AnsiString("true"):AnsiString("false"));               //Steven 20201201 : add event log for debug
    sList->Add(Str);

    Str.sprintf("bRecIndexDropAlarm2, %s", (bRecIndexDropAlarm2)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fRearNeedSuckIC, %s", (fRearNeedSuckIC)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fFrontNeedSuck, %s", (fFrontNeedSuck)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fFrontNeedDestroy, %s", (fFrontNeedDestroy)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fFrontNeedSuckIC, %s", (fFrontNeedSuckIC)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fRearNeedSuck, %s", (fRearNeedSuck)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fRearNeedDestroy, %s", (fRearNeedDestroy)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("f32SiteNeedDestroy, %s", (f32SiteNeedDestroy)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("f32SiteNeedSuck, %s", (f32SiteNeedSuck)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fRearNeedTest, %s", (fRearNeedTest)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fFrontNeedTest, %s", (fFrontNeedTest)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("fTwoArmNeedTest, %s", (fTwoArmNeedTest)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bLockPlaceToShuttleByAutoClean, %s", (bLockPlaceToShuttleByAutoClean)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bLockPickFromShuttleByAutoClean, %s", (bLockPickFromShuttleByAutoClean)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPlaceToCleanKit, %s", (bPlaceToCleanKit)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPickFromKitByAutoClean, %s", (bPickFromKitByAutoClean)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPlaceToShuttleByAutoClean, %s", (bPlaceToShuttleByAutoClean)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bPickFromShuttleByAutoClean, %s", (bPickFromShuttleByAutoClean)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("InArmSuck.iWhichSht, %d", InArmSuck.iWhichSht);
    sList->Add(Str);

    Str.sprintf("InArmSuck.iWhichKit, %d", InArmSuck.iWhichKit);
    sList->Add(Str);

    Str.sprintf("InArmSuck.iWhichShtPickFor32, %d", InArmSuck.iWhichShtPickFor32);
    sList->Add(Str);

    Str.sprintf("InArmSuck.iWhichKitPickFor32, %d", InArmSuck.iWhichKitPickFor32);
    sList->Add(Str);

    //Sam 20230619 : 新增 Clean吸放時間 Log
    Str.sprintf("bCheckShuttle1Flag, %s", (bCheckShuttle1Flag)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bCheckShuttle2Flag, %s", (bCheckShuttle2Flag)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("b1ShuttleMoveToLeft, %s", (b1ShuttleMoveToLeft)?AnsiString("true"):AnsiString("false"));               //JerryYang 20260525 : SHT1 direction flag for hang up analysis
    sList->Add(Str);

    Str.sprintf("b2ShuttleMoveToLeft, %s", (b2ShuttleMoveToLeft)?AnsiString("true"):AnsiString("false"));               //JerryYang 20260525 : SHT2 direction flag for hang up analysis
    sList->Add(Str);

    Str.sprintf("bZ1PickShuttle, %s", (bZ1PickShuttle)?AnsiString("true"):AnsiString("false"));                         //JerryYang 20260525 : Front index Z occupying shuttle area flag
    sList->Add(Str);

    Str.sprintf("bZ2PickShuttle, %s", (bZ2PickShuttle)?AnsiString("true"):AnsiString("false"));                         //JerryYang 20260525 : Rear index Z occupying shuttle area flag
    sList->Add(Str);

    Str.sprintf("NeedWaitTrayArm, %s", (NeedWaitTrayArm)?AnsiString("true"):AnsiString("false"));                       //KenHsieh 20230717 : LEADYO Hang up Log紀錄
    sList->Add(Str);

    bTemp=MOT[MMTrayY].Tray.HasIC();                                            //JerryYang 20250505 : add log
    Str.sprintf("MOT[MMTrayY].Tray.HasIC(), %s", (bTemp)?AnsiString("true"):AnsiString("false"));                       //KenHsieh 20230717 : LEADYO Hang up Log紀錄
    sList->Add(Str);

    Str.sprintf("MOT[MMTrayY].fHasTray, %s", (MOT[MMTrayY].fHasTray)?AnsiString("true"):AnsiString("false"));           //KenHsieh 20230717 : LEADYO Hang up Log紀錄
    sList->Add(Str);

//    bTemp=InArmXYZSafe();
//    Str.sprintf("InArmXYZSafe(), %s", (bTemp)?AnsiString("true"):AnsiString("false"));           //KenHsieh 20230717 : LEADYO Hang up Log紀錄
//    sList->Add(Str);

    Str.sprintf("MOT[MInArmX].Led[iInposLed], %s", (MOT[MInArmX].Led[iInposLed])?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("MOT[MInArmY].Led[iInposLed], %s", (MOT[MInArmY].Led[iInposLed])?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("Prod.iInArmSafeX, %d", Prod.iInArmSafeX);
    sList->Add(Str);

    Str.sprintf("Prod.iInArmSafeY, %d", Prod.iInArmSafeY);
    sList->Add(Str);

    iTemp=MOT[MInArmX].CompareEncoderPos(Prod.iInArmSafeX, 2);
    Str.sprintf("MOT[MInArmX].CompareEncoderPos(Prod.iInArmSafeX, 2), %d", iTemp);
    sList->Add(Str);

    iTemp=MOT[MInArmY].CompareEncoderPos(Prod.iInArmSafeY, 2);
    Str.sprintf("MOT[MInArmY].CompareEncoderPos(Prod.iInArmSafeY, 2), %d", iTemp);
    sList->Add(Str);

    iTemp=MOT[MInArmX].CompareCommandPos(Prod.iInArmSafeX, 2);
    Str.sprintf("MOT[MInArmX].CompareCommandPos(Prod.iInArmSafeX, 2), %d", iTemp);
    sList->Add(Str);

    iTemp=MOT[MInArmY].CompareCommandPos(Prod.iInArmSafeY, 2);
    Str.sprintf("MOT[MInArmY].CompareCommandPos(Prod.iInArmSafeY, 2), %d", iTemp);
    sList->Add(Str);

    bTemp=IsMoveInArm2XYToWait();
    Str.sprintf("IsMoveInArm2XYToWait(), %s", (bTemp)?AnsiString("true"):AnsiString("false"));
    sList->Add(Str);

    Str.sprintf("bWaitRotateFinish, %s", (bWaitRotateFinish)?AnsiString("true"):AnsiString("false"));                   //Ifor 20241015 add: 等待 Rotate 完成
    sList->Add(Str);

    Str.sprintf("bDoTrayDeviceCheck, %s", (bDoTrayDeviceCheck)?AnsiString("true"):AnsiString("false"));                 //Sam 20240827 : 新增紀錄
    sList->Add(Str);
    //Steven 20210217 : State record加上馬達資料
    //==>
    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        if(MOT[i].Motor!=NULL && MOT[i].Motor->Enable==true)                    //JerryYang 20250505 : add log
        {
            Str.sprintf("MOT[%s], fCanMove=%s, fCanMoveR=%s, fCanMoveM=%s, fCanMoveL=%s, CMD=%d, Encoder=%d, Speed=%d",
                        MOT[i].NumberAlias,
                        (MOT[i].fCanMove)?AnsiString("true"):AnsiString("false"),
                        (MOT[i].fCanMoveR)?AnsiString("true"):AnsiString("false"),
                        (MOT[i].fCanMoveM)?AnsiString("true"):AnsiString("false"),
                        (MOT[i].fCanMoveL)?AnsiString("true"):AnsiString("false"),
                        MOT[i].ReadPos(),
                        MOT[i].ReadEncoderPos(),
                        MOT[i].GetSpeed());
            sList->Add(Str);
        }
    }
    //<==
    //Steven 20210217 : State record加上馬達資料

    //Sam 20211001 : State record 紀錄 In/Out Shuttle IndexArm 資料
    //==>
    Str="InArmSuck";
    sList->Add(Str);
    Str="";
    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            Str+=IntToStr(InArmSuck.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="OutArmSuck";
    sList->Add(Str);
    Str="";
    for(int i=0; i<InArmSuck.iMaxRow; i++)                                     // golden 照翻：迴圈上限用 InArmSuck 的 iMaxRow／iMaxCol 印 OutArmSuck（golden :6647-6649）
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            Str+=IntToStr(OutArmSuck.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="FRCarryKit";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(FRCarryKit.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="BRCarryKit";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(BRCarryKit.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="FTestSuck";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(FTestSuck.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="BTestSuck";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(BTestSuck.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }
    //<==
    //Sam 20211001 : State record 紀錄 In/Out Shuttle IndexArm 資料

    //Sam 20230619 : 新增 Clean吸放時間 Log
    //==>
    Str="CleanKitTime";
    sList->Add(Str);
    Str="";
    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
    {
        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
        {
            if(CleanKitRecord[Y][X]!="")
                Str+=CleanKitRecord[Y][X]+",";
            else
                Str+="na,";
        }
        sList->Add(Str);
        Str="";
    }
    //<==
    //Sam 20230619 : 新增 Clean吸放時間 Log

    bMainProcAlive=IsMainProcAlive(10);                                         //JerryYang 20260414 : Add 執行緒最後進入時間log
    dMainProcSilentSec=GetMainProcSilentSeconds();
    sMainProcLastTime=GetMainProcLastEnterTimeString();
    Str.sprintf("MainProcMonitor, Alive=%s, CallCount=%u, LastEnter=%s, SilentSec=%.3f, SaveTime=%s",
                bMainProcAlive ? "Y" : "N",
                GetMainProcCallCount(),
                sMainProcLastTime.c_str(),
                dMainProcSilentSec,
                sSaveTime.c_str());
    sList->Add(Str);

    sList->SaveToFile(NewPath+"\\Task_ListWithTime.csv");
    sList->Clear();
    delete sList;

    sList2->Sort();
    sList2->Insert(0, "Time,Task,Task_Name");
    sList2->SaveToFile(NewPath+"\\Task_ListWithTime2.csv");
    sList2->Clear();
    delete sList2;
    (void)Str1;                                                                 // golden 宣告了沒用（:6407），照留
}
//---------------------------------------------------------------------------
// SaveDecisionVariables() - 新增：記錄關鍵決策變數（獨立檔案）
//
// 用途：記錄影響流程分支的關鍵變數，幫助 StateRecord 分析
// 輸出：DecisionVariables.csv（與 Task_ListWithTime.csv 分離）
// 版本：v1.0 (2026-04-17)
//---------------------------------------------------------------------------
//  AI(W906-STATEREC) 20260924: golden main.cpp:6753-6963，逐字翻譯。
//    ATC_InterfaceForm 在移植樹是 acarry_shims.h:115 的 shim（iATC_MODE_TYPE 離線恆為 0）——
//    golden 的全域名由它持有（forms/fATCHandlerSide.h:95-110 記載），所以這一行印出來的是 0。
void TfMain::SaveDecisionVariables(AnsiString NewPath)
{
    TStringList *sList=new TStringList();
    AnsiString Str="";

    try
    {
        // 檔頭註解
        sList->Add("# HT9045 StateRecord - Decision Variables (Level 2)");
        sList->Add("# Generated: " + DateTimeToStr(Now()));
        sList->Add("# Format: VariableName, Value");
        sList->Add("# Refer to: decision-variables-registry.md");
        sList->Add("");

        // ===== 1. 流程模式類 =====
        sList->Add("# === Flow Mode Variables ===");

        Str.sprintf("iInArmType, %d", iInArmType);
        sList->Add(Str);

        Str.sprintf("iCloseSiteModeFor2x8, %d", iCloseSiteModeFor2x8);
        sList->Add(Str);

        Str.sprintf("InArmSuck.iModeX, %d", InArmSuck.iModeX);
        sList->Add(Str);

        Str.sprintf("InArmSuck.iXStep, %d", InArmSuck.iXStep);
        sList->Add(Str);

        Str.sprintf("InArmSuck.iYStep, %d", InArmSuck.iYStep);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iModeX, %d", OutArmSuck.iModeX);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iXStep, %d", OutArmSuck.iXStep);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iYStep, %d", OutArmSuck.iYStep);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iShtKitStep, %d", OutArmSuck.iShtKitStep);
        sList->Add(Str);

//        Str.sprintf("bSingleInArm, %s", (bSingleInArm)?AnsiString("true"):AnsiString("false"));
//        sList->Add(Str);

        sList->Add("");

        // ===== 2. Shuttle / Kit 選擇類 =====
        sList->Add("# === Shuttle / Kit Selection ===");

        Str.sprintf("InArmSuck.iWhichShuttle, %d", InArmSuck.iWhichSht);
        sList->Add(Str);

        Str.sprintf("InArmSuck.iWhichKit, %d", InArmSuck.iWhichKit);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iWhichSht, %d", OutArmSuck.iWhichSht);
        sList->Add(Str);

        Str.sprintf("OutArmSuck.iWhichKit, %d", OutArmSuck.iWhichKit);
        sList->Add(Str);

        Str.sprintf("iOutArmiWhichKit, %d", iOutArmiWhichKit);
        sList->Add(Str);

        sList->Add("");

        // ===== 3. 測試模式與狀態類 =====
        sList->Add("# === Test Mode and Status ===");

        Str.sprintf("iRunStartMode, %d", LastSet.iRunStartMode);
        sList->Add(Str);

        Str.sprintf("LastSet.iTester, %d", LastSet.iTester);
        sList->Add(Str);

        Str.sprintf("LastSet.iRealDummy, %d", LastSet.iRealDummy);
        sList->Add(Str);

//        Str.sprintf("bUse32SiteMode, %s", (bUse32SiteMode)?AnsiString("true"):AnsiString("false"));
//        sList->Add(Str);

        sList->Add("");

        // ===== 4. Config 關鍵旗標 =====
        sList->Add("# === Critical Config Flags ===");

        Str.sprintf("IniConfig.bD30EnableSiteModeSelect, %s", (IniConfig.bD30EnableSiteModeSelect)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

//        Str.sprintf("IniConfig.bSingleInArmFunction, %s", (IniConfig.bSingleInArmFunction)?AnsiString("true"):AnsiString("false"));
//        sList->Add(Str);

        Str.sprintf("IniConfig.bNewResetFunction, %s", (IniConfig.bNewResetFunction)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("IniConfig.bSPILFunction, %s", (IniConfig.bSPILFunction)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        sList->Add("");

        // ===== 5. Shuttle / Index Key Status =====
        sList->Add("# === Shuttle / Index Key Status ===");

        Str.sprintf("TestIF_File.iShuttleMode, %d", TestIF_File.iShuttleMode);
        sList->Add(Str);

        Str.sprintf("TestIF_File.iShuttle_Sel, %d", TestIF_File.iShuttle_Sel);
        sList->Add(Str);

        sList->Add("");

        // ===== 6. Customer Code =====
        sList->Add("# === Customer Code ===");

        Str.sprintf("CUSTOMER_CODE, %d", CUSTOMER_CODE);
        sList->Add(Str);

        sList->Add("");

        // ===== 7. Tester Interface =====
        sList->Add("# === Tester Interface ===");

        Str.sprintf("TestIF_File.iTestType, %d", TestIF_File.iTestType);
        sList->Add(Str);

        Str.sprintf("TestIF_File.iTestMode, %d", TestIF_File.iTestMode);
        sList->Add(Str);

        sList->Add("");

        // ===== 8. Temperature / ATC =====
        sList->Add("# === Temperature / ATC ===");

        Str.sprintf("LastSet.iTemperature, %d", LastSet.iTemperature);
        sList->Add(Str);

        Str.sprintf("ATC_InterfaceForm->iATC_MODE_TYPE, %d", ATC_InterfaceForm->iATC_MODE_TYPE);
        sList->Add(Str);

        sList->Add("");

        // ===== 9. Run State / Alarm =====
        sList->Add("# === Run State / Alarm ===");

        Str.sprintf("bPauseHappen, %s", (bPauseHappen)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("bGPIBPause, %s", (bGPIBPause)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("bAlarmBuzzer, %s", (bAlarmBuzzer)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("bRunAutoClean, %s", (bRunAutoClean)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        sList->Add("");

        // ===== 10. Task State Snapshot =====
        sList->Add("# === Task State Snapshot ===");

        Str.sprintf("iArmTask, %d", iArmTask);
        sList->Add(Str);

        Str.sprintf("OutArmTask, %d", OutArmTask);
        sList->Add(Str);

        Str.sprintf("AutoSHT1Task, %d", AutoSHT1Task);
        sList->Add(Str);

        Str.sprintf("AutoSHT2Task, %d", AutoSHT2Task);
        sList->Add(Str);

        Str.sprintf("iTestTask, %d", iTestTask);
        sList->Add(Str);

        Str.sprintf("iTestHeadMotorTask, %d", iTestHeadMotorTask);
        sList->Add(Str);

        Str.sprintf("iTestYTask, %d", iTestYTask);
        sList->Add(Str);

        Str.sprintf("CatchTrayTask, %d", CatchTrayTask);
        sList->Add(Str);

        Str.sprintf("LoadTask, %d", LoadTask);
        sList->Add(Str);

        // 儲存檔案
        sList->SaveToFile(NewPath+"\\DecisionVariables.csv");
    }
    catch(...)
    {
#if 0 // GATE(W906-STATEREC-G8) VCL Dialogs.hpp 的 ShowMessage（模態框）移植樹沒有宣告（Grep 全樹 *.h 0 個宣告；Command.cpp:12000、ATC/ATCSystem.cpp:196 同一個缺件各自閘住）
        ShowMessage("SaveDecisionVariables Error!");
#else
        RecordProcess("SaveDecisionVariables Error!");                          // 閘住的模態框改記一筆，錯誤不至於完全無聲（不影響流程：golden 的框也只是按確定）
#endif // GATE(W906-STATEREC-G8)
    }

    sList->Clear();
    delete sList;
}
//==============================================================================
// END OF StateRecord Decision Variables Implementation
//==============================================================================

// =============================================================================
//  TfMain::ProcessTimeUpdate  --  golden main.cpp:7806-7818
//  DoStateRecord 用 ProcessTimeUpdate(true) 產生 sFileNameTime，資料夾名稱就是它
//  （"YYYY-MM-DD HH_MM_SS"）。沒有這一支，NewPath 會等於 SDataPath 本身 —— 壓縮成功後的
//  Del_Tree(NewPath) 就會把整個 D:\HT9045_StateRecord 刪掉。所以它必須翻，不能閘。
//  golden 另外由 Timer 週期呼叫（flag=false 時每 11 次才更新）；移植樹週期的那一半已由
//  WebBridgeTags.cpp:625 的 GetTimeInfo() 承接，本函式目前只有本檔呼叫。
// =============================================================================
void TfMain::ProcessTimeUpdate(bool flag)
{
    static int ct=0;

    if(ct++>10 || flag)
    {
        GetTimeInfo();
        ct=0;
        sAlarmTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        sFileNameTime.sprintf("%04d-%02d-%02d %02d_%02d_%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);                          //Steven 20190801 : 修正檔名錯誤
#if 0 // GATE(W906-STATEREC-G1) StatusBar1（golden main.h TStatusBar*）移植樹沒有：Grep 全樹 *.h 的 StatusBar1 在 forms/fMain.h 0 命中。純顯示。
        StatusBar1->Panels->Items[6]->Text=sAlarmTime;                          //Ifor 20160503 KYEC 客戶要求顯示詳細時間
#endif // GATE(W906-STATEREC-G1)
    }
}

// =============================================================================
//  TfMain::StateRecordImage  --  golden main.cpp:26109-26208
//
//  golden 由 Timer2Timer（main.cpp:21142-21153，Timer2 沒寫 Interval ⇒ VCL 預設 1000 ms）每兩拍推一格。
//  移植樹由下方的 W906_StateRecordTimer2Pump() 推（wb_serve 的 tick 迴圈呼叫）。
//
//  格 1～4 是抓螢幕（BitBlt 桌面 DC 存 .bmp）、格 5 是把畫面還原：網頁模式下 wb_serve 沒有 VCL 視窗可抓，
//  ImageRecord／pgMotionView／HVisionWnd／sbStateRecord／sbStateRecord2／fNote->BringToFront／
//  MainFormSizeToEpson／sbReturnToMainClick 在移植樹全部不存在 ⇒ 那些敘述閘住（GATE-G2），
//  **只保留每一格的推進與計數**（iSaveImageTask++／iSaveImageCT=0），所以格 6 仍在 golden 的節拍上發生。
//
//  格 6：
//    * 延後的警報（JAM0316／JAM0317／加熱盤格式）照翻，在 tick 執行緒上發 —— 這是 golden 把警報延後到
//      抓圖之後的原因（Steven 20220716「避免抓圖的時候被Alarm擋住」），呼叫端（例如 csystem.cpp:1265
//      之後的 fAllMotorHome=false）的時序因此跟 golden 一樣：先回來、警報幾秒後才出現。
//    * 7z 壓縮＋Del_Tree＋手動時開資料夾：**搬到背景執行緒**（使用者 20260924 裁決），由 RunStateRecordJob
//      在複製結束之後執行（MOVED-BG）。golden 靠格 1～5 的 ~10 秒當作「等非同步批次複製跑完」的隱含等待；
//      移植樹沒有抓圖，改成背景執行緒明確等批次結束。
// =============================================================================
void TfMain::StateRecordImage()
{
    AnsiString str1="";                                                         //KenHsieh 20230105 : 新增StateRecord 另存路徑
    int iret=-1;                                                                //KenHsieh 20230105 : 新增StateRecord 另存路徑
#if 0 // GATE(W906-STATEREC-G2) 螢幕解析度只給下面的抓圖用；抓圖本身缺 VCL 視窗（見函式頭）
    int iScrW=XResolution;                                                      //Steven 20260504 : Support 1920x1080 and 1280x1024
    int iScrH=YResolution;
    if(iScrW<=0) iScrW=1280;
    if(iScrH<=0) iScrH=1024;
#endif // GATE(W906-STATEREC-G2)

    switch(iSaveImageTask)
    {
        case 1:
#if 0 // GATE(W906-STATEREC-G2) 缺 ImageRecord（TImage）／MainFormSizeToEpson／BringToFront／pgMotionView：Grep forms/fMain.h 0 命中；wb_serve 沒有 VCL 視窗可 BitBlt
            ImageRecord->Height=iScrH;                                          //Steven 20260504 : Use actual screen resolution
            ImageRecord->Width=iScrW;

            MainFormSizeToEpson(true);
            fMain->BringToFront();                                              //Steven 20100825
            BitBlt(ImageRecord->Canvas->Handle,0,0,iScrW,iScrH,GetDC(0),0,0,SRCCOPY);
            ImageRecord->Picture->SaveToFile(NewPath+"\\MainForm.bmp");

            MainFormSizeToEpson(false);
            pgMotionView->ActivePage=tsMotorView;
#endif // GATE(W906-STATEREC-G2)
            iSaveImageTask++;
            iSaveImageCT=0;
            break;
        case 2:
#if 0 // GATE(W906-STATEREC-G2) 同上（MotorView.bmp）
            MainFormSizeToEpson(false);
            pgMotionView->ActivePage=tsMotorView;
            fMain->BringToFront();                                              //Steven 20100825
            BitBlt(ImageRecord->Canvas->Handle,0,0,iScrW,iScrH,GetDC(0),0,0,SRCCOPY);
            ImageRecord->Picture->SaveToFile(NewPath+"\\MotorView.bmp");
#endif // GATE(W906-STATEREC-G2)
            iSaveImageTask++;
            iSaveImageCT=0;
            break;
        case 3:
#if 0 // GATE(W906-STATEREC-G2) 同上（MotionView.bmp）；另缺 HVisionWnd（golden TfMain 的 HWND 成員，forms/fMain.h 0 命中）
            MainFormSizeToEpson(false);
            pgMotionView->ActivePage=tsActionView;
            fMain->BringToFront();                                              //Steven 20100825
            BitBlt(ImageRecord->Canvas->Handle,0,0,iScrW,iScrH,GetDC(0),0,0,SRCCOPY);
            ImageRecord->Picture->SaveToFile(NewPath+"\\MotionView.bmp");
            SetForegroundWindow(HVisionWnd);
#endif // GATE(W906-STATEREC-G2)
            iSaveImageTask++;
            iSaveImageCT=0;
            break;
        case 4:
#if 0 // GATE(W906-STATEREC-G2) 同上（GPIB.bmp：把 GPIB 橋接程式的視窗叫到前面再抓）
            SetForegroundWindow(HVisionWnd);                                    //Richard 20230418 : 有時沒切到畫面
            BitBlt(ImageRecord->Canvas->Handle,0,0,iScrW,iScrH,GetDC(0),0,0,SRCCOPY);
            ImageRecord->Picture->SaveToFile(NewPath+"\\GPIB.bmp");
#endif // GATE(W906-STATEREC-G2)
            iSaveImageTask++;
            iSaveImageCT=0;
            break;
        case 5:
#if 0 // GATE(W906-STATEREC-G2) 畫面還原：缺 Application／ImageRecord／fNote->BringToFront／MainFormSizeToEpson／sbStateRecord／sbStateRecord2／sbReturnToMainClick（forms/fMain.h:64-66 記載 sbStateRecord 刻意沒加）
            Application->BringToFront();
            ImageRecord->Height=10;
            ImageRecord->Width=10;
            if(fNote->fShow)                                                    //Steven 20100825
            {
                fNote->BringToFront();
                MainFormSizeToEpson(true);
            }
            sbStateRecord->Down=false;
#endif // GATE(W906-STATEREC-G2)
            iSaveImageTask++;
            iSaveImageCT=0;

#if 0 // GATE(W906-STATEREC-G2) 同上
            sbStateRecord2->Down=false;
            sbReturnToMainClick(this);
#endif // GATE(W906-STATEREC-G2)
            break;
        case 6:
            if(iSaveImgae==1)
            {
                ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
            }
            else if(iSaveImgae==2)
            {
                ShowErrorMessage("JAM0317", K_SKIP, MTestZ2);
            }
            else if(iSaveImgae==3)
            {
                ShowMyMessage("Not support Hot Plate matrix!!", "不支援的加熱盤格式");
            }

            iSaveImgae=-1;
            iSaveImageTask++;
            iSaveImageCT=0;

#if 0 // MOVED-BG(W906-STATEREC) 使用者 20260924 裁決：7z／Del_Tree／開資料夾交給背景執行緒，見本檔 RunStateRecordJob（不是缺相依的閘）
            if(FileExists("d:\\HT9045\\7z.exe"))                                //KenHsieh 20230105 : 新增StateRecord 另存路徑
            {
                str1.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s.zip\" \"%s\"", NewPath, NewPath);
                iret=system(str1.c_str());                                      //壓縮StateRecord
                MySleep(500);
                if(iret==0)                                                     //壓縮成功
                    Del_Tree(NewPath);                                          //刪除目錄
            }

            if(bManualStateRecord)                                              //KenHsieh 20230105 : 新增StateRecord 另存路徑
                ShellExecute(NULL, "open", SDataPath.c_str(), NULL, NULL, SW_SHOW);     //顯示目錄
#endif // MOVED-BG(W906-STATEREC)
            break;
    }
    (void)str1; (void)iret;                                                     // 只剩 MOVED-BG 段在用
}

//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260604: 將原本內嵌在 DoStateRecord 的 MainForm 變數快照獨立成函式,
// 方便辨識與維護; 並新增 Loader/TrayArm 交接診斷區, 記錄 WAR16122 空盤無法自動回收
// (InArm InArmPickFromLoadTask case10<->15 silent loop) 必看的旗標,
// 解決 Task_ListWithTime.csv 無變數值無法判定觸發分支的問題.
//  AI(W906-STATEREC) 20260924: golden main.cpp:26214-26339，逐字翻譯。
void DumpMainFormSnapshot(AnsiString sBasePath)
{
    try
    {
        AnsiString sSnapPath = sBasePath + "\\MainFormSnapshot.txt";
        TStringList *slSnap = new TStringList;
        try
        {
            AnsiString s;
            slSnap->Add("==== HT9045 MainForm Snapshot ====");
            s.sprintf("Time: %04d-%02d-%02d %02d:%02d:%02d",
                      SystemYear, SystemMonth, SystemDate,
                      SystemHour, SystemMin, SystemSec);
            slSnap->Add(s);
            slSnap->Add("");

            slSnap->Add("---- Task ----");
            s.sprintf("InArm.iArmTask=%d OutArmTask=%d", iArmTask, OutArmTask);
            slSnap->Add(s);
            s.sprintf("AutoSHT1Task=%d AutoSHT2Task=%d", AutoSHT1Task, AutoSHT2Task);
            slSnap->Add(s);
            s.sprintf("iPickFromShuttle1Task=%d iPickFromShuttle2Task=%d",
                      iPickFromShuttle1Task, iPickFromShuttle2Task);
            slSnap->Add(s);
            slSnap->Add("");

            slSnap->Add("---- InArmSuck ----");
            s.sprintf("iModeX=%d iXStep=%d iYStep=%d iPickRow=%d iPickCol=%d iMaxRow=%d iMaxCol=%d",
                      InArmSuck.iModeX, InArmSuck.iXStep, InArmSuck.iYStep,
                      InArmSuck.iPickRow, InArmSuck.iPickCol,
                      InArmSuck.iMaxRow, InArmSuck.iMaxCol);
            slSnap->Add(s);
            for(int i=0; i<InArmSuck.iMaxRow && i<_MAX_SUCK_ROW_ITEM; i++)
            {
                AnsiString sItem="Item r"+IntToStr(i)+":";
                AnsiString sNeed="Need r"+IntToStr(i)+":";
                for(int j=0; j<InArmSuck.iMaxCol && j<_MAX_SUCK_COL_ITEM; j++)
                {
                    sItem += " " + IntToStr(InArmSuck.Item[i][j]);
                    sNeed += " " + IntToStr(InArmSuck.Suck[i][j].GetNeedSuckStatus()?1:0);
                }
                slSnap->Add(sItem);
                slSnap->Add(sNeed);
            }
            slSnap->Add("");

            slSnap->Add("---- OutArmSuck ----");
            s.sprintf("iModeX=%d iXStep=%d iYStep=%d iPickRow=%d iPickCol=%d iMaxRow=%d iMaxCol=%d",
                      OutArmSuck.iModeX, OutArmSuck.iXStep, OutArmSuck.iYStep,
                      OutArmSuck.iPickRow, OutArmSuck.iPickCol,
                      OutArmSuck.iMaxRow, OutArmSuck.iMaxCol);
            slSnap->Add(s);
            for(int i=0; i<OutArmSuck.iMaxRow && i<_MAX_SUCK_ROW_ITEM; i++)
            {
                AnsiString sItem="Item r"+IntToStr(i)+":";
                AnsiString sNeed="Need r"+IntToStr(i)+":";
                for(int j=0; j<OutArmSuck.iMaxCol && j<_MAX_SUCK_COL_ITEM; j++)
                {
                    sItem += " " + IntToStr(OutArmSuck.Item[i][j]);
                    sNeed += " " + IntToStr(OutArmSuck.Suck[i][j].GetNeedSuckStatus()?1:0);
                }
                slSnap->Add(sItem);
                slSnap->Add(sNeed);
            }
            slSnap->Add("");

            slSnap->Add("---- Shuttle Item (FL/FR/BL/BR) ----");
            int iShRow = FRCarryKit.iMaxRow;
            int iShCol = FRCarryKit.iMaxCol;
            for(int i=0; i<iShRow && i<_MAX_SUCK_ROW_ITEM; i++)
            {
                AnsiString sFL="FL r"+IntToStr(i)+":";
                AnsiString sFR="FR r"+IntToStr(i)+":";
                AnsiString sBL="BL r"+IntToStr(i)+":";
                AnsiString sBR="BR r"+IntToStr(i)+":";
                for(int j=0; j<iShCol && j<_MAX_SUCK_COL_ITEM; j++)
                {
                    sFL += " " + IntToStr(FLCarryKit.Item[i][j]);
                    sFR += " " + IntToStr(FRCarryKit.Item[i][j]);
                    sBL += " " + IntToStr(BLCarryKit.Item[i][j]);
                    sBR += " " + IntToStr(BRCarryKit.Item[i][j]);
                }
                slSnap->Add(sFL);
                slSnap->Add(sFR);
                slSnap->Add(sBL);
                slSnap->Add(sBR);
            }

            //AI(ht9045-v899) 20260604: Loader / TrayArm 交接診斷區
            // WAR16122 空盤無法自動回收時, 用來判定卡在哪個交接分支:
            //   branch A = 空盤狀態落在 Car/Z (fHasTray) 但 MMTrayY.fHasTray=false
            //              -> InArm 視為有盤 (case15 三位置 OR), TrayArm 只看 MMTrayY -> silent loop
            //   branch B = manual-remove 旗標已設, 但 iInArmWaitPosition != 3 -> 交接前置 gate 永久 break
            slSnap->Add("");
            slSnap->Add("---- Loader / TrayArm Handoff Diag ----");
            s.sprintf("iPickFromLoadStageTask=%d CatchTrayTask=%d iCatchFromLoaderTask=%d iTrayZLoadTrayToWaitTask=%d",
                      iPickFromLoadStageTask, CatchTrayTask, iCatchFromLoaderTask, iTrayZLoadTrayToWaitTask);
            slSnap->Add(s);
            s.sprintf("MMTrayY.fHasTray=%d MMTrayY.HasIC=%d MMTrayY_Car.fHasTray=%d MMTrayZ.fHasTray=%d",
                      MOT[MMTrayY].fHasTray?1:0, MOT[MMTrayY].Tray.HasIC()?1:0,
                      MOT[MMTrayY_Car].fHasTray?1:0, MOT[MMTrayZ].fHasTray?1:0);
            slSnap->Add(s);
            s.sprintf("iInArmWaitPosition=%d (0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2)", iInArmWaitPosition);
            slSnap->Add(s);
            s.sprintf("bNeedManualRemoveTray=%d bLoaderHasSkip=%d iManualRemoveLoader=%d iManualRemoveTrayCnt=%d iLDTrayNeedManualRemoveTray=%d",
                      bNeedManualRemoveTray?1:0, bLoaderHasSkip?1:0,
                      TrayForm.iManualRemoveLoader, iManualRemoveTrayCnt, iLDTrayNeedManualRemoveTray);
            slSnap->Add(s);

            slSnap->SaveToFile(sSnapPath);
        }
        catch(...)                                                              // golden 是 __finally：清掉再往外丟（C++ 沒有 __finally，改成 catch 後 delete 再 rethrow）
        {
            delete slSnap;
            throw;
        }
        delete slSnap;
    }
    catch(...)
    {
        // never let snapshot dump break state record
    }
}

// =============================================================================
//  TfMain::DoStateRecord 的本體  --  golden main.cpp:26340-26678
//
//  逐字翻譯；三類改寫，逐處註明：
//   (a) CopyFile／SHFileOperation／寫並執行 1.bat 改成把「要做的事」記進 StateRecordJob，
//       由背景執行緒依 golden 的順序執行（使用者 20260924 的執行模型）。判斷（FileExists、客戶碼、
//       測試介面分支、ATK 目錄列舉）仍在呼叫當下同步做，所以「要複製哪些檔」是觸發那一刻決定的。
//   (b) 缺相依的閘（GATE(W906-STATEREC-G1..G9)，本函式用到 G2..G9），理由寫在閘上。
//   (c) 移植樹自己的「不重疊」守衛（任務要求）：背景工作還在跑時，第二次呼叫**不建新資料夾、不排新工作**，
//       但 golden 會影響機台行為的幾件事照做 —— bNoUseAutoRecord=false、SetCurrentDirectory、SaveMachineRecord()、
//       以及延後警報的武裝（iSaveImageTask=1／iSaveImgae=iShowAlarm），否則 hang-up 呼叫端的
//       JAM0316／JAM0317 會因為撞上前一筆錄製而消失。
// =============================================================================
void TfMain::W906_DoStateRecordBody(int iShowAlarm, bool bManual)              //Steven 20220716 : 把State record獨立出來, 避免抓圖的時候被Alarm擋住  //KenHsieh 20230116 : 區分手動或自動
{
    RecordProcess("State Record.");
    #if 0   // AI(W906-STATEREC-SIM) 20260924: 使用者裁決「模擬組態也錄」—— 刻意偏離 golden main.cpp:26343 的 #ifdef SOFT_SIMULTE（golden 模擬時整段不錄）；出貨組態行為不變
    #else
    // --- (c) 不重疊守衛（移植樹專有） ----------------------------------------
    if(W906_StateRecordWorkerBusy.load())
    {
        RecordProcess("State Record skipped: previous recording is still copying/compressing", NewPath);
        bNoUseAutoRecord=false;                                                 // golden :26345
        SetCurrentDirectory("D://");                                            // golden :26346（_T() 在 ANSI 建置等於原字串，同 cprod.cpp:2249）
        SaveMachineRecord();                                                    // golden :26347
        iSaveImageTask=1;                                                       // golden :26669
        iSaveImgae=iShowAlarm;                                                  // golden :26670
        return;                                                                 // 不做 LogIndexMaxMinPos／DumpMainFormSnapshot：NewPath 是前一筆、正在被壓縮的資料夾
    }
    StateRecordJob job;

    bNoUseAutoRecord=false;                                                     //wei 20160311
    SetCurrentDirectory("D://");                                                // golden SetCurrentDirectory(_T("D://")) -- _T() neutralized (ANSI build)
    SaveMachineRecord();                                                        //Steven 20171206 (Wei) : Hang Up前要存一次

    GetTimeInfo();
    AnsiString OrgPath;
    AnsiString asLastFileName;
    AnsiString str, str1;
    SDataPath="D:\\HT9045_StateRecord\\";                                       //Steven 20150520 : 更換State Record目錄
    bManualStateRecord=false;                                                   //KenHsieh 20230105 : 新增StateRecord 另存路徑
    bool bChangePathFlag=false;                                                 //KenHsieh 20230116 : 區分手動或自動
#if 0 // GATE(W906-STATEREC-G3) UpdateTaskList（golden main.cpp:6381-6400）只把 QueueTaskList 寫進 sgTaskList（golden main.h:553 TStringGrid），
      //   sgTaskList 移植樹沒有（Grep forms/fMain.h 0 命中；WebStart.cpp:3861 的 W906-T3-PAUSE-3 閘同一件事），而它唯一的消費者是下面的
      //   SGDToXLS(sgTaskList, Task_List.xls) —— SGDToXLS 在移植樹本身就是 no-op（SgdToXLS.cpp:88-94）。同一批資料由 SaveTaskList 寫進 Task_ListWithTime.csv。
    UpdateTaskList();
#endif // GATE(W906-STATEREC-G3)

    QueueGalilCmd.SafeData();                                                   //JerryYang 20250812 : 存state record要自動存galil log

    if(bManual==true)                                                           //KenHsieh 20230105 : 新增StateRecord 另存路徑   //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)
    {
        bManualStateRecord=true;
        fMain->ProcessTimeUpdate(true);
#if 0 // GATE(W906-STATEREC-G4) SaveDialog1（golden main.h:129 TSaveDialog）移植樹沒有（forms/fMain.h 0 命中）；網頁模式下 wb_serve 也沒有可彈的檔案對話框。
      //   效果 = 操作員在 golden 的另存對話框按「取消」：bChangePathFlag 維持 false，走下面的預設路徑 SDataPath+sFileNameTime。
        SaveDialog1->InitialDir=SDataPath;
        SaveDialog1->FileName=sFileNameTime;

        if(SaveDialog1->Execute())
        {
            NewPath=SaveDialog1->FileName;
            bChangePathFlag=true;
            SDataPath=ExtractFilePath(NewPath);
        }
#endif // GATE(W906-STATEREC-G4)
    }

    if(bChangePathFlag==false)
    {
        fMain->ProcessTimeUpdate(true);
        NewPath=SDataPath+sFileNameTime;                                        //Steven 20190801 : 修正檔名錯誤
    }

    MNetLog("Close");                                                           //Steven 20110418 : Hang Up前要存一次

    MyForceDirectories(NewPath);
    MyForceDirectories(NewPath+"\\HT9045\\IniData\\Data");
    MyForceDirectories(NewPath+"\\HT9045\\system");
    MyForceDirectories(NewPath+"\\HT9045\\config");
    MyForceDirectories(NewPath+"\\GPIB9045\\system");
    MyForceDirectories(NewPath+"\\GPIBLOG");

    fMain->SendMSG_CMD(MSG_CMD_State_Record);                                   //wei 20170911 (steven) State Record

    if(FileExists("d:\\HT9045\\7z.exe")==false)                                 //Steven 20110222 : 改成先把log給壓縮然後才複製
    {
        job.copies.push_back(std::make_pair(std::string("C:\\Program Files\\7-Zip\\7z.exe"), std::string("d:\\HT9045\\7z.exe")));   // (a) golden CopyFile(..., false)
    }

    //Steven 20170321 (wei) : Add eventlog in state record
    //Steven 20260504 : Copy current active EventLog file immediately (handles open file)
    //                  Different customers may use different folder structures (TByHour/TByDay/FixedFile/LotID)
    //                  robocopy from root with /MAXAGE:2 covers all structures (added to TestList below)
    MyForceDirectories(NewPath+"\\EventLogTxt");
    // 移植樹專有的 NULL 守衛：golden 在 TfMain 建構子 new 出 slEventLog，移植樹從來沒有建它
    // （Grep 全樹 *.cpp 的 `slEventLog\s*=`：只有 tests/ 兩支），wb_serve 裡它恆為 NULL。
    // 同 cMyDB.cpp:342／:398／:446 的 `if(slEventLog!=NULL)` 作法；NULL 時 str1="" ⇒ 下面 FileExists 為假，不複製（等於 golden 沒有作用中檔案）。
    str1=(slEventLog!=NULL) ? slEventLog->GetFileName() : AnsiString("");       //Steven 20200303 : 修正State Record裡面的Event Log檔名
    if(FileExists(str1))
    {
        AnsiString sDestEvt;
        sDestEvt.sprintf("%s\\EventLogTxt\\%s", NewPath, ExtractFileName(str1));
        job.copies.push_back(std::make_pair(std::string(str1.c_str()), std::string(sDestEvt.c_str())));             // (a) golden CopyFile(str1, sDestEvt, false)
    }

    //Steven 20260505 : Copy active MNetLog file immediately (sync); robocopy covers the rest in TestList
    //                  MNetLog uses TByDay+bFilePathWithDate => YYYY\MM subfolder, robocopy from root handles this
    MyForceDirectories(NewPath+"\\MNetLog");
#if 0 // GATE(W906-STATEREC-G5) slMNetLog（golden TfMain 的 TMyStringList*）移植樹沒有：Grep 全樹 *.cpp/*.h 0 個宣告；
      //   寫它的 MNetLog() 本體也整段閘住（Motor/myMN200motor.cpp:2512-2530 GATE (g)）。下面 TestList 的 robocopy MNetLog 照留。
    str1=slMNetLog->GetFileName();
    if(FileExists(str1))
    {
        AnsiString sDestMNet;
        sDestMNet.sprintf("%s\\MNetLog\\%s", NewPath, ExtractFileName(str1));
        CopyFile(str1.c_str(), sDestMNet.c_str(), false);
    }
#endif // GATE(W906-STATEREC-G5)

    str1="D:\\HT9045\\EXE\\HT9045.elf";                                         //Steven 20200826 : 自動複製出ELF檔案
    if(FileExists(str1))
        job.copies.push_back(std::make_pair(std::string(str1.c_str()), std::string(AnsiString(NewPath+"\\HT9045.elf").c_str())));      // (a)

    str1="D:\\HT9045\\setup.inf";                                               //Steven 20230316 : State Record複製setup.inf
    if(FileExists(str1))
        job.copies.push_back(std::make_pair(std::string(str1.c_str()), std::string(AnsiString(NewPath+"\\HT9045\\setup.inf").c_str()))); // (a)

    // 複製工作檔（golden :26430-26440 的 SHFileOperation(FO_COPY)）—— (a) 來源／目的在這裡決定，背景執行緒執行
    asLastFileName=GetLastOpenFN();
    str1=DataPath+asLastFileName;
    job.shFrom=str1.c_str();
    job.shTo=AnsiString(NewPath+"\\HT9045\\IniData\\Data").c_str();

    TStringList *TestList=new TStringList;

    //Steven 20260504 : EventLogTxt robocopy from root (supports all customer folder structures)
    //                  /S covers YYYY\MM\DD subfolders; /MAXAGE:2 limits to last 2 days
    str.sprintf("robocopy \"D:\\HT9045_Log\\EventLogTxt\" \"%s\\EventLogTxt\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP", NewPath);
    TestList->Add(str);

    str1="D:\\HT9045\\system";                                                  //複製LastSet
    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", str1, NewPath+"\\HT9045\\system");
    TestList->Add(str);

    str1="D:\\HT9045\\config";                                                  //複製Config.ini
    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", str1, NewPath+"\\HT9045\\config");
    TestList->Add(str);

    str1="D:\\GPIB9045\\system";                                                //複製GPIB設定
    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", str1, NewPath+"\\GPIB9045\\system");
    TestList->Add(str);

    //Steven 20260504 : Change log copy from whole month to last 2 days using robocopy
    AnsiString ActivePath11="";
    AnsiString ActivePath12="";
    int iPrevYear=SystemYear, iPrevMonth=SystemMonth-1;
    if(iPrevMonth==0){ iPrevMonth=12; iPrevYear--; }

    if(TestIF_File.iTestType==GPIB_MODE)
    {
        MyForceDirectories(NewPath+"\\GPIBLOG");
        ActivePath11.sprintf("%s\\%04d_%02d", "D:\\GPIBLOG\\Log", SystemYear, SystemMonth);
        str.sprintf("robocopy \"%s\" \"%s\\GPIBLOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath11, NewPath);
        TestList->Add(str);
        if(SystemDate <= 2)
        {
            ActivePath12.sprintf("%s\\%04d_%02d", "D:\\GPIBLOG\\Log", iPrevYear, iPrevMonth);
            str.sprintf("robocopy \"%s\" \"%s\\GPIBLOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath12, NewPath);
            TestList->Add(str);
        }
    }
    else if(TestIF_File.iTestType==TCP_IP_MODE)                                 //Steven 20240625 : Add TCP IP Log for state record
    {
        MyForceDirectories(NewPath+"\\TCP_LOG");
        ActivePath11.sprintf("%s\\%04d_%02d", asTestTCPIPLogPath, SystemYear, SystemMonth);
        str.sprintf("robocopy \"%s\" \"%s\\TCP_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath11, NewPath);
        TestList->Add(str);
        if(SystemDate <= 2)
        {
            ActivePath12.sprintf("%s\\%04d_%02d", asTestTCPIPLogPath, iPrevYear, iPrevMonth);
            str.sprintf("robocopy \"%s\" \"%s\\TCP_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath12, NewPath);
            TestList->Add(str);
        }
    }
    else
    {
        MyForceDirectories(NewPath+"\\TTL_LOG");
        ActivePath11.sprintf("%s\\%04d_%02d", "D:\\RS232Log\\Log_TTL", SystemYear, SystemMonth);
        str.sprintf("robocopy \"%s\" \"%s\\TTL_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath11, NewPath);
        TestList->Add(str);
        if(SystemDate <= 2)
        {
            ActivePath12.sprintf("%s\\%04d_%02d", "D:\\RS232Log\\Log_TTL", iPrevYear, iPrevMonth);
            str.sprintf("robocopy \"%s\" \"%s\\TTL_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath12, NewPath);
            TestList->Add(str);
        }

        MyForceDirectories(NewPath+"\\RS232_LOG");
        ActivePath11.sprintf("%s\\%04d_%02d", "D:\\RS232Log\\Log", SystemYear, SystemMonth);
        str.sprintf("robocopy \"%s\" \"%s\\RS232_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath11, NewPath);
        TestList->Add(str);
        if(SystemDate <= 2)
        {
            ActivePath12.sprintf("%s\\%04d_%02d", "D:\\RS232Log\\Log", iPrevYear, iPrevMonth);
            str.sprintf("robocopy \"%s\" \"%s\\RS232_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NDL /NC /NS /NP", ActivePath12, NewPath);
            TestList->Add(str);
        }
    }

    //Steven 20260504 : Add Galil_Log and MNetLog (last 2 days)
    //Steven 20260505 : robocopy from root dir; /S /MAXAGE:2 handles all subfolder structures
    //                  Galil uses YYYYMM subdir; MNetLog uses YYYY\MM subdir (TByDay+bFilePathWithDate)
    //                  month-boundary handled by /MAXAGE:2 naturally
    MyForceDirectories(NewPath+"\\Galil_LOG");
    str.sprintf("robocopy \"%s\" \"%s\\Galil_LOG\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP", asGalilCmdPath.c_str(), NewPath);
    TestList->Add(str);

    str.sprintf("robocopy \"D:\\HT9045_Log\\MNetLog\" \"%s\\MNetLog\" /S /MAXAGE:2 /R:0 /W:0 /NJH /NJS /NC /NS /NP", NewPath);
    TestList->Add(str);

    // golden fAutomation->SaveRecord()。golden 的 fAutomation 是隨程式建立的 OLP 表單；移植樹的真引擎叫
    // fAutomationEngine（全域名 fAutomation 被 atester_shims.h:334 的 shim 佔用，見 automation.h:81-99），
    // 而且是懶建構 —— NULL 就代表 OLP 引擎從沒被建起來、沒有任何 OLP 紀錄可存（automation.cpp:2795
    // 記載目前沒有人呼叫 AutomationEngine()）。所以只在它存在時呼叫；不在這裡把它建起來（那會開 OLP socket）。
    if(fAutomationEngine!=NULL)
        fAutomationEngine->SaveRecord();
    if(CosFunction.bOLPFunction && IniConfig.bN08_1SaveOLPLog)                  //Sam 20200826 : Add Automation Log
    {
        //D:\HT9045_Log\Automation Log複製
        ActivePath11.sprintf("%s\\%04d_%02d","D:\\HT9045_Log\\Automation", SystemYear, SystemMonth);
        str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s%04d_%02d\"", ActivePath11, NewPath+"\\Automation\\", SystemYear, SystemMonth);
        TestList->Add(str);

        //D:\HT9045_Log\Automation Log壓縮
        str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s\\%04d_%02d.7z\" \"%s\\%04d_%02d\\*.*\"", NewPath+"\\Automation", SystemYear, SystemMonth, NewPath+"\\Automation",SystemYear, SystemMonth);
        TestList->Add(str);

        //D:\HT9045_Log\Automation Log刪除
        str.sprintf("RMDIR /s/q \"%s\\%04d_%02d\"", NewPath+"\\Automation", SystemYear, SystemMonth);
        TestList->Add(str);
    }

    if(bRunAutoClean && TestIF_File.iAutoClean_Function)                        //Sam 20230616 : Add Auto Clean Record
    {
        for(int iRow=0; iRow<2 ;iRow++)
        {
            for(int iCol=0; iCol<4 ;iCol++)
            {
                if(InArmSuck.Item[iRow][iCol]!=NULL_IC)
                {
                    InArmSuck.PordRec[iRow][iCol].SaveRecordCleanPad("StateRecord");
                }
            }
        }

        //D:\HT9045_Log\CleanPad_Log Log複製
        ActivePath11.sprintf("%s\\%04d%02d","D:\\HT9045_Log\\CleanPad_Log", SystemYear, SystemMonth);
        str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s%04d%02d\"", ActivePath11, NewPath+"\\CleanPad_Log\\", SystemYear, SystemMonth);
        TestList->Add(str);

        //D:\HT9045_Log\CleanPad_Log Log壓縮
        // golden 照翻：壓縮的來源寫成 NewPath\Automation\YYYYMM\*.*，不是 CleanPad_Log（golden :26565 的複製貼上瑕疵）
        str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s\\%04d%02d.7z\" \"%s\\%04d%02d\\*.*\"", NewPath+"\\CleanPad_Log", SystemYear, SystemMonth, NewPath+"\\Automation",SystemYear, SystemMonth);
        TestList->Add(str);

        //D:\HT9045_Log\CleanPad_Log Log刪除
        str.sprintf("RMDIR /s/q \"%s\\%04d%02d\"", NewPath+"\\CleanPad_Log", SystemYear, SystemMonth);
        TestList->Add(str);
    }

    //==> ATK-AMR: SECS Log (today, max 5) + ATK Log (today)
    if(fAGV->IsATK_AMR())                                                      //AI(ht9045-atk-amr-flow) 20260428 (RogerYang) : ATK-AMR state record add SECS/ATK logs
    {
        AnsiString asSecsDir, asSecsDestDir;
        asSecsDir.sprintf("D:\\SECS_GEM_LOGS\\%04d\\%02d_%02d",
            SystemYear, SystemMonth, SystemDate);
        if(DirectoryExists(asSecsDir))
        {
            asSecsDestDir = NewPath + "\\SECS_GEM_LOGS";
            MyForceDirectories(asSecsDestDir);
            TSearchRec sr;
            int iSecsCount = 0;
            if(FindFirst(asSecsDir + "\\*.txt", faAnyFile, sr) == 0)
            {
                do
                {
                    if(iSecsCount >= 5) break;
                    job.copies.push_back(std::make_pair(std::string(AnsiString(asSecsDir + "\\" + sr.Name).c_str()),          // (a) golden CopyFile(..., false)
                                                        std::string(AnsiString(asSecsDestDir + "\\" + sr.Name).c_str())));
                    iSecsCount++;
                } while(FindNext(sr) == 0);
                FindClose(sr);
            }
        }

        AnsiString asAtkDir, asAtkSrc, asAtkDest, asAtkDestDir;
        asAtkDir.sprintf("D:\\HT9045_Log\\ATKDataTxt\\%04d%02d\\%02d%02d",
            SystemYear, SystemMonth, SystemMonth, SystemDate);
        asAtkSrc.sprintf("%s\\%04d%02d%02d.txt",
            asAtkDir, SystemYear, SystemMonth, SystemDate);
        if(FileExists(asAtkSrc))
        {
            asAtkDestDir = NewPath + "\\ATKDataTxt";
            MyForceDirectories(asAtkDestDir);
            asAtkDest.sprintf("%s\\%04d%02d%02d.txt",
                asAtkDestDir, SystemYear, SystemMonth, SystemDate);
            job.copies.push_back(std::make_pair(std::string(asAtkSrc.c_str()), std::string(asAtkDest.c_str())));      // (a)
        }
    }
    //<==

    AnsiString BatFile="D:\\HT9045\\system\\1.bat";                             //Steven 20210308 : 變更Batch檔位置

    // golden :26617-26627：try { 刪舊 1.bat；TestList->SaveToFile(BatFile)；ExecZipCommand(BatFile, " "); } catch(...){}
    // (a) 三個動作都搬到背景執行緒（RunStateRecordJob），這裡只交出內容。
    job.batFile=BatFile.c_str();
    for(int i=0; i<TestList->Count; i++)
        job.batLines.push_back(std::string(TestList->Strings[i].c_str()));
    TestList->Clear();
    delete TestList;

#if 0 // GATE(W906-STATEREC-G6) UpdateMotorScreen（golden main.cpp:8278-8540）與 StringGrid1（golden main.h:141）移植樹都沒有（Grep 0 命中）；
      //   它們只餵 SGDToXLS(StringGrid1, Motor.xls)，而 SGDToXLS 在移植樹是 no-op（SgdToXLS.cpp:88-94，XLSFile.pas 沒有 C++ 本體）。
      //   馬達資料另有 SaveTaskList 的 MOT[] 段（Task_ListWithTime.csv）。
    UpdateMotorScreen(true);                                                    //Steven 20210902 : 修正抓State Record時,要先更新馬達狀態
    SGDToXLS(StringGrid1, NewPath+"\\Motor.xls");                               //jou 2011-11-21 儲存Motor information
#endif // GATE(W906-STATEREC-G6)
    SGDToXLS(StringGrid2, NewPath+"\\Task.xls");
#if 0 // GATE(W906-STATEREC-G3) sgTaskList 不存在（見上方 UpdateTaskList 的閘）
    SGDToXLS(sgTaskList, NewPath+"\\Task_List.xls");
#endif // GATE(W906-STATEREC-G3)
    SGDToXLS(AutoCleanStringGrid, NewPath+"\\AutoClean.xls");                   //Steven 20200422 : State record紀錄Auto Clean狀態
    SaveTaskList(NewPath);                                                      //Steven 20200304 : 儲存紀錄Task+time的方式
    SaveDecisionVariables(NewPath);

    if(LastSet.iTemperature==Tempture_Hot)                                      //Steven 20160624 : 保存加熱盤資料
    {
#if 0 // GATE(W906-STATEREC-G7) rgHotplateShowMessage（golden main.h:807 TRadioGroup）、ProcessICHotTime（golden main.cpp:7907）、
      //   StringGrid4／StringGrid5（golden main.h:520／:517）移植樹都沒有（Grep forms/fMain.h 0 命中）；它們只餵 SGDToXLS（移植樹 no-op）。
        rgHotplateShowMessage->ItemIndex=0;
        ProcessICHotTime(true);                                                 //JerryYang 20160909 存state record的時候要做完,避免Hot plate缺資料
        SGDToXLS(StringGrid4, NewPath+"\\HP2_HotTime.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_HotTime.xls");
        rgHotplateShowMessage->ItemIndex=1;
        ProcessICHotTime(true);                                                 //JerryYang 20160909 存state record的時候要做完,避免Hot plate缺資料
        SGDToXLS(StringGrid4, NewPath+"\\HP2_WhichShuttle.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_WhichShuttle.xls");
        rgHotplateShowMessage->ItemIndex=2;
        ProcessICHotTime(true);                                                 //JerryYang 20160909 存state record的時候要做完,避免Hot plate缺資料
        SGDToXLS(StringGrid4, NewPath+"\\HP2_WhichKit.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_WhichKit.xls");
        rgHotplateShowMessage->ItemIndex=3;
        ProcessICHotTime(true);                                                 //JerryYang 20160909 存state record的時候要做完,避免Hot plate缺資料
        SGDToXLS(StringGrid4, NewPath+"\\HP2_Count.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_Count.xls");
        rgHotplateShowMessage->ItemIndex=4;                                     //JerryYang 20180718 (wei) : 放料至hot plate記錄吸嘴位置
        ProcessICHotTime(true);
        SGDToXLS(StringGrid4, NewPath+"\\HP2_Row.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_Row.xls");
        rgHotplateShowMessage->ItemIndex=5;                                     //JerryYang 20180718 (wei) : 放料至hot plate記錄吸嘴位置
        ProcessICHotTime(true);
        SGDToXLS(StringGrid4, NewPath+"\\HP2_Site.xls");
        SGDToXLS(StringGrid5, NewPath+"\\HP1_Site.xls");
#endif // GATE(W906-STATEREC-G7)
    }

#if 0 // GATE(W906-STATEREC-G2) MainFormSizeToEpson（golden main.cpp:8541）與 fShowMessage->FormClick（forms/fShowMessage.h 只有 ShowSpeed）移植樹沒有；純視窗操作
    MainFormSizeToEpson(true);
    fShowMessage->FormClick(fShowMessage);
#endif // GATE(W906-STATEREC-G2)
    iSaveImageTask=1;
    iSaveImgae=iShowAlarm;

    fObserver->Memo1->Lines->SaveToFile(NewPath+"\\Ver.txt");                   //jou 2011-05-26
    RecordIndexPosition(0,3);
    job.newPath=NewPath.c_str();
    job.sDataPath=SDataPath.c_str();
    job.bManual=bManualStateRecord;
    (void)OrgPath;                                                              // golden 宣告了沒用（:26351），照留
    #endif
#if 0 // GATE(W906-STATEREC-G9) LogIndexMaxMinPos 在 cpublic.h:327 有宣告但本體整段在 #if 0 內（cpublic.cpp:1816 GA1-B3：缺 fMain->slIndexYMaxMinShift／AddIndexPosLog）；
      //   量法：nm HT9011UC_Cpp_V3.33.906.0/build_night 的全部 .a 與 wb_serve 物件檔，這個符號沒有任何定義 ⇒ 呼叫就是 link error。asendic_Loader.cpp:273 對同一個缺件用 TU 內空殼。
    LogIndexMaxMinPos("StateRecord");                                           //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，記錄並存檔
#endif // GATE(W906-STATEREC-G9)
    //AI(ht9045-v899) 20260604: 改呼叫獨立函式 DumpMainFormSnapshot (內含 Loader/TrayArm 交接診斷區)
    DumpMainFormSnapshot(NewPath);
    #if 1   // AI(W906-STATEREC-SIM) 20260924: 同 :1017 —— 模擬組態也交給背景執行緒複製／壓縮
    // (a) 所有快照都寫完了才交給背景：7z 壓的是整個 NewPath，快照必須已經在裡面。
    //     Win32 CreateThread 而不是 std::thread：本樹的 MinGW 6.3（主 oracle）是 win32 執行緒模型，
    //     沒有 std::thread（vclcompat/Comm.cpp:18、tools/wb_serve.cpp:204 同一個理由）。
    W906_StateRecordWorkerBusy.store(true);
    StateRecordJob *pJob=new StateRecordJob(job);
    DWORD tid=0;
    HANDLE h=::CreateThread(NULL, 0, &StateRecordWorkerProc, pJob, 0, &tid);
    if(h!=NULL)
    {
        ::CloseHandle(h);                                                       // detached：執行緒結束時自己清 busy
    }
    else
    {
        delete pJob;
        W906_StateRecordWorkerBusy.store(false);
        RecordProcess("State Record: CreateThread failed; logs/config NOT copied, folder NOT zipped", NewPath);
    }
    #endif
}

// =============================================================================
//  背景工作：golden 的複製、批次、7z，依 golden 的順序。
//  ⚠ 這條執行緒只碰 job 裡的字串與 Win32 檔案 API；唯二呼叫的移植樹函式：
//    * RecordProcess —— 目前活的本體是 canary_support.cpp:116 的 printf，執行緒安全。
//      ⚠ 若 cMyDB.cpp:1831 的 golden 本體 homecoming（寫 ExString＋MyDBIProcess），這裡要改成回報給 tick 執行緒。
//    * Del_Tree（csystem.cpp:27726）—— 純遞迴刪檔，不碰全域。
// =============================================================================
namespace {

void RunStateRecordJob(StateRecordJob job)
{
    RecordProcess("State Record worker start", AnsiString(job.newPath.c_str()));

    // golden 的 CopyFile(src, dst, false)，順序同 golden（7z.exe、EventLog、elf、setup.inf、ATK）
    for(std::size_t i=0; i<job.copies.size(); i++)
        ::CopyFileA(job.copies[i].first.c_str(), job.copies[i].second.c_str(), FALSE);

    // golden :26430-26440 SHFileOperation(FO_COPY) 複製工作檔；hwnd 在 golden 是表單的 Handle，這裡沒有視窗 ⇒ NULL
    if(!job.shFrom.empty())
    {
        SHFILEOPSTRUCT oFile;
        char cStr1[256]="", cStr2[256]="";                                      // golden :26350（整個陣列歸零 ⇒ 字串後面自帶雙 NUL）
        ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
        oFile.hwnd=NULL;
        oFile.wFunc=FO_COPY;
        strncpy(cStr1, job.shFrom.c_str(), sizeof(cStr1));
        oFile.pFrom=cStr1;
        strncpy(cStr2, job.shTo.c_str(), sizeof(cStr2));
        oFile.pTo=cStr2;
        oFile.fFlags=FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI;
        SHFileOperation(&oFile);
    }

    // golden :26617-26627：刪舊 1.bat → TestList->SaveToFile → ExecZipCommand(BatFile, " ")
    try                                                                         //Steven 20210308 : 幫Batch檔加上Try Catch, 避免出現無法存檔的狀況
    {
        AnsiString BatFile=job.batFile.c_str();
        if(FileExists(BatFile))
            DeleteFile(BatFile);

        TStringList *TestList=new TStringList;
        for(std::size_t i=0; i<job.batLines.size(); i++)
            TestList->Add(AnsiString(job.batLines[i].c_str()));
        TestList->SaveToFile(BatFile);
        delete TestList;

        // ExecZipCommand（cpublic.cpp:810-841）的 CreateProcess 參數原樣照抄（命令列 = Path+" "+Param、SW_HIDE、
        // 工作目錄 = ExtractFilePath(Path)），差別只有一個：**等它結束**。golden 不等 —— 它靠 StateRecordImage
        // 第 1～5 格約 10 秒的抓圖時間當作隱含等待，第 6 格才壓縮；移植樹沒有抓圖，不等就會壓到複製一半的資料夾，
        // 更糟的是壓完的 Del_Tree 會跟還在複製的 robocopy 搶同一個資料夾。上限 10 分鐘（robocopy 都帶 /R:0 /W:0）。
        STARTUPINFO  FStartupInfo;
        PROCESS_INFORMATION  FProcessInformation;
        ZeroMemory(&FStartupInfo, sizeof(STARTUPINFO));
        ZeroMemory(&FProcessInformation, sizeof(PROCESS_INFORMATION));
        GetStartupInfo(&FStartupInfo);
        FStartupInfo.dwFlags=STARTF_USESHOWWINDOW;
        FStartupInfo.wShowWindow=SW_HIDE;
        AnsiString ExecFile=BatFile+" "+" ";                                    // golden ExecZipCommand(BatFile, " ") ⇒ Path+" "+Param
        AnsiString ExecPath=ExtractFilePath(BatFile);
        if(CreateProcess(NULL, const_cast<char*>(ExecFile.c_str()), NULL, NULL, false,
                         NORMAL_PRIORITY_CLASS, NULL, ExecPath.c_str(), &FStartupInfo, &FProcessInformation))
        {
            const DWORD w=::WaitForSingleObject(FProcessInformation.hProcess, 10u*60u*1000u);
            if(w!=WAIT_OBJECT_0)
                RecordProcess("State Record worker: copy batch still running after 600 s, compressing anyway", BatFile);
            ::CloseHandle(FProcessInformation.hThread);
            ::CloseHandle(FProcessInformation.hProcess);
        }
        else
        {
            RecordProcess("State Record worker: CreateProcess failed for copy batch", BatFile);
        }
    }
    catch(...)
    {
    };

    // StateRecordImage 第 6 格的後半（golden :26194-26205），MOVED-BG
    AnsiString NewPath=job.newPath.c_str();
    AnsiString str1="";
    int iret=-1;
    bool bZipped=false;
    if(FileExists("d:\\HT9045\\7z.exe"))                                        //KenHsieh 20230105 : 新增StateRecord 另存路徑
    {
        str1.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s.zip\" \"%s\"", NewPath, NewPath);
        iret=system(str1.c_str());                                              //壓縮StateRecord
        ::Sleep(500);                                                           // golden MySleep(500)；背景執行緒不走 MySleep（它是給 UI 執行緒抽訊息用的）
        if(iret==0)                                                             //壓縮成功
        {
            Del_Tree(NewPath);                                                  //刪除目錄
            bZipped=true;
        }
    }

    if(job.bManual)                                                             //KenHsieh 20230105 : 新增StateRecord 另存路徑
        ShellExecute(NULL, "open", job.sDataPath.c_str(), NULL, NULL, SW_SHOW);     //顯示目錄

    RecordProcess(bZipped ? "State Record worker done (zipped)" : "State Record worker done (NOT zipped: 7z missing or failed; folder kept)",
                  bZipped ? NewPath+".zip" : NewPath);
    W906_StateRecordWorkerBusy.store(false);
}

}  // namespace

// =============================================================================
//  golden Timer2Timer 的 State Record 段（main.cpp:21142-21153），逐字。
//  wb_serve 的 tick 迴圈每拍呼叫（tools/wb_serve.cpp，PumpTick 那一行之後的空行）；
//  golden 的 Timer2 是 1000 ms（main.dfm:17279 沒寫 Interval ⇒ VCL 預設），tick 是 500 ms，
//  所以用 GetTickCount 限成每秒一次，讓格 6（延後警報）落在 golden 的 ~12 秒上。
// =============================================================================
void W906_StateRecordTimer2Pump()
{
    static DWORD s_last=0;
    const DWORD now=::GetTickCount();
    if(s_last!=0 && (DWORD)(now-s_last)<1000u) return;
    s_last=now;
    if(fMain==0) return;

    if(fMain->iSaveImgae!=-1)
    {
        fMain->iSaveImageCT++;
        if(fMain->iSaveImageCT>1)
        {
            fMain->StateRecordImage();
        }
    }
    else
    {
        fMain->iSaveImageCT=0;
    }
}

// =============================================================================
//  安裝座（forms/fMain.h 檔尾）。wb_serve 開機時明確呼叫一次 —— 理由同 InstallClarnDataBody：
//  靜態 archive 裡的自我登錄 TU 不會被連進去（forms/fMain.h:1265-1271）。
// =============================================================================
static void StateRecordBodyTrampoline(TfMain *self, int iShowAlarm, bool bManual)
{
    self->W906_DoStateRecordBody(iShowAlarm, bManual);
}

//AI(W906-FLOWDIAG) 20260927: 動作流程對照（INBOX 第 49 列、RULINGS_20260927 第 3／5 條）專用 —— 不是 golden 的功能。
//  模擬組態下 golden DoStateRecord 整段不錄（上面 #ifdef SOFT_SIMULTE，照翻），比對工具就拿不到 Task_ListWithTime.csv。
//  這支只做錄製同步段裡的那兩行（本檔 DoStateRecord 的 SaveTaskList(NewPath)／SaveDecisionVariables(NewPath)，
//  同一個 tick 執行緒），寫到 D:\HT9045_StateRecord\<yyyy-mm-dd hh_nn_ss>_tasklist\（路徑在這裡決定，網頁給不了）。
//  不動 NewPath／sFileNameTime，不做 SaveMachineRecord、截圖、複製、7z。act.main.stateRecord 帶 {"taskListOnly":true} 才走這裡。
static AnsiString TaskListOnlyTrampoline(TfMain *self)
{
    AnsiString dir="D:\\HT9045_StateRecord\\"+FormatDateTime("yyyy-mm-dd hh_nn_ss", Now())+"_tasklist";
    MyForceDirectories(dir);
    self->SaveTaskList(dir);
    self->SaveDecisionVariables(dir);
    return dir;
}

void W906_InstallStateRecordBody()
{
    W906_StateRecordBody=&StateRecordBodyTrampoline;
    W906_TaskListOnlyBody=&TaskListOnlyTrampoline;                              //AI(W906-FLOWDIAG) 20260927
}

// =============================================================================
//  AI(W906-TASKLIST) 20260927: golden TfMain::FormShow main.cpp:9717-10036、:10049-10053 —— task 紀錄環的登錄
//  （QueueTaskList[1..281].SetAliasAndTask）。移植樹以前沒翻這一段：每一輪記錄的迴圈（csystem.cpp:30700，golden
//  csystem.cpp:17132-17136）有翻，但沒有任何一列登錄過 ⇒ State Record 的 Task_ListWithTime.csv 每列都是空的
//  （0927 動作流程對照第一次模擬實跑量到：152,307 bytes 全是逗號）。動作流程對照（INBOX 第 49 列、RULINGS_20260927 第 5 條）
//  整件事就靠這個檔。逐行照 golden（cp950 轉 UTF-8）；和 golden 不同的地方都標了：
//    ① 自由函式：golden 在 FormShow 裡直接寫的 StringGrid2 改成 fMain->StringGrid2；
//    ② sgTaskList 移植樹沒有（GATE W906-STATEREC-G3，同上面 DoStateRecord 那一個）⇒ 那幾行閘著；
//    ③ &fContact->X 改成 &fContactForm->X：移植樹的全域 fContact 是替身 TfContactShim（atester_shims.h:251），
//       真正在推這些 task 的 TfContact 物件是 fContactForm（forms/fContact.h:1634、forms/fContact.cpp:90；
//       例 CarlibrationTask=1 在 forms/fContact.cpp:742）。golden 只有一個 fContact，就是那個表單；
//    ④ 取表單成員位址的那幾列前面加 if(表單)：golden 的表單在 FormShow 時一定在，移植樹不一定；
//    ⑤ 編譯器量到移植樹沒有的 task 變數（6 列）⇒ 那一列閘著、不補假變數，理由寫在各自的 GATE。
//  golden :10038-10047（KYEC 系列顯示 pnlRear／pnlFront）是 FormShow 的畫面設定、不是 task 紀錄，不在這裡。
//  wb_serve 開機時呼叫（tools/wb_serve.cpp，跟 W906_InstallStateRecordBody 同一行）；測試執行檔不呼叫 ⇒ 行為不變。
//  golden main.cpp 什麼標頭都有；下面是這一段用到、本檔原本沒 include 的（放在這裡不移動本檔前面的行號）。
// =============================================================================
#include "asendic_Auto.h"            // BinTrayTask[]
#include "asendic_Auto2.h"           // iLoadNewAuto2TrayToCarTask、iAuto2TrayToFrontTask…iAuto2ReceiveTask
#include "asendic_Auto_RT.h"         // iLoadNewAutoTrayToCarTask[]、iAutoTrayToFrontTask[]…iTrayZAutoTrayToWaitTask[]
#include "asendic_Color.h"           // iLoadNewColorTrayToCarTask、iAutoColorTask
#include "AutoClean/AutoClean.h"     // iAutoCleanPickFromCleanKitStageTask、iAutoCleanPlaceToShuttleTask
#include "AutoRetest.h"              // iAutoRetestTask、iAuto_AutoRT_Task[]、iTrayArm_AutoRT_Task、iLoader_AutoRT_Task
#include "atester_shims.h"           // iTestYFrontTask、iTestYRearTask、iFTestSuckTestICTask、iBTestSuckTestICTask
#include "aTester_Front.h"           // iFrontTestDestroyICTask、iFrontTestSuckICTask
#include "aTester_Rear.h"            // iRearTestDestroyICTask、iRearTestSuckICTask
#include "atester_32Site.h"          // iTestTwoArm32SiteTask
#include "SortingBinTray/SortingBinTray.h"   // iTask_DoFix3FullTray
#include "forms/fContact.h"          // TfContact、fContactForm（③）
#include "forms/fHome.h"             // fHome->iHomeStep、fHome->TestZTask
#include "forms/fLotInfo.h"          // fLotInfo->iWaitRtcDeleteTask
#include "forms/fNote.h"             // fNote->iIndexMoveToFrontRearTask
#include "forms/fOCR.h"              // fOCR->iOcrWithTesterTask
void W906_BootRegisterTaskList()
{
    // AI(W906-FLOW-1) 20260927: golden TfMain::FormShow main.cpp:9159-9160 calls initLifterTask()/initAutoTask() (asendic.cpp:172-190)
    //   before the task registration at :9717; the port had no caller outside tests, so every lifter/auto state machine sat at 0
    //   (no case) -- e.g. the Color tray load (reference FT005054 iLifterTask[0][2] 1,1000,1100,1) could never start.
    { extern void initLifterTask(); extern void initAutoTask(); initLifterTask(); initAutoTask(); }
    // AI(W906-FLOW-1) 20260927: golden main.cpp:9136-9137 declares `extern int iLifterTask[3][7]; extern int iAutoTask[3][7];`
    //   while the definitions are [3][10] / [3][MAX_UNLOAD_TRAY] (asendic.cpp:162/:169), so golden's row "iLifterTask[r][c]" records
    //   storage slot r*7+c (e.g. "[1][5]" is definition [1][2]). Only these recorder rows reproduce that address so Task_ListWithTime
    //   lines up with golden and the real-machine reference; the engines and tests keep the definition strides (w3_cylinder_plant.h:163-167).
#define W906_GOLDEN_STRIDE7(a, r, c) (reinterpret_cast<int*>(a) + (r) * 7 + (c))
    extern int iKnockShuttleTask;                                               // ainarm2.cpp:104（沒有標頭；型別照定義）
    extern int iKnockShtFirstTask;                                              // ainarm2.cpp:105
    extern int iShakeShuttleTask;                                               // ainarm2.cpp:6572
    extern int iLifterTask[3][10];                                              // asendic.cpp:162
    extern int iAutoTask[3][MAX_UNLOAD_TRAY];                                   // asendic.cpp:169
    // 下面這些 task 變數在移植樹有定義、但本檔看得到的標頭沒有宣告（或那個標頭跟本檔衝突）⇒ 在這裡 extern，型別照定義那一行
    extern int iAutoChkInSHLatchTask;                                           // ainarm9045.cpp:1930
    extern int iAutoLoaderReceiveTask;                                          // asendic_Loader_RT.cpp:84
    extern int iAutoLoaderTask;                                                 // asendic_Loader_RT.cpp:524
    extern int iBGAViewInspectionTask;                                          // fAOI.cpp:100
    extern int iBGAViewTask;                                                    // fAOI.cpp:99
    extern int iCheckOCRInArmSuckTask;                                          // OCRInsp.cpp:749
    extern int iInArmAdditionalFunctionTask;                                    // ainarm9045.cpp:1171
    extern int iInArmDevicePosPrecise;                                          // ainarm2.cpp:101
    extern int iInArmInArmCheckShtFloatTask;                                    // ainarm9045.cpp:1495
    extern int iInArmLaserCheckTask;                                            // LaserSensorInArm.cpp:97
    extern int iInArmLaserInitTask;                                             // LaserSensorInArm.cpp:96
    extern int iInArmRotateKit;                                                 // aRotateKIT_In.cpp:406
    extern int iInArmZCheckPosTask;                                             // ainarm2.cpp:100
    extern int iLoadNewLoaderTrayToCarTask;                                     // asendic_Loader_RT.cpp:85
    extern int iLoaderTrayToFrontTask;                                          // asendic_Loader_RT.cpp:223
    extern int iLoaderTrayToRearTask;                                           // asendic_Loader_RT.cpp:391
    extern int iOCRFlow;                                                        // OCRInsp.cpp:743
    extern int iOutArmRotateKit;                                                // aRotateKIT_Out.cpp:422
    extern int iPADViewInspectionTask;                                          // fAOI.cpp:98
    extern int iPADViewTask;                                                    // fAOI.cpp:97
    extern int iProcessSCKARTLoadingCountTask;                                  // ainarm9045.cpp:129
    extern int iProcessTrayMapDataErrorTask;                                    // ainarm2.cpp:106
    extern int iScanAOITask;                                                    // fAOI.cpp:101
    extern int iScannerAOIInspectionTask;                                       // fAOI.cpp:102
    extern int iShakeInArmRotateKit;                                            // aRotateKIT_In.cpp:416
    extern int iShakeOutArmRotateKit;                                           // aRotateKIT_Out.cpp:431
    extern int iShtLaserCheckTask;                                              // LaserSensorShuttle.cpp:234
    extern int iShtLaserInitTask;                                               // LaserSensorShuttle.cpp:233
    extern int iTopViewInspectionTask;                                          // fAOI.cpp:96
    extern int iTopViewTask;                                                    // fAOI.cpp:95
    extern int iUnLoadNewLoaderTrayTask;                                        // asendic_Loader_RT.cpp:734
    extern int iInitialBarcodeInShuttle1Task;                                   // BarCode_Shuttle1_Scan.cpp:75（golden BarCode.h:786）
    extern int iInitialBarcodeInShuttle2Task;                                   // BarCode_Shuttle2_Scan.cpp:28（golden BarCode.h:787）
    extern int iInitialBarcodeOutShuttle1Task;                                  // BarCode_Shuttle1_Scan.cpp:76（golden BarCode.h:788）
    extern int iInitialBarcodeOutShuttle2Task;                                  // BarCode_Shuttle2_Scan.cpp:29（golden BarCode.h:789）
    extern int iSFCAutoTune1Task;                                               // BarCode_Shuttle1_SFCAutoTune.cpp:355（golden BarCode.h:884）
    extern int iSFCAutoTune2Task;                                               // BarCode_Shuttle2_SFCAutoTune.cpp:152（golden BarCode.h:885）
    extern int iShuttleFloatCheck1Task;                                         // BarCode_Shuttle1_Scan.cpp:77（golden BarCode.h:882）
    extern int iShuttleFloatCheck2Task;                                         // BarCode_Shuttle2_Scan.cpp:30（golden BarCode.h:883）
    //jou 2011-11-22 start : 修正正確Task名稱
    //Steven 20180808 (wei) : 修改紀錄Task的方式
    //==>
    QueueTaskList[1].SetAliasAndTask("AutoSHT1Task",                            &AutoSHT1Task);
    QueueTaskList[2].SetAliasAndTask("AutoSHT2Task",                            &AutoSHT2Task);
    QueueTaskList[3].SetAliasAndTask("InArmTask",                               &iArmTask);
    QueueTaskList[4].SetAliasAndTask("InArmPlaceToShuttleTask",                 &iInArmPlaceToShuttleTask);
    QueueTaskList[5].SetAliasAndTask("TestTask",                                &iTestTask);
    QueueTaskList[6].SetAliasAndTask("TestHeadMotorTask",                       &iTestHeadMotorTask);
    QueueTaskList[7].SetAliasAndTask("OutArmTask",                              &OutArmTask);
    QueueTaskList[8].SetAliasAndTask("BinTrayTask[0]",                          &BinTrayTask[0]);
    QueueTaskList[9].SetAliasAndTask("BinTrayTask[1]",                          &BinTrayTask[1]);

    QueueTaskList[10].SetAliasAndTask("BinTrayTask[2]",                         &BinTrayTask[2]);
    QueueTaskList[11].SetAliasAndTask("CatchTrayTask",                          &CatchTrayTask);
    QueueTaskList[12].SetAliasAndTask("TestYTask",                              &iTestYTask);
    QueueTaskList[13].SetAliasAndTask("InArmPickFromLoadTask",                  &iPickFromLoadStageTask);
    QueueTaskList[14].SetAliasAndTask("LoadTask",                               &LoadTask);
    QueueTaskList[15].SetAliasAndTask("PickFromShuttle1Task",                   &iPickFromShuttle1Task);
    QueueTaskList[16].SetAliasAndTask("PickFromShuttle2Task",                   &iPickFromShuttle2Task);
    QueueTaskList[17].SetAliasAndTask("PlaceToAutoTask",                        &iPlaceToAutoTask);
    QueueTaskList[18].SetAliasAndTask("PlaceToFixTask",                         &iPlaceToFixTask);
    QueueTaskList[19].SetAliasAndTask("TestYFrontTask",                         &iTestYFrontTask);

    QueueTaskList[20].SetAliasAndTask("RearTestSuckTestICTask",                 &iBTestSuckTestICTask);
    QueueTaskList[21].SetAliasAndTask("FrontTestDestroyICTask",                 &iFrontTestDestroyICTask);
    QueueTaskList[22].SetAliasAndTask("FrontTestSuckICTask",                    &iFrontTestSuckICTask);
    QueueTaskList[23].SetAliasAndTask("TestYRearTask",                          &iTestYRearTask);
    QueueTaskList[24].SetAliasAndTask("FrontTestSuckTestICTask",                &iFTestSuckTestICTask);
    QueueTaskList[25].SetAliasAndTask("RearTestDestroyICTask",                  &iRearTestDestroyICTask);
    QueueTaskList[26].SetAliasAndTask("RearTestSuckICTask",                     &iRearTestSuckICTask);
    QueueTaskList[27].SetAliasAndTask("TrayArmPlaceTrayToAutoTask",             &iPlaceTrayToAutoTask);
    QueueTaskList[28].SetAliasAndTask("InArmPickFromHotPlateTask",              &iInArmPickFromHotPlateTask);
    QueueTaskList[29].SetAliasAndTask("InArmPlaceToHotPlateTask",               &iInArmPlaceToHotPlateTask);

    QueueTaskList[30].SetAliasAndTask("InArmTryPickFromHotPlateTask",           &iInArmTryPickFromHotPlateTask);
    QueueTaskList[31].SetAliasAndTask("TrayArmCatchNewTrayFromBufferTask",      &iCatchNewTrayFromBufferTask);
    QueueTaskList[32].SetAliasAndTask("TrayArmCatchFromLoaderTask",             &iCatchFromLoaderTask);
    QueueTaskList[33].SetAliasAndTask("BarcodeInShuttle1Task",                  &iInitialBarcodeInShuttle1Task);                                      //Steven 20200422 : 新增Barcode Task紀錄   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle1_Scan.cpp:75（golden BarCode.h:786）
    QueueTaskList[34].SetAliasAndTask("BarcodeInShuttle2Task",                  &iInitialBarcodeInShuttle2Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle2_Scan.cpp:28（golden BarCode.h:787）
#if 0 // GATE(W906-TASKLIST) golden fBarCode->iBottom2DIDTask 在移植樹是 BarCode_Bottom2DID.cpp:84 匿名 namespace 裡的 TU 內變數，別的檔取不到位址（要登錄得先在那個檔開一個存取點）；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[35].SetAliasAndTask("Bottom2DIDTask",                         &fBarCode->iBottom2DIDTask);
#endif
    QueueTaskList[36].SetAliasAndTask("AutoCleanTask",                          &iDoAutoCleanTask);
    QueueTaskList[37].SetAliasAndTask("AutoCleanShuttle1Task",                  &iDoShuttle1AutoCleanTask);
    QueueTaskList[38].SetAliasAndTask("AutoCleanShuttle2Task",                  &iDoShuttle2AutoCleanTask);
    QueueTaskList[39].SetAliasAndTask("AutoCleanIndexTask",                     &iDoIndexAutoCleanTask);

    QueueTaskList[40].SetAliasAndTask("AutoCleanPickFromCleanKitStageTask",     &iAutoCleanPickFromCleanKitStageTask);
    QueueTaskList[41].SetAliasAndTask("AutoCleanPlaceToShuttleTask",            &iAutoCleanPlaceToShuttleTask);
    QueueTaskList[42].SetAliasAndTask("AutoRetestTask",                         &iAutoRetestTask);
    QueueTaskList[43].SetAliasAndTask("Auto_AutoRT_Task[0]",                    &iAuto_AutoRT_Task[0]);
    QueueTaskList[44].SetAliasAndTask("Auto_AutoRT_Task[1]",                    &iAuto_AutoRT_Task[1]);
    QueueTaskList[45].SetAliasAndTask("Auto_AutoRT_Task[2]",                    &iAuto_AutoRT_Task[2]);
    QueueTaskList[46].SetAliasAndTask("TrayArm_AutoRT_Task",                    &iTrayArm_AutoRT_Task);
    QueueTaskList[47].SetAliasAndTask("Loader_AutoRT_Task",                     &iLoader_AutoRT_Task);
    QueueTaskList[48].SetAliasAndTask("ReceiveAutoTrayTask[0]",                 &iReceiveAutoTrayTask[0]);
    QueueTaskList[49].SetAliasAndTask("ReceiveAutoTrayTask[1]",                 &iReceiveAutoTrayTask[1]);

    QueueTaskList[50].SetAliasAndTask("ReceiveAutoTrayTask[2]",                 &iReceiveAutoTrayTask[2]);
    QueueTaskList[51].SetAliasAndTask("TestTwoArm32SiteTask",                   &iTestTwoArm32SiteTask);                //jou 2015-05-04 add task iTestTwoArm32SiteTask
    QueueTaskList[52].SetAliasAndTask("AutoCleanPickFromShuttleTask",           &iAutoCleanPickFromShuttleTask);        //ChungHung 20150513 add
    QueueTaskList[53].SetAliasAndTask("ShakeShuttleTask",                       &iShakeShuttleTask);                    //Steven 20160504 : add task iShakeShuttleTask
    QueueTaskList[54].SetAliasAndTask("KnockShuttleTask",                       &iKnockShuttleTask);                    //Steven 20160504 : add task iKnockShuttleTask
    QueueTaskList[55].SetAliasAndTask("KnockShtFirstTask",                      &iKnockShtFirstTask);                   //Steven 20160504 : add task iKnockShtFirstTask
    QueueTaskList[56].SetAliasAndTask("LoadNewEmptyTrayToCarTask",              &iLoadNewEmptyTrayToCarTask);           //JerryYang 20161128 add DoEmptyTrayToCar Task記錄
    QueueTaskList[57].SetAliasAndTask("LoadNewColorTrayToCarTask",              &iLoadNewColorTrayToCarTask);           //JerryYang 20161128 add DoEmptyTrayToCar Task記錄
    QueueTaskList[58].SetAliasAndTask("AutoEmptyTask",                          &iAutoEmptyTask);
    QueueTaskList[59].SetAliasAndTask("AutoColorTask",                          &iAutoColorTask);

    QueueTaskList[60].SetAliasAndTask("AutoCleanPlaceToCleanKitTask",           &iAutoCleanPlaceToCleanKitTask);        //JerryYang 20161219 (Steven) add iAutoCleanPlaceToCleanKitTask
    QueueTaskList[61].SetAliasAndTask("LoadNewICTrayTask",                      &iLoadNewICTrayTask);                   //Steven 20170703 (Steven) : 補上Loader的Task
    QueueTaskList[62].SetAliasAndTask("DoFix3FullTray",                         &iTask_DoFix3FullTray);                 //Ifor 20180214 (Steven) : add 補上 Fix3 Full Tray 的Task
    QueueTaskList[63].SetAliasAndTask("DoPickFix3IC",                           &iTask_DoPickFix3IC);                   //Ifor 20180214 (Steven) : add 補上 Fix3 Full Tray 的Task
    QueueTaskList[64].SetAliasAndTask("DoPlaceFix3IC",                          &iTask_DoPlaceFix3IC);                  //Ifor 20180214 (Steven) : add 補上 Fix3 Full Tray 的Task
    QueueTaskList[65].SetAliasAndTask("Fix3CanFullTask",                        &iFix3CanFullTask);
    QueueTaskList[66].SetAliasAndTask("OneCycleTask",                           &iOneCycleTask);                        //JerryYang 20190925 one cycle task log
    QueueTaskList[67].SetAliasAndTask("CleanOutCycleTask",                      &iCleanOutCycleTask);                   //JerryYang 20190925 clean out task log
    QueueTaskList[68].SetAliasAndTask("TrayArmPlaceToBufferTask",               &iPlaceToBufferTask);                   //JerryYang 20200106 add log
    QueueTaskList[69].SetAliasAndTask("TrayFeedTask",                           &iTrayFeedTask);

    QueueTaskList[70].SetAliasAndTask("iLifterTask[0][0]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 0));
    QueueTaskList[71].SetAliasAndTask("iLifterTask[0][1]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 1));
    QueueTaskList[72].SetAliasAndTask("iLifterTask[0][2]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 2));
    QueueTaskList[73].SetAliasAndTask("iLifterTask[0][3]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 3));
    QueueTaskList[74].SetAliasAndTask("iLifterTask[0][4]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 4));
    QueueTaskList[75].SetAliasAndTask("iLifterTask[0][5]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 5));
    QueueTaskList[76].SetAliasAndTask("iLifterTask[0][6]",                      W906_GOLDEN_STRIDE7(iLifterTask, 0, 6));
    QueueTaskList[77].SetAliasAndTask("iLifterTask[1][0]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 0));
    QueueTaskList[78].SetAliasAndTask("iLifterTask[1][1]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 1));
    QueueTaskList[79].SetAliasAndTask("iLifterTask[1][2]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 2));

    QueueTaskList[80].SetAliasAndTask("iLifterTask[1][3]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 3));
    QueueTaskList[81].SetAliasAndTask("iLifterTask[1][4]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 4));
    QueueTaskList[82].SetAliasAndTask("iLifterTask[1][5]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 5));
    QueueTaskList[83].SetAliasAndTask("iLifterTask[1][6]",                      W906_GOLDEN_STRIDE7(iLifterTask, 1, 6));
    QueueTaskList[84].SetAliasAndTask("iLifterTask[2][0]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 0));
    QueueTaskList[85].SetAliasAndTask("iLifterTask[2][1]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 1));
    QueueTaskList[86].SetAliasAndTask("iLifterTask[2][2]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 2));
    QueueTaskList[87].SetAliasAndTask("iLifterTask[2][3]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 3));
    QueueTaskList[88].SetAliasAndTask("iLifterTask[2][4]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 4));
    QueueTaskList[89].SetAliasAndTask("iLifterTask[2][5]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 5));

    QueueTaskList[90].SetAliasAndTask("iLifterTask[2][6]",                      W906_GOLDEN_STRIDE7(iLifterTask, 2, 6));
    QueueTaskList[91].SetAliasAndTask("iAutoTask[0][0]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 0));
    QueueTaskList[92].SetAliasAndTask("iAutoTask[0][1]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 1));
    QueueTaskList[93].SetAliasAndTask("iAutoTask[0][2]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 2));
    QueueTaskList[94].SetAliasAndTask("iAutoTask[0][3]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 3));
    QueueTaskList[95].SetAliasAndTask("iAutoTask[0][4]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 4));
    QueueTaskList[96].SetAliasAndTask("iAutoTask[0][5]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 5));
    QueueTaskList[97].SetAliasAndTask("iAutoTask[0][6]",                        W906_GOLDEN_STRIDE7(iAutoTask, 0, 6));
    QueueTaskList[98].SetAliasAndTask("iAutoTask[1][0]",                        W906_GOLDEN_STRIDE7(iAutoTask, 1, 0));
    QueueTaskList[99].SetAliasAndTask("iAutoTask[1][1]",                        W906_GOLDEN_STRIDE7(iAutoTask, 1, 1));

    QueueTaskList[100].SetAliasAndTask("iAutoTask[1][2]",                       W906_GOLDEN_STRIDE7(iAutoTask, 1, 2));
    QueueTaskList[101].SetAliasAndTask("iAutoTask[1][3]",                       W906_GOLDEN_STRIDE7(iAutoTask, 1, 3));
    QueueTaskList[102].SetAliasAndTask("iAutoTask[1][4]",                       W906_GOLDEN_STRIDE7(iAutoTask, 1, 4));
    QueueTaskList[103].SetAliasAndTask("iAutoTask[1][5]",                       W906_GOLDEN_STRIDE7(iAutoTask, 1, 5));
    QueueTaskList[104].SetAliasAndTask("iAutoTask[1][6]",                       W906_GOLDEN_STRIDE7(iAutoTask, 1, 6));
    QueueTaskList[105].SetAliasAndTask("iAutoTask[2][0]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 0));
    QueueTaskList[106].SetAliasAndTask("iAutoTask[2][1]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 1));
    QueueTaskList[107].SetAliasAndTask("iAutoTask[2][2]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 2));
    QueueTaskList[108].SetAliasAndTask("iAutoTask[2][3]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 3));
    QueueTaskList[109].SetAliasAndTask("iAutoTask[2][4]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 4));

    QueueTaskList[110].SetAliasAndTask("iAutoTask[2][5]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 5));
    QueueTaskList[111].SetAliasAndTask("iAutoTask[2][6]",                       W906_GOLDEN_STRIDE7(iAutoTask, 2, 6));
#undef W906_GOLDEN_STRIDE7
#if 0 // GATE(W906-TASKLIST) dmTrayMotor（golden Motor/TrayStepMotor，TDataModule）移植樹沒有（forms/fSpeed.h:178-182）—— SetStepMotorTask 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[112].SetAliasAndTask("SetStepMotorTask",                      &dmTrayMotor->iSetStepMotorTask);
#endif
    QueueTaskList[113].SetAliasAndTask("IndexSocketCheckTask",                  &iDoIndexSocketCheckTask);
    QueueTaskList[114].SetAliasAndTask("CheckSocketHasIC",                      &iCheckSocketHasIC);
    QueueTaskList[115].SetAliasAndTask("InterFaceErrorStepTask",                &iDoInterFaceErrorStepTask);
#if 0 // GATE(W906-TASKLIST) iDoAllPassVerifyTask 只在 atester.cpp 的 golden 原文閘裡（GATE G-PTk4-DoAllPassVerifyRTC，:8867）—— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[116].SetAliasAndTask("AllPassVerifyTask",                     &iDoAllPassVerifyTask);
#endif
    QueueTaskList[117].SetAliasAndTask("SetupTask",                             &iSetupTask);
    QueueTaskList[118].SetAliasAndTask("TempICTask",                            &iTempICTask);
    QueueTaskList[119].SetAliasAndTask("IndexEveryTimeCheckEPTask",             &iIndexEveryTimeCheckEPTask);

    QueueTaskList[120].SetAliasAndTask("IndexStatus",                           &IndexStatus);
    QueueTaskList[121].SetAliasAndTask("IndexArm1PickUpErrNeedPiggybackTask",   &iIndexArm1PickUpErrNeedPiggybackTask);
    QueueTaskList[122].SetAliasAndTask("IndexArm2PickUpErrNeedPiggybackTask",   &iIndexArm2PickUpErrNeedPiggybackTask);
    QueueTaskList[123].SetAliasAndTask("SlapTrayTask",                          &iSlapTrayTask);
    if(fLotInfo) QueueTaskList[124].SetAliasAndTask("WaitRtcDeleteTask",                     &fLotInfo->iWaitRtcDeleteTask);
#if 0 // GATE(W906-TASKLIST) TfNote 移植樹沒有 iIndexMoveToFrontRearTask（btnMoveToFront/RearClick 與 IndexMoveToFrontRear 會動機台、沒翻，forms/fNote.h:84-88） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[125].SetAliasAndTask("IndexMoveToFrontRearTask",              &fNote->iIndexMoveToFrontRearTask);
#endif
    QueueTaskList[126].SetAliasAndTask("AutoTrackDetectICFloatingTask[0]",      &iAutoTrackDetectICFloatingTask[0]);
    QueueTaskList[127].SetAliasAndTask("AutoTrackDetectICFloatingTask[1]",      &iAutoTrackDetectICFloatingTask[1]);
    QueueTaskList[128].SetAliasAndTask("AutoTrackDetectICFloatingTask[2]",      &iAutoTrackDetectICFloatingTask[2]);
    QueueTaskList[129].SetAliasAndTask("Auto123TrayToRearTask[0]",              &iAuto123TrayToRearTask[0]);

    QueueTaskList[130].SetAliasAndTask("Auto123TrayToRearTask[1]",              &iAuto123TrayToRearTask[1]);
    QueueTaskList[131].SetAliasAndTask("Auto123TrayToRearTask[2]",              &iAuto123TrayToRearTask[2]);
    QueueTaskList[132].SetAliasAndTask("LoadNewAutoTrayToCarTask[0]",           &iLoadNewAutoTrayToCarTask[0]);
    QueueTaskList[133].SetAliasAndTask("LoadNewAutoTrayToCarTask[1]",           &iLoadNewAutoTrayToCarTask[1]);
    QueueTaskList[134].SetAliasAndTask("LoadNewAutoTrayToCarTask[2]",           &iLoadNewAutoTrayToCarTask[2]);
    QueueTaskList[135].SetAliasAndTask("AutoTrayToFrontTask[0]",                &iAutoTrayToFrontTask[0]);
    QueueTaskList[136].SetAliasAndTask("AutoTrayToFrontTask[1]",                &iAutoTrayToFrontTask[1]);
    QueueTaskList[137].SetAliasAndTask("AutoTrayToFrontTask[2]",                &iAutoTrayToFrontTask[2]);
    QueueTaskList[138].SetAliasAndTask("AutoTrayToRearTask[0]",                 &iAutoTrayToRearTask[0]);
    QueueTaskList[139].SetAliasAndTask("AutoTrayToRearTask[1]",                 &iAutoTrayToRearTask[1]);

    QueueTaskList[140].SetAliasAndTask("AutoTrayToRearTask[2]",                 &iAutoTrayToRearTask[2]);
//    QueueTaskList[141].SetAliasAndTask("iAutoTrayTask[0]",                      &iAutoTrayTask[0]);
//    QueueTaskList[142].SetAliasAndTask("iAutoTrayTask[1]",                      &iAutoTrayTask[1]);
//    QueueTaskList[143].SetAliasAndTask("iAutoTrayTask[2]",                      &iAutoTrayTask[2]);
    QueueTaskList[144].SetAliasAndTask("AutoTrayReceiveTask[0]",                &iAutoTrayReceiveTask[0]);
    QueueTaskList[145].SetAliasAndTask("AutoTrayReceiveTask[1]",                &iAutoTrayReceiveTask[1]);
    QueueTaskList[146].SetAliasAndTask("AutoTrayReceiveTask[2]",                &iAutoTrayReceiveTask[2]);
    QueueTaskList[147].SetAliasAndTask("UnLoadNewAutoTrayTask[0]",              &iUnLoadNewAutoTrayTask[0]);
    QueueTaskList[148].SetAliasAndTask("UnLoadNewAutoTrayTask[1]",              &iUnLoadNewAutoTrayTask[1]);
    QueueTaskList[149].SetAliasAndTask("UnLoadNewAutoTrayTask[2]",              &iUnLoadNewAutoTrayTask[2]);

    QueueTaskList[150].SetAliasAndTask("TrayZAutoTrayToWaitTask[0]",            &iTrayZAutoTrayToWaitTask[0]);
    QueueTaskList[151].SetAliasAndTask("TrayZAutoTrayToWaitTask[1]",            &iTrayZAutoTrayToWaitTask[1]);
    QueueTaskList[152].SetAliasAndTask("TrayZAutoTrayToWaitTask[2]",            &iTrayZAutoTrayToWaitTask[2]);
    QueueTaskList[153].SetAliasAndTask("LoadNewAuto2TrayToCarTask",             &iLoadNewAuto2TrayToCarTask);
    QueueTaskList[154].SetAliasAndTask("Auto2TrayToFrontTask",                  &iAuto2TrayToFrontTask);
    QueueTaskList[155].SetAliasAndTask("Auto2TrayToRearTask",                   &iAuto2TrayToRearTask);
    QueueTaskList[156].SetAliasAndTask("AutoAuto2Task",                         &iAutoAuto2Task);
    QueueTaskList[157].SetAliasAndTask("UnLoadNewAuto2TrayTask",                &iUnLoadNewAuto2TrayTask);
    QueueTaskList[158].SetAliasAndTask("Auto2ReceiveTask",                      &iAuto2ReceiveTask);
    QueueTaskList[159].SetAliasAndTask("LoadNewColorTrayToCarTask",             &iLoadNewColorTrayToCarTask);

    QueueTaskList[160].SetAliasAndTask("ColorTrayToRearTask",                   &iColorTrayToRearTask);
    QueueTaskList[161].SetAliasAndTask("UnLoadNewColorTrayTask",                &iUnLoadNewColorTrayTask);
    QueueTaskList[162].SetAliasAndTask("AutoColorReceiveTask",                  &iAutoColorReceiveTask);
    QueueTaskList[163].SetAliasAndTask("EmptyTrayToFrontTask",                  &iEmptyTrayToFrontTask);
    QueueTaskList[164].SetAliasAndTask("EmptyTrayToRearTask",                   &iEmptyTrayToRearTask);
    QueueTaskList[165].SetAliasAndTask("UnLoadNewEmptyTrayTask",                &iUnLoadNewEmptyTrayTask);
    QueueTaskList[166].SetAliasAndTask("AutoEmptyReceiveTask",                  &iAutoEmptyReceiveTask);
    QueueTaskList[167].SetAliasAndTask("ColorTrayToFrontTask",                  &iColorTrayToFrontTask);
    QueueTaskList[168].SetAliasAndTask("AutoEmpty1ReceiveTask",                 &iAutoEmpty1ReceiveTask);
    QueueTaskList[169].SetAliasAndTask("SupplyNewIC_From_LoaderCar",            &iSupplyNewIC_From_LoaderCar);

    QueueTaskList[170].SetAliasAndTask("ReTestStartTask",                       &iReTestStartTask);
    QueueTaskList[171].SetAliasAndTask("InitialICCheckTask",                    &iInitialICCheckTask);
    QueueTaskList[172].SetAliasAndTask("StepShuttleTask[0]",                    &iStepShuttleTask[0]);
    QueueTaskList[173].SetAliasAndTask("StepShuttleTask[1]",                    &iStepShuttleTask[1]);
    QueueTaskList[174].SetAliasAndTask("ClearSocketFunctionTask",               &iClearSocketFunctionTask);
    QueueTaskList[175].SetAliasAndTask("CheckShuttle1MustHasICTask",            &iCheckShuttle1MustHasICTask);
    QueueTaskList[176].SetAliasAndTask("CheckShuttle2MustHasICTask",            &iCheckShuttle2MustHasICTask);
    QueueTaskList[177].SetAliasAndTask("CheckShuttle1ProminentTasK",            &iCheckShuttle1ProminentTasK);
    QueueTaskList[178].SetAliasAndTask("CheckShuttle2ProminentTasK",            &iCheckShuttle2ProminentTasK);
    QueueTaskList[179].SetAliasAndTask("CheckShuttle1ProminentNoHasICTasK",     &iCheckShuttle1ProminentNoHasICTasK);

    QueueTaskList[180].SetAliasAndTask("CheckShuttle2ProminentNoHasICTasK",     &iCheckShuttle2ProminentNoHasICTasK);
    QueueTaskList[181].SetAliasAndTask("DoStartMode",                           &iDoStartMode);
    QueueTaskList[182].SetAliasAndTask("DoEndMode",                             &iDoEndMode);
    QueueTaskList[183].SetAliasAndTask("IndexYAxisServoOnStateTask",            &iIndexYAxisServoOnStateTask);
    QueueTaskList[184].SetAliasAndTask("32RTCAutoModelVerifyTask",              &i32RTCAutoModelVerifyTask);
    QueueTaskList[185].SetAliasAndTask("TestSuckTestIC_TwoArm32Site_Task",      &iTestSuckTestIC_TwoArm32Site_Task);
    QueueTaskList[186].SetAliasAndTask("FRTCUseSocketFloatTask",                &iFRTCUseSocketFloatTask);
    QueueTaskList[187].SetAliasAndTask("FRTCAutoModelVerifyTask",               &iFRTCAutoModelVerifyTask);
    QueueTaskList[188].SetAliasAndTask("FrontTestPurgBeforePickShuttle",        &iFrontTestPurgBeforePickShuttle);
    QueueTaskList[189].SetAliasAndTask("FTestSocketClampCloseTask",             &iFTestSocketClampCloseTask);

    QueueTaskList[190].SetAliasAndTask("FTestSocketClampOpenTask",              &iFTestSocketClampOpenTask);
    QueueTaskList[191].SetAliasAndTask("BRTCUseSocketFloatTask",                &iBRTCUseSocketFloatTask);
    QueueTaskList[192].SetAliasAndTask("BRTCGiveWayCheckTask",                  &iBRTCGiveWayCheckTask);
    QueueTaskList[193].SetAliasAndTask("BRTCAutoModelVerifyTask",               &iBRTCAutoModelVerifyTask);
    QueueTaskList[194].SetAliasAndTask("RearTestPurgBeforePickShuttle",         &iRearTestPurgBeforePickShuttle);
    QueueTaskList[195].SetAliasAndTask("BTestSocketClampCloseTask",             &iBTestSocketClampCloseTask);
    QueueTaskList[196].SetAliasAndTask("BTestSocketClampOpenTask",              &iBTestSocketClampOpenTask);
//    QueueTaskList[197].SetAliasAndTask("PrePushLoaderCylinderTask",             &PrePushLoaderCylinderTask);
#if 0 // GATE(W906-TASKLIST) TfHome 移植樹刻意沒宣告 TestZTask（forms/fHome.h:30／:39） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[198].SetAliasAndTask("TestZTask",                             &fHome->TestZTask);
#endif
#if 0 // GATE(W906-TASKLIST) iAuto_TeachPitch（golden uteach.cpp:43 全域）移植樹沒有（forms/fTeachPara.h:206）—— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[199].SetAliasAndTask("Auto_TeachPitch",                       &iAuto_TeachPitch);
#endif

    if(fContactForm) QueueTaskList[200].SetAliasAndTask("CarlibrationTask",                      &fContactForm->CarlibrationTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[201].SetAliasAndTask("DoFullViewCheck",                       &fContactForm->iDoFullViewCheck);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[202].SetAliasAndTask("Z_Height_Task",                         &fContactForm->Z_Height_Task);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[203].SetAliasAndTask("DeviceMapCheckTask",                    &fContactForm->iDeviceMapCheckTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    QueueTaskList[204].SetAliasAndTask("ProcessSCKARTLoadingCountTask",         &iProcessSCKARTLoadingCountTask);
    QueueTaskList[205].SetAliasAndTask("ProcessTrayMapDataErrorTask",           &iProcessTrayMapDataErrorTask);
    QueueTaskList[206].SetAliasAndTask("InArmZCheckPosTask",                    &iInArmZCheckPosTask);
    QueueTaskList[207].SetAliasAndTask("InArmDevicePosPrecise",                 &iInArmDevicePosPrecise);
    QueueTaskList[208].SetAliasAndTask("InArmRotateKit",                        &iInArmRotateKit);
    QueueTaskList[209].SetAliasAndTask("OutArmRotateKit",                       &iOutArmRotateKit);

    QueueTaskList[210].SetAliasAndTask("ShtLaserInitTask",                      &iShtLaserInitTask);
    QueueTaskList[211].SetAliasAndTask("ShtLaserCheckTask",                     &iShtLaserCheckTask);
    QueueTaskList[212].SetAliasAndTask("InArmLaserInitTask",                    &iInArmLaserInitTask);
    QueueTaskList[213].SetAliasAndTask("InArmLaserCheckTask",                   &iInArmLaserCheckTask);
    QueueTaskList[214].SetAliasAndTask("TrayArm_PickFromAuto_AutoRT_Task",      &iTrayArm_PickFromAuto_AutoRT_Task);
    QueueTaskList[215].SetAliasAndTask("TrayArm_PlaceToLoad_AutoRT_Task",       &iTrayArm_PlaceToLoad_AutoRT_Task);
    QueueTaskList[216].SetAliasAndTask("TopViewInspectionTask",                 &iTopViewInspectionTask);
    QueueTaskList[217].SetAliasAndTask("TopViewTask",                           &iTopViewTask);
    QueueTaskList[218].SetAliasAndTask("PADViewInspectionTask",                 &iPADViewInspectionTask);
    QueueTaskList[219].SetAliasAndTask("PADViewTask",                           &iPADViewTask);

    QueueTaskList[220].SetAliasAndTask("BGAViewInspectionTask",                 &iBGAViewInspectionTask);
    QueueTaskList[221].SetAliasAndTask("BGAViewTask",                           &iBGAViewTask);
    QueueTaskList[222].SetAliasAndTask("ScannerAOIInspectionTask",              &iScannerAOIInspectionTask);
    QueueTaskList[223].SetAliasAndTask("ScanAOITask",                           &iScanAOITask);
#if 0 // GATE(W906-TASKLIST) iAOITask 移植樹沒有（git grep 找不到 namespace 範圍、#if 0 之外的定義） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[224].SetAliasAndTask("AOITask",                               &iAOITask);
#endif
#if 0 // GATE(W906-TASKLIST) TfOCR 移植樹沒有成員 iOcrWithTesterTask（編譯器量的） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[225].SetAliasAndTask("OcrWithTesterTask",                     &fOCR->iOcrWithTesterTask);
#endif
    QueueTaskList[226].SetAliasAndTask("CheckOCRInArmSuckTask",                 &iCheckOCRInArmSuckTask);
    QueueTaskList[227].SetAliasAndTask("OCRFlow",                               &iOCRFlow);
    QueueTaskList[228].SetAliasAndTask("Task_DoSortingBinTray",                 &iTask_DoSortingBinTray);
    QueueTaskList[229].SetAliasAndTask("Task_ToTrayPickIC",                     &iTask_ToTrayPickIC);

    QueueTaskList[230].SetAliasAndTask("Task_ToTrayPlaceIC",                    &iTask_ToTrayPlaceIC);
    QueueTaskList[231].SetAliasAndTask("Task_DoFix3FullTray",                   &iTask_DoFix3FullTray);
    QueueTaskList[232].SetAliasAndTask("Task_DoPickFix3IC",                     &iTask_DoPickFix3IC);
    QueueTaskList[233].SetAliasAndTask("Task_DoPlaceFix3IC",                    &iTask_DoPlaceFix3IC);
#if 0 // GATE(W906-TASKLIST) iConntectionOkTask 移植樹沒有（git grep 找不到 namespace 範圍、#if 0 之外的定義） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[234].SetAliasAndTask("ConntectionOkTask",                     &iConntectionOkTask);
#endif
#if 0 // GATE(W906-TASKLIST) TfBarCode 移植樹沒有成員 iInitialChangeFileTask（編譯器量的） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[235].SetAliasAndTask("InitialChangeFileTask",                 &fBarCode->iInitialChangeFileTask);
#endif
#if 0 // GATE(W906-TASKLIST) golden fBarCode->i2DIDCheckTask 移植樹沒有（git grep 只有 forms/fContact.h:1531 那個同名的 TfContact 成員，是另一個變數） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[236].SetAliasAndTask("2DIDCheckTask",                         &fBarCode->i2DIDCheckTask);
#endif
#if 0 // GATE(W906-TASKLIST) i2DIDCheckSH1Task 移植樹沒有（git grep 找不到 namespace 範圍、#if 0 之外的定義） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[237].SetAliasAndTask("2DIDCheckSH1Task",                      &i2DIDCheckSH1Task);
#endif
#if 0 // GATE(W906-TASKLIST) i2DIDCheckSH2Task 移植樹沒有（git grep 找不到 namespace 範圍、#if 0 之外的定義） —— 那個 task 沒翻；不補假變數（補了就是永遠不變的樁）⇒ 環裡這一列是空的
    QueueTaskList[238].SetAliasAndTask("2DIDCheckSH2Task",                      &i2DIDCheckSH2Task);
#endif
    QueueTaskList[239].SetAliasAndTask("BarcodeOutShuttle1Task",                &iInitialBarcodeOutShuttle1Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle1_Scan.cpp:76（golden BarCode.h:788）

    QueueTaskList[240].SetAliasAndTask("ShuttleFloatCheck1Task",                &iShuttleFloatCheck1Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle1_Scan.cpp:77（golden BarCode.h:882）
    QueueTaskList[241].SetAliasAndTask("SFCAutoTune1Task",                      &iSFCAutoTune1Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle1_SFCAutoTune.cpp:355（golden BarCode.h:884）
    QueueTaskList[242].SetAliasAndTask("BarcodeOutShuttle2Task",                &iInitialBarcodeOutShuttle2Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle2_Scan.cpp:29（golden BarCode.h:789）
    QueueTaskList[243].SetAliasAndTask("ShuttleFloatCheck2Task",                &iShuttleFloatCheck2Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle2_Scan.cpp:30（golden BarCode.h:883）
    QueueTaskList[244].SetAliasAndTask("SFCAutoTune2Task",                      &iSFCAutoTune2Task);   //AI(W906-TASKLIST) 20260927: golden fBarCode 的成員，移植樹是全域 BarCode_Shuttle2_SFCAutoTune.cpp:152（golden BarCode.h:885）
    QueueTaskList[245].SetAliasAndTask("ShakeInArmRotateKit",                   &iShakeInArmRotateKit);
    QueueTaskList[246].SetAliasAndTask("ShakeOutArmRotateKit",                  &iShakeOutArmRotateKit);
    if(fContactForm) QueueTaskList[247].SetAliasAndTask("ROILearningTask",                       &fContactForm->ROILearningTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[248].SetAliasAndTask("AutoContactTestTask",                   &fContactForm->AutoContactTestTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[249].SetAliasAndTask("StepContactTestLoadTask",               &fContactForm->StepContactTestLoadTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）

    if(fContactForm) QueueTaskList[250].SetAliasAndTask("StepContactTestUnloadTask",             &fContactForm->StepContactTestUnloadTask);   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    QueueTaskList[251].SetAliasAndTask("InArmAdditionalFunctionTask",           &iInArmAdditionalFunctionTask);
    QueueTaskList[252].SetAliasAndTask("OutArmAdditionalFunctionTask",          &iOutArmAdditionalFunctionTask);
    QueueTaskList[253].SetAliasAndTask("InArmInArmCheckShtFloatTask;",          &iInArmInArmCheckShtFloatTask);
    QueueTaskList[254].SetAliasAndTask("TrayZLoadTrayToWaitTask",               &iTrayZLoadTrayToWaitTask);
    QueueTaskList[255].SetAliasAndTask("LoaderTrackDetectICFloatingTask",       &iLoaderTrackDetectICFloatingTask);
    QueueTaskList[256].SetAliasAndTask("LoadNewLoaderTrayToCarTask",            &iLoadNewLoaderTrayToCarTask);
    QueueTaskList[257].SetAliasAndTask("LoaderTrayToFrontTask",                 &iLoaderTrayToFrontTask);
    QueueTaskList[258].SetAliasAndTask("LoaderTrayToRearTask",                  &iLoaderTrayToRearTask);
    QueueTaskList[259].SetAliasAndTask("AutoLoaderTask",                        &iAutoLoaderTask);

    QueueTaskList[260].SetAliasAndTask("UnLoadNewLoaderTrayTask",               &iUnLoadNewLoaderTrayTask);
    QueueTaskList[261].SetAliasAndTask("AutoLoaderReceiveTask",                 &iAutoLoaderReceiveTask);
    QueueTaskList[262].SetAliasAndTask("InArmPitchCHKTask",                     &iDoInArmPitchCHKTask);
    QueueTaskList[263].SetAliasAndTask("OutArmPitchCHKTask",                    &iDoOutArmPitchCHKTask);
    QueueTaskList[264].SetAliasAndTask("WhichAutoNeedTray",                     &iWhichAutoNeedTray);
    QueueTaskList[265].SetAliasAndTask("OutArmAfterPlaceToAutoTask",            &iDoOutArmAfterPlaceToAutoTask);        //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
    QueueTaskList[266].SetAliasAndTask("SiteMappingStep",                       &iDoSiteMappingStep);                   //Steven 20220708 : 針對Auto Site Map作紀錄
    if(fHome) QueueTaskList[267].SetAliasAndTask("HomeStep",                              &fHome->iHomeStep);                     //JerryYang 20230530 : add
    QueueTaskList[268].SetAliasAndTask("InitialStartTask",                      &iInitialStartTask);
    if(fContactForm) QueueTaskList[269].SetAliasAndTask("DeviceMapCheckTask",                    &fContactForm->iDeviceMapCheckTask);        //JerryYang 20250220 : 2DID硬體順序檢查功能   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[270].SetAliasAndTask("i2DIDMapCheckLoadTask",                 &fContactForm->i2DIDMapCheckLoadTask);      //JerryYang 20250220 : 2DID硬體順序檢查功能   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）
    if(fContactForm) QueueTaskList[271].SetAliasAndTask("i2DIDMapCheckUnloadTask",               &fContactForm->i2DIDMapCheckUnloadTask);    //JerryYang 20250220 : 2DID硬體順序檢查功能   //AI(W906-TASKLIST) 20260927: golden fContact＝移植樹 fContactForm（見函式上方）

    QueueTaskList[272].SetAliasAndTask("iAuto3MagazineTask",                    &iAuto3MagazineTask);
    QueueTaskList[273].SetAliasAndTask("iMagazineUpDownTask",                   &iMagazineUpDownTask);
    QueueTaskList[274].SetAliasAndTask("iMagazineTrayFeedTask",                 &iMagazineTrayFeedTask);
    QueueTaskList[275].SetAliasAndTask("iDoMagazineScanHasTrayTask",            &iDoMagazineScanHasTrayTask);
    QueueTaskList[276].SetAliasAndTask("iCatchTrayChangeTrayTask",              &iCatchTrayChangeTrayTask);
    QueueTaskList[277].SetAliasAndTask("iUnloadFixTray",                        &LastSet.iUnloadFixTray);              //AI(ht9045-atk-amr-flow) 20260427 (RogerYang) : ATK state machine visibility in StateRecord
    QueueTaskList[278].SetAliasAndTask("iAutoChkInSHLatchTask",                 &iAutoChkInSHLatchTask);                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    QueueTaskList[279].SetAliasAndTask("ReceiveAutoTrayTask[3]",                 &iReceiveAutoTrayTask[3]);
    QueueTaskList[280].SetAliasAndTask("ReceiveAutoTrayTask[4]",                 &iReceiveAutoTrayTask[4]);
    QueueTaskList[281].SetAliasAndTask("ReceiveAutoTrayTask[5]",                 &iReceiveAutoTrayTask[5]);
    //目前陣列定義300,超過要在增加
    //<==
    //Steven 20180808 (wei) : 修改紀錄Task的方式

    //ChungHung 201400613 add informoation for debug
#if 0 // GATE(W906-STATEREC-G3) sgTaskList（golden main.h:553 TStringGrid）移植樹沒有；它只給 Task_List.xls（SGDToXLS 在移植樹是 no-op）
    sgTaskList->RowCount=qTaskCount;                                            //Steven 20180808 (wei) : 修改紀錄Task的方式
#endif
    fMain->StringGrid2->RowCount=qTaskCount;                                           //Steven 20180808 (wei) : 修改紀錄Task的方式
    fMain->StringGrid2->Cells[0][0]="Task";
#if 0 // GATE(W906-STATEREC-G3) sgTaskList（golden main.h:553 TStringGrid）移植樹沒有；它只給 Task_List.xls（SGDToXLS 在移植樹是 no-op）
    sgTaskList->Cells[0][0]="Task";
#endif

    for(int i=1; i<qTaskCount; i++)
    {
#if 0 // GATE(W906-STATEREC-G3) sgTaskList（golden main.h:553 TStringGrid）移植樹沒有；它只給 Task_List.xls（SGDToXLS 在移植樹是 no-op）
        sgTaskList->Cells[0][i]=QueueTaskList[i].Alias;
#endif
        fMain->StringGrid2->Cells[0][i]=QueueTaskList[i].Alias;
    }
}
