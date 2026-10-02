/* ht9045_hsys_events_c.js -- HW.HandlerSys: golden THandlerSystem's on-screen handlers that the C route (FileRW/HSys.cpp) does not run.
 *
 * AI(W906-HSYS-EVT) 20261002: the every-component check (compK ioS) found these golden OnClick / OnChange / OnMouseDown handlers with
 *   no web handler. None of them touches a file or the machine -- they only change what the form shows or what the next Save reads --
 *   so they run here, on the page, as golden runs them on the form (906 HandlerSys.cpp). Values they set are saved by the page's Save
 *   (C route editlist.save, golden SaveSystemSet) like any value the operator types. No confirm dialogs (golden has none here).
 *   rgTTLCardClick / rgRotateKit_TypeClick are also replayed by C++ before the save (FileRW/HSys.cpp BeforeApply, AI(W906-HSYS-EVT)),
 *   so a value they re-enable is not dropped as "not editable".
 *     rgTTLCard        OnClick     :1246-1255  card 3 -> Address = Yes, greyed; else Address enabled
 *     rgRotateKit_Type OnClick     :1132-1136  In / Out enabled only for type 0
 *     rgCustomerList   OnClick     :1054-1060  Customer Code = first 3 characters ("000" -> "0")
 *     ExitBtn          OnMouseDown :1062-1071  right button: Customer Code tab shown and selected, search box shown
 *     pcSetting        OnChange    :1144-1147  search box visible only on the Customer Code tab
 *     edtSearchCode    OnChange    :1149-1168  customer list filtered (upper-case substring), selection cleared
 *     btnSetATCCom     OnClick     :1081-1098  the 15 COM-port boxes set to golden's defaults
 *     BtnEnableAll     OnClick     :1045-1052  Safe-Door 1..10 unchecked (its group box is hidden by FormShow :41, as golden)
 *     edtSearchFunction OnChange   :1220-1244  + SortItemToMap :1170-1199 (FormShow :36): matched radio / group boxes move to the
 *                                              Search Function box
 *     FormShow :56-57 / FormClose :1076        first tab selected / Customer Code tab hidden again when the window opens / closes
 */
(function () {
  'use strict';
  function $(id) { return document.getElementById(id); }
  function radios(el) { return el ? Array.prototype.slice.call(el.querySelectorAll('input[type="radio"]')) : []; }
  function itemIndex(el) { var r = radios(el); for (var i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; }
  function setItemIndex(el, i) { var r = radios(el); r.forEach(function (x, k) { x.checked = (k === i); }); }
  function fire(el, ev) { try { el.dispatchEvent(new Event(ev, { bubbles: true })); } catch (e) {} }
  function upperAscii(s) { return String(s).replace(/[a-z]+/g, function (x) { return x.toUpperCase(); }); }   // AnsiString::UpperCase on Big5 text
  function big5Len(s) { var n = 0; for (var i = 0; i < s.length; i++) n += s.charCodeAt(i) < 0x80 ? 1 : 2; return n; }   // AnsiString::Length = bytes
  function tabOf(id) { return document.querySelector('.tab[data-htitle^="' + id + ' :"]') || document.querySelector('.tab[title^="' + id + ' :"]'); }
  // VCL Enabled on a radio group: the same marking the engine uses (ht9045_wire_engine.js gbSetEnabled), so a later engine load
  //   can still turn it back; never enables inside a container someone else disabled (login level etc.)
  function setEnabled(el, on) {
    if (!el) return;
    if (on) {
      for (var p = el.parentNode; p && p.getAttribute; p = p.parentNode) if (p.getAttribute('aria-disabled') === 'true') return;
      el.removeAttribute('aria-disabled');
      el.querySelectorAll('input').forEach(function (x) { x.disabled = false; x.removeAttribute('data-gb-dis'); });
    } else {
      el.setAttribute('aria-disabled', 'true');
      el.querySelectorAll('input').forEach(function (x) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } });
    }
  }
  function onRadioChange(id, fn) {
    var el = $(id); if (!el) return;
    el.addEventListener('change', function (ev) { if (ev.target && ev.target.type === 'radio' && ev.target.checked) fn(el, ev.target); });
  }

  // ---- rgTTLCardClick :1246-1255 ----
  onRadioChange('rgTTLCard', function (rg) {
    var addr = $('rgTTLUseAddress');
    if (itemIndex(rg) === 3) { setItemIndex(addr, 1); setEnabled(addr, false); }   // :1250-1251
    else setEnabled(addr, true);                                                   // :1254
  });
  // ---- rgRotateKit_TypeClick :1132-1136 ----
  onRadioChange('rgRotateKit_Type', function (rg) {
    var on = itemIndex(rg) === 0;
    setEnabled($('rgRotateKitIn'), on);
    setEnabled($('rgRotateKitOut'), on);
  });
  // ---- rgCustomerListClick :1054-1060 ----
  onRadioChange('rgCustomerList', function (rg, input) {
    var lab = input.closest('label'), str = lab ? lab.textContent : '';
    var ed = $('edtCustomerCode'); if (!ed) return;
    ed.value = str.substr(0, 3);                                                   // :1057 SubString(1, 3)
    if (ed.value === '000') ed.value = '0';                                        // :1058-1059
    fire(ed, 'input'); fire(ed, 'change');
  });

  // ---- pcSettingChange :1144-1147 / ExitBtnMouseDown :1062-1071 / FormShow :56-57 / FormClose :1076 ----
  var codeBox = $('edtSearchCode') && $('edtSearchCode').parentNode;               // the TLabeledEdit wrapper (label + edit)
  var custTab = tabOf('tsCustomerCode');
  var pcTabs = $('pcSetting') && $('pcSetting').querySelector(':scope > .pcTabs');
  var quiet = false;                                                               // programmatic ActivePageIndex: VCL fires no OnChange
  var custShown = false;                                                           // shown by the right click in this window opening
  function activeTabIs(tab) { return !!(tab && tab.classList.contains('act')); }
  if (pcTabs) pcTabs.querySelectorAll(':scope > .tab').forEach(function (tb) {
    tb.addEventListener('click', function () {                                     // runs after the page's own tab switch
      if (quiet || !codeBox) return;
      codeBox.style.display = activeTabIs(custTab) ? '' : 'none';                  // :1146
    });
  });
  function selectTab(t) {
    var tb = pcTabs && pcTabs.querySelector(':scope > .tab[data-t="' + t + '"]');
    if (tb) tb.click();
  }
  var exitBtn = $('ExitBtn');
  if (exitBtn) exitBtn.addEventListener('mousedown', function (ev) {
    if (ev.button !== 2) return;                                                   // :1065 mbRight
    if (custTab) custTab.style.display = '';                                       // :1067 TabVisible=true
    custShown = true;
    selectTab(7);                                                                  // :1068 ActivePageIndex=7 (= tsCustomerCode)
    if (codeBox) codeBox.style.display = '';                                       // :1069
  });
  // golden keeps the tab until the window closes; the C route's reload (Load / after Save = golden FormShow :57 / ExitBtnClick data
  //   half) hides it, which golden's Load (:985 LoaderSystemSet only) and Save (:979) do not -> keep it while this opening showed it
  if (custTab && window.MutationObserver) new MutationObserver(function () {
    if (custShown && custTab.style.display === 'none') custTab.style.display = '';
  }).observe(custTab, { attributes: true, attributeFilter: ['style'] });

  // ---- edtSearchCodeChange :1149-1168 ----
  var custList = $('rgCustomerList');
  var custItems = custList ? Array.prototype.slice.call(custList.querySelectorAll('label.rgi')) : [];   // slCustomerCode (ctor :21-23)
  var codeEd = $('edtSearchCode');
  if (codeEd) codeEd.addEventListener('input', function () {
    var q = upperAscii(codeEd.value);
    custItems.forEach(function (lab) {
      var show = q === '' || upperAscii(lab.textContent).indexOf(q) >= 0;          // :1152-1157 all / :1163-1165 AnsiPos != 0
      lab.style.display = show ? '' : 'none';
    });
    setItemIndex(custList, -1);                                                    // Items->Clear(): ItemIndex = -1
  });

  // ---- btnSetATCComClick :1081-1098 ----
  var ATC_DEFAULT = [['cbComIndex', 'COM11'], ['cbComTemp', 'COM12'], ['cbComTempOmron', 'COM13'], ['cbComDyTemp', 'COM4'],
                     ['cbComTester', 'COM2'], ['cbComBinDisp', 'COM14'], ['cbComRTC', 'COM1'], ['cbbATC1', 'COM15'],
                     ['cbbATC2', 'COM16'], ['cbbATC3', 'COM17'], ['cbbATC4', 'COM18'], ['cbOCR', 'COM3'], ['cbAirCon', 'COM15'],
                     ['cbComTTLRS232', 'COM3'], ['cbComTTLRS232_2', 'COM8']];
  function setComboText(el, text) {                                                // TComboBox->Text = text
    var olds = el.querySelectorAll('option[data-src="cpp-text"]');
    for (var i = 0; i < olds.length; i++) olds[i].parentNode.removeChild(olds[i]);
    for (var k = 0; k < el.options.length; k++) if (el.options[k].textContent === text) { el.selectedIndex = k; return; }
    var o = document.createElement('option'); o.textContent = text; o.setAttribute('data-src', 'cpp-text');   // not in Items (csDropDown)
    el.appendChild(o); el.selectedIndex = el.options.length - 1;
  }
  var atcBtn = $('btnSetATCCom');
  if (atcBtn) atcBtn.addEventListener('click', function () {
    ATC_DEFAULT.forEach(function (p) { var el = $(p[0]); if (el && el.tagName === 'SELECT') { setComboText(el, p[1]); fire(el, 'change'); } });
  });

  // ---- BtnEnableAllClick :1045-1052 ----
  var allBtn = $('BtnEnableAll');
  if (allBtn) allBtn.addEventListener('click', function () {
    for (var i = 1; i <= 10; i++) {
      var c = $('SafeDoor' + i), cb = c && (c.tagName === 'INPUT' ? c : c.querySelector('input[type="checkbox"]'));
      if (cb) { cb.checked = false; fire(cb, 'change'); }
    }
  });

  // ---- SortItemToMap :1170-1199 + edtSearchFunctionChange :1220-1244 ----
  var SORT_PANELS = ['pnlHandler1', 'pnlHandler2', 'pnlHandler3', 'pnlLoader1', 'pnlLoader2', 'pnlLoader3', 'pnlLoader4', 'pnlLoader5',
                     'pnlInArm1', 'pnlInArm2', 'pnlInArm3', '=gbRotateKit', '=grpOther', 'pnlESD1', 'pnlESD2', 'pnlOther1', 'pnlSht1',
                     'pnlSht2', 'pnlIndex1', 'pnlIndex2', 'pnlIndex3', 'pnlIndex4', 'pnlIndex5'];   // '=' = pushed itself (:1185-1186)
  var ALIGN = { rgTTLUseAddress: 'alNone', rgScanner_AOI: 'alNone', gbRotateKit: 'alLeft', grpOther: 'alLeft' };   // HandlerSys.dfm; others alTop
  var box = $('scrlbxSearchFunc'), fnEd = $('edtSearchFunction');
  var TempComp = [], ORIG = {}, seq = 0;
  function captionOf(el) { var lg = el.querySelector(':scope > legend'); return upperAscii(lg ? lg.textContent : ''); }   // ctor: Caption.UpperCase()
  function remember(el) {                                                          // where the control sits in its own parent
    if (ORIG[el.id]) return ORIG[el.id];
    var mark = document.createComment('hsys-search ' + el.id);
    el.parentNode.insertBefore(mark, el);
    ORIG[el.id] = { mark: mark, css: el.style.cssText, top: parseInt(el.style.top, 10) || 0 };
    return ORIG[el.id];
  }
  function pushItem(el) { if (el && el.id) { remember(el); TempComp.push({ el: el, par: el.parentNode, caption: captionOf(el) }); } }
  function SortItemToMap() {
    SORT_PANELS.forEach(function (p) {
      if (p.charAt(0) === '=') { pushItem($(p.slice(1))); return; }
      var pnl = $(p); if (!pnl) return;
      Array.prototype.forEach.call(pnl.children, function (c) { if (c.tagName === 'FIELDSET' && c.classList.contains('gbx')) pushItem(c); });   // InitItemToMap :1201-1218
    });
  }
  function shown(el) { return el.style.display !== 'none' && el.style.visibility !== 'hidden'; }
  function realign() {                                                             // VCL AlignControls in the scroll box
    var kids = Array.prototype.filter.call(box.children, function (c) { return c.hasAttribute && c.hasAttribute('data-hsys-in'); });
    var tops = kids.filter(function (c) { return (ALIGN[c.id] || 'alTop') === 'alTop'; })
                   .sort(function (a, b) { return (+a.getAttribute('data-hsys-key') - +b.getAttribute('data-hsys-key')) ||
                                                  (+a.getAttribute('data-hsys-seq') - +b.getAttribute('data-hsys-seq')); });
    var y = 0, w = box.clientWidth || (parseInt(box.style.width, 10) - 2);
    tops.forEach(function (c) {
      c.style.left = '0px'; c.style.top = y + 'px'; c.style.width = w + 'px'; c.style.right = 'auto'; c.style.bottom = 'auto';
      c.setAttribute('data-hsys-key', y);
      if (shown(c)) y += parseInt(c.style.height, 10) || c.offsetHeight || 0;      // invisible controls take no room
    });
    var x = 0, h = Math.max(0, (box.clientHeight || parseInt(box.style.height, 10) || 0) - y);
    kids.filter(function (c) { return ALIGN[c.id] === 'alLeft'; }).forEach(function (c) {
      c.style.left = x + 'px'; c.style.top = y + 'px'; c.style.height = h + 'px'; c.style.right = 'auto'; c.style.bottom = 'auto';
      if (shown(c)) x += parseInt(c.style.width, 10) || c.offsetWidth || 0;
    });
  }
  function setParent(t, par) {                                                     // TempComp[i]->Obj->Parent = par
    var el = t.el;
    if (el.parentNode === par) return;
    if (par === box) {                                                             // keeps Left / Top (alNone stays where it was)
      el.setAttribute('data-hsys-in', '1');
      el.setAttribute('data-hsys-key', ORIG[el.id].top);
      el.setAttribute('data-hsys-seq', ++seq);
      box.appendChild(el);
    } else {
      // golden-odd not copied: VCL re-aligns the old panel by Top, and the control comes back with the Top it had in the box, so it
      //   can come back higher up in its panel; here it goes back exactly where it was
      var o = ORIG[el.id];
      el.removeAttribute('data-hsys-in'); el.removeAttribute('data-hsys-key'); el.removeAttribute('data-hsys-seq');
      el.style.cssText = o.css;
      o.mark.parentNode.insertBefore(el, o.mark);
    }
  }
  if (fnEd && box) fnEd.addEventListener('input', function () {
    var text = fnEd.value;
    if (text === '') {
      // golden-odd, kept: :1226 TempComp[i]->Obj->Parent=TempComp[i]->Obj->Parent -- clearing the box puts nothing back
    } else if (big5Len(text) > 2) {                                                // :1229 Length()>2 (bytes; one Chinese character = 2)
      var q = upperAscii(text);
      TempComp.forEach(function (t) {
        if (t.caption.indexOf(q) >= 0) setParent(t, box);                          // :1233-1235
        else if (t.el.parentNode !== t.par) setParent(t, t.par);                   // :1239-1240
      });
      realign();
    }
  });

  // ---- window opening / closing (background.html HT_WIN) ----
  var openNow = true;
  SortItemToMap();                                                                 // FormShow :36 (first opening = page load)
  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
    if (!m.open) {                                                                 // FormClose :1076 / ExitBtnClick :1039
      openNow = false; custShown = false;
      if (custTab) custTab.style.display = 'none';
      return;
    }
    if (openNow) return;
    openNow = true;
    // golden-odd, kept: SortItemToMap runs on every FormShow and appends -- a control left in the search box is recorded again with
    //   the box as its "own" parent, so later searches that do not match it leave it there (TempComp is never cleared until FormDestroy)
    SortItemToMap();
    quiet = true; selectTab(0); quiet = false;                                     // :56 ActivePageIndex=0 (no OnChange in VCL)
    if (custTab) custTab.style.display = 'none';                                   // :57
  });
  window.HT9045HSysEvents = { TempComp: TempComp, realign: realign, SortItemToMap: SortItemToMap };
})();
