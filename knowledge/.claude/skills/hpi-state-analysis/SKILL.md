---
name: hpi-state-analysis
description: "Handler State Record卡住／timeout分析；讀Task_ListWithTime.csv、MainProcMonitor、LastEnter／SaveTime／SilentSec／CallCount與事件／runtime快照，區分正常等待、疑似阻塞及主迴圈停擺。HT9050與其他Handler同題，先確認版本／機型／客戶與實際記錄格式。"
---

# Handler State Record分析

同一入口整合HT9050與其他Handler的證據讀取、差異及歷史案例；先讀已提供的State Record，文件整理不產生機台記錄。

## 先確認

- [共同分析](references/common.md)：核對版本／runtime快照與三個時間，再作正常／可疑／停擺判斷。
- [機型與版本](references/machines/index.md)：BCB6／V906格式與附加診斷分開；缺欄不補猜數值。
- [客戶與工單](references/customers.md)：CUSTOMER_CODE、開關、recipe與歷史案例各自查，不用客戶名判根因。
- 活定位用版本／檔名、function、Task／case及關鍵變數；原文行號只作歷史定位。

## 按問題選路

| 問題 | Reference |
|---|---|
| CSV欄位、記錄包、Task字典、決策變數與互鎖 | [格式與資料](references/formats/index.md) |
| LastEnter／SaveTime落差、MainProc健康、deadlock／timeout | [診斷流程](references/diagnosis/index.md) |
| 快速結論、證據、前三個假設與最短驗證 | [輸出模板](references/output/index.md) |
| OCR／Barcode 1150、舊版MultiZone與其他歷史案例 | [案例](references/cases/index.md)，按原版本讀 |
| 現行V906 writer／診斷hook與查證界線 | [來源核對](references/runtime/current-source.md) |

原State Record與data-analysis的完整原文、metadata、同事材料及舊引用路徑保留。文件／靜態source核對不代表本輪實機或卡住案例驗證。
