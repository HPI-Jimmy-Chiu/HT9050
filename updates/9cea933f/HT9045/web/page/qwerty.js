/* HT9xxx 小鍵盤（TfQwertyKey / myQwertyKeyBoard 的 HTML 模擬）
   用法：HTQwerty.show(targetEl, flags, {dp, checkRange, min, max})
   flags 為 N_ 旗標（cmydef.cpp）位元 OR：
     HTQwerty.N.INTEGER / DOUBLE / NO_SYMBOL / PASSWORD / NO_SPACE / UPPERCASE / NO_NUM_PAD / PORT / IP_ADDR
   依 ShowQwertyKey() 設計：數字類→只顯示數字鍵盤(窄)；其餘→全 QWERTY(可含數字鍵盤)；
   密碼→遮罩、隱藏 Current Value；checkRange→顯示 Maximum/Minimum 並於 OK 時夾限。
   AI(W906-KB-GOLDEN) 20261004: 按鍵行為改成照 golden myQwertyKeyBoard.cpp（906 樹；V912 的 .cpp/.dfm/.h 與它位元組相同）：
     * 開窗時整段內容反白（FormShow :137-143 SetFocus；edQwertyContent 沒寫 AutoSelect（dfm:84-100）＝VCL 預設 True，
       stdctrls.pas:1967-1972 CMEnter → SelectAll），所以第一個字元鍵（數字／字母／符號／空白）先清空再打
       （spbKeyClick :320-339）——以前是接在舊值後面（-72700 打 72800 變成 -7270072800）。
     * '-' 是整段正負號切換（spbMinusClick :421-431），不是在尾巴加一個 '-'。
     * '%' 只輪換 +/- 步進鍵的刻度（spbPercentClick :368-377 → ChangeDecimalPoint :379-418），數值不動；以前是除以 100。
     * 步進鍵的字跟著 dp（每次開窗 iDecimalPoint=iDP :185 → ChangeDecimalPoint :187）：dp0 = ±10/±100/±1000……；
       按下＝atof(畫面)+atof(鍵上的字)，整數 int() 截斷、否則 "%1.6f"（spbAdd1Click :433-449）。
       dp 只決定步進鍵的字，OK 不會照 dp 四捨五入（:285-292 只有 CheckRange）。switch 沒有的 dp 保留這個鍵盤上一次的字——
       golden 自己就這樣呼叫（cContact.cpp:18557 N_DOUBLE dp 4、cConfiguration.cpp:5985 N_DOUBLE dp 6，906 樹）。
       ht9045_config_trayplate.js 的數字格照伺服器送的 kp.dp（golden 2：V912 cConfiguration.cpp:6977／:7113；FileRW/CfgTrayPlate.cpp:907），
       KB-GOLDEN 1/2（d1e77b4f）起已改（以前送 15 是配合舊鍵盤的 toFixed）。
     * OK 與 Abort 走同一段 ShowModal 之後的尾段（:285-301）：只有 N_INTEGER/N_DOUBLE 且 checkRange 才夾限；
       Abort 先放回原值（spbCancelClick :362-366）再夾——放回後的字與原本不同才寫回欄位並呼叫 onCommit。
     * 沒有標題列 ✕（dfm:4 BorderIcons=[]），點外面不關（:283 ShowModal）；離開只有 Abort／OK（Enter）。
     * 開著又來一個 show → 疊第二個（:171-178 fQwertyKey2），兩個都開著再來 → 不理並回 onAbort（:180-181）。
     * 實體鍵：照使用者 20260915 規則 B（docs/web-client/REPLICATE.md §2.5「不可違反」），本檔不處理——
       ht9045_wire_engine.js physicalKeys() 把實體鍵轉成「字相同的那顆鍵」的 click，所以實體鍵＝按畫面上那顆鍵：
       數字取代反白、'-' 切換正負、'%' 輪換刻度、Escape→Abort（上面的 Abort 尾段）；小鍵盤上沒有的鍵不收。
       AI(W906-KB-GOLDEN) 20261004 (2/2): 數字鍵盤的 BS／Del／OK 也對上了——Backspace→BS、Delete→Del、Enter→OK（QWERTY 照舊 ⌫／Delete／Enter；
       golden KeyDown :463-466 的 Enter 也是關窗＝OK）；小鍵盤開著時對不到按鈕的鍵一律吃掉（golden ShowModal：鍵碰不到後面的畫面）。
       對照表在 physicalKeys() 的 PHYS_MAP，本檔仍然沒有實體鍵的程式。
     * AI(W906-KB-GOLDEN) 20261004 (2/2): 頁面自己把最上層的 .qkOv 拿掉時（不是按 OK／Abort），按鍵／OK／Abort 先 settle()：
       被拿掉的那一個算關了（呼叫一次它的 onAbort，不寫欄位），下面那一個（fQwertyKey）馬上接手，不必等下一次 show()。 */
(function () {
  var N = { INTEGER:0x0001, DOUBLE:0x0002, NO_SYMBOL:0x0004, PASSWORD:0x0008,
            NO_SPACE:0x0010, UPPERCASE:0x0020, NO_NUM_PAD:0x0040, PORT:0x0080, IP_ADDR:0x0100 };

  var QL = [
    ['1','2','3','4','5','6','7','8','9','0','-','='],
    ['q','w','e','r','t','y','u','i','o','p','[',']','\\'],
    ['a','s','d','f','g','h','j','k','l',';','\'','~'],
    ['z','x','c','v','b','n','m',',','.','/']
  ];
  var QU = [
    ['!','@','#','$','%','^','&','*','(',')','_','+'],
    ['Q','W','E','R','T','Y','U','I','O','P','{','}','|'],
    ['A','S','D','F','G','H','J','K','L',':','"','~'],
    ['Z','X','C','V','B','N','M','<','>','?']
  ];
  function isSym(ch) { return !/[a-zA-Z0-9]/.test(ch); }

  // AI(W906-KB-GOLDEN) 20261004: CSS -- .qkDispW / .qkHl draw golden's selected text (FormShow AutoSelect, myQwertyKeyBoard.cpp:137-143)
  //   over the display without moving focus; .w2 = the two-column spbPercent / spbSummit2 (dfm:325-329 / :283-296, Width 104);
  //   .qkHid = a key golden sets Visible=false (:190-202) keeps its cell empty; the .qkx (✕) rule is gone with the ✕ (dfm:4 BorderIcons=[]).
  var CSS = '\
  .qkOv{position:fixed;inset:0;background:rgba(0,0,0,.35);z-index:99999;display:flex;align-items:center;justify-content:center;font-family:"Microsoft JhengHei",sans-serif;}\
  .qkWin{background:#c8c8bc;border:1px solid #6b6b60;border-radius:4px;box-shadow:4px 4px 16px rgba(0,0,0,.5);max-width:96vw;max-height:96vh;overflow:auto;}\
  .qkBar{background:linear-gradient(90deg,#0a246a,#3a6ea5);color:#fff;font-size:12px;font-weight:bold;padding:4px 8px;display:flex;align-items:center;gap:6px;}\
  .qkBody{padding:8px;}\
  .qkDispW{position:relative;margin-bottom:6px;}\
  .qkDisp{width:100%;height:34px;font-size:18px;font-family:Arial,sans-serif;box-sizing:border-box;border:2px inset #ddd;padding:2px 6px;margin:0;background:#fff;}\
  .qkDisp.qkSel{color:transparent;}\
  .qkHl{position:absolute;left:8px;top:50%;transform:translateY(-50%);max-width:calc(100% - 16px);overflow:hidden;font-size:18px;font-family:Arial,sans-serif;line-height:1.25;white-space:pre;background:#0078d7;color:#fff;pointer-events:none;}\
  .qkLim{display:flex;flex-direction:column;gap:4px;margin-bottom:6px;}\
  .qkLim .row{display:flex;align-items:center;gap:6px;font-size:12px;}\
  .qkLim .row b{display:inline-block;width:104px;color:#123;}\
  .qkLim input{width:130px;height:24px;font-size:14px;border:1px inset #ddd;background:#fff;padding:0 4px;box-sizing:border-box;}\
  .qkKbs{display:flex;gap:10px;align-items:flex-start;}\
  .qkQ .qkr{display:flex;gap:4px;margin-bottom:4px;justify-content:center;}\
  .qkNp{display:grid;grid-template-columns:repeat(5,52px);gap:4px;}\
  .qk{min-width:40px;height:38px;font-size:14px;border:2px outset #e6e3d6;background:#eceadf;cursor:pointer;border-radius:3px;padding:0 6px;}\
  .qk:active{border-style:inset;}\
  .qk.k{color:#00c;font-weight:bold;}\
  .qk.wide{min-width:64px;}\
  .qk.spc{flex:1;}\
  .qk.dis{color:#aaa;background:#dcdcd2;cursor:default;}\
  .qk.ok{background:#dff0d8;}\
  .qk.ab{background:#f2dede;}\
  .qkNp .qk{min-width:0;width:100%;padding:0 2px;}\
  .qkNp .qk.k{font-size:12px;}\
  .qkNp .w2{grid-column:span 2;}\
  .qkHid{visibility:hidden;}';

  // AI(W906-KB-GOLDEN) 20261004: the six step captions of golden ChangeDecimalPoint (myQwertyKeyBoard.cpp:384-418).
  //   Order = spbAdd1, spbAdd10, spbAdd100, spbMinus1, spbMinus10, spbMinus100 (dfm:437-520: rows 1-3, the '+' column then the '-' column).
  var STEP = [
    ['+10',  '+100',  '+1000',  '-10',  '-100',  '-1000'],     // case 0 :386-393
    ['+1',   '+10',   '+100',   '-1',   '-10',   '-100'],      // case 1 :394-401
    ['+1.0', '+0.1',  '+0.01',  '-1.0', '-0.1',  '-0.01'],     // case 2 :402-409
    ['+0.1', '+0.01', '+0.001', '-0.1', '-0.01', '-0.001']     // case 3 :410-417
  ];
  // AI(W906-KB-GOLDEN) 20261004: the captions belong to the golden form (fQwertyKey / fQwertyKey2, :53-54). A dp that no case of the
  //   switch matches keeps that form's last captions; before the first match they are the design-time ones (dfm:437-520: '+1' '+10'
  //   '+100' '-1' '-10' '-100'). Golden passes such dps itself (N_DOUBLE dp 4 cContact.cpp:18557, dp 6 cConfiguration.cpp:5985 /
  //   cOffSet.cpp:3999, 906 tree; the wire tables copy them). ht9045_config_trayplate.js passes the server's kp.dp (golden 2, V912
  //   cConfiguration.cpp:6977 / :7113) since d1e77b4f; it used to pass 15. Golden's history is app-wide (one form);
  //   here it is per page (each page loads its own qwerty.js).
  var CAPS = [['+1', '+10', '+100', '-1', '-10', '-100'], ['+1', '+10', '+100', '-1', '-10', '-100']];

  // AI(W906-KB-GOLDEN) 20261004: one state object per open keypad (was one set of closure variables). cur = the keypad on top;
  //   under = golden fQwertyKey while fQwertyKey2 is up (:171-178).
  var cur = null, under = null;

  function ensureCss() {
    if (document.getElementById('qkCss')) return;
    var st = document.createElement('style'); st.id = 'qkCss'; st.textContent = CSS;
    document.head.appendChild(st);
  }

  // AI(W906-KB-GOLDEN) 20261004: the C / BCB6 number conversions the golden keypad uses.
  function atof(s) {        // C atof: leading white space, [sign] digits [. digits] [e [sign] digits]; stops at the first other char; nothing -> 0
    var m = /^[ \t\n\v\f\r]*([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)/.exec(String(s == null ? '' : s));
    return m ? Number(m[1]) : 0;
  }
  function ftol(r) {        // int(double) = BCB6 __ftol (Source/Rtl/Source/math/ftol.asm): chop to a 64-bit integer, keep the low 32 bits
    if (!(Math.abs(r) < 9223372036854775808)) return 0;   // FISTP qword overflow / NaN -> 0x8000000000000000 -> low 32 bits = 0
    return r | 0;                                          // ToInt32: truncate toward 0, then the low 32 bits as a signed int
  }
  function floatToStr(v) {  // AnsiString(double) = Sysutils::FloatToStr (dstring.cpp:154-157): ffGeneral, 15 digits (sysutils.pas:8273-8279);
                            // fixed while the decimal exponent is -3..15 (sysutils.pas:7290-7298), else "1E-5" / "1E15". Same as
                            // ht9045_temp_set_c.js vclFloatStr.
    if (v !== v) return 'NAN';
    if (!isFinite(v)) return v > 0 ? 'INF' : '-INF';
    if (v === 0) return '0';
    var m = /^(-?)(\d)\.?(\d*)e([+-]\d+)$/.exec(v.toExponential(14));
    var sign = m[1], dig = (m[2] + m[3]).replace(/0+$/, ''), e = parseInt(m[4], 10);
    if (e < -4 || e > 14) return sign + dig.charAt(0) + (dig.length > 1 ? '.' + dig.slice(1) : '') + 'E' + e;
    if (e < 0) return sign + '0.' + new Array(-e).join('0') + dig;
    while (dig.length < e + 1) dig += '0';
    return sign + dig.slice(0, e + 1) + (dig.length > e + 1 ? '.' + dig.slice(e + 1) : '');
  }
  function checkRange(value, maximum, minimum) {   // template CheckRange (golden MachineType.h:1519-1540), called as CheckRange(d, min, max)
    if (maximum < minimum) {                       // at myQwertyKeyBoard.cpp:290 -> min / max in either order clamp into the interval
      if (value > minimum) return minimum;
      else if (value < maximum) return maximum;
      return value;
    }
    if (value > maximum) return maximum;
    else if (value < minimum) return minimum;
    return value;
  }
  function num0(v) {                                // a min / max the caller left out (or not a number) is golden's default 0 (.h:153)
    var n = (v == null || v === '') ? 0 : Number(v);
    return n === n ? n : 0;
  }

  // AI(W906-KB-GOLDEN) 20261004: the flag tests of ShowQwertyKey (myQwertyKeyBoard.cpp:190-213, :285), now per keypad state S.
  function numPad(S) { return S.flags & (N.INTEGER | N.DOUBLE | N.PORT | N.IP_ADDR); }   // the numeric panel (:190-193, :207-213)
  function intDbl(S) { return S.flags & (N.INTEGER | N.DOUBLE); }                       // steps / '%' / '-' (:194-202), the clamp (:285)
  function pwd(S) { return S.flags & N.PASSWORD; }
  function alive(S) { return !!(S && S.ov && S.ov.parentNode); }
  // AI(W906-KB-GOLDEN) 20261004 (2/2): a keypad whose overlay page code removed by hand was closed without OK: its onAbort is called once
  //   (a promise-style caller would wait forever otherwise) and nothing is written -- the same contract as discard(). The state is
  //   updated first, then the callbacks run (one of them may call show() again).
  function dropStale(S) {
    var ab = S.opt.onAbort;
    S.opt.onAbort = null; S.opt.onCommit = null;
    if (typeof ab === 'function') ab();
  }
  function settle() {                       // AI(W906-KB-GOLDEN) 20261004: an overlay that page code removed by hand counts as closed
    var gone = [];
    if (under && !alive(under)) { gone.push(under); under = null; }                     // (2/2) the one underneath, removed by hand
    if (cur && !alive(cur)) { gone.push(cur); cur = alive(under) ? under : null; under = null; }
    for (var i = 0; i < gone.length; i++) dropStale(gone[i]);
  }

  // AI(W906-KB-GOLDEN) 20261004: the display shows golden's selection (S.sel = SelLength>0) as highlighted text; .value stays the shown text.
  function disp(S) {
    var t = pwd(S) ? S.val.replace(/./g, '*') : S.val;
    var on = !!(S.sel && t !== '');
    S.d.value = t;
    S.d.className = on ? 'qkDisp qkSel' : 'qkDisp';
    S.hl.textContent = on ? t : '';
    S.hl.style.display = on ? '' : 'none';
  }
  // AI(W906-KB-GOLDEN) 20261004: the key handlers, one per golden OnClick (myQwertyKeyBoard.cpp). Every write of edQwertyContent->Text
  //   (WM_SETTEXT) or SelStart clears the selection, so each one ends with sel = false except '%' (captions only).
  function typed(S, ch) {                   // spbKeyClick :320-339 -- every QWERTY key, the space bar and the numpad digits (:119-134)
    if (S.sel) S.val = '';                  // :325-328 SelLength>0 -> Text=""
    S.val += ch;                            // :330-337
    S.sel = false;                          // :338 SelStart=Length
    disp(S);
  }
  function backspace(S) {                   // spbBackSpaceClick :341-345 -- drops the last char of the old text even when it is selected
    S.val = S.val.slice(0, -1);
    S.sel = false;
    disp(S);
  }
  function clear(S) {                       // spbClearClick :352-355
    S.val = '';
    S.sel = false;
    disp(S);
  }
  function minus(S) {                       // spbMinusClick :421-431 -- toggles a leading '-' on the whole text, never clears it
    S.val = (S.val.charAt(0) === '-') ? S.val.substring(1) : '-' + S.val;
    S.sel = false;
    disp(S);
  }
  function dot(S) {                         // spbDPClick :451-458
    if (S.flags & N.INTEGER) return;        // :453-454 bIntegerOnly (:184)
    if (S.val.indexOf('.') < 0) S.val += '.';
    S.sel = false;                          // :457 SelStart=Length
    disp(S);
  }
  function percent(S) {                     // spbPercentClick :368-377 -- the value and the selection stay
    S.dp++;
    if (S.dp > 3) S.dp = 0;
    relabel(S);
  }
  function relabel(S) {                     // ChangeDecimalPoint :379-418
    if ((S.flags & N.INTEGER) && S.dp > 1) S.dp = 0;   // :381-382
    if (S.dp >= 0 && S.dp <= 3) CAPS[S.level] = STEP[S.dp].slice();
    for (var i = 0; i < S.stepBtns.length; i++) if (S.stepBtns[i]) S.stepBtns[i].textContent = CAPS[S.level][i];
  }
  function stepKey(S, i) {                  // spbAdd1Click :433-449 (all six step buttons)
    var r = atof(S.val) + atof(S.stepBtns[i].textContent);           // :440-442 atof(caption) + atof(text)
    S.val = (S.flags & N.INTEGER) ? String(ftol(r)) : r.toFixed(6);  // :444-447 AnsiString(int(dResult)) / sprintf("%1.6f")
    S.sel = false;                                                   // :448
    disp(S);
  }

  // AI(W906-KB-GOLDEN) 20261004: after ShowModal returns, OK or Abort alike (myQwertyKeyBoard.cpp:285-292): only N_INTEGER / N_DOUBLE,
  //   and only with checkRange, the text becomes AnsiString(CheckRange(atof(text), min, max)); otherwise it is left as it is.
  function post(S, text) {
    if (!intDbl(S)) return text;
    var d = atof(text);
    if (S.ck) text = floatToStr(checkRange(d, S.mn, S.mx));
    return text;
  }
  function writeTarget(S, v) {               // :296-301 EditPtr->Text / PanelPtr->Caption
    var tgt = S.tgt;
    if (!tgt) return;
    if ('value' in tgt && (tgt.tagName === 'INPUT' || tgt.tagName === 'TEXTAREA')) tgt.value = v;
    else tgt.textContent = v;
  }
  function closeTop(S) {
    if (S.ov) {
      if (typeof S.ov.remove === 'function') S.ov.remove();
      else if (S.ov.parentNode) S.ov.parentNode.removeChild(S.ov);
    }
    if (cur === S) { cur = under; under = null; }   // golden FormClose :145-150: fQwertyKey2 closed -> fQwertyKey is on top again
  }
  function discard(S) {                      // AI(W906-KB-GOLDEN) 20261004: dropped for a newer request on the same field (see show):
    var ab = S.opt.onAbort;                  //   onAbort as the old close() did, no post-step, nothing written
    S.opt.onAbort = null;
    closeTop(S);
    if (typeof ab === 'function') ab();
  }
  function commit(S) {                       // spbSummitClick :347-350 (OK / Enter buttons) -> Close() -> :285-301
    settle();                                // AI(W906-KB-GOLDEN) 20261004 (2/2): a top removed by hand is dropped first
    if (S !== cur) return;                   // golden: the form under a modal one gets no input
    var v = post(S, S.val);
    writeTarget(S, v);                       // always written on OK, as before (the engine's onCommit -> input / change relies on it)
    var cb = S.opt.onCommit;
    S.opt.onAbort = null;                    // contract kept: OK never calls onAbort (ht9045_temp_set_c.js TS-8 header)
    closeTop(S);
    if (typeof cb === 'function') cb(v);
  }
  // AI(W906-KB-GOLDEN) 20261004: Abort = golden spbCancelClick (:362-366 Text=sBackup; Close()) + the same post-step (:285-301), so an
  //   out-of-range original is clamped on Abort too when checkRange is on. VCL TControl::SetText writes only a different text
  //   (controls.pas:3723-3726): the field is written, and onCommit called (after onAbort), only when the result differs from the
  //   original; onAbort is always called (Abort contract of ht9045_temp_set_c.js TS-8 / TS-9 and the machine's TEACH-KB).
  function abort(S) {
    settle();                                // AI(W906-KB-GOLDEN) 20261004 (2/2): a top removed by hand is dropped first
    if (S !== cur) return;
    var v = post(S, S.backup);
    var changed = (v !== S.backup);
    if (changed) writeTarget(S, v);
    var ab = S.opt.onAbort, cb = S.opt.onCommit;
    S.opt.onAbort = null;
    closeTop(S);
    if (typeof ab === 'function') ab();
    if (changed && typeof cb === 'function') cb(v);
  }

  // AI(W906-KB-GOLDEN) 20261004: buttons act only while their keypad is on top, and do not take focus (golden TSpeedButton never does,
  //   so the selection of edQwertyContent survives every click).
  function keyBtn(S, label, cls, fn) {
    var b = document.createElement('button');
    b.className = 'qk' + (cls ? ' ' + cls : '');
    b.textContent = label;
    if (fn) b.addEventListener('click', function () { settle(); if (S === cur) fn(); });   // AI(W906-KB-GOLDEN) 20261004 (2/2): settle() first
    b.addEventListener('mousedown', function (e) { if (e && e.preventDefault) e.preventDefault(); });
    return b;
  }
  function hole(w2) {                        // a key golden hides (Visible=false) leaves its place empty
    var s = document.createElement('span');
    s.className = 'qkHid' + (w2 ? ' w2' : '');
    return s;
  }

  // AI(W906-KB-GOLDEN) 20261004: the numeric panel as golden palNumKey (dfm:102-520), rows 7 8 9 +a -a / 4 5 6 +b -b / 1 2 3 +c -c /
  //   0 . - % / BS Del Abort OK, where '%' and OK are two columns wide. Visibility as ShowQwertyKey :190-202: BS / Del / Abort / OK for
  //   any numeric flag, the steps / '%' / '-' only for INTEGER|DOUBLE without PASSWORD, '.' only for DOUBLE|IP_ADDR without PASSWORD; in
  //   text mode only the digits show. Hidden keys are holes, not disabled buttons.
  function buildNumpad(S) {
    var np = document.createElement('div'); np.className = 'qkNp';
    var isN = !!numPad(S), steps = !!intDbl(S) && !pwd(S), dpv = !!(S.flags & (N.DOUBLE | N.IP_ADDR)) && !pwd(S);
    function digit(ch) { return keyBtn(S, ch, '', function () { typed(S, ch); }); }
    function step(i) {
      if (!steps) { S.stepBtns[i] = null; return hole(false); }
      var b = keyBtn(S, CAPS[S.level][i], 'k', function () { stepKey(S, i); });
      S.stepBtns[i] = b;
      return b;
    }
    function cellBtn(vis, label, cls, fn, w2) { return vis ? keyBtn(S, label, cls + (w2 ? ' w2' : ''), fn) : hole(w2); }
    var cells = [
      digit('7'), digit('8'), digit('9'), step(0), step(3),
      digit('4'), digit('5'), digit('6'), step(1), step(4),
      digit('1'), digit('2'), digit('3'), step(2), step(5),
      digit('0'),
      cellBtn(dpv, '.', '', function () { dot(S); }, false),
      cellBtn(steps, '-', '', function () { minus(S); }, false),
      cellBtn(steps, '%', 'k', function () { percent(S); }, true),
      cellBtn(isN, 'BS', '', function () { backspace(S); }, false),
      cellBtn(isN, 'Del', '', function () { clear(S); }, false),
      cellBtn(isN, 'Abort', 'ab', function () { abort(S); }, false),
      cellBtn(isN, 'OK', 'ok', function () { commit(S); }, true)
    ];
    for (var i = 0; i < cells.length; i++) np.appendChild(cells[i]);
    return np;
  }

  // AI(W906-KB-GOLDEN) 20261004: the QWERTY panel keeps its layout and NO_SYMBOL / NO_SPACE / UPPERCASE handling; its keys now work on
  //   this keypad's state S (a character key replaces a selection, spbKeyClick :320-339; Abort is the golden Abort path :362-366).
  function buildQwerty(S) {
    var q = document.createElement('div'); q.className = 'qkQ';
    var rows = S.upper ? QU : QL;
    rows.forEach(function (row, ri) {
      var r = document.createElement('div'); r.className = 'qkr';
      if (ri === 3) r.appendChild(keyBtn(S, '文A', 'wide', function () { S.upper = !S.upper; rebuildQwerty(S); }));   // spbChangeCaseClick :304-318
      row.forEach(function (ch) {
        var b = keyBtn(S, ch, '', function () { typed(S, ch); });     // AI(W906-KB-GOLDEN) 20261004: spbKeyClick (replaces a selection)
        if ((S.flags & N.NO_SYMBOL) && isSym(ch)) b.classList.add('dis'), b.disabled = true;
        r.appendChild(b);
      });
      if (ri === 0) r.appendChild(keyBtn(S, '⌫', 'wide', function () { backspace(S); }));
      if (ri === 3) r.appendChild(keyBtn(S, 'Delete', 'wide', function () { clear(S); }));
      q.appendChild(r);
    });
    var last = document.createElement('div'); last.className = 'qkr';
    if (!(S.flags & N.NO_SPACE)) last.appendChild(keyBtn(S, ' ', 'spc', function () { typed(S, ' '); }));
    last.appendChild(keyBtn(S, 'Abort', 'ab wide', function () { abort(S); }));   // AI(W906-KB-GOLDEN) 20261004: spbCancel -> the Abort path
    last.appendChild(keyBtn(S, 'Enter', 'ok wide', function () { commit(S); }));
    q.appendChild(last);
    return q;
  }
  function rebuildQwerty(S) {
    var old = S.q, nq = buildQwerty(S);
    if (old && typeof old.replaceWith === 'function') old.replaceWith(nq);
    else if (old && old.parentNode) { old.parentNode.insertBefore(nq, old); old.parentNode.removeChild(old); }
    S.q = nq;
  }

  function show(target, f, o) {
    ensureCss();
    // AI(W906-KB-GOLDEN) 20261004: golden ShowQwertyKey :171-181 -- a request while the keypad is up opens fQwertyKey2 over it (the
    //   first keeps its text; its comment: an alarm during input used to hang); with both up the call returns at once and the caller
    //   goes on with its unchanged field, so onAbort is called (promise-style callers would wait forever otherwise). Was: abort the
    //   open keypad, its typed value lost.
    settle();
    // AI(W906-KB-GOLDEN) 20261004: the same field asked again while its keypad is up is two binders on one input, not golden's alarm
    //   case -- Setup.Contact.html (ht9045_contact_wire.js) and Setup.HotPlate.html (ht9045_hotplate_wire.js) bind mousedown on every
    //   text input before the engine binds them again, so one tap calls show() twice. The later request replaces the open one as
    //   before (stacking them would leave a second keypad with the old value under the first).
    if (cur && target && cur.tgt === target) discard(cur);
    if (cur) {
      if (under) { if (o && typeof o.onAbort === 'function') o.onAbort(); return; }
      under = cur;
    }
    var S = { level: under ? 1 : 0, tgt: target || null, flags: f || 0, opt: o || {}, stepBtns: [], q: null };
    var opt = S.opt;

    /* Steven 20260918 (W906-FW-KBRANGE)
       上下限「伺服器有就用伺服器的」。opt.rangeHook 由 ht9045_wire_engine.js
       在綁定時掛上，**開啟的當下**才呼叫 —— 存檔→重讀之後再打開，看到的
       必須是新的限值，不是綁定當時那一份。
       沒有 hook、或這個欄位伺服器沒給 min/max 時，opt 原本的靜態值原樣保留，
       行為與 20260918 之前完全相同。 */
    opt.rangeFrom = 'wire';
    if (typeof opt.rangeHook === 'function') {
      var _r = null;
      try { _r = opt.rangeHook(); } catch (e) { _r = null; }
      if (_r) {
        opt.min = _r.min; opt.max = _r.max;
        opt.checkRange = true;          // 伺服器講了值域，就一定要擋
        opt.rangeFrom = 'server';
      }
    }
    S.upper = !!(S.flags & N.UPPERCASE);
    var val = target ? (('value' in target && target.value != null && target.tagName === 'INPUT') ? target.value : (target.textContent || '')) : '';
    S.val = String(val).trim();
    // AI(W906-KB-GOLDEN) 20261004: sBackup (:238) for Abort; the whole text starts selected (FormShow :137-143 + AutoSelect, see the header).
    S.backup = S.val;
    S.sel = S.val !== '';
    // AI(W906-KB-GOLDEN) 20261004: iDecimalPoint = iDP every time (:185; default 0, myQwertyKeyBoard.h:153); bCheckRange / min / max as
    //   given, then golden's N_PORT rule (:249-257): forces the range check and 0..65535 when min<0 || max<=0. The clamp itself still
    //   needs INTEGER|DOUBLE (:285), so for a PORT-only field this only shows Maximum / Minimum.
    S.dp = parseInt(opt.dp, 10);
    if (isNaN(S.dp)) S.dp = 0;
    // AI(W906-KB-GOLDEN) 20261004: min / max default to 0 like golden's parameters (myQwertyKeyBoard.h:153 double min=0, double max=0),
    //   so a PORT keypad opened without them gets golden's 0..65535 (:252-256) instead of NaN ("NAN" in Maximum / Minimun).
    S.ck = !!opt.checkRange; S.mn = num0(opt.min); S.mx = num0(opt.max); S.portRange = false;
    if (S.flags & N.PORT) {
      S.ck = true;
      if (S.mn < 0 || S.mx <= 0) { S.mn = 0; S.mx = 65535; S.portRange = true; }
    }

    var ov = document.createElement('div'); ov.className = 'qkOv';
    var win = document.createElement('div'); win.className = 'qkWin';
    // AI(W906-KB-GOLDEN) 20261004: no ✕ -- golden BorderIcons=[] with bsSingle (myQwertyKeyBoard.dfm:4-5; BCB6 forms.pas:3594 sets
    //   WS_SYSMENU only with biSystemMenu), so the caption bar has no close box; was an ✕ that aborted.
    win.innerHTML = '<div class="qkBar"><span>⌨ Qwerty Keyboard</span></div>';
    var body = document.createElement('div'); body.className = 'qkBody';

    // AI(W906-KB-GOLDEN) 20261004: the display keeps .qkDisp (an <input> whose .value is the shown text); .qkHl over it draws the selection.
    var dw = document.createElement('div'); dw.className = 'qkDispW';
    var d = document.createElement('input'); d.className = 'qkDisp'; d.readOnly = true; dw.appendChild(d);
    var hl = document.createElement('span'); hl.className = 'qkHl'; dw.appendChild(hl);
    body.appendChild(dw);
    S.d = d; S.hl = hl;

    // Current Value / Max / Min（密碼不顯示 Current Value）
    if (!pwd(S)) {
      var lim = document.createElement('div'); lim.className = 'qkLim';
      lim.innerHTML = '<div class="row"><b>Current Value :</b><input value="' + esc(S.val) + '" readonly></div>';
      if (S.ck) {
        // AI(W906-KB-GOLDEN) 20261004: golden :259-271 -- the larger one is Maximum, both as AnsiString(double); the range after N_PORT.
        var big = S.mx > S.mn;
        lim.innerHTML += '<div class="row"><b>Maximum :</b><input value="' + esc(floatToStr(big ? S.mx : S.mn)) + '" readonly></div>' +
                         '<div class="row"><b>Minimun :</b><input value="' + esc(floatToStr(big ? S.mn : S.mx)) + '" readonly></div>';
        // 限值的出處要講出來。兩者的意義差很多：server 是機台現在的說法，
        // wire 是接線檔產生當時從 golden 抄下來的靜態值，可能已經過期。
        lim.innerHTML += '<div class="row qkSrc" style="font-size:11px;color:#555;">' +
                         (S.portRange
                            ? '上下限來源：golden N_PORT 預設 0~65535（myQwertyKeyBoard.cpp:249-257）'
                            : opt.rangeFrom === 'server'
                            ? '上下限來源：機台（本次讀取隨資料帶回）'
                            : '上下限來源：接線檔靜態值（機台未提供）') + '</div>';
      }
      body.appendChild(lim);
    }

    var kbs = document.createElement('div'); kbs.className = 'qkKbs';
    if (numPad(S)) {
      kbs.appendChild(buildNumpad(S));               // 數字類：只有數字鍵盤（窄）
    } else {
      S.q = buildQwerty(S);
      kbs.appendChild(S.q);                          // 文字/密碼：全 QWERTY
      if (!(S.flags & N.NO_NUM_PAD)) kbs.appendChild(buildNumpad(S));
    }
    body.appendChild(kbs);
    win.appendChild(body); ov.appendChild(win);
    S.ov = ov;
    // AI(W906-KB-GOLDEN) 20261004: the second keypad goes above the first and first in document order, so document.querySelector('.qkOv')
    //   (ht9045_wire_engine.js physicalKeys, ht9045_config_trayplate.js patchKeys) finds the one on top.
    if (S.level && under && under.ov && under.ov.parentNode && typeof under.ov.parentNode.insertBefore === 'function') {
      ov.style.zIndex = '100000';
      under.ov.parentNode.insertBefore(ov, under.ov);
    } else {
      document.body.appendChild(ov);
    }
    // AI(W906-KB-GOLDEN) 20261004: no click / mousedown handler on the dimmed overlay -- golden is ShowModal (:283), a click outside the
    //   keypad does nothing. Was: a mousedown outside aborted and the typed value was lost.
    cur = S;
    relabel(S);                                      // ChangeDecimalPoint() at every show (:187)
    disp(S);
  }
  function esc(s) { return String(s).replace(/"/g, '&quot;').replace(/</g, '&lt;'); }

  // AI(W906-KB-GOLDEN) 20261004: no physical-key code here. The user's 20260915 Rule B (docs/web-client/REPLICATE.md §2.5, "must not
  //   be violated") keeps physical keys in ht9045_wire_engine.js physicalKeys(): a key clicks the keypad button with the same label,
  //   Escape -> 'Abort', anything not on the keypad is refused. So a physical key does what that button does here (golden's on-screen
  //   semantics: a digit replaces the selection, '-' toggles, '%' cycles the step captions, Escape = the Abort post-step). Golden's own
  //   edQwertyContent rules (myQwertyKeyBoard.cpp:460-538: Enter = OK on the numeric pad, Escape does nothing, '%' refused) differ in
  //   three places; changing Rule B is the user's decision. Keys that a hidden golden button would have had are holes (no .qk), so
  //   the label map cannot reach them ('.' on an INTEGER pad).
  //   AI(W906-KB-GOLDEN) 20261004 (2/2): the engine's PHYS_MAP now also names the numeric pad's buttons -- Backspace -> 'BS', Delete ->
  //   'Del', Enter -> 'OK' (so Enter = OK on the numeric pad, as golden's KeyDown :463-466) -- and a key that matches no button is
  //   swallowed while a keypad is open (golden ShowModal: no key reaches the form behind). Escape = Abort stays (Rule B requires it).
  window.HTQwerty = { N: N, show: show };
})();
