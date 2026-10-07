---
name: hpi-uph-model
description: "Handler UPH 模型與機型差異：HT9045／9046 的歷史 Per-Tray、平行路徑與完整／精簡模型，以及 HT9050 Hot／Ambient 排程、MotionView／Concept 動畫與 LIVE 界線。分析 UPH、Soak、瓶頸、site／批次單位、MotorProfiler 與模型來源時使用。"
---

# Handler UPH 模型

同一主題整合 HT9050 與其他 Handler；先選機型、資料來源與模型，詳情按需讀 references。

## 先確認

- 分清實機 Per-Tray 結果、離線預測、概念動畫週期與 LIVE 快照；動畫播放或函式存在不代表實機已驗證。
- 秒數、HP／Tray 容量、site／每批顆數、良率及暫停扣除的口徑要寫清楚；示意預設值不能當機台量測值。
- HT9050 計算頁的事件排程與動畫序列不同；兩個 repo 的 Hot／Ambient 支援也不同，見機型分流。
- MotorProfiler、掃描／Home、安全門與 CSV 範例保留為歷史設計；先核對實際版本與當次授權，不能照舊稿直接啟動機台。
- 本次來源與靜態查證範圍見 [來源與驗證](references/resources.md)；[目前 C++ 局部證據](references/current-cpp/index.md) 與歷史／HTML分開，完整caller、容量與機台量測仍待補。

## 按問題選路

| 問題 | Reference |
|---|---|
| 單位、量測／預測／動畫的分界 | [共用讀法](references/common.md) |
| HT9050 與其他機台差異 | [機型樹](references/machines/index.md) |
| HT9050 Hot／Ambient、Soak、換盤與瓶頸 | [9050 排程模型](references/models/ht9050-scheduler.md) |
| 9050 概念動畫、零秒輸入與暖機 | [動畫模型](references/models/ht9050-animation.md) |
| 正式 MotionView／Runtime／layout 的適用界線 | [9050 執行期契約](references/runtime/ht9050.md) |
| HT9045 Per-Tray、完整／精簡模式 | [9045／9046 對照](references/machines/ht9045.md) |
| 客戶分支與計數口徑 | [客戶查證](references/customers.md) |
| 原公式、11 項時間、Profiler、歷史案例 | [原文樹](references/history/index.md) |
| V906／V912計算、計數、輸出與Profiler符號 | [目前 C++ 局部查證](references/current-cpp/index.md) |
| 來源版本、metadata、原稿、相容入口 | [來源與驗證](references/resources.md) |

## 查證與交付

以來源樹／檔名、function、關鍵變數定位，必要時加 Task／case；原文行號僅是歷史定位。
追資料輸入→模型與單位→計數／輸出，記錄未查證部分。文件整理不啟動機台、不改 runtime；
推前整合最新 main、重驗引用與原文保存，按本對話既有批次授權交付與通知。
