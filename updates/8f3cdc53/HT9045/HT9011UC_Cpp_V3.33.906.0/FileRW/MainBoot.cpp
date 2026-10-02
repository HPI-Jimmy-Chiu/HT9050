// ===========================================================================
//  FileRW/MainBoot.cpp -- golden TfMain::FormShow（V912 main.cpp）裡「不屬於任何一個表單」的開機讀寫檔。
//
//  AI(W906-FRW-Boot) 20260926（Steven 團隊）：新檔。Steven 20260926「先以大量把讀寫檔進行移植為首要工作」；
//    清單來源：20260925 靜態盤點 cmydef_io_audit.md 第四節 P1、P8（行號已重查）。
//    golden 一律照 V912：D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 → UTF-8），
//    RULINGS_20260925 S37（畫面讀寫與開機讀檔照 912）、第 37 條分工（Steven＝畫面讀寫與開機讀檔）。
//
//  三支函式，各對應 golden FormShow 的一個位置；wb_serve.cpp 的開機序列在對應位置各呼叫一次
//  （只在開機；不在 W906_DoReadLastData 裡 —— golden 這三處都在 FormShow，換配方 ChangeSetUpFile 不會再跑）：
//
//    W906_FRWBoot_BinCountRead()       golden main.cpp:9609         ReadWriteBinCountMode(true)
//    W906_FRWBoot_VersionStamp()       golden main.cpp:9779-9782    WriteIniDataGeneral("Version", "Ver", asHandlerVersion)
//    W906_FRWBoot_JamRawDataRecord()   golden main.cpp:11576-11580  if(IniConfig.bN26_UseJamRawDataRecord)
//                                                                   { fObserver->ReadLoaderCount(); fObserver->StatisticalJamCount(); }
//    W906_FRWBoot_ShowLotInfoDownloadFlag()  golden main.cpp:10041-10046  if(IniConfig.bShowLotInfo)
//                                                                   { bHasDownloadFile=false（非 RMS 免下載）; WriteLastDataFile(); }
//    W906_FRWBoot_ResetLotInfo()       golden main.cpp:10991        fLotInfo->ResetLotInfo()（有 IC 時讀回 config.ini 的 Product Name／Temp、
//                                                                   bHasDownloadFile=true；本體 forms/fLotInfo.cpp 檔尾）
//                                      AI(W906-FRW-WC1) 20260927（Steven 團隊，GATE WC-1 退役）
//                                      AI(W906-FRW-S65) 20260926（Steven 團隊，S65 lastdata.dat 觸發者）
//    W906_FRWBoot_JamRateByDayRead()   golden main.cpp:9607         RunInfo.ReadJamRateByDay()（DailyJamRate txt）
//    W906_FRWBoot_CriticalParaAuth()   golden main.cpp:9787         GetCriticalParaAuth()（config\CriticalParaControl.ini）
//    W906_FRWBoot_LimitAuth()          golden main.cpp:9786         GetLimitAuth()（config\Security_new.def [Input Limit]；S91 的鄰居，
//                                      不在 S91 清單 —— 接不接由 Steven 決定，見該函式註解）
//                                      AI(W906-FRW-S91) 20260926（Steven 團隊）；這三列的行號是主 repo 那份 V912
//                                      （D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy；上面幾列是已移除的 D:\HT9045_ref，
//                                      這一帶比主 repo 少 2 行：BinCount 主 repo 是 :9611、版本戳記是 :9783）
//    W906_FRWBoot_LotListsRead()       golden main.cpp:11344-11365  2D 重複碼清單 LotData.txt → list2DByLot＋map2DList；
//                                                                   TrayIDByLot.txt → fTrayMapping->listTrayIDByLot（主 repo V912）
//                                      AI(W906-FRW-S94) 20260926（Steven 團隊，S94 依批號的三份清單的開機讀回；清除鈕在 WebLotInfo.cpp）
//
//  共同前提 —— golden FormShow :9589-9594：bHandlerModel==false（D:\GPIB9045\system\general.ini 的 Model 讀不到或不在
//    白名單）時 MessageDlg＋Application->Terminate()＋return，上面三個位置 golden 都走不到。wb_serve 不結束程式，
//    所以三支都先檢查 bHandlerModel，false 就印一行跳過（判斷同 FileRW/HSys.cpp 的 HS_ModelReadError，這裡不跳框）。
//
//  ⚠ 會碰到的真實檔（整合測試前先備份）：
//    D:\HT9045\system\BinCount.txt —— 讀。golden 的 CheckAndReadIniData 在 [System] Bin／ErrorBin 缺鍵時會補寫 "999"，
//      Bin 缺鍵時再把目前記憶體的計數寫回（csystem.cpp:28815-28848，golden csystem.cpp:24655 起）。兩鍵都在就只讀。
//      環境變數 W906_BINCOUNT_PATH 可導開（csystem.cpp:28808，tests/test_bootstrap.cpp 用它）。
//    D:\HT9045\system\Gerneral.ini [Version] Ver —— 每次開機都寫（golden 同，沒有條件；SIGURD 的條件 golden 自己註解掉）。
//    D:\HT9045\system\lastdata.dat／lastdata_backup.dat —— 只在 IniConfig.bShowLotInfo 為真時，每次開機整塊寫一次 LastSet
//      （W906_FRWBoot_ShowLotInfoDownloadFlag；golden 同，寫死路徑，--dry 蓋不到）。AI(W906-FRW-S65) 20260926
//    D:\HT9045_Log\EventLogTxt\SGJamCount\ —— 只在 IniConfig.bN26_UseJamRawDataRecord 為真時（ReadLoaderCount 的
//      MyForceDirectories 會建資料夾；StatisticalJamCount 寫 SGJamCount 的 csv）。W906_EVENTLOG_ROOT 可導開。
//    D:\HT9045_Log\JamRate_Daily\<sMachineType>_<SocketHandlerID>_<YYYY-MM-DD>_DailyJamRate.txt —— 只讀（每次開機）；
//      資料夾不在會先建（MyForceDirectories）。AI(W906-FRW-S91) 20260926
//    D:\HT9045\config\CriticalParaControl.ini [Parameter Control] 26 鍵 —— 讀（每次開機）；缺鍵才補寫預設值
//      （CheckAndReadIniData，golden 同；W906_AUTH_PATH 可導開）。20260926 量這台：26 鍵都在 ⇒ 只讀不寫。AI(W906-FRW-S91) 20260926
//    D:\HT9045\config\Security_new.def [Input Limit] —— 只在 wb_serve.cpp 接了 W906_FRWBoot_LimitAuth 時，每次開機讀；缺鍵補寫
//      （golden 同；檔不在時 CheckFile 會整份建出來）。20260926 量這台：只有 IniConfig.bSPILFunction 為真時才會補
//      Autoclean Contact High／Low 兩鍵，其餘都在。AI(W906-FRW-S91) 20260926
//    D:\HT9045_Log\2DBarCode\LotData.txt —— 只讀（每次開機；BAR_CODE_INSTALL 是 CCD／InShtIntel／EtherNetCCD／OCR 且檔在時）。
//    D:\HT9045_Log\2DBarCode\TrayIDByLot.txt —— 只讀（每次開機；TestIF_File.bCheckTrayIDBylot 且檔在時）。
//      20260926 量這台：BAR_CODE_INSTALL=3（ebctUseCCDMode），兩個檔都不在 ⇒ 兩段都什麼都不讀。AI(W906-FRW-S94) 20260926
//
//  建置：本檔不屬於 gen_editlist.py 的產生清單（_editlist_sources.cmake 是產生檔，重跑會蓋掉），
//    照 FileRW/Teach.cpp 的前例直接列在 CMakeLists.txt 的 add_executable(wb_serve …)。
//    沒列進去 ⇒ wb_serve 連結時 undefined reference（看得見的失敗，不會靜默少做）。
//    本檔只被 wb_serve 用；不要搬進任何 archive（沒人引用時成員不會被抽出，陷阱 #2）。
// ===========================================================================
#include "forms/fLotInfo.h"      // AI(W906-FRW-WC1) 20260927: fLotInfo->ResetLotInfo（W906_FRWBoot_ResetLotInfo）
#include "cmydef.h"             // asHandlerVersion / iByBinTotal / iSVByBinCount / iSV_ErrBinCnt / iOneDayLoaderCount /
                                // bHandlerModel / InitialOK / slEventLog / CUSTOMER_CODE（MachineType.h：TEST_MAX_BIN、CC_ASE_CL）
#include "common.h"             // WriteIniDataGeneral
#include "Config.h"             // IniConfig.bN26_UseJamRawDataRecord
#include "forms/fObserver.h"    // TfObserver / fObserver（cObserver.cpp:3289，靜態初始化 new）
#include "LastSet.h"            // LastSet.bHasDownloadFile                                    AI(W906-FRW-S65) 20260926
#include "CosFunction.h"        // CosFunction.bRMSNoNeedToDownloadEveryTime                  AI(W906-FRW-S65) 20260926
#include "cprod.h"              // RunInfo（RUN_INFO：InitialDailyData／sToday／DailyJamFileName／iDailyCount／vDailyJam）、JAM_COUNT   AI(W906-FRW-S91) 20260926
#include "ProductionInfo/FileInfo.h"   // FileInfo().PathCombin（本體 ProductionInfo/FileInfo.cpp:457，ht9045_sm；本檔只編進 wb_serve）   AI(W906-FRW-S91) 20260926
#include "vclcompat/SysUtils.h"  // FileExists／StringReplace   AI(W906-FRW-S91) 20260926
#include "BarCode/BarCode_Shuttle2_Scan.h"   // map2DList（本體 BarCode/BarCode_Shuttle2_Scan.cpp:48，ht9045_sm；本檔只編進 wb_serve）   AI(W906-FRW-S94) 20260926
#include "acatchtray_shims.h"   // fTrayMapping（TfTrayMapping 替身，本體 acatchtray_shims.cpp:94）＋ TListTrayIDShim   AI(W906-FRW-S94) 20260926

#include <cstdio>
#include <cstdlib>              // getenv

// csystem.h:294 的宣告（本體 csystem.cpp:28801，golden csystem.cpp:24655）。不 include csystem.h：本檔只要這一支，
// 少拉一串標頭（陷阱 #3 的兩個 TMyKitSuck 就是從標頭鏈進來的）。
void ReadWriteBinCountMode(bool bRead);
// cprod.h:3240 的宣告（本體 cprod.cpp:2014，golden cprod.cpp:1910）。同上理由不 include cprod.h；不帶預設引數重宣告
// （cprod.h 若經別的標頭進來，重給預設引數會編譯錯），呼叫處明寫 golden 預設值 (false,false)。AI(W906-FRW-S65) 20260926
bool WriteLastDataFile(bool BackUp2, bool bNotContact);
// AI(W906-FRW-S91) 20260926（Steven 團隊）：S91 兩支用到的宣告。
//   cAuthority.h 不 include（它帶 language.h；別的 FileRW 檔寫明與 HTEditList.h 衝突），照 cAuthority.h:64／:84 原樣宣告。
//   ⓘ 本檔從 S91 起 include cprod.h（RunInfo 要完整型別）；上面 WriteLastDataFile 的不帶預設引數重宣告跟 cprod.h:3240 相容
//     （後宣告不重給預設值是合法的），所以那兩行照舊不動 —— 那段註解「不 include cprod.h」指的是 S65 當時的狀態。
void GetCriticalParaAuth();                                                     // cAuthority.h:84（本體 cAuthority.cpp:642，golden cAuthority.cpp:511）
void GetLimitAuth();                                                            // cAuthority.h:77（本體 cAuthority.cpp:315，golden cAuthority.cpp:215）
extern bool bAuthCriticalPara[26];                                              // cAuthority.h:64（cAuthority.cpp:129）
void GetTimeInfo();                                                             // cpublic.h:23（本體 cpublic.cpp:450）
// AI(W906-FRW-S94) 20260926：golden fBarCode->list2DByLot 在移植樹是全域（BarCode/BarCode_Bottom2DID.h:144，本體 BarCode_Bottom2DID.cpp:60）。
//   不 include 那個標頭：它經 BarCode_Shuttle1_Scan.h 帶進 canary_support.h（LAST_GENERAL_SET 與本檔的 LastSet.h 版面不同）
//   與 aHotPlateSubstrate.h（陷阱 #3 的 TMyKitSuck）。型別照原宣告一字不改（變數的 mangled name 不含型別，寫錯不會連結失敗）。
extern TStringList *list2DByLot;

// golden FormShow :9589-9594 的網頁版判斷（見檔頭）。回 true ＝ golden 在這之前就已經結束程式。
static bool W906_FRWBoot_ModelReadError(const char* what)
{
    if(bHandlerModel==false)                                                    //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    {
        std::printf("FileRW MainBoot: %s skipped -- bHandlerModel=false (D:\\GPIB9045\\system\\general.ini \"Model\" read error); "
                    "golden TfMain::FormShow main.cpp:9589-9594 terminates before reaching it\n", what);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//  P1　golden TfMain::FormShow main.cpp:9609（V912），在 FormShow 的 ReadLastDataFile()（:9602）之後。
//  AI(W906-FRW-Boot) 20260926: 移植樹原本沒有任何 ReadWriteBinCountMode(true) 的呼叫點 ⇒ iByBinTotal／iSVByBinCount／
//    iSV_ErrBinCnt 開機一直是 0。golden 全樹唯一的讀點就是這一行（atester.cpp:5686 那一處 golden 自己註解掉）。
//  呼叫時機：LoadMachineConfig()（ReadGeneralIni → ReadLastSetIni → ReadLastDataFile，LastSet 已讀）與
//    FileRW_HSys_ReadMainCtorKeys()（golden TfMain 建構子）之後、asHandlerVersion（golden :9761）之前 —— 跟 golden 同序。
//    TArm／TMySocket 的建構（會把 iByBinTotal 歸零，cSocket.cpp:252）在移植樹是靜態初始化（cSocket.cpp:225），
//    golden 是 TfMain 建構子（main.cpp:2192）—— 兩邊都在這次讀之前，讀進來的值不會被蓋掉。
//  ⓘ 相依（golden 同）：CUSTOMER_CODE（ReadGeneralIni）；CC_ASE_CL 且缺鍵時才用到 LastSet.iBinData32、iTestBinCount
//    （iTestBinCount 此時還是 cmydef.cpp 初值 16 —— golden 在 :9609 也是，Tester.Data 要到 DoReadLastData :9328 才讀）。
//  ⚠ 20260926 重查：盤點說「移植樹的寫 live、每跑一次就用 0 蓋 BinCount.txt」不成立 —— csystem.cpp 裡三個寫點
//    （:5170／:5227／:8161）都在 `#define ReadWriteBinCountMode W7C2_ReadWriteBinCountMode`（csystem.cpp:4179-4180，
//    空函式）之後、`#undef`（:28296）之前，全部落到空替身（檔內 SEAM S2 註解 :28195-28203 自己寫明）。
//    所以現在移植樹完全不寫 BinCount.txt；golden 的第四個寫點（main.cpp:29127，程式結束）移植樹也沒有。
//    這支先落地是對的順序：之後 SEAM S2 退役（寫點變真的）時，開機已經先讀了檔，寫回去的才是累計值。
// ---------------------------------------------------------------------------
void W906_FRWBoot_BinCountRead()
{
    if(W906_FRWBoot_ModelReadError("ReadWriteBinCountMode(true) (golden main.cpp:9609)"))
        return;

    ReadWriteBinCountMode(true);                                                //JerryYang 20250120 : add   // golden main.cpp:9609

    long lTotal=0, lSV=0;                                                      // long＋%ld：MinGW 的 msvcrt printf 不認 %lld
    int nTotal=0, nSV=0;
    for(int i=0; i<256; i++)                                                    // iByBinTotal[256]（cmydef.cpp:4939）
    {
        if(iByBinTotal[i]!=0) { nTotal++; lTotal+=iByBinTotal[i]; }
    }
    for(int i=0; i<TEST_MAX_BIN; i++)                                           // iSVByBinCount[TEST_MAX_BIN]（cmydef.cpp:6254）
    {
        if(iSVByBinCount[i]!=0) { nSV++; lSV+=iSVByBinCount[i]; }
    }
    const char* szPath=getenv("W906_BINCOUNT_PATH") ? getenv("W906_BINCOUNT_PATH") : "D:\\HT9045\\system\\BinCount.txt";
    std::printf("FileRW MainBoot: golden FormShow main.cpp:9609 ReadWriteBinCountMode(true) <- %s: "
                "iByBinTotal sum=%ld (%d bins non-zero), iSVByBinCount sum=%ld (%d non-zero), iSV_ErrBinCnt=%d, CUSTOMER_CODE=%d%s\n",
                szPath, lTotal, nTotal, lSV, nSV, iSV_ErrBinCnt, CUSTOMER_CODE,
                (CUSTOMER_CODE==CC_ASE_CL) ? " (CC_ASE_CL: Bin -> iSVByBinCount)" : " (Bin -> iByBinTotal)");
}

// ---------------------------------------------------------------------------
//  P8　golden TfMain::FormShow main.cpp:9779-9782（V912），緊接在 asHandlerVersion 算好（:9734-9777）之後。
//  AI(W906-FRW-Boot) 20260926: asHandlerVersion 的計算 Steven 20260925 已放在 wb_serve.cpp 開機序列
//    （FileRW_HSys_ReadMainCtorKeys 下一段，照 golden 非 HiSilicon／非 ASE_KaohSiung 分支），但沒有這一行寫檔 ⇒
//    開機從不蓋版本戳記（只有 HandlerSys 頁存檔 HSys.gen.inc:3911 會寫）。呼叫點必須在那段計算之後。
//  ⓘ 值：wb_serve.exe 的版本資源 3.33.906.0（tools/wb_serve.rc）⇒ 非 SPIL 是 "V3.33"（VerInfo::GetMainVersion
//    cpublic.cpp:2181），跟 BCB6 V912 exe 寫的一樣；IniConfig.bSPILFunction 為真時是 "V3.33.906.0"，BCB6 下次開機會蓋回自己的。
//  ⓘ 空字串保護：golden 在 :9761 一定先指派才到 :9781，寫空字串在 golden 是走不到的狀態。這裡若看到空字串，表示呼叫點
//    被移到計算之前（整合錯誤），不寫、印一行 —— 不蓋量產共用檔，golden 走得到的路徑行為不變。
// ---------------------------------------------------------------------------
void W906_FRWBoot_VersionStamp()
{
    if(W906_FRWBoot_ModelReadError("[Version] Ver stamp (golden main.cpp:9781)"))
        return;
    if(asHandlerVersion=="")
    {
        std::printf("FileRW MainBoot: [Version] Ver NOT written -- asHandlerVersion is empty; the call must come after the "
                    "asHandlerVersion block (golden main.cpp:9734-9777 -> :9781)\n");
        return;
    }
//    if(IniConfig.bSIGURDFunction)                                             //Sam 20230327 : 矽格寫入軟體版本客戶可以遠端讀取
//    {
        WriteIniDataGeneral("Version", "Ver", asHandlerVersion);                //2014-05-30    Dell    for ATC6.0   // golden main.cpp:9781
//    }
    std::printf("FileRW MainBoot: golden FormShow main.cpp:9781 wrote Gerneral.ini [Version] Ver=%s\n", asHandlerVersion.c_str());
}

// ---------------------------------------------------------------------------
//  P8　golden TfMain::FormShow main.cpp:11576-11580（V912），在 :11513-11526（AutoClean 那段，
//    wb_serve 的 FileRW_Cleaning_MainFormShow）之後、:11591 LotSummary.ReadFile()（W906_BootReadLotSummary）之前。
//  AI(W906-FRW-Boot) 20260926: TfObserver::ReadLoaderCount（cObserver.cpp:3561，golden cObserver.cpp:5345）與
//    StatisticalJamCount（cObserver.cpp:3320，golden :5060）本體都已翻好、ctest 有測（tests/test_observer_core.cpp:552），
//    但移植樹沒有任何呼叫點（Data.Observer 的 C 路 W906_ObserverJson 走的是 golden TfObserver::FormShow，golden
//    的 FormShow 本來就不呼叫它）⇒ iOneDayLoaderCount 開機一直是 0，Jam 統計頁的 Rate(%) 分母錯。照 golden 補在這裡。
//  ⓘ StatisticalJamCount 第一句讀 slEventLog->Path／FileName（cObserver.cpp:3335-3336）。golden 的 slEventLog 在 TfMain 建構子 new
//    （main.cpp:1550-1560）；移植樹由 LogObjects.cpp 的 W906_CreateLogObjects 建（St02 W7，照 golden 912 main.cpp:1549-1724），
//    tools/wb_serve.cpp:4166 呼叫，早於本函式的呼叫點 :4269 ⇒ 開機時已不是 NULL。⛔ 20261002 更正（St01，St02 指出）：原註「移植樹沒有、仍是 NULL」已過時。
//    下面 slEventLog!=NULL 的保護照留（golden 的呼叫端也查 NULL，LogObjects.cpp:71），行為就是 golden。
//    ReadLoaderCount 不碰 slEventLog，照 golden 呼叫。
//  ⓘ fObserver 在 cObserver.cpp:3289 靜態初始化 new，這裡一定非 NULL（golden 同樣不檢查）。
// ---------------------------------------------------------------------------
void W906_FRWBoot_JamRawDataRecord()
{
    if(W906_FRWBoot_ModelReadError("JamRawData ReadLoaderCount/StatisticalJamCount (golden main.cpp:11576-11580)"))
        return;

    bool bJamCountRun=false;
    if(IniConfig.bN26_UseJamRawDataRecord)                                      //Sam 20210304 : JamRawData 新增開關 //KaiChen 20200618 ：矽格，增加Jam統計頁面   // golden main.cpp:11576
    {
        fObserver->ReadLoaderCount();                                           // golden main.cpp:11578
        if(slEventLog!=NULL)                                                    // AI(W906-FRW-Boot) 20260926: 見上方「跟 golden 不同的一處」
        {
            fObserver->StatisticalJamCount();                                   // golden main.cpp:11579
            bJamCountRun=true;
        }
        else
        {
            std::printf("FileRW MainBoot: StatisticalJamCount (golden main.cpp:11579) skipped -- slEventLog is NULL "
                        "(golden TfMain ctor main.cpp:1550-1560 new TMyStringList is not ported)\n");
        }
    }
    std::printf("FileRW MainBoot: golden FormShow main.cpp:11576-11580 bN26_UseJamRawDataRecord=%d -> iOneDayLoaderCount=%d "
                "(ReadLoaderCount %s, StatisticalJamCount %s)\n",
                (int)IniConfig.bN26_UseJamRawDataRecord, iOneDayLoaderCount,
                IniConfig.bN26_UseJamRawDataRecord ? "ran" : "not called",
                bJamCountRun ? "ran" : "not called");
}

// ---------------------------------------------------------------------------
//  S65　golden TfMain::FormShow main.cpp:10041-10046（V912，D:\HT9045_ref；D:\HT9045 那份 V912 的 main.cpp 在 :4796 多兩行，
//    同一段是 :10043-10048），在 :9993 DoReadLastData()／:9995 ReadLastSetIni()（LastSet 已從 lastdata.dat 讀進來）與
//    :10011 fConfiguration->ChangeCBListProperty() 之後、:10047 fNote->Timer1 啟動與 :10052 Auto Site Mapping 之前。
//  AI(W906-FRW-S65) 20260926（Steven 團隊）：S65 盤點 golden 全樹 23 個 WriteLastDataFile 呼叫點，這一處移植樹原本沒有
//    （wb_serve 開機序列沒有 :10041 這段）⇒ IniConfig.bShowLotInfo 的機台開機時 LastSet.bHasDownloadFile 不會被清掉，
//    上一次下載配方留下的 true 會一路帶到下一次開機（golden JerryYang 20160426 的註解就是在防這件事：
//    「避免 LastSet.bHasDownloadFile 被存為 true 時，發生沒有從 server 載入檔案的情況」）。照 golden 補在這裡。
//  ⓘ 相依（golden 同）：IniConfig.bShowLotInfo（ReadLastSetIni／CustomerFunctionSelect／cprod.cpp:3985 起的強制開）、
//    CosFunction.bRMSNoNeedToDownloadEveryTime（CustomerFunctionSelect，LoadMachineConfig 裡已跑）。
//  ⚠ 寫檔：WriteLastDataFile() 整塊寫 LastSet（sizeof(LAST_GENERAL_SET)）到 D:\HT9045\system\lastdata.dat 與
//    lastdata_backup.dat（寫死路徑）。golden 每次開機都寫（bShowLotInfo 為真時），不是只在值改變時。
//  呼叫時機（整合者，wb_serve.cpp 開機序列）：W906_DoReadLastData(true, …)（含 golden :9995 ReadLastSetIni）之後、
//    W906_BootTestCategory（golden :10594）與 PumpInit（InitialOK）之前 —— 跟 golden 同序。只在開機（換配方
//    ChangeSetUpFile 不跑 FormShow）。
// ---------------------------------------------------------------------------
void W906_FRWBoot_ShowLotInfoDownloadFlag()
{
    if(W906_FRWBoot_ModelReadError("bShowLotInfo lastdata.dat write (golden main.cpp:10041-10046)"))
        return;

    const bool bBefore=LastSet.bHasDownloadFile;
    bool bWrote=false;
    if(IniConfig.bShowLotInfo)                                                  //JerryYang 20160426 避免 LastSet.bHasDownloadFile被存為true時,發生沒有從server載入檔案的情況   // golden main.cpp:10041
    {
        if(CosFunction.bRMSNoNeedToDownloadEveryTime==false)                    //Steven 20240926 : RMS不要每次下載包成功能
            LastSet.bHasDownloadFile=false;                                     //Steven 20110125
        bWrote=WriteLastDataFile(false, false);                                 // golden main.cpp:10045 WriteLastDataFile();（預設引數 false,false；golden 不看回傳）
    }
    std::printf("FileRW MainBoot: golden FormShow main.cpp:10041-10046 bShowLotInfo=%d bRMSNoNeedToDownloadEveryTime=%d "
                "bHasDownloadFile %d->%d, lastdata.dat %s\n",
                (int)IniConfig.bShowLotInfo, (int)CosFunction.bRMSNoNeedToDownloadEveryTime,
                (int)bBefore, (int)LastSet.bHasDownloadFile,
                IniConfig.bShowLotInfo ? (bWrote ? "written" : "WRITE FAILED") : "not written (bShowLotInfo=0)");
}

// ---------------------------------------------------------------------------
//  WC-1　golden TfMain::FormShow main.cpp:10991 fLotInfo->ResetLotInfo();（:10041-10046 bHasDownloadFile=false 之後）
//  AI(W906-FRW-WC1) 20260927（Steven 團隊）：GATE WC-1 退役；Jimmy 20260927 00:5x（TO_STEVEN §4）「WC-1 歸 St01」。
//    機台內有 IC（開機時從 lastdata.dat 讀回的狀態）而且 bShowLotInfo：讀 config.ini 的 Product Name／Temp
//    （缺鍵補寫空字串，golden 同；W906_AUTH_PATH 可導開），bHasDownloadFile=true（只改記憶體）。
// ---------------------------------------------------------------------------
void W906_FRWBoot_ResetLotInfo()
{
    if(W906_FRWBoot_ModelReadError("fLotInfo->ResetLotInfo (golden main.cpp:10991)"))
        return;
    const bool bBefore=LastSet.bHasDownloadFile;
    fLotInfo->ResetLotInfo();                                                   //Steven 20240925 : 重開軟體時, 要讀回lot info   // golden main.cpp:10991
    std::printf("FileRW MainBoot: golden FormShow main.cpp:10991 ResetLotInfo -- bShowLotInfo=%d, Product Name=\"%s\" Temp=\"%s\", bHasDownloadFile %d->%d\n",
                (int)IniConfig.bShowLotInfo, fLotInfo->edDeviceName->Text.c_str(), fLotInfo->edTemp->Text.c_str(),
                (int)bBefore, (int)LastSet.bHasDownloadFile);
}

// ---------------------------------------------------------------------------
//  S91-a　golden TfMain::FormShow main.cpp:9607（V912 主 repo）RunInfo.ReadJamRateByDay()
//    —— 在 :9604 ReadLastDataFile()／:9606 LogSoftwareOnTime 之後、:9611 ReadWriteBinCountMode(true)
//    （W906_FRWBoot_BinCountRead）之前。只在開機（golden 全樹唯一的呼叫點就是這一行；換配方不跑 FormShow）。
//  AI(W906-FRW-S91) 20260926（Steven 團隊）：普查 S91。移植樹沒有任何 RunInfo.ReadJamRateByDay() 的呼叫點
//    ⇒ 開機時 RunInfo.iDailyCount／vDailyJam 一直是空的（當天稍早累計的 Jam 次數沒讀回來）。
//  ⚠ 為什麼本體寫在這裡、不直接呼叫 RUN_INFO::ReadJamRateByDay（cprod.cpp:1195）：
//    移植樹那一支的本體（InitialDailyData 之後的全部）還在 `#if 0 // TODO(GA1-B2)`（cprod.cpp:1201-1257），
//    擋它的理由「FileInfo 沒翻」已經不成立（ProductionInfo/FileInfo.cpp:457 PathCombin 已翻）。
//    但 cprod.cpp 在 ht9045_globals（最低一層），FileInfo.cpp 在 ht9045_sm；直接解閘會讓 cprod.o 多一個對 sm 的
//    未定義符號，只連 globals 的 6 支測試（FileRW/_fallback.cpp 檔頭列的 test_cUnitConvert 等）會連結失敗 ——
//    就是 05f2695b 踩過的那一次。本檔只編進 wb_serve（連得到 sm），所以在這裡照 golden 逐行寫一份，
//    `this->` 換成 `RunInfo.`；cprod.cpp 那一支不動（改法見交件報告：搬 FileInfo.cpp 到 globals 後解閘，本函式改成一行呼叫）。
//  跟 golden 不同的三處（前提不在或 C++ 語法，不是改行為）：
//    ① 先呼叫一次 GetTimeInfo()：golden 的 SystemYear／Month／Date 在 TfMain 建構子 main.cpp:1732 GetTimeInfo() 就更新了；
//       wb_serve 對應的那一次在 tools/wb_serve.cpp:4066（InitialHandler 前），比這個呼叫點晚 ⇒ 不先更新，InitialDailyData
//       會拿哨兵 9999（cmydef.cpp:292）組出 sToday="9999-9999-00"，找不到檔。GetTimeInfo 只讀時鐘，重複呼叫無副作用。
//    ② sprintf 的 AnsiString 引數加 .c_str()（BCB6 AnsiString 可以直接穿過 `...`，C++ 不行）。
//    ③ 格式壞的行（逗號切出來不足 3 段）：golden TStringList 索引越界會丟 EStringListError（FormShow 中斷）；
//       vclcompat 回空字串（vclcompat/TStringList.cpp:76-79）⇒ 那一行記成 JamCode=""、iCount=0，其餘照讀。
// ---------------------------------------------------------------------------
static void W906_RunInfo_ReadJamRateByDay()                                     // golden cprod.cpp:1112 void RUN_INFO::ReadJamRateByDay()（this = &RunInfo）
{
    AnsiString str, sFileName;
    int iPos;
    RunInfo.InitialDailyData();

    MyForceDirectories(sDailyJamPath);

    sFileName.sprintf("%s_%s_%s_DailyJamRate.txt", IniConfig.sMachineType.c_str(), IniConfig.SocketHandlerID.c_str(), RunInfo.sToday.c_str());
    RunInfo.DailyJamFileName=FileInfo().PathCombin(sDailyJamPath, sFileName);   //Steven 20250812 : 修正上傳檔名

    if(FileExists(RunInfo.DailyJamFileName)==false)
        return;

    TStringList *slReport=new TStringList();
    TStringList *sList=new TStringList();
    slReport->LoadFromFile(RunInfo.DailyJamFileName);
    sList->Delimiter=' ';

    for(int i=0; i<slReport->Count; i++)
    {
        str=slReport->Strings[i];
        if(str.AnsiPos("Date:")==1)
        {
        }
        else if(str.AnsiPos("Unloading counter:")==1)
        {
            iPos=str.AnsiPos(":");
            str=str.SubString(iPos+1, str.Length());
            RunInfo.iDailyCount=atoi(str.c_str());
        }
        else if(str.AnsiPos("Jam counter:")==1)
        {
        }
        else if(str.AnsiPos("MUBJ:")==1)
        {
        }
        else
        {
            sList->Clear();
            sList->CommaText=str;
            JAM_COUNT Temp;
            Temp.JamCode=sList->Strings[1];
            Temp.iCount =atoi(AnsiString(sList->Strings[2]).c_str());
            sList->Delete(2);
            sList->Delete(1);
            sList->Delete(0);
            str=StringReplace(AnsiString(sList->Text), "\r\n", " ", TReplaceFlags()<<rfReplaceAll);
            Temp.Message=str;
            RunInfo.vDailyJam[Temp.JamCode]=Temp;
        }
    }

    sList->Clear();
    slReport->Clear();
    delete slReport;
    delete sList;
}

void W906_FRWBoot_JamRateByDayRead()
{
    if(W906_FRWBoot_ModelReadError("RunInfo.ReadJamRateByDay() (golden main.cpp:9607)"))
        return;

    GetTimeInfo();                                                              // AI(W906-FRW-S91) 20260926: 見上方 ①（golden TfMain 建構子 main.cpp:1732 的前提）
    W906_RunInfo_ReadJamRateByDay();                                            //Steven 20250527 : By Lot Jam Rate   // golden main.cpp:9607

    const bool bExists=FileExists(RunInfo.DailyJamFileName);
    std::printf("FileRW MainBoot: golden FormShow main.cpp:9607 RunInfo.ReadJamRateByDay() <- %s: %s, iDailyCount=%d, "
                "vDailyJam=%u code(s)\n",
                RunInfo.DailyJamFileName.c_str(), bExists ? "read" : "no file for today (nothing read)",
                RunInfo.iDailyCount, (unsigned)RunInfo.vDailyJam.size());
}

// ---------------------------------------------------------------------------
//  S91-b　golden TfMain::FormShow main.cpp:9787（V912 主 repo）GetCriticalParaAuth()
//    —— 在 :9783 WriteIniDataGeneral("Version","Ver",…)（W906_FRWBoot_VersionStamp）與 :9786 GetLimitAuth() 之後、
//    :9788 RunInfo.MachineDefine=fMain->Caption 之前。只在開機。
//  AI(W906-FRW-S91) 20260926（Steven 團隊）：普查 S91。本體 cAuthority.cpp:642 與 golden cAuthority.cpp:511 逐行相同
//    （20260926 diff 過，funCriticalPara[] 26 個鍵名也相同），但移植樹沒有活的呼叫點（唯一的另一處 Command.cpp:9733
//    TfMain::FTPDownloadByDLL 在 `#if 0 GATE(FW3-WC)`，屬 Steven02／Jimmy，不動）⇒ bAuthCriticalPara[26] 一直是 0，
//    「開批後鎖 critical parameter」（CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart，
//    例 FileRW/ArmSpeed_File.gen.inc:1310）永遠不鎖任何欄位。
//  ⓘ golden :9786 的鄰居 GetLimitAuth() 移植樹開機段也沒有（只在各頁 FormShow 裡呼叫）；不屬 S91，交件報告列給 Steven 決定。
//  ⚠ 寫檔（golden 同）：CheckAndReadIniData(int) 在 [Parameter Control] 缺某個鍵時會把預設值寫回
//    D:\HT9045\config\CriticalParaControl.ini（AuthPath；TIniFile 立即寫）。預設值 =1 的 16 個：
//    Temperature_Setting_Value、Soake_Time、Bin_Setting、Test_Site_Assign_Handling_Mode_Site_Map、Contact_Force、
//    Socket_Pitch、Package_Dimensions_Pin_Count、Contact_Parameter、Double_Device_Height、Temperature_Mode、
//    Socket_Sensor、PiggyBackFunction、Tray_Form、Plate_Form、Tray_Assign、Test_IF；=0 的 10 個：Active_Site、
//    Vacuum_On_Off_Time、Drop_Wait_Time、Contact_Speed、QA_Sampling_Count、Index_Unit_Speed、Yield_Site_To_Site、
//    Yield_Continous_Fail、Yield_Yield、ACC_Function。檔不在會整份建出來。鍵都在就只讀。
// ---------------------------------------------------------------------------
void W906_FRWBoot_CriticalParaAuth()
{
    if(W906_FRWBoot_ModelReadError("GetCriticalParaAuth() (golden main.cpp:9787)"))
        return;

    GetCriticalParaAuth();                                                      //JerryYang 20220311 : ATP鎖定Critical parameter   // golden main.cpp:9787

    int nOn=0;
    for(int i=0; i<26; i++)
        if(bAuthCriticalPara[i]) nOn++;
    std::printf("FileRW MainBoot: golden FormShow main.cpp:9787 GetCriticalParaAuth() <- %sCriticalParaControl.ini: "
                "%d of 26 locked-when-lot-started (bLotStartLockCriticalPara=%d)\n",
                AuthPath.c_str(), nOn, (int)CosFunction.bLotStartLockCriticalPara);
}

// ---------------------------------------------------------------------------
//  S91 的鄰居　golden TfMain::FormShow main.cpp:9786（V912 主 repo）GetLimitAuth()　//Steven 20161004 : 要先取得權限
//    —— 緊接在 :9787 GetCriticalParaAuth() 之前（同一段），也在 :9993 DoReadLastData() 之前。
//  AI(W906-FRW-S91) 20260926（Steven 團隊）：不在 S91 清單，是接 :9787 時找到的鄰居。本函式先寫好，
//    **要不要在 wb_serve.cpp 呼叫由 Steven 決定**（交件報告 5 的 Q2；片段有「含」與「不含」兩個版本）。
//  為什麼值得接：GetLimitAuth() 填 InputLimit（cprod.cpp:55 全域，零初始化）。移植樹只在各頁 FormShow 呼叫它
//    （FileRW/*.gen.inc 的 DF_／OS_／SC_／TS_FormShow、cStartCondition.cpp:182），開機鏈沒有 ⇒ wb_serve 開機時
//    W906_DoReadLastData 跑的 ReadTempFile 用的是全 0 的上下限：
//      uTemp_Set.cpp:2424  （一般情況）dTempOffset[0][i]=ReadWriteIni(…"User OffSet"…, true, InputLimit.iTempHigh, InputLimit.iTempLow)
//      uTemp_Set.cpp:2855  （bTemp5PointKitOffset 為假時）Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(…"ATC"…), iTempHigh, iTempLow)
//    CheckRange(x, 0, 0) 恆為 0 ⇒ 開機讀進記憶體的溫度 User Offset／ATC offset 都被夾成 0（檔案本身不改：
//    讀的那一支 ReadWriteIni 不寫回，common.cpp ReadWriteIni(double) 的 bIsRead 分支）。直到有人開某一頁
//    （FormShow 叫 GetLimitAuth）再重讀才會回到檔案值。golden 在 :9786 先讀上下限，所以 BCB6 開機沒有這件事。
//    DeviceForm_File 的 ForcePerPin 上下限（FileRW/DeviceForm_File.gen.inc DF_ReadFile）同理。
//  ⚠ 寫檔（golden 同）：config\Security_new.def [Input Limit] 缺鍵時補寫預設值（CheckAndReadIniData）；
//    檔不在時 CheckFile（cAuthority.cpp:503）會先整份建出 [Main]／[Tool]／…的預設值。
// ---------------------------------------------------------------------------
void W906_FRWBoot_LimitAuth()
{
    if(W906_FRWBoot_ModelReadError("GetLimitAuth() (golden main.cpp:9786)"))
        return;

    GetLimitAuth();                                                             //Steven 20161004 : 要先取得權限   // golden main.cpp:9786

    std::printf("FileRW MainBoot: golden FormShow main.cpp:9786 GetLimitAuth() <- %sSecurity_new.def [Input Limit]: "
                "Temp %d..%d, OffsetXY %d..%d, OffsetZ %d..%d, Contact %.2f..%.2f\n",
                AuthPath.c_str(), InputLimit.iTempLow, InputLimit.iTempHigh,
                InputLimit.iOffsetXYLow, InputLimit.iOffsetXYHigh, InputLimit.iOffsetZLow, InputLimit.iOffsetZHigh,
                InputLimit.dContactLow, InputLimit.dContactHigh);
}

// ---------------------------------------------------------------------------
//  S94　golden TfMain::FormShow main.cpp:11344-11365（V912 主 repo）開機讀回「依批號」的兩份清單
//    —— 在 :11342 ResetShuttleWhichKit() 之後、:11366 InitialHT9045SModule() 之前；golden 全樹只有這一處讀回
//    （換配方 cbSetupFileNameChange／ChangeSetUpFile 不再讀，所以本函式不放進 W906_DoReadLastData）。
//    wb_serve.cpp 接在 :11191 fLaserSensor->ReadLaserFile()（W906_RC_LaserReadLaserFile）那一行的後面 —— 那一行已經在
//    W906_DoReadLastData(true) 之後（TestIF_File.bCheckTrayIDBylot 是配方值，要先讀配方）。
//  AI(W906-FRW-S94) 20260926（Steven 團隊）：普查 S94。移植樹開機鏈原本沒有這一段 ⇒ 重開 wb_serve 之後 2D 重複碼記憶
//    （map2DList）是空的：這一批已經讀過的 2D 碼再出現時不會被判成重複；golden 會（LotData.txt 讀回來了）。
//    golden 全樹沒有別處把 map2DList 整個清掉（只有 btClearBarcodeListClick；Initial*Scan 系列只 erase 單筆），讀回來的會一直留著。
//  ⓐ golden fBarCode->list2DByLot／fBarCode->map2DList：移植樹 TfBarCode 沒有這兩個成員，改成兩個全域
//    （list2DByLot：BarCode/BarCode_Bottom2DID.h:144，本體 BarCode_Bottom2DID.cpp:60；map2DList：BarCode/BarCode_Shuttle2_Scan.h:141，
//    本體 BarCode_Shuttle2_Scan.cpp:48）—— Bottom2DID／Shuttle2_Scan／Shuttle1_Scan／Bottom2DID8CCD／asortarm 用的就是這兩個。
//    ⚠ BarCode/BarCode_Shuttle1_CCDScan.cpp:211 另有一份 TU 內的 map2DList（匿名 namespace；該檔檔頭 SHARED-STATE CAVEAT），
//      這裡碰不到 —— 那個單元的重複碼記憶開機後仍是空的（既有差異，不是本次造成的）。
//  ⓑ golden fTrayMapping->listTrayIDByLot->LoadFromFile(asTrayIDByLot)：移植樹活的 fTrayMapping 是 acatchtray_shims.h 的
//    TfTrayMapping（不是 forms/fTrayMapping.h 的 TfTrayMappingForm —— 兩個不同的類別，見該檔檔頭第 1 節），它的 listTrayIDByLot
//    是 TListTrayIDShim（只有 Text／Clear／Add，沒有 LoadFromFile）。所以先用 TStringList 讀檔、再逐行 Add：
//    結果的 Text ＝ 每行後面接 "\r\n"，跟 VCL TStringList::LoadFromFile 之後的 Text 相同。
//    ⓘ 這一份讀回在 golden 就看不出效果：listTrayIDByLot 的讀者 DoTrayIDCheck（golden cTrayMapping.cpp:7370）每次先
//      Clear＋LoadFromFile 自己重讀；寫者（golden acatchtray.cpp:4558-4560／:4685-4687）先 Clear 再 Add 一筆；移植樹 shim 的
//      DoTrayIDCheck 固定回 true（acatchtray_shims.h 的 SEMANTIC NOTE）。照翻是為了開機狀態跟 golden 一樣，不是為了行為。
//  只讀，不寫任何檔（兩個檔不在就什麼都不做 —— golden :11349／:11361 的 FileExists 保護）。
// ---------------------------------------------------------------------------
void W906_FRWBoot_LotListsRead()
{
    if(W906_FRWBoot_ModelReadError("lot lists read back (golden main.cpp:11344-11365)"))
        return;

    int n2D=-1, nTray=-1;                                                       // 只給下面的 printf：-1＝golden 沒進去或檔不在
    if(BAR_CODE_INSTALL==ebctUseCCDMode     ||                                  //Steven 20160429 : 開程式要把2D List讀回來
       BAR_CODE_INSTALL==ebctInShtIntel     ||                                  //wei 20160505 Barcode 比對Lot
       BAR_CODE_INSTALL==ebctEtherNetCCD    ||                                  //Ifor 20190129 : add Cognex EtherNet 通訊
       BAR_CODE_INSTALL==ebcUseOCR           )                                  //Ifor 20210407 add: 自製OCR
    {
        if(FileExists(asBarCodeLot))                                            //Steven 20160505 : 加上保護, 不然開程式會跳Error
        {
            list2DByLot->LoadFromFile(asBarCodeLot);                            // golden fBarCode->list2DByLot（ⓐ）
            for(int i=0; i<list2DByLot->Count; i++)
            {
                map2DList[list2DByLot->Strings[i]]="1";                         // golden fBarCode->map2DList（ⓐ）
            }
            n2D=list2DByLot->Count;
        }
    }

    if(TestIF_File.bCheckTrayIDBylot)                                           //JerryYang 20250120 : add
    {
        if(FileExists(asTrayIDByLot))                                           //Steven 20160505 : 加上保護, 不然開程式會跳Error
        {
            // golden: fTrayMapping->listTrayIDByLot->LoadFromFile(asTrayIDByLot);（ⓑ）
            TStringList slTrayID;
            slTrayID.LoadFromFile(asTrayIDByLot);
            fTrayMapping->listTrayIDByLot->Clear();
            for(int i=0; i<slTrayID.Count; i++)
                fTrayMapping->listTrayIDByLot->Add(slTrayID.Strings[i]);
            nTray=slTrayID.Count;
        }
    }

    std::printf("FileRW MainBoot: golden FormShow main.cpp:11344-11365 lot lists: BAR_CODE_INSTALL=%d, %s %s; "
                "bCheckTrayIDBylot=%d, %s %s\n",
                BAR_CODE_INSTALL, asBarCodeLot.c_str(),
                n2D<0 ? "not read" : (AnsiString(n2D)+" line(s) -> list2DByLot / map2DList").c_str(),
                (int)TestIF_File.bCheckTrayIDBylot, asTrayIDByLot.c_str(),
                nTray<0 ? "not read" : (AnsiString(nTray)+" line(s) -> fTrayMapping->listTrayIDByLot").c_str());
}

// ---------------------------------------------------------------------------
//  FTP-START　golden TfMain::FormShow main.cpp:11495-11513（V912 主 repo D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy）
//    「FTP 下載資料比對」的開機快照 —— 在 :11247-11250（A14 才把 cbSetupFileName 抄進 edSetupFileName）之後、
//    :11515 fCleaning->LoadAutoCleanData()（wb_serve 的 FileRW_Cleaning_MainFormShow）之前。只在開機
//    （golden 全樹只有 FormShow 這一處開機快照；換配方 cbSetupFileNameChange／ChangeSetUpFile 不再抄）。
//    St02 20260928 12:16 引的是 906 golden main.cpp:11054-11066；906 那棵樹（HT9011UC_Code_V3.33.906.0_20260625_Steven）
//    不在這台，這裡照 V912 逐行翻。檔頭清單不加列（免得移動本檔行號），本段就是說明。
//  AI(W906-FTP-START) 20260928 [W906]（Steven 團隊 St01）：
//    START 前的比對 TfMainWeb::NETDownloadDataCheck（WebStart.cpp:751；golden V912 main.cpp:31472）拿「本機現值」跟
//    *_NET 逐欄比。golden 有四個地方寫 *_NET：這裡（開機）、FTP 下載成功（KYECFTP\FTPClient.cpp:1064-1084）、
//    RMS 下載（uLotInfo.cpp:2843-2859，只有 SCC／AMD）、ARMS（ARMS\ARMS.cpp:654-667，只有 ASE_M）。移植樹原本一個都沒有
//    ⇒ DownloadWorkFile_NET 一直是 ""、LastSetTemperature_NET 一直是 -1（cmydef.cpp:3774-3775）⇒ 出貨版、
//    [FTP] Enable FTP 有勾、On-Line、不是 PE 模式、CosFunction.bUseFTPDownloadDataCheck 為真（InitialCosFunction 預設 true，
//    CosFunction.cpp:4419）的機台，每一次按 START 都在 WebStart.cpp:799-803「Temperature Mode Check Error!!」被擋。
//    golden 開機時 LoadMachineRecord 把 bFTPDownloadSetupFile 設 true（移植樹 cinitial.cpp:10236，golden cinitial.cpp:8448），
//    接著在這裡把現值抄進 *_NET ⇒ 開機後沒改過東西就能 START；改了比對的欄位（Bin、溫度模式、Tray Form、Test IF…）才擋。
//  跟 golden 不同的一處（前提不在，不是改行為）：
//    golden :11500 讀 edSetupFileName->Text。它是 main.dfm:1742 的 TEdit，dfm 沒有 Text 屬性 ⇒ 初值 ""；開機唯一的寫點是
//    :11247-11250 `if(IniConfig.bA14UseBarCodeSetWorkFile) edSetupFileName->Text=cbSetupFileName->Text;`
//    （其他寫點 :32946-32984 是操作員在那一格按 Enter 的 edSetupFileNameKeyUp）。移植樹沒有 edSetupFileName 這個元件，
//    所以用區域變數照這兩行重建它開機時的值。
//    ⓘ 比對只看「SYS_SetupFile 等不等於 DownloadWorkFile_NET」（golden main.cpp:31513），而兩者下一行就設成一樣；
//      照重建是為了讓值也跟 golden 一樣：開機後第一次 FTP 下載的記錄行（FTPClient.cpp:1029-1030 "Download file : "+
//      DownloadWorkFile_NET，那一刻還是下載前的舊值，見 WebStart.cpp 檔尾）印的就是這裡設的值。
//  ⚠ 條件成立時會多跑一次 SetWorkParameter()（golden :11498，Ifor 20080105「移至 main 內處理，避免檔案不齊全導致
//    Data Check 異常」）。wb_serve 開機序列已經跑過它好幾次（例 tools/wb_serve.cpp:3591），這一次讀寫的檔跟那幾次相同，
//    不新增碰到的檔。這台實驗機（CUSTOMER_CODE=791 盛合晶微，非海思版 CosFunction.cpp:1734 設 false）條件不成立，什麼都不做。
//  呼叫時機（整合者，wb_serve.cpp 開機序列）：tools/wb_serve.cpp:4269 同一行、`FileRW_Cleaning_MainFormShow()` 的前面
//    —— 那一行是 golden :11515-11528（該行與 FileRW/TestIF_File_Cleaning.cpp 註解寫的 :11513-11526 是已移除的 D:\HT9045_ref
//    行號，少 2 行），本段是 golden :11495-11513，緊鄰在前。前提都已滿足：cbSetupFileName->Text（:4111）、
//    InitialHandler→LoadMachineRecord（bFTPDownloadSetupFile，:4066）、W906_DoReadLastData(true)（配方，:4159）、PumpInit（:4212）。
//    ⚠ 20260928 wb_serve.cpp 屬主畫面批次（別人正在改），本檔只放本體；那一行由主畫面批次接（交件報告的認領清單）。
//    沒接之前這支沒有呼叫者，START 照舊被擋（看得見的失敗，不會靜默少做）。
// ---------------------------------------------------------------------------
#include "forms/fMain.h"        // fMain->cbSetupFileName->Text（golden main.h:875 TComboBox*）   AI(W906-FTP-START) 20260928
// cinitial.h:76 的宣告（本體 cinitial.cpp:7084）。不 include cinitial.h：本檔只要這一支（同上方 ReadWriteBinCountMode 的理由）。
bool SetWorkParameter();

void W906_FRWBoot_FTPDownloadDataSnapshot()
{
    if(W906_FRWBoot_ModelReadError("FTP download data check snapshot (golden main.cpp:11495-11513)"))
        return;

    // golden main.cpp:11247-11250 —— 重建 edSetupFileName->Text 開機時的值（見上方「跟 golden 不同的一處」）
    AnsiString edSetupFileName_Text="";                                         // golden main.dfm:1742 TEdit（沒有 Text 屬性 ⇒ ""）
    if(IniConfig.bA14UseBarCodeSetWorkFile)                                     //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔   // golden main.cpp:11247
    {
        edSetupFileName_Text=fMain->cbSetupFileName->Text;                      // golden main.cpp:11249
    }

    bool bRecorded=false;
    if(CosFunction.bUseFTPDownloadDataCheck==true &&                            //Ifor 20080105 (Steven) : 移至main內處理，避免檔案不齊全導致Data Check 異常   // golden main.cpp:11495
       bFTPDownloadSetupFile==true)                                             //Ifor 20180125 : Use FTP Download Data Check   // golden main.cpp:11496
    {
        SetWorkParameter();                                                     // golden main.cpp:11498
        // record data
        SYS_SetupFile           = edSetupFileName_Text;                         // golden main.cpp:11500 SYS_SetupFile=edSetupFileName->Text;
        DownloadWorkFile_NET    = SYS_SetupFile;                                // golden main.cpp:11501
        LastSetTemperature_NET  = LastSet.iTemperature;                         // golden main.cpp:11502
        TrayForm_NET            = TrayForm;                                     // golden main.cpp:11503
        HotPlateForm_NET        = HotPlateForm_File;                            // golden main.cpp:11504
        Temperature_NET         = Temperature;                                  // golden main.cpp:11505
        TestIF_NET              = TestIF_File;                                  // golden main.cpp:11506
        DeviceForm_NET          = DeviceForm_File;                              // golden main.cpp:11507

        for(int i=0; i<8; i++)                                                  //ChungHung 20141002 add for KYEC AutoRetest 3->5   //Ifor 20170418 (wei) add MRT Mode 5 -> 8   // golden main.cpp:11509
        {
            BinSelect_NET[i]    = BinSelect[i];                                 // golden main.cpp:11511
        }
        bRecorded=true;
    }

    std::printf("FileRW MainBoot: golden FormShow main.cpp:11495-11513 FTP download data snapshot -- bUseFTPDownloadDataCheck=%d "
                "bFTPDownloadSetupFile=%d bA14UseBarCodeSetWorkFile=%d -> %s (SYS_SetupFile=\"%s\", LastSetTemperature_NET=%d)\n",
                (int)CosFunction.bUseFTPDownloadDataCheck, (int)bFTPDownloadSetupFile, (int)IniConfig.bA14UseBarCodeSetWorkFile,
                bRecorded ? "recorded *_NET (SetWorkParameter ran)" : "not recorded (golden condition false)",
                SYS_SetupFile.c_str(), LastSetTemperature_NET);
}

// ---------------------------------------------------------------------------
//  SYSINIT　golden TfMain::FormShow main.cpp:9659 `SystemInitialOK=true;`（golden 906 line numbers in this block;
//    the V912 main repo has it at :10092）
//  AI(W906-SYSINIT) 20260930: translate golden's ONLY writer of SystemInitialOK (INBOX 127). Before this the port had no
//    writer outside tests -- the definition `bool SystemInitialOK=false;` (cmydef.cpp:332) was all there was -- so every
//    reader behaved as "system not initialised" for the whole life of wb_serve (FileRW/MainRecord.cpp:60-61 noted it).
//    golden context, 906 main.cpp:9610-9661:
//        if(IniConfig.bShowLotInfo){...WriteLastDataFile();}   :9610-9615  = W906_FRWBoot_ShowLotInfoDownloadFlag (above)
//        fNote->Timer1->Enabled=true; TimerScanKey->Enabled=true;   :9616-9617  (no timers in the port)
//        auto-site-map / SetRunStartMode(LastSet.iRunStartMode)     :9619-9654  NOT in the port's boot sequence (see below)
//        fShowMessage->UpdateForm(20);                              :9656
//        SW[SwServerON].On();                                       :9658  NOT translated here
//        SystemInitialOK=true;                                      :9659  <- this function
//        if(INDEX_MOTION_CARD==0) Open_GaliCard();                  :9660-9661  NOT translated here
//    Lifetime: golden sets it once, unconditionally, and never clears it -- :9659 is the only assignment in the golden
//      tree (search 20260930: 21 occurrences in *.cpp/*.h, one write, plus the cmydef.cpp:328 initialiser). Same here:
//      one call from the wb_serve boot sequence, no reset anywhere.
//    Gate: golden FormShow returns at :9162-9167 (bHandlerModel==false -> MessageDlg + Application->Terminate()) long
//      before :9659, so a model-read error leaves it false -- the same W906_FRWBoot_ModelReadError skip the other boot
//      steps in this file use.
//    What turns live (non-test readers, census 20260930):
//      ScanPannelKey (ckernel.cpp:3152, golden ckernel.cpp:1924) in the three wb_serve wait loops (alarm, ShowMyMessage,
//        YES/NO); DoPanelLamp (ckernel.cpp:2817, golden :1730) on every 6th DoSystem pass;
//      StopAllMotor(true) in ShowMyMessage / MyMessageBox FormShow / YES-NO FormShow (tools/wb_serve.cpp:6870 / :6688 /
//        :6097, golden mymessbox.cpp:811-812 / :306-307);
//      the bProgramStartOnLine ON_LINE/REALLY forcing in ReadLastDataFile / ReadTestMode becomes boot-only (cprod.cpp:1803
//        / :3676, golden cprod.cpp:1700 / :3467; CosFunction sets it only for CC_Greatek, CosFunction.cpp:1621).
//    Not translated neighbours (task scope): :9658 SW[SwServerON].On() -- the port's DoSystem automatic motor power-on
//      turns it on (csystem.cpp:16109-16110, golden csystem.cpp:4116-4117); :9660-9661 Open_GaliCard() -- defined
//      (Motor/myGALILmotor.cpp:4090) with no caller in the port; not the HT9050 path.
//    ⚠ If golden :9619-9654 (the boot SetRunStartMode -> ReadTestMode) is ever translated, it goes BEFORE the call below:
//      golden runs that ReadTestMode with SystemInitialOK still false, i.e. with the on-line forcing active.
//  Call site (integrator, tools/wb_serve.cpp boot sequence): the W906_FRWBoot_ShowLotInfoDownloadFlag line, right after
//    that step (golden :9610-9615, before :9659) and before W906_BootTestCategory (V912 :10594), fCounterClear->ReadCTInfo
//    (V912 :10596), MyDBUpdateDB (906 :10222) and PumpInit (InitialOK=true, 906 :10464) -- golden's order.
//    Pinned by ctest SysinitBoot (tests/test_sysinit_boot.cpp).
// ---------------------------------------------------------------------------
void W906_FRWBoot_SystemInitialOK()
{
    if(W906_FRWBoot_ModelReadError("SystemInitialOK=true (golden main.cpp:9659)"))
        return;

    SystemInitialOK=true;                                                       // golden main.cpp:9659
    std::printf("FileRW MainBoot: golden FormShow main.cpp:9659 SystemInitialOK=true -- panel keys (ScanPannelKey), panel "
                "lamps (DoPanelLamp) and ShowMyMessage's StopAllMotor are live from here on\n");
}
