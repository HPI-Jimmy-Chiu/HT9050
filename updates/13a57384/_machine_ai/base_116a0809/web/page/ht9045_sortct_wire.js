/* ht9045_sortct_wire.js -- Data.SortCT.html <-> golden V912 TfSortCT（cSortCT.cpp／cSortCT.dfm）
 * ---------------------------------------------------------------------------
 * Steven 20260925 (Data.SortCT)
 * 手寫接線檔（檔名刻意不叫 ht9045_wire_<slug>.js，免得 gen_wire.py 重跑時覆蓋）。
 * 探針：HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_sortct_probe.py
 *
 * 顯示：全部是執行期 tag（WebBridgeTags.cpp 檔尾 W906_StageSortCTTags），null 一律 "---"（不可知，不是 0）。
 *   sort.loading／sort.total／sort.yield            Loading、Total、總良率（pnlLoader／pnlTotal／pnlYield）
 *   sort.<站>.count／.yield                          33 站（golden e6TrayName 順序）；沒設定的站 null
 *   sort.<站>.visible                                golden UpForm 的 myCountPanel[i].bVisible —— false 就把這一列藏起來
 *                                                    （ART 分頁同一站的列一起藏：golden SetVisible 一次設六個元件）
 *   sort.yield.visible                               pnlYield 可見度（TestIF_File.bLowYieldAlarmByBin）
 *   sort.art.*                                       ART 分頁；sort.art.tabVisible／tabCaption＝golden FormShow :196-204
 *   sort.ic.*                                        IC Count 分頁；sort.ic.tabVisible＝CosFunction.bShowHPICCount
 *   站名是 golden dfm 的固定字樣，寫在 HTML 裡（golden 執行期不改 lbl* 的 Caption）。
 *
 * Clear Count（btnClearCount）＝ golden btnClearCountClick（cSortCT.cpp:585-708），本體在 C++（cSortCT.cpp）：
 *   1. control.acquire
 *   2. act.sortCT.clearCount {"confirmed":false} —— C++ 跑 golden 守衛（SystemStart、fSecurity->Insufficient(108)、
 *      客戶專屬的 IC 檢查）到確認框為止；權限不足時 golden 會跳 WAR1676（告警框由既有的 modal 通道送來）。
 *   3. 回 needConfirm → 用 golden 的字樣問（"Clear Sort Count?"／"確定要清空計數？"，:637）→ 按確定才送
 *      {"confirmed":true}，C++ 守衛重跑、執行 golden 本體（fMain->Clarn_Data(8) 清 Loading／Sort／Tester Category／Index，
 *      寫 system\lastdata.dat）。網頁不做任何互鎖判斷，也不自己算數字 —— 清完的值等下一個 tag 快照。
 *   ⚠ act.sortCT.clearCount 的伺服器分派在這一波還沒接（tools/wb_serve.cpp／JsonBridge/ChanAction.cpp 由整合者加一行，
 *     C++ 函式是 WebSortCT.cpp 的 W906_SortCTClearCount）。沒接之前按鈕會顯示「分派未接」，不會假裝清了。
 *     （20260926 查：tools/wb_serve.cpp 已有 act.sortCT.clearCount 那一臂。）
 *
 * 點兩下數量／良率格（AI(W906-FRW-S65) 20260926）＝ golden pnlAuto1DblClick（cSortCT.cpp:1933-1950）：[O22] 開啟且沒在運轉時，
 *   那一站歸零並寫 system\lastdata.dat。送 act.sortCT.clearCount {"op":"dblClick","panel":"<元件 id>"}，守衛全在 C++。見下方 dblClick。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var ST = ['auto1', 'auto2', 'auto3', 'auto4', 'auto5', 'auto6',
            'fix1', 'fix2', 'fix3', 'fix4', 'fix5', 'fix6', 'fix7', 'fix8', 'fix9', 'fix10', 'fix11', 'fix12',
            'bulkbox',
            'mag1', 'mag2', 'mag3', 'mag4', 'mag5', 'mag6', 'mag7', 'mag8', 'mag9', 'mag10', 'mag11', 'mag12', 'mag13', 'mag14'];
  // 元件名的站名部分（golden cSortCT.h：pnlAuto1／pnlBinBox／pnARTMag3…）
  function sfx(slug) {
    if (slug === 'bulkbox') return 'BinBox';
    var m = /^([a-z]+)(\d+)$/.exec(slug);
    return m[1].charAt(0).toUpperCase() + m[1].slice(1) + m[2];
  }
  function $(id) { return document.getElementById(id); }

  // --- 可見度 ---------------------------------------------------------------
  // false 才藏；null（不可知）照樣顯示這一列（內容會是 "---"）—— 不把「不知道」當成「關」。
  function rowVis(slug) {
    return function (v, el) {
      var hide = (v === false);
      el.style.display = hide ? 'none' : '';
      var art = $('artrow_' + slug);
      if (art) art.style.display = hide ? 'none' : '';
    };
  }
  function yieldVis(v, el) { el.style.visibility = (v === false) ? 'hidden' : ''; }
  function tabVis(v, el) {
    var hide = (v === false);
    el.style.display = hide ? 'none' : '';
    if (hide && el.classList.contains('act')) {
      var first = $('tabSortCount');        // golden FormShow :206 ActivePageIndex=0
      if (first) first.click();
    }
  }
  function tabCaption(v, el) {
    if (typeof v === 'string' && v !== '') el.textContent = v;   // null 時保留 dfm 的字樣
  }

  var tags = {
    'sort.loading':        ['pnlLoader', 'text', 0],
    'sort.total':          ['pnlTotal', 'text', 0],
    'sort.yield':          ['pnlYield', 'text'],
    'sort.yield.visible':  ['pnlYield', yieldVis],
    'sort.art.loading':    ['pnlLoadingART', 'text', 0],
    'sort.art.total':      ['pnlTotalART', 'text', 0],
    'sort.art.yield':      ['pnlYieldART', 'text'],
    'sort.art.tabVisible': ['tabARTSortCount', tabVis],
    'sort.art.tabCaption': ['tabARTSortCount', tabCaption],
    'sort.ic.tabVisible':  ['tabICCount', tabVis],
    'sort.ic.load':        ['pnlLoad', 'text'],
    'sort.ic.hp1':         ['pnlHP1', 'text'],
    'sort.ic.hp2':         ['pnlHP2', 'text']
  };
  ST.forEach(function (slug) {
    var s = sfx(slug);
    tags['sort.' + slug + '.count']     = ['pnl' + s, 'text', 0];
    tags['sort.' + slug + '.yield']     = ['pnl' + s + 'Yield', 'text'];
    tags['sort.' + slug + '.visible']   = ['row_' + slug, rowVis(slug)];
    tags['sort.art.' + slug + '.count'] = ['pnART' + s, 'text', 0];
    tags['sort.art.' + slug + '.yield'] = ['pnART' + s + 'Yield', 'text'];
  });

  HT9045Wire.register({
    page: 'Data.SortCT.html',
    slug: 'datasortct',
    tags: tags
  });

  // --- Clear Count -------------------------------------------------------------
  var busy = false, last = null, lastError = '';

  function say(msg, bad) {
    var s = $('sortStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function send(confirmed) {
    return cmd('act.sortCT.clearCount', { value: JSON.stringify({ confirmed: !!confirmed }) })
      .then(function (r) { return r; }, function (e) { return parseErr(e); });
  }
  function describe(r) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd/.test(r.detail || '')) {
      return 'Clear Count 的伺服器分派還沒接（act.sortCT.clearCount → WebSortCT.cpp W906_SortCTClearCount），由整合者加一行。沒有清除任何東西。';
    }
    return '沒有清除：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }

  // opts.answer：測試用，true/false 直接回答確認框；沒給就用 window.confirm（golden 的兩行字）
  function clearCount(opts) {
    if (busy) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say('Clear Count…');
    var asked = false;
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ })
      .then(function () { return send(false); })
      .then(function (r) {
        if (r && r.needConfirm) {
          asked = true;
          var text = (r.prompt || ['Clear Sort Count?', '確定要清空計數？']).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          if (!yes) return { executed: false, guard: 'confirm-no', detail: '使用者取消', cancelled: true };
          return send(true);
        }
        return r;
      })
      .then(function (r) {
        last = r; if (r) r.asked = asked;
        if (r && r.executed) { lastError = ''; say('Sort Count 已清除（golden btnClearCountClick → Clarn_Data(8)）'); }
        else if (r && r.cancelled) { lastError = ''; say('已取消，沒有清除'); }
        else if (isBusyReply(r)) { lastError = ''; say(HT9045Busy.NOTE); }   // AI(W906-CMDGUARD-UI) 20260926：伺服器 busy: 不是失敗
        else { lastError = describe(r); say(lastError, true); }
        busy = false;
        return r;
      }, function (e) {
        busy = false; last = parseErr(e);
        if (isBusyReply(last)) { lastError = ''; say(HT9045Busy.NOTE); }     // AI(W906-CMDGUARD-UI) 20260926：同上
        else { lastError = describe(last); say(lastError, true); }
        return last;
      });
  }

  // --- 點兩下清單站數量（golden pnlAuto1DblClick，V912 cSortCT.cpp:1933-1950）-------------------------
  // AI(W906-FRW-S65) 20260926：golden 建構子 :127-135 把 Bulk Box 以外每一站的數量格／良率格 OnDblClick 接到
  //   pnlAuto1DblClick：運轉中（SystemStart）或 [O22]（config.ini [O_Count] bO22_ClearSortCntByDoubleClick）沒開 → 什麼都不做；
  //   否則那一站的 LastSet.BinCT 歸零並立刻寫 system\lastdata.dat（golden 沒有確認框、沒有權限檢查）。
  //   伺服器端：沿用 act.sortCT.clearCount，value={"op":"dblClick","panel":"<元件 id>"}（WebSortCT.cpp W906_SortCTDblClickOp）；
  //   「golden 有沒有接這一格」「看不看得見」與 golden 守衛都在 C++ 重查，網頁不判斷。
  //   golden 靜默不做的情況（O22 沒開、運轉中、沒接事件的格子、看不見）只在狀態列提示，不當錯誤。
  var QUIET = { 'O22-off': 1, 'SystemStart': 1, 'not-wired': 1, 'not-visible': 1 };
  // AI(W906-CMDGUARD-UI) 20260926（S107 防連點的前端第二道）：每一格在 ack 回來之後冷卻 coolMs()（400 ms），冷卻中「同一格」的
  //   點兩下直接忽略（不送、不顯示）；別的格子不受影響（只要不在 busy）。原本 busy 只擋到 ack：很快地連點四下會有兩個 dblclick 事件
  //   （間隔約 200–300 ms），第二個多半在 ack 之後才到 → golden pnlAuto1DblClick 跑兩次（同一站歸零兩次、lastdata.dat 寫兩次）。
  //   主防線是伺服器 WebCmdGuard（同指令＋tag＋value 400 ms 內回 busy:）；這裡讓第二下連送都不送。
  var dblCoolUntil = {}, dblCoolMax = 0;
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function isBusyReply(x) { return !!(window.HT9045Busy && HT9045Busy.is(x)); }   // ht9045_busy_util.js（沒載入時一律當一般錯誤）
  function coolPanel(id) { var t = Date.now() + coolMs(); dblCoolUntil[id] = t; if (t > dblCoolMax) dblCoolMax = t; }
  function dblClick(id) {
    if (busy || Date.now() < (dblCoolUntil[id] || 0)) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ })
      .then(function () {
        return cmd('act.sortCT.clearCount', { value: JSON.stringify({ op: 'dblClick', panel: id }) })
          .then(function (r) { return r; }, function (e) { return parseErr(e); });
      })
      .then(function (r) {
        last = r; busy = false; coolPanel(id);
        if (r && r.executed) { lastError = ''; say(id + ' 已歸零（golden pnlAuto1DblClick，已寫 lastdata.dat）'); }
        else if (r && QUIET[r.guard]) { lastError = ''; say('點兩下沒有清除：' + r.guard + (r.detail ? '（' + r.detail + '）' : '')); }
        else if (isBusyReply(r)) { lastError = ''; say(HT9045Busy.NOTE); }   // AI(W906-CMDGUARD-UI) 20260926：伺服器 busy: 不是失敗
        else { lastError = describe(r); say(lastError, true); }
        return r;
      }, function (e) {
        busy = false; coolPanel(id); last = parseErr(e);
        if (isBusyReply(last)) { lastError = ''; say(HT9045Busy.NOTE); }     // AI(W906-CMDGUARD-UI) 20260926：同上
        else { lastError = describe(last); say(lastError, true); }
        return last;
      });
  }

  function bind() {
    var b = $('btnClearCount');
    if (b) b.addEventListener('click', function () { clearCount(); });
    // AI(W906-FRW-S65) 20260926：golden :129 if(i!=eBulkBox) → Bulk Box 那一列不接；pnlLoadCID／pnlCoverTrayD（dfm :107／:184）頁面有才接
    var ids = [];
    ST.forEach(function (slug) {
      if (slug === 'bulkbox') return;
      var s = sfx(slug);
      ids.push('pnl' + s, 'pnl' + s + 'Yield');
    });
    ids.push('pnlLoadCID', 'pnlCoverTrayD');
    ids.forEach(function (id) {
      var el = $(id);
      if (el) el.addEventListener('dblclick', function () { dblClick(id); });  if (el) el.addEventListener('contextmenu', function (ev) { ev.preventDefault(); cmd('act.trayEdit', { value: JSON.stringify({ op: 'rightClick', panel: id }) }).catch(function () {}); });   // AI(W906-S10) 20260929 (St02-E, claim): golden cSortCT.cpp:436-583 / :1781-1884 right-click -> EditTray (uTrayEditForm); every guard runs in C++ (TrayEditForm.cpp)
    });
  }

  window.HT9045SortCT = {
    clear: clearCount,
    dblClick: dblClick,
    last: function () { return last; },
    lastError: function () { return lastError; },
    busy: function () { return busy || Date.now() < dblCoolMax; },   // AI(W906-CMDGUARD-UI) 20260926：含點兩下的冷卻（探針等 !busy() 再點下一下）
    stations: function () { return ST.slice(); }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind);
  else bind();
})();
