# MyForceDirectories：路徑分類與回傳

[上層](index.md)；定位 `common.cpp` 的Handler全域 `MyForceDirectories(AnsiString Directory, AnsiString Function)`，宣告Function預設空字串。不是RS232或EventLogAnalysis的同名版本。

## 分類順序

| 條件 | 本體行為 |
| --- | --- |
| 初始Directory空 | RecordProcess記錄空目錄，然後return -1。 |
| 最後字元是反斜線 | 略過GetFileAttributes／副檔名分類，直接進目錄階段。 |
| 最後字元不是反斜線、GetFileAttributes有效 | 只有屬性不含FILE_ATTRIBUTE_DIRECTORY才ExtractFilePath去掉檔名。 |
| 同上但屬性無效 | ExtractFileExt(...).LowerCase；僅.txt或.csv去掉檔名，其他字尾仍當作待建目錄。 |
| 分類後Directory空 | 再RecordProcess，然後return -1。 |
| 非空 | try內DirectoryExists=false才呼叫ForceDirectories；正常走到底return 1。 |
| try內拋例外 | catch(...)組Directory／Function訊息，ShowMyMessage後return -3。 |

例如不存在的單一basename.csv經ExtractFilePath可能變空；不存在的目錄名稱若恰以.csv結尾，也會被副檔名規則當成檔名。這些是分支推論，未建立或探測實際檔案。開頭沒有URI、網路路徑、相對路徑或磁碟根的完整分類；末尾 `/` 不等於此處的反斜線捷徑。

## `1`沒有證明建立成功

MyForceDirectories丟棄ForceDirectories的bool回傳。底層正常return false時，外層仍可return 1；上層MyStringList又未核這個int。不能把「取得filename」「helper回傳1」或「沒有例外」記成目錄／輸出成功。

RecordProcess在try外，catch內的ShowMyMessage也沒有第二層catch；上述負碼是在相應紀錄／訊息呼叫正常返回後到達的return，不保證任何外部失敗都轉成負碼。

原20260721 banner的golden例外分流差異（-2／e.Message與-3固定訊息合併）完整保存；其「當時兩callers丟棄回傳所以無害」是歷史評估，不能代表所有現在callers或最新913。

沒有修改目錄策略、加錯誤檢查或重命名機台檔案；底層契約見 [filesystem](filesystem.md)。
