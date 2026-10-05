# ---------------------------------------------------------------------------
#  macro_order_gate.ps1 -- find files that test a MachineType.h macro WITHOUT
#  MachineType.h being reachable at that point.
#
#  AI(W906-GL-0h) 20260827.
#
#  WHY THIS EXISTS
#  ---------------
#  `#ifdef` is evaluated AT THE DIRECTIVE.  A file that tests SOFT_SIMULTE before
#  MachineType.h has been included silently takes the #else arm -- and nothing
#  complains, because taking the other arm is legal.
#
#  Measured 20260827 (GL-0g): BarCode/BarCode_Helpers.h did exactly that.  Its
#  `#ifdef SOFT_SIMULTE` was reached before MachineType.h: BarCode_Helpers.cpp:24
#  includes that header FIRST and only reaches MachineType.h at :26 via cmydef.h --
#  two lines too late.  So
#  uCCDUnloaderClip::InitialDatas() set bSimulationCommand=false on a tree whose
#  MachineType.h:48 says SIM is ON.  One instruction in a 16 MB image.
#
#  DO NOT USE `g++ -MM` FOR THIS.  It answers "is MachineType.h in this TU's dependency
#  set", which was YES for the defective file -- true, and the wrong question.  The
#  question is whether the macro is defined by the time the directive is reached, which
#  is about ORDER, not membership.  That wrong answer cost a wave; it is the reason this
#  script walks the include list up to the citing line instead of asking the compiler
#  for dependencies.
#
#  WHAT IT DOES
#  ------------
#  1. Collects every macro name MachineType.h defines -- including the COMMENTED-OUT
#     ones, because a file testing a currently-commented macro is a latent instance of
#     the same bug (uncomment the macro and the arm still will not be seen).
#  2. Finds every `#ifdef / #ifndef / #if / #elif` in the tree that names one of them.
#  3. For each hit, BFS the file's OWN quoted includes -- only those appearing ABOVE the
#     directive -- looking for MachineType.h.  Reachable => safe in every TU.  Not
#     reachable => AT-RISK: the arm depends on the includer's ordering.
#
#  WHAT "AT-RISK" DOES AND DOES NOT MEAN
#  -------------------------------------
#  At-risk is a necessary, not sufficient, condition for a live defect.  A site inside a
#  `#if 0` gate is never evaluated at all, so it is LATENT: it becomes a defect the day
#  the gate opens.  The authoritative test for a CURRENT defect is the object-file diff
#  (build the tree twice, once with -D<macro>, and compare every .obj) -- that is what
#  proved BarCode_Helpers and what proved these three are not current.
#
#  EXPECTED SET
#  ------------
#  The known at-risk sites are listed below with why each is latent.  The gate
#  fails on anything NOT in that list, so a new one cannot arrive unnoticed.  Adding a
#  row here without an object-diff to back it is how this turns into decoration.
#
#  USAGE
#  -----
#    tools\macro_order_gate.ps1              # gate: exit 1 on an unexpected at-risk site
#    tools\macro_order_gate.ps1 -All         # also list every safe hit
# ---------------------------------------------------------------------------
[CmdletBinding()]
param([switch] $All)

$ErrorActionPreference = "Stop"
$Tree = Split-Path $PSScriptRoot -Parent

# file:line:macro -> why it is latent rather than a live defect.
$Expected = @{
  'ainarm9045_2x8_32.cpp:4335:SOFT_SIMULTE' =
    'whole file body is inside ONE #if 0 (:91 "dead variant", #endif :7804); DWARF shows 11 lines with code in a 7,804-line file and the .obj is 2,484 B. #ifdef inside a skipped group is never evaluated. WHEN THAT GATE OPENS the file must first reach MachineType.h -- it includes only ainarm9045_2x8_32.h (which has ZERO includes) and vclcompat/vcl_compat.h, so today the SIM arms would silently vanish.'
  'ainarm9045_2x8_32.cpp:4363:SOFT_SIMULTE' =
    'second site in the same dead #if 0 region; same remedy.'
  'Automation/auto9045.h:209:DEBUG_DUTONOFF' =
    'DEBUG_DUTONOFF is COMMENTED OUT at MachineType.h:20, so it is undefined everywhere and the absent DoHomeAndStart() declaration is correct today. Latent: auto9045.h reaches only vclcompat/vcl_compat.h (:27), so uncommenting the macro would NOT make this declaration appear. Its 4 includers (auto9045.cpp, automation.cpp, tests/test_auto9045.cpp, tests/test_automation.cpp) would each fail differently depending on their own include order -- the worst shape.'
  #AI(W906-GATE-DRIFT) 20261003: the three rows above re-baselined (line drift -30 / -19 since import 07f918a); two false positives added:
  'FileRW/TTLCfg.cpp:281:SOFT_SIMULTE' =
    'FALSE POSITIVE: :46 includes FileRW/TTLCfg.gen.inc which includes MachineType.h at its :14 -- the BFS indexes only *.h so it cannot follow .inc. MachineType.h is reached before :281 in the only TU.'
  'TesterComm/UiHome.cpp:74:SOFT_SIMULTE' =
    'FALSE POSITIVE: the directive tests W906_NO_SOFT_SIMULTE, a CMake -D (CMakeLists.txt:58-62), not a MachineType.h macro; the hit is the word SOFT_SIMULTE in the trailing // comment.'
}

$mtPath = Join-Path $Tree "MachineType.h"
$macros = @{}
foreach ($l in [System.IO.File]::ReadAllLines($mtPath)) {
  # both `#define X` and `//#define X` -- see WHY above
  if ($l -match '^\s*(//\s*)?#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)') { $macros[$Matches[2]] = $true }
}
Write-Host ("MachineType.h 巨集名稱（含被註解掉的）: {0}" -f $macros.Count)

$files = @(Get-ChildItem $Tree -Recurse -Include *.h, *.cpp -File |
           Where-Object { $_.FullName -notmatch '\\build|third_party' })
Write-Host ("掃 {0} 個檔 ..." -f $files.Count)

# basename -> path, for the BFS
$idx = @{}
foreach ($f in $files) { if ($f.Extension -eq '.h' -and -not $idx.ContainsKey($f.Name)) { $idx[$f.Name] = $f.FullName } }

$incCache = @{}
function Get-Includes($path, $maxLine) {
  $key = "$path|$maxLine"
  if ($incCache.ContainsKey($key)) { return $incCache[$key] }
  $L = [System.IO.File]::ReadAllLines($path)
  $n = if ($maxLine -gt 0) { [Math]::Min($maxLine, $L.Count) } else { $L.Count }
  $r = New-Object System.Collections.ArrayList
  for ($i = 0; $i -lt $n; $i++) {
    if ($L[$i] -match '^\s*#\s*include\s+"([^"]+)"') { [void]$r.Add((Split-Path $Matches[1] -Leaf)) }
  }
  $incCache[$key] = $r
  return $r
}

function Test-ReachesMachineType($path, $beforeLine) {
  $seen = @{}; $q = New-Object System.Collections.Queue
  foreach ($h in (Get-Includes $path $beforeLine)) { if (-not $seen.ContainsKey($h)) { $seen[$h] = $true; $q.Enqueue($h) } }
  while ($q.Count -gt 0) {
    $c = $q.Dequeue()
    if ($c -eq 'MachineType.h') { return $true }
    if (-not $idx.ContainsKey($c)) { continue }
    foreach ($h in (Get-Includes $idx[$c] 0)) { if (-not $seen.ContainsKey($h)) { $seen[$h] = $true; $q.Enqueue($h) } }
  }
  return $false
}

$hits = New-Object System.Collections.ArrayList
foreach ($f in $files) {
  if ($f.Name -eq 'MachineType.h') { continue }
  $L = [System.IO.File]::ReadAllLines($f.FullName)
  for ($i = 0; $i -lt $L.Count; $i++) {
    if ($L[$i] -match '^\s*#\s*(ifdef|ifndef|elif|if)\b(.*)$') {
      foreach ($m in [regex]::Matches($Matches[2], '[A-Za-z_][A-Za-z0-9_]*')) {
        if ($macros.ContainsKey($m.Value)) {
          [void]$hits.Add([PSCustomObject]@{
            Rel = ($f.FullName.Substring($Tree.Length + 1) -replace '\\', '/')
            Line = $i + 1; Macro = $m.Value; Path = $f.FullName
          })
        }
      }
    }
  }
}
Write-Host ("命中 {0} 處，{1} 個檔" -f $hits.Count, (@($hits | Select-Object -ExpandProperty Rel -Unique).Count))

$risk = New-Object System.Collections.ArrayList
$safe = 0
foreach ($h in ($hits | Sort-Object Rel, Line)) {
  if (Test-ReachesMachineType $h.Path $h.Line) {
    $safe++
    if ($All) { Write-Host ("  安全      {0,-52} :{1,-6} {2}" -f $h.Rel, $h.Line, $h.Macro) }
  } else {
    [void]$risk.Add($h)
  }
}
Write-Host ("安全 {0}   有風險 {1}" -f $safe, $risk.Count)

$unexpected = 0
Write-Host ""
foreach ($h in $risk) {
  $key = "{0}:{1}:{2}" -f $h.Rel, $h.Line, $h.Macro
  if ($Expected.ContainsKey($key)) {
    Write-Host ("  已知（潛在）  {0}" -f $key) -ForegroundColor Yellow
    Write-Host ("                {0}" -f $Expected[$key]) -ForegroundColor DarkGray
  } else {
    $unexpected++
    Write-Host ("  ★ 新增        {0}" -f $key) -ForegroundColor Red
  }
}
# A stale expected row is also a failure: it means the file moved and the entry now
# guards nothing, which is how a real site slips back in under a familiar-looking list.
$stale = @($Expected.Keys | Where-Object { $k = $_; -not (@($risk | ForEach-Object { "{0}:{1}:{2}" -f $_.Rel, $_.Line, $_.Macro }) -contains $k) })
foreach ($s in $stale) { Write-Host ("  ⚠ 過期條目（已不再命中，行號可能漂移了）: {0}" -f $s) -ForegroundColor Red }

Write-Host ""
if ($unexpected -eq 0 -and $stale.Count -eq 0) {
  Write-Host ("通過：有風險的 {0} 處全部是已登記的潛在項，無新增、無過期。" -f $risk.Count) -ForegroundColor Green
  exit 0
} else {
  Write-Host ("失敗：新增 {0} 處、過期 {1} 條。新增的要先用物件檔比對判斷是不是現行缺陷（見檔頭）。" -f $unexpected, $stale.Count) -ForegroundColor Red
  exit 1
}
