# V906 AnsiString 數值、printf 與串接

識別 `STGPT-UPH-ANSISTRING-NUMERIC-FORMAT-20261008`；來源 `6ce3c86225d661c67efa3da11d562722dc58efc9`。沿同題UPH caller reference接續，不另建Skill鏡像。

| 路由 | function／變數定位 |
| --- | --- |
| [格式與串接](formatting.md) | assignUInt／assignDouble、numeric constructors／operator=、free operator+ |
| [解析](parsing.md) | vc_parseIntBCB6、ToInt／ToIntDef／ToDouble、Trim、end |
| [printf族](printf-family.md) | sprintf／printf／cat_printf、conv、formatString、needed／ap2 |
| [版本與界線](limits.md) | 歷史裁決、913 golden、機台與ABI／locale／caller待查 |

[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)；[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)。完整選定函式、模板與歷史裁決正文保存在 [manifest](source-manifest.json)。

保存7完整cpp、29選定header inline（含原template宣告）與3歷史裁決，共39原文。前單元已完成assignInt、Trim、AnsiString(int)、str與byte／比較不重算；完整header歷史metadata仍在 [bytes manifest](../bytes/source-manifest.json)。

回 [caller樹](../../index.md)、[位元組／比較](../bytes/index.md)、[INI conversion](../../ini-conversion/index.md)。
