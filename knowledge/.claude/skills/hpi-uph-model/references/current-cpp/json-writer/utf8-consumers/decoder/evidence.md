# 保存與有限適用範圍

[上層](index.md)；[manifest](source-manifest.json)；[原契約及enum／WsMessage](raw/source-06.md)；
[實作framing圖](raw/source-07.md)；[前單元證據](../evidence.md)。
來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`，兩來源整檔blob／SHA256固定；5完整CPP／250行、2context、7讀頁。
56既有manifest按 `7fae2372db83dbf6734bf38c74f5487e60969ada` 去重；constructor、IsValidUtf8、class
透過既有頁／manifest hash引用，新增完成計數0。TryOneFrame包含前單元三個分支context但只算1函式。
保留原文註解、契約、framing圖、metadata與raw byte；若原文有尾端空白，僅按原文hash精確保留。

| 適用軸 | 本輪查證 |
|---|---|
| HT9050／其他Handler | 本pin V906共用WebBridge bytes層；五body沒有MachineType／客戶分支，實際採用caller另核。 |
| V906／V912／V899／golden913 | 只定位此V906來源；其他版本、客戶機台與部署不得由同名推定。 |
| runtime／平台 | header原C++14相容註解保留；本輪未build或link；沒有改machine snapshot／runtime。 |
| caller／驗證 | 只核WsFrame.cpp／.h完整選定正文、詞法位置及文件引用；不是network caller圖或實機驗證。 |

header原「required (and tested)」及RFC註解屬原作者契約／歷史聲明，本輪不把它稱為已重跑測試。
Feed先append、default 1 MiB、PendingBytes分開回值、SawClose先設旗標等以function／變數查證。
保留原註解與實際條件差異，不修改來源來讓文字看似一致。
未寄信、用Claude權杖、啟停其他session或更動共用checkout；!401通知已結依Batch ID不重寄。
仍待network owner／browser完整caller、encoder／handshake、部署角色／ACP／版本客戶、
UPH容量校正與其他Handler、S8／846未決候選及相容入口退役核對。
