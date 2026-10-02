---
name: hangup-intake
description: HT9045 hangup 案例自動化分析入口。當使用者貼出客戶 hangup 截圖、提到「state record / 卡站 / hang up / 客戶反應 / 異常 / hangup 資料夾」並有指向 hangup/ 內子資料夾時，立刻載入此技能。本技能負責：(1) 偵測或確認最新 hangup 子資料夾；(2) 執行 analyze_hangup.py 取得 JSON 摘要；(3) 解讀客戶截圖/描述；(4) 串聯到對應 ht9045 子技能與 subagent 進行根因分析。關鍵字：hangup, state record, Task_ListWithTime, EventLogTxt, PickHPRec, 客戶反應, 客戶說明, 卡站, 截圖, hang up 分析, 客戶問題, 客訴, 高通, ASE, JCET, SPIL。
---

# HT9045 Hangup 自動化分析 Skill

## 觸發時機
使用者出現以下任一情境：
1. 貼出 hangup 截圖 + 文字描述
2. 提到「state record 在 hangup/<時間> 資料夾內」
3. 詢問特定 hangup 為何卡站、為何 hang up
4. 把 hangup 子資料夾或內部檔案以 attachment 方式拉入對話

## SOP（每次嚴格依序執行）

### Step 1 — 鎖定 hangup 資料夾
- 若使用者已指明：使用該路徑
- 若使用者只說「最新」或未明示：執行 `python .tools\hangup\analyze_hangup.py --latest` 自動挑選 `hangup/` 下 mtime 最新者
- 若使用者貼了多個檔案附件：以附件所在資料夾為主

### Step 2 — 產出結構化摘要
執行：
```
python .tools\hangup\analyze_hangup.py "<hangup_folder>"
```
輸出 `<hangup_folder>/hangup_summary.json`，欄位：
- `eventlog.tail_significant`：最後 30 筆有意義事件（含 ts/code/msg/unit）
- `eventlog.top_codes` / `top_messages`：頻次排行
- `task_list.suspicious_stuck_tasks`：State record 中疑似卡死的 Task
- `pick_hp_rec` / `pick_hp_rec_exception`：HotPlate 排程結構是否被掏空
- `anomalies`：自動推斷的異常清單
- `suspected_modules`：嫌疑模組（InArm/OutArm/Shuttle/Index/HotPlate/CatchTray/AutoClean/ATC）
- `code_locations`：**自動把 EventLog 訊息反查到程式碼位置**（依 T4 索引），含 file/line/func

**重要**：先讀 JSON 摘要，再決定是否還要原始 CSV。避免直接讀 Task_ListWithTime.csv（檔案極寬，浪費 token）。

### Step 2.5 — 索引維護（必要時）
如果 `code_locations` 為空，或者剛同步了新的 V899 source，重建索引：
```
python .tools\hangup\build_error_index.py
```
（掃約 700 個 cpp/h 檔，~10 秒）

### Step 3 — 載入對應子技能
依 `suspected_modules` 載入：
| 嫌疑模組 | 必載技能 |
|----------|----------|
| InArm / HotPlate | `ht9045-inarm-flow`, `ht9045-inarm-suck-logic`, `ht9045-v899` |
| OutArm | `ht9045-outarm-flow`, `ht9045-sorting-bintray` |
| Shuttle | `ht9045-shuttle-flow` |
| Index | `ht9045-index-flow`, `ht9045-contact-force` |
| CatchTray | `ht9045-catchtray-flow`, `ht9045-tray-group-mechanism` |
| AutoClean | `ht9045-autoclean-flow` |
| ATC | `ht9045-atc-interface` |
| LotInfo / 批次 | `ht9045-lotinfo-flow`, `ht9045-autostart-flow` |
| 任何 V899 程式碼分析 | `ht9045-v899` + `data-analysis` |

### Step 4 — 反查程式碼
- **優先使用** `summary.code_locations`（已由 T4 索引自動反查）
- 若索引未命中或訊息為新增，才使用 `grep_search` 在 `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/**/*.cpp` 找觸發點
- 注意一個 message 可能在多處 hardcode，需逐一比對 Task 值與 EventLog 時序判斷

### Step 5 — 套 data-analysis 五段式輸出
最終回覆**必須**使用以下章節結構（zh-tw）：
1. **問題摘要** — 時間 / 模組 / 客戶 / Recipe / 表象
2. **關鍵證據** — EventLog 時序表 + 程式碼位置（用 markdown 連結 + 行號）
3. **根因假設** — 依機率排序 1~3 個
4. **最短驗證步驟** — 列出可立即取得的進一步證據
5. **建議修正** — 分臨時止血 / 永久修正

### Step 6 — 詢問是否沉澱經驗
回覆結尾詢問使用者：「是否要把本次案例寫入 repo memory 或 docs/cases/？」  
若同意：寫到 `/memories/repo/hangup-<yyyymmdd>-<short>.md`

## 不要做的事
- ? 不要直接讀 `Task_ListWithTime.csv` 全文（先讀 summary.json）
- ? 不要修改 `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422` 以外版本
- ? 不要在未經使用者確認下修改程式碼（V899 的 safety-critical-change 規則優先）
- ? 不要輸出英文報告，使用 zh-tw

## 加分動作
- 若 `pick_hp_rec.all_groups_empty == true` 且 EventLog 出現 `HasHotReadyIC_9045` → 直接引用 `/memories/repo/hangup-20260419-hotplate-stranded.md`（若存在）作為相似案例
- 若有 `RD5軟體` 報告檔需求，檔名格式：`RD5軟體_<主題>_<YYYYMMDD>_<HHMMSS>.md`
