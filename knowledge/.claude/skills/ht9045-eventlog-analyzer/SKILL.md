---
name: ht9045-eventlog-analyzer
description: >
  HT9045 Event Log Analyzer（舊的 BCB6 分析器 EventlogAnalyzer.exe，視窗 TfrmELA「Event Log Analyzer」）與它在 V906
  轉成 web 頁的紀錄。涵蓋：分析器架構（Analyzer.cpp 的 TAnalysisProcessThread、多日 GetEventLogText 統計、MySummary、
  sgByDay／sgByHour、Top5、Jam 等級 GetJamLevel／GetJemIncludeMTBA／GetJemIncludeMTBF、OEE TeeChart、SaveSummary／O06／N10／
  DoVTestSaveSummary、uChipAdvancedFunc、uChipMosZHUBEI_Func、uAnalysisProductionLog、TfFTP）、Handler 端七個 EL_* 指令
  （EL_UPDATE_PARAMETER／EL_UPLOAD_JAMWEEK／EL_UPLOAD_CHIPADV_LOTEND／EL_UPLOAD_SUMMARY／EL_UPLOAD_EVENTLOG／
  EL_UPLOAD_BYFILE_N10／EL_VTEST_MTBF_SUM）經 SendCommand_EventLog → WM_COPYDATA M_V 送過去、cObserver 的 tsEventLogTxt
  單檔檢視（TfObserver::GetEventLogText／cbbMonthChange／lstEventLogClick／ParseEventLogLine）、EventLogTxt CSV 格式，
  以及 V906 的轉換計畫（ElaCore、ElaHub、/api/ela/*、eventlog.html；W13 UploadProdLog 進排程、W15 只切逗號＋引號、W16 SPIL 照 golden、
  W17 寫回照 golden、W18 修明顯 bug、W19 去重＋其他存檔週期、W20 Jam Code 編輯器合一、W21 排程進 ElaHub＋上傳驗證錯開、
  W22／#22 報表 O06／O19／N25-3／N10／N34＝ElaReports、W23 樣本檔）。
  Use when：Event Log Analyzer、EventlogAnalyzer、事件分析、MTBF、MTBA、OEE 報表、Jam 彙總、Top5 alarm、tsEventLogTxt、
  GetEventLogText、SendCommand_EventLog、EL_ 指令、FTP 上傳 event log、偉測 MTBF 文件、ChipAdvanced、南茂竹北 summary、
  UploadProdLog、分析器轉 web。
  關鍵字：EventlogAnalyzer, TfrmELA, Event Log Analyzer, TAnalysisProcessThread, EventLog_COMMAND, EL_UPDATE_PARAMETER,
  EL_UPLOAD_JAMWEEK, EL_UPLOAD_CHIPADV_LOTEND, EL_UPLOAD_SUMMARY, EL_UPLOAD_EVENTLOG, EL_UPLOAD_BYFILE_N10, EL_VTEST_MTBF_SUM,
  SendCommand_EventLog, M_V, WM_EventAnalysis, HEventLogWnd, tsEventLogTxt, strngrdEventLog, lstEventLog, cbbFilter,
  GetEventLogText, ParseEventLogLine, cbbMonthChange, EventLogTxt, MySummary, MTBF, MTBA, OEE, GetJamLevel, SaveSummary,
  DoVTestSaveSummary, uChipAdvancedFunc, uChipMosZHUBEI_Func, TfFTP, KYECFTP, ht9045_nmftp, ElaCore, ElaHub, eventlog.html,
  ElaReports, ElaFtp, IElaFtp, WinINet, ElaFtpWinInet, ElaChipMos, FTP_Log, HadUpload, Jam Rate.txt, W36, ElaSchedule, UploadProdLog, N17, W13, W15, W16, W17, W18, W19, W20, W21, W22, W23, #22, O06, O19, N25-3,
  N10, N34, JamFileLock, JAM0000_dat 鎖, SplitEventLogCsv, EventLogCsv.h, EventLogFileSpan, dupRowsSkipped, keepGoldenBugs,
  goldenFileRule, goldenFileRuleVtest, bcbCommaText, AllEventLog 去重, 存檔週期, W38, W43, W44, W44-2, W47, 指定時間, dN10_3_1_SpecifiedTime,
  O06-8, EnanleTimePeriodSaveLog, TimePeriodSaveLog, 補發, boot catch-up.
  轉換計畫全文 → D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md；
  報表與上傳計畫（#22／W13／W21／W22）→ D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md
---

# HT9045 Event Log Analyzer 知識庫與 web 轉換紀錄

> **路徑一律寫絕對路徑**（Steven 20260927）。常用的根：
> - V906 C++ 樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（以下寫「V906」）
> - V906 網頁：`D:\HT9045\web\`
> - golden 906（St02 用）：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`（寫「906_0625_Steven」）
> - golden 912：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（寫「912」）
> - 分析器原始碼（SVN Rev891）：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`（寫「Rev891」）

## 1. 這支程式是什麼

- 舊的 BCB6 分析器 `D:\EventlogAnalyzer\EventlogAnalyzer.exe`，主表單 `TfrmELA`，視窗標題「Event Log Analyzer」。使用者 20260926：「這是舊的 event log 分析器」。
- 原始碼：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`（SVN 工作副本 20260330，＝SVN r891＝FileVersion 20.25.891.0）。舊的 VS Code agent `D:\EventlogAnalyzer\.github\agents\EventlogAnalyzer.agent.md`（原檔保留）20260926 已併進本 skill，只搬了「版本資料夾命名規則」與「約束條件」，見 §1.1、§1.2。
- 參考 exe：`D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe`（20.25.891.0）。
- 何時被用：golden Handler 在 `CosFunction.bUseMDB==false` 且 **O10 開**時啟動它（906_0625_Steven `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:17988` `execinfo.lpFile="d:\\EventlogAnalyzer\\EventlogAnalyzer.exe"`，看門狗 `:17854`；912 同段在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:18611-18614`、`:18475`）。O10＝O06-1（906_0625_Steven `cprod.cpp:2394`）。`bUseMDB==true` 時用的是另一支 `EventLogSaver.exe`。現行機台一律 false（見 `D:\HT9045\.claude\skills\ht9045-mydb\SKILL.md`）。
- 限制（BCB 線）：Big5、BCB6 無 C++11、不改 `D:\HT9045\elec\myvcl`／`D:\HT9045\elec\Component`。

### 1.1 版本資料夾命名規則（BCB 線，自舊 agent 併入）

- 格式：`EventlogAnalyzer_Rev<Rev>.<Minor>_<YYYYMMDD>[_<OwnerOrTag>]`。
- 找最新版本時以磁碟上實際存在的目錄為準。

### 1.2 約束條件（BCB 線，自舊 agent 併入）

- **必須**保持 Big5（CP950）編碼，禁止轉換 UTF-8。
- **必須**遵循 BCB6 C++ 限制（不可使用 C++11 以上語法）。
- **絕不**修改 `D:\HT9045\elec\myvcl` 與 `D:\HT9045\elec\Component` 下的共用元件。
- **臨時腳本**統一建立於 `D:\AI_TempFile\`。
- 以上是 BCB 分析器本身的規則。V906 的 web 轉換（ElaCore／eventlog.html）照 V906 的規則：UTF-8、C++17。

## 2. 與 Handler 的介面

- `SendCommand_EventLog(EventLog_COMMAND CMD, AnsiString Data)`（V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Interface\InterfaceSYS.cpp:554`）：`FindWindow("TfrmELA","Event Log Analyzer")`，組 `M_V`（`bModeType=TYPE_HANDLER_EventLog`、`bCommandType=CommandType_EventLog`、`bCommand=CMD`、`bData[0]=atoi(Data)`），`SendMessage(WM_COPYDATA, WM_EventAnalysis)`。
- 七個指令（值、Handler 呼叫點、分析器動作）見 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md` §1.1。分析器端 `OnMyCopyMsg` 只設旗標，由 `TAnalysisProcessThread::AnalysisThreadProcess` 依序處理。

## 3. 兩個同名的 `GetEventLogText`（不要混淆）

| | Handler `TfObserver::GetEventLogText()` | 分析器 `TfrmELA::GetEventLogText()`（Rev891 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:782`） |
|---|---|---|
| 位置 | V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1395`；906_0625_Steven `cObserver.cpp:3801`；912 `cObserver.cpp:3857` | 同左欄標題 |
| 做什麼 | 單檔檢視：讀 `lstEventLog` 選中的一個 CSV 填 `strngrdEventLog` | 多日統計：日期區間逐日讀 CSV，累計 Stop／Alarm／Fail、JAM／WAR／MES、各碼次數與停機時間 |
| 過濾 | `cbbFilter`：全部／JAM only／WAR only／MES only／UnitName 字串 | Jam 等級、是否計入 MTBA／MTBF |
| 上限 | 超過 10,000 行不顯示 | 無 |
| 觸發 | `cbbMonthChange`、`lstEventLogClick`、`btnQueryEventLogTxtClick` | `EL_UPDATE_PARAMETER`、`btnImport*` |

## 4. EventLogTxt CSV

- 路徑 `D:\HT9045_Log\EventLogTxt\yyyy\mm\EventLogTxt_yyyymmdd.csv`（`CC_SINOICTECH` 會隱藏路徑，檔名從第 35 字元截）。
- 表頭 `Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe`；SPIL 格式（`IniConfig.bSPILFunction`）欄位順序不同：第一欄是 UnitName，**檔案裡沒有序號欄**（序號是 Handler 檢視頁自己加的）。分析器對 SPIL 檔每行日期檢查都失敗，等於不分析（W16＝A：照 golden）。
- 產生者是 cMyDB 一族（`MyDBIProcess` 等），見 `D:\HT9045\.claude\skills\ht9045-mydb\SKILL.md`。
- **W23 樣本檔**：真的樣本在 `D:\HT9045_Log\EventLogTxt\`。**只讀**；測試要用就**複製**成 fixture（放 %TEMP% 或 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\`），**絕不寫進那個資料夾**。

## 5. V906 現況（20260927）

**已在 main**（函式庫 `ht9045_ela`，連 vclcompat＋ht9045_nmftp；帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`）：
- **ElaCore**（P1a）`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.cpp`：多日 GetEventLogText、SetRange、Jam ini 三個查詢（含寫回）、BCB6 CommaText、日期解析；W15／W18／W19 的開關與選檔（下面「W15／W18／W19 的程式」）。切欄在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\EventLogCsv.h`。ctest `ELA_Core`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_core.cpp`）。
- **ElaTables**（P1b）`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaTables.cpp`：Top5／Top5Filter／FailAndAlarm／ByFilter、sgByDay／sgByHour、SetTotalSummary、ListProductionLog、LoadFavorite。
- **ElaHub**（P2／P3）`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp`：`Hub::Post(cmd)`（EL_* 合併、golden 優先順序）、worker＋500 ms、`PostQuery`、`SnapshotJson`（FTP 密碼不放）；`EL_UPDATE_PARAMETER`＝ReadConfig。ctest `ELA_Hub`。
- **ElaService**（P3／P4）`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp`：`/api/ela`（`ela::ServeHttp`，`?since` 輪詢減量），開機排今天的查詢＋EL_UPDATE_PARAMETER。ctest `ELA_Service`。
- **網頁** `D:\HT9045\web\page\eventlog.html`＋`D:\HT9045\web\background.html` 視窗 id `eventlog`（hidden＋lazy，只有工作列按鈕）。
- **接線**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` :439／:2867／:4165（`W906_ElaStart`＋hook）／:5967（先清 hook 再 Stop）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt` :3421；InterfaceSYS `SendCommand_EventLog` 先呼叫 `W906_ElaPostHook`（golden 的 FindWindow／WM_COPYDATA 照送，給手動開的外部 exe）。
- **#31＝A**：V906 不啟動舊 exe（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerBridgeCtl.cpp:259` 永久 `#if 0`）。**#33**：SIM 開機實跑筆電跑過（備份→驗證→還原）。
- **S128 鎖**：JAM0000.dat 的查＋寫在具名鎖 `Local\HT9045_JAM0000_dat` 裡（ElaCore `JamFileLock`，`5884cf6a`），5 秒拿不到就只讀、這次不寫回。security.jam 頁那一端（`JamIniMerge.h`）已經在 main（`4fbaf7e9` 經 St01 `dd3aee54` 進 main `53a55b35`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamIniMerge.h`）。W20（Jam Code 編輯器合一，現在叫 ★W45）的設計見 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md`；Steven 0928 已裁決（下面 #20）。
- **R0 已做**（20260927）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\MiniFtpEngine.cpp` 拆成自己的靜態庫 `ht9045_nmftp`；`ht9045_kyecftp` 與 `ht9045_ela` 都連它（TNMFTP 只在一個 archive 定義）。

**R1／R2 已做**（St02-E 20260927，`v906/steven-ela-wip` `99dc54ba`／`1b6bf39e`，兩組態只編譯、未上機；ctest `ELA_Reports` 待 St01；帳本「R1／R2」）：
- R1：`ElaCore` 加 `LogRecord`、`GetEventLogTextToVec` ×2（跟 GetEventLogText 同一套 `ela::Options`：W15 切法、W19 選檔＋跨檔去重 `vecDupRowsSkipped`；golden 模式沒有 VTEST 分支，照翻）、`GetStartEndGap`。
  新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFileUtil.h`／`.cpp`：WriteDataToFile、MyForceDirectories、MyDBIProcess（FTP_Log 經 `SetDbiProcessHook` 交給 ElaFtp，R4）、PathCombin、EnsureDirectoriesExist、GetNameAndExtension、IsValidFileName、FixFolderPath、`Ht9045LogRoot()`（`W906_HT9045LOG_ROOT`）。
- R2：新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.h`／`.cpp`：SaveSummary、O06／N10／VTEST、`AutoJobsEnabled`（D-a O10 閘）、`Timer2Due`／`Timer2Run`（**R5 的呼叫點**；G3 原本修成整點一次，**★W43＝C（Steven 0928）**：沒勾 [O06-8]＝不存）、`Clock`（假時鐘）。
  報表 **UTF-8**（D-e）、CRLF、覆寫；頁面查詢與報表共用 `ela::BtnQuery`（ElaHub.h）。
  `EL_VTEST_MTBF_SUM` **不再 HOLD**：O10 開才跑，寫 `<asO19_SavePath>\<Machine ID>-SummaryData_<今天>.txt`（前 7 天、20 行）。
  **工作結果不換頁面快照**（St02-E 20260927，`673cf249`）：每個 EL_* 工作留一筆 `ela::JobRecord`（`Hub::Jobs()`、快照 `"jobs"`，R6 顯示）；頁面只在使用者自己查詢時變。
  報表編碼只在 `ela::ReportEncode`（D-e 改規則只改這裡）。
- R3（`929ced5a`）：N34 ChipAdvanced（`ela::ChipAdvancedFunc`，ElaReports.* 檔尾）；`EL_UPLOAD_CHIPADV_LOTEND` 解除 HOLD（O10 閘）。D-c 沒有列不出報表；
  報表重寫不附加；ParseField 不再吃掉 Lot ID 第一個字（golden 會，W18 B；**★W47＝A，Steven 0928**）。Handler 的送出端 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp` 第 7082～7095 行（St01 的）仍 `#if 0`。
- golden 事實（R2 查到）：分析器 `rgN10_3_1` 只有 3 項，iN10UploadProductMethod＝3 被夾成「每小時」（排程已改成 ★W44 的指定時間，見 R5）；O06／N10 功能關著也會建資料夾；N10 的 "FTP" 分支只存 `D:\HT9045_Log\EventLogSummary\yyyymm\`。
- oracle：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\fixtures\ela\oracle\README.txt`（BCB exe 的步驟；沒有 oracle＝SKIP）。

**寫檔（照 golden，W17＝A）**：wb_serve 開機 ReadConfig 會把缺的 key 寫進真的 `D:\HT9045\config\config.ini`（20260928 起 `W906_AUTH_PATH` 設了＝`<W906_AUTH_PATH>config.ini`，同 Handler 的 AuthPath；`ElaHub.h` `ElaConfigIniPath`）／`D:\HT9045\system\Gerneral.ini`，查詢會把缺的 key 寫進 `D:\HT9045\Error\English\JAM0000.dat`。`HT9045_ELA=0` 關掉。`POST /api/ela/query` 在 wb_serve 不回 403（allowCmd 恆為 true）；帶 from／to 就真的跑查詢。ELA 的 POST 路由都沒有登入等級檢查（只綁 127.0.0.1 所以可接受；改綁區網前要先加，帳本 P7a 段尾）。

**分析器讀 CSV 的規則**（帳本 §5）：golden 的 BCB6 `CommaText` 也用未加引號的空白切欄（`,16 System,` 會切壞）；Duplicate 含 "1" 整行跳過；StopedTime 單位秒。V906 預設（W15＝B）改用 `ela::SplitEventLogCsv`。
**golden 的坑**：`iFailCount` 從不歸零、By-Filter 清單一直長、小時／12 小時／月檔被跳過、AllEventLog 當天算兩次——V906 預設（W18／W19＝B）都修了，golden 模式（開關）還在。

**裁決狀態**（`D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`）：
- **#13（W13）**：UploadProdLog（N17，本體 V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:3608-3654` 是活的）的 01:00 觸發**進 ElaHub 的排程**，同樣要驗證＋錯開；**不翻** `TFormHS::TimerAutoBackupTimer`。
- **#15（W15＝B）**：**只用逗號切欄，引號可把一欄括起來**（不再照 BCB6 用空白切）；各頁可以共用一個 parser。✅ 程式已做（`v906/steven-ela-wip` `1042d4cc`／`2ceca61c`／`82196ca3`，見下）；cObserver.cpp（共用檔）也改用它：`a9b93d61`（行數不變）。
- **#16（W16＝A）**：SPIL 格式照 golden（不分析）。
- **#17（W17＝A）**：寫回 Handler 檔照 golden（上面「寫檔」）。
- **#18（W18＝B）**：golden 的明顯 bug **修掉**，並記成偏離（iFailCount 不歸零、By-Filter 清單一直長、功能過濾框對調、O06 每分鐘存一次…）。✅ ElaCore／ElaTables／頁面已做（O06 那項在 ElaReports，R2 時修；之後 **★W43＝C**：沒勾 [O06-8]＝不存，見 R5）。
- **#19（W19＝B）**：**去重**（AllEventLog 當天不再算兩次），並**接受其他存檔週期的檔名**（小時／12 小時／月檔不再跳過）。✅ 程式已做（見下）。
- **#20（W20，現在叫 ★W45）**：分析器與 Handler 的 Jam Code 編輯器**合成一個**。**W45：18a／18b／18c＝Steven 0928（第 18 項）；其他照 golden，未明確裁決。**
  18a W20-1＝A：編輯器＝Status.Security 的 Jam Code 分頁，eventlog.html 只有一顆「Jam Code Setting」鈕打開它；
  18b W20-3＝B：933／967 缺 `IncludeMTBF`＝true（Handler 的），**只有 ELA 偏離**（`ela::Options::goldenJamMtbfDefault`＝golden），Handler getter 不動；
  18c W20-5＝A：每個鍵一個框——Handler 的 cbIncludeMTBA 不動，第二框＝分析器的 cbIncludeMTBF 綁「另一個鍵」（一般客戶 IncludeMTBF「Include MTBF」；933／967 IncludeMTBA「Include MTBA (Analyzer)」），
  `JamRules.h` `jamrules::SecondBox`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJamW45.h`；`WebSecurityJam.cpp`／`ht9045_wire_statussecurity.js`／`Status.Security.html` 的呼叫點是認領（帳本「★W45 … 裁決後」）。
  W20-7（訊息說明可不可改）：golden 可改、V906 既有唯讀，W45 沒動，待定。St02-E helper 20260928 `v906/steven-w45-jam` `6f1f8024`，只編譯；ctest `Jam_Rules` 待 St01。
  **第一步已做**（St02-E 20260927，`143445be`，只編譯）：共用規則 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h`（header-only：路徑接縫 `W906_JAM0000_PATH`、933／967 別名、鍵名、缺鍵預設、
  ★W45 的**一行開關** `kIncludeMtbfRule`＝A 各照 golden／B 照 Handler／C 照分析器，當時預設 A，Steven 0928 起＝B）；ElaCore 的三個 JAM0000 getter 改走它（A 時行為不變，ctest `Jam_Rules` 證明）；
  `W906_ElaStart` 用 `jamrules::Jam0000Path()`。第一步時 Handler 端（cSecurity.cpp、WebSecurityJam.cpp、Status.Security 頁）**沒動**；裁決後的呼叫點（認領）見上一段與帳本。
- **#21（W21，推翻原本暫定的 A）**：依時間的排程**放進 ElaHub**；上傳要**驗證成功**並**錯開**。
- **#22＋W22**：先做 **[O06]／[O19]／[N25-3]／[N10]／[N34]**；其他客戶報表功能開關有開就做，排在後面。
- **#23（W23）**：樣本檔在 `D:\HT9045_Log\EventLogTxt\`，只讀、複製成 fixture（§4）。

**W15／W18／W19 的程式（20260927，St02-E；`v906/steven-ela-wip` `1042d4cc`／`2ceca61c`／`82196ca3`，兩組態只編譯、未上機、ctest 待 St01）**：
- `ela::Options`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.h`）預設＝裁決，每個都能切回 golden 給 #23 對照：
  `bcbCommaText`（#15，false＝SplitEventLogCsv）、`keepGoldenBugs`（#18，原名 keepGoldenLeaks，false＝修掉）、`goldenFileRule`（#19，false＝新選檔＋去重）、`goldenFileRuleVtest`（★W38＝B，true＝VTEST 915／919 照 golden 檔名）、`jamWriteBack`（#17＝A）。
- **一個 parser**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\EventLogCsv.h`（header-only）`ela::SplitEventLogCsv`＝檢視頁的兩個 helper
  （V906 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1351-1385`＝912 `cObserver.cpp:3812-3846`）：引號外逗號切；欄位 Trim 後開頭是引號才去頭尾引號＋`""`→`"`；
  引號內原樣（`"\t"` 仍是 TAB）；沒關的引號到行尾；結尾逗號多一欄。header-only 是因為 cObserver 在 `ht9045_sm`，sm 不連 `ht9045_ela`。
  cObserver.cpp（共用檔，FROM_STEVEN §1 `0c55fd18` 核准）改呼叫它：`a9b93d61`，行數不變（:110 空行改 include、:1351-1385）；SPIL 分支照 golden（W16＝A）。
  ctest `ELA_Core` 第 9 節會讀 cObserver.cpp 原始碼，確認測試裡編的那份就是 `:1369-1385` 的字——改 cObserver 那兩個 helper 要同步改測試。
- **W18 修了什麼**：iFailCount 每次查詢歸零、By-Filter 清單每次清、by-hour 測試時間 ×OneDayMS、單筆不再被 "No Record!!" 蓋、Top5Filter 重建、
  Top5 完全比對、Production_Log 壞行跳過；頁面 `D:\HT9045\web\page\eventlog.html` 功能框各管自己的清單、Top5 完全比對。留：":"＋訊息、Top5 尾端空白列、31 Cylinder。
- **W19 選檔**（`ela::EventLogFileSpan`）：路徑仍要含 `EventLogTxt_`；日檔（含 _RT／_FT／_TesterOffline）[D, D+1)、`D HH` [D+HH, D+1)、`D HHNNSS`（行數上限）[同 base 名前一個檔的時間, 下一個檔的時間)，頭尾開放（Handler 寫出時才命名，列在檔名時間之前）、
  `DHH00` [D+HH, +12 h)（2000 跨到隔天 08:00）、月／年檔整段；重疊就讀、每檔只讀一次（主資料夾先、AllEventLog 後）；
  會計入的列在**別的檔**已經算過就跳過（`dupRowsSkipped`，快照與 Log 分頁看得到），同一檔內重複照算。**★W38＝B，Steven 0928（客戶指定功能固定格式）**：VTEST 915／919 機台預設照 golden 的固定檔名（`<yyyymmdd>`＋`_EventLogTxt_`、不去重；`Options::goldenFileRuleVtest`，false＝回 W19 B；`bf7cb8e3`）。
  選檔集中在 `ela::SelectEventLogFiles`（R1 的 `GetEventLogTextToVec` 也要用它）。
- ctest `ELA_Core`：第 3～8 節釘 golden 模式（oracle），第 9 節 W15（跟檢視頁 helper 逐行比對）、第 10 節預設（裁決）、第 11 節 W19。
  Fixture＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\fixtures\ela\EventLogTxt\`（從 W23 樣本**複製**的 7 個檔），測試先複製到 `%TEMP%\ht9045_ela_core_fx` 再讀，
  AllEventLog 複本與 ByLot／JamStat／RawData 誘餌在測試裡產生。偏離全文：帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 檔尾「W15／W18／W19＝B」。

**R5 ElaSchedule 已做**（St02-E helper 20260927，`v906/steven-elasched-wip` `155e27dd`…`f08cea52`，已合 gpib-widget R1～R4，兩組態只編譯、未上機；ctest `ELA_Schedule` 待 St01；帳本「R5 ElaSchedule」）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.h`／`.cpp`：可注入時鐘（`IClock`／`SchedSystemClock`；R2 已有 `ela::SystemClock`，不能同名）、工作表（`JOB_O06_4`／`JOB_N10_3`／`JOB_N25_3`／`JOB_O19_VTEST`／`JOB_N17_PRODLOG`，
  `JobFn` 回 `JOB_VERIFIED`／`JOB_NOTHING_TO_DO`／`JOB_RETRY`／`JOB_FAILED`／`JOB_NO_BODY`）、閘（O10＝O06-1、功能開關、851／915／919、SIM 不自動 N25-3）、
  錯開（FNV-1a(Machine ID) mod 900／300／本機 0；O06 勾 [O06-6] Use Net Drive＝300 s）、退避（重試 6 次）、開機補跑（`<log root>\UploadFile\ElaScheduleState.ini`）、FTP_Log（`ScheduleLog`）、每個工作自己的紀錄（`StatusJson` 的 `results`）。
- 每個工作自己決定（golden 一個 `bNeedUpload` 同時跑 O06＋N10，拆開＝偏離）；N10-3 方法 3＝**★W44＝B，Steven 0928**：每天 Handler 設的時間一次（`[FTPUpLoad] dN10_3_1_SpecifiedTime` → `n10SpecifiedMinute`；golden 從不存這個欄位＝00:00）、錯過的開機後補發一次（**★W44-2＝補發，Steven 0928**；golden 沒有；D-b：只補最近一次，FTP_Log `boot catch-up`，`50c7d02e`）、改時間重新定位（`59a7dcc7`，帳本 R5「★W44」「★W44-2」）。
- O06-4＝**★W43＝C，Steven 0928**（`b11a7a20`，帳本 R5「★W43」）：跟 Handler 的 [O06-8]「Update Production Record every 10 / 30 minutes」走（`[Event Log] EnanleTimePeriodSaveLog`／`TimePeriodSaveLog`，golden `cConfiguration.cpp:3950`／`:3960`）——
  沒勾＝不自動存（原本 W18 B 每小時、golden 每分鐘），勾了＝每 10／30 分；手動「立即執行」照常；Auto Jobs 分頁的原因寫 O06-8。
- N17 UploadProdLog（W13）的本體就在這裡（`RunN17UploadProdLog`：複製昨天的 Production_Log、比大小）；O06／N10／VTEST（R2 `ElaReports`）與 N25-3（R4 `UploadJamCode`）
  由 `ela::ScheduleUseHubJobs(hub, opt)` 接上（只用 `Hub::Config()`／`GetFtpFactory()`，自己的 Analyzer、時鐘固定在時段；不用改 ElaHub）。
- 接線（合併時，四行）：`ElaService.cpp` `W906_ElaStart` 在 SetDbiProcessHook 之後、`g_hub->Start()` 之前加 `ela::ScheduleInstall(ela::ScheduleSetup());`＋`ela::ScheduleUseHubJobs(g_hub, o);`；
  `ElaHub.cpp` `Hub::Entry` 的 `h->RunOnce();` 後加 `ela::ScheduleTick();`；`W906_ElaStop` 的 `h->Stop();` 與 `delete h;` 之間加 `ela::ScheduleUninstall();`。
- 跟 R4 重疊：R4 `ElaFtp.h` 也有錯開／退避（`StaggerOffsetSeconds`／`BackoffSeconds`／`UploadRetry`，只算不排）；接線時由 R5 退避，`Fnv1a32` 只留 R4 那一份。
- **O10 只有一個來源** `ela::EffectiveO10`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.h`）：有裝 getter（wb_serve：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp:293`，`IniConfig.bO10UseEventLogSaver`）就每次判斷都問它；
  沒裝就用 golden `cprod.cpp:2379-2394` 的公式（`bEventLogAutoSaveFunction ? EnableAutoSaveEventLog : 存起來的 bO10UseEventLogSaver`，旗標注入、預設 true＝InitialCosFunction）。
  R2／R3／R4 的 `AutoJobsEnabled` 與 R5 的閘都走它。兩棵樹的 InitialCosFunction 都把旗標設 true、沒有地方設 false ⇒ 現在等於 `EnableAutoSaveEventLog`（本機是關）。
- 記錄（不需裁決）：第一次開機不補跑＝A；重試 6 次＝共 7 次。N10-3 方法 3 的「指定時間」＝★W44＝B（Steven 0928，已做）；★W44-2＝補發、★W43＝C（Steven 0928，已做，本機只編譯）。
- ★W44-1＝A（Steven 0928）的 ELA 這一半（`d133b136`）：picker 由 HTEditList 以 FloatToStr 存 `[FTPUpLoad] dN10_3_1_SpecifiedTime`（15 位有效數字，例 0.3125＝07:30、0.00763888888888889＝00:11），ELA 讀法不變；
  EL_UPDATE_PARAMETER（Configuration 存檔）後 `ScheduleReloadConfig()` 立刻重排（`ElaHub.cpp` `Hub::RunOnce`）；產生器那一行（tools/editlist/IniConfig.py＋IniConfig.gen.inc）與網頁時間欄是 St01 的。

**R6 已做（頁面＋路由；OEE 只有設計）**（St02-E helper 20260928，`v906/steven-elasched-wip` `353c0c6e`，兩組態只編譯、未上機；ctest `ELA_Schedule` 第 18 節待 St01；帳本「R6」）：
- `D:\HT9045\web\page\eventlog.html` 新分頁「Auto Jobs」：排程每個工作的狀態／原因、上次執行與結果、下次、嘗試 n／7、最後完成、「立即執行」（每次先確認；N25-3／4／5 會用 WinINet 連 FTP——★W36＝C 起模擬組態也是）；
  點一列看那個工作的紀錄；Hub 的 EL_* 工作紀錄（快照 `jobs`）；排程歷史。不換掉查詢畫面。
- 路由（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp`，wb_serve 不用改）：`GET /api/ela/schedule`（排程狀態＋組態／傳輸／有效 O10；不放快照，因為排程改變時 Hub 的 seq 不動）、`POST /api/ela/job?id=<工作名>`（`ScheduleRunNow`）。
- OEE：golden 三張圖是 `random()` 假資料（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:385-412`）；建議先用 `byDay` 在頁面畫每天測試／停機／其他的比例，再做每分鐘狀態條；真的 OEE 要 Steven 定計畫時間、理想 UPH、良率。

**★W48 OEE 圖已做（Steven 0928＝A）**（St02-E helper 20260928，分支 `v906/steven-w48-oee`，兩組態只編譯、未上機；ctest `ELA_Oee`（新）與 `ELA_Hub` 第 3 節待 St01；帳本「★W48 OEE (A)」）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaOee.h`／`.cpp`：`ela::ComputeOee(Analyzer, now, OeeOptions)` 只讀查詢已經留下的 `slStopList`／`slAllData`／`byDayKeys`／`dtStart`／`dtEnd`（不再讀檔、ElaCore 不動），
  `ela::OeeJson` → 快照 `"oee"`（`Hub::PublishSnapshot`，只有使用者的查詢；沒有新路由）。頁面 `D:\HT9045\web\page\eventlog.html` 的「OEE」分頁：每天一條 100 % 橫條（inline SVG）＋表格＋「本月」鈕。
- ~~三項 St02 預設（Q1＝B 觸壓時間軸 G＝300 秒／Q2＝A 關機算閒置／Q3＝A 全部告警算停機）~~ → **Steven 0928 09:3x 裁決**（St02-E helper，分支 `v906/steven-w48-rework`，兩組態只編譯；帳本「★W48 三項裁決」）：
  - **W48-1 測試**＝每次觸壓自己的 Test Time（Production_Log 第 27 欄）：golden 就是 0x41～BIN ON＋ECHO OK 量的（`RecordStartTestTime` 在 `RunTestProgram` 前、`RecordEndTestTime` 在 ECHOOK 之後的 `bEcho`；906_0625_Steven `atester.cpp:1481`／`:1530`／`:1645`、`cObserver.cpp:2136-2161`、`main.cpp:17122`），G 規則拿掉。
    **A8：第 27 欄只是預設順序**——超豐 CC_Greatek（956）走 `SaveDataForGreatek`，Test Time 在第 37 欄（906_0625_Steven `Public\MyProductionRecord.cpp:703-708`／`:858`，表頭 `:46-88`）；`ElaOee` 的 `ProdLogColumns` 先照表頭名字、再看 CUSTOMER_CODE（ctest `ELA_Oee` 第 8 節）。
    ⚠ 但 golden ListProductionLog 用第 5 欄（In Time）篩列，超豐的第 5 欄是 In Y ⇒ 超豐的列進不了 slAllData（上游缺口，待決定；帳本 W48-1 的 A8）。V906 的 SaveRecord 已移植（`Public\MyProductionRecord.cpp:1185`）、三支 `RecordEndTestTime` 已接真的（`4f6f4cbf`）。
  - **W48-2＝B 關機**＝golden 每小時的 TimeData（`RecordTimeData`，V906 `cMyDB.cpp:479` 本來就有）：整點呼叫由 St02 的 `W906_TimeDataHourTick`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp`，掛在 TesterCommTick／Poll，主執行緒）做，不用認領；
    ELA `ReadTimeData`／`ComputeOee` 用 PowerOnTime 分開閒置與關機（沒有 TimeData 的時間算閒置）；頁面多灰色「關機」段。
  - **★W61＝B（Steven，20260929，分支 `v906/steven-w61-startidle`，兩組態只編譯；帳本「★W61＝B」）**：SystemStart 全部算**生產**、再細分測試／contact test／home／其他；沒測試、沒 SystemStart＝閒置。
    最上層＝生產／停機／閒置／關機；生產＝測試＋min(max(TimeData StartTime－計入停機的 WAR24－測試, 0), 區間－測試－停機－關機)，contact test／home 取 TimeData 的 ContactTestTime／HomeTime（第 4／3 欄，同樣平均攤）。
    停機不從 StartTime 扣（golden ShowErrorMessage 先關 SystemStart，`note.cpp:801`／`:991`），**只有馬達告警 WAR24 例外**（ShowMotorErrorMessage 不關，`:1054-1059`／`:1133`、`csystem.cpp:4064-4067`）。
    off-line 在 golden 沒有任何紀錄 ⇒ 算在「其他」。JSON 多 `prodSec`／`contactSec`／`homeSec`／`prodRestSec`＋`…Pct`，W48 的鍵都留著；沒有 TimeData＝生產＝測試（W48 的數字）。
  - **W48-3＝B 停機**＝只有 `GetJemIncludeMTBA` 算的告警（`slAlarmList`，golden Alarm Time）；沒設過的 key 用 golden 預設（JAM＋01～05 區才算），不是一律算——W45 18b 只改 933／967 的 IncludeMTBF。
  - ctest：`ELA_Oee`（改寫，containment first）、`ELA_TimeData`（新，Handler 寫 TimeData＋ELA 讀回；containment first）、`ELA_Hub` 第 3 節，都待 St01。To 那一秒算進去（23:59:59＝整天）。
- golden 沒有真的 OEE（random() 假資料，分頁藏起來），所以這是新功能。V906 從這一版起每小時寫 TimeData，但還不寫帶 StopedTime 的告警列與 Production_Log ⇒ V906 的 log 目前只分得出生產（W61，沒有測試細項）、閒置與關機；測試與停機要 BCB6 機台的 log。
  上機要看：第一個整點的 TimeData 列（第一列帶著 S113 以來的累計）、關機後再開的那一列、第 27 欄跟 GPIB log 的 0x41／ECHOOK 時間差、跟 Handler 的 `labProductTime`／`labRunningTime`（906_0625_Steven `cObserver.cpp:757-759`）比。

**P7a 手動 Save Summary 已做**（St02-E helper 20260928，`v906/steven-elasched-wip` `8e8fc312`，兩組態只編譯、未上機；ctest `ELA_Service` 第 7 節待 St01；帳本「P7a」）：
- golden `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2557-2570`（對話框預設 `D:\RMS\<yyyy-mm-dd>`）→ 頁面查詢列的 Name＋「Save Summary」，`POST /api/ela/summary?name=`，在 hub worker 上用 R2 的 `SaveSummary` 存「頁面這次查詢」；
  資料夾 `HubPaths::summaryDir`（`W906_RMS_ROOT` 接縫），只收檔名；結果是 `SaveSummary` 工作紀錄，查詢畫面不換；報表工作用過分析器之後會先重跑頁面的查詢。
- **P7 XLS 四個按鈕：★ 第五題＝D（Steven 0928），頁面自己組真的 .xlsx**（St02-E helper 20260928，分支 `v906/steven-xls-d`，只改頁面；取代做法 C 的 CSV `e40f110a`；帳本「P7 XLS 匯出」）：
  `D:\HT9045\web\page\ht9045_ela_xlsx.js`（`window.HtXlsx`）＝手寫只儲存（不壓縮）ZIP（PKWARE APPNOTE 4.3.7／4.3.12／4.3.16，CRC-32 0xEDB88320，固定 DOS 日期）＋最小 SpreadsheetML（ECMA-376：`[Content_Types].xml`、`_rels/.rels`、`xl/workbook.xml`、`xl/_rels/workbook.xml.rels`、`xl/styles.xml`、`xl/worksheets/sheetN.xml`）；
  每格 `t="inlineStr"`＋文字格式 numFmtId 49（同 golden `XLSfile.pas:178-192` 的 LABEL 全文字，"0316" 不轉型）、表頭粗體、欄寬、UTF-8。檔名＝golden 預設名改 .xlsx（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2247`／`:2256`／`:2266`／`:2285-2292`）；
  **Top5＝一個 `Top5Alarm.xlsx` 三個工作表** `Top5Alarm`／`AlarmList`／`AlarmList1`（golden 三個 .xls 的檔名，:2266／:2272／:2275）；鈕字＝golden 的 `Save`；每表最多 5,000 列。未上機：Excel 開起來不能跳修復、中文、"0316" 仍是文字、Top5 三個工作表。

**W22 已做：N25-4／N25-5**（St02-E helper 20260928，`v906/steven-ela-wip` `7b69cdc1`，兩組態只編譯、未上機；ctest `ELA_Ftp` 第 15～17 節待 St01；帳本「W22」）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.cpp`：`UploadSummaryCount`（`SummaryCount\y\m\d\Jam_Summary.csv`＝30 天、CLEAN_OUT 分段、每段 Production_Log 顆數＋JAM 數；HadUpload 只有標記）、
  `UploadEventLog`（`UploadEventLog\y\m\d\EventLog.txt`＝昨天 `EventLogTxt_yyyymmdd.csv` 的位元組複本；HadUpload 兩行＋標記）；N25-3 的主檔＋BackUp 上傳共用 `UploadMainAndBackup`。
- 閘＝N25-3 那套＋自己的開關（`bN25_4_EnableUpload`／`bN25_5_EnableUpload`）；排程 `JOB_N25_4`／`JOB_N25_5`（00:00，FTP 900 s，跟 N25-3 同一時段、依 golden 順序）；Hub 的 `EL_UPLOAD_SUMMARY`／`EL_UPLOAD_EVENTLOG` 不再 HOLD。
- 偏離：重算不附加、D-c 沒有列不出、顆數 W18 B 讀每個日檔（golden 從段頭時刻跳一天會少最後一天）、沒東西不連線。待 Steven：N25-5 只找一般檔名（O15／N10 檔名的機台不上傳）。

**#22 盤點的結論**（20260927）：
- N34＝ChipAdvanced 868（lot end → `EL_UPLOAD_CHIPADV_LOTEND`；St01 分支 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp:7082-7095` 的送出端還閘著）；O19 在 VTEST（915／919）＝MTBF（`EL_VTEST_MTBF_SUM`，每週一次約 00:00:04）；N25-3＝ChipMos 竹北 851（`EL_UPLOAD_JAMWEEK`）。
- O06、N10-3 在分析器自己的 Timer2（50 s）；非 VTEST 的 O19 是 Handler 的 cMyDB／cObserver 報表（筆電的範圍）。三者共用 `SaveSummary`。
- golden 上傳從不驗證（沒丟例外＝成功）；V906 的 KYECFTP `TNMFTP` 預設 **SIM**、沒人呼叫 `SetSimMode(false)`，所以現在沒有任何 FTP 真的出網。

**報表與上傳的完整計畫（範圍、排程含 W13 UploadProdLog、上傳安全、R0～R6、ctest、golden 事實、待裁決）→ `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md`（計畫，未實作；R0 已做）。**

**W21 上傳安全**（摘要；WinINet 版，R4 已照這個做）：驗證＝STOR 成功＋`SIZE` 等於本機位元組數（不支援 SIZE 就用 LIST 的大小）＋重新 LIST 看得到檔名；每次重算報表、不再累加；每台錯開＝FNV-1a(Machine ID) mod W；退避 min(60·2^(n-1),1800) s ±20%，最多 6 次重試；真的出網**跟設定的功能開關走（D-f）**，沒有環境變數開關（舊提案的 `W906_ELA_FTP_REAL` 作廢）。

**R4 ElaFtp＋N25-3／N10 BYFILE（20260927，St02-E，`v906/steven-elaftp-wip`；兩組態只編譯、未上機、ctest `ELA_Ftp` 待 St01）**：
- 傳輸介面 `ela::IElaFtp`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h`）：`NewNullFtp`（Hub 預設，連不到網路）、`NewLogOnlyFtp`（只寫 FTP_Log）、`NewWinInetFtp`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtpWinInet.cpp`，全樹唯一 include `<wininet.h>`）。只有 `W906_ElaStart`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` :159-163）裝傳輸。
- N25-3（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.cpp`）：`UploadJamCode` → `SaveJamCodeFor7Days`（Jam Rate.txt 重建、UTF-8）→ `N25_UploadJamDataToFTP`（base／hostname／BackUp、兩個檔都驗證才寫 HadUpload.txt）；N10 BYFILE 讀 `D:\HT9045_Log\UploadFile\UploadFile.csv`。Hub 的 `EL_UPLOAD_JAMWEEK`／`EL_UPLOAD_BYFILE_N10` 走 `Hub::RunUploadJob`；**V906 還沒有人送這兩個指令**（排程是 R5）。
- 連不連（D-f）：N25-3＝O10（O06-1）＋客戶碼 851＋`bN25_3_EnableULJamLog`＋N25-2 主機＋N25-3 路徑；N10 BYFILE＝O10＋`iN10UploadMethod`＝0＋N10 主機。閘沒開＝不建傳輸。
- FTP_Log 歸 R4（`ela::FtpLog`，`D:\HT9045_Log\UploadFile\yyyy\mm\FTP_Log_yyyymmdd.csv`，走 `W906_HT9045LOG_ROOT`），每行遮帳密；R1 ElaFileUtil 的 `MyDBIProcess` 經 `SetDbiProcessHook` 也寫進去（`W906_ElaStart` 裝 `ela::FtpLogDbiLine`，golden Common.cpp:39 同）。R2 不寫 FTP_Log。
- `ht9045_ela` 改連 wininet、不連 `ht9045_nmftp`（庫保留給 KYECFTP）。
- **★W36＝C 已做（Steven 20260928；St02-E helper，`v906/steven-w36-w46`，兩組態只編譯、未上機）**：模擬組態跟出貨組態一樣上傳——`W906_ElaStart` 兩組態都裝 `NewWinInetFtp`，
  `W906_ELA_SIM_FTP_LOG_ONLY`／`W906_ELA_FTP_WININET` 拿掉（歷史在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h` 的 ★W36 註解）；`NewLogOnlyFtp` 留給 ctest。
  排程照 golden：模擬組態不自動跑 N25-3／4／5（main.cpp:21425），手動「立即執行」會真的連線。ctest 照舊不連線（只有 `W906_ElaStart` 裝 WinINet，ELA_Service 呼叫它前先設 `HT9045_ELA=0`）。帳本「R4」★W36 段。
- **待 Steven**：Passive＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h` `kElaFtpPassive`（暫定 false＝active；golden TfFTP 從不設 Passive）——★W46，來源待 St02-M 選。
- 偏離全文、ctest 各節、上機要看：帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 檔尾「R4」。

**計畫**：R0 已做 → R1 ElaCore 補 LogRecord／GetEventLogTextToVec＋ElaFileUtil → R2 ElaReports（SaveSummary、O06／N10／VTEST）→ R3 N34 ChipAdvanced → R4 ElaFtp（N25-3、N10 BYFILE；**20260927 在 `v906/steven-elaftp-wip` 做了**，上面）→ R5 ElaSchedule（排程含 W13 UploadProdLog、錯開、退避、O10 閘）→ R6 eventlog.html 上傳狀態＋OEE 圖。R1／R2 在 `v906/steven-ela-wip`（已合進 gpib-widget `bf8ecd42`）；R4 合併它之後 N25-3 改用 R1 的 `GetEventLogTextToVec`、時鐘用 R2 的 `ela::Clock`。
**#22 已全部裁決（Steven 20260927）**：D-a 照 golden 用 O10 閘住；D-b 驗證＋重算＋開機補跑都要；D-c 加 N34 下溢保護；D-d 產報表時重掃；D-e 報表用 UTF-8（偏離）；D-f 真的連 FTP 跟設定開關走（N25-3／N25-2），不另設 env。FTP 傳輸改用 **Windows WinINet**（`IElaFtp`＋ctest 假物件）。詳見 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` 第 3、6 節。

**還沒**：W15／W18／W19、R1／R2 與 R4 的上機驗證與 St01 的 ctest（ELA_Core、ELA_Reports、ELA_Ftp、ELA_Hub）、R3 的上機驗證；R5 ElaSchedule 的接線與上機（程式已做，見上；接上之前 O06／N10 與上傳不會自己跑）、R6 的上機（頁面與路由已做）；★W48 OEE 圖的上機、ctest `ELA_Oee`／`ELA_TimeData`、W48-1 的三行認領與 Production_Log 寫檔、「Start 但沒測試」要不要另分一段（程式已做，見上）；W22 N25-4／N25-5 的上機與 ctest（程式已做，見上）；V906 還寫不出帶停機時間的報警行，P1 測試檔要用 BCB Handler 的真檔（W23 樣本）。

## 6. 轉換計畫摘要（全文 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md`）

- 目標：`ElaCore`（無 VCL 分析核心）＋`ElaHub`（`EL_*` 行程內化，自己的 worker 執行緒；原計畫的「ElaThread」就是這個 worker）＋`/api/ela/*`＋`eventlog.html`；
  **W21／W13**：排程（分析器 Timer2、Handler 的定時觸發、N17 UploadProdLog 01:00）也在 ElaHub（R5 `ElaSchedule`），上傳由 `ElaFtp` 做並驗證。
- 階段：P0 基準＋帳本 → P1 ElaCore → P2 指令行程內化 → P3 worker＋API → P4 web 檔案檢視 → P5 web 分析 → P6 報表與上傳（＝上面的 R0～R6）→ P7 XLS（20260928 ★ 第五題＝D：頁面組 .xlsx）。
- 順位：使用者 20260926 定的工作順序是 cMyDB → 測試通訊 → event log 分析。

## 7. 跨技能連動

- CSV 的產生端、`TMyStringList`、`AlarmCodeList.txt`：`D:\HT9045\.claude\skills\ht9045-mydb\SKILL.md`。
- 執行緒隔離規則與 `SyncMailbox` 形狀：`D:\HT9045\.claude\skills\ht9045-gpib-bridge\SKILL.md` §8（裁決 11）。
- 執行期 event log 環形緩衝與 `log.tail`：`D:\HT9045\.claude\skills\ht9045-json-bridge\SKILL.md`。
- V906（AI(W906-E021-OB8) 20261002，St01，todo E-021 OB-8）：Observer「Text」分頁的 **Backup Log**＝golden 906 `cObserver.cpp:5371-5391`（V912 `:5602-5622`，兩棵相同；20261003 E-030 補 906）btnBackupLogYearClick——寫 `D:\HT9045\system\2.bat`（13 行 `XCopy /y/a/e/c/i/h/f/r "<EventLogTxt>\<yyyy>\<MM>" "<EventLogTxt>\<yyyy>\<yyyy>"`，12 月那一行 golden 寫兩次）再用 golden `ExecZipCommand`（cpublic.cpp:810，CreateProcess、不等）執行；只複製不刪。年份只取 `atoi(cbbEventLogYear->Text)` 的整數（非數字＝0）。移植樹：WS `act.observer.backupLogYear {"year":...}` → `cObserver.cpp` 檔尾 `W906_E021_btnBackupLogYearClick`（接縫 `W906_BACKUPLOGBAT_PATH`、`W906_EVENTLOG_ROOT`；執行走 `W906_E021_ExecHook`，預設就是 ExecZipCommand，ctest 換成記錄器、從不執行）。注意 golden `HS_Function.cpp:2091` 也用同一個 `system\2.bat`。
