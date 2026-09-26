# ===========================================================================
#  tools/nb2_assist/io_runtime_sampler.ps1  --  NB2 assist session 20260925
#
#  READ-ONLY field probe for the "cylinder button acts 2 s late" report
#  (docs/nb2_assist/RD5*_1203*_20260925_141228.md section 5-2).
#  It only sends HTTP GET /api/struct/io/runtime to wb_serve. It never writes
#  anything to the machine, never sends a command, needs no login.
#
#  What it answers:
#   * Is the 1203 poll loop running steadily? pollCount should rise ~5 per s
#     (kIoTickMs = 200). A stretch with no increase = the wb_serve main thread
#     was stuck (30 s SDO window, console QuickEdit, a modal alarm, ...).
#   * How long is one poll? pollMs (last poll only, ~15.6 ms steps).
#   * Do the stalls repeat every ~30 s? (= the drive-parameter SDO window)
#   * When did the watched points (e.g. the cylinder in-position sensors)
#     change state, in wall-clock ms? Compare with the time you clicked.
#   * HTTP round trip of the endpoint itself.
#   runtime.monitor.* exists only in builds that have commit 34243800
#   (20260925); without it the script still logs RTT and watched points.
#
#  Usage (PowerShell 5.1 on the machine, from any folder):
#    powershell -ExecutionPolicy Bypass -File io_runtime_sampler.ps1
#    ... -Seconds 180 -IntervalMs 100 -Watch "I0102,I0103" -Server http://127.0.0.1:8045
#  -Watch takes ioId values exactly as they appear in /api/struct/io/runtime.
#  Output: io_sampler_<yyyyMMdd_HHmmss>.csv and a summary printed at the end
#  (also saved as *_summary.txt) in the current folder or -OutDir.
#  Load: one ~150 KB JSON build per sample on wb_serve's socket thread. The web
#  IO page already does one every 500 ms; the default 200 ms is 2.5x that.
#  File is ASCII on purpose (PowerShell 5.1 + code page 950 breaks on
#  BOM-less non-ASCII scripts).
# ===========================================================================
param(
    [string]$Server = 'http://127.0.0.1:8045',
    [int]$Seconds = 120,
    [int]$IntervalMs = 200,
    [string]$Watch = '',
    [int]$StallMs = 450,
    [int]$TimeoutMs = 5000,
    [string]$OutDir = '.'
)

$ErrorActionPreference = 'Stop'
$url = $Server.TrimEnd('/') + '/api/struct/io/runtime'
$watchIds = @()
if ($Watch -ne '') { $watchIds = $Watch.Split(',') | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne '' } }
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$csv = Join-Path $OutDir ("io_sampler_" + $stamp + ".csv")
$sumFile = Join-Path $OutDir ("io_sampler_" + $stamp + "_summary.txt")

function Get-Body([string]$u, [int]$timeout) {
    $req = [System.Net.HttpWebRequest]::Create($u + '?_=' + [DateTime]::UtcNow.Ticks)
    $req.Method = 'GET'
    $req.Timeout = $timeout
    $req.ReadWriteTimeout = $timeout
    $req.KeepAlive = $false
    $req.Proxy = $null
    $resp = $req.GetResponse()
    try {
        $sr = New-Object System.IO.StreamReader($resp.GetResponseStream(), [System.Text.Encoding]::UTF8)
        return $sr.ReadToEnd()
    } finally { $resp.Close() }
}

function Get-Num([string]$body, [string]$key) {
    $m = [regex]::Match($body, '"' + $key + '"\s*:\s*(-?\d+)')
    if ($m.Success) { return [long]$m.Groups[1].Value } else { return $null }
}

function Get-Bool([string]$body, [string]$key) {
    $m = [regex]::Match($body, '"' + $key + '"\s*:\s*(true|false)')
    if ($m.Success) { return $m.Groups[1].Value } else { return '' }
}

function Get-Point([string]$body, [string]$id) {
    # "ioId":"<id>","isOn":true|false|null
    $m = [regex]::Match($body, '"ioId"\s*:\s*"' + [regex]::Escape($id) + '"\s*,\s*"isOn"\s*:\s*(true|false|null)')
    if ($m.Success) { return $m.Groups[1].Value } else { return 'missing' }
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$rows = New-Object System.Collections.ArrayList
$header = @('wall', 'tMs', 'rttMs', 'ok', 'pollCount', 'pollMs', 'pollErrors', 'monOpen', 'monDisabled', 'connected') + $watchIds
Set-Content -Path $csv -Value ($header -join ',') -Encoding ASCII
Write-Host ("Sampling " + $url + " every " + $IntervalMs + " ms for " + $Seconds + " s -> " + $csv)

$next = 0
while ($sw.ElapsedMilliseconds -lt ($Seconds * 1000)) {
    $t0 = $sw.ElapsedMilliseconds
    $wall = (Get-Date).ToString('HH:mm:ss.fff')
    $ok = 1; $body = ''
    try { $body = Get-Body $url $TimeoutMs } catch { $ok = 0 }
    $rtt = $sw.ElapsedMilliseconds - $t0
    $r = [ordered]@{
        wall = $wall; tMs = $t0; rttMs = $rtt; ok = $ok
        pollCount = $null; pollMs = $null; pollErrors = $null
        monOpen = ''; monDisabled = ''; connected = ''
    }
    if ($ok -eq 1) {
        $mi = $body.IndexOf('"monitor"')
        if ($mi -ge 0) {
            $mon = $body.Substring($mi)
            $r.pollCount = Get-Num $mon 'pollCount'
            $r.pollMs = Get-Num $mon 'pollMs'
            $r.pollErrors = Get-Num $mon 'pollErrors'
            $r.monOpen = Get-Bool $mon 'open'
            $r.monDisabled = Get-Bool $mon 'disabled'
        }
        $r.connected = Get-Bool $body 'connected'
    }
    $vals = @($r.wall, $r.tMs, $r.rttMs, $r.ok, $r.pollCount, $r.pollMs, $r.pollErrors, $r.monOpen, $r.monDisabled, $r.connected)
    $pts = @{}
    foreach ($id in $watchIds) {
        $v = 'error'
        if ($ok -eq 1) { $v = Get-Point $body $id }
        $pts[$id] = $v
        $vals += $v
    }
    $r['points'] = $pts
    [void]$rows.Add($r)
    Add-Content -Path $csv -Value ($vals -join ',') -Encoding ASCII
    $next += $IntervalMs
    $sleep = $next - $sw.ElapsedMilliseconds
    if ($sleep -gt 0) { Start-Sleep -Milliseconds $sleep } else { $next = $sw.ElapsedMilliseconds }
}

# ------------------------------- summary -----------------------------------
$L = New-Object System.Collections.ArrayList
function Say([string]$s) { [void]$L.Add($s); Write-Host $s }
$good = @($rows | Where-Object { $_.ok -eq 1 })
Say ('samples: ' + $rows.Count + '  ok: ' + $good.Count + '  failed: ' + ($rows.Count - $good.Count))
if ($good.Count -gt 0) {
    $rt = @($good | ForEach-Object { [long]$_.rttMs } | Sort-Object)
    $p = { param($a, $q) $a[[Math]::Min($a.Count - 1, [int][Math]::Floor($q * ($a.Count - 1)))] }
    Say ('http rtt ms  p50=' + (& $p $rt 0.5) + '  p95=' + (& $p $rt 0.95) + '  max=' + $rt[-1])
}
$withMon = @($good | Where-Object { $_.pollCount -ne $null })
if ($withMon.Count -lt 2) {
    Say 'runtime.monitor not present (build without 34243800) or no data: poll-loop checks skipped.'
} else {
    $first = $withMon[0]; $last = $withMon[-1]
    $dt = ($last.tMs - $first.tMs) / 1000.0
    $dc = $last.pollCount - $first.pollCount
    if ($dt -gt 0) { Say ('pollCount rate: ' + [Math]::Round($dc / $dt, 2) + ' per s (expected ~5 with a 200 ms IO clock)') }
    $pm = @($withMon | ForEach-Object { [long]$_.pollMs } | Sort-Object)
    Say ('pollMs (last poll, sampled)  median=' + $pm[[int][Math]::Floor(($pm.Count - 1) / 2)] + '  max=' + $pm[-1])
    Say ('pollErrors: first=' + $first.pollErrors + ' last=' + $last.pollErrors + '   monitor open=' + $last.monOpen + ' disabled=' + $last.monDisabled)
    # stalls: stretches in which pollCount did not increase
    $stalls = New-Object System.Collections.ArrayList
    $lastChangeT = $first.tMs; $lastChangeWall = $first.wall; $prev = $first.pollCount
    foreach ($s in $withMon) {
        if ($s.pollCount -ne $prev) {
            $gap = $s.tMs - $lastChangeT
            if ($gap -ge $StallMs) { [void]$stalls.Add([pscustomobject]@{ from = $lastChangeWall; to = $s.wall; ms = $gap; tMs = $lastChangeT }) }
            $lastChangeT = $s.tMs; $lastChangeWall = $s.wall; $prev = $s.pollCount
        }
    }
    Say ('stalls (no poll progress for >= ' + $StallMs + ' ms): ' + $stalls.Count)
    foreach ($x in $stalls) { Say ('   ' + $x.from + ' -> ' + $x.to + '   ' + $x.ms + ' ms') }
    if ($stalls.Count -ge 3) {
        $iv = @(); for ($i = 1; $i -lt $stalls.Count; $i++) { $iv += [Math]::Round(($stalls[$i].tMs - $stalls[$i - 1].tMs) / 1000.0, 1) }
        Say ('   spacing between stalls (s): ' + ($iv -join ', ') + '   (~30 = drive-parameter SDO window)')
    }
    $spikes = @($withMon | Where-Object { $_.pollMs -ge 400 })
    Say ('samples with pollMs >= 400: ' + $spikes.Count + $(if ($spikes.Count -gt 0) { '   first at ' + $spikes[0].wall } else { '' }))
}
foreach ($id in $watchIds) {
    Say ('point ' + $id + ' changes:')
    $prevV = $null; $n = 0
    foreach ($s in $good) {
        $v = $s.points[$id]
        if ($prevV -ne $null -and $v -ne $prevV) { Say ('   ' + $s.wall + '  ' + $prevV + ' -> ' + $v); $n++ }
        $prevV = $v
    }
    if ($good.Count -eq 0) { Say '   none (no successful sample)' }
    elseif ($n -eq 0) { Say ('   none (last value ' + $prevV + ')') }
}
Set-Content -Path $sumFile -Value $L -Encoding ASCII
Write-Host ('summary -> ' + $sumFile)
