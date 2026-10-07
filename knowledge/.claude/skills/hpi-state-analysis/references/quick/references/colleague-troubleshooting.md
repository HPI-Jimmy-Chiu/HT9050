> 保存來源：`.claude/skills/data-analysis/references/colleague-troubleshooting.md`，main `811d95068`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# State Record 分析 — 常見問題診斷

> 補充 SKILL.md 的判斷規則，提供具體場景的處理方式。

---

## 問題 1：Alive=N，但 SilentSec 只有幾秒

**現象**：
- `Alive=N`
- `SilentSec=5.x ~ 10.x`
- `LastEnter` 與 `SaveTime` 差距很小

**診斷**：正常快照，不代表 hang。  
`Alive=N` 的觸發條件是 SilentSec 超過閾值（預設 5 秒），機台靜止等待時即會觸發。

**處理**：
1. 確認 `LastEnter` 到 `SaveTime` 的間距是否 < 10s → 可判定正常
2. 查看 `Task_ListWithTime2.csv` 末尾是否有合理的 task 活動
3. 若確認正常，在報告中標記：`Interpretation: normal`

---

## 問題 2：SilentSec 超過 30 秒，但無法判定是否 hung

**現象**：
- `SilentSec=30.x ~ 90.x`
- `Alive=N`
- 不確定機台當時是否處於長時間等待的正常狀態

**診斷**：列為 Suspicious，需要更多脈絡。

**處理**：
1. 查看 `EventLogTxt_YYYYMMDD.csv` 在 `LastEnter` 時間前後是否有：
   - 溫控等待（ATC Soak/Wait）
   - 測試器逾時（TESTER_TIMEOUT）
   - 暫停命令（PAUSE）
2. 若有上述事件 → 正常等待，降低 hang 懷疑度
3. 若無特殊事件，且 `Task_ListWithTime2.csv` 也停在同一時間 → 升高懷疑度

---

## 問題 3：Task_ListWithTime2.csv 的時間比 LastEnter 還早很多

**現象**：
- `LastEnter=18:12:43`
- Task 最後活動：`18:12:40`
- `SaveTime=18:15:17`（間距 154 秒）

**診斷**：Likely Hung。  
MainProc 停止進入，且 task 狀態也停止更新，強烈指向 thread hang。

**處理**：
1. 確認程式碼路徑：
   - `Synchronize(ThreadProcess)` 是否可能因 GUI thread 阻塞而卡死
   - `MainProc()` 附近是否有 exception 路徑未處理
   - Sleep / Wait loop 是否無法退出
2. 查閱相關 alarm 記錄，找到最後的正常動作
3. 回報結論：`Interpretation: likely hung`，附上三時間點對照

---

## 問題 4：Task_ListWithTime.csv 沒有 MainProcMonitor 行

**現象**：
- 找不到 `MainProcMonitor` 欄位

**原因**：
- 版本較舊的程式碼尚未加入 heartbeat logging
- 或狀態記錄在寫入時 MainProcMonitor 尚未初始化

**處理**：
1. 改用 `Task_ListWithTime2.csv` 的最後一筆時間估計 main thread 活動
2. 與 `SaveTime`（資料夾建立時間）對比
3. 在報告中標注：`MainProcMonitor: 不存在，以 Task_ListWithTime2 末行替代`

---

## 問題 5：狀態記錄是手動觸發的

**現象**：
- 使用者說「有人手動存了狀態記錄」
- `SaveTime` 與實際發生問題的時間不吻合

**診斷**：`SaveTime` 是手動觸發的寫入時間，不是故障發生時間。

**處理**：
1. 重新以 `LastEnter` 為分析基準時間
2. `SaveTime` 只代表「狀態記錄被寫入」，不代表「故障發生」
3. 報告中標注：`SaveTime 為手動觸發，不作為故障時間點`

---

## 問題 6：同一時間有多份狀態記錄

**現象**：
- 發現 2 個以上的狀態記錄資料夾，時間接近

**處理**：
1. 以最後一份（SaveTime 最晚）為主要分析對象
2. 比對多份的 `LastEnter` 是否一致（若一致，表示 hang 確實在該時間點停止）
3. `CallCount` 若在不同快照間沒有增加 → 強烈 hang 證據

---

## 快速判斷速查表

| SilentSec | Task 活動 | LastEnter vs SaveTime | 結論 |
|-----------|----------|-----------------------|------|
| < 10s | 正常 | 接近 | ✅ Normal |
| 10~60s | 有 ATC/PAUSE 事件 | 接近 | ✅ Normal（等待狀態）|
| 10~60s | 無特殊事件 | 有差距 | ⚠️ Suspicious |
| > 60s | 停在 LastEnter | 相差 > 30s | 🔴 Likely Hung |
| > 60s | 連 Task 都停了 | 相差 > 60s | 🔴 Likely Hung（強烈）|

---

*最後更新：2026-04-20 | ht9045-state-record-analysis maintenance*

<!-- preserved-content:end -->
