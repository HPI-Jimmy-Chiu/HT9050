# ---------------------------------------------------------------------------
#  pe_truncation_check.ps1 -- find build artifacts that a killed build left
#  TRUNCATED, by asking the PE header how big the file should be.
#
#  AI(W906-GL-0g) 20260827.
#
#  WHY
#  ---
#  A `cmake --build` that is killed mid-link (tool timeout, Ctrl+C, exit 143)
#  leaves every output the linker had open truncated.  Measured 20260827: ONE kill
#  left 27 truncated .exe files in build_sim_nonoracle.
#
#  The trap is what happens next: a truncated file's mtime is NEWER than its inputs
#  (libht9045_sm.a and friends), so the next `cmake --build` decides nothing needs
#  doing and reports `exit 0` with 158 "Built target" lines, silently.  And a
#  truncated test executable RUNS and prints "RESULT: ALL PASS".
#
#  So `exit 0 / N targets` is not evidence that the artifacts are complete.
#
#  WHY NOT JUST COMPARE THE TWO LANES
#  ----------------------------------
#  Comparing exe sizes between build_nonoracle and build_sim_nonoracle does work
#  (it caught 27/27), but it needs a twin: the two lanes must be expected to produce
#  identical sizes.  That is true for those two and FALSE for build_dbg_nonoracle
#  (it carries -g).  This checks each file on its own terms instead.
#
#  HOW
#  ---
#  A PE file's section table says, for every section, where its raw bytes start
#  (PointerToRawData) and how many there are (SizeOfRawData).  The file must be at
#  least as long as the furthest of those.  A truncated file is shorter -- that is a
#  structural fact, not a heuristic, and it needs no reference copy.
#
#  Reported per file:
#     TRUNCATED  file is shorter than its own header says it must be
#     NOT-PE     no "PE\0\0" at e_lfanew -- measured 20260827: 26 exes in
#                build_dbg_nonoracle whose first 128 bytes are ALL ZERO (no "MZ",
#                e_lfanew=0).  70 MB files that look right by size and cannot be
#                loaded at all.  A kill can leave the header block unwritten.
#     SHORT-HDR  too small to even contain a PE header
#     LOCKED     the file could not be OPENED within -FirstWaitMs (2 s) and then the shared
#                -OpenTimeoutMs (30 s) of the second pass (AI(W906-PECHECK-2PASS), below) --
#                a WARNING, not a failure: its structure was not checked, but a lock is
#                not truncation.  AI(W906-PECHECK-TIMEOUT) 20261003 (NB2-1, laptop task (k)):
#                measured 1003 on the laptop gate -- the antivirus held a fresh
#                tests\test_tcp_tester_real.exe (19:09 to 19:33+, CPU 0) and
#                tests\test_pause_forward.exe (20:24 to 20:31), File.Open blocked, build.bat
#                never reached ctest, and stopping this check made build.bat say FATAL.
#                The open now runs through an async delegate call and is waited for at
#                most -OpenTimeoutMs; a timed-out open is left behind (it ends when the
#                antivirus lets go, or with the process).  Windows PowerShell 5.1 only
#                (build.bat starts `powershell`); where BeginInvoke is not supported the
#                open is synchronous as before.  Test hook: W906_PECHECK_TEST_SLOW_OPEN=<file
#                name> makes that one open wait 10 x the timeout (tools\pe_truncation_check_selftest.ps1).
#     AV-BLOCKED the open failed because the antivirus refused the file (virus / potentially
#                unwanted software, Win32 225 / 226) or the file vanished after the listing
#                (2 / 3, e.g. quarantined) -- a WARNING with its name, like LOCKED; its test will
#                not run. AI(W906-PECHECK-AVBLOCKED) 20261004 (see Get-OpenErrorVerdict). Test hook:
#                W906_PECHECK_TEST_OPEN_ERROR=<file name>=<win32 code>[;...].
#     ok         file length >= required
#
#  COVERAGE (measured, not assumed).  For a MinGW/GNU ld image the COFF string table
#  ends EXACTLY at EOF, so `required` computes to the exact file length for a complete
#  file -- e.g. test_barcode_helpers.exe: sections end at 11,589,120, symbol table at
#  13,950,126, string table adds 1,994,792 -> 15,944,918 = the file length, 0 bytes
#  unaccounted.  So any truncation at all is detectable, not just a large one.
#
#  Still not the only check worth having: a kill that damages CONTENT without changing
#  length is invisible here.  So the mtime clusters are printed too -- a cluster is how
#  both real incidents were recognised (27 exes at 05:38:30, 26 exes at 08-26 17:34).
#
#  USAGE
#  -----
#    tools\pe_truncation_check.ps1                       # every build* dir
#    tools\pe_truncation_check.ps1 -Dirs build_sim_nonoracle
# ---------------------------------------------------------------------------
[CmdletBinding()]
param(
  [string[]] $Dirs = @(),
  [int] $OpenTimeoutMs = 30000,                # AI(W906-PECHECK-TIMEOUT) 20261003: the shared wait for the opens still pending after the first pass (LOCKED above)
  [int] $FirstWaitMs = 2000                    # AI(W906-PECHECK-2PASS) 20261003: first-pass wait per file; a slower open is kept pending and the loop moves on
)

$ErrorActionPreference = "Stop"
$Tree = Split-Path $PSScriptRoot -Parent

# AI(W906-PECHECK-TIMEOUT) 20261003: File.Open(path, Open, Read, ReadWrite) -- the same call as before -- as a delegate, so it can
# run on a thread-pool thread (BeginInvoke) while this thread waits at most $OpenTimeoutMs.  No Add-Type (it compiles into %TEMP%).
$script:OpenFn = [Delegate]::CreateDelegate(
  [Func[string, System.IO.FileMode, System.IO.FileAccess, System.IO.FileShare, System.IO.FileStream]],
  [System.IO.File].GetMethod('Open', [type[]]@([string], [System.IO.FileMode], [System.IO.FileAccess], [System.IO.FileShare])))
$script:SleepFn = [Delegate]::CreateDelegate([Action[int]], [System.Threading.Thread].GetMethod('Sleep', [type[]]@([int])))
$script:AsyncOk = $true
#   AI(W906-PECHECK-2PASS) 20261003 (NB2-1, laptop 22:3x: gate b53a ship had 30 LOCKED x 30 s = 15 min of waiting, one after
#   the other): the first pass waits only -FirstWaitMs per file and keeps the still-running open; after the directory's loop
#   every pending open shares ONE deadline of -OpenTimeoutMs (they were all started already), so N locked files cost
#   N x FirstWaitMs + OpenTimeoutMs instead of N x OpenTimeoutMs.  An open finished in that window is checked normally.
function Open-PeFile($path) {        # a FileStream, or a pending @{ Ar; Hooked; Path } when the open did not finish within $FirstWaitMs
  $mode = [System.IO.FileMode]::Open; $acc = [System.IO.FileAccess]::Read; $share = [System.IO.FileShare]::ReadWrite
  if ($script:AsyncOk) {
    $hooked = $env:W906_PECHECK_TEST_SLOW_OPEN -and (($env:W906_PECHECK_TEST_SLOW_OPEN -split ';') -contains [IO.Path]::GetFileName($path))   # ';' list (2PASS)
    try {
      if ($hooked) { $ar = $script:SleepFn.BeginInvoke([Math]::Max(1, $OpenTimeoutMs) * 10, $null, $null) }   # test hook: an open that hangs (10 x: a loaded gate still ends long before it)
      else         { $ar = $script:OpenFn.BeginInvoke($path, $mode, $acc, $share, $null, $null) }
    } catch {
      $e = $_.Exception
      if (-not (($e -is [System.PlatformNotSupportedException]) -or ($e.InnerException -is [System.PlatformNotSupportedException]))) { throw }
      $script:AsyncOk = $false
      Write-Host "  ⓘ 這個 PowerShell 不支援非同步開檔（BeginInvoke），開檔沒有逾時保護（照舊）" -ForegroundColor Yellow
      return [System.IO.File]::Open($path, $mode, $acc, $share)
    }
    $p = @{ Ar = $ar; Hooked = $hooked; Path = $path }
    $fs = Complete-PeOpen $p ([Math]::Max(1, $FirstWaitMs))
    if ($null -eq $fs) { return $p }                                        # still running: pending (2PASS)
    return $fs
  }
  return [System.IO.File]::Open($path, $mode, $acc, $share)
}
function Complete-PeOpen($p, $waitMs) {   # the ONE wait the real open and the test hook share; a FileStream, or $null if not done in $waitMs
  if (-not $p.Ar.AsyncWaitHandle.WaitOne([int][Math]::Max(0, $waitMs))) { return $null }
  if ($p.Hooked) { $script:SleepFn.EndInvoke($p.Ar); return [System.IO.File]::Open($p.Path, 'Open', 'Read', 'ReadWrite') }   # (the hook's wait is 10 x: not reached)
  $fs = $script:OpenFn.EndInvoke($p.Ar)                                     # rethrows what File.Open threw (missing file ...), as before
  if ($env:W906_PECHECK_TEST_OPEN_ERROR) {                                  # AI(W906-PECHECK-AVBLOCKED) 20261004: test hook "<file name>=<win32 code>[;...]"
    foreach ($t in ($env:W906_PECHECK_TEST_OPEN_ERROR -split ';')) {
      $kv = $t -split '=', 2
      if ($kv.Count -eq 2 -and $kv[0] -eq [IO.Path]::GetFileName($p.Path)) {
        $fs.Close()
        throw (New-Object System.IO.IOException(("W906 test hook: open error " + $kv[1]), ([int](0x80070000 -bor [int]$kv[1]))))
      }
    }
  }
  return $fs
}
# AI(W906-PECHECK-AVBLOCKED) 20261004 (NB2-1 R207): measured on NB2 1004 02:4x -- F-Secure refused a freshly linked tests\test_cJSON.exe
#   ("the file contains a virus or potentially unwanted software", Win32 ERROR_VIRUS_INFECTED 225) and then removed it; the open threw,
#   the whole check died (build.bat: FATAL) and never said which file. An open the antivirus refuses (225 / 226 ERROR_VIRUS_DELETED) or a
#   file gone between the listing and the open (2 / 3, e.g. quarantined) is AV-BLOCKED: named, a warning like LOCKED (it is not a broken
#   link -- but its test will not run, so the line says so). Any other open error stops the check as before.
function Get-OpenErrorVerdict($err, $path) {
  $e = $err.Exception
  while ($e -and -not ($e -is [System.IO.IOException])) { $e = $e.InnerException }
  if (-not $e) { return $null }
  $code = $e.HResult -band 0xFFFF
  if (@(225, 226, 2, 3) -notcontains $code) { return $null }
  return @{ Verdict = 'AV-BLOCKED'; Need = 0; Len = 0; Why = ("Win32 " + $code + ": " + $e.Message.Trim()) }
}

if ($Dirs.Count -eq 0) {
  $Dirs = @(Get-ChildItem $Tree -Directory -Filter "build*" |
            Where-Object { $_.Name -notmatch 'STALE|DO_NOT_USE' } |
            Select-Object -ExpandProperty Name)
}

function Test-PeFile($path, $fs = $null) {                                 # AI(W906-PECHECK-2PASS) 20261003: + an already opened $fs (second pass)
  if ($null -eq $fs) { try { $fs = Open-PeFile $path } catch { $v = Get-OpenErrorVerdict $_ $path; if ($null -eq $v) { throw }; return $v } }   # AI(W906-PECHECK-AVBLOCKED) 20261004
  if ($fs -is [hashtable]) { return @{ Verdict = 'PENDING'; Pending = $fs; Need = 0; Len = (Get-Item -LiteralPath $path).Length } }
  if ($null -eq $fs) { return @{ Verdict = 'LOCKED'; Need = 0; Len = (Get-Item -LiteralPath $path).Length } }   # AI(W906-PECHECK-TIMEOUT) 20261003
  try {
    $len = $fs.Length
    if ($len -lt 0x40) { return @{ Verdict = 'SHORT-HDR'; Need = 0; Len = $len } }
    $br = New-Object System.IO.BinaryReader($fs)

    $fs.Position = 0x3C
    $lfanew = $br.ReadInt32()
    # PE sig (4) + COFF header (20) = 24 bytes before the optional header
    if ($lfanew -lt 0 -or ($lfanew + 24) -gt $len) {
      return @{ Verdict = 'SHORT-HDR'; Need = $lfanew + 24; Len = $len }
    }
    $fs.Position = $lfanew
    if ($br.ReadUInt32() -ne 0x00004550) {  # "PE\0\0"
      return @{ Verdict = 'NOT-PE'; Need = 0; Len = $len }
    }
    $null    = $br.ReadUInt16()              # Machine
    $nSec    = $br.ReadUInt16()              # NumberOfSections
    $null    = $br.ReadUInt32()              # TimeDateStamp
    $symPtr  = $br.ReadUInt32()              # PointerToSymbolTable
    $nSyms   = $br.ReadUInt32()              # NumberOfSymbols
    $optSize = $br.ReadUInt16()
    $null    = $br.ReadUInt16()              # Characteristics

    $secStart = $lfanew + 24 + $optSize
    if (($secStart + 40 * $nSec) -gt $len) {
      return @{ Verdict = 'SHORT-HDR'; Need = $secStart + 40 * $nSec; Len = $len }
    }

    # Furthest raw byte any section claims.  Sections with SizeOfRawData 0 (.bss)
    # carry no file bytes and must be skipped -- their PointerToRawData is garbage.
    $need = $secStart + 40 * $nSec
    for ($i = 0; $i -lt $nSec; $i++) {
      $fs.Position = $secStart + 40 * $i
      $null   = $br.ReadBytes(8)             # Name
      $null   = $br.ReadBytes(8)             # VirtualSize, VirtualAddress
      $rawSz  = $br.ReadUInt32()
      $rawPtr = $br.ReadUInt32()
      if ($rawSz -gt 0) {
        $end = [int64]$rawPtr + [int64]$rawSz
        if ($end -gt $need) { $need = $end }
      }
    }
    # AI(W906-GL-0g) 20260827: the section table is NOT the end of the file for a
    # MinGW/GNU ld image.  The COFF symbol table and the string table that follows it
    # sit AFTER the last section, and they are located by the COFF header, not by any
    # section entry.  Leaving them out is what made this tool's first version pass a
    # deliberately truncated 13,274,550-byte copy of a 15,944,918-byte exe -- the exact
    # failure it exists to catch.  Caught by the positive control, not by review.
    #
    # String table layout: immediately after the symbol table, first 4 bytes are its
    # own total size (including those 4 bytes).  When that prefix itself is past EOF the
    # file is already truncated, so require at least the symbol table's extent.
    if ($symPtr -gt 0 -and $nSyms -gt 0) {
      $symEnd = [int64]$symPtr + [int64]$nSyms * 18
      if ($symEnd -gt $need) { $need = $symEnd }
      if (($symEnd + 4) -le $len) {
        $fs.Position = $symEnd
        $strSz = $br.ReadUInt32()
        # A sane string table is at least the 4-byte prefix; cap the trust at the file
        # length so a garbage value cannot invent a false TRUNCATED.
        if ($strSz -ge 4 -and $strSz -lt 0x40000000) {
          $strEnd = $symEnd + [int64]$strSz
          if ($strEnd -gt $need) { $need = $strEnd }
        }
      }
    }

    $verdict = if ($len -lt $need) { 'TRUNCATED' } else { 'ok' }
    return @{ Verdict = $verdict; Need = $need; Len = $len }
  } finally { $fs.Close() }
}

$script:bad = 0
$tot = 0
$script:locked = New-Object System.Collections.ArrayList                   # AI(W906-PECHECK-TIMEOUT) 20261003: LOCKED files (warning)
$script:avBlocked = New-Object System.Collections.ArrayList                # AI(W906-PECHECK-AVBLOCKED) 20261004: AV-BLOCKED files (warning)
function Write-PeVerdict($f, $r) {                                          # AI(W906-PECHECK-2PASS) 20261003: one printer for both passes
  if ($r.Verdict -eq 'AV-BLOCKED') {                                       # AI(W906-PECHECK-AVBLOCKED) 20261004: named, a warning
    $null = $script:avBlocked.Add($f.FullName)
    Write-Host ("  {0,-11} {1,-46} 防毒拒絕或已移走這支（{2}）——結構沒檢查，它的測試不會跑  {3}" -f
                'AV-BLOCKED', $f.Name, $r.Why, $f.LastWriteTime.ToString("MM-dd HH:mm:ss")) -ForegroundColor Yellow
  } elseif ($r.Verdict -eq 'LOCKED') {                                           # AI(W906-PECHECK-TIMEOUT) 20261003: warning, not counted as broken
    $null = $script:locked.Add($f.FullName)
    Write-Host ("  {0,-11} {1,-46} 檔長 {2,12}  第一輪 {3} ms＋共用 {4} ms 內都打不開（防毒還鎖著？結構沒檢查）  {5}" -f
                'LOCKED', $f.Name, $r.Len, $FirstWaitMs, $OpenTimeoutMs, $f.LastWriteTime.ToString("MM-dd HH:mm:ss")) -ForegroundColor Yellow
  } elseif ($r.Verdict -ne 'ok') {
    $script:dBad++; $script:bad++
    Write-Host ("  {0,-11} {1,-46} 檔長 {2,12}  需要 {3,12}  {4}" -f
                $r.Verdict, $f.Name, $r.Len, $r.Need, $f.LastWriteTime.ToString("MM-dd HH:mm:ss")) -ForegroundColor Red
  }
}
foreach ($d in $Dirs) {
  $full = if ([IO.Path]::IsPathRooted($d)) { $d } else { Join-Path $Tree $d }   # AI(W906-OBJROOT) 20260925: build.bat 現在傳絕對路徑（<repo>\Obj\V906\<dir>）
  if (-not (Test-Path -LiteralPath $full)) { Write-Host "跳過（不存在）: $d"; continue }
  $files = @(Get-ChildItem $full -Recurse -File -Include *.exe, *.dll -ErrorAction SilentlyContinue)
  Write-Host ("=== {0}   {1} 個 exe/dll ===" -f $d, $files.Count)
  $script:dBad = 0
  $pending = New-Object System.Collections.ArrayList                       # AI(W906-PECHECK-2PASS) 20261003: opens not done within -FirstWaitMs
  foreach ($f in $files) {
    $tot++
    $r = Test-PeFile $f.FullName
    if ($r.Verdict -eq 'PENDING') { $null = $pending.Add(@{ F = $f; P = $r.Pending }); continue }
    Write-PeVerdict $f $r
  }
  if ($pending.Count -gt 0) {                                               # AI(W906-PECHECK-2PASS) 20261003: ONE shared deadline for all of them
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $nOk = 0
    foreach ($q in $pending) {
      try { $fs = Complete-PeOpen $q.P ($OpenTimeoutMs - $sw.ElapsedMilliseconds) } catch { $v = Get-OpenErrorVerdict $_ $q.F.FullName; if ($null -eq $v) { throw }; Write-PeVerdict $q.F $v; continue }   # AI(W906-PECHECK-AVBLOCKED) 20261004
      if ($null -eq $fs) { Write-PeVerdict $q.F @{ Verdict = 'LOCKED'; Need = 0; Len = $q.F.Length }; continue }
      $nOk++
      Write-PeVerdict $q.F (Test-PeFile $q.F.FullName $fs)
    }
    Write-Host ("  ⓘ 第一輪 {0} ms 內沒開成的 {1} 支一起再等：{2} 支開成並檢查、{3} 支 LOCKED（pending-wait-ms={4} limit-ms={5}）" -f
                $FirstWaitMs, $pending.Count, $nOk, ($pending.Count - $nOk), $sw.ElapsedMilliseconds, $OpenTimeoutMs)
  }
  if ($script:dBad -eq 0) { Write-Host "  全部結構完整" -ForegroundColor Green }

  # mtime clusters -- the complementary signal for a link killed cleanly at a
  # section boundary, which the structural check above cannot see.
  $cl = @($files | Group-Object { $_.LastWriteTime.ToString("MM-dd HH:mm:ss") } |
          Where-Object { $_.Count -ge 5 } | Sort-Object Count -Descending)
  if ($cl.Count -gt 0) {
    Write-Host "  ⓘ 同秒時間戳群（>=5 支，正常平行建置也會有，只是要知道往哪看）:"
    foreach ($c in ($cl | Select-Object -First 5)) {
      Write-Host ("      {0}  {1} 支" -f $c.Name, $c.Count)
    }
  }
}

Write-Host ""
if ($locked.Count -gt 0) {                                                  # AI(W906-PECHECK-TIMEOUT) 20261003
  Write-Host ("警告：{0} 支打不開（LOCKED：第一輪 {1} ms＋共用 {2} ms，多半是防毒剛好在掃），這幾支的結構沒有檢查——不算壞檔，" -f $locked.Count, $FirstWaitMs, $OpenTimeoutMs) -ForegroundColor Yellow
  Write-Host "  但如果剛剛有建置被中斷，等防毒放開後重跑這支工具再相信它們：" -ForegroundColor Yellow
  foreach ($p in $locked) { Write-Host ("    {0}" -f $p) -ForegroundColor Yellow }
}
if ($avBlocked.Count -gt 0) {                                               # AI(W906-PECHECK-AVBLOCKED) 20261004
  Write-Host ("警告：{0} 支被防毒拒絕或已被移走（AV-BLOCKED）——不是建置壞掉，但這幾支的測試不會跑；請 IT 看防毒紀錄（誤判要放行）再重建：" -f $avBlocked.Count) -ForegroundColor Yellow
  foreach ($p in $avBlocked) { Write-Host ("    {0}" -f $p) -ForegroundColor Yellow }
}
if ($bad -eq 0) {
  if ($locked.Count -gt 0 -or $avBlocked.Count -gt 0) {
    Write-Host ("通過：{0} 個 exe/dll 裡，檢查到的 {1} 支結構完整；{2} 支 LOCKED、{3} 支 AV-BLOCKED 沒檢查（見上面的警告）。" -f $tot, ($tot - $locked.Count - $avBlocked.Count), $locked.Count, $avBlocked.Count) -ForegroundColor Green
  } else {
    Write-Host ("通過：{0} 個 exe/dll，結構全部完整。" -f $tot) -ForegroundColor Green
  }
  exit 0
} else {
  # Deliberately NOT called "truncated" in the summary: NOT-PE files are not short,
  # they are 70 MB with an unwritten header.  Calling both "truncated" would send the
  # reader looking for a size problem that is not there.
  Write-Host ("失敗：{0} / {1} 個產物結構壞掉（TRUNCATED / NOT-PE / SHORT-HDR，逐行看上面）。" -f $bad, $tot) -ForegroundColor Red
  Write-Host "  處理方式：刪掉那些檔再重建 —— 不要只重建其中一支，也不要相信下一次" -ForegroundColor Red
  Write-Host "  cmake --build 的 exit 0（壞檔的 mtime 比輸入新，它會被沉默地放過）。" -ForegroundColor Red
  exit 1
}
