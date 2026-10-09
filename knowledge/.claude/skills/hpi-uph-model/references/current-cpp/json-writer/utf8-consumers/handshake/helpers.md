# ASCII、URL、header與query helpers

[上層](index.md)；[原header契約](raw/source-28.md)。

| 完整原文群 | 條件與責任 |
|---|---|
| [LowerCh](raw/source-01.md)、[ToLowerAscii](raw/source-02.md)、[IsOws](raw/source-03.md)、[IsTChar](raw/source-04.md)、[IsToken](raw/source-05.md) | LowerCh只轉A..Z；IsOws只有SP／HTAB。IsTChar按unsigned byte驗明列ASCII集合，IsToken拒空。 |
| [TrimOws](raw/source-06.md)、[EqualsIgnoreCase](raw/source-07.md)、[ListContainsToken](raw/source-08.md) | TrimOws只去兩端SP／HTAB；EqualsIgnoreCase先比長度；ListContainsToken逐comma拆、trim、忽略ASCII大小寫比完整element。 |
| [HexVal](raw/source-09.md)、[UrlDecode](raw/source-10.md)、[PathIsSuspicious](raw/source-11.md) | HexVal限0..9/A..F；UrlDecode合法%HH轉byte，malformed %保留；plusAsSpace才轉+。PathIsSuspicious只做明列path檢查。 |
| [HttpRequest::Clear](raw/source-12.md)、[HttpRequest::HasHeader](raw/source-13.md)、[HttpRequest::Header](raw/source-14.md) | Clear清所有欄位；查詢名字先小寫，vector內名字以相等比較；Header按原次序將重複值用comma+SP接起。 |
| [FindQueryRaw](raw/source-15.md)、[HttpRequest::QueryParam](raw/source-16.md)、[HttpRequest::HasQueryParam](raw/source-17.md) | FindQueryRaw逐&拆，key用UrlDecode(...,true)比name，第一次命中即回；QueryParam命中後解value，HasQueryParam只問是否存在。 |

HttpRequest::Header／HasHeader假定headers內名字已由parser轉小寫；手動填vector者也須守這個契約。
Header的空字串可能是不存在或存在但空；需要分辨時用HasHeader。
FindQueryRaw以std::string比較decoded key，大小寫敏感；重複query key第一個有效命中勝出。
沒=與有=但空值均可存在；QueryParam缺key用def，存在空值回空。plus空白轉換適用key和值。
這些helper不自行移除fragment；ParseHttpRequest在組query時另做，手動設query另負責。

UrlDecode不驗UTF8、不拒NUL或其他decoded byte，也不遞迴解碼；malformed escape保留原byte。
PathIsSuspicious拒空／非斜線開頭、NUL、backslash以及恰等..的path segment；
不是完整canonicalisation／Windows filename／symlink／權限判定，header原註解也明分必要與充分。
只按body記條件；本輪沒有URL、HTTP、filesystem或網路執行測試。
