param([string]$Ports = 'COM1,COM2,COM3,COM4,COM5,COM6', [string]$Addrs = '32,33,34,35', [int]$Baud = 9600, [int]$ListenMs = 1500)
# Golden SetNoBackGround_TFT frame (BinDisplay/MyBinDisp.cpp): 3A addr 00 0D 00 04 00 01 30 30 30 30 30 00 00 07 00 ck 0D 0A
# ck = two's complement of the byte sum from addr through the 9 data bytes. A display command only (TFT background), nothing else.
function Frame([int]$addr) {
    $body = @($addr, 0x00, 0x0D, 0x00, 0x04, 0x00, 0x01, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x07, 0x00)
    $sum = 0; foreach ($b in $body) { $sum += $b }
    $ck = ((-bnot $sum) + 1) -band 0xFF
    return [byte[]](@(0x3A) + $body + @($ck, 0x0D, 0x0A))
}
foreach ($p in ($Ports -split ',')) {
    $sp = New-Object System.IO.Ports.SerialPort $p, $Baud, 'None', 8, 'One'
    $sp.Handshake = 'None'; $sp.DtrEnable = $true; $sp.RtsEnable = $true
    try { $sp.Open() } catch { "{0}: open failed {1}" -f $p, $_.Exception.Message; continue }
    Start-Sleep -Milliseconds 200; $sp.DiscardInBuffer()
    foreach ($a in ($Addrs -split ',')) {
        $f = Frame ([int]$a)
        $sp.Write($f, 0, $f.Length)
        $t0 = Get-Date; $got = New-Object System.Collections.Generic.List[byte]
        while (((Get-Date) - $t0).TotalMilliseconds -lt $ListenMs) {
            $n = $sp.BytesToRead
            if ($n -gt 0) { $b = New-Object byte[] $n; $r = $sp.Read($b, 0, $n); for ($i = 0; $i -lt $r; $i++) { $got.Add($b[$i]) } }
            Start-Sleep -Milliseconds 20
        }
        $hex = ($got.ToArray() | ForEach-Object { $_.ToString('X2') }) -join ' '
        if ($got.Count -eq 0) { "{0} addr 0x{1:X2}: no reply" -f $p, [int]$a } else { "{0} addr 0x{1:X2}: {2} bytes  {3}" -f $p, [int]$a, $got.Count, $hex }
    }
    $sp.Close(); $sp.Dispose()
}
