# AI(W906-HTDESIGNER) 20260929: render the two panels (media/props.js, media/overview.js)
# in a headless Edge with REAL data written by smoke_extension.js
# (%TEMP%\htdesigner_panels\*.json), and check they draw without a JS error.
# Read-only; everything goes to %TEMP%. (ASCII only: Windows PowerShell 5.1 reads a
# BOM-less script in the ANSI code page, so the Chinese checks are \u escapes in JS.)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$media = Resolve-Path (Join-Path $here '..\media')
$dump = Join-Path $env:TEMP 'htdesigner_panels'
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $edge) { Write-Host 'msedge.exe not found'; exit 2 }
if (-not (Test-Path $dump)) { Write-Host "no data in $dump (run smoke_extension.js first)"; exit 2 }
$profileDir = Join-Path $dump 'edge-profile'
$utf8 = New-Object System.Text.UTF8Encoding($false)

function Render([string]$name, [string]$css, [string]$js, [string]$msgType, [string]$dataFile, [string]$checks) {
  # "</" inside the data (HTML snippets) must not end the <script> early
  $data = [IO.File]::ReadAllText($dataFile, $utf8).Replace('</', '<\/')
  $mediaUrl = ([Uri]("$media\")).AbsoluteUri
  $html = @"
<!DOCTYPE html><html><head><meta charset="UTF-8">
<link rel="stylesheet" href="$mediaUrl$css">
<script>
window.__err = [];
window.addEventListener('error', function (e) { window.__err.push(String(e.message) + ' @' + e.lineno); });
window.__out = [];
window.acquireVsCodeApi = function () { return { postMessage: function (m) { window.__out.push(m); }, getState: function () { return null; }, setState: function () {} }; };
</script></head><body><div id="root"></div>
<script src="$mediaUrl$js"></script>
<script>
var DATA = $data;
window.postMessage({ type: '$msgType', data: DATA }, '*');
setTimeout(function () {
  var R = { errors: window.__err, posted: window.__out.map(function (m) { return m.type; }) };
  try { $checks } catch (e) { R.checkError = String(e); }
  var pre = document.createElement('pre'); pre.id = 'HTDPANEL'; pre.textContent = 'HTDPANEL' + JSON.stringify(R) + 'HTDEND';
  document.body.appendChild(pre);
}, 600);
</script></body></html>
"@
  $file = Join-Path $dump ("render_$name.html")
  [IO.File]::WriteAllText($file, $html, $utf8)
  $out = Join-Path $dump ("render_$name.dom.txt")
  $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
    '--allow-file-access-from-files', '--virtual-time-budget=3000', '--dump-dom', ([Uri]$file).AbsoluteUri)
  Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow -RedirectStandardOutput $out | Out-Null
  $t = [IO.File]::ReadAllText($out, $utf8)
  $m = [regex]::Match($t, '<pre id="HTDPANEL">HTDPANEL(\{.*?\})HTDEND</pre>', 'Singleline')
  if (-not $m.Success) { Write-Host "[$name] no result"; return $null }
  return ([System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value) | ConvertFrom-Json)
}

$script:bad = 0
function Check([bool]$c, [string]$n, [string]$x) { if ($c) { Write-Host "PASS  $n   $x" } else { Write-Host "FAIL  $n   $x"; $script:bad++ } }

# section titles: events / DFM properties / commands to C++ / data field
$propsChecks = @'
/* WPF's Properties window: ONE list at a time -- the properties (the default), or the events (the lightning bolt) */
function view(v) { var b = document.querySelector('.views button[data-view="' + v + '"]'); if (b) b.click(); }
function secsNow() { return Array.prototype.map.call(document.querySelectorAll('details.sec > summary'), function (s) { return s.firstChild.textContent; }); }
var secs = secsNow();
R.secCount = secs.length;
R.propsView = !!document.querySelector('details.sec[data-key="props"], details.sec[data-key="edit"]') && !document.querySelector('details.sec[data-key="events"]') && !!document.querySelector('.views button.on[data-view="props"]');
R.hasProps = !!document.querySelector('details.sec[data-key="props"], details.sec[data-key="edit"]');
var txt = document.body.innerText;
R.propRows = document.querySelectorAll('table.kv tr').length;
var sysTxt = txt;
/* the properties list is only the properties; what the code does with it (listeners, commands, data field, uses) is
   the third list, 程式碼 */
var codeKeys = ['listeners', 'mentions', 'cmds', 'fields', 'tags', 'uses'];
R.propsNoCode = !codeKeys.some(function (k) { return !!document.querySelector('details.sec[data-key="' + k + '"]'); });
view('code');
var csecs = secsNow();
R.hasCmds = csecs.indexOf('送到 C++ 的命令') >= 0;
R.hasField = csecs.indexOf('資料欄位') >= 0;
var ctxt = document.body.innerText;
R.hasCmd = ctxt.indexOf('recipe.doc.put') >= 0;
R.hasXStart = ctxt.indexOf('[Hotplate Form] X Start') >= 0;
R.targets = document.querySelectorAll('.row.target').length;
/* a listener whose handler is written where it is bound: that line once, not twice */
R.oneLine = document.querySelectorAll('details.sec[data-key="listeners"] .row.target[data-oneline]').length;
R.lstTwice = Array.prototype.filter.call(document.querySelectorAll('details.sec[data-key="listeners"] .lst'), function (w) {
  var locs = Array.prototype.map.call(w.querySelectorAll(':scope > .row.target .loc'), function (x) { return x.textContent; });
  return locs.some(function (x, i) { return locs.indexOf(x) !== i; });
}).length;
R.codeView = !!document.querySelector('.views button.on[data-view="code"]') && !document.querySelector('details.sec[data-key="props"]') &&
  !document.querySelector('details.sec[data-key="events"]') && !document.querySelector('table.edit');
view('props');
view('events');
R.eventsView = !!document.querySelector('details.sec[data-key="events"]') && !document.querySelector('details.sec[data-key="props"]') && !!document.querySelector('.views button.on[data-view="events"]');
R.hasEvents = !!document.querySelector('details.sec[data-key="events"]');
R.hasHandler = Array.prototype.some.call(document.querySelectorAll('.row.evg'), function (r) { return r.title.indexOf('spbSaveClick') >= 0; });
/* the events table the WPF / Windows Forms way: two columns (the event | its handler), nothing under the rows;
   where it is wired is the row's tooltip (the BCB6 line of spbSaveClick among it) */
var evSec = document.querySelector('details.sec[data-key="events"]');
var evRows = evSec ? evSec.querySelectorAll('.row.evg') : [];
R.evRows = evRows.length;
R.evTwoCols = Array.prototype.every.call(evRows, function (er) {
  var kids = Array.prototype.filter.call(er.children, function (k) { return k.tagName !== 'DATALIST'; });
  return kids.length === 2 && kids[0].classList.contains('evn') && kids[1].tagName === 'INPUT';
});
R.evNoRows = evSec ? evSec.querySelectorAll('.row.target, details.more, .chip').length : -1;
/* A-Z, both lists (WPF's Events tab) */
var azOf = function (sel, attr) { return Array.prototype.map.call(document.querySelectorAll(sel), function (x) { return x.getAttribute(attr); }); };
var azSorted = function (a) { return a.join('|') === a.slice().sort(function (x, y) { return x.localeCompare(y, 'en', { sensitivity: 'base' }); }).join('|'); };
var evAZ = azOf('details.sec[data-key="events"] .row.evg', 'data-event'), jsAZ = azOf('details.sec[data-key="jsevents"] .row.jsev', 'data-jsevent');
R.evAZ = evAZ.length >= 5 && azSorted(evAZ) && (jsAZ.length === 0 || azSorted(jsAZ));
R.evAZlist = evAZ.join(',') + ' | ' + jsAZ.join(',');
var evClick = evSec && evSec.querySelector('.row.evg[data-event="OnClick"]');
var evUp = evSec && evSec.querySelector('.row.evg[data-event="OnMouseUp"]');
R.evClickTip = evClick ? evClick.title : '';
R.evUpEmpty = !!(evUp && evUp.querySelector('input').value === '' && evUp.querySelector('input').placeholder === '');
R.evClickRo = !!(evClick && evClick.querySelector('input').readOnly);
R.hasGolden440 = document.body.textContent.indexOf('cHotPlate.cpp:440') >= 0 || R.evClickTip.indexOf('cHotPlate.cpp:440') >= 0;
/* WPF: blank + Enter in an empty one = the default name (posts eventGrid); a typed name + Enter = that name, once */
var n0 = window.__out.length;
if (evUp) { var ui = evUp.querySelector('input'); ui.focus(); ui.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); }
R.evBlankEnter = window.__out.slice(n0).map(function (m) { return m.type + ':' + (m.name || m.event || ''); }).join(',');
var evDown = evSec && evSec.querySelector('.row.evg[data-event="OnMouseDown"]');
var n1 = window.__out.length;
if (evDown) {
  var di = evDown.querySelector('input'); di.focus(); di.value = 'spbSaveDown';
  di.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
  di.dispatchEvent(new Event('change', { bubbles: true })); di.blur();
}
R.evTyped = window.__out.slice(n1).map(function (m) { return m.type + ':' + m.event + '=' + m.value; }).join(',');
/* right-click: jump to the code / all the code found / Reset (off for the .dfm's) */
if (evClick) evClick.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 20, clientY: 20 }));
var cm = document.querySelector('.ctxmenu');
R.evMenu = cm ? Array.prototype.map.call(cm.querySelectorAll('.ctxitem'), function (x) { return x.textContent + (x.classList.contains('off') ? '(off)' : ''); }).join('|') : '';
var allItem = cm && cm.querySelectorAll('.ctxitem')[1];
if (allItem) allItem.click();
R.evMenuAll = (window.__out[window.__out.length - 1] || {}).type;
R.evMenuClosed = !document.querySelector('.ctxmenu');
/* Tab goes field to field (WPF's Events tab): the rows are not Tab stops, their fields are */
R.evRowNoTab = document.querySelectorAll('.row.evg').length > 0 && Array.prototype.every.call(document.querySelectorAll('.row.evg'), function (r) { return r.tabIndex === -1; }) &&
  Array.prototype.every.call(document.querySelectorAll('.row.evg input'), function (x) { return x.tabIndex >= 0; });
/* a page event's name + Enter: sent once, and you stay in the field */
var jsC = document.querySelector('input[data-field="js:click"]');
var nj = window.__out.length;
if (jsC) {
  jsC.focus(); jsC.value = 'spbSaveClickJs';
  jsC.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
  R.jsStays = document.activeElement === jsC && jsC.selectionStart === 0 && jsC.selectionEnd === jsC.value.length;
  jsC.dispatchEvent(new Event('change', { bubbles: true }));
  jsC.blur();
}
R.jsSent = window.__out.slice(nj).map(function (m) { return m.type + ':' + m.ev + '=' + m.fn; }).join(',');
var ev = document.querySelector('.row.event');
if (ev) ev.dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));
R.afterDbl = window.__out.map(function (m) { return m.type; });
view('props');
R.backToProps = !document.querySelector('details.sec[data-key="events"]') && !!document.querySelector('input[data-field="left"]');
R.hasEdit = secs.indexOf('\u5c6c\u6027') >= 0;
var left = document.querySelector('input[data-field="left"]');
var cap = document.querySelector('input[data-field="caption"]');
R.leftVal = left ? left.value : null;
R.capVal = cap ? cap.value : null;
if (left) { left.value = '77'; left.dispatchEvent(new Event('change')); }
if (cap) { cap.value = 'OK'; cap.dispatchEvent(new Event('change')); }
R.afterEdit = window.__out.slice(-2).map(function (m) { return m.type + ':' + (m.left != null ? m.left : m.value); });
/* Up / Down in a number field: +1 / -1 (Shift = 10) at once; ONE edit when the keys stop / on leaving it */
var topIn = document.querySelector('input[data-field="top"]');
var nTop = window.__out.length;
if (topIn) {
  topIn.focus();
  ['ArrowUp', 'ArrowUp', 'ArrowUp'].forEach(function (k) { topIn.dispatchEvent(new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true })); });
  topIn.dispatchEvent(new KeyboardEvent('keydown', { key: 'ArrowUp', shiftKey: true, bubbles: true, cancelable: true }));
  topIn.dispatchEvent(new KeyboardEvent('keydown', { key: 'ArrowDown', bubbles: true, cancelable: true }));
  R.topTyped = topIn.value;
  R.topBefore = window.__out.length - nTop;
  topIn.blur();
}
R.topSent = window.__out.slice(nTop).filter(function (m) { return m.type === 'setLayout'; }).map(function (m) { return m.top; }).join(',');
/* Esc = never mind (WPF): a number stepped twice, the caption typed over -> back to what they were, nothing sent;
   the field being edited marks its whole row */
var wIn = document.querySelector('input[data-field="width"]'), cIn = document.querySelector('input[data-field="caption"]');
var nEsc = window.__out.length, w0 = wIn ? wIn.value : null, c0 = cIn ? cIn.value : null;
if (wIn) {
  wIn.focus();
  R.rowMarked = getComputedStyle(wIn.closest('td')).backgroundColor !== getComputedStyle(document.querySelector('input[data-field="height"]').closest('td')).backgroundColor;
  ['ArrowUp', 'ArrowUp'].forEach(function (k) { wIn.dispatchEvent(new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true })); });
  wIn.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true, cancelable: true }));
}
if (cIn) {
  cIn.focus(); cIn.value = 'XX';
  cIn.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true, cancelable: true }));
}
R.escBack = (wIn ? wIn.value === w0 : false) && (cIn ? cIn.value === c0 : false);
R.escSent = window.__out.slice(nEsc).filter(function (m) { return m.type === 'setLayout' || m.type === 'setCaption'; }).length;
/* a double-click on a property's name = to where it is written in the HTML (revealProp) */
var rvK = Array.prototype.filter.call(document.querySelectorAll('table.edit td.k'), function (t) { return t.textContent === 'Top'; })[0];
if (rvK) rvK.dispatchEvent(new MouseEvent('dblclick', { bubbles: true, cancelable: true }));
var rvM = window.__out[window.__out.length - 1] || {};
R.revealProp = rvM.type + ':' + rvM.prop;
R.revealTip = rvK ? rvK.title : '';
/* compare with DFM: the marked rows, the DFM hints, the reset button */
R.diffRows = Array.prototype.map.call(document.querySelectorAll('table.edit tr.diff td.k'), function (t) { return t.textContent; });
R.dfmHints = document.querySelectorAll('table.edit .dfmv').length;
/* (a note not compared is the row's ⓘ tooltip now -- nothing under the value, WPF-clear) */
R.sysHint = Array.prototype.some.call(document.querySelectorAll('table.edit .tip'), function (t) { return t.title.indexOf('DFM clWindowText（系統色）') >= 0; });
R.gridCols = Array.prototype.every.call(document.querySelectorAll('table.edit tr:not(.cathead)'), function (tr) { return tr.children.length === 3 && tr.children[2].classList.contains('m'); }) &&
  document.querySelectorAll('table.edit td.m button.reset.marker').length === document.querySelectorAll('table.edit button.reset.marker').length;
R.dfmOnlyDiff = Array.prototype.every.call(document.querySelectorAll('table.edit tr:not(.diff) .dfmv'), function (d) { return getComputedStyle(d).display === 'none'; }) &&
  Array.prototype.every.call(document.querySelectorAll('table.edit tr.diff .dfmv'), function (d) { return getComputedStyle(d).display !== 'none'; });
R.palClosed = Array.prototype.every.call(document.querySelectorAll('table.edit .pal'), function (p) { return getComputedStyle(p).display === 'none'; });
var ptg = document.querySelector('button.paltog[data-paltog="color"]');
if (ptg) ptg.click();
R.palOpens = !!document.querySelector('.pal.open[data-for="color"]') && getComputedStyle(document.querySelector('.pal[data-for="color"]')).display !== 'none';
if (ptg) ptg.click();
R.diffNote = !!document.querySelector('.diffnote');
var fsRow = null, fzRow = null;
Array.prototype.forEach.call(document.querySelectorAll('table.edit tr'), function (tr) {
  if (tr.firstChild.textContent === 'Font.Bold') fsRow = tr;
  if (tr.firstChild.textContent === 'Font.Size') fzRow = tr;
});
R.sizeHint = fzRow ? (fzRow.querySelector('.dfmv') || {}).textContent : null;
/* WPF's FontFamily drop-down: Font.Name has a list (the DFM's font, the page's, the usual ones) */
var fnIn = document.querySelector('input[data-field="fontName"]');
var fnList = fnIn && fnIn.getAttribute('list') ? document.getElementById(fnIn.getAttribute('list')) : null;
R.fontList = fnList ? Array.prototype.map.call(fnList.querySelectorAll('option'), function (o) { return o.value; }) : [];
/* WPF's property marker: one per compared row, filled on the changed ones; a click = its menu, Reset in it */
R.markers = document.querySelectorAll('table.edit button.reset.marker').length;
R.markersSet = document.querySelectorAll('table.edit button.reset.marker.set').length;
var rb = fsRow && fsRow.querySelector('button.reset');
var nrb = window.__out.length;
if (rb) rb.click();
R.markerNoPostYet = window.__out.length === nrb;
var rmi = document.querySelector('.ctxmenu .ctxitem');
if (rmi) rmi.click();
var lastMsg = window.__out[window.__out.length - 1] || {};
R.afterReset = rb && rmi ? lastMsg.type + ':' + lastMsg.prop + '=' + lastMsg.value : 'no reset button / menu';
/* a marker on EVERY row (WPF: one right of each value) -- the ones not compared fainter; its menu: Reset + to the HTML;
   a right-click on the row opens the same menu */
var edRowsAll = Array.prototype.filter.call(document.querySelectorAll('table.edit tr'), function (tr) { return !tr.classList.contains('cathead'); });
R.markerEvery = edRowsAll.length > 0 && edRowsAll.every(function (tr) { return !!tr.querySelector('td.m button.reset.marker'); });
R.markerNone = document.querySelectorAll('table.edit button.reset.marker.none').length;
var topRow = edRowsAll.filter(function (tr) { return tr.firstChild.textContent === 'Top'; })[0];
var tmk = topRow && topRow.querySelector('button.reset.marker');
if (tmk) tmk.click();
R.topMenu = Array.prototype.map.call(document.querySelectorAll('.ctxmenu .ctxitem'), function (x) { return x.textContent + (x.classList.contains('off') ? '(off)' : ''); }).join('|');
var tjump = document.querySelectorAll('.ctxmenu .ctxitem')[1];
if (tjump) tjump.click();
var tjm = window.__out[window.__out.length - 1] || {};
R.topJump = tjump ? tjm.type + ':' + tjm.prop : 'no jump item';
/* (and the .dfm line BCB6 wrote it on: Top = line 353 of cHotPlate.dfm) */
if (tmk) tmk.click();
var tdfm = document.querySelectorAll('.ctxmenu .ctxitem')[2];
if (tdfm) tdfm.click();
var tdm = window.__out[window.__out.length - 1] || {};
R.topDfm = tdfm ? tdm.type + ':' + tdm.line : 'no dfm item';
/* WPF: one list -- what the grid shows is not repeated in the DFM list below it (only the rest, read only, open) */
/* 20261001: ONE table -- the .dfm's other properties are greyed read-only rows of the grid itself, in the same
   categories (each category once), no second DFM list below */
var dfmKeys = Array.prototype.map.call(document.querySelectorAll('table.edit tr.ro td.k'), function (t) { return t.textContent; });
var gridKeys = Array.prototype.map.call(document.querySelectorAll('table.edit tr:not(.ro) td.k'), function (t) { return t.textContent; });
R.dfmRest = dfmKeys.join(',');
R.dfmNoRepeat = dfmKeys.length > 0 && dfmKeys.every(function (k) { return gridKeys.indexOf(k) < 0 && !(gridKeys.length && (k === 'Font.Height' || k === 'Font.Style')); });
R.dfmOpen = !document.querySelector('details.sec[data-key="props"]') && dfmKeys.length > 0;
R.catOnce = (function () {
  var cs = Array.prototype.map.call(document.querySelectorAll('table.edit tr.cathead'), function (tr) { return tr.getAttribute('data-cat'); });
  return cs.length > 1 && cs.every(function (x, i) { return cs.indexOf(x) === i; });
})();
R.roMenu = (function () {
  var mk = document.querySelector('table.edit tr.ro button.marker');
  if (!mk) return '';
  mk.click();
  return Array.prototype.map.call(document.querySelectorAll('.ctxmenu .ctxitem'), function (x) { return x.classList.contains('off') ? 'off' : 'on'; }).join(',');
})();
document.body.click();
var wRow = edRowsAll.filter(function (tr) { return tr.firstChild.textContent === 'Width'; })[0];
var nctx = window.__out.length;
if (wRow) wRow.firstChild.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 30, clientY: 30 }));
R.ctxMenu = document.querySelectorAll('.ctxmenu .ctxitem').length;
var wjump = document.querySelectorAll('.ctxmenu .ctxitem')[1];
if (wjump) wjump.click();
var wjm = window.__out[window.__out.length - 1] || {};
R.ctxJump = wjump && window.__out.length === nctx + 1 ? wjm.type + ':' + wjm.prop : 'no menu / posted ' + (window.__out.length - nctx);
/* (a right-click in a text field: its own menu, not the marker's) */
var capIn2 = document.querySelector('input[data-field="caption"]');
if (capIn2) capIn2.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 30, clientY: 30 }));
R.ctxInText = document.querySelectorAll('.ctxmenu').length;
/* the 16 standard colours under Font.Color / Color: a click sets that colour */
R.swatches = document.querySelectorAll('.pal .sw').length;
var red = document.querySelector('.pal[data-for="color"] .sw[data-color="#ff0000"]');
if (red) red.click();
var lastC = window.__out[window.__out.length - 1] || {};
R.afterSwatch = red ? lastC.type + ':' + lastC.prop + '=' + lastC.value : 'no swatch';
R.swatchTitle = red ? red.title : '';
/* WPF's Name box at the top: a new name + Enter posts rename with it (once) */
var nbx = document.querySelector('.title input[data-field="name"]');
R.nameBox = nbx ? nbx.value : null;
var nr0 = window.__out.length;
if (nbx) {
  nbx.focus(); nbx.value = 'btnSave';
  nbx.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
  nbx.dispatchEvent(new KeyboardEvent('keyup', { key: 'Enter', bubbles: true }));
  nbx.dispatchEvent(new Event('change', { bubbles: true }));
}
R.afterRename = window.__out.slice(nr0).map(function (m) { return m.type + ':' + m.name; }).join(',');
/* WPF's tag navigator: the parents in the header, a click selects that one */
var crs = document.querySelectorAll('.crumbs .crumb');
R.crumbs = Array.prototype.map.call(crs, function (x) { return x.textContent; });
R.crumbHere = (document.querySelector('.crumbs .here') || {}).textContent || '';
if (crs.length) crs[crs.length - 1].click();
var lastCr = window.__out[window.__out.length - 1] || {};
R.afterCrumb = crs.length ? lastCr.type + ':' + lastCr.id : 'no crumbs';
/* the diff note's "reset all of this one" */
var rall = document.querySelector('.diffnote button[data-act="resetToDfm"]');
if (rall) rall.click();
R.afterResetAll = rall ? (window.__out[window.__out.length - 1] || {}).type : 'no button';
/* Tab walks the values (WPF's grid): nothing else in the grid is a Tab stop -- the marker, the colour well, its arrow are clicks */
var tabbable = Array.prototype.filter.call(document.querySelectorAll('table.edit tr:not(.cathead) input, table.edit tr:not(.cathead) button, table.edit tr:not(.cathead) select'),
  function (x) { return x.tabIndex >= 0 && !x.disabled && x.offsetParent !== null; });
R.tabOnlyValues = tabbable.length >= 10 && tabbable.every(function (x) { return (x.tagName === 'INPUT' || x.tagName === 'SELECT') && x.hasAttribute('data-field') && x.type !== 'color'; });
R.tabList = tabbable.map(function (x) { return x.getAttribute('data-field') || x.tagName + '.' + x.className; }).join(',');
/* Enter = written once, and you stay in the field (a text one with its value selected): type the next value, or Tab on */
function enterStays(field, v) {
  var i = document.querySelector('table.edit input[data-field="' + field + '"]');
  if (!i) return 'no ' + field;
  var n = window.__out.length;
  i.focus(); i.value = v;
  i.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
  var stays = document.activeElement === i;
  var sel = i.type === 'number' || (i.selectionStart === 0 && i.selectionEnd === i.value.length);
  i.dispatchEvent(new Event('change', { bubbles: true }));
  i.blur();
  return (stays ? 'stays' : 'left') + (sel ? '+sel' : '') + ' ' + window.__out.slice(n).map(function (m) { return m.type + ':' + (m.left != null ? m.left : m.value); }).join(',');
}
R.enterLeft = enterStays('left', '61');
R.enterCap = enterStays('caption', 'Go');
R.enterFont = enterStays('fontName', 'Tahoma');
'@
$r1 = Render 'props_spbSave' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $propsChecks
if ($r1) {
  Check ($r1.errors.Count -eq 0 -and -not $r1.checkError) 'props panel (spbSave) renders without a JS error' (($r1.errors -join ' / ') + $r1.checkError)
  Check ($r1.hasEvents -and $r1.hasProps -and $r1.hasCmds) 'props panel sections: events, DFM properties, commands' "sections=$($r1.secCount)"
  Check ($r1.propsNoCode -and $r1.codeView) 'three lists (WPF-clear): the properties only properties; listeners / commands / data field / uses in the third one, </> code' "noCode=$($r1.propsNoCode) code=$($r1.codeView)"
  Check ($r1.propsView -and $r1.eventsView -and $r1.backToProps) 'WPF Properties window: the properties list by default; the lightning-bolt button = the events list only; back' "props=$($r1.propsView) events=$($r1.eventsView) back=$($r1.backToProps)"
  Check ($r1.hasHandler -and $r1.hasGolden440 -and $r1.hasCmd) 'props panel shows spbSaveClick, cHotPlate.cpp:440 and recipe.doc.put' ''
  Check ($r1.oneLine -ge 1 -and $r1.lstTwice -eq 0) 'code list: a listener bound and handled on the same line shows that line ONCE (not as the handler and again as the binding)' "oneLine=$($r1.oneLine) twice=$($r1.lstTwice)"
  Check ($r1.targets -ge 5 -and $r1.propRows -ge 10) 'props panel rows' "targets=$($r1.targets) propRows=$($r1.propRows)"
  Check (($r1.afterDbl -join ',') -match 'eventGrid') 'double-click on an event row (the events table) posts eventGrid' ($r1.afterDbl -join ',')
  Check ($r1.evRows -ge 5 -and $r1.evTwoCols -and $r1.evNoRows -eq 0 -and $r1.evUpEmpty -and $r1.evClickRo -and $r1.evClickTip -match 'cHotPlate\.cpp:440') 'events table like WPF: two columns only, nothing under the rows, an empty one really empty, the .dfm one read only, where it is wired in its tooltip' "rows=$($r1.evRows) twoCols=$($r1.evTwoCols) under=$($r1.evNoRows) empty=$($r1.evUpEmpty)"
  Check ($r1.evAZ) 'the event lists A-Z (WPF Events tab): the component''s and the page''s' $r1.evAZlist
  Check ($r1.evBlankEnter -eq 'eventGrid:OnMouseUp' -and $r1.evTyped -eq 'eventName:OnMouseDown=spbSaveDown') 'WPF: blank + Enter = the default handler (eventGrid); a typed name + Enter = that name, sent once' "$($r1.evBlankEnter) | $($r1.evTyped)"
  Check ($r1.evMenu -match '^[^|]+\|[^|]+\|[^|]+\(off\)$' -and $r1.evMenuAll -eq 'eventAll' -and $r1.evMenuClosed) 'right-click an event: jump / all the code found / Reset (off for the .dfm one); an item posts and the menu closes' "$($r1.evMenu) -> $($r1.evMenuAll)"
  Check ($r1.hasEdit -and $r1.leftVal -eq '54' -and $r1.capVal -eq 'Save') 'editable section: Left = 54, Caption = Save' "left=$($r1.leftVal) caption=$($r1.capVal)"
  Check (($r1.afterEdit -join ',') -eq 'setLayout:77,setCaption:OK') 'changing Left / Caption posts setLayout / setCaption' ($r1.afterEdit -join ',')
  Check ($r1.revealProp -eq 'revealProp:Top' -and $r1.revealTip -match 'HTML') 'a double-click on a property name posts revealProp (to the HTML source), its tooltip says so' $r1.revealProp
  Check ($r1.topTyped -eq '20' -and $r1.topBefore -eq 0 -and $r1.topSent -eq '20') 'Up / Down in a number field (Blend): 8 +1 +1 +1 +10 -1 = 20 at once, ONE setLayout when leaving it' "typed=$($r1.topTyped) before=$($r1.topBefore) sent=$($r1.topSent)"
  Check ($r1.escBack -and $r1.escSent -eq 0 -and $r1.rowMarked) 'Esc = never mind (WPF): the stepped Width and the typed-over Caption back, nothing sent; the edited row marked' "back=$($r1.escBack) sent=$($r1.escSent) marked=$($r1.rowMarked)"
  # the page's save style is 14px bold; the DFM says Font.Height = -16 (= 14px on a generated page), no fsBold
  Check ((($r1.diffRows | Sort-Object) -join ',') -eq 'Font.Bold' -and $r1.diffNote) 'compare with DFM: exactly Font.Bold marked (Font.Size 14 = |-16|-2)' ($r1.diffRows -join ',')
  Check ($r1.dfmHints -ge 8 -and $r1.sysHint -and $r1.sizeHint -match '14.*-16') 'DFM hints on the rows, system colour by name, Font.Size shows the Height' "hints=$($r1.dfmHints) sys=$($r1.sysHint) size=$($r1.sizeHint)"
  Check ($r1.gridCols -and $r1.dfmOnlyDiff -and $r1.palClosed -and $r1.palOpens) 'WPF-clear grid: name | value | marker on every row; the DFM value shown only where it differs; the 16 colours behind the value''s arrow' "cols=$($r1.gridCols) dfm=$($r1.dfmOnlyDiff) pal=$($r1.palClosed)/$($r1.palOpens)"
  Check (@($r1.fontList).Count -ge 10 -and (@($r1.fontList) -contains 'MS Sans Serif') -and (@($r1.fontList) -contains 'Arial')) 'Font.Name drop-down (WPF FontFamily): the DFM / page fonts and the usual ones, no repeats' "$(@($r1.fontList).Count): $((@($r1.fontList) | Select-Object -First 4) -join ', ')"
  Check ($r1.afterReset -eq 'setLook:bold=false' -and $r1.markerNoPostYet -and $r1.markers -ge 8 -and $r1.markersSet -eq @($r1.diffRows).Count) 'WPF property marker: one per compared row, filled = the changed ones (Font.Bold); a click opens its menu, Reset posts the DFM value' "$($r1.afterReset) markers=$($r1.markers) set=$($r1.markersSet)"
  Check ($r1.markerEvery -and $r1.markerNone -ge 1 -and $r1.topMenu -match '^[^|]+\(off\)\|[^|]*HTML[^|]*\|[^|(]*\.dfm[^|(]*$' -and $r1.topJump -eq 'revealProp:Top' -and $r1.topDfm -eq 'openDfmLine:353' -and $r1.ctxMenu -eq 3 -and $r1.ctxJump -eq 'revealProp:Width' -and $r1.ctxInText -eq 0) 'a marker on every row (WPF), the ones not compared fainter; its menu = Reset (off when the same) + to the HTML source + to its .dfm line; a right-click on the row = the same menu, not in a text field' "every=$($r1.markerEvery) none=$($r1.markerNone) menu=$($r1.topMenu) jump=$($r1.topJump) dfm=$($r1.topDfm) ctx=$($r1.ctxMenu)/$($r1.ctxJump) inText=$($r1.ctxInText)"
  Check ($r1.dfmNoRepeat -and ($r1.dfmRest -split ',') -contains 'ParentFont' -and ($r1.dfmRest -split ',') -contains 'Font.Charset' -and $r1.dfmOpen -and $r1.catOnce -and $r1.roMenu -match '^(on|off),on$') 'WPF: ONE list -- the .dfm''s other properties (Font.Charset, ParentFont ...) are greyed rows of the grid, each category once, no second list; their marker: to the .dfm / copy' "$($r1.dfmRest) one=$($r1.dfmOpen) catOnce=$($r1.catOnce) menu=$($r1.roMenu)"
  Check ($r1.nameBox -eq 'spbSave' -and $r1.afterRename -eq 'rename:btnSave') 'WPF Name box at the top: spbSave; a new name + Enter posts rename once' "$($r1.nameBox) -> $($r1.afterRename)"
  Check (@($r1.crumbs).Count -ge 1 -and $r1.crumbHere -eq 'spbSave' -and $r1.afterCrumb -eq ('selectId:' + @($r1.crumbs)[-1])) 'WPF tag navigator: the parents of spbSave in the header, a click on one posts selectId for it' "$(@($r1.crumbs) -join ' > ') > $($r1.crumbHere) -> $($r1.afterCrumb)"
  Check ($r1.afterResetAll -eq 'resetToDfm') 'the difference note: "reset all of this one" posts resetToDfm' $r1.afterResetAll
  Check ($r1.tabOnlyValues -and $r1.evRowNoTab) 'Tab walks the values (WPF): in the grid only the value fields are Tab stops (not the marker / colour well / its arrow); in the events only the fields' "$($r1.tabList) | evRows=$($r1.evRowNoTab)"
  Check ($r1.enterLeft -eq 'stays+sel setLayout:61' -and $r1.enterCap -eq 'stays+sel setCaption:Go' -and $r1.enterFont -eq 'stays+sel setLook:Tahoma' -and $r1.jsStays -and $r1.jsSent -eq 'jsEvent:click=spbSaveClickJs') 'Enter = written ONCE and you stay in the field, its value selected (WPF): Left, Caption, Font.Name, a page event' "$($r1.enterLeft) | $($r1.enterCap) | $($r1.enterFont) | js=$($r1.jsStays) $($r1.jsSent)"
  Check ($r1.swatches -eq 32 -and $r1.afterSwatch -eq 'setLook:color=#ff0000' -and $r1.swatchTitle -match '^clRed') '16 standard colours under Font.Color and Color; clRed posts color #ff0000' "swatches=$($r1.swatches) $($r1.afterSwatch)"
} else { $script:bad++ }
$searchChecks = @'
function vis(e) { return e && e.offsetParent !== null; }
function view(v) { var b = document.querySelector('.views button[data-view="' + v + '"]'); if (b) b.click(); }
function run(q) {
  /* (the box is drawn again when the view changes) */
  var sq = document.querySelector('input[data-field="search"]');
  sq.value = q; sq.dispatchEvent(new Event('input'));
  var secs = Array.prototype.filter.call(document.querySelectorAll('details.sec'), function (s) { return s.style.display !== 'none'; });
  var rows = Array.prototype.filter.call(document.querySelectorAll('details.sec tr, details.sec .row'), function (r) { return r.style.display !== 'none' && vis(r); });
  /* (a row's text: what it shows, its input's value, an event row's tooltip) */
  var said = function (r) { var i = r.querySelector('input'); return (r.textContent + (i ? ' ' + i.value : '') + (r.classList.contains('evg') ? ' ' + r.title : '')).slice(0, 200); };
  return { secs: secs.length, rows: rows.length, text: rows.map(said).join(' | '), count: document.querySelector('.shits').textContent };
}
/* a handler name: in the events list (WPF searches the list it shows) */
view('events');
R.allE = document.querySelectorAll('details.sec').length;
R.q1 = run('spbSaveClick');
R.q1.all = R.allE;
view('props');
R.all = document.querySelectorAll('details.sec').length;
R.q2 = run('font.size');
R.q3 = run('left 54');
R.q4 = run('zzzz-nothing');
R.q0 = run('');
R.q0secs = Array.prototype.filter.call(document.querySelectorAll('details.sec'), function (s) { return s.style.display !== 'none'; }).length;
'@
# Alias (an IO page's field): ONE Enter commits it -- the suggestion list may take the key press, so the key's
# release commits too; the same value is never sent twice
$aliasFile = Join-Path $dump 'props_alias.json'
$aj = [IO.File]::ReadAllText((Join-Path $dump 'props_spbSave.json'), $utf8).Replace('"aliasOn":false,"alias":"","ioAliases":null', '"aliasOn":true,"alias":"C_A","ioAliases":{"file":"x","names":["C_A","C_B"]}')
[IO.File]::WriteAllText($aliasFile, $aj, $utf8)
$aliasChecks = @"
var inp = document.querySelector('input[data-field="alias"]');
R.hasAlias = !!inp && inp.value === 'C_A' && !!document.getElementById('ioAliasList');
if (inp) {
  inp.focus(); inp.value = 'C_B';
  inp.dispatchEvent(new Event('input', { bubbles: true }));
  inp.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
  inp.dispatchEvent(new KeyboardEvent('keyup', { key: 'Enter', bubbles: true }));
  inp.dispatchEvent(new Event('change', { bubbles: true }));
  inp.blur();
}
R.aliasSent = window.__out.filter(function (m) { return m.type === 'setAlias'; }).map(function (m) { return m.value; });
"@
$r7 = Render 'props_alias' 'props.css' 'props.js' 'show' $aliasFile $aliasChecks
if ($r7) {
  Check ($r7.errors.Count -eq 0 -and $r7.hasAlias) 'Alias row: the IO page field, with the IO names to pick from' ''
  Check ((@($r7.aliasSent) -join ',') -eq 'C_B') 'Alias: ONE Enter sends it, once (key release / change / leaving the field do not send it again)' (@($r7.aliasSent) -join ',')
}
# a page event with a function: right-click = the component events' menu -- jump to the code / Reset (takes the binding out)
$jsFile = Join-Path $dump 'props_jsev.json'
$jj = [IO.File]::ReadAllText((Join-Path $dump 'props_spbSave.json'), $utf8)
if ($jj.IndexOf('"keyup","focus","blur"],"handlers":{}') -lt 0) { Write-Host 'FAIL  (the jsEvents data changed: fix the test)'; $script:bad++ }
$jj = $jj.Replace('"keyup","focus","blur"],"handlers":{}', '"keyup","focus","blur"],"handlers":{"click":{"fn":"spbSaveClickJs"}}')
[IO.File]::WriteAllText($jsFile, $jj, $utf8)
$jsChecks = @'
var vb = document.querySelector('.views button[data-view="events"]');
if (vb) vb.click();
/* 20261001: the Events button = the default event's value focused (WPF: "the default event is selected") */
R.defFocus = document.activeElement && document.activeElement.getAttribute('data-field');
function ctx(ty) {
  var r = document.querySelector('.row.jsev[data-jsevent="' + ty + '"]');
  if (!r) return null;
  r.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 20, clientY: 20 }));
  return Array.prototype.map.call(document.querySelectorAll('.ctxmenu .ctxitem'), function (x) { return x; });
}
var it = ctx('click') || [];
R.jsMenu = it.map(function (x) { return x.textContent + (x.classList.contains('off') ? '(off)' : ''); }).join('|');
var n0 = window.__out.length;
if (it[0]) it[0].click();
R.jsJump = window.__out.slice(n0).map(function (m) { return m.type + ':' + m.ev; }).join(',');
it = ctx('click') || [];
var n1 = window.__out.length;
if (it[1]) it[1].click();
R.jsReset = window.__out.slice(n1).map(function (m) { return m.type + ':' + m.ev + '=' + m.fn; }).join(',');
R.jsResetBox = (document.querySelector('input[data-field="js:click"]') || {}).value;
var ib = ctx('blur') || [];
R.jsEmptyMenu = ib.map(function (x) { return x.classList.contains('off') ? 'off' : 'on'; }).join(',');
/* 20261001: a page event left blank + Enter / a double click on an empty one = the default name (as the C++ table) */
var sentOf = function (n) { return window.__out.slice(n).filter(function (m) { return m.type === 'jsEvent'; }).map(function (m) { return m.ev + '=' + m.fn; }).join(','); };
var mb = document.querySelector('input[data-field="js:mousedown"]'), n3 = window.__out.length;
if (mb) { mb.focus(); mb.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); }
R.jsEnterDef = sentOf(n3);
var mu = document.querySelector('input[data-field="js:mouseup"]'), n4 = window.__out.length;
if (mu) mu.dispatchEvent(new MouseEvent('dblclick', { bubbles: true, cancelable: true }));
R.jsDblDef = sentOf(n4);
'@
$rJs = Render 'props_jsev' 'props.css' 'props.js' 'show' $jsFile $jsChecks
if ($rJs) {
  Check ($rJs.errors.Count -eq 0 -and $rJs.jsMenu -match '^[^|(]+\|[^|(]+$' -and $rJs.jsJump -eq 'openJsEvent:click' -and $rJs.jsReset -eq 'jsEvent:click=' -and $rJs.jsResetBox -eq '' -and $rJs.jsEmptyMenu -eq 'off,off') 'right-click a page event (as a component event): jump to the code / Reset takes the binding out (sent once, the box empty); both off on an empty one' "$($rJs.jsMenu) -> $($rJs.jsJump) / $($rJs.jsReset) / empty=$($rJs.jsEmptyMenu)"
  Check ($rJs.defFocus -eq 'event:OnClick') 'the Events button: the default event (OnClick) has the focus (WPF)' "$($rJs.defFocus)"
  Check ($rJs.jsEnterDef -eq 'mousedown=spbSaveMousedown' -and $rJs.jsDblDef -eq 'mouseup=spbSaveMouseup') 'a page event: blank + Enter / a double click on an empty one = the default name' "$($rJs.jsEnterDef) / $($rJs.jsDblDef)"
} else { $script:bad++ }
# 20261001 -- several selected (BCB6's Object Inspector / Windows Forms): the events they all have; handlers that
# differ = the value empty, placeholder (different); a .dfm one among them = read only; blank + Enter = the default
# for all, a typed name = that one for all; the menu's Reset says it is all of them, its jump is off
$mvChecks = @'
var S = function () { return String.fromCharCode.apply(null, arguments); };
var d2 = JSON.parse(JSON.stringify(DATA));
d2.edit.multi = ['sbtExit'];
d2.eventGrid.multi = ['sbtExit'];
d2.eventGrid.rows.forEach(function (r) {
  if (r.name === 'OnMouseUp') { r.handler = 'spbSaveUp'; r.via = 'htd'; r.mixed = true; }
  if (r.name === 'OnClick') { r.multiRo = true; r.mixed = true; }
});
window.dispatchEvent(new MessageEvent('message', { data: { type: 'show', data: d2 } }));
var vb = document.querySelector('.views button[data-view="events"]');
if (vb) vb.click();
var up = document.querySelector('input[data-field="event:OnMouseUp"]');
var upRow = up ? up.closest('.row') : null;
R.mvUp = up ? [up.value, up.placeholder === S(0xff08, 0x4e0d, 0x540c, 0xff09), upRow.classList.contains('mixed'), upRow.classList.contains('set')].join('|') : null;
var sec = document.querySelector('details.sec[data-key="events"]');
R.mvTitle = !!sec && sec.textContent.indexOf('2 ' + S(0x500b, 0x5171, 0x6709)) >= 0 && !!sec.querySelector('.multinote');
var ck = document.querySelector('input[data-field="event:OnClick"]');
R.mvClickRo = ck ? (ck.readOnly && ck.value === '') : null;
var dbl = document.querySelector('input[data-field="event:OnDblClick"]');
R.mvDblRo = dbl ? dbl.readOnly : null;
var sent = function (n) { return window.__out.slice(n).filter(function (m) { return m.type === 'eventGrid' || m.type === 'eventName'; }).map(function (m) { return m.type + ':' + (m.name || m.event) + (m.type === 'eventName' ? '=' + m.value : ''); }).join(','); };
var n0 = window.__out.length;
if (up) { up.focus(); up.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); }
R.mvEnter = sent(n0);
var n1 = window.__out.length;
if (up) { up.value = 'spbSaveUp'; up.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); }
R.mvTyped = sent(n1);
if (upRow) upRow.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 20, clientY: 20 }));
R.mvMenu = Array.prototype.map.call(document.querySelectorAll('.ctxmenu .ctxitem'), function (x) { return (x.classList.contains('off') ? 'off:' : 'on:') + (x.textContent.indexOf('2 ' + S(0x500b)) >= 0 ? 'all' : 'one'); }).join(',');
'@
$rMv = Render 'props_multiev' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $mvChecks
if ($rMv) {
  Check ($rMv.errors.Count -eq 0 -and $rMv.mvUp -eq '|True|True|False' -and $rMv.mvTitle -and $rMv.mvClickRo -eq $true -and $rMv.mvDblRo -eq $false) 'several selected, Events: handlers that differ = empty + placeholder (mixed), the section says 2 in common, a .dfm one among them read only' "$($rMv.mvUp) title=$($rMv.mvTitle) clickRo=$($rMv.mvClickRo) dblRo=$($rMv.mvDblRo)"
  Check ($rMv.mvEnter -eq 'eventGrid:OnMouseUp' -and $rMv.mvTyped -eq 'eventName:OnMouseUp=spbSaveUp' -and $rMv.mvMenu -eq 'off:one,on:one,on:all') 'several selected: blank + Enter = the default for all; the primary one''s name typed still sends (attaches to all); the menu: jump off, Reset (2 all) on' "$($rMv.mvEnter) | $($rMv.mvTyped) | $($rMv.mvMenu)"
} else { $script:bad++ }
$r5 = Render 'props_search' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $searchChecks
if ($r5) {
  Check ($r5.errors.Count -eq 0 -and $r5.q1.rows -ge 1 -and $r5.q1.secs -le $r5.q1.all -and $r5.q1.text -match 'spbSaveClick') 'search spbSaveClick (events list): only the rows with it' "secs=$($r5.q1.secs)/$($r5.q1.all) rows=$($r5.q1.rows) $($r5.q1.count)"
  Check ($r5.q2.rows -ge 1 -and $r5.q2.text -match 'Font\.Size') 'search font.size: the Font.Size row' "rows=$($r5.q2.rows) [$($r5.q2.text)]"
  Check ($r5.q3.rows -ge 1 -and $r5.q3.text -match '^Left') 'search "left 54": both words, the value in the input counts' "rows=$($r5.q3.rows) [$($r5.q3.text)]"
  Check ($r5.q4.rows -eq 0 -and $r5.q4.secs -eq 0 -and $r5.q4.count -ne '') 'search with no hit: nothing shown, says so' "count=$($r5.q4.count)"
  Check ($r5.q0secs -eq $r5.all -and $r5.q0.count -eq '') 'clearing the search: every section back' "secs=$($r5.q0secs)/$($r5.all)"
} else { $script:bad++ }
$htmlChecks = @'
function last() { return window.__out[window.__out.length - 1] || {}; }
function row(tbl, name) { return Array.prototype.filter.call(document.querySelectorAll('table.html tr[data-name="' + name + '"]'), function (r) { return true; })[tbl] || null; }
var sl = document.querySelector('table.html tr.editable[data-name="left"]');
R.hasLeft = !!sl;
if (sl) {
  sl.dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));
  var inp = sl.querySelector('input');
  R.leftInput = inp ? inp.value : null;
  if (inp) { inp.value = '60px'; inp.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true })); }
  var m1 = last(); R.afterLeft = m1.type + ':' + m1.name + '=' + m1.value;
}
var idr = document.querySelector('table.html tr[data-name="id"]');
if (idr) idr.dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));
R.idEditable = !!(idr && idr.querySelector('input'));
var nn = document.querySelector('input[data-field="html:+name"]'), nv = document.querySelector('input[data-field="html:+value"]');
if (nn && nv) { nn.value = 'z-index'; nv.value = '5'; nv.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true })); }
var m2 = last(); R.afterAdd = m2.type + ':' + m2.name + '=' + m2.value;
var tr = document.querySelector('table.html tr.editable[data-name="title"]');
if (tr) {
  tr.dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));
  var ti = tr.querySelector('input');
  if (ti) { ti.value = 'Save it'; ti.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true })); }
}
var m3 = last(); R.afterTitle = m3.type + ':' + m3.name + '=' + m3.value;
/* ONE click on a value = the field (as the property grid: click and type); a click on the id's does nothing; Esc = back, nothing sent */
var hsec = document.querySelector('details.sec[data-key="html"]');
if (hsec) hsec.open = true;   /* (folded by default behind the grid; a field in a folded one cannot take the focus) */
var st = document.querySelector('table.html tr.editable[data-name="top"]');
var n4 = window.__out.length;
if (st) st.querySelector('td.v').dispatchEvent(new MouseEvent('click', { bubbles: true }));
var si = st && st.querySelector('input');
R.oneClick = !!si && document.activeElement === si;
if (si) si.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true }));
R.oneClickEsc = !!st && !st.querySelector('input') && window.__out.length === n4;
var idv = document.querySelector('table.html tr[data-name="id"] td.v');
if (idv) idv.dispatchEvent(new MouseEvent('click', { bubbles: true }));
R.idClick = !!document.querySelector('table.html tr[data-name="id"] input');
'@
$r6 = Render 'props_html' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $htmlChecks
if ($r6) {
  Check ($r6.errors.Count -eq 0 -and $r6.hasLeft -and $r6.leftInput -eq '54px' -and $r6.afterLeft -eq 'setStyleProp:left=60px') 'HTML style row: double-click -> input with 54px, Enter posts left=60px' "$($r6.leftInput) -> $($r6.afterLeft)"
  Check (-not $r6.idEditable) 'the id row is not editable' ''
  Check ($r6.afterAdd -eq 'setStyleProp:z-index=5') 'new style row: name + value, Enter' $r6.afterAdd
  Check ($r6.afterTitle -eq 'setAttrProp:title=Save it') 'title attribute: double-click, change, Enter' $r6.afterTitle
  Check ($r6.oneClick -and $r6.oneClickEsc -and -not $r6.idClick) 'HTML section: ONE click on a value = its field, focused (as the grid); Esc = back, nothing sent; the id stays not editable' "click=$($r6.oneClick) esc=$($r6.oneClickEsc) id=$($r6.idClick)"
} else { $script:bad++ }
$multiChecks = @'
function b(h) { return document.querySelector('button[data-align="' + h + '"]'); }
R.have = ['left', 'size', 'hspace', 'vspace', 'hcenterIn', 'vcenterIn'].filter(function (h) { return !!b(h); }).join(',');
R.spaceOff = !!(b('hspace') && b('hspace').disabled && b('vspace').disabled);
R.centerOn = !!(b('hcenterIn') && !b('hcenterIn').disabled);
if (b('hcenterIn')) b('hcenterIn').click();
if (b('hspace')) b('hspace').click();
R.afterMulti = window.__out.filter(function (m) { return m.type === 'align'; }).map(function (m) { return m.how; }).join(',');
R.note = !!document.querySelector('.multinote');
R.multiReset = !!document.querySelector('details.sec[data-key="align"] button[data-act="resetToDfm"]');
/* the fields whose values differ: empty with a placeholder, a checkbox half set, the row marked, not compared */
var mfs = document.querySelector('input[data-field="fontSize"]'), mlf = document.querySelector('input[data-field="left"]'), mbd = document.querySelector('input[data-field="bold"]');
var mcl = document.querySelector('input[data-field="color"]');
R.mixedShown = !!(mfs && mfs.value === '' && mfs.placeholder && mlf && mlf.value === '' && mbd && mbd.indeterminate &&
  mfs.closest('tr').classList.contains('mixed') && mcl && mcl.closest('tr').classList.contains('mixed') && !mbd.closest('tr').classList.contains('diff'));
R.notMixed = (function () { var t = document.querySelector('input[data-field="top"]'); return !!t && t.value !== '' && !t.closest('tr').classList.contains('mixed'); })();
R.editAll = Array.prototype.some.call(document.querySelectorAll('details.sec > summary'), function (s) { return /\u5957\u7528\u5230\u5168\u90e8 2 \u500b/.test(s.textContent); });
/* 20261001: Reset with 2 selected = each back to its OWN .dfm value: the marker's first item posts resetProp (not the
   primary one's value written to all) -- on a row whose values differ too */
(function () {
  var tr = mlf && mlf.closest('tr'), mk = tr && tr.querySelector('button.reset');
  var n0 = window.__out.length;
  if (mk) mk.click();
  var it = document.querySelector('.ctxmenu .ctxitem');
  R.multiResetOn = !!(it && !it.classList.contains('off'));
  if (it) it.click();
  var sent = window.__out.slice(n0);
  R.multiResetMsg = sent.map(function (m) { return m.type + (m.prop ? ':' + m.prop : ''); }).join(',');
})();
/* a double click on a True / False value = switched ONCE (BCB6's Object Inspector), not twice */
(function () {
  var n0 = window.__out.length;
  if (mbd) {
    mbd.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true, detail: 1 }));
    mbd.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true, detail: 2 }));
    mbd.dispatchEvent(new MouseEvent('dblclick', { bubbles: true, cancelable: true, detail: 2 }));
  }
  R.dblBold = window.__out.slice(n0).filter(function (m) { return m.type === 'setLook' && m.prop === 'bold'; }).length;
  /* ... and on the value's empty part: switched once */
  var td = mbd && mbd.closest('td.v'), n1 = window.__out.length;
  if (td) td.dispatchEvent(new MouseEvent('dblclick', { bubbles: true, cancelable: true, detail: 2 }));
  R.dblBoldCell = window.__out.slice(n1).filter(function (m) { return m.type === 'setLook' && m.prop === 'bold'; }).length;
})();
'@
$r8 = if (Test-Path (Join-Path $dump 'props_multi.json')) { Render 'props_multi' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_multi.json') $multiChecks } else { $null }
if ($r8) {
  Check ($r8.errors.Count -eq 0 -and $r8.have -eq 'left,size,hspace,vspace,hcenterIn,vcenterIn' -and $r8.spaceOff -and $r8.centerOn -and $r8.afterMulti -eq 'hcenterIn') '2 selected: spacing buttons off (need 3), centre on and posts align hcenterIn' "have=$($r8.have) posted=$($r8.afterMulti)"
  Check ($r8.multiReset) '2 selected: "reset all to DFM" in the multi-select section' "$($r8.multiReset)"
  Check ($r8.mixedShown -and $r8.notMixed) '2 selected, values differ: those fields empty / half set and marked, not compared; the others as they are' "mixed=$($r8.mixedShown) others=$($r8.notMixed)"
  Check ($r8.note -and $r8.editAll) '2 selected: the panel says a change goes to all 2 (note + section title)' "note=$($r8.note) title=$($r8.editAll)"
  Check ($r8.multiResetOn -and $r8.multiResetMsg -eq 'resetProp:Left') '2 selected: a row''s Reset = each back to its own .dfm value (resetProp, also where the values differ)' "$($r8.multiResetOn) $($r8.multiResetMsg)"
  Check ($r8.dblBold -eq 1 -and $r8.dblBoldCell -eq 1) 'a double click on a True / False value switches it once (on the box, and on the value''s empty part)' "box=$($r8.dblBold) cell=$($r8.dblBoldCell)"
} else { $script:bad++ }
$sortChecks = @'
function keys() { return Array.prototype.map.call(document.querySelectorAll('details.sec[data-key="props"] table.kv td.k'), function (t) { return t.textContent; }); }
function click(s) { var b = document.querySelector('.head button[data-sort="' + s + '"]'); if (b) b.click(); return !!b; }
function edKeys() { return Array.prototype.map.call(document.querySelectorAll('table.edit td.k'), function (t) { return t.textContent; }); }
/* WPF's default: by category -- the editable rows grouped too (Common / Layout / Appearance / Text) */
R.catDefault = !!document.querySelector('.head button.on[data-sort="cat"]');
R.edCats = Array.prototype.map.call(document.querySelectorAll('table.edit tr.cathead'), function (tr) { return tr.getAttribute('data-cat'); });
var edLay = document.querySelector('table.edit tr.cathead[data-cat="' + String.fromCharCode(0x7248, 0x9762) + '"]');
R.edLayoutNext = edLay && edLay.nextSibling ? (edLay.nextSibling.querySelector('td.k') || {}).textContent : '';
R.edCatRows = edKeys().length;
/* WPF: a category heading folds its rows away, again = open; a search shows what it finds in a folded one;
   the DFM section's categories fold the same way; the mouse on a property's name = its description */
function vis(x) { return !!x && getComputedStyle(x).display !== 'none'; }
function edRow(k) { return Array.prototype.filter.call(document.querySelectorAll('table.edit td.k'), function (t) { return t.textContent === k; }).map(function (t) { return t.closest('tr'); })[0] || null; }
var fLeft = edRow('Left');
var fOther = document.querySelector('table.edit tr[data-catof]:not([data-catof="ed:' + String.fromCharCode(0x7248, 0x9762) + '"])');
var fSq = document.querySelector('input.filter');
if (edLay) edLay.click();
R.foldEd = !!edLay && vis(fLeft) === false && edLay.classList.contains('closed') && vis(edLay) && vis(fOther) && edLay.textContent.indexOf(String.fromCharCode(0x25b8)) >= 0;
if (fSq) { fSq.value = 'Left'; fSq.dispatchEvent(new Event('input')); }
R.foldSearch = vis(fLeft);
if (fSq) { fSq.value = ''; fSq.dispatchEvent(new Event('input')); }
R.foldStays = vis(fLeft) === false;
var kdEv = new KeyboardEvent('keydown', { key: 'Enter', bubbles: true });
if (edLay) edLay.dispatchEvent(kdEv);
R.foldOpen = vis(fLeft) && !edLay.classList.contains('closed') && edLay.textContent.indexOf(String.fromCharCode(0x25be)) >= 0;
R.descEd = fLeft ? (fLeft.querySelector('td.k').title || '') : '';
/* WPF's brush editor: a colour typed in (#abc, clRed; a bad one is marked, nothing sent) and the eyedropper */
var cHex = document.querySelector('input.hex[data-field="backgroundHex"]');
var cOut0 = window.__out.length;
var cType = function (v) { cHex.value = v; cHex.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); };
if (cHex) { cType('#abc'); cType('clRed'); cType('zzz'); }
var cSent = window.__out.slice(cOut0).filter(function (m) { return m.type === 'setLook'; }).map(function (m) { return m.prop + '=' + m.value; });
R.colorTyped = cSent.join(',') + (cHex && cHex.classList.contains('bad') ? ' bad' : '');
var cDrop = document.querySelector('button.drop[data-drop="color"]');
window.EyeDropper = function () {};
window.EyeDropper.prototype.open = function () { return { then: function (ok) { ok({ sRGBHex: '#123456' }); } }; };
var cOut1 = window.__out.length;
if (cDrop) { cDrop.disabled = false; cDrop.click(); }
R.colorDrop = window.__out.slice(cOut1).filter(function (m) { return m.type === 'setLook'; }).map(function (m) { return m.prop + '=' + m.value; }).join(',');
/* by name: A-Z, no category rows */
click('name');
R.edByName = edKeys();
R.edNameSorted = R.edByName.length === R.edCatRows && R.edByName.join('|') === R.edByName.slice().sort(function (a, b) { return a.localeCompare(b, 'en', { sensitivity: 'base' }); }).join('|') &&
  !document.querySelector('table.edit tr.cathead');
'@
# the DFM list as a whole: spbSave's data drawn again without the grid (then the DFM list is all of it) -- its
# categories fold, by name, the .dfm's own order, by category
$dfmListChecks = @'
var d2 = JSON.parse(JSON.stringify(DATA)); d2.edit = null;
window.dispatchEvent(new MessageEvent('message', { data: { type: 'show', data: d2 } }));
function keys() { return Array.prototype.map.call(document.querySelectorAll('details.sec[data-key="props"] table.kv td.k'), function (t) { return t.textContent; }); }
function click(s) { var b = document.querySelector('.head button[data-sort="' + s + '"]'); if (b) b.click(); return !!b; }
function vis(x) { return !!x && getComputedStyle(x).display !== 'none'; }
R.noGrid = !document.querySelector('table.edit') && keys().length >= 10;
var fDfm = document.querySelector('details.sec[data-key="props"] .subhead[data-cat]');
if (fDfm) fDfm.click();
R.foldDfm = !!fDfm && vis(fDfm) && vis(fDfm.nextSibling) === false;
if (fDfm) fDfm.click();
R.foldDfmBack = !!fDfm && vis(fDfm.nextSibling);
var dLeft = Array.prototype.filter.call(document.querySelectorAll('details.sec[data-key="props"] table.kv td.k'), function (t) { return t.textContent === 'Left'; })[0];
R.descDfm = dLeft ? dLeft.title : '';
click('name');
R.byName = keys();
var srt = R.byName.slice().sort(function (a, b) { a = a.toLowerCase(); b = b.toLowerCase(); return a < b ? -1 : a > b ? 1 : 0; });
R.nameSorted = R.byName.length > 4 && R.byName.join('|') === srt.join('|');
R.nameOn = !!document.querySelector('.head button.on[data-sort="name"]');
/* the .dfm's own order: spbSave starts Left, Top, Width, Height, Caption */
R.hasButtons = click('dfm');
R.dfmOrder = keys();
/* by category: the first group is the layout one (Left / Width in it), no row lost */
click('cat');
R.cats = document.querySelectorAll('details.sec[data-key="props"] .subhead[data-cat]').length;
var lay = document.querySelector('details.sec[data-key="props"] .subhead[data-cat]');
R.layoutRows = lay && lay.nextSibling ? Array.prototype.map.call(lay.nextSibling.querySelectorAll('td.k'), function (t) { return t.textContent; }) : [];
R.catCount = keys().length;
click('name');
R.backToName = keys().join('|') === R.byName.join('|');
'@
# the IO lamp / panel button (EastSun 20261001: the old components' properties): spbSave's data drawn again as each
$ioChecks = @'
var CAT = String.fromCharCode(0x5143, 0x4ef6);
function redraw(io, dio) {
  var d2 = JSON.parse(JSON.stringify(DATA));
  d2.edit.look.io = io; d2.edit.dfm = d2.edit.dfm || {}; d2.edit.dfm.io = dio;
  window.dispatchEvent(new MessageEvent('message', { data: { type: 'show', data: d2 } }));
}
function rowOf(k) { return Array.prototype.filter.call(document.querySelectorAll('table.edit td.k'), function (t) { return t.textContent === k; }).map(function (t) { return t.closest('tr'); })[0] || null; }
function last() { return window.__out[window.__out.length - 1] || {}; }
redraw({ kind: 'btn', flat: true, down: false, trueColor: '#05b5dc', falseColor: '#01479d', trueFontColor: '#ffffff', falseFontColor: '#ffffff' },
  { flat: true, down: true, trueColor: '#05b5dc', falseColor: '#01479d', trueFontColor: '#ffffff', falseFontColor: null });
var ioHead = Array.prototype.filter.call(document.querySelectorAll('table.edit tr.cathead'), function (tr) { return (tr.getAttribute('data-cat') || '').indexOf(CAT) >= 0; })[0];
R.btnCat = !!ioHead;
R.btnRows = ['Style', 'Down', 'TrueColor', 'FalseColor', 'TrueFontColor', 'FalseFontColor'].filter(function (k) { return !!rowOf(k); }).join(',');
R.btnDiff = Array.prototype.map.call(document.querySelectorAll('table.edit tr.diff td.k'), function (t) { return t.textContent; }).join(',');
var stSel = document.querySelector('select[data-field="io.flat"]');
R.btnStyle = stSel ? stSel.value : null;
if (stSel) { stSel.value = 'tsButtons'; stSel.dispatchEvent(new Event('change')); }
var m1 = last(); R.btnStyleSent = m1.type + ':' + m1.prop + '=' + m1.value;
var dMk = rowOf('Down') && rowOf('Down').querySelector('button.reset.marker');
if (dMk) dMk.click();
var dItem = document.querySelector('.ctxmenu .ctxitem');
if (dItem) dItem.click();
var m2 = last(); R.btnDownReset = m2.type + ':' + m2.prop + '=' + m2.value;
var fcHex = document.querySelector('input.hex[data-field="io.trueFontColorHex"]');
if (fcHex) { fcHex.value = 'clBlack'; fcHex.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); }
var m3 = last(); R.btnFontSent = m3.type + ':' + m3.prop + '=' + m3.value;
redraw({ kind: 'led', ledStyle: 'LEDHorizontal', value: false, blink: false, trueColor: '#00ff00', falseColor: '#c0c0c0' },
  { ledStyle: 'LEDHorizontal', value: null, blink: null, trueColor: '#00ff00', falseColor: '#c0c0c0' });
R.ledRows = ['LEDStyle', 'Value', 'Blink', 'TrueColor', 'FalseColor'].filter(function (k) { return !!rowOf(k); }).join(',');
R.ledNoBtn = !rowOf('Down') && !rowOf('Style');
var lsSel = document.querySelector('select[data-field="io.ledStyle"]');
R.ledOpts = lsSel ? lsSel.options.length : 0;
if (lsSel) { lsSel.value = 'LEDSqLarge'; lsSel.dispatchEvent(new Event('change')); }
var m4 = last(); R.ledStyleSent = m4.type + ':' + m4.prop + '=' + m4.value;
var vCb = document.querySelector('input[data-field="io.value"]');
if (vCb) { vCb.checked = true; vCb.dispatchEvent(new Event('change')); }
var m5 = last(); R.ledValueSent = m5.type + ':' + m5.prop + '=' + m5.value;
R.ledDesc = (rowOf('LEDStyle') && rowOf('LEDStyle').querySelector('td.k').title || '').split('\n')[0];
redraw(null, null);
R.noIoRows = !rowOf('LEDStyle') && !rowOf('Down');
'@
$rIo = Render 'props_io' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $ioChecks
if ($rIo) {
  Check ($rIo.errors.Count -eq 0 -and -not $rIo.checkError -and $rIo.btnCat -and $rIo.btnRows -eq 'Style,Down,TrueColor,FalseColor,TrueFontColor,FalseFontColor' -and ($rIo.btnDiff -split ',') -contains 'Down' -and $rIo.btnStyle -eq 'tsFlatButtons') 'IO panel button: its own category with Style / Down / the four colours (BCB6 names); Down differs from the .dfm and is marked' "$($rIo.btnRows) | diff=$($rIo.btnDiff) | style=$($rIo.btnStyle) $($rIo.checkError)"
  Check ($rIo.btnStyleSent -eq 'setLook:io.flat=false' -and $rIo.btnDownReset -eq 'setLook:io.down=true' -and $rIo.btnFontSent -eq 'setLook:io.trueFontColor=#000000') 'IO panel button edits: Style tsButtons = flat off, the marker resets Down to the .dfm, a colour typed (clBlack)' "$($rIo.btnStyleSent) | $($rIo.btnDownReset) | $($rIo.btnFontSent)"
  Check ($rIo.ledRows -eq 'LEDStyle,Value,Blink,TrueColor,FalseColor' -and $rIo.ledNoBtn -and $rIo.ledOpts -eq 6 -and $rIo.ledStyleSent -eq 'setLook:io.ledStyle=LEDSqLarge' -and $rIo.ledValueSent -eq 'setLook:io.value=true' -and $rIo.ledDesc -match '^LEDStyle$|LEDSmall' -and $rIo.noIoRows) 'IO lamp: LEDStyle (the 6 shapes) / Value / Blink / TrueColor / FalseColor, each sends its change; no such rows for an ordinary control' "$($rIo.ledRows) | opts=$($rIo.ledOpts) | $($rIo.ledStyleSent) | $($rIo.ledValueSent) | none=$($rIo.noIoRows)"
} else { $script:bad++ }
$r9 = Render 'props_sort' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $sortChecks
$rDl = Render 'props_dfmlist' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_spbSave.json') $dfmListChecks
if ($r9 -and $rDl) {
  Check ($r9.errors.Count -eq 0 -and $r9.catDefault -and @($r9.edCats).Count -ge 3 -and $r9.edLayoutNext -eq 'Left' -and $r9.edNameSorted) 'WPF property grid: by category by default (the editable rows grouped, Layout starts with Left); by name = A-Z without groups' "cats=$(@($r9.edCats) -join ',') rows=$($r9.edCatRows)"
  Check ($r9.foldEd -and $r9.foldSearch -and $r9.foldStays -and $r9.foldOpen -and $rDl.noGrid -and $rDl.foldDfm -and $rDl.foldDfmBack) 'WPF property grid: a category heading folds its rows (the others stay), a search still finds them, Enter opens it again; the DFM categories fold too' "ed=$($r9.foldEd) search=$($r9.foldSearch) stays=$($r9.foldStays) open=$($r9.foldOpen) dfm=$($rDl.foldDfm)/$($rDl.foldDfmBack)"
  Check ($r9.descEd -match '^Left\n' -and $rDl.descDfm -match '^Left\n' -and $rDl.descDfm -match '\.dfm') 'WPF: the mouse on a property name = its description (the editable row and the DFM row; the DFM one keeps its double-click line)' (($rDl.descDfm -replace "`n", ' / '))
  Check ($r9.colorTyped -eq 'background=#aabbcc,background=#ff0000 bad' -and $r9.colorDrop -eq 'color=#123456') 'WPF brush editor: a colour typed in (#abc, clRed; zzz marked, not sent) and the eyedropper' "typed=$($r9.colorTyped) drop=$($r9.colorDrop)"
  Check ($rDl.errors.Count -eq 0 -and $rDl.nameSorted -and $rDl.nameOn) 'DFM properties by name: A-Z, the button marked' "rows=$($rDl.byName.Count) first=$($rDl.byName[0])"
  Check ($rDl.hasButtons -and (($rDl.dfmOrder | Select-Object -First 5) -join ',') -eq 'Left,Top,Width,Height,Caption' -and $rDl.dfmOrder.Count -eq $rDl.byName.Count) 'DFM order: as BCB6 wrote them (Left, Top, Width, Height, Caption ...)' (($rDl.dfmOrder | Select-Object -First 6) -join ',')
  Check ($rDl.cats -ge 2 -and ($rDl.layoutRows -contains 'Left') -and ($rDl.layoutRows -contains 'Width') -and $rDl.catCount -eq $rDl.byName.Count -and $rDl.backToName) 'by category: groups (layout first, with Left / Width), no row lost; back to by name' "groups=$($rDl.cats) layout=$($rDl.layoutRows -join ',')"
} else { $script:bad++ }
$propsChecks = $propsChecks + @'
/* (a component without the editable grid: its whole DFM list, open -- the properties list is never empty-looking) */
R.dfmOpenNoGrid = !document.querySelector('table.edit') ? !!(document.querySelector('details.sec[data-key="props"]') || {}).open : 'grid';
'@
$r2 = Render 'props_XST1' 'props.css' 'props.js' 'show' (Join-Path $dump 'props_XST1.json') $propsChecks
if ($r2) {
  Check ($r2.errors.Count -eq 0 -and $r2.hasField -and $r2.hasXStart) 'props panel (XST1) renders the data field [Hotplate Form] X Start' ($r2.errors -join ' / ')
  Check ($r2.dfmOpenNoGrid -eq $true) 'without the editable grid: the whole DFM list, open (the properties list never empty)' "noGrid=$($r2.dfmOpenNoGrid)"
} else { $script:bad++ }
$ovChecks = @'
R.rows = document.querySelectorAll('table.grid tbody tr').length;
R.chips = document.querySelectorAll('.chip').length;
var tr = document.querySelector('table.grid tbody tr');
if (tr) tr.click();
R.after = window.__out.map(function (m) { return m.type + (m.id ? ':' + m.id : ''); });
'@
$r3 = Render 'overview' 'overview.css' 'overview.js' 'data' (Join-Path $dump 'overview_HotPlate.json') $ovChecks
if ($r3) {
  Check ($r3.errors.Count -eq 0 -and $r3.rows -eq 19 -and $r3.chips -eq 7) 'overview renders 19 event rows and 7 summary chips' "rows=$($r3.rows) chips=$($r3.chips) errors=$($r3.errors -join ' / ')"
  Check (($r3.after -join ',') -match 'select:') 'clicking a row posts select' ($r3.after -join ',')
} else { $script:bad++ }
$ddChecks = @'
R.rows = document.querySelectorAll('tr.diffrow').length;
R.props = Array.prototype.map.call(document.querySelectorAll('tr.diffrow'), function (tr) { return tr.getAttribute('data-prop'); });
var rb = document.querySelector('tr.diffrow[data-prop="Font.Bold"] button.reset');
if (rb) rb.click();
var last = window.__out[window.__out.length - 1] || {};
R.afterReset = rb ? last.type + ':' + last.id + ':' + (last.reset && last.reset.prop) + '=' + (last.reset && last.reset.value) : 'no button';
var tr0 = document.querySelector('tr.diffrow');
if (tr0) tr0.click();
last = window.__out[window.__out.length - 1] || {};
R.afterClick = last.type + ':' + last.id;
var gb = document.querySelector('button[data-group="layout"]');
if (gb) gb.click();
R.rowsLayoutOnly = document.querySelectorAll('tr.diffrow').length;
var ghb = document.querySelector('button[data-act="ghosts"]');
R.ghostBtn = ghb ? 'on=' + /\bon\b/.test(ghb.className) : 'none';
if (ghb) ghb.click();
R.afterGhost = (window.__out[window.__out.length - 1] || {}).type;
'@
$patChecks = @'
var ab = document.querySelector('tr.patrow button[data-act="resetPattern"]');
R.hasButton = !!ab;
R.patKey = ab ? ab.closest('tr').getAttribute('data-key') : null;
if (ab) ab.click();
var lastP = window.__out[window.__out.length - 1] || {};
R.afterPat = lastP.type + ':' + lastP.key;
R.layoutButtons = Array.prototype.filter.call(document.querySelectorAll('tr.patrow'), function (r) { return /\|(Left|Top|Width|Height)\|/.test(r.getAttribute('data-key')) && r.querySelector('button'); }).length;
'@
$r7 = Render 'dfmdiff_pattern' 'overview.css' 'dfmdiff.js' 'data' (Join-Path $dump 'dfmdiff_pattern.json') $patChecks
if ($r7) {
  Check ($r7.errors.Count -eq 0 -and $r7.hasButton -and $r7.afterPat -eq ('resetPattern:' + $r7.patKey) -and $r7.layoutButtons -eq 0) 'pattern line: "reset all" posts resetPattern with its key; no button on position lines' "$($r7.afterPat)"
} else { $script:bad++ }
$r4 = Render 'dfmdiff' 'overview.css' 'dfmdiff.js' 'data' (Join-Path $dump 'dfmdiff_HotPlate.json') $ddChecks
if ($r4) {
  Check ($r4.errors.Count -eq 0 -and $r4.rows -eq 1 -and ($r4.props -join ',') -eq 'Font.Bold') 'DFM difference list renders Font.Bold' "rows=$($r4.rows) props=$($r4.props -join ',') errors=$($r4.errors -join ' / ')"
  Check ($r4.afterReset -eq 'reset:spbSave:bold=False' -and $r4.afterClick -eq 'select:spbSave') 'reset button posts reset, a row click posts select' "$($r4.afterReset) | $($r4.afterClick)"
  Check ($r4.rowsLayoutOnly -eq 0) 'filter layout (position/size) hides the look rows' "rows=$($r4.rowsLayoutOnly)"
  Check ($r4.ghostBtn -eq 'on=false' -and $r4.afterGhost -eq 'ghosts') 'the DFM-place button: shown off, a click posts ghosts' "$($r4.ghostBtn) -> $($r4.afterGhost)"
} else { $script:bad++ }
# the properties panel with REAL input (20261001): the wheel over a focused number field, Up / Down on a focused
# drop-down, a real double click on a True / False box -- synthetic events cannot show these
$propsReal = Join-Path $dump 'props_realinput.json'
if ([IO.File]::Exists($propsReal)) { [IO.File]::Delete($propsReal) }
$codeExe = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$env:ELECTRON_RUN_AS_NODE = '1'
$pr = Start-Process -FilePath $codeExe -ArgumentList "`"$(Join-Path $here 'props_realinput.js')`"", "`"$media`"", "`"$(Join-Path $dump 'props_spbSave.json')`"", "`"$propsReal`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
$PR = if ([IO.File]::Exists($propsReal)) { [IO.File]::ReadAllText($propsReal, $utf8) | ConvertFrom-Json } else { $null }
Check ($PR -and $PR.wheelOk) 'real input: the wheel over a focused number field scrolls the panel, the value stays, nothing written' $(if ($PR) { $PR.wheel | ConvertTo-Json -Compress } else { "exit $($pr.ExitCode)" })
Check ($PR -and $PR.selectArrowOk) 'real input: Up / Down on a focused drop-down = the row above / below, the value stays (Object Inspector)' $(if ($PR) { $PR.selectArrow | ConvertTo-Json -Compress } else { '-' })
Check ($PR -and $PR.dblBoldOk) 'real input: a double click on a True / False box switches it once' $(if ($PR) { $PR.dblBold | ConvertTo-Json -Compress } else { '-' })
Check ($PR -and -not $PR.error -and @($PR.pageErrors).Count -eq 0) 'real input: the driver ran, no page error' $(if ($PR) { "$($PR.error)" } else { '-' })
Write-Host ''
if ($script:bad) { Write-Host "RESULT: $($script:bad) fail" } else { Write-Host 'RESULT: all pass' }
exit $script:bad
