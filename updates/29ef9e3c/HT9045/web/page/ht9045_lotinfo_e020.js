/* ht9045_lotinfo_e020.js -- Data.LotInfo.html：Change File（golden btChangeFileClick）與 palSecsGem 滑鼠按下（golden palSecsGemMouseDown）
 * ---------------------------------------------------------------------------
 * //AI(W906-E020-LI11) 20261002 [W906] (St01) todo E-020 LI-11 / LI-13（D:\HT9045\.claude\skills\ht9050-construction\references\todo.md
 *   E-020；Jimmy RULINGS_20261001 第 0 條：照 golden 翻）。golden＝906
 *   D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uLotInfo.cpp：btChangeFileClick [AI(W906-E032) 20261003] :10213-10237（V912 :10394-10418）、palSecsGemMouseDown :10351-10425（V912 :10532-10606）（AI(W906-E030-CITE) 20261003）。
 * 手寫補件（不叫 ht9045_wire_<slug>.js）；可見度（lot.barcode.changeFile.visible）仍由 ht9045_lotinfo_wire.js 照 tag 貼。
 * 網頁不判斷任何條件，守衛與 golden 本體全部在 C++（LotInfo_E020.cpp 的 ht9045::sjson::W906_LotInfoE020Act，
 *   經 JsonBridge/ChanAction.cpp:348 同一行分派）：
 *   act.lotInfo.changeFile           value {}
 *     C++ 先照 golden FormShow :571-572 重算 tsBarCode／btChangeFile 看不看得到（tab-hidden／button-hidden），
 *     再照 golden 選分支；三個分支（Intel 重連、Cognex EtherNet、CCD sub job）的換檔鏈都還沒移植 ⇒ 回
 *     guard gated＋whyNot "GATE (W906-E020-LI11-n): …"，什麼都不做；golden 不走任何分支時回 executed（照 golden 什麼都不做）。
 *     結果寫在 BarCode 分頁的 #barcodeStatus。
 *   act.lotInfo.palSecsGemMouseDown  value {"button":"left|right|middle","seq":n}
 *     golden 的 6 下暗門（左左右右左左；HonPrec 等級、沒在運轉、不是 KYEC_LEE）：第 6 下 Lot ID 空的就 SetLotID("0123456789")
 *     （寫 config.ini [Lot Info]）、Operator ID 空的就填 "12345"。golden 不出任何提示 ⇒ 本檔也不顯示；欄位由 tag lot.id／lot.operator 更新。
 *     每一下都是一個事件（golden 每一下都算）：排隊照順序送、不丟、不套冷卻；seq＝第幾下，讓伺服器 WebCmdGuard
 *     （同指令＋同 value 400 ms 內 ⇒ busy:）不會把「左、左」併成一下。按在面板裡的按鈕上不算（VCL：子控制項自己收事件）；
 *     面板上按右鍵不跳瀏覽器選單。
 * 權杖：每次 control.acquire → act → control.release（同 ht9045_observer_ev.js）。
 * Change File 防連點：伺服器 WebCmdGuard（busy: 不是失敗，HT9045Busy）＋本頁 busy 旗標與 400 ms 冷卻。
 * 舊的 wb_serve（沒有 act.lotInfo.*）⇒ unknown-action ⇒ 狀態列寫「這一版 wb_serve 還沒有」，沒有執行。
 * 測試：ctest E020_LotInfoPage（tools/webprobe/e020_lotinfo_selftest.cjs，node、離線）。
 * window.HT9045LotInfoE020：changeFile()／mouseDown(button)／state()。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var st = { sent: [], last: null, lastError: '', seq: 0, clicks: 0, dropped: 0 };
  var cfBusy = false, cfCoolUntil = 0;
  var queue = [], running = false;

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
  function isBusy(x) { return !!(window.HT9045Busy && HT9045Busy.is(x)) || /^busy:/.test((x && (x.detail || x.message)) || ''); }
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function note() { return window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'; }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function send(op, v) {
    st.sent.push({ cmd: 'act.lotInfo.' + op, value: v });
    if (st.sent.length > 60) st.sent.shift();
    var held = false;
    function release() { return held ? raw('control.release').catch(function () {}) : Promise.resolve(); }
    return raw('control.acquire').then(function () { held = true; }, function () { /* 他人持有：後面的指令自己會回 not-operator */ })
      .then(function () { return raw('act.lotInfo.' + op, { value: JSON.stringify(v) }).then(unwrap, function (e) { return parseErr(e); }); })
      .then(function (r) { return release().then(function () { return r; }); });
  }
  function describe(r) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd|unknown-action/.test(r.detail || '')) return '這一版 wb_serve 還沒有 act.lotInfo.*（JsonBridge/ChanAction.cpp:348）：沒有執行';
    if (r.guard === 'gated') return '沒有執行：' + (r.whyNot || r.gate || 'GATE');
    if (r.guard === 'tab-hidden' || r.guard === 'button-hidden') return '沒有執行：golden 這台機台按不到 Change File（' + (r.detail || r.guard) + '）';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '');
  }
  function say(msg, bad) {
    var s = $('barcodeStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
  }

  // ---- Change File（golden btChangeFileClick :10213-10237（V912 :10394-10418））--------------------------------------------------------------
  function changeFile() {
    if (cfBusy || Date.now() < cfCoolUntil) { say(note()); return Promise.resolve({ executed: false, guard: 'busy' }); }
    cfBusy = true;
    say('Change File 送出中…');
    return send('changeFile', {}).then(function (r) {
      cfBusy = false; cfCoolUntil = Date.now() + coolMs();
      st.last = r;
      if (r && r.executed) { st.lastError = ''; say('Change File：' + (r.result || '完成')); }
      else if (isBusy(r)) { st.lastError = ''; say(note()); }
      else { st.lastError = describe(r); say(st.lastError, true); }
      return r;
    });
  }

  // ---- palSecsGem 滑鼠按下（golden palSecsGemMouseDown :10351-10425（V912 :10532-10606））--------------------------------------------------
  function buttonName(n) { return n === 0 ? 'left' : (n === 2 ? 'right' : (n === 1 ? 'middle' : '')); }
  function onControl(t) { return !!(t && t.closest && t.closest('button,input,select,textarea,a,label')); }
  function pump() {
    if (running || !queue.length) return;
    running = true;
    var item = queue.shift();
    send('palSecsGemMouseDown', item).then(function (r) {
      st.clicks++;
      st.last = r;
      st.lastError = (r && r.executed) ? '' : describe(r);
      if (r && r.filled && window.console && console.info) console.info('palSecsGemMouseDown: Lot ID / Operator ID filled (golden :10597-10601)');
      running = false;
      pump();
    });
  }
  function mouseDown(button) {
    if (button !== 'left' && button !== 'right' && button !== 'middle') { st.dropped++; return; }
    st.seq++;
    queue.push({ button: button, seq: st.seq });
    pump();
  }
  function onPanelMouseDown(ev) {
    if (onControl(ev && ev.target)) return;                                     // 按在子控制項上：那是它自己的事件
    mouseDown(buttonName(ev ? ev.button : -1));
  }
  function onPanelContextMenu(ev) {
    if (onControl(ev && ev.target)) return;
    if (ev && ev.preventDefault) ev.preventDefault();                           // 右鍵是 golden 暗門的一部分，不跳瀏覽器選單
  }

  function start() {
    var b = $('btChangeFile');
    if (b) b.addEventListener('click', function () { if (b.disabled) return; changeFile(); });
    var p = $('palSecsGem');
    if (p) {
      p.addEventListener('mousedown', onPanelMouseDown);
      p.addEventListener('contextmenu', onPanelContextMenu);
    }
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045LotInfoE020 = { changeFile: changeFile, mouseDown: mouseDown, state: function () { return st; } };
})();
