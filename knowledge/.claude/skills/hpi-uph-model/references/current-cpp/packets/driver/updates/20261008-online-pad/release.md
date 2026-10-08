# board 解除的程式條件

定位 `GpibEngine::Teardown`、wrapper `ibonl`、`NiGpibDriver::ibonl`／constructor、`SimGpibDriver::ibonl`、`IGpibDriver` 宣告；完整選定原文在 [manifest](source-manifest.json)。

## Teardown 順序

`up_.store(false)` 後刪除 `fRS232Main`、`fDummyART` 與 `SerialPoll`、清 tokens／視窗指標並發布 `up:false`。其後才處理 board：

1. `noncontroller >= 0` 時呼叫 `ibonl(noncontroller, 0)`。
2. 同一條件內把 `noncontroller` 設為 `-1`；沒有檢查呼叫返回值或以返回值決定是否重試。
3. `SetGpibDriver(0)`，再 `delete ownedDriver_`、清 mailbox／started／live。

這證明呼叫排列在 owned driver 刪除前；不能由此宣稱 NI 實體 board 一定已釋放。已注入 driver 的 ownership／完整 Start／Stop caller 仍沿 [既有生命週期](../../lifecycle/index.md) 的固定來源，再按當前版本續查。

## 三層返回值

| 層 | 已核對條件 |
| --- | --- |
| `gpibbridge::ibonl` | 無 driver → `NoCard` 設 `ibsta=ERR`、`iberr=0`、`ibcnt=0` 並回 ERR；有 driver → 呼叫 driver，再由 `Refresh` 重讀 Status／Error／Count，回原返回值 |
| NI | constructor 將13個 slot 歸零；12個 `kNames` 必要 export 缺任一即卸載、清slot並返回；全部具備後另查 optional `ibonl`。`F_onl`缺失 → 局部設定 `lastSta_=CMPL`並回 CMPL；具備 → `NI_CALL(FnII,F_onl,(ud,v))` |
| Sim | constructor 的 `onl_=-1`；`ibonl`只記 `onl_=v`、`Recompute()`、回 `sta_`。inline `LastOnline`回該記錄；它不是硬體釋放探測 |

`NiGpibDriver::Status` 優先呼叫 `fn_[F_sta]`，才在它缺失時取 `lastSta_`。因此 optional export 缺失時的**局部返回 CMPL**，不能推出 wrapper 同步後的**全域 ibsta 必為 CMPL**。`NI_CALL` 缺slot的局部fallback為ERR；DLL 實際狀態未量測。

`SimGpibDriver::Recompute` 只重設 sta_為CMPL、依 in_／talk_加LACS／TACS。`ibonl(...,0)` 沒有清佇列、talk_、err_或cnt_的語句；本段不把Sim記錄等同實機online/offline狀態。

回 [版本入口](index.md)；位址driver life重設見 [位址觀測](address.md)。
