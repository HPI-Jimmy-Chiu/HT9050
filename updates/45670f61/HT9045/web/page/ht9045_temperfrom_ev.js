/* ht9045_temperfrom_ev.js -- Status.TemperFrom.html 的隱藏入口：palLed 三顆燈（golden TfTemperFrom::Panel73／72／71MouseDown）
 * ---------------------------------------------------------------------------
 * //AI(W906-E023-TP2) 20261002 [W906] (St01) todo E-023 TP-2（D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-023；
 *   St02 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.14；Jimmy RULINGS_20261001 第 0、40 條）。
 * golden 906 cTemperFrom.cpp:1692-1769（V912 :1702-1779；AI(W906-E030-CITE) 20261003：下面裸的 :N 是 906、（V912 :N）是 V912；dfm 兩邊相同）（dfm palLed :21：Panel73 黃 :29、Panel72 綠 :45、Panel71 紅 :61）。
 * 每一下 mousedown 都送 C++：act.temperFrom.mouseDown {"panel":71|72|73,"button":"left"|"right"|"middle"}；順序（左鍵黃 → 左鍵綠 →
 *   非左鍵紅）、等級（HonPrec 以上）、運轉中、ASE／KYEC 例外全部在 C++ 判（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTemperFrom_E023.cpp）。
 *   C++ 回 needConfirm ⇒ window.confirm("Reset the hardware apparatus information?")（golden MB_YESNO）；needPassword ⇒ 小鍵盤
 *   （qwerty.js，N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD，同 golden fQwertyKey）輸入密碼 ⇒ 第二次請求 {answer, password}，C++ 比對密碼。
 *   ⚠ 密碼不在本頁、不記在 state()、不印主控台（RULINGS_20261001 第 40 條：程式裡的密碼是測試值，照 golden 放 C++）。
 *   回 opened ⇒ window.parent.postMessage({open:'handlersys'})（golden HandlerSystem->ShowModal()；HW.HandlerSys.html 開頁時 C++ 再查）。
 *   golden 在每一道檢查不過時都是「什麼都不做」：本頁也什麼都不顯示（只有 console.info）。
 * 點擊照順序一個一個送（promise 串起來）：三下要照 golden 的先後到 C++。
 * 權杖：act.* 要 control 權杖。acquire → 指令 → 只有本頁拿的才 release。
 * 測試：ctest E023_StatusPages（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\e023_status_selftest.cjs，node、離線）。
 * window.HT9045TemperFromEv：mouseDown(panel, button, opts)／state()——opts.answer／opts.password＝測試用。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var PROMPT = 'Reset the hardware apparatus information?';   // golden :1752（V912 :1762） Application->MessageBox(text, NULL, MB_YESNO | MB_TOPMOST)
  var BUTTONS = ['left', 'middle', 'right'];                   // DOM MouseEvent.button 0／1／2 ＝ golden mbLeft／mbMiddle／mbRight
  var chain = Promise.resolve();
  var st = { sent: [], last: null, opened: 0, asked: 0 };

  function $(id) { return document.getElementById(id); }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function clientHolds() { var s = (window.HT9045Recipe && typeof HT9045Recipe.status === 'function') ? HT9045Recipe.status() : null; return !!(s && s.holdsToken); }
  function send(v) {
    var shown = Object.assign({}, v);
    if ('password' in shown) shown.password = '***';             // 密碼不留在 state()
    st.sent.push(shown);
    return raw('act.temperFrom.mouseDown', { value: JSON.stringify(v) }).then(unwrap, parseErr);
  }
  function withToken(body) {
    var had = clientHolds(), took = false;
    return raw('control.acquire').then(function () { if (!had) took = true; }, function () {})
      .then(body)
      .then(function (r) {
        var rel = (took && !clientHolds()) ? raw('control.release').then(null, function () {}) : Promise.resolve();
        return rel.then(function () { return r; });
      });
  }
  // golden fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD)；Abort＝空字串（golden :1759（V912 :1769） 先 Clear()）
  function askPassword(opts) {
    if (opts && typeof opts.password === 'string') return Promise.resolve(opts.password);
    return new Promise(function (resolve) {
      if (window.HTQwerty && HTQwerty.show) {
        var N = HTQwerty.N;
        HTQwerty.show(null, N.NO_SYMBOL | N.NO_SPACE | N.PASSWORD, { onCommit: function (v) { resolve(String(v || '')); }, onAbort: function () { resolve(''); } });
      } else {
        var v = window.prompt('Password', '');
        resolve(v === null ? '' : String(v));
      }
    });
  }
  function opened(r) {
    st.opened++;
    try { if (window.parent && window.parent !== window) window.parent.postMessage({ open: r.open || 'handlersys' }, '*'); } catch (e) {}
  }

  function mouseDown(panel, button, opts) {
    var p = chain.then(function () {
      return withToken(function () {
        return send({ panel: panel, button: button }).then(function (r) {
          if (r && r.needConfirm) {
            st.asked++;
            var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm((r.prompt && r.prompt[0]) || PROMPT);
            if (!yes) return send({ panel: panel, button: button, answer: 'no' });
            if (!r.needPassword) return send({ panel: panel, button: button, answer: 'yes' });
            return askPassword(opts).then(function (pw) { return send({ panel: panel, button: button, answer: 'yes', password: pw }); });
          }
          return r;
        });
      });
    }).then(function (r) {
      st.last = r;
      if (r && r.executed && r.opened) opened(r);
      else if (window.console && console.info) console.info('Panel' + panel + ' ' + button + ' -> ' + ((r && (r.returned || r.guard)) || 'ok'));
      return r;
    }, function (e) { var r = parseErr(e); st.last = r; return r; });
    chain = p.then(null, function () {});
    return p;
  }

  function bind(id, panel) {
    var el = $(id);
    if (!el) return;
    el.addEventListener('mousedown', function (ev) {
      var b = BUTTONS[(ev && typeof ev.button === 'number') ? ev.button : 0] || 'left';
      if (ev && ev.preventDefault) ev.preventDefault();
      mouseDown(panel, b);
    });
    el.addEventListener('contextmenu', function (ev) { if (ev && ev.preventDefault) ev.preventDefault(); });   // 右鍵＝golden mbRight，不跳瀏覽器選單
  }
  function start() { bind('Panel73', 73); bind('Panel72', 72); bind('Panel71', 71); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045TemperFromEv = { mouseDown: mouseDown, state: function () { return st; } };
})();
