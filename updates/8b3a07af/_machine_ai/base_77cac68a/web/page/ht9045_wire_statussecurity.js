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
 * //AI(W906-FRW-S64) 20260926: 上面這段已過時 —— 0924 W906_SecurityBoot 讓 iMaxLevelItem＝180（查表生效），
 *   S64 解了 SEC-W1/SEC-W2，system.levels.put 改走 golden FormClose→SetLevelSet（見檔尾「權限表存檔（S64）」）。
 *
 * 另一個落差：畫面只有 4 個選項（Operator/Engineer/Supervisor/HonPrec），但
 * golden 在 CosFunction.bSecurityHave5Level 時是 5 階（0..4）。伺服器收 0..4，
 * 所以 5 階機台的值不會被夾壞；但這一頁表達不了第 5 階 —— 讀到 4 的那一格會
 * 被記成 UNFILLABLE 並讓整頁拒寫，而不是靜靜挑一顆選起來。
 * //AI(W906-FRW-S64F) 20260927: 上面這段已處理（Q28=B）—— 檔尾「權限表版面」先問 C++ 版面，照 golden TMySecurity::SetParent
 *   補成 4 或 5 顆、改選項名稱，再向引擎註冊（所以下面的設定物件不再直接 register）。
 */
// //AI(W906-FRW-S64F) 20260927: 不在這裡直接 HT9045Wire.register —— 檔尾「權限表版面」拿到 C++ 版面（radio 顆數）後才註冊，
//   引擎的 levelScan 用「第幾顆」當等級值，顆數要先定好（Q28=B 5 階）。拿不到版面 4 秒後照 HTML 原樣註冊。
window.HT9045SecurityWireCfg = {
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
   *   -> //AI(W906-SEC-S55) 20260926: 已接，見檔尾「改密碼」區塊（WS security.passwd，C++ 在 WebLogin.cpp 檔尾）。
   */
  pending: {
  }
};

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
 *                      //AI(W906-SEC-S54) 20260926: Export 改成 C++ 背景執行緒（op export 立刻回 job → exportStatus 輪詢 → exportChunk 分段取），見 exportCsv 上方說明
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
    else if (!(r.op === 'export' && r.started)) status((r.op || '') + ' OK — ' + who + (r.pageControlVisible === false ? '（此等級 golden 看不到本分頁，不能改）' : ''));
    if (r.export && r.export.state === 'running') watchExport(r.export.job);   //AI(W906-SEC-S54) 20260926: 重新整理頁面後接回進行中的匯出（按鈕停用、續看進度）
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
      if (r && r.sel) render(r);
      else if (window.HT9045Busy && HT9045Busy.is(r)) status(HT9045Busy.NOTE);   // AI(W906-CMDGUARD-UI) 20260926：伺服器 busy: 不是失敗（換掉「…中」，一般顏色）
      else status('失敗：' + ((r && ((r.guard || '') + ' ' + (r.detail || ''))) || ''), true);
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
  /* //AI(W906-SEC-S54) 20260926: 匯出改在 C++ 背景執行緒跑（Steven S54「如果是獨立的 cpu 處理就沒關係」）。
   *   按下 → op export 立刻回 {started, job}（按鈕停用）→ 每秒 op exportStatus（進度）→ done 後 op exportChunk 分段取
   *   （一段 96 KB；整份放一個 ack 會超過伺服器 256 KiB 送出上限、也會超過頁面 15 s 的 ack 逾時）→ 瀏覽器下載 → exportRelease。
   *   進行中用 HT9045Recipe.setTokenHold 登記「不能還權杖」（閒置 30 秒自動還，410d27d9；輪詢要權杖）。
   *   golden 在匯出期間整個畫面卡住（主執行緒跑約 97 秒），這裡畫面照常可用；golden 匯出完選單停在最後一區最後一碼，這裡不跳（C++ 檔頭 S54-1）。 */
  var exp = { job: 0, active: false, timer: null, fails: 0, noDownload: false, done: null };
  if (window.HT9045Recipe && HT9045Recipe.setTokenHold) HT9045Recipe.setTokenHold(function () { return exp.active; });
  function setExportBtn(disabled) {
    var be = $('spbExport'); if (!be) return;
    be.disabled = !!disabled; be.style.opacity = disabled ? '0.5' : '';
    be.title = disabled ? 'spbExport：背景匯出進行中' : 'spbExport : TSpeedButton';
  }
  //AI(W906-SEC-S127) 20260927 (St02): Steven Q5「也可以顯示進度條的方式就好」——匯出／合回進度條（Export 鈕下方，不跳視窗）。
  function progress(v, max, label) {
    var p = $('jamExportProgress');
    if (!p) {
      var pane = document.querySelector('.pcPane[data-p="10"]');
      if (!pane) return;
      p = document.createElement('progress');
      p.id = 'jamExportProgress';
      p.style.cssText = 'position:absolute;left:687px;top:376px;width:143px;height:14px;';
      pane.appendChild(p);
    }
    if (v == null) { p.style.display = 'none'; return; }
    p.style.display = ''; p.max = max > 0 ? max : 1; p.value = Math.min(v, p.max); p.title = label || '';
  }
  function mergeNote(m) {   //AI(W906-SEC-S128) 20260927: 缺鍵合回真的 JAM0000.dat 的結果（C++ export.merge）
    if (!m || m.state === 'none') return '';
    if (m.state === 'done') return '；JAM0000.dat 補回 ' + m.written + ' 個缺鍵' + (m.skipped ? '（' + m.skipped + ' 個已經有了）' : '') +
                                   (m.backup ? '，備份 ' + m.backup : (m.createdFile ? '（原本沒有這個檔）' : ''));
    if (m.state === 'failed') return '；⚠ JAM0000.dat 缺鍵合回失敗、已還原：' + (m.error || '');
    return '；JAM0000.dat 缺鍵合回中 ' + m.applied + ' / ' + m.total;
  }
  function b64ToBytes(s) { var bin = atob(s || ''), u = new Uint8Array(bin.length); for (var i = 0; i < bin.length; i++) u[i] = bin.charCodeAt(i); return u; }
  function finishExport(msg, bad, result) {
    exp.active = false; if (exp.timer) { clearTimeout(exp.timer); exp.timer = null; }
    setExportBtn(false); status(msg, bad); progress(null);
    var d = exp.done; exp.done = null; if (d) d(result || { executed: !bad });
  }
  function fetchChunks(job, total) {
    var parts = [], got = 0;
    function next(off) {
      return op({ op: 'exportChunk', job: job, offset: off }).then(function (r) {
        if (!r || r.executed === false) throw new Error((r && ((r.guard || '') + ' ' + (r.detail || ''))) || 'exportChunk');
        var u = b64ToBytes(r.csvBase64); parts.push(u); got += u.length;
        status('下載匯出結果… ' + got + ' / ' + (r.total || total) + ' bytes');
        return r.last ? r : next(off + u.length);
      });
    }
    return next(0).then(function (r) { return { parts: parts, bytes: got, fileName: r.fileName || 'JamCode.csv' }; });
  }
  function pollExport() {
    exp.timer = null;
    if (!exp.active || !exp.job) return;
    op({ op: 'exportStatus', job: exp.job }).then(function (r) {
      var e = r && r.export;
      if (!r || r.executed === false || !e) { finishExport('匯出狀態讀不到：' + ((r && ((r.guard || '') + ' ' + (r.detail || ''))) || ''), true, r); return; }
      exp.fails = 0;
      if (e.state === 'running') {
        status('匯出中（背景執行，畫面可照常操作）… 區 ' + e.areasDone + ' / ' + e.areas + '、已輸出 ' + e.rows + ' 筆、' + Math.round((e.elapsedMs || 0) / 1000) + ' 秒');
        progress(e.areasDone, e.areas, '匯出 ' + e.areasDone + ' / ' + e.areas + ' 區');
        exp.timer = setTimeout(pollExport, 1000);
        return;
      }
      if (e.state === 'done' && e.merge && (e.merge.state === 'pending' || e.merge.state === 'running')) {   //AI(W906-SEC-S128) 20260927
        status('匯出完成，把快照補的缺鍵合回 JAM0000.dat… ' + e.merge.applied + ' / ' + e.merge.total + (e.merge.error ? '（' + e.merge.error + '）' : ''));
        progress(e.merge.applied, e.merge.total, '合回 ' + e.merge.applied + ' / ' + e.merge.total);
        exp.timer = setTimeout(pollExport, 500);
        return;
      }
      if (e.state !== 'done') { finishExport('匯出' + (e.state === 'failed' ? '失敗：' + (e.error || '') : '已取消（' + e.state + '）'), true, r); return; }
      fetchChunks(exp.job, e.bytes).then(function (res) {
        if (!exp.noDownload) {
          var a = document.createElement('a'); a.href = URL.createObjectURL(new Blob(res.parts, { type: 'text/csv' }));
          a.download = res.fileName; document.body.appendChild(a); a.click();
          setTimeout(function () { URL.revokeObjectURL(a.href); a.remove(); }, 1000);
        }
        var job = exp.job;
        op({ op: 'exportRelease', job: job }).catch(function () {});
        finishExport('匯出完成：' + e.rows + ' 筆、' + res.bytes + ' bytes、' + Math.round((e.elapsedMs || 0) / 1000) + ' 秒（' + res.fileName + '）' + mergeNote(e.merge),
                     !!(e.merge && e.merge.state === 'failed'),
                     { executed: true, op: 'export', job: job, rows: e.rows, bytes: res.bytes, parts: res.parts, merge: e.merge });
      }, function (err) { finishExport('匯出結果下載失敗：' + err.message, true); });
    }, function (err) {
      if (++exp.fails >= 5) { finishExport('匯出狀態連續讀不到：' + ((err && err.message) || err), true); return; }
      exp.timer = setTimeout(pollExport, 2000);
    });
  }
  function watchExport(job) {
    if (!job) return;
    if (exp.active && exp.job === job) return;
    exp.job = job; exp.active = true; exp.fails = 0; setExportBtn(true);
    if (exp.timer) clearTimeout(exp.timer);
    exp.timer = setTimeout(pollExport, 500);
  }
  function exportCsv(noDownload) {
    if (!opened || !cur) return Promise.resolve({ executed: false, guard: 'not-opened' });
    if (exp.active) { status('匯出進行中（job ' + exp.job + '），請等它完成'); return Promise.resolve({ executed: false, guard: 'export-in-progress' }); }
    exp.noDownload = !!noDownload;
    return run({ op: 'export', from: cur }).then(function (r) {
      if (r && r.executed && r.started) {
        status('匯出已開始（背景執行，job ' + r.job + '）');
        return new Promise(function (resolve) { exp.done = resolve; watchExport(r.job); });   // 完成時 resolve（給測試／腳本）
      }
      if (r && r.guard === 'export-in-progress' && r.export && r.export.job) { status('匯出進行中（job ' + r.export.job + '），接著顯示進度'); watchExport(r.export.job); }
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
                               values: values, rtfToText: rtfToText, state: function () { return last; },
                               exportState: function () { return { job: exp.job, active: exp.active }; } };   //AI(W906-SEC-S54) 20260926
})();

/* ===========================================================================
 * 改密碼（Panel1 的 sbSupervisor／sbEngineer／btnHonPrec／btnOperator）—— //AI(W906-SEC-S55) 20260926
 * ---------------------------------------------------------------------------
 * Steven S55「名單可以，密碼需要加密」。golden V912 TfSecurity::ChangePassword（cSecurity.cpp:600-811）＋對話框 TfLogin
 * （login.cpp／login.dfm）。所有判斷與存檔在 C++（WebLogin.cpp 檔尾，WS security.passwd）；這裡只做 TfLogin 的畫面：
 *   state  依登入等級顯示／隱藏四顆鈕（golden FormShow :313-381、:429）；登入等級變了（tag auth.level）就重問
 *   按鈕   op open  ＝ golden 按下到 fLogin->ShowModal() 之前 → 開 TfLogin（元件名照 login.dfm）
 *   sbOk   op apply ＝ ShowModal 之後（Delete 先問 "Sure delete??"，golden :741）
 *   ✕     什麼都不送（golden 按 ✕ 仍會往下跑 —— 刻意差異，同主畫面登入的取消）
 * 密碼：輸入框 type=password、送出後立刻清空；伺服器任何回應都不含密碼（明文或 EncodeStr 後都不帶），這裡也不 console.log。
 * 錯誤碼 WAR*：golden ShowErrorMessage 會停機 → 這裡發不停機告警（比照 Steven 20260924 登入密碼錯誤的裁決）；MES* 只顯示訊息。
 * =========================================================================*/
(function () {
  'use strict';
  function $(id) { return document.getElementById(id); }
  var BTNS = ['btnOperator', 'sbEngineer', 'sbSupervisor', 'btnHonPrec'];
  var ST = null, dlg = null, curBtn = '', curDialog = null;

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
      .then(function () { return HT9045Recipe.rawCmd('security.passwd', { value: JSON.stringify(payload) }); })
      .then(unwrap, parseErr);
  }
  function nonStop(code, msg, detail) {
    var ns = window.HT9045NonStop || (window.parent && window.parent !== window && window.parent.HT9045NonStop);
    if (ns && ns.raise) {
      ns.raise({ code: code, title: msg, titleZh: msg, message: msg, messageZh: msg, unitName: 'System',
                 description: 'golden: TfSecurity::ChangePassword → ShowErrorMessage("' + code + '")（' + (detail || '') + '）\n' +
                              '網頁版改成不停機告警，機台未停機。',
                 hint: '請確認帳號／密碼後重試。' });
      return true;
    }
    return false;
  }
  function note(msg, bad) {
    var s = $('pwStatus');
    if (!s) {
      var p = $('Panel1'); if (!p) return;
      s = document.createElement('span'); s.id = 'pwStatus';
      s.style.cssText = 'position:absolute;left:8px;bottom:1px;font-size:11px;white-space:nowrap;';
      p.appendChild(s);
    }
    s.textContent = msg || ''; s.style.color = bad ? '#b00' : '#234';
  }
  function show(r) {
    ST = r;
    BTNS.forEach(function (n) {
      var b = $(n); if (!b) return;
      var v = !!(r && r.executed && r.buttons && r.buttons[n] && r.buttons[n].visible);
      b.style.display = v ? '' : 'none';                                 // golden Visible=false
    });
    if (r && r.executed && r.allowed === false) note('此等級不足以使用密碼設定（golden Insufficient(29)）', true);
    else if (r && r.executed && r.mode === 'book-binary') note('密碼本是 login.dat（二進位），golden 由 PW_Editor 編輯；這裡只能看名單', false);
  }
  function refresh() { return op({ op: 'state' }).then(function (r) { show(r); return r; }); }

  // ---- TfLogin（login.dfm：ClientWidth 405 × ClientHeight 337；元件座標照 dfm） ----
  function ensureDialog() {
    if (dlg) return dlg;
    var ov = document.createElement('div'); ov.id = 'fLogin';
    ov.style.cssText = 'position:fixed;inset:0;background:rgba(0,0,0,.3);z-index:9000;display:none;align-items:center;justify-content:center;';
    ov.innerHTML =
      '<div style="background:var(--form-bg,#ece9d8);border:1px solid #666;box-shadow:4px 4px 16px rgba(0,0,0,.4);">' +
      ' <div style="background:linear-gradient(90deg,#0a246a,#3a6ea5);color:#fff;font-weight:bold;font-size:12px;padding:3px 6px;display:flex;align-items:center;">' +
      '  <span id="fLoginCaption">Login</span><span id="fLoginClose" title="關閉（不送出）" style="margin-left:auto;cursor:pointer;background:#d4d0c8;color:#000;border:1px solid #999;width:16px;text-align:center;">✕</span></div>' +
      ' <div style="position:relative;width:405px;height:337px;font-family:\'MS Sans Serif\',sans-serif;">' +
      '  <label id="labUserName" style="position:absolute;left:24px;top:24px;font-size:24px;font-weight:bold;">Username</label>' +
      '  <input id="edUserName" autocomplete="off" list="cbLoginUserName_items" style="position:absolute;left:224px;top:16px;width:169px;height:45px;font-size:22px;box-sizing:border-box;">' +
      '  <datalist id="cbLoginUserName_items"></datalist>' +
      '  <label id="labOldPassword" style="position:absolute;left:24px;top:80px;font-size:24px;font-weight:bold;">Password</label>' +
      '  <input id="edLoginOldPassword" type="password" autocomplete="new-password" style="position:absolute;left:224px;top:72px;width:169px;height:45px;font-size:22px;box-sizing:border-box;">' +
      '  <label id="labNewPassword" style="position:absolute;left:24px;top:136px;font-size:24px;font-weight:bold;">NewPassword</label>' +
      '  <input id="edLoginNewPassword" type="password" autocomplete="new-password" style="position:absolute;left:224px;top:128px;width:169px;height:45px;font-size:22px;box-sizing:border-box;">' +
      '  <fieldset id="rgLoginOption" style="position:absolute;left:32px;top:192px;width:337px;height:65px;box-sizing:border-box;margin:0;font-size:15px;font-weight:bold;">' +
      '   <legend>Option Mode</legend>' +
      '   <label><input type="radio" name="rgLoginOption" value="0" checked>New</label>&nbsp;&nbsp;' +
      '   <label><input type="radio" name="rgLoginOption" value="1">Delete</label>&nbsp;&nbsp;' +
      '   <label><input type="radio" name="rgLoginOption" value="2">Edit</label></fieldset>' +
      '  <button id="sbOk" class="btn3d" style="position:absolute;left:120px;top:268px;width:145px;height:49px;font-size:18px;font-weight:bold;">OK</button>' +
      '  <div id="w906UserList" title="名單（帳號／等級；S55：名單可以，不含密碼）" style="position:absolute;left:4px;top:318px;right:4px;font-size:11px;color:#234;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;"></div>' +
      ' </div></div>';
    document.body.appendChild(ov);
    dlg = ov;
    $('fLoginClose').addEventListener('click', closeDialog);
    ov.querySelectorAll('input[name=rgLoginOption]').forEach(function (r) { r.addEventListener('change', function () { optionClick(); }); });
    $('sbOk').addEventListener('click', ok);
    // 小鍵盤（qwerty.js）：點輸入框開 TfQwertyKey 模擬（密碼欄遮罩）
    ['edUserName', 'edLoginOldPassword', 'edLoginNewPassword'].forEach(function (id) {
      var el = $(id);
      el.addEventListener('dblclick', function () {
        if (!window.HTQwerty) return;
        HTQwerty.show(el, id === 'edUserName' ? HTQwerty.N.NO_SPACE : (HTQwerty.N.NO_SPACE | HTQwerty.N.PASSWORD));
      });
    });
    return dlg;
  }
  function vis(id, v) { var e = $(id); if (e) e.style.display = v ? '' : 'none'; }
  function option() { var r = dlg.querySelector('input[name=rgLoginOption]:checked'); return r ? +r.value : 0; }
  // golden rgLoginOptionClick（login.cpp:77-101）：Delete 藏舊密碼；Edit 顯示新密碼、標籤 OldPassword（密碼本模式才有 rgLoginOption）
  function optionClick() {
    if (!curDialog || !curDialog.rgLoginOption || !curDialog.cbLoginUserName.visible) return;
    var i = option();
    vis('labOldPassword', i !== 1); vis('edLoginOldPassword', i !== 1);
    vis('labNewPassword', i === 2); vis('edLoginNewPassword', i === 2);
    $('labOldPassword').textContent = i === 2 ? 'OldPassword' : 'Password';
  }
  function clearPw() { ['edLoginOldPassword', 'edLoginNewPassword'].forEach(function (id) { var e = $(id); if (e) e.value = ''; }); }
  function closeDialog() { if (!dlg) return; clearPw(); dlg.style.display = 'none'; curDialog = null; curBtn = ''; }
  function openDialog(r) {
    ensureDialog();
    var d = r.dialog; curDialog = d;
    $('fLoginCaption').textContent = (d.caption || 'Login') + ' — ' + curBtn;
    $('edUserName').value = ''; clearPw();                               // golden FormShow login.cpp:22-24
    var dl = $('cbLoginUserName_items'); dl.innerHTML = '';
    (d.cbLoginUserName.items || []).forEach(function (n) { var o = document.createElement('option'); o.value = n; dl.appendChild(o); });
    vis('labUserName', d.labUserName); vis('edUserName', d.edUserName || d.cbLoginUserName.visible);
    vis('rgLoginOption', d.rgLoginOption.visible);
    var rr = dlg.querySelectorAll('input[name=rgLoginOption]'); for (var i = 0; i < rr.length; i++) rr[i].checked = (i === d.rgLoginOption.itemIndex);
    vis('labOldPassword', d.labOldPassword.visible); $('labOldPassword').textContent = d.labOldPassword.caption || 'Password';
    vis('edLoginOldPassword', d.edLoginOldPassword);
    vis('labNewPassword', d.labNewPassword); vis('edLoginNewPassword', d.edLoginNewPassword);
    var names = (d.cbLoginUserName.items || []);
    $('w906UserList').textContent = d.cbLoginUserName.visible ? ('名單（等級 ' + r.level + '）：' + (names.length ? names.join('、') : '（無）')) : '';
    dlg.style.display = 'flex';
    var f = d.cbLoginUserName.visible ? $('edUserName') : $('edLoginOldPassword'); if (f) f.focus();
  }
  function click(name) {
    curBtn = name;
    op({ op: 'open', button: name }).then(function (r) {
      if (!r) return;
      if (r.executed === false) {
        if (r.guard === 'binary-book') {
          var u = (r.users || []).map(function (x) { return x.name; });
          note('login.dat 是二進位密碼本，golden 這顆鈕改不到（由 PW_Editor 編輯）。等級 ' + r.level + ' 名單：' + (u.length ? u.join('、') : '（無）'), false);
        } else note('未執行：' + (r.guard || r.code || '') + ' ' + (r.detail || r.message || ''), true);
        return;
      }
      if (r.result === 'noop') { note('golden：沒有密碼本時 HonPrec 這顆不動作（cSecurity.cpp:797-798）', false); return; }
      if (r.result === 'secret-converted') { note(r.message || 'Password Secret finish!!', false); return; }   // golden ShowMyMessage
      if (r.dialog) openDialog(r);
    });
  }
  function ok() {
    if (!curDialog) return;
    var opt = curDialog.rgLoginOption.visible || curDialog.cbLoginUserName.visible ? option() : 0;
    var confirmDelete = false;
    if (curDialog.cbLoginUserName.visible && opt === 1) confirmDelete = window.confirm('Sure delete??');   // golden :741 MessageBox YES/NO（NO 也照 golden 送出，C++ 不刪但照樣存檔）
    var payload = { op: 'apply', button: curBtn, option: opt, user: $('edUserName').value,
                    oldPassword: $('edLoginOldPassword').value, newPassword: $('edLoginNewPassword').value, confirmDelete: confirmDelete };
    closeDialog();                                                        // 先清掉畫面上的密碼（payload 已取值）
    op(payload).then(function (r) {
      payload = null;
      if (!r) return;
      if (window.HT9045Busy && HT9045Busy.is(r)) { note(HT9045Busy.NOTE, false); refresh(); return; }   // AI(W906-CMDGUARD-UI) 20260926：busy: 不是失敗，不發告警
      var code = r.code || r.result || r.guard || '';
      var msg = (r.message || r.detail || '') + (r.user ? '（' + r.user + '）' : '');
      if (r.alarm || /^WAR/.test(code)) { nonStop(code, r.message || code, r.goldenLine); note(code + ' ' + msg, true); }
      else if (r.executed === false) note('未執行：' + code + ' ' + msg, true);
      else note((code === 'saved' ? '已存檔' : code) + ' ' + msg, false);
      refresh();
    });
  }

  function wire() {
    BTNS.forEach(function (n) { var b = $(n); if (b) b.addEventListener('click', function () { click(n); }); });
    refresh();
    var lastLevel;                                                        // snapshot（重連）會把沒變的 tag 也送一次 —— 只在真的變了才動作
    if (window.HT9045Tags && HT9045Tags.on) HT9045Tags.on('auth.level', function (v) {
      if (v === lastLevel) return;
      var first = (lastLevel === undefined); lastLevel = v;
      refresh(); if (!first && dlg && dlg.style.display !== 'none') closeDialog();
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', wire); else wire();
  window.HT9045SecurityPasswd = { state: refresh, list: function () { return op({ op: 'list' }); }, click: click,
                                  apply: function (p) { p.op = 'apply'; return op(p); } };   // 測試／腳本用；回應不含密碼
})();

/* ===========================================================================
 * 權限表存檔（S64 levelset.dat）—— //AI(W906-FRW-S64) 20260926（Steven 團隊）
 * ---------------------------------------------------------------------------
 * 存檔：Exit → 引擎送 WS system.levels.put（稀疏：只送改過的格子）→ C++ WebLevelSet.cpp 照 golden V912
 *   TfSecurity::FormClose（cSecurity.cpp:439-468）的順序：GetLevelSet（:263 起始值）→ 套網頁值（:441-446）→ 三條鉗制
 *   （:448-458：[87]≥Supervisor、[129]≤[130]、非矽格北興 [86]≥Engineer）→ SaveJamLevel（:463，用新的 [35]）→ SetLevelSet（:465，
 *   整塊 1024 bytes）。所以 Jam 分頁那半（上面 op save）與權限表這半誰先送都一樣：[35] 對 Jam 等級的下限由 C++ 在 SetLevelSet 前補跑。
 *   ack 的 clamped／normalized 會列出被 golden 改掉的格子；存完引擎會重讀檔案，畫面就是 golden 寫下的值。
 * 這一段只做畫面：golden FormShow :275-278 對 [163]（Contact Count Alarm Edit Permission）一律 SetEnabled(false)
 *   （值由 GetLevelSet :1497-1500 強制 3）→ 網頁同樣停用那一組 radio。
 *   CC_KYEC_LEE 的 [35]/[104]/[114]/[128]（:267-274）屬客戶專屬（S25 先跳過）；C++ 端仍會擋（改了會整批拒寫）。
 *   //AI(W906-FRW-S64F) 20260927: KYEC 那四格現在由檔尾「權限表版面」照 C++ 回的 disabled[] 停用；C++ 改成只擋那一格、
 *   其餘照存（Q26=B）。這一段留著當版面拿不到時的保底（[163] 在 golden 是無條件停用）。
 * =========================================================================*/
(function () {
  'use strict';
  var GOLDEN_DISABLED = [163];
  function mark() {
    var all = document.getElementsByTagName('*');
    for (var i = 0; i < all.length; i++) {
      var el = all[i];
      if (!el.getAttribute) continue;
      var t = el.getAttribute('data-htitle') || el.getAttribute('title') || '';   // theme.js 在 release 模式把 title 搬到 data-htitle（見引擎 levelScan 的說明）
      if (t.indexOf('TMySecurity') < 0) continue;
      var m = /\[(\d+)\]/.exec(t);
      if (!m || GOLDEN_DISABLED.indexOf(parseInt(m[1], 10)) < 0) continue;
      var rs = el.querySelectorAll('input[type="radio"]');
      for (var k = 0; k < rs.length; k++) rs[k].disabled = true;
      var fs = el.querySelector('fieldset');
      if (fs) fs.setAttribute('data-golden-disabled', 'cSecurity.cpp:275-278 SetEnabled(false)');
    }
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', mark); else mark();
})();

/* ===========================================================================
 * 權限表版面（Q28=B）＋改不到的格子提示（Q26=B）—— //AI(W906-FRW-S64F) 20260927（Steven 團隊）
 * ---------------------------------------------------------------------------
 * Steven 20260927 對 todo ★ Q26／Q28 的裁決（RULINGS_20260926 S146／S148）。判斷全部在 C++（WebLevelSet.cpp），這裡只排畫面：
 *
 * (1) 開頁先問 C++ 版面：WS system.levels.put {"op":"layout"}（唯讀；不讀檔進記憶體、不寫任何東西），拿到 golden 算出來的
 *       items               TMySecurity::SetParent（V912 cSecurity.cpp:861-903）每組 radio 的選項：4 階 Operator/Engineer/Supervisor/HonPrec；
 *                           bSecurityHave5Level 5 階 Open/Operator/Engineer/Supervisor/HonPrec（CC_KYEC_LEE：Operator/Engineer/PEngineer/Supervisor/HonPrec）
 *       hidden              FormShow :285-287 `#ifndef SOFT_SIMULTE SecurityPalVisible()`（:471-572）隱藏的格子（模擬組態是空的）
 *       disabled            FormShow :265-278 SetEnabled(false)：[163] 一律、CC_KYEC_LEE 的 35/104/114/128
 *       pageControlVisible  FormShow :302-381 依登入等級：看不到＝整張權限表（含 Jam 兩頁）都不顯示
 *       allowed             main.cpp:28592 Insufficient(29)（不夠 golden 根本開不了這張表單）
 *       pitch／start        FormShow :383-415 每個 ScrollBox 內「看得到的」面板依建立順序重排：Top = 2、54、106 …（每格 52）
 *     照這些把 radio 補成 4 或 5 顆、改選項文字、藏格子、停用、重排，**然後才**向引擎註冊（HT9045Wire.register）——
 *     引擎的 levelScan 以 radio 的「第幾顆」當等級值，所以顆數必須在引擎掃描之前定好（5 階機台的 4 才填得進第 5 顆）。
 *     4 秒內拿不到版面（沒連上 wb_serve 等）就照 HTML 原樣（4 顆、全部顯示）註冊，並在畫面說明；C++ 存檔時仍照 golden 擋格子。
 *     版面晚到、而顆數跟已註冊的不同（5 階機台）→ 不改顆數，提示重新整理頁面。
 *     登入等級變了（tag auth.level）→ 重問版面，只重套 PageControl1 可見性（其餘跟等級無關）。
 *
 * (2) Q26=B：C++ 存檔時把 golden 畫面改不到的格子（停用／隱藏／這個等級看不到）擋下、其餘照存，ack 帶 blocked[]。
 *     這裡包一層 HT9045System.levels（只在本頁；不動 ht9045_recipe_client.js／ht9045_wire_engine.js）：
 *       預演（dryRun）回來有 blocked → 先跳一個說明框（在引擎的「即將寫入 N 個值」確認框之前），並在頁面底部留一條提示；
 *       真的寫完 → 提示改成「已存檔；以下 N 格沒存（維持原值）」。
 *     例：[87] Main - Teaching 那格停用，沒存（選的是 3 HonPrec，維持 2 Supervisor）。
 *
 * 防連點：這一段沒有新增按鈕（存檔仍是引擎攔的 SecurityExit）；版面查詢遇到伺服器 busy: 只重試一次。
 * =========================================================================*/
(function () {
  'use strict';
  var CFG = window.HT9045SecurityWireCfg;
  var S = { layout: null, registered: false, fallback: false, radios: 0, panels: null, lastMsg: '', lastBad: false };
  var LV4 = ['Operator', 'Engineer', 'Supervisor', 'HonPrec'];   // HTML 原樣（＝golden 4 階 SetParent :886-890）
  function $(id) { return document.getElementById(id); }

  // ---- 面板掃描（同引擎 levelScan 的認法：data-htitle 或 title 裡的 TMySecurity［NN］） ----
  function panels() {
    if (S.panels) return S.panels;
    var out = [], all = document.getElementsByTagName('*');
    for (var i = 0; i < all.length; i++) {
      var el = all[i];
      if (!el.getAttribute) continue;
      var t = el.getAttribute('data-htitle') || el.getAttribute('title') || '';
      if (t.indexOf('TMySecurity') < 0) continue;
      var m = /\[(\d+)\]\s*([^）)]*)/.exec(t);
      if (!m) continue;
      var fs = el.querySelector('fieldset');
      if (!fs) continue;
      out.push({ idx: parseInt(m[1], 10), caption: '[' + m[1] + '] ' + (m[2] || '').trim(), el: el, fs: fs });
    }
    S.panels = out;
    return out;
  }
  function caption(idx) {
    var p = panels();
    for (var i = 0; i < p.length; i++) if (p[i].idx === idx) return p[i].caption;
    return '[' + idx + ']';
  }
  function labelsOf(fs) {
    var r = [], ls = fs.querySelectorAll('label');
    for (var i = 0; i < ls.length; i++) if (ls[i].querySelector('input[type="radio"]')) r.push(ls[i]);
    return r;
  }
  function setLabelText(lab, text) {
    for (var n = lab.firstChild; n; n = n.nextSibling) {
      if (n.nodeType === 3) { n.nodeValue = text; return; }
    }
    lab.appendChild(document.createTextNode(text));
  }
  function levelName(v) {
    var it = (S.layout && S.layout.items) || LV4;
    return (v >= 0 && v < it.length) ? (v + ' ' + it[v]) : String(v);
  }

  // ---- 狀態顯示（頁面自己的兩個元素；引擎的狀態列 #ht9045WireBar 會被引擎自己的訊息蓋掉，所以不用它） ----
  var STICKY = { msg: '', bad: false };                            // 狀態型提示（檔案不對、顆數對不上、版面沒拿到）：事件提示不會把它清掉
  function note(msg, bad, sticky) {
    if (sticky) { STICKY.msg = msg || ''; STICKY.bad = !!bad; msg = S.lastMsg || ''; bad = S.lastBad; }
    else { S.lastMsg = msg || ''; S.lastBad = !!bad; }
    var text = [STICKY.msg, msg || ''].filter(function (x) { return !!x; }).join('\n');
    var s = $('lvStatus');
    if (!s) {
      var form = document.querySelector('.form'); if (!form) return;
      s = document.createElement('div'); s.id = 'lvStatus';
      s.style.cssText = 'position:absolute;left:0;right:0;top:788px;height:40px;z-index:6;box-sizing:border-box;padding:2px 22px 2px 6px;' +
                        'font:12px/1.35 "Microsoft JhengHei",sans-serif;white-space:pre-wrap;overflow:auto;border:1px solid #c9a200;background:#fff4c2;display:none;';
      var x = document.createElement('span'); x.textContent = '×'; x.title = '關閉提示';
      x.style.cssText = 'position:absolute;right:6px;top:1px;cursor:pointer;color:#665;font:14px sans-serif;';
      x.addEventListener('click', function () { s.style.display = 'none'; });
      s.appendChild(x); s.appendChild(document.createElement('span'));
      form.appendChild(s);
    }
    s.lastChild.textContent = text;
    s.style.color = (bad || STICKY.bad) ? '#8a0000' : '#333';
    s.style.display = text ? '' : 'none';
  }
  function gate(msg) {                                           // PageControl1 看不到時，原位置的說明
    var g = $('lvGate');
    if (!g) {
      var form = document.querySelector('.form'); if (!form) return;
      g = document.createElement('div'); g.id = 'lvGate';
      g.style.cssText = 'position:absolute;left:40px;right:40px;top:180px;z-index:5;padding:16px;border:1px solid #999;background:#fff;' +
                        'font:14px/1.6 "Microsoft JhengHei",sans-serif;color:#333;white-space:pre-wrap;display:none;';
      form.appendChild(g);
    }
    g.textContent = msg || '';
    g.style.display = msg ? '' : 'none';
  }

  // ---- 版面 ----
  function fetchLayout(retried) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.resolve({ error: 'no-client' });
    return HT9045Recipe.rawCmd('control.acquire').catch(function () {})
      .then(function () { return HT9045Recipe.rawCmd('system.levels.put', { tag: 'levelset', value: JSON.stringify({ op: 'layout' }) }); })
      .then(function (a) { return a; }, function (e) {
        if (!retried && window.HT9045Busy && HT9045Busy.is(e)) {
          return new Promise(function (r) { setTimeout(r, 600); }).then(function () { return fetchLayout(true); });
        }
        return { error: (e && e.message) || String(e) };
      });
  }
  function applyRadios(items) {                                  // TMySecurity::SetParent：選項數＝items.length，第 k 顆＝等級 k
    var n = items.length, p = panels();
    p.forEach(function (x) {
      var ls = labelsOf(x.fs);
      if (!ls.length) return;
      while (ls.length < n) {
        var c = ls[ls.length - 1].cloneNode(true);
        var ci = c.querySelector('input'); if (ci) { ci.checked = false; ci.removeAttribute('checked'); }
        x.fs.appendChild(c); ls.push(c);
      }
      while (ls.length > n) { var d = ls.pop(); d.parentNode.removeChild(d); }
      ls.forEach(function (l, k) { setLabelText(l, items[k]); });
      x.fs.setAttribute('data-golden-columns', String(n));
    });
    S.radios = n;
  }
  function applyPal(lay) {
    var hid = {}, dis = {};
    (lay.hidden || []).forEach(function (i) { hid[i] = true; });
    (lay.disabled || []).forEach(function (i) { dis[i] = true; });
    var groups = [];
    panels().forEach(function (x) {
      x.el.style.display = hid[x.idx] ? 'none' : '';
      x.el.setAttribute('data-golden-visible', hid[x.idx] ? 'false' : 'true');
      var rs = x.fs.querySelectorAll('input[type="radio"]');
      for (var k = 0; k < rs.length; k++) rs[k].disabled = !!dis[x.idx];
      if (dis[x.idx]) x.fs.setAttribute('data-golden-disabled', 'cSecurity.cpp:265-278 SetEnabled(false)');
      else x.fs.removeAttribute('data-golden-disabled');
      var par = x.el.parentNode, g = null;
      for (var j = 0; j < groups.length; j++) if (groups[j].par === par) g = groups[j];
      if (!g) { g = { par: par, list: [] }; groups.push(g); }
      g.list.push(x);
    });
    // golden FormShow :383-415：每個 ScrollBox 捲回頂端，看得到的面板依 Controls[] 順序（＝建立順序＝索引遞增）Top=2 起每格 52
    var pitch = lay.pitch || 52, start = (typeof lay.start === 'number') ? lay.start : 2;
    groups.forEach(function (g) {
      g.list.sort(function (a, b) { return a.idx - b.idx; });
      var top = start;
      g.list.forEach(function (x) { if (!hid[x.idx]) { x.el.style.top = top + 'px'; top += pitch; } });
      if (g.par) g.par.scrollTop = 0;
    });
  }
  function applyGate(lay) {
    var pc = $('PageControl1');
    var ok = lay.allowed !== false && lay.pageControlVisible !== false;
    if (pc) pc.style.display = ok ? '' : 'none';
    if (ok) { gate(''); return; }
    gate(lay.allowed === false
      ? '登入等級不夠開這張表（golden 主畫面 sbPasswordClick：Insufficient(29)，需要 AccessLevel ≥ ' + lay.level29 + '，目前 ' + lay.accessLevel + '）。\n權限表不顯示、存檔會被 C++ 拒絕。'
      : '這個登入等級（AccessLevel=' + lay.accessLevel + '）golden 看不到權限表（FormShow cSecurity.cpp:302-381 PageControl1->Visible=false）。\n' +
        '上面的改密碼按鈕照 golden 顯示；權限表與 Jam 分頁不顯示，改了也不會存（C++ 會擋下）。');
  }
  function apply(lay) {
    S.layout = lay;
    var items = lay.items || LV4, warn = [];
    if (!S.registered) applyRadios(items);
    else {
      // 已經照 HTML 原樣註冊了（版面晚到）：顆數不能再改（引擎已掃過），但選項文字一定要跟 C++ 的等級對上 ——
      // 5 階機台第 0 顆是 Open 不是 Operator，文字不改的話選「HonPrec」實際存的是 3（Supervisor）。
      panels().forEach(function (x) { labelsOf(x.fs).forEach(function (l, k) { if (k < items.length) setLabelText(l, items[k]); }); });
      if (S.radios && items.length !== S.radios)
        warn.push('這台機器是 ' + items.length + ' 階（golden bSecurityHave5Level），畫面載入時還沒拿到版面、先照 ' + S.radios +
                  ' 顆 radio 接上了（選項文字已改成 golden 的名稱，但缺「' + items[items.length - 1] + '」那一顆）。請重新整理頁面（F5）再改權限表。');
    }
    applyPal(lay);
    applyGate(lay);
    if (lay.fileOk === false)
      warn.push('levelset.dat ' + (lay.fileExists ? '大小 ' + lay.fileBytes + ' bytes，不是 golden 的 1024 bytes' : '不存在') +
                '（' + lay.path + '）：存檔會被拒絕（Steven 20260927 Q29=A）。');
    note(warn.join('\n'), warn.length > 0, true);
  }
  function registerOnce() {
    if (S.registered || !CFG || !window.HT9045Wire) return;
    S.registered = true;
    if (!S.radios) S.radios = LV4.length;
    HT9045Wire.register(CFG);
  }
  function refresh() { return fetchLayout(false).then(function (lay) { if (lay && lay.op === 'layout') apply(lay); return lay; }); }
  function start() {
    var timer = setTimeout(function () {
      if (S.registered) return;
      S.fallback = true;
      registerOnce();
      note('4 秒內沒拿到 C++ 的權限表版面（wb_serve 沒連上？），先照 HTML 原樣（4 階、全部顯示）接上。存檔時 C++ 仍照 golden 擋停用／隱藏的格子。', true, true);
    }, 4000);
    fetchLayout(false).then(function (lay) {
      clearTimeout(timer);
      if (lay && lay.op === 'layout') apply(lay);
      else if (!S.registered) note('拿不到權限表版面：' + ((lay && lay.error) || '') + '；先照 HTML 原樣（4 階、全部顯示）接上。存檔時 C++ 仍照 golden 擋停用／隱藏的格子。', true, true);
      registerOnce();
    });
    var lastLevel;                                                // snapshot（重連）會把沒變的 tag 也送一次 —— 只在真的變了才重問
    if (window.HT9045Tags && HT9045Tags.on) HT9045Tags.on('auth.level', function (v) {
      if (v === lastLevel) return;
      var first = (lastLevel === undefined); lastLevel = v;
      if (!first) refresh();
    });
  }

  // ---- Q26=B：包 HT9045System.levels，看 ack 的 blocked[] ----
  function reasonText(b) {
    if (b.reason === 'disabled') return '那格停用（golden FormShow SetEnabled(false)）';
    if (b.reason === 'hidden') return '那格在這台機器的 golden 畫面上是隱藏的（SecurityPalVisible）';
    if (b.reason === 'pageControl') return '這個登入等級 golden 看不到權限表（PageControl1）';
    return '那格改不到（' + b.reason + '）';
  }
  function describe(b) {
    return caption(b.idx) + ' ' + reasonText(b) + '，沒存（選的是 ' + levelName(b.sent) + '，維持 ' + levelName(b.kept) + '）';
  }
  function onAck(ack, opts) {
    var bl = (ack && ack.blocked) || [];
    if (!bl.length) { if (!opts.dryRun) note('', false); return; }
    var lines = bl.map(describe);
    if (opts.dryRun) {
      var head = '以下 ' + bl.length + ' 格 golden 畫面上改不到，這次不會存（維持原值）；其他改動照存：';
      note(head + '\n' + lines.join('\n'), true);
      window.alert(head + '\n\n' + lines.join('\n') +
                   '\n\n（接下來的確認視窗若仍列出這幾格，它們不會被寫入；伺服器回報的「N 個值」已經不含它們）');
    } else {
      note('已存檔；以下 ' + bl.length + ' 格沒存（維持原值）：\n' + lines.join('\n'), true);
    }
  }
  function wrapLevels() {
    var api = window.HT9045System;
    if (!api || typeof api.levels !== 'function' || api.levels.__s64f) return;
    var orig = api.levels;
    var w = function (name, edits, opts) {
      var o = opts || {};
      return orig.apply(api, arguments).then(function (ack) { try { onAck(ack, o); } catch (e) {} return ack; });
    };
    w.__s64f = true;
    api.levels = w;
  }

  // 引擎還沒註冊（等版面，最多 4 秒）時按 Exit：引擎的捕獲攔截還沒掛上，按鈕自己的 .exitbtn 處理器會直接關視窗（不存檔）。
  // 這段期間先攔下來、提示稍候（golden 的 Exit＝關表單＋存檔，不能變成只關不存）。註冊之後這裡不做事，交給引擎。
  document.addEventListener('click', function (ev) {
    if (S.registered) return;
    var ex = $('SecurityExit'), t = ev.target;
    if (!ex || !(t === ex || (ex.contains && ex.contains(t)))) return;
    ev.stopPropagation(); ev.preventDefault();
    note('權限表還在載入（等 C++ 版面），這一下沒有動作；請稍候再按 Exit。', false);
  }, true);

  wrapLevels();
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
  window.HT9045SecurityLayout = { state: function () { return S; }, refresh: refresh, panels: panels, describe: describe };   // 測試／探針用
})();
