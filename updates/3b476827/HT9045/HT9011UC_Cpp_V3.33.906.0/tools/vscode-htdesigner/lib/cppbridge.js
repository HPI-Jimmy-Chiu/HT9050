'use strict';
// AI(W906-HTDESIGNER) 20260930: 自動接線 -- a C++ event handler the designer adds is reached from the web page
// with nothing left to do by hand (EastSun 20260930: "如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增";
// "你用json中間連線的 是你這邊要幫我處理 我只需要處理C++的部分"). The chain, like WPF / BCB6's .dfm wiring:
//
//   page   <script id="htdEvents">  htdCpp('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveMouseUp', 'OnMouseUp');
//            -> the page's WS client: rawCmd('htd.event', { tag: <page>, value: '{"form","handler",...}' })
//   server the dispatch branch of htd.event, the same shape as form.event's (one branch, on form.event's line)
//            -> <prefix>HtdEvent(tag, value, &ack, &err)
//   C++    HtdEvents/HtdEvents.gen.cpp (this module writes it): the table of the designer's events -> the form's
//            global object -> fHotPlate->spbSaveMouseUp(sender, button, shift, x, y)
//
// The same rules as form.event: not while the machine runs, under the same form lock. These functions only compute
// text and where it goes; the extension applies it as ONE edit of the documents (nothing saved). Plain Node.

const { mask, classBody } = require('./cppstub');

const CMD = 'htd.event';
const GEN_REL = 'HtdEvents/HtdEvents.gen.cpp';
const TAG = 'AI(W906-HTDESIGNER)';

function eolOf(text) { return /\r\n/.test(String(text || '')) ? '\r\n' : '\n'; }
function lineStart(t, i) { while (i > 0 && t[i - 1] !== '\n') i--; return i; }
function lineEnd(t, i) { const n = t.indexOf('\n', i); return n < 0 ? t.length : (t[n - 1] === '\r' ? n - 1 : n); }

/**
 * The server's dispatch: a branch for htd.event made from form.event's (its checks, its call, its CompleteCommand,
 * with the names changed), put in front of it ON THE SAME LINE (the file's rule: 接在同一行，不移動行號).
 * -> { has: true } (already there) | { edit: { s, e, text }, fn } | { error }
 */
function serverHook(text, stamp) {
  const t = String(text || '');
  if (/\.cmd\s*==\s*"htd\.event"/.test(t)) {
    const f = /extern\s+bool\s+(\w*HtdEvent)\s*\(/.exec(t);
    return { has: true, fn: f ? f[1] : 'HtdEvent' };
  }
  const head = /\}\s*else\s+if\s*\(\s*(\w+)\.cmd\s*==\s*"form\.event"\s*\)\s*\{/.exec(t);
  if (!head) return { error: '伺服器程式裡找不到 form.event 的分派（} else if (x.cmd == "form.event") {）' };
  const s = head.index;
  const le = lineEnd(t, s);
  const next = new RegExp('\\}\\s*else\\s+if\\s*\\(\\s*' + head[1] + '\\.cmd\\s*==', 'g');
  next.lastIndex = s + head[0].length;
  const nx = next.exec(t);
  const e = nx && nx.index < le ? nx.index : le;
  let branch = t.slice(s, e);
  const fe = /extern\s+bool\s+(\w*?)FormEvent\s*\(/.exec(branch);
  if (!fe) return { error: 'form.event 的分派裡找不到 extern bool …FormEvent(…) 的宣告' };
  const fn = fe[1] + 'HtdEvent';
  branch = branch.replace(/\/\*[\s\S]*?\*\//g, ' ').replace(/\/\/.*$/, '')
    .replace(/form\.event/g, 'htd.event')
    .replace(new RegExp('\\b' + fe[1] + 'FormEvent\\b', 'g'), fn)
    .replace(/[ \t]{2,}/g, ' ').replace(/\s+$/, '');
  if (!branch.includes(fn + '(') || !/CompleteCommand\s*\(/.test(branch)) return { error: 'form.event 的分派形狀認不得（沒有呼叫與 CompleteCommand）' };
  const note = '/*' + TAG + ' ' + stamp + '：設計工具的事件（網頁 → C++ 表單的事件函式），同 form.event 的檢查；本體 ' + GEN_REL + '（設計工具產生）；接在同一行，不移動行號*/';
  const open = branch.indexOf('{');
  const text2 = branch.slice(0, open + 1) + ' ' + note + branch.slice(open + 1) + ' ';
  return { edit: { s, e: s, text: text2 }, fn };
}

/**
 * CMake: the server executable's source list gets the generated file, in front of its closing parenthesis (same line).
 * `serverRel`: the server's source as the list names it (tools/wb_serve.cpp). -> { has } | { edit } | { error }
 */
function cmakeHook(text, serverRel, genRel) {
  const t = String(text || '');
  const rel = genRel || GEN_REL;
  const re = /add_executable\s*\(/g;
  let m;
  while ((m = re.exec(t))) {
    // the call's closing parenthesis: # comments and "strings" skipped, $<...> / nested parentheses counted
    let depth = 1, i = m.index + m[0].length, q = false;
    for (; i < t.length && depth; i++) {
      const c = t[i];
      if (q) { if (c === '\\') { i++; continue; } if (c === '"') q = false; continue; }
      if (c === '"') { q = true; continue; }
      if (c === '#') { const n = t.indexOf('\n', i); i = n < 0 ? t.length : n; continue; }
      if (c === '(') depth++;
      else if (c === ')') depth--;
    }
    if (depth) break;
    const close = i - 1;
    const call = t.slice(m.index, close);
    const code = call.replace(/#[^\n]*/g, '');
    if (!new RegExp('(^|[\\s(])' + serverRel.replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + '(\\s|$)').test(code)) continue;
    if (code.includes(rel)) return { has: true };
    return { edit: { s: close, e: close, text: '  ' + rel } };
  }
  return { error: 'CMakeLists.txt 裡找不到列出 ' + serverRel + ' 的 add_executable' };
}

/** The designer's events in the generated file: [{ form, handler, control, event }]. */
function rowsOf(genText) {
  const out = [];
  const re = /^\/\/\s*htd-row\s+(\w+)\s+(\w+)\s+(\w+)\s+(On\w+)\s*$/gm;
  let m;
  while ((m = re.exec(String(genText || '')))) out.push({ form: m[1], handler: m[2], control: m[3], event: m[4] });
  return out;
}

/** The class's global object: `extern TfHotPlate *fHotPlate;` in its .h, else `TfHotPlate *fHotPlate = ...` in its .cpp. */
function globalOf(hText, cppText, cls) {
  const h = new RegExp('\\bextern\\s+' + cls + '\\s*\\*\\s*(\\w+)\\s*;').exec(mask(hText || ''));
  if (h) return h[1];
  const c = new RegExp('^[ \\t]*' + cls + '\\s*\\*\\s*(\\w+)\\s*=', 'm').exec(mask(cppText || ''));
  return c ? c[1] : null;
}

/** public / private / protected at `off` inside the class body (a class starts private). */
function accessAt(maskedText, body, off) {
  const seg = maskedText.slice(body.open + 1, off);
  const re = /\b(public|private|protected)\s*:/g;
  let a = 'private', m;
  while ((m = re.exec(seg))) a = m[1];
  return a;
}

/** The handler's declaration in the class: { params, access } or null. */
function declOf(hText, cls, name) {
  const b = classBody(hText, cls);
  if (!b) return null;
  const m = mask(hText);
  const re = new RegExp('\\bvoid\\s+' + name + '\\s*\\(([^)]*)\\)\\s*;', 'g');
  re.lastIndex = b.open;
  const x = re.exec(m);
  if (!x || x.index > b.close) return null;
  return { params: hText.slice(x.index, x.index + x[0].length).replace(/^[^(]*\(/, '').replace(/\)\s*;$/, '').replace(/\s+/g, ' ').trim(), access: accessAt(m, b, x.index) };
}

/** The class's `void name(params);` declarations: [{ name, params, access }] (the value list of the Events tab). */
function methodsOf(hText, cls) {
  const b = classBody(hText, cls);
  if (!b) return [];
  const m = mask(hText);
  const re = /\bvoid\s+(\w+)\s*\(([^)]*)\)\s*;/g;
  re.lastIndex = b.open;
  const out = [];
  let x;
  while ((x = re.exec(m)) && x.index < b.close) {
    const params = hText.slice(x.index, x.index + x[0].length).replace(/^[^(]*\(/, '').replace(/\)\s*;$/, '').replace(/\s+/g, ' ').trim();
    out.push({ name: x[1], params, access: accessAt(m, b, x.index) });
  }
  return out;
}

/** The parameter types only ("TObject*,TMouseButton,TShiftState,int,int"): "a compatible method signature". */
function typesKey(params) {
  return String(params || '').split(',').map(p => p.replace(/\s*([*&])\s*/g, '$1 ').replace(/\s+/g, ' ').trim())
    .filter(Boolean).map(p => p.replace(/^const /, '').replace(/ ?\w+$/, '').replace(/\s+/g, '')).join(',');
}

/** Does the class have the control as a member (`TSpeedButton *spbSave ...`)? */
function hasMember(hText, cls, ctl) {
  const b = classBody(hText, cls);
  if (!b || !ctl) return false;
  return new RegExp('\\*\\s*' + ctl + '\\s*[=;]').test(mask(hText).slice(b.open, b.close));
}

/**
 * The call of `obj->name(...)` made from what the page sends (a.button / a.x / a.y / a.shift ... a.key / a.chr),
 * by the declared parameters. -> { pre: [statements], call, usesShift } | { error }
 */
function callOf(params, obj, name, sender) {
  const ps = String(params || '').split(',').map(s => s.trim()).filter(Boolean);
  const pre = [], args = [];
  let usesShift = false, n = 0;
  for (const p of ps) {
    // "TObject *Sender" / "WORD &Key" / "TShiftState Shift" / "unsigned short &Key" / "int X"
    const m = /^(?:const )?([\w:]+(?: [\w:]+)*?) ?([*&]?) ?(\w+)?$/.exec(p.replace(/\s*([*&])\s*/g, ' $1 ').replace(/\s+/g, ' ').trim());
    if (!m) return { error: '參數「' + p + '」認不得' };
    const type = m[1], ref = m[2] || '', pname = m[3] || '';
    if (type === 'TObject' && ref === '*') args.push(sender);
    else if (type === 'TMouseButton' && !ref) { args.push('HtdButton(a)'); usesShift = true; }
    else if (type === 'TShiftState' && !ref) { args.push('HtdShift(a)'); usesShift = true; }
    else if (type === 'int' && !ref) args.push(/^X$/i.test(pname) ? 'a.x' : /^Y$/i.test(pname) ? 'a.y' : '0');
    else if ((type === 'WORD' || type === 'Word' || type === 'unsigned short') && ref === '&') { const v = 'k' + (n++); pre.push(type + ' ' + v + ' = (' + type + ')a.key;'); args.push(v); }
    else if (type === 'char' && ref === '&') { const v = 'c' + (n++); pre.push('char ' + v + ' = (char)a.chr;'); args.push(v); }
    else if (type === 'bool' && ref === '&') { const v = 'b' + (n++); pre.push('bool ' + v + ' = true;'); args.push(v); }
    else return { error: '參數「' + p + '」網頁給不出來（網頁事件只有滑鼠按鍵、位置、Shift／Ctrl／Alt、按鍵碼）' };
  }
  return { pre, call: obj + '->' + name + '(' + args.join(', ') + ');', usesShift };
}

function esc(s) { return String(s).replace(/\\/g, '\\\\').replace(/"/g, '\\"'); }

/**
 * The generated C++ file. rows: [{ form, handler, control, event, header, call: { pre, call } }];
 * ctx: { fn, cjson, lockHeader, lock, unlock, running: [names], shiftHeader }. One edit writes it whole.
 */
function genFile(rows, ctx, eol) {
  const E = eol || '\n';
  const c = ctx || {};
  const L = [];
  L.push('// 產生檔 -- HTML 視覺設計工具（tools/vscode-htdesigner）。不要手改：在設計工具的事件表新增、改名，它會重寫這個檔。');
  L.push('// ---------------------------------------------------------------------------');
  L.push('//  WS 命令 ' + CMD + '：網頁上的事件 → C++ 表單類別的事件函式（像 BCB6 的 .dfm 把 OnClick 接到 spbSaveClick）。');
  L.push('//  網頁（<script id="htdEvents"> 的 htdCpp(...)）送 value＝{"form","handler","control","event",');
  L.push('//  "button","x","y","shift","ctrl","alt","dbl","key","chr"}（tag＝頁名）；這裡照表找到那一列，在表單的全域物件上呼叫它。');
  L.push('//  跟 form.event 一樣：' + (c.running && c.running.length ? '機台運轉中（' + c.running.join('／') + '）不跑、' : '') +
    (c.lock ? '持 ' + c.lock.replace(/^.*::/, '') + '（同一把鎖）。' : '（這棵樹沒有找到表單鎖）。'));
  L.push('//  伺服器的分派：' + CMD + ' 那一個分支（設計工具加在 form.event 的同一行）。');
  L.push('//  事件（一列一個，設計工具靠這幾行重寫這個檔）：');
  for (const r of rows) L.push('// htd-row ' + r.form + ' ' + r.handler + ' ' + r.control + ' ' + r.event);
  L.push('// ---------------------------------------------------------------------------');
  L.push('#include <exception>');
  L.push('#include <string>');
  L.push('#include <type_traits>');
  L.push('');
  if (c.cjson) L.push('#include "' + c.cjson + '"');
  if (c.lockHeader) L.push('#include "' + c.lockHeader + '"');
  const usesShift = rows.some(r => r.call && r.call.usesShift);
  if (usesShift && c.shiftHeader) L.push('#include "' + c.shiftHeader + '"');
  const hs = [];
  for (const r of rows) if (r.header && !hs.includes(r.header)) hs.push(r.header);
  for (const h of hs) L.push('#include "' + h + '"');
  L.push('');
  for (const v of c.running || []) L.push('extern bool ' + v + ';');
  if ((c.running || []).length) L.push('');
  L.push('namespace {');
  L.push('');
  L.push('struct HtdIn { int button = 0; int x = 0; int y = 0; bool shift = false; bool ctrl = false; bool alt = false; bool dbl = false; int key = 0; int chr = 0; };');
  L.push('');
  L.push('// Sender：表單有那個元件就傳它（VCL 的 Sender），沒有就 nullptr');
  L.push('template <class T> TObject* HtdSender(T* p, typename std::enable_if<std::is_convertible<T*, TObject*>::value>::type* = 0) { return p; }');
  L.push('template <class T> TObject* HtdSender(T*, typename std::enable_if<!std::is_convertible<T*, TObject*>::value>::type* = 0) { return nullptr; }');
  if (usesShift) {
    L.push('TShiftState HtdShift(const HtdIn& a) { TShiftState s; if (a.shift) s << ssShift; if (a.alt) s << ssAlt; if (a.ctrl) s << ssCtrl; if (a.dbl) s << ssDouble; return s; }');
    L.push('TMouseButton HtdButton(const HtdIn& a) { return a.button == 2 ? mbRight : a.button == 1 ? mbMiddle : mbLeft; }');
  }
  L.push('');
  rows.forEach((r, i) => {
    L.push('// ' + r.form + '::' + r.handler + '（' + r.control + '.' + r.event + '）');
    L.push('void E' + i + '(const HtdIn& a) { (void)a; ' + (r.call.pre || []).join(' ') + (r.call.pre && r.call.pre.length ? ' ' : '') + r.call.call + ' }');
  });
  L.push('');
  L.push('struct HtdRow { const char* form; const char* handler; const char* control; const char* event; void (*run)(const HtdIn&); };');
  L.push('const HtdRow kRows[] = {');
  rows.forEach((r, i) => L.push('    {"' + r.form + '", "' + r.handler + '", "' + r.control + '", "' + r.event + '", &E' + i + '},'));
  L.push('    {nullptr, nullptr, nullptr, nullptr, nullptr},');
  L.push('};');
  L.push('');
  L.push('std::string Str(const cJSON* o, const char* k) { const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k); return v && cJSON_IsString(v) && v->valuestring ? std::string(v->valuestring) : std::string(); }');
  L.push('int Int(const cJSON* o, const char* k) { const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k); return v && cJSON_IsNumber(v) ? (int)v->valuedouble : 0; }');
  L.push('bool Bool(const cJSON* o, const char* k) { const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k); return v && cJSON_IsTrue(v); }');
  L.push('');
  L.push('}  // namespace');
  L.push('');
  L.push('bool ' + c.fn + '(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err)');
  L.push('{');
  L.push('    (void)tag;');
  if ((c.running || []).length) {
    L.push('    if (' + c.running.join(' || ') + ') {');
    L.push('        *err = "running: 機台運轉中不能從網頁執行畫面的事件（同 form.event）";');
    L.push('        return false;');
    L.push('    }');
  }
  L.push('    cJSON* root = cJSON_Parse(valueJson.c_str());');
  L.push('    if (!root || !cJSON_IsObject(root)) {');
  L.push('        if (root) cJSON_Delete(root);');
  L.push('        *err = "bad-payload: value must be a JSON object string {\\"form\\",\\"handler\\",\\"control\\",\\"event\\",...}";');
  L.push('        return false;');
  L.push('    }');
  L.push('    const std::string form = Str(root, "form"), handler = Str(root, "handler"), control = Str(root, "control");');
  L.push('    HtdIn a;');
  L.push('    a.button = Int(root, "button"); a.x = Int(root, "x"); a.y = Int(root, "y");');
  L.push('    a.shift = Bool(root, "shift"); a.ctrl = Bool(root, "ctrl"); a.alt = Bool(root, "alt"); a.dbl = Bool(root, "dbl");');
  L.push('    a.key = Int(root, "key"); a.chr = Int(root, "chr");');
  L.push('    cJSON_Delete(root);');
  L.push('    const HtdRow* r = nullptr;');
  L.push('    // (the same handler may serve several controls: the row of this control first, for its Sender)');
  L.push('    for (const HtdRow* p = kRows; p->form; ++p) if (form == p->form && handler == p->handler && control == p->control) { r = p; break; }');
  L.push('    if (!r) for (const HtdRow* p = kRows; p->form; ++p) if (form == p->form && handler == p->handler) { r = p; break; }');
  L.push('    if (!r) {');
  L.push('        *err = "unknown-handler: " + form + "::" + handler + " is not in ' + esc(GEN_REL) + ' (add it in the designer\'s events table)";');
  L.push('        return false;');
  L.push('    }');
  L.push('    bool ok = true;');
  L.push('    std::string why;');
  if (c.lock) L.push('    ' + c.lock + '();');
  L.push('    try {');
  L.push('        r->run(a);');
  L.push('    } catch (const std::exception& x) {');
  L.push('        ok = false; why = std::string("exception: ") + x.what();');
  L.push('    } catch (...) {');
  L.push('        ok = false; why = "non-std exception";');
  L.push('    }');
  if (c.unlock) L.push('    ' + c.unlock + '();');
  L.push('    if (!ok) { *err = "handler-failed: " + why; return false; }');
  L.push('    *ack = std::string("{\\"form\\":\\"") + r->form + "\\",\\"handler\\":\\"" + r->handler + "\\",\\"control\\":\\"" + r->control + "\\",\\"event\\":\\"" + r->event + "\\"}";');
  L.push('    return true;');
  L.push('}');
  L.push('');
  return L.join(E);
}

module.exports = { CMD, GEN_REL, serverHook, cmakeHook, rowsOf, globalOf, declOf, methodsOf, typesKey, hasMember, callOf, genFile, accessAt };
