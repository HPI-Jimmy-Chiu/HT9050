/* ht9045_iniconfig_auth_c.js -- Config.Configuration.html「[M01] 監控功能」16 格的重新登入（Q45 甲 #4），C 路頁面補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q45-B5) 20260930 [W906] St01 新檔（手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 * Steven Q45「按照你的建議執行」（子題 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A）、20260929「請按照bcb的邏輯處理」。
 * 設計：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §2.4、§3.2；
 * 派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B5（CC-L4）。
 *
 * golden V912 cConfiguration.cpp:6531-6544 cbM01Click（cbM01、cbM01_01～15 都綁它）→ DoPassword（:6482-6529）：
 *   權限表第 92 項＝0 直接過；沒裝 Real Time CCD 直接過；否則跳主畫面同一個登入框（有密碼本：帳號＋密碼；下拉選單模式：只有密碼，
 *   只比「下拉目前選的那一級」），等級不到第 92 項就把點的那一格改回；**有密碼本時問完一律登出成 Operator**。
 * 網頁（[W906] Q45-2＝A）：golden 每點一格問一次；這裡按存檔時問一次，涵蓋這次改過的全部 M01 格子；錯了全部改回開頁值，其他設定照存。
 * 後端：FileRW/IniConfig.cpp（WS editlist.get／editlist.save tag=IniConfig）。editlist.get 的 extra.auth（WebLogin.cpp W906_ReauthOpenJson）：
 *   points[m01] 的 armed（＝裝了 RTC 且第 92 項≠0）、level、logoutAfter（有密碼本）；c12／a27／n07_5／sgpw 是 armed:false（網頁版不提供，
 *   標示由 ht9045_config_st01_ev.js 做，這裡不重複）。
 * 本檔包 HT9045Recipe.editlistSave（在 ht9045_config_st01_ev.js、ht9045_iniconfig_p26_c.js 之後載入 ⇒ 最外層）：M01 有格子跟開頁值不同
 *   （只算開頁時可改的格子，伺服器也只收這些）且 armed ⇒ 先跳登入小鍵盤（main.html 的 HTQwerty 兩段）→ 答案放進第 4 個參數 extra.reauth
 *   （跟 widgets 並列）→ 送。比對、等級、改回、登出全在 C++；回應的 reauth（不含密碼）轉成狀態列訊息。
 *   按 Abort＝取消＝golden 空白帳密＝錯（Q45-4）：照樣送 {cancelled:true}，C++ 照 golden 把 M01 改回、登入者變 Operator。
 *   ⚠ golden 怪處照翻（Q45-3＝A）：開了 [A01_2]「切到 Operator 後不能存設定」的機台，問完被登出成 Operator，接著 golden FormClose 就不存
 *   （狀態列會出現 C++ 的「[A01_2]目前已切換到Operator權限」訊息）。
 *   密碼：不 console、不存 localStorage／sessionStorage、回覆回來就把這一份清掉；訊息不帶輸入內容。
 *   防連點第二道：小鍵盤開著、存檔在路上時鎖住存檔鈕（btnSave）與 16 格；這段時間再進 editlistSave 一律回 busy。
 *   沒有 extra.auth（舊的 C++）⇒ 不問，C++ 照舊整次拒存（設計 3.1 第 2 條）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'IniConfig';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__iniconfigAuthWrapped) return;
  R.__iniconfigAuthWrapped = true;

  var SAVE_BTN = 'btnSave';                        // 引擎的存檔鈕（ht9045_wire_engine.js 的猜測清單 spbSave/btSave/btnSave…；本頁接線檔沒有 saveBtn）
  var LAST = null;                                 // 最近一次 editlist.get IniConfig 的回應（extra.auth、proxies＝開頁值）
  var BUSY = false, LOCKED = [], BANNER = null;

  function $(id) { return document.getElementById(id); }
  function inputOf(el) { return !el ? null : (el.tagName === 'INPUT' || el.tagName === 'BUTTON') ? el : el.querySelector('input'); }
  function m01Point() {
    var a = LAST && LAST.extra && LAST.extra.auth, pts = (a && a.points) || [];
    for (var i = 0; i < pts.length; i++) if (pts[i] && pts[i].id === 'm01' && pts[i].kind === 'relogin') return pts[i];
    return null;
  }
  function changedM01(p, widgets) {                // 跟開頁值不同、開頁時可改的格子（伺服器 IC_ReauthM01 同一個判斷）
    var out = [], px = (LAST && LAST.proxies) || {};
    (p.controls || []).forEach(function (id) {
      var w = widgets && widgets[id], o = px[id];
      if (!w || typeof w.checked !== 'boolean' || !o || typeof o.checked !== 'boolean') return;
      if (o.editable === false) return;            // 伺服器照通用規則丟掉的格子，golden 點不到也不會問
      if (w.checked !== o.checked) out.push(id);
    });
    return out;
  }
  function lock(ids) {
    unlock();
    ids.forEach(function (id) {
      var x = inputOf($(id));
      if (!x) return;
      LOCKED.push([x, x.disabled]);
      x.disabled = true;
    });
  }
  function unlock() { LOCKED.forEach(function (q) { q[0].disabled = q[1]; }); LOCKED = []; }
  function banner(text) {                          // 小鍵盤上方的說明（小鍵盤本身沒有標題列文字）
    if (!text) { if (BANNER) { BANNER.remove(); BANNER = null; } return; }
    if (!BANNER) {
      BANNER = document.createElement('div');
      BANNER.style.cssText = 'position:fixed;left:50%;top:6px;transform:translateX(-50%);z-index:100000;max-width:92vw;' +
        'background:#fff8d0;color:#000;border:2px solid #c90;border-radius:4px;padding:6px 10px;white-space:pre-wrap;' +
        'font:bold 13px "Microsoft JhengHei",sans-serif;box-shadow:2px 2px 8px rgba(0,0,0,.4);';
      document.body.appendChild(BANNER);
    }
    BANNER.textContent = text;
  }
  function keypad(flags) {                         // 一段 HTQwerty → Promise(字串)；Abort／✕ → null
    return new Promise(function (res, rej) {
      if (typeof HTQwerty === 'undefined') { rej(new Error('登入小鍵盤（qwerty.js）沒有載入：這一次存檔沒有送出')); return; }   // ST01-E 20260930 審查：沒有小鍵盤不能當成按了取消（取消＝golden 空白帳密＝錯，會登出成 Operator）⇒ 整次不送
      var done = false;
      HTQwerty.show(null, flags, { onCommit: function (v) { done = true; res(v); }, onAbort: function () { if (!done) { done = true; res(null); } } });
    });
  }
  function ask(p, cells) {                         // → Promise(reauth 物件)
    var N = (typeof HTQwerty !== 'undefined') ? HTQwerty.N : { NO_SYMBOL: 4, PASSWORD: 8, NO_SPACE: 16 };
    var a = LAST && LAST.extra && LAST.extra.auth, book = !a || a.mode !== 'select', user = null;
    var head = '改了「監控功能」[M01]（' + cells.join('、') + '）要重新登入（golden TfConfiguration::DoPassword，cConfiguration.cpp:6482）：' +
               '等級要到權限表第 ' + p.levelItem + ' 項（目前設 ' + p.level + '）。' +
               (p.logoutAfter ? '\n問完照 golden 一律登出成 Operator。' : '\n下拉選單模式：只比下拉目前那一級的密碼（golden 同）。') +
               '\n按 Abort＝取消＝登出成 Operator（golden 同）：M01 改回開頁值、其他設定照存。';
    banner(head + (book ? '\n① 輸入帳號' : '\n輸入密碼'));
    var first = book ? keypad(N.NO_SYMBOL | N.NO_SPACE) : Promise.resolve('');
    return first.then(function (u) {
      if (u === null) return null;
      user = u;
      banner(head + (book ? '\n② 輸入密碼' : '\n輸入密碼'));
      return keypad(N.NO_SYMBOL | N.NO_SPACE | N.PASSWORD);                     // golden N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD（main.cpp:13215）
    }).then(function (pw) {
      banner(null);
      var ra = (pw === null) ? { point: 'm01', cancelled: true }
             : (book ? { point: 'm01', userId: user, password: pw } : { point: 'm01', password: pw });
      user = null; pw = null;
      return ra;
    });
  }
  function nonStop(code) {                         // golden ShowErrorMessage("WAR1677") → 不停機告警（Steven 20260924，同 main.html）
    var ns = null;
    try { ns = window.HT9045NonStop || (window.parent !== window && window.parent.HT9045NonStop) || null; } catch (e) { ns = null; }
    if (!ns || code !== 'WAR1677') return;
    ns.raise({ code: 'WAR1677', title: 'UserName or PassWord Error', titleZh: '帳號或密碼錯誤',
               message: 'UserName or PassWord Error (Configuration M01 re-login)', messageZh: '帳號或密碼錯誤（Configuration [M01] 重新登入）',
               unitName: 'System',
               description: 'golden: TfConfiguration::DoPassword → TfMain::cbUserSelectChange → ShowErrorMessage("WAR1677")\n網頁版改成不停機告警（Steven 20260924），機台未停機。',
               hint: '要改 [M01] 請重讀頁面、再存一次並輸入正確的帳號密碼。' });
  }
  function report(a) {                             // 回應的 reauth（C++ 不含密碼）→ 引擎的狀態列（session.messages）
    var r = a && a.reauth;
    if (!r) return;
    var s = a.session || (a.session = {}), ms = s.messages || (s.messages = []);
    var lg = r.login || {}, who = (lg.userCaption || lg.levelName || '?') + '（等級 ' + lg.level + '）';
    var zh = '';
    if (r.handled === false) zh = '重新登入：golden 這次要問，但存檔沒有帶帳號密碼 —— M01 改回開頁值（請按重讀再存一次）';
    else if (!r.asked) zh = r.answered ? 'ⓘ 重新登入：這次 golden 不用問（' + r.reason + '），登入沒有變' : '';
    else if (r.passed) zh = '重新登入通過（' + r.reason + '）：M01 照存' + (r.loggedOut ? '；照 golden 已登出成 Operator' : '') + '；目前登入：' + who;
    else zh = '重新登入沒有通過（' + (r.alarm === 'WAR1677' ? '帳號或密碼錯誤 WAR1677；' : '') + r.reason + '）：' +
              ((r.reverted || []).join('、') || 'M01') + ' 照 golden 改回開頁值、其他設定照存' + (r.loggedOut ? '；已登出成 Operator' : '') + '；目前登入：' + who;
    if (zh) ms.push({ en: 'Q45 re-login (golden TfConfiguration::DoPassword): ' + (r.reason || ''), zh: zh });
    if (r.alarm) nonStop(r.alarm);
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) { LAST = d; hideBitBtn1(); return d; });
  };
  function send(self, st, widgets, answers, extra, ra) {
    var ex = extra;
    if (ra) { ex = {}; if (extra) Object.keys(extra).forEach(function (k) { ex[k] = extra[k]; }); ex.reauth = ra; }
    function wipe() { if (ra) { ra.password = ''; ra.userId = ''; ra = null; } if (ex && ex !== extra) ex.reauth = null; }   // recipe_client 在 takeover 之後才 stringify ⇒ 回來才清
    return save0.call(self, st, widgets, answers, ex).then(function (a) { wipe(); report(a); return a; },
                                                           function (e) { wipe(); throw e; });
  }
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (BUSY) return Promise.reject(new Error('busy: 重新登入（Q45）進行中，這一次存檔沒有送出'));
    var p = m01Point(), cells = p && p.armed ? changedM01(p, widgets) : [];
    if (!cells.length) return send(this, st, widgets, answers, extra, null);
    var self = this;
    BUSY = true;
    lock([SAVE_BTN].concat(p.controls || []));
    return ask(p, cells).then(function (ra) { return send(self, st, widgets, answers, extra, ra); })
      .then(function (a) { BUSY = false; unlock(); return a; },
            function (e) { BUSY = false; unlock(); banner(null); throw e; });
  };

  /* ---- CC-E5（派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B5）：golden BitBtn1「Set Vender Password」 ----
   *   golden V912 cConfiguration.dfm:810-818（V899 同）BitBtn1 是 `Visible = False`，OnClick＝BitBtn1Click（cConfiguration.cpp:6052-6060：
   *   fLogin->ShowModal() → 打的字（超過 20 字截成 19）寫進 LastSet.szSupervisor，空的就不動）。V912 全樹沒有任何程式把 BitBtn1 設成看得見
   *   （cConfiguration.cpp 只有 :6052 處理器本身提到它）⇒ golden 操作員點不到這顆鈕。頁面產生器沒有照 DFM 藏它（畫成看得見、點了沒反應），
   *   這裡照 golden 藏起來；C++ 不加寫入路徑（沒有入口可以走到，加了就是死碼）。 */
  function hideBitBtn1() { var b = $('BitBtn1'); if (b) { b.style.display = 'none'; b.setAttribute('data-golden', 'cConfiguration.dfm:817 Visible=False'); } }
  hideBitBtn1();

  window.HT9045IniConfigAuth = {                   // 探針／除錯用（不含任何輸入內容）
    state: function () { var p = m01Point(); return { busy: BUSY, armed: !!(p && p.armed), mode: LAST && LAST.extra && LAST.extra.auth ? LAST.extra.auth.mode : null }; },
    changed: function (widgets) { var p = m01Point(); return p ? changedM01(p, widgets) : []; }
  };
})();
