# AI(W906-HTDESIGNER) 20261001: push the designer tool's new commits to GitHub branch machine/integ-ioweb as
# tools\NNNN-*.patch (one git format-patch per commit). ASCII-only script (Windows PowerShell 5.1).
#   -Section  a UTF-8 text file: the README.txt section to add (put before README.txt's last line "MD5 ...")
#   -Expect   the remote branch head it must still be at (fast-forward only); default = the push folder's HEAD
#   -PushDir  the push folder (a clone of the GitHub repo on branch machine/integ-ioweb)
#   -Branch   the port tree's branch whose commits are counted (default: the one checked out)
# Every commit since the last tools\ patch that touched tools/vscode-htdesigner must touch ONLY that folder.
# The patches are scanned first (tokens, private keys, archive passwords, deploy key names, machine config files):
# the repo is PUBLIC.
param([Parameter(Mandatory)][string]$Section, [string]$Expect = '', [string]$PushDir = 'D:\HT9045\_push_github_20260926',
  [string]$Branch = '', [string]$Message = '',
  [string]$Trailer = 'Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>')
$ErrorActionPreference = 'Stop'
$P = $PushDir
$T = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$u8 = New-Object System.Text.UTF8Encoding($false)
if (-not $Branch) { $Branch = (git -C $T rev-parse --abbrev-ref HEAD).Trim() }
if (-not $Expect) { $Expect = (git -C $P rev-parse HEAD).Trim() }
# 1. which commits are new: every commit that touched the tool folder, after the ones already in tools\
$all = @(git -C $T log --reverse --format='%H' $Branch -- tools/vscode-htdesigner)
$have = @(Get-ChildItem "$P\tools" -Filter *.patch).Count
if ($all.Count -le $have) { Write-Host "nothing new ($have patches, $($all.Count) commits)"; exit 0 }
$new = $all[$have..($all.Count - 1)]
$tmp = Join-Path $env:TEMP ('htd_tools_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $tmp | Out-Null
$n = $have + 1
foreach ($h in $new) {
  # only the tool folder, or stop
  # (--relative: paths from the port tree -- on the machine it is the repo's root; on a GitLab clone it is a
  # subfolder, HT9011UC_Cpp_V3.33.906.0/, and the patches must still say tools/vscode-htdesigner/ -- AI 20261001)
  $all1 = @(git -C $T show --name-only --format='' $h | Where-Object { $_ })
  $rel1 = @(git -C $T show --relative --name-only --format='' $h | Where-Object { $_ })
  $other = @($rel1 | Where-Object { $_ -notmatch '^tools/vscode-htdesigner/' })
  if ($other.Count -or $rel1.Count -ne $all1.Count) { throw "commit $h touches other files: $((@($other) + @($all1 | Where-Object { $_ -notmatch 'tools/vscode-htdesigner/' })) -join ', ')" }
  git -C $T format-patch --relative -1 $h --start-number $n -o $tmp -q | Out-Null
  $n++
}
# 2. scan (the deploy key names are put together here, so this file itself does not match them)
$keyNames = @(('github-' + 'ht9050'), ('id_' + 'ed25519'), ('id_' + 'rsa')) -join '|'
$pat = 'BEGIN [A-Z ]*PRIVATE KEY|ghp_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}|gho_[A-Za-z0-9]{20,}|AKIA[0-9A-Z]{16}|xox[baprs]-[A-Za-z0-9-]{10,}|7z(a|\.exe)?\b[^\r\n]*\s-p\S+|\bpassword\s*[:=]\s*["''][^"'']{3,}|\bpasswd\b|api[_-]?key\s*[:=]|secret\s*[:=]\s*["''][^"'']{6,}|ssh-(rsa|ed25519) AAAA|' + $keyNames
$hits = @(Get-ChildItem $tmp -Filter *.patch | Select-String -Pattern $pat)
$cfg = @(Get-ChildItem $tmp -Filter *.patch | Select-String -Pattern '^\+\+\+ b/.*(Mot_Table|IO_Table|[Gg]ener[a]?l\.ini|Gerneral\.ini|MachineType\.h)')
if ($hits.Count -or $cfg.Count) { $hits | Select-Object -First 5 | ForEach-Object { Write-Host $_ }; throw "scan: $($hits.Count) secret-like, $($cfg.Count) config" }
Write-Host ("scan clean; new patches: " + (@(Get-ChildItem $tmp -Filter *.patch) | ForEach-Object { $_.Name }) -join ', ')
# 3. the remote has not moved (another session may have pushed: pull the push folder first, then run again)
$r = ((git -C $P ls-remote origin refs/heads/machine/integ-ioweb) -split '\s+')[0]
# (AI 20261001: the push folder may be AHEAD of the remote -- an earlier round committed but its push was refused;
#  those commits go up with this one. Still fast-forward only: the remote must be where we last saw it.)
$head0 = (git -C $P rev-parse HEAD).Trim()
if ($Expect -eq $head0 -and $r -ne $head0) { git -C $P merge-base --is-ancestor $r $head0; if ($LASTEXITCODE -eq 0) { $Expect = $r } }
git -C $P merge-base --is-ancestor $Expect $head0
$anc = ($LASTEXITCODE -eq 0)
if ($r -ne $Expect -or -not $anc) { throw "remote head is $r, expected $Expect (local $head0): someone else pushed -- fetch, put these on top, then run again" }
if (@(git -C $P status --short).Count) { throw 'the push folder has uncommitted changes' }
# 4. copy, README section, manifest
Copy-Item "$tmp\*.patch" "$P\tools\"
$rd = [IO.File]::ReadAllText("$P\README.txt", $u8)
$lastLine = ($rd.TrimEnd("`n") -split "`n")[-1] + "`n"
$sec = [IO.File]::ReadAllText($Section, $u8).Replace("`r`n", "`n")
if (-not $sec.EndsWith("`n")) { $sec += "`n" }
[IO.File]::WriteAllText("$P\README.txt", $rd.Substring(0, $rd.Length - $lastLine.Length) + $sec + $lastLine, $u8)
$files = Get-ChildItem $P -Recurse -File | Where-Object { $_.FullName -notmatch '\\\.git\\' -and $_.Name -ne 'MANIFEST_MD5.tsv' }
$rows = $files | ForEach-Object { [pscustomobject]@{ rel = $_.FullName.Substring($P.Length + 1); len = $_.Length; md5 = (Get-FileHash $_.FullName -Algorithm MD5).Hash } } | Sort-Object rel
$lines = @("path`tbytes`tmd5") + ($rows | ForEach-Object { "$($_.rel)`t$($_.len)`t$($_.md5)" })
[IO.File]::WriteAllText("$P\MANIFEST_MD5.tsv", (($lines -join "`r`n") + "`r`n"), $u8)
# 5. commit + push (fast-forward)
git -C $P add -- tools README.txt MANIFEST_MD5.tsv
$first = $have + 1; $last = $n - 1
$msg = "machine/integ-ioweb: + tools {0:D4}-{1:D4} (HTDESIGNER {2}..{3}; only tools/vscode-htdesigner){4}" -f $first, $last, $new[0].Substring(0, 7), $new[-1].Substring(0, 7), $(if ($Message) { ' -- ' + $Message } else { '' })
$mf = Join-Path $tmp 'msg.txt'
[IO.File]::WriteAllText($mf, $msg + "`n`n" + $Trailer + "`n", $u8)
git -C $P commit -q -F $mf
git -C $P push origin machine/integ-ioweb
$pushEc = $LASTEXITCODE
$r2 = ((git -C $P ls-remote origin refs/heads/machine/integ-ioweb) -split '\s+')[0]
$local = (git -C $P rev-parse HEAD).Trim()
# (AI 20261001: a refused push -- no login, no rights -- used to print "pushed: A -> A" and exit 0. The commit stays in
#  the push folder; push it by hand with: git -C <PushDir> push origin machine/integ-ioweb)
if ($pushEc -ne 0 -or $r2 -ne $local) {
  Write-Host "NOT pushed (git push exit $pushEc; remote $r2, local $local). The commit is in $P -- push it by hand: git -C `"$P`" push origin machine/integ-ioweb"
  Remove-Item $tmp -Recurse -Force
  exit 1
}
Write-Host "pushed: $Expect -> $r2 (local $local); manifest lines $($lines.Count)"
Remove-Item $tmp -Recurse -Force
