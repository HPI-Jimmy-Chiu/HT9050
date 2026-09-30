/* ht9045_main_st01_ev.js -- 主畫面（golden TfMain，V912 main.cpp）的圖示、按鈕、子頁事件：批次 B6＋B9 的頁面那一半。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB6) 20260928 [W906] St01 新檔（手寫）。派工單 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B6／B9、第四節 M-2～M-20。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」；
 *   S169：主畫面 Site 格與控制鈕「c++部分只需要做到事件觸發……實際動作可以先列表, 後面通知Jimmy進行接上」。
 *   C++ 那一半：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp 檔尾 W906_Main_EvB6Op（WS act.main.*，
 *   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp St01 那一行分派）。守衛（golden 的 Enabled／Visible／權限／運轉中）
 *   全部在 C++ 重查，這支只送「按了」、照回覆顯示。
 *
 * 同一支檔載入好幾頁，各段看頁面上有沒有那個元件才接（沒有就不做事）：
 *   main.html               M-2 ▣ Run Mode（act.main.runMode）、M-11 FT／RT 小方塊（act.main.ftrt，B8，AI(W906-B8-M11) 20260930）、M-3 🌡️ 溫度（act.main.tempMode）、M-4 🔗 Tester（act.main.testerConnect，
 *                           St02 的 C++，JsonBridge/actions/MainTesterConnect.cpp）、M-8 Light（act.main.light）、M-9 FAN（act.main.fan）、
 *                           M-10 ⬅️ 收合清單（act.main.funcView）、M-12 登入顯示跟著別頁變（tag auth.level／user.level → main.html 登入區的 refresh）、
 *                           B9 M-6 Site 格（act.main.siteClick，只登記事件）；Run Mode 字樣照 golden LoadRunModePicture（main.cpp:12903）由
 *                           tag lastset.realDummy 組（runmode.value 在 WebBridgeTags.cpp 的「不發布」清單，恆為 null）。
 *   Main.gbControlBtn.html  B9 M-7：HOME → main.home（Jimmy 816ce9b2，origin/main；本分支還沒合入時伺服器回 unknown cmd）、
 *                           CLEAN OUT → act.main.cleanOut（已翻 cCleanOut.cpp:64，照 golden 接上）、RESET／ONE CYCLE／TRAY FEED／
 *                           ALARM RESET → act.main.ctlButton（S169 只登記事件；⛔ AI(W906-FLOW4-HINT) 20260930：筆電 FLOW-4 起 ONE CYCLE／TRAY FEED／ALARM RESET 由 WebMainCtlButtons.cpp 照 golden 執行，RESET 仍沒接）。按鈕加 data-st01ev，ht9045_opbuttons.js 就不再說「尚未接線」。
 *   Main.CommView.html      M-15 Set／Read Z1／Z2（act.main.indexTorque）、M-18 StringGrid2 雙擊（act.main.sg2DblClick → 右鍵選單；
 *                           「Set To Define Value」＝act.main.setToDefineValue 兩段式確認）。
 *   Main.TaskList.html      M-18 sgTaskList 雙擊（golden 掛同一支 StringGrid2DblClick，main.dfm:16279 —— 怪處照留）。
 *   Main.HeaterView.html    M-16 palHP2View（act.main.hp2View；golden 後門，V912 沒人讀那個旗標）。
 *   Main.MotorView.html     M-17 cbCheckEncoderEveryTime（act.main.checkEncoder）。
 *   Main.MotionView.html    CL-5 主畫面那一半：Cleaning 頁 sbTrayAssign 之後顯示 AutoCleanStringGrid（golden
 *                           uCleaning.cpp:2313-2314 MainFormSizeToEpson(false)＋AutoCleanStringGrid->Visible=true）。
 *                           M-13 逐格托盤是 St02 的 ht9045_mv_trays.js（St02 分支 b243f15e），這裡不做。
 *
 * 規則：
 *   權杖：act.main.* 不在免權杖清單 → 本頁 client 沒拿著就先 HT9045Recipe.keepAlive()（＝control.acquire）；main.html 上做完、而且是這一下
 *     拿的，就馬上還（main.html 一直開著，拿著不還會把 IO／Motor Test 頁卡住，同 main.html 換配方那段的 AI(W906-MERGE-56bbf785)）；
 *     其他頁交給 recipe client 的 30 秒閒置自動還（AI(W906-TOKEN-IDLE)）。
 *   防連點（前端第二道；伺服器 WebCmdGuard 400 ms 是主防線）：同一個指令還在路上就不送；做完 400 ms 內的點擊忽略；
 *     伺服器回 busy:（上一下還在跑或剛做完）不算失敗、不跳框。
 *   失敗（ok:false）時伺服器的回覆 JSON 在 e.message 裡（ht9045_opbuttons.js detailOf 的說明），這裡解開顯示 guard／detail。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.rawCmd || window.__htMainEvB6) return;
  window.__htMainEvB6 = true;

  var LOG = '[MainEvB6] ';
  var IS_MAIN = !!(document.getElementById('palMainStatus') && document.getElementById('SitePanel'));
  var BUSY = {}, COOL = {};

  function $(id) { return document.getElementById(id); }
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function isBusyErr(e) { return !!(window.HT9045Busy && HT9045Busy.is(e)); }

  /* ---- 狀態列：有引擎用引擎的，控制鈕頁用 ht9045OpMsg，其他頁自己放一條 ---------------- */
  var ownBar = null;
  function say(msg, kind) {
    var colour = kind === 'err' ? '#f88' : (kind === 'ok' ? '#8f8' : (kind === 'warn' ? '#ffcc66' : '#ddd'));
    if (window.console) console.info(LOG + msg);
    var op = $('ht9045OpMsg');
    if (op) { op.textContent = msg; op.style.color = kind === 'err' ? '#a00' : (kind === 'ok' ? '#070' : (kind === 'warn' ? '#b26b00' : '#345')); return; }
    if (window.HT9045Wire && HT9045Wire.say) { HT9045Wire.say(msg, colour, 'transient'); return; }
    if (!ownBar) {
      ownBar = document.createElement('div');
      ownBar.style.cssText = 'position:fixed;left:4px;bottom:4px;max-width:70%;z-index:9999;padding:3px 8px;border-radius:3px;' +
                             "background:rgba(20,30,40,.85);font:12px 'Microsoft JhengHei',sans-serif;white-space:pre-wrap;";
      document.body.appendChild(ownBar);
    }
    ownBar.textContent = msg;
    ownBar.style.color = colour;
    ownBar.style.display = '';
    if (ownBar.__t) clearTimeout(ownBar.__t);
    ownBar.__t = setTimeout(function () { ownBar.style.display = 'none'; }, 8000);
  }

  function unwrap(m) {                                   // ok:true → 回覆併在 ack 頂層（舊形狀放在 value 字串）
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m || {};
  }
  function detailOf(e) {                                 // ok:false → 回覆 JSON 在 e.message（字串）
    var s = (e && typeof e.message === 'string') ? e.message : String(e || '');
    s = s.replace(/^\s*Error:\s*/, '');
    if (s.charAt(0) === '{') { try { var d = JSON.parse(s); if (d && typeof d === 'object') return d; } catch (x) { /* 不是 JSON */ } }
    return { executed: false, guard: 'transport', detail: s };
  }
  function why(d) { return (d.guard || '?') + (d.detail ? '（' + d.detail + '）' : ''); }

  /* ---- 送一條指令（權杖、防連點）------------------------------------------------ */
  function send(cmd, payload, label) {
    var key = cmd + '|' + JSON.stringify(payload || {});
    var now = Date.now();
    if (BUSY[key]) { say(label + '：上一下還在處理，這一下略過', 'warn'); return Promise.resolve(null); }
    if (COOL[key] && now < COOL[key]) return Promise.resolve(null);   // 剛做完 400 ms 內：忽略（雙擊的第二下）
    BUSY[key] = true;
    var took = false;
    var st = R.status ? R.status() : null;
    var pre = (st && !st.holdsToken && R.keepAlive) ? R.keepAlive().then(function () { took = true; }, function () { /* 別頁拿著：後面的指令自己會回 not-operator／control-held */ })
                                                    : Promise.resolve();
    var extra = (payload === null) ? undefined : { value: JSON.stringify(payload || {}) };
    return pre.then(function () { return R.rawCmd(cmd, extra); })
      .then(function (m) { return { ok: true, d: unwrap(m) }; }, function (e) { return { ok: false, d: detailOf(e), e: e }; })
      .then(function (r) {
        BUSY[key] = false;
        COOL[key] = Date.now() + coolMs();
        if (took && IS_MAIN && R.release) {              // main.html：這一下拿的就馬上還（見檔頭）
          var s2 = R.status ? R.status() : null;
          if (s2 && s2.holdsToken) R.release().catch(function () {});
        }
        if (!r.ok && isBusyErr(r.e)) { say(label + '：' + ((window.HT9045Busy && HT9045Busy.NOTE) || '同一個指令剛送過，這一下略過'), 'warn'); return null; }
        if (!r.ok && r.d && /unknown cmd|unknown-action/i.test(String(r.d.detail || '') + String(r.d.guard || ''))) {
          say(label + '：伺服器不認得 ' + cmd + '（wb_serve 是舊版，或這條指令在還沒合入的分支）', 'err');
          return null;
        }
        return r;
      });
  }

  /* ======================================================================
   * main.html
   * ====================================================================== */
  // golden TfMain::LoadRunModePicture（V912 main.cpp:12903 起）：palRunMode_1->Caption
  function runModeCaption(v) {
    if (v === 0) return 'No Device/ No Tray';            // DUMMY（cmydef.cpp:267）
    if (v === 1) return 'No Device';                     // HAS_TRAY（:266）
    if (v === 2) return 'Normal';                        // REALLY（:265）
    return null;
  }
  var lastRealDummy = null;
  function applyRunMode() {
    var el = $('palRunMode_1');
    if (!el) return;
    var c = runModeCaption(lastRealDummy);
    if (c === null) return;                              // 不知道就留給引擎的 '---'
    el.textContent = c;
    el.title = 'palRunMode_1：golden LoadRunModePicture（main.cpp:12903）的字樣，由 tag lastset.realDummy＝' + lastRealDummy + ' 組';
  }

  function wireIcon(id, cmd, label, payload, after) {
    var el = $(id);
    if (!el || el.__evb6) return;
    el.__evb6 = true;
    el.style.cursor = 'pointer';
    el.addEventListener('click', function () {
      send(cmd, payload, label).then(function (r) {
        if (!r) return;
        if (r.ok && r.d.executed !== false) { if (after) after(r.d, true); else say(label + '：完成', 'ok'); return; }
        if (after && r.ok) { after(r.d, false); return; }
        say(label + '：沒有執行 —— ' + why(r.d), r.d.guard === 'transport' ? 'err' : 'warn');
      });
    });
  }

  function initMain() {
    // M-2 Run Mode ▣（golden imgRunModeClick main.cpp:29796）
    wireIcon('imgRunMode', 'act.main.runMode', 'Run Mode', { op: 'click' }, function (d, ok) {
      if (typeof d.realDummy === 'number') { lastRealDummy = d.realDummy; applyRunMode(); }
      say('Run Mode：' + (ok ? '換成 ' + (d.caption || d.mode) : '沒有換 —— ' + why(d)), ok ? 'ok' : 'warn');
    });
    // M-3 溫度 🌡️（golden Panel42Click main.cpp:22392 → ChangeTempMode(10)）；畫面字樣由 tag temp.mode 更新
    wireIcon('imgTempOnOff', 'act.main.tempMode', '溫度模式', { op: 'click' }, function (d, ok) {
      if (!ok) { say('溫度模式：沒有切換 —— ' + why(d), 'warn'); return; }
      var names = { 0: 'Hot', 1: 'Ambient', 2: 'ATC', 3: 'AmbientHot' };
      var changed = d.machineTempModeBefore !== d.machineTempMode;
      say('溫度模式：' + (changed ? names[d.machineTempModeBefore] + ' → ' + names[d.machineTempMode]
                                 : '沒有變（ChangeTempMode 回 ' + d.changeTempModeReturn + '：golden 自己擋下，例如機台內有 IC、運轉中）') +
          (d.todo && d.todo.length ? '；未接：' + d.todo.join('；') : ''), changed ? 'ok' : 'warn');
    });
    // M-4 Tester 🔗（golden imgTesterClick main.cpp:29732；C++ 是 St02 的 act.main.testerConnect）
    wireIcon('imgTester', 'act.main.testerConnect', 'Tester 連線', {}, function (d, ok) {
      say('Tester 連線：' + (ok ? d.before + ' → ' + d.after + (d.modeChanged ? '' : '（沒變：' + (d.note || 'golden 自己擋下') + '）')
                                : '沒有切換 —— ' + why(d)), ok && d.modeChanged ? 'ok' : 'warn');
    });
    // M-8 Light（golden spbLightClick main.cpp:26780）；字樣由 tag light.off 更新
    var lightBtn = document.querySelector('.bigbtn[title="spbLight"]');
    if (lightBtn) { lightBtn.id = lightBtn.id || 'spbLightBtn'; wireIcon(lightBtn.id, 'act.main.light', 'Light', { op: 'click' }, function (d, ok) {
      say('Light：' + (ok ? d.caption + (d.ioEnable ? '' : '（IO 表沒載入：只記了命令，沒有真的輸出）') : '沒有動作 —— ' + why(d)), ok ? 'ok' : 'warn');
    }); }
    // M-9 FAN（golden spbFanClick main.cpp:26763）；fan.off 恆為 null（WebBridgeTags.cpp 的不發布清單）→ 字樣照回覆
    var fanBtn = document.querySelector('.bigbtn[title="spbFan"]');
    if (fanBtn) { fanBtn.id = fanBtn.id || 'spbFanBtn'; wireIcon(fanBtn.id, 'act.main.fan', 'FAN', { op: 'click' }, function (d, ok) {
      var cap = $('spbFanCap');
      if (ok && cap) { cap.textContent = String(d.caption || '---').replace(/^FAN\s*/, ''); cap.title = 'golden spbFan->Caption（LastSet.bBigFan=' + d.bigFan + '）；風扇輸出要等 golden Timer2Timer :21665 接上'; }
      say('FAN：' + (ok ? d.caption + '（LastSet.bBigFan=' + d.bigFan + '；實際風扇輸出 golden 在 Timer2Timer，還沒接）' : '沒有動作 —— ' + why(d)), ok ? 'ok' : 'warn');
    }); }
    // M-10 ⬅️ 收合／展開功能狀態清單（golden btnViewClick main.cpp:8689；ShowFunctions 寬 200／50）
    wireIcon('btnView', 'act.main.funcView', '功能清單', { op: 'click' }, function (d, ok) {
      if (!ok) { say('功能清單：沒有動作 —— ' + why(d), 'warn'); return; }
      var pf = $('palFunc'), bv = $('btnView');
      if (pf) { pf.style.maxWidth = d.showFunc ? '' : '50px'; pf.style.overflow = d.showFunc ? '' : 'hidden'; }
      if (bv) { bv.textContent = d.showFunc ? '⬅️' : '➡️'; bv.title = 'btnView（golden bShowFunc=' + d.showFunc + '，每項寬 ' + d.itemWidth + '）'; }
    });
    // AI(W906-B8-M11) 20260930 [W906] St01：B8 M-11 FT／RT 小方塊（golden palFTClick／palRTClick main.cpp:30700／:30705 → DoFTRTClick :35752 →
    //   FTClick :30710／RTClick :30834；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「M-11」）。main.html 的 palFT／palRT（Jimmy 的頁，
    //   不改）只顯示 tag run.ft／run.rt；這裡接 click → WS act.main.ftrt {"op":"click","tile":"FT"|"RT"}（C++ FileRW/MainClick.cpp 檔尾 W906_Main_FtRtOp）。
    //   點不點得到（golden palFT／palRT 的 Visible／Enabled：Enabled＝ChangeLevelAttr 算的起動模式鎖，機台有料、等級不夠、權限檔…都鎖；運轉中 golden 按得到、FTClick／RTClick 第一行回 1）全在 C++ 重算；
    //   起動模式與 Run Mode 字的結果照回覆顯示（主畫面的燈號由 tag 更新）。
    [['palFT', 'FT'], ['palRT', 'RT']].forEach(function (t) {
      wireIcon(t[0], 'act.main.ftrt', t[1], { op: 'click', tile: t[1] }, function (d, ok) {
        var msg = d.message ? '（golden 訊息：' + d.message + '）' : '';
        if (!ok) { say(t[1] + '：golden ' + t[1] + 'Click 回 ' + d.result + '，沒有切換 —— ' + (d.guard || '?') + msg, 'warn'); return; }   // 點得到、golden 自己擋下（伺服器點不到的由 wireIcon 顯示）
        say(t[1] + '：起動模式 ' + (d.captionBefore || '?') + ' → ' + (d.caption || '?') + '，Run Mode ' + (d.runModeBefore || '?') + ' → ' + (d.runMode || '?') +
            (d.todo && d.todo.length ? '；' + d.todo.join('；') : '') + msg, d.modeChanged ? 'ok' : 'warn');
      });
    });
    // B9 M-6 Site 格（golden mtDutOnOffMouseUp main.cpp:29932）：S169 只登記事件
    var sp = $('SitePanel');
    if (sp && !sp.__evb6) {
      sp.__evb6 = true;
      sp.addEventListener('click', function (ev) {
        var c = ev.target && ev.target.closest ? ev.target.closest('.cell') : null;
        if (!c || !sp.contains(c)) return;
        var x = parseInt(c.getAttribute('data-x'), 10), y = parseInt(c.getAttribute('data-y'), 10);
        if (isNaN(x) || isNaN(y)) return;
        send('act.main.siteClick', { op: 'click', x: x, y: y }, 'Site 格').then(function (r) {
          if (!r) return;
          if (r.ok) say('Site 格 Arm' + r.d.arm + ' 第 ' + (r.d.row + 1) + ' 列第 ' + (r.d.col + 1) + ' 格：C++ 已記下「點了」（S169；' + Math.round((r.d.eventTtlMs || 3000) / 1000) + ' 秒內沒人處理就丟掉）；實際開關 site 還沒接（Jimmy），畫面不會變', 'warn');
          else say('Site 格：沒有登記 —— ' + why(r.d), 'warn');
        });
      });
    }
    // Run Mode 字樣（tag lastset.realDummy）；引擎的 runmode.value 送 null 時會寫 '---'，所以之後再蓋一次
    if (window.HT9045Tags) {
      HT9045Tags.on('lastset.realDummy', function (v) { lastRealDummy = (typeof v === 'number') ? v : null; setTimeout(applyRunMode, 0); });
      HT9045Tags.on('runmode.value', function () { setTimeout(applyRunMode, 0); });
      // M-12：別頁（SetUp、Configuration 重新登入）改了登入狀態 → 主畫面登入區重讀（auth.mode 免權杖）
      var lt = null, seen = {};
      var loginChanged = function (v, tag) {
        if (!(tag in seen)) { seen[tag] = v; return; }   // 第一個訊框是現值，不是「變了」
        if (seen[tag] === v) return;
        seen[tag] = v;
        if (lt) clearTimeout(lt);
        lt = setTimeout(function () {
          lt = null;
          var L = window.HT9045MainLogin;
          if (L && L.refresh && !(L.busy && L.busy())) L.refresh();
        }, 150);
      };
      HT9045Tags.on('auth.level', loginChanged);
      HT9045Tags.on('user.level', loginChanged);
    }
  }

  /* ======================================================================
   * Main.gbControlBtn.html（B9 M-7）
   * ====================================================================== */
  function initCtlButtons() {
    // AI(W906-FLOW4-HINT) 20260930 St01: FLOW-4 (laptop, main e2dac07e) consumes act.main.ctlButton for these three on the next tick --
    //   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMainCtlButtons.cpp runs golden BtnOneCycleClick (906 main.cpp:4332) / BtnTrayEndClick
    //   (:13944) / BtnAlarmResetClick (:22159). "reset" and the site cell are still NOT consumed (Jimmy's decision 5).
    var CTL_WIRED = { oneCycle: 'BtnOneCycleClick', trayFeed: 'BtnTrayEndClick', alarmReset: 'BtnAlarmResetClick' };
    var defs = [
      ['BtnHome',       'main.home',          null,                                   'HOME'],
      ['BtnCleanOut',   'act.main.cleanOut',  { op: 'click' },                        'CLEAN OUT'],
      ['BtnReset',      'act.main.ctlButton', { op: 'click', button: 'reset' },       'RESET'],
      ['BtnOneCycle',   'act.main.ctlButton', { op: 'click', button: 'oneCycle' },    'ONE CYCLE'],
      ['BtnTrayEnd',    'act.main.ctlButton', { op: 'click', button: 'trayFeed' },    'TRAY FEED'],
      ['BtnAlarmReset', 'act.main.ctlButton', { op: 'click', button: 'alarmReset' },  'ALARM RESET']
    ];
    defs.forEach(function (d) {
      var b = $(d[0]);
      if (!b || b.__evb6) return;
      b.__evb6 = true;
      b.setAttribute('data-st01ev', '1');               // ht9045_opbuttons.js 看到就不說「尚未接線」
      b.addEventListener('click', function () {
        send(d[1], d[2], d[3]).then(function (r) {
          if (!r) return;
          var a = r.d;
          if (d[1] === 'main.home') {
            if (r.ok) say('HOME：' + (a.outcome || '?') + (a.why ? '（' + a.why + '）' : ''), a.outcome === 'homeArmed' ? 'ok' : 'warn');
            else say('HOME：沒有動作 —— ' + (a.outcome || a.guard || '?') + (a.why || a.detail ? '（' + (a.why || a.detail) + '）' : ''), 'warn');
            return;
          }
          if (d[1] === 'act.main.cleanOut') {
            if (r.ok && a.executed) say('CLEAN OUT：golden CleanOut 已執行（iCleanOut ' + a.cleanOutBefore + ' → ' + a.cleanOut + '）', 'ok');
            else say('CLEAN OUT：沒有執行 —— ' + why(a), 'warn');
            return;
          }
          var wired = d[2] && CTL_WIRED[d[2].button];   // AI(W906-FLOW4-HINT) 20260930: the three FLOW-4 buttons run golden on the next tick
          if (r.ok && wired) say(d[3] + '：C++ 已收到，下一拍照 golden ' + wired + ' 執行（golden 自己的條件不符合就不動；結果看狀態列與事件紀錄）', 'ok');
          else if (r.ok) say(d[3] + '：C++ 已記下「按了」（S169；' + Math.round((a.eventTtlMs || 3000) / 1000) + ' 秒內沒人處理就丟掉）；實際動作還沒接（Jimmy 決定），機台不會動', 'warn');
          else say(d[3] + '：沒有登記 —— ' + why(a), 'warn');
        });
      });
    });
  }

  /* ======================================================================
   * Main.CommView.html／Main.TaskList.html（M-15、M-18）
   * ====================================================================== */
  function initTorque() {
    [['btnSetZ1', 'set', 0, 'edtSetZ1'], ['btnSetZ2', 'set', 1, 'edtSetZ2'], ['btnReadZ1', 'read', 0, null], ['btnReadZ2', 'read', 1, null]].forEach(function (t) {
      var b = $(t[0]);
      if (!b || b.__evb6) return;
      b.__evb6 = true;
      b.addEventListener('click', function () {
        var p = { op: t[1], z: t[2] };
        if (t[3]) { var ed = $(t[3]); p.text = ed ? String(ed.value).trim() : ''; }
        var label = (t[1] === 'set' ? 'Set Z' : 'Read Z') + (t[2] + 1);
        send('act.main.indexTorque', p, label).then(function (r) {
          if (!r) return;
          if (r.ok) say(label + '：已送到 Index 驅動器' + (t[1] === 'set' ? '（golden：寫完要重新回原點，fAllMotorHome=' + r.d.allMotorHome + '）' : '（讀回值沒有 tag，畫面看不到）'), 'ok');
          else say(label + '：沒有送 —— ' + why(r.d), 'warn');
        });
      });
    });
  }

  var menu = null;
  function closeMenu() { if (menu && menu.parentNode) menu.parentNode.removeChild(menu); menu = null; }
  function openMenu(ev, grid, row, col, a) {
    closeMenu();
    menu = document.createElement('div');
    menu.style.cssText = 'position:fixed;z-index:10000;background:#f0f0f0;border:1px solid #888;box-shadow:2px 2px 4px rgba(0,0,0,.3);' +
                         "font:12px 'MS Sans Serif',sans-serif;color:#000;min-width:240px;";
    menu.style.left = Math.min(ev.clientX, window.innerWidth - 250) + 'px';
    menu.style.top = Math.min(ev.clientY, window.innerHeight - 80) + 'px';
    (a.items || []).forEach(function (it) {
      var m = document.createElement('div');
      m.textContent = it.caption + (it.ported ? '' : '（未移植）');
      m.style.cssText = 'padding:4px 10px;cursor:' + (it.ported ? 'pointer' : 'default') + ';color:' + (it.ported ? '#000' : '#888') + ';';
      m.title = 'golden ' + it.golden + (it.cmd ? ' → ' + it.cmd : '');
      m.addEventListener('mouseenter', function () { m.style.background = it.ported ? '#cde' : ''; });
      m.addEventListener('mouseleave', function () { m.style.background = ''; });
      m.addEventListener('click', function (e2) {
        e2.stopPropagation();
        closeMenu();
        if (!it.ported) { say(it.caption + '：golden ' + it.golden + ' 還沒翻（改吸嘴真空時間，交件清單）', 'warn'); return; }
        setToDefine(row, col, false);
      });
      menu.appendChild(m);
    });
    document.body.appendChild(menu);
  }
  function setToDefine(row, col, confirmed) {           // golden SetToDefineValue1Click（main.cpp:26672）：MessageDlg mbYes/mbNo → 兩段式
    send('act.main.setToDefineValue', { row: row, col: col, confirmed: confirmed }, 'Set To Define Value').then(function (r) {
      if (!r) return;
      if (r.ok && r.d.needConfirm) {
        if (window.confirm((r.d.prompt || ['Sure Set To Define Value?']).join('\n'))) setTimeout(function () { setToDefine(row, col, true); }, coolMs() + 20);
        return;
      }
      if (r.ok && r.d.executed) say('Set To Define Value：已照 Security_new.def [Dummy Vacuum] 設定（只改記憶體，golden 同）', 'ok');
      else say('Set To Define Value：沒有執行 —— ' + why(r.d), 'warn');
    });
  }
  function wireGridDbl(id) {
    var t = $(id);
    if (!t || t.__evb6) return;
    t.__evb6 = true;
    t.addEventListener('dblclick', function (ev) {
      var cell = ev.target && ev.target.closest ? ev.target.closest('td,th') : null;
      if (!cell || !t.contains(cell)) return;
      var tr = cell.parentElement;
      var row = tr ? tr.rowIndex : -1, col = cell.cellIndex;   // golden StringGrid2SelectCell 記的 ACol／ARow
      send('act.main.sg2DblClick', { row: row, col: col, grid: id }, '雙擊選單').then(function (r) {
        if (!r) return;
        if (r.ok && r.d.popup) openMenu(ev, id, row, col, r.d);
        else say('雙擊選單沒有出來 —— ' + why(r.d) + '（golden StringGrid2DblClick main.cpp:25128）', 'warn');
      });
    });
    document.addEventListener('click', closeMenu);
  }

  /* ======================================================================
   * Main.HeaterView.html（M-16）／Main.MotorView.html（M-17）
   * ====================================================================== */
  function initHeater() {
    wireIcon('palHP2View', 'act.main.hp2View', 'HOTPLATE 2', { op: 'click' }, function (d, ok) {
      if (ok) say('HOTPLATE 2：golden 後門已執行（IniConfig.bStartProductOnLine=false）；V912 沒有地方讀它，按了沒有效果（golden 同）', 'warn');
      else say('HOTPLATE 2：沒有執行 —— ' + why(d), 'warn');
    });
  }
  function initMotor() {
    var cb = $('cbCheckEncoderEveryTime');
    if (!cb || cb.__evb6) return;
    cb.__evb6 = true;
    cb.addEventListener('change', function () {
      send('act.main.checkEncoder', { op: 'click', checked: !!cb.checked }, 'Check Encoder').then(function (r) {
        if (!r) return;
        if (r.ok) say('Check Encoder：' + r.d.motorsSet + ' 軸設成每次檢查編碼器（golden 不看勾選狀態、不會清回來）', 'ok');
        else say('Check Encoder：沒有執行 —— ' + why(r.d), 'warn');
      });
    });
  }

  /* ======================================================================
   * Main.MotionView.html（CL-5 AutoCleanStringGrid；M-13 是 St02 的 ht9045_mv_trays.js）
   * ====================================================================== */
  var AC_KEY = 'ht9045-mv-autocleangrid';
  function acGrid() {
    var g = $('AutoCleanStringGrid');
    if (g) return g;
    g = document.createElement('div');
    g.id = 'AutoCleanStringGrid';
    g.title = 'AutoCleanStringGrid : TStringGrid（golden main.dfm:15048，20×20，DFM Visible=False）。' +
              '格子內容 golden SetAutoCleanStringGrid（main.cpp:11523）寫 —— 移植樹沒有 tag 送，所以是空的（不是 0）';
    g.style.cssText = 'position:fixed;right:8px;bottom:8px;z-index:900;display:none;background:#fff;border:2px solid #36c;padding:4px;' +
                      "font:10px 'MS Sans Serif',sans-serif;color:#000;";
    var hd = document.createElement('div');
    hd.style.cssText = 'display:flex;justify-content:space-between;gap:8px;margin-bottom:3px;';
    var t = document.createElement('b'); t.textContent = 'Auto Clean（Clean Count／Pad Set）';
    var x = document.createElement('span'); x.textContent = '✕'; x.style.cursor = 'pointer'; x.title = '關閉';
    x.addEventListener('click', function () { g.style.display = 'none'; });
    hd.appendChild(t); hd.appendChild(x); g.appendChild(hd);
    var tb = document.createElement('table');
    tb.style.cssText = 'border-collapse:collapse;';
    for (var r = 0; r < 20; r++) {
      var tr = document.createElement('tr');
      for (var c = 0; c < 20; c++) { var td = document.createElement('td'); td.style.cssText = 'width:13px;height:13px;border:1px solid #ccc;padding:0;'; tr.appendChild(td); }
      tb.appendChild(tr);
    }
    g.appendChild(tb);
    var note = document.createElement('div');
    note.textContent = '格子內容沒有來源（golden SetAutoCleanStringGrid 移植樹沒有 tag）';
    note.style.cssText = 'color:#a60;margin-top:3px;';
    g.appendChild(note);
    document.body.appendChild(g);
    return g;
  }
  function showAcGrid() { acGrid().style.display = ''; say('Motion View：golden sbTrayAssignClick（uCleaning.cpp:2313-2314）切到 Motion View、顯示 AutoCleanStringGrid', 'ok'); }

  function initMotionView() {
    // CL-5：Cleaning 頁送過 sbTrayAssign（ht9045_cleaning_ev.js 設 localStorage＋BroadcastChannel）
    try { var ts = parseInt(localStorage.getItem(AC_KEY) || '0', 10); if (ts && Date.now() - ts < 15000) { localStorage.removeItem(AC_KEY); showAcGrid(); } } catch (e) { /* 私密視窗 */ }
    try {
      var bc = new BroadcastChannel('ht9045-main-ev');
      bc.onmessage = function (m) { if (m && m.data && m.data.type === 'autoCleanGrid') { try { localStorage.removeItem(AC_KEY); } catch (e) {} showAcGrid(); } };
    } catch (e) { /* 沒有 BroadcastChannel：只靠 localStorage */ }

    // M-13（逐格托盤讀 tag motionView.trays.*）不在這裡：St02 已經做了 web/page/ht9045_mv_trays.js（St02 分支 b243f15e，
    //   Main.MotionView.html:2970 同一行載入），同一份 LIVE 兩支餵會互蓋 —— St01 不重做（交件報告）。
  }

  /* ---- 啟動 --------------------------------------------------------------- */
  function init() {
    try {
      if (IS_MAIN) initMain();
      if ($('gbControlBtn')) initCtlButtons();
      if ($('btnSetZ1')) initTorque();
      if ($('StringGrid2')) wireGridDbl('StringGrid2');
      if ($('sgTaskList')) wireGridDbl('sgTaskList');
      if ($('palHP2View')) initHeater();
      if ($('cbCheckEncoderEveryTime')) initMotor();
      if ($('liveStatus')) initMotionView();
    } catch (e) { if (window.console) console.error(LOG + 'init', e); }
  }
  window.HT9045MainEvB6 = { send: send, applyRunMode: applyRunMode, showAutoCleanGrid: showAcGrid };   // 探針／除錯用
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init);
  else init();
})();
