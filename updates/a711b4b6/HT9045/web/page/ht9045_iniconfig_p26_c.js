/* ht9045_iniconfig_p26_c.js -- Config.Configuration.html「[P26] OCR check」群組（gbP26_OCRCheck）的 OCR Tray Lot 10 格，C 路頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-Q19) 20260927: 新檔（Steven 團隊 St01；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 * Steven 20260927 Q19（RULINGS_20260926 S140）：「P26 OCR Tray Lot 可以補」（同一題的「Soft 速度」：不需要，不做）。
 *
 * golden V912 cConfiguration.cpp（wei 20151117 OCR Lot check）：
 *   建構子 :206-224 在 gbP26_OCRCheck 裡「動態」建 10 個 TEdit（Name OCRTrayLot0..9）＋10 個 TLabel（labOCRTrayLot0..9，"1："～"10："），
 *   DFM 沒有這些元件 → 頁面產生器畫不出來。
 *   FormShow :5289-5294：群組 Visible＝CosFunction.bTrayOCR；各格 Text＝LastSet.TrayCount[i]。
 *   edOCRTrayLotChange :5522-5531：點格子 → 小鍵盤 N_INTEGER 0～20 → LastSet.TrayCount[Tag]=atoi(Text) → WriteLastDataFile()（lastdata.dat）。
 * 後端：FileRW/IniConfig.cpp（WS editlist.get／editlist.save tag=IniConfig）。
 *   editlist.get 的 extra.ocrTrayLot：10 格的替身 id、golden 算的位置、標題、小鍵盤參數。值／可見／可改走引擎的通用 proxies。
 *   存檔：格子的值和開頁時不同 → 後端在 golden FormClose 之前重播 edOCRTrayLotChange（寫 lastdata.dat，同 golden 按下即寫；
 *   所以存檔的確認框答「否」時這幾格也已經寫了）。
 *
 * 本檔做兩件事：
 *  (1) 引擎套值之前，照 extra.ocrTrayLot 在 gbP26_OCRCheck 裡建 <span class="lb">／<input class="ed">（包 HT9045Recipe.editlistGet，
 *      比照 ht9045_hsys_heater_c.js：這個 then 在引擎的 then 之前跑，引擎接著照 proxies 填值）。
 *  (2) 小鍵盤：引擎的 attachKeyboards 只綁開頁當下就在的輸入框，這 10 格是讀回來才建的 → 自己綁
 *      （golden :5526 ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 0, 20)；後端 IC_QwertyKey 也照 golden 夾 0～20）。
 *      停用（引擎 gbSetEnabled 設 disabled）時不開。沒有新增按鈕。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'IniConfig';
  var KB0 = { flag: 'INTEGER', dp: 0, checkRange: true, min: 0, max: 20 };   // golden :5526（後端沒給 kb 時的備用）
  var KB = KB0;

  function $(id) { return document.getElementById(id); }
  function host(pid) {                                // 群組的內容區（頁面產生器畫的 .cli）
    var g = $(pid || 'gbP26_OCRCheck');
    if (!g) return null;
    return g.querySelector('.cli') || g;
  }
  var NOTE = 'OCR Tray Lot（golden V912 TfConfiguration 建構子 cConfiguration.cpp:206-224 動態建立）。' +
             '值＝lastdata.dat 的 LastSet.TrayCount；改了按存檔時照 golden edOCRTrayLotChange 寫 lastdata.dat。';

  function fire(el) {                                 // 同引擎 attachKeyboards 的 onCommit：補發 input／change
    ['input', 'change'].forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); }
      catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function kbOpen(el) {
    if (typeof HTQwerty === 'undefined') return;
    if (el.disabled || el.getAttribute('aria-disabled') === 'true') return;
    var k = KB || KB0;
    HTQwerty.show(el, HTQwerty.N[k.flag] || 0,
                  { dp: k.dp, checkRange: !!k.checkRange, min: k.min, max: k.max, onCommit: function () { fire(el); } });
  }

  /* ---- (1) 引擎套值之前建元件 ---------------------------------------------------------------- */
  function build(d) {
    var ox = d && d.extra && d.extra.ocrTrayLot;
    if (!ox || !ox.cells) return;
    var box = host(ox.parent);
    if (!box) return;
    KB = ox.kb || KB0;
    ox.cells.forEach(function (c) {
      var lb = $(c.lb);
      if (!lb) {
        lb = document.createElement('span');
        lb.className = 'lb'; lb.id = c.lb; lb.setAttribute('data-src', 'iniconfig-p26');
        lb.style.cssText = 'position:absolute;white-space:nowrap;font-size:11px;color:#000;';
        box.appendChild(lb);
      }
      lb.style.left = c.lbLeft + 'px'; lb.style.top = c.lbTop + 'px';
      lb.textContent = c.caption;
      lb.title = c.lb + ' : TLabel';
      var ed = $(c.ed), sp;
      if (!ed) {
        sp = document.createElement('span');          // 同頁面產生器的 TEdit 版型：定位用的 span 包 input
        sp.setAttribute('data-src', 'iniconfig-p26');
        ed = document.createElement('input');
        ed.className = 'ed'; ed.id = c.ed; ed.setAttribute('data-src', 'iniconfig-p26');
        ed.style.cssText = 'width:100%;height:100%;box-sizing:border-box;cursor:pointer;';
        ed.setAttribute('readonly', 'readonly');      // 機台上沒有實體鍵盤：同引擎，只能用小鍵盤
        ed.addEventListener('mousedown', function (ev) { ev.preventDefault(); kbOpen(ev.currentTarget); });
        sp.appendChild(ed);
        box.appendChild(sp);
      }
      sp = ed.parentNode;
      sp.style.cssText = 'position:absolute;left:' + c.left + 'px;top:' + c.top + 'px;width:' + c.width + 'px;height:24px;';
      ed.title = c.ed + ' : TEdit — 小鍵盤 ' + (KB.flag || 'INTEGER') + ' (' + KB.min + '~' + KB.max + ')\n' + NOTE;
    });
  }

  var R = window.HT9045Recipe;
  if (R && R.editlistGet && !R.__iniconfigP26Wrapped) {
    R.__iniconfigP26Wrapped = true;
    var get0 = R.editlistGet;
    R.editlistGet = function (st) {
      if (st !== STRUCT) return get0.apply(this, arguments);
      return get0.apply(this, arguments).then(function (d) {
        try { build(d); } catch (e) { console.warn('[IniConfig/P26] build failed: ' + e.message); }
        return d;                                     // 引擎接著在它自己的 then 裡套值（元件已經在了）
      });
    };
  }
})();
