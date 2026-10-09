# Encoder保存與適用範圍

[上層](index.md)；[manifest](source-manifest.json)；[census](symbol-census.json)；
[沿用header原契約](../decoder/raw/source-06.md)；[前單元證據](../decoder/evidence.md)。
pin `16f1ddb588497d250b9eaad227bc26dce9294ce4`；WsFrame.cpp九完整CPP／101函式行，相鄰原註解另外原樣保留。
WsFrame.h整檔byte作宣告權威；header／validator兩既有參照按manifest blob及payload hash核，新增完成0。
validator只用於契約比較，encoder正文沒有呼叫它；不能把比較參照當成實際callee。
57既有manifest按 `ebb664fc009190a0d2dd7af9f89fd12e04ab029b` 去重，九body未有完整保存；不重計decoder／primitive。

| 適用軸 | 限定範圍 |
|---|---|
| HT9050／其他Handler | 此pin V906共用WebBridge byte層；九body沒有MachineType／客戶分支，實際caller採用另核。 |
| V906／V912／V899／golden913 | 只查此V906；同名、客戶版本或不同機台部署不由此推定。 |
| 平台／runtime | 原header相容性註解保留；沒有build／link、讀機台ACP或改runtime／snapshot。 |
| caller／證據 | 本cpp與header九spellings遮罩後詞法定位；不稱完整network／browser caller graph。 |

活文件以版本／檔案、function／變數定位；offset只作固定pin保存metadata。
原作者RFC、測試或memory註解保留為歷史／契約聲明；本輪沒有執行程式／協定／機台測試。
通用frame限制、maskKey責任、UTF8 boundary非validity、status不檢查等依body分清。
!401主批通知已結不重寄；本批Ready由Jimmy／筆電整合，ST02-M核main後單次通知，ST-GPT不寄。
未啟停其他session、用Claude權杖或改共用checkout；W-195仍St02-E。
仍待network owner／browser完整caller、handshake、部署角色／ACP／版本客戶、
UPH容量校正／其他Handler、S8／846候選與legacy相容入口退役核對。
