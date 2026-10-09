# HT9050 更新包：覆蓋前檢查 / 安全套用（筆電 Claude 20260925 產生；Windows PowerShell 5.1 可跑）
#
#   -Mode Check   只讀，不改任何檔。逐檔分類並寫報告。
#   -Mode Apply   依分類套用：NEW／OLD 才覆蓋（OLD 先備份），LOCAL 一律不蓋、只準備合併材料。
#
# 分類（比對前把 CRLF 換成 LF，所以只有行尾不同算「相同」）：
#   SAME      機台的檔跟包裡一樣                         -> 不動
#   NEW       機台上沒有這個檔                           -> 複製
#   OLD       機台的檔是筆電歷史上某一版（沒被本地改過） -> 備份後覆蓋
#   LOCAL     機台的檔不是筆電歷史上任何一版             -> 不蓋！這是機台本地修改（例如 EastSun 的 IOWEB-P17／P25）
#             Apply 會在合併目錄放 machine／laptop／base（找得到時）／merged（git merge-file 結果）
#   GONE      筆電 main 已經沒有這個檔、機台上還有       -> 只列出，不刪
#
# 例：
#   powershell -NoProfile -ExecutionPolicy Bypass -File check_and_copy.ps1 -Mode Check
#   powershell -NoProfile -ExecutionPolicy Bypass -File check_and_copy.ps1 -Mode Apply -Target D:\HT9045
param(
    [ValidateSet('Check', 'Apply')][string]$Mode = 'Check',
    [string]$Target = 'D:\HT9045'
)
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkg = [IO.Path]::GetFullPath((Join-Path $here '..\HT9045'))
$ts = Get-Date -Format 'yyyyMMdd_HHmmss'
$latin1 = [Text.Encoding]::GetEncoding(28591)
$md5 = [Security.Cryptography.MD5]::Create()

function Hex([byte[]]$h) { ([BitConverter]::ToString($h)).Replace('-', '').ToLower() }
function Md5Raw([string]$p) { Hex $md5.ComputeHash([IO.File]::ReadAllBytes($p)) }
function Md5Norm([string]$p) {
    $s = $latin1.GetString([IO.File]::ReadAllBytes($p)).Replace("`r`n", "`n")
    Hex $md5.ComputeHash($latin1.GetBytes($s))
}

if (-not (Test-Path -LiteralPath $pkg)) { throw "找不到包的樹：$pkg" }
if (-not (Test-Path -LiteralPath $Target)) { throw "找不到目標：$Target" }

# --- 讀清單 ---
$man = @()
$rev = ''
foreach ($l in [IO.File]::ReadAllLines((Join-Path $here 'package_manifest.tsv'), [Text.Encoding]::UTF8)) {
    if ($l.StartsWith('# rev')) { $rev = $l.Split("`t")[1]; continue }
    if ($l.StartsWith('#') -or $l -eq '') { continue }
    $a = $l.Split("`t")
    $man += , @($a[0], $a[1], $a[2])
}
$known = @{}      # path -> hashset of md5norm
$knownBlob = @{}  # "path`tblob" -> 1
foreach ($l in [IO.File]::ReadAllLines((Join-Path $here 'known_versions.tsv'), [Text.Encoding]::UTF8)) {
    if ($l.StartsWith('#') -or $l -eq '') { continue }
    $a = $l.Split("`t")
    if (-not $known.ContainsKey($a[0])) { $known[$a[0]] = New-Object 'System.Collections.Generic.HashSet[string]' }
    [void]$known[$a[0]].Add($a[2])
    $knownBlob["$($a[0])`t$($a[1])"] = 1
}
$hasGit = $null -ne (Get-Command git -ErrorAction SilentlyContinue)

# 機台 git 歷史裡，找這個檔「最近一個是筆電已知版本」的那一版 = 三方合併的 base
function Find-GitBase([string]$path, [string]$dst) {
    if (-not $hasGit) { return $null }
    $ErrorActionPreference = 'Continue'
    $dir = Split-Path -Parent $dst
    $top = & git -C $dir rev-parse --show-toplevel 2>$null
    if (-not $top) { return $null }
    $top = ([string]$top).Trim().Replace('/', '\')
    $rel = $dst.Substring($top.Length).TrimStart('\').Replace('\', '/')
    $commits = & git -C $top log -n 60 --format=%H -- $rel 2>$null
    foreach ($c in @($commits)) {
        if (-not $c) { continue }
        $blob = & git -C $top rev-parse "$($c):$rel" 2>$null
        if (-not $blob) { continue }
        $blob = ([string]$blob).Trim()
        if ($knownBlob.ContainsKey("$path`t$blob")) { return @{ top = $top; rel = $rel; commit = $c; blob = $blob } }
        $tmp = [IO.Path]::GetTempFileName()
        cmd /c "git -C `"$top`" cat-file blob $blob > `"$tmp`"" | Out-Null
        $h = Md5Norm $tmp
        Remove-Item -LiteralPath $tmp -Force
        if ($known[$path] -and $known[$path].Contains($h)) { return @{ top = $top; rel = $rel; commit = $c; blob = $blob } }
    }
    return $null
}

# --- 分類 ---
$rows = New-Object System.Collections.ArrayList
$inPkg = @{}
foreach ($m in $man) {
    $path = $m[0]; $inPkg[$path] = 1
    $src = Join-Path $pkg ($path.Replace('/', '\'))
    $dst = Join-Path $Target ($path.Replace('/', '\'))
    if (-not (Test-Path -LiteralPath $dst -PathType Leaf)) { [void]$rows.Add(@{ s = 'NEW'; p = $path; src = $src; dst = $dst }); continue }
    if ((Md5Raw $dst) -eq $m[1]) { [void]$rows.Add(@{ s = 'SAME'; p = $path }); continue }
    $n = Md5Norm $dst
    if ($n -eq $m[2]) { [void]$rows.Add(@{ s = 'SAME'; p = $path }); continue }
    if ($known[$path] -and $known[$path].Contains($n)) { [void]$rows.Add(@{ s = 'OLD'; p = $path; src = $src; dst = $dst }); continue }
    $gb = Find-GitBase $path $dst
    $fb = Join-Path $here ('base_fb037f43\' + $path.Replace('/', '\'))   # 包內的 fb037f43 版（機台整合樹的底稿）
    if (-not (Test-Path -LiteralPath $fb -PathType Leaf)) { $fb = $null }
    [void]$rows.Add(@{ s = 'LOCAL'; p = $path; src = $src; dst = $dst; base = $gb; fb = $fb })
}
$gone = @()
$delList = Join-Path $here 'deleted_in_main.txt'
if (Test-Path -LiteralPath $delList) {
    # 差異包：只有 deleted_in_main.txt 列的才是「main 已刪除」（沒變動的檔本來就不在包裡）
    foreach ($l in [IO.File]::ReadAllLines($delList, [Text.Encoding]::UTF8)) {
        if ($l.StartsWith('#') -or $l -eq '') { continue }
        if (Test-Path -LiteralPath (Join-Path $Target ($l.Replace('/', '\'))) -PathType Leaf) { $gone += $l }
    }
}
else {
    foreach ($p in $known.Keys) {
        if ($inPkg.ContainsKey($p)) { continue }
        if (Test-Path -LiteralPath (Join-Path $Target ($p.Replace('/', '\'))) -PathType Leaf) { $gone += $p }
    }
}

# --- 套用 ---
$bak = "$Target" + "_update_bak_$ts"
$mrg = "$Target" + "_update_merge_$ts"
if ($Mode -eq 'Apply') {
    foreach ($r in $rows) {
        if ($r.s -eq 'OLD') {
            $b = Join-Path $bak ($r.p.Replace('/', '\'))
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $b) | Out-Null
            Copy-Item -LiteralPath $r.dst -Destination $b -Force
        }
        if ($r.s -eq 'OLD' -or $r.s -eq 'NEW') {
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $r.dst) | Out-Null
            Copy-Item -LiteralPath $r.src -Destination $r.dst -Force
            if ((Md5Raw $r.dst) -ne (Md5Raw $r.src)) { throw "複製後內容不符：$($r.dst)" }
        }
        if ($r.s -eq 'LOCAL') {
            $d = Join-Path $mrg ($r.p.Replace('/', '\'))
            New-Item -ItemType Directory -Force -Path $d | Out-Null
            Copy-Item -LiteralPath $r.dst -Destination (Join-Path $d 'machine') -Force
            Copy-Item -LiteralPath $r.src -Destination (Join-Path $d 'laptop') -Force
            $r.merge = '沒有 base（機台 git 歷史與包內 base_fb037f43 都找不到）：要人工合併'
            $mach = Join-Path $d 'machine'; $bas = Join-Path $d 'base'; $lap = Join-Path $d 'laptop'
            $haveBase = $false
            $ErrorActionPreference = 'Continue'
            if ($r.base) {
                cmd /c "git -C `"$($r.base.top)`" cat-file blob $($r.base.blob) > `"$bas`"" | Out-Null
                $haveBase = (Test-Path -LiteralPath $bas)
            }
            elseif ($r.fb) {
                Copy-Item -LiteralPath $r.fb -Destination $bas -Force
                $haveBase = $true
            }
            if ($haveBase -and $hasGit) {
                # 三份都先轉成 LF 再合併（git blob 常是 LF、機台檔是 CRLF，行尾不同會變成整份衝突），結果再照機台檔的行尾轉回去
                foreach ($x in @($mach, $bas, $lap)) {
                    $s = $latin1.GetString([IO.File]::ReadAllBytes($x)).Replace("`r`n", "`n")
                    [IO.File]::WriteAllBytes("$x.lf", $latin1.GetBytes($s))
                }
                cmd /c "git merge-file -p -L machine -L base -L laptop `"$mach.lf`" `"$bas.lf`" `"$lap.lf`" > `"$(Join-Path $d 'merged.lf')`""
                $rc = $LASTEXITCODE
                $m = $latin1.GetString([IO.File]::ReadAllBytes((Join-Path $d 'merged.lf')))
                if ($latin1.GetString([IO.File]::ReadAllBytes($mach)).Contains("`r`n")) { $m = $m.Replace("`n", "`r`n") }
                [IO.File]::WriteAllBytes((Join-Path $d 'merged'), $latin1.GetBytes($m))
                foreach ($x in @("$mach.lf", "$bas.lf", "$lap.lf", (Join-Path $d 'merged.lf'))) { Remove-Item -LiteralPath $x -Force }
                if ($rc -eq 0) { $r.merge = '乾淨合併（merged 可用，但要人看過再放回）' }
                elseif ($rc -gt 0) { $r.merge = "有 $rc 處衝突（merged 裡有 <<<<<<< 標記）" }
                else { $r.merge = "git merge-file 失敗（rc=$rc）" }
            }
            elseif ($haveBase) { $r.merge = '有 base、機台沒有 git：merge 要人工做（machine／base／laptop 三份都在）' }
            $ErrorActionPreference = 'Stop'
        }
    }
}

# --- 報告 ---
$cnt = @{}
foreach ($r in $rows) { $cnt[$r.s] = 1 + [int]$cnt[$r.s] }
$L = New-Object System.Collections.ArrayList
[void]$L.Add("HT9050 更新包 $Mode  $ts")
[void]$L.Add("包的版本：筆電 main $rev")
[void]$L.Add("目標：$Target")
[void]$L.Add(("SAME {0}  NEW {1}  OLD {2}  LOCAL {3}  GONE {4}" -f [int]$cnt['SAME'], [int]$cnt['NEW'], [int]$cnt['OLD'], [int]$cnt['LOCAL'], $gone.Count))
if ($Mode -eq 'Apply') {
    [void]$L.Add("已覆蓋 NEW+OLD；OLD 的原檔備份在：$bak")
    if ([int]$cnt['LOCAL'] -gt 0) { [void]$L.Add("LOCAL 沒有覆蓋；合併材料在：$mrg") }
}
[void]$L.Add('')
[void]$L.Add('== LOCAL（機台本地修改，不可直接蓋）==')
foreach ($r in $rows) {
    if ($r.s -ne 'LOCAL') { continue }
    $b = if ($r.base) { "base=機台 commit $($r.base.commit.Substring(0,8))" } elseif ($r.fb) { 'base=包內 fb037f43' } else { 'base=找不到' }
    $x = if ($r.merge) { "  | $($r.merge)" } else { '' }
    [void]$L.Add("  $($r.p)   ($b)$x")
}
[void]$L.Add('')
[void]$L.Add('== GONE（筆電 main 已刪除，機台還在；沒有刪）==')
foreach ($p in ($gone | Sort-Object)) { [void]$L.Add("  $p") }
[void]$L.Add('')
[void]$L.Add('== NEW / OLD ==')
foreach ($r in $rows) { if ($r.s -eq 'NEW' -or $r.s -eq 'OLD') { [void]$L.Add("  $($r.s)  $($r.p)") } }
$rep = Join-Path $here "report_$($Mode)_$ts.txt"
[IO.File]::WriteAllLines($rep, [string[]]$L, (New-Object Text.UTF8Encoding($true)))
$L[0..4] | ForEach-Object { Write-Host $_ }
Write-Host "LOCAL 清單與完整報告：$rep"
