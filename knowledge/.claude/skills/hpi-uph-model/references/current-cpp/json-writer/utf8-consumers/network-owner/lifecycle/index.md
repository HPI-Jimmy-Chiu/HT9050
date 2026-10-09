# WebSocket啟動、停止與平台同步

[socket owner上層](../index.md)；[既有transport](../transport/index.md)；[原設定／API前段](../raw/source-07.md)／[後段](../raw/source-08.md)。
固定V906來源`bebe1bf379f961899a85cc8376ebf1aacc9cf2df`：7完整CPP／194函式原文行，9原文頁含Sync.h全文及WSA globals兩context；不是新增canonical主題。

| 要查的行為 | 入口 |
|---|---|
| Start的同步錯誤、boundPort與thread建立回傳值 | [啟動](start.md) |
| Stop持鎖join、wake／listener與queue清理 | [停止與喚醒](stop.md) |
| WSA refs、nonblocking、Win32 lock／thread helper | [平台與同步](platform.md) |
| 保存、去重、版本／客戶／runtime界線 | [證據](evidence.md) |
| 7 body／2 context與61既有manifest | [manifest](source-manifest.json)／[有限census](symbol-census.json) |

ThreadMain、CloseConn、CloseAllSockets、NowMs等已完成正文沿用transport，不再計新增。
原comment包含歷史工具鏈／probe說明，本次只保存與靜態核對；建置、Win32 API執行結果、browser及機台現場均未測。
尚餘snapshot／outgoing pumps、命令ACK／browser／crypto、UPH容量與版本客戶／S8／846及相容入口退役。
