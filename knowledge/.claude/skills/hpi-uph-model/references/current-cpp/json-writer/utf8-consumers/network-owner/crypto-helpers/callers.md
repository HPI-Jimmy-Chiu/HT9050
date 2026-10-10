# 握手caller與RAW／文字界線

[上層](index.md)；既有[獨立handshake](../../handshake/index.md)／[socket owner Upgrade](../upgrade.md)。
沿用原文：[ComputeAcceptKey](../../handshake/raw/source-22.md)、[IsValidWebSocketKey](../../handshake/raw/source-19.md)、[macro adapters](../../handshake/raw/source-28.md)、[DoWebSocketUpgrade](../raw/source-05.md)。
這些caller只重核固定pin與既有保存內容，completion credit0，沒有重建原文頁或宣稱全repo完整call graph。

## ComputeAcceptKey與編譯期adapter

`WsHandshake.cpp::ComputeAcceptKey`把caller給的Sec-WebSocket-Key字串原樣接kWebSocketGuid，WB_SHA1_RAW取raw20 bytes，再WB_BASE64_ENCODE。
這條路徑不先解碼key再hash，也不把raw digest轉hex；trim由caller負責，ComputeAcceptKey本體未做驗證。
`WsHandshake.h`的ifndef預設指向webbridge::Sha1Raw／Base64Encode，但允許編譯期macro覆寫；目前程式body不能證明每個已部署binary都用相同primitive。
`WebBridgeServer.cpp::sib::AcceptKey`／DoWebSocketUpgrade沿用原owner文件；owner自行解析與獨立checker不得混成同一保證。
本輪有限census包含四helper與三caller來源；tests／其他source tree／實際編譯命令不在完整call graph驗證範圍。

## IsValidWebSocketKey增加的gate

key.size()==24、末兩字元==、前22必須為+/標準alphabet，再Base64Decode成功且raw.size()==16。
它不接受generic decoder可跳過的內部ASCII whitespace；CheckWebSocketUpgrade先TrimOws，再把key傳入checker。
header的strictness理由與body原comment均由既有完整reader保留；padding低位canonical限制未在這兩body找到，不擴稱RFC全符合。
generic decode成功、key checker成功、101入列／socket送出與browser連線成功是不同觀察；本輪只查所列三個caller來源及其既有保存內容。

## Handler／版本／客戶

四helper body沒有MachineTypeChoice或客戶碼分流，可作HT9050及其他Handler共用V906 helper讀法；每台caller／build macro／部署版仍未核。
V912 BCB6／Big5與V899唯讀客戶版沒有在這段做同名程式對照，不推為兩版同實作；golden與歷史HTML保持原版本。
digest、base64 byte長度不是site／HP／Tray容量，也不是UPH計數；宿主answer／browser及產能完整路徑仍待接續。
