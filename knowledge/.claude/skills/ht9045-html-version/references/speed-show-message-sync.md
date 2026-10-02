# Setup Speed 與 Show Message 同步

`Setup.Speed.html` 與 `Status.ShowMessage.html` 共用 `page/speed-view.js`。它對應 BCB6 `TfSpeed` 與 `TfShowMessage::ShowSpeed()` / `sgdSpeedViewDrawCell()`；HTML 僅使用 JSON 或同一桌面內的預覽訊息，絕不讀取 `ArmCondition.Data`。

## 資料契約

權威資料為 `Production-update.json.state.productionStreams.speedView`：

```json
{
  "available": true,
  "visible": true,
  "autoSpeed": false,
  "showTotal": false,
  "indexAccelVisible": false,
  "rows": [
    {"name": "Index Arm", "speed": 80, "accel": 100},
    {"name": "Input Arm", "speed": 50, "accel": 50},
    {"name": "Output Arm", "speed": 50, "accel": 50},
    {"name": "Tray Arm", "speed": 30, "accel": 30},
    {"name": "Shuttle 1", "speed": 80, "accel": 80},
    {"name": "Shuttle 2", "speed": 80, "accel": 80}
  ]
}
```

`speed` 與 `accel` 是未加 `%` 的數值或字串；HTML 顯示時才加百分比。`indexAccelVisible` 對應 BCB6 `CUSTOMER_CODE==CC_SCS`，其他客戶的 Index Accel 儲存格保持空白。

## 行為

- `Setup.Speed.html` 的六組輸入值變動時發送 `HT_SPEED_VIEW`；`background.html` 轉送給桌面中所有 iframe，使 `Status.ShowMessage.html` 立即預覽。
- `TfSpeed::spbSaveClick()` 寫檔後會重新讀取並刷新 `sgdSpeedView`；bridge 應於此時發布權威 snapshot。`TfSpeed::sbtExitClick()` 重繪時亦應發布目前值。
- `TfShowMessage::ShowSpeed()` 在非 Auto Speed 且六組 Speed/Accel 全相同時只顯示 `Motor Speed` 一列。其他情況顯示六列。C++ 可直接傳 `showTotal`，HTML 不自行猜測實機 Auto Speed 分支。
- runtime snapshot 有效時優先於 HTML 預覽；缺少 snapshot 時 ShowMessage 保留既有畫面，不以假資料覆寫。