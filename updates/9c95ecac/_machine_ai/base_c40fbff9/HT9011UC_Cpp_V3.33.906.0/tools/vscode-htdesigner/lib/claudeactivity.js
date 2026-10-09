'use strict';
// AI(W906-HTDESIGNER) 20261006 (EastSun: "我圖片上視窗 想要偵測claude 是否再作動 再作動的話 希望有轉圈圈動畫"): is a Claude Code
// session working? Each session writes its transcript to ~/.claude/projects/<project>/<session>.jsonl as it goes; the
// tab's title is the transcript's latest "ai-title" (or "custom-title"). Working = its last message is not a finished
// answer: the user's prompt / a tool result waiting for the model, or the model's turn still going (a tool call).
// Only the end of each file is read (a transcript can be tens of MB). Plain Node.
const fs = require('fs');
const path = require('path');

const TAIL = 256 * 1024;

/** The last `n` bytes of a file as text (the first, likely cut, line dropped when it is not the file's start). */
function tailOf(file, n) {
  let fd = null;
  try {
    fd = fs.openSync(file, 'r');
    const size = fs.fstatSync(fd).size;
    const len = Math.min(size, n || TAIL);
    const b = Buffer.alloc(len);
    fs.readSync(fd, b, 0, len, size - len);
    let t = b.toString('utf8');
    if (len < size) { const i = t.indexOf('\n'); t = i >= 0 ? t.slice(i + 1) : ''; }
    return t;
  } catch (e) {
    return '';
  } finally {
    if (fd !== null) try { fs.closeSync(fd); } catch (e) { /* closed */ }
  }
}

/** A user entry that only says the turn was stopped (Esc), or a tool the user said no to: not work waiting. */
function interrupted(j) {
  const c = j && j.message && j.message.content;
  const text = typeof c === 'string' ? c : Array.isArray(c) ? c.map(x => (x && typeof x.text === 'string' ? x.text : '')).join(' ') : '';
  if (/\[Request interrupted/i.test(text)) return true;
  // (1009 review #1: "No" to a tool's permission question -- the turn stops and waits for the user; its tool_result (an
  //  error saying so) is the last entry, and the tab spun "作動中" for 15 minutes)
  if (Array.isArray(c)) {
    for (const x of c) {
      if (!x || x.type !== 'tool_result' || !x.is_error) continue;
      const t = typeof x.content === 'string' ? x.content : Array.isArray(x.content) ? x.content.map(y => (y && typeof y.text === 'string' ? y.text : '')).join(' ') : '';
      if (/doesn't want to (take this action|proceed)|user (rejected|denied|declined)|tool use was rejected/i.test(t)) return true;
    }
  }
  return false;
}

/**
 * The state of a transcript from its text (its end is enough): { title, busy, at } -- title = the latest ai-title /
 * custom-title ('' none), busy = the last user / assistant entry is not a finished answer, at = that entry's timestamp.
 */
function stateOf(text) {
  const lines = String(text || '').split('\n');
  let title = '', custom = '', last = null;
  for (let i = lines.length - 1; i >= 0; i--) {
    const l = lines[i].trim();
    if (!l || l[0] !== '{') continue;
    let j;
    try { j = JSON.parse(l); } catch (e) { continue; }
    if (!custom && j.type === 'custom-title' && j.customTitle) custom = String(j.customTitle);
    if (!title && j.type === 'ai-title' && j.aiTitle) title = String(j.aiTitle);
    if (!last && (j.type === 'user' || j.type === 'assistant') && !j.isSidechain && !j.isMeta) last = j;
    if (last && title && custom) break;
  }
  let busy = false;
  if (last && last.type === 'assistant') {
    const sr = last.message && last.message.stop_reason;
    busy = !(sr === 'end_turn' || sr === 'stop_sequence' || sr === 'max_tokens' || sr === 'refusal');
  } else if (last && last.type === 'user') busy = !interrupted(last);
  return { title: custom || title, busy, at: last && last.timestamp ? Date.parse(last.timestamp) || null : null, found: !!last };
}

// (1009 review #2: a last line longer than the tail (a pasted screenshot, a big tool result -- 78 of 135 transcripts here
//  have one) left no user / assistant entry in it: the tab was "not working", "Claude 完成" said, and its title gone. The
//  tail is read again twice as long up to 8 MB; the last title of each file is remembered)
const titles = new Map();
const noTitle = new Set();
function stateOfFile(file) {
  let s = null;
  for (let n = TAIL; n <= 8 * 1024 * 1024; n *= 4) {
    s = stateOf(tailOf(file, n));
    // (1009 review (tree #7): found but untitled -- read deeper for the title once per file, not every 2 s)
    if (s.found && (s.title || titles.has(file) || noTitle.has(file))) break;
    if (s.found && n >= 1024 * 1024) { noTitle.add(file); if (noTitle.size > 500) noTitle.delete(noTitle.values().next().value); break; }
  }
  if (s.title) titles.set(file, s.title);
  else if (titles.has(file)) s.title = titles.get(file);
  if (titles.size > 500) titles.delete(titles.keys().next().value);
  return s;
}

/**
 * Every session written to in the last `windowMs` under `projectsDir` (~/.claude/projects): [{ file, title, busy, mtime }].
 * A session silent for longer than that is not counted as working (a crash leaves its last tool call open).
 */
function scan(projectsDir, now, windowMs) {
  const out = [];
  const t = now || Date.now(), w = windowMs || 15 * 60 * 1000;
  let dirs = [];
  try { dirs = fs.readdirSync(projectsDir, { withFileTypes: true }).filter(d => d.isDirectory()).map(d => path.join(projectsDir, d.name)); } catch (e) { return out; }
  for (const d of dirs) {
    let names = [];
    try { names = fs.readdirSync(d).filter(n => n.endsWith('.jsonl')); } catch (e) { continue; }
    for (const n of names) {
      const f = path.join(d, n);
      let st;
      try { st = fs.statSync(f); } catch (e) { continue; }
      if (t - st.mtimeMs > w) continue;
      const s = stateOfFile(f);
      out.push({ file: f, title: s.title, busy: s.busy, mtime: st.mtimeMs });
    }
  }
  return out;
}

module.exports = { tailOf, stateOf, scan };
