# ELA 報表與上傳計畫（#22／W21／W22）

> St02 20260927。**整份都是「計畫，未實作」**，除非標明「已在 main」或「已做」。**R0 已做；R1／R2 已做**（St02-E 20260927，`v906/steven-ela-wip` `99dc54ba`／`1b6bf39e`，只編譯、未上機，ctest `ELA_Reports` 待 St01）。
> 依據：St02 的 #22 唯讀盤點（20260927，已送 github-59）。Handler 行號＝**906_0625_Steven**；分析器行號＝**SVN Rev891**
> （`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code`，FileVersion 20.25.891.0）。帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 的 Handler 行號是 V912 的，
> 對應：906 main.cpp:21446＝V912 :22066；906 HS_Function.cpp:268＝V912 :272。
>
> Steven 的裁決：
> - **#22**：先做 [O06]／[O19]／[N25-3]／[N10]／[N34] 這五個，一起做。
> - **W22**：「功能有開的都要做」——其他客戶報表，功能開關有開就做，排在後面。
> - **W21**（推翻原本暫定的 A）：依時間的排程**放進 ElaHub**；上傳要**檢查成功**，或**錯開／依條件觸發**（多台同時上傳曾把伺服器塞爆）。
> - **W20**：分析器與 Handler 的 Jam Code 編輯器合成一個（另案）。
> - **W13**：UploadProdLog（N17）01:00 的觸發進 ElaHub 排程（見第 2 節）。
> - **W15＝B／W16＝A／W17＝A／W18＝B／W19＝B／W23**：見 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\SKILL.md` §5 裁決狀態（切欄、SPIL、寫回、修 bug、去重、樣本檔）。
> - **#22 D-a～D-f 全部裁決（Steven 20260927，經 St02-M）**：見第 6 節。
> - **FTP 傳輸＝Windows WinINet（Steven 20260927）**：ElaFtp 不再用 Jimmy 的 MiniFtpEngine `TNMFTP`；做一個小的傳輸介面（`IElaFtp`），實作用 WinINet（`InternetOpen`／`InternetConnect(INTERNET_SERVICE_FTP)`、`FtpSetCurrentDirectory`／`FtpCreateDirectory`／`FtpPutFile`／`FtpGetFileSize`／`FtpFindFirstFile`、passive 旗標），ctest 用腳本化的假物件，**從不真的連線**。MinGW 有 `C:\MinGW\include\wininet.h`＋`C:\MinGW\lib\libwininet.a`，MSVC 也有。提案（含 R0 拆庫留或撤）先送 St02-M；R1～R6 排在 P4 之後。

## 1. 範圍

### 1.1 先做的五個

| 項目 | 功能開關（IniConfig／客戶碼） | 觸發 | golden 本體 | 輸出／FTP 目標 |
|---|---|---|---|---|
| **[O06]** Auto save event log | `[Event Log]` `EnableAutoSaveProductiont`（O06-4，`bEnableProductionAutoSave`）；`EnanleTimePeriodSaveLog`＋`TimePeriodSaveLog`（0＝10 分、1＝30 分）；golden 的分析器要 O10 開（O10＝O06-1，906 cprod.cpp:2394） | 分析器 Timer2（Rev891 Analyzer.cpp:3010-3088，50 秒、分鐘鎖）：每 1 分鐘，或 10／30 分鐘 | `O06SaveSummaryData`（:3142-3152）：今天 00:00～23:59:59 → 查詢 → `SaveSummary(AutoSaveProductionPath)` | `<AutoSaveProductionPath>\<MachineID>-SummaryData_<yyyy-mm-dd>.txt`（覆寫，cp950，CRLF）；不上傳 |
| **[O19]** Auto Record Report（**VTEST 915／919 那一支才是 ELA**） | `bO19_AutoRecordReportByEveryWeek`＋`iO19_WeekPeriod`；`asO19_SavePath`；`bVTESTFunction`（CC 915／919） | Handler `TFormHS::TimerAutoBackupTimer`（HS_Function.cpp:257-272）：每週一次，約 00:00:04 → `EL_VTEST_MTBF_SUM` | `DoVTestSaveSummary`（Analyzer.cpp:3240-3261）：前 7 天 → 查詢 → `SaveSummary(asO19_SavePath, "", bDetail=false)` | `<asO19_SavePath>\<MachineID>-SummaryData_<today>.txt`；不上傳 |
| **[N25-3]** Jam log FTP（**只有 CC_ChipMos_ZHUBEI 851**） | `bN25_3_EnableULJamLog`＋`sN25_3_JamLogFTPPath`；帳密用 N25-2（`sN25_2_FTPUserName／Password／Host`，沒有 port／passive） | Handler `TfMain::Timer2Timer` 00:00:01～04（main.cpp:21442-21447，最多送 4 次，分析器合併）→ `EL_UPLOAD_JAMWEEK`；手動鈕 | `UploadJamCode`（uChipMosZHUBEI_Func.cpp:328-350）→ `SaveJamCodeFor7Days`（:25-56）→ `N25_UploadJamDataToFTP`（:184-255） | 本機 `D:\HT9045_Log\JamWeek\<y>\<m>\<d>\Jam Rate.txt`（LF，**附加**）＋`HadUpload.txt`；FTP `<path>/<hostname>/Jam Rate.txt`＋`<path>/<hostname>/BackUp/Jam Rate_yyyymmdd_hhnn.txt` |
| **[N10]** Log File Upload to Server（**ELA 只有 N10-3 與 BYFILE 兩塊**） | `bN10_DailyUploadProdData`（N10-3）＋`iN10UploadProductMethod`（0＝00:00、1＝08:00＋20:00、2＝每小時、3＝指定時間；**★W44＝B，Steven 0928**：排程每天在 Handler 設的時間跑一次、不補跑，golden 從不存那個時間＝00:00）；`iN10UploadMethod`（0 FTP、1 網路磁碟）；`sN10UploadDrivePath`；FTP `cN10Ftp*`、`iN10FtpPort`、`bN10FtpPassive` | 分析器 Timer2 N10-3（Analyzer.cpp:3042-3068；分析器的 `rgN10_3_1` 只有 3 項，ReadConfig 設 3 會被 TRadioGroup 夾成 2＝**每小時**，R2 查到）；`EL_UPLOAD_BYFILE_N10`（**沒有送出端**，HS:432-436 註解掉） | `N10SaveSummaryData`（:3154-3180）：**只存檔不上傳**（網路磁碟或 `D:\HT9045_Log\EventLogSummary\yyyymm\`）；`UploadByFile_N10`（uChipMosZHUBEI_Func.cpp:392-418）讀 `UploadFile.csv` 每列上傳 | summary txt；BYFILE 走 N10 的 FTP（分析器不理 port／passive） |
| **[N34]** OEE And Failure Report（**只有 CC_CYUEAN 868**） | `bN34_GenerateOEEAlarmRpt`＋`sN34_OEEAlarmRptPath` | Handler `TfLotInfo::SetLotEnd`（uLotInfo.cpp:2229-2236）→ `EL_UPLOAD_CHIPADV_LOTEND`（事件，不是計時）；**St01 分支 `forms/fLotInfo.cpp:7082-7095` 的送出端還閘著** | uChipAdvancedFunc `AnalysisLog`（:22-72）：找最後一次 Lot Start → 查詢 → 指標／「OEE」＝(input-fail)/input／警報比例 → `SaveReport`（:232-241） | `<sN34_OEEAlarmRptPath>\ClipAdv_<LotID>_<y><m><d>.csv`（**附加**，LF）；不上傳 |

- **不是 ELA、屬於 Handler 的**（不在 ElaReports 裡）：非 VTEST 的 O19（cMyDB `RecordTimeData(3)` → `fObserver->DoProduction_Summary_Report` → `<asO19_SavePath>\<yyyy>\<HostName>_<mmdd>.csv`，筆電的範圍）、N10 的 Handler 部分（`UpDataToServer_KYEC(0..7)`、N10-1 定時上傳、N10-9 tray log、JamRate by day）。
- 三者共用一個寫檔函式：`SaveSummary`（Analyzer.cpp:2614-2688）＝O06、N10-3、O19-VTEST 的輸出。

### 1.2 之後才做的（W22：功能開關有開就做）

**分析器端**：N25-4 `Jam_Summary.csv` FTP（851，00:00，`bN25_4_EnableUpload`）、N25-5 前一天 EventLog.txt FTP（851，00:00，`bN25_5_EnableUpload`）、手動 Save Summary（預設 D:\RMS）、四個 XLS 匯出（P7）。
**N25-4／N25-5 已做**（W22，20260928，St02-E helper，`v906/steven-ela-wip` `7b69cdc1`；兩組態只編譯、未上機，ctest `ELA_Ftp` 第 15～17 節待 St01；帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 檔尾「W22」）：R4 傳輸＋驗證、R5 排程工作 `N25-4`／`N25-5`（O10／851／開關閘，FTP 錯開 900 s，00:00，跟 N25-3 同一時段依序）、Hub `EL_UPLOAD_SUMMARY`／`EL_UPLOAD_EVENTLOG`。手動 Save Summary（P7a）與 XLS（P7）見帳本各自的一節。
**Handler 端**（不是 ELA，Handler／筆電負責）：O06-1/2/3 XLS 自動存（golden 就是死的）、N10 KYEC 0..7、N10-1、N10-9、N10-2 by lot、JamRate by day、N25-2 溫度 FTP、N26 JamRawData、N17 UploadProdLog（01:00）、N35 Ground／ESD、N14-1 OEE、CC_ASE_CL SaveOEELog、N33 LEADYO。
（Handler 這張表只看了三個排程點：TimerAutoBackupTimer、TfMain::Timer2Timer、SetLotEnd；不是 N 群約 80 個欄位的完整清單。）

## 2. 排程放進 ElaHub（W21）——計畫，未實作

- **搬進 ElaHub 的（依時間）**：分析器 Timer2 的 O06-4 與 N10-3；Handler 半夜的 N25-3（之後 N25-4／5）；每週的 VTEST O19。這些在 Handler 端的送出點（main.cpp:21442-21458、HS_Function.cpp:257-272）就不必翻。
- **W13（Steven 20260927）**：Handler 的 N17 **UploadProdLog** 每天 01:00 那一次（本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:3608-3654` 是活的；golden 由 `TFormHS::TimerAutoBackupTimer` 的 clock-1 觸發，HS_Function.cpp:204-207）也**進 ElaHub 的排程**，同樣驗證＋錯開；**不翻** TimerAutoBackupTimer。
- **維持 `Post(cmd)` 的（事件）**：N34（lot end）、手動鈕、`EL_UPDATE_PARAMETER`（存設定時，cprod.cpp:3120-3121）。
- ElaHub 要多讀：`bN25_3/4/5_Enable*`（分析器從來沒讀）、`bVTESTFunction`（由 `sCustCode` 915／919 推）、客戶碼閘 851／868、（視 D-a）O10。
- 手動觸發：`POST /api/ela/job?cmd=N`（在 `ElaService::ServeHttp` 裡，不用改 wb_serve）取代 golden 的手動鈕。

## 3. 上傳安全（W21 提案）——計畫，未實作

1. **驗證**（WinINet 版，Steven 20260927）：一次上傳算成功＝(a) `FtpPutFile` 回 TRUE，**而且** (b) 伺服器的大小等於本機檔案大小，**而且** (c) `FtpFindFirstFile` 在目標資料夾列得到檔名。（舊的 MiniFtpEngine 版要 Jimmy 加 `TNMFTP::Size()`，改用 WinINet 就不用了。）
   **R4 實作（20260927）**：(b) 用 `FtpCommand` 送 `TYPE I`＋`SIZE <path>`、讀 `InternetGetLastResponseInfo` 的 `213 n`——**不用 `FtpGetFileSize`**（它要 `FtpOpenFile` 的 handle，會開始 RETR 卡住 session，研究 §3）；伺服器不支援 SIZE（500／502／504）就用 LIST 的大小，列表也沒大小＝「只驗檔名」並寫 FTP_Log。
2. `HadUpload.txt` 只有主檔**和** BackUp 都驗證成功才寫（偏離 golden，要記）。
3. **每次重算報表**（先刪再寫），不再附加（偏離 golden）。
4. **每台錯開**：offset＝FNV-1a-32(Machine ID) mod W。
   - Machine ID：Gerneral.ini `[Version] "Machine ID"`（Handler `IniConfig.SocketHandlerID`；ELA `ElaService.cpp:154-155`、`ElaHub.cpp:307`）；是 "HT-90xx" 或沒有就用主機名。
   - W＝900 秒（每日／每週的 FTP）；300 秒（N10-3 目標是網路磁碟）；O06-4 目標在遠端時 min(週期/2, 120 秒)。**存本機的不錯開**（O06 本機、VTEST D:\MTBF_Summary、N34 本機路徑）。
   - 資料範圍一律用**表定的時間點**算，錯開不影響資料。
5. **退避**：第 n 次等 min(60·2^(n-1), 1800) 秒 ±20%（以 Machine ID＋n 為種子）＝約 1／2／4／8／16／30 分鐘；6 次後放棄，或到下一次表定時間。
6. 傳輸**一次一個**（在 Hub worker 上）；連線逾時 5 秒（golden）、傳輸 30 秒。
7. 每次嘗試記進 **FTP_Log**（golden 路徑 `D:\HT9045_Log\UploadFile\FTP_Log`，走 `W906_HT9045LOG_ROOT` seam）與 `/api/ela` 的上傳狀態／歷史。
8. **開機補跑（D-b＝要，Steven：「是可以的，也是客戶期望的」）**：Handler 沒開而錯過表定的報表／上傳時段 → 下次開機跑一次，照 Machine-ID offset 錯開；成功一樣要驗證（第 1 項），報表一樣重算不附加（第 3 項）。
9. **真的連 FTP 與否跟設定的功能開關走（D-f，照 golden）**：例如 N25-3 `bN25_3_EnableULJamLog`，加上 N25-2 的主機／帳密；**不另設環境開關**。ctest 一律用注入的假傳輸，永遠不連出去。
10. **報表檔用 UTF-8（D-e，偏離 golden 的 cp950）**：帳本要記。ELA_Reports 跟 BCB exe 產生的 oracle 檔比位元組時，要先把 oracle 轉成 UTF-8 再比。

## 4. 元件與階段——計畫，未實作

| 階段 | 內容 | 檔案（St02 除非註明） |
|---|---|---|
| R0 | **（已做 20260927）** 把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\MiniFtpEngine.cpp` 拆成新的靜態庫 `ht9045_nmftp`（vclcompat＋ws2_32）；`ht9045_kyecftp` 與 `ht9045_ela` 都連它 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt`（筆電 TO_STEVEN §4 11:4x 同意；St02 R0 commit） |
| R1 | **（已做 20260927，St02-E，`99dc54ba`）** ElaCore 補 `LogRecord`、`GetEventLogTextToVec` ×2（**跟 GetEventLogText 用同一套 `ela::Options`**：W15 切法、W19 選檔＋跨檔去重；golden 模式照開關，AI(W906-ELA-W19) 20260927）、`GetStartEndGap`；新檔 `ElaFileUtil`（WriteDataToFile、MyForceDirectories、PathCombin、EnsureDirectoriesExist、GetNameAndExtension、IsValidFileName、FixFolderPath；**FTP_Log 歸 R4 ElaFtp**，經 `SetDbiProcessHook` 接 `MyDBIProcess`） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.*`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFileUtil.*` |
| R2 | **（已做 20260927，St02-E，`1b6bf39e`）** 新檔 `ElaReports`：`SaveSummary`、`O06SaveSummaryData`、`N10SaveSummaryData`、`DoVTestSaveSummary`；解除 `EL_VTEST_MTBF_SUM` 的 HOLD（O10 閘，D-a）；報表 UTF-8（D-e）；給 R5 的呼叫點 `Timer2Due`／`Timer2Run`（G3 修成整點一次）、`AutoJobsEnabled`（D-a）；頁面查詢與報表共用 `BtnQuery`（ElaHub） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.*` |
| R3 | **（已做 20260927，St02-E，`929ced5a`）** ElaReports 加 N34 ChipAdvanced（AnalysisLog…SaveReport），加空向量下溢的保護（D-c）；報表重寫不附加；ParseField 不再吃掉 Lot ID 第一個字（W18 B）；解除 `EL_UPLOAD_CHIPADV_LOTEND` 的 HOLD（O10 閘）；端到端要 St01 解開 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp` 第 7082～7095 行 | 同上 |
| R4 | **（已做 20260927，St02-E，`v906/steven-elaftp-wip`；兩組態只編譯、未上機、ctest `ELA_Ftp` 待 St01）** 新檔 `ElaFtp`：`IElaFtp`＋null／只寫 log／**WinINet**（`ElaFtpWinInet.cpp`）三種傳輸、第 3 節的驗證、FTP_Log、退避純函式；`ElaChipMos`：N25-3（`N25_UploadJamDataToFTP`、`SaveJamCodeFor7Days`、`UploadJamCode`）與 N10 BYFILE，接在 Hub 的 `EL_UPLOAD_JAMWEEK`／`EL_UPLOAD_BYFILE_N10`（D-f 設定閘）。`ht9045_ela` 改連 wininet，不連 `ht9045_nmftp`（庫保留給 KYECFTP）。偏離與待 Steven（★W36 已裁決＝C，Steven 20260928：模擬組態也用 WinINet；Passive＝★W46 待定）見帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 檔尾「R4」 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.*`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtpWinInet.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.*` |
| R5 | **（程式已做 20260927，St02-E helper，`v906/steven-elasched-wip` `155e27dd`；兩組態只編譯、ctest `ELA_Schedule` 待 St01；接線三行與 R2／R4 的本體在合併時；帳本「R5 ElaSchedule」。錯開 W＝FTP 900 s／網路磁碟 300 s（UNC、網路磁碟機，或 O06-4 勾 [O06-6] Use Net Drive；O06-4 不超過半個週期）／本機 0；退避 ±20% 用 fmix32(FNV-1a(Machine ID＋"#"＋n))，重試 6 次＝共 7 次；每個工作自己決定（golden 一個 bNeedUpload 拆開）；N10-3 方法 3 照 golden 夾成每小時＋加掛點；開機補跑紀錄 `<log root>\UploadFile\ElaScheduleState.ini`，沒有紀錄的第一次開機不補跑；手動不看 O10）** 新檔 `ElaSchedule`：可注入的時鐘、Timer2 O06／N10、半夜 N25-3、每週 VTEST、01:00 UploadProdLog（W13）、錯開與退避、O10 閘（D-a）、手動觸發 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.*`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` |
| W22 | **（已做 20260928，St02-E helper，`v906/steven-ela-wip` `7b69cdc1`；兩組態只編譯、ctest `ELA_Ftp` 第 15～17 節／`ELA_Schedule` 待 St01）** N25-4 `Jam_Summary.csv`（30 天、CLEAN_OUT 分段、Production_Log 顆數）、N25-5 昨天的 `EventLog.txt`（位元組複本）：R4 傳輸＋驗證、R5 排程 `JOB_N25_4`／`JOB_N25_5`、Hub `EL_UPLOAD_SUMMARY`／`EL_UPLOAD_EVENTLOG`（不再 HOLD）；偏離見帳本「W22」 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.*`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.*` |
| R6 | **（頁面與路由已做 20260928，St02-E helper，`v906/steven-elasched-wip` `353c0c6e`；兩組態只編譯、ctest `ELA_Schedule` 第 18 節待 St01；OEE 圖只有設計，見帳本「R6」）** `eventlog.html` 新分頁 Auto Jobs：每個工作的上次執行／結果／原因／下次／嘗試次數、手動執行（先確認）、EL_* 工作紀錄；`GET /api/ela/schedule`、`POST /api/ela/job?id=`；OEE 網頁圖（golden 沒有真的 OEE 計算，是新功能） | `D:\HT9045\web\page\eventlog.html` |

**ctest**（只寫 %TEMP%，不碰真的 FTP）：
- `ELA_Reports`：cp950 的 EventLogTxt／Production_Log／JAM0000.dat 測試檔（W23：從 `D:\HT9045_Log\EventLogTxt\` **複製**，不寫回那裡）、假時鐘；SaveSummary／VTEST／ClipAdv 的位元組跟 oracle 檔比（oracle 要筆電或 St01 用 BCB exe 產生，St02 不能執行 exe）。
  **R1／R2 已寫**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_reports.cpp` 第 1～7 節）；oracle 步驟 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\fixtures\ela\oracle\README.txt`，沒有 oracle 檔＝SKIP。ClipAdv 在 R3 加。
- `ELA_Ftp`（**R4 已寫**，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_ftp.cpp`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\ElaFtpFake.h`）：腳本化的假 `IElaFtp`（改用 WinINet 後不再用 TNMFTP SIM）；驗證 `<path>/<host>/BackUp` 的整串 CWD／MKD／STOR／SIZE／LIST、BackUp 檔名、HadUpload 規則、驗證失敗（STOR 550、SIZE 不符、SIZE 502、LIST 沒有、登入 530）、D-f 閘、sim 組態＝只寫 log、N10 BYFILE、FTP_Log；退避用假時鐘算（不 sleep）；開頭與結尾確認 wininet.dll 沒載入。另有只編譯的 `ela_ftp_wininet_link`。
- `ELA_Schedule`：假時鐘；依 Machine ID 錯開的觸發時間、星期、一天一次、退避序列、O10 與客戶碼閘。

## 5. 要知道的 golden 事實

- golden 的上傳**從不驗證**：「沒丟例外」就算成功。V906 的 `TNMFTP` 從不丟例外，所以照 golden 做會永遠「成功」。
- **沒有錯開**：每台機器在同一秒觸發（N25 在 00:00:01～04，而且緊接在 N25-2 00:00:00 對同一台主機的上傳之後；Timer2 在整分鐘）。
- golden 的分析器（連同它的所有工作）**只在 O10 開時存在**，而 O10＝O06-1（906 cprod.cpp:2394）。V906 的 ElaHub 一直在跑。
- V906 的 KYECFTP `TNMFTP` **預設 SIM**，全樹沒有人呼叫 `SetSimMode(false)`，所以現在沒有任何 FTP 真的出網。
- N25-3 的怪處：`HadUpload.txt` 在主檔上傳後就寫了（BackUp 失敗也擋住重試）；BackUp 的 log 印錯資料夾；重試會把 `Jam Rate.txt` 再附加一份。
- N34 的崩潰：`for(size_t i=iTsize-1; i>0; i--)` 在沒有紀錄時下溢（uChipAdvancedFunc.cpp:216）；「OEE」用的 `MySummary.iFailCount` 跨查詢累加（G1／#18；V906 預設 W18＝B 已每次歸零）。
- VTEST O19 是每週**一次**（`CheckClockTrigger` 是邊緣觸發），不是帳本寫的「可能重複觸發」。
- N10-3 方法 3「指定時間」：Handler（906_0625_Steven `HS_Function.cpp:488-509`）只在那一分鐘跑一次、不補跑；時間欄位 `IniConfig.dN10_3_1_SpecifiedTime` golden 從不讀也不存（＝00:00）。
  **★W44＝B，Steven 0928**：排程照這個做（R5 表的「夾成每小時＋加掛點」已由它取代；帳本 R5「★W44」）。
- N34 的 ParseField 在「Lot ID:」後面固定跳一個字元，批號少第一個字（uChipAdvancedFunc.cpp:243-257）。**★W47＝A，Steven 0928**：修正（R3 已照 A 做，沒改程式）。
- VTEST 915／919 的讀檔：golden 只讀 `<yyyymmdd>`＋`_EventLogTxt_` 的檔（Analyzer.cpp:807-811）。**★W38＝B，Steven 0928**：照 golden 固定檔名（`Options::goldenFileRuleVtest`）。

## 6. 裁決（Steven 20260927，經 St02-M；#22 全部定了）

| # | 問題 | 裁決 |
|---|---|---|
| D-a | 自動工作要不要照 golden 用 O10（＝O06-1）閘住，還是一直跑？ | **照 golden 用 O10 閘住**：O10 有勾才跑自動報表／上傳；網頁的分析不受影響 |
| D-b | 驗證成功才算、每次重算、開機補跑，這三個偏離要不要？ | **三個都要**：驗證、重算照 W21；開機補跑錯過的時段，照 Machine-ID offset 錯開（第 3 節第 8 項） |
| D-c | N34 沒有紀錄時 golden 會崩潰，要不要加保護？ | **加保護**（記成偏離） |
| D-d | `GetEventLogTextToVec` 照 golden 用上次查詢留下的清單，還是重掃？ | **產報表時重掃資料夾** |
| D-e | 報表位元組照 golden 用 cp950？ | **UTF-8**（偏離 golden，計畫與帳本都要記）。St02 提醒：上傳到客戶 FTP 的報表是誰在讀、吃不吃 UTF-8，還沒查到（St02-M 要跟 Steven 確認） |
| D-f | 真的出網的開關怎麼做？ | **跟設定的功能開關走**（N25-3 `bN25_3_EnableULJamLog`、N25-2 主機／帳密），照 golden，不另設 env；ctest 一律用假傳輸 |
| R0 | `ht9045_nmftp` 拆庫（Jimmy 的 CMake） | 已推（`58dea595`）。改用 WinINet 之後 ELA 不連它；留或撤由 St02 在 WinINet 提案裡建議 |
| 傳輸 | ElaFtp 用什麼 FTP？ | **Windows WinINet**（Steven 20260927），放在 `IElaFtp` 介面後面，ctest 用腳本化的假物件 |
