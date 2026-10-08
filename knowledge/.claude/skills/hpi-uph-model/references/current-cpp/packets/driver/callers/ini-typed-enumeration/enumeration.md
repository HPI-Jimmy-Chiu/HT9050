# 磁碟名稱列舉、NUL 與 byte 緩衝

定位 IniFiles.cpp 的 W906RdNames／W906RdSplitNames／W906RdBcbSection、W906_DiskIniReadSection／ReadSections、TIniFile::ReadSection／ReadSections 與 kW906BcbReadSectionBuf=16384；完整 body 與常數見 [manifest](source-manifest.json)。前置 Slurp、RdFindSection／NextLine／Header／KeyLine／Trim／Same 沿 [共用讀取](../ini-core/read.md)。

## 清單來源與名稱

W906RdNames 的 sec!=null 時，先找第一個匹配 section；找不到回空字串，只列到下一個 header。每條 KeyLine 解析成功就 append key bytes 與 NUL，沒有去重，也沒有排除空 key。sec==null 則掃全部 header，append name 與 NUL，包含重複或空名稱。

W906RdBcbSection 先由 section.c_str() 建 std::string，再 Slurp 當下檔案、產生名稱清單與 SplitNames。這個 C-string 建構與含嵌入 NUL 的 caller／名稱契約尚未查完，不將它等同 memory store 的所有字串路徑。

兩個 DiskIniRead 方法先 dest null guard、Clear，使用新的 vector 接收名稱，再逐項 Add(AnsiString(name))。public 方法依 writeThrough_ 選此路或 FillSectionKeys／FillSectionNames；本輪沒有執行 TStrings::Clear／Add 或檔案競寫測試。

## 截斷與清單停止

W906RdNames 在 list.size()+2<=size 時回完整 list；否則只取 size>=2?size-2:0 bytes。size 是 byte 緩衝，不是名稱數或檔案容量；16384 wrappers 的 cut 上限是 16382 bytes。回傳 std::string 保存內嵌 NUL，沒有在這個 body 額外 append 結尾 NUL。

SplitNames 從 p=0 開始，只在 p<raw.size() 且 raw[p]!=NUL 時繼續；找下一個 NUL，沒有就用 raw.size()，push substring 後移到 e+1。因此 cut 可以留下末尾部分名稱；遇到空名稱（含第一個空 key／空 section）就停止，後面的名稱不會被列出。不能把解析 raw、列舉輸出與 Exists 視為同一判準。

Windows cut 路徑用 IsDBCSLeadByte 判定 pair，且下一 byte 必須在原 list 中存在、非 NUL；若 cut 剛好只留下該 pair 的 lead byte，就將該 byte 改為 NUL。此分支沒有驗證 trail byte，非 Windows 沒有此修正。來源 ACP950「Measured」註解保留為歷史；本輪沒量當前 ACP、Big5／UTF-8 byte 邊界或實機結果。

Slurp 與列舉沒有一致快照鎖／獨立錯誤碼；讀檔空內容、無 section、key-less 或被空名稱終止都可能得到空 vector，要保留原因差異。
