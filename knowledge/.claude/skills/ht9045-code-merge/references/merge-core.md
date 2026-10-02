# 合併核心 — Step 3–4 詳細參考

> 摘自原 ht9045-merge-core 技能。主 Skill 見 [ht9045-code-merge/SKILL.md](../SKILL.md)。

## Step 3：檔案合併

### 直接複製

> **前提**：只有通過 Step 2 跨版本比對確認「安全直接複製」的檔案才能執行以下操作。

```powershell
foreach ($rel in $directCopyFiles) {
    Copy-Item "$Source\$rel" "$Target\$rel" -Force
}
```

### 合併前風險評估（必做）

每個進入 3-way merge 的檔案，在正式合併前先執行風險評估：

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"
& $Python scripts/three_way_merge.py `
    --base   "$BaseDir\base_$rel" `
    --source "$Source\$rel" `
    --target "$Target\$rel" `
    --output "$Target\$rel" `
    --risk-check
```

| 風險等級 | 條件 | 強制動作 |
|---------|------|--------|
| `SAFE` | 修改 ≤50 行、≤3 個區塊 | 全自動合併 + post-merge hook |
| `MEDIUM` | 修改 ≤50 行、4–10 個區塊 | 自動合併 + **人工審查輸出** |
| `HIGH` | 修改 >50 行或 >10 個區塊 | **直接手動合併**，禁止自動合併 |

> **HIGH 風險處理**：執行 `svn diff` 取差異，對照 `svn revert` 後的乾淨 target 手動插入修改，完成後執行 post-merge hook。

---

### 跨版本 3-way Merge

當 Step 2 將某檔歸類為「跨版本 3-way merge」時，base 取 **source revision 的目標檔快照**：

```powershell
$Python = "C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"
foreach ($rel in $crossRevMergeFiles) {
    $base = "$TempDir\crossrev_$($rel -replace '\\','_')"
    # base 已在 Step 2 跨版本比對時取得（svn cat -r $sourceRev）
    & $Python scripts/three_way_merge.py `
        --base   $base `
        --source "$Source\$rel" `
        --target "$Target\$rel" `
        --output "$Target\$rel"
}
```

### 一般 3-way Merge

```
Base (SVN rev N) ←→ Source（開發者修改）←→ Target（Steven 修改）
```

```powershell
& $Python scripts/three_way_merge.py `
    --base   "$BaseDir\base_$rel" `
    --source "$Source\$rel" `
    --target "$Target\$rel" `
    --output "$Target\$rel"
```

> 取得 Base 版本：`svn cat "<WorkingCopy>\file.cpp" -r BASE > "<TempDir>\base_file.cpp"`

### 衝突處理原則

- **0 衝突** → 自動合併完成，但**必須執行 post-merge hook**（見下方）
- **有衝突** → 手動檢視每個衝突區塊：
  - 兩邊修復相同問題：選擇更完整的版本
  - 修改不同功能：嘗試合併兩邊
  - 無法判斷：保留目標（Steven）版本，記錄待確認
- **diff3 衝突標記格式**（使用 diff3 後端時）：
  ```
  <<<<<<< target
  ... Steven 的版本 ...
  ||||||| SVN_BASE
  ... 共同 base ...
  =======
  ... 開發者的版本 ...
  >>>>>>> source
  ```

### 假衝突識別（False Conflict）

衝突標記兩側內容完全相同（行尾符號或空白差異），直接取目標版本。

### 雙邊獨立新增衝突（Both-Added Conflict）

兩邊各自在相同位置新增不同程式碼（Base 中此處不存在）。

**處理原則**：
- 若兩邊新增**不同內容**（如不同客戶代碼）：**兩邊都合入**，但必須人工確認無重複
- 若兩邊新增**相同內容**：保留一份，嚴禁重複

**⚠ post-merge hook 強制執行**（Both-Added 後驗，合併後立即執行）：

```powershell
# 重複 #define 檢查（MachineType.h 必做）
$lines = Get-Content "$Target\MachineType.h" -Encoding Default
$defines = $lines | Select-String '^\s*#define\s+(\w+)' | ForEach-Object { $_.Matches[0].Groups[1].Value }
$dupes = $defines | Group-Object | Where-Object Count -gt 1
if ($dupes) {
    Write-Warning "⚠ 重複 #define：$($dupes.Name -join ', ')  請立即移除！"
}

# CC_ 客戶代碼數字順序檢查（MachineType.h 必做）
$ccLines = $lines | Select-String '^\s*#define\s+CC_\w+\s+(\d+)'
$nums = $ccLines | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
for ($i=1; $i -lt $nums.Count; $i++) {
    if ($nums[$i] -lt $nums[$i-1]) {
        Write-Warning "⚠ CC_ 代碼順序錯誤（第 $($i+1) 項 $($nums[$i]) < 第 $i 項 $($nums[$i-1])）"
    }
}
```

> **根因**：舊版 difflib 後端對純插入（old_count=0）的 overlap 偵測有 bug，會靜默產生重複內容而不報衝突。新版已修正，但 Both-Added 後驗仍是必要步驟。（案例：2026-04-08 RogerYang 合併，`MachineType.h` 出現兩個重複 `#define CC_Carsem_Thai 731` 和 `#define CC_Mellanox_Israel 751`）

### 開發用旗標注意事項

來源開發者可能啟用測試旗標，**不應合入正式機台版本**：

```cpp
// ❌ 來源（開發者）取消注解 → 合併時保留目標版本（注解狀態）
#define SOFT_SIMULTE
```

一律保留目標版本的旗標：`SOFT_SIMULTE`、`SOFT_SIMULTE_EtherCAT`、`BETA_*`、`TEST_*`、`DEV_*`

### difflib「source == base」邊界案例

**問題**：Source 未修改某區段（source == base），但 Target 有新增時，difflib 可能錯誤選 base 版本，丟失 Target 的新增。

**典型案例**（2026-03-30 Jimmy → 9045AU）：`MachineType.h` 的 `eSortOffset`/`SortOfsTotal` enum 遺失 → 71 個編譯錯誤。

**必做驗證**（合併 `MachineType.h` 後立即執行）：
```powershell
@("SortOfsTotal","eSortOffset","SortArmOffSet") | ForEach-Object {
    if ((Select-String $_ "$Target\MachineType.h").Count -eq 0) {
        Write-Warning "⚠ MachineType.h 中 $_ 遺失！還原備份"
        Copy-Item "$Target\MachineType.h.pre_*_merge.bak" "$Target\MachineType.h" -Force
    }
}
```

### 3-way Merge 結構破壞問題

症狀：`Declaration terminated incorrectly`、大括號不配對、return 語句位置錯誤。

對以下特徵的檔案改用手動精確合併：
- 修改行數 > 50 行
- 修改分散在多個函式中
- 涉及控制流程結構 (if/for/while/switch)

```powershell
# 手動合併流程
svn revert "<TargetPath>\complex_file.cpp"   # 恢復目標檔案到原版
svn diff "<SourcePath>\complex_file.cpp"      # 查看來源差異
# 根據 diff 輸出手動在目標版本插入修改
```

### .dfm 表單檔處理

雙邊衝突時直接複製目標（HT-9046AU 版面佈局優先），不進行 3-way merge。

### difflib 已知瑕疵（2026-04-17 合併實證）

#### 瑕疵 A：if-body 錯位（鄰近無大括號 if 的插入）

**問題**：當 source 與 target 都在無大括號 `if` 語句後方插入新行時，difflib 可能將原始 if body 與新插入行互換位置。

**典型案例**（uYieldMonitoring.cpp，L1079-1085）：

```cpp
// ❌ difflib 合併結果（WRONG）
if(CosFunction.bUseLowYieldAlarmByBin==false)
TestIF_File.bSlidingWindowYield  =ReadIniData(...);  // ← 變成 if body（錯！）
TestIF_File.iSlidingWindowSize   =ReadIniData(...);
    TestIF_File.bLowYieldAlarmByBin=false;               // ← 原 if body 被推到這裡（孤立！）

// ✅ 正確結果
if(CosFunction.bUseLowYieldAlarmByBin==false)
    TestIF_File.bLowYieldAlarmByBin=false;               // ← if body
TestIF_File.bSlidingWindowYield  =ReadIniData(...);      // ← 新增行在 if 之後
TestIF_File.iSlidingWindowSize   =ReadIniData(...);
```

**根因**：difflib 對 `unified_diff(base, source/target, n=0)` 產生的 hunk 邊界不感知 C++ 語法結構。無大括號 if 的 body 只有一行，difflib 會把 hunk 切在 if 條件行之後，導致插入行取代了原始 body。

**偵測方法**：單純搜尋 `if(` 後方的行會有大量誤報（行號偏移、`{` 在註解內等）。正確偵測方式必須**同時比對 Steven 基線**：

```python
# 精確偵測 if-body 錯位的方法
# 1. 在 target 找到 if(...) 行（無 { 在行末）
# 2. 在 Steven 找到相同的 if 文字
# 3. 比較兩邊的下一行是否相同
# 4. 若不同 → 可能是錯位
# 注意：必須用 SequenceMatcher 對齊行號，不能直接用行號比較
```

**實測結果**（2026-04-17）：全專案 573 個原始碼檔案掃描，`uYieldMonitoring.cpp` 發現兩個 Defect A 實例：

1. **L1079-1085 if-body 錯位**：無大括號 `if(bUseLowYieldAlarmByBin==false)` 的 body 與新插入的 `bSlidingWindowYield` 行互換。
2. **L1985-1986 for-loop body 分裂**：`for(int i=0; i<TEST_MAX_BIN; i++)` 迴圈內的 JerryYang 新增行（bFailCountEnable/iFailCountLimit）被 difflib 包裹成獨立的 `for(int i=0; i<16; i++)` 迴圈，導致外層 for 迴圈缺少 `}`，編譯錯誤 E2089。

⚠️ **重要**：Build 8 因此案例 2 失敗（前 7 輪未觸發），因 Build 7 是連結階段修復，Build 8 才觸發了 uYieldMonitoring.cpp 的完整重編譯。此瑕疵在單純搜尋 `if(` 時不會被偵測到——它發生在 for 迴圈的 hunk 邊界，需要比對 Steven 基線的大括號深度才能發現。

#### 瑕疵 B：空白行膨脹

**問題**：difflib 在跨版本 3-way merge 時，會在 hunk 邊界處多複製空白行，導致合併結果比任何來源版本都多空白行。

**典型案例**（uYieldMonitoring.cpp）：Target 比 Steven 多 59 個空白行，全部散落在 hunk 邊界。

**實測結果**（2026-04-17）：全專案 62 個被修改的檔案中，有 8 個出現空白行膨脹（+1~+59），其中可用 SequenceMatcher 自動修復的佔 3 個（main.h +2、ainarm9045_All_1Pick.cpp +2、uYieldMonitoring.cpp +59），其餘的多餘空白行夾在 Roger 新增程式碼中屬正常。

**根因**：difflib 的 hunk `added` 列表包含原始空白行，而逆序套用多個 hunk 時，相鄰 hunk 的空白行被重複保留。

**修復方法**：合併後用 Steven（目標基線）對比，若某區塊的非空白行完全一致、僅空白行較多，則還原 Steven 版本的空白行格式：

```python
# 修復 difflib 空白行膨脹的 Python 腳本概念
sm = difflib.SequenceMatcher(None, steven_lines, target_lines)
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag != 'equal':
        s_nonblank = [l for l in steven[i1:i2] if l.strip()]
        t_nonblank = [l for l in target[j1:j2] if l.strip()]
        if s_nonblank == t_nonblank and t_blanks > s_blanks:
            # 只有空白行差異 → 用 Steven 版本
            result.extend(steven[i1:i2])
        else:
            result.extend(target[j1:j2])
```

**建議**：在 Step 4 空白標準化中加入此步驟，或將腳本加入 post-merge hook。

### 新增檔案處理

1. 複製檔案到目標
2. 修改 `.bpr` 專案檔，加入 OBJFILES 和 FILE 區段
3. **立即驗證新增 .cpp 已在 .bpr 中**（不依賴編譯隱性確認）：

```powershell
# 例：確認 asortarm.cpp 和 myEthercatmotor.cpp 已加入
$newCpps = @('asortarm.cpp', 'myEthercatmotor.cpp')  # 替換為實際新增檔案
foreach ($f in $newCpps) {
    if ((Select-String $f "$Target\HT9045.bpr").Count -eq 0) {
        Write-Warning "⚠ $f 未在 HT9045.bpr 中，請手動加入！"
    } else {
        Write-Host "✓ $f 已在 .bpr 中"
    }
}
```

4. 確認 .h 檔有對應的 `#include` 路徑（若需要）

---

## Step 4：空白標準化

所有修改過的 .cpp/.h/.c/.asm/.dfm/.rc 檔案：

- Tab → 4 個空格
- 移除行尾空白
- 刪除檔案末尾空白行
- 連續兩個以上空白行壓縮為一個
- **必須使用 CP950 編碼讀寫**

```powershell
& $Python scripts/whitespace_normalize.py "<TargetPath>" --extensions .cpp .h .c
```

---

## Big5 編碼規則

- 讀寫 .cpp/.h/.dfm/.rc/.c/.asm 一律使用 CP950
- PowerShell：`[System.Text.Encoding]::GetEncoding(950)`
- Python：`open(path, 'r', encoding='cp950', errors='replace')`
- **禁止將 Big5 檔案另存為 UTF-8**
- 報告文件（.md/.html）使用 UTF-8

---

## 備份機制

- 3-way merge 前：備份目標版本為 `<filename>.pre_<developer>_merge.bak`
- 來源版本資料夾保持完整不修改
- 備份檔案在確認合併成功後可刪除

---

## 腳本清單

| 腳本 | 說明 |
|------|------|
| `scripts/merge_main.py` | 主程式（完整合併流程） |
| `scripts/svn_helper.py` | SVN 狀態檢查與自動修復 |
| `scripts/overlap_analysis.py` | 重疊偵測（含跨版本比對） |
| `scripts/three_way_merge.py` | 3-way merge：優先 diff3 後端，fallback difflib；含 post-merge hook |
| `scripts/whitespace_normalize.py` | 空白標準化 |
| `scripts/bpr_updater.py` | .bpr 專案檔更新 |
| `scripts/restore_changeToFloat.py` | ChangeToFloatNonPcnt 保護還原（緊急補救） |

### three_way_merge.py 後端說明

| 後端 | 優先順序 | 觸發條件 |
|------|---------|--------|
| `diff3` (Git for Windows) | 優先使用 | `C:\Program Files\Git\usr\bin\diff3.exe` 存在 |
| `difflib` | Fallback | diff3 找不到時自動切換 |

diff3 優點：Both-Added 自動產生明確衝突標記（不靜默重複）、bytes 層面操作（Big5 透明）。

**新增參數**：

| 參數 | 說明 |
|------|------|
| `--risk-check` | 評估合併風險等級後退出（SAFE/MEDIUM/HIGH），不執行合併 |
| `--source-label` | 衝突標記中的 source 說明文字 |
| `--target-label` | 衝突標記中的 target 說明文字 |

**退出碼**：`0`=乾淨合併、`1`=有衝突、`2`=post-merge hook 發現問題
