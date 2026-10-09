# WebBridge socket owner：HTTP分流與WS訊息交付

[上層](../index.md)；[獨立handshake](../handshake/index.md)；[encoder](../encoder/index.md)；[decoder](../decoder/index.md)。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；六完整CPP／347函式原文行、13原文頁含六段context。

| 問題 | 入口 |
|---|---|
| socket緩衝、HTTP header、dynamic／static route | [HTTP分流](http.md) |
| key／version／Origin、101、snapshot與query replay | [Upgrade](upgrade.md) |
| Feed消耗、ready先於failure、mask／FIN角色 | [Adapter](adapters.md) |
| ping／pong／close、JSON text、pending cap | [訊息處理](messages.md) |
| 版本、機型、原文／metadata與有限查證範圍 | [證據](evidence.md) |
| 固定pin、body/context／reader bytes與59舊manifest | [manifest](source-manifest.json) |
| 五檔遮罩後詞法定位 | [census](symbol-census.json) |

socket owner自行解析HTTP，不能沿用獨立parser／checker的全部保證。
既有helper只沿用引用，不重計canonical主題；UPH容量、完整caller、browser、crypto與實機仍待查。

## 接收與生命週期子樹

[Transport 10完整函式](transport/index.md)追接收、partial send、queue／backlog、逐連線與批次清理、idle／Ping；Start／Stop／Wake與命令owner另續。

## 啟閉與平台同步子樹

[Lifecycle 7完整函式](lifecycle/index.md)追Start／Stop／Wake與WSA refs、Sync.h完整context；thread建立回傳／join／queue清理與原歷史說明分清。

## Snapshot／outgoing與ACK子樹

[Pumps與ACK 5完整函式](pumps/index.md)追generation／baseline、ticket／connId與queue移交；原header／metadata／歷史效能說明保留，送出與peer收到分清。
