/* ht9045_showbinselect_wire.js -- Status.ShowBinSelect.html（golden V912 TfShowBinSelect，cShowBinSelect.cpp／.dfm）
 * ---------------------------------------------------------------------------
 * AI(W906-PROD-S114) 20260926（Steven 團隊，手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 資料全部來自 wb_serve 的執行期 tag（WebBridgeTags.cpp 檔尾「AI(W906-PROD-S114)」那一段），只讀、不寫檔：
 *   Test Bin       bin.<站>（33 站；auto1..3／fix1..3 那六個由 ht9045_wire_statusshowbinselect.js 經引擎接，這裡接其餘 27 個）
 *                  bin.visible（33 字元 '1'／'0'，golden SetAutoVisible → MyBinSel[i]->Visible）
 *                  bin.color（33 個 TColor，golden ShowBinSel → MyBinSel[i]->Font->Color）
 *   Category Info  binsel.cat.counts（LastSet.iBinData32[0][0..iTestBinCount]，最後一個是 Error Bin）、binsel.cat.sum（golden 的分母）、
 *                  binsel.cat.sort2d（LastSet.iTester==_2D_SORT，golden 換另一種畫法 —— 沒接）
 *                  百分比在這裡照 golden ShowCategoryBin（cShowBinSelect.cpp:1929-1946）＋ChangeToPercentage（MachineType.h:1653）算：
 *                  sum>0 ? sprintf("%0.2f%%", n/sum*100) : "0.00%"。
 *                  ⚠ 不在 C++ 的 tick 裡呼叫 golden ShowCategoryBin（它在 SystemStart 時會發低良率告警、ClearCount）。
 *                  ⚠ 四捨五入：JS toFixed 對「剛好一半」（例 1/32 = 3.125%）一律進位；golden BCB6 的 sprintf 在這種剛好一半的
 *                    情況怎麼捨入沒有量過 —— 只有分母是 2 的次方倍數的少數情況會差 0.01%。
 *   Index          binsel.index.io（LastSet.iIndexInputOutPut[0..3]）、binsel.index.clearVisible（golden FormShow :832-835）
 *                  CLEAR 鈕＝golden btnClearCountClick（:2411-2426）：WS act.showBinSelect.clearCount，兩段式確認，
 *                  守衛（CC_KYEC_LEE 看不到、Insufficient(108) 權限）全在 C++（WebShowBinSelect.cpp），不信任這一頁。
 *   UPH Information  AI(W906-PROD-S114) 20260926 第二輪：binsel.uph.grid（fShowBinSelect->UPH_StringGrid 全部格子，列 "\n"、格 "\t"，
 *                  第 0 列是表頭；golden 唯一寫入者 CalculateUPH，V912 ainarm9045.cpp:5566-5764）、binsel.uph.tabVisible（golden FormShow :863
 *                  Tab_UPH->TabVisible=IniConfig.bShowUPH）。格子照原字貼，空格就是空的（golden 開機到第一次算 UPH 之前整張是空的）。
 *   Bin Display Status（NUMBER_PANEL 選配）：沒有發布，頁面顯示 "---"。
 *
 * null 一律顯示 "---"（不可知），不是 0。
 * 防連點（S107）：CLEAR 在路上時按鈕停用、完成後冷卻 HT9045Busy.coolMs()；伺服器回 busy: 不跳錯誤框（ht9045_busy_util.js）。
 * 權杖：act.* 要 control 權杖。acquire → 指令 → 只有本頁拿的才 release（同 main.html 換配方那一段的規則），不長期佔用。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  // e6TrayName 順序 ＝ WebBridgeTags.cpp kSortSlug ＝ golden dfm laAuto1..laMag14 由上而下（Top 0..640）
  // [slug, 元件後綴, golden dfm labXxx 的 Caption]
  var ST = [
    ['auto1', 'Auto1', 'Auto 1'], ['auto2', 'Auto2', 'Auto 2'], ['auto3', 'Auto3', 'Auto 3'],
    ['auto4', 'Auto4', 'Auto 4'], ['auto5', 'Auto5', 'Auto 5'], ['auto6', 'Auto6', 'Auto 6'],
    ['fix1', 'Fix1', 'Fix1'], ['fix2', 'Fix2', 'Fix2'], ['fix3', 'Fix3', 'Fix3'],
    ['fix4', 'Fix4', 'Fix4'], ['fix5', 'Fix5', 'Fix5'], ['fix6', 'Fix6', 'Fix6'],
    ['fix7', 'Fix7', 'Fix 7'], ['fix8', 'Fix8', 'Fix 8'], ['fix9', 'Fix9', 'Fix 9'],
    ['fix10', 'Fix10', 'Fix 10'], ['fix11', 'Fix11', 'Fix 11'], ['fix12', 'Fix12', 'Fix 12'],
    ['bulkbox', 'BinBox', 'BulkBox'],
    ['mag1', 'Mag1', 'Mag 1'], ['mag2', 'Mag2', 'Mag 2'], ['mag3', 'Mag3', 'Mag 3'], ['mag4', 'Mag4', 'Mag 4'],
    ['mag5', 'Mag5', 'Mag 5'], ['mag6', 'Mag6', 'Mag 6'], ['mag7', 'Mag7', 'Mag 7'], ['mag8', 'Mag8', 'Mag 8'],
    ['mag9', 'Mag9', 'Mag 9'], ['mag10', 'Mag10', 'Mag 10'], ['mag11', 'Mag11', 'Mag 11'], ['mag12', 'Mag12', 'Mag 12'],
    ['mag13', 'Mag13', 'Mag 13'], ['mag14', 'Mag14', 'Mag 14']
  ];
  // 引擎（ht9045_wire_statusshowbinselect.js）已接的六站 —— 這裡不再寫它們的字，免得兩個寫入者搶同一格
  var ENGINE = { auto1: 1, auto2: 1, auto3: 1, fix1: 1, fix2: 1, fix3: 1 };

  var st = window.__showBinSelect = { clears: 0, last: null, lastError: '' };

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function tag(name) { return (window.HT9045Tags && HT9045Tags.has(name)) ? HT9045Tags.get(name) : null; }
  function on(name, fn) { if (window.HT9045Tags) HT9045Tags.on(name, fn); }
  function say(msg, bad) {
    var el = $('sbsStatus');
    if (!el) return;
    el.textContent = msg || '';
    el.style.color = bad ? '#b00' : '';
  }

  // golden TColor（$00BBGGRR；$80000000|n 是系統色，VCL 畫的時候才用 GetSysColor 查）→ CSS
  var SYSCOLOR = { 0x05: '#ffffff', 0x08: '#000000', 0x0F: '#f0f0f0', 0x10: '#a0a0a0', 0x12: '#000000' };
  function tcolor(v) {
    var n = Number(v);
    if (!isFinite(n)) return '';
    n = n >>> 0;
    if ((n & 0xFF000000) >>> 0 === 0x80000000) return SYSCOLOR[n & 0xFF] || '';
    var r = n & 0xFF, g = (n >>> 8) & 0xFF, b = (n >>> 16) & 0xFF;
    return 'rgb(' + r + ',' + g + ',' + b + ')';
  }

  // ---- Test Bin -------------------------------------------------------------------------------------------------
  function renderBinRows() {
    var vis = tag('bin.visible'), col = tag('bin.color');
    var cols = (typeof col === 'string' && col.length) ? col.split(',') : null;
    ST.forEach(function (s, i) {
      var row = $('row' + s[1]), val = $('la' + s[1]);
      if (row) {
        if (typeof vis === 'string' && vis.length === ST.length) {
          row.style.display = (vis.charAt(i) === '1') ? '' : 'none';
          row.classList.remove('sbsUnknownRow');
        } else {
          row.style.display = '';                       // 可見度不可知：全部列出，名稱變灰
          row.classList.add('sbsUnknownRow');
        }
      }
      if (val) val.style.color = cols ? tcolor(cols[i]) : '';
    });
  }
  function bindBinCaptions() {
    ST.forEach(function (s) {
      if (ENGINE[s[0]]) return;
      on('bin.' + s[0], function (v) {
        var el = $('la' + s[1]);
        if (!el) return;
        el.textContent = (v === null || v === undefined) ? '---' : String(v);
        el.title = 'la' + s[1] + '（golden MyBinSel[' + s[0] + ']->Caption，ShowBinSel :668／:709）' + (v === null ? '：沒有來源 → 不可知' : '');
      });
    });
    on('bin.visible', renderBinRows);
    on('bin.color', renderBinRows);
  }

  // ---- Category Info（golden ShowCategoryBin 的顯示那一半）---------------------------------------------------------
  function pct(n, den) { return den > 0 ? (n / den * 100).toFixed(2) + '%' : '0.00%'; }   // golden :1933-1941 ＋ ChangeToPercentage
  function renderCat() {
    var body = $('StrGrdCategoryBody');
    if (!body) return;
    var counts = tag('binsel.cat.counts'), sum = tag('binsel.cat.sum'), s2d = tag('binsel.cat.sort2d');
    var h = '';
    if (typeof counts !== 'string' || counts === '' || typeof sum !== 'number') {
      h = '<tr><td>---</td><td>---</td><td>---</td></tr>';
      body.title = 'binsel.cat.* 沒有來源（LastSet／配方還沒載入）→ 不可知';
    } else if (s2d === true) {
      h = '<tr><td colspan="3" style="color:#b60;">LastSet.iTester==_2D_SORT：golden 換成另一種畫法（ShowCategoryBin :1879-1925，只列 iByBinCnt&gt;0 的 bin、多一欄 expect count）—— ATK 2DID 分揀的客戶分支（S25），這一版沒接</td></tr>';
      body.title = '2DID sort 分支沒接';
    } else {
      var a = counts.split(',').map(Number), nBin = a.length - 1;
      for (var i = 0; i < nBin; i++)
        h += '<tr><td>Category ' + i + '</td><td>' + a[i] + '</td><td>' + pct(a[i], sum) + '</td></tr>';   // ShowInitialString :1789-1790
      h += '<tr><td>Error Bin</td><td>' + a[nBin] + '</td><td>' + pct(a[nBin], sum) + '</td></tr>';        // :1794
      body.title = 'LastSet.iBinData32[0][0..' + nBin + ']，分母 sum=' + sum + '（golden ShowCategoryBin :1862-1874）';
    }
    body.innerHTML = h;
  }

  // ---- UPH Information（AI(W906-PROD-S114) 20260926：golden CalculateUPH 寫的 UPH_StringGrid）------------------------------
  function renderUph() {
    var body = $('UPH_StringGridBody');
    if (!body) return;
    var g = tag('binsel.uph.grid');
    if (typeof g !== 'string') {
      body.innerHTML = '<tr><td>---</td><td>---</td><td>---</td><td>---</td></tr>';
      body.title = 'binsel.uph.grid 沒有來源（設定還沒載入或還沒連上）→ 不可知';
      return;
    }
    var rows = g.split('\n'), h = '';
    rows.forEach(function (line, r) {
      var cells = line.split('\t'), tagName = (r === 0) ? 'th' : 'td';   // golden FixedRows 預設 1
      h += '<tr data-r="' + r + '">';   // AI(W906-E023-SB3) 20261002 [W906] (St01): the grid row for the double-click (ht9045_showbinselect_ev.js); same line
      cells.forEach(function (v) { h += '<' + tagName + '>' + esc(v) + '</' + tagName + '>'; });
      h += '</tr>';
    });
    body.innerHTML = h;
    body.title = 'UPH_StringGrid ' + (rows[0] ? rows[0].split('\t').length : 0) + '×' + rows.length +
                 '（golden CalculateUPH ainarm9045.cpp:5566-5764；表頭 :5582-5585、第 1..10 列最近 10 次、第 12 列平均 :5717-5719）';
  }
  function renderUphTab() {
    var v = tag('binsel.uph.tabVisible');
    var t = document.querySelector('#PageControl1 > .tab[data-tab="uph"]');
    if (!t) return;
    t.style.display = (v === false) ? 'none' : '';              // golden FormShow :863；不可知時照樣顯示
    t.title = 'Tab_UPH（golden FormShow :863 TabVisible=IniConfig.bShowUPH）' + (v === null ? '：可見度不可知' : '');
    if (v === false && t.classList.contains('act')) {           // 藏起來的分頁正開著 → 回到第一頁（golden 藏掉的分頁點不到）
      var first = document.querySelector('#PageControl1 > .tab[data-tab="testbin"]');
      if (first) first.click();
    }
  }

  // ---- Index --------------------------------------------------------------------------------------------------
  var IDX =[['IndexInput', 0], ['IndexOut', 1], ['OutArm_input', 2], ['labInArm_input', 3]];   // golden cSortCT.cpp:425-428
  function renderIndex() {
    var io = tag('binsel.index.io');
    var a = (typeof io === 'string' && io.length) ? io.split(',') : null;
    IDX.forEach(function (d) {
      var el = $(d[0]);
      if (el) el.textContent = (a && a.length === 4) ? a[d[1]] : '---';
    });
  }
  function renderClearVisible() {
    var b = $('btnClearCount'), v = tag('binsel.index.clearVisible');
    if (!b) return;
    b.style.display = (v === false) ? 'none' : '';    // golden FormShow :832-835（CC_KYEC_LEE 藏起來）；不可知時照樣顯示，C++ 會再擋
    b.title = 'btnClearCount（golden btnClearCountClick cShowBinSelect.cpp:2411-2426 → ClearCount(ctIndexCount)）' +
              (v === null ? '：可見度不可知（C++ 會再檢查）' : '');
  }

  // ---- CLEAR（golden btnClearCountClick）-------------------------------------------------------------------------
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
  function send(confirmed) {
    return raw('act.showBinSelect.clearCount', { value: JSON.stringify({ confirmed: !!confirmed }) })
      .then(unwrap, parseErr);
  }
  function isBusy(r) { return !!(window.HT9045Busy && HT9045Busy.is(r)); }
  function describe(r) {
    if (!r) return '沒有回應';
    if (/unknown cmd|unknown-action/.test((r.detail || '') + (r.guard || ''))) {
      return 'CLEAR 的伺服器分派還沒接（act.showBinSelect.clearCount → WebShowBinSelect.cpp），由整合者加一行。沒有清除任何東西。';
    }
    if (/not-operator|control-held/.test(r.detail || '')) return '拿不到控制權杖（別的頁面正持有），沒有清除';
    return '沒有清除：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }

  var busy = false, coolUntil = 0;
  // opts.answer：測試用，true/false 直接回答確認框；沒給就用 window.confirm（golden 的兩行字）
  function clearCount(opts) {
    if (busy || Date.now() < coolUntil) return Promise.resolve({ executed: false, guard: 'busy-local' });
    var b = $('btnClearCount');
    busy = true; if (b) b.disabled = true;
    say('CLEAR…');
    var had = clientHolds(), took = false;
    return raw('control.acquire').then(function () { if (!had) took = true; }, function () { /* 已持有或他人持有：後面的指令自己會回錯 */ })
      .then(function () { return send(false); })
      .then(function (r) {
        if (r && r.needConfirm) {
          var text = (r.prompt || ['Sure To Clear Counter?', '確定是否要重新計數？']).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          if (!yes) return { executed: false, guard: 'confirm-no', cancelled: true };
          return send(true);
        }
        return r;
      })
      .then(function (r) {
        st.last = r;
        if (r && r.executed) { st.clears++; st.lastError = ''; say('Index 計數已清除（golden btnClearCountClick → ClearCount(ctIndexCount)；記憶體，golden 這支不存檔）'); }
        else if (r && r.cancelled) { st.lastError = ''; say('已取消，沒有清除'); }
        else if (isBusy(r)) { st.lastError = ''; say(HT9045Busy.NOTE); }
        else { st.lastError = describe(r); say(st.lastError, true); }
        return r;
      }, function (e) {
        var r = parseErr(e); st.last = r;
        if (isBusy(r)) { say(HT9045Busy.NOTE); } else { st.lastError = describe(r); say(st.lastError, true); }
        return r;
      })
      .then(function (r) {
        var rel = (took && !clientHolds()) ? raw('control.release').then(null, function () {}) : Promise.resolve();
        return rel.then(function () {
          busy = false; if (b) b.disabled = false;
          coolUntil = Date.now() + ((window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400);
          return r;
        });
      });
  }

  function start() {
    bindBinCaptions();
    renderBinRows();
    ['binsel.cat.counts', 'binsel.cat.sum', 'binsel.cat.sort2d'].forEach(function (t) { on(t, renderCat); });
    renderCat();
    on('binsel.uph.grid', renderUph);                                  // AI(W906-PROD-S114) 20260926
    on('binsel.uph.tabVisible', renderUphTab);
    renderUph(); renderUphTab();
    on('binsel.index.io', renderIndex);
    on('binsel.index.clearVisible', renderClearVisible);
    renderIndex(); renderClearVisible();
    var b = $('btnClearCount');
    if (b) b.addEventListener('click', function () { clearCount(); });
    if (window.HT9045Tags && typeof HT9045Tags.connect === 'function') HT9045Tags.connect().catch(function () {});
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ShowBinSelect = { clear: clearCount, state: function () { return st; }, pct: pct, renderUph: renderUph };
})();
