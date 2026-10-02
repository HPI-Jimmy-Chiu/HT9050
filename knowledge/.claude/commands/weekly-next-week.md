---
description: "建立下週週報：繼承本週資料、已完成項目轉黑、未完成保持紅字、日期推進到下週三，產出新 Excel。"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。這是破壞性動作。

請建立下週週報，操作 Weekly_AI 工作區：

1. 先讀 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\weekly_data.json` 的 `report_date`，算出下週三（出報日 20260713 起由週五改為週三，copy_next_week.py 走 next_wednesday()）。
2. **先向我說明**：將把哪些「已完成」項目轉黑、保留哪些未完成紅字、產生的新 Excel 檔名與日期。等我確認後再執行。
3. 確認後執行：
   ```
   cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools && python copy_next_week.py
   ```
4. 回報：新報告日期、轉黑筆數、保留紅字筆數、新 Excel 路徑（`output/`）。
