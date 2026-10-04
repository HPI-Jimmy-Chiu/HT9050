/* ht9045_hsys_table_c.js -- HW.HandlerSys: the table layout of D:\HT9045\page_Old\HW.HandlerSys.html on the C-route page.
 *
 * AI(W906-HSYS-TABLE) 20261002 (St01 ST01-E2): Steven「我比較喜歡 D:\HT9045\page_Old\HW.HandlerSys.html 這個版面」.
 *   Ported from page_Old handler-settings-table.js / handler-track-table.js / handler-comport-table.js and its tab-order script
 *   (page_Old is a read-only reference outside git). Styles: ht9045_hsys_table.css.
 *
 *   The original DFM controls stay the binding authority: the C-route engine (ht9045_wire_engine.js) reads and writes only them.
 *   The tables are proxies:
 *     original -> table  every 500 ms and right after any input / change / click outside the tables
 *                        (value, select options, enabled, read-only, golden Visible)
 *     table -> original  through the event an operator's own click or typing makes (radio / checkbox click, select change,
 *                        text input + change), so golden handlers on the originals run (ht9045_hsys_events_c.js rgTTLCardClick,
 *                        rgRotateKit_TypeClick ..., ht9045_hsys_heater_c.js rgHeaterTypeClick).
 *   Differences from page_Old (A route):
 *     - no HTSettings / JSON export (restoreExportedState, bindFix3FullTray, collect): the C route saves the originals only.
 *     - Machine Track only shows / hides rows. page_Old also unticked Empty / Color, reset Unloader Cover Tray ID and Auto Can Go
 *       Rear every 500 ms; golden HandlerSys has no MachineTrackClick, so values are left as they are.
 *     - AUTO 1..6 ART (rgAuto1ART..rgAuto6ART): page_Old hid them; Steven 1003 (B69, "Go") put them back as an "ART" tick at the
 *       top of each Auto row's option list. golden shows them always and never greys them: 906 0618 HandlerSys.cpp:134-139 load,
 *       :543-559 save, and :544 `if(USE_AUTO_RETEST)` takes the six values only when ART Function (rgInstallAutoRestest) is Install
 *       (V912 :665-679 the same) -- so with ART Function Un Install a changed tick is not saved; golden behaviour, kept.
 *     - settings golden does not have (Fix 1/2/4/5/6 install, Auto 4-6 Y motor, Auto 4-6 cylinder / magazine) are N/A, not
 *       editable; Fix 3 "full tray" is [E55] cbE55 on the Configuration page, greyed here.
 *     - Com Port "Set Default" presses golden btnSetATCCom (ht9045_hsys_events_c.js) instead of keeping its own list.
 *     - no handler-search.js / handler-customer-search.js: golden edtSearchFunction / edtSearchCode handlers are in
 *       ht9045_hsys_events_c.js; originals hidden behind the tables still show when Search Function moves them to its box.
 *     - text boxes open the same on-screen keypad as the original (the mousedown is passed on to the original box).
 *     - 2nd / 3rd Socket sensor amplifier qty (cbSocketSenAmpCnt2nd / 3rd) and rgAutoFormSize, which page_Old's tables left out
 *       (hidden with no proxy), have rows again.
 *   Not changed: Customer Code, Search Function tabs.
 *
 * AI(W906-HSYS-TABLE) 20261002 (St01 ST01-E2, todo E-029 layout half): Steven 1002 16:4x「heater type 看起來已經整合到 Heater頁面」
 *   「Index Heater Count也是整合到 Heater頁面」「把整個Heater頁面搬到 Temperature Setting 與 Tri Temperature的中間」:
 *   - grpHeater (tsHeater) moves into the Temperature tab between Temperature Settings and Tri Temperature; the tab scrolls as one page.
 *   - Heater Type (rgHeaterType) and Index Heater Counts (rgHeater) are proxy rows at the top of that section, no longer rows of
 *     Temperature Settings (same write-back through real events, so ht9045_hsys_heater_c.js's rgHeaterTypeClick hook still runs).
 *   - the Heater tab is hidden by class, not removed: the engine still applies C++'s TabVisible to it (style.display,
 *     FileRW/_EditList.cpp:105) and the section follows that, and golden Visible on grpHeater.
 *   ht9045_hsys_heater_c.js (ST01-E's engineer) still builds into grpHeater .cli and touches only the original controls.
 */
(function () {
  'use strict';
  function $(id) { return id ? document.getElementById(id) : null; }
  function fire(el, type) {
    var e;
    try { e = new Event(type, { bubbles: true }); } catch (x) { e = document.createEvent('Event'); e.initEvent(type, true, true); }
    el.dispatchEvent(e);
  }
  function esc(s) { return String(s).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); }
  function text(value) { return (value || '').replace(/\s+/g, ' ').trim(); }
  function legendOf(el) { var lg = el && el.querySelector('legend'); return text(lg && lg.textContent); }
  function radios(el) { return el ? Array.prototype.slice.call(el.querySelectorAll('input[type="radio"]')) : []; }
  function checkedIndex(el) { var r = radios(el); for (var i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; }
  function boxOf(el) { return !el ? null : el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]'); }
  function hide(el) { if (el && el.classList) el.classList.add('ht-src-hidden'); }

  // ---- table -> original: the same event the operator's click / typing on the original makes ----
  function pick(el, i) { var r = radios(el)[i]; if (r && !r.checked) r.click(); }
  function check(el, on) { var b = boxOf(el); if (b && !!b.checked !== !!on) b.click(); }
  function choose(el, i) { if (el && el.selectedIndex !== i) { el.selectedIndex = i; fire(el, 'change'); } }
  function typeIn(el, v) { if (el && el.value !== v) { el.value = v; fire(el, 'input'); fire(el, 'change'); } }
  function writeBack(proxy, source) {
    if (!source) return;
    if (source.matches('fieldset.rg')) pick(source, proxy.selectedIndex);
    else if (source.matches('label.ckb') || source.type === 'checkbox') check(source, proxy.checked);
    else if (source.tagName === 'SELECT') choose(source, proxy.selectedIndex);
    else typeIn(source, proxy.value);
  }

  // ---- original -> table ----
  function sameOptions(p, s) {
    if (p.options.length !== s.options.length) return false;
    for (var i = 0; i < s.options.length; i++) if (p.options[i].text !== s.options[i].text) return false;
    return true;
  }
  function copyOptions(p, s) {     // the engine adds options the page did not have (cpp-text / cpp-item, ht9045_wire_engine.js gbApply)
    if (sameOptions(p, s)) return;
    while (p.options.length) p.remove(0);
    for (var i = 0; i < s.options.length; i++) p.add(new Option(s.options[i].text, s.options[i].value));
  }
  function readInto(proxy, source) {
    if (!source) return;
    if (source.matches('fieldset.rg')) proxy.selectedIndex = checkedIndex(source);
    else if (source.matches('label.ckb') || source.type === 'checkbox') { var b = boxOf(source); proxy.checked = !!(b && b.checked); }
    else if (source.tagName === 'SELECT') { copyOptions(proxy, source); proxy.selectedIndex = source.selectedIndex; }
    else if (proxy.value !== source.value) proxy.value = source.value;
  }
  function isOff(src) {            // golden Enabled=false on the original or a container (engine gbSetEnabled / events setEnabled)
    if (src.closest('[aria-disabled="true"]')) return true;
    if (src.matches('fieldset.rg')) { var r = radios(src); return !r.length || r.every(function (x) { return x.matches(':disabled'); }); }
    var b = src.matches('label.ckb') ? boxOf(src) : src;
    return !b || b.matches(':disabled');
  }
  function isGone(src) {           // golden Visible=false: engine style.visibility, or the DFM's own display:none
    if (src.style.display === 'none') return true;
    try { return getComputedStyle(src).visibility === 'hidden'; } catch (e) { return false; }
  }
  function mirrorAll() {
    var ps = document.querySelectorAll('.ht-proxy-root [data-hst-src]');
    for (var i = 0; i < ps.length; i++) {
      var p = ps[i], s = $(p.getAttribute('data-hst-src'));
      if (!s) continue;
      var off = isOff(s);
      if (p.disabled !== off) p.disabled = off;
      if (p.tagName === 'SELECT' && !p.hasAttribute('data-hst-combined') && s.matches('fieldset.rg')) {
        var r = radios(s);
        for (var k = 0; k < p.options.length && k < r.length; k++) { var d = r[k].matches(':disabled'); if (p.options[k].disabled !== d) p.options[k].disabled = d; }
      }
      if (p.type === 'text' && p.readOnly !== !!s.readOnly) p.readOnly = !!s.readOnly;
      var t = s.getAttribute('title');
      if (t !== null && p.title !== t) p.title = t;
      var unit = p.closest('.ht-three-column-inline,.ht-four-column-inline,label') || p;
      unit.classList.toggle('ht-proxy-gone', isGone(s));
    }
    var trs = document.querySelectorAll('.ht-proxy-root tr');
    for (var j = 0; j < trs.length; j++) {
      var q = trs[j].querySelectorAll('[data-hst-src]');
      if (!q.length) continue;
      var all = true;
      for (var m = 0; m < q.length; m++) if (!q[m].closest('.ht-proxy-gone')) { all = false; break; }
      trs[j].classList.toggle('ht-row-gone', all);
    }
  }
  var SYNCS = [];
  function syncAll() {
    for (var i = 0; i < SYNCS.length; i++) {
      try { SYNCS[i](); } catch (e) { if (window.console) console.error('[HSys table] sync', e); }
    }
    try { mirrorAll(); } catch (e2) { if (window.console) console.error('[HSys table] mirror', e2); }
  }
  var pending = false;
  function soon() { if (!pending) { pending = true; setTimeout(function () { pending = false; syncAll(); }, 0); } }

  // text proxies: the original's keypad (engine attachKeyboards binds mousedown on the original and makes it read-only)
  function bindText(proxy) {
    var source = $(proxy.getAttribute('data-hst-src'));
    if (!source) return;
    proxy.addEventListener('mousedown', function (ev) {
      if (!source.readOnly) return;                  // no keypad on the original -> type in the table (written back on input)
      ev.preventDefault();
      if (proxy.disabled) return;
      var e;
      try { e = new MouseEvent('mousedown', { bubbles: true, cancelable: true, button: 0 }); } catch (x) { return; }
      source.dispatchEvent(e);
    });
    proxy.addEventListener('input', function () { typeIn(source, proxy.value); });
  }
  // the tables' own search boxes: generic keypad (no physical keyboard on the machine)
  function bindSearch(input) {
    if (!input || typeof HTQwerty === 'undefined') return;
    input.readOnly = true;
    input.style.cursor = 'pointer';
    input.addEventListener('mousedown', function (ev) {
      ev.preventDefault();
      HTQwerty.show(input, 0, { onCommit: function () { fire(input, 'input'); } });
    });
  }

  /* ================= Settings tables (page_Old handler-settings-table.js) ================= */
  var TABS = [
    { pane: '0', id: 'htHandlerSettings', title: 'Handler Settings', excludeContainers: ['GroupBox1', 'GroupBox2'], informationContainer: 'GroupBox3', informationId: 'htHandlerInformation', thirdColumnTitle: 'Option 2', linkedRadioGroups: { rgTTLCard: 'rgTTLUseAddress' }, linkedOptionLabels: { rgTTLCard: 'Address' }, mergedRows: [['rgUseSensor_9046_DB', 'rgUseSucker_9046_DB'], ['rgIOCard', 'rgMNetSpeed']], mergedRowLabels: ['HT9045 DB Setting', 'I/O Card'], mergedRowItemLabels: [['Sensor_9046.DB', 'Sucker_9046.DB'], ['', 'MotionNet Speed']], extraSourceIds: ['rgSafeDoor', 'ed24VMonitorPulseCount'], extraHiddenIds: ['rgSafeDoor', 'GroupBox7'] },
    { pane: '2', id: 'htInOutArmTables', multiTables: [
      { id: 'htInOutArmCore', title: 'In / Out Arm', fourColumnRows: [
        ['ArmZAtoZH'],
        ['rgPickerCount', 'edtHPLimit'],
        ['rgInOutArmXPitch', 'edtMaxXPitch', 'edtMinXPitch'],
        ['rgInOutArmYPitch', 'edtMaxYPitch', 'edtMinYPitch'],
        ['rgOutArmYPitch'],
        ['rgOutSortArm', 'edtOutSortXPitchMax', 'edtOutSortXPitchMin']
      ], labels: { rgOutSortArm: 'Out Sort Picker Count', edtOutSortXPitchMin: 'Min X Pitch', edtOutSortXPitchMax: 'Max X Pitch' } },
      { id: 'htInOutArmRotateKit', title: 'Rotate Kit', containerId: 'gbRotateKit', combinedRotateKit: true },
      { id: 'htInOutArmOther', title: 'Other In / Out Arm Settings', sortable: true, mergedThreeColumnRows: [['rgPreciser', 'rgPreciserPos'], ['rgHotplateType', 'rgHotPlatePos']], mergedThreeColumnLabels: ['Preciser', 'Hot Plate Type'], threeColumnItemLabels: [null, ['', 'Pin Position:']], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] }
    ] },
    { pane: '4', id: 'htShuttleTables', multiTables: [
      { id: 'htShuttleSettings', title: 'Shuttle Settings', threeColumn: true, mergedThreeColumnRows: [['rgShuttleSensor', 'rgNUECType'], ['rgUseOutSht', 'cbEnableOutShtSensor']], mergedThreeColumnLabels: ['Shuttle Sensor Type', 'Use Individual Out Shuttle'], threeColumnItemLabels: [['', 'NU-EC Type:'], ['', '']], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] },
      { id: 'htShuttle2DIDSettings', title: '2DID Setting', containerId: 'pnlSht2' }
    ], extraSourceIds: ['rgShuttleFloodgate'], extraHiddenIds: ['rgShuttleFloodgate'] },
    { pane: '3', id: 'htIndexTables', multiTables: [
      { id: 'htIndexSettings', title: 'Index Settings', threeColumn: true, mergedThreeColumnRows: [
        ['rgIndexMotorType', 'rgIndexMotorAxis'],
        ['rgIndexSuckerType', 'rgIndexPressType'],
        ['ElectronPressure', 'rgCleanAir'],
        ['edMaxKpa', 'edMinMpa'],
        ['edMaxMpaFB', 'edtMinMpaFB'],
        ['rgDoubleEPControl', 'cbIndEPCnt'],
        ['chkUser_Define_IndexZ_SafePos', 'edtUserDefineIndexZSafePos'],
        ['chkUserDefMaxContactHeight', 'edtUserDefMaxContactHeight']
      ], mergedThreeColumnLabels: ['Index Motor', 'Index Option', 'Electron Pressure', 'EP Setting for Output', 'EP Setting for Feedback', 'Dual EP Control', 'User Define Index Z SafePos', 'User define max contact height'], threeColumnItemLabels: [['Type:', 'Axis Count:'], ['Vacuum Type:', 'Pressure Type:'], ['', 'Clean Air:'], ['Max KPA:', 'Min MPA:'], ['Max VDC:', 'Min VDC:'], ['', 'Ind. EP Count:'], ['', 'mm'], ['', 'mm']], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] },
      { id: 'htIndexSensorSettings', title: 'Socket / Rotate / Color Sensor', containerId: 'grpOther', threeColumnRows: [
        ['rgSocketSen', 'cbSocketSenAmpCnt'],
        ['rgColorSensor', 'cbColorSenAmpCnt'],
        ['cbRotateSenAmpCnt'],
        ['cbSocketSenAmpCnt2nd'],
        ['cbSocketSenAmpCnt3rd']
      ], rowLabels: ['Socket sensror', 'Use Color Tray Sensor', 'Rotate sensor', '2nd Socket sensor amplifier qty', '3rd Socket sensor amplifier qty'], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] },
      { id: 'htIndexCanBusSettings', title: 'CanBus Setting', containerId: 'grpOther', sourceIds: ['rgCanBusMethod', 'coCanBusNudn1', 'coNudn1Macid11', 'coNudn1Macid12', 'coNudn1Macid13', 'coNudn1Macid14'], labels: {
        rgCanBusMethod: 'Method',
        coCanBusNudn1: 'NUDN1 qty',
        coNudn1Macid11: 'NUDN1 MACID11 amplifier qty',
        coNudn1Macid12: 'NUDN1 MACID12 amplifier qty',
        coNudn1Macid13: 'NUDN1 MACID13 amplifier qty',
        coNudn1Macid14: 'NUDN1 MACID14 amplifier qty'
      } }
    ], externalContainerIds: ['grpOther'], excludeSourceIds: ['rgATC', 'rgHeaterType', 'edATCSystemIP', 'edATCSystemPort', 'edATCSystemUseHeat', 'rgHeater', 'rgATCMixMode', 'rgRealTimeCCD', 'rgRealTimeCCDTempNum', 'rgUse4DUT', 'rgLBTemp', 'rgLBTemp2', 'rgESDTemp', 'rgAirConditioner', 'rgATCHeatGun', 'rgHighTempLimit', 'rgHeatGun', 'GroupBox8', 'rgHotGunFlow', 'edHotGunFlow_LineNo', 'edHotGunFlow_DevNo', 'edHotGunFlow_Gun1_ChannelNo', 'edHotGunFlow_Gun2_ChannelNo', 'rgDewpointHW'] },
    { pane: '5', id: 'htTemperatureTables', multiTables: [
      { id: 'htTemperatureSettings', title: 'Temperature Settings', externalContainerId: 'pnlIndex2', externalSourceIds: ['rgBaseHeaterCount', 'rgRealTimeCCD', 'rgRealTimeCCDTempNum', 'rgUse4DUT', 'rgLBTemp', 'rgLBTemp2', 'rgESDTemp', 'rgAirConditioner', 'rgATCHeatGun', 'rgHighTempLimit', 'rgHeatGun', 'GroupBox8', 'rgHotGunFlow', 'edHotGunFlow_LineNo', 'edHotGunFlow_DevNo', 'edHotGunFlow_Gun1_ChannelNo', 'edHotGunFlow_Gun2_ChannelNo', 'rgDewpointHW'], sortable: true, threeColumn: true, consumedSourceIds: ['rgATCMixMode'], mergedThreeColumnRows: [['rgATC', 'rgATCHeatGun'], ['edATCSystemIP', 'edATCSystemPort'], ['edATCSystemUseHeat'], ['rgRealTimeCCD', 'rgRealTimeCCDTempNum'], ['rgLBTemp', 'rgLBTemp2'], ['rgHeatGun', 'rgHotGunFlow'], ['edHotGunFlow_Gun1_ChannelNo', 'edHotGunFlow_LineNo'], ['edHotGunFlow_Gun2_ChannelNo', 'edHotGunFlow_DevNo']], mergedThreeColumnLabels: ['ATC Mode', 'ATC Setting', 'ATC Head Count', 'RTC Temp', 'L/B Temp', 'Hot Gun', 'Hot Gun 1', 'Hot Gun 2'], threeColumnItemLabels: [['', 'Heat Gun:'], ['IP:', 'Port:'], null, ['', 'Count:'], ['1st', '2nd'], ['Type:', 'Flow:'], ['ChannelNo:', 'LineNo:'], ['ChannelNo:', 'DevNo:']], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] },
      { id: 'htTriTemperatureSettings', title: 'Tri Temperature', containerId: 'grpTriTemperature', threeColumnRows: [
        ['rgTriTempMachine'],
        ['rgAirStreamSelect'],
        ['edtTriTemperature_MaxDegree', 'edtTriTemperature_MinDegree'],
        ['edtTriTempTotalCh'],
        ['edt_Total_Compressor'],
        ['edtIndexMaxTemp'],
        ['edtBaseMaxTemp'],
        ['edtOutShtMaxTemp']
      ], rowLabels: ['Tri Temperature mode', 'Air Stream', 'Degree', 'Total Channel', 'Total Compressor', 'Set Heater Temp for index', 'Set Heater Temp for base', 'Set Heater Temp for Out Shuttle'] }
    ], externalContainerIds: ['pnlIndex2', 'pnlIndex3', 'pnlIndex4', 'pnlIndex5'], extraSourceIds: ['rg_IndexDoorHeater'], excludeContainerIds: ['grpOther', 'grpIndexZSafePos', 'grpContactHeight'], excludeSourceIds: ['rgAOI', 'rgTopScanner_AOI', 'rgScanner_AOI', 'rgMagBinDispType', 'rgE84Sensor', 'rgSafeDoor', 'ed24VMonitorPulseCount', 'rgShuttleFloodgate', 'rgCleanAir', 'rgHeaterType', 'rgHeater', 'rgIOChangeToque', 'rgIndexMotorAxis', 'rgIndexMotorType', 'rgIndexSuckerType', 'rgOTDInstall', 'rgSocketClamp', 'rgFinePitch', 'rgVibrationCommuncation', 'cbVibrationCardQty'] },
    { pane: '8', id: 'htIonFanTables', multiTables: [
      { id: 'htIonFanSettings', title: 'ION Fan Settings', threeColumn: true, mergedThreeColumnRows: [['rgIonFanType'], ['rgUsePulseType', 'edIONPulseCount'], ['rgChamberUsePulseType']], mergedThreeColumnLabels: ['ION Fan Alarm Type', 'Use ION Pulse Type', 'Use Chamber Pulse Type'], threeColumnItemLabels: [null, ['', 'ION Pulse Count:'], null], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] },
      { id: 'htGroundManSettings', title: 'Ground Man', containerId: 'grpGroundMan', threeColumnRows: [['rgGroundMan'], ['rgGroundMan_ScanPoint', 'edGroundMan_AlarmOhm']], rowLabels: ['Use Ground Man', 'Scan Point'], threeColumnItemLabels: [null, ['', 'Alarm Ohm:']], threeColumnHeaders: ['Setting', 'Option 1', 'Option 2'] }
    ] }
  ];
  var ATC_MIX_SYSTEM = 6;          // golden: ATC 3.3+6.0 = ATC_SYSTEM==eNewATCSystem (rgATC item 6) && ATC_MixMode==eMixATC60_ATC33 (1)
                                   //   (V912 ATC/ATC_Handler_Side.cpp:1123-1124, uTemp_Set.cpp:589-591; 906 :1120-1121, :588-590 -- both listed, Steven 1002 19:4x)
  function titleFromId(id) {
    return id.replace(/^(chk|cb|cbb|rg|edt|ed)/, '').replace(/([A-Z]+)([A-Z][a-z])/g, '$1 $2').replace(/([a-z0-9])([A-Z])/g, '$1 $2').replace(/_/g, ' ').trim();
  }
  function sourceKind(source) {
    if (source.matches('fieldset.rg')) return 'radio';
    if (source.matches('label.ckb')) return 'checkbox';
    return source.tagName === 'SELECT' ? 'select' : 'text';
  }
  function makeOption(item) {       // <td> whose firstChild is the proxy (a <label> around it for a checkbox)
    var option = document.createElement('td'), p;
    if (item.kind === 'radio' || item.kind === 'select') {
      p = document.createElement('select');
      if (item.kind === 'radio') radios(item.source).forEach(function (radio) { p.add(new Option(text(radio.parentNode.textContent))); });
      else Array.prototype.forEach.call(item.source.options, function (o) { p.add(new Option(o.text, o.value)); });
      if (item.source.id === 'rgATC') {
        p.setAttribute('data-atc-mode-combined', 'true');
        p.setAttribute('data-hst-combined', '1');
        p.add(new Option('ATC3.3+6.0'));
      }
      option.appendChild(p);
    } else if (item.kind === 'checkbox') {
      var lab = document.createElement('label');
      lab.className = 'ht-track-check ht-track-check-caption';
      p = document.createElement('input');
      p.type = 'checkbox';
      lab.appendChild(p);
      lab.appendChild(document.createTextNode(' ' + item.label));
      option.appendChild(lab);
    } else {
      p = document.createElement('input');
      p.type = 'text';
      option.appendChild(p);
    }
    p.setAttribute('data-setting-source', item.source.id);
    p.setAttribute('data-hst-src', item.source.id);
    return option;
  }
  function settingSync(proxies, root, extra) {
    proxies.forEach(function (table) { table.querySelectorAll('[data-setting-source]').forEach(function (proxy) {
      var source = $(proxy.getAttribute('data-setting-source'));
      if (!source) return;
      if (proxy.hasAttribute('data-rotate-kit-combined')) {
        var installIndex = checkedIndex($('rgRotateKit')), typeIndex = checkedIndex(source);
        proxy.selectedIndex = installIndex === 0 ? 0 : typeIndex + 1;
        proxy.closest('table').querySelectorAll('[data-rotate-kit-cylinder-position]').forEach(function (row) { row.style.display = proxy.selectedIndex === 1 ? '' : 'none'; });
        root.classList.toggle('ht-rotate-kit-cylinder', proxy.selectedIndex === 1);
      } else if (proxy.hasAttribute('data-atc-mode-combined')) {
        var atc = checkedIndex(source);
        proxy.selectedIndex = (atc === ATC_MIX_SYSTEM && checkedIndex($('rgATCMixMode')) === 1) ? radios(source).length : atc;
      } else readInto(proxy, source);
    }); });
    if (extra) extra();
  }
  function settingWrite(proxy, root) {
    var source = $(proxy.getAttribute('data-setting-source'));
    if (!source) return;
    if (proxy.hasAttribute('data-rotate-kit-combined')) {          // item 0 = rgRotateKit Uninstall; 1.. = Install + rgRotateKit_Type item-1
      pick($('rgRotateKit'), proxy.selectedIndex === 0 ? 0 : 1);
      if (proxy.selectedIndex > 0) pick(source, proxy.selectedIndex - 1);
      proxy.closest('table').querySelectorAll('[data-rotate-kit-cylinder-position]').forEach(function (row) { row.style.display = proxy.selectedIndex === 1 ? '' : 'none'; });
      root.classList.toggle('ht-rotate-kit-cylinder', proxy.selectedIndex === 1);
    } else if (proxy.hasAttribute('data-atc-mode-combined')) {     // last item = ATC 3.3+6.0
      var mix = proxy.selectedIndex === radios(source).length;
      pick($('rgATCMixMode'), mix ? 1 : 0);
      pick(source, mix ? ATC_MIX_SYSTEM : proxy.selectedIndex);
    } else writeBack(proxy, source);
  }
  function renderMultiTables(tab) {
    var pane = document.querySelector('.pcPane[data-p="' + tab.pane + '"]');
    if (!pane || $(tab.id)) return;
    var original = Array.prototype.slice.call(pane.children), sourceIds = {}, proxyTables = [], root = document.createElement('section');
    root.id = tab.id;
    root.className = 'ht-proxy-root';
    function makeRow(item, threeColumns) {
      var row = document.createElement('tr'), name = document.createElement('th');
      row.dataset.settingName = item.label.toLowerCase();
      name.textContent = item.label;
      row.appendChild(name);
      row.appendChild(makeOption(item));
      if (threeColumns) row.appendChild(document.createElement('td'));
      return row;
    }
    function makeFourColumnRow(items) {
      var row = document.createElement('tr'), title = document.createElement('th');
      title.className = 'ht-four-column-title';
      title.textContent = items[0].label;
      row.appendChild(title);
      items.forEach(function (item, index) {
        var cell = document.createElement('td'), label = document.createElement('label'), option = makeOption(item), content;
        cell.className = 'ht-four-column-cell';
        label.className = 'ht-four-column-label';
        if (index > 0) {
          cell.className += ' ht-four-column-cell-labeled';
          label.textContent = item.label;
          content = document.createElement('div');
          content.className = 'ht-four-column-inline';
          content.appendChild(label);
          content.appendChild(option.firstChild);
          cell.appendChild(content);
        } else {
          cell.appendChild(option.firstChild);
        }
        row.appendChild(cell);
      });
      while (row.cells.length < 4) row.appendChild(document.createElement('td'));
      return row;
    }
    function makeThreeColumnRow(items, titleText, itemLabels) {
      var row = document.createElement('tr'), title = document.createElement('th');
      title.className = 'ht-three-column-title';
      title.textContent = titleText;
      row.dataset.settingName = titleText.toLowerCase();
      row.appendChild(title);
      items.forEach(function (item, index) {
        var cell = document.createElement('td'), cellItems = Array.isArray(item) ? item : [item], cellLabels = itemLabels && itemLabels[index], compound, option, labelText, content, label;
        cell.className = 'ht-three-column-cell';
        if (cellItems.length > 1) {
          compound = document.createElement('div');
          compound.className = 'ht-three-column-compound';
          cellItems.forEach(function (cellItem, itemIndex) {
            var o = makeOption(cellItem), lt = Array.isArray(cellLabels) ? cellLabels[itemIndex] : '';
            if (lt) {
              var c = document.createElement('div'), l = document.createElement('label');
              c.className = 'ht-three-column-inline';
              l.textContent = lt;
              c.appendChild(l);
              c.appendChild(o.firstChild);
              compound.appendChild(c);
            } else compound.appendChild(o.firstChild);
          });
          cell.appendChild(compound);
        } else {
          option = makeOption(cellItems[0]);
          labelText = Array.isArray(cellLabels) ? cellLabels[0] : cellLabels;
          if (labelText) {
            content = document.createElement('div');
            label = document.createElement('label');
            content.className = 'ht-three-column-inline';
            label.textContent = labelText;
            content.appendChild(label);
            content.appendChild(option.firstChild);
            cell.appendChild(content);
          } else cell.appendChild(option.firstChild);
        }
        row.appendChild(cell);
      });
      while (row.cells.length < 3) row.appendChild(document.createElement('td'));
      return row;
    }
    function isInSection(source, section) {
      if (section.sourceIds) return section.sourceIds.indexOf(source.id) >= 0;
      return (section.externalSourceIds || []).indexOf(source.id) >= 0 || !!(section.containerId && source.closest('#' + section.containerId)) || !!(section.externalContainerId && source.closest('#' + section.externalContainerId)) || (section.containerIds || []).some(function (id) { return source.closest('#' + id); });
    }
    var allSources = [];
    function collectSources(container) {
      if (!container) return;
      container.querySelectorAll('fieldset.rg').forEach(function (source) { allSources.push({ source: source, kind: 'radio', label: legendOf(source) }); });
      container.querySelectorAll('select,input').forEach(function (control) {
        if (control.closest('fieldset.rg') || control.type === 'radio') return;
        var source = control.type === 'checkbox' ? control.closest('label.ckb') : control;
        if (source) allSources.push({ source: source, kind: sourceKind(source), label: source.matches('label.ckb') ? text(source.textContent) : titleFromId(source.id) });
      });
    }
    collectSources(pane);
    (tab.externalContainerIds || []).forEach(function (id) { collectSources($(id)); });
    (tab.extraSourceIds || []).forEach(function (id) {
      var source = $(id);
      if (source) allSources.push({ source: source, kind: sourceKind(source), label: source.matches('fieldset.rg') ? legendOf(source) : titleFromId(source.id) });
    });
    allSources = allSources.filter(function (item, index, items) {
      return item.source.id && item.source.id !== 'rgRotateKit' && item.source.id !== 'rgRotateKit_Type' && (tab.excludeSourceIds || []).indexOf(item.source.id) < 0 && !(tab.excludeContainerIds || []).some(function (id) { return item.source.closest('#' + id); }) && items.findIndex(function (candidate) { return candidate.source.id === item.source.id; }) === index;
    });
    function sourceItem(id, labels) {   // page_Old crashed the whole tab on a missing id; here the cell is just left out
      var item = allSources.find(function (candidate) { return candidate.source.id === id; });
      if (!item) { if (window.console) console.warn('[HSys table] ' + tab.id + ': no source ' + id); return null; }
      sourceIds[id] = true;
      return { source: item.source, kind: item.kind, label: (labels || {})[id] || item.label };
    }
    function present(x) { return !!x && (!Array.isArray(x) || x.length > 0); }
    tab.multiTables.forEach(function (section) {
      var table = document.createElement('section'), sources = [], tbody;
      var three = section.threeColumn || section.threeColumnRows || section.mergedThreeColumnRows;
      table.id = section.id;
      table.className = 'ht-inout-table';
      table.innerHTML = '<div class="ht-setting-head">' + (section.hideTitle ? '' : '<h2>' + esc(section.title) + '</h2>') + (section.sortable ? '<label>Sort <select data-setting-sort><option value="asc">Setting: A-Z</option><option value="desc">Setting: Z-A</option></select></label>' : '') + '</div><div class="ht-setting-scroll"><table class="ht-setting-table' + (section.fourColumnRows ? ' ht-four-column-table' : three ? ' ht-three-column-table' : '') + '">' + (section.fourColumnRows ? '<thead><tr><th>Title</th><th>Option 1</th><th>Max</th><th>Min</th></tr></thead>' : three ? '<thead><tr><th>' + (section.threeColumnHeaders || ['Setting', 'Option 1', 'Option 2']).join('</th><th>') + '</th></tr></thead>' : '<thead><tr><th>Setting</th><th>Option</th></tr></thead>') + '<tbody></tbody></table></div>';
      tbody = table.querySelector('tbody');
      if (section.fourColumnRows) {
        section.fourColumnRows.forEach(function (ids) {
          var rowItems = ids.map(function (id) { return sourceItem(id, section.labels); });
          if (rowItems[0]) tbody.appendChild(makeFourColumnRow(rowItems.filter(Boolean)));
        });
      }
      if (section.threeColumnRows) {
        section.threeColumnRows.forEach(function (ids, rowIndex) {
          var rowItems = ids.map(function (id) { return sourceItem(id); }).filter(Boolean);
          if (rowItems.length) tbody.appendChild(makeThreeColumnRow(rowItems, section.rowLabels[rowIndex], section.threeColumnItemLabels && section.threeColumnItemLabels[rowIndex]));
        });
      }
      if (section.mergedThreeColumnRows) {
        section.mergedThreeColumnRows.forEach(function (ids, rowIndex) {
          var rowItems = ids.map(function (cellIds) {
            return (Array.isArray(cellIds) ? cellIds : [cellIds]).map(function (id) { return sourceItem(id); }).filter(Boolean);
          });
          if (rowItems.some(present)) tbody.appendChild(makeThreeColumnRow(rowItems.filter(present), section.mergedThreeColumnLabels[rowIndex], section.threeColumnItemLabels && section.threeColumnItemLabels[rowIndex]));
        });
      }
      (section.consumedSourceIds || []).forEach(function (id) { sourceIds[id] = true; });
      allSources.forEach(function (item) {
        var belongs = section.fourColumnRows || section.threeColumnRows ? false : (section.containerId || section.sourceIds || section.containerIds) ? isInSection(item.source, section) : !tab.multiTables.some(function (fixedSection) { return fixedSection !== section && isInSection(item.source, fixedSection); });
        if (belongs && !sourceIds[item.source.id]) {
          sourceIds[item.source.id] = true;
          sources.push({ source: item.source, kind: item.kind, label: (section.labels || {})[item.source.id] || item.label });
        }
      });
      if (section.sourceIds) sources.sort(function (left, right) { return section.sourceIds.indexOf(left.source.id) - section.sourceIds.indexOf(right.source.id); });
      sources.forEach(function (item) { tbody.appendChild(makeRow(item, section.threeColumn || !!section.mergedThreeColumnRows)); });
      if (section.combinedRotateKit) {
        var installSource = $('rgRotateKit'), typeSource = $('rgRotateKit_Type');
        if (installSource && typeSource) {
          var typeRow = makeRow({ source: typeSource, kind: 'radio', label: legendOf(typeSource) });
          var typeProxy = typeRow.querySelector('select');
          typeProxy.setAttribute('data-rotate-kit-combined', 'true');
          typeProxy.setAttribute('data-hst-combined', '1');
          while (typeProxy.options.length) typeProxy.remove(0);
          typeProxy.add(new Option(text(installSource.querySelector('label.rgi').textContent)));
          radios(typeSource).forEach(function (radio) { typeProxy.add(new Option(text(radio.parentNode.textContent))); });
          tbody.insertBefore(typeRow, tbody.firstChild);
          Array.prototype.forEach.call(tbody.querySelectorAll('tr'), function (row) {
            var source = row.querySelector('[data-setting-source]');
            if (source && (source.getAttribute('data-setting-source') === 'rgRotateKitIn' || source.getAttribute('data-setting-source') === 'rgRotateKitOut')) row.dataset.rotateKitCylinderPosition = 'true';
          });
        }
      }
      if (section.sortable) {
        var sort = function () {
          var direction = table.querySelector('[data-setting-sort]').value;
          Array.prototype.slice.call(tbody.querySelectorAll('tr')).sort(function (left, right) { return left.dataset.settingName.localeCompare(right.dataset.settingName) * (direction === 'asc' ? 1 : -1); }).forEach(function (row) { tbody.appendChild(row); });
        };
        table.querySelector('[data-setting-sort]').addEventListener('change', sort);
        sort();
      }
      root.appendChild(table);
      proxyTables.push(table);
    });
    pane.appendChild(root);
    root.querySelectorAll('input[type="text"][data-hst-src]').forEach(bindText);
    root.addEventListener('change', function (event) {
      if (event.target.hasAttribute('data-setting-source') && event.target.type !== 'text') settingWrite(event.target, root);
    });
    original.forEach(hide);
    (tab.extraHiddenIds || []).forEach(function (id) { hide($(id)); });
    SYNCS.push(function () { settingSync(proxyTables, root); });
  }
  function renderSettings(tab) {
    if (tab.multiTables) { renderMultiTables(tab); return; }
    var pane = document.querySelector('.pcPane[data-p="' + tab.pane + '"]');
    if (!pane || $(tab.id)) return;
    var table = document.createElement('section'), original = Array.prototype.slice.call(pane.children), sources = [], sourceIds = {}, proxyTables = [table], information = null;
    table.id = tab.id;
    table.className = 'ht-proxy-root';
    table.innerHTML = '<div class="ht-setting-head"><h2>' + esc(tab.title) + '</h2></div><div class="ht-setting-scroll"><table class="ht-setting-table"><thead><tr><th>Setting</th><th>Option</th>' + (tab.thirdColumnTitle ? '<th>' + esc(tab.thirdColumnTitle) + '</th>' : '') + '</tr></thead><tbody></tbody></table></div>';
    function isExcluded(source) {
      return (tab.excludeContainers || []).some(function (id) { return source.closest('#' + id); }) || !!(tab.informationContainer && source.closest('#' + tab.informationContainer));
    }
    function addSource(source, kind, label, linkedSource) {
      if (!source || !source.id || sourceIds[source.id] || isExcluded(source)) return;
      sourceIds[source.id] = true;
      sources.push({ source: source, kind: kind, label: label || titleFromId(source.id), linkedSource: linkedSource });
    }
    pane.querySelectorAll('fieldset.rg').forEach(function (source) {
      var linkedRadioGroups = tab.linkedRadioGroups || {}, linkedId = linkedRadioGroups[source.id], isLinkedChild = Object.keys(linkedRadioGroups).some(function (id) { return linkedRadioGroups[id] === source.id; });
      if (!isLinkedChild) addSource(source, 'radio', legendOf(source), $(linkedId));
    });
    pane.querySelectorAll('select,input').forEach(function (control) {
      if (control.closest('fieldset.rg') || control.type === 'radio') return;
      if (control.type === 'checkbox') {
        var label = control.closest('label.ckb');
        if (label) addSource(label, 'checkbox', text(label.textContent));
      } else if (control.tagName === 'SELECT') addSource(control, 'select');
      else if (control.type === 'text') addSource(control, 'text');
    });
    (tab.extraSourceIds || []).forEach(function (id) {
      var source = $(id);
      if (!source) return;
      addSource(source, sourceKind(source), source.matches('fieldset.rg') ? legendOf(source) : titleFromId(source.id));
    });
    function makeRow(item, includeThirdColumn) {
      var row = document.createElement('tr'), name = document.createElement('th');
      row.dataset.settingName = item.label.toLowerCase();
      name.textContent = item.label;
      row.appendChild(name);
      row.appendChild(makeOption(item));
      if (includeThirdColumn) {
        var linkedCell = document.createElement('td'), linkedLabel = (tab.linkedOptionLabels || {})[item.source.id];
        if (item.linkedSource) {
          var linkedOption = makeOption({ source: item.linkedSource, kind: 'radio', label: legendOf(item.linkedSource) });
          if (linkedLabel) {
            var content = document.createElement('div'), label = document.createElement('label');
            content.className = 'ht-three-column-inline';
            label.textContent = linkedLabel + ':';
            content.appendChild(label);
            content.appendChild(linkedOption.firstChild);
            linkedCell.appendChild(content);
          } else linkedCell.appendChild(linkedOption.firstChild);
        }
        row.appendChild(linkedCell);
      }
      return row;
    }
    function makeMergedRow(items, titleText, itemLabels) {
      var row = document.createElement('tr'), title = document.createElement('th');
      title.textContent = titleText;
      row.dataset.settingName = titleText.toLowerCase();
      row.appendChild(title);
      items.forEach(function (item, index) {
        var cell = document.createElement('td'), content = document.createElement('div'), label = document.createElement('label'), option = makeOption(item);
        content.className = 'ht-three-column-inline';
        if (itemLabels[index]) {
          label.textContent = itemLabels[index] + ':';
          content.appendChild(label);
        }
        content.appendChild(option.firstChild);
        cell.appendChild(content);
        row.appendChild(cell);
      });
      return row;
    }
    var mergedRows = (tab.mergedRows || []).map(function (ids, rowIndex) {
      var items = ids.map(function (id) { return sources.find(function (item) { return item.source.id === id; }); });
      return items.every(Boolean) ? { items: items, label: tab.mergedRowLabels[rowIndex], itemLabels: tab.mergedRowItemLabels[rowIndex] } : null;
    }).filter(Boolean), mergedSourceIds = {};
    mergedRows.forEach(function (merged) { merged.items.forEach(function (item) { mergedSourceIds[item.source.id] = true; }); });
    sources.forEach(function (item) {
      var merged = mergedRows.find(function (candidate) { return candidate.items[0].source.id === item.source.id; });
      var row = merged ? makeMergedRow(merged.items, merged.label, merged.itemLabels) : !mergedSourceIds[item.source.id] && makeRow(item, !!tab.thirdColumnTitle);
      if (row) table.querySelector('tbody').appendChild(row);
    });
    pane.appendChild(table);
    if (tab.informationContainer && $(tab.informationContainer)) {
      information = document.createElement('section');
      information.id = tab.informationId;
      information.className = 'ht-proxy-root';
      information.innerHTML = '<div class="ht-setting-head"><h2>' + esc(legendOf($(tab.informationContainer))) + '</h2></div><div class="ht-information-body"><table class="ht-setting-table"><thead><tr><th>Setting</th><th>Option 1</th><th>Option 2</th></tr></thead><tbody></tbody></table></div>';
      ['edtCustomerCode', 'edtSeriaNo', 'cbHandlerModel'].forEach(function (id) {
        var source = $(id);
        if (!source) return;
        var row = makeRow({ source: source, kind: sourceKind(source), label: source.matches('fieldset.rg') ? legendOf(source) : titleFromId(id) }, true);
        if (id === 'edtCustomerCode') row.cells[2].innerHTML = '<span data-customer-name></span>';
        var subModel = $('rgModel');
        if (id === 'cbHandlerModel' && subModel) {
          var content = document.createElement('div'), label = document.createElement('label'), option = makeOption({ source: subModel, kind: 'radio', label: legendOf(subModel) });
          content.className = 'ht-three-column-inline';
          label.textContent = 'Sub-model:';
          content.appendChild(label);
          content.appendChild(option.firstChild);
          row.cells[2].appendChild(content);
        }
        information.querySelector('tbody').appendChild(row);
      });
      pane.insertBefore(information, table);
      var divider = document.createElement('div');
      divider.className = 'ht-handler-divider';
      pane.insertBefore(divider, table);
      proxyTables.push(information);
    }
    proxyTables.forEach(function (proxyTable) {
      proxyTable.querySelectorAll('input[type="text"][data-hst-src]').forEach(bindText);
      proxyTable.addEventListener('change', function (event) {
        if (event.target.hasAttribute('data-setting-source') && event.target.type !== 'text') settingWrite(event.target, proxyTable);
      });
    });
    original.forEach(hide);
    (tab.extraHiddenIds || []).forEach(function (id) { hide($(id)); });
    SYNCS.push(function () {
      settingSync(proxyTables, table, function () {
        var customerCode = $('edtCustomerCode'), customerName = information && information.querySelector('[data-customer-name]');
        if (customerCode && customerName) {
          var code = text(customerCode.value), entry = Array.prototype.slice.call(document.querySelectorAll('#rgCustomerList label.rgi')).find(function (e) { return text(e.textContent).indexOf(code.padStart(3, '0') + ' ') === 0; });
          var name = entry ? text(entry.textContent).substring(4) : '';
          if (customerName.textContent !== name) customerName.textContent = name;
        }
      });
    });
  }

  /* ================= Loader / Unloader track matrix (page_Old handler-track-table.js) ================= */
  var ENUM_MAP = {
    t3: {
      auto1: 'e3Auto1 = 0', auto2: 'e3Auto2 = 1', auto3: 'e3Auto3 = 2',
      auto4: 'e3Auto4 = 24', auto5: 'e3Auto5 = 25', auto6: 'e3Auto6 = 26',
      fix1: 'e3Fix1 = 3', fix2: 'e3Fix2 = 4', fix3: 'e3Fix3 = 5',
      fix4: 'e3Fix4 = 6', fix5: 'e3Fix5 = 7', fix6: 'e3Fix6 = 8'
    },
    t6: {
      auto1: 'eAuto1 = 0', auto2: 'eAuto2 = 1', auto3: 'eAuto3 = 2',
      auto4: 'eAuto4 = 3', auto5: 'eAuto5 = 4', auto6: 'eAuto6 = 5',
      fix1: 'eFix1 = 6', fix2: 'eFix2 = 7', fix3: 'eFix3 = 8',
      fix4: 'eFix4 = 9', fix5: 'eFix5 = 10', fix6: 'eFix6 = 11'
    }
  };
  // [name, Z motor, Y motor, cassette, separated-empty cylinder / magazine, fix kind]
  var TRACK_ROWS = [
    ['Loader', 'chkLoader', 'chkLoaderY', 'cbLoaderCassette', null, null],
    ['Empty', 'chkEmpty', 'chkEmptyY', 'cbEmptyCassette', null, null],
    ['Color', 'chkColor', 'chkColorY', 'cbColorCassette', null, null],
    ['Auto 1', 'chkAuto1', 'chkAuto1Y', 'cbAuto1Cassette', null],
    ['Auto 2', 'chkAuto2', 'chkAuto2Y', 'cbAuto2Cassette', 'Auto2SelectCy'],
    ['Auto 3', 'chkAuto3', 'chkAuto3Y', 'cbAuto3Cassette', 'rgAuto3Magazine'],
    ['Auto 4', 'chkAuto4', null, 'cbAuto4Cassette', null],
    ['Auto 5', 'chkAuto5', null, 'cbAuto5Cassette', null],
    ['Auto 6', 'chkAuto6', null, 'cbAuto6Cassette', null],
    ['Fix 1', null, null, null, null, 'none'],
    ['Fix 2', null, null, null, null, 'none'],
    ['Fix 3', null, null, null, null, 'fix3'],
    ['Fix 4', null, null, null, null, 'none'],
    ['Fix 5', null, null, null, null, 'none'],
    ['Fix 6', null, null, null, null, 'none']
  ];
  var NA = '<span class="ht-track-na">N/A</span>';
  function trackId(name) { return name.replace(' ', '').toLowerCase(); }
  function makeCheck(id) {
    if (!$(id)) return NA;
    hide($(id));
    return '<label class="ht-track-check"><input type="checkbox" data-track-source="' + id + '" data-hst-src="' + id + '"></label>';
  }
  function makeToggle(label, sourceId) {   // a 2-item radio group (Un Install / Install) as one tick
    if (!$(sourceId)) return '';
    hide($(sourceId));
    return '<label class="ht-track-option"><input type="checkbox" data-track-toggle="' + sourceId + '" data-hst-src="' + sourceId + '"><span>' + esc(label) + '</span></label>';
  }
  function makeAutoOptions(id, functionId) {
    var n = id.slice(4), out = '<details class="ht-track-options" data-track-auto-options="' + id + '"><summary><span data-track-auto-summary>Select options</span></summary><div class="ht-track-option-list">';
    out += makeToggle('ART', 'rgAuto' + n + 'ART');                                  // per-track ART first (Steven 1003 B69; see header)
    out += makeToggle('Auto Can Go Rear', 'rgAutoTrackCanGoRear');                   // one shared setting, shown on every Auto row
    if (id === 'auto2') out += makeToggle('Separated Empty cylinder', functionId);    // bUseAuto2Empty
    if (id === 'auto3') out += makeToggle('Magazine', functionId);
    return out + '</div></details>';
  }
  function makeSelect(id) {
    var el = $(id), out = '';
    if (!el) return NA;
    hide(el);
    radios(el).forEach(function (r, index) { out += '<option value="' + index + '">' + esc(text(r.parentNode.textContent)) + '</option>'; });
    return '<select class="ht-track-select" data-track-source="' + id + '" data-hst-src="' + id + '">' + out + '</select>';
  }
  function makeCombo(id) {
    var el = $(id), out = '';
    if (!el) return NA;
    hide(el);
    for (var index = 0; index < el.options.length; index++) out += '<option value="' + index + '">' + esc(text(el.options[index].text)) + '</option>';
    return '<select class="ht-track-select" data-track-combo-source="' + id + '" data-hst-src="' + id + '">' + out + '</select>';
  }
  function makeTrackFunction(id, functionId) {
    if (id === 'loader') return '<label class="ht-track-function-select"><span>Cover Tray ID</span>' + makeSelect('rgCoverTrayID') + '</label>';
    if (id === 'empty') return '<label class="ht-track-function-select"><span>Unloader Cover Tray ID</span>' + makeSelect('rgEmptyKeyence') + '</label>';
    return id.indexOf('auto') === 0 ? makeAutoOptions(id, functionId) : NA;
  }
  // Fix 3 = rgInstallFix3 + rgFix3FullPlace; "full tray" is [E55] cbE55 on the Configuration page (config.ini), not this form
  function makeFix3Select() {
    if (!$('rgInstallFix3') || !$('rgFix3FullPlace')) return NA;
    hide($('rgInstallFix3'));
    hide($('rgFix3FullPlace'));
    return '<select class="ht-track-select" data-track-fix3="1" data-hst-src="rgInstallFix3" data-hst-combined="1"><option value="0">Uninstall</option><option value="1">Install Fix3 for half tray</option><option value="2" disabled>Install Fix3 for full tray (Configuration [E55])</option><option value="3">Short Shuttle</option><option value="4">Use Cylinder</option><option value="5">Use Cylinder for 46LA</option><option value="6">Use Stepper Motor</option></select>';
  }
  function fix3Value() {
    var fullPlace = checkedIndex($('rgFix3FullPlace'));
    if (fullPlace > 0) return fullPlace + 2;
    return checkedIndex($('rgInstallFix3')) > 0 ? 1 : 0;
  }
  function writeFix3(control) {
    var value = control.selectedIndex;
    if (value === 2) return;
    pick($('rgInstallFix3'), value === 0 ? 0 : 1);
    pick($('rgFix3FullPlace'), value > 2 ? value - 2 : 0);
  }
  function machineTrackSummary(mode) {
    if (mode === 0) return '4 Track: E/C Manual; Auto 1-3';
    if (mode === 1) return '6 Track: E/C Automatic; Auto 1-3';
    if (mode === 2) return '7 Track: Empty Unloader; Auto 1-3';
    if (mode === 3) return '8 Track: 5 Unloader; Auto 1-5';
    return '9 Track: 6 Unloader; Auto 1-6';
  }
  function enumCell(kind, id) { return '<td class="ht-track-enum">' + (ENUM_MAP[kind][id] || '--') + '</td>'; }
  function renderTrack() {
    var pane = document.querySelector('.pcPane[data-p="1"]');
    if (!pane || $('htTrackMatrix')) return;
    var body = '';
    ['MachineTrack', 'rgInstallAutoRestest'].forEach(function (id) { hide($(id)); });
    TRACK_ROWS.forEach(function (item) {
      var id = trackId(item[0]);
      if (item[5]) {
        body += '<tr data-track-name="' + id + '" class="ht-track-fix-row"><th>' + item[0] + '</th><td colspan="3"><span class="ht-track-na">No track motor setting</span></td><td colspan="2">' + (item[5] === 'fix3' ? makeFix3Select() : '<span class="ht-track-na">N/A (no golden setting)</span>') + '</td>' + enumCell('t3', id) + enumCell('t6', id) + '</tr>';
      } else {
        body += '<tr data-track-name="' + id + '"><th>' + item[0] + '</th><td>' + makeCheck(item[1]) + '</td><td>' + (item[2] ? makeCheck(item[2]) : NA) + '</td><td>' + makeCheck(item[3]) + '</td><td colspan="2">' + makeTrackFunction(id, item[4]) + '</td>' + enumCell('t3', id) + enumCell('t6', id) + '</tr>';
      }
    });
    var table = document.createElement('section');
    table.id = 'htTrackMatrix';
    table.className = 'ht-proxy-root';
    table.innerHTML = '<div class="ht-track-global"><label>Machine Track ' + makeSelect('MachineTrack') + '</label><label>ART Function ' + makeSelect('rgInstallAutoRestest') + '</label></div><div class="ht-track-summary" id="htTrackSummary"></div><table class="ht-track-table"><colgroup><col><col><col><col><col><col><col><col></colgroup><thead><tr><th>Track</th><th>Z Motor</th><th>Y Motor</th><th>Cassette</th><th colspan="2">ART / Track Function</th><th>T3 enum</th><th>T6 enum</th></tr></thead><tbody>' + body + '</tbody></table><div class="ht-track-preview-note">Fix 1, 2, 4, 5, 6 install, Auto 4-6 Y Motor and Auto 4-6 cylinder / magazine have no golden setting (no Gerneral.ini key): N/A. Rows follow Machine Track; hidden rows keep their values.</div><section class="ht-track-other-wrap"><h2>Other Settings</h2><div class="ht-track-other-tools"><label>Search <input type="search" data-track-other-search></label><label>Sort <select data-track-other-sort><option value="asc">Setting: A-Z</option><option value="desc">Setting: Z-A</option><option value="dfm">DFM order</option></select></label></div><table class="ht-track-other-table"><thead><tr><th>Setting</th><th>Option</th></tr></thead><tbody id="htTrackOther"></tbody></table></section>';
    var other = table.querySelector('#htTrackOther');
    function used(id) { return !!table.querySelector('[data-hst-src="' + id + '"]'); }
    function addOther(title, html) {
      var row = document.createElement('tr'), th = document.createElement('th'), td = document.createElement('td');
      th.textContent = title;
      row.dataset.settingTitle = title.toLowerCase();
      td.innerHTML = html;
      row.appendChild(th);
      row.appendChild(td);
      other.appendChild(row);
    }
    var groups = Array.prototype.slice.call(pane.querySelectorAll('fieldset.rg'));
    ['rgMagBinDispType', 'rgE84Sensor', 'rgAOI', 'rgTopScanner_AOI', 'rgScanner_AOI', 'rgVibrationCommuncation'].forEach(function (id) { if ($(id)) groups.push($(id)); });
    groups.sort(function (left, right) { return (parseInt(left.style.top, 10) || 0) - (parseInt(right.style.top, 10) || 0); });
    groups.forEach(function (group) { if (group.id && !used(group.id)) addOther(legendOf(group), makeSelect(group.id)); });   // B69: the per-track ART are used by the matrix
    if ($('cbVibrationCardQty')) addOther('Vibration Card Qty', makeCombo('cbVibrationCardQty'));
    pane.appendChild(table);
    pane.classList.add('ht-track-matrix-active');
    function filterAndSortOtherSettings() {
      var search = table.querySelector('[data-track-other-search]'), sort = table.querySelector('[data-track-other-sort]');
      var rows = Array.prototype.slice.call(other.rows), query = search.value.toLowerCase();
      if (sort.value !== 'dfm') rows.sort(function (left, right) {
        var order = left.dataset.settingTitle.localeCompare(right.dataset.settingTitle);
        return sort.value === 'asc' ? order : -order;
      });
      rows.forEach(function (row) {
        row.style.display = !query || row.dataset.settingTitle.indexOf(query) >= 0 ? '' : 'none';
        other.appendChild(row);
      });
    }
    function showRows() {                 // page_Old applyMachineTrack, display only (golden has no MachineTrackClick)
      var mode = checkedIndex($('MachineTrack'));
      function show(name, on) { var r = table.querySelector('[data-track-name="' + name + '"]'); if (r) r.style.display = on ? '' : 'none'; }
      show('empty', mode !== 0);
      show('color', mode !== 0);
      show('auto4', mode >= 3);
      show('auto5', mode >= 3);
      show('auto6', mode >= 4);
      ['fix4', 'fix5', 'fix6'].forEach(function (name) { show(name, mode >= 3); });
      var s = table.querySelector('#htTrackSummary'), t = mode < 0 ? '' : machineTrackSummary(mode);
      if (s.textContent !== t) s.textContent = t;
    }
    function trackSync() {
      table.querySelectorAll('[data-track-source],[data-track-combo-source]').forEach(function (control) { readInto(control, $(control.getAttribute('data-hst-src'))); });
      table.querySelectorAll('[data-track-toggle]').forEach(function (control) { control.checked = checkedIndex($(control.getAttribute('data-track-toggle'))) > 0; });
      var fix3 = table.querySelector('[data-track-fix3]');
      if (fix3) fix3.selectedIndex = fix3Value();
      table.querySelectorAll('[data-track-auto-options]').forEach(function (options) {
        var selected = Array.prototype.slice.call(options.querySelectorAll('input:checked')).map(function (control) { return control.parentNode.querySelector('span').textContent; });
        var t = selected.length ? selected.join(', ') : 'Select options', s = options.querySelector('[data-track-auto-summary]');
        if (s.textContent !== t) s.textContent = t;
      });
      showRows();
    }
    table.addEventListener('change', function (event) {
      var t = event.target;
      if (t.hasAttribute('data-track-source') || t.hasAttribute('data-track-combo-source')) writeBack(t, $(t.getAttribute('data-hst-src')));
      else if (t.hasAttribute('data-track-toggle')) pick($(t.getAttribute('data-track-toggle')), t.checked ? 1 : 0);
      else if (t.hasAttribute('data-track-fix3')) writeFix3(t);
      else if (t.hasAttribute('data-track-other-sort')) filterAndSortOtherSettings();
      soon();
    });
    var search = table.querySelector('[data-track-other-search]');
    search.addEventListener('input', filterAndSortOtherSettings);
    bindSearch(search);
    filterAndSortOtherSettings();
    SYNCS.push(trackSync);
  }

  /* ================= Com Port table (page_Old handler-comport-table.js) ================= */
  var PORTS = [
    ['Index Torque', 'cbComIndex', 'chkUseHPComCard'], ['Temperature Controller', 'cbComTemp'], ['Temperature Controller (Omron)', 'cbComTempOmron'],
    ['Dynamic Temperature IC', 'cbComDyTemp'], ['Tester', 'cbComTester'], ['Bin Display', 'cbComBinDisp'],
    ['Real Time CCD', 'cbComRTC'], ['ATC 1', 'cbbATC1'], ['ATC 2', 'cbbATC2'], ['ATC 3', 'cbbATC3'], ['ATC 4', 'cbbATC4'],
    ['OCR', 'cbOCR'], ['OCR with Tester', 'cbOCRwithTester'], ['Air Conditioner', 'cbAirCon'], ['KASUGA Fan', 'cbKASUGA_Fan'],
    ['Tray / Pad Step Motor', 'cbbTrayStepMotor'], ['Bin Display 2', 'cbbComBinDisp2'], ['EM Aware Port 1', 'cbEMAwarePort1', 'cbESDUse4COM'],
    ['EM Aware Port 2', 'cbEMAwarePort2'], ['EM Aware Port 3', 'cbEMAwarePort3'], ['EM Aware Port 4', 'cbEMAwarePort4'],
    ['Simco ION Novx 3360', 'cbNovx3360'], ['Laser 1', 'cbComLaser1'], ['Laser 2', 'cbComLaser2'],
    ['Laser In Arm', 'cbComLaserInArm'], ['2D Reader 1', 'cb2DReader1'], ['2D Reader 2', 'cb2DReader2'],
    ['2D Reader 3', 'cb2DReader3'], ['2D Reader 4', 'cb2DReader4'], ['Ground Man', 'cbbGroundMan'],
    ['TTL1 RS232', 'cbComTTLRS232'], ['TTL2 RS232', 'cbComTTLRS232_2'], ['RFID Reader', 'cbbRFIDReader']
  ];
  function portNo(value) { var result = /\d+/.exec(value || ''); return result ? parseInt(result[0], 10) : 999; }
  function renderComPort() {
    var pane = document.querySelector('.pcPane[data-p="6"]');
    if (!pane || $('htComPortTable')) return;
    var table = document.createElement('section'), original = Array.prototype.slice.call(pane.children), descending = false, sortBy = 'name';
    table.id = 'htComPortTable';
    table.className = 'ht-proxy-root';
    table.innerHTML = '<div class="ht-comport-head"><label>Search <input type="search" data-com-search></label><label>Sort <select data-com-sort><option value="name">Device name</option><option value="port">COM Port</option></select></label><button type="button" data-com-set-atc-default>Set Default</button><button type="button" class="ht-comport-sort">Device name &#9650;</button></div><div class="ht-comport-scroll"><table class="ht-track-table ht-comport-table"><thead><tr><th>Device</th><th>Component</th><th>COM Port</th><th>Setting</th></tr></thead><tbody></tbody></table></div>';
    var tbody = table.querySelector('tbody');
    PORTS.forEach(function (item) {
      var source = $(item[1]);
      if (!source) return;
      var row = document.createElement('tr');
      row.dataset.port = item[1];
      row.dataset.device = item[0].toLowerCase();
      row.innerHTML = '<th>' + esc(item[0]) + '</th><td class="ht-track-enum">' + item[1] + '</td><td><select class="ht-track-select" data-com-source="' + item[1] + '" data-hst-src="' + item[1] + '"></select></td><td></td>';
      copyOptions(row.querySelector('select'), source);
      var checkSource = item[2] && $(item[2]);
      if (checkSource && boxOf(checkSource)) row.cells[3].innerHTML = '<label class="ht-track-check ht-track-check-caption"><input type="checkbox" data-com-source="' + item[2] + '" data-hst-src="' + item[2] + '"> ' + esc(text(checkSource.textContent)) + '</label>';
      tbody.appendChild(row);
    });
    function sort() {
      var rows = Array.prototype.slice.call(tbody.querySelectorAll('tr'));
      rows.sort(function (left, right) {
        var order = sortBy === 'name' ? left.dataset.device.localeCompare(right.dataset.device) : portNo(left.querySelector('select').value) - portNo(right.querySelector('select').value);
        if (!order) order = sortBy === 'name' ? portNo(left.querySelector('select').value) - portNo(right.querySelector('select').value) : left.dataset.device.localeCompare(right.dataset.device);
        return order * (descending ? -1 : 1);
      });
      rows.forEach(function (row) { tbody.appendChild(row); });
      table.querySelector('.ht-comport-sort').innerHTML = (sortBy === 'name' ? 'Device name' : 'COM Port') + ' ' + (descending ? '&#9660;' : '&#9650;');
    }
    function comSync() { table.querySelectorAll('[data-com-source]').forEach(function (proxy) { readInto(proxy, $(proxy.getAttribute('data-com-source'))); }); }
    table.addEventListener('change', function (event) {
      var id = event.target.getAttribute('data-com-source');
      if (!id) return;
      writeBack(event.target, $(id));
      sort();
    });
    table.querySelector('.ht-comport-sort').addEventListener('click', function () { descending = !descending; sort(); });
    table.querySelector('[data-com-sort]').addEventListener('change', function (event) { sortBy = event.target.value; sort(); });
    table.querySelector('[data-com-set-atc-default]').addEventListener('click', function () {   // golden btnSetATCComClick (HandlerSys.cpp:1081-1098)
      var btn = $('btnSetATCCom');
      if (btn && !btn.disabled) btn.click();
      comSync();
      sort();
    });
    var search = table.querySelector('[data-com-search]');
    search.addEventListener('input', function () {
      var query = search.value.toLowerCase();
      tbody.querySelectorAll('tr').forEach(function (row) { row.style.display = !query || row.dataset.device.indexOf(query) >= 0 ? '' : 'none'; });
    });
    bindSearch(search);
    pane.appendChild(table);
    original.forEach(hide);
    comSync();
    sort();
    SYNCS.push(comSync);
  }

  /* ================= Heater section in the Temperature tab (todo E-029; see the header) ================= */
  var HEATER_ROWS = ['rgHeaterType', 'rgHeater'];          // excluded from Temperature Settings (TABS pane 5)
  function heaterTab() { return document.querySelector('#pcSetting > .pcTabs > .tab[data-t="10"]'); }
  function renderHeaterSection() {
    var root = $('htTemperatureTables'), tri = $('htTriTemperatureSettings'), grp = $('grpHeater'), tab = heaterTab();
    if (!root || !tri || !grp || $('htHeaterSection')) return;
    var section = document.createElement('section');
    section.id = 'htHeaterSection';
    section.className = 'ht-inout-table';
    section.innerHTML = '<div class="ht-setting-head"><h2>' + esc(text(tab ? tab.textContent : '') || 'Heater') + '</h2></div><div class="ht-setting-scroll"><table class="ht-setting-table ht-three-column-table"><thead><tr><th>Setting</th><th>Option 1</th><th>Option 2</th></tr></thead><tbody></tbody></table><div class="ht-heater-body"></div></div>';
    var tbody = section.querySelector('tbody');
    HEATER_ROWS.forEach(function (id) {
      var source = $(id);
      if (!source) return;
      var row = document.createElement('tr'), name = document.createElement('th');
      name.textContent = legendOf(source) || titleFromId(id);
      row.dataset.settingName = name.textContent.toLowerCase();
      row.appendChild(name);
      row.appendChild(makeOption({ source: source, kind: sourceKind(source), label: name.textContent }));
      row.appendChild(document.createElement('td'));
      tbody.appendChild(row);
    });
    root.insertBefore(section, tri);                          // write-back: the Temperature tables' own change listener (settingWrite)
    section.querySelector('.ht-heater-body').appendChild(grp);
    if (tab) tab.classList.add('ht-tab-moved');
    SYNCS.push(function () {
      settingSync([section], section);
      section.classList.toggle('ht-section-gone', !!((tab && tab.style.display === 'none') || isGone(grp)));   // C++ TabVisible / golden Visible
    });
  }
  function heaterSectionBack(e) {                            // build failed: the Heater tab as before
    if (window.console) console.error('[HSys table] Heater section kept on its own tab:', e);
    var grp = $('grpHeater'), pane = document.querySelector('.pcPane[data-p="10"]'), s = $('htHeaterSection'), tab = heaterTab();
    if (grp && pane && grp.parentNode !== pane) pane.appendChild(grp);
    if (s && s.parentNode) s.parentNode.removeChild(s);
    if (tab) tab.classList.remove('ht-tab-moved');
  }

  /* ================= tab order (page_Old inline script :171-182) ================= */
  function reorderTabs() {
    var pc = $('pcSetting');
    if (!pc) return;
    var tabs = pc.querySelector(':scope > .pcTabs'), body = pc.querySelector(':scope > .pcBody');
    if (!tabs || !body) return;
    function tab(n) { return tabs.querySelector(':scope > .tab[data-t="' + n + '"]'); }
    if (tab(3) && tab(4)) tabs.insertBefore(tab(4), tab(3));              // Shuttle before Index Items
    if (tab(6) && tab(8)) tabs.insertBefore(tab(8), tab(6));              // ION Fan before Com Port
    if (tab(5)) tab(5).textContent = 'Temperature';                        // tsOtherItems shows the temperature settings
    if (tab(7)) tabs.appendChild(tab(7));                                  // Customer Code last
    var customerPane = body.querySelector(':scope > .pcPane[data-p="7"]');
    if (customerPane) body.appendChild(customerPane);
    if (typeof window.layoutPC === 'function') window.layoutPC(pc);
  }

  /* ================= start ================= */
  // one tab failing to build falls back to that tab's original controls, the other tabs still get their tables
  function guarded(paneNo, ids, fn) {
    var before = SYNCS.length;
    try { fn(); } catch (e) {
      if (window.console) console.error('[HSys table] pane ' + paneNo + ' kept its original layout:', e);
      SYNCS.length = before;
      ids.forEach(function (id) { var x = $(id); if (x && x.parentNode) x.parentNode.removeChild(x); });
      var pane = document.querySelector('.pcPane[data-p="' + paneNo + '"]');
      if (pane) {
        pane.classList.remove('ht-track-matrix-active');
        Array.prototype.forEach.call(pane.querySelectorAll('.ht-src-hidden'), function (x) { x.classList.remove('ht-src-hidden'); });
        Array.prototype.forEach.call(pane.querySelectorAll(':scope > .ht-handler-divider'), function (x) { x.parentNode.removeChild(x); });
      }
    }
  }
  function start() {
    try { reorderTabs(); } catch (e) { if (window.console) console.error('[HSys table] tab order', e); }
    TABS.forEach(function (tab) { guarded(tab.pane, [tab.id, tab.informationId], function () { renderSettings(tab); }); });
    guarded('1', ['htTrackMatrix'], renderTrack);
    guarded('6', ['htComPortTable'], renderComPort);
    var before = SYNCS.length;
    try { renderHeaterSection(); } catch (e) { SYNCS.length = before; heaterSectionBack(e); }
    ['input', 'change', 'click'].forEach(function (type) {
      document.addEventListener(type, function (ev) {
        var t = ev.target;
        if (t && t.closest && !t.closest('.ht-proxy-root')) soon();
      }, true);
    });
    syncAll();
    window.setInterval(syncAll, 500);
  }
  window.HT9045HSysTable = { sync: syncAll };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
