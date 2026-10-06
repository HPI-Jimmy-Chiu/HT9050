# AI(W906-HTDESIGNER) 20261006 (ES02): every property row of every component class, in a real VS Code
# (test\integration_props\index.js). -Mode dump | replay ; -Dir = the folder with plan.json / replay.json (written by
# test\props_enum.ps1). A throw-away --user-data-dir / --extensions-dir under %TEMP%; nothing is saved (the page files
# are checked byte for byte afterwards). ASCII only (Windows PowerShell 5.1).
param([Parameter(Mandatory = $true)][string]$Mode, [Parameter(Mandatory = $true)][string]$Dir, [int]$TimeoutSec = 3600)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$ext = Resolve-Path (Join-Path $here '..')
$port = Resolve-Path (Join-Path $here '..\..\..')
$webRoot = Join-Path (Split-Path $port -Parent) 'web'
$golden = @(Get-ChildItem (Split-Path $port -Parent) -Directory -Filter 'HT9011UC_Code_V*') | Select-Object -First 1
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$tmp = Join-Path $env:TEMP ('htd_vscode_it_props_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $tmp 'user\User'), (Join-Path $tmp 'ext') | Out-Null
$settings = [ordered]@{
  'ht9045Designer.webRoot'            = [string]$webRoot
  'ht9045Designer.portRoot'           = [string]$port
  'ht9045Designer.goldenRoot'         = [string]$golden.FullName
  'ht9045Designer.defaultView'        = 'design'
  'ht9045Designer.layout.apply'       = $false
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
$pagesDir = Join-Path $webRoot 'page'
$before = @{}
foreach ($f in Get-ChildItem $pagesDir -File -Filter *.html) { $before[$f.FullName] = (Get-FileHash $f.FullName -Algorithm MD5).Hash }
$report = Join-Path $tmp 'report.txt'
$env:ELECTRON_RUN_AS_NODE = $null
$env:HTD_IT_REPORT = $report
$env:HTD_PROPS_MODE = $Mode
$env:HTD_PROPS_DIR = (Resolve-Path $Dir).Path
$a = @("--user-data-dir=`"$(Join-Path $tmp 'user')`"", "--extensions-dir=`"$(Join-Path $tmp 'ext')`"",
  "--extensionDevelopmentPath=`"$ext`"", "--extensionTestsPath=`"$(Join-Path $here 'integration_props')`"",
  '--new-window', '--skip-welcome', '--skip-release-notes', '--disable-workspace-trust')
$p = Start-Process -FilePath $code -ArgumentList $a -PassThru -NoNewWindow -RedirectStandardOutput (Join-Path $tmp 'stdout.txt') -RedirectStandardError (Join-Path $tmp 'stderr.txt')
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'timeout: killing the test instance'; $p.Kill() }
$env:HTD_IT_REPORT = $null; $env:HTD_PROPS_MODE = $null; $env:HTD_PROPS_DIR = $null
if (-not (Test-Path $report)) { Write-Host "no report written (logs: $tmp)"; exit 2 }
Get-Content $report -Raw -Encoding utf8
$changed = @($before.Keys | Where-Object { (Get-FileHash $_ -Algorithm MD5).Hash -ne $before[$_] })
if ($changed.Count) { Write-Host ('FAIL  page files changed on disk: ' + ($changed -join ', ')); exit 1 }
Write-Host ('PASS  every page file unchanged on disk (' + $before.Count + ' pages, MD5)')
Write-Host "(scratch: $tmp)"
exit 0
