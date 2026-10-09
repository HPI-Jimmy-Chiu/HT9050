# Upgrade分類與key檢查

[上層](index.md)；[IsValidWebSocketKey](raw/source-19.md)、[LooksLikeWebSocketUpgrade](raw/source-20.md)、[CheckWebSocketUpgrade](raw/source-21.md)；[原header](raw/source-28.md)。

LooksLikeWebSocketUpgrade只看Upgrade值comma token中有websocket；不要求GET／Connection／endpoint。
CheckWebSocketUpgrade先清whyNot；沒有websocket token回NotWebSocket，之後才視為upgrade attempt。
依序要求method恰GET、version恰HTTP/1.1、Connection有upgrade token、version header存在且trim後13、
key trim後非空且IsValidWebSocketKey。method錯為MethodNotAllowed，HTTP／Connection／缺version／key錯為BadRequest，
有version但非13為UpgradeRequired；第一個失敗決定結果與whyNot。

IsValidWebSocketKey要求24字元、最後兩字元==、前22字元皆base64 alphabet，
再Base64Decode成功且raw.size()==16。字串本身不接受OWS；checker先TrimOws後才送進它。
本body不自行檢key隨機性，也不以此查讀宣稱crypto callee或canonical padding細節全部驗證。
HttpRequest::Header會合併重複欄位；version／key合併後仍走上述完整字串比較與形狀條件。

checker不看path／Host／Origin／authentication／subprotocol／extensions／MachineType。
這些仍屬owner／其他caller；不能因checker存在就推定實際socket入口採用其全部條件。
選定WebBridgeServer.cpp遮罩後未見ParseHttpRequest／CheckWebSocketUpgrade／IsValidWebSocketKey的呼叫spellings。
其ProcessHttpHead／DoWebSocketUpgrade自行解析與分類，只能另按正文追；本輪未保存它們為完成函式。
這個有限定位不涵蓋全repo、間接呼叫、編譯巨集／binary link或browser／部署角色。
