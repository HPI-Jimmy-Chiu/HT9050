/* ht9045_config_trayplate.js -- Config.Configuration.html（golden TfConfiguration，V912 cConfiguration.cpp）的 Tray／Hot Plate 兩個分頁。
 * ---------------------------------------------------------------------------
 * AI(W906-S98) 20261001 [W906] St01 新檔（手寫）。todo E-003 ①。依據 RULINGS_20260926 S98（這兩張表）＋ S169（Steven 20260928「任何畫面的
 *   事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」）——蓋過 Q41 盤點
 *   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 五、「Configuration 的 Tray／HP 分頁：S98，屬 Q41 的『新頁面先不做』」（S158）。
 *   C++ 那一半：WS cfgtrayplate.op，本體 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp 檔尾（value／回覆／守衛都寫在那裡）。
 *   引擎 ht9045_wire_engine.js、St02 的 ht9045_config_q41.js、St01 的 ht9045_config_st01_ev.js 都不改；頁面 Config.Configuration.html:138 同一行載入。
 *
 * golden（V912 cConfiguration.dfm:21346 tsTrayData／:22005 tsHPData）：上面 pnlTray／pnlHP 五顆鈕，下面 strngrdTray／strngrdHP。
 *   格子沒有 goEditing（不能直接打字）：點一格＝OnSelectCell（:6945／:7083），連點兩下或按 Modify Data 開小鍵盤（:6952→:6957／:7090→:7095），
 *   Add（:6984／:7120）、Delete（:6996／:7132）、Load Data（:7037／:7148）、Save（:7012／:7194）。**沒有確認框**：Save／Delete 按了就做（照 golden）。
 *   HP 的 Save 存完重讀的是 Tray 表（R33，golden 怪處照留）；存一次再讀第 16 欄會掉（R34）。
 *
 * 什麼時候讀（stage F，RULINGS_20260930 第 12 條）：本檔不自己輪詢、不看 tag、開頁不讀。引擎（H4）在視窗真的打開時才送
 *   editlist.get IniConfig（＝golden FormShow；FormShow :4676-4684 已經重讀兩張表）——本檔包 HT9045Recipe.editlistGet，那一次回來
 *   （而且引擎套完值，setTimeout 0）才送一次 cfgtrayplate.op {"op":"get"}。所以跟著開窗邊緣走：嵌在外框、視窗關著＝什麼都不送；
 *   最小化算開著；單獨開頁＝引擎 attach 時讀一次。寫法同 D-022（ht9045_contact_slk.js §20）。另外使用者點 Tray／Hot Plate 頁籤時補一次 get
 *   （golden 切頁不重讀，但這兩張表會被別的頁 FormShow 重讀，畫面要跟著伺服器的表；也拿到切頁上鎖之後的可按不可按）。
 *   開頁時照伺服器的 PageControl1 activePageIndex 程式切頁（golden FormShow :4636-4646：ActivePage=tsConfig，ASE 高雄除外；見 syncTab）。
 *
 * 送什麼（全部 WS cfgtrayplate.op，value＝JSON 字串）：
 *   點格      {"op":"select","table":"tray"|"hp","col":c,"row":r}   （固定列／欄不送：VCL MouseDown 不選固定格）
 *   連點兩下  {"op":"modify","table":…,"step":0,"via":"dblclick"}    Modify Data 鈕 → "via":"button"
 *             回 keypad.open ⇒ 開小鍵盤（qwerty.js HTQwerty.show，規格照伺服器：flags／checkRange／min／max）；OK ⇒ {"step":1,"text":…}、
 *             Abort／✕／點外面 ⇒ {"step":1,"cancel":true}（golden Cancel 也會跑尾段，數字欄會被整理成 golden 的樣子）
 *   Add／Delete／Load Data／Save  {"op":"add"|"delete"|"reload"|"save","table":…}
 *   每個回覆都帶兩張表的快照（cells／fixedRows／fixedCols／colWidths／cursor＝反白格／operable），照它整張重畫。
 * 規則（同 ht9045_config_st01_ev.js）：只收使用者真的點的（isTrusted）；一次一個、等回覆再送下一個；同一個動作還沒回覆又來＝連點，丟掉；
 *   小鍵盤開著時別的都不收（golden 小鍵盤是 modal）；busy:（WebCmdGuard 400 ms）等 450 ms 重送、最多 3 次；not-operator 先續權杖再送一次；
 *   "reload page" ⇒ HT9045Page.load()（引擎重讀 ⇒ 本檔跟著 get）；not-operable ⇒ 說原因、補一次 get（按鈕灰掉）；伺服器沒有這個指令 ⇒ 之後不送。
 *   按鈕可不可按照回覆的 operable（伺服器照 golden FormShow :5381-5382 權限、PageControl1Change :6113 分頁鎖算；伺服器每一次都重查，頁面只是第二道）。
 * 小鍵盤的鍵：qwerty.js 的 N_NO_SYMBOL 把所有符號鍵關掉；golden 的 NO_SYMBOL 鍵盤還打得出 - _ = + [ ] { } ( )
 *   （myQwertyKeyBoard.cpp:79-94 eKeyalphabet，Package Type 像 "QFN-48" 要用）⇒ 開鍵盤後把這幾顆打開（patchKeys；qwerty.js 不改）。
 *   數字欄 dp 送 15：qwerty.js OK 時 toFixed(dp) 會四捨五入，golden 不會（atof→CheckRange→FloatToStr）；伺服器照 golden 再整理一次。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.rawCmd || R.__s98TrayPlate) return;
  R.__s98TrayPlate = true;

  var STRUCT = 'IniConfig';                 // 引擎開頁送的 editlist.get tag（FileRW/IniConfig.cpp）
  var CMD = 'cfgtrayplate.op';
  var LOG = '[Config/S98] ';
  var TABS = {
    tray: { grid: 'strngrdTray', add: 'btnAddTray', del: 'btnDeleteTray', mod: 'btnModifyTray', save: 'sbUpdateTray', load: 'sbtReloadTray', page: 3, label: 'Tray' },
    hp:   { grid: 'strngrdHP',   add: 'btnAddHP',   del: 'btnDeleteHP',   mod: 'btnModifyHP',   save: 'sbUpdateHP',   load: 'sbtReloadHP',   page: 4, label: 'Hot Plate' }
  };
  var GOLDEN = {                             // 狀態列用的 golden 出處
    select: ':6945 / :7083 SelectCell', modify: ':6957 / :7095 Modify Data', add: ':6984 / :7120 Add', 'delete': ':6996 / :7132 Delete',
    reload: ':7037 / :7148 Load Data', save: ':7012 / :7194 Save', get: ''
  };
  var KEY_OK = '-_=+[]{}()';                 // golden NO_SYMBOL 鍵盤打得出來的符號（見檔頭）

  var SNAP = null;                           // 最近一次回覆（{tray, hp}）
  var GEN = 0;                               // 每次引擎開頁讀取 +1：之前送出去的回覆不再套
  var QUEUE = [], INFLIGHT = null, OFF = '', KEYPAD = null, TABTIMER = null;

  function $(id) { return document.getElementById(id); }
  function say(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info(LOG + msg);
  }
  function unwrap(m) {                       // 伺服器把回傳欄位併進 ack；舊寫法放在 value 字串裡 —— 兩種都收
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }

  /* ---- 畫格子 ---------------------------------------------------------- */
  function ensureCss() {
    if (document.getElementById('s98Css')) return;
    var st = document.createElement('style');
    st.id = 's98Css';
    // golden dfm:21372／:22643 Color = 14670284（BGR → #ccd9df）；固定格 FixedColor 預設 clBtnFace；Font->Size=10（:7044）；列高 VCL 預設 24
    st.textContent = '.s98g{border-collapse:collapse;table-layout:fixed;font:13px Arial,sans-serif;}' +
      '.s98g td{border:1px solid #8a8a8a;height:22px;padding:0 3px;white-space:nowrap;overflow:hidden;background:#ccd9df;cursor:default;box-sizing:border-box;}' +
      '.s98g td.fx{background:var(--form-bg,#ece9d8);}' +
      '.s98g td.cur{background:#0a246a;color:#fff;}';
    (document.head || document.documentElement).appendChild(st);
  }
  function render(key) {
    var t = SNAP && SNAP[key], g = $(TABS[key].grid);
    if (!t || !g) return;
    ensureCss();
    var table = document.createElement('table');
    table.className = 's98g';
    var cur = t.cursor || {};
    for (var r = 0; r < t.rowCount; r++) {
      var tr = document.createElement('tr');
      for (var c = 0; c < t.colCount; c++) {
        var td = document.createElement('td');
        var row = t.cells && t.cells[r];
        td.textContent = row && row[c] != null ? String(row[c]) : '';
        td.setAttribute('data-c', String(c));
        td.setAttribute('data-r', String(r));
        var w = (t.colWidths && t.colWidths[c]) || t.defaultColWidth || 64;
        td.style.width = w + 'px';
        td.style.minWidth = w + 'px';
        td.style.maxWidth = w + 'px';
        td.className = (r < t.fixedRows || c < t.fixedCols) ? 'fx' : (r === cur.row && c === cur.col ? 'cur' : '');
        tr.appendChild(td);
      }
      table.appendChild(tr);
    }
    while (g.firstChild) g.removeChild(g.firstChild);
    g.appendChild(table);
    buttons(key);
  }
  function setDis(id, dis) {                 // 只動自己關的（引擎 gbSetEnabled 關的用 data-gb-dis，不碰）
    var el = $(id);
    if (!el) return;
    if (dis) { if (!el.disabled) { el.disabled = true; el.setAttribute('data-s98-dis', '1'); } }
    else if (el.getAttribute('data-s98-dis') === '1') { el.disabled = false; el.removeAttribute('data-s98-dis'); }
  }
  function buttons(key) {
    var T = TABS[key], op = (SNAP && SNAP[key] && SNAP[key].operable) || {}, off = !!OFF;
    setDis(T.add, off || !op.buttons);
    setDis(T.del, off || !op.buttons);
    setDis(T.mod, off || !op.buttons);
    setDis(T.save, off || !op.buttons);
    setDis(T.load, off || !op.reload);
  }
  function apply(a) {
    if (!a || !a.tray || !a.hp) return;
    SNAP = { tray: a.tray, hp: a.hp };
    render('tray');
    render('hp');
  }

  /* ---- 小鍵盤 ---------------------------------------------------------- */
  function patchKeys() {                      // golden NO_SYMBOL 打得出來的符號鍵打開（qwerty.js 大小寫切換會重建，每次點完再補）
    var ov = document.querySelector('.qkOv');
    if (!ov) return;
    var bs = ov.querySelectorAll('.qkQ button');
    Array.prototype.forEach.call(bs, function (b) {
      if (b.disabled && b.textContent.length === 1 && KEY_OK.indexOf(b.textContent) >= 0) {
        b.disabled = false;
        if (b.classList) b.classList.remove('dis');
      }
    });
    if (!ov.__s98) { ov.__s98 = true; ov.addEventListener('click', function () { setTimeout(patchKeys, 0); }, true); }
  }
  function openKeypad(key, kp, gen) {
    var Q = window.HTQwerty;
    if (!Q || typeof Q.show !== 'function') {
      say('Configuration ' + TABS[key].label + '：小鍵盤（qwerty.js）沒有載入 —— 這次當作 Cancel', '#f88');
      enqueue({ op: 'modify', table: key, step: 1, cancel: true, gen: gen });
      return;
    }
    var box = document.createElement('input');   // golden edtTemp（dfm:21989，看不見）
    box.value = kp.current == null ? '' : String(kp.current);
    KEYPAD = { table: key, gen: gen, done: false };
    var finish = function (payload) {
      if (!KEYPAD || KEYPAD.done) return;
      KEYPAD.done = true;
      KEYPAD = null;
      payload.op = 'modify'; payload.table = key; payload.step = 1; payload.gen = gen;
      enqueue(payload);
    };
    Q.show(box, kp.flags, {
      dp: kp.kind === 'double' ? 15 : 0, checkRange: !!kp.checkRange, min: kp.min, max: kp.max,
      onCommit: function (v) { finish({ text: String(v == null ? '' : v) }); },
      onAbort: function () { finish({ cancel: true }); }
    });
    if (kp.kind === 'text') patchKeys();
  }

  /* ---- 佇列 ------------------------------------------------------------ */
  function sameItem(a, b) {
    return a && b && a.op === b.op && a.table === b.table && a.step === b.step && a.col === b.col && a.row === b.row && a.via === b.via;
  }
  function enqueue(item) {
    if (OFF) return false;
    for (var i = 0; i < QUEUE.length; i++) if (sameItem(QUEUE[i], item)) return false;   // 連點
    if (sameItem(INFLIGHT, item) && item.op !== 'get') return false;
    QUEUE.push(item);
    pump();
    return true;
  }
  function pump() {
    if (INFLIGHT || !QUEUE.length) return;
    INFLIGHT = QUEUE.shift();
    var done = function () { INFLIGHT = null; pump(); };
    try { send(INFLIGHT, 0).then(done, done); } catch (x) { if (window.console) console.error(LOG + 'send', x); done(); }
  }
  function payloadOf(item) {
    var v = { op: item.op };
    ['table', 'col', 'row', 'step', 'via', 'text', 'cancel'].forEach(function (k) { if (item[k] !== undefined) v[k] = item[k]; });
    return v;
  }
  function send(item, tries) {
    var extra = { value: JSON.stringify(payloadOf(item)) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd(CMD, extra); }).then(function (m) {
      if (item.gen !== GEN) return null;      // 期間重開過頁：引擎已經重讀、本檔會再 get
      var a = unwrap(m) || {};
      apply(a);
      if (a.error && a.error.text) say('Configuration ' + TABS[item.table || 'tray'].label + '：golden 會跳 VCL 例外框（' + (a.error['class'] || '') + '）：' + a.error.text, '#f88');
      if (a.todo && a.todo.length && window.console) console.info(LOG + item.op + ' todo: ' + a.todo.join(' | '));
      if (item.op === 'modify' && item.step === 0 && a.keypad && a.keypad.open) openKeypad(item.table, a.keypad, item.gen);
      if (item.op === 'save' && a.wrote && !a.error) say('Configuration ' + TABS[item.table].label + '：已照 golden ' + GOLDEN.save + ' 寫入 ' + a.wrote, '#9f9');
      return a;
    }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(item, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(item, tries + 1); }, function () { fail(item, msg); return null; });
      }
      if (/reload page/.test(msg)) {
        QUEUE = [];
        say('Configuration ' + TABS[item.table || 'tray'].label + '：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      if (/unknown cmd/i.test(msg)) { OFF = msg; buttons('tray'); buttons('hp'); }
      fail(item, msg);
      if (/^not-operable/.test(msg)) enqueue({ op: 'get', gen: GEN });   // 按鈕狀態跟上伺服器
      return null;
    });
  }
  function fail(item, msg) {
    var lbl = TABS[item.table || 'tray'].label;
    say('Configuration ' + (item.op === 'get' ? '' : lbl + ' ') + (GOLDEN[item.op] || item.op) + ' 沒有執行（cfgtrayplate.op）：' + msg, '#f88');
  }

  /* ---- 接線 ------------------------------------------------------------ */
  function cellOf(ev) {
    for (var n = ev && ev.target; n && n.getAttribute; n = n.parentNode) {
      if (n.tagName === 'TD' && n.getAttribute('data-c') !== null) return { c: parseInt(n.getAttribute('data-c'), 10), r: parseInt(n.getAttribute('data-r'), 10) };
    }
    return null;
  }
  function onGrid(key, dbl) {
    return function (ev) {
      if (ev && ev.isTrusted === false) return;
      if (KEYPAD || OFF || !SNAP) return;
      var p = cellOf(ev), t = SNAP[key];
      if (!t) return;
      // VCL OnDblClick 是整個格子元件的事件（點在哪一格都一樣，golden :6952 只按 Modify Data）；選哪一格由前面的 mousedown 決定
      if (dbl) { enqueue({ op: 'modify', table: key, step: 0, via: 'dblclick', gen: GEN }); return; }
      if (!p || p.r < t.fixedRows || p.c < t.fixedCols) return;  // VCL：點固定格不選
      enqueue({ op: 'select', table: key, col: p.c, row: p.r, gen: GEN });
    };
  }
  function onButton(key, op) {
    return function (ev) {
      if (ev && ev.isTrusted === false) return;
      if (KEYPAD || OFF) return;
      var el = ev && ev.currentTarget;
      if (el && el.disabled) return;
      var item = { op: op, table: key, gen: GEN };
      if (op === 'modify') { item.step = 0; item.via = 'button'; }
      enqueue(item);
    };
  }
  function onTab(ev) {                        // 使用者點 Tray／Hot Plate 頁籤：等 st01_ev 的 PageControl1 事件先到，再補一次 get
    if (ev && ev.isTrusted === false) return;
    var tb = ev && ev.currentTarget, n = tb ? parseInt(tb.getAttribute('data-t'), 10) : NaN;
    if (n !== TABS.tray.page && n !== TABS.hp.page) return;
    if (TABTIMER !== null) clearTimeout(TABTIMER);
    TABTIMER = setTimeout(function () { TABTIMER = null; if (!KEYPAD) enqueue({ op: 'get', gen: GEN }); }, 300);
  }
  function hook() {
    Object.keys(TABS).forEach(function (key) {
      var T = TABS[key], g = $(T.grid);
      if (g && !g.__s98) {
        g.__s98 = true;
        g.addEventListener('mousedown', onGrid(key, false));
        g.addEventListener('dblclick', onGrid(key, true));
      }
      [['add', 'add'], ['del', 'delete'], ['mod', 'modify'], ['save', 'save'], ['load', 'reload']].forEach(function (b) {
        var el = $(T[b[0]]);
        if (!el || el.__s98) return;
        el.__s98 = true;
        el.addEventListener('click', onButton(key, b[1]));
      });
    });
    var pc = $('PageControl1');
    if (pc && !pc.__s98) {
      pc.__s98 = true;
      Array.prototype.forEach.call(pc.querySelectorAll(':scope > .pcTabs > .tab'), function (tb) { tb.addEventListener('click', onTab); });
    }
  }

  // golden FormShow :4636-4646：非 ASE 高雄 PageControl1->ActivePage=tsConfig（每次開窗都停在 Config 頁）、ASE 高雄停在原頁（Config 頁改 Soft）；
  // C 路 IniConfig.gen.inc:8442 伺服器照算，引擎 C 路 gbApply 不切頁籤 ⇒ 這裡照伺服器算出來的頁切（不寫死 Config）。頁面停在別頁（例：上次開窗停在 Tray，存檔＝golden FormClose 之後重讀）⇒ 伺服器說 Config、畫面是 Tray，
  // 本檔的指令會被 not-on-tab 擋、而且點同一頁籤不會送 PageControl1 事件 ⇒ 開頁時照伺服器的 activePageIndex 程式切頁（isTrusted=false：
  // st01_ev 的切頁事件不送、本檔 onTab 不動）。在 then 裡同步做：排在 st01_ev 開頁 setTimeout（refreshKnown 記頁籤）之前。
  function syncTab(d) {
    var pcx = d && d.proxies && d.proxies.PageControl1, n = pcx && pcx.activePageIndex, pc = $('PageControl1');
    if (typeof n !== 'number' || !pc || !pc.querySelector) return;
    var act = pc.querySelector(':scope > .pcTabs > .tab.act');
    if (act && parseInt(act.getAttribute('data-t'), 10) === n) return;
    var tb = pc.querySelector(':scope > .pcTabs > .tab[data-t="' + n + '"]');
    if (tb && typeof tb.click === 'function') tb.click();
  }

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    var p = get0.apply(this, arguments);
    if (st !== STRUCT || !p || typeof p.then !== 'function') return p;
    return p.then(function (d) {
      GEN++;
      QUEUE = [];
      try { syncTab(d); } catch (e) { if (window.console) console.error(LOG + 'syncTab', e); }
      var gen = GEN;
      setTimeout(function () {                // 引擎在這個 promise 的 then 裡同步套完值（同 ht9045_config_st01_ev.js）
        try { hook(); enqueue({ op: 'get', gen: gen }); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };

  window.HT9045S98TrayPlate = {               // 探針／除錯用
    snap: function () { return SNAP; }, queue: function () { return QUEUE.slice(); }, inflight: function () { return INFLIGHT; },
    keypad: function () { return KEYPAD; }, off: function () { return OFF; }, gen: function () { return GEN; }
  };
})();
