# CommandQueue實作續篇

[回命令入口](../index.md)。V906 `WebBridge/CommandQueue.cpp`，配合既有[header契約](../raw/source-04.md)與[Sync全文](../../lifecycle/raw/source-08.md)。

| 問題 | 路徑 |
|---|---|
| tryPush／drain、界限與FIFOappend | [入列與取出](push-drain.md) |
| constructor／destructor／waitForPush與event | [生命期與等待](lifecycle-wait.md) |
| capacity／size／empty／peak／counter | [觀察值](observations.md) |
| 固定來源、metadata、原文與機型邊界 | [證據](evidence.md) |
| 十二function locator、原設計comment | [完整原文](raw/source-01.md)／[manifest](source-manifest.json) |

新增12完整out-of-line CPP函式／112函式原文行；完整214行CPP一頁保留，包含102行非上述body的comment／include／Impl等脈絡。
單頁context保存與12個function map共用原文，不複製成十二原文頁；Impl inline constructor、header與Sync不增加函式完成數。
函式存在、queue成功、event喚醒、宿主執行與browser回答分開核；本段僅靜態文件。
