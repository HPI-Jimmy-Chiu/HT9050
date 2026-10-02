# Pre-Release Check 工作流程

> 7 步驟總流程；個別技術細節請參照 SKILL.md 的「風險模式索引」與相關 reference 檔。

---

## 流程概覽

```
[1] 確認目標檔案
       │
       ▼
[2] 平行掃描 P1–P13 + DFM Batch D     ──┐
[3] 格式化 Batch E                      │  Batch A/B/C/D 平行
       │                                │  Batch E 接續
       ▼
[4] 輸出 findings 表格
[5] 若無 findings → 標記 clean
       │
       ▼
[6] 平行：Track A（編譯）+ Track B（MD 草稿）+ Track C（語意變動分析）
       │   ⚠️ 等待使用者確認 Track B 草稿與 Track C 不確定項
       ▼
[7] 完整報告 MD + HTML
```

---

## 步驟細節

### [1] 確認目標檔案

範圍：`.cpp` / `.h` / `.dfm`。建議先取得 `svn status` 或本次 commit diff 範圍。

#### [1a] Scope Fallback（D）— `svn status` 找不到 `M` 修改檔時

當 `svn status` **沒有任何 `M`（已修改）檔案**（例如工作複本剛 checkout、或變更已 commit），不可直接判定「無可掃描範圍」。依序套用下列 fallback：

1. **與前一版資料夾比對** — 若手上有上一版原始碼資料夾，對兩版做檔案差異，取有變動的檔案為掃描範圍。
2. **指定模組範圍** — 詢問使用者本次關注的模組（如 InArm / Tester / Yield），只對該批檔案做完整 P1–P13。
3. **退回全量 baseline 稽核** — 若使用者要「不管有沒有改都檢查」，改走 [1b] 全量稽核變體。

> ⚠️ 切勿因為「沒有 M 檔」就回報 clean —— 先確認使用者意圖（scoped vs 全量）。

#### [1b] 全量 / Baseline 稽核變體（E）

當使用者要求「全面性檢查 / 不管有沒有改都檢查整個版本」時，改用此變體（**而非**逐檔語意精掃）：

- **只跑高精度、可自動驗證的模式**：**P11 / P12 / P1 / DFM(Form)**。這些誤報可控、可逐筆人工確認。
- **P11 必須套用 B 的 4 道 FP 過濾**；**DFM 只看 Form 物件本身**（見 [dfm-resolution.md](dfm-resolution.md)）。
- **每筆 finding 逐一讀上下文確認**，避免 false positive。
- **誠實標註限制**：P3/P4/P5/P6/P7/P8/P9/P10/P13 等語意型模式在 74 萬行層級誤報率過高，**未在全量變體中保證涵蓋**；如需請縮小到模組範圍分批深掃。
- 可直接執行 `scripts/scan_and_report_pre_release.py <版本資料夾>`（已含 P11/P12/P1/DFM 與 FP 過濾）。

### [2] 平行掃描（Batch A/B/C/D）

| Batch | 模式 | 觸發條件 |
|-------|------|----------|
| A | P1–P3 | 任何掃描 |
| B | P4–P5 | 任何掃描 |
| C | P6–P8 | 任何掃描 |
| C2 | P9–P13（含 **P11 `==`→`=`**、**P12 malloc/delete**） | 任何掃描 |
| D | DFM 解析度（**只看 Form 物件本身**） | 有 `.dfm` 修改時 |

→ 詳見 [parallel-scan-strategy.md](parallel-scan-strategy.md) 與 [dfm-resolution.md](dfm-resolution.md)。

### [3] 格式化檢查（Batch E）

A/B/C/D 完成後執行；F1–F11 規則。→ 詳見 [formatting-standards.md](formatting-standards.md)。

### [4] 輸出 findings

```
## Pre-Release Risk Check Results

| # | File | Line | Pattern | Severity | Description | Suggested Fix |
|---|------|------|---------|----------|-------------|---------------|
| 1 | cOffSet.cpp | 1794 | P1 | Critical | Bare `OutOfsAuto3` in `||` chain | Add `i==` before `OutOfsAuto3` |

**Summary:** X file(s) scanned, Y risk(s) found (Z critical).
```

### [4a] P9 有發現時：確認使用者偏好 helper

若 findings 表格中存在 P9 項目，**輸出完 findings 後立即提問，再繼續**：

> **P9 發現 X 個寫檔前未確認資料夾的風險點。**
> 修復時要使用哪種保護方式？
>
> **A** — `MyForceDirectories`（推薦用於 HT9045 大型專案；有例外彈窗 + `RecordProcess` LOG，方便現場追查）
> **B** — `FileInfo().EnsureDirectoriesExist`（適用於 catch 內、不需彈窗的場合）
> **C** — `if(!DirectoryExists) ForceDirectories`（其他專案，如 GPIB9045 / RS232Standard）
> **D** — 讓我自己逐點決定

⚠️ **等待使用者回覆後**，才在 Suggested Fix 欄填入對應的具體寫法。

若使用者選 **A**，但搜尋目標專案後**找不到** `MyForceDirectories` 的宣告，執行下列兩步確認：

**步驟 1 — 詢問是否新增 `MyForceDirectories`：**
> 此專案目前沒有 `MyForceDirectories`。是否要將它移植進來？
> - **是** → 繼續步驟 2
> - **否** → 改用選項 C（`if(!DirectoryExists) ForceDirectories`）

**步驟 2 — 確認依賴函式是否存在（僅在回答「是」後執行）：**
> `MyForceDirectories` 內部使用 `RecordProcess` 與 `ShowMyMessage`。
> 此專案有對應的 LOG / 錯誤彈窗函式嗎？
> - `RecordProcess` → 專案中是否有類似的 log 函式（如 `WriteLog`、`AddLog` 等）？
> - `ShowMyMessage` → 是否有對應的錯誤對話框函式（如 `MessageBox`、`ShowMessage` 等）？
>
> 請告知，我會根據你的回覆調整移植內容（替換依賴或保留空實作）。

### [5] Clean 標記（無 findings 時）

```
## Pre-Release Risk Check Results

All X file(s) scanned — no risk patterns detected. ✔
```

### [6] 平行：編譯 + 草稿 + 語意分析

| Track | 內容 | 參照 |
|-------|------|------|
| A | BCB6 全重建 | [compile-verification.md](compile-verification.md) |
| B | sub-agent 產生 MD 草稿 | [report-output.md](report-output.md) §階段一 |
| C | sub-agent 對被修改片段做語意變動分析 | [semantic-change-analysis.md](semantic-change-analysis.md) |

⚠️ 三軌平行啟動；**Track B 草稿與 Track C 不確定項需使用者確認**，且 Track A 編譯完成後，再進入步驟 7。

### [7] 完整報告

合併 Build 結果到 MD 並轉 HTML。→ 詳見 [report-output.md](report-output.md) §階段二。

---

## 注意事項

- 多階段迴圈一旦修正，務必重新驗證所有 stage，不可只驗證原始問題 stage。
- 對 3 項以上的 `||` / `&&` 條件需特別小心，每一項都必須有明確比較運算子。
- P6（除以零）建議使用 [division-safety.md](division-safety.md) 的完整批次流程。
- **Big5 修復提醒（H）**：HT9045 / HT9011UC 的 `.cpp` / `.h` / `.dfm` 多為 **Big5(CP950)** 編碼。
  進入修復階段（將 finding 改成正確程式碼）時，**禁止用 `replace_string_in_file` 寫入含中文的內容**（會以 UTF-8 bytes 寫入造成亂碼）。
  修法：① 純 `==`→`=` 等 ASCII 變更可用 Python binary（latin-1 1:1 round-trip）；② 需加中文註解時改用純 ASCII，或從來源檔以 binary 複製整段 bytes。詳見 user memory `big5-replace-string-utf8-bug.md`。

---

*最後更新：2026-04-24*
