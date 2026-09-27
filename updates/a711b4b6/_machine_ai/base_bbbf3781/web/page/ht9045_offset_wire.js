/* ht9045_offset_wire.js -- Setup.OffSet.html（golden TfOffSet，cOffSet.cpp V912）C 路的選取式編輯器。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋。
 * 同頁的 ht9045_wire_offset.js／ht9045_wire_setupoffset.js 是產生檔，只登錄小鍵盤表，本檔不改它們）
 *
 * 後端：FileRW/Offset_File.cpp（FileRW_Offset_Page／FileRW_Offset_Save）。
 * Steven 定案：「全部 offset 透過 JSON 整包傳輸，但點到某個按鈕才顯示指定的項目」。
 *
 *   開頁：引擎 gbLoad 送 WS editlist.get tag=Offset_File → 回應 offsets 整包：
 *         offsets["<sel>"]   = golden SpBotSelClick(Tag=sel) 之後的畫面（sb* 按鈕，stander）
 *         offsets["<tag>:2"] = golden IndexOffSetBT2Click(Tag=tag) 之後的畫面（btnTrayOfs*／IndexOffSetBT*，special）
 *         每組 {part, button, buttonVisible, clickable, refused, display{名:{visible,caption}}, widgets{id:{text,visible,editable}}}
 *     引擎照通用規則把 proxies（最後一次選取的畫面）套上去；本檔接著：
 *       - 部位按鈕：顯示＝buttonVisible 且非 refused；可按＝clickable
 *       - golden 建構子 iNowOffsetSel=-1／iSpecialOffSetSel=-1：開頁時沒有選取，編輯框清空停用，點了按鈕才顯示那一組
 *       - 選組：值／visible／可改一律取該組 widgets（proxies 的 editable 反映的是最後一次選取與 tsIndexOffset 的
 *         TabVisible=false，不能拿來判斷）；標題標籤取該組 display
 *   切組：目前這一組畫面上的值先記進工作副本（golden 切組會先 SaveFile 前一組；網頁改成記住、按存檔才一起送，見報告）
 *   存檔（spbSave）：送 value.widgets = {offsets:{<key>:{id:{text|checked}}}, common:{…}}：
 *       - 所有「改過」的組（該組可改欄位全部送，golden SaveSetupFile 也是整組寫畫面值）
 *       - 加上 golden spbSaveClick 會存的那一組：ActivePageIndex==0 → 目前 stander 選取，否則目前 special 選取
 *         （沒改也送：golden 按存檔一定會寫目前選取，並跑尾端 fMain->Pause／SetWorkParameter）
 *       - common（tsScale 分頁）只送改過的欄位
 *     golden spbSaveClick 沒有確認框 → answers 空物件；頁面仍確認一次（引擎的網頁誤觸保護慣例）
 *     ack.groups 逐組顯示 saved／ignored／unknown／refused／error；寫完一律重讀（規則 3），存檔沒成功的組保留使用者的修改
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'Offset_File';
  var LETTERS = 'ABCDEFGHIJKLMNOP';
  var SPECIAL_IDS = ['edtTrayArmX', 'edtARTPlace', 'edlLoadZ', 'IndexArmOffSet1', 'IndexArmOffSet2', 'IndexArmOffSet3',
                     'IndexArmOffSet4', 'IndexArmOffSet5', 'IndexArmOffSet6', 'edShtFor2D'];
  var STANDER_BASE = ['edArmX', 'edArmY', 'edPickUp', 'edRelease', 'edPitchY', 'edPitchX4', 'edPitchX3', 'edPitchX2',
                      'edPitchX1', 'edPreciserOpen', 'edPreciserClose'];
  var PAIR_LABEL = {};                 // 吸嘴編輯框 → golden ShowOneByOneOffSet 一起設 Visible 的標籤
  var STANDER_IDS = STANDER_BASE.slice();
  for (var li = 0; li < LETTERS.length; li++) {
    var c = LETTERS.charAt(li);
    PAIR_LABEL['EditPick' + c] = 'LabPick' + c;
    PAIR_LABEL['EdtRels' + c] = 'LabRels' + c;
    STANDER_IDS.push('EditPick' + c, 'EdtRels' + c, 'EdtOffset' + c + 'X', 'EdtOffset' + c + 'Y');
  }

  var D = null;          // 最近一次 editlist.get 的回應
  var WORK = {};         // key -> {id: {text}|{checked}}：各組的工作副本（使用者的修改）
  var WCOMMON = {};      // common 的工作副本
  var cur = { stander: null, special: null };   // 目前顯示的組（key）
  var keepWork = null;   // 存檔沒成功的組：重讀後保留修改 {key: {id: v}}
  var pendingMsg = null; // 存檔結果：重讀完成後與重讀摘要一起顯示
  var pendingGets = 0, loads = 0, lastAck = null, bound = false;

  function $(id) { return document.getElementById(id); }
  function clone(o) { return JSON.parse(JSON.stringify(o)); }
  function say(msg, colour) { if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour); }
  function isSpecial(key) { return /:2$/.test(key); }
  function group(key) { return D && D.offsets ? D.offsets[key] : null; }
  function cbOf(el) { return el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); }

  function setVis(el, v) {
    if (!el) return;
    el.style.visibility = v ? '' : 'hidden';
    if (v && el.style.display === 'none') el.style.display = '';   // dfm Visible=false 的元件產生器寫成 display:none
  }
  function setEna(el, on) {
    if (!el) return;
    var list = el.tagName === 'INPUT' || el.tagName === 'BUTTON' ? [el] : [el].concat([].slice.call(el.querySelectorAll('input')));
    list.forEach(function (x) { if ('disabled' in x) { x.disabled = !on; x.removeAttribute('data-gb-dis'); } });
    if (on) el.removeAttribute('aria-disabled'); else el.setAttribute('aria-disabled', 'true');
  }
  function putVal(el, v) {
    if (!el || !v) return;
    if (v.checked !== undefined) { var cb = cbOf(el); if (cb) cb.checked = !!v.checked; }
    else if (v.text !== undefined && 'value' in el) el.value = v.text;
  }
  function readVal(el, like) {
    if (!el) return null;
    if (like && like.checked !== undefined) { var cb = cbOf(el); return cb ? { checked: cb.checked } : null; }
    return 'value' in el ? { text: String(el.value) } : null;
  }
  function same(a, b) {
    if (!a || !b) return a === b;
    return a.checked !== undefined ? a.checked === b.checked : a.text === b.text;
  }
  function setCaption(id, cap) {
    var el = $(id);
    if (!el || !cap) return;          // 後端替身沒有 dfm 的 Caption（回 ""）；golden 程式只會設非空的 Caption → 空字串不套
    var t = el.querySelector(':scope > .pnlCap');
    if (t) t.textContent = cap;
    else if (el.classList.contains('lb')) el.textContent = cap;
  }

  // ---------------------------------------------------------------------------
  //  回應 → 畫面
  // ---------------------------------------------------------------------------
  function onLoaded(d) {
    if (!d || d.struct !== STRUCT || !d.offsets) {
      say('❌ Offset_File 回應形狀不對（沒有 offsets），頁面無法顯示。', '#f88');
      return;
    }
    D = d;
    WORK = {};
    Object.keys(d.offsets).forEach(function (k) {
      var w = d.offsets[k].widgets || {}, o = {};
      Object.keys(w).forEach(function (id) { o[id] = w[id].checked !== undefined ? { checked: w[id].checked } : { text: w[id].text }; });
      WORK[k] = o;
    });
    if (keepWork) {                    // 存檔沒成功的組：使用者的修改留著（仍是「改過」）
      Object.keys(keepWork).forEach(function (k) {
        if (!WORK[k]) return;
        Object.keys(keepWork[k]).forEach(function (id) { if (WORK[k][id]) WORK[k][id] = keepWork[k][id]; });
      });
      keepWork = null;
    }
    WCOMMON = {};
    var cm = d.common || {};
    Object.keys(cm).forEach(function (id) {
      var el = $(id);
      var v = cm[id].checked !== undefined ? { checked: cm[id].checked } : { text: cm[id].text };
      WCOMMON[id] = v;
      putVal(el, v); setVis(el, cm[id].visible); setEna(el, !!cm[id].editable);
    });
    // 部位按鈕
    Object.keys(d.offsets).forEach(function (k) {
      var g = d.offsets[k], b = $(g.button);
      if (!b) return;
      setVis(b, g.buttonVisible && !g.refused);
      setEna(b, !!g.clickable && !g.refused);
    });
    // 目前選取（存檔後重讀：golden 表單不關，選取留著）；那一組現在不能按了就回到「沒有選取」
    ['stander', 'special'].forEach(function (m) {
      var k = cur[m], g = k ? group(k) : null;
      if (!g || !g.clickable || g.refused || !g.buttonVisible) cur[m] = null;
    });
    render('stander'); render('special');
    loads++;
    var n = Object.keys(d.offsets).length, nv = 0;
    Object.keys(d.offsets).forEach(function (k) { var g = d.offsets[k]; if (g.buttonVisible && g.clickable && !g.refused) nv++; });
    var lines = [];
    if (pendingMsg) { lines.push(pendingMsg.text, '— 重讀 —'); }
    lines.push('✔ 讀取完成（C 路，golden TfOffSet FormShow）：' + n + ' 組 offset（可選 ' + nv + ' 組），檔案夾 ' + (d.offsetDir || '?') +
               (d.temperatureHot ? '（Hot）' : '') + '\n   點部位按鈕顯示該組；切組會記住修改，按 Save 一起寫入。');
    say(lines.join('\n'), pendingMsg ? pendingMsg.colour : '#9f9');
    pendingMsg = null;
  }

  // 把某一組畫到畫面（stander 或 special 那一區）；key=null → 沒有選取：編輯框清空停用
  function render(mode) {
    var key = cur[mode], g = key ? group(key) : null;
    var ids = mode === 'special' ? SPECIAL_IDS : STANDER_IDS;
    var w = g ? g.widgets || {} : {}, disp = g ? g.display || {} : {};
    ids.forEach(function (id) {
      var el = $(id);
      if (!el) return;
      if (w[id]) {
        putVal(el, WORK[key][id]);
        setVis(el, w[id].visible);
        setEna(el, !!w[id].editable);
        return;
      }
      // 這一組存檔不讀的元件：值不屬於這一組 → 清空停用；吸嘴框的可見度跟著 golden 同時設的標籤
      if ('value' in el) el.value = '';
      setEna(el, false);
      if (!g) return;
      var lab = PAIR_LABEL[id];
      setVis(el, lab && disp[lab] ? disp[lab].visible : false);
    });
    Object.keys(disp).forEach(function (n) {
      setVis($(n), disp[n].visible);
      setCaption(n, disp[n].caption);
    });
    if (!g) setCaption(mode === 'special' ? 'pnlIndexOffset' : 'palOffsetParts', ' ');
  }

  // 畫面上目前這一組的值 → 工作副本
  function harvest(mode) {
    var key = cur[mode], g = key ? group(key) : null;
    if (!g) return;
    Object.keys(g.widgets || {}).forEach(function (id) {
      if (!g.widgets[id].editable) return;
      var v = readVal($(id), WORK[key][id]);
      if (v) WORK[key][id] = v;
    });
  }
  function harvestCommon() {
    var cm = (D && D.common) || {};
    Object.keys(cm).forEach(function (id) {
      if (!cm[id].editable) return;
      var v = readVal($(id), WCOMMON[id]);
      if (v) WCOMMON[id] = v;
    });
  }

  function select(key) {
    var g = group(key);
    if (!g || !g.clickable || g.refused || !g.buttonVisible) return false;
    var mode = isSpecial(key) ? 'special' : 'stander';
    harvest(mode);
    cur[mode] = key;
    render(mode);
    return true;
  }

  // 一組裡改過的可改欄位
  function changedIn(key) {
    var g = group(key), out = [];
    if (!g) return out;
    Object.keys(g.widgets || {}).forEach(function (id) {
      var w = g.widgets[id];
      if (!w.editable) return;
      var o = w.checked !== undefined ? { checked: w.checked } : { text: w.text };
      if (!same(o, WORK[key][id])) out.push(id);
    });
    return out;
  }
  function editableSet(key) {
    var g = group(key), o = {};
    Object.keys(g.widgets || {}).forEach(function (id) { if (g.widgets[id].editable) o[id] = WORK[key][id]; });
    return o;
  }
  // golden PageControl1->ActivePageIndex==0（tsInOutArmOffset）→ 存 stander 選取；其他分頁 → 存 special 選取
  function activeIsStander() {
    // theme.js release 模式把 title 搬到 data-htitle → 兩個都看
    var t = document.querySelector('#PageControl1 > .pcTabs > .tab.act');
    return !t || /^tsInOutArmOffset :/.test(t.getAttribute('title') || t.getAttribute('data-htitle') || '');
  }
  function showPage(ts) {
    var t = document.querySelector('#PageControl1 > .pcTabs > .tab[title^="' + ts + ' :"],' +
                                   '#PageControl1 > .pcTabs > .tab[data-htitle^="' + ts + ' :"]');
    if (t) t.click();                  // 頁籤可能 TabVisible=false（display:none），click() 仍會切換內容
  }

  // 存檔要送的東西（探針也用它看「頁面真正會送出的」）
  function collect() {
    harvest('stander'); harvest('special'); harvestCommon();
    var offsets = {}, why = {};
    Object.keys(WORK).forEach(function (k) {
      var ch = changedIn(k);
      if (ch.length) { offsets[k] = editableSet(k); why[k] = 'changed: ' + ch.join(', '); }
    });
    var act = activeIsStander() ? cur.stander : cur.special;
    if (act && !offsets[act] && group(act) && group(act).clickable) {
      offsets[act] = editableSet(act); why[act] = 'golden spbSaveClick 目前選取';
    }
    var common = {}, cm = (D && D.common) || {};
    Object.keys(cm).forEach(function (id) {
      if (!cm[id].editable) return;
      var o = cm[id].checked !== undefined ? { checked: cm[id].checked } : { text: cm[id].text };
      if (!same(o, WCOMMON[id])) common[id] = WCOMMON[id];
    });
    var w = { offsets: offsets };
    if (Object.keys(common).length) w.common = common;
    return { widgets: w, why: why };
  }

  function partName(k) { var g = group(k); return g ? g.part + '（' + g.button + '）' : k; }

  function save() {
    lastAck = null;
    if (!D) { say('❌ 還沒讀取過，不能存檔（請先重讀）。', '#f88'); return Promise.resolve(null); }
    var c = collect(), keys = Object.keys(c.widgets.offsets);
    if (!keys.length && !c.widgets.common) {
      say('沒有選取任何部位、也沒有修改，未寫入。（golden spbSaveClick 在沒有選取時 SaveFile(-1) 也不寫）', '#ffcc66');
      return Promise.resolve(null);
    }
    var list = keys.map(function (k) { return '  ' + partName(k) + (c.why[k] ? ' — ' + c.why[k] : ''); });
    if (c.widgets.common) list.push('  In/Out Scale：' + Object.keys(c.widgets.common).join(', '));
    if (!window.confirm('確定要寫入 Offset？\n' + list.join('\n') + '\n\n走 golden TfOffSet 的存檔流程（spbSaveClick）。')) {
      say('已取消，未寫入。', '#ffcc66');
      return Promise.resolve(null);
    }
    say('寫入中（C 路：golden TfOffSet spbSaveClick）...');
    return HT9045Recipe.editlistSave(STRUCT, c.widgets, {}, null).then(function (a) {
      lastAck = a;
      var lines = [a.saved ? '✔ 已寫入（golden TfOffSet）' : '⚠ 沒有任何一組寫入'], bad = false, keep = {};
      var gs = a.groups || {};
      keys.forEach(function (k) {
        var r = gs[k];
        if (!r) { lines.push('❌ ' + partName(k) + '：回應裡沒有這一組'); bad = true; keep[k] = c.widgets.offsets[k]; return; }
        if (r.saved) lines.push('✔ ' + partName(k) + '：已寫入（套用 ' + (r.applied || []).length + ' 欄）');
        else {
          bad = true; keep[k] = c.widgets.offsets[k];
          lines.push('❌ ' + partName(k) + '：沒有寫入' + (r.refused ? '（拒絕：' + r.refused + '）' : '') + (r.error ? '（錯誤：' + r.error + '）' : ''));
        }
        if ((r.ignored || []).length) lines.push('   ⓘ 權限／畫面狀態不能改，沿用原值：' + r.ignored.join(', '));
        if ((r.unknown || []).length) { bad = true; lines.push('   ⚠ 伺服器不認得的欄位：' + r.unknown.join(', ')); }
        var s = r.session || {};
        (s.messages || []).forEach(function (m) { lines.push('   訊息：' + (m.zh || m.en)); });
        if ((s.todo || []).length) lines.push('   ⚠ golden 還有沒做到的步驟：' + s.todo.join('；'));
      });
      var t = a.tail || {};
      if (t.ran) {
        var ts = t.session || {};
        (ts.messages || []).forEach(function (m) { lines.push('訊息（存檔尾端）：' + (m.zh || m.en)); });
        if ((ts.todo || []).length) lines.push('⚠ 存檔尾端 golden 還有沒做到的步驟：' + ts.todo.join('；'));
        if ((t.ignored || []).length) lines.push('ⓘ In/Out Scale 不能改，沿用原值：' + t.ignored.join(', '));
      }
      if (a.commonError) { bad = true; lines.push('❌ In/Out Scale：' + a.commonError); }
      keepWork = Object.keys(keep).length ? keep : null;
      pendingMsg = { text: lines.join('\n'), colour: bad ? (a.saved ? '#ffcc66' : '#f88') : '#9f9' };
      return window.HT9045Page.load();                                   // 規則 3：寫完一定重讀
    }, function (e) {
      lastAck = { saved: false, error: e.message };
      say('❌ 寫入失敗：' + e.message + (/reload page|409/.test(e.message) ? '\n→ 請先重讀（沒開過頁，或權限／登入狀態變了）' : ''), '#f88');
      return null;
    });
  }

  // 引擎 gbLoad 呼叫 HT9045Recipe.editlistGet：回應在引擎套完 proxies 之後（下一輪）交給 onLoaded
  if (window.HT9045Recipe && HT9045Recipe.editlistGet) {
    var origGet = HT9045Recipe.editlistGet;
    HT9045Recipe.editlistGet = function (st) {
      if (st !== STRUCT) return origGet.apply(this, arguments);
      pendingGets++;
      return origGet.apply(this, arguments).then(function (d) {
        setTimeout(function () { pendingGets--; onLoaded(d); }, 0);
        return d;
      }, function (e) { pendingGets--; throw e; });
    };
  }

  function bind() {
    if (bound) return;
    bound = true;
    // 部位按鈕（golden SpBotSelClick／IndexOffSetBT2Click）：按鈕 id 取自回應（每組的 button）
    document.addEventListener('click', function (ev) {
      var t = ev.target && ev.target.closest ? ev.target.closest('button') : null;
      if (!t || !t.id || !D) return;
      var key = null;
      Object.keys(D.offsets).forEach(function (k) { if (D.offsets[k].button === t.id) key = k; });
      if (key === null) return;
      ev.preventDefault();
      if (t.disabled) return;
      select(key);
    });
    // spbSave：引擎只在接線檔有 B 路欄位時才攔存檔鈕，本頁沒有 → 自己接（捕獲階段，不讓別的處理器跑）
    document.addEventListener('click', function (ev) {
      var t = ev.target && ev.target.closest ? ev.target.closest('#spbSave') : null;
      if (!t) return;
      ev.stopPropagation(); ev.preventDefault();
      save();
    }, true);
    // golden btnToIndexOffsetClick／btnToArmOffsetClick／btnBackClick：切 PageControl1 分頁（tsIndexOffset 頁籤是藏起來的）
    var nav = { btnToIndexOffset: 'tsIndexOffset', btnToArmOffset: 'tsInOutArmOffset', btnBack: 'tsInOutArmOffset' };
    Object.keys(nav).forEach(function (id) {
      var b = $(id);
      if (b) b.addEventListener('click', function (ev) { ev.preventDefault(); showPage(nav[id]); });
    });
    var ol = $('btnOffsetList');
    if (ol) ol.addEventListener('click', function (ev) {
      ev.preventDefault();
      say('Arm Offset List（golden btnOffsetListClick → ShowOffSetList）網頁端尚未接：清單內容後端沒有提供。', '#ffcc66');
    });
  }

  // 探針／除錯用
  window.HT9045Offset = {
    state: function () {
      return { loads: loads, pendingGets: pendingGets, cur: clone(cur), hasData: !!D, activeStander: activeIsStander() };
    },
    work: function (k) { return WORK[k] ? clone(WORK[k]) : null; },
    collect: function () { return collect(); },
    select: select,
    save: save,
    showPage: showPage,
    lastAck: function () { return lastAck; }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind); else bind();
})();
