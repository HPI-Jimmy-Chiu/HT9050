# bridge transition 的 HMI 頁面通知

定位 `THandlerTesterSide::StartBridgeProgram`、`CloseGpibProgram` 與 inline `W906_FormProgramShow`，完整原文見 [manifest](source-manifest.json)。

## Start 的條件

無hub回false；先算 `EffectiveBridgeTestType`、`PublishSettings`，type不同或未up時呼叫 `W906_GpibAuxBeforeBridgeStart`。接著保存 `typeChanged`與`wasUp`：type不同→`SelectTestType(t)`，type相同但未up→`Restart()`，已up且type相同→ok=true。

只有 `ok && (typeChanged || !wasUp)` 才呼叫 `W906_FormProgramShow("TSerialPoll",true,where)`。已有同type且up的重複Start不發此open通知。ok仍是hub方法的返回值，沒有檢查瀏覽器已顯示的ack。

## Close 的條件

`bFind==false` 或 `LastSet.iRunStartMode==rsmQAMode && bQAModeFinishCleanOut` 都先返回，未到頁面close通知。
其餘路徑建立COPYDATA、設 `bCloseGpib`／`MSG_CMD_CloseGpib`，`SendToBridge`後刪packet、清bCloseGpib、寫MyDBIProcess、設5秒WakeupGPIBdelay、令bFind=false，最後才呼叫同hook、open=false。

`CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI` 會在送出前把 `HHandler2Gpib.iLotStatus=0`；這只是該函式已看到的分支，不代表全部客戶封包格式或送達已查證。函式沒有由 `SendToBridge` 成功回執決定是否發close通知。

## hook 的證據界線

inline函式只有 `if (W906_FormProgramShowHook) ...`。空hook就沒有呼叫；非空才把form／open／where轉交。來源註解所述wb_serve安裝點、頁面表kPgBoth、wseq、ctest空hook是歷史說明，本單元尚未核對安裝、瀏覽器消費或視窗實際顯示。

兩個bridge函式都有TSerialPoll通知；實際engine可依有效type選擇，不能僅以頁面名稱推定這次必為GPIB或HT9050。

回 [版本入口](index.md) 與 [版本／客戶界線](versions.md)。
