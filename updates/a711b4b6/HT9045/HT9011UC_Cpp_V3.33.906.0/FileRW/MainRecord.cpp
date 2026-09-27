// ===========================================================================
//  FileRW/MainRecord.cpp -- golden TfMain::UpdateRecordScreen（稼動時間累計）與 TfMain::UpdateRunInfo（Run Info／良率圖）。
//
//  AI(W906-PROD-S113) 20260926（Steven 團隊）：新檔。RULINGS_20260926 S113、S120-3「做，先跟 Steven02 對齊」；
//    Steven02 20260926 21:06（FROM_STEVEN §4）同意兩支本體都由 St01 翻，UpdateRunInfo 的每秒呼叫也由 St01 掛在 wb_serve 主迴圈。
//    golden 一律照主 repo 那份 V912：D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 → UTF-8），行號都是那一份的。
//
//  golden 的兩支與呼叫時機
//    UpdateRecordScreen  main.cpp:8584-8687。呼叫：
//      * Timer1Timer :3284 UpdateRecordScreen(false) —— Timer1 Interval=30（main.dfm:17306），本體前面還要過
//        InitialOK（:2776）、bRunTimer1 防重入、bCloseByAutoUpdate、N07 工號等待、bTimer1Begin（:3208）、兩拍間隔 ≥20 ms（:3254）、
//        fForGetCompomentName||SystemInitialOK==false（:3263）這幾關 ⇒ 約每 30 ms 一次。
//      * Timer1Timer :3036 —— SECS host alarm 鎖機期間也照叫（RogerYang 20260823），確保停機時間照算。
//      * TfNote::Timer1Timer（note.cpp:3172，Interval=10）:3384／TMyMessageBox::Timer1Timer（mymessbox.cpp:542）:547 都呼叫 fMain->Timer1Timer ——
//        **告警框／訊息框開著的時候照樣累計**（Jam Time 就是這樣來的）。見下面「⚠ 框開著時」。
//      * spbClearRecordClick :31137、TfLotInfo::btnASECL_LotStartClick uLotInfo.cpp:10813 呼叫 UpdateRecordScreen(true)
//        （本體不用 Attr，true／false 做的事一樣）。前者是 S119（Main.Record CLEAR），後者是 CC_ASE_CL（S25），本檔都不接。AI(W906-PROD-S113) 20260927：S119 的 CLEAR 已由 St02 經函式指標 W906_UpdateRecordScreenBody（JsonBridge/actions/MainRecordClear.cpp:24）呼叫本檔 W906_TfMain_UpdateRecordScreen，指標在 wb_serve.cpp:4111（St01 行尾）裝上。
//    UpdateRunInfo  main.cpp:22740-22812。呼叫：Timer2Timer :21666（Timer2 沒寫 Interval ⇒ VCL 預設 1000 ms，main.dfm:17311），
//      前面只有 :21492-21499 的 fShow／InitialOK／bTimer2Run 三關；:22080-22084 的 `if(SystemStart==true) return;` 在它**後面**
//      ⇒ 運轉中也會算（Steven02 21:06 提醒要看這一點）。本體自己再限成「分鐘變了才做」，而且見下面 golden 怪處 (1)。
//
//  寫到哪些欄位（交件報告有同一張表）
//    UpdateRecordScreen（每拍，P = 距上一次呼叫的毫秒數）：
//      LastSet.SystemAccSecond[0..3][stPowerOn]      —— P>0 就加（負值先歸 0 並寫一筆 NewRecordProcess，Sam 20241218）
//      LastSet.SystemAccSecond[0..3][stStartTime]    —— SystemStart==true
//        [stHomeTime]（fAllMotorHome==false）／[stContactTest]（fContact->fShow）／[stProductTime]（其餘）三選一
//      LastSet.SystemAccSecond[0..3][stPauseTime]    —— 沒在跑、沒告警框、不是 SystemNG，而且機台內有 IC
//      LastSet.SystemAccSecond[0..3][stJamTime]      —— fNote->fShow（告警框開著）
//      LastSet.SystemAccSecond[0..3][stSystemNGTime] —— SystemNG
//      iPauseTime（cmydef.cpp 全域，cShowBinSelect 的 Gross UPH 用）、lAutoClean_TimeCount（VTEST 的定時 Auto Clean 用）
//      ⇒ 這些都在 LAST_GENERAL_SET 裡（iPauseTime／lAutoClean_TimeCount 除外），之後任何一次 WriteLastDataFile 都會把它寫進
//        D:\HT9045\system\lastdata.dat（ctest 由 W906_LastDataPath 轉開）。開機值＝上一次存的 lastdata.dat（ReadLastDataFile）。
//    UpdateRunInfo（本體每 5 分鐘真的做一次）：RunInfo.iYieldChart[4][8][25]、iYieldHour[25]、iYieldMin[25]、SystemTime、MTBA、MUBA；
//      另外叫 fObserver->ProcessRunInfo()（labMTBA／labMUBA／labMTBF／pnlDayJamRate 的 Caption）與 fObserver->bShow 時的
//      UpdateYieldChart()（ChartYield 替身的 Series）。RunInfo 不存檔（SECS SV 1026／1027／1031 與 cMyDB 讀它）。
//
//  接法（本檔只編進 wb_serve；不要搬進任何 archive —— 沒人引用時成員不會被抽出，陷阱 #2）
//    wb_serve 主迴圈 PumpTick 那一行的**下一行**（W906_StateRecordTimer2Pump 那一行）行尾，`if (pumpBeat)` 裡呼叫
//    W906_MainRecordTimer1Tick() 與 W906_MainRunInfoTimer2Tick()（片段見交件報告）。
//    為什麼不放進 WebBridgeTags.cpp 的 PumpTick：WebBridgeTags.cpp 另外編進 test_wb_tags、test_wb_simpump（tests/CMakeLists.txt），
//    那兩支沒有本檔 ⇒ PumpTick 裡直接呼叫會讓兩支 ctest 連結失敗（undefined reference）；要補就得把本檔連同它用到的
//    FileRW/_KitSuck.cpp、JsonBridge/FormJson.cpp 一起加進兩個測試 target。放在 wb_serve 的 pumpBeat 上，節拍與 PumpTick 完全相同
//    （同一個 500 ms 拍子、緊接在 PumpTick 之後），而且只有 wb_serve 會連到本檔。
//
//  ⚠ 框開著時（golden note.cpp:3384／mymessbox.cpp:547 → fMain->Timer1Timer → :3284）
//    移植樹的告警框／是否框／ShowMyMessage 框是 wb_serve 裡的三個等待迴圈（tick 執行緒停在 PumpTick 裡面等網頁回答），
//    主迴圈的 pumpBeat 在框開著時不會輪到 ⇒ 本檔的每拍呼叫也停。停的期間時間不會丟（P 是「距上次呼叫」，下一拍一次補上），
//    但**歸類錯**：框關掉之後 fNote->fShow 已經是 false、SystemStart 也被告警停機設成 false ⇒ 整段告警時間落進 PauseTime
//    （機台內有 IC 時）或什麼都不算，stJamTime 幾乎不會動。要照 golden，需要在 W906_ModalWaitTick（wb_serve.cpp，
//    `ht9045::W906_FlushFlagTick();` 那一行，NB2 R70 MW-F 已經在那裡放 golden fMain->Timer1Timer 的其他段）也呼叫
//    W906_MainRecordTimer1Tick()。那一行在三個等待迴圈的範圍、不是 St01 的段 ⇒ 本檔不動，交件報告列給段落 owner。
//
//  跟 golden 不同的地方（前提不在或 C++ 語法，不是改行為）
//    ① 系統計時器的起點：golden 在 SYSTEM_MODULAR 建構子（database.cpp:47，程式一載入就建 HSys）呼叫
//       HSys.SysTimer.LatchCycleTime(true)；移植樹那個建構子整段 #if 0（database.cpp:139-156），HSys.SysTimer 的 rStartDelay
//       從來沒被設過（靜態儲存 = 0）⇒ 第一次 LatchCycleTime() 會回「開機以來的 QPC 毫秒數」（電腦開了幾天就是幾億毫秒，
//       超過約 24.8 天還會溢位成負數）一次灌進 PowerOn。所以 W906_MainRecordTimer1Tick 第一次被呼叫時先補這一下
//       （W906_MainRecordBootLatch 若已由 wb_serve 開機呼叫過就不再補）。代價：wb_serve 開機到第一拍之間（通常幾秒到幾十秒）
//       不算進 PowerOn —— golden 會算。要不要在 wb_serve 開機鏈（LoadMachineConfig 之後那一行）補呼叫 W906_MainRecordBootLatch()，見交件報告「待 Steven 決定」。
//    ② 呼叫端的關卡：golden Timer1 的 SystemInitialOK（:3263）在 wb_serve 永遠是 false（golden 在 FormShow :10092 無條件設 true；
//       移植樹沒有人設，tools/wb_serve.cpp 只讀）⇒ 照翻這一關的話本功能永遠不會動。這裡只看 InitialOK（Timer1 :2776、
//       Timer2 :21495；wb_serve 在 PumpInit 成功時設 true，WebBridgeTags.cpp:563）。fForGetCompomentName、bTimer1Begin、
//       TfMain::fShow 是 TfMain 成員、移植樹 forms/fMain.h 沒有（golden 開機完成後分別是 false／true／true）⇒ 等同通過。
//       bCloseByAutoUpdate、N07 工號等待（客戶功能 bN07_EnableEmployeeIdCheak，S25）不接：它們只讓 golden 少叫幾次，
//       時間不會丟（同上，P 是距上次呼叫）。
//    ③ 節拍：golden Timer1 約 30 ms 一次，這裡是 wb_serve 的 500 ms 拍子（B13 裁決 kServeTickMs）。總時間相同，
//       只有「狀態剛好在兩拍之間變化」時，那不到 500 ms 會整段算給下一拍看到的狀態。
//    ④ fMesSystem->SetOEEState（:8635／:8648／:8657／:8659）：移植樹 TfMesSystem::SetOEEState 是 GATE（forms/fMesSystem.h:686，
//       沒有本體），golden 本體第一行是 `if(IniConfig.bVTESTFunction==false) return;`（Mes/fVATMesFileSys.cpp，客戶功能，S25）
//       ⇒ 閘住、只註記。非 VTEST 機台行為相同。
//    ⑤ fLotInfo->SaveBackEventLogInfo（:8669-8675）：移植樹 forms/fLotInfo.h 沒有這一支；它的開關 fLotInfo->bEventLogAlarm 在
//       golden 由 TfNote::FormShow（note.cpp:2367）設 true，移植樹沒有任何地方設 true（只有 fLotInfo.cpp:582 設 false）
//       ⇒ 整段閘住（連 bEventLogAlarm=false 一起閘：將來有人設 true 而本體還沒翻時，旗標留著比默默清掉好查）。
//    ⑥ fLotInfo->SaveASECLTestLogInfo（:8685）：本體第一行 `if(CUSTOMER_CODE!=CC_ASE_CL) return;`（uLotInfo.cpp:11031，S25），
//       移植樹沒有這一支 ⇒ 分鐘判斷照翻、呼叫閘住。
//    ⑦ RunInfo.SystemTime（:22796 `=StatusBar1->Panels->Items[6]->Text`）：移植樹沒有 StatusBar1（同 cStateRecord.cpp:729-731 GATE-G1）。
//       golden 那一格只有 ProcessTimeUpdate 寫（main.cpp:8239-8241，sAlarmTime 的格式 "%04d-%02d-%02d %02d:%02d:%02d"），
//       移植樹週期性的 ProcessTimeUpdate 沒有跑（WebBridgeTags.cpp PumpTick 只承接了 GetTimeInfo），sAlarmTime 大多是舊值 ⇒
//       這裡用同一個格式、從 PumpTick 剛更新的 SystemYear..SystemSec 組出來（golden 那一格最多晚 ~330 ms，這裡同一拍）。
//    ⑧ TestSocket（:22762／:22764）：不 include aHotPlateSubstrate.h（它跟本檔用到的標頭鏈有衝突，FileRW/_KitSuck.cpp 檔頭），
//       改經 FileRW_KitSuckDims(0, …) 讀 iShtRow／iShtCol（同 FileRW/StartCondition.gen.inc 的作法）。
//    ⑨ UpdateRunInfo 會碰 fObserver 的 Caption 與 ChartYield，W906_MainRunInfoTimer2Tick 持 FormLock（可重入；observer.get 也持它）。
//    ⑩ 觀測用（golden 沒有）：UpdateRunInfo 真的做了一次就印一行 stdout（約 5 分鐘一行）；第一次補計時器起點時印一行。
//
//  golden 怪處（照翻，不修；要改行為要 Steven 決定）
//    (1) :22748 `LastSet.SystemAccSecond[0][stPowerOn]/60>25` —— 函式上方的註解（:22737-22738）說「程式啟動後前 25 分鐘每分鐘
//        記一次，之後每 5 分鐘」。但 SystemAccSecond 從 Steven 20190714 起改成毫秒（LastSet.h:114），/60>25 變成「> 1.5 秒」；
//        而且 [0] 是從上次清除計數以來的累計、不是這次開機以來 ⇒ 實際上幾乎一開始就是「只在分鐘 %5==0 時記」。
//    (2) SystemAccSecond 是 long（32 位元，毫秒）⇒ 同一格累計超過約 24.8 天會變負數。golden 只替 PowerOn 做了歸 0 保護
//        （:8596-8600），其他格沒有。
// ===========================================================================
#include "cmydef.h"             // SystemStart／fAllMotorHome／InitialOK／SystemYear..SystemSec／iPauseTime／lAutoClean_TimeCount／
                                // bUseTwoArm32Site（MachineType.h：eSystemTime st*、MAX_SOCKET_ROW／COL、ChangeToFloatNonPcnt）
#include "LastSet.h"            // LastSet.SystemAccSecond[4][8]（LastSet.h:114）／iJamCount[3]（:255）
#include "cprod.h"              // RunInfo（RUN_INFO，cprod.h:2705）、TestIF_File（bEnabledAutoCleanTimeCT、iShuttleMode、iShuttle_Sel）
#include "CosFunction.h"        // CosFunction.bAutoCleanTimeCT（只有 VTEST_Funtion 設 true，CosFunction.cpp:2548）
#include "database.h"           // HSys.SysTimer（TQPF_Timer，database.h:244）
#include "halarm.h"             // SystemNG（halarm.h:66，定義 HAlarm.cpp:34）
#include "cMyDB.h"              // NewRecordProcess（cMyDB.h:129）
#include "cpublic.h"            // ConvertMSecToSPC（cpublic.h:28）
#include "cSocket.h"            // ArmData[3]（cSocket.h:270；TArm::ArmSKET[][]->GetPassCT()／GetTotal()）
#include "atester_shims.h"      // fContact（TfContactShim::fShow，atester_shims.h:157／:251）
#include "forms/fObserver.h"    // fObserver（cObserver.cpp:3289）：bShow／UpdateYieldChart／ProcessRunInfo／labMUBA
#include "forms/fNote.h"        // fNote（forms/fNote.cpp:48）：fShow／edErrorCode

#include <windows.h>            // GetTickCount
#include <cstdio>
#include <cstdlib>              // abs

// 不 include csystem.h／aHotPlateSubstrate.h（理由同 FileRW/MainBoot.cpp、MainClose.cpp 檔頭：標頭鏈會帶進衝突的定義），只宣告要用的函式。
bool HasICUnderMachine();                                                       // csystem.h:105
bool HasAnyICInMachine();                                                       // csystem.h:109
void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket）
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } } // JsonBridge/FormJson.cpp

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

bool     g_sysTimerLatched = false;   // ① HSys.SysTimer 的起點補過了沒
unsigned g_runInfoRecords  = 0;       // ⑩ UpdateRunInfo 真的做了幾次

}  // namespace

// ---------------------------------------------------------------------------
//  golden main.cpp:8584  void __fastcall TfMain::UpdateRecordScreen(bool Attr)
//  golden 本體只用全域與 fXxx->（不用 this），所以這裡是自由函式。S119（Main.Record CLEAR，golden :31137）之後可直接呼叫這一支。
// ---------------------------------------------------------------------------
void W906_TfMain_UpdateRecordScreen(bool Attr)                                  //Steven 20110827 : 重新整理
{
    (void)Attr;                                                                 // golden 本體沒有用到 Attr
    static Word OldRecordMin=9999;

    int P=HSys.SysTimer.LatchCycleTime();                                       //Steven 20190714 : 計算系統時間 (MS)
    HSys.SysTimer.LatchCycleTime(true);

    //程式打開-------
    if(P>0)
    {
        for(int i=0; i<4; i++)
        {
            if(LastSet.SystemAccSecond[i][stPowerOn]<0)                         //Sam 20241218 : 增加保護避免修正PowerOn負值問題
            {
                NewRecordProcess("", "Reset PowerOn time", IntToStr(i));
                LastSet.SystemAccSecond[i][stPowerOn]=0;
            }
            LastSet.SystemAccSecond[i][stPowerOn]+=P;                           //Acc Power On
        }
    }

    //機器有在跑------
    if(SystemStart==true)
    {
        for(int i=0; i<4; i++)
        {
            LastSet.SystemAccSecond[i][stStartTime]+=P;                         //機器在動的時間
        }

        if(fAllMotorHome==false)
        {
            for(int i=0; i<4; i++)
                LastSet.SystemAccSecond[i][stHomeTime]+=P;                      //歸零時間
        }
        else if(fContact->fShow==true)
        {
            for(int i=0; i<4; i++)
                LastSet.SystemAccSecond[i][stContactTest]+=P;                   //contact test or auto height Time
        }
        else
        {
            for(int i=0; i<4; i++)
                LastSet.SystemAccSecond[i][stProductTime]+=P;                   //Acc Wrok Time

            // 客戶功能（S25）：CosFunction.bAutoCleanTimeCT 只有 VTEST 設 true ⇒ 其他機台每拍都是 else 那一支（歸 0）。
            //   VTEST 且 TestIF_File.bEnabledAutoCleanTimeCT 時，這個累計讓 atester_ProcessCount.cpp:2025-2040 的「定時 Auto Clean」
            //   （OneCycle＋AutoClean）從此會真的觸發 —— 移植樹以前沒有人加 lAutoClean_TimeCount，那一段走不到。
            if(CosFunction.bAutoCleanTimeCT==true &&                            //jou 20250102 : auto clean triger time count
               TestIF_File.bEnabledAutoCleanTimeCT==true)
                lAutoClean_TimeCount+=P;
            else
                lAutoClean_TimeCount=0;
        }

#if 0 // GATE(W906-PROD-S113-G1) ④ TfMesSystem::SetOEEState 沒有本體（forms/fMesSystem.h:686 GATE）；golden 本體第一行 bVTESTFunction==false return（S25）
        fMesSystem->SetOEEState(stStartTime);
#endif // GATE(W906-PROD-S113-G1)
    }

    //機器沒在跑------
    if(SystemStart==false && fNote->fShow==false && SystemNG==false)
    {
        if(HasICUnderMachine() || HasAnyICInMachine())                          //JerryYang 20180423 (Steven) 機台內有IC才算進PAUSE TIME
        {
            for(int i=0; i<4; i++)
                LastSet.SystemAccSecond[i][stPauseTime]+=P;                     //Pause Time
            iPauseTime+=P;                                                      //JerryYang 20250120 : add
        }

#if 0 // GATE(W906-PROD-S113-G1) ④ 同上
        fMesSystem->SetOEEState(stPauseTime);
#endif // GATE(W906-PROD-S113-G1)
    }

    if(fNote->fShow)
    {
        for(int i=0; i<4; i++)
            LastSet.SystemAccSecond[i][stJamTime]+=P;                           //Jam Time
        iPauseTime+=P;                                                          //JerryYang 20250120 : add
#if 0 // GATE(W906-PROD-S113-G1) ④ 同上（兩支都是 SetOEEState，整個 if/else 一起閘）
        if(fNote->edErrorCode->Text.Pos("JAM")>0)
            fMesSystem->SetOEEState(stJamTime);
        else
            fMesSystem->SetOEEState(stPauseTime);
#endif // GATE(W906-PROD-S113-G1)
    }

    if(SystemNG)
    {
        for(int i=0; i<4; i++)
            LastSet.SystemAccSecond[i][stSystemNGTime]+=P;                      //SystemNG Time
        iPauseTime+=P;                                                          //JerryYang 20250120 : add
    }

#if 0 // GATE(W906-PROD-S113-G2) ⑤ TfLotInfo::SaveBackEventLogInfo 沒翻（golden uLotInfo.cpp:10903，寫 HANDLER LOG_*.csv）；bEventLogAlarm 移植樹沒有人設 true（golden note.cpp:2367）
    if(SystemStart && fAllMotorHome==true &&                                    //Steven 20181224 : For ASE-CL
       fNote->fShow==false && fSetup->fShow==false &&
       fLotInfo->bEventLogAlarm==true)
    {
        fLotInfo->SaveBackEventLogInfo(aBackEventLogMessage);
        fLotInfo->bEventLogAlarm=false;
    }
#endif // GATE(W906-PROD-S113-G2)

    if(OldRecordMin==9999)
    {
        OldRecordMin=SystemMin;
    }

    if(abs(OldRecordMin-SystemMin)>0)
    {
        OldRecordMin=SystemMin;
#if 0 // GATE(W906-PROD-S113-G3) ⑥ TfLotInfo::SaveASECLTestLogInfo 沒翻；golden 本體第一行 CUSTOMER_CODE!=CC_ASE_CL return（uLotInfo.cpp:11031，S25）
        fLotInfo->SaveASECLTestLogInfo();
#endif // GATE(W906-PROD-S113-G3)
    }
}

// ---------------------------------------------------------------------------
//  golden main.cpp:22736-22740
//  // 紀錄每個Site的良率，並繪製成圖表於fObserver內
//  // 程式啟動後，前25分鐘，每分鐘紀錄一次；之後每5分鐘紀錄一次          ← 見檔頭 golden 怪處 (1)：實際不是這樣
//  void __fastcall TfMain::UpdateRunInfo()
// ---------------------------------------------------------------------------
void W906_TfMain_UpdateRunInfo()                                                //Steven 20090714
{
    static int iOldMin=-1;
    long lPass=0, lTotal=0;

    if(iOldMin==SystemMin)
        return;
    iOldMin=SystemMin;
    if(LastSet.SystemAccSecond[0][stPowerOn]/60>25 && iOldMin%5)                //AI(W906-PROD-S113) 20260926: 照翻。SystemAccSecond 是毫秒 ⇒ /60>25 是 >1.5 秒，不是註解說的 25 分鐘（檔頭 golden 怪處 (1)）
        return;
    ++g_runInfoRecords;                                                         // ⑩ 觀測用（golden 沒有）

    for(int i=0; i<MAX_SOCKET_ROW; i++)                                         //Steven 20140402 : MAX_Index_Row -->  MAX_SOCKET_ROW
        for(int j=0; j<MAX_SOCKET_COL; j++)                                     //Steven 20140402 : MAX_Index_Row -->  MAX_SOCKET_ROW
            for(int k=24; k>0; k--)
                RunInfo.iYieldChart[i][j][k]=RunInfo.iYieldChart[i][j][k-1];

    for(int j=24; j>0; j--)
    {
        RunInfo.iYieldHour[j]=RunInfo.iYieldHour[j-1];
        RunInfo.iYieldMin[j] =RunInfo.iYieldMin[j-1];
    }

    struct { int iShtRow, iShtCol; } TestSocket;                                // ⑧ golden TestSocket（TMyKitSuck）的 iShtRow／iShtCol
    {
        int iMaxRow=0, iMaxCol=0;
        FileRW_KitSuckDims(0, &TestSocket.iShtRow, &TestSocket.iShtCol, &iMaxRow, &iMaxCol);
    }
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            lPass=0;
            lTotal=0;
            if(bUseTwoArm32Site==true)
            {
                lPass=ArmData[2]->ArmSKET[i][j]->GetPassCT();
                lTotal=ArmData[2]->ArmSKET[i][j]->GetTotal() ;
            }
            else
            {
                if(TestIF_File.iShuttleMode==0 || (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0))
                {
                    lPass+=ArmData[0]->ArmSKET[i][j]->GetPassCT();
                    lTotal+=ArmData[0]->ArmSKET[i][j]->GetTotal() ;
                }

                if(TestIF_File.iShuttleMode==0 || (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1))
                {
                    lPass+=ArmData[1]->ArmSKET[i][j]->GetPassCT();
                    lTotal+=ArmData[1]->ArmSKET[i][j]->GetTotal() ;
                }
            }

            if(lTotal!=0)
                RunInfo.iYieldChart[i][j][0]=lPass*100/lTotal;
        }
    }

    RunInfo.iYieldHour[0]=SystemHour;
    RunInfo.iYieldMin[0] =SystemMin;

#if 0 // GATE(W906-PROD-S113-G4) ⑦ StatusBar1（golden main.h TStatusBar*）移植樹沒有；下一行用同一格式組出同一個值
    RunInfo.SystemTime=StatusBar1->Panels->Items[6]->Text;
#endif // GATE(W906-PROD-S113-G4)
    RunInfo.SystemTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);   // ⑦ golden ProcessTimeUpdate main.cpp:8239 sAlarmTime → :8241 Panels[6]

    if(fObserver->bShow)
        fObserver->UpdateYieldChart();

    fObserver->ProcessRunInfo();
    if(LastSet.iJamCount[1]!=0)                                                 //Steven 20141111 : 秒轉為時分秒
    {
        RunInfo.MTBA=ConvertMSecToSPC(ChangeToFloatNonPcnt((double)((LastSet.SystemAccSecond[0][stPauseTime]+LastSet.SystemAccSecond[0][stProductTime]+LastSet.SystemAccSecond[0][stJamTime])), (double)(LastSet.iJamCount[1])));  //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
    }
    else
    {
        RunInfo.MTBA=ConvertMSecToSPC(LastSet.SystemAccSecond[0][stPauseTime]+LastSet.SystemAccSecond[0][stProductTime]+LastSet.SystemAccSecond[0][stJamTime]);
    }

    RunInfo.MUBA=fObserver->labMUBA->Caption;

    std::printf("FileRW MainRecord: UpdateRunInfo #%u at %02d:%02d (golden main.cpp:22740) -- iYieldChart shifted; MTBA=%s MUBA=%s PowerOn=%ld ms\n",
                g_runInfoRecords, (int)SystemHour, (int)SystemMin, RunInfo.MTBA.c_str(), RunInfo.MUBA.c_str(),
                (long)LastSet.SystemAccSecond[0][stPowerOn]);   // ⑩
}

// ---------------------------------------------------------------------------
//  ① golden SYSTEM_MODULAR::SYSTEM_MODULAR（database.cpp:47）`SysTimer.LatchCycleTime(true);` —— 系統時間的起點。
//  wb_serve 開機時可以明確呼叫一次（片段見交件報告「待 Steven 決定」B）；沒呼叫的話 W906_MainRecordTimer1Tick 第一拍補。
// ---------------------------------------------------------------------------
void W906_MainRecordBootLatch()
{
    HSys.SysTimer.LatchCycleTime(true);                                         //Steven 20190714 : 計算系統時間   // golden database.cpp:47
    g_sysTimerLatched=true;
}

// ---------------------------------------------------------------------------
//  golden Timer1Timer :3284 的呼叫點（wb_serve 主迴圈每個 pumpBeat 呼叫一次，緊接在 PumpTick 之後）。關卡見檔頭 ②。
// ---------------------------------------------------------------------------
void W906_MainRecordTimer1Tick()
{
    if(InitialOK==false)                                                        // golden Timer1Timer :2776
        return;

    if(g_sysTimerLatched==false)                                                // ① 起點沒補過 ⇒ 先補，這一拍 P≈0
    {
        W906_MainRecordBootLatch();
        std::printf("FileRW MainRecord: HSys.SysTimer start latched on the first tick (golden SYSTEM_MODULAR ctor database.cpp:47 is #if 0 in this tree); "
                    "LastSet.SystemAccSecond now accumulates every 500 ms (golden TfMain::UpdateRecordScreen main.cpp:8584)\n");
    }

    W906_TfMain_UpdateRecordScreen(false);                                      // golden :3284
}

// ---------------------------------------------------------------------------
//  golden Timer2Timer :21666 的呼叫點（wb_serve 主迴圈每個 pumpBeat 呼叫；Timer2 是 1000 ms，這裡用 GetTickCount 限成每秒一次，
//  同 cStateRecord.cpp W906_StateRecordTimer2Pump 的作法）。關卡：Timer2Timer :21492-21499 的 InitialOK（fShow／bTimer2Run 見檔頭 ②）。
// ---------------------------------------------------------------------------
void W906_MainRunInfoTimer2Tick()
{
    static DWORD s_last=0;
    const DWORD now=::GetTickCount();
    if(s_last!=0 && (DWORD)(now-s_last)<1000u) return;
    s_last=now;

    if(InitialOK==false)                                                        // golden Timer2Timer :21495-21496
        return;

    FormLockGuard lock;                                                         // ⑨ fObserver 的 Caption／ChartYield
    W906_TfMain_UpdateRunInfo();                                                // golden :21666
}
