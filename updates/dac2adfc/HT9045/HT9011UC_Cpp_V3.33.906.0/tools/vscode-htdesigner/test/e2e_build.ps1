# AI(W906-HTDESIGNER) 20261001 (EastSun: new components / new events / other changes from the designer -> 9050 must
# still build AND run). End to end, on a THROW-AWAY git worktree of the port (never the machine's own tree):
#   1. e2e_build.js : the designer adds every toolbox component, every event it can, renames / resets / reuses one,
#                     then "Save All" (+ one marker line in each new handler = the user's code)
#   2. build        : build_nonoracle.bat quick (wb_serve) in that worktree
#   3. run          : D:\HT9045 system / config / IniData backed up; wb_serve (SIM build) started with the
#                     D:\HT9050\run sandbox's W906_* paths; e2e_run.js sends each wired event as the page would
#                     (WS htd.event); every new handler's marker must appear in wb_serve's stderr
#   4. restore      : D:\HT9045 back to the backup (robocopy /MIR), the worktree back to its commit
# ASCII only (PowerShell 5.1 reads a BOM-less file in the system code page).
#   -Port 8065  -Keep (leave the worktree changes for a look)  -Tree <port tree of the worktree>
param([int]$Port = 8065, [switch]$Keep, [string]$Tree = '')
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
if (-not $Tree) { $Tree = (Resolve-Path (Join-Path $here '..\..\..')).Path }
$wt = (Resolve-Path (Join-Path $Tree '..')).Path
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$work = Join-Path $env:TEMP 'htd_e2e'
New-Item -ItemType Directory -Force $work | Out-Null
$pass = 0; $fail = 0
function ok($c, $n, $x) { if ($c) { $script:pass++; Write-Host "PASS  $n   $x" } else { $script:fail++; Write-Host "FAIL  $n   $x" } }
function Node([string[]]$a) {
  $env:ELECTRON_RUN_AS_NODE = '1'
  $p = Start-Process -FilePath $code -ArgumentList ($a | ForEach-Object { '"' + $_ + '"' }) -Wait -PassThru -NoNewWindow
  $env:ELECTRON_RUN_AS_NODE = $null
  return $p.ExitCode
}

# --- 0. only a throw-away worktree, clean outside the extension
if ($wt -match '^[A-Za-z]:\\HT9045(\\|$)') { Write-Host "refusing: $wt is the machine's tree"; exit 2 }
$gd = (& git -C $wt rev-parse --git-dir) 2>$null
if (-not $gd -or $gd -notmatch 'worktrees') { Write-Host "refusing: $wt is not a git worktree (git-dir $gd)"; exit 2 }
$dirty = @(& git -C $wt status --porcelain | Where-Object { $_ -notmatch 'tools/vscode-htdesigner/' })
if ($dirty.Count) { Write-Host 'refusing: the worktree has other changes:'; $dirty | Select-Object -First 10; exit 2 }

$rep = Join-Path $work 'report.json'
$runOut = Join-Path $work 'run.json'
$bk = Join-Path $work 'HT9045_backup'
try {
  # --- 1. the designer's changes
  $ec = Node @((Join-Path $here 'e2e_build.js'), $rep)
  $R = Get-Content $rep -Raw -Encoding UTF8 | ConvertFrom-Json
  ok ($ec -eq 0 -and $R.counts.errors -eq 0) 'designer: ran with no error' ("added=$($R.counts.added) events=$($R.counts.events) wired=$($R.counts.wired) changes=$($R.counts.changes) refused=$($R.counts.refused)")
  $pages = @($R.pages | ForEach-Object { $_.page })
  foreach ($pg in $pages) {
    $n = @($R.added | Where-Object { $_.page -eq $pg }).Count
    ok ($n -gt 0) "designer: components added on $pg" "$n"
  }
  $outside = @($R.added | Where-Object { -not $_.inside })
  ok ($outside.Count -eq 0) 'designer: each component landed inside the container it was put into (a form, a page body without a form root, a tab sheet)' ((($R.added | Group-Object page | ForEach-Object { "$($_.Name): " + (($_.Group | ForEach-Object { "$($_.into)/$($_.kind)" } | Sort-Object -Unique) -join ',') }) -join ' | ') + $(if ($outside.Count) { ' | OUTSIDE: ' + (($outside | ForEach-Object { "$($_.page) $($_.id)" }) -join ',') } else { '' }))
  $moves = @($R.changes | Where-Object { $_.what -eq 'move into sheet' })
  ok ($moves.Count -ge 1 -and @($moves | Where-Object { -not $_.inside }).Count -eq 0) 'designer: a component moved into another tab sheet lands inside it (Blend reparent / the outline drag)' (($moves | ForEach-Object { "$($_.id) -> $($_.into) inside=$($_.inside) $($_.why)" }) -join ' | ')
  $kinds = @($R.added | ForEach-Object { $_.cls } | Sort-Object -Unique)
  ok ($kinds.Count -ge 12) 'designer: every toolbox kind added at least once' ($kinds -join ',')
  ok (@($R.changes | Where-Object { $_.what -eq 'rename' }).Count -ge 1 -and @($R.changes | Where-Object { $_.what -eq 'reset' }).Count -ge 1 -and @($R.changes | Where-Object { $_.what -eq 'use existing' }).Count -ge 1) 'designer: a handler renamed, an event reset, an existing handler picked' (($R.changes | ForEach-Object { $_.what }) -join ',')
  $badRefuse = @($R.refused | Where-Object { $_.why -notmatch 'vclcompat' })
  ok ($badRefuse.Count -eq 0) 'designer: every refusal is the "type not in the port" one (nothing else failed)' (($badRefuse | ForEach-Object { "$($_.page) $($_.what): $($_.why)" }) -join ' | ')

  # --- 2. build 9050
  $blog = Join-Path $work 'build.log'
  $bat = Join-Path $Tree 'build_nonoracle.bat'
  $t0 = Get-Date
  & cmd /c "cd /d `"$Tree`" && call `"$bat`" quick > `"$blog`" 2>&1"
  $bec = $LASTEXITCODE
  $errs = @(Get-Content $blog | Select-String ': error' | ForEach-Object { $_.Line })
  ok ($bec -eq 0 -and $errs.Count -eq 0) '9050 builds (build_nonoracle quick) with everything the designer added' ("exit $bec, $($errs.Count) errors, $([int]((Get-Date) - $t0).TotalSeconds) s" + $(if ($errs.Count) { ' | ' + ($errs | Select-Object -First 3) -join ' | ' } else { '' }))
  $exe = Join-Path $Tree 'build_nonoracle\wb_serve.exe'
  if ($bec -ne 0) { throw 'build failed' }

  # --- 3. run it
  $listen = netstat -ano | Select-String ":$Port\s.*LISTENING"
  if ($listen) { throw "port $Port is in use: $listen" }
  foreach ($s in 'system', 'config', 'IniData') {
    & robocopy "D:\HT9045\$s" "$bk\$s" /MIR /R:1 /W:1 /NP /NFL /NDL /NJH /NJS | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "backup of D:\HT9045\$s failed (robocopy $LASTEXITCODE)" }
  }
  $logBefore = @(Get-ChildItem 'D:\HT9045_Log' -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName + '|' + $_.Length })
  $RUN = 'D:\HT9050\run'
  $envs = @{ W906_GENERAL_INI_PATH = "$RUN\system\Gerneral.ini"; W906_SETUPINF_PATH = "$RUN\SetUp.inf"; W906_IOTABLE_PATH = "$RUN\system\IO_Table.csv"
    W906_MOTTABLE_PATH = "$RUN\system\Mot_Table.csv"; W906_AUTH_PATH = "$RUN\config\"; W906_E84DATA_ROOT = "$RUN\logs\E84DataTxt"; W906_TCPDATA_ROOT = "$RUN\logs\TCP_Data"
    W906_SUMMARYLOT_ROOT = "$RUN\logs\Summary_Lot"; W906_EVENTLOG_ROOT = "$RUN\logs\EventLogTxt"; W906_BINCOUNT_PATH = "$RUN\logs\BinCount.ini"; W906_PWBOOK_PATH = "$RUN\logs\pwbook.ini"
    W906_TEACH_INI_PATH = "$RUN\system\teach.ini"; W906_IOTIMING_LOG = "$RUN\logs\io_click_timing.csv"; W906_OPLOG_DIR = "$RUN\logs" }
  foreach ($k in $envs.Keys) { Set-Item "env:$k" $envs[$k] }
  $so = Join-Path $work 'serve_out.txt'; $se = Join-Path $work 'serve_err.txt'
  $sv = Start-Process -FilePath $exe -ArgumentList '--root', (Join-Path $wt 'web'), '--port', "$Port", '--seconds', '180' -WorkingDirectory $Tree -PassThru -NoNewWindow -RedirectStandardOutput $so -RedirectStandardError $se
  $up = $false
  for ($i = 0; $i -lt 90 -and -not $up; $i++) { Start-Sleep -Milliseconds 500; $up = [bool](netstat -ano | Select-String ":$Port\s.*LISTENING\s+$($sv.Id)\s*$") }
  ok $up "9050 runs: wb_serve (pid $($sv.Id)) listens on $Port itself" ''
  $htm = ''
  try { $htm = (Invoke-WebRequest "http://127.0.0.1:$Port/page/$($pages[0])" -UseBasicParsing -TimeoutSec 15).Content } catch { $htm = '' }
  ok ($htm -match 'htdCpp\(') "9050 serves the page the designer changed ($($pages[0]), with its htdCpp lines)" ''
  $rc = Node @((Join-Path $here 'e2e_run.js'), "$Port", $rep, $runOut)
  Start-Sleep -Milliseconds 800
  Stop-Process -Id $sv.Id -ErrorAction SilentlyContinue
  $null = $sv.WaitForExit(30000)
  $U = Get-Content $runOut -Raw -Encoding UTF8 | ConvertFrom-Json
  $err = Get-Content $se -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
  $outT = Get-Content $so -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
  $acks = @($U.acks)
  $nok = @($acks | Where-Object { -not $_.ok })
  ok ($rc -eq 0 -and $acks.Count -gt 0 -and $nok.Count -eq 0) 'run: every wired event sent as the page sends it (WS htd.event) is acknowledged ok' ("$($acks.Count) sent, $($nok.Count) refused" + $(if ($nok.Count) { ' | ' + (($nok | Select-Object -First 3 | ForEach-Object { "$($_.handler) $($_.event): $($_.error)" }) -join ' | ') } else { '' }))
  # each handler that kept its body (not reused, not reset) printed its marker
  $want = @($R.events | Where-Object { $_.wired -and -not $_.reset -and $_.marked } | ForEach-Object { "$($_.form)::$($_.handler)" } | Sort-Object -Unique)
  $miss = @($want | Where-Object { $err -notmatch ('HTD-E2E ' + [regex]::Escape($_) + '\r?\n') })
  ok ($want.Count -gt 0 -and $miss.Count -eq 0) 'run: each new C++ handler really ran in 9050 (its line in wb_serve stderr)' ("$($want.Count) handlers" + $(if ($miss.Count) { ', missing: ' + ($miss | Select-Object -First 5) -join ',' } else { '' }))
  $reused = @($R.events | Where-Object { $_.reused })
  if ($reused.Count) {
    $hn = "$($reused[0].form)::$($reused[0].handler)"
    $cnt = ([regex]::Matches($err, 'HTD-E2E ' + [regex]::Escape($hn) + '\r?\n')).Count
    ok ($cnt -ge 2) 'run: a handler picked for a second control''s event runs for both' "$hn x$cnt"
  }
  $crash = $outT -match '(?i)exception|access violation|terminate called'
  ok (-not $crash) 'run: no exception in wb_serve while the events ran' ''
  $U.acks | ConvertTo-Json -Depth 3 | Set-Content (Join-Path $work 'acks.json') -Encoding UTF8
  $logAfter = @(Get-ChildItem 'D:\HT9045_Log' -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName + '|' + $_.Length })
  $logChanged = @($logAfter | Where-Object { $logBefore -notcontains $_ })
  Write-Host "note: D:\HT9045_Log files new or grown during the run: $($logChanged.Count) (logs, not restored)"
}
catch {
  $fail++
  Write-Host "FAIL  $($_.Exception.Message)"
}
finally {
  if (Get-Variable sv -ErrorAction SilentlyContinue) { if ($sv -and -not $sv.HasExited) { Stop-Process -Id $sv.Id -ErrorAction SilentlyContinue; $null = $sv.WaitForExit(30000) } }
  # --- 4. restore
  if (Test-Path (Join-Path $bk 'IniData')) {
    $changed = @()
    foreach ($s in 'system', 'config', 'IniData') {
      $l = & robocopy "$bk\$s" "D:\HT9045\$s" /MIR /L /R:1 /W:1 /NP /NDL /NJH /NJS /FP
      $changed += @($l | Where-Object { $_ -match '\S' -and $_ -notmatch '^\s*$' } | ForEach-Object { $_.Trim() })
      & robocopy "$bk\$s" "D:\HT9045\$s" /MIR /R:1 /W:1 /NP /NFL /NDL /NJH /NJS | Out-Null
      if ($LASTEXITCODE -ge 8) { Write-Host "FAIL  restore of D:\HT9045\$s (robocopy $LASTEXITCODE)"; $fail++ }
    }
    $left = 0
    foreach ($s in 'system', 'config', 'IniData') { $left += @(& robocopy "$bk\$s" "D:\HT9045\$s" /MIR /L /R:1 /W:1 /NP /NDL /NJH /NJS /FP | Where-Object { $_ -match '\S' }).Count }
    ok ($left -eq 0) 'restore: D:\HT9045 system / config / IniData back exactly as before the run' ("$($changed.Count) file(s) the run had touched, restored: " + (($changed | Select-Object -First 6) -join ' ; '))
  }
  if (-not $Keep) {
    $mine = @(& git -C $wt status --porcelain | Where-Object { $_ -notmatch 'tools/vscode-htdesigner/' })
    foreach ($line in $mine) {
      $f = $line.Substring(3).Trim('"')
      if ($line.StartsWith('??')) { & git -C $wt clean -fdq -- $f } else { & git -C $wt checkout -- $f }
    }
    $still = @(& git -C $wt status --porcelain | Where-Object { $_ -notmatch 'tools/vscode-htdesigner/' })
    ok ($still.Count -eq 0) 'restore: the worktree back to its commit (outside the extension)' "$($mine.Count) path(s) reverted"
  }
}
Write-Host "RESULT: $pass pass, $fail fail"
if ($fail) { exit 1 }
exit 0
