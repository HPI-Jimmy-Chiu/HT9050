/* ht9045_hsys_heater_c.js -- HW.HandlerSys.html「Heater」分頁（grpHeater）的溫控器廠牌設定，C 路頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-P8) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * golden V912 HandlerSys.cpp（EN_HEATER_SHEET=1，MachineType.h:658）：
 *   建構子 :70-112 依 g_tHeaterInsInfo（MachineTypeUtility.cpp，71 通道、23 個 occupy）在 grpHeater 裡「動態」建 TLabel＋TComboBox，
 *   DFM 沒有這些元件 → 頁面產生器畫不出來，grpHeater 是空的。
 * 後端：FileRW/HSys.cpp（WS editlist.get／editlist.save tag=HSys）。值／可見／可改走引擎的通用 proxies（本檔只負責「元件要先在」
 *   ＋畫面上的連動）。
 *
 * //AI(W906-FRW-S166) 20260927 [W906] 偏離 golden（裁決過）：Q34 方案 D＋Q15（RULINGS_20260926 S166／S137；D-2＝A 依 RULINGS_20260927
 *   第 7 條第 35 題）。全文 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q34.」。
 *   Heater 分頁改成：
 *   ・最上面一列「溫控器廠牌：○ 全機相同 ○ 各溫控器不同」（替身 rgHeaterInsMode）。
 *   ・「Index 位置溫控器」／「其他位置溫控器」兩個下拉（cbHeaterInsIndexOpt／cbHeaterInsOtherOpt）；Index 位置＝Head1～4＋Index 32 區
 *     （D-2a，36 個，灰字列出）。
 *   ・逐通道表：名稱｜組別｜廠牌｜站號｜預設站號。「全機相同」時表唯讀，只顯示每個通道算出來的廠牌與預設站號；「各溫控器不同」時
 *     可改廠牌與站號（空白＝預設，灰字顯示 golden 算的預設值；TC401 只指定「第幾台」，通道仍是序號%4，D-7a）。
 *   ・D-9a 提示：「不同」模式、Index≠其他、或改過站號時顯示「目前溫控只看 HEATER_CTRL_TYPE（單一廠牌、預設站號），這些設定要等底層翻完才生效」。
 *   ・存檔前檢查在後端（D-7a 站號範圍、D-8a 同一個匯流排同廠牌同站號 → 整頁不存，訊息列出是哪兩個通道）。
 *   開頁不寫檔（Q15）；存檔寫的鍵見 FileRW/HSys_Heater.h 檔尾。
 *
 * //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden（Steven 裁決過；todo E-029／Q72）。Steven 1002 原話：「溫控器少了 DTM」「這邊選不到DTM」
 *   「還有EJ1N也選不到」；指著 golden V912 MachineType.h:638-655（＝906 MachineType.h:632 enum eTempControll，71 個名字相同，Jimmy RULINGS_20261002 #20）
 *   「這些全部都要可以選,所以數量也是少了」；「使用 EJ1N 或是 DTM 應該是要設定兩個變數」。
 *   golden＝906_0625（RULINGS_20261002 #20）：逐通道廠牌是 V912 才有的功能（906_0625 沒有 MachineTypeUtility.cpp／EN_HEATER_SHEET），照 Steven
 *   Q34／Q71～Q76 當 St01 的設計留著、等 Jimmy 定（NIGHT_REPORT s0 #63）；906 行號對照：移植樹 FileRW/HSys_Heater.h 檔尾 E029。
 *   ・廠牌 7 個：golden 5 個＋5 Omron EJ1N＋6 Delta DTM（同一個鍵 [TempCtrl] HeaterInsOpt_<通道>）。
 *     ⚠ 已接受的風險：同一台之後若跑 BCB V912，5／6 的通道 golden 認不得（廠牌分派沒有 else，裝了的通道溫控迴圈會停在那裡）。
 *   ・71 個通道全部列出、全部可選（以前只列 golden 23 個有下拉的＋走溫控 COM 埠的 Index 區）。這台沒裝的通道灰字標「這台沒裝」。
 *   ・Index 區（Aa1～Bd2，32 組再加 Ae1～Bh2）的 EJ1N／DTM 跟 Index Heater Counts（原本的 #rgHeater；表格版面在本區塊上面那一列，
 *     [System] USE_16_HEATER）即時連動，兩個一起存：
 *       - Index 區選 EJ1N／DTM → Index Heater Counts 點成同組數的 EJ1N／DTME08（16→2／5、32→3／6；4 Heaters→16）；改回溫控 COM 埠的廠牌 → 1／4。
 *       - Index Heater Counts 點 EJ1N／DTME08 → Index 區顯示 EJ1N／DTM；點回 16／32 Heaters → 原本的 EJ1N／DTM 改成 KT4H。
 *       - golden 的 USE_16_HEATER 是一個選項、整個 Index 區一起 ⇒ 任一 Index 區通道選 EJ1N／DTM＝整區；Head1～4 照 golden 留在溫控 COM 埠
 *         （「全機相同」時 Index 選 EJ1N／DTM，Head1～4 跟著「其他位置」，畫面有寫）。
 *       - 改 Index Heater Counts 一律「真的點」原本的 radio（r.click()，跟操作員點一樣，golden 事件與表格版面的 500 ms 鏡像都會跟上）；
 *         從不直接設 .checked。本檔只碰原本的 #rgHeater／#rgHeaterType 與自己的 cbHeaterInsOpt_*／edHeaterInsAddr_*／edHeaterInsCh_*，
 *         不碰任何 data-hst-src（表格版面 ht9045_hsys_table_c.js 的替身）。
 *   ・EJ1N／DTM 的站號＝兩格：台號（EJ1N＝SW1 1～15）／內部站號（DTM 0～3，0＝DTME08 主機）＋CH（EJ1N 1～4、DTM 1～8）。空白＝預設：Index 區照
 *     golden 接線（iTempCode），Index 區以外 golden 沒有對照 ⇒ 「各溫控器不同」裝了的通道要填。
 *   ・連線設定（唯讀顯示）：溫控 COM 埠 [TempCtrl] COM_PORT、EJ1N 的 [TempCtrl] COM_PORT_OMRON（兩個都在「Com Port」分頁改）、
 *     DTM 的 DTME08_Control.ini asAddress／asPort（本頁不寫這個檔：golden 讀 exe 資料夾、移植樹讀 system\，todo D-035）。
 *   ・Steven 1002 17:1x（細則，經 ST01-E2 問過、每題選建議）：「我的認知是: 當選擇全機相同, 那就是 index 跟其他部位的分成兩種溫控器進行選擇
 *     EJ1N 跟 DTM 在index站都是可以選的」「當選擇全機不同單獨設定, 那就是每個位置要可以單獨設定, 包含使用 EJ1N跟DTM」
 *     「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的, 也就是同時間只會顯示其中的一種」「socket跟 Dut 1~4也是互斥的」——
 *       - 全機相同：EJ1N／DTM 只在「Index 位置溫控器」（7 個）；「其他位置溫控器」只有 golden 5 個。各溫控器不同：每個通道 7 個。
 *       - Head1～4 與 Index 區照 Index Heater Counts：4 Heaters＝Head1～4；16 組＝Aa1～Bd2；32 組＝Aa1～Bh2。⚠ 跟 golden V912
 *         MachineTypeUtility.cpp:241-293 GetCtrlItemVisProp 相反（golden 16／32 組才顯示 Head1～4），Steven 知道仍這樣選 [W906]。
 *       - Socket 與 DUT1～4 照 Dut Heater Count（#rgUse4DUT，golden enum：0 1 EA、1 4 EA、2 2 EA）：1 EA＝Socket、2 EA＝DUT1～2、4 EA＝DUT1～4
 *         （Socket 那條＝golden :268 註解掉的條件打開）。
 *       - golden 只在開機算一次可見；這兩組改成跟著頁面上的 #rgHeater／#rgUse4DUT 即時顯示／隱藏（新行為）。其他通道照樣全列（Steven
 *         「這些全部都要可以選」），golden 可見條件只標「這台沒裝」。
 *   ・底層（bthermo／rs232／OmronEJ1N／fDTME08，Jimmy）翻完之前，機台溫控仍只看 HEATER_CTRL_TYPE：逐通道的 EJ1N／DTM 要等底層會讀 5／6 才生效（D-9a）。
 *   ・版面：只在 $('grpHeater') 的 .cli 裡面建，grpHeater 在哪就建在哪（表格版面會把它搬進 Temperature 分頁）；不動 grpHeater 自己。
 *   規則全文：FileRW/HSys_Heater.h 檔尾 E029；ctest：HSys_HeaterMix（C++）、E029_HeaterPages（node：本檔的連動規則、溫度條位址）。
 *
 * (2) golden rgHeaterTypeClick（:1519）的「畫面那一半」：操作員點 Heater Type（原本的 #rgHeaterType；表格版面在本區塊上面那一列）某個廠牌 →
 *     「全機相同，其他位置設成這個廠牌；Index 也是，除非 Index Heater Counts 是 EJ1N／DTME08（Index 區照舊，golden 按 Heater Type 不動
 *     USE_16_HEATER）」（方案 D ⑤＋E029）。檔案那一半：//AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927
 *     Q14＝B —— 按下不寫檔；後端存檔時重播這個事件只改記憶體（FileRW/HSys.cpp BeforeApply → W906_HeaterMixTypeClick），
 *     等整頁存檔（答「是」）才寫。點了但沒存、或存檔答「否」→ 不寫檔（golden 會寫）。
 *
 * ⚠ 底層：溫控流程（bthermo／rs232…）在 Jimmy 翻完之前仍只用單一廠牌 [TempCtrl] HEATER_CTRL_TYPE（RULINGS_20260926 第 26 條；
 *   Q34 ⑥ 1～3、5～8 歸 Jimmy），所以上面 D-9a 的提示要一直顯示到底層翻完（之後由 St01 拿掉，Q34 順序 ⑤）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'HSys';
  var MODE_SAME = 0, MODE_DIFF = 1;
  var N = 71;                                         // golden V912 eTempControll tcTotalCount
  var TC401 = 0, KT4H = 1, E5DC = 2, NOHEATER = 3, DTK4848 = 4, EJ1N = 5, DTM = 6;   // golden cmydef.cpp:222-227＋E029
  var AA1 = 11, BD2 = 26, AE1 = 33, BH2 = 48, HEAD1 = 4, HEAD4 = 7;                 // golden 906 MachineType.h:632 eTempControll（V912 :638 同）
  var TEMPCODE = [11, 15, 12, 16, 13, 17, 14, 18, 19, 23, 20, 24, 21, 25, 22, 26,     // golden cmydef.cpp:111-117 iTempCode（Aa1、Ba1、Ab1…）
                  33, 37, 34, 38, 35, 39, 36, 40, 41, 45, 42, 46, 43, 47, 44, 48];

  /* ================= 規則（純函式；ctest E029_HeaterPages 直接測，跟 FileRW/HSys.cpp 第 (4) 段同一套）================= */
  function isArea(b) { return b === EJ1N || b === DTM; }
  function isZone(ti) { return (ti >= AA1 && ti <= BD2) || (ti >= AE1 && ti <= BH2); }
  function isIndexGroup(ti) { return (ti >= HEAD1 && ti <= HEAD4) || isZone(ti); }   // D-2a：Head1～4＋Index 32 區
  function countOfU(u) {                                                             // golden MachineType.h:717-723
    if (u === 1 || u === 2 || u === 5) return 16;
    if (u === 3 || u === 4 || u === 6) return 32;
    return 0;
  }
  function areaCodeOfU(u) { return (u === 2 || u === 3) ? EJ1N : (u === 5 || u === 6) ? DTM : -1; }
  function inArea(ti, u) { return (ti >= AA1 && ti <= BD2) || (countOfU(u) === 32 && ti >= AE1 && ti <= BH2); }
  function u16For(code, u) {                                                         // Index 區選 code → USE_16_HEATER（組數不變；4 Heaters→16）
    var b32 = countOfU(u) === 32;
    if (code === EJ1N) return b32 ? 3 : 2;
    if (code === DTM) return b32 ? 6 : 5;
    if (u === 2 || u === 5) return 1;
    if (u === 3 || u === 6) return 4;
    return u;
  }
  function planSame(ti, I, O, u) {                                                   // 「全機相同」時通道的廠牌
    if (!isIndexGroup(ti)) return O;
    if (!isArea(I)) return I;
    return (isZone(ti) && inArea(ti, u)) ? I : O;                                    // Head1～4 與組數以外的 Index 區跟著其他位置
  }
  function defaultAddr(ti, b) {                                                      // 預設站號（golden 沒有「指定站號」）
    if (b === TC401) return { kind: 'tc401', st: Math.floor(ti / 4) + 1, ch: ti % 4 };
    if (b === KT4H || b === E5DC || b === DTK4848) return { kind: 'com', st: ti + 1, ch: null };
    if (isArea(b)) {
      var p = TEMPCODE.indexOf(ti);
      if (p < 0) return { kind: b === EJ1N ? 'ej1n' : 'dtm', st: null, ch: null };   // Index 區以外：沒有對照
      return b === EJ1N ? { kind: 'ej1n', st: Math.floor(p / 4) + 1, ch: p % 4 + 1 } : { kind: 'dtm', st: Math.floor(p / 8), ch: p % 8 + 1 };
    }
    return { kind: 'none', st: null, ch: null };
  }
  function areaH(st) {                                                               // Heater 分頁的 Index 區控制器（-1＝溫控 COM 埠）
    if (st.mode === MODE_SAME) return isArea(st.I) ? st.I : -1;
    var e = false, d = false;
    for (var ti = AA1; ti <= BH2; ti++) {
      if (!isZone(ti) || !inArea(ti, st.u)) continue;
      if (st.ch[ti] === EJ1N) e = true;
      if (st.ch[ti] === DTM) d = true;
    }
    return e ? EJ1N : d ? DTM : -1;
  }
  // 兩組互斥（Steven 1002 17:1x；後端 FileRW/HSys.cpp HmActive／HmPair 同一套）：dut＝golden eSocketTempControll（0 1 EA、1 4 EA、2 2 EA）
  function rowShown(ti, u, dut) {
    if (ti >= HEAD1 && ti <= HEAD4) return u === 0;
    if (ti >= AA1 && ti <= BD2) return countOfU(u) > 0;
    if (ti >= AE1 && ti <= BH2) return countOfU(u) === 32;
    if (ti === 8) return dut === 0;                                                 // tcSocket
    if (ti === 29 || ti === 30) return dut === 2 || dut === 1;                     // tcDUT1／2
    if (ti === 31 || ti === 32) return dut === 1;                                  // tcDUT3／4
    return true;
  }
  function inPair(ti) { return (ti >= HEAD1 && ti <= HEAD4) || isZone(ti) || ti === 8 || (ti >= 29 && ti <= 32); }
  function copy(st) { return { mode: st.mode, I: st.I, O: st.O, u: st.u, ch: st.ch.slice() }; }
  function setArea(st, u, code) { for (var ti = AA1; ti <= BH2; ti++) if (isZone(ti) && inArea(ti, u)) st.ch[ti] = code; }
  function alignIndex(st) {                                                          // 「全機相同」的 Index 下拉跟 USE_16_HEATER 對齊（後端 HmAlignIndexProxy）
    var code = areaCodeOfU(st.u);
    if (code > 0) st.I = code; else if (isArea(st.I)) st.I = KT4H;
  }
  // Heater 分頁改了（Index 下拉／Index 區某個通道）→ USE_16_HEATER 跟著；zone＝改的是哪個通道（「各溫控器不同」）
  function linkTab(st0, zone) {
    var st = copy(st0);
    if (st.mode === MODE_DIFF && zone !== undefined && isZone(zone) && inArea(zone, st.u)) {
      var b = st.ch[zone];
      if (isArea(b)) setArea(st, st.u, b);                                           // 一個選 EJ1N／DTM＝整區
      else if (areaCodeOfU(st.u) > 0)                                                 // 從 EJ1N／DTM 改回 COM 埠廠牌＝整區一起改回
        for (var ti = AA1; ti <= BH2; ti++) if (isZone(ti) && inArea(ti, st.u) && isArea(st.ch[ti])) st.ch[ti] = b;
    }
    var H = areaH(st);
    if (H !== areaCodeOfU(st.u)) {
      st.u = u16For(H, st.u);
      if (st.mode === MODE_DIFF && H > 0) setArea(st, st.u, H);
    }
    alignIndex(st);
    return st;
  }
  // Index Heater Counts 改了（uOld → st.u）→ Heater 分頁跟著
  function linkU16(st0, uOld) {
    var st = copy(st0), code = areaCodeOfU(st.u);
    if (st.mode === MODE_DIFF) {
      if (code > 0) setArea(st, st.u, code);
      else for (var ti = AA1; ti <= BH2; ti++)
        if (isZone(ti) && (inArea(ti, st.u) || inArea(ti, uOld)) && isArea(st.ch[ti])) st.ch[ti] = KT4H;
    }
    alignIndex(st);
    return st;
  }
  // Heater Type（golden rgHeaterTypeClick 的畫面那一半）：全機相同、其他＝點的廠牌、Index＝點的廠牌（Index Heater Counts 是 EJ1N／DTME08 時照舊）
  //   Heater Type 是 golden 的 5 個（0～4），所以「其他位置」一定是 golden 的廠牌
  function heaterType(st0, idx) {
    var st = copy(st0), code = areaCodeOfU(st.u);
    st.mode = MODE_SAME; st.O = idx; st.I = code > 0 ? code : idx;
    return st;
  }
  var RULES = { isArea: isArea, isZone: isZone, isIndexGroup: isIndexGroup, countOfU: countOfU, areaCodeOfU: areaCodeOfU, inArea: inArea,
                u16For: u16For, planSame: planSame, defaultAddr: defaultAddr, areaH: areaH, linkTab: linkTab, linkU16: linkU16,
                heaterType: heaterType, rowShown: rowShown, inPair: inPair, N: N, TEMPCODE: TEMPCODE };

  /* ================= 畫面 ================= */
  function $(id) { return document.getElementById(id); }
  function host() {                                   // grpHeater 的內容區（頁面產生器畫的 .cli）；grpHeater 在哪就建在哪
    var g = $('grpHeater');
    if (!g) return null;
    return g.querySelector('.cli') || g;
  }
  function el(tag, attrs, text) {
    var e = document.createElement(tag);
    Object.keys(attrs || {}).forEach(function (k) {
      if (k === 'style') e.style.cssText = attrs[k]; else e.setAttribute(k, attrs[k]);
    });
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function radios(g) { return g ? Array.prototype.slice.call(g.querySelectorAll('input[type="radio"]')) : []; }
  function checkedIndex(g) { var r = radios(g); for (var i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; }
  function radioText(g, i) { var r = radios(g)[i], lb = r && r.closest ? r.closest('label') : null; return lb ? lb.textContent.trim() : String(i); }

  var MIX = null;                                     // extra.heater.mix（後端 FileRW/HSys.cpp W906_HeaterMixExtraJson）
  var OPTS = [];                                      // 7 個廠牌（golden g_HeaterInsOptStr＋EJ1N＋DTM）：Index 下拉、逐通道下拉
  var OPTS_OTHER = [];                                // 「其他位置溫控器」：golden 5 個（Steven 1002 17:1x）
  var LAST_MODE = MODE_SAME;                          // 上一次畫面上的模式（切換時暫存／還原逐通道值）
  var LAST_U = -1;                                    // 上一次畫面上的 Index Heater Counts（#rgHeater）
  var SELF = false;                                   // 本檔自己點 #rgHeater 時，不要再跑一次反方向的連動
  var GREY = 'color:#777;';

  function selectWithOptions(id, list) {
    var s = el('select', { 'class': 'ed', id: id, 'data-src': 'hsys-heater' });
    (list || OPTS).forEach(function (t) { s.appendChild(el('option', {}, t)); });
    return s;
  }
  function rowsByTi() { var m = {}; (MIX && MIX.rows || []).forEach(function (r) { m[r.typeIdx] = r; }); return m; }

  /* ---- (1) 引擎套值之前建元件 ---------------------------------------------------------------- */
  function build(d) {
    var hx = d && d.extra && d.extra.heater;
    var box = host();
    if (!hx || !box || !hx.mix) return;
    MIX = hx.mix;
    OPTS = MIX.options || hx.options || [];
    OPTS_OTHER = MIX.otherOptions || OPTS.slice(0, 5);
    LAST_MODE = MIX.mode === MODE_DIFF ? MODE_DIFF : MODE_SAME;
    LAST_U = -1;

    var old = $('hsysHeaterMix');
    if (old && old.parentNode) old.parentNode.removeChild(old);
    // 舊版（20260926 起）照 golden 版面直接放在 .cli 裡的下拉／標籤一併清掉（id 會在下面重建）
    Array.prototype.slice.call(box.querySelectorAll('[data-src="hsys-heater"]')).forEach(function (x) {
      if (x.parentNode) x.parentNode.removeChild(x);
    });

    // 在原本的 Heater 分頁（.cli 是 position:absolute 的整頁）自己捲動；表格版面搬進 Temperature 分頁後由它的 CSS 改成自然高度
    var root = el('div', { id: 'hsysHeaterMix', 'data-src': 'hsys-heater',
      style: 'position:absolute;left:8px;top:6px;right:8px;bottom:6px;overflow:auto;font-size:12px;color:#000;' });

    // 第一列：模式（替身 rgHeaterInsMode，TRadioGroup：0 全機相同／1 各溫控器不同）
    var r1 = el('div', { style: 'margin:2px 0 8px 0;' });
    r1.appendChild(el('b', {}, '溫控器廠牌：'));
    var rg = el('span', { id: MIX.modeProxy, 'data-src': 'hsys-heater',
      title: MIX.modeProxy + '：[TempCtrl] HeaterInsMode（0 全機相同／1 各溫控器不同）' });
    ['全機相同', '各溫控器不同'].forEach(function (t) {
      var lb = el('label', { style: 'margin-right:18px;cursor:pointer;' });
      lb.appendChild(el('input', { type: 'radio', name: 'rg_' + MIX.modeProxy }));
      lb.appendChild(document.createTextNode(' ' + t));
      rg.appendChild(lb);
    });
    r1.appendChild(rg);
    root.appendChild(r1);

    // 第二列：Index 位置／其他位置（替身 cbHeaterInsIndexOpt／cbHeaterInsOtherOpt）
    var r2 = el('div', { style: 'margin:0 0 4px 0;' });
    r2.appendChild(el('span', { style: 'display:inline-block;width:110px;' }, 'Index 位置溫控器'));
    var sI = selectWithOptions(MIX.indexProxy);
    sI.title = MIX.indexProxy + '：[TempCtrl] HeaterInsIndexOpt（全機相同時 Index 位置的廠牌；選 EJ1N／DTM 會連動 Index Heater Counts）';
    r2.appendChild(sI);
    r2.appendChild(el('span', { id: 'hsysHeaterHeadNote', style: GREY + 'margin-left:8px;' }, ''));
    root.appendChild(r2);
    root.appendChild(el('div', { style: GREY + 'margin:0 0 6px 110px;' },
      'Index 位置（' + (MIX.indexGroup || []).length + ' 個通道）：' + (MIX.indexGroup || []).join('、')));
    var r3 = el('div', { style: 'margin:0 0 6px 0;' });
    r3.appendChild(el('span', { style: 'display:inline-block;width:110px;' }, '其他位置溫控器'));
    var sO = selectWithOptions(MIX.otherProxy, OPTS_OTHER);
    sO.title = MIX.otherProxy + '：[TempCtrl] HeaterInsOtherOpt（全機相同時其他位置的廠牌；golden 的 5 個，EJ1N／DTM 只在 Index 位置）';
    r3.appendChild(sO);
    r3.appendChild(el('span', { id: 'hsysHeaterDiffNote', style: GREY + 'margin-left:8px;' }, ''));
    root.appendChild(r3);

    // Index Heater Counts（原本的 #rgHeater；表格版面＝本區塊上面那一列）——這裡只讀、說明連動
    root.appendChild(el('div', { id: 'hsysHeaterU16', style: 'margin:0 0 4px 0;' }, ''));
    // 連線設定（唯讀）
    root.appendChild(el('div', { id: 'hsysHeaterConn', style: GREY + 'margin:0 0 6px 0;' }, ''));

    // D-9a 提示
    root.appendChild(el('div', { id: 'hsysHeaterHint',
      style: 'display:none;margin:4px 0 8px 0;padding:4px 8px;border:1px solid #d08a00;background:#fff4dd;color:#8a4b00;' },
      '⚠ ' + (MIX.hint || '')));

    // 逐通道表：71 個全部（E029），分欄放
    var cols = el('div', { style: 'display:flex;flex-wrap:wrap;gap:18px;align-items:flex-start;' });
    var PER = 18, table = null;
    function newTable() {
      var t = el('table', { style: 'border-collapse:collapse;' });
      var hr = el('tr', {});
      ['通道', '組別', '廠牌', '站號／台號', 'CH', '預設', ''].forEach(function (h) {
        hr.appendChild(el('th', { style: 'text-align:left;padding:1px 6px;border-bottom:1px solid #999;font-weight:normal;' + GREY }, h));
      });
      t.appendChild(hr);
      cols.appendChild(t);
      return t;
    }
    function rowOf(r) {
      var tr = el('tr', { 'data-ti': r.typeIdx });
      var nm = el('td', { style: 'padding:1px 6px;white-space:nowrap;' });
      nm.appendChild(r.lb ? el('span', { 'class': 'lb', id: r.lb }, r.name) : el('span', {}, r.name));
      nm.title = r.saveName + (r.golden ? '（golden 有下拉）' : (r.zone ? '（golden 沒有下拉：Index 區）' : '（golden 沒有下拉）'));
      tr.appendChild(nm);
      tr.appendChild(el('td', { style: 'padding:1px 6px;' + GREY }, r.indexGroup ? 'Index' : '其他'));
      var tdc = el('td', { style: 'padding:1px 6px;' });
      var cb = selectWithOptions(r.cb);
      cb.setAttribute('data-ti', r.typeIdx);
      cb.title = r.cb + '：[TempCtrl] ' + r.saveName;
      tdc.appendChild(cb);
      tr.appendChild(tdc);
      var tda = el('td', { style: 'padding:1px 6px;' });
      tda.appendChild(el('input', { 'class': 'ed', id: r.addr, type: 'text', size: '4', 'data-src': 'hsys-heater',
        title: r.addr + '：[TempCtrl] ' + r.addrKey + '（空白＝預設；COM 埠 1～' + MIX.stationMax + '、E5DC 1～' + MIX.stationMaxE5DC +
               '；EJ1N 台號＝SW1 ' + MIX.ej1nUnitMin + '～' + MIX.ej1nUnitMax + '；DTM 內部站號 ' + MIX.dtmStationMin + '～' + MIX.dtmStationMax + '）' }));
      tr.appendChild(tda);
      var tdh = el('td', { style: 'padding:1px 6px;' });
      tdh.appendChild(el('input', { 'class': 'ed', id: r.chEd, type: 'text', size: '2', 'data-src': 'hsys-heater',
        title: r.chEd + '：[TempCtrl] ' + r.chKey + '（只有 Omron EJ1N 1～' + MIX.ej1nChMax + '、Delta DTM 1～' + MIX.dtmChMax + '）' }));
      tr.appendChild(tdh);
      tr.appendChild(el('td', { 'class': 'hsys-def', style: 'padding:1px 6px;white-space:nowrap;' + GREY }, ''));
      tr.appendChild(el('td', { 'class': 'hsys-inst', style: 'padding:1px 6px;white-space:nowrap;' + GREY }, ''));
      return tr;
    }
    (MIX.rows || []).forEach(function (r, i) {
      if (i % PER === 0) table = newTable();
      table.appendChild(rowOf(r));
    });
    if (!(MIX.rows || []).length) cols.appendChild(el('div', { style: GREY }, '（沒有通道）'));
    root.appendChild(cols);
    box.appendChild(root);

    root.addEventListener('change', onTabChange);
    root.addEventListener('input', function (ev) { if (ev.target && ev.target.tagName === 'INPUT' && ev.target.type === 'text') refresh(); });
    setTimeout(function () { LAST_U = checkedIndex($('rgHeater')); refresh(); }, 0);   // 引擎在自己的 then 裡套值（在這之後）→ 套完再整理畫面
  }

  function modeNow() {
    var rs = MIX ? document.querySelectorAll('#' + MIX.modeProxy + ' input[type="radio"]') : [];
    return (rs.length > 1 && rs[1].checked) ? MODE_DIFF : MODE_SAME;
  }
  function setModeRadio(m) {
    var rs = MIX ? document.querySelectorAll('#' + MIX.modeProxy + ' input[type="radio"]') : [];
    if (rs.length > 1) { rs[0].checked = m === MODE_SAME; rs[1].checked = m === MODE_DIFF; }
  }
  function setRO(x, on) {                             // 自己的唯讀（不碰引擎依權限關掉的 data-gb-dis）
    if (!x) return;
    if (on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-hs-ro', '1'); } }
    else if (x.getAttribute('data-hs-ro') === '1') { x.disabled = false; x.removeAttribute('data-hs-ro'); }
  }
  function u16Now() { var u = checkedIndex($('rgHeater')); return u >= 0 ? u : (MIX ? MIX.use16Heater : 0); }
  function dutNow() { var d = checkedIndex($(MIX && MIX.dutProxy || 'rgUse4DUT')); return d >= 0 ? d : (MIX ? MIX.use4Dut : 0); }

  // 畫面 ⇄ 規則的狀態（「全機相同」時通道值不讀，由 planSame 算）
  function readState() {
    var st = { mode: modeNow(), I: $(MIX.indexProxy).selectedIndex, O: $(MIX.otherProxy).selectedIndex, u: u16Now(), ch: [] };
    for (var ti = 0; ti < N; ti++) st.ch.push(-1);
    (MIX.rows || []).forEach(function (r) { var cb = $(r.cb); if (cb) st.ch[r.typeIdx] = cb.selectedIndex; });
    return st;
  }
  function writeState(st) {
    var sI = $(MIX.indexProxy), sO = $(MIX.otherProxy);
    if (sI && st.I >= 0 && st.I < sI.options.length) sI.selectedIndex = st.I;
    if (sO && st.O >= 0 && st.O < sO.options.length) sO.selectedIndex = st.O;
    if (st.mode === MODE_DIFF) (MIX.rows || []).forEach(function (r) { var cb = $(r.cb); if (cb && st.ch[r.typeIdx] >= 0) cb.selectedIndex = st.ch[r.typeIdx]; });
    if (st.u !== u16Now()) clickU16(st.u);
  }
  // Index Heater Counts：真的點原本的 radio（跟操作員點一樣；golden 事件、表格版面的鏡像都跟上）。點不到（停用）就不動。
  function clickU16(u) {
    var r = radios($('rgHeater'))[u];
    if (!r || r.checked || r.disabled) return;
    SELF = true;
    try { r.click(); } finally { SELF = false; }
    LAST_U = u16Now();
  }

  function onTabChange(ev) {
    var t = ev.target;
    if (!MIX || !t) return;
    if (t.type === 'text') { refresh(); return; }
    var st = readState();
    if (t.type === 'radio') {                                         // 切模式：Index Heater Counts 為準（模式不是 Index 區的廠牌選擇）
      refresh();                                                      // 先照舊規則暫存／還原逐通道值
      st = readState();
      writeState(linkU16(st, st.u));
    } else if (t.id === MIX.indexProxy) {
      writeState(linkTab(st));
    } else if (t.getAttribute && t.getAttribute('data-ti') !== null) {
      writeState(linkTab(st, parseInt(t.getAttribute('data-ti'), 10)));
    } else if (t.id === MIX.otherProxy) {
      writeState(linkTab(st));
    }
    refresh();
  }
  function onU16Change() {                                            // 原本的 #rgHeater 被點了（操作員、表格版面、本檔）
    if (!MIX || !$('hsysHeaterMix')) return;
    var u = u16Now();
    if (!SELF && LAST_U >= 0 && u !== LAST_U) writeState(linkU16(readState(), LAST_U));
    LAST_U = u;
    refresh();
  }

  /* ---- 畫面整理（模式、Index／其他、逐通道表、D-9a 提示、連線）-------------------------------------- */
  function selText(id) { var s = $(id); return (s && s.selectedIndex >= 0 && s.options[s.selectedIndex]) ? s.options[s.selectedIndex].text : '—'; }
  function refresh() {
    if (!MIX || !$('hsysHeaterMix')) return;
    var mode = modeNow();
    var sI = $(MIX.indexProxy), sO = $(MIX.otherProxy);
    var I = sI ? sI.selectedIndex : -1, O = sO ? sO.selectedIndex : -1, u = u16Now(), dut = dutNow();
    var anyStation = false;
    (MIX.rows || []).forEach(function (r) {
      var ti = r.typeIdx, cb = $(r.cb), ad = $(r.addr), chx = $(r.chEd);
      if (!cb || !ad || !chx) return;
      if (mode === MODE_SAME) {
        if (LAST_MODE === MODE_DIFF) cb.setAttribute('data-hs-diff', String(cb.selectedIndex));   // 切回「相同」前先記下逐通道值
        cb.selectedIndex = planSame(ti, I, O, u);                                                // 算出來的廠牌（唯讀）
        setRO(cb, true); setRO(ad, true); setRO(chx, true);
        ad.style.display = 'none'; chx.style.display = 'none';
      } else {
        if (LAST_MODE === MODE_SAME && cb.getAttribute('data-hs-diff') !== null) {
          cb.selectedIndex = parseInt(cb.getAttribute('data-hs-diff'), 10);
        }
        cb.removeAttribute('data-hs-diff');
        setRO(cb, false); setRO(ad, false);
        ad.style.display = '';
      }
      var b = cb.selectedIndex, def = defaultAddr(ti, b);
      var areaBrand = isArea(b);
      if (mode === MODE_DIFF) { setRO(chx, !areaBrand); chx.style.display = areaBrand ? '' : 'none'; }
      ad.placeholder = def.st === null ? '' : String(def.st);
      chx.placeholder = def.ch === null ? '' : String(def.ch);
      var tr = cb.closest ? cb.closest('tr') : null;
      var dc = tr ? tr.querySelector('.hsys-def') : null, ic = tr ? tr.querySelector('.hsys-inst') : null;
      var txt = '';
      if (def.kind === 'tc401') txt = '第 ' + def.st + ' 台・通道 ' + def.ch;
      else if (def.kind === 'com') txt = '預設 ' + def.st;
      else if (def.kind === 'ej1n') txt = def.st === null ? '未指定（要填台號、CH）' : '第 ' + def.st + ' 台 CH' + def.ch;
      else if (def.kind === 'dtm') txt = def.st === null ? '未指定（要填站號、CH）' : '站 ' + def.st + ' CH' + def.ch;
      if (dc) dc.textContent = txt;
      var shown = rowShown(ti, u, dut);                                       // 兩組互斥：跟著 #rgHeater／#rgUse4DUT（Steven 1002 17:1x）
      var inst = inPair(ti) ? shown : !!r.goldenShow;                          // 其他通道：golden 可見條件只標「這台沒裝」，照樣列出可選
      if (ic) ic.textContent = inst ? '' : '這台沒裝';
      if (tr) { tr.style.opacity = inst ? '' : '0.75'; tr.style.display = shown ? '' : 'none'; }
      var sa = String(ad.value).trim(), sc = String(chx.value).trim();
      if (mode === MODE_DIFF && (sa !== '' || (areaBrand && sc !== '')) &&
          (sa !== String(def.st === null ? '' : def.st) || (areaBrand && sc !== String(def.ch === null ? '' : def.ch)))) anyStation = true;
    });
    var note = $('hsysHeaterDiffNote');
    if (note) note.textContent = mode === MODE_DIFF ? '（各溫控器不同：這兩個下拉照樣存檔，切回全機相同時用）' : '';
    var hn = $('hsysHeaterHeadNote');
    if (hn) hn.textContent = (mode === MODE_SAME && isArea(I)) ?
      'Index 區是 ' + (OPTS[I] || '') + '（Index Heater Counts 16／32 組：Head1～4 不顯示，跟 Ax／Bx 互斥；Head1～4 的檔案值照 golden 留在溫控 COM 埠＝其他位置的廠牌）' : '';
    var un = $('hsysHeaterU16');
    if (un) {
      var code = areaCodeOfU(u);
      un.textContent = 'Index Heater Counts（[System] USE_16_HEATER）：' + (u >= 0 ? radioText($('rgHeater'), u) : '—') +
        (code > 0 ? '　→ Index 區（Aa1～Bd2' + (countOfU(u) === 32 ? '、Ae1～Bh2' : '') + '）是 ' + (OPTS[code] || '') : '') +
        '。Index 區選 Omron EJ1N／Delta DTM 會跟著改成同組數的 EJ1N／DTME08，改回溫控 COM 埠的廠牌會回到 16／32 Heaters；兩個一起存。';
    }
    var cn = $('hsysHeaterConn'), c = MIX.conn || {};
    if (cn) {
      var d = c.dtm || {};
      cn.textContent = '連線（唯讀）：溫控 COM 埠 ' + (c.comTempKey || '') + '＝' + selText(c.comTempWidget) +
        '｜Omron EJ1N ' + (c.comOmronKey || '') + '＝' + selText(c.comOmronWidget) + '（兩個在「Com Port」分頁改）' +
        '｜Delta DTM：' + (d.exists ? (d.address || '?') + ':' + (d.port || '?') : '檔案不存在＝程式預設 ' + (d.defAddress || '') + ':' + (d.defPort || '')) +
        '（' + (d.file || 'DTME08_Control.ini') + '；golden 讀 exe 資料夾，本頁不寫這個檔）';
    }
    var hint = $('hsysHeaterHint');
    if (hint) hint.style.display = (mode === MODE_DIFF || (mode === MODE_SAME && I !== O && !isArea(I)) || anyStation) ? '' : 'none';
    LAST_MODE = mode;
  }

  var R = window.HT9045Recipe;
  if (R && R.editlistGet && !R.__hsysHeaterWrapped) {
    R.__hsysHeaterWrapped = true;
    var get0 = R.editlistGet;
    R.editlistGet = function (st) {
      if (st !== STRUCT) return get0.apply(this, arguments);
      return get0.apply(this, arguments).then(function (d) {
        try { build(d); } catch (e) { console.warn('[HSys/heater] build failed: ' + e.message); }
        return d;                                     // 引擎接著在它自己的 then 裡套值（元件已經在了）
      });
    };
  }

  /* ---- (2) golden rgHeaterTypeClick 的畫面那一半（方案 D ⑤＋E029）＋ Index Heater Counts 的反方向連動 ----------------- */
  function bindHeaterType() {
    var rg = $('rgHeaterType');
    if (rg && !rg.__hsysHeater) {
      rg.__hsysHeater = true;
      rg.title = (rg.title ? rg.title + '\n' : '') +
                 'golden rgHeaterTypeClick（HandlerSys.cpp:1519）：點選廠牌 → Heater 分頁設成「全機相同」、其他位置是這個廠牌，Index 也是' +
                 '（Index Heater Counts 是 EJ1N／DTME08 時 Index 區照舊）；按下不寫檔（Steven 20260927 Q14＝B，偏離 golden），按「存檔」才寫 [TempCtrl]。';
      rg.addEventListener('change', function (ev) {
        var t = ev.target;
        if (!t || t.type !== 'radio' || !t.checked || !MIX) return;
        var rs = rg.querySelectorAll('input[type="radio"]'), idx = -1;
        for (var i = 0; i < rs.length; i++) if (rs[i] === t) idx = i;
        if (idx < 0) return;
        var st = heaterType(readState(), idx);
        setModeRadio(MODE_SAME);
        writeState(st);
        (MIX.rows || []).forEach(function (r) { var cb = $(r.cb); if (cb) cb.removeAttribute('data-hs-diff'); });
        LAST_MODE = MODE_SAME;                        // 點了 Heater Type ＝ 全部同一廠牌：之後切到「各溫控器不同」從這個廠牌開始（不還原先前的逐通道值）
        refresh();
      });
    }
    var rh = $('rgHeater');                           // 原本的 Index Heater Counts（不是表格版面的替身）
    if (rh && !rh.__hsysHeaterU16) { rh.__hsysHeaterU16 = true; rh.addEventListener('change', onU16Change); }
    var rd = $('rgUse4DUT');                          // 原本的 Dut Heater Count：Socket／DUT1～4 互斥跟著它（只影響顯示）
    if (rd && !rd.__hsysHeaterDut) { rd.__hsysHeaterDut = true; rd.addEventListener('change', function () { refresh(); }); }
    ['cbComTemp', 'cbComTempOmron'].forEach(function (id) {   // 連線那一行跟著「Com Port」分頁
      var s = $(id);
      if (s && !s.__hsysHeaterConn) { s.__hsysHeaterConn = true; s.addEventListener('change', refresh); }
    });
  }
  window.HT9045HSysHeater = { rules: RULES, refresh: refresh };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bindHeaterType);
  else bindHeaterType();
})();
