---
name: ht9045-code-merge
description: >
  HT9045/HT9011UC IC Handler 多人協作 SVN 合併工作流程。
  涵蓋：Step 0-2 SVN 分析/修復/差異比對，詳見 references/merge-svn-analysis.md；
  Step 3-4 檔案合併/衝突解決，詳見 references/merge-core.md；
  Step 5-6 BCB6 編譯驗證/上線檢查，詳見 references/merge-build-verify.md。
  觸發關鍵字：merge, 合併, 整合, SVN, HT9045, HT9011UC, 3-way merge, 多人協作,
  code merge, svn status, svn diff, 三方合併, 衝突解決, 分支整合,
  .svn 修復, svn repair, DirectCopy, ChangeToFloatNonPcnt, 版本交叉合併,
  SVN commit 逆向合併, difflib, 逆向差異, crossing revision, 合併衝突,
  衝突標記, Both-Added, SOFT_SIMULTE, eSortOffset, whitespace normalize,
  Tab轉換, cp950, 備份, .bak, scripts/three_way_merge.py,
  BCB6 編譯, bpr2mak, make, build log, Unresolved external,
  Undefined symbol, Declaration syntax error, E2148, cmydef.h, cmydef.cpp,
  連結錯誤, MergeReport, 合併報告, build verify, 0 errors,
  多來源合併, 目標版本命名, 目標版本命名規則, Phase 0a, Phase 0b,
  svn export, SVN Base Revision, 审核者, 執行者, commit message,
  Section 8, Section 9, 功能來源追蹤, SVN Commit Message.
applyTo: "**/*"
---

# HT9045/HT9011UC 多人協作 SVN 合併工作流程

## 專案資訊

| 項目 | 值 |
|------|-----|
| 專案 | HT9011UC (HT9045 IC Handler) |
| 開發環境 | Borland C++ Builder 6 (BCB6) |
| BCB6 路徑 | `D:\ProgramFiles\Borland\CBuilder6` |
| 原始碼編碼 | **Big5 (CP950)** — 所有 .cpp/.h/.dfm/.rc |
| SVN 儲存庫 | `file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20` |
| EXE 路徑 | `D:\HT9045\EXE\HT9045.exe` |
| 合併報告路徑 | `<入口網站 repo>\public\Docs\MergeReport\<年度>\` |
| SVN Base 路徑 | `D:\HT9045\HT9045_SVN_TempFile\` |
| Python | `C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe` |

> **注意**：Windows App Store 的 `python` 無法使用，請使用完整路徑的 Python。

---

## 目標版本命名規則

合併前必須先從 SVN 建立一份**乾淨的目標版本**，名稱格式如下：

```
HT9011UC_Code_V3.33.<SVN_REV>.0_<YYYYMMDD>_<作者一>_<作者二>[_<作者三>]
```

| 範例情境 | 目標版本資料夾名稱 |
|---------|------------------|
| Steven + RogerYang | `HT9011UC_Code_V3.33.901.0_20260408_Steven_RogerYang` |
| Steven + RogerYang + Kevin | `HT9011UC_Code_V3.33.902.0_20260415_Steven_RogerYang_Kevin` |

> `SVN_REV` = merge 完成後準備 commit 的目標 revision 號；合併開始時可先用「預計號」，commit 後補正。

---

## 快速啟動（多來源合併）

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"

# 來源 N → 目標版本（每位來源分别執行一次）
& $Python `
    "D:\HT9045\.github\skills\ht9045-code-merge\scripts\merge_main.py" `
    --source "<來源版本路徑>" `
    --target "<目標版本路徑>" `
    --developer "<來源開發者>"
```

> 多來源時對每位來源依序執行一次，目標路徑不變。詳細將 [references/merge-workflow.md](references/merge-workflow.md)。

---

## 合併流程概覽

```
[前置] Phase 0a: 從 SVN 建立乾淨目標版本  ← 必要步驟
[前置] Phase 0b: 建立每位來源的功能清單

對每個來源版本依序執行：
  Step 0: SVN 環境準備與診斷      → references/merge-svn-analysis.md
  Step 1: SVN 分析                 → references/merge-svn-analysis.md
  Step 2: 差異比對 + 風險分類      → references/merge-svn-analysis.md
  Step 3: 檔案合併                 → references/merge-core.md
  Step 4: 後處理步驟               → references/merge-core.md
  Step 4.5: Layer 1 靜態驗證       → references/merge-build-verify.md
  ↑ 每輪整合後均需通過 Step 4.5

[最終] Step 5: BCB6 編譯驗證（所有來源整合完畢後執行一次）
[最終] Step 6: 上線檢查 + 報告產生
```

> **多來源順序建議**：衝突較少（直接複製多）的來源先合，較複雜的後合，降低迭代衝突機率。

---

## Phase 0a：建立乾淨目標版本

> 詳細指令詳見 [references/merge-workflow.md → Phase 0a](references/merge-workflow.md)

**流程**：`svn info` 取 Revision 號 → `svn export -r <REV>` 建立乾淨目標資料夾

**目標版本命名格式：**

```
HT9011UC_Code_V3.33.<SVN_REV>.0_<YYYYMMDD>_<作者一>[_<作者二>]
```

> `SVN_REV` = merge 完成後準備 commit 的目標 revision；合併開始時可先用「預計號」，commit 後補正。

---

## Phase 0b：建立功能清單（防漏合前置）

對**每位來源開發者**各建立一份清單，在開始合併前完成：
1. 請開發者提供 Merge 分支的修改摘要（或 commit log / changelog）
2. 建立功能 → 檔案 → 行號範圍對應表（格式如下）
3. 合併過程中逐一勾選確認
4. 最終在 Step 6 回頭逐一驗收每個功能已合入

**功能 Checklist 格式（每位來源一表）：**

| 功能名稱 | 標誌性字串 | 涉及檔案 | 大致行號 | 合併狀態 |
|---------|---------|---------|---------|--------|
| 範例：ATK AMR DoLotEnd | `bATK_AMR_DoLotEndSent` | main.cpp, cprod.h | ~L3200, ~L450 | ☐ |
| ... | ... | ... | ... | ... |

> **重要**：一個功能可能跨多個檔案。標誌性字串用於 Phase 6 grep 驗收，檔案+行號用於合併中對照確認完整性。

---

## 大檔案差異比對規則

對超過 3000 行的檔案，用 `fc.exe /N` 產生全量 diff 並存入檔案，避免終端輸出截斷導致遺漏：

```powershell
fc.exe /N "<主幹路徑>\<檔案.cpp>" "<Merge路徑>\<檔案.cpp>" > "d:\HT9045\AI_Temp\diff_<檔名>.txt"
```

> Session memory 必須記錄大檔案的處理進度（已處理到第幾行），確保分段處理無遺漏。

---

## Diff 區塊標記規則

對每個 diff 區塊明確標記處理結果：

| 標記 | 意義 |
|------|------|
| ✅ 合併 | Merge 分支的變更已合入主幹 |
| ⏭️ 保留主幹 | 保留主幹版本，不套用 Merge 分支 |
| ❓ 待確認 | 兩邊都有修改同一區域，需詢問使用者 |

---

## Step 0–2：SVN 分析

> 詳見 SVN 分析流程，含版本診斷、.svn 修復與 ChangeToFloatNonPcnt 安全保護，以及 Commit 歷史追蹤。
> [references/merge-svn-analysis.md](references/merge-svn-analysis.md)

### 重要摘要

| 步驟 | 摘要 |
|------|------|
| Step 0 | `.svn` 診斷/修復 — 自動偵測與修復（`scripts/svn_helper.py repair`）|
| Step 1 | `svn status`、`svn diff` 取得修改清單 |
| Step 2 | 分類：直接複製 / 3-way merge / 跨版本 3-way / 新增檔案 / 跳過 .bpr|

> **跨版本注意**：當來源 revision < 目標 revision，對候選直接複製檔必須以 `svn cat -r` 做跨版本比對，避免覆蓋已 commit 的修改。

---

## Step 3–4：合併與後處理

> 詳見合併核心，含直接複製/Both-Added 衝突/後處理/source==base 比對/.dfm 特殊處理等完整說明。
> [references/merge-core.md](references/merge-core.md)

### 重要摘要

| 操作 | 工具呼叫 |
|------|---------|
| 直接複製 | `Copy-Item` — 依 Step 2 分類結果執行 |
| 跨版本 3-way merge | `three_way_merge.py --base <svn cat -r N>` |
| 一般 3-way merge | `three_way_merge.py --base <svn BASE>` |
| .dfm 合併 | 視覺比對後人工合併|
| 新增檔案 | 複製 + 加入 .bpr + L1-5 驗證 |
| 空白正規化 | `whitespace_normalize.py`：Tab 換 4 空格、CP950 保護|

**合併前風險評估（每個 3-way 候選檔必做）：**

```powershell
& $Python scripts/three_way_merge.py --base $base --source $src --target $tgt --output $out --risk-check
# SAFE → 全自動  |  MEDIUM → 自動+人工審查  |  HIGH → 直接手動合併
```

> **注意**：合併完成後確認 MachineType.h 中的 `eSortOffset`、`SortOfsTotal`、`SortArmOffSet` 陣列大小正確。

---

## 衝突解決規則 ⚠️

**絕對不要在未經使用者確認的情況下自行決定衝突的解決方式。**

當發現衝突時：
1. 清楚展示兩邊的程式碼差異（標註哪段來自主幹、哪段來自 Merge 分支）
2. 說明各自修改的意圖（從 code 推斷）
3. **詢問使用者**選擇保留哪一邊，或是否需要手動合併

---

## 逐檔交叉驗證（防漏合關鍵步驟）

每個檔案合併完成後，**立即**執行：

1. 用 `fc.exe /N` 比對主幹 vs Merge 分支，產生剩餘差異
2. 逐一檢查剩餘差異，確認每個都屬於以下之一：
   - ✅ 主幹獨有功能（保留不動）
   - ✅ 已合併的區塊（行號偏移造成的誤判）
   - ❌ 遺漏（需補合）
3. 若有 ❌ 項目，回到 Step 3 補合
4. **確認零遺漏後才標記該檔案為「已完成」**

---

## Step 5–6：編譯驗證與上線檢查

> 詳見編譯驗證流程，含 cmydef.h/cpp 補充、連結錯誤處理、Build Log 確認，以及 Kevin/Jimmy 功能確認。
> [references/merge-build-verify.md](references/merge-build-verify.md)

> **Step 6 報告格式**：遵循 `d:\.github\skills\make-report-skill\SKILL.md` 的「合併報告（Merge Report）」類型規範，含輸出路徑（`<入口網站 repo>\public\Docs\MergeReport\<年度>\`）、MD-only 格式，以及 Logo 使用規則（MD 報告不放 Logo）。

### Step 4.5：Layer 1 靜態驗證（編譯前必做）

| 規則 | 檢查內容 | 目標檔案 |
|------|---------|--------|
| L1-1 | 重複 `#define` | MachineType.h |
| L1-2 | `CC_` 客戶代碼數字順序 | MachineType.h |
| L1-3 | `eSortOffset`/`SortOfsTotal`/`SortArmOffSet` 存在性 | MachineType.h |
| L1-4 | `SOFT_SIMULTE`/`BETA_*`/`TEST_*`/`DEV_*` 皆為注解 | *.h |
| L1-5 | 新增 .cpp 對應在 HT9045.bpr（對**來源 working copy** 執行 `svn status`，Target 為 export 無 `.svn`）| HT9045.bpr |

**任何 L1 失敗必須修復後才能進入 Step 5。**

### Step 5 編譯

```powershell
$env:PATH = "D:\ProgramFiles\Borland\CBuilder6\Bin;" + $env:PATH
Push-Location "<TargetPath>"
bpr2mak HT9045.bpr ; make -f HT9045.mak 2>&1 | Tee-Object -FilePath build.log
(Select-String "Error E\d+" build.log | Measure-Object).Count  # 必須為 0
Pop-Location
```

| 錯誤類型 | 解決方案 |
|------|-----------|
| `Unresolved external` | 補 .cpp 到 .bpr |
| `Undefined symbol` | 補 cmydef.h `extern` 宣告 + cmydef.cpp 定義|
| `Declaration syntax error` | 3-way merge 殘留標記 → svn revert 後人工修改 |
| `E2148` | 重複 .h 有 default argument → 移除重複宣告 |
| difflib if-body 錯位 | 搜尋 `if(` 後方接 `=ReadIniData` 的行（缺 4 空格縮排）→ 手動復位 |
| difflib 空白行膨脹 | 合併後比對 Steven 基線，空白行差異區塊還原 Steven 版格式 |

---

## Output Format

處理每個檔案時，清楚呈現：

```text
📄 [檔名]
━━━ 差異 #N ━━━
主幹 (目標版本):
  [程式碼片段]

Merge 分支:
  [程式碼片段]

分析：[說明修改意圖]
建議：[直接合併 / 需要確認]
```

---

## 合併操作約束

| 約束 | 說明 |
|------|------|
| ❌ 不可自行決定衝突 | 所有衝突必須詢問使用者 |
| ❌ 不可一次處理多個檔案 | 逐一確認後再進行下一個 |
| ❌ 不可跳過大檔案 diff 尾段 | 必須處理到最後一個 diff 區塊 |
| ❌ 不可在未執行逐檔交叉驗證前標記完成 | 逐檔交叉驗證是合併對每檔必要步驟 |
| ❌ HIGH 風險檔案不可自動合併 | 必須手動合併 |
| ❌ 不可跳過 L1 靜態驗證 | L1 通過後才能進入編譯 |
| ✅ 必須使用檔案 diff | 超過 3000 行的檔案用 `fc.exe /N` 存入 .txt |
| ✅ 必須記錄大檔案處理進度 | 在 session memory 記錄已處理到第幾行 |
| ✅ 必須執行功能清單驗收 | 所有功能 checklist 全部確認後才算合併完成 |
| ✅ 不修改 .bpr/.res/.obj 等非文字檔 | 專案結構變更風險高，跳過 |
| ✅ three_way_merge.py 輸出必須通過 post-merge hook | exit code 2 = 有問題，不可繼續 |
| ⚠️ difflib 合併後必須檢查 if-body 錯位 | 參見 merge-core.md「瑕疵 A」|
| ⚠️ difflib 合併後必須比對空白行膨脹 | 參見 merge-core.md「瑕疵 B」|

---

## Reference 文件索引

| Reference | 內容 |
|-----------|------|
| [merge-svn-analysis.md](references/merge-svn-analysis.md) | Step 0–2：SVN 環境診斷與差異分析 |
| [merge-core.md](references/merge-core.md) | Step 3–4：檔案合併與衝突解決流程 |
| [merge-build-verify.md](references/merge-build-verify.md) | Step 5–6：BCB6 編譯驗證與上線檢查 |
| [merge-workflow.md](references/merge-workflow.md) | 合併完整工作流程（含 Phase 0a 詳細指令）|
| [scripts-reference.md](references/scripts-reference.md) | 全部腳本 CLI 用法與觸發時機說明 |
| [ht9046au-merge-case.md](references/ht9046au-merge-case.md) | HT-9046AU 合併案例：build/linker 錯誤處理 |
| `d:\.github\skills\make-report-skill\SKILL.md` | Step 6 合併報告格式規範（輸出路徑、Logo 規則、報告類型路由）|
