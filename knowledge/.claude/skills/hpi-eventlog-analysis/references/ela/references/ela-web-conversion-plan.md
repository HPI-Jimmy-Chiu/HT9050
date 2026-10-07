> 保存來源：`.claude/skills/ht9045-eventlog-analyzer/references/ela-web-conversion-plan.md`，main `2db43115d`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Event Log Analyzer（EventlogAnalyzer.exe）轉成 V906 web 頁的計畫

> 本檔為 `ht9045-eventlog-analyzer` 的 reference。工作副本同步放在 V906 樹
> `docs/ELA_20260926_WEB_CONVERSION_PLAN.md`；兩邊內容相同，改動時兩邊一起改。

- 日期：2026-09-26
- 來源
  - 分析器：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code`（SVN 工作副本，20260330）。agent 記錄的最新版本名是 `EventlogAnalyzer_Rev891.0_20251125`，但 `D:\EventlogAnalyzer` 現在只剩 `.github` 與空的 `backup`。
  - agent：`D:\EventlogAnalyzer\.github\agents\EventlogAnalyzer.agent.md`（使用者 20260926：「這是舊的 event log 分析器」）
  - Handler 端：golden V912 `cObserver.cpp`（`tsEventLogTxt` 分頁：`GetEventLogText()` :3857、`cbbMonthChange` :3764、`lstEventLogClick` :3758、`btnQueryEventLogTxtClick` :4479、`ParseEventLogLine` :3848）、`Interface/InterfaceSYS.{h,cpp}`（`EventLog_COMMAND`、`SendCommand_EventLog()` :479）、`main.cpp:18330/18447/18611`（找視窗、拉起 exe）
  - 資料：`D:\HT9045_Log\EventLogTxt\yyyy\mm\EventLogTxt_yyyymmdd.csv`（`TMyStringList` 產，見 `ht9045-mydb`）、`D:\HT9045_Log\Production_Log`
- 目標樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`
- 使用者裁決（20260926）：「EventlogAnalyzer.exe 由 web 頁取代的時機——這個也提出一個計畫，它會跟 tsEventLogTxt 這裡有關係（`void TfObserver::GetEventLogText()`）」；「針對分析器的轉換，也要有個 skill 做紀錄」；工作順序 cMyDB → 測試通訊 → event log 分析。
- 狀態：計畫，尚未動工（排在第三順位）。

---

## 0. 一句話

分析器是一支獨立的 BCB6 程式。Handler 在 `bUseMDB==false` 時把它拉起來，用 `WM_COPYDATA` 丟 7 種 `EL_*` 指令。它讀 `EventLogTxt` 的 CSV 算停機、告警、MTBF、OEE，畫表畫圖、存報表、FTP 上傳。
Handler 自己的 `tsEventLogTxt` 分頁只是「看某一天的檔、簡單過濾」。
併入 V906 後，分析核心翻成無 VCL 的 `ElaCore`，跑在一條與機台隔離的 worker 執行緒。`EL_*` 指令改成行程內呼叫，結果用 JSON 給 `eventlog.html`。這一頁同時取代 `tsEventLogTxt` 分頁與 `EventlogAnalyzer.exe`。

---

## 1. 現況盤點

### 1.1 分析器程式（SVN Code 20260330）

| 檔 | 行數 | 內容 |
|---|---|---|
| `Analyzer.cpp/.h/.dfm` | 3,264／655 | 主表單 `TfrmELA`；`TAnalysisProcessThread`（TThread，7 個 `bEL_*` 旗標觸發）；`Timer1`（掃 `EventLogTxt`／`Production_Log` 目錄、觸發 `EL_UPDATE_PARAMETER`）、`Timer2`；`OnMyCopyMsg`（`M_V` 封包，`WM_EventAnalysis`）；`ProcessHMountConnect`；核心 `GetEventLogText()`（:782-1182：日期區間逐日讀 CSV，算 `MySummary` 的 Stop／Alarm／Fail 次數與時間、JAM／WAR／MES 計數，`mapAlarmSummary`／`mapJamSummary`／`mapWarSummary`）；`GetEventLogTextToVec`；`SetStringGrid`（`sgByDay`／`sgByHour` 含 MTBF 欄）、`UpdateSgTop5*`／`UpdateSgByFilter`／`UpdateSgFailAndAlarm`／`UpdateSgProduction`；OEE 三組 TeeChart（`SetOee*`，FormShow 時三個分頁 `TabVisible=false`）；Jam 等級 `GetJamLevel`／`GetJemIncludeMTBA`／`GetJemIncludeMTBF`；`SaveSummary`／`O06SaveSummaryData`／`N10SaveSummaryData`／`DoVTestSaveSummary`；`DiskCheck` |
| `Common.cpp/.h` | 1,020／179 | ini 讀寫、`EventLog_COMMAND`、`M_V`、`LogRecord`、`VerInfo` |
| `uChipAdvancedFunc` | 258 | ChipAdvanced：`AnalysisLog`、`GenerateOEEReport`、`GenerateAlarmReport`、`FindStartLotFromRecVec`、`SaveReport` |
| `uChipMosZHUBEI_Func` | 419 | 南茂竹北：`UploadSummaryCount` 等 |
| `uAnalysisProductionLog` | 210 | `Production_Log` CSV、時間段內裝置數 |
| `uAnalysisEventLogText` | 42 | 檔名／資料夾規則 |
| `uAnalysisProductData` | 58 | 生產資料 |
| `TfFTP.cpp/.h` | 657 | FTP（BCB FastNet `NMFTP2`），log 到 `D:\HT9045_Log\UploadFile\FTP_Log` |
| `FileInfo`、`HTMD5`、`MyStringList`（839 行，舊版） | 545／632／839 | 檔案資訊、MD5、CSV log 類別 |
| `SgdToXLS` | 16 | StringGrid → XLS（殼） |

七個指令與觸發者：

| 指令 | Handler 端呼叫（golden V912） | 分析器動作 |
|---|---|---|
| `EL_UPDATE_PARAMETER`=0 | `cprod.cpp:3140`、`main.cpp:25374`；分析器 `Timer1` 也會 | 重讀設定、重算 summary |
| `EL_UPLOAD_JAMWEEK`=1 | `cConfiguration.cpp:7862`、`main.cpp:22066` | 週 Jam 報表上傳 |
| `EL_UPLOAD_CHIPADV_LOTEND`=2 | `uLotInfo.cpp:2332` | ChipAdvanced Lot End 報表 |
| `EL_UPLOAD_SUMMARY`=3 | `cConfiguration.cpp:7867`、`main.cpp:22071` | 南茂 `UploadSummaryCount` |
| `EL_UPLOAD_EVENTLOG`=4 | `cConfiguration.cpp:7872`、`main.cpp:22076` | Event log 上傳 |
| `EL_UPLOAD_BYFILE_N10`=5 | N10 串聯傳檔 | FTP 傳檔紀錄 |
| `EL_VTEST_MTBF_SUM`=6 | `HS_Function.cpp:272` | 偉測 MTBF 文件（前七天） |

### 1.2 Handler 端的 `tsEventLogTxt` 分頁（golden V912 cObserver）

- 位置：`fObserver.pgcObserv.tsMDBQuery.pgcMessage.tsEventLogTxt`；元件 `cbbEventLogYear`／`cbbMonth`／`lstEventLog`／`cbbFilter`／`strngrdEventLog`／`btnQueryEventLogTxt`。
- `cbbMonthChange`：`SearchFileAll("D:\HT9045_Log\EventLogTxt\<yyyy>\<mm>\", "*.CSV")` 列檔，選最後一個，呼叫 `GetEventLogText()`。
- `GetEventLogText()`：讀選中的檔；超過 10,000 行不顯示；`cbbFilter` 0=全部、`JAM only`／`WAR only`／`MES only`（SPIL 格式看第 1 欄、一般看第 3 欄，`AnsiPos(...)==1`）、其他=UnitName 含字串；`ParseEventLogLine` 逐欄；SPIL 格式多一個 `No.` 欄。
- **這一段是「單檔檢視器」**，與分析器的 `GetEventLogText()` 同名但不同物（分析器那個是多日統計）。

### 1.3 V906 現況

| 項目 | 事實 |
|---|---|
| `cObserver.cpp` | `lstEventLogClick`／`cbbMonthChange`／`GetEventLogText`／`btnQueryEventLogTxtClick` 已翻譯 ACTIVE（:1245-1472），用 `Public/HTMD5.h` 的 `SearchFileAll` |
| `InterfaceSYS.cpp:552` | `SendCommand_EventLog()` 已翻，`FindWindow("TfrmELA")` 找不到就 return；V906 只有 `cprod.cpp:3303` 一個呼叫點 |
| `JsonBridge/EventLog.cpp` | 環形緩衝＋`log.tail`；只有執行期事件，不讀 CSV |
| web | `page/Data.Observer.html` 已有 `tsEventLogTxt` 版面，未接資料 |
| 分析器 | 沒有翻譯；C++ 機台不帶 `EventlogAnalyzer.exe`（單一執行檔裁決） |
| 樣本資料 | 本機 `D:\HT9045_Log\EventLogTxt\2026\04~05\` 有真檔，表頭 `Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe` |

---

## 2. 目標架構

```
 EventLogTxt CSV ──► ElaCore（無 VCL：讀檔、ParseEventLogLine、MySummary、by-day/by-hour、Top5、Jam 等級、MTBF/MTBA）
                         ▲ 跑在 ElaThread（WbThread，與 tick、TesterComm 都隔離；只讀 CSV、只寫報表檔）
   EL_* 指令 ─────────────┤  ← SendCommand_EventLog() 先呼叫 ElaHub::Post(cmd)（FindWindow／WM_COPYDATA 那段保留給手動開的 exe；#31＝A（使用者 20260927），見下）
   web /api/ela/*  ────────┘  ← files / view?file&filter / summary?from&to / byday / byhour / top5 / alarms
                         ▼ JSON 結果快照（WbMutex）＋ ela.* tag（busy / lastRun / lastError）
                 eventlog.html：檔案檢視（取代 tsEventLogTxt）｜Summary｜Top5｜OEE 圖｜匯出｜上傳狀態
```

- **執行緒**：一條 `ElaThread`，接點只有指令 inbox 與結果快照（比照測試通訊裁決 11 的隔離規則）。分析是檔案密集，永遠不在 tick 上跑；web 查詢先回 busy，完成時由 tag 通知。`Timer1` 的目錄掃描改成按需。
- **指令**：`SendCommand_EventLog(cmd, data)` 本體改成 `ElaHub::Post`；`M_V`／`WM_EventAnalysis`／`HEventLogWnd`／`FindWindow("TfrmELA")` 退場。（改成：hook 先送 Hub、WM_COPYDATA 照送，給手動開的外部 EventlogAnalyzer.exe——它還做 HOLD 的報表／上傳，等 #22。#31＝A（使用者 20260927）：V906 **不啟動**舊 exe，G8 永久 `#if 0`，分析器只有行程內 ElaHub。）上傳類用 V906 既有 `KYECFTP`（`ht9045_kyecftp`）取代 `TfFTP`。
- **檔案**：讀寫路徑與檔名照分析器；XLS 往後排（與 cMyDB 的 Summary Report XLS 同一批），先提供 CSV 下載。

| 分頁 | 取代什麼 | API |
|---|---|---|
| 檔案檢視 | Handler `tsEventLogTxt` | `/api/ela/files?y&m`、`/api/ela/view?file&filter`（沿用 10,000 行上限與過濾規則） |
| Summary | `sgByDay`／`sgByHour`／`MySummary` | `/api/ela/summary`、`byday`、`byhour` |
| Top5／Filter | `sgTop5`／`sgTop5Filter`／`sgByFilter`／`sgAlarm` | `/api/ela/top5`、`/api/ela/alarms` |
| OEE | 三組 TeeChart | summary 資料畫 web 圖 |
| 匯出／上傳 | `btnSave*`、`EL_UPLOAD_*` | CSV 下載；`ela.upload.*` tag |

---

## 3. 目錄與建置

```
HT9011UC_Cpp_V3.33.906.0/
  EventLogAnalysis/
    ElaCore.h/.cpp        ← Analyzer.cpp 的多日統計、表格資料、Top5／Filter、Jam 等級
    ElaReports.cpp        ← SaveSummary／O06／N10／VTEST／uChipAdvancedFunc／uChipMosZHUBEI_Func
    ElaProduction.cpp     ← uAnalysisProductionLog／uAnalysisProductData／uAnalysisEventLogText
    ElaHub.h/.cpp         ← EL_* inbox、結果快照、worker 生命週期
    ElaApi.cpp            ← /api/ela/* 與 ela.* tag（tick stage）
  Interface/InterfaceSYS.cpp  ← SendCommand_EventLog 改呼叫 ElaHub
  web-overlay/eventlog.html, js/eventlog.js
  tests/test_ela_*.cpp（fixture：D:\HT9045_Log\EventLogTxt 2026\04~05 真檔副本）
  docs/ELA_PORT_LEDGER.md
```

CMake：新 target `ht9045_ela`（依 `vclcompat`、`ht9045_globals`、`ht9045_kyecftp`），`wb_serve` 連結；無 TeeChart、無 FastNet。

---

## 4. 分階段與驗收 gate

| 階段 | 內容 | 驗收 |
|---|---|---|
| **P0 基準＋帳本** | 定基準版本（SVN Code 0330 vs agent 記的 Rev891.0）；`Analyzer.cpp` 每個函式分類 core／report／ui／comm；`docs/ELA_PORT_LEDGER.md` | 函式 100% 分類 |
| **P1 ElaCore** | 多日 `GetEventLogText`、`ParseEventLogLine`（與 cObserver 共用一份；W15＝B 20260927：`EventLogAnalysis\EventLogCsv.h` `ela::SplitEventLogCsv`，cObserver 也改用它 `a9b93d61`）、summary 結構、by-day／by-hour、Top5／Filter、Jam 等級與其 ini | `test_ela_core`：兩天真檔的計數與 BCB exe 同資料結果一致（先用 exe 跑一次留 oracle） |
| **P2 指令行程內化** | `SendCommand_EventLog`→`ElaHub::Post`；七個入口；`FindWindow` 保留給手動開的 exe（#31＝A（使用者 20260927）：V906 不啟動它） | `test_ela_hub`：七個指令各到對應函式 |
| **P3 worker＋API** | `ElaThread`、快照、`/api/ela/*`、`ela.*` tag；隔離驗收比照測試通訊 | busy／完成／錯誤三態；故障注入下 tick 抖動 ≤1 |
| **P4 web 檔案檢視** | `eventlog.html` 第一分頁，取代 `tsEventLogTxt` | 年月切換、檔清單、三種過濾、10,000 行上限訊息 |
| **P5 web 分析分頁** | Summary／by-day／by-hour／Top5／Filter／OEE 圖 | 與 P1 oracle 一致 |
| **P6 報表與上傳** | `ElaReports`；FTP 改用 `KYECFTP` | 報表檔與 BCB 版逐行相同（除時間戳）；測試 FTP 上傳成功 |
| **P7 XLS** | 往後排 | — |

工作量：core 約 1,500 行、reports 約 900、hub/api 約 400、web 約 600。

---

## 5. 需要使用者裁決的點

1. `eventlog.html` P4 上線後，web 端的 `tsEventLogTxt` 分頁是否直接下架（建議下架，一個入口）。
2. FTP 用既有 `KYECFTP`（建議）或另翻 `TfFTP`。
3. OEE 圖先做 web 圖（建議）或只出表。
4. 客戶報表（ChipMos 竹北、ChipAdvanced、VTEST MTBF、O06／N10）全翻或只翻有出貨的客戶。
5. XLS 確認與 cMyDB 的 Summary Report XLS 同一批往後排。

---

## 6. 風險

- 分析結果依賴 CSV 欄位順序與 SPIL 旗標；`ht9045-mydb` 的 P1 之後，V906 寫的 CSV 必須與 BCB 逐欄一致。
- 編碼：BCB 寫的 CSV 是 Big5，V906 讀取要轉碼。
- 基準版本不明：`D:\EventlogAnalyzer` 的版本目錄已不在，P0 要先確認 SVN Code 與 Rev891.0 的新舊。
- 目錄掃描改按需後，第一次開頁列大量檔案可能慢。

<!-- preserved-content:end -->
