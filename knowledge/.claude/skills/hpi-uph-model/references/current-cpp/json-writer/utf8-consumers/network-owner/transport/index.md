# 接收、送出與連線生命週期

[socket owner上層](../index.md)；[原header前段](../raw/source-07.md)／[後段](../raw/source-08.md)；[Conn原文](../raw/source-11.md)。
V906 UTF-8 C++17來源`8cd0be50e14294b42774510c7fa8accc51b511d6`，10完整CPP／226函式原文行、13原文頁含三個context；此局部不是新增canonical主題。

| 問題 | 路由 |
|---|---|
| select與pumps、送出／retire順序 | [socket thread](loop.md) |
| accept、逐連線關閉、批次清理 | [連線生命週期](connections.md) |
| recv、Enqueue、SendJson、partial send與backlog | [輸入輸出緩衝](buffers.md) |
| idle、Ping、控制權與時鐘界線 | [liveness](liveness.md) |
| 原文保存、intake更正與機型適用 | [證據](evidence.md) |
| 10 body／3 context與既有60manifest | [manifest](source-manifest.json)／[有限census](symbol-census.json) |

傳輸排隊、send接受與peer處理是分開的階段；本單元只核程式中狀態轉換，未做runtime、建置、socket或實機測試。
原HTTP／upgrade／decoder路由沿用上層；完整Start／Stop／Wake、PumpSnapshot／PumpOutgoing、命令／browser／crypto與UPH容量仍待續。
