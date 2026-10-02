param(
  [string[]]$Cpp = @(), [string[]]$Web = @(),
  [Parameter(Mandatory = $true)][string]$ReadmeFile,   # UTF-8 text to insert before the last README line
  [Parameter(Mandatory = $true)][string]$MsgFile,      # commit message (UTF-8)
  [switch]$NoParams,                                   # skip the machine_params / workorder snapshot (default: always take it)
  [switch]$Commit                                      # without it: make patches + snapshot + README + manifest + scan only
)
# Push stage for D:\HT9045\_push_github_20260926 -> origin machine/integ-ioweb (fast-forward only).
#   AI(W906-PARAMS-PUSH) 20261002: EastSun 1002 -- every machine push also carries the machine parameters and the
#   active work order (recipe), so colleagues can test / check with the same settings. He chose: as-is (passwords
#   included, the repo is public -- his ruling), and only the recipe SetUp.inf names (not all of IniData).
#   Snapshot sources (what wb_serve on this machine actually reads, see .vscode/launch.json of the integ tree):
#     D:\HT9045\system              -> machine_params\D_HT9045_system   (Gerneral.ini / IO_Table.csv / Mot_Table.csv ...)
#     D:\HT9045\config              -> machine_params\D_HT9045_config
#     D:\HT9045\_integ_ioweb\runcfg -> machine_params\runcfg            (SetUp.inf, system\teach.ini, config\; logs\ left out)
#     D:\HT9045\IniData\Data\<SetUp.inf recipe> -> workorder\<recipe>, plus runcfg\config\LastSet.ini -> workorder\LastSet.ini
#   Mirrors (robocopy /MIR): git history of this branch is the history of the machine's settings.
$ErrorActionPreference = 'Stop'
$Cpp = @(($Cpp -join ',') -split ',' | Where-Object { $_ }); $Web = @(($Web -join ',') -split ',' | Where-Object { $_ })   # -File passes "a,b" as one string
$PUSHDIR = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$CT = 'D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0'
$WT = 'D:\HT9045\_integ_ioweb\web'
$RUNCFG = 'D:\HT9045\_integ_ioweb\runcfg'
$u8 = New-Object Text.UTF8Encoding $false
function NextNum($dir) {
  $m = (Get-ChildItem (Join-Path $PUSHDIR $dir) -Filter '*.patch' | ForEach-Object { [int]$_.Name.Substring(0, 4) } | Measure-Object -Maximum).Maximum
  return [int]$m + 1
}
function Mirror($from, $to, [string[]]$xd = @(), [string[]]$pat = @()) {
  $a = @($from, $to) + $pat + @('/MIR', '/R:1', '/W:1', '/NFL', '/NDL', '/NJH', '/NJS', '/NP')
  if ($xd.Count) { $a += '/XD'; $a += $xd }
  & robocopy @a | Out-Null
  if ($LASTEXITCODE -ge 8) { throw "robocopy $from -> $to failed ($LASTEXITCODE)" }
}
$new = @()
$dirty = @(git -C $PUSHDIR status --porcelain | Where-Object { $_ -notmatch '_tools/' })   # an edit to this script itself is allowed (it gets committed with the stage)
if ($dirty.Count) { throw "push folder not clean: $($dirty -join '; ')" }
$n = NextNum 'cpp'
foreach ($c in $Cpp) { $f = git -C $CT format-patch -1 $c --start-number $n -o (Join-Path $PUSHDIR 'cpp'); if ($LASTEXITCODE) { throw "format-patch $c" }; $new += $f; $n++ }
$n = NextNum 'web'
foreach ($c in $Web) { $f = git -C $WT format-patch -1 $c --start-number $n -o (Join-Path $PUSHDIR 'web'); if ($LASTEXITCODE) { throw "format-patch $c" }; $new += $f; $n++ }
"new patches:"; $new | ForEach-Object { '  ' + (Split-Path $_ -Leaf) }

# --- machine parameters + work order snapshot ---
$paramNote = ''
if (-not $NoParams) {
  $MP = Join-Path $PUSHDIR 'machine_params'
  $WO = Join-Path $PUSHDIR 'workorder'
  Mirror 'D:\HT9045\system' (Join-Path $MP 'D_HT9045_system')
  Mirror 'D:\HT9045\config' (Join-Path $MP 'D_HT9045_config')
  Mirror $RUNCFG (Join-Path $MP 'runcfg') @('logs')
  Mirror 'D:\GPIB9045\system' (Join-Path $MP 'D_GPIB9045_system') @() @('*.ini', '*.dat')   # AI(W906-PARAMS-PUSH) 20261002: the GPIB general.ini ([Version] Model) -- settings only, not the .obj build junk
  $recipe = ([IO.File]::ReadAllText((Join-Path $RUNCFG 'SetUp.inf'))).Trim()
  if (-not $recipe -or $recipe -match '[\\/:*?"<>|]') { throw "SetUp.inf recipe name not usable: '$recipe'" }
  $rsrc = Join-Path 'D:\HT9045\IniData\Data' $recipe
  if (-not (Test-Path -LiteralPath $rsrc -PathType Container)) { throw "recipe folder not found: $rsrc" }
  if (Test-Path $WO) { Get-ChildItem $WO -Directory | Where-Object { $_.Name -ne $recipe } | Remove-Item -Recurse -Force }   # only the active one
  Mirror $rsrc (Join-Path $WO $recipe)
  Copy-Item -LiteralPath (Join-Path $RUNCFG 'config\LastSet.ini') -Destination (Join-Path $WO 'LastSet.ini') -Force
  $ts = Get-Date -Format 'yyyy-MM-dd HH:mm'
  $cHead = (git -C $CT log -1 --format='%h %s').Substring(0, [Math]::Min(90, (git -C $CT log -1 --format='%h %s').Length))
  $wHead = (git -C $WT log -1 --format='%h')
  $cnt = { param($d) (Get-ChildItem $d -Recurse -File -Force).Count }
  $rd = @"
HT9050 機台參數快照（machine_params\）—— $ts
來源：HT9050 實機（裝 PCIE-1203 的那台）。程式版本：C++ $cHead／web $wHead。
每次機台端推 GitHub 都會重拍一次（整個資料夾鏡像），所以這個分支的 git 歷史就是機台設定的歷史。

資料夾 → 放回機台的位置
  D_HT9045_system\  ($(& $cnt (Join-Path $MP 'D_HT9045_system')) 檔)  → D:\HT9045\system\      機台正本：Gerneral.ini、IO_Table.csv、Mot_Table.csv（wb_serve 直接讀這三個）
  D_HT9045_config\  ($(& $cnt (Join-Path $MP 'D_HT9045_config')) 檔)  → D:\HT9045\config\
  runcfg\           ($(& $cnt (Join-Path $MP 'runcfg')) 檔)  → D:\HT9045\_integ_ioweb\runcfg\   SetUp.inf（目前工單）、system\teach.ini（教導值）、config\（config.ini、LastSet.ini、Pci1203*.ini …）；logs\ 沒放
  D_GPIB9045_system\ ($(& $cnt (Join-Path $MP 'D_GPIB9045_system')) 檔) → D:\GPIB9045\system\   只收 *.ini／*.dat；general.ini 的 [Version] Model＝機種（HT9050＝9050GPIB，程式靠它啟動 HT9050 分支）

注意
  * 這是 HT9050 這一台的設定。別台機台不要整包覆蓋：IO 對照或馬達表錯了，程式會照錯的對照推線圈、動馬達。
  * 放回機台前先備份原本的資料夾。
  * 內容照原樣，沒有遮掉任何值（含密碼檔；EastSun 20261002 裁決）。
"@
  [IO.File]::WriteAllText((Join-Path $MP 'README_PARAMS.txt'), $rd.Replace("`r`n", "`n"), $u8)
  $wd = @"
HT9050 目前工單（workorder\）—— $ts
  $recipe\   ← D:\HT9045\IniData\Data\$recipe\（$(& $cnt (Join-Path $WO $recipe)) 檔；機台 SetUp.inf 指定的配方）
  LastSet.ini ← D:\HT9045\_integ_ioweb\runcfg\config\LastSet.ini（上一批工單資訊）
只放機台目前用的那一個配方（EastSun 20261002）。換工單後下次推送會自動換成新的那個。
"@
  [IO.File]::WriteAllText((Join-Path $WO 'README_WORKORDER.txt'), $wd.Replace("`r`n", "`n"), $u8)
  $paramNote = "machine_params／workorder 快照 $ts（工單 $recipe）"
  "snapshot: $paramNote"
}

# README: insert before the last line (UTF-8 no BOM, LF)
$rdm = Join-Path $PUSHDIR 'README.txt'
$txt = [IO.File]::ReadAllText($rdm, $u8).Replace("`r`n", "`n")
$last = 'MD5 清單在 MANIFEST_MD5.tsv。'
$add = [IO.File]::ReadAllText($ReadmeFile, $u8).Replace("`r`n", "`n").TrimEnd("`n") + "`n"
$i = $txt.LastIndexOf($last)
if ($i -lt 0) { throw 'README last line not found' }
$txt = $txt.Substring(0, $i) + $add + $txt.Substring($i)
[IO.File]::WriteAllText($rdm, $txt, $u8)

# scan the new patches + the README addition; stop on any hit.
# machine_params / workorder are NOT scanned for passwords (EastSun 20261002: as-is), but still for keys / tokens.
$rx = '(ghp_[A-Za-z0-9]{10,}|gho_[A-Za-z0-9]{10,}|github_pat_|glpat-|xox[abprs]-|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY|ssh-(rsa|ed25519) AAAA|\b7z\b[^\n]*\s-p\S|passw(or)?d\s*[:=]\s*["'']?[^\s"''<>]{4,})'
$rxKey = '(ghp_[A-Za-z0-9]{10,}|gho_[A-Za-z0-9]{10,}|github_pat_|glpat-|xox[abprs]-|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY|ssh-(rsa|ed25519) AAAA)'
$hits = @()
foreach ($f in @($new) + @($ReadmeFile)) {
  $k = 0
  foreach ($line in [IO.File]::ReadAllLines($f, $u8)) {
    $k++
    if (($line.StartsWith('+') -or $f -eq $ReadmeFile) -and $line -match $rx) { $hits += ((Split-Path $f -Leaf) + ':' + $k + ': ' + $line.Substring(0, [Math]::Min(200, $line.Length))) }
    if ($line -match '^diff --git a/\S*(IO_Table|Mot_Table|Gerneral\.ini|general\.ini|SetUp\.inf)') { $hits += ((Split-Path $f -Leaf) + ': MACHINE CONFIG FILE in a patch (it goes in machine_params\, not cpp\): ' + $line) }
  }
}
if (-not $NoParams) {
  $l1 = [Text.Encoding]::GetEncoding(28591)
  foreach ($f in Get-ChildItem (Join-Path $PUSHDIR 'machine_params'), (Join-Path $PUSHDIR 'workorder') -Recurse -File -Force) {
    if ($f.Length -gt 50MB) { $hits += "$($f.FullName): larger than 50 MB" ; continue }
    if ($l1.GetString([IO.File]::ReadAllBytes($f.FullName)) -match $rxKey) { $hits += "$($f.FullName): key / token pattern" }
  }
}

# manifest: every file except the manifest and .git, path<TAB>bytes<TAB>MD5, OrdinalIgnoreCase, CRLF
$files = Get-ChildItem $PUSHDIR -Recurse -File -Force | Where-Object { $_.FullName -notmatch '\\\.git\\' -and $_.Name -ne 'MANIFEST_MD5.tsv' }
$rows = New-Object 'System.Collections.Generic.List[string]'
foreach ($f in $files) { $rel = $f.FullName.Substring($PUSHDIR.Length + 1); $rows.Add($rel + "`t" + $f.Length + "`t" + (Get-FileHash -LiteralPath $f.FullName -Algorithm MD5).Hash.ToUpper()) }
$arr = $rows.ToArray(); [Array]::Sort($arr, [StringComparer]::OrdinalIgnoreCase)
[IO.File]::WriteAllText((Join-Path $PUSHDIR 'MANIFEST_MD5.tsv'), ("path`tbytes`tmd5`r`n" + ($arr -join "`r`n") + "`r`n"), $u8)
"manifest rows: $($arr.Count)"

if ($hits.Count) { "SCAN HITS ($($hits.Count)) -- NOT COMMITTED:"; $hits; exit 3 }
"scan: 0 hits"
if (-not $Commit) { 'dry run done (not committed; the folder now has the changes -- commit/push by hand or git checkout/clean to undo)'; exit 0 }

$remote = (git -C $PUSHDIR ls-remote origin refs/heads/machine/integ-ioweb) -split "`t" | Select-Object -First 1
$local = git -C $PUSHDIR rev-parse HEAD
if ($remote -ne $local) { "remote $remote != local $local -- NOT COMMITTED"; exit 4 }
git -C $PUSHDIR add -A -- cpp web tools _tools machine_params workorder README.txt MANIFEST_MD5.tsv
git -C $PUSHDIR commit -q -F $MsgFile
if ($LASTEXITCODE) { throw 'commit failed' }
git -C $PUSHDIR push origin HEAD:machine/integ-ioweb
if ($LASTEXITCODE) { throw 'push failed' }
$after = (git -C $PUSHDIR ls-remote origin refs/heads/machine/integ-ioweb) -split "`t" | Select-Object -First 1
"pushed: $after (local $(git -C $PUSHDIR rev-parse HEAD))"
