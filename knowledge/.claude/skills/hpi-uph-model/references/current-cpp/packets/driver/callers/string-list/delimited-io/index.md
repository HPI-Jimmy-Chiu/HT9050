# V906 分隔文字、代理與檔案 I/O

範圍識別：`STGPT-UPH-TSTRINGS-DELIMITED-IO-20261008`。Source pin：`42323e80ed44da6cb62b7482227364aae221e72c`；只讀 selected definitions，不代表完整 caller 或機台驗證。

| 路由 | 定位與用途 |
| --- | --- |
| [Parser](parser.md) | CtSetDelimited／CtExtractQuoted；空白、尾分隔符、引號、NUL 與 byte offset |
| [輸出](rendering.md) | CtGetDelimited／CtQuoted；單空項、多空項及 quote 分支 |
| [代理](proxies.md) | CommaTextProxy／DelimitedTextProxy；序列化後解析與目的端參數 |
| [檔案 I/O](file-io.md) | LoadFromFile／SaveToFile；清空時機、CRLF 與失敗回傳 |
| [版本與驗證界線](limits.md) | V906 共用部分、HT9050／其他機台差異與尚未查證範圍 |

[原文 manifest](source-manifest.json) 保存兩個完整來源檔的 byte/blob hash、22 完整 cpp 定義、4 inline、2 class、1 struct 與4段原始註解／context，共33摘錄。重疊的 context 用來保留 helper 前的歷史限制註解，並非多出新函式。

[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)；[TStringList.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h)。活文件依 function／變數定位；歷史註解中的行號與 oracle 成績原樣保存，沒有在本單元重新執行。

回 [字串清單樹](../index.md)、[核心與 Text](../core/index.md)、[caller 樹](../../index.md)。同一 hpi-uph-model reference，SKILL.md／metadata／相容入口沿用。
