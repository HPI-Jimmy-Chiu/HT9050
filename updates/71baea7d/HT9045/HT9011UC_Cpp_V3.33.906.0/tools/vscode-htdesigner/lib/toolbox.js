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
    html: (id, x, y, w, h) => '<select class="ed" id="' + id + '" title="' + id + ' : TComboBox" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 145) + 'px;height:' + (h || 24) + 'px;"><option>' + esc(id) + '</option></select>' },   // (1007 audit: one <option> as the generator writes; BCB6's Text = the name)
  { cls: 'TPanel', base: 'Panel', label: 'Panel', note: '面板（容器）', icon: 'layout-panel',
    html: (id, x, y, w, h) => '<div class="pnl" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 41) + 'px;background:var(--form-bg,#ece9d8);border:1px outset #ddd;" title="' + id + ' : TPanel"><span class="pnlCap" style="color:#334455;font-size:11px;font-weight:400;">' + '</span></div>' },
  { cls: 'TGroupBox', base: 'GroupBox', label: 'GroupBox', note: '群組框（容器）', icon: 'group-by-ref-type',
    html: (id, x, y, w, h) => '<fieldset class="gbx" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 105) + 'px;--gbi:2px 3px 3px 2px;" title="' + id + ' : TGroupBox"><legend style="background:var(--panel,#ece9d8);">' + esc(id) + '</legend><div class="cli" style="position:absolute;inset:0;overflow:hidden;"></div></fieldset>' },
  // 0.153 (the WPF gap list G2: the Toolbox has every common control): the rest of the VCL ones the pages use, written as
  // the generator writes them (copied from the pages: Data.SmartDiagnostic TPageControl, Config.Configuration TRadioGroup /
  // TRadioButton, Alert.Note TImage / TButton, HW.IoSetView TShape, Data.Observer TBevel, Alert.MyMessageBox TMemo)
  { cls: 'TButton', base: 'Button', label: 'Button', note: '按鈕（一般）', icon: 'symbol-event', cat: '常用',
    html: (id, x, y, w, h) => '<button class="btn3d" id="' + id + '" title="' + id + ' : TButton" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 75) + 'px;height:' + (h || 25) + 'px;font-size:11px;">' + esc(id) + '</button>' },
  { cls: 'TBitBtn', base: 'BitBtn', label: 'BitBtn', note: '按鈕（可放圖）', icon: 'symbol-event', cat: '常用',
    html: (id, x, y, w, h) => '<button class="btn3d" id="' + id + '" title="' + id + ' : TBitBtn" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 75) + 'px;height:' + (h || 25) + 'px;font-size:11px;">' + esc(id) + '</button>' },
  { cls: 'TRadioButton', base: 'RadioButton', label: 'RadioButton', note: '單選鈕', icon: 'circle-large-outline', cat: '常用',
    html: (id, x, y, w, h) => '<label class="ckb" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 113) + 'px;height:' + (h || 17) + 'px;" title="' + id + ' : TRadioButton"><input type="radio" >' + esc(id) + '</label>' },
  { cls: 'TMemo', base: 'Memo', label: 'Memo', note: '多行輸入框', icon: 'note', cat: '常用',
    html: (id, x, y, w, h) => '<textarea class="ed" id="' + id + '" title="' + id + ' : TMemo" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 89) + 'px;resize:none;">' + esc(id) + '</textarea>' },
  { cls: 'TRadioGroup', base: 'RadioGroup', label: 'RadioGroup', note: '單選群組（兩個選項）', icon: 'list-unordered', cat: '容器',
    html: (id, x, y, w, h) => '<fieldset class="gbx rg" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 105) + 'px;" title="' + id + ' : TRadioGroup"><legend style="background:var(--form-bg,#ece9d8);">' + esc(id) + '</legend><div class="cli" style="position:absolute;left:4px;top:14px;right:2px;bottom:2px;display:grid;grid-template-rows:repeat(2,1fr);grid-auto-flow:column;align-content:start;"><label class="rgi"><input type="radio" name="rg_' + id + '" checked>Item1</label><label class="rgi"><input type="radio" name="rg_' + id + '" >Item2</label></div></fieldset>' },
  { cls: 'TPageControl', base: 'PageControl', label: 'PageControl', note: '分頁（兩頁，容器）', icon: 'browser', cat: '容器',
    // (1009 second review (props #1 #2): its own placing written on it -- 39 of the 83 pages have no .pcBody / .pcPane rule
    //  and a drop into a sheet landed a tab strip higher; the sheets named TabSheetN free on the page (opts.sheets), not
    //  id+'Sheet1' -- a second PageControl1 after a rename took the first one's sheet names and drops went into it)
    html: (id, x, y, w, h, opts) => { const sh = (opts && opts.sheets) || [id + 'Sheet1', id + 'Sheet2']; return '<div class="pcWrap" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 289) + 'px;height:' + (h || 193) + 'px;box-sizing:border-box;" title="' + id + ' : TPageControl"><div class="tabs pcTabs" style="display:flex;flex-wrap:wrap;gap:2px;padding:2px 2px 0;position:relative;z-index:2;align-items:end;">' +
      '<div class="tab act" data-t="0" title="' + sh[0] + ' : TTabSheet">' + (opts && opts.sheets ? sh[0] : 'TabSheet1') + '</div><div class="tab" data-t="1" title="' + sh[1] + ' : TTabSheet">' + (opts && opts.sheets ? sh[1] : 'TabSheet2') + '</div></div>' +
      '<div class="pcBody" style="position:absolute;left:2px;right:2px;top:26px;bottom:2px;border:1px solid var(--pane-border,#999);background:var(--form-bg,#ece9d8);"><div class="pcPane" data-p="0" title="' + sh[0] + '" style="position:absolute;left:0;right:0;top:6px;bottom:0;overflow:hidden;display:block;"></div><div class="pcPane" data-p="1" title="' + sh[1] + '" style="position:absolute;left:0;right:0;top:6px;bottom:0;overflow:hidden;display:none;"></div></div></div>'; } },
  { cls: 'TImage', base: 'Image', label: 'Image', note: '圖片（先放空的，src 在屬性填）', icon: 'file-media', cat: '其他',
    html: (id, x, y, w, h) => '<img src="" alt="" id="' + id + '" title="' + id + ' : TImage" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 105) + 'px;height:' + (h || 105) + 'px;outline:1px dashed #888;">' },
  { cls: 'TShape', base: 'Shape', label: 'Shape', note: '色塊／框', icon: 'primitive-square', cat: '其他',
    html: (id, x, y, w, h) => '<div style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 65) + 'px;height:' + (h || 65) + 'px;background:#ffffff;border:1px solid #666;" id="' + id + '" title="' + id + ' : TShape"></div>' },
  { cls: 'TBevel', base: 'Bevel', label: 'Bevel', note: '分隔線（凹線）', icon: 'dash', cat: '其他',
    html: (id, x, y, w, h) => '<div style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 3) + 'px;border:1px inset #ccc;" id="' + id + '" title="' + id + ' : TBevel"></div>' },
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
  // AI(W906-HTDESIGNER) 20261008 (feature gap #1: the classes the pages use most that the Toolbox did not have -- placed
  // only by copying one): the markup the generator writes for each (taken from Setup.BarCode / Setup.Temp_Set /
  // Setup.SetUp / Setup.Speed / Main.CommView / Config.Configuration / HW.MotorTest / Alert.Note / HW.IoSetView)
  { cls: 'TLabeledEdit', base: 'LabeledEdit', label: 'LabeledEdit', note: '有標題的輸入框（EditLabel 在上面）', icon: 'symbol-string', cat: '常用',
    html: (id, x, y, w, h) => '<span style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 121) + 'px;height:' + (h || 24) + 'px;"><span class="elab" style="position:absolute;top:-13px;left:0;">' + esc(id) + '</span><input class="ed" id="' + id + '" title="' + id + ' : TLabeledEdit" value="" style="width:100%;height:100%;box-sizing:border-box;"></span>' },
  { cls: 'TListBox', base: 'ListBox', label: 'ListBox', note: '清單', icon: 'list-flat', cat: '常用',
    html: (id, x, y, w, h) => '<div class="lbx" id="' + id + '" title="' + id + ' : TListBox" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 121) + 'px;height:' + (h || 97) + 'px;background:var(--input-bg,#fff);border:1px inset #ccc;overflow:auto;font-size:11px;"></div>' },   // (1008 review: the generator's lbx -- lstBox had no rule anywhere: no background, no frame, not seen)
  { cls: 'TStringGrid', base: 'StringGrid', label: 'StringGrid', note: '表格（3×3）', icon: 'table', cat: '常用',
    html: (id, x, y, w, h) => '<div class="sgd" id="' + id + '" title="' + id + ' : TStringGrid" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 320) + 'px;height:' + (h || 120) + 'px;overflow:auto;"><table><tr><th>--</th><th>--</th><th>--</th></tr><tr><td>--</td><td>--</td><td>--</td></tr><tr><td>--</td><td>--</td><td>--</td></tr></table></div>' },
  { cls: 'TTrackBar', base: 'TrackBar', label: 'TrackBar', note: '滑桿（1～100）', icon: 'settings', cat: '常用',
    html: (id, x, y, w, h) => '<input type="range" class="trk" id="' + id + '" title="' + id + ' : TTrackBar" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 150) + 'px;height:' + (h || 33) + 'px;" min="1" max="100" value="1">' },
  { cls: 'TScrollBar', base: 'ScrollBar', label: 'ScrollBar', note: '捲軸（1～100）', icon: 'arrow-both', cat: '其他',
    html: (id, x, y, w, h) => '<input type="range" id="' + id + '" title="' + id + ' : TScrollBar" min="1" max="100" step="1" value="1" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 121) + 'px;height:' + (h || 17) + 'px;">' },
  { cls: 'TDateTimePicker', base: 'DateTimePicker', label: 'DateTimePicker', note: '日期時間', icon: 'calendar', cat: '其他',
    html: (id, x, y, w, h) => '<input class="ed" id="' + id + '" title="' + id + ' : TDateTimePicker" value="" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 109) + 'px;height:' + (h || 24) + 'px;">' },
  { cls: 'TScrollBox', base: 'ScrollBox', label: 'ScrollBox', note: '可捲動的容器', icon: 'window', cat: '容器',
    html: (id, x, y, w, h) => '<div class="pnl" id="' + id + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 185) + 'px;height:' + (h || 120) + 'px;overflow:auto;" title="' + id + ' : TScrollBox"><span class="pnlCap" style="font-size:11px;"></span></div>' },
  { cls: 'TTMyTray', base: 'TMyTray', label: 'MyTray', note: 'Tray 盤（5×8，格子在屬性改）', icon: 'symbol-array', cat: 'IO 元件',
    html: (id, x, y, w, h) => {
      const W = w || 52, H = h || 98;
      const tray = '{"name": "' + id + '", "xitem": 5, "yitem": 8, "xblockItem": 0, "yblockItem": 0, "w": ' + W + ', "h": ' + H + ', "trayColor": "#808040", "trayDirect": "csNull"}';
      return '<div class="traypos" id="' + id + '" data-tray="' + esc(tray) + '" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + W + 'px;height:' + H + 'px;" title="' + id + ' : TTMyTray"></div>';
    } },
  { cls: 'TMyLabeledLedLane', base: 'MyLabeledLedLane', label: 'MyLabeledLedLane', note: 'IO 燈＋標題（Alias＝輸入點）', icon: 'lightbulb', cat: 'IO 元件',
    html: (id, x, y, w, h) => '<span class="lled lpTop" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 70) + 'px;height:' + (h || 32) + 'px;" title="' + id + ' : TMyLabeledLedLane｜Caption=' + esc(id) + ' CaptionPos=lpTop｜Alias="><span class="aled LEDHorizontal" id="' + id + '" style="position:absolute;left:0px;top:18px;width:22px;height:14px;--led-on:#00ff00;--led-off:#c0c0c0;border-radius:2px;" title="' + id + ' : TMyLedLane｜Alias="></span><span class="lledCap" style="position:absolute;left:0px;top:0px;font-size:10px;white-space:nowrap;" title="lbl' + id + ' : TLabel">' + esc(id) + '</span></span>' },
  { cls: 'TLabeledALed', base: 'LabeledALed', label: 'LabeledALed', note: '燈＋標題（沒有 Alias）', icon: 'circle-filled', cat: 'IO 元件',
    html: (id, x, y, w, h) => '<span class="lled lpTop" style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + (w || 22) + 'px;height:' + (h || 39) + 'px;" title="' + id + ' : TLabeledALed｜Caption=' + esc(id) + ' CaptionPos=lpTop"><span class="aled LEDHorizontal" id="' + id + '" style="position:absolute;left:0px;top:25px;width:22px;height:14px;--led-on:#00ff00;--led-off:#c0c0c0;border-radius:2px;" title="' + id + ' : TALed"></span><span class="lledCap" style="position:absolute;left:0px;top:0px;font-size:14px;white-space:nowrap;" title="lbl' + id + ' : TLabel">' + esc(id) + '</span></span>' },
];

/** The first free BCB6-style name: Label1, Label2 ... */
function newName(base, used) {
  for (let k = 1; ; k++) {
    const c = base + k;
    if (!used.has(c)) return c;
  }
}

function itemOf(cls) { return ITEMS.find(i => i.cls === cls) || null; }

/** 0.153 the Toolbox's categories (WPF: Common WPF Controls / All WPF Controls; Visual Studio's groups), in this order. */
const CATS = ['常用', '容器', 'IO 元件', '其他'];
function catOf(it) {
  if (it.cat) return it.cat;
  if (/^T(MyLed|ALed|BtnPanel)/.test(it.cls)) return 'IO 元件';
  if (/^(TPanel|TGroupBox)$/.test(it.cls)) return '容器';
  return '常用';
}

/** The VCL classes a new component can be put into (else: next to the selection, in its parent). */
const CONTAINERS = /^(TPanel|TGroupBox|TTabSheet|TScrollBox|TFrame)$/;

module.exports = { ITEMS, itemOf, newName, CONTAINERS, CATS, catOf };
