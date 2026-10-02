//Steven 20260915
// ----------------------------------------------------------------------
// 新檔。全站共用的接線引擎：配方讀寫、機台設定檔讀寫（sysFields）、QWERTY 小鍵盤。
// physicalKeys() 實作「機台上沒有實體鍵盤」規格：畫面 readonly，只有小鍵盤開著時實體鍵才有效。
// ⚠ 狀態列停靠 left:614px，絕不可用 bottom:0——會蓋住 Setup 頁底部的 Save/Exit。
// 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
// ----------------------------------------------------------------------

/* ht9045_wire_engine.js -- 所有 Setup 頁共用的接線引擎
 * ---------------------------------------------------------------------------
 * AI(W906-FW-ENGINE) 20260914。
 *
 * 為什麼是共用引擎而不是每頁一份：load/save 的三條規則完全一樣，複製 N 份
 * 等於一個 bug 要改 N 次（狀態列蓋住 Save 鈕那個坑已經踩過兩次）。
 * 每頁只提供「資料」，行為由這裡統一。
 *
 * 用法（在 HTML 的 </body> 之前，順序不可顛倒）：
 *   <script src="qwerty.js"></script>
 *   <script src="ht9045_recipe_client.js"></script>
 *   <script src="ht9045_wire_engine.js"></script>
 *   <script src="ht9045_wire_<page>.js"></script>
 *
 * 每頁資料檔呼叫 HT9045Wire.register({page, slug, fields, pending, kb})
 *   fields  { id: [doc, section, key] }        只放「所有配方都有」的鍵
 *   pending { id: [doc, section, key, 原因] }  抽到了但不安全 / 需人工確認
 *   kb      { id: [FLAG, dp, checkRange, min, max] }   golden ShowQwertyKey
 *
 * 三條規則（來自 ht9045_contact_wire.js，不要破壞）
 *   1. 寫入前一定 preview，把 changed 顯示給操作員看，由人確認。
 *   2. notFound 不是空的就拒絕寫入 -- 那代表對照表有錯，不是資料有錯。
 *   3. 寫完一定重讀回填。
 * 互鎖不要寫在這裡。該擋的由 C++ 端擋；兩層互鎖會互相遮蔽。
 *
 * 版面：狀態列停在表單右側，絕對不要用 bottom:0 -- 這些頁的 Save / Exit
 * 多半在底部 Panel，橫跨底部的固定列會把它們整個蓋掉。
 */
(function () {
  'use strict';

  var CFG = null, KBN = 0, LAST = {};   // LAST[doc] = 上次讀到的 sections
  var TAGN = 0;                          // Steven 20260916：接上執行期 tag 的元素數

  function $(id) { return document.getElementById(id); }

  function bar() {
    var b = $('ht9045WireBar');
    if (b) return b;
    b = document.createElement('div');
    b.id = 'ht9045WireBar';
    b.style.cssText = 'position:fixed;left:614px;right:8px;top:8px;z-index:99999;' +
      'font:12px/1.5 monospace;padding:6px 24px 6px 10px;background:#222;color:#ddd;' +
      'border:1px solid #555;border-radius:4px;max-height:60vh;overflow:auto;' +
      'white-space:pre-wrap;min-width:260px';
    var x = document.createElement('span');
    x.textContent = '×';
    x.title = '隱藏';
    x.style.cssText = 'position:absolute;right:6px;top:2px;cursor:pointer;color:#888;font:14px sans-serif';
    x.addEventListener('click', function () { b.style.display = 'none'; });
    b.appendChild(x);
    document.body.appendChild(b);
    return b;
  }

  /* ---------------------------------------------------------------------------
   * 狀態列的顯示政策            //Steven 20260921
   * ---------------------------------------------------------------------------
   * 使用者回報：開頁時右上角那塊黑底訊息（「讀取完成：填入 39 / 43 個欄位 ...」）
   * 要自己關掉，而且只在 debug 模式出現。
   *
   * 但這條狀態列同時在講兩種完全不同的事，不能一視同仁地關掉：
   *
   *   startup   開頁的接線自我報告（填了幾個、缺了幾個、存檔鈕是哪一顆…）。
   *             **開發用**，操作員看了也不能做什麼 -> 只在 debug 顯示，
   *             AUTOHIDE_MS 之後自動收起來。
   *   transient 動作成功的回覆（✅ 寫入完成、沒有任何值改變、已取消…）。
   *             兩種模式都要顯示，但看過就好 -> 自動收起來。
   *   sticky    ❌ 拒絕寫入 / 讀取失敗 / ⚠ 警告，以及「寫入中 ...」這種進行中的
   *             訊息。**預設值**，兩種模式都顯示，而且不自動消失。
   *
   * ⚠ 不要把 sticky 也關掉。「❌ 拒絕寫入：有 N 個鍵在目標檔裡不存在」如果只在
   *   debug 顯示，操作員在機台上按了存檔、畫面什麼都沒說，他會以為存進去了。
   *   那比看到一塊黑框糟得多。
   *
   * debug 的判斷用 <html data-mode> —— theme.js 依網址的 ?mode= 在解析階段就設好，
   * 而 theme.js 在每一頁都排在本檔之前（PAGE_TMPL 固定順序），所以這裡讀得到。
   * background.html 的 withMode() 會把 ?mode= 帶進每一個 iframe。
   * 沒有 ?mode= 就是 release —— 單獨開一頁來看時狀態列不會跳出來。
   *
   * ⚠ 倒數計時器掛在 **元素上**（b.__htHideTimer），不是模組變數。
   *   #ht9045WireBar 這個 id 不只本檔會建 —— ht9045_contact_wire.js（那份獨立的
   *   參考實作）也建同一個 id，Setup.Contact.html 兩支都載入。計時器各放各的
   *   模組變數的話，本檔開頁那則 startup 的倒數會在 6 秒後把對方稍晚貼上來的
   *   「❌ 拒絕寫入」一起收掉。掛在元素上，誰貼新訊息誰就取消掉前一個倒數，
   *   兩支才不會互相關掉對方的訊息。
   */
  var AUTOHIDE_MS = 6000;

  function isDebug() {
    try {
      return document.documentElement.getAttribute('data-mode') === 'debug';
    } catch (e) { return false; }       // 極端情況（沒有 documentElement）當 release
  }

  // AI(W906-SCREEN-TOKEN) 20261001（RULINGS_20261001，Jimmy 1001 的截圖「C 路讀取失敗（editlist.get TestIF_File_YieldMonitoring）：control-held」）：
  //   伺服器的權杖錯誤原樣印出來看不懂。control-held／not-operator ＝ 操作權在另一條連線（另一個畫面或分頁）——一個畫面裡的視窗共用一條連線、不會互搶
  //   （ht9045_link.js），所以補一行白話與怎麼辦；原本的錯誤字串照留（探針與 log 認它）。
  function tokenHint(msg) {
    return /^(control-held|not-operator)$/.test(String(msg || '')) ?
      '\n   → 操作權在另一個畫面（較新開啟的 HMI 視窗或另一個瀏覽器分頁），這個畫面現在只能看。要在這裡操作：在這個畫面按一下存檔、馬達或 IO 輸出這類操作按鈕（會把操作權拿回來），或關掉另一個畫面；之後再按重讀。' : '';
  }

  function say(msg, colour, kind) {
    kind = kind || 'sticky';
    if (kind === 'startup' && !isDebug()) return msg;   // 連 bar 都不建

    var b = bar();
    b.style.display = '';
    b.style.color = colour || '#ddd';
    var t = b.firstChild;
    if (!t || t.nodeType !== 3) { t = document.createTextNode(''); b.insertBefore(t, b.firstChild); }
    t.nodeValue = msg;

    if (b.__htHideTimer) { clearTimeout(b.__htHideTimer); b.__htHideTimer = null; }
    if (kind === 'startup' || kind === 'transient') {
      b.__htHideTimer = setTimeout(function () {
        b.__htHideTimer = null;
        b.style.display = 'none';
      }, AUTOHIDE_MS);
    }
    return msg;
  }

  function byDoc(map) {
    var g = {};
    Object.keys(map).forEach(function (id) {
      var d = map[id][0];
      (g[d] = g[d] || []).push(id);
    });
    return g;
  }

  /* -------------------------------------------------------------------------
   * 系統檔（system\ 與 config\）—— wb_serve 20260915 的 /api/system/
   * -------------------------------------------------------------------------
   *   sysFields { id: [file, section, key] }   ini：gerneral / teach / config
   *   sysRows   { id: [file, rowKey, column] } csv：motTable / ioTable
   * 兩者與配方欄位共用同一套 preview→確認→write→重讀流程。
   * 與配方的差別只有 API 端點與 payload 形狀，規則完全一樣。
   */
  /* Steven 20260916
   * -------------------------------------------------------------------------
   * 非文字控制項（sysEnums）
   * -------------------------------------------------------------------------
   * 起因：HW.HandlerSys 有 705 個輸入框，其中只有 35 個是文字框 ——
   * 626 個 radio、46 個 select、44 個 checkbox。引擎原本只會 `el.value = ...`，
   * 所以那一頁「有對照表也接不上」。Config.Configuration（662 個 checkbox）與
   * Status.Security（722 個 radio）卡在同一件事上。
   *
   *   sysEnums { id: [file, section, key, kind] }   kind: 'index' | 'bool'
   *
   * golden 的對應關係（已對 HTML 實體結構查證）：
   *   TRadioGroup -> <fieldset id=X> 內含順序排列的 input[type=radio]，
   *                  ItemIndex = 第幾個被選中                       kind='index'
   *   TComboBox   -> <select id=X>，ItemIndex = selectedIndex       kind='index'
   *   TCheckBox   -> <label id=X><input type="checkbox">，Checked   kind='bool'
   *
   * ⚠ id 掛在 fieldset / label 這層外框上，真正的 input 是沒有 id 的子節點。
   *   直接對 el 取 .value / .checked 會拿到 undefined 而且不會拋錯 ——
   *   存檔時就寫出一整排 "undefined"。所以一律走 ctlGet / ctlSet。
   *
   * 值一律轉成字串後交給既有的 preview -> 確認 -> 寫入 -> 重讀流程，
   * 與文字欄位共用同一套三條規則，不另開寫入路徑。
   */
  function radios(el) {
    return el ? el.querySelectorAll('input[type="radio"]') : [];
  }
  function checkbox(el) {
    if (!el) return null;
    return el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]');
  }

  function ctlGet(el, kind) {
    if (!el) return null;
    if (kind === 'bool') {
      var c = checkbox(el);
      return c ? (c.checked ? '1' : '0') : null;
    }
    if (kind === 'index') {
      if (el.tagName === 'SELECT') return String(el.selectedIndex);
      var rs = radios(el);
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return String(i);
      return '-1';                       // golden 的 ItemIndex 未選取就是 -1
    }
    return el.value;
  }

  function ctlSet(el, kind, v) {
    if (!el) return false;
    if (kind === 'bool') {
      var c = checkbox(el);
      if (!c) return false;
      // golden 這些鍵有寫 0/1 也有寫 true/false，兩種都要認得。
      var s = String(v).trim().toLowerCase();
      c.checked = (s === '1' || s === 'true' || s === 'yes');
      return true;
    }
    if (kind === 'index') {
      var n = parseInt(v, 10);
      if (isNaN(n)) return false;
      // -1 是 VCL ItemIndex 的「未選取」，是合法值不是缺陷。
      // Gerneral.ini 目前就有兩個鍵是 -1（ROTATE_KIT 的 iRotate_In_Index /
      // iRotate_Out_Index）。把它當成錯誤回報會在每次讀取都跳假警告，
      // 而真正的越界（值 >= 選項數）就淹沒在裡面看不到了。
      if (el.tagName === 'SELECT') {
        if (n < -1 || n >= el.options.length) return false;
        el.selectedIndex = n;
        return true;
      }
      var rs = radios(el);
      if (!rs.length) return false;
      // Steven 20260916（審查 B1）：先驗再改 DOM。原本是先把全部取消勾選再回 false，
      // 控制項因此停在「沒有任何選項」，之後 Save 就把 -1 寫回去。
      if (!(n === -1 || (n >= 0 && n < rs.length))) return false;
      for (var i = 0; i < rs.length; i++) rs[i].checked = (i === n);
      return true;
    }
    el.value = v;
    return true;
  }

  /* Steven 20260916
   * 一律用 cell.raw，不要用 cell.value。  （AI(W906-RECIPE-BCB-R13) 20260926：顯示改用 cell.bcb＝BCB6 讀到的字；沒動過的欄位存檔時仍送 raw，見 unshown()）
   *
   * GET 回來的每個欄位有兩種形式：
   *     {"value": 0, "type": "float", "raw": "0.000000"}
   * raw 是檔案裡的字面值，value 是解析過的數字。寫入只認 raw
   *（wb_serve.cpp：「Only `raw` is honoured because only `raw` is lossless」）。
   *
   * 原本這裡填的是 cell.value，於是讀 "0.000000" 填成 "0"，存檔再以 raw 送出，
   * 就把檔案改成了 "0"。實測：HW.teach 送回剛讀出的值，伺服器仍回報 changed=8；
   * HandlerSys changed=1。也就是操作員什麼都沒改、只按了存檔，就會改掉 9 個
   * 機台設定值 —— 而且畫面顯示成功。配方欄位走的是同一段程式，同樣受影響。
   */
  function cellText(cell) {
    if (cell === null || cell === undefined) return '';
    if (typeof cell !== 'object') return String(cell);
    if (cell.bcb !== undefined && cell.bcb !== null) return String(cell.bcb); if (cell.raw !== undefined && cell.raw !== null) return String(cell.raw);   // AI(W906-RECIPE-BCB-R13) 20260926: 顯示 BCB6 讀到的字（第 13 條）；寫回由 collect／collectSysIni 的 unshown() 換回 raw
    return String(cell.value);
  }  function unshown(cell, v) { return (cell && typeof cell === 'object' && cell.bcb !== undefined && cell.bcb !== null && cell.raw !== undefined && cell.raw !== null && v === String(cell.bcb)) ? String(cell.raw) : v; }   // AI(W906-RECIPE-BCB-R13) 20260926: 畫面值等於讀到的 bcb（操作員沒動）就送回原本的 raw → 檔案位元組不變、預覽 changed 不變

  function sysEntry(id) {
    return (CFG.sysFields && CFG.sysFields[id]) || (CFG.sysEnums && CFG.sysEnums[id]) || null;
  }
  function sysKind(id) {
    return (CFG.sysEnums && CFG.sysEnums[id]) ? (CFG.sysEnums[id][3] || 'index') : 'text';
  }

  function sysIniByFile() {
    var g = {};
    [CFG.sysFields || {}, CFG.sysEnums || {}].forEach(function (m) {
      Object.keys(m).forEach(function (id) { (g[m[id][0]] = g[m[id][0]] || []).push(id); });
    });
    return g;
  }
  function sysCsvByFile() {
    var g = {}, m = CFG.sysRows || {};
    Object.keys(m).forEach(function (id) { (g[m[id][0]] = g[m[id][0]] || []).push(id); });
    // Steven 20260916：表格模式的檔也算一個 csv 目標，讓 save() 的 preview/寫入
    // 迴圈自然把它涵蓋進來（沒有 widget id，所以陣列是空的）。
    if (CFG.sysGrid && CFG.sysGrid.file && !g[CFG.sysGrid.file]) g[CFG.sysGrid.file] = [];
    return g;
  }
  /* -------------------------------------------------------------------------
   * sysLevels —— 二進位定長陣列（Status.Security 的 levelset.dat）
   * -------------------------------------------------------------------------
   * //Steven 20260916 (W906-FW-LEVELSET)
   *
   * 為什麼不能用 sysEnums：
   *   sysEnums 的三元組是 (檔, 區段, 鍵)，而 levelset.dat 沒有鍵名、沒有表頭，
   *   只有 256 個小端 int32；第 NN 格就是 AccessLevel[NN]。伺服器那側是
   *   /api/system/levelset 的 {kind:'i32', values:[...]} 投影 ＋ system.levels.put。
   *
   * 為什麼對照表是「執行期從 DOM 掃出來」而不是產生器產一張靜態表：
   *   這一頁的 radio **完全沒有 id**（722 個，0 個帶 id），連外框 fieldset 也沒有，
   *   所以 sysEnums 那條「靠 id 取容器」的路走不通。而索引本身就寫在 DOM 裡：
   *       title="MySecurity_Panel_N : TMySecurity（[NN] Main - Tools）"
   *   其中 [NN] 才是 AccessLevel 的下標。
   *
   *   ⚠ N 與 NN 不一樣，這是這一頁最容易靜默寫錯的地方。
   *     交付包那份 HTML 的 MySecurity_Panel_14 對應的是 [32]；
   *     群組名 sec_sbTools_0 對應的是 [14]、sec_sbTools_29 對應的是 [178]。
   *     `_<n>` 是「該 scrollbox 內的第幾個」，不是下標。用它當索引會把 179 個
   *     權限全部寫到錯的格子，而且因為值域都是 0..3，寫進去不會報錯。
   *     （機台那份 D:\HT9045\page\ 另外帶了 data-security-index，兩份一致；
   *      但交付包那份沒有，所以一律只認 title 的 [NN]，兩份才能共用同一支接線。）
   *
   * 自我驗證：掃完之後檢查「組數 == 期望值、索引唯一、落在 0..count-1」，
   * 對不上就整頁拒接並大聲說為什麼 —— 靜默接一半比完全不接更糟。
   */
  var LV = null;   // { file, expect, binds:[{idx, inputs:[el..], caption}], bad:[...] }

  function levelScan() {
    if (LV) return LV;
    var cfg = CFG.sysLevels;
    if (!cfg || !cfg.file) return null;
    LV = { file: cfg.file, expect: cfg.expect || 0, binds: [], bad: [] };
    var seen = {};
    /* Steven 20260916 —— 這一段的兩個坑，都是實測出來的，不要「簡化」回去
     *
     * (1) 不能只讀 title，一定要先讀 data-htitle。
     *     theme.js:14-24 在 release 模式（沒帶 ?mode= 就是 release）於
     *     DOMContentLoaded 把每個元素的 title 搬到 data-htitle 再 removeAttribute
     *     ——「去掉 hover 露出的 cpp/dfm 名，但保留值供程式判斷」。theme.js 在頁面
     *     裡比接線檔早載入，所以它的 DOMContentLoaded listener 也先跑；等
     *     attach() 執行時，整頁一個 title 都不剩。
     *     第一版只讀 title，於是在 2332 個元素裡掃到 0 個面板。
     *     兩個都讀才能同時在 release 與 ?mode=debug 下成立。
     *
     * (2) 用 JS 過濾而不是 [title*="TMySecurity"] 屬性選擇器。
     *     選擇器要寫成兩條（title 與 data-htitle）才等價，而且下次再多一個
     *     搬名字的人就又漏一條。走訪一次全頁比對字串沒有這個問題，
     *     這一頁 2332 個元素也不值得為此省。
     */
    var all = document.getElementsByTagName('*');
    var panels = [];
    for (var pi = 0; pi < all.length; pi++) {
      var el = all[pi];
      if (!el.getAttribute) continue;
      var ti = el.getAttribute('data-htitle') || el.getAttribute('title');
      if (ti && ti.indexOf('TMySecurity') >= 0) panels.push(el);
    }
    LV.scanned = all.length;
    LV.titled = panels.length;
    panels.forEach(function (p) {
      var t = p.getAttribute('data-htitle') || p.getAttribute('title') || '';
      var m = /\[(\d+)\]\s*([^）)]*)/.exec(t);
      if (!m) { LV.bad.push('title 解不出 [NN]：' + t); return; }
      var idx = parseInt(m[1], 10);
      var ins = p.querySelectorAll('input[type="radio"]');
      if (!ins.length) { LV.bad.push('[' + idx + '] 面板內沒有 radio'); return; }
      if (seen[idx]) { LV.bad.push('索引 [' + idx + '] 重複出現'); return; }
      seen[idx] = true;
      LV.binds.push({ idx: idx, inputs: Array.prototype.slice.call(ins), caption: (m[2] || '').trim() });
    });
    return LV;
  }

  /* -------------------------------------------------------------------------
   * tags —— 執行期唯讀顯示（snapshot / patch）
   * -------------------------------------------------------------------------
   * //Steven 20260916 (W906-FW-TAGSUB)
   *
   *   tags: { '<tag 名>': ['<元素 id>', '<形狀>', <小數位>] }
   *   形狀： 'text'（預設，寫 textContent）
   *
   * 與 fields/sysFields 的差別：那些是「檔案 ←→ 畫面」的讀寫欄位；這裡是
   * 「機台狀態 → 畫面」的單向顯示，沒有存檔、沒有 dryRun、不進 save()。
   *
   * ⚠ null 要顯示成 "---"，不是空白也不是 0。
   *   伺服器對「不可知」送 null（WebBridge/TagSnapshot.h section 4 rule 3），
   *   而 0 是合法的實值。把 null 顯示成 0 會讓人看著一個編造的數字做決定。
   *   wb_serve 目前 117 個 tag 只有 9 個有來源，所以畫面上大量 "---" 是實話。
   *
   * ⚠ 這些欄位在 main.html 原本是由 settings.js 的 HT_SETTINGS 廣播（JSON
   *   Simulator 那條路）填的。tag 每 500ms 會覆蓋回去，兩者會互相打架。
   *   既定政策是資料一律走 wb_serve，所以 tag 是權威；但 HT_SETTINGS 那段
   *   仍在頁面裡，移除它不在本次範圍。
   */
  /* Steven 20260918 (W906-FW-OBSERVER)
   * -------------------------------------------------------------------------
   * showText —— 把一段文字寫到「畫面上看得到的地方」
   * -------------------------------------------------------------------------
   * golden 的 TPanel 顯示的是 Caption。產生器把 TPanel 匯成
   *   <div class="pnl" id="labModel"><span class="pnlCap">...</span></div>
   * 而 .pnlCap 是絕對定位置中的那一層（見各頁 <style> 的 .pnlCap 規則）。
   *
   * ⚠ 對 .pnl 這個 div 直接寫 textContent 會**把 .pnlCap 整個刪掉**，文字改貼在
   *   左上角、失去置中與字色 —— 而且看起來像「樣式壞了」而不是「接錯地方」。
   *   所以 .pnl 一律寫進它的 .pnlCap（沒有就補一個），其餘元素維持原本的
   *   textContent 行為（實測全樹已接 text/map 的元素沒有一個是 .pnl，
   *   所以這一段對既有頁面是零行為變更）。
   */
  function showText(el, text) {
    if (!el) return;
    if (el.classList && el.classList.contains('pnl')) {
      var cap = el.querySelector(':scope > .pnlCap');
      if (!cap) {
        cap = document.createElement('span');
        cap.className = 'pnlCap';
        el.appendChild(cap);
      }
      // 這個 class 讓值用 Arial 9px 白字顯示（頁面 CSS 已有規則），
      // 與 golden TPanel 的 Font 設定一致，也和標題字重區分開。
      cap.classList.add('runtimePanelValue');
      cap.textContent = text;
      return;
    }
    el.textContent = text;
  }

  var TAGSUB = [];

  function tagFmt(v, dp) {
    if (v === null || v === undefined) return '---';
    if (typeof v === 'number' && typeof dp === 'number') return v.toFixed(dp);
    return String(v);
  }

  /* Steven 20260918 (W906-FW-TAGSUB2)
   * -------------------------------------------------------------------------
   * tag 綁定的模式 —— spec[1]
   * -------------------------------------------------------------------------
   *   'text'  (預設)  el.textContent = tagFmt(v, spec[2])
   *   'chip'          主畫面狀態小方塊：true -> .on-g／false -> .off／null -> .unknown
   *   'site'          SitePanel(mtDutOnOff) 的格子；引擎自己從 tag 名算座標
   *   'select'        <select>：選中對應的 option
   *   'led'           .aled 元件：true -> .on／false -> 熄／null -> .unknown
   *   'map'           spec[3] 查表把值換成人看得懂的字，查不到退回原值
   *   function        直接呼叫 fn(v, el)（產生器吐不出函式，手寫接線檔才用得到）
   *
   * ⚠ 為什麼每個非文字模式都一定要有第三種狀態（unknown）：
   *   布林 tag 的 null 是「不可知」，不是「關」。只用 classList.toggle('on', !!v)
   *   的話，斷線的燈和機台真的關著的燈**長得一模一樣** —— 沒有紅字、沒有 "---"，
   *   操作員看著一顆熄掉的燈以為安全。這正是本檔 patch 那段註解在講的
   *   「看起來完全正常」的壞法；文字欄位用 "---" 解掉了，燈號與色塊不能漏掉。
   *   127 個非 pci1203 的 tag 目前只有 58 個有值（20260918 實測），unknown 會很常見。
   */

  /* 'site' 模式的定址 —— 權威來源有兩處，而且兩處一致：
   *   tagmap.js:96          "XItem=8 YItem=4 -> col=(site-1)%8, row=(arm-1)*2+(site>8)"
   *   WebBridgeTags.cpp:783 "sites 1..8 = row 0, 9..16 = row 1, col=(n-1)%8"
   * 一個 TTMyTray（mtDutOnOff）裝下 2 臂 × 16 site，所以 32 個 tag 定址的是
   * **同一個控制項的格子**，不是 32 個控制項。
   */
  function siteAddr(tag) {
    var m = /^site\.arm(\d+)\.s(\d+)$/.exec(tag || '');
    if (!m) return null;
    var arm = parseInt(m[1], 10), site = parseInt(m[2], 10);
    if (!(arm >= 1 && site >= 1 && site <= 16)) return null;
    return { arm: arm, x: (site - 1) % 8, y: (arm - 1) * 2 + (site > 8 ? 1 : 0) };
  }

  /* Steven 20260918 (W906-FW-OBSERVER)
   * text / map 模式的 title。null 與有值要分得開 —— '---' 本身不說明原因。
   * ⚠ 覆寫 title 不會弄丟 dfm 的元件說明：產生器把它另存在 data-htitle，
   *   頁面的 tooltip 走的是那一個（實測 <div ... data-htitle="labModel : TPanel">）。
   */
  function tagTitle(el, tag, v) {
    if (!el) return;
    el.title = tag + '：執行期 tag（wb_serve 唯讀顯示，不進存檔）。' +
               ((v === null || v === undefined)
                  ? '目前不可知 —— 伺服器對這個 tag 送的是 null（不是 0、不是空字串）。'
                  : '目前有值。');
  }

  function tagApply(el, spec, v, tag) {
    var mode = spec[1] || 'text';
    if (typeof mode === 'function') { mode(v, el); return; }
    // Steven 20260924：golden TComboBox 的 ->Text（例 user.level → cbUserSelect）。元素是 <select> 時，
    // 寫 textContent 會把選項清光 —— 改成選對應的選項（同 'select' 模式）。
    if (mode === 'text' && el.tagName === 'SELECT') mode = 'select';

    // 主畫面的狀態小方塊（FT / RT / OFF Line / Normal / Prime）。
    // golden 是 TPanel 靠 Color 表示狀態，網頁沿用既有的 .chip 樣式。
    if (mode === 'chip') {
      var un = (v === null || v === undefined);
      el.classList.remove('on-g', 'on-r', 'off', 'unknown');
      el.classList.add(un ? 'unknown' : (v ? 'on-g' : 'off'));
      el.title = (spec[3] || el.getAttribute('data-chipcap') || '') +
                 (un ? '（不可知）' : (v ? '（ON）' : '（OFF）'));
      return;
    }

    if (mode === 'site') {
      var a = siteAddr(tag);
      if (!a) return;
      var cell = el.querySelector('.cell[data-x="' + a.x + '"][data-y="' + a.y + '"]');
      if (!cell) return;
      var unk = (v === null || v === undefined);
      // 綠＝這個 site 有在用，灰＝關掉，斜線紋＝不可知。
      cell.style.background = unk ? '' : (v ? '#00c000' : '#c0c0c0');
      if (cell.classList) cell.classList.toggle('cellUnknown', unk);
      cell.title = 'site.arm' + a.arm + '  (col ' + a.x + ', row ' + a.y + ')：' +
                   (unk ? '不可知' : (v ? '使用中' : '關閉'));
      return;
    }

    // golden 是 TComboBox 的欄位（startmode.value）。tag 目前是唯讀顯示，
    // 但保留 <select> 是為了日後的寫入通道，所以選中對應的 option 而不是
    // 把它換成純文字。
    // ⚠ 值不在選項裡時**補一個 option 並選它**，不是靜默不動 —— 靜默不動會
    //   顯示上一個選項，那是一個看起來很正常的錯值。
    if (mode === 'select') {
      if (v === null || v === undefined) { el.selectedIndex = -1; el.title = '不可知'; return; }
      var s = String(v), hit = -1, i;
      for (i = 0; i < el.options.length; i++) {
        if (el.options[i].value === s || el.options[i].text === s) { hit = i; break; }
      }
      if (hit < 0) {
        var op = document.createElement('option');
        op.text = s; op.value = s;
        op.setAttribute('data-fromtag', '1');
        el.appendChild(op);
        hit = el.options.length - 1;
        el.title = '機台回報的值不在設計時的選單裡：' + s;
      } else {
        el.title = '';
      }
      el.selectedIndex = hit;
      // IDL property 之外把 selected 這個 content attribute 也設上，理由同
      // 本檔 lvCheck() 那段：只設 property 的話 --dump-dom 類的驗收工具看到的
      // 仍是「載入前」的選項，而且看起來完全正常。
      for (i = 0; i < el.options.length; i++) {
        if (i === hit) el.options[i].setAttribute('selected', 'selected');
        else el.options[i].removeAttribute('selected');
      }
      return;
    }

    if (mode === 'led') {
      var unknown = (v === null || v === undefined);
      if (el.classList) {
        el.classList.toggle('on', !unknown && !!v);
        el.classList.toggle('unknown', unknown);
      }
      // title 一起更新，滑過去才知道這顆燈現在到底是哪一種狀態。
      var base = spec[3] || el.getAttribute('data-ledcap') || '';
      el.title = (base ? base + '：' : '') +
                 (unknown ? '不可知（沒有來源或尚未連上）' : (v ? 'ON' : 'OFF'));
      return;
    }

    if (mode === 'map') {
      var tbl = spec[3] || {};   // 產生器把查表當第 4 個元素吐出來
      tagTitle(el, tag, v);
      if (v === null || v === undefined) { showText(el, '---'); return; }
      showText(el, (Object.prototype.hasOwnProperty.call(tbl, String(v)))
                   ? tbl[String(v)] : tagFmt(v, spec[2]));
      return;
    }

    // Steven 20260918：走 showText 而不是直接 textContent —— TPanel 匯出的
    // <div class="pnl"> 要寫進子層的 .pnlCap，否則值會出現但樣式全失。
    tagTitle(el, tag, v);
    showText(el, tagFmt(v, spec[2]));
  }

  function attachTags() {
    var m = CFG.tags || {}, names = Object.keys(m);
    if (!names.length) return 0;
    if (typeof HT9045Tags === 'undefined') {
      say('❌ 這一頁要接執行期 tag，但沒有載入 ht9045_recipe_client.js（HT9045Tags 不存在）。', '#f88');
      return 0;
    }
    var bound = 0, missing = [];
    names.forEach(function (tag) {
      var spec = m[tag], el = $(spec[0]);
      if (!el) { missing.push(tag + ' -> #' + spec[0]); return; }
      bound++;
      TAGSUB.push(HT9045Tags.on(tag, function (v) {
        tagApply(el, spec, v, tag);
      }));
      // 還沒收到任何訊框之前先寫成「不可知」，不要讓 HTML 裡寫死的假值留在畫面上。
      // ⚠ 走 tagApply(null) 而不是直接寫 '---'：燈號與色塊要的是 class，
      //    對一個 <span class="aled"> 寫 textContent 沒有任何效果，那顆燈會
      //    一直停在 dfm 匯出時的樣子 —— 又是一個「看起來完全正常」的假象。
      if (!HT9045Tags.has(tag)) tagApply(el, spec, null, tag);
      else tagApply(el, spec, HT9045Tags.get(tag), tag);
    });
    if (missing.length) {
      say('⚠ 這些 tag 在頁面上找不到對應元素（' + missing.length + '）：' + missing.join(', '), '#ffcc66', 'startup');
    }
    HT9045Tags.connect().catch(function (e) {
      say('⚠ 執行期資料連不上（' + e.message + '）—— 畫面上的狀態值會停在 "---"。', '#ffcc66');
    });
    return bound;
  }

  //Steven 20260916
  // 設選取狀態時，property 與 content attribute 一起設。
  // 只設 r.checked（IDL property）畫面是對的，但 outerHTML 序列化出來的仍是
  // 原始 HTML 寫死的那顆 —— 也就是說 --dump-dom 類的驗收工具會看到「載入前」
  // 的畫面，而且看起來完全正常。這個盲點害 security_smoke.py 第一版連續兩次
  // 報 34/179 假失敗。兩個一起設，DOM 說的就等於畫面顯示的。
  function lvCheck(r, on) {
    r.checked = on;
    if (on) r.setAttribute('checked', 'checked');
    else r.removeAttribute('checked');
  }

  function levelCaption(idx) {
    var lv = levelScan(), out = '';
    if (!lv) return out;
    lv.binds.forEach(function (b) { if (String(b.idx) === String(idx)) out = b.caption; });
    return out;
  }

  // 傳回 null 表示對照表本身有問題，呼叫端要整頁拒絕動作。
  function levelCheck() {
    var lv = levelScan();
    if (!lv) return null;
    var why = lv.bad.slice();
    if (lv.expect && lv.binds.length !== lv.expect) {
      why.push('掃到 ' + lv.binds.length + ' 組，期望 ' + lv.expect + ' 組');
    }
    if (!lv.binds.length) {
      // Steven 20260916：診斷數字要跟著錯誤一起出來。只說「掃到 0 組」無法分辨
      // 是「選擇器寫錯」「頁面還沒建好」還是「這根本不是那一頁」。
      why.push('一組都沒掃到（走訪 ' + (lv.scanned || 0) + ' 個元素，其中 title 含 TMySecurity 的有 ' +
               (lv.titled || 0) + ' 個）');
    }
    return why.length ? why : [];
  }

  /* Steven 20260918 (W906-FW-OBSERVER)
   * -------------------------------------------------------------------------
   * sysText —— 機台設定檔的唯讀顯示
   * -------------------------------------------------------------------------
   *   sysText { id: [file, section, key] }
   *
   * 讀的是跟 sysFields 同一個 /api/system/<file>，但：
   *   - 只寫畫面（showText），不碰 .value；
   *   - **不進 sysIniByFile()**，所以 save() / collectSysIni() 看不到它，
   *     這一頁不會因為接了幾個顯示欄位就變成可寫；
   *   - 不進 hasSave 判斷，狀態列那句「這一頁沒有可用的存檔鈕，資料唯讀」照樣會出現。
   *
   * 為什麼不直接用 sysFields：Data.Observer 在 golden 對這些元素只有 ->Caption=
   * 的指派，沒有任何寫回；而且它們是 TPanel（div），sysFields 的 ctlSet 走
   * `el.value = v`，對 div 是掛一個看不見的 JS 屬性 —— 畫面完全沒變，
   * 而且看起來完全正常。
   */
  function sysTextByFile() {
    var g = {}, m = CFG.sysText || {};
    Object.keys(m).forEach(function (id) { (g[m[id][0]] = g[m[id][0]] || []).push(id); });
    return g;
  }

  /* Steven 20260918 (W906-FW-OBSERVER)
   * -------------------------------------------------------------------------
   * noSource —— 「這一格沒有來源」要自己說出來
   * -------------------------------------------------------------------------
   *   noSource { id: ['<golden 來源>', '<為什麼 wb_serve 接不到>'] }
   *
   * 寫 '---' 並把兩句話放進 title。三件刻意的事：
   *  1. **'---' 不是 0、不是空白。** 0 是合法實值；空白看起來像還沒載入完。
   *  2. **在 attach 階段就寫**，不等任何網路往返 —— HTML 裡原本寫死的展示值
   *     （dfm 匯出的 'NA'、模擬器留下的數字）必須在使用者看到之前就被蓋掉。
   *  3. **title 寫 golden 來源**，這樣哪天 C++ 端補了 producer，改哪裡是自明的。
   */
  function applyNoSource() {
    var m = CFG.noSource || {}, ids = Object.keys(m), n = 0, missing = [];
    ids.forEach(function (id) {
      var el = $(id);
      if (!el) { missing.push(id); return; }
      showText(el, '---');
      el.title = id + '：wb_serve 無 producer，顯示 "---"（不可知，不是 0）。\n' +
                 'golden 來源：' + m[id][0] + '\n' + m[id][1];
      n++;
    });
    if (missing.length) {
      say('⚠ noSource 有 ' + missing.length + ' 個 id 在頁面上找不到：' + missing.join(', '), '#ffcc66', 'startup');
    }
    return n;
  }

  function hasSys() {
    return Object.keys(CFG.sysFields || {}).length > 0 ||
           Object.keys(CFG.sysEnums || {}).length > 0 ||   // Steven 20260916
           Object.keys(CFG.sysRows || {}).length > 0 ||
           !!(CFG.sysGrid && CFG.sysGrid.file) ||            // Steven 20260916
           !!(CFG.sysLevels && CFG.sysLevels.file);          // Steven 20260916
  }

  /* Steven 20260916
   * -------------------------------------------------------------------------
   * 表格模式（sysGrid）—— 「整張表就是檔案」
   * -------------------------------------------------------------------------
   * HW.MotorTest / HW.IoSetView 在 golden 是 TStringGrid：整個 csv 載進格子，
   * 操作員雙擊格子改值，按 Save 整張寫回（uMotorTest.cpp:2160-2271、
   * iosetview.cpp:3439）。widget 與 (列,欄) 沒有一對一的 id，所以 sysRows 那種
   * 「id -> 格子」的對照表表達不了；這裡直接把 /api/system/<csv> 的
   * {columns, keyColumn, rows} 畫成表格，並沿用同一套 preview -> 確認 -> 寫入 -> 重讀。
   *
   *   sysGrid: {
   *     file:     'motTable',            /api/system/<file>（csv）
   *     host:     'strngrdMotorData',    golden TStringGrid 對應的容器 id
   *     save:     'sbUpdate',            頁面上的 Save 鈕（接管）
   *     reload:   'sbtReload',           頁面上的 Load 鈕（接管，可省略）
   *     readOnly: ['Motorname'],         不可編輯的欄（鍵欄一律唯讀，不必列）
   *     kb:       [['Alias','NO_SYMBOL|NO_SPACE'], ['GearRatio','DOUBLE'], ['*','INTEGER']],
   *               golden 用 AnsiPos 對「表頭文字」做子字串比對決定小鍵盤，這裡照抄：
   *               依序找第一條 substr 落在欄名裡的規則；'*' 是預設。
   *     filters:  [{el:'edtSearchIO', kind:'search'}, {el:'cbbType', kind:'equals', col:'IOType', all:'All'}]
   *               沿用頁面既有的搜尋框/下拉，只做顯示過濾，不動資料。
   *   }
   *
   * 三件刻意的事：
   *  1. **鍵欄唯讀。** 寫入是用鍵值定位列的；把鍵改掉，這一列就再也對不到自己。
   *  2. **只送有改過的格子**，不是整張表。伺服器逐格回 changed/identical/notFound，
   *     整張送會把 9,000 格的 identical 淹掉真正的訊號。
   *  3. **原本的容器藏起來、旁邊放自己的。** 這兩頁的 legacy JS 會非同步從
   *     ../JSON/*-config.json（09-02 的過期快照）把格子畫進同一個 host；
   *     給它一個看不見的舊容器讓它畫，比修 100KB / 830KB 的頁面 JS 安全。
   *     同理 Save/Load 鈕上還掛著 legacy 的 click（寫 JSON 用的），用 document
   *     捕獲階段攔下來，不讓它跑。
   */
  var G = null;     // { file, cols, keyCol, keyIdx, rows, orig:{key:{col:v}}, edits:{key:{col:v}}, host, table }
  var UNFILLABLE = {};   // Steven 20260916（審查 B1）：load 時 ctlSet 填不進去的 id -> 描述
  var LV_BROKEN = null;  // Steven 20260916：sysLevels 對照表對不上 HTML 時的原因清單
  var NOSRCN = 0;        // Steven 20260918：applyNoSource() 寫成 '---' 的格子數
  var NOSAVE = false;    // Steven 20260916：這一頁找不到存檔鈕（唯讀），要在狀態列講出來
  var SAVEBTN_NOTE = ''; // Steven 20260916：實際接管的存檔鈕 id，狀態列要講出來
                         // （這個檔是 'use strict'，未宣告就賦值會丟 ReferenceError）

  function gridCss() {
    if ($('ht9045WireGridCss')) return;
    var st = document.createElement('style');
    st.id = 'ht9045WireGridCss';
    st.textContent =
      '.wbGridHost{background:#fff;}' +
      '.wbGrid{border-collapse:collapse;font:11px/1.4 Consolas,monospace;white-space:nowrap;}' +
      '.wbGrid th,.wbGrid td{border:1px solid #9aa;padding:1px 5px;text-align:left;}' +
      '.wbGrid th{position:sticky;top:0;background:#d4d0c8;z-index:1;}' +
      '.wbGrid td{cursor:pointer;}' +
      '.wbGrid td.ro{color:#667;background:#f0f0f0;cursor:default;}' +
      '.wbGrid td.key{font-weight:bold;}' +
      '.wbGrid td.dirty{background:#fff2a8;font-weight:bold;}' +
      '.wbGrid tr.sel td{outline:1px solid #000080;outline-offset:-1px;}' +
      '.wbGrid tr:hover td{background:#eef3ff;}' +
      '.wbGrid tr:hover td.dirty{background:#ffe680;}';
    document.head.appendChild(st);
  }

  function gridFlags(col) {
    var rules = (CFG.sysGrid && CFG.sysGrid.kb) || [], i, r, dflt = null;
    for (i = 0; i < rules.length; i++) {
      r = rules[i];
      if (r[0] === '*') { dflt = r; continue; }
      if (String(col).indexOf(r[0]) >= 0) return r;       // golden AnsiPos 子字串語意
    }
    return dflt || ['INTEGER', 0, false, 0, 0];
  }
  function flagBits(names) {
    if (typeof HTQwerty === 'undefined') return 0;
    return String(names).split('|').reduce(function (a, n) { return a | (HTQwerty.N[n.trim()] || 0); }, 0);
  }

  function gridHost() {
    var cfg = CFG.sysGrid, legacy = $(cfg.host);
    if (!legacy) return null;
    var mine = $(cfg.host + '__wb');
    if (mine) return mine;
    mine = document.createElement('div');
    mine.id = cfg.host + '__wb';
    mine.className = 'wbGridHost';
    // 先抄版面（position/left/top/right/bottom/overflow），再把舊的藏起來。
    mine.style.cssText = legacy.style.cssText;
    mine.style.display = '';
    mine.style.overflow = 'auto';
    legacy.style.display = 'none';
    legacy.setAttribute('data-wb-hidden', '1');
    legacy.parentNode.insertBefore(mine, legacy);
    return mine;
  }

  function gridRender(d) {
    var cfg = CFG.sysGrid;
    gridCss();
    var host = gridHost();
    if (!host) { say('❌ 找不到表格容器 #' + cfg.host, '#f88'); return 0; }
    var cols = d.columns || [], keyCol = d.keyColumn, keyIdx = cols.indexOf(keyCol);
    var ro = {}; (cfg.readOnly || []).forEach(function (c) { ro[c] = true; });
    if (keyCol) ro[keyCol] = true;
    G = { file: cfg.file, cols: cols, keyCol: keyCol, keyIdx: keyIdx, rows: d.rows || [],
          orig: {}, edits: {}, host: host, table: null, sel: null };

    var h = ['<table class="wbGrid"><thead><tr>'];
    cols.forEach(function (c) { h.push('<th' + (c === keyCol ? ' title="鍵欄（唯讀，寫入用它定位列）"' : '') + '>' + esc(c) + '</th>'); });
    h.push('</tr></thead><tbody>');
    G.rows.forEach(function (r) {
      var k = r[keyCol];
      G.orig[k] = {};
      h.push('<tr data-k="' + esc(k) + '">');
      cols.forEach(function (c) {
        var v = r[c] === undefined || r[c] === null ? '' : String(r[c]);
        G.orig[k][c] = v;
        h.push('<td data-c="' + esc(c) + '" class="' + (ro[c] ? 'ro' : '') + (c === keyCol ? ' key' : '') +
               '" title="' + esc(k + ' / ' + c) + '">' + esc(v) + '</td>');
      });
      h.push('</tr>');
    });
    h.push('</tbody></table>');
    host.innerHTML = h.join('');
    G.table = host.querySelector('table');

    if (!host.getAttribute('data-wb-bound')) {
      host.setAttribute('data-wb-bound', '1');
      host.addEventListener('click', function (ev) {
        var td = ev.target.closest ? ev.target.closest('td[data-c]') : null;
        if (!td) return;
        gridSelect(td.parentNode);
      });
      host.addEventListener('dblclick', function (ev) {
        var td = ev.target.closest ? ev.target.closest('td[data-c]') : null;
        if (!td || td.classList.contains('ro')) return;
        gridEdit(td);
      });
    }
    gridFilters();
    gridApplyFilter();
    return G.rows.length;
  }

  function gridSelect(tr) {
    if (G.sel) G.sel.classList.remove('sel');
    G.sel = tr; tr.classList.add('sel');
  }

  function gridEdit(td) {
    if (typeof HTQwerty === 'undefined') { say('❌ 沒有載入 qwerty.js，表格無法編輯。', '#f88'); return; }
    var tr = td.parentNode, key = tr.getAttribute('data-k'), col = td.getAttribute('data-c');
    var rule = gridFlags(col);
    var flags = flagBits(rule[0]);
    var opt = { dp: rule[1] || 0, checkRange: !!rule[2], min: rule[3] || 0, max: rule[4] || 0 };
    gridSelect(tr);
    opt.onCommit = function (v) {
      v = String(v);
      var o = G.orig[key][col];
      if (v === o) {
        if (G.edits[key]) { delete G.edits[key][col]; if (!Object.keys(G.edits[key]).length) delete G.edits[key]; }
        td.classList.remove('dirty');
        td.title = key + ' / ' + col;
      } else {
        (G.edits[key] = G.edits[key] || {})[col] = v;
        td.classList.add('dirty');
        td.title = key + ' / ' + col + '   原值 ' + o + '  →  ' + v + '（尚未存檔）';
      }
      gridStatus();
    };
    HTQwerty.show(td, flags, opt);
  }

  /* Steven 20260916
   * 新增 / 刪除整列 —— 走 system.csv.rows（見 ht9045_recipe_client.js sysApi.rows）。
   * golden 的 btnAdd* 是「多一個空白列讓人填」，btnDelete* 是「刪掉選中的那列」。
   * 網頁這邊識別欄一定要先有值（寫入靠它定位列），所以新增時先用小鍵盤問鍵值，
   * 其餘欄位留空（與 golden 空白列一致），寫入後重讀、操作員再雙擊填其他欄。
   * 兩者都走 dryRun -> 確認 -> 真寫入 -> 重讀，與存檔同一套。
   */
  function gridRows(ops, what) {
    var file = G.file;
    return HT9045System.rows(file, ops, {dryRun: true}).then(function (p) {
      if (!p || !p.ok) { say('❌ ' + what + '預演失敗：' + ((p && p.error) || '沒有 ack'), '#f88'); return null; }
      var nf = (typeof p.notFound === 'number') ? p.notFound : 0;
      if (nf) {
        say('❌ 拒絕' + what + '：伺服器回報 notFound=' + nf +
            '（新增：識別欄空白或已存在／含逗號；刪除：找不到或命中多列）', '#f88');
        return null;
      }
      var n = (p.added || 0) + (p.deleted || 0);
      if (!n) { say('沒有任何列會改變，不寫入。', '#9f9', 'transient'); return null; }
      var desc = (ops.add || []).map(function (r) { return '  新增 ' + G.keyCol + ' = ' + r[G.keyCol]; })
        .concat((ops.del || []).map(function (k) { return '  刪除 ' + G.keyCol + ' = ' + k; }));
      // Steven 20260916（審查 E2）：這不只是資料變更。刪掉一列，機台程式啟動時會
      // 找不到那顆馬達／那個 IO 點（golden cinitial.cpp "Can not find motor" 後把它
      // 當成未安裝繼續開機）。要講明白再讓人按確定。
      var warn = (ops.del && ops.del.length)
        ? '\n\n⚠ 刪除後機台程式會把這顆馬達／IO 點視為「不存在」並照常啟動，不會另外報警。'
        : '\n\n新列只填識別欄，其餘欄位留空，寫入後請雙擊補齊。';
      if (!window.confirm('即將對 ' + file + ' ' + what + '：\n\n' + desc.join('\n') + warn + '\n\n確定嗎？')) {
        say('已取消。', '#ffcc66', 'transient'); return null;
      }
      say(what + '中 ...');
      return HT9045System.rows(file, ops, {dryRun: false}).then(function (w) {
        if (!w || !w.ok) { say('❌ ' + what + '失敗：' + ((w && w.error) || '沒有 ack'), '#f88'); return null; }
        return load().then(function () {
          say('✅ ' + what + '完成並已重讀：added=' + (w.added || 0) + ' deleted=' + (w.deleted || 0), '#9f9', 'transient');
          return w;
        });
      });
    }).catch(function (e) { say('❌ ' + what + '失敗：' + e.message + tokenHint(e.message), '#f88'); throw e; });   // AI(W906-SCREEN-TOKEN) 20261001: tokenHint
  }

  function gridAddRow() {
    if (!G) return;
    if (typeof HTQwerty === 'undefined') { say('❌ 沒有載入 qwerty.js。', '#f88'); return; }
    var tmp = document.createElement('input');   // 只當小鍵盤的目標，不進 DOM
    tmp.value = '';
    var rule = gridFlags(G.keyCol);
    say('請用小鍵盤輸入新列的 ' + G.keyCol + '（識別欄，之後不可改）...', '#ffcc66');
    HTQwerty.show(tmp, flagBits(rule[0]), {
      onCommit: function (v) {
        v = String(v).trim();
        if (!v) { say('識別欄不能空白，未新增。', '#ffcc66'); return; }
        if (G.orig[v]) { say('❌ ' + G.keyCol + ' = ' + v + ' 已存在，未新增。', '#f88'); return; }
        // Steven 20260916（審查 E1）：motTable 的 Motorname 必須是 M00..M99 —— golden 以
        // "M%02d" 對馬達 enum（cinitial.cpp:3666），別的字串對機台是死資料。
        var pat = CFG.sysGrid.keyPattern;
        if (pat && !(new RegExp(pat)).test(v)) {
          say('❌ ' + G.keyCol + ' = ' + v + ' 不符合格式 ' + pat + '，未新增。', '#f88'); return;
        }
        var r = {}; r[G.keyCol] = v;
        gridRows({add: [r]}, '新增列');
      },
      onAbort: function () { gridStatus(); }
    });
  }

  function gridDeleteRow() {
    if (!G) return;
    if (!G.sel) { say('請先點選要刪除的那一列。', '#ffcc66'); return; }
    var k = G.sel.getAttribute('data-k');
    gridRows({del: [k]}, '刪除列');
  }

  function gridDirtyCount() {
    var n = 0;
    Object.keys(G ? G.edits : {}).forEach(function (k) { n += Object.keys(G.edits[k]).length; });
    return n;
  }
  // Steven 20260921：kind 預設 transient —— 編輯格子之後的「未存檔的變更 N 格」
  // 是操作員要看的，release 也要出現，只是看過就收起來。
  // 開頁那一次由 load() 傳 'startup'（開發用的自我報告，只在 debug 顯示）。
  function gridStatus(kind) {
    if (!G) return;
    var n = gridDirtyCount();
    say('表格 ' + G.file + '：' + G.rows.length + ' 列 × ' + G.cols.length + ' 欄（鍵欄 ' + G.keyCol + '）' +
        (KBN ? '；小鍵盤已掛上 ' + KBN + ' 個輸入框' : '') +
        '\n雙擊格子修改；' + (n ? '未存檔的變更 ' + n + ' 格（黃底），按 Save 寫回。' : '目前沒有未存檔的變更。'),
        n ? '#ffcc66' : '#9f9', kind || 'transient');
  }

  // 沿用頁面既有的搜尋框／下拉做顯示過濾。qwerty 提交後會補發 input/change
  // 事件（見 attachKeyboards），所以用小鍵盤打搜尋字也會即時過濾。
  function gridFilters() {
    var fs = (CFG.sysGrid && CFG.sysGrid.filters) || [];
    fs.forEach(function (f) {
      var el = $(f.el);
      if (!el || el.getAttribute('data-wb-filter')) return;
      el.setAttribute('data-wb-filter', '1');
      ['input', 'change'].forEach(function (evn) { el.addEventListener(evn, gridApplyFilter); });
    });
  }
  function gridApplyFilter() {
    if (!G || !G.table) return;
    var fs = (CFG.sysGrid && CFG.sysGrid.filters) || [];
    var tests = [], only = null;
    fs.forEach(function (f) {
      var el = $(f.el); if (!el) return;
      var v = (el.tagName === 'SELECT') ? (el.options[el.selectedIndex] ? el.options[el.selectedIndex].text : '') : el.value;
      v = String(v || '').trim();
      if (!v) return;
      // AI(W906-IOSV-FILTER) 20261002: opt-in golden semantics (only a page whose wire file asks for them; other pages unchanged).
      //   f.col = match that column only; f.minLen = shorter text filters nothing (golden iosetview.cpp:3775 `Length()>=2`);
      //   f.exclusive = while it is active the other filters are ignored (golden edtSearchIOChange :3773-3875 lists every
      //   matching Alias whatever cbbType / cbbLane say); kind 'pos' = substring, case-sensitive (golden LoadIoTable :3066
      //   Type.AnsiPos(sType): "Cylinder" also lists Cylinder_On / Cylinder_Off).
      if (f.minLen && v.length < f.minLen) return;
      if (f.kind === 'search' && f.col) {
        var qc = v.toLowerCase(), cc = f.col;
        var tcol = function (tr) { var td = tr.querySelector('td[data-c="' + cc + '"]'); return !!td && td.textContent.toLowerCase().indexOf(qc) >= 0; };
        if (f.exclusive) { only = tcol; return; }
        tests.push(tcol);
      } else if (f.kind === 'pos') {
        if (f.all && v === f.all) return;
        tests.push(function (tr) {
          var td = tr.querySelector('td[data-c="' + f.col + '"]');
          return !!td && td.textContent.indexOf(v) >= 0;
        });
      } else if (f.kind === 'search') {
        var q = v.toLowerCase();
        tests.push(function (tr) { return tr.textContent.toLowerCase().indexOf(q) >= 0; });
      } else if (f.kind === 'equals') {
        if (f.all && v === f.all) return;
        tests.push(function (tr) {
          var td = tr.querySelector('td[data-c="' + f.col + '"]');
          return td && td.textContent.trim() === v;
        });
      }
    });
    if (only) tests = [only];   // AI(W906-IOSV-FILTER) 20261002: f.exclusive (above)
    var shown = 0;
    Array.prototype.forEach.call(G.table.tBodies[0].rows, function (tr) {
      var ok = tests.every(function (t) { return t(tr); });
      tr.style.display = ok ? '' : 'none';
      if (ok) shown++;
    });
    return shown;
  }

  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/"/g, '&quot;'); }

  function allMap() {
    var m = {}, k;
    for (k in CFG.fields) m[k] = CFG.fields[k];
    for (k in (CFG.optional || {})) if (!m[k]) m[k] = CFG.optional[k];
    return m;
  }

  function isOptional(id) { return !!(CFG.optional && CFG.optional[id]); }

  /* Steven 20260918 (W906-FW-KBRANGE)
   * -------------------------------------------------------------------------
   * 小鍵盤的上下限改成「伺服器有就用伺服器的」
   * -------------------------------------------------------------------------
   * 使用者 20260918：「小鍵盤顯示時要出現上下限卡控，JSON 裡面有相關資料
   * 一起傳輸吧，避免臨時有人改了上下限的數值。」
   *
   * 問題在哪：接線檔 kb 區塊的 [FLAG, dp, checkRange, min, max] 是
   * **產生的時候**從 golden 的 ShowQwertyKey 抄下來的靜態值。限值後來若在
   * 機台那邊改了，畫面還是拿舊的在擋 —— 而且擋錯了完全看不出來，
   * 操作員只會覺得「這個數字打不進去」或更糟：打得進一個現在已經不合法的值。
   *
   * 做法：讀回來的每個欄位若帶了 min／max，就用它；沒帶才退回接線檔的靜態值。
   * 這樣 C++ 端一旦在 /api/recipe/<doc>、/api/system/<name> 的
   * {value,type,raw} 旁邊補上 min／max，前端不必改就生效；補之前行為完全不變。
   *
   * ⚠ 這只是「顯示與輸入時的即時回饋」那一層。真正的把關必須在伺服器端 ——
   *   WS 直連完全繞得過瀏覽器（目前只有 system.levels.put 會擋值域）。
   *   兩件事不要混為一談，也不要因為這裡擋了就以為安全。
   * ⚠ 伺服器送回來的欄位值若本身就超出 min／max（現場既有的壞值），
   *   這裡**不動它**。夾回去會讓那個問題永遠不被發現。
   */
  function cellOf(id) {
    var m = allMap(), e;
    if (m[id]) {                                    // 配方欄位：[doc, section, key]
      e = m[id];
      var sec = (LAST[e[0]] || {})[e[1]];
      return (sec && sec[e[2]]) || null;
    }
    e = sysEntry(id);                               // 機台設定檔欄位
    if (e) {
      var ss = (LAST['sys:' + (e[0])] || {})[e[1]];
      return (ss && ss[e[2]]) || null;
    }
    return null;
  }

  function num(x) {
    if (x === null || x === undefined || x === '') return null;
    var n = Number(x);
    return isFinite(n) ? n : null;
  }

  /* 回傳 {min, max, from} 或 null。from 只用在提示文字上，讓人知道這組限值
     是機台現在講的，還是接線檔當初抄下來的。 */
  function serverRange(id) {
    var c = cellOf(id);
    if (!c || typeof c !== 'object') return null;
    var mn = num(c.min), mx = num(c.max);
    if (mn === null || mx === null) return null;
    if (mn === mx) return null;                     // 沒有有效值域，別拿來擋
    return { min: mn, max: mx, from: 'server' };
  }


  /* ===========================================================================
   * Steven 20260924：C 路（golden 表單橋）。Steven 20260924 定名「C 路」。
   *   規格：.claude/skills/ht9045-html-json/references/route-c-golden-bridge.md
   *   與 B 路（檔案鏡像）不同：這一頁的值、Visible、Enabled（權限）都是 C++ 跑 golden
   *   開頁（FormShow）後給的；存檔跑 golden 關頁存檔流程（WS editlist.save），不直接改檔。
   *   這些頁面 load()／save() 整個改走這裡，B 路的 sysFields 不讀也不寫（同一個檔只能有一個寫者）。
   * =========================================================================== */
  var GOLDEN_BRIDGE = {                // 頁面 → C++ 結構（FileRW/<結構>.cpp）
    'Config.Configuration.html': 'IniConfig',
    'Setup.Ld_ULd.html': 'Ld_UldDelayTime',      // Steven 20260924：golden TfLd_ULd（FileRW/Ld_UldDelayTime.cpp）
    'Setup.TrayForm.html': 'UserDefForm_File',   // Steven 20260924：golden TfTrayForm（FileRW/UserDefForm_File.cpp）
    'Setup.Speed.html': 'ArmSpeed_File',         // Steven 20260924：golden TfSpeed（FileRW/ArmSpeed_File.cpp）
    'Setup.YieldMonitoring.html': 'TestIF_File_YieldMonitoring',  // Steven 20260925：golden TfYieldMonitoring
    'Setup.TrayAssignment.html': 'TrayForm',     // Steven 20260925：golden TfTrayAssignment（FileRW/TrayForm.cpp）
    'Setup.Contact.html': 'DeviceForm_File',  'Setup.ContactForce.html': 'ContactForce',   /* AI(W906-CF-WIRE) 20261002: golden TfContactForce (FileRW/ContactForce.cpp, open gate GContactForce FileRW/_EditPage.cpp; page supplement ht9045_contactforce_c.js builds the constructor panels from extra.panels) -- the page was a static picture */   // Steven 20260925：golden TfContact（力量公式已移植）
    'HW.teach.html': 'Teach',                    // AI(W906-W5-TEACH) 20260925：FileRW/Teach.cpp（golden TfTeach FormShow／btnSaveClick）
    'Setup.BinSel.html': 'BinSelect',            // Steven 20260925：golden TfBinSel（FileRW/BinSelect.cpp；回應多 bin，存檔可附 bin／actions）
    'Config.DIOInterFaceCFG.html': 'TTLCfg',     // Steven 20260925：golden TfDIOFrom（FileRW/TTLCfg.cpp；DIO 檔＝TFTestIF cbDIOType->Text）
    'HW.HandlerSys.html': 'HSys',                // Steven 20260925：golden THandlerSystem（FileRW/HSys.cpp；開頁照 golden 補寫 Gerneral.ini 缺鍵）
    'Setup.Temp_Set.html': 'Temperature',        // Steven 20260925：golden TfTemp_Set（FileRW/Temperature.cpp；myTempPal<i>_<成員>）
    'Setup.OffSet.html': 'Offset_File',          // Steven 20260925：golden TfOffSet（FileRW/Offset_File.cpp；回應 offsets 整包，頁面按鈕選組顯示）
    'Data.StartCondition.html': 'StartCondition',   // Steven 20260925：golden TfStartCondition（FileRW/StartCondition.cpp；三顆存檔鈕各跑 golden 處理器）
    'Setup.QAMode.html': 'TestIF_File_QAMode',  'Setup.AGV.html': 'TestIF_File_AGV',   /* AI(W906-B8-AG1) 20260930（St01）：golden TfAGV（FileRW/TestIF_File_AGV.cpp；頁面補件 ht9045_agv_c.js；spbSaveClick 沒有 YES/NO → 不列 GB_SAVE_Q） */      // Steven 團隊 20260925：golden TfQAMode（FileRW/TestIF_File_QAMode.cpp；頁面補件 ht9045_qamode_c.js）
    'HW.VacuumUnit.html': 'TestIF_File_VacuumUnit', // Steven 團隊 20260925：golden TfVacuumUnit elVacuumUnit（FileRW/TestIF_File_VacuumUnit.cpp；頁面補件 ht9045_vacuumunit_c.js）
    'HW.ShuttleMove.html': 'ShuttleMove',            // Steven 團隊 20260926：golden TfShuttleMove 讀寫段（FileRW/ShuttleMove.cpp；頁面補件 ht9045_shuttlemove_c.js；sbUpdateClick 沒有 YES/NO → 不列 GB_SAVE_Q）
    'Status.GroundMan.html': 'GroundMan',  'Main.AOAInfo.html': 'AOAOffset',  'Status.CounterSel.html': 'IniConfig_CounterSel',   /* AI(W906-FRW-AOA) 20260926（Steven 團隊）：golden TfMain OffsetSaveClick（FileRW/AOAOffset.cpp；頁面補件 ht9045_aoaoffset_c.js；golden 沒有 YES/NO → 不列 GB_SAVE_Q） */            //AI(W906-CRT-GroundMan) 20260926（Steven 團隊）：golden TfGroundMan 讀寫段（FileRW/GroundMan.cpp；頁面補件 ht9045_groundman_c.js；spbSaveClick 沒有 YES/NO → 不列 GB_SAVE_Q）
    'Setup.Cleaning.html': 'TestIF_File_Cleaning',  // Steven 團隊 20260925（暫接，整合者定）：golden TfCleaning（FileRW/TestIF_File_Cleaning.cpp；頁面補件 ht9045_cleaning_c.js）
    'Setup.TesterIF.html': 'TestIF_File_TesterIF',  // Steven 團隊 20260925：golden TFTestIF（FileRW/TestIF_File_TesterIF.cpp；頁面補件 ht9045_testerif_c_wire.js）
    'Setup.BarCode.html': 'TestIF_File_BarCode',    // Steven 團隊 20260925：golden TfBarCode（FileRW/TestIF_File_BarCode.cpp；頁面補件 ht9045_barcode_c.js；spbSaveClick 沒有 YES/NO → 不列 GB_SAVE_Q）
    'Setup.SetUp.html': 'TestIF_File_SetUp'      // Steven 20260925：golden TfSetup（FileRW/TestIF_File_SetUp.cpp；存檔照 golden 送 ATC7 @CH_ENABLED——沒有 ATC7 用戶端時只記 "No Client!"）
  };
  // AI(W906-W5-TEACH) 20260925：各結構 golden 存檔流程的確認框（ELAsk 以英文原字串查答案）[英文原字串, 頁面問的中文]
  var GB_SAVE_Q = {
    // Steven 20260925（合併）：沒列的結構 golden 存檔鈕不問 → 頁面仍確認一次（網頁誤觸保護），但不送答案
    'IniConfig': ['Config data save to define?', '確定要寫入資料？'],                 // golden TfConfiguration::FormClose
    'Teach':     ['Sure to Save? (確定要存檔?)', '確定要存檔？（教導值會寫進 teach.ini）'],  // golden TfTeach::btnSaveClick（fAllMotorHome==false 時才問）
    'HSys':      ['Do you want to store the setting?', '確定要存檔？（寫 Gerneral.ini，部分設定要重開程式才生效）']   // Steven 20260925：golden THandlerSystem::SaveSystemSet :577
  };  var GB_NO_ASK = { StartCondition: 1, TestIF_File_BarCode: 1, IniConfig_CounterSel: 1,  TestIF_File_QAMode: 1, Ld_UldDelayTime: 1, UserDefForm_File: 1, DeviceForm_File: 1, TTLCfg: 1, TestIF_File_TesterIF: 1, ContactForce: 1, TestIF_File_AGV: 1, ArmSpeed_File: 1, TestIF_File_YieldMonitoring: 1, TrayForm: 1, Temperature: 1, Offset_File: 1, TestIF_File_Cleaning: 1, TestIF_File_VacuumUnit: 1, ShuttleMove: 1, TestIF_File_SetUp: 1, AOAOffset: 1, TfHotPlate: 1, GroundMan: 1 /* AI(W906-NOASK) 20261002 (compK ioS): golden GroundMan.cpp:1418 spbSaveClick, 0 YES/NO */ };   /* AI(W906-MOTB-NOASK) 20261002: golden saves read, no YES/NO / MessageDlg in the handler or what it calls (only ShowMyMessage notices, which come back from C++): cSpeed.cpp:1433 spbSaveClick, uYieldMonitoring.cpp:3064 btnApplyClick, cTrayAssignment.cpp:1254 spbSaveClick, uTemp_Set.cpp:4201 spbSaveClick, cOffSet.cpp:2803 spbSaveClick, uCleaning.cpp:1761 sbCleanSaveClick, VacuumUnit.cpp:378 spbSaveClick, ShuttleMove.cpp:1965 sbUpdateClick, cSetUp.cpp:3468 sbUpdateClick (its DoPassword re-login is the page's own Q45 code), main.cpp:33822 OffsetSaveClick, cHotPlate.cpp:440 spbSaveClick (TfHotPlate = the A-shape form, see bridgeSave) */   /* AI(W906-NOASK) 20261002 (setupA): golden saves with no YES/NO -- QAMode.cpp:73 btnApplyClick, cLd_ULd.cpp:179 / cTrayForm.cpp:610 / cTesterIF.cpp:1302 spbSaveClick, cContact.cpp:14072 spbSaveClick (the :14124 height question is CC_KYEC only), DIOInterFaceCFG.cpp:191 spbSaveClick (its SaveDialog = the C path saves to the loaded file), ContactForce.cpp:935 btSaveClick, Automation/AGV.cpp:928 spbSaveClick */   /* AI(W906-NOASK) 20261001: golden save has no YES/NO and EastSun 1001 "我按下不要有提醒視窗" -> these structs save without the page's own window.confirm (golden cStartCondition.cpp:627 sbSaveClick / :926 sbHeadCondition1SaveClick / :670 spbExitClick, BarCode.cpp:1254 spbSaveClick, cCounterSel.cpp:56 FormClose: 0 MessageBox); a struct is added here only after its golden save is read */
  var GB = null;                       // 最近一次 editlist.get 的回應
  var GB_KIND = {};                    // id -> 值的種類（checked／itemIndex／text／position／dateTime／cells／tag）
  var GB_LAST = null;                  // 最近一次 editlist.save 的 ack（探針／除錯）
  var GB_KINDS = ['checked', 'itemIndex', 'text', 'position', 'dateTime', 'cells', 'tag'];  var GB_TIME_ONLY = { dtpN10_3_1_SpecifiedTime: 1, dtpO06NextTime: 1 }, GB_DT = {};   /* AI(W906-FRW-W44-1) 20260928 (St02-E claim): golden Kind = dtkTime (906_0625_Steven cConfiguration.dfm:15412 / :13629) -- show and type the time only; GB_DT keeps the date part */
  var GB_UNFILL = {};  /* AI(W906-R33) 20260926: C 路 id -> 為什麼填不進（NB2 R33；比照 sys-INI 路的 UNFILLABLE，審查 B1） */ var GB_ENA = {};                     // id -> 能不能改（清單筆 enabled 與替身 editable 取 AND；gbLoad 最後一次套）
  function gbStruct() { return CFG && CFG.page && GOLDEN_BRIDGE[CFG.page]; }

  // golden TDateTime（1899-12-30 起的天數）<-> 畫面字串
  function oleToText(d) {
    var ms = Math.round((d - 25569) * 86400000);           // 25569 = 1970-01-01
    var t = new Date(ms), p = function (n) { return (n < 10 ? '0' : '') + n; };
    return t.getUTCFullYear() + '/' + p(t.getUTCMonth() + 1) + '/' + p(t.getUTCDate()) + ' ' +
           p(t.getUTCHours()) + ':' + p(t.getUTCMinutes()) + ':' + p(t.getUTCSeconds());
  }
  function textToOle(s) {
    var m = /^\s*(\d{4})\/(\d{1,2})\/(\d{1,2})(?:\s+(\d{1,2}):(\d{2})(?::(\d{2}))?)?\s*$/.exec(s || '');
    if (!m) return null;
    var ms = Date.UTC(+m[1], +m[2] - 1, +m[3], +(m[4] || 0), +(m[5] || 0), +(m[6] || 0));
    return ms / 86400000 + 25569;
  }  function hhmmToOle(s, base) { var m = /^\s*(\d{1,2}):(\d{2})(?::(\d{2}))?\s*$/.exec(s || ''); if (!m || +m[1] > 23 || +m[2] > 59 || +(m[3] || 0) > 59) return textToOle(s); return Math.floor(typeof base === 'number' ? base : 0) + (+m[1] * 3600 + +m[2] * 60 + +(m[3] || 0)) / 86400; }   /* AI(W906-FRW-W44-1) 20260928 (St02-E claim): 'HH:MM[:SS]' on the picker's own date (a full date text still works) */
  function gbGrid(el, cells) {                              // TStringGrid：cells[欄][列]
    var cols = cells.length, rows = cols ? cells[0].length : 0, h = '<table>';
    for (var r = 0; r < rows; r++) {
      h += '<tr>';
      for (var c = 0; c < cols; c++) h += (r === 0 ? '<th' : '<td') + ' data-c="' + c + '" data-r="' + r + '">' +
                                          String(cells[c][r]).replace(/[&<>]/g, function (x) { return {'&':'&amp;','<':'&lt;','>':'&gt;'}[x]; }) +
                                          (r === 0 ? '</th>' : '</td>');
      h += '</tr>';
    }
    el.innerHTML = h + '</table>';
    // golden strngrdAutoSaveLogMouseDown（cConfiguration.cpp:6418）：第 1 列點一下 "" <-> "On"
    el.onclick = function (ev) {
      var td = ev.target.closest && ev.target.closest('td');
      if (!td || el.getAttribute('aria-disabled') === 'true') return;
      td.textContent = td.textContent === '' ? 'On' : '';
    };
  }
  function gbGridRead(el) {
    var out = [], trs = el.querySelectorAll('tr');
    for (var r = 0; r < trs.length; r++) {
      var cs = trs[r].children;
      for (var c = 0; c < cs.length; c++) { (out[c] = out[c] || [])[r] = cs[c].textContent; }
    }
    return out;
  }
  function gbSetEnabled(el, on) {                           // 容器停用＝底下全部停用（golden ChangeCompomentEnabled）
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if ('disabled' in x) {
        // 審查 M-1（第 7 輪）：只標記「自己關的」—— 本來就停用的（HTML 原生 disabled、別的寫者關的）不標記，
        // 之後容器重新可改時也不會被打開
        if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
        else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
      }
    });
    if (!on) el.setAttribute('aria-disabled', 'true'); else el.removeAttribute('aria-disabled');
  }
  // Steven 20260925：TTabSheet 頁籤沒有 id，是 title="<id> : TTabSheet" 的 .tab。release 模式 theme.js 會把 title 搬到
  //   data-htitle 再 removeAttribute（見上方 (1)），只查 [title^=] 會找不到 → 後端的 TabVisible 從來沒套上（TesterIF 實測）。
  function gbTabOf(id) {
    return document.querySelector('.tab[data-htitle^="' + id + ' :"]') || document.querySelector('.tab[title^="' + id + ' :"]');
  }
  function gbApply(id, v) {                                 // 一個替身／清單筆的值與狀態套上畫面
    var el = $(id);
    if (!el && v.tabVisible !== undefined) {                // TTabSheet：頁籤沒有 id
      var tab = gbTabOf(id);
      if (tab) { tab.style.display = v.tabVisible ? '' : 'none'; return true; }
    }
    if (!el) return false;
    if (v.checked !== undefined) {
      var cb = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]');
      if (cb) { cb.checked = !!v.checked; GB_KIND[id] = 'checked'; }
    }
    if (v.itemIndex !== undefined) {
      if (el.tagName === 'SELECT') {                          // 審查 M2（第 6 輪）：上一次補的清單外選項先拿掉，重讀不會一直疊
        var olds = el.querySelectorAll('option[data-src="cpp-text"],option[data-src="cpp-item"]');
        for (var oi = 0; oi < olds.length; oi++) olds[oi].parentNode.removeChild(olds[oi]);
      }
      if (el.tagName === 'SELECT' && v.itemIndex < 0 && v.text) {       // 清單外的文字（VCL csDropDown）
        var o = document.createElement('option'); o.textContent = v.text; o.setAttribute('data-src', 'cpp-text');
        el.appendChild(o); el.selectedIndex = el.options.length - 1;
      } else if (el.tagName === 'SELECT' && v.itemIndex >= el.options.length) {
        // Steven 20260925：C++ 的 Items 比頁面多（例 HSys cbHandlerModel 補第 8 項 HT9050）→ 補到那一項再選；
        // 不補的話 selectedIndex 變 -1，存檔送 -1，golden switch 會寫出預設值。重讀時先移除（上面 cpp-item）。
        while (el.options.length < v.itemIndex) { var pz = document.createElement('option'); pz.textContent = ''; pz.setAttribute('data-src', 'cpp-item'); el.appendChild(pz); }
        var oz = document.createElement('option'); oz.textContent = v.text || ('#' + v.itemIndex); oz.setAttribute('data-src', 'cpp-item');
        el.appendChild(oz); el.selectedIndex = v.itemIndex;
      } else {
        if (!ctlSet(el, 'index', String(v.itemIndex))) GB_UNFILL[id] = 'itemIndex=' + v.itemIndex + '（畫面只有 ' + radios(el).length + ' 個選項）';   // AI(W906-R33) 20260926: 以前回 false 也照樣算套上，畫面停在 HTML 預設，存檔就把預設值寫回去（HandlerSys IO_CARD_TYPE 4→2）
      }
      GB_KIND[id] = 'itemIndex';
    } else if (v.text !== undefined && 'value' in el && el.type !== 'checkbox' && el.type !== 'radio') {
      el.value = v.text; GB_KIND[id] = 'text';
    }
    if (v.position !== undefined) { el.value = v.position; GB_KIND[id] = 'position'; }
    // AI(W906-INBOX108) 20260930 St01 (Jimmy INBOX 108, claim handoff 41433415): golden sets TLabel->Caption at run time (FormShow, OnChange ...);
    //   C++ sends `caption` only for TLabel proxies and only when it is non-empty (FileRW/_EditList.cpp:58). Applied only to a
    //   label-like element (not INPUT / SELECT / TEXTAREA, and not one with child elements such as a checkbox label), and only
    //   when it differs from what the element shows now: the page was generated with the DFM caption, so an untouched label
    //   is a no-op. Not a saved value, so GB_KIND is not set.
    if (typeof v.caption === 'string' && el.tagName !== 'INPUT' && el.tagName !== 'SELECT' && el.tagName !== 'TEXTAREA' &&
        !el.children.length && el.textContent !== v.caption) el.textContent = v.caption;
    if (v.dateTime !== undefined) { GB_DT[id] = v.dateTime; el.value = GB_TIME_ONLY[id] ? oleToText(v.dateTime).slice(11, 19) : oleToText(v.dateTime); GB_KIND[id] = 'dateTime'; }   /* AI(W906-FRW-W44-1) 20260928 (St02-E claim): dtkTime shows HH:MM:SS */
    if (v.cells !== undefined) { gbGrid(el, v.cells); GB_KIND[id] = 'cells'; }
    if (v.tag !== undefined && el.tagName === 'IMG') {        // golden TImage：Tag＝值，圖是 type<Tag>.bmp
      el.setAttribute('data-tag', v.tag);
      el.src = 'img/dfm_type' + v.tag + '.png';
      GB_KIND[id] = 'tag';
      el.onclick = function () {                             // golden imgI37_3Click（:6715）：0..7 循環
        if (el.getAttribute('aria-disabled') === 'true') return;
        var t = (parseInt(el.getAttribute('data-tag'), 10) + 1) % 8;
        el.setAttribute('data-tag', t); el.src = 'img/dfm_type' + t + '.png';
      };
    }
    if (v.visible !== undefined) el.style.visibility = v.visible ? '' : 'hidden';
    // 審查 H1（第 6 輪）：替身帶 editable（自己＋上層都 Enabled／Visible、非 ReadOnly，和存檔丟值同一個判斷）；
    // 這裡只記下來，gbLoad 全部套完值之後才一次處理（先開後關，停用的容器一定蓋過子元件）
    var ena = v.editable !== undefined ? v.editable : v.enabled;
    if (ena !== undefined) GB_ENA[id] = GB_ENA[id] !== false && !!ena;
    if (v.tabVisible !== undefined) {
      var t2 = gbTabOf(id);
      if (t2) t2.style.display = v.tabVisible ? '' : 'none';
    }
    el.setAttribute('data-src', 'cpp');
    return true;
  }
  function gbLoad(prefix, prefixColour) {             // prefix：存檔結果（重讀後仍要讓操作員看到）
    var st = gbStruct();
    say((prefix ? prefix + '\n' : '') + '讀取中（C 路：golden ' + st + ' 開頁 FormShow）...', prefixColour);
    return HT9045Recipe.editlistGet(st).then(function (d) {
      GB = d; GB_KIND = {}; GB_ENA = {}; GB_UNFILL = {};
      var miss = [], n = 0;
      Object.keys(d.lists || {}).forEach(function (ln) {
        ((d.lists[ln] || {}).entries || []).forEach(function (e) {
          if (!e.id) return;
          if (gbApply(e.id, e)) n++; else miss.push(e.id);
        });
      });
      Object.keys(d.proxies || {}).forEach(function (id) {
        if (!gbApply(id, d.proxies[id])) {
          var p = d.proxies[id];
          if (GB_KINDS.some(function (k) { return p[k] !== undefined; })) miss.push(id);   // 只列有值的；純容器沒有也無妨
        }
      });
      var ids = Object.keys(GB_ENA).filter(function (id) { return $(id); });
      ids.forEach(function (id) { if (GB_ENA[id]) gbSetEnabled($(id), true); });
      ids.forEach(function (id) { if (!GB_ENA[id]) gbSetEnabled($(id), false); });
      var lines = ['✔ 讀取完成（C 路，golden ' + st + '）：套用 ' + n + ' 筆清單值與 ' +
                   Object.keys(d.proxies || {}).length + ' 個元件狀態'];
      var mustMiss = (d.mustSend || []).filter(function (id) { return !$(id); });
      if (mustMiss.length) lines.push('❌ 存檔必須送回、但頁面上沒有的元件 (' + mustMiss.length + ')：' + mustMiss.join(', ') +
                                      '\n   → 這一頁目前不能存檔（golden 存檔會讀到它們）');
      var unf = Object.keys(GB_UNFILL); if (unf.length) lines.push('❌ 填不進畫面的值 (' + unf.length + ')：' + unf.map(function (k) { return k + ' ' + GB_UNFILL[k]; }).join('；') + '\n   → 這一頁目前不能存檔（存了會把畫面上的預設值寫回檔案）');   // AI(W906-R33) 20260926
      if (miss.length) lines.push('⚠ 頁面上沒有的 id (' + miss.length + ')：' + miss.slice(0, 40).join(', ') + (miss.length > 40 ? ' ...' : ''));
      if (prefix) lines.unshift(prefix, '— 重讀 —');
      say(lines.join('\n'), prefixColour || ((mustMiss.length || unf.length) ? '#ffcc66' : '#9f9'));
      return d;
    }, function (e) {
      say('❌ C 路讀取失敗（editlist.get ' + st + '）：' + e.message + tokenHint(e.message), '#f88');   // AI(W906-SCREEN-TOKEN) 20261001: tokenHint
      return null;
    });
  }
  function gbValue(id) {
    var el = $(id), k = GB_KIND[id];
    if (!el || !k) return null;
    if (k === 'checked') {
      var cb = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]');
      return cb ? { checked: cb.checked } : null;
    }
    if (k === 'itemIndex') {
      if (el.tagName === 'SELECT') {
        var o = el.options[el.selectedIndex];
        // 審查 M2（第 6 輪）：補的清單外選項不是 golden Items 的一項 → ItemIndex=-1，值在 Text（VCL csDropDown）
        if (o && o.getAttribute('data-src') === 'cpp-text') return { itemIndex: -1, text: o.textContent };
        return { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
      }
      var rs = radios(el);
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
      return { itemIndex: -1 };
    }
    if (k === 'text') return { text: String(el.value) };
    if (k === 'position') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }
    if (k === 'dateTime') { var d = GB_TIME_ONLY[id] ? hhmmToOle(el.value, GB_DT[id]) : textToOle(el.value); return d === null ? null : { dateTime: d }; }   /* AI(W906-FRW-W44-1) 20260928 (St02-E claim) */
    if (k === 'cells') return { cells: gbGridRead(el) };
    if (k === 'tag') return { tag: parseInt(el.getAttribute('data-tag'), 10) || 0 };
    return null;
  }
  function gbSave() {
    var st = gbStruct();
    if (!GB) { say('❌ 還沒讀取過，不能存檔（請先重讀）。', '#f88'); return Promise.resolve(null); }
    var widgets = {}, bad = [];
    Object.keys(GB_KIND).forEach(function (id) {
      var v = gbValue(id);
      if (v) widgets[id] = v; else bad.push(id);
    });
    var mustMiss = (GB.mustSend || []).filter(function (id) { return !widgets[id]; });
    var unfill = Object.keys(GB_UNFILL);   // AI(W906-R33) 20260926: 讀取時填不進的格子 ⇒ 整頁拒寫
    if (bad.length || mustMiss.length || unfill.length) {
      say('❌ 拒絕寫入：' + (bad.length ? '有 ' + bad.length + ' 個欄位的值讀不出來（例如日期格式不對）：' + bad.join(', ') + '\n' : '') +
          (mustMiss.length ? 'golden 存檔會讀、但頁面給不出值的元件：' + mustMiss.join(', ') : '') +
          (unfill.length ? '\n讀取時填不進畫面的值（存了會寫回預設值）：' + unfill.map(function (k) { return k + ' ' + GB_UNFILL[k]; }).join('；') : ''), '#f88');
      return Promise.resolve(null);
    }
    // golden 存檔流程裡的確認框（ShowMyMessageBox_YES_NO／MessageDlg）由頁面問，答案帶給伺服器（GB_SAVE_Q 有列的結構才帶題目）
    var QQ = GB_SAVE_Q[st] || null, Q = QQ ? QQ[0] : '';
    if (!(GB_NO_ASK[st] && !QQ) && !window.confirm((QQ ? QQ[1] : '確定要寫入資料？') + '\n' + (Q ? '（' + Q + '）\n' : '') + '\n走 golden ' + st + ' 的存檔流程。')) {   // AI(W906-NOASK) 20261001: GB_NO_ASK structs save at once, as golden does (see GB_NO_ASK)
      say('已取消，未寫入。', '#ffcc66', 'transient');
      return Promise.resolve(null);
    }
    var answers = {}; if (Q) answers[Q] = 1;
    // Steven 20260925（BinSelect）：頁面自己的資料（例 bin 的 7 組 panel、actions）由頁面 hook 附進存檔的 value。
    // window.GB_EXTRA_SAVE(GB) 回傳物件（例 {bin:{tags:[…]}, actions:['btnSettingSpecificBinClick']}）；沒有 hook 就不附。
    var extra = null;
    if (typeof window.GB_EXTRA_SAVE === 'function') {
      try { extra = window.GB_EXTRA_SAVE(GB) || null; } catch (e) { say('❌ 頁面資料整理失敗：' + e.message, '#f88'); return Promise.resolve(null); }
    }
    say('寫入中（C 路：golden 存檔流程）...');
    return HT9045Recipe.editlistSave(st, widgets, answers, extra).then(function (a) {
      GB_LAST = a;
      var s = a.session || {};
      var lines = [a.saved ? '✔ 已寫入（golden ' + st + '）'
                           : '⚠ 沒有寫入（golden 存檔流程在寫檔前就結束了，原因見下方訊息）'];
      (s.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });
      (s.asked || []).forEach(function (q) {
        if (q.answer !== 1) lines.push('⚠ golden 問了「' + (q.zh || q.en) + '」，這一版頁面沒有回答 → 視為「否」');
      });
      if ((a.ignored || []).length) lines.push('ⓘ ' + a.ignored.length + ' 個欄位目前權限不能改，沿用原值');
      if ((s.todo || []).length) lines.push('⚠ golden 還有沒做到的步驟：\n  ' + s.todo.join('\n  '));
      return gbLoad(lines.join('\n'), a.saved ? ((s.todo || []).length ? '#ffcc66' : '#9f9') : '#f88');   // 規則 3：寫完一定重讀
    }, function (e) {
      GB_LAST = { saved: false, error: e.message };
      say('❌ 寫入失敗：' + e.message + (/reload page/.test(e.message) ? '\n→ 請先按重讀（權限或登入狀態變了）' : '') + tokenHint(e.message), '#f88');   // AI(W906-SCREEN-TOKEN) 20261001: tokenHint
      return null;
    });
  }

  function load() {
    if (gbStruct()) return gbLoad();                                        // Steven 20260924：C 路
    var map = allMap(), groups = byDoc(map), docs = Object.keys(groups);
    // Steven 20260918：sysText 不進 hasSys()（那個函式決定「這一頁可不可寫」），
    // 但它確實要發出讀取請求，所以空頁判斷要另外把它算進來。
    var hasRO = Object.keys(CFG.sysText || {}).length > 0;
    if (!docs.length && !hasSys() && !hasRO) {
      // Steven 20260916：只接 tag（沒有任何讀寫欄位）的頁面是正常情況，
      // 不要說「沒有可讀寫的欄位」就算了 —— 那會讓人以為 tag 也沒接上。
      say('這一頁沒有可讀寫的欄位。' +
          (TAGN ? '執行期資料已接上 ' + TAGN + ' 個顯示欄位。' : '') +
          (KBN ? '小鍵盤已掛上 ' + KBN + ' 個輸入框。' : ''), '#9f9', 'startup');
      return Promise.resolve();
    }
    say('讀取中 ... (' + docs.concat(Object.keys(sysIniByFile()))
                       .concat(Object.keys(sysCsvByFile())).join(', ') + ')',
        null, 'startup');
    var filled = 0, absent = 0, missingKey = [], missingEl = [], gridRows = -1;   // Steven 20260916 gridRows
    var jobs = docs.map(function (doc) {
      return HT9045Recipe.read(doc).then(function (d) {
        LAST[doc] = d.sections || {};
        groups[doc].forEach(function (id) {
          var sec = map[id][1], key = map[id][2], el = $(id);
          if (!el) { if (!isOptional(id)) missingEl.push(id); return; }
          var s = d.sections[sec], cell = s && s[key];
          if (!cell) {
            // optional 的鍵本來就不是每個配方都有，缺了是正常的，不是警告。
            if (isOptional(id)) { absent++; el.value = ''; el.placeholder = '(本配方無此參數)'; }
            else missingKey.push(doc + ' / ' + sec + ' / ' + key);
            return;
          }
          el.placeholder = '';
          el.value = cellText(cell);   // Steven 20260916: raw，不是解析後的 value
          filled++;
        });
      });
    });

    // --- 系統檔：ini ---
    var si = sysIniByFile();
    Object.keys(si).forEach(function (file) {
      jobs.push(HT9045System.read(file).then(function (d) {
        LAST['sys:' + file] = d.sections || {};
        si[file].forEach(function (id) {
          // Steven 20260916：欄位可能來自 sysFields（文字）或 sysEnums（radio/checkbox/select）
          var e = sysEntry(id), sec = e[1], key = e[2], el = $(id);
          if (!el) { missingEl.push(id); return; }
          var sc = d.sections && d.sections[sec], cell = sc && sc[key];
          if (!cell) { missingKey.push(file + ' / ' + sec + ' / ' + key); return; }
          // ctlSet 回 false = 值填不進這個控制項（例如 ItemIndex 超出選項數）。
          // 當成缺陷回報，不要靜靜跳過 —— 否則存檔會把 -1 寫回去。
          // Steven 20260916: raw，不是解析後的 value
          // Steven 20260916（審查 B1）：另外記進 UNFILLABLE，collectSysIni 不送它、
          // save() 看到非空就整頁拒寫。黃字警告擋不住人按 Save。
          if (ctlSet(el, sysKind(id), cellText(cell))) { filled++; delete UNFILLABLE[id]; }
          else {
            UNFILLABLE[id] = file + ' / ' + sec + ' / ' + key + ' = "' + cellText(cell) + '"';
            missingKey.push(UNFILLABLE[id] + '（值與控制項選項對不上，這一格不會被存檔）');
          }
        });
      }));
    });
    // --- 系統檔：ini，唯讀顯示（sysText） ---
    // Steven 20260918 (W906-FW-OBSERVER)
    // ⚠ 這一段**刻意**與上面的 sysFields 分開，不是重複。合併的話 sysText 的 id
    //   會進 sysIniByFile()，save() 就會把這些顯示欄位一起送去寫檔 ——
    //   Data.Observer 在 golden 是唯讀頁，那會是一條沒人要求過的寫入路徑。
    var st = sysTextByFile();
    Object.keys(st).forEach(function (file) {
      jobs.push(HT9045System.read(file).then(function (d) {
        st[file].forEach(function (id) {
          var e = CFG.sysText[id], sec = e[1], key = e[2], el = $(id);
          if (!el) { missingEl.push(id); return; }
          var sc = d.sections && d.sections[sec], cell = sc && sc[key];
          if (!cell) {
            // 鍵不在檔裡：顯示 '---' 並說清楚是**哪一個鍵**找不到。
            // 這與「沒有 producer」看起來一樣，但原因完全不同，title 要分得開。
            showText(el, '---');
            el.title = id + '：' + file + ' / [' + sec + '] ' + key + ' —— 這台機器的設定檔裡沒有這個鍵。';
            missingKey.push(file + ' / ' + sec + ' / ' + key);
            return;
          }
          var txt = cellText(cell);
          showText(el, txt === '' ? '---' : txt);
          el.title = id + '：/api/system/' + file + ' [' + sec + '] ' + key +
                     '（唯讀顯示，不進存檔）';
          filled++;
        });
      }, function (err) {
        // 讀不到整個檔也要說出來，不要留著 HTML 裡寫死的值。
        st[file].forEach(function (id) {
          var el = $(id);
          if (el) { showText(el, '---'); el.title = id + '：讀不到 /api/system/' + file + '（' + err.message + '）'; }
        });
        missingKey.push(file + '（整個檔讀不到：' + err.message + '）');
      }));
    });

    // --- 系統檔：csv ---
    var sc2 = sysCsvByFile();
    Object.keys(sc2).forEach(function (file) {
      jobs.push(HT9045System.read(file).then(function (d) {
        var byKey = {}, kc = d.keyColumn;
        (d.rows || []).forEach(function (r) { byKey[r[kc]] = r; });
        LAST['sys:' + file] = byKey;
        // Steven 20260916：表格模式 —— 整張畫出來，重讀即清掉未存變更。
        if (CFG.sysGrid && CFG.sysGrid.file === file) gridRows = gridRender(d);
        sc2[file].forEach(function (id) {
          var rk = CFG.sysRows[id][1], col = CFG.sysRows[id][2], el = $(id);
          if (!el) { missingEl.push(id); return; }
          var row = byKey[rk];
          if (!row || !(col in row)) { missingKey.push(file + ' / ' + rk + ' / ' + col); return; }
          el.value = row[col]; filled++;
        });
      }));
    });

    // --- 系統檔：i32 投影（levelset） ---
    // Steven 20260916 (W906-FW-LEVELSET)
    var lvBad = null, lvFilled = -1;
    if (CFG.sysLevels && CFG.sysLevels.file) {
      lvBad = levelCheck();
      if (lvBad && lvBad.length) {
        // 對照表壞掉就不要去讀檔、更不要填任何一格 —— 填一半再讓人按存檔，
        // 會把畫面上沒對到的格子當成真值寫回去。
        LV_BROKEN = lvBad;
      } else {
        LV_BROKEN = null;
        jobs.push(HT9045System.read(CFG.sysLevels.file).then(function (d) {
          var vals = d.values || [];
          LAST['sys:' + CFG.sysLevels.file] = vals;
          var lv = levelScan(), n = 0;
          lv.binds.forEach(function (b) {
            var v = vals[b.idx];
            if (typeof v !== 'number') { missingKey.push(CFG.sysLevels.file + ' [' + b.idx + '] 超出檔案範圍'); return; }
            if (v < 0 || v >= b.inputs.length) {
              // 這台機器是 5 階模式而畫面只有 4 顆，或檔案的值超出選項數。
              // 記成缺陷並讓 save() 整頁拒寫，不要靜靜挑一顆選起來。
              UNFILLABLE['levelset[' + b.idx + ']'] =
                CFG.sysLevels.file + ' [' + b.idx + '] ' + b.caption + ' = ' + v +
                '（畫面只有 ' + b.inputs.length + ' 個選項）';
              missingKey.push(UNFILLABLE['levelset[' + b.idx + ']']);
              b.inputs.forEach(function (r) { lvCheck(r, false); });
              return;
            }
            b.inputs.forEach(function (r, i) { lvCheck(r, i === v); });
            n++;
          });
          lvFilled = n;
          filled += n;
        }));
      }
    }

    // Steven 20260924 (S12)：檔案讀完後，用 /api/form 的結果覆蓋（見 formOverlay）。AI(W906-Q4-S126) 20260927：第一型（DoIniDataToForm）退役，只剩第二型 bridge（HotPlate）。
    return Promise.all(jobs).then(formOverlay).then(function (fo) {
      var total = Object.keys(map).length +
                  Object.keys(CFG.sysFields || {}).length +
                  Object.keys(CFG.sysText || {}).length +    // Steven 20260918（唯讀顯示）
                  Object.keys(CFG.sysEnums || {}).length +   // Steven 20260916
                  Object.keys(CFG.sysRows || {}).length +
                  ((LV && !LV_BROKEN) ? LV.binds.length : 0);   // Steven 20260916
      var lines = ['讀取完成：填入 ' + filled + ' / ' + total + ' 個欄位'];
      if (fo) lines.push(fo.note);                  // Steven 20260924 (S12)
      if (absent) lines.push('其中 ' + absent + ' 個是本配方沒有的選用參數（正常，存檔時會自動略過）');
      if (KBN) lines.push('小鍵盤已掛上 ' + KBN + ' 個輸入框');
      // Steven 20260918：一頁滿是 '---' 時，要能一眼看出那是「查過、沒有來源」，
      // 而不是「接線壞了」或「還在載入」。滑過該格的 title 有 golden 來源與原因。
      if (NOSRCN) lines.push('另有 ' + NOSRCN + ' 個欄位 wb_serve 沒有 producer，'
                           + '已顯示 "---"（滑過去看 golden 來源與原因）');
      if (missingEl.length)  lines.push('⚠ 頁面上找不到的 id (' + missingEl.length + ')：' + missingEl.join(', '));
      if (missingKey.length) lines.push('⚠ 配方裡沒有的鍵 (' + missingKey.length + ')：' + missingKey.join(' | '));
      var np = Object.keys(CFG.pending || {}).length;
      if (np) lines.push('尚未接線（不安全或需人工確認）：' + np + ' 個，見 ht9045_wire_' + CFG.slug + '.js 檔尾');
      // Steven 20260916：唯讀要明說。不再注入浮動存檔鈕之後，「沒有存檔鈕」
      // 在畫面上看起來跟「還沒載入完」一模一樣。
      // Steven 20260918 (W906-FW-OBSERVER)：純唯讀頁（只有 sysText，沒有任何
      // 可寫欄位）。這種頁面根本不會走到上面的存檔鈕查詢，NOSAVE 永遠是 false，
      // 所以下面那條訊息不會出現 —— 必須自己講。
      var RO_ONLY = !docs.length && !hasSys() && Object.keys(CFG.sysText || {}).length > 0;
      if (RO_ONLY) lines.push('這是唯讀資訊頁：沒有存檔鈕，上面的欄位只顯示機台設定檔的內容，'
                            + '不會寫回任何檔案。');
      else if (NOSAVE) lines.push('⚠ 這一頁沒有可用的存檔鈕，資料唯讀。'
                           + '若該頁其實有存檔鈕，請在 ht9045_wire_' + CFG.slug + '.js 補 saveBtn:\'<id>\'。');
      // Steven 20260916：講出來是哪一顆。少了固定位置的浮動鈕之後，操作員沒有
      // 別的方法知道該按哪一顆；而且引擎是在捕獲階段攔截，那顆鈕原本的行為
      // （例如 Status.Security 的 SecurityExit 本來會關閉頁面）不會發生 ——
      // 這件事必須寫在畫面上，不能只寫在接線檔的註解裡。
      else if (SAVEBTN_NOTE) lines.push('存檔鈕：#' + SAVEBTN_NOTE
                                      + (CFG.saveBtnNote ? '（' + CFG.saveBtnNote + '）' : ''));
      // Steven 20260916 (W906-FW-LEVELSET)
      if (LV_BROKEN) {
        say('❌ 這一頁的權限對照表對不上 HTML，已整頁拒接（不讀也不寫）：\n  ' +
            LV_BROKEN.join('\n  ') +
            '\n索引來源是 title 的 [NN]（不是 MySecurity_Panel_N，也不是群組名的 _n）。', '#f88');
        return;
      }
      if (lvFilled >= 0) lines.unshift('權限表 ' + CFG.sysLevels.file + '：' + lvFilled + ' / ' +
                                       LV.binds.length + ' 組已載入');
      // Steven 20260916：純表格頁沒有欄位計數可講，改由 gridStatus 說話。
      if (gridRows >= 0 && !total) { gridStatus('startup'); return; }
      if (gridRows >= 0) lines.unshift('表格 ' + CFG.sysGrid.file + '：' + gridRows + ' 列已載入（雙擊格子修改）');
      // Steven 20260921：開頁的接線自我報告 -> startup（只在 debug 顯示並自動收起）。
      // 這裡就算是 ⚠ 也一樣：缺欄位／缺鍵是接線檔要修的事，操作員看了做不了什麼。
      // 真正會讓畫面上的值不可信的那幾條（整頁拒接、讀取失敗）在下面，仍是 sticky。
      say(lines.join('\n'), (missingEl.length || missingKey.length || (fo && fo.diff)) ? '#ffcc66' : '#9f9',
          'startup');
    }).catch(function (e) {
      say('讀取失敗：' + e.message + '\n（伺服器有起來嗎？應該看得到 http://127.0.0.1:8045/）', '#f88');
      throw e;
    });
  }

  /* Steven 20260924 (S12)：C++ 跑 DoIniDataToForm() 出 JSON，頁面照它顯示。
   * ---------------------------------------------------------------------------
   * 使用者 20260924：「DoIniDataToForm() 就等於是 C++ 發送 JSON 給 HTML」
   * （json-bridge skill decisions.md 二之二）。在這之前頁面只讀檔案，
   * ReadFile() 修正過的值（鉗制、換算、依旗標挑的顯示）畫面看不出來。
   *
   * GET /api/form/<page> 只送「這次 DoIniDataToForm() 真的有賦值」的屬性
   * （C++ 端兩輪哨兵法，見 JsonBridge/FormJson.cpp），所以沒送的 widget 維持讀檔結果。
   *
   * 文字欄位的規則（存檔位元組不變）：
   *   兩支 loader 存檔時都把整頁欄位送出、伺服器逐字串比對。若畫面換成 C++ 的
   *   格式化字串（-134.00），沒動過的欄位也會被當成改過（檔案是 -134.0000）。
   *   所以：依 C++ 的小數位數比較後**相同 → 換回檔案原字串**；**不同 → 用 C++ 的值**
   *   並記進 diff（那是 ReadFile() 修正過的值，存檔會把它寫回檔案 —— golden 的
   *   SaveSetupFile 存的也是畫面值）。
   *
   * 只套 visible:false／enabled:false：true 是 vclcompat 建構子的預設，可能蓋掉
   * 畫面規則（HTSettings.applyBlocks）已經藏起來的東西。
   *
   * 舊的 per-page loader（ht9045_hotplate_wire.js 等）自己也會讀檔填值，它們讀完
   * 會再呼叫一次 HT9045Page.formOverlay()，誰最後跑完結果都一樣。
   */
  var FORM = null;                     // 最近一次 /api/form 的回應，給探針／除錯看
  var BRIDGE_UNFILLED = {};            // Steven 20260924 審查 #3：C++ 有送、但畫面填不進去的 widget（id -> 理由）
  function decimals(s) {
    var m = /\.(\d+)\s*$/.exec(String(s));
    return m ? m[1].length : 0;
  }
  function sameShown(fileText, cppText) {
    var a = String(fileText).trim(), b = String(cppText).trim();
    if (a === b) return true;
    if (a === '' || b === '') return false;
    var fa = Number(a), fb = Number(b);
    if (!isFinite(fa) || !isFinite(fb)) return false;
    var d = decimals(b);
    return fa.toFixed(d) === fb.toFixed(d);
  }
  function fileRawOf(id, map) {
    var t = map[id];
    if (!t) return null;
    var s = (LAST[t[0]] || {})[t[1]], c = s && s[t[2]];
    return c ? cellText(c) : null;
  }
  function formOverlay() {
    if (typeof HT9045Recipe === 'undefined' || !HT9045Recipe.form || !CFG || !CFG.page) {
      return Promise.resolve(null);
    }
    return HT9045Recipe.form(CFG.page).then(function (d) {
      FORM = d;
      if (!d || d.available === false) {
        return { note: '⚠ C++ 表單 ' + ((d && d.form) || '') + ' 不在（' + ((d && d.why) || '') +
                       '），本頁只顯示檔案內容' };
      }
      // Steven 20260924 (S12 第二型)：golden 原檔產生的 bridge 若回報讀檔端缺口，
      // 它算出來的值是結構初值 —— 不可拿來蓋檔案值。畫面維持讀檔結果，存檔仍走原本的路。
      if (d.kind === 'golden-bridge' && d.sourceGap) {
        return { note: '⚠ C++ golden bridge ' + d.form + ' 已就緒，但讀檔端未移植，畫面仍以檔案為準：\n  ' +
                       d.sourceGap };
      }
      var map = allMap(), n = 0, diff = [], miss = [];
      BRIDGE_UNFILLED = {};
      Object.keys(d.widgets || {}).forEach(function (id) {
        var w = d.widgets[id], el = $(id), touched = false;
        // Steven 20260924 (S12 第二型)：TTabSheet 在頁面上沒有 id，是 PageControl 裡
        // title="<id> : TTabSheet" 的 .tab 頁籤（配對的內容是同序號的 .pcPane）。
        if (!el && w.tabVisible !== undefined) {
          var tab = gbTabOf(id);
          if (tab) {
            tab.style.display = w.tabVisible ? '' : 'none';
            tab.setAttribute('data-src', 'cpp');
            n++;
            return;
          }
        }
        if (!el) { miss.push(id); return; }
        if (w.items !== undefined && el.tagName === 'SELECT') {        // 下拉選項（例：cbDIOType 由 DioCfg\*.ini 列出）
          el.innerHTML = '';
          w.items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; el.appendChild(o); });
          touched = true;
        }
        if (w.activePageIndex !== undefined) {                          // TPageControl：點對應的頁籤
          var t2 = el.querySelector('.tabs .tab[data-t="' + w.activePageIndex + '"]');
          if (t2) { t2.click(); touched = true; }
        }
        // VCL：ItemIndex 未設或 -1 而 Text 有值 ＝ 顯示清單外的文字（Items->Clear() 之後再設 Text 就是這個狀態）
        var textSel = w.text !== undefined && el.tagName === 'SELECT' && (w.itemIndex === undefined || w.itemIndex < 0);
        if (textSel) {
          var found = false;
          for (var oi = 0; oi < el.options.length; oi++) {             // TComboBox->Text＝某個選項的文字
            if (el.options[oi].textContent === w.text) { el.selectedIndex = oi; touched = true; found = true; break; }
          }
          // Steven 20260924：VCL 的 TComboBox（csDropDown）可以顯示不在 Items 裡的文字（例：HotPlate golden
          // FormShow 先 Items->Clear() 再設 Text="Select from Database..."；cbbBaudRate 的值不在頁面選項裡），
          // golden 存檔時寫的也是 ->Text。HTML <select> 只能顯示選項，所以補一個該文字的選項並選取它 ——
          // 畫面與存檔都和 golden 一致。（原本記成 BRIDGE_UNFILLED 拒寫，那會讓 golden 合法的狀態存不了檔。）
          if (!found) {
            var extra = document.createElement('option');
            extra.textContent = w.text;
            extra.setAttribute('data-src', 'cpp-text');
            el.appendChild(extra);
            el.selectedIndex = el.options.length - 1;
            touched = true;
          }
        }
        if (w.text !== undefined && el.tagName !== 'SELECT' && 'value' in el &&
            !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) {
          var raw = fileRawOf(id, map);
          if (raw !== null && sameShown(raw, w.text)) {
            el.value = raw;                              // 與機台相同：保留檔案原字串（AI(W906-RECIPE-BCB-R13) 20260926：現在是 BCB6 讀到的字 bcb；存檔時 collect 的 unshown() 換回 raw）
          } else {
            if (raw !== null) diff.push(id + '：檔案 ' + raw + ' → 機台 ' + w.text);
            el.value = w.text;
          }
          touched = true;
        }
        if (w.checked !== undefined)   touched = ctlSet(el, 'bool', w.checked ? '1' : '0') || touched;
        if (w.itemIndex !== undefined && !textSel) touched = ctlSet(el, 'index', String(w.itemIndex)) || touched;
        if (w.caption !== undefined && !('value' in el) && !el.children.length) {
          el.textContent = w.caption; touched = true;
        }
        if (w.visible === false) { el.style.visibility = 'hidden'; touched = true; }
        if (w.enabled === false) {
          if ('disabled' in el) el.disabled = true;
          el.setAttribute('aria-disabled', 'true');
          touched = true;
        }
        if (touched) {
          n++;
          el.setAttribute('data-src', 'cpp');
          el.title = (el.title ? el.title + '\n' : '') + '值來自 C++ ' + d.form + '::DoIniDataToForm()';
        }
      });
      var note = 'C++ ' + d.form + '::DoIniDataToForm() 送來 ' + (d.assignedCount || 0) +
                 ' 個 widget，套用 ' + n + ' 個';
      if (diff.length) note += '\n⚠ 其中 ' + diff.length + ' 個與檔案不同（顯示機台在用的值；存檔會寫回）：\n  ' +
                               diff.join('\n  ');
      if (miss.length) note += '\n⚠ 頁面上沒有的 id (' + miss.length + ')：' + miss.join(', ');
      return { note: note, diff: diff.length };
    }, function (e) {
      if (/HTTP 404/.test(e.message)) return null;   // 這頁還沒接 C++ 表單：只讀檔，正常
      return { note: '⚠ /api/form 讀取失敗：' + e.message + '（本頁只顯示檔案內容）' };
    });
  }

  function collect(doc) {
    var map = allMap(), edits = {}, sections = LAST[doc] || {};
    Object.keys(map).forEach(function (id) {
      if (map[id][0] !== doc) return;
      var el = $(id);
      if (!el) return;
      var sec = map[id][1], key = map[id][2];
      // optional：只送「這份配方真的有」的鍵。這是 notFound 良性與惡性的分界線 --
      // 惡性（對照表抽錯）仍然會走到 notFound 並被規則 2 擋下，保護沒有變弱。
      if (isOptional(id) && !(sections[sec] && sections[sec][key])) return;
      (edits[sec] = edits[sec] || {})[key] = unshown(sections[sec] && sections[sec][key], String(el.value));   // AI(W906-RECIPE-BCB-R13) 20260926
    });
    return edits;
  }

  function collectSysIni(file) {
    // Steven 20260916：文字與非文字控制項共用同一份 edits，形狀不變
    //（{區段: {鍵: 字串}}），所以 preview / 確認 / 寫入 / 重讀完全不用改。
    var edits = {};
    [CFG.sysFields || {}, CFG.sysEnums || {}].forEach(function (m) {
      Object.keys(m).forEach(function (id) {
        if (m[id][0] !== file) return;
        if (UNFILLABLE[id]) return;   // Steven 20260916（B1）：讀不進去的格子不送出
        var el = $(id); if (!el) return;
        var v = ctlGet(el, sysKind(id));
        // 讀不到值就整個不送。少寫一個鍵是安全的，寫一個 "undefined" 不是。
        if (v === null || v === undefined) return;
        (edits[m[id][1]] = edits[m[id][1]] || {})[m[id][2]] = unshown(((LAST['sys:' + file] || {})[m[id][1]] || {})[m[id][2]], String(v));   // AI(W906-RECIPE-BCB-R13) 20260926
      });
    });
    return edits;
  }
  function collectSysCsv(file) {
    var edits = {}, m = CFG.sysRows || {};
    Object.keys(m).forEach(function (id) {
      if (m[id][0] !== file) return;
      var el = $(id); if (!el) return;
      (edits[m[id][1]] = edits[m[id][1]] || {})[m[id][2]] = String(el.value);
    });
    // Steven 20260916：表格模式只送「改過的格子」（見 sysGrid 註解第 2 點）。
    if (G && G.file === file) {
      Object.keys(G.edits).forEach(function (k) {
        Object.keys(G.edits[k]).forEach(function (c) {
          (edits[k] = edits[k] || {})[c] = String(G.edits[k][c]);
        });
      });
    }
    return edits;
  }
  //Steven 20260916 (W906-FW-LEVELSET)
  // { <AccessLevel 索引>: <0..n> }，只收「有選中且與檔案現值不同」的格子。
  function collectSysLevels() {
    var edits = {}, lv = levelScan();
    if (!lv) return edits;
    var was = LAST['sys:' + lv.file] || [];
    lv.binds.forEach(function (b) {
      if (UNFILLABLE['levelset[' + b.idx + ']']) return;   // 讀不進去的格子不送出
      var sel = -1;
      b.inputs.forEach(function (r, i) { if (r.checked) sel = i; });
      if (sel < 0) return;                                  // 一顆都沒選 -> 不送，不猜
      if (was[b.idx] === sel) return;                       // 沒改過 -> 不送
      edits[b.idx] = sel;
    });
    return edits;
  }

  /* Steven 20260924 (S12 第二型)：golden 原檔產生的存檔 bridge（WS form.save）。
   * 只在 /api/form 回 saveable:true 時走（有 saveFlow 且沒有讀檔端缺口）。
   * 欄位清單是 C++ 給的 saveReads —— golden SaveSetupFile 讀到的全部。golden 會把整頁寫回檔案，
   * 所以任何一個取不到值就整頁拒寫，不送半套。確認框照規則 1。 */
  function widgetValue(el) {
    if (el.tagName === 'SELECT') {
      var o = el.options[el.selectedIndex];
      return { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
    }
    var cb = el.tagName === 'INPUT' && el.type === 'checkbox' ? el : el.querySelector('input[type="checkbox"]');
    if (cb && !('value' in el && el.tagName === 'INPUT' && el.type !== 'checkbox')) return { checked: cb.checked };
    var rs = el.querySelectorAll ? el.querySelectorAll('input[type="radio"]') : [];
    if (rs.length) {
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
      return { itemIndex: -1 };
    }
    if ('value' in el) return { text: String(el.value) };
    return null;
  }
  function bridgeSave() {
    // Steven 20260924 審查更正（高級審查員 #3）：golden SaveSetupFile 會把整頁寫回檔案，所以只送「這次載入有可信來源」
    // 的欄位：C++ display 設過值屬性（text／checked／itemIndex），或從檔案接線填過（allMap／系統檔對照）。
    // 沒有來源的（例：TesterIF 的 AntiSignalCBox，golden 在 FormShow 才填）與 C++ 值填不進畫面的（例：選項對不到）
    // **不送**。伺服器 form.save 會先空跑 golden 存檔，golden 真的讀到沒送的值就整筆拒寫並列出 ——
    // 條件分支裡才讀的欄位只有伺服器知道這次會不會讀到，所以判斷放在那邊。
    var ids = FORM.saveReads || [], widgets = {}, map = allMap(), skipped = [];
    ids.forEach(function (id) {
      var el = $(id), fw = FORM.widgets && FORM.widgets[id];
      var cppVal = fw && (fw.text !== undefined || fw.checked !== undefined || fw.itemIndex !== undefined);
      if (!el || BRIDGE_UNFILLED[id] || !(cppVal || map[id] || sysEntry(id))) { skipped.push(id); return; }
      var v = widgetValue(el);
      if (v) widgets[id] = v; else skipped.push(id);
    });
    if (!GB_NO_ASK[FORM.form] && !window.confirm('即將以 golden ' + FORM.form + ' 的存檔流程寫入 ' + ids.length + ' 個欄位。\n確定要寫入嗎？')) {   // AI(W906-MOTB-NOASK) 20261002: the same GB_NO_ASK table, keyed by the golden form class for this A-shape (/api/form) save -- TfHotPlate (cHotPlate.cpp:440 spbSaveClick asks nothing)
      say('已取消，未寫入。', '#ffcc66', 'transient');
      return Promise.resolve(null);
    }
    say('寫入中（golden 存檔流程）...');
    return HT9045Recipe.formSave(CFG.page, widgets).then(function (ack) {
      var a = (ack && ack.value) || ack || {};
      if (typeof a === 'string') { try { a = JSON.parse(a); } catch (e) { a = {}; } }
      // 高級審查員第二輪：golden 存檔鈕可能沒走到 SaveSetupFile 就 return（例：兩個 HotPlate 都沒勾）——照 ack.saved 講
      var lines = [a.saved === false ? '⚠ 沒有寫入（golden ' + FORM.form + ' 的存檔流程在寫檔前就結束了，原因見下方訊息）'
                                     : '✔ 已寫入（golden ' + FORM.form + '）'];
      (a.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });
      if ((a.todo || []).length) lines.push('⚠ 這次存檔 golden 還有沒做到的步驟：\n  ' + a.todo.join('\n  '));
      say(lines.join('\n'), a.saved === false ? '#f88' : ((a.todo || []).length ? '#ffcc66' : '#9f9'));
      return load();                                   // 規則 3：寫完一定重讀
    }, function (e) {
      say('❌ 寫入失敗：' + e.message + tokenHint(e.message), '#f88');   // AI(W906-SCREEN-TOKEN) 20261001: tokenHint
      return null;
    });
  }

  function save() {
    if (gbStruct()) return gbSave();                                        // Steven 20260924：C 路
    if (FORM && FORM.kind === 'golden-bridge' && FORM.saveable) return bridgeSave();   // Steven 20260924 (S12)
    var docs = Object.keys(byDoc(allMap()));
    if (!docs.length && !hasSys()) return Promise.resolve();
    // Steven 20260916（審查 B1）：有任何一格在讀取時填不進控制項，整頁拒寫。
    // 那一格的現值已經不是檔案的值，送出去就是「改到沒動的值」。
    var unf = Object.keys(UNFILLABLE);
    if (unf.length) {
      say('❌ 拒絕寫入：有 ' + unf.length + ' 個欄位讀取時填不進畫面控制項（值超出選項數），' +
          '存檔會把錯的值寫回去。\n' + unf.map(function (k) { return '  ' + k + ': ' + UNFILLABLE[k]; }).join('\n') +
          '\n請先修正 HTML 選項或檔案的值，再重讀。', '#f88');
      return Promise.resolve(null);
    }
    // Steven 20260916 (W906-FW-LEVELSET)：對照表對不上就整頁拒寫。
    if (LV_BROKEN) {
      say('❌ 拒絕寫入：權限對照表對不上 HTML，這一頁沒有載入過任何值。\n  ' +
          LV_BROKEN.join('\n  '), '#f88');
      return Promise.resolve(null);
    }
    say('預演中（dryRun，不會寫入）...');
    var plan = [], tooBig = [];   // Steven 20260916 tooBig
    var pre = docs.map(function (doc) {
      var edits = collect(doc);
      return HT9045Recipe.preview(doc, edits).then(function (p) { plan.push({ doc: doc, edits: edits, p: p }); });
    });
    Object.keys(sysIniByFile()).forEach(function (f) {
      var e = collectSysIni(f);
      pre.push(HT9045System.preview(f, e).then(function (p) {
        plan.push({ doc: 'system:' + f, sys: f, edits: e, p: p });
      }));
    });
    Object.keys(sysCsvByFile()).forEach(function (f) {
      var e = collectSysCsv(f);
      // Steven 20260916：表格模式沒有改過任何格子時 edits 是空的；送空 payload
      // 伺服器會回 "no rows/<key>/<column> fields"，變成一個假的寫入失敗。直接略過。
      if (!Object.keys(e).length) return;
      // Steven 20260916：伺服器單則 WS 訊息上限 64 KB（WebBridgeServer.cpp kMaxWsMessage），
      // 超過會直接斷線而不是回錯。一次改幾十格遠不到，但與其讓人看到「連線中斷」，
      // 不如在這裡先講清楚要分批。
      // Steven 20260916（審查 B6）：量「實際上線的訊框」的 UTF-8 長度——外層 envelope
      // 把 edits 再 JSON 字串化一次，每個引號多一個反斜線；原本只量 edits 的字元數會低估。
      var envelope = JSON.stringify({type: 'cmd', id: 999999, cmd: 'system.file.put', tag: f,
                                     value: JSON.stringify({rows: e, dryRun: false})});
      var bytes = (typeof TextEncoder !== 'undefined') ? new TextEncoder().encode(envelope).length
                                                        : unescape(encodeURIComponent(envelope)).length;
      if (bytes > 60 * 1024) {
        tooBig.push(f + '（約 ' + Math.round(bytes / 1024) + ' KB）');
        return;
      }
      pre.push(HT9045System.preview(f, e, {csv: true}).then(function (p) {
        plan.push({ doc: 'system:' + f, sys: f, csv: true, edits: e, p: p });
      }));
    });
    // Steven 20260916 (W906-FW-LEVELSET)：i32 投影的預演。
    // 只送「畫面上有選中、而且與上次讀到的值不同」的索引 —— 稀疏寫入是伺服器
    // 那側的契約，整批回送會把這台機器可能有的 5 階值（4）壓成 4 階畫面能表達的東西。
    if (CFG.sysLevels && CFG.sysLevels.file && !LV_BROKEN) {
      var lvEdits = collectSysLevels();
      if (Object.keys(lvEdits).length) {
        pre.push(HT9045System.levels(CFG.sysLevels.file, lvEdits, {dryRun: true}).then(function (p) {
          plan.push({ doc: 'system:' + CFG.sysLevels.file, levels: CFG.sysLevels.file, edits: lvEdits, p: p });
        }));
      }
    }
    if (tooBig.length) {
      say('❌ 一次變更太多，超過伺服器單則訊息上限：' + tooBig.join('、') +
          '\n請先存一部分（Save 後表格會重讀），再改剩下的。', '#f88');
      return Promise.resolve(null);
    }
    if (!pre.length) { say('沒有任何值改變，不寫入。', '#9f9', 'transient'); return Promise.resolve(null); }
    return Promise.all(pre).then(function () {
      /* Steven 20260916
       * -----------------------------------------------------------------------
       * ack 回的是「計數」，不是鍵的陣列。
       * -----------------------------------------------------------------------
       * 原本這裡寫 `(x.p.notFound || []).forEach(...)`。伺服器那側從來沒有把
       * notFound / changed 送出來（WebBridgeServer.cpp 的 AckJson 在成功時把
       * 整個結果物件丟掉了，20260916 修），所以兩個欄位都是 undefined：
       *   notFound -> [] -> 規則 2 永遠不會觸發
       *   changed  -> [] -> save() 永遠停在「沒有任何值改變，不寫入」
       * 也就是說按了存檔會看到綠色訊息，但什麼都沒寫。
       *
       * 修好之後它們是數字（RecipeWriteResult 只帶計數，不帶鍵名），
       * 所以這裡一律用 cnt() 正規化：數字就取值，陣列就取長度。
       * 這樣舊版伺服器（沒有這兩個欄位）也不會炸，只是退回舊行為。
       */
      function cnt(v) {
        if (typeof v === 'number') return v;
        if (v && typeof v.length === 'number') return v.length;
        return 0;
      }
      function keysOf(edits, flat) {
        var out = [];
        Object.keys(edits || {}).forEach(function (a) {
          // Steven 20260916 (W906-FW-LEVELSET)：i32 投影的 edits 是平的
          // （{索引: 值}），不是兩層的 {區段:{鍵:值}}。用兩層的走法會對數字
          // 跑 Object.keys，得到 [] 而讓確認視窗一個項目都不列。
          if (flat) { out.push('[' + a + '] ' + levelCaption(a) + ' -> ' + edits[a]); return; }
          Object.keys(edits[a]).forEach(function (b) { out.push(a + ' / ' + b); });
        });
        return out;
      }

      var nf = 0, nfWhere = [];
      plan.forEach(function (x) {
        var n = cnt(x.p && x.p.notFound);
        if (n) { nf += n; nfWhere.push(x.doc + ' (' + n + ')'); }
      });
      if (nf) {
        // 伺服器只回數量、不回是哪些鍵，所以這裡指出是哪一份文件，
        // 讓人拿著文件名去對 ht9045_wire_<slug>.js 找那一段。
        say('❌ 拒絕寫入：有 ' + nf + ' 個鍵在目標檔裡不存在（' + nfWhere.join('、') + '）。\n' +
            '這是對照表的錯，不是資料的錯 —— 請檢查 ht9045_wire_' + CFG.slug + '.js。', '#f88');
        return null;
      }

      var changed = 0, detail = [];
      plan.forEach(function (x) {
        var n = cnt(x.p && x.p.changed);
        if (!n) return;
        changed += n;
        detail.push(x.doc + '：' + n + ' 個值');
        var ks = keysOf(x.edits, !!x.levels);   // Steven 20260916
        ks.slice(0, 12).forEach(function (k) { detail.push('    ' + k); });
        if (ks.length - 12 > 0) detail.push('    ...（本次共送出 ' + ks.length + ' 個欄位）');
      });
      if (!changed) { say('沒有任何值改變，不寫入。', '#9f9', 'transient'); return null; }
      if (!CFG.noAsk && !window.confirm('即將寫入 ' + changed + ' 個值。\n\n' + detail.join('\n') +   // AI(W906-NOASK) 20261001: CFG.noAsk = golden saves this page without asking (e.g. TfSecurity::FormClose cSecurity.cpp:438-468, 0 MessageBox)
                          '\n\n（清單是本頁送出的欄位；伺服器回報其中 ' + changed +
                          ' 個與現值不同）\n\n確定要寫入嗎？')) {
        say('已取消，未寫入。', '#ffcc66', 'transient');
        return null;
      }
      say('寫入中 ...');
      // Steven 20260916（審查 B5）：逐檔 settle。原本 Promise.all 一檔 reject 就進 catch、
      // 不重讀，已經寫成功的那份檔畫面看不出來。現在成功失敗都重讀、都回報。
      return Promise.all(plan.map(function (x) {
        // Steven 20260916 (W906-FW-LEVELSET)：三條寫入路徑，levels 要排在
        // sys 之前判斷 —— x.levels 的項目沒有 x.sys，漏掉會掉進配方那條。
        var p = x.levels ? HT9045System.levels(x.levels, x.edits)
              : x.sys    ? HT9045System.write(x.sys, x.edits, {csv: !!x.csv})
                         : HT9045Recipe.write(x.doc, x.edits);
        return p.then(function (w) { return {doc: x.doc, ok: true, w: w}; },
                      function (e) { return {doc: x.doc, ok: false, err: e}; });
      })).then(function (rs) {
        return load().then(function () {
          var n = 0, bad = [];
          rs.forEach(function (r) {
            if (!r.ok) { bad.push(r.doc + '：' + ((r.err && r.err.message) || r.err)); return; }
            // Steven 20260916（審查 R2）：伺服器在 apply 階段拒寫時 ack 現在是 ok:false（會進上面那支），
            // 但保險起見也看 notFound——有它就不是成功。
            if (r.w && typeof r.w.notFound === 'number' && r.w.notFound > 0) {
              bad.push(r.doc + '：伺服器回報 notFound=' + r.w.notFound + '，未寫入'); return;
            }
            var v = r.w && r.w.changed;
            n += (typeof v === 'number' ? v : (v && v.length) || 0);
          });
          if (bad.length) say('⚠ 部分寫入失敗（其餘已寫入並重讀）：changed=' + n + '\n' + bad.join('\n'), '#ffcc66');
          else say('✅ 寫入完成並已重讀驗證：changed=' + n, '#9f9', 'transient');
          return rs;
        });
      });
    }).catch(function (e) {
      say('寫入失敗：' + e.message, '#f88');
      throw e;
    });
  }

  /* -------------------------------------------------------------------------
   * 實體按鍵：只在小鍵盤開著的時候有效
   * -------------------------------------------------------------------------
   * 規格（使用者 20260915 定案）：
   *   HTML 畫面本身不可以使用實體鍵盤；
   *   只有 QWERTY 小鍵盤出現時，才可以用「對應到小鍵盤按鍵」的實體鍵。
   *
   * 前半段由 attachKeyboards() 的 readonly 達成（欄位打不進字）。
   * 後半段就是這裡：偵測到 .qkOv 覆蓋層存在時，把實體按鍵轉成對應按鈕的 click，
   * 沒有覆蓋層就完全不處理（讓事件照常被忽略）。
   *
   * ⚠ 為什麼做在引擎不做在 qwerty.js：qwerty.js 是網頁作者的檔案，存在於鏡像
   *   來源，sync_web.py --apply 會整檔覆蓋。做在這裡才不會被洗掉。
   *   長久解法是網頁作者把這段收進 qwerty.js。
   *
   * 對應表用「按鈕上的文字」比對，不依賴 qwerty.js 的內部結構：
   *   可見字元 -> 文字相同的那顆      Backspace -> 一     Delete -> 'Delete'
   *   Enter -> 'Enter'                Escape -> 'Abort'   Space -> ' '
   */
  var PHYS_BOUND = false;

  function physicalKeys() {
    if (PHYS_BOUND) return;
    PHYS_BOUND = true;
    document.addEventListener('keydown', function (ev) {
      var ov = document.querySelector('.qkOv');
      if (!ov) return;                       // 小鍵盤沒開 -> 實體鍵一律無效
      var want = null;
      if (ev.key === 'Backspace')      want = '⌫';
      else if (ev.key === 'Delete')    want = 'Delete';
      else if (ev.key === 'Enter')     want = 'Enter';
      else if (ev.key === 'Escape')    want = 'Abort';
      else if (ev.key === ' ')         want = ' ';
      else if (ev.key && ev.key.length === 1) want = ev.key;
      if (want === null) return;

      var btns = ov.querySelectorAll('.qk'), hit = null, i;
      for (i = 0; i < btns.length; i++) {
        if (btns[i].textContent === want) { hit = btns[i]; break; }
      }
      // 大小寫：小鍵盤目前是小寫版時，按實體大寫字母要先能對上小寫那顆。
      if (!hit && want.length === 1 && /[A-Za-z]/.test(want)) {
        var alt = want === want.toLowerCase() ? want.toUpperCase() : want.toLowerCase();
        for (i = 0; i < btns.length; i++) {
          if (btns[i].textContent === alt) { hit = btns[i]; break; }
        }
      }
      if (!hit) return;                      // 小鍵盤上沒有這顆 -> 不接受
      ev.preventDefault();
      ev.stopPropagation();
      hit.click();
    }, true);                                // capture：搶在頁面自己的 handler 之前
  }

  function attachKeyboards() {
    if (typeof HTQwerty === 'undefined') return 0;
    var kb = CFG.kb || {}, n = 0, unmapped = [];
    var inputs = document.querySelectorAll('input[type="text"], input:not([type])');
    Array.prototype.forEach.call(inputs, function (el) {
      if (!el.id || el.disabled) return;
      var c = kb[el.id];
      var flags = c ? (HTQwerty.N[c[0]] || 0) : 0;
      var opt = c ? { dp: c[1], checkRange: !!c[2], min: c[3], max: c[4] } : {};
      if (!c) unmapped.push(el.id);
      // Steven 20260918：伺服器若在欄位上帶了 min／max 就以它為準。
      // 每次開小鍵盤的當下重算（不是綁定時算一次），因為 LAST 會在重讀之後更新 ——
      // 存檔→重讀之後打開小鍵盤，看到的必須是新的限值。
      opt.rangeHook = function () { return serverRange(el.id); };
      // AI(W906-FW-KB) 20260915：機台上沒有實體鍵盤，小鍵盤是唯一輸入途徑，
      // 所以單擊就開（與 golden 的 OnMouseDown 一致），並設 readonly 避免游標
      // 停在欄位裡卻打不了字。20260914 曾短暫改成雙擊＋可直接輸入，那是基於
      // 「開發機有實體鍵盤」的錯誤前提，已還原。
      el.addEventListener('mousedown', function (ev) {
        ev.preventDefault();  if (el.disabled || (el.closest && el.closest('[aria-disabled="true"]'))) return;   // AI(W906-KB-DISABLED) 20261002 (setupA): a disabled TEdit (or one inside a disabled container) gets no OnMouseDown in VCL -> no keypad; this used to open the generic QWERTY on a field golden keeps disabled (e.g. Setup.TesterIF edPowerSwitchDelay)
        // Steven 20260916：qwerty 直接改 el.value，不會觸發 input/change，
        // 頁面上靠這兩個事件過濾的搜尋框（IoSetView 的 edtSearchIO）就沒反應。
        // 提交後補發一次，讓既有的監聯器照常工作。
        var o = {};
        for (var k in opt) o[k] = opt[k];
        o.onCommit = function () {
          ['input', 'change'].forEach(function (evn) {
            var e2; try { e2 = new Event(evn, { bubbles: true }); }
            catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
            el.dispatchEvent(e2);
          });
        };
        HTQwerty.show(el, flags, o);
      });
      el.setAttribute('readonly', 'readonly');
      el.title = (el.title ? el.title + ' | ' : '') +
                 (c ? '小鍵盤 ' + c[0] + (c[2] ? ' (' + c[3] + '~' + c[4] + ')' : '')
                    : '小鍵盤（通用 QWERTY，無 golden 依據）');
      el.style.cursor = 'pointer';
      n++;
    });
    if (unmapped.length) {
      console.warn('[' + CFG.page + '] 這些輸入框沒有 golden 旗標依據，用預設全 QWERTY：', unmapped);
    }
    window.HT9045KbAudit = { page: CFG.page, bound: n, unmapped: unmapped };
    return n;
  }

  // AI(W906-PAGETAB-Q51) 20260928 [W906] H4（Q49 的 D；Steven S168「頁面表取代 fShow」第 5 步）：C 路頁只在視窗真的打開時才向 C++ 要資料。
  //   golden 每按一次開窗鈕才跑：查開窗閘 → 記 "Enter ..." → FormShow（例 V912 main.cpp:28306-28314 sbContactClick）。
  //   web\background.html 的非 lazy 視窗開站就把 iframe 載好藏著；以前 attach() 一跑就 load() ⇒ 開站那一刻每一頁都「開了一次」
  //   （R108：Enter 記在開站），之後操作員開窗只是取消隱藏，C++ 不再跑 FormShow。現在：
  //   * 嵌在外框裡 ⇒ 等外框的 HT_WIN（background.html postWinState：{type:'HT_WIN', id, open, state, initial}）說 open 才 load()；
  //   * 每一次 關→開 都再 load() 一次：C++ 在網頁回報「關了」的那一拍把這頁的「開過了」清掉（頁面表邊緣 →
  //     D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp 檔尾 W906_EditPageWindowClosed）⇒ 這一次 editlist.get＝重新開頁：
  //     開窗閘重查、記 Enter、golden FormShow。最小化不算關（open 仍是 true，golden 最小化也不跑 FormClose）。
  //   * 存檔後的自動重讀（規則 3）、重讀鈕照舊直接 load()，C++ 看「這一頁的視窗還開著」就不再記 Enter（R110）。
  //   * 3 秒沒收到任何 HT_WIN（舊外框、單獨開這一頁、被別的頁嵌著）⇒ 照舊直接 load()。
  //   監聽器在模組載入時就掛：外框在 iframe load 事件補送的 HT_WIN {initial:true} 可能比 attach() 早到。樣板：HW.MotorTest.html:995-1019。
  var H4 = { hosted: false, open: false, waiting: false, timer: null };
  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    H4.hosted = true;
    if (H4.timer) { clearTimeout(H4.timer); H4.timer = null; }
    if (m.open && !H4.open) { H4.open = true; if (H4.waiting) load(); }
    else if (!m.open) H4.open = false;
  });

  function attach() {
    physicalKeys();
    // Steven 20260918 (W906-FW-OBSERVER)：先把沒有來源的格子寫成 '---'。
    // ⚠ 順序有意義 —— 必須在任何非同步讀取之前，也在 attachTags() 之前。
    //   HTML 裡寫死的展示值（dfm 匯出的 'NA'、模擬器留下的數字）在這一刻就要
    //   消失；晚一步，使用者會先看到一個假值，再看到它被換掉。
    NOSRCN = applyNoSource();
    KBN = attachKeyboards();
    TAGN = attachTags();   // Steven 20260916 (W906-FW-TAGSUB)
    if (Object.keys(allMap()).length || hasSys()) {
      /* Steven 20260916（使用者要求）
       * -----------------------------------------------------------------------
       * 不再注入浮動的 "Save to recipe" / "Reload" 兩顆鈕。
       * -----------------------------------------------------------------------
       * 它們原本是「找不到頁面自己的存檔鈕就自己生一顆」的退路，用 position:fixed
       * 蓋在畫面上，每一頁的同一個位置都冒出兩顆不屬於那張 dfm 的按鈕。
       *
       * ⚠ 直接拿掉會讓 16 頁靜默失去存檔能力 —— 它們有自己的存檔鈕，只是 id 不在
       *   舊的猜測清單（spbSave/btSave/btnSave/btOK/btApply）裡。所以按鈕來源改成
       *   **接線檔明寫**：
       *       saveBtn:   '<id>'      存檔鈕
       *       reloadBtn: '<id>'      重讀鈕（沒有就不提供重讀）
       *   查詢順序：CFG.saveBtn -> sysGrid.save -> 舊的猜測清單（相容既有頁面）。
       *
       * 找不到任何存檔鈕時**不再自己生一顆**，而是在狀態列講清楚這一頁是唯讀。
       * 沉默地少一顆按鈕，跟沉默地多一顆假按鈕，一樣糟。
       */
      var grid = CFG.sysGrid || null;
      var btn = (CFG.saveBtn && $(CFG.saveBtn)) ||
                (grid && grid.save && $(grid.save)) ||
                $('spbSave') || $('btSave') || $('btnSave') || $('btOK') || $('btApply');
      var relBtn = (CFG.reloadBtn && $(CFG.reloadBtn)) ||
                   (grid && grid.reload ? $(grid.reload) : null);
      var addBtn = grid && grid.add ? $(grid.add) : null;      // Steven 20260916
      var delBtn = grid && grid.del ? $(grid.del) : null;      // Steven 20260916

      // Steven 20260916：一律走 document 捕獲階段攔截，不再只有表格頁這樣做。
      // 這些鈕上大多還掛著頁面 legacy 的 click（JSON 快照那套，已裁決退場）；
      // 用 addEventListener 掛在按鈕上會變成兩個處理器都跑。捕獲階段先攔，
      // 不讓事件走到按鈕本身，legacy 那支就不會被觸發。
      // Steven 20260925：一頁只掛一個攔截處理器。頁面若載入兩份接線檔（例 ht9045_wire_speed.js ＋
      // ht9045_wire_setupspeed.js 各 register 一次），以前每次 attach 都再掛一個 → 按一次存檔跑兩次 golden 存檔。
      // 按鈕放模組層變數（CAP），處理器讀最新的那一組。
      CAP.btn = btn; CAP.relBtn = relBtn; CAP.addBtn = addBtn; CAP.delBtn = delBtn;
      if ((btn || relBtn || addBtn || delBtn) && !CAP.installed) {
        CAP.installed = true;
        document.addEventListener('click', function (ev) {
          var t = ev.target && ev.target.closest ? ev.target.closest('button,a,[id]') : null;
          if (!t) return;
          if (CAP.btn && t === CAP.btn)            { ev.stopPropagation(); ev.preventDefault(); save(); }
          else if (CAP.relBtn && t === CAP.relBtn) { ev.stopPropagation(); ev.preventDefault(); load(); }
          else if (CAP.addBtn && t === CAP.addBtn) { ev.stopPropagation(); ev.preventDefault(); gridAddRow(); }
          else if (CAP.delBtn && t === CAP.delBtn) { ev.stopPropagation(); ev.preventDefault(); gridDeleteRow(); }
        }, true);
      }
      NOSAVE = !btn;
      // Steven 20260916：把「哪一顆是存檔鈕」講出來。
      // 不再有固定位置的浮動鈕之後，操作員沒有別的方法知道該按哪一顆；
      // 而且引擎是在捕獲階段攔截，那顆鈕原本的行為（例如 Exit 會關閉）不會發生，
      // 這件事更必須寫在畫面上而不是只寫在接線檔的註解裡。
      SAVEBTN_NOTE = btn ? (btn.id || '(無 id)') : '';
    }
    // AI(W906-PAGETAB-Q51) 20260928 [W906] H4：C 路頁嵌在外框裡 ⇒ 視窗打開才讀（見上面 var H4 的說明）；其他頁照舊一 attach 就讀。
    if (gbStruct() && window.parent && window.parent !== window) {
      H4.waiting = true;
      if (H4.open) { load(); return; }
      if (!H4.hosted && !H4.timer) H4.timer = setTimeout(function () { H4.timer = null; if (!H4.hosted) load(); }, 3000);
      return;
    }
    load();
  }

  function register(cfg) {
    CFG = cfg;
    CFG.fields = CFG.fields || {};
    CFG.optional = CFG.optional || {};
    CFG.sysFields = CFG.sysFields || {};
    CFG.sysEnums = CFG.sysEnums || {};   // Steven 20260916
    CFG.sysText = CFG.sysText || {};     // Steven 20260918：系統檔唯讀顯示（不進 save）
    CFG.noSource = CFG.noSource || {};   // Steven 20260918：明說「沒有來源」的格子
    CFG.sysRows = CFG.sysRows || {};
    CFG.sysGrid = CFG.sysGrid || null;   // Steven 20260916
    CFG.sysLevels = CFG.sysLevels || null;   // Steven 20260916 (W906-FW-LEVELSET)
    CFG.tags = CFG.tags || {};               // Steven 20260916 (W906-FW-TAGSUB)
    CFG.saveBtn = CFG.saveBtn || '';         // Steven 20260916：頁面自己的存檔鈕 id
    CFG.reloadBtn = CFG.reloadBtn || '';     // Steven 20260916：頁面自己的重讀鈕 id
    CFG.saveBtnNote = CFG.saveBtnNote || ''; // Steven 20260916：存檔鈕的額外說明（例如 Exit 兼存檔）
    NOSAVE = false; SAVEBTN_NOTE = ''; NOSRCN = 0;
    LV = null; LV_BROKEN = null;             // 重新註冊 -> 對照表重掃
    CFG.slug = CFG.slug || 'page';
    window.HT9045Page = { load: load, save: save, collect: collect, cfg: CFG,
                          // Steven 20260924 (S12)：舊的 per-page loader 讀完檔後呼叫它；form() 給探針看最近一次回應
                          formOverlay: formOverlay, form: function () { return FORM; },
                          golden: function () { return { struct: gbStruct(), page: GB, kinds: GB_KIND, lastSave: GB_LAST }; },   // Steven 20260924：C 路
                          grid: function () { return G; },     // Steven 20260916：給探針/除錯看
                          // Steven 20260916（審查 D）：讓 DOM 級探針能拿到「引擎真正會送出的東西」
                          collectSysIni: collectSysIni, collectSysCsv: collectSysCsv,
                          // Steven 20260916 (W906-FW-LEVELSET)：同上，讓探針看得到
                          // 「引擎真正會送出的索引與值」與掃出來的對照表。
                          collectSysLevels: collectSysLevels,
                          levels: function () { return { scan: levelScan(), broken: LV_BROKEN }; },
                          unfillable: function () { return UNFILLABLE; } };
    if (typeof HT9045Recipe === 'undefined' && (Object.keys(allMap()).length || hasSys() || gbStruct())) {
      say('❌ 沒有載入 ht9045_recipe_client.js -- 請在本檔之前先載入它。', '#f88');
      return;
    }
    if (document.readyState === 'loading') {
      // Steven 20260925：DOM 還沒好時多次 register（兩份接線檔）只排一次 attach，用最後一份 CFG
      if (!ATTACH_PENDING) { ATTACH_PENDING = true; document.addEventListener('DOMContentLoaded', function () { ATTACH_PENDING = false; attach(); }); }
    } else {
      attach();
    }
  }
  var ATTACH_PENDING = false;
  var CAP = { installed: false, btn: null, relBtn: null, addBtn: null, delBtn: null };

  /* AI(W906-R76) 20260927：VCL 的 TRadioButton 在同一個父容器裡互斥；dfm2web 產出的
   *   <label class="ckb" id="rbXxx"><input type="radio"></label>
   * 沒有 name，瀏覽器就不會互斥 —— 例：Setup.YieldMonitoring 的 On／Off 點過 On 就關不掉，
   * 存進去還是 On（golden uYieldMonitoring.cpp:1891／:1898／:1903／:1907 讀 On->Checked）。
   * 這裡照 VCL 的規則：沒有 name 的 radio，以「它的 label（那顆 TRadioButton）的父元素」
   * 為一組、補同一個 name。已經有 name 的（TRadioGroup 的 rg_*、Status.Security 的 sec_*）不動。
   * 為什麼修在引擎、不修 dfm2web 產生器：頁面產生後都被手改過，重跑產生器不是現在的流程；
   * 引擎一處修好所有載入它的頁（St01 R76，筆電 0927 決定）。
   * 0927 量過的分組：Yield 4 組（各一對 On／Off）、SetUp 2 組＋2 顆單獨、Temp_Set 2 組＋1 顆、
   * Contact 1 組模式選擇＋1 對＋1 顆、StartCondition 1 組、Configuration 1 組 —— 都是一個群組框一組。 */
  var RB_SEQ = 0;
  function groupBareRadios(root) {
    var rs = (root || document).querySelectorAll('input[type="radio"]:not([name])');
    for (var i = 0; i < rs.length; i++) {
      var r = rs[i];
      var btn = r.parentNode;
      while (btn && btn.tagName !== 'LABEL' && btn !== document.body) btn = btn.parentNode;
      var host = (btn && btn.tagName === 'LABEL') ? btn.parentNode : r.parentNode;
      if (!host || !host.getAttribute) continue;
      var g = host.getAttribute('data-ht-rbgroup');
      if (!g) { g = 'htrb' + (++RB_SEQ) + (host.id ? '_' + host.id : ''); host.setAttribute('data-ht-rbgroup', g); }
      r.name = g;
    }
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', function () { groupBareRadios(document); });
  else groupBareRadios(document);

  window.HT9045Wire = { register: register, say: say };
})();
