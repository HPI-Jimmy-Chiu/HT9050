# AI(W906-SNAPSHOT-PUSH) 20261003: EastSun「我做了什麼事情 也幫我推上去 不是只有改軟體才要推 嚴格執行」.
# Snapshot-only push of what EastSun did on the machine (settings, work order, operation log), then a look at GitHub main for a
# new laptop package. Run every 30 min by the machine-side Claude session (and by hand any time):
#   powershell -NoProfile -ExecutionPolicy Bypass -File D:\HT9045\_push_github_20260926\_tools\push_snapshot.ps1
# Exit 0 = pushed or nothing to push; prints NEW-PACKAGE lines when main has moved past the last integrated package.
$ErrorActionPreference = 'Stop'
$PUSHDIR = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$dirty = @(git -C $PUSHDIR status --porcelain | Where-Object { $_ -notmatch '_tools/' })
if ($dirty.Count) { "push folder not clean (another push in progress?) -- skipped: $($dirty -join '; ')"; exit 0 }
git -C $PUSHDIR pull --ff-only -q origin machine/integ-ioweb
if ($LASTEXITCODE) { 'pull --ff-only failed -- skipped'; exit 0 }
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'push_stage.ps1') -Commit
$rc = $LASTEXITCODE
if ($rc -ne 0) {
  # AI(W906-SNAPSHOT-PUSH) 20261003: exit 4 with an empty remote = GitHub not reachable (measured 12:49: "Failed to connect to
  #   github.com:443"). Put the folder back as it was, so the next run (30 min later) retries instead of skipping on "not clean";
  #   the snapshot is taken again then. Any other failure (scan hits = 3) is left for a look.
  $remoteNow = (git -C $PUSHDIR ls-remote origin refs/heads/machine/integ-ioweb 2>$null)
  if ($rc -eq 4 -and -not $remoteNow) {
    git -C $PUSHDIR checkout -- README.txt MANIFEST_MD5.tsv machine_params workorder machine_log 2>$null
    git -C $PUSHDIR clean -fdq -- machine_params workorder machine_log 2>$null
    'GITHUB-UNREACHABLE: nothing pushed; push folder restored, the next run retries'
  } else {
    "push_stage exit $rc -- NOT pushed; the push folder is left as it is for a look"
  }
}
# new laptop package on main?
$FROM = 'D:\HT9045\_from_github'
git -C $FROM fetch -q origin
$last = (Get-Content (Join-Path $PSScriptRoot 'last_integrated_main.txt') -ErrorAction SilentlyContinue | Select-Object -First 1)
if ($last) {
  $newer = @(git -C $FROM log --oneline "$last..origin/main")
  foreach ($c in $newer) { "NEW-PACKAGE? $c" }
  if (-not $newer.Count) { "main: nothing newer than $last" }
}
exit 0
