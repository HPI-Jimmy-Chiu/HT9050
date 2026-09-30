# AI(W906-BOOTSPEED-2) 20260929: preLaunchTask step of the no-debugger F5 ("IOWEB(這台): wb_serve 出貨組態（不接除錯器，較快）").
# Without gdb the red STOP button does not end wb_serve, so pressing F5 again while it runs would fail late and unclearly
# (the build cannot replace the running exe, the new one cannot bind port 8055). This step fails early and says so.
# Read-only: it lists processes and never stops one.
$ErrorActionPreference = 'Stop'
$procs = @(Get-Process -Name 'wb_serve' -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) {
    Write-Output 'OK: no wb_serve.exe running'
    exit 0
}
foreach ($p in $procs) {
    $path = ''
    try { $path = $p.Path } catch { $path = '' }
    Write-Output ("wb_serve.exe is still running: PID {0}, started {1}, {2}" -f $p.Id, $p.StartTime, $path)
}
Write-Output 'Close it first: the HMI Exit button, or Ctrl+C in its terminal tab (named after the exe). Then press F5 again.'
Write-Output '舊的 wb_serve 還在跑：請先關掉（HMI 的 Exit，或在它的終端機分頁按 Ctrl+C），再按 F5。'
exit 1
