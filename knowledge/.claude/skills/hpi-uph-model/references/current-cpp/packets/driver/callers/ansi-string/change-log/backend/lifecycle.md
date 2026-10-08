# 記錄物件、ProductionLog 與啟閉時序

[cMyDB.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.cpp)、[LogObjects.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/LogObjects.cpp)、[aHotPlateSubstrate.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.cpp)。完整所選原文、介面、歷史裁決與 byte／body hash 見 [manifest](source-manifest.json)。

W906_CreateLogObjects在g_logObjectsCreated已true或fMain==0時return，否則先settrue，再建立一系列TMyStringList。這個布林是重複呼叫guard，沒有mutex、分配失敗rollback或設定變更重建流程。

slEventLog以as9045LogPath+"\EventLogTxt"、名稱EventLogTxt建立。SPIL header是UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart；一般header是Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe。實際新增cell走 [事件文字](event-rows.md)，加日期／行號與filename規則待TMyStringList深層單元；不把header直接當每個caller已對齊的證據。

完整create body也保留其他log物件、SaveType／FixedFileMax／header與SiteMap facade swap；這些內容是完整保存範圍，未宣稱全24類log的writer與機台流程已完成。SiteMap adapter／HeaterLog hook等callee也沒有在本輪新增完成數。

create尾端，IniConfig.bO06SaveLogTimePeriod=true且當天ProductionLog檔存在時，先讀進fMain->MemoProductionLog->Lines。當年comment描述避免既有日檔被memo整份保存截斷；本輪只確認所選呼叫與條件，沒有讀寫runtime日檔或驗證path。

## ProductionLog

bO06SaveLogTimePeriod=false即return；true時GetTimeInfo、組asProductionLogPath\<SocketHandlerID>_YYYYMMDD.logs。檔不存在時Clear memo；依JamCode是否空字串組時間／訊息列，Lines->Add。行數>32768、Message=="Close"或bSaveToFile true才SaveToFile；只有超32768時再Clear。MyDBIProcess／New傳true，所以啟用此功能時每次會走所選SaveToFile分支，並非a+append單列。每日切換／memo讀入與底層TStrings保存例外語意仍依已完成及後續callee文檔，不當runtime成功。

所選ProductionLog沒有fMain／MemoProductionLog／Lines逐層null guard；只看bO06 flag不能推導物件有效。完整歷史comment保留VCL／vclcompat差異說明，未重新做913 RTL／BCB6或路徑失敗測試。

## destroy與wb_serve

W906_DestroyLogObjects在!g_logObjectsCreated或fMain==0時return，否則先false、HeaterLog("Close",false)與ProductionLog("Close")，再清Heater hook、delete／null所選log物件。SiteMap real刪除後還回g_siteMapStandIn。slEventLog、slLotInfolog、slHanaTrayMap依保留comment未delete；所以不能稱所有log objects都釋放、或slEventLog在此必然flush。destructor／thread/callee另續。

[wb_serve](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp) 所選boot line在載入config／recipe的後續段落呼叫W906_CreateLogObjects、MyDBUpdateDB，再ElaStart／設Ela hook；關閉段落是TesterCommShutdown、W906_DestroyLogObjects，再清Ela hook／ElaStop。這是明確所選source順序，不代表每個啟動分支／測試／thread都停妥，也沒有啟動wb_serve。歷史banner「ctest全不create」保留為當年說明，本輪未對當前tests作全面coverage裁決。

回 [入口](index.md)、[待驗範圍](limits.md)。
