'use strict';
const heCI = require('./htmledit');
// AI(W906-HTDESIGNER) 20260930: 網頁事件 -- a web page's own JS event handlers, typed into the properties panel
// (EastSun: "網路js監聽器也要讓我填function"、"我改了之後系統都要自動幫我生成更改後的程式碼"). The code the
// designer writes lives in ONE block at the end of the page's <body>, which it keeps up to date:
//
//   <script id="htdEvents">
//   /* ...designer's note... */
//   function spbSaveClick(e) {
//     // TODO
//   }
//   document.getElementById('spbSave').addEventListener('click', spbSaveClick);
//   </script>
//
// Typing a name adds the function (an empty one) and the binding; a new name renames it (the function and every
// binding of it); clearing it takes the binding out, and the function too when it is still the empty one and
// nothing else uses it. A renamed component's bindings follow (renameIdEdits). Plain Node; edits are
// { s, e, text } on the page's text, never overlapping.

const BLOCK_ID = 'htdEvents';
const RESERVED = new Set(('break case catch class const continue debugger default delete do else enum export extends false finally for function if import in instanceof new null return super switch this throw true try typeof var void while with yield let static implements interface package private protected public await arguments eval ' +
  'document window location top self parent frames history navigator screen name length status opener closed event NaN undefined Infinity globalThis htdCpp').split(' '));
const NOTE = '/* HTML 視覺設計工具：設計檢視新增的網頁事件。這一段由工具維護——在屬性面板打函式名稱＝新增、改名稱＝這裡跟著改、清掉＝拿掉；函式裡面的程式自己寫。 */';
const STUB_BODY = '  // TODO';
const IDENT = /^[A-Za-z_$][\w$]*$/;
// the DOM events offered (the ones a BCB6 form's controls have on a web page)
const TYPES = ['click', 'dblclick', 'change', 'input', 'mousedown', 'mouseup', 'mouseenter', 'mouseleave', 'keydown', 'keyup', 'focus', 'blur'];

function eolOf(text) { return /\r\n/.test(String(text || '')) ? '\r\n' : '\n'; }

/** The designer's block: { start, innerStart, innerEnd, end } (offsets in the page), or null. */
function findBlock(html) {
  const t = String(html || '');
  const m = new RegExp('<script\\b[^>]*\\bid\\s*=\\s*["\']' + BLOCK_ID + '["\'][^>]*>', 'i').exec(t);
  if (!m) return null;
  const innerStart = m.index + m[0].length;
  const close = heCI.findCI(t, '</script', innerStart);
  if (close < 0) return null;
  const gt = t.indexOf('>', close);
  return { start: m.index, innerStart, innerEnd: close, end: gt < 0 ? t.length : gt + 1 };
}

const BIND_RE = /^[ \t]*document\.getElementById\(\s*(['"])([^'"]+)\1\s*\)\.addEventListener\(\s*(['"])(\w+)\3\s*,\s*([A-Za-z_$][\w$]*)\s*\)\s*;?[ \t]*(\r?\n)?/gm;

/** The bindings in the block: [{ id, type, fn, s, e }] (a whole line each, its line end included). */
function bindings(html) {
  const t = String(html || '');
  const b = findBlock(t);
  if (!b) return [];
  const inner = t.slice(b.innerStart, b.innerEnd);
  const out = [];
  BIND_RE.lastIndex = 0;
  let m;
  while ((m = BIND_RE.exec(inner))) out.push({ id: m[2], type: m[4], fn: m[5], s: b.innerStart + m.index, e: b.innerStart + m.index + m[0].length });
  return out;
}

/** A function of the block: { name, s, e (after its closing brace and line end), body, empty } or null. */
function fnOf(html, name) {
  const t = String(html || '');
  const b = findBlock(t);
  if (!b) return null;
  const re = new RegExp('^[ \\t]*function\\s+' + name.replace(/\$/g, '\\$') + '\\s*\\([^)]*\\)\\s*\\{', 'm');
  const inner = t.slice(b.innerStart, b.innerEnd);
  const m = re.exec(inner);
  if (!m) return null;
  let depth = 0, i = m.index + m[0].length - 1, q = null;
  for (; i < inner.length; i++) {
    const c = inner[i];
    if (q) { if (c === '\\') { i++; continue; } if (c === q) q = null; continue; }
    if (c === '"' || c === "'" || c === '`') { q = c; continue; }
    if (c === '/' && inner[i + 1] === '/') { const n = inner.indexOf('\n', i); i = n < 0 ? inner.length : n; continue; }
    if (c === '/' && inner[i + 1] === '*') { const n = inner.indexOf('*/', i + 2); i = n < 0 ? inner.length : n + 1; continue; }
    if (c === '{') depth++;
    else if (c === '}') { depth--; if (!depth) break; }
  }
  let e = i + 1;
  if (inner[e] === '\r') e++;
  if (inner[e] === '\n') e++;
  const body = inner.slice(m.index + m[0].length, i);
  return { name, s: b.innerStart + m.index, e: b.innerStart + e, body, empty: body.replace(/\s+/g, '') === '//TODO' || !body.trim() };
}

/** Where the new function's "// TODO" goes after the edits: the line's offset in the new text (for the cursor). */
function stub(fn, eol) {
  return 'function ' + fn + '(e) {' + eol + STUB_BODY + eol + '}' + eol;
}

/**
 * Set the handler of `type` on element `id` to `fn` ('' = none). -> { edits, error?, fnAdded?, renamed? }
 */
/** 1009 second review (wiring #6): does `text` define a function / variable `fn` (function fn( / var|let|const fn =)? */
function definesFn(text, fn) {
  const e = String(fn || '').replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  if (!e) return false;
  return new RegExp('(^|[^\\w$.])function\\s+' + e + '\\s*\\(|\\b(?:var|let|const)\\s+' + e + '\\s*=').test(String(text || ''));
}

function setBinding0(html, id, type, fn, opts) {
  const t = String(html || '');
  const eol = eolOf(t);
  fn = String(fn || '').trim();
  if (fn && !IDENT.test(fn)) return { edits: [], error: '函式名稱只能用英文字母、數字、_、$，不能用數字開頭' };
  // (1007 audit, events P3: a reserved word made the block a syntax error, a read-only global (document, window, top...)
  //  threw when the page loaded)
  if (fn && RESERVED.has(fn)) return { edits: [], error: '「' + fn + '」是 JavaScript 的保留字或瀏覽器的全域名稱，不能當函式名稱' };
  if (!IDENT.test(String(type || ''))) return { edits: [], error: '事件名稱不對：' + type };
  const binds = bindings(t);
  const cur = binds.find(x => x.id === id && x.type === type);
  // (1009 second review (wiring #6): the page's own function of that name (outside the designer's block, or in a script
  //  file it loads: opts.existing) -- bound to as it is; no empty one added over it)
  const b0 = findBlock(t);
  const outside = !!fn && (!!(opts && opts.existing) || definesFn(b0 ? t.slice(0, b0.start) + t.slice(b0.end) : t, fn));
  const line = 'document.getElementById(\'' + id + '\').addEventListener(\'' + type + '\', ' + fn + ');' + eol;
  const b = findBlock(t);
  if (!b) {
    if (!fn) return { edits: [] };
    const block = '<script id="' + BLOCK_ID + '">' + eol + NOTE + eol + (outside ? '' : stub(fn, eol)) + line + '</script>' + eol;
    const bodyEnd = t.toLowerCase().lastIndexOf('</body');
    const at = bodyEnd >= 0 ? bodyEnd : t.length;
    return { edits: [{ s: at, e: at, text: block }], fnAdded: outside ? null : fn, existing: outside || undefined };
  }
  if (cur && cur.fn === fn) return { edits: [] };
  const edits = [];
  if (!fn) {
    // take the binding out; the function too when it is still empty and nothing else uses it
    edits.push({ s: cur ? cur.s : 0, e: cur ? cur.e : 0, text: '' });
    if (!cur) return { edits: [] };
    const others = binds.filter(x => x !== cur && x.fn === cur.fn);
    const f = fnOf(t, cur.fn);
    if (f && f.empty && !others.length) edits.push({ s: f.s, e: f.e, text: '' });
    return { edits: edits.sort((a, c) => a.s - c.s) };
  }
  if (cur) {
    const others = binds.filter(x => x !== cur && x.fn === cur.fn);
    const oldF = fnOf(t, cur.fn);
    const newF = fnOf(t, fn);
    if (oldF && !others.length && !newF && !outside) {
      // only this one uses it: the function is renamed (its code stays)
      const head = /function\s+[A-Za-z_$][\w$]*/.exec(t.slice(oldF.s, oldF.e));
      const at = oldF.s + head.index;
      edits.push({ s: at, e: at + head[0].length, text: 'function ' + fn });
      edits.push({ s: cur.s, e: cur.e, text: line });
      return { edits: edits.sort((a, c) => a.s - c.s), renamed: cur.fn };
    }
    // (the old function stays: something else uses it, or the new name has its own -- 1007 audit, events P3: unless it is
    //  still empty and nothing else uses it: renamed onto an existing function, the old empty one was left behind)
    const ed2 = [{ s: cur.s, e: cur.e, text: (newF || outside ? '' : stub(fn, eol)) + line }];
    if (newF && oldF && oldF.empty && !others.length && (oldF.e <= cur.s || oldF.s >= cur.e)) ed2.push({ s: oldF.s, e: oldF.e, text: '' });
    return { edits: ed2.sort((a, c) => a.s - c.s), fnAdded: newF || outside ? null : fn, existing: outside || undefined };
  }
  // a new binding: its function (unless the block has one of that name) and the line, at the end of the block
  const text = (fnOf(t, fn) || outside ? '' : stub(fn, eol)) + line;
  let at = b.innerEnd;
  while (at > b.innerStart && (t[at - 1] === ' ' || t[at - 1] === '\t')) at--;
  const needEol = at > b.innerStart && t[at - 1] !== '\n';
  return { edits: [{ s: at, e: at, text: (needEol ? eol : '') + text }], fnAdded: fnOf(t, fn) || outside ? null : fn, existing: outside || undefined };
}

// ---------------------------------------------------------------------------
// C++ 事件 (EastSun 20260930: "如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增"): a C++ event handler the
// designer added is sent from the same block, one line each --
//   htdCpp('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveMouseUp', 'OnMouseUp');
// -- and ONE helper function (htdCpp) that sends WS htd.event through the page's client (the loaded script
// object that has rawCmd), like the page's own senders do (token first, a failure said on the page).
// ---------------------------------------------------------------------------
// (1007 audit, events P1: a handler for an event the web has no counterpart of -- OnCanResize, OnResize, OnDragDrop... --
//  or one that cannot be wired yet is kept as a COMMENTED line, type 'none': // htdCpp('Panel1', 'none', 'TfHotPlate',
//  'Panel1CanResize', 'OnCanResize'); -- nothing is sent, but the event grid finds its handler again (it used to show the
//  row empty after a re-select: a second handler was made, 重設 did nothing, a rename lost it))
const CPP_RE = /^[ \t]*(\/\/[ \t]*)?htdCpp\(\s*'([^']+)'\s*,\s*'(\w+)'\s*,\s*'(\w+)'\s*,\s*'(\w+)'\s*,\s*'(On\w+)'\s*\)\s*;?[ \t]*(\r?\n)?/gm;
const HELPER_NAME = 'htdCpp';

function helperText(eol) {
  return [
    '/* 送到 C++：WS htd.event → 伺服器 → C++ 表單的事件函式（設計工具產生，不要改名；每個事件一行在下面） */',
    'function htdCpp(id, type, form, handler, event) {',
    '  var el = document.getElementById(id);',
    '  if (!el) return;',
    '  el.addEventListener(type, function (e) {',
    '    var R = htdCpp.client;',
    '    if (!R) { for (var k in window) { try { var o = window[k]; if (o && o !== window && typeof o.rawCmd === \'function\') { R = htdCpp.client = o; break; } } catch (x) { /* next */ } } }',
    '    var say = function (msg) {',
    '      if (window.console) console.warn(\'[htd.event] \' + form + \'::\' + handler + \'：\' + msg);',
    '      var d = document.createElement(\'div\');',
    '      d.textContent = form + \'::\' + handler + \' 沒有執行：\' + msg;',
    '      d.style.cssText = \'position:fixed;left:8px;bottom:8px;z-index:99999;padding:6px 10px;background:#fff3cd;color:#000;border:1px solid #c90;font:12px sans-serif;\';',
    '      document.body.appendChild(d);',
    '      setTimeout(function () { if (d.parentNode) d.parentNode.removeChild(d); }, 5000);',
    '    };',
    '    if (!R) { say(\'這一頁沒有載入送命令的程式（rawCmd）\'); return; }',
    '    var v = { form: form, handler: handler, control: id, event: event, button: e.button || 0, x: e.offsetX || 0, y: e.offsetY || 0,',
    '      shift: !!e.shiftKey, ctrl: !!e.ctrlKey, alt: !!e.altKey, dbl: e.type === \'dblclick\',',
    '      key: e.keyCode || 0, chr: e.key && e.key.length === 1 ? e.key.charCodeAt(0) : 0 };',
    '    var extra = { tag: (location.pathname.split(\'/\').pop() || \'\').replace(/\\.html?$/i, \'\'), value: JSON.stringify(v) };',
    '    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();',
    '    pre.then(function () { return R.rawCmd(\'htd.event\', extra); }).then(null, function (err) { say((err && err.message) || String(err)); });',
    '  });',
    '}',
    '',
  ].join(eol);
}

/** The C++ event lines of the block: [{ id, type, form, handler, event, s, e }]. */
function cppLines(html) {
  const t = String(html || '');
  const b = findBlock(t);
  if (!b) return [];
  const inner = t.slice(b.innerStart, b.innerEnd);
  const out = [];
  CPP_RE.lastIndex = 0;
  let m;
  while ((m = CPP_RE.exec(inner))) out.push({ id: m[2], type: m[3], form: m[4], handler: m[5], event: m[6], only: !!m[1], s: b.innerStart + m.index, e: b.innerStart + m.index + m[0].length });
  return out;
}

/**
 * Send `event` of element `id` (on DOM `type`) to C++ `form::handler`; handler '' = no more. The helper is added
 * once. -> { edits, error? }
 */
function setCpp0(html, id, type, form, handler, event) {
  const t = String(html || '');
  const eol = eolOf(t);
  if (!IDENT.test(String(type || '')) || !/^\w+$/.test(String(form || '')) || !/^On\w+$/.test(String(event || ''))) return { edits: [], error: '事件不對：' + type + ' / ' + event };
  if (handler && !/^[A-Za-z_]\w*$/.test(handler)) return { edits: [], error: '函式名稱不對：' + handler };
  const only = type === 'none';
  const line = handler ? (only ? '// ' : '') + 'htdCpp(\'' + id + '\', \'' + type + '\', \'' + form + '\', \'' + handler + '\', \'' + event + '\');' + eol : '';
  const b = findBlock(t);
  if (!b) {
    if (!handler) return { edits: [] };
    const block = '<script id="' + BLOCK_ID + '">' + eol + NOTE + eol + (only ? '' : helperText(eol)) + line + '</script>' + eol;
    const bodyEnd = t.toLowerCase().lastIndexOf('</body');
    const at = bodyEnd >= 0 ? bodyEnd : t.length;
    return { edits: [{ s: at, e: at, text: block }] };
  }
  const hasHelper = only || new RegExp('^[ \\t]*function\\s+' + HELPER_NAME + '\\s*\\(', 'm').test(t.slice(b.innerStart, b.innerEnd));
  const cur = cppLines(t).find(x => x.id === id && x.event === event);
  // (1009 review (web events #1): a block made with only a commented line has no helper -- that line made live called
  //  htdCpp, which was not there: the whole block threw at load, the page's own events in it too)
  if (cur) return cur.handler === handler && cur.type === type && cur.form === form ? { edits: [] } : { edits: [{ s: cur.s, e: cur.e, text: (handler && !hasHelper ? helperText(eol) : '') + line }] };
  if (!handler) return { edits: [] };
  let at = b.innerEnd;
  while (at > b.innerStart && (t[at - 1] === ' ' || t[at - 1] === '\t')) at--;
  const needEol = at > b.innerStart && t[at - 1] !== '\n';
  return { edits: [{ s: at, e: at, text: (needEol ? eol : '') + (hasHelper ? '' : helperText(eol)) + line }] };
}

/**
 * 1009 review (web events #2 / #3): the DOM event a BCB6 event is on THIS element -- by what the page made of it:
 *   a check box / radio button (<label> around the <input>) and a RadioGroup (<fieldset class="rg">): OnClick = change
 *     (click on the label fired twice -- the label's own and the one it passes to the input -- and on the group's empty
 *     space without a choice);
 *   a ComboBox (<select>): OnClick / OnSelect = change (click fires when the list opens, never for the keyboard);
 *   OnEnter / OnExit = focusin / focusout (focus / blur do not bubble up to a label / fieldset).
 * null = the general table (lib/format.js DOM_OF).
 */
function domTypeFor(html, id, evName) {
  const t = String(html || '');
  if (evName === 'OnEnter') return 'focusin';
  if (evName === 'OnExit') return 'focusout';
  if (evName !== 'OnClick' && evName !== 'OnSelect') return null;
  const m = new RegExp('<([A-Za-z][\\w-]*)\\b[^>]*\\sid\\s*=\\s*["\']' + String(id).replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + '["\'][^>]*>', 'i').exec(t);
  if (!m) return null;
  const tag = m[1].toLowerCase(), st = m[0];
  if (tag === 'select') return 'change';
  if (evName === 'OnSelect') return null;
  if (tag === 'fieldset' && /\bclass\s*=\s*["'][^"']*\brg\b/.test(st)) return 'change';
  if (tag === 'label') {
    const after = t.slice(m.index + st.length, m.index + st.length + 400);
    if (/^\s*<input\b[^>]*\btype\s*=\s*["']?(checkbox|radio)/i.test(after)) return 'change';
  }
  return null;
}

/** A C++ handler renamed: its lines follow. -> edits */
function renameCppEdits(html, form, oldName, newName) {
  const t = String(html || '');
  const out = [];
  for (const x of cppLines(t)) {
    if (x.form !== form || x.handler !== oldName) continue;
    const seg = t.slice(x.s, x.e);
    // (1009 review (web events #7): the 4th field (id, type, form, HANDLER, event) -- a handler named "change" renamed the
    //  DOM type, one named like its component renamed the id)
    const fm = /^([^(]*\(\s*'[^']*'\s*,\s*'[^']*'\s*,\s*'[^']*'\s*,\s*')([^']*)'/.exec(seg);
    if (fm && fm[2] === oldName) out.push({ s: x.s + fm[1].length, e: x.s + fm[1].length + oldName.length, text: newName });
  }
  return out;
}

/** A component renamed: its bindings in the block follow (the JS ones and the C++ ones). -> edits */
function renameIdEdits(html, oldId, newId) {
  const t = String(html || '');
  const out = [];
  for (const x of bindings(t)) {
    if (x.id !== oldId) continue;
    const seg = t.slice(x.s, x.e);
    const m = /getElementById\(\s*(['"])([^'"]+)\1/.exec(seg);
    const at = x.s + m.index + m[0].indexOf(m[2], m[0].indexOf(m[1]));
    out.push({ s: at, e: at + oldId.length, text: newId });
  }
  for (const x of cppLines(t)) {
    if (x.id !== oldId) continue;
    const i = t.slice(x.s, x.e).indexOf('\'' + oldId + '\'');
    if (i >= 0) out.push({ s: x.s + i + 1, e: x.s + i + 1 + oldId.length, text: newId });
  }
  return out.sort((a, b) => a.s - b.s);
}

/** The handlers of element `id`: { type: { fn, s (the function's offset, or the binding's) } }. */
function handlersOf(html, id) {
  const out = {};
  for (const x of bindings(html)) {
    if (x.id !== id) continue;
    const f = fnOf(html, x.fn);
    out[x.type] = { fn: x.fn, at: f ? f.s : x.s };
  }
  return out;
}

/**
 * 1007 audit (events P2): the last binding / C++ line taken out -- the block (its note, the helper) goes too, so an add
 * and a remove give the page back byte for byte (an empty block with its note used to stay on every page).
 */
function dropEmptyBlock(html, res) {
  const t = String(html || '');
  if (!res || res.error || !res.edits || !res.edits.length) return res;
  const b = findBlock(t);
  if (!b || !res.edits.every(x => x.s >= b.innerStart && x.e <= b.innerEnd)) return res;
  let after = t;
  for (const x of res.edits.slice().sort((p, q) => q.s - p.s)) after = after.slice(0, x.s) + x.text + after.slice(x.e);
  const b2 = findBlock(after);
  if (!b2) return res;
  let inner = after.slice(b2.innerStart, b2.innerEnd);
  for (const eol of ['\r\n', '\n']) inner = inner.split(helperText(eol)).join('');
  inner = inner.split(NOTE).join('');
  if (inner.trim()) return res;
  const eol = eolOf(t);
  const e = t.startsWith(eol, b.end) ? b.end + eol.length : b.end;
  // (1009 review (web events #9): the command client the designer put right before the block (data-htd-client) goes too)
  let s0 = b.start;
  const before = t.slice(Math.max(0, b.start - 400), b.start);
  const cm = /<script\b[^>]*\bdata-htd-client\b[^>]*>\s*<\/script>\r?\n?$/i.exec(before);
  if (cm) s0 = b.start - (before.length - cm.index);
  return Object.assign({}, res, { edits: [{ s: s0, e, text: '' }], blockGone: true });
}
function setCpp(html, id, type, form, handler, event) { return dropEmptyBlock(html, setCpp0(html, id, type, form, handler, event)); }
function setBinding(html, ...rest) { return dropEmptyBlock(html, setBinding0(html, ...rest)); }

module.exports = { domTypeFor, BLOCK_ID, TYPES, findBlock, bindings, fnOf, definesFn, setBinding, renameIdEdits, handlersOf, cppLines, setCpp, renameCppEdits, helperText };
