# QueueRx、DrainRx 與 Update caller

定位 GpibEngine.cpp 的 TSerialPoll::QueueRx／DrainRx，以及 GpibUi.cpp 的 btnUpdateClick；完整 body 見 [manifest](source-manifest.json)。

## QueueRx

buf 為空或 len 為 0 就返回；其餘在 WbGuard(rxMutex) 區段內，將 port 與指定 len 的 std::string 複本 push 到 rxQueue。這裡複製的是指定長度的資料，包含其中的 NUL；不保存傳入 buffer 指標。

mailbox 非空時 SetEvent(WakeHandle(kEngineSide))；body 未檢查 SetEvent 回傳值，也沒有容量／port 有效性 guard 或送達確認。WbGuard、WakeHandle 與全部 rxQueue writer 並未在本單元重新閉合。

## DrainRx

先在 WbGuard(rxMutex) 區段把 rxQueue swap 到本地 q，再逐筆處理；swap 之後新進資料屬於後續 rxQueue，不能由這一輪推出已全部處理。

每筆取原字串大小轉 WORD n，再附加 NUL，依 port 分流：

| port | 呼叫 |
|---|---|
| kRxCommAMD | CommAMDReceiveData(CommAMD, buffer, n) |
| kRxCommAMD2 | 同一 handler，Sender 為 CommAMD2 |
| kRxAuxTester | fRS232Main 存在時呼叫 CommTesterReceiveData(CommTester, buffer, n) |
| 其他值 | default 不派送 |

Aux 分支只在這裡檢查 fRS232Main，沒有另查 CommTester。附加 NUL 的原註解提及 strlen，不能代替重新查證 parser 的長度／嵌入 NUL 語意。

每筆派送／default 後才檢查 closeRequested，為 true 就 break。若進函式前已為 true，這個 body 仍沒有在第一筆派送之前拒絕；break 後本地 q 的剩餘項沒有寫回 rxQueue。不能把 close request 改寫成「所有資料都派送完成」。

## btnUpdateClick

直接呼叫 fRS232Main->SetFormToData()，沒有本函式內的空指標、連線狀態或 widget.Enabled guard；事件綁定與實際 UI 可觸發條件仍待查。表單欄位、timeout 與 INI 的差異見 [映射](../form-settings/mapping.md)。
