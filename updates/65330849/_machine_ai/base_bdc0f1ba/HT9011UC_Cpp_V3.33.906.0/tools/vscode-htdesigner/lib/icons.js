/* AI(W906-HTDESIGNER) 20261001 (0.148): the icons of VS Code itself for the 方案總管 the extension draws (EastSun:
 * "方案總管整個重做成一格" -- the search box right under its title, so the tree is a webview, not VS Code's tree).
 * - codicons: VS Code's own font (out/media/codicon.ttf); a name's character is read from the workbench's code
 *   (the table VS Code itself uses), FALLBACK if that cannot be read.
 * - file icons: the Seti theme VS Code ships (extensions/theme-seti): the same icon and colour as VS Code's explorer.
 * Pure: give it VS Code's app folder (vscode.env.appRoot). */
'use strict';
const fs = require('fs');
const path = require('path');

// (read from VS Code 1.11x's workbench 20261001; a codicon keeps its character between versions)
const FALLBACK = {
  'folder-library': 60383, project: 60208, globe: 60161, history: 60034, 'symbol-folder': 60035, layout: 60395, file: 60027,
  'file-code': 60137, eye: 60016, 'eye-closed': 60135, 'arrow-right': 60060, 'root-folder': 60230, 'symbol-class': 60251,
  'symbol-text': 60051, 'symbol-field': 60255, 'symbol-method': 60044, 'symbol-event': 60038, 'symbol-string': 60301,
  'chevron-right': 60086, 'chevron-down': 60084, search: 60013, close: 60022, filter: 60145, 'clear-all': 60095, target: 60408,
  refresh: 60215, 'collapse-all': 60101, check: 60082, 'circle-large-outline': 60341, files: 60144, 'file-media': 60138,
  'list-selection': 60293, table: 60343, window: 60287, lightbulb: 60001, 'group-by-ref-type': 60311, folder: 60035,
  'folder-opened': 60151, warning: 60012, error: 60039, info: 60020, lock: 60021,
};

const cache = new Map();

/** name -> character code of every codicon VS Code has (its own table), FALLBACK under it. */
function codicons(appRoot) {
  const k = 'c|' + appRoot;
  if (cache.has(k)) return cache.get(k);
  const m = Object.assign({}, FALLBACK);
  try {
    const t = fs.readFileSync(path.join(appRoot, 'out', 'vs', 'workbench', 'workbench.desktop.main.js'), 'utf8');
    const re = /\("([a-z0-9]+(?:-[a-z0-9]+)*)",(6\d{4})\)/g;
    let x, n = 0;
    const got = {};
    while ((x = re.exec(t))) { if (!(x[1] in got)) { got[x[1]] = +x[2]; n++; } }
    if (n > 300) Object.assign(m, got);
  } catch (e) { /* the fallback */ }
  cache.set(k, m);
  return m;
}

/** VS Code's codicon font, or null. */
function codiconFont(appRoot) {
  const f = path.join(appRoot, 'out', 'media', 'codicon.ttf');
  return fs.existsSync(f) ? f : null;
}

/** The Seti file icon theme VS Code ships: { dir, font (file), theme (its json) } or null. */
function seti(appRoot) {
  const k = 's|' + appRoot;
  if (cache.has(k)) return cache.get(k);
  let r = null;
  try {
    const dir = path.join(appRoot, 'extensions', 'theme-seti', 'icons');
    const theme = JSON.parse(fs.readFileSync(path.join(dir, 'vs-seti-icon-theme.json'), 'utf8'));
    const font = theme.fonts && theme.fonts[0] && theme.fonts[0].src && theme.fonts[0].src[0] ? path.join(dir, theme.fonts[0].src[0].path) : null;
    if (font && fs.existsSync(font)) r = { dir, font, theme };
  } catch (e) { r = null; }
  cache.set(k, r);
  return r;
}

// the file extensions' languages (Seti finds most files by language: .cpp is "cpp", not an extension of its own)
const LANG = {
  cpp: 'cpp', cc: 'cpp', cxx: 'cpp', hpp: 'cpp', hh: 'cpp', hxx: 'cpp', c: 'c', h: 'cpp', js: 'javascript', mjs: 'javascript',
  cjs: 'javascript', ts: 'typescript', json: 'json', jsonc: 'jsonc', html: 'html', htm: 'html', css: 'css', md: 'markdown',
  bat: 'bat', cmd: 'bat', ps1: 'powershell', psm1: 'powershell', py: 'python', xml: 'xml', ini: 'ini', txt: 'plaintext',
  yml: 'yaml', yaml: 'yaml', sh: 'shellscript', sql: 'sql', cs: 'csharp', java: 'java', rc: 'plaintext', log: 'log',
};

/** A file's Seti icon: { char, color } (the theme's default when nothing fits). light = a light colour theme. */
function fileIcon(st, name, light) {
  if (!st) return null;
  const t = st.theme;
  const sects = light && t.light ? [t.light, t] : [t];
  const lower = String(name).toLowerCase();
  const exts = [];
  const parts = lower.split('.');
  for (let i = 1; i < parts.length; i++) exts.push(parts.slice(i).join('.'));
  let id = null;
  for (const s of sects) {
    if (!id && s.fileNames && s.fileNames[lower]) id = s.fileNames[lower];
    for (const e of exts) { if (!id && s.fileExtensions && s.fileExtensions[e]) id = s.fileExtensions[e]; }
    const lang = exts.length ? LANG[exts[exts.length - 1]] : null;
    if (!id && lang && s.languageIds && s.languageIds[lang]) id = s.languageIds[lang];
  }
  if (!id) id = (light && t.light && t.light.file) || t.file;
  const d = t.iconDefinitions[id];
  if (!d || !d.fontCharacter) return null;
  return { char: parseInt(String(d.fontCharacter).replace(/^\\/, ''), 16), color: d.fontColor || null };
}

module.exports = { codicons, codiconFont, seti, fileIcon, FALLBACK, LANG };
