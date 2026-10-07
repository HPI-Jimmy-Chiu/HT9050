> 保存來源：`.claude/skills/data-analysis/references/colleague-analysis-sop.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# State Record 分析 SOP

> 從 SKILL.md 分離出的操作範例與快速參考。

## 快速判讀流程

```
讀取 Task_ListWithTime.csv
  ↓
找到 MainProcMonitor 行 → 提取 LastEnter / SaveTime / SilentSec / Alive / CallCount
  ↓
計算 Gap = SaveTime - LastEnter
  ↓
讀取 Task_ListWithTime2.csv 尾端 → 找到最後一次 task 更新時間
  ↓
┌─ Gap < 10s 且 task 近期有更新 → ✅ Normal
├─ Gap 10~60s 且 task 停止時間早於 LastEnter → ⚠️ Suspicious
└─ Gap > 60s 且 task 與 LastEnter 同時停止 → 🔴 Likely Hung
```

## 欄位定義

| 欄位 | 說明 |
|------|------|
| `Alive` | N = SilentSec 超過閾值（通常 5 秒）；**不能單獨作為 hang 依據** |
| `CallCount` | MainProc 累計呼叫次數；單次快照無意義，需跨多次比較 |
| `LastEnter` | MainProc 最後進入時間 |
| `SilentSec` | 從 LastEnter 到 SaveTime 的秒數 |
| `SaveTime` | State Record 寫入時間（手動觸發時 = 人工擷取時間） |

## 範例報告格式

```
- MainProc last enter: 2026/04/02 17:39:27.318
- State record save time: 2026/04/02 17:39:32.806
- Gap: 5.5 sec
- Last normal task activity: 2026/04/02 17:39:16.125
- Interpretation: normal
- Reason: MainProc was still entering until 17:39:27, state record written 5.5s later.
  Alive=N due to silence threshold, not indicative of hang.
```

## 補充調查路徑

若判定為 Likely Hung，進一步調查：

1. `EventLogTxt_YYYYMMDD.csv` — 附近時間的機台動作、Alarm、Start/Stop
2. 程式碼路徑：
   - `Synchronize(ThreadProcess)` — GUI thread 依賴，可能造成 deadlock
   - `ThreadProcess()` → `MainProc()` — 主迴圈流程
   - 異常路徑：`MainProc()` 周邊的 exception handling
   - Sleep/Wait loops：可能遮蔽真正的 stall 狀態

<!-- preserved-content:end -->
