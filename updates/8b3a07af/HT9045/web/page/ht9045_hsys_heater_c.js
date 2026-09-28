/* ht9045_hsys_heater_c.js -- HW.HandlerSys.html「Heater」分頁（grpHeater）的溫控器廠牌設定，C 路頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-P8) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * golden V912 HandlerSys.cpp（EN_HEATER_SHEET=1，MachineType.h:658）：
 *   建構子 :70-112 依 g_tHeaterInsInfo（MachineTypeUtility.cpp，71 通道、23 個 occupy）在 grpHeater 裡「動態」建 TLabel＋TComboBox，
 *   DFM 沒有這些元件 → 頁面產生器畫不出來，grpHeater 是空的。
 * 後端：FileRW/HSys.cpp（WS editlist.get／editlist.save tag=HSys）。值／可見／可改走引擎的通用 proxies（本檔只負責「元件要先在」
 *   ＋畫面上的連動）。
 *
 * //AI(W906-FRW-S166) 20260927 [W906] 偏離 golden（裁決過）：Q34 方案 D＋Q15（RULINGS_20260926 S166／S137；D-2＝A 依 RULINGS_20260927
 *   第 7 條第 35 題）。全文 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q34.」。
 *   Heater 分頁改成：
 *   ・最上面一列「溫控器廠牌：○ 全機相同 ○ 各溫控器不同」（替身 rgHeaterInsMode）。
 *   ・「Index 位置溫控器」／「其他位置溫控器」兩個下拉（cbHeaterInsIndexOpt／cbHeaterInsOtherOpt）；Index 位置＝Head1～4＋Index 32 區
 *     （D-2a，36 個，灰字列出）。USE_16_HEATER 是 EJ1N／DTME08 時 Index 下拉停用、顯示控制器名稱（那些 Index 區不走溫控 COM 埠，
 *     golden V912 bthermo.cpp:1331-1367），而且 Index 跟著其他位置（後端 W906_HeaterInsIndexLocked 的說明）。
 *   ・逐通道表（D-3a：golden 23 個有下拉的通道照 golden 可見條件＋依 USE_16_HEATER 走溫控 COM 埠的 Index 區）：名稱｜組別｜廠牌｜站號｜預設站號。
 *     「全機相同」時表唯讀，只顯示每個通道算出來的廠牌與預設站號；「各溫控器不同」時可改廠牌與站號（空白＝預設，灰字顯示 golden 算的預設值；
 *     TC401 只指定「第幾台」，通道仍是序號%4，D-7a）。
 *   ・D-9a 提示：「不同」模式、Index≠其他、或改過站號時顯示「目前溫控只看 HEATER_CTRL_TYPE（單一廠牌、預設站號），這些設定要等底層翻完才生效」。
 *   ・存檔前檢查在後端（D-7a 站號範圍、D-8a 同一個溫控 COM 埠同廠牌同站號 → 整頁不存，訊息列出是哪兩個通道）。
 *   開頁不寫檔（Q15）；存檔寫的鍵見 FileRW/HSys_Heater.h 檔尾。
 *
 * (2) golden rgHeaterTypeClick（:1519）的「畫面那一半」：操作員點「Index Items」分頁的 Heater Type 某個廠牌 →
 *     「全機相同，Index 與其他都設成這個廠牌」（方案 D ⑤）。檔案那一半：//AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927
 *     Q14＝B —— 按下不寫檔；後端存檔時重播這個事件只改記憶體（FileRW/HSys.cpp BeforeApply → W906_HeaterMixTypeClick），
 *     等整頁存檔（答「是」）才寫。點了但沒存、或存檔答「否」→ 不寫檔（golden 會寫）。
 *
 * ⚠ 底層：溫控流程（bthermo／rs232…）在 Jimmy 翻完之前仍只用單一廠牌 [TempCtrl] HEATER_CTRL_TYPE（RULINGS_20260926 第 26 條；
 *   Q34 ⑥ 1～3、5～8 歸 Jimmy），所以上面 D-9a 的提示要一直顯示到底層翻完（之後由 St01 拿掉，Q34 順序 ⑤）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'HSys';
  var MODE_SAME = 0, MODE_DIFF = 1;

  function $(id) { return document.getElementById(id); }
  function host() {                                   // grpHeater 的內容區（頁面產生器畫的 .cli）
    var g = $('grpHeater');
    if (!g) return null;
    return g.querySelector('.cli') || g;
  }
  function el(tag, attrs, text) {
    var e = document.createElement(tag);
    Object.keys(attrs || {}).forEach(function (k) {
      if (k === 'style') e.style.cssText = attrs[k]; else e.setAttribute(k, attrs[k]);
    });
    if (text !== undefined) e.textContent = text;
    return e;
  }

  var MIX = null;                                     // extra.heater.mix（後端 FileRW/HSys.cpp W906_HeaterMixExtraJson）
  var OPTS = [];                                      // 5 個廠牌（golden g_HeaterInsOptStr）
  var LAST_MODE = MODE_SAME;                          // 上一次畫面上的模式（切換時暫存／還原逐通道值）
  var GREY = 'color:#777;';

  function selectWithOptions(id) {
    var s = el('select', { 'class': 'ed', id: id, 'data-src': 'hsys-heater' });
    OPTS.forEach(function (t) { s.appendChild(el('option', {}, t)); });
    return s;
  }

  /* ---- (1) 引擎套值之前建元件 ---------------------------------------------------------------- */
  function build(d) {
    var hx = d && d.extra && d.extra.heater;
    var box = host();
    if (!hx || !box || !hx.mix) return;
    MIX = hx.mix;
    OPTS = hx.options || [];
    LAST_MODE = MIX.mode === MODE_DIFF ? MODE_DIFF : MODE_SAME;

    var old = $('hsysHeaterMix');
    if (old && old.parentNode) old.parentNode.removeChild(old);
    // 舊版（20260926 起）照 golden 版面直接放在 .cli 裡的下拉／標籤一併清掉（id 會在下面重建）
    Array.prototype.slice.call(box.querySelectorAll('[data-src="hsys-heater"]')).forEach(function (x) {
      if (x.parentNode) x.parentNode.removeChild(x);
    });

    var root = el('div', { id: 'hsysHeaterMix', 'data-src': 'hsys-heater',
      style: 'position:absolute;left:8px;top:6px;right:8px;bottom:6px;overflow:auto;font-size:12px;color:#000;' });

    // 第一列：模式（替身 rgHeaterInsMode，TRadioGroup：0 全機相同／1 各溫控器不同）
    var r1 = el('div', { style: 'margin:2px 0 8px 0;' });
    r1.appendChild(el('b', {}, '溫控器廠牌：'));
    var rg = el('span', { id: MIX.modeProxy, 'data-src': 'hsys-heater',
      title: MIX.modeProxy + '：[TempCtrl] HeaterInsMode（0 全機相同／1 各溫控器不同）' });
    ['全機相同', '各溫控器不同'].forEach(function (t) {
      var lb = el('label', { style: 'margin-right:18px;cursor:pointer;' });
      lb.appendChild(el('input', { type: 'radio', name: 'rg_' + MIX.modeProxy }));
      lb.appendChild(document.createTextNode(' ' + t));
      rg.appendChild(lb);
    });
    r1.appendChild(rg);
    root.appendChild(r1);

    // 第二列：Index 位置／其他位置（替身 cbHeaterInsIndexOpt／cbHeaterInsOtherOpt）
    var r2 = el('div', { style: 'margin:0 0 4px 0;' });
    r2.appendChild(el('span', { style: 'display:inline-block;width:110px;' }, 'Index 位置溫控器'));
    var sI = selectWithOptions(MIX.indexProxy);
    sI.title = MIX.indexProxy + '：[TempCtrl] HeaterInsIndexOpt（全機相同時 Index 位置的廠牌）';
    r2.appendChild(sI);
    var lock = el('span', { id: 'hsysHeaterIndexLock', style: 'margin-left:8px;font-weight:bold;' }, MIX.indexLocked ? MIX.indexLockedLabel : '');
    r2.appendChild(lock);
    root.appendChild(r2);
    root.appendChild(el('div', { style: GREY + 'margin:0 0 6px 110px;' },
      'Index 位置（' + (MIX.indexGroup || []).length + ' 個通道）：' + (MIX.indexGroup || []).join('、') +
      (MIX.indexLocked ? '。這台的 Index 區不走溫控 COM 埠，Index 的廠牌跟著「其他位置」。' : '')));
    var r3 = el('div', { style: 'margin:0 0 6px 0;' });
    r3.appendChild(el('span', { style: 'display:inline-block;width:110px;' }, '其他位置溫控器'));
    var sO = selectWithOptions(MIX.otherProxy);
    sO.title = MIX.otherProxy + '：[TempCtrl] HeaterInsOtherOpt（全機相同時其他位置的廠牌）';
    r3.appendChild(sO);
    r3.appendChild(el('span', { id: 'hsysHeaterDiffNote', style: GREY + 'margin-left:8px;' }, ''));
    root.appendChild(r3);

    // D-9a 提示
    root.appendChild(el('div', { id: 'hsysHeaterHint',
      style: 'display:none;margin:4px 0 8px 0;padding:4px 8px;border:1px solid #d08a00;background:#fff4dd;color:#8a4b00;' },
      '⚠ ' + (MIX.hint || '')));

    // 逐通道表：列出的通道（D-3a）分欄放；沒列出的放在看不見的暫存區（元件仍在，引擎套值／存檔照常，後端也不收）
    var cols = el('div', { style: 'display:flex;flex-wrap:wrap;gap:18px;align-items:flex-start;' });
    var hold = el('div', { style: 'display:none;' });
    var listed = (MIX.rows || []).filter(function (r) { return r.listed; });
    var PER = 20, table = null;
    function newTable() {
      var t = el('table', { style: 'border-collapse:collapse;' });
      var hr = el('tr', {});
      ['通道', '組別', '廠牌', '站號', '預設站號'].forEach(function (h) {
        hr.appendChild(el('th', { style: 'text-align:left;padding:1px 6px;border-bottom:1px solid #999;font-weight:normal;' + GREY }, h));
      });
      t.appendChild(hr);
      cols.appendChild(t);
      return t;
    }
    function rowOf(r) {
      var tr = el('tr', { 'data-ti': r.typeIdx });
      var nm = el('td', { style: 'padding:1px 6px;white-space:nowrap;' });
      nm.appendChild(r.lb ? el('span', { 'class': 'lb', id: r.lb }, r.name) : el('span', {}, r.name));
      nm.title = r.saveName + (r.golden ? '（golden 有下拉）' : '（golden 沒有下拉：Index 區）');
      tr.appendChild(nm);
      tr.appendChild(el('td', { style: 'padding:1px 6px;' + GREY }, r.indexGroup ? 'Index' : '其他'));
      var tdc = el('td', { style: 'padding:1px 6px;' });
      var cb = selectWithOptions(r.cb);
      cb.title = r.cb + '：[TempCtrl] ' + r.saveName;
      tdc.appendChild(cb);
      tr.appendChild(tdc);
      var tda = el('td', { style: 'padding:1px 6px;' });
      var ad = el('input', { 'class': 'ed', id: r.addr, type: 'text', size: '4', 'data-src': 'hsys-heater',
        title: r.addr + '：[TempCtrl] ' + r.addrKey + '（空白＝預設站號；1～' + MIX.stationMax + '，E5DC 1～' + MIX.stationMaxE5DC + '）' });
      tda.appendChild(ad);
      tr.appendChild(tda);
      tr.appendChild(el('td', { 'class': 'hsys-def', style: 'padding:1px 6px;white-space:nowrap;' + GREY }, ''));
      return tr;
    }
    listed.forEach(function (r, i) {
      if (i % PER === 0) table = newTable();
      table.appendChild(rowOf(r));
    });
    var tHold = el('table', {});
    (MIX.rows || []).forEach(function (r) { if (!r.listed) tHold.appendChild(rowOf(r)); });
    hold.appendChild(tHold);
    if (!listed.length) cols.appendChild(el('div', { style: GREY }, '（這台沒有要列的通道）'));
    root.appendChild(cols);
    root.appendChild(hold);
    box.appendChild(root);

    root.addEventListener('change', refresh);
    root.addEventListener('input', function (ev) { if (ev.target && ev.target.tagName === 'INPUT' && ev.target.type === 'text') refresh(); });
    setTimeout(refresh, 0);                           // 引擎在自己的 then 裡套值（在這之後）→ 套完再整理畫面
  }

  function modeNow() {
    var rs = MIX ? document.querySelectorAll('#' + MIX.modeProxy + ' input[type="radio"]') : [];
    return (rs.length > 1 && rs[1].checked) ? MODE_DIFF : MODE_SAME;
  }
  function setRO(x, on) {                             // 自己的唯讀（不碰引擎依權限關掉的 data-gb-dis）
    if (!x) return;
    if (on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-hs-ro', '1'); } }
    else if (x.getAttribute('data-hs-ro') === '1') { x.disabled = false; x.removeAttribute('data-hs-ro'); }
  }

  /* ---- 畫面連動（模式、Index／其他、逐通道表、D-9a 提示）-------------------------------------- */
  function refresh() {
    if (!MIX || !$('hsysHeaterMix')) return;
    var mode = modeNow();
    var sI = $(MIX.indexProxy), sO = $(MIX.otherProxy);
    if (MIX.indexLocked && sI && sO) sI.selectedIndex = sO.selectedIndex;   // Index 跟著其他位置（後端同規則）
    var I = sI ? sI.selectedIndex : -1, O = sO ? sO.selectedIndex : -1;
    var B = MIX.brand || {}, anyStation = false;
    (MIX.rows || []).forEach(function (r) {
      var cb = $(r.cb), ad = $(r.addr);
      if (!cb || !ad) return;
      if (mode === MODE_SAME) {
        if (LAST_MODE === MODE_DIFF) cb.setAttribute('data-hs-diff', String(cb.selectedIndex));   // 切回「相同」前先記下逐通道值
        cb.selectedIndex = r.indexGroup ? I : O;                                              // 算出來的廠牌（唯讀）
        setRO(cb, true); setRO(ad, true);
        ad.style.display = 'none';
      } else {
        if (LAST_MODE === MODE_SAME && cb.getAttribute('data-hs-diff') !== null) {
          cb.selectedIndex = parseInt(cb.getAttribute('data-hs-diff'), 10);
        }
        cb.removeAttribute('data-hs-diff');
        setRO(cb, false); setRO(ad, false);
        ad.style.display = '';
      }
      var tc = cb.selectedIndex === B.tc401;
      var def = tc ? r.defTc401 : r.defOther;
      ad.placeholder = String(def);
      var tr = cb.closest ? cb.closest('tr') : null;
      var dc = tr ? tr.querySelector('.hsys-def') : null;
      if (dc) dc.textContent = tc ? ('第 ' + def + ' 台・通道 ' + r.tc401Ch) : ('預設 ' + def);
      if (mode === MODE_DIFF && r.listed && String(ad.value).trim() !== '' && String(ad.value).trim() !== String(def)) anyStation = true;
    });
    var note = $('hsysHeaterDiffNote');
    if (note) note.textContent = mode === MODE_DIFF ? '（各溫控器不同：這兩個下拉照樣存檔，切回全機相同時用）' : '';
    var hint = $('hsysHeaterHint');
    if (hint) hint.style.display = (mode === MODE_DIFF || (!MIX.indexLocked && I !== O) || anyStation) ? '' : 'none';
    LAST_MODE = mode;
  }

  var R = window.HT9045Recipe;
  if (R && R.editlistGet && !R.__hsysHeaterWrapped) {
    R.__hsysHeaterWrapped = true;
    var get0 = R.editlistGet;
    R.editlistGet = function (st) {
      if (st !== STRUCT) return get0.apply(this, arguments);
      return get0.apply(this, arguments).then(function (d) {
        try { build(d); } catch (e) { console.warn('[HSys/heater] build failed: ' + e.message); }
        return d;                                     // 引擎接著在它自己的 then 裡套值（元件已經在了）
      });
    };
  }

  /* ---- (2) golden rgHeaterTypeClick 的畫面那一半（方案 D ⑤：全機相同、Index＝其他＝點的廠牌）----------------- */
  function bindHeaterType() {
    var rg = $('rgHeaterType');
    if (!rg || rg.__hsysHeater) return;
    rg.__hsysHeater = true;
    rg.title = (rg.title ? rg.title + '\n' : '') +
               'golden rgHeaterTypeClick（HandlerSys.cpp:1519）：點選廠牌 → Heater 分頁設成「全機相同」、Index 與其他位置都是這個廠牌；' +
               '按下不寫檔（Steven 20260927 Q14＝B，偏離 golden），按「存檔」才寫 [TempCtrl]。';
    rg.addEventListener('change', function (ev) {
      var t = ev.target;
      if (!t || t.type !== 'radio' || !t.checked || !MIX) return;
      var rs = rg.querySelectorAll('input[type="radio"]'), idx = -1;
      for (var i = 0; i < rs.length; i++) if (rs[i] === t) idx = i;
      if (idx < 0) return;
      var ms = document.querySelectorAll('#' + MIX.modeProxy + ' input[type="radio"]');
      if (ms.length > 1) { ms[0].checked = true; ms[1].checked = false; }
      var sI = $(MIX.indexProxy), sO = $(MIX.otherProxy);
      if (sO && idx < sO.options.length) sO.selectedIndex = idx;
      if (sI && idx < sI.options.length) sI.selectedIndex = idx;
      (MIX.rows || []).forEach(function (r) { var cb = $(r.cb); if (cb) cb.removeAttribute('data-hs-diff'); });
      LAST_MODE = MODE_SAME;                          // 點了 Heater Type ＝ 全部同一廠牌：之後切到「各溫控器不同」從這個廠牌開始（不還原先前的逐通道值）
      refresh();
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bindHeaterType);
  else bindHeaterType();
})();
