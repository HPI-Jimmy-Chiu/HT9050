/* ht9045_counterclear_wire.js -- Data.CounterClear.html <-> fCounterClear（golden V912 cCounterClear.cpp TfCounterClear）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * 後端：tools/wb_serve.cpp 的 counterclear.* 分派 → JsonBridge/ChanAction.cpp 檔尾（W906_CounterClear*），
 *       呼叫的是移植樹裡已逐行翻譯的 fCounterClear 實例（cCounterClear.cpp）。網頁只是畫面，
 *       勾選狀態、權限、清除邏輯全部在 C++。
 *
 *   開頁      WS counterclear.get                         = golden FormShow（:92-114）：GetCountClrAuth()
 *                                                           → 8 個清除框 Enabled=authCounterClr[i]，未授權的取消勾選
 *   點勾選框  WS counterclear.click tag=<id> {checked}    = golden OnMouseUp：cbSelectAll → cbSelectAllMouseUp（:50-77，
 *                                                           只勾／取消有授權的框，Caption 換成 UnSelect All／Select All）；
 *                                                           其他 8 個 → cbAlarmDataMouseUp（:79-88，取消任一框會把 Select All 取消）
 *   Execute   WS counterclear.exe {checked, dryRun}       = golden spbExeClick（:399-455）：勾幾個清幾類，依 golden 順序，
 *                                                           前後只有一對 Clarn_Data(10)。golden 沒有確認框，本頁也不問。
 *
 * 伺服器回應一律是整份畫面狀態（widgets{id:{enabled,checked,caption}}、auth[8]、counters），本檔照著套，不自己推論。
 * 互鎖不寫在這裡：未授權／停用的框由 C++ 拒絕（not-enabled／not-authorized）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var IDS = ['cbAlarmData', 'cbTestCategory', 'cbScanner', 'cbLoadingCount', 'cbContactCountCurr',
             'cbContactCountHis', 'cbSortingCount', 'cbTimeData', 'cbSelectAll'];
  var state = null, busy = false, lastExe = null, lastError = '', loads = 0;
  // AI(W906-CMDGUARD-UI) 20260926：Execute（spbExe）按下後停用，直到 ack 回來再冷卻 coolMs()（400 ms）才恢復。
  //   原本 busy 只擋到 ack，雙擊的第二下多半在 ack 之後才到 → golden spbExeClick 跑兩次（Clarn_Data(10) 兩對、QtyLog 多一筆）。
  var exeHold = false;
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function isBusyReply(e) { return !!(window.HT9045Busy && HT9045Busy.is(e)); }   // ht9045_busy_util.js

  function $(id) { return document.getElementById(id); }
  function box(id) { var el = $(id); return el ? el.querySelector('input[type="checkbox"]') : null; }

  function statusLine() {
    var s = $('ccStatus');
    if (!s) {
      s = document.createElement('div');
      s.id = 'ccStatus';
      // AI(W906-NOOVERLAP) 20260930: the status line was appended under the 559px form, i.e. below the window (background.html counterclear h:585),
      //   where nobody could read it. It now sits inside gbItems, in the free strip between cbSelectAll (golden T=396 H=17) and spbExe (T=441).
      var host = document.querySelector('#gbItems > .cli');
      if (host) {
        s.style.cssText = 'position:absolute;left:2px;top:415px;width:203px;height:24px;overflow:hidden;box-sizing:border-box;padding:0 4px;' +
                          'font-size:10px;line-height:12px;color:#234;white-space:pre-wrap;';
        host.appendChild(s);
      } else {
        s.style.cssText = 'font-size:11px;padding:2px 6px;color:#234;white-space:pre-wrap;width:258px;';
        document.body.appendChild(s);
      }
    }
    return s;
  }
  function say(msg, bad) {
    var s = statusLine();
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '#234';
  }

  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function errText(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && j.guard) return j.guard + (j.detail ? '：' + j.detail : ''); } catch (x) {}
    return t;
  }

  function setCaption(label, text) {
    for (var n = label.firstChild; n; n = n.nextSibling) {
      if (n.nodeType === 3) { n.nodeValue = text; return; }
    }
    label.appendChild(document.createTextNode(text));
  }

  function apply(s) {
    if (!s || !s.widgets) return;
    state = s;
    IDS.forEach(function (id) {
      var w = s.widgets[id], el = $(id), cb = box(id);
      if (!w || !el || !cb) return;
      cb.checked = !!w.checked;
      cb.disabled = !w.enabled;
      el.style.color = w.enabled ? '' : 'var(--text-dim,#889)';
      if (w.enabled) el.removeAttribute('aria-disabled'); else el.setAttribute('aria-disabled', 'true');
      if (w.caption) setCaption(el, w.caption);
    });
    var exe = $('spbExe');
    if (exe && s.widgets.spbExe) exe.disabled = exeHold || !s.widgets.spbExe.enabled;   // AI(W906-CMDGUARD-UI) 20260926：冷卻中維持停用
  }
  // AI(W906-CMDGUARD-UI) 20260926：Execute 的停用／恢復（恢復時照伺服器最近一次的 spbExe.enabled）
  function holdExe() { exeHold = true; var exe = $('spbExe'); if (exe) exe.disabled = true; }
  function releaseExeLater() {
    setTimeout(function () {
      exeHold = false;
      var exe = $('spbExe');
      if (!exe) return;
      var w = state && state.widgets && state.widgets.spbExe;
      exe.disabled = w ? !w.enabled : false;
    }, coolMs());
  }

  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function acquire() {
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ });
  }

  function load() {
    return acquire().then(function () { return cmd('counterclear.get'); }).then(function (s) {
      apply(s); loads++; lastError = '';
      say('已讀取 C++ fCounterClear（golden FormShow）');
      return s;
    }).catch(function (e) {
      lastError = errText(e);
      say('讀取失敗：' + lastError, true);
      throw e;
    });
  }

  function click(id) {
    var cb = box(id);
    if (!cb || busy) return;
    var want = cb.checked;
    busy = true;
    cmd('counterclear.click', { tag: id, value: JSON.stringify({ checked: want }) }).then(function (s) {
      apply(s); lastError = '';
    }).catch(function (e) {
      if (isBusyReply(e)) { if (state) apply(state); return; }   // AI(W906-CMDGUARD-UI) 20260926：busy: 不是失敗 → 不顯示，回伺服器狀態
      lastError = errText(e);
      say('勾選被拒：' + lastError, true);
      if (state) apply(state);          // 回到伺服器的狀態
    }).then(function () { busy = false; });
  }

  function checkedMap() {
    var m = {};
    IDS.forEach(function (id) {
      if (id === 'cbSelectAll') return;
      var cb = box(id);
      if (cb) m[id] = !!cb.checked;
    });
    return m;
  }

  function execute(opts) {
    if (busy || exeHold) return Promise.reject(new Error('busy'));   // AI(W906-CMDGUARD-UI) 20260926：含 ack 後的冷卻
    busy = true;
    holdExe();                                                       // AI(W906-CMDGUARD-UI) 20260926：按下就停用 spbExe
    var dry = !!(opts && opts.dryRun);
    var v = { checked: checkedMap(), dryRun: dry };
    say(dry ? '預覽中…' : '清除中…');
    return cmd('counterclear.exe', { value: JSON.stringify(v) }).then(function (r) {
      lastExe = r; lastError = '';
      apply(r);
      say(dry ? ('預覽（未執行）：\n' + (r.would || []).join('\n')) : 'Counter Clear has been executed!!');
      return r;
    }).catch(function (e) {
      if (isBusyReply(e)) { say(HT9045Busy.NOTE); throw e; }       // AI(W906-CMDGUARD-UI) 20260926：busy: 不是失敗（換掉「清除中…」，一般顏色）
      lastError = errText(e); lastExe = null;
      say('清除被拒：' + lastError, true);
      throw e;
    }).then(function (r) { busy = false; releaseExeLater(); return r; },               // AI(W906-CMDGUARD-UI) 20260926：ack 後冷卻再恢復
            function (e) { busy = false; releaseExeLater(); throw e; });
  }

  function bind() {
    IDS.forEach(function (id) {
      var cb = box(id);
      if (cb) cb.addEventListener('click', function () { click(id); });
    });
    var exe = $('spbExe');
    if (exe) {
      exe.style.cursor = 'pointer';
      exe.addEventListener('click', function () { execute().catch(function () {}); });
    }
  }

  window.HT9045CounterClear = {
    reload: load,
    execute: execute,
    state: function () { return state; },
    lastExe: function () { return lastExe; },
    lastError: function () { return lastError; },
    loads: function () { return loads; },
    busy: function () { return busy || exeHold; }   // AI(W906-CMDGUARD-UI) 20260926：含 Execute 冷卻（探針等 !busy()）
  };

  function start() { bind(); load().catch(function () {}); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
