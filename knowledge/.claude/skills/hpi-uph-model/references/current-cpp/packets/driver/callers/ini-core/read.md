# 磁碟讀取、缺值與數字解析

定位 IniFiles.cpp 的 W906_DiskIniReadRaw → W906RdSlurp／W906RdGetString → W906RdLookup → W906RdFindSection、W906RdNextLine、W906RdHeader、W906RdKeyLine、W906RdSame、W906RdTrim。全部選讀原文見 [manifest](source-manifest.json)。

## 載入與 found

W906_IniProbe 在 Windows 依 GetFileAttributesA／GetLastError 回 0、ERROR_FILE_NOT_FOUND 的 1、ERROR_PATH_NOT_FOUND 的 2；其他錯誤及非 Windows 回 0。W906RdSlurp 在 probe !=0 回空字串。

Windows 使用 CreateFileA 的讀／寫／刪共享旗標；先按 GetFileSizeEx 做一次 sized read，再用 4096 byte buffer 讀餘量。非 Windows 用 fopen／fread。size 的 0x40000000 是選擇一次 sized read 的條件，不是整個檔案的容量保證；後面仍有 loop。未開成功時回空內容，wrapper 再以 found=false／預設處理；本層沒有獨立錯誤碼或一致快照鎖。

W906_DiskIniReadRaw 使用 kW906BcbReadStringBuf=2048。W906RdGetString 查到值則保留 found=true；找不到則取 def 並只刪除尾端字元 ' '。size==0 清值，否則長度超過 size-1 就裁切。磁碟 ReadString 的缺值預設也經過這條路，因此會裁掉尾端空格並最多留下 2047 bytes；memory ReadString 在缺值時直接回 def。

有 key 且值為空字串仍可 found=true：ReadString 回空值，ReadInteger 進 parseIntDef 後回數字預設。缺 key 的 ReadInteger 直接回 def；缺值、空值與讀檔失敗的 caller 提示不能由此推成相同 UI 行為。

## Section 與 key

- W906RdNextLine 識別 CR、LF、CRLF，W906RdSpan 記 pointer／長度。
- W906RdTrim 移除兩端 byte 0x01..0x20；它不等同「所有空白」或包括 NUL。
- W906RdHeader 接受 '[' 開頭；若沒有 ']' 仍以餘下內容做 name，若有則取其前方內容並 trim。
- W906RdFindSection 取第一個匹配 section；W906RdLookup 在下一個 header 停止，回第一個匹配 key。
- W906RdKeyLine 只在 trim 後首字元 ';' 時當註解，依第一個 '=' 分 key／value；未確認其他註解格式或行尾註解會被刪除。
- W906RdSame 只將 ASCII A..Z 轉小寫，高位元 byte 逐字比較；value 若有成對相同單／雙引號則去掉外層引號。

## ReadInteger

parseIntDef 先 trim byte <=0x20；依開頭 '$'、0x／0X 或其他字串選十六／十進位 strtol，無字元可解析或 end 指向非 NUL 時回 def，再 static_cast<int>(long)。完整 body 沒有 errno／int 範圍檢查；end 是 C-string 判斷，嵌入 NUL、溢位及跨平台 long 寬度尚未驗證。這不是完整型別／容量 ABI 或全部 BCB6 語意的證明。
