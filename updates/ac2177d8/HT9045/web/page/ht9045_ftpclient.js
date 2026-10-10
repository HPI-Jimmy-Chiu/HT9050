/* ht9045_ftpclient.js -- Data.FTPClient.html <-> golden TfFTPClient (906_0625_Steven KYECFTP/FTPClient.cpp / .dfm), card LI-9 F1.
 * ---------------------------------------------------------------------------
 * AI(W906-LI9-F1) 20261002 (St02-E helper) new file (hand-written).  Every piece of logic and every guard runs in C++
 * (KYECFTP/FTPClientForm_St02.cpp, WebLotInfoFtp_St02.cpp); this file draws the state C++ returns and sends the operator's
 * operations.  It never decides whether something is allowed.
 *
 * Command: WS `act.lotInfoFtp.<op>`, value = JSON string.  EVERY reply (also a refusal, which arrives as the ack error text)
 *   is {"executed", "op", "guard", "golden", "detail", "messages":[{s1,s2}], "gated":[], "notes":[], "vclException"?, "state":{...}}.
 *   ops sent from this page:
 *     state                                  on load / when the frame says the window opened
 *     filter {edit:'hd', text}               typing in edtHDWaferName (golden OnChange -> FilterList), 200 ms after the last key
 *     pick {list:'hd', index, name}          lstHDFile double click (golden lstHDFileDblClick)
 *     loadHD                                 plLoad 'Load from HD' (golden plLoadClick)
 *     upload                                 plUnload 'Upload to Server' (golden 913 plUnloadClick; card LI-9 F2-3, AI(W906-W202) 20261009
 *                                            (St02-E)): C++ runs the whole FTP upload before it replies (W-203 = B, as golden)
 *     enterHD                                Enter in edtHDWaferName (golden edtHDWaferNameKeyPress)
 *     filter {edit:'server', text}           typing in edtServerWaferName (POOL-14 MR-A, AI(W906-P14) 20261010 (St02-E); golden 913 :2167)
 *     pick {list:'server', index, name}      lstServerFile double click (golden 913 :2172 lstServerFileDblClick)
 *     copyName / cleanName                   Panel15 'Copy File Name' / Panel16 'Clean File Name' (golden 913 :2311 / :2317)
 *     download / enterServer                 plSLoad 'Download to Handler' / Enter in edtServerWaferName (golden 913 plSLoadClick :862,
 *                                            :2244-2246; POOL-14 MR-C): C++ runs the whole download before it replies
 *     testerType / testerId {index}|{text}   cbTesterType / cbTesterID OnChange (picked from the list = index; typed = text)
 *     inputMethod {index}                    rgInputMethod OnClick
 *     testerName {text}                      edTesterName (sent on change; Save also carries it)
 *     saveTester {edTesterName}              btSafeTasterName 'Save'
 *     exit                                   Button3 'Exit'
 *     memoClear                              memoFTP double click
 *     close                                  the frame closed this window while the dialog was open (= golden's close box)
 *   golden's ShowMyMessage boxes are shown by the wb_serve host (MbWait); this page only repeats them in the status bar.
 * Rules (ht9045_dio_delete.js / ht9045_trayedit.js): only clicks the operator made (isTrusted); one request at a time, a
 *   typed filter waits for the request in flight and only the newest text is sent; busy: (WebCmdGuard) = HT9045Busy.NOTE,
 *   not resent; not-operator = keepAlive (control.acquire) and one resend; a missing command = said once, then nothing sent.
 * The window: C++ opens it (page table "ftpclient", or ht9045_lotinfo_ftp.js posts {open:'ftpclient'}).  When the state says
 *   the dialog closed (Load from HD done, Save with a good name, Exit), this page asks the frame to close it ({closeMe:true}).
 *   When the frame closes it while the state says open, `close` is sent; C++ may keep it open (golden FormCloseQuery,
 *   bCanExit false) -- then the window is opened again.
 * window.HT9045FtpClient: state(), last(), busy(), send(op, payload), stats() -- for probes.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  if (window.HT9045FtpClient) return;

  var PFX = 'act.lotInfoFtp.';
  var LOG = '[FTPClient] ';
  var FILTER_MS = 200;
  var S = null, LAST = null, BUSY = false, OFF = '', queue = [], filterTimer = null, pendingFilter = null;
  var pendingEdit = 'hd';                     // which edit pendingFilter came from: 'hd' or 'server' (POOL-14 MR-A)
  var hosted = !!(window.parent && window.parent !== window);
  var winOpen = null;                         // HT_WIN from background.html (null = never told)
  var st = { sent: 0, busy: 0, errors: 0, renders: 0 };

  function $(id) { return document.getElementById(id); }
  function show(el, on) { if (el) el.hidden = !on; }
  function say(msg, bad) {
    var el = $('sbFTPStatus');
    if (el) { el.textContent = msg || ''; el.className = bad ? 'bad' : ''; }
    if (window.console && msg) console.info(LOG + msg);
  }
  function isBusy(m) { return (window.HT9045Busy && HT9045Busy.is) ? HT9045Busy.is(m) : /^busy:/.test(String(m || '')); }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function refusal(msg) { try { var j = JSON.parse(msg); if (j && typeof j === 'object') return j; } catch (x) {} return null; }

  /* ---- transport ------------------------------------------------------------------------------ */
  function rawSend(op, payload, tries) {
    var R = window.HT9045Recipe;
    if (!R || !R.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd(PFX + op, { value: JSON.stringify(payload || {}) }); }).then(unwrap, function (e) {
      var msg = (e && e.message) || String(e);
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return rawSend(op, payload, tries + 1); }, function () { throw e; });
      }
      throw e;
    });
  }
  function send(op, payload) {
    if (OFF) { say('FTP：伺服器沒有 ' + PFX + '*（' + OFF + '），這一頁不能用', true); return Promise.resolve(null); }
    if (BUSY) { queue.push({ op: op, payload: payload }); return Promise.resolve(null); }
    BUSY = true; st.sent++;
    return rawSend(op, payload, 0).then(function (a) { done(op, a, null); }, function (e) { done(op, null, e); })
      .then(function () { BUSY = false; next(); }, function () { BUSY = false; next(); });
  }
  function next() {
    if (BUSY) return;
    if (pendingFilter !== null && !filterTimer) { var t = pendingFilter; pendingFilter = null; send('filter', { edit: pendingEdit, text: t }); return; }
    var q = queue.shift();
    if (q) send(q.op, q.payload);
  }
  function done(op, a, e) {
    if (e) {
      var msg = (e && e.message) || String(e);
      var j = refusal(msg);
      if (j) { a = j; }
      else {
        st.errors++;
        if (isBusy(msg)) { st.busy++; say((window.HT9045Busy && HT9045Busy.NOTE) || '同一個指令剛送過，這一下略過'); return; }
        if (/unknown cmd|unknown command/i.test(msg)) OFF = msg;
        var hint = /^(control-held|not-operator)$/.test(msg) ? '（操作權在另一個畫面）' : '';
        say('FTP ' + op + ' 沒有送到：' + msg + hint, true);
        return;
      }
    }
    LAST = a;
    if (!a) return;
    if (a.state) render(a.state);
    var parts = [];
    if (a.messages && a.messages.length) a.messages.forEach(function (m) { parts.push(m.s1 + (m.s2 ? '｜' + m.s2 : '')); });
    if (a.vclException) parts.push('VCL：' + a.vclException);
    if (!a.executed && a.guard) parts.push((a.guard === 'not-installed' ? 'C++ 沒有安裝 FTP 對話框（' : '沒有執行（') + a.guard + '）' + (a.detail ? '：' + a.detail : ''));
    if (a.gated && a.gated.length) parts.push('未移植：' + a.gated.join('；'));
    if (parts.length) say(parts.join('\n'), !a.executed || !!a.vclException);
    else if (op !== 'state') say('');
    afterState(op, a);
  }

  /* ---- the window ----------------------------------------------------------------------------- */
  function afterState(op, a) {
    var s = a && a.state;
    if (!s) return;
    if (!s.open && hosted && winOpen === true && op !== 'state') {     // the dialog closed itself (Close in golden): close the window
      try { window.parent.postMessage({ closeMe: true }, '*'); } catch (e) {}
    }
    if (op === 'close' && s.open && hosted) {                          // golden FormCloseQuery kept it open: show it again
      try { window.parent.postMessage({ open: 'ftpclient' }, '*'); } catch (e) {}
    }
  }
  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    var was = winOpen;
    winOpen = !!m.open;
    if (winOpen && was !== true) send('state', {});
    if (!winOpen && was === true && S && S.open) send('close', {});
  });

  /* ---- drawing -------------------------------------------------------------------------------- */
  function clear(el) { while (el && el.firstChild) el.removeChild(el.firstChild); }
  function setBtn(id, c, f2) {
    var b = $(id);
    if (!b || !c) return;
    show(b, !!c.visible);
    b.disabled = !!f2 || !c.enabled;
    if (c.caption) b.textContent = c.caption;
  }
  function setEdit(id, e, keepIfFocused) {
    var el = $(id);
    if (!el || !e) return;
    show(el, e.visible !== false);
    if (!(keepIfFocused && document.activeElement === el && (pendingFilter !== null || filterTimer))) el.value = e.text || '';
  }
  function drawList(id, l, enabled) {
    var box = $(id);
    if (!box || !l) return;
    clear(box);
    (l.items || []).forEach(function (t, i) {
      var d = document.createElement('div');
      d.className = 'it' + (i === l.itemIndex ? ' sel' : '');
      d.setAttribute('data-i', String(i));
      d.textContent = t;
      box.appendChild(d);
    });
    box.setAttribute('aria-disabled', enabled ? 'false' : 'true');
  }
  function drawCombo(id, c) {
    var el = $(id), dl = $('dl_' + id);
    if (!el || !c) return;
    if (document.activeElement !== el) el.value = c.text || '';
    el.disabled = !c.enabled;
    if (dl) {
      clear(dl);
      (c.items || []).forEach(function (t) { var o = document.createElement('option'); o.value = t; dl.appendChild(o); });
    }
  }
  function render(s) {
    S = s; st.renders++;
    var pages = { server: 'TabSheet2', hd: 'TabSheet3', tester: 'TabSheet4' };
    (s.tabs || []).forEach(function (t) { show($('tab_' + t.name), !!s.open && !!t.visible); });
    ['TabSheet2', 'TabSheet3', 'TabSheet4'].forEach(function (id) {
      var p = $(id);
      if (p) p.className = 'ftpPane' + (id === 'TabSheet4' ? ' tester' : '') + (s.open && pages[s.page] === id ? ' on' : '');
    });
    show($('ftpClosed'), !s.open);
    var hd = s.hd || {};
    setEdit('edtHDWaferName', hd.edtHDWaferName, true);
    var e = $('edtHDWaferName');
    if (e) e.readOnly = !!s.barcodeKeys;                                // golden KeyPress swallows keys in barcode mode
    drawList('lstHDFile', hd.lstHDFile, hd.lstHDFile && hd.lstHDFile.enabled);
    setBtn('plLoad', hd.plLoad, false);
    setBtn('plUnload', hd.plUnload, false);                           // F2-3: C++ decides (golden plUnload->Enabled)
    setBtn('plUnloadALL', hd.plUnloadALL, true);
    var pv = !!(hd.labN06_DownloadPath1 && hd.labN06_DownloadPath1.visible);
    ['rowHdPaths', 'rowHdPaths1', 'rowHdPaths2', 'rowHdPaths3'].forEach(function (id) { show($(id), pv); });
    if ($('FTP_DownPath1') && hd.FTP_DownPath1) $('FTP_DownPath1').value = hd.FTP_DownPath1.text || '';
    if ($('FTP_UpLdPath1') && hd.FTP_UpLdPath1) $('FTP_UpLdPath1').value = hd.FTP_UpLdPath1.text || '';
    var sv = s.server || {};                                            // POOL-14 MR-A: the Server page (golden 913 ShowFTPModal case 0)
    setEdit('edtServerWaferName', sv.edtServerWaferName, true);
    var es = $('edtServerWaferName');
    if (es) { es.readOnly = !!s.barcodeKeys; es.disabled = !!(sv.edtServerWaferName && sv.edtServerWaferName.enabled === false); }
    drawList('lstServerFile', sv.lstServerFile, sv.lstServerFile && sv.lstServerFile.enabled);
    setBtn('plSLoad', sv.plSLoad, false);                              // POOL-14 MR-C: C++ decides (golden plSLoad->Enabled)
    setBtn('Panel15', sv.Panel15, false);
    setBtn('Panel16', sv.Panel16, false);
    var t = s.tester || {};
    setEdit('edHandlerType', t.edHandlerType, false);
    setEdit('edHandlerID', t.edHandlerID, false);
    var rg = t.rgInputMethod || {};
    var radios = document.getElementsByName ? document.getElementsByName('rgInputMethod') : [];
    for (var i = 0; i < radios.length; i++) { radios[i].checked = (String(rg.itemIndex) === radios[i].value); radios[i].disabled = rg.enabled === false; }
    show($('grpTesterMap'), !!(t.grpTesterMap && t.grpTesterMap.visible));
    drawCombo('cbTesterType', t.cbTesterType);
    drawCombo('cbTesterID', t.cbTesterID);
    if ($('cbTasterIp') && t.cbTasterIp) $('cbTasterIp').value = t.cbTasterIp.text || '';
    var en = $('edTesterName');
    if (en && t.edTesterName) {
      if (document.activeElement !== en) en.value = t.edTesterName.text || '';
      en.disabled = !(t.grpTesterName && t.grpTesterName.enabled) || !t.edTesterName.enabled;
    }
    setBtn('btSafeTasterName', t.btSafeTasterName, false);
    setBtn('Button3', t.Button3, false);
    var memo = $('memoFTP');
    if (memo) { memo.value = (s.memo || []).join('\n'); memo.scrollTop = memo.scrollHeight || 0; }
  }

  /* ---- events --------------------------------------------------------------------------------- */
  function trusted(ev) { return !ev || ev.isTrusted !== false; }
  function flushFilter() {
    if (filterTimer) { clearTimeout(filterTimer); filterTimer = null; }
    if (pendingFilter !== null) { var t = pendingFilter; pendingFilter = null; send('filter', { edit: pendingEdit, text: t }); }
  }
  function itemOf(ev, box) {
    for (var n = ev && ev.target; n && n !== box; n = n.parentNode) {
      if (n.getAttribute && n.getAttribute('data-i') !== null) return n;
    }
    return null;
  }
  function comboPayload(id) {
    var el = $(id), c = S && S.tester && S.tester[id];
    var v = el ? el.value : '';
    var idx = c && c.items ? c.items.indexOf(v) : -1;
    return idx >= 0 ? { index: idx } : { text: v };
  }
  function hook() {
    var e = $('edtHDWaferName');
    if (e && !e.__w906Li9) {
      e.__w906Li9 = true;
      e.addEventListener('input', function (ev) {
        if (!trusted(ev) || (S && S.barcodeKeys)) return;
        if (pendingFilter !== null && pendingEdit !== 'hd') flushFilter();
        pendingEdit = 'hd';
        pendingFilter = e.value;
        if (filterTimer) clearTimeout(filterTimer);
        filterTimer = setTimeout(function () { filterTimer = null; if (!BUSY) next(); }, FILTER_MS);
      });
      e.addEventListener('keydown', function (ev) {
        if (!trusted(ev) || ev.key !== 'Enter') return;
        if (ev.preventDefault) ev.preventDefault();
        flushFilter();
        send('enterHD', {});
      });
    }
    var l = $('lstHDFile');
    if (l && !l.__w906Li9) {
      l.__w906Li9 = true;
      l.addEventListener('click', function (ev) {                      // VCL: the first click only selects (local highlight)
        var it = itemOf(ev, l);
        if (!it) return;
        for (var c = l.firstChild; c; c = c.nextSibling) c.className = 'it' + (c === it ? ' sel' : '');
      });
      l.addEventListener('dblclick', function (ev) {
        if (!trusted(ev) || l.getAttribute('aria-disabled') === 'true') return;
        var it = itemOf(ev, l);
        if (!it) return;
        flushFilter();
        send('pick', { list: 'hd', index: +it.getAttribute('data-i'), name: it.textContent });
      });
    }
    var es = $('edtServerWaferName');                                  // POOL-14 MR-A: typing = OnChange -> FilterList (golden 913 :2167)
    if (es && !es.__w906Li9) {
      es.__w906Li9 = true;
      es.addEventListener('input', function (ev) {
        if (!trusted(ev) || (S && S.barcodeKeys)) return;
        if (pendingFilter !== null && pendingEdit !== 'server') flushFilter();
        pendingEdit = 'server';
        pendingFilter = es.value;
        if (filterTimer) clearTimeout(filterTimer);
        filterTimer = setTimeout(function () { filterTimer = null; if (!BUSY) next(); }, FILTER_MS);
      });
      es.addEventListener('keydown', function (ev) {                    // POOL-14 MR-C: Enter = golden 913 :2244-2246 plSLoadClick
        if (!trusted(ev) || ev.key !== 'Enter') return;
        if (ev.preventDefault) ev.preventDefault();
        flushFilter();
        send('enterServer', {});
      });
    }
    var ls = $('lstServerFile');
    if (ls && !ls.__w906Li9) {
      ls.__w906Li9 = true;
      ls.addEventListener('click', function (ev) {
        var it = itemOf(ev, ls);
        if (!it) return;
        for (var c = ls.firstChild; c; c = c.nextSibling) c.className = 'it' + (c === it ? ' sel' : '');
      });
      ls.addEventListener('dblclick', function (ev) {                  // golden 913 :2172 lstServerFileDblClick
        if (!trusted(ev) || ls.getAttribute('aria-disabled') === 'true') return;
        var it = itemOf(ev, ls);
        if (!it) return;
        flushFilter();
        send('pick', { list: 'server', index: +it.getAttribute('data-i'), name: it.textContent });
      });
    }
    var bind = function (id, fn) {
      var b = $(id);
      if (!b || b.__w906Li9) return;
      b.__w906Li9 = true;
      b.addEventListener('click', function (ev) { if (!trusted(ev) || b.disabled) return; fn(); });
    };
    bind('plLoad', function () { flushFilter(); send('loadHD', {}); });
    bind('plUnload', function () { flushFilter(); send('upload', {}); });  // F2-3
    bind('Panel15', function () { flushFilter(); send('copyName', {}); });  // POOL-14 MR-A: golden 913 :2311 Panel15Click
    bind('Panel16', function () { flushFilter(); send('cleanName', {}); }); // golden 913 :2317 Panel16Click
    bind('plSLoad', function () { flushFilter(); send('download', {}); });  // POOL-14 MR-C: golden 913 :862 plSLoadClick (C++ runs the whole download)
    bind('btSafeTasterName', function () {
      var en = $('edTesterName');
      send('saveTester', (en && !en.disabled) ? { edTesterName: en.value } : {});
    });
    bind('Button3', function () { send('exit', {}); });
    ['cbTesterType', 'cbTesterID'].forEach(function (id) {
      var c = $(id);
      if (!c || c.__w906Li9) return;
      c.__w906Li9 = true;
      c.addEventListener('change', function (ev) { if (!trusted(ev)) return; send(id === 'cbTesterType' ? 'testerType' : 'testerId', comboPayload(id)); });
    });
    var radios = document.getElementsByName ? document.getElementsByName('rgInputMethod') : [];
    for (var i = 0; i < radios.length; i++) {
      (function (r) {
        if (r.__w906Li9) return;
        r.__w906Li9 = true;
        r.addEventListener('change', function (ev) { if (!trusted(ev) || !r.checked) return; send('inputMethod', { index: +r.value }); });
      })(radios[i]);
    }
    var en = $('edTesterName');
    if (en && !en.__w906Li9) {
      en.__w906Li9 = true;
      en.addEventListener('change', function (ev) { if (!trusted(ev) || en.disabled) return; send('testerName', { text: en.value }); });
    }
    var memo = $('memoFTP');
    if (memo && !memo.__w906Li9) {
      memo.__w906Li9 = true;
      memo.addEventListener('dblclick', function (ev) { if (trusted(ev)) send('memoClear', {}); });
    }
    if (!hosted) send('state', {});                                     // opened on its own (a tab): read once
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045FtpClient = {
    state: function () { return S; }, last: function () { return LAST; }, busy: function () { return BUSY; },
    send: function (op, payload) { return send(op, payload); }, stats: function () { return st; }
  };
})();
