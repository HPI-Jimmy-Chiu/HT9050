# 版本、機台與驗證界線

固定 pin `42323e80ed44da6cb62b7482227364aae221e72c` 的 V906 `vclcompat/TStringList.cpp`／`.h`：[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)；[TStringList.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h)。

| 層級 | 共用部分／差異 | 本單元證據 |
| --- | --- | --- |
| V906 C++17 port | Ct* parser／writer、TStringList setter/getter、proxy、Load/Save、CommaTextCells | selected body／byte/blob/text hash 靜態查證 |
| HT9050 與其他 V906 機台 | 同一 library 可由不同 caller 使用；caller、工單、機型、runtime 字串／路徑與編碼可能不同 | 兩來源沒有 MachineTypeChoice／Type_HT9050 literal；不代表全 caller 共用證明 |
| HT9045／HT9046A／HT9046LS 客戶版本 | 需先分實際版本、客戶與輸入來源 | 未新查 caller／customer matrix，不將歷史量測套到所有機台 |
| V912／V899 BCB6 Big5 | 原始 RTL／MBCS CharNext 與本 port UTF-8 byte walk 有界線 | 只保存來源歷史 BCB 註解，沒有讀最新完整 BCB RTL 或新跑 oracle |
| runtime／現場 | 每台設定與工單不同 | 未操作、同步、build、fixture、啟動或實機驗證 |

有效 ASCII 逗號／雙引號與 UTF-8 多 byte 項內資料可依本次 helper 分支閱讀；這不能涵蓋非 ASCII delimiter、QuoteChar=0、原始 Big5 trail byte、embedded NUL、overflow／配置失敗、malformed quote、locale／ABI／容量／並行或窄路徑編碼。未關引號少最後 byte 的行為可能破壞 UTF-8。

原文保存在 [manifest](source-manifest.json)：2來源 byte/blob pin、22 cpp、4 inline、2 class、1 struct、4註解／context，33摘錄；重疊 context 保留 helper 前限制與歷史資訊，不能加算新函式或新 machine gate。活文件以 function／變數定位。歷史 oracle 名稱、成績、原行號不變，不宣稱本次實測。

延續 [核心界線](../core/limits.md)、[caller 樹](../../index.md) 與原 hpi-uph-model metadata／裁決／資源／相容入口。本單元沒有新增 canonical Skill，未改原始碼或執行期設定。

下一步：讀 AnsiString 深層 comparison／char*／locale／ABI，續查 INI／GPIB／UI consumer 的 caller 閉環；以最新 main 核對相鄰變更，客戶機型與 UPH／S8 實機仍需另行授權。批次約20:24 Ready MR target main，交 Jimmy／筆電整合；本角色不 auto merge／合 main，通知由 ST02-M 在核 main 後處理。

回 [入口](index.md)。
