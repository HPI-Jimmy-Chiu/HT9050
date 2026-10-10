# AI(W906-HTDESIGNER) 20261008 (EastSun: keep HT9045 and HT9050 parameters in two folders; "fix it completely";
# ruling: dev PCs only). The machine data folders the program hard-codes become directory junctions that point at the
# data of the machine being run:
#     D:\HT9045\system   D:\HT9045\config   D:\HT9045\IniData   D:\GPIB9045\system   D:\HT9045_Log
#  -> <Root9050>\HT9045\system ...          (running HT9050)
#  -> <Root9045>\HT9045\system ...          (running HT9045)
# The first switch MOVES each real folder (same drive = a rename, nothing copied, nothing lost) into Root9045, then links.
# A junction is only ever removed as a link (never recursive), after checking it IS a junction.
# AI(W906-HTDESIGNER) 20261008 (full test, audit A1-A9):
#   * the roots are made absolute and checked (no relative path, not the same, not a live folder or inside one);
#   * the machine's own data must be there: a missing <root>\HT9045\system etc. is refused, not created empty (only the
#     log folder may be created);
#   * the running-program check only when something has to change (a link already right = nothing to do);
#   * every move is tried first (a rename to a probe name and back: an open file or another drive fails it there), so a
#     switch never stops halfway; and a step that fails puts the links it already changed back.
# Refuses (exit 3) while wb_serve / wb_publish / wb_gateway / the GPIB programs / the BCB6 HT9045.exe run.
# -LiveRoot (tests): where the five live paths are, instead of D:\ .  -Check: print the mapping, change nothing.
param(
  [ValidateSet('9050', '9045')][string]$Machine = '9045',
  [string]$Root9050 = 'D:\HP9050',
  [string]$Root9045 = 'D:\HP9045',
  [string]$LiveRoot = 'D:\',
  [switch]$Check,
  [switch]$NoProcessCheck
)
$ErrorActionPreference = 'Stop'
$rels = @('HT9045\system', 'HT9045\config', 'HT9045\IniData', 'GPIB9045\system', 'HT9045_Log')
$canCreate = @('HT9045_Log')   # (a log folder may start empty; parameters may not)

function Norm($p) { return [IO.Path]::GetFullPath(($p -replace '/', '\')).TrimEnd('\') }
function Under($a, $b) { return ($a -ieq $b) -or $a.StartsWith($b + '\', [StringComparison]::OrdinalIgnoreCase) }
function LinkTarget($p) {
  $i = Get-Item -LiteralPath $p -Force -ErrorAction SilentlyContinue
  if (-not $i) { return $null }
  if (-not ($i.Attributes -band [IO.FileAttributes]::ReparsePoint)) { if (-not $i.PSIsContainer) { return 'file' }; return '' }   # a real folder (review of 0.427 #2: a FILE is none)
  $t = $i.Target
  if ($t -is [array]) { $t = $t[0] }
  if (-not $t) { return '?' }
  return (Norm ([string]$t))
}
function Refuse($why, $code) { Write-Output "[machine] refused: $why"; exit $code }

foreach ($r in @($Root9050, $Root9045, $LiveRoot)) {
  if (-not [IO.Path]::IsPathRooted($r) -or -not ($r -match '^[A-Za-z]:[\\/]|^\\\\')) { Refuse "'$r' is not an absolute path" 4 }
  # (review of 0.425 switch #2: New-Item -Target takes [ ] as a wildcard -- the real folder was moved, the link could not be made)
  if ($r -match '[\[\]]') { Refuse "'$r' has [ or ] in it -- PowerShell cannot make a link to it; use a folder name without them" 4 }
}
$R50 = Norm $Root9050; $R45 = Norm $Root9045; $LR = Norm $LiveRoot
if ($R50 -ieq $R45) { Refuse "the HT9050 and HT9045 folders are the same ($R50)" 4 }
$target = if ($Machine -eq '9050') { $R50 } else { $R45 }
foreach ($r in $rels) {
  $live = Norm (Join-Path $LR $r)
  foreach ($root in @($R50, $R45)) { $w = Norm (Join-Path $root $r); if ((Under $w $live) -or (Under $live $w)) { Refuse "$w and $live overlap" 4 } }
}

if ($Check) {
  foreach ($r in $rels) {
    $live = Join-Path $LR $r
    $t = LinkTarget $live
    $state = if ($t -eq $null) { 'missing' } elseif ($t -eq '') { 'real folder' } else { '-> ' + $t }
    Write-Output ("[machine] {0}  {1}" -f $live, $state)
  }
  exit 0
}

# (1009 second review (switch #1): one switch at a time on this PC -- two runs relinked the same five folders in turn and
#  left them half switched while one said it was done)
$swMx = New-Object System.Threading.Mutex($false, 'Local\htd_machine_switch')
$swOwn = $false
try { $swOwn = $swMx.WaitOne(30000) } catch [System.Threading.AbandonedMutexException] { $swOwn = $true }
if (-not $swOwn) { Refuse 'another machine switch is running (another VS Code window?)' 8 }

# the plan: nothing is changed until all five are known to be switchable
$plan = @()
foreach ($r in $rels) {
  $live = Join-Path $LR $r
  # (1009 review #8: a switch ended between its two probe renames (killed, Ctrl+C) left the real folder as <live>.htdprobe --
  #  the next run took the folder for missing and linked past it, or said "put HT9045's data there first")
  if (Test-Path -LiteralPath ($live + '.htdprobe')) {
    Refuse ("$live.htdprobe is there -- a switch was stopped half way; that IS the real folder. Rename it back to $live by hand (if $live is missing) and run again") 4
  }
  $want = Norm (Join-Path $target $r)
  $t = LinkTarget $live
  # (review of 0.427 #2: a file where a folder belongs -- moved as "the real folder", the machine never switched back)
  if ($t -eq 'file') { Refuse "$live is a file, not a folder -- move it away by hand first (nothing was changed)" 4 }
  if ((Test-Path -LiteralPath $want) -and -not (Test-Path -LiteralPath $want -PathType Container)) { Refuse "$want is a file, not a folder (nothing was changed)" 4 }
  $wantThere = Test-Path -LiteralPath $want
  # (review of 0.425 switch #1: HT9045 has none of it (a PC without GPIB) and the link points into HT9050's -- the link is taken
  #  away, as it was before the switch to 9050; refused, it could never be switched back and HT9045 ran on HT9050's data)
  # (review of 0.426 #1: ONLY the GPIB folder (the one a PC may never have had), and only when HT9045's own data IS there --
  #  a mistyped Root9045 took all five links away and said "HT9045 data: …" with exit 0)
  # (review of 0.427 #3: or comes by THIS run -- the live HT9045\system is still the real folder, moved there now)
  $gpibOnly = ($r -eq 'GPIB9045\system') -and ((Test-Path -LiteralPath (Join-Path $R45 'HT9045\system')) -or ((LinkTarget (Join-Path $LR 'HT9045\system')) -eq ''))
  if ($gpibOnly -and -not $wantThere -and $Machine -eq '9045' -and $t -and $t -ne '?' -and ($t -ieq $R50 -or $t.StartsWith($R50 + '\', [StringComparison]::OrdinalIgnoreCase))) {
    $plan += @{ live = $live; want = $want; act = 'unlink'; old = $t }
    continue
  }
  # (review of 0.426 #2: absent on both sides (a PC without GPIB, on 9045 already) -- nothing to do, not "put the data there")
  if ($gpibOnly -and -not $wantThere -and $Machine -eq '9045' -and $t -eq $null) {
    $plan += @{ live = $live; want = $want; act = 'ok' }
    continue
  }
  if (-not $wantThere -and -not ($canCreate -contains $r) -and -not ($t -eq '' -and $Machine -eq '9045')) {
    Refuse "$want does not exist -- put HT$Machine's data there first (nothing was changed)" 6
  }
  if ($t -eq '') {
    $keep = Norm (Join-Path $R45 $r)
    if (Test-Path -LiteralPath $keep) { Refuse "$live is a real folder and $keep already exists -- move one of them by hand" 4 }
    if ((Split-Path -Qualifier $live) -ine (Split-Path -Qualifier $keep)) { Refuse "$live and $keep are on different drives (a move would copy)" 4 }
    $plan += @{ live = $live; want = $want; act = 'move'; keep = $keep }
  } elseif ($t -eq $null) {
    $plan += @{ live = $live; want = $want; act = 'link' }
  } elseif ($t -eq '?') {
    Refuse "$live is a link whose target cannot be read" 4
  } elseif ($t -ieq $want) {
    $plan += @{ live = $live; want = $want; act = 'ok' }
  } else {
    $plan += @{ live = $live; want = $want; act = 'relink'; old = $t }
  }
}

$todo = @($plan | Where-Object { $_.act -ne 'ok' })
if (-not $todo.Count) { foreach ($p in $plan) { Write-Output ("[machine] {0} -> {1} (already)" -f $p.live, $p.want) }; Write-Output "[machine] HT$Machine data: $target"; exit 0 }

if (-not $NoProcessCheck) {
  # (1009 review #7: the tests (test_automation writes IniData) and the probes too)
  # (1009 review (F5 #3): every HT9045* / H904x*GPIB* build, and programs started from D:\HT9045 / D:\GPIB9045)
  $busy = Get-Process -ErrorAction SilentlyContinue | Where-Object { ($_.ProcessName -match '^(wb_serve|wb_publish|wb_gateway|ioweb_probe|pci1203_linkprobe|test_.*|HT9045.*|H904\d.*GPIB.*)$' -or ($_.Path -and $_.Path -match '^[A-Za-z]:\\(HT9045|GPIB9045)\\')) }
  if ($busy) { Refuse ("still running: " + (($busy | ForEach-Object { $_.ProcessName + '(' + $_.Id + ')' }) -join ', ') + ' -- close it first') 3 }
}

# every move tried first: renamed to a probe name and back (an open file inside, a locked folder fails here, before anything)
foreach ($p in ($todo | Where-Object { $_.act -eq 'move' })) {
  $probe = $p.live + '.htdprobe'
  try { [IO.Directory]::Move($p.live, $probe); [IO.Directory]::Move($probe, $p.live) }
  catch {
    $why0 = $_.Exception.Message.Trim()
    # (1009 second review (switch #7): the move back failing too -- said with what to do, not a bare error)
    if ((Test-Path -LiteralPath $probe) -and -not (Test-Path -LiteralPath $p.live)) {
      try { [IO.Directory]::Move($probe, $p.live) }
      catch { Refuse ("$($p.live) is now at $probe and could not be moved back (" + $_.Exception.Message.Trim() + ") -- rename $probe back to $($p.live) by hand before starting anything") 9 }
    }
    Refuse ("$($p.live) cannot be moved now (" + $why0 + ') -- a file in it is open?') 5
  }
}

$done = @()
try {
  foreach ($p in $plan) {
    if ($p.act -eq 'ok') { Write-Output ("[machine] {0} -> {1} (already)" -f $p.live, $p.want); continue }
    $entry = @{ live = $p.live; linked = $false }
    $done += $entry   # (the same table: what is filled in below is what the undo sees)
    if ($p.act -eq 'move') {
      if (-not (Test-Path -LiteralPath (Split-Path $p.keep))) { New-Item -ItemType Directory -Force -Path (Split-Path $p.keep) | Out-Null; $entry.madeDirs = @(Split-Path $p.keep) }   # (review of 0.427 #1: made here -- an empty one is taken away on a put-back)
      [IO.Directory]::Move($p.live, $p.keep)
      $entry.undoMove = $p.keep
      Write-Output ("[machine] moved the real folder {0} -> {1}" -f $p.live, $p.keep)
    } elseif ($p.act -eq 'relink') {
      if ((LinkTarget $p.live) -in @('', $null)) { throw "$($p.live) changed while switching" }
      [IO.Directory]::Delete($p.live, $false)   # (a junction: the link only, never its target)
      $entry.undoLink = $p.old
    } elseif ($p.act -eq 'unlink') {
      if ((LinkTarget $p.live) -in @('', $null)) { throw "$($p.live) changed while switching" }
      [IO.Directory]::Delete($p.live, $false)   # (the link only)
      $entry.undoLink = $p.old
      Write-Output ("[machine] {0}: link to HT9050's taken away (HT9045 has none)" -f $p.live)
      continue
    }
    if (-not (Test-Path -LiteralPath $p.want)) { New-Item -ItemType Directory -Force -Path $p.want | Out-Null }
    $parent = Split-Path $p.live
    if ($parent -and -not (Test-Path -LiteralPath $parent)) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }
    New-Item -ItemType Junction -Path $p.live -Target $p.want | Out-Null
    $entry.linked = $true
    Write-Output ("[machine] {0} -> {1}" -f $p.live, $p.want)
  }
} catch {
  $why = $_.Exception.Message.Trim()
  # (put back what this run changed, newest first)
  $undoFail = 0
  for ($k = $done.Count - 1; $k -ge 0; $k--) {
    $d = $done[$k]
    try {
      if ($d.linked -and (LinkTarget $d.live) -notin @('', $null)) { [IO.Directory]::Delete($d.live, $false) }
      if ($d.undoMove) { [IO.Directory]::Move($d.undoMove, $d.live) }
      elseif ($d.undoLink) { New-Item -ItemType Junction -Path $d.live -Target $d.undoLink | Out-Null }
      # (review of 0.427 #1: the folders this run made, now empty, away too -- up to the root it made)
      foreach ($md in @($d.madeDirs)) { $cur = $md; while ($cur -and (Test-Path -LiteralPath $cur) -and -not (Get-ChildItem -LiteralPath $cur -Force) -and -not (Under $LR $cur)) { [IO.Directory]::Delete($cur); $cur = Split-Path $cur -Parent; if (-not $cur -or (Test-Path -LiteralPath $cur) -and (Get-ChildItem -LiteralPath $cur -Force)) { break } } }
    } catch { $undoFail++; Write-Output ("[machine] could not put back {0}: {1}" -f $d.live, $_.Exception.Message.Trim()) }
  }
  # (1009 second review (switch #8): not said "put back" when some could not be -- the folders may be half switched)
  if ($undoFail) { Write-Output "[machine] failed, and $undoFail change(s) could NOT be put back -- the folders may be half switched, check the five links: $why"; exit 9 }
  Write-Output "[machine] failed, changes put back: $why"
  exit 5
}
Write-Output "[machine] HT$Machine data: $target"
exit 0
