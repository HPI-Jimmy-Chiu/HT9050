'use strict';
// AI(W906-HTDESIGNER) 20260929: DFM value formatting and the VCL-event -> DOM-event map.
// Plain Node (no vscode).

/** Which DOM event types stand in for a VCL event on the web page. */
const DOM_OF = {
  OnClick: ['click'],
  OnDblClick: ['dblclick'],
  OnMouseDown: ['mousedown', 'pointerdown', 'touchstart'],
  OnMouseUp: ['mouseup', 'pointerup', 'touchend'],
  OnMouseMove: ['mousemove', 'pointermove'],
  OnMouseEnter: ['mouseenter', 'mouseover'],
  OnMouseLeave: ['mouseleave', 'mouseout'],
  OnKeyDown: ['keydown'],
  OnKeyUp: ['keyup'],
  OnKeyPress: ['keypress', 'keydown'],
  OnChange: ['change', 'input'],
  OnEnter: ['focus', 'focusin'],
  OnExit: ['blur', 'focusout'],
  OnContextPopup: ['contextmenu'],
  OnSelectCell: ['click'],
  OnShow: ['DOMContentLoaded', 'load'],
};

function domTypesOf(vclEvent) {
  if (DOM_OF[vclEvent]) return DOM_OF[vclEvent];
  const t = String(vclEvent || '').replace(/^On/, '').toLowerCase();
  return t ? [t] : [];
}

/** The event a double-click on the component should open (WPF: the default event). */
function defaultEvent(events, isForm) {
  if (!events || !events.length) return -1;
  const order = isForm ? ['OnShow', 'OnCreate', 'OnActivate'] : ['OnClick', 'OnChange', 'OnDblClick', 'OnMouseDown'];
  for (const name of order) {
    const i = events.findIndex(e => e.name === name);
    if (i >= 0) return i;
  }
  return 0;
}

/** "spbSave : TSpeedButton（統一 save 樣式）" -> { name, cls, note } */
function parseVclTitle(title) {
  const m = /^\s*([A-Za-z_@][\w]*)\s*:\s*(T\w+)\s*(.*)$/.exec(String(title || ''));
  if (!m) return null;
  const note = m[3].trim().replace(/^[（(]\s*/, '').replace(/\s*[）)]$/, '');
  return { name: m[1], cls: m[2], note };
}

// --- TColor ---------------------------------------------------------------
const CL = {
  clBlack: '#000000', clMaroon: '#800000', clGreen: '#008000', clOlive: '#808000',
  clNavy: '#000080', clPurple: '#800080', clTeal: '#008080', clGray: '#808080',
  clSilver: '#c0c0c0', clRed: '#ff0000', clLime: '#00ff00', clYellow: '#ffff00',
  clBlue: '#0000ff', clFuchsia: '#ff00ff', clAqua: '#00ffff', clWhite: '#ffffff',
  clMoneyGreen: '#c0dcc0', clSkyBlue: '#a6caf0', clCream: '#fffbf0', clMedGray: '#a0a0a4',
  clScrollBar: '#d4d0c8', clBackground: '#3a6ea5', clActiveCaption: '#0a246a',
  clInactiveCaption: '#808080', clMenu: '#d4d0c8', clWindow: '#ffffff', clWindowFrame: '#000000',
  clMenuText: '#000000', clWindowText: '#000000', clCaptionText: '#ffffff',
  clActiveBorder: '#d4d0c8', clInactiveBorder: '#d4d0c8', clAppWorkSpace: '#808080',
  clHighlight: '#0a246a', clHighlightText: '#ffffff', clBtnFace: '#d4d0c8',
  clBtnShadow: '#808080', clGrayText: '#808080', clBtnText: '#000000',
  clInactiveCaptionText: '#d4d0c8', clBtnHighlight: '#ffffff', cl3DDkShadow: '#404040',
  cl3DLight: '#d4d0c8', clInfoText: '#000000', clInfoBk: '#ffffe1',
};
// index of the system colour (TColor 0x80000000 | index)
const SYS = ['clScrollBar', 'clBackground', 'clActiveCaption', 'clInactiveCaption', 'clMenu',
  'clWindow', 'clWindowFrame', 'clMenuText', 'clWindowText', 'clCaptionText', 'clActiveBorder',
  'clInactiveBorder', 'clAppWorkSpace', 'clHighlight', 'clHighlightText', 'clBtnFace',
  'clBtnShadow', 'clGrayText', 'clBtnText', 'clInactiveCaptionText', 'clBtnHighlight',
  'cl3DDkShadow', 'cl3DLight', 'clInfoText', 'clInfoBk'];

function hex2(n) { return (n & 255).toString(16).padStart(2, '0'); }

/** TColor (int, "$00BBGGRR", or clName) -> "#rrggbb" or null. */
function tcolor(v) {
  if (typeof v === 'string') {
    if (Object.prototype.hasOwnProperty.call(CL, v)) return CL[v];
    if (/^\$[0-9a-f]+$/i.test(v)) v = parseInt(v.slice(1), 16);
    else if (/^-?\d+$/.test(v)) v = parseInt(v, 10);
    else return null;
  }
  if (typeof v !== 'number' || !isFinite(v)) return null;
  if (v < 0 || v >= 0x80000000) {
    const name = SYS[v & 0xff];
    return name ? CL[name] : null;
  }
  return '#' + hex2(v) + hex2(v >> 8) + hex2(v >> 16);
}

/** The system colour's name (clBtnFace ...) when v is one, else null: its RGB depends on the Windows theme. */
function sysColorName(v) {
  if (typeof v === 'string') {
    if (SYS.includes(v)) return v;
    if (/^\$[0-9a-f]+$/i.test(v)) v = parseInt(v.slice(1), 16);
    else if (/^-?\d+$/.test(v)) v = parseInt(v, 10);
    else return null;
  }
  if (typeof v !== 'number' || !isFinite(v)) return null;
  if (v < 0 || v >= 0x80000000) return SYS[v & 0xff] || null;
  return null;
}

/** Font.Height -> the px the generated pages use for it (|h| - 2, the generator's rule). */
function pagePx(h) {
  return Math.max(1, Math.abs(h) - 2);
}

/** A VCL caption as shown: "&Save" -> "Save", "A && B" -> "A & B" (the & marks the accelerator). */
function vclText(s) {
  return String(s).replace(/&&|&/g, m => (m === '&&' ? '&' : ''));
}

/** One DFM property -> [key, text, colour|null, hint|null]. */
function formatProp(key, prop) {
  const type = prop && prop.type;
  const v = prop ? prop.value : undefined;
  let text;
  switch (type) {
    case 'STR': text = "'" + String(v) + "'"; break;
    case 'SET': text = '[' + (Array.isArray(v) ? v.join(', ') : String(v)) + ']'; break;
    case 'LIST': {
      const a = Array.isArray(v) ? v : [];
      text = a.slice(0, 8).map(String).join(' | ') + (a.length > 8 ? ' …共 ' + a.length + ' 項' : '');
      if (!a.length) text = '（空）';
      break;
    }
    case 'COLLECTION': text = '（集合，' + (Array.isArray(v) ? v.length : 0) + ' 項）'; break;
    case 'BLOB': {
      let n = null;
      if (typeof v === 'string') n = Math.floor(v.replace(/\s+/g, '').length / 2);
      else if (v && typeof v === 'object') n = v.length || v.bytes || v.size || null;
      text = n != null ? '（二進位資料 ' + n + ' bytes）' : '（二進位資料）';
      break;
    }
    default: text = v === undefined ? '' : (typeof v === 'object' ? JSON.stringify(v) : String(v));
  }
  let color = null;
  let hint = null;
  if (/Color$/.test(key) && (type === 'SCALAR' || type === 'STR')) {
    color = tcolor(v);
    if (color && typeof v === 'number') hint = color;
  }
  if (/(^|\.)Font\.Height$/.test(key) && typeof v === 'number' && v < 0) {
    hint = '約 ' + Math.round(-v * 72 / 96) + ' pt';
  }
  return [key, text, color, hint];
}

/** All properties except the On* events (those have their own section). */
function formatProps(properties) {
  if (!properties) return [];
  return Object.keys(properties)
    .filter(k => !/^On[A-Z]/.test(k))
    .sort((a, b) => a.localeCompare(b, 'en', { sensitivity: 'base' }))
    .map(k => formatProp(k, properties[k]));
}

/**
 * The DFM (BCB6) values of the properties the designer can edit, in the designer's
 * terms, for "compare with DFM" / "reset to DFM". Only what the DFM actually says:
 * a property the DFM leaves at its default is missing here (null), not guessed.
 * A system colour (clBtnFace ...) is not given as RGB -- its colour is the Windows
 * theme's, so there is nothing to compare -- only its name (colorSys / backgroundSys).
 * fontSize is in the pages' px: the generator writes |Font.Height| - 2
 * (_gen_dfm_abs.py fontpx: "VCL Font.Height(-13)≈11px 顯示"), so that is the value a
 * page made from this DFM has; fontHeight keeps the DFM's own number for display.
 *   node: an IR node (properties + geometry)
 */
function dfmEditValues(node) {
  if (!node) return null;
  const p = node.properties || {};
  const val = k => (p[k] && p[k].value !== undefined ? p[k].value : null);
  const g = node.geometry || {};
  const fc = val('Font.Color');
  const bg = val('Color');
  const out = {
    left: typeof g.left === 'number' ? g.left : null,
    top: typeof g.top === 'number' ? g.top : null,
    width: typeof g.width === 'number' ? g.width : null,
    height: typeof g.height === 'number' ? g.height : null,
    caption: val('Caption') != null ? vclText(val('Caption')) : null,
    text: val('Text') != null ? String(val('Text')) : null,
    fontName: val('Font.Name') != null ? String(val('Font.Name')) : null,
    fontSize: typeof val('Font.Height') === 'number' && val('Font.Height') !== 0 ? pagePx(val('Font.Height')) : null,
    fontHeight: typeof val('Font.Height') === 'number' ? val('Font.Height') : null,
    bold: Array.isArray(val('Font.Style')) ? val('Font.Style').includes('fsBold') : null,
    italic: Array.isArray(val('Font.Style')) ? val('Font.Style').includes('fsItalic') : null,
    color: fc != null && !sysColorName(fc) ? tcolor(fc) : null,
    colorSys: fc != null ? sysColorName(fc) : null,
    background: bg != null && !sysColorName(bg) ? tcolor(bg) : null,
    backgroundSys: bg != null ? sysColorName(bg) : null,
    visible: val('Visible') != null ? String(val('Visible')) !== 'False' : null,
    enabled: val('Enabled') != null ? String(val('Enabled')) !== 'False' : null,
    // a TLabel's AutoSize is True unless the .dfm says False (VCL: "property AutoSize default True",
    // so the .dfm never writes True) -- that one default is known, not guessed
    autoSize: val('AutoSize') != null ? String(val('AutoSize')) !== 'False' : node.class === 'TLabel' ? true : null,
    // the caption's alignment -- VCL defaults, never written: TLabel taLeftJustify, TPanel taCenter
    alignment: val('Alignment') != null ? String(val('Alignment')) : node.class === 'TLabel' ? 'taLeftJustify' : node.class === 'TPanel' ? 'taCenter' : null,
  };
  // the IO lamp (TALed / TMyLed / TMyLedLane) and panel button (TBtnPanel / TBtnPanelLane) properties -- again only what the
  // .dfm writes (their defaults live in the components' source, elec\Component / elec\myvcl, not here)
  const col = k => { const v = val(k); return v != null && !sysColorName(v) ? tcolor(v) : null; };
  const yes = k => (val(k) != null ? String(val(k)) !== 'False' : null);
  out.io = {
    ledStyle: val('LEDStyle') != null ? String(val('LEDStyle')) : null,
    trueColor: col('TrueColor'), falseColor: col('FalseColor'),
    trueFontColor: col('TrueFontColor'), falseFontColor: col('FalseFontColor'),
    value: yes('Value'), blink: yes('Blink'), down: yes('Down'),
    // Style = tsFlatButtons (the flat look, Steven 20230826 "新版GUI") / tsButtons
    flat: val('Style') != null ? String(val('Style')) === 'tsFlatButtons' : null,
  };
  return out;
}

module.exports = { DOM_OF, domTypesOf, defaultEvent, parseVclTitle, tcolor, sysColorName, vclText, pagePx, formatProp, formatProps, dfmEditValues };
