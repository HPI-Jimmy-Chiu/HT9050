# AI(W906-HTDESIGNER) 20261006: the 'wpf' view in a real VS Code (test\integration_wpf\index.js) -- two rows for a page,
# one page at a time, a C++ file closes the pages, an unsaved one stays. A throw-away --user-data-dir / --extensions-dir
# under %TEMP%; no folder opened; nothing saved. ASCII only (Windows PowerShell 5.1).
param([int]$TimeoutSec = 300)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$port = Resolve-Path (Join-Path $here '..\..\..')
$webRoot = Join-Path (Split-Path $port -Parent) 'web'
$code = if ($env:HTD_CODE_EXE) { $env:HTD_CODE_EXE } else { Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe' }   # (1008: HTD_CODE_EXE = a copy without the staged update)
$tmp = Join-Path $env:TEMP ('htd_vscode_wpfit_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
$settings = [ordered]@{
  'ht9045Designer.webRoot'            = [string]$webRoot
  'ht9045Designer.portRoot'           = [string]$port
  'ht9045Designer.defaultView'        = 'wpf'
  'ht9045Designer.layout.apply'       = $false
  'security.workspace.trust.enabled'  = $false
  'workbench.startupEditor'           = 'none'
  'telemetry.telemetryLevel'          = 'off'
  'update.mode'                       = 'none'
  'extensions.autoCheckUpdates'       = $false
  'extensions.autoUpdate'             = $false
  'files.autoSave'                    = 'off'
  'files.hotExit'                     = 'off'
}
[IO.File]::WriteAllText((Join-Path $tmp 'user\User\settings.json'), ($settings | ConvertTo-Json), (New-Object System.Text.UTF8Encoding($false)))
$report = Join-Path $tmp 'report.txt'
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$env:HTD_WEB_PAGE_DIR = Join-Path $webRoot 'page'
$env:HTD_PORT_ROOT = [string]$port
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_wpf')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
foreach ($v in 'HTD_IT_REPORT', 'HTD_WEB_PAGE_DIR', 'HTD_PORT_ROOT') { Remove-Item "Env:$v" -ErrorAction SilentlyContinue }
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
$text = Get-Content $report -Raw -Encoding utf8
$text
Write-Host "(scratch: $tmp)"
$m = [regex]::Match($text, 'RESULT: (\d+) pass, (\d+) fail')
if (-not $m.Success -or [int]$m.Groups[2].Value -gt 0) { exit 1 }
exit 0
