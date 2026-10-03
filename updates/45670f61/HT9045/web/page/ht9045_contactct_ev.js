/* ht9045_contactct_ev.js -- Data.ContactCT.html 的三個操作（golden TfContactCT，906 cContactCT.cpp；V912 同一份、行號一樣——AI(W906-E030-CITE) 20261003）
 * ---------------------------------------------------------------------------
 * //AI(W906-E022-CK1) 20261002 [W906] (St01) todo E-022 CK-1／CK-2／CK-3（D:\HT9045\.claude\skills\ht9050-construction\references\todo.md
 *   E-022；St02 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.3；Jimmy RULINGS_20261001 第 0 條）。
 * 手寫補件（檔名刻意不叫 ht9045_wire_<slug>.js）；畫面與每秒重取仍在 ht9045_contactct_wire.js（它的 start() 看到本檔就不綁「尚未接」）。
 *
 * 守衛全部在 C++（cContactCT.cpp 檔尾 ht9045::sjson::W906_ContactCTAct，經 JsonBridge/ChanAction.cpp:349）；頁面只問 YES／NO、顯示結果：
 *   CK-1 Count Clear  （golden btClearCountClick :944-1069）  act.contactCT.clearCount {"answer":null|"yes"|"no"}
 *   CK-2 格子點兩下   （golden sgYieldDblClick :740-886）     act.contactCT.dblClick   {"col","row","yieldType","answer"}
 *   CK-3 Yield Chart  （golden btYieldChartClick :1071-1075） act.contactCT.yieldChart {}
 * 流程（同 ht9045_sortct_wire.js 的 Clear Count）：control.acquire → 不帶 answer 送一次（C++ 跑 golden 的守衛到確認框為止）→
 *   回 needConfirm ⇒ 用 golden 的兩行字問（window.confirm；golden Application->MessageBox(..., MB_YESNO)）→ 確定送 "yes"、取消送 "no"。
 *   ⚠ 取消也要送：golden 按 NO 之後 Count Clear 照樣跑 :1050-1068 的收尾（良率監控計數、LastSet.iIndexCount=0…），C++ 照做。
 *   MB_YESNO 沒有「關掉」：confirm() 只有確定／取消兩種回答。頁面在問的時候被關掉＝什麼都不送（golden 不會有這種情況）。
 *   回應 ok:false 的 JSON 在 ack.error（JsonBridge/ChanAction.h）：refused（guard：running／not-authorized／customer-gated／not-open／
 *   bad-cell／stale-view／no-form…）照印；權限不足時 golden 自己跳 WAR1676（告警框由既有的 modal 通道送來）。
 *   做完 ⇒ HT9045ContactCT.reload(目前的選項)＝golden sgYield->Refresh()（C++ 不觸發 OnClick，只重畫）。
 * CK-3：C++ 設 fObserver->iShowYieldChart=1（:1073）→ 寫 localStorage 'ht9045.observer.yieldChart'（Observer 頁聽 storage 事件、重跑
 *   golden FormShow ⇒ iShowYieldChart==1 ⇒ Yield 分頁）→ postMessage({open:'observer'})（golden fObserver->ShowModal() :1074；開窗守衛在
 *   background.html openWin）。
 * 防連點：伺服器 WebCmdGuard（busy: 不是失敗，HT9045Busy）＋本頁 busy 旗標與 400 ms 冷卻。
 * 測試：ctest E022_ContactCTPage（tools/webprobe/e022_contactct_selftest.cjs，node、離線）。
 * window.HT9045ContactCTEv：clearCount(opts)／dblClick(col,row,opts)／yieldChart()／state()——opts.answer＝測試用（true/false 直接回答確認框）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var PROMPT = ['Do you want to clear the data?', 'Warning!!'];   // golden :1001 / :853 Application->MessageBox(text, caption)
  var KEY_YIELD = 'ht9045.observer.yieldChart';                   // Observer 頁（ht9045_observer_wire.js）聽這個鍵
  var busy = false, coolUntil = 0;
  var st = { sent: [], last: null, lastError: '', asked: 0 };

  function $(id) { return document.getElementById(id); }
  function say(msg, bad) {
    var el = $('ctStatus');
    if (el) { el.textContent = msg || ''; el.style.color = bad ? '#c00' : ''; }
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
  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function act(name, v) {
    st.sent.push({ cmd: name, value: v });
    return cmd(name, { value: JSON.stringify(v) }).then(function (r) { return r; }, function (e) { return parseErr(e); });
  }
  function describe(r) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd/.test(r.detail || '')) return '這一版 wb_serve 還沒有 act.contactCT.*（JsonBridge/ChanAction.cpp:349）：沒有清除任何東西';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }
  function shownIndex() {
    var d = window.HT9045ContactCT && HT9045ContactCT.data && HT9045ContactCT.data();
    return (d && d.rgYieldType && typeof d.rgYieldType.itemIndex === 'number') ? d.rgYieldType.itemIndex : null;
  }
  function repaint() {
    var yt = shownIndex();
    if (window.HT9045ContactCT && HT9045ContactCT.reload && yt !== null) return HT9045ContactCT.reload(yt);
    return null;
  }

  // 兩段：不帶 answer → needConfirm ⇒ 問 → 帶 answer 再送（C++ 守衛重跑）
  function twoStep(name, base, opts, doneText) {
    if (busy || Date.now() < coolUntil) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say('送出中…');
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ })
      .then(function () { return act(name, Object.assign({}, base, { answer: null })); })
      .then(function (r) {
        if (r && r.needConfirm) {
          st.asked++;
          var text = (r.prompt || PROMPT).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          return act(name, Object.assign({}, base, { answer: yes ? 'yes' : 'no' }));   // 取消＝NO，也要送（golden NO 有收尾）
        }
        return r;
      })
      .then(function (r) {
        st.last = r; busy = false; coolUntil = Date.now() + coolMs();
        if (r && r.executed) { st.lastError = ''; say(doneText(r)); repaint(); }
        else if (isBusy(r)) { st.lastError = ''; say(window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'); }
        else { st.lastError = describe(r); say(st.lastError, true); }
        return r;
      }, function (e) {
        busy = false; coolUntil = Date.now() + coolMs();
        var r = parseErr(e); st.last = r; st.lastError = describe(r); say(st.lastError, true);
        return r;
      });
  }

  // CK-1 golden btClearCountClick
  function clearCount(opts) {
    return twoStep('act.contactCT.clearCount', {}, opts, function (r) {
      return r.cleared ? 'Count Clear 完成（golden btClearCountClick：Contact 計數與良率監控計數已清）'
                       : '沒有清 Contact 計數（按了 NO）；golden 照樣做了收尾（良率監控計數、Index Count 歸 0）';
    });
  }
  // CK-2 golden sgYieldDblClick（cell＝golden sgYieldMouseDown＋MouseToCell）
  function dblClick(col, row, opts) {
    var yt = shownIndex();
    if (yt === null) { say('還沒讀到格子（contactct.get），這一下略過', true); return Promise.resolve({ executed: false, guard: 'no-data' }); }
    return twoStep('act.contactCT.dblClick', { col: col, row: row, yieldType: yt }, opts, function (r) {
      if (!r.asked) return '這一項（History）點兩下不清除（golden sgYieldDblClick :846-851）';
      return r.cleared ? ('第 ' + row + ' 列那一站的接觸種類計數已清（golden sgYieldDblClick）') : '沒有清除（按了 NO）';
    });
  }
  function onDblClick(ev) {
    var td = ev.target && ev.target.closest ? ev.target.closest('td') : null;
    if (!td || !td.hasAttribute('data-r')) return;
    dblClick(+td.getAttribute('data-c'), +td.getAttribute('data-r'));
  }
  // CK-3 golden btYieldChartClick
  function yieldChart() {
    if (busy || Date.now() < coolUntil) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say('Yield Chart…');
    return cmd('control.acquire').catch(function () {})
      .then(function () { return act('act.contactCT.yieldChart', {}); })
      .then(function (r) {
        st.last = r; busy = false; coolUntil = Date.now() + coolMs();
        if (r && r.executed) {
          st.lastError = '';
          try { window.localStorage.setItem(KEY_YIELD, String(Date.now())); } catch (e) { /* 沒有 localStorage：Observer 開著時不會切到 Yield 頁 */ }
          try { if (window.parent && window.parent !== window) window.parent.postMessage({ open: r.open || 'observer' }, '*'); } catch (e2) {}
          say('Yield Chart：開 Observer 的 Yield 分頁（golden btYieldChartClick → fObserver->ShowModal）');
        } else if (isBusy(r)) { st.lastError = ''; say(window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'); }
        else { st.lastError = describe(r); say(st.lastError, true); }
        return r;
      });
  }

  function start() {
    var a = $('btClearCount'), b = $('btYieldChart'), g = $('sgYield');
    if (a) a.addEventListener('click', function () { if (!a.disabled) clearCount(); });
    if (b) b.addEventListener('click', function () { if (!b.disabled) yieldChart(); });
    if (g) g.addEventListener('dblclick', onDblClick);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ContactCTEv = {
    clearCount: clearCount,
    dblClick: dblClick,
    yieldChart: yieldChart,
    state: function () { return st; }
  };
})();
