# Close status與UTF8邊界截斷

[上層](index.md)；[TruncateUtf8完整正文](raw/source-08.md)；[EncodeClose完整正文](raw/source-09.md)。

EncodeClose的code是uint16_t；code=0時p保持空字串，reason被忽略，再產生unmasked FIN close frame。
code非0時先加入兩byte big-endian status，接TruncateUtf8(reason,125-2)，所以reason最多123 byte。
body沒有檢查status是否合法；caller給1005／1006等值也會被寫入wire payload，
而選定decoder的legal條件會拒絕；這是固定正文比較，不是本輪已測出的peer或機台現象。
傳送、close回應與socket關閉仍屬network owner，encoder只回std::string。

TruncateUtf8先看s.size()<=limit，符合時整串原樣返回，沒有掃描或驗證UTF8。
超限時cut=limit，檢查的是s[cut]，即第一個將被排除的byte；
若該byte為continuation（80..BF），便向前退到非continuation或0，再回substr(0,cut)。
由s.size()>limit可知初次s[cut]位於字串內；cut>0才會進while，limit=0直接回空字串。
在輸入已是有效UTF8的前提下，此條件避開切到code point中途；
不驗leading byte、continuation數量、overlong、surrogate或最大scalar。

短invalid字串會走整串返回；長invalid字串也可能保留invalid prefix。
靜態反例：C0 AF的兩byte字串若limit=123，直接返回這兩byte，沒有拒絕或repair。
這個推導由size分支即可得出，未執行程式測試。
與[WsDecoder member有效性檢查](../raw/source-03.md)不同，encoder沒有呼叫該validator或SanitizeToUtf8。
EncodeClose的reason使用std::string size／substr／append，內部NUL仍在byte範圍；
實際caller是否先用C字串截斷、從ANSI轉碼、檢驗內容，需另查。

Ping／Pong對payload截125是raw byte resize；control payload本身不被此body視為UTF8文字。
close原註解提到UTF8／RFC與對端1007屬原契約，本輪僅保存與核對條件，未宣稱RFC完整認證。
