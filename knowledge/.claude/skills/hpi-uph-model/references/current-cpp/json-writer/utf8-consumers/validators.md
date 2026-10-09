# 三個IsValidUtf8的身分與判定

[上層](index.md)；[primitive契約](../primitives/encoding.md)。

| 定義 | 選定consumer與判定 |
|---|---|
| `ela`內ElaHub.cpp的static IsValidUtf8 | [ToUtf8](raw/source-02.md)未加限定名，解析到[本地validator](raw/source-01.md)。 |
| `webbridge::WsDecoder::IsValidUtf8` | [完整body](raw/source-03.md)及class內TryOneFrame的3個選定分支；屬static member。 |
| `webbridge::IsValidUtf8` | JsonWriter共用函式；六個ANSI wrapper明確限定呼叫它，沿用primitive原文，不重計。 |

ElaHub本地validator以c的高bit判1／2／3個continuation，兩byte要求c>=C2，四byte要求c<=F4。
它檢查輸入長度與continuation格式，沒有組出code point，也沒有三／四byte overlong、
surrogate或最大Unicode scalar的檢查。空字串回true，ASCII NUL也是ASCII。
依條件靜態推導，E0 80 80、ED A0 80、F4 90 80 80可通過此本地body；
這是正文推導的反例，未在本輪執行程式或將現象歸因於機台。

WsDecoder member把首bytebit與continuation組成cp；依extra拒絕低於80／800／10000的overlong，
也拒絕D800..DFFF與大於10FFFF，並先核i+extra<n。它對整個std::string檢查，不是串流逐chunk驗證。
以上三種反例會被這個body的cp條件拒絕，與JsonWriter primitive的scalar檢查方向相符；
兩份獨立body仍各自保留身分，不把詞法同名命中當成呼叫共用函式。

namespace opener與enclosing brace固定pin證據見manifest；WebLogin U8Text位於noteauth區域。
字串修復只由選定ANSI wrapper的特定失敗分支進SanitizeToUtf8；WsDecoder分支選擇拒絕，
ElaHub有效判定則直接原樣傳回。不能因同名函式就承諾所有入口會做同一份修復。
