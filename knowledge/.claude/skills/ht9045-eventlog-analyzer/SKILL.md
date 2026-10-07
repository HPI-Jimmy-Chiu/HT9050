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

# ht9045-eventlog-analyzer 相容入口

同主題已整合到 [hpi-eventlog-analysis](../hpi-eventlog-analysis/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-eventlog-analysis/references/ela/original-entry.md)

## 1. 這支程式是什麼

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#1-這支程式是什麼)

### 1.1 版本資料夾命名規則（BCB 線，自舊 agent 併入）

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#11-版本資料夾命名規則bcb-線自舊-agent-併入)

### 1.2 約束條件（BCB 線，自舊 agent 併入）

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#12-約束條件bcb-線自舊-agent-併入)

## 2. 與 Handler 的介面

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#2-與-handler-的介面)

## 3. 兩個同名的 `GetEventLogText`（不要混淆）

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#3-兩個同名的-geteventlogtext不要混淆)

## 4. EventLogTxt CSV

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#4-eventlogtxt-csv)

## 5. V906 現況（20260927）

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#5-v906-現況20260927)

## 6. 轉換計畫摘要（全文 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md`）

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#6-轉換計畫摘要全文-dht9045claudeskillsht9045-eventlog-analyzerreferencesela-web-conversion-planmd)

## 7. 跨技能連動

[讀取此節](../hpi-eventlog-analysis/references/ela/original-entry.md#7-跨技能連動)
