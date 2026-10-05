/* ht9045_teach_3axes_c.js -- HW.teach.html：MOutShuttle1／MOutShuttle2／MCCDY 的教導點（W906 擴充，不是 golden）顯示規則。
 * ---------------------------------------------------------------------------
 * AI(W906-TEACH-3AXES) 20261001: EastSun 1001「三軸都幫我加 在合適的地方」。
 *   手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js（同 ht9045_teach_trayz_c.js 的理由），免得產生器覆蓋。
 *
 * C++ 那一半（移植樹）：
 *   forms/fTeachRegistry.cpp 的 E1 段（產生器 tools/gen_teach_registry.py EXT_ROWS）只在 IO_CARD_TYPE==PCI1203_IO 登錄 6 個 TECH_PARA：
 *     MOutShuttle1  setEditOutSht1Left／setEditOutSht1Right  ← Tech.iOutShuttle1Left／1Right（引擎 Prod.OutSHT[0].iLeft／iRight）
 *     MOutShuttle2  setEditOutSht2Left／setEditOutSht2Right  ← Tech.iOutShuttle2Left／2Right（Prod.OutSHT[1]）
 *     MCCDY         setEditCCDYSite1x1／setEditCCDYCal1x1    ← Tech.iCCDYSite1x1（檢查位）／Tech.iCalY1x1（校正位）
 *   teach.ini 區段＝[MOutShuttle1]／[MOutShuttle2]／[MCCDY]，鍵＝欄位名（golden TECH_PARA ReadFromFile／SaveToFile，C 路開頁／存檔照常）。
 *   Set／GO 鈕（btnSet*／btnGo*）由 C++ 照 golden SetButton140Click／GoButton140Click 做（WebMotorAccess.cpp MotorAccessResolveTeachButton）；
 *   動之前過 IsCanQuickJogMove 的非 golden 互鎖（forms/fTeach.cpp W906_Teach3AxesHomeOk）：MOutShuttle1／2 要 Index Z 與 In/Out Arm Z
 *   在原點，MCCDY 要 Index Z 在原點。
 *
 * 頁面上的元件：
 *   Shuttle 頁 > Shuttle Pos：Panel28 的 golden 保留欄位 setEditOutSht1Left／1Right／2Left／2Right 與標籤 Label74 "OutShuttle1"／
 *     Label76 "OutShuttle2"（uteach.dfm:4349-4400 Visible=False，golden 從不打開）＋ Panel6 右邊新加的 gbW906OutShuttle（4 顆 Set＋4 顆 GO）；
 *   Index 頁：新加的 gbW906CCDY（MotorCCDY 軸選取鈕＋兩個點的 Set／GO／欄位）。
 *
 * 規則（簡單、fail-safe）：HTML 預設全部 display:none。C++ 實際載入的馬達表（頁面 teachLoadMotors 讀的 /api/struct/motor/config，
 *   全域 teachMotorConfig；來源 teachMotorSrc 必須是 'C++'，?offline=1 的畫面開發模式是 'offline'）裡有那一軸，才顯示那一軸的元件。
 *   來源是靜態檔（C++ 讀不到）、等不到馬達表、回應不對 ⇒ 維持藏著，原因寫在右下角狀態（teachSetInfo）與兩個群組的 data-rule。
 *   不送任何命令、不寫任何檔、不改任何 id／title（title 是 theme.js／teachAttachMotorButtons／teachBindTechButtons 找元件的鍵）。
 *   每次開窗（HT_WIN 關→開）重算一次；馬達表本身是頁面載入時讀一次（同 teachLoadMotors）。
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';

  // AI(W906-TEACH-3AXES) 20261001: 每一軸的元件（欄位寫 input 的 id，顯示／隱藏的是外面那一層 span）
  var AXES = {
    MOutShuttle1: ['Label74', 'setEditOutSht1Left', 'setEditOutSht1Right',
                   'btnSetOutSht1Left', 'btnGoOutSht1Left', 'btnSetOutSht1Right', 'btnGoOutSht1Right'],
    MOutShuttle2: ['Label76', 'setEditOutSht2Left', 'setEditOutSht2Right',
                   'btnSetOutSht2Left', 'btnGoOutSht2Left', 'btnSetOutSht2Right', 'btnGoOutSht2Right'],
    MCCDY:        ['MotorCCDY', 'btnSetCCDYSite1x1', 'btnGoCCDYSite1x1', 'setEditCCDYSite1x1',
                   'btnSetCCDYCal1x1', 'btnGoCCDYCal1x1', 'setEditCCDYCal1x1']
  };
  // 群組：任何一軸在就顯示
  var GROUPS = { gbW906OutShuttle: ['MOutShuttle1', 'MOutShuttle2'], gbW906CCDY: ['MCCDY'] };
  var WAIT_MS = 20000, POLL_MS = 500;

  function $(id) { return document.getElementById(id); }
  function holder(el) {                                    // 欄位外面那一層定位用的 span（產生器的 TEdit 樣板）
    return (el && el.tagName === 'INPUT' && el.parentNode && el.parentNode.tagName === 'SPAN') ? el.parentNode : el;
  }
  function setDisp(id, show) { var el = holder($(id)); if (el) el.style.display = show ? '' : 'none'; return !!el; }
  function info(msg, cls) {
    if (typeof global.teachSetInfo === 'function') { try { global.teachSetInfo(msg, cls); return; } catch (e) {} }
    if (global.console) console.warn('[3axes] ' + msg);
  }

  // ---- 純函式（給測試用）：馬達表 → 哪一軸在 ----
  function compute(cfg) {
    var have = {}, ax = Object.keys(AXES), motors = (cfg && cfg.motors) || [], i, j;
    for (i = 0; i < ax.length; i++) {
      have[ax[i]] = false;
      for (j = 0; j < motors.length; j++) if (motors[j] && motors[j].motorId === ax[i]) { if (motors[j].enabledByDefault !== false) have[ax[i]] = true; break; }   // AI(W906-TEACH-HIDEAXIS) 20261001: Enable=0 hidden too (EastSun 1001)
    }
    return have;
  }

  var ST = { ok: null, reason: '', have: null, missingIds: [], at: 0, seq: 0 };

  function hideAll(reason) {
    Object.keys(AXES).forEach(function (a) { AXES[a].forEach(function (id) { setDisp(id, false); }); });
    Object.keys(GROUPS).forEach(function (g) {
      setDisp(g, false);
      var el = $(g); if (el) el.setAttribute('data-rule', 'W906 三軸教導點未顯示：' + reason);
    });
  }
  function applyHave(have) {
    var lost = [];
    Object.keys(AXES).forEach(function (a) { AXES[a].forEach(function (id) { if (!setDisp(id, have[a])) lost.push(id); }); });
    Object.keys(GROUPS).forEach(function (g) {
      var show = GROUPS[g].some(function (a) { return have[a]; });
      if (!setDisp(g, show)) lost.push(g);
      var el = $(g); if (el) el.removeAttribute('data-rule');
    });
    return lost;
  }

  // AI(W906-TEACH-3AXES) 20261001: 等頁面的 teachLoadMotors 把馬達表綁好（bind() 設 teachMotorSrc／teachMotorConfig）
  function waitConfig(my) {
    return new Promise(function (resolve, reject) {
      var t0 = Date.now();
      (function poll() {
        if (my !== ST.seq) { reject(new Error('superseded')); return; }
        var src = global.teachMotorSrc, cfg = global.teachMotorConfig;
        if (src && src !== 'none') { resolve({ src: src, cfg: cfg }); return; }
        if (Date.now() - t0 > WAIT_MS) { reject(new Error('等了 ' + (WAIT_MS / 1000) + ' 秒還沒有馬達表（teachLoadMotors 沒跑完？）')); return; }
        setTimeout(poll, POLL_MS);
      })();
    });
  }

  function formShow(why) {
    var my = ++ST.seq;
    return waitConfig(my).then(function (r) {
      if (my !== ST.seq) return ST;
      if (r.src !== 'C++' && r.src !== 'offline')
        throw new Error('馬達表不是 C++ 實際載入的（來源 ' + r.src + '）');
      if (!r.cfg || !r.cfg.motors || typeof r.cfg.motors.length !== 'number') throw new Error('馬達表回應沒有 motors');
      var have = compute(r.cfg), lost = applyHave(have);
      ST.ok = true; ST.reason = ''; ST.have = have; ST.missingIds = lost; ST.at = Date.now();
      document.documentElement.setAttribute('data-teach3axes', 'ok');
      if (lost.length) info('三軸教導點：頁面上找不到 ' + lost.join(', ') + '（頁面改版？）', 'warn');
      return ST;
    }).catch(function (e) {
      if (my !== ST.seq) return ST;
      var reason = (e && e.message) || String(e);
      if (reason === 'superseded') return ST;
      hideAll(reason);
      ST.ok = false; ST.reason = reason; ST.have = null; ST.missingIds = []; ST.at = Date.now();
      document.documentElement.setAttribute('data-teach3axes', 'fail');
      var msg = 'MOutShuttle1／2、MCCDY 教導點不顯示：' + reason;
      info(msg, 'warn');
      setTimeout(function () { if (my === ST.seq && ST.ok === false) info(msg, 'warn'); }, 3000);   // 狀態列是共用的，載入時可能被別的初始化蓋掉
      return ST;
    });
  }

  // ---- 開窗時機（HT_WIN），同 ht9045_teach_trayz_c.js ----
  var W = { hosted: false, shown: false, timer: null };
  global.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    if (global.parent === global || ev.source !== global.parent) return;
    W.hosted = true;
    if (W.timer) { clearTimeout(W.timer); W.timer = null; }
    if (m.open && !W.shown) { W.shown = true; formShow('HT_WIN open' + (m.initial ? ' (initial)' : '')); }
    else if (!m.open) W.shown = false;
  });
  function boot() {
    if (global.parent === global) { formShow('standalone'); return; }
    if (!W.hosted && !W.timer) W.timer = setTimeout(function () {
      W.timer = null;
      if (!W.hosted) formShow('no HT_WIN in 3 s');
    }, 3000);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true });
  else boot();

  global.HT9045Teach3Axes = { AXES: AXES, GROUPS: GROUPS, compute: compute, formShow: formShow, state: ST };
})(window);
