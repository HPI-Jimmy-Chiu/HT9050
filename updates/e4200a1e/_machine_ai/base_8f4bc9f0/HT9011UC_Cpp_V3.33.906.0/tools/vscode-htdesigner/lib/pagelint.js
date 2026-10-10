'use strict';
// AI(W906-HTDESIGNER) 20260929: 頁面檢查 -- problems in a page's source, for VS Code's
// Problems panel (WPF: the Error List). Plain Node (no vscode).
//   issue = { kind, level: 'error'|'warn'|'info', at, len, msg, id?, fix?: { at, len, repl } }
// kinds:
//   quote-cut  an attribute value closed early by a quote inside it: the generator writes
//              style="…font-family:"MS Sans Serif",sans-serif;font-weight:400;" -- the browser
//              ends style at 'font-family:' and drops the font and everything after it.
//              Fix: the inner " become ' (what the value meant).
//   dup-id     the same id twice on a page (getElementById finds only the first)
//   unclosed   an element with an id whose end tag is not found (what follows nests into it)
//   missing    <img src> / <script src> / <link href> naming a file that is not there
const fs = require('fs');
const path = require('path');
const he = require('./htmledit');
const hb = require('./htmlblock');

/** Every start tag (not in comments / scripts / styles): [{ start, end, text, name }]. */
function startTags(text) {
  const out = [];
  let i = 0;
  const n = text.length;
  while (i < n) {
    const lt = text.indexOf('<', i);
    if (lt < 0) break;
    if (text.startsWith('<!--', lt)) { const e = text.indexOf('-->', lt); i = e < 0 ? n : e + 3; continue; }
    if (!/^<[A-Za-z]/.test(text.slice(lt, lt + 2))) { i = lt + 1; continue; }
    const e = he.tagEnd(text, lt);
    // (1007 audit, lint #1: a tag whose quotes never close ended the whole check -- the page then said "沒有問題" with
    //  every problem after it unseen. That tag is reported (unterminated) and the check goes on at the next "<")
    if (e < 0) {
      const nx = text.indexOf('<', lt + 1);
      out.push({ start: lt, end: nx < 0 ? n : nx, text: text.slice(lt, nx < 0 ? n : nx), name: hb.nameOf(text.slice(lt, lt + 40)), broken: true });
      i = nx < 0 ? n : nx;
      continue;
    }
    const t = text.slice(lt, e + 1);
    const name = hb.nameOf(t);
    out.push({ start: lt, end: e + 1, text: t, name });
    // (1010 operate walkthrough #5: the text inside <textarea> / <title> (and noscript / xmp) is no markup -- "<div id=z>"
    //  typed in a textarea was taken as an unclosed tag and a duplicate id)
    if (name === 'script' || name === 'style' || name === 'textarea' || name === 'title' || name === 'noscript' || name === 'xmp') {
      const endTag = he.findCI(text, '</' + name, e + 1);
      // (1010 review of 0.395 B: no end tag -- the browser takes the whole rest of the page as its text; it was not checked
      //  and "no problems" was said)
      if (endTag < 0 && name !== 'script' && name !== 'style') out[out.length - 1].rawUnclosed = true;
      i = endTag < 0 ? n : endTag;
      continue;
    }
    i = e + 1;
  }
  return out;
}

/**
 * In a start tag: attribute values closed early by a quote of their own kind.
 * [{ name, at, len, value }] -- at/len = the whole value (offsets in the tag text),
 * value = what it meant (the inner quotes turned into the other kind).
 */
function cutValues(tagText) {
  const out = [];
  const re = /\s([A-Za-z_:][\w:.-]*)\s*=\s*(["'])/g;
  let m;
  while ((m = re.exec(tagText))) {
    const q = m[2];
    const start = m.index + m[0].length;
    let i = start, end = -1;
    const inner = [];
    for (;;) {
      const p = tagText.indexOf(q, i);
      if (p < 0) break;
      // (1009 review #5 / #6: the value ends where the tag goes on as after a value -- the end, > or />, another attribute
      //  (with or without a space before it: class="a"id="b" is two attributes), a bare one (disabled>). A quote followed by
      //  " ;color:red" is inside the value: the fix used to stop there and lose what came after)
      const rest = tagText.slice(p + 1, p + 200);
      if (!rest || /^(\s*(\/?>|$)|\s+[A-Za-z_:][\w:.-]*(\s*=|\s|\/?>|$)|[A-Za-z_:][\w:.-]*\s*=\s*["'])/.test(rest)) { end = p; break; }
      inner.push(p);
      i = p + 1;
    }
    if (end < 0) break;
    if (inner.length) {
      const other = q === '"' ? "'" : '"';
      out.push({ name: m[1].toLowerCase(), at: start, len: end - start, value: tagText.slice(start, end).split(q).join(other) });
    }
    re.lastIndex = end + 1;
  }
  return out;
}

/**
 * Problems in one page's source.
 *   opts.dir     the page's folder (for missing files); opts.exists(path) -> bool
 */
const OMIT_END = new Set(['p', 'li', 'option', 'optgroup', 'tr', 'td', 'th', 'thead', 'tbody', 'tfoot', 'dt', 'dd', 'rt', 'rp', 'colgroup', 'caption']);
function lintPage(text, opts) {
  opts = opts || {};
  const exists = opts.exists || (p => { try { return fs.existsSync(p); } catch (e) { return false; } });
  const issues = [];
  const seen = new Map();
  for (const t of startTags(text)) {
    if (t.rawUnclosed) {
      issues.push({ kind: 'unclosed', level: 'error', at: t.start, len: t.end - t.start,
        msg: '<' + t.name + '> 沒有結束標籤 </' + t.name + '>：瀏覽器會把後面整頁都當成它的文字（後面的內容沒有檢查）' });
    }
    if (t.broken) {
      issues.push({ kind: 'unterminated', level: 'error', at: t.start, len: Math.min(t.end - t.start, 200),
        msg: '<' + (t.name || '?') + ' …> 的引號沒有成對，標籤沒有結束：瀏覽器會把後面一段當成這個標籤的屬性' });
      continue;
    }
    for (const c of cutValues(t.text)) {
      issues.push({ kind: 'quote-cut', level: 'error', at: t.start + c.at, len: c.len,
        msg: c.name + '="…" 被裡面的引號提早結束：瀏覽器只讀到「' + t.text.slice(c.at, c.at + Math.min(c.len, 60)).split('"')[0] + '」，後面' +
          '（字型、粗細…）都被丟掉。修正：裡面的 " 改成 \'',
        fix: { at: t.start + c.at, len: c.len, repl: c.value } });
    }
    // (1009 review #8: the id attribute itself -- "id='q'" written inside a title was taken for it)
    const idt = he.attrTokens(t.text).find(a => a.name === 'id' && a.q !== null && a.ve > a.vs);
    const idm = idt ? Object.assign([t.text.slice(idt.ws, idt.end), null, t.text.slice(idt.vs, idt.ve)], { index: idt.ws }) : null;
    if (idm) {
      const id = idm[2];
      if (seen.has(id)) {
        issues.push({ kind: 'dup-id', level: 'error', at: t.start + idm.index + 1, len: idm[0].length - 1, id,
          msg: 'id="' + id + '" 在這一頁出現第二次（第一次在第 ' + seen.get(id) + ' 個字元）：getElementById 只找得到第一個' });
      } else seen.set(id, t.start);
      // (1009 review #7: <p> <li> <option> <tr> <td> ... may leave their end tag out -- valid HTML, closed by the next one)
      if (!he.VOID.has(t.name) && !OMIT_END.has(t.name) && !/\/>$/.test(t.text) && !hb.elementRange(text, t)) {
        issues.push({ kind: 'unclosed', level: 'warn', at: t.start, len: t.end - t.start, id,
          msg: '<' + t.name + ' id="' + id + '"> 找不到它的結束標籤 </' + t.name + '>：後面的元素都會被當成在它裡面' });
      }
    }
    if (opts.dir) {
      // (1009 review (lint #9): SRC= / an unquoted src= too; a path from the site's root ("/theme.js") is resolved from the
      //  web root when one is given, else not checked -- it was taken from the drive's root and said missing)
      const rx = n => new RegExp('\\s' + n + '\\s*=\\s*(?:(["\'])([^"\']*)\\1|([^\\s>"\']+))', 'i');
      let ref = t.name === 'img' || t.name === 'script' ? rx('src').exec(t.text) : t.name === 'link' ? rx('href').exec(t.text) : null;
      if (ref && ref[2] === undefined && ref[3] !== undefined) ref = Object.assign([ref[0], '', ref[3]], { index: ref.index });
      if (ref && /^\//.test(ref[2]) && !/^\/\//.test(ref[2])) {
        if (opts.webRoot) ref = Object.assign([ref[0], ref[1], ref[2]], { index: ref.index, root: opts.webRoot }); else ref = null;
      }
      if (ref && ref[2] && !/^(data:|https?:|\/\/|#|javascript:)/i.test(ref[2])) {
        // (1007 audit, lint #2: a bad % escape threw -- and "lint all" stopped at that page; it counts as a missing file)
        let rel0 = ref[2].split(/[?#]/)[0];
        try { rel0 = decodeURIComponent(rel0); } catch (e) { /* kept as written */ }
        const f = ref.root ? path.join(ref.root, rel0) : path.resolve(opts.dir, rel0);
        if (!exists(f)) {
          issues.push({ kind: 'missing', level: 'warn', at: t.start + ref.index + 1, len: ref[0].length - 1,
            msg: '<' + t.name + '> 用到的檔案不存在：' + ref[2] });
        }
      }
    }
  }
  issues.sort((a, b) => a.at - b.at);
  return issues;
}

/*
 * dfm-gap (info): what the page generator leaves out of a form, read from the source against
 * its .dfm (the page's title names it) -- a TLabel with AutoSize = False drawn "width:auto"
 * (its text sits elsewhere than in BCB6), a fixed-width label or a generated panel caption
 * whose Alignment is not the .dfm's. Fix: the .dfm's width / height and alignment.
 *   ir: IrStore.load() of the page's form
 */
const fmt = require('./format');
const TA = { taCenter: 'center', taRightJustify: 'right', taLeftJustify: 'left' };
const TA_ZH = { taCenter: '置中', taRightJustify: '靠右', taLeftJustify: '靠左' };
function lastDecl(st, name) { const h = st.decls.filter(x => x.name === name); return h.length ? h[h.length - 1].value.trim().toLowerCase() : null; }
function dfmGaps(text, ir) {
  const out = [];
  if (!ir || !ir.byName) return out;
  for (const [name, node] of ir.byName) {
    if (node.class !== 'TLabel' && node.class !== 'TPanel') continue;
    const tag = he.startTagOf(text, name);
    if (!tag || cutValues(tag.text).length) continue;   // (a cut style first: 頁面檢查's quote-cut)
    const dv = fmt.dfmEditValues(node);
    if (node.class === 'TLabel') {
      const st = he.styleOf(tag.text);
      // (1009 review #4: no text-align of its own = left only for the generator's .lb -- Data.LotInfo's .liPnl is centred by
      //  the page's <style>; the "fix" added a second text-align:center)
      const own = lastDecl(st, 'text-align');
      if (!own && !/\bclass\s*=\s*["']([^"']*\s)?lb(\s[^"']*)?["']/.test(tag.text)) continue;
      const w = lastDecl(st, 'width'), h = lastDecl(st, 'height'), ta = own || 'left';
      const auto = w === 'auto' || h === 'auto';
      const al = dv.alignment || 'taLeftJustify';
      const wantTa = TA[al] || 'left';
      if (dv.autoSize === false && auto && dv.width !== null && dv.height !== null) {
        const ch = { width: dv.width + 'px', height: dv.height + 'px' };
        if (wantTa !== 'left') ch['text-align'] = wantTa;
        out.push({ kind: 'dfm-gap', level: 'info', at: tag.start, len: tag.end - tag.start, id: name,
          msg: name + '：BCB6 是 AutoSize=False、' + dv.width + '×' + dv.height + (wantTa !== 'left' ? '、' + TA_ZH[al] : '') +
            '，這裡寫成 auto（大小跟著文字）：文字的位置跟 BCB6 不同（網頁產生器沒處理）',
          fix: { at: tag.start, len: tag.end - tag.start, repl: he.setStyle(tag.text, ch) } });
      } else if (!auto && ta !== wantTa && !(wantTa === 'left' && ta === 'start')) {
        out.push({ kind: 'dfm-gap', level: 'info', at: tag.start, len: tag.end - tag.start, id: name,
          msg: name + '：BCB6 的 Alignment 是 ' + al + '（' + TA_ZH[al] + '），這裡的文字是 ' + ta,
          // (1009 second review (lint #2): left = no text-align of its own only on the generator's .lb -- another class
          //  (.liPnl) centres it, and taking the own one away left it centred and hid the warning)
          fix: { at: tag.start, len: tag.end - tag.start, repl: he.setStyle(tag.text, { 'text-align': wantTa === 'left' ? (/\bclass\s*=\s*["']([^"']*\s)?lb(\s[^"']*)?["']/.test(tag.text) ? null : 'left') : wantTa }) } });
      }
    } else if (/\bclass\s*=\s*["'][^"']*\bpnl\b/.test(tag.text)) {
      const cap = he.captionHostTag(text, tag, 'pnlCap');
      if (!cap || cutValues(cap.text).length) continue;
      const al = dv.alignment || 'taCenter';
      const ta = lastDecl(he.styleOf(cap.text), 'text-align') || 'center';   // (.pnlCap's class centres it)
      const wantTa = TA[al] || 'center';
      if (ta !== wantTa) {
        out.push({ kind: 'dfm-gap', level: 'info', at: cap.start, len: cap.end - cap.start, id: name,
          msg: name + '：BCB6 的 Panel 標題是 ' + al + '（' + TA_ZH[al] + '），這裡是 ' + (ta === 'center' ? '置中' : ta),
          fix: { at: cap.start, len: cap.end - cap.start, repl: he.setStyle(cap.text, { 'text-align': wantTa === 'center' ? null : wantTa }) } });
      }
    }
  }
  return out.sort((a, b) => a.at - b.at);
}

/** 1009 second review (DFM #4): the ids whose start tag has a value cut by a quote inside it (the browser drops the rest) */
function cutIds(text) {
  const out = new Set();
  let lastId = null;
  for (const t of startTags(String(text || ''))) {
    if (t.broken) continue;
    const m = /\sid\s*=\s*["']([^"']+)["']/i.exec(t.text);
    if (m) lastId = m[1];
    if (!cutValues(t.text).length) continue;
    // (a TPanel's caption span / a group box's legend carries the font -- and has no id: its component's)
    if (m) out.add(m[1]); else if (lastId && (/\bclass\s*=\s*["'][^"']*\bpnlCap\b/i.test(t.text) || /^<legend\b/i.test(t.text))) out.add(lastId);
  }
  return out;
}

module.exports = { startTags, cutValues, lintPage, dfmGaps, cutIds };
