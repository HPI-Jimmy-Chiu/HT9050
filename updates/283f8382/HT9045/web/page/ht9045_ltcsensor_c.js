/* ht9045_ltcsensor_c.js -- Status.LtcSensor.html（golden TfLtcSensor，LtcSensor.cpp／LtcSensor.dfm V906）頁面補件。
 * ---------------------------------------------------------------------------
 * AI(W906-LTC-C) 20261001: 新檔（每個元件逐一檢查，compK 群組 ioS）。手寫，檔名刻意不叫 ht9045_wire_<slug>.js（產生器不覆蓋）。
 *   這一頁原本只有 theme.js／hwidgets.js：沒有任何元件接到 C++，按鈕按了沒反應、燈永遠不亮、群組照 .dfm 全部顯示。
 *
 * golden 怎麼做（LtcSensor.cpp）、這裡怎麼做：
 *   FormShow :77-103
 *     gbOutSh1Y／gbOutSh2Y／gbInSh1Y／gbInSh2Y ->Visible = (MOTION_CARD_TYPE==1)                    :82-85
 *     gbOutSh3Z／gbOutSh3Y／btSh3Servo／ledSht3Servo／lblSht3Servo ->Visible = (USE_OUT_SORT_ARM!=eartUninstall)  :87-91
 *     palInSHAutoLtc->Visible = (In_Shuttle_Auto_Latch==eInSHAutoLtc)                                :93-102
 *     旗標照 golden database.cpp 的讀法從 wb_serve 即時的 Gerneral.ini 讀（HT9045System.read('gerneral')）：
 *       MOTION_CARD_TYPE      [System]     MOTION_CARD_TYPE       預設 0   （database.cpp:1202）
 *       USE_OUT_SORT_ARM      [OutSortArm] USE_OUT_SORT_ARM       預設 0＝eartUninstall（database.cpp:826）
 *       In_Shuttle_Auto_Latch [System]     In_Shuttle_Auto_Latch  預設 0，CheckRange 0..1（database.cpp:1665）
 *     pnlCenter 底下全是 Align=alLeft（LtcSensor.dfm）：藏掉的群組不留空位，後面的往左補（VCL 排列規則）。
 *     讀不到 ＝ 照 .dfm 全部顯示，狀態列寫原因（不猜）。
 *   TimerLtcSensorTimer :130-171（Interval 100，FormShow 開、FormClose 關）
 *     ledSht1Servo／ledSht2Servo／ledSht3Servo ->Value = MOT[MInShuttle1／MInShuttle2／MOutSortSht].Led[iServoOn]   :138-139／:165
 *       → 讀 C++ 的 /api/struct/motor/runtime（state.servoOn；quality 不是 good 或沒有這一軸 ＝ 斜線「不知道」，不畫成滅）。
 *     lblLtcCount／ledOut*／ledIn*（latch）：golden 讀 Motor->GetLatchTotalLen()／GetLatchIOStatus()。
 *       這台的軸全是 PCI1203（TMyEtherCatMotor），golden 那幾個函式本體是空的（golden myEthercatmotor.cpp：ResetLatch／
 *       GetLatchTotalLen 回 0／GetLatchBuffer 回 0／GetLatchIOStatus 回 false／SetFIFOLatchSrc 空，golden Motor\myEthercatmotor.cpp:1545-1566）——golden 在這台也只會顯示
 *       「Total Ltc Counter:0」、燈全滅、Get 之後計數 0、memo 清空。這裡照 golden 的結果顯示，並在提示說明「1203 沒有 latch」；
 *       只在 C++ 的馬達表說這幾軸是 PCI1203 時這樣顯示，否則（不是 1203、讀不到）畫成「不知道」。
 *   btSetLtc（:123 SetLtcSensor）／btGet*（:482 btGetSh1LtcClick …）：在 1203 上 golden 呼叫的都是上面那些空函式 ——
 *     灰掉並說明原因（golden 按了也什麼都不會發生，只會把計數寫成 0）。
 *   btSh1Servo／btSh2Servo／btSh3Servo（:105-121 ServoOnOff(!Led[iServoOn])＋fAllMotorHome=false）：這一頁沒有送到 C++ 的通道
 *     （motor.access 只認 uMotorTest／uteach），要接得 EastSun 同意 ⇒ 灰掉並說明；不送任何東西。
 *   btnClose：頁面內建 .exitbtn（golden btnCloseClick :678 Close）。
 * 不送任何命令、不寫任何檔；只讀 /api/system/gerneral、/api/struct/motor/config、/api/struct/motor/runtime。
 * 「開窗」＝外框 background.html 的 HT_WIN（同 ht9045_teach_trayz_c.js）：關→開跑一次 FormShow、開著才輪詢（golden Timer 只在 FormShow 後跑）。
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';

  var TAG = '[W906-LTC-C]';
  // golden FormShow :82-102 —— 群組 → 條件（v：讀到的旗標）
  var RULES = {
    gbOutSh1Y: function (v) { return v.MOTION_CARD_TYPE === 1; },
    gbOutSh2Y: function (v) { return v.MOTION_CARD_TYPE === 1; },
    gbInSh1Y:  function (v) { return v.MOTION_CARD_TYPE === 1; },
    gbInSh2Y:  function (v) { return v.MOTION_CARD_TYPE === 1; },
    gbOutSh3Z:    function (v) { return v.USE_OUT_SORT_ARM !== 0; },
    gbOutSh3Y:    function (v) { return v.USE_OUT_SORT_ARM !== 0; },
    btSh3Servo:   function (v) { return v.USE_OUT_SORT_ARM !== 0; },
    ledSht3Servo: function (v) { return v.USE_OUT_SORT_ARM !== 0; },
    lblSht3Servo: function (v) { return v.USE_OUT_SORT_ARM !== 0; },
    palInSHAutoLtc: function (v) { return v.In_Shuttle_Auto_Latch === 1; }
  };
  // LtcSensor.dfm pnlCenter 的 alLeft 子元件，照 .dfm 的 Left 排序（VCL 依這個順序由左往右排）
  var ALLEFT = [['pnlCaption', 76], ['gbOutSh1Z', 142], ['gbOutSh2Z', 142], ['gbOutSh3Z', 142], ['gbOutSh1Y', 90],
                ['gbOutSh2Y', 90], ['gbOutSh3Y', 142], ['gbInSh1Y', 90], ['gbInSh2Y', 90], ['palInSHAutoLtc', 185]];
  var ALLEFT_X0 = 4;                                    // LtcSensor.dfm：pnlCaption Left=4
  // golden TimerLtcSensorTimer 的伺服燈 → 軸
  var SERVO = { ledSht1Servo: 'MInShuttle1', ledSht2Servo: 'MInShuttle2', ledSht3Servo: 'MOutSortSht' };
  // golden 的 latch 燈 → 軸（:144-169）
  var LATCH_LED = { ledOut1_1: 'MInShuttle1', ledOut1_2: 'MInShuttle1', ledOut2_1: 'MInShuttle2', ledOut2_2: 'MInShuttle2',
                    ledIn1_Y: 'MInShuttle1', ledOut1_Y: 'MInShuttle1', ledIn2_Y: 'MInShuttle2', ledOut2_Y: 'MInShuttle2',
                    ledIn1_2_Y: 'MInShuttle1', ledIn2_2_Y: 'MInShuttle2',
                    ledOut3_1: 'MOutSortSht', ledOut3_2: 'MOutSortSht', ledOut3_Y: 'MOutSortSht', ledOut3_Y2: 'MOutSortSht' };
  var LATCH_BTN = { btSetLtc: 'LtcSensor.cpp:123 btSetLtcClick → SetLtcSensor(0..2)',
                    btGetSh1Ltc: 'LtcSensor.cpp:482 btGetSh1LtcClick', btGetSh1YLtc: 'LtcSensor.cpp:482 btGetSh1LtcClick',
                    btGetInSh1YLtc: 'LtcSensor.cpp:482 btGetSh1LtcClick', Button5: 'LtcSensor.cpp:482 btGetSh1LtcClick',
                    btGetSh2Ltc: 'LtcSensor.cpp:556 btGetSh2LtcClick', btGetSh2YLtc: 'LtcSensor.cpp:556 btGetSh2LtcClick',
                    btGetInSh2YLtc: 'LtcSensor.cpp:556 btGetSh2LtcClick', Button6: 'LtcSensor.cpp:556 btGetSh2LtcClick',
                    btGetSh3Ltc: 'LtcSensor.cpp:630 btGetSh3LtcClick', btGetSh3YLtc: 'LtcSensor.cpp:630 btGetSh3LtcClick' };
  var SERVO_BTN = { btSh1Servo: ['MInShuttle1', 'LtcSensor.cpp:105-109 btSh1ServoClick'],
                    btSh2Servo: ['MInShuttle2', 'LtcSensor.cpp:111-115 btSh2ServoClick'],
                    btSh3Servo: ['MOutSortSht', 'LtcSensor.cpp:117-121 btSh3ServoClick'] };
  var NO_LATCH = 'PCI1203 軸沒有 latch：golden TMyEtherCatMotor 的 GetLatchTotalLen／GetLatchBuffer 回 0、GetLatchIOStatus 回 false、' +
                 'ResetLatch／SetFIFOLatchSrc 是空的（golden Motor\\myEthercatmotor.cpp:1545-1566）—— golden 在這台也只會顯示 0／燈滅';

  function $(id) { return document.getElementById(id); }
  function setTitle(el, t) {
    if (!el) return;
    var a = el.hasAttribute('title') ? 'title' : (el.hasAttribute('data-htitle') ? 'data-htitle' : 'title');
    var base = el.getAttribute('data-ltc-title');
    if (base === null) { base = el.getAttribute(a) || ''; el.setAttribute('data-ltc-title', base); }
    el.setAttribute(a, base + (t ? '\n' + t : ''));
  }

  // ---- 狀態列（右下角一行，同其他補件頁的做法）----
  function info(msg, bad) {
    var s = $('ltcInfo');
    if (!s) {
      s = document.createElement('div');
      s.id = 'ltcInfo';
      s.style.cssText = 'position:absolute;left:470px;top:44px;width:720px;font-size:11px;white-space:pre-wrap;color:#234;pointer-events:none;';
      ($('pnlTop') || document.body).appendChild(s);
    }
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '#234';
  }

  // ---- BCB TIniFile::ReadInteger（同 ht9045_teach_trayz_c.js 的 strToIntDef）----
  function strToIntDef(s, d) {
    if (typeof s !== 'string') return d;
    var m = /^ *([+-]?)(?:(?:\$|0?[xX])([0-9a-fA-F]+)|([0-9]+))$/.exec(s);
    if (!m) return d;
    var n = m[2] !== undefined ? (parseInt(m[2], 16) | 0) : parseInt(m[3], 10);
    return m[1] === '-' ? -n : n;
  }
  function pick(o, name) {
    if (!o || typeof o !== 'object') return undefined;
    if (Object.prototype.hasOwnProperty.call(o, name)) return o[name];
    var low = name.toLowerCase(), ks = Object.keys(o);
    for (var i = 0; i < ks.length; i++) if (ks[i].toLowerCase() === low) return o[ks[i]];
    return undefined;
  }
  function iniInt(secs, sec, key, dflt) {
    var e = pick(pick(secs, sec), key);
    if (e === undefined || e === null) return dflt;                 // golden CheckAndReadIniDataGeneral：鍵不在＝預設
    if (typeof e === 'object') {
      if (typeof e.bcb === 'string') return strToIntDef(e.bcb, dflt);
      if (typeof e.raw === 'string') return strToIntDef(e.raw, dflt);
      e = e.value;
    }
    if (typeof e === 'number') return e | 0;
    return strToIntDef(String(e), dflt);
  }

  // ---- golden FormShow ----
  function relayout() {                                              // VCL alLeft：看得到的依序往左排
    var x = ALLEFT_X0;
    ALLEFT.forEach(function (p) {
      var el = $(p[0]);
      if (!el || el.style.display === 'none') return;
      el.style.left = x + 'px';
      x += p[1];
    });
  }
  function formShow(why) {
    if (!global.HT9045System || typeof global.HT9045System.read !== 'function') {
      info('FormShow 規則沒有套用：ht9045_recipe_client.js 沒有載入（群組照 .dfm 全部顯示）', true);
      return;
    }
    global.HT9045System.read('gerneral').then(function (d) {
      if (!d || !d.sections || d.available === false) throw new Error('Gerneral.ini 讀不到' + (d && d.path ? '（' + d.path + '）' : ''));
      var v = {
        MOTION_CARD_TYPE: iniInt(d.sections, 'System', 'MOTION_CARD_TYPE', 0),
        USE_OUT_SORT_ARM: iniInt(d.sections, 'OutSortArm', 'USE_OUT_SORT_ARM', 0),
        In_Shuttle_Auto_Latch: Math.max(0, Math.min(1, iniInt(d.sections, 'System', 'In_Shuttle_Auto_Latch', 0)))
      };
      Object.keys(RULES).forEach(function (id) {
        var el = $(id); if (!el) return;
        var show = RULES[id](v);
        el.style.display = show ? '' : 'none';
        el.setAttribute('data-golden-visible', show ? '1' : '0');
      });
      relayout();
      ST.flags = v;
      info('golden FormShow（' + why + '）：MOTION_CARD_TYPE=' + v.MOTION_CARD_TYPE + '、USE_OUT_SORT_ARM=' + v.USE_OUT_SORT_ARM +
           '、In_Shuttle_Auto_Latch=' + v.In_Shuttle_Auto_Latch + '（Gerneral.ini，wb_serve 即時）');
    }).catch(function (e) {
      info('FormShow 規則沒有套用（' + ((e && e.message) || e) + '）——群組照 .dfm 全部顯示', true);
    });
  }

  // ---- golden TimerLtcSensorTimer（只在開窗後跑）----
  var ST = { flags: null, cfg: null, busy: false, timer: null, shown: false, hosted: false };
  function getJson(url) {
    return fetch(url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error('HTTP ' + r.status); return r.json();
    });
  }
  function driverOf(id) {                                            // C++ 馬達表（/api/struct/motor/config）
    var ms = (ST.cfg && ST.cfg.motors) || [];
    for (var i = 0; i < ms.length; i++) if (ms[i].motorId === id) return String(ms[i].driverType || '');
    return null;
  }
  function paintLed(el, state, why) {
    if (!el) return;
    el.classList.remove('on', 'unknown');
    if (state === true) el.classList.add('on');
    else if (state !== false) el.classList.add('unknown');
    setTitle(el, why);
  }
  function tick() {
    if (ST.busy || document.hidden || (ST.hosted && !ST.shown)) return;
    ST.busy = true;
    var cfgP = ST.cfg ? Promise.resolve(ST.cfg) : getJson('/api/struct/motor/config').then(function (c) { ST.cfg = c; return c; });
    cfgP.then(function () { return getJson('/api/struct/motor/runtime'); }).then(function (rt) {
      ST.busy = false;
      var by = {};
      ((rt && rt.motors) || []).forEach(function (m) { by[m.motorId] = m; });
      Object.keys(SERVO).forEach(function (led) {                    // :138-139／:165 Led[iServoOn]
        var m = by[SERVO[led]], s = m && m.state;
        if (!m) { paintLed($(led), null, SERVO[led] + '：C++ 馬達表沒有這一軸（不知道）'); return; }
        if (!s || s.quality !== 'good' || typeof s.servoOn !== 'boolean')
          { paintLed($(led), null, SERVO[led] + '：伺服狀態讀不到（quality=' + (s && s.quality) + '）'); return; }
        paintLed($(led), s.servoOn, SERVO[led] + ' Servo ' + (s.servoOn ? 'ON' : 'OFF') + '（C++ /api/struct/motor/runtime）');
      });
      var s1 = driverOf('MInShuttle1'), s2 = driverOf('MInShuttle2');
      var all1203 = /PCI1203/i.test(s1 || '') && /PCI1203/i.test(s2 || '');
      var lc = $('lblLtcCount');                                      // :141-142（MOTION_CARD_TYPE==0：只有 MInShuttle1 的長度）
      if (lc) {
        if (all1203) { lc.textContent = 'Total Ltc Counter:0'; setTitle(lc, NO_LATCH); }
        else { lc.textContent = 'Total Ltc Counter:---'; setTitle(lc, 'Shuttle 軸不是 PCI1203（' + s1 + '／' + s2 + '）或馬達表讀不到 —— latch 長度沒有來源'); }
      }
      Object.keys(LATCH_LED).forEach(function (id) {                 // :144-169 GetLatchIOStatus
        var d = driverOf(LATCH_LED[id]);
        if (/PCI1203/i.test(d || '')) paintLed($(id), false, NO_LATCH);
        else paintLed($(id), null, LATCH_LED[id] + '：' + (d === null ? 'C++ 馬達表沒有這一軸' : '不是 PCI1203（' + d + '）—— latch 狀態沒有來源'));
      });
    }).catch(function (e) {
      ST.busy = false;
      Object.keys(SERVO).forEach(function (led) { paintLed($(led), null, '/api/struct/motor 讀不到（' + ((e && e.message) || e) + '）'); });
    });
  }

  // ---- 按鈕：灰掉並說明（不送任何東西）----
  function lockButtons() {
    Object.keys(LATCH_BTN).forEach(function (id) {
      var b = $(id); if (!b) return;
      b.disabled = true; b.style.cursor = 'not-allowed';
      setTitle(b, 'golden ' + LATCH_BTN[id] + '。' + NO_LATCH + '（按了也只會把計數寫成 0），所以停用。');
    });
    Object.keys(SERVO_BTN).forEach(function (id) {
      var b = $(id); if (!b) return;
      b.disabled = true; b.style.cursor = 'not-allowed';
      setTitle(b, 'golden ' + SERVO_BTN[id][1] + '：MOT[' + SERVO_BTN[id][0] + '].ServoOnOff(!Led[iServoOn])、fAllMotorHome=false。' +
               '這一頁還沒有送到 C++ 的通道（motor.access 只認 Motor Test／Teach），要 EastSun 同意才接 —— 目前停用；要切伺服請用 Motor Test。');
    });
  }

  function start(why) {
    formShow(why);
    if (!ST.timer) ST.timer = setInterval(tick, 100);               // golden TimerLtcSensor Interval=100
    tick();
  }
  global.addEventListener('message', function (ev) {                 // HT_WIN（background.html），同 ht9045_teach_trayz_c.js
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN' || global.parent === global || ev.source !== global.parent) return;
    ST.hosted = true;
    if (m.open && !ST.shown) { ST.shown = true; start('HT_WIN open' + (m.initial ? ' (initial)' : '')); }
    else if (!m.open) ST.shown = false;                              // golden FormClose :70-75：Timer 關
  });
  function boot() {
    lockButtons();
    if (global.parent === global) { start('standalone'); return; }
    setTimeout(function () { if (!ST.hosted) start('no HT_WIN in 3 s'); }, 3000);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true }); else boot();
  global.HT9045LtcSensor = { RULES: RULES, formShow: formShow, tick: tick, state: ST, iniInt: iniInt };
  if (global.console) console.info(TAG + ' loaded');
})(window);
