'use strict';
// =============================================================================
//  tools/webprobe/teach_zdown_selftest.cjs -- ctest TeachZDownPage.  AI(W906-TEACH-ZDOWN) 20261003 (INBOX 147).
//
//  web/page/ht9045_teach_zdown_c.js offline: a minimal fake DOM, the REAL file, the REAL catalog (web/JSON/motor-access.json) and
//  the REAL TEACH_UNWIRED grey code cut out of web/page/HW.teach.html (from `var TEACH_UNWIRED_CSS=` to its load-time call).
//   1. HW.teach.html loads ht9045_teach_zdown_c.js, and teachOnAck hands the teachSetAllArmZ / teachOutZAllDown replies to
//      HT9045TeachZDown.onAck BEFORE its teachSet / teachGo filter (else the Set All reply would never reach the boxes).
//   2. the page's own grey no longer greys btnSetAllInArmZ / btnSetAllOutArmZ / btnOutZAllDown (TEACH_UNWIRED + TEACH_UNWIRED_B38 as
//      the page has them), while it still greys btnSetAllSortArmZ (control: the grey ran).
//   3. after the page's motor channel is up, the three buttons are bound (data-zdown / data-acc) and a click sends
//      HTMotorAccess.send(<button>, {fields}) with exactly the screen values teachFieldValue trusts, from golden's list:
//      Set All = the arm's sixteen setEditZ<arm>A..P, All Down = SetEditPickOutSht + setEditZ2A..P.
//   4. a Set All reply (ack.edits) fills those boxes and marks them data-src cpp-pos; an All Down reply, an error or a reply
//      without edits changes nothing.
//   5. a catalog without the three rows greys them with the reason (teachMarkUnwiredEl), no binding.
//  argv[2] = web/page.  CONTROL: W906_TEACH_ZDOWN_JS / W906_TEACH_HTML pointing at copies without the binding / the ack hook /
//  the list filter must go red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const jsFile = process.env.W906_TEACH_ZDOWN_JS || path.join(pageDir, 'ht9045_teach_zdown_c.js');
const htmlFile = process.env.W906_TEACH_HTML || path.join(pageDir, 'HW.teach.html');
const catFile = path.join(pageDir, '..', 'JSON', 'motor-access.json');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
const settle = () => new Promise(r => setImmediate(r));
const THREE = ['btnSetAllInArmZ', 'btnSetAllOutArmZ', 'btnOutZAllDown'];
const LET = 'ACEGIKMOBDFHJLNP';

// ---- fake DOM --------------------------------------------------------------
class El {
  constructor(id, tag) {
    this.id = id || ''; this.tagName = String(tag || 'div').toUpperCase(); this.attrs = {}; this.style = {}; this.value = '';
    this.disabled = false; this.listeners = []; this.children = []; this.textContent = '';
    const cls = new Set();
    this.classList = { add(c) { cls.add(c); }, remove(c) { cls.delete(c); }, contains(c) { return cls.has(c); }, toggle(c, on) { if (on) cls.add(c); else cls.delete(c); } };
  }
  getAttribute(k) { return Object.prototype.hasOwnProperty.call(this.attrs, k) ? this.attrs[k] : null; }
  setAttribute(k, v) { this.attrs[k] = String(v); }
  removeAttribute(k) { delete this.attrs[k]; }
  addEventListener(t, fn, cap) { this.listeners.push({ t, fn, cap: !!cap }); }
  removeEventListener(t, fn, cap) { this.listeners = this.listeners.filter(l => !(l.t === t && l.fn === fn && l.cap === !!cap)); }
  appendChild(c) { this.children.push(c); return c; }
  querySelector() { return null; }
  click() {                                   // target phase: capture listeners first, then the others, in registration order
    if (this.disabled) return;
    let stop = false;
    const ev = { type: 'click', target: this, stopImmediatePropagation() { stop = true; }, stopPropagation() {}, preventDefault() {} };
    const order = this.listeners.filter(l => l.t === 'click' && l.cap).concat(this.listeners.filter(l => l.t === 'click' && !l.cap));
    for (const l of order) { if (stop) break; l.fn(ev); }
  }
}

function makeWorld(catalog) {
  const ids = {};
  const mk = (id, tag) => { ids[id] = new El(id, tag); return ids[id]; };
  THREE.forEach(id => mk(id, 'button'));
  mk('btnSetAllSortArmZ', 'button');
  mk('SetEditPickOutSht', 'input');
  for (const arm of [1, 2]) for (const c of LET) mk('setEditZ' + arm + c, 'input');
  const docListeners = [];
  const document = {
    readyState: 'loading', head: new El('head', 'head'), body: new El('body', 'body'), documentElement: new El('html', 'html'),
    getElementById: id => ids[id] || null,
    querySelector: () => null, querySelectorAll: () => [],
    createElement: t => new El('', t),
    addEventListener(t, fn) { docListeners.push({ t, fn }); },
  };
  const sends = [], infos = [], greys = [], timers = [];
  // the values the page trusts (teachFieldValue: loaded from C++ / typed); every other box -> null (not sent)
  const trusted = { SetEditPickOutSht: 5000, setEditZ2A: 10, setEditZ2C: -3, setEditZ1A: 1, setEditZ1E: 7 };
  const sb = {
    document, console: { log() {}, warn() {}, info() {}, error() {} }, location: { search: '' },
    setTimeout(fn, ms) { timers.push({ fn, ms: ms || 0 }); return timers.length; }, clearTimeout() {},
    addEventListener() {},
    teachSetInfo(m, c) { infos.push([m, c]); },
    teachFieldValue(id) { return Object.prototype.hasOwnProperty.call(trusted, id) ? trusted[id] : null; },
    teachMarkUnwiredEl(el, why) { if (!el) return; el.classList.add('teach-unwired'); el.setAttribute('data-unwired', why); greys.push([el.id, why]); },
    HTMotorAccess: {
      debug() { return { hasCatalog: true, source: 'uteach' }; },
      loadJson() { return Promise.resolve(JSON.parse(JSON.stringify(catalog))); },
      send(btn, p) { sends.push({ btn, p: JSON.parse(JSON.stringify(p || {})) }); return { state: 'requested' }; },
    },
  };
  sb.window = sb;
  vm.createContext(sb);
  const fireDoc = t => docListeners.filter(l => l.t === t).forEach(l => l.fn({ type: t }));
  return { ids, sb, document, sends, infos, greys, timers, fireDoc };
}

(async function main() {
  const html = fs.readFileSync(htmlFile, 'utf8');
  const catalog = JSON.parse(fs.readFileSync(catFile, 'utf8'));

  console.log('-- 1. HW.teach.html loads the file and routes its replies');
  check(html.indexOf('<script src="ht9045_teach_zdown_c.js"></script>') >= 0, 'HW.teach.html has <script src="ht9045_teach_zdown_c.js"></script>');
  const onAckAt = html.indexOf('function teachOnAck(');
  const hook = "if(req.action==='teachSetAllArmZ'||req.action==='teachOutZAllDown'){ if(window.HT9045TeachZDown) HT9045TeachZDown.onAck(req,ack,err); return; }";
  const hookAt = html.indexOf(hook, onAckAt);
  const filterAt = html.indexOf("if(req.action!=='teachSet' && req.action!=='teachGo') return;", onAckAt);
  check(onAckAt >= 0 && hookAt > onAckAt && filterAt > hookAt, 'teachOnAck hands teachSetAllArmZ / teachOutZAllDown to HT9045TeachZDown.onAck before its teachSet / teachGo filter');

  console.log('-- 2. the page no longer greys the three (its own grey code, cut from the page)');
  const g0 = html.indexOf('var TEACH_UNWIRED_CSS='), g1 = html.indexOf('teachMarkUnwired();', g0);
  const greyCode = (g0 >= 0 && g1 > g0) ? html.slice(g0, html.indexOf('\n', g1)) : '';
  check(greyCode.indexOf('function teachMarkUnwiredEl(') >= 0, 'cut the real TEACH_UNWIRED grey code out of HW.teach.html');
  {
    const w = makeWorld(catalog);
    delete w.sb.teachMarkUnwiredEl;                                // the page defines its own
    if (greyCode) vm.runInContext(greyCode, w.sb, { filename: 'HW.teach.html#TEACH_UNWIRED' });
    w.document.readyState = 'interactive';
    w.fireDoc('DOMContentLoaded');
    const greyed = id => w.ids[id].classList.contains('teach-unwired') || w.ids[id].getAttribute('data-unwired') !== null;
    check(greyed('btnSetAllSortArmZ'), 'control: the grey ran (btnSetAllSortArmZ is still greyed, golden Sort Z not wired)');
    check(THREE.every(id => !greyed(id)), 'btnSetAllInArmZ / btnSetAllOutArmZ / btnOutZAllDown are not greyed  [' + THREE.filter(greyed).join(',') + ']');
    const listed = new Set();
    (w.sb.TEACH_UNWIRED || []).forEach(r => listed.add(r[0]));
    (w.sb.TEACH_UNWIRED_B38 || []).forEach(g => String(g[1]).split(' ').forEach(id => listed.add(id)));
    check(THREE.every(id => !listed.has(id)) && listed.has('btnSetAllSortArmZ') && listed.has('btnSortZAllUp'),
          'neither TEACH_UNWIRED nor TEACH_UNWIRED_B38 lists the three; the two Sort Arm buttons stay listed');
  }

  console.log('-- 3. binding and what each click sends');
  const w = makeWorld(catalog);
  vm.runInContext(fs.readFileSync(jsFile, 'utf8'), w.sb, { filename: path.basename(jsFile) });
  w.document.readyState = 'interactive';
  w.fireDoc('DOMContentLoaded');
  await settle(); await settle();
  const Z = w.sb.HT9045TeachZDown;
  check(Z && typeof Z.onAck === 'function', 'HT9045TeachZDown exported (onAck)');
  check(THREE.every(id => w.ids[id].getAttribute('data-zdown') === '1' && w.ids[id].getAttribute('data-acc') === '1' &&
        w.ids[id].listeners.filter(l => l.t === 'click').length === 1), 'the three buttons are bound once (data-zdown / data-acc, one click listener)');
  check(w.ids.btnSetAllSortArmZ.listeners.length === 0, 'btnSetAllSortArmZ is not bound');
  w.ids.btnOutZAllDown.click();
  w.ids.btnSetAllInArmZ.click();
  w.ids.btnSetAllOutArmZ.click();
  const s = w.sends;
  check(s.length === 3 && s[0].btn === 'btnOutZAllDown' && JSON.stringify(s[0].p) === JSON.stringify({ fields: { SetEditPickOutSht: 5000, setEditZ2A: 10, setEditZ2C: -3 } }),
        'All Down sends {fields: SetEditPickOutSht + the trusted setEditZ2*}  [' + JSON.stringify(s[0] && s[0].p) + ']');
  check(s.length === 3 && s[1].btn === 'btnSetAllInArmZ' && JSON.stringify(s[1].p) === JSON.stringify({ fields: { setEditZ1A: 1, setEditZ1E: 7 } }),
        'Set InArm Z sends {fields: the trusted setEditZ1* (the base setEditZ1E among them)}  [' + JSON.stringify(s[1] && s[1].p) + ']');
  check(s.length === 3 && s[2].btn === 'btnSetAllOutArmZ' && JSON.stringify(s[2].p) === JSON.stringify({ fields: { setEditZ2A: 10, setEditZ2C: -3 } }),
        'Set OutArm Z sends {fields: the trusted setEditZ2*}');
  check(JSON.stringify(Z.FIELDS.btnOutZAllDown) === JSON.stringify(['SetEditPickOutSht'].concat(LET.split('').map(c => 'setEditZ2' + c))) &&
        JSON.stringify(Z.FIELDS.btnSetAllInArmZ) === JSON.stringify(LET.split('').map(c => 'setEditZ1' + c)),
        'the ids asked = golden teInArm / teOutArm order (uteach.cpp:286-290) [+ SetEditPickOutSht]');

  console.log('-- 4. the replies');
  w.ids.setEditZ1A.value = '1'; w.ids.setEditZ1E.value = '7';
  Z.onAck({ button: 'btnSetAllInArmZ', action: 'teachSetAllArmZ' }, { arm: 'in', edits: { setEditZ1A: 993, setEditZ1E: 0 } }, null);
  check(w.ids.setEditZ1A.value === '993' && w.ids.setEditZ1E.value === '0' && w.ids.setEditZ1A.getAttribute('data-src') === 'cpp-pos' &&
        w.ids.setEditZ1E.getAttribute('data-src') === 'cpp-pos' && w.ids.setEditZ1C.getAttribute('data-src') === null,
        'a Set All reply fills exactly the boxes it names, data-src cpp-pos');
  check(w.infos.some(x => x[1] === 'ok' && /存檔/.test(x[0])), 'and says it only changed the screen (save to keep it)');
  w.ids.setEditZ2A.value = '10';
  Z.onAck({ button: 'btnOutZAllDown', action: 'teachOutZAllDown' }, { edits: { setEditZ2A: 1 } }, null);
  Z.onAck({ button: 'btnSetAllOutArmZ', action: 'teachSetAllArmZ' }, null, new Error('refused'));
  Z.onAck({ button: 'btnSetAllOutArmZ', action: 'teachSetAllArmZ' }, { arm: 'out' }, null);
  check(w.ids.setEditZ2A.value === '10' && w.ids.setEditZ2A.getAttribute('data-src') === null,
        'an All Down reply, a refusal and a reply without edits change no box');

  console.log('-- 5. a catalog without the rows');
  const cat2 = JSON.parse(JSON.stringify(catalog));
  cat2.commands = cat2.commands.filter(c => THREE.indexOf(c.button) < 0);
  const w2 = makeWorld(cat2);
  vm.runInContext(fs.readFileSync(jsFile, 'utf8'), w2.sb, { filename: path.basename(jsFile) });
  w2.document.readyState = 'interactive';
  w2.fireDoc('DOMContentLoaded');
  await settle(); await settle();
  check(THREE.every(id => w2.greys.some(g => g[0] === id && /motor-access\.json/.test(g[1])) && w2.ids[id].listeners.length === 0),
        'without the catalog rows: greyed with the reason, not bound');

  console.log('\nRESULT: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail === 0 ? 0 : 1);
})().catch(e => { console.log('FATAL ' + (e && e.stack || e)); process.exit(2); });
