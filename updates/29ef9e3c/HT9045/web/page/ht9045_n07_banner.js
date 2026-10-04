/* ht9045_n07_banner.js -- main.html 狀態區的 N07「SECS DISCONNECTED」紅框（golden 906 TfMain::Timer2Timer N07 區塊）
 * ---------------------------------------------------------------------------
 * AI(W906-C15-N07-UI) 20261003 (St02-E) 新檔（手寫）。卡 ST02-C15 的畫面那一半（St02-M／筆電 1003 07:1x：畫面歸 Steven 這邊）。
 * golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven（cp950；RULINGS_20261002 #20：只照 906）：
 *   main.cpp:20974-20982  bN07AlarmActive 時：Off_lineDisplay->BorderWidth=20、Color=(FlushFlag)?clRed:clBtnFace；
 *                         labTesterMode->Caption="SECS DISCONNECTED"、Visible=true、Font->Color=clRed；
 *                         palMainStatus 移到 15,14／345x200
 *   main.cpp:20987-20993  解除（下降邊）：palMainStatus 回 3,3／365x215，LoadTestModePicture() 讓 Off_lineDisplay 回原樣
 * 資料：wb_serve 的 tag n07.alarm（WebBridgeTags.cpp:1156，= bN07AlarmActive；[N07] SECS GEM／SECS GEM Alarm 沒開時 null），
 *   推送式（HT9045Tags）。null／false／沒連上 ⇒ 什麼都不畫（狀態區保持原樣）。
 * [W906] 閃爍：golden 用全域 FlushFlag（Timer1 30 ms，main.dfm:17274；main.cpp:3182-3184 每 9 拍＝270 ms 翻一次）。
 *   tag 每秒推一次，拿 FlushFlag 來閃會跟推送頻率疊頻 ⇒ 這一頁自己用 270 ms 的計時器翻，跟 golden 同樣的速度。
 * [W906] 版面：palMainStatus 的移位／縮放是 VCL 的座標細節；網頁的狀態區是排版流，這裡只加框和字，不移動 palMainStatus。
 * 不改 main.html 的任何既有元素：只在 div.statusPane 加一個子元素（#labTesterMode）和框線的 inline style，解除時拿掉。
 */
(function () {
  'use strict';
  var BLINK_MS = 270;                       // golden Timer1 30 ms x 9 拍（main.cpp:3182：ct > 250/30）
  var RED = '#FF0000', BTNFACE = '#F0F0F0'; // clRed / clBtnFace
  var st = { on: false, phase: false, timer: null, pane: null, label: null, saved: null };

  function tag(name) { return (window.HT9045Tags && HT9045Tags.has(name)) ? HT9045Tags.get(name) : null; }
  function pane() {
    if (st.pane) return st.pane;
    st.pane = document.querySelector('div.statusPane');
    return st.pane;
  }
  function paint() {
    var p = pane();
    if (!p) return;
    p.style.borderColor = st.phase ? RED : BTNFACE;
  }
  function tick() { st.phase = !st.phase; paint(); }
  function show() {
    var p = pane();
    if (!p || st.on) return;
    st.on = true;
    st.saved = { borderWidth: p.style.borderWidth || '', borderStyle: p.style.borderStyle || '', borderColor: p.style.borderColor || '' };
    p.style.borderWidth = '20px';
    p.style.borderStyle = 'solid';
    st.phase = true;
    paint();
    var l = document.createElement('div');
    l.id = 'labTesterMode';
    l.className = 'n07Banner';
    l.title = 'labTesterMode（golden main.cpp:20978-20980）';
    l.textContent = 'SECS DISCONNECTED';
    l.style.color = RED;
    l.style.fontWeight = 'bold';
    p.appendChild(l);
    st.label = l;
    st.timer = setInterval(tick, BLINK_MS);
  }
  function hide() {
    if (!st.on) return;
    st.on = false;
    if (st.timer) { clearInterval(st.timer); st.timer = null; }
    var p = pane();
    if (p && st.saved) {
      p.style.borderWidth = st.saved.borderWidth;
      p.style.borderStyle = st.saved.borderStyle;
      p.style.borderColor = st.saved.borderColor;
    }
    if (p && st.label && typeof p.removeChild === 'function') p.removeChild(st.label);
    st.label = null;
    st.saved = null;
  }
  function render() { if (tag('n07.alarm') === true) show(); else hide(); }
  function start() {
    render();
    if (window.HT9045Tags && typeof HT9045Tags.subscribe === 'function')
      HT9045Tags.subscribe(function (changed) { if (changed && Object.prototype.hasOwnProperty.call(changed, 'n07.alarm')) render(); });
    if (window.HT9045Tags && typeof HT9045Tags.connect === 'function') HT9045Tags.connect().catch(function () {});
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
  window.HT9045N07Banner = { render: render, state: function () { return st; }, BLINK_MS: BLINK_MS };
})();
