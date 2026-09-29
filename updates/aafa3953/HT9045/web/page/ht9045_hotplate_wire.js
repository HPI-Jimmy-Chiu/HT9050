/* ht9045_hotplate_wire.js -- Setup.HotPlate.html <-> HotPlate.Data 接線
 * ---------------------------------------------------------------------------
 * AI(W906-FW-HOTPLATE) 20260914。照 ht9045_contact_wire.js 的形狀做，
 * load()/save() 邏輯原樣沿用，只換 DOC 與 FIELD_MAP。
 *
 * 用法（在 HTML 的 </body> 之前，順序不可顛倒）：
 *   <script src="ht9045_recipe_client.js"></script>
 *   <script src="ht9045_hotplate_wire.js"></script>
 *
 * ---------------------------------------------------------------------------
 * FIELD_MAP 的來源：golden cHotPlate.cpp 的 WriteIniData 那一側
 * ---------------------------------------------------------------------------
 * 這一頁不用兩跳法也不用 HTEditList（cHotPlate.cpp 的 el*->Add( 是 0 個）。
 * 寫入端直接把 (區段, 鍵) 與 widget 配在同一行，是最不會出錯的來源：
 *
 *   cHotPlate.cpp:598  WriteIniData(szDir, "Hotplate Form", "Name",       HotPlateName->Text);
 *   cHotPlate.cpp:599  WriteIniData(szDir, "Hotplate Form", "X Start",    FormatFloat("0.000", XST1->Text.ToDouble()));
 *   cHotPlate.cpp:603  WriteIniData(szDir, "Hotplate Form", "X Division", XCT1->Text);
 *
 * 讀取端 :162-:170 用同一組 (區段, 鍵)，兩側一致。
 *
 * ⚠ 一個已經擋下來的誤配，記在這裡免得有人重踩：
 *   通用抽取器（scratchpad/extract_page_map.py）把本頁的 XCT1 / XPitch1 /
 *   XST1 / YCT1 / YPitch1 / YST1 抽成 Tray.Data 的 [Type0] X Division 等——
 *   因為 Setup.TrayForm.html 也有同名 id，而抽取器是全樹依 id 反查、沒有
 *   限定來源檔。照抽出來的結果接，會把 HotPlate 的值寫進 Tray.Data。
 *   本檔用的是 cHotPlate.cpp 親自驗過的對照，不是抽取器的輸出。
 *
 * ---------------------------------------------------------------------------
 * 涵蓋率（20260914 掃描 D:\HT9045\IniData\Data 全部 216 個配方）
 * ---------------------------------------------------------------------------
 *   有 HotPlate.Data 的配方          215
 *   下列 7 個鍵                      215/215  <- 全有，無條件安全
 *   Using Flag                       215/215  <- 全有，但是位元欄位，見 PENDING
 *   Use Wide Hotplate                213/215  <- 配方相依，見 PENDING
 *
 * ---------------------------------------------------------------------------
 * 三條規則沿用，不要破壞
 * ---------------------------------------------------------------------------
 * 1. 寫入前一定 preview，把 changed 顯示給操作員看。
 * 2. notFound 不是空的就拒絕寫入 —— 那代表 FIELD_MAP 有錯，不是資料有錯。
 * 3. 寫完一定重讀回填。
 *
 * ⚠ 互鎖不要寫在這裡。該擋的由 C++ 端擋。
 */
(function () {
  'use strict';

  var DOC = 'hotPlate';

  /* 頁面 id -> [區段, 鍵]   (golden cHotPlate.cpp:598-604，7 筆純文字欄位) */
  var FIELD_MAP = {
    HotPlateName: ['Hotplate Form', 'Name'],        // :598
    XST1:         ['Hotplate Form', 'X Start'],     // :599  FormatFloat("0.000")
    YST1:         ['Hotplate Form', 'Y Start'],     // :600  FormatFloat("0.000")
    XPitch1:      ['Hotplate Form', 'X Pitch'],     // :601  FormatFloat("0.000")
    YPitch1:      ['Hotplate Form', 'Y Pitch'],     // :602  FormatFloat("0.000")
    XCT1:         ['Hotplate Form', 'X Division'],  // :603  整數，原樣送出
    YCT1:         ['Hotplate Form', 'Y Division']   // :604  整數，原樣送出
  };

  /* 抽到了但還不能接的 4 筆 —— 需要人先確認語意，見檔尾 */
  var PENDING = {
    cbEnableHP1:          ['Hotplate Form', 'Using Flag',        'checkbox bit0 (+1)'],
    cbEnableHP2:          ['Hotplate Form', 'Using Flag',        'checkbox bit1 (+2)'],
    chkUseWideHotplate:   ['Hotplate Form', 'Use Wide Hotplate', 'checkbox, 213/215 配方才有'],
    chkTrayHotplateCheck: ['System',        'bTrayHotplateCheck','checkbox, 不同區段且走 CheckAndReadIniData'],
    cbSelectHPFromDB:     ['-',             '-',                 'UI 控制項，不是配方欄位（cHotPlate.cpp:627 只設 Items/Text）']
  };
  /* ---------------------------------------------------------------------
   * 小鍵盤：旗標與範圍不是猜的，是 golden cHotPlate.cpp + cHotPlate.dfm 抽的
   * ---------------------------------------------------------------------
   *   cHotPlate.cpp:386  ShowQwertyKey((TEdit*)Sender, N_DOUBLE,  3, true, 0.001, 1000.00);
   *   cHotPlate.cpp:396  ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 1,     1000);
   *   cHotPlate.cpp:637  ShowQwertyKey((TEdit*)Sender, N_NO_SYMBOL);
   *
   *   cHotPlate.dfm      XST1/XPitch1/YST1/YPitch1 -> OnMouseDown = XST1MouseDown
   *                      XCT1/YCT1                 -> OnMouseDown = XCT1MouseDown
   *                      HotPlateName              -> OnMouseDown = HotPlateNameMouseDown
   *
   * BCB6 的簽章 ShowQwertyKey(edit, flags, dp, checkRange, min, max) 與
   * HTQwerty.show(target, flags, {dp, checkRange, min, max}) 一一對應。
   * 觸發事件同樣用 mousedown，跟機台一致。
   */
  var KB = {
    XST1:    ['DOUBLE',    3, true, 0.001, 1000.00],
    XPitch1: ['DOUBLE',    3, true, 0.001, 1000.00],
    YST1:    ['DOUBLE',    3, true, 0.001, 1000.00],
    YPitch1: ['DOUBLE',    3, true, 0.001, 1000.00],
    XCT1:    ['INTEGER',   0, true, 1,     1000],
    YCT1:    ['INTEGER',   0, true, 1,     1000],
    HotPlateName: ['NO_SYMBOL', 0, false, 0, 0]
  };

  function attachKeyboards() {
    if (typeof HTQwerty === 'undefined') {
      say('⚠ 小鍵盤未載入：請在本檔之前加入 <script src="qwerty.js"></script>', '#ffcc66');
      return 0;
    }
    var n = 0, unmapped = [];
    var inputs = document.querySelectorAll('input[type="text"], input:not([type])');
    Array.prototype.forEach.call(inputs, function (el) {
      if (!el.id || el.readOnly || el.disabled) return;
      var c = KB[el.id];
      // 沒有 golden 依據的輸入框：給全 QWERTY、不夾限。有依據的才帶範圍。
      var flags = c ? HTQwerty.N[c[0]] : 0;
      var opt = c ? { dp: c[1], checkRange: c[2], min: c[3], max: c[4] } : {};
      if (!c) unmapped.push(el.id);
      el.addEventListener('mousedown', function (ev) {
        ev.preventDefault();
        HTQwerty.show(el, flags, (el.id === 'XCT1' || el.id === 'YCT1') ? hp2Opt(opt) : opt);   // AI(W906-Q41) 20260927: HP-2（見 :122）
      }); if (el.id === 'XCT1' || el.id === 'YCT1') el.addEventListener('change', hp2Check);   // AI(W906-Q41) 20260927: 別的小鍵盤提交（引擎補發 change）也算
      el.setAttribute('readonly', 'readonly');   // 只能用小鍵盤輸入，跟機台一致
      el.style.cursor = 'pointer';
      n++;
    });
    if (unmapped.length) {
      console.warn('[hotplate-wire] 這些輸入框沒有 golden 旗標依據，用預設全 QWERTY：', unmapped);
    }
    return n;
  }
  /* AI(W906-Q41) 20260927 (St02-E): Q41 HP-2 —— golden TfHotPlate::XCT1MouseDown（906_0625_Steven／912 cHotPlate.cpp:389-399；DFM 912 :267 XCT1、:312 YCT1 都綁這支）：小鍵盤關掉後 XCT1 是 "1" 就把 XPitch1 設 "0"。golden 怪處照翻：YCT1 也綁這支，但只看 XCT1、只改 XPitch1。KYEC Barcode_Reader 客戶專屬，跳過。 */ function hp2Check() { var x = $('XCT1'), p = $('XPitch1'); if (!x || !p || x.value !== '1' || p.value === '0') return; p.value = '0'; ['input', 'change'].forEach(function (evn) { var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (e) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); } p.dispatchEvent(e2); }); } function hp2Opt(opt) { var o = {}; for (var k in opt) o[k] = opt[k]; var prev = o.onCommit; o.onCommit = function () { if (typeof prev === 'function') prev.apply(this, arguments); hp2Check(); }; return o; }
  // ---------------------------------------------------------------- helpers
  function $(id) { return document.getElementById(id); }

  function bar() {
    var b = $('ht9045WireBar');
    if (b) return b;
    b = document.createElement('div');
    b.id = 'ht9045WireBar';
    // AI(W906-FW-HOTPLATE) 20260914: 停靠在表單右側，不要用 bottom:0。
    // 這一頁的 Save / Exit 在 Panel2 (top:475px)，橫跨底部的固定列會整個蓋住
    // 它們，而且是視覺上完全看不到、按不到。表單寬 606px，所以從 614px 起算。
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
  function say(msg, colour) {
    var b = bar();
    b.style.display = '';
    b.style.color = colour || '#ddd';
    var t = b.firstChild;
    if (!t || t.nodeType !== 3) { t = document.createTextNode(''); b.insertBefore(t, b.firstChild); }
    t.nodeValue = msg;
    return msg;
  }

  // ------------------------------------------------------------------ load
  function load() {
    say('讀取中 ...');
    return HT9045Recipe.read(DOC).then(function (doc) {
      var filled = 0, missingKey = [], missingEl = [];
      Object.keys(FIELD_MAP).forEach(function (id) {
        var sec = FIELD_MAP[id][0], key = FIELD_MAP[id][1];
        var el = $(id);
        if (!el) { missingEl.push(id); return; }
        var s = doc.sections[sec];
        var cell = s && s[key];
        if (!cell) { missingKey.push(sec + ' / ' + key); return; }
        el.value = cell.value;
        filled++;
      });
      var lines = ['讀取完成：填入 ' + filled + ' / ' + Object.keys(FIELD_MAP).length + ' 個欄位'];
      if (missingEl.length)  lines.push('⚠ 頁面上找不到的 id ('  + missingEl.length  + ')：' + missingEl.join(', '));
      if (missingKey.length) lines.push('⚠ 配方裡沒有的鍵 ('    + missingKey.length + ')：' + missingKey.join(' | '));
      lines.push('尚未接線（需人工確認語意）：' + Object.keys(PENDING).length + ' 個，見 ht9045_hotplate_wire.js 檔尾');
      say(lines.join('\n'), missingEl.length || missingKey.length ? '#ffcc66' : '#9f9');
      // Steven 20260924 (S12)：本檔自己讀檔填值，可能比引擎晚完成而蓋掉 C++ 的值。
      // 讀完再跑一次引擎的 formOverlay()，誰最後完成結果都一樣。
      if (window.HT9045Page && window.HT9045Page.formOverlay) window.HT9045Page.formOverlay();
      return doc;
    }).catch(function (e) {
      say('讀取失敗：' + e.message + '\n（伺服器有起來嗎？F5 之後應該看到 http://127.0.0.1:8045/）', '#f88');
      throw e;
    });
  }

  // ------------------------------------------------------------------ save
  function collect() {
    var edits = {};
    Object.keys(FIELD_MAP).forEach(function (id) {
      var sec = FIELD_MAP[id][0], key = FIELD_MAP[id][1], el = $(id);
      if (!el) return;
      (edits[sec] = edits[sec] || {})[key] = String(el.value);
    });
    return edits;
  }

  function save() {
    var edits = collect();
    say('預演中（dryRun，不會寫入）...');
    return HT9045Recipe.preview(DOC, edits).then(function (p) {
      // 規則 2：notFound 不是空的 -> FIELD_MAP 有錯，停下來，不要寫
      if (p.notFound && p.notFound.length) {
        say('❌ 拒絕寫入：有 ' + p.notFound.length + ' 個鍵在配方裡不存在。\n' +
            '這是 FIELD_MAP 的錯，不是資料的錯。\n' + p.notFound.join('\n'), '#f88');
        return null;
      }
      var changed = p.changed || [];
      if (!changed.length) { say('沒有任何值改變，不寫入。', '#9f9'); return null; }

      // 規則 1：把「會改什麼」攤開給操作員看，由人按下確認
      var detail = changed.map(function (c) { return '  ' + JSON.stringify(c); }).join('\n');
      if (!window.confirm('即將寫入 ' + changed.length + ' 個值到配方 ' + DOC + '：\n\n' +
                          detail + '\n\n確定要寫入嗎？')) {
        say('已取消，未寫入。', '#ffcc66');
        return null;
      }
      say('寫入中 ...');
      return HT9045Recipe.write(DOC, edits).then(function (w) {
        // 規則 3：重讀回填，不要拿送出去的物件當結果
        return load().then(function () {
          say('✅ 寫入完成並已重讀驗證：changed=' + (w.changed || []).length +
              '  identical=' + (w.identical || []).length, '#9f9');
          return w;
        });
      });
    }).catch(function (e) {
      say('寫入失敗：' + e.message, '#f88');
      throw e;
    });
  }

  // -------------------------------------------------------------- wire up
  function attach() {
    /* Steven 20260916（使用者要求）：不再注入浮動的 "Save to recipe" / "Reload"。
     * ------------------------------------------------------------------------
     * 那兩顆是「找不到頁面自己的存檔鈕就自己生一顆」的退路，用 position:fixed
     * 蓋在畫面上，不屬於這張 dfm。ht9045_wire_engine.js 今天已經這樣改了，
     * 這支獨立接線檔照同一個做法，免得兩套接線的行為不一致。
     *
     * 本頁的 spbSave 一直都找得到，所以浮動的存檔鈕其實沒有生出來，多出來的
     * 是那顆無條件注入的 Reload。現在兩顆都不生：存檔按頁面自己的 spbSave，
     * 重讀在 Console 用 HT9045HotPlate.load()。找不到存檔鈕時不再自己生一顆，
     * 改在狀態列講清楚這一頁唯讀。
     *
     * 用 document 捕獲階段攔截，不用 btn.addEventListener：那顆鈕上可能還掛著
     * 頁面 legacy 的 click，掛在按鈕上會變成兩個處理器都跑。
     */
    var btn = $('spbSave') || $('btSave') || $('btnSave') || $('btOK');
    if (btn) {
      document.addEventListener('click', function (ev) {
        var t = ev.target && ev.target.closest ? ev.target.closest('button,a,[id]') : null;
        if (t !== btn) return;
        // Steven 20260924（高級審查員第二輪，高）：引擎也在這頁時由引擎存檔。兩個 document 捕獲 listener
        // 會同時命中 spbSave（stopPropagation 擋不住同節點的另一個 listener）→ 一次按鈕同時送
        // recipe.doc.put（本檔，原字串）與 form.save（引擎，golden 鉗制值），後到的蓋掉先到的。
        if (window.HT9045Page) return;
        ev.stopPropagation(); ev.preventDefault(); save();
      }, true);
    }

    var kbn = attachKeyboards();
    load().then(function () {
      var b = $('ht9045WireBar');
      if (!b || !b.firstChild || b.firstChild.nodeType !== 3) { return; }
      var tail = '';
      if (kbn) { tail += String.fromCharCode(10) + '小鍵盤已掛上 ' + kbn + ' 個輸入框'; }
      tail += String.fromCharCode(10) +
        (btn ? ('存檔鈕：' + (btn.id || '(無 id)') + '（按它才會寫入配方）')
             : '⚠ 這一頁找不到存檔鈕，資料唯讀。');
      b.firstChild.nodeValue += tail;
    });
  }

  // 對外，方便在 Console 手動操作
  window.HT9045HotPlate = { load: load, save: save, collect: collect, KB: KB,
                           FIELD_MAP: FIELD_MAP, PENDING: PENDING };

  if (typeof HT9045Recipe === 'undefined') {
    say('❌ 沒有載入 ht9045_recipe_client.js —— 請在本檔之前先載入它。', '#f88');
    return;
  }
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', attach);
  } else {
    attach();
  }
})();

/* ---------------------------------------------------------------------------
 * 還沒接的 5 筆，以及為什麼
 * ---------------------------------------------------------------------------
 * 不是漏掉，是「不知道語意就不能接」。
 *
 *   Using Flag（cbEnableHP1 / cbEnableHP2）
 *     golden cHotPlate.cpp:482 int flag=0; :573-576
 *         if (cbEnableHP2->Checked) flag += 2;
 *         if (cbEnableHP1->Checked) flag += 1;
 *     所以它是位元欄位：HP1=bit0、HP2=bit1，兩個 checkbox 合成一個整數。
 *     語意已經查明，但一個鍵對兩個控件，load/save 都要特別處理，
 *     不能照 FIELD_MAP 的一對一形狀接。
 *
 *   Use Wide Hotplate（chkUseWideHotplate）
 *     布林 checkbox。而且只有 213/215 個配方有這個鍵 —— 缺的那兩個會讓
 *     preview 回 notFound，觸發規則 2 而整頁拒絕寫入。接之前要先決定
 *     「鍵不存在時要跳過還是要新增」，那是產品決定，不是實作決定。
 *
 *   bTrayHotplateCheck（chkTrayHotplateCheck）
 *     cHotPlate.cpp:203，區段是 [System] 不是 [Hotplate Form]，而且走
 *     CheckAndReadIniData。要先確認它是不是存在同一份 HotPlate.Data 裡。
 *
 *   cbSelectHPFromDB
 *     cHotPlate.cpp:627-629 只對它設 ItemIndex / Items / Text，
 *     沒有任何 ReadIniData / WriteIniData —— 它是 UI 控制項，不是配方欄位。
 *
 * 接法建議：一次接一個，接完在機台上讀一次、改一個值、存檔、再讀一次。
 * --------------------------------------------------------------------------- */
