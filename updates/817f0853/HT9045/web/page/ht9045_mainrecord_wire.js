/* ht9045_mainrecord_wire.js -- Main.Record.html <-> golden V912 TfMain tsRecord（main.dfm:16128-16262）
 * ---------------------------------------------------------------------------
 * AI(W906-S119) 20260927 (St02)。手寫接線檔（檔名刻意不叫 ht9045_wire_<slug>.js，免得 gen_wire.py 重跑時覆蓋）。
 * 帳本：HT9011UC_Cpp_V3.33.906.0/docs/S119_MAIN_RECORD.md。
 *
 * golden 怎麼顯示這一頁（照做）：
 *   sgDebugRecord  只有表頭四格會被寫，而且只在語言切換鈕（sbLaguageClick main.cpp:8921-8926／8942-8945）；
 *                  資料列從來沒人寫。⇒ 開機後表頭是空的，要等語言切換（golden 怪處，照留）。
 *                  網頁的語言切換＝background.html 廣播的 {type:'HT_LANG', lang}；zh* 用中文字樣，其他用英文字樣
 *                  （golden 只有 iLanguageCountry 0／1 兩種）。
 *   memoAutoClean／btSavelog  FormShow :11147-11153 只在 #ifdef DEBUG_AUTO_CLEAN 顯示；MachineType.h 那行是註解掉的
 *                  ⇒ 出貨版藏起來（HTML 已設 display:none）。
 *   AseRecordMemo  golden 程式裡沒有人寫它 ⇒ 永遠是空的。OnDblClick＝meShuttle2DblClick（清 meShuttle2／meShuttle1，
 *                  不是清它自己）⇒ 雙擊送 act.main.meShuttle2Dbl。
 *   CLEAR          spbClearRecordClick ⇒ act.main.clearRecord。golden 沒有確認框（照做）；只有
 *                  IniConfig.bShowMainDebugRecord 一關（預設 false，只有 CC_SIGURD_HUKOU／CC_RICHTEK 打開），
 *                  關著時伺服器回 guard "debug-record-off"，這裡照實顯示「沒有清除」。
 *   ⚠ 兩個 act 的伺服器分派（JsonBridge/ChanAction.cpp）還沒接時，伺服器回 unknown-action，這裡顯示「分派未接」，
 *     不會假裝清了。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  // golden sbLaguageClick 的字樣（逐字，含英文的尾端空白）
  var HEAD_ZH = ['本次記錄', '累計記錄', '系統記錄', '前次記錄'];
  var HEAD_EN = ['Current  ', 'Aggregate', 'System   ', 'Last     '];
  var busy = false, last = null;

  function $(id) { return document.getElementById(id); }
  function say(msg, bad) {
    var s = $('rcStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
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
  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function describe(r, what) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd|unknown-action/.test(r.detail || '')) {
      return what + ' 的伺服器分派還沒接（JsonBridge/ChanAction.cpp → MainRecordClear.cpp）。沒有做任何事。';
    }
    if (r.guard === 'debug-record-off') return '沒有清除：bShowMainDebugRecord 沒開（golden main.cpp:31111）';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '');
  }
  function run(name, what) {
    if (busy) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say(what + '…');
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ })
      .then(function () { return cmd(name, { value: JSON.stringify({}) }); })
      .then(function (r) { return r; }, function (e) { return parseErr(e); })
      .then(function (r) {
        last = r; busy = false;
        if (r && r.executed) say(what + ' 完成');
        else say(describe(r, what), true);
        return r;
      });
  }

  // golden sbLaguageClick：只在語言切換時寫表頭
  function fillHeaders(lang) {
    var t = $('sgDebugRecord');
    if (!t || !t.rows.length) return;
    var h = /^zh/i.test(String(lang || '')) ? HEAD_ZH : HEAD_EN;
    for (var c = 1; c <= 4 && c < t.rows[0].cells.length; c++) t.rows[0].cells[c].textContent = h[c - 1];
  }

  function bind() {
    var b = $('spbClearRecord');
    if (b) b.addEventListener('click', function () { run('act.main.clearRecord', 'CLEAR'); });
    var m = $('AseRecordMemo');
    if (m) m.addEventListener('dblclick', function () { run('act.main.meShuttle2Dbl', '清除 Shuttle 記錄'); });
    window.addEventListener('message', function (ev) {
      if (ev.data && ev.data.type === 'HT_LANG') fillHeaders(ev.data.lang);
    });
  }

  window.HT9045MainRecord = {
    clear: function () { return run('act.main.clearRecord', 'CLEAR'); },
    shuttleClear: function () { return run('act.main.meShuttle2Dbl', '清除 Shuttle 記錄'); },
    fillHeaders: fillHeaders,
    last: function () { return last; },
    busy: function () { return busy; }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind);
  else bind();
})();
