# RS232Standard獨立writer：版本、覆寫與回報契約

本單元定位V906 `TesterComm/Rs232/Rs232Globals.cpp` 的
`rs232std::WriteDataToFile(char* cFilePath, char* cData, int iSize)`。
1完整CPP與6選定region共7新原文／5來源；原始byte／body／guard與歷史註解見
[manifest](source-manifest.json)，既有reader與writer單元不重算。

| 查什麼 | 入口 |
| --- | --- |
| desired access／share mode、覆寫、長度、失敗回報 | [writer契約](writer.md) |
| RS232Standard與GPIB aux、golden／Handler版本、CMake與caller搜尋界線 | [證據與適用範圍](evidence.md) |
| 共用reader及其他writer平台比較 | [IO入口](../index.md) |
| HT9050與其他Handler、UPH與模型版本 | [目前C++入口](../../../../../../../../../../index.md) |

只完成選定寫檔本體與其靜態上下文。完整RS232 caller／upload、
實際link／ABI／IO、其他版本與機台仍待查；未執行build、tests或runtime。
