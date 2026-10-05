# HT9045 程式碼合併詳細工作流程

## 前置條件

- SVN client 已安裝（`svn` 命令可用）
- Python 3.x（完整路徑：`C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe`）
- BCB6 安裝於 `D:\ProgramFiles\Borland\CBuilder6`
- 目標版本為從 SVN export 的乾淨版本（不使用個人工作複本）

---

## 完整工作流程

### Phase 0a：建立乾淨目標版本

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"

# 1. 取得最新 SVN Revision 號
svn info "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20" | Select-String "Revision"

# 2. 匯出乾淨版本（不含 .svn 目錄）
$Rev = <查到的 REV>
$Date = Get-Date -Format "yyyyMMdd"
$Authors = "Steven_RogerYang"   # 依實際參與者填寫
$TargetDir = "D:\HT9045\HT9011UC_Code_V3.33.${Rev}.0_${Date}_${Authors}"
svn export -r $Rev "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20" $TargetDir
```

> 目標版本資料夾即本次所有來源合入的「共同目的地」，整個合併過程不更換。

---

### Phase 0b：SVN 狀態檢查與自動修復

若開發者的工作複本 `.svn` 資料夾遺失或損毀：

```powershell
# 檢查狀態
& $Python scripts/svn_helper.py check --path "<工作複本路徑>"

# 自動修復（從資料夾名稱解析 Revision）
& $Python scripts/svn_helper.py repair --path "<工作複本路徑>"

# 手動指定 Revision 修復
& $Python scripts/svn_helper.py repair --path "<工作複本路徑>" --revision 897
```

**資料夾命名規則**：
```
HT9011UC_Code_V{Major}.{Minor}.{Revision}.{Patch}_{YYYYMMDD}_{Developer1}[_{Developer2}]
```

例如：`HT9011UC_Code_V3.33.901.0_20260408_Steven_RogerYang` → Revision = 901

---

### Phase 1：SVN 分析

```powershell
# 確認來源與目標路徑
# 目標版本己由 Phase 0a svn export 建立
$Source = "D:\HT9045\HT9011UC_Code_V3.33.900.0_20260407_<Developer>"
$Target = "D:\HT9045\HT9011UC_Code_V3.33.901.0_20260408_Steven_RogerYang"
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"

# 確認 SVN 版本
svn info $Source | Select-String "Revision|URL"
svn info $Target | Select-String "Revision|URL"

# 列出修改檔案
svn status $Source | Select-String "^M"
svn status $Target | Select-String "^M"
```

---

### Phase 2：重疊偵測

```powershell
# 若 source revision < target revision，加上 --check-cross-rev 旗標
& $Python scripts/overlap_analysis.py --source $Source --target $Target --check-cross-rev
```

輸出分類：
- **直接複製**：目標未修改，且跨版本比對確認安全
- **跨版本 3-way merge**：目標未修改，但在兩版之間有 committed 變更
- **3-way merge**：兩邊都修改，需取 SVN base 合併
- **新增檔案**：來源有但目標沒有（未版控）
- **跳過**：.bpr/.bpf/.res 等專案檔

腳本同時輸出 `ChangeToFloatNonPcnt` 基線計數，供 Phase 4 合併完成後驗收使用。

---

### Phase 3：SVN Base 提取（僅重疊檔案）

```powershell
$svnUrl = (svn info $Source | Select-String "^URL:").Line -replace "^URL:\s*", ""

foreach ($rel in $overlapFiles) {
    $fileUrl = "$svnUrl/$($rel -replace '\\','/')"
    $baseRev = (svn info "$Source\$rel" | Select-String "Last Changed Rev").Line -replace ".*:\s*", ""
    svn cat -r $baseRev $fileUrl > "$BaseDir\base_$rel"
}
```

---

### Phase 4：合併執行

#### 直接複製

```powershell
foreach ($rel in $directCopyFiles) {
    # 備份目標
    Copy-Item "$Target\$rel" "$Target\$($rel).pre_merge.bak" -Force
    # 複製來源
    Copy-Item "$Source\$rel" "$Target\$rel" -Force
}
```

#### 跨版本 3-way Merge

```powershell
# base 已在 Phase 2 --check-cross-rev 時由 svn cat -r $sourceRev 存入暫存
foreach ($rel in $crossRevMergeFiles) {
    $base = "$TempDir\crossrev_$($rel -replace '\\','_')"
    & $Python scripts/three_way_merge.py `
        --base   $base `
        --source "$Source\$rel" `
        --target "$Target\$rel" `
        --output "$Target\$rel"
}
```

#### 3-way Merge

```powershell
foreach ($rel in $overlapFiles) {
    # 備份目標
    Copy-Item "$Target\$rel" "$Target\$($rel).pre_merge.bak" -Force

    # 執行 3-way merge
    & $Python scripts/three_way_merge.py `
        --base "$BaseDir\base_$rel" `
        --source "$Source\$rel" `
        --target "$Target\$rel" `
        --output "$Target\$rel"
}
```

#### ChangeToFloatNonPcnt 驗收

```powershell
# Phase 2 已記錄基線數量，合併完成後驗收
$afterCount = (Select-String "ChangeToFloatNonPcnt" "$Target\*.cpp" -Recurse | Measure-Object).Count
if ($afterCount -lt $baseline) {
    Write-Warning "⚠ ChangeToFloatNonPcnt 數量從 $baseline 減少為 $afterCount！請立即執行還原腳本。"
    & $Python scripts/restore_changeToFloat.py --project $Target
}
```

#### 新增檔案處理

```powershell
foreach ($rel in $newFiles) {
    # 複製檔案
    Copy-Item "$Source\$rel" "$Target\$rel" -Force
    
    # 如果是 .cpp，加入專案
    if ($rel -match '\.cpp$') {
        & $Python scripts/bpr_updater.py add --bpr "$Target\HT9045.bpr" --file "$Target\$rel"
    }
}
```

---

### Phase 5：空白標準化

```powershell
& $Python scripts/whitespace_normalize.py $Target --extensions .cpp .h .c
```

處理項目：
- Tab → 4 個空格
- 移除行尾空白
- 刪除檔案末尾的空白行
- 壓縮連續空白行

**重要**：使用 CP950 編碼讀寫！

---

### Phase 6：BCB6 編譯

```powershell
$env:BCB = "D:\ProgramFiles\Borland\CBuilder6"
$env:PATH = "$env:BCB\Bin;" + $env:PATH

Push-Location $Target
bpr2mak HT9045.bpr
make -f HT9045.mak 2>&1 | Tee-Object -FilePath "build.log"
Pop-Location
```

#### 常見問題處理

| 錯誤 | 原因 | 解決 |
|------|------|------|
| `Unable to open include file 'xxx.h'` | 標頭檔未複製 | 從來源複製檔案 |
| `Unresolved external 'xxx'` | .cpp 未加入專案 | 用 bpr_updater.py 加入 |
| `Call to undefined function` | 函數未宣告 | 在 cprod.h 加入宣告 |
| `Declaration syntax error` | merge 語法錯誤 | 手動修復 |

---

### Phase 7：報告產生

```powershell
# 產生 HTML 版本（選用）
& $Python scripts/md2html.py "report.md" "report.html"
```

報告位置：`<入口網站 repo>\public\Docs\MergeReport\<年度>\MergeReport_<YYYYMMDD>_<Developer1>[_<Developer2>].md`

---

## 衝突解決原則

1. **兩邊修復相同問題**：選擇更完整的版本
2. **修改不同功能**：嘗試合併兩邊
3. **無法判斷**：保留目標（Steven）版本，記錄待確認

---

## 多人整合順序

**原則**：每位開發者各為一個「來源版本」，全部合入同一個由 Phase 0a 建立的「目標版本」。

```
[SVN export] → 目標版本
  ↓ 來源一（第一輪：Step 0~4.5）
  ↓ 來源二（第二輪：Step 0~4.5）
  ↓ ...
  ↓ [Step 5 編譯驗證] → [Step 6 報告產生] → SVN commit
```

每一輪合併後必須通過 Step 4.5 Layer 1 靜態驗證，Step 5 BCB6 編譯僅所有來源整合完畢後執行一次。

**合併順序建議**：衝突較少（直接複製多）的來源先合，較複雜的後合。
