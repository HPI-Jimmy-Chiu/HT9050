# Dialog bridge：頁面送答與關閉回覆

[回命令入口](../index.md)／[宿主回應](../browser-host/index.md)／[Recipe Client](../recipe-client/index.md)。接續同一hpi-uph-model證據樹，並非新增機種Skill。

| 問題 | Reference |
|---|---|
| iframe ready、active／activeNS、訊息來源及requestId | [頁面訊息](messages.md) |
| 宿主失敗轉送、JsonWriter／WebView／postMessage及debug | [傳輸順序](transport.md) |
| complete、關閉回覆佇列、失敗與拒絕回覆 | [關閉生命期](close.md) |
| 版本、HT9050與其他Handler、客戶／runtime／歷史註解 | [適用界線](contracts.md) |
| 全文、七完整函式maps與metadata保存 | [證據](evidence.md)／[manifest](source-manifest.json) |

七完整選定JavaScript函式175行；全文835行分五原文頁，一context credit0。其餘named functions、anonymous callbacks與API object完整保留，但不增加完成credit。
靜態查證分清頁面送答、傳輸回傳、ACK與C++處理；本單元沒有執行JS、browser、機台或runtime。完整page consumer／載入圖及所有caller仍待續。

## 回覆schema與recent路由接續

[關閉／輪詢入口](close-routing/index.md)：九完整JS142行，沿用同份835全文五頁；同批完成credit另記，原七函式單元及保存件不重做。

## 顯示與兩層佇列接續

[顯示／佇列入口](display-queue/index.md)：十二完整JS141行，沿用同份835全文五頁；分清stop警告／nonstop丟舊、直接render與FIFO、關窗與回覆時序，不重計前單元。
