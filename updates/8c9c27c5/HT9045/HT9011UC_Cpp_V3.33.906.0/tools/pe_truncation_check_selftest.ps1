# ---------------------------------------------------------------------------
#  pe_truncation_check_selftest.ps1 -- selftest of tools\pe_truncation_check.ps1's open timeout (LOCKED).
#
#  AI(W906-PECHECK-TIMEOUT) 20261003 (NB2-1, laptop task (k)).  ctest PE_TruncationCheckTimeout.
#
#  Fixtures (written only under -Scratch): ok\a.exe and ok\b.exe = copies of %SystemRoot%\System32\where.exe
#  (a signed system PE: length >= its own header's need), bad\c.exe = its first half (TRUNCATED).
#  The hang is simulated by the check's test hook W906_PECHECK_TEST_SLOW_OPEN=b.exe: that one open waits
#  10 x -OpenTimeoutMs on a background thread, so the check must give up after -OpenTimeoutMs.
#    [1] ok\ with the hook, timeout 3000 ms: exit 0, a LOCKED line naming b.exe, no TRUNCATED, and the run
#        ends well before the hook's 30 s (limit 20 s: a loaded parallel gate starts PowerShell slowly) (the process does not wait for the hung open)
#    [2] the whole scratch with the hook: exit 1 (c.exe TRUNCATED keeps its exit code), b.exe still LOCKED
#    [3] ok\ without the hook (default 30 s): exit 0, no LOCKED
#    [4] AI(W906-PECHECK-2PASS) 20261003: a.exe AND b.exe hung: both LOCKED, and the second pass's printed
#        pending-wait-ms is < 1.5 x -OpenTimeoutMs (one shared wait, not one per file)
#  Exit 0 = all pass, 1 = a check failed, 2 = setup failed.  Nothing outside -Scratch is written.
# ---------------------------------------------------------------------------
[CmdletBinding()]
param(
  [string] $Scratch = ""
)
$ErrorActionPreference = "Stop"
$check = Join-Path $PSScriptRoot "pe_truncation_check.ps1"
if (-not $Scratch) { $Scratch = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) "Obj\V906\pecheck_selftest" }
$src = Join-Path $env:SystemRoot "System32\where.exe"
if (-not (Test-Path -LiteralPath $check)) { Write-Host "setup: $check not found"; exit 2 }
if (-not (Test-Path -LiteralPath $src))   { Write-Host "setup: $src not found"; exit 2 }

$ok  = Join-Path $Scratch "ok"
$bad = Join-Path $Scratch "bad"
if (Test-Path -LiteralPath $Scratch) { Remove-Item -LiteralPath $Scratch -Recurse -Force }
$null = New-Item -ItemType Directory -Path $ok, $bad -Force
Copy-Item -LiteralPath $src -Destination (Join-Path $ok "a.exe")
Copy-Item -LiteralPath $src -Destination (Join-Path $ok "b.exe")
$bytes = [System.IO.File]::ReadAllBytes($src)
[System.IO.File]::WriteAllBytes((Join-Path $bad "c.exe"), $bytes[0..([int]($bytes.Length / 2) - 1)])

$fail = 0
function Check($cond, $what) {
  if ($cond) { Write-Host "  PASS $what" } else { $script:fail++; Write-Host "  FAIL $what" }
}
function Run-Check($dir, $timeoutMs, $slow, $firstMs = 0) {
  if ($slow) { $env:W906_PECHECK_TEST_SLOW_OPEN = $slow } else { Remove-Item Env:\W906_PECHECK_TEST_SLOW_OPEN -ErrorAction SilentlyContinue }
  $sw = [System.Diagnostics.Stopwatch]::StartNew()
  $args2 = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $check, "-Dirs", $dir)
  if ($timeoutMs -gt 0) { $args2 += @("-OpenTimeoutMs", "$timeoutMs") }
  if ($firstMs -gt 0)   { $args2 += @("-FirstWaitMs", "$firstMs") }          # AI(W906-PECHECK-2PASS) 20261003
  $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'      # AI(W906-PECHECK-AVBLOCKED) 20261004: a child that dies writes stderr ([6]); under Stop, 2>&1 would end this selftest
  $out = (& powershell @args2 2>&1 | Out-String)
  $rc = $LASTEXITCODE
  $ErrorActionPreference = $eap
  $sw.Stop()
  Remove-Item Env:\W906_PECHECK_TEST_SLOW_OPEN -ErrorAction SilentlyContinue
  return @{ Out = $out; Rc = $rc; Ms = $sw.ElapsedMilliseconds }
}

Write-Host "[1] ok\ with b.exe hung, -OpenTimeoutMs 3000"
$r = Run-Check $ok 3000 "b.exe"
Check ($r.Rc -eq 0) "exit 0 (a lock is a warning, not a failure) -- got $($r.Rc)"
Check ($r.Out -match 'LOCKED\s+b\.exe') "a LOCKED line names b.exe"
Check ($r.Out -notmatch 'LOCKED\s+a\.exe') "a.exe is not LOCKED"
Check ($r.Out -notmatch 'TRUNCATED|NOT-PE|SHORT-HDR') "nothing else reported"
Check ($r.Ms -lt 20000) "finished in $($r.Ms) ms, well before the hook's 30000 ms (the hung open is not waited for)"

Write-Host "[2] whole scratch (ok\ + bad\) with b.exe hung"
$r = Run-Check $Scratch 3000 "b.exe"
Check ($r.Rc -eq 1) "exit 1 (TRUNCATED keeps its exit code) -- got $($r.Rc)"
Check ($r.Out -match 'TRUNCATED\s+c\.exe') "c.exe TRUNCATED"
Check ($r.Out -match 'LOCKED\s+b\.exe') "b.exe still LOCKED"

Write-Host "[3] ok\ without the hook, default timeout"
$r = Run-Check $ok 0 ""
Check ($r.Rc -eq 0) "exit 0 -- got $($r.Rc)"
Check ($r.Out -notmatch 'LOCKED') "no LOCKED line"

Write-Host "[4] ok\ with a.exe AND b.exe hung, -OpenTimeoutMs 4000 -FirstWaitMs 300: one shared wait, not one each (AI(W906-PECHECK-2PASS))"
$r = Run-Check $ok 4000 "a.exe;b.exe" 300
$m = [regex]::Match($r.Out, 'pending-wait-ms=(\d+) limit-ms=(\d+)')
Check ($r.Rc -eq 0) "exit 0 -- got $($r.Rc)"
Check (($r.Out -match 'LOCKED\s+a\.exe') -and ($r.Out -match 'LOCKED\s+b\.exe')) "both a.exe and b.exe LOCKED"
Check ($m.Success -and [int]$m.Groups[1].Value -lt 6000) ("the second pass waited {0} ms for both together (< 6000 = 1.5 x 4000; one wait each would be ~8000)" -f $(if ($m.Success) { $m.Groups[1].Value } else { '(no pending-wait-ms line)' }))

Write-Host "[5] the antivirus refuses b.exe (Win32 225 ERROR_VIRUS_INFECTED) -- AI(W906-PECHECK-AVBLOCKED), measured on NB2 1004 02:4x"
$env:W906_PECHECK_TEST_OPEN_ERROR = "b.exe=225"
$r = Run-Check $ok 0 ""
Check ($r.Rc -eq 0) "exit 0 (named warning, the check does not die) -- got $($r.Rc)"
Check ($r.Out -match 'AV-BLOCKED\s+b\.exe') "an AV-BLOCKED line names b.exe"
Check ($r.Out -notmatch 'AV-BLOCKED\s+a\.exe' -and $r.Out -notmatch 'TRUNCATED|NOT-PE|SHORT-HDR|IOException') "a.exe is checked normally, nothing else reported"
Write-Host "[6] a different open error (Win32 5 ACCESS_DENIED) still stops the check, as before"
$env:W906_PECHECK_TEST_OPEN_ERROR = "b.exe=5"
$r = Run-Check $ok 0 ""
Check ($r.Rc -ne 0 -and $r.Out -notmatch 'AV-BLOCKED') "exit non-zero, no AV-BLOCKED -- got $($r.Rc)"
Remove-Item Env:\W906_PECHECK_TEST_OPEN_ERROR -ErrorAction SilentlyContinue

Remove-Item -LiteralPath $Scratch -Recurse -Force -ErrorAction SilentlyContinue
Write-Host ("RESULT: {0} failed" -f $fail)
if ($fail -gt 0) { exit 1 }
exit 0
