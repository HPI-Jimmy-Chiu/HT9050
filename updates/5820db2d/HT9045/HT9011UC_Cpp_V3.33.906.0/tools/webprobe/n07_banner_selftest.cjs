// n07_banner_selftest.cjs -- ctest St02_N07BannerPage (node, offline): web/page/ht9045_n07_banner.js, the main-page
// N07 "SECS DISCONNECTED" frame (golden 906_0625_Steven main.cpp:20974-20993).  AI(W906-C15-N07-UI) 20261003 (St02-E).
// Usage: node n07_banner_selftest.cjs <web/page dir>.  Control: W906_N07_BANNER_JS=<an empty .js> must make it red.
// Fake DOM rule (St02 workflow skill section 5 item 20): every property the checks read is set when the DOM is built;
// page pins look for the full <script src="..."></script> tag, not the bare file name.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const jsFile = process.env.W906_N07_BANNER_JS || path.join(pageDir, 'ht9045_n07_banner.js');
let pass = 0, fail = 0;
function check(ok, what) { if (ok) { pass++; console.log('  ok   ' + what); } else { fail++; console.log('  FAIL ' + what); } }

function makeDom() {
  function El(tag) {
    this.tagName = String(tag).toUpperCase(); this.children = []; this.style = {}; this.title = ''; this.className = '';
    this.id = ''; this.textContent = ''; this.parent = null;
  }
  El.prototype.appendChild = function (c) { this.children.push(c); c.parent = this; return c; };
  El.prototype.removeChild = function (c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parent = null; return c; };
  const pane = new El('div'); pane.className = 'statusPane';
  const fail = new El('div'); fail.id = 'labFailAlarmCnt'; fail.textContent = '---'; pane.appendChild(fail);   // St01's two children (main.html :168 / :173)
  const state = new El('div'); state.id = 'palMainStatus'; state.textContent = '---'; pane.appendChild(state);
  const dom = { El, pane };
  dom.document = {
    readyState: 'complete',
    addEventListener() {},
    querySelector(sel) { return sel === 'div.statusPane' ? pane : null; },
    createElement(t) { return new El(t); }
  };
  return dom;
}

function load(tags) {
  const dom = makeDom();
  const subs = [];
  const timers = [];
  const sb = {
    console: { log() {}, info() {}, warn() {}, error() {} }, document: dom.document, Promise,
    setInterval(fn, ms) { const t = { fn, ms, live: true }; timers.push(t); return t; },
    clearInterval(t) { if (t) t.live = false; }
  };
  sb.window = sb;
  if (tags) {
    sb.HT9045Tags = {
      has(k) { return k in tags; }, get(k) { return (k in tags) ? tags[k] : null; },
      subscribe(fn) { subs.push(fn); return () => {}; }, connect() { return Promise.resolve(true); }
    };
  }
  vm.createContext(sb);
  let err = null;
  try { vm.runInContext(fs.readFileSync(jsFile, 'utf8'), sb, { filename: path.basename(jsFile) }); } catch (e) { err = e; }
  const push = (changed) => { Object.assign(tags, changed); subs.forEach(f => f(changed, tags)); };
  const live = () => timers.filter(t => t.live);
  const tick = () => live().forEach(t => t.fn());
  return { sb, dom, err, api: sb.HT9045N07Banner, push, live, tick };
}
const label = (v) => v.dom.pane.children.filter(c => c.id === 'labTesterMode');

console.log('-- 1. offline / null / false: nothing is drawn');
{
  const v = load(null);
  check(v.err === null && !!v.api, 'the script loads and exports HT9045N07Banner');
  check(label(v).length === 0 && !v.dom.pane.style.borderWidth && v.live().length === 0, 'offline (no HT9045Tags): no frame, no label, no timer');
  const w = load({ 'n07.alarm': null });
  check(label(w).length === 0 && !w.dom.pane.style.borderWidth, 'n07.alarm null ([N07] off): nothing');
  const f = load({ 'n07.alarm': false });
  check(label(f).length === 0 && !f.dom.pane.style.borderWidth, 'n07.alarm false: nothing');
}

console.log('-- 2. alarm: golden main.cpp:20976-20980');
{
  const v = load({ 'n07.alarm': false });
  v.push({ 'n07.alarm': true });
  const l = label(v);
  check(l.length === 1 && l[0].textContent === 'SECS DISCONNECTED' && l[0].style.color === '#FF0000', 'labTesterMode "SECS DISCONNECTED" in red');
  check(v.dom.pane.style.borderWidth === '20px' && v.dom.pane.style.borderStyle === 'solid', 'Off_lineDisplay BorderWidth 20');
  check(v.dom.pane.style.borderColor === '#FF0000', 'first phase red');
  check(v.live().length === 1 && v.live()[0].ms === 270, 'one blink timer at 270 ms (golden Timer1 30 ms x 9, main.cpp:3182-3184)');
  v.tick();
  check(v.dom.pane.style.borderColor === '#F0F0F0', 'blink: clBtnFace');
  v.tick();
  check(v.dom.pane.style.borderColor === '#FF0000', 'blink: clRed again');
  v.push({ 'n07.alarm': true });
  check(label(v).length === 1 && v.live().length === 1, 'a repeated true adds no second label or timer');
  check(v.dom.pane.children.some(c => c.id === 'palMainStatus' && c.textContent === '---') &&
        v.dom.pane.children.some(c => c.id === 'labFailAlarmCnt' && c.textContent === '---'), 'St01\'s #palMainStatus / #labFailAlarmCnt are left alone (St01 07:32 iii)');
  check(v.dom.pane.children.length === 3 && v.dom.pane.children[2].id === 'labTesterMode', 'the banner is its own element after them, not over or instead of palMainStatus');
}

console.log('-- 3. release: golden main.cpp:20987-20993');
{
  const v = load({ 'n07.alarm': true });
  check(label(v).length === 1, 'shown at start when the tag is already true');
  v.push({ 'n07.alarm': false });
  check(label(v).length === 0 && v.live().length === 0 && v.dom.pane.children.length === 2, 'false: label removed, timer stopped, only St01\'s two children left');
  check(v.dom.pane.style.borderWidth === '' && v.dom.pane.style.borderStyle === '' && v.dom.pane.style.borderColor === '',
        'the pane style is back to what it was');
  v.push({ 'n07.alarm': true });
  v.push({ 'n07.alarm': null });
  check(label(v).length === 0 && v.live().length === 0, 'null releases too ([N07] switched off)');
  v.push({ 'other.tag': 1 });
  check(label(v).length === 0, 'unrelated tags do not redraw it');
}

console.log('-- 4. page / file pins');
{
  const page = fs.readFileSync(path.join(pageDir, 'main.html'));
  const js = fs.readFileSync(jsFile);
  const txt = page.toString('utf8');
  const count = (s, n) => s.split(n).length - 1;
  const at = (f) => txt.indexOf('<script src="' + f + '"></script>');
  check(count(txt, '<script src="ht9045_n07_banner.js"></script>') === 1, 'main.html loads ht9045_n07_banner.js exactly once');
  check(at('ht9045_recipe_client.js') > 0 && at('ht9045_n07_banner.js') > at('ht9045_recipe_client.js'), 'after ht9045_recipe_client.js (HT9045Tags)');
  // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #17 -- no line number / line count pins (every main.html change had to
  //   keep 886 lines); the order is what matters: right after ht9045_nonstop_alarm.js (St01 07:32 i)
  check(at('ht9045_nonstop_alarm.js') > 0 && at('ht9045_n07_banner.js') > at('ht9045_nonstop_alarm.js'), 'after ht9045_nonstop_alarm.js');
  check(count(txt, 'class="statusPane"') === 1, 'main.html has one div.statusPane');
  for (const [name, b] of [['main.html', page], [path.basename(jsFile), js]]) {
    const s = b.toString('latin1');
    const crlf = count(s, '\r\n'), lf = count(s, '\n');
    check(!(b[0] === 0xEF && b[1] === 0xBB && b[2] === 0xBF), name + ': no BOM');
    check(crlf === 0 || crlf === lf, name + ': one line ending (CRLF ' + crlf + ' / LF ' + lf + ')');
  }
}

console.log('St02_N07BannerPage: ' + pass + ' ok, ' + fail + ' failed');
process.exit(fail === 0 ? 0 : 1);
