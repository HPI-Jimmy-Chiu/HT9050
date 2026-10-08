# W906_TesterCommTick 的次序

定位 `TesterComm/Handler/TesterCommWiring.cpp` 的 `W906_TesterCommTick`；完整定義保存於 [manifest](source-manifest.json)。

1. 先呼叫 `W906_TimeDataHourTick`，才檢查 `g_inited`；未初始化時仍可能走前者，不能把整個 tick 說成被 guard 擋住。
2. 初始化後依序 `PollHandler`、`W906_TcpPumpTick`、`W906_CmdServerPumpTick`、`W906_HanaRmsPumpTick`。
3. `fTesterSide` 存在且 `now - g_lastConnect >= kConnectPeriodMs` 才更新連線時間、呼叫 `ProcessHVisionConnect`，然後再 `PollHandler`。
4. `fTesterSide && InitialOK && !SystemStart` 才呼叫 `SyncBridgeSettings`。
5. 通過 g_inited 後，`THandlerTesterSide::PublishSettings` 每輪呼叫；它不在上述 InitialOK／SystemStart guard 裡。
6. `fTesterSide && W906_GpibAuxNeedsRestart(fTesterSide->bFind)` 才請 `CloseGpibProgram` 處理。

`CloseGpibProgram` 有自己的 QA／bFind 早退條件，見 [bridge](bridge.md)。tick 的請求不等於 Hub 已 Restart、COM 已關閉或 relaunch 已完成。

`PublishSettings`、`W906_GpibAuxNeedsRestart` 的完整定義已在上一層 [設定](../settings-hub/settings.md) 與 [recipe](../settings-hub/recipe.md) 查證；本次補上 caller 次序。`TimeDataHourTick`、pumps、`ProcessHVisionConnect`、poll 全路徑及外層排程仍待追蹤。timer／poll 次數也不是測試成功顆數或 UPH。
