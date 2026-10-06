# AI(W906-HTDESIGNER) 20261006 (ES02, EastSun: properties -- every one, enumerated): media/props.js drawn in a headless Edge
# with each class's real panel data (test\vscode_props_it.ps1 -Mode dump wrote <Dir>\<cls>.json); EVERY row of the
# property grid is used once the way a person would (a number +3 and Enter, a box clicked, another drop-down item, a
# colour typed, a text typed and Enter, the items list changed) and what it sends is recorded. A row that can be edited
# but sends nothing is a FAIL; a read-only row (grey, DFM only) is listed as such. Writes <Dir>\replay.json for the
# replay in the real VS Code. ASCII only (Windows PowerShell 5.1).
param([Parameter(Mandatory = $true)][string]$Dir)
$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
$media = (Resolve-Path (Join-Path $PSScriptRoot '..\media')).Path
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
$work = Join-Path $env:TEMP 'htd_props_rows'
New-Item -ItemType Directory -Force $work | Out-Null
$profileDir = Join-Path $work 'profile'
$Dir = (Resolve-Path $Dir).Path
$enum = @'
var out = [];
function take(n0) { return window.__out.slice(n0).filter(function (m) { return m.type !== 'ready'; }); }
function key(el, k) { el.dispatchEvent(new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true })); }
var rows = Array.prototype.filter.call(document.querySelectorAll('table.edit tr'), function (tr) { return tr.querySelector('td.k') && !tr.classList.contains('cathead'); });
rows.forEach(function (tr) {
  var name = (tr.querySelector('td.k').getAttribute('data-dname') || tr.querySelector('td.k').textContent || '').trim();
  var v = tr.querySelector('td.v');
  if (!v) return;
  var ctl = v.querySelector('input.num:not(:disabled), input[type=checkbox]:not(:disabled), select:not(:disabled), input.hex:not(:disabled), textarea:not(:disabled), input[type=text]:not(:disabled), input:not([type]):not(:disabled), button.pick:not(:disabled), button.items:not(:disabled)');
  if (!ctl) { out.push({ row: name, kind: 'readonly' }); return; }
  var n0 = window.__out.length, kind = '';
  try {
    if (ctl.matches('input.num')) { kind = 'number'; ctl.focus(); ctl.value = String((+ctl.value || 0) + 3); key(ctl, 'Enter'); ctl.blur(); }
    else if (ctl.matches('input[type=checkbox]')) { kind = 'check'; ctl.click(); }
    else if (ctl.matches('select')) {
      kind = 'select';
      var cur = ctl.selectedIndex, alt = -1;
      for (var i = 0; i < ctl.options.length; i++) if (i !== cur && ctl.options[i].value !== '' && !ctl.options[i].disabled) { alt = i; break; }
      if (alt < 0) { out.push({ row: name, kind: 'select-one-choice' }); return; }
      ctl.selectedIndex = alt; ctl.dispatchEvent(new Event('change', { bubbles: true }));
    }
    else if (ctl.matches('input.hex')) { kind = 'color'; ctl.focus(); ctl.value = '#123456'; key(ctl, 'Enter'); ctl.blur(); }
    else if (ctl.matches('textarea')) { kind = 'items'; ctl.focus(); ctl.value = ctl.value + '\nE2E'; ctl.dispatchEvent(new Event('change', { bubbles: true })); ctl.blur(); }
    else if (ctl.matches('button')) { kind = 'button'; ctl.click(); }
    else { kind = 'text'; ctl.focus(); ctl.value = (ctl.value || '') + 'E2E'; key(ctl, 'Enter'); ctl.dispatchEvent(new Event('change', { bubbles: true })); ctl.blur(); }
  } catch (e) { out.push({ row: name, kind: kind, err: String(e) }); return; }
  var ms = take(n0);
  out.push({ row: name, kind: kind, msgs: ms });
});
/* the Name box on top (WPF): a new name + Enter = rename */
var nb = document.querySelector('.namebox:not(:disabled)');
if (nb) { var n1 = window.__out.length; nb.focus(); nb.value = nb.value + 'E2E'; key(nb, 'Enter'); nb.dispatchEvent(new Event('change', { bubbles: true })); nb.blur(); out.push({ row: '(Name)', kind: 'name', msgs: take(n1) }); }
R.rows = out;
'@
$pages = @()
$replay = @()
$bad = 0
foreach ($f in Get-ChildItem $Dir -Filter 'T*.json' | Sort-Object Name) {
  $cls = [IO.Path]::GetFileNameWithoutExtension($f.Name)
  $data = [IO.File]::ReadAllText($f.FullName, $utf8)
  $meta = $data | ConvertFrom-Json
  $mediaUrl = ([Uri]("$media\")).AbsoluteUri
  $dataJs = $data.Replace('</', '<\/')
  $html = @"
<!DOCTYPE html><html><head><meta charset="UTF-8"><link rel="stylesheet" href="${mediaUrl}props.css">
<script>window.__err = []; window.addEventListener('error', function (e) { window.__err.push(String(e.message) + ' @' + e.lineno); });
window.__out = []; window.acquireVsCodeApi = function () { return Object.freeze({ postMessage: function (m) { window.__out.push(m); }, getState: function () { return null; }, setState: function () {} }); };
</script></head><body><div id="root"></div><script src="${mediaUrl}props.js"></script>
<script>window.postMessage({ type: 'show', data: $dataJs }, '*');
setTimeout(function () { var R = { errors: window.__err }; try { $enum } catch (e) { R.err = String(e && e.stack || e); }
var pre = document.createElement('pre'); pre.id = 'HTDP'; pre.textContent = 'HTDP' + JSON.stringify(R) + 'HTDEND'; document.body.appendChild(pre); }, 700);
</script></body></html>
"@
  $hf = Join-Path $work "rows_$cls.html"
  [IO.File]::WriteAllText($hf, $html, $utf8)
  $of = Join-Path $work "rows_$cls.dom.txt"
  $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
    '--allow-file-access-from-files', '--virtual-time-budget=4000', '--dump-dom', ([Uri]$hf).AbsoluteUri)
  Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow -RedirectStandardOutput $of | Out-Null
  $t = [IO.File]::ReadAllText($of, $utf8)
  $m = [regex]::Match($t, '<pre id="HTDP">HTDP(\{.*?\})HTDEND</pre>', 'Singleline')
  if (-not $m.Success) { Write-Host "FAIL  $cls : no result"; $bad++; continue }
  $R = [System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value) | ConvertFrom-Json
  if ($R.errors.Count -or $R.err) { Write-Host "FAIL  $cls : JS error $($R.errors -join ' / ') $($R.err)"; $bad++ }
  $sendRows = @()
  foreach ($r in $R.rows) {
    $ms = @($r.msgs | Where-Object { $_ })
    if ($r.kind -eq 'readonly' -or $r.kind -eq 'select-one-choice') { Write-Host ("INFO  $cls · $($r.row) : $($r.kind)"); continue }
    $edits = @($ms | Where-Object { $_.type -match '^(set|rename|pickImage|align)' })
    if (-not $edits.Count) { Write-Host ("FAIL  $cls · $($r.row) ($($r.kind)) : edited, nothing sent " + (($ms | ForEach-Object { $_.type }) -join ',')); $bad++; continue }
    Write-Host ("PASS  $cls · $($r.row) ($($r.kind)) -> " + (($edits | ForEach-Object { $_.type + $(if ($_.prop) { ':' + $_.prop } else { '' }) }) -join ','))
    foreach ($e in $edits) { $sendRows += [ordered]@{ row = $r.row; msg = $e } }
  }
  $replay += [ordered]@{ cls = $cls; page = $meta.__page; id = $meta.__id; rows = $sendRows }
}
[IO.File]::WriteAllText((Join-Path $Dir 'replay.json'), ($replay | ConvertTo-Json -Depth 8), $utf8)
$n = ($replay | ForEach-Object { $_.rows.Count } | Measure-Object -Sum).Sum
Write-Host "replay.json: $($replay.Count) classes, $n edits; FAILS: $bad"
exit $(if ($bad) { 1 } else { 0 })
