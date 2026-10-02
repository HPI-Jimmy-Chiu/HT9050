/* ht9045_home_c.js -- HW.home.html（golden TfHome，uhome.dfm／uhome.cpp）的執行期接線。
 * ---------------------------------------------------------------------------
 * AI(W906-HOMEMON) 20261001（手寫，不是產生器產物）
 *
 * 後端：JsonBridge/ChanHome.cpp（wb_serve，tick 執行緒）。下面的 tag 只在「有瀏覽器開著這個視窗（含縮小）」或
 * 「回原點程序把它叫出來（golden fHome->Show，fShow）」時才發布；其他時候整組不在快照裡：
 *   home.rows      JSON 陣列 [{slot, motor, name, x, y, pos, lamp}] ＝ golden 執行期 THomeClass（uhome.cpp:61-104）：
 *                  Visible 的列、vector 順序（golden 排版迴圈 :357-375），name＝labName->Caption（MOT[].NumberAlias），
 *                  x／y＝labName->Left／Top，pos＝edPos->Text（"0"，之後 ShowMotorHomePos 寫的位置），
 *                  lamp＝ShowLed 的 attr（:664-680）：0 灰（FalseColor clSilver）／1 clLime／2 clRed／3 clYellow
 *   home.log       JSON 陣列：ListBox1 最新的行（最新在上；golden 只有 Insert(0, …) 與 Clear()），最多 100 行
 *   home.log.count ListBox1 全部行數
 *   home.resetOk   Panel2（Reset OK ＋ Exit）顯示與否：golden FormShow 藏（:4846）、ProcessMotorHome case 1100 顯示（:3516）
 *   home.fShow／home.fAbort／home.step   TfHome 的 fShow／fAbort／iHomeStep
 *
 * 指令：sbAbortHome → WS act.home.abort ＝ golden TfHome::sbAbortHomeClick（uhome.cpp:4980-4986：
 *   GaliMotorServoOff("sbAbortHomeClick")＝停全部馬達、切馬達電源、抓全部煞車；fAbort=true；Close()）。
 *   * 按下就送，沒有確認框（golden 沒有；EastSun 不准自己加確認視窗）。
 *   * 不拿操作權杖（同 motor.stop：伺服器豁免，停的方向不能被別的分頁的權杖擋住）。
 *   * C++ 只在回原點程序開著這個畫面（fHome->fShow）時才做；否則回 not-open、什麼都不做 ⇒ 頁面把理由顯示出來。
 *   * home.fShow 明確是 false 時把鈕鎖住、title 寫原因；不知道（tag 不在／null）時不鎖，交給 C++ 判斷 —— 停的方向寧可送出去。
 * ⚠ Panel2 的疊放順序照 golden：uhome.dfm 裡 Panel2 寫在 alClient 的 Panel1 前面，VCL 的 z-order＝dfm 順序（後面的在上面），
 *   所以 golden 的 Panel2->Visible=true（:3516）其實被 Panel1 蓋住、畫面上看不到；本頁的 DOM 順序跟 dfm 一樣（Panel2 在 Panel1 前、
 *   都沒有 z-index），所以這裡把它設成 display:block 之後一樣被蓋住 —— 跟 golden 相同，不是壞掉。要讓它浮到上面是一行 CSS
 *   （#Panel2{z-index:1}），那是偏離 golden，要 EastSun 決定。
 * Panel2 的 Exit（SpeedButton1，頁面內建的 .exitbtn）＝關視窗＝golden SpeedButton1Click → Close()：外框關窗 → C++ 頁面表
 *   （WebPageTable.cpp，操作員關掉程序開的 Home Monitor）→ fHome->Close()。本檔不碰它。
 * 沒有 home.rows（舊的 wb_serve、或還沒發布）：Panel1 只顯示一行「等待 C++ 發布」，不顯示假資料。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var T = window.HT9045Tags, R = window.HT9045Recipe, W = window.HTWidgets;
  if (!T || typeof T.subscribe !== 'function' || typeof T.get !== 'function') return;

  var LAMP = { 1: '#00ff00', 2: '#ff0000', 3: '#ffff00' };   // golden clLime / clRed / clYellow（0＝FalseColor clSilver）
  var panel1 = document.getElementById('Panel1');
  var listBox = document.getElementById('ListBox1');
  var panel2 = document.getElementById('Panel2');
  var btn = document.getElementById('sbAbortHome');
  var hdr = document.getElementById('Panel104');
  var host = null, rowEls = [], rowSig = null, lastRows = null, lastLog = null, lastCount = null;
  var btnTitle = btn ? (btn.title || '') : '';
  var retryMs = 2000, nextTry = 0, connecting = false, wasConnected = false;

  function parse(s) {
    if (typeof s !== 'string' || s === '') return null;
    try { return JSON.parse(s); } catch (e) { return null; }
  }

  // ---- 狀態列（Panel104 右側一行；回應講清楚：已執行／被拒＋理由） ----
  var bar = null;
  function say(msg, colour) {
    if (!hdr) { console.info('[HW.home] ' + msg); return; }
    if (!bar) {
      bar = document.createElement('span');
      bar.id = 'homeStatus';
      bar.style.cssText = 'position:absolute;right:8px;bottom:2px;max-width:640px;font-size:11px;color:#fff;' +
        'white-space:nowrap;overflow:hidden;text-overflow:ellipsis;';
      hdr.appendChild(bar);
    }
    bar.textContent = msg;
    bar.title = msg;
    bar.style.color = colour || '#ffffff';
    console.info('[HW.home] ' + msg);
  }

  // ---- 列：golden THomeClass（labName＋ledHome＋edPos）用 HTWidgets.makeHomeRow，位置照 C++ 的 labName->Left／Top ----
  function ensureHost() {
    if (host || !panel1) return host;
    host = document.createElement('div');
    host.id = 'homeRows';
    host.style.cssText = 'position:absolute;left:0;top:0;right:0;bottom:0;';
    panel1.appendChild(host);
    return host;
  }
  function setLamp(led, v) {
    if (!led) return;
    if (typeof v !== 'number') {                                   // 不可知：斜線紋，不畫成「關」
      led.classList.remove('on'); led.classList.add('unknown');
      led.title = 'ledHome：燈號不可知（C++ 沒有給值）';
      return;
    }
    led.classList.remove('unknown');
    if (v && LAMP[v]) { led.style.setProperty('--led-on', LAMP[v]); led.classList.add('on'); }
    else led.classList.remove('on');
    led.title = 'ledHome：ShowLed attr ' + v + (v === 1 ? '（clLime）' : v === 2 ? '（clRed）' : v === 3 ? '（clYellow）' : '（熄，clSilver）');
  }
  function buildRows(rows) {
    ensureHost();
    if (!host) return;
    host.textContent = '';
    rowEls = [];
    rows.forEach(function (r) {
      var el = W && W.makeHomeRow ? W.makeHomeRow({ name: 'ledHome' + r.slot, label: r.name == null ? '---' : String(r.name),
        pos: r.pos == null ? '---' : String(r.pos), on: false, left: (typeof r.x === 'number' ? r.x : 0), top: (typeof r.y === 'number' ? r.y : 0) }) : null;
      if (!el) return;
      var lab = el.firstChild;
      if (lab && lab.style) lab.style.top = '0';                   // golden：labName、ledHome、edPos 同一個 Top（:361-366）
      el.title = (r.name == null ? '' : r.name) + '：THomeClass slot ' + r.slot + '（MOT[' + r.motor + ']）';
      host.appendChild(el);
      rowEls.push({ el: el, led: el.querySelector('.aled'), ed: el.querySelector('input') });
    });
  }
  function renderRows(rows) {
    if (!rows || !rows.length) {
      ensureHost();
      if (host) {
        host.textContent = '';
        var p = document.createElement('div');
        p.style.cssText = 'position:absolute;left:8px;top:8px;font-size:12px;color:#889;';
        p.textContent = rows ? 'HomeClass 沒有 Visible 的列（InitialHomeClass 還沒跑，或這台的機型設定沒有要顯示的馬達）' : '等待 C++ 發布 home.rows（回原點畫面的馬達列）';
        host.appendChild(p);
      }
      rowEls = []; rowSig = null;
      return;
    }
    var sig = rows.map(function (r) { return r.slot + '|' + r.name + '|' + r.x + '|' + r.y; }).join(';');
    if (sig !== rowSig || rowEls.length !== rows.length) { buildRows(rows); rowSig = sig; }
    for (var i = 0; i < rows.length && i < rowEls.length; i++) {
      var r = rows[i], e = rowEls[i];
      var pos = r.pos == null ? '---' : String(r.pos);
      if (e.ed && e.ed.value !== pos) e.ed.value = pos;
      setLamp(e.led, r.lamp);
    }
  }

  // ---- ListBox1（最新在上） ----
  function renderLog(lines, count) {
    if (!listBox) return;
    listBox.textContent = '';
    listBox.style.fontSize = '16px';                               // golden uhome.dfm ListBox1 Font.Height=-16, MS Sans Serif
    listBox.style.fontFamily = '"MS Sans Serif",sans-serif';
    (lines || []).forEach(function (s) {
      var d = document.createElement('div');
      d.style.cssText = 'white-space:pre;padding:0 3px;line-height:19px;';
      d.textContent = String(s);
      listBox.appendChild(d);
    });
    if (lines && typeof count === 'number' && count > lines.length) {
      var more = document.createElement('div');
      more.style.cssText = 'white-space:pre;padding:0 3px;font-size:11px;color:#889;';
      more.textContent = '（另有 ' + (count - lines.length) + ' 行較舊的訊息沒有送到畫面）';
      listBox.appendChild(more);
    }
  }

  function renderButton(fShow) {
    if (!btn) return;
    if (fShow === false) {
      btn.disabled = true;
      btn.title = btnTitle + '｜回原點畫面目前不是回原點程序開的（fHome->fShow=false）：C++ 不會執行 Abort Home';
    } else {
      btn.disabled = false;
      btn.title = btnTitle;
    }
  }

  function push() {
    var st = typeof T.status === 'function' ? T.status() : null;
    if (st) {
      if (st.connected && !wasConnected) { lastRows = null; lastLog = null; lastCount = null; }   // 重連：快照是新的基準
      wasConnected = !!st.connected;
    }
    var rs = T.get('home.rows');
    if (rs !== lastRows) { lastRows = rs; renderRows(parse(rs)); }
    var ls = T.get('home.log'), lc = T.get('home.log.count');
    if (ls !== lastLog || lc !== lastCount) { lastLog = ls; lastCount = lc; renderLog(parse(ls), lc); }
    if (panel2) panel2.style.display = (T.get('home.resetOk') === true) ? 'block' : 'none';
    var fs = T.get('home.fShow');
    renderButton(fs === false ? false : (fs === true ? true : null));
  }

  // ---- Abort Home：按下就送（golden 沒有確認框） ----
  if (btn) {
    btn.style.cursor = 'pointer';
    btn.addEventListener('click', function () {
      if (!R || typeof R.rawCmd !== 'function') { say('Abort Home 送不出去：ht9045_recipe_client.js 沒有載入', '#ffcc66'); return; }
      say('Abort Home 送出中…', '#ffffff');
      R.rawCmd('act.home.abort', { value: JSON.stringify({ source: 'HW.home', button: 'sbAbortHome' }) }).then(function () {
        say('Abort Home：已執行（golden sbAbortHomeClick：停止全部馬達、切馬達電源、抓住煞車、中止回原點、關閉本畫面）', '#ffffff');
      }, function (e) {
        say('Abort Home 被拒：' + (e && e.message ? e.message : String(e)), '#ffcc66');
      });
    });
  }

  function tryConnect() {
    if (typeof T.connect !== 'function' || connecting) return;
    connecting = true;
    T.connect().then(function () { connecting = false; retryMs = 2000; push(); }, function () {
      connecting = false;
      nextTry = Date.now() + retryMs;
      retryMs = Math.min(retryMs * 2, 15000);
    });
  }
  function watchdog() {                                            // HT9045Tags 自己不重連（同 ht9045_mv_trays.js）
    var st = typeof T.status === 'function' ? T.status() : null;
    if (!st || st.connected) return;
    if (wasConnected) { wasConnected = false; push(); }
    if (Date.now() >= nextTry) tryConnect();
  }

  renderRows(null);
  window.HT9045HomeMonitor = { push: push };
  T.subscribe(function (changed) {
    var k = Object.keys(changed || {});
    for (var i = 0; i < k.length; i++) if (k[i].indexOf('home.') === 0) { push(); return; }
  });
  if (typeof T.connect === 'function') {
    tryConnect();
    if (typeof T.status === 'function') window.setInterval(watchdog, 2000);
  } else {
    push();
  }
})();
