# Memory store 的載入、查找與保存

定位 IniFiles.cpp 的 TIniStore::LoadFromFile／SaveToFile／iequal／findSection 兩個 overload／ReadRaw／WriteRaw／SectionExists／ValueExists／FillSectionKeys／FillSectionNames；關鍵容器 sections_、Section.items、KeyVal.key／val。完整原文見 [manifest](source-manifest.json)，宣告沿用 [INI 共用層完整 header](../ini-core/source-manifest.json)。

## 載入與比較

LoadFromFile 先清空 sections_；rb 開啟失敗直接返回空 store。fread 以 8192 byte chunks 累積，未檢查 ferror／fclose，不能把載入返回說成完整讀檔成功。

行尾識別 CR／LF／CRLF。結構左 trim 與 section／key 的兩端 trim 使用 unsigned byte<=0x20，與 Fast 的空格／tab 不同；';' 或 '#' 開頭行跳過。Header 用 rfind(']') 取最後一個 ']'，缺 ']' 時取剩餘字串；每個 header 都 append 新 Section，沒有合併重複 section。

Key 依第一個 '=' 分割；header 前的 key 與 trim 後空 key 跳過；value 保留 '=' 後所有 byte，不 trim、去引號或刪行尾註解。每個 key 都 append，沒有載入去重。

iequal 先比較 byte 長度，再對 <0x80 的 byte 呼叫 std::toupper，>=0x80 原樣比較；這裡不是 Fast 的明寫 A..Z 算術折疊。Locale／多位元編碼的跨平台一致性與完整 BCB6 保真度未在本輪執行驗證。

## 重複、缺值與寫入

findSection 兩個 overload 都回第一個匹配 section。ReadRaw 先設 found=false，再取第一個匹配 key，匹配即 found=true 並回保存 value；空 value 與不存在可由 found 區分，caller 是否使用此區分仍依 [讀取入口](../ini-core/read.md)。

WriteRaw 取第一個 section；沒有就 append Section，name 用傳入原字串。第一個匹配 key 只改 val，保留 key spelling；沒有 key 就 append KeyVal。ReadRaw／WriteRaw 沒有 trim 呼叫參數；含空白名稱或空 key 的寫入與重新 LoadFromFile 可能不同，不宣稱往返保真。

SectionExists／ValueExists 使用相同第一個 section 與 key 比較；FillSectionKeys 在 dest!=null 時先 Clear，再列第一個 section 的全部 items（含重複）；FillSectionNames 列全部 sections_（含重複）。本輪沒有驗證 TStrings 實作或磁碟列舉 wrapper／public ReadSection 的完整鏈。

## 保存格式與失敗

SaveToFile 開 wb，失敗直接返回；逐 Section 寫 '[name]\r\n'、每個 'key=value\r\n'。blankLineAfterSection=true 時每個 section（包括最後一個）後再寫空行。註解、原縮排、原始行尾等沒有被 store 表示保存，不能套用 Fast 的局部修改說法。

fputc／fwrite／fputs／fclose 回傳值沒有檢查，也沒有回讀、交易或 concurrent writer 鎖。WriteRaw 更新記憶體與 SaveToFile 成功寫入磁碟是兩個狀態。
