# test_common：讀斷言，不把原註解當本輪驗證

保存 `tests/test_common.cpp` 的A／B／C／L四個region，保留全部原註解。
這些是既有測試source；本輪沒有建立或執行測試、沒有建立scratch檔或改LastDataPath。

| region | 原斷言／使用 | 本輪可支持的範圍 |
| --- | --- | --- |
| A writer/reader | four buffers非NULL；strncmp固定8／17／28／5長度，if非NULL後free | 測試期待已知前綴與caller free，沒有做全buffer長度／尾部初始化驗證 |
| B missing reader | DeleteFile後ReadDataFromFile，CHECK(buf==NULL) | 程式寫了該期待，並非本輪實際測過缺檔 |
| C CheckFileIsEmpty | zero-byte期待false、有內容期待true、missing期待true | missing期待不能消除本體fclose(NULL)問題；原「VERIFIED」屬歷史註解 |
| L grid CSV | writer後reader；strncmp固定20前綴，free | 只保留既有使用與斷言，未重新驗證CSV或runtime路徑 |

A註解描述Windows文字模式CRLF與未知tail，故使用strncmp前綴。
這不等於LoadFile得到有效完整JSON，也沒有覆蓋本輪23個HTEditList CPP本體。
LoadFile清舊群組後失敗、成功root未Delete、group拒收與team欄位fallback、
allocator／短讀／seek失敗、CTR-Z／內嵌NUL、parser尾隨內容、重複load與thread仍未實測。

與 [條件式CRT契約](../crt.md) 一起讀；MinGW／BCB6／其他部署CRT的實際行為仍分開查。
