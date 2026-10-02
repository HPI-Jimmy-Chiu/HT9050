/* ht9045_contactforce_c.js -- Setup.ContactForce.html（golden TfContactForce，ContactForce.cpp）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * AI(W906-CF-WIRE) 20261002 新檔（手寫，不是 gen_wire.py 產物）。EastSun 1001「請檢查每個頁面元件…用枚舉 每個東西都檢查」：
 *   這一頁以前是純靜態畫面（只有 page-widgets.js 畫的 Load rate 樣板），開頁不讀、Save 不存。C++ 那一半早就做好了
 *   （FileRW/ContactForce.cpp kPage tag "ContactForce"，開窗閘 FileRW/_EditPage.cpp GContactForce；建頁備忘在該檔檔尾），
 *   這個檔照備忘補頁面那一半：
 *
 *  (1) 開頁＝WS editlist.get ContactForce（golden FormShow :685）。引擎（ht9045_wire_engine.js GOLDEN_BRIDGE）套 proxies 之前，
 *      先照 extra.panels 在四個 ScrollBox 裡建 golden 建構子 new 出來的動態面板（THTSLKClass :21-206、THTSLKIndClass :1439-1561、
 *      THTDieForceSLKClass :331-419、THTDieForceOneByOneSLKClass :209-328），元件 id＝golden Name（items[].ids），位置＝golden 建構子
 *      的 Top／Left／Width／Height；Align=alTop ⇒ 看得見的組依序往下疊、每組高 100（看不見的組 VCL 不佔位置）。
 *      之後引擎照 proxies 套值／Visible／Enabled（golden FormShow :822-888 的顯示規則在 C++ 跑）。
 *  (2) 面板 trackbar 的 OnChange（golden trckbrDiameter_Change :1402 等）：edtLoadRate*->Text=AnsiString(Position/100.0)（只改畫面，
 *      golden 不寫檔、不動機台）。tb30mm_10kg…（tsNewMethod）的 OnChange tb30mm_10kgChange :1375：ShowValue :1382 的 6 個數字照改；
 *      同一支的 ADAM_DirectWriteData（EP 直接輸出）是機台動作，C++ 沒接（ContactForce.cpp:379 notWired）—— 不送。
 *  (3) 存檔鈕 btSave → 引擎 HT9045Page.save()＝editlist.save ContactForce（golden btSaveClick :935，沒有 YES/NO ⇒ 不問）。
 *      btExit 照頁面內建 .exitbtn 關視窗（golden btExitClick :929 的 ADAM_WriteVoltage 與 FormClose :916 的 fContact 力量重算 C++ 沒接）。
 *  (4) 小鍵盤＝golden 各 OnClick／OnMouseDown 的 ShowQwertyKey：edMaxKpa* :1426、edMinMpa* :1575、edMaxMpaFB*／edtMinMpaFB* :1580、
 *      edD25_* :1570、edtArm*Offset_* edtContactOffsetClick :1431；動態面板的 edtHotOffset :1399、edtContactOffset* :1431／:1436
 *      （面板是開頁之後才建 ⇒ 不經引擎的 attachKeyboards，這裡自己掛）。edtLoadRate* golden Enabled=false（按不到）。
 *  (5) Button1（golden Button1Click :1588）：edtMaxVol=((Mid-Min)/5)*9+Min（只算畫面，不存檔）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'ContactForce';
  var PAGE = 'Setup.ContactForce.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__cfC) return;
  R.__cfC = true;

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info('[ContactForce/C] ' + msg);
  }
  function fire(el) {                                    // 同引擎小鍵盤 onCommit：補發 input／change
    ['input', 'change'].forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function vclFloat(v) { return String(v); }             // AnsiString(double)：1 → "1"、0.95 → "0.95"（同 JS 最短表示）

  /* ---- (1) 動態面板 ---------------------------------------------------------------------------------- */
  // [角色, 種類, Left, Top, Width, Height, 小鍵盤]（golden 建構子的座標；小鍵盤＝golden OnClick 的 ShowQwertyKey）
  var KB_HOT = ['DOUBLE', 2, true, 0.5, -0.5];           // golden edtHotOffsetClick :1399
  var KB_OFS = ['DOUBLE', 2, true, 10.0, -10.0];         // golden edtContactOffsetClick :1431／edtContactOffset_NSClick :1436
  var GEO = {
    SLKClass: { gb: 'gbLoadRate', min: 80, cap: function (it) { return 'Load rate of ' + it.sDiameter + ' mm'; }, parts: [
      ['lblDiameter', 'lb', 6, 31], ['lblDiameter_NS', 'lb', 6, 63], ['lblHotOffset', 'lb', 464, 30], ['lblContactOffset', 'lb', 464, 62],
      ['edtHotOffset', 'ed', 678, 26, 80, 28, KB_HOT], ['edtContactOffset', 'ed', 622, 58, 80, 28, KB_OFS],
      ['trckbrDiameter', 'tk', 108, 26, 275, 35, 'edtLoadRate'], ['trckbrDiameter_NS', 'tk', 108, 58, 275, 35, 'edtLoadRate_NS'],
      ['edtLoadRate', 'ed', 380, 26, 65, 28], ['edtLoadRate_NS', 'ed', 380, 58, 65, 28],
      ['lblContactOffset_NS', 'lb', 464, 58], ['edtContactOffset_NS', 'ed', 650, 58, 80, 28, KB_OFS]] },          // golden :21-206
    SLKIndClass: { gb: 'gbLoadRateInd', min: 80, cap: null, parts: [
      ['lblDiameterInd', 'lb', 6, 31], ['lblContactOffsetInd', 'lb', 464, 62], ['edtContactOffsetInd', 'ed', 676, 58, 80, 28, KB_OFS],
      ['trckbrDiameterInd', 'tk', 108, 26, 275, 35, 'edtLoadRateInd'], ['edtLoadRateInd', 'ed', 380, 26, 65, 28]] },   // golden :1439-1561
    DieForceSLKClass: { gb: 'gbDieForceLoadRate', min: 80, cap: function (it) { return 'DieForce Load rate of ' + it.sDiameter + ' mm'; }, parts: [
      ['lblDieForceDiameter', 'lb', 6, 31], ['lblDieForceContactOffset', 'lb', 464, 62], ['edtDieForceContactOffset', 'ed', 622, 58, 80, 28, KB_OFS],
      ['trckbrDieForceDiameter', 'tk', 108, 26, 275, 35, 'edtDieForceLoadRate'], ['edtDieForceLoadRate', 'ed', 380, 26, 65, 28]] },   // golden :331-419
    DieForceOneByOneSLKClass: { gb: 'gbDieForceOneByOneLoadRate', min: 80, cap: null, parts: [
      ['lblDieForceOneByOneDiameter', 'lb', 6, 31], ['lblDieForceOneByOneContactOffset', 'lb', 464, 24],
      ['edtDieForceOneByOneContactOffset', 'ed', 676, 26, 80, 28, KB_OFS],
      ['trckbrDieForceOneByOneDiameter', 'tk', 108, 26, 275, 35, 'edtDieForceOneByOneLoadRate'], ['edtDieForceOneByOneLoadRate', 'ed', 380, 26, 65, 28]] }   // golden :209-328
  };
  // Ind／OneByOne 的群組標題 golden 依 DOUBLE_EP_MULTI 分 Arm1_n／Arm2_n（ContactForce.cpp:1452-1466、:217-231）；標籤的字由 C++ 帶（TLabel caption）
  function indCap(kind, it, multi) {
    var base = kind === 'SLKIndClass' ? 'Load rate of ' : 'DieForce One By One Load rate of ';
    if (!multi) return base + it.sDiameter + ' mm';
    var n = (it.iTag % 8) + 1;
    return base + it.sDiameter + ' mm ' + (n <= 4 ? 'Arm1_' + n : 'Arm2_' + (n - 4));
  }
  function put(p, tag, cls, css) { var e = document.createElement(tag); if (cls) e.className = cls; e.style.cssText = 'position:absolute;' + css; p.appendChild(e); return e; }
  function keypadOn(el, k) {                             // 面板欄位的小鍵盤（golden OnClick → ShowQwertyKey）
    if (!el || !k || !window.HTQwerty) return;
    el.setAttribute('readonly', 'readonly');
    el.style.cursor = 'pointer';
    el.addEventListener('mousedown', function (ev) {
      ev.preventDefault();
      if (el.disabled || el.getAttribute('aria-disabled') === 'true') return;
      HTQwerty.show(el, HTQwerty.N[k[0]] || 0, { dp: k[1], checkRange: !!k[2], min: k[3], max: k[4], onCommit: function () { fire(el); } });
    });
  }
  function buildPanels(d) {
    var P = d && d.extra && d.extra.panels;
    if (!P) { setTimeout(function () { say2('⚠ 後端回應沒有 extra.panels（golden 建構子 new 的動態面板）：Load rate 分頁沒有可存的欄位', '#f88'); }, 0); return; }
    var multi = d.extra.INSTALL_DOUBLE_EP === 3;         // DOUBLE_EP_MULTI（cmydef.h）——只影響 Ind／OneByOne 的群組標題
    Object.keys(GEO).forEach(function (kind) {
      var g = GEO[kind], src = P[kind], host = src && $(src.parent);
      if (!host) return;
      while (host.firstChild) host.removeChild(host.firstChild);   // 拿掉 page-widgets.js 畫的設計期樣板
      var y = 2, w = Math.max(842, (host.clientWidth || 0) - 4);
      (src.items || []).forEach(function (it) {
        var ids = it.ids || {};
        var gb = document.createElement('fieldset');
        gb.className = 'gbx';
        gb.id = ids[g.gb] || '';
        gb.title = gb.id + ' : TGroupBox（golden ' + kind + '，執行期 new）';
        gb.style.cssText = 'position:absolute;left:0px;top:' + y + 'px;width:' + w + 'px;height:100px;';
        if (!it.bShow) gb.style.display = 'none'; else y += 100;   // alTop：看不見的組不佔位置
        var lg = document.createElement('legend');
        lg.style.cssText = 'background:var(--form-bg,#ece9d8);font-size:16px;line-height:18px;';
        lg.textContent = g.cap ? g.cap(it) : indCap(kind, it, multi);
        gb.appendChild(lg);
        var cli = put(gb, 'div', 'cli', 'inset:0;overflow:hidden;');
        g.parts.forEach(function (p) {
          var id = ids[p[0]];
          if (!id) return;
          var el;
          if (p[1] === 'lb') { el = put(cli, 'span', 'lb', 'left:' + p[2] + 'px;top:' + p[3] + 'px;font-size:14px;white-space:nowrap;'); }
          else if (p[1] === 'tk') {
            el = put(cli, 'input', 'trk', 'left:' + p[2] + 'px;top:' + p[3] + 'px;width:' + p[4] + 'px;height:' + p[5] + 'px;margin:0;');
            el.type = 'range'; el.min = g.min; el.max = 150; el.step = 1; el.value = 100;
            (function (tk, rateRole) {                     // golden trckbr*_Change：edtLoadRate*->Text=AnsiString(Position/100.0)
              tk.addEventListener('input', function () { var r = $(ids[rateRole]); if (r) r.value = vclFloat(parseInt(tk.value, 10) / 100); });
            })(el, p[6]);
          } else {
            el = put(cli, 'input', 'ed', 'left:' + p[2] + 'px;top:' + p[3] + 'px;width:' + p[4] + 'px;height:' + p[5] + 'px;font-size:16px;');
            el.type = 'text';
            if (p[6]) keypadOn(el, p[6]); else { el.readOnly = true; el.disabled = true; }   // edtLoadRate*：golden Enabled=false
          }
          el.id = id;
          el.title = id + '（golden ' + kind + '::' + p[0] + '）';
        });
        host.appendChild(gb);
      });
      host.style.position = 'relative';
      host.style.minHeight = (y + 4) + 'px';
    });
  }

  /* ---- (2) tsNewMethod 的 ShowValue（golden :1382-1390） ------------------------------------------------- */
  var SHOW = [['tb30mm_10kg', 'lab30mm10kgNum'], ['tb30mm_60kg', 'lab30mm60kgNum'], ['tb40mm_10kg', 'lab40mm10kgNum'],
              ['tb40mm_60kg', 'lab40mm60kgNum'], ['tb60mm_10kg', 'lab60mm10kgNum'], ['tb60mm_60kg', 'lab60mm60kgNum']];
  function showValue() { SHOW.forEach(function (p) { var t = $(p[0]), l = $(p[1]); if (t && l) l.textContent = String(parseInt(t.value, 10) || 0); }); }
  function hookStatic() {
    ['tb30mm_10kg', 'tb30mm_60kg', 'tb40mm_10kg', 'tb40mm_60kg', 'tb60mm_10kg', 'tb60mm_60kg', 'tb56mm_10kg', 'tb56mm_60kg'].forEach(function (id) {
      var t = $(id);
      if (t && !t.__cfC) { t.__cfC = true; t.addEventListener('input', showValue); }   // golden tb30mm_10kgChange :1375（ADAM_DirectWriteData 不送，見檔頭 (2)）
    });
    var b1 = $('Button1');                                 // golden Button1Click :1588
    if (b1 && !b1.__cfC) {
      b1.__cfC = true;
      b1.addEventListener('click', function () {
        if (b1.disabled) return;
        var mn = parseFloat(($('edtMinVol') || {}).value), md = parseFloat(($('edtMidVol') || {}).value), mx = $('edtMaxVol');
        if (!isFinite(mn) || !isFinite(md)) { say2('\'' + (isFinite(mn) ? ($('edtMidVol') || {}).value : ($('edtMinVol') || {}).value) + '\' is not a valid floating point value（golden ToDouble 例外）', '#f88'); return; }
        if (mx) mx.value = vclFloat(((md - mn) / 5) * 9 + mn);
      });
    }
    var s = $('btSave');                                   // golden btSaveClick :935
    if (s && !s.__cfC) {
      s.__cfC = true;
      s.title = 'btSave : golden btSaveClick（ContactForce.cpp:935）→ iContactForceMap／LastSet.dIndexLoadRate → SaveLastSetIni → WriteFile（ContactInfo.ini、Gerneral.ini EP 鍵）→ ReadFile';
      s.addEventListener('click', function (ev) {
        ev.preventDefault(); ev.stopPropagation();
        if (s.disabled || s.getAttribute('aria-disabled') === 'true') return;
        if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔', '#f88'); return; }
        if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) { say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88'); return; }
        HT9045Page.save();
      });
    }
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hookStatic); else hookStatic();

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      try { buildPanels(d); } catch (e) { if (window.console) console.error('[ContactForce/C] buildPanels', e); }   // 引擎在這個 promise 的 then 裡才套值
      setTimeout(showValue, 0);                            // golden FormShow :910 ShowValue（引擎套完 position 之後）
      return d;
    });
  };

  /* ---- (4) 小鍵盤（靜態元件；golden 範圍原樣，min>max 的由 qwerty.js 取較小／較大） --------------------------- */
  var kb = {};
  ['edMaxKpa', 'edMaxKpaDual', 'edMaxKpa_1032'].forEach(function (id) { kb[id] = ['INTEGER', 0, true, 400, 950]; });        // golden edMaxKpaClick :1426
  ['edMinMpa', 'edMinMpaDual', 'edMinMpa_1032'].forEach(function (id) { kb[id] = ['DOUBLE', 3, true, -1.0, 10.0]; });       // golden edMinMpaClick :1575
  ['edMaxMpaFB', 'edtMinMpaFB', 'edMaxMpaFBDual', 'edtMinMpaFBDual', 'edMaxMpaFB_1032', 'edtMinMpaFB_1032'].forEach(function (id) { kb[id] = ['DOUBLE', 3, true, 0.0, 6.0]; });   // golden edMaxMpaFBClick :1580
  ['edD25_60mm', 'edD25_40mm', 'edD25_30mm', 'edD60_56mm'].forEach(function (id) { kb[id] = ['DOUBLE', 3, true, 0.5, -0.5]; });   // golden edD25_60mmMouseDown :1570
  for (var a = 1; a <= 2; a++) for (var j = 1; j <= 15; j++) kb['edtArm' + a + 'Offset_' + (j < 10 ? '0' : '') + j] = ['DOUBLE', 2, true, 10.0, -10.0];   // golden edtContactOffsetClick :1431
  if (window.HT9045Wire && HT9045Wire.register) HT9045Wire.register({ page: PAGE, slug: 'contactforce_c', fields: {}, optional: {}, kb: kb });
})();
