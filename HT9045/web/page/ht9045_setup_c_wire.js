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
    if (widgets && widgets.ScrollBar1 && serverPos !== null) widgets.ScrollBar1 = { position: scrollPos() };
    // golden btnLUpToRDownNClick：最後按的排序鈕（後端照 golden 重播 → IniConfig.iSiteMapDirection）
    if (widgets && lastSort) widgets[lastSort] = { click: true };
    return save0.call(this, st, widgets, answers, extra).then(function (a) {
      if (a && a.events && a.events.length) notes.push('ⓘ 存檔前照 golden 重播的事件：' + a.events.join(', '));
      return a;
    });
  };

  // 探針／除錯用
  window.HT9045SetupC = {
    state: function () {
      var sm = SM();
      return { serverPos: serverPos, posAtLoad: posAtLoad, clientPos: sm ? sm.position() : null,
               smReady: smReady, floating: floating, sensors: LAST ? sensorIds(LAST).length : 0, sendPos: scrollPos(),
               lastSort: lastSort, scrollMoved: scrollMoved,
               siteMapDirection: LAST && LAST.extra ? LAST.extra.iSiteMapDirection : null };
    },
    coSocketComboChange: coSocketComboChange
  };
})();
