// AI(W906-HSYS-TABLE) 20261002 [W906] (St01 ST01-E2): ctest HSys_TablePage -- D:\HT9045\web\page\ht9045_hsys_table_c.js (the page_Old
//   table layout of HW.HandlerSys.html, todo E-028) on the real page in headless Edge (offline: no wb_serve, nothing written outside the temp
//   folder). The script needs a real DOM (innerHTML tables, closest / matches(':disabled'), getComputedStyle), so instead of a fake DOM this
//   copies the page into a temp folder with <base href> pointing back at web\page and adds a probe script, then reads the probe's JSON
//   out of `msedge --headless --dump-dom`.
//   [A] files: no BOM, one EOL style; HW.HandlerSys.html links ht9045_hsys_table.css and loads ht9045_hsys_table_c.js after
//       ht9045_hsys_heater_c.js and ht9045_hsys_events_c.js
//   [B] coverage: every original control in tabs 0-6 / 8 that the tables hide has a proxy, except the golden-hidden GroupBox1 / GroupBox2
//       (SafeDoor / HeaterDoor); no source has proxies in two tables; tab order
//   [C] table -> original goes through real events, so golden handlers run (ht9045_hsys_events_c.js / ht9045_hsys_heater_c.js):
//       TTL Card 3 -> Address = Yes + greyed; Rotate Kit type -> In / Out greyed; ATC3.3+6.0 = rgATC 6 + rgATCMixMode 1; Fix 3; Can Go Rear;
//       a checkbox; rgHeaterType gets its change event; Com Port Set Default = golden btnSetATCCom (cbComIndex COM11); the keypad opens on
//       the original edit with its value; Machine Track hides rows and changes no value; B69 (Steven 1003): six per-track ART ticks,
//       the Auto 2 one writes rgAuto2ART
//   [D] original -> table: a value set without an event, golden Visible=false (whole row folds), Enabled=false
//   [E] Search Function still moves a hidden GroupBox into its box (shown) and back (hidden again); tabs 7 / 9 untouched
//   [F] E-029 (Steven 1002 16:4x): grpHeater sits in the Temperature tab between Temperature Settings and Tri Temperature; Heater Type /
//       Index Heater Counts are proxy rows of that section (not of Temperature Settings); the Heater tab is hidden by class and pane 10
//       is empty; the section follows C++ TabVisible on tsHeater (engine style.display) both ways
//   Control: the same probe on a copy whose pick() sets .checked without an event (page_Old's way) must be red ([C] TTL card).
//   Exit 77 (ctest SKIP) when Edge is not installed. Use: only through ctest (HSys_TablePage); W906_EDGE overrides the Edge path.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');
const url = require('url');
const child = require('child_process');

const PAGE_DIR = path.resolve(process.env.W906_HSYS_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page'));
const PAGE = 'HW.HandlerSys.html', JS = 'ht9045_hsys_table_c.js', CSS = 'ht9045_hsys_table.css';
let failed = 0, passed = 0;
function check(name, fn) {
  try { fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
function eq(a, b, what) { if (JSON.stringify(a) !== JSON.stringify(b)) throw new Error(what + ': got ' + JSON.stringify(a) + ', want ' + JSON.stringify(b)); }

function findEdge() {
  const c = [process.env.W906_EDGE, 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'];
  for (const p of c) if (p && fs.existsSync(p)) return p;
  return null;
}

// The probe: runs inside the page after the tables are built; writes <pre id="__result"> JSON.
const PROBE = String.raw`
<script>
(function () {
  function $(id) { return document.getElementById(id); }
  function radios(el) { return el ? Array.prototype.slice.call(el.querySelectorAll('input[type="radio"]')) : []; }
  function ci(el) { var r = radios(el); for (var i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; }
  function fire(el, t) { el.dispatchEvent(new Event(t, { bubbles: true })); }
  function px(id) { return document.querySelector('.ht-proxy-root [data-hst-src="' + id + '"]'); }
  var out = { errors: [], tests: {} }, T = out.tests;
  window.addEventListener('error', function (e) { out.errors.push(String(e.message) + ' @' + e.filename + ':' + e.lineno); });
  function done() { var p = document.createElement('pre'); p.id = '__result'; p.textContent = JSON.stringify(out); document.body.appendChild(p); }
  function step(name, fn) { try { fn(); } catch (e) { out.errors.push(name + ': ' + (e && e.message || e)); } }
  setTimeout(function () {
    step('coverage', function () {
      var COMBINED = { rgRotateKit: 'rgRotateKit_Type', rgATCMixMode: 'rgATC', rgFix3FullPlace: 'rgInstallFix3' }, miss = [], proxied = 0;
      ['0', '1', '2', '3', '4', '5', '6', '8'].forEach(function (p) {
        document.querySelector('.pcPane[data-p="' + p + '"]').querySelectorAll('fieldset.rg, label.ckb, select, input').forEach(function (el) {
          if (el.closest('.ht-proxy-root')) return;
          if (el.type === 'radio' || (el.tagName === 'INPUT' && el.type === 'checkbox' && el.closest('label.ckb'))) return;
          if (!(el.closest('.ht-src-hidden') || el.closest('.pcPane.ht-track-matrix-active > .pnl'))) return;
          if (el.id && (px(el.id) || (COMBINED[el.id] && px(COMBINED[el.id])))) { proxied++; return; }
          var box = el.closest('fieldset.gbx:not(.rg)');
          miss.push((el.id || el.tagName) + '@' + (box ? box.id : ''));
        });
      });
      var seen = {}, dup = [];
      document.querySelectorAll('.ht-proxy-root [data-hst-src]').forEach(function (p) {
        var id = p.getAttribute('data-hst-src'), root = p.closest('.ht-inout-table, .ht-proxy-root').id;
        if (seen[id] && seen[id] !== root && id !== 'rgAutoTrackCanGoRear') dup.push(id); seen[id] = seen[id] || root;
      });
      out.coverage = { proxied: proxied, missing: miss, duplicates: dup };
      out.tabs = Array.prototype.map.call(document.querySelectorAll('#pcSetting > .pcTabs > .tab'), function (t) { return t.dataset.t; });
      var hs = $('htHeaterSection'), t10 = document.querySelector('#pcSetting > .pcTabs > .tab[data-t="10"]');
      out.heater = [!!hs && !!$('grpHeater').closest('#htHeaterSection'), hs && hs.previousElementSibling ? hs.previousElementSibling.id : '',
                    hs && hs.nextElementSibling ? hs.nextElementSibling.id : '',
                    ['rgHeaterType', 'rgHeater'].map(function (id) { var p = px(id); return p ? p.closest('section').id : ''; }).join(','),
                    !!t10 && t10.classList.contains('ht-tab-moved') && getComputedStyle(t10).display === 'none',
                    !document.querySelector('.pcPane[data-p="10"] #grpHeater')];
      out.untouched = ['7', '9'].map(function (p) { return document.querySelectorAll('.pcPane[data-p="' + p + '"] .ht-src-hidden, .pcPane[data-p="' + p + '"] .ht-proxy-root').length; });
    });
    step('ttlCard', function () { var c = px('rgTTLCard'); c.selectedIndex = 3; fire(c, 'change');
      T.ttlCard = [ci($('rgTTLCard')), ci($('rgTTLUseAddress')), $('rgTTLUseAddress').getAttribute('aria-disabled')]; });
    step('rotateKit', function () { var r = document.querySelector('[data-rotate-kit-combined]'); r.selectedIndex = 3; fire(r, 'change');
      T.rotateKit = [ci($('rgRotateKit')), ci($('rgRotateKit_Type')), $('rgRotateKitIn').getAttribute('aria-disabled')]; });
    step('atc', function () { var a = document.querySelector('[data-atc-mode-combined]'); a.selectedIndex = a.options.length - 1; fire(a, 'change');
      T.atc = [ci($('rgATC')), ci($('rgATCMixMode'))]; });
    step('fix3', function () { var f = document.querySelector('[data-track-fix3]'); f.selectedIndex = 4; fire(f, 'change');
      T.fix3 = [ci($('rgInstallFix3')), ci($('rgFix3FullPlace'))]; });
    step('rear', function () { var r = document.querySelector('[data-track-toggle="rgAutoTrackCanGoRear"]'); r.checked = !r.checked; fire(r, 'change');
      T.rear = [r.checked, ci($('rgAutoTrackCanGoRear')) === (r.checked ? 1 : 0)];
      T.artCount = document.querySelectorAll('.ht-proxy-root [data-hst-src^="rgAuto"][data-hst-src$="ART"]').length; });
    step('art', function () { var a = document.querySelector('[data-track-toggle="rgAuto2ART"]');   // B69: the tick writes the original radio
      a.checked = !a.checked; fire(a, 'change'); T.art = [T.artCount, ci($('rgAuto2ART')), a.checked ? 1 : 0]; });
    step('checkbox', function () { var b = px('chkUser_Define_IndexZ_SafePos'), s = $('chkUser_Define_IndexZ_SafePos').querySelector('input'), was = s.checked;
      b.checked = !was; fire(b, 'change'); T.checkbox = [was, s.checked]; });
    var empty0 = $('chkEmpty').querySelector('input').checked;
    step('machineTrack', function () { var m = px('MachineTrack'); m.selectedIndex = 0; fire(m, 'change'); });
    var heaterEvents = 0;
    step('heaterType', function () { $('rgHeaterType').addEventListener('change', function () { heaterEvents++; });
      var h = px('rgHeaterType'); h.selectedIndex = (h.selectedIndex + 1) % h.options.length; fire(h, 'change'); });
    step('setDefault', function () { document.querySelector('[data-com-set-atc-default]').click(); });
    step('keypad', function () { var t = px('edtHPLimit'); $('edtHPLimit').value = '7.25';
      t.dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, button: 0 }));
      var ov = document.querySelector('.qkOv'), d = ov && ov.querySelector('.qkDisp');
      T.keypad = [t.readOnly, !!ov, d ? d.value : null]; if (ov) ov.parentNode.removeChild(ov); });
    step('tabVisibleOff', function () { document.querySelector('#pcSetting > .pcTabs > .tab[data-t="10"]').style.display = 'none'; });   // what the engine does for TabVisible=false
    step('mirrorSetup', function () { radios($('rgPickerCount'))[1].checked = true;
      $('rgIndexMotorType').style.visibility = 'hidden'; $('rgIndexMotorAxis').style.visibility = 'hidden'; $('edMaxKpa').disabled = true; });
    setTimeout(function () {
      step('later', function () {
        T.machineTrack = [document.querySelector('[data-track-name="empty"]').style.display, $('chkEmpty').querySelector('input').checked === empty0];
        T.heaterType = heaterEvents;
        var c = $('cbComIndex'), p = px('cbComIndex');
        T.setDefault = [c.options[c.selectedIndex].text, p.options[p.selectedIndex].text];
        T.mirror = [px('rgPickerCount').selectedIndex, !!px('rgIndexMotorType').closest('.ht-proxy-gone'), !!px('rgIndexMotorType').closest('tr.ht-row-gone'),
                    px('edMaxKpa').disabled, px('rgTTLUseAddress').disabled];
        var s = $('edtSearchFunction'), g = $('gbRotateKit');
        s.value = 'ROTATE'; fire(s, 'input');
        T.search = [!!g.closest('#scrlbxSearchFunc'), getComputedStyle(g).display];
        s.value = 'ZZZQ'; fire(s, 'input');
        T.search.push(!g.closest('#scrlbxSearchFunc'), getComputedStyle(g).display);
        T.tabVisible = [$('htHeaterSection').classList.contains('ht-section-gone')];
        document.querySelector('#pcSetting > .pcTabs > .tab[data-t="10"]').style.display = '';                   // TabVisible=true again
      });
      setTimeout(function () {
        step('tabVisibleOn', function () { T.tabVisible.push(!$('htHeaterSection').classList.contains('ht-section-gone')); });
        done();
      }, 700);
    }, 700);
  }, 900);
})();
</script>
`;

function probe(edge, tmp, tag, jsOverride) {
  let html = fs.readFileSync(path.join(PAGE_DIR, PAGE), 'utf8');
  const base = url.pathToFileURL(PAGE_DIR).href.replace(/\/?$/, '/');
  html = html.replace('<head>', '<head>\n<base href="' + base + '">');
  if (jsOverride) {
    const tagJs = '<script src="' + JS + '"></script>';
    ok(html.includes(tagJs), 'page has no ' + tagJs);
    html = html.replace(tagJs, '<script src="' + url.pathToFileURL(jsOverride).href + '"></script>');
  }
  ok(html.includes('</body>'), 'page has no </body>');
  html = html.replace('</body>', PROBE + '</body>');
  const file = path.join(tmp, tag + '.html');
  fs.writeFileSync(file, html, 'utf8');
  const r = child.spawnSync(edge, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--disable-extensions',
    '--user-data-dir=' + path.join(tmp, 'profile-' + tag), '--allow-file-access-from-files', '--virtual-time-budget=8000',
    '--dump-dom', url.pathToFileURL(file).href], { encoding: 'utf8', timeout: 90000, maxBuffer: 64 * 1024 * 1024, windowsHide: true });
  const m = /<pre id="__result">([\s\S]*?)<\/pre>/.exec(r.stdout || '');
  ok(m, tag + ': no probe result from Edge (status ' + r.status + (r.error ? ', ' + r.error.message : '') + ')');
  const txt = m[1].replace(/&quot;/g, '"').replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&amp;/g, '&');
  return JSON.parse(txt);
}

function verdicts(d) {        // [name, fn] list so the control run can tell which ones go red
  const allowed = /^(SafeDoor\d+|HeaterDoor\d+)@GroupBox[12]$/;   // B69 (Steven 1003): the per-track Auto ART have proxies now
  const T = d.tests;
  return [
    ['[B] no page error', () => eq(d.errors, [], 'errors')],
    ['[B] every hidden original has a proxy', () => {
      const bad = (d.coverage.missing || []).filter(x => !allowed.test(x));
      eq(bad, [], 'hidden originals without a proxy');
      ok(d.coverage.proxied >= 250, 'only ' + d.coverage.proxied + ' originals proxied');
    }],
    ['[B] no source in two tables', () => eq(d.coverage.duplicates, [], 'duplicates')],
    ['[B] tab order', () => eq(d.tabs, ['0', '1', '2', '4', '3', '5', '8', '6', '9', '10', '7'], 'tabs')],
    ['[E] tabs 7 / 9 untouched', () => eq(d.untouched, [0, 0], 'hidden / proxy elements in tabs 7, 9')],
    ['[F] Heater section between Temperature Settings and Tri Temperature', () => eq(d.heater.slice(0, 3), [true, 'htTemperatureSettings', 'htTriTemperatureSettings'], 'grpHeater in #htHeaterSection, previous, next')],
    ['[F] Heater Type / Index Heater Counts are rows of the Heater section', () => eq(d.heater[3], 'htHeaterSection,htHeaterSection', 'section of the rgHeaterType, rgHeater proxies')],
    ['[F] Heater tab hidden by class, pane 10 empty', () => eq(d.heater.slice(4), [true, true], 'tab 10 ht-tab-moved + display none, no grpHeater in pane 10')],
    ['[F] Heater section follows C++ TabVisible', () => eq(T.tabVisible, [true, true], 'gone when tsHeater display none, back when shown')],
    ['[C] TTL Card 3 -> Address Yes + greyed (rgTTLCardClick ran)', () => eq(T.ttlCard, [3, 1, 'true'], 'rgTTLCard, rgTTLUseAddress, aria-disabled')],
    ['[C] Rotate Kit type -> In greyed (rgRotateKit_TypeClick ran)', () => eq(T.rotateKit, [1, 2, 'true'], 'rgRotateKit, rgRotateKit_Type, In aria-disabled')],
    ['[C] ATC3.3+6.0 = rgATC 6 + MixMode 1', () => eq(T.atc, [6, 1], 'rgATC, rgATCMixMode')],
    ['[C] Fix 3 Use Cylinder', () => eq(T.fix3, [1, 2], 'rgInstallFix3, rgFix3FullPlace')],
    ['[C] Can Go Rear written', () => eq(T.rear[1], true, 'rgAutoTrackCanGoRear follows the tick')],
    ['[C] B69 per-track ART: 6 ticks, the Auto 2 tick writes rgAuto2ART', () => eq([T.art[0], T.art[1]], [6, T.art[2]], 'ART proxies, rgAuto2ART after the tick')],
    ['[C] checkbox written', () => eq(T.checkbox[1], !T.checkbox[0], 'chkUser_Define_IndexZ_SafePos')],
    ['[C] rgHeaterType gets its change event', () => eq(T.heaterType, 1, 'change events on rgHeaterType')],
    ['[C] Set Default = golden btnSetATCCom', () => eq(T.setDefault, ['COM11', 'COM11'], 'cbComIndex, proxy')],
    ['[C] keypad opens on the original with its value', () => eq(T.keypad, [true, true, '7.25'], 'proxy readOnly, keypad open, shown value')],
    ['[C] Machine Track hides rows, changes no value', () => eq(T.machineTrack, ['none', true], 'Empty row display, chkEmpty unchanged')],
    ['[D] original -> table (value, Visible, Enabled)', () => eq(T.mirror, [1, true, true, true, true], 'pickerCount, unit gone, row gone, edMaxKpa disabled, Address disabled')],
    ['[E] Search Function moves a hidden GroupBox and back', () => eq(T.search, [true, 'block', true, 'none'], 'in box, display, back, display')],
  ];
}

function main() {
  // [A] files
  const page = fs.readFileSync(path.join(PAGE_DIR, PAGE));
  const files = { [PAGE]: page, [JS]: fs.readFileSync(path.join(PAGE_DIR, JS)), [CSS]: fs.readFileSync(path.join(PAGE_DIR, CSS)) };
  for (const f of Object.keys(files)) check('[A] ' + f + ': no BOM, one EOL style', () => {
    const b = files[f], t = b.toString('utf8');
    ok(!(b[0] === 0xEF && b[1] === 0xBB && b[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (t.match(/\r\n/g) || []).length, lf = (t.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  check('[A] page links the css and loads the script after heater / events', () => {
    const t = page.toString('utf8');
    const iCss = t.indexOf('href="' + CSS + '"'), iHead = t.indexOf('</head>');
    ok(iCss > 0 && iCss < iHead, CSS + ' not linked in <head>');
    const iJs = t.indexOf('<script src="' + JS + '">'), iH = t.indexOf('<script src="ht9045_hsys_heater_c.js">'), iE = t.indexOf('<script src="ht9045_hsys_events_c.js">');
    ok(iJs > 0, JS + ' not loaded');
    ok(iH > 0 && iE > 0 && iJs > iH && iJs > iE, JS + ' must load after ht9045_hsys_heater_c.js and ht9045_hsys_events_c.js');
  });

  const edge = findEdge();
  if (!edge) { console.log('SKIP no Microsoft Edge (set W906_EDGE) -- [B]..[E] need a real DOM'); process.exit(failed ? 1 : 77); }
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'hsys_table_selftest_'));
  try {
    let d = null;
    check('[B] probe ran in Edge', () => { d = probe(edge, tmp, 'real'); });
    if (d) for (const [name, fn] of verdicts(d)) check(name, fn);

    // Control: page_Old's write-back (.checked = true, no event) must be caught by [C]
    check('control: a copy without real events is red', () => {
      const src = fs.readFileSync(path.join(PAGE_DIR, JS), 'utf8');
      const good = 'function pick(el, i) { var r = radios(el)[i]; if (r && !r.checked) r.click(); }';
      ok(src.includes(good), 'pick() not found in ' + JS + ' (update the control)');
      const broken = path.join(tmp, 'broken_' + JS);
      fs.writeFileSync(broken, src.replace(good, 'function pick(el, i) { var r = radios(el)[i]; if (r && !r.checked) r.checked = true; }'), 'utf8');
      const b = probe(edge, tmp, 'broken', broken);
      const red = verdicts(b).filter(([, fn]) => { try { fn(); return false; } catch (e) { return true; } }).map(([n]) => n);
      ok(red.some(n => n.indexOf('TTL Card') >= 0), 'the broken copy passed the TTL Card check (red: ' + JSON.stringify(red) + ')');
      console.log('     control red: ' + red.length + ' check(s), e.g. ' + red[0]);
    });
  } finally {
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { /* Edge may still hold the profile for a moment */ }
  }
  console.log((failed ? 'FAILED ' : 'OK ') + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
}
main();
