/* HT9xxx 自訂 VCL 元件 HTML 模板庫
 * 對應原始碼：
 *   TALed        elec/Component/aled.pas    （6 種 LEDStyle、TrueColor/FalseColor/Blink/Interval/Value）
 *   TMyLed       elec/myvcl/MyLed.h         （TALed + Port/Bit/Type/Alias）
 *   TMyLedLane   elec/myvcl/MyLedLane.h     （TALed + Ring/IP/Port/Bit/Type/Alias/IsISA）
 *   TLabeledALed / TMyLabeledLed / TMyLabeledLedLane（虛擬元件，尚未實作於 BCB6）
 *                → LED + Caption，CaptionPos 可 lpLeft/lpRight/lpTop/lpBottom（預設 lpRight）
 *   TBtnPanel    elec/myvcl/butPa1.h        （TPanel + Down/TrueColor/FalseColor/TrueFontColor/FalseFontColor/Port/Bit/Type/Alias/Style）
 *   TBtnPanelLane elec/myvcl/BtnPanelLane.h （TBtnPanel + Lane/IP/IsISA）
 *   TTMyTray     elec/myvcl/HTray.h         （XItem/YItem/XBlockItem/YBlockItem/LineWidth/EdgeWidth/TrayDirect/ShowFont/ColorMap/CellText）
 */
(function () {
  'use strict';

  // ---------- 共用樣式（載入時注入一次） ----------
  var css = [
    /* TALed：LED 本體，6 種樣式對應 TLEDStyle */
    '.aled{display:inline-block;vertical-align:middle;box-sizing:border-box;',
    '  border:1px solid #666;background:var(--led-off,#c0c0c0);}',
    '.aled.on{background:var(--led-on,#00ff00);box-shadow:inset 1px 1px 3px rgba(255,255,255,.8),0 0 4px var(--led-on,#00ff00);}',
    /* Steven 20260918：接執行期 tag 的燈，第三種狀態「不可知」(null)。
       一顆熄掉的燈和一顆沒有來源的燈如果長得一樣，操作員會把「不知道」
       讀成「關著」。斜線紋是刻意選的 —— 顏色可以被佈景蓋掉，紋路不會。 */
    '.aled.unknown{background:repeating-linear-gradient(45deg,#8a8a8a 0 3px,#bdbdbd 3px 6px)!important;',
    '  box-shadow:none!important;border-style:dashed;opacity:.85;}',
    '.aled.LEDSmall{width:14px;height:14px;border-radius:50%;}',
    '.aled.LEDLarge{width:22px;height:22px;border-radius:50%;}',
    '.aled.LEDSqSmall{width:14px;height:14px;border-radius:2px;}',
    '.aled.LEDSqLarge{width:22px;height:22px;border-radius:2px;}',
    '.aled.LEDVertical{width:10px;height:22px;border-radius:2px;}',
    '.aled.LEDHorizontal{width:22px;height:10px;border-radius:2px;}',
    '.aled.blink{animation:aledBlink var(--led-interval,1s) steps(1) infinite;}',
    '@keyframes aledBlink{0%,100%{background:var(--led-on,#00ff00);}50%{background:var(--led-off,#c0c0c0);box-shadow:none;}}',
    /* LED + Alias 文字的包裝 */
    '.ledbox{display:inline-flex;align-items:center;gap:4px;font:11px var(--font-ui,"Microsoft JhengHei",sans-serif);margin:2px 6px;white-space:nowrap;}',
    /* TLabeledALed 系列：LED + Caption，CaptionPos 控制四方位（flex 流式版，供模板/新畫面用） */
    '.lledf{display:inline-flex;align-items:center;gap:4px;margin:2px 10px;font:11px var(--font-ui,"Microsoft JhengHei",sans-serif);white-space:nowrap;}',
    '.lledf.lpLeft{flex-direction:row-reverse;}',
    '.lledf.lpTop{flex-direction:column-reverse;gap:2px;}',
    '.lledf.lpBottom{flex-direction:column;gap:2px;}',
    /* TBtnPanel / TBtnPanelLane：可按壓面板按鈕 */
    '.btnpanel{display:inline-flex;align-items:center;justify-content:center;box-sizing:border-box;',
    '  min-width:72px;min-height:26px;padding:3px 10px;margin:2px;cursor:pointer;user-select:none;',
    '  font:bold 11px var(--font-ui,"Microsoft JhengHei",sans-serif);text-align:center;',
    '  border:2px outset #e6e3d6;background:var(--bp-false,#ece9d8);color:var(--bp-false-font,#000);}',
    '.btnpanel.down{border-style:inset;background:var(--bp-true,#00c000);color:var(--bp-true-font,#fff);}',
    '.btnpanel.flat{border:1px solid #888;border-radius:4px;}',   /* Style=tsFlatButtons（新版 GUI） */
    /* TTMyTray：Tray 格盤 */
    '.mytray{display:inline-block;background:var(--tray-color,#8a9a8a);',
    '  border:var(--tray-edge,2px) solid #444;padding:4px;position:relative;line-height:0;}',
    '.mytray .trow{display:flex;}',
    '.mytray .cell{box-sizing:border-box;display:inline-flex;align-items:center;justify-content:center;',
    '  border:var(--tray-line,1px) solid #555;background:#fff;',
    '  font:9px/1 var(--font-ui,"Segoe UI",sans-serif);color:#123;overflow:hidden;}',
    '.mytray .dirmark{position:absolute;width:0;height:0;border:8px solid transparent;}',
    '.mytray .dirmark.csLeftTop{left:0;top:0;border-left-color:#ff0;border-top-color:#ff0;}',
    '.mytray .dirmark.csLeftBottom{left:0;bottom:0;border-left-color:#ff0;border-bottom-color:#ff0;}',
    '.mytray .dirmark.csRightTop{right:0;top:0;border-right-color:#ff0;border-top-color:#ff0;}',
    '.mytray .dirmark.csRightBottom{right:0;bottom:0;border-right-color:#ff0;border-bottom-color:#ff0;}'
  ].join('\n');
  var st = document.createElement('style');
  st.textContent = css;
  document.head.appendChild(st);

  function el(tag, cls) { var e = document.createElement(tag); if (cls) e.className = cls; return e; }

  // ---------- TALed ----------
  // opt: {name, value, blink, interval(ms), ledStyle:'LEDSmall|LEDLarge|LEDSqSmall|LEDSqLarge|LEDVertical|LEDHorizontal',
  //       trueColor, falseColor, alias}
  function makeALed(opt) {
    opt = opt || {};
    var d = el('span', 'aled ' + (opt.ledStyle || 'LEDSmall'));
    d.id = opt.name || '';
    d.style.setProperty('--led-on', opt.trueColor || '#00ff00');   // TrueColor default clLime
    d.style.setProperty('--led-off', opt.falseColor || '#c0c0c0'); // FalseColor default clSilver
    if (opt.value) d.classList.add('on');
    if (opt.blink) {
      d.classList.add('blink');
      d.style.setProperty('--led-interval', ((opt.interval || 1000) * 2) + 'ms'); // Interval=半週期
    }
    d.title = (opt.name || 'TALed') + ' : TALed' + (opt.alias ? '｜Alias=' + opt.alias : '');
    // 對應 ChangeValue()
    d.setValue = function (v) { d.classList.toggle('on', !!v); };
    return d;
  }

  // ---------- TMyLed / TMyLedLane ----------
  // TMyLed 增加 Port/Bit/Type/Alias；TMyLedLane 再增加 Ring/IP/IsISA
  function makeMyLed(opt) {
    opt = opt || {};
    var box = el('span', 'ledbox');
    var led = makeALed(opt);
    var io = ['Port=' + (opt.port || ''), 'Bit=' + (opt.bit || ''), 'Type=' + (opt.type || '')];
    if (opt.ring !== undefined) io.unshift('Ring=' + opt.ring, 'IP=' + (opt.ip || ''));
    if (opt.isISA !== undefined) io.push('IsISA=' + opt.isISA);
    var typ = (opt.ring !== undefined) ? 'TMyLedLane' : 'TMyLed';
    led.title = (opt.name || typ) + ' : ' + typ + '｜' + io.join(' ') + (opt.alias ? '｜Alias=' + opt.alias : '');
    box.appendChild(led);
    if (opt.alias || opt.caption) {
      var lb = el('span');
      lb.textContent = opt.alias || opt.caption;
      box.appendChild(lb);
    }
    box.dataset.port = opt.port || ''; box.dataset.bit = opt.bit || '';
    box.setValue = led.setValue;
    return box;
  }
  function makeMyLedLane(opt) {
    opt = opt || {}; if (opt.ring === undefined) opt.ring = '';
    return makeMyLed(opt);
  }

  // ---------- TLabeledALed / TMyLabeledLed / TMyLabeledLedLane ----------
  // opt 同對應 LED，另加 {caption, captionPos:'lpLeft|lpRight|lpTop|lpBottom'(預設 lpRight), captionColor, captionBold}
  function makeLabeledLed(baseMaker, vtyp, opt) {
    opt = opt || {};
    var pos = opt.captionPos || 'lpRight';
    var box = el('span', 'lledf ' + pos);
    box.id = opt.name || '';
    var led = (baseMaker === makeALed) ? makeALed(opt) : baseMaker(opt);
    // makeMyLed 回傳 .ledbox 包裝；取內部 LED 本體避免雙層包裝
    var core = led.classList && led.classList.contains('ledbox') ? led.querySelector('.aled') : led;
    box.appendChild(core);
    var lb = el('span', 'lledfCap');
    lb.textContent = opt.caption || '';
    if (opt.captionColor) lb.style.color = opt.captionColor;
    if (opt.captionBold) lb.style.fontWeight = 'bold';
    box.appendChild(lb);
    box.title = (opt.name || vtyp) + ' : ' + vtyp + '｜Caption=' + (opt.caption || '') +
      ' CaptionPos=' + pos + (opt.alias ? '｜Alias=' + opt.alias : '');
    core.title = box.title;
    box.setValue = core.setValue || function () {};
    box.setCaption = function (s) { lb.textContent = s; };
    return box;
  }
  function makeLabeledALed(opt)     { return makeLabeledLed(makeALed,     'TLabeledALed', opt); }
  function makeMyLabeledLed(opt)    { return makeLabeledLed(makeMyLed,    'TMyLabeledLed', opt); }
  function makeMyLabeledLedLane(opt){ if (opt && opt.ring === undefined) opt.ring = '';
    return makeLabeledLed(makeMyLed, 'TMyLabeledLedLane', opt); }

  // ---------- TBtnPanel / TBtnPanelLane ----------
  // opt: {name, caption, down, trueColor, falseColor, trueFontColor, falseFontColor,
  //       port, bit, type, alias, lane, ip, isISA, flat, onChange}
  function makeBtnPanel(opt) {
    opt = opt || {};
    var b = el('div', 'btnpanel' + (opt.flat ? ' flat' : ''));   // Style=tsFlatButtons → flat
    b.id = opt.name || '';
    b.textContent = opt.caption || opt.alias || opt.name || 'TBtnPanel';
    b.style.setProperty('--bp-true', opt.trueColor || '#00c000');
    b.style.setProperty('--bp-false', opt.falseColor || '#ece9d8');
    b.style.setProperty('--bp-true-font', opt.trueFontColor || '#ffffff');
    b.style.setProperty('--bp-false-font', opt.falseFontColor || '#000000');
    if (opt.down) b.classList.add('down');
    var io = ['Port=' + (opt.port || ''), 'Bit=' + (opt.bit || ''), 'Type=' + (opt.type || '')];
    var typ = (opt.lane !== undefined) ? 'TBtnPanelLane' : 'TBtnPanel';
    if (opt.lane !== undefined) io.unshift('Lane=' + opt.lane, 'IP=' + (opt.ip || ''));
    if (opt.isISA !== undefined) io.push('IsISA=' + opt.isISA);
    b.title = (opt.name || typ) + ' : ' + typ + '｜' + io.join(' ') + (opt.alias ? '｜Alias=' + opt.alias : '');
    // 對應 SetPanelStatus()：點擊切換 Down（輸出 ON/OFF）
    b.addEventListener('click', function () {
      b.classList.toggle('down');
      if (opt.onChange) opt.onChange(b.classList.contains('down'));
    });
    b.setDown = function (v) { b.classList.toggle('down', !!v); };
    return b;
  }
  function makeBtnPanelLane(opt) {
    opt = opt || {}; if (opt.lane === undefined) opt.lane = '';
    return makeBtnPanel(opt);
  }

  // ---------- TSpeedButton ----------
  // opt: {name, caption, img(glyph URL), w, h, fs, layout:'left|top', flat, onClick}
  // 對應 VCL TSpeedButton：Glyph 圖示＋Caption 置中排列（blGlyphLeft/blGlyphTop）
  function makeSpeedButton(opt) {
    opt = opt || {};
    var b = el('button', 'btn3d spdbtn');
    b.type = 'button';
    b.id = opt.name || '';
    b.title = (opt.name || 'TSpeedButton') + ' : TSpeedButton';
    b.style.cssText = 'overflow:hidden;white-space:nowrap;display:inline-flex;align-items:center;justify-content:center;gap:3px;padding:0 2px;box-sizing:border-box;font-family:var(--font-ui,sans-serif);';
    if (opt.w) b.style.width = opt.w + 'px';
    if (opt.h) b.style.height = opt.h + 'px';
    b.style.fontSize = (opt.fs || 11) + 'px';
    if (opt.layout === 'top' || opt.layout === 'blGlyphTop') b.style.flexDirection = 'column';
    if (opt.flat) { b.style.border = 'none'; b.style.background = 'transparent'; }
    var im = null;
    if (opt.img) {
      im = document.createElement('img');
      im.src = opt.img; im.alt = ''; im.style.flex = 'none';
      b.appendChild(im);
    }
    if (opt.caption) b.appendChild(document.createTextNode(opt.caption));
    b.setGlyph = function (url) {
      if (!im) { im = document.createElement('img'); im.alt = ''; im.style.flex = 'none'; b.insertBefore(im, b.firstChild); }
      im.src = url;
    };
    if (opt.onClick) b.addEventListener('click', opt.onClick);
    return b;
  }

  // ---------- TTMyTray ----------
  // opt: {name, xitem, yitem, xblockItem, yblockItem, xblockWidth, yblockWidth,
  //       cellW, cellH, lineWidth, edgeWidth, trayColor, frameColor,
  //       trayDirect:'csNull|csLeftTop|csLeftBottom|csRightTop|csRightBottom',
  //       showFont, colorMap:[...], cells:[{x,y,colorIndex,text}], onCellClick(x,y)}
  function makeMyTray(opt) {
    opt = opt || {};
    var xi = opt.xitem || 2, yi = opt.yitem || 2;           // XItem/YItem default 2
    var xb = opt.xblockItem || 0, yb = opt.yblockItem || 0; // Block：每 n 格加分隔
    var cw = opt.cellW || 22, ch = opt.cellH || 22;
    var cmap = opt.colorMap || ['#ffffff', '#00c000', '#ff4040', '#ffd700', '#40a0ff', '#c0c0c0'];
    var t = el('div', 'mytray');
    t.id = opt.name || '';
    t.title = (opt.name || 'TTMyTray') + ' : TTMyTray｜XItem=' + xi + ' YItem=' + yi +
      (xb ? ' XBlockItem=' + xb : '') + (yb ? ' YBlockItem=' + yb : '');
    t.style.setProperty('--tray-color', opt.trayColor || '#8a9a8a');
    t.style.setProperty('--tray-line', (opt.lineWidth || 1) + 'px');
    t.style.setProperty('--tray-edge', (opt.edgeWidth || 2) + 'px');
    var cellIdx = {};
    (opt.cells || []).forEach(function (c) { cellIdx[c.x + ',' + c.y] = c; });
    for (var y = 0; y < yi; y++) {
      var row = el('div', 'trow');
      if (yb && y > 0 && y % yb === 0) row.style.marginTop = (opt.yblockWidth || 4) + 'px'; // YBlockWidth
      for (var x = 0; x < xi; x++) {
        var cell = el('div', 'cell');
        cell.style.width = cw + 'px'; cell.style.height = ch + 'px';
        if (xb && x > 0 && x % xb === 0) cell.style.marginLeft = (opt.xblockWidth || 4) + 'px'; // XBlockWidth
        var c = cellIdx[x + ',' + y];
        if (c) {
          cell.style.background = cmap[c.colorIndex || 0] || '#fff';         // SetCellColorIndex
          if (opt.showFont !== false && c.text !== undefined) cell.textContent = c.text; // SetCellNumber
          if (opt.frameColor) cell.style.borderColor = opt.frameColor;      // SetFrameColor
        }
        cell.dataset.x = x; cell.dataset.y = y;
        cell.title = (opt.name || 'Tray') + ' Cell[' + x + '][' + y + ']' + (c && c.text ? ' = ' + c.text : '');
        if (opt.onCellClick) cell.addEventListener('click', (function (cx, cy) {
          return function () { opt.onCellClick(cx, cy); };
        })(x, y));
        row.appendChild(cell);
      }
      t.appendChild(row);
    }
    if (opt.trayDirect && opt.trayDirect !== 'csNull') {   // TrayDirect 方向角標
      t.appendChild(el('div', 'dirmark ' + opt.trayDirect));
    }
    // 對應 SetCellColorIndex / SetCellNumber / ClearCell
    t.setCell = function (x, y, colorIndex, text) {
      var cell = t.querySelector('.cell[data-x="' + x + '"][data-y="' + y + '"]');
      if (!cell) return;
      if (colorIndex !== undefined) cell.style.background = cmap[colorIndex] || '#fff';
      if (text !== undefined) cell.textContent = text;
    };
    t.clearCell = function () {
      t.querySelectorAll('.cell').forEach(function (c) { c.style.background = '#fff'; c.textContent = ''; });
    };
    return t;
  }

  // ===== TMyOmronPanel（EJ1N/MyOmronPanel.cpp）：Omron 溫控單通道面板 =====
  // opt:{name, ch:'CH1-1', pv:'0.0', degree:'℃', runStop:'STOP', at:'AT', inputErr:'Input Error', event:'Event', sv:'85', left, top}
  function makeOmronPanel(opt) {
    opt = opt || {};
    var box = el('div', 'omronpanel');
    box.style.cssText = 'position:absolute;width:206px;height:120px;background:#c3c7cf;'
      + 'border:1px solid #8890a0;box-sizing:border-box;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    box.title = (opt.name || 'myPal') + ' : TMyOmronPanel｜Caption=' + (opt.ch || '');
    var cap = el('span');
    cap.style.cssText = 'position:absolute;left:8px;top:-2px;font:bold 12px Arial;color:#000080;background:#c3c7cf;padding:0 3px;';
    cap.textContent = opt.ch || 'CH1-1';
    var disp = el('div');
    disp.style.cssText = 'position:absolute;left:4px;top:20px;width:197px;height:95px;background:#808080;'
      + 'border:1px solid #6e6e6e;font-family:Arial;overflow:hidden;';
    [[opt.degree || '℃', 1, 12, 4], [opt.runStop || 'STOP', 20, 12, 4],
     [opt.inputErr || 'Input Error', 39, 11, 2], [opt.at || 'AT', 58, 12, 4]].forEach(function (a) {
      var s = el('span');
      s.style.cssText = 'position:absolute;left:' + a[3] + 'px;top:' + a[1] + 'px;font-size:' + a[2] + 'px;color:#000;white-space:nowrap;';
      s.textContent = a[0]; disp.appendChild(s);
    });
    var pv = el('span');
    pv.style.cssText = 'position:absolute;left:72px;top:-3px;width:125px;text-align:center;font-size:40px;font-weight:bold;color:#ff0;';
    pv.textContent = opt.pv || '0.0'; disp.appendChild(pv);
    var ev = el('span');
    ev.style.cssText = 'position:absolute;left:72px;top:46px;width:125px;text-align:center;font-size:12px;color:#000;';
    ev.textContent = opt.event || 'Event'; disp.appendChild(ev);
    var cb = el('input'); cb.type = 'checkbox';
    cb.style.cssText = 'position:absolute;left:48px;top:76px;width:15px;height:17px;margin:0;';
    var ed = el('input'); ed.className = 'ed'; ed.value = opt.sv || '85';
    ed.style.cssText = 'position:absolute;left:72px;top:85px;width:128px;height:28px;border:none;background:#fff;color:#000;font:22px Arial;padding:0 4px;box-sizing:border-box;';
    box.appendChild(cap); box.appendChild(disp); box.appendChild(cb); box.appendChild(ed);
    return box;
  }

  // ===== TMotorTestClass（uMotorTest.cpp）：Motor Test 單列（cbUsing+labName+edPos1+edPos2）=====
  // opt:{name, label:'M00', v1:'0', v2:'0', left, top}
  function makeMotorTestRow(opt) {
    opt = opt || {};
    var row = el('div', 'motorrow');
    row.style.cssText = 'position:absolute;width:182px;height:24px;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    row.title = (opt.name || 'Motor') + ' : TMotorTestClass（cbUsing+labName+edPos1+edPos2）';
    var cb = el('input'); cb.type = 'checkbox';
    cb.style.cssText = 'position:absolute;left:0;top:3px;width:15px;height:17px;margin:0;';
    var lb = el('span');
    lb.style.cssText = 'position:absolute;left:16px;top:3px;width:31px;height:18px;font:13px Arial;color:#000;background:#a6b8c2;';
    lb.textContent = opt.label || 'M00';
    var e1 = el('input'); e1.className = 'ed'; e1.value = opt.v1 != null ? opt.v1 : '0';
    e1.style.cssText = 'position:absolute;left:50px;top:0;width:65px;height:24px;box-sizing:border-box;';
    var e2 = el('input'); e2.className = 'ed'; e2.value = opt.v2 != null ? opt.v2 : '0';
    e2.style.cssText = 'position:absolute;left:117px;top:0;width:65px;height:24px;box-sizing:border-box;';
    row.appendChild(cb); row.appendChild(lb); row.appendChild(e1); row.appendChild(e2);
    return row;
  }

  // ===== THomeClass（uhome.cpp）：Home Monitor 單列（labName+ledHome+edPos）=====
  // opt:{name, label:'[M00] MInArmX', pos:'0', on:true, left, top}
  function makeHomeRow(opt) {
    opt = opt || {};
    var row = el('div', 'homerow');
    row.style.cssText = 'position:absolute;width:245px;height:24px;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    row.title = (opt.name || 'Home') + ' : THomeClass（labName+ledHome+edPos）';
    var lb = el('span');
    lb.style.cssText = 'position:absolute;left:0;top:2px;height:20px;font:12pt "MS Sans Serif";color:#000080;white-space:nowrap;';
    lb.textContent = opt.label || '[M00] MInArmX';
    var led = makeALed({ name: (opt.name || 'ledHome'), ledStyle: 'LEDSqLarge',
      value: opt.on !== false, trueColor: '#00e000', falseColor: '#c0c0c0' });
    led.style.position = 'absolute'; led.style.left = '165px'; led.style.top = '0';
    var ed = el('input'); ed.className = 'ed'; ed.value = opt.pos != null ? opt.pos : '0'; ed.readOnly = true;
    ed.style.cssText = 'position:absolute;left:190px;top:0;width:50px;height:20px;box-sizing:border-box;';
    row.appendChild(lb); row.appendChild(led); row.appendChild(ed);
    return row;
  }

  // ===== TMyATPanel（AutoTemperature.cpp）：ATC 溫控量測點面板 =====
  // opt:{name, index:0, label:'HP 1-1', temp:'000.00', channel:'101', enabled:false, left, top}
  function makeATPanel(opt) {
    opt = opt || {};
    var idx = opt.index != null ? opt.index : 0;
    var gb = el('div', 'atpanel');
    gb.style.cssText = 'position:absolute;width:280px;height:105px;background:#ece9d8;border:1px solid #9aa;box-sizing:border-box;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    gb.title = (opt.name || 'gbMeasurePoint' + idx) + ' : TMyATPanel（ATC 量測點）';
    var lg = el('span');
    lg.style.cssText = 'position:absolute;left:8px;top:-9px;background:#ece9d8;padding:0 4px;font:14px Arial;color:#000;';
    lg.textContent = '[' + (idx < 10 ? '0' : '') + idx + '] ' + (opt.label || 'HP 1-1');
    var tp = el('div');
    tp.style.cssText = 'position:absolute;left:4px;top:21px;width:168px;height:25px;background:#9efcfe;border:1px inset #ccc;'
      + 'font:12px Arial;color:#000;display:flex;align-items:center;justify-content:center;';
    tp.textContent = opt.temp || '000.00';
    var ch = el('select'); ch.className = 'ed';
    ch.style.cssText = 'position:absolute;left:4px;top:48px;width:168px;height:28px;font:12px Arial;';
    var op = el('option'); op.textContent = opt.channel || '101'; ch.appendChild(op);
    var cb = el('label');
    cb.style.cssText = 'position:absolute;left:4px;top:80px;width:89px;font:12px Arial;color:#000;display:flex;align-items:center;gap:3px;';
    var cbi = el('input'); cbi.type = 'checkbox'; if (opt.enabled) cbi.checked = true;
    cb.appendChild(cbi); cb.appendChild(document.createTextNode('Use'));
    var img = el('div');
    img.style.cssText = 'position:absolute;left:174px;top:16px;width:100px;height:84px;border:1px dashed #99a;color:#889;'
      + 'font-size:10px;display:flex;align-items:center;justify-content:center;text-align:center;';
    img.textContent = 'Offset 曲線';
    gb.appendChild(lg); gb.appendChild(tp); gb.appendChild(ch); gb.appendChild(cb); gb.appendChild(img);
    return gb;
  }

  // ===== TMySecurity（cSecurity.cpp）：權限項目列（Panel+Glyph+RadioGroup 等級）=====
  // opt:{name, caption:'[00] Main - Tools', level:1, levels:[...], img, width, left, top}
  function makeSecurityRow(opt) {
    opt = opt || {};
    var levels = opt.levels || ['Open', 'Operator', 'Engineer', 'Supervisor', 'HonPrec'];
    var pnl = el('div', 'secrow');
    pnl.style.cssText = 'position:absolute;height:50px;background:#c2b8a6;box-sizing:border-box;'
      + 'width:' + (opt.width || 560) + 'px;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    pnl.title = (opt.name || 'Panel') + ' : TMySecurity（權限項目）';
    var sb = el('span');
    sb.style.cssText = 'position:absolute;left:4px;top:8px;width:34px;height:34px;border:2px outset #e6e3d6;background:#ece9d8;'
      + 'display:flex;align-items:center;justify-content:center;font-size:16px;';
    sb.textContent = opt.img || '🔑';
    var cap = el('span');
    cap.style.cssText = 'position:absolute;left:44px;top:4px;font:bold 12px "MS Sans Serif";color:#000080;white-space:nowrap;';
    cap.textContent = opt.caption || '[00] Main - Tools';
    var rg = el('fieldset');
    rg.style.cssText = 'position:absolute;left:44px;top:20px;right:6px;bottom:2px;border:1px solid #9aa;padding:0 0 0 6px;'
      + 'display:flex;gap:12px;align-items:center;';
    var rid = 'sec_' + (opt.name || Math.random().toString(36).slice(2));
    levels.forEach(function (lv, i) {
      var lb = el('label');
      lb.style.cssText = 'font:11px "MS Sans Serif";color:#222;display:inline-flex;align-items:center;gap:2px;white-space:nowrap;';
      var r = el('input'); r.type = 'radio'; r.name = rid; if (i === (opt.level != null ? opt.level : 1)) r.checked = true;
      lb.appendChild(r); lb.appendChild(document.createTextNode(lv)); rg.appendChild(lb);
    });
    pnl.appendChild(sb); pnl.appendChild(cap); pnl.appendChild(rg);
    return pnl;
  }

  // ===== THTSLKClass 家族（ContactForce.cpp）：接觸力 Load rate 群組（每個 Kit 直徑一組）=====
  //
  // Steven 20260919 重寫。舊版的座標是估的（trackbar left:130 / width:300、
  // 只畫 2 個編輯框、offset 欄位擺錯邊），與 golden 對不上。現在四個變體的
  // 每一個座標都抄自 ContactForce.dfm 的設計期原型群組，原型本身
  // Visible=False（它是樣板，執行期 new 出來的才是真的），但 BCB6 設計器仍會
  // 畫出來 —— 交付的實機截圖就是設計器畫面，所以截圖與 dfm 對得起來。
  //
  // 四個變體（同一家族，差在有沒有 NS 那一列、以及 offset 欄位的 Left）：
  //   THTSLKClass                  gbLoadRate                  842x100  兩列（含 NS）
  //   THTSLKIndClass               gbLoadRateInd               842x61   一列
  //   THTDieForceSLKClass          gbDieForceLoadRate          850x65   一列
  //   THTDieForceOneByOneSLKClass  gbDieForceOneByOneLoadRate  842x61   一列
  //
  // ⚠ 使用者 20260919 交付的 TfContactForce.THTSLKClass.png 其實是**一列**的樣子，
  //   對應的是 gbLoadRateInd（THTSLKIndClass），不是兩列的 gbLoadRate。
  //   兩者都留著，用 variant 選。
  //
  // TrackBar：Min=80 Max=150 Position=80 Frequency=1（dfm），對應 Load rate 0.80~1.50；
  // edtLoadRate 是 Position/100 的唯讀顯示（ContactForce.cpp trckbrDiameter_Change）。
  var SLK_VARIANTS = {
    // variant: {cls, w, h, rows:[{lbl,lblL,lblT, tkT, edRateT, offLbl,offLblL,offLblT, offEdL,offEdT}]}
    std: {
      cls: 'THTSLKClass', proto: 'gbLoadRate', w: 842, h: 100,
      rows: [{ lblT: 31, lblSfx: 'mm : ', tkT: 26, rateT: 26,
               offLblL: 464, offLblT: 18, offLblSfx: 'mm offset by heater mode', offEdL: 677, offEdT: 14 },
             { lblT: 63, lblSfx: 'mm for NS: ', tkT: 58, rateT: 58,
               offLblL: 464, offLblT: 45, offLblSfx: 'mm contact offset', offEdL: 677, offEdT: 39 }],
      // dfm 還有第三個標籤/編輯框（contact offset__NS），擺在第二列下面
      extra: [{ lblL: 464, lblT: 73, lblSfx: 'mm contact offset__NS', edL: 677, edT: 68 }]
    },
    ind: {
      cls: 'THTSLKIndClass', proto: 'gbLoadRateInd', w: 842, h: 61,
      rows: [{ lblT: 26, lblSfx: 'mm : ', tkT: 24, rateT: 24,
               offLblL: 507, offLblT: 26, offLblSfx: 'mm contact offset', offEdL: 676, offEdT: 23 }]
    },
    dieforce: {
      cls: 'THTDieForceSLKClass', proto: 'gbDieForceLoadRate', w: 850, h: 65,
      rows: [{ lblT: 31, lblSfx: 'mm : ', tkT: 26, rateT: 26,
               offLblL: 464, offLblT: 29, offLblSfx: 'mm contact offset', offEdL: 677, offEdT: 23 }]
    },
    dieforce1: {
      cls: 'THTDieForceOneByOneSLKClass', proto: 'gbDieForceOneByOneLoadRate', w: 842, h: 61,
      rows: [{ lblT: 26, lblSfx: 'mm : ', tkT: 24, rateT: 24,
               offLblL: 507, offLblT: 26, offLblSfx: 'mm contact offset', offEdL: 676, offEdT: 23 }]
    }
  };

  // opt:{name, dia:'60', variant:'std|ind|dieforce|dieforce1', pos:80, left, top, width}
  function makeContactForceGroup(opt) {
    opt = opt || {};
    var V = SLK_VARIANTS[opt.variant || 'std'] || SLK_VARIANTS.std;
    var dia = opt.dia == null ? '60' : String(opt.dia);
    var w = opt.width || V.w;
    var gb = el('div', 'slkgroup');
    gb.style.cssText = 'position:absolute;box-sizing:border-box;width:' + w + 'px;height:' + V.h + 'px;'
      + 'background:var(--form-bg,#ece9d8);border:1px solid var(--gbx-border,#99aaaa);'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    gb.id = opt.name || (V.proto + '_' + dia);
    gb.title = gb.id + ' : ' + V.cls + '（Load rate of ' + dia + ' mm；原型 ' + V.proto + '）';

    var lg = el('span');
    lg.style.cssText = 'position:absolute;left:8px;top:-10px;padding:0 4px;background:var(--form-bg,#ece9d8);'
      + 'font:16px var(--font-ui,"MS Sans Serif",sans-serif);color:var(--text,#000);white-space:nowrap;';
    lg.textContent = 'Load rate of ' + dia + ' mm';
    gb.appendChild(lg);

    function lab(left, top, text) {
      var s = el('span');
      s.style.cssText = 'position:absolute;left:' + left + 'px;top:' + top + 'px;'
        + 'font:16px var(--font-ui,"MS Sans Serif",sans-serif);color:var(--text,#000);white-space:nowrap;';
      s.textContent = text; gb.appendChild(s); return s;
    }
    function edit(left, top, width, ro) {
      var e = el('input', 'ed');
      e.style.cssText = 'position:absolute;left:' + left + 'px;top:' + top + 'px;width:' + width
        + 'px;height:28px;box-sizing:border-box;';
      if (ro) { e.readOnly = true; e.style.background = 'var(--panel-dk,#d4d0c8)'; }
      gb.appendChild(e); return e;
    }

    var pos = opt.pos == null ? 80 : opt.pos;    // dfm Position=80 → Load rate 0.80
    V.rows.forEach(function (r) {
      lab(6, r.lblT, dia + r.lblSfx);
      // TTrackBar：dfm Left=108 Width=275 Height=35，Min=80 Max=150
      var tk = el('input'); tk.type = 'range'; tk.min = 80; tk.max = 150; tk.step = 1; tk.value = pos;
      tk.style.cssText = 'position:absolute;left:108px;top:' + r.tkT + 'px;width:275px;height:35px;';
      gb.appendChild(tk);
      var rate = edit(380, r.rateT, 65, true);   // edtLoadRate：Enabled=false，值＝Position/100
      rate.value = (pos / 100).toFixed(2);
      tk.addEventListener('input', function () { rate.value = (tk.value / 100).toFixed(2); });
      lab(r.offLblL, r.offLblT, dia + r.offLblSfx);
      edit(r.offEdL, r.offEdT, 80, false);
    });
    (V.extra || []).forEach(function (x) {
      lab(x.lblL, x.lblT, dia + x.lblSfx);
      edit(x.edL, x.edT, 80, false);
    });
    return gb;
  }

  // ===== TMyVacuumPanel（VacuumUnit/MyVacuumPanel.cpp）：單一吸嘴真空面板 =====
  //
  // Steven 20260919 新增。VACUUM_UNIT_WIDTH=81 / VACUUM_UNIT_HEIGHT=177（VacuumUnit.h），
  // 座標全部抄自 VacuumUnit.dfm 的設計期原型 GroupBox1（Caption='myPalSamle'，Visible=False）。
  //
  // ⚠ 執行期與設計期的畫法不一樣，這裡照執行期畫：
  //   設計期原型用三個 TPanel（pnlCurectVal / pnlEvent / pnlThreshold）當「樣式來源」；
  //   執行期 TMyVacuumPanel 只 new 一個 TImage（ImgVacuumPanel，Left=4 Top=20，
  //   73x152），三行字是 Canvas->TextOutA 畫上去的，並不是三個真的 Panel。
  //   所以 HTML 用一個 .vacImg 區塊放三行置中文字，而不是三個 panel。
  //
  // 顏色取自 dfm：底 14540252=#DCDCDC、外框 clGray、目前值 clMaroon、閥值 clBlue、
  // Event clBlack；bplOn/bplOff Color=10307329=#014A9D、TrueColor=14464261=#45B0DC。
  //
  // opt:{name, caption:'Aa', cur:'0.0', event:'Event', threshold:'0.0', sv:'0',
  //      on:false, led:false, left, top}
  function makeVacuumPanel(opt) {
    opt = opt || {};
    var gb = el('div', 'vacpanel');
    gb.id = opt.name || 'myPal';
    gb.style.cssText = 'position:absolute;box-sizing:border-box;width:81px;height:177px;'
      + 'background:var(--form-bg,#ece9d8);border:1px solid var(--gbx-border,#99aaaa);'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    gb.title = gb.id + ' : TMyVacuumPanel（' + (opt.caption || '') + '；81x177）';

    var lg = el('span');
    lg.style.cssText = 'position:absolute;left:6px;top:-8px;padding:0 3px;background:var(--form-bg,#ece9d8);'
      + 'font:11px var(--font-ui,"MS Sans Serif",sans-serif);color:var(--text,#000);white-space:nowrap;';
    lg.textContent = opt.caption || 'myPalSamle';
    gb.appendChild(lg);

    // ImgVacuumPanel：dfm Top=20 Left=4，Width=81-8、Height=177-25
    var img = el('div', 'vacImg');
    img.style.cssText = 'position:absolute;left:4px;top:20px;width:73px;height:152px;'
      + 'background:#dcdcdc;border:1px solid #808080;box-sizing:border-box;';
    img.title = 'ImgVacuumPanel : TImage（執行期 Canvas 畫字，非三個 Panel）';
    gb.appendChild(img);

    // 三行 Canvas 文字。y 取自 cpp：pnlCurectVal.Top-10 / pnlEvent.Top-10 / pnlThreshold.Top-20，
    // 再換算成 ImgVacuumPanel 內的座標（扣掉 Img 自己的 Top=20）。
    function line(top, color, size, text, ttl) {
      var s = el('div');
      s.style.cssText = 'position:absolute;left:0;top:' + top + 'px;width:71px;text-align:center;'
        + 'font:' + size + 'px Arial,sans-serif;color:' + color + ';white-space:nowrap;overflow:hidden;';
      s.textContent = text; s.title = ttl; img.appendChild(s); return s;
    }
    var elCur = line(0, '#800000', 15, opt.cur == null ? '0.0' : opt.cur,
                     'ShowCurectVal()：目前真空值，clMaroon');
    var elEvt = line(20, '#000000', 11, opt.event || 'Event',
                     'SetEvent()：警報事件，clBlack');
    var elThr = line(34, '#0000ff', 15, opt.threshold == null ? '0.0' : opt.threshold,
                     'ShowThreshold()：閥值，clBlue');

    // edSV（Left=5 Top=72 71x20）
    var sv = el('input', 'ed');
    sv.id = gb.id + '_edSV';
    sv.value = opt.sv == null ? '0' : opt.sv;
    sv.style.cssText = 'position:absolute;left:5px;top:72px;width:71px;height:20px;box-sizing:border-box;'
      + 'font:13px var(--font-ui,"MS Sans Serif",sans-serif);';
    sv.title = 'edSV : TEdit（真空閥值輸入，範圍 -116~148）';
    gb.appendChild(sv);

    // btnSV（Left=6 Top=92 71x23，Caption='Set'）
    var set = el('button', 'btn3d');
    set.textContent = 'Set';
    set.style.cssText = 'position:absolute;left:6px;top:92px;width:71px;height:23px;'
      + 'font:13px var(--font-ui,"MS Sans Serif",sans-serif);';
    set.title = 'btnSV : TSpeedButton（寫入閥值）';
    gb.appendChild(set);

    // bplOn / bplOff（TBtnPanelLane，Left=15/41 Top=120 22x24，Caption '^' / 'v'）
    [['bplOn', '^', 15, '吸真空 ON'], ['bplOff', 'v', 41, '吸真空 OFF']].forEach(function (b) {
      var p = el('div');
      p.id = gb.id + '_' + b[0];
      p.textContent = b[1];
      p.style.cssText = 'position:absolute;left:' + b[2] + 'px;top:120px;width:22px;height:24px;'
        + 'display:flex;align-items:center;justify-content:center;cursor:pointer;user-select:none;'
        + 'font:bold 13px var(--font-ui,"MS Sans Serif",sans-serif);color:#fff;background:#014a9d;';
      p.title = b[0] + ' : TBtnPanelLane（' + b[3] + '；Down 時轉 TrueColor #45b0dc）';
      p.addEventListener('click', function () {
        var down = p.dataset.down === '1';
        p.dataset.down = down ? '0' : '1';
        p.style.background = down ? '#014a9d' : '#45b0dc';
      });
      gb.appendChild(p);
    });

    // myld1（TMyLed LEDSqLarge，Left=30 Top=149 20x20）＝真空感測回授
    var led = makeALed({ name: gb.id + '_myld1', value: !!opt.led, ledStyle: 'LEDSqLarge' });
    led.style.position = 'absolute'; led.style.left = '30px'; led.style.top = '149px';
    led.style.width = '20px'; led.style.height = '20px';
    led.title = 'myld1 : TMyLed（真空 Sensor 回授；Hint 帶 Lane/IP/Port/Bit/VCNo）';
    gb.appendChild(led);

    gb.setCurrent = function (v, c) { elCur.textContent = v; if (c) elCur.style.color = c; };
    gb.setThreshold = function (v, c) { elThr.textContent = v; if (c) elThr.style.color = c; };
    gb.setEvent = function (v, c) { elEvt.textContent = v; if (c) elEvt.style.color = c; };
    return gb;
  }

  // ===== TMyDutPanel（GPIB9045 MyDutPanel.cpp）：GPIB 橋接程式的單站面板，執行期動態建立 =====
  // Steven 20260926：新增。座標抄 TMyDutPanel::TMyDutPanel() 執行期的值，不抄 MyDutPanel.dfm 的原型
  //（dfm 原型 gbSite 是 150x120、cbSiteOn Checked=True、labOcr='OCR Text'；執行期是 150x110、
  //   Checked=false、labOcr=''。跑起來的是執行期 new 出來的那一份）。
  // 排列：iLeft=4 iTop=0，iLPitch=154 iTPitch=110，index%8 為欄、index/8 為列（32 站 = 8x4）。
  // 母表單 Color=8421440（BGR 0x808040 → #408080），GroupBox 字白色。
  // opt:{name, index:0, site:'01', bin:'1', on:false, ocr:'', bg, left, top}
  //   site 未給時依 index+1 補零；bin 是 cbBin 的 Text；ocr 是 labOcr（2D code 文字）。
  // 回傳元素帶 setSite(text,color) / setBin(text) / setOn(bool) / setOcr(text) 供執行期更新。
  var DUT_BIN_ITEMS = ['0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','1..6','0..16','0..255'];
  function makeDutPanel(opt) {
    opt = opt || {};
    var idx = opt.index != null ? opt.index : 0;
    var nn = ((idx + 1) < 10 ? '0' : '') + (idx + 1);
    var bg = opt.bg || '#408080';
    var gb = el('div', 'dutpanel');
    gb.id = opt.name || ('gpSite' + (idx < 10 ? '0' : '') + idx);
    gb.style.cssText = 'position:absolute;box-sizing:border-box;width:150px;height:110px;'
      + 'background:' + bg + ';'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    gb.title = gb.id + ' : TMyDutPanel（GPIB 32-site 面板，index=' + idx + '；150x110，pitch 154x110）';

    // 框線照 VCL TGroupBox 畫在控制項自己的範圍內：標題在 y=0，框線從字高一半（8px）開始。
    // 縱向 pitch 110 = 高度 110，若標題凸出框外會蓋到上一列的 labOcr，所以這裡不用其他 maker 的 top:-9 畫法。
    var fr = el('div');
    fr.style.cssText = 'position:absolute;left:0;right:0;top:8px;bottom:0;border:1px solid #d0d8d8;'
      + 'border-radius:3px;pointer-events:none;';
    gb.appendChild(fr);

    // gpSite Caption：'Site %02d'（index+1），Arial 12pt 白字
    var lg = el('span');
    lg.style.cssText = 'position:absolute;left:8px;top:0;padding:0 3px;background:' + bg + ';'
      + 'font:16px/16px Arial,sans-serif;color:#fff;white-space:nowrap;';
    lg.textContent = 'Site ' + (opt.site != null ? opt.site : nn);
    gb.appendChild(lg);

    // plSite（Left=12 Top=18 129x49，clSilver，BevelInner=bvLowered，Impact 26pt 白字，Caption='01'）
    var pl = el('div');
    pl.id = gb.id + '_plSite';
    pl.style.cssText = 'position:absolute;left:12px;top:18px;width:129px;height:49px;box-sizing:border-box;'
      + 'background:#c0c0c0;border:2px inset #e8e8e8;display:flex;align-items:center;justify-content:center;'
      + 'font:35px Impact,"Arial Black",sans-serif;color:#fff;overflow:hidden;white-space:nowrap;';
    pl.textContent = opt.site != null ? opt.site : nn;
    pl.title = 'plSite : TPanel（站號／測試結果顯示，執行期依 Bin 改 Color 與 Caption）';
    gb.appendChild(pl);

    // cbSiteOn（Left=15 Top=72 14x17，Caption=''，執行期 Checked=false）
    var on = el('input');
    on.type = 'checkbox';
    on.id = gb.id + '_cbSiteOn';
    on.checked = !!opt.on;
    on.style.cssText = 'position:absolute;left:15px;top:72px;width:14px;height:17px;margin:0;';
    on.title = 'cbSiteOn : TCheckBox（此站是否啟用；關 Site 不能有 Bin）';
    gb.appendChild(on);

    // cbBin（Left=35 Top=68 106x28，Arial 12pt 黑字，Items 0~15/1..6/0..16/0..255，Text='1'）
    var cb = el('select', 'ed');
    cb.id = gb.id + '_cbBin';
    cb.style.cssText = 'position:absolute;left:35px;top:68px;width:106px;height:28px;box-sizing:border-box;'
      + 'font:16px Arial,sans-serif;color:#000;background:#fff;';
    cb.title = 'cbBin : TComboBox（手動測試／模擬用的 Bin 選擇；0..255 需 iBinSelect>15）';
    DUT_BIN_ITEMS.forEach(function (t) { var o = el('option'); o.value = t; o.textContent = t; cb.appendChild(o); });
    cb.value = opt.bin != null ? String(opt.bin) : '1';
    gb.appendChild(cb);

    // labOcr（Align=alBottom，置中，Arial 8pt 白字，執行期 Caption=''；2D Code 文字）
    var oc = el('div');
    oc.id = gb.id + '_labOcr';
    oc.style.cssText = 'position:absolute;left:2px;right:2px;bottom:1px;height:13px;text-align:center;'
      + 'font:11px Arial,sans-serif;color:#fff;white-space:nowrap;overflow:hidden;';
    oc.textContent = opt.ocr || '';
    oc.title = 'labOcr : TLabel（2D Code / OCR 文字，Steven 20150713）';
    gb.appendChild(oc);

    gb.setSite = function (t, c) { pl.textContent = t; if (c) pl.style.background = c; };
    gb.setBin  = function (t) { cb.value = String(t); };
    gb.setOn   = function (b) { on.checked = !!b; };
    gb.setOcr  = function (t) { oc.textContent = t || ''; };
    return gb;
  }

  // ===== TMyYieldPanel（uYieldMonitoring.cpp）：Bin 良率設定面板（3 Tray + BinSetting ScrollBox + ART limit）=====
  // opt:{name, bins:8, left, top}
  function makeYieldPanel(opt) {
    opt = opt || {};
    var bins = opt.bins || 8, rh = 24;
    var pnl = el('div', 'yieldpanel');
    pnl.style.cssText = 'position:absolute;width:980px;height:' + (rh * bins + 60) + 'px;background:#ece9d8;box-sizing:border-box;'
      + (opt.left != null ? 'left:' + opt.left + 'px;' : '') + (opt.top != null ? 'top:' + opt.top + 'px;' : '');
    pnl.title = (opt.name || 'PalYield') + ' : TMyYieldPanel（Bin 良率設定）';
    [['mtTrayItem', 8, 117, 'Item'], ['mtTrayName', 125, 140, 'Name']].forEach(function (c) {
      var col = el('div');
      col.style.cssText = 'position:absolute;left:' + c[1] + 'px;top:8px;width:' + c[2] + 'px;height:' + (rh * bins) + 'px;'
        + 'background:#8a9a8a;border:2px solid #444;box-sizing:border-box;';
      col.title = c[0] + ' : TTMyTray256';
      for (var i = 0; i < bins; i++) {
        var cell = el('div');
        cell.style.cssText = 'height:' + rh + 'px;border-bottom:1px solid #555;background:#fff;font:11px sans-serif;'
          + 'display:flex;align-items:center;padding:0 4px;box-sizing:border-box;';
        cell.textContent = c[3] === 'Item' ? ('BIN' + (i + 1)) : ('Bin Name ' + (i + 1));
        col.appendChild(cell);
      }
      pnl.appendChild(col);
    });
    var sb = el('div');
    sb.style.cssText = 'position:absolute;left:265px;top:8px;width:701px;height:' + (rh * bins) + 'px;'
      + 'background:#fff;border:1px inset #ccc;overflow:auto;box-sizing:border-box;';
    sb.title = 'sbBinSetting : TScrollBox（Bin 選擇矩陣）';
    for (var r = 0; r < bins; r++) {
      var row = el('div');
      row.style.cssText = 'height:' + rh + 'px;display:flex;gap:10px;align-items:center;padding:0 6px;border-bottom:1px solid #eee;';
      for (var b = 0; b < 12; b++) { var ck = el('input'); ck.type = 'checkbox'; row.appendChild(ck); }
      sb.appendChild(row);
    }
    pnl.appendChild(sb);
    var la = el('span');
    la.style.cssText = 'position:absolute;left:8px;top:' + (rh * bins + 20) + 'px;font:11px sans-serif;color:#000;';
    la.textContent = 'Auto Retest limit'; la.title = 'laARTLinit : TLabel';
    var cb = el('select'); cb.className = 'ed';
    cb.style.cssText = 'position:absolute;left:150px;top:' + (rh * bins + 16) + 'px;width:100px;';
    cb.title = 'ReTestLimit : TComboBox';
    ['1', '2', '3', '4', '5'].forEach(function (v) { var o = el('option'); o.textContent = v; cb.appendChild(o); });
    pnl.appendChild(la); pnl.appendChild(cb);
    return pnl;
  }

  window.HTWidgets = {
    makeALed: makeALed,
    makeMyLed: makeMyLed,
    makeMyLedLane: makeMyLedLane,
    makeLabeledALed: makeLabeledALed,
    makeMyLabeledLed: makeMyLabeledLed,
    makeMyLabeledLedLane: makeMyLabeledLedLane,
    makeBtnPanel: makeBtnPanel,
    makeBtnPanelLane: makeBtnPanelLane,
    makeSpeedButton: makeSpeedButton,
    makeMyTray: makeMyTray,
    makeOmronPanel: makeOmronPanel,
    makeMotorTestRow: makeMotorTestRow,
    makeHomeRow: makeHomeRow,
    makeATPanel: makeATPanel,
    makeSecurityRow: makeSecurityRow,
    makeContactForceGroup: makeContactForceGroup,
    makeVacuumPanel: makeVacuumPanel,      // Steven 20260919
    makeDutPanel: makeDutPanel,            // Steven 20260926（GPIB9045 TMyDutPanel）
    makeYieldPanel: makeYieldPanel
  };
})();
