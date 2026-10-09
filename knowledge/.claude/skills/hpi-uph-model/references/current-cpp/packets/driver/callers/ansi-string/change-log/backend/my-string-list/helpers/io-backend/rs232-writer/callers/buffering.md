# 通訊文字的緩衝與保存

[上層](index.md)。以下符號均在 `TesterComm/Rs232/` 的 namespace `rs232std`；
`TMyStringList` 有自建 `MyList`，與 Handler 的 `Public/MyStringList.cpp` 分開。

## 資料路徑

`Rs232Ui.cpp::TfRS232Main` 建構片段用三參數 `TMyStringList` 建立 `slRS232Log`，
路徑 `D:RS232LogLOG`、檔名 `RS232_Log`、首列 `Date, Time, Action, Message, Hex`，
再將 `SaveType` 改為 `TBy2Hour`。`Rs232Support.cpp` 的該建構式設
`MaxLineCount=1`、`AutoSave=true`、`bFilePathWithDate=true`，另建 `MyList`。
無參數建構式則設 1000 及 `D:RS232Log`，不能拿它推算此通訊 log 的門檻。

`Rs232Log.cpp::ShowCommData` 兩個 overload 都將
`AddText`／`AddTextWithHex` 的回傳字串加入 `MemoLog->Lines`；
Memo 在 `Count>10240` 時先清畫面。兩個 method 本身沒有 checked 狀態或存檔成功判斷；
上游呼叫的各模式／checkbox 條件不在本輪完整查證範圍。

## 上限 1 不等於每筆或每第二筆立即保存

`AddText`／`AddTextWithHex` 先 `GetTimeInfo()` 組出含毫秒的字串，再做
`if(MyList->Count>HTMaxLineCount) { MySaveToFile(); MyList->Clear(); }`，
最後才 `MyList->Add(Str)`；檢查位置在加入之前，且比較式是嚴格大於。

從空緩衝開始、此 instance 維持上限 1、沒有其他 save／clear 時：

| 呼叫 | 呼叫前 Count | 行為 | 呼叫後 Count |
|---|---|---|---|
| 第 1 筆 | 0 | 加入，沒有保存 | 1 |
| 第 2 筆 | 1 | 加入，沒有保存 | 2 |
| 第 3 筆 | 2 | 先保存前兩筆、清空，再加入第 3 筆 | 1 |

因此 source banner 的「every second line」只能描述穩定段每次保存兩筆的概略，
不能解讀成第 2 筆到來就已落盤。AddText 的回傳值供畫面顯示，沒有保存結果。
`AutoSave=false` 時 MySaveToFile 會早退，但上述 caller 仍 Clear，舊緩衝仍可能丟棄。

## MySaveToFile 的失敗與時間界線

`Rs232Support.cpp::MySaveToFile` 在 AutoSave false、MyList NULL 或 Count 0 時早退。
其他情形先 `GetFileName()`，檔案不存在且 `HTFirstRow!=""` 時加首列；
以 `MyList->Text` 組字串、將 CRLF 換成 LF，呼叫 `fopen(path,"a")`。
只有 `pFile!=NULL` 時才 `fputs`／`fclose`；最後不論開啟成功與否均 `MyList->Clear()`。
fputs／fclose 回傳值也沒有核對，function 為 void；無重試、保存回執或持久化保證。
這是靜態控制流程，未故障注入或量到實際遺失資料。

`GetFileName()` 另取一次 GetTimeInfo，以保存時的 System* 時間選資料夾與檔名；
TBy2Hour 使用 `SystemHour-SystemHour%2`。跨時段的待存行可進入保存時所選檔案，
行內 AddText 時間與檔名分段的取樣點不同，不能據檔名還原每筆到達時刻。
GetTimeInfo 是 Now→DecodeDate／DecodeTime；此鏈未使用單調計時。

GetFileName 會呼叫 `Rs232Globals.cpp::MyForceDirectories` 但不檢查其 int 回傳值。
該 helper 空路徑或例外回傳 -1；ForceDirectories 的回傳值本身未檢查，
無例外走到尾端就回傳 1。helper 的例外記錄還會呼叫 ShowCommData，
遞迴／例外失敗實際行為尚未執行查證。CRT text mode、存放裝置及 flush 的部署效果另查。
