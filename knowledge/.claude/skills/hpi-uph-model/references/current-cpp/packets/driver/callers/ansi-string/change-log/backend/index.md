# Change Log 現行記錄後端

識別 `STGPT-UPH-CHANGELOG-BACKEND-20261008`；V906 pin `98de4512b969bda046947413191dbb31d31aae31`。沿 UPH 字串依賴樹追查記錄呼叫的副作用；舊入口、metadata、裁決與 source 註解逐字保留。

| 問題 | function／變數定位與路由 |
| --- | --- |
| 表名、重複訊息與兩參數轉接 | [路由](routing.md)：RecordChangeLogProcess、MyDBIProcess、ExString |
| SQL 是否執行、如何回報 | [編譯與 SQL](sqlite-gates.md)：W906_CMYDB_SQLITE、MyDBExecSQL、ht9045_db |
| EventLogTxt、HANDLER／EventTracker 欄位 | [事件文字](event-rows.md)：SaveEventLog、SaveEventLogInfo、SaveEventTracker |
| 物件、日檔與啟閉時序 | [生命週期](lifecycle.md)：W906_CreateLogObjects、W906_DestroyLogObjects、ProductionLog |
| 版本／機型、未閉合的 callee | [界線](limits.md)：TMyStringList、upload／通訊、thread、913與runtime |

[cMyDB.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.cpp)、[LogObjects.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/LogObjects.cpp)、[aHotPlateSubstrate.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.cpp)。完整所選原文、介面、歷史裁決與 byte／body hash 見 [manifest](source-manifest.json)。

本輪保存14完整cpp、2完整interface header、12歷史／編譯／接線region，共28原文、11來源。歷史 banner 的「stand-in／尚無 writer／所有測試物件皆 null」是當年敘述；現行函式與編譯開關須以本文保存正文判讀。沒有執行C++、build、tests、檔案寫入或機台。

回 [Change Log數值／hook](../index.md)、[caller樹](../../../index.md)、[printf適配](../../numeric-format/printf-family.md)。

## 檔案型MyStringList下層補充（20261009）

[完整buffer／writer／filename與專用保存](my-string-list/index.md)：32完整cpp／1完整header／12region共45原文，4來源；前段「深層writer待續」保留為當時範圍，現在由此下層補充。GetText既有正文重核不重算，通訊／upload／helper／其餘caller及版本實機另續。
