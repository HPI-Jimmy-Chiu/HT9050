# Accept與HTTP response組字串

[上層](index.md)；[ComputeAcceptKey](raw/source-22.md)、[BuildHandshakeResponse](raw/source-23.md)、[SimpleResponse](raw/source-24.md)、[BuildHandshakeErrorResponse](raw/source-25.md)、[BuildTooLargeResponse](raw/source-26.md)；[原macro／契約](raw/source-28.md)。

ComputeAcceptKey把傳入key原樣串接kWebSocketGuid，呼叫WB_SHA1_RAW回raw digest再WB_BASE64_ENCODE。
它不先Base64Decode、trim或驗key；trim與key檢查須由caller安排。
header的macro預設對應Sha1Raw／Base64Encode，允許compile override；本輪沒有build／crypto probe或確認部署override。

BuildHandshakeResponse直接組101、Upgrade／Connection、Accept與結尾CRLFCRLF；
subprotocol非空便原樣加Sec-WebSocket-Protocol，沒有在此驗token／offered清單或去CRLF。
因此這是byte組字串介面，caller須先選擇與驗輸入；它不自行呼叫upgrade checker或送socket。
body不加入Sec-WebSocket-Extensions；這是選定helper的輸出，不能套到所有別的response builder。

SimpleResponse組HTTP/1.1 statusLine、可選extraHeader、Content-Length:0、Connection:close與CRLFCRLF。
這裡的Connection:close只是回傳的文字；實際flush／斷線由network owner決定。
BuildHandshakeErrorResponse的UpgradeRequired用426＋Sec-WebSocket-Version:13，MethodNotAllowed用405＋Allow:GET；
BadRequest／NotWebSocket／Ok以及未知值全走400。傳Ok並不產生101，成功caller必須用另一builder。
BuildTooLargeResponse固定431，與ParseHttpRequest cap回傳值之間仍須caller接線。

選定WebBridgeServer.cpp只有sib::AcceptKey轉呼ComputeAcceptKey的詞法證據；
它未呼BuildHandshakeResponse，在DoWebSocketUpgrade自行組101。獨立checker／error builder存在不等於socket已採用。
原header測試向量與RFC註解完整保留為來源聲明，本輪未執行互通／協定／實機測試。
