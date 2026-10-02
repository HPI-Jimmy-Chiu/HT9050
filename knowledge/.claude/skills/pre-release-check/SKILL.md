---
name: pre-release-check
description: >-
  用於 BCB6 / HT9045 專案的 C/C++ 上線前風險檢查技能。於 build 或 commit 前掃描
  已修改檔案中的常見缺陷模式：布林條件中的裸 enum 值、鏈式 || / && 遺漏比較運算子、
  多階段迴圈邏輯回歸、Save/Reload 路徑一致性問題、條件式誤用變數、以及除以零風險。
  適用時機：
  (1) 準備 release build，
  (2) SVN commit 前 code review，
  (3) 驗證 bug fix 未引入新回歸，
  (4) 稽核多階段迴圈條件邏輯，
  (5) 掃描未防護除法，
  (6) 稽核 LastSet 結構陣列越界風險,
  (7) 稽核索引映射陣列（如 iOCRMap）使用一致性,
  (8) 稽核寫檔前資料夾存在性（ctor / FormCreate / 客製路徑）。
  觸發關鍵字：pre-release, release check, code review, risk check,
  上線前檢查, release audit, regression check, 除法, divide by zero, division safety,
  除以零, 除法替換, LastSet, array overflow, out-of-bounds, buffer overrun, array index,
  boundary check, iOCRMap, index map, mapping array, row swap, OCR mapping,
  copy-paste error, hardcoded index,
  fopen, SaveToFile, MyForceDirectories, EnsureDirectoriesExist, folder exist, 寫檔, 資料夾未建立,
  vector clear memory leak, push_back new, delete pointer, vector<T*>, IOTable, MotTable, memory leak, 記憶體洩漏,
  == used as =, assignment operator error, comparison instead of assignment, 賦值誤用比較, no side effect, expression has no effect,
  malloc delete mismatch, malloc free, new delete array, delete[], new[], memory allocator mismatch, C C++ allocator, UB undefined behavior, free() vs delete.
---

# 上線前程式碼風險檢查（Pre-Release）

在上線前，針對原始碼檔案掃描高風險 C/C++ 缺陷模式。
每一種模式皆來自 HT9045 專案中曾發生過的實際生產問題。

---

## 觸發時機

當使用者說以下任一語句時執行：「pre-release check」、「上線前檢查」、「release audit」、「code review」、「準備 release」、「commit 前檢查」

---

## 工作流程

| 步驟 | 動作 | 參照 |
|------|------|------|
| 1 | 確認目標檔案（`.cpp` / `.h` / `.dfm`） | — |
| 2 | 平行掃描 P1–P12 + DFM Batch D | [parallel-scan-strategy.md](references/parallel-scan-strategy.md)<br>[dfm-resolution.md](references/dfm-resolution.md) |
| 3 | 格式化 Batch E（F1–F11） | [formatting-standards.md](references/formatting-standards.md) |
| 4 | 輸出 findings 表格 | [workflow.md](references/workflow.md) §[4] |
| 5 | 若無 findings → 標記 clean | [workflow.md](references/workflow.md) §[5] |
| 6 | 平行：Track A 編譯 + Track B MD 草稿 + Track C 語意分析 | [compile-verification.md](references/compile-verification.md)<br>[report-output.md](references/report-output.md) §階段一<br>[semantic-change-analysis.md](references/semantic-change-analysis.md) |
| 7 | 完整報告 MD + HTML | [report-output.md](references/report-output.md) §階段二 |

⚠️ 步驟 6 需**等待使用者確認 Track B 草稿與 Track C 不確定項，且 Track A 編譯完成後**，才進行步驟 7。

完整流程細節：[workflow.md](references/workflow.md)

---

## 風險模式索引

| Pattern | 嚴重度 | 一句話說明 | 詳細參照 |
|---------|--------|------------|----------|
| **P1** | Critical | 鏈式布林條件中的裸值（`var==A \|\| B \|\| var==C` 中的 `B`） | [patterns.md](references/patterns.md) §P1 |
| **P2** | High | Enum 直接當布林（`if(EnumName)` 未比較） | [patterns.md](references/patterns.md) §P2 |
| **P3** | High | 多階段迴圈條件變更，可能影響其他迭代 | [patterns.md](references/patterns.md) §P3 |
| **P4** | High | Save/Reload 路徑不對稱，悄悄覆寫正確資料 | [patterns.md](references/patterns.md) §P4 |
| **P5** | Critical | 條件式誤用變數（Copy-Paste Error，例：Output 區段誤用 `InOfsXxx`） | [patterns.md](references/patterns.md) §P5 |
| **P6** | Critical | 除以零風險（`a / b` 或 `a / (x-y)` 未防護除數為 0） | [division-safety.md](references/division-safety.md) |
| **P7** | Critical | 陣列越界（LastSet 結構陣列缺邊界 guard、`\|\|`/`&&` 方向錯、off-by-one；**迴圈硬編碼上限 N > 陣列宣告大小 M**） | [lastset-array-audit.md](references/lastset-array-audit.md)；迄圖 [ht9045-array-audit skill](../../../HT9045/.github/skills/ht9045-array-audit/SKILL.md) |
| **P8** | Critical | 索引映射陣列旁路（`iOCRMap` / `iBarCodeRowA/B` 混用硬碼索引） | [index-map-bypass.md](references/index-map-bypass.md) |
| **P9** | High | 寫檔前未確認資料夾存在（ctor / FormCreate / 客製路徑 `fopen`、`SaveToFile`） | [folder-ensure-before-write.md](references/folder-ensure-before-write.md) |
| **P10** | High | `vector<T*>.clear()` 前未 delete 指標，Reload 按鈕每次觸發均洩漏（e.g. `IOTable`、`MotTable`） | [patterns.md](references/patterns.md) §P10 |
| **P11** | Critical | `==` 誤作 `=`（陳述式位置比較無 side effect，變數實際未賦值，編譯器僅 warning） | [patterns.md](references/patterns.md) §P11 |
| **P12** | Critical | `malloc`/`delete` 配對錯誤（C 標準庫分配必須用 `free()` 釋放，不得用 `delete`；`new[]` 必須用 `delete[]`） | [patterns.md](references/patterns.md) §P12 |
| **P13** | High | block-memory size 比對錯誤（`strcpy`/`strncpy` 以來源長度為 N 而非目標大小；`strncpy` 填滿無結尾 `\0`；跨結構 `memcpy` 兩端大小不一致） | [patterns.md §P13](references/patterns.md) |

### 格式化規則

| 規則 | 範圍 | 詳細 |
|------|------|------|
| F1–F3 | 全局格式化（Tab→空白、行尾空白、連續空白行） | [formatting-standards.md](references/formatting-standards.md) |
| F4 | 控制結構括號一致性 | 同上 |
| F5–F9 | 控制結構空白行管理（⚠️ F9 需在 F5–F8 後執行） | 同上 |
| F10 | 閉括號後接 `if` 應有空白行 | 同上 |
| F11 | 函式分隔線 | 同上 |
| F12 | 行內註解階梯對齊（選用） | 同上 |

---

## 核心禁令

- ❌ **不得使用 PowerShell `|` 接 `make`**：BCB6 ilink32 需建 `MAKE0000.@@@`，stdout 被 pipe 接管會 Fatal 並刪除 EXE。**必須用 `cmd /c "make ... > log.txt 2>&1"`**。詳見 [compile-verification.md](references/compile-verification.md)。
- ❌ **Track C UNCERTAIN 不得自行判斷**：必須向使用者確認後才標記 OK 或 CHANGED。詳見 [semantic-change-analysis.md](references/semantic-change-analysis.md)。
- ❌ **未清 Obj 直接 build**：增量編譯可能隱藏真實錯誤；全重建前必須 `Remove-Item "<Project>\Obj\*" -Recurse -Force`。

---

## 下游技能同步

- 本技能的風險模式索引（**P1–P13**）若有增減，**必須同步更新** [make-ht9045-installer/workflow.md](../make-ht9045-installer/references/workflow.md) §Step 2「完整範圍」所列的掃描範圍。兩處 Pattern 範圍不一致 → 發版流程會漏掃。

---

## DFM 視窗超出可視範圍：修復約定

當 Batch D 發現 **Form**（`object Xxx: TXxx`）的 `Left`/`Top` 設計值落在單螢幕可視範圍外（>1280 / >1024 且 <60000）：

- **修法**：僅將該 **Form 物件本身** 的 `Left` / `Top` 改為 `10` / `10`。
- **禁止**：變動 Form 內任何子元件（Panel / Button / Edit…）的座標——子元件座標為相對父層，Form 移動後版面不變。
- **命中非 Form（如 `TPanel`）**：屬內部元件，**不修改**，僅標記待人工確認母 Form 寬度 / `Anchors`。
- 案例：V3.33.906.0 — `frmDefrost`(2038) / `fBinAOISel`(1959) / `fAdam6024`(1611) 三個 Form 改為 10/10；`Magazine.dfm` 的 `pnlMagazineTrayOut`(TPanel) 屬內部元件未動。

```
## Pre-Release Risk Check Results

| # | File | Line | Pattern | Severity | Description | Suggested Fix |
|---|------|------|---------|----------|-------------|---------------|
| 1 | cOffSet.cpp | 1794 | P1 | Critical | Bare `OutOfsAuto3` in `||` chain | Add `i==` before `OutOfsAuto3` |

**Summary:** X file(s) scanned, Y risk(s) found (Z critical).
```

無 findings 時：

```
## Pre-Release Risk Check Results

All X file(s) scanned — no risk patterns detected. ✔
```

---

## 參考腳本（scripts/）

| 腳本 | 用途 |
|------|------|
| `scripts/scan_and_report_pre_release.py` | 自動掃描 **P1/P2/P4/P5/P6/P7/P9/P11/P12/P13 + DFM(Form)** 並生成報告。**吃命令列參數**：`python scan_and_report_pre_release.py <版本資料夾> [--report-dir ...] [--developer ...]`，版本標籤由資料夾名稱自動抽出。預設輸出至 `D:\docs\customers\HPI-TW\0000_HonPrec`。**報告產製 follow `make-report-skill`**：MD 不放 Logo，HTML 一律由 `md_to_html.py --template blue`（honprec-blue-template）轉出，不在腳本內嵌 base64。P11 內建 4 道 FP 過濾與嚴重度啟發式；DFM 只看 Form 物件本身。 |
| `scripts/run_build_verify_20260323.ps1` | BCB6 全重建腳本（建議改用 [compile-verification.md](references/compile-verification.md) 的 `cmd /c` 版本以避免 MAKE0000.@@@ 問題） |
| `scripts/parse_build_log_20260323.py` | 解析 build log 輸出 Error/Warning 統計 |

---

## References 索引

| 檔案 | 內容 |
|------|------|
| [workflow.md](references/workflow.md) | 7 步驟詳細流程 |
| [patterns.md](references/patterns.md) | P1–P13 完整定義、regex 提示、案例 |
| [parallel-scan-strategy.md](references/parallel-scan-strategy.md) | Batch A/B/C/D/E 分組與 sub-agent prompt 範本 |
| [dfm-resolution.md](references/dfm-resolution.md) | Batch D：1280×1024 / 1920×1080 規範 |
| [formatting-standards.md](references/formatting-standards.md) | F1–F12 格式化規則與自動化腳本 |
| [division-safety.md](references/division-safety.md) | P6 完整 4 階段流程、`ChangeToFloatNonPcnt` 與 P6b UB 處理 |
| [lastset-array-audit.md](references/lastset-array-audit.md) | P7 陣列邊界表、grep 腳本、歷史案例 |
| [index-map-bypass.md](references/index-map-bypass.md) | P8 重構範本、Fix C-1 / Fix D 案例 |
| [folder-ensure-before-write.md](references/folder-ensure-before-write.md) | P9 高風險 API 清單、helper 用法、ctor/FormCreate 必查、修復範例、歷史案例 |
| [patterns.md §P10](references/patterns.md) | P10 `vector<T*>` clear 不 delete 洩漏：偵測 regex、修復樣板、案例（IOTable/MotTable） |
| [patterns.md §P10b](references/patterns.md) | P10b 可重入 `Initial*` 函式無 null guard 直接 `new`（PordRec 案例）、建構子當語句（ctor-as-statement）|
| [patterns.md §P11](references/patterns.md) | P11 `==` 誤作 `=`：偵測 regex、11 個歷史案例（905.8 2026-06-12） |
| [patterns.md §P12](references/patterns.md) | P12 `malloc/delete` 配對錯誤：偵測策略、真假陽性區分、1 個歷史案例（uLotInfo.cpp 905.8 2026-06-12） |
| [patterns.md §P13](references/patterns.md) | P13 block-memory size 比對：`strcpy`/`strncpy`/`memcpy` 偵測 regex、三種常見陷阱、歷史案例（Command.cpp:10491、cShowBinSelect.cpp:2336）。詳見 [ht9045-array-audit §G](../../../HT9045/.github/skills/ht9045-array-audit/references/cprod-array-audit.md) |
| [memory-leak-runtime-debug.md](references/memory-leak-runtime-debug.md) | P10/P10b Runtime 現場除錯：MEM Sample 埋點、General.ini 開關、EventLog 對照、~130MB Reload 洩漏案例（**非靜態掃描**，僅供事後追查） |
| [compile-verification.md](references/compile-verification.md) | Track A：BCB6 全重建腳本與結果分析 |
| [semantic-change-analysis.md](references/semantic-change-analysis.md) | Track C：sub-agent prompt 與後處理規則 |
| [report-output.md](references/report-output.md) | 階段一/階段二報告規格與命名規則 |
