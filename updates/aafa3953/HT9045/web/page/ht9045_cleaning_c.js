/* ht9045_cleaning_c.js -- Setup.Cleaning.html（golden TfCleaning，AutoClean/uCleaning.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_Cleaning.cpp（WS editlist.get／editlist.save tag=TestIF_File_Cleaning）。
 * 取代 B 路那兩份接線檔 ht9045_wire_cleaning.js／ht9045_wire_setupcleaning.js（65 鍵直接讀寫 HandlerCondition.Data／
 * HotPlate.Data；handlerCondition 與 hotPlate 都已被 C 路擁有，存檔回 409）——同一個檔只能有一個寫者，
 * 現在全部由 golden 存檔鈕（sbCleanSaveClick → SaveAutoCleanData）寫。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Setup.Cleaning.html' → 'TestIF_File_Cleaning'，整合者登錄）照通用規則讀寫元件；
 * 本檔補通用規則做不到的：
 *
 *  (1) 兩顆 golden 按鈕（引擎只在有 B 路欄位時才攔存檔鈕，這頁沒有 B 路 → 自己綁）：
 *        sbCleanSave  golden sbCleanSaveClick（:1778）→ HT9045Page.save()（引擎 gbSave：確認框、送值、存完重讀）；
 *        sbCleanExit  golden sbCleanExitClick（:1886 Close()）→ FormClose（:2106，SetWorkParameter）→ DoStructUnitConvert
 *                     （main.cpp:29673）。golden Exit 不存 Auto Clean 參數、不問 → 直接送 editlist.save、不跳確認框，完成後關視窗。
 *      editlist.save 注入 widgets.W906_clButton（後端 BeforeApply 讀掉）。
 *  (2) golden DFM ReadOnly=True、但 OnClick 開小鍵盤的元件（edDevicePices／edAlarmCount／edCleaningCount／edIndexArmAutoCleanCnt）：
 *      VCL ReadOnly 只擋打字、不擋點擊，golden 就是靠小鍵盤改它們（edDevicePices 的範圍是 udDeviceCT->Min..Max）。
 *      引擎把 ReadOnly 畫成停用（審查 L-3）→ 這裡在「自己 Enabled＋Visible、父容器可改」時重新打開，值由後端小鍵盤規則夾限。
 *  (3) 分頁：golden rgCleanKitTypeClick（:2252-2259）依 Kit／Tray 換 pgCleanType 頁；開頁照後端（extra.activePage）；
 *      pcCleanYield 的 tsOffset 在 golden FormShow 一律 TabVisible=false（:1622/:1628）→ VCL 換到下一個看得見的頁。
 *  (4) 未移植（停用並註明，後端檔頭同一份清單）：btInclude（:2269）、btnResetCleanCount（:2116）、btnResetInterval（:2842）、
 *      btnStartAutoClean（:2901）、sbTrayAssign（:2311）、cbbSelectTray（:2322，Tray CSV）、udDeviceCT（頁面沒有 TUpDown）、
 *      tmyAutoClean 的 Clean Pad 配置預覽（:1932-1949）。客戶專屬條件：Steven 20260925 決定先跳過（golden 程式碼照轉）。
 *
 * golden 存檔流程沒有 YES/NO 確認框 → 不登錄引擎 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 視窗標題列 ✕ 關閉：golden 會跑 FormClose（SetWorkParameter），網頁端沒有這個事件 → 不跑（用 Exit）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_Cleaning';
  var PAGE = 'Setup.Cleaning.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__cleaningCWrapped) return;
  R.__cleaningCWrapped = true;

  var get0 = R.editlistGet, save0 = R.editlistSave;
  var META = null;                 // editlist.get 回應的 extra（activePage／udDeviceCT）
  var LAST = null;                 // editlist.get 回應（proxies）
  var BUTTON = '';                 // 這次存檔是哪一顆 golden 按鈕
  var RO_KB = { edDevicePices: 'grpCleanDevice', edAlarmCount: 'grpCleanDevice',
                edCleaningCount: 'grpCleanDevice', edIndexArmAutoCleanCnt: 'grpCleanDevice' };   // (2) 元件 → DFM 父容器
  var NOT_PORTED = {
    btInclude: 'golden btIncludeClick（:2269）：把 Loader 盤格式（TrayForm.Loader）帶進 Clean Tray 欄位——要伺服器當下的值，這一版未接',
    btnResetCleanCount: 'golden btnResetCleanCountClick（:2116）：清潔計數歸零＋SetAutoCleanICCount＋SECS AutoCleanClearCount——機台動作，這一版未接',
    btnResetInterval: 'golden btnResetIntervalClick（:2842）：ChangeACSmartInterval(0) 直接寫配方——這一版未接',
    btnStartAutoClean: 'golden btnStartAutoCleanClick（:2901）：啟動 Auto Clean 動作——不在設定頁接',
    sbTrayAssign: 'golden sbTrayAssignClick（:2311）：切主畫面 AutoCleanStringGrid——不在設定頁接',
    cbbSelectTray: 'golden cbbSelectTrayChange（:2322）：Tray CSV 資料庫（fConfiguration）未移植；只有 ASE 高雄／OSE／K3 顯示'
  };

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[Cleaning/C] ' + msg);
  }
  function usable(el) {                              // golden 使用者按得到：沒停用、沒藏起來
    if (!el || el.disabled) return false;
    for (var p = el; p && p.nodeType === 1; p = p.parentNode) {
      if (p.getAttribute('aria-disabled') === 'true') return false;
      var cs = window.getComputedStyle(p);
      if (cs.visibility === 'hidden' || cs.display === 'none') return false;
    }
    return true;
  }

  /* ---- (3) 分頁 ------------------------------------------------------------------------------------ */
  function tabsOf(pcId) {
    var pc = $(pcId);
    return pc ? Array.prototype.slice.call(pc.querySelectorAll(':scope > .pcTabs > .tab')) : [];
  }
  function visibleTab(t) { return t && t.style.display !== 'none'; }
  function activate(pcId, idx) {                     // golden 設 ActivePageIndex；那一頁藏起來 → VCL 換到下一個看得見的頁
    var tabs = tabsOf(pcId);
    if (!tabs.length) return;
    for (var k = 0; k < tabs.length; k++) {
      var t = tabs[(idx + k) % tabs.length];
      if (visibleTab(t)) { t.click(); return; }
    }
  }
  function bindKitType() {                           // golden rgCleanKitTypeClick（:2252-2259）的畫面那一半
    var rg = $('rgCleanKitType');
    if (!rg || rg.__clC) return;
    rg.__clC = true;
    rg.addEventListener('change', function () {
      var rs = rg.querySelectorAll('input[type="radio"]');
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) { activate('pgCleanType', i === 0 ? 0 : 1); return; }
    });
  }

  /* ---- (2) ReadOnly＋小鍵盤 -------------------------------------------------------------------------- */
  function reopenReadOnlyKb(d) {
    var px = (d && d.proxies) || {};
    Object.keys(RO_KB).forEach(function (id) {
      var el = $(id), me = px[id], par = px[RO_KB[id]];
      if (!el || !me) return;
      var ok = me.enabled && me.visible && (!par || par.editable);
      el.disabled = !ok;
      if (ok) el.removeAttribute('data-gb-dis');
      el.title = id + '：golden DFM ReadOnly，只能點開小鍵盤改（後端照 golden 小鍵盤夾限）' +
                 (id === 'edDevicePices' && META && META.udDeviceCT ? '；範圍 ' + META.udDeviceCT.min + '~' + META.udDeviceCT.max : '');
    });
  }

  /* ---- (4) 未移植 ---------------------------------------------------------------------------------- */
  function markNotPorted() {
    Object.keys(NOT_PORTED).forEach(function (id) {
      var el = $(id);
      if (!el) return;
      el.disabled = true;
      el.style.opacity = '.45';
      el.title = id + '：' + NOT_PORTED[id];
    });
  }

  /* ---- (1) 按鈕 ------------------------------------------------------------------------------------ */
  function collect() {                               // 引擎 gbValue 的同一套規則（C 路：text／checked／itemIndex）
    var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null;
    if (!g || g.struct !== STRUCT || !g.page) return null;
    var out = {};
    Object.keys(g.kinds || {}).forEach(function (id) {
      var el = $(id), k = g.kinds[id];
      if (!el) return;
      if (k === 'checked') {
        var cb = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]');
        if (cb) out[id] = { checked: cb.checked };
      } else if (k === 'itemIndex') {
        if (el.tagName === 'SELECT') {
          var o = el.options[el.selectedIndex];
          out[id] = (o && o.getAttribute('data-src') === 'cpp-text') ? { itemIndex: -1, text: o.textContent }
                                                                     : { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
        } else {
          var rs = el.querySelectorAll('input[type="radio"]'), ix = -1;
          for (var i = 0; i < rs.length; i++) if (rs[i].checked) ix = i;
          out[id] = { itemIndex: ix };
        }
      } else if (k === 'text') {
        out[id] = { text: String(el.value) };
      }
    });
    return out;
  }
  function bindButtons() {
    var sv = $('sbCleanSave');
    if (sv && !sv.__clC) {
      sv.__clC = true;
      sv.addEventListener('click', function (ev) {
        ev.preventDefault(); ev.stopPropagation();
        if (!usable(sv)) return;
        if (!(window.HT9045Page && HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
          say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88');
          return;
        }
        BUTTON = 'sbCleanSave';
        HT9045Page.save();
      });
    }
    var ex = $('sbCleanExit');
    if (ex && !ex.__clC) {                           // 頁面內建的 .exitbtn 處理器會直接關視窗 → 換掉（cloneNode 不帶監聽器）
      var n = ex.cloneNode(true);
      ex.parentNode.replaceChild(n, ex);
      ex = n;
      ex.__clC = true;
      ex.title = 'sbCleanExit : golden sbCleanExitClick（:1886）→ FormClose（:2106，SetWorkParameter）→ DoStructUnitConvert，之後關視窗（不存 Auto Clean 參數）';
      ex.addEventListener('click', function (ev) {
        ev.preventDefault(); ev.stopPropagation();
        if (exitLocked) return;                       // AI(W906-CMDGUARD-UI) 20260926：已按過、視窗還沒關 → 忽略（見 exitLock）
        var close = function () {
          if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*');
          else exitUnlock();                          // AI(W906-CMDGUARD-UI) 20260926：單獨開頁（沒有外框）關不掉 → 解鎖
          var g = exitGen;                            // AI(W906-CMDGUARD-UI) 20260926：外框沒關、也沒送 HT_WIN 時的保險
          setTimeout(function () { if (g === exitGen) exitUnlock(); }, 3000);   //   （只解這一次的鎖，不誤解之後再開窗按的那一次）
        };
        var w = collect();
        if (!w) { close(); return; }                  // 還沒走 C 路／還沒開頁：golden 表單沒開就沒有 FormClose
        w.W906_clButton = { text: 'sbCleanExit' };
        exitLock(ex);                                 // AI(W906-CMDGUARD-UI) 20260926
        save0.call(R, STRUCT, w, {}, null).then(function (a) {
          var s = (a && a.session) || {};
          say2('Exit：golden FormClose（SetWorkParameter）完成' + ((s.todo || []).length ? '\n⚠ ' + s.todo.join('\n⚠ ') : ''), '#9f9');
          setTimeout(close, 600);                     // 成功：維持鎖住，等視窗關掉才解鎖
        }, function (e) {
          exitUnlock();                               // AI(W906-CMDGUARD-UI) 20260926：失敗 ack → 解鎖
          if (!(window.HT9045Busy && HT9045Busy.is(e)))   // AI(W906-CMDGUARD-UI) 20260926：busy:＝上一下還在跑或剛做完，不顯示成失敗
            say2('⚠ Exit：伺服器端 FormClose 沒有跑（' + e.message + '）；視窗照樣關閉', '#ffcc66');
          setTimeout(close, 1200);
        });
      });
    }   q41UdDeviceCT();   // AI(W906-Q41) 20260927 (St02-E): CL-6（見下一行）
  }   /* AI(W906-Q41) 20260927 (St02-E): Q41 CL-6 —— golden TfCleaning 的 udDeviceCT（TUpDown，Associate＝edDevicePices，DFM 912 AutoClean\uCleaning.dfm:1863-1874，Min 0、Wrap=False）：udDeviceCTChangingEx（906_0625_Steven uCleaning.cpp:2242；912 :2263）記 iPosTemp，udDeviceCTClick（906 :2057-2078；912 :2078-2099）：不是 Increment 的倍數 → 文字退回 iPosTemp；等於 Increment 放行；(int)(iPos/Increment) 是奇數 → 同方向再走一格（VCL 夾在 Min..Max）。Min／Max／Increment＝後端 golden SetDeviceMaxMin（:1403）開頁時算的 extra.udDeviceCT（FileRW/TestIF_File_Cleaning.cpp ExtraJson 已帶，不用改 C++）；可見／可改照 proxies.udDeviceCT（golden :1503 2x6 模式藏、:1554／:1571 等級 43）。TUpDown 帶 UDS_SETBUDDYINT：位置取 edDevicePices 當下的數字。頁面沒有這個元件 → 在同一個群組框照 DFM 位置（Left 201、Top 32、16x24）補兩顆鈕，id 用 q41udDeviceCT（不用 udDeviceCT：引擎會把替身值套到同 id 的元素上）。沒做：DrawAutoClean（:2098，清潔盤示意圖重畫）；改 X／Y 數量之後 golden SetDeviceMaxMin 重算範圍（CL-7，St01）——這裡用開頁時的範圍。 */ function q41UdPx() { var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null; return g && g.page && g.page.proxies ? g.page.proxies.udDeviceCT : null; } function q41UdSet(ed, v) { if (ed.value === String(v)) return; ed.value = String(v); ['input', 'change'].forEach(function (evn) { var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); } ed.dispatchEvent(e2); }); } function q41UdClick(dir) { var ud = META && META.udDeviceCT, ed = $('edDevicePices'), box = $('q41udDeviceCT'); if (!ud || !ed || ed.disabled || (box && box.getAttribute('aria-disabled') === 'true')) return; var inc = (ud.increment | 0) || 1, lo = ud.min | 0, hi = ud.max | 0, clamp = function (v) { if (hi >= lo) { if (v > hi) v = hi; if (v < lo) v = lo; } return v; }; var cur = parseInt(ed.value, 10); if (isNaN(cur)) cur = lo; var iPosTemp = cur, iPos = clamp(cur + dir * inc); if (iPos === cur) return; q41UdSet(ed, iPos); var iDirect = (iPos > iPosTemp) ? 1 : -1; if ((iPos % inc) !== 0) { q41UdSet(ed, iPosTemp); return; } else if (iPos === inc) { /* golden pass */ } else if ((Math.trunc(iPos / inc) % 2) !== 0) { q41UdSet(ed, clamp(iPos + inc * iDirect)); } } function q41UdDeviceCT() { var ed = $('edDevicePices'), host = ed && ed.parentElement, cli = host && host.parentElement; if (!cli) return; var box = $('q41udDeviceCT'); if (!box) { box = document.createElement('span'); box.id = 'q41udDeviceCT'; box.setAttribute('data-htitle', 'udDeviceCT : TUpDown（Q41 頁面補件）'); box.style.cssText = 'position:absolute;left:201px;top:32px;width:16px;height:24px;display:flex;flex-direction:column;'; [['\u25B2', 1], ['\u25BC', -1]].forEach(function (q) { var b = document.createElement('button'); b.type = 'button'; b.textContent = q[0]; b.style.cssText = 'flex:1 1 0;min-height:0;padding:0;font-size:7px;line-height:1;cursor:pointer;'; b.addEventListener('click', function (ev) { ev.preventDefault(); q41UdClick(q[1]); }); box.appendChild(b); }); cli.appendChild(box); } var px = q41UdPx(), on = !(px && (px.editable === false || px.enabled === false)); box.style.display = (px && px.visible === false) ? 'none' : ''; Array.prototype.forEach.call(box.querySelectorAll('button'), function (b) { b.disabled = !on; }); if (on) box.removeAttribute('aria-disabled'); else box.setAttribute('aria-disabled', 'true'); }
  /* AI(W906-CMDGUARD-UI) 20260926：sbCleanExit 按下後鎖住，直到視窗關掉（或收到失敗 ack）才解鎖。
   *   原本沒有任何防護：雙擊會送兩次 editlist.save → golden FormClose（SetWorkParameter）＋DoStructUnitConvert 跑兩次。
   *   非 lazy 的視窗關閉只是藏起來（background.html closeWin，iframe 不卸載）→ 解鎖要看外框送來的 HT_WIN
   *   （background.html postWinState：關閉 open:false、再開 open:true），不能等頁面卸載，否則再開窗時 Exit 永遠按不動。 */
  var exitLocked = false, exitBtn = null, exitGen = 0;
  function exitLock(btn) { exitLocked = true; exitGen++; exitBtn = btn; if (btn) btn.setAttribute('aria-busy', 'true'); }
  function exitUnlock() { exitLocked = false; if (exitBtn) exitBtn.removeAttribute('aria-busy'); }
  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (m && m.type === 'HT_WIN' && exitLocked) exitUnlock();
  });

  /* ---- editlist.get／save 包裝 ---------------------------------------------------------------------- */
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      META = (d && d.extra) || null;
      LAST = d;
      BUTTON = '';
      // 引擎在這個 then 之後才套元件（gbApply 設 visibility／disabled、頁籤）→ 下一輪再補
      setTimeout(function () {
        bindButtons();
        bindKitType();
        markNotPorted();
        reopenReadOnlyKb(LAST);
        var ap = META && META.activePage;
        if (ap) { activate('pgCleanType', ap.pgCleanType | 0); activate('pcCleanYield', ap.pcCleanYield | 0); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st, widgets) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    widgets = widgets || {};
    widgets.W906_clButton = { text: BUTTON || 'sbCleanSave' };   // 見 FileRW/TestIF_File_Cleaning.cpp BeforeApply
    BUTTON = '';
    arguments[1] = widgets;
    return save0.apply(this, arguments);
  };

  /* ---- 引擎註冊（取代 B 路兩份接線檔的 register；只留小鍵盤）--------------------------------------------------
   * 小鍵盤表照抄 ht9045_wire_setupcleaning.js（gen_wire.py 20260918 從 golden ShowQwertyKey 抽的；負值／max<=0 範圍已關夾限）。
   * 後端存檔時照 golden 小鍵盤再夾一次（FileRW/TestIF_File_Cleaning.cpp CL_QwertyKey），這裡只是即時回饋。 */
  var kb = {
    IndexArmSpeed:               ['INTEGER', 0, true, 100, 1],
    OutArmSpeed:                 ['INTEGER', 0, true, 100, 1],
    ShuttleSpeed:                ['INTEGER', 0, true, 100, 1],
    XCT1:                        ['INTEGER', 0, true, 100, 1],
    XCT2:                        ['INTEGER', 0, true, 100, 1],
    XPitch1:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    XPitch2:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    XST1:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    XST2:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    YCT1:                        ['INTEGER', 0, true, 100, 1],
    YCT2:                        ['INTEGER', 0, true, 100, 1],
    YPitch1:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    YPitch2:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    YST1:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    YST2:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    edACContactCleanHeight:      ['DOUBLE', 2, false, 0, 0],
    edACContactCount:            ['INTEGER', 0, true, 100, 1],
    edACContactShiftHeight:      ['DOUBLE', 2, true, 0.01, 30.0],
    edACInitalContactCount:      ['INTEGER', 0, false, 0, 0],
    edAdaptiveIntervalAdj:       ['INTEGER', 10, true, 1000, 1],
    edAdaptiveIntervalMax:       ['INTEGER', 10, true, 9999, 10],
    edAdaptiveIntervalMin:       ['INTEGER', 10, true, 9999, 1],
    edAlarmCount:                ['INTEGER', 0, true, 100000, 1],
    edAutoCleanAirForce_Kg:      ['DOUBLE', 2, false, 0, 0],
    edAutoCleanAirForce_N:       ['DOUBLE', 2, false, 0, 0],
    edCleanPadDeviation:         ['DOUBLE', 1, false, 0, 0],
    edContactTime:               ['DOUBLE', 1, false, 0, 0],
    edDevicePices:               ['INTEGER', 0, false, 0, 0],     // golden 範圍 udDeviceCT->Min..Max（伺服器端算，後端夾）
    edDropOffset1:               ['DOUBLE', 1, false, 0, 0],
    edIntervalContact:           ['INTEGER', 0, true, 100000, 1],
    edPinSingleGf:               ['DOUBLE', 2, false, 0, 0],
    edPinSingleN:                ['DOUBLE', 2, false, 0, 0],
    edPinsCount:                 ['INTEGER', 0, true, 20000, 1],
    edtACSmart_ACContactCount:   ['INTEGER', 0, true, 100, 1],
    edtAutoCleanDieForce:        ['DOUBLE', 2, false, 0, 0],
    edtConseFailureCountByHead_Normal: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountByHead_Retest: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountBySocket_Normal: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountBySocket_Retest: ['INTEGER', 0, true, 100000, 1],
    edtFailAlarmSiteYield:       ['INTEGER', 0, true, 100, 1],
    edtFailAlarmSiteYieldDifferent: ['INTEGER', 0, true, 100000, 1],
    edtInArmAirOn:               ['DOUBLE', 2, true, 10.0, 0.01],
    edtInArmVacuum:              ['DOUBLE', 2, true, 10.0, 0.01],
    edtInArmZSpeed:              ['INTEGER', 0, true, 100, 1],
    edtIndexAirOn:               ['DOUBLE', 2, true, 10.0, 0.01],
    edtIndexVacuum:              ['DOUBLE', 2, true, 10.0, 0.01],
    edtLowYieldCount:            ['INTEGER', 0, true, 100000, 1],
    edtLowYieldLimit:            ['INTEGER', 0, true, 100, 1]
  };
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'cleaning_c', fields: {}, optional: {}, kb: kb });
  }
})();
