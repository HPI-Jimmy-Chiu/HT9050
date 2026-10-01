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
//
//  AI(W906-FRW-S167) 20260928 [W906] Q44（最小版）「按 Exit 必須先停下才能關閉」（RULINGS S163 的做法）。Steven 20260928 原話：
//    「Q44 bcb還沒移植的都先列為待辦, 至少馬達跟溫度的要停下來, 這個只要兩個命令就可以做到了」；追問三題都選建議：
//    ① 兩個命令＝停全部馬達（golden StopAllMotor＋移植樹停 1203 監看器開的每一軸）＋關加熱器繼電器（SW[SwHeaterRelay].Off()）；
//       SIM 建置接真 1203 卡時 Off() 只到模擬值 ⇒ 另外經 1203 命令面把那一個 DO 位元寫 0。
//    ② 讀回沒停（軸還在動／停止被拒；繼電器讀回仍是 1／寫入被拒；這次開機開過卡、現在失聯）⇒ 不關：頁面「還沒停：…」＋
//       ［重試停機］［強制關閉］（強制要 Exit 自己的等級 LevelSet.AccessLevel[6]、再確認一次、log 記「強制關閉，未停：…」）。
//    ③ Q44 其餘（1203 輸出全清、關站中只收關與停、Ctrl-C／主控台 X、看不見的裝置要操作員確認、Q44-1～6）與 golden 關站段
//       還沒移植的每一步 ⇒ 待辦（todo D-007），不擋 Exit。
//    做法：停機搬進 wb_serve 主迴圈（連線還在，結果送得到頁面）—— W906_ServeQuitDue 晚一圈之後改走 Q44QuitTick（檔尾）：
//      送兩個命令 → 每一圈讀回判一次 → 確認停了才離開主迴圈，之後照今天（ShutdownSequence(true)、Program Close=1、server.Stop()）；
//      沒停就留在主迴圈等 act.main.closeProgram {"op":"retry"}／{"op":"force","confirm":true}；頁面輪詢 {"op":"status"}。
//    檔尾 (甲) 純判斷 w906q44::Evaluate 只用標準庫：tests/test_mainclose_stop.cpp 定義 W906_MAINCLOSE_Q44_DECIDE_ONLY 後
//      #include 本檔，只編那一半 ⇒ 下面這一行 #ifndef 包住原本整個檔。
// ===========================================================================
#ifndef W906_MAINCLOSE_Q44_DECIDE_ONLY   //AI(W906-FRW-S167) 20260928 [W906] Q44：測試只編檔尾的純判斷（對應的 #endif 在檔尾 Q44 段前）
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
// AI(W906-FRW-S167) 20260928 [W906] Q44：本體在檔尾 (乙) 段（W906_ServeQuitDue／W906_ProdCloseShutdown／W906_Main_CloseProgramOp 先用）
static bool Q44QuitTick();
static bool Q44Op(const std::string& payloadJson, std::string* out, bool* ok);
static void Q44AtShutdown();
static void Q44WriteStep2(webbridge::JsonWriter& w);
static bool Q44ConsoleTick(bool inModal);                                       // AI(W906-D012) 20260929 [W906] Q44 A3：主控台 Ctrl-C／X（本體在檔尾 (乙)）
static void Q44MainClosing();                                                   // AI(W906-D012) 20260929 [W906] Q44 A3：主執行緒進正常關站路
static bool Q44CloseRecordClaim();                                              // AI(W906-D012-A8) 20260930 [W906]：golden :12443 MES2109「Program Close」一個行程只寫一次 —— 認領（本體在檔尾 (乙) A8）
static void Q44CloseRecordWritten();                                            // AI(W906-D012-A8) 20260930 [W906]：寫完了（主控台處理函式等的是這個，不是認領）
static const char* Q44CloseRecordLine();                                        // AI(W906-D012-A8) 20260930 [W906]：主控台一行（寫了沒有）
namespace w906q44 {                                                             // AI(W906-D012) 20260929 [W906] Q44 B：ShutdownSequence 用的純判斷（本體在檔尾 (甲)，ctest [17] 測）
inline const char* FileStepStatus(bool applies, bool handlerModel, bool execute);
inline bool        BarcodeClearApplies(int barCodeInstall, int kInShtIntel, int kEtherNetCCD, int kUseOCR, bool enableBarCode);
inline std::string KitSuckStepStatus(int nIo, int n1203, const std::string& ioGroup);
}
// AI(W906-D012) 20260929 [W906] Q44 B：golden FormClose 補接的三個呼叫（不 include 各自的標頭，理由同上；不帶預設引數）
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);    // cMyDB.h:129（本體 cMyDB.cpp:1865，golden cMyDB.cpp:1545-1562；St02 的檔，只呼叫、不改）。
                                                                                //   刻意不帶預設引數：cMyDB.h:129 是 Debug=" "、acatchtray_shims.h:439 是 S2=""／S3=""，兩個宣告不一樣；呼叫處三個引數寫全（golden 也寫全）
int  FileRW_CheckKitSuckNormal(bool execute, int* n1203);                       // FileRW/_KitSuck.cpp（golden :12047-12048 CheckKitSuck.Suck[0][0..1].Normal()）
extern void (*W906_HeaterLogHook)(AnsiString);                                  // cpublic.cpp:2594（LogObjects.cpp:130 W906_CreateLogObjects 裝）—— :12158 HeaterLog 的狀態照實標

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

#ifdef W906_EXIT_REQUIRE_HOME   // AI(W906-EXIT-NOHOME) 20260930: EastSun「我不需要確認都歸零」-- golden's home check is off by default (MachineType.h tail)
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
#endif // W906_EXIT_REQUIRE_HOME

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
struct SdItem { std::string golden, cls, what, status, detail; bool q44 = false; };   // AI(W906-FRW-S167) 20260928: q44＝Q44 的兩個命令也做這一項（頁面以 Q44 讀回為準，notStopped 不重列）

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
    SdAdd(&v, "11881-11884", "ui", "if(bRunAutoClean) ResetAutoClean()", bRunAutoClean ? "missing" : "noop",   // AI(W906-D012) 20260929: 沒在 Auto Clean＝golden 也不叫（noop）
          bRunAutoClean
              ? "不呼叫（AI(W906-D012) 20260929 ST01-E 決定）：本體在 AutoClean/AutoClean.cpp:3847，吸嘴上有 Clean pad 時跳 ShowMyMessage（請移除 Clean pad，"
                "主迴圈已停、網頁接不到這個框）、清 5 組 kit 資料、fAllMotorHome=false，並經 ReadWriteAutoCleanCount 寫配方的 HandlerCondition.Data；不是停機動作"
              : "沒有在 Auto Clean（bRunAutoClean=false）：golden 同樣不叫");
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
    // AI(W906-D012) 20260929 [W906] Q44 B：golden :12002-12008 照接（D-012 前是 missing）。本體 forms/fLotInfo.cpp:6443
    //   TfLotInfo::btClearBarcodeCountClick（逐行翻 V912 uLotInfo.cpp:10168-10184；vclcompat 的 Click() 是 no-op ⇒ 直接叫本體，
    //   同 WebLotInfo.cpp:248）：建 D:\HT9045_Log\2DBarCode\YYYY_MM_DD\ 資料夾、清五組條碼計數、重算表格。
    //   golden 會留下的清除前 .xls 在移植樹不產生（SGDToXLS 是 no-op，SgdToXLS.cpp GATE (1)）。bHandlerModel=false 時不做（存檔的共同前提）。
    {
        const bool applies = w906q44::BarcodeClearApplies(BAR_CODE_INSTALL, ebctInShtIntel, ebctEtherNetCCD, ebcUseOCR, TestIF_File.bEnableBarCode);
        if (execute && bHandlerModel)
        {
            if((BAR_CODE_INSTALL==ebctInShtIntel ||
                BAR_CODE_INSTALL==ebctEtherNetCCD ||                            //Ifor 20190129 : add Cognex EtherNet 通訊
                BAR_CODE_INSTALL==ebcUseOCR) &&                                 //Ifor 20210407 add: 自製OCR
               TestIF_File.bEnableBarCode)                                      //wei 20160413 Tray Feed後記錄Barcode顆數   // golden :12002-12005
            {
                fLotInfo->btClearBarcodeCountClick();                           // golden :12007 fLotInfo->btClearBarcodeCount->Click();
            }
        }
        const char* st = w906q44::FileStepStatus(applies, bHandlerModel, execute);
        std::snprintf(buf, sizeof(buf), "（BAR_CODE_INSTALL=%d、TestIF_File.bEnableBarCode=%d）", BAR_CODE_INSTALL, (int)TestIF_File.bEnableBarCode);
        const std::string d = std::string(st) == "noop" ? std::string("這台沒有 InShtIntel／EtherNetCCD／OCR 條碼，或沒開條碼（golden 同樣不清）") + buf
                            : std::string(st) == "missing" ? std::string("存檔段不寫（bHandlerModel=false：golden FormClose 不會跑）") + buf
                            : std::string(execute ? "" : "（預估）") + "TfLotInfo::btClearBarcodeCountClick（forms/fLotInfo.cpp:6443）：建 D:\\HT9045_Log\\2DBarCode\\<日期>\\ 資料夾、"
                              "清五組條碼計數；golden 的清除前 .xls 不產生（SGDToXLS 是 no-op）" + buf;
        SdAdd(&v, "12002-12008", "save", "fLotInfo->btClearBarcodeCount->Click()（條碼顆數）", st, d);
    }
    SdAdd(&v, "12010-12019", "comm", "fOCR->DoOCRReleaseAndInspEnd()／DoDisConnect()", "missing", "");
    SdAdd(&v, "12021-12025", "heat", "fOmron->Timer2->Enabled=false（EJ1N 溫控輪詢）", "noop",
          "網頁版沒有這個 VCL 計時器、tick 已停；golden 這一步只停輪詢，不關加熱器（加熱器是 :12157 的繼電器）");
    {
        const bool honAtc = (ATC_SYSTEM==eATCHonPrecType);
        std::snprintf(buf, sizeof(buf), "（ATC_SYSTEM=%d）", ATC_SYSTEM);
        SdAdd(&v, "12028-12035", "heat", "ATCInterfaceForm->SetRunATC(false)／bRunATC=false／ATCChillerSwitch(false)／OffLine()（HonPrec ATC）",
              honAtc ? "missing" : "noop",
              std::string(honAtc ? "有本體（ATC/ATCInterface.cpp:1129／:1201／:536），不接：ATC 區歸別組（AI(W906-D012) 20260929 ST01-E：ATC 不在本檔呼叫）⇒ ATC 沒有收到 STOP"
                                 : "不是 HonPrec ATC：golden 同樣不做") + buf);
    }

    // ---- :12041-12048  蜂鳴器、Index kit 吸嘴 ----------------------------------------------------------------
    v.push_back(SwOffItem(SwMusic1+0, "SwMusic1+0", "12043", "output", execute));   // golden :12041-12044  for(i<4) SW[SwMusic1+i].Off();
    v.push_back(SwOffItem(SwMusic1+1, "SwMusic1+1", "12043", "output", execute));
    v.push_back(SwOffItem(SwMusic1+2, "SwMusic1+2", "12043", "output", execute));
    v.push_back(SwOffItem(SwMusic1+3, "SwMusic1+3", "12043", "output", execute));
    // AI(W906-D012) 20260929 [W906] Q44 B：golden :12047-12048 照接（D-012 前是 missing「兩個 TMyKitSuck 佈局」—— 那個理由已過期：
    //   A4-6 起 aHotPlateSubstrate.h:97 改 include mykitsuck.h、舊的精簡佈局是 #if 0，只剩一個 TMyKitSuck）。本檔仍不 include 那兩個標頭
    //   （HTEditList.h／cMyDB.h 的名字會撞），經 FileRW/_KitSuck.cpp 的 FileRW_CheckKitSuckNormal 轉接。
    //   Normal()（mykitsuck.cpp:2189）＝OffSuck（真空關）＋OffDestroy（破壞關）＋bSuckOK／bDestroyOK=true；輸出是吸嘴自己的 On／Off 點
    //   （IO_Table 的 CheckKitSuck_1／_2 列，cinitial.cpp:514-515），沒有這些列就只改旗標。
    {
        SdItem it; it.golden = "V912 main.cpp:12047-12048"; it.cls = "output";
        it.what = "CheckKitSuck.Suck[0][0]／[0][1].Normal()（Index kit 吸嘴偵測：真空／破壞關）";
        int n1203 = 0;
        const IoSnap a = TakeSnap();
        int nIo = 0;
        if (execute) {
            ht9045::Pci1203RouteSetSource("shutdown");
            nIo = FileRW_CheckKitSuckNormal(true, &n1203);                      //jou 2011-12-26 關閉index kit吸嘴偵測 真空/破壞   // golden :12047-12048
            ht9045::Pci1203RouteSetSource("engine");
        } else {
            nIo = FileRW_CheckKitSuckNormal(false, &n1203);                     // 只數點，不寫
        }
        const IoSnap b = execute ? TakeSnap() : a;
        if (nIo > 0)
            ClassifyIoGroup(&it, a, b, execute, "CheckKitSuck_1／_2 的真空／破壞點 " + std::to_string(nIo) + " 個（其中 1203 " + std::to_string(n1203) + " 個）");
        it.status = w906q44::KitSuckStepStatus(nIo, n1203, it.status);
        if (it.status == "noop")
            it.detail = "IO_Table 沒有 CheckKitSuck_1／_2 的 On／Off 點（OnEnable／OffEnable 都是 false；SIM 建置也一律 false，cinitial.cpp:601-603）"
                        "⇒ Normal() 只改旗標，沒有輸出可關（golden 同）";
        else if (it.status == "unverified")
            it.detail += "｜有非 1203 的點：MN200／MNet／ISA 後端有沒有寫到卡，這裡沒有查證";
        v.push_back(it);
    }
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
          (NUMBER_PANEL_TYPE==3 || NUMBER_PANEL_TYPE==4)
              ? "不呼叫（AI(W906-D012) 20260929）：本體有（BinDisplay/MyBinDisp.cpp:311、vclcompat/Comm.cpp:349），但 HSys.BinDisCtrl 在 wb_serve 是 NULL —— "
                "建它的 InstallColorBinDisplay 只由 SystemModularInitial 呼叫（database.cpp:200），wb_serve 沒有呼叫 ⇒ 沒有 BinDisplay 可停；golden 直接解參考，照翻會當掉"
              : "這台不是 BinDisplay TFT（NUMBER_PANEL_TYPE 不是 3／4）");
    SdAdd(&v, "12088", "comm", "fCCLink->SetInitialOK(InitialOK)", "missing", "");
    SdAdd(&v, "12090", "save", "TimerRecordLoaderDate()", s_bTailRan ? "done" : "missing",
          s_bTailRan ? "在存檔段（W906_ProdCloseSave_Tail／Exit step 2），不在這裡；結果見那邊的 writes"
                     : "存檔段沒有跑（bHandlerModel=false：LastSet 沒讀過，不存）");
    SdAdd(&v, "12094-12146", "save", "CosFunction.bFTPFunction：_NET 工作檔 XCOPY／RD、刪 TesterMap.txt", "missing", "不做：會 XCOPY／RD 配方資料夾");

    // ---- :12149-12171  停執行緒、停馬達、煞車、加熱器繼電器、風扇 -------------------------------------------------
    // AI(W906-D012) 20260929 [W906] Q44 B：golden :12149 照接（D-012 前標 stub「acarry_shims.cpp:254 空殼」—— 已過期：AI(W906-MEMO) 20260927
    //   起 golden 本體逐行在 acarry_shims.cpp:269-293，cpublic.cpp:718 那份只是參考）。宣告 cpublic.h:39（經 cmydef.h），bFlag=true 照 golden 寫出。
    //   寫：<asShtLogPath>\YYYYMM\SH2_*.logs、SH1_*.logs（D:\HT9045_Log\ShuttleLog；fMain->cbShowShuttleSensor 勾著才寫，golden dfm 預設勾，
    //   forms/fMain.cpp:74），再清兩個 memo。bHandlerModel=false 時不寫（存檔的共同前提）。
    {
        const bool applies = (fMain != 0 && fMain->cbShowShuttleSensor->Checked == true);   // 本體的條件（bFlag=true 時只剩這一個）
        if (execute && bHandlerModel && fMain != 0)
            OutShuttleLog(true);                                                // golden :12149
        const char* st = w906q44::FileStepStatus(applies, bHandlerModel, execute);
        SdAdd(&v, "12149", "ui", "OutShuttleLog(true)", st,
              std::string(st) == "noop" ? "fMain->cbShowShuttleSensor 沒勾（或沒有 fMain）：本體不寫（golden 同）"
              : std::string(st) == "missing" ? "存檔段不寫（bHandlerModel=false：golden FormClose 不會跑）"
              : std::string(execute ? "" : "（預估）") + "本體 acarry_shims.cpp:269-293（golden cpublic.cpp:534-558 逐行）：寫 " + asShtLogPath.c_str() +
                "\\YYYYMM\\SH2_*.logs／SH1_*.logs（兩個 Shuttle memo 的內容），然後清空");
    }
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
        it.q44 = true;                                                          // AI(W906-FRW-S167) 20260928: Q44 先做過（Q44Issue ①）
        v.push_back(it);
    }
    v.push_back(Stop1203Item(execute));  v.back().q44 = true;                  // AI(W906-FRW-S167) 20260928: Q44 先做過並讀回（Q44Issue ②）
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
    v.push_back(SwOffItem(SwHeaterRelay, "SwHeaterRelay", "12157", "heat", execute));  v.back().q44 = true;   // golden :12157   AI(W906-FRW-S167) 20260928: Q44 先做過並讀回（Q44Issue ③④）
    if (execute) HeaterLog("Close", false);                                     // golden :12158 //Steven 20151123 : Log for Heater Relay
    // AI(W906-D012) 20260929 [W906] Q44 B：標籤更正（D-012 前標 stub「GATE PT-W5c」—— 已過期：St02 AI(W906-LOGOBJ-W7) 20260927 起
    //   cpublic.cpp:695-708 經 W906_HeaterLogHook 寫進 fMain->slHeaterLog）。呼叫照舊（上一行），狀態看掛鉤在不在。
    SdAdd(&v, "12158", "ui", "HeaterLog(\"Close\", false)", W906_HeaterLogHook != 0 ? "done" : "stub",
          W906_HeaterLogHook != 0
              ? "本體 cpublic.cpp:695-708 經 W906_HeaterLogHook（LogObjects.cpp:130）記進 fMain->slHeaterLog（D:\\HT9045_Log\\Heater_On_Off_LOG；"
                "W906_DestroyLogObjects 刪 list 時存檔）。W906_DestroyLogObjects（LogObjects.cpp:287）會再叫一次，同一句被本體的 OldMessage 擋掉，不重記"
              : "W906_HeaterLogHook 沒裝（log 物件沒建，LogObjects.cpp W906_CreateLogObjects 沒跑）⇒ 本體只比對字串、沒有記");
    SdAdd(&v, "12160", "ui", "#ifndef SOFT_SIMULTE TTLLog(\"Close\")", "missing", "cpublic.cpp 的 TTLLog 在 #if 0（GA1-B3）");
    // AI(W906-D012) 20260929 [W906] Q44 B：標籤更正（D-012 前標 missing「cpublic.cpp 在 #if 0」—— 已過期：本體 LogObjects.cpp:357 是活的）。
    //   這裡**不呼叫**：W906_DestroyLogObjects（LogObjects.cpp:288，wb_serve 的 server.Stop() 之後）已經照 golden 叫 ProductionLog("Close")，
    //   這裡再叫一次會記兩行 Close（ST01-E 20260929：不要叫兩次）。位置偏離（比 golden 晚）記在 LogObjects.cpp:288。
    SdAdd(&v, "12162", "ui", "ProductionLog(\"Close\")", "done",
          "在別處做：W906_DestroyLogObjects（LogObjects.cpp:288，server.Stop() 之後）叫一次，這裡不重叫（避免記兩行 Close）。"
          "本體 LogObjects.cpp:357：IniConfig.bO06SaveLogTimePeriod=true 才寫 <asProductionLogPath>\\<SocketHandlerID>_YYYYMMDD.logs");
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
              std::string(atc ? "不接：ATC 區歸別組（AI(W906-D012) 20260929 ST01-E：ATC 不在本檔呼叫；ATC6.0／3.0 的 SetATCRun／CloseSocket 有本體 ATC/ATCSystem.cpp:1920／:1986，"
                                "New ATC 的 Stop／Disconnet 只有宣告）⇒ ATC 沒有收到 STOP" : "不是 ATC6.0／3.0／New ATC：golden 同樣不做") + buf);
    }
    SdAdd(&v, "12193", "ui", "EndMainThread()", "stub",
          "不呼叫：本體 uruncontrol.cpp:229-237 直接解參考 MyThread，而 MyThread 在 wb_serve 從來沒有 new（uruncontrol.cpp:146，沒有 golden 主執行緒；tick 迴圈已停）⇒ 呼叫會當掉");   // AI(W906-D012) 20260929: 加出處
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
    // AI(W906-D012) 20260929 [W906] Q44 B：golden :12443 照接（D-012 前是 missing）。本體 cMyDB.cpp:1865（St02；本檔只呼叫，不改 cMyDB.*），
    //   原型在本檔上方（不帶預設引數，三個引數寫全，同 golden）。它走 MyDBIProcessNew：SQL 那一半退場（CSV 版）、事件紀錄 slEventLog
    //   （D:\HT9045_Log\EventLogTxt\）、SaveEventLogInfo（D:\HT9045_Log\SaveEventLog\HANDLER LOG_<ID>_YYYY_MM_DD.csv）、
    //   bO06SaveLogTimePeriod 時 ProductionLog。log 物件由 W906_DestroyLogObjects 在 server.Stop() 之後才刪 ⇒ 這裡還活著（順序對）。
    //   bHandlerModel=false 時不寫（存檔的共同前提）。
    {
        // AI(W906-D012-A8) 20260930 [W906]：golden 一個行程只跑一次 FormClose ⇒ 這一筆只寫一次。主控台那條路（按視窗 X／登出／關機、第二次
        //   Ctrl-C／Break：行程在處理函式裡結束、走不到這裡）由 tick 執行緒先寫（檔尾 (乙) A8）；先認領的寫，這裡看到已經認領就不寫第二次。
        const bool mine = execute && bHandlerModel && Q44CloseRecordClaim();
        if (mine)
            NewRecordProcess("MES2109", "Program Close", asHandlerVersion +"."+ AnsiString(SVNRevision));   //Steven 20091004   // golden :12443
        if (mine) Q44CloseRecordWritten();
        const bool dup = execute && bHandlerModel && !mine;
        const char* st = w906q44::FileStepStatus(true, bHandlerModel, execute);
        SdAdd(&v, "12443", "save", "NewRecordProcess(\"MES2109\", \"Program Close\", asHandlerVersion+\".\"+SVNRevision)", st,
              std::string(st) == "missing" ? "存檔段不寫（bHandlerModel=false：golden FormClose 不會跑）"
              : dup ? "主控台那條路（按視窗 X／登出／關機、第二次 Ctrl-C）已經寫過這一筆，不寫第二次（golden 一個行程只跑一次 FormClose）"
              : std::string(execute ? "" : "（預估）") + "cMyDB.cpp:1865 → MyDBIProcessNew：事件紀錄 D:\\HT9045_Log\\EventLogTxt\\、"
                "D:\\HT9045_Log\\SaveEventLog\\HANDLER LOG_<ID>_<日期>.csv（MES2109 Program Close " + (asHandlerVersion + "." + AnsiString(SVNRevision)).c_str() + "）");
    }
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
        if (IsHwCls(v[i].cls) && v[i].status != "done" && v[i].status != "noop" && !v[i].q44)   // AI(W906-FRW-S167) 20260928: Q44 那兩個命令另外列（q44Covered）
            w.String(v[i].what + "（" + v[i].status + "）");
    w.EndArray();
    w.Key("q44Covered").BeginArray();                                           // AI(W906-FRW-S167) 20260928 [W906] Q44：關站段前先做、而且讀回確認過的（停馬達、關加熱器繼電器）
    for (std::size_t i = 0; i < v.size(); ++i)
        if (v[i].q44) w.String(v[i].what + "（照 golden 順序這一步：" + v[i].status + "；以 Q44 的讀回為準）");
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
    if (Q44ConsoleTick(false)) return true;                                     // AI(W906-D012) 20260929 [W906] Q44 A3：主控台 Ctrl-C／Break —— 停機命令送出之後就離開主迴圈（走正常關站路）
    if (!W906_ServeQuitRequested.load()) return false;
    if (!s_bQuitSeen) { s_bQuitSeen = true; return false; }
    if (!Q44QuitTick()) return false;                                           // AI(W906-FRW-S167) 20260928 [W906] Q44：先停兩個（馬達、加熱器繼電器）並讀回確認；確認了（或強制）才離開主迴圈
    std::printf("MainClose: Exit confirmed and the Q44 stop confirmed (or forced) -> leaving the main loop (golden FormClose: save -> shutdown -> Program Close=1 -> server.Stop)\n");
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
    Q44AtShutdown();                                                            // AI(W906-FRW-S167) 20260928 [W906] Q44：主控台先記一行 Q44 做到哪（沒按 Exit 時不印）
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
    Q44MainClosing();                                                           // AI(W906-D012) 20260929 [W906] Q44 A3：主控台 X 的處理函式看到它就不催（主執行緒自己會停機、寫 Program Close=1、結束）
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

    { std::string q44; if (Q44Op(payloadJson, &q44, ok)) return q44; }         // AI(W906-FRW-S167) 20260928 [W906] Q44：{"op":"status"|"retry"|"force"}（關站中也收，本體在檔尾）；沒有 op＝原本的 step
    if (W906_ServeQuitRequested.load())                                         // AI(W906-PROD-S121) 20260926: 已經在關站 —— 不再接任何一步
        return Refuse(-1, "closing", "", "Exit 已確認，程式正在關站（先送停機命令，送出去了才結束；結果用 {\"op\":\"status\"} 問），不再接受 Exit 的任何一步", msgs, writes);   // AI(W906-FRW-S167) 20260928: 字改成 Q44 的流程

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
    w.Key("detail").String("生產資料已存。接著先停機：送出停全部馬達、關加熱器繼電器（Q44；Steven 20260929：送出去了就算停）；然後照 golden FormClose 的順序做其餘關站步驟"   // AI(W906-FRW-S167) 20260928 [W906] Q44
                           "（鎖煞車、關風扇與蜂鳴器…；途中照 golden 的位置再存兩項 :11924 SaveProductionRecord、:12051 SaveRmsInfo），"   // AI(W906-PROD-S95R) 20260926
                           "然後寫 Program Close=1、關掉網頁連線。停機命令送不出去（例如 1203 卡失聯）才不關閉，畫面列出是哪一項，可以［重試停機］或［強制關閉］；SIM 建置不設限。"
                           "下面是其餘步驟的預估（不擋關閉）；替身、空殼、沒翻的標「未停」，那些東西**沒有被停下來**（列在待辦）。"
                           "實際結果看 wb_serve 主控台 \"[Q44]\"／\"MainClose:\" 開頭的那幾行。");
    Q44WriteStep2(w);                                                           // AI(W906-FRW-S167) 20260928 [W906] Q44：shutdown.q44（頁面看到它才輪詢 status）
    WriteSdItems(w, plan, "plan");
    w.EndObject();
    w.EndObject();
    if (ok) *ok = true;
    std::printf("main.closeProgram step=%d -> production data saved (%u file group(s)); QUIT requested -- wb_serve leaves the main loop next pass and runs W906_ProdCloseShutdown\n",
                step, (unsigned)writes.size());
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

#endif  // !W906_MAINCLOSE_Q44_DECIDE_ONLY   //AI(W906-FRW-S167) 20260928 [W906] Q44：上面是原本整個檔；下面是 Q44 段（甲：純判斷、乙：機台那一半）

// ===========================================================================
//  AI(W906-FRW-S167) 20260928 [W906] Q44（最小版）：按 Exit 必須先停下才能關閉。說明見檔頭 S167 段。
//  golden 沒有這一段：golden FormClose 送完停止命令就結束，不讀回（V912 main.cpp:12153／:12157）。
//  (甲) 純判斷，只用標準庫。tests/test_mainclose_stop.cpp 先 #define W906_MAINCLOSE_Q44_DECIDE_ONLY 再 #include 本檔，
//       只編這一半（不 link god-stack）。-Wall -Wextra 乾淨。
//  狀態：kOk 已停／kAbsent 這台（這次開機）沒有／kUnverified 送了但這裡讀不回（非 1203 的卡：待辦，不擋）／
//        kWait 還在等讀回（擋，過了 kSettleMs 變 kBlocked）／kBlocked 有證據沒停或沒辦法確認（擋）。
//  判斷原則 AI(W906-FRW-Q44B) 20260929（Steven 細化 Q44）：「SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制, 機台上只要c++有回復 StopAllMotor() 是已經發送, 且加熱io也有off, 就可以當成已停機」
//    ⇒ SIM 建置：三項一律不擋（命令照送，只列出來）。機台：停止命令送到了、加熱器 Off 寫出去了就算停，不等讀回；
//    只有「送不到」（沒武裝／被拒／DRY RUN／廠商回錯／這次開機開過 1203 卡、現在失聯）才擋，等［重試停機］／［強制關閉］。
//    送不到但讀回確定停了／關了 ⇒ 也不擋（註明）；讀回只寫在說明裡。
//    （原本 20260928 追問第 2 題的做法：讀回說了算，軸還在減速或繼電器讀回 1 就等，過了 kSettleMs 就擋。）
// ===========================================================================
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace w906q44 {

enum ItemState { kOk = 0, kAbsent, kUnverified, kWait, kBlocked };
enum Verdict   { kConfirmed = 0, kPending, kBlock };

const unsigned long kFreshPolls = 2;       // 送出停止之後，監看器至少再輪詢 2 次（wb_serve kIoTickMs=200 ⇒ 約 0.4 秒）才採信讀回
const unsigned long kSettleMs   = 5000;    // 命令送不到時，等讀回的上限：過了還讀不到「已停」＝擋（AI(W906-FRW-Q44B) 20260929：送到了就不等）
const unsigned long kLingerMs   = 2000;    // 確認停了之後，最多再等 2 秒讓頁面輪詢拿到「已停」，然後離開主迴圈

struct AxisFact {
    int      index;        // 監看器的軸槽
    int      station;      // 顯示用站號（旋鈕號；不明時用 ESC 位址）
    bool     valid;        // 這一次輪詢讀到了
    unsigned state;        // Acm_AxGetState（STA_AX_*，EtherCAT/vendor/AdvMotDrv.h:793-806）
    double   cmdVel;       // Acm_AxGetCmdVelocity
    AxisFact() : index(-1), station(-1), valid(false), state(0), cmdVel(0.0) {}
};

struct WriteFact {                 // 關加熱器繼電器的那一筆 1203 輸出寫入
    bool          attempted;       // 送到了路由／命令面
    bool          accepted;        // 通過檢查（路由 CheckWrite_、命令面 ValidateDo）
    bool          issued;          // 真的呼叫了廠商 API（DRY RUN 時 false）
    bool          dry;             // 通過檢查但沒有送到卡（命令面 DRY RUN）
    unsigned long ret;             // 廠商回傳（issued 時才有意義，0＝SUCCESS）
    std::string   via;             // 走哪一條（顯示用）
    std::string   why;             // 拒絕原因／廠商錯誤
    WriteFact() : attempted(false), accepted(false), issued(false), dry(false), ret(0) {}
};

struct Facts {
    bool simBuild;                 // SOFT_SIMULTE
    // 1203 卡／監看器
    bool monPresent;               // Pci1203Monitor() != 0
    bool cardEverOpen;             // 這次開機開過卡（現在開著，或輪詢過至少一次；pollCount 在 Close 之後保留）
    bool cardUsable;               // 現在握著卡、沒有自己停用、還在輪詢（同 Pci1203IoRoute.cpp CheckWrite_ 的條件）
    std::string cardWhy;           // 停用原因／最後一個錯誤
    unsigned long pollsSinceIssue; // 送出停止之後監看器又輪詢了幾次
    // 1203 軸的停止命令（W906_Stop1203AllHook 前後的命令面計數差）
    bool stopHook, ctlArmed, ctlDry;
    unsigned long stopIssued, stopRefused, stopDry;
    int stopVendorErr;
    std::vector<AxisFact> axes;    // 只放監看器開成功的軸
    // 加熱器繼電器 SW[SwHeaterRelay]
    bool relayEnable, relayIs1203;
    int  relayIsaBase, relayRing, relayIp, relayPort;
    WriteFact relayWrite;
    int  relaySlot;                // 監看器 DO 槽；-1＝卡片的 DO 對應表沒有這個 byte
    bool relayByteValid;
    int  relayBit;                 // 讀回的那一位元（0／1）
    Facts() : simBuild(false), monPresent(false), cardEverOpen(false), cardUsable(false), pollsSinceIssue(0),
              stopHook(false), ctlArmed(false), ctlDry(false), stopIssued(0), stopRefused(0), stopDry(0), stopVendorErr(0),
              relayEnable(false), relayIs1203(false), relayIsaBase(-1), relayRing(-1), relayIp(-1), relayPort(-1),
              relaySlot(-1), relayByteValid(false), relayBit(0) {}
};

struct Item   { std::string key, what, reason; int state; Item() : state(kOk) {} };
struct Result { int verdict; std::vector<Item> items; Result() : verdict(kConfirmed) {} };

inline const char* StateName(int s)
{
    switch (s) {
        case kOk:         return "ok";
        case kAbsent:     return "absent";
        case kUnverified: return "unverified";
        case kWait:       return "wait";
        case kBlocked:    return "blocked";
    }
    return "?";
}
inline bool Holds(int s) { return s == kWait || s == kBlocked; }   // 擋關閉（還在等讀回也算）

inline const char* AxisStateName(unsigned st)
{
    switch (st & 0xFFu) {
        case 0:  return "DISABLE";   case 1:  return "READY";     case 2:  return "STOPPING";  case 3:  return "ERROR_STOP";
        case 4:  return "HOMING";    case 5:  return "PTP_MOT";   case 6:  return "CONTI_MOT"; case 7:  return "SYNC_MOT";
        case 8:  return "EXT_JOG";   case 9:  return "EXT_MPG";   case 10: return "PAUSE";     case 11: return "BUSY";
        case 12: return "WAIT_DI";   case 13: return "WAIT_PTP";  case 15: return "EXT_JOG_READY";
    }
    return "?";
}
// 停了＝沒有命令中的運動：READY／DISABLE（伺服沒開）／ERROR_STOP（出錯已停），而且命令速度 0。
//   遮罩同 WebMotorAccess.cpp:1068 IsReadyState；golden TMyEtherCatMotor::MotionDone 只認 READY（那是「動作做完」，不是「停了」）。
inline bool AxisStopped(unsigned state, double cmdVel)
{
    const unsigned s = state & 0xFFu;
    return (s == 0u || s == 1u || s == 3u) && std::fabs(cmdVel) < 1e-9;
}

inline std::string Fmt(const char* f, ...)
{
    char b[512];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(b, sizeof(b), f, ap);
    va_end(ap);
    return std::string(b);
}
inline std::string Paren(const std::string& s) { return s.empty() ? std::string() : "（" + s + "）"; }

// 強制關閉的等級門檻＝golden Exit 自己的門檻（CloseGuards：AccessLevel<LevelSet.AccessLevel[6] 算「低權限」，golden V912 main.cpp:29062）。
inline bool ForceAllowed(int accessLevel, int needLevel) { return accessLevel >= needLevel; }

inline Result Evaluate(const Facts& f, unsigned long elapsedMs, unsigned long settleMs)
{
    //  AI(W906-FRW-Q44B) 20260929：Steven 20260929 細化——送到了就算停（不等讀回），送不到才擋；SIM 一律不擋（函式最後）。
    Result r;
    const bool fresh = f.pollsSinceIssue >= kFreshPolls;
    const bool late  = elapsedMs >= settleMs;
    const std::string waited = Fmt("等了 %.1f 秒，", elapsedMs / 1000.0);

    // ---- (1) 1203 軸（移植樹：W906_Stop1203AllHook 對監看器開的每一軸減速停止＋ExtDrive 0）----------------------------
    {
        Item it;
        it.key  = "motor1203";
        it.what = "1203 軸（監看器開的每一軸：減速停止＋ExtDrive 0）";
        if (!f.monPresent || !f.cardEverOpen) {
            it.state  = kAbsent;
            it.reason = "這次開機沒有開過 1203 卡（這台沒有 1203 卡，或沒有啟用監看器）：沒有 1203 軸要停";
        } else if (!f.cardUsable) {
            it.state  = kBlocked;
            it.reason = "1203 卡失聯：這次開機開過卡，現在讀不到" + Paren(f.cardWhy) + "——停止命令送不到卡";
        } else if (f.axes.empty()) {
            it.state  = kAbsent;
            it.reason = "1203 卡開著，但監看器沒有開成功任何軸：沒有 1203 軸要停";
        } else {
            std::string cmd;
            bool cannot = true;                                                 // 停止命令到不了卡
            if (!f.stopHook)                                   cmd = "停止命令沒有送（motor.access 引擎掛鉤沒有註冊）";
            else if (!f.ctlArmed)                              cmd = "停止命令沒有送（1203 命令面沒有武裝）";
            else if (f.stopRefused > 0 || f.stopVendorErr > 0) cmd = Fmt("停止命令被拒 %lu 筆、廠商回錯 %d 筆", f.stopRefused, f.stopVendorErr);
            else if (f.ctlDry)                                 cmd = "1203 命令面是 DRY RUN：停止命令只記錄、沒有送到卡";
            else { cmd = Fmt("停止命令已送到卡（ISSUED %lu 筆）", f.stopIssued); cannot = false; }
            std::string moving, unknown;
            int nMov = 0, nUnk = 0;
            for (std::size_t i = 0; i < f.axes.size(); ++i) {
                const AxisFact& a = f.axes[i];
                if (!a.valid) {
                    unknown += (nUnk++ ? "、" : "") + Fmt("軸 %d（站 %d）", a.index, a.station);
                } else if (!AxisStopped(a.state, a.cmdVel)) {
                    moving += (nMov++ ? "、" : "") + Fmt("軸 %d（站 %d）狀態 %u %s、命令速度 %g", a.index, a.station, a.state & 0xFFu, AxisStateName(a.state), a.cmdVel);
                }
            }
            const bool allStopped = fresh && nMov == 0 && nUnk == 0;
            std::string rb;                                                     // 讀回：只當說明
            if (!fresh)          rb = "讀回還沒更新";
            else if (allStopped) rb = Fmt("%d 軸讀回都已停（READY／DISABLE／ERROR_STOP，命令速度 0）", (int)f.axes.size());
            else                 rb = (nMov ? "讀回還在減速：" + moving : std::string()) + (nMov && nUnk ? "；" : "") +
                                      (nUnk ? "讀不回軸狀態：" + unknown : std::string());
            if (!cannot) {
                it.state  = kOk;
                it.reason = cmd + "——送到了就算停（Steven 20260929）；" + rb;
            } else if (allStopped) {
                it.state  = kOk;
                it.reason = rb + "——停止命令沒送到，但讀回確定停了；" + cmd;
            } else if (fresh || late) {
                it.state  = kBlocked;
                it.reason = (fresh ? std::string() : waited) + cmd + "；" + rb;
            } else {
                it.state  = kWait;
                it.reason = cmd + "；等監看器讀回，看軸是不是已經停了";
            }
        }
        r.items.push_back(it);
    }

    // ---- (2) golden StopAllMotor()（非 1203 的馬達卡）：已照 golden 呼叫＝已發送，永遠不擋 ------------------------------
    {
        Item it;
        it.key  = "motorGolden";
        it.what = "golden StopAllMotor()（非 1203 的馬達卡：PCIL132／MN200／Galil…）";
        if (f.simBuild) {
            it.state  = kAbsent;
            it.reason = "SIM 建置：golden 每一軸 Motor->Enable=false（cinitial.cpp:4011-4034），不下命令，也沒有真的馬達可停";
        } else {
            it.state  = kOk;
            it.reason = "已照 golden 呼叫 StopAllMotor()＝已發送（Steven 20260929：發送了就算停）；非 1203 的馬達卡沒有讀回";
        }
        r.items.push_back(it);
    }

    // ---- (3) 加熱器繼電器 SW[SwHeaterRelay].Off()（golden :12157）---------------------------------------------------
    {
        Item it;
        it.key  = "heaterRelay";
        it.what = "加熱器繼電器 SW[SwHeaterRelay].Off()（golden V912 main.cpp:12157）";
        const std::string at = Fmt("（Lane %d、IP %d、Port %d）", f.relayRing, f.relayIp, f.relayPort);
        if (!f.relayEnable) {
            it.state  = kAbsent;
            it.reason = "IO_Table 沒有這一點（Enable=0）：golden 的 Off() 直接 return，golden 同樣不關";
        } else if (!f.relayIs1203) {
            it.state  = kOk;
            it.reason = Fmt("非 1203 的 IO 點（ISABase=%d）", f.relayIsaBase) + at +
                        (f.simBuild ? "：SIM 建置 golden Off() 只寫到模擬值"
                                    : "：已照 golden 送 Off()＝加熱 IO 已 off（Steven 20260929）；MN200／MNet／ISA 沒有讀回");
        } else if (!f.monPresent || !f.cardEverOpen) {
            it.state  = kAbsent;
            it.reason = "這次開機沒有開過 1203 卡：沒有真的 1203 輸出可以關（golden Off() 已照呼叫）" + at;
        } else if (!f.cardUsable) {
            it.state  = kBlocked;
            it.reason = "1203 卡失聯：這次開機開過卡，現在讀不到" + Paren(f.cardWhy) + "——關閉命令送不到卡" + at;
        } else {
            const WriteFact& w = f.relayWrite;
            std::string wr;
            bool noEffect = true;                                               // 寫 0 到不了卡
            if (!w.attempted)               wr = "關閉命令沒有送出" + Paren(w.why);
            else if (!w.accepted)           wr = w.via + " 拒寫：" + w.why;
            else if (w.dry)                 wr = w.via + " 是 DRY RUN：只記錄、沒有送到卡";
            else if (w.issued && w.ret != 0) wr = w.via + Fmt(" 送了，廠商回 0x%08lX", w.ret) + Paren(w.why);
            else { wr = w.via + " 已送到卡（寫 0）"; noEffect = false; }
            std::string rb;                                                     // 讀回：只當說明
            bool rbOff = false, rbKnown = true;
            if (f.relaySlot < 0)
                rb = Fmt("卡片的 DO 對應表裡沒有 ring %d 站 %d 的第 %d 個 byte，讀不回（IO_Table 的 Lane／IP／Port 對不上卡片）",
                         f.relayRing, f.relayIp, f.relayPort >= 0 ? f.relayPort / 8 : -1);
            else if (!fresh)            { rb = "讀回還沒更新"; rbKnown = false; }
            else if (!f.relayByteValid) rb = "DO byte 讀不回來（valid=false）";
            else if (f.relayBit == 0)   { rb = "讀回 0（已關）"; rbOff = true; }
            else                        rb = "讀回仍是 1";
            if (!noEffect) {
                it.state  = kOk;
                it.reason = wr + "——加熱 IO 已 off（Steven 20260929）；" + rb + at;
            } else if (rbOff) {
                it.state  = kOk;
                it.reason = rb + at + "——關閉命令沒送到，但讀回確定已關；" + wr;
            } else if (rbKnown || late) {
                it.state  = kBlocked;
                it.reason = (rbKnown ? std::string() : waited) + wr + "；" + rb + at;
            } else {
                it.state  = kWait;
                it.reason = wr + "；等監看器讀回，看是不是已經關了" + at;
            }
        }
        r.items.push_back(it);
    }

    // ---- SIM 建置：關閉不設限（Steven 20260929「SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制」）----------------
    if (f.simBuild) {
        for (std::size_t i = 0; i < r.items.size(); ++i) {
            if (!Holds(r.items[i].state)) continue;
            r.items[i].state  = kUnverified;
            r.items[i].reason = "SIM 建置，關閉不設限（Steven 20260929：馬達不會真的動作）；" + r.items[i].reason;
        }
    }

    for (std::size_t i = 0; i < r.items.size(); ++i) {
        if (r.items[i].state == kBlocked) r.verdict = kBlock;
        else if (r.items[i].state == kWait && r.verdict == kConfirmed) r.verdict = kPending;
    }
    return r;
}

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 A2：關站中只收「關」與「停」（待辦 D-012；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md A2）。
//  golden：TfMain::FormClose（V912 main.cpp:11852-12478）在 VCL 主執行緒上一口氣跑完，期間訊息迴圈不轉 ⇒ 別的按鈕都按不到。
//  移植樹的關站在 wb_serve 主迴圈裡等（連線還在，頁面要拿得到結果），所以另外擋：從 Exit 第二框確認（W906_ServeQuitRequested）
//  或主控台 Ctrl-C／關視窗之後，到主迴圈結束為止，wb_serve 只放行下面這張白名單，其餘一律回 ClosingRefusal()、不執行。
//  白名單以外的新指令預設擋（加指令時不用記得來這裡擋）。
//    sub：motor.access 的 action／observer.get 的 act（value 的 JSON；沒帶＝""）；start：motor.access 的 params.start（-1＝沒帶或不是布林）。
// ===========================================================================
inline bool CmdAllowedWhileClosing(const std::string& cmd, const std::string& sub, int start, bool lightScaleActive = false)   //AI(W906-D012-M2) 20260929: + lightScaleActive（St02-E2 review M2）
{
    // (1) 關站本身：{"op":"status"|"retry"|"force"}。step 0／1／2 照樣進 W906_Main_CloseProgramOp，由它回 guard=closing（頁面靠這個接回蓋層）。
    if (cmd == "act.main.closeProgram") return true;
    // (2) 停
    if (cmd == "motor.stop") return true;                                       // 本身只收 action=stop（WebMotorAccessLive.cpp:985）
    if (cmd == "pci1203.ax.stop" || cmd == "pci1203.ax.emgStop") return true;   // 1203 頁的軸停止／緊急停止
    if (cmd == "pause.run") return true;                                        // 主畫面 PAUSE：TfMainWeb::PauseFromWeb（WebStart.cpp:3810，golden TfMain::Pause main.cpp:6325）＝SoftStop=true＋StopAllMotor
    if (cmd == "motor.access") {
        if (sub == "stop") return true;
        if (sub == "lightScale" && lightScaleActive) return true;   //AI(W906-D012-M2) 20260929: St02-E2 review M2 -- Light Scale 在掃描時，網頁唯一能停它的就是這一條（golden BitBtn2Click 切掉 Timer2，uMotorTest.cpp:2096-2099；Q44 的停機不碰 Light Scale）⇒ 掃描中放行；沒在掃描時這一條＝開始掃描（會歸零、移動手臂）⇒ 照樣擋。判斷同 WebMotorAccess.cpp MotorTestStopDirection（lightScale → g_ls.timer）
        if ((sub == "loopMove" || sub == "home") && start == 0) return true;    // Motor Test 放開 LoopMove／HOME（WebMotorAccess.cpp:3562 MotorTestStopDirection）
        return false;                                                           // jog／move／home／servoToggle／motorPowerToggle／teach*／set*…
    }
    // (3) 基礎設施：不碰機台
    if (cmd == "sys.ping" || cmd == "cfg.resync" || cmd == "log.event" || cmd == "ui.windows.put" || cmd == "stream.resync") return true;
    // (4) 唯讀（WebBridgeServer 免權杖的三個讀取；observer.get 只放讀取型 act，名單同 WebCmdGuard.cpp:112-113 kObserverActs）
    if (cmd == "contactct.get" || cmd == "counterclear.get") return true;
    if (cmd == "observer.get") {
        static const char* const kRead[] = { "", "open", "timer", "tab", "rowNo", "form", "year", "month", "file", "filter", "query",
                                             "ccKinds", "ccKindsForm", "ccHistory", "ccHistoryForm", 0 };
        for (int i = 0; kRead[i]; ++i) if (sub == kRead[i]) return true;
        return false;                                                           // yieldSite／yieldMax／yieldMin／yieldClear 改記憶體
    }
    // (5) 回答 golden 的框（阻塞框的等待迴圈自己收；走到主分派＝沒有人在問，照舊回「沒有在問」）
    if (cmd == "modal.answer" || cmd == "dialog.response" || cmd == "dialog.notifyAck") return true;   // AI(W906-J5-ACK) 20260930: closing a kCode==0 notice (INBOX 119) is an answer too
    // (6) 登入：［強制關閉］要 Exit 自己的等級（LevelSet.AccessLevel[6]），要能換人登入
    if (cmd == "auth.login" || cmd == "auth.logout" || cmd == "auth.mode" || cmd == "auth.select" || cmd == "dialog.auth") return true;
    return false;
}

// 擋下時回給頁面的字（開頭固定 "closing:"，頁面與探針用它認）。byConsole＝主控台 Ctrl-C／關視窗那條，不是 Exit 鈕。
inline std::string ClosingRefusal(const std::string& cmd, bool byConsole)
{
    return std::string("closing: ") + (byConsole ? "console Ctrl-C / window close" : "Exit") +
           " is stopping the machine -- until wb_serve ends only close / stop / read-only commands are accepted; '" + cmd + "' not run"
           "（關站中：" + (byConsole ? "主控台 Ctrl-C／關視窗" : "Exit") + " 正在停機，程式結束前只收關閉、停止與唯讀指令；這一條沒有執行）";
}

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 A3：主控台 Ctrl-C／Ctrl-Break／按主控台視窗 X／登出／關機（W906_ConsoleCtrl，tools/wb_serve.cpp）。
//  golden：程式結束一定走 TfMain::FormClose（先停機，最後 :12444 寫 Program Close=1）。
//  事件號碼＝wincon.h：CTRL_C_EVENT 0、CTRL_BREAK_EVENT 1、CTRL_CLOSE_EVENT 2、CTRL_LOGOFF_EVENT 5、CTRL_SHUTDOWN_EVENT 6。
//  nth＝這是這次開機第幾個主控台事件（1 起算）。
//    kConLetMainClose  第一次 Ctrl-C／Break：處理函式回 TRUE（行程不結束）；主迴圈先送停機命令、再走正常關站路
//                      （存檔 → golden 關站段 → Program Close=1 → server.Stop()），跟 --seconds 到期同一條
//    kConExitNow       第二次 Ctrl-C／Break：照今天（寫 Program Close=1、回 FALSE 立刻結束），給主迴圈卡住時用
//    kConStopThenExit  X／登出／關機：Windows 只給約 5 秒 ⇒ 等主迴圈把停機命令送出去（不等讀回，最多 kConsoleHardWaitMs），
//                      然後寫 Program Close=1、回 FALSE（行程結束）；不存生產資料（跟今天一樣）
//    ⛔ AI(W906-D012-A3W) 20260930 [W906] 更正（ST01-E 20260930 裁定）：登出／關機（5、6）分出去成 kConSessionEnd —— 只送停機命令，
//      **不寫** MES2109、**不寫** Program Close=1（golden 登出／關機不跑 FormClose，見下面 A3W 段）；kConStopThenExit 只剩按視窗 X（2），照舊。
//    kConSessionEnd    登出／關機（主控台 CTRL_LOGOFF／SHUTDOWN，或隱藏視窗的 WM_ENDSESSION）：等主迴圈把停機命令送出去（最多 kConsoleHardWaitMs），
//                      不寫 MES2109、不寫 Program Close=1（留 0＝golden：下次開機看到異常關機、要手動清料），然後結束行程
// ===========================================================================
enum ConsoleAct { kConIgnore = 0, kConLetMainClose, kConExitNow, kConStopThenExit, kConSessionEnd };   // AI(W906-D012-A3W) 20260930: + kConSessionEnd（加在最後，其餘值不變）
const unsigned long kConsoleHardWaitMs = 3500;   // CTRL_CLOSE_EVENT 5 秒就被系統結束：留 1.5 秒給寫 Program Close=1 與結束

inline int ConsoleDecide(unsigned long ev, int nth)
{
    if (ev == 0 || ev == 1) return nth <= 1 ? kConLetMainClose : kConExitNow;
    if (ev == 2) return kConStopThenExit;
    if (ev == 5 || ev == 6) return kConSessionEnd;                              // AI(W906-D012-A3W) 20260930 [W906]：原本同 2（kConStopThenExit）
    return kConIgnore;
}
inline const char* ConsoleEventName(unsigned long ev)
{
    switch (ev) {
        case 0: return "Ctrl-C";          case 1: return "Ctrl-Break";      case 2: return "console window closed";
        case 5: return "logoff";          case 6: return "shutdown";
    }
    return "?";
}
// X／登出／關機的等待什麼時候結束：停機命令送出去了（而且主執行緒沒有在跑關站路），或主執行緒已經在跑正常關站路
//   （它自己會停機、寫 Program Close=1 並結束行程，這裡只等到期限），或到期限。
//   AI(W906-D012-A8) 20260930 [W906]：再加一個條件 recordSettled＝golden 關站的事件紀錄（main.cpp:12443 MES2109）寫完了、或這台照 golden
//   不寫（CloseRecordSettled）——tick 執行緒送完停機命令緊接著寫它，處理函式等它寫完才結束行程（寫到一半被結束會截斷檔案）。預設 true＝舊行為。
inline bool ConsoleHardWaitDone(bool stopIssued, bool mainClosing, unsigned long elapsedMs, unsigned long limitMs, bool recordSettled = true)
{
    if (elapsedMs >= limitMs) return true;
    return stopIssued && recordSettled && !mainClosing;
}

// ===========================================================================
//  AI(W906-D012-A8) 20260930 [W906] 待辦 D-012 A8（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md A 第 8 條）：
//  golden 關站的事件紀錄＝TfMain::FormClose 的 NewRecordProcess("MES2109", "Program Close", asHandlerVersion+"."+SVNRevision)
//  （V912 main.cpp:12443；bHandlerModel=false 時 golden FormShow 就結束程式、FormClose 不跑 ⇒ 不寫）。golden 關程式（Exit 鈕）走 FormClose
//  ⇒ 每個行程寫一次、同樣的字。（⛔ AI(W906-D012-A3W) 20260930 更正：原本這裡寫「VCL 登出／關機也是：WM_QUERYENDSESSION → 關主視窗 → FormClose」—— 不對，
//  見下面 A3W 段：VCL 6 登出／關機不呼叫 FormClose，golden 那時不寫這一筆、不寫 Program Close=1。）移植樹：
//    正常關站路（Exit 確認、Exit［強制關閉］、--seconds 到期、第一次 Ctrl-C／Break）＝ShutdownSequence(true) 在 golden 的位置寫（已經是）；
//    行程在主控台處理函式裡結束的兩條（kConExitNow 第二次 Ctrl-C／Break、kConStopThenExit 按視窗 X／登出／關機）走不到那裡 ⇒
//    處理函式請 tick 執行緒先寫（NewRecordProcess 不是執行緒安全，cMyDB.cpp:1891-1893），寫完才結束行程。
//    ⛔ AI(W906-D012-A3W) 20260930 [W906] 更正（ST01-E 20260930 裁定）：登出／關機不寫了 —— 分出去成 kConSessionEnd（下面 A3W 段），
//      golden 登出／關機不跑 FormClose、不寫這一筆；現在只有 X（kConStopThenExit）與第二次 Ctrl-C／Break（kConExitNow）由 tick 執行緒先寫，
//      這兩條 golden 沒有對應，是移植樹自己的選擇（照 7e60e445 不變）。ConsoleNeedsCloseRecord 的式子沒變（它本來就只列這兩個 act）。
//  「強制關閉」沒有 golden 的字（golden 沒有這個按鈕）⇒ 事件紀錄照 golden 只記 MES2109「Program Close」；「強制、哪幾項沒停」留在主控台與
//  D:\HT9045\Error\BootLog.txt 那一行（W906 Q44 FORCE CLOSE …，Q44Op）。
// ===========================================================================
// 這個主控台事件之後行程會在處理函式裡結束（不走正常關站路）⇒ 要請 tick 執行緒先寫 golden 的那一筆。
inline bool ConsoleNeedsCloseRecord(int act) { return act == kConExitNow || act == kConStopThenExit; }
// tick 執行緒這一拍要不要寫：有人要、還沒人認領、主執行緒不在正常關站路上（在的話它自己在 golden 的位置寫）、這台照 golden 會寫。
inline bool ConsoleTickWritesCloseRecord(bool wanted, bool claimed, bool mainClosing, bool handlerModel)
{
    return wanted && !claimed && !mainClosing && handlerModel;
}
// 處理函式看的「紀錄這件事結束了沒」：寫完了，或這台照 golden 不寫（bHandlerModel=false）。
inline bool CloseRecordSettled(bool written, bool handlerModel) { return written || !handlerModel; }
// 第二次 Ctrl-C／Break（主迴圈卡住時用）：最多等這麼久讓 tick 執行緒（主迴圈底或阻塞框的等待，每圈約 50～140 ms）寫那一筆；
//   卡死的話到期就照舊結束（紀錄沒寫，主控台印一行）。
const unsigned long kConsoleExitNowWaitMs = 500;
inline bool ConsoleExitNowWaitDone(bool recordSettled, unsigned long elapsedMs, unsigned long limitMs)
{
    return recordSettled || elapsedMs >= limitMs;
}

// ===========================================================================
//  AI(W906-D012-A3W) 20260930 [W906] 待辦 D-012 A3 的登出／關機（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md A 第 3 條）。
//  ① 為什麼要隱藏視窗：微軟 SetConsoleCtrlHandler 備註（learn.microsoft.com/windows/console/setconsolectrlhandler，Remarks「Windows 7, Windows 8,
//     Windows 8.1 and Windows 10」段）——載入 gdi32.dll／user32.dll 的主控台程式，處理函式收不到 CTRL_LOGOFF_EVENT／CTRL_SHUTDOWN_EVENT
//     （系統把它當視窗程式），要收得自己建一個隱藏視窗（CreateWindowEx，dwExStyle=0）接 WM_QUERYENDSESSION／WM_ENDSESSION。HandlerRoutine 頁
//     （learn.microsoft.com/windows/console/handlerroutine，參數表）另說這兩個事件「received only by services」，互動程式在那之前就被結束了。
//     wb_serve 會載入 user32 ⇒ wb_serve.cpp W906_ConsoleCtrl 的 5／6 分支大概跑不到。隱藏視窗在 (乙) 檔尾 W906_SessionEndWatchStart。
//  ② golden 登出／關機做什麼（BCB6 VCL 6 原始碼 D:\ProgramFiles\Borland\CBuilder6\Source\vcl\forms.pas）：
//     WM_QUERYENDSESSION → TCustomForm.WMQueryEndSession＝「CloseQuery and CallTerminateProcs」（:4104-4107；TfMain 沒有 OnCloseQuery ⇒
//     CloseQuery :4543-4554 回 True；TfMain 的訊息表 main.h:1206-1209 只接 WM_COPYDATA／WM_HOTKEY）⇒ 回 TRUE，**不呼叫** Close／FormClose。
//     WM_ENDSESSION → TApplication.WndProc 只設 FTerminate（:6420）；Application->Run 回來（golden HT9045.cpp:285）之後 DoneApplication 拆表單
//     ＝TfMain::FormDestroy（main.cpp:12480，不停馬達），不是 FormClose。
//     ⇒ golden 登出／關機：不 StopAllMotor（:12153）、不關加熱器繼電器（:12157）、不寫 MES2109（:12443）、不寫 Program Close=1（:12444，
//       留 0 ⇒ 下次開機看到異常關機、要手動清料）。
//  ③ 移植樹的規則（ST01-E 20260930 裁定：BCB 清楚的照 BCB；停機是 Q44-6 選 a「停輸出」，V906 的安全補強，golden 沒有）：
//     登出／關機（不管從主控台 5／6 還是隱藏視窗的 WM_ENDSESSION 進來）＝kConSessionEnd：**送停機命令**（tick 執行緒，同 Exit 的 Q44Issue）、
//     **不寫** MES2109、**不寫** Program Close=1（跟 golden 一樣留 0；這條路也不存 lastdata.dat）。
//     按主控台視窗 X（2）、第二次 Ctrl-C／Break：golden 沒有對應 ⇒ 照今天（停機＋MES2109（A8）＋Program Close=1），是移植樹自己的選擇。
//  ④ 視窗訊息：
//     WM_QUERYENDSESSION（0x0011）：立刻回 TRUE、不做別的（golden 同 ②）。微軟 WM_QUERYENDSESSION 備註「Each application should return TRUE or
//       FALSE immediately upon receiving this message, and defer any cleanup operations until it receives the WM_ENDSESSION message」；之後也可能
//       來 WM_ENDSESSION(FALSE)（別的程式擋下、不登出了）——先停了機，程式就卡在「關站中」回不去。沒有可見視窗的程式本來就擋不了關機
//       （微軟 Shutdown Changes for Windows Vista 表「Canceling shutdown」列）。
//     WM_ENDSESSION（0x0016）wParam=TRUE：lParam 帶 ENDSESSION_LOGOFF（0x80000000；微軟：bit mask、「do not test for equality」）＝登出＝5；
//       其餘（0＝關機或重開機、ENDSESSION_CLOSEAPP 0x1＝Restart Manager 要關這個程式、ENDSESSION_CRITICAL 0x40000000＝強制）＝關機＝6。
//       等待放在這裡：微軟 WM_ENDSESSION wParam「the session can end any time after all applications have returned from processing this message」；
//       沒有可見視窗的程式 5 秒不回就被結束（同上 Vista 表）⇒ kConsoleHardWaitMs 3.5 秒上限在 5 秒內。
//     WM_ENDSESSION wParam=FALSE：不登出了 ⇒ 什麼都不做。
//  ⑤ 兩條路都到（主控台處理函式那條系統執行緒收到 CTRL_CLOSE，隱藏視窗那條執行緒收到 WM_ENDSESSION；或按 X 等待中又連按兩次 Ctrl-C）：
//     停機命令（s_conStopIssued）與 MES2109（Q44CloseRecordClaim）本來就只做一次，但「寫 Program Close=1、結束行程」原本兩條各做一次 ——
//     兩條執行緒各寫一次 D:\HT9045\system\Gerneral.ini，先結束行程的那條會把另一條寫到一半的檔砍斷。⇒ 行程要在處理函式裡結束的三種
//     （kConExitNow、kConStopThenExit、kConSessionEnd）一個行程只有一個 owner，先到先得（(乙) s_conExitOwner 的 compare_exchange）：
//     owner 照自己的規則做（ConsolePlanFor：X／第二次 Ctrl-C 寫 MES2109＋Program Close=1；登出／關機什麼都不寫），然後結束行程；
//     後到的 follower 只補「送停機命令」這個旗標（冪等），不要 MES2109、不寫 Program Close、不結束行程，等 owner 結束行程（被 ExitProcess
//     一起結束；HandlerRoutine 頁 Remarks：「it is possible that the handler function will be terminated by another thread in the process」），
//     最多 kConsoleFollowerWaitMs，到了回 true（處理函式回 TRUE、不寫）。所以 CTRL_CLOSE 先認領＝照 X 寫 Program Close=1；
//     登出／關機先認領＝什麼都不寫。第一次 Ctrl-C／Break（kConLetMainClose）不認領：它走正常關站路，由主執行緒在 golden 的位置寫。
// ===========================================================================
const unsigned      kWmQueryEndSession  = 0x0011;          // WinUser.h WM_QUERYENDSESSION（(乙) static_assert 對過標頭）
const unsigned      kWmEndSession       = 0x0016;          // WinUser.h WM_ENDSESSION
const unsigned long kEndSessionLogoff   = 0x80000000UL;    // WinUser.h ENDSESSION_LOGOFF
const unsigned long kEndSessionCloseApp = 0x00000001UL;    // WinUser.h ENDSESSION_CLOSEAPP（MinGW.org 6.3 的標頭沒有）
const unsigned long kEndSessionCritical = 0x40000000UL;    // WinUser.h ENDSESSION_CRITICAL（同上）
// WM_QUERYENDSESSION 的回答：永遠 TRUE（不擋，golden 同）
inline bool QueryEndSessionAnswer() { return true; }
// WM_ENDSESSION 當成哪個主控台事件（wincon.h 號碼，交給 ConsoleDecide）；-1＝不動作（wParam=FALSE：不登出了）
inline long SessionEndConsoleEvent(bool ending, unsigned long lParam)
{
    if (!ending) return -1;
    return (lParam & kEndSessionLogoff) ? 5L : 6L;
}
inline const char* SessionEndName(unsigned long ev) { return ev == 5 ? "Windows logoff (WM_ENDSESSION)" : "Windows shutdown (WM_ENDSESSION)"; }

enum ConsoleExitRole { kExitNotMine = 0, kExitOwner, kExitFollower };
inline bool ConsoleEndsProcess(int act) { return act == kConExitNow || act == kConStopThenExit || act == kConSessionEnd; }
inline int ConsoleExitRoleFor(int act, bool alreadyClaimed)
{
    if (!ConsoleEndsProcess(act)) return kExitNotMine;
    return alreadyClaimed ? kExitFollower : kExitOwner;
}
// 事件（act）＋角色（role）⇒ 這一條做什麼。stop＝請 tick 執行緒送停機命令；record＝請 tick 執行緒寫 golden 的 MES2109；
//   programClose＝處理函式寫 Program Close=1（wb_serve.cpp W906_ConsoleCtrl：W906_ConsoleQuit 回 false 才寫）；endsProcess＝這一條結束行程。
struct ConsolePlan { bool stop; bool record; bool programClose; bool endsProcess; };
inline ConsolePlan ConsolePlanFor(int act, int role)
{
    ConsolePlan p = { false, false, false, false };
    if (role == kExitFollower) { p.stop = (act == kConStopThenExit || act == kConSessionEnd); return p; }
    switch (act) {
        case kConLetMainClose: p.stop = true; break;                                                      // 主執行緒走正常關站路（它在 golden 的位置寫兩樣）
        case kConExitNow:      p.record = true; p.programClose = true; p.endsProcess = true; break;       // 停機命令第一個事件就要過了
        case kConStopThenExit: p.stop = true; p.record = true; p.programClose = true; p.endsProcess = true; break;
        case kConSessionEnd:   p.stop = true; p.endsProcess = true; break;                                // golden 登出／關機不跑 FormClose ⇒ 兩樣都不寫
        default: break;
    }
    return p;
}
// 隱藏視窗那條執行緒做完 owner 的等待之後要不要自己 ExitProcess（同主控台預設處理函式）：是 owner、而且主執行緒不在正常關站路上
//   （在的話它自己寫完、自己結束行程；這裡回 0，讓系統要結束時再結束 —— 不在它寫檔寫到一半時砍它）。
inline bool SessionEndWindowExits(int role, bool mainClosing) { return role == kExitOwner && !mainClosing; }
const unsigned long kConsoleFollowerWaitMs = 4500;   // 要 > owner 的 kConsoleHardWaitMs（3.5 秒＋寫檔）、< 5000（系統 5 秒就結束行程）

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 B：golden 關站段補接的步驟的狀態（純判斷；呼叫本身在 ShutdownSequence，只編進 wb_serve、
//    ctest 不呼叫 —— 那三個會寫 D:\HT9045_Log\… 與事件紀錄）。
//  寫檔的三步（:12002-12008 條碼顆數清除、:12149 OutShuttleLog(true)、:12443 NewRecordProcess("MES2109", …)）：
//    applies＝golden 自己的條件成立（不成立＝golden 同樣不做 ⇒ noop）；handlerModel＝bHandlerModel（false：golden FormShow 就結束、
//    FormClose 不跑 ⇒ 關站時不寫，跟本檔其他存檔同一個前提 ⇒ missing）；execute＝true 照做／false 只預估（預估也回 done）。
// ===========================================================================
inline const char* FileStepStatus(bool applies, bool handlerModel, bool execute)
{
    if (!applies) return "noop";
    if (execute && !handlerModel) return "missing";
    return "done";
}
// golden :12002-12005：(BAR_CODE_INSTALL==ebctInShtIntel || ==ebctEtherNetCCD || ==ebcUseOCR) && TestIF_File.bEnableBarCode。
//   三個列舉值由呼叫端給（MachineType.h:639-642），本段只用標準庫。
inline bool BarcodeClearApplies(int barCodeInstall, int kInShtIntel, int kEtherNetCCD, int kUseOCR, bool enableBarCode)
{
    return (barCodeInstall == kInShtIntel || barCodeInstall == kEtherNetCCD || barCodeInstall == kUseOCR) && enableBarCode;
}
// :12047-12048 Index kit 吸嘴 Normal()：nIo＝兩個吸嘴啟用的 IO 點數（OnEnable／OffEnable，0～4），n1203＝其中 1203 點數，
//   ioGroup＝1203 快照判斷的結果（ClassifyIoGroup，同 IndexMotorBreakerOFF）。
//   沒有點 ⇒ noop（Normal() 只改旗標，golden 同）；有非 1203 點而快照說 done ⇒ unverified（MN200／MNet 看不到有沒有寫到卡）。
inline std::string KitSuckStepStatus(int nIo, int n1203, const std::string& ioGroup)
{
    if (nIo <= 0) return "noop";
    if (n1203 < nIo && ioGroup == "done") return "unverified";
    return ioGroup;
}

}  // namespace w906q44

#ifndef W906_MAINCLOSE_Q44_DECIDE_ONLY
// ===========================================================================
//  AI(W906-FRW-S167) 20260928 [W906] (乙) Q44 機台那一半（只編進 wb_serve）。全部在 tick 執行緒（wb_serve 主迴圈；
//  W906_ServeQuitDue 與 act.main.closeProgram 的分派都在這條執行緒；廠商 API 也只能在這條執行緒呼叫，Pci1203Control.h:800-803）。
//  流程：Exit 第二框確認（step 2）→ W906_ServeQuitDue 晚一圈 → Q44QuitTick：
//    waiting → 送兩個命令（Q44Issue）→ stopping：每一圈讀回判一次（Evaluate）
//      → confirmed：最多再等 kLingerMs 讓頁面拿到結果 → 離開主迴圈 → 照今天：W906_ProdCloseSave（已存過不重存）→
//                   W906_ProdCloseShutdown（golden 關站段全部，含再停一次）→ Program Close=1 → server.Stop()
//      → blocked：留在主迴圈（程式照常收網頁指令），等 {"op":"retry"}（再送一次兩個命令）或 {"op":"force","confirm":true}。
//  --seconds 到期、Ctrl-C：跟今天一樣（不等 Q44；W906_ProdCloseShutdown 開頭印 Q44 做到哪），不會卡住。
// ===========================================================================
#include "EtherCAT/Pci1203Monitor.h"     // AI(W906-FRW-S167) 20260928: 只讀 card()／axis()／do_()（讀回）；EastSun 的檔不改
#include "WebMotorAccess.h"               //AI(W906-D012-M2) 20260929: MotorAccessLightScale（Light Scale 掃描中才放行 lightScale，M2）；Jimmy 的檔不改
#include <chrono>

extern int  (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos);                                   // canary_support.h:129（定義 canary_support.cpp:87）
extern void (*W906_ShowMyMessage_Hook)(const char* S1, const char* S2);                                            // canary_support.h:176（定義 canary_support.cpp:151）
extern void (*W906_ShowMyMessageEx_Hook)(const char* S1, const char* S2, const char* S3, bool Ok, bool bServoOff);  // 定義 canary_support.cpp:154（wb_serve 的網頁訊息框宿主裝）
extern int  (*W906_ShowMyMessageBoxYesNo_Hook)(const char* S1, const char* S2, const char* S3);                    // canary_support.h:215（定義 canary_support.cpp:198）
extern void (*W906_MotorAllBtnUpHook)(const char* why);                                                              // csystem.h:424（定義 csystem.cpp:30043；wb_serve 由 WebMotorAccessLive.cpp:1198 註冊）
void WriteBootLog(AnsiString sStep);                                                                                  // Public/cBootLog.h（本體 Public/cBootLog.cpp:76；wb_serve.cpp:5965 同樣這樣宣告）

namespace {

enum Q44Phase { kQ44Idle = 0, kQ44Stopping, kQ44Blocked, kQ44Retry, kQ44Confirmed, kQ44Forced };
Q44Phase           s_q44         = kQ44Idle;
int                s_q44Attempt  = 0;
unsigned long      s_q44Poll0    = 0;                                           // 送出停止時監看器的 pollCount
bool               s_q44Served   = false;                                       // 帶「已停／強制」的回覆已經交給 server.CompleteCommand
bool               s_q44Final    = false;                                       // 那一圈過了（回覆送出去了）⇒ 下一次詢問離開主迴圈
std::chrono::steady_clock::time_point s_q44T0, s_q44DoneAt;
w906q44::Result    s_q44Last;
w906q44::WriteFact s_q44Relay;
unsigned long      s_q44StopIssued = 0, s_q44StopRefused = 0, s_q44StopDry = 0;
int                s_q44StopVendorErr = 0;
std::string        s_q44ForceText;

// AI(W906-D012) 20260929 [W906] Q44 A3：主控台事件。W906_ConsoleQuit 跑在系統另開的執行緒（見檔尾），只碰這幾個 atomic；
//   停機命令一律由 tick 執行緒送（廠商 API 只能在那條執行緒，Pci1203Control.h:800-803）。
std::atomic<int>   s_conCount(0);                                               // 這次開機收到幾個主控台事件
std::atomic<long>  s_conFirstEv(-1);                                            // 第一個事件（wincon.h 號碼）
std::atomic<bool>  s_conStopWanted(false);                                      // 處理函式要 tick 執行緒送停機命令
std::atomic<bool>  s_conStopIssued(false);                                      // tick 執行緒送出去了
std::atomic<bool>  s_conLeave(false);                                           // Ctrl-C／Break：送完就離開主迴圈（正常關站路）
std::atomic<bool>  s_mainClosing(false);                                        // 主執行緒已經離開主迴圈、在跑正常關站路（W906_ProdCloseSave 起）
std::atomic<bool>  s_conRecordWanted(false);                                    // AI(W906-D012-A8) 20260930 [W906]：處理函式要 tick 執行緒先寫 golden 的 MES2109（行程要在處理函式裡結束）
std::atomic<bool>  s_closeRecClaimed(false);                                    // AI(W906-D012-A8)：golden :12443 那一筆本行程已經有人認領（一次）
std::atomic<bool>  s_closeRecWritten(false);                                    // AI(W906-D012-A8)：寫完了（處理函式等這個）
std::atomic<long>  s_conExitOwner(-1);                                          // AI(W906-D012-A3W) 20260930 [W906]：結束行程的 owner（-1＝還沒有；Q44ExitToken：事件號碼＋0x100＝隱藏視窗；(甲) A3W ⑤）
bool               s_conModalNoted = false;                                     // tick 執行緒：「框開著、離不開主迴圈」只印一次
unsigned long      s_q44Refused = 0;                                            // A2：關站中擋掉幾條（主控台只印前 30 條，之後每 100 條一行）

unsigned long Q44Ms(std::chrono::steady_clock::time_point from)
{
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - from).count();
}

const char* Q44PhaseName(Q44Phase p)
{
    switch (p) {
        case kQ44Idle:      return "waiting";
        case kQ44Stopping:  return "stopping";
        case kQ44Retry:     return "stopping";                                  // 已收到重試，下一圈重送
        case kQ44Blocked:   return "blocked";
        case kQ44Confirmed: return "confirmed";
        case kQ44Forced:    return "forced";
    }
    return "?";
}

// 送兩個命令的時候不讓任何 golden 訊息框把 tick 執行緒卡在「等網頁回答」（例：Galil 命令失敗跳 ShowErrorMessage，
//   而 Exit 蓋層蓋在告警框上面）—— 跟今天的關站段同一個條件（wb_serve.cpp:5957-5959 在迴圈外先卸下）。送完原樣裝回。
struct Q44QuietScope {
    int  (*e)(const char*, int, int);
    void (*m)(const char*, const char*);
    void (*x)(const char*, const char*, const char*, bool, bool);
    int  (*y)(const char*, const char*, const char*);
    Q44QuietScope() : e(W906_ShowErrorMessage_Hook), m(W906_ShowMyMessage_Hook), x(W906_ShowMyMessageEx_Hook), y(W906_ShowMyMessageBoxYesNo_Hook)
    { W906_ShowErrorMessage_Hook = 0; W906_ShowMyMessage_Hook = 0; W906_ShowMyMessageEx_Hook = 0; W906_ShowMyMessageBoxYesNo_Hook = 0; }
    ~Q44QuietScope()
    { W906_ShowErrorMessage_Hook = e; W906_ShowMyMessage_Hook = m; W906_ShowMyMessageEx_Hook = x; W906_ShowMyMessageBoxYesNo_Hook = y; }
};

bool Q44CardUsable(ht9045::TPci1203Monitor* m)                                  // 同 Pci1203IoRoute.cpp CheckWrite_ 的「卡沒開」判斷
{
    return m != 0 && m->Open_() && !m->Disabled() && m->card().open;
}

// SW[] 的 1203 點在卡上的 DO 槽：比對 (ring, 站號, 站內 byte)，第一個符合的（同 JsonBridge/ChanIoPoints.cpp:412 PickIoSample；
//   golden 的 1203 點 Port＝站內通道，byte＝Port/8、bit＝Port%8，Pci1203IoRoute.cpp RouteWriteBit）。
int Q44RelaySlot(ht9045::TPci1203Monitor* m, const TMySwitch& s)
{
    if (m == 0 || s.Ring < 0 || s.IP <= 0 || s.Port < 0) return -1;
    for (int k = 0; k < m->doCount(); ++k) {
        const ht9045::Pci1203DoSample& d = m->do_(k);
        if (d.ring < 0 || d.station <= 0 || d.stationChan < 0) continue;
        if (d.ring == s.Ring && d.station == s.IP && d.stationChan == s.Port / 8) return k;
    }
    return -1;
}

// 兩個命令（Steven 20260928：「這個只要兩個命令就可以做到了」；追問第 1 題）。
void Q44Issue()
{
    FormLockGuard lock;                                                         // 同 W906_Main_CloseProgramOp（可重入）
    Q44QuietScope quiet;

    // [W906] 提前到這裡：golden FormClose 裡 :11953 InitialOK=false 在 :12153／:12157 之前，兩個命令一向在 InitialOK==false 時送；
    //   InitialOK 還是 true 時，IdleCheckSafeDoorByCylinder（csystem.cpp:21344）在門開著時連 Off 也吞掉（MyLaneIo.cpp:326）。
    //   golden 答「是」之後程式一定結束，不會回去生產；這裡也沒有取消（關站段 ShutdownSequence 之後照樣再設一次）。
    InitialOK=false;                                                            // golden :11953

    // ① golden StopAllMotor()（golden :12153，預設參數 true）
    ht9045::Pci1203RouteSetSource("shutdown");
    StopAllMotor(true);
    ht9045::Pci1203RouteSetSource("engine");

    // ② 移植樹：停 1203 監看器開的每一軸（golden 的 MOT[] handle 碰不到監看器開的軸；同 Stop1203Item）
    const IoSnap a = TakeSnap();
    if (W906_Stop1203AllHook != 0 && a.ctl)
        W906_Stop1203AllHook("MainClose Q44: Exit stop-before-close (golden FormClose StopAllMotor, V912 main.cpp:12153)");
    // [W906] 網頁的馬達工作（Motor Test 的 LoopMove／HOME…）一起結束：主迴圈等讀回時還在跑，不結束的話下一拍會再下命令。
    //   golden 不需要這一步 —— FormClose 關掉計時器（:12219-12225）之後程式就結束；停法同 csystem.cpp:30176-30185 VerifyMotorAction
    //   （StopAllMotor＋W906_Stop1203AllHook＋W906_MotorAllBtnUpHook）。它送的停止命令也算進下面的計數。
    if (W906_MotorAllBtnUpHook != 0)
        W906_MotorAllBtnUpHook("MainClose Q44: Exit stop-before-close");
    const IoSnap b = TakeSnap();
    const IoDelta d = TakeDelta(a, b);
    s_q44StopIssued = d.issued; s_q44StopRefused = d.refused; s_q44StopDry = d.dry; s_q44StopVendorErr = d.vendorErr;

    // ③ golden SW[SwHeaterRelay].Off()（golden :12157）
    TMySwitch& sw = SW[SwHeaterRelay];
    w906q44::WriteFact wf;
    const unsigned long seq0 = ht9045::Pci1203RouteLastWrite().seq;
    ht9045::Pci1203RouteSetSource("shutdown");                                  // 路由不略過「卡片已經是這個值」（同 SwOffItem）
    sw.Off();
    ht9045::Pci1203RouteSetSource("engine");
    if (sw.Enable && sw.ISABase == ePCI1203)
    {
        if (ht9045::Pci1203RouteInstalled())
        {
            wf.via = "1203 路由（golden SW[].Off()）";
            const ht9045::Pci1203RouteWrite& w = ht9045::Pci1203RouteLastWrite();
            if (w.seq == seq0) {
                wf.why = "MyLaneIO 在後端之前就 return（位址全 0、CheckPortRangeErr 範圍錯誤，或安全門互鎖）——沒有送出";
            } else {
                wf.attempted = true;
                wf.accepted  = w.reached && w.accepted;
                wf.issued    = w.issued;
                wf.dry       = wf.accepted && !w.issued;
                wf.ret       = w.ret;
                wf.why       = w.why;
            }
        }
        else
        {
            // ④ SIM 建置（或引擎 IO 沒接 1203）：③ 只寫到模擬值 ⇒ 經 1203 命令面把這一位元寫 0（追問第 1 題）。
            //    寫之前過路由的檢查（命令面沒武裝、卡沒開、ring 0、對不到 byte、驅動器站一律不寫 —— 命令面自己的 DO 路徑不擋驅動器站）。
            wf.via = "1203 命令面（SIM 建置：golden Off() 只到模擬值，另寫 Acm_DaqDoSetBitEx）";
            ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
            if (Q44CardUsable(m))
            {
                int slot = -1, chan = -1;
                std::string why;
                wf.attempted = true;
                if (!ht9045::Pci1203RouteCanWriteBit(sw.Ring, sw.IP, sw.Port, &slot, &chan, why)) {
                    wf.why = why;
                } else {
                    ht9045::Pci1203Cmd c;
                    c.kind  = ht9045::kCmdDoSetBit;
                    c.port  = slot;
                    c.bit   = sw.Port % 8;
                    c.value = 0.0;
                    const ht9045::Pci1203CmdResult cr = ht9045::Pci1203Control()->Execute(c);   // CanWriteBit 過了 ⇒ 命令面一定在
                    wf.accepted = cr.accepted;
                    wf.issued   = cr.issued;
                    wf.dry      = cr.accepted && !cr.issued;
                    wf.ret      = cr.ret;
                    wf.why      = cr.why;
                }
            }
            else
            {
                wf.why = "1203 卡沒有開，或監看器已停止輪詢";
            }
        }
    }
    s_q44Relay = wf;

    ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
    s_q44Poll0 = m ? m->card().pollCount : 0;
    s_q44T0    = std::chrono::steady_clock::now();
    std::printf("[Q44] 第 %d 次停機：golden StopAllMotor()；1203 軸停止 ISSUED %lu／DRY %lu／REFUSED %lu（廠商回錯 %d）；SW[SwHeaterRelay].Off()%s%s —— 命令已送（送不到的才等讀回）\n",   // AI(W906-D012) 20260929: 字改成 Steven 0929 的規則（主控台那條也用這支）
                s_q44Attempt, d.issued, d.dry, d.refused, d.vendorErr,
                wf.via.empty() ? "" : "＋", wf.via.empty() ? "" : wf.via.c_str());
    std::fflush(stdout);
}

w906q44::Facts Q44Facts()
{
    w906q44::Facts f;
#ifdef SOFT_SIMULTE
    f.simBuild = true;
#endif
    ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
    f.monPresent = (m != 0);
    if (m) {
        const ht9045::Pci1203CardSample& c = m->card();
        f.cardEverOpen    = m->Open_() || c.pollCount > 0;
        f.cardUsable      = Q44CardUsable(m);
        f.cardWhy         = m->Disabled() ? m->disabledReason() : c.lastErrorText;
        f.pollsSinceIssue = c.pollCount - s_q44Poll0;
        if (f.cardUsable) {
            for (int i = 0; i < m->axisCount(); ++i) {
                const ht9045::Pci1203AxisSample& s = m->axis(i);
                if (!s.opened) continue;
                w906q44::AxisFact a;
                a.index   = i;
                a.station = s.stationAlias >= 0 ? s.stationAlias : s.station;
                a.valid   = s.valid;
                a.state   = s.state;
                a.cmdVel  = s.cmdVel;
                f.axes.push_back(a);
            }
        }
    }
    ht9045::TPci1203Control* ctl = ht9045::Pci1203Control();
    f.stopHook      = (W906_Stop1203AllHook != 0);
    f.ctlArmed      = (ctl != 0);
    f.ctlDry        = ctl ? ctl->IsDryRun() : true;
    f.stopIssued    = s_q44StopIssued;
    f.stopRefused   = s_q44StopRefused;
    f.stopDry       = s_q44StopDry;
    f.stopVendorErr = s_q44StopVendorErr;

    const TMySwitch& sw = SW[SwHeaterRelay];
    f.relayEnable  = sw.Enable;
    f.relayIs1203  = (sw.ISABase == ePCI1203);
    f.relayIsaBase = sw.ISABase;
    f.relayRing    = sw.Ring;
    f.relayIp      = sw.IP;
    f.relayPort    = sw.Port;
    f.relayWrite   = s_q44Relay;
    if (f.cardUsable && f.relayEnable && f.relayIs1203) {
        f.relaySlot = Q44RelaySlot(m, sw);
        if (f.relaySlot >= 0) {
            const ht9045::Pci1203DoSample& dd = m->do_(f.relaySlot);
            f.relayByteValid = dd.valid;
            f.relayBit       = (dd.byteData >> (sw.Port % 8)) & 1;
        }
    }
    return f;
}

std::string Q44Text(const w906q44::Result& r, bool holdingOnly)
{
    std::string s;
    for (std::size_t i = 0; i < r.items.size(); ++i) {
        const w906q44::Item& it = r.items[i];
        if (holdingOnly && !w906q44::Holds(it.state)) continue;
        if (!s.empty()) s += "｜";
        s += it.what + (holdingOnly ? std::string() : std::string("[") + w906q44::StateName(it.state) + "]") + "：" + it.reason;
    }
    return s;
}

std::string Q44Keys(const w906q44::Result& r)                                   // BootLog 用（ASCII）
{
    std::string s;
    for (std::size_t i = 0; i < r.items.size(); ++i)
        if (w906q44::Holds(r.items[i].state)) s += (s.empty() ? "" : ", ") + r.items[i].key + "(" + w906q44::StateName(r.items[i].state) + ")";
    return s.empty() ? std::string("(none)") : s;
}

void Q44Json(webbridge::JsonWriter& w)
{
    const int need = LevelSet.AccessLevel[6];
    w.Key("q44").BeginObject();
    w.Key("phase").String(Q44PhaseName(s_q44));
    w.Key("attempt").Number((wb_int64)s_q44Attempt);
    w.Key("elapsedMs").Number((wb_int64)(s_q44Attempt > 0 ? Q44Ms(s_q44T0) : 0));
    w.Key("settleMs").Number((wb_int64)w906q44::kSettleMs);
    w.Key("items").BeginArray();
    for (std::size_t i = 0; i < s_q44Last.items.size(); ++i) {
        const w906q44::Item& it = s_q44Last.items[i];
        w.BeginObject();
        w.Key("key").String(it.key);
        w.Key("what").String(it.what);
        w.Key("state").String(w906q44::StateName(it.state));
        w.Key("holds").Bool(w906q44::Holds(it.state));
        w.Key("reason").String(it.reason);
        w.EndObject();
    }
    w.EndArray();
    w.Key("notStopped").BeginArray();
    for (std::size_t i = 0; i < s_q44Last.items.size(); ++i)
        if (w906q44::Holds(s_q44Last.items[i].state)) w.String(s_q44Last.items[i].what + "：" + s_q44Last.items[i].reason);
    w.EndArray();
    w.Key("force").BeginObject();
    w.Key("needLevel").Number((wb_int64)need);
    w.Key("level").Number((wb_int64)AccessLevel);
    w.Key("allowed").Bool(w906q44::ForceAllowed(AccessLevel, need));
    w.EndObject();
    if (s_q44 == kQ44Forced) w.Key("forcedText").String(s_q44ForceText);
    w.EndObject();
}

// AI(W906-D012) 20260929 [W906] Q44 A3：W906_ProdCloseShutdown 開頭那一行（主控台 Ctrl-C 那條、沒按 Exit）。回 true＝印了。
bool Q44ConsoleNote()
{
    if (!s_conStopWanted.load() || W906_ServeQuitRequested.load()) return false;   // 按過 Exit：照 Exit 的那一行印（Q44AtShutdown 後段）
    const long ev = s_conFirstEv.load();
    std::printf("MainClose: [Q44] 主控台 %s：停機命令%s（停全部馬達、關加熱器繼電器；Steven 20260929：送出去了就算停，不等讀回）—— 接著照 golden FormClose 做其餘關站步驟\n",
                w906q44::ConsoleEventName((unsigned long)(ev < 0 ? 99 : ev)), s_conStopIssued.load() ? "已送出" : "沒有送出（主迴圈沒有回來）");
    std::fflush(stdout);
    return true;
}

}  // namespace

// W906_ServeQuitDue（晚一圈之後）每圈呼叫：回 true＝可以離開主迴圈。
static bool Q44QuitTick()
{
    if (s_q44 == kQ44Idle || s_q44 == kQ44Retry)
    {
        ++s_q44Attempt;
        Q44Issue();
        s_q44 = kQ44Stopping;
    }
    if (s_q44 == kQ44Stopping)
    {
        s_q44Last = w906q44::Evaluate(Q44Facts(), Q44Ms(s_q44T0), w906q44::kSettleMs);
        if (s_q44Last.verdict == w906q44::kConfirmed)
        {
            s_q44       = kQ44Confirmed;
            s_q44DoneAt = std::chrono::steady_clock::now();
            s_q44Served = false;
            s_q44Final  = false;
            std::printf("[Q44] 已停（第 %d 次，%lu ms）：%s —— 照 golden FormClose 繼續關站\n", s_q44Attempt, Q44Ms(s_q44T0), Q44Text(s_q44Last, false).c_str());
            std::fflush(stdout);
        }
        else if (s_q44Last.verdict == w906q44::kBlock)
        {
            s_q44 = kQ44Blocked;
            std::printf("[Q44] 還沒停：%s —— 不關閉；等網頁［重試停機］或［強制關閉］\n", Q44Text(s_q44Last, true).c_str());
            std::fflush(stdout);
        }
        return false;
    }
    if (s_q44 == kQ44Confirmed)
    {
        if (s_q44Final) return true;
        if (s_q44Served) { s_q44Final = true; return false; }                   // 「已停」那一筆回覆這一圈才送出去；下一圈走
        return Q44Ms(s_q44DoneAt) >= w906q44::kLingerMs;                        // 沒有頁面在問：最多等 kLingerMs
    }
    if (s_q44 == kQ44Forced)
    {
        if (s_q44Final) return true;
        s_q44Final = true;                                                      // 強制那一筆回覆這一圈才送出去；下一圈走
        return false;
    }
    return false;                                                               // kQ44Blocked：等重試／強制
}

// act.main.closeProgram 的 {"op":"status"|"retry"|"force"}。回 true＝payload 帶 op（*out 是回應）；false＝原本的 step 0／1／2。
static bool Q44Op(const std::string& payloadJson, std::string* out, bool* ok)
{
    std::string op;
    bool confirm = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0) return false;                                            // 不是 JSON：交回原本的解析（回 bad-payload）
        const cJSON* jop = cJSON_GetObjectItemCaseSensitive(root, "op");
        if (jop == 0) { cJSON_Delete(root); return false; }
        if (cJSON_IsString(jop) && jop->valuestring) op = jop->valuestring;
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirm");
        confirm = (jc != 0 && cJSON_IsTrue(jc));
        cJSON_Delete(root);
    }
    const bool closing = W906_ServeQuitRequested.load();
    const int  need    = LevelSet.AccessLevel[6];
    std::string guard, detail;
    if (op != "status" && op != "retry" && op != "force") {
        guard = "bad-op"; detail = "op 要是 status、retry 或 force";
    } else if (!closing) {
        guard = "not-closing"; detail = "目前沒有在關站（Exit 還沒確認）";
    } else if (op == "retry") {
        if (s_q44 != kQ44Blocked) {
            guard = "not-blocked"; detail = std::string("現在不是「還沒停」狀態（") + Q44PhaseName(s_q44) + "），不用重試";
        } else {
            s_q44 = kQ44Retry;
            std::printf("[Q44] 網頁［重試停機］（上一次是第 %d 次）\n", s_q44Attempt);
        }
    } else if (op == "force") {
        if (s_q44 != kQ44Blocked) {
            guard = "not-blocked"; detail = std::string("現在不是「還沒停」狀態（") + Q44PhaseName(s_q44) + "），不用強制關閉";
        } else if (!confirm) {
            guard = "force-confirm"; detail = "強制關閉要再確認一次（confirm:true）";
        } else if (!w906q44::ForceAllowed(AccessLevel, need)) {
            guard  = "force-level";
            detail = w906q44::Fmt("強制關閉要權限等級 ≥ %d（Exit 的等級 LevelSet.AccessLevel[6]，golden V912 main.cpp:29062），目前 %d", need, AccessLevel);
        } else {
            s_q44ForceText = Q44Text(s_q44Last, true);
            s_q44          = kQ44Forced;
            s_q44Final     = false;
            std::printf("[Q44] 強制關閉，未停：%s（權限 %d ≥ %d）\n", s_q44ForceText.c_str(), AccessLevel, need);   // Steven 20260928 追問第 2 題：log 記「強制關閉，未停：…」
            std::fflush(stdout);
            WriteBootLog(AnsiString(("W906 Q44 FORCE CLOSE by operator (AccessLevel " + std::to_string(AccessLevel) + " >= " + std::to_string(need) +
                                     "), not stopped: " + Q44Keys(s_q44Last)).c_str()));   // D:\HT9045\Error\BootLog.txt（ASCII：這支檔 BCB6 版也寫）
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(guard.empty());
    w.Key("op").String(op);
    w.Key("closing").Bool(closing);
    if (!guard.empty()) { w.Key("guard").String(guard); w.Key("detail").String(detail); }
    if (closing) Q44Json(w);
    w.EndObject();
    if (guard.empty() && (s_q44 == kQ44Confirmed || s_q44 == kQ44Forced)) s_q44Served = true;
    if (ok) *ok = guard.empty();
    if (out) *out = w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    return true;
}

// W906_ProdCloseShutdown 開頭：主控台記一行 Q44 做到哪（--seconds 到期時可能還沒做完）。
static void Q44AtShutdown()
{
    if (Q44ConsoleNote()) return;                                               // AI(W906-D012) 20260929 [W906] Q44 A3：主控台 Ctrl-C 那條（沒按 Exit 時）
    if (!W906_ServeQuitRequested.load()) return;                                // 沒按 Exit（--seconds 到期）：跟今天一樣
    if (s_q44 == kQ44Confirmed)
        std::printf("MainClose: [Q44] 關站前已確認停下（第 %d 次）：%s\n", s_q44Attempt, Q44Text(s_q44Last, false).c_str());
    else if (s_q44 == kQ44Forced)
        std::printf("MainClose: [Q44] 強制關閉，未停：%s\n", s_q44ForceText.c_str());
    else
        std::printf("MainClose: [Q44] 停機確認沒有做完（phase=%s）就離開主迴圈（--seconds 到期或主控台 Ctrl-C）：照舊結束 —— 無人值守時怎麼做是待辦（Q44-4）%s%s\n",   // AI(W906-D012) 20260929: ＋主控台 Ctrl-C
                    Q44PhaseName(s_q44), s_q44Attempt > 0 ? "；還沒停：" : "", s_q44Attempt > 0 ? Q44Text(s_q44Last, true).c_str() : "");
    std::fflush(stdout);
}

// Exit step 2 的回應：告訴頁面接下來要輪詢 status（新頁面看到 shutdown.q44 才走 Q44 蓋層）。
static void Q44WriteStep2(webbridge::JsonWriter& w)
{
    w.Key("q44").BeginObject();
    w.Key("phase").String("waiting");
    w.Key("settleMs").Number((wb_int64)w906q44::kSettleMs);
    w.Key("poll").String("act.main.closeProgram {\"op\":\"status\"}");
    w.EndObject();
}

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 A2：關站中只收「關」與「停」—— 白名單與理由在 (甲) w906q44::CmdAllowedWhileClosing。
//  wb_serve 的兩個分派入口每一條命令先問這裡（同一行插入，tools/wb_serve.cpp）：
//    ① 主分派迴圈頭（WebCmdGuard 防連點之前）：所有 WS 命令都經過這一點；
//    ② 「輸出優先」W906_ServiceOutputs：io.btnPanelClick／pci1203.do.setBit／setByte／ax.stop／ax.emgStop／motor.stop 會在這裡提前跑
//       （也從 1203 Poll 的 yield 點進來），不經 ①。
//  阻塞框的三個等待迴圈本來就只收回答、cfg.resync、ui.windows.put、motor.stop（其餘 modal-pending），不用再擋。
//  control.acquire／release 在 WebBridgeServer 的 socket 執行緒上回答，不到這裡；HTTP（/api/*）是讀取，不在這裡擋。
//  沒在關站：只讀兩個 atomic 就回 false。都在 tick 執行緒。
// ===========================================================================
bool W906_Q44CmdRefused(const std::string& cmd, const std::string& valueStr, std::string* why)
{
    const bool byExit    = W906_ServeQuitRequested.load();
    const bool byConsole = s_conStopWanted.load();
    if (!byExit && !byConsole) return false;
    std::string sub;
    int start = -1;
    if ((cmd == "motor.access" || cmd == "observer.get") && !valueStr.empty())
    {
        cJSON* root = cJSON_Parse(valueStr.c_str());
        if (root == 0) sub = "(unparsable value)";                              // 不在白名單 ⇒ 擋（本體也會拒）
        else {
            const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, cmd == "motor.access" ? "action" : "act");
            if (ja && cJSON_IsString(ja) && ja->valuestring) sub = ja->valuestring;
            const cJSON* jp = cJSON_GetObjectItemCaseSensitive(root, "params");
            if (jp && cJSON_IsObject(jp)) {
                const cJSON* js = cJSON_GetObjectItemCaseSensitive(jp, "start");
                if (js && cJSON_IsBool(js)) start = cJSON_IsTrue(js) ? 1 : 0;
            }
            cJSON_Delete(root);
        }
    }
    const bool lsActive = (cmd == "motor.access" && sub == "lightScale") && ht9045::MotorAccessLightScale(0).active;   //AI(W906-D012-M2) 20260929: golden Timer2->Enabled（WebMotorAccess.h:499；tick 執行緒）
    if (w906q44::CmdAllowedWhileClosing(cmd, sub, start, lsActive)) return false;
    if (why) *why = w906q44::ClosingRefusal(cmd, byConsole && !byExit);
    ++s_q44Refused;
    if (s_q44Refused <= 30 || s_q44Refused % 100 == 0)
        std::printf("[Q44] 關站中拒絕 %s%s%s（第 %lu 條；只收關閉／停止／唯讀）\n", cmd.c_str(), sub.empty() ? "" : " ", sub.c_str(), s_q44Refused);
    return true;
}

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 A3：主控台 Ctrl-C／Break／按視窗 X／登出／關機。規則在 (甲) w906q44::ConsoleDecide。
//  W906_ConsoleQuit：tools/wb_serve.cpp W906_ConsoleCtrl（SetConsoleCtrlHandler 的處理函式）呼叫。
//    ⚠ 執行緒：Windows 在本行程裡**另開一條執行緒**跑處理函式（不是 tick 執行緒）。1203 廠商 API 只能在 tick 執行緒呼叫
//      （Pci1203Control.h:800-803），golden 的 SW[]／MOT[] 也是 tick 執行緒的 ⇒ 這裡只設 atomic 旗標，停機命令由 tick 執行緒送：
//      主迴圈底（W906_ServeQuitDue，每圈）與阻塞框的等待（W906_Q44ConsoleServiceTick，由 W906_ModalWaitTick 每圈呼叫）。
//    回 true＝處理函式回 TRUE（行程不結束，主執行緒自己關站、寫 Program Close=1）；
//    回 false＝處理函式寫 Program Close=1、回 FALSE（系統預設處理＝結束行程）。
//    AI(W906-D012-A3W) 20260930 [W906]：回 true 另外兩種 —— 登出／關機（kConSessionEnd：停機命令送出後回 true，處理函式回 TRUE、不寫 Program Close，
//      系統結束行程）與 follower（另一條已經在結束行程，這條等它、不寫）。
//  每個事件：
//    第一次 Ctrl-C／Break：設旗標、回 true。tick 執行緒下一次到主迴圈底（最多一圈：≤50 ms 睡眠＋PumpTick＋1203 Poll 約 140 ms）
//      送停機命令（Q44Issue：InitialOK=false、golden StopAllMotor、1203 軸 W906_Stop1203AllHook、網頁馬達工作結束、SW[SwHeaterRelay].Off()，
//      SIM 接真卡時另經命令面寫 0），然後離開主迴圈 → 正常關站路（W906_ProdCloseSave 存生產資料 → W906_ProdCloseShutdown 照 golden
//      FormClose 做其餘關站〔含再停一次、鎖煞車、關風扇〕→ Program Close=1 → server.Stop()）。按過 Exit、還在等［重試］／［強制］時，
//      Ctrl-C 等於「再送一次停機命令、然後結束」（不再等讀回）。阻塞框開著時：命令照送，但要等框被回答才離得開主迴圈（主控台印一行）。
//    第二次 Ctrl-C／Break：照今天——寫 Program Close=1、立刻結束（主迴圈卡住時用；停機命令送了沒有，主控台印一行）。
//    按視窗 X／登出／關機：設旗標、等 tick 執行緒送出停機命令（不等讀回），最多 kConsoleHardWaitMs（3.5 秒；CTRL_CLOSE 系統給 5 秒），
//      然後寫 Program Close=1、結束行程。不存生產資料（跟今天一樣：5 秒內寫 lastdata.dat 可能寫到一半被砍）。主執行緒已經在正常關站路上
//      （例：先 Ctrl-C 再按 X）時不催，等它自己結束行程，到期限就結束。
//    ⚠ 登出／關機：微軟文件（SetConsoleCtrlHandler 備註）說載入了 user32.dll／gdi32.dll 的主控台程式**收不到** CTRL_LOGOFF_EVENT／
//      CTRL_SHUTDOWN_EVENT（系統把它當視窗程式，改送 WM_QUERYENDSESSION／WM_ENDSESSION 給它的隱藏視窗）。wb_serve 會載入 user32，
//      所以這兩個分支在機台上很可能不會被呼叫到 —— 要在機台上量；要保證登出／關機也停機，得另做隱藏視窗（待辦）。
//      ⛔ AI(W906-D012-A3W) 20260930 [W906] 更新：隱藏視窗做了（檔尾 W906_SessionEndWatchStart／Q44SessionWndProc；規則 (甲) A3W）。
//      登出／關機（主控台 5／6，或隱藏視窗的 WM_ENDSESSION(TRUE)）現在是 kConSessionEnd：設停機旗標、等 tick 執行緒送出（最多 3.5 秒），
//      **不寫** MES2109、**不寫** Program Close=1（golden 登出／關機不跑 FormClose：VCL 6 forms.pas:4104-4107／:6420；ST01-E 20260930 裁定），
//      回 true ⇒ 主控台處理函式回 TRUE、不寫（HandlerRoutine 頁：回 TRUE 時「the system terminates the process」）；隱藏視窗那條由視窗執行緒
//      ExitProcess（主執行緒在正常關站路上時不結束它）。按視窗 X（kConStopThenExit）、第二次 Ctrl-C 照舊。
//      兩條路都到（主控台 CTRL_CLOSE＋WM_ENDSESSION，或 X 等待中再按兩次 Ctrl-C）時只有先到的是 owner（照自己的規則寫／不寫、結束行程），
//      後到的等它、不寫（(甲) A3W ⑤ ConsolePlanFor）；主控台印誰是 owner。實際登出／關機會走哪一條、有沒有兩條都到，要在機台上量（交件報告）。
//    AI(W906-D012-A8) 20260930 [W906]：第二次 Ctrl-C／Break 與 X／登出／關機的行程在這裡結束、走不到正常關站路 ⇒ 結束前請 tick 執行緒照 golden
//      FormClose 寫事件紀錄 MES2109「Program Close」（main.cpp:12443；Q44ConsoleTick 送完停機命令緊接著寫），寫完才結束：X／登出／關機併在
//      上面那 3.5 秒裡（ConsoleHardWaitDone 多等「紀錄寫完」），第二次 Ctrl-C 最多等 0.5 秒（kConsoleExitNowWaitMs）。主控台印寫了沒有。
//      ⛔ AI(W906-D012-A3W) 20260930：登出／關機不寫了（上一段）；只剩第二次 Ctrl-C／Break 與按視窗 X。
// ===========================================================================
namespace {
// AI(W906-D012-A3W) 20260930 [W906]：s_conExitOwner 存的值＝主控台事件號碼，隱藏視窗那條再加 0x100（主控台印誰是 owner 用）
long Q44ExitToken(unsigned long ev, bool fromWindow) { return (long)ev + (fromWindow ? 0x100L : 0L); }
const char* Q44ExitTokenName(long tok)
{
    if (tok < 0) return "?";
    const unsigned long ev = (unsigned long)(tok & 0xFF);
    return (tok & 0x100) ? w906q44::SessionEndName(ev) : w906q44::ConsoleEventName(ev);
}
}  // namespace

// AI(W906-D012-A3W) 20260930 [W906]：本體（原本就是 W906_ConsoleQuit 的內容）。fromWindow＝隱藏視窗那條（WM_ENDSESSION）；
//   roleOut＝給隱藏視窗判斷要不要自己 ExitProcess（(甲) SessionEndWindowExits）。
static bool Q44ConsoleQuit(unsigned long ev, bool fromWindow, int* roleOut)
{
    const int nth = ++s_conCount;
    long none = -1;
    s_conFirstEv.compare_exchange_strong(none, (long)ev);
    const int act = w906q44::ConsoleDecide(ev, nth);
    const char* name = fromWindow ? w906q44::SessionEndName(ev) : w906q44::ConsoleEventName(ev);
    // AI(W906-D012-A3W) 20260930 [W906]：結束行程一個行程只有一個 owner，先到先得（(甲) A3W ⑤）；follower 只補停機旗標、不寫、不結束行程。
    int  role  = w906q44::kExitNotMine;
    long owner = -1;
    if (w906q44::ConsoleEndsProcess(act))
    {
        const long mine = Q44ExitToken(ev, fromWindow);
        const bool claimedBefore = !s_conExitOwner.compare_exchange_strong(owner, mine);   // 失敗時 owner＝先到那一條的值
        role = w906q44::ConsoleExitRoleFor(act, claimedBefore);
        if (!claimedBefore) owner = mine;
    }
    if (roleOut) *roleOut = role;
    const w906q44::ConsolePlan plan = w906q44::ConsolePlanFor(act, role);
    if (role == w906q44::kExitFollower)
    {
        if (plan.stop) s_conStopWanted.store(true);                             // 冪等：tick 執行緒只送一次（s_conStopIssued）
        const long oev = owner & 0xFF;
        std::printf("[Q44] %s：結束行程的 owner 是先到的「%s」（%s）；這一條只補停機旗標，不寫 MES2109、不寫 Program Close、不結束行程，"
                    "等它結束（最多 %lu ms）\n", name, Q44ExitTokenName(owner),
                    (oev == 5 || oev == 6) ? "登出／關機：不寫 MES2109、不寫 Program Close" : "寫 MES2109＋Program Close=1", w906q44::kConsoleFollowerWaitMs);
        std::fflush(stdout);
        const DWORD t0 = ::GetTickCount();
        while ((unsigned long)(::GetTickCount() - t0) < w906q44::kConsoleFollowerWaitMs) ::Sleep(20);
        std::printf("[Q44] %s：等了 %lu ms 行程還沒結束（owner 還在寫或卡住）；這一條照樣不寫、回 TRUE\n", name, w906q44::kConsoleFollowerWaitMs);
        std::fflush(stdout);
        return true;
    }
    if (role == w906q44::kExitOwner)
    {
        std::printf("[Q44] %s：這一條是結束行程的 owner（%s）\n", name,
                    plan.programClose ? "寫 MES2109＋Program Close=1" : "登出／關機：照 golden 不寫 MES2109、不寫 Program Close（留 0）");
        std::fflush(stdout);
    }
    if (act == w906q44::kConLetMainClose)
    {
        s_conLeave.store(true);
        s_conStopWanted.store(true);
        std::printf("[Q44] 主控台 %s：主迴圈先送停機命令（停全部馬達、關加熱器繼電器），再照 golden FormClose 存檔、停機、寫 Program Close=1 後結束。"
                    "主迴圈卡住時再按一次 %s 立刻結束（不等）\n", name, name);
        std::fflush(stdout);
        return true;
    }
    if (act == w906q44::kConExitNow)
    {
        // AI(W906-D012-A8) 20260930 [W906]：結束前請 tick 執行緒寫 golden 關站的事件紀錄（main.cpp:12443 MES2109；見 (甲) A8），最多等
        //   kConsoleExitNowWaitMs（主迴圈卡住時用的出口，不能等太久）；主執行緒已經在正常關站路上時等它自己在 golden 的位置寫。
        if (plan.record) s_conRecordWanted.store(true);                         // AI(W906-D012-A3W)：原本 ConsoleNeedsCloseRecord(act)（owner 時同值）
        const DWORD t0 = ::GetTickCount();
        while (!w906q44::ConsoleExitNowWaitDone(w906q44::CloseRecordSettled(s_closeRecWritten.load(), bHandlerModel),
                                                (unsigned long)(::GetTickCount() - t0), w906q44::kConsoleExitNowWaitMs))
            ::Sleep(10);
        std::printf("[Q44] 主控台再按一次 %s：立刻結束 —— 停機命令%s；%s\n", name,
                    s_conStopIssued.load() ? "已送出" : "還沒送出（主迴圈沒有回來）；機台上請照程序停機（例如按 EMG）",
                    Q44CloseRecordLine());
        std::fflush(stdout);
        return false;
    }
    if (act == w906q44::kConStopThenExit || act == w906q44::kConSessionEnd)   // AI(W906-D012-A3W) 20260930 [W906]：+ kConSessionEnd（登出／關機）
    {
        if (plan.stop) s_conStopWanted.store(true);
        if (plan.record) s_conRecordWanted.store(true);                         // AI(W906-D012-A8) 20260930 [W906]：送完停機命令緊接著寫 golden 的 MES2109（只有 X）
        const DWORD t0 = ::GetTickCount();
        for (;;)
        {
            const unsigned long el = (unsigned long)(::GetTickCount() - t0);
            if (w906q44::ConsoleHardWaitDone(s_conStopIssued.load(), s_mainClosing.load(), el, w906q44::kConsoleHardWaitMs,
                                             plan.record ? w906q44::CloseRecordSettled(s_closeRecWritten.load(), bHandlerModel) : true)) break;   // AI(W906-D012-A8)：紀錄寫完才算（登出／關機不等紀錄）
            ::Sleep(20);
        }
        const unsigned long el = (unsigned long)(::GetTickCount() - t0);
        const char* stopText = s_conStopIssued.load() ? "停機命令已由主迴圈送出（不等讀回）"
                             : s_mainClosing.load()   ? "主執行緒在正常關站路上（它自己停機），等到期限"
                                                      : "停機命令沒有送出：主迴圈在期限內沒有回來；機台上請照程序停機（例如按 EMG）";
        if (act == w906q44::kConSessionEnd)
            std::printf("[Q44] %s：%s（%lu ms）；golden 登出／關機不跑 FormClose ⇒ 不寫 MES2109、不寫 Program Close（留 0：下次開機看到異常關機），"
                        "不存生產資料；結束行程\n", name, stopText, el);
        else
            std::printf("[Q44] 主控台 %s：%s（%lu ms）；%s；寫 Program Close=1、結束行程（不存生產資料）\n", name, stopText, el, Q44CloseRecordLine());
        std::fflush(stdout);
        return !plan.programClose;   // X：false（處理函式寫 Program Close=1、回 FALSE）；登出／關機：true（處理函式回 TRUE、不寫；系統結束行程）
    }
    return false;
}

bool W906_ConsoleQuit(unsigned long ev)
{
    return Q44ConsoleQuit(ev, false, 0);                                        // AI(W906-D012-A3W) 20260930 [W906]：本體搬到 Q44ConsoleQuit（隱藏視窗也用）
}

// ---------------------------------------------------------------------------
//  AI(W906-D012-A8) 20260930 [W906]：golden 關站的事件紀錄（V912 main.cpp:12443）一個行程只寫一次（規則在 (甲) A8）。
//    認領（Q44CloseRecordClaim）先到先得：主執行緒的 ShutdownSequence(true)（golden 的位置）或 tick 執行緒的 Q44ConsoleTick（主控台那條路）；
//    寫完（Q44CloseRecordWritten）之後主控台處理函式才結束行程。兩個都只碰 atomic（處理函式那條執行緒只讀）。
// ---------------------------------------------------------------------------
static bool Q44CloseRecordClaim()
{
    return !s_closeRecClaimed.exchange(true);
}

static void Q44CloseRecordWritten()
{
    s_closeRecWritten.store(true);
}

// 主控台一行：這一次結束之前 golden 那一筆寫了沒有（處理函式那條執行緒呼叫，只讀 atomic）
static const char* Q44CloseRecordLine()
{
    if (!bHandlerModel) return "golden 事件紀錄 MES2109 照 golden 不寫（bHandlerModel=false：golden FormClose 不會跑）";
    if (s_closeRecWritten.load()) return "golden 事件紀錄 MES2109「Program Close」已寫（golden FormClose main.cpp:12443）";
    if (s_closeRecClaimed.load()) return "golden 事件紀錄 MES2109 正在寫、還沒寫完（可能截斷）";
    if (s_mainClosing.load()) return "golden 事件紀錄 MES2109 由正常關站路在 golden 的位置寫（ShutdownSequence），結束這一刻還沒寫到";
    return "golden 事件紀錄 MES2109 沒寫：主迴圈在期限內沒有回來";
}

// tick 執行緒：處理函式要停機時，送一次兩個命令（Q44Issue）。inModal＝從阻塞框的等待呼叫（離不開主迴圈）。
//   回 true＝主迴圈可以離開（Ctrl-C／Break 那條，已送）。
static bool Q44ConsoleTick(bool inModal)
{
    if (!s_conStopWanted.load() && !s_conRecordWanted.load()) return false;   // AI(W906-D012-A8) 20260930 [W906]：+ 只要寫紀錄（第二次 Ctrl-C；停機命令第一次就要過了）
    if (s_conStopWanted.load() && !s_conStopIssued.load())                      // AI(W906-D012-A8)：+ s_conStopWanted（原本上一行已經擋掉沒要停的）
    {
        ++s_q44Attempt;
        Q44Issue();                                                             // 只送，不等讀回（Steven 20260929：送出去了就算停）
        s_conStopIssued.store(true);
        const long ev = s_conFirstEv.load();
        const char* name = w906q44::ConsoleEventName((unsigned long)(ev < 0 ? 99 : ev));
        std::printf("[Q44] 主控台 %s：停機命令已送出（第 %d 次；不等讀回）%s\n", name, s_q44Attempt,
                    s_conLeave.load() ? "—— 離開主迴圈，照 golden FormClose 存檔、停機、寫 Program Close=1" : "—— 等處理函式結束行程");
        std::fflush(stdout);
        WriteBootLog(AnsiString((std::string("W906 Q44 console ") + name + ": stop commands sent (StopAllMotor, 1203 axes, SwHeaterRelay off; no read-back)").c_str()));   // D:\HT9045\Error\BootLog.txt（ASCII）
    }
    // AI(W906-D012-A8) 20260930 [W906]：行程要在主控台處理函式裡結束（按視窗 X／登出／關機、第二次 Ctrl-C／Break）⇒ 走不到正常關站路的
    //   ShutdownSequence(true) ⇒ 這裡（tick 執行緒；NewRecordProcess 不是執行緒安全）照 golden FormClose 寫同一筆、同樣的字，寫完處理函式才結束
    //   行程。規則 (甲) A8；主執行緒在正常關站路上時不寫（它在 golden 的位置寫），已經有人寫過不寫第二次。
    if (w906q44::ConsoleTickWritesCloseRecord(s_conRecordWanted.load(), s_closeRecClaimed.load(), s_mainClosing.load(), bHandlerModel) &&
        Q44CloseRecordClaim())
    {
        NewRecordProcess("MES2109", "Program Close", asHandlerVersion +"."+ AnsiString(SVNRevision));   //Steven 20091004   // golden main.cpp:12443（FormClose）
        Q44CloseRecordWritten();
        std::printf("[Q44] 主控台（行程要在處理函式裡結束）：golden 事件紀錄已寫 —— NewRecordProcess(\"MES2109\", \"Program Close\", \"%s\")"
                    "（golden FormClose main.cpp:12443；這條路不經 FormClose 的其他步驟）\n", (asHandlerVersion + "." + AnsiString(SVNRevision)).c_str());
        std::fflush(stdout);
    }
    if (inModal)
    {
        if (s_conLeave.load() && !s_conModalNoted)
        {
            s_conModalNoted = true;
            std::printf("[Q44] 主控台：停機命令已送出，但網頁上有一個阻塞框在等回答 —— 回答它之後才會離開主迴圈；或再按一次 Ctrl-C 立刻結束\n");
            std::fflush(stdout);
        }
        return false;
    }
    return s_conLeave.load();
}

// 阻塞框的等待迴圈每圈呼叫（tools/wb_serve.cpp W906_ModalWaitTick 第一行）：框開著時主控台 X／Ctrl-C 也送得出停機命令。
void W906_Q44ConsoleServiceTick()
{
    (void)Q44ConsoleTick(true);
}

static void Q44MainClosing()
{
    s_mainClosing.store(true);
}

// ---------------------------------------------------------------------------
//  AI(W906-D012-A3W) 20260930 [W906]：登出／關機的隱藏視窗（規則 (甲) A3W；tools/wb_serve.cpp 在 SetConsoleCtrlHandler 旁同一行呼叫
//  W906_SessionEndWatchStart，主迴圈開始前）。
//    執行緒：自己一條（CreateThread；MinGW.org 6.3 沒有 std::thread），建窗＋GetMessageW 迴圈，行程結束前一直在。視窗屬於建它的執行緒，
//      系統把 WM_QUERYENDSESSION／WM_ENDSESSION 送到這條（不是 tick 執行緒 —— tick 執行緒卡在 1203 Poll 或阻塞框的等待時照樣收得到）。
//      這條跟主控台處理函式一樣只設旗標、等：停機命令仍由 tick 執行緒送（廠商 API 只能在 tick 執行緒，EtherCAT/Pci1203Control.h:800-803）。
//    視窗：真的頂層視窗（CreateWindowExW：dwExStyle=0、WS_OVERLAPPED、父視窗 NULL，照微軟「Registering a Control Handler Function」頁的
//      「Listen with Hidden Window Example」），不加 WS_VISIBLE、大小 0 ⇒ 看不見、不上工作列。不能用 HWND_MESSAGE：訊息專用視窗收不到
//      WM_QUERYENDSESSION 這種送給所有頂層視窗的訊息。不呼叫 ShowWindow：行程第一次 ShowWindow 可能被換成 STARTUPINFO 的顯示方式
//      （微軟 ShowWindow 的 nCmdShow 說明），會吃掉原生表單（ui/native，W906_NATIVE_FORMS=ON）第一個視窗該用的那一次。
//    WM_QUERYENDSESSION：回 TRUE（QueryEndSessionAnswer），不做別的。
//    WM_ENDSESSION(TRUE)：Q44ConsoleQuit(5／6, fromWindow=true)——跟主控台登出／關機同一條（kConSessionEnd：送停機命令、不寫 MES2109、
//      不寫 Program Close）。之後：owner 而且主執行緒不在正常關站路上 ⇒ ExitProcess（同主控台預設處理函式，HandlerRoutine 頁「a default
//      handler function that calls ExitProcess」；結束碼同它 STATUS_CONTROL_C_EXIT）；主執行緒在正常關站路上 ⇒ 回 0，讓它自己寫完、結束
//      （系統要結束時再結束）；follower ⇒ Q44ConsoleQuit 裡等 owner 結束行程。
//    WM_ENDSESSION(FALSE)：不登出了 ⇒ 什麼都不做。
//    WM_CLOSE（例如 taskkill 沒加 /F）：不拆視窗、不關程式（DefWindowProc 會拆掉它，之後就收不到登出／關機；加視窗之前 taskkill 沒加 /F 本來就關不掉 wb_serve）。
//    ShutdownBlockReasonCreate 不用：視窗看不見，系統不讓沒有可見視窗的程式擋關機、5 秒就結束（Shutdown Changes for Windows Vista）；
//      MinGW.org 6.3 的 winuser.h 也沒有宣告它。
//    ctest 測不到（要真的登出／關機才有這兩個訊息）：MainCloseStop [19] 只測 (甲) 的規則；機台上要看的寫在交件報告與 Q44 待辦 A 第 3 條。
// ---------------------------------------------------------------------------
namespace {
const DWORD       kQ44ConsoleDefaultExitCode = 0xC000013AUL;   // STATUS_CONTROL_C_EXIT：主控台預設處理函式結束行程用的碼（MinGW.org 標頭沒有這個名字）
HANDLE            s_sesReady = 0;                              // 建窗結果好了（manual-reset event）
HWND              s_sesHwnd  = 0;                              // 視窗執行緒寫、SetEvent 之後 W906_SessionEndWatchStart 才讀
DWORD             s_sesErr   = 0;
std::atomic<bool> s_sesStarted(false);

LRESULT CALLBACK Q44SessionWndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_QUERYENDSESSION)
    {
        std::printf("[Q44] Windows 要%s（WM_QUERYENDSESSION，lParam=0x%08lX）：回答可以（golden 同：VCL 6 CloseQuery）；收到 WM_ENDSESSION 才送停機命令\n",
                    (((unsigned long)lp) & w906q44::kEndSessionLogoff) ? "登出" : "關機", (unsigned long)lp);
        std::fflush(stdout);
        return w906q44::QueryEndSessionAnswer() ? TRUE : FALSE;
    }
    if (msg == WM_ENDSESSION)
    {
        const long ev = w906q44::SessionEndConsoleEvent(wp != 0, (unsigned long)lp);
        if (ev < 0)
        {
            std::printf("[Q44] WM_ENDSESSION(FALSE)：不登出／關機了（有程式擋下），什麼都不做\n");
            std::fflush(stdout);
            return 0;
        }
        const char* name = w906q44::SessionEndName((unsigned long)ev);
        std::printf("[Q44] %s（lParam=0x%08lX）：送停機命令；照 golden 不寫 MES2109、不寫 Program Close（留 0）\n", name, (unsigned long)lp);
        std::fflush(stdout);
        int role = w906q44::kExitNotMine;
        (void)Q44ConsoleQuit((unsigned long)ev, true, &role);
        if (w906q44::SessionEndWindowExits(role, s_mainClosing.load()))
        {
            std::printf("[Q44] %s：結束行程（ExitProcess）\n", name);
            std::fflush(stdout);
            ::ExitProcess(kQ44ConsoleDefaultExitCode);
        }
        if (role == w906q44::kExitOwner)
            std::printf("[Q44] %s：主執行緒在正常關站路上（它在 golden 的位置寫 MES2109 與 Program Close=1、自己結束行程），這裡不結束它；回 0\n", name);
        else
            std::printf("[Q44] %s：不是 owner，回 0\n", name);
        std::fflush(stdout);
        return 0;
    }
    if (msg == WM_CLOSE)
    {
        // 例如 taskkill 沒加 /F（送 WM_CLOSE 給行程的頂層視窗）：DefWindowProc 會把這個視窗拆掉，之後登出／關機就收不到了 ⇒ 不拆、不關程式
        //   （加這個視窗之前，taskkill 沒加 /F 對 wb_serve 本來就關不掉）。要關程式請按 Exit 或 Ctrl-C。
        std::printf("[Q44] 登出／關機的隱藏視窗收到 WM_CLOSE（例如 taskkill 沒加 /F）：不處理 —— 要關程式請按 Exit 或 Ctrl-C\n");
        std::fflush(stdout);
        return 0;
    }
    return ::DefWindowProcW(h, msg, wp, lp);
}

DWORD WINAPI Q44SessionThread(LPVOID)
{
    const HINSTANCE inst = ::GetModuleHandleW(0);
    WNDCLASSEXW wc;
    ::ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = Q44SessionWndProc;
    wc.hInstance     = inst;
    wc.lpszClassName = L"W906_HT9045_SessionEnd";
    HWND h = 0;
    if (::RegisterClassExW(&wc) != 0)
        h = ::CreateWindowExW(0, wc.lpszClassName, L"HT9045 wb_serve logoff/shutdown listener", WS_OVERLAPPED,
                              0, 0, 0, 0, 0, 0, inst, 0);
    if (h == 0) s_sesErr = ::GetLastError();
    s_sesHwnd = h;
    if (s_sesReady) ::SetEvent(s_sesReady);
    if (h == 0) return 1;
    MSG m;
    while (::GetMessageW(&m, 0, 0, 0) > 0)
    {
        ::TranslateMessage(&m);
        ::DispatchMessageW(&m);
    }
    return 0;
}
}  // namespace

void W906_SessionEndWatchStart()
{
    static_assert(WM_QUERYENDSESSION == w906q44::kWmQueryEndSession, "WM_QUERYENDSESSION");
    static_assert(WM_ENDSESSION == w906q44::kWmEndSession, "WM_ENDSESSION");
    static_assert((unsigned long)ENDSESSION_LOGOFF == w906q44::kEndSessionLogoff, "ENDSESSION_LOGOFF");
    if (s_sesStarted.exchange(true)) return;
    s_sesReady = ::CreateEventW(0, TRUE, FALSE, 0);
    HANDLE t = ::CreateThread(0, 0, Q44SessionThread, 0, 0, 0);
    if (t == 0)
    {
        std::printf("[Q44] 登出／關機的隱藏視窗：開執行緒失敗（GetLastError=%lu）—— 登出／關機時不會送停機命令\n", (unsigned long)::GetLastError());
        std::fflush(stdout);
        return;
    }
    ::CloseHandle(t);
    const DWORD w = s_sesReady ? ::WaitForSingleObject(s_sesReady, 2000) : WAIT_FAILED;
    if (w == WAIT_OBJECT_0 && s_sesHwnd != 0)
        std::printf("[Q44] 登出／關機的隱藏視窗已建立：Windows 登出／關機時先送停機命令（不寫 MES2109、不寫 Program Close，golden 同）\n");
    else if (w == WAIT_OBJECT_0)
        std::printf("[Q44] 登出／關機的隱藏視窗：建窗失敗（GetLastError=%lu）—— 登出／關機時不會送停機命令\n", (unsigned long)s_sesErr);
    else
        std::printf("[Q44] 登出／關機的隱藏視窗：2 秒內沒有建好（wait=%lu），照常開站\n", (unsigned long)w);
    std::fflush(stdout);
}
#endif  // !W906_MAINCLOSE_Q44_DECIDE_ONLY
