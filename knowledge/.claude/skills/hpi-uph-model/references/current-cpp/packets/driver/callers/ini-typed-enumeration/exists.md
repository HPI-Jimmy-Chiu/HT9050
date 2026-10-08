# Exists 的磁碟／memory 分流

定位 IniFiles.cpp 的 TIniFile::SectionExists／ValueExists、W906_DiskIniSectionExists／ValueExists；[manifest](source-manifest.json) 保存完整 body。磁碟名稱 parser 見 [列舉](enumeration.md)，memory 容器查找見 [store](../ini-writer-store/store.md)。

| 呼叫 | writeThrough_ 磁碟 | memory store |
|---|---|---|
| SectionExists | W906RdBcbSection 取得 16384-byte key 清單後，回 !names.empty() | findSection 命中即 true，無 key 也可 true |
| ValueExists | 在上述 names 逐項 W906RdSame(have,want)，命中 true | 第一個匹配 section 中查 KeyVal.key，命中 true |
| ReadSection | 當下檔案第一個 section 的 key 清單；受 NUL 停止及 byte 截斷 | 第一個 matching Section 全部 items，含重複 |
| ReadSections | 所有 header 清單；受 NUL 停止及 byte 截斷 | sections_ 全部名稱，含重複 |

磁碟 SectionExists 不是單看 header 存在：key-less section 回 false。解析出的第一個空 key 會讓 SplitNames 停止，影響後續 Exists；重複 section 只採第一個匹配的 key 列表，不合併後面的同名段落。

磁碟 ValueExists 將 ident.c_str() 建成 id，want 沒有 trim；W906RdSame 只折疊 ASCII A..Z、高位元 byte 原樣。名稱已在 KeyLine 做 trim，caller 識別字尾端空格不會在 want 自動消失。byte cut 可移除後面的 key，甚至留下部分 key；因此 Exists 回 false 不足以證明檔案內從未有該 key。

memory iequal 是對 <0x80 byte 呼叫 std::toupper，與磁碟的明寫 ASCII 算術分開；Load／WriteRaw、空 key、重複項目的差異沿 store。ReadRaw 的 found、typed 預設與 ValueExists 的列舉路徑不能交換使用，尤其空名稱、截斷及重複 section。

這張表是所選函式的靜態控制流，未執行 BCB6 differential、TStrings、磁碟並行、權限、容量或 runtime 測試。
