# AI(W906-STOPBTN) 20260929: EastSun「目前不是debug模式 但是你也要有按鈕可以給我直接關閉程式停止」.
# Asks every running wb_serve.exe for a NORMAL close (named event Local\HT9045_wb_serve_quit_<pid>, checked by its main loop:
# tools/wb_serve.cpp W906_ExternalQuitDue -- the same close as the main screen's Exit: motors stopped, brakes held, data saved),
# waits for it to end, and only if it does not end in time (a message box waiting for an answer holds the main loop) ends it.
#   -FromStopButton : run as the no-debugger F5's postDebugTask. VS Code also ends that session by itself right after the start
#                     when the terminal has no shell integration, so a wb_serve younger than 20 s is left alone in that mode.
param([switch]$FromStopButton, [int]$WaitSeconds = 30)
# AI(W906-HMI-SHELL-2) 20260930: EastSun「紅色正方形 需要連同頁面 也關掉」-- after wb_serve is gone, the HMI program window
#   (tools/hmi_shell, ht9045_hmi.exe) goes too: wb_serve's normal close already asks it (W906_HmiShellQuit); this covers the
#   forced end below (no normal close) and a window left over. Same message first ("HT9045_HMI_SHELL_QUIT": closes without the
#   close question), then Stop-Process after 5 s. Not when a young wb_serve was left alone (the start-up session end).
function Close-HmiWindow {
    $w = @(Get-Process -Name 'ht9045_hmi' -ErrorAction SilentlyContinue)
    if ($w.Count -eq 0) { return }
    try {
        Add-Type -Namespace W906 -Name HmiWin -ErrorAction Stop -MemberDefinition '[DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern IntPtr FindWindowW(string c, IntPtr t); [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern uint RegisterWindowMessageW(string s); [DllImport("user32.dll")] public static extern bool PostMessageW(IntPtr h, uint m, IntPtr wp, IntPtr lp);'
    } catch {}
    try {
        $h = [W906.HmiWin]::FindWindowW('HT9045HmiShell', [IntPtr]::Zero)
        if ($h -ne [IntPtr]::Zero) { [void][W906.HmiWin]::PostMessageW($h, [W906.HmiWin]::RegisterWindowMessageW('HT9045_HMI_SHELL_QUIT'), [IntPtr]::Zero, [IntPtr]::Zero) }
    } catch {}
    foreach ($x in $w) {
        if (-not $x.WaitForExit(5000)) { try { Stop-Process -Id $x.Id -Force -ErrorAction Stop } catch {} }
    }
    Write-Output 'HMI window closed / HMI 畫面已關閉'
}
$procs = @(Get-Process -Name 'wb_serve' -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) { Write-Output 'no wb_serve.exe running / 沒有在跑的 wb_serve'; Close-HmiWindow; exit 0 }
$rc = 0
$leftYoung = $false
foreach ($p in $procs) {
    $age = ((Get-Date) - $p.StartTime).TotalSeconds
    if ($FromStopButton -and $age -lt 20) {
        $leftYoung = $true
        Write-Output ("PID {0} started {1:N0} s ago -- not stopped from the stop button (it may be the session ending at start-up). Use the task 'IOWEB(這台): 關閉程式（正常關站）'. / 剛啟動未滿 20 秒，停止鈕不關；要關請用工作「IOWEB(這台): 關閉程式（正常關站）」" -f $p.Id, $age)
        continue
    }
    $asked = $false
    try {
        $ev = [System.Threading.EventWaitHandle]::OpenExisting("Local\HT9045_wb_serve_quit_$($p.Id)")
        [void]$ev.Set(); $ev.Close(); $asked = $true
        Write-Output ("PID {0}: normal close requested (motors stop, brakes hold, data saved) / 已要求正常關站" -f $p.Id)
    } catch {
        Write-Output ("PID {0}: no stop signal (an older build?) / 這支程式沒有關站訊號（舊版？）" -f $p.Id)
    }
    if ($asked -and $p.WaitForExit($WaitSeconds * 1000)) { Write-Output ("PID {0}: closed / 已正常結束" -f $p.Id); continue }
    Write-Output ("PID {0}: did not end within {1} s (a message box may be waiting) -> ending it now / 未在 {1} 秒內結束（可能有對話框在等），強制結束" -f $p.Id, $WaitSeconds)
    try { Stop-Process -Id $p.Id -Force -ErrorAction Stop; Write-Output ("PID {0}: ended / 已強制結束" -f $p.Id) } catch { Write-Output ("PID {0}: could not end it: {1}" -f $p.Id, $_.Exception.Message); $rc = 1 }
}
if (-not $leftYoung) { Close-HmiWindow }
exit $rc