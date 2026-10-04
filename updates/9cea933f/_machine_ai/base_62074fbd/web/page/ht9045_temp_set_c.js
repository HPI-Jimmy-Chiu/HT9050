/* ht9045_temp_set_c.js -- Setup.Temp_Set.html (golden TfTemp_Set, uTemp_Set.cpp) Q41 TS-7: base-point count / System-Kit offset over WS form.event; TS-8 (W39): Boost-min rule, page-local.
 * ---------------------------------------------------------------------------
 * AI(W906-Q41-TS7) 20260928 (St02-E helper). St02's new file (hand-written, not a gen_wire.py product). Loaded on
 *   Setup.Temp_Set.html:348 (same line, after St01's inline script -- claim request), so this file wraps HT9045Recipe.editlistGet
 *   outside that script: the channel panels are built and the combos fixed before this file's then runs; the engine applies values after.
 * Server half: St01 70aa17e8 (tools/editlist/Temperature.py events rb1PointClick / rgBasePointClick, FileRW/Temperature.cpp BasePointReplay,
 *   RunPageEvent FileRW/_EditPage.cpp). form.event format FileRW/_FormEvent.h.
 * golden (906_0625_Steven uTemp_Set.cpp; 912 line in brackets):
 *   rb1PointClick :421 [:421] -- OnClick of the five TRadioButtons rb1Point / rb2Point / rb3Point / rb5Point / rb6Point (uTemp_Set.dfm
 *     :10705-:10785, gbBasePoint): SetBasePointIMG (picture only) + UpDateEdit.
 *   rgBasePointClick :6276 [:6384] (dfm :10654, OnClick :10670): UpDateEdit.
 *   UpDateEdit :3671 [:3708]: Visible / Enabled of the myTempPal[i] members and palTemp, from the checked base-point button,
 *     rgBasePoint->ItemIndex, btnSort->Tag and the machine setup. VCL TRadioButton: a click that checks the button runs OnClick;
 *     the server adds VCL TurnSiblingsOff (the other four go checked:false, returned in ack.changed).
 * Gate (same idea as ht9045_config_q41.js; C-route pages do not send "eventTag", only IniConfig does, FileRW/IniConfig.cpp:458):
 *   editlist.get Temperature "events" must list all six controls (older servers list only the TS-1 pair rgIndexHeatMode /
 *   chkTempCalByRecipe) -> send. Tag = eventTag when the server sends one, otherwise "Setup.Temp_Set". Not listed -> nothing is sent,
 *   the page behaves as before (no TS-7 on the page; the server replays the base-point change at save, BasePointReplay).
 * Sent on every click:
 *   rb<n>Point  {"form":"TfTemp_Set","control":"rb3Point","event":"click","checked":true,"state":{...}}
 *   rgBasePoint {"form":"TfTemp_Set","control":"rgBasePoint","event":"click","itemIndex":1,"state":{...}}
 *   state = the values the engine would save, taken when the event goes out, except
 *     (a) controls that have their own server event ("events" keys: the six above and TS-1 rgIndexHeatMode / chkTempCalByRecipe) --
 *         they are synced by their own events; rgIndexHeatMode in state would set ItemIndex without golden rgIndexHeatModeClick
 *         (the compensation table would not be reloaded) and SaveFlow (1) compares against the pre-apply server value, so it would no
 *         longer refuse the save (FileRW/Temperature.cpp BeforeApply / SaveFlow);
 *     (b) channel-panel edits (myTempPal<i>_ed*) whose text equals what the server holds (open values, then values sent or returned in
 *         ack.changed): same 64 KB WS limit and contract as the save wrapper in the inline script (1420 panel members);
 *     (c) cells / dateTime / tag kinds (UpDateEdit does not touch them; the save sends them).
 *   Why state: RunPageEvent applies it with the visibility from before the click, then runs the handler, so edits typed under the old
 *   point count are kept even when they are hidden afterwards (golden saves hidden edits; St01 Temperature.py 'events' note).
 * ack.changed is applied (siblings checked:false, myTempPal<i>_* visible / enabled / editable, palTemp visible = display like the
 *   inline after()). One event at a time; busy -> retry after 450 ms (max 3); not-operator -> keepAlive and retry once; "reload page"
 *   -> reopen the page.
 * btnSort stays page-only (not sent; the server can never operate it, St01 Temperature.py 'events' note). The page has no sort yet.
 * ---------------------------------------------------------------------------
 * AI(W906-W39-TS8) 20260928 (St02-E helper). TS-8: the Boost-min rule of golden edtIdleTime_LongClick, page-local.
 *   PREPARED FOR STEVEN'S RULING W39 = A (port golden as written, the recommended option). The ruling is still open (0928 ChangeLog
 *   section 11); this commit stays on a local branch until Steven answers. If Steven picks B (fix the rule), invert the comparison
 *   in ts8Rule (d1 <= d2 -> d1 >= d2) and change nothing else; if he means "Boost min at least LB min + 2", it is d2 < d1 + 2.
 * golden 906_0625_Steven uTemp_Set.cpp:5736-5747 (912 uTemp_Set.cpp:5814-5825, same text):
 *     fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 1200.0);
 *     d1 = atof(edtLBTempMin->Text.c_str());   d2 = atof(edtBoostTempMin->Text.c_str());
 *     if (d1 <= d2) edtBoostTempMin->Text = AnsiString(d1 + 2.0);
 *   It looks reversed: Boost min is pulled down to LB min + 2 when it is at or above LB min, and left alone when it is below.
 *   Ported as written (W39 = A).
 * Boxes with OnClick = edtIdleTime_LongClick (906_0625_Steven uTemp_Set.dfm; 912 dfm has the same lines): edtIdleTime_Mid :2686,
 *   edtBoostDuration_Mid :2787, edtPostBoost_Mid :2830, edtIdleTime_Short :2924, edtBoostDuration_Short :3034, edtPostBoost_Short
 *   :3089, edtIdleTime_Long :3159, edtBoostDuration_Long :3269, edtPostBoost_Long :3324, edtIdleTime_LB :3695, edtBoostDuration_LB
 *   :3805, edtPostBoost_LB :3860. edtLBTempMin (:3445) and edtBoostTempMin (:3943) use edtLBTempMinClick (uTemp_Set.cpp:6271,
 *   keypad only), so a keypad on those two does NOT run the rule; it runs on the next keypad of any of the twelve boxes.
 * When: golden runs the rule after ShowModal returns (906_0625_Steven myQwertyKeyBoard.cpp:283), whether the keypad was closed by
 *   OK or by Cancel. Page: OK -> the engine's onCommit fires input + change on the box (ht9045_wire_engine.js attachKeyboards) ->
 *   the 'change' listener here. Abort / X / click outside -> qwerty.js close() calls opt.onAbort -> HTQwerty.show is wrapped here,
 *   for the twelve boxes only, to chain onAbort (commit() clears onAbort before close(), so one keypad = one rule run). A change
 *   written by code while APPLYING (ack.changed) does not run it (VCL: setting Text does not fire OnClick).
 * Text: AnsiString(double) = FloatToStrF(v, ffGeneral, 15, 0): up to 15 significant digits, trailing zeros dropped ("32", "32.5");
 *   in range this equals the port's AnsiString::assignDouble (%.15g). The box opens with FormatFloat("0.0") text
 *   (906_0625_Steven uTemp_Set.cpp:3403 / :3409); LB "30.0" + Boost "35.0" -> Boost reads "32" after the rule, as in golden. The
 *   engine's save reads the DOM (gbValue), so the new text is saved; input + change are fired on edtBoostTempMin for any
 *   other listener.
 * Page-local: no server event and no "events" gate. No-op when edtLBTempMin / edtBoostTempMin (or a box) is missing, or when
 *   HTQwerty is missing (then only the OK path runs, through 'change').
 * AI(W906-Q41-TS9) 20260928 (St02-E helper): TS-9, the air-stream offset clamp (page-only, no form.event).
 *   golden (906_0625_Steven uTemp_Set.cpp:6903-6929; 912 :7018-7044, same body) TfTemp_Set::edtSetTempature2AirMachineClick is the
 *   OnClick of four TEdits (906_0625_Steven uTemp_Set.dfm :4903 edt_SetIndexAirstreamTemp, :4914 edt_SetAirstreamTemperatureRang_Index,
 *   :5033 edtSetTempature2AirMachine, :5044 edt_SetAirstreamTemperatureRang_Socket; no Tag line = Tag 0 for all four):
 *     dbSetTemp = atof(fMain->edWorkTemperBase->Text) (:6912); ShowQwertyKey(Sender, N_INTEGER, 0, true, 20, -20) (:6914, modal);
 *     then on edt[Tag] of {edt_SetIndexAirstreamTemp, edtSetTempature2AirMachine} (:6910): dsum = dbSetTemp + atof(Text) (:6916);
 *     dsum < -70 -> Text = -70 - dbSetTemp (:6917); Tag 0: dsum > 35 -> Text = "0" (:6921); Tag 1: dsum > 230 -> 230 - dbSetTemp (:6926).
 *   Tag is 0 for every box, so golden always checks edt_SetIndexAirstreamTemp, whichever of the four was clicked, and the
 *   Tag-1 branch (230 cap on the socket box) is never reached. Ported as is (St02-M 20260927 18:59 item 5).
 *   Page: the keypad is the engine's (ht9045_wire_engine.js attachKeyboards, spec ht9045_wire_tempset.js kb [-20, 20]); this file wraps
 *   HTQwerty.show for those four ids and runs the clamp when the keypad closes, OK or Abort (golden runs it after ShowModal returns
 *   either way). A clamped value is written to the box and input / change are fired, so the save sends it (golden: the TEdit Text).
 *   dbSetTemp comes from editlist.get Temperature "extra".fMain.edWorkTemperBase.text (server FileRW/TempSet_Ts9.cpp, kPage extraJson
 *   FileRW/Temperature.cpp :245). No such extra (older server) -> nothing is clamped, the page behaves as before.
 *   Differences: golden reads the main-screen Text at each click; the page has the value from the last editlist.get (page open /
 *   reload after save). If the work temperature changes while this page is open (GPIB SETTEMP, SECS, the main screen: web windows
 *   are not modal), the clamp uses the older value until the page is reopened. Numbers: atof() and AnsiString(double) (FloatToStr,
 *   15 significant digits) are emulated below.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || R.__tempSetTs7Wrapped) return;
  R.__tempSetTs7Wrapped = true;

  var STRUCT = 'Temperature', FORM = 'TfTemp_Set', PAGE_TAG = 'Setup.Temp_Set';
  var RBS = ['rb1Point', 'rb2Point', 'rb3Point', 'rb5Point', 'rb6Point'];   // golden uTemp_Set.dfm gbBasePoint (Tag 1 / 2 / 4 / 8 / 16)
  var EV_CTL = RBS.concat(['rgBasePoint']);                              // St01 70aa17e8 kTS_Events (TS-7 part)
  var PANEL_RE = /^myTempPal\d+_ed[A-Za-z_]+$/;                          // same members as the inline script PANEL_RE
  var PAL_RE = /^myTempPal\d+_palTemp$/;
  var LAB_RE = /^myTempPal\d+_labName$/;
  var LAST = null;                                                       // last editlist.get Temperature reply (combos already fixed by the inline script)
  var QUEUE = [], sending = false, APPLYING = false;                     // APPLYING: change events fired while applying ack.changed are not user clicks

  function $(id) { return document.getElementById(id); }
  function fire(el, names) {
    (names || ['input', 'change']).forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function evTag() {                                                     // null = no server TS-7 events (older server) -> nothing is sent
    var ev = LAST && LAST.events;
    if (!ev || typeof ev !== 'object') return null;
    for (var i = 0; i < EV_CTL.length; i++) if (!ev[EV_CTL[i]]) return null;
    return (typeof LAST.eventTag === 'string' && LAST.eventTag) ? LAST.eventTag : PAGE_TAG;
  }
  function ancestorDisabled(el) {
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function setVis(el, on) {                                              // VCL Visible (engine: visibility; DFM Visible=False is display:none; .pcPane display is the tab switch)
    el.style.visibility = on ? '' : 'hidden';
    if (on && el.style.display === 'none' && !el.classList.contains('pcPane')) el.style.display = '';
  }
  function setEditable(el, on) {                                         // same rule as the engine gbSetEnabled: only undo what it disabled itself
    if (on && ancestorDisabled(el)) return;
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if (!('disabled' in x)) return;
      if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
      else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
    });
    if (!on) el.setAttribute('aria-disabled', 'true'); else el.removeAttribute('aria-disabled');
  }
  function usable(el) {
    var x = el && (el.tagName === 'INPUT' ? el : el.querySelector('input'));
    return !!x && !x.disabled && el.getAttribute('aria-disabled') !== 'true' && !ancestorDisabled(el);
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
  var VKEYS = ['text', 'checked', 'itemIndex', 'position'];
  function known(id) { return (LAST && LAST.proxies && LAST.proxies[id]) || null; }
  // Panel edits: keep LAST.proxies[id] equal to what the server holds after an event (state applied / ack.changed).
  //   LAST is the same object as HT9045Page.golden().page, which the inline save wrapper compares against ("panel field equal to the
  //   proxy -> not sent, the server keeps its value"). Without this, an edit pushed by a TS-7 state and then typed back to the open
  //   value would not be sent at save and the server would keep the pushed value.
  function remember(id, v) {
    if (!PANEL_RE.test(id) || !LAST || !LAST.proxies) return;
    var p = LAST.proxies[id] || (LAST.proxies[id] = {});
    VKEYS.forEach(function (k) { if (v[k] !== undefined) p[k] = v[k]; });
  }
  function applyChanged(ch) {                                            // ack.changed: {name:{text?,itemIndex?,checked?,visible?,enabled?,editable?,caption?}}, changed keys only
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id] || {}, el = $(id);
        remember(id, v);
        if (!el) return;
        if (v.checked !== undefined) {
          var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]');
          if (c && c.checked !== !!v.checked) { c.checked = !!v.checked; fire(c, ['change']); }
        }
        if (v.itemIndex !== undefined) setIndexEl(el, v.itemIndex);
        if (v.position !== undefined && el.type === 'range') el.value = String(v.position);
        else if (v.text !== undefined && isTextBox(el) && el.value !== String(v.text)) { el.value = String(v.text); fire(el); }
        if (LAB_RE.test(id) && (v.caption !== undefined || v.text !== undefined)) el.textContent = v.caption !== undefined ? v.caption : v.text;
        if (v.visible !== undefined) {
          if (PAL_RE.test(id)) { el.style.display = v.visible ? '' : 'none'; el.style.visibility = v.visible ? '' : 'hidden'; }   // like the inline after(): a hidden alTop panel takes no room
          else setVis(el, !!v.visible);
        }
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
    return null;                                                         // (c) cells / dateTime / tag
  }
  function stateNow() {                                                  // see the header: (a) event controls out, (b) unchanged panel edits out
    var g = (window.HT9045Page && HT9045Page.golden) ? HT9045Page.golden() : null, kinds = (g && g.kinds) || {};
    var ev = (LAST && LAST.events) || {}, out = {};
    Object.keys(kinds).forEach(function (id) {
      if (ev[id]) return;
      var v = valueOf(id, kinds[id]);
      if (!v) return;
      if (PANEL_RE.test(id)) {
        var k = known(id);
        if (k && k.text !== undefined && v.text !== undefined && String(k.text) === String(v.text)) return;
      }
      out[id] = v;
    });
    return out;
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info('[Temp_Set/Q41] ' + msg);
  }
  function send(item, tries) {
    var tag = evTag();
    if (!tag || !R.rawCmd) return Promise.resolve(null);                // gate: no server events -> nothing is sent
    var v = { form: FORM, control: item.control, event: 'click' };
    if (item.checked !== undefined) v.checked = item.checked;
    if (item.itemIndex !== undefined) v.itemIndex = item.itemIndex;
    var st = stateNow();                                                 // taken now (after the previous ack was applied), not at click time
    v.state = st;
    return R.rawCmd('form.event', { tag: tag, value: JSON.stringify(v) }).then(function (m) {
      var a = unwrap(m);
      Object.keys(st).forEach(function (id) { if (PANEL_RE.test(id)) remember(id, st[id]); });   // the server applied the state (RunPageEvent step 4)
      applyChanged(a && a.changed);
      if (a && a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a && a.todo && a.todo.length && window.console) console.info('[Temp_Set/Q41] ' + item.control + ' todo: ' + a.todo.join(' | '));
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
        say2('Temp_Set：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        QUEUE = [];
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      say2('Temp_Set form.event ' + item.control + ' 沒有執行：' + msg + '（存檔時伺服器會照 golden 重播基準點數）', '#f88');
      return null;
    });
  }
  function pump() {
    if (sending || !QUEUE.length) return;  var t1 = window.HT9045EvB2TempSetTs1; if (t1 && ((typeof t1.inflight === 'function' && t1.inflight()) || (typeof t1.queue === 'function' && t1.queue().length))) { setTimeout(pump, 60); return; }   // AI(W906-Q41-TS7) 20260928 (St02-E): TEMPORARY until St01's shared Temp_Set queue -- let St01 ht9045_temp_set_ts1.js TS-1 / TS-2 acks land before TS-7 takes its state (their header :44-45; probe :379-381)
    sending = true;
    var item = QUEUE.shift();
    send(item, 0).then(function () { sending = false; pump(); }, function () { sending = false; pump(); });
  }
  function enqueue(item) {
    if (!evTag()) return false;
    QUEUE.push(item);
    pump();
    return true;
  }

  function onRadio(id) {                                                 // golden rb1PointClick (VCL: the click that checks the button)
    if (APPLYING || !evTag()) return;
    var el = $(id), c = el && el.querySelector('input[type="radio"]');
    if (!c || !c.checked || !usable(el)) return;
    enqueue({ control: id, checked: true });
  }
  function onBasePoint() {                                               // golden rgBasePointClick (ItemIndex changed)
    if (APPLYING || !evTag()) return;
    var el = $('rgBasePoint'), v = valueOf('rgBasePoint', 'itemIndex');
    if (!el || !v || v.itemIndex < 0 || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) return;
    enqueue({ control: 'rgBasePoint', itemIndex: v.itemIndex });
  }
  // AI(W906-W39-TS8) 20260928 (St02-E helper): TS-8, prepared for W39 = A (see the header). The twelve boxes whose golden
  //   OnClick is edtIdleTime_LongClick (906_0625_Steven uTemp_Set.dfm :2686 ... :3860).
  var TS8_BOXES = ['edtIdleTime_Mid', 'edtBoostDuration_Mid', 'edtPostBoost_Mid', 'edtIdleTime_Short', 'edtBoostDuration_Short',
                   'edtPostBoost_Short', 'edtIdleTime_Long', 'edtBoostDuration_Long', 'edtPostBoost_Long', 'edtIdleTime_LB',
                   'edtBoostDuration_LB', 'edtPostBoost_LB'];
  function ts8Bound(id) { return TS8_BOXES.indexOf(id) >= 0; }
  function atof(s) {
    // C atof: leading white space, [sign] digits [. digits] [e [sign] digits], stops at the first other char; no number -> 0
    var m = /^[ \t\n\v\f\r]*([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)/.exec(String(s == null ? '' : s));
    return m ? Number(m[1]) : 0;
  }
  function vclFloatStr(v) {
    // VCL AnsiString(double) = FloatToStrF(v, ffGeneral, 15, 0): fixed while 1e-4 <= |v| < 1e15, else "1.5E20" / "1E-5"
    if (v !== v) return 'NAN';
    if (!isFinite(v)) return v > 0 ? 'INF' : '-INF';
    if (v === 0) return '0';
    var m = /^(-?)(\d)\.?(\d*)e([+-]\d+)$/.exec(v.toExponential(14));
    var sign = m[1], dig = (m[2] + m[3]).replace(/0+$/, ''), e = parseInt(m[4], 10);
    if (e < -4 || e > 14) return sign + dig.charAt(0) + (dig.length > 1 ? '.' + dig.slice(1) : '') + 'E' + e;
    if (e < 0) return sign + '0.' + new Array(-e).join('0') + dig;
    while (dig.length < e + 1) dig += '0';
    return sign + dig.slice(0, e + 1) + (dig.length > e + 1 ? '.' + dig.slice(e + 1) : '');
  }
  function ts8Rule() {
    // golden edtIdleTime_LongClick after ShowQwertyKey returns (906_0625_Steven uTemp_Set.cpp:5739-5746)
    var lb = $('edtLBTempMin'), bo = $('edtBoostTempMin');
    if (!lb || !bo) return;
    var d1 = atof(lb.value), d2 = atof(bo.value);
    // W39 = A: golden as written. B (fix) would be d1 >= d2 on the next line, nothing else.
    if (d1 <= d2) {
      var t = vclFloatStr(d1 + 2.0);
      // VCL TControl::SetText does nothing when the text is the same; otherwise set it and tell the page
      if (bo.value !== t) { bo.value = t; fire(bo); }
    }
  }
  function ts8WrapKeypad() {
    // Abort path (see the header): chain opt.onAbort for the twelve boxes only; every other keypad goes to qwerty.js untouched
    var Q = window.HTQwerty;
    if (!Q || typeof Q.show !== 'function' || Q.__w39ts8) return;
    var show0 = Q.show;
    Q.show = function (target, f, o) {
      if (target && ts8Bound(target.id) && o && typeof o === 'object') {
        var ab0 = o.onAbort;
        o.onAbort = function () { if (typeof ab0 === 'function') ab0(); ts8Rule(); };
      }
      return show0.apply(this, arguments);
    };
    Q.__w39ts8 = true;
  }
  function hook() {
    RBS.forEach(function (id) {
      var el = $(id);
      if (el && !el.__q41ts7) { el.__q41ts7 = true; el.addEventListener('change', function () { onRadio(id); }); }
    });
    var rg = $('rgBasePoint');
    if (rg && !rg.__q41ts7) { rg.__q41ts7 = true; rg.addEventListener('change', onBasePoint); }
    TS8_BOXES.forEach(function (id) {
      // AI(W906-W39-TS8) 20260928 (St02-E helper): OK on the keypad -> the engine fires change -> golden rule
      var el = $(id);
      if (el && !el.__w39ts8) { el.__w39ts8 = true; el.addEventListener('change', function () { if (!APPLYING) ts8Rule(); }); }
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();
  ts8WrapKeypad();

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    var pr = get0.apply(R, arguments);
    if (st !== STRUCT) return pr;
    return pr.then(function (d) { LAST = d; QUEUE = []; return d; });
  };

  // ---- AI(W906-Q41-TS9) 20260928 (St02-E helper): TS-9 air-stream clamp (see the header) ----------------------------------------
  var TS9_BOXES = {                                                      // golden dfm OnClick = edtSetTempature2AirMachineClick; value = Tag
    edt_SetIndexAirstreamTemp: 0, edt_SetAirstreamTemperatureRang_Index: 0,
    edtSetTempature2AirMachine: 0, edt_SetAirstreamTemperatureRang_Socket: 0
  };
  var TS9_EDT = ['edt_SetIndexAirstreamTemp', 'edtSetTempature2AirMachine'];   // golden edt[2] (:6910)
  // atof: TS-8's C atof above serves TS-9 too (AI(W906-Q41-TS9) merge with W39 TS-8, 20260928)
  function floatToStr(x) { return vclFloatStr(x); }                     // BCB AnsiString(double): TS-8's FloatToStrF(ffGeneral, 15) emulation
  function ts9WorkText() {                                               // null = the server sent no TS-9 extra -> no clamp
    var x = LAST && LAST.extra && LAST.extra.fMain && LAST.extra.fMain.edWorkTemperBase;
    return (x && typeof x.text === 'string') ? x.text : null;
  }
  function ts9Clamp(senderId) {                                          // golden :6909-:6928, after ShowQwertyKey returned
    var wt = ts9WorkText(), tag = TS9_BOXES[senderId];
    if (wt === null || tag === undefined) return null;
    var el = $(TS9_EDT[tag]);
    if (!el) return null;
    var dbSetTemp = atof(wt), dsum = dbSetTemp + atof(el.value), nv = null;
    if (dsum < -70) nv = floatToStr(-70 - dbSetTemp);
    if (tag === 0) { if (dsum > 35) nv = '0'; }
    else { if (dsum > 230) nv = floatToStr(230 - dbSetTemp); }
    if (nv !== null && el.value !== nv) {
      el.value = nv;
      fire(el);                                                          // the engine marks it changed; the save sends it
      if (window.console) console.info('[Temp_Set/Q41] TS-9 ' + senderId + ': ' + TS9_EDT[tag] + ' -> ' + nv + ' (work temp ' + wt + ', sum ' + dsum + ')');
    }
    return nv;
  }
  function ts9Hook() {                                                   // wrap the keypad for the four boxes; other fields unchanged
    var Q = window.HTQwerty;
    if (!Q || typeof Q.show !== 'function' || Q.__q41ts9) return;
    Q.__q41ts9 = true;
    var show0 = Q.show;
    Q.show = function (target, f, o) {
      var id = target && target.id;
      if (!id || TS9_BOXES[id] === undefined || ts9WorkText() === null) return show0.apply(this, arguments);
      var o2 = {}, k, c0 = o && o.onCommit, a0 = o && o.onAbort;
      if (o) for (k in o) o2[k] = o[k];
      o2.onCommit = function (v) { if (typeof c0 === 'function') c0(v); ts9Clamp(id); };   // OK: engine writes + fires first
      o2.onAbort = function () { if (typeof a0 === 'function') a0(); ts9Clamp(id); };     // Abort / X / outside click
      return show0.call(this, target, f, o2);
    };
  }
  ts9Hook();

  window.HT9045TempSetC = {                                               // probe / debug
    evTag: evTag, queue: function () { return QUEUE.slice(); }, applyChanged: applyChanged, state: stateNow,
    known: known, ts8Rule: ts8Rule, vclFloatStr: vclFloatStr, ts9: { workText: ts9WorkText, clamp: ts9Clamp, atof: atof, floatToStr: floatToStr }   // AI(W906-Q41-TS9) 20260928
  };
})();
