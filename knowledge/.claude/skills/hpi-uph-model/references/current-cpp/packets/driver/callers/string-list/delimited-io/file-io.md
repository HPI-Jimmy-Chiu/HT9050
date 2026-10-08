# TStringList 檔案 I/O

來源：[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)。定位 `LoadFromFile`、`SaveToFile`，搭配前單元 [Text 分行](../core/text.md)。[manifest](source-manifest.json) 保存這兩個完整 body；本單元沒有呼叫它們、建立 runtime 檔或開機台。

## LoadFromFile

以 `path.c_str()` 建 binary ifstream，接著清掉 items_／objects_，**再**檢查 stream。開檔失敗時 `syncCount()` 後返回，因此舊清單不保留；正常路徑把 rdbuf 讀入 ostringstream，再交 `SetText` 分行，基底項目 Objects 為 null。

selected body 沒有明示 BOM stripping／編碼轉換、讀取錯誤後的專用判斷或例外回報。narrow path、AnsiString 路徑編碼、OS 權限、部分讀取與失敗例外的實際效果需環境／caller 證據，不能從 binary mode 推得 UTF-8／Big5 自動判斷或無損匯入。

## SaveToFile

以 `path.c_str()` 建 binary ofstream，stream 初始失敗直接返回；記憶體項目未修改。正常路徑逐項寫 items_[i].str() 與 CRLF，包括最後一行；沒有寫 Objects／Delimiter／QuoteChar。空清單迴圈零次。

這個 ofstream constructor 的輸出模式沿用標準預設 truncation，正常開啟既有檔會截斷；空清單因此可產生空檔。這是 source 加標準 stream 語意的靜態推導，未做檔案實測。selected body 沒有 temp file＋rename、fsync 或逐次寫入失敗檢查，不能稱 atomic／durable delivery。

| 情況 | 清單 | 可從正文確定的回報 |
| --- | --- | --- |
| Load 開檔初始失敗 | 已清空、Count 同步 | void 返回，無明示錯誤回報 |
| Load 正常讀取 | SetText 重建 | CR／LF 分行依前單元正文 |
| Save 開檔初始失敗 | 保留記憶體內容 | void 返回，無明示錯誤回報 |
| Save 正常寫入 | 逐字串＋CRLF | 沒有另驗最後 stream 狀態 |

呼叫者是否備份、先驗 input、使用 CommaTextCells 改特定欄位，以及機台 snapshot／runtime 位置的寫入政策，仍需各 caller 與現場證據；本文件不授權執行 runtime 同步或存檔。

回 [入口](index.md)、[界線](limits.md)。
