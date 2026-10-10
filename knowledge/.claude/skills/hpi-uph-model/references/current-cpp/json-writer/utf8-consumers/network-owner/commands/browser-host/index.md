# browser dialog host與overlay差異

[回命令入口](../index.md)；[C++宿主回答](../host-modal/index.md)。同一hpi-uph-model樹，JavaScript與CPP完成統計分開。

| 責任 | 入口 |
|---|---|
| query對應、message／notice／ACK與重試 | [web/page回應](responses.md) |
| auth序列化、結果轉換與取消 | [權限轉送](auth.md) |
| query訂閱、message／auth事件及globals | [訂閱與生命期](subscription.md) |
| 兩份host與各client配對、版本／客戶／runtime | [版本與overlay](variants.md) |
| 完整原文、metadata、function maps及保存 | [證據](evidence.md)／[manifest](source-manifest.json) |

13個完整選定JavaScript函式／302函式原文行；兩整檔507行、4原文頁／2context credit0，包含所有原註解與未計數的IIFE／anonymous callbacks。
這兩份host不是同一內容，配對client的API也不同；未核現場載入路徑／sync部署，不能任選一份稱實機行為。
僅靜態正文、固定來源及引用保存驗證；完整dialog-bridge／page／recipe client與UPH容量／客戶版／S8／846／legacy待續。
