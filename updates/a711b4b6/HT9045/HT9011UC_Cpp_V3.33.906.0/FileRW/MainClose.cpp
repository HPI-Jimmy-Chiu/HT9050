// ===========================================================================
//  FileRW/MainClose.cpp -- golden 關程式時的「生產資料存檔」與主畫面 Exit 鈕（V912 main.cpp）。
//
//  AI(W906-PROD-S95) 20260926（Steven 團隊）：新檔。RULINGS_20260926 S95 拆成兩段，S120-1（Steven 20:3x）
//    「提前做生產資料那幾項」＝本檔；RunMode.txt／LotSummary.csv／RMS 與 Jimmy 的 SaveMachineRecord／
//    TimerRecordLoaderDate／CheckRunMode 仍排後，本檔不做（下面每一處都標了 golden 行號與「不做」）。
//    （AI(W906-PROD-S121) 20260926：SaveMachineRecord／TimerRecordLoaderDate 已照 Jimmy J10 併進，見下面 S121 段；RunMode.txt／CheckRunMode 仍只盤點。）
//    （AI(W906-PROD-S95R) 20260926（Steven 團隊）：S95 剩下的四項也接了（S121 交件的 D5）—— :11863 SaveRunMode（RunMode.txt）、
//      :11864 LotSummary.WriteFile（LotSummary.csv）、:11924-11927 SaveProductionRecord、:12051-12057 SaveRmsInfo，見下面 S95R 段；
//      CheckRunMode（golden main.cpp:33661，RunMode.txt 的讀者）歸 Jimmy，仍不做；:11881-11884 ResetAutoClean 仍不做（會跳模態框）。）
//    golden：D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950，唯讀）；行號對 UTF-8 轉檔（只換編碼、行號不變）。
//
//  為什麼要做：golden 沒有定時存 lastdata.dat（23 個呼叫點沒有一個在 Timer 裡），關程式時的 WriteLastDataFile(true)
//    是最後一次存檔 —— 移植樹沒有它的話，上一次事件存檔之後累計的產量，關程式時全部丟掉。
//
//  兩個入口：
//    W906_ProdCloseSave()          golden TfMain::FormClose（main.cpp:11852）的生產資料段。wb_serve 在「正常關站」那一點呼叫
//                                  （tools/wb_serve.cpp 主迴圈結束後、寫 Program Close=1 之前；共用檔，片段見交件報告）。
//                                  ⚠ 目前 wb_serve 唯一的正常關站是 --seconds 到期；平常不帶 --seconds 時只能 Ctrl-C／關主控台，
//                                    那條路（W906_ConsoleCtrl，另一條執行緒）只寫 Program Close=1，不會跑到這裡。
//    W906_Main_CloseProgramOp()    golden TfMain::sbCloseProgramClick（main.cpp:29051-29131）＋它最後的 Close() 走進 FormClose 的
//                                  生產資料段。WS 動作 act.main.closeProgram（分派片段見交件報告；不在 WebCmdGuard 白名單）。
//                                  ⚠ （S95 原句）只做「先存生產資料」那一半：wb_serve 沒有「請伺服器正常結束」的入口，存完不會關站
//                                    （回應 shutdown.implemented=false，頁面照實告訴操作員）。關站那一半的設計見交件報告。
//                                  AI(W906-PROD-S121) 20260926：關站那一半做了（見下面 S121 段）—— 存完會要求 wb_serve 結束。
//
//  golden FormClose 裡屬於生產資料、本檔做的（依 golden 順序）：
//    :11861  WriteLastDataFile(true)                 lastdata.dat／lastdata_backup.dat／lastdata_backup2.dat＋config.ini 四節
//    :11862  SaveMachineRecord()                     machinerecord.dat…（AI(W906-PROD-S121)，本體 cinitial.cpp:17017）
//    :11863  SaveRunMode()                           system\RunMode.txt 一行 "RunMode=%d"（AI(W906-PROD-S95R)，本體在本檔 W906_TfMain_SaveRunMode）
//    :11864  LotSummary.WriteFile()                  system\LotSummary.csv（AI(W906-PROD-S95R)，本體 cSocket.cpp:1198，跟 golden cSocket.cpp:997 逐字同）
//    :11919  RunInfo.SaveJamRateByDay(false)         DailyJamRate txt（本體 cprod.cpp 還在 #if 0，這裡照 golden 另寫一份，見下）
//    :11924-11927  SaveProductionRecord（每個 tray）   AI(W906-PROD-S95R)：在關站段 ShutdownSequence（照 golden 的位置，executed 才做）
//    :12051-12057  SaveRmsInfo（bShowLotInfo＋機台內有 IC）   AI(W906-PROD-S95R)：同上，在關站段
//    :11999-12001  #ifdef DEBUG_AUTO_CLEAN btSavelog->Click() #endif
//                  —— golden MachineType.h:26 把 DEBUG_AUTO_CLEAN 註解掉，出貨版不編；btSavelogClick（:31310）存的是
//                     Auto Clean 除錯 memo（d:\AutoCleanLogs\），不是生產資料。照 golden 保留成同一個 #ifdef。
//    :12195  fCounterClear->WriteCTInfo()            system\Arm0-2／ArmHis0-2／ArmByLot0-2 .dat（＋_backup.dat）
//  golden sbCloseProgramClick 的 :29129 ReadWriteBinCountMode(false)（system\BinCount.txt）只在 Exit 鈕這條路（FormClose 沒有）。
//
//  ⚠ 會碰到的真實檔（本檔的程式照 golden；**沒有實際執行過**）：
//    D:\HT9045\system\lastdata.dat、lastdata_backup.dat、lastdata_backup2.dat（WriteLastDataFile(true)；ctest 由 W906_LastDataPath 轉開）
//    D:\HT9045\config\config.ini [O_Count]…（WriteLastDataFile 的尾段，cprod.cpp；VTEST＋bUseHeadContactCount 時改寫配方 HandlerCondition.Data）
//    D:\HT9045\system\BinCount.txt [System] Bin／ErrorBin（ReadWriteBinCountMode(false)；W906_BINCOUNT_PATH 可轉開）—— 只有 Exit 鈕
//    D:\HT9045_Log\JamRate_Daily\<sMachineType>_<SocketHandlerID>_<開機那天 YYYY-MM-DD>_DailyJamRate.txt（整份覆寫）
//    D:\HT9045\system\Arm{0,1,2}.dat、ArmHis{0,1,2}.dat、ArmByLot{0,1,2}.dat 與各自的 _backup.dat（18 檔；
//      TestIF_File.bLowYieldAlarmByBin 時另寫同名 .ini 9 檔）—— W906_MACHINERECORD_DIR 可轉開（ctest 全體都設）
//    AI(W906-PROD-S95R) 20260926 加的四個（本檔只編進 wb_serve ⇒ ctest 碰不到；前三個是 golden 字面路徑，沒有轉開接縫）：
//    D:\HT9045\system\RunMode.txt（SaveRunMode，整份覆寫，一行 "RunMode=<LastSet.iRunStartMode>"、沒有換行）
//    D:\HT9045\System\LotSummary.csv（LotSummary.WriteFile，整份覆寫：32 列 site × 256 欄＋1 列總計，CRLF）
//    D:\HT9045_Log\UnloadTrayLog\YYYYMM\ 資料夾（SaveProductionRecord：只有 CosFunction.bSaveProductionLogByUnloaderTray〔全樹只有
//      FUNC_CC_Murata 設 true〕且那一盤 HasRealIC 才建；csv 那一段在移植樹是 #if 0 [G2]，不寫）
//    D:\HT9045\config\config.ini [Server]（CC_SCC／CC_SCK 是 [RMS]）Product Name／Product Temp（SaveRmsInfo：IniConfig.bShowLotInfo
//      且 HasICUnderMachine() 才寫；W906_AUTH_PATH 可轉開 AuthPath，common.cpp:139）
//
//  共同前提（兩個入口都檢查）：bHandlerModel==false（D:\GPIB9045\system\general.ini 的 Model 不在白名單）時，移植樹的
//    ReadGeneralIni 在 database.cpp:352 就 return，ReadLastSetIni／ReadLastDataFile 都沒跑 ⇒ LastSet 是零；golden 在 FormShow
//    （main.cpp:9589-9594）就 Terminate，FormClose 不會跑。這時寫 lastdata.dat 會用零蓋掉機台的產量 ⇒ 一律不寫。
//
//  建置：本檔只編進 wb_serve（照 FileRW/MainBoot.cpp 的前例列在 CMakeLists.txt 的 add_executable(wb_serve …)；共用檔，
//    片段見交件報告）。沒列進去而 wb_serve.cpp 已接上呼叫 ⇒ 連結失敗（看得見）。不要搬進任何 archive（陷阱 #2）。
//
//  AI(W906-PROD-S121) 20260926（Steven 團隊）：RULINGS_20260926 S121（Steven 原話「是的話，要通知 c++ 完全停工，
//    馬達跟加熱都要關掉，安全第一」—— 指主畫面 Exit 鈕）＋主 session 追加（Jimmy J10：SaveMachineRecord／
//    TimerRecordLoaderDate 併進關站順序；Jimmy 22:4x：他回覆前不動停機本體，只呼叫已經有真本體的）。本檔加了三件：
//    ① W906_ProdCloseShutdown()  golden FormClose（main.cpp:11920-12474）存檔以外的每一步，照 golden 順序。
//       **只呼叫移植樹已經有真本體的**（StopAllMotor(true)＋1203 監看器的軸〔W906_Stop1203AllHook〕、IndexMotorBreakerOFF、
//       SW[加熱器繼電器／風扇／冷卻／蜂鳴器].Off()、SendCommand_ESD、EndHeaterThread／EndShuttleThread、InitialOK／bSystemClose／
//       SoftStop 旗標）；替身、空殼、沒翻的一律不呼叫、在結果裡標「未停」（stub／missing），**不假裝已停**。
//       每一筆 1203 輸出照實回報：ISSUED＝done、DRY RUN＝stub、被拒＝failed；SIM 建置的輸出只到模擬後端＝stub。
//       wb_serve 在正常關站那一點呼叫（W906_ProdCloseSave 之後、寫 Program Close=1 之前；共用檔片段見交件報告）。
//    ② 結束旗標 W906_ServeQuitRequested：Exit 第二框（"Sure To Exit?"）確認、守衛全過、生產資料存完之後設；
//       wb_serve 主迴圈底部每圈問 W906_ServeQuitDue()，**晚一圈**才 break（讓 step 2 的 ack 先送出去），
//       走既有正常關站路：W906_ProdCloseSave → W906_ProdCloseShutdown → Program Close=1 → server.Stop()。
//    ③ 存檔補兩項（Jimmy J10）：golden FormClose :11862 SaveMachineRecord()（cinitial.cpp 真本體）、:12090 TimerRecordLoaderDate()
//       （golden TfMain 成員 main.cpp:27970，移植樹沒有 —— 三行照翻在本檔；它呼叫的 MyDBITotalLoader／MyDBITimeData 有本體：
//       DB 那一半退場〔MyDBExecSQL 回 0，W906-CSVONLY＝golden bUseMDB==false〕，寫的是 EventLogTxt 與 HANDLER LOG csv）。
//    ⚠ Ctrl-C／關主控台（W906_ConsoleCtrl，另一條執行緒）**這一輪不改**：它仍然只寫 Program Close=1，不停機（見交件報告）。
// ===========================================================================
#include "cmydef.h"             // SystemStart／AccessLevel／CUSTOMER_CODE／AirStream_Select／bUpdateAutomatically／bHandlerModel／
                                // REALLY／MMSystem／TOTAL_MOTOR／MTrayX／MInArmY／MOutArmY
#include "cprod.h"              // RunInfo（RUN_INFO／JAM_COUNT）、LevelSet、WriteLastDataFile
#include "common.h"             // sDailyJamPath／MyForceDirectories
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction.bOEEFunction
#include "LastSet.h"            // LastSet.iRealDummy
#include "MachineType.h"        // CC_*／bcExit
#include "ProductionInfo/FileInfo.h"   // FileInfo().PathCombin（本體 ProductionInfo/FileInfo.cpp:457，ht9045_sm）
#include "forms/fCounterClear.h"       // fCounterClear->WriteCTInfo
#include "vclcompat/vcl_compat.h"      // AnsiString／TStringList
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "myswitch.h"                      // AI(W906-PROD-S121) 20260926: SW[]（TMySwitch::Off；Enable／ISABase／Ring／IP／Port 做回報）
#include "cSocket.h"                       // AI(W906-PROD-S121) 20260926: ArmHistory[]（TimerRecordLoaderDate，golden main.cpp:27973）
#include "forms/fMain.h"                   // AI(W906-PROD-S121) 20260927: fMain->HESDWnd（forms/fMain.h:251；St02 3ce47956 起由 HandlerBridgeCtl.cpp:124 照 golden FindWindow 設）
#include "Interface/InterfaceSYS.h"        // AI(W906-PROD-S121) 20260926: SendCommand_ESD／ESD_SYSTEM_CLOSE（golden main.cpp:11970；wb_serve.cpp:6024 同樣 include）
#include "SECSGEM/SecsEventType.h"         // AI(W906-PROD-S121) 20260926: SECS_EVENT.DoExit（golden main.cpp:11913）
#include "SECSGEM/SecsEventReport.h"       // AI(W906-PROD-S121) 20260926: EventReport(unsigned)（移植樹是 Sim 計數器，SecsEventReport.cpp:15）
#include "EtherCAT/Pci1203Control.h"       // AI(W906-PROD-S121) 20260926: Pci1203Control()：IsDryRun／log／accepted／issued／refused（回報用，不送命令）
#include "EtherCAT/Pci1203IoRoute.h"       // AI(W906-PROD-S121) 20260926: 引擎 IO→1203 路由：Installed／LastWrite／SetSource／CanWriteBit
#include "Motor/mymotor.h"                 // AI(W906-PROD-S95R) 20260926: MOT[]（TTrayMotor，.Tray＝TMyTray）—— golden :11926 SaveProductionRecord(&MOT[iMMAuto[i]].Tray, …)（FileRW/Teach.cpp 同樣 include）
#include "forms/fLotInfo.h"                // AI(W906-PROD-S95R) 20260926: fLotInfo->edDeviceName／edTemp（golden :12055 SaveRmsInfo 的兩個引數；FileRW/MainRecord.cpp 同樣 include）

#include <algorithm>            // std::sort（golden cprod.cpp:1046 sort）
#include <atomic>               // AI(W906-PROD-S121) 20260926: W906_ServeQuitRequested
#include <cstdio>
#include <cstdlib>              // getenv（W906_BINCOUNT_PATH，同 csystem.cpp ReadWriteBinCountMode 的轉開規則）
#include <string>
#include <vector>

// 不 include 的標頭（理由同 FileRW/MainBoot.cpp／MainClick.cpp：csystem.h／aHotPlateSubstrate.h 會帶進兩個 TMyKitSuck〔陷阱 #3〕、
// canary_support.h 與別的標頭的預設引數會撞），只宣告要用的函式；**不帶預設引數**，呼叫處把 golden 的引數寫全。
void ReadWriteBinCountMode(bool bRead);                                         // csystem.h:294（本體 csystem.cpp，golden csystem.cpp:24655）
bool ShuttleHasIC();                                                            // csystem.h:135
bool IndexHasIC();                                                              // csystem.h:140
bool FileRW_ArmSuckHasIC(int which);                                            // FileRW/_KitSuck.cpp（0＝InArmSuck，1＝OutArmSuck；正確的那個 TMyKitSuck）
void W906_TeachScanMotorStatus(int motIndex);                                   // forms/fTeach.h（golden MOT[i].ScanMotorStatus()；1203 軸不打 golden handle）
bool W906_TeachHomeLed(int motIndex);                                           // forms/fTeach.h（golden MOT[i].Led[iHomeLed]；1203 軸讀監看器 ORG，不明＝不在原點）
int  Barcode_Reader(int Barcode);                                               // BarcodeReader.h:111（本體 BarcodeReader.cpp:445，golden :415-444）
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr, AnsiString errPart);   // canary_support.h:66（golden note.h:466）
bool struct_cmp_by_count(JAM_COUNT a, JAM_COUNT b);                             // cprod.cpp:1065（golden cprod.cpp:990）
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } } // JsonBridge/FormJson.cpp
// AI(W906-PROD-S121) 20260926: golden FormClose 關站段要呼叫的真本體（不 include 各自的標頭，理由同上；不帶預設引數）
void StopAllMotor(bool bIndexCanStop);                                          // Motor/myGALILmotor.h:78（本體 Motor/myGALILmotor.cpp:5759，golden Motor/myGALILmotor.cpp:4712）
extern void (*W906_Stop1203AllHook)(const char* why);                           // csystem.h:420（定義 csystem.cpp:30039；wb_serve 由 WebMotorAccessLive.cpp W906_MotorAccessEngineHooks 註冊）
void IndexMotorBreakerOFF();                                                    // csystem.h:143（本體 csystem.cpp:19144，golden csystem.cpp:1086）
void EndHeaterThread();                                                         // uHeaterThread.h:76（本體 uHeaterThread.cpp:446，golden uHeaterThread.cpp:88）
void EndShuttleThread();                                                        // acarry.h:48（本體 acarry.cpp:7283）
void HeaterLog(AnsiString Message, bool bOnOff);                                // cpublic.h:43（本體 cpublic.cpp:695）
void SaveMachineRecord(bool bSpare);                                            // cinitial.h:246（本體 cinitial.cpp:17017，golden cinitial.cpp:7790）
void __fastcall MyDBITotalLoader(int iLoader);                                  // cMyDB.h:91（本體 cMyDB.cpp:335）
int  __fastcall MyDBITimeData(long StartTime, long HomeTime, long ContactTest, long PauseTime, long ProductTime, long JamTime, long PowerOn);   // cMyDB.h:92（本體 cMyDB.cpp:388）
extern bool bGali_CardInstall;                                                  // Motor/myGALILmotor.h:80（定義 Motor/myGALILmotor.cpp:736）
// AI(W906-PROD-S95R) 20260926: 同上（不 include 各自的標頭，不帶預設引數）
bool HasICUnderMachine();                                                       // csystem.h:105（本體 csystem.cpp:13304，golden csystem.cpp:13226，逐字同）
void SaveProductionRecord(TMyTray *TrayData, AnsiString Name);                  // SortingBinTray/SortingBinTray.h:94（本體 SortingBinTray.cpp:2699，golden SortingBinTray.cpp:2425；同 csystem.cpp:13087 的宣告）
bool W906_TfMain_SaveRunMode();                                                 // 本檔下面 S95R 段（golden TfMain::SaveRunMode，main.cpp:33648）

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// ---------------------------------------------------------------------------
//  [W906] 本行程裡「關程式的 SaveJamRateByDay(false) 已經跑過」。
//  golden 的 FormClose 一個行程只跑一次；移植樹有兩個入口（Exit 鈕、--seconds 關站），而且 Exit 之後程式沒有真的結束。
//  SaveJamRateByDay 的最後一行是 InitialDailyData()（清掉 iDailyCount／vDailyJam）——第二次再存，會用清空後的記憶體
//  整份覆寫當天的 DailyJamRate 檔（例：早上 12 筆 Jam，按 Exit 存了一次、沒關程式，下午 --seconds 到期再存 ⇒ 檔案只剩 0 筆）。
//  所以第二次起不再存（其餘兩個存檔照寫：它們寫的是目前記憶體，重寫無害）。要不要這樣做見交件報告「待 Steven 決定」。
// ---------------------------------------------------------------------------
bool s_bJamRateSavedOnClose = false;

// [W906] Exit 鈕的兩個確認框在網頁上分成三步（golden 是同一個呼叫裡的兩個模態框）。伺服器記住現在問到哪一框，
//   不接受跳步（不信任前端）：0＝沒有在問；1＝問過 "Sure close??"（等 step 1）；2＝問過 "Sure To Exit?"（等 step 2）。
int  s_iCloseStage = 0;

// AI(W906-PROD-S121) 20260926: Exit 鈕這條路已經把 golden FormClose 的存檔全部做完（step 1：lastdata＋SaveMachineRecord；
//   step 2：JamRate＋TimerRecordLoaderDate＋WriteCTInfo），然後才要求結束。主迴圈晚一圈 break 之後 wb_serve 還會叫
//   W906_ProdCloseSave —— golden 的 FormClose 一個行程只跑一次，所以那一次不再重存（見 W906_ProdCloseSave）。
//   AI(W906-PROD-S95R) 20260926: step 1 另加 RunMode.txt＋LotSummary.csv（golden :11863／:11864），同樣不重存。
//     SaveProductionRecord（:11924）／SaveRmsInfo（:12051）在關站段 ShutdownSequence(true)，那一段一個行程只跑一次（W906_ProdCloseShutdown）。
bool s_bExitSaved = false;
bool s_bTailRan   = false;                                                      // W906_ProdCloseSave_Tail 跑過（:12090／:12195 做了）—— 關站清單照實標
bool s_bQuitSeen  = false;                                                      // W906_ServeQuitDue：要求之後的第一次詢問＝「這一圈」，不 break

struct Msg { std::string s1, s2, s3; };

// ---------------------------------------------------------------------------
//  golden cprod.cpp:995-1110  void RUN_INFO::SaveJamRateByDay(bool bUpload)（this = &RunInfo）
//  ⚠ 為什麼不直接呼叫 RunInfo.SaveJamRateByDay(false)：移植樹 cprod.cpp:1070 那一支的本體（InitialDailyData 之前的全部）還在
//    `#if 0 // TODO(GA1-B2)`（理由「FileInfo 沒翻」已過期 —— ProductionInfo/FileInfo.cpp:457 PathCombin 已翻），呼叫它只會
//    清記憶體、不寫檔。cprod.cpp 在 ht9045_globals、FileInfo.cpp 在 ht9045_sm，直接解閘會讓只連 globals 的測試連結失敗
//    （FileRW/MainBoot.cpp S91 讀檔那一支同一個理由、同一個做法）。本檔只編進 wb_serve，所以在這裡照 golden 逐行寫一份。
//  跟 golden 不同的兩處（C++ 語法，不是改行為）：
//    ① sprintf 的 AnsiString 引數加 .c_str()（BCB6 AnsiString 可以直接穿過 `...`，C++ 不行）。
//    ② sort 用 std::sort ＋ golden 自己的 struct_cmp_by_count（cprod.cpp:1065）。次數相同的兩筆誰先，golden（BCB6 STL）與
//       MinGW 的 introsort 都沒有保證 —— 同次數的列順序可能跟 BCB6 版寫出來的不同，數字不會不同。
//  golden 的怪處（照翻，不修）：
//    "Jam counter" 加總的是 vByLotJam（這一批），不是 vDailyJam（今天）；"Unloading counter" 的 iDailyCount 全 golden 只有
//    ReadJamRateByDay 讀檔時設（沒有任何地方累加），所以它永遠是開機讀回來的值。
// ---------------------------------------------------------------------------
AnsiString W906_RunInfo_SaveJamRateByDay(bool bUpload)                         // [W906] 回傳寫的檔名（golden 是 void）
{
    AnsiString str, sFileName, sPath;
    int iJamCount=0, iTag;

    TStringList *slReport=new TStringList();

    MyForceDirectories(sDailyJamPath);

    sFileName.sprintf("%s_%s_%s_DailyJamRate.txt", IniConfig.sMachineType.c_str(), IniConfig.SocketHandlerID.c_str(), RunInfo.sToday.c_str());
    RunInfo.DailyJamFileName=FileInfo().PathCombin(sDailyJamPath, sFileName);   //Steven 20250812 : 修正上傳檔名

    str.sprintf("Date: %s", RunInfo.sToday.c_str());
    slReport->Add(str);

    str.sprintf("Unloading counter: %d", RunInfo.iDailyCount);
    slReport->Add(str);

    iJamCount=0;
    for(RunInfo.vJamIter=RunInfo.vByLotJam.begin(); RunInfo.vJamIter!=RunInfo.vByLotJam.end(); RunInfo.vJamIter++)
    {
        iJamCount+=RunInfo.vJamIter->second.iCount;
    }

    str.sprintf("Jam counter: %d", iJamCount);
    slReport->Add(str);

    if(iJamCount==0)
    {
        str.sprintf("MUBJ: 0/%d", RunInfo.iDailyCount);
    }
    else
    {
        if((RunInfo.iDailyCount/iJamCount)<1)
            str.sprintf("MUBJ: 1/1");
        else
            str.sprintf("MUBJ: 1/%d", RunInfo.iDailyCount/iJamCount);
    }
    slReport->Add(str);

    iTag=0;
    std::vector<JAM_COUNT> vec;
    for(RunInfo.vJamIter=RunInfo.vDailyJam.begin(); RunInfo.vJamIter!=RunInfo.vDailyJam.end(); RunInfo.vJamIter++)
    {
        JAM_COUNT Temp;
        Temp.JamCode=RunInfo.vJamIter->second.JamCode;
        Temp.Message=RunInfo.vJamIter->second.Message;
        Temp.iCount =RunInfo.vJamIter->second.iCount;
        vec.push_back(Temp);
    }

    std::sort(vec.begin(), vec.end(), struct_cmp_by_count);
    for(unsigned int i=0; i<vec.size(); i++)
    {
        iTag++;
        str.sprintf("%d %s %d %s", iTag, vec[i].JamCode.c_str(), vec[i].iCount, vec[i].Message.c_str());
        slReport->Add(str);
    }

    slReport->SaveToFile(RunInfo.DailyJamFileName);
    slReport->Clear();
    vec.clear();
    delete slReport;

    if(bUpload==true &&
       IniConfig.bN10_DailyUploadProdData==true)                                //Steven 20250527 : 上傳Jam Rate
    {
        // golden :1062-1106 上傳（FTP 或 XCOPY 到 N10 路徑）。本檔唯一的呼叫點是 FormClose :11919 的 (false) ⇒ 走不到。
        //   原文留在 #if 0：FormHS->UpDataToServerByFTP（TFormHS 在移植樹是 Automation/SCK_ART_Remainder.h 的替身）、
        //   slExe、ExecZipCommand 這幾個相依沒有查證過；哪天有 (true) 的呼叫者再逐一接。
#if 0 // GATE (S95-J1) golden cprod.cpp:1062-1106 -- 本檔沒有 bUpload==true 的呼叫者
        if(FileExists(DailyJamFileName)==true)
        {
            if(IniConfig.iN10UploadMethod==0)
            {
                FormHS->UpDataToServerByFTP(ExtractFilePath(DailyJamFileName), ExtractFileName(DailyJamFileName), "JamRateDaily");
            }
            else
            {
                sPath.sprintf("%sJamRateByDay\\%04d", IncludeTrailingPathDelimiter(IniConfig.sN10UploadDrivePath), SystemYear);
                if(MyForceDirectories(sPath, "[N10] Upload_JamRateByDay_Log")!=1)
                {
                    ;
                }
                else
                {
                    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", DailyJamFileName, sPath);
                    slExe->Add(str);

                    try
                    {
                        slExe->SaveToFile("D:\\HT9045_Log\\JamRateByDay.bat");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC");
                    }

                    for(int i=0; i<slExe->Count; i++)
                    {
                        RecordProcess(slExe->Strings[i]);
                    }

                    try
                    {
                        ExecZipCommand("D:\\HT9045_Log\\JamRateByDay.bat", " ");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC ExecZipCommand");
                    }

                    slExe->Clear();
                }
            }
        }
#endif // GATE (S95-J1)
    }

    const AnsiString sWritten=RunInfo.DailyJamFileName;                         // [W906] 回報用：下一行 InitialDailyData() 會把它清成 ""
    RunInfo.InitialDailyData();
    (void)sPath;
    return sWritten;
}

// golden TfMain::Check_AllMOT_Home（main.cpp:15650-15663），逐行。
//   MOT[i].ScanMotorStatus()／Led[iHomeLed] 經 forms/fTeach.cpp 的兩支轉接：非 1203 軸就是 golden 原本那兩行；
//   1203 軸（EastSun 監看器開的）不打 golden handle（打了會回 WAR16121＋舊值），改讀監看器的 ORG 位元，狀態不明一律當「不在原點」。
bool W906_CheckAllMOTHome()
{
    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        W906_TeachScanMotorStatus(i);                                           // golden :15654 MOT[i].ScanMotorStatus();
        if(W906_TeachHomeLed(i)==false)                                         // golden :15655 MOT[i].Led[iHomeLed]==false  //不在home
        {
            if(i==MTrayX || i==MInArmY || i==MOutArmY)
                continue;
            return false;
        }
    }
    return true;
}

// ---- JSON ------------------------------------------------------------------
void WriteMessages(webbridge::JsonWriter& w, const std::vector<Msg>& msgs)
{
    w.Key("messages").BeginArray();
    for (std::size_t i = 0; i < msgs.size(); ++i) {
        w.BeginObject();
        w.Key("s1").String(msgs[i].s1);
        w.Key("s2").String(msgs[i].s2);
        w.Key("s3").String(msgs[i].s3);
        w.EndObject();
    }
    w.EndArray();
}

void WriteList(webbridge::JsonWriter& w, const char* key, const std::vector<std::string>& v)
{
    w.Key(key).BeginArray();
    for (std::size_t i = 0; i < v.size(); ++i) w.String(v[i]);
    w.EndArray();
}

std::string Refuse(int step, const char* guard, const char* goldenLine, const std::string& detail,
                   const std::vector<Msg>& msgs, const std::vector<std::string>& writes)
{
    s_iCloseStage = 0;                                                          // 擋下＝golden 的 return：兩個框都沒開著
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("step").Number((wb_int64)step);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(goldenLine);
    w.Key("detail").String(detail);
    WriteMessages(w, msgs);
    WriteList(w, "writes", writes);                                             // 擋下之前 golden 已經寫了的（例 step 1 的 lastdata.dat）
    w.EndObject();
    std::printf("main.closeProgram step=%d -> refused (%s)\n", step, guard);
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

// golden sbCloseProgramClick :29053-29092 的守衛。每一步都重跑（不信任前端；兩步之間機台狀態可能變了）。
//   回 true＝通過；false＝*guard／*golden／*detail／msgs 填好了。
bool CloseGuards(const char** guard, const char** golden, std::string* detail, std::vector<Msg>* msgs)
{
    if(SystemStart==true)                                                       // golden :29054-29055（無聲 return）
    {
        *guard = "SystemStart"; *golden = "V912 main.cpp:29054-29055";
        *detail = "機台運轉中，golden 的 Exit 鈕按了沒有反應";
        return false;
    }

    if(Barcode_Reader(bcExit)==0)                                               // golden :29057-29060 // 20140103 wei KYEC Barcode Reader
    {
        *guard = "barcode-reader"; *golden = "V912 main.cpp:29057-29060";
        *detail = "KYEC 條碼登入（Barcode_Reader(bcExit)）沒有輸入操作員 ID；移植樹的輸入框是離線空殼，這一步一律擋下";
        return false;
    }

    if(AccessLevel<LevelSet.AccessLevel[6])                                     // golden :29062
    {
        if(W906_CheckAllMOTHome()==false)                                       // golden :29064
        {
            Msg m; m.s1 = "Must finish home process before terminal"; m.s2 = "請在關閉程式前,執行歸零步驟"; m.s3 = "Program Close";
            msgs->push_back(m);                                                 // golden :29066 ShowMyMessage(...)（網頁：回在 messages，頁面顯示）
            *guard = "not-home"; *golden = "V912 main.cpp:29062-29068";
            *detail = "權限低於 LevelSet.AccessLevel[6] 時，所有馬達（MTrayX／MInArmY／MOutArmY 除外）都要在原點";
            return false;
        }
    }

//    if(CUSTOMER_CODE==CC_KYEC_LEE ||                                          // golden :29076-29079 自己註解掉 ⇒ 所有客戶都檢查
//       CUSTOMER_CODE==CC_AMKOR_Philippines)
//    {
    if(LastSet.iRealDummy==REALLY)                                              // golden :29080
    {
        if(FileRW_ArmSuckHasIC(0)   ||                                          // golden :29082 InArmSuck.HasIC()
           FileRW_ArmSuckHasIC(1)   ||                                          // golden :29083 OutArmSuck.HasIC()
           ShuttleHasIC()           ||
           IndexHasIC()             )
        {
            ShowErrorMessage("MES1645", 0, MMSystem, false, "CloseProgramClick");   // golden :29088 //Must finish [One Cycle]
            *guard = "has-ic"; *golden = "V912 main.cpp:29080-29090";
            *detail = "實跑（REALLY）時 In／Out Arm、Shuttle 或 Index 上還有 IC，要先 One Cycle（MES1645）";
            return false;
        }
    }
//    }
    return true;
}

void NeedConfirm(webbridge::JsonWriter& w, int step, int next, const char* p1, const char* p2, const char* goldenLine)
{
    w.Key("executed").Bool(false);
    w.Key("step").Number((wb_int64)step);
    w.Key("needConfirm").Bool(true);
    w.Key("next").Number((wb_int64)next);
    w.Key("prompt").BeginArray();
    w.String(p1);
    w.String(p2);
    w.EndArray();
    w.Key("goldenLine").String(goldenLine);
}

// Arm*.dat 的 18 個檔名（fCounterClear->WriteCTInfo → TArm::WriteFile，cSocket.cpp；名字見 cSocket.cpp:218-221）
void ListCTInfoFiles(std::vector<std::string>* writes)
{
    writes->push_back("D:\\HT9045\\system\\Arm{0,1,2}.dat、ArmHis{0,1,2}.dat、ArmByLot{0,1,2}.dat ＋ 各自 _backup.dat（fCounterClear->WriteCTInfo，golden FormClose :12195）");
}

// AI(W906-PROD-S95R) 20260926: golden FormClose :11863／:11864 的 writes 清單文字（Exit step 1 與 W906_ProdCloseSave 共用）。
std::string RunModeWriteText(bool bOpened)
{
    char b[64];
    std::snprintf(b, sizeof(b), "RunMode=%d", LastSet.iRunStartMode);
    return std::string("D:\\HT9045\\system\\RunMode.txt 「") + b + "」（SaveRunMode()，golden FormClose :11863；整份覆寫）"
           + (bOpened ? "" : " —— ⚠ fopen 失敗，沒有寫（golden 同樣靜靜略過）");
}
std::string LotSummaryWriteText()
{
    return "D:\\HT9045\\System\\LotSummary.csv（LotSummary.WriteFile()，golden FormClose :11864；32 列 site × 256 欄＋1 列總計，整份覆寫。"
           "開不了檔時 vclcompat TStringList::SaveToFile 靜靜 return〔TStringList.cpp:262-264〕，這裡看不出有沒有寫成）";
}

// ---------------------------------------------------------------------------
//  AI(W906-PROD-S121) 20260926: golden TfMain::TimerRecordLoaderDate（V912 main.cpp:27970-27980），逐行。
//  移植樹沒有這個成員（forms/fMain.h 門面沒有；cMyDB.cpp:676、cObserver.cpp:3766 都因此閘住）。本體只有三個呼叫，
//  三個都有真本體（TArm::GetTotalCT cSocket.cpp、MyDBITotalLoader cMyDB.cpp:330、MyDBITimeData cMyDB.cpp:378），所以照翻在這裡；
//  golden 是 TfMain 成員，這裡是自由函式。ArmHistory[0..1] 在 cSocket.cpp:219 的靜態初始化就 new（golden main.cpp:2144），不會是 NULL。
//  會寫的檔（以 29554f87 合進來的 cMyDB.cpp／LogObjects.cpp 為準）：
//    D:\HT9045_Log\EventLogTxt\…（slEventLog：LogObjects.cpp W906_CreateLogObjects 建；NULL 時兩個函式都跳過這一段）＋
//    D:\HT9045_Log\SaveEventLog\HANDLER LOG_<SocketHandlerID>_YYYY_MM_DD.csv（SaveEventLogInfo("220000000", …, 22)，各一行 PROCESS）。
//    DB 那一半：MyDBExecSQL 在 wb_serve 恆回 0（sqlite 退場，AI(W906-CSVONLY)；golden bUseMDB==false 的那一支）⇒ rowID 恆 0。
//  ⚠ W906_DestroyLogObjects 在 wb_serve 的 server.Stop() 之後才跑（tools/wb_serve.cpp:5968），所以關站存檔時 slEventLog 還活著。
// ---------------------------------------------------------------------------
int W906_Main_TimerRecordLoaderDate()                                           //Steven 20140815 : Void --> int
{
    int iCount=0;
    iCount=ArmHistory[0]->GetTotalCT()+ArmHistory[1]->GetTotalCT();
    MyDBITotalLoader(iCount);
    int rowID=MyDBITimeData(LastSet.SystemAccSecond[0][stStartTime]/1000, LastSet.SystemAccSecond[0][stHomeTime]/1000, LastSet.SystemAccSecond[0][stContactTest]/1000,
                            LastSet.SystemAccSecond[0][stPauseTime]/1000, LastSet.SystemAccSecond[0][stProductTime]/1000, LastSet.SystemAccSecond[0][stJamTime]/1000,
                            LastSet.SystemAccSecond[0][stPowerOn]/1000);

    return rowID;
}

// ===========================================================================
//  AI(W906-PROD-S121) 20260926: golden FormClose 關站段的回報結構。
//    status：done＝做了而且確定到了機台（1203 ISSUED ret=0）或本身就是記憶體旗標；
//            noop＝照 golden 呼叫了，但這台／這個建置上沒有它要作用的東西（golden 同樣不做事）；
//            stub＝移植樹的本體是替身、空殼、只到模擬後端或 DRY RUN —— golden 會做的事**這裡沒有做到**；
//            missing＝移植樹沒有翻，或這一輪不接 —— 沒有呼叫；
//            failed＝送了，但被 1203 路由／命令面拒絕，或廠商回錯；
//            unverified＝呼叫了，但從這裡看不出有沒有到機台。
//    cls：motor 停馬達／heat 關加熱／output 關輸出／save 存檔／comm 通訊離線／ui 畫面與記憶體。
//    label：motor/heat/output 類只有 done 叫「已停」、noop 叫「這台沒有」，其餘一律「未停」（主 session 追加第 2 條）。
// ===========================================================================
struct SdItem { std::string golden, cls, what, status, detail; };

bool IsHwCls(const std::string& cls) { return cls == "motor" || cls == "heat" || cls == "output"; }
const char* SdLabel(const SdItem& it)
{
    if (it.status == "done") return IsHwCls(it.cls) ? "已停" : "已做";
    if (it.status == "noop") return "這台沒有";
    return IsHwCls(it.cls) ? "未停" : "未做";
}

void SdAdd(std::vector<SdItem>* v, const char* golden, const char* cls, const std::string& what, const char* status, const std::string& detail)
{
    SdItem it; it.golden = std::string("V912 main.cpp:") + golden; it.cls = cls; it.what = what; it.status = status; it.detail = detail;
    v->push_back(it);
}

// 1203 命令面／路由的快照（只讀計數，不送命令）。Pci1203Control() 在 wb_serve 關站時 server.Stop() 之後才 Disable，這裡還活著。
struct IoSnap { bool route; unsigned long seq; bool ctl; bool dry; unsigned long acc, ref, iss; };
IoSnap TakeSnap()
{
    IoSnap s;
    s.route = ht9045::Pci1203RouteInstalled();
    s.seq   = ht9045::Pci1203RouteLastWrite().seq;
    ht9045::TPci1203Control* c = ht9045::Pci1203Control();
    s.ctl = (c != 0);
    s.dry = c ? c->IsDryRun() : true;
    s.acc = c ? c->acceptedCount() : 0;
    s.ref = c ? c->refusedCount()  : 0;
    s.iss = c ? c->issuedCount()   : 0;
    return s;
}

// 兩個快照之間：路由寫了幾筆、命令面 issued／dry／refused 幾筆、廠商回錯幾筆（Execute 每一筆恰好一行 log：ISSUED／DRY／REFUSED）。
struct IoDelta { unsigned long routeWrites, issued, dry, refused; int vendorErr; std::vector<std::string> lines; };
IoDelta TakeDelta(const IoSnap& a, const IoSnap& b)
{
    IoDelta d;
    d.routeWrites = b.seq - a.seq;
    d.issued  = b.iss - a.iss;
    d.dry     = (b.acc - a.acc) >= d.issued ? (b.acc - a.acc) - d.issued : 0;
    d.refused = b.ref - a.ref;
    d.vendorErr = 0;
    ht9045::TPci1203Control* c = ht9045::Pci1203Control();
    if (c) {
        const std::vector<std::string>& L = c->log();
        std::size_t k = (std::size_t)((b.acc - a.acc) + d.refused);
        if (k > L.size()) k = L.size();
        for (std::size_t i = L.size() - k; i < L.size(); ++i) {
            d.lines.push_back(L[i]);
            if (L[i].compare(0, 6, "ISSUED") == 0 && L[i].find("-> 0x00000000") == std::string::npos) ++d.vendorErr;
        }
    }
    return d;
}

std::string DeltaText(const IoDelta& d)
{
    char b[200];
    std::snprintf(b, sizeof(b), "1203 路由寫入 %lu 筆；命令面 ISSUED %lu／DRY %lu／REFUSED %lu（廠商回錯 %d）",
                  d.routeWrites, d.issued, d.dry, d.refused, d.vendorErr);
    std::string s(b);
    for (std::size_t i = 0; i < d.lines.size() && i < 12; ++i) s += "｜" + d.lines[i];
    if (d.lines.size() > 12) s += "｜…";
    return s;
}

const char* kSimIoText =
    "SIM 建置（SOFT_SIMULTE）或引擎 IO 沒接 1203（WB_ENGINE_IO_1203）：MyLaneIO 是模擬後端，這一步的輸出只寫到模擬值，沒有到卡。"
    "⚠ 網頁 IO 頁（pci1203.do.setBit）直接打到卡的輸出不會被這裡關掉";

// 一個 SW[].Off()。execute=false 只預估（不寫任何東西）。來源設成 "shutdown"：路由不略過「卡片已經是這個值」的寫入
//   （IOWEB-P25 S7 那個優化只給 engine），每一筆都真的送、而且記在 Pci1203RouteLastWrite —— 關站寧可重送。
SdItem SwOffItem(int sw, const char* swName, const char* golden, const char* cls, bool execute)
{
    SdItem it;
    it.golden = std::string("V912 main.cpp:") + golden;
    it.cls = cls;
    it.what = std::string("SW[") + swName + "].Off()";
    TMySwitch& s = SW[sw];

    IoSnap a = TakeSnap(), b = a;
    if (execute) {
        ht9045::Pci1203RouteSetSource("shutdown");
        s.Off();                                                                // golden 原句
        ht9045::Pci1203RouteSetSource("engine");
        b = TakeSnap();
    }

    char buf[160];
    if (!s.Enable) {
        it.status = "noop";
        it.detail = "IO_Table 沒有這一點（Enable=0）：golden 的 Off() 在 Enable==false 直接 return（只清 OutValue）";
        return it;
    }
    std::snprintf(buf, sizeof(buf), "（Lane %d、IP %d、Port %d、Type %d）", s.Ring, s.IP, s.Port, s.Type);
    if (s.ISABase != ePCI1203) {
        it.status = "unverified";
        it.detail = std::string("非 1203 點（ISABase=") + std::to_string(s.ISABase) + "）" + buf +
                    "：MyLaneIO 的 MN200／MNet／ISA 後端有沒有真的寫到卡，這裡沒有查證";
        return it;
    }
    if (!a.route) { it.status = "stub"; it.detail = std::string(kSimIoText) + buf; return it; }

    if (!execute) {
        int slot = -1, chan = -1; std::string why;
        if (!a.ctl)                                                       { it.status = "failed"; it.detail = std::string("1203 命令面沒有武裝：路由會拒絕，這一點不會被關") + buf; }
        else if (!ht9045::Pci1203RouteCanWriteBit(s.Ring, s.IP, s.Port, &slot, &chan, why)) { it.status = "failed"; it.detail = "路由會拒絕：" + why + buf; }
        else if (a.dry)                                                   { it.status = "stub";   it.detail = std::string("1203 命令面是 DRY RUN：會記錄、不會送到卡") + buf; }
        else                                                              { it.status = "done";   it.detail = std::string("（預估）會送 Acm_DaqDoSetBitEx 到卡") + buf; }
        return it;
    }

    if (b.seq == a.seq) {
        it.status = "failed";
        it.detail = std::string("MyLaneIO 在後端之前就 return（位址全 0、CheckPortRangeErr 範圍錯誤，或安全門互鎖 IdleCheckSafeDoorByCylinder）——沒有送出") + buf;
        return it;
    }
    const ht9045::Pci1203RouteWrite& w = ht9045::Pci1203RouteLastWrite();
    if (!w.reached || !w.accepted) { it.status = "failed"; it.detail = "被拒：" + w.why + buf; }
    else if (w.issued && w.ret == 0) { it.status = "done"; it.detail = w.exCall + " ISSUED" + buf; }
    else if (w.issued) { std::snprintf(buf, sizeof(buf), " 廠商回 0x%08lX", w.ret); it.status = "failed"; it.detail = w.exCall + buf; }
    else { it.status = "stub"; it.detail = "DRY RUN（沒有送到卡）：" + w.why; }
    return it;
}

// 一個會寫好幾個輸出的 golden 函式（IndexMotorBreakerOFF／StopAllMotor 的 IO 部分），用兩個快照的差判斷。
void ClassifyIoGroup(SdItem* it, const IoSnap& a, const IoSnap& b, bool execute, const std::string& what)
{
    if (!a.route) { it->status = "stub"; it->detail = std::string(kSimIoText) + "。" + what; return; }
    if (!a.ctl)   { it->status = "failed"; it->detail = "1203 命令面沒有武裝：路由拒絕每一筆輸出。" + what; return; }
    if (!execute) {
        it->status = a.dry ? "stub" : "done";
        it->detail = std::string(a.dry ? "1203 命令面是 DRY RUN：會記錄、不會送到卡。" : "（預估）會經路由送到卡。") + what;
        return;
    }
    const IoDelta d = TakeDelta(a, b);
    const unsigned long exec = d.issued + d.dry + d.refused;
    const unsigned long pre  = d.routeWrites > exec ? d.routeWrites - exec : 0;   // 路由自己在 Execute 之前拒絕的（CheckWrite_）
    std::string s = DeltaText(d);
    if (pre > 0) s += "｜路由在命令面之前拒絕 " + std::to_string(pre) + " 筆（最後一筆：" + ht9045::Pci1203RouteLastWrite().why + "）";
    if (d.refused > 0 || d.vendorErr > 0 || pre > 0) it->status = "failed";
    else if (d.issued > 0)                            it->status = "done";
    else if (d.dry > 0)                               it->status = "stub";
    else                                              it->status = "unverified";   // 沒有任何一筆經過路由：這些點都沒 Enable，或都在後端之前就 return
    it->detail = s + "。" + what;
}

// golden StopAllMotor() 之後的 1203 那一半（移植樹專用；golden 的 MOT[] handle 碰不到監看器開的軸 —— 同 csystem.cpp:30176 VerifyMotorAction）。
SdItem Stop1203Item(bool execute)
{
    SdItem it;
    it.golden = "V912 main.cpp:12153（StopAllMotor 的 1203 那一半；移植樹專用，同 csystem.cpp:30176）";
    it.cls = "motor";
    it.what = "W906_Stop1203AllHook：每一個 1203 監看器開成功的軸 Acm_AxStopDec＋Acm_AxSetExtDrive(0)";
    const IoSnap a = TakeSnap();
    if (W906_Stop1203AllHook == 0) {
        it.status = "stub";
        it.detail = "motor.access 引擎掛鉤沒有註冊（WebMotorAccessLive.cpp W906_MotorAccessEngineHooks 沒跑）：1203 軸沒有收到停止命令";
        return it;
    }
    if (!a.ctl) {
        it.status = "stub";
        it.detail = "1203 命令面沒有武裝（沒有 SDK、卡沒開，或 Pci1203ControlEnable 失敗）：沒有停止命令可送 —— 這台若有 1203 軸在動，這裡停不了";
        return it;
    }
    if (!execute) {
        it.status = a.dry ? "stub" : "done";
        it.detail = a.dry ? "1203 命令面是 DRY RUN：停止命令只記錄、不送到卡" : "（預估）對每一個開成功的軸送減速停止";
        return it;
    }
    W906_Stop1203AllHook("MainClose: golden FormClose StopAllMotor (V912 main.cpp:12153)");
    const IoSnap b = TakeSnap();
    const IoDelta d = TakeDelta(a, b);
    if (d.refused > 0 || d.vendorErr > 0) it.status = "failed";
    else if (d.issued > 0)                it.status = "done";
    else if (d.dry > 0)                   it.status = "stub";
    else                                  it.status = "noop";                    // 監看器沒有開成功的軸
    it.detail = DeltaText(d) + (it.status == "noop" ? "。監看器沒有開成功的軸：沒有 1203 軸要停" : "");
    return it;
}

// ---------------------------------------------------------------------------
//  golden TfMain::FormClose（V912 main.cpp:11852-12478）存檔以外的每一步，照 golden 順序。
//  execute=false：只預估（step 2 的回應用，不寫任何東西）；true：照做（wb_serve 關站那一點）。
//  存檔（:11861 WriteLastDataFile、:11862 SaveMachineRecord、:11863 SaveRunMode、:11864 LotSummary.WriteFile、
//  :11919 SaveJamRateByDay、:12090 TimerRecordLoaderDate、:12195 WriteCTInfo）在 W906_ProdCloseSave／Exit 鈕的 step 1、2，不在這裡。
//  AI(W906-PROD-S95R) 20260926: 例外兩個存檔在這裡、照 golden 的位置：:11924-11927 SaveProductionRecord、:12051-12057 SaveRmsInfo
//    （S121 時兩個都是 missing）。execute=false 只預估、不寫；true 才寫。bHandlerModel==false 時不寫（跟上面那些存檔同一個前提：
//    golden 在那時 FormShow 就 Terminate，FormClose 不會跑）——停機項目照做，存檔項目標 missing。
//    跟 golden 順序不同的一處：golden 是 :11919 JamRate → :11924 SaveProductionRecord → … → :12090 → :12195；移植樹的
//    :12090／:12195 在存檔段（W906_ProdCloseSave_Tail），比這裡的 :11924 早跑。寫的是不同的檔，互不讀取，先後不影響內容。
// ---------------------------------------------------------------------------
std::vector<SdItem> ShutdownSequence(bool execute)
{
    std::vector<SdItem> v;
    char buf[200];

    // ---- sbCloseProgramClick :29097-29105（Exit 鈕 step 0；--seconds 關站沒有這一步）--------------------------------
    SdAdd(&v, "29097-29105", "heat", "AirStream（HT-1032 TriTemp）：\"Sure close Air Stream??\" → bCloseAirMachine=true＋SendAirMachineStatus(0, …)",
          AirStream_Select==1 ? "missing" : "noop",
          AirStream_Select==1 ? "沒有接（S95 step 0 的 gaps）⇒ Air Stream 沒有被關掉" : "這台沒有 Air Stream（AirStream_Select!=1）：golden 同樣不問");

    // ---- :11881-11892（在 "Sure To Exit?" 之前）---------------------------------------------------------------
    SdAdd(&v, "11881-11884", "ui", "if(bRunAutoClean) ResetAutoClean()", "missing",
          "這一輪不接：本體在 AutoClean/AutoClean.cpp:3847，會跳 ShowMyMessage（請移除 Clean pad）並清吸嘴資料；不是停機動作");
    SdAdd(&v, "11886-11892", "comm", "CosFunction.bDLLCommands：UnmapViewOfFile／CloseHandle（Epson DLL 檔案對映）", "missing", "移植樹沒有 Epson DLL 檔案對映");

    // ---- :11920-11945（存檔與上傳）-----------------------------------------------------------------------------
    SdAdd(&v, "11920", "ui", "iSoftwareExeTag=-1", "missing", "計時旗標，移植樹沒有這個變數");
    SdAdd(&v, "11921", "save", "CloseFile_PLCRecvDataLog()", "missing", "PLC 接收資料 log，移植樹沒有");
    SdAdd(&v, "11922", "ui", "LogSoftwareOffTime(...)（本段每一處）", "stub", "acarry_shims.cpp:255 空殼；不呼叫");
    // AI(W906-PROD-S95R) 20260926（Steven 團隊）：golden :11924-11927 照接（S121 時是 missing）。
    //   golden 這個迴圈本身沒有條件（每個客戶都跑、eTrayCount 盤全跑）；條件都在本體裡（SortingBinTray/SortingBinTray.cpp:2699，
    //   golden SortingBinTray.cpp:2425）：
    //   ① CosFunction.bSaveProductionLogByUnloaderTray==false 就 return —— 全樹只有 FUNC_CC_Murata 設 true（CosFunction.cpp:3090）；
    //   ② 那一盤 TrayData->HasRealIC()==false 就 return；
    //   ③ 過了才 MyForceDirectories(D:\HT9045_Log\UnloadTrayLog\YYYYMM\)、組 csv 內容；csv 只有 IniConfig.bN10_9_UploadUnloadTrayToFTP
    //      （且 fLotInfo->iXMLOnLineStatus 是 0／1）時才 SaveToFile 並排進 fMain->UnloadTrayLog（FTP 上傳）——那一段在移植樹是
    //      #if 0 [G2]（門面沒有 iXMLOnLineStatus／UnloadTrayLog，SortingBinTray.cpp:2758-2780）⇒ 移植樹最多只建資料夾，不寫 csv。
    //      （SortingBinTray.cpp:2763-2766 說 bN10_9 在移植樹「沒有人設」已過期：cConfiguration.cpp:3853 會從 config.ini [FTPUpLoad] 讀。）
    {
        const bool bOn  = CosFunction.bSaveProductionLogByUnloaderTray;
        const bool bFtp = IniConfig.bN10_9_UploadUnloadTrayToFTP;
        int nReal = 0;
        for (int i = 0; i < eTrayCount; i++)                                    // 回報用：本體的條件 ②（HasRealIC 只讀 Data[][]，mytray.cpp:240）
            if (MOT[iMMAuto[i]].Tray.HasRealIC()) ++nReal;
        if (execute && bHandlerModel)
        {
            for(int i=0; i<eTrayCount; i++)                                     //Steven 20201007 : production log by unloader tray存檔, 關程式前要存一次, 避免資料遺失   // golden :11924
            {
                SaveProductionRecord(&MOT[iMMAuto[i]].Tray, s6TrayName[i]);     // golden :11926
            }
        }
        const char* pre = execute ? "" : "（預估）";
        const char* st; std::string d;
        std::snprintf(buf, sizeof(buf), "（CUSTOMER_CODE=%d；有實料的盤 %d／%d）", CUSTOMER_CODE, nReal, (int)eTrayCount);
        if (!bOn) {
            st = "noop";
            d  = std::string("這台不是 Murata（CosFunction.bSaveProductionLogByUnloaderTray=false，全樹只有 FUNC_CC_Murata 設 true）：") +
                 (execute && bHandlerModel ? "照 golden 呼叫了，" : "") + "本體第一行就 return（golden 同）" + buf;
        } else if (execute && !bHandlerModel) {
            st = "missing";
            d  = std::string("存檔段不寫（bHandlerModel=false：golden FormClose 不會跑）") + buf;
        } else if (nReal == 0) {
            st = "noop";
            d  = std::string(pre) + "沒有一盤有實料（HasRealIC 全 false）：每一盤本體都 return（golden 同）" + buf;
        } else if (!bFtp) {
            st = "done";
            d  = std::string(pre) + "有實料的盤建了 D:\\HT9045_Log\\UnloadTrayLog\\YYYYMM\\ 資料夾；bN10_9_UploadUnloadTrayToFTP=false ⇒ golden 同樣不寫 csv" + buf;
        } else {
            st = "stub";
            d  = std::string(pre) + "golden 會寫 D:\\HT9045_Log\\UnloadTrayLog\\YYYYMM\\<PC_NAME>_<LotID>_<時間>_<盤名>.csv 並排進 FTP 上傳；"
                 "移植樹那一段是 #if 0 [G2]（SortingBinTray.cpp:2767-2780）⇒ 只建了資料夾，csv 沒寫、沒上傳" + buf;
        }
        SdAdd(&v, "11924-11927", "save", "SaveProductionRecord(&MOT[iMMAuto[i]].Tray, s6TrayName[i])（每個 Unloader tray）", st, d);
    }
    SdAdd(&v, "11928-11929", "save", "PickFromHPList->SaveFile(sHPPickRec)／PlaceToCleanList->SaveFile(sCleanPlaceRec)", "missing",
          "PickHPRec 已由 SaveMachineRecord 的尾段寫（cinitial.cpp:17241，存檔段）；PlaceToCleanList 在移植樹閘住（cinitial.cpp:17242 N1-G3b）");
    SdAdd(&v, "11932", "save", "FormHS->RecordLog_HS(true)", "stub", "TFormHS 是替身（Automation/SCK_ART_Remainder.h）；不呼叫");
    SdAdd(&v, "11935-11938", "comm", "FormHS->UpDataToServer_KYEC(5)（N10 每日上傳）", "missing", "");
    SdAdd(&v, "11940-11943", "save", "fBarCode->UpdateWhite2DIDList（JCET 2D 白名單）", "missing", "");

    // ---- :11947-11953  EP 歸零、InitialOK ---------------------------------------------------------------------
    SdAdd(&v, "11947", "output", "ADAM_WriteVoltage(0)（EP 設 0，避免增壓缸一直起動）", "stub",
          "atester_shims.cpp:327 是空殼 ⇒ EP 沒有歸零；不呼叫（Jimmy：adam6024）");
    SdAdd(&v, "11948", "output", "ADAM_DirectWriteData(0, 0)", "stub", "atester_shims.cpp:326 是空殼；不呼叫");
    SdAdd(&v, "11949-11952", "output", "if(IsIndependentEPPressureRouteActive()) APAX_WriteData(true, 0)", "missing",
          "APAX 與 IsIndependentEPPressureRouteActive 都沒有翻（aTester_Front.cpp:2422 GATE）");
    if (execute) InitialOK=false;                                               // golden :11953
    SdAdd(&v, "11953", "ui", "InitialOK=false", "done",
          "記憶體旗標（之後 IdleCheckSafeDoorByCylinder 不再擋輸出，golden 同）");

    // ---- :11957-11970  ATC 計時器／socket、SECS 物件、ESD ------------------------------------------------------
    SdAdd(&v, "11957-11961", "comm", "ATC_InterfaceForm／ATCInterfaceForm 的 Timer、CommFlagTimer、ClientSocket、ATC7_ServerSocket、TimerATC 關掉", "missing",
          "這一輪不接（ATC 區同事正在改）；tick 迴圈已停，計時器不會再跑，但 socket 沒有主動關");
    SdAdd(&v, "11963-11967", "comm", "delete HSys.MyGem（SECS）", "missing", "不做：行程結束時釋放");
    {
        const bool hasEsd = (USE_NOVX3360 || USE_KASUGA || USE_KASUGA_Fan || iUseHTIonBarFunction!=0);
        if (execute)
        {
            bESDSystemtype=true;                                                //kevin 20160111 ESD 通訊驅動   // golden :11969
            SendCommand_ESD(ESD_SYSTEM_CLOSE);                                  //Frank 20150309 : 程式關閉後將ESD系統停止   // golden :11970
        }
        // AI(W906-PROD-S121) 20260927: 狀態改看當下的 fMain->HESDWnd —— Steven02 3ce47956（ESD G3）起 HandlerBridgeCtl.cpp:124
        //   照 golden 在 ProcessHVisionConnect 裡 FindWindow("TfESDMain","ESD_Monitor")；找到＝WM_COPYDATA 送出（對方收沒收到這裡看不到
        //   ⇒ unverified），找不到＝ESD 程式沒開 ⇒ _SendStructMessage_Send 直接 return（stub，未停）。
        const bool bEsdWnd = (fMain != 0 && fMain->HESDWnd != NULL);
        SdAdd(&v, "11969-11970", "output", "bESDSystemtype=true; SendCommand_ESD(ESD_SYSTEM_CLOSE)", hasEsd ? (bEsdWnd ? "unverified" : "stub") : "noop",
              !hasEsd ? "這台沒有 ESD／離子風扇（USE_NOVX3360／USE_KASUGA／USE_KASUGA_Fan／iUseHTIonBarFunction 都是 0）：golden 同樣不送"
              : bEsdWnd ? "本體 Interface/InterfaceSYS.cpp:532；已找到 ESD_Monitor 視窗（fMain->HESDWnd），停止指令用 WM_COPYDATA 送出 —— ESD 程式有沒有照做這裡看不到"
                        : "本體 Interface/InterfaceSYS.cpp:532；沒找到 ESD_Monitor 視窗（HandlerBridgeCtl.cpp:124 FindWindow 回 NULL：ESD 程式沒開）"
                          "⇒ _SendStructMessage_Send 直接 return，ESD／離子風扇系統沒有收到停止");
    }

    // ---- :11972-12035  計時器、CCD、FTP 執行緒、條碼、OCR、EJ1N、HonPrec ATC -------------------------------------
    SdAdd(&v, "11972-11973", "ui", "fLotInfo->Timer1／LotKeyInTime->Enabled=false", "noop", "網頁版沒有這兩個 VCL 計時器；tick 迴圈已停");
    SdAdd(&v, "11974-11985", "output", "REAL_TIME_CCD：COM2->DoReleaseAndInspEnd／SendCommToVision(rtLightOff)／ScanBtnThd->EndThread",
          REAL_TIME_CCD ? "missing" : "noop", REAL_TIME_CCD ? "移植樹沒有 Real Time CCD 的 COM2 物件：CCD 光源沒有關" : "這台沒有 Real Time CCD（REAL_TIME_CCD=false）");
    SdAdd(&v, "11989-11995", "comm", "FtpUploadThd->EndThread／WaitFor／delete", "missing", "移植樹沒有這條 FTP 背景上傳執行緒");
    SdAdd(&v, "11999-12001", "ui", "#ifdef DEBUG_AUTO_CLEAN btSavelog->Click()", "noop", "巨集沒開（golden MachineType.h:26 同）");
    SdAdd(&v, "12002-12008", "save", "fLotInfo->btClearBarcodeCount->Click()（條碼顆數）", "missing", "");
    SdAdd(&v, "12010-12019", "comm", "fOCR->DoOCRReleaseAndInspEnd()／DoDisConnect()", "missing", "");
    SdAdd(&v, "12021-12025", "heat", "fOmron->Timer2->Enabled=false（EJ1N 溫控輪詢）", "noop",
          "網頁版沒有這個 VCL 計時器、tick 已停；golden 這一步只停輪詢，不關加熱器（加熱器是 :12157 的繼電器）");
    {
        const bool honAtc = (ATC_SYSTEM==eATCHonPrecType);
        std::snprintf(buf, sizeof(buf), "（ATC_SYSTEM=%d）", ATC_SYSTEM);
        SdAdd(&v, "12028-12035", "heat", "ATCInterfaceForm->SetRunATC(false)／bRunATC=false／ATCChillerSwitch(false)／OffLine()（HonPrec ATC）",
              honAtc ? "missing" : "noop",
              std::string(honAtc ? "有本體（ATC/ATCInterface.cpp:1129／:1201／:536），這一輪不接：ATC 區同事正在改，而且 SetRunATC 對 ATC_SYS_PAL[0] 取值沒有空檢查 ⇒ ATC 沒有收到 STOP"
                                 : "不是 HonPrec ATC：golden 同樣不做") + buf);
    }

    // ---- :12041-12048  蜂鳴器、Index kit 吸嘴 ----------------------------------------------------------------
    v.push_back(SwOffItem(SwMusic1+0, "SwMusic1+0", "12043", "output", execute));   // golden :12041-12044  for(i<4) SW[SwMusic1+i].Off();
    v.push_back(SwOffItem(SwMusic1+1, "SwMusic1+1", "12043", "output", execute));
    v.push_back(SwOffItem(SwMusic1+2, "SwMusic1+2", "12043", "output", execute));
    v.push_back(SwOffItem(SwMusic1+3, "SwMusic1+3", "12043", "output", execute));
    SdAdd(&v, "12047-12048", "output", "CheckKitSuck.Suck[0][0]／[0][1].Normal()（Index kit 吸嘴偵測：真空／破壞關）", "missing",
          "不呼叫：CheckKitSuck 是 mykitsuck.h 那個 TMyKitSuck（陷阱 #3：兩個佈局），本檔不能 include；要一支放在正確 TU 的轉接函式");
    // AI(W906-PROD-S95R) 20260926（Steven 團隊）：golden :12050-12058 照接（S121 時是 missing「RMS 排後（S120-1）」）。plan 只預估，executed 才寫。
    //   本體 cprod.cpp:3325（golden cprod.cpp:3150，逐字同）：IniConfig.sProductName／sProductTemp＝兩個引數，WriteIniData 寫
    //   AuthPath+"config.ini" 的 [Server]（CC_SCC／CC_SCK 是 [RMS]）"Product Name"／"Product Temp"（本體裡又檢查一次 bShowLotInfo，golden 同）。
    //   bShowLotInfo 只由客戶函式設 true（CosFunction.cpp，例 FUNC_CC_SJ_Semiconductor :1737）。
    //   ⚠ 兩個引數在 wb_serve 裡沒有人填：golden 開機 TfLotInfo::ResetLotInfo（uLotInfo.cpp:14820-14834）在機台內有 IC 時把 config.ini
    //     這兩個鍵讀回 edDeviceName／edTemp —— 移植樹那一步是 GATE WC-1（forms/fLotInfo.cpp:3109-3111、:3209-3211 都是 #if 0）；
    //     網頁 Data.LotInfo 綁的是 Lot 分頁的 lbledtDeviceName（web/page/ht9045_lotinfo_wire.js:10-13），tsDeviceInfo 分頁的
    //     edDeviceName／edTemp 沒有接線。移植樹寫它們的只有 cbbDeviceNameChange（fLotInfo.cpp:1987，cbbDeviceName 也沒人填）、
    //     edDeviceNameMouseDown／KeyDown（條碼掃入）與清空（csystem.cpp:8722-8724、fLotInfo.cpp:4705）
    //     ⇒ 關站時兩個值通常是 ""，照 golden 寫下去＝把 config.ini 這兩個鍵寫成空字串。
    //   [W906] AI(W906-PROD-S95R) 20260927（主 session 整合）：WC-1 解閘之前，兩個值**都是空的**就不寫 —— golden 在同樣情況寫回的是開機
    //     從 config.ini 讀進來的值（檔案不變）；移植樹照字面寫會把 BCB6 存過的 Product Name／Product Temp 清掉。任一個有值（條碼掃入等）
    //     照 golden 寫。WC-1 解閘後拿掉這個守衛（s_bRmsBlankGuard）。見 todo ★。
    {
        const bool s_bRmsBlankGuard = false;                                    // AI(W906-FRW-WC1) 20260927: WC-1（ResetLotInfo）已解閘 ⇒ 照 golden 寫（守衛留著，要恢復改回 true）
        const AnsiString sSec  = (CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK) ? "RMS" : "Server";   // 回報用（同本體 cprod.cpp:3329）
        const AnsiString sName = fLotInfo->edDeviceName->Text;                  // 回報用：golden :12055 的兩個引數（fLotInfo 在 fLotInfo.cpp:4435 靜態初始化就 new）
        const AnsiString sTemp = fLotInfo->edTemp->Text;
        bool bHasIC = false;
        const bool bBlankSkip = s_bRmsBlankGuard && sName.IsEmpty() && sTemp.IsEmpty();
        if (execute && bHandlerModel)
        {
            //jou 2011-08-27 start : 從lotInfo Form close拿到這邊來,不然不會執行到
            if(IniConfig.bShowLotInfo)                                          //Steven 20110117   // golden :12051
            {
                if(HasICUnderMachine())                                         // golden :12053
                {
                    bHasIC = true;
                    if (!bBlankSkip)                                            // [W906] 見上面 s_bRmsBlankGuard
                    SaveRmsInfo(fLotInfo->edDeviceName->Text, fLotInfo->edTemp->Text);  //Stevn 20110527   // golden :12055
                }
            }
            //jou 2011-08-27 end
        }
        else if (IniConfig.bShowLotInfo)
        {
            bHasIC = HasICUnderMachine();                                       // plan（或 bHandlerModel=false）：只看、不寫
        }
        const char* st; std::string d;
        const std::string sFile = std::string(AuthPath.c_str()) + "config.ini [" + sSec.c_str() + "]";
        if (!IniConfig.bShowLotInfo) {
            st = "noop"; d = "IniConfig.bShowLotInfo=false（這個客戶沒有開 Lot Info）：golden 同樣不存";
        } else if (execute && !bHandlerModel) {
            st = "missing"; d = "存檔段不寫（bHandlerModel=false：golden FormClose 不會跑）";
        } else if (!bHasIC) {
            st = "noop"; d = "機台內沒有 IC（HasICUnderMachine()=false）：golden 同樣不存";
        } else if (bBlankSkip) {
            st = "missing";
            d  = std::string(execute ? "沒寫 " : "（預估）不寫 ") + sFile + " Product Name／Product Temp：門面 edDeviceName／edTemp 兩個都是空字串"
                 "（golden 開機 ResetLotInfo 讀回它們，移植樹是 GATE WC-1、網頁也沒接 tsDeviceInfo）；照字面寫會把 config.ini 既有的值清空，"
                 "golden 在同樣情況寫回的是開機讀進來的值 ⇒ [W906] WC-1 解閘前不寫（檔案保持原值）";
        } else {
            st = "done";
            d  = std::string(execute ? "寫了 " : "（預估）會寫 ") + sFile + " Product Name=\"" + sName.c_str() + "\"、Product Temp=\"" + sTemp.c_str() +
                 "\"（同時設 IniConfig.sProductName／sProductTemp）";
        }
        SdAdd(&v, "12051-12057", "save", "if(bShowLotInfo) if(HasICUnderMachine()) SaveRmsInfo(fLotInfo->edDeviceName->Text, fLotInfo->edTemp->Text)", st, d);
    }

    if (execute) bSystemClose=true;                                             // golden :12060
    SdAdd(&v, "12060", "ui", "bSystemClose=true", "done", "記憶體旗標（TesterComm HandlerGpibMsg.cpp:791 之後不回應測試機）");

    // ---- :12062-12146  BinDisplay、CCLink、TimerRecordLoaderDate、FTP ------------------------------------------
    SdAdd(&v, "12062-12086", "comm", "BinDisplay（NUMBER_PANEL_TYPE 3／4）ProcessStopStart(false)／CommBin->StopComm()",
          (NUMBER_PANEL_TYPE==3 || NUMBER_PANEL_TYPE==4) ? "missing" : "noop",
          (NUMBER_PANEL_TYPE==3 || NUMBER_PANEL_TYPE==4) ? "這一輪沒有接" : "這台不是 BinDisplay TFT（NUMBER_PANEL_TYPE 不是 3／4）");
    SdAdd(&v, "12088", "comm", "fCCLink->SetInitialOK(InitialOK)", "missing", "");
    SdAdd(&v, "12090", "save", "TimerRecordLoaderDate()", s_bTailRan ? "done" : "missing",
          s_bTailRan ? "在存檔段（W906_ProdCloseSave_Tail／Exit step 2），不在這裡；結果見那邊的 writes"
                     : "存檔段沒有跑（bHandlerModel=false：LastSet 沒讀過，不存）");
    SdAdd(&v, "12094-12146", "save", "CosFunction.bFTPFunction：_NET 工作檔 XCOPY／RD、刪 TesterMap.txt", "missing", "不做：會 XCOPY／RD 配方資料夾");

    // ---- :12149-12171  停執行緒、停馬達、煞車、加熱器繼電器、風扇 -------------------------------------------------
    SdAdd(&v, "12149", "ui", "OutShuttleLog(true)", "stub", "acarry_shims.cpp:254 空殼（真本體 cpublic.cpp:718 在 #if 0）；不呼叫");
    if (execute) EndHeaterThread();                                             // golden :12151
    SdAdd(&v, "12151", "heat", "EndHeaterThread()", "noop",
          "HeaterThread==NULL（移植樹沒有溫控輪詢執行緒）⇒ 本體第一行就 return。golden 這一步也只是停輪詢，不是關加熱器");
    if (execute) EndShuttleThread();                                            // golden :12152
    SdAdd(&v, "12152", "motor", "EndShuttleThread()", "noop", "HThreadCtrlShuttle 在移植樹沒有 OS 執行緒（CloseThread 只清欄位）＋MySleep(100)");
    {
        SdItem it; it.golden = "V912 main.cpp:12153"; it.cls = "motor";
        it.what = "StopAllMotor()（golden 物件那一半：MOT[i].Motor 逐軸 PCIL132_StopMotor、TrayMoveOut(false)、Auto CW／CCW、震動輸出 Off）";
        const IoSnap a = TakeSnap();
        if (execute) {
            ht9045::Pci1203RouteSetSource("shutdown");
            StopAllMotor(true);                                                 // golden :12153 StopAllMotor();（預設參數 true）
            ht9045::Pci1203RouteSetSource("engine");
        }
        const IoSnap b = execute ? TakeSnap() : a;
        ClassifyIoGroup(&it, a, b, execute,
                        "馬達部分：SIM 建置 golden 把每一軸的 Motor->Enable 設 false（cinitial.cpp:4011-4034）⇒ 不下命令；"
                        "HT9050 的 1203 軸是監看器開的、golden handle 沒開（TMyEtherCatMotor::DecStop 在 bAxisOpen==false 直接 return）"
                        "⇒ 1203 軸靠下一項；非 1203 的馬達卡這裡沒有查證。⚠ golden：SystemStart 還是 true 時，PServoAlarmOn==0 的步進馬達不停");
        if (it.status == "done") it.status = "unverified";                      // 輸出有送到卡，但馬達那一部分看不到 —— 不說「已停」
        v.push_back(it);
    }
    v.push_back(Stop1203Item(execute));
    SdAdd(&v, "12154-12155", "ui", "if(iControlPanelMode==1) MyPad232Thread->Suspend()",
          iControlPanelMode==1 ? "missing" : "noop", iControlPanelMode==1 ? "uPadInterface 沒有翻（W3 子項 4b）" : "不是 RS232 實體面板（iControlPanelMode!=1）");
    {
        SdItem it; it.golden = "V912 main.cpp:12156"; it.cls = "motor";
        it.what = "IndexMotorBreakerOFF()（煞車鎖住：Index Z1／Z2＋MagazineBreakerOFF／InOutArmZBreakerOFF／LDCarRotArmZBreakerOFF／CassetteBreakerOFF）";
        const IoSnap a = TakeSnap();
        if (execute) {
            ht9045::Pci1203RouteSetSource("shutdown");
            IndexMotorBreakerOFF();                                             // golden :12156
            ht9045::Pci1203RouteSetSource("engine");
        }
        const IoSnap b = execute ? TakeSnap() : a;
        ClassifyIoGroup(&it, a, b, execute,
                        "HT9050 會寫到的：SwFMotorBreaker、SwInArmZBreaker、SwOutArmZBreaker、SwCassetteLD／Auto1／Auto2MotBreaker（Off＝鎖住，EastSun R1）。"
                        "⚠ golden 分支：SnFMotorDown／SnBMotorDown（手動下降鍵）按著且 D48==false 時改成 SwManualZ.On()＋煞車 On（放開），HT9050 這兩個感測器 Enable=0");
        v.push_back(it);
    }
    v.push_back(SwOffItem(SwHeaterRelay, "SwHeaterRelay", "12157", "heat", execute));   // golden :12157
    if (execute) HeaterLog("Close", false);                                     // golden :12158 //Steven 20151123 : Log for Heater Relay
    SdAdd(&v, "12158", "ui", "HeaterLog(\"Close\", false)", "stub", "本體的寫檔那一行在 #if 0（cpublic.cpp GATE PT-W5c）⇒ 沒有寫 Heater log");
    SdAdd(&v, "12160", "ui", "#ifndef SOFT_SIMULTE TTLLog(\"Close\")", "missing", "cpublic.cpp 的 TTLLog 在 #if 0（GA1-B3）");
    SdAdd(&v, "12162", "ui", "ProductionLog(\"Close\")", "missing", "cpublic.cpp 的 ProductionLog 在 #if 0（GA1-B3）");
    v.push_back(SwOffItem(SwHeaterFan,        "SwHeaterFan",        "12163", "output", execute));
    v.push_back(SwOffItem(SwIndexIonFan,      "SwIndexIonFan",      "12164", "output", execute));
    v.push_back(SwOffItem(SwDutHeaterCoolFan, "SwDutHeaterCoolFan", "12165", "output", execute));
    v.push_back(SwOffItem(SwShuttleCooling,   "SwShuttleCooling",   "12166", "output", execute));   //jou 2010-06-09 start
    v.push_back(SwOffItem(SwHotplateCooling,  "SwHotplateCooling",  "12167", "output", execute));   //jou 2013-11-07
    v.push_back(SwOffItem(SwCarRecord,        "SwCarRecord",        "12168", "output", execute));   //2012-12-09 行車紀錄關閉
    if(SW[SwAirOff].Enable==true)                                               //Steven 20221215 : Power saving for vacuum pump   // golden :12169
        v.push_back(SwOffItem(SwAirOff, "SwAirOff", "12170", "output", execute));
    else
        SdAdd(&v, "12169-12170", "output", "if(SW[SwAirOff].Enable) SW[SwAirOff].Off()", "noop", "IO_Table 沒有 SwAirOff（Enable=0）：golden 同樣不寫");
    if (execute) SoftStop=true;                                                 // golden :12171
    SdAdd(&v, "12171", "motor", "SoftStop=true", "done", "記憶體旗標（tick 迴圈已停，狀態機不會再跑）");

    // ---- :12174-12216  ATC6.0／3.0／New ATC、主執行緒、ADAM、條碼、Galil ---------------------------------------
    {
        const bool atc = (ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);
        std::snprintf(buf, sizeof(buf), "（ATC_SYSTEM=%d）", ATC_SYSTEM);
        SdAdd(&v, "12174-12191", "heat", "ATC6.0／3.0：ATC_60_SYS.SetATCRun(false)＋CloseSocket()；New ATC：ATC_InterfaceForm->Stop()＋Disconnet()",
              atc ? "missing" : "noop",
              std::string(atc ? "這一輪不接（ATC 區同事正在改）⇒ ATC 沒有收到 STOP" : "不是 ATC6.0／3.0／New ATC：golden 同樣不做") + buf);
    }
    SdAdd(&v, "12193", "ui", "EndMainThread()", "stub",
          "不呼叫：MyThread 在 wb_serve 是 NULL（沒有 golden 主執行緒；tick 迴圈已停），golden 本體直接解參考 ⇒ 呼叫會當掉");
    SdAdd(&v, "12194", "output", "Close_ADAM_6024()", "missing", "adam6024 沒有翻（Jimmy）");
    SdAdd(&v, "12195", "save", "fCounterClear->WriteCTInfo()", s_bTailRan ? "done" : "missing",
          s_bTailRan ? "在存檔段（W906_ProdCloseSave_Tail／Exit step 2），不在這裡" : "存檔段沒有跑（bHandlerModel=false：LastSet 沒讀過，不存）");
    SdAdd(&v, "12198-12203", "comm", "fBarCode->btBarcodeChangeFileDisConnect->Click()", "missing", "");
    SdAdd(&v, "12205-12216", "motor", "#ifndef SOFT_SIMULTE if(bGali_CardInstall) DMCCommand(hDmc, \"RS\")（Galil 重置）",
          bGali_CardInstall ? "missing" : "noop",
          bGali_CardInstall ? "這一輪沒有接（要 Galil 廠商標頭；Index 的 Galil 控制器沒有重置）" : "這台沒有開 Galil 卡（bGali_CardInstall=false）");

    // ---- :12219-12477  計時器、delete、hotkey、MN200、紀錄、強制結束 ---------------------------------------------
    SdAdd(&v, "12219-12225", "ui", "fCCLink->Timer1／Timer1..4／TimerScanKey／TimerESD->Enabled=false", "noop", "tick 迴圈已停");
    SdAdd(&v, "12227-12420", "ui", "delete InArmOffSet／ArmData／Alarm／MyThread／HeaterThread／My232Thread／MyPLCIOThread／PassList／…", "noop", "行程結束時釋放");
    SdAdd(&v, "12341", "save", "RecordIndexPosition(0, 3)", "stub", "Motor/mymotor.cpp:2704 空殼（golden 本體在 #if 0）；不呼叫");
    SdAdd(&v, "12422-12431", "ui", "ReleaseDC／UnregisterHotKey", "noop", "網頁版沒有");
    {
        const bool mn = ((IO_CARD_TYPE==MotionnetIO_MN200 || IO_CARD_TYPE==NewIO_MN200) && USE_ROTATE_KIT==1);
        SdAdd(&v, "12433-12440", "motor", "#ifndef SOFT_SIMULTE mn_stop_line(0..3)（MN200＋旋轉 Kit）", mn ? "missing" : "noop",
              mn ? "這一輪沒有接" : "這台不是 MN200 IO＋旋轉 Kit");
    }
    SdAdd(&v, "12443", "save", "NewRecordProcess(\"MES2109\", \"Program Close\", …)", "missing", "");
    SdAdd(&v, "12444", "save", "WriteIniDataGeneral(\"Record\", \"Program Close\", 1)", "done", "wb_serve 在本函式之後寫（tools/wb_serve.cpp 關站那一行）");
    SdAdd(&v, "12446", "save", "LogIndexMaxMinPos(\"Program closed\")", "missing", "");
    SdAdd(&v, "12453-12474", "ui", "del.bat：timeout 10 秒後 taskkill /F /IM HT9045.exe", "missing",
          "刻意不做：wb_serve 自己結束；那支 bat 會殺掉同一台電腦上的 BCB6 HT9045.exe");
    return v;
}

void WriteSdItems(webbridge::JsonWriter& w, const std::vector<SdItem>& v, const char* phase)
{
    int nDone = 0, nNoop = 0, nStub = 0, nMissing = 0, nFailed = 0, nUnverified = 0;
    w.Key("phase").String(phase);
    w.Key("items").BeginArray();
    for (std::size_t i = 0; i < v.size(); ++i) {
        const SdItem& it = v[i];
        if (it.status == "done") ++nDone; else if (it.status == "noop") ++nNoop; else if (it.status == "stub") ++nStub;
        else if (it.status == "missing") ++nMissing; else if (it.status == "failed") ++nFailed; else ++nUnverified;
        w.BeginObject();
        w.Key("golden").String(it.golden);
        w.Key("cls").String(it.cls);
        w.Key("what").String(it.what);
        w.Key("status").String(it.status);
        w.Key("label").String(SdLabel(it));
        w.Key("detail").String(it.detail);
        w.EndObject();
    }
    w.EndArray();
    w.Key("counts").BeginObject();
    w.Key("done").Number((wb_int64)nDone);       w.Key("noop").Number((wb_int64)nNoop);
    w.Key("stub").Number((wb_int64)nStub);       w.Key("missing").Number((wb_int64)nMissing);
    w.Key("failed").Number((wb_int64)nFailed);   w.Key("unverified").Number((wb_int64)nUnverified);
    w.EndObject();
    w.Key("notStopped").BeginArray();                                           // 停馬達／關加熱／關輸出 類裡，沒有確定停下來的
    for (std::size_t i = 0; i < v.size(); ++i)
        if (IsHwCls(v[i].cls) && v[i].status != "done" && v[i].status != "noop")
            w.String(v[i].what + "（" + v[i].status + "）");
    w.EndArray();
}

}  // namespace

// ===========================================================================
//  AI(W906-PROD-S95R) 20260926（Steven 團隊）：golden TfMain::SaveRunMode（V912 main.cpp:33648-33658，宣告 main.h:1227），逐行。
//  RULINGS_20260926 S95／S120-1 排後的 RunMode.txt（S121 交件的 D5）。golden 全樹只有兩個呼叫點：
//    FormClose :11863（本檔兩個入口：Exit step 1、W906_ProdCloseSave）與 SetRunStartMode :1129
//    （＝移植樹 RunStartMode.cpp:883 `fMain->SaveRunMode();`，移植樹沒有第三個呼叫者；那個檔不改）。
//  接法照 S92 BackupSetupFile 的安裝座（理由同 FileRW/MainBackup.cpp 檔頭）：
//    forms/fMain.h:321 原本是空的 inline `void SaveRunMode() {}`（它上面 :317 的註解說「寫回 LastSet」—— 不對，golden 寫的是
//    RunMode.txt）；改成宣告，定義在 forms/fMain.cpp:458（跟 BackupSetupFile 同一行，經 W906_SaveRunModeBody 轉，預設 0＝原本的 no-op）；
//    wb_serve 開機呼叫 W906_FRW_InstallSaveRunMode() 裝上（共用檔 tools/wb_serve.cpp，片段見交件報告）。
//    ⇒ ctest 沒裝：RunStartMode.cpp:883 維持 no-op，測試不會寫真機的 RunMode.txt；本檔只編進 wb_serve，關程式那一次也只有 wb_serve 會寫。
//  關程式那一次（:11863）本檔直接叫本體、不經 fMain->SaveRunMode()：golden 叫的是同一支；這樣拿得到 fopen 成敗照實回報，
//    而且開機那一行片段就算還沒套，關程式照樣寫。
//  ⚠ 會寫的真實檔：d:\HT9045\system\RunMode.txt（整份覆寫，一行 "RunMode=<LastSet.iRunStartMode>"，沒有換行）。golden 字面路徑，沒有轉開接縫。
//  讀者：golden TfMain::CheckRunMode（main.cpp:33661-33681，Frank／Eastsun 20260710；main.cpp:5061 用它設 bShowRunModeStatus）——
//    歸 Jimmy，移植樹沒有，本檔不做。
//  [W906] 跟 golden 不同的一處：回傳 fopen 有沒有成功（golden 是 void）。fclose 之後 FILE* 的值不能再用（C11 7.21.3p4），所以先記下來。
// ===========================================================================
extern bool (*W906_SaveRunModeBody)();                                          // forms/fMain.cpp:458（預設 0）

bool W906_TfMain_SaveRunMode()                                                  //JerryYang 20161027 把Run mode存起來
{
    FILE *P;
    char str[256];
    P=fopen("d:\\HT9045\\system\\RunMode.txt", "w");
    const bool bOpened=(P!=NULL);                                               // [W906] 回報用
    if(P!=NULL)
    {
        sprintf(str, "RunMode=%d", LastSet.iRunStartMode);
        fputs(str, P);
        fclose(P);
    }
    return bOpened;
}

// wb_serve 開機呼叫（tools/wb_serve.cpp，接在 W906_FRW_InstallBackupSetupFile() 那一行同一行後面；LastSet 已讀，LoadMachineConfig 之後）。
//   裝上之後，SetRunStartMode（網頁換配方 WebRecipeChange.cpp:480、WebStart.cpp:3890、JsonBridge/actions/MainTesterConnect.cpp:100、
//   QA／SetUp 頁存檔…）每一次都照 golden main.cpp:1129 寫 RunMode.txt。
//   [W906] bHandlerModel==false 時不裝（理由同 W906_ProdCloseSave 的共同前提：LastSet 沒讀過是零，一換模式就把 RunMode.txt
//   寫成 "RunMode=0"；golden 在那時 FormShow 就 Terminate，走不到 SetRunStartMode）。
void W906_FRW_InstallSaveRunMode()
{
    if (bHandlerModel == false)
    {
        std::printf("FileRW MainClose: TfMain::SaveRunMode body NOT installed -- bHandlerModel=false (LastSet was never read; "
                    "RunMode.txt would be overwritten with RunMode=0; golden FormShow main.cpp:9589-9594 terminates)\n");
        return;
    }
    W906_SaveRunModeBody=&W906_TfMain_SaveRunMode;
    std::printf("FileRW MainClose: TfMain::SaveRunMode body installed (golden main.cpp:33648; SetRunStartMode -> RunStartMode.cpp:883 now writes "
                "d:\\HT9045\\system\\RunMode.txt like golden main.cpp:1129)\n");
}

bool W906_FRW_SaveRunModeInstalled() { return W906_SaveRunModeBody==&W906_TfMain_SaveRunMode; }

// ---------------------------------------------------------------------------
//  AI(W906-PROD-S121) 20260926: 結束旗標。Exit 第二框確認、守衛全過、存檔完之後由 W906_Main_CloseProgramOp 設（tick 執行緒）。
//  std::atomic：將來 W906_ConsoleCtrl（系統另開的執行緒）若也要走停機，只設這個旗標、讓 tick 執行緒自己關站（見交件報告）。
// ---------------------------------------------------------------------------
std::atomic<bool> W906_ServeQuitRequested(false);

// wb_serve 主迴圈底部每圈問一次（tools/wb_serve.cpp --seconds 檢查的下一個敘述；共用檔片段見交件報告）。
//   要求之後的第一次詢問回 false（這一圈：step 2 的 ack 剛交給 server.CompleteCommand、publish 之後 Wake），下一圈才回 true。
bool W906_ServeQuitDue()
{
    if (!W906_ServeQuitRequested.load()) return false;
    if (!s_bQuitSeen) { s_bQuitSeen = true; return false; }
    std::printf("MainClose: Exit confirmed one loop ago -> leaving the main loop (golden FormClose: save -> shutdown -> Program Close=1 -> server.Stop)\n");
    return true;
}

// ---------------------------------------------------------------------------
//  AI(W906-PROD-S121) 20260926: golden FormClose 存檔以外的關站段（見 ShutdownSequence）。wb_serve 正常關站那一點呼叫：
//    W906_ProdCloseSave() 之後、WriteIniDataGeneral("Record","Program Close",1) 之前、server.Stop() 之前（1203 命令面與監看器還活著）。
//  主迴圈已停：沒有別的執行緒會同時碰 SW[]／1203（WebBridgeServer 的 socket 執行緒不執行機台碼）。
//  訊息 hook 已卸（wb_serve.cpp:5957-5959）⇒ 途中任何 ShowMyMessage／ShowErrorMessage 都不會卡住等網頁回答。
//  [W906] 跟 golden 不同的一處：bHandlerModel==false 時照樣停機（W906_ProdCloseSave 在那時不存檔）。golden 在那時 FormShow 就
//    Terminate，沒有機會啟動任何東西；移植樹的 wb_serve 照常跑，網頁可能打開過輸出 —— 停機是安全方向，所以不跳過。
// ---------------------------------------------------------------------------
void W906_ProdCloseShutdown()
{
    const std::vector<SdItem> v = ShutdownSequence(true);
    std::printf("MainClose: golden FormClose shutdown (V912 main.cpp:11881-12474), %u steps:\n", (unsigned)v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (!IsHwCls(v[i].cls) && v[i].status != "failed") continue;           // 主控台只列停機類與失敗的；完整清單在下面的 JSON
        std::printf("  [%s/%s] %s %s -- %s\n", v[i].status.c_str(), SdLabel(v[i]), v[i].golden.c_str(), v[i].what.c_str(), v[i].detail.c_str());
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    WriteSdItems(w, v, "executed");
    w.EndObject();
    std::printf("MainClose: shutdown JSON %s\n", w.Ok() ? w.Str().c_str() : "(json-writer-misuse)");
    std::fflush(stdout);
}

// ---------------------------------------------------------------------------
//  golden FormClose 生產資料段的後半（在 golden 的 "Sure To Exit?" 之後）：:11919 → :11999-12001 → :12195。
//  writes／skipped 可以是 NULL。
// ---------------------------------------------------------------------------
void W906_ProdCloseSave_Tail(std::vector<std::string>* writes, std::vector<std::string>* skipped)
{
    if (!s_bJamRateSavedOnClose)
    {
        const AnsiString sJam=W906_RunInfo_SaveJamRateByDay(false);             // golden :11919 RunInfo.SaveJamRateByDay(false);  //Steven 20250528 : By Day Jam Rate
        s_bJamRateSavedOnClose = true;
        if (writes) writes->push_back(std::string(sJam.c_str()) + "（RunInfo.SaveJamRateByDay(false)，golden FormClose :11919；整份覆寫）");
    }
    else if (skipped)
    {
        skipped->push_back("RunInfo.SaveJamRateByDay(false)（golden :11919）：本行程已經存過一次，它的尾端 InitialDailyData() 已清掉記憶體，"
                           "再存會用空資料蓋掉當天的 DailyJamRate 檔 —— 不再存（[W906] s_bJamRateSavedOnClose）");
    }

    // golden :11999-12001（DEBUG_AUTO_CLEAN 在 golden MachineType.h:26、移植樹 MachineType.h:31 都是註解掉的 ⇒ 不編）
    #ifdef DEBUG_AUTO_CLEAN
    btSavelog->Click();                                                         // memoAutoClean／btSavelog 不在移植樹：誰打開這個巨集會編譯失敗（看得見）
    #endif

    W906_Main_TimerRecordLoaderDate();                                          //Steven 20101105 : 關掉的時候也要存一下   // golden :12090  AI(W906-PROD-S121) 20260926: Jimmy J10 併進
    if (writes) writes->push_back("TimerRecordLoaderDate()（golden FormClose :12090，本體照翻 golden main.cpp:27970）：D:\\HT9045_Log\\EventLogTxt\\…（slEventLog 兩行）"
                                  "＋ D:\\HT9045_Log\\SaveEventLog\\HANDLER LOG_<ID>_<日期>.csv（兩行）；DB 那一半退場（MyDBExecSQL 回 0，golden bUseMDB==false 同）");

    fCounterClear->WriteCTInfo();                                               // golden :12195（呼叫方式同 Jimmy J4 csystem.cpp:4096；fCounterClear 在 cCounterClear.cpp:26 靜態初始化就 new，不會是 NULL）
    if (writes) ListCTInfoFiles(writes);
    s_bTailRan = true;                                                          // AI(W906-PROD-S121) 20260926: 關站清單的 :12090／:12195 照實標
}

// ---------------------------------------------------------------------------
//  wb_serve 正常關站那一點（golden FormClose 被觸發）。golden 在 FormClose 裡的順序：:11861 WriteLastDataFile(true) →
//  （"Sure To Exit?"：這條路是程式自己要結束，沒有人可以回答，照 golden 自動更新關站 bCloseByAutoUpdate2 的路徑不問）→ 後半。
//  golden :11865-11879 KYEC／AMKOR 空跑的 One Cycle 檢查（Action=caNone 取消關閉）在這裡不能取消 —— 程式一定會結束；不做。
// ---------------------------------------------------------------------------
void W906_ProdCloseSave()
{
    if (bHandlerModel == false)
    {
        std::printf("MainClose: production-data save at close SKIPPED -- bHandlerModel=false (LastSet was never read: "
                    "database.cpp returns before ReadLastSetIni; golden FormShow main.cpp:9589-9594 terminates, FormClose never runs)\n");
        return;
    }
    if (s_bExitSaved)                                                           // AI(W906-PROD-S121) 20260926: Exit 鈕這條路 step 1／2 已經把 golden FormClose 的存檔做完
    {
        std::printf("MainClose: production data already saved by the Exit button (steps 1/2 = golden FormClose :11861/:11862/:11863/:11864/:11919/:12090/:12195) -- not saved twice\n");
        return;
    }
    const bool bLast = WriteLastDataFile(true, false);                          // golden FormClose :11861 WriteLastDataFile(true);
    SaveMachineRecord(false);                                                   // golden FormClose :11862 SaveMachineRecord();（預設參數 false）  AI(W906-PROD-S121) 20260926: Jimmy J10 併進
    std::vector<std::string> writes, skipped;
    writes.push_back("SaveMachineRecord()（golden :11862）：D:\\HT9045\\system\\machinerecord.dat（REAL_TIME_CCD 時 machinerecordRealCCD.dat）、PickHPRec、D:\\UnloaderInfo\\…");
    const bool bRunMode = W906_TfMain_SaveRunMode();                            // golden FormClose :11863 SaveRunMode();  //JerryYang 20161027 把Run mode存起來   AI(W906-PROD-S95R) 20260926（Steven 團隊）
    writes.push_back(RunModeWriteText(bRunMode));
    LotSummary.WriteFile();                                                     // golden FormClose :11864 LotSummary.WriteFile();  //Steven 20190726 : ATK ART Lot Count   AI(W906-PROD-S95R) 20260926（Steven 團隊）
    writes.push_back(LotSummaryWriteText());
    W906_ProdCloseSave_Tail(&writes, &skipped);
    std::printf("MainClose: golden FormClose production data -- lastdata.dat(+backup, backup2) %s", bLast ? "written" : "WRITE FAILED");
    for (std::size_t i = 0; i < writes.size(); ++i)  std::printf("; %s", writes[i].c_str());
    for (std::size_t i = 0; i < skipped.size(); ++i) std::printf("; skipped: %s", skipped[i].c_str());
    std::printf("\n");
}

// ---------------------------------------------------------------------------
//  WS act.main.closeProgram　value={"step":0|1|2}
//    step 0＝按下 Exit（golden sbCloseProgramClick 的守衛 :29053-29105）→ 通過就回 needConfirm "Sure close??"（:29107）。
//    step 1＝"Sure close??" 回 YES（:29108-29131：AMKOR 密碼、VTEST／OEE 報表、ReadWriteBinCountMode(false)）→ Close() 進 FormClose：
//           :11861 WriteLastDataFile(true) → :11865-11879 KYEC／AMKOR One Cycle 檢查 → 回 needConfirm "Sure To Exit?"（:11903）。
//           （AI(W906-PROD-S121／S95R) 20260926：:11861 之後依序 :11862 SaveMachineRecord → :11863 SaveRunMode → :11864 LotSummary.WriteFile。）
//           golden 在 bUpdateAutomatically（FTP 自動更新）時不問，直接做後半。
//    step 2＝"Sure To Exit?" 回 YES → 後半（W906_ProdCloseSave_Tail）。
//    任一步回 NO：頁面不再送（golden 的 return／Action=caNone）。第二框 "Sure To Exit?" 回 NO 時，step 1 已經寫了的 BinCount.txt、lastdata.dat 留著（golden 同：兩者都在第二框之前）。
//  ⚠ （S95 原句）存完不關站：回應 shutdown.implemented=false。
//    AI(W906-PROD-S121) 20260926：改成**會關站** —— 後半存完之後設 W906_ServeQuitRequested，wb_serve 主迴圈晚一圈 break，
//    走正常關站路（W906_ProdCloseSave〔已存過就不重存〕→ W906_ProdCloseShutdown → Program Close=1 → server.Stop()）。
//    回應 shutdown.implemented=true／pending=true，shutdown.items 是關站段每一步的**預估**（phase=plan：還沒做；
//    真正的結果在 wb_serve 主控台 "MainClose: shutdown JSON" 那一行 —— 做完的時候網頁連線已經斷了）。
//  權杖：跟其他動作一樣要控制權杖（WebBridgeServer）；不在 WebCmdGuard 白名單（三步的 value 不同，不會互擋）。
//  呼叫端持 FormLock？不用 —— 本函式自己持（可重入，同 WebShowBinSelect.cpp）。
// ---------------------------------------------------------------------------
std::string W906_Main_CloseProgramOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    std::vector<Msg> msgs;
    std::vector<std::string> writes, skipped, gaps;

    if (W906_ServeQuitRequested.load())                                         // AI(W906-PROD-S121) 20260926: 已經在關站 —— 不再接任何一步
        return Refuse(-1, "closing", "", "Exit 已確認，程式正在關站（下一圈主迴圈結束），不再接受 Exit 的任何一步", msgs, writes);

    int step = -1;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0)
            return Refuse(-1, "bad-payload", "", "value 不是合法 JSON（要 {\"step\":0|1|2}）", msgs, writes);
        const cJSON* js = cJSON_GetObjectItemCaseSensitive(root, "step");
        if (js && cJSON_IsNumber(js) && js->valuedouble == (double)js->valueint && js->valueint >= 0 && js->valueint <= 2)
            step = js->valueint;
        cJSON_Delete(root);
        if (step < 0)
            return Refuse(-1, "bad-payload", "", "step 要是 0、1 或 2 的整數", msgs, writes);
    }

    FormLockGuard lock;

    if (bHandlerModel == false)
        return Refuse(step, "model-read-error", "V912 main.cpp:9589-9594",
                      "bHandlerModel=false：開機沒有讀 lastdata.dat（LastSet 是零），存檔會用零蓋掉機台的產量；golden 在開機時就結束程式", msgs, writes);

    // ---- golden sbCloseProgramClick 的守衛（每一步都重跑）------------------------------------------------------
    {
        const char* guard = ""; const char* golden = ""; std::string detail;
        if (!CloseGuards(&guard, &golden, &detail, &msgs))
            return Refuse(step, guard, golden, detail, msgs, writes);
    }

    if (step == 0)
    {
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     // golden :29094-29095 //kevin 20210601 add close program 需確認吸嘴上是否有IC
        {
            Msg m; m.s1 = "Please confirm if IC is still in the In/Out & index arm "; m.s2 = "請確認IC是否還留在 In/Out & index arm 上?";
            msgs.push_back(m);
        }

        if(AirStream_Select==1)                                                 // golden :29097 ...&& ATC_InterfaceForm->IsConnect()==true
        {
            // golden :29097-29105 問 "Sure close Air Stream??"，YES 就 bCloseAirMachine=true＋SendAirMachineStatus(0, …)（送 ATC 的設備命令）。
            // 設備控制屬關站那一半（Jimmy），這裡不問也不送 ⇒ Air Stream 不會被關掉。
            gaps.push_back("V912 main.cpp:29097-29105 AirStream（HT-1032 TriTemp）關閉詢問與 SendAirMachineStatus 沒有接：Air Stream 不會被關掉");
        }

        s_iCloseStage = 1;
        webbridge::JsonWriter w;
        w.BeginObject();
        NeedConfirm(w, 0, 1, "Sure close??", "確定要關閉程式??", "V912 main.cpp:29107 ShowMyMessageBox_YES_NO");
        WriteMessages(w, msgs);
        WriteList(w, "gaps", gaps);
        w.EndObject();
        if (ok) *ok = true;
        std::printf("main.closeProgram step=0 -> needConfirm \"Sure close?\?\"\n");   // "\?"：避開 ??) 三字符警告
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    if (step == 1)
    {
        if (s_iCloseStage != 1)
            return Refuse(step, "confirm-order", "", "還沒有問過 \"Sure close??\"（要先送 step 0）", msgs, writes);
        s_iCloseStage = 0;

        // ---- golden :29108 if(ret==1) 分支 -------------------------------------------------------------------------
        if(CUSTOMER_CODE==CC_AMKOR_Philippines)                                 // golden :29110 && DoPassword()==false  //JerryYang : 20200522
        {
            // DoPassword()（golden 的密碼框）移植樹沒有；驗不了密碼就不放行（fail-closed，客戶專屬 S25）。
            return Refuse(step, "password-not-ported", "V912 main.cpp:29110-29113",
                          "CC_AMKOR_Philippines 關程式要先過 DoPassword()，移植樹沒有這個密碼框", msgs, writes);
        }

        if(IniConfig.bVTESTFunction==true)                                      // golden :29115
        {
#if 0 // GATE (S95-J2) golden :29117 -- TfMesSystem::SaveStringlRecordReport 在 forms/fMesSystem.h:671 只宣告沒定義（GATE W-07，呼叫＝連結錯誤）
            fMesSystem->SaveStringlRecordReport();
#endif
            gaps.push_back("V912 main.cpp:29117 fMesSystem->SaveStringlRecordReport()（VTEST）：移植樹 GATE W-07，沒有存");
        }

        if(CosFunction.bOEEFunction)                                            //Steven 20180417 (Jou) : OEE功能   // golden :29120
        {                                                                       //Sam 20170810 (Steven) 移植超豐 OEE 功能 form HT-7045
            if(IniConfig.bN14_1_EnableOEEFunction==true &&
               IniConfig.iN14_1_OEERecordCycleTime>0)
            {
#if 0 // GATE (S95-J3) golden :29125 -- TfProductionInfo 門面（forms/fProductionInfo.h）沒有 EachCycleSecondDo_SaveAndUpdateOEEFiles
                fProductionInfo->EachCycleSecondDo_SaveAndUpdateOEEFiles(true);
#endif
                gaps.push_back("V912 main.cpp:29125 fProductionInfo->EachCycleSecondDo_SaveAndUpdateOEEFiles(true)（OEE）：移植樹沒有，沒有存");
            }
        }

        ReadWriteBinCountMode(false);                                           //kevin 20210825 寫 Bin 1  Bin 2...資料記錄   // golden :29129
        {
            const char* p = getenv("W906_BINCOUNT_PATH");
            writes.push_back(std::string(p ? p : "D:\\HT9045\\system\\BinCount.txt") + " [System] Bin／ErrorBin（ReadWriteBinCountMode(false)，golden :29129）");
        }

        // golden :29130 Close() → TfMain::FormClose（main.cpp:11852）
        //   :11860 SaveFormPos() —— 主視窗位置，網頁沒有這個視窗；不做。
        const bool bLast = WriteLastDataFile(true, false);                      // golden :11861 WriteLastDataFile(true);
        writes.push_back(std::string("D:\\HT9045\\system\\lastdata.dat、lastdata_backup.dat、lastdata_backup2.dat ＋ config\\config.ini（WriteLastDataFile(true)，golden FormClose :11861）")
                         + (bLast ? "" : " —— ⚠ WriteLastDataFile 回 false（WAR1682）"));
        SaveMachineRecord(false);                                               // golden :11862 SaveMachineRecord();（預設參數 false）  AI(W906-PROD-S121) 20260926: Jimmy J10 併進（本體 cinitial.cpp:17017）
        writes.push_back("SaveMachineRecord()（golden FormClose :11862）：D:\\HT9045\\system\\machinerecord.dat（REAL_TIME_CCD 時 machinerecordRealCCD.dat）、PickHPRec、D:\\UnloaderInfo\\…");
        //   AI(W906-PROD-S95R) 20260926（Steven 團隊）：S120-1 排後的 :11863／:11864 接上（S95 時是 skipped）。
        //   SaveRunMode 直接叫本檔的本體（golden 是 TfMain 成員；移植樹 fMain->SaveRunMode() 經 W906_SaveRunModeBody 轉到同一支，見 S95R 段）。
        const bool bRunMode = W906_TfMain_SaveRunMode();                        // golden :11863 SaveRunMode();  //JerryYang 20161027 把Run mode存起來
        writes.push_back(RunModeWriteText(bRunMode));
        LotSummary.WriteFile();                                                 // golden :11864 LotSummary.WriteFile();  //Steven 20190726 : ATK ART Lot Count（本體 cSocket.cpp:1198）
        writes.push_back(LotSummaryWriteText());

        if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_AMKOR_Philippines)   // golden :11865 //JerryYang 20200430 空跑不用完成one cycle
        {
            if(LastSet.iRealDummy==REALLY)
            {
                if(FileRW_ArmSuckHasIC(0)   ||                                  // InArmSuck.HasIC()
                   FileRW_ArmSuckHasIC(1)   ||                                  // OutArmSuck.HasIC()
                   ShuttleHasIC()           ||
                   IndexHasIC()             )
                {
                    Msg m; m.s1 = "Must finish OneCycle process before terminal"; m.s2 = "請在關閉程式前完成One Cycle"; m.s3 = "Program Close";
                    msgs.push_back(m);                                          // golden :11874 ShowMyMessage(...)
                    return Refuse(step, "one-cycle", "V912 main.cpp:11865-11879（Action=caNone）",
                                  "KYEC／AMKOR 實跑時機台內有 IC（上面 sbCloseProgramClick 的同一個檢查已經先擋，這裡照 golden 保留）", msgs, writes);
                }
            }
        }

        // :11881-11884 ResetAutoClean()、:11886-11892 Epson DLL 檔案對映 —— 不是生產資料，屬關站那一半；不做。
        // :11894-11917 "Sure To Exit?"（#ifndef DEBUG —— golden 專案只定義 _DEBUG，DEBUG 沒定義 ⇒ 會問）
        //   golden bCloseByAutoUpdate2／bNeedRestartProgram 是 TfMain 成員，移植樹 forms/fMain.h 沒有 ⇒ 等同 false。
        const bool bAsk = (bUpdateAutomatically == false);                      // golden :11900-11903（bUpdateAutomatically 時 ret=1）
        if (bAsk)
        {
            s_iCloseStage = 2;
            webbridge::JsonWriter w;
            w.BeginObject();
            NeedConfirm(w, 1, 2, "Sure To Exit?", "確定要離開？", "V912 main.cpp:11903 ShowMyMessageBox_YES_NO（FormClose）");
            WriteList(w, "writes", writes);                                     // golden 在問這一框之前已經寫了的
            WriteList(w, "skipped", skipped);
            WriteList(w, "gaps", gaps);
            WriteMessages(w, msgs);
            w.EndObject();
            if (ok) *ok = true;
            std::printf("main.closeProgram step=1 -> BinCount.txt + lastdata.dat %s + machinerecord + RunMode.txt %s + LotSummary.csv; needConfirm (Sure To Exit?)\n",
                        bLast ? "written" : "(lastdata WRITE FAILED)", bRunMode ? "written" : "(RunMode fopen FAILED)");   // AI(W906-PROD-S95R) 20260926: 加 RunMode／LotSummary
            return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
        }
        // bUpdateAutomatically：golden 不問，直接往下（落到下面的後半）
    }
    else  // step == 2
    {
        if (s_iCloseStage != 2)
            return Refuse(step, "confirm-order", "", "還沒有問過 \"Sure To Exit?\"（要先送 step 1）", msgs, writes);
        s_iCloseStage = 0;
    }

    // ---- golden FormClose "Sure To Exit?" 之後 -------------------------------------------------------------------
    //   :11911-11914 if(IniConfig.bEnable_SECS_GEM) EventReport(SECS_EVENT.DoExit)
    //   （S95 原本不送：程式沒有真的結束。AI(W906-PROD-S121) 20260926：現在會結束，照 golden 送；
    //    移植樹的 EventReport 是 Sim 計數器（SECSGEM/SecsEventReport.cpp:15，只記 CEID），主機收不到 —— 回報照實寫。）
    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
    {
        EventReport(SECS_EVENT.DoExit);                                         //24     按下 Exit
        gaps.push_back("V912 main.cpp:11911-11914 SECS EventReport(SECS_EVENT.DoExit)：已呼叫，但移植樹的 EventReport 是 Sim 計數器（SecsEventReport.cpp:15），主機收不到（stub）");
    }
    W906_ProdCloseSave_Tail(&writes, &skipped);

    // AI(W906-PROD-S121) 20260926: 存檔做完 ⇒ 要求 wb_serve 結束（主迴圈晚一圈 break，見 W906_ServeQuitDue）。
    //   golden 的 Close() 走完 FormClose 程式就結束；這裡關站段在 wb_serve 主迴圈外面做（W906_ProdCloseShutdown），
    //   所以回應帶的是**預估**（phase=plan，不寫任何東西），不是結果。
    s_bExitSaved = true;
    W906_ServeQuitRequested.store(true);
    const std::vector<SdItem> plan = ShutdownSequence(false);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("step").Number((wb_int64)step);
    w.Key("goldenLine").String("V912 main.cpp:29051-29131 sbCloseProgramClick → FormClose :11861／:11862／:11863／:11864／:11919／:12090／:12195（存檔）＋ :11920-12474（關站，wb_serve 主迴圈結束後做；:11924 SaveProductionRecord、:12051 SaveRmsInfo 在這一段）");
    WriteList(w, "writes", writes);
    WriteList(w, "skipped", skipped);
    WriteList(w, "gaps", gaps);
    WriteMessages(w, msgs);
    w.Key("shutdown").BeginObject();
    w.Key("implemented").Bool(true);
    w.Key("pending").Bool(true);
    w.Key("detail").String("生產資料已存。wb_serve 會在下一圈主迴圈結束，照 golden FormClose 的順序停機（停馬達、鎖煞車、關加熱器繼電器、風扇與蜂鳴器），"
                           "途中照 golden 的位置再存兩項（:11924 SaveProductionRecord、:12051 SaveRmsInfo，見下面清單）；"   // AI(W906-PROD-S95R) 20260926
                           "然後寫 Program Close=1、關掉網頁連線。下面是每一步的預估；替身、空殼、沒翻的標「未停」，那些東西**沒有被停下來**。"
                           "實際結果看 wb_serve 主控台 \"MainClose:\" 開頭的那幾行。");
    WriteSdItems(w, plan, "plan");
    w.EndObject();
    w.EndObject();
    if (ok) *ok = true;
    std::printf("main.closeProgram step=%d -> production data saved (%u file group(s)); QUIT requested -- wb_serve leaves the main loop next pass and runs W906_ProdCloseShutdown\n",
                step, (unsigned)writes.size());
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
