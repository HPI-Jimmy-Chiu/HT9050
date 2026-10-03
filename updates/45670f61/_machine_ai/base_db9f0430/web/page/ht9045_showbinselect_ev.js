/* ht9045_showbinselect_ev.js -- Status.ShowBinSelect.html 的三個操作（golden TfShowBinSelect，V912 cShowBinSelect.cpp）
 * ---------------------------------------------------------------------------
 * //AI(W906-E023-SB1) 20261002 [W906] (St01) todo E-023 SB-1／SB-3／SB-4（D:\HT9045\.claude\skills\ht9050-construction\references\todo.md
 *   E-023；St02 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.10；Jimmy RULINGS_20261001 第 0 條）。
 * 手寫補件（檔名刻意不叫 ht9045_wire_<slug>.js）；畫面與 CLEAR 仍在 ht9045_showbinselect_wire.js。
 *
 * 守衛全部在 C++（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cShowBinSelect_E023.cpp，經 JsonBridge/ChanAction.cpp:346）；頁面只問、只顯示：
 *   SB-1 Index 分頁 Auto Clean（golden btnAutoCleanClick :2248-2315）  act.showBinSelect.autoClean {}
 *        ＝Cleaning 頁「Clean」鈕的同一支 golden（CL-4 的本體）；看得到＝[Enable Auto Clean]（golden FormShow :864，C++ 用 state 回）；
 *        golden 沒有確認框。運轉中也收，照 golden（Steven Q67＝B，1002 08:0x：機台內有料先做 One Cycle，再 Auto Clean；AI(W906-E023-Q67B) St01）。golden 自己的訊息框（要歸零、Tray arm 不在安全位…）
 *        由 wb_serve 的網頁訊息框照 golden 顯示（golden ShowMyMessage），按 OK 之後這一頁才收到回應。
 *   SB-3 UPH 表點兩下（golden UPH_StringGridDblClick :2201-2246）    act.showBinSelect.uphDblClick {row,pageIndex,cells,answer}
 *        不帶 answer 送一次（C++ 跑 golden 的守衛到確認框）→ needConfirm ⇒ window.confirm(golden 兩行字；MB_OKCANCEL)→ "ok"／"cancel"。
 *        pageIndex＝目前分頁照 golden dfm 頁序（tsTestBin 0、tsCategoryInfo 1、Tab_UPH 2、tsUnloadMap 3、tsIndex 4）；
 *        cells＝這一列畫面上的字（tag binsel.uph.grid），C++ 比對，不一樣就 stale-view（中間寫入了新紀錄）。
 *   SB-4 Copy Recipe（golden sbCopyRecipeClick :2994-2999 → RunBatchCopyRecipe main.cpp:35596）  act.showBinSelect.copyRecipe {}
 *        golden 沒有確認框、不看等級與運轉中：把目前配方資料夾 xcopy 到 D:\Run。
 *   state（唯讀）  act.showBinSelect.state {}：Auto Clean 鈕看不看得到（golden FormShow :864）。載入時與點 Index 分頁時各問一次。
 * 權杖：act.* 要 control 權杖。acquire → 指令 → 只有本頁拿的才 release（同 ht9045_showbinselect_wire.js 的 CLEAR）。
 * 防連點：伺服器 WebCmdGuard（busy: 不是失敗，HT9045Busy）＋本頁 busy 旗標與冷卻。
 * 測試：ctest E023_StatusPages（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\e023_status_selftest.cjs，node、離線）。
 * window.HT9045ShowBinSelectEv：autoClean()／uphDblClick(row, opts)／copyRecipe()／refreshState()／state()——opts.answer＝測試用。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var TABS = { testbin: 0, catinfo: 1, uph: 2, bindisp: 3, index: 4 };   // golden cShowBinSelect.dfm 頁序（:98 / :1296 / :1336 / :1361 / :2793）
  var PROMPT = ['Do you want to delete this record?', 'Confirm'];          // golden :2210 MessageBoxA(text, caption, MB_OKCANCEL)
  var busy = false, coolUntil = 0;
  var st = { sent: [], last: null, lastError: '', asked: 0, state: null };

  function $(id) { return document.getElementById(id); }
  function say(id, msg, bad) {
    var el = $(id);
    if (el) { el.textContent = msg || ''; el.style.color = bad ? '#b00' : ''; }
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function isBusy(x) { return !!(window.HT9045Busy && HT9045Busy.is(x)) || /^busy:/.test((x && (x.detail || x.message)) || ''); }
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function clientHolds() { var s = (window.HT9045Recipe && typeof HT9045Recipe.status === 'function') ? HT9045Recipe.status() : null; return !!(s && s.holdsToken); }
  function act(name, v) {
    st.sent.push({ cmd: name, value: v });
    return raw(name, { value: JSON.stringify(v) }).then(unwrap, parseErr);
  }
  // control.acquire → body() → 只有本頁拿到的才 release
  function withToken(body) {
    var had = clientHolds(), took = false;
    return raw('control.acquire').then(function () { if (!had) took = true; }, function () { /* 已持有或他人持有：後面的指令自己會回錯 */ })
      .then(body)
      .then(function (r) {
        var rel = (took && !clientHolds()) ? raw('control.release').then(null, function () {}) : Promise.resolve();
        return rel.then(function () { return r; });
      });
  }
  function describe(r) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd|unknown-action/.test((r.detail || '') + (r.guard || ''))) {
      return '這一版 wb_serve 還沒有 act.showBinSelect.*（JsonBridge/ChanAction.cpp:346）：什麼都沒做';
    }
    if (/not-operator|control-held/.test(r.detail || '')) return '拿不到控制權杖（別的頁面正持有），什麼都沒做';
    if (r.zh) return r.zh + '（' + (r.guard || '?') + '）';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }
  // 一個請求的外框：busy／冷卻、權杖、結果顯示
  function run(statusId, btn, body, doneText) {
    if (busy || Date.now() < coolUntil) return Promise.resolve({ executed: false, guard: 'busy-local' });
    busy = true; if (btn) btn.disabled = true;
    say(statusId, '送出中…');
    return withToken(body).then(function (r) {
      st.last = r;
      if (r && r.executed) { st.lastError = ''; say(statusId, doneText(r)); }
      else if (r && r.cancelled) { st.lastError = ''; say(statusId, '已取消'); }
      else if (isBusy(r)) { st.lastError = ''; say(statusId, window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'); }
      else { st.lastError = describe(r); say(statusId, st.lastError, true); }
      return r;
    }, function (e) {
      var r = parseErr(e); st.last = r; st.lastError = describe(r); say(statusId, st.lastError, true);
      return r;
    }).then(function (r) {
      busy = false; if (btn) btn.disabled = false; coolUntil = Date.now() + coolMs();
      return r;
    });
  }

  // ---- state：Auto Clean 鈕看不看得到（golden FormShow :864 btnAutoClean->Visible=IniConfig.bEnableAutoCleanFunction）----------------
  function applyState(r) {
    var b = $('btnAutoClean');
    if (!b) return;
    if (r && r.executed) {
      st.state = r;
      b.style.display = r.autoCleanVisible === false ? 'none' : '';
      b.title = 'btnAutoClean（golden btnAutoCleanClick cShowBinSelect.cpp:2248-2315；看得到＝[Enable Auto Clean]，FormShow :864）' +
                (r.running ? '：機台運轉中也可以按（照 golden，Steven Q67＝B）——機台內有料會先做 One Cycle，再 Auto Clean' : '');   // AI(W906-E023-Q67B) 20261002 [W906] (St01)
    } else {
      b.style.display = '';                                   // 不可知：照樣顯示，C++ 會再檢查
      b.title = 'btnAutoClean：可見度不可知（C++ 會再檢查）';
    }
  }
  function refreshState() {
    return withToken(function () { return act('act.showBinSelect.state', {}); }).then(function (r) { applyState(r); return r; },
      function (e) { var r = parseErr(e); applyState(r); return r; });
  }

  // ---- SB-1 golden btnAutoCleanClick --------------------------------------------------------------------------------------
  function autoClean() {
    return run('sbsStatus', $('btnAutoClean'), function () { return act('act.showBinSelect.autoClean', {}); }, function (r) {
      if (r.result === 'armed') return r.running ? 'Auto Clean 已登記：機台運轉中，由運轉中的流程去清潔（golden btnAutoCleanClick 設 bRunAutoClean）'   // AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B
                                             : 'Auto Clean 已登記：按 START 之後機台才會去清潔（golden btnAutoCleanClick）';
      if (r.result === 'oneCycle') return '機台內還有料：先做 One Cycle 清料，清完再 Auto Clean（golden InitialAutoCleanAllTask）';
      var m = (r.messages && r.messages.length) ? r.messages[0] : null;
      return m ? ('沒有登記 Auto Clean：' + m.en + (m.zh ? '（' + m.zh + '）' : '') + '（golden 訊息框）')
               : '沒有登記 Auto Clean（已經在清潔或 One Cycle 中、Auto Clean 沒開、或不是手動模式；golden 不提示）';
    });
  }

  // ---- SB-3 golden UPH_StringGridDblClick ----------------------------------------------------------------------------------
  function activeTab() {
    var t = document.querySelector('#PageControl1 > .tab.act');
    var k = t && t.getAttribute ? t.getAttribute('data-tab') : null;
    return (k && TABS.hasOwnProperty(k)) ? TABS[k] : -1;
  }
  function rowCells(row) {
    var g = (window.HT9045Tags && HT9045Tags.has && HT9045Tags.has('binsel.uph.grid')) ? HT9045Tags.get('binsel.uph.grid') : null;
    if (typeof g !== 'string') return null;
    var rows = g.split('\n');
    return row >= 0 && row < rows.length ? rows[row].split('\t') : null;
  }
  function uphDblClick(row, opts) {
    var cells = rowCells(row);
    if (!cells) { say('sbsUphStatus', '還沒讀到 UPH 表（binsel.uph.grid），這一下略過', true); return Promise.resolve({ executed: false, guard: 'no-data' }); }
    var base = { row: row, pageIndex: activeTab(), cells: cells };
    return run('sbsUphStatus', null, function () {
      return act('act.showBinSelect.uphDblClick', Object.assign({}, base, { answer: null })).then(function (r) {
        if (r && r.needConfirm) {
          st.asked++;
          var text = (r.prompt || PROMPT).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          return act('act.showBinSelect.uphDblClick', Object.assign({}, base, { answer: yes ? 'ok' : 'cancel' }));
        }
        return r;
      });
    }, function (r) {
      return r.deleted ? ('第 ' + row + ' 筆 UPH 紀錄已刪除，平均 UPH ' + r.avgUPH + '（golden UPH_StringGridDblClick）') : '沒有刪除（按了 Cancel）';
    });
  }
  function onDblClick(ev) {
    var tr = ev.target && ev.target.closest ? ev.target.closest('tr') : null;
    if (!tr || !tr.hasAttribute || !tr.hasAttribute('data-r')) return;
    uphDblClick(+tr.getAttribute('data-r'));
  }

  // ---- SB-4 golden sbCopyRecipeClick ---------------------------------------------------------------------------------------
  function copyRecipe() {
    return run('sbsStatus', $('sbCopyRecipe'), function () { return act('act.showBinSelect.copyRecipe', {}); }, function (r) {
      var ok = r.rc && r.rc[0] === 0 && r.rc[1] === 0;
      return (ok ? '已把配方「' + r.recipe + '」複製到 ' + r.target : '複製指令回傳 ' + JSON.stringify(r.rc) + '（配方「' + r.recipe + '」→ ' + r.target + '）') +
             '（golden sbCopyRecipeClick → xcopy）';
    });
  }

  function start() {
    var a = $('btnAutoClean'), c = $('sbCopyRecipe'), g = $('UPH_StringGrid');
    if (a) a.addEventListener('click', function () { if (!a.disabled) autoClean(); });
    if (c) c.addEventListener('click', function () { if (!c.disabled) copyRecipe(); });
    if (g) g.addEventListener('dblclick', onDblClick);
    var t = document.querySelector('#PageControl1 > .tab[data-tab="index"]');
    if (t) t.addEventListener('click', function () { refreshState(); });
    refreshState();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ShowBinSelectEv = {
    autoClean: autoClean,
    uphDblClick: uphDblClick,
    copyRecipe: copyRecipe,
    refreshState: refreshState,
    state: function () { return st; }
  };
})();
