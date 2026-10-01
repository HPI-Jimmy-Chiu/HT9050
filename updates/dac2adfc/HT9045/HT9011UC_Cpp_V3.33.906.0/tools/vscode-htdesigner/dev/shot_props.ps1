# AI(W906-HTDESIGNER) 20261001: screenshots of the properties panel (media/props.js) in headless Edge with the panel
# test's real data (%TEMP%\htdesigner_panels\props_*.json, written by test\run_all.ps1): the properties, events and code
# views, in VS Code's dark colours. Look at them after a change of the panel -- a test passing is not the same as the
# panel reading well. -> <Out>\props_<view><Tag>.png
param([string]$Data = 'props_spbSave.json', [int]$Width = 380, [int]$Height = 2200, [string]$Tag = '', [string]$Out = (Join-Path $env:TEMP 'htd_shots'))
$media = (Resolve-Path (Join-Path $PSScriptRoot '..\media')).Path
$dump = Join-Path $env:TEMP 'htdesigner_panels'
New-Item -ItemType Directory -Force $Out | Out-Null
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
$utf8 = New-Object System.Text.UTF8Encoding($false)
$data = [IO.File]::ReadAllText((Join-Path $dump $Data), $utf8).Replace('</', '<\/')
$mediaUrl = ([Uri]("$media\")).AbsoluteUri
$vars = '--vscode-font-family:"Segoe UI","Microsoft JhengHei",sans-serif;--vscode-font-size:13px;--vscode-foreground:#cccccc;--vscode-descriptionForeground:#9d9d9d;' +
  '--vscode-sideBar-background:#252526;--vscode-editor-font-family:Consolas,monospace;--vscode-input-background:#3c3c3c;--vscode-input-foreground:#cccccc;' +
  '--vscode-input-border:#3c3c3c;--vscode-focusBorder:#007fd4;--vscode-panel-border:#3c3c3c;--vscode-button-background:#0e639c;--vscode-button-foreground:#ffffff;' +
  '--vscode-button-secondaryBackground:#3a3d41;--vscode-button-secondaryForeground:#cccccc;--vscode-list-hoverBackground:#2a2d2e;--vscode-textLink-foreground:#3794ff;'
foreach ($view in 'props', 'events', 'code') {
  $click = if ($view -ne 'props') { "setTimeout(function(){ var b=document.querySelector('.views button[data-view=\""$view\""]'); if(b) b.click(); }, 150);" } else { '' }
  $html = @"
<!DOCTYPE html><html><head><meta charset="UTF-8">
<style>:root{$vars} body{margin:0;width:${Width}px;overflow:hidden;background:#252526;color:#cccccc;font-family:var(--vscode-font-family);font-size:13px;}</style>
<link rel="stylesheet" href="${mediaUrl}props.css">
<script>window.acquireVsCodeApi = function () { return { postMessage: function () {}, getState: function () { return null; }, setState: function () {} }; };</script>
</head><body><div id="root"></div>
<script src="${mediaUrl}props.js"></script>
<script>var DATA = $data; window.postMessage({ type: 'show', data: DATA }, '*'); $click</script></body></html>
"@
  $file = Join-Path $dump ("shot_$view.html")
  [IO.File]::WriteAllText($file, $html, $utf8)
  $png = Join-Path $Out ("props_$view$Tag.png")
  $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', "--user-data-dir=`"$(Join-Path $dump 'edge-shot')`"", '--allow-file-access-from-files',
    '--virtual-time-budget=2000', '--hide-scrollbars', "--window-size=520,$Height", "--screenshot=`"$png`"", ([Uri]$file).AbsoluteUri)
  Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow | Out-Null
  "$png $((Get-Item $png).Length)"
}
