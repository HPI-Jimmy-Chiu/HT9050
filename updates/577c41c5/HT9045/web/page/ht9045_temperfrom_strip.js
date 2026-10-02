/* AI(W906-E027) 20261002 [W906] (St01 ST01-E2)：主畫面溫度條新版（todo E-027，Steven 1002 選定的「縮小版只有 PV／展開版全部資訊」）。
 *   Steven：「客戶比較期望可以一眼看到全部站的溫度」「那個配置挺不錯的, 協助整合進去畫面中吧, 要注意原本就有四種的佈景主題」。
 *   對照依據（畫面名稱、71 通道、各廠牌位址、各機型版面）：D:\HT9045\.claude\skills\ht9045-temperature\references\main-screen-display.md。
 *
 *   資料（全部是 C++ 發布的 tag／設定檔，本檔不猜、不填預設值）：
 *     - 每個通道：temp.zone.<tc>.pv／.comm／.inst（P JsonBridge\StageThermo.cpp:171；今天一律 null ⇒ 格子顯示 ---）
 *     - 設定值與模式：temp.sv（fWorkTemperBase）、temp.mode（0 Hot／1 Ambient／2 ATC／3 AmbientHot）
 *     - 機種：HT9045Live 的 machine.gpibModel（9050GPIB＝HT9050，JSON\Machine-profile.json gpibModelMap）
 *     - 版面開關：/api/system/gerneral [System] USE_16_HEATER、REAL_TIME_CCD、RTC_TemperNumber、CCD2_TEMPER、INSTALL_HEAT_GUN、
 *       INSTALL_ATC_HEAT_GUN、SocketBasedAdd4Temp、LB_TEMP、Index_ESDAir、IndexDoorHeater；[TempCtrl] HEATER_CTRL_TYPE、HeaterInsOpt_<通道>
 *     - 允差：/api/system/config [Tempture] iL04TemptureRange（golden cTemperFrom.cpp:1018-1048 的一般通道允差）
 *   判讀照 golden ShowThermo（cTemperFrom.cpp:587-1480）的子集：沒裝 ---；999 或通訊錯 ERR；<18 「...」；
 *   著色只做「設定值＝fWorkTemperBase、允差＝iL04」的通道（Plate／Shuttle／Head／Index 區），且只在 Hot／AmbientHot；
 *   Chamber、DUT、熱風、CCD 等 golden 用別的設定值或允差 ⇒ 只顯示數值、不著色（不猜）。
 *   展開：請 background.html 把本視窗放大（{resizeMe:{h}}，只接受 temperf），收合送 {resizeMe:null} 恢復 925×140。
 *   多語系（Steven 1002「你有做多國語言支援嗎?」）：介面字以英文為底，譯文在 i18n.js HTI18N.terms（key＝'Temp: '＋英文原文；
 *   字典編輯器 IDE.I18nEditor.html 會保留 terms）；語言＝localStorage ht9xxx-lang（預設 en），background.html 換語言時廣播
 *   {type:'HT_LANG', lang}。通道名稱（Plate 1、SH 1、A1、DUT 1…）照 golden 的英文字樣不翻。 */
(function (global) {
  'use strict';
  var doc = global.document;
  var BRAND = { 0: 'TC401', 1: 'KT4H', 2: 'E5DC', 3: 'No Heater', 4: 'DTK4848', 5: 'EJ1N', 6: 'DTM' };   // golden cmydef.cpp:222-227；5／6＝E029（見 addressOf）
  var MODE = { 0: 'Hot', 1: 'Ambient', 2: 'ATC', 3: 'AmbientHot' };                   // ht9045_wire_main.js temp.mode
  var LANG = 'en';
  try { LANG = global.localStorage.getItem('ht9xxx-lang') || 'en'; } catch (e) { /* 預設 en */ }
  /* 詞條 key＝'Temp: '＋英文原文（只給溫度條用，不跟別頁共用通用字）；查不到或 en ⇒ 顯示英文原文 */
  function T(k) { var D = global.HTI18N, e = D && D.terms && D.terms['Temp: ' + k]; return (e && LANG !== 'en' && e[LANG]) ? e[LANG] : k; }

  /* eTempControll（golden MachineType.h:638-655）與畫面名稱 asTempCtrl（cmydef.cpp:89-109） */
  var IDX = { tcHotPlate1: 0, tcHotPlate2: 1, tcShuttle1: 2, tcShuttle2: 3, tcHead1: 4, tcHead2: 5, tcHead3: 6, tcHead4: 7,
              tcSocket: 8, tcChamber: 9, tcCCD: 10, tcHeatGun1: 27, tcHeatGun2: 28, tcDUT1: 29, tcDUT2: 30, tcDUT3: 31, tcDUT4: 32,
              tc2D: 49, tcLB: 50, tcIndexESD: 51, tcCCD_2: 52, tcATCHotAir1: 53, tcATCHotAir2: 54, tcDoor1: 67, tcDoor2: 68 };
  var LETTER = 'ABCDEFGHIJKLMNOP';
  /* Index 區：排 A／B（r）、欄 a～h（i）、臂 1／2（n）。16 組只到 d（i<4）。 */
  function zone(n, i, r) {
    var code = (r === 0 ? 'A' : 'B') + 'abcdefgh'.charAt(i) + n;
    var idx = i < 4 ? 11 + (n - 1) * 8 + r * 4 + i : 33 + (n - 1) * 8 + r * 4 + (i - 4);
    return { tc: 'tc' + code, idx: idx, short: LETTER.charAt(i * 2 + r) + n, name: LETTER.charAt(i * 2 + r) + n + ' (' + code + ')',
             useSv: true, arm: n, col: i, row: r };
  }
  function ch(tc, short, name, useSv, extra) {
    var c = { tc: tc, idx: IDX[tc], short: short, name: name, useSv: !!useSv };
    if (extra) for (var k in extra) c[k] = extra[k];
    return c;
  }

  /* ---------------- 版面：依機種與 Gerneral.ini 排出群組 ---------------- */
  function layoutHT9050() {
    /* D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md §1、§2：3 站台達 DTM。SLK-1～8 → tcAa1…tcBd1 是推導（SLK-SITE-ORDER 未確認）。 */
    function a(st, c) { return { addr: 'CH' + st + '-' + c }; }                    // 站-CH（Steven 1002：「第x台 CHx」簡化成 CHx-x）
    var slk = ['tcAa1', 'tcAb1', 'tcAc1', 'tcAd1', 'tcBa1', 'tcBb1', 'tcBc1', 'tcBd1'].map(function (tc, k) {
      return { tc: tc, idx: [11, 12, 13, 14, 15, 16, 17, 18][k], short: 'S' + (k + 1), name: 'SLK-' + (k + 1), useSv: true, addr: 'CH3-' + (k + 1) };
    });
    return {
      machine: 'HT9050', sub: T('3-station Delta DTM'),
      groups: [
        { label: T('Hot Plate'), chans: [ch('tcHotPlate1', 'HP1', 'Hotplate 1', true, a(1, 1)), ch('tcHotPlate2', 'HP2', 'Hotplate 2', true, a(1, 2))] },
        { label: T('In Shuttle'), chans: [ch('tcShuttle1', 'SH1', 'In Shuttle 1', true, a(1, 3)), ch('tcShuttle2', 'SH2', 'In Shuttle 2', true, a(1, 4))] },
        { label: 'DUT', chans: [1, 2, 3, 4].map(function (k) { return ch('tcDUT' + k, 'D' + k, 'DUT ' + k, false, a(1, 4 + k)); }) },
        { label: T('Chamber · Hot Air'), chans: [ch('tcChamber', 'CHB', 'Chamber', false, a(2, 1)), ch('tcHeatGun1', 'HA1', 'Hot Air 1', false, a(2, 2)),
                                                 ch('tcHeatGun2', 'HA2', 'Hot Air 2', false, a(2, 3))] },
        /* 縮小版是兩排、照欄往下排：上排 1～4、下排 5～8 */
        { label: T('SLK heads'), chans: [slk[0], slk[4], slk[1], slk[5], slk[2], slk[6], slk[3], slk[7]], expandOrder: slk }
      ]
    };
  }
  function layoutHT9045(sys) {
    var u16 = num(sys.USE_16_HEATER), groups = [];
    var arms = (u16 === 3 || u16 === 4 || u16 === 6) ? 8 : ((u16 === 1 || u16 === 2 || u16 === 5) ? 4 : 0);
    groups.push({ label: T('Plate · SH'), chans: [ch('tcHotPlate1', 'P1', 'Plate 1', true), ch('tcHotPlate2', 'P2', 'Plate 2', true),
                                                 ch('tcShuttle1', 'SH1', 'SH 1', true), ch('tcShuttle2', 'SH2', 'SH 2', true)] });
    if (u16 === 0) {
      /* eht4Heater：Head1／Head2＝臂 0、Head3／Head4＝臂 1（golden main.cpp:20846-20864） */
      groups.push({ label: T('Head'), chans: [ch('tcHead1', 'H12', 'Head 1/2', true), ch('tcHead2', 'H34', 'Head 3/4', true),
                                              ch('tcHead3', 'H56', 'Head 5/6', true), ch('tcHead4', 'H78', 'Head 7/8', true)] });
    }
    for (var n = 1; n <= 2 && arms; n++) {
      var list = [];
      for (var i = 0; i < arms; i++) for (var r = 0; r < 2; r++) list.push(zone(n, i, r));   // 照欄：A 排在上、B 排在下
      groups.push({ label: 'Arm' + n + ' (A–' + LETTER.charAt(arms * 2 - 1) + ' ' + n + ')', chans: list, arm: n, cols: arms });
    }
    var idx = [ch('tcChamber', 'CHB', 'Chamber', false)];
    var dut = num(sys.SocketBasedAdd4Temp);                     // golden MachineType.h:622-624：0 eDut1ea、1 eDut4ea、2 eDut2ea
    if (dut === 1 || dut === 2) {
      for (var k = 1; k <= (dut === 1 ? 4 : 2); k++) idx.push(ch('tcDUT' + k, 'D' + k, 'DUT ' + k, false));
    } else {
      idx.push(ch('tcSocket', 'Dut', 'Dut (Socket)', false));
    }
    if (num(sys.REAL_TIME_CCD) > 0) {
      idx.push(ch('tcCCD', 'CCD', 'CCD', false));
      if (num(sys.RTC_TemperNumber) === 2) idx.push(ch('tcCCD_2', 'CCD1', 'CCD1', false));
    }
    if (num(sys.CCD2_TEMPER) > 0) idx.push(ch('tc2D', 'CCD2', 'CCD2 (2D)', false));
    groups.push({ label: T('Index · others'), chans: idx });
    var ext = [];
    if (num(sys.INSTALL_HEAT_GUN) > 0) ext.push(ch('tcHeatGun1', 'G1', 'Heat Gun 1', false), ch('tcHeatGun2', 'G2', 'Heat Gun 2', false));
    if (num(sys.INSTALL_ATC_HEAT_GUN) > 0) ext.push(ch('tcATCHotAir1', 'AA1', 'ATC air 1', false), ch('tcATCHotAir2', 'AA2', 'ATC air 2', false));
    if (num(sys.LB_TEMP) > 0) ext.push(ch('tcLB', 'LB', 'L/B', false));
    if (num(sys.Index_ESDAir) > 0) ext.push(ch('tcIndexESD', 'ESD', 'ESD Air', false));
    if (num(sys.IndexDoorHeater) === 1) ext.push(ch('tcDoor1', 'Dr1', 'Door 1', false), ch('tcDoor2', 'Dr2', 'Door 2', false));
    if (ext.length) groups.push({ label: T('Hot air · others'), chans: ext });
    return { machine: 'HT9045', sub: arms ? ('Index ' + (arms * 4) + ' ' + T('heaters')) : ('Index 4 ' + T('heaters') + ' (Head)'), u16: u16, groups: groups };
  }

  /* ---------------- 實體位址（main-screen-display.md §3；Steven 1002：寫成 CHx-x） ---------------- */
  function brandOf(c, tempCtrl) {
    var o = num(tempCtrl['HeaterInsOpt_' + c.tc.slice(2)]);
    if (o !== null && o !== -9999) return o;                    // V912 逐通道廠牌；-9999＝跟著 HEATER_CTRL_TYPE
    return num(tempCtrl.HEATER_CTRL_TYPE);
  }
  /* AI(W906-FRW-E029) 20261002 [W906]（St01，todo E-029／Q72；Steven 1002「溫控器少了 DTM」「還有EJ1N也選不到」）：HW.HandlerSys「Heater」
   *   分頁現在可以把任何通道設成 5＝Omron EJ1N／6＝Delta DTM，「各溫控器不同」（HeaterInsMode=1）時存 HeaterInsAddr_<通道>（COM 埠廠牌＝站號、
   *   TC401＝第幾台、EJ1N＝台號、DTM＝內部站號）與 HeaterInsCh_<通道>（EJ1N／DTM 的 CH）。規則：移植樹 FileRW/HSys_Heater.h 檔尾 E029。
   *   這裡照同一套：「不同」模式有寫站號就用寫的；沒寫就用 golden 預設（Index 區 EJ1N／DTM 照 iTempCode 接線、COM 埠廠牌＝序號＋1）；
   *   Index 區以外的 EJ1N／DTM golden 沒有對照、又沒寫 → 「站號未設定」（不猜）。寫法照 Steven「第x台 CHx → CHx-x」：EJ1N CH台-CH、DTM CH站-CH。 */
  function explicitAddr(c, tempCtrl) {
    if (num(tempCtrl.HeaterInsMode) !== 1) return null;         // 「全機相同」不用站號鍵（同 C++ W906_HeaterStationIdx）
    var nm = c.tc.slice(2), a = num(tempCtrl['HeaterInsAddr_' + nm]), ch = num(tempCtrl['HeaterInsCh_' + nm]);
    if (a === null || a === -9999) return null;
    return { st: a, ch: (ch === null || ch === -9999) ? null : ch };
  }
  function addressOf(c, L, tempCtrl) {
    if (c.addr) return c.addr;
    var b = brandOf(c, tempCtrl), x = explicitAddr(c, tempCtrl);
    // Index 區的控制器：Index Heater Counts（USE_16_HEATER）是 EJ1N／DTME08 就是那一種（golden bthermo.cpp:1331-1367），否則看通道自己的廠牌
    var zoneCtl = !c.arm ? null : (L.u16 === 2 || L.u16 === 3) ? 5 : (L.u16 === 5 || L.u16 === 6) ? 6 : (b === 5 || b === 6) ? b : null;
    if (zoneCtl === 5) {                                         // golden bthermo.cpp:3903-3918／:3943-3958：台-CH
      if (x && x.ch !== null) return 'EJ1N CH' + x.st + '-' + x.ch;
      return 'EJ1N CH' + ((c.col < 4 ? 0 : 4) + (c.arm - 1) * 2 + Math.floor((c.col % 4) / 2) + 1) + '-' + ((c.col % 2) * 2 + c.row + 1);
    }
    if (zoneCtl === 6) {                                         // golden cmydef.cpp:111-117 iTempCode：內部站號-CH（0＝DTME08 主機）
      if (x && x.ch !== null) return 'DTM CH' + x.st + '-' + x.ch;
      return 'DTM CH' + ((c.col < 4 ? 0 : 2) + (c.arm - 1)) + '-' + ((c.col % 4) * 2 + c.row + 1);
    }
    if (b === null) return T('Brand unknown');
    if (b === 0) return 'TC401 CH' + (x ? x.st : Math.floor(c.idx / 4) + 1) + '-' + (c.idx % 4);   // 台-通道（通道 0～3，照協定）
    if (b === 3) return 'No Heater';
    if (b === 5 || b === 6) {                                    // Index 區以外的 EJ1N／DTM：只有寫了站號才知道
      return (b === 5 ? 'EJ1N' : 'DTM') + ((x && x.ch !== null) ? ' CH' + x.st + '-' + x.ch : ' ' + T('Station not set'));
    }
    return (BRAND[b] || ('Brand ' + b)) + ' #' + (x ? x.st : c.idx + 1);   // 站號＝序號＋1（面板設十進位）；「不同」模式寫了就用寫的
  }

  /* ---------------- 判讀（golden ShowThermo 的子集，見檔頭） ---------------- */
  var live = { sv: null, mode: null, band: null };
  function judge(c) {
    if (c.inst === false) return { cls: 's-nil', text: '---', tag: T('Not installed'), g: '' };
    if (c.comm === true || c.pv === 999) return { cls: 's-err', text: 'ERR', tag: '! ' + T('Comm error'), g: '!', k: 'err' };
    if (c.pv === null || c.pv === undefined || isNaN(Number(c.pv))) return { cls: 's-nil', text: '---', tag: T('No data'), g: '', k: 'nil' };
    var pv = Number(c.pv);
    if (pv < 18) return { cls: 's-neu', text: '...', tag: T('Too low'), g: '' };            // golden cTemperFrom.cpp:1171-1215
    var t = pv.toFixed(1);
    if (c.useSv && live.sv !== null && live.band !== null && (live.mode === 0 || live.mode === 3)) {
      if (pv > live.sv + live.band) return { cls: 's-over', text: t, tag: '▲ ' + T('Over temp'), g: '▲', k: 'over' };
      if (pv < live.sv - live.band) return { cls: 's-low', text: t, tag: '▼ ' + T('Low'), g: '▼', k: 'low' };
      return { cls: 's-ok', text: t, tag: T('OK'), g: '', k: 'ok' };
    }
    return { cls: 's-neu', text: t, tag: '', g: '' };
  }

  /* ---------------- DOM ---------------- */
  function el(tag, cls, text) {
    var e = doc.createElement(tag);
    if (cls) e.className = cls;
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function num(v) { if (v === undefined || v === null || v === '') return null; var n = Number(v); return isNaN(n) ? null : n; }

  var L = null, CH = [], tempCtrl = {}, expanded = false, GEN = null, GPIB = '';
  var VAL = {};                                              // tc → {pv, comm, inst}（tag 寫這裡；換語言重排時值不會掉）

  function buildCompact() {
    var box = doc.getElementById('tzGroups');
    box.textContent = '';
    L.groups.forEach(function (g) {
      var gr = el('div', 'tzGrp'), grid = el('div', 'tzGrid');
      gr.appendChild(el('div', 'tzLbl', g.label));
      g.chans.forEach(function (c) {
        var t = el('div', 'tzT s-nil'), nm = el('div', 'n'), v = el('div', 'v', '---'), sh = el('span', '', c.short), gl = el('span', '', '');
        nm.appendChild(sh); nm.appendChild(gl); t.appendChild(nm); t.appendChild(v);
        c.tile = { box: t, v: v, g: gl };
        grid.appendChild(t);
      });
      gr.appendChild(grid);
      box.appendChild(gr);
    });
    fitCompact();
  }
  /* 寬度不夠就一級一級縮（32 組＋DUT 4 顆時會用到），不捲動、不截掉格子 */
  function fitCompact() {
    var box = doc.getElementById('tzGroups'), root = doc.getElementById('tzRoot');
    root.classList.remove('tight', 'tighter');
    if (box.scrollWidth > box.clientWidth) root.classList.add('tight');
    if (box.scrollWidth > box.clientWidth) root.classList.add('tighter');
  }
  function buildExpanded() {
    var body = doc.getElementById('tzExpBody');
    body.textContent = '';
    L.groups.forEach(function (g) {
      var sec = el('div', 'tzSec' + (g.arm ? ' full' : ''));
      sec.appendChild(el('div', 'tzLbl', g.label + (g.arm ? '　' + T('Top row = socket row A, bottom row = row B, columns a →') + ' ' + 'abcdefgh'.charAt(g.cols - 1) : '')));
      if (g.arm) {
        var grid = el('div', 'tzArm' + (g.cols === 4 ? ' c4' : ''));
        var top = g.chans.filter(function (c) { return c.row === 0; }), bot = g.chans.filter(function (c) { return c.row === 1; });
        top.concat(bot).forEach(function (c) {
          var z = el('div', 'tzZ s-nil'), h = el('div', 'h'), pv = el('div', 'pv', '---'), sub = el('div', 'sub', ''), ad = el('div', 'addr', '');
          h.appendChild(el('b', '', c.short)); h.appendChild(el('span', '', c.tc.slice(2)));
          z.appendChild(h); z.appendChild(pv); z.appendChild(sub); z.appendChild(ad);
          c.zoneEl = { box: z, pv: pv, sub: sub, ad: ad };
          grid.appendChild(z);
        });
        sec.appendChild(grid);
      } else {
        var cards = el('div', 'tzCards');
        (g.expandOrder || g.chans).forEach(function (c) {
          var card = el('div', 'tzC'), h = el('div', 'h'), tag = el('span', 'tag s-nil', ''), pv = el('div', 'pv', '---'),
              sub = el('div', 'sub', ''), bar = el('div', 'tzBar'), mk = el('b'), ad = el('div', 'addr', '');
          h.appendChild(el('span', '', c.name)); h.appendChild(tag);
          bar.appendChild(el('i')); bar.appendChild(mk);
          card.appendChild(h); card.appendChild(pv); card.appendChild(sub); card.appendChild(bar); card.appendChild(ad);
          c.card = { box: card, tag: tag, pv: pv, sub: sub, bar: bar, mk: mk, ad: ad };
          cards.appendChild(card);
        });
        sec.appendChild(cards);
      }
      body.appendChild(sec);
    });
  }
  function setCls(node, base, cls) { node.className = base + ' ' + cls; }

  function paint() {
    if (!L) return;
    var cnt = { ok: 0, low: 0, over: 0, err: 0, nil: 0 };
    var seen = {};
    CH.forEach(function (c) {
      var v = VAL[c.tc] || {};
      c.pv = v.pv; c.comm = v.comm; c.inst = v.inst;
      var j = judge(c);
      if (!seen[c.tc]) { seen[c.tc] = 1; if (j.k) cnt[j.k] += 1; }
      var tip = c.name + ' · ' + c.address + (j.tag ? ' · ' + j.tag : '');
      if (c.tile) { setCls(c.tile.box, 'tzT', j.cls); c.tile.v.textContent = j.text; c.tile.g.textContent = j.g; c.tile.box.title = tip; }
      var hasV = j.text !== '---' && j.text !== 'ERR' && j.text !== '...';
      var d = (hasV && c.useSv && live.sv !== null) ? (Number(c.pv) - live.sv) : null;
      var subTxt = (c.useSv && live.sv !== null) ? ('SV ' + live.sv.toFixed(1) + (d === null ? '' : ' Δ ' + (d >= 0 ? '+' : '') + d.toFixed(1))) : '';
      if (c.card) {
        setCls(c.card.box, 'tzC', j.cls === 's-nil' ? '' : j.cls); setCls(c.card.tag, 'tag', j.cls);
        c.card.tag.textContent = j.tag; c.card.tag.style.display = j.tag ? '' : 'none'; c.card.pv.textContent = j.text;
        c.card.sub.textContent = subTxt; c.card.ad.textContent = c.address;
        var showBar = d !== null && live.band !== null && live.band > 0;
        c.card.bar.style.visibility = showBar ? 'visible' : 'hidden';
        if (showBar) {
          var pct = Math.max(0, Math.min(100, ((d + 2 * live.band) / (4 * live.band)) * 100));
          c.card.mk.style.left = 'calc(' + pct.toFixed(1) + '% - 1px)';
        }
        c.card.box.title = tip;
      }
      if (c.zoneEl) {
        setCls(c.zoneEl.box, 'tzZ', j.cls); c.zoneEl.pv.textContent = j.text;
        c.zoneEl.sub.textContent = subTxt.replace('SV ', ''); c.zoneEl.ad.textContent = c.address; c.zoneEl.box.title = tip;
      }
    });
    var sv = live.sv === null ? '---' : live.sv.toFixed(1);
    ['tzSvC', 'tzSvE'].forEach(function (id) { var e = doc.getElementById(id); if (e) e.textContent = sv; });
    var mode = live.mode === null ? 'Mode ---' : (MODE[live.mode] || ('Mode ' + live.mode)) + ' Mode';
    var bandTxt = T('Tolerance') + (live.band === null ? ' ---' : (' ±' + live.band));
    var mEl = doc.getElementById('tzMode'); if (mEl) { mEl.textContent = mode + ' · ' + L.sub; mEl.title = mEl.textContent; }
    var mEl2 = doc.getElementById('tzModeE'); if (mEl2) mEl2.textContent = L.machine + ' · ' + L.sub + ' · ' + mode + ' · ' + bandTxt;
    ['ok', 'low', 'over', 'err', 'nil'].forEach(function (k) {
      doc.querySelectorAll('[data-tzcnt="' + k + '"]').forEach(function (e) {
        e.textContent = (e.getAttribute('data-tzpre') || '') + T(e.getAttribute('data-tzlbl')) + ' ' + cnt[k];
      });
    });
  }

  /* 頁面上寫死的字（data-tz-i18n＝英文 key；data-tzpre／data-tzpost＝前後符號；data-tz-title＝tooltip 的 key） */
  function applyStatic() {
    doc.querySelectorAll('[data-tz-i18n]').forEach(function (e) {
      e.textContent = (e.getAttribute('data-tzpre') || '') + T(e.getAttribute('data-tz-i18n')) + (e.getAttribute('data-tzpost') || '');
    });
    doc.querySelectorAll('[data-tz-title]').forEach(function (e) { e.title = T(e.getAttribute('data-tz-title')); });
  }

  /* ---------------- 展開／收合 ---------------- */
  function postParent(msg) { try { if (global.parent && global.parent !== global) global.parent.postMessage(msg, '*'); } catch (e) { /* 單頁開啟 */ } }
  function setExpanded(on) {
    expanded = !!on;
    doc.getElementById('tzCompact').hidden = expanded;
    doc.getElementById('tzExp').hidden = !expanded;
    if (expanded) {
      paint();
      var h = doc.documentElement.scrollHeight;
      postParent({ resizeMe: { h: h + 26 } });             // +標題列
      var b = doc.getElementById('tzCollapse'); if (b) b.focus();
    } else {
      postParent({ resizeMe: null });
      var b2 = doc.getElementById('tzExpand'); if (b2) b2.focus();
      fitCompact();
    }
  }

  /* ---------------- 啟動 ---------------- */
  var failMsg = null;
  function fail(key, detail) {
    failMsg = { key: key, detail: detail || '' };
    var box = doc.getElementById('tzGroups');
    if (box) { box.textContent = ''; box.appendChild(el('div', 'tzNote', T(key) + failMsg.detail)); }
  }
  function rebuild() {
    if (!GEN) { if (failMsg) fail(failMsg.key, failMsg.detail); return; }
    L = (GPIB === '9050GPIB') ? layoutHT9050() : layoutHT9045(GEN.System);
    if (L.machine === 'HT9045' && num(tempCtrl.HEATER_CTRL_TYPE) === 3) L.sub += ' · No Heater';
    CH = [];
    L.groups.forEach(function (g) { g.chans.forEach(function (c) { c.address = addressOf(c, L, tempCtrl); CH.push(c); }); });
    buildCompact(); buildExpanded(); paint();
    if (expanded) setExpanded(true);
  }
  function setLang(lang) {
    LANG = lang || 'en';
    applyStatic();
    rebuild();
  }
  function start() {
    if (!doc.getElementById('tzRoot')) return;
    applyStatic();
    doc.getElementById('tzExpand').addEventListener('click', function () { setExpanded(true); });
    doc.getElementById('tzCollapse').addEventListener('click', function () { setExpanded(false); });
    doc.addEventListener('keydown', function (e) { if (expanded && e.key === 'Escape') setExpanded(false); });
    global.addEventListener('resize', function () { if (!expanded && L) fitCompact(); });
    global.addEventListener('message', function (ev) { if (ev.data && ev.data.type === 'HT_LANG') setLang(ev.data.lang); });
    if (!global.HT9045Live || !global.HT9045Live.load) { fail('Cannot read machine settings; temperature cells cannot be laid out', ' (HT9045Live)'); return; }
    global.HT9045Live.load().then(function (cache) {
      var gen = cache && cache.general && cache.general.sections ? cache.general.sections : null;
      if (!gen || !gen.System) { fail('Cannot read machine settings; temperature cells cannot be laid out', ' (/api/system/gerneral [System])'); return; }
      GEN = gen; GPIB = cache.gpibModel || ''; tempCtrl = gen.TempCtrl || {};
      live.band = num(global.HT9045Live.get('config.sections.Tempture.iL04TemptureRange'));
      rebuild();
      var T2 = global.HT9045Tags;
      if (!T2 || !T2.on) return;
      var seen = {};
      CH.forEach(function (c) {
        if (seen[c.tc]) return; seen[c.tc] = 1;
        var tc = c.tc; VAL[tc] = VAL[tc] || { pv: null, comm: null, inst: null };
        T2.on('temp.zone.' + tc + '.pv', function (v) { VAL[tc].pv = v; paint(); });
        T2.on('temp.zone.' + tc + '.comm', function (v) { VAL[tc].comm = v; paint(); });
        T2.on('temp.zone.' + tc + '.inst', function (v) { VAL[tc].inst = v; paint(); });
      });
      T2.on('temp.sv', function (v) { live.sv = num(v); paint(); });
      T2.on('temp.mode', function (v) { live.mode = num(v); paint(); });
      if (T2.connect) T2.connect().catch(function () { /* 連不上就維持 ---（不可知） */ });
    }).catch(function () { fail('Cannot read machine settings; temperature cells cannot be laid out', ' (/api/system/gerneral)'); });
  }
  global.HT9045TemperStrip = { state: function () { return { layout: L, channels: CH, live: live, expanded: expanded, lang: LANG }; }, judge: judge,
                               layoutHT9045: layoutHT9045, layoutHT9050: layoutHT9050, addressOf: addressOf, setLang: setLang, T: T };
  if (doc.readyState === 'loading') doc.addEventListener('DOMContentLoaded', start); else start();
})(window);
