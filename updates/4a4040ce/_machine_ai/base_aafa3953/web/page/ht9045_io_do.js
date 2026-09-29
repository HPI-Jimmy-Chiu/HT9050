/* ht9045_io_do.js -- HW.IoSetView 的輸出按鈕接到 PCIE-1203
 * ---------------------------------------------------------------------------
 * AI(W906-IOWEB-P17) 20260925：新檔。使用者 20260925「輸出請幫我接上」「請幫我接上1203」。
 *
 * 這一頁原本的按鈕處理只做 classList.toggle('down')（HW.IoSetView.html:110-112）：
 * 按鈕變綠、看起來像出力了，其實什麼都沒送。這個檔接手所有「帶 Alias 的
 * .btnpanel」的點擊，改成送一個指令給 wb_serve：
 *
 *     {cmd:'io.btnPanelClick', tag:<Alias>, value:<按下之後的 Down，0|1>}
 *
 * C++ 端是 golden 的 BtnPanelClick（JsonBridge/IoBtnPanelClick.cpp）→ MyLaneIO.IOBitOn／IOBitOff
 * → 1203 路由 → Pci1203Control（DRY RUN 或 LIVE 由建置決定，MachineType.h）。
 *
 * 規則（每一條都是有代價換來的，改之前先看理由）：
 *   * Down 的來源是**卡片的 DO 回讀**（/api/struct/io/runtime），不是按鈕自己記的狀態。
 *     按鈕的底色（.down）每次輪詢依回讀重畫；按下去不會先自己變色 —— DRY RUN、被拒、
 *     逾時的時候，自己變色就是在說謊。
 *   * 讀不到狀態（quality 不是 good）就不送：不知道現在是 ON 還是 OFF，就不知道要切成什麼。
 *   * 資料必須來自 wb_serve 的 1203 監看器（runtime.provider），offline／快取的 JSON 一律不送。
 *   * 寫不了的按鈕（不在 IO 表、Enable=0、不是 1203、不是輸出）畫成灰色 —— golden 同樣把
 *     沒綁上的按鈕畫成 clSilver（iosetview.cpp:2255-2258）。
 *   * 回應一定顯示出來，而且分清楚「DRY RUN（沒有真的出力）」「已送出」「卡片回錯誤」「被拒」。
 *
 * 放在獨立檔、只在 HTML 加一行 <script>，理由與 ht9045_opbuttons.js 相同：HW.IoSetView.html
 * 是同事的檔，產生器（gen_wire.py）只寫 ht9045_wire_*.js —— 這個名字刻意不落在那個樣式上。
 * ------------------------------------------------------------------------- */
(function () {
  'use strict';
  if (window.__ht9045IoDo) return;
  window.__ht9045IoDo = true;

  var PROVIDER = 'wb_serve:pci1203-monitor';
  var last = null;        // 最近一次 ioBindStatus 的資料（config＋runtime 合併後）
  var byAlias = {};       // alias -> point
  var busy = {};          // alias -> true：這顆的指令還在路上

  // ------------------------------------------------------------------ style
  (function () {
    var st = document.createElement('style');
    st.id = 'ioDoStyle';
    st.textContent =
      '.btnpanel.io-na{opacity:.45;filter:grayscale(.7);cursor:not-allowed;}' +
      '.btnpanel.io-busy{outline:2px solid #ffb000;outline-offset:1px;}' +
      '#ioDoMsg{position:fixed;left:8px;bottom:30px;max-width:calc(100% - 16px);z-index:100001;' +
      'padding:6px 10px;border:1px solid #c9a000;border-radius:4px;background:#fffbe6;color:#222;' +
      'font:12px/1.45 "Microsoft JhengHei","MS Sans Serif",sans-serif;white-space:pre-wrap;display:none;' +
      'box-shadow:0 2px 6px rgba(0,0,0,.18);}' +
      '#ioDoMsg.ok{border-color:#228b22;background:#effaef;}' +
      '#ioDoMsg.dry{border-color:#c98a00;background:#fff4dc;}' +
      '#ioDoMsg.err{border-color:#b22222;background:#fdecec;}';
    document.head.appendChild(st);
  })();

  var msgTimer = null;
  function say(text, level, ms) {
    var el = document.getElementById('ioDoMsg');
    if (!el) {
      el = document.createElement('div');
      el.id = 'ioDoMsg';
      el.title = '點一下關閉';
      el.addEventListener('click', function () { el.style.display = 'none'; });
      document.body.appendChild(el);
    }
    el.className = level || '';
    el.textContent = text;
    el.style.display = 'block';
    if (msgTimer) clearTimeout(msgTimer);
    msgTimer = setTimeout(function () { el.style.display = 'none'; }, ms || 9000);
  }

  // ------------------------------------------------------------------ data
  function aliasOf(el) {
    // ioPaintElement 會在 title 後面接「｜IO=…｜state=…」，原始 title 存在 data-io-title；
    // theme.js 的 release 模式會把 title 搬到 data-htitle。三個都認。
    var t = el.getAttribute('data-io-title') || el.getAttribute('title') || el.getAttribute('data-htitle') || '';
    var m = t.match(/Alias=([^｜|\s]+)/);
    return m ? m[1].trim() : '';
  }

  function fromCpp(data) {
    var rt = data && data.runtime;
    return !!(rt && rt.provider === PROVIDER && rt.connected === true);
  }

  function indexByAlias(data) {
    var out = {};
    var pts = (data && data.points) || [];
    for (var i = 0; i < pts.length; i++) {
      var p = pts[i];
      if (p && p.alias && !out[p.alias]) out[p.alias] = p;   // 第一筆，與 C++ 的 mapIOTable 同一個原則
    }
    return out;
  }

  function whyNotWritable(pt) {
    if (!pt) return '不在 IO 表';
    if (pt.direction !== 'output') return '不是輸出（' + (pt.ioType || '?') + '）';
    if (!pt.hw || pt.hw.isaBase !== 3) return '不是 1203 的點（ISABase=' + (pt.hw ? pt.hw.isaBase : '?') + '）';
    if (pt.hw.enable !== 1) return 'IO 表 Enable=0';
    return '';
  }

  // ------------------------------------------------------------------ paint
  function paint() {
    var live = fromCpp(last);
    var nodes = document.querySelectorAll('.btnpanel');
    for (var i = 0; i < nodes.length; i++) {
      var el = nodes[i];
      var a = aliasOf(el);
      if (!a) continue;
      var pt = byAlias[a];
      var na = !live || whyNotWritable(pt) !== '';
      if (na) { el.classList.add('io-na'); el.classList.remove('down'); continue; }
      el.classList.remove('io-na');
      var st = pt.status || {};
      // golden：Down = OutType ? 回讀 : !回讀（iosetview.cpp:2247-2250）。runtime 的 isOn 已經
      // 依 InType 換算過（ChanIoPoints.cpp），所以 state==='on' 就是 golden 的 Down。
      if (st.state === 'on') el.classList.add('down');
      else el.classList.remove('down');   // off, or unknown/bad: a coil we cannot read must not look energised (review WEB-5)
    }
  }

  var origBind = window.ioBindStatus;
  if (typeof origBind !== 'function') {
    say('IO 輸出：找不到 ioBindStatus（頁面結構變了），輸出按鈕沒有接上', 'err', 20000);
    return;
  }
  window.ioBindStatus = function (data) {
    var r = origBind.apply(this, arguments);
    try { last = data; byAlias = indexByAlias(data); paint(); } catch (e) { /* 畫不出來不影響原本的上色 */ }
    return r;
  };

  // ------------------------------------------------------------------ click
  function describe(a, ack) {
    var ad = ack.address || {};
    var where = 'ring ' + ad.ring + '・站 ' + ad.station + '・通道 ' + ad.port +
                '（IO 表 Lane ' + ad.lane + ' / IP ' + ad.ip + ' / Port ' + ad.port + '）';
    var what = a + ' → ' + (ack.raw ? 'ON' : 'OFF') + '（按鈕 Down=' + ack.down + '）';
    var side = (ack.sideEffects && ack.sideEffects.length) ? '\n附帶：' + ack.sideEffects.join('；') : '';
    if (ack.dryRun && !ack.issued)
      return { level: 'dry', text: 'DRY RUN —— 沒有真的出力\n' + what + '\n' + where +
               '\nLIVE 時會送：' + ack.exCall + side };
    if (ack.issued && ack.vendorOk)
      return { level: 'ok', text: '已送到 1203\n' + what + '\n' + where + '\n' + ack.exCall + side };
    if (ack.issued)
      return { level: 'err', text: '已送出，但卡片回錯誤 0x' + (ack.ret >>> 0).toString(16).toUpperCase() +
               '\n' + what + '\n' + where + (ack.why ? '\n' + ack.why : '') + side };
    return { level: 'err', text: '沒有送出\n' + what + (ack.why ? '\n' + ack.why : '') + side };
  }

  // AI(W906-IOWEB-P21) 20260925：計時（使用者：「你寫個LOG 給我測測看不就知道了?」）。
  // 網頁量「按下 → 收到回應」的總時間；伺服器在回應裡附上它那一段的拆解（ack.timing）。
  function now() { return (window.performance && performance.now) ? performance.now() : Date.now(); }
  // AI(W906-LAT-1) 20260925：權杖和按鈕現在同時送出（見 send()），所以「權杖回覆」是跟按鈕重疊的那一段，
  // 不再是加在前面的一趟來回；伺服器端多一欄「發布」（PublishHandlerTags，C++ 同一版加的計時）。
  function timingText(ack, t0, tAcq) {
    var total = Math.round(now() - t0);
    var s = '\n計時：按下到回應共 ' + total + ' ms（權杖回覆 ' + Math.round(tAcq - t0) + ' ms，與按鈕同時送出）';
    var t = ack && ack.timing;
    if (t) {
      s += '\n  伺服器端：排隊 ' + Math.round(t.queueMs) + ' ms' +
           '（跑流程 ' + Math.round(t.pumpMs) + '、讀卡 ' + Math.round(t.pollMs) +
           '、畫面資料 ' + Math.round(t.cacheMs) +
           (typeof t.pubMs === 'number' ? '、發布 ' + Math.round(t.pubMs) : '') +
           '、其他 ' + Math.round(t.otherMs) + '）' +
           '，送卡 ' + (Math.round(t.execMs * 10) / 10) + ' ms';
    }
    try { console.log('[io.btnPanelClick timing]', ack && ack.alias, 'total', total, 'ms', t); } catch (e) {}
    return s;
  }

  function send(el, a, down) {
    var R = window.HT9045Recipe;
    if (!R || typeof R.rawCmd !== 'function') { say('IO 輸出：通訊元件（ht9045_recipe_client.js）沒有載入', 'err'); return; }
    var t0 = now(), tAcq = t0;
    busy[a] = true;
    el.classList.add('io-busy');
    // 每次都重新拿權杖：伺服器閒置 10 分鐘會收回，而 client 端的 haveToken 不會跟著變
    // （ht9045_recipe_client.js:150-152 vs WebBridgeServer.cpp:1566-1568），所以不靠它。
    // AI(W906-LAT-1) 20260925：acquire 與 btnPanelClick 連著送出，中間不等 acquire 的回覆 —— 少一趟來回
    //   （輸出延遲調查第 3 步）。可以這樣做的理由：兩個訊框走同一條 WebSocket，送出順序就是 rawCmd 的呼叫順序
    //   （ht9045_recipe_client.js cmd()：同一個 connect() 之後依序 send），伺服器照到達順序處理，而
    //   control.acquire 在 socket 執行緒上當場回答（WebBridgeServer.cpp 的 control.acquire 分支）——
    //   輪到按鈕做權杖檢查時，權杖已經是這條連線的了。
    //   ⚠ 權杖在別人手上時：acquire 回 control-held，按鈕接著回 not-operator（伺服器擋下，不會進 tick、不會寫卡）。
    //     畫面上要顯示的仍然是「操作權杖在別的畫面手上…」那句，所以按鈕失敗時先等 acquire 的結果再決定訊息。
    //   ⚠ acq 這個 promise 永遠不 reject（錯誤當成值傳下去），否則權杖被佔時會多一個沒人接的 rejection。
    var acq = R.rawCmd('control.acquire').then(function () { tAcq = now(); return null; },
      function (e) { tAcq = now(); return e || new Error('control.acquire failed'); });
    R.rawCmd('io.btnPanelClick', { tag: a, value: down }).then(function (ack) {
      return acq.then(function () { return ack; });   // 等 acquire 也有結果，計時裡的「權杖回覆」才有數字
    }, function (e) {
      return acq.then(function (acqErr) {
        if (!acqErr) throw e;                          // 權杖拿到了：按鈕自己的拒絕理由原樣顯示
        var m = String(acqErr && acqErr.message || acqErr);
        // AI(W906-MT-FIX1) 20260925：只有伺服器「真的拒絕」權杖時才把原因說成權杖。連線斷掉（socket closed／逾時）時兩個
        //   pending 會一起被拒 —— 那時按鈕的訊框可能早已送出、伺服器也可能已經執行（線圈可能已經切了），不能說成「沒有送出」。
        if (/socket closed|no ack within/i.test(m)) throw e;
        throw new Error(m.indexOf('control-held') >= 0
          ? '操作權杖在別的畫面手上（例如開著的 pci1203 監看頁或別的設定頁）——那邊關掉或閒置 10 分鐘後再試'
          : '拿不到操作權杖：' + m);
      });
    }).then(function (ack) {
      var d = describe(a, ack);
      say(d.text + timingText(ack, t0, tAcq), d.level, d.level === 'ok' ? 9000 : 12000);
    }, function (e) {
      var m = String(e && e.message || e);
      if (/no ack within/.test(m)) m = '沒有回應（逾時）——不代表沒有執行，看一下燈號與 wb_serve 主控台';
      else if (/socket closed/i.test(m)) {             // AI(W906-MT-FIX1) 20260925
        say('「' + a + '」連線中斷，不確定有沒有送出——看燈號\n' + m + '\n計時：按下到回應共 ' + Math.round(now() - t0) + ' ms', 'err', 12000);
        return;
      }
      say('「' + a + '」沒有送出\n' + m + '\n計時：按下到回應共 ' + Math.round(now() - t0) + ' ms', 'err', 12000);
    }).then(function () {
      busy[a] = false;
      el.classList.remove('io-busy');
      scheduleRelease();
    });
  }

  // 權杖用完就還（review WEB-4）：IO 視窗的 iframe 關掉只是隱藏，連線一直在；
  // 一直握著的話，別頁按「存檔」會被 control-held 擋 10 分鐘。
  // ⚠ 只還「我們自己拿的」：recipe client 自己 acquire() 過（status().holdsToken）就不動，
  //   否則它以為自己還握著，下一次存檔會直接收到 not-operator。
  var releaseTimer = null;
  function scheduleRelease() {
    if (releaseTimer) clearTimeout(releaseTimer);
    releaseTimer = setTimeout(function () {
      releaseTimer = null;
      var R = window.HT9045Recipe;
      if (!R || typeof R.rawCmd !== 'function') return;
      for (var k in busy) if (busy.hasOwnProperty(k) && busy[k]) { scheduleRelease(); return; }
      var s = (typeof R.status === 'function') ? R.status() : null;
      if (s && s.holdsToken) return;
      R.rawCmd('control.release').then(null, function () { /* 已經不是我們的就算了 */ });
    }, 30000);
  }

  function onClick(el, a) {
    if (busy[a]) return;
    if (!last || !fromCpp(last)) {
      say('IO 輸出：資料不是從 wb_serve 的 1203 監看器來的（還在載入、offline 模式、或用 file:// 開的），不送', 'err');
      return;
    }
    var pt = byAlias[a];
    var why = whyNotWritable(pt);
    if (why) { say('「' + a + '」不能輸出：' + why, 'err'); return; }
    var st = pt.status || {};
    if (st.quality !== 'good' || (st.state !== 'on' && st.state !== 'off')) {
      say('「' + a + '」讀不到卡片上目前的狀態（' + (st.quality || 'unknown') + '），不知道要切成什麼，所以不送', 'err');
      return;
    }
    // golden：Ptr->Down = !Ptr->Down（iosetview.cpp:1087），Down 就是畫面上的底色
    send(el, a, st.state === 'on' ? 0 : 1);
  }

  // capture 階段、掛在 document：比頁面自己掛在每顆按鈕上的 toggle 先執行，並攔下它
  // （與 ht9045_wire_engine.js:1613-1620 同一個作法）。沒有 Alias 的 .btnpanel 不碰。
  document.addEventListener('click', function (e) {
    // 版面拖曳／放置模式（debug 選單，theme.js）：那時的點擊是在搬元件，不是在按按鈕。
    // theme.js 的 capture 監聽先註冊、會 preventDefault，但 stopPropagation 擋不住同一個節點上
    // 的其他監聽 —— 所以這裡要自己看（review WEB-1：拖一下 SwMotorRelay 就會切馬達電源）。
    if (e.defaultPrevented ||
        document.body.classList.contains('layoutEdit') ||
        document.body.classList.contains('layoutPlace')) return;
    var el = e.target && e.target.closest ? e.target.closest('.btnpanel') : null;
    if (!el) return;
    var a = aliasOf(el);
    if (!a) return;
    e.stopPropagation();
    e.preventDefault();
    if (el.classList.contains('io-na')) {
      var why = last && fromCpp(last) ? whyNotWritable(byAlias[a]) : 'IO 資料還沒從 wb_serve 載入';
      say('「' + a + '」不能輸出：' + (why || '目前不可用'), 'err');
      return;
    }
    onClick(el, a);
  }, true);
})();
