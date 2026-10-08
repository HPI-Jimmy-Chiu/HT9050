# AI(W906-HTDESIGNER) 20261008 (full test, audit B5): the BCB keys land in the keybindings.json of the VS Code profile the
# window runs -- a real VS Code started twice in a scratch user folder: once with --profile=htdprof, once without
# (test\integration_profile\index.js). Nothing of the user's own VS Code is touched. ASCII only (Windows PowerShell 5.1).
# -CodeExe / HTD_CODE_EXE: another Code.exe (a copy without the staged update, see vscode_dbg_it.ps1).
param([int]$TimeoutSec = 180, [string]$CodeExe = '')
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$code = if ($CodeExe) { $CodeExe } elseif ($env:HTD_CODE_EXE) { $env:HTD_CODE_EXE } else { Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe' }
$all = 0; $bad = 0
foreach ($prof in @('htdprof', '')) {
  $tmp = Join-Path $env:TEMP ('htd_profile_it_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
  New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
  $report = Join-Path $tmp 'report.txt'
  $env:ELECTRON_RUN_AS_NODE = $null
  $env:HTD_IT_REPORT = $report
  $env:HTD_USER_DIR = Join-Path $tmp 'user'
  $env:HTD_PROFILE = $prof
  $a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
    "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_profile')`"",
    '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
  if ($prof) { $a += "--profile=$prof" }
  $p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
  if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
  foreach ($v in 'HTD_IT_REPORT', 'HTD_USER_DIR', 'HTD_PROFILE') { Remove-Item "Env:$v" -ErrorAction SilentlyContinue }
  if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; $bad++; continue }
  $text = Get-Content $report -Raw -Encoding utf8
  $text
  if ($text -match 'RESULT: (\d+) pass, (\d+) fail') { $all += [int]$Matches[1]; $bad += [int]$Matches[2] } else { $bad++ }
}
Write-Host "RESULT: $all pass, $bad fail"
if ($bad) { exit 1 } else { exit 0 }
