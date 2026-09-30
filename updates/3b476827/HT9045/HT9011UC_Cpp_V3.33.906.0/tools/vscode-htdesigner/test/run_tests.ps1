# AI(W906-HTDESIGNER) 20260929: run test\run_tests.js with VS Code's Electron as Node
# (this machine has no node.exe). Read-only; prints the report and exits with its code.
$ErrorActionPreference = 'Stop'
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
if (-not (Test-Path $code)) { Write-Host "Code.exe not found: $code"; exit 2 }
$js = Join-Path $PSScriptRoot 'run_tests.js'
$report = Join-Path $env:TEMP ('htdesigner_tests_' + [guid]::NewGuid().ToString('N') + '.txt')
$env:ELECTRON_RUN_AS_NODE = '1'
$p = Start-Process -FilePath $code -ArgumentList "`"$js`"", "`"$report`"" -Wait -PassThru -NoNewWindow
if (Test-Path $report) { Get-Content $report -Encoding utf8; Remove-Item $report -Force }
else { Write-Host 'no report written' }
exit $p.ExitCode
