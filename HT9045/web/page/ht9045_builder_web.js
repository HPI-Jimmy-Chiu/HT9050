/* ht9045_builder_web.js -- Data.Builder.html <-> golden V912 TfBuilder（cBuilder.cpp／cBuilder.dfm）
 * ---------------------------------------------------------------------------
 * Steven 20260925 (Data.Builder)
 * 手寫接線檔（檔名刻意不叫 ht9045_wire_<slug>.js，免得 gen_wire.py 重跑時覆蓋）。
 * 新名稱的小鍵盤（NO_SYMBOL）仍由 ht9045_wire_databuilder.js（引擎）掛。
 * 探針：HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_builder_probe.py
 *
 * 後端：wb_serve WS 指令 builder.op → WebBuilder.cpp W906_BuilderOp，value = JSON 字串：
 *   {act:'open'}                              開窗（golden 入口守衛 + NewRecordProcess MES2179 + FormShow）
 *   {act:'change', widget, text}              選 cbSourceFile／cbDeleteFile、打 edNewFileName（golden OnChange／OnKeyPress）
 *   {act:'create', source, name, confirmed}   Create（:72-159）→ 建 IniData\Data\<name>、IniData\Offset\<name>
 *   {act:'delete', name, confirmed}           Delete（:202-230）→ 刪上面兩個資料夾（資源回收筒）；使用中的配方 golden 會擋
 *   {act:'dir', path} / {act:'drive', drive}  DirectoryListBox1 雙擊資料夾／DriveComboBox1 換磁碟機
 *   {act:'import', confirmed}                 Import（:475-482）→ 瀏覽的資料夾複製進 IniData\Data
 *   {act:'export', checked:[…], confirmed}    Export（:396-473）→ 勾選的配方複製到瀏覽的資料夾（先刪同名）
 *   {act:'close'}                             Exit（FormClose，fShow=false）
 * 兩段式確認（golden Application->MessageBox OK/CANCEL）：先送 confirmed:false，回 needConfirm 時用 golden 的題目問，
 *   按確定才送 confirmed:true —— C++ 重跑守衛（不信任前端）。網頁不做判斷，一律用 C++ 回來的 state 重畫。
 * ⚠ builder.op 的伺服器分派由整合者加（WebBuilder.h 有範例）。沒接之前畫面會明講「分派未接」，不會假裝成功。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var S = null, busy = false, queue = [], last = null, lastError = '';
  var ticks = {};                 // CheckListBox1 的勾（golden 是控制項自己的狀態；InitCompData 會清掉）
  var selDir = null;              // DirectoryListBox1 目前反白的那一列（單擊選、雙擊進入＝VCL）
  var edTimer = null;
  var st = window.__builder = { ops: [] };

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function say(msg, bad) {
    var el = $('bdStatus');
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
      return 'builder.op 的伺服器分派還沒接（WebBuilder.cpp W906_BuilderOp，由整合者在 wb_serve 加一臂）。沒有動任何檔案。';
    if (/no ack within/i.test(d)) return '伺服器 15 秒內沒有回應（可能正等著操作員回答權限警報，或複製很久）—— 結果以重新整理清單為準';
    var msgs = (r.messages || []).map(function (m) { return m.s1 + (m.s2 ? '／' + m.s2 : ''); });
    if (r.goldenStopped && msgs.length) return 'golden 擋下：' + msgs.join('；');
    return (r.portGuard ? '（移植樹守衛）' : '') + (r.guard || '失敗') + (d ? '：' + d : '') + (r.goldenLine ? '（' + r.goldenLine + '）' : '') +
           (msgs.length ? '；訊息：' + msgs.join('；') : '');
  }

  function op(v) {
    if (busy) return new Promise(function (resolve) { queue.push({ v: v, resolve: resolve }); });
    busy = true;
    var held = false;
    return raw('control.acquire').then(function () { held = true; }, function () { /* 已持有或別頁持有：指令自己會回錯 */ })
      .then(function () { return raw('builder.op', { value: JSON.stringify(v) }).then(unwrap, parseErr); })
      .then(function (r) {
        last = r; st.ops.push(v.act);
        if (r && r.state) { S = r.state; render(); }
        return r;
      })
      .then(function (r) {
        var rel = held ? raw('control.release').catch(function () {}) : Promise.resolve();
        return rel.then(function () {
          busy = false;
          var next = queue.shift();
          if (next) op(next.v).then(next.resolve);
          return r;
        });
      });
  }

  // ---- 畫面 -----------------------------------------------------------------
  function fillSelect(el, items, idx) {
    if (!el) return;
    el.innerHTML = items.map(function (t) { return '<option value="' + esc(t) + '">' + esc(t) + '</option>'; }).join('');
    el.selectedIndex = idx;                                  // -1 = 沒選（csDropDownList 預設）
  }
  function render() {
    if (!S) return;
    fillSelect($('cbSourceFile'), S.cbSourceFile.items, S.cbSourceFile.itemIndex);
    fillSelect($('cbDeleteFile'), S.cbDeleteFile.items, S.cbDeleteFile.itemIndex);
    var ed = $('edNewFileName');
    if (ed && document.activeElement !== ed) ed.value = S.edNewFileName.text;
    if (ed) ed.disabled = !S.edNewFileName.enabled;          // golden cbSourceFileChange :35-41
    var b;
    if ((b = $('btCreateSetupFile'))) b.disabled = !S.btCreateSetupFile.enabled;   // golden edNewFileNameChange :43-49
    if ((b = $('btDeleteSetupFile'))) b.disabled = !S.btDeleteSetupFile.enabled;   // golden cbDeleteFileChange :194-200
    // CheckListBox1：項目換了的就丟掉舊勾
    var items = S.CheckListBox1.items, keep = {};
    items.forEach(function (t) { if (ticks[t]) keep[t] = true; });
    ticks = keep;
    var cl = $('CheckListBox1');
    if (cl) cl.innerHTML = items.map(function (t) {
      return '<label class="bdRow"><input type="checkbox" data-name="' + esc(t) + '"' + (ticks[t] ? ' checked' : '') + '> ' + esc(t) + '</label><br>';
    }).join('');
    var lab = $('labDir'); if (lab) lab.textContent = S.labDir;
    var dl = $('DirectoryListBox1');
    if (dl) dl.innerHTML = (S.dirTree || []).map(function (n) {
      var icon = n.kind === 'closed' ? '📁' : '📂';
      return '<div class="bdDir' + (n.kind === 'current' ? ' sel' : '') + '" data-path="' + esc(n.path) + '" style="padding-left:' + (2 + 14 * n.level) + 'px;">' +
             icon + ' ' + esc(n.name) + '</div>';
    }).join('');
    var dc = $('DriveComboBox1');
    if (dc) {
      dc.innerHTML = (S.drives || []).map(function (d) { return '<option value="' + d.letter + '">' + esc(d.label) + '</option>'; }).join('');
      dc.value = S.drive;
    }
  }

  // ---- 兩段式確認 --------------------------------------------------------------
  // opts.answer：測試用（true/false）；沒給就 window.confirm（golden 的題目）
  function twoPhase(v, opts, doneText, onCancelExtra) {
    say(v.act + '…');
    v.confirmed = false;
    return op(v).then(function (r) {
      if (r && r.needConfirm) {
        var text = (r.prompt || []).join('\n');
        if (r.nameExists) text += '\n\n⚠ 已經有同名的配方：golden 不檢查，會把來源的檔案複製進去（覆蓋同名檔）。';
        var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
        if (!yes) {
          if (onCancelExtra) onCancelExtra();
          say('已取消（' + text.split('\n')[0] + ' → Cancel）');
          return { executed: false, guard: 'confirm-no', detail: '操作員按了 Cancel', cancelled: true, asked: true, phase1: r };
        }
        var v2 = JSON.parse(JSON.stringify(v)); v2.confirmed = true;
        return op(v2).then(function (r2) { if (r2) r2.asked = true; return r2; });
      }
      return r;
    }).then(function (r) {
      if (r && r.cancelled) { lastError = ''; return r; }
      var bad = (r && r.shellOps || []).filter(function (o) { return o.ret !== 0 || o.aborted; });
      if (r && r.executed && !bad.length) { lastError = ''; say(doneText(r)); }
      // golden 不看 SHFileOperation 的回傳（只有 DeleteSetupFile :253 會報錯），照樣算完成；把 Windows 的回報列出來
      // （例：來源配方沒有 IniData\Offset\<來源> 時，Offset 那次 FO_COPY 找不到檔案）
      else if (r && r.executed) { lastError = ''; say(doneText(r) + '\n⚠ Windows 檔案操作回報：' + bad.map(function (o) { return o.op + ' ' + o.from + (o.to ? ' → ' + o.to : '') + ' ret=' + o.ret; }).join('；')); }
      else { lastError = describe(r); say(lastError, true); }
      return r;
    });
  }

  function create(source, name, opts) {
    if (typeof source !== 'string') source = $('cbSourceFile') ? $('cbSourceFile').value : '';
    if (typeof name !== 'string') name = $('edNewFileName') ? $('edNewFileName').value : '';
    return twoPhase({ act: 'create', source: source, name: name }, opts, function (r) {
      return '已建立 ' + name + '（' + (r.after || []).map(function (a) { return a.path + (a.exists ? ' ✓' : ' ✗'); }).join('、') + '）';
    });
  }
  function del(name, opts) {
    if (typeof name !== 'string') name = $('cbDeleteFile') ? $('cbDeleteFile').value : '';
    // golden :223-229：問完不管答什麼都重列清單、清掉勾（InitCompData）
    return twoPhase({ act: 'delete', name: name }, opts, function (r) {
      ticks = {};
      return '已刪除 ' + name + '（移到資源回收筒；' + (r.after || []).map(function (a) { return a.path + (a.exists ? ' 還在' : ' 已不在'); }).join('、') + '）';
    }, function () { ticks = {}; });
  }
  function doImport(opts) {
    return twoPhase({ act: 'import' }, opts, function (r) {
      ticks = {};
      return 'Import 完成（' + (r.shellOps || []).length + ' 次複製，來源 ' + (S ? S.directory : '') + '）';
    });
  }
  function doExport(checked, opts) {
    if (!Array.isArray(checked)) checked = Object.keys(ticks).filter(function (k) { return ticks[k]; });
    return twoPhase({ act: 'export', checked: checked }, opts, function (r) {
      return (r.infoBoxes || []).join('；') + '（' + (r.after || []).map(function (a) { return a.path + (a.exists ? ' ✓' : ' ✗'); }).join('、') + '）';
    });
  }
  function change(widget, text) {
    return op({ act: 'change', widget: widget, text: text }).then(function (r) {
      if (r && r.keyFiltered && widget === 'edNewFileName') {
        var ed = $('edNewFileName'); if (ed && S) ed.value = S.edNewFileName.text;
        say('golden 的 edNewFileNameKeyPress 擋掉了 \' / : * ? " < | 這些字元');
      } else if (r && !r.executed) say(describe(r), true);
      return r;
    });
  }
  function dir(path) {
    return op({ act: 'dir', path: path }).then(function (r) { if (r && !r.executed) say(describe(r), true); else say(S ? S.labDir : ''); return r; });
  }
  function drive(letter) {
    return op({ act: 'drive', drive: letter }).then(function (r) { if (r && !r.executed) say(describe(r), true); else say(S ? S.labDir : ''); return r; });
  }
  function open() {
    say('開窗中…');
    return op({ act: 'open' }).then(function (r) {
      if (r && r.executed) { lastError = ''; say('Build Setup：' + (S ? S.cbSourceFile.items.length : 0) + ' 個配方（' + (S ? S.dataPath : '') + '）；使用中：' + (S ? S.currentRecipe : '')); }
      else { lastError = describe(r); say(lastError, true); disableAll(); }
      return r;
    });
  }
  function disableAll() {
    ['btCreateSetupFile', 'btDeleteSetupFile', 'spbImport', 'spbExport', 'edNewFileName', 'cbSourceFile', 'cbDeleteFile'].forEach(function (id) {
      var b = $(id); if (b) b.disabled = true;
    });
  }

  function bind() {
    var el;
    if ((el = $('cbSourceFile'))) el.addEventListener('change', function () { change('cbSourceFile', this.value); });
    if ((el = $('cbDeleteFile'))) el.addEventListener('change', function () { change('cbDeleteFile', this.value); });
    if ((el = $('edNewFileName'))) {
      var fire = function () { var v = this.value; clearTimeout(edTimer); edTimer = setTimeout(function () { change('edNewFileName', v); }, 150); };
      el.addEventListener('input', fire);
      el.addEventListener('change', fire);
    }
    if ((el = $('btCreateSetupFile'))) el.addEventListener('click', function () { create(); });
    if ((el = $('btDeleteSetupFile'))) el.addEventListener('click', function () { del(); });
    if ((el = $('spbImport'))) el.addEventListener('click', function () { doImport(); });
    if ((el = $('spbExport'))) el.addEventListener('click', function () { doExport(); });
    if ((el = $('spbExit'))) el.addEventListener('click', function () { op({ act: 'close' }); });   // 頁面既有的 .exitbtn 處理器負責關窗
    if ((el = $('CheckListBox1'))) el.addEventListener('change', function (ev) {
      var cb = ev.target; if (cb && cb.dataset && cb.dataset.name !== undefined) ticks[cb.dataset.name] = cb.checked;
    });
    if ((el = $('DirectoryListBox1'))) {
      el.addEventListener('click', function (ev) {
        var row = ev.target.closest ? ev.target.closest('.bdDir') : null; if (!row) return;
        if (selDir) selDir.classList.remove('sel');
        selDir = row; row.classList.add('sel');
      });
      el.addEventListener('dblclick', function (ev) {                     // VCL TDirectoryListBox.DblClick → OpenCurrent
        var row = ev.target.closest ? ev.target.closest('.bdDir') : null; if (row) dir(row.dataset.path);
      });
    }
    if ((el = $('DriveComboBox1'))) el.addEventListener('change', function () { drive(this.value); });
    open();
  }

  window.HT9045Builder = {
    open: open, create: create, del: del, importDir: doImport, exportTo: doExport, change: change, dir: dir, drive: drive,
    state: function () { return S; }, last: function () { return last; }, lastError: function () { return lastError; },
    busy: function () { return busy; }, tick: function (name, on) { ticks[name] = !!on; if (S) render(); }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind); else bind();
})();
