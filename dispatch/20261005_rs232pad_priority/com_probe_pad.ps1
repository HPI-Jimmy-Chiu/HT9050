param([string[]]$Ports = @('COM1','COM2','COM3','COM4','COM6'), [string]$Frame = 't051120', [int]$Baud = 115200, [int]$ListenMs = 1500)
# Golden uPadInterface query probe: sends ONE golden query frame (+CR) per port, listens for the reply. Queries only.
$Ports = @($Ports | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
foreach ($p in $Ports) {
    $sp = New-Object System.IO.Ports.SerialPort $p, $Baud, 'None', 8, 'One'
    $sp.Handshake = 'None'; $sp.DtrEnable = $true; $sp.RtsEnable = $true
    try { $sp.Open() } catch { "{0}: open failed {1}" -f $p, $_.Exception.Message; continue }
    Start-Sleep -Milliseconds 200
    $sp.DiscardInBuffer()
    $bytes = [Text.Encoding]::ASCII.GetBytes($Frame + [char]13)
    $sp.Write($bytes, 0, $bytes.Length)
    $t0 = Get-Date; $got = New-Object System.Collections.Generic.List[byte]
    while (((Get-Date) - $t0).TotalMilliseconds -lt $ListenMs) {
        $n = $sp.BytesToRead
        if ($n -gt 0) { $b = New-Object byte[] $n; $r = $sp.Read($b, 0, $n); for ($i = 0; $i -lt $r; $i++) { $got.Add($b[$i]) } }
        Start-Sleep -Milliseconds 20
    }
    $sp.Close(); $sp.Dispose()
    if ($got.Count -eq 0) { "{0}: sent '{1}<CR>' -> no reply" -f $p, $Frame }
    else {
        $a = $got.ToArray()
        $asc = -join ($a | ForEach-Object { if ($_ -ge 32 -and $_ -lt 127) { [char]$_ } elseif ($_ -eq 13) { '<CR>' } elseif ($_ -eq 10) { '<LF>' } else { '.' } })
        "{0}: sent '{1}<CR>' -> {2} bytes |{3}|  hex {4}" -f $p, $Frame, $a.Length, $asc, (($a | ForEach-Object { $_.ToString('X2') }) -join ' ')
    }
}
