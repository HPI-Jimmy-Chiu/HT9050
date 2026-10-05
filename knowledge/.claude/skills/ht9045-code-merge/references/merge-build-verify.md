# BCB6 編譯驗證 — Step 5–6 詳細參考

> 摘自原 ht9045-merge-build-verify 技能。主 Skill 見 [ht9045-code-merge/SKILL.md](../SKILL.md)。

## Step 4.5：Layer 1 靜態驗證（編譯前必做）

> **設計建置**：編譯只驗語法，不驗語義。這些靜態規則捕捉編譯透明的錯誤，必須在編譯前執行。

```powershell
# 在 Step 5 編譯前執行以下、全部通過後才進入編譯

# L1-1：MachineType.h 重複 #define 檢查
$lines = Get-Content "$Target\MachineType.h" -Encoding Default
$dupes = $lines | Select-String '^\s*#define\s+(\w+)' |
    ForEach-Object { $_.Matches[0].Groups[1].Value } |
    Group-Object | Where-Object Count -gt 1
if ($dupes) { Write-Warning "⚠ L1-1 失敗：重複 #define $($dupes.Name -join ', ')" }
else { Write-Host "L1-1 通過：無重複 #define" }

# L1-2：CC_ 客戶代碼順序檢查
$nums = $lines | Select-String '^\s*#define\s+CC_\w+\s+(\d+)' |
    ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
for ($i=1; $i -lt $nums.Count; $i++) {
    if ($nums[$i] -lt $nums[$i-1]) {
        Write-Warning "⚠ L1-2 失敗：CC_ 順序錯誤 ($($nums[$i-1]) > $($nums[$i])）" }
}
if ($nums.Count -gt 0) { Write-Host "L1-2 通過：CC_ 領域順序正確" }

# L1-3：MachineType.h 關鍵符號存在性（source==base 邊界案例防護）
@("SortOfsTotal","eSortOffset","SortArmOffSet") | ForEach-Object {
    if ((Select-String $_ "$Target\MachineType.h").Count -eq 0) {
        Write-Warning "⚠ L1-3 失敗：MachineType.h 中 $_ 遺失！請還原備份"
    } else { Write-Host "L1-3 通過：$_ 存在" }
}

# L1-4：SOFT_SIMULTE 開發旗標檢查（必須為注解狀態）
$devFlags = @('SOFT_SIMULTE','SOFT_SIMULTE_EtherCAT') + (
    Get-ChildItem "$Target\*.h" "$Target\*.cpp" -Recurse |
        Select-String 'BETA_|TEST_|DEV_' | ForEach-Object { $_.Matches.Value } |
        Sort-Object -Unique)
foreach ($flag in $devFlags) {
    $active = Select-String "^\s*#define\s+$flag\b" "$Target\*.h" -ErrorAction SilentlyContinue
    if ($active) { Write-Warning "⚠ L1-4 失敗：$flag 為啟用狀態，將影響正式機台！" }
}
Write-Host "L1-4 通過：開發旗標檢查完成"

# L1-5：新增 .cpp 已在 .bpr 中登記
# 注意：$Target 是 svn export 產生的目錄（無 .svn），不可對它執行 svn status。
# 改對每個來源工作複本（$SourcePaths）執行 svn status，找出 A 狀態的 .cpp，
# 再確認這些檔案已登記在 Target 的 HT9045.bpr 中。
# $SourcePaths 請在 Phase 0b 時填入，例如：
#   $SourcePaths = @("D:\HT9045\HT9011UC_Code_V3.33.897.1_RogerYang")
if (-not $SourcePaths) {
    Write-Warning "⚠ L1-5 略過：\$SourcePaths 未設定，請手動確認所有新增 .cpp 均已加入 HT9045.bpr"
} else {
    foreach ($src in $SourcePaths) {
        $addedCpps = svn status $src | Select-String '^A\s+.*\.cpp$' |
            ForEach-Object { Split-Path ($_.Line -replace '^A\s+','') -Leaf }
        if (-not $addedCpps) {
            Write-Host "L1-5 通過：$src 無新增 .cpp"
            continue
        }
        foreach ($fname in $addedCpps) {
            if ((Select-String ([regex]::Escape($fname)) "$Target\HT9045.bpr").Count -eq 0) {
                Write-Warning "⚠ L1-5 失敗：$fname（來自 $src）未在 HT9045.bpr 中"
            } else { Write-Host "L1-5 通過：$fname 已在 .bpr" }
        }
    }
}
```

> **任何 L1-x 失敗都必須修復後才能進入 Step 5 編譯**。

---

## Step 5：BCB6 編譯驗證

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"
# 推薦：使用安全編譯腳本
& $Python scripts/build_verify_safe.ps1 `
    -TargetPath "<TargetPath>" `
    -BprFile   "HT9045.bpr" `
    -LogPath   "<build_log_path>"
```

手動執行：
```powershell
$env:BCB = "D:\ProgramFiles\Borland\CBuilder6"
$env:PATH = "$env:BCB\Bin;" + $env:PATH
Push-Location "<TargetPath>"
bpr2mak HT9045.bpr
make -f HT9045.mak 2>&1 | Tee-Object -FilePath "<build_log_path>"
Pop-Location
```

### 常見編譯錯誤與解決方案

| 錯誤類型 | 原因 | 解決方案 |
|----------|------|----------|
| `Unable to open include file` | 新增的標頭檔未複製 | 從來源複製遺漏的檔案 |
| `Unresolved external` | 新增的 .cpp 未加入專案 | 修改 .bpr 加入檔案 |
| `Call to undefined function` | 函數宣告遺失 | 在 cprod.h 或對應 .h 加入宣告 |
| `Undefined symbol` | 全域變數未宣告/定義 | 見下方「缺失全域變數修復流程」|
| `Declaration syntax error` | 3-way merge 語法錯誤 | 手動檢查修復語法 |
| `Declaration terminated incorrectly` | 函式結構被破壞（if 在花括號外） | 恢復原版後手動合併 |
| `E2148 default argument value redeclared` | 同函式在兩個 .h 都帶 default argument | 保留一處，另一處移除 |
| `Fatal: Unable to open file 'MAKE0000.@@@'` | `make` 不在專案目錄執行 | 切回 .bpr/.mak 所在路徑後重跑 |

### 缺失全域變數的標準修復流程

當出現 `Undefined symbol 'XXX'` 時：

```powershell
# 1. 在來源版本搜尋變數定義
Select-String "int\s+XXX|extern.*XXX" "<SourcePath>\*.cpp" "<SourcePath>\*.h"

# 2. 在目標版本 cmydef.h 加入 extern 宣告
# 格式：extern int  USE_LdUldCassetteMode ; //Frank 20251217 add

# 3. 在 cmydef.cpp 加入變數定義
# 格式：int  USE_LdUldCassetteMode  =0;  //Frank 20251217 add

# 4. 確認 database.cpp 中是否有從 INI 讀取的邏輯需要複製
Select-String "XXX.*CheckAndReadIniData" "<SourcePath>\database.cpp"
```

### 函式邊界語法錯誤模式

```cpp
// ❌ 錯誤（3-way merge 產生，if 在花括號外）
double GetArmX(int iX, int iY)
    if(iX<0||iX>=4||iY<0||iY>=8) return 0;
{
    // 函式內容...
}

// ✅ 正確
double GetArmX(int iX, int iY)
{
    if(iX<0||iX>=4||iY<0||iY>=8) return 0;
    // 函式內容...
}
```

### Build Log 解析

```powershell
# 計算 errors / warnings
Select-String -Path "<log>" -Pattern "Error E\d+"   | Measure-Object
Select-String -Path "<log>" -Pattern "Warning W\d+" | Measure-Object
```

**可忽略的警告**：`W8058`（precompiled header）、`W8004`（unused variable）、`W8080`（declared but never used）、`W8012`（signed/unsigned）、Public symbol defined in both module。

---

## Step 5.5：上線前風險檢查（Pre-Release Check）

BCB6 編譯通過後，**在產生報告前**，必須對本次合併涉及的所有修改檔案執行上線前風險掃描。

使用 `pre-release-check` 技能，載入方式：

```
@HT9045 對 <目標版本路徑> 執行 pre-release check
```

### 掃描範圍

本次合併異動的所有 `.cpp` / `.h` / `.dfm` 檔案（可從 Step 1 的 SVN diff 清單取得）。

### 掃描項目

| 類別 | 說明 |
|------|------|
| P1–P6 風險模式 | 裸 enum 條件、鏈式 `||`/`&&` 遺漏比較子、多階段迴圈回歸、Save/Reload 一致性、條件式誤用變數、除以零 |
| F1–F12 格式規則 | 依 `pre-release-check/references/formatting-standards.md` |
| .dfm 解析度 | 畫面在 1280×1024 與 1920×1080 下皆不超出框架 |

### 輸出格式

```
File | Line | Pattern | Severity | Suggested Fix
```

> 若無 findings，明確標記為 **clean**。有 findings 必須修正後才能進入 Step 6。

---

## Step 6：報告產生

> 報告格式、Section 1–9 結構、命名規則詳見：
> **[make-report-skill → 合併報告（Merge Report）](d:\.github\skills\make-report-skill\references\merge-report\merge-report.md)**

### 快速摘要

| 項目 | 值 |
|------|----|
| 格式 | MD only，不輸出 HTML，無 Logo |
| 輸出路徑 | `<入口網站 repo>\public\Docs\MergeReport\<年度>\MergeReport_<yyyyMMdd>_<Developer1>[_<Developer2>].md` |
| 必填 Section | Section 8 功能來源追蹤、Section 9 SVN Commit Message |
| SVN Commit | 報告與程式碼同一筆 commit，不可分開 |

---

## 錯誤處理

### Revision 差距建議

| 差距 | 建議 |
|------|------|
| ≤ 3 | 可嘗試自動 3-way merge |
| 4–10 | 複雜檔案改用手動合併 |
| > 10 | 所有重疊檔案都手動合併 |

### SVN 常見錯誤

```
svn: E155021: This client is too old...  →  更新 SVN client 或 svn upgrade
svn: E155004: Working copy locked       →  svn cleanup "<path>"
```

### 編譯錯誤排查

持續出現 Unresolved external：
1. 確認來源版本是否有新增 .cpp
2. 確認 .bpr 已加入新檔案
3. 重新執行 `bpr2mak`

### 3-way Merge 產生語法錯誤

1. 檢查衝突區塊是否正確解決
2. 確認函數大括號配對正確
3. 確認 return 語句位置正確

---

## 實際案例參考

### 案例：RogerYang → Steven（2026-04-08，Revision 差距 3）

| 類別 | 數量 |
|------|------|
| 直接複製 | 31 |
| 3-way 合併 | 13 |
| 新增檔案 | 4 |

主要問題：
1. `main.cpp` `ProcessARTMessage()` — 3-way merge 產生括號結構損壞，手動輸入樣式修復
2. `ainarm9045.cpp` `MoveInArmXYToLoader_9045()` — 同上，手動修復
3. `CosFunction.cpp` Both-Added 衝突 — `FUNC_CC_Carsem_Thai()` 和 `FUNC_CC_Mellanox_Israel()` 各並入一份
4. `MachineType.h` 重複 #define — `CC_Carsem_Thai 731` 和 `CC_Mellanox_Israel 751` 各對应兩個 #define，從 Both-Added 類別漏網進入 SVN

**轉化為預防規則**：
- `both-added` 合併 `MachineType.h` 後必做：執行 L1-1（重複 #define）+ L1-2（CC_ 順序）
- 3-way merge HIGH 風險檔案（`main.cpp`、`ainarm9045.cpp`）止從上次生算法已加入 `--risk-check HIGH` 強制手動合併流程

### 案例：Kevin Cheng → Steven（2026-03-20，Revision 差距 10）

| 類別 | 數量 |
|------|------|
| 直接複製 | 28 |
| 3-way 合併 | 12 |
| 需手動修復 | 4 |

主要遇到問題：
1. `cprod.h` — GetArmX/Y 邊界檢查放在花括號外（4 處手動移入）
2. `USE_LdUldCassetteMode` 未定義，補 cmydef.h/cpp
3. `acatchtray.cpp` 3-way 產生 22 個語法錯誤 → svn revert 手動插入
4. `main.cpp` 多個 Illegal structure operation → svn revert 手動插入

**轉化為預防規則**：
- `cprod.h` 貧雜逻輯檔案列入 HIGH 風險，下次強制手動合併
- Revision 差距 > 5 的所有重疊檔案從上次起即重評風險

### 案例：Jimmy → 9045AU（2026-03-30）

主要教訓：`MachineType.h` 的 `eSortOffset`/`SortOfsTotal` 因 source==base 邊界案例遺失 → 71 個編譯錯誤。

**轉化為預防規則**：`MachineType.h` 合併後必做 L1-3（關鍵符號存在性）。

### 合併文件類型注意事項

| 檔案類型 | 建議合併方式 | 原因 |
|----------|--------------|------|
| 簡單 .h 檔案 | 自動 3-way | 通常只有宣告變更 |
| 設定相關 .cpp | 自動 3-way | 修改集中在特定區塊 |
| 複雜邏輯 .cpp | 手動合併 | 多處分散修改易破壞結構 |
| 狀態機/流程控制 | 手動合併 | switch/case 結構敏感 |
| .dfm 表單檔 | 直接複製目標 | 結構化格式不適合合併 |
| MachineType.h（9046AU） | 3-way + 關鍵符號驗證 | source==base 邊界案例 |
