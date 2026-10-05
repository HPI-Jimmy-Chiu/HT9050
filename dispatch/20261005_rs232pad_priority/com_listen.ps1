param([int]$Seconds = 60, [int]$Baud = 115200, [string[]]$Ports = @('COM1','COM2','COM3','COM4','COM5','COM6'), [string]$Out = '')
# Listen-only RS-232 probe: opens every port, asserts DTR/RTS (golden comTrayStepMotor DtrEnable/RtsEnable), NEVER writes.
$open = @{}
foreach ($p in $Ports) {
    $sp = New-Object System.IO.Ports.SerialPort $p, $Baud, 'None', 8, 'One'
    $sp.Handshake = 'None'; $sp.DtrEnable = $true; $sp.RtsEnable = $true; $sp.ReadTimeout = 1
    try { $sp.Open(); $open[$p] = $sp } catch { "{0} open failed: {1}" -f $p, $_.Exception.Message }
}
$total = @{}; foreach ($p in $open.Keys) { $total[$p] = 0 }
$log = New-Object System.Collections.Generic.List[string]
$t0 = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt $Seconds) {
    foreach ($p in @($open.Keys)) {
        $sp = $open[$p]
        $n = 0; try { $n = $sp.BytesToRead } catch { $n = 0 }
        if ($n -gt 0) {
            $buf = New-Object byte[] $n
            $r = $sp.Read($buf, 0, $n)
            $total[$p] += $r
            $hex = ($buf[0..($r-1)] | ForEach-Object { $_.ToString('X2') }) -join ' '
            $asc = -join ($buf[0..($r-1)] | ForEach-Object { if ($_ -ge 32 -and $_ -lt 127) { [char]$_ } elseif ($_ -eq 13) { '<CR>' } elseif ($_ -eq 10) { '<LF>' } else { '.' } })
            $line = "{0:HH:mm:ss.fff} {1} {2,3}B  {3}  |{4}|" -f (Get-Date), $p, $r, $hex, $asc
            $log.Add($line); $line
        }
    }
    Start-Sleep -Milliseconds 20
}
foreach ($sp in $open.Values) { try { $sp.Close() } catch {} ; $sp.Dispose() }
"--- totals (bytes received, nothing sent):"
foreach ($p in $Ports) { if ($open.ContainsKey($p)) { "{0}: {1}" -f $p, $total[$p] } }
if ($Out) { [IO.File]::WriteAllLines($Out, $log) }
