/* ht9045_speed_c.js -- Setup.Speed.html（golden TfSpeed，cSpeed.cpp）Q41 的頁面事件補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.5 的 SP-1～SP-5（ST01-E 20260927 16:50 准：只改頁面 JS）。
 * St02 的新檔（手寫，不是 gen_wire.py 產物）；頁面 Setup.Speed.html:125 同一行載入，在兩份接線資料檔之後。
 * 引擎 ht9045_wire_engine.js、FileRW/ArmSpeed_File*、tools/editlist/ArmSpeed_File.py 都不改（存檔端 ST01-E 在改 saveFlow）。
 * golden 行號：906_0625_Steven 與 912 的 cSpeed.cpp 這幾支行號相同。
 *
 *  SP-1  tbAllSpeedChange（:1282-1343）／tbAccSpeedChange（:1345-1406）：拖「全部速度／加速度」滑桿（VCL OnChange 拖動中一直觸發 →
 *        這裡聽 input），edAllSpeed／edAllAccSpeed＝Position，勾選的各軸速度（加速度）欄一起改。udXxx（TUpDown）在網頁上沒有，
 *        它們的 Associate（DFM 912 :862 等）就是同一格 edit，所以只改 edit。⚠ Index 的加速度：golden :1351 把
 *        edIndexAccDec->Text 註解掉（Steven 20091214「Index ACC & DEC must be 100%」），但 :1352 udIndexAcc->Position 還在，
 *        udIndexAcc 的 Associate＝edIndexAccDec（DFM :2300）；VCL TUpDown 帶 UDS_SETBUDDYINT，設 Position 會把數字寫回 buddy
 *        edit → golden 實際上 edIndexAccDec 也會跟著變。這裡照 VCL 的實際行為做（寫在 Q41 回報，要照註解的意圖改請 Steven 決定）。
 *  SP-2  cbIndexArmClick（:1426-1431；DFM :533 起 9 顆共用）：勾或取消任何一軸都把 tbAllSpeed、spbSpeedAdd、spbSpeedDec 打開
 *        （建構子 :35 起先停用，C 路開頁照搬 → 引擎停用）。spbSelectAllClick（:1795-1810）：9 顆全勾＋打開三個元件。
 *        上層容器被引擎停用（權限，aria-disabled）時不打開（VCL 父層停用一樣按不到）。
 *  SP-3  spbSpeedAddClick（:1414-1419）：Position+=10、/=10、*=10（每一步 VCL 夾在 Min..Max，值有變就觸發 OnChange）；
 *        spbSpeedDecClick（:1421-1424）：Position-=10。
 *  SP-4  spbSetToDefClick（:1812-1933）：照 golden 順序：兩條滑桿 100、edAllSpeed／edAllAccSpeed＝100、全選、＋10、tbAccSpeedChange，
 *        再逐欄填出廠值（AnsiString 的數字字串：100、0.1、-0.01、0…）。
 *  SP-5  tbEPControlChange（:1408-1412）：edEPControl＝Position。
 * 值改了之後補發 input／change（同引擎小鍵盤提交）。存檔：edAllSpeed／edAllAccSpeed／edEPControl 是唯讀、由伺服器存（golden :1453-1455），
 * 頁面只負責畫面；那一半在 ST01-E 的 saveFlow（St02-M 17:21 已轉）。
 *
 * AI(W906-Q41-SPD) 20260928 (St02-E helper): server-side golden handlers over WS form.event (St01 70aa17e8: FileRW/ArmSpeed_File.cpp
 *   g_evreg + ArmSpeed_File.gen.inc kSP_Events; form.event format FileRW/_FormEvent.h).
 *  Gate (same idea as ht9045_config_q41.js): the server has to advertise the events. IniConfig is the only struct that sends
 *    "eventTag" (FileRW/IniConfig.cpp:458); C-route pages advertise them in editlist.get "events" (FileRW/_EditPage.cpp:79, the key is
 *    absent when the page has no event table). Event mode is on only when "events" lists all 13 controls below (EV_CTL); the tag is
 *    eventTag when the server sends one, otherwise "Setup.Speed" (FindPageForEvent accepts the page name, _EditPage.cpp FindPageForEvent).
 *  Event mode (all-event, never mixed with the local SP-2..SP-4): the nine axis boxes send {"event":"click","checked":<value after the
 *    click>}; spbSelectAll / spbSetToDef / spbSpeedAdd / spbSpeedDec send {"event":"click"}. Every event carries "state" = the values the
 *    engine would save (taken when the event goes out, after the previous ack was applied) plus the three slider positions and the nine
 *    boxes: golden spbSpeedAddClick / spbSpeedDecClick read the server's tbAllSpeed Position and the boxes. ack.changed is applied to the
 *    page (values, visible, enabled / editable). One event at a time; busy -> retry after 450 ms (max 3); not-operator -> keepAlive and
 *    retry once; "reload page" -> reopen the page (editlist.get). The +-10 buttons stay disabled until the server enables them (golden
 *    cbIndexArmClick / spbSelectAllClick), so the server never sees +-10 before a box / select-all event.
 *  No events advertised (older server): nothing is sent; SP-2..SP-4 run locally exactly as before.
 *  Sliders: SP-1 / SP-5 run locally while dragging (display). AI(W906-Q41-SPD-X2) 20260928 (St02-E): when editlist.get "events" also lists
 *    tbAllSpeed / tbAccSpeed / tbEPControl with event "change" (St01 review6 7e1785dc X-2, FileRW/_FormEvent.h "position"), releasing a
 *    slider sends {"control":<tb>,"event":"change","position":<n>} through the same queue (one queued item per slider, the position is
 *    read when it goes out) and the server runs golden tb*Change; review6 29a13bdb ignores "state" for a control that has its own event
 *    row, so without this ±10 / SetToDef would read a stale server Position. Servers without those rows: as before (position in "state").
 *  At save the engine sends the positions,
 *    the boxes and the axis edits as shown; the server replays golden tb*Change for the read-only edAllSpeed / edAllAccSpeed / edEPControl
 *    (ArmSpeed_File.cpp BeforeApply) and keeps the axis edits as the page shows them.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var AXES = ['cbIndexArm', 'cbInArm', 'cbOutArm', 'cbShuttle', 'cbTrayArm', 'cbInArmZ', 'cbOutArmZ', 'cbInRotate', 'cbOutRotate'];   // DFM :533-655
  var SPD = {                                                            // golden :1286-1342（勾選框 → 速度欄）
    cbIndexArm: ['edIndexSpeed'], cbInArm: ['edInXYSpd', 'edInPitchSpd'], cbOutArm: ['edOutXSpd', 'edOutPitchSpd'],
    cbShuttle: ['edSht1Spd', 'edSht2Spd'], cbTrayArm: ['edTrayXSpd'], cbInArmZ: ['edInZSpd'], cbOutArmZ: ['edOutZSpd'],
    cbInRotate: ['edInRotSpd'], cbOutRotate: ['edOutRotSpd']
  };
  var ACC = {                                                            // golden :1349-1405（cbIndexArm → udIndexAcc 的 buddy edIndexAccDec）
    cbIndexArm: ['edIndexAccDec'], cbInArm: ['edInXYAcc', 'edInPitchAcc'], cbOutArm: ['edOutXAcc', 'edOutPitchAcc'],
    cbShuttle: ['edSht1Acc', 'edSht2Acc'], cbTrayArm: ['edTrayXAcc'], cbInArmZ: ['edInZAcc'], cbOutArmZ: ['edOutZAcc'],
    cbInRotate: ['edInRotAcc'], cbOutRotate: ['edOutRotAcc']
  };
  // golden spbSetToDefClick :1822-1932 的逐欄出廠值（[id, 值]；值是字串＝TEdit->Text，數字＝ItemIndex，布林＝Checked）
  var DEFS = [
    ['edIndexSpeed', '100'], ['edIndexAccDec', '100'], ['edIndexRetryCount', '0'], ['edIndexArmRetryMM', '0'], ['chkIndexPreSuck', true],
    ['edIndexVacumCheckTime', '0.1'], ['edIndexAirOnTime', '0.1'], ['rgSocketCheck', 1],
    ['edInXYSpd', '100'], ['edInXYAcc', '100'], ['edInZSpd', '100'], ['edInZAcc', '100'], ['edInPitchSpd', '100'], ['edInPitchAcc', '100'],
    ['edInArmRetryCount', '2'], ['edInArmRetryMM', '-0.01'], ['chkInArmPreSuck', true], ['edInVacumCheckTime', '0.1'], ['edtHPVacuumDelay', '0.1'],
    ['edInArmAirOnTime', '0.1'], ['edInArmShtWaitTime', '0.01'], ['rgInArmAutoSpeed', 1], ['rgInArmPitch', 0],
    ['edOutXSpd', '100'], ['edOutXAcc', '100'], ['edOutZSpd', '100'], ['edOutZAcc', '100'], ['edOutPitchSpd', '100'], ['edOutPitchAcc', '100'],
    ['edOutArmRetryCount', '2'], ['edOutArmRetryMM', '-0.01'], ['chkOutArmPreSuck', true], ['edtOutArmPreSuck', '0.01'],
    ['edOutVacumCheckTime', '0.1'], ['edOutArmAirOnTime', '0.1'], ['rgOutArmPitch', 0],
    ['edSht1Spd', '100'], ['edSht1Acc', '100'], ['edSht2Spd', '100'], ['edSht2Acc', '100'], ['edShtDeviceCheckTime', '0.01'], ['rgStepShuttle', 1],
    ['edTrayXSpd', '100'], ['edTrayXAcc', '100'], ['edTrayVacumCheckTime', '0.1'], ['edTrayArmAirOnTime', '0.1'], ['edTrayArmHandDown', '0.5'],
    ['edTrayArmRetryCount', '2'], ['edShakeCycles', '1'], ['edShakeDistance', '5'], ['edShakeDelay', '0'], ['edShakeAccDec', '100'],
    ['edMagCatchYSpd', '50'], ['edMagCatchYAcc', '50'], ['edMagZSpd', '50'], ['edMagZAcc', '50']
  ];

  function $(id) { return document.getElementById(id); }
  function fire(el, names) {
    (names || ['input', 'change']).forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function setText(id, v) { var el = $(id); if (el && el.value !== String(v)) { el.value = String(v); fire(el); } }
  function cbOf(id) { var el = $(id); return el ? (el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]')) : null; }
  function isChecked(id) { var c = cbOf(id); return !!(c && c.checked); }
  function setChecked(id, v) { var c = cbOf(id); if (c && c.checked !== !!v) { c.checked = !!v; fire(c, ['change']); } }
  function setIndex(id, i) {                                             // TRadioGroup->ItemIndex
    var el = $(id), rs = el ? el.querySelectorAll('input[type="radio"]') : [];
    if (rs[i] && !rs[i].checked) { rs[i].checked = true; fire(rs[i], ['change']); }
  }
  function pos(id) { var el = $(id), n = el ? parseInt(el.value, 10) : NaN; return isNaN(n) ? 0 : n; }
  function setPos(id, v) {                                               // VCL TTrackBar->Position：夾在 Min..Max，值有變才觸發 OnChange
    var el = $(id);
    if (!el) return;
    var lo = parseInt(el.min, 10), hi = parseInt(el.max, 10);
    if (!isNaN(lo) && v < lo) v = lo;
    if (!isNaN(hi) && v > hi) v = hi;
    if (pos(id) === v) return;
    el.value = String(v);
    fire(el, ['input']);                                                 // → 下面的 OnChange 處理器
  }
  function enableCtl(id) {                                               // golden xxx->Enabled=true
    var el = $(id);
    if (!el) return;
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return;
    el.disabled = false;
    el.removeAttribute('data-gb-dis');
    el.removeAttribute('aria-disabled');
    el.style.opacity = '';                                               // dfm2web 把 DFM Enabled=False 畫成 opacity:.45
  }

  /* ---- SP-1／SP-5 ------------------------------------------------------- */
  function tbAllSpeedChange() {                                          // golden :1282-1343
    var p = pos('tbAllSpeed');
    setText('edAllSpeed', p);
    AXES.forEach(function (cb) { if (isChecked(cb)) SPD[cb].forEach(function (id) { setText(id, p); }); });
  }
  function tbAccSpeedChange() {                                          // golden :1345-1406
    var p = pos('tbAccSpeed');
    setText('edAllAccSpeed', p);
    AXES.forEach(function (cb) { if (isChecked(cb)) ACC[cb].forEach(function (id) { setText(id, p); }); });
  }
  function tbEPControlChange() { setText('edEPControl', pos('tbEPControl')); }   // golden :1408-1412

  /* ---- SP-2 ------------------------------------------------------------- */
  function cbIndexArmClick() { enableCtl('tbAllSpeed'); enableCtl('spbSpeedAdd'); enableCtl('spbSpeedDec'); }   // golden :1426-1431
  function spbSelectAllClick() {                                         // golden :1795-1810
    ['cbIndexArm', 'cbInArm', 'cbOutArm', 'cbShuttle', 'cbTrayArm', 'cbInArmZ', 'cbOutArmZ'].forEach(function (id) { setChecked(id, true); });
    cbIndexArmClick();
    ['cbInRotate', 'cbOutRotate'].forEach(function (id) { setChecked(id, true); });
  }

  /* ---- SP-3 ------------------------------------------------------------- */
  function spbSpeedAddClick() {                                          // golden :1414-1419
    setPos('tbAllSpeed', pos('tbAllSpeed') + 10);
    setPos('tbAllSpeed', Math.floor(pos('tbAllSpeed') / 10));
    setPos('tbAllSpeed', pos('tbAllSpeed') * 10);
  }
  function spbSpeedDecClick() { setPos('tbAllSpeed', pos('tbAllSpeed') - 10); }   // golden :1421-1424

  /* ---- SP-4 ------------------------------------------------------------- */
  function spbSetToDefClick() {                                          // golden :1812-1933
    setPos('tbAllSpeed', 100);
    setPos('tbAccSpeed', 100);
    setText('edAllSpeed', 100);
    setText('edAllAccSpeed', 100);
    spbSelectAllClick();
    spbSpeedAddClick();
    tbAccSpeedChange();
    DEFS.forEach(function (d) {
      if (typeof d[1] === 'boolean') setChecked(d[0], d[1]);
      else if (typeof d[1] === 'number') setIndex(d[0], d[1]);
      else setText(d[0], d[1]);
    });
  }

  /* ---- AI(W906-Q41-SPD) 20260928 (St02-E helper): WS form.event (see the header) --------------------------------------- */
  var R = window.HT9045Recipe;
  var STRUCT = 'ArmSpeed_File', FORM = 'TfSpeed', PAGE_TAG = 'Setup.Speed';
  var EV_CTL = AXES.concat(['spbSelectAll', 'spbSetToDef', 'spbSpeedAdd', 'spbSpeedDec']);   // St01 70aa17e8 kSP_Events (13)
  var TRACKS = ['tbAllSpeed', 'tbAccSpeed', 'tbEPControl'];            // golden cSpeed.dfm :53 / :108 / :692
  var LAST = null;                                                       // last editlist.get ArmSpeed_File reply
  var QUEUE = [], sending = false, APPLYING = false;                     // APPLYING: change events fired while applying ack.changed are not user clicks
  function evTag() {                                                     // null = no server events (older server) -> local SP-2..SP-4
    var ev = LAST && LAST.events;
    if (!ev || typeof ev !== 'object') return null;
    for (var i = 0; i < EV_CTL.length; i++) if (!ev[EV_CTL[i]]) return null;
    return (typeof LAST.eventTag === 'string' && LAST.eventTag) ? LAST.eventTag : PAGE_TAG;
  }
  function trackEv(id) {                                                 // AI(W906-Q41-SPD-X2) 20260928 (St02-E): the slider has its own "change" row
    var ev = LAST && LAST.events, r = ev && ev[id];
    return !!(r && (r.event === 'change' || (Array.isArray(r) && r.indexOf('change') >= 0)));
  }
  function ancestorDisabled(el) {
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function setVis(el, on) {                                              // VCL Visible (the engine uses visibility; dfm2web DFM Visible=False is display:none)
    el.style.visibility = on ? '' : 'hidden';
    if (on && el.style.display === 'none') el.style.display = '';
  }
  function setEditable(el, on) {                                         // same rule as the engine gbSetEnabled: only undo what it disabled itself
    if (on && ancestorDisabled(el)) return;
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if (!('disabled' in x)) return;
      if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
      else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
    });
    if (!on) el.setAttribute('aria-disabled', 'true');
    else { el.removeAttribute('aria-disabled'); el.style.opacity = ''; }   // same as enableCtl: dfm2web draws DFM Enabled=False as opacity:.45
  }
  function isTextBox(el) { return (el.tagName === 'INPUT' && el.type !== 'checkbox' && el.type !== 'radio' && el.type !== 'range') || el.tagName === 'TEXTAREA'; }
  function setIndexEl(el, i) {                                           // TComboBox / TRadioGroup ItemIndex from ack.changed
    if (el.tagName === 'SELECT') { if (el.selectedIndex !== i && i < el.options.length) { el.selectedIndex = i; fire(el, ['change']); } return; }
    var rs = el.querySelectorAll('input[type="radio"]');
    for (var k = 0; k < rs.length; k++) {
      var want = (k === i);
      if (rs[k].checked !== want) { rs[k].checked = want; if (want) fire(rs[k], ['change']); }
    }
  }
  function applyChanged(ch) {                                            // ack.changed: {name:{text?,itemIndex?,checked?,position?,visible?,enabled?,editable?}}, changed keys only
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id] || {}, el = $(id);
        if (!el) return;                                                 // e.g. udXxx (TUpDown): no page element, the buddy edit carries the value
        if (v.checked !== undefined) {
          var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]');
          if (c && c.checked !== !!v.checked) { c.checked = !!v.checked; fire(c, ['change']); }
        }
        if (v.itemIndex !== undefined) setIndexEl(el, v.itemIndex);
        if (v.position !== undefined && el.type === 'range') el.value = String(v.position);   // no input event: the server already ran golden tb*Change and sent its edits
        else if (v.text !== undefined && isTextBox(el) && el.value !== String(v.text)) { el.value = String(v.text); fire(el); }
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
      });
      dis.forEach(function (el) { setEditable(el, false); });           // enable first, then disable (engine gbLoad order): a disabled container wins
    } finally { APPLYING = false; }
  }
  function valueOf(id, k) {                                              // same shapes as the engine gbValue (checked / itemIndex / text / position)
    var el = $(id);
    if (!el) return null;
    if (k === 'checked') { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); return c ? { checked: c.checked } : null; }
    if (k === 'itemIndex') {
      if (el.tagName === 'SELECT') {
        var o = el.options[el.selectedIndex];
        if (o && o.getAttribute('data-src') === 'cpp-text') return { itemIndex: -1, text: o.textContent };
        return { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
      }
      var rs = el.querySelectorAll('input[type="radio"]');
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
      return { itemIndex: -1 };
    }
    if (k === 'text') return { text: String(el.value) };
    if (k === 'position') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }
    return null;                                                         // cells / dateTime / tag: not sent in state, the server keeps its values
  }
  function stateNow(skip) {                                              // the page's current values (skip = the clicked control; the server drops it anyway)
    var g = (window.HT9045Page && HT9045Page.golden) ? HT9045Page.golden() : null, kinds = (g && g.kinds) || {}, out = {};
    Object.keys(kinds).forEach(function (id) {
      if (id === skip) return;
      var v = valueOf(id, kinds[id]);
      if (v) out[id] = v;
    });
    TRACKS.forEach(function (id) {                                       // servers without slider rows: +-10 reads tbAllSpeed from here; with them the server ignores it (29a13bdb)
      var el = $(id), n = el ? parseInt(el.value, 10) : NaN;
      if (!out[id] && !isNaN(n)) out[id] = { position: n };
    });
    AXES.forEach(function (id) { if (id !== skip && !out[id] && cbOf(id)) out[id] = { checked: isChecked(id) }; });
    return out;
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info('[Speed/Q41] ' + msg);
  }
  function send(item, tries) {
    var tag = evTag();
    if (!tag || !R || !R.rawCmd) return Promise.resolve(null);          // gate: no server events -> nothing is sent
    var v = { form: FORM, control: item.control, event: item.event || 'click' };
    if (item.checked !== undefined) v.checked = item.checked;
    if (v.event === 'change' && TRACKS.indexOf(item.control) >= 0) v.position = pos(item.control);   // AI(W906-Q41-SPD-X2) 20260928 (St02-E): read when it goes out
    v.state = stateNow(item.control);                                    // taken now (after the previous ack was applied), not at click time
    return R.rawCmd('form.event', { tag: tag, value: JSON.stringify(v) }).then(function (m) {
      var a = unwrap(m);
      applyChanged(a && a.changed);
      if (a && a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a && a.todo && a.todo.length && window.console) console.info('[Speed/Q41] ' + item.control + ' todo: ' + a.todo.join(' | '));
      return a;
    }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {                             // same cmd+tag+value within 400 ms (WebCmdGuard) -> wait and resend
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(item, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(item, tries + 1); });
      }
      if (/reload page/.test(msg)) {
        say2('Speed：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        QUEUE = [];
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      say2('Speed form.event ' + item.control + ' 沒有執行：' + msg +
           (AXES.indexOf(item.control) >= 0 ? '（勾選框存檔時伺服器會照 golden 補重播）' : ''), '#f88');
      return null;
    });
  }
  function pump() {
    if (sending || !QUEUE.length) return;
    sending = true;
    var item = QUEUE.shift();
    send(item, 0).then(function () { sending = false; pump(); }, function () { sending = false; pump(); });
  }
  function enqueue(item) {
    if (!evTag()) return false;
    if (item.event === 'change') QUEUE = QUEUE.filter(function (q) { return !(q.event === 'change' && q.control === item.control); });   // AI(W906-Q41-SPD-X2) 20260928 (St02-E)
    QUEUE.push(item);
    pump();
    return true;
  }
  if (R && R.editlistGet && !R.__speedQ41Wrapped) {                    // capture the editlist.get reply (events / eventTag); values are applied by the engine
    R.__speedQ41Wrapped = true;
    var get0 = R.editlistGet;
    R.editlistGet = function (st) {
      var pr = get0.apply(R, arguments);
      if (st !== STRUCT) return pr;
      return pr.then(function (d) { LAST = d; QUEUE = []; return d; });
    };
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function usable(el) { return el && !el.disabled && el.getAttribute('aria-disabled') !== 'true'; }
  function onAxis(id) {                                                  // AI(W906-Q41-SPD) 20260928: axis box clicked
    if (APPLYING) return;                                                // value set from ack.changed, not a user click
    if (evTag()) { enqueue({ control: id, checked: isChecked(id) }); return; }   // event mode: golden cbIndexArmClick runs on the server
    cbIndexArmClick();                                                   // no server events: local, as before
  }
  function hook() {
    [['tbAllSpeed', tbAllSpeedChange], ['tbAccSpeed', tbAccSpeedChange], ['tbEPControl', tbEPControlChange]].forEach(function (q) {
      var el = $(q[0]);
      if (el && !el.__q41) {
        el.__q41 = true;
        el.addEventListener('input', q[1]);
        el.addEventListener('change', function () {                     // AI(W906-Q41-SPD-X2) 20260928 (St02-E): released -> golden tb*Change on the server
          if (!APPLYING && evTag() && trackEv(q[0])) enqueue({ control: q[0], event: 'change' });
        });
      }
    });
    AXES.forEach(function (id) {
      var el = $(id);
      if (el && !el.__q41) { el.__q41 = true; el.addEventListener('change', function () { onAxis(id); }); }
    });
    [['spbSpeedAdd', spbSpeedAddClick], ['spbSpeedDec', spbSpeedDecClick], ['spbSelectAll', spbSelectAllClick], ['spbSetToDef', spbSetToDefClick]].forEach(function (q) {
      var b = $(q[0]);
      if (b && !b.__q41) {
        b.__q41 = true;
        b.addEventListener('click', function () {
          if (!usable(b)) return;
          if (evTag()) { enqueue({ control: q[0] }); return; }          // AI(W906-Q41-SPD) 20260928: event mode, golden handler on the server
          q[1]();
        });
      }
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045SpeedC = {                                                 // 探針／除錯用
    tbAllSpeedChange: tbAllSpeedChange, tbAccSpeedChange: tbAccSpeedChange, tbEPControlChange: tbEPControlChange,
    cbIndexArmClick: cbIndexArmClick, spbSelectAllClick: spbSelectAllClick, spbSpeedAddClick: spbSpeedAddClick,
    spbSpeedDecClick: spbSpeedDecClick, spbSetToDefClick: spbSetToDefClick,
    evTag: evTag, trackEv: trackEv, queue: function () { return QUEUE.slice(); }, applyChanged: applyChanged, state: stateNow
  };
})();

/* ---- AI(W906-MOTB-UD) 20261002: the 48 golden TUpDown (cSpeed.dfm, udInXSpd ...) were not generated, so the little up/down
 *   arrows next to every speed / acc box were missing. Built here at the golden place (dfm Left/Top/Width/Height, same parent as the
 *   Associate edit). VCL TUpDown with Associate (UDS_SETBUDDYINT): a click reads the buddy text, adds / subtracts Increment, clamps
 *   to Min..Max (Wrap=False) and writes the buddy text -> the edit's input/change fire like a keypad commit (the engine saves the edit,
 *   golden saves the edit too, never the TUpDown). Max: dfm default 100; golden FormShow cSpeed.cpp:84-122 lowers it to
 *   CosFunction.iLimitMaxSpeed only when bLimitMaxSpeed (false for CC_PTI, CosFunction.cpp:4236). Visible / Enabled = dfm
 *   (udIndexAcc Visible=False Enabled=False), and greyed whenever the buddy edit cannot be used (permission / C++ disabled).
 *   The element ids are "<ud>__ud" (not the golden name) so the C route never mistakes them for its own proxies. */
(function () {
  'use strict';
  var UD = [    ['udInXSpd', 'edInXYSpd', 171, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:857
    ['udInXAcc', 'edInXYAcc', 287, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:869
    ['udInZSpd', 'edInZSpd', 171, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:905
    ['udInZAcc', 'edInZAcc', 287, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:917
    ['udInPitchSpd', 'edInPitchSpd', 171, 112, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:953
    ['udInPitchAcc', 'edInPitchAcc', 287, 112, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:965
    ['udInRotSpd', 'edInRotSpd', 169, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1005
    ['udInRotAcc', 'edInRotAcc', 285, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1029
    ['udPrecisorOpenSp', 'edPrecisorOpenSp', 169, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1368
    ['udPrecisorCloseSp', 'edPrecisorCloseSp', 302, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1392
    ['udSht1Spd', 'edSht1Spd', 171, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1933
    ['udSht1Acc', 'edSht1Acc', 287, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1945
    ['udSht2Spd', 'edSht2Spd', 171, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1981
    ['udSht2Acc', 'edSht2Acc', 287, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:1993
    ['udIndexSpd', 'edIndexSpeed', 234, 35, 17, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:2269
    ['udIndexAcc', 'edIndexAccDec', 234, 73, 16, 24, 1, 100, 10, 0, 0],   // cSpeed.dfm:2295
    ['udOutXSpd', 'edOutXSpd', 171, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:2954
    ['udOutXAcc', 'edOutXAcc', 287, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:2966
    ['udOutZSpd', 'edOutZSpd', 171, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3002
    ['udOutZAcc', 'edOutZAcc', 287, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3014
    ['udOutPitchSpd', 'edOutPitchSpd', 171, 112, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3050
    ['udOutPitchAcc', 'edOutPitchAcc', 287, 112, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3062
    ['udOutRotSpd', 'edOutRotSpd', 169, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3102
    ['udOutRotAcc', 'edOutRotAcc', 285, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3126
    ['udTrayXSpd', 'edTrayXSpd', 234, 35, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3821
    ['udTrayXAcc', 'edTrayXAcc', 234, 73, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:3845
    ['udLoaderSpeedZ', 'edtLoaderSpeedZ', 235, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4041
    ['udLoaderSpeed1', 'edtLoaderSpeed1', 235, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4085
    ['udAuto2Speed1', 'edtAuto2Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4144
    ['udAuto2SpeedZ', 'edtAuto2SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4181
    ['udAuto3Speed1', 'edtAuto3Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4240
    ['udAuto3SpeedZ', 'edtAuto3SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4277
    ['udAuto1Speed1', 'edtAuto1Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4336
    ['udAuto1SpeedZ', 'edtAuto1SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4373
    ['udColorSpeed1', 'edtColorSpeed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4432
    ['udColorSpeedZ', 'edtColorSpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4469
    ['udEmptySpeed1', 'edtEmptySpeed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4528
    ['udEmptySpeedZ', 'edtEmptySpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4565
    ['udAuto6Speed1', 'edtAuto6Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4664
    ['udAuto6SpeedZ', 'edtAuto6SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4701
    ['udAuto5Speed1', 'edtAuto5Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4760
    ['udAuto5SpeedZ', 'edtAuto5SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4797
    ['udAuto4Speed1', 'edtAuto4Speed1', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4856
    ['udAuto4SpeedZ', 'edtAuto4SpeedZ', 237, 4, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4893
    ['udMagCatchYSpd', 'edMagCatchYSpd', 195, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:4988
    ['udMagCatchYAcc', 'edMagCatchYAcc', 311, 36, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:5000
    ['udMagZSpd', 'edMagZSpd', 195, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:5036
    ['udMagZAcc', 'edMagZAcc', 311, 74, 16, 24, 1, 100, 5, 1, 1],   // cSpeed.dfm:5048
  ];
  function $(id) { return document.getElementById(id); }
  function usable(el) {
    if (!el || el.disabled) return false;
    for (var p = el; p && p.nodeType === 1; p = p.parentElement) if (p.getAttribute('aria-disabled') === 'true') return false;
    return true;
  }
  function shown(el) {
    for (var p = el; p && p.nodeType === 1; p = p.parentElement) { var cs = getComputedStyle(p); if (cs.display === 'none' || cs.visibility === 'hidden') return false; }
    return true;
  }
  function fire(el) { ['input', 'change'].forEach(function (n) { var e2; try { e2 = new Event(n, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(n, true, true); } el.dispatchEvent(e2); }); }
  var made = [];
  function build() {
    UD.forEach(function (u) {
      if (!u[9]) return;                                                 // dfm Visible=False (udIndexAcc): golden never shows it
      var ed = $(u[1]), wrap = ed && ed.parentElement;
      if (!ed || !wrap || !wrap.parentElement || $(u[0] + '__ud')) return;
      var box = document.createElement('div');
      box.id = u[0] + '__ud';
      box.title = u[0] + ' : TUpDown（Associate ' + u[1] + '，Min ' + u[6] + ' Max ' + u[7] + ' Increment ' + u[8] + '）';
      box.style.cssText = 'position:absolute;left:' + u[2] + 'px;top:' + u[3] + 'px;width:' + u[4] + 'px;height:' + u[5] + 'px;display:flex;flex-direction:column;box-sizing:border-box;';
      [['▲', 1], ['▼', -1]].forEach(function (b) {
        var k = document.createElement('button');
        k.type = 'button'; k.className = 'btn3d'; k.textContent = b[0];
        k.style.cssText = 'flex:1;width:100%;padding:0;margin:0;font-size:7px;line-height:1;min-height:0;';
        k.addEventListener('click', function (ev) {
          ev.preventDefault(); ev.stopPropagation();
          if (!u[10] || !usable(ed)) return;
          var v = parseInt(String(ed.value), 10); if (isNaN(v)) v = u[6];
          v += b[1] * u[8];
          if (v > u[7]) v = u[7]; if (v < u[6]) v = u[6];                // Wrap=False
          if (String(v) !== String(ed.value)) { ed.value = String(v); fire(ed); }
        });
        box.appendChild(k);
      });
      wrap.parentElement.appendChild(box);
      made.push([box, ed, u]);
    });
  }
  function sync() {                                                      // follow the buddy: hidden with it, greyed when it cannot be used
    made.forEach(function (m) {
      var on = shown(m[1]);
      m[0].style.visibility = on ? '' : 'hidden';
      var en = !!m[2][10] && usable(m[1]);
      Array.prototype.forEach.call(m[0].querySelectorAll('button'), function (k) { k.disabled = !en; });
      m[0].style.opacity = en ? '' : '0.45';
    });
  }
  function start() { build(); sync(); setInterval(sync, 500); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
  window.HT9045SpeedUD = { list: function () { return made.map(function (m) { return m[2][0]; }); }, sync: sync };
})();