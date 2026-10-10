# Base64 alphabet、padding與輸出保留

[上層](index.md)；[完整CPP](raw/source-03.md)／[完整header與原strictness說明](raw/source-04.md)。

## DecodeChar與Encode

[DecodeChar](raw/source-03.md#decodechar)把A-Z／a-z／0-9／+／/映成0..63，=為kPad；space、tab、CR、LF、form feed、vertical tab為kWhitespace，其餘kInvalid。
這是標準+/ alphabet；不接受URL-safe的-與_，也不把任意Unicode空白當可跳過字元。
[byte Encode](raw/source-03.md#encode-bytes)data==0或len==0返回空字串；每3-byte拼24-bit，輸出4 alphabet字元，尾1 byte補==、尾2 bytes補=。
[string Encode](raw/source-03.md#encode-string)空值返回空，否則以data.data()/size()轉交；NUL與非UTF-8輸入按byte處理。
reserve式與實際長度／配置上限分開；沒有本輪極長輸入、配置失敗或效能測試。Base64文字編碼不等於加密或machine key驗證。

## Base64Decode

[完整body](raw/source-03.md#decode)先拒絕out==0；第一遍讀全部text，跳過六種ASCII whitespace，拒絕alphabet外字元，再收clean。
clean空時清空*out並true；不是空輸入保留先前out。非空clean長度須為4倍數，末尾至多2個=，其他位置的=會false。
逐4字元轉q[4]，最後quantum的pad譯為0 bits，以padCount決定輸出1／2／3 byte，全部暫存在local decoded。
一般false路徑在最後out->swap之前返回，所以caller原out保留；這不宣稱std::string配置例外也會轉為false。
success以swap取代原out，內容可以有NUL及非UTF-8；不是UTF-8有效性檢查，與JsonWriter文字處理責任分開。

body沒有檢查padding所對應的未使用低位是否全0，也沒有decode→encode比回原字串；不要由header「strictness」推為所有canonical encoding都已核。
任意base64可解碼與WebSocket key有效不同；key caller還有24字元、22 alphabet字元＋==及16 raw bytes檢查，見[caller](callers.md)。
以上按實際分支靜態讀取，原RFC 4648 section4與header理由全保留；本輪未執行algorithm、fuzz、browser或機台。
