# WsDecoder完整接收與frame解析

[UTF8 consumer](../index.md)；[原文／固定來源](source-manifest.json)。
來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；5完整CPP／250原文行／2來源／7原文頁。
2段原契約context不計新增；constructor、validator與class沿用前單元。

| 問題 | 入口 |
|---|---|
| Reset、first failure、Feed多frame與輸出責任 | [狀態及接收](state.md) |
| header、mask、大小、control與fragment派送 | [解析條件](frames.md) |
| 保存、來源、版本／機台及查證界線 | [證據](evidence.md) |
| 固定pin五完整body及2context | [manifest](source-manifest.json) |
| 本cpp／header有限詞法定位 | [census](symbol-census.json) |

完整TryOneFrame含既有三段UTF8context；此次只新增一個完整函式，沒有重計原context。
此單元仍屬hpi-uph-model現行C++依賴子樹；不增加canonical主題或宣稱完整網路／UPH結案。
