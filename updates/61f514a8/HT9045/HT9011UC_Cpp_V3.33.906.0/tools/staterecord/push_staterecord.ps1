# AI(W906-SRRELAY) 20261005: push ONE State Record to GitHub branch machine/integ-ioweb as
#   dispatch/<yyyyMMdd>_staterecord_<HHmmss>/  =  the record's .zip + REQUEST.md (the operator's note + provenance)
# so NB2-1 (or anyone) can fetch and analyse it (tools/staterecord/fetch_staterecord.py).  RULINGS_20261004 #1:
# on any machine problem press State Record FIRST (HOME stuck: before Abort), then push it.  Not golden; not built.
# ASCII-only on purpose: Windows PowerShell 5.1 on an ACP-950 machine reads a BOM-less non-ASCII .ps1 as ANSI.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File push_staterecord.ps1 -Note "HOME stuck at step 1520, pressed before Abort"
#   ... -NoteFile D:\note.txt          (UTF-8 text; use this for Chinese when the caller is cmd / Git Bash)
#   ... -Record "D:\HT9045_StateRecord\2026-10-05 10_15_30.zip"    (default = the newest record)
#   ... -Preview                       (show what would be pushed; nothing is copied, committed or pushed)
#
# The record: wb_serve writes D:\HT9045_StateRecord\<yyyy-MM-dd HH_mm_ss>\, a background job copies logs / config into it,
# zips it to <same name>.zip and deletes the folder (golden main.cpp:26194-26201).  This script waits for that zip; when
# the folder is still there after -WaitSec (zip step failed, e.g. no 7-Zip) it zips the folder itself and says so.
param(
  [string]$Note = '',
  [string]$NoteFile = '',
  [string]$Record = '',
  [string]$SrRoot = 'D:\HT9045_StateRecord',
  [string]$PushDir = 'D:\HT9045\_push_github_20260926',
  [string]$Branch = 'machine/integ-ioweb',
  [string]$WebRepo = 'D:\HT9045\_integ_ioweb\web',
  [int]$WaitSec = 900,
  [switch]$Preview,
  [string]$Trailer = ''
)
$ErrorActionPreference = 'Stop'
$u8 = New-Object System.Text.UTF8Encoding($false)
$MaxBytes = 95MB      # GitHub refuses a file over 100 MB
$WarnBytes = 45MB     # GitHub warns over 50 MB
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

# git with stderr kept apart: under 'Stop', PowerShell 5.1 turns a redirected native stderr line into a terminating error
function GitRun([string[]]$GitArgs, [switch]$Soft) {
  $ea = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
  try { $all = @(& git.exe @GitArgs 2>&1); $code = $LASTEXITCODE } finally { $ErrorActionPreference = $ea }
  $out = @($all | Where-Object { $_ -isnot [System.Management.Automation.ErrorRecord] } | ForEach-Object { "$_" })
  $err = @($all | Where-Object { $_ -is [System.Management.Automation.ErrorRecord] } | ForEach-Object { "$_" })
  $script:GitCode = $code
  $script:GitErr = ($err -join ' | ')
  if ($code -ne 0 -and -not $Soft) { throw ("git " + ($GitArgs -join ' ') + " failed (exit $code): " + $script:GitErr) }
  return $out
}
function One($lines) { return (@($lines) -join "`n").Trim() }
function Find7z {
  foreach ($p in @('D:\HT9045\7z.exe', "$env:ProgramFiles\7-Zip\7z.exe", "${env:ProgramFiles(x86)}\7-Zip\7z.exe")) {
    if ($p -and (Test-Path -LiteralPath $p)) { return $p }
  }
  return ''
}
# A record name starts with the press time: "yyyy-MM-dd HH_mm_ss" (a suffix such as "_hang" is kept).  "_tasklist" folders
# are the flow-compare dumps (act.main.stateRecord taskListOnly), not records -- never picked by default.
$NameRx = '^(\d{4})-(\d{2})-(\d{2}) (\d{2})_(\d{2})_(\d{2})(.*)$'

# ---- the note ---------------------------------------------------------------------------------------------------------
if ($NoteFile) { $Note = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $NoteFile).Path, $u8) }
$Note = $Note.Trim()
if (-not $Note) { throw 'give -Note "what happened, when, what you did before pressing State Record" (or -NoteFile)' }

# ---- which record -----------------------------------------------------------------------------------------------------
if (-not $Record) {
  if (-not (Test-Path -LiteralPath $SrRoot)) { throw "no State Record folder $SrRoot -- press State Record first" }
  $cand = @(Get-ChildItem -LiteralPath $SrRoot | Where-Object {
      $_.Name -match $NameRx -and $_.Name -notmatch '_tasklist' -and ($_.PSIsContainer -or $_.Extension -eq '.zip') } |
    Sort-Object Name -Descending)
  if (-not $cand.Count) { throw "no record under $SrRoot -- press State Record first" }
  $stem = if ($cand[0].PSIsContainer) { $cand[0].Name } else { [IO.Path]::GetFileNameWithoutExtension($cand[0].Name) }
  $Record = Join-Path $SrRoot $stem
}
$Record = $Record.TrimEnd('\')
if ($Record -like '*.zip') { $folder = $Record.Substring(0, $Record.Length - 4); $zip = $Record }
else { $folder = $Record; $zip = $Record + '.zip' }
$stem = Split-Path $folder -Leaf
if (-not ($stem -match $NameRx)) { throw "'$stem' is not a State Record name (yyyy-MM-dd HH_mm_ss...)" }
$pressed = '{0}-{1}-{2} {3}:{4}:{5}' -f $Matches[1], $Matches[2], $Matches[3], $Matches[4], $Matches[5], $Matches[6]
$suffix = ($Matches[7] -replace '[^A-Za-z0-9_-]', '_')
$destName = '{0}{1}{2}_staterecord_{3}{4}{5}{6}' -f $Matches[1], $Matches[2], $Matches[3], $Matches[4], $Matches[5], $Matches[6], $suffix

# wait while wb_serve's background job is still copying / zipping (it deletes the folder after a good zip);
# a folder nobody has written to for 10 minutes is a leftover (the zip step failed) -- no point waiting for it
$zippedBy = 'wb_serve (golden DoStateRecord: 7z a -tzip, then the folder is deleted)'
if (Test-Path -LiteralPath $folder) {
  $idle = ((Get-Date) - (Get-Item -LiteralPath $folder).LastWriteTime).TotalMinutes
  $t0 = Get-Date
  while ($idle -lt 10 -and (Test-Path -LiteralPath $folder) -and ((Get-Date) - $t0).TotalSeconds -lt $WaitSec) {
    Write-Host ("waiting for wb_serve to finish the record ({0:N0} s; it deletes the folder after its zip)" -f ((Get-Date) - $t0).TotalSeconds)
    Start-Sleep -Seconds 5
  }
}
$tmp = Join-Path $env:TEMP ('srpush_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $tmp | Out-Null
try {
  if (Test-Path -LiteralPath $folder) {
    # the zip step did not finish: zip the folder ourselves (top-level folder name kept, like 7z a <folder>.zip <folder>)
    $zippedBy = "push_staterecord.ps1 (the record folder was still there -- wb_serve's zip step did not finish)"
    $zip = Join-Path $tmp ($stem + '.zip')
    $z7 = Find7z
    if ($z7) {
      & $z7 a -tzip $zip $folder -bso0 -bsp0
      if ($LASTEXITCODE -ne 0) { throw "7z could not zip $folder (exit $LASTEXITCODE)" }
    } else {
      [IO.Compression.ZipFile]::CreateFromDirectory($folder, $zip, [IO.Compression.CompressionLevel]::Optimal, $true)
      $zippedBy += ' with .NET ZipFile (no 7-Zip found)'
    }
  }
  if (-not (Test-Path -LiteralPath $zip)) { throw "no record zip $zip" }

  # ---- check the zip --------------------------------------------------------------------------------------------------
  $za = [IO.Compression.ZipFile]::OpenRead($zip)
  try { $entries = $za.Entries.Count } finally { $za.Dispose() }
  if ($entries -lt 1) { throw "$zip is empty" }
  $len = (Get-Item -LiteralPath $zip).Length
  if ($len -gt $MaxBytes) { throw ("{0} is {1:N1} MB -- over GitHub's 100 MB per-file limit; repack it smaller and pass -Record" -f $zip, ($len / 1MB)) }
  if ($len -gt $WarnBytes) { Write-Host ("warning: {0:N1} MB (GitHub warns over 50 MB; still accepted)" -f ($len / 1MB)) }
  $md5 = (Get-FileHash -LiteralPath $zip -Algorithm MD5).Hash

  # ---- provenance ---------------------------------------------------------------------------------------------------
  $cppTree = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
  $cppRev = One (GitRun @('-C', $cppTree, 'log', '-1', '--format=%h %ci') -Soft); if ($script:GitCode -ne 0 -or -not $cppRev) { $cppRev = '(not a git tree)' }
  $webRev = '(no web repo at ' + $WebRepo + ')'
  if (Test-Path -LiteralPath (Join-Path $WebRepo '.git')) { $w = One (GitRun @('-C', $WebRepo, 'log', '-1', '--format=%h %ci') -Soft); if ($script:GitCode -eq 0 -and $w) { $webRev = $w } }
  $now = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
  $zipName = $stem + '.zip'
  $req = @(
    "# State Record $pressed -- $env:COMPUTERNAME",
    '',
    "- Pressed: $pressed (from the record name)   Pushed: $now",
    ("- File: ``{0}``  {1} bytes  MD5 {2}  ({3} entries)" -f $zipName, $len, $md5, $entries),
    "- Zipped by: $zippedBy",
    "- Machine: $env:COMPUTERNAME   C++ tree: $cppTree @ $cppRev   web: $webRev",
    '',
    '## What happened (operator note)',
    '',
    $Note,
    '',
    '## For the analyst',
    '',
    '- Fetch + extract: `python HT9011UC_Cpp_V3.33.906.0/tools/staterecord/fetch_staterecord.py` (new records only; see its README).',
    '- Read with the skill `ht9045-state-record-analysis` (SOP: references/analysis-sop.md). The port writes no screenshots, no Motor.xls and',
    '  no IO table (St02 S-24 gap analysis, docs/handoff/ST02_S24_GAP_ANALYSIS_20261004.md on v906/steven-handoff) -- read oplog_*.txt too.'
  ) -join "`n"
  $req += "`n"

  if ($Preview) {
    Write-Host 'PREVIEW -- nothing copied, committed or pushed'
    Write-Host "record : $zip"
    Write-Host "target : $Branch dispatch/$destName/ ($zipName + REQUEST.md)"
    Write-Host '----- REQUEST.md -----'
    Write-Host $req
    return
  }

  # ---- the push folder: clean, on the branch, at the remote head ------------------------------------------------------
  $P = $PushDir
  if (-not (Test-Path -LiteralPath (Join-Path $P '.git'))) { throw "$P is not the GitHub push folder (no .git)" }
  $cur = One (GitRun @('-C', $P, 'rev-parse', '--abbrev-ref', 'HEAD'))
  if ($cur -ne $Branch) { throw "$P is on '$cur', not '$Branch'" }
  if (@(GitRun @('-C', $P, 'status', '--porcelain') | Where-Object { $_ }).Count) { throw "$P has uncommitted changes -- the other push scripts use this folder too; finish or clean them first" }
  GitRun @('-C', $P, 'fetch', '-q', 'origin', $Branch) | Out-Null
  $remote = One (GitRun @('-C', $P, 'rev-parse', 'FETCH_HEAD'))
  $local = One (GitRun @('-C', $P, 'rev-parse', 'HEAD'))
  if ($remote -ne $local) {
    GitRun @('-C', $P, 'merge-base', '--is-ancestor', $local, $remote) -Soft | Out-Null
    if ($script:GitCode -ne 0) { throw "local $Branch ($local) is not behind origin ($remote) -- someone has unpushed commits in $P" }
    GitRun @('-C', $P, 'merge', '-q', '--ff-only', $remote) | Out-Null
  }
  $rel = 'dispatch/' + $destName
  $dest = Join-Path $P ('dispatch\' + $destName)
  if (Test-Path -LiteralPath $dest) { throw "$rel is already on the branch (this record was pushed before)" }
  New-Item -ItemType Directory $dest -Force | Out-Null
  Copy-Item -LiteralPath $zip -Destination (Join-Path $dest $zipName)
  [IO.File]::WriteAllText((Join-Path $dest 'REQUEST.md'), $req, $u8)
  if ((Get-FileHash -LiteralPath (Join-Path $dest $zipName) -Algorithm MD5).Hash -ne $md5) { throw 'copy check failed (MD5 differs)' }

  $first = ($Note -split "`n")[0].Trim(); if ($first.Length -gt 100) { $first = $first.Substring(0, 100) + '...' }
  $msg = "dispatch ${destName}: State Record $pressed -- $first"
  if ($Trailer) { $msg += "`n`n" + $Trailer }
  $mf = Join-Path $tmp 'msg.txt'
  [IO.File]::WriteAllText($mf, $msg + "`n", $u8)
  GitRun @('-C', $P, 'add', '--', $rel) | Out-Null
  GitRun @('-C', $P, 'commit', '-q', '-F', $mf) | Out-Null
  GitRun @('-C', $P, 'push', '-q', 'origin', $Branch) -Soft | Out-Null
  if ($script:GitCode -ne 0) {
    # someone pushed in between: our commit only adds a new folder, so the rebase cannot conflict
    Write-Host "push refused ($script:GitErr) -- rebasing on the new remote head and pushing again"
    GitRun @('-C', $P, 'fetch', '-q', 'origin', $Branch) | Out-Null
    GitRun @('-C', $P, 'rebase', '-q', 'FETCH_HEAD') | Out-Null
    GitRun @('-C', $P, 'push', '-q', 'origin', $Branch) | Out-Null
  }
  $head = One (GitRun @('-C', $P, 'rev-parse', 'HEAD'))
  $r2 = ((One (GitRun @('-C', $P, 'ls-remote', 'origin', "refs/heads/$Branch"))) -split '\s+')[0]
  if ($r2 -ne $head) { throw "pushed, but the remote head is $r2, not $head -- check by hand" }
  Write-Host ("pushed {0} ({1}, {2:N1} MB) -> {3} {4}" -f $rel, $zipName, ($len / 1MB), $Branch, $head.Substring(0, 7))
  Write-Host "https://github.com/HPI-Jimmy-Chiu/HT9050/tree/$Branch/$rel"
} finally {
  Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
}
