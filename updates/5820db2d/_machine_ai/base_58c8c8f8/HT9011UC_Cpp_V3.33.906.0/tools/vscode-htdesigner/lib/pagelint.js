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
    if (e < 0) break;
    const t = text.slice(lt, e + 1);
    const name = hb.nameOf(t);
    out.push({ start: lt, end: e + 1, text: t, name });
    if (name === 'script' || name === 'style') {
      const endTag = text.toLowerCase().indexOf('</' + name, e + 1);
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
      const nx = tagText[p + 1];
      if (nx === undefined || /[\s>/]/.test(nx)) { end = p; break; }
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
function lintPage(text, opts) {
  opts = opts || {};
  const exists = opts.exists || (p => { try { return fs.existsSync(p); } catch (e) { return false; } });
  const issues = [];
  const seen = new Map();
  for (const t of startTags(text)) {
    for (const c of cutValues(t.text)) {
      issues.push({ kind: 'quote-cut', level: 'error', at: t.start + c.at, len: c.len,
        msg: c.name + '="…" 被裡面的引號提早結束：瀏覽器只讀到「' + t.text.slice(c.at, c.at + Math.min(c.len, 60)).split('"')[0] + '」，後面' +
          '（字型、粗細…）都被丟掉。修正：裡面的 " 改成 \'',
        fix: { at: t.start + c.at, len: c.len, repl: c.value } });
    }
    const idm = /\sid\s*=\s*(["'])([^"']+)\1/.exec(t.text);
    if (idm) {
      const id = idm[2];
      if (seen.has(id)) {
        issues.push({ kind: 'dup-id', level: 'error', at: t.start + idm.index + 1, len: idm[0].length - 1, id,
          msg: 'id="' + id + '" 在這一頁出現第二次（第一次在第 ' + seen.get(id) + ' 個字元）：getElementById 只找得到第一個' });
      } else seen.set(id, t.start);
      if (!he.VOID.has(t.name) && !/\/>$/.test(t.text) && !hb.elementRange(text, t)) {
        issues.push({ kind: 'unclosed', level: 'warn', at: t.start, len: t.end - t.start, id,
          msg: '<' + t.name + ' id="' + id + '"> 找不到它的結束標籤 </' + t.name + '>：後面的元素都會被當成在它裡面' });
      }
    }
    if (opts.dir) {
      const ref = t.name === 'img' || t.name === 'script' ? /\ssrc\s*=\s*(["'])([^"']*)\1/.exec(t.text)
        : t.name === 'link' ? /\shref\s*=\s*(["'])([^"']*)\1/.exec(t.text) : null;
      if (ref && ref[2] && !/^(data:|https?:|\/\/|#|javascript:)/i.test(ref[2])) {
        const f = path.resolve(opts.dir, decodeURIComponent(ref[2].split(/[?#]/)[0]));
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
      const w = lastDecl(st, 'width'), h = lastDecl(st, 'height'), ta = lastDecl(st, 'text-align') || 'left';
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
          fix: { at: tag.start, len: tag.end - tag.start, repl: he.setStyle(tag.text, { 'text-align': wantTa === 'left' ? null : wantTa }) } });
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

module.exports = { startTags, cutValues, lintPage, dfmGaps };
