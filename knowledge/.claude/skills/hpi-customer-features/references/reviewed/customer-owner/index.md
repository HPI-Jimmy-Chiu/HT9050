# 客戶碼Web存檔：owner與queue的局部時序

S8續查，來源釘住main `de7c2bf77`，以版本／檔名＋function／變數定位。這次補WebSocket接收、控制權變更、命令保留來源與主迴圈選讀路徑，不宣稱完整權限／競態政策已驗證。

| 問題 | Reference |
|---|---|
| 接收owner、接管／釋放、連線與idle | [控制權生命週期](lifecycle.md) |
| connId／ticket、queue／guard、存檔與ack | [排隊到執行](queue-dispatch.md) |
| 7份Git blob／16個選讀function與界線 | [來源manifest](source-manifest.json) |
| 先前PageSave／HSys、reauth與INI | [Web入口](../customer-web-save/index.md)／[reauth](../customer-reauth/index.md)／[proxy與INI](../customer-storage/index.md) |

V906共用Web結構與機型／客戶／runtime分流都留在同一份Skill，包含HT9050與其他Handler。V912沿原表單查證，不將Web owner／queue說成BCB已驗證等效。原12列人工差異、6382候選／846未決、原文／metadata／來源日期保持；本次沒有新增客戶差異列或執行Web／機台操作。
