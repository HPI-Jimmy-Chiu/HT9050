param([Parameter(Mandatory=$true)][string]$GateLog, [Parameter(Mandatory=$true)][string]$Out,
      [string[]]$Dirs = @('D:\AI_TempFile\st02-gb-p1-build\tests', 'D:\AI_TempFile\st02-gb-p1-build-ship\tests'), [int]$MaxHours = 6)
# Vanishing test exe hunt (20260929 WB_WsProto x2, 20260930 WB_Crypto): log every *.exe Deleted / Renamed in the gate build
# dirs until the gate log says "gate done". Keep this file ASCII-only (Windows PowerShell 5.1).
# - The time is $Event.TimeGenerated (when the file system event arrived), not the handler time; handlers lag behind.
# - The phase is the last "=== ... HH:mm:ss" line of the gate log at or before that time.
# - Relinks during a build also delete (ld removes the old output) - expected. Only for events in a ctest phase is the slow
#   process query run (Win32_Process started in the last 120 s, with ppid and command line), to name the deleter.
# - After the gate: an exe deleted in a ctest phase and still missing is the vanishing case.
# Run in the background next to the gate:  powershell -File exe_watch.ps1 -GateLog <gate log> -Out <watch log>
"=== watch start $(Get-Date -Format 'MM-dd HH:mm:ss.fff') gate log $GateLog" | Out-File $Out -Encoding ascii
$act = {
  $t = $Event.TimeGenerated
  $e = $Event.SourceEventArgs
  $md = $Event.MessageData
  $phase = '?'
  foreach ($l in (Select-String -Path $md.GateLog -Pattern '^=== .*?(\d\d:\d\d:\d\d)\s*$')) {
    $h = [datetime]::ParseExact($l.Matches[0].Groups[1].Value, 'HH:mm:ss', $null)
    $h = $t.Date.Add($h.TimeOfDay)
    if ($h -le $t) { $phase = $l.Line } else { break }
  }
  $procs = ''
  if ($phase -match 'ctest') {
    $dm = [Management.ManagementDateTimeConverter]::ToDmtfDateTime($t.AddSeconds(-120))
    $procs = (Get-CimInstance Win32_Process -Filter "CreationDate > '$dm'" -ErrorAction SilentlyContinue | Where-Object { $_.Name -ne 'conhost.exe' } | ForEach-Object { $cl = "$($_.CommandLine)"; if ($cl.Length -gt 160) { $cl = $cl.Substring(0, 160) }; "[$($_.Name)#$($_.ProcessId)@$($_.CreationDate.ToString('HH:mm:ss')) ppid $($_.ParentProcessId): $cl]" }) -join ' '
  }
  "$($t.ToString('MM-dd HH:mm:ss.fff')) $($e.ChangeType) $($e.FullPath) | phase: $phase | procs: $procs" | Out-File $md.Out -Append -Encoding ascii
}
$md = @{ GateLog = $GateLog; Out = $Out }
$i = 0
foreach ($d in $Dirs) {
  if (-not (Test-Path $d)) { continue }
  $w = New-Object System.IO.FileSystemWatcher $d, '*.exe'
  $w.IncludeSubdirectories = $false
  $w.NotifyFilter = [System.IO.NotifyFilters]'FileName'
  $w.InternalBufferSize = 65536
  Register-ObjectEvent -InputObject $w -EventName Deleted -SourceIdentifier "exeDel$i" -Action $act -MessageData $md | Out-Null
  Register-ObjectEvent -InputObject $w -EventName Renamed -SourceIdentifier "exeRen$i" -Action $act -MessageData $md | Out-Null
  $w.EnableRaisingEvents = $true
  $i++
}
$end = (Get-Date).AddHours($MaxHours)   # stop even if the gate was killed before writing "gate done"
while (-not (Select-String -Path $GateLog -Pattern '=== gate done' -Quiet) -and (Get-Date) -lt $end) { Start-Sleep -Seconds 2 }
Start-Sleep -Seconds 10
Get-EventSubscriber | Unregister-Event
$gone = Select-String -Path $Out -Pattern ' Deleted (\S+\.exe) \| phase: [^|]*ctest' | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique | Where-Object { -not (Test-Path $_) }
"=== deleted during a ctest phase and still missing: $(@($gone).Count) $($gone -join ' ')" | Out-File $Out -Append -Encoding ascii
"=== watch end $(Get-Date -Format 'MM-dd HH:mm:ss.fff')" | Out-File $Out -Append -Encoding ascii
