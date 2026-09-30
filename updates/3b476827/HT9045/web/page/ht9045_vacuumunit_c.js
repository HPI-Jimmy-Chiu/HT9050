/* ht9045_vacuumunit_c.js -- HW.VacuumUnit.html（golden TfVacuumUnit，VacuumUnit\VacuumUnit.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_VacuumUnit.cpp（WS editlist.get／editlist.save tag=TestIF_File_VacuumUnit）。
 * golden elVacuumUnit（HTEditList）→ <recipe>\HandlerCondition.Data [Vacuum Threshold] IndexArm1_<row>_<col>…／[Tray] CheckBox1、Edit1。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'HW.VacuumUnit.html' → 'TestIF_File_VacuumUnit'，整合者登錄）照通用規則讀寫：
 *   清單筆 id＝面板 edSV（hwidgets.js makeVacuumPanel：myPal<Arm>_<iCol>_<iRow>_edSV）、CheckBox1、Edit1；
 *   面板 myPal<Arm>_<iCol>_<iRow> 的 visible＝golden SetPanelPos／ShowSuckMode（Index 面板只顯示 FTestSuck.iShtRow×iShtCol 那幾顆），
 *   看不見的面板值不收（後端丟掉，沿用檔案值）。
 * 本檔補四件事：
 *
 *  (1) 面板數＝golden Initial（:38-49）：HT9045 Index 4 欄；HT9046／HT9046LS／HT1032／USE_46_SUCKER_DB=1 是 8 欄。頁面是照 4 欄產生的
 *      → editlist.get 回應裡有、頁面沒有的 myPal*_<c>_<r>_edSV，在引擎套值之前照 makeVacuumPanel 補建（caption 照 golden
 *      MyVacuumPanel.cpp:20-24 sIndexName_16／sInOutName_8），小鍵盤照 golden edSVClick。
 *  (2) 存檔鈕 spbSave → 引擎 HT9045Page.save()（golden spbSaveClick 沒有 YES/NO → 不登錄 GB_SAVE_Q，頁面仍確認一次，引擎慣例）。
 *      這一頁沒有 B 路欄位，引擎不會自己攔存檔鈕 → 由這裡綁。sbtExit 照頁面內建 .exitbtn 關視窗（golden sbtExitClick → Close →
 *      FormClose：ReadFile＋DoIniDataToForm，不寫檔）。
 *  (3) AI(W906-VACUNIT-1203) 20260930：硬體那一半 —— 原本這裡把硬體鈕全部停用，現在接上 C++（EastSun 20260930
 *      「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」）。WS 指令（FileRW/TestIF_File_VacuumUnit.cpp 檔尾）：
 *        vacuum.open／vacuum.get {act:'timer'}（每秒一次＝golden tmr1Timer）／vacuum.close   —— 讀，不拿操作權杖
 *        vacuum.setSV  {panel,kPa}                 每顆面板的 Set（golden btnSVClick → WriteVaccumThreshold）
 *        vacuum.do     {panel,which:'on'|'off',value}  ^ 吸真空／v 破真空（golden btnVaccumOnOffOnClick；value＝按下之後的 Down）
 *        vacuum.setAll {arm:'in'|'index'|'out',kPa}   Set All Value 三顆（golden btnSetInArmClick，Tag 由 C++ 照 DFM 表對應）
 *        vacuum.reset  {}                            Reset（golden sbResetClick）
 *      規則（每一條都有理由）：
 *        * 畫面上的現值／閥值／Event／燈／^v 的底色＝C++ 快照裡 golden 真的畫出來的東西，不是頁面自己記的狀態；
 *          讀不到的燈與 DO 畫成灰色（不知道），不畫成「關」。
 *        * 一顆鈕只在快照說 available 時才能按；不能按的鈕鎖住，滑鼠停在上面看得到 C++ 給的原因（例如「ring 1 站 0x50 回報的模組是
 *          ECx-P32-HON …，不在 ECAT-VC8 身分表」）。瀏覽器不做互鎖判斷 —— 互鎖、身分檢查、範圍檢查都在 C++，這裡只是不讓人按注定被拒的鈕。
 *        * 每一次寫入先跳確認框，送出後把回應講清楚：已送到卡片／DRY RUN（沒有真的寫）／被拒（原因）／卡片回錯誤。
 *        * 視窗關掉（外框 HT_WIN open=false）就停止輪詢並送 vacuum.close；golden FormClose 也把 tmr1 關掉。
 *      ⚠ 今天這台的 ring 上沒有 ECAT-VC8（EastSun 20260930「沒有，先把軟體做好」）：每顆面板會顯示 golden 的 999.0／Error3／Error5，
 *        每顆硬體鈕鎖住並寫明原因。這是預期，不是故障。
 *  (4) 小鍵盤：golden edSVClick（MyVacuumPanel.cpp:382）N_DOUBLE (-116.0, 148.0)；Edit1 golden 沒有小鍵盤事件 → 引擎的通用鍵盤
 *      （範圍取清單筆 0~300）。Set All 的三個輸入框同 golden edSetInArmClick（VacuumUnit.cpp:599）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_VacuumUnit';
  var PAGE = 'HW.VacuumUnit.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__vacuumCWrapped) return;
  R.__vacuumCWrapped = true;

  var get0 = R.editlistGet;
  var ARMS = { myPalArm1: 'scrlbxIndexArm1', myPalArm2: 'scrlbxIndexArm2', myPalInArm: 'scrlbxInArm', myPalOutArm: 'scrlbxOutArm' };
  var KB_SV = ['DOUBLE', 0, true, -116.0, 148.0];          // golden MyVacuumPanel.cpp:382 edSVClick / VacuumUnit.cpp:599 edSetInArmClick
  var ON_BG = '#45b0dc', OFF_BG = '#014a9d', NA_BG = '#8a8a8a';   // TBtnPanelLane TrueColor / Color；灰＝讀不到

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[VacuumUnit/C] ' + msg);
  }
  function sayNow(msg, colour) {                        // 取代狀態列（硬體鈕的回應不要疊在舊訊息後面）
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66');
    console.info('[VacuumUnit/C] ' + msg);
  }

  /* ---- (3a) 鈕的鎖：鎖住＝不能按，title 寫原因；div 形的 TBtnPanelLane 換成不帶監聽器的複本（它自己的 toggle 會假裝有動作） ---- */
  function plain(el) {
    if (!el || el.__vuPlain) return el;
    if (el.tagName !== 'BUTTON' && el.tagName !== 'INPUT') {
      var n = el.cloneNode(true);
      el.parentNode.replaceChild(n, el);
      el = n;
    }
    el.__vuPlain = true;
    if (el.__vuTitle == null) el.__vuTitle = el.title || '';
    return el;
  }
  function setLock(el, locked, why) {
    el = plain(el);
    if (!el) return;
    if (el.tagName === 'BUTTON' || el.tagName === 'INPUT') el.disabled = !!locked;
    else {
      el.style.cursor = locked ? 'not-allowed' : 'pointer';
      el.style.opacity = locked ? '0.55' : '1';
      el.setAttribute('aria-disabled', locked ? 'true' : 'false');
    }
    el.__vuLocked = !!locked;
    el.__vuWhy = why || '';
    el.title = el.__vuTitle ? el.__vuTitle + (why ? '\n' + why : '') : (why || '');
  }
  function panelSetBtn(gb) { return gb ? gb.querySelector('button') : null; }
  function lockPanel(gb, why) {
    if (!gb) return;
    setLock(panelSetBtn(gb), true, why);
    setLock($(gb.id + '_bplOn'), true, why);
    setLock($(gb.id + '_bplOff'), true, why);
  }
  var WAIT_NOTE = '等 C++ 的即時快照（vacuum.open）——還沒確認這一站是不是 ECAT-VC8，所以先鎖住';
  function lockStatic(why) {
    ['btnSetInArm', 'btnSetIndexArm', 'btnSetOutArm', 'sbReset'].forEach(function (id) { setLock($(id), true, why); });
    Array.prototype.forEach.call(document.querySelectorAll('.vacpanel'), function (gb) { lockPanel(gb, why); });
  }

  /* ---- (1) golden Initial 的面板數 --------------------------------------------------------- */
  function caption(arr, col, row) {                  // golden MyVacuumPanel.cpp:20-24
    if (arr === 'myPalArm1' || arr === 'myPalArm2') return '    ' + 'AB'.charAt(row) + 'abcdefgh'.charAt(col) + '    ';
    return '     ' + 'ACEGBDFH'.charAt(row * 4 + col) + '    ';
  }
  function kbAttach(el) {
    if (!window.HTQwerty || el.__vuKb) return;
    el.__vuKb = true;
    el.setAttribute('readonly', 'readonly');
    el.style.cursor = 'pointer';
    el.addEventListener('mousedown', function (ev) {
      ev.preventDefault();
      if (el.disabled) return;
      HTQwerty.show(el, HTQwerty.N[KB_SV[0]] || 0, { dp: KB_SV[1], checkRange: KB_SV[2], min: KB_SV[3], max: KB_SV[4] });
    });
  }
  function buildMissing(d) {
    var made = [];
    var W = window.HTWidgets;
    var ents = ((d && d.lists && d.lists.elVacuumUnit) || {}).entries || [];
    ents.forEach(function (e) {
      var m = /^(myPal(?:Arm1|Arm2|InArm|OutArm))_(\d+)_(\d+)_edSV$/.exec(e.id || '');
      if (!m || $(e.id)) return;
      var arr = m[1], col = +m[2], row = +m[3];
      var box = $(ARMS[arr]), host = box && box.querySelector('.htWidgetHost');
      if (!host || !W || !W.makeVacuumPanel) return;
      var gb = W.makeVacuumPanel({ name: arr + '_' + col + '_' + row, caption: caption(arr, col, row), cur: '0.0',
                                   event: 'Event', threshold: '0.0', sv: '0', left: col * 81, top: 10 + row * 177 });
      host.appendChild(gb);
      host.style.width = Math.max(parseInt(host.style.width, 10) || 0, (col + 1) * 81) + 'px';
      lockPanel(gb, (VU.snap ? '' : WAIT_NOTE));
      if ($(e.id)) kbAttach($(e.id));
      made.push(arr + '_' + col + '_' + row);
    });
    if (made.length) setTimeout(function () {
      say2('ⓘ 後端 golden Initial 建了 ' + made.length + ' 個頁面沒有的面板（Index 8 欄機種），已補上：' + made.join(', '));
    }, 0);
  }

  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      buildMissing(d);                       // 引擎在這個 promise 的 then 裡才套值：元件要先在
      setTimeout(vuOpen, 0);                 // (3) golden FormShow 跑完（editlist.get）→ 即時畫面開始
      return d;
    });
  };

  /* ---- (2) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('spbSave');
    if (!b || b.__vuC) return;
    b.__vuC = true;
    b.title = 'spbSave : golden spbSaveClick（VacuumUnit.cpp:378）→ SaveSetupFile → elVacuumUnit->SaveEditTextToFile（HandlerCondition.Data）→ ReadFile';
    b.addEventListener('click', function (ev) {
      ev.preventDefault(); ev.stopPropagation();
      if (!usable(b)) return;
      if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔', '#f88'); return; }
      if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
        say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88');
        return;
      }
      HT9045Page.save();
    });
  }
  bindSave();

  /* ---- (3) 硬體那一半：即時快照與硬體鈕（AI(W906-VACUNIT-1203) 20260930） ----------------------------- */
  var VU = { snap: null, live: false, timer: null, inflight: false, busy: false, lastPollErr: '', said: '' };

  function unwrap(m) {                                  // 伺服器把 JSON 欄位併進 ack；舊寫法放在 value 字串裡（同 recipe client）
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function vuCmd(cmd, obj) { return R.rawCmd(cmd, { value: JSON.stringify(obj || {}) }).then(unwrap); }

  function stationOf(s, p) { return (s && s.stations && s.stations[p.st]) || null; }
  function hex2(n) { var h = (n >>> 0).toString(16).toUpperCase(); return '0x' + (h.length < 2 ? '0' + h : h); }
  function addrText(s, p) {
    var st = stationOf(s, p);
    return 'ring ' + (st ? st.ring : '?') + '・站 ' + (st ? hex2(st.station) : '?') + '・VC' + p.vc +
           '（吸真空 DO ' + p.onCh + '／破真空 DO ' + p.offCh + '／真空 OK DI ' + p.diCh + '）';
  }
  function paintLed(el, v) {
    if (!el) return;
    if (el.setValue) el.setValue(v === true);
    el.style.opacity = (v === null || v === undefined) ? '0.4' : '1';
  }
  function paintBpl(el, v, locked) {
    el = plain(el);
    if (!el) return;
    el.style.background = v === true ? ON_BG : v === false ? OFF_BG : NA_BG;
    el.dataset.down = v === true ? '1' : '0';
    el.style.opacity = locked ? '0.55' : '1';
  }

  function render(s) {
    if (!s || !s.panels) return;
    VU.snap = s;
    var armOk = { myPalInArm: false, myPalArm1: false, myPalArm2: false, myPalOutArm: false };
    var firstWhy = {};
    s.panels.forEach(function (p) {
      var gb = $(p.id);
      if (!gb) return;                                   // 頁面沒有這顆（例如欄數不同）
      var arr = p.id.replace(/_\d+_\d+$/, '');
      if (p.vis) {
        if (gb.setCurrent) gb.setCurrent(p.cur);
        if (gb.setThreshold) gb.setThreshold(p.thr);
        if (gb.setEvent) gb.setEvent(p.evt, p.evtColor);
      }
      paintLed($(p.id + '_myld1'), p.led);
      var why = p.available ? '' : (p.reason || '不可用');
      var where = addrText(s, p);
      var setBtn = panelSetBtn(gb);
      setLock(setBtn, !p.available, (p.available ? '' : '🔒 ' + why + '\n') + where);
      setLock($(p.id + '_bplOn'), !p.available, (p.available ? '' : '🔒 ' + why + '\n') + 'bplOn 吸真空 ' + where);
      setLock($(p.id + '_bplOff'), !p.available, (p.available ? '' : '🔒 ' + why + '\n') + 'bplOff 破真空 ' + where);
      paintBpl($(p.id + '_bplOn'), p.on, !p.available);
      paintBpl($(p.id + '_bplOff'), p.off, !p.available);
      if (p.available) armOk[arr] = true;
      else if (!firstWhy[arr]) firstWhy[arr] = why;
    });
    var block = s.writeBlock || '';
    var idxOk = armOk.myPalArm1 || armOk.myPalArm2;
    setLock($('btnSetInArm'), !armOk.myPalInArm, armOk.myPalInArm ? 'Set All：InArm（golden btnSetInArmClick Tag 0）' : '🔒 ' + (block || firstWhy.myPalInArm || '沒有可寫的 ECAT-VC8'));
    setLock($('btnSetIndexArm'), !idxOk, idxOk ? 'Set All：Index Arm1／Arm2（Tag 1）' : '🔒 ' + (block || firstWhy.myPalArm1 || firstWhy.myPalArm2 || '沒有可寫的 ECAT-VC8'));
    setLock($('btnSetOutArm'), !armOk.myPalOutArm, armOk.myPalOutArm ? 'Set All：OutArm（Tag 2）' : '🔒 ' + (block || firstWhy.myPalOutArm || '沒有可寫的 ECAT-VC8'));
    var resetOk = !!s.live && !block;
    setLock($('sbReset'), !resetOk, resetOk ? 'Reset：golden sbResetClick（清面板事件，下一個 tick 重讀並重寫閥值模式）' : '🔒 ' + (block || '即時畫面沒有在跑'));
    // Set All 的三個輸入框不鎖（改數字不會動到硬體）；小鍵盤由引擎 attachKeyboards 依 (4) 的 kb 表掛上，這裡不再掛第二個

    // 一行總結（換狀況才講一次）
    var nOk = (s.stations || []).filter(function (x) { return x.writable; }).length;
    var line;
    if (!s.route) line = '真空單元：這個 wb_serve 沒有 ECAT-VC8 路由 —— 只有讀寫檔（Save），硬體鈕鎖住';
    else if (block) line = '真空單元：' + block;
    else if (!nOk) {
      var st0 = (s.stations || []).filter(function (x) { return !x.writable; })[0];
      line = '真空單元：ring 上沒有可用的 ECAT-VC8（' + (s.stations || []).length + ' 站都不行）—— 面板顯示 golden 的 999.0／Error3／Error5，硬體鈕鎖住。' +
             (st0 ? '\n例：' + st0.why : '');
    } else line = '真空單元：即時畫面（golden tmr1Timer 每秒）—— ' + nOk + '／' + (s.stations || []).length + ' 站 ECAT-VC8 可用' + (s.dryRun ? '（DRY RUN：按了也不會真的寫）' : '');
    if (line !== VU.said) { VU.said = line; say2(line, nOk ? '#9f9' : '#ffcc66'); }
  }

  function poll() {
    if (VU.inflight || !VU.live) return;
    VU.inflight = true;
    vuCmd('vacuum.get', { act: 'timer' }).then(function (s) {
      VU.inflight = false; VU.lastPollErr = '';
      if (s && s.open === false) {                               // golden FormClose 跑過了（視窗關掉），或 wb_serve 重新啟動過
        stopPolling();
        lockStatic('Vacuum Unit 在 wb_serve 那邊沒有開著（視窗關過、或 wb_serve 重新啟動過）——請重新開啟這個視窗');
        return;
      }
      render(s);
    }, function (e) {
      VU.inflight = false;
      var m = String(e && e.message || e);
      if (m !== VU.lastPollErr) { VU.lastPollErr = m; say2('⚠ 真空單元即時畫面讀不到：' + m, '#f88'); lockStatic('即時畫面讀不到（' + m + '）'); }
    });
  }
  function stopPolling() {
    VU.live = false;
    if (VU.timer) { clearInterval(VU.timer); VU.timer = null; }
  }
  function vuOpen() {
    vuCmd('vacuum.open', {}).then(function (s) {
      render(s);
      VU.live = true;
      if (!VU.timer) VU.timer = setInterval(poll, 1000);   // golden tmr1 Interval 1000 ms（DFM 預設）
    }, function (e) {
      lockStatic('vacuum.open 被拒：' + (e && e.message || e));
      say2('⚠ 真空單元即時畫面沒有啟動：' + (e && e.message || e), '#f88');
    });
  }
  window.addEventListener('message', function (ev) {      // 外框 background.html 的視窗狀態（ht9045_wire_engine.js H4 同一個訊息）
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    if (m.open === false && (VU.live || VU.timer)) {
      stopPolling();
      vuCmd('vacuum.close', {}).then(null, function () {});
      lockStatic('視窗已關閉');
    }
  });

  // 操作權杖：跟 IO 頁同一個作法（ht9045_io_do.js）——按下去就接管，30 秒沒再按就還
  var releaseTimer = null;
  function scheduleRelease() {
    if (releaseTimer) clearTimeout(releaseTimer);
    releaseTimer = setTimeout(function () {
      releaseTimer = null;
      if (VU.busy) { scheduleRelease(); return; }
      var st = (typeof R.status === 'function') ? R.status() : null;
      if (st && st.holdsToken) return;
      R.rawCmd('control.release').then(null, function () {});
    }, 30000);
  }

  function describe(what, a) {
    var ws = (a && a.writes) || [];
    var lines = [what];
    if (a && a.note) lines.push(a.note);
    if (!ws.length) lines.push('（沒有任何寫入：golden 在這一步沒有要寫卡片的東西）');
    ws.slice(0, 12).forEach(function (w) {
      var tgt = 'ring ' + w.ring + ' 站 ' + hex2(w.station) + ' ' + (w.kind === 'do' ? 'DO ' + w.chan : hex2(w.index).replace('0x', '') + 'h:' + hex2(w.sub).replace('0x', '') + 'h') + ' = ' + w.value;
      var res = w.issued ? (w.rc === 0 ? '已送到卡片' : '卡片回錯誤 0x' + (w.rc >>> 0).toString(16).toUpperCase()) : w.dryRun ? 'DRY RUN（沒有真的寫）' : '被拒';
      lines.push('・' + tgt + ' → ' + res + (w.why ? '：' + w.why : ''));
    });
    if (ws.length > 12) lines.push('…還有 ' + (ws.length - 12) + ' 筆（' + (a.issued || 0) + ' 送出／' + (a.dry || 0) + ' DRY／' + (a.refused || 0) + ' 被拒／' + (a.failed || 0) + ' 錯誤）');
    var level = !ws.length ? 'info' : (a.issued && !a.refused && !a.failed) ? 'ok' : (a.dry && !a.refused && !a.failed) ? 'dry' : 'err';
    return { text: lines.join('\n'), colour: level === 'ok' ? '#9f9' : level === 'dry' ? '#ffcc66' : level === 'err' ? '#f88' : '#cde' };
  }

  function press(cmd, obj, label) {
    if (VU.busy) { sayNow('上一個指令還在路上，請稍候', '#ffcc66'); return; }
    VU.busy = true;
    var acq = R.rawCmd('control.takeover').then(function () { return null; }, function (e) { return e || new Error('control.takeover failed'); });
    vuCmd(cmd, obj).then(function (a) {
      var d = describe(label, a);
      sayNow(d.text, d.colour);
      if (a && a.panels) render(a);
      return a;
    }, function (e) {
      return acq.then(function (ae) {
        var m = String(e && e.message || e);
        if (/no ack within/.test(m)) m = '沒有回應（逾時）——不代表沒有執行，看一下畫面與 wb_serve 主控台';
        else if (ae && !/socket closed|no ack within/i.test(String(ae.message || ae))) m += '\n（操作權杖：' + (ae.message || ae) + '）';
        sayNow('❌ ' + label + '\n沒有送出：' + m, '#f88');
      });
    }).then(function () { VU.busy = false; scheduleRelease(); poll(); });
  }

  function panelOf(el) { return el && el.closest ? el.closest('.vacpanel') : null; }
  function snapPanel(id) { return VU.snap && VU.snap.panels ? VU.snap.panels.filter(function (p) { return p.id === id; })[0] : null; }
  function captionOf(gb) { var c = gb && gb.querySelector('span'); return c ? c.textContent.trim() : ''; }
  function numOf(el) {
    var v = el ? String(el.value).trim() : '';
    var n = Number(v);
    return (v === '' || !isFinite(n)) ? null : n;
  }
  function kpaOk(n) { return n !== null && n >= -116 && n <= 148; }

  function onSet(gb) {
    var p = snapPanel(gb.id);
    if (!p || !p.available) { sayNow('❌ ' + gb.id + ' 不能寫：' + (p ? p.reason : '還沒有快照'), '#f88'); return; }
    var n = numOf($(gb.id + '_edSV'));
    if (!kpaOk(n)) { sayNow('❌ 閥值要是 -116 ~ 148 kPa 的數字（golden 小鍵盤範圍）', '#f88'); return; }
    if (!window.confirm('寫入 ECAT-VC8 真空閥值\n面板 ' + captionOf(gb) + '（' + gb.id + '）\n' + addrText(VU.snap, p) +
                        '\n閥值 = ' + n + ' kPa（SDO ' + (0x8000 + 0x10 * p.vc).toString(16).toUpperCase() + 'h:13h）' +
                        (p.modeOk ? '' : '\n（這一格的閥值模式還沒寫成功：下一個 tick 照 golden 寫 02h = 1）') + '\n\n確定送出？')) return;
    press('vacuum.setSV', { panel: gb.id, kPa: n }, 'Set（golden btnSVClick）' + captionOf(gb) + ' → ' + n + ' kPa');
  }
  function onBpl(gb, which) {
    var p = snapPanel(gb.id);
    if (!p || !p.available) { sayNow('❌ ' + gb.id + ' 不能寫：' + (p ? p.reason : '還沒有快照'), '#f88'); return; }
    var cur = which === 'on' ? p.on : p.off;
    if (cur !== true && cur !== false) { sayNow('❌ 讀不到這個輸出目前的狀態，不知道要切成什麼，所以不送', '#f88'); return; }
    var down = cur ? 0 : 1;                              // golden Ptr->Down = !Ptr->Down
    var name = which === 'on' ? '吸真空（^ bplOn）' : '破真空（v bplOff）';
    if (!window.confirm(name + ' → ' + (down ? 'ON' : 'OFF') + '\n面板 ' + captionOf(gb) + '（' + gb.id + '）\n' + addrText(VU.snap, p) +
                        '\nDO 通道 ' + (which === 'on' ? p.onCh : p.offCh) + '\n\n確定送出？')) return;
    press('vacuum.do', { panel: gb.id, which: which, value: down }, name + ' ' + (down ? 'ON' : 'OFF') + '：' + captionOf(gb));
  }
  function onSetAll(arm, editId, label) {
    var n = numOf($(editId));
    if (!kpaOk(n)) { sayNow('❌ Set All 的值要是 -116 ~ 148 kPa 的數字（golden 小鍵盤範圍）', '#f88'); return; }
    if (!window.confirm('Set All Value：' + label + ' 的每一格閥值都寫成 ' + n + ' kPa\n（golden btnSetInArmClick：先把每一格的 edSV 改成 ' + Math.trunc(n) +
                        '，再逐格寫 ECAT-VC8；不是 VC8 的站會被 C++ 拒絕並列出原因）\n\n確定送出？')) return;
    var prefix = arm === 'in' ? ['myPalInArm'] : arm === 'out' ? ['myPalOutArm'] : ['myPalArm1', 'myPalArm2'];
    press('vacuum.setAll', { arm: arm, kPa: n }, 'Set All（' + label + '）→ ' + n + ' kPa');
    // golden :563 / :575-576 / :590 fVacuumUnit->myPal*->edSV->Text=(int)dValue —— 頁面的 edSV 也跟著改（按 Save 才存檔）
    Array.prototype.forEach.call(document.querySelectorAll('.vacpanel'), function (gb) {
      if (prefix.indexOf(gb.id.replace(/_\d+_\d+$/, '')) < 0) return;
      var e = $(gb.id + '_edSV');
      if (!e) return;
      e.value = String(Math.trunc(n));
      try { e.dispatchEvent(new Event('input', { bubbles: true })); e.dispatchEvent(new Event('change', { bubbles: true })); } catch (x) {}
    });
  }
  function onReset() {
    if (!window.confirm('Reset（golden sbResetClick）\n清掉每一格面板的事件；下一個 tick 重新讀值，並照 golden 重寫閥值模式（SDO 02h = 1）\n\n確定？')) return;
    press('vacuum.reset', {}, 'Reset（golden sbResetClick）');
  }

  // capture 階段、掛在 document：比面板自己的 toggle 與頁面內建的 .btnpanel 監聽都先執行，並攔下它們（同 ht9045_io_do.js）
  document.addEventListener('click', function (e) {
    if (e.defaultPrevented || document.body.classList.contains('layoutEdit') || document.body.classList.contains('layoutPlace')) return;
    var t = e.target;
    if (!t || !t.closest) return;
    var gb = panelOf(t);
    var hit = null;
    if (gb) {
      if (t.closest('#' + gb.id + '_bplOn')) hit = 'on';
      else if (t.closest('#' + gb.id + '_bplOff')) hit = 'off';
      else if (t.closest('button') && t.closest('button') === panelSetBtn(gb)) hit = 'set';
    } else {
      var b = t.closest('#btnSetInArm, #btnSetIndexArm, #btnSetOutArm, #sbReset');
      if (b) hit = b.id;
    }
    if (!hit) return;
    e.stopPropagation(); e.preventDefault();
    var el = hit === 'on' ? $(gb.id + '_bplOn') : hit === 'off' ? $(gb.id + '_bplOff') : hit === 'set' ? panelSetBtn(gb) : $(hit);
    if (el && el.__vuLocked) { var w = el.__vuWhy || '這顆鈕目前不能按'; sayNow(w.indexOf('🔒') === 0 ? w : '🔒 ' + w, '#ffcc66'); return; }
    if (hit === 'on' || hit === 'off') onBpl(gb, hit);
    else if (hit === 'set') onSet(gb);
    else if (hit === 'btnSetInArm') onSetAll('in', 'edSetInArm', 'InArm');
    else if (hit === 'btnSetIndexArm') onSetAll('index', 'edSetIndexArm', 'Index Arm1／Arm2');
    else if (hit === 'btnSetOutArm') onSetAll('out', 'edSetOutArm', 'OutArm');
    else if (hit === 'sbReset') onReset();
  }, true);

  /* ---- 面板由 page-widgets.js 在 DOMContentLoaded 建立（它的監聽器先註冊）；本監聽器排在它之後、引擎 attach 之前 ---- */
  function onReady() { lockStatic(WAIT_NOTE); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();

  /* ---- (4) 引擎註冊（這一頁沒有 B 路接線檔；只給小鍵盤） -------------------------------------- */
  var kb = {};
  Object.keys(ARMS).forEach(function (arr) {
    for (var c = 0; c < 8; c++) for (var r = 0; r < 2; r++) kb[arr + '_' + c + '_' + r + '_edSV'] = KB_SV;
  });
  ['edSetInArm', 'edSetIndexArm', 'edSetOutArm'].forEach(function (id) { kb[id] = KB_SV; });   // golden edSetInArmClick（VacuumUnit.cpp:599）
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'vacuumunit_c', fields: {}, optional: {}, kb: kb });
  }
})();
