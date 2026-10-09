# 原文：original Steven20260916 AckJson metadata and historical ChangeLog locators（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `6c968381336076cf6cd679f7466790cc200a0ed6da4d2374a642ccb0b5760bfa`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
//Steven 20260916
// ----------------------------------------------------------------------
// AckJson()：成功時若第三個參數是 JSON 物件就併進 ack，不再丟掉。
// 原本那個參數只在失敗時輸出（它叫 error），於是 wb_serve 組好的
// {changed, identical, notFound} 從未送達瀏覽器 —— 前端的「notFound 就拒寫」
// 與變更確認對話框因此雙雙失效，按存檔顯示成功但什麼都沒寫。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

// 本地副本（HT9045 這棵樹）：D:\HT9045\backup\HT9045_V906_changes_20260916b\CHANGES_20260916_Steven.md


<!-- preserved-content:end -->
```
