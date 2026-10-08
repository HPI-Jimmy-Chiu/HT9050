# EventLogTxt、HANDLER 與 EventTracker

[cMyDB.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.cpp)、[LogObjects.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/LogObjects.cpp)、[aHotPlateSubstrate.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.cpp)。完整所選原文、介面、歷史裁決與 byte／body hash 見 [manifest](source-manifest.json)。

## 暫存cell與EventLogTxt

MyDBIProcess的bSPILFunction分支先GetTimeInfo，以System*組時間，8cells為table、tab AlarmCode、時間、tab Recovery／StopedTime／Duplicate、S1、S2；一般7cells為table、4個tab placeholder、S1、S2。MyDBIProcessNew的SPIL分支把真AlarmCode放第二格；一般分支為table、AlarmCode、3個tab、S1、S2。cell位置不能只看共用header猜測。

兩者用TStringList::CommaText給slEventLog：SPIL走AddTextWithLineNo、一般走AddTextWithDateTime。本輪沿已完成的 [TStrings分隔文字](../../../string-list/delimited-io/index.md) 的quoting／byte語意；TMyStringList再加日期、行號、檔名與保存callee另續。caller僅guard slEventLog!=NULL，沒有實際保存成功回執。

LogObjects.cpp的SaveEventLog呼叫slEventLog->MySaveToFile，再取RunInfo.slEventLogFile->Count與slEventLog->GetFileName。空list直接Add；非空時以AnsiString(Text).AnsiPos(FileName)==0才Add。這是整段Text的substring測試，並非逐cell相等集合。函式本身沒有slEventLog或RunInfo list的null guard、沒有檢查MySaveToFile結果，不能由記下filename宣稱檔案落盤。

## SaveEventLogInfo

alarm code非220000000才先SaveEventTracker；本ChangeLog caller固定220000000，所以不由這條路徑產生EventTracker。所選MyDBIProcessNew可產生其他code，仍須依實參判斷。

fMain非null才讀recipe；fLotInfo非null才讀testerID、lot、handler／login及JamStartDate／Time。其餘placeholder起始為空白。時間取System* globals，本函式沒有每次無條件GetTimeInfo。iType10至21各映射START、END、INPUT、PASS、FAIL、RATE、IDLE TIME、PRODUCTION TIME、DOWN TIME、SOCKET、WARNING、CHANGE LOG；其他PROCESS。iType0先ERROR再改WARNING、latch timer、加error counter、bWrite=false並保存待寫文字。

asSaveEventLogPath下的HANDLER LOG_<SocketHandlerID>_YYYY_MM_DD.csv，FileExists false才以a+開啟並寫16欄header；open失敗return。資料列16欄依序aHandler、aRecipe、aTesterID、JamStartDate／Time、aLotName、LotInName、JamStopTime／ReStartTime／DownTime、aSubject、aStatus、aAlarmCode、aSubject、aMess、sErrPart。SITEID／PROJECTID／MACHINE身分須按這個實參序，不用標籤替換原值。aUPH雖以iRecordEventLogUPH組字，沒有放進此16欄列，不能稱HANDLER列包含UPH。

這份CSV以raw sprintf %s拼逗號，不經CommaText；正文未escape欄位逗號、換行／引號。basename變更時若fLotInfo非null會UploadEventLogFile(aBackEventLogFile)，隨後更新backname；upload的目標／成敗留callee追查。bWrite true fputs、false aBackEventLogMessage=str，最後fclose；回傳void及filename存在都不能保證所有I/O成功。

## SaveEventTracker

aMess含tab且iType!=0時return；type／form取值同所選正文。目標as9045LogPath\ASE log\YYYY\MM\DD\<SocketHandlerID>@YYYY_MM_DD_EventTracker.csv，a+開啟、首次寫12欄header。資料以sprintf組SWVERSION、空SiteID、recipe、SocketHandlerID、日期、JamStartTime、lot／login、subject、alarm、雙引號包aMess與空尾MESSAGE。只有aMess外層雙引號，沒有embedded quote escape；不是通用CSV serializer。aUPH／aTesterID組字未進此資料列。type0改WARNING並latch／加counter，但bWrite=false是comment，仍寫資料。

回 [生命週期](lifecycle.md)、[SQL gate](sqlite-gates.md)、[界線](limits.md)。
