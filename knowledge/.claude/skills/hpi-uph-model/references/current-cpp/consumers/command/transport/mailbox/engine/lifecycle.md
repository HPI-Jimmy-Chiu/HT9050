# Hub 回呼、啟動旗標與停止順序

本頁只描述[manifest](source-manifest.json)釘住的七完整函式和一個宣告；所有呼叫端、OS thread 包裝及並行生命週期仍待查。

## EngineSideHandler 的本地結果

`TesterCommHub::EngineSideHandler`從 `ctx`取得 hub，讀 `engine_`；engine 為空回 -1，否則呼叫 `OnHandlerMessage(payload)`並返回其結果。catch-all 增加 `handlerErrors_`後回 -1。ctx 有效性、engine 持有期間和所有回呼同時切換的情況未查完。

這個整數由先前所選 `SyncMailbox::Process`存入 request result；`kSent`、callback 結果與外部設備收到是三個不同判讀層。GPIB／RS232 的結果另看[payload 子層](payload/index.md)。

## SelectTestType、Start 與 IsUp

`SelectTestType`在 `testType==type_`時直接返回 `engine_!=0 && thread_.Running()`；此分支未呼叫 engine 的 `IsUp`。不同 type 先 `StopEngine`、寫入 `type_`，再檢查 type 範圍／factory、建立 engine、註冊 `EngineSideHandler`與呼叫 thread `Start`；Start 失敗再 StopEngine 並回 false。全部 factory、型別登錄與使用者選取端尚未查完。

`TesterCommThread::Start`在 thread 已 joinable、engine 或 mailbox 為空時回 false；通過後保存指標、清 `stop_`／`startFailed_`，先設 `running_=true`再呼叫 thread 包裝的 `start`。包裝 start 回 false 時清 running 並回 false。

`Running()`只讀 `running_.load()`。因此上述 Start／SelectTestType 的本地 true 不證明稍後 `engine_->Start`已完成。`TesterCommHub::IsUp`還要求 engine 非空、Running 與 `engine_->IsUp()`；各 engine IsUp 實作及外部設備就緒條件仍待查。

## Loop 與停止

`Loop`先 try `engine_->Start(mailbox_)`；例外增加 `errors_`。未 started 時設 `startFailed_=true`、清 running 並返回。正常迴圈增加 heartbeat、Poll engine side，再呼叫 `RunOnce`取得 waitMs；例外計入 errors 並使用 10。stop 檢查後 `WaitForSingleObject`使用該 waitMs，所選 body 不判讀其返回值；10 不是全流程硬性時限。

離開迴圈後，Poll 與 `engine_->Stop()`在同一 try 內依序呼叫；若前面的 Poll 丟例外，這次 try 中的 Stop 不會執行，catch 計 errors，最後清 running。原註解寫 queued sender 不需等待 timeout，不能當成所有排隊、例外、並行條件的保證。

`TesterCommThread::Stop`在非 joinable 時立即返回；否則設 stop、在 mailbox 非空時喚醒 engine side、join，然後清 running。`TesterCommHub::StopEngine`則依序清 engine-side handler、thread Stop、mailbox Reset、delete engine、設空。這是本地語句順序；所有 Reset／Send caller、thread 包裝、同 thread join、request 在途與切換鎖契約仍未閉合。

回[界線](limits.md)與[入口](index.md)。
