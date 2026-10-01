# AI(W906-HTDESIGNER) 20261001: syntax check of every .js of the extension + package.json (dev\syn.js), with VS Code's
# Electron as Node (no Node.js needed). Prints the bad ones and "syntax: N checked, M bad"; exit 1 when any is bad.
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$out = Join-Path $env:TEMP 'htd_syn_out.txt'
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$env:ELECTRON_RUN_AS_NODE = '1'
$p = Start-Process -FilePath $code -ArgumentList "`"$PSScriptRoot\syn.js`"", "`"$root`"", "`"$out`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
$all = @(Get-Content $out)
$bad = @($all | Where-Object { $_ -notmatch '^ok ' })
$bad
"syntax: $($all.Count) checked, $($bad.Count) bad"
if ($bad.Count) { exit 1 }
