# AI(W906-HTDESIGNER) 20261001: push GIVEN commits of the web repo (-Kind web) or of the C++ port tree (-Kind cpp) to
# GitHub branch machine/integ-ioweb as web\NNNN-*.patch / cpp\NNNN-*.patch, numbered after the last one there.
# ASCII-only script (Windows PowerShell 5.1). The designer session uses it only for its OWN commits (the IO page's
# page/ht9045_io_*.js, a C++ fix asked for); the laptop-package integration has its own owner -- never push theirs.
#   -Commits  the commits, oldest first (or one comma separated string)
#   -Section  a UTF-8 text file: the README.txt section to add (before README.txt's last line "MD5 ...")
#   -Expect   the remote branch head it must still be at (fast-forward only); default = the push folder's HEAD
param([Parameter(Mandatory)][ValidateSet('web', 'cpp')][string]$Kind, [Parameter(Mandatory)][string[]]$Commits,
  [Parameter(Mandatory)][string]$Section, [string]$Expect = '', [string]$PushDir = 'D:\HT9045\_push_github_20260926',
  [string]$WebRepo = 'D:\HT9045\_integ_ioweb\web', [string]$Message = '',
  [string]$Trailer = 'Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>')
$ErrorActionPreference = 'Stop'
$P = $PushDir
$W = if ($Kind -eq 'web') { $WebRepo } else { (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path }
$u8 = New-Object System.Text.UTF8Encoding($false)
if (-not $Expect) { $Expect = (git -C $P rev-parse HEAD).Trim() }
if ($Commits.Count -eq 1 -and $Commits[0] -match ',') { $Commits = @($Commits[0] -split ',' | ForEach-Object { $_.Trim() } | Where-Object { $_ }) }
$have = @(Get-ChildItem "$P\$Kind" -Filter *.patch | ForEach-Object { [int]($_.Name.Substring(0, 4)) } | Sort-Object)[-1]
$tmp = Join-Path $env:TEMP ('htd_' + $Kind + '_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $tmp | Out-Null
$n = $have + 1
foreach ($h in $Commits) {
  $full = (git -C $W rev-parse $h).Trim()
  if (Select-String -Path "$P\$Kind\*.patch" -Pattern ('^From ' + $full) -Quiet) { throw "commit $full is already in $Kind\" }
  # files the series keeps out (the C++ tree's machine-only / shared build files)
  if ($Kind -eq 'cpp') {
    $bad = @(git -C $W show --name-only --format='' $full | Where-Object { $_ -match '^(MachineType\.h|CMakeLists\.txt|forms/fMain\.(cpp|h))$' })
    if ($bad.Count) { throw "commit $full touches files kept out of the series: $($bad -join ', ')" }
  }
  git -C $W format-patch -1 $full --start-number $n -o $tmp -q | Out-Null
  $n++
}
$keyNames = @(('github-' + 'ht9050'), ('id_' + 'ed25519'), ('id_' + 'rsa')) -join '|'
$pat = 'BEGIN [A-Z ]*PRIVATE KEY|ghp_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}|gho_[A-Za-z0-9]{20,}|AKIA[0-9A-Z]{16}|xox[baprs]-[A-Za-z0-9-]{10,}|7z(a|\.exe)?\b[^\r\n]*\s-p\S+|\bpassword\s*[:=]\s*["''][^"'']{3,}|\bpasswd\b|api[_-]?key\s*[:=]|secret\s*[:=]\s*["''][^"'']{6,}|ssh-(rsa|ed25519) AAAA|' + $keyNames
$hits = @(Get-ChildItem $tmp -Filter *.patch | Select-String -Pattern $pat)
$cfg = @(Get-ChildItem $tmp -Filter *.patch | Select-String -Pattern '^\+\+\+ b/.*(Mot_Table|IO_Table|[Gg]ener[a]?l\.ini|Gerneral\.ini|MachineType\.h)')
if ($hits.Count -or $cfg.Count) { $hits | Select-Object -First 5 | ForEach-Object { Write-Host $_ }; throw "scan: $($hits.Count) secret-like, $($cfg.Count) config" }
Write-Host ("scan clean; new patches: " + (@(Get-ChildItem $tmp -Filter *.patch) | ForEach-Object { $_.Name }) -join ', ')
$r = ((git -C $P ls-remote origin refs/heads/machine/integ-ioweb) -split '\s+')[0]
if ($r -ne $Expect -or (git -C $P rev-parse HEAD) -ne $Expect) { throw "remote / local head is not $Expect (remote $r)" }
if (@(git -C $P status --short).Count) { throw 'the push folder has uncommitted changes' }
Copy-Item "$tmp\*.patch" "$P\$Kind\"
$rd = [IO.File]::ReadAllText("$P\README.txt", $u8)
$lastLine = ($rd.TrimEnd("`n") -split "`n")[-1] + "`n"
$sec = [IO.File]::ReadAllText($Section, $u8).Replace("`r`n", "`n")
if (-not $sec.EndsWith("`n")) { $sec += "`n" }
[IO.File]::WriteAllText("$P\README.txt", $rd.Substring(0, $rd.Length - $lastLine.Length) + $sec + $lastLine, $u8)
$files = Get-ChildItem $P -Recurse -File | Where-Object { $_.FullName -notmatch '\\\.git\\' -and $_.Name -ne 'MANIFEST_MD5.tsv' }
$rows = $files | ForEach-Object { [pscustomobject]@{ rel = $_.FullName.Substring($P.Length + 1); len = $_.Length; md5 = (Get-FileHash $_.FullName -Algorithm MD5).Hash } } | Sort-Object rel
$lines = @("path`tbytes`tmd5") + ($rows | ForEach-Object { "$($_.rel)`t$($_.len)`t$($_.md5)" })
[IO.File]::WriteAllText("$P\MANIFEST_MD5.tsv", (($lines -join "`r`n") + "`r`n"), $u8)
git -C $P add -- $Kind README.txt MANIFEST_MD5.tsv
$first = $have + 1; $last = $n - 1
$label = if ($Kind -eq 'web') { 'web' } else { 'C++' }
$msg = "machine/integ-ioweb: + {0} {1:D4}-{2:D4} ({3} {4}..{5}){6}" -f $Kind, $first, $last, $label, $Commits[0].Substring(0, 7), $Commits[-1].Substring(0, 7), $(if ($Message) { ' -- ' + $Message } else { '' })
$mf = Join-Path $tmp 'msg.txt'
[IO.File]::WriteAllText($mf, $msg + "`n`n" + $Trailer + "`n", $u8)
git -C $P commit -q -F $mf
git -C $P push origin machine/integ-ioweb
$r2 = ((git -C $P ls-remote origin refs/heads/machine/integ-ioweb) -split '\s+')[0]
Write-Host "pushed: $Expect -> $r2 (local $(git -C $P rev-parse HEAD)); manifest lines $($lines.Count)"
Remove-Item $tmp -Recurse -Force
