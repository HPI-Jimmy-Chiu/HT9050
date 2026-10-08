# AI(W906-HTDESIGNER) 20261007 (EastSun "驗證功能"): the 0.245 C++ checks / build error list / new items in a REAL VS Code
# (test\integration_cpp\index.js). A separate VS Code with a throw-away --user-data-dir and --extensions-dir under %TEMP%:
# the user's own window, settings and extensions are not touched; no folder is opened (no workspace task can run).
# The real file the test edits (Public\cBootLog.cpp) is never saved -- checked byte for byte here too.
param([int]$TimeoutSec = 300)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$port = Resolve-Path (Join-Path $here '..\..\..')
$webRoot = Join-Path (Split-Path $port -Parent) 'web'
$golden = @(Get-ChildItem (Split-Path $port -Parent) -Directory -Filter 'HT9011UC_Code_V*') | Select-Object -First 1
$code = if ($env:HTD_CODE_EXE) { $env:HTD_CODE_EXE } else { Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe' }   # (1008: HTD_CODE_EXE = a copy without the staged update)
$tmp = Join-Path $env:TEMP ('htd_cpp_it_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
# (1008 gap list #7: the C/C++ extension too -- the IntelliSense provider registers with it; copied once, cached)
$cpp = Get-ChildItem (Join-Path $env:USERPROFILE '.vscode\extensions') -Directory -Filter 'ms-vscode.cpptools-*-win32-x64' | Sort-Object Name | Select-Object -Last 1
if ($cpp) {
  $cacheExt = Join-Path (Join-Path $env:TEMP 'htd_it_cpptools') $cpp.Name
  if (-not (Test-Path (Join-Path $cacheExt 'package.json'))) { New-Item -ItemType Directory -Force (Split-Path $cacheExt) | Out-Null; robocopy $cpp.FullName $cacheExt /E /NFL /NDL /NJH /NJS /NP | Out-Null }
  cmd /c mklink /J "$(Join-Path $tmp ('ext\' + $cpp.Name))" "$cacheExt" | Out-Null
}
$settings = [ordered]@{
  'ht9045Designer.webRoot'           = [string]$webRoot
  'ht9045Designer.portRoot'          = [string]$port
  'ht9045Designer.goldenRoot'        = [string]$(if ($golden) { $golden.FullName } else { '' })
  'security.workspace.trust.enabled' = $false
  'workbench.startupEditor'          = 'none'
  'telemetry.telemetryLevel'         = 'off'
  'update.mode'                      = 'none'
  'extensions.autoCheckUpdates'      = $false
  'extensions.autoUpdate'            = $false
  'task.allowAutomaticTasks'         = 'off'
  'files.autoSave'                   = 'off'
  'files.hotExit'                    = 'off'
}
[IO.File]::WriteAllText((Join-Path $tmp 'user\User\settings.json'), ($settings | ConvertTo-Json), (New-Object System.Text.UTF8Encoding($false)))
$report = Join-Path $tmp 'report.txt'
$real = Join-Path $port 'Public\cBootLog.cpp'
$before = (Get-FileHash $real -Algorithm MD5).Hash
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_cpp')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
$env:HTD_IT_REPORT = $null
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
$text = Get-Content $report -Raw -Encoding utf8
$text
$after = (Get-FileHash $real -Algorithm MD5).Hash
if ($after -ne $before) { Write-Host "FAIL  $real changed on disk"; exit 1 }
Write-Host "PASS  $real unchanged on disk (MD5 $after)"
Write-Host "(scratch: $tmp)"
if ($text -match 'RESULT: \d+ pass, 0 fail') { exit 0 } else { exit 1 }
