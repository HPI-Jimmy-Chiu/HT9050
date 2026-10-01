# AI(W906-BOOTWAIT) 20260929: F5's first step (.vscode/tasks.json "IOWEB(這台): 開啟等待畫面") -- open ../web/boot_wait.html in the
#   DEFAULT HTTP BROWSER'S EXE. Measured on this machine: Start-Process <file>.html and explorer.exe <file>.html (the .html file
#   association) start NO browser at all, so the first version of the task opened nothing and EastSun waited minutes with no screen
#   ("你一樣是當 橘色這行 出現時 好幾分鐘都沒反應"); starting chrome.exe with the file URL works.
#   Resolution: HKCU ...\UrlAssociations\http\UserChoice ProgId -> HKCR\<ProgId>\shell\open\command -> the exe; else Chrome / Edge.
#   -DryRun prints what it would start and starts nothing.
#   AI(W906-F5-HMI) 20261001: -Query "<k=v&k=v>" is appended to the file URL as ?<query> (boot_wait.html reads port / target).
#   THE F5 (port 8045, background.html?mode=debug) passes it; the IOWEB entries pass none (8055 and their target, unchanged).
param([string]$Page = '', [switch]$DryRun, [string]$Query = '')
if (-not $Page) { $Page = Join-Path $PSScriptRoot '..\..\web\boot_wait.html' }
$full = (Resolve-Path -LiteralPath $Page -ErrorAction SilentlyContinue)
if (-not $full) { Write-Output "boot_wait.html not found: $Page"; exit 1 }
$url = 'file:///' + ($full.Path -replace '\\', '/')
if ($Query) { $url += '?' + $Query }
# AI(W906-HMI-SHELL) 20260930: the HMI program window first (tools\hmi_shell, ht9045_hmi.exe -- no browser frame, browser
#   shortcuts off, its own taskbar icon). Built here when missing or older than its source (a few seconds). W906_HMI_SHELL=0
#   (or off) skips it; a missing / failed build falls back to the browser below.
$useShell = -not ($env:W906_HMI_SHELL -eq '0' -or $env:W906_HMI_SHELL -eq 'off')
if ($useShell) {
    $shDir = Join-Path $PSScriptRoot '..\build_hmi_shell'
    $shExe = Join-Path $shDir 'ht9045_hmi.exe'
    $shSrc = Join-Path $PSScriptRoot 'hmi_shell'
    $newest = (Get-ChildItem -LiteralPath $shSrc -File -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1)
    $stale = (-not (Test-Path -LiteralPath $shExe)) -or ($newest -and ($newest.LastWriteTime -gt (Get-Item -LiteralPath $shExe).LastWriteTime))
    if ($stale -and -not $DryRun) {
        Write-Output "building the HMI window (tools\hmi_shell\build_hmi_shell.bat) ..."
        & cmd /c ('"' + (Join-Path $shSrc 'build_hmi_shell.bat') + '"')
    }
    if ((Test-Path -LiteralPath $shExe) -and (Test-Path -LiteralPath (Join-Path $shDir 'WebView2Loader.dll'))) {
        Write-Output "open $url with $shExe [HMI window]"
        # --debug: boot_wait.html switches to background.html?mode=debug (F5 is the development start), so DevTools stay on
        #   Ctrl+Shift+F12 even though this first URL (file://) does not say mode=debug
        if (-not $DryRun) { Start-Process -FilePath $shExe -ArgumentList ('"' + $url + '" --debug') }
        exit 0
    }
    Write-Output "HMI window not available -> browser"
}
$exe = $null; $how = ''
try {
    $prog = (Get-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice' -ErrorAction Stop).ProgId
    $cmd = (Get-ItemProperty -Path "Registry::HKEY_CLASSES_ROOT\$prog\shell\open\command" -ErrorAction Stop).'(default)'
    if ($cmd -match '^\s*"([^"]+?\.exe)"') { $exe = $matches[1] } elseif ($cmd -match '^\s*(\S+?\.exe)') { $exe = $matches[1] }
    $how = "default http browser ($prog)"
} catch {}
if (-not $exe -or -not (Test-Path -LiteralPath $exe)) {
    $exe = $null
    foreach ($c in @((Join-Path $env:ProgramFiles 'Google\Chrome\Application\chrome.exe'),
                     (Join-Path ${env:ProgramFiles(x86)} 'Microsoft\Edge\Application\msedge.exe'),
                     (Join-Path $env:ProgramFiles 'Microsoft\Edge\Application\msedge.exe'))) {
        if (Test-Path -LiteralPath $c) { $exe = $c; $how = 'fallback'; break }
    }
}
if (-not $exe) { Write-Output "no browser found; open by hand: $url"; exit 1 }
Write-Output "open $url with $exe [$how]"
if (-not $DryRun) { Start-Process -FilePath $exe -ArgumentList $url }
