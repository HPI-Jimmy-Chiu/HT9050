# SVN 分析 — Step 0–2 詳細參考

> 摘自原 ht9045-merge-svn-analysis 技能。主 Skill 見 [ht9045-code-merge/SKILL.md](../SKILL.md)。

## 專案資訊（共用）

| 項目 | 值 |
|------|-----|
| 原始碼編碼 | **Big5 (CP950)** — 所有 .cpp/.h/.dfm/.rc |
| SVN 儲存庫 | `file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20` |
| Python | `C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe` |

> **重要**：Windows App Store 的 `python` 指令無法使用，必須用完整路徑呼叫 Python。

---

## Step 0：SVN 狀態檢查與自動修復

### 問題情境

開發者工作複本可能有以下問題：
1. `.svn` 資料夾遺失（手動複製檔案時未包含）
2. `.svn` 參考資料損毀
3. SVN relocate 問題

### 自動修復機制

當偵測到 SVN 狀態異常時，系統會：

1. **從資料夾名稱解析 Revision 號碼**
   - 命名規則：`HT9011UC_Code_V{Major}.{Minor}.{Revision}.{Patch}_{Date}_{Developer}`
   - 例如：`HT9011UC_Code_V3.33.898.0_20260313_Steven` → Revision = 898

2. **查找參考 SVN 來源**（優先順序）
   1. 同一 Revision 的其他工作複本
   2. SVN repository 直接 checkout

3. **修復方式**
   - 複製完整 `.svn` 資料夾
   - 執行 `svn cleanup` 和 `svn revert` 恢復乾淨狀態

### 手動修復指令

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"
# 檢查 SVN 狀態
& $Python scripts/svn_helper.py check --path "<工作複本路徑>"
# 自動修復
& $Python scripts/svn_helper.py repair --path "<工作複本路徑>" --revision <rev>
# 從另一個工作複本複製 .svn
& $Python scripts/svn_helper.py clone-svn --source "<來源路徑>" --target "<目標路徑>"
```

---

## Step 1：SVN 分析

取得來源開發者版本的修改清單。

```powershell
# 修改檔案清單
svn status "<DeveloperPath>" | Select-String "^M"
# 版本資訊
svn info "<DeveloperPath>"
# 個別檔案差異
svn diff "<DeveloperPath>\<file.cpp>"
```

關鍵欄位：`Revision`（working copy 版本）、`Last Changed Rev`（個別檔案基準版本）。

---

## Step 2：重疊偵測

> **⚠ 直接複製的安全風險**：當來源版本基於較舊的 SVN revision 時，「目標未有本地修改」不代表目標與來源 base revision 時的版本相同。目標可能已在兩版之間透過 commit 累積修改（如 `ChangeToFloatNonPcnt` 安全保護），此時直接複製等同於回退已 commit 的修改，必須改以跨版本 3-way merge 處理。

### 分類規則（SVN 版號相同時）

| 類別 | 條件 | 處理方式 |
|------|------|----------|
| **直接複製** | 目標無本地修改（SVN status clean） | 直接取用來源版本 |
| **重疊（需 3-way merge）** | 兩邊都有本地修改（MD5 不同） | 取 SVN base + 3-way merge |
| **新增檔案** | 來源有但目標沒有（且非 SVN 追蹤） | 複製並加入專案 |
| **跳過** | .bpr/.bpf/.res 專案檔 | 不合併（專案結構變更風險高） |

### 分類規則（SVN 版號不同時）⚠ 重要

當來源 revision **< 目標 revision** 時，對所有「候選直接複製檔」需加做跨版本比對：

| 條件 | 處理方式 |
|------|---------|
| 目標無本地修改，且目標在 `source_rev` 後**無** committed 變更 | 直接複製 |
| 目標無本地修改，但目標在 `source_rev` 後**有** committed 變更 | **跨版本 3-way merge**（以 source_rev 的檔案內容為 base） |
| 目標有本地修改 | 一般 3-way merge |

#### 跨版本比對方法

```powershell
$sourceRev = svn info "<SourcePath>" | Select-String "^Revision:" |
    ForEach-Object { ($_ -split ":\s*")[1].Trim() }

foreach ($rel in $candidateCopyFiles) {
    $targetFile = "$TargetPath\$rel"
    $tmpBase    = "$TempDir\crossrev_$($rel -replace '\\','_')"
    svn cat -r $sourceRev $targetFile > $tmpBase 2>$null
    if ($LASTEXITCODE -ne 0) { $newFiles += $rel; continue }
    $md5Rev = (Get-FileHash $tmpBase    -Algorithm MD5).Hash
    $md5Now = (Get-FileHash $targetFile -Algorithm MD5).Hash
    if ($md5Rev -ne $md5Now) {
        $crossRevMergeFiles += $rel
    } else {
        $directCopyFiles    += $rel
    }
}
```

```powershell
# Python 腳本（含跨版本比對 + 基線統計）
& $Python scripts/overlap_analysis.py --source "<source>" --target "<target>" --check-cross-rev
```

### ChangeToFloatNonPcnt 基線統計（版號不同時必做）

```powershell
# 記錄合併前基線數量
$baseline = (Select-String "ChangeToFloatNonPcnt" "$TargetPath\*.cpp" -Recurse | Measure-Object).Count
Write-Host "合併前 ChangeToFloatNonPcnt 基線計數：$baseline"
```

合併後立即驗收：

```powershell
$afterCount = (Select-String "ChangeToFloatNonPcnt" "$TargetPath\*.cpp" -Recurse | Measure-Object).Count
if ($afterCount -lt $baseline) {
    Write-Warning "⚠ 保護數量從 $baseline 減少為 $afterCount！"
    & $Python scripts/restore_changeToFloat.py
}
```

---

## 多人整合順序

逐一合併，每人完成後編譯驗證：

```
Steven(base) → +Jimmy → +RogerYang → +Ifor → ...
```

每一輪合併後必須：
1. 通過 BCB6 編譯（0 errors）
2. 產生該開發者的合併報告
3. 確認功能正常後再進行下一位

---

## SVN Commit 訊息範本

```text
V3.33.899.0 多人版本整合

整合 RogerYang V3.33.898.1_20260317:
- 合入 73 個檔案異動（56 cpp / 12 h / 5 dfm）
- [功能群組摘要1]
- [功能群組摘要2]

整合 KevinCheng V3.33.896.0_20260206:
- 合入 NV Contact Test 相關功能
- [功能群組摘要]

保留 Steven 20260316 Bug Fix:
- [修正內容摘要]

驗證結果:
- BCB6 full rebuild 成功
- 0 errors, XX warnings
- Pre-release check PASS
```
