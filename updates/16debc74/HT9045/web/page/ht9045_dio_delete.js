/* ht9045_dio_delete.js -- DIO Interface Configuration（golden TfDIOFrom）的 Delete 鈕 spbDelete。
 * ---------------------------------------------------------------------------
 * AI(W906-DIO-DEL) 20261001 (St02-E helper) 新檔（手寫）。census C1-006／C3-004：兩個 DIO 頁的 Delete 鈕原本只有外觀、沒有接線
 *   （C++ 那一半 RULINGS_20260926 S101 已做好）。兩頁都載入，各在接線資料檔那一行的後面同一行載入（不動行號）：
 *     Config.DIOInterFaceCFG.html  background.html 的 dioform 視窗（C 路 TTLCfg，開窗＝editlist.get＝golden FormShow）
 *     Setup.DIOInterFaceCFG.html   舊頁（B 路；沒有 editlist.get TTLCfg，伺服器的 page-not-open 守衛會說要先開 DIO 視窗）
 *   引擎 ht9045_wire_engine.js、兩支接線資料檔（產生器的產物）都不改。
 *
 * golden（906_0625_Steven DIOInterFaceCFG.cpp:248-256；V912 :249-257）逐行：
 *     OpenDialog1->Title="Select file to delete";
 *     if(OpenDialog1->Execute()) { DeleteFile(OpenDialog1->FileName); }
 *     spbDelete->Down=false;
 *   OpenDialog1（DIOInterFaceCFG.dfm:789-795）：Filter '*.ini|*.ini'，InitialDir＝DIOCFGPath（FormShow :26 每次開窗再設一次）。
 *   沒有確認框（對話框選了檔按 Open 就是確認）、沒有權限檢查（整個表單由 906_0625_Steven main.cpp:27739 sbDioSetClick 的 fSecurity->Insufficient(31) 擋）、
 *   刪完不重讀任何清單、不碰 TTLCfg（刪的若是使用中的 DIO 檔，要等之後 LoadData 才發現）—— 本檔照做：刪完只顯示結果。
 *
 * C++ 那一半：WS ttlcfg.op（tools/wb_serve.cpp 分派，呼叫端持 FormLock；本體 FileRW/TTLCfg.cpp FileRW_TTLCfg_DeleteOp，
 *   value／回覆／守衛都寫在該函式上方）。value＝JSON 字串：
 *     按 Delete      {"op":"list"}                       回 {executed:true, title, dir, filter, inUse, files[]}，不動任何東西 ＝ 開對話框
 *     選檔按 Open    {"op":"delete","file":"<名>.ini"}    回 {executed（檔真的不見了）, file（完整路徑）, wasInUse, deleted[], filesAfter[]}
 *     Cancel／✕／Esc  什麼都不送（golden Execute() 回 false，只剩 spbDelete->Down=false）
 *   伺服器每一次都重查守衛，擋下時 ack ok:false，error 是同形狀的 JSON（executed:false, guard, goldenLine, detail）：
 *     page-not-open（這次開機還沒開過 DIO 視窗、或登入等級變了）、not-authorized（Insufficient(31)）、button-disabled、
 *     bad-file／not-in-list（只收 DIOCFGPath 底下現有的 *.ini；golden 的對話框可以換資料夾，網頁版不開放）。
 *
 * 規則（照 ht9045_config_trayplate.js 的寫法）：只收使用者真的點的（isTrusted）；一次一個、等回覆再收下一下；
 *   busy:（WebCmdGuard）等 450 ms 重送、最多 3 次；not-operator 先續權杖再送一次；
 *   page-not-open 在 C 路頁 ⇒ HT9045Page.load()（引擎重送 editlist.get＝golden FormShow），舊頁只說原因；
 *   伺服器沒有這個指令（unknown cmd）⇒ 說明、之後不送。外框說視窗關了（HT_WIN open:false）⇒ 對話框當作 Cancel 收掉。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  if (window.HT9045DioDelete) return;

  var CMD = 'ttlcfg.op';
  var LOG = '[DIO/Delete] ';
  var GOLDEN = 'golden spbDeleteClick（906_0625_Steven DIOInterFaceCFG.cpp:248-256）';
  var BUSY = false, OFF = '', DLG = null, PICK = '', LAST = null;

  function $(id) { return document.getElementById(id); }
  function say(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info(LOG + msg);
  }
  function unwrap(m) {                       // 伺服器把回傳欄位併進 ack；舊寫法放在 value 字串裡 —— 兩種都收
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function refusal(msg) {                    // 守衛擋下時 error 是 W906_DelRefuse 的 JSON；busy:／not-operator／連線錯誤是純文字
    try { var j = JSON.parse(msg); if (j && typeof j === 'object') return j; } catch (x) {}
    return null;
  }
  function isBusy(msg) { return (window.HT9045Busy && HT9045Busy.is) ? HT9045Busy.is(msg) : /^busy:/.test(String(msg || '')); }

  /* ---- 送 ttlcfg.op ---------------------------------------------------- */
  function send(payload, tries) {
    var R = window.HT9045Recipe;
    if (!R || !R.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd(CMD, { value: JSON.stringify(payload) }); }).then(unwrap, function (e) {
      var msg = (e && e.message) || String(e);
      if (isBusy(msg) && tries < 3) {
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(payload, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(payload, tries + 1); }, function () { throw e; });
      }
      throw e;
    });
  }
  function fail(what, e) {
    var msg = (e && e.message) || String(e);
    var j = refusal(msg);
    LAST = j || { executed: false, guard: 'transport', detail: msg };
    if (j && j.guard === 'page-not-open') {
      var P = window.HT9045Page;
      if (P && P.load && P.golden && P.golden().struct === 'TTLCfg') {
        say('Delete 沒有執行（' + what + '）：伺服器要求重新開頁（page-not-open）—— 重讀中，讀完請再按一次 Delete', '#f88');
        P.load();
      } else {
        say('Delete 沒有執行（' + what + '）：要先從主畫面 DIO Set 開 DIO Interface Configuration 視窗（golden FormShow），而且登入等級不能變（page-not-open）', '#f88');
      }
      return;
    }
    if (j && j.guard) {
      say('Delete 沒有執行（' + what + '，' + j.guard + '）：' + (j.detail || '') + (j.goldenLine ? '［' + j.goldenLine + '］' : ''), '#f88');
      return;
    }
    if (/unknown cmd|unknown command/i.test(msg)) OFF = msg;
    if (isBusy(msg)) { say('Delete：伺服器忙（' + msg + '），這一下略過，請再按一次'); return; }
    var hint = /^(control-held|not-operator)$/.test(msg) ? '（操作權在另一個畫面；在這個畫面按一下存檔之類的操作鈕把操作權拿回來，或關掉另一個畫面）' : '';
    say('Delete 沒有執行（' + CMD + ' ' + what + '）：' + msg + hint, '#f88');
  }

  /* ---- 對話框（golden OpenDialog1，Windows 的開啟檔案框）------------------- */
  function esc(s) {
    return String(s == null ? '' : s).replace(/[&<>"]/g, function (x) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[x]; });
  }
  function ensureDialog() {
    if (DLG) return DLG;
    var ov = document.createElement('div');
    ov.id = 'w906DioDelDlg';
    ov.style.cssText = 'position:fixed;inset:0;background:rgba(0,0,0,.3);z-index:9000;display:none;align-items:center;justify-content:center;';
    ov.innerHTML =
      '<div style="background:var(--form-bg,#ece9d8);border:1px solid #666;box-shadow:4px 4px 16px rgba(0,0,0,.4);width:440px;max-width:96%;font:12px Arial,sans-serif;">' +
      ' <div style="background:linear-gradient(90deg,#0a246a,#3a6ea5);color:#fff;font-weight:bold;padding:3px 6px;display:flex;align-items:center;">' +
      '  <span id="w906DioDelTitle">Select file to delete</span>' +
      '  <span id="w906DioDelX" title="取消（不刪）" style="margin-left:auto;cursor:pointer;background:#d4d0c8;color:#000;border:1px solid #999;width:16px;text-align:center;">&#10005;</span></div>' +
      ' <div style="padding:8px;">' +
      '  <div style="white-space:nowrap;overflow:hidden;text-overflow:ellipsis;margin-bottom:4px;">Look in: <span id="w906DioDelDir"></span></div>' +
      '  <div id="w906DioDelList" style="height:170px;overflow:auto;background:#fff;border:1px inset #aaa;"></div>' +
      '  <div style="display:flex;align-items:center;gap:6px;margin-top:6px;"><span style="width:84px;">File name:</span>' +
      '   <input id="w906DioDelName" readonly style="flex:1;height:22px;box-sizing:border-box;"></div>' +
      '  <div style="display:flex;align-items:center;gap:6px;margin-top:4px;"><span style="width:84px;">Files of type:</span><span id="w906DioDelFilter">*.ini</span></div>' +
      '  <div style="display:flex;justify-content:flex-end;gap:8px;margin-top:8px;">' +
      '   <button id="w906DioDelOpen" class="btn3d" style="width:90px;height:28px;">Open</button>' +
      '   <button id="w906DioDelCancel" class="btn3d" style="width:90px;height:28px;">Cancel</button></div>' +
      ' </div></div>';
    document.body.appendChild(ov);
    DLG = ov;
    $('w906DioDelX').addEventListener('click', function (ev) { if (ev.isTrusted !== false) closeDialog(); });
    $('w906DioDelCancel').addEventListener('click', function (ev) { if (ev.isTrusted !== false) closeDialog(); });
    $('w906DioDelOpen').addEventListener('click', function (ev) { if (ev.isTrusted !== false) openPicked(); });
    $('w906DioDelList').addEventListener('click', function (ev) {
      var it = ev.target && ev.target.closest ? ev.target.closest('[data-f]') : null;
      if (it) pick(it.getAttribute('data-f'));
    });
    $('w906DioDelList').addEventListener('dblclick', function (ev) {   // 開檔框：連點兩下＝選它＋Open
      if (ev.isTrusted === false) return;
      var it = ev.target && ev.target.closest ? ev.target.closest('[data-f]') : null;
      if (it) { pick(it.getAttribute('data-f')); openPicked(); }
    });
    ov.addEventListener('keydown', function (ev) {   // Esc＝Cancel；Enter＝Open（焦點在按鈕上時讓那顆鈕自己處理，Cancel 上按 Enter 不會刪）
      if (ev.isTrusted === false) return;
      if (ev.key === 'Escape') { ev.preventDefault(); closeDialog(); }
      else if (ev.key === 'Enter' && !(ev.target && ev.target.tagName === 'BUTTON')) { ev.preventDefault(); openPicked(); }
    });
    return DLG;
  }
  function pick(name) {
    PICK = name || '';
    $('w906DioDelName').value = PICK;
    Array.prototype.forEach.call($('w906DioDelList').querySelectorAll('[data-f]'), function (n) {
      var on = n.getAttribute('data-f') === PICK;
      n.style.background = on ? '#0a246a' : '';
      n.style.color = on ? '#fff' : '';
    });
  }
  function showDialog(a) {
    ensureDialog();
    $('w906DioDelTitle').textContent = a.title || 'Select file to delete';   // golden :250
    $('w906DioDelDir').textContent = a.dir || '';                            // golden FormShow :26 InitialDir＝DIOCFGPath
    $('w906DioDelFilter').textContent = a.filter || '*.ini';                 // golden dfm:790 Filter
    var files = a.files || [], h = '';
    for (var i = 0; i < files.length; i++) {
      h += '<div data-f="' + esc(files[i]) + '" style="padding:1px 4px;cursor:default;white-space:nowrap;">' + esc(files[i]) + '</div>';
    }
    $('w906DioDelList').innerHTML = h || '<div style="padding:2px 4px;color:#889;">（沒有 *.ini）</div>';
    pick('');
    DLG.style.display = 'flex';
    $('w906DioDelList').setAttribute('tabindex', '0');
    $('w906DioDelList').focus();
  }
  function closeDialog() {                   // golden OpenDialog1->Execute() 回 false：不刪，只剩 spbDelete->Down=false（網頁的鈕沒有 Down 狀態）
    if (!DLG) return;
    DLG.style.display = 'none';
    PICK = '';
  }
  function openPicked() {
    if (!DLG || DLG.style.display === 'none') return;
    if (!PICK) return;                       // 開檔框沒選檔按 Open：不關、不做事
    var file = PICK;
    closeDialog();
    BUSY = true;
    send({ op: 'delete', file: file }, 0).then(function (a) {
      LAST = a;
      if (a && a.executed) {
        say('已照 ' + GOLDEN + ' 刪除 ' + (a.file || file) +
            (a.wasInUse ? '（這是目前 TesterIF 選用的 DIO 型態檔；golden 不擋也不提示，之後讀這個 DIO 檔時才會發現它不見了）' : ''), '#9f9');
      } else {
        say('DeleteFile 沒有刪掉 ' + ((a && a.file) || file) + '（golden 失敗也不報錯；這裡照實顯示，檔案還在）', '#f88');
      }
    }, function (e) { fail('delete', e); }).then(function () { BUSY = false; }, function () { BUSY = false; });
  }

  /* ---- 接線 ------------------------------------------------------------ */
  function onDelete(ev) {
    if (ev && ev.isTrusted === false) return;
    var el = ev && ev.currentTarget;
    if (el && el.disabled) return;
    if (BUSY || (DLG && DLG.style.display !== 'none')) return;     // 一次一個；對話框開著＝golden modal
    if (OFF) { say('Delete：伺服器沒有 ' + CMD + '（' + OFF + '），這顆鈕不能用', '#f88'); return; }
    BUSY = true;
    send({ op: 'list' }, 0).then(function (a) {
      LAST = a;
      if (a && a.executed && a.op === 'list') showDialog(a);
      else fail('list', new Error(JSON.stringify(a || {})));
    }, function (e) { fail('list', e); }).then(function () { BUSY = false; }, function () { BUSY = false; });
  }
  function hook() {
    var b = $('spbDelete');
    if (!b || b.__w906DioDel) return;
    b.__w906DioDel = true;
    b.style.cursor = 'pointer';
    b.addEventListener('click', onDelete);
  }
  window.addEventListener('message', function (ev) {   // 外框 HT_WIN（background.html postWinState）：視窗關了 ⇒ 對話框當作 Cancel
    var m = ev && ev.data;
    if (m && m.type === 'HT_WIN' && !m.open) closeDialog();
  });
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045DioDelete = {                 // 探針／除錯用
    busy: function () { return BUSY; }, off: function () { return OFF; }, last: function () { return LAST; },
    dialogOpen: function () { return !!(DLG && DLG.style.display !== 'none'); }, picked: function () { return PICK; }
  };
})();
