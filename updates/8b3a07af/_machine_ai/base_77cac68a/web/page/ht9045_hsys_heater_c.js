/* ht9045_hsys_heater_c.js -- HW.HandlerSys.html「Heater」分頁（grpHeater）的逐通道溫控器廠牌下拉，C 路頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-P8) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * golden V912 HandlerSys.cpp（EN_HEATER_SHEET=1，MachineType.h:658）：
 *   建構子 :70-112 依 g_tHeaterInsInfo（MachineTypeUtility.cpp，71 通道、23 個 occupy）在 grpHeater 裡「動態」建 TLabel＋TComboBox，
 *   DFM 沒有這些元件 → 頁面產生器畫不出來，grpHeater 是空的。
 * 後端：FileRW/HSys.cpp（WS editlist.get／editlist.save tag=HSys）。
 *   editlist.get 的 extra.heater：通道清單（替身 id lb/cbHeaterInsOpt_<通道>、標題、golden 算的 Left／Top、可見與否）、5 個廠牌選項、版面常數。
 *   值／可見／可改走引擎的通用 proxies（本檔只負責「元件要先在」）。
 *   開頁＝golden LoaderSystemSet :262-278（讀 [TempCtrl] 71 鍵、缺鍵補寫 -9999）；存檔＝golden SaveSystemSet :779-794（寫 71 鍵＋HEATER_CTRL_TYPE）。
 *
 * 本檔做兩件事：
 *  (1) 引擎套值之前，照 extra.heater 在 grpHeater 裡建 <span class="lb">／<select class="ed">（包 HT9045Recipe.editlistGet，
 *      比照 ht9045_groundman_c.js／ht9045_setup_c_wire.js：這個 then 在引擎的 then 之前跑）。
 *  (2) golden rgHeaterTypeClick（:1519）的「畫面那一半」：操作員點 Heater Type 的某個廠牌 → 所有逐通道下拉設成同一個廠牌。
 *      檔案那一半（golden 按下即寫 71 鍵＋HEATER_CTRL_TYPE）：//AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927 Q14＝B
 *      —— 按 Heater Type 不寫檔；後端在存檔時重播這個事件只改記憶體（FileRW/HSys.cpp BeforeApply），71 鍵＋HEATER_CTRL_TYPE
 *      等整頁存檔（golden SaveSystemSet :782-793，答「是」之後）一起寫，寫的鍵與值跟 golden 按下即寫的相同。
 *      點了廠牌但沒按存檔、或存檔答「否」→ 不寫檔（golden 會寫）。
 *
 * ⚠ 底層：溫控流程（bthermo／rs232…）仍用單一廠牌 [TempCtrl] HEATER_CTRL_TYPE（底層照 906，RULINGS_20260926 第 26 條）。
 *   各通道設成不同廠牌時，存檔會把 HEATER_CTRL_TYPE 寫成 Heater Type 單選的值（golden 912 同），逐通道的差異溫控目前用不到（列給 Jimmy）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'HSys';

  function $(id) { return document.getElementById(id); }
  function host() {                                   // grpHeater 的內容區（頁面產生器畫的 .cli）
    var g = $('grpHeater');
    if (!g) return null;
    return g.querySelector('.cli') || g;
  }
  var BUILT = [];                                     // 本檔建的下拉 id（給 (2) 用）
  var NOTE = '逐通道溫控器廠牌（golden V912 THandlerSystem 建構子 HandlerSys.cpp:70-112 動態建立）。' +
             '存檔寫 system\\Gerneral.ini [TempCtrl] 各通道鍵；溫控流程目前只讀 HEATER_CTRL_TYPE（底層照 906）。';

  /* ---- (1) 引擎套值之前建元件 ---------------------------------------------------------------- */
  function build(d) {
    var hx = d && d.extra && d.extra.heater;
    var box = host();
    if (!hx || !box || !hx.channels) return;
    var sz = hx.size || { lbW: 100, lbH: 28, cbW: 170, cbH: 28 };
    var opts = hx.options || [];
    BUILT = [];
    hx.channels.forEach(function (c) {
      var lb = $(c.lb);
      if (!lb) {
        lb = document.createElement('span');
        lb.className = 'lb'; lb.id = c.lb; lb.setAttribute('data-src', 'hsys-heater');
        lb.style.cssText = 'position:absolute;white-space:nowrap;font-size:11px;color:#000;';
        box.appendChild(lb);
      }
      lb.style.left = c.lbLeft + 'px'; lb.style.top = (c.lbTop + 6) + 'px';
      lb.textContent = c.caption;
      lb.title = c.lb + ' : TLabel（' + c.saveName + '）';
      var cb = $(c.cb);
      if (!cb) {
        cb = document.createElement('select');
        cb.className = 'ed'; cb.id = c.cb; cb.setAttribute('data-src', 'hsys-heater');
        cb.style.cssText = 'position:absolute;';
        box.appendChild(cb);
      }
      cb.style.left = c.cbLeft + 'px'; cb.style.top = c.cbTop + 'px';
      cb.style.width = sz.cbW + 'px'; cb.style.height = sz.cbH + 'px';
      // 選項每次照後端重建（引擎補的 cpp-item／cpp-text 也一起清掉，引擎套值時會再補）
      while (cb.options.length) cb.remove(0);
      opts.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; cb.appendChild(o); });
      cb.title = c.cb + ' : TComboBox — [TempCtrl] ' + c.saveName + '\n' + NOTE;
      // 可見與否：引擎只切 visibility，這裡同步給標籤（golden SetCtrlItemVis 同時切 Label 與 ComboBox）
      var vis = c.visible ? '' : 'hidden';
      lb.style.visibility = vis; cb.style.visibility = vis;
      BUILT.push(c.cb);
    });
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

  /* ---- (2) golden rgHeaterTypeClick 的畫面那一半 ------------------------------------------------- */
  function bindHeaterType() {
    var rg = $('rgHeaterType');
    if (!rg || rg.__hsysHeater) return;
    rg.__hsysHeater = true;
    rg.title = (rg.title ? rg.title + '\n' : '') +
               'golden rgHeaterTypeClick（HandlerSys.cpp:1519）：點選廠牌 → Heater 分頁所有通道設成同一個廠牌；' +
               '按下不寫檔（Steven 20260927 Q14＝B，偏離 golden），[TempCtrl] 各通道鍵與 HEATER_CTRL_TYPE 等按「存檔」一起寫。';
    rg.addEventListener('change', function (ev) {
      var t = ev.target;
      if (!t || t.type !== 'radio' || !t.checked) return;
      var rs = rg.querySelectorAll('input[type="radio"]'), idx = -1;
      for (var i = 0; i < rs.length; i++) if (rs[i] === t) idx = i;
      if (idx < 0) return;
      BUILT.forEach(function (id) {
        var cb = $(id);
        if (cb && idx < cb.options.length) cb.selectedIndex = idx;   // golden :1535 pCb->ItemIndex=iOpt（看不見的也設）
      });
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bindHeaterType);
  else bindHeaterType();
})();
