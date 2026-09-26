/* ht9045_smartdiag_web.js -- Data.SmartDiagnostic.html <-> golden V912 TfSmartDiagnostic
 *                             （SmartDiagnostic.cpp／SmartDiagnostic.dfm）
 * ---------------------------------------------------------------------------
 * Steven 20260925 (Data.SmartDiagnostic)
 * 手寫接線檔（檔名刻意不叫 ht9045_wire_<slug>.js，免得 gen_wire.py 重跑時覆蓋）。
 * 小鍵盤仍由 ht9045_wire_datasmartdiagnostic.js（引擎）掛在兩個參數框上。
 * 探針：HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_smartdiag_probe.py
 *
 * 後端：wb_serve WS 指令 smartdiag.op → WebSmartDiag.cpp W906_SmartDiagOp，value = JSON 字串：
 *   {act:'open'}                      開窗（golden 的入口守衛 + W906_Create + FormShow :274-357）
 *   {act:'timer'}                     SmartDiagnosticTimer 一拍（每 1000 ms，只在 timerEnabled 時；只回顏色）
 *   {act:'create', confirmed}         Create Initial Cylider Name（:594-623，只改記憶體）
 *   {act:'reset', itemIndex, confirmed}  Reset Cylider Record（:692-724，只改記憶體）
 *   {act:'save', edits, confirmed}    Save（FormButtonClick Tag==9 :172-180）→ 寫 system\SmartDiagnosticRecord.txt、
 *                                     SmartDiagnosticRecordReset.txt、SmartDiagnosticPara.ini
 *   {act:'cell', col, row, value?}    點 sgCylinder 可改的格子（:854-865；整數框 MyInputBox，取消＝不給 value）
 * 兩段式確認（golden MessageDlg，:174／:597／:698）：先送 confirmed:false，回 needConfirm 時用 golden 的題目問，
 *   按確定才送 confirmed:true —— C++ 會重跑守衛（不信任前端）。網頁不做任何判斷、不自己改表格內容，
 *   一律等 C++ 回來的 state 重畫。
 * ⚠ smartdiag.op 的伺服器分派由整合者加（WebSmartDiag.h 有範例）。沒接之前畫面會明講「分派未接」，不會假裝成功。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var S = null;                   // 最近一次 C++ 回來的 state
  var busy = false, queue = [], tick = null, last = null, lastError = '';
  var st = window.__smartdiag = { ops: [], ticks: 0, skipped: 0 };

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function say(msg, bad) {
    var el = $('sdStatus');
    if (el) { el.textContent = msg || ''; el.style.color = bad ? '#b00' : '#234'; }
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
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function describe(r) {
    if (!r) return '沒有回應';
    var d = r.detail || '';
    if (/unknown cmd|unknown command|unknown-action/i.test(d) || r.guard === 'unknown-action')
      return 'smartdiag.op 的伺服器分派還沒接（WebSmartDiag.cpp W906_SmartDiagOp，由整合者在 wb_serve 加一臂）。沒有做任何事。';
    if (/no ack within/i.test(d)) return '伺服器 15 秒內沒有回應（可能正等著操作員回答權限警報，或還在寫檔）—— 結果以重新開頁為準';
    return (r.guard || '失敗') + (d ? '：' + d : '') + (r.goldenLine ? '（' + r.goldenLine + '）' : '');
  }

  // acquire → smartdiag.op → release。quiet（timer）撞上別人正在送時略過這一拍；操作員的點擊排隊。
  function op(v, quiet) {
    if (busy) {
      if (quiet) { st.skipped++; return Promise.resolve(null); }
      return new Promise(function (resolve) { queue.push({ v: v, resolve: resolve }); });
    }
    return opNow(v, quiet);
  }
  function opNow(v, quiet) {
    busy = true;
    var held = false;
    return raw('control.acquire').then(function () { held = true; }, function () { /* 已持有或別頁持有：指令自己會回錯 */ })
      .then(function () { return raw('smartdiag.op', { value: JSON.stringify(v) }).then(unwrap, parseErr); })
      .then(function (r) {
        last = r;
        if (v.act === 'timer') st.ticks++; else st.ops.push(v.act);
        if (r && r.state) { S = r.state; render(v.act === 'open'); }
        else if (r && r.summaryColors) paintSummary(r.summaryColors);
        return r;
      })
      .then(function (r) {
        var rel = held ? raw('control.release').catch(function () {}) : Promise.resolve();
        return rel.then(function () {
          busy = false;
          var next = queue.shift();
          if (next) opNow(next.v, false).then(next.resolve);
          return r;
        });
      });
  }

  // ---- 畫面 -----------------------------------------------------------------
  function pnlText(el, text) {
    if (!el) return;
    var cap = el.querySelector(':scope > .pnlCap');
    if (!cap) { cap = document.createElement('span'); cap.className = 'pnlCap'; el.appendChild(cap); }
    cap.textContent = text;
  }
  // golden DrawCell（:366-384／:1036-1053）：每格底色＝GridColor[c][r]；第 0 列置中
  function gridHtml(g, id, editable) {
    var h = '<table data-grid="' + id + '">';
    for (var r = 0; r < g.rowCount; r++) {
      h += '<tr>';
      for (var c = 0; c < g.colCount; c++) {
        var col = g.colors && g.colors[r] ? g.colors[r][c] : null;
        var tag = r === 0 ? 'th' : 'td';
        var cls = (editable && editable(c, r)) ? ' class="sdEdit"' : '';
        var sty = (col ? 'background:' + col + ';' : '') + (r === 0 ? 'text-align:center;' : '') +
                  (r === 0 && g.colWidths ? 'min-width:' + Math.max(20, g.colWidths[c] - 18) + 'px;' : '');
        h += '<' + tag + cls + ' data-c="' + c + '" data-r="' + r + '" style="' + sty + '">' + esc(g.cells[r][c]) + '</' + tag + '>';
      }
      h += '</tr>';
    }
    return h + '</table>';
  }
  function cylEditable(c, r) { return r > 0 && (c === 3 || c === 4 || c === 6 || c === 7 || c === 8); }   // golden :857-859

  function render(fromOpen) {
    if (!S) return;
    pnlText($('pn_SmartDiagnostic_Time'), S.timeCaption || '');
    var sg = $('sg_SmartDiagnostic_Summary'); if (sg) sg.innerHTML = gridHtml(S.summary, 'summary');
    var mg = $('sg_SmartDiagnostic_CyliderManagement'); if (mg) mg.innerHTML = gridHtml(S.management, 'management');
    var cy = $('sgCylinder'); if (cy) cy.innerHTML = gridHtml(S.cylinder, 'cylinder', cylEditable);
    var cb = $('cob_SmartDiagnostic_CyliderName');
    if (cb) {
      cb.innerHTML = S.combo.items.map(function (t, i) { return '<option value="' + i + '">' + esc(t) + '</option>'; }).join('');
      cb.selectedIndex = S.combo.itemIndex;                  // -1 = 沒選（VCL TComboBox 預設）
    }
    ['ed_SmartDiagnostic_LimitCountValue', 'ed_SmartDiagnostic_LimitCheckTime'].forEach(function (id) {
      var e = $(id); if (e && S.edits && typeof S.edits[id] === 'string') e.value = S.edits[id];
    });
    Object.keys(S.visible || {}).forEach(function (id) { var e = $(id); if (e) e.style.display = S.visible[id] ? '' : 'none'; });
    if (fromOpen) showPage(S.activePage);                     // golden FormShow :285／:348 ActivePageIndex
    if (S.timerEnabled && !tick) tick = setInterval(function () { if (!document.hidden) op({ act: 'timer' }, true); }, 1000);
    if (!S.timerEnabled && tick) { clearInterval(tick); tick = null; }
  }
  function paintSummary(colors) {
    var t = document.querySelector('#sg_SmartDiagnostic_Summary table');
    if (!t) return;
    colors.forEach(function (row, r) {
      var tr = t.rows[r]; if (!tr) return;
      row.forEach(function (col, c) { var td = tr.cells[c]; if (td && col) td.style.background = col; });
    });
  }
  function showPage(i) {
    var tab = document.querySelector('#pc_SmartDiagnostic .pcTabs .tab[data-t="' + i + '"]');
    if (tab) tab.click();
  }

  // ---- 兩段式確認 --------------------------------------------------------------
  // opts.answer：測試用（true/false 直接回答）；沒給就 window.confirm（golden 的英文題目）
  function twoPhase(v, opts, doneText) {
    say(v.act + '…');
    v.confirmed = false;
    return op(v).then(function (r) {
      if (r && r.needConfirm) {
        var text = (r.prompt || []).join('\n');
        var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
        if (!yes) { var c = { executed: false, guard: 'confirm-no', detail: '操作員按了 No', cancelled: true, asked: true }; say('已取消（' + text + ' → No）'); return c; }
        var v2 = JSON.parse(JSON.stringify(v)); v2.confirmed = true;
        return op(v2).then(function (r2) { if (r2) r2.asked = true; return r2; });
      }
      return r;
    }).then(function (r) {
      if (r && r.cancelled) { lastError = ''; return r; }
      if (r && r.executed) { lastError = ''; say(doneText + (r.writes ? '（寫入：' + r.writes.join('、') + '）' : '')); }
      else { lastError = describe(r); say(lastError, true); }
      return r;
    });
  }

  function create(opts) { return twoPhase({ act: 'create' }, opts, 'Create Initial Cylider Name 完成（只在記憶體，按 Save 才寫檔）'); }
  function reset(itemIndex, opts) {
    if (typeof itemIndex !== 'number') { var cb = $('cob_SmartDiagnostic_CyliderName'); itemIndex = cb ? cb.selectedIndex : -1; }
    return twoPhase({ act: 'reset', itemIndex: itemIndex }, opts, 'Reset Cylider Record 完成（只在記憶體，按 Save 才寫檔）');
  }
  function save(edits, opts) {
    if (!edits) {
      edits = {};
      ['ed_SmartDiagnostic_LimitCountValue', 'ed_SmartDiagnostic_LimitCheckTime'].forEach(function (id) { var e = $(id); if (e) edits[id] = e.value; });
    }
    return twoPhase({ act: 'save', edits: edits }, opts, 'Save 完成');
  }
  // golden sgCylinderSelectCell → MyInputBox（整數框；取消時 golden 把 atoi(原字) 寫回 —— 送不帶 value 的 cell）
  function cell(col, row, value) {
    var v = { act: 'cell', col: col, row: row };
    if (typeof value === 'number') v.value = value;
    return op(v).then(function (r) {
      if (r && r.executed) { lastError = ''; say('sgCylinder[' + col + ',' + row + '] 已改（只在記憶體：golden 不存這張表，SaveCylinderData 沒有入口）'); }
      else { lastError = describe(r); say(lastError, true); }
      return r;
    });
  }
  function open() {
    say('開窗中…');
    return op({ act: 'open' }).then(function (r) {
      if (r && r.executed) { lastError = ''; say('Smart Diagnostic：已載入（golden FormShow）' + (S && S.timerEnabled ? '；計時器執行中' : '')); }
      else { lastError = describe(r); say(lastError, true); disableAll(); }
      return r;
    });
  }
  function disableAll() {
    ['sb_SmarDiagnostic_CreateCyliderName', 'sb_SmartDiagnostic_ResetRecordCount', 'sb_SmartDiagnostic_Save'].forEach(function (id) {
      var b = $(id); if (b) { b.disabled = true; b.title += '（入口守衛擋下，見狀態列）'; }
    });
  }

  function bind() {
    var b;
    if ((b = $('sb_SmarDiagnostic_CreateCyliderName'))) b.addEventListener('click', function () { create(); });
    if ((b = $('sb_SmartDiagnostic_ResetRecordCount'))) b.addEventListener('click', function () { reset(); });
    if ((b = $('sb_SmartDiagnostic_Save'))) b.addEventListener('click', function () { save(); });
    // golden FormButtonClick 的 else 分支（:181-193）：Summary（Tag 0）／Setup（Tag 1）只換 ActivePageIndex
    if ((b = $('sb_SmartDiagnostic_Summary'))) b.addEventListener('click', function () { showPage(0); });
    if ((b = $('sb_SmartDiagnostic_Setup'))) b.addEventListener('click', function () { showPage(1); });
    var cy = $('sgCylinder');
    if (cy) cy.addEventListener('click', function (ev) {
      var td = ev.target.closest ? ev.target.closest('td.sdEdit') : null;
      if (!td) return;
      var c = +td.dataset.c, r = +td.dataset.r;
      var s = window.prompt('sgCylinder ' + (S && S.cylinder.cells[0][c] || '') + '（整數，golden MyInputBox）', td.textContent);
      if (s === null) { cell(c, r); return; }
      if (!/^\s*-?\d+\s*$/.test(s)) { say('要整數（golden MyInputBox 是整數框）', true); return; }
      cell(c, r, parseInt(s, 10));
    });
    open();
  }

  window.HT9045SmartDiag = {
    open: open, create: create, reset: reset, save: save, cell: cell,
    state: function () { return S; }, last: function () { return last; },
    lastError: function () { return lastError; }, busy: function () { return busy; }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind); else bind();
})();
