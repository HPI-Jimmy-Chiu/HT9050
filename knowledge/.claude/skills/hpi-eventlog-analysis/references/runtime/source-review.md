# 本次局部靜態來源查證

基準main `2db43115d065beaa81ddbc69d0bd34df17a9959b`；[source manifest](source-manifest.json)記錄12份Git blob與各檔查閱範圍。只讀Git來源，不讀機台runtime／外部SVN、不執行build／ctest／exe、HTTP、排程或FTP。

| 版本／檔案 | function／變數定位 | 此批確認的界線 |
|---|---|---|
| V912與V906 Interface/InterfaceSYS.cpp | SendCommand_EventLog、CosFunction.bUseMDB、HEventLogWnd、M_V／CMD | 兩函式正文的MDB閘及WM_COPYDATA相同；V906先加W906_ElaPostHook；沒有查完整編譯／caller |
| V906 EventLogAnalysis/ElaService.cpp | W906_ElaStart／Stop、g_hub、HT9045_ELA、NewWinInetFtp、ScheduleInstall／UseHubJobs／Uninstall | start正文安裝WinINet factory／schedule並start worker；stop先停worker再卸schedule；沒有查wb_serve全生命週期 |
| V906 EventLogAnalysis/ElaHub.cpp | Hub::Entry／RunOnce、ScheduleTick、EL_UPDATE_PARAMETER、ScheduleReloadConfig | worker有tick呼叫，重讀設定分支有排程重排呼叫；未證實際worker／runtime啟用 |
| V906 EventLogAnalysis/ElaSchedule.cpp | ReadScheduleConfig、N10SpecifiedTimeRule、AddN10Rules、Scheduler::Tick、JobEnabled／CustomerCodeAllows | W44指定時間／W44-2補發與局部客戶閘；完整排程／傳輸流程未查 |
| V906 EventLogAnalysis/EventLogCsv.h | SplitEventLogCsv、detail::AddEventLogCsvField、inQuote | 局部切欄正文，沒有重新跑parser或全下游統計 |
| V906 EventLogAnalysis/ElaCore.h | Options各宣告 | 僅宣告與原註解；構造預設值／算法未重查 |

ElaReports.cpp、ElaOee.cpp、ElaFtpWinInet.cpp、ELA_PORT_LEDGER.md及web/page/eventlog.html本次只核對存在／blob，未重查其行為／測試結果。來源檔與函式陳述不是完整前處理／link或實機驗證，不把舊「只編譯／ctest待測」升級為本次通過。

ReadConfig與查詢／報表原有寫回行為保留在原文；不從名稱「analysis」推為純讀取。完整binary／caller、Model、CC、功能開關、資料完整性與版本對照待查。
