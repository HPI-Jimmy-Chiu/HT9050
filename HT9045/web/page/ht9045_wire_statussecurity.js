/* ht9045_wire_statussecurity.js -- Status.Security.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260916
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-LEVELSET) 20260916。這個檔是手寫的，不是 gen_wire.py 產生的 ——
 * 理由見下面「為什麼沒有靜態對照表」。
 *
 * 來源表單    cSecurity.dfm / cSecurity.cpp
 *
 * ---------------------------------------------------------------------------
 * 這一頁橫跨三套不同的持久化機制，這次只接第一套
 * ---------------------------------------------------------------------------
 *   (1) 179 組 radio（權限表）   -> system\levelset.dat
 *       cSecurity.cpp:1620 GetLevelSet() / :1682 SetLevelSet()，
 *       整塊 ReadData/WriteData 一個 LAST_LEVEL_SET（cprod.h:1148，
 *       `int AccessLevel[256]`，1024 bytes）。**本檔接的就是這一套。**
 *
 *   (2) 13 個 checkbox ＋ rgJamLevel / rgMachineStatusBit8
 *                                -> Error\English\JAM0000.dat（ini）
 *       cSecurity.cpp:272 定義 FileNameJam000，寫入集中在 :1269-1324。
 *       Steven 團隊 20260926 已接：見檔尾「Jam 分頁」區塊（WS security.jam，WebSecurityJam.cpp）。以下是當時的說明：
 *       **未接**：那個檔不在 wb_serve 的 40 支 sysfile 表裡（Error 目錄只列了
 *       AlarmDescription.ini 與 AlarmCodeList.txt）。要接得先補表。
 *
 *   (3) config\Security_new.def -> **與這一頁無關**。
 *       它是 cAuthority.cpp 讀的各表單 Enable/Disable 旗標；cSecurity.cpp 全檔
 *       0 次提及它。20260915/16 的日誌寫「後端 securityNew 已在服務中，可以直接
 *       接」是把兩個檔搞混了，那個推論是斷的。
 *
 * ---------------------------------------------------------------------------
 * 為什麼沒有靜態對照表（id -> 檔/區段/鍵）
 * ---------------------------------------------------------------------------
 * 這一頁的 722 個 radio **沒有任何一個帶 id**，包住它們的 fieldset 也沒有。
 * 引擎的 sysEnums 是靠 `<fieldset id=X>` 取容器，這條路在這裡走不通。
 * 而索引本身就在 DOM 裡：
 *     title="MySecurity_Panel_N : TMySecurity（[NN] Main - Tools）"
 * 所以引擎的 sysLevels 模式改成執行期從 title 掃出 [NN] 建表，並自我驗證。
 *
 * ⚠ N 與 NN 是兩個不同的數，這是這一頁最容易靜默寫錯的地方：
 *     MySecurity_Panel_14  ->  [32] Main - Temperature Deg Setup
 *     sec_sbTools_0        ->  [14] Tools - Tray Form
 *     sec_sbTools_29       ->  [178] Tools - Tray Function
 *   `_<n>` 是「該 scrollbox 內的第幾個」，`[NN]` 才是 AccessLevel 的下標。
 *   拿 `_<n>` 當索引會把 179 個權限全部寫到錯的格子；因為值域都是 0..3，
 *   寫進去不會報錯，只會在下次有人登入時發現權限全亂。
 *
 * 期望 179 組：cSecurity.cpp:71-260 的 mySecurityPal.push_back 實測 179 筆
 * （[00] 到 [178]）。該檔 :65 的註解寫「178-entry」是錯的，forms/fSecurity.h:188
 * 的 179 才對。掃到的組數與這個數字對不上，引擎會整頁拒接。
 *
 * ---------------------------------------------------------------------------
 * ⚠ 已知落差：寫進去的值，這個行程不會讀到
 * ---------------------------------------------------------------------------
 * C++ 端 GATE SEC1（cSecurity.cpp:71-260，mySecurityPal 全部建構）與
 * GATE SEC-W1/SEC-W2（:1642 / :1690，兩處 WriteData）都還關著，iMaxLevelItem
 * 恆為 0。所以：
 *   * handler 行程自己不會寫 levelset.dat，也不會在執行中重讀；
 *   * web 寫進去的值要等 handler 下次啟動 GetLevelSet() 才生效。
 * 這是使用者 20260916 明確定的範圍：「讀檔跟寫檔要先做起來，實際使用先不用管」。
 *
 * 另一個落差：畫面只有 4 個選項（Operator/Engineer/Supervisor/HonPrec），但
 * golden 在 CosFunction.bSecurityHave5Level 時是 5 階（0..4）。伺服器收 0..4，
 * 所以 5 階機台的值不會被夾壞；但這一頁表達不了第 5 階 —— 讀到 4 的那一格會
 * 被記成 UNFILLABLE 並讓整頁拒寫，而不是靜靜挑一顆選起來。
 */
HT9045Wire.register({
  page: 'Status.Security.html',
  slug: 'statussecurity',
  // Steven 20260916：頁面自己的存檔鈕（引擎不再注入浮動的 Save to recipe / Reload）
  // 依據：golden cSecurity.cpp:510 FormClose() -> SetLevelSet()。這一頁沒有存檔鈕，golden 是「關閉表單時才寫檔」，所以 Exit 就是存檔動作 —— 這是忠實移植，不是自訂
  saveBtn: 'SecurityExit',
  // ⚠ 引擎在捕獲階段攔截這顆鈕，所以按下去會**存檔但不會離開頁面**。
  //   golden 的 Exit 是「關閉表單 -> FormClose -> SetLevelSet 寫檔」，
  //   web 這邊沒有表單關閉的語意，而存檔流程是非同步的（預演->確認->寫入->重讀），
  //   放行關閉會把流程中斷。這行說明會顯示在狀態列，讓操作員知道行為不一樣。
  saveBtnNote: 'golden 是關閉表單時才寫檔，所以這裡按 Exit＝存檔；存完不會離開頁面',
  fields: {
  },
  optional: {
  },
  // i32 投影：/api/system/levelset ＋ system.levels.put
  //   file   伺服器那側的名字（wb_serve.cpp 的 SysBinTable）
  //   expect 期望掃到的組數；對不上就整頁拒接
  sysLevels: {
    file: 'levelset',
    expect: 180   // Steven 20260924（審查第 8 輪 M-3）：golden V912 180 組（[00]..[179]）；W906_SecurityBoot iMaxLevelItem=180
  },
  kb: {
  },
  /* PENDING —— 這一頁還沒接的部分，不是漏掉，是各有原因
   *
   * cbJamNeedRed / cbSilentMode / cbUnlockPassWord / cbIncludeMTBA /
   * chkCheckContAlarm / chkO17 / cbAddAlarmLog / cbN27AlarmSel /
   * cbN27AlarmSelByArea / cbN27AddBoard / chkTCPAlarm / cbContAlarmNotUpload /
   * chkAlarmAfterFullTray / rgJamLevel / rgMachineStatusBit8
   *   -> Error\English\JAM0000.dat，不在 wb_serve 的 sysfile 表裡（見檔頭 (2)）。
   *
   * sbSupervisor / sbEngineer / btnHonPrec / btnOperator / ChangePassword 那組
   *   -> 密碼與登入，走 auth.login 指令不是檔案讀寫，另案。
   */
  pending: {
  }
});

/* ===========================================================================
 * Jam 分頁（tsJamCode／tsStatisticsJam）—— Steven 團隊 20260926
 * ---------------------------------------------------------------------------
 * golden V912 TfSecurity（cSecurity.cpp）。所有判斷在 C++（WebSecurityJam.cpp，WS security.jam），這裡只貼狀態、送操作：
 *   開分頁            op open   ＝ golden FormShow（:261-437；:418-427 選第 0 區第 0 碼）
 *   換區／換碼／換語言 op select ＝ golden cbJamAreaChange :952／cbJamCodeChange :965／cbJamLangChange :972
 *                      （ChangeJamMessage(true) 先 SaveJamLevel 存上一筆，所以把畫面目前的勾選一起送）
 *   Exit              op save   ＝ golden FormClose :463 SaveJamLevel（levelset 那段仍由引擎的 system.levels.put 做）
 *   Import／Export    op import／export ＝ golden spbImportClick :1517／spbExportClick :1587
 *                      檔案對話框的替代：Import＝瀏覽器選檔（原位元組 base64 上傳），Export＝瀏覽器下載 JamCode.csv（原位元組）
 *   Statistics Jam    op stats  ＝ sgStatisticsJam（只在記憶體，唯讀）
 * RichEditJamCode 唯讀：網頁不能改訊息說明（C++ 存檔時照 golden 把載入的內容寫回）。
 * 權限：C++ 依 golden FormShow :302-381 算 PageControl1 可見性，看不到時回 guard not-authorized。
 * =========================================================================*/
(function () {
  'use strict';
  function $(id) { return document.getElementById(id); }
  var BOXES = ['cbJamNeedRed', 'cbSilentMode', 'cbUnlockPassWord', 'cbIncludeMTBA', 'chkCheckContAlarm', 'chkO17', 'cbAddAlarmLog',
               'cbN27AddBoard', 'cbN27AlarmSel', 'cbN27AlarmSelByArea', 'chkTCPAlarm', 'cbContAlarmNotUpload', 'chkAlarmAfterFullTray'];
  var cur = null, opened = false, busy = false, last = null;

  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function op(payload) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.resolve({ executed: false, guard: 'no-client' });
    return HT9045Recipe.rawCmd('control.acquire').catch(function () {})
      .then(function () { return HT9045Recipe.rawCmd('security.jam', { value: JSON.stringify(payload) }); })
      .then(unwrap, parseErr);
  }
  function status(msg, bad) {
    var s = $('jamStatus');
    if (!s) {
      var pane = document.querySelector('.pcPane[data-p="10"]');
      if (!pane) return;
      s = document.createElement('span');
      s.id = 'jamStatus';
      s.style.cssText = 'position:absolute;left:4px;top:360px;font-size:12px;white-space:nowrap;';
      pane.appendChild(s);
    }
    s.textContent = msg; s.style.color = bad ? '#b00' : '#333';
  }
  var BS = String.fromCharCode(92);
  function rtfToText(t) {
    if (t.indexOf('{' + BS + 'rtf') !== 0) return t;
    var out = '', i = 0, depth = 0, skip = -1;
    while (i < t.length) {
      var ch = t[i];
      if (ch === '{') { depth++; i++; continue; }
      if (ch === '}') { if (skip === depth) skip = -1; depth--; i++; continue; }
      if (ch === BS) {
        var m = /^\\([a-zA-Z]+)(-?\d+)? ?|^\\'([0-9a-fA-F]{2})|^\\(.)/.exec(t.slice(i, i + 40));
        if (!m) { i++; continue; }
        i += m[0].length;
        if (m[1]) {
          if (m[1] === 'fonttbl' || m[1] === 'colortbl' || m[1] === 'stylesheet' || m[1] === 'info') skip = depth;
          else if (skip < 0 && (m[1] === 'par' || m[1] === 'line')) out += '\n';
          else if (skip < 0 && m[1] === 'tab') out += '\t';
        } else if (m[3] && skip < 0) out += String.fromCharCode(parseInt(m[3], 16));
        else if (m[4] && skip < 0 && m[4] !== '*') out += m[4];
        continue;
      }
      if (skip < 0 && ch !== '\r' && ch !== '\n') out += ch;
      i++;
    }
    return out;
  }
  function radios(fsId) { var fs = $(fsId); return fs ? fs.querySelectorAll('input[type=radio]') : []; }
  function setRadioItems(fsId, items) {
    var r = radios(fsId);
    if (!items || r.length === items.length) return;
    var cli = $(fsId).querySelector('.cli');
    cli.innerHTML = '';
    items.forEach(function (t) {
      var l = document.createElement('label'); l.className = 'rgi';
      var i = document.createElement('input'); i.type = 'radio'; i.name = 'rg_' + fsId;
      l.appendChild(i); l.appendChild(document.createTextNode(t)); cli.appendChild(l);
    });
  }
  function radioIndex(fsId) { var r = radios(fsId); for (var i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; }

  function render(r) {
    if (!r || !r.sel) return;
    last = r;
    cur = { area: r.sel.area, code: r.sel.code, lang: r.sel.lang };
    var a = $('cbJamArea'); if (a) a.selectedIndex = r.sel.area;
    var c = $('cbJamCode');
    if (c) {
      c.innerHTML = '';
      (r.codes || []).forEach(function (t) { var o = document.createElement('option'); o.textContent = t; c.appendChild(o); });
      c.selectedIndex = r.sel.code;
    }
    var l = $('cbJamLang'); if (l) l.selectedIndex = r.sel.lang;
    BOXES.forEach(function (n) {
      var lab = $(n), b = r.boxes && r.boxes[n];
      if (!lab || !b) return;
      var inp = lab.querySelector('input');
      lab.style.display = b.visible ? '' : 'none';
      if (inp) { inp.checked = !!b.checked; inp.disabled = !b.enabled; }
    });
    if (r.rgJamLevel) {
      setRadioItems('rgJamLevel', r.rgJamLevel.items);
      var rr = radios('rgJamLevel'); for (var i = 0; i < rr.length; i++) rr[i].checked = (i === r.rgJamLevel.itemIndex);
      $('rgJamLevel').style.display = r.rgJamLevel.visible ? '' : 'none';
    }
    if (r.rgMachineStatusBit8) {
      var rb = radios('rgMachineStatusBit8'); for (var k = 0; k < rb.length; k++) rb[k].checked = (k === r.rgMachineStatusBit8.itemIndex);
      $('rgMachineStatusBit8').style.display = r.rgMachineStatusBit8.visible ? '' : 'none';
    }
    var m = $('labMustCheck_35'); if (m) m.style.display = r.labMustCheck_35 ? '' : 'none';
    var t = $('RichEditJamCode'); if (t) { t.readOnly = true; t.value = rtfToText(r.message || ''); }
    var tab = document.querySelector('.tab[data-t="11"]'); if (tab) tab.style.display = r.statisticsTabVisible ? '' : 'none';
    var who = (r.sel.jamArea || '') + ' / ' + (r.sel.jamCode || '');
    if (r.executed === false) status('未執行：' + (r.guard || '') + ' ' + (r.detail || ''), true);
    else status((r.op || '') + ' OK — ' + who + (r.pageControlVisible === false ? '（此等級 golden 看不到本分頁，不能改）' : ''));
  }
  function values() {
    var v = {};
    BOXES.forEach(function (n) {
      var lab = $(n); if (!lab || lab.style.display === 'none') return;
      var inp = lab.querySelector('input'); if (inp && !inp.disabled) v[n] = !!inp.checked;
    });
    var i1 = radioIndex('rgJamLevel'); if (i1 >= 0 && $('rgJamLevel').style.display !== 'none') v.rgJamLevel = i1;
    var i2 = radioIndex('rgMachineStatusBit8'); if (i2 >= 0 && $('rgMachineStatusBit8').style.display !== 'none') v.rgMachineStatusBit8 = i2;
    return v;
  }
  function run(payload) {
    if (busy) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true; status(payload.op + '…');
    return op(payload).then(function (r) {
      busy = false;
      if (r && r.sel) render(r); else status('失敗：' + ((r && ((r.guard || '') + ' ' + (r.detail || ''))) || ''), true);
      return r;
    }, function (e) { busy = false; status(String(e), true); return { executed: false, guard: 'transport' }; });
  }
  function open() { return run({ op: 'open' }).then(function (r) { if (r && r.sel) opened = true; return r; }); }
  function select(to) {
    if (!opened || !cur) return open();
    return run({ op: 'select', from: cur, to: to, values: values() });
  }
  function save() { if (!opened || !cur) return Promise.resolve({ executed: false, guard: 'not-opened' }); return run({ op: 'save', from: cur, values: values() }); }
  function b64(buf) { var s = '', u = new Uint8Array(buf); for (var i = 0; i < u.length; i += 0x8000) s += String.fromCharCode.apply(null, u.subarray(i, i + 0x8000)); return btoa(s); }
  function importBytes(buf) { if (!opened || !cur) return Promise.resolve({ executed: false, guard: 'not-opened' }); return run({ op: 'import', from: cur, csvBase64: b64(buf) }); }
  function exportCsv(noDownload) {
    if (!opened || !cur) return Promise.resolve({ executed: false, guard: 'not-opened' });
    return run({ op: 'export', from: cur }).then(function (r) {
      if (r && r.executed && r.csvBase64 && !noDownload) {
        var bin = atob(r.csvBase64), u = new Uint8Array(bin.length);
        for (var i = 0; i < bin.length; i++) u[i] = bin.charCodeAt(i);
        var a = document.createElement('a'); a.href = URL.createObjectURL(new Blob([u], { type: 'text/csv' }));
        a.download = r.fileName || 'JamCode.csv'; document.body.appendChild(a); a.click();
        setTimeout(function () { URL.revokeObjectURL(a.href); a.remove(); }, 1000);
      }
      return r;
    });
  }
  function esc(x) { return String(x).replace(/[&<>]/g, function (q) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;' }[q]; }); }
  function stats() {
    return op({ op: 'stats' }).then(function (r) {
      var g = $('sgStatisticsJam');
      if (g && r && r.executed) {
        var h = '<table>';
        (r.rows || []).forEach(function (row, ri) {
          h += '<tr>' + row.map(function (c) { return ri === 0 ? '<th>' + esc(c) + '</th>' : '<td>' + esc(c) + '</td>'; }).join('') + '</tr>';
        });
        g.innerHTML = h + '</table>';
      }
      return r;
    });
  }

  function wire() {
    var a = $('cbJamArea'), c = $('cbJamCode'), l = $('cbJamLang');
    if (a) a.addEventListener('change', function () { select({ area: a.selectedIndex, code: 0, lang: 0 }); });
    if (c) c.addEventListener('change', function () { select({ area: cur ? cur.area : 0, code: c.selectedIndex, lang: 0 }); });
    if (l) l.addEventListener('change', function () { select({ area: cur ? cur.area : 0, code: cur ? cur.code : 0, lang: l.selectedIndex }); });
    var t10 = document.querySelector('.tab[data-t="10"]'); if (t10) t10.addEventListener('click', function () { if (!opened) open(); });
    var t11 = document.querySelector('.tab[data-t="11"]'); if (t11) t11.addEventListener('click', function () { stats(); });
    // Exit：引擎在捕獲階段攔 SecurityExit（levelset 存檔）；Jam 這半在 window 捕獲階段先送（兩者寫不同檔，順序無關）
    window.addEventListener('click', function (ev) { var ex = $('SecurityExit'); if (ex && opened && (ev.target === ex || ex.contains(ev.target))) save(); }, true);
    var fi = document.createElement('input'); fi.type = 'file'; fi.accept = '.csv'; fi.style.display = 'none'; document.body.appendChild(fi);
    fi.addEventListener('change', function () {
      var f = fi.files && fi.files[0]; if (!f) return;
      var rd = new FileReader(); rd.onload = function () { importBytes(rd.result); fi.value = ''; }; rd.readAsArrayBuffer(f);
    });
    var bi = $('spbImport'); if (bi) bi.addEventListener('click', function () { if (!opened) { open(); return; } fi.click(); });
    var be = $('spbExport'); if (be) be.addEventListener('click', function () { if (!opened) { open(); return; } exportCsv(false); });
    var re = $('RichEditJamCode'); if (re) re.readOnly = true;
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', wire); else wire();
  window.HT9045SecurityJam = { open: open, select: select, save: save, importBytes: importBytes, exportCsv: exportCsv, stats: stats,
                               values: values, rtfToText: rtfToText, state: function () { return last; } };
})();
