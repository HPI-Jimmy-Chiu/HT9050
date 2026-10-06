---
name: hpi-lotinfo-recipe
description: "HT9050與其他Handler的LotInfo／Recipe／QA共同入口。處理Lot Start／End、SetLotStart／Lot ID、Recipe與.Data／configByRecipe、下載／備份／覆寫、HTTP讀取／recipe.doc.put／C路擁有者、QA抽測與RunType／Untest Bin，以及工單Shuttle Mode／Cancel／Temperature Mode的機型、客戶與版本差異。"
---

# Handler LotInfo／Recipe／QA

同一份Skill按版本、機型、客戶及讀寫入口分層，HT9050與其他Handler共用資料觀念、差異獨立核對。

## 先確認

- 讀 [共同項](references/common.md)，分清Lot生命週期、Recipe切換、設定寫入、QA停測與機械START。
- Recipe文件可列出不等於可寫：先確認 [目前API／owner](references/runtime/recipe.md)，C路擁有者、notFound與編碼各自核對。
- Lot開始成功須看RunInfo.bLotStart與目前caller；[Lot reference](references/lot/index.md)保存完整流程與客戶gate，不能只看到函式就當全部入口已接通。
- [QA現況](references/runtime/qa.md)保留OFF_LINE停測守衛，切模式且CleanOut結束才還原；V904.4被撤回的原歷史不可當現在修法。
- HT9050工單的Shuttle Mode／Cancel／Temperature Mode先讀 [機型與快照](references/machines/index.md)，不套固定值到別台。
- 活文件依版本／檔名＋function／變數／Task；舊行號與測試結果保留原日期，不稱本次實機驗證。
- 最新整合見 [20261006 main更新](references/runtime/main-update-20261006.md)：LotInfo WA-5／WA-9／WD-3已開，視窗Show的no-op與客戶條件另分清。

## 按問題選路

| 問題 | Reference |
|---|---|
| Lot Start／End、Lot ID、SECS／OLP | [Lot生命週期](references/lot/index.md) |
| Recipe檔案、MD5、下載／備份／覆寫 | [Recipe檔案樹](references/recipe/index.md)／[目前API](references/runtime/recipe.md) |
| QA抽測、RunType、Bin與還原 | [QA完整原文](references/qa/index.md)／[目前QA](references/runtime/qa.md) |
| HT9050／其他機台工單與客戶差異 | [機型](references/machines/index.md)／[客戶](references/customers.md) |
| 舊主體與原引用 | [Lot](references/lot/original-entry.md)／[Recipe](references/recipe/original-entry.md)／[QA](references/qa/original-entry.md) |
