# reader 與 SIM 的 callback

定位 Comm.cpp 的 ReaderProc_ 與 SimInjectReceive；完整 body 見 [manifest](source-manifest.json)。

## ReaderProc_

Win32 body 使用 Impl 指標與本地 `char buf[1024]`，每輪在 ReadFile 前與返回後各檢查 bStopReader；停止時退出，ReadFile 失敗時也退出。

讀到資料且 OnReceiveData 存在，直接呼叫 im->self->OnReceiveData(self, buf, Word(nRead))。這個函式沒有把 callback 自動改派到 form thread；本地 buffer 的生命期依呼叫返回界線處理，後續保存要看接收 callee。

讀取成功但無資料時，evStop 非零就 WaitForSingleObject(evStop, 1)，否則 Sleep(1)。原註解的單機毫秒量測保留為歷史，不能當固定 UPH 開銷。非 Win32 body 僅忽略 param 並回 0。

停止旗標是在讀取前後檢查，不是在 callback 呼叫期間完成同步；不能由這兩個檢查宣稱所有停止／釋放競態已排除。

## SimInjectReceive

先檢查 OnReceiveData；沒有就返回。有 callback 時建立本地 vector：pData 非空且 len 大於 0 才複製資料，否則 vector 為空。以空指標或 vector 首位指標直接呼叫 OnReceiveData(this, p, len)。

此 body 沒有檢查 bSim、bOpen、bStopReader；也沒有在 pData 為空時把原 len 改成 0。因此「SIM receive 注入」的名字不能當作只在 SIM／連線期間才可呼叫的 guard。這裡沒有實際注入資料。

Aux callback 到 QueueRx 的 caller 見 [所有權](../form-settings/ownership.md)；QueueRx 的立即複製與後續 parser 界線見 [佇列](queue.md)。
