/* ht9045_setup_c_wire.js -- Setup.SetUp.html（golden TfSetup，cSetUp.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_SetUp.cpp（WS editlist.get／editlist.save tag=TestIF_File_SetUp）。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE）照通用規則讀寫；本檔只補通用規則做不到的四件事：
 *
 *  (1) rgSensor1..N：golden 建構子執行期產生（cSetUp.cpp:201-215，MyTempRGBox[i]=new TRadioGroup(this)，
 *      Parent=scrlbxSocketSensor、Name="rgSensor"+(i+1)、Height=36、Caption="Sensor %d usage"、Columns=3、
 *      Items＝No use／Has IC／Floating、Align=alTop、OnClick=rgSensor1Click）。DFM 沒有 → 產生器畫不出來。
 *      這裡在 editlist.get 回應交給引擎「之前」依回應的 rgSensor* 建好，引擎就照通用規則套值／可見／可改，
 *      存檔也照通用規則送回（mustSend）。N 取回應（＝後端的 iSnSocketCnt），不寫死。
 *      Align=alTop：看不見的不佔位 → 用 display（引擎只改 visibility，套完再轉成 display）。
 *  (2) golden 兩個小事件搬到瀏覽器（純畫面，存檔時後端 golden sbUpdateClick 仍重新檢查）：
 *        CoSocketComboChange（cSetUp.cpp:4785）：顯示前 atoi(Text) 顆，顯示的設 bSocketSensorCheckFloating?2:1，其餘 0
 *        rgSensor1Click（cSetUp.cpp:4805）：看得見的選到 No use（0）→ 改回 Has IC（1）
 *  (3) ht9045_setup_sitemap.js（B 路時代的 Site Mode 行為檔：捲軸／CompChange／排序鈕）與 C 路共存：
 *        - 它開頁時自己讀 HandlerCondition.Data（B 路 read）再跑一次 ScrollBar1Change —— 這裡把那一次 read
 *          換成「後端 golden FormShow 之後的值」（Test Mode＝ScrollBar1.Position、Site Xx＝cbXx.ItemIndex），
 *          並讓引擎等它跑完才套後端的值 → 畫面以後端（C 路）為準，它只負責之後的互動。
 *        - 它的捲軸不寫 ScrollBar1 元件的 value → 存檔時由這裡補 ScrollBar1.position：
 *          操作員動過捲軸才送它的位置，否則原樣送後端給的位置（它的客戶端夾限不能偷偷改 Test Mode）。
 *        - 下拉選項數比後端 ItemIndex 少時（客戶端 CompChange 的 CH 數和後端不同），補選項讓 ItemIndex 對得上，
 *          不然引擎套不上、存檔會把錯的 ItemIndex 送回去。
 *  (4) 產生器把 DFM Visible=False 的元件寫成 display:none，引擎只改 visibility → 後端說 visible 的元件補回 display。
 *  (5) Steven 團隊 20260925（後端 site map 修正同批）：
 *        - Site 格子 cbAa..cbDh 的選項照後端 golden CompChange（cSetUp.cpp:1203）執行期建的那份重建
 *          （editlist.get 回應 extra.items；"- - -"、"CH 1".."CH n"，看不見的格子是空的）—— DFM 的 "1".."8" 不是執行期選項。
 *        - Site Map 排序鈕（btnLUpToRDownZ／…N 六顆，golden btnLUpToRDownNClick :4200 設 IniConfig.iSiteMapDirection，
 *          SaveSetupFile :3851 寫 SiteMapDirection）：記下最後按的那一顆，存檔時 widgets 多送 "<鈕名>":{"click":true}，
 *          後端在套值前照 golden 重播那一下（FileRW/TestIF_File_SetUp.cpp BeforeApply）。
 *        - 後端存檔前會先照 golden 重播 ScrollBar1Change／CoSocketComboChange（值和開頁不同時），再判斷哪些元件可改：
 *          CoSocketCombo 改大之後新顯示的 rgSensor 這次存檔就收得到 → 這裡把它們打開（容器 scrlbxSocketSensor 可改時）。
 *
 * golden 存檔鈕 sbUpdateClick（:3492-3660）沒有 YES/NO 確認框 → 不登錄引擎 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 客戶專屬條件（CC_ASE_KaohSiung_K3、CC_SCC、CC_ASE_CL…）：Steven 20260925 決定先跳過，這裡不做任何客戶碼分支，
 * 可見與否全由後端 golden FormShow 決定。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_SetUp';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__setupCWrapped) return;
  R.__setupCWrapped = true;

  var ROWCH = 'ABCD', COLCH = 'abcdefgh';
  var get0 = R.editlistGet, save0 = R.editlistSave, read0 = R.read;
  var LAST = null;                     // 最近一次 editlist.get 的回應
  var dataWait = [];                   // 等第一次回應的 resolve
  var smReady = false, smWait = [];    // sitemap.js 開頁流程跑完了沒
  var posAtLoad = null;                // 開頁（套完後端值）時 sitemap 捲軸的位置
  var serverPos = null;                // 後端給的 ScrollBar1.Position
  var floating = null;                 // TestIF_File.bSocketSensorCheckFloating（CoSocketComboChange 用）
  var notes = [];                      // 這次開頁要補在狀態列的說明
  var lastSort = null;                 // 開頁後最後按的 Site Map 排序鈕（存檔時送 {"click":true}）
  var SORT_BTNS = ['btnLUpToRDownZ', 'btnLDownToRUpZ', 'btnRUpToLDownZ', 'btnRDownToLUpZ', 'btnLUpToRDownN', 'btnRUpToLDownN'];

  function $(id) { return document.getElementById(id); }
  function SM() { return window.HT9045SetupSiteMap || null; }
  function sensorIds(d) {
    var ids = {};
    Object.keys((d && d.proxies) || {}).concat((d && d.mustSend) || []).forEach(function (k) {
      if (/^rgSensor\d+$/.test(k)) ids[k] = parseInt(k.slice(8), 10);
    });
    return Object.keys(ids).sort(function (a, b) { return ids[a] - ids[b]; });
  }

  /* ---- (1) rgSensor1..N ------------------------------------------------- */
  function buildSensors(d) {
    var box = $('scrlbxSocketSensor');
    if (!box) { notes.push('❌ 找不到 scrlbxSocketSensor（golden rgSensor 的 Parent），Socket Sensor 用途無法顯示'); return; }
    sensorIds(d).forEach(function (id) {
      if ($(id)) return;
      var n = id.slice(8);
      var fs = document.createElement('fieldset');
      fs.className = 'gbx rg';
      fs.id = id;
      fs.title = id + ' : TRadioGroup（golden cSetUp.cpp:203 執行期建立）';
      // Align=alTop、Height=36：由上往下排，寬度貼滿 ScrollBox
      fs.style.cssText = 'position:relative;display:block;height:36px;margin:0;';
      var lg = document.createElement('legend');
      lg.style.background = 'var(--form-bg,#ece9d8)';
      lg.textContent = 'Sensor ' + n + ' usage';
      fs.appendChild(lg);
      var cli = document.createElement('div');
      cli.className = 'cli';
      // Columns=3：三個選項一列
      cli.style.cssText = 'position:absolute;left:4px;top:14px;right:2px;bottom:2px;display:grid;' +
                          'grid-template-columns:repeat(3,1fr);align-content:start;';
      ['No use', 'Has IC', 'Floating'].forEach(function (t) {
        var lb = document.createElement('label');
        lb.className = 'rgi';
        var r = document.createElement('input');
        r.type = 'radio';
        r.name = 'rg_' + id;
        lb.appendChild(r);
        lb.appendChild(document.createTextNode(t));
        cli.appendChild(lb);
      });
      fs.appendChild(cli);
      box.appendChild(fs);
      // golden rgSensor1Click（:4805）：看得見的不能是 No use
      cli.addEventListener('change', function () {
        var rs = cli.querySelectorAll('input[type="radio"]');
        if (fs.style.display !== 'none' && rs[0].checked) { rs[0].checked = false; rs[1].checked = true; }
      });
    });
  }
  function sensorRadios(id) { var el = $(id); return el ? el.querySelectorAll('input[type="radio"]') : []; }
  function setSensor(id, show, idx) {
    var el = $(id);
    if (!el) return;
    el.style.display = show ? 'block' : 'none';
    el.style.visibility = '';
    var rs = sensorRadios(id);
    for (var i = 0; i < rs.length; i++) rs[i].checked = (i === idx);
  }
  // golden CoSocketComboChange（:4785）
  function coSocketComboChange() {
    var cb = $('CoSocketCombo');
    if (!cb || !LAST) return;
    var o = cb.options[cb.selectedIndex];
    var cnt = parseInt(o ? o.textContent : '', 10);          // atoi(CoSocketCombo->Text.c_str())
    if (isNaN(cnt)) cnt = 0;
    // 後端存檔時先照 golden 重播 CoSocketComboChange、再判斷可不可改（FileRW/TestIF_File_SetUp.cpp BeforeApply）→
    // 新顯示的 rgSensor 這次存檔收得到：開頁時只因「看不見」被引擎停用的，這裡打開（容器 scrlbxSocketSensor 與
    // CoSocketCombo 可改、它自己 Enabled 時；其他原因停用的不動）
    var px = LAST.proxies || {}, box = px.scrlbxSocketSensor, co = px.CoSocketCombo;
    var canOpen = (!box || box.editable !== false) && (!co || co.editable !== false);
    sensorIds(LAST).forEach(function (id, i) {
      var show = i < cnt;
      setSensor(id, show, show ? (floating ? 2 : 1) : 0);
      var el = $(id), p = px[id] || {};
      if (show && el && canOpen && p.enabled !== false) {
        el.querySelectorAll('input[data-gb-dis="1"]').forEach(function (x) { x.disabled = false; x.removeAttribute('data-gb-dis'); });
        el.removeAttribute('aria-disabled');
      }
    });
  }

  /* ---- (5) Site 格子選項＝後端 golden CompChange；排序鈕 ------------------ */
  function siteItems(d) {
    var items = d && d.extra && d.extra.items, done = {};
    if (!items) { notes.push('⚠ 後端回應沒有 extra.items（golden CompChange 的 Site 選項），Site 格子沿用網頁自己的選項'); return done; }
    Object.keys(items).forEach(function (id) {
      var el = $(id);
      if (!el || el.tagName !== 'SELECT') return;
      el.innerHTML = '';
      items[id].forEach(function (t, k) {
        var o = document.createElement('option');
        o.value = String(k);
        o.textContent = t;
        o.setAttribute('data-src', 'cpp-items');
        el.appendChild(o);
      });
      el.selectedIndex = -1;                 // 值等一下由引擎照後端 itemIndex 套
      done[id] = true;
    });
    return done;
  }
  function hookSortButtons() {
    SORT_BTNS.forEach(function (id) {
      var b = $(id);
      if (!b || b.__setupC) return;
      b.__setupC = true;
      // sitemap.js 自己的 click（客戶端 sortSiteMap）照舊；這裡只記下是哪一顆，停用的不記（後端也會照權限丟掉）
      b.addEventListener('click', function () {
        if (b.getAttribute('aria-disabled') === 'true' || b.disabled) return;
        lastSort = id;
      });
    });
  }
  function readFloating() {
    // golden 用記憶體裡的 TestIF_File.bSocketSensorCheckFloating（FormShow 的 ReadFile 剛讀過檔 → 就是檔案值）。
    // 後端回應沒有這個欄位 → 唯讀讀同一個鍵（ReadFile :2620 [Configuration] bSocketSensorCheckFloating，預設 false）。
    if (floating !== null || typeof read0 !== 'function') return;
    floating = false;
    read0.call(R, 'handlerCondition').then(function (doc) {
      var s = doc && doc.sections && doc.sections.Configuration, v = s && s.bSocketSensorCheckFloating;
      if (v && typeof v === 'object') v = v.raw !== undefined && v.raw !== null ? v.raw : v.value;
      floating = (v !== undefined && v !== null) && /^\s*(1|true)\s*$/i.test(String(v));
    }, function () { floating = false; });
  }

  /* ---- (3) sitemap.js 共存 ----------------------------------------------- */
  function markSmReady() {
    if (smReady) return;
    smReady = true;
    smWait.splice(0).forEach(function (f) { f(); });
  }
  function waitSitemap() {
    if (smReady || !SM()) return Promise.resolve();
    return new Promise(function (res) {
      smWait.push(res);
      setTimeout(function () {
        if (!smReady) {
          console.warn('[Setup/C] ht9045_setup_sitemap.js 15 秒內沒有完成開頁（沒有讀 handlerCondition），引擎直接套後端的值');
          markSmReady();
        }
      }, 15000);
    });
  }
  // sitemap.js 開頁時的 HT9045Recipe.read('handlerCondition') → 後端 golden FormShow 之後的值
  function serverDoc(d) {
    var px = d.proxies || {}, cfg = {}, sm = SM();
    var pos = px.ScrollBar1 && px.ScrollBar1.position;
    if (sm && sm.modes && sm.modes[pos]) cfg['Test Mode'] = sm.modes[pos].name;   // golden TestSiteFileName[0][Position]
    for (var r = 0; r < ROWCH.length; r++)
      for (var c = 0; c < COLCH.length; c++) {
        var p = px['cb' + ROWCH[r] + COLCH[c]];
        if (p && p.itemIndex !== undefined) cfg['Site ' + ROWCH[r] + COLCH[c]] = String(p.itemIndex);
      }
    return { sections: { Configuration: cfg }, source: 'C 路 editlist.get ' + STRUCT };
  }
  if (typeof read0 === 'function') {
    R.read = function (doc) {
      if (doc !== 'handlerCondition') return read0.apply(this, arguments);
      // 這一頁走 C 路：HandlerCondition.Data 的畫面值只認後端（同一個檔只能有一套真相）
      var p = LAST ? Promise.resolve(LAST) : new Promise(function (res) { dataWait.push(res); });
      return p.then(function (d) {
        setTimeout(markSmReady, 0);      // sitemap.js 的後續（ScrollBar1Change＋填 Site Map）是微任務，跑完才放行引擎
        return serverDoc(d);
      });
    };
  }

  function padSelects(d, skip) {
    var px = d.proxies || {}, pads = [];
    var olds = document.querySelectorAll('option[data-src="c-pad"]');
    for (var i = 0; i < olds.length; i++) olds[i].parentNode.removeChild(olds[i]);
    Object.keys(px).forEach(function (id) {
      var el = $(id), v = px[id];
      if (skip && skip[id]) return;          // 選項已照後端 golden CompChange 重建（siteItems）
      if (!el || el.tagName !== 'SELECT' || v.itemIndex === undefined || v.itemIndex < el.options.length) return;
      var site = /^cb[A-D][a-h]$/.test(id);
      for (var k = el.options.length; k <= v.itemIndex; k++) {
        var o = document.createElement('option');
        o.textContent = site ? (k === 0 ? '- - -' : 'CH ' + k) : (k === v.itemIndex && v.text ? v.text : String(k));
        o.setAttribute('data-src', 'c-pad');
        el.appendChild(o);
      }
      if (v.visible !== false) pads.push(id + '→' + v.itemIndex);   // 看不見的（後端沒跑到 CompChange 的格子）只補不報
    });
    if (pads.length) notes.push('ⓘ 下拉選項比後端 ItemIndex 少，已補選項讓值對得上：' + pads.join(', '));
  }

  function syncSitemap(d) {
    var sm = SM(), px = d.proxies || {};
    serverPos = px.ScrollBar1 && px.ScrollBar1.position !== undefined ? px.ScrollBar1.position : null;
    if (!sm || serverPos === null) return;
    if (sm.position() !== serverPos) {
      sm.setPosition(serverPos);         // 客戶端 ScrollBar1Change：依後端的模式重建格子（值等一下由引擎套）
      if (sm.position() !== serverPos) {
        notes.push('⚠ 後端的 Test Mode 是「' + (sm.modes[serverPos] || {}).name + '」，網頁捲軸的客戶端夾限只到「' +
                   (sm.modes[sm.position()] || {}).name + '」；存檔仍送後端的模式（沒動捲軸就不改）');
      }
    }
  }

  /* ---- (4) 引擎套完之後 -------------------------------------------------- */
  function afterApply(d) {
    var px = d.proxies || {}, bad = [];
    Object.keys(px).forEach(function (id) {
      var el = $(id), v = px[id];
      if (!el) return;
      if (/^rgSensor\d+$/.test(id)) {
        el.style.display = v.visible === false ? 'none' : 'block';
        el.style.visibility = '';
      } else if (v.visible === true && el.style.display === 'none') {
        el.style.display = '';
      }
      if (el.tagName === 'SELECT' && v.itemIndex !== undefined && v.itemIndex >= -1 &&
          el.selectedIndex !== v.itemIndex && !el.querySelector('option[data-src="cpp-text"]')) bad.push(id);
    });
    var sm = SM();
    posAtLoad = sm ? sm.position() : null;
    watchScroll();
    if (bad.length) notes.push('❌ 這些下拉的畫面值和後端 ItemIndex 對不上（存檔會送畫面值）：' + bad.join(', '));
    var cb = $('CoSocketCombo');
    if (cb && !cb.__setupC) { cb.__setupC = true; cb.addEventListener('change', coSocketComboChange); }
    if (notes.length) say2(notes.join('\n'));
    notes = [];
  }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[Setup/C] ' + msg);
  }

  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    readFloating();
    return get0.apply(this, arguments).then(function (d) {
      LAST = d;
      lastSort = null;                   // 每次開頁（含存檔後重讀）重新算
      dataWait.splice(0).forEach(function (f) { f(d); });
      return waitSitemap().then(function () {
        buildSensors(d);                 // 引擎套值之前：元件要先在
        syncSitemap(d);
        var skip = siteItems(d);         // syncSitemap 可能讓 sitemap.js 重建格子 → 之後才蓋回後端的選項
        padSelects(d, skip);
        hookSortButtons();
        setTimeout(function () { afterApply(d); }, 0);   // 引擎在這個 promise 的 then 裡同步套完
        return d;
      });
    });
  };

  // sitemap.js 的捲軸只改它自己的狀態 → 操作員動過才送它的位置；沒動就送後端給的位置。
  // Steven 團隊 20260925：「動過」改看 sitemap.js 每次 setPosition 都會寫的 ScrollBar1 aria-valuenow（開頁套完值之後有變過
  // 就算）。原本比「現在位置 ≠ 開頁位置」：客戶端夾限讓開頁位置≠後端位置時（例 後端 16-Site、網頁夾到 1x2），
  // 操作員選回開頁那一格（1x2）會被當成沒動、送後端的 16-Site。
  var scrollMoved = false, scrollObs = null;
  function watchScroll() {
    scrollMoved = false;                     // 開頁／存檔後重讀：之前（含 syncSitemap 的夾限）不算
    var el = $('ScrollBar1');
    if (!el || scrollObs || !window.MutationObserver) return;
    scrollObs = new MutationObserver(function (recs) {
      recs.forEach(function (r) { if (r.oldValue !== el.getAttribute('aria-valuenow')) scrollMoved = true; });
    });
    scrollObs.observe(el, { attributes: true, attributeFilter: ['aria-valuenow'], attributeOldValue: true });
  }
  function scrollPos() {
    var sm = SM();
    if (!sm || serverPos === null) return serverPos;
    if (scrollObs) return scrollMoved ? sm.position() : serverPos;
    return (posAtLoad !== null && sm.position() !== posAtLoad) ? sm.position() : serverPos;
  }
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (AUTH_BUSY) return Promise.reject(new Error('busy: 重新登入（Q45）進行中，這一次存檔沒有送出'));   // AI(W906-Q45-B5) 20260930：(7) 防連點第二道
    if (widgets && widgets.ScrollBar1 && serverPos !== null) widgets.ScrollBar1 = { position: scrollPos() };
    // golden btnLUpToRDownNClick：最後按的排序鈕（後端照 golden 重播 → IniConfig.iSiteMapDirection）
    if (widgets && lastSort) widgets[lastSort] = { click: true };
    // AI(W906-Q45-B5) 20260930：golden sbUpdateClick :3573-3576 要重新登入時先問（見 (7)），答案放進 extra.reauth（跟 widgets 並列）
    var self = this, p = authNeeded(widgets);
    if (!p) return save1(self, st, widgets, answers, extra, null);
    AUTH_BUSY = true;
    authLock([STRUCT_SAVE_BTN].concat(AUTH_CTLS));
    return authAsk(p).then(function (ra) {
      return save1(self, st, widgets, answers, extra, ra);
    }).then(function (a) { AUTH_BUSY = false; authUnlock(); return a; },
            function (e) { AUTH_BUSY = false; authUnlock(); authBanner(null); throw e; });
  };
  function save1(self, st, widgets, answers, extra, ra) {
    var ex = extra;
    if (ra) { ex = {}; if (extra) Object.keys(extra).forEach(function (k) { ex[k] = extra[k]; }); ex.reauth = ra; }
    function wipe() { if (ra) { ra.password = ''; ra.userId = ''; ra = null; } if (ex && ex !== extra) ex.reauth = null; }   // 送出（recipe_client 在 takeover 之後才 stringify）回來才清
    return save0.call(self, st, widgets, answers, ex).then(function (a) {
      wipe();
      if (a && a.events && a.events.length) notes.push('ⓘ 存檔前照 golden 重播的事件：' + a.events.join(', '));
      authReport(a);
      return a;
    }, function (e) { wipe(); throw e; });
  }

  /* ---- (6) AI(W906-EVB10A) 20260929 [W906] St01：SU-9 Exit 鈕 sbtExit ＝ golden sbtExitClick（cSetUp.cpp:3476-3490）---------------------
   *   Steven 20260928「任何畫面的事件, 都是我們做」、20260929「照 BCB 的邏輯」。派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 SU-9。
   *   頁面內建的 .exitbtn 會直接叫外框關視窗（C++ 什麼都沒跑）→ 捕獲階段攔下，改送 WS form.event {"form":"TfSetup","control":"sbtExit","event":"click"}：
   *     golden：AUTO_SENSOR_INSTALL && bSaveNeedHome（改了 shuttle pitch 還沒重新回原點）⇒ 跳 "Auto Shuttle Sensor Need Reset!!"、視窗**不關**；
   *     否則 bNeedEnterPassword=true、Close() → FormClose（ReadFile、Auto Site Map 關掉時清 HotPlate site map）、GetCZSiteMap(false)。
   *   伺服器回 ack.closed＝true 才關視窗（C++ FileRW/TestIF_File_SetUp.cpp 檔尾、產生檔 SU_sbtExitClick）；false＝golden return，顯示訊息、留在頁面。
   *   伺服器沒有這個事件（舊版、還沒開頁讀取）或回「reload page」（伺服器端表單沒開著）⇒ 照舊直接關；關窗那一下伺服器由頁面表的關窗邊緣
   *   照 golden 跑 FormClose（✕ 也走那一條，golden 的 ✕ 只跑 FormClose）。
   *   防連點：送出到回覆之間再按不送（WebCmdGuard 另外擋同一個指令）；busy: 等 450 ms 重送（最多 3 次）；not-operator 續權杖再送一次。 */
  var exitBusy = false;
  function exitSay(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info('[SetUp/EVB10A] ' + msg);
  }
  function exitCloseWin() { if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); }
  function exitUnwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function exitSend(v, tries) {
    var extra = { tag: STRUCT, value: JSON.stringify(v) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) { return exitUnwrap(m) || {}; }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return exitSend(v, tries + 1); });
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) return R.keepAlive().then(function () { return exitSend(v, tries + 1); });
      throw new Error(msg);
    });
  }
  function exitClick() {
    if (exitBusy) return;
    var ev = LAST && LAST.events && LAST.events.sbtExit;
    if (!ev || ev.event !== 'click' || !R.rawCmd) { exitCloseWin(); return; }
    exitBusy = true;
    exitSend({ form: 'TfSetup', control: 'sbtExit', event: 'click' }, 0).then(function (a) {
      exitBusy = false;
      var ms = (a.messages || []).map(function (x) { return x.zh || x.en; }).filter(Boolean);
      if (a.closed) { if (ms.length) exitSay(ms.join('\n')); exitCloseWin(); return; }
      exitSay((ms.length ? ms.join('\n') + '\n' : '') + '視窗不關：golden sbtExitClick 沒有關表單（cSetUp.cpp:3478-3482）', '#f88');
    }, function (e) {
      exitBusy = false;
      var msg = (e && e.message) || String(e);
      if (/reload page/.test(msg)) { exitCloseWin(); return; }   // 伺服器端這一頁沒開著：golden 表單沒開就沒有 sbtExitClick
      exitSay('⚠ Exit：伺服器端 sbtExitClick 沒有跑（' + msg + '）；視窗照樣關閉（關窗時伺服器照 golden 跑 FormClose）', '#ffcc66');
      setTimeout(exitCloseWin, 1200);
    });
  }
  // 捕獲階段掛在 window：比頁面內建 .exitbtn 的 click（掛在按鈕上、直接 postMessage closeMe）先跑，並擋住它（同 ht9045_countersel_c.js）
  window.addEventListener('click', function (ev) {
    var t = ev.target && ev.target.closest ? ev.target.closest('#sbtExit') : null;
    if (!t) return;
    ev.stopPropagation();
    ev.preventDefault();
    exitClick();
  }, true);

  /* ---- (7) AI(W906-Q45-B5) 20260930 [W906] St01：Q45 甲 #6／#7 —— 關掉「Enable Real Time CCD」／「Enable OCR Function」要重新登入 ---------------
   *   golden TfSetup::sbUpdateClick（V912 cSetUp.cpp:3573-3606）→ TfSetup::DoPassword（:4325-4362）。Steven Q45「按照你的建議執行」
   *   （子題 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A）、20260929「請按照bcb的邏輯處理」。設計：
   *   D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §3.2；派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B5。
   *   開頁回應的 extra.auth（C++ FileRW/TestIF_File_SetUp.cpp ExtraJson → WebLogin.cpp W906_ReauthOpenJson）：points rtcOff／ocrOff 的
   *   armed＝裝了 REAL_TIME_CCD、沒被 [RTC Lock by file] 鎖、gbRTC／gbOcr 可用；armedAtOpen＝開頁時勾著（golden bNeedPassword／bOCRNeedPassword）。
   *   按 Save 時 armed＋armedAtOpen、而現在沒勾 ⇒ 先跳跟主畫面一樣的登入小鍵盤（main.html 的 HTQwerty 兩段：有密碼本先帳號、再密碼；
   *   下拉選單模式只有密碼），答案放進 editlistSave 的第 4 個參數 extra.reauth（跟 widgets 並列）一起送。比對、等級、改回、事件紀錄全在 C++。
   *   RTC、OCR 一起關只問一次（golden 同）；錯了不自動重問，要重試再按一次 Save。
   *   按 Abort＝取消＝golden 空白帳密＝錯（Q45-4＝A）：照樣送 {cancelled:true} —— C++ 照 golden 把勾改回、登入者變 Operator、其他設定照存。
   *   SetUp 版沒有「第 37 項＝0 就不問」的捷徑（golden 同）：37 項設 0 也會問一次（一定過），而且那一次會改掉主畫面的登入者。
   *   密碼：不 console、不存 localStorage／sessionStorage、回覆回來就把這一份清掉；訊息不帶輸入內容。
   *   防連點第二道：小鍵盤開著、存檔在路上時鎖住 Save 鈕與兩個勾選框；這段時間再進 editlistSave 一律回 busy（WebCmdGuard 另外擋重送）。
   *   沒有 extra.auth（舊的 C++）⇒ 不問，C++ 照舊「視同密碼錯」（設計 3.1 第 2 條）。 */
  var AUTH_BUSY = false, AUTH_LOCKED = [], AUTH_BANNER = null;
  var STRUCT_SAVE_BTN = 'sbUpdate';                                            // ht9045_wire_setupsetup.js saveBtn
  var AUTH_CTLS = ['cbEnableRealTimeCCD', 'cbOcrFunction'];
  function authInfo() { return (LAST && LAST.extra && LAST.extra.auth) || null; }
  function authNeeded(widgets) {                                                // 要問的那一點；golden 只問一次
    var a = authInfo(), pts = (a && a.points) || [];
    for (var i = 0; i < pts.length; i++) {
      var p = pts[i], c = p && (p.controls || [])[0], w = c && widgets && widgets[c];
      if (p && p.kind === 'relogin' && p.armed && p.armedAtOpen && w && w.checked === false) return p;
    }
    return null;
  }
  function authLock(ids) {
    authUnlock();
    ids.forEach(function (id) {
      var el = $(id);
      if (!el) return;
      var x = (el.tagName === 'INPUT' || el.tagName === 'BUTTON') ? el : (el.querySelector('input') || null);
      if (!x) return;
      AUTH_LOCKED.push([x, x.disabled]);
      x.disabled = true;
    });
  }
  function authUnlock() { AUTH_LOCKED.forEach(function (q) { q[0].disabled = q[1]; }); AUTH_LOCKED = []; }
  function authBanner(text) {                                                   // 小鍵盤上方的說明（小鍵盤本身沒有標題列文字）
    if (!text) { if (AUTH_BANNER) { AUTH_BANNER.remove(); AUTH_BANNER = null; } return; }
    if (!AUTH_BANNER) {
      AUTH_BANNER = document.createElement('div');
      AUTH_BANNER.style.cssText = 'position:fixed;left:50%;top:6px;transform:translateX(-50%);z-index:100000;max-width:92vw;' +
        'background:#fff8d0;color:#000;border:2px solid #c90;border-radius:4px;padding:6px 10px;white-space:pre-wrap;' +
        'font:bold 13px "Microsoft JhengHei",sans-serif;box-shadow:2px 2px 8px rgba(0,0,0,.4);';
      document.body.appendChild(AUTH_BANNER);
    }
    AUTH_BANNER.textContent = text;
  }
  function authKeypad(flags) {                                                  // 一段 HTQwerty → Promise(字串)；Abort／✕ → null
    return new Promise(function (res, rej) {
      if (typeof HTQwerty === 'undefined') { rej(new Error('登入小鍵盤（qwerty.js）沒有載入：這一次存檔沒有送出')); return; }   // ST01-E 20260930 審查：沒有小鍵盤不能當成按了取消（取消＝golden 空白帳密＝錯，會登出成 Operator）⇒ 整次不送
      var done = false;
      HTQwerty.show(null, flags, { onCommit: function (v) { done = true; res(v); }, onAbort: function () { if (!done) { done = true; res(null); } } });
    });
  }
  function authAsk(p) {                                                         // → Promise(reauth 物件)
    var N = (typeof HTQwerty !== 'undefined') ? HTQwerty.N : { NO_SYMBOL: 4, PASSWORD: 8, NO_SPACE: 16 };
    var a = authInfo(), book = !a || a.mode !== 'select', user = null;
    var head = '關閉 ' + (p.id === 'ocrOff' ? 'OCR' : 'Real Time CCD') + ' 要重新登入（golden TfSetup::DoPassword，cSetUp.cpp:4325）：' +
               '等級要到權限表第 ' + p.levelItem + ' 項（目前設 ' + p.level + '）。\n按 Abort＝取消＝登出成 Operator（golden 同）：勾會改回、其他設定照存。';
    authBanner(head + (book ? '\n① 輸入帳號' : '\n輸入密碼'));
    var first = book ? authKeypad(N.NO_SYMBOL | N.NO_SPACE) : Promise.resolve('');   // golden fPassword：帳號（密碼本模式）
    return first.then(function (u) {
      if (u === null) return null;
      user = u;
      authBanner(head + (book ? '\n② 輸入密碼' : '\n輸入密碼'));
      return authKeypad(N.NO_SYMBOL | N.NO_SPACE | N.PASSWORD);                // golden N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD（main.cpp:13215）
    }).then(function (pw) {
      authBanner(null);
      var ra = (pw === null) ? { point: p.id, cancelled: true }
             : (book ? { point: p.id, userId: user, password: pw } : { point: p.id, password: pw });
      user = null; pw = null;
      return ra;
    });
  }
  function authNonStop(code) {                                                  // golden ShowErrorMessage("WAR1677") → 不停機告警（Steven 20260924，同 main.html）
    var ns = null;
    try { ns = window.HT9045NonStop || (window.parent !== window && window.parent.HT9045NonStop) || null; } catch (e) { ns = null; }
    if (!ns || code !== 'WAR1677') return;
    ns.raise({ code: 'WAR1677', title: 'UserName or PassWord Error', titleZh: '帳號或密碼錯誤',
               message: 'UserName or PassWord Error (SetUp re-login)', messageZh: '帳號或密碼錯誤（SetUp 重新登入）', unitName: 'System',
               description: 'golden: TfSetup::DoPassword → TfMain::cbUserSelectChange → ShowErrorMessage("WAR1677")\n網頁版改成不停機告警（Steven 20260924），機台未停機。',
               hint: '要關 RTC／OCR 請再按一次 Save 並輸入正確的帳號密碼。' });
  }
  function authReport(a) {                                                      // 回應的 reauth（C++ 不含密碼）→ 引擎的狀態列（session.messages）
    var r = a && a.reauth;
    if (!r) return;
    var s = a.session || (a.session = {}), ms = s.messages || (s.messages = []);
    var lg = r.login || {}, who = (lg.userCaption || lg.levelName || '?') + '（等級 ' + lg.level + '）';
    var zh = '';
    if (r.handled === false) zh = '重新登入：golden 這次要問，但存檔沒有帶帳號密碼 —— 照舊視同密碼錯，勾改回（請按重讀再存一次）';
    else if (!r.asked) zh = r.answered ? 'ⓘ 重新登入：這次 golden 不用問（' + r.reason + '），登入沒有變' : '';
    else if (r.passed) zh = '重新登入通過（' + r.reason + '）：照 golden 關掉並記事件；目前登入：' + who;
    else zh = '重新登入沒有通過（' + (r.alarm === 'WAR1677' ? '帳號或密碼錯誤 WAR1677；' : '') + r.reason + '）：' +
              ((r.reverted || []).join('、') || '勾選') + ' 照 golden 改回、其他設定照存；目前登入：' + who;
    if (zh) ms.push({ en: 'Q45 re-login (golden TfSetup::DoPassword): ' + (r.reason || ''), zh: zh });
    if (r.alarm) authNonStop(r.alarm);
  }

  // 探針／除錯用
  window.HT9045SetupC = {
    state: function () {
      var sm = SM();
      return { serverPos: serverPos, posAtLoad: posAtLoad, clientPos: sm ? sm.position() : null,
               smReady: smReady, floating: floating, sensors: LAST ? sensorIds(LAST).length : 0, sendPos: scrollPos(),
               lastSort: lastSort, scrollMoved: scrollMoved, authBusy: AUTH_BUSY,   // AI(W906-Q45-B5) 20260930
               siteMapDirection: LAST && LAST.extra ? LAST.extra.iSiteMapDirection : null };
    },
    coSocketComboChange: coSocketComboChange
  };
})();
