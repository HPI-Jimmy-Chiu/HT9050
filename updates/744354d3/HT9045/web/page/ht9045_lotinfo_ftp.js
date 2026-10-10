/* ht9045_lotinfo_ftp.js -- Lot Info「FTP」分頁的鈕（golden TfLotInfo::btnFtpServerClick），卡片 LI-9 F1。
 * ---------------------------------------------------------------------------
 * AI(W906-LI9-F1) 20261002 (St02-E helper) 新檔（手寫）。由 Data.LotInfo.html:130-133 的認領行載入（St01 頁）。
 *
 * golden（906_0625_Steven uLotInfo.cpp:5001-5126；uLotInfo.dfm:1803-1949 tsFTP／Panel25）：Server（Tag 0）、HD（Tag 1）、
 *   Tester Name（Tag 2）、DataFTP Save to Data（Tag 3，只有 CC_TSMC_TAINAN 看得到）四顆共用 btnFtpServerClick：權限
 *   （iServerEnable／iHDEnable 對 AccessLevel）、SystemStart、"FTP Form already Opened!!" 都過了才開 KYEC 的 FTP 對話框
 *   （TfFTPClient::ShowFTPModal(Tag)）。所有守衛都在 C++（forms/fLotInfo_Ftp_St02.cpp）；這一頁只送 WS 指令、畫鈕的狀態。
 *
 * 指令：act.lotInfoFtp.open，value＝{"tag":n}；act.lotInfoFtp.state（唯讀）。回覆的 state.lotInfo.buttons[] 帶每顆鈕的
 *   caption／visible／enabled（ckernel.cpp 依機台狀態設 btnFtpServer／btnFtpHD 的 Enabled）。
 *   opened:true ⇒ 請外框開 FTP 視窗（postMessage {open:'ftpclient'}，background.html WINDOWS 表 id 'ftpclient'；
 *   C++ 頁面表 kPgBoth 也會開，兩條都只開同一個視窗）；單獨開這一頁時開新分頁 Data.FTPClient.html。
 *   guard（executed:false）：access-level／system-start／already-open／button-disabled／tab-hidden／spil／tag3-return／
 *   tsmc-ftp-off／s25-customer／server-list-failed（Server＝FTP 伺服器列檔失敗；POOL-14 MR-A 起真的連線列檔）／not-installed。
 *   golden 的訊息框（例 "FTP Form already Opened!!"）由 wb_serve 照 golden 跳；這一頁只在狀態行寫一句。
 * 狀態：FTP 分頁看得到時讀一次、之後每 3 秒讀一次（不搶操作權杖：not-operator 時不重送，下一次再讀）；分頁看不到就不讀。
 * 規則（ht9045_lotinfo_testertcp.js／ht9045_dio_delete.js）：只收操作員真的點的；按鈕一次一個（讀狀態另算）；busy: 不重送。
 * window.HT9045LotInfoFtp：state()、last()、busy()、open(tag)、refresh() —— 探針用。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  if (window.HT9045LotInfoFtp) return;

  var CMD_OPEN = 'act.lotInfoFtp.open', CMD_STATE = 'act.lotInfoFtp.state';
  var POLL_MS = 3000, WATCH_MS = 1000;
  var IDS = ['btnFtpServer', 'btnDataFTPSaveToData', 'btnFtpHD', 'btnFtpTester', 'btnFTPTryConnect'];
  var ALWAYS = { btnFtpServer: true, btnFtpHD: true, btnFtpTester: true };   // dfm Visible（golden FormShow 的客戶專屬隱藏是 S25）
  var S = null, LAST = null, BUSY = false, POLLING = false, OFF = '', lastPoll = 0, wasShown = false;

  function $(id) { return document.getElementById(id); }
  function say(msg, bad) {
    var el = $('lotFtpStatus');
    if (el) { el.textContent = msg || ''; el.style.color = bad ? 'var(--red,#c00)' : ''; }
    if (window.console && msg) console.info('[LotInfo/FTP] ' + msg);
  }
  function isBusy(m) { return (window.HT9045Busy && HT9045Busy.is) ? HT9045Busy.is(m) : /^busy:/.test(String(m || '')); }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function refusal(msg) { try { var j = JSON.parse(msg); if (j && typeof j === 'object') return j; } catch (x) {} return null; }
  function pane() { var b = $('btnFtpServer'); var p = b; while (p && !(p.getAttribute && p.getAttribute('data-pane') === 'ftp')) p = p.parentNode; return p; }
  function paneShown() {
    var p = pane();
    if (!p || document.hidden) return false;
    for (var n = p; n && n.style; n = n.parentNode) if (n.style.display === 'none') return false;
    return true;
  }

  function draw(s) {
    S = s;
    var li = s && s.lotInfo;
    if (!li || !li.buttons) return;
    li.buttons.forEach(function (b) {
      var el = $(b.id);
      if (!el) return;
      if (!ALWAYS[b.id]) el.hidden = !b.visible;
      el.disabled = !b.enabled;
      if (b.caption) el.textContent = b.caption;
    });
    var lb = $('lbFTPStatus');
    if (lb && li.lbFTPStatus) { lb.hidden = !li.lbFTPStatus.visible; if (li.lbFTPStatus.caption) lb.textContent = li.lbFTPStatus.caption; }
  }

  function send(cmd, payload, write, tries) {
    var R = window.HT9045Recipe;
    if (!R || !R.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    var pre = (write && R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd(cmd, { value: JSON.stringify(payload || {}) }); }).then(unwrap, function (e) {
      var msg = (e && e.message) || String(e);
      if (write && msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(cmd, payload, write, tries + 1); }, function () { throw e; });
      }
      throw e;
    });
  }

  var TEXT = {
    'access-level': '權限不夠（IniConfig 的 FTP 等級大於目前登入等級）',
    'system-start': '機台運轉中（SystemStart）不能開',
    'already-open': 'FTP 視窗已經開著（FTP Form already Opened!!）',
    'button-disabled': '這顆鈕現在是灰的（依機台狀態）',
    'tab-hidden': 'FTP 分頁沒有開（config.ini [FTP] Enable FTP）',
    'server-list-failed': 'FTP 伺服器列不出檔案（連不上／斷線／資料夾沒有檔，golden 已跳訊息框）',   // AI(W906-P14) 20261010 (St02-E) POOL-14 MR-A
    'not-installed': 'C++ 沒有安裝 FTP 對話框（wb_serve 開機的安裝行還沒上）'
  };
  function result(a, tag) {
    LAST = a;
    if (a && a.state) draw(a.state);
    if (a && a.executed && a.opened) {
      say('');
      if (window.parent && window.parent !== window) window.parent.postMessage({ open: 'ftpclient' }, '*');   // background.html id 'ftpclient'
      else window.open('Data.FTPClient.html', '_blank');                                                    // 單獨開這一頁時
      return;
    }
    if (!a) return;
    var g = a.guard || '';
    say((TEXT[g] || ('沒有開（' + (g || '?') + '）')) + (a.detail && !TEXT[g] ? '：' + a.detail : ''), true);
    if (window.console) console.info('[LotInfo/FTP] tag ' + tag + ' -> ' + JSON.stringify({ guard: g, golden: a.golden, detail: a.detail, gated: a.gated }));
  }
  function open(tag) {
    if (BUSY) return;
    if (OFF) { say('伺服器沒有 act.lotInfoFtp（' + OFF + '），這幾顆鈕不能用', true); return; }
    BUSY = true;
    send(CMD_OPEN, { tag: tag }, true, 0).then(function (a) { result(a, tag); }, function (e) {
      var msg = (e && e.message) || String(e);
      var j = refusal(msg);
      if (j) { result(j, tag); return; }
      if (isBusy(msg)) { say((window.HT9045Busy && HT9045Busy.NOTE) || '同一個指令剛送過，這一下略過'); return; }
      if (/unknown cmd|unknown command/i.test(msg)) OFF = msg;
      say('沒有送到（' + CMD_OPEN + '）：' + msg, true);
    }).then(function () { BUSY = false; }, function () { BUSY = false; });
  }
  function refresh() {
    if (POLLING || OFF) return;
    POLLING = true; lastPoll = Date.now();
    send(CMD_STATE, {}, false, 0).then(function (a) { LAST = a; if (a && a.state) draw(a.state); }, function (e) {
      var msg = (e && e.message) || String(e);
      var j = refusal(msg);
      if (j && j.state) { draw(j.state); if (j.guard === 'not-installed') say(TEXT['not-installed'], true); return; }
      if (/unknown cmd|unknown command/i.test(msg)) OFF = msg;
    }).then(function () { POLLING = false; }, function () { POLLING = false; });
  }
  function watch() {
    var shown = paneShown();
    if (shown && (!wasShown || Date.now() - lastPoll >= POLL_MS)) refresh();
    wasShown = shown;
  }

  function hook() {
    IDS.slice(0, 4).forEach(function (id) {
      var b = $(id);
      if (!b || b.__w906Li9) return;
      b.__w906Li9 = true;
      b.addEventListener('click', function (ev) {
        if (ev && ev.isTrusted === false) return;
        if (b.disabled) return;
        open(+b.getAttribute('data-tag'));
      });
    });
    setInterval(watch, WATCH_MS);
    watch();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045LotInfoFtp = {
    state: function () { return S; }, last: function () { return LAST; }, busy: function () { return BUSY; },
    open: function (tag) { open(tag); }, refresh: function () { refresh(); }
  };
})();
