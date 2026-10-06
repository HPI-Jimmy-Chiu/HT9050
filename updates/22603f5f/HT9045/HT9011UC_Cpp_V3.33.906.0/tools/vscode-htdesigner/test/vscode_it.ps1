# AI(W906-HTDESIGNER) 20260929: run test\integration\index.js in a real VS Code window.
# A separate VS Code instance with a throw-away --user-data-dir and --extensions-dir
# under %TEMP%: the user's own window, settings and installed extensions are not
# touched. No folder is opened (so no workspace tasks can run); the roots are given
# as user settings instead. The window closes by itself when the test ends.
param(
  [string]$Page = '',
  # other pages to open one after another (generated / hand-written / JS-built / iframe)
  [string[]]$Sweep = @('main.html', 'HW.IoSetView.html', 'Setup.Speed.html', 'Data.LotInfo.html',
    'Main.MotionView.html', 'IDE.MotionView9050-UPH.html', 'Status.Security.html', 'HW.MotorTest.html'),
  [switch]$NoSweep,
  # every page of web\page (a long run: use -TimeoutSec 1200)
  [switch]$AllPages,
  # (AI 20261002 machine: 180 s killed the default run -- 0.157's 159 checks take ~190 s here)
  [int]$TimeoutSec = 600
)
if ($NoSweep) { $Sweep = @() }
# "powershell -File" hands "-Sweep a.html,b.html" over as one string
if ($Sweep.Count -eq 1 -and $Sweep[0] -match ',') { $Sweep = @($Sweep[0] -split ',' | ForEach-Object { $_.Trim() } | Where-Object { $_ }) }
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$port = Resolve-Path (Join-Path $here '..\..\..')
$webRoot = Join-Path (Split-Path $port -Parent) 'web'
if (-not $Page) { $Page = Join-Path $webRoot 'page\Setup.HotPlate.html' }
# (AI 20261001: next to the port tree first -- a GitLab clone has the golden trees beside it -- then one level up, as on the machine)
$golden = @(Get-ChildItem (Split-Path $port -Parent) -Directory -Filter 'HT9011UC_Code_V*') + @(Get-ChildItem (Split-Path (Split-Path $port -Parent) -Parent) -Directory -Filter 'HT9011UC_Code_V*') | Select-Object -First 1
# (AI 20261005 machine: the tree moved to another disk, a junction left where it was and the golden tree beside the junction
#  -- the same search from a junction in the first two levels of a drive that points at the tree's folder: lib/roots.js viaLinks)
if (-not $golden) {
  $treeUp = (Split-Path $port -Parent).ToLower()
  $links = foreach ($dr in (Get-PSDrive -PSProvider FileSystem | Where-Object { $_.Root -match '^[C-Z]:\\$' })) {
    $l1 = @(Get-ChildItem $dr.Root -Directory -Force -ErrorAction SilentlyContinue | Where-Object { $_.Name -notmatch '^(\$|Windows$|Program Files|ProgramData$|Users$|System Volume Information$|Recovery$)' })
    $l1 + @($l1 | Where-Object { -not $_.LinkType } | ForEach-Object { Get-ChildItem $_.FullName -Directory -Force -ErrorAction SilentlyContinue | Select-Object -First 400 })
  }
  foreach ($lk in @($links | Where-Object { $_.LinkType })) {
    $tg = @($lk.Target)[0]
    if ($tg -and ([string]$tg).TrimEnd('\').ToLower() -eq $treeUp) {
      $golden = @(Get-ChildItem (Split-Path $lk.FullName -Parent) -Directory -Filter 'HT9011UC_Code_V*') | Select-Object -First 1
      if ($golden) { break }
    }
  }
}
if ($AllPages) {
  # every page of web\page, plus web\*.html (as ..\name.html, relative to web\page)
  $Sweep = @(Get-ChildItem (Split-Path $Page -Parent) -File -Filter *.html | Where-Object { $_.Name -ne (Split-Path $Page -Leaf) } | Sort-Object Name | ForEach-Object { $_.Name }) +
    @(Get-ChildItem $webRoot -File -Filter *.html | Sort-Object Name | ForEach-Object { '..\' + $_.Name })
  if ($TimeoutSec -lt 900) { $TimeoutSec = 1200 }
}
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'

$tmp = Join-Path $env:TEMP ('htd_vscode_it_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
$settings = [ordered]@{
  'ht9045Designer.webRoot'            = [string]$webRoot
  'ht9045Designer.portRoot'           = [string]$port
  'ht9045Designer.goldenRoot'         = [string]$golden.FullName
  'ht9045Designer.eventJump'          = 'golden'
  # AI(W906-HTDESIGNER) 20261002 (machine): the checks below were written for the design-only / beside layout; since
  # 0.140 the default ('wpf') closes the page when a C++ / .dfm file opens, so "opens beside the designer" and the
  # steps after it no longer meant what they test. The wpf layout needs checks of its own (HANDOVER section 7).
  'ht9045Designer.defaultView'        = 'design'
  'security.workspace.trust.enabled'  = $false
  'workbench.startupEditor'           = 'none'
  'telemetry.telemetryLevel'          = 'off'
  'update.mode'                       = 'none'
  'extensions.autoCheckUpdates'       = $false
  'extensions.autoUpdate'             = $false
  'task.allowAutomaticTasks'          = 'off'
  'files.autoSave'                    = 'off'
  'files.hotExit'                     = 'off'
}
[IO.File]::WriteAllText((Join-Path $tmp 'user\User\settings.json'), ($settings | ConvertTo-Json), (New-Object System.Text.UTF8Encoding($false)))

$report = Join-Path $tmp 'report.txt'
# the files the test opens must be byte-identical afterwards
$watched = @($Page, (Join-Path $golden.FullName 'cHotPlate.cpp'), (Join-Path $golden.FullName 'cHotPlate.dfm'), (Join-Path $port 'tools\wb_serve.cpp'), (Join-Path $port 'CMakeLists.txt'), (Join-Path $port 'forms\fHotPlate.h'), (Join-Path $port 'forms\fHotPlate.cpp')) + @($Sweep | ForEach-Object { Join-Path (Split-Path $Page -Parent) $_ })
# the designer's generated C++ table must not appear on disk (the test saves nothing)
$genFile = Join-Path $port 'HtdEvents\HtdEvents.gen.cpp'
$genBefore = Test-Path $genFile
$before = @{}
foreach ($f in $watched) { $before[$f] = (Get-FileHash $f -Algorithm MD5).Hash }
$t0 = Get-Date
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$env:HTD_IT_PAGE = $Page
$env:HTD_IT_SWEEP = ($Sweep -join ';')
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
$env:HTD_IT_REPORT = $null
$env:HTD_IT_PAGE = $null
$env:HTD_IT_SWEEP = $null
# judge by the report, not the process: Code.exe's own exit code is not reliable here
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
$text = Get-Content $report -Raw -Encoding utf8
$text
Write-Host "(scratch: $tmp)"
$changed = @($watched | Where-Object { (Get-FileHash $_ -Algorithm MD5).Hash -ne $before[$_] })
# the pages and the BCB6 files: nobody else writes them while the test runs -> a change is a FAIL.
# a file of the port tree (tools\wb_serve.cpp) is being worked on by other sessions in this
# worktree: when the test itself saved nothing (its NO-SAVES line), a change there is theirs.
$noSaves = $text -match 'PASS  NO-SAVES'
# a page (web\ is its own git repo) changed AND committed by someone else while the test ran:
# its content is what its repo holds and its last commit is from after the test started. The
# test saved nothing and never commits, so that change is theirs (20260930: another session
# merged laptop web packages during the sweep). Anything else stays a FAIL.
function CommittedSince([string]$f, [datetime]$since) {
  $dir = Split-Path $f -Parent
  $st = & git -C $dir status --porcelain -- $f 2>$null
  if ($LASTEXITCODE -ne 0 -or $st) { return $false }
  $ct = & git -C $dir log -1 --format='%ct' -- $f 2>$null
  if (-not $ct) { return $false }
  return ([DateTimeOffset]::FromUnixTimeSeconds([int64]$ct).LocalDateTime -ge $since.AddSeconds(-5))
}
# ... or put BACK to what its repo holds while the test ran: before the test it held someone's uncommitted work,
# now it is clean again (20260930 14:39: another session reverted HW.IoSetView.html mid-run; the test saved
# nothing, and nothing it does can write a page's committed text back byte for byte).
function BackToCommitted([string]$f) {
  $dir = Split-Path $f -Parent
  $st = & git -C $dir status --porcelain -- $f 2>$null
  return ($LASTEXITCODE -eq 0 -and -not $st -and (& git -C $dir ls-files -- $f 2>$null))
}
# ... or changed on disk (uncommitted) when the extension has NO way to write an existing file but a save: no fs write
# call and no WorkspaceEdit delete / rename anywhere in extension.js or lib\ (a createFile only makes a NEW file), and
# the test saved nothing (20260930 15:12: another session added a button to HW.MotorTest.html during the sweep)
function ExtensionCannotWrite() {
  $src = @(Join-Path $ext 'extension.js') + @(Get-ChildItem (Join-Path $ext 'lib') -Filter *.js | ForEach-Object { $_.FullName })
  foreach ($s in $src) {
    $t = [IO.File]::ReadAllText($s)
    # (a CALL -- fs.writeFile(...), fsp.rename(...) -- not a method of the same name: the read-only golden file
    # provider's writeFile(uri) { throw NoPermissions } refuses writes)
    if ($t -match '\.\s*(writeFile|writeFileSync|appendFile|appendFileSync|rename|renameSync|copyFile|copyFileSync|unlink|unlinkSync|rm|rmSync|rmdir|rmdirSync|createWriteStream|truncate|truncateSync)\s*\(' -or
        $t -match 'workspace\.fs\.(write|delete|rename|copy)' -or $t -match '\.(deleteFile|renameFile)\s*\(') { return $false }
  }
  return $true
}
$cannotWrite = ExtensionCannotWrite
$strict = @($changed | Where-Object { -not ($noSaves -and (($_.StartsWith([string]$port, [StringComparison]::OrdinalIgnoreCase) -and $_ -match '(\.(cpp|h)|CMakeLists\.txt)$') -or (CommittedSince $_ $t0) -or (BackToCommitted $_) -or $cannotWrite)) })
if ((Test-Path $genFile) -and -not $genBefore) { Write-Host ('FAIL  the generated file appeared on disk: ' + $genFile); exit 1 }
$theirs = @($changed | Where-Object { $strict -notcontains $_ })
foreach ($f in $theirs) {
  $who = & git -C (Split-Path $f -Parent) log -1 --format='%h %ci %s' -- $f 2>$null
  $st = & git -C (Split-Path $f -Parent) diff --shortstat -- $f 2>$null
  Write-Host ('WARN  changed during the test by someone else (this test saved nothing' + $(if ($cannotWrite) { '; the extension writes no file itself' } else { '' }) + '): ' + $f + '   ' + $(if ($st) { 'uncommitted:' + $st } else { 'last commit: ' + $who }) + '   written ' + (Get-Item $f).LastWriteTime.ToString('HH:mm:ss'))
}
if ($strict.Count) { Write-Host ('FAIL  files changed by the test: ' + ($strict -join ', ')); exit 1 }
Write-Host ('PASS  unchanged (MD5): ' + (($watched | Where-Object { $theirs -notcontains $_ } | ForEach-Object { Split-Path $_ -Leaf }) -join ', '))
$m = [regex]::Match($text, 'RESULT: (\d+) pass, (\d+) fail')
if (-not $m.Success -or [int]$m.Groups[2].Value -gt 0) { exit 1 }
exit 0
