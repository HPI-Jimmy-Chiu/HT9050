'use strict';
// AI(W906-HTDESIGNER) 20260929: 工具箱 -- new components for a page, the WPF Toolbox.
// Each template is the markup the page generator (_gen_dfm_abs.py) writes for that
// VCL class, so a new component looks and behaves like the generated ones:
//   TLabel       <span class="lb">            TSpeedButton <button class="btn3d">
//   TEdit        <span pos><input class="ed">  TCheckBox    <label class="ckb"><input type="checkbox">
//   TComboBox    <select class="ed">           TPanel       <div class="pnl"><span class="pnlCap">
//   TGroupBox    <fieldset class="gbx"><legend>…</legend><div class="cli">
// Names follow the BCB6 designer: Label1, SpeedButton1 ... (the first one not used).
// Plain Node (no vscode).

function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }

const ITEMS = [
  { cls: 'TLabel', base: 'Label', label: 'Label', note: '文字標籤', icon: 'symbol-text',
    html: (id, x, y) => '<span class="lb" id="' + id + '" title="' + id + ' : TLabel" style="position:absolute;left:' + x + 'px;top:' + y + 'px;color:#000000;white-space:nowrap;font-size:11px;">' + esc(id) + '</span>' },
  { cls: 'TSpeedButton', base: 'SpeedButton', label: 'SpeedButton', note: '按鈕', icon: 'symbol-event',
    html: (id, x, y, w, h) => '<button class="btn3d" id="' + id + '" title="' + id + ' : TSpeedButton" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 75) + 'px;height:' + (h || 25) + 'px;font-size:11px;">' + esc(id) + '</button>' },
  { cls: 'TEdit', base: 'Edit', label: 'Edit', note: '輸入框', icon: 'symbol-string',
    html: (id, x, y, w, h) => '<span style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 121) + 'px;height:' + (h || 21) + 'px;"><input class="ed" id="' + id + '" title="' + id + ' : TEdit" value="" style="width:100%;height:100%;box-sizing:border-box;"></span>' },
  { cls: 'TCheckBox', base: 'CheckBox', label: 'CheckBox', note: '核取方塊', icon: 'check',
    html: (id, x, y, w, h) => '<label class="ckb" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 97) + 'px;height:' + (h || 17) + 'px;" title="' + id + ' : TCheckBox"><input type="checkbox" >' + esc(id) + '</label>' },
  { cls: 'TComboBox', base: 'ComboBox', label: 'ComboBox', note: '下拉選單', icon: 'list-selection',
    html: (id, x, y, w, h) => '<select class="ed" id="' + id + '" title="' + id + ' : TComboBox" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 145) + 'px;height:' + (h || 24) + 'px;"></select>' },
  { cls: 'TPanel', base: 'Panel', label: 'Panel', note: '面板（容器）', icon: 'layout-panel',
    html: (id, x, y, w, h) => '<div class="pnl" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 41) + 'px;background:var(--form-bg,#ece9d8);border:1px outset #ddd;" title="' + id + ' : TPanel"><span class="pnlCap" style="color:#000000;font-size:11px;font-weight:400;">' + esc(id) + '</span></div>' },
  { cls: 'TGroupBox', base: 'GroupBox', label: 'GroupBox', note: '群組框（容器）', icon: 'group-by-ref-type',
    html: (id, x, y, w, h) => '<fieldset class="gbx" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 105) + 'px;--gbi:2px 3px 3px 2px;" title="' + id + ' : TGroupBox"><legend style="background:var(--panel,#ece9d8);">' + esc(id) + '</legend><div class="cli" style="position:absolute;inset:0;overflow:hidden;"></div></fieldset>' },
  // AI(W906-IOWIDGET) 20261001: the company's own IO components (elec\Component, elec\myvcl; hwidgets.js) as the
  // generator writes them -- a lamp span.aled with --led-on / --led-off, a panel button div.btnpanel with the four
  // --bp-* colours; "｜Alias=" left empty for the grid's Alias row (the IO table's names). Their default sizes and
  // colours are the ones the golden .dfm files use most (TMyLedLane 22x14 LEDHorizontal, TMyLed 23x23 LEDSqLarge,
  // TBtnPanelLane TrueColor 14464261 / FalseColor 10307329, Style tsFlatButtons).
  { cls: 'TMyLedLane', base: 'MyLedLane', label: 'MyLedLane', note: 'IO 燈（Alias＝輸入點）', icon: 'lightbulb',
    html: (id, x, y, w, h) => '<span class="aled LEDHorizontal" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 22) + 'px;height:' + (h || 14) + 'px;--led-on:#00ff00;--led-off:#c0c0c0;border-radius:2px;" title="' + id + ' : TMyLedLane｜Alias="></span>' },
  { cls: 'TMyLed', base: 'MyLed', label: 'MyLed', note: 'IO 燈（方形；Alias＝輸入點）', icon: 'lightbulb',
    html: (id, x, y, w, h) => '<span class="aled LEDSqLarge" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 23) + 'px;height:' + (h || 23) + 'px;--led-on:#00ff00;--led-off:#c0c0c0;border-radius:2px;" title="' + id + ' : TMyLed｜Alias="></span>' },
  { cls: 'TALed', base: 'ALed', label: 'ALed', note: '燈（沒有 Alias，由程式設亮滅）', icon: 'circle-filled',
    html: (id, x, y, w, h) => '<span class="aled LEDHorizontal" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 22) + 'px;height:' + (h || 14) + 'px;--led-on:#00ff00;--led-off:#c0c0c0;border-radius:2px;" title="' + id + ' : TALed"></span>' },
  { cls: 'TBtnPanelLane', base: 'BtnPanelLane', label: 'BtnPanelLane', note: 'IO 按鈕（Alias＝輸出點，按一下切換）', icon: 'symbol-boolean',
    html: (id, x, y, w, h) => '<div class="btnpanel flat" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 75) + 'px;height:' + (h || 25) + 'px;--bp-true:#05b5dc;--bp-false:#01479d;--bp-true-font:#ffffff;--bp-false-font:#ffffff;min-width:0;min-height:0;margin:0;padding:0;font-size:11px;" title="' + id + ' : TBtnPanelLane｜Alias=">' + esc(id) + '</div>' },
  { cls: 'TBtnPanel', base: 'BtnPanel', label: 'BtnPanel', note: '面板按鈕（Alias＝輸出點，舊 ISA 卡）', icon: 'symbol-boolean',
    html: (id, x, y, w, h) => '<div class="btnpanel flat" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 75) + 'px;height:' + (h || 25) + 'px;--bp-true:#05b5dc;--bp-false:#01479d;--bp-true-font:#ffffff;--bp-false-font:#ffffff;min-width:0;min-height:0;margin:0;padding:0;font-size:11px;" title="' + id + ' : TBtnPanel｜Alias=">' + esc(id) + '</div>' },
];

/** The first free BCB6-style name: Label1, Label2 ... */
function newName(base, used) {
  for (let k = 1; ; k++) {
    const c = base + k;
    if (!used.has(c)) return c;
  }
}

function itemOf(cls) { return ITEMS.find(i => i.cls === cls) || null; }

/** The VCL classes a new component can be put into (else: next to the selection, in its parent). */
const CONTAINERS = /^(TPanel|TGroupBox|TTabSheet|TScrollBox|TFrame)$/;

module.exports = { ITEMS, itemOf, newName, CONTAINERS };
