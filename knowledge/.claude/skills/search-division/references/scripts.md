# 除法安全掃描腳本集

## 階段 1：搜尋除法位置（find_divisions.ps1）

使用 PowerShell 逐行解析，先移除字串常量與註解，再以精確 regex 抓取真正的除法運算及除數變數名稱。

> **用法**：將下方腳本存為 `find_divisions.ps1`，修改 `$projectDir` 後以 `powershell -ExecutionPolicy Bypass -File find_divisions.ps1` 執行。

```powershell
$projectDir = "<PROJECT_DIR>"
$outDir = Join-Path $projectDir "search-division"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$results = @()

Get-ChildItem -Path $projectDir -Filter "*.cpp" -Recurse | ForEach-Object {
    $file = $_
    $lineNum = 0
    Get-Content $file.FullName -Encoding Default | ForEach-Object {
        $lineNum++
        $origLine = $_
        $line = $origLine

        # 移除字串常量，避免路徑中的 / 被誤判
        $line = $line -replace '"[^"]*"', '""'
        $line = $line -replace "'[^']*'", "''"

        # 移除行尾註解 //
        if ($line -match '^(.*?)//') { $line = $Matches[1] }

        # 跳過空行、區塊註解行、前處理指令
        if ($line.Trim() -match '^\s*$') { return }
        if ($line.Trim().StartsWith('/*')) { return }
        if ($line.Trim().StartsWith('*'))  { return }
        if ($line.Trim() -match '^#')      { return }

        # 精確匹配：前方為 ) ] 或英數字，後方為變數名（非 / * =）
        $matches2 = [regex]::Matches($line,
            '(?<=[)\]\w])\s*/\s*(?![\*/=])([a-zA-Z_]\w*)')

        foreach ($m in $matches2) {
            $divisor = $m.Groups[1].Value

            # 排除 C++ 關鍵字與型別
            if ($divisor -match '^(double|float|int|long|short|char|void|bool|unsigned|signed|const|static|class|struct|enum|return|if|else|for|while|do|switch|case|break|continue|goto|typedef|sizeof|new|delete|try|catch|throw|this|virtual|public|private|protected|template|typename|namespace|using|inline|extern|register|volatile|mutable|explicit|friend|operator|and|or|not|xor|auto)$') {
                continue
            }

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

**輸出**：`search-division/division_candidates.csv`

### 搜尋階段的過濾邏輯

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
    if ($c.Divisor -match '^\d+$' -and [int]$c.Divisor -in $constantDivisors) {
        $constantCount++
        continue
    }

    $filePath = $c.FullPath
    if (-not (Test-Path $filePath)) { continue }

    $allLines = Get-Content $filePath -Encoding Default
    $lineIdx = [int]$c.Line - 1

    $startCtx = [Math]::Max(0, $lineIdx - 20)
    $endCtx   = [Math]::Min($allLines.Count - 1, $lineIdx + 20)
    $context  = ($allLines[$startCtx..$endCtx]) -join "`n"
    $divVar   = [regex]::Escape($c.Divisor)

    $isProtected = $false

    if ($context -match "if\s*\(\s*${divVar}\s*[!><=]+\s*0") { $isProtected = $true }
    if ($context -match "if\s*\(\s*0\s*[!<>=]+\s*${divVar}") { $isProtected = $true }
    if ($context -match "${divVar}\s*[!=]=\s*0\s*\)\s*\?")    { $isProtected = $true }

    $divLine = $allLines[$lineIdx]
    if ($divLine -match 'ChangeToFloat|ChangeToPercentage|ChangeToFloatNonPcnt') {
        $isProtected = $true
    }

    $assignCtx = ($allLines[[Math]::Max(0,$lineIdx-5)..$lineIdx]) -join "`n"
    if ($assignCtx -match "${divVar}\s*=\s*[1-9]\d*\s*;") { $isProtected = $true }

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

**輸出**：`search-division/final_risky.csv`

### 保護偵測規則

| 保護模式 | 判定依據 | 結果 |
|----------|----------|------|
| `if(var!=0)` / `if(var>0)` | 上下文 20 行內有條件檢查 | 安全，排除 |
| 三元運算子 `(var==0)?0:(a/var)` | 同行或上下文有三元防護 | 安全，排除 |
| 已使用 `ChangeToFloat*` | 同行已包含安全模板呼叫 | 安全，排除 |
| 除數前幾行被賦值為非零常數 | 如 `dAcc=1;` 後接 `/dAcc` | 安全，排除 |
| `if(A>0 \|\| B>0)` 後 `/A` `/B` | OR 無法同時保護兩個除數 | **有風險，保留** |

---

## 階段 3：產生替換對並套用

### 3.1 產生替換對（generate_replacements.ps1）

從 `final_risky.csv` 讀取，解析分子與分母，產生 `OldExpr → NewExpr` 替換對。

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

    $denomFullPattern = "/\s*(${escapedDiv}(?:\[[^\]]*\])?(?:\.\w+|\->\w+)*(?:\[[^\]]*\])?(?:\.\w+|\->\w+)*)"

    if ($origLine -match $denomFullPattern) {
        $fullDenominator = $Matches[1]
        $divisionMatch   = $Matches[0]
        $divPos          = $origLine.IndexOf($divisionMatch)

        # 函式呼叫除數：分母後方緊接 ( 時擴展至完整函式呼叫
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

        $beforeDiv = $origLine.Substring(0, $divPos)

        # --- 反向掃描分子 ---
        $numerator = ""
        $numEnd = $beforeDiv.Length - 1

        if ($numEnd -ge 0) {
            $ch = $beforeDiv[$numEnd]

            if ($ch -eq ')') {
                $depth = 1; $pos = $numEnd - 1
                while ($pos -ge 0 -and $depth -gt 0) {
                    if ($beforeDiv[$pos] -eq ')') { $depth++ }
                    elseif ($beforeDiv[$pos] -eq '(') { $depth-- }
                    $pos--
                }
                $startPos = $pos + 1
                while ($startPos -gt 0 -and $beforeDiv[$startPos-1] -match '\w') { $startPos-- }

                # 繼續往前掃描物件前綴（. -> [] 等成員存取鏈）
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
                $pos = $numEnd
                while ($pos -ge 0) {
                    $currentChar = $beforeDiv[$pos]
                    if ($currentChar -match '\w') { $pos-- }
                    elseif ($currentChar -eq '.') { $pos-- }
                    elseif ($currentChar -eq '>' -and $pos -gt 0 -and $beforeDiv[$pos-1] -eq '-') {
                        $pos -= 2
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
                        $pos--
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

**輸出**：`search-division/replacement_pairs.csv`

#### 分子反向掃描規則

| 結尾字元 | 掃描邏輯 |
|----------|----------|
| `)` | 找匹配的 `(`，再往前掃描函式名及物件前綴 |
| `]` | 找匹配的 `[`，再往前掃描陣列變數及成員存取 |
| 英數字 | 往前掃描，遇 `.` `->` `[]` `()` `*`(乘法) 繼續，遇 `+ - = , ;` 停止 |

#### 已知特殊案例與修正方式

| 問題模式 | 範例 | 修正方式 |
|----------|------|----------|
| 成員函式分子 | `MOT[x].Method()/Divisor` | 分子掃描後繼續往前遍歷成員存取鏈 |
| 函式呼叫除數 | `a/abs(x-y)` | 分母匹配後檢查後方 `(` 並擴展 |
| 空分子 | `x = /divisor` 形式 | 列入警告，需以 `$fixCases` 手動補正 |

`$fixCases` 字典用於手動補正：

```powershell
$fixCases = @{
    # "檔名:行號" = @{ OldExpr="完整除法"; Num="完整分子"; Den="完整分母" }
    "aTester_Front.cpp:287" = @{
        OldExpr = "MOT[MTestY1].Gali_ReadEncoderPos()/MOT[MTestY1].Motor->GearRatio"
        Num     = "MOT[MTestY1].Gali_ReadEncoderPos()"
        Den     = "MOT[MTestY1].Motor->GearRatio"
    }
}
```

### 3.2 套用替換（apply_replacements.ps1）

讀取替換對 CSV，使用 Big5/CP950 編碼讀寫，由下往上替換（避免行號偏移）。

```powershell
$dir = "<PROJECT_DIR>"
$outDir = Join-Path $dir "search-division"
$rep = Import-Csv (Join-Path $outDir "replacement_pairs.csv")

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

    $rawBytes = [System.IO.File]::ReadAllBytes($filePath)
    $encoding = [System.Text.Encoding]::GetEncoding(950)
    $content  = $encoding.GetString($rawBytes)
    $lines    = $content -split "`r`n|`n"

    $modified = $false

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

**輸出**：`search-division/replacement_report.csv`

---

## 階段 3.5：替換後驗證與自動修復

替換完成後，掃描已修改的 .cpp 檔案，偵測並自動修復兩種已知的破損模式。

### 破損模式 A：`.ChangeToFloatNonPcnt` 成員函式誤判

```
錯誤: MOT[MTestY1].ChangeToFloatNonPcnt((double)(Gali_ReadEncoderPos()), ...)
正確: ChangeToFloatNonPcnt((double)(MOT[MTestY1].Gali_ReadEncoderPos()), ...)
```

### 破損模式 B：`(double)(funcname))(args)` 函式引數分離

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

    # --- 修復模式 A ---
    $patternA = '(\w+\[[^\]]+\])\.ChangeToFloatNonPcnt\(\(double\)\((\w+(?:\([^)]*\))?)\)'
    while ($content -match $patternA) {
        $m = [regex]::Match($content, $patternA)
        $objPrefix = $m.Groups[1].Value
        $methodPart = $m.Groups[2].Value
        $oldText = $m.Value
        $newText = "ChangeToFloatNonPcnt((double)(${objPrefix}.${methodPart})"
        $content = $content.Replace($oldText, $newText)
        $modified = $true
        $fixCountA++
    }

    # --- 修復模式 B ---
    $patternB = '\(double\)\((\w+)\)\)\(([^)]*(?:\([^)]*\)[^)]*)*)\)'
    while ($content -match $patternB) {
        $m = [regex]::Match($content, $patternB)
        $funcName = $m.Groups[1].Value
        $funcArgs = $m.Groups[2].Value
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

Write-Host "`n驗證修復完成！模式A: $fixCountA / 模式B: $fixCountB"

# 最終確認
$remainA = 0; $remainB = 0
Get-ChildItem -Path $projectDir -Filter "*.cpp" -Recurse | ForEach-Object {
    $content = $encoding.GetString([System.IO.File]::ReadAllBytes($_.FullName))
    if ($content -match '\.ChangeToFloatNonPcnt') { $remainA++; Write-Host "殘留A: $($_.FullName)" }
    if ($content -match '\(double\)\(\w+\)\)\(') { $remainB++; Write-Host "殘留B: $($_.FullName)" }
}
Write-Host "殘留破損: 模式A=$remainA, 模式B=$remainB"
```

---

## 階段 4：產出報告

報告檔名格式：`search-division/除法安全替換報告_YYYYMMDD_HHMMSS.md`

```powershell
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$outDir = Join-Path "<PROJECT_DIR>" "search-division"
$reportPath = Join-Path $outDir "除法安全替換報告_${timestamp}.md"
```

### 報告必含欄位

| 項目 | 說明 |
|------|------|
| 原始除法候選數 | 階段 1 輸出數 |
| 已有保護排除數 | 階段 2 過濾數 |
| 實際替換數 | 階段 3 成功數 |
| 修改檔案數 | 涉及的 .cpp 檔案數 |
| 跳過的編譯時常數 | 名稱與數量 |
| 各檔案替換明細 | 檔案 + 替換數 |
| 替換範例 | 替換前/後對照 |
