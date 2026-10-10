# WebSocket命令驗證與告警query

[socket owner](../index.md)；[既有ACK](../pumps/ack.md)；[transport](../transport/index.md)。
V906固定來源`34c8e2e164014a93699f0b0a712366f5da01ebd2`：3完整CPP／271函式原文行、7新原文頁含4 context；constants沿用既有頁，同一hpi-uph-model樹，0新增canonical主題。

| 要查的責任 | 入口 |
|---|---|
| JSON欄位、名稱／值型別、拒絕及統計 | [命令驗證](validation.md) |
| acquire／takeover／release與19個完全比對豁免 | [控制權](control.md) |
| browser id、ticket、PendingAck與presence旗標 | [queue交界](queue.md) |
| alarm／query欄位、9按鈕順序與保留狀態 | [事件輸出](events.md) |
| 版本／客戶／runtime、原文／metadata與查證界線 | [證據](evidence.md) |
| 3 body／4 context、沿用constants、63既有manifest、有限census | [manifest](source-manifest.json)／[census](symbol-census.json) |

完整body：[HandleTextMessage](raw/source-01.md)、[PostAlarm](raw/source-02.md)、[PostQuery](raw/source-03.md)。
Context：[CommandQueue.h全文](raw/source-04.md)、[名稱與local timestamp helpers](raw/source-05.md)、[QueuePush adapter](raw/source-06.md)、[owner欄位](raw/source-07.md)、[沿用caps](../transport/raw/source-11.md)。
三候選由intake轉完整references；context／inline／adapter不重計完成。原先「已查讀」不是reference完成，manifest記本次實際保存。
未完成：DoWebSocketUpgrade／ClearQuery／PostQueryOptions與完整query重播／browser／dispatch、crypto、UPH容量／客戶版本／S8／846及legacy退役。

定位更正：先前待辦的SendSnapshot／ClearPendingQuery是在這三個固定來源檔未定位到的暫稱；現以DoWebSocketUpgrade／ClearQuery為準，不作全repo不存在的推論。
