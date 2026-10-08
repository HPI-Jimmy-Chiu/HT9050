# 分隔文字輸出

來源：[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)。讀 `CtGetDelimited`、`CtQuoted`、`GetCommaText`、`GetDelimitedText` 與 helper `CtScan`／`CtMove`，完整正文與歷史 context 見 [manifest](source-manifest.json)。

## Getters 與欄位

`GetCommaText` 直接傳逗號／雙引號給 helper，不改 list 的 Delimiter／QuoteChar。`GetDelimitedText` 使用目前兩欄。`SetCommaText` 的永久改欄位效果見 [parser](parser.md)，不能套用 getter 的無變更結果。

`CtGetDelimited` 對單一空項特判成兩個 QuoteChar。一般項逐 byte 掃到 NUL、unsigned byte ≤空白、QuoteChar 或 Delimiter；若停在非 NUL 才呼叫 `CtQuoted`。每項加 delimiter，最後移除最後一 byte delimiter。

以下皆為逗號／雙引號、selected body 的靜態推導，沒有跑 oracle：

| 項值清單 | 輸出文字 | 原因 |
| --- | --- | --- |
| `[]` | 空字串 | 迴圈零次 |
| `[""]` | `""` | 單空項特判 |
| `["a","","b"]` | `a,,b` | 多項中的空項未觸發 quote |
| `["a b","c"]` | `"a b",c` | 空白觸發 quote |
| `["a,b"]` | `"a,b"` | delimiter 觸發 quote |
| 含雙引號的單項 `a"b` | `"a""b"` | quote 字元加倍 |

## NUL 與 quoting 分支

`CtQuoted` 的 quote count／copy 掃描都在第一個 NUL 截止。NUL 前沒有 QuoteChar 時直接包住 **完整 std::string**，內含 NUL 及後方內容也可能保留。NUL 前已有 QuoteChar 時，結果長度用完整 `S.size()` 計算，實際 copy 只到 NUL；未寫入的尾部在本 port 先填零。這兩個分支不可概括成「一律截 NUL」或完整 binary round-trip。

歷史註解說 RTL 在 quote+NUL 分支可能留下未初始化尾部，本 port 填零；此為保存的歷史差異，本單元沒有重跑 BCB。ASCII 合法文字例子不足以證明任意 QuoteChar／非 ASCII delimiter、NUL、malformed input 都相容。

回 [入口](index.md)、[代理](proxies.md)、[界線](limits.md)。
