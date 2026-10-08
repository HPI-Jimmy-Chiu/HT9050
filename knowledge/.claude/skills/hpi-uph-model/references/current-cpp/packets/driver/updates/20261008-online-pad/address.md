# 最後成功 ibpad 與不一致訊息

定位 `g_lastPad`、`SetGpibDriver`、`LastPrimaryAddress`、wrapper `ibpad`、`GpibEngine::RunOnce`、`padSaidCard_`／`padSaidWanted_`；原文見 [manifest](source-manifest.json)。

## 這個數字的來源

- `g_lastPad` 初值為-1；每次 `SetGpibDriver(d)` 都先換指標並把它重設-1，包含設同一指標及清指標。getter直接回這個普通int。
- 無driver的 `ibpad` 先回 `NoCard`，不設定記錄。有driver時，先 `Refresh(driver->ibpad(...))`，只有同步後的 `ibsta & ERR` 為0才記 `v`；判準不是局部返回值r。
- 它是 wrapper 對最後成功呼叫參數的快取；不是讀回硬體位址。未經此wrapper的外部改址、driver內部狀態與完整併發caller未驗。

## RunOnce 的觀測順序

`SerialPoll==0 || closed_` 先回idle；close request與DrainRx之後，只有 `bEnableThread && InitialOK` 才呼叫 `ProcessAddress`並進本段。其內另須 `bGpibMode`：

| 條件 | 行為 |
| --- | --- |
| onCard<0 | 未知值，不進不一致log，也不進相符重設分支 |
| onCard>=0、不同於LastSet.GpibAddress，且pair不同於上次記錄 | 記pair，`WriteLog` ADDRESS MISMATCH文字 |
| 同一個不相符pair | 不再重複同一則；pair變成不同的不相符值仍可記 |
| onCard>=0、相符，且padSaidCard_>=0 | 清兩個log cache為-1，讓往後不一致可再次記 |

兩個log cache初值在constructor為-1；本單元不把它們描述成每次Start必重設。記錄之後仍有close check，才進 `ProcessMessage`；timer／UiChannel段各有原本close checks。

這段只有觀測／記錄，沒有補呼叫ibpad來修正位址。log文字含「ibpad never ran」是來源固定字串；單憑快取不一致，不足以排除ibpad失敗或外部改址，也不能把該文字當本輪已證實的故障成因。

`LastSet.GpibAddress` 是比較的意圖值；新 `BuildUiSnapshot`／WebBridge tags的完整發布與消費路徑仍待另一單元查證，不能用舊WebBridge片段仍相同就宣稱新欄位已驗。

回 [版本入口](index.md)；Sim／無driver斷言見 [fixture](fixtures.md)。
