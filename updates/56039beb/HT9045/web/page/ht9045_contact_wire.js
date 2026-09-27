/* ht9045_contact_wire.js -- Setup.Contact.html <-> Contact.Data 接線（參考實作）
 * ---------------------------------------------------------------------------
 * 這是「一頁怎麼接」的範本。其他頁面照同一個形狀做：
 *   1. 一張 FIELD_MAP（頁面 id -> 配方區段/鍵）
 *   2. load()   讀配方、填欄位
 *   3. save()   先 preview 給人看、再 write、寫完重讀驗證
 *
 * 用法（在 HTML 的 </body> 之前）：
 *   <script src="ht9045_recipe_client.js"></script>
 *   <script src="ht9045_contact_wire.js"></script>
 *
 * ---------------------------------------------------------------------------
 * FIELD_MAP 的來源不是猜的
 * ---------------------------------------------------------------------------
 * 是從 BCB6 golden 原始碼機械抽出來的：
 *   cContact.cpp 裡 `<成員> = ReadIniData(szDir, "<區段>", "<鍵>", 預設)`
 *   再接 `<widget>->Text = ...<成員>...`
 * 兩跳串起來得到 widget -> 區段/鍵。抽出 47 筆，逐一比對後
 * 頁面上 47 個 id 全部存在（同事是照 cContact.dfm 忠實建頁的）。
 *
 * 本檔只接其中 35 筆「純文字輸入框」。另外 12 筆需要型別感知，
 * 列在 PENDING 裡，接線前必須先由人確認語意 —— 見檔尾。
 *
 * ---------------------------------------------------------------------------
 * 三條規則，改這個檔時不要破壞
 * ---------------------------------------------------------------------------
 * 1. 寫入前一定 preview，把 changed 顯示給操作員看。
 * 2. notFound 不是空的就拒絕寫入 —— 那代表 FIELD_MAP 有錯，不是資料有錯。
 * 3. 寫完一定重讀回填。伺服器回報成功不等於你手上的物件是權威狀態。
 *
 * ⚠ 互鎖不要寫在這裡。該擋的由 C++ 端擋；兩層互鎖會互相遮蔽，
 *   JS 這層一旦被繞過或改壞，沒有人知道下面那層還在不在。
 *
 * ---------------------------------------------------------------------------
 * //Steven 20260921：collect() 多了一個擴充點 addCollector()
 * ---------------------------------------------------------------------------
 * 不是純文字欄位的鍵（捲軸 Position、radio ItemIndex…）由各自的接線檔算好
 * 再掛進來，仍然走同一條 preview -> 確認 -> 寫入 -> 重讀。細節見 collect()
 * 上面那段註解。目前的使用者是 ht9045_contact_slk.js。
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260921_Steven.md
 */
(function () {
  'use strict';

  var DOC = 'contact';

  /* 頁面 id -> [區段, 鍵]  (golden cContact.cpp 抽出，35 筆純文字欄位) */
  var FIELD_MAP = {
    // --- Test Arm1 ---------------------------------------------------------
    edPickUp1:                        ['Test Arm1', 'Pick Up'],
    edContactHeight1:                 ['Test Arm1', 'Contact'],
    edReleaseHeight1:                 ['Test Arm1', 'Place'],
    edContactBackUp1:                 ['Test Arm1', 'ContactBackUp'],
    edOrgPick1:                       ['Test Arm1', 'ShuttlePickBackUp'],
    edUpOffset1:                      ['Test Arm1', 'Up'],
    edDropOffset1:                    ['Test Arm1', 'Drop_DropContactMode'],
    edLoadCellHeight1:                ['Test Arm1', 'LoadCellZ1'],
    // --- Test Arm2 ---------------------------------------------------------
    edPickUp2:                        ['Test Arm2', 'Pick Up'],
    edContactHeight2:                 ['Test Arm2', 'Contact'],
    edReleaseHeight2:                 ['Test Arm2', 'Place'],
    edContactBackUp2:                 ['Test Arm2', 'ContactBackUp'],
    edOrgPick2:                       ['Test Arm2', 'ShuttlePickBackUp'],
    edUpOffset2:                      ['Test Arm2', 'Up'],
    edDropOffset2:                    ['Test Arm2', 'Drop_DropContactMode'],
    edLoadCellHeight2:                ['Test Arm2', 'LoadCellZ2'],
    // --- Wait Time ---------------------------------------------------------
    edDropWaitTime:                   ['Wait Time', 'Drop Wait'],
    edDropSpeed:                      ['Wait Time', 'Drop Speed'],
    edUpWaitTime:                     ['Wait Time', 'Up Wait'],
    edUpSpeed:                        ['Wait Time', 'Up Speed'],
    edSidePushWaitTime:               ['Wait Time', 'Side Push Wait Time'],
    // --- Torque Control ----------------------------------------------------
    edAirForce:                       ['Torque Control', 'Torque'],
    edPinCount:                       ['Torque Control', 'Pin Number'],
    edXDimension:                     ['Torque Control', 'X Dimension'],
    edYDimension:                     ['Torque Control', 'Y Dimension'],
    edForcePerPinN:                   ['Torque Control', 'Force Per Pin'],
    edForcePerPinG:                   ['Torque Control', 'Force Per Pin Kg'],
    edDieForcePerPinN:                ['Torque Control', 'Die Force Per Pin'],
    edDieForcePerPinG:                ['Torque Control', 'Die Force Per Pin Kg'],
    edDoubleForce:                    ['Torque Control', 'Double Force'],
    edtPinOfDie:                      ['Torque Control', 'Pin of Die'],
    // --- Mode（文字欄位那幾個）---------------------------------------------
    edAutoKSHTReleaseOfs:             ['Mode', 'AutoKSHTOfs'],
    edtPurgeBeforePickShuttleTime:    ['Mode', 'Air Purge Before Pick Shuttle Time'],
    edtPurgeBeforePickShuttleInterval:['Mode', 'Air Purge Before Pick Shuttle Interval'],
    edtPurgeBdforePickShuttleOffSet:  ['Mode', 'Air Purge Before Pick Shuttle OffSet']
    // 註：最後一個 id 的 "Bdfore" 是頁面與 golden 都有的既存拼字，不要「順手修正」，
    //     改了就對不上 golden 的 widget 名。
  };

  /* 抽到了但還不能接的 12 筆 —— 需要人先確認語意，見檔尾說明 */
  var PENDING = {
    cbContactMode:     ['Mode', 'Contact',                          'select'],
    cbVacuumMode:      ['Mode', 'Vacuum',                           'select'],
    coD41:             ['Mode', 'iSocketInitialICCheckPosition',    'select'],
    rgKitDiameter:     ['Mode', 'Kit Diameter',                     'radio-group'],
    rgTesterSidePush:  ['Mode', 'Tester Side Push',                 'radio-group'],
    rgSidePushMode:    ['Mode', 'Tester Side Push Mode',            'radio-group'],
    rbNormal:          ['Mode', 'Dummy Contact',                    'radio (pair with rbDummyMode)'],
    rbDummyMode:       ['Mode', 'Dummy Contact',                    'radio (pair with rbNormal)'],
    cbTestContactMode: ['?',    '?',   'UNVERIFIED - 抽取時誤配，語意待確認'],
    edD41:             ['?',    '?',   'UNVERIFIED - 抽取時誤配，語意待確認'],
    edSpeed:           ['?',    '?',   'UNVERIFIED - 抽取時誤配，語意待確認'],
    edSpeedZ:          ['?',    '?',   'UNVERIFIED - 抽取時誤配，語意待確認']
  };

  /* ---------------------------------------------------------------------
   * 小鍵盤：旗標與夾限抽自 golden cContact.cpp 的 ShowQwertyKey 呼叫，
   * 配上 cContact.dfm 的 OnMouseDown = <handler>。
   *
   * ⚠ cContact.cpp 有 15 個 ShowQwertyKey 呼叫，其中多數的 min/max 是 C++
   *   執行期變數（InputLimit.dContactHigh 之類），JS 這邊拿不到值。那些一律
   *   關掉夾限（checkRange=false）而不是填一個猜的數字 —— 猜錯的夾限會把
   *   合法輸入擋掉或放過非法輸入，比沒有夾限更危險。
   *   46 個 OnMouseDown 中抽到 42 個，其中只有 5 個有硬編的上下限。
   */
  var KB = {
    edAutoKSHTReleaseOfs:              ['DOUBLE', 2, true, 10, 2],
    edContactHeight1:                  ['DOUBLE', 3, false, 0, 0],
    edContactHeight2:                  ['DOUBLE', 3, false, 0, 0],
    edContactOffsetArm1:               ['DOUBLE', 2, false, 0, 0],
    edContactOffsetArm2:               ['DOUBLE', 2, false, 0, 0],
    edContactRelativeZ1:               ['DOUBLE', 3, false, 0, 0],
    edContactRelativeZ2:               ['DOUBLE', 3, false, 0, 0],
    edD41:                             ['DOUBLE', 2, true, 5.00, 0.00],
    edDieForcePerPinG:                 ['DOUBLE', 4, false, 0, 0],
    edDieForcePerPinN:                 ['DOUBLE', 4, false, 0, 0],
    edDoubleForce:                     ['DOUBLE', 2, false, 0, 0],
    edDropByPassDetect:                ['DOUBLE', 2, true, 20.0, 0.00],
    edDropOffset1:                     ['DOUBLE', 3, false, 0, 0],
    edDropOffset2:                     ['DOUBLE', 3, false, 0, 0],
    edDropSpeed:                       ['DOUBLE', 3, false, 0, 0],
    edDropWaitTime:                    ['DOUBLE', 3, false, 0, 0],
    edForcePerDeviceKG:                ['DOUBLE', 2, true, 1.0, 120.0],
    edForcePerPinG:                    ['DOUBLE', 4, false, 0, 0],
    edForcePerPinN:                    ['DOUBLE', 4, false, 0, 0],
    edLoadCellHeight1:                 ['DOUBLE', 3, false, 0, 0],
    edLoadCellHeight2:                 ['DOUBLE', 3, false, 0, 0],
    edOrgPick1:                        ['DOUBLE', 3, false, 0, 0],
    edOrgPick2:                        ['DOUBLE', 3, false, 0, 0],
    edPickUp1:                         ['DOUBLE', 3, false, 0, 0],
    edPickUp2:                         ['DOUBLE', 3, false, 0, 0],
    edReleaseHeight1:                  ['DOUBLE', 3, false, 0, 0],
    edReleaseHeight2:                  ['DOUBLE', 3, false, 0, 0],
    edShtPickOffset1:                  ['DOUBLE', 2, false, 0, 0],
    edShtPickOffset2:                  ['DOUBLE', 2, false, 0, 0],
    edSidePushWaitTime:                ['DOUBLE', 3, false, 0, 0],
    edUpOffset1:                       ['DOUBLE', 3, false, 0, 0],
    edUpOffset2:                       ['DOUBLE', 3, false, 0, 0],
    edUpSpeed:                         ['DOUBLE', 3, false, 0, 0],
    edUpWaitTime:                      ['DOUBLE', 3, false, 0, 0],
    edXDimension:                      ['DOUBLE', 3, false, 0, 0],
    edYDimension:                      ['DOUBLE', 3, false, 0, 0],
    edtPinOfDie:                       ['INTEGER', 0, true, 10, 10000],
    edtPurgeBdforePickShuttleOffSet:   ['DOUBLE', 3, false, 0, 0],
    edtPurgeBeforePickShuttleInterval: ['DOUBLE', 3, false, 0, 0],
    edtPurgeBeforePickShuttleTime:     ['DOUBLE', 3, false, 0, 0],
    edtTorqueCmp:                      ['DOUBLE', 3, false, 0, 0],
    edtTorqueMax:                      ['DOUBLE', 3, false, 0, 0],
  };

  function attachKeyboards() {
    if (typeof HTQwerty === 'undefined') {
      say('⚠ 小鍵盤未載入：請在本檔之前加入 <script src="qwerty.js"></script>', '#ffcc66');
      return 0;
    }
    var n = 0, unmapped = [];
    var inputs = document.querySelectorAll('input[type="text"], input:not([type])');
    Array.prototype.forEach.call(inputs, function (el) {
      if (!el.id || el.disabled) return;
      var c = KB[el.id];
      var flags = c ? (HTQwerty.N[c[0]] || 0) : 0;
      var opt = c ? { dp: c[1], checkRange: !!c[2], min: c[3], max: c[4] } : {};
      if (!c) unmapped.push(el.id);
      el.addEventListener('mousedown', function (ev) {
        ev.preventDefault();
        HTQwerty.show(el, flags, opt);
      });
      el.setAttribute('readonly', 'readonly');
      el.style.cursor = 'pointer';
      n++;
    });
    if (unmapped.length) {
      console.warn('[contact-wire] 這些輸入框沒有 golden 旗標依據，用預設全 QWERTY：', unmapped);
    }
    return n;
  }

  // ---------------------------------------------------------------- helpers
  function $(id) { return document.getElementById(id); }

  function bar() {
    var b = $('ht9045WireBar');
    if (b) return b;
    b = document.createElement('div');
    b.id = 'ht9045WireBar';
    b.style.cssText = 'position:fixed;left:614px;right:8px;top:8px;z-index:99999;' +
      'font:12px/1.5 monospace;padding:6px 10px;background:#222;color:#ddd;' +
      'border:1px solid #555;border-radius:4px;max-height:60vh;overflow:auto;white-space:pre-wrap;min-width:260px';
    document.body.appendChild(b);
    return b;
  }
  /* Steven 20260921：狀態列顯示政策，和 ht9045_wire_engine.js 那一份同一套
   * （那邊的註解是權威版本，改這裡記得兩邊一起改）：
   *   startup   開頁的接線自我報告 -> 只在 debug 顯示，AUTOHIDE_MS 後自動收起
   *   transient 動作成功的回覆     -> 兩種模式都顯示，自動收起
   *   sticky    ❌ / ⚠ / 進行中     -> 預設，兩種模式都顯示，不自動消失
   *
   * ⚠ 兩件容易錯的事：
   *   1. 倒數計時器掛在元素上（b.__htHideTimer）。#ht9045WireBar 這個 id
   *      本檔和引擎都會建，Setup.Contact.html 兩支都載入；計時器各放各的
   *      模組變數，其中一支的倒數會把另一支稍晚貼的訊息一起收掉。
   *   2. say() 一定要把 display 設回來。原本這裡沒設，只改 textContent ——
   *      引擎那邊把 bar 收起來之後，本檔後續的 ❌ 訊息會寫進一個
   *      display:none 的元素，操作員什麼都看不到。
   */
  var AUTOHIDE_MS = 6000;

  function isDebug() {
    try {
      return document.documentElement.getAttribute('data-mode') === 'debug';
    } catch (e) { return false; }
  }

  function say(msg, colour, kind) {
    kind = kind || 'sticky';
    if (kind === 'startup' && !isDebug()) return msg;
    var b = bar();
    b.style.display = '';
    b.style.color = colour || '#ddd';
    b.textContent = msg;
    if (b.__htHideTimer) { clearTimeout(b.__htHideTimer); b.__htHideTimer = null; }
    if (kind === 'startup' || kind === 'transient') {
      b.__htHideTimer = setTimeout(function () {
        b.__htHideTimer = null;
        b.style.display = 'none';
      }, AUTOHIDE_MS);
    }
    return msg;
  }

  // ------------------------------------------------------------------ load
  function load() {
    say('讀取中 ...', null, 'startup');
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
      lines.push('尚未接線（需人工確認語意）：' + Object.keys(PENDING).length + ' 個，見 ht9045_contact_wire.js 檔尾');
      // Steven 20260921：開頁的接線自我報告 -> startup（只在 debug 顯示並自動收起）
      say(lines.join('\n'), missingEl.length || missingKey.length ? '#ffcc66' : '#9f9',
          'startup');
      // Steven 20260924 (S12)：讀完檔再跑一次引擎的 formOverlay()（/api/form 的值）。
      // Setup.Contact.html 沒有 /api/form 的第二型 bridge（第一型 AI(W906-Q4-S126) 20260927 退役），回 404，這行等同無作用。
      if (window.HT9045Page && window.HT9045Page.formOverlay) window.HT9045Page.formOverlay();
      return doc;
    }).catch(function (e) {
      say('讀取失敗：' + e.message + '\n（伺服器有起來嗎？F5 之後應該看到 http://127.0.0.1:8045/）', '#f88');
      throw e;
    });
  }

  // ------------------------------------------------------------------ save
  /* Steven 20260921：collect() 的擴充點。
   * ---------------------------------------------------------------------
   * FIELD_MAP 只認「頁面 id -> 一個純文字欄位」。有些鍵不是那個形狀 ——
   * 值在別的元件上（捲軸的 Position、radio 的 ItemIndex），而且存檔前還要
   * 依 golden 換算（例如 Kit Diameter 存的是公分，"40x2" 存 40.2）。
   * 那種鍵掛在這裡，由自己的接線檔負責算好再交出來。
   *
   * 註冊函式回傳 {區段: {鍵: 字串}}；save() 會併進 edits，所以三條規矩
   * （preview -> 人確認 -> 寫完重讀）照樣適用，不會有一條繞過檢查的側門。
   *
   * ⚠ 回傳的值一定要是字串，而且格式要跟 golden 的 WriteIniData 一樣
   *   （double 是 "%0.4f"，int 就是整數）—— 格式不同會讓 preview 每次都
   *   報 changed，操作員每次存檔都看到一堆「其實沒變」的差異。
   *
   * 目前的使用者：ht9045_contact_slk.js（[Mode] Head Device Mode /
   *               Kit Diameter / KitDiameterMode）。
   */
  var EXTRA_COLLECTORS = [];

  function collect() {
    var edits = {};
    Object.keys(FIELD_MAP).forEach(function (id) {
      var sec = FIELD_MAP[id][0], key = FIELD_MAP[id][1], el = $(id);
      if (!el) return;
      (edits[sec] = edits[sec] || {})[key] = String(el.value);
    });
    EXTRA_COLLECTORS.forEach(function (c) {
      var more;
      try { more = c.fn(); } catch (e) {
        // 擴充點壞掉不可以讓整個存檔靜默少幾個鍵 —— 講出來，然後照常存其餘的
        say('⚠ 存檔擴充點「' + c.name + '」丟出例外，它負責的鍵這次沒有寫入：'
            + e.message, '#ffcc66');
        return;
      }
      if (!more) return;
      Object.keys(more).forEach(function (sec) {
        Object.keys(more[sec]).forEach(function (key) {
          (edits[sec] = edits[sec] || {})[key] = String(more[sec][key]);
        });
      });
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
      if (!changed.length) { say('沒有任何值改變，不寫入。', '#9f9', 'transient'); return null; }

      // 規則 1：把「會改什麼」攤開給操作員看，由人按下確認
      var detail = changed.map(function (c) { return '  ' + JSON.stringify(c); }).join('\n');
      if (!window.confirm('即將寫入 ' + changed.length + ' 個值到配方 ' + DOC + '：\n\n' +
                          detail + '\n\n確定要寫入嗎？')) {
        say('已取消，未寫入。', '#ffcc66', 'transient');
        return null;
      }
      say('寫入中 ...');
      return HT9045Recipe.write(DOC, edits).then(function (w) {
        // 規則 3：重讀回填，不要拿送出去的物件當結果
        return load().then(function () {
          say('✅ 寫入完成並已重讀驗證：changed=' + (w.changed || []).length +
              '  identical=' + (w.identical || []).length, '#9f9', 'transient');
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
     * 存檔鈕改接 spbSave —— cContact.dfm 自己的 Save 鈕。舊的猜測清單
     * (btSave/btnSave/btOK/btApply) 沒有它，所以這一頁每次都掉進退路，
     * 才會多出那兩顆浮動鈕。找不到任何存檔鈕時不再自己生一顆，改在狀態列
     * 講清楚這一頁唯讀 —— 沉默地少一顆按鈕，跟沉默地多一顆假按鈕一樣糟。
     *
     * 用 document 捕獲階段攔截，不用 btn.addEventListener：那顆鈕上可能還掛著
     * 頁面 legacy 的 click，掛在按鈕上會變成兩個處理器都跑。
     *
     * 重讀沒有對應的頁面按鈕，所以不再提供；Console 仍可用 HT9045Contact.load()。
     */
    var btn = $('spbSave') || $('btSave') || $('btnSave') || $('btOK') || $('btApply');
    if (btn) {
      document.addEventListener('click', function (ev) {
        var t = ev.target && ev.target.closest ? ev.target.closest('button,a,[id]') : null;
        if (t !== btn) return;
        // Steven 20260924（高級審查員第二輪）：引擎也在這頁時由引擎存檔，本檔不再同時存一次
        // （兩個 document 捕獲 listener 會同時命中，stopPropagation 擋不住同節點的另一個）。
        if (window.HT9045Page) return;
        ev.stopPropagation(); ev.preventDefault(); save();
      }, true);
    }

    var kbn = attachKeyboards();
    load().then(function () {
      var b = $('ht9045WireBar');
      if (!b) return;
      var tail = '';
      if (kbn) { tail += String.fromCharCode(10) + '小鍵盤已掛上 ' + kbn + ' 個輸入框'; }
      tail += String.fromCharCode(10) +
        (btn ? ('存檔鈕：' + (btn.id || '(無 id)') + '（按它才會寫入配方）')
             : '⚠ 這一頁找不到存檔鈕，資料唯讀。');
      b.textContent += tail;
    });
  }

  // 對外，方便在 Console 手動操作
  window.HT9045Contact = { load: load, save: save, collect: collect,
                           FIELD_MAP: FIELD_MAP, PENDING: PENDING,
                           /* 見上面 EXTRA_COLLECTORS 的說明。name 只用在錯誤訊息，
                              但不要省 —— 少了它，擴充點出問題時看不出是哪一支。 */
                           addCollector: function (name, fn) {
                             EXTRA_COLLECTORS.push({ name: name, fn: fn });
                             return EXTRA_COLLECTORS.length;
                           } };

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
 * 還沒接的 12 筆，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些不是漏掉，是「不知道語意就不能接」。配方檔存的是數字，頁面上是選項，
 * 中間的對應關係（哪個數字代表哪個選項）在 golden 的 ItemIndex 指派裡，
 * 需要逐個確認，猜錯會把錯的模式寫進機台配方。
 *
 *   select (3)        cbContactMode / cbVacuumMode / coD41
 *                     -> 需要「option 的 value 順序」對上配方裡的整數
 *
 *   radio group (3)   rgKitDiameter / rgTesterSidePush / rgSidePushMode
 *                     -> rgKitDiameter 特別要小心：頁面上的選項清單會依機種
 *                        改變（fContact.cpp:191-203 與 :344-360 各自 Add 一組
 *                        不同的字串），所以 ItemIndex 與實際口徑的對應不是固定的
 *
 *   radio pair (2)    rbNormal / rbDummyMode -> 同一個鍵 'Dummy Contact' 的兩極
 *
 *   未證實 (4)        cbTestContactMode / edD41 / edSpeed / edSpeedZ
 *                     -> 機械抽取時被誤配到 'Height Calibration / Test Contact
 *                        Count'，四個 id 指到同一個鍵，明顯錯誤。要回 golden
 *                        cContact.cpp 逐個確認它們真正對應什麼，才可以接。
 *
 * 接法建議：一次接一個，接完在機台上讀一次、改一個值、存檔、再讀一次，
 * 確認往返都對，再接下一個。不要一次接十二個然後一起測。
 * --------------------------------------------------------------------------- */
