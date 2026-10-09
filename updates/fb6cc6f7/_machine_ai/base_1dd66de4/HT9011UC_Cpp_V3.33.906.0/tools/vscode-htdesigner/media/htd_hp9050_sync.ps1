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
      [switch]$NoFetch, [switch]$Force, [switch]$NoProcessCheck, [string]$Root9045 = '')
$ErrorActionPreference = 'Stop'
# (1009 review: the messages in UTF-8 -- the extension reads them so; a Chinese recipe folder came out garbled)
try { [Console]::OutputEncoding = [Text.Encoding]::UTF8 } catch { }
function Say($t) { Write-Output "[9050 sync] $t" }
# (1009 review (windows #5): one sync at a time on this PC -- two windows' F5 for 9050 at once both fetched and copied into
#  the same folders, and the first to finish removed the in-progress mark while the other was still writing. The second
#  waits for the first (2.5 min at most), then syncs (nothing left to copy); still busy = refused (8: the start stops))
$syncMx = New-Object System.Threading.Mutex($false, 'Local\htd_hp9050_sync')
$syncOwn = $false
# (1009 second review (switch #6): 60 s -- the extension ends the whole sync at 180 s; a 150 s wait left 30 s to copy and the
#  second window's sync was killed half way)
try { $syncOwn = $syncMx.WaitOne(60000) } catch [System.Threading.AbandonedMutexException] { $syncOwn = $true }
if (-not $syncOwn) { Say 'refused: another 9050 sync is still running (another VS Code window?)'; exit 8 }
if (-not [IO.Path]::IsPathRooted($Root)) { Say "refused: '$Root' is not an absolute path"; exit 4 }
$Root = [IO.Path]::GetFullPath($Root).TrimEnd('\')
# (1008 review: never into the live machine folders -- a Root of "D:\" or "D:\HT9045\.." made <Root>\HT9045\system the live
#  D:\HT9045\system, and the snapshot was written over this PC's own parameters before the switch's own checks ran)
if ($Root -match '^[A-Za-z]:$') { Say "refused: '$Root\' is a drive root"; exit 4 }
$live = @('D:\HT9045\system', 'D:\HT9045\config', 'D:\HT9045\IniData', 'D:\GPIB9045\system', 'D:\HT9045_Log', 'D:\HT9045')
foreach ($sub in @('HT9045\system', 'HT9045\config', 'GPIB9045\system', 'runcfg', 'HT9045\IniData')) {
  $t = (Join-Path $Root $sub).TrimEnd('\')
  foreach ($l in $live) {
    if ($t -ieq $l -or $t.StartsWith($l + '\', [StringComparison]::OrdinalIgnoreCase) -or $l.StartsWith($t + '\', [StringComparison]::OrdinalIgnoreCase)) { Say "refused: '$t' is a live machine folder ($l)"; exit 4 }
  }
}
# (1009 review #2: never HT9045's own data -- hp9050Root set to D:\HP9045 (hp9045Root's default) wrote the 9050 snapshot over
#  HT9045's real parameters, before the switch's own "the two roots are the same" check ran)
if ($Root9045) {
  if (-not [IO.Path]::IsPathRooted($Root9045)) { Say "refused: '$Root9045' (HT9045's data) is not an absolute path"; exit 4 }
  $r45 = [IO.Path]::GetFullPath($Root9045).TrimEnd('\')
  if ($Root -ieq $r45 -or $Root.StartsWith($r45 + '\', [StringComparison]::OrdinalIgnoreCase) -or $r45.StartsWith($Root + '\', [StringComparison]::OrdinalIgnoreCase)) {
    Say "refused: '$Root' is HT9045's data folder ($r45) or overlaps it -- set hp9050Root to a folder of its own"; exit 4
  }
}
if (-not $NoProcessCheck) {
  # (1009 review #3: the same programs as the switch -- the GPIB ones and BCB6's HT9045.exe hold these files too; and the
  #  tests / probes that write IniData)
  $busy = Get-Process -ErrorAction SilentlyContinue | Where-Object { ($_.ProcessName -match '^(wb_serve|wb_publish|wb_gateway|ioweb_probe|pci1203_linkprobe|test_.*|HT9045.*|H904\d.*GPIB.*)$' -or ($_.Path -and $_.Path -match '^[A-Za-z]:\\(HT9045|GPIB9045)\\')) }
  if ($busy) { Say ('refused: still running: ' + (($busy | ForEach-Object { $_.ProcessName + '(' + $_.Id + ')' }) -join ', ')); exit 3 }
}
$eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
# (1008: the extension passes the C++ tree, a folder INSIDE the repository -- a path after "--" is taken from there, and
#  machines/HT9050/snapshot was never found: "no machines/HT9050/snapshot". The repository's own top folder, then)
$top = (& git -C $Repo rev-parse --show-toplevel 2>$null | Select-Object -First 1)
if ($top) { $Repo = $top }
if (-not $NoFetch) {
  & git -C $Repo fetch -q $Remote $Branch 2>&1 | ForEach-Object { Say "git: $_" }
  # (1009 second review (switch #5): a failed fetch is not "up to date" -- 5 = started on what is there, said)
  if ($LASTEXITCODE -ne 0) { $ErrorActionPreference = $eap; Say "fetch failed (git $LASTEXITCODE): GitLab not reached, $Root not updated"; exit 5 }
}
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
# (1009 second review (switch #4): any error from here on goes through Done -- the unpacked snapshot is not left in %TEMP%)
#  (a trap covers the whole script: before Done exists it just ends)
trap { Say ('failed: ' + $_.Exception.Message); if (Get-Command Done -ErrorAction SilentlyContinue) { Done 1 } else { exit 1 } }
# (… a new hp9050Root: made, not "DirectoryNotFound" at the in-progress mark)
New-Item -ItemType Directory -Force -Path $Root | Out-Null
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
# (1008 review: this PC's own changes (teach.ini, SetUp.inf, Gerneral.ini, Mot_Table) were written over without a copy --
#  every file the snapshot is about to replace is kept first in <Root>\.htd_backup\<time>\ (the newest 10 kept))
$bk = Join-Path $Root ('.htd_backup\' + (Get-Date -Format 'yyyyMMdd_HHmmss'))
$nb = 0
$nbFail = 0
foreach ($p in $pairs) {
  if (-not (Test-Path -LiteralPath $p.from) -or -not (Test-Path -LiteralPath $p.to)) { continue }
  # (the snapshot's own files, compared with this PC's by size then content -- robocopy's /L list came in the console's
  #  code page and a Chinese name turned into characters a path cannot have, which stopped the whole sync)
  foreach ($sf in @(Get-ChildItem -LiteralPath $p.from -Recurse -File -ErrorAction SilentlyContinue)) {
    try {
      $rel = $sf.FullName.Substring($p.from.Length).TrimStart('\')
      $dst = Join-Path $p.to $rel
      if (-not (Test-Path -LiteralPath $dst -PathType Leaf)) { continue }
      $df = Get-Item -LiteralPath $dst
      if ($df.Length -eq $sf.Length -and (Get-FileHash -LiteralPath $dst -Algorithm MD5).Hash -eq (Get-FileHash -LiteralPath $sf.FullName -Algorithm MD5).Hash) { continue }
      $keep = Join-Path $bk ($dst.Substring($Root.Length).TrimStart('\'))
      New-Item -ItemType Directory -Force -Path (Split-Path $keep -Parent) | Out-Null
      Copy-Item -LiteralPath $dst -Destination $keep -Force
      $nb++
    } catch { $nbFail++; Say ("could not keep a copy of " + $sf.Name + ": " + $_.Exception.Message) }
  }
}
# (1009 review #4: a file that could not be kept is not written over -- this PC's own change would have no copy)
if ($nbFail) { Say "refused: $nbFail file(s) this snapshot replaces could not be kept in $bk -- nothing copied (disk full? locked?)"; Done 7 }
if ($nb) {
  Say "kept $nb file(s) this snapshot replaces in $bk"
  # (1009 review #5: the newest 10 AND everything from the last 30 days kept -- with a sync every few minutes the copy of
  #  this PC's own change went after ten of them)
  $old = (Get-Date).AddDays(-30).ToString('yyyyMMdd_HHmmss')
  $all = @(Get-ChildItem -LiteralPath (Join-Path $Root '.htd_backup') -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending)
  if ($all.Count -gt 10) { $all | Select-Object -Skip 10 | Where-Object { $_.Name -lt $old } | ForEach-Object { Remove-Item -LiteralPath $_.FullName -Recurse -Force -ErrorAction SilentlyContinue } }
}
# (1009 review (F5 #5): a mark while files are being written -- a sync killed or failed part way leaves it, and the extension
#  starts nothing on those folders until a sync goes through)
$busyMark = Join-Path $Root '.htd_sync_inprogress'
Set-Content -LiteralPath $busyMark -Value (Get-Date -Format s) -Encoding ascii
foreach ($p in $pairs) {
  if (-not (Test-Path -LiteralPath $p.from)) { continue }
  # (robocopy: changed files only; /XO not used -- the machine's copy wins even when this PC's is newer)
  # (1009 review (F5 #9): no /IS /IT -- every file of the snapshot was written again each time, the count meant nothing)
  $out = & robocopy $p.from $p.to /E /R:1 /W:1 /NP /NDL /NJH /NJS /FP
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
Remove-Item -LiteralPath $busyMark -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $tar -Force -ErrorAction SilentlyContinue
Say "synced to $($commit.Substring(0, 9)) ($when): $n file(s) written into $Root"
Done 0
