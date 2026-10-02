# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


# Search Division — 除法安全性掃描與替換

## 概述

四階段工作流程：**搜尋** → **過濾** → **替換** → **報告**，將 C/C++ 原始碼中有除以零風險的除法運算替換為安全模板。

## 四階段流程速覽

| 階段 | 名稱 | 輸入 | 輸出 | 腳本 |
|------|------|------|------|------|
| 1 | 搜尋除法位置 | `*.cpp` 原始碼 | `division_candidates.csv` | `find_divisions.ps1` |
| 2 | 上下文分析與風險過濾 | `division_candidates.csv` | `final_risky.csv` | inline PowerShell |
| 3 | 產生替換對並套用 | `final_risky.csv` | `replacement_pairs.csv` + 修改後的 .cpp | `generate_replacements.ps1` + `apply_replacements.ps1` |
| 3.5 | 替換後驗證與自動修復 | 修改後的 .cpp | 修復後的 .cpp | `verify_replacements.ps1` |
| 4 | 產出報告 | 所有中繼 CSV | `除法安全替換報告_*.md` | inline PowerShell |

## 安全模板

```cpp
// 定義於 MachineType.h
template <class T>
T ChangeToFloatNonPcnt(T Numerator, T Denominator) {
    return (Denominator == 0) ? 0 : (double)Numerator / (double)Denominator;
}
```

除數為 0 時回傳 0，否則回傳 `(double)Numerator/(double)Denominator`。

## 保護偵測規則摘要

| 保護模式 | 判定 | 結果 |
|----------|------|------|
| `if(var!=0)` / `if(var>0)` | 上下文 20 行內有條件檢查 | 安全，排除 |
| 三元運算子 `(var==0)?0:(a/var)` | 同行或上下文有三元防護 | 安全，排除 |
| 已使用 `ChangeToFloat*` | 同行已包含安全模板呼叫 | 安全，排除 |
| 除數被賦值為非零常數 | 如 `dAcc=1;` 後接 `/dAcc` | 安全，排除 |

## 完整腳本與詳細說明

所有 PowerShell 腳本、分子反向掃描規則、已知特殊案例、驗證修復邏輯、報告範本均在：

→ [references/scripts.md](references/scripts.md)

## 使用步驟

1. 讀取 [references/scripts.md](references/scripts.md) 取得完整腳本
2. 將 `<PROJECT_DIR>` 替換為實際專案路徑
3. 依序執行階段 1 → 2 → 3 → 3.5 → 4
4. 檢查 `replacement_report.csv` 確認無 FAIL
5. 使用 BCB6 編譯驗證

        foreach ($m in $matches2) {
            $divisor = $m.Groups[1].Value

            # 排除 C++ 關鍵字與型別
            if ($divisor -match '^(double|float|int|long|short|char|void|bool|unsigned|signed|const|static|class|struct|enum|return|if|else|for|while|do|switch|case|break|continue|goto|typedef|sizeof|new|delete|try|catch|throw|this|virtual|public|private|protected|template|typename|namespace|using|inline|extern|register|volatile|mutable|explicit|friend|operator|and|or|not|xor|auto)$') {
                continue
            }

            # 【可自訂】排除使用者指定的非零變數
            # 以下變數來自 MachineType.h（由使用者指定為非零常數），可直接略過檢查：
            # MAX_SAFE_DOOR_CNT, MAX_HATCH_DOOR_CNT,
            # MAX_ARM_Row, MAX_ARM_Col,
            # MAX_Index_Row, MAX_Index_Col,
            # MAX_SOCKET_ROW, MAX_SOCKET_COL, MAX_SOCKET_TOTAL,
            # MAX_SENSOR_ITEM, MAX_TTL_BIT, MAX_IONFAN,
            # MAX_AUTO_TRAY, MAX_TRACK, MAX_FIX_TRAY, MAX_UNLOAD_TRAY, MAX_MGZ_TRAY,
            # RecordTrayCnt
            # 範例：若要排除上述變數，解除註解以下判斷（或改為支援更多變數的 regex）：
            # if ($divisor -match 'MAX_ARM_Col|MAX_ARM_Row|MAX_SOCKET_ROW|MAX_SOCKET_COL|MAX_Index_Col') { continue }

            # 跳過已使用安全模板的行
            if ($origLine -match 'ChangeToFloat|ChangeToPercentage') { continue }

            $results += [PSCustomObject]@{
                File     = $file.Name
                FullPath = $file.FullName
                Line     = $lineNum
                Code     = $origLine.Trim()
                Divisor  = $divisor
                CodeOnly = $line.Trim()
            }
        }
    }
}

$results | Export-Csv -Path (Join-Path $outDir "division_candidates.csv") -NoTypeInformation -Encoding UTF8
Write-Host "候選除法數量: $($results.Count)"
Write-Host "`n除數變數 TOP 30:"
$results | Group-Object Divisor | Sort-Object Count -Descending |
    Select-Object -First 30 | Format-Table Name, Count -AutoSize
```

**輸出**：`search-division/division_candidates.csv`（欄位：File, FullPath, Line, Code, Divisor, CodeOnly）

### 搜尋階段的過濾邏輯

腳本在搜尋時已內建以下過濾（比起單純 `Select-String` 更精確）：

| 過濾項目 | 處理方式 |
|----------|----------|
| 字串常量中的 `/`（如路徑） | 移除 `"..."` 和 `'...'` 後再掃描 |
| 行尾註解 `//` | 截斷至 `//` 前 |
| 區塊註解 `/* */` | 跳過以 `/*` 或 `*` 開頭的行 |
| 前處理指令 `#include`/`#define` | 跳過以 `#` 開頭的行 |
| C++ 關鍵字（`sizeof` 等） | 由關鍵字清單排除 |
| 已使用安全模板的行 | 偵測 `ChangeToFloat`/`ChangeToPercentage` |

---

## 階段 2：上下文分析與風險過濾

讀取每個候選除法前後 20 行上下文，自動判斷是否已有保護。

> **用法**：修改 `$csv` 路徑後以 PowerShell inline 或存檔執行。

```powershell
$outDir = Join-Path "<PROJECT_DIR>" "search-division"
$csv = Import-Csv (Join-Path $outDir "division_candidates.csv")

# 【可自訂】排除常數除數清單
$constantDivisors = @(2,3,4,5,8,10,16,32,60,64,100,128,
    255,256,360,512,1000,1024,3600,4096,10000,65535,100000,1000000)

$riskyResults = @()
$protectedCount = 0
$constantCount = 0

foreach ($c in $csv) {
    # 排除純數字常數除數
    if ($c.Divisor -match '^\d+$' -and [int]$c.Divisor -in $constantDivisors) {
        $constantCount++
        continue
    }

    $filePath = $c.FullPath
    if (-not (Test-Path $filePath)) { continue }

    $allLines = Get-Content $filePath -Encoding Default
    $lineIdx = [int]$c.Line - 1

    # 取得前後 20 行上下文
    $startCtx = [Math]::Max(0, $lineIdx - 20)
    $endCtx   = [Math]::Min($allLines.Count - 1, $lineIdx + 20)
    $context  = ($allLines[$startCtx..$endCtx]) -join "`n"
    $divVar   = [regex]::Escape($c.Divisor)

    $isProtected = $false

    # 檢查 1：if(var != 0) / if(var > 0) / if(var < 0) 等比較
    if ($context -match "if\s*\(\s*${divVar}\s*[!><=]+\s*0") { $isProtected = $true }
    if ($context -match "if\s*\(\s*0\s*[!<>=]+\s*${divVar}") { $isProtected = $true }

    # 檢查 2：三元運算子 (var==0)?0:(...) 或 (var!=0)?(...):0
    if ($context -match "${divVar}\s*[!=]=\s*0\s*\)\s*\?")    { $isProtected = $true }

    # 檢查 3：已使用安全模板
    $divLine = $allLines[$lineIdx]
    if ($divLine -match 'ChangeToFloat|ChangeToPercentage|ChangeToFloatNonPcnt') {
        $isProtected = $true
    }

    # 檢查 4：除數在前面幾行被賦值為非零常數（如 var=1）
    $assignCtx = ($allLines[[Math]::Max(0,$lineIdx-5)..$lineIdx]) -join "`n"
    if ($assignCtx -match "${divVar}\s*=\s*[1-9]\d*\s*;") { $isProtected = $true }

    # 檢查 5：if(var>0) || 型的 OR 條件不完全保護 → 仍視為有風險
    # （此情況不設為 protected）

    if ($isProtected) {
        $protectedCount++
    } else {
        $riskyResults += $c
    }
}

$riskyResults | Export-Csv (Join-Path $outDir "final_risky.csv") -NoTypeInformation -Encoding UTF8
Write-Host "候選總數:       $($csv.Count)"
Write-Host "常數除數排除:   $constantCount"
Write-Host "已有保護排除:   $protectedCount"
Write-Host "有風險需替換:   $($riskyResults.Count)"
```

**輸出**：`search-division/final_risky.csv`（與輸入同欄位，僅保留未受保護的除法）

### 保護偵測規則

| 保護模式 | 判定依據 | 結果 |
|----------|----------|------|
| `if(var!=0)` / `if(var>0)` | 上下文 20 行內有條件檢查 | 安全，排除 |
| 三元運算子 `(var==0)?0:(a/var)` | 同行或上下文有三元防護 | 安全，排除 |
| 已使用 `ChangeToFloat*` | 同行已包含安全模板呼叫 | 安全，排除 |
| 除數前幾行被賦值為非零常數 | 如 `dAcc=1;` 後接 `/dAcc` | 安全，排除 |
| `if(A>0 \|\| B>0)` 後 `/A` `/B` | OR 無法同時保護兩個除數 | **有風險，保留** |
| 鏈式除法 `a/b/c`（兩除數皆已保護） | 上游有 `if(b<=0) b=1` 等 | 安全，排除 |

---

## 階段 3：產生替換對並套用（generate_replacements + apply_replacements）

### 3.1 產生替換對（generate_replacements.ps1）

從 `search-division/final_risky.csv` 讀取，解析每行除法的分子與分母，產生 `OldExpr → NewExpr` 替換對。

> **用法**：修改 `$data` 路徑與 `$skipDivisors` 後執行。

```powershell
$dir  = "<PROJECT_DIR>"
$outDir = Join-Path $dir "search-division"
$data = Import-Csv (Join-Path $outDir "final_risky.csv")

# 【可自訂】排除編譯時常數除數
$skipDivisors = @(
    'MAX_SAFE_DOOR_CNT','MAX_HATCH_DOOR_CNT',
    'MAX_ARM_Row','MAX_ARM_Col',
    'MAX_Index_Row','MAX_Index_Col','NEW_MAX_Index_Col',
    'MAX_SOCKET_ROW','MAX_SOCKET_COL','MAX_SOCKET_TOTAL',
    'MAX_SENSOR_ITEM','MAX_TTL_BIT','MAX_IONFAN',
    'MAX_AUTO_TRAY','MAX_TRACK','MAX_FIX_TRAY','MAX_UNLOAD_TRAY','MAX_MGZ_TRAY',
    'RecordTrayCnt'
)

$replacements = @()
$warnings = @()

foreach ($c in $data) {
    $filePath = $c.FullPath
    if (-not (Test-Path $filePath)) { continue }
    if ($c.Divisor -in $skipDivisors) { continue }

    $lineNum   = [int]$c.Line
    $divisor   = $c.Divisor
    $allLines  = Get-Content $filePath -Encoding Default
    $origLine  = $allLines[$lineNum - 1]
    $escapedDiv = [regex]::Escape($divisor)

    # 匹配完整分母（含成員存取鏈 . -> [] 等）
    $denomFullPattern = "/\s*(${escapedDiv}(?:\[[^\]]*\])?(?:\.\w+|\->\w+)*(?:\[[^\]]*\])?(?:\.\w+|\->\w+)*)"

    if ($origLine -match $denomFullPattern) {
        $fullDenominator = $Matches[1]
        $divisionMatch   = $Matches[0]
        $divPos          = $origLine.IndexOf($divisionMatch)

        # 【修正】函式呼叫除數：若分母後方緊接 (，擴展至完整函式呼叫
        $denomEnd = $divPos + $divisionMatch.Length
        if ($denomEnd -lt $origLine.Length -and $origLine[$denomEnd] -eq '(') {
            $depth = 1; $pos = $denomEnd + 1
            while ($pos -lt $origLine.Length -and $depth -gt 0) {
                if ($origLine[$pos] -eq '(') { $depth++ }
                elseif ($origLine[$pos] -eq ')') { $depth-- }
                $pos++
            }
            $funcArgs = $origLine.Substring($denomEnd, $pos - $denomEnd)
            $fullDenominator += $funcArgs
            $divisionMatch   += $funcArgs
        }

        $beforeDiv       = $origLine.Substring(0, $divPos)

        # --- 反向掃描分子 ---
        $numerator = ""
        $numEnd = $beforeDiv.Length - 1

        if ($numEnd -ge 0) {
            $ch = $beforeDiv[$numEnd]

            if ($ch -eq ')') {
                # 括號運算式：找到匹配的 (
                $depth = 1; $pos = $numEnd - 1
                while ($pos -ge 0 -and $depth -gt 0) {
                    if ($beforeDiv[$pos] -eq ')') { $depth++ }
                    elseif ($beforeDiv[$pos] -eq '(') { $depth-- }
                    $pos--
                }
                $startPos = $pos + 1
                while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }

                # 【修正】繼續往前掃描物件前綴（. -> [] 等成員存取鏈）
                # 修復 MOT[x].Method()/Divisor 形式只抓到 Method() 的問題
                while ($startPos -gt 0) {
                    $pc = $beforeDiv[$startPos - 1]
                    if ($pc -eq '.') {
                        $startPos--
                        while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }
                        if ($startPos -gt 0 -and $beforeDiv[$startPos-1] -eq ']') {
                            $startPos--; $d2 = 1
                            while ($startPos -gt 0 -and $d2 -gt 0) {
                                $startPos--
                                if ($beforeDiv[$startPos] -eq ']') { $d2++ }
                                elseif ($beforeDiv[$startPos] -eq '[') { $d2-- }
                            }
                            while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }
                        }
                    }
                    elseif ($pc -eq '>' -and $startPos -gt 1 -and $beforeDiv[$startPos-2] -eq '-') {
                        $startPos -= 2
                        while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }
                        if ($startPos -gt 0 -and $beforeDiv[$startPos-1] -eq ']') {
                            $startPos--; $d2 = 1
                            while ($startPos -gt 0 -and $d2 -gt 0) {
                                $startPos--
                                if ($beforeDiv[$startPos] -eq ']') { $d2++ }
                                elseif ($beforeDiv[$startPos] -eq '[') { $d2-- }
                            }
                            while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }
                        }
                    }
                    elseif ($pc -eq ']') {
                        $startPos--; $d2 = 1
                        while ($startPos -gt 0 -and $d2 -gt 0) {
                            $startPos--
                            if ($beforeDiv[$startPos] -eq ']') { $d2++ }
                            elseif ($beforeDiv[$startPos] -eq '[') { $d2-- }
                        }
                        while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }
                    }
                    else { break }
                }

                $numerator = $beforeDiv.Substring($startPos).Trim()
            }
            elseif ($ch -eq ']') {
                # 陣列存取：找到匹配的 [
                $depth = 1; $pos = $numEnd - 1
                while ($pos -ge 0 -and $depth -gt 0) {
                    if ($beforeDiv[$pos] -eq ']') { $depth++ }
                    elseif ($beforeDiv[$pos] -eq '[') { $depth-- }
                    $pos--
                }
                while ($pos -ge 0 -and $beforeDiv[$pos] -match '[\w\.\>]') {
                    if ($beforeDiv[$pos] -eq '>' -and $pos -gt 0 -and $beforeDiv[$pos-1] -eq '-') { $pos-- }
                    $pos--
                }
                $numerator = $beforeDiv.Substring($pos + 1).Trim()
            }
            elseif ($ch -match '[\w\d]') {
                # 一般變數 / 成員存取鏈
                $pos = $numEnd
                while ($pos -ge 0) {
                    $currentChar = $beforeDiv[$pos]
                    if ($currentChar -match '\w') { $pos-- }
                    elseif ($currentChar -eq '.') { $pos-- }
                    elseif ($currentChar -eq '>' -and $pos -gt 0 -and $beforeDiv[$pos-1] -eq '-') {
                        $pos -= 2  # 跳過 '->'
                    }
                    elseif ($currentChar -eq ']') {
                        $depth = 1; $pos--
                        while ($pos -ge 0 -and $depth -gt 0) {
                            if ($beforeDiv[$pos] -eq ']') { $depth++ }
                            elseif ($beforeDiv[$pos] -eq '[') { $depth-- }
                            $pos--
                        }
                    }
                    elseif ($currentChar -eq ')') {
                        $depth = 1; $pos--
                        while ($pos -ge 0 -and $depth -gt 0) {
                            if ($beforeDiv[$pos] -eq ')') { $depth++ }
                            elseif ($beforeDiv[$pos] -eq '(') { $depth-- }
                            $pos--
                        }
                        while ($pos -ge 0 -and $beforeDiv[$pos] -match '\w') { $pos-- }
                    }
                    elseif ($currentChar -eq '*') {
                        $pos--  # 乘法與除法同優先權，左結合
                    }
                    else { break }
                }
                $numerator = $beforeDiv.Substring($pos + 1).Trim()
            }
        }

        $numerator       = $numerator.Trim()
        $fullDenominator = $fullDenominator.Trim()

        if ($numerator -ne "" -and $fullDenominator -ne "") {
            $exactMatch = [regex]::Match($origLine,
                [regex]::Escape($numerator) + "\s*/\s*" + [regex]::Escape($fullDenominator))
            $oldExact = if ($exactMatch.Success) { $exactMatch.Value }
                        else { "${numerator}/${fullDenominator}" }

            $newExpr = "ChangeToFloatNonPcnt((double)($numerator), (double)($fullDenominator))"

            $replacements += [PSCustomObject]@{
                File         = $c.File
                FullPath     = $filePath
                Line         = $lineNum
                OldExpr      = $oldExact
                NewExpr      = $newExpr
                Numerator    = $numerator
                Denominator  = $fullDenominator
                OrigLine     = $origLine.Trim()
            }
        } else {
            $warnings += "WARN 空分子/分母: $($c.File):$lineNum"
        }
    } else {
        $warnings += "WARN 未匹配除法: $($c.File):$lineNum 除數='$divisor'"
    }
}

$replacements | Export-Csv (Join-Path $outDir "replacement_pairs.csv") -NoTypeInformation -Encoding UTF8
Write-Host "替換對數量: $($replacements.Count)"
Write-Host "警告數量:   $($warnings.Count)"
$warnings | ForEach-Object { Write-Host $_ }
```

**輸出**：`search-division/replacement_pairs.csv`（欄位：File, FullPath, Line, OldExpr, NewExpr, Numerator, Denominator, OrigLine）

#### 分子反向掃描規則

| 結尾字元 | 掃描邏輯 |
|----------|----------|
| `)` | 找匹配的 `(`，再往前掃描函式名 |
| `]` | 找匹配的 `[`，再往前掃描陣列變數及成員存取 |
| 英數字 | 往前掃描，遇 `.` `->` `[]` `()` `*`(乘法) 繼續，遇 `+ - = , ;` 停止 |

> **注意**：`-` 只在 `->` 成員存取時才包含，獨立的減號會停止掃描。`*` 因與 `/` 同優先權（左結合）會被納入分子。

#### 已知特殊案例與修正方式

以下三種模式可能產生錯誤的替換結果，已在腳本中修正，但仍需透過階段 3.5 驗證以確保完整性：

| 問題模式 | 範例 | 原因 | 修正方式 |
|----------|------|------|----------|
| 成員函式分子 | `MOT[x].Method()/Divisor` | `)` 分支只抓 `Method()` 遺失 `MOT[x].` | 分子掃描後繼續往前遍歷成員存取鏈 |
| 函式呼叫除數 | `a/abs(x-y)` | `$denomFullPattern` 只抓 `abs` 遺失引數 `(x-y)` | 分母匹配後檢查後方是否查 `(` 並擴展 |
| 空分子 | `x = /divisor` 形式 | `=` 停止掃描後分子為空 | 列入警告，需以 `$fixCases` 手動補正 |

**成員函式分子** — 已在 `)` 分支加入物件前綴掃描革新（見上方程式碼）。
**函式呼叫除數** — 已在 `$denomFullPattern` 匹配後加入函式引數擴展邏輯。
**空分子** — 仍需手動處理，將產生 WARN 警告，需在 `$fixCases` 中補充完整的分子/分母。

若仍有殘留破損，階段 3.5 的驗證腳本會自動偵測並修復。

$fixCases 字典用於手動補正反向掃描失敗的案例：

```powershell
$fixCases = @{
    # "檔名:行號" = @{ OldExpr="完整除法"; Num="完整分子"; Den="完整分母" }
    "aTester_Front.cpp:287" = @{
        OldExpr = "MOT[MTestY1].Gali_ReadEncoderPos()/MOT[MTestY1].Motor->GearRatio"
        Num     = "MOT[MTestY1].Gali_ReadEncoderPos()"
        Den     = "MOT[MTestY1].Motor->GearRatio"
    }
    # ... 其他類似案例
}
```

### 3.2 套用替換（apply_replacements.ps1）

讀取替換對 CSV，使用 Big5/CP950 編碼讀寫檔案，由下往上替換（避免行號偏移）。

> **用法**：修改 `$rep` 路徑後執行。如有需手動修正的案例，填入 `$fixCases`。

```powershell
$dir = "<PROJECT_DIR>"
$outDir = Join-Path $dir "search-division"
$rep = Import-Csv (Join-Path $outDir "replacement_pairs.csv")

# 【可自訂】手動修正案例（分子解析不完整時）
$fixCases = @{
    # "檔名:行號" = @{ OldExpr="..."; Num="..."; Den="..." }
}

$finalReplacements = @()
foreach ($r in $rep) {
    $key = "$($r.File):$($r.Line)"
    if ($fixCases.ContainsKey($key)) {
        $fix = $fixCases[$key]
        $finalReplacements += [PSCustomObject]@{
            File     = $r.File
            FullPath = $r.FullPath
            Line     = [int]$r.Line
            OldExpr  = $fix.OldExpr
            NewExpr  = "ChangeToFloatNonPcnt((double)($($fix.Num)), (double)($($fix.Den)))"
        }
    } else {
        $finalReplacements += [PSCustomObject]@{
            File     = $r.File
            FullPath = $r.FullPath
            Line     = [int]$r.Line
            OldExpr  = $r.OldExpr
            NewExpr  = $r.NewExpr
        }
    }
}

Write-Host "替換總數: $($finalReplacements.Count)"

$success = 0; $fail = 0; $report = @()
$fileGroups = $finalReplacements | Group-Object FullPath

foreach ($fg in $fileGroups) {
    $filePath = $fg.Name
    if (-not (Test-Path $filePath)) { continue }

    # 以 Big5/CP950 編碼讀取，保留原始編碼
    $rawBytes = [System.IO.File]::ReadAllBytes($filePath)
    $encoding = [System.Text.Encoding]::GetEncoding(950)
    $content  = $encoding.GetString($rawBytes)
    $lines    = $content -split "`r`n|`n"

    $modified = $false

    # 由下往上替換，避免行號偏移
    foreach ($r in ($fg.Group | Sort-Object { [int]$_.Line } -Descending)) {
        $lineIdx = [int]$r.Line - 1
        $line    = $lines[$lineIdx]

        if ($line -match [regex]::Escape($r.OldExpr)) {
            $lines[$lineIdx] = $line -replace [regex]::Escape($r.OldExpr), $r.NewExpr
            $modified = $true
            $success++
            $report += [PSCustomObject]@{
                File=$r.File; Line=$r.Line; Status="OK"; Old=$r.OldExpr; New=$r.NewExpr
            }
        } else {
            $fail++
            Write-Host "FAIL: $($r.File):$($r.Line) - '$($r.OldExpr)'"
            $report += [PSCustomObject]@{
                File=$r.File; Line=$r.Line; Status="FAIL"; Old=$r.OldExpr; New=$r.NewExpr
            }
        }
    }

    if ($modified) {
        $newContent = $lines -join "`r`n"
        $newBytes   = $encoding.GetBytes($newContent)
        [System.IO.File]::WriteAllBytes($filePath, $newBytes)
        Write-Host "已儲存: $($fg.Name) ($($fg.Group.Count) 處替換)"
    }
}

$report | Export-Csv (Join-Path $outDir "replacement_report.csv") -NoTypeInformation -Encoding UTF8
Write-Host "`n完成！成功: $success / 失敗: $fail / 總計: $($success + $fail)"
```

**輸出**：`search-division/replacement_report.csv`（欄位：File, Line, Status, Old, New）

---

## 階段 3.5：替換後驗證與自動修復（verify_replacements.ps1）

替換完成後，必須掃描所有已修改的 .cpp 檔案，偵測並自動修復兩種已知的破損模式。

> **用法**：修改 `$projectDir` 後執行。此步驟應在 Phase 3.2 套用替換後、Phase 4 報告前執行。

### 破損模式 A：成員函式誤判（`.ChangeToFloatNonPcnt`）

**症狀**：替換結果產生 `MOT[x].ChangeToFloatNonPcnt(...)` 形式的成員函式呼叫（語法錯誤）。
**原因**：分子反向掃描遺失物件前綴 `MOT[x].`，導致 `ChangeToFloatNonPcnt` 被插在物件成員存取位置。
**範例**：
```
錯誤: MOT[MTestY1].ChangeToFloatNonPcnt((double)(Gali_ReadEncoderPos()), (double)(MOT[MTestY1].Motor->GearRatio))
正確: ChangeToFloatNonPcnt((double)(MOT[MTestY1].Gali_ReadEncoderPos()), (double)(MOT[MTestY1].Motor->GearRatio))
```

### 破損模式 B：函式呼叫除數引數分離（`(double)(funcname))(args)`）

**症狀**：替換結果產生 `(double)(abs))(dLUx-dRDx)` 形式（多餘的右括號，引數脫離函式名）。
**原因**：分母只抓到函式名 `abs` 而遺失引數 `(dLUx-dRDx)`，替換後引數變成孤立片段。
**範例**：
```
錯誤: ChangeToFloatNonPcnt((double)(400.0), (double)(abs))(dLUx-dRDx)
正確: ChangeToFloatNonPcnt((double)(400.0), (double)(abs(dLUx-dRDx)))
```

### 驗證與修復腳本

```powershell
$projectDir = "<PROJECT_DIR>"
$outDir = Join-Path $projectDir "search-division"
$encoding = [System.Text.Encoding]::GetEncoding(950)
$fixCountA = 0; $fixCountB = 0

Get-ChildItem -Path $projectDir -Filter "*.cpp" -Recurse | ForEach-Object {
    $filePath = $_.FullName
    $rawBytes = [System.IO.File]::ReadAllBytes($filePath)
    $content  = $encoding.GetString($rawBytes)
    $modified = $false

    # --- 修復模式 A：.ChangeToFloatNonPcnt 成員函式誤判 ---
    # 匹配 OBJ[idx].ChangeToFloatNonPcnt((double)(Method(...)), ...)
    # 修復為 ChangeToFloatNonPcnt((double)(OBJ[idx].Method(...)), ...)
    $patternA = '(\w+\[[^\]]+\])\.ChangeToFloatNonPcnt\(\(double\)\((\w+(?:\([^)]*\))?)\)'
    while ($content -match $patternA) {
        $m = [regex]::Match($content, $patternA)
        $objPrefix = $m.Groups[1].Value   # e.g. MOT[MTestY1]
        $methodPart = $m.Groups[2].Value  # e.g. Gali_ReadEncoderPos()
        $oldText = $m.Value
        $newText = "ChangeToFloatNonPcnt((double)(${objPrefix}.${methodPart})"
        $content = $content.Replace($oldText, $newText)
        $modified = $true
        $fixCountA++
    }

    # --- 修復模式 B：(double)(funcname))(args) 函式引數分離 ---
    # 匹配 (double)(abs))(dLUx-dRDx) 或 (double)(sqrt))(dLength) 等
    $patternB = '\(double\)\((\w+)\)\)\(([^)]*(?:\([^)]*\)[^)]*)*)\)'
    while ($content -match $patternB) {
        $m = [regex]::Match($content, $patternB)
        $funcName = $m.Groups[1].Value    # e.g. abs, sqrt, Round
        $funcArgs = $m.Groups[2].Value    # e.g. dLUx-dRDx
        $oldText = $m.Value
        $newText = "(double)(${funcName}(${funcArgs}))"
        $content = $content.Replace($oldText, $newText)
        $modified = $true
        $fixCountB++
    }

    if ($modified) {
        $newBytes = $encoding.GetBytes($content)
        [System.IO.File]::WriteAllBytes($filePath, $newBytes)
        Write-Host "已修復: $filePath"
    }
}

Write-Host "`n驗證修復完成！"
Write-Host "模式A修復 (.ChangeToFloatNonPcnt): $fixCountA"
Write-Host "模式B修復 ((double)(func))(args)):  $fixCountB"

# --- 最終確認：搜尋殘留的破損模式 ---
$remainA = 0; $remainB = 0
Get-ChildItem -Path $projectDir -Filter "*.cpp" -Recurse | ForEach-Object {
    $content = $encoding.GetString([System.IO.File]::ReadAllBytes($_.FullName))
    if ($content -match '\.ChangeToFloatNonPcnt') { $remainA++; Write-Host "殘留A: $($_.FullName)" }
    if ($content -match '\(double\)\(\w+\)\)\(') { $remainB++; Write-Host "殘留B: $($_.FullName)" }
}
Write-Host "`n殘留破損: 模式A=$remainA, 模式B=$remainB"
if ($remainA -eq 0 -and $remainB -eq 0) { Write-Host "所有破損模式已修復！" }
```

**輸出**：直接修改 .cpp 檔案並輸出修復統計。

---

## 階段 4：產出報告

報告檔名格式：`search-division/除法安全替換報告_YYYYMMDD_HHMMSS.md`

```powershell
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$outDir = Join-Path "<PROJECT_DIR>" "search-division"
$reportPath = Join-Path $outDir "除法安全替換報告_${timestamp}.md"
```

### 報告內容範本（繁體中文）

```markdown
# 除法安全替換報告

**專案**：<專案名稱> (<原始碼目錄>)
**日期**：<YYYY-MM-DD HH:MM:SS>
**作者**：Steven
**使用模板**：`ChangeToFloatNonPcnt<T>(Numerator, Denominator)`（定義於 MachineType.h）
**模板行為**：除數為 0 時回傳 0，否則回傳 `(double)Numerator/(double)Denominator`

---

## 摘要

| 項目 | 數量 |
|------|------|
| 原始除法候選 | <N> |
| 已有保護（排除） | <N> |
| 未受保護的有風險除法 | <N> |
| 編譯時常數（跳過） | <N> |
| 其他安全（跳過） | <N> |
| **實際替換數** | **<N>** |
| **修改檔案數** | **<N>** |

### 跳過的編譯時常數

| 常數名稱 | 數量 | 原因 |
|----------|------|------|
| <NAME> | <N> | 編譯時常數，不會為 0 |

### 特殊案例

| 檔案:行號 | 原因 | 處理 |
|-----------|------|------|
| <File:Line> | <原因描述> | 跳過 / 手動處理 |

---

## 各檔案替換明細

| # | 檔案名稱 | 替換數 |
|---|----------|--------|
| 1 | <file.cpp> | <N> |

---

## 替換範例

### 替換前
\```cpp
m = (double)(Prod.iInArmX120Pitch-Prod.iInArmX40Pitch) / iPitch_Max_minus_Min;
LP = LP / GearRatio;
\```

### 替換後
\```cpp
m = ChangeToFloatNonPcnt((double)(((double)(Prod.iInArmX120Pitch-Prod.iInArmX40Pitch))), (double)(iPitch_Max_minus_Min));
LP = ChangeToFloatNonPcnt((double)(LP), (double)(GearRatio));
\```

---

## 依使用者要求跳過的變數

- `<變數名稱>` — 所有使用此變數作為除數的除法均已跳過

---

## 中繼檔案

| 檔案 | 說明 |
|------|------|
| search-division/division_candidates.csv | 原始除法候選清單 |
| search-division/final_risky.csv | 過濾後的有風險除法清單 |
| search-division/replacement_pairs.csv | 替換對（舊→新） |
| search-division/replacement_report.csv | 替換執行結果（成功/失敗） |
| search-division/除法安全替換報告_*.md | 最終報告 |
```

---

## 安全除法模板

使用專案中已定義的 `ChangeToFloatNonPcnt` 模板（定義於 `MachineType.h`）：

```cpp
template <class T>
float ChangeToFloatNonPcnt(const T Numerator, const T Denominator)
{
    double str=0.00;
    if(Denominator!=0)
       str= ((double)Numerator/(double)Denominator);
    return str;
};
```

### 替換規則

| 原始形式 | 替換為 |
|----------|--------|
| `a / b` | `ChangeToFloatNonPcnt((double)(a), (double)(b))` |
| `(int)(a / b)` | `(int)(ChangeToFloatNonPcnt((double)(a), (double)(b)))` |
| `x = a / b;` | `x = ChangeToFloatNonPcnt((double)(a), (double)(b));` |

### 型別轉換注意事項

- 模板回傳 `float`，如原始運算為整數除法需外包 `(int)`
- 分子分母一律加上 `(double)` 轉型確保模板正確推導
- 若原始碼已是 `double` 運算，外層 `(double)` 不影響結果

---

## 完整工作流程

1. **確認目標目錄**：與使用者溝通確定掃描範圍及排除變數
2. **階段 1 — 搜尋**：執行 `find_divisions.ps1` → 產生 `search-division/division_candidates.csv`
3. **階段 2 — 過濾**：執行上下文保護分析 → 產生 `search-division/final_risky.csv`
4. **階段 3 — 替換**：
   - 執行 `generate_replacements.ps1` → 產生 `search-division/replacement_pairs.csv`
   - 檢查警告，填入 `$fixCases` 修正特殊案例（空分子等）
   - 執行 `apply_replacements.ps1` → 產生 `search-division/replacement_report.csv`
5. **階段 3.5 — 驗證修復**：執行 `verify_replacements.ps1` 自動偵測並修復兩種破損模式（成員函式誤判、函式引數分離）
6. **測試與編譯**：將替換後的修改用於產生測試並執行編譯驗證，參考專案內的 `bcb_build` 流程。實作細節例：

- **產生測試**：建立或更新必要的測試用例（unit test 或手動測試清單），以覆蓋修改過的關鍵函式與邊界狀況。
- **編譯驗證**：使用專案提供的建置腳本（例如 `scripts\build_bcb.bat` 或相應的 `bcb_build` 腳本）在 BCB6 環境下進行完整編譯；在 CI 或本地執行時請確保使用 Big5/CP950 編碼以避免中文註解亂碼。
- **範例命令**（視腳本存在與否）：

```powershell
:: 在 PowerShell 中執行（若存在）
powershell -ExecutionPolicy Bypass -File scripts\build_bcb.ps1
:: 或直接執行批次（Windows CMD）
scripts\build_bcb.bat
```

執行完成後，確認 `EXE\HT9045.exe`（或對應輸出）成功產生，並以測試清單驗證行為。
7. **驗證**：確認殘留破損數為 0，抽查關鍵檔案的替換結果是否正確
8. **階段 4 — 報告**：產生 `search-division/除法安全替換報告_YYYYMMDD_HHMMSS.md`（繁體中文）

---

## 注意事項

- **Big5 編碼**：BCB6 專案原始碼使用 Big5 編碼。所有檔案讀寫必須使用 `-Encoding Default`（PowerShell）或 `Encoding.GetEncoding(950)`（.NET），避免中文註解亂碼。
- **由下往上替換**：同一檔案多處替換時，從最後一行開始往上替換，避免前面的替換改變後面的行號。
- **成員函式分子**（已修正）：`Object.Method()/Divisor` 形式，`)` 分支反向掃描現已包含物件前綴遍歷。若仍有殘留，階段 3.5 會自動修復 `.ChangeToFloatNonPcnt` 模式。
- **函式呼叫除數**（已修正）：`a/abs(x)`, `a/sqrt(x)`, `a/Round(x,n)` 等形式，分母解析現已擴展函式引數。若仍有殘留，階段 3.5 會自動修復 `(double)(func))(args)` 模式。
- **空分子**：當反向掃描因 `=` 停止而分子為空時，會產生 WARN 警告，需在 `$fixCases` 中手動填入完整的 OldExpr/Num/Den。
- **OR 條件不完全保護**：`if(A>0 || B>0)` 後的 `/A` 或 `/B` 仍有風險（其中一個可能為 0），應視為需替換。
- **鏈式除法**：`a/b/c` 需逐一檢查 `b` 與 `c` 是否皆受保護，若皆安全則整段跳過。
- **已有部分轉換**：某些檔案可能已被先前開發者部分轉換，搜尋階段已自動排除含 `ChangeToFloat*` 的行。
- **模板位置確認**：替換前先確認專案中 `ChangeToFloatNonPcnt` 的定義位置和簽名，不同專案可能有不同實作。
- **階段 3.5 必要性**：即使已修正分子/分母解析邏輯，仍建議每次執行後都運行階段 3.5 驗證腳本，作為最後一道安全網。