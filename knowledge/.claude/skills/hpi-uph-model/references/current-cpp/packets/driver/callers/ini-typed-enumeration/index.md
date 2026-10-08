# INI typed API 與磁碟列舉

接續 [INI 共用讀寫](../ini-core/index.md) 與 [Fast writer／memory store](../ini-writer-store/index.md)。V906 vclcompat/IniFiles.cpp pin `21d163f0d1b0f368b9066204a930969978231ad1`；[manifest](source-manifest.json) 保存 18 個完整 cpp body 與 16384 緩衝常數，共 19 份片段。九個 body 沿用 typed intake，另九個新選讀；未宣告全檔、全部型別轉換或 caller 已查完。

- [Typed 讀寫與預設](typed.md)：found、Float／Bool、日期與 WriteString 綁定。
- [磁碟名稱列舉](enumeration.md)：第一個 section、NUL 清單、重複與截斷、Windows DBCS 分支。
- [Exists 與 memory 差異](exists.md)：key-less／空 key／截斷清單、名稱比較與記憶體容器。
- [機型、版本與未查範圍](limits.md)：同題機型樹、來源註解、深層轉換與現場界線。

回 [caller 索引](../index.md)。本輪只讀碼與驗證文件、引用及保存內容；未執行 INI 寫入、C++、機台或 runtime。
