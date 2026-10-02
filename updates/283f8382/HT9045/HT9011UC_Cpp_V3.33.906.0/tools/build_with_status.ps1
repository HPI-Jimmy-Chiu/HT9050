# AI(W906-F5-PROGRESS) 20261002: EastSun「又出現 我按F5 編譯沒反應 我把UI 關掉 才編譯的情況」.
#   Measured that morning (hmi_shell.log + the build dir): F5 08:16:56 opened the wait page AND started the build in the same second;
#   the build ran 6 minutes (390 objects: cmydef.h had changed) while the wait page said only "還沒回應", covering VS Code -- so it
#   looked dead and the window was closed at 08:17:40.  This wrapper runs the same `cmake --build`, passes every line through
#   unchanged (VS Code's terminal and problem matcher see what they saw before), and every 0.5 s writes the progress to
#   ..\..\web\JSON\runtime\boot_build.js, which boot_wait.html reads (a <script> reload: file:// pages cannot fetch).
#   Exit code = cmake's.  It never stops the build itself.
param([Parameter(Mandatory = $true)][string]$BuildDir, [string]$Target = 'wb_serve', [int]$Jobs = 6, [string]$StatusFile = '')
if (-not $StatusFile) { $StatusFile = Join-Path $PSScriptRoot '..\..\web\JSON\runtime\boot_build.js' }
$dir = Split-Path -Parent $StatusFile
if ($dir -and -not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
$utf8 = New-Object System.Text.UTF8Encoding($false)
$t0 = Get-Date
$st = @{ state = 'building'; pct = 0; file = ''; n = 0; errors = 0; lastError = ''; target = $Target; dir = (Split-Path -Leaf $BuildDir) }
$lastWrite = [datetime]::MinValue
function Esc([string]$s) { return ($s -replace '\\', '\\' -replace '"', '\"' -replace "[`r`n]", ' ') }
function WriteStatus([bool]$force) {
    $now = Get-Date
    if (-not $force -and ($now - $script:lastWrite).TotalMilliseconds -lt 500) { return }
    $script:lastWrite = $now
    $ms = [long](($now.ToUniversalTime() - [datetime]'1970-01-01').TotalMilliseconds)
    $el = [int]($now - $t0).TotalSeconds
    $js = 'window.__HT_BUILD={"state":"' + $st.state + '","pct":' + $st.pct + ',"file":"' + (Esc $st.file) + '","n":' + $st.n +
          ',"errors":' + $st.errors + ',"lastError":"' + (Esc $st.lastError) + '","target":"' + (Esc $st.target) + '","dir":"' + (Esc $st.dir) +
          '","elapsed":' + $el + ',"t":' + $ms + '};'
    try { [IO.File]::WriteAllText($StatusFile, $js, $utf8) } catch { }
}
WriteStatus $true
& cmake --build $BuildDir --target $Target -j $Jobs 2>&1 | ForEach-Object {
    $line = "$_"
    Write-Output $line
    if ($line -match '^\[\s*(\d+)%\]\s+(Building|Linking)\s+\S+\s+(?:object\s+)?(.*)$') {
        $st.pct = [int]$Matches[1]
        $st.file = Split-Path -Leaf (($Matches[3] -replace '\.obj$', '') -replace '/', '\')
        if ($Matches[2] -eq 'Building') { $st.n++ } else { $st.file = 'link ' + $st.file }
    } elseif ($line -match ':\d+:\d+:\s+(fatal error|error):\s+(.*)$') {
        $st.errors++; $st.lastError = $line.Substring(0, [Math]::Min(200, $line.Length))
    }
    WriteStatus $false
}
$rc = $LASTEXITCODE
$st.state = $(if ($rc -eq 0) { 'done' } else { 'failed' })
if ($rc -eq 0) { $st.pct = 100 }
WriteStatus $true
exit $rc
