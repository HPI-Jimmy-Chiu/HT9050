# AI(W906-F5-CLOSE) 20261004: NB2-1 (i), machine dispatch 4 -- replay of "the operator closes the HMI window while F5 is still
#   building / starting" (machine hmi_shell.log 18:20:48 F5 opens boot_wait, 18:21:32 close) without the machine or VS Code:
#   the real F5 wait-page step (tools/open_boot_wait.ps1 -> ht9045_hmi.exe on boot_wait.html), a scripted X + YES on the shell
#   window, and the wb_serve start F5 does. PASS = the window does not come back and wb_serve ends normally.
#   Isolation: LOCALAPPDATA for the children -> a scratch folder (hmi_shell.log, WebView2 profile, the flag); wb_serve's
#   Gerneral.ini / IO_Table / Mot_Table / SetUp.inf / teach.ini / logs -> scratch copies via the same W906_* variables F5 sets.
#   A booting wb_serve (cases A0 / B) still writes system\lastdata*.dat / machinerecord.dat like every start: run
#   `python tools/realfile_guard.py snap <tag>` before and `check` / `restore` after (measured 1004 08:5x: those three only).
#   Run in an interactive session (WebView2 needs a desktop); no ht9045_hmi.exe / wb_serve.exe may run (single instance).
#   -Case A0 : no marker = the behaviour before W906-F5-CLOSE: the keeper reopens the window ~35 s after wb_serve starts
#              (the machine's 10-03 15:20:04 -> 15:20:39) -- the positive control.
#   -Case A  : the 18:21:32 case: the wait page is closed BEFORE wb_serve starts (the build is still running); with the marker
#              wb_serve must not start at all (exit 0 before any config is read) and no window may come back.
#   -Case B  : closed WHILE wb_serve boots: wb_serve must leave through the normal close, no window may come back.
param([ValidateSet('A0', 'A', 'B')] [string]$Case = 'A', [int]$Port = 8065, [string]$WbServe = '', [string]$Scratch = '',
      [int]$CloseAfterSec = 2, [int]$StartAfterSec = 3, [int]$WatchSec = 60)
$ErrorActionPreference = 'Stop'
$tree = Split-Path -Parent $PSScriptRoot                                   # HT9011UC_Cpp_V3.33.906.0
$web = Join-Path (Split-Path -Parent $tree) 'web'
if (-not $WbServe) { $WbServe = Join-Path (Split-Path -Parent $tree) 'Obj\V906\build\wb_serve.exe' }
if (-not $Scratch) { $Scratch = Join-Path $env:TEMP ('f5_close_replay_' + $Case + '_' + (Get-Date -Format 'HHmmss')) }
$fail = 0
function Say([string]$s) { Write-Output ("[{0:HH:mm:ss.fff}] {1}" -f (Get-Date), $s) }
function Check([bool]$ok, [string]$what) { if ($ok) { Say "PASS  $what" } else { Say "FAIL  $what"; $script:fail++ } }

if (Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue) { Say 'an ht9045_hmi.exe is running (single instance) -- close it first'; exit 2 }
if (Get-Process -Name wb_serve -ErrorAction SilentlyContinue) { Say 'a wb_serve.exe is running -- close it first'; exit 2 }
if (-not (Test-Path -LiteralPath $WbServe)) { Say "no wb_serve: $WbServe"; exit 2 }

Add-Type @'
using System; using System.Runtime.InteropServices;
public static class W {
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string c, IntPtr t);   // IntPtr: PowerShell turns $null into "" for a string
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
'@

# --- scratch: LOCALAPPDATA for the children, a runcfg copy for wb_serve
$lad = Join-Path $Scratch 'localappdata'; $rc = Join-Path $Scratch 'runcfg'
New-Item -ItemType Directory -Force -Path $lad, (Join-Path $rc 'system'), (Join-Path $rc 'logs'), (Join-Path $rc 'config') | Out-Null
foreach ($f in 'Gerneral.ini', 'IO_Table.csv', 'Mot_Table.csv', 'teach.ini') {
    $src = Join-Path 'D:\HT9045\system' $f
    if (Test-Path -LiteralPath $src) { Copy-Item -LiteralPath $src -Destination (Join-Path $rc 'system') }
}
# the active recipe comes from SetUp.inf (without it: "web API: no active recipe; refusing to serve" -- wb_serve never reaches
#   its main loop, so HMI-KEEP never runs; measured 1004 08:54)
if (Test-Path -LiteralPath 'D:\HT9045\SetUp.inf') { Copy-Item -LiteralPath 'D:\HT9045\SetUp.inf' -Destination (Join-Path $rc 'SetUp.inf') }
$hashBefore = @{}; Get-ChildItem -LiteralPath (Join-Path $rc 'system') -File | ForEach-Object { $hashBefore[$_.Name] = (Get-FileHash -LiteralPath $_.FullName).Hash }
# the HMI window is built BEFORE LOCALAPPDATA moves: build_hmi_shell.bat finds its g++ under %LOCALAPPDATA%\Programs, and
#   open_boot_wait.ps1 (which rebuilds a stale shell) would otherwise fall back to a browser
$shExe = Join-Path $tree 'build_hmi_shell\ht9045_hmi.exe'
$shNew = Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'hmi_shell') -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not (Test-Path -LiteralPath $shExe) -or ($shNew.LastWriteTime -gt (Get-Item -LiteralPath $shExe).LastWriteTime)) {
    & cmd /c ('"' + (Join-Path $PSScriptRoot 'hmi_shell\build_hmi_shell.bat') + '"') | ForEach-Object { Say "build_hmi_shell: $_" }
}
if (-not (Test-Path -LiteralPath $shExe)) { Say "FAIL  no HMI window exe ($shExe)"; exit 2 }
$env:LOCALAPPDATA = $lad
$shellLog = Join-Path $lad 'HT9045_HMI_Shell\hmi_shell.log'
$flag = Join-Path $lad ("HT9045_HMI_Shell\f5_close_$Port.flag")
$out = Join-Path $Scratch 'wb_serve.out.txt'
$p = $null
Say "case $Case  port $Port  scratch $Scratch"

function Start-Wb {
    $env:W906_GENERAL_INI_PATH = Join-Path $rc 'system\Gerneral.ini'; $env:W906_IOTABLE_PATH = Join-Path $rc 'system\IO_Table.csv'
    $env:W906_MOTTABLE_PATH = Join-Path $rc 'system\Mot_Table.csv'; $env:W906_TEACH_INI_PATH = Join-Path $rc 'system\teach.ini'
    $env:W906_SETUPINF_PATH = Join-Path $rc 'SetUp.inf'; $env:W906_AUTH_PATH = (Join-Path $rc 'config') + '\'
    foreach ($k in 'E84DATA', 'TCPDATA', 'SUMMARYLOT', 'EVENTLOG') { Set-Item -Path ("env:W906_{0}_ROOT" -f $k) -Value (Join-Path $rc ('logs\' + $k)) }
    $env:W906_BINCOUNT_PATH = Join-Path $rc 'logs\BinCount.ini'; $env:W906_PWBOOK_PATH = Join-Path $rc 'logs\pwbook.ini'
    $env:W906_IOTIMING_LOG = Join-Path $rc 'logs\io_click_timing.csv'; $env:W906_OPLOG_DIR = Join-Path $rc 'logs'
    $env:W906_HMI_URL = "http://127.0.0.1:$Port/background.html?mode=debug"
    $env:W906_HMI_SHELL = $shExe
    if ($Case -ne 'A0') { $env:W906_F5_OPERATOR_CLOSE = '1' } else { Remove-Item env:W906_F5_OPERATOR_CLOSE -ErrorAction SilentlyContinue }
    $q = Start-Process -FilePath $WbServe -ArgumentList @('--root', $web, '--port', "$Port") -WorkingDirectory $tree -RedirectStandardOutput $out -RedirectStandardError ($out + '.err') -PassThru -WindowStyle Hidden
    $null = $q.Handle                                                      # keep the handle: without it ExitCode reads empty after the exit
    Say "wb_serve pid $($q.Id) started" | Out-Host                       # Out-Host: a function returns ALL its pipeline output --
                                                                        #   with Say on the pipeline $p was [string, Process]
    return $q
}

function Read-Shared([string]$path) {                                   # wb_serve may still hold its stdout file open
    if (-not (Test-Path -LiteralPath $path)) { return '' }
    $fs = [System.IO.File]::Open($path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try { return (New-Object System.IO.StreamReader($fs)).ReadToEnd() } finally { $fs.Dispose() }
}

try {
# --- B: wb_serve is already booting when the wait page is opened and closed
if ($Case -eq 'B') { $p = Start-Wb }

# --- 1. the F5 wait-page step, as tasks.json runs it
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'open_boot_wait.ps1') -Query ("port=$Port") | ForEach-Object { Say "open_boot_wait: $_" }
$t0 = Get-Date
while (-not ((Test-Path -LiteralPath $shellLog) -and (Select-String -LiteralPath $shellLog -Pattern 'boot_wait.html' -Quiet))) {
    if (((Get-Date) - $t0).TotalSeconds -gt 30) { Say 'FAIL  the shell never navigated to boot_wait.html'; exit 1 }
    Start-Sleep -Milliseconds 200
}
Start-Sleep -Seconds $CloseAfterSec

# --- 2. X, then YES on the shell's one question (DlgProc: Enter = YES)
$h = [W]::FindWindow('HT9045HmiShell', [IntPtr]::Zero)
Check ($h -ne [IntPtr]::Zero) 'the shell window is there (class HT9045HmiShell)'
[void][W]::PostMessage($h, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)          # WM_CLOSE
$t0 = Get-Date; $d = [IntPtr]::Zero
while ($d -eq [IntPtr]::Zero -and ((Get-Date) - $t0).TotalSeconds -lt 10) { Start-Sleep -Milliseconds 100; $d = [W]::FindWindow('HT9045HmiDlg', [IntPtr]::Zero) }
Check ($d -ne [IntPtr]::Zero) 'the close question appeared (class HT9045HmiDlg)'
[void][W]::PostMessage($d, 0x0100, [IntPtr]0x0D, [IntPtr]::Zero)          # WM_KEYDOWN VK_RETURN = YES
$t0 = Get-Date
while ((Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue) -and ((Get-Date) - $t0).TotalSeconds -lt 10) { Start-Sleep -Milliseconds 200 }
Check (-not (Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue)) 'the shell exited after YES'
if (Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue) {          # else the window that "comes back" would be this one
    Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue | Stop-Process -Force
    if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force }
    Say 'the close did not happen -- nothing after this would mean anything; stopped'; exit 1
}
$ans = (Select-String -LiteralPath $shellLog -Pattern 'close: page answered' | Select-Object -Last 1)
Say ("shell: " + $(if ($ans) { $ans.Line } else { '(no answer line)' }))
if ($Case -ne 'A0') { Check (Test-Path -LiteralPath $flag) "the flag was written ($flag)" }

# --- 3. A0 / A: the build is over, F5 starts wb_serve now
if ($Case -ne 'B') { Start-Sleep -Seconds $StartAfterSec; $p = Start-Wb }


# --- 4. watch: does the window come back? does wb_serve end? (process state only; files are read after the cleanup)
$back = $false; $t0 = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt $WatchSec) {
    if (Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue) { $back = $true; Say ("window came back after {0:N1} s" -f ((Get-Date) - $t0).TotalSeconds); break }
    if ($p.HasExited -and $Case -ne 'A0') { Say ("wb_serve ended after {0:N1} s" -f ((Get-Date) - $t0).TotalSeconds); break }
    Start-Sleep -Milliseconds 250
}
if ($Case -ne 'A0' -and -not $back -and -not $p.HasExited) { [void]$p.WaitForExit(30000) }
$endedSelf = $p.HasExited
$exitCode = if ($endedSelf) { $p.ExitCode } else { $null }
} finally {
    # --- 5. clean up (always): a wb_serve still running is closed the STOPBTN way (named quit event), any shell window closed
    if ($p -and -not $p.HasExited) {
        & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'stop_wb_serve.ps1') | ForEach-Object { Say "stop_wb_serve: $_" }
        if (-not $p.WaitForExit(30000)) { Say 'wb_serve did not close in 30 s -> Stop-Process'; Stop-Process -Id $p.Id -Force }
    }
    Get-Process -Name ht9045_hmi -ErrorAction SilentlyContinue | Stop-Process -Force
}

# --- 6. verdicts (every process is gone now)
$o = Read-Shared $out
if ($Case -eq 'A0') { Check $back 'A0 (positive control, no marker): the keeper reopened the window' }
else {
    Check (-not $back) "${Case}: no window for the whole watch (or until wb_serve ended)"
    Check $endedSelf "${Case}: wb_serve ended by itself"
    if ($endedSelf) { Check ($exitCode -eq 0) "${Case}: wb_serve exit code 0 (was $exitCode)" }
    Check ($o -match '\[f5-close\]') "${Case}: wb_serve printed [f5-close]"
    Check (-not (Test-Path -LiteralPath $flag)) "${Case}: the flag was consumed"
    if ($Case -eq 'A') {
        $changed = @(); Get-ChildItem -LiteralPath (Join-Path $rc 'system') -File | ForEach-Object { if ($hashBefore[$_.Name] -ne (Get-FileHash -LiteralPath $_.FullName).Hash) { $changed += $_.Name } }
        Check ($changed.Count -eq 0) ("A: nothing loaded -- the scratch config files are unchanged" + $(if ($changed.Count) { ' (changed: ' + ($changed -join ', ') + ')' } else { '' }))
        Check ($o -notmatch 'loading machine config') 'A: wb_serve stopped before loading the machine config'
    } else {
        Check ($o -match 'normal close') 'B: wb_serve left through the normal close'
    }
}
Say ("result: {0} failure(s)" -f $fail)
exit ([int]($fail -ne 0))
