# INI Fast writer 與 memory store

接續 [INI 共用讀寫](../ini-core/index.md) 的未查 callee。來源為 V906 vclcompat/IniFiles.cpp，pin `21aa8a460c6d590c3231551531be8dc4af50c60a`；[manifest](source-manifest.json) 保存 25 個完整 cpp 定義與 W906FsLine，共 26 份片段及 hash。六個定義接續既讀 intake，其餘十九個是本單元新選讀；未宣告整個 cpp 或完整 caller 已查完。

- [Fast 修改與 span](fast.md)：第一個 section／key、插入位置、尾端空白與換行。
- [Memory 載入、查找與保存](store.md)：sections_／items、重複名稱、found、字元與格式差異。
- [TMem 生命期與失敗界線](lifetime.md)：constructor、destructor、UpdateFile／flush、TouchSame。
- [機型、版本與未查範圍](limits.md)：HT9050／其他 Handler 同題分流，歷史與本輪結果分開。

回 [caller 索引](../index.md)、[Aux 表單映射](../form-settings/mapping.md)。本輪只讀碼與核對文件，未呼叫 INI 保存、C++ 測試或機台程式。
