# AI(W906-HTDESIGNER) 20261001: the component tree with tab sheets (WPF Document Outline / BCB6 Object TreeView),
# on the real HW.IoSetView.html in headless Edge (tree_driver.js). Read-only for the trees; scratch in %TEMP%.
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$web = Join-Path $here '..\..\..\..\web'
$page = (Resolve-Path (Join-Path $web 'page\HW.IoSetView.html')).Path
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $edge) { Write-Host 'msedge.exe not found'; exit 2 }
$work = Join-Path $env:TEMP 'htdesigner_tree'
New-Item -ItemType Directory -Force $work | Out-Null
$out = Join-Path (Split-Path $page) '__htd_tree_test.html'
$env:ELECTRON_RUN_AS_NODE = '1'
$b = Start-Process -FilePath $code -ArgumentList "`"$(Join-Path $here 'build_harness.js')`"", "`"$page`"", "`"$out`"", "`"$(Join-Path $here 'tree_driver.js')`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
if ($b.ExitCode -ne 0 -or -not (Test-Path $out)) { Write-Host 'build_harness failed'; exit 1 }
$dom = Join-Path $work 'dom.html'
$profileDir = Join-Path $work 'edge'
try {
  $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
    '--allow-file-access-from-files', '--virtual-time-budget=8000', '--window-size=1400,1000', '--dump-dom', ([Uri]$out).AbsoluteUri)
  Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow -RedirectStandardOutput $dom -RedirectStandardError (Join-Path $work 'err.txt') | Out-Null
} finally {
  Remove-Item $out -Force -ErrorAction SilentlyContinue
}
$text = Get-Content $dom -Raw -Encoding utf8
$m = [regex]::Match($text, '<pre id="HTDTEST">HTDTEST(\{.*?\})HTDEND</pre>', 'Singleline')
if (-not $m.Success) { Write-Host 'no result'; exit 1 }
$json = [System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value)
[IO.File]::WriteAllText((Join-Path $work 'result.json'), $json, (New-Object System.Text.UTF8Encoding($false)))
$R = $json | ConvertFrom-Json
$pass = 0; $fail = 0
function ok($c, $n, $x) { if ($c) { $script:pass++; Write-Host "PASS  $n   $x" } else { $script:fail++; Write-Host "FAIL  $n   $x" } }
if ($R.error) { Write-Host "ERROR $($R.error)" }
$sheetKids = @($R.pgcKids | Where-Object { $_ -like '*:TTabSheet:*' })
ok ($R.pgc -and $sheetKids.Count -ge 5 -and $R.pgcNonSheetKids -eq 0) 'tree: pgcStack1 (TPageControl) branches into its tab sheets, nothing else directly under it' ($R.pgcKids -join ' | ')
ok ($R.cassette -and $R.cassette.parent -eq 'pgcStack1' -and $R.cassette.kids -gt 0 -and $R.cassette.cap -eq 'Cassette' -and -not $R.cassette.hidden) 'tree: tsStack1_Cassette : TTabSheet "Cassette" under pgcStack1, its controls under it, not hidden' ($R.cassette | ConvertTo-Json -Compress)
ok ($R.otherTabCount -gt 0 -and @($R.otherTabHidden).Count -eq 0) 'tree: controls on a tab that is not the active one are not marked hidden (BCB6 / WPF)' ("$($R.otherTabCount) controls, hidden: " + (@($R.otherTabHidden) -join ','))
ok ($R.selId -eq 'tsStack1_Cassette' -and $R.paneShown -and $R.activeTab -eq 'Cassette') 'tree: a sheet picked in the tree is selected and its tab shown (WPF Document Outline)' "sel=$($R.selId) shown=$($R.paneShown) tab=$($R.activeTab)"
ok ($R.plainSel -eq 'pnlStack1') 'tree: an ordinary id still selects as before' "sel=$($R.plainSel)"
ok ($R.placePane -match '^tsStack1_Cassette@\d+,\d+$') 'toolbox: a tool put down on a tab sheet names the sheet, with the point in it (0.135, WPF: it goes into that TabItem)' "$($R.placePane)"
ok ($R.sheets -ge 20) 'tree: every PageControl of the page has its sheets' "sheets=$($R.sheets) nodes=$($R.count)"
Write-Host "RESULT: $pass pass, $fail fail"
if ($fail) { exit 1 }
exit 0
