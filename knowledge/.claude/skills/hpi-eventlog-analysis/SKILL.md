---
name: hpi-eventlog-analysis
description: "Handler Event Log Analyzer共同入口，整合HT9050與其他Handler的BCB6外部分析器、V906 ElaCore／ElaHub／Web、EventLogTxt、MTBF／MTBA／OEE、報表／排程／FTP及W44指定時間與補發裁決；按版本、客戶碼與runtime分流，保留舊Skill與原計畫。"
---

# Handler Event Log分析

同題整理HT9050與其他Handler；先分單檔檢視、多日統計、報表／排程及執行期事件串流。

## 先確認

- [共同介面與資料](references/common.md)：SendCommand_EventLog、EventLogTxt與GetEventLogText的不同用途。
- [版本](references/versions/index.md)、[機型](references/machines/index.md)及[客戶](references/customers.md)：機型不代替來源版本、客戶碼或功能開關。
- 活定位用版本／檔名＋function／變數；保存原文中的程式行號只作歷史定位。

## 按問題選路

| 問題 | Reference |
|---|---|
| BCB6外部分析器／V906 worker、hook與Web | [版本樹](references/versions/index.md)與[局部來源查證](references/runtime/source-review.md) |
| CSV切欄、單檔／多日與統計範圍 | [資料與分析路由](references/analysis/index.md) |
| O06／O19／N10／N25／N34、排程與FTP | [報表樹](references/reports/index.md) |
| W44／W44-1／W44-2、指定時間與開機補發 | [裁決與實作界線](references/reports/w44.md)，保留原文及更正順序 |
| 原版本、作者、量測、待測項及完整計畫 | [保存原文](references/history/index.md) |
| State Record、hang及來源CSV產生者 | [State分析](../hpi-state-analysis/SKILL.md)與[MyDB](../ht9045-mydb/SKILL.md) |

這次整理只改文件與相容入口；不啟動分析器、機台、API、報表排程或FTP，不修改runtime。歷史「已做／待測」與本次靜態查證分開，未查機型／客戶／caller保持未查。
