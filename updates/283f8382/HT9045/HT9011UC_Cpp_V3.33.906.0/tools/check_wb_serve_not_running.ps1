# AI(W906-BOOTSPEED-2) 20260929: preLaunchTask step of the no-debugger F5 ("IOWEB(這台): wb_serve 出貨組態（不接除錯器，較快）").
# Without gdb the red STOP button does not end wb_serve, so pressing F5 again while it runs would fail late and unclearly
# (the build cannot replace the running exe, the new one cannot bind port 8055). This step fails early and says so.
# Read-only: it lists processes and never stops one.
# AI(W906-F5WAIT) 20261001: EastSun pressed F5 right after the HMI Exit (oplog 08:46:01 closeProgram executed) and got
#   "preLaunchTask ... 後存在錯誤": the old wb_serve was still closing (Q44 settle 5 s, brakes, save) when this step looked.
#   So a running wb_serve is now given up to $WaitSec to exit by itself before this step fails. Still read-only: it only
#   waits and looks, it never stops a process. A wb_serve that is NOT closing still fails here, just $WaitSec later.
$ErrorActionPreference = 'Stop'
$WaitSec = 30
$procs = @(Get-Process -Name 'wb_serve' -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) {
    Write-Output 'OK: no wb_serve.exe running'
    exit 0
}
Write-Output ("wb_serve.exe still running (PID {0}) -- waiting up to {1} s for it to finish closing ..." -f (($procs | ForEach-Object { $_.Id }) -join ','), $WaitSec)
Write-Output ("舊的 wb_serve 還在（可能正在關站），最多等 {0} 秒讓它自己關完 ..." -f $WaitSec)
$deadline = (Get-Date).AddSeconds($WaitSec)
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 500
    $procs = @(Get-Process -Name 'wb_serve' -ErrorAction SilentlyContinue)
    if ($procs.Count -eq 0) {
        Write-Output 'OK: the old wb_serve.exe has exited'
        Write-Output 'OK：舊的 wb_serve 已經關完'
        exit 0
    }
}
foreach ($p in $procs) {
    $path = ''
    try { $path = $p.Path } catch { $path = '' }
    Write-Output ("wb_serve.exe is still running after {3} s: PID {0}, started {1}, {2}" -f $p.Id, $p.StartTime, $path, $WaitSec)
}
Write-Output 'Close it first: the HMI Exit button, or Ctrl+C in its terminal tab (named after the exe). Then press F5 again.'
Write-Output '舊的 wb_serve 還在跑：請先關掉（HMI 的 Exit，或在它的終端機分頁按 Ctrl+C），再按 F5。'
exit 1
