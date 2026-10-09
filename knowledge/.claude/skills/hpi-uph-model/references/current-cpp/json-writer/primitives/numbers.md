# JsonNumber整數、double與locale界線

[上層](index.md)；[int64原文](raw/source-07.md)；[double原文](raw/source-08.md)。

`JsonNumber(wb_int64 v)`不使用printf格式，配置tmp[32]，由尾端pos=31開始產生decimal。
v<0用static_cast<unsigned long long>(-(v+1))+1ull求mag並記neg；其他直接轉unsigned。
mag==0寫一個0；之後逐次mag%10與mag/=10，負號最後補，回傳從tmp[pos]開始的字串。
這個寫法避開直接對最小負值取負；此處是正文分析，不是本輪CRT／編譯器邊界測試。

`JsonNumber(double v)`先用v!=v判NaN、std::isinf判正負Infinity，這兩條回"null"。
原20260911註解記錄x87 excess precision與兩個MinGW工具鏈測量、probe差異及舊threshold原因；
整段原文保留為歷史測量，未在本輪重跑，也不推定其他編譯器及版本已有同樣結果。

有限值迴圈prec=15..17，以snprintf("%.*g",prec,v)產生buf，再ForceJsonDecimalPoint修text；
只有strtod(text.c_str(),0)==v時回傳text。若三次都不相等，最後用"%.17g"再修decimal並回傳，
fallback沒有再驗strtod。正文沒有snprintf結果檢查、endptr解析完畢檢查或errno處理。
原註解「Shortest form」是原作者描述；本函式只嘗試三個precision，不能承諾全域最短表示。

[ForceJsonDecimalPoint](raw/source-06.md)呼叫localeconv；lc或decimal_point為0就返回，
sep=decimal_point[0]為'.'或NUL也返回，其他則把text每個等於sep的byte換成'.'。
它只使用locale小數符號的第一個byte，不處理完整多byte符號。
格式已改'.'，strtod仍使用當前C locale；本選定caller沒有另設C locale或locale專屬parser。
因此須分開讀「輸出修成JSON小數點」與「在當前locale用strtod比較」；
不由原policy的locale-independent文字推定所有locale、thread locale或fallback round trip已實測。

數值返回null與空字串的UI意義見[header policy](raw/source-09.md)，屬原契約文字。
原[TestJsonNumbers](raw/source-13.md)保留int64極值、非有限值、有限值與round trip assert。
測試正文沒有setlocale呼叫；assert與註解只代表測試來源，不代表本輪執行結果或所有locale覆蓋。
