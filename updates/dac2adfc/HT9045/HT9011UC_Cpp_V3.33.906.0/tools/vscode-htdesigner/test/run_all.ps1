# AI(W906-HTDESIGNER) 20260929: all three test layers, in order.
#   1. lib\*.js against the real trees             (run_tests.ps1)
#   2. the in-page probe in headless Edge           (probe_test.ps1)
#   3. extension.js with a fake vscode, fed the probe's real selection (smoke_extension.js)
# Read-only for the source trees; scratch files go to %TEMP%.
#   4. (-WithVSCode) the real VS Code, in a separate throw-away instance (vscode_it.ps1)
param([switch]$WithVSCode)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$bad = 0
Write-Host '=== 1. lib ==='
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'run_tests.ps1')
if ($LASTEXITCODE -ne 0) { $bad++ }
Write-Host ''
Write-Host '=== 2. probe (headless Edge) ==='
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'probe_test.ps1')
if ($LASTEXITCODE -ne 0) { $bad++ }
# AI(W906-HTDESIGNER) 20261001: the component tree's tab sheets on HW.IoSetView.html (part of layer 2)
Write-Host '--- 2b. component tree: tab sheets (headless Edge) ---'
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'tree_test.ps1')
if ($LASTEXITCODE -ne 0) { $bad++ }
Write-Host ''
Write-Host '=== 3. extension (fake vscode) ==='
$res = Join-Path $env:TEMP 'htdesigner_probe\Setup.HotPlate.html.result.json'
$rep = Join-Path $env:TEMP ('htd_smoke_' + [guid]::NewGuid().ToString('N') + '.txt')
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$env:ELECTRON_RUN_AS_NODE = '1'
$p = Start-Process -FilePath $code -ArgumentList "`"$(Join-Path $here 'smoke_extension.js')`"", "`"$res`"", "`"$rep`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
if (Test-Path $rep) { Get-Content $rep -Encoding utf8; Remove-Item $rep -Force } else { Write-Host 'no report' }
if ($p.ExitCode -ne 0) { $bad++ }
Write-Host ''
Write-Host '=== 3b. panels rendered in headless Edge with the data of layer 3 ==='
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'panels_render.ps1')
if ($LASTEXITCODE -ne 0) { $bad++ }
$layers = 3
if ($WithVSCode) {
  $layers = 4
  Write-Host ''
  Write-Host '=== 4. real VS Code (a separate test window opens and closes by itself) ==='
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'vscode_it.ps1')
  if ($LASTEXITCODE -ne 0) { $bad++ }
}
Write-Host ''
if ($bad) { Write-Host "ALL: $bad of $layers layer(s) failed" } else { Write-Host "ALL: $layers/$layers layers pass" }
exit $bad
