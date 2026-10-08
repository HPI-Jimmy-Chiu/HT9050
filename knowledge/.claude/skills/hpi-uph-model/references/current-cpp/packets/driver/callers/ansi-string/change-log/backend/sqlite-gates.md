# SQL 組字、預設開關與回傳界線

[cMyDB.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.cpp)、[LogObjects.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/LogObjects.cpp)、[aHotPlateSubstrate.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.cpp)。完整所選原文、介面、歷史裁決與 byte／body hash 見 [manifest](source-manifest.json)。

cMyDB.cpp 的 W906_CMYDB_SQLITE 在未外部定義時為0。保留的20260926裁決註解說SQLite退役、CSV-only；include sqlite3與所選MyDBExecSQL實作分支都受此macro控制。MyDBExecSQL在0分支只(void)str、return0。上層仍組INSERT並接續文字記錄，不能從函式名或banner稱預設production仍寫handler.db。

| 編譯來源 | source證據與界線 |
| --- | --- |
| [CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt) 的ht9045_db | 同target包含cMyDB.cpp、LogObjects.cpp；保留sqlite3 link edge。link到library不代表W906_CMYDB_SQLITE已打開 |
| [tests/CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt) 的test_ga1_cmydb | 自編cMyDB.cpp／myTimer／AlarmCodeCatalog／AlarmCodeUpdater、link vclcompat sqlite3，PRIVATE定義W906_CMYDB_SQLITE=1。這個oracle設定不代表production或每個測試的設定 |
| 其他build／caller | 未作完整compile-definition census、未建置／執行。本輪只核上述兩個target及source預設 |

macro1的MyDBExecSQL先在CosFunction.bUseMDB==false時return0，否則sqlite3_exec(dbReadWrite,str.c_str(),...)。ret!=SQLITE_OK時free errMsg；dbReadWrite非null才讀sqlite3_last_insert_rowid，最後回傳int rowID。回傳不是sqlite錯誤碼，也沒有把insert成功與舊last rowID分開；兩個Process caller忽略回傳值。null connection、rowID型別範圍、實際failure結果未執行測量。

MyDBIProcess／MyDBIProcessNew以sprintf將asTable、S1、S2及AlarmCode直接插進SQL字串，所選正文沒有escape或parameter bind。%s的AnsiString適配沿 [既有printf backend](../../numeric-format/printf-family.md)，不等於SQL引號或欄位安全檢查。SQLite已gate不表示其他文字路徑沒有寫入。

cpublic.cpp 的ProductionLog reference copy與RespondASECom舊正文位於保存的#if0區；cmydef.cpp的SaveEventLog reference copy也仍gate。現行所選SaveEventLog／ProductionLog完整body在LogObjects.cpp；保留舊body與歷史搬回理由，不以舊gate推論現行body不存在。RespondASECom今日callee則留後續追查。

回 [路由](routing.md)、[儲存與欄位](event-rows.md)、[生命週期](lifecycle.md)。
