/* HT9xxx 小鍵盤（TfQwertyKey / myQwertyKeyBoard 的 HTML 模擬）
   用法：HTQwerty.show(targetEl, flags, {dp, checkRange, min, max})
   flags 為 N_ 旗標（cmydef.cpp）位元 OR：
     HTQwerty.N.INTEGER / DOUBLE / NO_SYMBOL / PASSWORD / NO_SPACE / UPPERCASE / NO_NUM_PAD / PORT / IP_ADDR
   依 ShowQwertyKey() 設計：數字類→只顯示數字鍵盤(窄)；其餘→全 QWERTY(可含數字鍵盤)；
   密碼→遮罩、隱藏 Current Value；checkRange→顯示 Maximum/Minimum 並於 OK 時夾限。 */
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

  var CSS = '\
  .qkOv{position:fixed;inset:0;background:rgba(0,0,0,.35);z-index:99999;display:flex;align-items:center;justify-content:center;font-family:"Microsoft JhengHei",sans-serif;}\
  .qkWin{background:#c8c8bc;border:1px solid #6b6b60;border-radius:4px;box-shadow:4px 4px 16px rgba(0,0,0,.5);max-width:96vw;max-height:96vh;overflow:auto;}\
  .qkBar{background:linear-gradient(90deg,#0a246a,#3a6ea5);color:#fff;font-size:12px;font-weight:bold;padding:4px 8px;display:flex;align-items:center;gap:6px;}\
  .qkBar .qkx{margin-left:auto;cursor:pointer;border:1px solid #99a;background:#d4d0c8;color:#000;width:18px;height:16px;line-height:14px;text-align:center;border-radius:2px;}\
  .qkBody{padding:8px;}\
  .qkDisp{width:100%;height:34px;font-size:18px;box-sizing:border-box;border:2px inset #ddd;padding:2px 6px;margin-bottom:6px;background:#fff;}\
  .qkLim{display:flex;flex-direction:column;gap:4px;margin-bottom:6px;}\
  .qkLim .row{display:flex;align-items:center;gap:6px;font-size:12px;}\
  .qkLim .row b{display:inline-block;width:104px;color:#123;}\
  .qkLim input{width:130px;height:24px;font-size:14px;border:1px inset #ddd;background:#fff;padding:0 4px;box-sizing:border-box;}\
  .qkKbs{display:flex;gap:10px;align-items:flex-start;}\
  .qkQ .qkr{display:flex;gap:4px;margin-bottom:4px;justify-content:center;}\
  .qkNp{display:grid;grid-template-columns:repeat(5,44px);gap:4px;}\
  .qk{min-width:40px;height:38px;font-size:14px;border:2px outset #e6e3d6;background:#eceadf;cursor:pointer;border-radius:3px;padding:0 6px;}\
  .qk:active{border-style:inset;}\
  .qk.k{color:#00c;font-weight:bold;}\
  .qk.wide{min-width:64px;}\
  .qk.spc{flex:1;}\
  .qk.dis{color:#aaa;background:#dcdcd2;cursor:default;}\
  .qk.ok{background:#dff0d8;}\
  .qk.ab{background:#f2dede;}\
  .qkNp .qk{min-width:44px;}';

  var ov, tgt, val, upper, opt = {}, flags = 0;

  function ensureCss() {
    if (document.getElementById('qkCss')) return;
    var st = document.createElement('style'); st.id = 'qkCss'; st.textContent = CSS;
    document.head.appendChild(st);
  }
  function num() { return flags & (N.INTEGER | N.DOUBLE | N.PORT | N.IP_ADDR); }
  function pwd() { return flags & N.PASSWORD; }
  function disp() {
    var d = ov.querySelector('.qkDisp');
    d.value = pwd() ? val.replace(/./g, '*') : val;
  }
  function put(ch) { val += ch; disp(); }
  function bs() { val = val.slice(0, -1); disp(); }
  function clr() { val = ''; disp(); }
  function adj(n) { var v = parseFloat(val) || 0; v += n; val = String((flags & N.DOUBLE) ? v : Math.round(v)); disp(); }
  function pct() { var v = parseFloat(val) || 0; val = String((flags & N.DOUBLE) ? v / 100 : Math.round(v / 100)); disp(); }
  function commit() {
    if (num() && opt.checkRange) {
      var v = parseFloat(val) || 0, lo = Math.min(opt.min, opt.max), hi = Math.max(opt.min, opt.max);
      if (v < lo) v = lo; if (v > hi) v = hi;
      val = String((flags & N.DOUBLE) ? v : Math.round(v));   // AI(W906-TIF-P3) 20261002 (St02-E, claim): golden myQwertyKeyBoard.cpp:290 edQwertyContent->Text=AnsiString(CheckRange(d, min, max)) -- the clamped value as is; the decimals argument only sets the +/- step (:379-418)
    }
    if (tgt) {
      if ('value' in tgt && (tgt.tagName === 'INPUT' || tgt.tagName === 'TEXTAREA')) tgt.value = val;
      else tgt.textContent = val;
    }
    var cb = opt.onCommit, v = val;
    opt.onAbort = null;
    close();
    if (typeof cb === 'function') cb(v);
  }
  function close() {
    if (ov) { ov.remove(); ov = null; }
    var ab = opt.onAbort; opt.onAbort = null;
    if (typeof ab === 'function') ab();
  }

  function keyBtn(label, cls, fn) {
    var b = document.createElement('button');
    b.className = 'qk' + (cls ? ' ' + cls : '');
    b.textContent = label;
    if (fn) b.addEventListener('click', fn);
    return b;
  }

  function buildNumpad() {
    var np = document.createElement('div'); np.className = 'qkNp';
    var showAdj = num() && !pwd();
    var showDp  = (flags & N.DOUBLE || flags & N.IP_ADDR) && !pwd();
    var showPct = num() && !pwd();
    var rows = [
      [['7'],['8'],['9'],['+1',showAdj,function(){adj(1);}],['-1',showAdj,function(){adj(-1);}]],
      [['4'],['5'],['6'],['+10',showAdj,function(){adj(10);}],['-10',showAdj,function(){adj(-10);}]],
      [['1'],['2'],['3'],['+100',showAdj,function(){adj(100);}],['-100',showAdj,function(){adj(-100);}]],
      [['0'],['.',showDp,function(){if(val.indexOf('.')<0)put('.');}],['-',num()&&!pwd(),function(){put('-');}],['%',showPct,pct],[null]],
      [['BS',true,bs],['Del',true,clr],['Abort','ab',close2],['OK','ok',commit],[null]]
    ];
    function close2(){ close(); }
    rows.forEach(function (r) {
      r.forEach(function (c) {
        if (!c || c[0] === null) { var s = document.createElement('span'); np.appendChild(s); return; }
        var lbl = c[0];
        if (c.length === 1) { np.appendChild(keyBtn(lbl, '', function () { put(lbl); })); return; }
        var vis = c[1], fn = c[2];
        var cls = (lbl === 'OK') ? 'ok' : (lbl === 'Abort') ? 'ab' : (/^[+\-]\d|%/.test(lbl) ? 'k' : '');
        var b = keyBtn(lbl, cls, fn);
        if (!vis) b.classList.add('dis'), b.disabled = true;
        np.appendChild(b);
      });
    });
    return np;
  }

  function buildQwerty() {
    var q = document.createElement('div'); q.className = 'qkQ';
    var rows = upper ? QU : QL;
    rows.forEach(function (row, ri) {
      var r = document.createElement('div'); r.className = 'qkr';
      if (ri === 3) r.appendChild(keyBtn('文A', 'wide', function () { upper = !upper; rebuildQwerty(); }));
      row.forEach(function (ch) {
        var b = keyBtn(ch, '', function () { put(ch); });
        if ((flags & N.NO_SYMBOL) && isSym(ch)) b.classList.add('dis'), b.disabled = true;
        r.appendChild(b);
      });
      if (ri === 0) r.appendChild(keyBtn('⌫', 'wide', bs));
      if (ri === 3) r.appendChild(keyBtn('Delete', 'wide', clr));
      q.appendChild(r);
    });
    var last = document.createElement('div'); last.className = 'qkr';
    if (!(flags & N.NO_SPACE)) last.appendChild(keyBtn(' ', 'spc', function () { put(' '); }));
    last.appendChild(keyBtn('Abort', 'ab wide', close));
    last.appendChild(keyBtn('Enter', 'ok wide', commit));
    q.appendChild(last);
    return q;
  }
  function rebuildQwerty() {
    var old = ov.querySelector('.qkQ');
    if (old) old.replaceWith(buildQwerty());
  }

  function show(target, f, o) {
    ensureCss();
    close();
    tgt = target; flags = f || 0; opt = o || {}; upper = !!(flags & N.UPPERCASE);

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
    val = target ? (('value' in target && target.value != null && target.tagName === 'INPUT') ? target.value : (target.textContent || '')) : '';
    val = String(val).trim();

    ov = document.createElement('div'); ov.className = 'qkOv';
    var win = document.createElement('div'); win.className = 'qkWin';
    win.innerHTML = '<div class="qkBar"><span>⌨ Qwerty Keyboard</span><span class="qkx">✕</span></div>';
    var body = document.createElement('div'); body.className = 'qkBody';

    var d = document.createElement('input'); d.className = 'qkDisp'; d.readOnly = true; body.appendChild(d);

    // Current Value / Max / Min（密碼不顯示 Current Value）
    if (!pwd()) {
      var lim = document.createElement('div'); lim.className = 'qkLim';
      lim.innerHTML = '<div class="row"><b>Current Value :</b><input value="' + esc(val) + '" readonly></div>';
      if (opt.checkRange) {
        lim.innerHTML += '<div class="row"><b>Maximum :</b><input value="' + esc(Math.max(opt.min, opt.max)) + '" readonly></div>' +
                         '<div class="row"><b>Minimun :</b><input value="' + esc(Math.min(opt.min, opt.max)) + '" readonly></div>';
        // 限值的出處要講出來。兩者的意義差很多：server 是機台現在的說法，
        // wire 是接線檔產生當時從 golden 抄下來的靜態值，可能已經過期。
        lim.innerHTML += '<div class="row qkSrc" style="font-size:11px;color:#555;">' +
                         (opt.rangeFrom === 'server'
                            ? '上下限來源：機台（本次讀取隨資料帶回）'
                            : '上下限來源：接線檔靜態值（機台未提供）') + '</div>';
      }
      body.appendChild(lim);
    }

    var kbs = document.createElement('div'); kbs.className = 'qkKbs';
    if (num()) {
      kbs.appendChild(buildNumpad());               // 數字類：只有數字鍵盤（窄）
    } else {
      kbs.appendChild(buildQwerty());               // 文字/密碼：全 QWERTY
      if (!(flags & N.NO_NUM_PAD)) kbs.appendChild(buildNumpad());
    }
    body.appendChild(kbs);
    win.appendChild(body); ov.appendChild(win);
    document.body.appendChild(ov);
    win.querySelector('.qkx').addEventListener('click', close);
    ov.addEventListener('mousedown', function (e) { if (e.target === ov) close(); });
    disp();
  }
  function esc(s) { return String(s).replace(/"/g, '&quot;').replace(/</g, '&lt;'); }

  window.HTQwerty = { N: N, show: show };
})();
