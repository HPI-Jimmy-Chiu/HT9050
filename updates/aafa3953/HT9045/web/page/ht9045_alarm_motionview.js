/* ht9045_alarm_motionview.js -- 把 Motion View 內嵌進 Alert.Note.html 的 tsHandler
 * ---------------------------------------------------------------------------
 * //Steven 20260924
 * 相關技能：ht9045-alarm-dismissal §9（尤其 §9.3-4、§9.5）
 * 對照表：  web/JSON/Alarm-unit-map.json（產生器 scripts/gen_alarm_unit_map.py）
 * ---------------------------------------------------------------------------
 * 這支做兩件 golden 在 V906 上做不到的事：
 *
 *   1. 補上紅框。golden 的 ShowErrorUnit()（note.cpp:4551）在移植樹零命中，
 *      所以 display.flushPanel 恆為 null，dialog-page.js:75 那行永遠沒東西可做
 *      —— **V906 上那張 Handler 圖從來沒亮過**。
 *      但 arguments.position 是活的（wb_serve.cpp 的 DialogMailboxPostAlarm
 *      已經在寫 "position":%d），所以這裡查表就能算出 panel。
 *
 *   2. 在 Panel5 上緣疊一張整機圖（Alert.MotionView.html，947x355），
 *      把出事的模組標紅。
 *
 * 為什麼是「疊」不是「換」
 * ---------------------------------------------------------------------------
 * tsHandler 是 note.cpp 裡 27 處 `->Parent=tsHandler` 的容器（16 個控制項），
 * 還有一個 11 個面板輪流用的訊息槽 (160,290,680,160)。換掉 = 那些全部失去家。
 * 疊在 Panel5 y 0..355 則：
 *   - 訊息槽在 y267..427，照舊蓋在最上面（它是 tsHandler 的直接子元件）
 *   - 十道安全門九道在 Panel5 外、SafeDoor9 在 y381，全部不受影響
 *   - ATC 九格、palOCR、九個料車、ION FAN 9/10/12 也都在切線以下
 *
 * 退回原圖的條件
 * ---------------------------------------------------------------------------
 * panels[p].hideMotionView === true 才隱藏整機圖（只有 16 個 panel 符合：
 * ION FAN 1-8/11、Temp、Scan、CCD、Sys、IF、Load2、ManualAll）。
 * 其餘一律保留整機圖當底圖 —— 就算沒有模組可標紅，它也提供上下文。
 * **絕不顯示一張沒有紅點、又看不出哪裡不對的整機圖。**
 */
(function (global) {
  'use strict';

  var MAP = null, frame = null, host = null, lastKey = null;

  /* ---- 對照表：http 走 JSON，file: 走墊片 ------------------------------- */
  function jsonBase() {
    var p = location.pathname.replace(/\\/g, '/');
    return p.substring(0, p.lastIndexOf('/') + 1) + '../JSON/';
  }
  /* http 走 JSON，file: 走墊片（Edge 在 file: 下不支援 fetch 這個 scheme） */
  function loadJson(name) {
    return fetch(jsonBase() + name + '.json', { cache: 'no-store' })
      .then(function (r) { if (!r.ok) throw 0; return r.json(); })
      .catch(function () {
        return new Promise(function (res, rej) {
          var store = global.__HT9045_DATA__ = global.__HT9045_DATA__ || {};
          if (store[name]) return res(store[name]);
          var sc = document.createElement('script');
          sc.src = jsonBase() + 'js/' + name + '.js?_=' + Date.now();
          sc.onload = function () {
            sc.remove();
            store[name] ? res(store[name]) : rej(new Error('empty shim ' + name));
          };
          sc.onerror = function () { sc.remove(); rej(new Error('shim 載入失敗 ' + name)); };
          document.head.appendChild(sc);
        });
      });
  }
  function loadMap() {
    if (MAP) return Promise.resolve(MAP);
    return loadJson('Alarm-unit-map').then(function (m) { MAP = m; return m; });
  }

  /* ---- position -> panel -> 模組 --------------------------------------- */
  function resolve(position, machine) {
    var u = MAP.units[String(position)];
    var panel = (u && u.panel) || MAP.defaultPanel;
    var info = MAP.panels[panel] || {};
    var key = (machine === 'HT9050') ? 'mv9050' : 'mv9045';
    /* 逐 unit 覆寫優先（使用者 20260924）：tsHandler 沒有專屬面板、但整機圖
     * 畫得出來的機構 —— In/Out Rotator、Precisor、Auto Clean。
     * panel 仍是 note.dfm 的事實，只有 mv 指得更精確。
     * 有覆寫就代表「有模組可指」，所以不套 hideMotionView（那條是給
     * 「panel 被內嵌區蓋住、整機圖又沒有對應物」用的）。 */
    var over = u && u.mvFrom === 'unit-override';
    var mv = over ? u[key] : info[key];
    return {
      unit: u ? u.unit : null,
      origin: u ? u.origin : 'default',
      panel: panel,
      pending: info.status === 'pending-dfm',
      mvModule: mv || null,
      mvFrom: over ? 'unit' : 'panel',
      hide: !over && info.hideMotionView === true
    };
  }

  /* ---- 機種：決定要疊哪一頁 ---------------------------------------------
   * background.html 解析完機種後會留一個全域 MACHINE（settings.js
   * resolveMachine()：?machine= > Version.Model 前綴 > localStorage > 預設）。
   * 同源 file:// 下 iframe 讀得到 parent，所以直接問它最準。
   * ⚠ 不要用 C++ 的 MachineTypeChoice —— 那個 tag 還沒發布
   *   （Type_HT9050=800 只在 V906 樹，全樹 0 處比對，見 skill §9.6）。
   * ⚠ Alert.MotionView9050.html 有機種閘門（m.id!=="HT9050" 整頁 gate），
   *   所以一定要帶 ?machine=HT9050。 */
  function machineId() {
    try { if (parent && parent.MACHINE && parent.MACHINE.id) return parent.MACHINE.id; } catch (e) {}
    var q = /[?&]machine=([A-Za-z0-9_-]+)/.exec(location.search);
    return q ? q[1] : 'HT9045';
  }

  /* ---- 交叉驗證：code 裡的單元編號 vs position 推出來的 panel ------------
   * Alarm Code = 前綴 + **單元編號 2 碼** + 代碼（手冊 AlarmCode_Manual）。
   * golden 自己也把它拆出來顯示：note.cpp:4408 Edit3=Code.SubString(4,2)，
   * 就是畫面上的 Subsidiary 欄。
   *
   * 同一則告警因此有兩個獨立來源在講「哪個單元」。golden 不可能不一致 ——
   * 兩者都來自同一個 ShowErrorMessage 呼叫點（例如 JAM0109 的四個呼叫點
   * 全都是 ShowErrorMessage("JAM0109", ..., MInArmX, ...)，unit 01 = 入料臂）。
   * 所以對不上就是**產生端餵了不可能的組合**，要大聲講，不能默默畫上去。
   * ⚠ 單元號不指向單一機構的那幾個（Event/System/ESD/Log/Cassette/
   *   Motor 24 / Cylinder 31）在對照表裡 check:false，不參與比對。 */
  function unitCheck(code, panel) {
    var u = String(code || '').substr(3, 2);          // 同 Code.SubString(4,2)
    var row = MAP.codeUnits && MAP.codeUnits[u];
    if (!row || !row.check || !row.panels.length) return null;
    if (row.panels.indexOf(panel) >= 0) return null;
    return { unitNo: u, unitName: row.name, zh: row.zh,
             expect: row.panels.join(' / '), got: panel };
  }

  /* ---- 紅框：補 dialog-page.js:75 做不到的那一步 ----------------------- */
  function flush(panel, pending) {
    var prev = document.querySelectorAll('.dbFlush');
    for (var i = 0; i < prev.length; i++) prev[i].classList.remove('dbFlush');
    // panel 還沒進 note.dfm（例如 palMagazineTray）-> 退回 golden 的預設出口
    var id = pending ? MAP.defaultPanel : panel;
    var el = document.getElementById(id);
    if (el) el.classList.add('dbFlush');
    return !!el;
  }

  /* ---- 外框配色：安全門環與外側軌道改成 Motion View 風格 ----------------
   * 使用者 20260924 指定。整機圖疊上來之後，四周還留著 dfm 的 #517b91 深青
   * 色塊，兩種視覺語言並置很跳。這裡把門環（後排 5 道／左右 4 道／前 1 道）、
   * Tray Arm 軌、以及下方的料車統一成 Motion View 的色票。
   *
   * ⚠ dfm 產生的面板背景與框線是**行內樣式**，所以每一條都要 !important。
   * ⚠ dbFlush 的 keyframes 寫死 #517b91 當「滅」的那半（dialog-page.js:135），
   *   改了底色不換動畫的話，閃爍會在新舊兩種顏色之間跳。所以另給一組 almFlush。
   * ⚠ 只在 body.almMvOn 生效 —— 整機圖沒疊上來時（hideMotionView 那 16 個
   *   panel）維持 golden 原樣，不要動人家的畫面。 */
  function injectSkin() {
    if (document.getElementById('almSkin')) return;
    var st = document.createElement('style');
    st.id = 'almSkin';
    st.textContent = [
      ':root{--mvmech:#7d879c;--mvrail:#aab2c4;--mvplate:#e7eaf1;',
      '  --mvplate2:#dbe0ea;--mvink:#2b3648;--mvdim:#5d6a80;--mvhot:#c0392b}',
      ':root[data-theme="dark"]{--mvmech:#5b6577;--mvrail:#6c7689;--mvplate:#2b3242;',
      '  --mvplate2:#333c4e;--mvink:#dde3ee;--mvdim:#9aa6bb}',
      /* 門環與料車區的外容器：讓底色透出來，不要再有一塊實心板 */
      'body.almMvOn #pnlSafeDoorLeft,body.almMvOn #pnlSafeDoorRight,',
      'body.almMvOn #pnlSafeDoorRear,body.almMvOn #pnlTrayCar,',
      'body.almMvOn #Panel8,body.almMvOn #Panel9{background:transparent!important;border:0!important}',
      /* 安全門：十道門已經畫進斜投影裡了（Alert.MotionView.html 的 almDoors()，
       * 使用者 20260924 指定），外框這一圈 dfm 面板就是重複 -> 整組收起來。
       * 它們照樣會拿到 dbFlush，只是看不到；真正閃的是圖裡的那一道。 */
      'body.almMvOn [id^="palSafeDoor"]{display:none!important}',
      /* Tray Arm 軌：Motion View 的 rail 色 */
      'body.almMvOn #palTrayArm2{background:var(--mvrail)!important;',
      '  border:1px solid var(--mvmech)!important;border-radius:3px!important}',
      'body.almMvOn #palTrayArm2 > .pnlCap{color:var(--mvink)!important;',
      '  font-family:var(--font-ui,"Microsoft JhengHei",sans-serif)!important}',
      /* 外側料車：已經用 §4-1 的 createTrayStatusUnit() 畫進斜投影裡了
       * （IDE.WidgetTemplates.html：名稱＋Sensor LED＋Tray 外框＋平行四邊形格點）。
       * dfm 的 HTML 面板是軸對齊的，規格明寫「不可用軸對齊 rect」，
       * 拿 CSS skewX 去仿只是外型像而已 —— 整列收起來，以圖裡那一排為準。 */
      'body.almMvOn [id$="_Car"],body.almMvOn #palTrayArm2{display:none!important}'
    ].join('');
    document.head.appendChild(st);
  }

  /* ---- 內嵌整機圖 ------------------------------------------------------- */
  function ensureFrame() {
    if (frame) return frame;
    host = document.getElementById('Panel5');
    if (!host) return null;
    injectSkin();
    if (getComputedStyle(host).position === 'static') host.style.position = 'relative';
    var box = document.createElement('div');
    box.id = 'almMvHost';
    /* 吃滿整個 Panel5。
     * 原本只佔上半 355（Tray Arm 軌 361 以上），是為了讓 dfm 的 Tray Arm 軌、
     * SafeDoor9 與料車列露出來。現在門環與料車都畫進斜投影裡了，下半段沒有
     * 還需要露出的東西，讓圖吃滿反而字才看得清 —— viewBox 因為多了料車列
     * 從 500 長到 632，高度不跟著給就會糊掉。
     * 訊息槽（tsHandler 的 (160,290,680,160)）是 tsHandler 的直接子元件，
     * 照樣蓋在這一層上面，跟 golden 一樣。 */
    box.style.cssText = 'position:absolute;left:0;top:0;right:0;bottom:0;' +
                        'z-index:4;background:var(--form-bg,#ece9d8);display:none;' +
                        'overflow:hidden';
    frame = document.createElement('iframe');
    frame.id = 'almMvFrame';
    frame.title = 'Motion View（告警）';
    frame.style.cssText = 'border:0;display:block;width:100%;height:100%';
    var mode = (/[?&]mode=(release|debug)/.exec(location.search) || [])[1] || 'release';
    frame.src = (machineId() === 'HT9050')
      ? ('Alert.MotionView9050.html?machine=HT9050&mode=' + mode)
      : ('Alert.MotionView.html?mode=' + mode);
    box.appendChild(frame);
    host.appendChild(box);
    frame.__box = box;
    return frame;
  }

  /* ⚠ iframe 是這一刻才建的，postMessage 會跟它的載入搶跑（實測：早送的那幾發
   *   全部掉在地上，畫面只有底圖沒有紅框）。所以一律先存起來，等子頁回報
   *   HT_ALARM_MV_READY 再送；另外保留幾發重試，避免 ready 訊息自己掉了。 */
  var waiting = null;
  function pump() {
    if (!waiting || !frame) return;
    try { frame.contentWindow.postMessage(waiting, '*'); } catch (e) {}
  }
  function showMv(payload) {
    var f = ensureFrame();
    if (!f) return;
    f.__box.style.display = 'block';
    document.body.classList.add('almMvOn');
    waiting = payload;
    pump();
    [150, 400, 900, 1800, 3000].forEach(function (t) { setTimeout(pump, t); });
  }
  function hideMv() {
    waiting = null;
    document.body.classList.remove('almMvOn');
    if (frame) frame.__box.style.display = 'none';
  }

  /* ---- reDescription：照 TfNote::ErrShowToForm() 填 ----------------------
   * golden note.cpp:4347-4520。`DEBUG_NEW_ALARM_DESCRIPTION` 在
   * MachineType.h:17 是註解掉的，所以跑的是 #else 那條：
   *
   *     TextPath = D:\\HT9045\\Error\\<Language>\\<Code>.dat      (iMotorErr == -1)
   *              = D:\\HT9045\\Error\\<Language>\\MOT<n>.dat      (馬達錯誤)
   *     if(FileExists) { reDescription->Lines->LoadFromFile(TextPath);
   *                      reBigDescription->Lines->LoadFromFile(TextPath); }
   *
   * ⚠ 檔案不存在時 golden **什麼都不做**（停在 Clear() 後的空白），
   *   不是顯示「查無資料」。這裡照做：查不到就不覆寫 dialog-page.js already
   *   填進去的 display.description，並在 console 說一聲。絕不編內容。
   * ⚠ 馬達錯誤那條（MOT<n>.dat）現在接不了 —— request 沒有帶 iMotorErr，
   *   C++ 端 ForwardShowErrorMessage 的簽章只有 (code,kcode,pos)。要接得先
   *   補這個欄位。這裡只走 <Code>.dat。
   * 資料：web/JSON/Alarm-description.json（scripts/gen_alarm_description.py
   *       從 Error\\<Language>\\*.dat 全文抽出，948 碼有全文、3079 碼有短描述）*/
  var DESC = null, DESC_TRIED = false;
  function loadDesc() {
    if (DESC || DESC_TRIED) return Promise.resolve(DESC);
    DESC_TRIED = true;
    return loadJson('Alarm-description').then(function (d) { DESC = d; return d; },
      function (e) { if (global.console) console.warn('[alarm-mv] 描述檔載入失敗', e); return null; });
  }
  /* golden 的語系決策在 C++（CUSTOMER_CODE / IniConfig.iUserLanguage /
   * LastSet.iLanguageCountry）—— 前端拿不到，所以只做這三層，
   * 並且**不猜**：都問不到就用 Chinese，英文當備援。 */
  function langOf() {
    var q = /[?&]lang=([A-Za-z]+)/.exec(location.search);
    if (q) return q[1];
    try { var v = localStorage.getItem('ht9045-alarm-lang'); if (v) return v; } catch (e) {}
    return 'Chinese';
  }
  function setVal(el, text) {
    if (!el) return;
    if (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA') el.value = text || '';
    else el.textContent = text || '';
  }
  function fillDescription(code) { global.__htEvB6AlarmCode = code; if (!global.__htEvB6Lang) { global.__htEvB6Lang = true; global.addEventListener('message', function (ev) { if (ev.data && ev.data.type === 'HT_LANG' && global.__htEvB6AlarmCode) setTimeout(function () { fillDescription(global.__htEvB6AlarmCode); }, 0); }); }   // AI(W906-EVB6) 20260928 [W906]: W42-c 記下目前這一筆；主畫面換語言（background 先寫 localStorage ht9xxx-lang 再廣播 HT_LANG）時重填長說明；同一行
    var re = document.getElementById('reDescription'),
        big = document.getElementById('reBigDescription');
    if (!re || !DESC || !DESC.text) return false;
    var row = DESC.text[code];
    var lang = langOf(); if (lang === 'Chinese' && !/[?&]lang=/.test(location.search)) { try { if (!localStorage.getItem('ht9045-alarm-lang')) { var ml = localStorage.getItem('ht9xxx-lang'); if (ml) lang = ({ zh: 'Chinese', en: 'English', ko: 'Korea' })[ml] || 'English'; } } catch (e) {} }   // AI(W906-EVB6) 20260928 [W906]: W42-c 跟主畫面的語言選單（main.html selLang；en／zh／ja／ko → English／Chinese／English（沒有日文 .dat）／Korea）；langOf() 的 ?lang= 與 ht9045-alarm-lang（:264，St02 登記）仍然優先；只改這一行，不碰 St02 登記的 :37／:264／:352／:392 與它們的鄰行
    var body = row && (row[lang] || row.English || row.Chinese);
    if (!body) {
      if (global.console) console.info(
        '[alarm-mv] %s 沒有 %s 的 .dat 描述 -> 照 golden 留白，不編內容', code, lang);
      return false;
    }
    /* ⚠ reDescription 在 dfm 是 TRichEdit，產生器把它產成 <textarea>。
     *   textContent 對 textarea 只改「預設值」，畫面上不會變 —— 要設 .value。
     *   dialog-page.js:15 的 setText() 已經處理過這件事，這裡照抄它的判斷。 */
    setVal(re, body);
    if (big) setVal(big, body);           // golden 也載同一份（Ifor 20200331）
    return true;
  }

  /* ---- edUnitName / ShowMessageEdit1：也從 Code 推 ------------------------
   * 使用者 20260924 裁定：這部分不必 C++ 處理，前端直接查表。
   *
   * golden note.cpp:856 -> cMyDB.cpp:603-706 MyDBIEvent()：
   *     UnitNo   = atoi(Code.SubString(4,2))
   *     Message  = fMain->AlarmCodeMap[Code]          <- Error\AlarmCodeList.txt
   *     UnitName = fMain->UnitNameMap->Strings[UnitNo] <- AlarmUnit[32] (cMyDB.cpp:32-64)
   *                Code 第 4-5 碼 == "24" -> 強制 "Motor"
   *     查不到 Code -> AlarmID=41、Message="Unknown Alarm Code"
   *
   * ⚠ cMyDB.cpp:677-678 那個 else **是註解掉的** —— 就算 bUseMDB 成立、
   *   前面已經從 SQLite 的 AlarmList 查到 Message，下面這段還是會無條件再跑、
   *   用 AlarmCodeList.txt 的值覆寫掉。所以 **AlarmCodeList.txt 才是實際來源**。
   *
   * 推導是純 Code 查表、沒有任何 runtime 狀態，所以前端做得到，
   * 跟 flushPanel 是同一類（見 §9.2）。
   *
   * 產生端有給值就尊重它、只在**不一致**時出聲；沒給才由前端推。
   * 這樣日後 C++ 真的填了也不會打架，填錯了也不會被蓋掉看不見。 */
  var CODES = null, CODES_TRIED = false;
  function loadCodes() {
    if (CODES || CODES_TRIED) return Promise.resolve(CODES);
    CODES_TRIED = true;
    return loadJson('AlarmCodeList-index').then(
      function (d) { CODES = (d && d.codes) || {}; return CODES; },
      function (e) { if (global.console) console.warn('[alarm-mv] AlarmCodeList 載入失敗', e); return null; });
  }
  function deriveFromCode(code) {
    var uno = String(code || '').substr(3, 2);
    var row = MAP.codeUnits && MAP.codeUnits[uno];
    var msg = CODES && CODES[code];
    return {
      unitNo: uno,
      unitName: (uno === '24') ? 'Motor' : ((row && row.name) || ('Unit ' + uno)),
      message: msg || 'Unknown Alarm Code',      // golden 的 AlarmID=41 那條
      known: !!msg
    };
  }
  function fillFromCode(req) {
    var code = String(req.arguments.code || ''), d = req.display || {};
    var g = deriveFromCode(code);
    [['edUnitName', d.unitName, g.unitName],
     ['ShowMessageEdit1', d.message, g.message]].forEach(function (x) {
      var el = document.getElementById(x[0]);
      if (!el) return;
      var given = (x[1] || '').trim();
      if (!given) { setVal(el, x[2]); return; }
      if (given !== x[2] && global.console) console.warn(
        '[alarm-mv] %s：產生端給的是 %j，但 %s 查表得到 %j —— 採用產生端的值，請查來源',
        x[0], given, code, x[2]);
    });
    if (!g.known && global.console) console.warn(
      '[alarm-mv] AlarmCodeList 裡沒有 %s -> 照 golden 顯示 "Unknown Alarm Code"', code);
  }

  /* ---- 主流程 ----------------------------------------------------------- */
  function onRequest(req) {
    if (!req || !req.arguments) return;
    var key = String(req.requestId) + '/' + String(req.seq);
    if (key === lastKey) return;
    lastKey = key;
    loadMap().then(function () {
      var pos = Number(req.arguments.position) || 0;
      var machine = (machineId() === 'HT9050') ? 'HT9050' : 'HT9045';
      var r = resolve(pos, machine);
      var bad = unitCheck(req.arguments.code, r.panel);
      if (bad && global.console) console.error(
        '[alarm-mv] ⚠ 不一致：%s 的單元編號 %s（%s / %s）與 position=%d 推出的 ' +
        'panel %s 對不上（預期 %s）。golden 不會出現這種組合，請查產生端。',
        req.arguments.code, bad.unitNo, bad.unitName, bad.zh, pos, bad.got, bad.expect);
      var drew = flush(r.panel, r.pending);
      Promise.all([loadDesc(), loadCodes()]).then(function () {
        fillDescription(String(req.arguments.code || ''));
        fillFromCode(req);
      });
      if (r.hide || !r.mvModule) {
        // 沒有可標紅的模組 -> 不疊整機圖，讓 golden 的機構圖自己說話
        hideMv();
      } else {
        showMv({
          type: 'HT_ALARM_MV', mvModule: r.mvModule,
          code: String(req.arguments.code || ''),
          unit: r.unit, panel: r.panel,
          conflict: bad ? ('單元編號 ' + bad.unitNo + '（' + bad.zh + '）與 panel ' +
                           bad.got + ' 不符，預期 ' + bad.expect) : null
        });
      }
      if (global.console && console.info) {
        console.info('[alarm-mv] position=%d unit=%s panel=%s(%s%s) mv=%s -> %s',
          pos, r.unit, r.panel, r.origin, r.pending ? ',pending-dfm' : '',
          r.mvModule, drew ? 'flush ok' : '找不到 panel 元素');
      }
    }, function (e) {
      if (global.console) console.error('[alarm-mv] 對照表載入失敗，維持 golden 行為', e);
    });
  }

  global.addEventListener('message', function (ev) {
    var m = ev.data;
    if (!m) return;
    if (m.type === 'HT_DIALOG_REQUEST' && m.kind === 'alarm') onRequest(m.request);
    if (m.type === 'HT_ALARM_MV_READY') pump();
  });

  /* 手動測試：HT9045AlarmMV.test(35) -> MLoaderZ */
  global.HT9045AlarmMV = {
    test: function (position, code) {
      lastKey = null;
      onRequest({ requestId: 'test', seq: Date.now(),
                  arguments: { position: position, code: code || 'TEST0000', kCode: 0 } });
    },
    resolve: function (position) { return MAP ? resolve(position, 'HT9045') : null; },
    map: function () { return MAP; }
  };
})(window);
