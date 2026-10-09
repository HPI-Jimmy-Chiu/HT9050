# WebSocket encoder九完整函式

[UTF8 consumer](../index.md)；[decoder](../decoder/index.md)。
來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；9完整CPP／101函式原文行、9讀頁及相鄰原註解。
WsFrame.cpp是九函式來源；WsFrame.h是沿用契約／enum／預設值權威；兩檔byte固定。

| 問題 | 入口 |
|---|---|
| 三種長度、FIN／opcode／mask、文字／binary／ping／pong | [frame與wrapper](frames.md) |
| close code0、status／reason、UTF8截斷界線 | [close與截斷](close.md) |
| 保存、詞法查證、版本／客戶／runtime界線 | [證據](evidence.md) |
| 九完整body及既有header／validator參照 | [manifest](source-manifest.json) |
| 兩來源九spellings有限定位 | [census](symbol-census.json) |

通用encoder只產生byte；caller仍需決定合法opcode、方向、分片、UTF8與實際傳送。
此單元沿hpi-uph-model的現行依賴子樹，不增加canonical主題，也不宣稱network／UPH已結案。
