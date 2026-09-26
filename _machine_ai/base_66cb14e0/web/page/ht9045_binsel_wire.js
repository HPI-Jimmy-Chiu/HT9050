/* ht9045_binsel_wire.js -- Setup.BinSel.html（golden TfBinSel，cBinSel.cpp V912）C 路接線與 bin 格子。
 * ---------------------------------------------------------------------------
 * Steven 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 資料來源：WS editlist.get tag=BinSelect（FileRW/BinSelect.cpp FileRW_BinSelect_Page）
 *   一般回應 {struct, form, booted, lists, proxies, session, mustSend} 由 ht9045_wire_engine.js 套上表單元件；
 *   回應多一個 bin：{activeTag, iTestBinCount, eTrayCount, eBinNotUse, eBinSetting, s6TrayName,
 *   tags[7]:{tag,name,tab,pageIndex,file,loaded,panelEnabled,editable,lists{30 組},panel{iErrorT6, 各欄陣列, BackT6PosTrayRow}}}。
 *   格子由「作用中那一組」的 panel 畫；編輯語意逐條照 golden（行號見各函式註解）；
 *   存檔時 window.GB_EXTRA_SAVE 把 panel 原樣（完整欄位、長度）送回（FileRW_BinSelect_Save／ParsePanel），
 *   後端套進 MyBinPanel[tag] → golden InitDataToEdit(tag) → spbSaveClick。
 *
 * golden 的編輯規則有很多取決於機台設定（Prod.iTrayType、bCanLinkT6、IniConfig.bG07MultiColorForFailBin、
 * USE_AUTO_RETEST、UNLOADER_ART、Prod.bD22SupportMultiDoubleContact、Prod.bLowYieldAlarmByBin、
 * TestIF_File.iAutoClean_Function、CosFunction.b… 等）。回應目前沒有這些值：後端若之後在 bin.rules 帶上
 * （欄位名見 RULE_KEYS），本檔就照它；沒帶的用推定值，並在畫面上方列出「推定」的項目 —— 不假裝知道。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  // golden cBinSel.cpp:53-82 eBinSettingItems（1..24），名稱照 TMyBinPanel ctor :330-357
  //   kind: scan=Scanning（永遠灰）、dc=Double Contact、v=打 V 的 Enable、n=數值（en=對應的 Enable）、ac=Auto clean 數值
  var ROWS = [
    null,
    { y: 1,  name: 'Scanning',               kind: 'scan' },
    { y: 2,  name: 'Double Contact',         kind: 'dc', f: 'i2Contact' },
    { y: 3,  name: 'Cons. Fail',             kind: 'v',  f: 'bConFail' },
    { y: 4,  name: 'Yield % Bin',            kind: 'v',  f: 'bPersentEnable', grp: 'persent' },
    { y: 5,  name: 'Yield Ignore Cnt',       kind: 'n',  f: 'iPersentIgnore', en: 'bPersentEnable', grp: 'persent' },
    { y: 6,  name: 'Yield % Number',         kind: 'n',  f: 'dPersentNumber', en: 'bPersentEnable', grp: 'persent' },
    { y: 7,  name: 'Count Bin',              kind: 'v',  f: 'bCountEnable' },
    { y: 8,  name: 'Count Ignored',          kind: 'n',  f: 'iCountIgnore', en: 'bCountEnable' },
    { y: 9,  name: 'Count Number',           kind: 'n',  f: 'iCountNumber', en: 'bCountEnable' },
    { y: 10, name: 'Spc. Bin By Arm',        kind: 'v',  f: 'bSpecialBinByArm' },
    { y: 11, name: 'Spc. Cnt By Arm',        kind: 'n',  f: 'iSpecialBinCountByArm', en: 'bSpecialBinByArm' },
    { y: 12, name: 'Spc. Bin By Socket',     kind: 'v',  f: 'bSpecialBinBySocket' },
    { y: 13, name: 'Spc. Cnt By Socket',     kind: 'n',  f: 'iSpecialBinCountBySocket', en: 'bSpecialBinBySocket' },
    { y: 14, name: 'Low Yield',              kind: 'v',  f: 'bLowYield', grp: 'ly' },
    { y: 15, name: 'By Arm Yield',           kind: 'v',  f: 'bArmYield', grp: 'ly' },
    { y: 16, name: 'By Site Yield',          kind: 'v',  f: 'bSiteYield', grp: 'ly' },
    { y: 17, name: 'By Bin Cleaning',        kind: 'ac', f: 'iAutoCleanByBin' },
    { y: 18, name: 'By Site Cleaning',       kind: 'ac', f: 'iAutoCleanBySite' },
    { y: 19, name: 'By Bin Site Gap',        kind: 'v',  f: 'bSpecBinBySiteCompareEnable', grp: 'cmp' },
    { y: 20, name: 'Count Ignore',           kind: 'n',  f: 'iSpecBinBySiteCompareIgnore', en: 'bSpecBinBySiteCompareEnable', grp: 'cmp' },
    { y: 21, name: 'Site Gap %',             kind: 'n',  f: 'dSpecBinBySiteComparePercent', en: 'bSpecBinBySiteCompareEnable', grp: 'cmp' },
    { y: 22, name: 'By Arm By Bin Site Gap', kind: 'v',  f: 'bSpecBinByArmPerSiteCompareEnable', grp: 'cmp' },
    { y: 23, name: 'Count Ignore',           kind: 'n',  f: 'iSpecBinByArmPerSiteCompareIgnore', en: 'bSpecBinByArmPerSiteCompareEnable', grp: 'cmp' },
    { y: 24, name: 'By Arm Site Gap%',       kind: 'n',  f: 'dSpecBinByArmPerSiteComparePercent', en: 'bSpecBinByArmPerSiteCompareEnable', grp: 'cmp' }
  ];
  // golden eTrayNameFunc（:98-105）：托盤狀態欄
  var COL_LINK = 0, COL_PASS = 1, COL_ERROR = 2, COL_CATER = 3, COL_ART = 4;
  var STAT_CAPS = ['Link', 'Failed', 'Error', 'CateR', 'Retest'];     // ctor :323-327
  // golden eTrayColorMap（:106-117）
  var CL = ['cWhite', 'cGreen', 'cRed', 'cYellow', 'cPurple', 'cBlue', 'cGray', 'cSilver', 'cBtnFace', 'cOlive'];
  var eCLWhite = 0, eCLGreen = 1, eCLRed = 2, eCLBlue = 5, eCLGray = 6, eCLSilver = 7, eCLBtnFace = 8, eCLOlive = 9;
  // cbTestModeChange（:2147-2230）的文字 → PageControl1 分頁索引（tags[].pageIndex 對到 tag）
  var MODE_PAGE = { 'Normal': 0, 'Re-Test': 1, 'Off-Line': 2, 'ART Normal': 3, 'ART Re-Test': 4, 'MRT Normal': 5, 'MRT Re-Test': 6 };
  // 後端若帶 bin.rules，照這些欄位（缺的欄位才推定）
  var RULE_KEYS = ['trayNotUse', 'trayCanUse', 'canLinkT6', 'multiColorFail', 'artInstall', 'sltSummary', 'sckArt',
                   'sckArtSortMode1', 'unloaderArt', 'iAutoRight', 'qaT3Pos', 'multiDoubleContact', 'doubleContactMax',
                   'fromYieldForm', 'cmpActive', 'lowYieldByBin', 'autoClean', 'countNeedsPassword', 'oneCycle',
                   'icInMachine', 'bin1CanNotInFix', 'posFix1Row', 'offLineTag', 'iFixRight', 'testRunModeTag',
                   'lockFuncRows', 'lockTrayRows', 'kyecForceConsFail', 'modeItems'];

  var BIN = null;        // 最近一次 editlist.get 的 bin
  var PROX = {};         // 最近一次的 proxies
  var R = null;          // 規則（見 buildRules）
  var GUESS = [];        // 推定的規則名稱（畫面上列出）
  var cur = null;        // {tag, meta, P}：作用中那一組（P = panel 的工作副本）
  var N = 0, T = 0, NOTUSE = 25, SETTING = 26;
  var BIN_PAGE_SIZE = 32, binPage = 0, binPageCount = 1, autoHideOff = false;
  var specOpened = false;      // 使用者按過 btnSettingSpecificBin（存檔帶 actions）
  var keepModeText = null;     // 存檔後重讀時留在同一個 Control Bin Point（golden 存檔不關表單）
  // golden SeteConsFail(-1)／SetePersentEnable(-1) 的 static bool bEnable=true（:3374、:3423）：第一次全開、之後全關
  var STATIC_ENABLE = { consFail: true, persent: true };
  var drag = null;

  function $(id) { return document.getElementById(id); }
  function clone(o) { return JSON.parse(JSON.stringify(o)); }
  function say(msg, colour) { if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour); }
  function tagMeta(tag) { var a = (BIN && BIN.tags) || []; for (var i = 0; i < a.length; i++) if (a[i].tag === tag) return a[i]; return null; }
  function tagOfPage(idx) { var a = (BIN && BIN.tags) || []; for (var i = 0; i < a.length; i++) if (a[i].pageIndex === idx) return a[i].tag; return -1; }
  function names() { return (BIN && BIN.s6TrayName) || []; }
  function anyTag(fn) { var a = (BIN && BIN.tags) || []; for (var i = 0; i < a.length; i++) if (fn(a[i])) return true; return false; }

  // ---------------------------------------------------------------------------
  //  規則：bin.rules 優先；沒有的推定（推定的列在 GUESS，畫面上看得到）
  // ---------------------------------------------------------------------------
  function buildRules() {
    var given = (BIN && BIN.rules) || {}, r = {}, nm = names(), P = cur ? cur.P : null, L = cur ? cur.meta.lists || {} : {};
    GUESS = [];
    function take(k, guess) {
      if (given[k] !== undefined) { r[k] = given[k]; return; }
      r[k] = guess; GUESS.push(k);
    }
    var isAuto = function (i) { return /^Auto\d/.test(nm[i] || ''); };
    var fix1 = nm.indexOf('Fix1'), bulk = nm.indexOf('BulkBox'), autoRight = -1, fixRight = -1;
    for (var i = 0; i < T; i++) { if (isAuto(i)) autoRight = i; if (/^Fix\d/.test(nm[i] || '')) fixRight = i; }
    var fill = function (v) { var a = []; for (var k = 0; k < T; k++) a.push(typeof v === 'function' ? v(k) : v); return a; };
    take('trayNotUse', fill(false));                                   // Prod.iTrayType[i]==tNotUse
    take('trayCanUse', fill(function (k) { return k !== bulk; }));      // bCheckTrayCanUse（:2844）
    // bCanLinkT6（ReadFile :1154-1185）：Auto1、Fix1、BulkBox 不行；bAutoTrayLink=false 時 Auto 都不行；檔案已 Link 的一定可
    take('canLinkT6', fill(function (k) {
      if (k === 0 || k === fix1 || k === bulk) return false;
      if (anyTag(function (t) { return t.panel && t.panel.bT6Link && t.panel.bT6Link[k]; })) return true;
      return !isAuto(k);
    }));
    take('multiColorFail', anyTag(function (t) { return (t.panel.iT6IsFail || []).some(function (v) { return v > 1; }); }));
    take('artInstall', anyTag(function (t) { return (t.name === 'RT_ART' || t.name === 'FT_ART') && t.loaded; }));
    take('sltSummary', false);
    take('sckArt', false);
    take('sckArtSortMode1', false);
    take('unloaderArt', fill(function (k) { return isAuto(k); }));
    take('iAutoRight', autoRight);
    take('qaT3Pos', -1);
    take('multiDoubleContact', !!(P && (P.i2Contact || []).some(function (v) { return v !== 0; })));
    take('doubleContactMax', 2);
    // InitDataToEdit：bByBinAlarmFromYieldForm 時 Persent 三列清成 ""（:4355-4368）→ 清單是空的
    take('fromYieldForm', N > 0 && (L.sBinEnableFail || []).length === 0);
    // InitDataToEdit：bBySiteByBinPercentCompare==false（或 FromYieldForm）時 Compare 六列清成 ""（:4629-4652）
    take('cmpActive', (L.sSpecBinBySiteCompareEnable || []).length > 0);
    // Prod.bLowYieldAlarmByBin==false 時清單寫 "0" 但 panel 保留（:4513-4526）→ panel 有 true 而清單是 0 就是 false
    var lyGuess = false;
    if (P) {
      [['bLowYield', 'sLowYield'], ['bArmYield', 'sArmYield'], ['bSiteYield', 'sSiteYield']].forEach(function (p) {
        (P[p[0]] || []).forEach(function (v, k) { if (v && (L[p[1]] || [])[k] === '1') lyGuess = true; });
      });
    }
    take('lowYieldByBin', lyGuess);
    take('autoClean', !!(P && ((P.iAutoCleanByBin || []).some(function (v) { return v; }) ||
                              (P.iAutoCleanBySite || []).some(function (v) { return v; }))));
    take('countNeedsPassword', false);                                 // 推定 false：後端會照 golden CheckCountSettingPassword 擋
    take('oneCycle', false);
    take('icInMachine', false);
    take('bin1CanNotInFix', false);
    take('posFix1Row', SETTING + (fix1 >= 0 ? fix1 : T));
    take('offLineTag', 2);
    take('iFixRight', fixRight);
    take('testRunModeTag', BIN ? BIN.activeTag : -1);
    take('lockFuncRows', false);
    take('lockTrayRows', false);
    take('kyecForceConsFail', false);
    take('modeItems', null);
    r.bulk = bulk;
    return r;
  }

  // ---------------------------------------------------------------------------
  //  golden InitDataToEdit 對 MyBinPanel 的副作用（:4471-4508）：pass 盤的 bin 取消 Spc. By Arm／Socket
  // ---------------------------------------------------------------------------
  function trayOf(x) { var r = cur.P.BackT6PosTrayRow[x]; return r >= SETTING ? r - SETTING : -1; }
  function assignedToTray(x) { return trayOf(x) >= 0; }                 // BackT6PosTray[X][j]==1, j>=eBinSetting
  function trayFail(x) { var t = trayOf(x); return t >= 0 && cur.P.iT6IsFail[t] > 0; }
  function initDataToEdit() {
    var P = cur.P;
    for (var i = 0; i < N; i++) {
      var t = trayOf(i);
      if (t >= 0 && P.iT6IsFail[t] === 0) { P.bSpecialBinByArm[i] = false; P.bSpecialBinBySocket[i] = false; }
    }
  }

  // ---------------------------------------------------------------------------
  //  鍵盤（golden fQwertyKey->ShowQwertyKey(edInput, …)：edInput 先放預設值；取消＝保留預設值）
  // ---------------------------------------------------------------------------
  function ask(preset, dbl, dp, lo, hi, cb) {
    if (typeof HTQwerty === 'undefined') { cb(String(preset)); return; }
    var tmp = document.createElement('input'); tmp.value = String(preset);
    var done = false, fin = function (v) { if (done) return; done = true; cb(v); };
    HTQwerty.show(tmp, dbl ? HTQwerty.N.DOUBLE : HTQwerty.N.INTEGER,
                  { dp: dp, checkRange: true, min: lo, max: hi,
                    onCommit: function (v) { fin(v); }, onAbort: function () { fin(tmp.value); } });
  }
  function atoi(s) { var n = parseInt(s, 10); return isNaN(n) ? 0 : n; }
  function atof(s) { var n = parseFloat(s); return isNaN(n) ? 0 : n; }
  function fileVal(f, x) { var p = cur.meta.panel; return p && p[f] ? p[f][x] : 0; }   // golden BinSelect[tag].… ＝檔案值

  // ---------------------------------------------------------------------------
  //  golden Sete*（:3319-3881）。X>=0 單一 bin；X<0 由列名點擊（mtTrayItemMouseUp）來。done() 在值定了之後呼叫。
  // ---------------------------------------------------------------------------
  function sete(y, X, done) {
    var P = cur.P, i;
    switch (y) {
      case 2:   // SeteDoubleContact :3319（CC_ASE_CL 權限 132 未模擬）
        if (!R.multiDoubleContact) return done(true);
        return ask(X >= 0 ? P.i2Contact[X] : '', false, 0, 0, R.doubleContactMax, function (v) {
          if (X >= 0) P.i2Contact[X] = atoi(v); else for (i = 0; i < N; i++) P.i2Contact[i] = atoi(v);
          done(true);
        });
      case 3:   // SeteConsFail :3372
        if (X >= 0) {
          if (assignedToTray(X)) {
            if (trayFail(X)) P.bConFail[X] = R.kyecForceConsFail ? true : !P.bConFail[X];
            else P.bConFail[X] = false;
          }
        } else {
          for (i = 0; i < N; i++) if (assignedToTray(i)) P.bConFail[i] = trayFail(i) ? STATIC_ENABLE.consFail : false;
          STATIC_ENABLE.consFail = false;
        }
        return done(true);
      case 4:   // SetePersentEnable :3421
        if (X >= 0) {
          if (assignedToTray(X)) P.bPersentEnable[X] = !P.bPersentEnable[X];
          if (P.bPersentEnable[X]) {
            P.bCountEnable[X] = false;
            return ask(fileVal('dPersentNumber', X), true, 2, 0, 100, function (v) {
              P.dPersentNumber[X] = atof(v);
              if (P.iPersentIgnore[X] === 0) P.iPersentIgnore[X] = 1;
              done(true);
            });
          }
          P.dPersentNumber[X] = 0;
          return done(true);
        }
        for (i = 0; i < N; i++) if (assignedToTray(i)) P.bPersentEnable[i] = STATIC_ENABLE.persent;
        if (STATIC_ENABLE.persent) {
          STATIC_ENABLE.persent = false;
          return ask('', true, 2, 0, 100, function (v) {
            for (var k = 0; k < N; k++) {
              P.bCountEnable[k] = false; P.dPersentNumber[k] = atof(v);
              if (P.iPersentIgnore[k] === 0) P.iPersentIgnore[k] = 1;
            }
            done(true);
          });
        }
        for (i = 0; i < N; i++) P.dPersentNumber[i] = 0;
        return done(true);
      case 5:  return seteNum(X, 'bPersentEnable', 'iPersentIgnore', false, 1, 100000, done);   // :3492
      case 6:  return seteNum(X, 'bPersentEnable', 'dPersentNumber', true, 0, 100, done);       // :3511
      case 7:   // SeteCountEnable :3530
        if (X < 0) return done(true);
        if (!countPassword()) return done(false);
        if (assignedToTray(X)) P.bCountEnable[X] = !P.bCountEnable[X];
        if (P.bCountEnable[X]) {
          P.bPersentEnable[X] = false;
          return ask(fileVal('iCountNumber', X), false, 0, 1, 100000, function (v) { P.iCountNumber[X] = Math.trunc(atof(v)); done(true); });
        }
        P.iCountNumber[X] = 0;
        return done(true);
      case 8:  if (X >= 0 && !countPassword()) return done(false);
               return seteNum(X, 'bCountEnable', 'iCountIgnore', false, 1, 100000, done);         // :3561
      case 9:  if (X >= 0 && !countPassword()) return done(false);
               return seteNum(X, 'bCountEnable', 'iCountNumber', false, 1, 100000, done, true);   // :3583（atof → int）
      case 10: return seteSpecial(X, 'bSpecialBinByArm', 'iSpecialBinCountByArm', done);         // :3605
      case 11: return seteNum(X, 'bSpecialBinByArm', 'iSpecialBinCountByArm', false, 1, 100000, done, true);        // :3635
      case 12: return seteSpecial(X, 'bSpecialBinBySocket', 'iSpecialBinCountBySocket', done);   // :3654
      case 13: return seteNum(X, 'bSpecialBinBySocket', 'iSpecialBinCountBySocket', false, 1, 100000, done);         // :3684
      case 14: case 15: case 16:   // SeteLowYield／ArmYield／SiteYield :3703-3734
        if (X >= 0 && P.BackT6PosTrayRow[X] !== NOTUSE) { var f = ROWS[y].f; P[f][X] = !P[f][X]; }
        return done(true);
      case 17: case 18:   // SeteAutoCleanByBin／BySite :3736-3764
        if (X >= 0 && R.autoClean) {
          var fa = ROWS[y].f;
          return ask(P[fa][X], false, 0, 0, 100000, function (v) { P[fa][X] = atoi(v); done(true); });
        }
        return done(true);
      case 19: case 22:   // SeteSpecBinBySiteCompareEnable／ByArmPerSite… :3766、:3824（切換後一定跳鍵盤設 %）
        if (X < 0) return done(true);
        var fe = ROWS[y].f, fp = ROWS[y + 2].f;
        if (assignedToTray(X)) P[fe][X] = !P[fe][X];
        return ask(fileVal(fp, X), true, 2, 1, 100, function (v) { P[fp][X] = atof(v); done(true); });
      case 20: case 23: return seteNum(X, ROWS[y].en, ROWS[y].f, false, 1, 100000, done);         // :3786、:3844
      case 21: case 24: return seteNum(X, ROWS[y].en, ROWS[y].f, true, 0, 100, done);             // :3805、:3863
    }
    return done(true);
  }
  // Ignore／Number 類：Enable 開著才跳鍵盤（預設現值），關著就歸 0
  function seteNum(X, en, f, dbl, lo, hi, done, truncFromFloat) {
    var P = cur.P;
    if (X < 0) return done(true);
    if (P[en][X]) {
      return ask(P[f][X], dbl, dbl ? 2 : 0, lo, hi, function (v) {
        P[f][X] = dbl ? atof(v) : (truncFromFloat ? Math.trunc(atof(v)) : atoi(v));
        done(true);
      });
    }
    P[f][X] = 0;
    return done(true);
  }
  // SeteSpecialBinByArm／BySocket：fail 盤才可切；開了跳鍵盤（預設檔案值）
  function seteSpecial(X, fb, fc, done) {
    var P = cur.P;
    if (X < 0) return done(true);
    if (assignedToTray(X)) P[fb][X] = trayFail(X) ? !P[fb][X] : false;
    if (P[fb][X]) return ask(fileVal(fc, X), false, 0, 1, 100000, function (v) { P[fc][X] = Math.trunc(atof(v)); done(true); });
    P[fc][X] = 0;
    return done(true);
  }
  // golden CheckCountSettingPassword（:6717）：網頁沒有密碼框 → 需要密碼時視同密碼錯誤
  function countPassword() {
    if (!R.countNeedsPassword) return true;
    say('Count Bin／Ignored／Number 需要密碼（golden CheckCountSettingPassword），網頁端沒有密碼框 → 視同密碼錯誤，不改。', '#ffcc66');
    return false;
  }

  // ---------------------------------------------------------------------------
  //  事件（golden 滑鼠事件）
  // ---------------------------------------------------------------------------
  function editable() { return !!(cur && cur.meta && cur.meta.editable); }
  function warning() { var w = $('labWarning'); return w ? w.textContent : ''; }
  function afterEdit() { initDataToEdit(); render(); }

  // mtBinSelectMouseDown（:3010-3317）
  function cellDown(y, X) {
    if (!editable()) return;
    // OffT＋CC_SCC 密碼（:3016-3031）未模擬
    if (R.oneCycle && warning() !== '' && y !== 3) return;          // :3033-3048（CC_JCET／TERAPOWER／SIGURD_PeiXing 例外未模擬）
    if (X < 0 || X >= N) return;
    if (y >= SETTING && !R.trayCanUse[y - SETTING]) return;
    if (y - SETTING === R.bulk) return;                             // :3057 eBulkBox
    if (!R.cmpActive && y >= 19 && y < 25) return;                   // :3061-3068
    if (R.fromYieldForm && y >= 4 && y < 6) return;                  // :3070-3074（golden 只擋 4、5 兩列，照翻）
    if (R.lockFuncRows && y > 0 && y < NOTUSE) return;               // :3075-3096 Yield Control 權限 148
    if (R.lockTrayRows && y >= NOTUSE) return;                       // :3097-3102 SCK RMS／Korea 149
    if (y > 0 && y < NOTUSE) {
      if (R.oneCycle && R.icInMachine) return;                       // :3107-3113
      sete(y, X, function (ok) { if (ok) afterEdit(); });
      return;
    }
    if (y < NOTUSE || y >= SETTING + T) return;
    if (y > SETTING && cur.P.bT6Link[y - SETTING]) return;           // :3294-3298（golden 用 >，第一盤不檢查，照翻）
    if (R.oneCycle && warning() !== '') return;                      // :3299 CanChangeData(true)（以 labWarning 推定）
    drag = { y: y, sx: X, ex: X };
    paintDrag();
  }
  // mtBinSelectMouseMove（:3882）→ ShowBinTray（:4021）：拖拉中的範圍黃色
  function cellEnter(y, X) {
    if (!drag || y < NOTUSE) return;
    drag.ex = X; paintDrag();
  }
  function paintDrag() {
    var tds = document.querySelectorAll('#binBody td.bc.drag');
    for (var k = 0; k < tds.length; k++) tds[k].classList.remove('drag');
    if (!drag) return;
    var lo = Math.min(drag.sx, drag.ex), hi = Math.max(drag.sx, drag.ex);
    var tr = document.querySelector('#binBody tr[data-y="' + drag.y + '"]');
    if (!tr) return;
    for (var x = lo; x <= hi; x++) { var td = tr.querySelector('td.bc[data-b="' + x + '"]'); if (td) td.classList.add('drag'); }
  }
  // mtBinSelectMouseUp（:3908）→ SetBinTray（:3982）
  function mouseUp() {
    if (!drag) return;
    var d = drag; drag = null;
    setBinTray(d.y, Math.min(d.sx, d.ex), Math.max(d.sx, d.ex));
    afterEdit();
  }
  function setBinTray(y, s, e) {
    var P = cur.P;
    e = Math.min(e, N - 1);                                           // btnSetAll2NotUse 的 iEndX=iTestBinCount（多一格）
    for (var i = s; i <= e; i++) {
      if (R.bin1CanNotInFix && cur.tag !== R.offLineTag && s === 1 && y >= R.posFix1Row) y = NOTUSE;   // :3989-3994
      P.BackT6PosTrayRow[i] = y;
      P.bConFail[i] = (y !== NOTUSE && P.iT6IsFail[y - SETTING] > 0);   // :4005-4014
    }
  }
  // mtTrayNameMouseDown（:2879-3008）
  function statDown(X, t) {
    var P = cur.P, i;
    if (!editable()) return;
    if (X < 0 || X > COL_ART || t < 0 || t >= T) return;
    if (!R.trayCanUse[t]) return;
    if (X === COL_PASS) {
      if (!P.bT6Link[t]) {
        if (P.iErrorT6 === t) P.iT6IsFail[t] = 1;
        else if (R.multiColorFail) { P.iT6IsFail[t]++; if (P.iT6IsFail[t] >= 5) P.iT6IsFail[t] = 0; }
        else P.iT6IsFail[t] = P.iT6IsFail[t] === 0 ? 1 : 0;
      }
    } else if (X === COL_LINK) {
      if (R.canLinkT6[t]) {
        P.bT6Link[t] = !P.bT6Link[t];
        if (P.bT6Link[t] && P.iErrorT6 === t && t > 0) { P.iErrorT6 = t - 1; P.iT6IsFail[t - 1] = P.iT6IsFail[t]; }
        for (i = 0; i < N; i++) if (P.bT6Link[t] && P.BackT6PosTrayRow[i] === t + SETTING) P.BackT6PosTrayRow[i] = NOTUSE;
      }
    } else if (X === COL_ERROR) {
      if (!P.bT6Link[t]) { P.iT6IsFail[t] = 1; P.iErrorT6 = t; }
    } else if (X === COL_ART) {
      if (R.artInstall && t <= R.iAutoRight && P.iT6IsFail[t] > 0) P.bT6ART[t] = R.unloaderArt[t] ? !P.bT6ART[t] : false;
    } else if (X === COL_CATER) {
      if ((R.artInstall && R.sckArt && R.sckArtSortMode1) || R.sltSummary) P.bT6CateR[t] = P.iT6IsFail[t] > 0 ? !P.bT6CateR[t] : false;
      else P.bT6CateR[t] = false;
    }
    for (i = 0; i < N; i++) if (P.BackT6PosTrayRow[i] === SETTING + t) P.bConFail[i] = P.iT6IsFail[t] > 0;   // :2990-2998
    for (i = 1; i < T; i++) if (R.canLinkT6[i] && P.bT6Link[i]) P.iT6IsFail[i] = P.iT6IsFail[i - 1];         // :2999-3006 eAuto2..
    afterEdit();
  }
  // mtTrayItemMouseUp（:6473-6677）：點列名
  function nameUp(y) {
    if (!editable()) return;
    if (y < 0 || y >= SETTING + T || N < 17) return;
    if (R.oneCycle && R.icInMachine) return;
    if (y >= NOTUSE) { setBinTray(y, 0, N - 1); afterEdit(); return; }
    sete(y, -1, function () { afterEdit(); });
  }

  // ---------------------------------------------------------------------------
  //  畫面（golden InitDataToEdit／mtTrayNameSetColor 的顏色與文字）
  // ---------------------------------------------------------------------------
  function numText(v) { return String(v); }
  function funcCell(r, x) {
    var P = cur.P, v;
    if (r.kind === 'scan') return ['', eCLGray];
    if (r.kind === 'dc') {
      if (!R.multiDoubleContact) return ['', eCLGray];
      v = P.i2Contact[x]; return [numText(v), v ? eCLOlive : eCLBtnFace];
    }
    if (r.grp === 'persent' && R.fromYieldForm) return ['', eCLGray];
    if (r.grp === 'ly' && !R.lowYieldByBin) return ['', eCLGray];
    if (r.grp === 'cmp' && !R.cmpActive) return ['', eCLGray];
    if (r.kind === 'v') return P[r.f][x] ? ['V', eCLOlive] : ['', eCLBtnFace];
    if (r.kind === 'n') return P[r.en][x] ? [numText(P[r.f][x]), eCLOlive] : ['', eCLBtnFace];
    if (r.kind === 'ac') {
      if (!R.autoClean) return ['', eCLGray];
      v = P[r.f][x]; return v ? [numText(v), eCLOlive] : ['', eCLBtnFace];
    }
    return ['', eCLBtnFace];
  }
  function trayCell(y, x) {
    var P = cur.P, t = y - SETTING, asg = P.BackT6PosTrayRow[x] === y;
    if (y === NOTUSE) return [String(x), asg ? eCLSilver : eCLWhite, asg];
    if (R.trayNotUse[t]) return ['', eCLBtnFace, asg];
    if (asg) return [String(x), P.iT6IsFail[t] > 0 ? eCLRed : eCLGreen, asg];
    return [String(x), eCLWhite, asg];
  }
  function statCells(t) {
    var P = cur.P, out = [];
    if (R.trayNotUse[t]) { for (var k = 0; k < 5; k++) out.push(['', eCLBtnFace]); return out; }
    out[COL_LINK] = (P.bT6Link[t] && R.canLinkT6[t]) ? ['Linked', eCLGreen] : ['', eCLWhite];
    if (R.qaT3Pos >= 0 && t + 1 === R.qaT3Pos) out[COL_PASS] = ['QA', eCLBlue];
    else if (P.iT6IsFail[t] > 0) out[COL_PASS] = ['Failed', Math.min(P.iT6IsFail[t] + 1, 9)];
    else out[COL_PASS] = ['Pass', eCLGreen];
    out[COL_ERROR] = ['Error', P.iErrorT6 === t ? eCLRed : eCLWhite];
    if (R.artInstall || R.sltSummary) {
      out[COL_ART] = t <= R.iAutoRight ? ['Retest', (R.unloaderArt[t] && P.bT6ART[t]) ? eCLRed : eCLWhite] : ['', eCLGray];
      out[COL_CATER] = ['CateR', (R.sckArt && (R.sckArtSortMode1 || R.sltSummary) && P.bT6CateR[t]) ? eCLRed : eCLWhite];
    } else {
      out[COL_ART] = ['', eCLWhite]; out[COL_CATER] = ['', eCLWhite];
    }
    return out;
  }
  function esc(s) { return String(s).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); }
  function td(cls, text, attrs) { return '<td class="' + cls + '"' + (attrs || '') + '>' + esc(text) + '</td>'; }
  function pageAttr(x) { return ' data-page="' + Math.floor(x / BIN_PAGE_SIZE) + '"'; }

  function render() {
    if (!cur) return;
    var head = '<th class="colName">Items</th>';
    STAT_CAPS.forEach(function (s) { head += '<th class="stat">' + s + '</th>'; });
    for (var x = 0; x < N; x++) head += '<th class="bhdr"' + pageAttr(x) + '>' + x + '</th>';
    $('headRow').innerHTML = head;
    var span = 1 + 5 + N, h = '<tr class="sect"><td colspan="' + span + '">功能監控設定</td></tr>';
    var blank = ''; for (var k = 0; k < 5; k++) blank += '<td class="stat cGray"></td>';
    for (var y = 1; y < NOTUSE; y++) {
      var r = ROWS[y], cells = '', off = true;
      for (x = 0; x < N; x++) {
        var c = funcCell(r, x);
        if (c[1] !== eCLGray && c[1] !== eCLBtnFace) off = false;
        cells += td('bc ' + CL[c[1]], c[0], ' data-b="' + x + '"' + pageAttr(x));
      }
      h += '<tr data-y="' + y + '" data-off="' + (off ? 1 : 0) + '"><td class="colName" title="' + esc(r.name) + '（點列名＝golden mtTrayItemMouseUp）">' +
           esc(r.name) + '</td>' + blank + cells + '</tr>';
    }
    h += '<tr class="sect"><td colspan="' + span + '">托盤 Bin 指派（每個 bin 只屬一列；拖拉＝SetBinTray）</td></tr>';
    for (y = NOTUSE; y < SETTING + T; y++) {
      var t = y - SETTING, name = y === NOTUSE ? 'Not in use' : (R.trayNotUse[t] ? '' : (names()[t] || ('Tray' + t)));
      var st = y === NOTUSE ? null : statCells(t), sc = '';
      if (st) st.forEach(function (c, i) { sc += td('stat ' + CL[c[1]], c[0], ' data-col="' + i + '"'); });
      else sc = blank;
      cells = '';
      for (x = 0; x < N; x++) {
        var tc = trayCell(y, x);
        cells += td('bc ' + CL[tc[1]] + (tc[2] ? ' asg' : ''), tc[0], ' data-b="' + x + '"' + pageAttr(x));
      }
      h += '<tr data-y="' + y + '" data-tray="' + (y === NOTUSE ? -1 : t) + '" data-off="0"><td class="colName">' + esc(name) + '</td>' + sc + cells + '</tr>';
    }
    $('binBody').innerHTML = h;
    $('binTable').classList.toggle('ro', !editable());
    applyAutoHide();
    renderTabs();
    setBinPage(Math.min(binPage, Math.max(0, binPageCount - 1)));
  }
  function applyAutoHide() {
    var trs = document.querySelectorAll('#binBody tr[data-y]');
    for (var i = 0; i < trs.length; i++) trs[i].classList.toggle('rowAutoHidden', autoHideOff && trs[i].getAttribute('data-off') === '1');
    $('btnAutoHide').textContent = autoHideOff ? '顯示停用列' : '自動隱藏停用列';
  }
  function setBinPage(p) {
    binPage = p;
    $('binPageStyle').textContent = binPageCount > 1 ? '#binTable [data-page]:not([data-page="' + p + '"]){display:none;}' : '';
    var bs = document.querySelectorAll('#pgbar .pgtab');
    for (var i = 0; i < bs.length; i++) bs[i].classList.toggle('act', +bs[i].getAttribute('data-p') === p);
  }
  function renderTabs() {
    var bar = $('pgbar');
    binPageCount = Math.ceil(N / BIN_PAGE_SIZE);
    if (binPageCount <= 1) { bar.style.display = 'none'; bar.innerHTML = ''; return; }
    bar.style.display = 'flex';
    var h = '<span class="lbl">Bin（每頁 ' + BIN_PAGE_SIZE + '）：</span>';
    for (var p = 0; p < binPageCount; p++) {
      var lo = p * BIN_PAGE_SIZE, hi = Math.min(N - 1, lo + BIN_PAGE_SIZE - 1);
      h += '<button type="button" class="pgtab" data-p="' + p + '">' + lo + '–' + hi + '</button>';
    }
    bar.innerHTML = h;
  }
  // golden 只顯示作用中那一個 TabSheet（cbTestModeChange TabVisible）
  function renderTabsheet() {
    var m = cur.meta, caps = ['Normal', 'Re-Test', 'Off-Line', 'ART Normal', 'ART Re-Test', 'MRT Normal', 'MRT Re-Test'];
    $('tsbar').innerHTML = '<div class="tstab" title="' + esc(m.tab + ' → ' + m.file) + '">' + esc(caps[m.pageIndex] || m.tab) +
      '<span style="font-weight:normal;color:#567;margin-left:8px;font-size:11px;">' + esc(m.file) + '</span>' +
      (m.editable ? '' : '<span class="ro">（不可修改：' + (m.panelEnabled ? '分頁權限' : 'Bin 面板停用') + '）</span>') + '</div>';
    $('bsRules').textContent = GUESS.length ? '規則推定（後端未提供 bin.rules）：' + GUESS.join(', ') : '';
    $('bsRules').title = '這些 golden 條件目前回應裡沒有，頁面用推定值；存檔時後端照 golden 重算（InitDataToEdit）。';
  }

  // 換成某一組（golden cbTestModeChange 先 ReadFile：沒存的修改丟掉，改看該組的檔案值）
  function selectTag(tag) {
    var m = tagMeta(tag);
    if (!m) { cur = null; $('binBody').innerHTML = ''; $('headRow').innerHTML = ''; return; }
    cur = { tag: tag, meta: m, P: clone(m.panel) };
    R = buildRules();
    renderTabsheet();
    render();
  }

  // cbTestMode 的選項：golden 建構子（:1080-1098）依設定增刪；回應沒帶 Items → 用 tags[].loaded 推（ART／MRT）
  function fixModeOptions(serverText) {
    var sel = $('cbTestMode'); if (!sel) return;
    var items = (R && R.modeItems) || null;
    if (!items) {
      var art = tagMeta(3), mrt = tagMeta(5);
      items = ['Normal', 'Re-Test', 'Off-Line'];
      if (art && art.loaded) items.push('ART Re-Test', 'ART Normal');
      if (mrt && mrt.loaded) items.push('MRT Re-Test', 'MRT Normal');
    }
    if (serverText && items.indexOf(serverText) < 0) items.push(serverText);
    sel.innerHTML = items.map(function (s) { return '<option>' + esc(s) + '</option>'; }).join('');
    var want = keepModeText && items.indexOf(keepModeText) >= 0 ? keepModeText : serverText;
    for (var i = 0; i < sel.options.length; i++) if (sel.options[i].textContent === want) sel.selectedIndex = i;
    return want;
  }

  // ---------------------------------------------------------------------------
  //  開頁／重讀（引擎 gbLoad 套完元件之後）
  // ---------------------------------------------------------------------------
  function onLoaded(d) {
    if (!d || !d.bin) { say('❌ editlist.get BinSelect 的回應沒有 bin（後端版本不對？）', '#f88'); return; }
    BIN = d.bin; PROX = d.proxies || {};
    N = BIN.iTestBinCount | 0; T = BIN.eTrayCount | 0; NOTUSE = BIN.eBinNotUse; SETTING = BIN.eBinSetting;
    // 標籤文字（引擎只套 visible，不套 Caption）
    ['labWarning', 'Label1', 'LabBulkBox'].forEach(function (id) {
      var p = PROX[id], el = $(id);
      if (el && p) el.textContent = p.caption !== undefined ? p.caption : (id === 'LabBulkBox' ? el.textContent : '');
    });
    // sgSpecificBin：golden sgSpecificBinMouseDown（:6148）第 1 列 "" <-> "V"（引擎預設是 "On"，那是 Config 頁的）
    var sg = $('sgSpecificBin');
    if (sg) sg.onclick = function (ev) {
      var c = ev.target.closest && ev.target.closest('td');
      if (!c || sg.getAttribute('aria-disabled') === 'true' || c.getAttribute('data-r') !== '1') return;
      c.textContent = c.textContent === '' ? 'V' : '';
    };
    specOpened = false;
    var serverText = PROX.cbTestMode && PROX.cbTestMode.text;
    var tag = BIN.activeTag;
    R = null;
    cur = { tag: tag, meta: tagMeta(tag) || { lists: {}, panel: null }, P: null };   // 給 buildRules 的 lists
    R = buildRules();
    var want = fixModeOptions(serverText);
    if (want && want !== serverText && MODE_PAGE[want] !== undefined) tag = tagOfPage(MODE_PAGE[want]);
    keepModeText = null;
    selectTag(tag);
  }

  // 存檔附帶：作用中那一組的完整 panel（ParsePanel 要的型別與長度）＋ actions
  window.GB_EXTRA_SAVE = function () {
    var ex = {};
    if (specOpened) ex.actions = ['btnSettingSpecificBinClick'];
    var sel = $('cbTestMode');
    keepModeText = sel && sel.options[sel.selectedIndex] ? sel.options[sel.selectedIndex].textContent : null;
    if (!cur || !cur.P || !editable()) return ex;
    var P = cur.P, src = cur.meta.panel, out = {};
    Object.keys(src).forEach(function (k) {
      var v = P[k], o = src[k];
      if (k === 'iErrorT6') { out[k] = Math.trunc(Number(v)); return; }
      if (!Array.isArray(o)) { out[k] = v; return; }
      if (!Array.isArray(v) || v.length !== o.length) throw new Error('panel.' + k + ' 長度不對（' + (v && v.length) + ' != ' + o.length + '）');
      out[k] = v.map(function (e, i) {
        if (typeof o[i] === 'boolean') return !!e;
        var n = Number(e);
        if (!isFinite(n)) throw new Error('panel.' + k + '[' + i + '] 不是數字');
        return k.charAt(0) === 'd' ? n : Math.trunc(n);
      });
    });
    ex.bin = { tags: [{ tag: cur.tag, panel: out }] };
    return ex;
  };

  // 引擎的 gbLoad 呼叫 HT9045Recipe.editlistGet → 回應在引擎套完元件後（同一輪 microtask 之後）交給 onLoaded
  if (window.HT9045Recipe && HT9045Recipe.editlistGet) {
    var origGet = HT9045Recipe.editlistGet;
    HT9045Recipe.editlistGet = function (st) {
      return origGet.apply(this, arguments).then(function (d) {
        if (st === 'BinSelect') setTimeout(function () { onLoaded(d); }, 0);
        return d;
      });
    };
  }

  // ---------------------------------------------------------------------------
  //  DOM 事件
  // ---------------------------------------------------------------------------
  function bind() {
    var body = $('binBody');
    body.addEventListener('mousedown', function (ev) {
      var c = ev.target.closest && ev.target.closest('td'); if (!c) return;
      var tr = c.parentNode, y = +tr.getAttribute('data-y');
      if (!tr.hasAttribute('data-y')) return;
      ev.preventDefault();
      if (c.classList.contains('bc')) cellDown(y, +c.getAttribute('data-b'));
      else if (c.classList.contains('stat') && c.hasAttribute('data-col')) statDown(+c.getAttribute('data-col'), +tr.getAttribute('data-tray'));
    });
    body.addEventListener('mouseover', function (ev) {
      var c = ev.target.closest && ev.target.closest('td.bc'); if (!c || !drag) return;
      var y = +c.parentNode.getAttribute('data-y');
      if (y === drag.y) cellEnter(y, +c.getAttribute('data-b'));
    });
    body.addEventListener('mouseup', function (ev) {
      var c = ev.target.closest && ev.target.closest('td.colName'); if (!c || drag) return;
      nameUp(+c.parentNode.getAttribute('data-y'));
    });
    document.addEventListener('mouseup', mouseUp);
    $('pgbar').addEventListener('click', function (ev) {
      var b = ev.target.closest && ev.target.closest('.pgtab'); if (b) setBinPage(+b.getAttribute('data-p'));
    });
    $('btnAutoHide').addEventListener('click', function () { autoHideOff = !autoHideOff; applyAutoHide(); });
    // cbTestModeChange（:2147）
    $('cbTestMode').addEventListener('change', function () {
      var t = this.options[this.selectedIndex].textContent, idx = MODE_PAGE[t];
      if (idx === undefined) return;                                  // golden 沒有 else：不換頁
      var tag = tagOfPage(idx);
      if (tag >= 0) selectTag(tag);
    });
    // btnSetAll2NotUseClick（:6420）：golden 用 iTestRunMode 那一組（testRunModeTag）；網頁只送作用中那一組
    $('btnSetAll2NotUse').addEventListener('click', function () {
      if (!cur || !editable()) { say('這一組 bin 目前不能修改。', '#ffcc66'); return; }
      if (R.testRunModeTag !== cur.tag) {
        say('golden「Set all to not use」改的是 iTestRunMode 那一組（' + R.testRunModeTag + '），不是畫面這一組；畫面這一組不變。', '#ffcc66');
        return;
      }
      setBinTray(NOTUSE, 0, N); afterEdit();
    });
    // btnSettingSpecificBinClick（:6166）：面板顯示切換；存檔帶 actions 讓後端也打開（它底下的元件才改得到）
    $('btnSettingSpecificBin').addEventListener('click', function () {
      var pal = $('palSpecificBin'), open = pal.style.visibility === 'hidden';
      pal.style.visibility = open ? '' : 'hidden';
      if (open) {
        specOpened = true;
        [pal].concat([].slice.call(pal.querySelectorAll('input,select,button'))).forEach(function (x) {
          if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
        });
        pal.removeAttribute('aria-disabled');
      }
    });
    // CancelErrorBinClick（:6124）／rg_FixBinBoxClick（:6136）：iErrorT6=iFixRight
    function errToFixRight() {
      if (!cur || !editable()) return;
      if (R.iFixRight < 0) return;
      cur.P.iErrorT6 = R.iFixRight; afterEdit();
    }
    var ceb = $('CancelErrorBin').querySelector('input');
    ceb.addEventListener('change', function () { if (!ceb.checked) errToFixRight(); });
    var rgs = $('rg_FixBinBox').querySelectorAll('input');
    rgs[0].addEventListener('change', function () { if (rgs[0].checked) errToFixRight(); });
    // spbSave：引擎只在接線檔有欄位時才攔存檔鈕，本頁沒有 B 路欄位 → 自己接
    $('spbSave').addEventListener('click', function () { if (window.HT9045Page) HT9045Page.save(); });
    $('sbtExit').addEventListener('click', function () { if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); });
    ['spbAOIBin', 'spbNormal', 'spbPrime'].forEach(function (id) {
      $(id).addEventListener('click', function () { say(id + '：網頁端尚未接（golden 會開別的表單或立即寫檔），請在機台上操作。', '#ffcc66'); });
    });
  }

  // 探針／除錯用
  window.HT9045BinSel = {
    state: function () { return cur ? { tag: cur.tag, editable: editable(), N: N, T: T, NOTUSE: NOTUSE, SETTING: SETTING, panel: cur.P, rules: R, guessed: GUESS } : null; },
    filePanel: function () { return cur ? cur.meta.panel : null; }
  };

  HT9045Wire.register({
    page: 'Setup.BinSel.html',
    slug: 'setupbinsel',
    fields: {},
    kb: {
      ed_FixBinBoxAlarmCount: ['INTEGER', 0, true, 2, 20]      // golden ed_FixBinBoxAlarmCountMouseDown（:6118）
    }
  });
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind); else bind();
})();
