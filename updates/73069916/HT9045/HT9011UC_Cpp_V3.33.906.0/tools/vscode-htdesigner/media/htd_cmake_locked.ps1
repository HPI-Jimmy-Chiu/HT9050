# AI(W906-HTDESIGNER) 20261009 (second review, run buttons #3): Make / Build (rebuild) / Clean of one build folder --
# cmake run under the same per-folder lock as htd_build.ps1 and htd_f5_mode.ps1 (Local\htd_build_<sha1 of the full
# path, lower case>), so another VS Code window's build of the same folder is never joined by a second cmake
# (CLAUDE.md hard rule 5: truncated exes, false undefined references).
# -Dir = the build folder; -ArgsB64 = the cmake arguments, one per line, UTF-8, base64 (no quoting through the
# command line). ASCII only (Windows PowerShell 5.1 reads this file in the ANSI code page).
param(
  [Parameter(Mandatory = $true)][string]$Dir,
  [Parameter(Mandatory = $true)][string]$ArgsB64
)
$a = @(([Text.Encoding]::UTF8.GetString([Convert]::FromBase64String($ArgsB64))) -split "`n" | Where-Object { $_ -ne '' })
$cmake = (Get-Command cmake.exe -ErrorAction SilentlyContinue)
if (-not $cmake) { Write-Host 'FATAL: cmake.exe not found'; exit 2 }
$key = [IO.Path]::GetFullPath($Dir).TrimEnd('\', '/').ToLowerInvariant()
$h = [BitConverter]::ToString([Security.Cryptography.SHA1]::Create().ComputeHash([Text.Encoding]::UTF8.GetBytes($key))).Replace('-', '').Substring(0, 16)
$mx = New-Object System.Threading.Mutex($false, "Local\htd_build_$h")
$own = $false
try { $own = $mx.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $own = $true }
if (-not $own) { Write-Host "FATAL: another build is running in $Dir (another VS Code window?) -- nothing done"; exit 5 }
$rc = 1
try {
  & $cmake.Source @a
  $rc = $LASTEXITCODE
} finally {
  try { $mx.ReleaseMutex() } catch { }
}
exit $rc
