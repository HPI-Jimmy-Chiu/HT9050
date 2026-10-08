# AI(W906-HTDESIGNER) 20261008 (EastSun「看一下網路上9050有沒有新版本 要一直持續更新」): D:\HP9050 kept equal to the HT9050
# machine's newest snapshot on GitLab (main: machines/HT9050/snapshot, mirrored from the machine every hour or so).
#   snapshot\machine_params\D_HT9045_system   -> <Root>\HT9045\system
#   snapshot\machine_params\D_HT9045_config   -> <Root>\HT9045\config
#   snapshot\machine_params\D_GPIB9045_system -> <Root>\GPIB9045\system
#   snapshot\machine_params\runcfg            -> <Root>\runcfg            (logs\ is not in it: this PC's logs stay)
#   snapshot\workorder\<recipe>\              -> <Root>\HT9045\IniData\Data\<recipe>   (the machine's current work order)
# Files of the snapshot are written over; files only this PC has are kept (nothing is deleted).
# The commit synced last is kept in <Root>\.htd_snapshot -- the same commit again = nothing copied (-Force: copy anyway).
# Refuses (exit 3) while wb_serve / wb_publish / wb_gateway run (they hold these files open).
param([Parameter(Mandatory = $true)][string]$Repo, [string]$Root = 'D:\HP9050', [string]$Remote = 'origin', [string]$Branch = 'main',
      [switch]$NoFetch, [switch]$Force, [switch]$NoProcessCheck)
$ErrorActionPreference = 'Stop'
function Say($t) { Write-Output "[9050 sync] $t" }
if (-not [IO.Path]::IsPathRooted($Root)) { Say "refused: '$Root' is not an absolute path"; exit 4 }
$Root = [IO.Path]::GetFullPath($Root).TrimEnd('\')
if (-not $NoProcessCheck) {
  $busy = Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match '^(wb_serve|wb_publish|wb_gateway)$' }
  if ($busy) { Say ('refused: still running: ' + (($busy | ForEach-Object { $_.ProcessName + '(' + $_.Id + ')' }) -join ', ')); exit 3 }
}
$eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
# (1008: the extension passes the C++ tree, a folder INSIDE the repository -- a path after "--" is taken from there, and
#  machines/HT9050/snapshot was never found: "no machines/HT9050/snapshot". The repository's own top folder, then)
$top = (& git -C $Repo rev-parse --show-toplevel 2>$null | Select-Object -First 1)
if ($top) { $Repo = $top }
if (-not $NoFetch) { & git -C $Repo fetch -q $Remote $Branch 2>&1 | ForEach-Object { Say "git: $_" } }
$ref = "$Remote/$Branch"
$commit = (& git -C $Repo log -1 --format=%H $ref -- machines/HT9050/snapshot 2>$null | Select-Object -First 1)
$when = (& git -C $Repo log -1 --format='%ad %s' --date=format:'%m-%d %H:%M' $ref -- machines/HT9050/snapshot 2>$null | Select-Object -First 1)
$ErrorActionPreference = $eap
if (-not $commit) { Say "no machines/HT9050/snapshot on $ref (fetch failed?)"; exit 5 }
$mark = Join-Path $Root '.htd_snapshot'
$last = if (Test-Path -LiteralPath $mark) { (Get-Content -LiteralPath $mark -TotalCount 1).Trim() } else { '' }
if ($last -eq $commit -and -not $Force) { Say "up to date: $($commit.Substring(0, 9)) ($when)"; exit 0 }

$tmp = Join-Path $env:TEMP ('htd_hp9050_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
# (1008 review: the unpacked snapshot (~20 MB) taken away on every way out -- 20 of them were left in %TEMP%)
function Done([int]$code) { Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue; exit $code }
$tar = Join-Path $tmp 'snap.zip'
# (a zip read by .NET: Windows' tar.exe cannot write the snapshot's Chinese file names -- "Invalid empty pathname")
& git -C $Repo archive --format=zip -o $tar $commit machines/HT9050/snapshot
if ($LASTEXITCODE -ne 0) { Say 'git archive failed'; Done 5 }
try { Add-Type -AssemblyName System.IO.Compression.FileSystem; [IO.Compression.ZipFile]::ExtractToDirectory($tar, $tmp, [Text.Encoding]::UTF8) }
catch { Say ('unzip failed: ' + $_.Exception.Message); Done 5 }
$S = Join-Path $tmp 'machines\HT9050\snapshot'
$P = Join-Path $S 'machine_params'
$pairs = @(
  @{ from = (Join-Path $P 'D_HT9045_system'); to = (Join-Path $Root 'HT9045\system') },
  @{ from = (Join-Path $P 'D_HT9045_config'); to = (Join-Path $Root 'HT9045\config') },
  @{ from = (Join-Path $P 'D_GPIB9045_system'); to = (Join-Path $Root 'GPIB9045\system') },
  @{ from = (Join-Path $P 'runcfg'); to = (Join-Path $Root 'runcfg') }
)
$W = Join-Path $S 'workorder'
if (Test-Path -LiteralPath $W) {
  Get-ChildItem -LiteralPath $W -Directory | ForEach-Object { $pairs += @{ from = $_.FullName; to = (Join-Path $Root ('HT9045\IniData\Data\' + $_.Name)) } }
}
$n = 0
foreach ($p in $pairs) {
  if (-not (Test-Path -LiteralPath $p.from)) { continue }
  # (robocopy: changed files only; /XO not used -- the machine's copy wins even when this PC's is newer)
  $out = & robocopy $p.from $p.to /E /IS /IT /R:1 /W:1 /NP /NDL /NJH /NJS /FP
  if ($LASTEXITCODE -ge 8) { Say "copy failed: $($p.from) -> $($p.to) (robocopy $LASTEXITCODE)"; Done 6 }
  $files = @($out | Where-Object { $_ -match '\S' })
  $n += $files.Count
  if ($files.Count) { Say ("{0}: {1} file(s)" -f $p.to, $files.Count) }
}
foreach ($f in @('SNAPSHOT_SOURCE.md', 'machine_params\README_PARAMS.txt', 'workorder\README_WORKORDER.txt')) {
  $src = Join-Path $S $f
  if (Test-Path -LiteralPath $src) { Copy-Item -LiteralPath $src -Destination (Join-Path $Root (Split-Path $f -Leaf)) -Force }
}
Set-Content -LiteralPath $mark -Value @($commit, $when) -Encoding ascii
Remove-Item -LiteralPath $tar -Force -ErrorAction SilentlyContinue
Say "synced to $($commit.Substring(0, 9)) ($when): $n file(s) written into $Root"
Done 0
