# 來源、保存與機型適用界線

[上層](index.md)；[manifest](source-manifest.json)；[有限census](symbol-census.json)。
來源固定`5eaf921a326691d45eccfcd5f00b6fdda26a9340`，UTF-8 V906 C++17的WebBridge；以file、function與變數定位。
六完整body347原文行、13原文頁含六context；header320行分兩頁，原註解、日期、作者、設定契約與metadata保留。
59既有manifest逐blob保留，新manifest與reader payload逐hash、offset及完整正文比對；offset只供此pin保存證據，活文件不用舊行號當權威。

| 來源 | 用途 |
|---|---|
| WebBridgeServer.cpp | AcceptKey／EncodeServerFrame／TryDecode／ProcessHttpHead／DoWebSocketUpgrade／ProcessWsBytes完整原文；Frame／Decoder／Conn與ReceiveInto dispatch有限context |
| WebBridgeServer.h | 全header原文、WebBridgeConfig／stats／threading/API契約 |
| WsHandshake.cpp | 核已完成ComputeAcceptKey與socket自有parser分流，body沿用既有樹 |
| WsFrame.cpp／.h | 核已完成encoder／decoder角色與constructor契約，body沿用既有樹 |

五檔comment／string遮罩後有限spellings查讀；selected server file未命中ParseHttpRequest／CheckWebSocketUpgrade／IsValidWebSocketKey／BuildHandshakeResponse呼叫拼法。
這不是全樹／語意call graph證明；callee、平台、read loop／flush／drop、browser與部署還要分別追。
ReceiveInto只保存dispatch尾段，不算第七個完整函式；Frame／Decoder／Conn、constants與header亦不計新增body完成。

HT9045／9046與HT9050只共享這個通訊層的程式證據；本六body沒有MachineTypeChoice／customer分流，因此不能推定每台都部署此路徑。
HTTP／WS傳輸與UPH預測／量測是不同單位；snapshotBytes只在本body按JSON string長度計數。
V912 BCB6／Big5、golden913與歷史HTML模型不等同這份V906 server原文；版本／客戶／runtime實例與UPH容量仍未查清。
不增加canonical主題，不修改相容入口或舊裁決正文；未執行程式／build／test／runtime／機台。
