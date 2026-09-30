/* ht9045_observer_wire.js -- Data.Observer.html（golden V912 TfObserver，cObserver.cpp）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：wb_serve WS 指令 observer.get → cObserver.cpp 檔尾 W906_ObserverJson。
 *   value = {"act":..., "arg":n, "text":"..."}，每個 act 重播 golden 的一個操作：
 *     open    開頁＝golden 開窗 FormShow（:347-652）
 *     timer   每 1000 ms 一次＝golden Timer1（dfm 沒設 Interval → VCL 預設 1000；FormShow :435 啟動）
 *             → Timer1Timer → GetMachineData → ProcessRunInfo，只回 captions
 *     tab     點「Tester Category」(1)／「System Message」(3) 頁籤＝pgcObservChange（:2336）
 *             AI(W906-PROD-S116) 20260926（Steven 團隊）：另外放行 0 Counter／4 Yield（第一次點才 SetSiteYieldDiagram）／6 Test Information
 *     ccKinds／ccKindsForm／ccHistory／ccHistoryForm  點 Counter 分頁的四個 RadioGroup（arg＝項次）＝golden :3220-3241（WriteContactKind）
 *     AI(W906-PROD-S116Y) 20260927（Steven 團隊）：Yield 分頁的四個操作（C++ 回 captions＋yield，full:false）—— 會改記憶體，要權杖（見下面「權杖」）
 *     yieldSite   點 mtRowA..D 任一格（arg＝0..3，text＝"x,y" 格子座標）＝golden mtRowAMouseUp（:3244-3273）：
 *                 site 那一列切換那一條線；表頭那一列（(0,0) 的 Show All／Hide All、Now、Last n）整排切換
 *     yieldMax／yieldMin  edYieldMax／edYieldMin 打完字（text＝框的字）＝golden edYieldMaxClick :3208-3212／edYieldMinClick :3214-3218
 *                 （QWERTY 鍵盤 → CheckRange → LeftAxis->Maximum／Minimum）
 *     yieldClear  按 SpeedButton1 'Clear'＝golden SpeedButton1Click（:865-872，只清記憶體裡的 HistroyBin；golden 不重畫格子）
 *             回應多三塊：counter（StringGrid2／3 畫出來的字）、yield（mtRowA..D 格子＋ChartYield：AI(W906-PROD-S116) 20260927 起
 *             C++ 送 golden UpdateYieldChart AddY 進 32 條 Series 的點，這裡用 inline SVG 畫；舊版 C++ 送 chart.noSource 時照舊顯示 ---）、
 *             testInfo（pgcTestInfo 七張表；RecordTimeInfo 是空殼 → 空格顯示 ---；Index Time 欄（第 5 欄）AI(W906-PROD-S116) 20260926 起
 *             由 C++ 照 golden RecordTimeInfo :1973-1974／:2021-2022 的式子從 fRecordIndexTime 算好送來，RecordIndexTime 由 Jimmy J6 解閘）
 *     rowNo   點 rgRowNo 選項＝rgRowNoClick（:1665）
 *     form    點 Display Form 四顆 radio（text＝元件名）＝dfm OnClick=rgRowNoClick
 *     year    改 cbbEventLogYear（golden 沒有 OnChange，只改 Text）
 *     month   改 cbbMonth＝cbbMonthChange（:3764）
 *     file    點 lstEventLog＝lstEventLogClick（:3758）
 *     filter  改 cbbFilter（golden 沒有 OnChange，按 Query 才套用）
 *     query   按 Query＝btnQueryEventLogTxtClick（:4479）
 *   回應：captions{元件:Caption}、visible、noSource{元件:原因}、sources（golden 讀的輸入）、
 *         lastdata（那些欄位在 system\lastdata.dat 的位移）、category（16 個 TTMyTray＋radio）、
 *         eventLog（strngrdEventLog 全格＋年／月／檔案／Filter 狀態）。timer 只有前四項（full:false）。
 *
 * 這一支只接 golden 在 wb_serve 沒有其他 producer 的 14 格（Operating Information 8 格、
 * labVersion／labFactory、四個 *Version）＋ APHeadLabel18、Tester Category 整張、System Message 整張。
 * labModel／labSerialNo／labMachineID／labDeviceName／labReleaseDate 維持 ht9045_wire_dataobserver.js
 * （sysText／tag）接線，不在這裡重複寫，免得兩個寫入者搶同一格。
 *
 * 權杖：AI(W906-Q2-S124) 20260927（Steven 團隊；RULINGS_20260926 S124＝B「唯讀的查詢免權杖」）
 *   唯讀的 act（READ_ACTS：open／timer／tab／rowNo／form／year／month／file／filter／query／ccKinds／ccKindsForm／
 *   ccHistory／ccHistoryForm ＝ WebCmdGuard.cpp:112-113 kObserverActs 同一張）直接送 observer.get、不拿權杖 ——
 *   observer.get 在 WebBridgeServer.cpp:1448 名稱級免權杖（St02 9d790ff2）。
 *   舊的 wb_serve（沒有那一行）回 not-operator：退回原本的 acquire → observer.get → release（持有只有幾毫秒；
 *   timer 那一拍權杖在別人手上就略過，跟改之前一樣）。
 *   會改記憶體的四個 act（yieldSite／yieldMax／yieldMin／yieldClear：切換 bShowYieldSeries、設 LeftAxis、
 *   SpeedButton1Click 清 HistroyBin）照舊每次 acquire → observer.get → release —— St02 要把這四個改成要權杖，
 *   這樣它的改動進來之前、之後都能用。不在 READ_ACTS 的 act 一律當成要權杖（新的 act 要一條一條放行，同伺服器）。
 *   （原本每一條都 acquire → get → release：Timer 每秒一次，別的頁面正要存檔時剛好撞上就會拿不到權杖。）
 * 定時更新：AI(W906-Q2-S124) 20260927，比照 ht9045_contactct_wire.js（c35f375e）
 *   一次只送一條：timer 撞上送出中就略過；使用者的操作排隊、不丟（每一個都是 golden 的一個操作，照順序送）；
 *   送出中那一拍 timer 的回應，若後面已經排了使用者的操作，就是過時的，不畫。
 *   沒人看就不拍：分頁在背景（document.hidden）或外框被縮小／關掉（background.html 把外框設 display:none，iframe 還在）。
 *   回應沒變就不動畫面：timer 回應逐塊（captions／yield／testInfo）比對上一次，一樣的那一塊不重畫；狀態列 timer
 *   不每秒改寫（「尚未接」「剛送過」之類的提示不會一秒就被蓋掉），只有「沒有來源」清單變了或剛從錯誤恢復才改。
 *   開頁那一下沒成功（wb_serve 還沒起來…）：之後每拍重試開窗（原本會停在錯誤，要重新整理）。
 *   window.__observer：loads（開頁＋操作）、ticks、same（timer 回應整包沒變）、renders（timer 有改畫面）、skipped（略過的拍）、
 *   stale（過時不畫的拍）、tokenFallback（舊 wb_serve 退回拿權杖）、tokenOps（四個改記憶體的 act 拿權杖）、error、tickError、
 *   data、last、acts、sent（這一頁送出的指令，最後 40 條：{cmd, act}）—— 探針 tools/webprobe/data_observer_token_probe.py 讀這些。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var CAPS = ['labPowerOnTime', 'labRunningTime', 'labProductTime', 'labLoadingCount',
              'labMUBA', 'labMTBA', 'labMTBF', 'pnlDayJamRate',
              'labVersion', 'labFactory', 'pnlGPIBVersion', 'pnlESDVersion', 'pnlATCVersion', 'pnlTTLRS232Version',
              'APHeadLabel18'];
  var SRC = {   // 畫面 title 用：golden 寫這一格的那一行
    labPowerOnTime: 'golden cObserver.cpp:757 ConvertMSecToTime(LastSet.SystemAccSecond[0][stPowerOn])',
    labRunningTime: 'golden cObserver.cpp:758 ConvertMSecToTime(LastSet.SystemAccSecond[0][stStartTime])',
    labProductTime: 'golden cObserver.cpp:759 ConvertMSecToTime(LastSet.SystemAccSecond[0][stProductTime])',
    labLoadingCount: 'golden cObserver.cpp:761-764 LastSet.SendCT[1]（CC_AMKOR_Korea 用 SendCT[0]）',
    labMUBA: 'golden cObserver.cpp:2283-2307 ProcessRunInfo（LastSet.iJamCount[1]／SendCT[1]）',
    labMTBA: 'golden cObserver.cpp:2261-2281 ProcessRunInfo（(Pause+Product+Jam)/1000/iJamCount[1]）',
    labMTBF: 'golden cObserver.cpp:2309 ConvertSecondToSPC(LastSet.SystemAccSecond[0][stPowerOn]/1000)',
    pnlDayJamRate: 'golden cObserver.cpp:2311-2330 ProcessRunInfo（IniConfig.bVTESTFunction 才顯示）',
    labVersion: 'golden cObserver.cpp:3629 labVersion=Memo1 第 0 行（ShowVer :3554 asVer）',
    labFactory: 'golden main.cpp:11057 labFactory=RunInfo.Factory',
    APHeadLabel18: 'golden cObserver.cpp:375 MyDBQClearDT()'
  };

  var TICK_MS = 1000;       // golden Timer1（dfm 沒設 Interval → VCL 預設 1000）
  var D = null;             // 最近一次完整（full）回應
  var busy = false, opened = false, tick = null;
  var st = window.__observer = { loads: 0, ticks: 0, same: 0, renders: 0, skipped: 0, stale: 0, tokenFallback: 0, tokenOps: 0,
                                 data: null, last: null, error: null, tickError: null, acts: [], sent: [] };
  // AI(W906-Q2-S124) 20260927（Steven 團隊）：唯讀、免權杖的 act ＝ WebCmdGuard.cpp:112-113 kObserverActs（"" 本體也當 open）。
  //   不在這張表的（yieldSite／yieldMax／yieldMin／yieldClear，以及將來新加的）一律 acquire → get → release。
  var READ_ACTS = { '': 1, open: 1, timer: 1, tab: 1, rowNo: 1, form: 1, year: 1, month: 1, file: 1, filter: 1, query: 1,
                    ccKinds: 1, ccKindsForm: 1, ccHistory: 1, ccHistoryForm: 1 };
  function isReadAct(act) { return Object.prototype.hasOwnProperty.call(READ_ACTS, act === undefined || act === null ? '' : String(act)); }

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function say(msg, colour) {
    var el = $('obsStatus');
    if (el) { el.textContent = msg || ''; el.style.color = colour || ''; }
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    // AI(W906-Q2-S124) 20260927：記下送出的指令（最後 40 條），探針用來看「唯讀 act 前面沒有 control.acquire」
    var act = null;
    if (name === 'observer.get') { try { act = JSON.parse(extra.value).act; } catch (e) { act = null; } if (act === undefined) act = null; }
    st.sent.push({ cmd: name, act: act });
    if (st.sent.length > 40) st.sent.shift();
    return HT9045Recipe.rawCmd(name, extra);
  }

  // TPanel 匯出的 <div class="pnl">：值寫進 .pnlCap（同 ht9045_wire_engine.js showText）
  function showText(el, text) {
    if (!el) return;
    if (el.classList && el.classList.contains('pnl')) {
      var cap = el.querySelector(':scope > .pnlCap');
      if (!cap) { cap = document.createElement('span'); cap.className = 'pnlCap'; el.appendChild(cap); }
      cap.classList.add('runtimePanelValue');
      cap.textContent = text;
      return;
    }
    el.textContent = text;
  }

  // AI(W906-Q2-S124) 20260927（Steven 團隊）：acquire → observer.get → release（持有只有幾毫秒）。
  //   quiet=true（timer）時，權杖在別人手上就丟 'control-held'，讓這一拍略過。release 不論成敗都會送（有拿到才送）。
  function getWithToken(extra, quiet, why) {
    var held = false;
    function release() { return held ? raw('control.release').catch(function () {}) : Promise.resolve(); }
    return raw('control.acquire').then(function () {
      held = true;
      return raw('observer.get', extra).then(unwrap);
    }, function (e) {
      if (quiet) throw new Error('control-held');
      throw new Error('拿不到控制權杖（' + e.message + '）—— 別的頁面正持有，' + why);
    }).then(function (d) { return release().then(function () { return d; }); },
            function (e) { return release().then(function () { throw e; }); });
  }
  // 唯讀 act：直接 observer.get（S124 免權杖）；舊 wb_serve 回 not-operator 才退回 getWithToken。
  // 其他 act（四個改記憶體的 Yield 操作）：每次都 getWithToken（St02 把它們改成要權杖之前、之後都能用）。
  function fetchObs(v, quiet) {
    var extra = { value: JSON.stringify(v) };
    if (!isReadAct(v.act)) {
      st.tokenOps++;
      return getWithToken(extra, quiet, 'observer.get 的 ' + v.act + ' 會改記憶體，要權杖');
    }
    return raw('observer.get', extra).then(unwrap, function (e) {
      if (!e || e.message !== 'not-operator') throw e;
      st.tokenFallback++;               // 舊的 wb_serve（沒有 WebBridgeServer.cpp:1448 的 observer.get 免權杖）
      return getWithToken(extra, quiet, '這一版 wb_serve 的 observer.get 還要權杖（沒有 S124 免權杖那一行）');
    });
  }

  // 一次只送一條。操作員的點擊不可以因為剛好撞上 timer 那一拍而被吃掉：排隊，等前一個做完照順序送；timer 撞上就略過。
  var queue = [];
  var errShown = false, lastNote = null;
  function send(v, quiet) {
    if (busy) {
      if (quiet) { st.skipped++; return Promise.resolve(null); }
      return new Promise(function (resolve) { queue.push({ v: v, resolve: resolve }); });
    }
    return sendNow(v, quiet);
  }
  function sendNow(v, quiet) {
    busy = true;
    var isTick = v.act === 'timer';
    return fetchObs(v, quiet).then(function (d) {
      if (isTick) { st.ticks++; st.tickError = null; } else { st.loads++; st.acts.push(v.act); st.error = null; }
      if (isTick && queue.length) { st.stale++; return d; }   // 後面已經排了使用者的操作：這一拍的回應過時，不畫
      st.last = d;
      if (d.full) { D = d; st.data = d; }
      render(d, isTick);
      return d;
    }).catch(function (e) {
      if (quiet && e.message === 'control-held') { st.skipped++; return null; }
      if (window.HT9045Busy && HT9045Busy.is(e)) { st.busy = (st.busy || 0) + 1; say(HT9045Busy.NOTE); return null; }   // AI(W906-PROD-S116Y) 20260927：busy: 不是失敗（ht9045_busy_util.js）
      if (isTick) st.tickError = e.message; else st.error = e.message;
      errShown = true;
      say('❌ observer.get（' + v.act + '）失敗：' + e.message, '#c00');
      return null;
    }).then(function (d) {
      busy = false;
      var next = queue.shift();
      if (next) sendNow(next.v, false).then(next.resolve);
      return d;
    });
  }

  // ---- AI(W906-PROD-S116Y) 20260927（Steven 團隊）：Yield 分頁四個操作（yieldSite／yieldMax／yieldMin／yieldClear）的防連點 --------
  //   Steven 20260926 規則（全部按鈕要防連點）的頁面這一道，同 ht9045_counterclear_wire.js 的 exeHold：送出中、或 ack 回來後
  //   coolMs()（400 ms）內，同一個操作（act＋arg＋text 都一樣）再來一律丟掉 —— 不排隊（send() 碰到 busy 會排隊：連點兩下 site 格
  //   就會切換兩次＝等於沒點）。golden TTMyTray 連點兩下是 OnMouseUp 兩次＝切換兩次；這裡照 Steven 的規則擋掉第二下（刻意的差異）。
  //   ⚠ 伺服器那一道（WebCmdGuard）目前不擋這四個：observer.get 在它的名稱級白名單（WebCmdGuard.cpp:71「純讀」）。
  //   AI(W906-Q2-S124) 20260927：⛔ 上一行已過時 —— WebCmdGuard 現在按 act 分（WebCmdGuard.cpp:112-113 kObserverActs、:127），
  //   這四個不在讀取型名單，伺服器那一道也會擋（回 busy:，sendNow 當成「剛送過」處理）。這四個送出時都拿權杖（fetchObs）。
  var yHold = {};
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function busyNote() { return (window.HT9045Busy && HT9045Busy.NOTE) || '同一個指令剛送過（上一下還在跑或剛做完），這一下略過'; }
  function holdClear(on) {
    var b = $('SpeedButton1');
    if (b) b.disabled = !!on;
  }
  function yieldAct(v) {
    if (!opened) return Promise.resolve(null);
    var key = v.act + '|' + (v.arg === undefined ? '' : v.arg) + '|' + (v.text === undefined ? '' : v.text);
    if (yHold[key]) { st.yieldDropped = (st.yieldDropped || 0) + 1; say(busyNote()); return Promise.resolve(null); }
    yHold[key] = true;
    if (v.act === 'yieldClear') holdClear(true);
    return send(v).then(function (d) {
      setTimeout(function () { delete yHold[key]; if (v.act === 'yieldClear') holdClear(false); }, coolMs());
      return d;
    });
  }

  // AI(W906-Q2-S124) 20260927（Steven 團隊）：isTick＝timer 回應。開頁／操作的回應照原本整塊重畫；
  //   timer 回應逐塊比對上一次畫的內容（sig），一樣就不動那一塊的 DOM（每秒整塊 innerHTML 會把正在點的格子／radio 換掉）。
  var sig = {};
  function render(d, isTick) {
    var drew = false;
    function block(key, obj, fn) {
      var s = JSON.stringify(obj);
      if (isTick && s === sig[key]) return;
      sig[key] = s;
      fn(obj);
      drew = true;
    }
    block('caps', [d.captions || null, d.visible || null, d.noSource || null], function () { renderCaptions(d); });
    if (d.full) {
      block('category', d.category, renderCategory);
      block('eventLog', d.eventLog, renderEventLog);
    }
    // AI(W906-PROD-S116) 20260926（Steven 團隊）：Counter／Yield／Test Information。full 回應一定帶 counter；
    //   timer 回應只在 C++ 端目前停在 Yield（4）或 Test Information（6）時帶那一頁（golden 那兩頁的格子是生產時即時更新的）。
    if (d.counter) block('counter', d.counter, renderCounter);
    if (d.yield) block('yield', d.yield, renderYield);
    if (d.testInfo) block('testInfo', d.testInfo, renderTestInfo);
    if (isTick) { if (drew) st.renders++; else st.same++; }
    var ns = Object.keys(d.noSource || {});
    var note = ns.length ? ns.join('、') : '無';
    // timer 不每秒改寫狀態列：只有「沒有來源」清單變了、或剛從錯誤恢復才改（別的提示不被一秒蓋掉）
    if (!isTick || note !== lastNote || errShown) {
      say('observer.get ' + d.act + ' ✓ ' + new Date().toLocaleTimeString() + '（沒有來源、顯示 "---" 的格子：' + note + '）', '#282');
      errShown = false;
    }
    lastNote = note;
  }

  function renderCaptions(d) {
    var caps = d.captions || {}, ns = d.noSource || {}, vis = d.visible || {};
    CAPS.forEach(function (id) {
      var el = $(id);
      if (!el) return;
      if (Object.prototype.hasOwnProperty.call(ns, id)) {
        showText(el, '---');
        el.title = id + '：沒有來源，顯示 "---"（不可知，不是 0）。\n' + ns[id];
        el.setAttribute('data-obs', 'nosource');
      } else if (Object.prototype.hasOwnProperty.call(caps, id)) {
        showText(el, caps[id]);
        el.title = id + '：C++ observer.get（' + (SRC[id] || 'golden TfObserver') + '）';
        el.setAttribute('data-obs', 'value');
      }
    });
    ['pnlDayJamRate', 'labDayJamRate'].forEach(function (id) {
      var el = $(id);
      if (el && Object.prototype.hasOwnProperty.call(vis, id)) el.style.display = vis[id] ? '' : 'none';
    });
  }

  // ---- Tester Category：golden ScrollBox1 的 16 個 TTMyTray ＋ rgRowNo ＋ GroupBox8，座標照 dfm ----
  function renderCategory(c) {
    var host = $('tcTrays');
    if (!host || !c) return;
    var sb = $('ScrollBox1');
    if (sb && c.scrollBoxColor) sb.style.background = c.scrollBoxColor;
    var h = '', maxH = 0;
    var rg = c.rgRowNo;
    h += '<fieldset class="tcRg" id="rgRowNo" title="rgRowNo : TRadioGroup（golden :1665 rgRowNoClick）" style="left:' + rg.left + 'px;top:' + rg.top +
         'px;width:' + rg.width + 'px;height:' + rg.height + 'px;"><legend>' + esc(rg.caption) + '</legend><div class="tcRgItems" style="grid-template-columns:repeat(' +
         Math.max(1, rg.columns) + ',1fr);">';
    rg.items.forEach(function (t, i) {
      h += '<label><input type="radio" name="rgRowNo" value="' + i + '"' + (i === rg.itemIndex ? ' checked' : '') + '>' + esc(t) + '</label>';
    });
    h += '</div></fieldset>';
    var gb = c.groupBox8;
    h += '<fieldset class="tcRg" id="GroupBox8" title="GroupBox8 : TGroupBox" style="left:' + gb.left + 'px;top:' + gb.top +
         'px;width:' + gb.width + 'px;height:' + gb.height + 'px;"><legend>' + esc(gb.caption) + '</legend>';
    (c.labels || []).forEach(function (l) {
      h += '<span class="tcLab" style="left:' + l.left + 'px;top:' + l.top + 'px;">' + esc(l.caption) + '</span>';
    });
    Object.keys(c.radios).forEach(function (id) {
      var r = c.radios[id];
      h += '<label class="tcRb" title="' + id + ' : TRadioButton" style="left:' + r.left + 'px;top:' + r.top + 'px;width:' + r.width + 'px;' +
           (r.enabled ? '' : 'color:#999;') + '"><input type="radio" name="tcForm" id="' + id + '" value="' + id + '"' +
           (r.checked ? ' checked' : '') + (r.enabled ? '' : ' disabled') + '>' + esc(r.caption) + '</label>';
    });
    h += '</fieldset>';
    c.trays.forEach(function (t) {
      maxH = Math.max(maxH, t.top + t.height);
      h += '<div class="tcTray" id="' + t.name + '" data-x="' + t.x + '" data-y="' + t.y + '" title="' + t.name +
           ' : TTMyTray（' + t.x + '×' + t.y + '）" style="left:' + t.left + 'px;top:' + t.top + 'px;width:' + t.width + 'px;height:' +
           t.height + 'px;background:' + t.color + ';grid-template-columns:repeat(' + Math.max(1, t.x) + ',1fr);grid-template-rows:repeat(' +
           Math.max(1, t.y) + ',1fr);">';
      for (var y = 0; y < t.y; y++) {
        for (var x = 0; x < t.x; x++) {
          var ci = (t.colorIndex[y] && t.colorIndex[y][x]) || 0;
          h += '<div class="tcCell" data-cx="' + x + '" data-cy="' + y + '" style="background:' + (t.colorMap[ci] || t.colorMap[0]) + ';">' +
               esc(t.cells[y][x]) + '</div>';
        }
      }
      h += '</div>';
    });
    host.innerHTML = h;
    host.style.height = (maxH + 12) + 'px';
    host.querySelectorAll('input[name=rgRowNo]').forEach(function (r) {
      r.addEventListener('change', function () { if (r.checked) send({ act: 'rowNo', arg: +r.value }); });
    });
    host.querySelectorAll('input[name=tcForm]').forEach(function (r) {
      r.addEventListener('change', function () { if (r.checked) send({ act: 'form', text: r.value }); });
    });
  }

  // ---- System Message／Text：strngrdEventLog 全格，年／月／檔案清單／Filter 照 C++ 狀態 ----
  function fillSelect(sel, items, idx, text) {
    if (!sel) return;
    var h = '';
    items.forEach(function (t, i) { h += '<option value="' + i + '">' + esc(t) + '</option>'; });
    sel.innerHTML = h;
    if (idx >= 0 && idx < items.length) sel.selectedIndex = idx;
    else if (text !== undefined && text !== '') {       // csDropDown：Text 可以不在 Items 裡
      var op = document.createElement('option'); op.value = 'text'; op.text = text; op.setAttribute('data-text', '1');
      sel.appendChild(op); sel.selectedIndex = sel.options.length - 1;
    } else sel.selectedIndex = -1;
    for (var i = 0; i < sel.options.length; i++) {
      if (i === sel.selectedIndex) sel.options[i].setAttribute('selected', 'selected'); else sel.options[i].removeAttribute('selected');
    }
  }

  function renderEventLog(e) {
    if (!e) return;
    var yi = e.years.indexOf(e.yearText);
    fillSelect($('cbbEventLogYear'), e.years, yi, e.yearText);
    fillSelect($('cbbMonth'), e.months, e.monthIndex, e.monthText);
    fillSelect($('cbbFilter'), e.filters, e.filterIndex, e.filterText);
    var lst = $('lstEventLog');
    if (lst) {
      var h = '';
      e.files.forEach(function (f, i) {
        h += '<div class="obsLi' + (i === e.fileIndex ? ' sel' : '') + '" data-i="' + i + '">' + esc(f) + '</div>';
      });
      lst.innerHTML = h;
      lst.querySelectorAll('.obsLi').forEach(function (d) {
        d.addEventListener('click', function () { send({ act: 'file', arg: +d.getAttribute('data-i') }); });
      });
    }
    var t = $('evGrid');
    if (!t) return;
    var cw = e.colWidths || [], tw = 0;
    var h2 = '<colgroup>';
    for (var c = 0; c < e.cols; c++) { h2 += '<col style="width:' + (cw[c] || 64) + 'px">'; tw += (cw[c] || 64); }
    h2 += '</colgroup><thead>';
    for (var r = 0; r < e.rows; r++) {
      if (r === e.fixedRows) h2 += '</thead><tbody>';
      var fixed = r < e.fixedRows;
      h2 += '<tr data-r="' + r + '">';
      for (c = 0; c < e.cols; c++) {
        var v = (e.cells[r] && e.cells[r][c] !== undefined) ? e.cells[r][c] : '';
        h2 += (fixed ? '<th' : '<td') + ' style="background:' + (fixed ? e.fixedColor : e.color) + ';">' + esc(v) + (fixed ? '</th>' : '</td>');
      }
      h2 += '</tr>';
    }
    if (e.rows <= e.fixedRows) h2 += '</thead><tbody>';
    h2 += '</tbody>';
    t.style.tableLayout = 'fixed';
    t.style.width = tw + 'px';
    t.innerHTML = h2;
    t.title = 'strngrdEventLog（golden GetEventLogText :3857）：' + (e.file || '（沒有檔案）') + '，' + e.rows + ' 列 × ' + e.cols + ' 欄';
  }

  // ---- AI(W906-PROD-S116) 20260926（Steven 團隊）：Counter 分頁（tsCounter 的 StringGrid2／3）--------------------------------
  //   C++ 送的是 golden 畫出來的字（WriteContactKind :1301-1648 寫 Cells[..][2]／sCounterColKind；StringGrid2DrawCell :952-984、
  //   StringGrid3DrawCell :988-1019、DrawCellCounter :1080-1134 畫 "Row-x"／"Col-x"／"Arm 1/2"／Socket 列、APHeadLabel13／14 的總數）。
  //   一組 Col 跨兩欄（golden MyDrawText 從 iLeft+iCellWidth*2*i 畫到 *(i+1)）；9045 的格子 ColCount=9 → 看得到 Col-a～Col-d。
  //   ⚠ golden 兩張表的 "Row-x" 都用 rgContactCountKinds（History 那張也是），C++ 照送。
  var CC = {
    kinds:   { grid: 'StringGrid2', total: 'APHeadLabel13', rg: 'rgContactCountKinds',     rgForm: 'rgContactCountKindsForm',     act: 'ccKinds',   actForm: 'ccKindsForm' },
    history: { grid: 'StringGrid3', total: 'APHeadLabel14', rg: 'rgContactCountHistory',   rgForm: 'rgContactCountHistoryForm',   act: 'ccHistory', actForm: 'ccHistoryForm' }
  };
  function renderRadio(id, r, act) {
    var fs = $(id);
    if (!fs || !r) return;
    var cli = fs.querySelector('.cli');
    if (!cli) return;
    var key = JSON.stringify(r.items);
    if (cli.getAttribute('data-items') !== key) {           // 項目變了才重建（FormShow 依機台排數換 Row-A..D）
      var h = '';
      r.items.forEach(function (t, i) {
        h += '<label class="rgi"><input type="radio" name="rg_' + id + '" value="' + i + '">' + esc(t) + '</label>';
      });
      cli.innerHTML = h;
      cli.setAttribute('data-items', key);
      cli.querySelectorAll('input[type=radio]').forEach(function (x) {
        x.addEventListener('change', function () { if (x.checked) send({ act: act, arg: +x.value }); });
      });
    }
    cli.querySelectorAll('input[type=radio]').forEach(function (x) { x.checked = (+x.value === r.itemIndex); });
    fs.title = id + ' : TRadioGroup（golden OnClick → WriteContactKind，cObserver.cpp:3220-3241）';
  }
  function renderCounterGrid(g, c) {
    var host = $(g.grid);
    if (!host || !c) return;
    var nData = Math.max(0, c.cols - 1);
    var groups = Math.min(8, Math.floor(nData / 2));       // MAX_SOCKET_COL=8
    var h = '<table class="ccTable" data-cc="' + g.grid + '"><thead><tr><th class="cclab" data-rowlab rowspan="2">' + esc(c.rowLabel) + '</th>';
    for (var i = 0; i < groups; i++) h += '<th colspan="2">Col-' + String.fromCharCode(97 + i) + '</th>';
    h += '</tr><tr>';
    for (var k = 0; k < groups * 2; k++) h += '<th><div class="ccarm">' + esc(c.arm[k] || '') + '</div></th>';
    h += '</tr></thead><tbody><tr><td class="cclab">Head</td>';
    for (k = 0; k < groups * 2; k++) h += '<td class="ccval">' + esc(c.head[k] !== undefined ? c.head[k] : '') + '</td>';
    h += '</tr><tr><td class="cclab">Socket</td>';
    for (i = 0; i < groups; i++) h += '<td class="ccval" colspan="2">' + esc(c.socket[i] !== undefined ? c.socket[i] : '') + '</td>';
    h += '</tr></tbody></table>';
    host.innerHTML = h;
    host.title = g.grid + '（golden WriteContactKind／' + g.grid + 'DrawCell）：C++ observer.get，只在開頁與點 Raw No.／Display Form 時重寫（golden 同）';
    var tot = $(g.total);
    if (tot) { showText(tot, c.total); tot.title = g.total + '：golden ' + g.grid + 'DrawCell 把 Socket 列 atoi 加總（% 模式也照加，golden 的怪處）'; }
    renderRadio(g.rg, c.row, g.act);
    renderRadio(g.rgForm, c.form, g.actForm);
  }
  function renderCounter(c) {
    renderCounterGrid(CC.kinds, c.kinds);
    renderCounterGrid(CC.history, c.history);
  }

  // ---- AI(W906-PROD-S116) 20260926：Yield 分頁（tsYield 的 mtRowA..D 與 ChartYield）------------------------------------------
  //   mtRow 的格子＝golden SetSiteYieldDiagram（:771-836）畫的表頭／site 名＋UpdateBin（:873-904）抄進去的 HistroyBin；
  //   x＝時間（0 欄是 site 名，1..10＝Now／Last 1..9），y＝site（0 列是表頭）。ChartYield 見下面 renderYieldChart（AI(W906-PROD-S116) 20260927）。
  // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：格子可以點 → observer.get {act:'yieldSite', arg:0..3, text:'x,y'}。
  //   golden dfm 四顆 TTMyTray 都是 OnMouseUp = mtRowAMouseUp（cObserver.cpp:3244-3273），處理器不看是哪一顆滑鼠鍵 → 這裡也聽 mouseup
  //   （左／中／右鍵都算，右鍵的瀏覽器選單擋掉）。送的是格子座標（跟 C++ cells[y][x] 同一組），C++ 換回那一格中央的像素交給 golden。
  //   Y>0（site 列）切換那一條線（X 不看，整列任一格都一樣）；Y==0（表頭列：Show All／Hide All、Now、Last n）整排切換。
  var TRAY_IDX = { mtRowA: 0, mtRowB: 1, mtRowC: 2, mtRowD: 3 };   // mtRow[0..3]（golden 建構子 :141-144；Tag＝0..3，:152）
  function cellOf(host, n) {
    while (n && n !== host) {
      if (n.getAttribute && n.getAttribute('data-cx') !== null) return n;
      n = n.parentNode;
    }
    return null;
  }
  function wireTray(host, name) {
    if (host.getAttribute('data-yw') === '1' || !Object.prototype.hasOwnProperty.call(TRAY_IDX, name)) return;
    host.setAttribute('data-yw', '1');
    host.addEventListener('mouseup', function (e) {
      var c = cellOf(host, e.target);
      if (!c) return;
      yieldAct({ act: 'yieldSite', arg: TRAY_IDX[name], text: c.getAttribute('data-cx') + ',' + c.getAttribute('data-cy') });
    });
    host.addEventListener('contextmenu', function (e) { if (e && e.preventDefault) e.preventDefault(); });
  }
  function renderYield(y) {
    (y.trays || []).forEach(function (t) {
      var host = $(t.name);
      if (!host) return;
      host.style.display = t.visible ? '' : 'none';
      if (t.width) host.style.width = t.width + 'px';
      // AI(W906-PROD-S116Y) 20260927：字色＝dfm Font.Color（clYellow；golden DrawTray Canvas->Font=Font）。表頭 (0,0) 底色是黑的，
      //   原本的黑字讓 "Show All／Hide All" 看不見。舊版 C++ 沒送 fontColor 時照舊（CSS 黑字）。
      var fc = t.fontColor ? 'color:' + t.fontColor + ';' : '';
      var h = '<table class="mrGrid">';
      for (var r = 0; r < t.y; r++) {
        h += '<tr>';
        for (var x = 0; x < t.x; x++) {
          var txt = (t.cells[r] && t.cells[r][x] !== undefined) ? t.cells[r][x] : '';
          var ci = (t.colorIndex[r] && t.colorIndex[r][x]) || 0;
          var bg = t.colorMap[ci] || t.colorMap[0];
          var tagName = (r === 0 || x === 0) ? 'th' : 'td';
          h += '<' + tagName + ' data-cx="' + x + '" data-cy="' + r + '" style="background:' + bg + ';' + fc + '">' + esc(txt) + '</' + tagName + '>';   // AI(W906-PROD-S116Y) 20260927：data-cx／data-cy＝格子座標
        }
        h += '</tr>';
      }
      h += '</table>';
      host.innerHTML = h;
      host.title = t.name + ' : TTMyTray（' + t.x + '×' + t.y + '）—— golden SetSiteYieldDiagram＋UpdateBin，C++ observer.get' +
                   (y.loaded ? '' : '（還沒點過 Yield 頁：golden 第一次點才畫）') +
                   '\n點格子＝golden mtRowAMouseUp（:3244-3273）：site 那一列切換那一條線；表頭那一列（Show All／Hide All、Now、Last n）整排切換';   // AI(W906-PROD-S116Y) 20260927
      wireTray(host, t.name);                      // AI(W906-PROD-S116Y) 20260927：只綁一次（事件掛在 host，格子每次重建也有效）
    });
    renderYieldChart(y.chart);                     // AI(W906-PROD-S116) 20260927
  }

  // ---- AI(W906-PROD-S116) 20260927（Steven 團隊）：ChartYield（TChart，dfm Left=0 Top=0 924×249，Align=alTop）-----------------------
  //   C++ W906Obs_YieldChartJson 送的是 golden UpdateYieldChart（cObserver.cpp:838-864）AddY 進 ChartYield 32 條 Series 的點
  //   （上一次 UpdateYieldChart 的內容；golden 的圖也只在那時候換）：
  //     labels[25]    X 軸標籤：偶數點 "%02d:%02d"（RunInfo.iYieldHour／iYieldMin），奇數點空字串（:855-856）。
  //                   X＝0 在最左＝RunInfo.iYieldChart[..][..][0]＝最新，往右越舊（AddY 依序加點；dfm 沒有 BottomAxis.Inverted）。
  //     series[32]    {title,color,width,active,values?}，編號＝row*8+col（SeriesAa..SeriesDh）。color＝點的顏色 TC[row*8+col]
  //                   （golden AddY 第三個參數；也是 mtRow site 格的底色），width＝dfm LinePen.Width（A、B 排 2，C、D 排沒設＝1），
  //                   active＝golden Series->Active（bShowYieldSeries），只有 active 的線才帶 values（golden 不畫的線不送）。
  //     leftAxis      {minimum,maximum,increment,title}＝golden LeftAxis（dfm -5／105／10／'Yield (%)'，Automatic=False）。
  //     edYieldMax／edYieldMin  兩個 TEdit 的 Text（dfm '100'／'0'）。golden 點框開 QWERTY 鍵盤、改完才把軸設成框裡的值
  //                   （edYieldMaxClick :3208-3212／edYieldMinClick :3214-3218）—— 這一版不接，框設唯讀、只顯示 C++ 的值。
  //                   AI(W906-PROD-S116Y) 20260927：接了 —— 框可以編輯（wireYieldEdit），act yieldMax／yieldMin；limits＝golden 鍵盤顯示的上下限。
  //   畫法：inline SVG（這個目錄沒有既有的圖表工具；不引外部函式庫）。dfm Title.Visible=False、Legend.Visible=False、View3D=False、
  //   Pointer.Visible=False → 只畫底色、格線、軸標籤、軸標題、折線。繪圖區（YC）是照 TeeChart 預設邊界（左右 3%、上下 4%）＋軸標籤寬度
  //   推的近似值，不是 dfm 值；格線照 TeeChart 預設（灰色點線），線超出軸範圍的部分裁掉（TeeChart 預設 ClipPoints）。
  //   沒改就不重畫（timer 每秒帶一次，JSON 一樣就跳過）。
  var YC = { L: 74, R: 896, T: 10, B: 220 };
  var ycKey = null;
  function ycSeriesName(i) { return 'Series' + String.fromCharCode(65 + Math.floor(i / 8)) + String.fromCharCode(97 + (i % 8)); }
  function ycNum(v) { return Math.round(v * 10) / 10; }
  function yieldSvg(c) {
    var W = c.width || 924, H = c.height || 249, L = YC.L, R = YC.R, T = YC.T, B = YC.B;
    var ax = c.leftAxis || {}, mn = +ax.minimum, mx = +ax.maximum, inc = +ax.increment;
    var okAxis = isFinite(mn) && isFinite(mx) && mx > mn;
    if (!(inc > 0)) inc = 10;
    var n = +c.points || 0, labels = c.labels || [];
    function xOf(k) { return n > 1 ? L + k * (R - L) / (n - 1) : (L + R) / 2; }
    function yOf(v) { return B - (v - mn) * (B - T) / (mx - mn); }
    var h = '<svg xmlns="http://www.w3.org/2000/svg" class="ycSvg" width="' + W + '" height="' + H + '" viewBox="0 0 ' + W + ' ' + H +
            '" font-family="Arial,sans-serif" style="display:block;">';
    h += '<defs><clipPath id="ycClip"><rect x="' + L + '" y="' + T + '" width="' + (R - L) + '" height="' + (B - T) + '"/></clipPath></defs>';
    h += '<rect x="0" y="0" width="' + W + '" height="' + H + '" fill="' + esc(c.color || '#CCD9DF') + '"/>';   // dfm Color／BackColor／BackWall.Color
    if (okAxis) {                                                   // Y 軸：Increment 的整數倍（-5..105 → 0,10..100）
      for (var v = Math.ceil(mn / inc - 1e-9) * inc; v <= mx + 1e-9; v += inc) {
        var y = ycNum(yOf(v));
        h += '<line x1="' + L + '" x2="' + R + '" y1="' + y + '" y2="' + y + '" stroke="#808080" stroke-dasharray="1,2"/>';
        h += '<line x1="' + (L - 4) + '" x2="' + L + '" y1="' + y + '" y2="' + y + '" stroke="#000"/>';
        h += '<text x="' + (L - 6) + '" y="' + (y + 4) + '" font-size="11" text-anchor="end" fill="#000">' + esc(String(ycNum(v))) + '</text>';
      }
    }
    for (var k = 0; k < n; k++) {                                   // X 軸：有字的點才畫標籤、刻度、格線
      if (!labels[k]) continue;
      var x = ycNum(xOf(k));
      h += '<line x1="' + x + '" x2="' + x + '" y1="' + T + '" y2="' + B + '" stroke="#808080" stroke-dasharray="1,2"/>';
      h += '<line x1="' + x + '" x2="' + x + '" y1="' + B + '" y2="' + (B + 4) + '" stroke="#000"/>';
      h += '<text x="' + x + '" y="' + (B + 16) + '" font-size="11" text-anchor="middle" fill="#000">' + esc(labels[k]) + '</text>';
    }
    h += '<rect x="' + L + '" y="' + T + '" width="' + (R - L) + '" height="' + (B - T) + '" fill="none" stroke="#000"/>';
    var ty = (T + B) / 2;
    h += '<text x="30" y="' + ty + '" font-size="13" text-anchor="middle" fill="#000" transform="rotate(-90 30 ' + ty + ')">' + esc(ax.title || '') + '</text>';
    var drawn = 0;
    if (okAxis) {
      h += '<g clip-path="url(#ycClip)" fill="none" stroke-linejoin="round">';
      (c.series || []).forEach(function (s, i) {
        if (!s || !s.active || !s.values || !s.values.length) return;
        var pts = s.values.map(function (val, kk) { return ycNum(xOf(kk)) + ',' + ycNum(yOf(+val)); }).join(' ');
        h += '<polyline data-series="' + i + '" points="' + pts + '" stroke="' + esc(s.color) + '" stroke-width="' + (s.width || 1) + '">' +
             '<title>' + esc(ycSeriesName(i) + ' : TLineSeries（' + s.title + '）\n良率 %（最左＝最新）：' + s.values.join(', ')) + '</title></polyline>';
        drawn++;
      });
      h += '</g>';
    } else {
      h += '<text x="' + ((L + R) / 2) + '" y="' + ty + '" font-size="11" text-anchor="middle" fill="#c00">LeftAxis 範圍不合法（minimum=' +
           esc(String(ax.minimum)) + '，maximum=' + esc(String(ax.maximum)) + '）—— 不畫線</text>';
    }
    if (n === 0) {
      h += '<text x="' + ((L + R) / 2) + '" y="' + (ty + 4) + '" font-size="11" text-anchor="middle" fill="#666">' +
           '（還沒有點：golden 第一次點 Yield 頁才跑 UpdateYieldChart）</text>';
    }
    h += '</svg>';
    return { html: h, drawn: drawn };
  }
  // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：edYieldMax／edYieldMin 的編輯。golden：點框（OnClick）開 QWERTY 數字鍵盤（ShowModal），
  //   打字後 Enter／Summit 送出、Cancel 還原 —— 兩種關法都會跑 ShowQwertyKey 的尾巴（CheckRange → 寫回框），處理器再設軸；沒改字也一樣。
  //   這裡：框拿到焦點＝開鍵盤（記住舊字）；Enter 或離開框（blur）＝送出目前的字；Esc＝字還原成舊字再送出（＝golden Cancel）。
  //   視窗切走造成的 blur 不算關鍵盤（document.hasFocus() 為 false 時不送，回來繼續打）。
  //   字的檢查照 golden N_INTEGER 鍵盤打得出來的樣子（數字＋開頭一個 '-'，可以是空的；C++ 同一條規則再擋一次）：不合就還原、不送。
  function wireYieldEdit(ed, act) {
    if (ed.getAttribute('data-yw') === '1') return;
    ed.setAttribute('data-yw', '1');
    var before = null;
    ed.addEventListener('focus', function () {
      if (ed.getAttribute('data-editing') === '1' && before !== null) return;   // 視窗切回來：同一次鍵盤
      ed.setAttribute('data-editing', '1');
      before = ed.value;
    });
    ed.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') ed.blur();
      else if (e.key === 'Escape') { if (before !== null) ed.value = before; ed.blur(); }
    });
    ed.addEventListener('blur', function () {
      if (typeof document.hasFocus === 'function' && !document.hasFocus()) return;
      ed.removeAttribute('data-editing');
      before = null;
      var v = String(ed.value);
      if (!/^-?[0-9]*$/.test(v) || v.length > 11) {
        say(ed.id + ' 只收整數（golden N_INTEGER 鍵盤只打得出數字與開頭的 -）：「' + v + '」不送，還原成 C++ 的值', '#c60');
        ed.value = ed.getAttribute('data-server') || '';
        return;
      }
      yieldAct({ act: act, text: v });
    });
  }
  function renderYieldChart(c) {
    var ch = $('ChartYield');
    if (!ch || !c) return;
    var note = $('obsYieldChartNote');
    if (c.noSource) {                                               // 舊版 C++（S113 之前）：照舊顯示 ---
      if (!note) {
        note = document.createElement('div');
        note.id = 'obsYieldChartNote';
        note.style.cssText = 'position:absolute;left:60px;top:90px;right:20px;text-align:center;color:#666;font-size:11px;';
        ch.appendChild(note);
      }
      note.textContent = 'ChartYield：---（沒有來源）';
      note.title = c.noSource;
      ch.title = 'ChartYield : TChart —— ' + c.noSource;
      return;
    }
    if (note) note.parentNode.removeChild(note);
    [['edYieldMax', c.edYieldMax, 'edYieldMaxClick :3208-3212', 'Maximum', 'yieldMax'],
     ['edYieldMin', c.edYieldMin, 'edYieldMinClick :3214-3218', 'Minimum', 'yieldMin']].forEach(function (e) {
      var ed = $(e[0]);
      if (!ed || e[1] === undefined) return;
      // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：可以編輯（見 wireYieldEdit）；打字中（data-editing）不蓋掉，timer 每秒帶一次值
      wireYieldEdit(ed, e[4]);
      ed.readOnly = false;
      ed.setAttribute('data-server', e[1]);
      if (ed.getAttribute('data-editing') !== '1') {
        ed.value = e[1];
        ed.setAttribute('value', e[1]);
      }
      var lim = c.limits && c.limits[e[0]];
      ed.title = e[0] + ' : TEdit（C++ Text＝"' + e[1] + '"）—— golden ' + e[2] + '：點框開 QWERTY 數字鍵盤（N_INTEGER）' +
                 (lim ? '，上下限 ' + lim.min + '..' + lim.max : '') + '。\nEnter 或離開框＝送出（Summit），Esc＝還原（Cancel；golden 也照舊字再跑一次）；' +
                 'C++ 照 golden CheckRange 夾在上下限內、寫回框，再把 ChartYield->LeftAxis->' + e[3] + ' 設成 atoi(框的字)';
    });
    var key = JSON.stringify(c);
    var host = $('obsYieldSvgHost');
    if (host && key === ycKey) return;
    ycKey = key;
    if (!host) {
      host = document.createElement('div');
      host.id = 'obsYieldSvgHost';
      host.style.cssText = 'position:absolute;left:0;top:0;';
      ch.insertBefore(host, ch.firstChild);                         // 放最底層：SpeedButton1／edYieldMax／edYieldMin 仍在上面
    }
    var r = yieldSvg(c);
    host.innerHTML = r.html;
    host.setAttribute('data-points', String(+c.points || 0));
    host.setAttribute('data-drawn', String(r.drawn));
    var inp = c.inputs || {};
    ch.title = 'ChartYield : TChart —— C++ observer.get：' + (c.src || '') +
               '\n畫了 ' + r.drawn + ' 條（golden Series->Active 的線）；' + (+c.points || 0) + ' 點；X 軸最左＝最新' +
               '\nTestSocket iShtRow×iShtCol＝' + inp.shtRow + '×' + inp.shtCol + '（iMaxRow×iMaxCol＝' + inp.maxRow + '×' + inp.maxCol + '）' +
               '\nLeftAxis ' + (c.leftAxis ? c.leftAxis.minimum + '..' + c.leftAxis.maximum : '?');
  }

  // ---- AI(W906-PROD-S116) 20260926：Test Information 分頁（pgcTestInfo 底下的七張 TStringGrid）----------------------------------
  //   固定列／欄（第 0 列、第 0 欄）照 C++ 的字；資料格空白＝沒有資料 → 顯示 "---"（上游見各格 title：RecordTimeInfo 是空殼）。
  //   AI(W906-PROD-S116) 20260926：TimeInfoGrid／strngrdTestTime 的 Index Time 欄（第 5 欄）C++ 已照 golden 算好（ti.indexTime.recorded
  //   為 true 之後才有值；之前仍是 ---）。tsIndexAirOn1／2 只在 golden CosFunction.RecordIndexAirOnTime 時看得到。
  function renderTestInfo(ti) {
    var tabs = document.querySelectorAll('#pgcTestInfo > .pcTabs > .tab');
    tabs.forEach(function (tb) {
      var tt = tb.getAttribute('title') || '';
      if (/^tsIndexAirOn1/.test(tt)) tb.style.display = ti.tabVisible && ti.tabVisible.tsIndexAirOn1 ? '' : 'none';
      if (/^tsIndexAirOn2/.test(tt)) tb.style.display = ti.tabVisible && ti.tabVisible.tsIndexAirOn2 ? '' : 'none';
    });
    var ns = ti.noSource || {};
    Object.keys(ti.grids || {}).forEach(function (id) {
      var g = ti.grids[id], host = $(id);
      if (!host || !g) return;
      var h = '<table>';
      for (var r = 0; r < g.rows; r++) {
        h += '<tr>';
        for (var c = 0; c < g.cols; c++) {
          var v = (g.cells[r] && g.cells[r][c] !== undefined) ? g.cells[r][c] : '';
          var fixed = r < g.fixedRows || c < g.fixedCols;
          if (fixed) h += '<th>' + esc(v) + '</th>';
          else h += '<td' + (v === '' ? ' style="color:#999;"' : '') + '>' + (v === '' ? '---' : esc(v)) + '</td>';
        }
        h += '</tr>';
      }
      h += '</table>';
      host.innerHTML = h;
      host.title = id + ' : TStringGrid（' + g.cols + '×' + g.rows + '，C++ observer.get）' + (ns[id] ? '\n' + ns[id] : '\n"---"＝這一格沒有資料') +
                   ((id === 'TimeInfoGrid' || id === 'strngrdTestTime') && ti.indexTime   // AI(W906-PROD-S116) 20260926
                    ? '\nIndex Time 欄：' + (ti.indexTime.recorded ? '已有 RecordIndexTime 資料（' : '還沒有 Index 完成過（') + (ti.indexTime.source || '') + '）' : '');
    });
    if (ti.notWired && ti.notWired.tsLoadInfo) {
      ['Memo2', 'Memo3', 'Memo4'].forEach(function (id) { var m = $(id); if (m) m.title = id + '：' + ti.notWired.tsLoadInfo; });
    }
  }

  function wire() {
    var y = $('cbbEventLogYear'), m = $('cbbMonth'), f = $('cbbFilter'), q = $('btnQueryEventLogTxt'), b = $('btnBackupLogYear');
    if (y) y.addEventListener('change', function () {
      send({ act: 'year', text: y.options[y.selectedIndex] ? y.options[y.selectedIndex].text : '' });
    });
    if (m) m.addEventListener('change', function () { send({ act: 'month', arg: m.selectedIndex }); });
    if (f) f.addEventListener('change', function () { send({ act: 'filter', arg: f.selectedIndex }); });
    if (q) q.addEventListener('click', function () { send({ act: 'query' }); });
    // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：Yield 分頁 Clear（SpeedButton1）→ golden SpeedButton1Click（:865-872）
    var clr = $('SpeedButton1');
    if (clr) {
      clr.title = 'SpeedButton1 : TSpeedButton（Clear）—— golden SpeedButton1Click（cObserver.cpp:865-872）：把記憶體裡的 HistroyBin 清 0' +
                  '（不清 HistroyPassFail、不寫檔）。golden 按完不重畫格子，要等下一次測完（RecordHistroy → UpdateBin）或點 site 格才看得到變空';
      clr.addEventListener('click', function () { if (!clr.disabled) yieldAct({ act: 'yieldClear' }); });
    }
    if (b) b.addEventListener('click', function () {
      say('尚未接：btnBackupLogYear（golden btnBackupLogYearClick cObserver.cpp:5427 會複製整年記錄）—— 這一版只顯示資料，不寫入', '#c60');
    });
    document.querySelectorAll('#pgcObserv > .pcTabs > .tab').forEach(function (tb) {
      var n = +tb.getAttribute('data-t');
      // AI(W906-PROD-S116) 20260926：放行 0 Counter／4 Yield／6 Test Information（C++ W906_ObserverJson 同步放行）
      if (n === 0 || n === 1 || n === 3 || n === 4 || n === 6) tb.addEventListener('click', function () { if (opened) send({ act: 'tab', arg: n }); });
    });
  }

  function start() {
    // 開頁前先把 dfm 匯出的展示值拿掉（lstEventLog 的 3333/1/1…、evGrid 的說明列），等 C++ 回應
    var lst = $('lstEventLog'); if (lst) lst.innerHTML = '';
    var t = $('evGrid'); if (t) t.innerHTML = '<tbody><tr><td style="text-align:center;color:#666;padding:8px;">讀取中…（observer.get）</td></tr></tbody>';
    wire();
    say('讀取中…（observer.get open）');
    openPage();
    tick = setInterval(onTick, TICK_MS);         // AI(W906-Q2-S124) 20260927：一開始就拍；開頁沒成功時由 onTick 重試開窗
  }
  function openPage() {
    return send({ act: 'open' }).then(function (d) {
      if (d) opened = true;
      return d;
    });
  }
  // AI(W906-Q2-S124) 20260927（Steven 團隊）：沒人看就不拍（同 ht9045_contactct_wire.js visible()）。
  //   golden Timer1 只在視窗開著時跑；分頁在背景、或 background.html 把外框設 display:none（縮小／關掉，iframe 還在）都算沒人看。
  function visible() {
    if (document.hidden) return false;
    try {
      var fe = window.frameElement;               // background.html 的 iframe；直接開這一頁時是 null
      if (fe) { var w = fe.closest && fe.closest('.win'), W = w && window.parent && window.parent.WIN_STATE; if (W) { var st = W[String(w.id).replace(/^win-/, '')]; if (st !== 'open' && st !== 'minimized') return false; } else if (fe.getClientRects().length === 0) return false; }   // AI(W906-STREAM-F2) 20261001: the frame's WIN_STATE decides (RULINGS_20260930 #12; St01 1001 00:39 FYI 3) -- open / minimized = poll, closed / never = stop; golden keeps this form running when MINIMIZED (TfObserver Timer1: enabled in FormShow, disabled only in FormClose, V912 cObserver.cpp:435 / :658), which the old display test (外框 display:none（縮小／關掉）) stopped. No .win / no WIN_STATE = the old display test
    } catch (e) { /* 拿不到 frameElement：當作看得到 */ }
    return true;
  }
  function onTick() {
    if (!visible()) return;
    if (!opened) {                               // 開頁那一下沒成功（例如 wb_serve 還沒起來）：每拍重試開窗
      if (!busy) openPage();
      return;
    }
    send({ act: 'timer' }, true);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045Observer = { send: send, data: function () { return D; }, tick: onTick, isReadAct: isReadAct };
})();
