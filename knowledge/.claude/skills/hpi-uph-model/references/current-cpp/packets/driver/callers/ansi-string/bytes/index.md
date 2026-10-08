# V906 AnsiString byte、搜尋與比較

識別 `STGPT-UPH-ANSISTRING-BYTES-20261008`；來源 `6ce3c86225d661c67efa3da11d562722dc58efc9`。接續 TStrings 與 INI callee，仍為同一 hpi-uph-model reference。

| 路由 | function／變數定位 |
| --- | --- |
| [儲存與 char*](storage.md) | data_、constructor、c_str／Length／IsEmpty、operator[]／=／+= |
| [搜尋與編輯](search-edit.md) | Pos／AnsiPos／LastDelimiter、SubString／Delete／Insert／SetLength |
| [比較與 case copies](comparison-case.md) | operator ==／!=／ordering、UpperCase／LowerCase／TrimLeft |
| [版本與查證界線](limits.md) | 機台共用與差異、locale／ABI／容量、尚未完成 caller |

[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)；[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)。完整選定函式與歷史註解保存在 [manifest](source-manifest.json)。

保存9完整 cpp、35選定 header inline與1完整 header，共45摘錄。header 中已完成 Trim／assignInt 的相關宣告、歷史 metadata 與非本輪函式原樣保存；全 header snapshot 不等於新增全函式 census。

回 [caller 樹](../../index.md)、[TStrings 核心](../../string-list/core/index.md)、[分隔文字與 I/O](../../string-list/delimited-io/index.md)、[已完成 INI conversion](../../ini-conversion/index.md)。
