/* ht9045_padinterface_c.js -- HW.PadInterface.html（golden 0618 TfPadInterface，uPadInterface.cpp／.dfm）的頁面接線。
 * ---------------------------------------------------------------------------
 * AI(W906-W155) 20261007 (St02-E)，W-155 第 2 張（MR B）。手寫，不是 gen_wire.py 產物。
 *
 * 後端：PadInterface_St02.cpp 檔尾 W906_PadWire（tools/wb_serve.cpp 把 pad.* 轉過去，跟 vacuum.* 同一個寫法）。
 * golden 這個視窗由 IO 頁的 Pad 鈕開（iosetview.cpp:3882-3885 ShowModal，ControlPanelMode 1 才看得到那顆鈕，:1012）。
 *   視窗顯示（外框 HT_WIN open=true）  → control.takeover＋pad.open＝golden FormShow :159-177（6 個初始化燈號封包、按鈕全部放開、bShow=true）
 *   每秒 pad.get                       → 畫面（燈號＝面板按鍵 mlEvent、按鈕底色＝Down、閃爍勾選、紀錄框）；自己 pad.open 成功時帶 beat:true＝心跳（DEVIATION W155-D3）
 *   面板按鈕                           → pad.button {name}＝golden MouseDown（Down 反轉）＋PadButtonClick（那一面所有按下的鈕組成一個燈號封包）
 *   Send                               → pad.send {text}＝golden ManualSendClick；DEVIATION W155-D2：只收燈號封包 t05{0|1|2}49{0|1}XXXXXX
 *   Pad Led Control Bling              → pad.bling {on}
 *   Exit                               → pad.exit＝golden ExitClick（bShow=false、所有燈號狀態清掉），再關視窗
 *   視窗隱藏（HT_WIN open=false）      → 停止輪詢＋pad.close＝golden FormClose（bShow=false）——DEVIATION W155-D3
 *   Reset Com                          → golden dfm 沒有 OnClick（沒有作用），鎖住並說明
 * 規則：
 *   * 按鈕是用元件名稱（sb_PadInterface_*）送，不是 Alias：dfm 把 Safe Lock 鈕的 Alias 寫成 SwRKManualStep（golden dfm 的錯，golden 程式照元件綁）。
 *   * 畫面上的狀態一律是 C++ 快照，頁面不自己記；按鈕不在本地切換（產生器的 .btnpanel 點擊切換被這裡的 capture 擋掉）。
 *   * 寫 COM 的指令（open／button／send／bling／exit）要操作權杖：先 control.takeover，30 秒沒再用就還（同 IO 頁、真空頁）。
 *   * ControlPanelMode 不是 1：只顯示，全部鎖住（golden 這時打不開這個視窗）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || typeof R.rawCmd !== 'function' || window.__padInterfaceC) return;
  window.__padInterfaceC = true;

  var LAMP_RE = /^t05[012]49[01][0-9A-F]{6}$/;          // DEVIATION W155-D2（C++ 是權威，這裡只是不讓注定被拒的字串先拿權杖）
  var P = { snap: null, open: false, wantOpen: false, timer: null, inflight: false, busy: false, said: '', lastErr: '' };
  var ITEM_IDS = {};                                    // sb_* / ml_* -> item index（從快照來）

  function $(id) { return document.getElementById(id); }
  function unwrap(m) {                                  // 伺服器把 JSON 欄位併進 ack；舊寫法放在 value 字串裡（同真空頁）
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function padCmd(cmd, obj) { return R.rawCmd(cmd, { value: JSON.stringify(obj || {}) }).then(unwrap); }

  /* ---- 狀態列（頁面底部；這一頁沒有接線引擎的狀態列） ---- */
  var bar = null;
  function say(msg, level) {
    if (!bar) {
      bar = document.createElement('div');
      bar.id = 'padStatusBar';
      bar.style.cssText = 'position:fixed;left:0;right:0;bottom:0;max-height:40%;overflow:auto;white-space:pre-wrap;font:12px/1.35 "Microsoft JhengHei",sans-serif;' +
                          'padding:3px 6px;background:rgba(20,24,32,.92);color:#cde;z-index:50;';
      (document.body || document.documentElement).appendChild(bar);
    }
    bar.style.color = level === 'ok' ? '#9f9' : level === 'err' ? '#f88' : level === 'warn' ? '#ffcc66' : '#cde';
    bar.textContent = msg;
    try { console.info('[PadInterface] ' + msg); } catch (e) {}
  }

  /* ---- 鎖 ---- */
  function lock(el, locked, why) {
    if (!el) return;
    if (el.__padAttr == null) {                         // theme.js（release）把 title 搬到 data-htitle：還原時放回原來那個屬性
      el.__padAttr = el.hasAttribute('title') ? 'title' : 'data-htitle';
      el.__padTitle = el.getAttribute(el.__padAttr) || '';
    }
    el.__padLocked = !!locked;
    el.__padWhy = locked ? why : '';
    if (el.tagName === 'BUTTON' || el.tagName === 'INPUT') el.disabled = !!locked;
    el.style.opacity = locked ? '0.55' : '';
    el.style.cursor = locked ? 'not-allowed' : 'pointer';
    if (locked && why) el.setAttribute('title', '🔒 ' + why);
    else if (el.__padAttr === 'title') el.setAttribute('title', el.__padTitle);
    else el.removeAttribute('title');
  }
  function setLed(el, v) {
    if (!el) return;
    var core = el.classList.contains('ledbox') ? el.querySelector('.aled') : el;
    if (core && core.setValue) core.setValue(!!v);
    else if (core) core.classList.toggle('on', !!v);
  }

  /* ---- 畫面＝C++ 快照 ---- */
  function paint(s) {
    if (!s || !s.items) return;
    P.snap = s;
    var why = s.mode !== 1 ? 'ControlPanelMode ' + s.mode + '：這台沒有通訊面板（golden 這時 IO 頁不顯示 Pad 鈕，視窗打不開）'
            : !P.open ? 'Pad 視窗在 wb_serve 那邊沒有開著（pad.open 沒有成功）——關掉再開一次這個視窗' : '';
    s.items.forEach(function (it) {
      ITEM_IDS[it.sb] = it.i;
      var b = $(it.sb);
      if (b) { b.classList.toggle('down', !!it.down); lock(b, !!why, why); }
      setLed($(it.ml), it.led);
    });
    var cbl = $('cb_PadInterface_PadLedBling');
    var cb = cbl ? cbl.querySelector('input') : null;
    if (cb) { cb.checked = !!s.bling; lock(cb, !!why, why); }
    lock($('sb_PadInterface_ManualSend'), !!why, why);
    lock($('btnResetCom'), true, 'Reset Com：golden dfm 沒有 OnClick（這顆鈕在 golden 不做任何事）');
    var memo = $('Memo_PadInterface');
    if (memo && s.memo) {
      var t = s.memo.join('\n');
      if (memo.value !== t) { memo.value = t; memo.scrollTop = memo.scrollHeight; }
    }
    var line = 'Pad：ControlPanelMode ' + s.mode + '・' + (s.com || '?') + (s.rs232Ok ? ' 已開' : ' 沒有開（只記在面板紀錄，沒有送出）') +
               (s.show ? '・視窗開著（bShow）' : '・bShow=false') + (s.requestVer ? '・面板有回版本' : '') +
               (s.droppedT07T08 ? '・丟掉 t07/t08 ' + s.droppedT07T08 + ' 筆' : '') +
               '\n手動送只收燈號封包 t05{0|1|2}49{0|1}XXXXXX；視窗 ' + Math.round((s.showTimeoutMs || 0) / 1000) + ' 秒沒有更新就自動關（bShow）' +
               (why ? '\n🔒 ' + why : '');
    if (line !== P.said) { P.said = line; say(line, why ? 'warn' : 'ok'); }
  }

  /* ---- 輪詢（＝心跳） ---- */
  function poll() {
    if (P.inflight) return;
    P.inflight = true;
    padCmd('pad.get', { beat: !!P.open }).then(function (s) {   // 只有自己 pad.open 成功的視窗才算心跳（St02-M 審查：失敗或鎖住的視窗不能幫死掉的視窗續命）
      P.inflight = false; P.lastErr = '';
      if (P.open && s && s.show === false) {            // 看門狗關過、或別的地方跑了 FormClose
        P.open = false;
        say('Pad 視窗在 wb_serve 那邊已關閉（bShow=false：' + (s.watchdogCloses ? '太久沒有更新，看門狗關的' : '關過') + '）——關掉再開一次這個視窗', 'warn');
      }
      paint(s);
    }, function (e) {
      P.inflight = false;
      var m = String(e && e.message || e);
      if (m !== P.lastErr) { P.lastErr = m; say('⚠ 讀不到 Pad 狀態：' + m, 'err'); }
    });
  }
  function startPolling() { if (!P.timer) P.timer = setInterval(poll, 1000); poll(); }
  function stopPolling() { if (P.timer) { clearInterval(P.timer); P.timer = null; } }

  /* ---- 操作權杖（同 IO 頁 ht9045_io_do.js／真空頁）：用到就接管，30 秒沒再用就還 ---- */
  var releaseTimer = null;
  function scheduleRelease() {
    if (releaseTimer) clearTimeout(releaseTimer);
    releaseTimer = setTimeout(function () {
      releaseTimer = null;
      if (P.busy) { scheduleRelease(); return; }
      var st = (typeof R.status === 'function') ? R.status() : null;
      if (st && st.holdsToken) return;
      R.rawCmd('control.release').then(null, function () {});
    }, 30000);
  }
  function withToken(cmd, obj) {                        // takeover 與指令連著送（同一條 WebSocket，照順序處理）
    var acq = R.rawCmd('control.takeover').then(function () { return null; }, function (e) { return e || new Error('control.takeover failed'); });
    return padCmd(cmd, obj).then(null, function (e) {
      return acq.then(function (ae) {
        var m = String(e && e.message || e);
        if (/no ack within/.test(m)) m = '沒有回應（逾時）——不代表沒有執行，看一下面板燈號與 wb_serve 主控台';
        else if (ae && !/socket closed|no ack within/i.test(String(ae.message || ae))) m += '\n（操作權杖：' + (ae.message || ae) + '）';
        throw new Error(m);
      });
    });
  }

  function openWin() {
    if (P.wantOpen) return;
    P.wantOpen = true;
    P.busy = true;
    withToken('pad.open', {}).then(function (s) {
      P.open = true;
      paint(s);
      say('Pad 視窗開啟（golden FormShow）：送了 ' + ((s.frames || []).length) + ' 行\n' + (s.frames || []).join('\n'), 'ok');
    }, function (e) {
      P.open = false;
      say('❌ pad.open 被拒：' + (e && e.message || e), 'err');
    }).then(function () { P.busy = false; scheduleRelease(); startPolling(); });
  }
  function closeWin() {
    if (!P.wantOpen && !P.timer) return;
    P.wantOpen = false;
    stopPolling();
    if (P.open) padCmd('pad.close', {}).then(null, function () {});
    P.open = false;
  }

  function press(cmd, obj, label) {
    if (P.busy) { say('上一個指令還在路上，請稍候', 'warn'); return; }
    if (!P.open) { say('🔒 ' + label + '：' + (P.snap && P.snap.mode !== 1 ? 'ControlPanelMode 不是 1' : 'Pad 視窗在 wb_serve 那邊沒有開著'), 'warn'); return; }
    P.busy = true;
    withToken(cmd, obj).then(function (s) {
      paint(s);
      var fr = (s && s.frames) || [];
      say(label + '\n' + (fr.length ? fr.join('\n') : '（沒有送出任何東西）') + (s && !s.rs232Ok ? '\n⚠ 面板的 RS-232 沒有開：只記在面板紀錄、沒有送到面板' : ''),
          s && s.rs232Ok ? 'ok' : 'warn');
    }, function (e) {
      say('❌ ' + label + '\n沒有送出：' + (e && e.message || e), 'err');
    }).then(function () { P.busy = false; scheduleRelease(); poll(); });
  }

  /* ---- 點擊（capture：產生器的 .btnpanel 點擊切換不能先跑） ---- */
  document.addEventListener('click', function (e) {
    var t = e.target;
    if (!t || !t.closest) return;
    var bp = t.closest('.btnpanel');
    if (bp && bp.id && bp.id.indexOf('sb_PadInterface_') === 0) {
      e.stopPropagation(); e.preventDefault();
      if (bp.__padLocked) { say('🔒 ' + (bp.__padWhy || '這顆鈕目前不能按'), 'warn'); return; }
      if (!(bp.id in ITEM_IDS)) { say('「' + bp.id + '」不是面板按鈕', 'err'); return; }
      press('pad.button', { name: bp.id }, bp.id.replace('sb_PadInterface_', '') + '（golden PadButtonClick）');
      return;
    }
    var btn = t.closest('button');
    if (btn && btn.id === 'sb_PadInterface_ManualSend') {
      e.stopPropagation(); e.preventDefault();
      var ed = $('ed_PadInterface_ManualSend');
      var text = ed ? String(ed.value).trim() : '';
      if (!LAMP_RE.test(text)) { say('手動送只收燈號封包 t05{0|1|2}49{0|1}XXXXXX（13 個字，X＝0-9 A-F）：「' + text + '」——不送', 'err'); return; }
      press('pad.send', { text: text }, 'Send ' + text + '（golden ManualSendClick）');
      return;
    }
    if (btn && btn.id === 'btnResetCom') { e.stopPropagation(); e.preventDefault(); say('🔒 Reset Com：golden dfm 沒有 OnClick（這顆鈕在 golden 不做任何事）', 'warn'); return; }
    if (btn && btn.id === 'sb_PadInterface_Exit') {     // golden ExitClick；頁面內建的 .exitbtn 監聽接著關視窗（不擋）
      if (P.open) withToken('pad.exit', {}).then(function (s) { P.open = false; paint(s); }, function () {});
      P.open = false;
      return;
    }
  }, true);
  document.addEventListener('change', function (e) {
    var t = e.target;
    var cbl = $('cb_PadInterface_PadLedBling');
    if (!t || !cbl || !cbl.contains(t)) return;
    var want = !!t.checked;
    t.checked = !want;                                  // 畫面等 C++ 回來再改
    press('pad.bling', { on: want }, 'Pad Led Control Bling ' + (want ? '勾選' : '取消') + '（之後的燈號封包 ' + (want ? '閃爍' : '恆亮') + '）');
  }, true);

  /* ---- 外框的視窗狀態（background.html：{type:'HT_WIN', open}） ---- */
  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
    if (m.open) openWin(); else closeWin();
  });
  window.addEventListener('pagehide', function () { closeWin(); });

  function onReady() {
    say('Pad：等外框開啟視窗（HT_WIN）…', 'warn');
    if (window.parent === window) openWin();            // 單獨開這一頁（不是 background 的視窗）時直接開
    else poll();                                        // 先讀一次狀態（pad.get 不用權杖、不送任何東西）
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();

  window.HT9045PadInterface = { _state: P, _poll: poll, _open: openWin, _close: closeWin };   // node 自測用
})();
