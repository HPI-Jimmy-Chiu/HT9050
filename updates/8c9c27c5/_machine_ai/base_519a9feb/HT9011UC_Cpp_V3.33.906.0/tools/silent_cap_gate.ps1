# ---------------------------------------------------------------------------
#  silent_cap_gate.ps1 -- every display truncation in tools\census\ must be
#  registered, and must say what it dropped.
#
#  AI(W906-GL-1c) 20260827.
#
#  WHY THIS EXISTS
#  ---------------
#  Three waves in a row fixed the SAME defect shape in a different file:
#
#    GL-0z  expired_gate_scan.py  `sorted(full,...)[:30]`  -- summary said 288, printed 30.
#           MEASURED CONSEQUENCE: under the cap the brake-hazard patterns matched
#           Breaker 9 / MotorPower 0; uncapped, 9 / 3.  The truncation was concealing
#           3 of 12 motor-power rows -- the exact class that file's docstring warns about.
#    GL-1b  macro_seam_scan.py    `real[:3]`               -- hid that WriteIniData has 13
#           candidate real bodies and MySleep 12, i.e. hid the unambiguous-vs-ambiguous
#           split, which is the only thing deciding whether a row is workable at all.
#    GL-1c  body_size_scan.py     `rows[:30]`              -- summary said 114, printed 30.
#
#  The standing rule after three repeats is: stop fixing the instance, fix the mechanism.
#  So this gate enumerates EVERY display slice in the directory and requires each one to be
#  adjudicated.  A new slice nobody classified fails; a registry entry that no longer
#  matches the file also fails, so the registry cannot rot into a blanket excuse.  Same
#  shape as macro_order_gate.ps1's $Expected and soft_simulte_gate.ps1's $Adjudicated.
#
#  IT ALSO CAUGHT ITS OWN AUTHOR.  GL-0z removed expired_gate_scan.py's [:30] and, two
#  screens lower in the same wave, wrote `', '.join(hz[:3])` with no count -- so a gate
#  actuating five brake functions would name three silently.  A reviewer who had just
#  fixed this defect reintroduced it in the same file in the same session.  That is the
#  argument for a gate rather than more prose.
#
#  WHAT COUNTS AS OK
#  -----------------
#  A slice is fine when the reader can tell something was dropped:
#    COUNT-ADJACENT   the full count is printed on the same or the preceding line
#                     (expired_gate_scan's have[:4] follows "%d/%d callees live")
#    NOTICE           the loop prints "... N more NOT SHOWN"
#    DECLARED-SAMPLE  the section heading itself says it is a sample
#                     (part_coverage's "--- sample misses ---")
#    COSMETIC         a STRING clipped to a column width, not a list of items
#  Not fine: a list of rows or locations clipped with nothing telling the reader.
#
#  SCOPE, STATED RATHER THAN QUIETLY NARROWED
#  ------------------------------------------
#  tools\census\*.py only.  The `tools\*.ps1` scripts use `Select-Object -First N` for many
#  non-display purposes (picking a newest object, a winner, a best match -- live_lines.ps1
#  :148, stub_shadow_audit.ps1 :241, stub_vs_golden.ps1 :177), so the same regex there would
#  be mostly false positives and a gate that cries wolf gets ignored.  Those files' report
#  truncations are listed in $PsNote below so the omission is visible rather than implied.
#
#  USAGE
#    tools\silent_cap_gate.ps1           # exit 0 = every slice registered and adjudicated
#    tools\silent_cap_gate.ps1 -Emit     # print the registry rows found, to update $Expected
# ---------------------------------------------------------------------------
[CmdletBinding()]
param([switch] $Emit)

$ErrorActionPreference = "Stop"
$Tree = Split-Path $PSScriptRoot -Parent
$dir  = Join-Path $Tree "tools\census"

# key = "<file>|<slice text>", value = verdict + why.  Line numbers are DELIBERATELY not
# part of the key: they shift whenever prose is added above, and this campaign has already
# burned two waves on line-keyed registries rotting (GL-0z measured a recorded gate list at
# 0/9 correct).  Slice text plus filename is stable and still unique here -- and where a
# file legitimately holds the same slice text twice, the count is checked instead.
$Expected = @{
  "body_size_scan.py|rows[:30]"     = "NOTICE -- prints '... N more NOT SHOWN' (GL-1c)"
  "expired_gate_scan.py|have[:4]"   = "COUNT-ADJACENT -- preceding line prints '%d/%d callees live'"
  "expired_gate_scan.py|hz[:3]"     = "COUNT-ADJACENT -- prints 'actuates %d:' plus '(+N more)' (GL-1c)"
  "expired_gate_scan.py|comment[:200]" = "COSMETIC -- clips one gate comment string, not a list"
  # AI(W906-GL-1l) 20260828: this gate caught its own author again.  GL-1l added the claim TEXT
  # to the absence-claim inventory and clipped it to 150 characters for the column, which is a
  # display slice and was unregistered -- the runner failed until it was adjudicated here.
  # COSMETIC, not a hidden list: it clips ONE claim's prose, and every claim gets its own row,
  # so no row can go missing.  (Two entries in the file, the stamped and the bare listing.)
  "expired_gate_scan.py|what[:150]"    = "COSMETIC -- clips one claim's prose per row; no row is dropped (GL-1l)"
  "macro_seam_scan.py|real[:3]"     = "NOTICE -- prints '(+N more)' (GL-1b)"
  "macro_seam_scan.py|calls[:8]"    = "COUNT-ADJACENT -- same print starts with len(calls)"
  "census.py|partial[:60]"          = "COUNT-ADJACENT -- 'mirrored but INCOMPLETE: %d files' printed above (35 today, under the cap)"
  "part_coverage.py|miss[:6]"       = "DECLARED-SAMPLE -- per-name line prints '%d missing'; feeds the '--- sample misses ---' block"
  "part_coverage.py|allmiss[:40]"   = "DECLARED-SAMPLE -- heading says 'sample misses (READ THESE...)'"
  "part_coverage.py|name[:40]"      = "COSMETIC -- column width on a function name"
  "wave_targets.py|where[:3]"       = "NOTICE -- prints '(+N more)' (GL-1c)"
}
# Both failure directions were exercised 20260827 rather than assumed:
#   unregistered slice   drop zz_cap_probe.py with `rows[:2]`      -> FAIL, named.  Removed -> 0.
#   stale registry       add "zz_stale_control.py|nope[:1]"        -> FAIL as stale.  Removed -> 0.
#   mention-not-carry    a slice appearing ONLY inside a comment   -> still 11, not 12.
# Report truncations in tools\*.ps1, out of scope above.  Listed so the scope gap is
# visible; if one of these ever needs a notice it is a deliberate decision, not an oversight.
$PsNote = @(
  "cite_check.ps1:691   byFrom | Select-Object -First 30  (queue doc's per-source table)"
  "live_lines.ps1:327   deadList | Select-Object -First 60"
  "live_lines.ps1:341   runs | Select-Object -First 20"
  "pe_truncation_check.ps1:219  cl | Select-Object -First 5"
  "production_audit.ps1:124-126  chg/new/gone | Select-Object -First 5  (each prints its full count first)"
  "soft_simulte_gate.ps1:309    adjMiss | Select-Object -First 40"
)

if (-not (Test-Path -LiteralPath $dir)) {
  Write-Host "找不到 $dir" -ForegroundColor Red; exit 2
}

$found = @{}
$rows  = New-Object System.Collections.ArrayList
foreach ($f in (Get-ChildItem (Join-Path $dir "*.py") -File | Sort-Object Name)) {
  $L = [System.IO.File]::ReadAllLines($f.FullName)
  for ($i = 0; $i -lt $L.Count; $i++) {
    # Skip the slice mentioned INSIDE a comment.  Without this, a comment explaining a fix
    # ("`real[:3]` printed three bodies and said nothing") registers as a second live slice
    # -- the mention-vs-carry trap this tree has now hit five times.
    $code = ($L[$i] -split '#')[0]
    foreach ($m in [regex]::Matches($code, '(\w+)\s*\[\s*:\s*\d+\s*\]')) {
      $key = "$($f.Name)|$($m.Groups[0].Value)"
      if (-not $found.ContainsKey($key)) { $found[$key] = 0 }
      $found[$key]++
      [void]$rows.Add([PSCustomObject]@{ Key = $key; Line = $i + 1; Text = $code.Trim() })
    }
  }
}

if ($Emit) {
  Write-Host "=== 實際找到的切片（用來更新 `$Expected）===" -ForegroundColor Cyan
  foreach ($r in ($rows | Sort-Object Key, Line)) {
    Write-Host ("  {0,-46} :{1}" -f $r.Key, $r.Line)
  }
  Write-Host ""
  Write-Host "=== tools\*.ps1 的報表截斷（本 gate 範圍外，明列）===" -ForegroundColor DarkYellow
  foreach ($n in $PsNote) { Write-Host "  $n" }
  exit 0
}

$fails = New-Object System.Collections.ArrayList
foreach ($k in ($found.Keys | Sort-Object)) {
  if (-not $Expected.ContainsKey($k)) {
    [void]$fails.Add("未登記的顯示截斷: $k -- 判它是 NOTICE / COUNT-ADJACENT / DECLARED-SAMPLE / COSMETIC 之一並登記")
  }
}
foreach ($k in ($Expected.Keys | Sort-Object)) {
  if (-not $found.ContainsKey($k)) {
    [void]$fails.Add("登記表過期: $k 已不在檔案裡 -- 移除該筆或修正它")
  }
}

Write-Host ("tools\census\*.py 顯示切片: {0} 種（{1} 處），登記 {2} 種" -f `
            $found.Count, ($rows.Count), $Expected.Count)
if ($fails.Count -gt 0) {
  Write-Host "GATE FAIL" -ForegroundColor Red
  foreach ($x in $fails) { Write-Host "  - $x" -ForegroundColor Red }
  Write-Host ""
  Write-Host "  -Emit 可列出實際找到的每一處。" -ForegroundColor DarkYellow
  exit 1
}
Write-Host "GATE OK -- 每一處顯示截斷都已登記，且登記表無過期項。" -ForegroundColor Green
Write-Host ("  （範圍：tools\census\*.py。tools\*.ps1 的 {0} 處報表截斷明列在 `$PsNote，不在本 gate 內。）" -f $PsNote.Count)
exit 0
