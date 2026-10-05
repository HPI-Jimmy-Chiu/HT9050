# AI(W906-HTDESIGNER) 20261005 (ES02): every toolbar button pressed in a real VS Code (test\integration_run\index.js).
# A separate VS Code with a throw-away --user-data-dir and --extensions-dir under %TEMP%; the C++ debugger extension
# (ms-vscode.cpptools) is COPIED in from the user's extensions (once, cached in %TEMP%\htd_it_cpptools) -- the run button starts
# wb_serve under gdb through it. The user's own window, settings and extensions are not touched.
# the run button really builds wb_serve (simulation + Debug -> build_dbg_nonoracle) and starts it with the tree's own launch entry
# (W906_* paths = the runcfg sandbox). ASCII only (Windows PowerShell 5.1).
param([int]$TimeoutSec = 3600)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$port = Resolve-Path (Join-Path $here '..\..\..')
$webRoot = Join-Path (Split-Path $port -Parent) 'web'
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$cpp = Get-ChildItem (Join-Path $env:USERPROFILE '.vscode\extensions') -Directory -Filter 'ms-vscode.cpptools-*-win32-x64' | Sort-Object Name | Select-Object -Last 1
if (-not $cpp) { Write-Host 'FATAL: ms-vscode.cpptools is not installed (the C++ debugger the run buttons start wb_serve with)'; exit 2 }
$cache = Join-Path $env:TEMP 'htd_it_cpptools'
$cacheExt = Join-Path $cache $cpp.Name
if (-not (Test-Path (Join-Path $cacheExt 'package.json'))) {
  New-Item -ItemType Directory -Force $cache | Out-Null
  robocopy $cpp.FullName $cacheExt /E /NFL /NDL /NJH /NJS /NP | Out-Null
}
$tmp = Join-Path $env:TEMP ('htd_vscode_runit_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
# (a directory junction: the copy is shared between runs, nothing is copied again)
cmd /c mklink /J "$(Join-Path $tmp ('ext\' + $cpp.Name))" "$cacheExt" | Out-Null
$settings = [ordered]@{
  'ht9045Designer.webRoot'            = [string]$webRoot
  'ht9045Designer.portRoot'           = [string]$port
  'ht9045Designer.defaultView'        = 'design'
  'ht9045Designer.run.simulation'     = $true
  'ht9045Designer.run.debug'          = $true
  'ht9045Designer.layout.apply'       = $false
  'ht9045Designer.build.window'       = $false
  'security.workspace.trust.enabled'  = $false
  'workbench.startupEditor'           = 'none'
  'telemetry.telemetryLevel'          = 'off'
  'update.mode'                       = 'none'
  'extensions.autoCheckUpdates'       = $false
  'extensions.autoUpdate'             = $false
  'task.allowAutomaticTasks'          = 'off'
  'files.autoSave'                    = 'off'
  'files.hotExit'                     = 'off'
  'debug.openDebug'                   = 'neverOpen'
}
[IO.File]::WriteAllText((Join-Path $tmp 'user\User\settings.json'), ($settings | ConvertTo-Json), (New-Object System.Text.UTF8Encoding($false)))
$report = Join-Path $tmp 'report.txt'
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_run')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
$env:HTD_IT_REPORT = $null
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
$text = Get-Content $report -Raw -Encoding utf8
$text
Write-Host "(scratch: $tmp)"
$m = [regex]::Match($text, 'RESULT: (\d+) pass, (\d+) fail')
if (-not $m.Success -or [int]$m.Groups[2].Value -gt 0) { exit 1 }
exit 0
