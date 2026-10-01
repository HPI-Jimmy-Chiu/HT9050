# AI(W906-HTDESIGNER) 20261001: the smoke layer alone (test\smoke_extension.js: extension.js with a fake vscode, fed the
# probe's real selection) -- quicker than test\run_all.ps1 while working. Needs one run_all (or probe_test) before it:
# it reads %TEMP%\htdesigner_probe\Setup.HotPlate.html.result.json. Prints the FAIL / RESULT lines (or -Pattern).
param([string]$Pattern = 'FAIL|RESULT')
$here = (Resolve-Path (Join-Path $PSScriptRoot '..\test')).Path
$res = Join-Path $env:TEMP 'htdesigner_probe\Setup.HotPlate.html.result.json'
$rep = Join-Path $env:TEMP 'htd_smoke_report.txt'
if (Test-Path $rep) { Clear-Content $rep }
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$env:ELECTRON_RUN_AS_NODE = '1'
$p = Start-Process -FilePath $code -ArgumentList "`"$here\smoke_extension.js`"", "`"$res`"", "`"$rep`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
if (Test-Path $rep) { Get-Content $rep -Encoding utf8 | Select-String -Pattern $Pattern | ForEach-Object { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) } } else { 'no report' }
"smoke exit $($p.ExitCode)"
